/* @source ajbamindex *********************************************************
**
** AJAX BAM files indexing/querying functions
**
** @version $Revision: 1.12 $
** @modified 2011 Mahmut Uludag ported from samtools (samtools.sf.net)
** @modified $Date: 2012/07/04 07:40:28 $ by $Author: rice $
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public
** License along with this library; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA  02110-1301,  USA.
**
******************************************************************************/

/* The MIT License
**
**   Copyright (c) 2008 Genome Research Ltd (GRL).
**
**   Permission is hereby granted, free of charge, to any person obtaining
**   a copy of this software and associated documentation files (the
**   "Software"), to deal in the Software without restriction, including
**   without limitation the rights to use, copy, modify, merge, publish,
**   distribute, sublicense, and/or sell copies of the Software, and to
**   permit persons to whom the Software is furnished to do so, subject to
**   the following conditions:
**
**   The above copyright notice and this permission notice shall be
**   included in all copies or substantial portions of the Software.
**
**   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
**   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
**   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
**   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
**   BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
**   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
**   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
**   SOFTWARE.
*/


#include "ajlib.h"

#include "ajbamindex.h"
#include "ajutil.h"

#include <ctype.h>
#include <assert.h>


#ifdef WIN32
#define strdup _strdup
#endif

/*!
  @header

  Alignment indexing. Before indexing, BAM must be sorted based on the
  leftmost coordinate of alignments. In indexing, BAM uses two indices:
  a UCSC binning index and a simple linear index. The binning index is
  efficient for alignments spanning long distance, while the auxiliary
  linear index helps to reduce unnecessary seek calls especially for
  short alignments.

  The UCSC binning scheme was suggested by Richard Durbin and Lincoln
  Stein and is explained by Kent et al. (2002). In this scheme, each bin
  represents a contiguous genomic region which can be fully contained in
  another bin; each alignment is associated with a bin which represents
  the smallest region containing the entire alignment. The binning
  scheme is essentially another representation of R-tree. A distinct bin
  uniquely corresponds to a distinct internal node in a R-tree. Bin A is
  a child of Bin B if region A is contained in B.

  In BAM, each bin may span 2^29, 2^26, 2^23, 2^20, 2^17 or 2^14 bp. Bin
  0 spans a 512Mbp region, bins 1-8 span 64Mbp, 9-72 8Mbp, 73-584 1Mbp,
  585-4680 128Kbp and bins 4681-37449 span 16Kbp regions. If we want to
  find the alignments overlapped with a region [rbeg,rend), we need to
  calculate the list of bins that may be overlapped the region and test
  the alignments in the bins to confirm the overlaps. If the specified
  region is short, typically only a few alignments in six bins need to
  be retrieved. The overlapping alignments can be quickly fetched.

 */

#define BAM_MIN_CHUNK_GAP 32768
/* 1<<14 is the size of minimum bin. */

#define BAM_LIDX_SHIFT    14

#define BAM_MAX_BIN 37450 /* =(8^6-1)/7+1 */


struct __bam_iter_t
{
    int from_first; /* read from the first record; no random access */
    int tid, beg, end, n_off, i, finished;
    ajulong curr_off;
    pair64_t *off;
};

typedef struct __bam_iter_t *bam_iter_t;


static void bamBinDel(void** bin);
static void bamIndexSave(const AjPBamIndex idx, FILE *fp);
static void bamIterDel(bam_iter_t* iter);
static bam_iter_t bamIterQuery(const AjPBamIndex idx, int tid,
			       int beg, int end);
static int bamIterRead(AjPSeqBamBgzf fp, bam_iter_t iter, AjPSeqBam);
static AjPBamIndex bamIndexCore(AjPSeqBamBgzf fp);




/* @funcstatic bamInsertOffset ************************************************
**
** Inserts a new chunk to the binning index h
**
** @param [u] h [AjPTable] binning index
** @param [r] bin [ajint] bin number
** @param [r] beg [ajulong] start of target region
** @param [r] end [ajulong] end of target region
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static inline void bamInsertOffset(AjPTable h, ajint bin, ajulong beg,
				   ajulong end)
{
    bam_binlist_t *l = NULL;
    int* tmp = NULL;

    l = ajTableFetchmodV(h, &bin);

    if (!l)
    {
	AJNEW0(l);
	l->m = 1;
	l->n = 0;
	l->list = (pair64_t*)calloc(l->m, 16);

	AJNEW0(tmp);
	*tmp = bin;

	ajTablePut(h, tmp, l);
    }

    if (l->n == l->m)
    {
	l->m <<= 1;
	l->list = (pair64_t*)realloc(l->list, l->m * 16);
    }

    l->list[l->n].u = beg;
    l->list[l->n++].v = end;
}




/* @funcstatic bamInsertOffset2 ***********************************************
**
** Inserts a new offset to the linear index
**
** @param [u] index2 [bam_lidx_t*] linear index
** @param [r] b [const AjPSeqBam] alignment record
** @param [r] offset [ajulong] offset
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static inline void bamInsertOffset2(bam_lidx_t *index2, const AjPSeqBam b,
				    ajulong offset)
{
    ajint i;
    ajint beg;
    ajint end;
    ajint old_m;

    beg = b->core.pos >> BAM_LIDX_SHIFT;
    end = (ajSeqBamCalend(&b->core, MAJSEQBAMCIGAR(b)) - 1) >> BAM_LIDX_SHIFT;

    if (index2->m < end + 1)
    {
	old_m = index2->m;
	index2->m = end + 1;
	kroundup32(index2->m);
	index2->offset = (ajulong*)realloc(index2->offset, index2->m * 8);
	memset(index2->offset + old_m, 0, 8 * (index2->m - old_m));
    }

    if (beg == end)
    {
	if (index2->offset[beg] == 0)
	    index2->offset[beg] = offset;
    }
    else
    {
	for (i = beg; i <= end; ++i)
	    if (index2->offset[i] == 0)
		index2->offset[i] = offset;
    }

    index2->n = end + 1;
}




/* @funcstatic bamMergeChunks *************************************************
**
** merge binning index chunks
**
** @param [u] idx [AjPBamIndex] index object
** @return [void]
**
**
** @release 6.5.0
** @@
******************************************************************************/

static void bamMergeChunks(AjPBamIndex idx)
{
#if defined(BAM_TRUE_OFFSET) || defined(BAM_VIRTUAL_OFFSET16)
    AjPTable bindex = NULL;
    bam_binlist_t* p = NULL;
    bam_binlist_t** bins = NULL;
    ajint** binids = NULL;
    int i, m;
    ajulong n;
    ajuint l;
    ajulong k =0UL;

    for (i = 0; i < idx->n; ++i)
    {
	bindex = idx->bindex[i];

	n = ajTableToarrayKeysValues(bindex, (void***)&binids, (void***)&bins);

	for (k = 0; k<n; ++k)
	{

	    if(*binids[k] == BAM_MAX_BIN)  /* metadata */
		continue;

	    p = bins[k];
	    m = 0;

	    for (l = 1; l < p->n; ++l)
	    {

#ifdef BAM_TRUE_OFFSET
		if (p->list[m].v + BAM_MIN_CHUNK_GAP > p->list[l].u)
		    p->list[m].v = p->list[l].v;
#else
		if (p->list[m].v>>16 == p->list[l].u>>16)
		    p->list[m].v = p->list[l].v;
#endif

		else
		    p->list[++m] = p->list[l];
	    }

	    p->n = m + 1;

	}
    }

    AJFREE(bins);
    AJFREE(binids);

#endif
}




/* @funcstatic bamFillMissing *************************************************
**
** Fills missing linear index bits with the values of previous cells
** in the index
**
** @param [u] idx [AjPBamIndex] index object
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void bamFillMissing(AjPBamIndex idx)
{
    bam_lidx_t *idx2 = NULL;
    ajint i;
    ajint j;

    for (i = 0; i < idx->n; ++i)
    {
	idx2 = &idx->index2[i];

	for (j = 1; j < idx2->n; ++j)
	    if (idx2->offset[j] == 0)
		idx2->offset[j] = idx2->offset[j-1];
    }
}




/* @funcstatic bamIndexCore ***************************************************
**
** Reads BAM index files
**
** @param [u] fp [AjPSeqBamBgzf] BAM file handler
** @return [AjPBamIndex] Bam index object
**
** @release 6.5.0
** @@
******************************************************************************/

static AjPBamIndex bamIndexCore(AjPSeqBamBgzf fp)
{
    AjPSeqBam b = NULL;
    AjPSeqBamHeader h = NULL;
    AjPBamIndex idx = NULL;
    AjPSeqBamCore c = NULL;
    ajint i;
    ajint ret;
    ajuint last_bin, save_bin;
    ajint last_coor;
    ajint last_tid;
    ajint save_tid;
    ajlong last_off;
    ajulong save_off, n_mapped, n_unmapped, off_beg, off_end, n_no_coor;

    AJNEW0(idx);
    AJNEW0(b);

    h = ajSeqBamHeaderRead(fp);

    c = &b->core;

    idx->n = h->n_targets;
    ajSeqBamHeaderDel(&h);

    idx->bindex = (AjPTable*)calloc(idx->n, sizeof(AjPTable));

    for (i = 0; i < idx->n; ++i)
    {
	idx->bindex[i] = ajTableintNew(100);
    }

    idx->index2 = (bam_lidx_t*)calloc(idx->n, sizeof(bam_lidx_t));

    save_bin = save_tid = last_tid = last_bin = 0xffffffffu;
    save_off = last_off = bam_tell(fp); last_coor = 0xffffffffu;
    n_mapped = n_unmapped = n_no_coor = off_end = 0;
    off_beg = off_end = bam_tell(fp);


    while ((ret = ajSeqBamRead(fp, b)) >= 0)
    {
	if (c->tid < 0)
	    ++n_no_coor;

	if (last_tid < c->tid || (last_tid >= 0 && c->tid < 0))
	{ /* change of chromosomes/reference-sequences */
	    last_tid = c->tid;
	    last_bin = 0xffffffffu;
	}
	else if (last_tid > c->tid)
	{
	    ajErr("[bam_index_core] the alignment is not sorted (%s):"
		    " %d-th chr > %d-th chr\n",
		    MAJSEQBAMQNAME(b), last_tid+1, c->tid+1);
	    return NULL;
	}
	else if (c->tid >= 0 && last_coor > c->pos)
	{
	    ajErr("[bam_index_core] the alignment is not sorted (%s):"
		    " %u > %u in %d-th chr\n",
		    MAJSEQBAMQNAME(b), last_coor, c->pos, c->tid+1);
	    return NULL;
	}

	if (c->tid >= 0 && !(c->flag & BAM_FUNMAP))
	    bamInsertOffset2(&idx->index2[b->core.tid], b, last_off);

	if (c->bin != last_bin) /* then possibly write the binning index */
	{
	    /* save_bin==0xffffffffu only happens to the first record */
	    if (save_bin != 0xffffffffu)
		bamInsertOffset(idx->bindex[save_tid], save_bin, save_off,
		              last_off);

	    if (last_bin == 0xffffffffu && save_tid != -1)
	    { /* write the meta element */
		off_end = last_off;
		bamInsertOffset(idx->bindex[save_tid], BAM_MAX_BIN,
			off_beg, off_end);
		bamInsertOffset(idx->bindex[save_tid], BAM_MAX_BIN,
			n_mapped, n_unmapped);
		n_mapped = n_unmapped = 0;
		off_beg = off_end;
	    }

	    save_off = last_off;
	    save_bin = last_bin = c->bin;
	    save_tid = c->tid;

	    if (c->tid < 0)
		break;
	}

	if (bam_tell(fp) <= last_off)
	{
	    ajErr("bamIndexCore: bug in BGZF/RAZF: %llx < %llx\n",
	          (unsigned long long)bam_tell(fp),
	          (unsigned long long)last_off);
	    return NULL;
	}

	if (c->flag & BAM_FUNMAP)
	    ++n_unmapped;
	else
	    ++n_mapped;

	last_off = bam_tell(fp);
	last_coor = b->core.pos;
    }

    if (save_tid >= 0)
    {
	bamInsertOffset(idx->bindex[save_tid], save_bin, save_off,
	              bam_tell(fp));
	bamInsertOffset(idx->bindex[save_tid], BAM_MAX_BIN, off_beg,
		      bam_tell(fp));
	bamInsertOffset(idx->bindex[save_tid], BAM_MAX_BIN, n_mapped,
		      n_unmapped);
    }

    bamMergeChunks(idx);
    bamFillMissing(idx);

    if (ret >= 0)
    {
	while ((ret = ajSeqBamRead(fp, b)) >= 0)
	{
	    ++n_no_coor;
	    if (c->tid >= 0 && n_no_coor)
	    {
		ajErr("bamIndexCore: the alignment is not sorted:"
			" reads without coordinates"
			" prior to reads with coordinates.");
		return NULL;
	    }
	}
    }

    if (ret < -1)
	ajWarn("bamIndexCore: truncated file");

    AJFREE(b->data);
    AJFREE(b);

    idx->n_no_coor = n_no_coor;
    return idx;
}




/* @funcstatic bamBinDel ******************************************************
**
** Deletes a bin of a binning index
**
** is called from ajTableSetDestroyvalue function
**
** @param [d] bin [void**] bin object in a binning index
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void bamBinDel(void** bin)
{
    bam_binlist_t* p = *bin;
    AJFREE(p->list);
    AJFREE(p);
    *bin = NULL;
}




/* @func ajBamIndexDel ********************************************************
**
** Deletes a BAM index object
**
**
** @param [d] Pidx [AjPBamIndex*] index object
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ajBamIndexDel(AjPBamIndex* Pidx)
{
    AjPBamIndex idx = NULL;
    int i;

    idx = *Pidx;

    if (idx == 0) return;

    for (i = 0; i < idx->n; ++i)
    {
	bam_lidx_t *index2 = idx->index2 + i;

	free(index2->offset);
	ajTableSetDestroyvalue(idx->bindex[i], bamBinDel);
	ajTableDel(&idx->bindex[i]);
    }

    AJFREE(idx->index2);
    AJFREE(idx->bindex);
    AJFREE(*Pidx);
}




/* @funcstatic bamIndexSave ***************************************************
**
** Save an index
**
** @param [r] idx [const AjPBamIndex] index object
** @param [u] fp [FILE*] file pointer
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void bamIndexSave(const AjPBamIndex idx, FILE *fp)
{
    AjPTable bindex = NULL;
    bam_lidx_t *index2 = NULL;
    bam_binlist_t **bins = NULL;
    ajuint **binids = NULL;
    ajulong l = 0;
    ajint i=0;
    ajuint j=0;
    ajuint size=0;
    ajint k=0;
    ajuint x = 0;
    ajint bam_is_be = 0;

    bam_is_be = ajUtilGetBigendian();

    fwrite("BAI\1", 1, 4, fp);

    if (bam_is_be)
    {
	x = idx->n;
	ajByteRevLen4u(&x);
	fwrite(&x, 4, 1, fp);
    }
    else
	fwrite(&idx->n, 4, 1, fp);

    for (i = 0; i < idx->n; ++i)
    {
	bindex = idx->bindex[i];
	index2 = idx->index2 + i;

	/* binning index */

	/* # distinct bins */
	size = (ajuint) ajTableToarrayKeysValues(bindex,
						 (void***)&binids,
						 (void***)&bins);

	if (bam_is_be) /*big endian*/
	{
	    x = size;
	    ajByteRevLen4u(&x);
	    fwrite(&x, 4, 1, fp);
	}
	else
	    fwrite(&size, 4, 1, fp);

	for (j = 0; j < size; ++j)
	{
	    bam_binlist_t *binlist = bins[j];

	    if (bam_is_be) /*big endian*/
	    {
		x = *binids[j];
		ajByteRevLen4u(&x);
		fwrite(&x, 4, 1, fp);
		x = binlist->n;
		ajByteRevLen4u(&x);
		fwrite(&x, 4, 1, fp);

		for (x = 0; x < binlist->n; ++x)
		{
		    ajByteRevLen8u((ajulong*)&binlist->list[x].u);
		    ajByteRevLen8u((ajulong*)&binlist->list[x].v);
		}

		fwrite(binlist->list, 16, binlist->n, fp);

		for (x = 0; x < binlist->n; ++x)
		{
		    ajByteRevLen8u((ajulong*)&binlist->list[x].u);
		    ajByteRevLen8u((ajulong*)&binlist->list[x].v);
		}

	    }
	    else
	    {
		fwrite(binids[j], 4, 1, fp);
		fwrite(&binlist->n, 4, 1, fp);
		fwrite(binlist->list, 16, binlist->n, fp);
	    }
	}

	/* write linear index (index2) */
	if (bam_is_be)
	{
	    x = index2->n;
	    ajByteRevLen4u(&x);
	    fwrite(&x, 4, 1, fp);
	}
	else
	    fwrite(&index2->n, 4, 1, fp);

	if (bam_is_be)
	{
	    for (k = 0; k < index2->n; ++k)
		ajByteRevLen8u((ajulong*)&index2->offset[k]);

	    fwrite(index2->offset, 8, index2->n, fp);

	    for (k = 0; k < index2->n; ++k)
		ajByteRevLen8u((ajulong*)&index2->offset[k]);
	}
	else
	    fwrite(index2->offset, 8, index2->n, fp);

	AJFREE(bins);
	AJFREE(binids);
    }

    { /* write the number of reads without coordinate */
	l = idx->n_no_coor;

	if (bam_is_be)
	    ajByteRevLen8u((ajulong*)&l);

	fwrite(&l, 8, 1, fp);
    }

    fflush(fp);
}




/* @funcstatic bamIndexLoadCore ***********************************************
**
** Load BAM index data from a file
**
** @param [u] fp [FILE*] file pointer
** @return [AjPBamIndex] BAM index
**
** @release 6.5.0
** @@
******************************************************************************/

static AjPBamIndex bamIndexLoadCore(FILE *fp)
{
    AjPBamIndex idx = NULL;
    ajint i;
    char magic[4];
    ajint bam_is_be = 0;
    bam_lidx_t *index2 = NULL;
    ajuint binid;
    ajint n_bin; /* # distinct bins */
    ajint j;
    bam_binlist_t *binlist = NULL;
    ajint* tmp = NULL;
    ajuint x;

    bam_is_be = ajUtilGetBigendian();

    if (fp == 0)
    {
	ajErr("bamIndexLoadCore: fail to load index.\n");
	return 0;
    }

    fread(magic, 1, 4, fp);

    if (strncmp(magic, "BAI\1", 4))
    {
	ajErr("bamIndexLoadCore: wrong magic number.\n");
	fclose(fp);
	return 0;
    }

    AJNEW0(idx);

    fread(&idx->n, 4, 1, fp);
    if (bam_is_be)
	ajByteRevLen4(&idx->n);
    ajDebug("# reference sequences: %d\n", idx->n);

    idx->bindex = (AjPTable*)calloc(idx->n, sizeof(AjPTable));
    idx->index2 = (bam_lidx_t*)calloc(idx->n, sizeof(bam_lidx_t));

    for (i = 0; i < idx->n; ++i)
    {
	index2 = idx->index2 + i;
	binlist = NULL;

	idx->bindex[i] = ajTableintNew(10);

	/* load binning index */
	fread(&n_bin, 4, 1, fp);
	if (bam_is_be)
	    ajByteRevLen4(&n_bin);
	ajDebug("# distinct bins (for the reference sequence %d): %d\n",
		i, n_bin);

	for (j = 0; j < n_bin; ++j)
	{

	    fread(&binid, 4, 1, fp);
	    if (bam_is_be)
		ajByteRevLen4u(&binid);

	    AJNEW0(tmp);
	    *tmp = binid;
	    AJNEW0(binlist);
	    ajTablePut(idx->bindex[i], tmp, binlist);

	    fread(&binlist->n, 4, 1, fp);
	    if (bam_is_be)
		ajByteRevLen4u(&binlist->n);
	    ajDebug("\tbin %u has %d chunks\n", binid, binlist->n);


	    binlist->m = binlist->n;
	    binlist->list = (pair64_t*)malloc(binlist->m * 16);
	    fread(binlist->list, 16, binlist->n, fp);
	    if (bam_is_be)
	    {
		for (x = 0; x < binlist->n; ++x)
		{
		    ajByteRevLen8u((ajulong*)&binlist->list[x].u);
		    ajByteRevLen8u((ajulong*)&binlist->list[x].v);
		}
	    }

	    if (ajDebugOn())
		for (x = 0; x < binlist->n; ++x)
		{
		    ajlong mChunkStart = binlist->list[x].u;
		    ajlong mChunkEnd   = binlist->list[x].v;

		    ajDebug("\t\tchunk: %d:%d-%d:%d\n",
			    mChunkStart >> 16,
			    mChunkStart & 0xFFFF,
			    mChunkEnd >> 16,
			    mChunkEnd & 0xFFFF);
		}

	}

	/* load linear index */
	fread(&index2->n, 4, 1, fp);
	if (bam_is_be)
	    ajByteRevLen4(&index2->n);
	index2->m = index2->n;

	ajDebug("\tRef %u has %d 16kbp intervals\n", i, index2->n);

	index2->offset = (ajulong*)calloc(index2->m, 8);
	fread(index2->offset, index2->n, 8, fp);
	if (bam_is_be)
	    for (j = 0; j < index2->n; ++j)
		ajByteRevLen8u((ajulong*)&index2->offset[j]);


    }

    if (fread(&idx->n_no_coor, 8, 1, fp) == 0)
	idx->n_no_coor = 0;

    if (bam_is_be)
	ajByteRevLen8u((ajulong*)&idx->n_no_coor);

    return idx;
}




/* @func ajBamIndexLoad *******************************************************
**
** Load index from file "filename.bai"
**
** TODO: ftp and http support not yet implemented
**
** @param [r] _fn [const char*] name of the BAM file (NOT the index file)
** @return [AjPBamIndex] BAM index object
**
** @release 6.5.0
******************************************************************************/

AjPBamIndex ajBamIndexLoad(const char *_fn)
{
    FILE *fp = NULL;
    AjPBamIndex idx = NULL;
    char *fnidx = NULL;
    char *fn = NULL;
    char *s = NULL;
    const char *p = NULL;
    ajint l = 0;

    if (strstr(_fn, "ftp://") == _fn || strstr(_fn, "http://") == _fn)
    {
	l = strlen(_fn);

	for (p = _fn + l - 1; p >= _fn; --p)
	    if (*p == '/')
		break;

	fn = strdup(p + 1);
    }
    else
	fn = strdup(_fn);

    fnidx = (char*)calloc(strlen(fn) + 5, 1);
    strcpy(fnidx, fn); strcat(fnidx, ".bai");
    fp = fopen(fnidx, "rb");

    if (fp == 0)
    { /* try "{base}.bai" */
	s = strstr(fn, "bam");

	if (s == fn + strlen(fn) - 3)
	{
	    strcpy(fnidx, fn);
	    fnidx[strlen(fn)-1] = 'i';
	    fp = fopen(fnidx, "rb");
	}
    }

    free(fnidx);
    free(fn);

    if (fp)
    {
	idx = bamIndexLoadCore(fp);
	fclose(fp);
	return idx;
    }
    else
	return NULL;
}




/* @func ajBamIndexBuild ******************************************************
**
** Build index for a BAM file, using .bai extension for the index file
**
** @param [r] fn [const char*] name of the BAM file
** @return [int] 0 if the index build was successful
**
**
** @release 6.5.0
******************************************************************************/

int ajBamIndexBuild(const char *fn)
{
    char *fnidx;
    FILE *fpidx = NULL;
    AjPSeqBamBgzf fp = NULL;
    AjPBamIndex idx = NULL;

    if(!strcmp(fn, "stdout"))
	return -1;

    if ((fp = ajSeqBamBgzfOpenC(fn, "r")) == 0)
    {
	ajErr("ajBamIndexBuild: fail to open the BAM file '%s'.", fn);
	return -1;
    }

    idx = bamIndexCore(fp);
    ajSeqBamBgzfClose(fp);

    if(idx == 0)
    {
	ajErr("ajBamIndexBuild: fail to index the BAM file '%s'.", fn);
	return -1;
    }

    fnidx = (char*)calloc(strlen(fn) + 5, 1);
    strcpy(fnidx, fn); strcat(fnidx, ".bai");

    fpidx = fopen(fnidx, "wb");

    if (fpidx == 0)
    {
	ajErr("ajBamIndexBuild: fail to create the index file '%s'.", fn);
	free(fnidx);
	return -1;
    }

    bamIndexSave(idx, fpidx);

    AJFREE(fnidx);

    ajBamIndexDel(&idx);
    fclose(fpidx);

    return 0;
}




/* @funcstatic bamReg2bins ****************************************************
**
** Calculate the list of bins that may overlap with region [beg, end)
 * (zero based).
 *
 * picard def: Get candidate bins for the specified region.
**
** @param [r] beg [ajuint] 1-based start of target region, inclusive
** @param [r] end [ajuint] 1-based end of target region, inclusive
** @param [w] list [ajushort[BAM_MAX_BIN]] list of bins that may overlap
**                                         with region
** @return [int] number of bins
**
** @release 6.5.0
** @@
******************************************************************************/

static inline int bamReg2bins(ajuint beg, ajuint end,
			      ajushort list[BAM_MAX_BIN])
{
    ajuint i=0;
    ajuint k=0;

    if (beg >= end)
	return 0;

    if (end >= 1u<<29)
	end = 1u<<29;

    --end;
    list[i++] = 0;

    for (k =    1 + (beg>>26); k <=    1 + (end>>26); ++k) list[i++] = k;
    for (k =    9 + (beg>>23); k <=    9 + (end>>23); ++k) list[i++] = k;
    for (k =   73 + (beg>>20); k <=   73 + (end>>20); ++k) list[i++] = k;
    for (k =  585 + (beg>>17); k <=  585 + (end>>17); ++k) list[i++] = k;
    for (k = 4681 + (beg>>14); k <= 4681 + (end>>14); ++k) list[i++] = k;

    return i;
}




/* @funcstatic bamIsOverlap ***************************************************
**
** Checks whether a region overlaps a BAM record
**
** @param [r] beg [ajuint] region start position
** @param [r] end [ajuint] region end position
** @param [r] b [const AjPSeqBam] BAM record
** @return [int] 1 if the region overlaps the BAM record, 0 otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

static inline int bamIsOverlap(ajuint beg, ajuint end, const AjPSeqBam b)
{
    ajuint rbeg = b->core.pos;
    ajuint rend = 0;

    if(b->core.n_cigar)
	rend = ajSeqBamCalend(&b->core, MAJSEQBAMCIGAR(b));
    else
	rend = b->core.pos + 1;

    return (rend > beg && rbeg < end);
}




/* @funcstatic bamChunkCompare ************************************************
**
** Compares the virtual file offset of the start of two chunks
** for binning indices
**
** @param [r] a [const void*] first chunk
** @param [r] b [const void*] second chunk
** @return [ajint] 1 if the first virtual offset is smaller, 0 otherwise
**
** @release 6.5.0
** @@
******************************************************************************/

static ajint bamChunkCompare(const void* a, const void* b)
{
    return  (((pair64_t const *)a)->u > ((pair64_t const *)b)->u);
}




/* @funcstatic bamIterQuery ***************************************************
**
** query a region on a reference sequence
**
** @param [r] idx [const AjPBamIndex] BAM index
** @param [r] tid [int] reference sequence id
** @param [r] beg [int] 1-based start of target region, inclusive
** @param [r] end [int] 1-based end of target region, inclusive
** @return [bam_iter_t] iteration object
**
** @release 6.5.0
** @@
******************************************************************************/

static bam_iter_t bamIterQuery(const AjPBamIndex idx, int tid,
			       int beg, int end)
{
    pair64_t *off=NULL;
    AjPTable bindex = NULL;
    AjPSeqBam b = NULL;
    ajushort *bins=NULL;
    const bam_binlist_t *p = NULL;
    ajulong min_off;
    bam_iter_t iter = 0;
    ajint i=0;
    ajint n_bins=0;
    ajint n_off=0;
    ajint key;
    ajint l;
    ajint n;
    ajuint j;

    if (beg < 0)
	beg = 0;

    if (end < beg)
	return 0;

    /* initialize iter */
    iter = calloc(1, sizeof(struct __bam_iter_t));
    iter->tid = tid;
    iter->beg = beg;
    iter->end = end;
    iter->i = -1;

    bins = (ajushort*)calloc(BAM_MAX_BIN, 2);
    n_bins = bamReg2bins(beg, end, bins);

    bindex = idx->bindex[tid];

    if (idx->index2[tid].n > 0)
    {
	if (beg>>BAM_LIDX_SHIFT >= idx->index2[tid].n)
	    min_off = idx->index2[tid].offset[idx->index2[tid].n-1];
	else
	    min_off = idx->index2[tid].offset[beg>>BAM_LIDX_SHIFT];

	if (min_off == 0)
	{ /* improvement for mapindex files built by tabix prior to 0.1.4 */
	    n = beg>>BAM_LIDX_SHIFT;

	    if (n > idx->index2[tid].n)
		n = idx->index2[tid].n;

	    for (i = n - 1; i >= 0; --i)
		if (idx->index2[tid].offset[i] != 0)
		    break;

	    if (i >= 0)
		min_off = idx->index2[tid].offset[i];
	}
    }
    else
	min_off = 0; /* tabix 0.1.2 may produce such mapindex files */


    /* iterate over the list of bins that may overlap with region */
    for (i = n_off = 0; i < n_bins; ++i)
    {
	key = bins[i];
	p =  ajTableFetchV(bindex, &key);

	if(p)
	{
	    n_off += p->n;
	    ajDebug("bin:%u   # chunks:%d\n", key, p->n);
	}

    }

    ajDebug("n_off:%d\n", n_off);

    if (n_off == 0)
    {
	free(bins);
	return iter;
    }

    off = (pair64_t*)calloc(n_off, 16);

    for (i = n_off = 0; i < n_bins; ++i)
    {
	key = bins[i];
	p = ajTableFetchV(bindex, &key);

	if(p)
	{
	    for (j = 0; j < p->n; ++j)
	    {
		if (p->list[j].v > min_off)
		    off[n_off++] = p->list[j];

		ajDebug("p->list[%d].v:%u  min_off:%d\n",
			j, p->list[j].v, min_off);
	    }
	}
    }

    free(bins);

    if (n_off == 0)
    {
	free(off);
	return iter;
    }

    AJNEW0(b);

    qsort(off, n_off, sizeof(pair64_t), bamChunkCompare);

    /* resolve completely contained adjacent blocks */
    for (i = 1, l = 0; i < n_off; ++i)
	if (off[l].v < off[i].v)
	    off[++l] = off[i];
    n_off = l + 1;

    /* resolve overlaps between adjacent blocks;
     * this may happen due to the merge in indexing
     */

    for (i = 1; i < n_off; ++i)
	if (off[i-1].v >= off[i].u)
	    off[i-1].v = off[i].u;

    { /* merge adjacent blocks */
#if defined(BAM_TRUE_OFFSET) || defined(BAM_VIRTUAL_OFFSET16)
	    for (i = 1, l = 0; i < n_off; ++i)
	    {
#ifdef BAM_TRUE_OFFSET
		if (off[l].v + BAM_MIN_CHUNK_GAP > off[i].u)
		    off[l].v = off[i].v;
#else
		if (off[l].v>>16 == off[i].u>>16)
		    off[l].v = off[i].v;
#endif
		else off[++l] = off[i];
	    }
	    n_off = l + 1;
#endif
    }

    ajSeqBamDel(&b);

    iter->n_off = n_off; iter->off = off;

    return iter;
}




/* @funcstatic bamIterDel *****************************************************
**
** Deletes a BAM iteration object
**
** @param [d] iterp [bam_iter_t*] pointer to iteration object to be deleted
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void bamIterDel(bam_iter_t* iterp)
{
    bam_iter_t iter;

    iter = *iterp;

    if (iter)
    {
	free(iter->off);
	free(iter);
    }

    *iterp = NULL;
}




/* @funcstatic bamIterRead ****************************************************
**
** ajBamFetch helper function retrieves data into alignment record b
**
** @param [u] fp [AjPSeqBamBgzf] BAM file
** @param [u] iter [bam_iter_t] BAM iteration object
** @param [u] b [AjPSeqBam] BAM record
** @return [int] positive integer (1 or more) if a record successfully read
**
** @release 6.5.0
** @@
******************************************************************************/

static int bamIterRead(AjPSeqBamBgzf fp, bam_iter_t iter, AjPSeqBam b)
{
    int ret;

    if (iter && iter->finished)
	return -1;

    if (iter == 0 || iter->from_first)
    {
	ret = ajSeqBamRead(fp, b);

	if (ret < 0 && iter)
	    iter->finished = 1;

	return ret;
    }

    if (iter->off == 0)
	return -1;

    for (;;)
    {
	if (iter->curr_off == 0 || iter->curr_off >= iter->off[iter->i].v)
	{ /* then jump to the next chunk */
	    if (iter->i == iter->n_off - 1)
	    {
		ret = -1;
		break; /* no more chunks */
	    }

	    if (iter->i >= 0)
		assert(iter->curr_off==iter->off[iter->i].v); /*otherwise bug*/

	    if (iter->i < 0 || iter->off[iter->i].v != iter->off[iter->i+1].u)
	    { /* not adjacent chunks; then seek */
		ajSeqBamBgzfSeek(fp, iter->off[iter->i+1].u, SEEK_SET);
		iter->curr_off = bam_tell(fp);
	    }
	    ++iter->i;
	}

	if ((ret = ajSeqBamRead(fp, b)) >= 0)
	{
	    iter->curr_off = bam_tell(fp);

	    if (b->core.tid != iter->tid || b->core.pos >= iter->end)
	    { /* no need to proceed */

		/* determine whether end of region or error */
		ret = ajSeqBamValidate(NULL, b)? -1 : -5;
		break;
	    }
	    else if (bamIsOverlap(iter->beg, iter->end, b))
		return ret;
	}
	else
	    break; /* end of file or error */
    }

    iter->finished = 1;

    return ret;
}




/* @func ajBamFetch ***********************************************************
**
**  Retrieve the alignments that are overlapped with the specified region.
**
** @param [u] fp   [AjPSeqBamBgzf] BAM file handler
** @param [r] idx  [const AjPBamIndex] pointer to the alignment index
** @param [r] tid  [int]             chromosome ID as is defined in the header
** @param [r] beg  [int]             start coordinate, 0-based
** @param [r] end  [int]             end coordinate, 0-based
** @param [u] data [void*]           user provided data
**                                   (will be transferred to func)
** @param [f] func [bam_fetch_f]     user defined function
**
** @return [int] Comparison value. 0 if equal, -1 if first is lower,
**               +1 if first is higher.
**
** @release 6.5.0
** @@
******************************************************************************/

int ajBamFetch(AjPSeqBamBgzf fp, const AjPBamIndex idx, int tid,
	       int beg, int end, void *data, bam_fetch_f func)
{
    int ret;
    bam_iter_t iter = NULL;
    AjPSeqBam b;

    AJNEW0(b);
    iter = bamIterQuery(idx, tid, beg, end);

    while ((ret = bamIterRead(fp, iter, b)) >= 0)
	func(b, data);

    bamIterDel(&iter);
    ajSeqBamDel(&b);

    return (ret == -1)? 0 : ret;
}

/* @include ajbamindex ********************************************************
**
** AJAX BAM files indexing/querying functions
**
** @version $Revision: 1.4 $
** @modified 2011 Mahmut Uludag ported from samtools (samtools.sf.net)
** @modified $Date: 2012/04/26 17:36:15 $ by $Author: mks $
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

#ifndef AJBAMINDEX_H
#define AJBAMINDEX_H

#include "ajdefine.h"
#include "ajseqbam.h"


/* @data pair64_t *************************************************************
**
** A [start,stop) file pointer pairing into the BAM file, stored
** as a BAM file index.  A chunk is represented as a single 64-bit
** value where the high-order 48 bits point to the location of the
** start of a compressed BGZF block within a BGZF file and the
** low-order 16 bits point to a position within the decompressed
** data in the BGZF block.
** ref: Chunk.java in picard project
**
** @attr u [ajulong] virtual file offset of the start of the chunk
** @attr v [ajulong] virtual file offset of the end of the chunk
******************************************************************************/

typedef struct pair64_t
{
    ajulong u;
    ajulong v;
} pair64_t;


/* @data bam_binlist_t ********************************************************
**
** bin object in a binning index for a reference sequence
**
** @attr m [ajuint] allocated size of the chunk array
**                  and is always >= n
** @attr n [ajuint] number of chunks
** @attr list [pair64_t*] array of BAM file chunk start/stop offsets
******************************************************************************/

typedef struct bam_binlist_t
{
    ajuint m;
    ajuint n;
    pair64_t *list;
} bam_binlist_t;


/* @data bam_lidx_t ***********************************************************
**
** linear index for a reference sequence
**
** @attr n [ajint] number of chunks
** @attr m [ajint] allocated size of the list or offset array
**                 and is always >= n
** @attr offset [ajulong*] Array of offsets
**
******************************************************************************/

typedef struct bam_lidx_t
{
    ajint n;
    ajint m;
    ajulong *offset;
} bam_lidx_t;




/* @data AjPBamIndex ********************************************************
**
** Structure for BAM indexes. (samtools name: bam_index_t)
**
** @attr  Padding [ajuint] Padding to alignment boundary
** @attr  n     [ajint]  number of reference sequences
** @attr  n_no_coor [ajulong]  unmapped reads without coordinate
** @attr  bindex    [AjPTable*]  list of binning indices
** @attr  index2    [bam_lidx_t*]  list of intervals for the linear index
******************************************************************************/

typedef struct AjSBamIndex
{
    ajuint Padding;
    ajint n;
    ajulong n_no_coor;
    AjPTable* bindex;
    bam_lidx_t *index2;
} AjOBamIndex;

#define AjPBamIndex AjOBamIndex*




/*
 * Return a virtual file pointer to the current location in the file.
 * No interpretation of the value should be made, other than a subsequent
 * call to ajSeqBamBgzfSeek can be used to position the file at the same point.
 * Return value is non-negative on success.
 * Returns -1 on error.
 */
#define bgzf_tell(fp) ((fp->block_address << 16) | (fp->block_offset & 0xFFFF))
#define bam_tell(fp) bgzf_tell(fp)




/* @datatype bam_fetch_f ******************************************************
**
**  Type of function to be called by ajBamFetch().
**
**  @attr typedef [int] Value returned
**
******************************************************************************/
typedef int (*bam_fetch_f)(AjPSeqBam b, void *data);



int ajBamFetch(AjPSeqBamBgzf fp, const AjPBamIndex idx, int tid,
	       int beg, int end, void *data, bam_fetch_f func);

int ajBamIndexBuild(const char *fn);
void ajBamIndexDel(AjPBamIndex* idx);
AjPBamIndex ajBamIndexLoad(const char *fn);


#endif /* !AJBAMINDEX_H */

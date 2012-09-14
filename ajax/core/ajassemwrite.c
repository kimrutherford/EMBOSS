/* @source ajassemwrite *******************************************************
**
** AJAX assembly writing functions
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.50 $
** @modified Oct 25 2010 pmr First AJAX version
** @modified $Date: 2012/07/05 12:04:37 $ by $Author: rice $
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


#include "ajlib.h"

#include "ajassem.h"
#include "ajassemwrite.h"
#include "ajseqbam.h"
#include "ajfileio.h"
#include "ajseq.h"
#include "ajutil.h"
#include "ajnam.h"
#include "ajbamindex.h"

#include <ctype.h>

#ifdef WIN32
#define strdup _strdup
#endif

static AjPStr assemoutSamLinetxt = NULL;

static AjBool assemoutWriteList(AjPFile outf, const AjPAssem assem);

static AjBool assemoutWriteBam(AjPFile outf, const AjPAssem assem);
static AjBool assemoutWriteMaf(AjPFile outf, const AjPAssem assem);
static AjBool assemoutWriteSam(AjPFile outf, const AjPAssem assem);
static AjBool assemoutWriteNextList(AjPOutfile outfile, const AjPAssem assem);
static AjBool assemoutWriteNextBam(AjPOutfile outfile, const AjPAssem assem);
static AjBool assemoutWriteNextMaf(AjPOutfile outfile, const AjPAssem assem);
static AjBool assemoutWriteNextSam(AjPOutfile outfile, const AjPAssem assem);
static AjBool assemoutWriteSamAlignment(AjPFile outf, const AjPAssemRead r,
					AjPAssemContig const* contigs,
					ajint ncontigs);
static AjBool assemoutWriteBamAlignment(AjPSeqBamBgzf gzfile,
					const AjPAssemRead r,
					AjPSeqBam bam);

static AjPStr assemoutMakeCigar(const char* contig, const char* readseq);

static const AjPStr assemSAMGetReadgroupHeaderlines(const AjPAssem assem);
static void assemoutBamIndex(AjPOutfile outf);




/* @datastatic AssemPOutFormat ************************************************
**
** Assembly output formats data structure
**
** @alias AssemSoutFormat
** @alias AssemOOutFormat
**
** @attr Name [const char*] Format name
** @attr Desc [const char*] Format description
** @attr Write [AjBool function] Output function, returns ajTrue on success
** @attr WriteNext [AjBool function] Partial output function
** @attr Cleanup [void function] Function to write remaining lines on closing
** @@
******************************************************************************/

typedef struct AssemSOutFormat
{
    const char *Name;
    const char *Desc;
    AjBool (*Write) (AjPFile outf, const AjPAssem assem);
    AjBool (*WriteNext) (AjPOutfile outfile, const AjPAssem assem);
    void (*Cleanup) (AjPOutfile outf);
} AssemOOutFormat;

#define AssemPOutFormat AssemOOutFormat*


static AssemOOutFormat assemoutFormatDef[] =
{
/* "Name",        "Description" */
/*      WriteFunction       WriteNextFunction  */

  {"bam",         "BAM assembly format",
        &assemoutWriteBam,  &assemoutWriteNextBam,  &assemoutBamIndex},
  {"list",        "Identifier only",
        &assemoutWriteList, &assemoutWriteNextList, NULL},
  {"maf",        "MIRA assembly format",
        &assemoutWriteMaf,  &assemoutWriteNextMaf,  NULL},
  {"sam",        "SAM assembly format",
        &assemoutWriteSam,  &assemoutWriteNextSam,  NULL},

  {NULL, NULL, NULL, NULL, NULL}
};




/* @filesection ajassem *******************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
******************************************************************************/





/* @datasection [AjPFile] Assembly data output ********************************
**
** Function is for manipulating assembly data objects
**
** @nam2rule Assemout Assembly data output
**
******************************************************************************/




/* @section assembly data outputs *********************************************
**
** These functions write the assembly data provided by the first argument
**
** @fdata [AjPFile]
**
** @nam3rule Write Write assembly data
** @nam4rule Format Use a named format
** @nam4rule Next  Write next block of data
**
** @argrule Write outf [AjPOutfile] Output file
** @argrule Write assem [const AjPAssem] Assembly data
** @argrule Format fmt [const AjPStr] Format name
**
** @valrule * [AjBool] true on success
**
** @fcategory output
**
******************************************************************************/




/* @func ajAssemoutWrite ******************************************************
**
** Write assembly data in a named format
**
** @param [u] outf [AjPOutfile] Output file
** @param [r] assem [const AjPAssem] Assembly data object
**
** @return [AjBool] True on success
**
**
** @release 6.5.0
******************************************************************************/

AjBool ajAssemoutWrite(AjPOutfile outf, const AjPAssem assem)
{
    ajuint i = ajOutfileGetFormatindex(outf);
    AjPFile outfile = ajOutfileGetFile(outf);

    ajDebug("ajAssemoutWrite %d\n",i);
    return (*assemoutFormatDef[i].Write)(outfile, assem);
}




/* @func ajAssemoutWriteNext **************************************************
**
** Write latest chunk of assembly data in a named format
**
** @param [u] outf [AjPOutfile] Output file
** @param [r] assem [const AjPAssem] Assembly data object
**
** @return [AjBool] True on success
**
**
** @release 6.5.0
******************************************************************************/

AjBool ajAssemoutWriteNext(AjPOutfile outf, const AjPAssem assem)
{
    ajuint i = ajOutfileGetFormatindex(outf);

    ajDebug("ajAssemoutWriteNext %d\n",i);
    if(!outf->Cleanup && assemoutFormatDef[i].Cleanup)
        outf->Cleanup = assemoutFormatDef[i].Cleanup;

    return (*assemoutFormatDef[i].WriteNext)(outf, assem);
}




/* @funcstatic assemoutWriteBam ***********************************************
**
** Writes an assembly in binary alignment/map (BAM) format.
**
** The sort order is "unsorted". Samtools can re-sort the file.
**
** @param [u] outf [AjPFile] Output file object.
** @param [r] assem [const AjPAssem] Assembly object
** @return [AjBool] True on success
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool assemoutWriteBam(AjPFile outf, const AjPAssem assem)
{
    AjPSeqBamHeader header = NULL;
    AjPAssemContig c = NULL;
    AjPSeqBam bam;
    AjPAssemRead   r = NULL;
    AjPAssemContig* contigs = NULL;
    AjPAssemTag    t = NULL;
    AjIList j = NULL;
    AjPSeqBamBgzf gzfile = NULL;
    AjPStr headertext=NULL;
    const AjPStr rgheadertext=NULL;
    AjBool ret = ajTrue;
    ajint i=0;
    ajulong ncontigs=0UL;


    if(ajListGetLength(assem->ContigsOrder))
	ncontigs = ajListToarray(assem->ContigsOrder, (void***)&contigs);
    else
	ncontigs = ajTableToarrayValues(assem->Contigs, (void***)&contigs);

    AJNEW0(bam);
    bam->m_data=10;
    AJCNEW0(bam->data, bam->m_data);

    gzfile = ajSeqBamBgzfNew(ajFileGetFileptr(outf), "w");

    ajFmtPrintS(&headertext, "@HD\tVN:1.3\tSO:%s\n",
		ajAssemGetSortorderC(assem));
    header = ajSeqBamHeaderNewN((ajuint) ncontigs);

    while (contigs[i])   /* contigs */
    {
	c = contigs[i];

	if(ajStrMatchC(c->Name, "*"))
	{
	    i++;
	    continue;
	}

	header->target_name[i] = strdup(ajStrGetPtr(c->Name));
	header->target_len[i++] = c->Length;

	ajFmtPrintAppS(&headertext, "@SQ\tSN:%S\tLN:%d",
		c->Name, c->Length);

	if(c->URI)
	    ajFmtPrintAppS(&headertext, "\tUR:%S", c->URI);

	if(c->MD5)
	    ajFmtPrintAppS(&headertext, "\tM5:%S", c->MD5);

	if(c->Species)
	    ajFmtPrintAppS(&headertext, "\tSP:%S", c->Species);

	ajFmtPrintAppS(&headertext, "\n");


	j = ajListIterNewread(c->Tags);
	while (!ajListIterDone(j))
	{
	    t = ajListIterGet(j);
	    ajFmtPrintAppS(&headertext,
		    "@CO\t%S %u %u %S\n", t->Name, t->x1, t->y1,
		    t->Comment);
	}
	ajListIterDel(&j);
    }

    rgheadertext = assemSAMGetReadgroupHeaderlines(assem);
    if(rgheadertext)
	ajStrAppendS(&headertext, rgheadertext);

    ajSeqBamHeaderSetTextC(header, ajStrGetPtr(headertext));
    ajSeqBamHeaderWrite(gzfile, header);

    j = ajListIterNewread(assem->Reads);

    while (!ajListIterDone(j))  /* reads */
    {
	r = ajListIterGet(j);
	assemoutWriteBamAlignment(gzfile, r, bam);
    }

    ajListIterDel(&j);

    ajSeqBamBgzfClose(gzfile);
    ajSeqBamHeaderDel(&header);
    ajStrDel(&headertext);

    AJFREE(contigs);
    AJFREE(bam->data);
    AJFREE(bam);

    ajBamIndexBuild(ajFileGetNameC(outf));

    return ret;
}




/* @funcstatic assemoutBamIndex ***********************************************
**
** Closes and idexes a BAM output file
**
** @param [u] outf [AjPOutfile] Output file object.
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void assemoutBamIndex(AjPOutfile outf)
{
    char *fname = ajCharNewC(ajFileGetNameC(ajOutfileGetFile(outf)));
    AjPSeqBamBgzf gzfile = outf->OutData;

    ajSeqBamBgzfClose(gzfile);
    ajFileClose(&outf->File);

    ajBamIndexBuild(fname);

    AJFREE(fname);

    return;
}




/* @funcstatic assemoutWriteNextBam *******************************************
**
** Writes an assembly in binary alignment/map (BAM) format.
**
** The sort order is "unsorted". Samtools can re-sort the file.
**
** @param [u] outfile [AjPOutfile] Output file object.
** @param [r] assem [const AjPAssem] Assembly object
** @return [AjBool] True on success
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool assemoutWriteNextBam(AjPOutfile outfile, const AjPAssem assem)
{
    AjPFile outf = ajOutfileGetFile(outfile);
    AjPSeqBamHeader header = NULL;
    AjPAssemContig c = NULL;
    AjPSeqBam bam;
    AjPAssemRead   r = NULL;
    AjPAssemContig* contigs = NULL;
    AjPAssemTag    t = NULL;
    AjIList j = NULL;
    AjPSeqBamBgzf gzfile = NULL;
    AjPStr headertext=NULL;
    const AjPStr rgheadertext=NULL;
    AjBool ret = ajTrue;
    ajint i=0;
    ajulong ncontigs=0UL;

    if(!outf) return ajFalse;
    if(!assem) return ajFalse;

    if(!assem->Hasdata)
    {
        if(ajListGetLength(assem->ContigsOrder))
            ncontigs = ajListToarray(assem->ContigsOrder, (void***)&contigs);
        else
            ncontigs = ajTableToarrayValues(assem->Contigs, (void***)&contigs);

        ajFmtPrintS(&headertext, "@HD\tVN:1.3\tSO:%s\n",
                    ajAssemGetSortorderC(assem));
        header = ajSeqBamHeaderNewN((ajuint) ncontigs);

        gzfile = ajSeqBamBgzfNew(ajFileGetFileptr(outf), "w");
        outfile->OutData = gzfile;

        while (contigs[i])   /* contigs */
        {
            c = contigs[i];

            if(ajStrMatchC(c->Name, "*"))
            {
                i++;
                continue;
            }

            header->target_name[i] = strdup(ajStrGetPtr(c->Name));
            header->target_len[i++] = c->Length;

            ajFmtPrintAppS(&headertext, "@SQ\tSN:%S\tLN:%d",
                           c->Name, c->Length);

            if(c->URI)
                ajFmtPrintAppS(&headertext, "\tUR:%S", c->URI);

            if(c->MD5)
                ajFmtPrintAppS(&headertext, "\tM5:%S", c->MD5);

            if(c->Species)
                ajFmtPrintAppS(&headertext, "\tSP:%S", c->Species);

            ajFmtPrintAppS(&headertext, "\n");


            j = ajListIterNewread(c->Tags);
            while (!ajListIterDone(j))
            {
                t = ajListIterGet(j);
                ajFmtPrintAppS(&headertext,
                               "@CO\t%S %u %u %S\n", t->Name, t->x1, t->y1,
                               t->Comment);
            }
            ajListIterDel(&j);
        }

        rgheadertext = assemSAMGetReadgroupHeaderlines(assem);
        if(rgheadertext)
            ajStrAppendS(&headertext, rgheadertext);

        ajSeqBamHeaderSetTextC(header, ajStrGetPtr(headertext));
        ajSeqBamHeaderWrite(gzfile, header);

        ajSeqBamHeaderDel(&header);
        ajStrDel(&headertext);

        AJFREE(contigs);

        if(!assem->BamHeader)
            return ajTrue;
    }

    /* data */

    gzfile = outfile->OutData;

    AJNEW0(bam);
    bam->m_data=10;
    AJCNEW0(bam->data, bam->m_data);

    j = ajListIterNewread(assem->Reads);

    while (!ajListIterDone(j))  /* reads */
    {
	r = ajListIterGet(j);
	assemoutWriteBamAlignment(gzfile, r, bam);
    }

    ajListIterDel(&j);

    AJFREE(bam->data);
    AJFREE(bam);

    /* ajSeqBamBgzfClose(gzfile);*/

    return ret;
}




/* @funcstatic assemoutWriteBamAlignment **************************************
**
** Write individual alignment records of assembly data in BAM format
**
** @param [u] gzfile [AjPSeqBamBgzf] Output file
** @param [r] r [const AjPAssemRead] Read object with alignment information
** @param [u] bam [AjPSeqBam] samtools data structure to store
**                            alignment data before it is written in BAM format
**
** @return [AjBool] True on success
**
** @release 6.5.0
**
******************************************************************************/

static AjBool assemoutWriteBamAlignment(AjPSeqBamBgzf gzfile,
					const AjPAssemRead r,
					AjPSeqBam bam)
{
    AjPSeqBamCore c;
    AjPAssemTag tag;
    unsigned char *dpos;
    const char *s;
    ajuint ilen;
    ajuint slen;
    ajuint i;
    AjIList l = NULL;

    /* optional fields */
    ajint tagvalsize = 0;
    const unsigned char* tagval = 0;
    ajint intval =0;

    /* processing cigar strings*/
    char *t;
    int op;
    long x;


    unsigned char bam_nt16_table[256] =
    {
     15,15,15,15, 15,15,15,15, 15,15,15,15, 15,15,15,15,
     15,15,15,15, 15,15,15,15, 15,15,15,15, 15,15,15,15,
     15,15,15,15, 15,15,15,15, 15,15,15,15, 15,15,15,15,
     1, 2, 4, 8, 15,15,15,15, 15,15,15,15, 15, 0 /*=*/,15,15,
     15, 1,14, 2, 13,15,15, 4, 11,15,15,12, 15, 3,15,15,
     15,15, 5, 6,  8,15, 7, 9, 15,10,15,15, 15,15,15,15,
     15, 1,14, 2, 13,15,15, 4, 11,15,15,12, 15, 3,15,15,
     15,15, 5, 6,  8,15, 7, 9, 15,10,15,15, 15,15,15,15,
     15,15,15,15, 15,15,15,15, 15,15,15,15, 15,15,15,15,
     15,15,15,15, 15,15,15,15, 15,15,15,15, 15,15,15,15,
     15,15,15,15, 15,15,15,15, 15,15,15,15, 15,15,15,15,
     15,15,15,15, 15,15,15,15, 15,15,15,15, 15,15,15,15,
     15,15,15,15, 15,15,15,15, 15,15,15,15, 15,15,15,15,
     15,15,15,15, 15,15,15,15, 15,15,15,15, 15,15,15,15,
     15,15,15,15, 15,15,15,15, 15,15,15,15, 15,15,15,15,
     15,15,15,15, 15,15,15,15, 15,15,15,15, 15,15,15,15
    };

    /* bam_write1 for each alignment */

    c = &bam->core;

    ilen = ajStrGetLen(r->Seq);

    c->tid = (int) r->Reference;

    if(r->Flag & BAM_FREVERSE)
	c->pos = r->y1-1;
    else
	c->pos = r->x1-1; /* BAM format is zero based;
                             -1 is translated to 0, meaning unmapped */
    c->bin = 0;
    c->qual = r->MapQ;
    c->l_qname = 1 + ajStrGetLen(r->Name);
    c->flag = r->Flag;
    c->n_cigar = 0;
    c->l_qseq = ilen;
    c->mtid = (int) r->Rnext;
    c->mpos = (int) r->Pnext-1;
    c->isize = r->Tlen;


    /* get cigar string length */
    s = ajStrGetPtr(r->Cigar);
    if (strcmp(s,"*")) /* '*' means unavailable  */
    {
	for (; *s; ++s)
	{
	  if ((isalpha((int)*s)) || (*s=='='))
		++c->n_cigar;
	    else
	      if (!isdigit((int)*s))
		    ajWarn("invalid CIGAR character: %c\n", *s);
	}
    }


    bam->data_len = c->n_cigar*4 + c->l_qname +
        (ilen + 1)/2 + ilen;

    /* allocation for optional tags are made as they are appended */

    if(bam->data_len > bam->m_data)
    {
        AJCRESIZE0(bam->data,bam->m_data, bam->data_len);
        bam->m_data = bam->data_len;
    }

    dpos = bam->data;

    /* copy query name to bam->data */
    memcpy(dpos, ajStrGetPtr(r->Name), c->l_qname);
    dpos += c->l_qname;

    /* copy cigar string to bam->data */
    s = ajStrGetPtr(r->Cigar);
    for (i = 0; i != c->n_cigar; ++i)
    {
	x = strtol(s, &t, 10);
	op = toupper((int)*t);
	if (op == 'M') op = BAM_CMATCH;
	else if (op == 'I') op = BAM_CINS;
	else if (op == 'D') op = BAM_CDEL;
	else if (op == 'N') op = BAM_CREF_SKIP;
	else if (op == 'S') op = BAM_CSOFT_CLIP;
	else if (op == 'H') op = BAM_CHARD_CLIP;
	else if (op == 'P') op = BAM_CPAD;
	else if (op == '=') op = BAM_CEQUAL;
	else if (op == 'X') op = BAM_CDIFF;
	else ajWarn("invalid CIGAR operation: %c",op);
	s = t + 1;
	((ajuint*)dpos)[i] = x << BAM_CIGAR_SHIFT | op;
    }

    if (*s && c->n_cigar)
	ajWarn("unmatched CIGAR operation: %c", *s);

    c->bin = ajSeqBamReg2bin(c->pos, ajSeqBamCalend(c, MAJSEQBAMCIGAR(bam)));

    dpos += c->n_cigar*4;


    /* copy sequence string to bam->data */
    s = ajStrGetPtr(r->Seq);
    slen = (ilen+1)/2;
    for (i = 0; i < slen; ++i)
        dpos[i] = 0;
    for (i = 0; i < ilen; ++i)
        dpos[i/2] |= bam_nt16_table[(ajuint)s[i]] << 4*(1-i%2);
    dpos += slen;

    /* copy quality values to bam->data */
    if(r->SeqQ && !ajStrMatchC(r->SeqQ, "*"))
    {
	s = ajStrGetPtr(r->SeqQ);

	for(i=0;i<ilen;i++)
	    dpos[i]= s[i]-33;

    }
    else
	for(i=0;i<ilen;i++)
	    dpos[i]= 0xff;



    l = ajListIterNewread(r->Tags);
    bam->l_aux=0;
    while (!ajListIterDone(l))
    {

	tag = ajListIterGet(l);

	/* TODO: array type 'B' and other types */

	if(tag->type == 'i' || tag->type == 'I')
	{
	    tagvalsize = 4;
	    ajStrToInt(tag->Comment, &intval);
	    tagval = (unsigned char*)&intval;
	}
	else if(tag->type =='s' || tag->type =='S')
	{
	    tagvalsize = 2;
	    ajStrToInt(tag->Comment, &intval);
	    tagval = (unsigned char*)&intval;
	}
	else if(tag->type =='c' || tag->type =='C')
	{
	    tagvalsize = 1;
	    ajStrToInt(tag->Comment, &intval);
	    tagval = (unsigned char*)&intval;
	}
	else if(tag->type =='A')
	{
	    tagvalsize = 1;
	    tagval = (const unsigned char*)ajStrGetPtr(tag->Comment);
	}
	else if(tag->type =='Z')
	{
	    tagvalsize = ajStrGetLen(tag->Comment)+1;
	    tagval = (const unsigned char*)ajStrGetPtr(tag->Comment);
	}
	else
	{
	    ajWarn("tag type '%c' not yet supported",tag->type);
	    continue;
	}
	ajSeqBamAuxAppend(bam,
		ajStrGetPtr(tag->Name),
		tag->type,
		tagvalsize,
		tagval);


    }
    ajListIterDel(&l);



    ajSeqBamWrite(gzfile, bam);

    return ajTrue;
}




/* @funcstatic assemoutWriteMaf ***********************************************
**
** Write assembly data in Mira Assembly Format
**
** @param [u] outf [AjPFile] Output file
** @param [r] assem [const AjPAssem] Assembly data object
**
** @return [AjBool] True on success
**
**
** @release 6.5.0
******************************************************************************/

static AjBool assemoutWriteMaf(AjPFile outf, const AjPAssem assem)
{
    AjPAssemContig c = NULL;
    AjPAssemRead   r = NULL;
    AjPAssemTag    t = NULL;
    ajint i = 0;
    ajint nreads = 0;
    AjIList j = NULL;
    AjIList k = NULL;
    AjIList reads = NULL;
    AjPAssemContig* contigs = NULL;

    if(!outf) return ajFalse;
    if(!assem) return ajFalse;

    ajTableToarrayValues(assem->Contigs, (void***)&contigs);
    reads = ajListIterNewread(assem->Reads);

    for (;contigs[i];i++)
    {
	c = contigs[i];
	ajFmtPrintF(outf, "CO\t%S\n", c->Name);
	ajFmtPrintF(outf, "NR\t%d\n", c->Nreads);
	ajFmtPrintF(outf, "LC\t%d\n", c->Length);

	j = ajListIterNewread(c->Tags);
	while (!ajListIterDone(j))
	{
	    t = ajListIterGet(j);
	    ajFmtPrintF(outf, "CT\t%S %u %u %S\n", t->Name, t->x1, t->y1,
		    t->Comment);
	}
	ajListIterDel(&j);

	ajFmtPrintF(outf, "CS\t%S\n", c->Consensus);
	ajFmtPrintF(outf, "CQ\t%S\n", c->ConsensusQ);

	ajFmtPrintF(outf, "\\\\\n");

	while (nreads++ < c->Nreads && !ajListIterDone(reads))
	{
	    r = ajListIterGet(reads);

	    if(r->Reference !=i)
		ajErr("different reference/contig number(%d) than expected(%d)"
			"\nreads were expected to be sorted by contigs");

	    ajFmtPrintF(outf, "RD\t%S\n", r->Name);
	    ajFmtPrintF(outf, "RS\t%S\n", r->Seq);
	    ajFmtPrintF(outf, "RQ\t%S\n", r->SeqQ);
	    ajFmtPrintF(outf, "TN\t%S\n", r->Template);

	    if(r->Direction)
		ajFmtPrintF(outf, "DI\t%c\n", r->Direction);

	    if(r->TemplateSizeMin)
		ajFmtPrintF(outf, "TF\t%d\n", r->TemplateSizeMin);

	    if(r->TemplateSizeMax)
		ajFmtPrintF(outf, "TT\t%d\n", r->TemplateSizeMax);

	    if(r->File)
		ajFmtPrintF(outf, "SF\t%S\n", r->File);

	    if(r->VectorLeft)
		ajFmtPrintF(outf, "SL\t%d\n", r->VectorLeft);

	    if(r->VectorRight)
		ajFmtPrintF(outf, "SR\t%d\n", r->VectorRight);

	    if(r->QualLeft)
		ajFmtPrintF(outf, "QL\t%d\n", r->QualLeft);

	    if(r->QualRight)
		ajFmtPrintF(outf, "QR\t%d\n", r->QualRight);

	    if(r->ClipLeft)
		ajFmtPrintF(outf, "SL\t%d\n", r->ClipLeft);

	    if(r->ClipRight)
		ajFmtPrintF(outf, "SR\t%d\n", r->ClipRight);

	    k = ajListIterNewread(r->Tags);

	    while (!ajListIterDone(k))
	    {
		t = ajListIterGet(k);

		ajFmtPrintF(outf, "RT\t%S %u %u\n", t->Name,
			t->x1, t->y1);
	    }

            ajListIterDel(&k);

	    ajFmtPrintF(outf, "ST\t%S\n", r->Technology);
	    ajFmtPrintF(outf, "ER\n");
	    ajFmtPrintF(outf, "AT\t%u %u %u %u\n", r->x1, r->y1, r->x2, r->y2);
	}

	ajListIterDel(&j);
	ajFmtPrintF(outf, "//\n");
	ajFmtPrintF(outf, "EC\n");
    }

    AJFREE(contigs);
    ajListIterDel(&reads);

    return ajTrue;
}




/* @funcstatic assemoutWriteNextMaf *******************************************
**
** Write latest chunk of assembly data in Mira Assembly Format
**
** @param [u] outfile [AjPOutfile] Output file
** @param [r] assem [const AjPAssem] Assembly data object
**
** @return [AjBool] True on success
**
**
** @release 6.5.0
******************************************************************************/

static AjBool assemoutWriteNextMaf(AjPOutfile outfile, const AjPAssem assem)
{
    AjPFile outf = ajOutfileGetFile(outfile);
    AjPAssemContig c = NULL;
    AjPAssemRead   r = NULL;
    AjPAssemTag    t = NULL;
    ajint i = 0;
    ajint nreads = 0;
    AjIList j = NULL;
    AjIList k = NULL;
    AjIList reads = NULL;
    AjPAssemContig* contigs = NULL;

    if(!outf) return ajFalse;
    if(!assem) return ajFalse;

    if(!assem->Hasdata)
    {
        ajTableToarrayValues(assem->Contigs, (void***)&contigs);
        for (;contigs[i];i++)
        {
            c = contigs[i];
            ajFmtPrintF(outf, "CO\t%S\n", c->Name);
            ajFmtPrintF(outf, "NR\t%d\n", c->Nreads);
            ajFmtPrintF(outf, "LC\t%d\n", c->Length);

            j = ajListIterNewread(c->Tags);
            while (!ajListIterDone(j))
            {
                t = ajListIterGet(j);
                ajFmtPrintF(outf, "CT\t%S %u %u %S\n", t->Name, t->x1, t->y1,
                            t->Comment);
            }
            ajListIterDel(&j);

            ajFmtPrintF(outf, "CS\t%S\n", c->Consensus);
            ajFmtPrintF(outf, "CQ\t%S\n", c->ConsensusQ);

            ajFmtPrintF(outf, "\\\\\n");
        }
        AJFREE(contigs);
        
        return ajTrue;
    }
    
    /* data */

    reads = ajListIterNewread(assem->Reads);

    while (!ajListIterDone(reads))
    {
        nreads++;
        r = ajListIterGet(reads);

        if(r->Reference !=i)
            ajErr("different reference/contig number(%d) than expected(%d)"
                  "\nreads were expected to be sorted by contigs");

        ajFmtPrintF(outf, "RD\t%S\n", r->Name);
        ajFmtPrintF(outf, "RS\t%S\n", r->Seq);
        ajFmtPrintF(outf, "RQ\t%S\n", r->SeqQ);
        ajFmtPrintF(outf, "TN\t%S\n", r->Template);

        if(r->Direction)
            ajFmtPrintF(outf, "DI\t%c\n", r->Direction);

        if(r->TemplateSizeMin)
            ajFmtPrintF(outf, "TF\t%d\n", r->TemplateSizeMin);

        if(r->TemplateSizeMax)
            ajFmtPrintF(outf, "TT\t%d\n", r->TemplateSizeMax);

        if(r->File)
            ajFmtPrintF(outf, "SF\t%S\n", r->File);

        if(r->VectorLeft)
            ajFmtPrintF(outf, "SL\t%d\n", r->VectorLeft);

        if(r->VectorRight)
            ajFmtPrintF(outf, "SR\t%d\n", r->VectorRight);

        if(r->QualLeft)
            ajFmtPrintF(outf, "QL\t%d\n", r->QualLeft);

        if(r->QualRight)
            ajFmtPrintF(outf, "QR\t%d\n", r->QualRight);

        if(r->ClipLeft)
            ajFmtPrintF(outf, "SL\t%d\n", r->ClipLeft);

        if(r->ClipRight)
            ajFmtPrintF(outf, "SR\t%d\n", r->ClipRight);

        k = ajListIterNewread(r->Tags);

        while (!ajListIterDone(k))
        {
            t = ajListIterGet(k);

            ajFmtPrintF(outf, "RT\t%S %u %u\n", t->Name,
			t->x1, t->y1);
        }

        ajListIterDel(&k);

        ajFmtPrintF(outf, "ST\t%S\n", r->Technology);
        ajFmtPrintF(outf, "ER\n");
        ajFmtPrintF(outf, "AT\t%u %u %u %u\n", r->x1, r->y1, r->x2, r->y2);
    }

    ajListIterDel(&j);
    ajListIterDel(&reads);

    if(!nreads)
    {
        ajFmtPrintF(outf, "//\n");
        ajFmtPrintF(outf, "EC\n");
    }
    

    return ajTrue;
}




/* @funcstatic assemoutMakeCigar **********************************************
**
** Given a reference sequence region and a read sequence returns CIGAR string
** representing the alignment.
**
** Based on make_cigar function in Peter Cock's maf2sam project
**
** We have a similar function in ajalign.c, called alignCigar(),
** we should aim using that function rather than having two similar functions
**
** @param [r] contig  [const char*] Reference sequence region
** @param [r] readseq [const char*] Read sequence
**
** @return [AjPStr] CIGAR string
**
**
** @release 6.5.0
******************************************************************************/

static AjPStr assemoutMakeCigar(const char* contig, const char* readseq)
{
    AjPStr cigar = NULL;
    ajint count =0;

    char mode = '\0';
    char c;
    char r;

    c = contig[0];
    r = readseq[0];

    cigar = ajStrNewC("");

    for(;c!=0; c=*++contig, r=*++readseq)
    {

	if(c=='*' && r=='*')
	    continue;
	else if(c!='*' && r!='*')
	{
	    /* alignment match/mismatch */
	    if (mode!='M')
	    {
		if(count)
		    ajFmtPrintAppS(&cigar, "%d%c", count, mode);
		mode = 'M';
		count = 1;
	    }
	    else
		count++;
	}
	else if(c=='*')
	{
	    if (mode!='I')
	    {
		if(count)
		    ajFmtPrintAppS(&cigar, "%d%c", count, mode);
		mode = 'I';
		count = 1;
	    }
	    else
		count++;
	}
	else if(r=='*')
	{
	    if (mode!='D')
	    {
		if(count)
		    ajFmtPrintAppS(&cigar, "%d%c", count, mode);
		mode = 'D';
		count = 1;
	    }
	    else
		count++;
	}
	else
	    ajErr("something wrong!!!");
    }

    if(count)
	ajFmtPrintAppS(&cigar, "%d%c", count, mode);

    return cigar;
}




/* @funcstatic assemoutWriteSam ***********************************************
**
** Write assembly data in SAM format
**
** @param [u] outf [AjPFile] Output file
** @param [r] assem [const AjPAssem] Assembly data object
**
** @return [AjBool] True on success
**
**
** @release 6.5.0
******************************************************************************/

static AjBool assemoutWriteSam(AjPFile outf, const AjPAssem assem)
{
    AjPAssemContig c = NULL;
    AjPAssemRead   r = NULL;
    AjPAssemTag    t = NULL;
    AjPAssemContig* contigs = NULL;
    AjIList j = NULL;
    AjPStr argstr = NULL;
    const AjPStr headertext = NULL;
    ajint n = 0;
    ajint i = 0;
    AjBool ret = ajTrue;

    if(!outf || !assem)
	return ajFalse;

    ajDebug("assemoutWriteSam: # of contigs = %d\n", n);

    ajFmtPrintF(outf, "@HD\tVN:1.3\tSO:%s\n", ajAssemGetSortorderC(assem));

    /* Program record */
    argstr = ajStrNewS(ajUtilGetCmdline());
    ajStrExchangeKK(&argstr, '\n', ' ');
    ajFmtPrintF(outf, "@PG\tID:%S\tVN:%S\tCL:%S\n",
	    ajUtilGetProgram(), ajNamValueVersion(), argstr);
    ajStrDel(&argstr);


    if(ajListGetLength(assem->ContigsOrder))
	ajListToarray(assem->ContigsOrder, (void***)&contigs);
    else
	ajTableToarrayValues(assem->Contigs, (void***)&contigs);

    while (contigs[i])   /* contigs */
    {
	c = contigs[i++];

	if(!ajStrMatchC(c->Name, "*"))
	{
	    ajFmtPrintF(outf, "@SQ\tSN:%S\tLN:%d", c->Name, c->Length);

	    if(c->URI)
		ajFmtPrintF(outf, "\tUR:%S", c->URI);

	    if(c->MD5)
		ajFmtPrintF(outf, "\tM5:%S", c->MD5);

	    if(c->Species)
		ajFmtPrintF(outf, "\tSP:%S", c->Species);

	    ajFmtPrintF(outf, "\n");

	    j = ajListIterNewread(c->Tags);
	    while (!ajListIterDone(j))
	    {
		t = ajListIterGet(j);
		ajFmtPrintF(outf, "@CO\t%S %u %u %S\n", t->Name, t->x1, t->y1,
			t->Comment);
	    }
	    ajListIterDel(&j);
	}
    }

    headertext = assemSAMGetReadgroupHeaderlines(assem);
    if(headertext)
	ajFmtPrintF(outf,"%S", headertext);

    j = ajListIterNewread(assem->Reads);

    while (!ajListIterDone(j))  /* reads */
    {
	r = ajListIterGet(j);
	assemoutWriteSamAlignment(outf, r, contigs, i);
    }

    ajListIterDel(&j);
    AJFREE(contigs);

    return ret;
}




/* @funcstatic assemoutWriteNextSam *******************************************
**
** Write latest chunk of assembly data in SAM format
**
** @param [u] outfile [AjPOutfile] Output file
** @param [r] assem [const AjPAssem] Assembly data object
**
** @return [AjBool] True on success
**
**
** @release 6.5.0
******************************************************************************/

static AjBool assemoutWriteNextSam(AjPOutfile outfile, const AjPAssem assem)
{
    AjPFile outf = ajOutfileGetFile(outfile);
    AjPAssemContig c = NULL;
    AjPAssemRead   r = NULL;
    AjPAssemTag    t = NULL;
    AjPAssemContig* contigs = NULL;
    AjIList j = NULL;
    AjPStr argstr = NULL;
    const AjPStr headertext = NULL;
    ajint n = 0;
    ajulong i = 0UL;
    AjBool ret = ajTrue;

    if(!outf || !assem)
	return ajFalse;

    ajDebug("assemoutWriteSam: # of contigs = %d\n", n);

    if(!assem->Hasdata)
    {
        ajFmtPrintF(outf, "@HD\tVN:1.3\tSO:%s\n", ajAssemGetSortorderC(assem));

        /* Program record */
        argstr = ajStrNewS(ajUtilGetCmdline());
        ajStrExchangeKK(&argstr, '\n', ' ');
        ajFmtPrintF(outf, "@PG\tID:%S\tVN:%S\tCL:%S\n",
                    ajUtilGetProgram(), ajNamValueVersion(), argstr);
        ajStrDel(&argstr);


        if(ajListGetLength(assem->ContigsOrder))
            ajListToarray(assem->ContigsOrder, (void***)&contigs);
        else
            ajTableToarrayValues(assem->Contigs, (void***)&contigs);

        while (contigs[i])   /* contigs */
        {
            c = contigs[i++];

            if(!ajStrMatchC(c->Name, "*"))
            {
                ajFmtPrintF(outf, "@SQ\tSN:%S\tLN:%d", c->Name, c->Length);

                if(c->URI)
                    ajFmtPrintF(outf, "\tUR:%S", c->URI);

                if(c->MD5)
                    ajFmtPrintF(outf, "\tM5:%S", c->MD5);

                if(c->Species)
                    ajFmtPrintF(outf, "\tSP:%S", c->Species);

                ajFmtPrintF(outf, "\n");

                j = ajListIterNewread(c->Tags);
                while (!ajListIterDone(j))
                {
                    t = ajListIterGet(j);
                    ajFmtPrintF(outf, "@CO\t%S %u %u %S\n",
                                t->Name, t->x1, t->y1,
                                t->Comment);
                }
                ajListIterDel(&j);
            }
        }

        headertext = assemSAMGetReadgroupHeaderlines(assem);
        if(headertext)
            ajFmtPrintF(outf,"%S", headertext);

        AJFREE(contigs);

        if(!assem->BamHeader)
            return ajTrue;
    }


    /* data */

    j = ajListIterNewread(assem->Reads);
    if(ajListGetLength(assem->ContigsOrder))
        i = ajListToarray(assem->ContigsOrder, (void***)&contigs);
    else
        i = ajTableToarrayValues(assem->Contigs, (void***)&contigs);

    while (!ajListIterDone(j))  /* reads */
    {
	r = ajListIterGet(j);
	assemoutWriteSamAlignment(outf, r, contigs, (ajuint) i);
    }

    ajListIterDel(&j);
    AJFREE(contigs);

    return ret;
}




/* @funcstatic assemSAMGetReadgroupHeaderlines ********************************
**
** Returns read-group header lines as text for the given assembly
**
** @param [r] assem [const AjPAssem] assembly object
** @return [const AjPStr] read-group header lines
**
**
** @release 6.5.0
******************************************************************************/

static const AjPStr assemSAMGetReadgroupHeaderlines(const AjPAssem assem)
{
    AjPStr* rgids = NULL;
    const AjPAssemReadgroup rg = NULL;
    ajint i =0;

    ajTableToarrayKeys(assem->Readgroups, (void***)&rgids);

    ajStrAssignC(&assemoutSamLinetxt, "");

    while (rgids[i])  /* read groups */
    {
	rg = ajTableFetchS(assem->Readgroups, rgids[i++]);
	ajFmtPrintAppS(&assemoutSamLinetxt, "@RG\tID:%S", rg->ID);

	if(rg->CN)
	    ajFmtPrintAppS(&assemoutSamLinetxt, "\tCN:%S", rg->CN);

	if(rg->Desc)
	    ajFmtPrintAppS(&assemoutSamLinetxt, "\tDS:%S", rg->Desc);

	if(rg->Date)
	    ajFmtPrintAppS(&assemoutSamLinetxt, "\tDT:%S", rg->Date);

	if(rg->FlowOrder)
	    ajFmtPrintAppS(&assemoutSamLinetxt, "\tFO:%S", rg->FlowOrder);

	if(rg->KeySeq)
	    ajFmtPrintAppS(&assemoutSamLinetxt, "\tKS:%S", rg->KeySeq);

	if(rg->Library)
	    ajFmtPrintAppS(&assemoutSamLinetxt, "\tLB:%S", rg->Library);

	if(rg->Programs)
	    ajFmtPrintAppS(&assemoutSamLinetxt, "\tPG:%S", rg->Programs);

	if(rg->Isize)
	    ajFmtPrintAppS(&assemoutSamLinetxt, "\tPI:%d", rg->Isize);

	if(rg->Platform)
	    ajFmtPrintAppS(&assemoutSamLinetxt, "\tPL:%s",
	                   ajAssemreadgroupGetPlatformname(rg));

	if(rg->Unit)
	    ajFmtPrintAppS(&assemoutSamLinetxt, "\tPU:%S", rg->Unit);

	if(rg->Sample)
	    ajFmtPrintAppS(&assemoutSamLinetxt, "\tSM:%S", rg->Sample);

	ajStrAppendC(&assemoutSamLinetxt, "\n");

    }

    AJFREE(rgids);

    return assemoutSamLinetxt;
}




/* @funcstatic assemoutWriteSamAlignment **************************************
**
** Write individual alignment records of assembly data in SAM format
**
** @param [u] outf [AjPFile] Output file
** @param [r] r [const AjPAssemRead] Read object with alignment information
** @param [r] contigs [AjPAssemContig const*] Contigs/refseqs the read
**                                             aligns to
** @param [r] ncontigs [ajint] Number of contigs
**
** @return [AjBool] True on success
**
**
** @release 6.5.0
******************************************************************************/

static AjBool assemoutWriteSamAlignment(AjPFile outf, const AjPAssemRead r,
					AjPAssemContig const * contigs,
					ajint ncontigs)
{
    AjPAssemTag    t = NULL;
    AjIList l = NULL;
    AjPStr qualstr = NULL;
    AjPStr tmp  = NULL;
    ajint  POS  = 0;
    AjPStr CIGAR = NULL;
    const char* RNEXT = NULL;
    AjPStr SEQ  = NULL;
    AjPStr QUAL = NULL;
    AjPStr SEQunpadded  = NULL;
    AjPStr QUALunpadded = NULL;
    AjPStr consensus = NULL;
    AjBool rc= ajFalse;
    AjBool ret = ajTrue;
    const char* refseq = NULL;
    const AjPAssemContig contig = NULL;

    ajuint k = 0;

    if(r->Reference>=ncontigs)
	ajDie("assemoutWriteSamAlignment: reference sequence number"
		" '%d' is larger than or equal to known number of reference"
		" sequences '%d'. Problem while processing read '%S'.",
		r->Reference,
		ncontigs,
		r->Name);

    contig = (r->Reference==-1 ? NULL : contigs[r->Reference]);

    ajStrAssignRef(&SEQ, r->Seq);
    consensus = contig==NULL? NULL : contig->Consensus;

    if (r->Rnext==-1)
	RNEXT= "*";
    else if(r->Rnext==r->Reference)
	RNEXT = "=";
    else
	RNEXT = ajStrGetPtr(contigs[r->Rnext]->Name);

    if (r->Flag & BAM_FREVERSE)
    {
	rc = ajTrue;
	qualstr = ajStrNewS(r->SeqQ);

	if(!r->Reversed)
	{
	    ajStrReverse(&qualstr);
	    ajSeqstrReverse(&SEQ);
	}

	QUAL = qualstr;
	POS = r->y1;
	ajStrAssignSubS(&tmp, SEQ,
		ajStrGetLen(r->Seq) - r->y2,
		ajStrGetLen(r->Seq) - r->x2
	);

    }
    else
    {
	rc= ajFalse;
	POS = r->x1;
	QUAL = r->SeqQ;
	ajStrAssignSubS(&tmp, SEQ,
		r->x2-1,
		r->y2-1
	);
    }

    if(r->Cigar==NULL && consensus)
    {
	refseq = ajStrGetPtr(consensus) + (rc ? r->y1-1 : r->x1-1);

	CIGAR = assemoutMakeCigar(refseq, ajStrGetPtr(tmp));

	SEQunpadded = ajStrNewRes(ajStrGetLen(SEQ));
	QUALunpadded = ajStrNewRes(ajStrGetLen(SEQ));

	for(k=0; k< ajStrGetLen(SEQ); k++)
	{
	    if (ajStrGetCharPos(SEQ, k) == '*')
		continue;

	    ajStrAppendK(&SEQunpadded, ajStrGetCharPos(SEQ, k));
	    ajStrAppendK(&QUALunpadded, ajStrGetCharPos(QUAL, k));
	}

	ajDebug("cigar: %S\n", CIGAR);

	ajStrAssignS(&tmp, CIGAR);

	if(rc)
	{
	    if(r->y2 < (ajint)ajStrGetLen(SEQ))
		ajFmtPrintS(&CIGAR, "%dS%S",
		            ajStrGetLen(SEQ) - r->y2, tmp);
	    if(r->x2 > 1)
		ajFmtPrintAppS(&CIGAR, "%dS", r->x2 - 1);
	}
	else
	{
	    if(r->x2 > 1)
		ajFmtPrintS(&CIGAR, "%dS%S", r->x2 - 1, tmp);
	    if(r->y2 < (ajint)ajStrGetLen(SEQ))
		ajFmtPrintAppS(&CIGAR, "%dS",
		               ajStrGetLen(SEQ) - r->y2);
	}
	ajStrDel(&tmp);
    }
    else if(r->Cigar==NULL)
    {
	ajErr("both CIGAR string and consensus sequence not available");
	ret = ajFalse;
	ajStrAssignK(&CIGAR, '*');
    }
    else if(!ajStrGetLen(r->Cigar))
	ajStrAssignK(&CIGAR, '*');
    else if(ajStrGetLen(r->Cigar))
    {
	if(!ajStrGetLen(SEQ))
	    ajStrAssignK(&SEQ, '*');

	if(!ajStrGetLen(QUAL))
	    ajStrAssignK(&QUAL, '*');
    }

    ajStrDel(&tmp);

    ajFmtPrintF(outf, "%S\t%d\t%s\t%d\t%d\t%S\t%s\t%Ld\t%d\t%S\t%S",
	    r->Name,
	    r->Flag,
	    (contig==NULL ? "*" : ajStrGetPtr(contig->Name)),
	    POS,
	    r->MapQ,
	    (CIGAR ? CIGAR : r->Cigar),
	    RNEXT,
	    r->Pnext,
	    r->Tlen,
	    (r->Cigar ? SEQ  : SEQunpadded),
	    (r->Cigar ? QUAL : QUALunpadded));

    l = ajListIterNewread(r->Tags);
    while (!ajListIterDone(l))
    {
	t = ajListIterGet(l);

	/* TODO: array type, 'B' */

	/* In SAM, all single integer types are mapped to int32_t [SAM spec] */
	ajFmtPrintF(outf, "\t%S:%c:",
		t->Name,
		(t->type == 'c' || t->type == 'C' ||
		 t->type == 's' || t->type == 'S'
				|| t->type == 'I') ? 'i' : t->type
	);

	if(t->x1 || t->y1)
	    ajFmtPrintF(outf, " %u %u", t->x1, t->y1);

	if(t->Comment && ajStrGetLen(t->Comment)>0)
	    ajFmtPrintF(outf, "%S", t->Comment);

    }
    ajListIterDel(&l);

    ajFmtPrintF(outf, "\n");

    if(qualstr)
	ajStrDel(&qualstr);

    ajStrDel(&SEQ);
    ajStrDel(&CIGAR);
    ajStrDel(&SEQunpadded);
    ajStrDel(&QUALunpadded);

    return ret;
}




/* @funcstatic assemoutWriteList **********************************************
**
** Write an assembly as the simple id
**
** @param [u] outf [AjPFile] Output file
** @param [r] assem [const AjPAssem] Assembly object
**
** @return [AjBool] True on success
**
**
** @release 6.4.0
******************************************************************************/

static AjBool assemoutWriteList(AjPFile outf, const AjPAssem assem)
{
    if(!outf) return ajFalse;
    if(!assem) return ajFalse;

    if(ajStrGetLen(assem->Db))
        ajFmtPrintF(outf, "%S:%S\n", assem->Db, assem->Id);
    else
        ajFmtPrintF(outf, "%S\n", assem->Id);
    
    return ajTrue;
}




/* @funcstatic assemoutWriteNextList ******************************************
**
** Write an assembly as the simple id
**
** @param [u] outfile [AjPOutfile] Output file
** @param [r] assem [const AjPAssem] Assembly object
**
** @return [AjBool] True on success
**
**
** @release 6.4.0
******************************************************************************/

static AjBool assemoutWriteNextList(AjPOutfile outfile, const AjPAssem assem)
{
    AjPFile outf = ajOutfileGetFile(outfile);

    if(!outf) return ajFalse;
    if(!assem) return ajFalse;

    if(!assem->Hasdata)
        return ajTrue;

    if(ajStrGetLen(assem->Db))
        ajFmtPrintF(outf, "%S:%S\n", assem->Db, assem->Id);
    else
        ajFmtPrintF(outf, "%S\n", assem->Id);
    
    return ajTrue;
}




/* @datasection [none] Miscellaneous functions ********************************
**
** Functions to initialise and clean up internals
**
** @nam2rule Assemout Assembly output internals
**
******************************************************************************/




/* @section Printing **********************************************************
**
** Printing details of the internals to a file
**
** @fdata [none]
**
** @nam2rule Assemoutprint
**
** @fcategory output
**
******************************************************************************/




/* @section Print *************************************************************
**
** Printing to a file
**
** @fdata [none]
**
** @nam3rule Book Print as docbook table
** @nam3rule Html Print as html table
** @nam3rule Wiki Print as wiki table
** @nam3rule Text Print as text
**
** @argrule * outf [AjPFile] output file
** @argrule Text full [AjBool] Print all details
**
** @valrule * [void]
**
** @fcategory cast
**
******************************************************************************/




/* @func ajAssemoutprintBook **************************************************
**
** Reports the assembly format internals as Docbook text
**
** @param [u] outf [AjPFile] Output file
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajAssemoutprintBook(AjPFile outf)
{
    ajint i = 0;

    ajFmtPrintF(outf, "<para>The supported assembly output "
                "formats are summarised "
                "in the table below. The columns are as follows: "
                "<emphasis>Output format</emphasis> (format name), "
                 "<emphasis>Description</emphasis> (short description of "
                "the format).</para> \n\n");

    ajFmtPrintF(outf, "<table frame=\"box\" rules=\"cols\">\n");
    ajFmtPrintF(outf, "  <caption>Assembly output formats</caption>\n");
    ajFmtPrintF(outf, "  <thead>\n");
    ajFmtPrintF(outf, "    <tr align=\"center\">\n");
    ajFmtPrintF(outf, "      <th>Output Format</th>\n");
    ajFmtPrintF(outf, "      <th>Description</th>\n");
    ajFmtPrintF(outf, "    </tr>\n");
    ajFmtPrintF(outf, "  </thead>\n");
    ajFmtPrintF(outf, "  <tbody>\n");

    for(i=0; assemoutFormatDef[i].Name; i++)
    {
        ajFmtPrintF(outf, "    <tr>\n");
        ajFmtPrintF(outf, "      <td>%s</td>\n",
                    assemoutFormatDef[i].Name);
        ajFmtPrintF(outf, "      <td>%s</td>\n",
                    assemoutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "  </tbody>\n");
    ajFmtPrintF(outf, "</table>\n");

    return;
}




/* @func ajAssemoutprintHtml **************************************************
**
** Reports the internal data structures
**
** @param [u] outf [AjPFile] Output file
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajAssemoutprintHtml(AjPFile outf)
{
    ajint i = 0;

    ajFmtPrintF(outf, "<table border=3>");
    ajFmtPrintF(outf, "<tr><th>Assembly Format</th>\n");
    ajFmtPrintF(outf, "<th>Description</th></tr>\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# Assembly output formats\n");
    ajFmtPrintF(outf, "# Name    Format name (or alias)\n");
    ajFmtPrintF(outf, "# Desc    Format description\n");
    ajFmtPrintF(outf, "# Name         Description\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "AssemFormat {\n");

    for(i=0; assemoutFormatDef[i].Name; i++)
    {
        ajFmtPrintF(outf, "<tr><td>\n%-12s\n"
                        "<td>\"%s\"</td></tr>\n",
			assemoutFormatDef[i].Name,
			assemoutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajAssemoutprintText **************************************************
**
** Reports the internal data structures
**
** @param [u] outf [AjPFile] Output file
** @param [r] full [AjBool] Full report (usually ajFalse)
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajAssemoutprintText(AjPFile outf, AjBool full)
{
    ajint i = 0;

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# Assembly output formats\n");
    ajFmtPrintF(outf, "# Name    Format name (or alias)\n");
    ajFmtPrintF(outf, "# Desc    Format description\n");
    ajFmtPrintF(outf, "# Name         Description\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "AssemFormat {\n");

    for(i=0; assemoutFormatDef[i].Name; i++)
    {
	if(full)
	    ajFmtPrintF(outf, "  %-12s \"%s\"\n",
			assemoutFormatDef[i].Name,
			assemoutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajAssemoutprintWiki **************************************************
**
** Reports the asembly output format internals as wikitext
**
** @param [u] outf [AjPFile] Output file
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajAssemoutprintWiki(AjPFile outf)
{
    ajint i = 0;
    ajint j = 0;
    AjPStr namestr = NULL;

    ajFmtPrintF(outf, "{| class=\"wikitable sortable\" border=\"2\"\n");
    ajFmtPrintF(outf, "|-\n");
    ajFmtPrintF(outf, "!Format!!"
                "class=\"unsortable\"|Description\n");

    for(i=0; assemoutFormatDef[i].Name; i++)
    {
        ajFmtPrintF(outf, "|-\n");
        ajStrAssignC(&namestr, assemoutFormatDef[i].Name);

        for(j=i+1; assemoutFormatDef[j].Name; j++)
        {
            if(assemoutFormatDef[j].Write == assemoutFormatDef[i].Write)
            {
                ajFmtPrintAppS(&namestr, "<br>%s",
                               assemoutFormatDef[j].Name);
                ajWarn("Assembly output format '%s' same as '%s' "
                       "but not alias",
                       assemoutFormatDef[j].Name,
                       assemoutFormatDef[i].Name);
            }
        }
        ajFmtPrintF(outf, "|%S||%s\n",
                    namestr,
                    assemoutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "|}\n");

    ajStrDel(&namestr);

    return;
}




/* @datasection [none] Output formats *****************************************
**
** Output formats internals
**
** @nam2rule Assemoutformat Assembly output format specific
**
******************************************************************************/




/* @section cast **************************************************************
**
** Values for output formats
**
** @fdata [none]
**
** @nam3rule Find Return index to named format
** @nam3rule Test Test format value
**
** @argrule * format [const AjPStr] Format name
** @argrule Find iformat [ajint*] Index matching format name
**
** @valrule * [AjBool] True if found
**
** @fcategory cast
**
******************************************************************************/




/* @func ajAssemoutformatFind *************************************************
**
** Looks for the specified format(s) in the internal definitions and
** returns the index.
**
** Sets iformat as the recognised format, and returns ajTrue.
**
** @param [r] format [const AjPStr] Format required.
** @param [w] iformat [ajint*] Index
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajAssemoutformatFind(const AjPStr format, ajint* iformat)
{
    AjPStr tmpformat = NULL;
    ajuint i = 0;

    /* ajDebug("ajAssemoutformatFind '%S'\n", format); */
    if(!ajStrGetLen(format))
	return ajFalse;

    ajStrAssignS(&tmpformat, format);
    ajStrFmtLower(&tmpformat);

    for(i=0; assemoutFormatDef[i].Name; i++)
    {
	/* ajDebug("test %d '%s' \n", i, assemoutFormatDef[i].Name); */
	if(ajStrMatchCaseC(tmpformat, assemoutFormatDef[i].Name))
	{
	    *iformat = i;
	    ajStrDel(&tmpformat);
	    /* ajDebug("found '%s' at %d\n", assemoutFormatDef[i].Name, i); */
	    return ajTrue;
	}
    }

    ajErr("Unknown output format '%S'", format);

    ajStrDel(&tmpformat);

    *iformat = 0;

    return ajFalse;
}




/* @func ajAssemoutformatTest *************************************************
**
** Tests whether a named assembly output format is known
**
** @param [r] format [const AjPStr] Format
** @return [AjBool] ajTrue if formats was accepted
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajAssemoutformatTest(const AjPStr format)
{
    ajint i;

    for (i=0; assemoutFormatDef[i].Name; i++)
	if(ajStrMatchCaseC(format, assemoutFormatDef[i].Name))
	    return ajTrue;

    return ajFalse;
}




/* @datasection [none] Miscellaneous functions ********************************
**
** Functions to initialise and clean up internals
**
** @nam2rule Assemout Assembly output internals
**
******************************************************************************/




/* @section Miscellaneous *****************************************************
**
** Functions to initialise and clean up internals
**
** @fdata [none]
**
** @nam3rule Exit Clean up and exit
**
** @valrule * [void]
**
** @fcategory misc
**
******************************************************************************/




/* @func ajAssemoutExit *******************************************************
**
** Cleans up assembly output internal memory
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajAssemoutExit(void)
{
    ajStrDel(&assemoutSamLinetxt);

    return;
}

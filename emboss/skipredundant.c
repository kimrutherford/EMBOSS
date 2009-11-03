/* @source noredundant application
**
** Remove redundant sequences from an input set
** @author Copyright (C) Jon Ison (jison@ebi.ac.uk)
** @@
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
******************************************************************************/

#include "emboss.h"


static AjBool skipredundant_ClearList (AjPList list);
static AjBool skipredundant_SeqsetToList (AjPList list, AjPSeqset seqset);


/* @prog skipredundant *********************************************************
**
** Remove redundant sequences from an input set
**
******************************************************************************/

int main(int argc, char **argv)
{
    /* Variable Declarations */
    AjPSeqset  seqset    = NULL;
    AjPMatrixf fmat      = NULL;
    float      thresh;
    float      threshlow;
    float      threshup;
    float      gapopen;
    float      gapextend;
    AjPSeqout  seqout    = NULL;
    AjPSeqout  seqoutred = NULL;
    AjPStr     mode      = NULL;
    ajint      moden;
    ajuint i;


    /* toggle "feature" from ACD not retrieved ... no need */

    const AjPSeq seq    = NULL;
    AjPList      list   = NULL;    /* List for redundancy removal.       */
    AjPUint      keep   = NULL;    /* 1: Sequence in list was non-redundant,
                                      0: redundant.    */
    ajuint       nseq   = 0;       /* No. seqs. in list.                 */
    ajint        nseqnr = 0;       /* No. non-redundant seqs. in list.   */

    /* ACD File Processing */
    embInit("skipredundant", argc, argv);
    seqset        = ajAcdGetSeqset("sequences");
    mode          = ajAcdGetListSingle("mode");
    fmat          = ajAcdGetMatrixf("datafile");
    thresh        = ajAcdGetFloat("threshold");
    threshlow     = ajAcdGetFloat("minthreshold");
    threshup      = ajAcdGetFloat("maxthreshold");
    gapopen       = ajAcdGetFloat("gapopen");
    gapextend     = ajAcdGetFloat("gapextend");
    seqout        = ajAcdGetSeqoutall("outseq");
    seqoutred     = ajAcdGetSeqoutall("redundantoutseq");



    /* Application logic */
    list    = ajListNew();
    skipredundant_SeqsetToList(list, seqset);
    keep = ajUintNew();  
    ajStrToInt(mode, &moden);


    if(moden == 1) 
      /* Remove redundancy at a single threshold % sequence similarity */
      {
	if((!embDmxSeqNR(list, &keep, &nseqnr, fmat, gapopen, 
			 gapextend, thresh, ajFalse)))
	  ajFatal("embDmxSeqNR unexpected failure!");
      }
    else if (moden == 2)
      /* 2: Remove redundancy outside a range of acceptable threshold % similarity */
      {
	if((!embDmxSeqNRRange(list, &keep, &nseqnr, fmat, gapopen, 
			      gapextend, threshlow, threshup, ajFalse)))
	  ajFatal("embDmxSeqNRRange unexpected failure!");
      }
    else 
      ajFatal("Invalid mode (not 1 or 2) which should never occur (check ACD file!)");

    nseq = ajSeqsetGetSize(seqset);
    for(i=0; i<nseq; i++)
      {
	seq = ajSeqsetGetseqSeq(seqset, i);

	if(ajUintGet(keep, i))
	  ajSeqoutWriteSeq(seqout, seq);
	else if(seqoutred)
	  ajSeqoutWriteSeq(seqoutred, seq);
      }

    /* Memory management and exit */
    ajSeqsetDel(&seqset);
    ajMatrixfDel(&fmat);
    ajStrDel(&mode);
    ajSeqoutClose(seqout);
    ajSeqoutDel(&seqout);
    if(seqoutred)
    {
	ajSeqoutClose(seqoutred);
	ajSeqoutDel(&seqoutred);
    }
    skipredundant_ClearList(list);

    ajListFree(&list);
    ajUintDel(&keep);

    embExit();

    return 0;
}






/* @funcstatic skipredundant_SeqsetToList **************************************
**
** Builds a list of sequences from a sequence set.
** The sequences are NOT copied (only a reference is pushed onto the list)
**
** @param [u] list   [AjPList] List 
** @param [w] seqset [AjPSeqset] Sequence set
** @return [AjBool] True on success
******************************************************************************/
static AjBool skipredundant_SeqsetToList (AjPList list, AjPSeqset seqset)
{
    ajint     n      = 0;
    ajint     x      = 0;
    EmbPDmxNrseq seq_tmp = NULL;    /* Temp. pointer for making seq_list.    */
  
    if(!list || !seqset)
      return ajFalse;
    
    n = ajSeqsetGetSize(seqset);
    for(x=0; x<n; x++)
    {
        seq_tmp = embDmxNrseqNew(ajSeqsetGetseqSeq(seqset, x));
	ajListPushAppend(list, seq_tmp);
        seq_tmp = NULL;
    }

    return ajTrue;
}





/* @funcstatic skipredundant_ClearList **************************************
**
** Clears a list of sequences from a sequence set.
** The sequences are copied
**
** @param [u] list   [AjPList] List 
** @return [AjBool] True on success
******************************************************************************/
static AjBool skipredundant_ClearList (AjPList list)
{
    EmbPDmxNrseq seq_tmp = NULL;
    AjIList iter;

    if(!list)
      return ajFalse;
    
    iter = ajListIterNew(list);
    while(!ajListIterDone(iter))
    {
        seq_tmp = (EmbPDmxNrseq)ajListIterGet(iter);
        embDmxNrseqDel(&seq_tmp);
    }
    ajListIterDel(&iter);
    
    return ajTrue;
}

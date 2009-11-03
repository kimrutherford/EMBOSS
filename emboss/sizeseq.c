/* @source sizeseq application
**
** Filter input sequences on basis of size
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


static ajint sizeseq_SeqCompBigFirst(const void *seq1, const void *seq2);
static ajint sizeseq_SeqCompSmallFirst(const void *seq1, const void *seq2);
static AjBool  sizeseq_SeqsetToList (AjPList list, AjPSeqset seqset);


/* @prog sizeseq **************************************************************
**
** Filter input sequences on basis of size
**
******************************************************************************/

int main(int argc, char **argv)
{
    /* Variable Declarations */
    AjPSeqset seqset  = NULL;
    AjPSeqout seqout  = NULL;
    AjBool    bigfirst;

    AjPList   list   = NULL;   
    AjPSeq    seq    = NULL;

    /* ACD File Processing */
    embInit("sizeseq", argc, argv);
    seqset      = ajAcdGetSeqset("sequences");
    bigfirst    = ajAcdGetBoolean("descending");
    seqout      = ajAcdGetSeqoutall("outseq");

    /* Application logic */
    list    = ajListNew();
    sizeseq_SeqsetToList(list, seqset);
    if(bigfirst)
      ajListSort(list, sizeseq_SeqCompBigFirst);
    else
      ajListSort(list, sizeseq_SeqCompSmallFirst);

    while(ajListPop(list,(void **)&seq))
      ajSeqoutWriteSeq(seqout, seq);


    /* Memory management and exit */
    ajSeqsetDel(&seqset);
    ajSeqoutClose(seqout);
    ajSeqoutDel(&seqout);

    ajListFree(&list);

    embExit();

    return 0;
}




/* Same as noredundant_SeqsetToList and should be library function */

/* @funcstatic sizeseq_SeqsetToList ********************************************
**
** Builds a list of sequences from a sequence set.
** The sequences are NOT copied (only a reference is pushed onto the list)
**
** @param [u] list   [AjPList] List 
** @param [w] seqset [AjPSeqset] Sequence set
** @return [AjBool] True on success
******************************************************************************/
static AjBool  sizeseq_SeqsetToList (AjPList list, AjPSeqset seqset)
{
    ajint     n      = 0;
    ajint     x      = 0;
    AjPSeq    *seqs  = NULL;
  
    if(!list || !seqset)
      return ajFalse;

    seqs = ajSeqsetGetSeqarray(seqset);
    n = ajSeqsetGetSize(seqset);
    for(x=0; x<n; x++)
    {
	ajListPushAppend(list, seqs[x]);
    } 
    AJFREE(seqs);
    return ajTrue;
}




/* @funcstatic sizeseq_SeqCompSmallFirst **************************************
**
** Function to sort Seq objects by length. Usually called by ajListSort.  
**
** @param [r] seq1  [const void*] Pointer to Seq object 1
** @param [r] seq2  [const void*] Pointer to Seq object 2
**
** @return [ajint] -1 if len1<len2, 0 if len1==len2, else 1.
** @@
****************************************************************************/

static ajint sizeseq_SeqCompSmallFirst(const void *seq1, const void *seq2)
{
    const AjPSeq q1 = NULL;
    const AjPSeq q2 = NULL;
    const AjPStr s1 = NULL;
    const AjPStr s2 = NULL;

    q1 = (*(AjPSeq const *)seq1);
    q2 = (*(AjPSeq const *)seq2);
    
    s1 = (AjPStr) q1->Seq;
    s2 = (AjPStr) q2->Seq;

    if(s1->Len < s2->Len)
      return -1;
    else if (s1->Len == s2->Len)
      return 0;
    else
      return 1;
}





/* @funcstatic sizeseq_SeqCompBigFirst ****************************************
**
** Function to sort Seq objects by length. Usually called by ajListSort.  
**
** @param [r] seq1  [const void*] Pointer to Seq object 1
** @param [r] seq2  [const void*] Pointer to Seq object 2
**
** @return [ajint] 1 if len1<len2, 0 if len1==len2, else -1.
** @@
****************************************************************************/

static ajint sizeseq_SeqCompBigFirst(const void *seq1, const void *seq2)
{
    const AjPSeq q1 = NULL;
    const AjPSeq q2 = NULL;
    const AjPStr s1 = NULL;
    const AjPStr s2 = NULL;

    q1 = (*(AjPSeq const *)seq1);
    q2 = (*(AjPSeq const *)seq2);
    
    s1 = (AjPStr) q1->Seq;
    s2 = (AjPStr) q2->Seq;

    if(s1->Len < s2->Len)
      return 1;
    else if (s1->Len == s2->Len)
      return 0;
    else
      return -1;
}

/******************************************************************************
** @source makeseq.c application
**
** Create random sequence.
**
** @author Copyright (C) 2004 Henrikki Almusa, Medicel Oy
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


static AjPStr makenucseq_random_sequence(AjPStr const* seqchar,
                                         ajint scmax, ajint length);
static void makenucseq_default_chars(AjPList* list);




/* @prog makenucseq ***********************************************************
**
** Creates a random nucleotide sequence. Can use cusp codon distribution to
** specify sequence composition.
**
******************************************************************************/
int main(int argc, char **argv)
{
    AjPSeqout outseq = NULL;
    AjPList list     = NULL;
    AjPSeq seq       = NULL;
    AjPStr insert    = NULL;
    AjPStr seqstr    = NULL;
    AjPStr* seqr     = NULL;
    AjPCod codondata = NULL;
    ajint start   = 0;
    ajint length  = 0;
    ajint amount  = 0;
    ajint scmax   = 0;
    ajint extra   = 0;

    embInit("makenucseq", argc, argv);

    codondata= ajAcdGetCodon("codonfile");
    insert   = ajAcdGetString("insert");
    start    = ajAcdGetInt("start");
    length   = ajAcdGetInt("length");
    amount   = ajAcdGetInt("amount");
    outseq   = ajAcdGetSeqoutall("outseq");

    list = ajListstrNew();

    /* this is checked by acd
    if(amount <=0 || length <= 0)
	ajFatal("Amount or length is 0 or less. "
	         "Unable to create any sequences"); */

    /* if insert, make sure sequence is large enough */
    if(ajStrGetLen(insert))
    {
	length -= ajStrGetLen(insert);
	/* start= start <= 1 ? 0 : --start; */ /* checked in acd */
	start--;
	if(length <= 0)
	    ajFatal("Sequence smaller than inserted part. "
		     "Unable to create sequences.");
    }

    /* make the list of AjPStr to be used in sequence creation */
    if(codondata)
    {
	ajCodGetCodonlist(codondata, list);
	    /* length is length in nucleotides, random sequence is
               made of triplets */
	extra = length % 3;
	length /= 3;
	if(extra)
	{
	    length++;
	    extra = 0 - (3 - extra);
	}
    }
    else
	makenucseq_default_chars(&list);

    
    /* if insert, make sure type is correct */
    /* typechecking code is not working, uncomment and test after it is
    if(ajStrGetLen(insert))
    {
	seqstr = ajStrNew();
	ajStrAssignC(&seqstr,"purenucleotide");
	if(!ajSeqTypeCheckS(&insert,seqstr))
	    ajFatal("Insert not the same sequence type as sequence itself.");
	ajStrDel(&seqstr);
    } */

    /* array allows fast creation of a sequences */
    scmax = ajListstrToarray(list,&seqr);

    if(!scmax)
	ajFatal("No strings in list. No characters to make the sequence.");

    ajRandomSeed();

    while(amount-- > 0)
    {
	seqstr = makenucseq_random_sequence(seqr,scmax,length);
	if(ajStrGetLen(insert))
	    ajStrInsertS(&seqstr,start,insert);
	ajStrFmtLower(&seqstr);
	seq = ajSeqNew();

	ajStrExchangeSetCC(&seqstr,"u","t");

	if(extra < 0)
	    ajStrCutEnd(&seqstr,-extra);

	ajSeqAssignSeqS(seq, seqstr);
	ajSeqSetNuc(seq);

	ajSeqoutWriteSeq(outseq, seq);
	ajSeqDel(&seq);
	ajStrDel(&seqstr);
    }

    ajSeqoutClose(outseq);
    ajSeqoutDel(&outseq);
    ajListstrFreeData(&list);
    ajStrDel(&insert);
 
    ajCodDel(&codondata);
    AJFREE(seqr);

    embExit();

    return 0;
}




/* @funcstatic makenucseq_random_sequence *************************************
**
** Creates string containing random sequence from given character distribution.
**
** @param [r] seqchar [AjPStr const*] Characters use to make sequence from
** @param [r] scmax [ajint] lenght of the seqchar string
** @param [r] length [ajint] Length of the wanted sequence
** @return [AjPStr] Sequence string
** @@
******************************************************************************/
static AjPStr makenucseq_random_sequence(AjPStr const* seqchar,
                                         ajint scmax, ajint length)
{
    AjPStr seq = ajStrNew();
    ajint idx  = 0;
    ajint len  = length;

    while(len-- > 0)
    {
	idx = (ajint) (ajRandomDouble()*scmax);
	ajStrAppendS(&seq,seqchar[idx]);
    }

    return seq;
}




/* @funcstatic makenucseq_default_chars ***************************************
**
** Crates equal distribution of characters for completely random sequences.
**
** @param [w] list [AjPList*] List with character distributions
** @return [void]
** @@
******************************************************************************/
static void makenucseq_default_chars(AjPList* list)
{
    int i;
    int max;
    char *chars;
    char seqCharNucPure[]   = "ACGTacgt";
    int  seqCharNucPureLength = 8;
    AjPStr tmp;

    chars = seqCharNucPure;
    max = seqCharNucPureLength;

    for(i = 0; i < max; i++)
    {
	tmp = ajStrNew();
	tmp = ajFmtStr("%c",chars[i]);
	ajListstrPushAppend(*list,tmp);
    }

    return;
}

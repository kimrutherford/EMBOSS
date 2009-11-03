/* @source listor application
**
** Writes a list file of the logical OR of two sets of sequences
**
** @author Copyright (C) Gary Williams
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




static void listor_Output(AjPFile list, ajint Operator,
			  const AjPSeqset seq1, const AjPSeqset seq2,
			  const ajint *hits1, const ajint *hits2,
			  ajint n1, ajint n2);

static void listor_Write(AjPFile list, const AjPSeqset seqset, ajint i);




enum {L_OR, L_AND, L_XOR, L_NOT};




/* @prog listor ***************************************************************
**
** Writes a list file of the logical OR of two sets of sequences
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqset seq1;
    AjPSeqset seq2;
    AjPFile list;
    ajint n1;
    ajint n2;
    ajint *lengths1;
    ajint *lengths2;
    ajuint *order1;
    ajuint *order2;
    ajint *hits1;
    ajint *hits2;
    ajint curr1;
    ajint curr2;
    ajint tmp1;
    ajint tmp2 = 0;
    ajint i;
    AjPStr operator;
    ajint OperatorCode=0;


    embInit("listor", argc, argv);

    seq1     = ajAcdGetSeqset("firstsequences");
    seq2     = ajAcdGetSeqset("secondsequences");
    list     = ajAcdGetOutfile("outfile");
    operator = ajAcdGetListSingle("operator");

    /* get the operator value */
    switch(ajStrGetCharFirst(operator))
    {
    case 'O':
	OperatorCode = L_OR;
	break;
    case 'A':
	OperatorCode = L_AND;
	break;
    case 'X':
	OperatorCode = L_XOR;
	break;
    case 'N':
	OperatorCode = L_NOT;
	break;
    default:
	ajFatal("Invalid operator type: %S", operator);
	embExitBad();
    }


    /* get the order of seqset 1 by length */
    n1 = ajSeqsetGetSize(seq1);

    /* lengths of seq1 entries */
    lengths1 = AJCALLOC0(n1, sizeof(ajint));

    /* seq1 entries which match seq2 */
    hits1    = AJCALLOC0(n1, sizeof(ajint));

    /* seq1 entries in length order */
    order1   = AJCALLOC0(n1, sizeof(ajint));
    for(i=0; i<n1; i++)
    {
	lengths1[i] = ajSeqGetLen(ajSeqsetGetseqSeq(seq1, i));
	order1[i]   = i;
	hits1[i]    = -1;
    }
    ajSortIntIncI(lengths1, order1, n1);

    /* get the order of seqset 2 by length */
    n2 = ajSeqsetGetSize(seq2);
    lengths2 = AJCALLOC0(n2, sizeof(ajint));
    hits2    = AJCALLOC0(n2, sizeof(ajint));
    order2   = AJCALLOC0(n2, sizeof(ajint));

    for(i=0; i<n2; i++)
    {
	lengths2[i] = ajSeqGetLen(ajSeqsetGetseqSeq(seq2, i));
	order2[i]   = i;
	hits2[i]    = -1;
    }
    ajSortIntIncI(lengths2, order2, n2);

    /*
    ** go down the two sequence sets, by size order, looking for identical
    **lengths
    */
    curr1 = 0;
    curr2 = 0;
    while(curr1 < n1 &&  curr2 < n2)
    {
	if(lengths1[order1[curr1]] < lengths2[order2[curr2]])
	    /* seq1 is shorter - increment curr1 index */
	    curr1++;
	else if(lengths1[order1[curr1]] > lengths2[order2[curr2]])
	    /* seq2 is shorter - increment curr2 index */
	    curr2++;
	else
	{
	    /* identical lengths - check all seq1/seq2 entries of this len */
	    for(tmp1=curr1; tmp1<n1
		 && lengths1[order1[tmp1]] == lengths2[order2[curr2]]; tmp1++)
		for(tmp2=curr2; tmp2<n2 && lengths2[order2[tmp2]] ==
		    lengths2[order2[curr2]]; tmp2++)
		    /* check to see if the sequences are identical */
		    if(!ajStrCmpCaseS(ajSeqGetSeqS(ajSeqsetGetseqSeq(seq1,
							     order1[tmp1])),
				      ajSeqGetSeqS(ajSeqsetGetseqSeq(seq2,
				      order2[tmp2]))))
		    {
			hits1[order1[tmp1]] = order2[tmp2];
			hits2[order2[tmp2]] = order1[tmp1];
		    }

	    curr1 = tmp1;
	    curr2 = tmp2;
	}
    }

    /* output the required entries to the list file */
    listor_Output(list, OperatorCode, seq1, seq2, hits1, hits2, n1, n2);


    AJFREE(lengths1);
    AJFREE(lengths2);
    AJFREE(order1);
    AJFREE(order2);
    AJFREE(hits1);
    AJFREE(hits2);
    ajFileClose(&list);
    ajStrDel(&operator);

    ajSeqsetDel(&seq1);
    ajSeqsetDel(&seq2);

    embExit();

    return 0;
}




/* @funcstatic listor_Output **************************************************
**
** Writes out USA of a sequence to a file
**
** @param [u] list [AjPFile] Output file
** @param [r] Operator [ajint] logical operation to perform
** @param [r] seq1 [const AjPSeqset] first seqset
** @param [r] seq2 [const AjPSeqset] second seqset
** @param [r] hits1 [const ajint *] array of hits to seq1
** @param [r] hits2 [const ajint *] array of hits to seq2
** @param [r] n1 [ajint] number of sequences in seq1
** @param [r] n2 [ajint] number of sequences in seq2
** @return [void]
** @@
******************************************************************************/

static void listor_Output(AjPFile list, ajint Operator,
			  const AjPSeqset seq1, const AjPSeqset seq2,
			  const ajint *hits1, const ajint *hits2,
			  ajint n1, ajint n2)
{

     /*
     ** OR - output all of seq1 and then those of seq2 with no hits
     **
     ** AND - output only those of seq1 with hits
     **
     ** XOR - output only those of seq1 with no hits and seq2 with no hits
     **
     ** NOT - output only those of seq1 with no hits
     */

    ajint i;

    /* do seq1 */
    for(i=0; i<n1; i++)
    {
	if(Operator == L_OR)
	    listor_Write(list, seq1, i);
	else if(Operator == L_AND)
	{
	    if(hits1[i] != -1)
		listor_Write(list, seq1, i);
	}
	else if(Operator == L_XOR || Operator == L_NOT)
	{
	    if(hits1[i] == -1)
		listor_Write(list, seq1, i);
	}
    }

    /* do seq2 - write if no hits and OR or no hits and XOR */
    if(Operator == L_OR || Operator == L_XOR)
	for(i=0; i<n2; i++)
	    if(hits2[i] == -1)
		listor_Write(list, seq2, i);

    return;
}




/* @funcstatic listor_Write ***************************************************
**
** Writes out USA of a sequence to a file
**
** @param [u] list [AjPFile] Output file
** @param [r] seqset [const AjPSeqset] seqset
** @param [r] i [ajint] index into seqset for the sequence to write
** @return [void]
** @@
******************************************************************************/

static void listor_Write(AjPFile list, const AjPSeqset seqset, ajint i)
{
    ajFmtPrintF(list, "%S\n", ajSeqGetUsaS(ajSeqsetGetseqSeq(seqset, i)));

    return;
}

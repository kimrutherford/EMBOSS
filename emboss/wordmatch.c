/* @source wordmatch application
**
** Finds matching words  words in DNA sequences
**
** @author
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

/* wordmatch
** Create a word table for the first sequence.  Then go down second
** sequence checking to see if the word matches.  If word matches then
** check to see if the position lines up with the last position if it
** does continue else stop.
**
*/

#include "ajax.h"
#include "emboss.h"




/* @prog wordmatch ************************************************************
**
** Finds all exact matches of a given size between 2 sequences
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPSeq seq1;
    AjPSeq seq2;
    ajint wordlen;
    AjPTable seq1MatchTable = 0;
    AjPList matchlist = NULL;
    AjPFile outf = NULL;
    AjPFeattable Tab1 = NULL;
    AjPFeattable Tab2 = NULL;
    AjPFeattabOut seq1out = NULL;
    AjPFeattabOut seq2out = NULL;
    AjPAlign align = NULL;
    AjIList iter = NULL;
    ajint start1;
    ajint start2;
    ajint len;


    embInit("wordmatch", argc, argv);

    wordlen = ajAcdGetInt("wordsize");
    seq1    = ajAcdGetSeq("asequence");
    seq2    = ajAcdGetSeq("bsequence");

    ajSeqTrim(seq1);
    ajSeqTrim(seq2);

    /* outf = ajAcdGetOutfile("outfile"); */
    align    = ajAcdGetAlign("outfile");

    ajAlignSetExternal(align, ajTrue);

    seq1out  =  ajAcdGetFeatout("aoutfeat");
    seq2out  =  ajAcdGetFeatout("boutfeat");

    embWordLength(wordlen);
    if(embWordGetTable(&seq1MatchTable, seq1))
	matchlist = embWordBuildMatchTable(seq1MatchTable, seq2, ajTrue);

    if(matchlist && outf)
	ajFmtPrintF(outf, "FINALLY length = %d\n",ajListGetLength(matchlist));

    embWordFreeTable(&seq1MatchTable);	/* free table of words */

    if(outf)
	ajFmtPrintF(outf, "%10s %10s Length\n", ajSeqGetNameC(seq1),
		    ajSeqGetNameC(seq2));

    if(matchlist)
    {
	if(outf)
	    embWordMatchListPrint(outf, matchlist);

	iter = ajListIterNewread(matchlist) ;
	while(embWordMatchIter(iter, &start1, &start2, &len))
	{
	    ajAlignDefineSS(align, seq1, seq2);
	    ajAlignSetScoreI(align, len);
	    /* ungapped alignment so same length for both sequences */
	    ajAlignSetSubRange(align,
			       start1, 1, len,
			       ajSeqIsReversed(seq1), ajSeqGetLen(seq1),
			       start2, 1, len,
			       ajSeqIsReversed(seq2), ajSeqGetLen(seq2));
	}
	ajListIterDel(&iter) ;

	embWordMatchListConvToFeat(matchlist,&Tab1,&Tab2,seq1, seq2);

	embWordMatchListDelete(&matchlist); /* free the match structures */
    }
    ajAlignWrite(align);
    ajFeatWrite(seq1out, Tab1);
    ajFeatWrite(seq2out, Tab2);

    ajAlignClose(align);
    ajAlignDel(&align);
    ajSeqDel(&seq1);
    ajSeqDel(&seq2);
    ajFeattableDel(&Tab1);
    ajFeattableDel(&Tab2);
    ajFeattabOutDel(&seq1out);
    ajFeattabOutDel(&seq2out);

    embExit();

    return 0;
}

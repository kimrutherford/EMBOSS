/* @source seqmatchall application
**
** All against all comparison of a set of sequences
**
** @author Copyright (C) Ian Longden (guess by ajb)
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




static void seqmatchall_matchListPrint(void *x,void *cl);
static void seqmatchall_listPrint(AjPAlign align, const AjPList list);

static AjPSeq *seqs = NULL;
static ajuint iseq1 = 0;
static ajuint iseq2 = 0;

ajuint statwordlen;




/* @prog seqmatchall **********************************************************
**
** Does an all-against-all comparison of a set of sequences
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPTable seq1MatchTable = 0;
    AjPList matchlist;
    AjPSeqset seqset;
    AjPAlign align = NULL;

    ajuint i;
    ajuint j;
    ajuint nseqs;

    embInit("seqmatchall", argc, argv);

    seqset      = ajAcdGetSeqset("sequence1");
    statwordlen = ajAcdGetInt("wordsize");
    align    = ajAcdGetAlign("outfile");

    ajAlignSetExternal(align, ajTrue);
    embWordLength(statwordlen);
    seqs = ajSeqsetGetSeqarray(seqset);
    nseqs = ajSeqsetGetSize(seqset);

    for(i=0;i<nseqs;i++)
    {
        iseq1 = i;
	seq1MatchTable = 0;
	if(ajSeqGetLen(seqs[iseq1]) > statwordlen)
	{
	    if(embWordGetTable(&seq1MatchTable, seqs[i])) /* get word table */
	    {
		for(j=i+1;j<ajSeqsetGetSize(seqset);j++)
		{
		    iseq2 = j;
		    if(ajSeqGetLen(seqs[j]) >= statwordlen)
		    {
			matchlist = embWordBuildMatchTable(seq1MatchTable,
							   seqs[j], ajTrue);
			if (ajListGetLength(matchlist))
			{
			    seqmatchall_listPrint(align, matchlist);
			    ajAlignWrite(align);
			    ajAlignReset(align);
			}
			/* free the match structures */
			embWordMatchListDelete(&matchlist);
		    }
		}
	    }
	    embWordFreeTable(&seq1MatchTable); /* free table of words */
	}
    }

    ajAlignClose(align);
    ajAlignDel(&align);
    ajSeqsetDel(&seqset);
    ajSeqDelarray(&seqs);

    embExit();

    return 0;
}




/* @funcstatic seqmatchall_matchListPrint *************************************
**
** Undocumented.
**
** @param [r] x [void*] Undocumented
** @param [r] cl [void*] Undocumented
** @return [void]
** @@
******************************************************************************/

static void seqmatchall_matchListPrint(void *x,void *cl)
{
    EmbPWordMatch p;
    AjPAlign align;

    p = (EmbPWordMatch)x;
    align = (AjPAlign) cl;
/*
    ajFmtPrintF(outfile, "%d  %d %d %s %d %d %s\n",
		(*p).length,
		(*p).seq1start+1,(*p).seq1start+(*p).length,seq1->Name->Ptr,
		(*p).seq2start+1,(*p).seq2start+(*p).length,seq2->Name->Ptr);
*/

    ajAlignDefineSS(align, seqs[iseq1], seqs[iseq2]);
    ajAlignSetScoreI(align, p->length);
    /* ungapped so same length for both sequences */
    ajAlignSetSubRange(align,
                       p->seq1start, 1, p->length,
                       ajSeqIsReversed(seqs[iseq1]), ajSeqGetLen(seqs[iseq1]),
                       p->seq2start, 1, p->length,
                       ajSeqIsReversed(seqs[iseq2]), ajSeqGetLen(seqs[iseq2]));
    

    return;
}




/* @funcstatic seqmatchall_listPrint ******************************************
**
** Undocumented.
**
** @param [u] align [AjPAlign] Alignment object
** @param [r] list [const AjPList] Undocumented
** @@
******************************************************************************/

static void seqmatchall_listPrint(AjPAlign align, const AjPList list)
{
    ajListMapread(list, &seqmatchall_matchListPrint, align);

    return;
}

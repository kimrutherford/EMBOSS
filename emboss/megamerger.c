/* @source megamerger application
**
** Merge two large overlapping nucleic acid sequences
**
** @author Copyright (C) Gary Williams (gwilliam@hgmp.mrc.ac.uk)
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




static void megamerger_Merge(const AjPList matchlist,
			     const AjPSeq seq1, const AjPSeq seq2,
			     AjPSeqout seqout, AjPFile outfile, AjBool prefer);




/* @prog megamerger ***********************************************************
**
** Merge two large overlapping nucleic acid sequences
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPSeq seq1;
    AjPSeq seq2;
    AjPSeqout seqout;
    ajint wordlen;
    AjPTable seq1MatchTable = NULL;
    AjPList matchlist = NULL;
    AjPFile outfile;
    AjBool prefer;

    embInit("megamerger", argc, argv);

    wordlen = ajAcdGetInt("wordsize");
    seq1    = ajAcdGetSeq("asequence");
    seq2    = ajAcdGetSeq("bsequence");
    prefer  = ajAcdGetBool("prefer");
    outfile = ajAcdGetOutfile("outfile");
    seqout  = ajAcdGetSeqout("outseq");

    /* trim sequences to -sbegin and -send */
    ajSeqTrim(seq1);
    ajSeqTrim(seq2);

    embWordLength(wordlen);
    if(embWordGetTable(&seq1MatchTable, seq1))
	/* get table of words */
	matchlist = embWordBuildMatchTable(seq1MatchTable, seq2, ajTrue);
    else
	ajFatal("No match found\n");


    /* get the minimal set of overlapping matches */
       embWordMatchMin(matchlist);

    if(ajListGetLength(matchlist))
    {
	/* make the output file */
	megamerger_Merge(matchlist, seq1, seq2, seqout, outfile, prefer);

	/* tidy up */
    }

    embWordMatchListDelete(&matchlist); /* free the match structures */
    embWordFreeTable(&seq1MatchTable);

    ajSeqoutClose(seqout);
    ajFileClose(&outfile);

    ajSeqDel(&seq1);
    ajSeqDel(&seq2);
    ajSeqoutDel(&seqout);

    embExit();

    return 0;
}




/* @funcstatic megamerger_Merge ***********************************************
**
** Marge and write a report on the merge of the two sequences.
**
** @param [r] matchlist [const AjPList] List of minimal non-overlapping matches
** @param [r] seq1 [const AjPSeq] Sequence to be merged.
** @param [r] seq2 [const AjPSeq] Sequence to be merged.
** @param [w] seqout [AjPSeqout] Output merged sequence
** @param [u] outfile [AjPFile] Output file containing report.
** @param [r] prefer [AjBool] If TRUE, use the first sequence when there
**                            is a mismatch
** @return [void]
** @@
******************************************************************************/

static void megamerger_Merge(const AjPList matchlist,
			     const AjPSeq seq1,const  AjPSeq seq2,
			     AjPSeqout seqout, AjPFile outfile, AjBool prefer)
{
    AjIList iter = NULL;       		/* match list iterator */
    EmbPWordMatch p = NULL;  		/* match structure */
    ajint count = 0;			/* count of matches */
    AjPStr seqstr;			/* merged sequence string */
    AjPStr s1;			/* string of seq1 */
    AjPStr s2;			/* string of seq2 */
    ajuint prev1end = 0;
    ajuint prev2end = 0;       /* end positions (+1) of previous match */
    ajuint mid1;
    ajuint mid2;			/* middle of a mismatch region */
    AjPStr tmp;			/* holds sequence string while uppercasing */
    AjPSeq seq = NULL;

    tmp    = ajStrNew();
    seqstr = ajStrNew();
    s1     = ajSeqGetSeqCopyS(seq1);
    s2     = ajSeqGetSeqCopyS(seq2);

    /* change the sequences to lowercase to highlight problem areas */
    ajStrFmtLower(&s1);
    ajStrFmtLower(&s2);

    /* title line */
    ajFmtPrintF(outfile, "# Report of megamerger of: %s and %s\n\n",
		ajSeqGetNameC(seq1), ajSeqGetNameC(seq2));

    iter = ajListIterNewread(matchlist);
    while(!ajListIterDone(iter))
    {
	p = (EmbPWordMatch) ajListIterGet(iter);
	/* first match? */
	if(!count++)
	{
	    ajFmtPrintF(outfile, "%s overlap starts at %d\n",
			ajSeqGetNameC(seq1), p->seq1start+1);
	    ajFmtPrintF(outfile, "%s overlap starts at %d\n\n",
			ajSeqGetNameC(seq2), p->seq2start+1);

	    /* get initial sequence before the overlapping region */
	    if(p->seq1start == 0)
	    {
		/*
		**  ignore the initial bit if both sequences match from
		**  the start
		*/
		if(p->seq2start > 0)
		{
		    ajFmtPrintF(outfile, "Using %s 1-%d as the "
				"initial sequence\n\n",
				ajSeqGetNameC(seq2), p->seq2start);
		    ajStrAssignSubS(&seqstr, s2, 0, p->seq2start-1);
		}

	    }
	    else if(p->seq2start == 0)
	    {
		ajFmtPrintF(outfile, "Using %s 1-%d as the initial "
			    "sequence\n\n", ajSeqGetNameC(seq1),
			    p->seq1start);
		ajStrAssignSubS(&seqstr, s1, 0, p->seq1start-1);

	    }
	    else
	    {
		ajFmtPrintF(outfile, "WARNING!\n");
		ajFmtPrintF(outfile, "Neither sequence's overlap is at "
			    "the start of the sequence\n");
		if(p->seq1start > p->seq2start)
		{
		    ajFmtPrintF(outfile, "Using %s 1-%d as the "
				"initial sequence\n\n",
				ajSeqGetNameC(seq1), p->seq1start);
		    ajStrAssignSubS(&tmp, s1, 0, p->seq1start-1);
		    ajStrFmtUpper(&tmp);
		    ajStrAssignS(&seqstr, tmp);

		}
		else
		{
		    ajFmtPrintF(outfile, "Using %s 1-%d as the initial "
				"sequence\n\n", ajSeqGetNameC(seq2),
				p->seq2start);
		    ajStrAssignSubS(&tmp, s2, 0, p->seq2start-1);
		    ajStrFmtUpper(&tmp);
		    ajStrAssignS(&seqstr, tmp);

		}
	    }
	}
	else
	{
	    /*
	    **  output the intervening mismatch between the previous
	    **  matching region
	    */
	    ajFmtPrintF(outfile, "\nWARNING!\nMismatch region found:\n");
	    if(prev1end<p->seq1start)
		ajFmtPrintF(outfile, "Mismatch %s %d-%d\n",
			    ajSeqGetNameC(seq1), prev1end+1,
			    p->seq1start);
	    else
		ajFmtPrintF(outfile, "Mismatch %s %d\n",
			    ajSeqGetNameC(seq1), prev1end);

	    if(prev2end<p->seq2start)
		ajFmtPrintF(outfile, "Mismatch %s %d-%d\n",
			    ajSeqGetNameC(seq2), prev2end+1, p->seq2start);
	    else
		ajFmtPrintF(outfile, "Mismatch %s %d\n",
			    ajSeqGetNameC(seq2), prev2end);

            if(prefer)
	    {
                /* use sequence 1 as the 'correct' one */
	        ajStrAssignSubS(&tmp, s1, prev1end, p->seq1start-1);
	        ajStrFmtUpper(&tmp);
	        ajStrAppendS(&seqstr, tmp);
            	
            }
	    else
	    {
                /*
		** use the sequence where the mismatch is furthest
		** from the end as the 'correct' one
		*/
	        mid1 = (prev1end + p->seq1start-1)/2;
	        mid2 = (prev2end + p->seq2start-1)/2;
	        /* is the mismatch closer to the ends of seq1 or seq2? */
	        if(AJMIN(mid1, ajSeqGetLen(seq1)-mid1-1) <
		    AJMIN(mid2, ajSeqGetLen(seq2)-mid2-1))
	        {
		    ajFmtPrintF(outfile, "Mismatch is closer to the ends "
				"of %s, so use %s in the merged "
				"sequence\n\n", ajSeqGetNameC(seq1),
				ajSeqGetNameC(seq2));
		    if(prev2end < p->seq2start)
		    {
		        ajStrAssignSubS(&tmp, s2, prev2end, p->seq2start-1);
		        ajStrFmtUpper(&tmp);
		        ajStrAppendS(&seqstr, tmp);

		    }
	        }
	        else
	        {
		    ajFmtPrintF(outfile,
				"Mismatch is closer to the ends of %s, "
				"so use %s in the merged sequence\n\n",
				ajSeqGetNameC(seq2), ajSeqGetNameC(seq1));
		    if(prev1end < p->seq1start)
		    {
		        ajStrAssignSubS(&tmp, s1, prev1end, p->seq1start-1);
		        ajStrFmtUpper(&tmp);
		        ajStrAppendS(&seqstr, tmp);
		    }
		}
	    }
	}

	/* output the match */
	ajFmtPrintF(outfile, "Matching region %s %d-%d : %s %d-%d\n",
		    ajSeqGetNameC(seq1), p->seq1start+1,
		    p->seq1start + p->length, ajSeqGetNameC(seq2),
		    p->seq2start+1, p->seq2start + p->length);
	ajFmtPrintF(outfile, "Length of match: %d\n", p->length);
	ajStrAppendSubS(&seqstr, s1, p->seq1start, p->seq1start + p->length-1);

	/*
	** note the end positions (+1) to get the intervening region
	** between matches
	*/
	prev1end = p->seq1start + p->length;
	prev2end = p->seq2start + p->length;
    }

    /* end of overlapping region */
    ajFmtPrintF(outfile, "\n%s overlap ends at %d\n", ajSeqGetNameC(seq1),
		p->seq1start+p->length);
    ajFmtPrintF(outfile, "%s overlap ends at %d\n\n", ajSeqGetNameC(seq2),
		p->seq2start+p->length);

    /* is seq1 only longer than the matched regions? */
    if(prev2end >= ajSeqGetLen(seq2) &&
       prev1end < ajSeqGetLen(seq1))
    {
	ajFmtPrintF(outfile, "Using %s %d-%d as the final "
		    "sequence\n\n", ajSeqGetNameC(seq1), prev1end+1,
		    ajSeqGetLen(seq1));
	ajStrAppendSubS(&seqstr, s1, prev1end, ajSeqGetLen(seq1)-1);

	/* is seq2 only longer than the matched regions? */
    }
    else if(prev1end >= ajSeqGetLen(seq1) && prev2end < ajSeqGetLen(seq2))
    {
	ajFmtPrintF(outfile, "Using %s %d-%d as the final "
		    "sequence\n\n", ajSeqGetNameC(seq2), prev2end+1,
		    ajSeqGetLen(seq2));
	ajStrAppendSubS(&seqstr, s2, prev2end, ajSeqGetLen(seq2)-1);

	/* both are longer! */
    }
    else if(prev1end < ajSeqGetLen(seq1) && prev2end < ajSeqGetLen(seq2))
    {
	ajFmtPrintF(outfile, "WARNING!\n");
	ajFmtPrintF(outfile, "Neither sequence's overlap is at the "
		    "end of the sequence\n");
	if(ajSeqGetLen(seq1)-prev1end > ajSeqGetLen(seq2)-prev2end)
	{
	    ajFmtPrintF(outfile, "Using %s %d-%d as the final "
			"sequence\n\n", ajSeqGetNameC(seq1), prev1end+1,
			ajSeqGetLen(seq1));
	    ajStrAssignSubS(&tmp, s1, prev1end, ajSeqGetLen(seq1)-1);
	    ajStrFmtUpper(&tmp);
	    ajStrAppendS(&seqstr, tmp);
	}
	else
	{
	    ajFmtPrintF(outfile, "Using %s %d-%d as the final "
			       "sequence\n\n", ajSeqGetNameC(seq2), prev2end+1,
			       ajSeqGetLen(seq2));
	    ajStrAssignSubS(&tmp, s2, prev2end, ajSeqGetLen(seq2)-1);
	    ajStrFmtUpper(&tmp);
	    ajStrAppendS(&seqstr, tmp);

	}
    }

    /* write out sequence at end */
    seq = ajSeqNewSeq(seq1);
    ajSeqAssignSeqS(seq, seqstr);
    ajSeqoutWriteSeq(seqout, seq);

    ajSeqDel(&seq);
    ajStrDel(&s1);
    ajStrDel(&s2);
    ajStrDel(&tmp);
    ajStrDel(&seqstr);
    ajListIterDel(&iter);

    return;
}

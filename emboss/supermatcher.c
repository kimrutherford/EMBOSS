/* @source supermatcher application
**
** Local alignment of large sequences
**
** @author Copyright (C) Ian Longden
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

/* supermatcher
** Create a word table for the sequence set.
** Then iterates over the sequence stream first checking any possible word
** matches.
** If word matches then check to see if the position lines up with the last
** position if it does continue else stop.
** This gives us the start (offset) for the smith-waterman match by finding
** the biggest match and calculating start and ends for both sequences.
*/


/*
** Word matching function was re-implemented using Rabin-Karp multi-pattern
** search algorithm. May 2010.
*/

#include "emboss.h"




static void supermatcher_findendpoints(const EmbPWordMatch max,
	const AjPSeq trgseq, const AjPSeq qryseq,
	ajint *trgstart, ajint *qrystart,
	ajint *trgend, ajint *qryend);




/* @prog supermatcher *********************************************************
**
** Finds matches of a set of sequences against one or more sequences
**
** Create a word table for the second sequence.
** Then go down first sequence checking to see if the word matches.
** If word matches then check to see if the position lines up with the last
** position if it does continue else stop.
** This gives us the start (offset) for the smith-waterman match by finding
** the biggest match and calculating start and ends for both sequences.
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall queryseqs;
    AjPSeqset targetseqs;
    AjPSeq queryseq;
    const AjPSeq targetseq;
    AjPStr queryaln = 0;
    AjPStr targetaln = 0;

    AjPFile errorf;
    AjBool show = ajFalse;

    const char   *queryseqc;
    const char   *targetseqc;

    AjPMatrixf matrix;
    AjPSeqCvt cvt = 0;
    float **sub;
    ajint *compass = NULL;
    float *path = NULL;

    float gapopen;
    float gapextend;
    float score;
    float minscore;

    ajuint j, k;
    ajint querystart = 0;
    ajint targetstart = 0;
    ajint queryend   = 0;
    ajint targetend   = 0;
    ajint width  = 0;
    AjPTable kmers = 0;
    ajint wordlen = 6;
    ajint oldmax = 0;
    ajint newmax = 0;

    ajuint ntargetseqs;
    ajuint nkmers;

    AjPAlign align = NULL;
    EmbPWordMatch maxmatch; /* match with maximum score */

    /* Cursors for the current sequence being scanned,
    ** i.e., until which location it was scanned.
    ** Separate cursor/location entries for each sequence in the seqset.
    */
    ajuint* lastlocation;

    EmbPWordRK* wordsw = NULL;
    AjPList* matchlist = NULL;

    embInit("supermatcher", argc, argv);

    matrix    = ajAcdGetMatrixf("datafile");
    queryseqs = ajAcdGetSeqall("asequence");
    targetseqs= ajAcdGetSeqset("bsequence");
    gapopen   = ajAcdGetFloat("gapopen");
    gapextend = ajAcdGetFloat("gapextend");
    wordlen   = ajAcdGetInt("wordlen");
    align     = ajAcdGetAlign("outfile");
    errorf    = ajAcdGetOutfile("errorfile");
    width     = ajAcdGetInt("width");	/* width for banded Smith-Waterman */
    minscore  = ajAcdGetFloat("minscore");

    gapopen   = ajRoundFloat(gapopen, 8);
    gapextend = ajRoundFloat(gapextend, 8);

    sub = ajMatrixfGetMatrix(matrix);
    cvt = ajMatrixfGetCvt(matrix);

    embWordLength(wordlen);

    /* seqset sequence is the reference sequence for SAM format */
    ajAlignSetRefSeqIndx(align, 1);

    ajSeqsetTrim(targetseqs);

    ntargetseqs = ajSeqsetGetSize(targetseqs);

    AJCNEW0(matchlist, ntargetseqs);

    /* get tables of words */
    for(k=0;k<ntargetseqs;k++)
    {
	targetseq = ajSeqsetGetseqSeq(targetseqs, k);
	embWordGetTable(&kmers, targetseq);
	ajDebug("Number of distinct kmers found so far: %d\n",
		ajTableGetLength(kmers));
    }
    AJCNEW0(lastlocation, ntargetseqs);

    if(ajTableGetLength(kmers)<1)
	ajErr("no kmers found");

    nkmers = embWordRabinKarpInit(kmers, &wordsw, wordlen, targetseqs);

    while(ajSeqallNext(queryseqs,&queryseq))
    {
	ajSeqTrim(queryseq);

	queryaln = ajStrNewRes(1+ajSeqGetLen(queryseq));

	ajDebug("Read '%S'\n", ajSeqGetNameS(queryseq));

	for(k=0;k<ntargetseqs;k++)
	{
	    lastlocation[k]=0;
	    matchlist[k] = ajListstrNew();
	}

	embWordRabinKarpSearch(ajSeqGetSeqS(queryseq), targetseqs,
		(const EmbPWordRK*)wordsw, wordlen, nkmers,
		matchlist, lastlocation, ajFalse);


	for(k=0;k<ajSeqsetGetSize(targetseqs);k++)
	{
	    targetseq      = ajSeqsetGetseqSeq(targetseqs, k);

	    ajDebug("Processing '%S'\n", ajSeqGetNameS(targetseq));

	    if(ajListGetLength(matchlist[k])==0)
	    {
		ajFmtPrintF(errorf,
		            "No wordmatch start points for "
		            "%s vs %s. No alignment\n",
		            ajSeqGetNameC(queryseq),ajSeqGetNameC(targetseq));
		embWordMatchListDelete(&matchlist[k]);
		continue;
	    }


	    /* only the maximum match is used as seed
	     * (if there is more than one location with the maximum match
	     * only the first one is used)
	     * TODO: we should add a new option to make above limit optional
	     */
	    maxmatch = embWordMatchFirstMax(matchlist[k]);

	    supermatcher_findendpoints(maxmatch,targetseq, queryseq,
		    &targetstart, &querystart,
		    &targetend, &queryend);

	    targetaln=ajStrNewRes(1+ajSeqGetLen(targetseq));
	    queryseqc = ajSeqGetSeqC(queryseq);
	    targetseqc = ajSeqGetSeqC(targetseq);

	    ajStrAssignC(&queryaln,"");
	    ajStrAssignC(&targetaln,"");

	    ajDebug("++ %S v %S start:%d %d end:%d %d\n",
		    ajSeqGetNameS(targetseq), ajSeqGetNameS(queryseq),
		    targetstart, querystart, targetend, queryend);

	    newmax = (targetend-targetstart+2)*width;

	    if(newmax > oldmax)
	    {
		AJCRESIZE0(path,oldmax,newmax);
		AJCRESIZE0(compass,oldmax,newmax);
		oldmax=newmax;
		ajDebug("++ memory re/allocation for path/compass arrays"
			" to size: %d\n", newmax);
	    }
	    else
	    {
		AJCSET0(path,newmax);
		AJCSET0(compass,newmax);
	    }

	    ajDebug("Calling embAlignPathCalcSWFast "
		    "%d..%d [%d/%d] %d..%d [%d/%d] width:%d\n",
		    querystart, queryend, (queryend - querystart + 1),
		    ajSeqGetLen(queryseq),
		    targetstart, targetend, (targetend - targetstart + 1),
		    ajSeqGetLen(targetseq),
		    width);

	    score = embAlignPathCalcSWFast(&targetseqc[targetstart],
	                                   &queryseqc[querystart],
	                                   targetend-targetstart+1,
	                                   queryend-querystart+1,
	                                   0,width,
	                                   gapopen,gapextend,
	                                   path,sub,cvt,
	                                   compass,show);
	    if(score>minscore)
	    {
		embAlignWalkSWMatrixFast(path,compass,gapopen,gapextend,
		                         targetseq,queryseq,
		                         &targetaln,&queryaln,
		                         targetend-targetstart+1,
		                         queryend-querystart+1,
		                         0,width,
		                         &targetstart,&querystart);

		if(!ajAlignFormatShowsSequences(align))
		{
		    ajAlignDefineCC(align, ajStrGetPtr(targetaln),
		                    ajStrGetPtr(queryaln),
		                    ajSeqGetNameC(targetseq),
		                    ajSeqGetNameC(queryseq));
		    ajAlignSetScoreR(align, score);
		}
		else
		{
		    ajDebug(" queryaln:%S \ntargetaln:%S\n",
		            queryaln,targetaln);
		    embAlignReportLocal(align,
			    queryseq, targetseq,
			    queryaln, targetaln,
			    querystart, targetstart,
			    gapopen, gapextend,
			    score, matrix,
			    1 + ajSeqGetOffset(queryseq),
			    1 + ajSeqGetOffset(targetseq)
		    );
		}
		ajAlignWrite(align);
		ajAlignReset(align);
	    }
	    ajStrDel(&targetaln);

	    embWordMatchListDelete(&matchlist[k]);
	}

	ajStrDel(&queryaln);
    }


    for(k=0;k<nkmers;k++)
    {
	AJFREE(wordsw[k]->seqindxs);
	AJFREE(wordsw[k]->nSeqMatches);

	for(j=0;j<wordsw[k]->nseqs;j++)
	    AJFREE(wordsw[k]->locs[j]);

	AJFREE(wordsw[k]->nnseqlocs);
	AJFREE(wordsw[k]->locs);
	AJFREE(wordsw[k]);
    }

    embWordFreeTable(&kmers);

    if(!ajAlignFormatShowsSequences(align))
	ajMatrixfDel(&matrix);
    
    AJFREE(path);
    AJFREE(compass);
    AJFREE(kmers);
    AJFREE(wordsw);

    AJFREE(matchlist);
    AJFREE(lastlocation);

    ajAlignClose(align);
    ajAlignDel(&align);
    ajSeqallDel(&queryseqs);
    ajSeqDel(&queryseq);
    ajSeqsetDel(&targetseqs);
    ajFileClose(&errorf);

    embExit();

    return 0;
}




/* @funcstatic supermatcher_findendpoints ***********************************
**
** Calculate end points for banded Smith-Waterman alignment.
**
** @param [r] max [const EmbPWordMatch] match with maximum similarity
** @param [r] trgseq [const AjPSeq] target sequence
** @param [r] qryseq [const AjPSeq] query sequence
** @param [w] trgstart [ajint*] start in target sequence
** @param [w] qrystart [ajint*] start in query sequence
** @param [w] trgend [ajint*] end in target sequence
** @param [w] qryend [ajint*] end in query sequence
** @return [void]
** @@
******************************************************************************/

static void supermatcher_findendpoints(const EmbPWordMatch max,
	const AjPSeq trgseq, const AjPSeq qryseq,
	ajint *trgstart, ajint *qrystart,
	ajint *trgend, ajint *qryend)
{
    ajint amax;
    ajint bmax;
    ajint offset;

    *trgstart = max->seq1start;
    *qrystart = max->seq2start;

    offset = *trgstart - *qrystart;

    if(offset > 0)
    {
	*trgstart = offset;
	*qrystart = 0;
    }
    else
    {
	*qrystart = 0-offset;
	*trgstart = 0;
    }

    amax = ajSeqGetLen(trgseq)-1;
    bmax = ajSeqGetLen(qryseq)-1;

    *trgend = *trgstart;
    *qryend = *qrystart;

    ajDebug("++ end1 %d -> %d end2 %d -> %d\n", *trgend, amax, *qryend, bmax);

    while(*trgend<amax && *qryend<bmax)
    {
	(*trgend)++;
	(*qryend)++;
    }

    ajDebug("++ end1 %d end2 %d\n", *trgend, *qryend);

    ajDebug("supermatcher_findendpoints: %d..%d [%d] %d..%d [%d]\n",
	    trgstart, *trgend, ajSeqGetLen(trgseq), qrystart, *qryend,
	    ajSeqGetLen(qryseq));

    return;
}

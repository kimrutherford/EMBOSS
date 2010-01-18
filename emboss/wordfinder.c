/* @source wordfinder application
**
** Word-based rapid comparison of large sequences
**
** @author Copyright (C) Peter Rice
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
#include <limits.h>
#include <math.h>


/*
** Notes:
** Specific requests to search for word-based matches with 15-35 identical
** residues ungapped, and then to check that these are the only significant
** matches by checking for general homology, for example histones matching.
** Intended for use in widely divergent species.
*/


/* @datastatic concat *********************************************************
**
** wordfinder internals
**
** @alias concatS
**
** @attr offset [ajint] Undocumented
** @attr count [ajint] Undocumented
** @attr list [AjPList] Undocumented
** @attr total [ajint] Undocumented
** @attr Padding [char[4]] Padding to alignment boundary
******************************************************************************/

typedef struct concatS
{
    ajint offset;
    ajint count;
    AjPList list;
    ajint total;
    char  Padding[4];
} concat;




static void wordfinder_matchListOrder(void **x,void *cl);
static void wordfinder_orderandconcat(AjPList list,AjPList ordered);
static void wordfinder_removelists(void **x,void *cl);
static ajint wordfinder_findstartpoints(AjPTable seq1MatchTable,
					const AjPSeq b, const AjPSeq a,
					ajint *trgstart, ajint *qrystart,
					ajint *trgend, ajint *qryend);
static void wordfinder_findmax(void **x,void *cl);




concat *conmax = NULL;
ajint maxgap   = 0;




/* @prog wordfinder *********************************************************
**
** Finds a match of a large sequence against one or more sequences
**
** Create a word table for the first sequence.
** Then go down second sequence checking to see if the word matches.
** If word matches then check to see if the position lines up with the last
** position if it does continue else stop.
** This gives us the start (offset) for the smith-waterman match by finding
** the biggest match and calculating start and ends for both sequences.
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqset qryseqs;
    AjPSeqall trgseqs;
    AjPSeq trgseq;
    const AjPSeq qryseq;
    AjPStr mtrg = 0;
    AjPStr nqry = 0;

    AjPFile errorf;
    AjBool show = ajFalse;

    ajint    trglen = 0;
    ajint    qrylen = 0;

    const char   *ptrg;
    const char   *qqry;

    AjPMatrixf matrix;
    AjPSeqCvt cvt = 0;
    float **sub;
    ajint *compass = 0;
    float *path = 0;

    float gapopen;
    float gapextend;
    float score = 0.0;
    ajint matchscore = 0;
    ajint limitmatch;
    ajint limitalign;
    ajint lowmatch;
    ajint lowalign;

    ajint trgbegin;
    ajint i;
    ajuint k;
    ajint qrybegin;
    ajint trgstart = 0;
    ajint qrystart = 0;
    ajint trgend   = 0;
    ajint qryend   = 0;
    ajint width  = 0;
    AjPTable seq1MatchTable = 0;
    ajint wordlen = 6;
    ajint oldmax = 0;

    AjPAlign align = NULL;
    ajint imatches = 0;
    ajint itrg = 0;
    AjPStr tmpstr = NULL;

    embInit("wordfinder", argc, argv);

    matrix    = ajAcdGetMatrixf("datafile");
    qryseqs   = ajAcdGetSeqset("asequence");
    trgseqs   = ajAcdGetSeqall("bsequence");
    gapopen   = ajAcdGetFloat("gapopen");
    gapextend = ajAcdGetFloat("gapextend");
    wordlen   = ajAcdGetInt("wordlen");
    limitmatch = ajAcdGetInt("limitmatch");
    limitalign = ajAcdGetInt("limitalign");
    lowmatch = ajAcdGetInt("lowmatch");
    lowalign = ajAcdGetInt("lowalign");
    align     = ajAcdGetAlign("outfile");
    errorf    = ajAcdGetOutfile("errorfile");
    width     = ajAcdGetInt("width");	/* not the same as awidth */

    gapopen   = ajRoundFloat(gapopen, 8);
    gapextend = ajRoundFloat(gapextend, 8);


    sub = ajMatrixfGetMatrix(matrix);
    cvt = ajMatrixfGetCvt(matrix);

    embWordLength(wordlen);

    ajSeqsetTrim(qryseqs);

    while(ajSeqallNext(trgseqs,&trgseq))
    {
        itrg++;
        imatches = 0;
        ajSeqTrim(trgseq);
	trgbegin = 1 + ajSeqGetOffset(trgseq);

	mtrg = ajStrNewRes(1+ajSeqGetLen(trgseq));

	trglen = ajSeqGetLen(trgseq);

	ajDebug("Read '%S'\n", ajSeqGetNameS(trgseq));
	ajSeqTrace(trgseq);
	if(embWordGetTable(&seq1MatchTable, trgseq)) /* get table of words */
	{
	    for(k=0;k<ajSeqsetGetSize(qryseqs);k++)
	    {
	        qryseq = ajSeqsetGetseqSeq(qryseqs, k);
		qrylen   = ajSeqGetLen(qryseq);
		qrybegin = 1 + ajSeqGetOffset(qryseq);

		ajDebug("Processing '%S'\n", ajSeqGetNameS(qryseq));

		matchscore = wordfinder_findstartpoints(seq1MatchTable,
							qryseq, trgseq,
							&qrystart, &trgstart,
							&qryend, &trgend);
		if(!matchscore ||
		   (limitmatch && (matchscore > limitmatch)) ||
		   (matchscore < lowmatch))
		{
		    if(matchscore)
		    {
			ajFmtPrintF(errorf,"Match limits (%d..%d) exclude '%S' '%S' %d\n",
			       lowmatch, limitmatch,
			       ajSeqGetNameS(qryseq),
			       ajSeqGetNameS(trgseq),
			       matchscore);
		    }
		    continue;
		}
		
		nqry=ajStrNewRes(1+ajSeqGetLen(qryseq));
		ptrg = ajSeqGetSeqC(trgseq);
		qqry = ajSeqGetSeqC(qryseq);

		ajStrAssignC(&mtrg,"");
		ajStrAssignC(&nqry,"");

		ajDebug("++ %S v %S start:%d %d end:%d %d\n",
			ajSeqGetNameS(qryseq), ajSeqGetNameS(trgseq),
			qrystart, trgstart, qryend, trgend);
		imatches++;

		if(trgend-trgstart > oldmax)
		{
		    oldmax = ((trgend-trgstart)+1)*width;
		    AJRESIZE(path,oldmax*width*sizeof(float));
		    AJRESIZE(compass,oldmax*width*sizeof(ajint));
		    ajDebug("++ resize to oldmax: %d\n", oldmax);
		}

		for(i=0;i<((trgend-trgstart)+1)*width;i++)
		  path[i] = 0.0;

		ajDebug("Calling embAlignPathCalcSWFast "
			"%d..%d [%d/%d] %d..%d [%d/%d] width:%d\n",
			trgstart, trgend, (trgend - trgstart + 1), trglen,
			qrystart, qryend, (qryend - qrystart + 1), qrylen,
                        width);

		score = embAlignPathCalcSWFast(&qqry[qrystart],
                                               &ptrg[trgstart],
                                               qryend-qrystart+1,
                                               trgend-trgstart+1,
                                               0,width,
                                               gapopen,gapextend,path,sub,cvt,
                                               compass,show);

		    embAlignWalkSWMatrixFast(path,compass,gapopen,gapextend,
					     qryseq,trgseq,
					     &nqry,&mtrg,
					     qryend-qrystart+1,
					     trgend-trgstart+1,
					     0, width,
					     &qrystart,&trgstart);
		    
            if(!ajAlignFormatShowsSequences(align))
            {
                ajAlignDefineCC(align, ajStrGetPtr(mtrg),
                        ajStrGetPtr(nqry), ajSeqGetNameC(trgseq),
                        ajSeqGetNameC(qryseq));
                ajAlignSetScoreR(align, score);
            }
            else
                embAlignReportLocal(align,
                        qryseq, trgseq,
                        nqry,mtrg,
                        qrystart,trgstart,
                        gapopen, gapextend,
                        score,matrix,
                        qrybegin, trgbegin);


		    if((limitalign && (ajAlignGetLen(align) > limitalign)) ||
		       (ajAlignGetLen(align) < lowalign))
		    {
			imatches--;
			ajFmtPrintF(errorf,"Align limits (%d..%d) excludes '%S' '%S' %d\n",
			       lowalign, limitalign,
			       ajSeqGetNameS(qryseq),
			       ajSeqGetNameS(trgseq),
			       ajAlignGetLen(align));
		    }
		    else
		    {
		        if(ajAlignFormatShowsSequences(align))
		          {
		              ajFmtPrintS(&tmpstr, "\nWordscore:%d", matchscore);
		              ajAlignSetSubHeader(align, tmpstr);
		              ajFmtPrintS(&tmpstr, "Alignlength:%d",
		                      ajAlignGetLen(align));
		              ajAlignSetSubHeaderApp(align, tmpstr);
		              ajStrDel(&tmpstr);
		          }
		        ajAlignWrite(align);
		    }
            ajAlignReset(align);
            ajStrDel(&nqry);
	    }
	}
	embWordFreeTable(&seq1MatchTable); /* free table of words */
	seq1MatchTable=0;

	ajStrDel(&mtrg);

	ajDebug("... %d matches", imatches);
	if(imatches)
	    ajFmtPrintF(errorf, "Target %d %S matches %d\n",
	         itrg, ajSeqGetNameS(trgseq), imatches);
    }

    if(!ajAlignFormatShowsSequences(align))
    {
        ajMatrixfDel(&matrix);        
    }
    
    AJFREE(path);
    AJFREE(compass);

    ajAlignClose(align);
    ajAlignDel(&align);
    ajSeqallDel(&trgseqs);
    ajSeqDel(&trgseq);
    ajSeqsetDel(&qryseqs);
    ajFileClose(&errorf);

    embExit();

    return 0;
}




/* @funcstatic wordfinder_matchListOrder ************************************
**
** Undocumented.
**
** @param [r] x [void**] Undocumented
** @param [r] cl [void*] Undocumented
** @return [void]
** @@
******************************************************************************/

static void wordfinder_matchListOrder(void **x,void *cl)
{
    EmbPWordMatch p;
    AjPList ordered;
    ajint offset;
    AjIList listIter;
    concat *con;
    concat *c=NULL;

    p = (EmbPWordMatch)*x;
    ordered = (AjPList) cl;

    offset = (*p).seq1start-(*p).seq2start;

    /* iterate through ordered list to find if it exists already*/
    listIter = ajListIterNewread(ordered);

    while(!ajListIterDone( listIter))
    {
	con = ajListIterGet(listIter);
	if(con->offset == offset)
	{
	    /* found so add count and set offset to the new value */
	    con->offset = offset;
	    con->total+= (*p).length;
	    con->count++;
	    ajListPushAppend(con->list,p);
	    ajListIterDel(&listIter);
	    return;
	}
    }
    ajListIterDel(&listIter);

    /* not found so add it */
    AJNEW(c);
    c->offset = offset;
    c->total  = (*p).length;
    c->count  = 1;
    c->list   = ajListNew();
    ajListPushAppend(c->list,p);
    ajListPushAppend(ordered, c);

    return;
}




/* @funcstatic wordfinder_orderandconcat ************************************
**
** Undocumented.
**
** @param [u] list [AjPList] unordered input list - elements added to the
**                           ordered list, but apparently not deleted.
** @param [w] ordered [AjPList] ordered output list
** @return [void]
** @@
******************************************************************************/

static void wordfinder_orderandconcat(AjPList list,AjPList ordered)
{
    ajListMap(list,wordfinder_matchListOrder, ordered);

    return;
}




/* @funcstatic wordfinder_removelists ***************************************
**
** Undocumented.
**
** @param [r] x [void**] Undocumented
** @param [r] cl [void*] Undocumented
** @return [void]
** @@
******************************************************************************/

static void wordfinder_removelists(void **x,void *cl)
{
    concat *p;

    (void) cl;				/* make it used */

    p = (concat *)*x;

    ajListFree(&(p)->list);
    AJFREE(p);

    return;
}




/* @funcstatic wordfinder_findmax *******************************************
**
** Undocumented.
**
** @param [r] x [void**] Undocumented
** @param [r] cl [void*] Undocumented
** @return [void]
** @@
******************************************************************************/

static void wordfinder_findmax(void **x,void *cl)
{
    concat *p;
    ajint *max;

    p   = (concat *)*x;
    max = (ajint *) cl;

    if(p->total > *max)
    {
	*max = p->total;
	conmax = p;
    }

    return;
}




/* @funcstatic wordfinder_findstartpoints ***********************************
**
** Undocumented.
**
** @param [w] seq1MatchTable [AjPTable] match table
** @param [r] qryseq [const AjPSeq] query sequence 1
** @param [r] trgseq [const AjPSeq] target sequence 2
** @param [w] qrystart [ajint*] start in sequence 2
** @param [w] trgstart [ajint*] start in sequence 1
** @param [w] qryend [ajint*] end in sequence 2
** @param [w] trgend [ajint*] end in sequence 1
** @return [ajint] Undocumented
** @@
******************************************************************************/

static ajint wordfinder_findstartpoints(AjPTable seq1MatchTable,
					const AjPSeq qryseq,
					const AjPSeq trgseq,
					ajint *qrystart, ajint *trgstart,
					ajint *qryend,	ajint *trgend)
{
    ajint max = -10;
    ajint offset = 0;
    AjPList matchlist = NULL;
    AjPList ordered = NULL;
    ajint trgmax;
    ajint qrymax;
    ajint bega;
    ajint begb;

    trgmax = ajSeqGetLen(trgseq)-1;
    qrymax = ajSeqGetLen(qryseq)-1;
    bega = ajSeqGetOffset(trgseq);
    begb = ajSeqGetOffset(qryseq);


    ajDebug("wordfinder_findstartpoints len %d %d off %d %d\n",
	     trgmax, qrymax, bega, begb);
    matchlist = embWordBuildMatchTable(seq1MatchTable, qryseq, ajTrue);

    if(!matchlist)
	return 0;
    else if(!matchlist->Count)
    {
        embWordMatchListDelete(&matchlist);
	return 0;
    }


    /* order and add if the gap is gapmax or less */

    /* create list header bit*/
    ordered = ajListNew();

    wordfinder_orderandconcat(matchlist, ordered);

    ajListMap(ordered,wordfinder_findmax, &max);

    ajDebug("findstart conmax off:%d count:%d total:%d\n",
	    conmax->offset, conmax->count, conmax->total,
	    ajListGetLength(conmax->list));
    offset = conmax->offset;

    ajListMap(ordered,wordfinder_removelists, NULL);
    ajListFree(&ordered);
    embWordMatchListDelete(&matchlist);	/* free the match structures */


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
    *trgend = *trgstart;
    *qryend = *qrystart;

    ajDebug("++ trgend %d -> %d qryend %d -> %d\n",
	    *trgend, trgmax, *qryend, qrymax);
    while(*trgend<trgmax && *qryend<qrymax)
    {
	(*trgend)++;
	(*qryend)++;
    }

    ajDebug("++ trgend %d qryend %d\n", *trgend, *qryend);
    
    
    ajDebug("wordfinder_findstartpoints has %d..%d [%d] %d..%d [%d]\n",
	    *qrystart, *qryend, ajSeqGetLen(qryseq),
	    *trgstart, *trgend, ajSeqGetLen(trgseq));

    return max;
}

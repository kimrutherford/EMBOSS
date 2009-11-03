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
** Create a word table for the first sequence.
** Then go down second sequence checking to see if the word matches.
** If word matches then check to see if the position lines up with the last
** position if it does continue else stop.
** This gives us the start (offset) for the smith-waterman match by finding
** the biggest match and calculating start and ends for both sequences.
*/

#include "emboss.h"
#include <limits.h>
#include <math.h>




/* @datastatic concat *********************************************************
**
** supermatcher internals
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




static void supermatcher_matchListOrder(void **x,void *cl);
static void supermatcher_orderandconcat(AjPList list,AjPList ordered);
static void supermatcher_removelists(void **x,void *cl);
static ajint supermatcher_findstartpoints(AjPTable seq1MatchTable,
					  const AjPSeq b, const AjPSeq a,
					  ajint *start1, ajint *start2,
					  ajint *end1, ajint *end2);
static void supermatcher_findmax(void **x,void *cl);




concat *conmax = NULL;
ajint maxgap   = 0;




/* @prog supermatcher *********************************************************
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
    AjPSeqall seq1;
    AjPSeqset seq2;
    AjPSeq a;
    const AjPSeq b;
    AjPStr m = 0;
    AjPStr n = 0;

    AjPFile outf = NULL;
    AjPFile errorf;
    AjBool show = ajFalse;
    AjBool scoreonly = ajFalse;
    AjBool showalign = ajTrue;

    ajint    lena = 0;
    ajint    lenb = 0;

    const char   *p;
    const char   *q;

    AjPMatrixf matrix;
    AjPSeqCvt cvt = 0;
    float **sub;
    ajint *compass = 0;
    float *path = 0;

    float gapopen;
    float gapextend;
    float score;


    ajint begina;
    ajint i;
    ajuint k;
    ajint beginb;
    ajint start1 = 0;
    ajint start2 = 0;
    ajint end1   = 0;
    ajint end2   = 0;
    ajint width  = 0;
    AjPTable seq1MatchTable = 0;
    ajint wordlen = 6;
    ajint oldmax = 0;

    AjPAlign align = NULL;

    embInit("supermatcher", argc, argv);

    matrix    = ajAcdGetMatrixf("datafile");
    seq1      = ajAcdGetSeqall("asequence");
    seq2      = ajAcdGetSeqset("bsequence");
    gapopen   = ajAcdGetFloat("gapopen");
    gapextend = ajAcdGetFloat("gapextend");
    wordlen   = ajAcdGetInt("wordlen");
    align     = ajAcdGetAlign("outfile");
    errorf    = ajAcdGetOutfile("errorfile");
    width     = ajAcdGetInt("width");	/* not the same as awidth */

    /* obsolete. Can be uncommented in acd file and here to reuse */

    /* outf      = ajAcdGetOutfile("originalfile"); */
    /* show      = ajAcdGetBool("showinternals");*/
    /* scoreonly = ajAcdGetBool("scoreonly"); */
    /* showalign = ajAcdGetBool("showalign"); */

    gapopen   = ajRoundF(gapopen, 8);
    gapextend = ajRoundF(gapextend, 8);

    if(!showalign)
	scoreonly = ajTrue;

    sub = ajMatrixfArray(matrix);
    cvt = ajMatrixfCvt(matrix);

    embWordLength(wordlen);

    ajSeqsetTrim(seq2);

    while(ajSeqallNext(seq1,&a))
    {
        ajSeqTrim(a);
	begina = 1 + ajSeqGetOffset(a);

	m = ajStrNewRes(1+ajSeqGetLen(a));

	lena = ajSeqGetLen(a);

	ajDebug("Read '%S'\n", ajSeqGetNameS(a));

	if(!embWordGetTable(&seq1MatchTable, a)) /* get table of words */
	    ajErr("Could not generate table for %s\n",
		  ajSeqGetNameC(a));

	for(k=0;k<ajSeqsetGetSize(seq2);k++)
	{
	    b      = ajSeqsetGetseqSeq(seq2, k);
	    lenb   = ajSeqGetLen(b);
	    beginb = 1 + ajSeqGetOffset(b);

	    n=ajStrNewRes(1+ajSeqGetLen(b));

	    ajDebug("Processing '%S'\n", ajSeqGetNameS(b));
	    p = ajSeqGetSeqC(a);
	    q = ajSeqGetSeqC(b);

	    ajStrAssignC(&m,"");
	    ajStrAssignC(&n,"");


	    if(!supermatcher_findstartpoints(seq1MatchTable,b,a,
					     &start1, &start2,
					     &end1, &end2))
	    {
		start1 = 0;
		end1   = lena-1;
		start2 = (ajint)(width/2);
		end2   = lenb-1;

		ajFmtPrintF(errorf,
			    "No wordmatch start points for "
			    "%s vs %s. No alignment\n",
			    ajSeqGetNameC(a),ajSeqGetNameC(b));
		ajStrDel(&n);
		continue;
	    }
	    ajDebug("++ %S v %S start:%d %d end:%d %d\n",
		    ajSeqGetNameS(a), ajSeqGetNameS(b),
		    start1, start2, end1, end2);

	    if(end1-start1 > oldmax)
	    {
		oldmax = ((end1-start1)+1)*width;
		AJRESIZE(path,oldmax*width*sizeof(float));
		AJRESIZE(compass,oldmax*width*sizeof(ajint));
		ajDebug("++ resize to oldmax: %d\n", oldmax);
	    }

	    for(i=0;i<((end1-start1)+1)*width;i++)
		path[i] = 0.0;

	    ajDebug("Calling embAlignPathCalcFast "
		     "%d..%d [%d/%d] %d..%d [%d/%d]\n",
		     start1, end1, (end1 - start1 + 1), lena,
		     start2, end2, (end2 - start2 + 1), lenb);

	    embAlignPathCalcFast(&p[start1],&q[start2],
				 end1-start1+1,end2-start2+1,
				 gapopen,gapextend,path,sub,cvt,
				 compass,show,width);


	    ajDebug("Calling embAlignScoreSWMatrixFast\n");

	    score = embAlignScoreSWMatrixFast(path,compass,gapopen,gapextend,
					      a,b,end1-start1+1,end2-start2+1,
					      sub,cvt,&start1,&start2,width);

	    if(scoreonly)
	    {
		if(outf)
		    ajFmtPrintF(outf,"%s %s %.2f\n",ajSeqGetNameC(a),ajSeqGetNameC(b),
				score);
	    }
	    else
	    {
		ajDebug("Calling embAlignWalkSWMatrixFast\n");
		embAlignWalkSWMatrixFast(path,compass,gapopen,gapextend,a,b,
					 &m,&n,end1-start1+1,end2-start2+1,
					 sub,cvt,&start1,&start2,width);

		ajDebug("Calling embAlignPrintLocal\n");
		if(outf)
		    embAlignPrintLocal(outf,m,n,start1,start2,
				       score,1,sub,cvt,ajSeqGetNameC(a),
				       ajSeqGetNameC(b),
				       begina,beginb);
		embAlignReportLocal(align, a, b,
				    m,n,start1,start2,
				    gapopen, gapextend,
				    score,matrix, begina, beginb);
		ajAlignWrite(align);
		ajAlignReset(align);
	    }
	    ajStrDel(&n);
	}

	embWordFreeTable(&seq1MatchTable); /* free table of words */
	seq1MatchTable=0;

	ajStrDel(&m);

    }

    AJFREE(path);
    AJFREE(compass);

    ajAlignClose(align);
    ajAlignDel(&align);
    ajSeqallDel(&seq1);
    ajSeqDel(&a);
    ajSeqsetDel(&seq2);
    ajFileClose(&outf);
    ajFileClose(&errorf);

    embExit();

    return 0;
}




/* @funcstatic supermatcher_matchListOrder ************************************
**
** Undocumented.
**
** @param [r] x [void**] Undocumented
** @param [r] cl [void*] Undocumented
** @return [void]
** @@
******************************************************************************/

static void supermatcher_matchListOrder(void **x,void *cl)
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




/* @funcstatic supermatcher_orderandconcat ************************************
**
** Undocumented.
**
** @param [u] list [AjPList] unordered input list - elements added to the
**                           ordered list, but apparently not deleted.
** @param [w] ordered [AjPList] ordered output list
** @return [void]
** @@
******************************************************************************/

static void supermatcher_orderandconcat(AjPList list,AjPList ordered)
{
    ajListMap(list,supermatcher_matchListOrder, ordered);

    return;
}




/* @funcstatic supermatcher_removelists ***************************************
**
** Undocumented.
**
** @param [r] x [void**] Undocumented
** @param [r] cl [void*] Undocumented
** @return [void]
** @@
******************************************************************************/

static void supermatcher_removelists(void **x,void *cl)
{
    concat *p;

    (void) cl;				/* make it used */

    p = (concat *)*x;

    ajListFree(&(p)->list);
    AJFREE(p);

    return;
}




/* @funcstatic supermatcher_findmax *******************************************
**
** Undocumented.
**
** @param [r] x [void**] Undocumented
** @param [r] cl [void*] Undocumented
** @return [void]
** @@
******************************************************************************/

static void supermatcher_findmax(void **x,void *cl)
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




/* @funcstatic supermatcher_findstartpoints ***********************************
**
** Undocumented.
**
** @param [w] seq1MatchTable [AjPTable] match table
** @param [r] b [const AjPSeq] second sequence
** @param [r] a [const AjPSeq] first sequence
** @param [w] start1 [ajint*] start in sequence 1
** @param [w] start2 [ajint*] start in sequence 2
** @param [w] end1 [ajint*] end in sequence 1
** @param [w] end2 [ajint*] end in sequence 2
** @return [ajint] Undocumented
** @@
******************************************************************************/

static ajint supermatcher_findstartpoints(AjPTable seq1MatchTable,
					  const AjPSeq b,
					  const AjPSeq a, ajint *start1,
					  ajint *start2, ajint *end1,
					  ajint *end2)
{
    ajint max = -10;
    ajint offset = 0;
    AjPList matchlist = NULL;
    AjPList ordered = NULL;
    ajint amax;
    ajint bmax;
    ajint bega;
    ajint begb;

    amax = ajSeqGetLen(a)-1;
    bmax = ajSeqGetLen(b)-1;
    bega = ajSeqGetOffset(a);
    begb = ajSeqGetOffset(b);


    ajDebug("supermatcher_findstartpoints len %d %d off %d %d\n",
	     amax, bmax, bega, begb);
    matchlist = embWordBuildMatchTable(seq1MatchTable, b, ajTrue);

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

    supermatcher_orderandconcat(matchlist, ordered);

    ajListMap(ordered,supermatcher_findmax, &max);

    ajDebug("findstart conmax off:%d count:%d total:%d\n",
	    conmax->offset, conmax->count, conmax->total,
	    ajListGetLength(conmax->list));
    offset = conmax->offset;

    ajListMap(ordered,supermatcher_removelists, NULL);
    ajListFree(&ordered);
    embWordMatchListDelete(&matchlist);	/* free the match structures */


    if(offset > 0)
    {
	*start1 = offset;
	*start2 = 0;
    }
    else
    {
	*start2 = 0-offset;
	*start1 = 0;
    }
    *end1 = *start1;
    *end2 = *start2;

    ajDebug("++ end1 %d -> %d end2 %d -> %d\n", *end1, amax, *end2, bmax);
    while(*end1<amax && *end2<bmax)
    {
	(*end1)++;
	(*end2)++;
    }

    ajDebug("++ end1 %d end2 %d\n", *end1, *end2);
    
    
    ajDebug("supermatcher_findstartpoints has %d..%d [%d] %d..%d [%d]\n",
	    *start1, *end1, ajSeqGetLen(a), *start2, *end2, ajSeqGetLen(b));

    return 1;
}

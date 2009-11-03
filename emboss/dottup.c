/* @source dottup application
**
** Dotplot of two sequences
**
** @author Copyright (C) Ian Longden
** @modified: Alan Bleasby. Added non-proportional plot
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




static void dottup_drawPlotlines(void *x, void *cl);
static void dottup_plotMatches(const AjPList list);
static void dottup_stretchplot(AjPGraph graph, const AjPList matchlist,
			       const AjPSeq seq1, const AjPSeq seq2,
			       ajint begin1, ajint begin2,
			       ajint end1, ajint end2);

static PLFLT offset1;
static PLFLT offset2;


/* @prog dottup ***************************************************************
**
** Displays a wordmatch dotplot of two sequences
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeq seq1;
    AjPSeq seq2;
    ajint wordlen;
    AjPTable seq1MatchTable = 0;
    AjPList matchlist = NULL;
    AjPGraph graph    = NULL;
    AjPGraph xygraph  = NULL;
    AjBool boxit;
    /*
    ** Different ticks as they need to be different for x and y due to
    ** length of string being important on x
    */
    ajuint acceptableticksx[]=
    {
	1,10,50,100,500,1000,1500,10000,
	500000,1000000,5000000
    };
    ajuint acceptableticks[]=
    {
	1,10,50,100,200,500,1000,2000,5000,10000,15000,
	500000,1000000,5000000
    };
    ajint numbofticks = 10;
    float xmargin;
    float ymargin;
    float ticklen;
    float tickgap;
    float onefifth = 0.0;
    ajint i;
    float k;
    float max;
    char ptr[10];
    ajint begin1;
    ajint begin2;
    ajint end1;
    ajint end2;
    ajuint len1;
    ajuint len2;
    float fbegin1;
    float fbegin2;
    float fend1;
    float fend2;
    float flen1;
    float flen2;
    AjBool stretch;
    
    ajGraphInit("dottup", argc, argv);

    wordlen = ajAcdGetInt("wordsize");
    seq1    = ajAcdGetSeq("asequence");
    seq2    = ajAcdGetSeq("bsequence");
    graph   = ajAcdGetGraph("graph");
    boxit   = ajAcdGetBoolean("boxit");
    stretch = ajAcdGetToggle("stretch");
    xygraph = ajAcdGetGraphxy("xygraph");

    begin1 = ajSeqGetBegin(seq1);
    begin2 = ajSeqGetBegin(seq2);
    end1   = ajSeqGetEnd(seq1);
    end2   = ajSeqGetEnd(seq2);
    len1   = end1 - begin1 + 1;
    len2   = end2 - begin2 + 1;

    flen1  = (float) len1;
    flen2  = (float) len2;
    fbegin1 = (float) begin1;
    fbegin2 = (float) begin2;
    fend1   = (float) end1;
    fend2   = (float) end2;

    offset1 = fbegin1;
    offset2 = fbegin2;

    ajSeqTrim(seq1);
    ajSeqTrim(seq2);

    embWordLength(wordlen);
    if(embWordGetTable(&seq1MatchTable, seq1))
	matchlist = embWordBuildMatchTable(seq1MatchTable, seq2, ajTrue);


    if(stretch)
    {
	dottup_stretchplot(xygraph,matchlist,seq1,seq2,begin1,begin2,end1,
			   end2);
	if(matchlist)
	    embWordMatchListDelete(&matchlist); /* free the match structures */
    }

    else
    {
	/* only here if stretch is false */

	max= flen1;
	if(flen2 > max)
	    max = flen2;

	xmargin = ymargin = max * (float)0.15;

	ajGraphOpenWin(graph, fbegin1-ymargin,fend1+ymargin,
		       fbegin2-xmargin,(float)fend2+xmargin);

	ajGraphSetCharScale(0.5);

	if(matchlist)
	    dottup_plotMatches(matchlist);

	if(boxit)
	{
	    ajGraphRect(fbegin1, fbegin2, fend1, fend2);
	    i = 0;
	    while(acceptableticksx[i]*numbofticks < len1)
		i++;

	    if(i<=13)
		tickgap = (float) acceptableticksx[i];
	    else
		tickgap = (float) acceptableticksx[10];

	    ticklen = xmargin*0.1;
	    onefifth  = xmargin*0.2;
	    ajGraphTextMid(fbegin1+flen1*0.5,fbegin1-(onefifth*3.0),
			   ajGraphGetYTitleC(graph));

	    if(len2/len1 > 10 )
	    {
		/* a lot smaller then just label start and end */
		ajGraphLine(fbegin1,fbegin2,fbegin1,
			    fbegin2-ticklen);
		sprintf(ptr,"%d",ajSeqGetOffset(seq1));
		ajGraphTextMid(fbegin1,fbegin2-(onefifth),ptr);
		
		ajGraphLine(fend1,fbegin2,
			    fend1,fbegin2-ticklen);
		sprintf(ptr,"%d",end1);
		ajGraphTextMid(fend1,fbegin2-(onefifth),ptr);
	    }
	    else
		for(k=fbegin1;k<fend1;k+=tickgap)
		{
		    ajGraphLine(k,fbegin2,k,fbegin2-ticklen);
		    sprintf(ptr,"%d",(ajint)k);
		    ajGraphTextMid( k,fbegin2-(onefifth),ptr);
		}

	    i = 0;
	    while(acceptableticks[i]*numbofticks < len2)
		i++;

	    tickgap   = (float) acceptableticks[i];
	    ticklen   = ymargin*(float)0.1;
	    onefifth  = ymargin*(float)0.2;
	    ajGraphTextLine(fbegin1-(onefifth*(float)4.),
			    fbegin2+flen2*(float)0.5,
			    fbegin2-(onefifth*(float)4.),
			    fbegin2+flen2,
			    ajGraphGetXTitleC(graph),0.5);

	    if(len1/len2 > 10 )
	    {
		/* a lot smaller then just label start and end */
		ajGraphLine(fbegin1,fbegin2,fbegin1-ticklen,
			    fbegin2);
		sprintf(ptr,"%d",ajSeqGetOffset(seq2));
		ajGraphTextEnd(fbegin1-(onefifth),fbegin2,ptr);

		ajGraphLine(fbegin1,fend2,fbegin1-ticklen,
			    fend2);
		sprintf(ptr,"%d",end2);
		ajGraphTextEnd(fbegin2-(onefifth),fend2,ptr);
	    }
	    else
		for(k=fbegin2;k<fend2;k+=tickgap)
		{
		    ajGraphLine(fbegin1,k,fbegin1-ticklen,k);
		    sprintf(ptr,"%d",(ajint)k);
		    ajGraphTextEnd(fbegin1-(onefifth),k,ptr);
		}
	}
    }

    ajGraphClose();
    ajSeqDel(&seq1);
    ajSeqDel(&seq2);
    ajGraphxyDel(&graph);
    ajGraphxyDel(&xygraph);

    embWordFreeTable(&seq1MatchTable);

    if(matchlist)
	embWordMatchListDelete(&matchlist); /* free the match structures */

    embExit();

    return 0;
}




#ifndef NO_PLOT

/* @funcstatic dottup_drawPlotlines *******************************************
**
** Undocumented.
**
** @param [r] x [void*] Undocumented
** @param [r] cl [void*] Undocumented
** @return [void]
** @@
******************************************************************************/

static void dottup_drawPlotlines(void *x, void *cl)
{
    EmbPWordMatch p;
    PLFLT x1;
    PLFLT y1;
    PLFLT x2;
    PLFLT y2;

    (void) cl;				/* make it used */

    p  = (EmbPWordMatch)x;

    x1 = x2 = offset1 + (PLFLT)(p->seq1start);
    y1 = y2 = offset2 + (PLFLT)(p->seq2start);

    x2 += (PLFLT)p->length -1.0;
    y2 += (PLFLT)p->length -1.0;

    ajGraphLine(x1, y1, x2, y2);

    return;
}




/* @funcstatic dottup_plotMatches *********************************************
**
** Undocumented.
**
** @param [r] list [const AjPList] Undocumented
** @return [void]
** @@
******************************************************************************/

static void dottup_plotMatches(const AjPList list)
{
    ajListMapread(list,dottup_drawPlotlines, NULL);

    return;
}

#endif




/* @funcstatic dottup_stretchplot *********************************************
**
** Undocumented.
**
** @param [u] graph [AjPGraph] Undocumented
** @param [r] matchlist [const AjPList] Undocumented
** @param [r] seq1 [const AjPSeq] Undocumented
** @param [r] seq2 [const AjPSeq] Undocumented
** @param [r] begin1 [ajint] Undocumented
** @param [r] begin2 [ajint] Undocumented
** @param [r] end1 [ajint] Undocumented
** @param [r] end2 [ajint] Undocumented
** @return [void]
** @@
******************************************************************************/

static void dottup_stretchplot(AjPGraph graph, const AjPList matchlist,
			       const AjPSeq seq1, const AjPSeq seq2,
			       ajint begin1, ajint begin2,
			       ajint end1, ajint end2)
{
    EmbPWordMatch wmp = NULL;
    float xa[1];
    float ya[2];
    AjPGraphPlpData gdata = NULL;
    AjPStr tit = NULL;
    float x1;
    float y1;
    float x2;
    float y2;
    AjIList iter = NULL;

    tit = ajStrNew();
    ajFmtPrintS(&tit,"%S",ajGraphGetTitle(graph));


    gdata = ajGraphPlpDataNewI(1);
    xa[0] = (float)begin1;
    ya[0] = (float)begin2;

    ajGraphSetTitleC(graph,ajStrGetPtr(tit));

    ajGraphSetXTitleC(graph,ajSeqGetNameC(seq1));
    ajGraphSetYTitleC(graph,ajSeqGetNameC(seq2));

    ajGraphPlpDataSetTypeC(gdata,"2D Plot Float");
    ajGraphPlpDataSetMaxMin(gdata,(float)begin1,(float)end1,(float)begin2,
			   (float)end2);
    ajGraphPlpDataSetMaxima(gdata,(float)begin1,(float)end1,(float)begin2,
			   (float)end2);
    ajGraphxySetXStart(graph,(float)begin1);
    ajGraphxySetXEnd(graph,(float)end1);
    ajGraphxySetYStart(graph,(float)begin2);
    ajGraphxySetYEnd(graph,(float)end2);

    ajGraphxySetXRangeII(graph,begin1,end1);
    ajGraphxySetYRangeII(graph,begin2,end2);


    if(matchlist)
    {
	iter = ajListIterNewread(matchlist);
	while((wmp = ajListIterGet(iter)))
	{
	    x1 = x2 = (float) (wmp->seq1start + begin1);
	    y1 = y2 = (float) (wmp->seq2start + begin2);
	    x2 += (float) wmp->length-1;
	    y2 += (float) wmp->length-1;
	    ajGraphAddLine(graph,x1,y1,x2,y2,0);
	}
	ajListIterDel(&iter);
    }

    ajGraphPlpDataSetXY(gdata,xa,ya);
    ajGraphDataReplace(graph,gdata);


    ajGraphxyDisplay(graph,ajFalse);
    ajGraphClose();

    ajStrDel(&tit);

    return;
}

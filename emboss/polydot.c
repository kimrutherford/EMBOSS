/* @source polydot application
**
** Create a polydot from a multiple sequence file.
** @author Copyright (C) Peter Rice (pmr@ebi.ac.uk)
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
#include "ajgraph.h"

static float xstart = 0;
static float ystart = 0;
static ajint *lines;
static ajint *pts;
static ajint which;




static void polydot_drawPlotlines(void *x, void *cl);
static void polydot_plotMatches(const AjPList list);




/* @prog polydot **************************************************************
**
** Displays all-against-all dotplots of a set of sequences
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPSeqset seqset;
    const AjPSeq seq1;
    const AjPSeq seq2;
    ajint wordlen;
    AjPTable seq1MatchTable = NULL;
    AjPList matchlist ;
    AjPGraph graph = 0;
    ajuint i;
    ajuint j;
    float total=0;
    ajuint acceptableticks[]=
    {
	1,10,50,100,200,500,1000,1500,10000,50000,
	100000,500000,1000000,5000000
    };
    ajint numbofticks = 10;
    ajint gap,tickgap;
    AjBool boxit    = AJTRUE;
    AjBool dumpfeat = AJFALSE;
    float xmargin;
    float ymargin;
    float k;
    char ptr[10];
    float ticklen;
    float onefifth;
    AjPFeattable *tabptr = NULL;
    AjPFeattabOut seq1out = NULL;
    AjPStr sajb = NULL;
    float flen1;
    float flen2;
    ajuint tui;
    
    ajGraphInit("polydot", argc, argv);

    wordlen  = ajAcdGetInt("wordsize");
    seqset   = ajAcdGetSeqset("sequences");
    graph    = ajAcdGetGraph("graph");
    gap      = ajAcdGetInt("gap");
    boxit    = ajAcdGetBool("boxit");
    seq1out  = ajAcdGetFeatout("outfeat");
    dumpfeat = ajAcdGetToggle("dumpfeat");

    sajb = ajStrNew();
    embWordLength(wordlen);

    AJCNEW(lines,ajSeqsetGetSize(seqset));
    AJCNEW(pts,ajSeqsetGetSize(seqset));
    AJCNEW(tabptr,ajSeqsetGetSize(seqset));

    for(i=0;i<ajSeqsetGetSize(seqset);i++)
    {
	seq1 = ajSeqsetGetseqSeq(seqset, i);
	total += ajSeqGetLen(seq1);

    }
    
    total +=(float)(gap*(ajSeqsetGetSize(seqset)-1));
    
    xmargin = total*(float)0.15;
    ymargin = total*(float)0.15;
    
    ticklen = xmargin*(float)0.1;
    onefifth  = xmargin*(float)0.2;
    
    i = 0;
    while(acceptableticks[i]*numbofticks < ajSeqsetGetLen(seqset))
	i++;
    
    if(i<=13)
	tickgap = acceptableticks[i];
    else
	tickgap = acceptableticks[13];
    
    ajGraphSetTitlePlus(graph, ajSeqsetGetUsa(seqset));

    ajGraphOpenWin(graph, (float)0.0-xmargin,(total+xmargin)*(float)1.35,
		   (float)0.0-ymargin,
		   total+ymargin);
    ajGraphSetCharScale((float)0.3);
    
    
    for(i=0;i<ajSeqsetGetSize(seqset);i++)
    {
	which = i;
	seq1 = ajSeqsetGetseqSeq(seqset, i);
	tui = ajSeqGetLen(seq1);
	flen1 = (float) tui;

	if(embWordGetTable(&seq1MatchTable, seq1)){ /* get table of words */
	    for(j=0;j<ajSeqsetGetSize(seqset);j++)
	    {
		seq2 = ajSeqsetGetseqSeq(seqset, j);
		tui  = ajSeqGetLen(seq2);
		flen2 = (float) tui;

		if(boxit)
		    ajGraphRect(xstart,ystart,
				xstart+flen1,
				ystart+flen2);

		matchlist = embWordBuildMatchTable(seq1MatchTable, seq2,
						   ajTrue);
		if(matchlist)
		    polydot_plotMatches(matchlist);

		if(i<j && dumpfeat)
		    embWordMatchListConvToFeat(matchlist,&tabptr[i],
					       &tabptr[j],seq1,
					       seq2);

		if(matchlist)	       /* free the match structures */
		    embWordMatchListDelete(&matchlist);

		if(j==0)
		{
		    for(k=0.0;k<ajSeqGetLen(seq1);k+=tickgap)
		    {
			ajGraphLine(xstart+k,ystart,xstart+k,
				    ystart-ticklen);

			sprintf(ptr,"%d",(ajint)k);
			ajGraphTextMid(xstart+k,ystart-(onefifth),ptr);
		    }
		    ajGraphTextMid(xstart+(flen1/(float)2.0),
				   ystart-(3*onefifth),
				   ajStrGetPtr(ajSeqsetGetseqNameS(seqset, i)));
		}

		if(i==0)
		{
		    for(k=0.0;k<ajSeqGetLen(seq2);k+=tickgap)
		    {
			ajGraphLine(xstart,ystart+k,xstart-ticklen,
				    ystart+k);

			sprintf(ptr,"%d",(ajint)k);
			ajGraphTextEnd(xstart-(onefifth),ystart+k,ptr);
		    }
		    ajGraphTextLine(xstart-(3*onefifth),
				    ystart+(flen2/(float)2.0),
				    xstart-(3*onefifth),ystart+flen2,
				    ajStrGetPtr(ajSeqsetGetseqNameS(seqset, j)),0.5);
		}
		ystart += flen2+(float)gap;
	    }
	}
	embWordFreeTable(&seq1MatchTable);
	seq1MatchTable = NULL;
	xstart += flen1+(float)gap;
	ystart = 0.0;
    }
    
    ajGraphTextStart(total+onefifth,total-(onefifth),
		     "No. Length  Lines  Points Sequence");
    
    for(i=0;i<ajSeqsetGetSize(seqset);i++)
    {
	seq1 = ajSeqsetGetseqSeq(seqset, i);
	ajFmtPrintS(&sajb,"%3u %6d %5d %6d %s",i+1,
		    ajSeqGetLen(seq1),lines[i],
		    pts[i],ajSeqGetNameC(seq1));

	ajGraphTextStart(total+onefifth,total-(onefifth*(i+2)),
			 ajStrGetPtr(sajb));
    }
    
    if(dumpfeat && seq1out)
    {
	for(i=0;i<ajSeqsetGetSize(seqset);i++)
	{
	    ajFeatWrite(seq1out, tabptr[i]);
	    ajFeattableDel(&tabptr[i]);
	}
    }
    
    ajGraphClose();
    ajGraphxyDel(&graph);

    ajStrDel(&sajb);
    AJFREE(lines);
    AJFREE(pts);
    AJFREE(tabptr);

    ajSeqsetDel(&seqset);
    ajFeattabOutDel(&seq1out);;

    embExit();

    return 0;
}




/* @funcstatic polydot_drawPlotlines ******************************************
**
** Undocumented.
**
** @param [r] x [void*] Undocumented
** @param [r] cl [void*] Undocumented
** @return [void]
** @@
******************************************************************************/

static void polydot_drawPlotlines(void *x, void *cl)
{
    EmbPWordMatch p;
    PLFLT x1;
    PLFLT y1;
    PLFLT x2;
    PLFLT y2;

    (void) cl;				/*make it used */

    p = (EmbPWordMatch)x;

    lines[which]++;
    pts[which]+= p->length;
    x1 = x2 = (p->seq1start)+xstart;
    y1 = y2 = (PLFLT)(p->seq2start)+ystart;
    x2 += p->length;
    y2 += (PLFLT)p->length;

    ajGraphLine(x1, y1, x2, y2);

    return;
}




/* @funcstatic polydot_plotMatches ********************************************
**
** Undocumented.
**
** @param [r] list [const AjPList] Undocumented
** @return [void]
** @@
******************************************************************************/

static void polydot_plotMatches(const AjPList list)
{
    ajListMapread(list,polydot_drawPlotlines, NULL);

    return;
}

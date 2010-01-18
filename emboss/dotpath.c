/* @source dotpath application
**
** Highlight best non-overlapping set of word matches in dotplot
**
** Heavily based on the application 'dottup'.
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




static void dotpath_drawPlotlines(void *x, void *cl);
static void dotpath_plotMatches(const AjPList list);




/* @prog dotpath **************************************************************
**
** Displays a non-overlapping wordmatch dotplot of two sequences
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeq seq1;
    AjPSeq seq2;
    ajint wordlen;
    AjPTable seq1MatchTable = 0;
    AjPList matchlist = NULL;
    AjPGraph graph = 0;
    AjBool boxit;
    AjBool overlaps;

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
    ajint oldcolour = -1;
    ajuint len1;
    ajuint len2;
    float flen1;
    float flen2;
    ajuint tui;
    
    embInit("dotpath", argc, argv);
    
    wordlen  = ajAcdGetInt("wordsize");
    seq1     = ajAcdGetSeq("asequence");
    seq2     = ajAcdGetSeq("bsequence");
    overlaps = ajAcdGetBoolean("overlaps");
    graph    = ajAcdGetGraph("graph");
    boxit    = ajAcdGetBoolean("boxit");
    
    len1 = ajSeqGetLen(seq1);
    len2 = ajSeqGetLen(seq2);
    tui = ajSeqGetLen(seq1);
    flen1 = (float) tui;
    tui = ajSeqGetLen(seq2);
    flen2 = (float) tui;

    ajSeqTrim(seq1);
    ajSeqTrim(seq2);
    
    
    embWordLength(wordlen);
    if(embWordGetTable(&seq1MatchTable, seq1))
    {					/* get table of words */
	matchlist = embWordBuildMatchTable(seq1MatchTable, seq2, ajTrue);
    }
    
    max = flen1;
    if(flen2 > max)
	max = flen2;
    
    xmargin = ymargin = max * (float)0.15;
    
    ajGraphOpenWin(graph, (float)0.0-ymargin,(max*(float)1.35)+ymargin,
		   (float)0.0-xmargin,(float)max+xmargin);
    
    ajGraphicsSetCharscale(0.5);
    
    /* display the overlapping matches in red */
    if(overlaps && ajListGetLength(matchlist))
    {
	oldcolour = ajGraphicsSetFgcolour(RED);
	dotpath_plotMatches(matchlist);
	ajGraphicsSetFgcolour(oldcolour);  /* restore colour we were using */
    }
    
    /* get the minimal set of overlapping matches */
    embWordMatchMin(matchlist);
    
    
    
    /* display them */
    if(ajListGetLength(matchlist))
	dotpath_plotMatches(matchlist);
    
    if(boxit)
    {
	ajGraphicsDrawposRect( 0.0,0.0,flen1,flen2);
	i = 0;
	while(acceptableticksx[i]*numbofticks < flen1)
	    i++;

	if(i<=11)
	    tickgap = (float) acceptableticksx[i];
	else
	    tickgap = (float) acceptableticksx[10];

	ticklen = xmargin*(float)0.1;
	onefifth  = xmargin*(float)0.2;
	ajGraphicsDrawposTextAtmid(flen1*(float)0.5,
                                   (float)0.0-(onefifth*(float)3.),
                                   ajGraphGetYlabelC(graph));

	if(len2/len1 > 10 )
	{
	    /* a lot smaller then just label start and end */
	    ajGraphicsDrawposLine((float)0.0,(float)0.0,
                                  (float)0.0,(float)0.0-ticklen);
	    sprintf(ptr,"%d",ajSeqGetOffset(seq1));
	    ajGraphicsDrawposTextAtmid((float)0.0,(float)0.0-(onefifth),ptr);

	    ajGraphicsDrawposLine(flen1,(float)0.0,
			flen1,(float)0.0-ticklen);
	    sprintf(ptr,"%d",len1+ajSeqGetOffset(seq1));
	    ajGraphicsDrawposTextAtmid(flen1,(float)0.0-(onefifth),ptr);
	}
	else
	    for(k=0.0;k<len1;k+=tickgap)
	    {
		ajGraphicsDrawposLine(k,(float)0.0,k,(float)0.0-ticklen);
		sprintf(ptr,"%d",(ajint)k+ajSeqGetOffset(seq1));
		ajGraphicsDrawposTextAtmid( k,(float)0.0-(onefifth),ptr);
	    }

	i = 0;
	while(acceptableticks[i]*numbofticks < len2)
	    i++;

	tickgap   = (float) acceptableticks[i];
	ticklen   = ymargin*(float)0.1;
	onefifth  = ymargin*(float)0.2;
	ajGraphicsDrawposTextAtlineJustify((float)0.0-(onefifth*(float)4.),
                                           flen2*(float)0.5,
                                           (float)0.0-(onefifth*(float)4.),
                                           flen2,
                                           ajGraphGetXlabelC(graph),
                                           (float)0.5);

	if(len1/len2 > 10 )
	{
	    /* a lot smaller then just label start and end */
	    ajGraphicsDrawposLine((float)0.0,(float)0.0,
                                  (float)0.0-ticklen,(float)0.0);
	    sprintf(ptr,"%d",ajSeqGetOffset(seq2));
	    ajGraphicsDrawposTextAtend( (float)0.0-(onefifth),(float)0.0,ptr);

	    ajGraphicsDrawposLine((float)0.0,flen2,
			(float)0.0-ticklen,flen2);
	    sprintf(ptr,"%d",len2+ajSeqGetOffset(seq2));
	    ajGraphicsDrawposTextAtend((float)0.0-(onefifth),flen2,ptr);
	}
	else
	    for(k=0.0;k<len2;k+=tickgap)
	    {
		ajGraphicsDrawposLine((float)0.0,k,(float)0.0-ticklen,k);
		sprintf(ptr,"%d",(ajint)k+ajSeqGetOffset(seq2));
		ajGraphicsDrawposTextAtend( (float)0.0-(onefifth),k,ptr);
	    }
    }

    ajGraphicsClose();

    embWordFreeTable(&seq1MatchTable);
    if(matchlist)
	embWordMatchListDelete(&matchlist);

    ajSeqDel(&seq1);
    ajSeqDel(&seq2);

    ajGraphxyDel(&graph);

    embExit();
    
    return 0;
}




#ifndef NO_PLOT

/* @funcstatic dotpath_drawPlotlines ******************************************
**
** Undocumented.
**
** @param [r] x [void*] Undocumented
** @param [r] cl [void*] Undocumented
** @return [void]
** @@
******************************************************************************/

static void dotpath_drawPlotlines(void *x, void *cl)
{
    EmbPWordMatch p;
    PLFLT x1;
    PLFLT y1;
    PLFLT x2;
    PLFLT y2;

    (void) cl;				/* make it used */

    p  = (EmbPWordMatch)x;

    x1 = x2 = (PLFLT)((p->seq1start)+1);
    y1 = y2 = (PLFLT)((p->seq2start)+1);

    x2 += p->length;
    y2 += (PLFLT)p->length;

    ajGraphicsDrawposLine(x1, y1, x2, y2);

    return;
}




/* @funcstatic dotpath_plotMatches ********************************************
**
** Undocumented.
**
** @param [r] list [const AjPList] Undocumented
** @return [void]
** @@
******************************************************************************/


static void dotpath_plotMatches(const AjPList list)
{
    ajListMapread(list,dotpath_drawPlotlines, NULL);

    return;
}

#endif

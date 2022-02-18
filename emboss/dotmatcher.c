/* @source dotmatcher application
**
** dotmatcher displays a dotplot for two sequences.
**
** @author Copyright (C) Ian Longden (il@sanger.ac.uk)
** @modified: Added non-proportional plot. Copyright (C) Alan Bleasby
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




static void dotmatcher_pushpoint(AjPList *l, float x1, float y1, float x2,
				 float y2, AjBool stretch);




/* @datastatic PPoint *********************************************************
**
** Dotmatcher point data
**
** @alias SPoint
** @alias OPoint
**
** @attr x1 [float] x1 coordinate
** @attr y1 [float] y1 coordinate
** @attr x2 [float] x2 coordinate
** @attr y2 [float] y2 coordinate
******************************************************************************/

typedef struct SPoint
{
    float x1;
    float y1;
    float x2;
    float y2;
} OPoint;
#define PPoint OPoint*




/* @prog dotmatcher ***********************************************************
**
** Displays a thresholded dotplot of two sequences
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPList list = NULL;
    AjPSeq seq;
    AjPSeq seq2;
    AjPStr aa0str = 0;
    AjPStr aa1str = 0;
    const char *s1;
    const char *s2;
    char *strret = NULL;
    ajuint i;
    ajuint j;
    ajuint k;
    ajint l;
    ajint abovethresh;
    ajint total;
    ajint starti = 0;
    ajint startj = 0;
    ajint windowsize;
    float thresh;
    AjPGraph graph   = NULL;
    AjPGraph xygraph = NULL;
    float flen1;
    float flen2;
    ajuint len1;
    ajuint len2;

    AjPTime ajtime = NULL;
    time_t tim;
    AjBool boxit=AJTRUE;
    /* Different ticks as they need to be different for x and y due to
       length of string being important on x */
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
    float onefifth;
    float k2;
    float max;
    char ptr[10];
    AjPMatrix matrix = NULL;
    ajint** sub;
    AjPSeqCvt cvt;
    AjPStr  subt = NULL;

    ajint b1;
    ajint b2;
    ajint e1;
    ajint e2;
    AjPStr se1;
    AjPStr se2;
    ajint ithresh;
    AjBool stretch;
    PPoint ppt = NULL;
    float xa[1];
    float ya[1];
    AjPGraphdata gdata=NULL;
    AjPStr tit   = NULL;
    AjIList iter = NULL;
    float x1 = 0.;
    float x2 = 0.;
    float y1 = 0.;
    float y2 = 0.;
    ajuint tui;
    
    se1 = ajStrNew();
    se2 = ajStrNew();

    embInit("dotmatcher", argc, argv);
    
    seq       = ajAcdGetSeq("asequence");
    seq2        = ajAcdGetSeq("bsequence");
    stretch    = ajAcdGetToggle("stretch");
    graph      = ajAcdGetGraph("graph");
    xygraph    = ajAcdGetGraphxy("xygraph");
    windowsize = ajAcdGetInt("windowsize");
    ithresh    = ajAcdGetInt("threshold");
    matrix     = ajAcdGetMatrix("matrixfile");
    
    sub = ajMatrixGetMatrix(matrix);
    cvt = ajMatrixGetCvt(matrix);
    
    thresh = (float)ithresh;

    ajtime = ajTimeNew();

    tim = time(0);
    ajTimeSetLocal(ajtime, tim);
    
    b1 = ajSeqGetBegin(seq);
    b2 = ajSeqGetBegin(seq2);
    e1 = ajSeqGetEnd(seq);
    e2 = ajSeqGetEnd(seq2);
    len1 = ajSeqGetLen(seq);
    len2 = ajSeqGetLen(seq2);
    tui   = ajSeqGetLen(seq);
    flen1 = (float) tui;
    tui   = ajSeqGetLen(seq2);
    flen2 = (float) tui;
    
    ajStrAssignSubC(&se1,ajSeqGetSeqC(seq),b1-1,e1-1);
    ajStrAssignSubC(&se2,ajSeqGetSeqC(seq2),b2-1,e2-1);
    ajSeqAssignSeqS(seq,se1);
    ajSeqAssignSeqS(seq2,se2);
    
    
    s1 = ajStrGetPtr(ajSeqGetSeqS(seq));
    s2 = ajStrGetPtr(ajSeqGetSeqS(seq2));
    
    
    aa0str = ajStrNewRes(1+len1); /* length plus trailing blank */
    aa1str = ajStrNewRes(1+len2);
    
    list = ajListNew();
    
    
    for(i=0;i<len1;i++)
	ajStrAppendK(&aa0str,(char)ajSeqcvtGetCodeK(cvt, *s1++));
    
    for(i=0;i<len2;i++)
	ajStrAppendK(&aa1str,(char)ajSeqcvtGetCodeK(cvt, *s2++));
    
    max = (float)len1;
    if(len2 > max)
	max = (float) len2;
    
    xmargin = ymargin = max *(float)0.15;
    ticklen = xmargin*(float)0.1;
    onefifth  = xmargin*(float)0.2;
    
    subt = ajStrNewC((strret=
		      ajFmtString("(windowsize = %d, threshold = %3.2f  %D)",
				  windowsize,thresh,ajtime)));
    
    

    if(!stretch)
    {
	if( ajStrGetLen(ajGraphGetSubtitleS(graph)) <=1)
	    ajGraphSetSubtitleS(graph,subt);

	ajGraphOpenWin(graph, (float)0.0-ymargin,(max*(float)1.35)+ymargin,
		       (float)0.0-xmargin,(float)max+xmargin);

	ajGraphicsDrawposTextAtmid(flen1*(float)0.5,
                                   (float)0.0-(xmargin/(float)2.0),
		       ajGraphGetXlabelC(graph));
	ajGraphicsDrawposTextAtlineJustify((float)0.0-(xmargin*(float)0.75),
                                           flen2*(float)0.5,
			(float)0.0-(xmargin*(float)0.75),flen1,
			ajGraphGetYlabelC(graph),0.5);

	ajGraphicsSetCharscale(0.5);
    }
    
    
    
    s1= ajStrGetPtr(aa0str);
    s2 = ajStrGetPtr(aa1str);
    
    for(j=0; (ajint)j < (ajint)len2-windowsize;j++)
    {
	i =0;
	total = 0;
	abovethresh =0;

	k = j;
	for(l=0;l<windowsize;l++)
	    total = total + sub[(ajint)s1[i++]][(ajint)s2[k++]];

	if(total >= thresh)
	{
	    abovethresh=1;
	    starti = i-windowsize;
	    startj = k-windowsize;
	}

	while(i < len1 && k < len2)
	{
	    total = total - sub[(ajint)s1[i-windowsize]]
		[(ajint)s2[k-windowsize]];
	    total = total + sub[(ajint)s1[i]][(ajint)s2[k]];

	    if(abovethresh)
	    {
		if(total < thresh)
		{
		    abovethresh = 0;
		    /* draw the line */
		    dotmatcher_pushpoint(&list,(float)starti,(float)startj,
					 (float)i-1,(float)k-1,stretch);
		}
	    }
	    else if(total >= thresh)
	    {
		starti = i-windowsize;
		startj = k-windowsize;
		abovethresh= 1;
	    }
	    i++;
	    k++;
	}

	if(abovethresh)
	    /* draw the line */
	    dotmatcher_pushpoint(&list,(float)starti,(float)startj,
				 (float)i-1,(float)k-1,
				 stretch);
    }
    
    for(i=0; (ajint)i < (ajint)len1-windowsize;i++)
    {
	j = 0;
	total = 0;
	abovethresh =0;

	k = i;
	for(l=0;l<windowsize;l++)
	    total = total + sub[(ajint)s1[k++]][(ajint)s2[j++]];

	if(total >= thresh)
	{
	    abovethresh=1;
	    starti = k-windowsize;
	    startj = j-windowsize;
	}

	while(k < len1 && j < len2)
	{
	    total = total - sub[(ajint)s1[k-windowsize]]
		[(ajint)s2[j-windowsize]];
	    total = total + sub[(ajint)s1[k]][(ajint)s2[j]];

	    if(abovethresh)
	    {
		if(total < thresh)
		{
		    abovethresh = 0;
		    /* draw the line */
		    dotmatcher_pushpoint(&list,(float)starti,(float)startj,
					 (float)k-1,(float)j-1,stretch);
		}
	    }
	    else if(total >= thresh)
	    {
		starti = k-windowsize;
		startj = j-windowsize;
		abovethresh= 1;
	    }
	    j++;
	    k++;
	}

	if(abovethresh)
	    /* draw the line */
	    dotmatcher_pushpoint(&list,(float)starti,(float)startj,
				 (float)k-1,(float)j-1,
				 stretch);
    }
    
    if(boxit && !stretch)
    {
	ajGraphicsDrawposRect(0.0,0.0,flen1, flen2);

	i=0;
	while(acceptableticksx[i]*numbofticks < len1)
	    i++;

	if(i<=13)
	    tickgap = (float)acceptableticksx[i];
	else
	    tickgap = (float)acceptableticksx[10];
	ticklen   = xmargin*(float)0.1;
	onefifth  = xmargin*(float)0.2;

	if(len2/len1 > 10 )
	{
	    /* if a lot smaller then just label start and end */
	    ajGraphicsDrawposLine((float)0.0,(float)0.0,(float)0.0,(float)0.0-ticklen);
	    sprintf(ptr,"%d",b1-1);
	    ajGraphicsDrawposTextAtmid((float)0.0,(float)0.0-(onefifth),ptr);

	    ajGraphicsDrawposLine(flen1,(float)0.0,
			flen1,(float)0.0-ticklen);
	    sprintf(ptr,"%u",len1+b1-1);
	    ajGraphicsDrawposTextAtmid(flen1,(float)0.0-(onefifth),ptr);

	}
	else
	    for(k2=0.0;k2<len1;k2+=tickgap)
	    {
		ajGraphicsDrawposLine(k2,(float)0.0,k2,(float)0.0-ticklen);
		sprintf(ptr,"%d",(ajint)k2+b1-1);
		ajGraphicsDrawposTextAtmid(k2,(float)0.0-(onefifth),ptr);
	    }

	i = 0;
	while(acceptableticks[i]*numbofticks < len2)
	    i++;

	tickgap   = (float)acceptableticks[i];
	ticklen   = ymargin*(float)0.01;
	onefifth  = ymargin*(float)0.02;

	if(len1/len2 > 10 )
	{
	    /* if a lot smaller then just label start and end */
	    ajGraphicsDrawposLine((float)0.0,(float)0.0,(float)0.0-ticklen,(float)0.0);
	    sprintf(ptr,"%d",b2-1);
	    ajGraphicsDrawposTextAtend((float)0.0-(onefifth),(float)0.0,ptr);

	    ajGraphicsDrawposLine((float)0.0,flen2,(float)0.0-ticklen,
			flen2);
	    sprintf(ptr,"%u",len2+b2-1);
	    ajGraphicsDrawposTextAtend((float)0.0-(onefifth),flen2,ptr);
	}
	else
	    for(k2=0.0;k2<len2;k2+=tickgap)
	    {
		ajGraphicsDrawposLine((float)0.0,k2,(float)0.0-ticklen,k2);
		sprintf(ptr,"%d",(ajint)k2+b2-1);
		ajGraphicsDrawposTextAtend((float)0.0-(onefifth),k2,ptr);
	    }
    }
    
    
    if(!stretch)
	ajGraphicsClose();
    else			/* the xy graph for -stretch */
    {
	tit = ajStrNew();
	ajFmtPrintS(&tit,"%S",ajGraphGetTitleS(xygraph));


	gdata = ajGraphdataNewI(1);
	xa[0] = (float)b1;
	ya[0] = (float)b2;

	ajGraphSetTitleC(xygraph,ajStrGetPtr(tit));

	ajGraphSetXlabelC(xygraph,ajSeqGetNameC(seq));
	ajGraphSetYlabelC(xygraph,ajSeqGetNameC(seq2));

	ajGraphdataSetTypeC(gdata,"2D Plot Float");
	ajGraphdataSetTitleS(gdata,subt);
	ajGraphdataSetMinmax(gdata,(float)b1,(float)e1,(float)b2,
			       (float)e2);
	ajGraphdataSetTruescale(gdata,(float)b1,(float)e1,(float)b2,
			       (float)e2);
	ajGraphxySetXstartF(xygraph,(float)b1);
	ajGraphxySetXendF(xygraph,(float)e1);
	ajGraphxySetYstartF(xygraph,(float)b2);
	ajGraphxySetYendF(xygraph,(float)e2);

	ajGraphxySetXrangeII(xygraph,b1,e1);
	ajGraphxySetYrangeII(xygraph,b2,e2);


	if(list)
	{
	    iter = ajListIterNewread(list);
	    while((ppt = ajListIterGet(iter)))
	    {
		x1 = ppt->x1+b1-1;
		y1 = ppt->y1+b2-1;
		x2 = ppt->x2+b1-1;
		y2 = ppt->y2+b2-1;
		ajGraphAddLine(xygraph,x1,y1,x2,y2,0);
		AJFREE(ppt);
	    }
	    ajListIterDel(&iter);
	}

	ajGraphdataAddXY(gdata,xa,ya);
	ajGraphDataReplace(xygraph,gdata);


	ajGraphxyDisplay(xygraph,ajFalse);
	ajGraphicsClose();

	ajStrDel(&tit);
    }
    
    
    
    ajListFree(&list);

    ajSeqDel(&seq);
    ajSeqDel(&seq2);
    ajGraphxyDel(&graph);
    ajGraphxyDel(&xygraph);
    ajMatrixDel(&matrix);
    ajTimeDel(&ajtime);
    
    /* deallocate memory */
    ajStrDel(&aa0str);
    ajStrDel(&aa1str);
    ajStrDel(&se1);
    ajStrDel(&se2);
    ajStrDel(&subt);

    AJFREE(strret);			/* created withing ajFmtString */
    
    embExit();

    return 0;
}




/* @funcstatic dotmatcher_pushpoint *******************************************
**
** Undocumented.
**
** @param [u] l [AjPList*] Undocumented
** @param [r] x1 [float] Undocumented
** @param [r] y1 [float] Undocumented
** @param [r] x2 [float] Undocumented
** @param [r] y2 [float] Undocumented
** @param [r] stretch [AjBool] Do a stretch plot
** @return [void]
** @@
******************************************************************************/

static void dotmatcher_pushpoint(AjPList *l, float x1, float y1, float x2,
				 float y2, AjBool stretch)
{
    PPoint p;

    if(!stretch)
    {
	ajGraphicsDrawposLine(x1+1,y1+1,x2+1,y2+1);
	return;
    }

    AJNEW0(p);
    p->x1 = x1+1;
    p->y1 = y1+1;
    p->x2 = x2+1;
    p->y2 = y2+1;
    ajListPush(*l,(void *)p);

    return;
}

/******************************************************************************
** @source AJAX GRAPH (ajax histogram) functions
**
** These functions control all aspects of AJAX histogram.
**
** @author Copyright (C) 1998 Peter Rice
** @version 1.0
** @modified 1988-11-12 pmr First version
** @modified 1999 ajb ANSIfication
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA  02111-1307, USA.
******************************************************************************/

#include "ajax.h"
#include "ajgraph.h"
#include "ajhist.h"
#include <limits.h>
#include <float.h>


ajint aj_hist_mark=GRAPH_HIST;

/* @filesection ajhist *******************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
*/



/* @datasection [none] Histogram internals ************************************
**
** Function is for controlling the histogram internals without using
** an AjPGraph object
**
** @nam2rule Histogram
*/




/* @section modifiers **********************************************************
**
** Controlling the internals
**
** @fdata [none]
** @fcategory modify
**
** @nam3rule Close Close a histogram plot
** @nam3rule Set Set internal state
** @nam4rule Mark Set the histogram mark internal value
**
** @argrule Mark mark [ajint] Mark value
**
** @valrule * [void]
**
******************************************************************************/

/* @func ajHistogramClose ******************************************************
**
** Closes the histograms window.
**
** @return [void]
**
** @@
******************************************************************************/

void ajHistogramClose(void)
{
    ajGraphicsCloseWin();

    return;
}


/* @obsolete ajHistClose
** @rename ajHistogramClose
*/
__deprecated void ajHistClose(void)
{
    ajHistogramClose();

    return;
}



/* @func ajHistogramSetMark ***************************************************
**
** Set the histogram mark internal value
**
** @param [r] mark [ajint] Mark value
** @return [void]
******************************************************************************/

void ajHistogramSetMark(ajint mark)
{
  aj_hist_mark = mark;
  return;
}

/* @obsolete ajHistSetMark
** @rename ajHistogramSetMark
*/
__deprecated void ajHistSetMark(ajint mark)
{
    ajHistogramSetMark(mark);
    return;
}


/* @datasection [AjPHist] Histogram object *************************************
**
** Function is for manipulating an AjPHist histogram object
**
** @nam2rule Hist
**
******************************************************************************/

/* @section Constructors ******************************************************
**
** Construct a new histogram object to be populated by other functions
**
** @fdata [AjPHist]
** @fcategory new
**
** @nam3rule New
** @suffix G New with multiple graphdata
**
** @argrule New numofsets [ajuint] Number of sets of data.
** @argrule New numofpoints [ajuint] Number of data points per set.
** @argrule G graph [AjPGraph] Graph object, device, multi and name are set
**
** @valrule * [AjPHist] New histogram object
**
******************************************************************************/

/* @func ajHistNew ************************************************************
**
** Create a histogram Object. Which can hold "numofsets" set of data of which
** all must have "numofpoints" data points in them.
**
** @param [r] numofsets [ajuint] Number of sets of data.
** @param [r] numofpoints [ajuint] Number of data points per set.
** @return [AjPHist] histogram structure.
** @@
******************************************************************************/

AjPHist ajHistNew(ajuint numofsets, ajuint numofpoints)
{
    static AjPHist hist = NULL;
    ajuint i;

    AJNEW0(hist);

    hist->numofsets = 0;
    hist->numofsetsmax    = numofsets;
    hist->numofdatapoints = numofpoints;
    hist->xmin = 0;
    hist->xmax = 0;
    hist->displaytype = HIST_SIDEBYSIDE; /* default draw
					    multiple histograms
					    side by side */
    hist->bins = 0;
    hist->BaW  = AJFALSE;
    ajStrAssignEmptyC(&hist->title,"");
    ajStrAssignEmptyC(&hist->xaxis,"");
    ajStrAssignEmptyC(&hist->yaxisleft,"");
    ajStrAssignEmptyC(&hist->yaxisright,"");

    AJCNEW0(hist->hists,numofsets);

    for(i=0;i<numofsets; i++)
    {
	AJNEW0((hist->hists[i]));
	(hist->hists)[i]->data       = NULL;
	(hist->hists)[i]->deletedata = AJFALSE;
	(hist->hists)[i]->colour  = i+2;
	(hist->hists)[i]->pattern = 0;
	(hist->hists)[i]->title = NULL;
	(hist->hists)[i]->xaxis = NULL;
	(hist->hists)[i]->yaxis = NULL;
    }

    return hist;
}




/* @func ajHistNewG ***********************************************************
**
** Create a histogram Object which has the histogram data and graph data
** storage capacity.
**
** @param [r] numofsets [ajuint] Number of sets of data.
** @param [r] numofpoints [ajuint] Number of data points per set.
** @param [u] graph [AjPGraph] Graph object, device, multi and name are set.
**                             The orignial AjPGraph object will be used
**                             by the AjPHist
** @return [AjPHist] histogram structure.
** @@
******************************************************************************/

AjPHist ajHistNewG(ajuint numofsets, ajuint numofpoints, AjPGraph graph)
{
    AjPHist hist = ajHistNew(numofsets, numofpoints);

    hist->graph = graph;
    ajGraphicsSetDevice(graph);
    ajGraphSetMulti(graph, numofsets);
    ajGraphicsSetFilename(graph);

    return hist;
}



/* @section Destructors *******************************************************
**
** Destriuctors for histogram objects
**
** @fdata [AjPHist]
** @fcategory delete
**
** @nam3rule Del
**
** @argrule Del phist [AjPHist*] Histogram to be deleted.
**
** @valrule * [void]
**
******************************************************************************/

/* @func ajHistDel ************************************************************
**
** Delete and free all memory associated with the histogram.
** Does not delete the graph.
**
** @param [d] phist [AjPHist*] Histogram to be deleted.
** @return [void]
**
** @@
******************************************************************************/

void ajHistDel(AjPHist* phist)
{
    ajuint i;
    AjPHist hist;

    if(!phist) return;

    hist = *phist;

    if (!hist)
	return;

    for(i=0;i<hist->numofsets; i++)
    {
	if(hist->hists[i]->deletedata)
	{
	    ajStrDel(&hist->hists[i]->title);
	    ajStrDel(&hist->hists[i]->xaxis);
	    ajStrDel(&hist->hists[i]->yaxis);
	    AJFREE(hist->hists[i]->data);
	}
	AJFREE((hist->hists[i]));
    }

    AJFREE(hist->hists);

    ajStrDel(&hist->title);
    ajStrDel(&hist->xaxis);
    ajStrDel(&hist->yaxisleft);
    ajStrDel(&hist->yaxisright);

    AJFREE(*phist);
    return;
}


/* @obsolete ajHistDelete
** @rename ajHistDel
*/
__deprecated void ajHistDelete(AjPHist* phist)
{
    ajHistDel(phist);
    return;
}


/* @section Display ***********************************************************
**
** Functions to display or write the histogram
**
** @fdata [AjPHist]
** @fcategory use
**
** @nam3rule Display Display the histogram
**
** @argrule Display thys [const AjPHist] Histogram object
**
** @valrule * [void]
**
******************************************************************************/

/* @func ajHistDisplay ********************************************************
**
** Display the histogram.
**
** @param [r] thys [const AjPHist] Histogram Structure.
** @return [void]
** @@
******************************************************************************/

void ajHistDisplay(const AjPHist thys)
{
    PLFLT *data    = NULL;		/* points to data in hist */
    PLFLT *totals  = NULL;
    PLFLT *totals2 = NULL;
    float ptsperbin;
    float max = FLT_MIN;
    float min = 0.0;
    ajuint i;
    ajuint j;
    ajint ratioint;
    ajint num;
    ajint old;
    float bin_range;
    float bar_width;
    float offset;
    float start;
    float tot;
    float percent5;
    
    /* Sanity check */
    if(thys->numofdatapoints < 1 || thys->numofsets < 1 || thys->bins < 1)
    {
	ajErr("points =%d, sets = %d, bins = %d !!! "
	      "Must all be Greater than 1 ",
	      thys->numofdatapoints,thys->numofsets,thys->bins);

	return;
    }

    /* what multiple is the bins to numofdatasets */
    /* as i may have to take an average if not identical */
    ptsperbin = (float)thys->numofdatapoints/(float)thys->bins;
    
    if(ptsperbin < 1.0)
    {
	ajErr("You cannot more have bins than datapoints!!");

	return;
    }
    /* is the ratio a whole number? */

    ratioint = (ajint)ptsperbin;

    if((ptsperbin - (float)ratioint) != 0.0)
    {
	ajErr("number of data points needs to be a multiple of bins");

	return;
    }
    /* end Sanity check */
    
    /* Add spacing either side */
    percent5 = (thys->xmax - thys->xmin)*(float)0.025;
    
    
    /* calculate max and min for each set */
    if(thys->numofsets != 1)
    {	
	/* if NOT side by side max and min as the sets added together */
	if(thys->displaytype == HIST_SIDEBYSIDE)
	{
	    /* find the max value */
	    max = INT_MIN;
	    min = 0;

	    for(j=0; j < thys->numofsets; j++)
	    {
		data = thys->hists[j]->data;

		for(i=0;i<thys->numofdatapoints;i++)
		{
		    if(data[i] > max)
			max = data[i];

		    if(data[i] < min)
			min = data[i];
		}
	    }
	}
	else if(thys->displaytype == HIST_ONTOP)
	{
	    totals = AJALLOC(thys->numofdatapoints*(sizeof(PLFLT)));

	    /* set all memory to 0.0 */
	    for(i=0;i<thys->numofdatapoints;i++)
	    {
		totals[i] = 0.0;
	    }

	    min = 0;
	    max = 0;

	    for(j=0; j < thys->numofsets; j++)
	    {
		data = thys->hists[j]->data;
		for(i=0;i<thys->numofdatapoints;i++)
		{
		    totals[i] += data[i];
		    if(totals[i] > max)
			max = totals[i];

		    if(totals[i] < min)
			min = totals[i];
		    /*	  ajDebug("%d %d\t%f",j,i,totals[i]);*/
		}
	    }
	}
	else if(thys->displaytype == HIST_SEPARATE)
	{
	    totals = AJALLOC(thys->numofsets*(sizeof(PLFLT)));
	    totals2 = AJALLOC(thys->numofsets*(sizeof(PLFLT)));

	    for(j=0; j < thys->numofsets; j++)
	    {
		data = thys->hists[j]->data;
		totals[j] = 0;
		totals2[j] = 0;

		for(i=0;i<thys->numofdatapoints;i++)
		{
		    if(totals[j] < data[i])
			totals[j] = data[i];

		    if(totals2[j] > data[i])
			totals2[j] = data[i];
		}
	    }
	}
    }
    else
    {
	data = thys->hists[0]->data;
	max = data[0];
	min = 0;

	for(i=1; i < thys->numofdatapoints; i++)
	{
	    if(data[i] > max)
		max = data[i];

	    if(data[i] < min)
		min = data[i];
	}

	if(thys->displaytype == HIST_ONTOP /*!thys->sidebyside*/)
	{
	    totals = AJALLOC(thys->numofdatapoints*(sizeof(PLFLT)));

	    /* set all memory to 0.0 */
	    for(i=0; i < thys->numofdatapoints; i++)
		totals[i] = 0.0;
	}
	else if(thys->displaytype == HIST_SEPARATE)
	{
	    totals = AJALLOC((sizeof(PLFLT)));
	    totals[0]= max;
	    totals2 = AJALLOC((sizeof(PLFLT)));
	    totals2[0]= min;
	}
    }
    
    bin_range = (thys->xmax - thys->xmin)/(float)thys->bins;
    
    if(thys->displaytype != HIST_SEPARATE)
    {
	if(max <= 0.01)
	{
	    if(max < 0.0)
		max = 0.0;
	    else
		max = 1.0;
	}

	ajGraphOpenPlotset(thys->graph, 1);
	ajGraphicsPlenv(thys->xmin-percent5, thys->xmax+percent5, min,
		     max*((float)1.025), aj_hist_mark);
	ajGraphicsSetLabelsS(thys->xaxis ,
                             thys->yaxisleft ,
                             thys->title,
                             thys->subtitle);

	ajGraphicsSetRlabelS(thys->yaxisright);
    }
    else 
	ajGraphOpenPlotset(thys->graph, thys->numofsets);
    
    if(thys->displaytype == HIST_SIDEBYSIDE)
    {
	bar_width = bin_range/thys->numofsets;

	for(i=0; i < thys->numofsets; i++)
	{
	    offset = i*bar_width;
	    start = thys->xmin;
	    num = 0;
	    tot=0.0;
	    data = thys->hists[i]->data;

	    for(j=0; j < thys->numofdatapoints; j++)
	    {
		tot += data[j];
		num++;

		if(num >= ptsperbin)
		{
		    tot = tot / (float)num;

		    if(thys->BaW)
			old = ajGraphicsSetFillpat(thys->hists[i]->pattern);
		    else
			old = ajGraphicsSetFgcolour(thys->hists[i]->colour);
		    ajGraphicsDrawposRectFill(start+offset,0.0,
                                              start+offset+bar_width,tot);

		    if(thys->BaW)
			ajGraphicsSetFillpat(old);
		    else
			ajGraphicsSetFgcolour(old);

		    ajGraphicsDrawposRect(start+offset,0.0,
                                          start+offset+bar_width,tot);
		    num = 0;
		    tot = 0;
		    start +=bin_range;
		}		
	    }
	}
    }
    else if(thys->displaytype == HIST_SEPARATE)
    {
	bar_width = bin_range;

	for(i=0; i < thys->numofsets; i++)
	{	    
	    if(totals[i] <= 0.01)
	    {			       /* apparently the ymin value */
		if(totals[i] < 0.0)
		    totals[i] = 0.0;
		else
		    totals[i] = 1.0;
	    }

	    ajGraphicsPlenv(thys->xmin - percent5, thys->xmax + percent5,
                            totals2[i]*((float)1.025), totals[i]*((float)1.025),
                            aj_hist_mark);
	    offset = /*bar_width*/0.0;
	    start = thys->xmin;
	    num = 0;
	    tot=0.0;
	    data = thys->hists[i]->data;
	    ajGraphicsSetLabelsS(thys->hists[i]->xaxis,
                                 thys->hists[i]->yaxis,
                                 thys->hists[i]->title,
                                 thys->hists[i]->subtitle);
	    
	    for(j=0; j < thys->numofdatapoints; j++)
	    {
		tot += data[j];
		num++;

		if(num >= ptsperbin)
		{
		    tot = tot / (float)num;

		    if(thys->BaW)
			old = ajGraphicsSetFillpat(thys->hists[i]->pattern);
		    else
			old = ajGraphicsSetFgcolour(thys->hists[i]->colour);

		    ajGraphicsDrawposRectFill(start+offset,0.0,
                                              start+offset+bar_width,tot);

		    if(thys->BaW)
			ajGraphicsSetFillpat(old);
		    else
			ajGraphicsSetFgcolour(old);

		    ajGraphicsDrawposRect(start+offset,0.0,
                                          start+offset+bar_width,tot);
		    num = 0;
		    tot = 0;
		    start +=bin_range;
		}		
	    }
	}
    }
    else if(thys->displaytype == HIST_ONTOP)
    {
	for(i=0; i < thys->numofdatapoints; i++)
	    totals[i] = 0.0;

	for(i=0; i < thys->numofsets; i++)
	{
	    data = thys->hists[i]->data;
	    start = thys->xmin;
	    num = 0;
	    tot=0.0;

	    for(j=0; j < thys->numofdatapoints; j++)
	    {
		tot += data[j];
		num++;

		if(num >= ptsperbin)
		{
		    tot = tot / (float)num;

		    if(thys->BaW)
			old = ajGraphicsSetFillpat(thys->hists[i]->pattern);
		    else
			old = ajGraphicsSetFgcolour(thys->hists[i]->colour);

		    ajGraphicsDrawposRectFill(start,totals[j],
                                              start+bin_range,tot+totals[j]);
		    if(thys->BaW)
			ajGraphicsSetFillpat(old);
		    else
			ajGraphicsSetFgcolour(old);

		    ajGraphicsDrawposRect(start,totals[j],
                                          start+bin_range,tot+totals[j]);
		    totals[j] += tot;
		    tot = 0;
		    /*	  ajDebug("num = %d",num);*/
		    num = 0;
		    start +=bin_range;
		}
	    }
	}
    }

    AJFREE(totals);
    AJFREE(totals2);

    return;
}




/* @section modifiers **********************************************************
**
** Controlling the internals
**
** @fdata [AjPHist]
** @fcategory modify
**
** @nam3rule Set Set histogram internals
** @nam3rule Setmulti Set multiple graph member
** @nam4rule Title Histogram title
** @nam4rule Rlabel Right axis label
** @nam4rule Xlabel X-axis label
** @nam4rule Ylabel Y-axis label
** @nam4rule Colour Set colour for a block
** @nam4rule Mono Set to plot in black and white with patterns
** @nam4rule Pattern Set pattern for a block in a black and white histogram
** @suffix C Character data
** @suffix S String data
**
** @argrule * thys [AjPHist]  Histogram object
** @argrule Setmulti indexnum [ajuint] Index for the set number
** @argrule Colour colour [ajint] Colour for bar set.
** @argrule Mono set [AjBool] Set to use patterns or colour for filling
** @argrule Pattern style [ajint] Line style number for bar set.
** @argrule C txt [const char*]  Text value
** @argrule S str [const AjPStr]  Text value
**
** @valrule * [void]
**
******************************************************************************/

/* @func ajHistSetMono ********************************************************
**
** Set patterns instead of colours for printing to B/W printers etc.
**
** @param [u] thys [AjPHist] Histogram to have ptr set.
** @param [r] set [AjBool]    Set to use patterns or colour for filling,
**                            false to reset to colour.
** @return [void]
** @@
******************************************************************************/

void ajHistSetMono(AjPHist thys, AjBool set)
{
    thys->BaW = set;

    return;
}

/* @obsolete ajHistSetBlackandWhite
** @rename ajHistSetMono
*/
__deprecated void ajHistSetBlackandWhite(AjPHist thys, AjBool set)
{
    ajHistSetMono(thys, set);
}


/* @func ajHistSetRlabelC *****************************************************
**
** Store Y Axis Right Label for the histogram
**
** @param [u] thys [AjPHist] histogram to set string in.
** @param [r] txt [const char*] text to be copied.
** @return [void]
** @@
******************************************************************************/

void ajHistSetRlabelC(AjPHist thys, const char* txt)
{
    ajStrAssignC(&thys->yaxisright,txt);

    return;
}


/* @obsolete ajHistSetYAxisRightC
** @rename ajHistSetYlabelC
*/
__deprecated void ajHistSetYAxisRightC(AjPHist thys, const char* strng)
{
    ajHistSetRlabelC(thys, strng);
    return;
}




/* @func ajHistSetTitleC ******************************************************
**
** Copy Title for the histogram.
**
** @param [u] thys [AjPHist] histogram to set string in.
** @param [r] txt [const char*] text to be copied.
** @return [void]
** @@
******************************************************************************/

void ajHistSetTitleC(AjPHist thys, const char* txt)
{
    ajStrAssignC(&thys->title,txt);

    return;
}




/* @func ajHistSetXlabelC *****************************************************
**
** Store X axis label for the histogram
**
** @param [u] thys [AjPHist] histogram to set string in.
** @param [r] txt [const char*] text to be copied.
** @return [void]
** @@
******************************************************************************/

void ajHistSetXlabelC(AjPHist thys, const char* txt)
{
    ajStrAssignC(&thys->xaxis,txt);

    return;
}

/* @obsolete ajHistSetXAxisC
** @rename ajHistSetXlabelC
*/
__deprecated void ajHistSetXAxisC(AjPHist thys, const char* strng)
{
    ajHistSetXlabelC(thys, strng);
    return;
}



/* @func ajHistSetYlabelC *****************************************************
**
** Store Y Axis Left Label for the histogram
**
** @param [u] thys [AjPHist] histogram to set string in.
** @param [r] txt [const char*] text to be copied.
** @return [void]
** @@
******************************************************************************/

void ajHistSetYlabelC(AjPHist thys, const char* txt)
{
    ajStrAssignC(&thys->yaxisleft,txt);

    return;
}


/* @obsolete ajHistSetYAxisLeftC
** @rename ajHistSetYlabelC
*/
__deprecated void ajHistSetYAxisLeftC(AjPHist thys, const char* strng)
{
    ajHistSetYlabelC(thys, strng);
    return;
}



/* @func ajHistSetmultiColour *************************************************
**
** Set colour for bars in histogram for index'th set.
**
** @param [u] thys [AjPHist] Histogram to have ptr set.
** @param [r] indexnum [ajuint]     Index for the set number.
** @param [r] colour [ajint]    Colour for bar set.
** @return [void]
** @@
******************************************************************************/

void ajHistSetmultiColour(AjPHist thys, ajuint indexnum, ajint colour)
{
    if(indexnum >= thys->numofdatapoints)
    {
	ajErr("Histograms can only be allocated from 0 to %d. NOT %u",
	      thys->numofdatapoints-1,indexnum);

	return;
    }

    thys->hists[indexnum]->colour = colour;

    return;
}


/* @obsolete ajHistSetColour
** @rename ajHistSetmltiColour
*/
__deprecated void ajHistSetColour(AjPHist thys, ajuint indexnum, ajint colour)
{
    ajHistSetmultiColour(thys, indexnum, colour);
    return;
}


/* @func ajHistSetmultiPattern ************************************************
**
** Set colour for bars in histogram for one set.
**
** @param [u] thys [AjPHist] Histogram to have ptr set.
** @param [r] indexnum [ajuint]     Index for the set number.
** @param [r] style [ajint]    Line style number for bar set.
** @return [void]
** @@
******************************************************************************/

void ajHistSetmultiPattern(AjPHist thys, ajuint indexnum, ajint style)
{
    if(indexnum >= thys->numofdatapoints)
    {
	ajErr("Histograms can only be allocated from 0 to %d. NOT %u",
	      thys->numofdatapoints-1,indexnum);

	return;
    }

    thys->hists[indexnum]->pattern = style;

    return;
}


/* @obsolete ajHistSetPattern
** @rename ajHistSetmultiPattern
*/
__deprecated void ajHistSetPattern(AjPHist thys, ajuint indexnum, ajint style)
{
    ajHistSetmultiPattern(thys, indexnum, style);
    return;
}




/* @func ajHistSetmultiTitleC *************************************************
**
** Store title for the index'th set.
**
** @param [u] thys [AjPHist]   Histogram to have ptr set.
** @param [r]  indexnum [ajuint]      Index for the set number.
** @param [r]  txt  [const char *]  Title.
** @return [void]
** @@
******************************************************************************/

void ajHistSetmultiTitleC(AjPHist thys, ajuint indexnum, const char *txt)
{
    if(indexnum >= thys->numofdatapoints)
    {
	ajErr("Histograms can only be allocated from 0 to %d. NOT %u",
	      thys->numofdatapoints-1,indexnum);

	return;
    }

    ajStrAssignC(&thys->hists[indexnum]->title,txt);

    return;
}




/* @obsolete ajHistSetMultiTitleC
** @rename ajHistSetmultiTitleC
*/
__deprecated void ajHistSetMultiTitleC(AjPHist hist, ajint indexnum,
                                      const char* title)
{
    ajHistSetmultiTitleC(hist, indexnum, title);
    return;
}


/* @func ajHistSetmultiTitleS **************************************************
**
** Set ptr for title for index'th set..
**
** @param [u] thys [AjPHist]   Histogram to have ptr set.
** @param [r] indexnum [ajuint]       Index for the set number.
** @param [r] str [const AjPStr]    Title.
** @return [void]
** @@
******************************************************************************/

void ajHistSetmultiTitleS(AjPHist thys, ajuint indexnum, const AjPStr str)
{
    if(indexnum >= thys->numofdatapoints)
    {
	ajErr("Histograms can only be allocated from 0 to %d. NOT %u",
	      thys->numofdatapoints-1,indexnum);
	return;
    }

    ajStrAssignS(&thys->hists[indexnum]->title, str);

    return;
}


/* @obsolete ajHistSetMultiTitle
** @rename ajHistSetmultiTitleS
*/
__deprecated void ajHistSetMultiTitle(AjPHist hist, ajint indexnum,
                                      const AjPStr title)
{
    ajHistSetmultiTitleS(hist, indexnum, title);
    return;
}


/* @func ajHistSetmultiXlabelC ************************************************
**
** Store X axis title for the index'th set.
**
** @param [u] thys [AjPHist]   Histogram to have ptr set.
** @param [r] indexnum [ajuint]       Index for the set number.
** @param [r] txt [const char *]    x Title.
** @return [void]
** @@
******************************************************************************/

void ajHistSetmultiXlabelC(AjPHist thys, ajuint indexnum, const char *txt)
{
    if(indexnum >= thys->numofdatapoints)
    {
	ajErr("Histograms can only be allocated from 0 to %d. NOT %u",
	      thys->numofdatapoints-1,indexnum);

	return;
    }

    ajStrAssignC(&thys->hists[indexnum]->xaxis,txt);

    return;
}


/* @obsolete ajHistSetMultiXTitleC
** @rename ajHistSetmultiXlabelC
*/
__deprecated void ajHistSetMultiXTitleC(AjPHist hist, ajint indexnum,
                                      const char* title)
{
    ajHistSetmultiXlabelC(hist, indexnum, title);
    return;
}


/* @func ajHistSetmultiXlabelS *************************************************
**
** Set ptr for X axis title for index'th set..
**
** @param [u] thys [AjPHist]   Histogram to have ptr set.
** @param [r] indexnum [ajuint]       Index for the set number.
** @param [r] str [const AjPStr]    x Title.
** @return [void]
** @@
******************************************************************************/

void ajHistSetmultiXlabelS(AjPHist thys, ajuint indexnum, const AjPStr str)
{
    if(indexnum >= thys->numofdatapoints)
    {
	ajErr("Histograms can only be allocated from 0 to %d. NOT %u",
	      thys->numofdatapoints-1,indexnum);
	return;
    }

    ajStrAssignS(&thys->hists[indexnum]->xaxis, str);

    return;
}


/* @obsolete ajHistSetMultiXTitle
** @rename ajHistSetmultiXlabelS
*/
__deprecated void ajHistSetMultiXTitle(AjPHist hist, ajint indexnum,
                                      const AjPStr title)
{
    ajHistSetmultiXlabelS(hist, indexnum, title);
    return;
}


/* @func ajHistSetmultiYlabelC ************************************************
**
** Store Y axis title for the index'th set.
**
** @param [u] thys [AjPHist]   Histogram to have ptr set.
** @param [r] indexnum [ajuint]       Index for the set number.
** @param [r] txt [const char *]    Y Title.
** @return [void]
** @@
******************************************************************************/

void ajHistSetmultiYlabelC(AjPHist thys, ajuint indexnum, const char *txt)
{
    if(indexnum >= thys->numofdatapoints)
    {
	ajErr("Histograms can only be allocated from 0 to %d. NOT %u",
	      thys->numofdatapoints-1,indexnum);
	return;
    }

    ajStrAssignC(&thys->hists[indexnum]->yaxis,txt);

    return;
}




/* @obsolete ajHistSetMultiYTitleC
** @rename ajHistSetmultiYlabelC
*/
__deprecated void ajHistSetMultiYTitleC(AjPHist hist, ajint indexnum,
                                      const char* title)
{
    ajHistSetmultiYlabelC(hist, indexnum, title);
    return;
}


/* @func ajHistSetmultiYlabelS *************************************************
**
** Set ptr for Y axis title for index'th set..
**
** @param [u] thys [AjPHist]   Histogram to have ptr set.
** @param [r] indexnum [ajuint]       Index for the set number.
** @param [r] str [const AjPStr]    Y Title.
** @return [void]
** @@
******************************************************************************/

void ajHistSetmultiYlabelS(AjPHist thys, ajuint indexnum, const AjPStr str)
{
    if(indexnum >= thys->numofdatapoints)
    {
	ajErr("Histograms can only be allocated from 0 to %d. NOT %u",
	      thys->numofdatapoints-1,indexnum);
	return;
    }

    ajStrAssignS(&thys->hists[indexnum]->yaxis, str);

    return;
}




/* @obsolete ajHistSetMultiYTitle
** @rename ajHistSetmultiYlabelS
*/
__deprecated void ajHistSetMultiYTitle(AjPHist hist, ajint indexnum,
                                      const AjPStr title)
{
    ajHistSetmultiYlabelS(hist, indexnum, title);
    return;
}


/* @section Histogram data management *****************************************
**
** Functions to manage the data values associated with a histogram object
**
** @fdata [AjPHist]
** @fcategory modify
**
** @nam3rule Data
** @nam4rule Add Add data values to be owned by the histogram object
** @nam4rule Copy Copy data values to histogram object
** @nam4rule Set Set internals of one plot
**
** @argrule * thys [AjPHist] Histogram object
** @argrule Data indexnum [ajuint] number within multiple graph.
** @argrule Add data [PLFLT*] graph to be added.
** @argrule Copy srcdata [PLFLT const*] graph to be added.
**
** @valrule * [void]
**
******************************************************************************/

/* @func ajHistDataAdd ********************************************************
**
** Set internal pointer to data for a set of data points for one set.
**
** @param [u] thys [AjPHist] Histogram to have ptr set.
** @param [r] indexnum [ajuint] Index for the set number.
** @param [u] data  [PLFLT*]  Data values to be owned by the histogram object
** @return [void]
** @@
******************************************************************************/

void ajHistDataAdd(AjPHist thys, ajuint indexnum, PLFLT *data)
{
    if(indexnum >= thys->numofdatapoints)
    {
	ajErr("Histograms can only be allocated from 0 to %d. NOT %u",
	      thys->numofdatapoints-1,indexnum);

	return;
    }

    if(!thys->hists[indexnum]->data)
	thys->numofsets++;

    thys->hists[indexnum]->data = data;

    return;
}


/* @obsolete ajHistSetPtrToData
** @rename ajHistDataAdd
*/
__deprecated void ajHistSetPtrToData(AjPHist thys, ajint indexnum, PLFLT *data)
{
    ajHistDataAdd(thys, indexnum, data);
    return;
}


/* @func ajHistDataCopy *******************************************************
**
** Copy data from data array to histogram for one set.
**
** @param [u] thys [AjPHist] Histogram to have ptr set.
** @param [r] indexnum [ajuint] Index for the set number.
** @param [r] srcdata  [PLFLT const*]  Data to be copied
** @return [void]
** @@
******************************************************************************/

void ajHistDataCopy(AjPHist thys, ajuint indexnum, PLFLT const *srcdata)
{
    ajuint i;

    if(indexnum >= thys->numofdatapoints)
    {
	ajErr("Histograms can only be allocated from 0 to %d. NOT %u",
	      thys->numofdatapoints-1,indexnum);

	return;
    }

    thys->hists[indexnum]->data = AJALLOC(thys->numofdatapoints*sizeof(PLFLT));

    for(i=0; i < thys->numofdatapoints; i++)
	thys->hists[indexnum]->data[i] = srcdata[i];

    thys->hists[indexnum]->deletedata = AJTRUE;
    thys->numofsets++;

    return;
}


/* @obsolete ajHistCopyData
** @rename ajHistDataCopy
*/
__deprecated void ajHistCopyData(AjPHist thys, ajuint indexnum,
                                 const PLFLT *data)
{
    ajHistDataCopy(thys, indexnum, data);
    return;
}





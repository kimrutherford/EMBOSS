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
#include "limits.h"
#include "float.h"


ajint aj_hist_mark=GRAPH_HIST;




/* @func ajHistDisplay ********************************************************
**
** Display the histogram.
**
** @param [r] hist [const AjPHist] Histogram Structure.
** @return [void]
** @@
******************************************************************************/

void ajHistDisplay(const AjPHist hist)
{
    PLFLT *data    = NULL;		/* points to data in hist */
    PLFLT *totals  = NULL;
    PLFLT *totals2 = NULL;
    float ptsperbin;
    float max = FLT_MIN;
    float min = 0.0;
    ajint i;
    ajint j;
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
    if(hist->numofdatapoints < 1 || hist->numofsets < 1 || hist->bins < 1)
    {
	ajErr("points =%d, sets = %d, bins = %d !!! "
	      "Must all be Greater than 1 ",
	      hist->numofdatapoints,hist->numofsets,hist->bins);

	return;
    }

    /* what multiple is the bins to numofdatasets */
    /* as i may have to take an average if not identical */
    ptsperbin = (float)hist->numofdatapoints/(float)hist->bins;
    
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
    percent5 = (hist->xmax-hist->xmin)*(float)0.025;
    
    
    /* calculate max and min for each set */
    if(hist->numofsets != 1)
    {	
	/* if NOT side by side max and min as the sets added together */
	if(hist->displaytype == HIST_SIDEBYSIDE)
	{
	    /* find the max value */
	    max = INT_MIN;
	    min = 0;

	    for(j=0;j<hist->numofsets;j++)
	    {
		data = hist->hists[j]->data;

		for(i=0;i<hist->numofdatapoints;i++)
		{
		    if(data[i] > max)
			max = data[i];

		    if(data[i] < min)
			min = data[i];
		}
	    }
	}
	else if(hist->displaytype == HIST_ONTOP)
	{
	    totals = AJALLOC(hist->numofdatapoints*(sizeof(PLFLT)));

	    /* set all memory to 0.0 */
	    for(i=0;i<hist->numofdatapoints;i++)
	    {
		totals[i] = 0.0;
	    }

	    min = 0;
	    max = 0;

	    for(j=0;j<hist->numofsets;j++)
	    {
		data = hist->hists[j]->data;
		for(i=0;i<hist->numofdatapoints;i++)
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
	else if(hist->displaytype == HIST_SEPARATE)
	{
	    totals = AJALLOC(hist->numofsets*(sizeof(PLFLT)));
	    totals2 = AJALLOC(hist->numofsets*(sizeof(PLFLT)));

	    for(j=0;j<hist->numofsets;j++)
	    {
		data = hist->hists[j]->data;
		totals[j] = 0;
		totals2[j] = 0;

		for(i=0;i<hist->numofdatapoints;i++)
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
	data = hist->hists[0]->data;
	max = data[0];
	min = 0;

	for(i=1;i<hist->numofdatapoints;i++)
	{
	    if(data[i] > max)
		max = data[i];

	    if(data[i] < min)
		min = data[i];
	}

	if(hist->displaytype == HIST_ONTOP /*!hist->sidebyside*/)
	{
	    totals = AJALLOC(hist->numofdatapoints*(sizeof(PLFLT)));

	    /* set all memory to 0.0 */
	    for(i=0;i<hist->numofdatapoints;i++)
		totals[i] = 0.0;
	}
	else if(hist->displaytype == HIST_SEPARATE)
	{
	    totals = AJALLOC((sizeof(PLFLT)));
	    totals[0]= max;
	    totals2 = AJALLOC((sizeof(PLFLT)));
	    totals2[0]= min;
	}
    }
    
    bin_range = (hist->xmax - hist->xmin)/(float)hist->bins;
    
    if(hist->displaytype != HIST_SEPARATE)
    {
	if(max <= 0.01)
	{
	    if(max < 0.0)
		max = 0.0;
	    else
		max = 1.0;
	}

	ajGraphOpenPlot(hist->graph, 1);
	ajGraphPlenv(hist->xmin-percent5, hist->xmax+percent5, min,
		     max*((float)1.025), aj_hist_mark);
	ajGraphLabel(ajStrGetPtr(hist->xaxis) ,
		     ajStrGetPtr(hist->yaxisleft) ,
		     ajStrGetPtr(hist->title)," ");

	ajGraphLabelYRight(ajStrGetPtr(hist->yaxisright));
    }
    else 
	ajGraphOpenPlot(hist->graph, hist->numofsets);
    
    if(hist->displaytype == HIST_SIDEBYSIDE)
    {
	bar_width = bin_range/hist->numofsets;

	for(i=0;i<hist->numofsets;i++)
	{
	    offset = i*bar_width;
	    start = hist->xmin;
	    num = 0;
	    tot=0.0;
	    data = hist->hists[i]->data;

	    for(j=0;j<hist->numofdatapoints;j++)
	    {
		tot += data[j];
		num++;

		if(num >= ptsperbin)
		{
		    tot = tot / (float)num;

		    if(hist->BaW)
			old = ajGraphSetFillPat(hist->hists[i]->pattern);
		    else
			old = ajGraphSetFore(hist->hists[i]->colour);
		    ajGraphRectFill(start+offset,0.0,
				    start+offset+bar_width,tot);

		    if(hist->BaW)
			ajGraphSetFillPat(old);
		    else
			ajGraphSetFore(old);

		    ajGraphRect(start+offset,0.0,start+offset+bar_width,tot);
		    num = 0;
		    tot = 0;
		    start +=bin_range;
		}		
	    }
	}
    }
    else if(hist->displaytype == HIST_SEPARATE)
    {
	bar_width = bin_range;

	for(i=0;i<hist->numofsets;i++)
	{	    
	    if(totals[i] <= 0.01)
	    {			       /* apparently the ymin value */
		if(totals[i] < 0.0)
		    totals[i] = 0.0;
		else
		    totals[i] = 1.0;
	    }

	    ajGraphPlenv(hist->xmin-percent5, hist->xmax+percent5,
			 totals2[i]*((float)1.025), totals[i]*((float)1.025),
			 aj_hist_mark);
	    offset = /*bar_width*/0.0;
	    start = hist->xmin;
	    num = 0;
	    tot=0.0;
	    data = hist->hists[i]->data;
	    ajGraphLabel(ajStrGetPtr(hist->hists[i]->xaxis) ,
			 ajStrGetPtr(hist->hists[i]->yaxis) ,
			 ajStrGetPtr(hist->hists[i]->title)," ");
	    
	    for(j=0;j<hist->numofdatapoints;j++)
	    {
		tot += data[j];
		num++;

		if(num >= ptsperbin)
		{
		    tot = tot / (float)num;

		    if(hist->BaW)
			old = ajGraphSetFillPat(hist->hists[i]->pattern);
		    else
			old = ajGraphSetFore(hist->hists[i]->colour);

		    ajGraphRectFill(start+offset,0.0,
				    start+offset+bar_width,tot);

		    if(hist->BaW)
			ajGraphSetFillPat(old);
		    else
			ajGraphSetFore(old);

		    ajGraphRect(start+offset,0.0,start+offset+bar_width,tot);
		    num = 0;
		    tot = 0;
		    start +=bin_range;
		}		
	    }
	}
    }
    else if(hist->displaytype == HIST_ONTOP)
    {
	for(i=0;i<hist->numofdatapoints;i++)
	    totals[i] = 0.0;

	for(i=0;i<hist->numofsets;i++)
	{
	    data = hist->hists[i]->data;
	    start = hist->xmin;
	    num = 0;
	    tot=0.0;

	    for(j=0;j<hist->numofdatapoints;j++)
	    {
		tot += data[j];
		num++;

		if(num >= ptsperbin)
		{
		    tot = tot / (float)num;

		    if(hist->BaW)
			old = ajGraphSetFillPat(hist->hists[i]->pattern);
		    else
			old = ajGraphSetFore(hist->hists[i]->colour);

		    ajGraphRectFill(start,totals[j],
				    start+bin_range,tot+totals[j]);
		    if(hist->BaW)
			ajGraphSetFillPat(old);
		    else
			ajGraphSetFore(old);

		    ajGraphRect(start,totals[j],
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




/* @func ajHistClose **********************************************************
**
** Closes the histograms window.
**
** @return [void]
**
** @@
******************************************************************************/

void ajHistClose(void)
{
    ajGraphCloseWin();

    return;
}




/* @func ajHistDelete *********************************************************
**
** Delete and free all memory associated with the histogram.
** Does not delete the graph.
**
** @param [d] phist [AjPHist*] Histogram to be deleted.
** @return [void]
**
** @@
******************************************************************************/

void ajHistDelete(AjPHist* phist)
{
    ajint i;
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




/* @func ajHistNew ************************************************************
**
** Create a histogram Object. Which can hold "numofsets" set of data of which
** all must have "numofpoints" data points in them.
**
** @param [r] numofsets [ajint] Number of sets of data.
** @param [r] numofpoints [ajint] Number of data points per set.
** @return [AjPHist] histogram structure.
** @@
******************************************************************************/

AjPHist ajHistNew(ajint numofsets, ajint numofpoints)
{
    static AjPHist hist = NULL;
    ajint i;

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
** @param [r] numofsets [ajint] Number of sets of data.
** @param [r] numofpoints [ajint] Number of data points per set.
** @param [u] graph [AjPGraph] Graph object, device, multi and name are set.
**                             The orignial AjPGraph object will be used
**                             by the AjPHist
** @return [AjPHist] histogram structure.
** @@
******************************************************************************/

AjPHist ajHistNewG(ajint numofsets, ajint numofpoints, AjPGraph graph)
{
    AjPHist hist = ajHistNew(numofsets, numofpoints);
    hist->graph = graph;
    ajGraphSetDevice(graph);
    ajGraphSetMulti(graph, numofsets);
    ajGraphSetName(graph);

    return hist;
}




/* @func ajHistSetMultiTitle **************************************************
**
** Set ptr for title for index'th set..
**
** @param [u] hist [AjPHist]   Histogram to have ptr set.
** @param [r] indexnum [ajint]       Index for the set number.
** @param [r] title [const AjPStr]    Title.
** @return [void]
** @@
******************************************************************************/

void ajHistSetMultiTitle(AjPHist hist, ajint indexnum, const AjPStr title)
{
    if(indexnum >= hist->numofdatapoints || indexnum < 0)
    {
	ajErr("Histograms can only be allocated from 0 to %d. NOT %d",
	      hist->numofdatapoints-1,indexnum);
	return;
    }

    ajStrAssignS(&hist->hists[indexnum]->title, title);

    return;
}




/* @func ajHistSetMultiTitleC *************************************************
**
** Store title for the index'th set.
**
** @param [u] hist [AjPHist]   Histogram to have ptr set.
** @param [r]  indexnum [ajint]      Index for the set number.
** @param [r]  title  [const char *]  Title.
** @return [void]
** @@
******************************************************************************/

void ajHistSetMultiTitleC(AjPHist hist, ajint indexnum, const char *title)
{
    if(indexnum >= hist->numofdatapoints || indexnum < 0)
    {
	ajErr("Histograms can only be allocated from 0 to %d. NOT %d",
	      hist->numofdatapoints-1,indexnum);

	return;
    }

    ajStrAssignC(&hist->hists[indexnum]->title,title);

    return;
}




/* @func ajHistSetMultiXTitle *************************************************
**
** Set ptr for X axis title for index'th set..
**
** @param [u] hist [AjPHist]   Histogram to have ptr set.
** @param [r] indexnum [ajint]       Index for the set number.
** @param [r] title [const AjPStr]    x Title.
** @return [void]
** @@
******************************************************************************/

void ajHistSetMultiXTitle(AjPHist hist, ajint indexnum, const AjPStr title)
{
    if(indexnum >= hist->numofdatapoints || indexnum < 0)
    {
	ajErr("Histograms can only be allocated from 0 to %d. NOT %d",
	      hist->numofdatapoints-1,indexnum);
	return;
    }

    ajStrAssignS(&hist->hists[indexnum]->xaxis, title);

    return;
}




/* @func ajHistSetMultiXTitleC ************************************************
**
** Store X axis title for the index'th set.
**
** @param [u] hist [AjPHist]   Histogram to have ptr set.
** @param [r] indexnum [ajint]       Index for the set number.
** @param [r] title [const char *]    x Title.
** @return [void]
** @@
******************************************************************************/

void ajHistSetMultiXTitleC(AjPHist hist, ajint indexnum, const char *title)
{
    if(indexnum >= hist->numofdatapoints || indexnum < 0)
    {
	ajErr("Histograms can only be allocated from 0 to %d. NOT %d",
	      hist->numofdatapoints-1,indexnum);

	return;
    }

    ajStrAssignC(&hist->hists[indexnum]->xaxis,title);

    return;
}




/* @func ajHistSetMultiYTitle *************************************************
**
** Set ptr for Y axis title for index'th set..
**
** @param [u] hist [AjPHist]   Histogram to have ptr set.
** @param [r] indexnum [ajint]       Index for the set number.
** @param [r] title [const AjPStr]    Y Title.
** @return [void]
** @@
******************************************************************************/

void ajHistSetMultiYTitle(AjPHist hist, ajint indexnum, const AjPStr title)
{
    if(indexnum >= hist->numofdatapoints || indexnum < 0)
    {
	ajErr("Histograms can only be allocated from 0 to %d. NOT %d",
	      hist->numofdatapoints-1,indexnum);
	return;
    }

    ajStrAssignS(&hist->hists[indexnum]->yaxis, title);

    return;
}




/* @func ajHistSetMultiYTitleC ************************************************
**
** Store Y axis title for the index'th set.
**
** @param [u] hist [AjPHist]   Histogram to have ptr set.
** @param [r] indexnum [ajint]       Index for the set number.
** @param [r] title [const char *]    Y Title.
** @return [void]
** @@
******************************************************************************/

void ajHistSetMultiYTitleC(AjPHist hist, ajint indexnum, const char *title)
{
    if(indexnum >= hist->numofdatapoints || indexnum < 0)
    {
	ajErr("Histograms can only be allocated from 0 to %d. NOT %d",
	      hist->numofdatapoints-1,indexnum);
	return;
    }

    ajStrAssignC(&hist->hists[indexnum]->yaxis,title);

    return;
}




/* @func ajHistSetPtrToData ***************************************************
**
** Set ptr to data for a set of data points for index'th set..
**
** @param [u] hist [AjPHist] Histogram to have ptr set.
** @param [r] indexnum [ajint]     Index for the set number.
** @param [u] data  [PLFLT*]  Ptr to the data. Will now be used by the AjPHist
** @return [void]
** @@
******************************************************************************/

void ajHistSetPtrToData(AjPHist hist, ajint indexnum, PLFLT *data)
{
    if(indexnum >= hist->numofdatapoints || indexnum < 0)
    {
	ajErr("Histograms can only be allocated from 0 to %d. NOT %d",
	      hist->numofdatapoints-1,indexnum);

	return;
    }

    if(!hist->hists[indexnum]->data)
	hist->numofsets++;

    hist->hists[indexnum]->data = data;

    return;
}




/* @func ajHistCopyData *******************************************************
**
** Copy data from data ptr to histogram for index'th set.
**
** @param [u] hist [AjPHist] Histogram to have ptr set.
** @param [r] indexnum [ajint]     Index for the set number.
** @param [r] data  [const PLFLT*]  Ptr to the data.
** @return [void]
** @@
******************************************************************************/

void ajHistCopyData(AjPHist hist, ajint indexnum, const PLFLT *data)
{
    ajint i;

    if(indexnum >= hist->numofdatapoints || indexnum < 0)
    {
	ajErr("Histograms can only be allocated from 0 to %d. NOT %d",
	      hist->numofdatapoints-1,indexnum);

	return;
    }

    hist->hists[indexnum]->data = AJALLOC(hist->numofdatapoints*sizeof(PLFLT));

    for(i=0;i<hist->numofdatapoints;i++)
	hist->hists[indexnum]->data[i] = data[i];

    hist->hists[indexnum]->deletedata = AJTRUE;
    hist->numofsets++;

    return;
}




/* @func ajHistSetTitleC ******************************************************
**
** Copy Title for the histogram.
**
** @param [u] hist [AjPHist] histogram to set string in.
** @param [r] strng [const char*] text to be copied.
** @return [void]
** @@
******************************************************************************/

void ajHistSetTitleC(AjPHist hist, const char* strng)
{
    ajStrAssignC(&hist->title,strng);

    return;
}




/* @func ajHistSetXAxisC ******************************************************
**
** Store X axis label for the histogram
**
** @param [u] hist [AjPHist] histogram to set string in.
** @param [r] strng [const char*] text to be copied.
** @return [void]
** @@
******************************************************************************/

void ajHistSetXAxisC(AjPHist hist, const char* strng)
{
    ajStrAssignC(&hist->xaxis,strng);

    return;
}




/* @func ajHistSetYAxisLeftC **************************************************
**
** Store Y Axis Left Label for the histogram
**
** @param [u] hist [AjPHist] histogram to set string in.
** @param [r] strng [const char*] text to be copied.
** @return [void]
** @@
******************************************************************************/

void ajHistSetYAxisLeftC(AjPHist hist, const char* strng)
{
    ajStrAssignC(&hist->yaxisleft,strng);

    return;
}




/* @func ajHistSetYAxisRightC**************************************************
**
** Store Y Axis Right Label for the histogram
**
** @param [u] hist [AjPHist] histogram to set string in.
** @param [r] strng [const char*] text to be copied.
** @return [void]
** @@
******************************************************************************/

void ajHistSetYAxisRightC(AjPHist hist, const char* strng)
{
    ajStrAssignC(&hist->yaxisright,strng);

    return;
}




/* @func  ajHistSetColour *****************************************************
**
** Set colour for bars in histogram for index'th set.
**
** @param [u] hist [AjPHist] Histogram to have ptr set.
** @param [r] indexnum [ajint]     Index for the set number.
** @param [r] colour [ajint]    Colour for bar set.
** @return [void]
** @@
******************************************************************************/

void ajHistSetColour(AjPHist hist, ajint indexnum, ajint colour)
{
    if(indexnum >= hist->numofdatapoints || indexnum < 0)
    {
	ajErr("Histograms can only be allocated from 0 to %d. NOT %d",
	      hist->numofdatapoints-1,indexnum);

	return;
    }

    hist->hists[indexnum]->colour = colour;

    return;
}




/* @func  ajHistSetPattern ****************************************************
**
** Set colour for bars in histogram for index'th set.
**
** @param [u] hist [AjPHist] Histogram to have ptr set.
** @param [r] indexnum [ajint]     Index for the set number.
** @param [r] style [ajint]    Line style number for bar set.
** @return [void]
** @@
******************************************************************************/

void ajHistSetPattern(AjPHist hist, ajint indexnum, ajint style)
{
    if(indexnum >= hist->numofdatapoints || indexnum < 0)
    {
	ajErr("Histograms can only be allocated from 0 to %d. NOT %d",
	      hist->numofdatapoints-1,indexnum);

	return;
    }

    hist->hists[indexnum]->pattern = style;

    return;
}




/* @func  ajHistSetBlackandWhite **********************************************
**
** Set patterns instead of colours for printing to B/W printers etc.
**
** @param [u] hist [AjPHist] Histogram to have ptr set.
** @param [r] set [AjBool]    Set to use patterns or colour for filling.
** @return [void]
** @@
******************************************************************************/

void ajHistSetBlackandWhite(AjPHist hist, AjBool set)
{
    hist->BaW = set;

    return;
}

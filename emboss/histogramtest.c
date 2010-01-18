/* @source histogramtest application
**
** Plot test for histograms
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

#include "emboss.h"
#include "ajhist.h"




/* @prog histogramtest ********************************************************
**
** Testing histogram plot functions
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPHist hist   = NULL;
    AjPGraph graph = NULL;
    PLFLT **data;
    PLFLT *data2;
    ajint i;
    ajint j;
    ajint k = 1;
    ajint sets;
    ajint points;
    char temp[20];
    ajint ibins;
    ajint xstart;
    ajint xend;

    embInit("histogramtest", argc, argv);

    graph  = ajAcdGetGraphxy("graph");
    sets   = ajAcdGetInt("sets");
    points = ajAcdGetInt("points");

    hist = ajHistNewG(sets,points, graph);

    ibins = ajAcdGetInt("bins");
    hist->bins = ibins;

    hist->displaytype = HIST_ONTOP;

    xstart = ajAcdGetInt("xstart");
    hist->xmin = xstart;
    xend = ajAcdGetInt("xend");
    hist->xmax = xend;

    ajHistSetTitleC(hist, "A demo of the Histogram suite");
    ajHistSetXlabelC(hist, "X axis");
    ajHistSetYlabelC(hist, "LEFT");
    ajHistSetRlabelC(hist, "RIGHT");

    AJCNEW(data, sets);
    for(i=0;i<sets;i++)
    {
	AJCNEW(data[i], points);
	data2 = data[i];
	for(j=0;j<points;j++)
	    data2[j] = k++;

	ajHistDataAdd(hist, i, data2);
    }

    ajHistDisplay(hist);

    for(i=0;i<sets;i++)
	AJFREE(data[i]);
    ajHistDel(&hist);

    /* now do again but copy the data */
    hist = ajHistNewG(sets,points, graph);

    hist->bins = ibins;
    hist->displaytype = HIST_SIDEBYSIDE;
    hist->xmin = xstart;
    hist->xmax = xend;

    ajHistSetTitleC(hist, "A demo of the Histogram suite");
    ajHistSetXlabelC(hist, "X axis");
    ajHistSetYlabelC(hist, "LEFT");
    ajHistSetRlabelC(hist, "RIGHT");

    k = -10;
    for(i=0;i<sets;i++)
    {
	AJCNEW(data2, points);
	for(j=0;j<points;j++)
	    data2[j] = k++;

	ajHistDataCopy(hist, i, data2);
	AJFREE(data2);
    }

    for(i=0;i<sets;i++)
	ajHistSetmultiPattern(hist, i, i);

    ajHistSetMono(hist, AJTRUE);
    ajHistDisplay(hist);

    hist->displaytype=HIST_SEPARATE;

    for(i=0;i<sets;i++)
    {
	sprintf(temp,"number %d",i);
	ajHistSetmultiTitleC(hist,i,temp);
	sprintf(temp,"sequence %d",i);
	ajHistSetmultiXlabelC(hist,i,temp);
	sprintf(temp,"y value for %d",i);
	ajHistSetmultiYlabelC(hist,i,temp);
    }

    ajHistSetMono(hist , AJFALSE);
    ajHistDisplay(hist);

    for(i=0;i<sets;i++)
        AJFREE(data[i]);
    AJFREE(data);

    ajGraphxyDel(&graph);

    ajHistDel(&hist);

    ajGraphicsClose();

    embExit();

    return 0;
}

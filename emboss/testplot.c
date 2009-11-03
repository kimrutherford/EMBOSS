/* @source testplot.c
**
** General test routine for graph plotting PLPLOT.
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

#ifndef NO_PLOT
#include "emboss.h"
#include "ajax.h"
#include "ajgraph.h"
#include <math.h>
#define numsets 3
#define numpoints 360

ajint ipoints;




/* @prog testplot *************************************************************
**
** Test of plotting
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPGraphPlpData graphdata;
    ajint i;
    AjPGraph mult;
    AjBool overlap;

    ajGraphInit("testplot", argc, argv);

    mult    = ajAcdGetGraphxy("graph");
    ipoints = ajAcdGetInt("points");
    overlap = ajAcdGetBoolean("overlap");

    ajUser("Plotting sin, cos and  tan for %d degrees",ipoints);

    /* Create multiple graph store for a set of graphs */
    /* This is used for drawing several graphs in one window */

    /* create a new graph */
    graphdata = ajGraphPlpDataNewI(ipoints);

    /* add graph to list in a multiple graph */
    ajGraphDataAdd(mult,graphdata);

    /* set overlap based on bool*/
    ajGraphxySetOverLap(mult, overlap);

    /* create the point values for this graph */
    for(i=0;i<ipoints; i++)
    {
	graphdata->x[i] = (float)i;
	graphdata->y[i] = sin(ajDegToRad(i));
    }

    /* embGraphSetData(graphdata,&array[0][0]);*/
    ajGraphPlpDataSetYTitleC(graphdata,"SINE(degrees)");
    ajGraphPlpDataSetXTitleC(graphdata,"degrees");
    ajGraphPlpDataSetTitleC(graphdata,"hello");
    ajGraphPlpDataSetColour(graphdata,GREEN);

    if(!overlap)
    {
	ajGraphPlpDataAddRect(graphdata,70.0,0.55,80.0,0.45,GREEN,1);
	ajGraphPlpDataAddText(graphdata,82.0,0.5,GREEN,"Sine");
    }

    graphdata = ajGraphPlpDataNewI(ipoints);

    ajGraphDataAdd(mult,graphdata);

    for(i=0;i<ipoints; i++)
    {
	graphdata->x[i] = (float)i;
	graphdata->y[i] = cos(ajDegToRad((float)i));
    }

    ajGraphPlpDataSetXTitleC(graphdata,"degrees");
    ajGraphPlpDataSetYTitleC(graphdata,"COS(degrees)");
    ajGraphPlpDataSetTitleC(graphdata,"hello");
    ajGraphPlpDataSetColour(graphdata,RED);

    if(!overlap)
    {
	ajGraphPlpDataAddLine(graphdata,5.0,0.1,15.0,0.1,RED);
	ajGraphPlpDataAddText(graphdata,17.0,0.1,RED,"Cosine");
    }

    /* now set larger axis than needed */
    ajGraphPlpDataSetMaxMin(graphdata,0.0,(float)ipoints,-0.5,1.5);

    graphdata = ajGraphPlpDataNewI(ipoints);
    ajGraphPlpDataSetLineType(graphdata, 3);
    ajGraphDataAdd(mult,graphdata);

    for(i=0;i<ipoints; i++)
    {
	graphdata->x[i] = (float)i;
	graphdata->y[i] = (tan(ajDegToRad(i))*0.2);
    }

    ajGraphPlpDataSetXTitleC(graphdata,"degrees");
    ajGraphPlpDataSetYTitleC(graphdata,"TAN(degrees)");
    ajGraphPlpDataSetTitleC(graphdata,"hello");
    ajGraphPlpDataSetLineType(graphdata, 2);
    ajGraphPlpDataSetColour(graphdata,BLUE);

    if(!overlap)
    {
	ajGraphPlpDataAddRect(graphdata,5.0,9.0,15.0,8.5,BLUE,0);
	ajGraphPlpDataAddText(graphdata,17.0,8.75,BLUE,"Tangent");
    }

    ajGraphSetYTitleC(mult,"sin,cos,tan");
    ajGraphSetXTitleC(mult,"degrees");
    ajGraphSetTitleC(mult,"Trig functions");

    ajGraphxySetYStart(mult,0.0);
    ajGraphxySetYEnd(mult,2.0);

    if(overlap)
    {
	ajGraphAddRect(mult,5.0,9.0,15.0,8.5,BLUE,0);
	ajGraphAddRect(mult,5.0,8.0,15.0,7.5,GREEN,1);
	ajGraphAddLine(mult,5.0,6.75,15.0,6.75,RED);
	ajGraphAddText(mult,17.0,8.75,BLUE,"Tangent");
	ajGraphAddText(mult,17.0,7.75,GREEN,"Sine");
	ajGraphAddText(mult,17.0,6.75,RED,"Cosine");
    }

    ajGraphxyDisplay(mult,AJTRUE);

    ajGraphxyDel(&mult);

    embExit();

    return 0;
}
#else
/* @header ********************************************************************
**
******************************************************************************/
int main(int argc, char **argv)
{
    ajFatal("Sorry no PLplot was found on compilation hence NO graph\n");
    return 0;
}
#endif





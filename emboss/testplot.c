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
    AjPGraphdata graphdata;
    ajint i;
    AjPGraph mult;
    AjBool overlap;

    embInit("testplot", argc, argv);

    mult    = ajAcdGetGraphxy("graph");
    ipoints = ajAcdGetInt("points");
    overlap = ajAcdGetBoolean("overlap");

    ajUser("Plotting sin, cos and  tan for %d degrees",ipoints);

    /* Create multiple graph store for a set of graphs */
    /* This is used for drawing several graphs in one window */

    /* create a new graph */
    graphdata = ajGraphdataNewI(ipoints);

    /* add graph to list in a multiple graph */
    ajGraphDataAdd(mult,graphdata);

    /* set overlap based on bool*/
    ajGraphxySetflagOverlay(mult, overlap);

    /* create the point values for this graph */
    for(i=0;i<ipoints; i++)
    {
	graphdata->x[i] = (float)i;
	graphdata->y[i] = sin(ajCvtDegToRad(i));
    }

    /* embGraphSetData(graphdata,&array[0][0]);*/
    ajGraphdataSetYlabelC(graphdata,"SINE(degrees)");
    ajGraphdataSetXlabelC(graphdata,"degrees");
    ajGraphdataSetTitleC(graphdata,"hello");
    ajGraphdataSetColour(graphdata,GREEN);

    if(!overlap)
    {
	ajGraphdataAddposRect(graphdata,70.0,0.55,80.0,0.45,GREEN,1);
	ajGraphdataAddposTextC(graphdata,82.0,0.5,GREEN,"Sine");
    }

    graphdata = ajGraphdataNewI(ipoints);

    ajGraphDataAdd(mult,graphdata);

    for(i=0;i<ipoints; i++)
    {
	graphdata->x[i] = (float)i;
	graphdata->y[i] = cos(ajCvtDegToRad((float)i));
    }

    ajGraphdataSetXlabelC(graphdata,"degrees");
    ajGraphdataSetYlabelC(graphdata,"COS(degrees)");
    ajGraphdataSetTitleC(graphdata,"hello");
    ajGraphdataSetColour(graphdata,RED);

    if(!overlap)
    {
	ajGraphdataAddposLine(graphdata,5.0,0.1,15.0,0.1,RED);
	ajGraphdataAddposTextC(graphdata,17.0,0.1,RED,"Cosine");
    }

    /* now set larger axis than needed */
    ajGraphdataSetMinmax(graphdata,0.0,(float)ipoints,-0.5,1.5);

    graphdata = ajGraphdataNewI(ipoints);
    ajGraphdataSetLinetype(graphdata, 3);
    ajGraphDataAdd(mult,graphdata);

    for(i=0;i<ipoints; i++)
    {
	graphdata->x[i] = (float)i;
	graphdata->y[i] = (tan(ajCvtDegToRad(i))*0.2);
    }

    ajGraphdataSetXlabelC(graphdata,"degrees");
    ajGraphdataSetYlabelC(graphdata,"TAN(degrees)");
    ajGraphdataSetTitleC(graphdata,"hello");
    ajGraphdataSetLinetype(graphdata, 2);
    ajGraphdataSetColour(graphdata,BLUE);

    if(!overlap)
    {
	ajGraphdataAddposRect(graphdata,5.0,9.0,15.0,8.5,BLUE,0);
	ajGraphdataAddposTextC(graphdata,17.0,8.75,BLUE,"Tangent");
    }

    ajGraphSetYlabelC(mult,"sin,cos,tan");
    ajGraphSetXlabelC(mult,"degrees");
    ajGraphSetTitleC(mult,"Trig functions");

    ajGraphxySetYstartF(mult,0.0);
    ajGraphxySetYendF(mult,2.0);

    if(overlap)
    {
	ajGraphAddRect(mult,5.0,9.0,15.0,8.5,BLUE,0);
	ajGraphAddRect(mult,5.0,8.0,15.0,7.5,GREEN,1);
	ajGraphAddLine(mult,5.0,6.75,15.0,6.75,RED);
	ajGraphAddTextC(mult,17.0,8.75,BLUE,"Tangent");
	ajGraphAddTextC(mult,17.0,7.75,GREEN,"Sine");
	ajGraphAddTextC(mult,17.0,6.75,RED,"Cosine");
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





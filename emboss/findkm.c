/* @source findkm.c
**
** @author Copyright (C) Sinead O'Leary (soleary@hgmp.mrc.ac.uk),
** David Martin (david.martin@biotek.uio.no)
**
** Application to calculate the Michaelis Menton Constants (Km) of different
** enzymes and their substrates.
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
#include <math.h>
#include <stdlib.h>
#include <limits.h>




static float findkm_summation(const float *arr, ajint number);
static float findkm_multisum (const float *arr1, const float *arr2,
			      ajint number);
static float findkm_findmax(const float *arr1, ajint number);
static float findkm_findmin(const float *arr1, ajint number);




/* @prog findkm ***************************************************************
**
** Find Km and Vmax for an enzyme reaction by a Hanes/Woolf plot
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPFile infile = NULL;
    AjPFile outfile = NULL;
    AjPStr line;
    AjPGraph graphLB = NULL;
    AjPGraphdata xygraph  = NULL;
    AjPGraphdata xygraph2 = NULL;
    AjBool doplot;

    ajint N=0;

    float *xdata = NULL;
    float *ydata = NULL;
    float *V = NULL;
    float *S = NULL;

    float a;
    float b;
    float upperXlimit;
    float upperYlimit;

    float A;
    float B;
    float C;
    float D;
    float xmin;
    float xmax;
    float ymin;
    float ymax;
    float xmin2;
    float xmax2;
    float ymin2;
    float ymax2;

    float Vmax;
    float Km;
    float cutx;
    float cuty;

    float amin = 0.;
    float amax = 0.;
    float bmin = 0.;
    float bmax = 0.;


    embInit("findkm", argc, argv);

    infile  = ajAcdGetInfile("infile");
    outfile = ajAcdGetOutfile ("outfile");
    doplot  = ajAcdGetBoolean("plot");
    graphLB = ajAcdGetGraphxy("graphLB");
    line = ajStrNew();


    /* Determine N by reading infile */

    while(ajReadlineTrim(infile, &line))
        if(ajStrGetLen(line) >0)
	    N++;


    /* only allocate memory to the arrays */

    AJCNEW(xdata, N);
    AJCNEW(ydata, N);
    AJCNEW(S, N);
    AJCNEW(V, N);

    ajFileSeek(infile, 0L, 0);

    N=0;
    while(ajReadlineTrim(infile, &line))
    {
	if(ajStrGetLen(line) > 0)
        {
            sscanf(ajStrGetPtr(line),"%f %f",&S[N],&V[N]);
            if(S[N] > 0.0 && V[N] > 0.0)
            {
                xdata[N] = S[N];
                ydata[N] = S[N]/V[N];
		N++;
            }
        }
    }


    /* find the max and min values for the graph parameters*/
    xmin = (float)0.5*findkm_findmin(xdata, N);
    xmax = (float)1.5*findkm_findmax(xdata, N);
    ymin = (float)0.5*findkm_findmin(ydata, N);
    ymax = (float)1.5*findkm_findmax(ydata, N);

    xmin2 = (float)0.5*findkm_findmin(S, N);
    xmax2 = (float)1.5*findkm_findmax(S, N);
    ymin2 = (float)0.5*findkm_findmin(V, N);
    ymax2 = (float)1.5*findkm_findmax(V, N);



    /*
    ** In case the casted ints turn out to be same number on the axis,
    ** make the max number larger than the min so graph can be seen.
    */

    if((ajint)xmax == (ajint)xmin)
        ++xmax;
    if((ajint)ymax == (ajint)ymin)
        ++ymax;


    if((ajint)xmax2 == (ajint)xmin2)
        ++xmax2;
    if((ajint)ymax2 == (ajint)ymin2)
        ++ymax2;



    /*
    ** Gaussian Elimination for Best-fit curve plotting and
    ** calculating Km and Vmax
    */

    A = findkm_summation(xdata, N);
    B = findkm_summation(ydata, N);

    C = findkm_multisum(xdata, ydata, N);
    D = findkm_multisum(xdata, xdata, N);


    /*
    ** To find the best fit line, Least Squares Fit:    y =ax +b;
    ** Two Simultaneous equations, REARRANGE FOR b
    **
    ** findkm_summation(ydata, N) - findkm_summation(xdata,N)*a - N*b =0;
    ** b = (findkm_summation(ydata,N) - findkm_summation(xdata,N)*a) /  N;
    ** b = (B - A*a)/ N;
    **
    ** C - D*a - A*((B - A*a)/ N) =0;
    ** C - D*a - A*B/N + A*A*a/N =0;
    ** C - A*B/N = D*a - A*A*a/N;
    */

    /* REARRANGE FOR a */

    a = (N*C - A*B)/ (N*D - A*A);
    b = (B - A*a)/ N;

    /*
    ** Equation of Line - Lineweaver burk eqn
    ** 1/V = (Km/Vmax)*(1/S) + 1/Vmax;
    */


    Vmax = 1/a;
    Km = b/a;

    cutx = -1/Km;
    cuty = Km/Vmax;

    /* set limits for last point on graph */

    upperXlimit = findkm_findmax(xdata,N)+3;
    upperYlimit = (upperXlimit)*a + b;

    ajFmtPrintF(outfile, "---Hanes Woolf Plot Calculations---\n");
    ajFmtPrintF(outfile, "Slope of best fit line is a = %.2f\n", a);
    ajFmtPrintF(outfile,"Coefficient in Eqn of line y = ma +b is b "
		"= %.2f\n", b);

    ajFmtPrintF(outfile, "Where line cuts x axis = (%.2f, 0)\n", cutx);
    ajFmtPrintF(outfile, "Where line cuts y axis = (0, %.2f)\n", cuty);
    ajFmtPrintF(outfile, "Limit-point of graph for plot = (%.2f, %.2f)\n\n",
		upperXlimit, upperYlimit);
    ajFmtPrintF(outfile, "Vmax = %.2f, Km = %f\n",Vmax, Km);

    /* draw graphs */

    if(doplot)
    {
	xygraph = ajGraphdataNewI(N);
	ajGraphdataAddXY(xygraph, S, V);
	ajGraphDataAdd(graphLB, xygraph);
	ajGraphdataSetTitleC(xygraph, "Michaelis Menten Plot");
	ajGraphdataSetXlabelC(xygraph, "[S]");
	ajGraphdataSetYlabelC(xygraph, "V");

	ajGraphxySetXstartF(graphLB, 0.0);
	ajGraphxySetXendF(graphLB, xmax2);
	ajGraphxySetYstartF(graphLB, 0.0);
	ajGraphxySetYendF(graphLB, ymax2);
	ajGraphxySetXrangeII(graphLB, (ajint)0.0, (ajint)xmax2);
	ajGraphxySetYrangeII(graphLB, (ajint)0.0, (ajint)ymax2);
	ajGraphdataAddposLine(xygraph, 0.0, 0.0, S[0], V[0], (ajint)BLACK);
	ajGraphxyShowPointsCircle(graphLB, ajTrue);
	ajGraphdataSetMinmax(xygraph,0.0,xmax2,0.0,ymax2);


	ajGraphicsCalcRange(S,N,&amin,&amax);
	ajGraphicsCalcRange(V,N,&bmin,&bmax);
	ajGraphdataSetTruescale(xygraph,amin,amax,bmin,bmax);
	ajGraphdataSetTypeC(xygraph,"2D Plot Float");

	xygraph2 = ajGraphdataNewI(N);
	ajGraphdataAddXY(xygraph2, xdata, ydata);
	ajGraphDataAdd(graphLB, xygraph2);

	ajGraphdataSetTitleC(xygraph2, "Hanes Woolf Plot");
	ajGraphdataSetXlabelC(xygraph2, "[S]");
	ajGraphdataSetYlabelC(xygraph2, "[S]/V");

	ajGraphxySetXstartF(graphLB, cutx);
	ajGraphxySetXendF(graphLB, upperXlimit);
	ajGraphxySetYstartF(graphLB, 0.0);
	ajGraphxySetYendF(graphLB, upperYlimit);
	ajGraphxySetXrangeII(graphLB, (ajint)cutx, (ajint)upperXlimit);
	ajGraphxySetYrangeII(graphLB, (ajint)0.0, (ajint)upperYlimit);

	ajGraphxyShowPointsCircle(graphLB, ajTrue);
	ajGraphdataSetMinmax(xygraph2, cutx,upperXlimit,0.0,upperYlimit);
	ajGraphicsCalcRange(xdata,N,&amin,&amax);
	ajGraphicsCalcRange(ydata,N,&bmin,&bmax);
	ajGraphdataSetTruescale(xygraph2,amin,amax,bmin,bmax);
	ajGraphdataSetTypeC(xygraph2,"2D Plot");



	ajGraphSetTitleC(graphLB,"FindKm");
	ajGraphxySetflagOverlay(graphLB,ajFalse);
	ajGraphxyDisplay(graphLB, ajTrue);
    }

    AJFREE(xdata);
    AJFREE(ydata);

    AJFREE(S);
    AJFREE(V);

    ajFileClose(&infile);
    ajFileClose(&outfile);

    ajGraphxyDel(&graphLB);
    ajStrDel(&line);

    embExit();

    return 0;
}




/* @funcstatic findkm_summation ***********************************************
**
** Undocumented.
**
** @param [r] arr [const float*] Undocumented
** @param [r] number [ajint] Array size
** @return [float] Undocumented
** @@
******************************************************************************/


static float findkm_summation(const float *arr, ajint number)
{
    ajint i;
    float sum=0;

    for(i = 0; i < number; i++)
        sum += arr[i];

    return sum;
}




/* @funcstatic findkm_multisum ************************************************
**
** Undocumented.
**
** @param [r] arr1 [const float*] Undocumented
** @param [r] arr2 [const float*] Undocumented
** @param [r] number [ajint] Array size
** @return [float] Undocumented
** @@
******************************************************************************/

static float findkm_multisum(const float *arr1, const float *arr2,
			     ajint number)
{
    ajint i;
    float sum = 0;

    for(i = 0; i < number; i++)
        sum += arr1[i]*arr2[i];

    return sum;
}




/* @funcstatic findkm_findmax *************************************************
**
** Undocumented.
**
** @param [r] arr [const float*] Undocumented
** @param [r] number [ajint] Array size
** @return [float] Undocumented
** @@
******************************************************************************/

static float findkm_findmax(const float *arr, ajint number)
{
    ajint i;
    float max;

    max = arr[0];

    for(i=1; i<number; ++i)
        if(arr[i] > max)
            max = arr[i];

    return max;
}




/* @funcstatic findkm_findmin *************************************************
**
** Undocumented.
**
** @param [r] arr [const float*] Undocumented
** @param [r] number [ajint] Array size
** @return [float] Undocumented
** @@
******************************************************************************/

static float findkm_findmin(const float *arr, ajint number)
{
    ajint i;
    float min;

    min = arr[0];

    for(i=1; i<number; ++i)
        if(arr[i] < min)
            min = arr[i];

    return min;
}

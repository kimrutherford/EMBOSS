/* @source isochore application
**
** Finds isochores
**
** @author Copyright (C) Peter Rice
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
#ifndef NO_PLOT
#include "ajgraph.h"
#endif




/* @datastatic AjPIntarr ******************************************************
**
** Integer array
**
** @alias AjSIntarr
** @alias AjPIntarr
**
** @attr Array [ajint*] Integer array
** @attr Size [ajint] Size
** @attr Padding [char[4]] Padding to alignment boundary
******************************************************************************/

typedef struct AjSIntarr
{
    ajint* Array;
    ajint Size;
    char Padding[4];
} AjOIntarr;
#define AjPIntarr AjOIntarr*




/* @datastatic IsochorePFltarr ************************************************
**
** Integer array
**
** @alias IsochoreSFltarr
** @alias IsochorePFltarr
**
** @attr Array [float*] Float array
** @attr Size [ajint] Size
** @attr Padding [char[4]] Padding to alignment boundary
******************************************************************************/

typedef struct IsochoreSFltarr
{
    float* Array;
    ajint Size;
    char Padding[4];
} IsochoreOFltarr;
#define IsochorePFltarr IsochoreOFltarr*



static void isochore_FltarrDel(IsochorePFltarr *parr);
static IsochorePFltarr isochore_FltarrNew0(size_t size);




/* @prog isochore *************************************************************
**
** Calculates the G+C content of a DNA sequence
** by sliding a window of size "iwin" in increments of "ishift" bases
** at a time.
** Results are stored in float array "results" with one position for
** each calculated value.
**
** Results are also written to an output file with tab delimiters.
**
** To plot this, start at the centre of the first window (0 + iwin/2)
** and plot a point from results0>Array[0] every "ishift" bases until
** the end (use isize).
**
** Future changes: Users should be able to ask for a sequence range
** and plot just that range. Currently a range can be set on the
** command line but it is ignored. The range is from "ajSeqGetBegin(seq)"
** to "ajSeqGetEnd(seq)".
**
******************************************************************************/


int main(int argc, char **argv)
{
    AjPSeq seq;
    AjPFile out;
    IsochorePFltarr results;

    AjPGraph plot;
    AjPGraphPlpData graphdata;

    ajint iwin;
    ajint ishift;
    ajint i;
    ajint j;
    ajint k;
    ajint ipos;
    ajint isize;
    const char *sq;
    ajint igc;
    ajint imax;
    ajint ibeg;
    ajint iend;
    ajint ilen;
    float amin = 0.;
    float amax = 0.;

    ajGraphInit("isochore", argc, argv);

    seq  = ajAcdGetSeq("sequence");
    out  = ajAcdGetOutfile("outfile");
    plot = ajAcdGetGraphxy("graph");

    ajGraphInitSeq(plot, seq);

    sq = ajStrGetPtr(ajSeqGetSeqS(seq));

    ibeg = ajSeqGetBegin(seq);
    iend = ajSeqGetEnd(seq);
    ilen = ajSeqGetLen(seq);

    iwin = ajAcdGetInt("window");
    ishift = ajAcdGetInt("shift");

    imax = iend +  iwin/2;		/* stop at imax */
    if(imax > ilen)
	imax = ilen;

    i = ibeg - iwin/2;			/* start calculating from i */
    if(i < 0)
	i = 0;

    isize = 1 + (imax - iwin - i)/ishift; /* size of results array */
    results = isochore_FltarrNew0(isize);

    ajDebug("ilen: %d ibeg: %d iend: %d\n", ilen, ibeg, iend);
    ajDebug("iwin: %d ishift: %d isize: %d imax: %d i: %d\n",
	    iwin, ishift, isize, imax, i);

    ipos = i + iwin/2;
    ajFmtPrintF(out, "Position\tPercent G+C %d .. %d\n",
		ibeg, iend);

    for(j=0; j < isize; i+=ishift, j++)
    {					/* sum over window */
	igc = 0;
	for(k=0; k < iwin; k++)
	    if(strchr("CcGg", sq[i+k]))
		igc++;

	results->Array[j] = (float) igc / (float) iwin;
	ajFmtPrintF(out, "%d\t%.3f\n", ipos, results->Array[j]);
	ipos += ishift;
    }


    ajFileClose(&out);


#ifndef NO_PLOT
    i = ibeg - iwin/2;			/* start calculating from i */
    if(i < 0)
	i = 0;
    ipos = i + iwin/2;

    /* create the graph */

    graphdata = ajGraphPlpDataNew();

    ajGraphArrayMaxMin(results->Array,isize,&amin,&amax);

    ajGraphPlpDataSetMaxima(graphdata,(float)ipos,(float)(ipos+(ishift*isize)),
			   amin,amax);
    ajGraphPlpDataSetMaxMin(graphdata,(float)ipos,(float)(ipos+(ishift*isize)),
			   amin,amax);
    ajGraphPlpDataSetTypeC(graphdata,"2D Plot");
    ajGraphPlpDataSetTitleC(graphdata,"");



    ajGraphDataAdd(plot,graphdata);
    ajGraphPlpDataCalcXY(graphdata, isize,(float)(ipos),(float)ishift,
			    results->Array);


    /* display the region 0 -> 1 for the y axis */
    ajGraphxySetYStart(plot,0.0);
    ajGraphxySetYEnd(plot,1.0);

    /* draw the graph */
    ajGraphxyDisplay(plot,AJTRUE);

    /* Delete the structures and data */
    ajGraphxyDel(&plot);
#endif



     /*
     ** plot is an XY graph definition object created by acdSetGraphxy
     ** something like this to plot the data:
     ** sequence, startposition, increment, array, arraysize
     ** Note: seq has Begin and End values which can limit the plot range
     **
     ** ioff = ibeg + iwin/2;
     ** ajPlotInit(plot, seq);
     ** ajPlotFloat(plot, ioff, ishift, results->Array, isize);
     **
     */

    ajSeqDel(&seq);
    ajFileClose(&out);
    ajGraphxyDel(&plot);
    isochore_FltarrDel(&results);
    embExit();

    return 0;
}





/* @funcstatic isochore_FltarrNew0 ********************************************
**
** Undocumented.
**
** @param [r] size [size_t] Undocumented
** @return [IsochorePFltarr] Undocumented
** @@
******************************************************************************/

static IsochorePFltarr isochore_FltarrNew0(size_t size)
{
    IsochorePFltarr ret;

    AJNEW(ret);
    ret->Size = (ajint) size;
    AJCNEW(ret->Array,size);

    return ret;
}




/* @funcstatic isochore_FltarrDel *********************************************
**
** Undocumented.
**
** @param [d] parr [IsochorePFltarr*] Undocumented
** @return [void]
** @@
******************************************************************************/

static void isochore_FltarrDel(IsochorePFltarr *parr)
{
    if(!*parr) return;
    AJFREE((*parr)->Array);
    AJFREE(*parr);

    return;
}

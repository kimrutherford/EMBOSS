/* @source hmoment application
**
** Calculate hydrophobic moment
**
** @author Copyright (C) Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
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
#include <math.h>




static float hmoment_calchm(const char *p, int pos, int window, ajint angle);
static void  hmoment_addgraph(AjPGraph graph, ajint limit,
			      const float *x, const float *y,
			      float ymax, ajint colour, ajint angle,
			      ajint window, float baseline, const char *sname);




/* @prog hmoment **************************************************************
**
** Hydrophobic moment calculation
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall  seqall;
    AjPSeq seq;
    AjPFile outf;
    AjPStr str = NULL;
    AjPStr st  = NULL;

    AjBool plot;
    AjPGraph graph = NULL;
    AjBool twin;
    float baseline;
    float ymax;

    ajint beg;
    ajint end;
    ajint window;
    ajint len;
    ajint limit;

    ajint aangle;
    ajint bangle;

    ajint i;

    float *x  = NULL;
    float *ya = NULL;
    float *yb = NULL;

    const char *p;
    const char *sname;


    ajGraphInit("hmoment", argc, argv);

    seqall    = ajAcdGetSeqall("seqall");
    plot      = ajAcdGetToggle("plot");
    window    = ajAcdGetInt("window");
    aangle    = ajAcdGetInt("aangle");
    bangle    = ajAcdGetInt("bangle");
    baseline  = ajAcdGetFloat("baseline");
    twin      = ajAcdGetBoolean("double");

    /* only one will be used - see variable 'plot' */

    outf  = ajAcdGetOutfile("outfile");
    graph = ajAcdGetGraphxy("graph");


    str = ajStrNew();
    st  = ajStrNew();

    while(ajSeqallNext(seqall, &seq))
    {
	beg = ajSeqallGetseqBegin(seqall);
	end = ajSeqallGetseqEnd(seqall);
	len = end-beg+1;

	limit = len-window+1;
	sname = ajSeqGetNameC(seq);

	ajStrAssignSubC(&str,ajSeqGetSeqC(seq),--beg,--end);
	ajStrFmtUpper(&str);
	p = ajStrGetPtr(str);

	if(limit>0)
	{
	    AJCNEW0(x,limit);
	    AJCNEW0(ya,limit);
	    AJCNEW0(yb,limit);
	}

	for(i=0;i<limit;++i)
	{
	    x[i] = (float)i+1+beg;
	    ya[i] = hmoment_calchm(p,i,window,aangle);
	    yb[i] = hmoment_calchm(p,i,window,bangle);
	}

	for(i=0,ymax=-100.;i<limit;++i)
	{
	    ymax = ymax > ya[i] ? ymax : ya[i];
	    if(twin)
		ymax = ymax > yb[i] ? ymax : yb[i];
	}

	if(!plot && outf)
	{
	    ajFmtPrintF(outf,"HMOMENT of %s from %d to %d\n\n",sname,
			beg+1,end+1);
	    if(twin)
	    {
		ajFmtPrintF(outf,"Window: %d First angle: %d second angle:"
			    " %d Max uH: %.3f\n",window, aangle, bangle,
			    ymax);
		ajFmtPrintF(outf,"Position\tFirst\tSecond\n");
		for(i=0;i<limit;++i)
		    ajFmtPrintF(outf,"%d\t\t%.3lf\t%.3lf\n",(int)x[i],ya[i],
				yb[i]);
	    }
	    else
	    {
		ajFmtPrintF(outf,"Window: %d Angle: %d Max uH: %.3f\n",
			    window, aangle, ymax);
		ajFmtPrintF(outf,"Position\tuH\n");
		for(i=0;i<limit;++i)
		    ajFmtPrintF(outf,"%d\t\t%.3lf\n",(int)x[i],ya[i]);
	    }

	}
	else if (plot)
	{
	    if(twin)
		ajGraphSetMulti(graph,2);
	    else
		ajGraphSetMulti(graph,1);

	    ajGraphxySetOverLap(graph,ajFalse);

	    hmoment_addgraph(graph,limit,x,ya,ymax,BLACK,aangle,window,
			     baseline, sname);
	    if(twin)
		hmoment_addgraph(graph,limit,x,yb,ymax,RED,bangle,window,
				 baseline, sname);

	    if(limit>0)
		ajGraphxyDisplay(graph,ajFalse);
	}

	if(limit>0)
	{
	    AJFREE(x);
	    AJFREE(ya);
	    AJFREE(yb);
	}
    }

    ajGraphClose();
    ajGraphxyDel(&graph);
    ajFileClose(&outf);

    ajStrDel(&str);
    ajStrDel(&st);

    ajSeqallDel(&seqall);
    ajSeqDel(&seq);
    embExit();

    return 0;
}




/* @funcstatic hmoment_addgraph ***********************************************
**
** Undocumented.
**
** @param [u] graph [AjPGraph] Undocumented
** @param [r] limit [ajint] Undocumented
** @param [r] x [const float*] Undocumented
** @param [r] y [const float*] Undocumented
** @param [r] ymax [float] Undocumented
** @param [r] colour [ajint] Undocumented
** @param [r] angle [ajint] Undocumented
** @param [r] window [ajint] Undocumented
** @param [r] baseline [float] Undocumented
** @param [r] sname [const char*] Sequence name
** @@
******************************************************************************/


static void hmoment_addgraph(AjPGraph graph, ajint limit,
			     const float *x, const float *y,
			     float ymax, ajint colour, ajint angle,
			     ajint window, float baseline, const char *sname)
{
    ajint i;

    AjPGraphPlpData data;
    AjPStr st = NULL;

    if(limit<1)
	return;


    data = ajGraphPlpDataNewI(limit);

    st = ajStrNew();

    for(i=0;i<limit;++i)
    {
	data->x[i] = x[i];
	data->y[i] = y[i];
    }

    ajGraphPlpDataSetColour(data,colour);
    ajGraphPlpDataSetMaxMin(data,x[0],x[limit-1],0.,ymax);
    ajGraphPlpDataSetMaxima(data,x[0],x[limit-1],0.,ymax);

    ajGraphPlpDataSetTypeC(data,"2D Plot Float");

    ajFmtPrintS(&st,"HMOMENT of %s. Window:%d",sname,window);
    ajGraphPlpDataSetTitle(data,st);

    ajFmtPrintS(&st,"uH (%d deg)",angle);
    ajGraphPlpDataSetYTitle(data,st);

    ajFmtPrintS(&st,"Position (w=%d)",window);
    ajGraphPlpDataSetXTitle(data,st);

    ajGraphPlpDataAddLine(data,x[0],baseline,x[limit-1],baseline,BLUE);

    ajGraphDataAdd(graph,data);

    ajStrDel(&st);

    return;
}




/* @funcstatic hmoment_calchm *************************************************
**
** Undocumented.
**
** @param [r] p [const char*] Undocumented
** @param [r] pos [int] Undocumented
** @param [r] window [int] Undocumented
** @param [r] angle [ajint] Undocumented
** @return [float] Undocumented
** @@
******************************************************************************/


static float hmoment_calchm(const char *p, int pos, int window, ajint angle)
{
    ajint  i;
    double h;
    ajint  res;
    double sumsin;
    double sumcos;
    double tangle;
    double hm;

/*
** B interpolated from D and N using Dayhoff frequencies
** Z interpolated from E and Q using Dayhoff frequencies
** J interpolated from I and L using Dayhoff frequencies
** X average of all using Dayhoff frequencies
** O and U set to X due to lack of available data
*/
    double hydata[]=
    {
	 0.62, -0.84,  0.29, -0.90, -0.74,  1.19,  0.48, -0.40, /* ABCDEFGH */
	 1.38,  1.18, -1.50,  1.06,  0.64, -0.78, -0.68,  0.12, /* IJKLMNOP */
	-0.85, -2.53, -0.18, -0.05, -0.68,  1.08,  0.81, -0.68, /* QRSTUVWX */
	 0.26, -0.78			/* YZ */
    };

    sumsin = sumcos = (double)0.;
    tangle = angle;

    for(i=0;i<window;++i)
    {
	res = p[pos+i];
	h   = hydata[ajBasecodeToInt(res)];

	sumsin  += (h * sin(ajDegToRad((float)tangle)));
	sumcos  += (h * cos(ajDegToRad((float)tangle)));
	tangle = (double) (((ajint)tangle+angle) % 360);
    }


    sumsin *= sumsin;
    sumcos *= sumcos;


    hm = sqrt(sumsin+sumcos) / (double)window;

    return (float)hm;
}

/* @source freak application
**
** Calculate residue frequencies using sliding window
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




/* @prog freak ****************************************************************
**
** Residue/base frequency table or plot
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall  seqall;
    AjPSeq seq;
    AjPFile outf;
    AjPStr bases = NULL;
    AjPStr str   = NULL;
    AjBool plot;
    AjPGraph graph=NULL;
    AjPGraphdata fgraph=NULL;
    AjPStr st = NULL;

    ajint c;
    ajint pos;
    ajint end;
    ajint step;
    ajint window;
    ajint t;

    ajint i;
    ajint j;
    ajint k;
    const char *p;
    const char *q;
    float f;

    float *x = NULL;
    float *y = NULL;

    float max = 0.;
    float min = 0.;


    embInit("freak", argc, argv);

    seqall = ajAcdGetSeqall("seqall");
    plot   = ajAcdGetToggle("plot");
    step   = ajAcdGetInt("step");
    window = ajAcdGetInt("window");
    bases  = ajAcdGetString("letters");

    /* only one will be used - see variable 'plot' */

    outf  = ajAcdGetOutfile("outfile");
    graph = ajAcdGetGraphxy("graph");


    st = ajStrNew();

    ajStrFmtUpper(&bases);
    q = ajStrGetPtr(bases);

    while(ajSeqallNext(seqall, &seq))
    {
	pos = ajSeqallGetseqBegin(seqall);
	end = ajSeqallGetseqEnd(seqall);

	str = ajSeqGetSeqCopyS(seq);
	ajStrFmtUpper(&str);
	p = ajStrGetPtr(str);

	c = 0;
	--pos;
	--end;
	t = pos;
	while(t+window <= end+1)
	{
	    ++c;
	    t += step;
	}


	if(c)
	{
	    x = (float *) AJALLOC(c * sizeof(float));
	    y = (float *) AJALLOC(c * sizeof(float));
	}


	for(i=0;i<c;++i)
	{
	    t = i*step+pos;
	    x[i] = (float)(t+1) /*+ window/2.0*/;
	    f = 0.;
	    for(j=0;j<window;++j)
	    {
		k = t + j;
		if(strchr(q,p[k]))
		    ++f;
	    }
	    y[i] = f / (float)window;
	}

	if(!plot && c && outf)
	{
	    ajFmtPrintF(outf,"FREAK of %s from %d to %d Window %d Step %d\n\n",
			ajSeqGetNameC(seq),pos+1,end+1,window,step);
	    for(i=0;i<c;++i)
		ajFmtPrintF(outf,"%-10d %f\n",(ajint)x[i],y[i]);
	}
	else if(plot && c)
	{
	    fgraph = ajGraphdataNewI(c);
	    ajGraphSetTitleS(graph,ajSeqGetNameS(seq));
	    ajFmtPrintS(&st,"From %d to %d. Residues:%s Window:%d Step:%d",
			pos+1,end+1,ajStrGetPtr(bases),window,step);
	    ajGraphSetSubtitleS(graph,st);
	    ajGraphSetXlabelC(graph,"Position");
	    ajGraphSetYlabelC(graph,"Frequency");
	    ajGraphxySetXstartF(graph,x[0]);
	    ajGraphxySetXendF(graph,x[c-1]);
	    ajGraphxySetYstartF(graph,0.);
	    ajGraphxySetYendF(graph,y[c-1]);
	    ajGraphxySetXrangeII(graph,(ajint)x[0],(ajint)x[c-1]);
	    ajGraphxySetYrangeII(graph,0,(ajint)y[c-1]);
	    ajGraphdataSetMinmax(fgraph,x[0],x[c-1],0.,1.0);
	    ajGraphicsCalcRange(y,c,&min,&max);
	    ajGraphdataSetTruescale(fgraph,x[0],x[c-1],min,max);
	    ajGraphdataSetTypeC(fgraph,"2D Plot");

	    ajGraphdataAddXY(fgraph,x,y);
	    ajGraphDataReplace(graph,fgraph);
	    ajGraphxyDisplay(graph,ajFalse);
	}


	AJFREE(x);
	AJFREE(y);
	ajStrDel(&str);
    }

    ajGraphicsClose();
    ajGraphxyDel(&graph);

    ajFileClose(&outf);
    ajSeqallDel(&seqall);
    ajSeqDel(&seq);

    ajStrDel(&str);
    ajStrDel(&bases);
    ajStrDel(&st);

    embExit();

    return 0;
}

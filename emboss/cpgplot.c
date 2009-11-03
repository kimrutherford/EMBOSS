/* @source cpgplot application
**
** Plots CpG island areas
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
#include <stdlib.h>




static void cpgplot_findbases(const AjPStr substr, ajint len,
			      ajint window, ajint shift, float *obsexp,
			      float *xypc, const AjPStr bases,
			      float *obsexpmax,
			      ajint *plstart, ajint *plotend);

static void cpgplot_countbases(const char *seq, const char *bases,
			       ajint window,
			       float *cx, float *cy, float *cxpy);

static void cpgplot_identify(AjPFile outf,
			     const float *obsexp, const float *xypc,
			     AjBool *thresh, ajint begin, ajint len,
			     ajint shift, const char *bases, const char *name,
			     ajint minlen, float minobsexp, float minpc,
			     AjPFeattabOut featout);

static void cpgplot_reportislands(AjPFile outf, const AjBool *thresh,
				  const char *bases, const char *name,
				  float minobsexp, float minpc,
				  ajint minlen, ajint begin, ajint len);

static void cpgplot_plotit(const char *seq,
			   ajint begin, ajint len,
			   const float *obsexp, const float *xypc,
			   const AjBool *thresh,
			   float obsexpmax, AjBool doobsexp,
			   AjBool docg, AjBool dopc, AjPGraph mult);

static void cpgplot_dumpfeatout(AjPFeattabOut featout, const AjBool *thresh,
				const char *seqname, ajint begin, ajint len);




/* @prog cpgplot **************************************************************
**
** Plot CpG rich areas
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPSeq seq    = NULL;
    AjPFile outf  = NULL;
    AjPStr strand = NULL;
    AjPStr substr = NULL;
    AjPStr bases  = NULL;
    AjPGraph mult;
    AjBool doobsexp;
    AjBool dopc;
    AjBool docg;

    ajint begin;
    ajint end;
    ajint len;

    ajint minlen;
    float minobsexp;
    float minpc;
    AjBool doplot;

    ajint window;
    ajint shift;
    ajint plotstart;
    ajint plotend;

    float  *xypc   = NULL;
    float  *obsexp = NULL;
    AjBool *thresh = NULL;
    float  obsexpmax;

    ajint i;
    ajint maxarr;
    AjPFeattabOut featout = NULL;


    ajGraphInit("cpgplot",argc,argv);

    seqall    = ajAcdGetSeqall("sequence");
    window    = ajAcdGetInt("window");
    shift     = 1;     /* other values broken - needs rewrite to fix*/
    outf      = ajAcdGetOutfile("outfile");
    minobsexp = ajAcdGetFloat("minoe");
    minlen    = ajAcdGetInt("minlen");
    minpc     = ajAcdGetFloat("minpc");
    doplot    = ajAcdGetToggle("plot");
    mult      = ajAcdGetGraphxy ("graph");
    doobsexp  = ajAcdGetBoolean("obsexp");
    docg      = ajAcdGetBoolean("cg");
    dopc      = ajAcdGetBoolean("pc");
    featout   = ajAcdGetFeatout("outfeat");



    substr = ajStrNew();
    bases  = ajStrNewC("CG");


    maxarr = 0;

    while(ajSeqallNext(seqall, &seq))
    {
	begin = ajSeqallGetseqBegin(seqall);
	end   = ajSeqallGetseqEnd(seqall);
	strand = ajSeqGetSeqCopyS(seq);
	ajStrFmtUpper(&strand);

	ajStrAssignSubC(&substr,ajStrGetPtr(strand),--begin,--end);
	len = ajStrGetLen(substr);

	if(len > maxarr)
	{
	  AJCRESIZE(obsexp,len);
	  AJCRESIZE(thresh,len);
	  AJCRESIZE(xypc,len);
	}

	for(i=0;i<len;++i)
	{
	    thresh[i] = ajFalse;
	    obsexp[i] = xypc[i] = 0.0;
	}



	cpgplot_findbases(substr, len, window, shift, obsexp, xypc,
			  bases, &obsexpmax, &plotstart, &plotend);


	cpgplot_identify(outf, obsexp, xypc, thresh, 0, len, shift,
			 ajStrGetPtr(bases), ajSeqGetNameC(seq),
			 minlen, minobsexp,
			 minpc, featout);

	if(doplot)
	    cpgplot_plotit(ajSeqGetNameC(seq), begin, len,
			   obsexp, xypc, thresh,
			   obsexpmax, doobsexp, docg, dopc, mult);

	ajStrDel(&strand);
    }

    if(mult)
    {
	ajGraphCloseWin();
	ajGraphxyDel(&mult);
    }

    ajSeqDel(&seq);
    ajStrDel(&substr);
    ajStrDel(&bases);

    ajFileClose(&outf);
    ajSeqallDel(&seqall);
    ajFeattabOutDel(&featout);

    AJFREE(obsexp);
    AJFREE(thresh);
    AJFREE(xypc);

    embExit();

    return 0;
}




/* @funcstatic cpgplot_findbases **********************************************
**
** Undocumented.
**
** @param [r] substr [const AjPStr] Undocumented
** @param [r] len [ajint] Undocumented
** @param [r] window [ajint] Undocumented
** @param [r] shift [ajint] Undocumented
** @param [w] obsexp [float*] Undocumented
** @param [w] xypc [float*] Undocumented
** @param [r] bases [const AjPStr] Undocumented
** @param [w] obsexpmax [float*] Undocumented
** @param [w] plotstart [ajint*] Undocumented
** @param [w] plotend [ajint*] Undocumented
** @@
******************************************************************************/

static void cpgplot_findbases(const AjPStr substr, ajint len,
			      ajint window, ajint shift, float *obsexp,
			      float *xypc, const AjPStr bases,
			      float *obsexpmax,
			      ajint *plotstart, ajint *plotend)
{
    float cxpy;
    float cxf;
    float cyf;
    float windowf;


    float obs;
    float expect;
    ajint i;
    ajint j = 0;
    ajint offset;

    const char *p;
    const char *q;

    windowf    = (float)window;
    *obsexpmax = 0.0;
    offset     = window/2;
    *plotstart   = offset;
    q          = ajStrGetPtr(bases);

    for(i=0; i<(len-window+1);i+=shift)
    {
	j = i+offset;
	p = ajStrGetPtr(substr) + i;
	cpgplot_countbases(p, q, window, &cxf, &cyf, &cxpy);

	obs = cxpy;
	expect = (cxf*cyf)/windowf;
	if(!expect)
	    obsexp[j]=0.0;
	else
	{
	    obsexp[j] = obs/expect;
	    *obsexpmax = (*obsexpmax > obsexp[j]) ? *obsexpmax : obsexp[j];
	}
	xypc[j] = (cxf/windowf)*(float)100.0 + (cyf/windowf)*(float)100.0;
    }

    *plotend = j;

    return;
}




/* @funcstatic cpgplot_countbases *********************************************
**
** Undocumented.
**
** @param [r] seq [const char*] Undocumented
** @param [r] bases [const char*] Undocumented
** @param [r] window [ajint] Undocumented
** @param [w] cx [float*] Undocumented
** @param [w] cy [float*] Undocumented
** @param [w] cxpy [float*] Undocumented
** @@
******************************************************************************/



static void cpgplot_countbases(const char *seq, const char *bases,
			       ajint window,
			       float *cx, float *cy, float *cxpy)
{
    ajint i;

    ajint codex;
    ajint codey;
    ajint codea;
    ajint codeb;

    *cxpy = *cx = *cy = 0.;

    codex = ajBaseAlphaToBin(bases[0]);
    codey = ajBaseAlphaToBin(bases[1]);

    codeb = ajBaseAlphaToBin(seq[0]);

    for(i=0; i<window; ++i)
    {
	codea=codeb;
	codeb=ajBaseAlphaToBin(seq[i+1]);

        if(!(15-codea))   /* look for ambiguity code 'N' */
        {
	    *cx = *cx + (float)0.25;
	    if(!(15-codeb))
		*cxpy = *cxpy + (float)0.0625;
        }
        else
        {
	    if(codea && !(codea & (15-codex)))
	    {
		++*cx;
		if(codeb && !(codeb & (15-codey)))
		    ++*cxpy;
	    }
	    if(codea && !(codea & (15-codey)))
		++*cy;
        }
    }

    return;
}




/* @funcstatic cpgplot_identify ***********************************************
**
**    This subroutine identifies the CpG line, identifying the possible
**    dinucleotide 'islands' in the sequence. These are defined as
**    base positions where, over an average of 10 windows, the calculated
**    % composition is over 50% and the calculated Obs/Exp ratio is over 0.6
**    and the conditions hold for a minimum of 200 bases.
**
** @param [u] outf [AjPFile] Undocumented
** @param [r] obsexp [const float*] Undocumented
** @param [r] xypc [const float*] Undocumented
** @param [w] thresh [AjBool*] Undocumented
** @param [r] begin [ajint] Undocumented
** @param [r] len [ajint] Undocumented
** @param [r] shift [ajint] Undocumented
** @param [r] bases [const char*] Undocumented
** @param [r] name [const char*] Undocumented
** @param [r] minlen [ajint] Undocumented
** @param [r] minobsexp [float] Undocumented
** @param [r] minpc [float] Undocumented
** @param [u] featout [AjPFeattabOut] Undocumented
** @@
******************************************************************************/

static void cpgplot_identify(AjPFile outf,
			     const float *obsexp, const float *xypc,
			     AjBool *thresh, ajint begin, ajint len,
			     ajint shift, const char *bases, const char *name,
			     ajint minlen, float minobsexp, float minpc,
			     AjPFeattabOut featout)
{
    static ajint avwindow = 10;
    float avpc;
    float avobsexp;
    float sumpc;
    float sumobsexp;

    ajint i;
    ajint pos;
    ajint sumlen;
    ajint first = 0;

    for(i=0; i<len; ++i)
	thresh[i] = ajFalse;

    sumlen = 0;
    for(pos=0,first=0;pos<(len-avwindow*shift);pos+=shift)
    {
	sumpc = sumobsexp = 0.0;
	ajDebug("pos: %d max: %d\n", pos, pos+avwindow*shift);
	for(i=pos;i<=(pos+avwindow*shift);++i)
	{
	    ajDebug("obsexp[%d] %.2f xypc[%d] %.2f\n",
		    i, obsexp[i], i, xypc[i]);
	    sumpc     += xypc[i];
	    sumobsexp += obsexp[i];
	}

	avpc     = sumpc/(float)avwindow;
	avobsexp = sumobsexp/(float)avwindow;
	ajDebug("sumpc: %.2f sumobsexp: %.2f\n", sumpc, sumobsexp);
	ajDebug(" avpc: %.2f  avobsexp: %.2f\n", avpc, avobsexp);
	if((avobsexp>minobsexp)&&(avpc>minpc))
	{
	    if(!sumlen)
		first = pos;	/* start a new island */
	    sumlen += shift;
	    ajDebug(" ** hit first: %d sumlen: %d\n", first, sumlen);
	}
	else
	{
	    if(sumlen >= minlen)
		/* island long enough? */
		for(i=first; i<=pos-shift;++i)
		    thresh[i] = ajTrue;

	    sumlen=0;
	}
    }

    if(sumlen>=minlen)
	for(i=first;i<len;++i)
	    thresh[i] = ajTrue;

    cpgplot_reportislands(outf, thresh, bases, name, minobsexp, minpc,
			  minlen, begin, len);

    cpgplot_dumpfeatout(featout,thresh, name, begin, len );

    return;
}




/* @funcstatic cpgplot_reportislands ******************************************
**
** Undocumented.
**
** @param [u] outf [AjPFile] Undocumented
** @param [r] thresh [const AjBool*] Undocumented
** @param [r] bases [const char*] Undocumented
** @param [r] name [const char*] Undocumented
** @param [r] minobsexp [float] Undocumented
** @param [r] minpc [float] Undocumented
** @param [r] minlen [ajint] Undocumented
** @param [r] begin [ajint] Undocumented
** @param [r] len [ajint] Undocumented
** @@
******************************************************************************/

static void cpgplot_reportislands(AjPFile outf, const AjBool *thresh,
				  const char *bases, const char *name,
				  float minobsexp, float minpc,
				  ajint minlen, ajint begin, ajint len)
{
    AjBool island;
    ajint startpos = 0;
    ajint endpos;
    ajint slen;
    ajint i;


    ajFmtPrintF(outf,"\n\nCPGPLOT islands of unusual %s composition\n",
		bases);
    ajFmtPrintF(outf,"%s from %d to %d\n\n",name,begin+1,begin+len);
    ajFmtPrintF(outf,"     Observed/Expected ratio > %.2f\n",minobsexp);
    ajFmtPrintF(outf,"     Percent %c + Percent %c > %.2f\n",bases[0],
		bases[1],minpc);
    ajFmtPrintF(outf,"     Length > %d\n",minlen);

    island = ajFalse;
    for(i=0;i<len;++i)
    {
	if(island)
	{
	    island = thresh[i];
	    if(!island)
	    {
		slen = i - startpos;
		endpos = i;
		ajFmtPrintF(outf,"\n Length %d (%d..%d)\n",slen,
			    startpos+begin+1,endpos+begin);
	    }
	}
	else
	{
	    island = thresh[i];
	    if(island)
		startpos = i;
	}
    }

    if(island)
    {
	slen = len-startpos+1;
	endpos = len-1;
	ajFmtPrintF(outf,"\n Length %d (%d..%d)\n",slen,
		    startpos+begin+1, endpos+begin);
    }

    return;
}




/* @funcstatic cpgplot_plotit *************************************************
**
** Undocumented.
**
** @param [r] seq [const char*] Undocumented
** @param [r] begin [ajint] Undocumented
** @param [r] len [ajint] Undocumented
** @param [r] obsexp [const float*] Undocumented
** @param [r] xypc [const float*] Undocumented
** @param [r] thresh [const AjBool*] Undocumented
** @param [r] obsexpmax [float] Undocumented
** @param [r] doobsexp [AjBool] Undocumented
** @param [r] docg [AjBool] Undocumented
** @param [r] dopc [AjBool] Undocumented
** @param [u] graphs [AjPGraph] Undocumented
** @@
******************************************************************************/

static void cpgplot_plotit(const char *seq,
			   ajint begin, ajint len,
			   const float *obsexp, const float *xypc,
			   const AjBool *thresh,
			   float obsexpmax,
			   AjBool doobsexp, AjBool docg,
			   AjBool dopc, AjPGraph graphs)

{
    AjPGraphPlpData tmGraph  = NULL;
    AjPGraphPlpData tmGraph2 = NULL;
    AjPGraphPlpData tmGraph3 = NULL;
    float *tmp = NULL;
    ajint i;
    float min = 0.;
    float max = 0.;

    ajint igraph=0;

    if(doobsexp)
    {
	tmGraph2 = ajGraphPlpDataNew();
	ajGraphPlpDataSetTitleC(tmGraph2,"Observed vs Expected");
	ajGraphPlpDataSetXTitleC(tmGraph2,"Base number");
	ajGraphPlpDataSetYTitleC(tmGraph2,"Obs/Exp");

	min = 64000.;
	max = -64000.;
	for(i=0;i<len;++i)
	{
	    min = (min<obsexp[i]) ? min : obsexp[i];
	    max = (max>obsexp[i]) ? max : obsexp[i];
	}

	ajGraphPlpDataSetMaxMin(tmGraph2,(float)begin,(float)begin+len-1,
			       min,max);
	ajGraphPlpDataSetMaxima(tmGraph2,(float)begin,(float)begin+len-1,
			       0.0,max);
	ajGraphPlpDataSetTypeC(tmGraph2,"Multi 2D Plot");

	ajGraphxySetXStart(graphs,(float)begin);
	ajGraphxySetXEnd(graphs,(float)(begin+len-1));

	ajGraphxySetYStart(graphs,0.0);
	ajGraphxySetYEnd(graphs,obsexpmax);
	ajGraphxySetXRangeII(graphs,begin,begin+len-1);
	ajGraphxySetYRangeII(graphs,0,(ajint)(obsexpmax+1.0));

	ajGraphPlpDataCalcXY(tmGraph2,len,(float)begin,1.0,obsexp);
	ajGraphDataReplaceI(graphs,tmGraph2,igraph++);
	tmGraph2 = NULL;
    }

    if(dopc)
    {
	tmGraph3 = ajGraphPlpDataNew();
	ajGraphPlpDataSetTitleC(tmGraph3,"Percentage");
	ajGraphPlpDataSetXTitleC(tmGraph3,"Base number");
	ajGraphPlpDataSetYTitleC(tmGraph3,"Percentage");

	min = 64000.;
	max = -64000.;
	for(i=0;i<len;++i)
	{
	    min = (min<xypc[i]) ? min : xypc[i];
	    max = (max>xypc[i]) ? max : xypc[i];
	}

	ajGraphPlpDataSetMaxMin(tmGraph3,(float)begin,(float)begin+len-1,
			       min,max);
	ajGraphPlpDataSetMaxima(tmGraph3,(float)begin,(float)begin+len-1,
			      0.0,max);
	ajGraphPlpDataSetTypeC(tmGraph3,"Multi 2D Plot");

	ajGraphxySetXStart(graphs,(float)begin);
	ajGraphxySetXEnd(graphs,(float)(begin+len-1));
	ajGraphxySetYStart(graphs,0);
	ajGraphxySetYEnd(graphs,100);
	ajGraphxySetXRangeII(graphs,begin,begin+len-1);
	ajGraphxySetYRangeII(graphs,0,100);

	ajGraphPlpDataCalcXY(tmGraph3,len,(float)begin,1.0,xypc);
	ajGraphDataReplaceI(graphs,tmGraph3,igraph++);
	tmGraph3 = NULL;
    }

    if(docg)
    {
	AJCNEW (tmp, len);
	for(i=0;i<len;++i)
	{
	    if(thresh[i])
		tmp[i] = 1.0;
	    else
		tmp[i] = 0.0;
	}

	tmGraph = ajGraphPlpDataNew();
	ajGraphPlpDataSetTitleC(tmGraph,"Putative Islands");
	ajGraphPlpDataSetXTitleC(tmGraph,"Base Number");
	ajGraphPlpDataSetYTitleC(tmGraph,"Threshold");

	ajGraphPlpDataSetMaxMin(tmGraph,(float)begin,(float)begin+len-1,
			       0.,1.);
	ajGraphPlpDataSetTypeC(tmGraph,"Multi 2D Plot");


	ajGraphxySetXStart(graphs,(float)begin);
	ajGraphxySetXEnd(graphs,(float)(begin+len-1));
	ajGraphxySetYStart(graphs,0);
	ajGraphxySetYEnd(graphs,2);
	ajGraphxySetXRangeII(graphs,begin,begin+len-1);
	ajGraphxySetYRangeII(graphs,0,2);
	ajGraphPlpDataSetMaxMin(tmGraph,(float)begin,(float)(begin+len-1),
			       (float) 0.0, (float) 1.2);
	ajGraphPlpDataSetMaxima(tmGraph,(float)begin,(float)(begin+len-1),
			       (float) 0.0, (float) 1.0);

	ajGraphPlpDataCalcXY(tmGraph,len,(float)begin,1.0,tmp);
	ajGraphDataReplaceI(graphs,tmGraph,igraph++);
	tmGraph = NULL;
    }


    if(docg || dopc || doobsexp)
    {
	ajGraphSetTitleC(graphs,seq);
	ajGraphxySetOverLap(graphs,ajFalse);
	ajGraphxyDisplay(graphs, AJFALSE);
    }


    if(docg)
	AJFREE (tmp);

    return;
}




/* @funcstatic cpgplot_dumpfeatout ********************************************
**
** Undocumented.
**
** @param [u] featout [AjPFeattabOut] Undocumented
** @param [r] thresh [const AjBool*] Undocumented
** @param [r] seqname [const char*] Undocumented
** @param [r] begin [ajint] Undocumented
** @param [r] len [ajint] Undocumented
** @@
******************************************************************************/

static void cpgplot_dumpfeatout(AjPFeattabOut featout, const AjBool *thresh,
				const char *seqname, ajint begin, ajint len)
{
    AjBool island;
    ajint startpos = 0;
    ajint endpos;
    ajint i;
    AjPFeattable feattable;
    AjPStr name   = NULL;
    AjPStr source = NULL;
    AjPStr type   = NULL;
    char strand   = '+';
    ajint frame   = 0;
    AjPFeature feature;
    float score = 0.0;

    ajStrAssignC(&name,seqname);

    feattable = ajFeattableNewDna(name);

    ajStrAssignC(&source,"cpgplot");
    ajStrAssignC(&type,"misc_feature");


    island = ajFalse;
    for(i=0;i<len;++i)
    {
	if(island)
	{
	    island = thresh[i];
	    if(!island)
	    {
		endpos = i;
		feature = ajFeatNew(feattable, source, type,
				    startpos+begin+1,endpos+begin,
				    score, strand, frame) ;
		if(!feature)
		  ajDebug("Error feature not added to feature table");
	    }
	}
	else
	{
	    island = thresh[i];
	    if(island)
		startpos = i;
	}
    }

    if(island)
    {
	endpos  = len-1;
	feature = ajFeatNew(feattable, source, type,
			    startpos+begin+1,endpos+begin,
			    score, strand, frame);
    }
    ajFeatSortByStart(feattable);
    ajFeatWrite (featout, feattable);
    ajFeattableDel(&feattable);

    ajStrDel(&source);
    ajStrDel(&type);
    ajStrDel(&name);

    return;
}

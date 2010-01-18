/* @source dan application
**
** Displays and plots nucleic acid duplex melting temperatures
** @author Copyright (C) Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
** Contributions from Michael Schmitz
** @@
**
** Replaces program "MELTDNA/MELTPLOT" by Rodrigo Lopez (EGCG)
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




static void dan_reportgc(const AjPSeq seq,
			 AjPFeattable TabRpt, ajint window,
			 ajint shift, float formamide, float mismatch,
			 ajint prodLen, float dna, float salt,
			 float temperature, AjBool isDNA, AjBool isProduct,
			 AjBool dothermo, AjBool doplot,
			 float xa[], float ta[],
			 float tpa[], float cga[],
			 ajint *npoints);

static void dan_findgc(const AjPStr strand, ajint begin, ajint end,
		       ajint window,
		       ajint shift, float formamide, float mismatch,
		       ajint prodLen, float dna, float salt,
		       float temperature, AjBool isDNA, AjBool isProduct,
		       AjBool dothermo, AjPFile outf, AjBool doplot,
		       float *xa, float *ta, float *tpa, float *cga,
		       ajint *npoints);

static void dan_plotit(const AjPSeq seq, const float *xa, const float *ta,
		       ajint npoints, ajint ibegin, ajint iend,
		       AjPGraph mult, float mintemp);

static void dan_unfmall(float *xa, float *ta, float *tpa, float *cga);




/* @prog dan ******************************************************************
**
** Calculates DNA RNA/DNA melting temperature
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPFile outf = NULL;
    AjPReport report    = NULL;
    AjPFeattable TabRpt = NULL;
    ajint begin;
    ajint end;
    ajint window;
    ajint shift;
    float DNAConc;
    float saltConc;
    float temperature = 0.0;
    AjBool doThermo;
    AjBool isProduct;
    AjBool isDNA  = ajTrue;
    AjBool isRNA  = ajFalse;
    AjBool doplot = ajFalse;
    AjPGraph mult = NULL;
    float mintemp = 0.0;
    float formamide;
    float mismatch;
    float prodLen;

    AjPSeq seq;
    AjPStr strand = NULL;
    ajint len;

    static float *xa = NULL;
    static float *ta = NULL;
    static float *tpa = NULL;
    static float *cga = NULL;
    static ajint npoints;
    ajint n;
    ajint ti;
    

    embInit("dan", argc, argv);

    seqall    = ajAcdGetSeqall("sequence");
    report    = ajAcdGetReport("outfile");
    window    = ajAcdGetInt("windowsize");
    shift     = ajAcdGetInt("shiftincrement");
    DNAConc   = ajAcdGetFloat("dnaconc");
    saltConc  = ajAcdGetFloat("saltconc");
    doThermo  = ajAcdGetToggle("thermo");
    isProduct = ajAcdGetToggle("product");
    isRNA     = ajAcdGetBoolean("rna");
    doplot    = ajAcdGetToggle("plot");

    formamide = ajAcdGetFloat("formamide");
    mismatch  = ajAcdGetFloat("mismatch");
    ti        = ajAcdGetInt("prodLen");

    prodLen = (float) ti;

    if(!isProduct)
    {
	formamide = mismatch = 0.0;
	prodLen   = (float) window;
    }

    temperature = ajAcdGetFloat("temperature");
    if (!doThermo)
	temperature = 0.0;

    mintemp = ajAcdGetFloat("mintemp");
    mult = ajAcdGetGraphxy ("graph");
    if(!doplot)
    {
	mintemp = 0.0;
	ajGraphxyDel(&mult);
    }



    if(isRNA)
	isDNA = ajFalse;
    else
	isDNA = ajTrue;


    while(ajSeqallNext(seqall, &seq))
    {
	npoints = 0;
	strand  = ajSeqGetSeqCopyS(seq);
	len     = ajStrGetLen(strand);
	begin   = ajSeqallGetseqBegin(seqall);
	end     = ajSeqallGetseqEnd(seqall);

	TabRpt = ajFeattableNewSeq(seq);

	if(!doplot)
	    if(outf)
		ajFmtPrintF(outf,"DAN of: %s   from: %d  to: %d\n\n",
			    ajSeqGetNameC(seq), begin, end);

	ajStrFmtUpper(&strand);


	n=ajRound(len,shift);

	AJCNEW(xa, n);
	AJCNEW(ta, n);
	AJCNEW(tpa, n);
	AJCNEW(cga, n);

	if(outf)
	    dan_findgc(strand, begin, end,
		       window,shift,formamide,mismatch,
		       (ajint)prodLen,DNAConc,saltConc,
		       temperature, isDNA, isProduct,
		       doThermo, outf, doplot, xa, ta, tpa, cga, &npoints);

	dan_reportgc(seq, TabRpt,
		     window,shift,formamide,mismatch,
		     (ajint)prodLen,DNAConc,saltConc, temperature,
		     isDNA, isProduct, doThermo, doplot,
		     xa, ta, tpa, cga, &npoints);

	if(doplot)
	    dan_plotit(seq,xa,ta,npoints,begin,end, mult,
		       mintemp);

	dan_unfmall(xa, ta, tpa, cga);
	ajStrDel(&strand);

	if(report)
	    ajReportWrite(report, TabRpt, seq);
	ajFeattableDel(&TabRpt);
    }

    /*if(mult)*/
	ajGraphicsClose();

    ajGraphxyDel(&mult);
    ajFileClose(&outf);

    ajSeqallDel(&seqall);
    ajFileClose(&outf);
    ajReportDel(&report);
    ajFeattableDel(&TabRpt);
    ajGraphxyDel(&mult);
    ajSeqDel(&seq);

    ajStrDel(&strand);

    embExit();
    return 0;
}




/* @funcstatic dan_findgc *****************************************************
**
** Undocumented
**
** @param [r] strand [const AjPStr] Undocumented
** @param [r] begin [ajint] Undocumented
** @param [r] end [ajint] Undocumented
** @param [r] window [ajint] Undocumented
** @param [r] shift [ajint] Undocumented
** @param [r] formamide [float] Undocumented
** @param [r] mismatch [float] Undocumented
** @param [r] prodLen [ajint] Undocumented
** @param [r] dna [float] Undocumented
** @param [r] salt [float] Undocumented
** @param [r] temperature [float] Undocumented
** @param [r] isDNA [AjBool] Undocumented
** @param [r] isproduct [AjBool] Undocumented
** @param [r] dothermo [AjBool] Undocumented
** @param [u] outf [AjPFile] Undocumented
** @param [r] doplot [AjBool] Undocumented
** @param [w] xa [float[]] Array elements updated
** @param [w] ta [float[]] Array elements updated
** @param [w] tpa [float[]] Array elements updated
** @param [w] cga [float[]] Array elements updated
** @param [w] np [ajint*] Undocumented
** @@
******************************************************************************/

static void dan_findgc(const AjPStr strand, ajint begin, ajint end, 
		       ajint window,
		       ajint shift, float formamide, float mismatch,
		       ajint prodLen, float dna, float salt,float temperature,
		       AjBool isDNA, AjBool isproduct,  AjBool dothermo,
		       AjPFile outf, AjBool doplot, float xa[],
		       float ta[], float tpa[], float cga[], ajint *np)
{
    static AjBool initialised = 0;
    AjPStr substr = NULL;
    float  fwindow;
    ajint    i;
    ajint ibegin;
    ajint iend;
    float fprodlen;
    float TmP1;
    float TmP2;
    ajint   e;

    float DeltaG = 0.0;
    float DeltaH;
    float DeltaS;

    --begin;
    --end;


    if(!initialised)
    {
	ajMeltInit(isDNA, window);
    }

    fwindow  = (float) window;
    fprodlen = (float) prodLen;
    substr   = ajStrNew();

    e = (end - window) + 2;
    for(i=begin; i < e; i+=shift)
    {
	ibegin = i;
	iend   = i + window -1;
	ajStrAssignSubC(&substr, ajStrGetPtr(strand), ibegin, iend);

	xa[*np]  = (float)(i+1);
	ta[*np]  = ajMeltTemp(substr, (iend-ibegin)+1, shift, salt, dna, isDNA);
	cga[*np] = (float)100.0 * ajMeltGC(substr, window);

	if(dothermo)
	{
	    DeltaG = (float)-1. * ajMeltEnergy(substr, (iend-ibegin)+1,
					       shift, isDNA, ajFalse, &DeltaH,
					       &DeltaS);

	    DeltaH = (float)-1. * DeltaH;
	    DeltaS = (float)-1. * DeltaS;
	    DeltaG = DeltaH - (float)0.001*DeltaS*((float)273.15+temperature);
	}

	if(!doplot)
	{
	    ajFmtPrintF(outf,"%4d %s",ibegin+1,ajStrGetPtr(substr));
	    if(iend-ibegin+1 > 40)
		ajFmtPrintF(outf,"...");
	    if(dothermo)
	        ajFmtPrintF(outf," %4d Tm=%2.1f GC%%=%2.1f dG %f dH %f dS %f",
			    iend+1,ta[*np], cga[*np], DeltaG, DeltaH, DeltaS);
	    else
	        ajFmtPrintF(outf," %4d Tm=%2.1f GC%%=%2.1f",iend+1,ta[*np],
			    cga[*np]);
	}


	if(isproduct)
	{
	    TmP1 = (float)81.5 + (float)16.6 *
		(float)log10((double)(salt/(float)1000.0)) + (float)0.41 *
		cga[*np];
	    TmP2 = -((float)0.65 * formamide) - ((float)675.0/fprodlen) -
		mismatch;
	    tpa[*np] = TmP1 + TmP2;
	    if(!doplot)
		ajFmtPrintF(outf," Tm(prod)=%1.1f",tpa[*np]);
	}
	if(!doplot)
	    ajFmtPrintF(outf,"\n");

	if(doplot)
	    xa[*np] += fwindow / (float)2.0;

	++(*np);
    }

    ajStrDel(&substr);

    return;
}




/* @funcstatic dan_reportgc ***************************************************
**
** Undocumented
**
** @param [r] seq [const AjPSeq] Undocumented
** @param [u] TabRpt [AjPFeattable] Undocumented
** @param [r] window [ajint] Undocumented
** @param [r] shift [ajint] Undocumented
** @param [r] formamide [float] Undocumented
** @param [r] mismatch [float] Undocumented
** @param [r] prodLen [ajint] Undocumented
** @param [r] dna [float] Undocumented
** @param [r] salt [float] Undocumented
** @param [r] temperature [float] Undocumented
** @param [r] isDNA [AjBool] Undocumented
** @param [r] isproduct [AjBool] Undocumented
** @param [r] dothermo [AjBool] Undocumented
** @param [r] doplot [AjBool] Undocumented
** @param [w] xa [float[]] Undocumented
** @param [w] ta [float[]] Undocumented
** @param [w] tpa [float[]] Undocumented
** @param [w] cga [float[]] Undocumented
** @param [w] np [ajint*] Undocumented
** @@
******************************************************************************/


static void dan_reportgc(const AjPSeq seq, AjPFeattable TabRpt,
			 ajint window,
			 ajint shift, float formamide, float mismatch,
			 ajint prodLen, float dna, float salt,
			 float temperature,
			 AjBool isDNA, AjBool isproduct,  AjBool dothermo,
			 AjBool doplot, float xa[], float ta[],
			 float tpa[], float cga[], ajint *np)
{

    AjPFeature gf = NULL;
    AjPStr tmpStr = NULL;
    static AjBool initialised = 0;
    AjPStr substr = NULL;
    float  fwindow;
    ajint    i;
    ajint ibegin;
    ajint iend;
    float fprodlen;
    float TmP1;
    float TmP2;
    ajint   e;

    float DeltaG = 0.0;
    float DeltaH;
    float DeltaS;

    ajint begin;
    ajint end;

    begin = ajSeqGetBegin(seq);
    end   = ajSeqGetEnd(seq);

    --begin;
    --end;

    if(!initialised)
    {
	ajMeltInit(isDNA, window);
    }

    fwindow  = (float) window;
    fprodlen = (float) prodLen;
    substr   = ajStrNew();

    e = (end - window) + 2;
    for(i=begin; i < e; i+=shift)
    {
	ibegin = i;
	iend = i + window -1;
	ajStrAssignSubS(&substr, ajSeqGetSeqS(seq), ibegin, iend);

	xa[*np]  = (float)(i+1);
	ta[*np]  = ajMeltTemp(substr, (iend-ibegin)+1, shift, salt, dna, isDNA);
	cga[*np] = (float) 100.0 * ajMeltGC(substr, window);

	if(dothermo)
	{
	    DeltaG = (float)-1. * ajMeltEnergy(substr, (iend-ibegin)+1, shift,
					       isDNA, ajFalse, &DeltaH,
					       &DeltaS);
	    DeltaH = (float)-1. * DeltaH;
	    DeltaS = (float)-1. * DeltaS;
	    DeltaG = DeltaH - (float)0.001*DeltaS*((float)273.15+temperature);
	}

	if(!doplot)
	{
	  gf = ajFeatNewII(TabRpt,
			   ibegin+1, iend+1);
	  ajFmtPrintS(&tmpStr, "*tm %4.1f", ta[*np]);
	  ajFeatTagAdd(gf,  NULL, tmpStr);
	  ajFmtPrintS(&tmpStr, "*gc %4.1f", cga[*np]);
	  ajFeatTagAdd(gf,  NULL, tmpStr);
	  if(dothermo)
	  {
	      ajFmtPrintS (&tmpStr, "*dg %.3f", DeltaG);
	      ajFeatTagAdd(gf,  NULL, tmpStr);
	      ajFmtPrintS (&tmpStr, "*dh %.3f", DeltaH);
	      ajFeatTagAdd(gf,  NULL, tmpStr);
	      ajFmtPrintS (&tmpStr, "*ds %.3f", DeltaS);
	      ajFeatTagAdd(gf,  NULL, tmpStr);
	  }
	}
	if(isproduct)
	{
	    TmP1 = (float)81.5 + (float)16.6 *
		(float)log10((double)(salt/(float)1000.0)) + (float)0.41 *
		cga[*np];
	    TmP2 = -((float)0.65 * formamide) - ((float)675.0/fprodlen) -
		mismatch;
	    tpa[*np] = TmP1 + TmP2;
	    if(!doplot)
	    {
		ajFmtPrintS (&tmpStr, "*tmprod %4.1f", tpa[*np]);
		ajFeatTagAdd(gf,  NULL, tmpStr);
	    }
	}

	if(doplot)
	    xa[*np] += fwindow / (float)2.0;

	++(*np);
    }

    ajStrDel(&substr);
    ajStrDel(&tmpStr);

    return;
}




/* @funcstatic dan_unfmall ****************************************************
**
** Undocumented.
**
** @param [d] xa [float*] Undocumented
** @param [d] ta [float*] Undocumented
** @param [d] tpa [float*] Undocumented
** @param [d] cga [float*] Undocumented
** @@
******************************************************************************/

static void dan_unfmall(float *xa, float *ta, float *tpa, float *cga)
{
    AJFREE( xa);
    AJFREE( ta);
    AJFREE( tpa);
    AJFREE( cga);

    return;
}




/* @funcstatic dan_plotit *****************************************************
**
** Undocumented.
**
** @param [r] seq [const AjPSeq] Undocumented
** @param [r] xa [const float*] Undocumented
** @param [r] ta [const float*] Undocumented
** @param [r] npoints [ajint] Undocumented
** @param [r] ibegin [ajint] Undocumented
** @param [r] iend [ajint] Undocumented
** @param [u] graphs [AjPGraph] Undocumented
** @param [r] mintemp [float] Undocumented
** @@
******************************************************************************/

static void dan_plotit(const AjPSeq seq, const float *xa, const float *ta,
		       ajint npoints, ajint ibegin, ajint iend,
		       AjPGraph graphs, float mintemp)
{
    AjPGraphdata tmGraph = NULL;
    float max = -64000.;
    float min = 64000.;

    ajint i;

    for(i=0;i<npoints;++i)
    {
	min = (min<ta[i]) ? min : ta[i];
	max = (max>ta[i]) ? max : ta[i];
    }

    tmGraph = ajGraphdataNewI(npoints);
    ajGraphShowTitle(graphs,ajTrue);
    ajGraphxyShowXlabel(graphs,ajTrue);
    ajGraphxyShowYlabel(graphs,ajTrue);

    ajGraphAppendTitleS(graphs, ajSeqGetUsaS(seq));
    ajGraphSetXlabelC(graphs,"Base number");
    ajGraphSetYlabelC(graphs,"Melt temp (C)");

    ajGraphxySetXstartF(graphs,(float)ibegin);
    ajGraphxySetXendF(graphs,(float)iend);
    ajGraphxySetYstartF(graphs,0.0);
    ajGraphxySetYendF(graphs,100.0);
    ajGraphxySetXrangeII(graphs,ibegin,iend);
    ajGraphxySetYrangeII(graphs,(ajint)mintemp,100);

    ajGraphdataSetTypeC(tmGraph,"2D Plot");
    ajGraphdataSetMinmax(tmGraph,(float)ibegin,(float)iend,min,max);
    ajGraphdataSetTruescale(tmGraph,(float)ibegin,(float)iend,min,max);


    ajGraphdataAddXY(tmGraph,xa,ta);
    ajGraphDataAdd(graphs,tmGraph);

    ajGraphxyDisplay(graphs,AJTRUE);

    return;
}

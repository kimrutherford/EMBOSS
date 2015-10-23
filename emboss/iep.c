/* @source iep application
**
** Calculate isoelectric point of a protein
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
#include <stdlib.h>
#include <limits.h>

#define GSTEP 0.1




/* @prog iep ******************************************************************
**
** Calculates the isoelectric point of a protein
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall all;
    AjPSeq a;
    AjPStr substr;
    AjPFile outf;
    AjBool termini;
    ajint sscount;
    ajint modlysine;
    AjBool doplot;
    AjBool dofile;
    AjPGraph graph = NULL;

    float step;
    ajint amino = 1;
    ajint carboxyl = 1;
    
    double H;
    double pH;
    double iep;

    ajuint *c   = NULL;
    ajuint *op  = NULL;
    double **pK = NULL;
    double *K   = NULL;
    double *pro = NULL;
    double sum;
    double charge;

    AjPGraphdata phGraph = NULL;
    AjPStr title = NULL;
    AjPStr tmp = NULL;

    AjPFile pkptr = NULL;
    float *xa = NULL;
    float *ya = NULL;
    float minchg = 0.0;
    float maxchg = 0.0;
    ajint npoints;
    ajint k;
    ajint be;
    ajint en;
    ajint i;
    ajuint resn = 0;
    ajuint resc = 0;

    embInit("iep", argc, argv);

    all       = ajAcdGetSeqall("sequence");
    doplot    = ajAcdGetToggle("plot");
    dofile    = ajAcdGetToggle("report");
    step      = ajAcdGetFloat("step");
    termini   = ajAcdGetBoolean("termini");
    amino     = ajAcdGetInt("amino");
    carboxyl  = ajAcdGetInt("carboxyl");
    sscount   = ajAcdGetInt("disulphides");
    modlysine = ajAcdGetInt("lysinemodified");
    pkptr     = ajAcdGetDatafile("pkdata");
    outf      = ajAcdGetOutfile("outfile");


    substr=ajStrNew();
    AJCNEW(K,   EMBIEPSIZE);
    AJCNEW(c,   EMBIEPSIZE);
    AJCNEW(op,  EMBIEPSIZE);
    AJCNEW(pro, EMBIEPSIZE);

    embIepPkNewFile(pkptr, &pK);
    
    embIepCalcK(K,pK);				/* Convert to dissoc consts */

     /* only used if variable 'plot' is true */

   graph = ajAcdGetGraphxy("graph");

    while(ajSeqallNext(all,&a))
    {
	be = ajSeqallGetseqBegin(all);
	en = ajSeqallGetseqEnd(all);
	ajStrAssignSubC(&substr,ajSeqGetSeqC(a),be-1,en-1);

	for(i=0;i<EMBIEPSIZE;++i)
	{
	    c[i]=op[i]=0;
	    pro[i]=0.;
	}

	embIepCompS(substr, amino, carboxyl, sscount, modlysine,
                    c, &resn, &resc);

        embIepCalcKend(K, pK, resn, resc);

	if(dofile && outf)
	{
	    ajFmtPrintF(outf,"IEP of %S from %d to %d\n",
			ajSeqGetNameS(a), be, en);
	    if(!embIepIepS(substr, amino, carboxyl, sscount, modlysine,
                           pK, &iep, termini))
		ajFmtPrintF(outf,"Isoelectric Point = None\n\n");
	    else
		ajFmtPrintF(outf,"Isoelectric Point = %-6.4lf\n\n", iep);

	    ajFmtPrintF(outf,"   pH     Bound    Charge\n");

	    for(pH=1.0;pH<=14.0;pH+=step)
	    {
		H = embIepPhToHconc(pH);
		if(!termini)
		    c[EMBIEPAMINO]=c[EMBIEPCARBOXYL]=0;
		embIepGetProto(K,c,op,H,pro);
		charge=embIepGetCharge(c,pro,&sum);
		ajFmtPrintF(outf,"%6.2lf   %6.2lf   %6.2lf\n",pH,sum,charge);
	    }
	}

	if(doplot)
	{
	    npoints = (ajint)((14.0-1.0)/(double)GSTEP) +1;
	    AJCNEW(xa,npoints);
	    AJCNEW(ya,npoints);

	    minchg = (float) INT_MAX;
	    maxchg = (float)-INT_MAX;

	    for(pH=1.0,k=0;pH<=14.0;pH+=GSTEP,++k)
	    {
		xa[k] = (float)pH;
		H = embIepPhToHconc(pH);
		if(!termini)
		    c[EMBIEPAMINO] = c[EMBIEPCARBOXYL]=0;
		embIepGetProto(K,c,op,H,pro);
		charge = embIepGetCharge(c,pro,&sum);
		ya[k]  = (float)charge;
		minchg = (minchg<charge) ? minchg : (float)charge;
		maxchg = (maxchg>charge) ? maxchg : (float)charge;
	    }
	    npoints = k;

	    title = ajStrNew();
	    tmp = ajStrNew();
	    ajFmtPrintS(&title,"%s %d-%d IEP=",ajSeqGetNameC(a),be,en);

	    if(!embIepIepS(substr, amino, carboxyl, sscount, modlysine,
			   pK, &iep,termini))
		ajStrAssignC(&tmp,"none");
	    else
		ajFmtPrintS(&tmp,"%-8.4f",iep);
	    ajStrAppendS(&title,tmp);


	    phGraph = ajGraphdataNewI(npoints);
	    ajGraphSetTitleS(graph,title);
	    ajGraphSetXlabelC(graph,"pH");
	    ajGraphSetYlabelC(graph,"Charge");

	    ajGraphdataSetTypeC(phGraph,"2D Plot Float");
	    ajGraphdataSetMinmax(phGraph,1.0,14.0,minchg,maxchg);
	    ajGraphdataSetTruescale(phGraph,1.0,14.0,minchg,maxchg);
	    ajGraphxySetXstartF(graph,1.0);
	    ajGraphxySetXendF(graph,14.0);
	    ajGraphxySetYstartF(graph,minchg);
	    ajGraphxySetYendF(graph,maxchg);
	    ajGraphxySetXrangeII(graph,1,14);
	    ajGraphxySetYrangeII(graph,(ajint)minchg,(ajint)maxchg);

	    ajGraphdataAddXY(phGraph,xa,ya);
	    ajGraphDataReplace(graph,phGraph);


	    ajGraphxyDisplay(graph,ajFalse);
	    ajStrDel(&tmp);
	    ajStrDel(&title);
	    AJFREE(ya);
	    AJFREE(xa);
	}
    }
    ajGraphicsClose();
    ajGraphxyDel(&graph);

    embIepPkDel(&pK);
    
    AJFREE(K);
    AJFREE(pro);
    AJFREE(op);
    AJFREE(c);

    ajStrDel(&substr);
    ajFileClose(&outf);
    ajFileClose(&pkptr);

    ajSeqallDel(&all);
    ajSeqDel(&a);

    embExit();

    return 0;
}

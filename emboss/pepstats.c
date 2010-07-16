/* @source pepstats application
**
** Calculate protein statistics
**
** @author Copyright (C) Alan Bleasby (ajb@ebi.ac.uk)
** @@
**
** Dayhoff statistic by Rodrigo Lopez (rls@ebi.ac.uk)
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
#include <ctype.h>
#include <math.h>

#define DAYHOFF_FILE "Edayhoff.freq"
#define LAM1 (double)15.43
#define LAM2 (double)-29.56
#define CVDISC (double)1.71




static AjBool pepstats_issize(const EmbPPropAmino aa, ajint crit);




/* @prog pepstats *************************************************************
**
** Protein statistics
**
******************************************************************************/

int main(int argc, char **argv)
{
    const char *prop[]=
    {
	"Tiny\t\t(A+C+G+S+T)\t\t","Small\t\t(A+B+C+D+G+N+P+S+T+V)\t",
	"Aliphatic\t(A+I+L+V)\t\t","Aromatic\t(F+H+W+Y)\t\t",
	"Non-polar\t(A+C+F+G+I+L+M+P+V+W+Y)\t",
	"Polar\t\t(D+E+H+K+N+Q+R+S+T+Z)\t","Charged\t\t(B+D+E+H+K+R+Z)\t\t",
	"Basic\t\t(H+K+R)\t\t\t","Acidic\t\t(B+D+E+Z)\t\t"
    };

    AjPSeqall a;
    AjPStr substr;
    ajint be;
    ajint en;
    AjPFile outf;
    AjBool termini;

    double molwt;
    double charge;
    double iep;
    double molpc;
    double *pK = NULL;
    
    ajint *c;
    ajint i;
    ajint j;
    ajint len;
    ajint sum;
    ajint ngps;
    ajint rk;
    ajint de;
    double cv;
    double psolu;

    double molar_ext_coeff;
    double molar_ext_coeff_ss;

    float *dhstat = NULL;
    AjPFile mfptr = NULL;
    AjPFile wfptr = NULL;
    AjPSeq  seq = NULL;

    EmbPPropMolwt *mwdata;
    EmbPPropAmino *aadata;
    AjBool mono;
    
    
    embInit("pepstats", argc, argv);

    a       = ajAcdGetSeqall("sequence");
    termini = ajAcdGetBoolean("termini");
    outf    = ajAcdGetOutfile("outfile");
    mfptr   = ajAcdGetDatafile("aadata");
    wfptr   = ajAcdGetDatafile("mwdata");
    mono    = ajAcdGetBoolean("mono");
    
    substr  = ajStrNew();

    pK = embIeppKNew();

    aadata = embPropEaminoRead(mfptr);
    mwdata = embPropEmolwtRead(wfptr);

    if(!embReadAminoDataFloatC(DAYHOFF_FILE,&dhstat,(float)0.001))
	ajFatal("Set the EMBOSS_DATA environment variable");

    AJCNEW (c, EMBIEPSIZE);

    while(ajSeqallNext(a,&seq))
    {
	be = ajSeqallGetseqBegin(a);
	en = ajSeqallGetseqEnd(a);
	ajStrAssignSubC(&substr,ajSeqGetSeqC(seq),be-1,en-1);
	len = en-be+1;

	embIepCompS(substr,1,1,0,0,c);
	if(!termini)
	    c[EMBIEPAMINO]=c[EMBIEPCARBOXYL]=0;

	ajFmtPrintF(outf,"PEPSTATS of %s from %d to %d\n\n",ajSeqGetNameC(seq),
		    be,en);

        molwt=embPropCalcMolwt(ajStrGetPtr(substr),0,len-1,mwdata,mono);
	ajFmtPrintF(outf,"Molecular weight = %-10.2f\t\tResidues = %-6d\n",
                    molwt, len);

	for(i=0,charge=0.0;i<26;++i)
	    charge += (double)c[i] * (double) embPropGetCharge(aadata[i]);

	ajFmtPrintF(outf,"Average Residue Weight  = %-7.3f \t"
		    "Charge   = %-6.1f\n", molwt/(double)len,charge);

	if(!embIepIepS(substr,1,1,0,0,pK,&iep,termini))
	    ajFmtPrintF(outf,"Isoelectric Point = None\n\n");
	else
	    ajFmtPrintF(outf,"Isoelectric Point = %-6.4lf\n",iep);

	molar_ext_coeff = embPropCalcMolextcoeff(ajStrGetPtr(substr),
                                                 0,len-1,
						 ajFalse, aadata);
	molar_ext_coeff_ss = embPropCalcMolextcoeff(ajStrGetPtr(substr),
                                                    0,len-1,
                                                    ajTrue, aadata);

	ajFmtPrintF(outf,
                    "A280 Molar Extinction Coefficients  = %d (reduced)"
                    "   %d (cystine bridges)\n", 
		    (ajint)molar_ext_coeff,
                    (ajint) molar_ext_coeff_ss);

	ajFmtPrintF(outf,
                    "A280 Extinction Coefficients 1mg/ml = %.3f (reduced)"
                    "   %.3f (cystine bridges)\n",
		    molar_ext_coeff / molwt,
		    molar_ext_coeff_ss / molwt);

	ngps = c['N'-'A'] + c['G'-'A'] + c['P'-'A'] + c['S'-'A'];
	rk   = c['R'-'A'] + c['K'-'A'];
	de   = c['D'-'A'] + c['E'-'A'];

	cv = (double)LAM1*((double)((double)ngps/(double)len)) +
	    (double)LAM2*fabs((double)(((double)(rk-de)/(double)len)-
				       (double)0.03));

	psolu = 0.4934 + 0.276*fabs((double)(cv-CVDISC)) -
	    0.0392*(cv-CVDISC)*(cv-CVDISC);

	if(cv-CVDISC >= 0.0)
	    ajFmtPrintF(outf,"Imp");
	else
	    ajFmtPrintF(outf,"P");
	ajFmtPrintF(outf,
		    "robability of expression in inclusion bodies = %.3lf\n\n",
		    psolu);

	ajFmtPrintF(outf,"Residue\t\tNumber\t\tMole%%\t\tDayhoffStat\n");
	for(i=0;i<26;++i)
	{
	    molpc=(100.0 * (double)c[i]/(double)len);
	    ajFmtPrintF(outf,"%c = %s\t\t%d\t\t%-7.3f\t\t%-7.3f\t\n",i+'A',
			embPropIntToThree(i),c[i],molpc,molpc/dhstat[i]);
	}


	ajFmtPrintF(outf,"\nProperty\tResidues\t\tNumber\t\tMole%%\n");
	for(i=1;i<10;++i)
	{
	    ajFmtPrintF(outf,"%s",prop[i-1]);
	    for(j=0,sum=0;j<26;++j)
		if(pepstats_issize(aadata[j],i))
		    sum += c[j];
	    ajFmtPrintF(outf,"%d\t\t%6.3f\n",sum,
			100.0 * (double)sum/(double)len);
	}
	ajFmtPrintF(outf,"\n");
    }
    

    embPropAminoDel(&aadata);
    embPropMolwtDel(&mwdata);
    embIeppKDel(pK);
    
    AJFREE(dhstat);
    AJFREE(c);

    ajStrDel(&substr);
    ajFileClose(&outf);
    ajFileClose(&mfptr);
    ajFileClose(&wfptr);

    ajSeqallDel(&a);
    ajSeqDel(&seq);

    embExit();

    return 0;
}




/* @funcstatic pepstats_issize **********************************************
**
** Returns true if an amino acd matches a given size criterion
**
** @param [r] aa [const EmbPPropAmino] amino acid data
** @param [r] crit [ajint] criterion 
** @return [AjBool] true if criterion is met
** @@
******************************************************************************/

static AjBool pepstats_issize(const EmbPPropAmino aa, ajint crit)
{
    if(crit==1)
	return aa->tiny;
    else if(crit==2)
	return aa->sm_all;
    else if(crit==3)
	return aa->aliphatic;
    else if(crit==4)
	return aa->aromatic;
    else if(crit==5)
	return aa->nonpolar;
    else if(crit==6)
	return aa->polar;
    else if(crit==7)
    {
	if(aa->charge)
	    return ajTrue;
    }
    else if(crit==8)
	return aa->pve;
    else if(crit==9)
	return aa->nve;
    
    return ajFalse;
}

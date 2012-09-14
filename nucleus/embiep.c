/* @source embiep *************************************************************
**
** Isoelectric point routines
**
** @author Copyright (c) 1999 Alan Bleasby
** @version $Revision: 1.26 $
** @modified $Date: 2011/11/08 15:12:52 $ by $Author: rice $
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public
** License along with this library; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA  02110-1301,  USA.
**
******************************************************************************/


#include "ajlib.h"

#include "embiep.h"
#include "ajsys.h"
#include "ajfiledata.h"
#include "ajfileio.h"
#include "ajbase.h"

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <ctype.h>



#define PKFILE "Epk.dat"




/* @func embIeppKNew  *********************************************************
**
** Create a pK array and read the data
**
** @return [double*] pK data
**
** @release 6.1.0
******************************************************************************/

double* embIeppKNew(void)
{
    double *pK = NULL;

    AJCNEW(pK,EMBIEPSIZE);

    embIepPkRead(pK);			/* read pK's */
    
    return pK;
}




/* @func embIeppKDel  *********************************************************
**
** Delete a pK array and read the data
**
** @param [w] pK [double *] pKs
**
** @return [void]
**
** @release 6.1.0
******************************************************************************/

void embIeppKDel(double *pK)
{

    AJFREE(pK);

    return;
}




/* @func embIepPhToHconc  *****************************************************
**
** Convert pH to hydrogen ion concontration
**
** @param [r] pH [double] pH
**
** @return [double] hydrogen ion concentrration
**
** @release 4.0.0
******************************************************************************/

double embIepPhToHconc(double pH)
{
    return pow(10.0,-pH);
}




/* @func embIepPhFromHconc ****************************************************
**
** Convert hydrogen ion concontration to pH
**
** @param [r] H [double] H
**
** @return [double] pH
**
** @release 4.0.0
******************************************************************************/

double embIepPhFromHconc(double H)
{
    return -log10(H);
}




/* @func embIepPkToK  *********************************************************
**
** Convert pK to dissociation constant
**
** @param [r] pK [double] pK
**
** @return [double] dissociation constant
**
** @release 1.0.0
******************************************************************************/

double embIepPkToK(double pK)
{
    return pow(10.0,-pK);
}




/* @func embIepPkFromK ********************************************************
**
** Convert dissociation constant to pK
**
** @param [r] K [double] K
**
** @return [double] pK
**
** @release 4.0.0
******************************************************************************/

double embIepPkFromK(double K)
{
    return -log10(K);
}




/* @func embIepPkRead  ********************************************************
**
** Read the pK values from Epk.dat
**
** @param [w] pK [double*] pK
**
** @return [void]
**
** @release 1.0.0
******************************************************************************/

void embIepPkRead(double *pK)
{
    AjPFile inf = NULL;
    AjPStr line;
    const char *p;
    double  amino    = 8.6;
    double  carboxyl = 3.6;
    char ch;
    ajint i;


    inf = ajDatafileNewInNameC(PKFILE);

    if(!inf)
	ajFatal("%s file not found",PKFILE);

    for(i=0;i<EMBIEPSIZE;++i)
	pK[i]=0.0;

    line = ajStrNew();

    while(ajReadline(inf,&line))
    {
	p = ajStrGetPtr(line);

	if(*p=='#' || *p=='!' || *p=='\n' || *p=='\r')
	    continue;

	if(ajStrPrefixCaseC(line,"Amino"))
	{
	    p = ajSysFuncStrtok(p," \t\n\r");
	    p = ajSysFuncStrtok(NULL," \t\n\r");
	    sscanf(p,"%lf",&amino);
	    continue;
	}

	if(ajStrPrefixCaseC(line,"Carboxyl"))
	{
	    p = ajSysFuncStrtok(p," \t\n\r");
	    p = ajSysFuncStrtok(NULL," \t\n\r");
	    sscanf(p,"%lf",&carboxyl);
	    continue;
	}

	p  = ajSysFuncStrtok(p," \t\n\r");
	ch = ajSysCastItoc(toupper((ajint)*p));
	p  = ajSysFuncStrtok(NULL," \t\n\r");
	sscanf(p,"%lf",&pK[ajBasecodeToInt(ch)]);
    }

    pK[EMBIEPAMINO]    = amino;
    pK[EMBIEPCARBOXYL] = carboxyl;

    ajStrDel(&line);
    ajFileClose(&inf);

    return;
}




/* @func embIepCompC **********************************************************
**
** Calculate the amino acid composition of a protein sequence
**
** @param [r] s [const char *] protein sequence
** @param [r] amino [ajint] number of amino termini
** @param [r] carboxyl [ajint] number of carboxyl termini
** @param [r] sscount [ajint] number of disulphide bridges
** @param [r] modlysine [ajint] number of modified lysines
** @param [w] c [ajint *] amino acid composition
**
** @return [void]
**
** @release 4.0.0
******************************************************************************/

void embIepCompC(const char *s, ajint amino, ajint carboxyl,
		 ajint sscount, ajint modlysine,
		 ajint *c)
{
    ajint i;
    ajint j;
    const char *p;
    
    for(i=0;i<EMBIEPSIZE;++i)
	c[i]=0;

    p=s;

    while(*p)
    {
	++c[ajBasecodeToInt(ajSysCastItoc(toupper((ajint)*p)))];
	++p;
    }

    if(c[1])				/* B = D or N use Dayhoff freq */
    {
	j = (int) (0.5 + ((float)c[1]) * 5.5 / 9.8);
	c[3] += j;
	c[13] += c[1] - j;
	ajDebug("embIepCompC B:%d => D:%d N:%d\n",
		c[1], j, c[1]-j);
	c[1] = 0;
    }

    if(c[25])				/* Z = E or Q use Dayhoff freq */
    {
	j = (int) (0.5 + ((float)c[25]) * 6.0 / 9.9);
	c[4] += j;
	c[16] += c[25] - j;
	ajDebug("embIepCompC Z:%d => E:%d Q:%d\n",
		c[25], j, c[25]-j);
	c[25] = 0;
    }

    c[EMBIEPAMINO]    = amino;
    c[EMBIEPCARBOXYL] = carboxyl;

    if (sscount > 0)
    {
	if(c[EMBIEPCYSTEINE] <  2*sscount)
	{
	    ajWarn("embIepCompC %d disulphides but only %d cysteines\n",
		   sscount, c[EMBIEPCYSTEINE]+2*sscount);
	    c[EMBIEPCYSTEINE] = 0;
	}
	else
	{
	    c[EMBIEPCYSTEINE] -= 2*sscount;
	}
    }

    if (modlysine > 0)
    {
	if(c[EMBIEPLYSINE] < modlysine)
	{
	    ajWarn("embIepCompC %d modified lysines but only %d lysines\n",
		   sscount, c[EMBIEPLYSINE]);
	    c[EMBIEPLYSINE] = 0;
	}
	else
	{
	    c[EMBIEPLYSINE] -= modlysine;
	}
    }

    return;
}




/* @func embIepCompS **********************************************************
**
** Calculate the amino acid composition of a protein sequence
**
** @param [r] str [const AjPStr] protein sequence
** @param [r] amino [ajint] number of amino termini
** @param [r] carboxyl [ajint] number of carboxyl termini
** @param [r] sscount [ajint] number of disulphide bridges
** @param [r] modlysine [ajint] number of modified lysines
** @param [w] c [ajint *] amino acid composition
**
** @return [void]
**
** @release 4.0.0
******************************************************************************/

void embIepCompS(const AjPStr str, ajint amino, ajint carboxyl,
		 ajint sscount, ajint modlysine,
		 ajint *c)
{
    embIepCompC(ajStrGetPtr(str), amino, carboxyl, sscount, modlysine, c);

    return;
}




/* @func embIepCalcK  *********************************************************
**
** Calculate the dissociation constants
** Amino acids for which there is no entry in Epk.dat have K set to 0.0
**
** @param [w] K [double *] dissociation constants
** @param [w] pK [double *] pK values
**
** @return [void]
**
** @release 1.0.0
******************************************************************************/

void embIepCalcK(double *K, double *pK)
{
    ajint i;
    
    for(i=0;i<EMBIEPSIZE;++i)
	if(!pK[i])
	    K[i] = 0.0;
	else
	    K[i] = embIepPkToK(pK[i]);

    return;
}




/* @func embIepGetProto  ******************************************************
**
** Calculate the number of H+ bound
** Amino acids for which there is no entry in Epk.dat have this set to 0.0
**
** @param [r] K [const double *] dissociation constants
** @param [r] c [const ajint *] sequence composition
** @param [w] op [ajint *] printout flags
** @param [r] H [double] hydrogen ion concentration
** @param [w] pro [double *] number of protons bound
**
** @return [void]
**
** @release 1.0.0
******************************************************************************/

void embIepGetProto(const double *K, const ajint *c,
		    ajint *op, double H, double *pro)
{
    ajint i;

    for(i=0;i<EMBIEPSIZE;++i)
	if(!K[i])
	{
	    pro[i]=0.0;
	    op[i]=0;
	}
	else
	{
	    if(!c[i])
		op[i]=0;
	    else
		op[i]=1;

	    pro[i]=(double)c[i] * (H/(H+K[i]));
	}

    return;
}




/* @func embIepGetCharge  *****************************************************
**
** Calculate the number of H+ bound
**
** @param [r] c [const ajint *] sequence composition
** @param [r] pro [const double *] number of protons
** @param [w] total [double *] total protons
**
** @return [double] charge
**
** @release 1.0.0
******************************************************************************/

double embIepGetCharge(const ajint *c, const double *pro, double *total)
{
    ajint i;
    double C;

    for(i=0,*total=0.0;i<EMBIEPSIZE;++i)
	*total += pro[i];

    C = (pro[10]+pro[17]+pro[7]+pro[EMBIEPAMINO]) - /* basic: KRH */
	((double)c[24]-pro[24] +	/* Y */
	 (double)c[2]-pro[2] +		/* C */
	 (double)c[3]-pro[3] +		/* D */
	 (double)c[4]-pro[4] +		/* E */
	 (double)c[EMBIEPCARBOXYL]-pro[EMBIEPCARBOXYL] );

    return C;
}




/* @func embIepPhConverge  ****************************************************
**
** Calculate the pH nearest the IEP or return 0.0 if one doesn't exist
**
** @param [r] c [const ajint *] sequence composition
** @param [r] K [const double *] sequence dissociation constants
** @param [w] op [ajint *] printout flags
** @param [w] pro [double *] number of protons
**
** @return [double] IEP or 0.0
**
** @release 1.0.0
******************************************************************************/

double embIepPhConverge(const ajint *c, const double *K,
			ajint *op, double *pro)
{
    double sum = 0.0;
    double charge;
    double top;
    double mid;
    double bot;
    double H;
    double tph = 1.0;
    double bph = 14.0;

    H = embIepPhToHconc(tph);
    embIepGetProto(K,c,op,H,pro);
    top = embIepGetCharge(c,pro,&sum);
    H = embIepPhToHconc(bph);
    embIepGetProto(K,c,op,H,pro);
    bot = embIepGetCharge(c,pro,&sum);

    if((top>0.0 && bot>0.0) || (top<0.0 && bot<0.0))
	return 0.0;

    while(bph-tph>0.0001)
    {
	mid = ((bph-tph) / 2.0) + tph;
	H = embIepPhToHconc(mid);
	embIepGetProto(K,c,op,H,pro);
	charge = embIepGetCharge(c,pro,&sum);

	if(charge>0.0)
	{
	    tph = mid;
	    continue;
	}

	if(charge<0.0)
	{
	    bph = mid;
	    continue;
	}
	else
	    return mid;
    }

    return tph;
}




/* @func embIepIepC ***********************************************************
**
** Calculate the pH nearest the IEP.
**
** @param [r] s [const char *] sequence
** @param [r] amino [ajint] number of N-termini
** @param [r] carboxyl [ajint] number of C-termini
** @param [r] sscount [ajint] number of disulphide bridges
** @param [r] modlysine [ajint] number of modified lysines
** @param [w] pK [double *] pK values
** @param [w] iep [double *] IEP
** @param [r] termini [AjBool] use termini
**
** @return [AjBool] True if IEP exists
**
** @release 4.0.0
******************************************************************************/

AjBool embIepIepC(const char *s, ajint amino, ajint carboxyl,
		  ajint sscount, ajint modlysine,
		  double *pK, double *iep, AjBool termini)
{
    ajint *c    = NULL;
    ajint *op   = NULL;
    double *K   = NULL;
    double *pro = NULL;
    
    *iep = 0.0;

    AJCNEW(c,   EMBIEPSIZE);
    AJCNEW(op,  EMBIEPSIZE);
    AJCNEW(K,   EMBIEPSIZE);
    AJCNEW(pro, EMBIEPSIZE);


    embIepCalcK(K,pK);			/* Convert to dissoc consts */

    /* Get sequence composition */
    embIepCompC(s,amino,carboxyl,sscount, modlysine,c);

    if(!termini)
	c[EMBIEPAMINO] = c[EMBIEPCARBOXYL] = 0;

    *iep = embIepPhConverge(c,K,op,pro);

    AJFREE(pro);
    AJFREE(K);
    AJFREE(op);
    AJFREE(c);

    if(!*iep)
	return ajFalse;

    return ajTrue;
}




/* @func embIepIepS ***********************************************************
**
** Calculate the pH nearest the IEP.
**
** @param [r] str [const AjPStr] sequence
** @param [r] amino [ajint] number of N-termini
** @param [r] carboxyl [ajint] number of C-termini
** @param [r] sscount [ajint] number of disulphide bridges
** @param [r] modlysine [ajint] number of modified lysines
** @param [w] pK [double *] pK values
** @param [w] iep [double *] IEP
** @param [r] termini [AjBool] use termini
**
** @return [AjBool] True if IEP exists
**
** @release 4.0.0
******************************************************************************/

AjBool embIepIepS(const AjPStr str, ajint amino, ajint carboxyl,
		  ajint sscount, ajint modlysine,
		  double *pK, double *iep, AjBool termini)
{
    return embIepIepC(ajStrGetPtr(str), amino, carboxyl, sscount, modlysine,
		      pK, iep, termini);
}




#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif




#ifdef AJ_COMPILE_DEPRECATED
/* @obsolete embIepComp
** @replace embIepCompC (1,2,3/1,2,0,0,3)
*/
__deprecated void  embIepComp(const char *s, ajint amino, ajint carboxyl,
                              ajint *c)
{
    embIepCompC(s, amino, carboxyl, 0, 0, c);

    return;
}




/* @obsolete embIepIEP
** @replace embIepIepC (1,2,3,4/1,2,0,0,3,4)
*/

__deprecated AjBool  embIepIEP(const char *s, ajint amino, ajint carboxyl,
                               double *pK, double *iep, AjBool termini)
{
    return embIepIepC(s, amino, carboxyl, 0, 0, pK, iep, termini);
}
#endif

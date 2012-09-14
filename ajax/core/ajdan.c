/* @source ajdan **************************************************************
**
** AJAX nucleic acid functions
**
** @author Copyright (C) 1999 Alan Bleasby
** @version $Revision: 1.26 $
** @modified Feb 25 ajb First version
** @modified $Date: 2011/11/08 15:07:45 $ by $Author: rice $
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

#include "ajdan.h"
#include "ajsys.h"
#include "ajfileio.h"
#include "ajfiledata.h"
#include "ajbase.h"

#include <math.h>
#include <string.h>
#include <ctype.h>

#define DNAMELTFILE "Edna.melt"
#define RNAMELTFILE "Erna.melt"
#define MAXMELTSAVE 10000

static AjOMelt meltTable[256];
static AjBool meltInitDone = AJFALSE;

static ajint aj_melt_savesize   = 0;
static AjBool aj_melt_saveinit  = 0;
static AjBool aj_melt_saveshift = 1;

static float meltProbScore(const AjPStr seq1, const AjPStr seq2, ajint len);




/* @func ajMeltInit ***********************************************************
**
** Initialises melt entropies, enthalpies and energies. Different data
** files are read for DNA or RNA heteroduplex. Also sets optional flag
** for array saving of the above.
**
** @param  [r] isdna [AjBool] true for DNA, false for RNA
** @param  [r]  savesize [ajint] Size of array to save, or zero if none
** @return [void] Number of energies to save
**
** @release 1.0.0
******************************************************************************/

void ajMeltInit(AjBool isdna, ajint savesize)
{
    AjPFile mfptr;
    AjPStr  mfname = NULL;
    AjPStr  pair   = NULL;
    AjPStr  pair1  = NULL;
    AjPStr  pair2  = NULL;
    AjPStr  acgt   = NULL;
    AjPStr  line   = NULL;

    ajint i;
    ajint j;
    ajint k;
    
    char *p;
    const char *q;
    float enthalpy;
    float entropy;
    float energy;
    ajuint iline = 0;

    AjBool got1;
    AjBool got2;


    aj_melt_savesize = savesize;
    aj_melt_saveinit = ajFalse;

    if(meltInitDone)
	return;

    mfname = ajStrNew();

    if(isdna)
	ajStrAssignEmptyC(&mfname,DNAMELTFILE);
    else
	ajStrAssignEmptyC(&mfname,RNAMELTFILE);

    mfptr = ajDatafileNewInNameS(mfname);

    if(!mfptr)
	ajFatal("Entropy/enthalpy/energy file '%S' not found\n",
		mfname);

    pair1 = ajStrNew();
    pair2 = ajStrNew();
    line  = ajStrNew();

    ajStrAssignC(&pair,"AA");
    ajStrAssignC(&acgt,"ACGT");

    p = ajStrGetuniquePtr(&pair);
    q = ajStrGetPtr(acgt);

    for(i=0,k=0;i<4;++i)
    {
	*p = *(q+i);

	for(j=0;j<4;++j)
	{
	    *(p+1) = *(q+j);
	    meltTable[k++].pair = ajStrNewC(p);
	}
    }

    iline = 0;
    while(ajReadline(mfptr, &line))
    {
        ajStrRemoveWhiteExcess(&line);
        iline++;
	p = ajStrGetuniquePtr(&line);

	if(*p=='#' || *p=='!' || !*p)
	    continue;

	p = ajSysFuncStrtok(p," ");

        if(!p)
            ajDie("Bad melt data file '%F' line %d '%S'",
                  mfptr, iline, line);
	ajStrAssignC(&pair1,p);
	p = ajSysFuncStrtok(NULL," ");

        if(!p)
            ajDie("Bad melt data file '%F' line %d '%S'",
                  mfptr, iline, line);
	ajStrAssignC(&pair2,p);
	p = ajSysFuncStrtok(NULL," ");

        if(!p)
            ajDie("Bad melt data file '%F' line %d '%S'",
                  mfptr, iline, line);

	if(sscanf(p,"%f",&enthalpy)!=1)
	    ajDie("Bad melt data file '%F' line %d '%S'",
                  mfptr, iline, line);

	p = ajSysFuncStrtok(NULL," ");

	if(sscanf(p,"%f",&entropy)!=1)
	    ajDie("Bad melt data file '%F' line %d '%S'",
                  mfptr, iline, line);
	p = ajSysFuncStrtok(NULL," ");

	if(sscanf(p,"%f",&energy)!=1)
	    ajDie("Bad melt data file '%F' line %d '%S'",
                  mfptr, iline, line);

	got1 = got2 = ajFalse;

	for(k=0;k<16;++k)
	    if(!ajStrCmpS(meltTable[k].pair,pair1))
	    {
		meltTable[k].enthalpy = enthalpy;
		meltTable[k].entropy  = entropy;
		meltTable[k].energy   = energy;
		got1 = ajTrue;
	    }

	for(k=0;k<16;++k)
	    if(!ajStrCmpS(meltTable[k].pair,pair2))
	    {
		meltTable[k].enthalpy = enthalpy;
		meltTable[k].entropy  = entropy;
		meltTable[k].energy   = energy;
		got2 = ajTrue;
	    }

	if(!got1 || !got2)
	    ajDie("Bad melt data file '%F' line %d '%S' duplicate pair",
                  mfptr, iline, line);
    }

    ajStrDel(&mfname);
    ajStrDel(&pair);
    ajStrDel(&pair1);
    ajStrDel(&pair2);
    ajStrDel(&acgt);
    ajStrDel(&line);

    ajFileClose(&mfptr);

    meltInitDone = ajTrue;

    return;
}




/* @funcstatic meltProbScore **************************************************
**
** Gives a score for the probability of two sequences being the same.
** The sequences are the same length.
**
** Uses IUB ambiguity codes. The result is the sum of the probabilities
** at each position.
**
** @param  [r] seq1 [const AjPStr] Pointer to a sequence string
** @param  [r] seq2 [const AjPStr] Pointer to a another sequence
** @param  [r]  len [ajint] Length of sequences
** @return [float] Match probability
**
** @release 6.2.0
******************************************************************************/

static float meltProbScore(const AjPStr seq1, const AjPStr seq2, ajint len)
{
    ajint mlen;
    float score;
    ajint i;
    ajint x;
    ajint y;
    const char *p;
    const char *q;


    mlen = (ajStrGetLen(seq1) < ajStrGetLen(seq2)) ? ajStrGetLen(seq1) :
	ajStrGetLen(seq2);

    if(len > 0)
	mlen = (mlen < len) ? mlen : len;

    score = 0.0;

    if(!mlen)
	return score;

    score = 1.0;
    p = ajStrGetPtr(seq1);
    q = ajStrGetPtr(seq2);

    for(i=0; i<mlen; ++i)
    {
	x = ajBasecodeToInt(*(p+i));
	y = ajBasecodeToInt(*(q+i));
	score *= ajBaseAlphaCompare(x,y);
    }

    return score;
}




/* @func ajMeltEnergy *********************************************************
**
** Calculates melt energy and enthalpy/entropy for a sequence string.
** An optional shift is given for stepping along the sequence and loading
** up energy arrays.
**
** @param  [r] strand [const AjPStr] Pointer to a sequence string
** @param  [r] len [ajint] Length of sequence
** @param  [r] shift [ajint] Stepping value
** @param  [r] isDNA [AjBool] DNA or RNA
** @param  [r] maySave [AjBool] May use the save arrays for speedup
** @param  [w] enthalpy [float*] enthalpy
** @param  [w] entropy [float*] entropy
**
** @return [float] Melt energy
**
** @release 1.0.0
******************************************************************************/

float ajMeltEnergy(const AjPStr strand, ajint len, ajint shift, AjBool isDNA,
		   AjBool maySave, float *enthalpy, float *entropy)
{
    AjPStr line;
    ajint i;
    ajint j;
    ajint k;
    ajint ipos;
    const char *p;
    static float energy;
    float ident;
    AjBool doShift;

    static float saveEnthalpy[MAXMELTSAVE];
    static float saveEntropy[MAXMELTSAVE];
    static float saveEnergy[MAXMELTSAVE];


    ajMeltInit(isDNA,len);

    if (maySave == ajFalse)
	aj_melt_saveshift = ajFalse;

    doShift = (aj_melt_saveshift && aj_melt_savesize > 0) ? ajTrue : ajFalse;


    ipos = 0;

    if(doShift)
    {
	if(!aj_melt_saveinit)
	{
	    ipos = 0;

	    for(i=0;i<aj_melt_savesize;++i)
		saveEnergy[i] = saveEntropy[i] = saveEnthalpy[i] = 0.0;
	    energy = *entropy = *enthalpy = 0.0;
	    aj_melt_saveinit = ajTrue;
	}
	else
	{
	    ipos = (len - shift) - 1;

	    for(i=0;i<shift;++i)
	    {
		energy    -= saveEnergy[i];
		*entropy  -= saveEntropy[i];
		*enthalpy -= saveEnthalpy[i];
	    }

	    for(i=0,k=shift; k < aj_melt_savesize; ++i, ++k)
	    {
		saveEnergy[i]   = saveEnergy[k];
		saveEntropy[i]  = saveEntropy[k];
		saveEnthalpy[i] = saveEnthalpy[k];
	    }
	}
    }
    else
    {
        ipos=0;
	energy = *entropy = *enthalpy = 0.0;
    }

    line = ajStrNew();
    p = ajStrGetPtr(strand);

    while(ipos < len-1)
    {
	if(doShift)
	{
	    saveEnthalpy[ipos] = 0.0;
	    saveEntropy[ipos]  = 0.0;
	    saveEnergy[ipos]   = 0.0;
	}

	for(j=0;j<16;++j)
	{
	    ajStrAssignSubC(&line,p+ipos,0,1);
	    ident = meltProbScore(meltTable[j].pair, line, 2);

	    if(ident>0.9)
	    {
		if(doShift)
		{
		    saveEnergy[ipos]   += (ident * meltTable[j].energy);
		    saveEntropy[ipos]  += (ident * meltTable[j].entropy);
		    saveEnthalpy[ipos] += (ident * meltTable[j].enthalpy);
		}
		else
		{
		    energy    += (ident * meltTable[j].energy);
		    *entropy  += (ident * meltTable[j].entropy);
		    *enthalpy += (ident * meltTable[j].enthalpy);
		}
	    }
	}


	if(doShift)
	{
	    energy    += saveEnergy[ipos];
	    *entropy  += saveEntropy[ipos];
	    *enthalpy += saveEnthalpy[ipos];
	}
	++ipos;
    }

    ajStrDel(&line);

    return energy;
}




/* @func ajMeltTemp ***********************************************************
**
** Calculates melt temperature of DNA or RNA
** An optional shift is given for stepping along the sequence and loading
** up energy arrays.
**
** @param  [r] strand [const AjPStr] Pointer to a sequence string
** @param  [r] len [ajint] Length of sequence
** @param  [r] shift [ajint] Stepping value
** @param  [r] saltconc [float] mM salt concentration
** @param  [r] DNAconc [float] nM DNA concentration
** @param  [r] isDNA [AjBool] DNA or RNA
**
** @return [float] Melt temperature
**
** @release 6.2.0
******************************************************************************/

float ajMeltTemp(const AjPStr strand, ajint len, ajint shift, float saltconc,
                 float DNAconc, AjBool isDNA)
{
    double entropy;
    double enthalpy;
    double dTm;
    float Tm;
    static float sumEntropy;
    static float sumEnthalpy;
    float To;
    float R;
    double LogDNA;


    R      = (float) 1.987;	       /* molar gas constant (cal/c * mol) */
    LogDNA = R * (float)log((double)(DNAconc/4000000000.0));	     /* nM */
    To = (float) 273.15;

    ajMeltEnergy(strand, len, shift, isDNA, ajFalse, &sumEnthalpy,
			&sumEntropy);

    entropy = -10.8 - sumEntropy;
    entropy += (len-1) * (log10((double) (saltconc/1000.0))) *
	(float) 0.368;

    enthalpy = -sumEnthalpy;

    dTm = ((enthalpy*1000.0) / (entropy+LogDNA)) - To;
    Tm  = (float) dTm;

    return Tm;
}




/* @func ajMeltGC *************************************************************
**
** Calculates GC fraction of a sequence allowing for ambiguity
**
** @param  [r] strand [const AjPStr] Pointer to a sequence string
** @param  [r] len [ajint] Length of sequence
**
** @return [float] GC fraction
**
** @release 1.0.0
******************************************************************************/

float ajMeltGC(const AjPStr strand, ajint len)
{
    ajint t;
    ajint i;
    const char *p;
    double count;

    p=ajStrGetPtr(strand);
    count = 0.0;

    for(i=0;i<len;++i)
    {
	t = toupper((ajint) *(p+i));

	if(strchr("GCS",t))
            ++count;
	else if(strchr("ATUW",t))
            count += 0.0;
	else if(strchr("RYMK",t))
            count += 0.5;
	else if(strchr("NX",t))
            count += 0.5;
	else if(strchr("BV",t))
            count += 0.6666667;
	else if(strchr("DH",t))
            count += 0.3333333;
    }

    return ((float)(count/(double)len));
}




/* @func ajMeltEnergy2 ********************************************************
**
** Calculates melt energy for use with programs like prima
**
** Giving this routine the complete sequence on the first call and
** setting meltInitDone to false will initialise the energy, entropy
** and enthalpy arrays. Subsequent calls will not look at the
** sequence directly.
**
** @param  [r] strand [const char *] Pointer to a sequence string
** @param  [r] pos [ajint] Position within sequence
** @param  [r] len [ajint] Length of sequence segment
** @param  [r] isDNA [AjBool] true if dna
** @param  [w] enthalpy [float *] calculated enthalpy
** @param  [w] entropy [float *] calculated entropy
** @param  [w] saveentr [float **] entropy save array
** @param  [w] saveenth [float **] enthalpy save array
** @param  [w] saveener [float **] energy save array
**
** @return [float] melt energy
**
** @release 1.0.0
******************************************************************************/

float ajMeltEnergy2(const char *strand, ajint pos, ajint len, AjBool isDNA,
		    float *enthalpy, float *entropy,
		    float **saveentr, float **saveenth, float **saveener)
{
    ajint i;
    ajint j;

    ajint limit = 0;
    AjPStr line = NULL;
    float ident = 0.0;
    float energy;

    limit = len-1;

    ajMeltInit(isDNA,len);

    line = ajStrNewC("AA");

    for(i=0;i<limit;++i)
    {
	ajStrAssignSubC(&line,strand,i,i+1);

	for(j=0;j<16;++j)
	{
	    ident = meltProbScore(meltTable[j].pair,line,2);

	    if(ident>.9)
	    {
		(*saveentr)[i] += (ident * meltTable[j].entropy);
		(*saveenth)[i] += (ident * meltTable[j].enthalpy);
		(*saveener)[i] += (ident * meltTable[j].energy);
	    }
	}
    }
    
    ajStrDel(&line);

    energy = *enthalpy = *entropy = 0.0;

    for(i=0;i<limit;++i)
    {
	energy    += (*saveener)[pos+i];
	*entropy  += (*saveentr)[pos+i];
	*enthalpy += (*saveenth)[pos+i];
    }

    return energy;
}




/* @func ajMeltTempSave *******************************************************
**
** Calculates melt temperature of DNA or RNA
**
** @param  [r] strand [const char*] Pointer to a sequence string
** @param  [r] pos [ajint] position within sequence
** @param  [r] len [ajint] Length of sequence (segment)
** @param  [r] saltconc [float] mM salt concentration
** @param  [r] DNAconc [float] nM DNA concentration
** @param  [r] isDNA [AjBool] DNA or RNA
** @param  [w] saveentr [float **] entropy save array
** @param  [w] saveenth [float **] enthalpy save array
** @param  [w] saveener [float **] energy save array
**
** @return [float] Melt temperature
**
** @release 6.2.0
******************************************************************************/

float ajMeltTempSave(const char *strand, ajint pos, ajint len, float saltconc,
                     float DNAconc, AjBool isDNA,
                     float **saveentr, float **saveenth, float **saveener)
{
    double entropy;
    double enthalpy;
    double dTm;
    float  Tm;
    float sumEntropy;
    float sumEnthalpy;
    float To;
    float R;
    double LogDNA;
    /* double LogSalt;*/


 /* LogSalt = 16.6 * (float) (log10((double) (saltconc/1000.0))); */ /* mM */
    R = (float) 1.987;		 /* molar gas constant (cal/c * mol)        */
    LogDNA = R * (float)log((double)(DNAconc/4000000000.0)); /* nM */
    To = (float) 273.15;

    ajMeltEnergy2(strand, pos, len, isDNA, &sumEnthalpy,
		  &sumEntropy, saveentr, saveenth, saveener);

    entropy = -10.8 - sumEntropy;

    /* Added for santa lucia */
    entropy += (len-1) * (log10((double) (saltconc/1000.0))) *
	(float) 0.368;


    enthalpy = -sumEnthalpy;

    /* logsalt removed for santa lucia */
    dTm = ((enthalpy*1000.0) / (entropy+LogDNA)) + /*LogSalt*/ - To;
    Tm = (float) dTm; /* slight loss of precision here but no matter */

    return Tm;
}




/* @func ajMeltTempProd *******************************************************
**
** Calculates product melt temperature of DNA
**
** @param  [r] gc [float] GC percentage
** @param  [r] saltconc [float] mM salt concentration
** @param  [r] len [ajint] Length of sequence (segment)
**
** @return [float] Melt temperature
**
** @release 6.2.0
******************************************************************************/

float ajMeltTempProd(float gc, float saltconc, ajint len)
{
    float ptm;
    float LogSalt;


    LogSalt = (float)16.6 * (float) (log10((double) (saltconc/1000.0)));

    ptm = (float)81.5 - (float)(675/len) + LogSalt + ((float)0.41 * gc);

    return ptm;
}




/* @func ajAnneal *************************************************************
**
** Calculates annealing temperature of product and primer
**
** @param  [r] tmprimer [float] primer Tm
** @param  [r] tmproduct [float] product Tm
**
** @return [float] Annealing temperature
**
** @release 1.0.0
******************************************************************************/

float ajAnneal(float tmprimer, float tmproduct)
{
    return ((float).7*tmproduct)-(float)14.9+((float).3*tmprimer);
}




/* @func ajMeltExit ***********************************************************
**
** Cleans up DNA melting processing internal memory
**
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajMeltExit(void)
{
    ajint i;

    for(i=0;i<256;i++)
	ajStrDel(&meltTable[i].pair);

    return;
}




#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif




#ifdef AJ_COMPILE_DEPRECATED
/* @obsolete ajTm
** @rename ajMeltTemp
*/

__deprecated float ajTm(const AjPStr strand, ajint len,
                        ajint shift, float saltconc,
                        float DNAconc, AjBool isDNA)
{

    return ajMeltTemp(strand, len, shift, saltconc, DNAconc, isDNA);
}




/* @obsolete ajTm2
** @rename ajMeltTempSave
*/

__deprecated float ajTm2(const char *strand, ajint pos,
                         ajint len, float saltconc,
	    float DNAconc, AjBool isDNA,
	    float **saveentr, float **saveenth, float **saveener)
{

    return ajMeltTempSave(strand, pos, len, saltconc, DNAconc, isDNA,
                          saveentr, saveenth, saveener);
}




/* @obsolete ajProdTm
** @rename ajMeltTempProd
*/

__deprecated float ajProdTm(float gc, float saltconc, ajint len)
{
    return ajMeltTempProd(gc, saltconc, len);
}
#endif

/* @source embcom.c
**
** General routines for program Complex
**
** NB: THESE ROUTINES DO NOT CONFORM TO THE LIBRARY WRITING STANDARD AND
**     THEREFORE SHOULD NOT BE USED AS A TEMPLATE FOR WRITING EMBOSS CODE
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
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#ifndef WIN32
#define fmodf(a,b) (float)fmod((double)a,(double)b)
#endif

static void comWriteTable(AjPFile fp, const char *name,
		          const UJWin *RUj,const UJWin *MedUj,
			  const UJWin *SDUj, const UJWin *RatUj,
		          ajint Nwin, ajint jmin, ajint jmax);
static void comCalcComplex(const UJWin *RUj,const UJWin *MedUj,
			   UJWin *RatUj,
			   ajint Nwin, ajint Nword, float *CompSeq);
static void comCalcMedValue(const UJSim *SetUj,
			    UJWin *MedUj,UJWin *SDUj,
			    ajint Nsim, ajint Nwin, ajint Nword);
static void comElabSetSim(const SEQSim* SetSeqSim,UJSim *SetUjSim,
		          ajint lseq, ajint nsim,
		          ajint jmin, ajint jmax, ajint lwin, ajint step);
static void comElabSeq(const char *seq,UJWin *ujwin, ajint jmin, ajint jmax,
		       ajint lwin, ajint lseq,
		       ajint step);
static ajint comCalcNOfWin(ajint lseq, ajint lwin, ajint step);
static void  comWriteSimValue(const float *ComplexOfSim,
			      ajint nsim,AjPFile fp);
static void  comWriteValue(const char *name, ajint lseq,
			   const float *ComplexOfSeq,
			   ajint NumOfWin, ajint l, ajint step,
			   ajint jmin, ajint jmax,
			   ajint sim,float MedValue, AjPFile fp);
static void  comComplexOnSeq(const char *seqsim,const char *seq,
			     ajint lseq, ajint lwin,
			     ajint jmin, ajint jmax,
			     ajuint step,float *ComplexOfSeq);
static void  comComplexOnSeq2(const char *seq, ajint lseq, ajint lwin,
			      ajint jmin, ajint jmax, ajint step,
			      float *ComplexOfSeq);
static void comSimulSeq(char *seqsim,
			ajint lseq,const comtrace *Freq,
			const char *ACN, ajint freq);
static void comSortFreq(comtrace *set);
static void comCalcComplexMed(const float *ComplexOfSeq, ajint NumOfWin,
			      float *MedValue);
static void comReadWin(const char *seq, ajint bwin, ajint ewin,char *win);
static void comRead_j_mer(const char *win, ajint lwin, ajint jlen,AjPStr *str);
static void comWinComplex2(const char *win, ajint lwin,float *ComplexOfWin,
			   ajint jmin, ajint jmax);
static void comWinComplex(const char *win,const char *winsim, ajint lwin,
			  float *ComplexOfWin, ajint jmin, ajint jmax);
static void comCalcUj2(ajint lwin, ajint jlen,const char *win,float *Ujvalue);
static void comCalcUj(ajint lwin, ajint jlen,const char *win,float *Ujvalue);
static ajint comCounter(AjPStr const * str, ajint k);
static void comAmbiguity(char *seq);
static void comReplace(const char *vet,char *ch);
static void comCalcFreqACN(const char *seq, ajint lseq,float *Freq);




/* @func embComComplexity *****************************************************
**
** Complexity calculation
**
** @param [r] seq [const char *] Sequence
** @param [r] name [const char *] Sequence name
** @param [r] len [ajint] Sequence length
** @param [r] jmin [ajint] Minimum
** @param [r] jmax [ajint] Maximum
** @param [r] l [ajint] Window length
** @param [r] step [ajint] Step size
** @param [r] sim [ajint] Simulation count
** @param [r] freq [ajint] Frequency calculation (boolean)
** @param [r] omnia [ajint] All sequences (boolean)
** @param [u] fp [AjPFile] Output file
** @param [u] pf [AjPFile] Temp file
** @param [r] print [ajint] Print (boolean)
** @param [w] MedValue [float *] Results
** @return [void]
** @@
******************************************************************************/

void embComComplexity(const char *seq,const char *name,
		      ajint len, ajint jmin, ajint jmax,
		      ajint l, ajint step,
		      ajint sim, ajint freq, ajint omnia,
		      AjPFile fp,  AjPFile pf, ajint print,
		      float *MedValue)
{

    float  *ComplexOfSeq = NULL;
    float  Freq[4];

    ajint NumOfWin;
    ajint i;
    ajint j;
    char ACN[]   = "ACGT";
    char *seqsim = NULL;

    comtrace SortedFreq[4];
    SEQSim *SetSeqSim = NULL;
    UJSim *SetUjSim   = NULL;
    UJWin *MedValueUj = NULL;
    UJWin *SDValueUj  = NULL;
    UJWin *RealUjValue;
    UJWin *RatioUj    = NULL;

    if(l == len)
	NumOfWin = 1;
    else
	NumOfWin = comCalcNOfWin(len,l,step);

    AJCNEW(ComplexOfSeq, NumOfWin);
    if(freq)
    {
	for(i=0;i<4;i++)
	    Freq[i] = 0.0;

	comCalcFreqACN(seq,len,Freq);

	for(i=0;i<4;i++)
	{
	    SortedFreq[i].ind = i;
	    SortedFreq[i].pc = Freq[i];
	}

	comSortFreq(SortedFreq);
	for(i=0;i<4;i++)
	    SortedFreq[i].pc = SortedFreq[i].pc*10;
    }

    AJCNEW(RealUjValue, NumOfWin);

    for(i=0;i<NumOfWin;i++)
	AJCNEW(RealUjValue[i].Ujwin, jmax - jmin +1);



    if(sim)
    {
        AJCNEW(SetUjSim, sim);

	for(i=0;i<sim;i++)
	    AJCNEW(SetUjSim[i].Ujsim, NumOfWin);

	for(i=0;i<sim;i++)
	    for(j=0;j<NumOfWin;j++)
                AJCNEW(SetUjSim[i].Ujsim[j].Ujwin, jmax-jmin+1);

	AJCNEW(SetSeqSim, sim);

	for(i=0;i<sim;i++)
	    AJCNEW(SetSeqSim[i].Sqsim, len+1);

	AJCNEW(seqsim, (len+1));

	AJCNEW(MedValueUj, NumOfWin);
	for(i=0;i<NumOfWin;i++)
	    AJCNEW(MedValueUj[i].Ujwin, jmax-jmin+1);

	AJCNEW(SDValueUj, NumOfWin);
	for(i=0;i<NumOfWin;i++)
	    AJCNEW(SDValueUj[i].Ujwin, jmax-jmin+1);

	AJCNEW(RatioUj, NumOfWin);
	for(i=0;i<NumOfWin;i++)
	    AJCNEW(RatioUj[i].Ujwin, jmax-jmin+1);
    }



    if(sim)
    {
	for(i=0;i<sim;i++)
	{
	    comSimulSeq(seqsim,len,SortedFreq,ACN,freq);
	    strcpy(SetSeqSim[i].Sqsim,seqsim);
	}
	comElabSetSim(SetSeqSim,SetUjSim,len,sim,jmin,jmax,l,step);
	comCalcMedValue(SetUjSim,MedValueUj,SDValueUj,sim,NumOfWin,
			jmax-jmin+1);

	comElabSeq(seq,RealUjValue,jmin,jmax,l,len,step);
	comCalcComplex(RealUjValue,MedValueUj,RatioUj,NumOfWin,
		       jmax-jmin+1,ComplexOfSeq);
	comCalcFreqACN(seqsim,len,Freq);
    }
    else
	comComplexOnSeq2(seq,len,l,jmin,jmax,step,ComplexOfSeq);

    comCalcComplexMed(ComplexOfSeq,NumOfWin,MedValue);

    if(sim && print)
	comWriteTable(pf,name,RealUjValue,MedValueUj,SDValueUj,RatioUj,
		      NumOfWin,jmin, jmax);
    if(!omnia)
	comWriteValue(name,len,ComplexOfSeq,NumOfWin,l,
		      step,jmin,jmax,sim,*MedValue,fp);
    AJFREE(ComplexOfSeq);
    if(sim)
    {
	AJFREE(seqsim);
	for(i=0;i<NumOfWin;i++)
	{
	    AJFREE(RatioUj[i].Ujwin);
	    AJFREE(SDValueUj[i].Ujwin);
	    AJFREE(MedValueUj[i].Ujwin);
	}

	AJFREE(RatioUj);
	AJFREE(SDValueUj);
	AJFREE(MedValueUj);
	for(i=0;i<sim;i++)
	{
	    for(j=0;j<NumOfWin;j++)
		AJFREE(SetUjSim[i].Ujsim[j].Ujwin);
	    AJFREE(SetUjSim[i].Ujsim);	/* was missing before pmr 25-jan-00 */
	}

	AJFREE(SetSeqSim);
	AJFREE(SetUjSim);
    }


    for(i=0;i<NumOfWin;i++)
	AJFREE(RealUjValue[i].Ujwin);

    AJFREE(RealUjValue);

    return;
}




/* @func embComWriteFile ******************************************************
**
** Write output file for complexity calculation
**
** @param [u] fp [AjPFile] Output file
** @param [r] jmin [ajint] Minimum
** @param [r] jmax [ajint] Maximum
** @param [r] lwin [ajint] Window
** @param [r] step [ajint] Step size
** @param [r] sim [ajint] Simulation count
** @return [void]
** @@
******************************************************************************/

void embComWriteFile(AjPFile fp, ajint jmin, ajint jmax,
		     ajint lwin, ajint step, ajint sim)
{

    ajFmtPrintF(fp,"Length of window : %d \n",lwin);
    ajFmtPrintF(fp,"jmin : %d \n",jmin);
    ajFmtPrintF(fp,"jmax : %d \n",jmax);
    ajFmtPrintF(fp,"step : %d \n",step);

    if(sim)
	ajFmtPrintF(fp,"sim : %d \n",sim);

    if(sim == 0)
	ajFmtPrintF(fp,"Execution without simulation \n");

    ajFmtPrintF(fp,"---------------------------------------"
		"-------------------------------------\n");
    ajFmtPrintF(fp,"|                  |                  |"
		"                  |                  |\n");
    ajFmtPrintF(fp,"|     number of    |      name of     |"
		"     length of    |      value of    |\n");
    ajFmtPrintF(fp,"|     sequence     |     sequence     |"
		"     sequence     |     complexity   |\n");
    ajFmtPrintF(fp,"|                  |                  |"
		"                  |                  |\n");
    ajFmtPrintF(fp,"---------------------------------------"
		"-------------------------------------\n");

    return;
}




/* @func embComWriteValueOfSeq ************************************************
**
** Output sequence values for complexity calculation
**
** @param [u] fp [AjPFile] Output file
** @param [r] n [ajint] Sequence number
** @param [r] name [const char *] Sequence name
** @param [r] len [ajint] Sequece length
** @param [r] MedValue [float] Mean value
** @return [void]
** @@
******************************************************************************/

void embComWriteValueOfSeq(AjPFile fp, ajint n,const char *name,
			   ajint len,float MedValue)
{

    ajFmtPrintF(fp,"%10d         %19s %14d     %14.4f \n",
		n,name,len,MedValue);

    return;
}




/* @funcstatic comWriteTable **************************************************
**
** Output table of complexity results
**
** @param [u] fp [AjPFile] Output file
** @param [r] name [const char *] Sequence name
** @param [r] RUj [const UJWin *] RUj values
** @param [r] MedUj [const UJWin *] MedUj values
** @param [r] SDUj [const UJWin *] SDUj values
** @param [r] RatUj [const UJWin *] RatUj values
** @param [r] Nwin [ajint] Number
** @param [r] jmin [ajint] Minimum
** @param [r] jmax [ajint] Maximum
** @return [void]
** @@
******************************************************************************/

static void comWriteTable(AjPFile fp,const char *name,
			  const UJWin *RUj,const UJWin *MedUj,
			  const UJWin *SDUj, const UJWin *RatUj,
			  ajint Nwin, ajint jmin, ajint jmax)
{
    ajint i;
    ajint j;
    ajint k;
    
    ajFmtPrintF(fp,"\n");
    ajFmtPrintF(fp,"Sequence: %s \n\n",name);
    
    
    for(j=jmin;j<=jmax;j++)
	ajFmtPrintF(fp,"%11s%-1d       ","U",j);

    for(j=jmin;j<=jmax;j++)
	ajFmtPrintF(fp,"%5s%-1d","R",j);
    ajFmtPrintF(fp,"\n");
    
    
    
    ajFmtPrintF(fp,"--------------------------------------------"
		"-----------------------------------------------"
		"----------------\n");
    for(i=0;i<Nwin;i++)
    {
	ajFmtPrintF(fp,"w%-2d ",i+1);
	for(k=0;k<jmax-jmin+1;k++)
	    ajFmtPrintF(fp,"%4d  %5.2f+/-%-5.2f",
			(ajint)(RUj[i].Ujwin[k]),MedUj[i].Ujwin[k],
			SDUj[i].Ujwin[k]);
	for(k=0;k<jmax-jmin+1;k++)
	    ajFmtPrintF(fp,"%5.2f ",RatUj[i].Ujwin[k]);
	ajFmtPrintF(fp,"\n");
	ajFmtPrintF(fp,"\n");

    }

    return;
}




/* @funcstatic comCalcComplex *************************************************
**
** Calculation for complexity
**
** @param [r] RUj [const UJWin *] RUj values
** @param [r] MedUj [const UJWin *] MedUj values
** @param [w] RatUj [UJWin *] RatUj values
** @param [r] Nwin [ajint] Window number
** @param [r] Nword [ajint] Word number
** @param [w] CompSeq [float*] Results
** @return [void]
** @@
******************************************************************************/

static void comCalcComplex(const UJWin *RUj,const UJWin *MedUj,
			   UJWin *RatUj,
			   ajint Nwin, ajint Nword, float *CompSeq)
{
    ajint i;
    ajint k;

    for(i=0;i<Nwin;i++)
	CompSeq[i] = 1.0;

    for(i=0;i<Nwin;i++)
	for(k=0;k<Nword;k++)
	{
	    RatUj[i].Ujwin[k] = RUj[i].Ujwin[k]/MedUj[i].Ujwin[k];
	    CompSeq[i] *= RatUj[i].Ujwin[k];
	}

    for(i=0;i<Nwin;i++)
	if(CompSeq[i] > 1.0)
	    CompSeq[i] = 1.0;

    return;
}




/* @funcstatic comCalcMedValue ************************************************
**
** Mean value calculation for complexity
**
** @param [r] SetUj [const UJSim *] SetUj values
** @param [w] MedUj [UJWin *] MedUj values
** @param [w] SDUj [UJWin *] SDUj values
** @param [r] Nsim [ajint] Number of simulations
** @param [r] Nwin [ajint] Window number
** @param [r] Nword [ajint] Word number
** @return [void]
** @@
******************************************************************************/

static void comCalcMedValue(const UJSim *SetUj,UJWin *MedUj,UJWin *SDUj,
			 ajint Nsim, ajint Nwin, ajint Nword)
{
    ajint i;
    ajint j;
    ajint k;
    float *sum;
    float *var;


    AJCNEW0(sum, Nword);

    AJCNEW0(var, Nword);

    for(j=0;j<Nwin;j++)
    {
	for(i=0;i<Nsim;i++)
	    for(k=0;k<Nword;k++)
		sum[k]+=SetUj[i].Ujsim[j].Ujwin[k];

	for(k=0;k<Nword;k++)
	    MedUj[j].Ujwin[k] = sum[k]/(float)(Nsim);

	for(k=0;k<Nword;k++)
	    sum[k] = 0.0;
    }

    for(j=0;j<Nwin;j++)
    {
	for(i=0;i<Nsim;i++)
	    for(k=0;k<Nword;k++)
		var[k]+=(float)(pow((SetUj[i].Ujsim[j].Ujwin[k]
				     - MedUj[j].Ujwin[k]),
				    (float)(2)));

	for(k=0;k<Nword;k++)
	    var[k] = var[k]/(float)(Nsim);


	for(k=0;k<Nword;k++)
	    SDUj[j].Ujwin[k] = (float)sqrt(var[k]);

	for(k=0;k<Nword;k++)
	    var[k] = 0.0;
    }

    AJFREE(sum);
    AJFREE(var);

    return;
}




/* @funcstatic comElabSetSim **************************************************
**
** Set simulations for complexity calculation
**
** @param [r] SetSeqSim [const SEQSim*] SeqSim values
** @param [w] SetUjSim [UJSim *] SetUj values
** @param [r] lseq [ajint] Sequence length
** @param [r] nsim [ajint] Number of simulations
** @param [r] jmin [ajint] Minimum
** @param [r] jmax [ajint] Maximum
** @param [r] lwin [ajint] Window length
** @param [r] step [ajint] Window step size
** @return [void]
** @@
******************************************************************************/

static void comElabSetSim(const SEQSim* SetSeqSim,UJSim *SetUjSim,
			  ajint lseq, ajint nsim,
			  ajint jmin, ajint jmax, ajint lwin, ajint step)
{
    ajint i;

    for(i=0;i<nsim;i++)
	comElabSeq(SetSeqSim[i].Sqsim,SetUjSim[i].Ujsim,jmin,jmax,
		   lwin,lseq,step);

    return;
}




/* @funcstatic comElabSeq *****************************************************
**
** Input function for complexity calculations
**
** @param [r] seq [const char *] Sequence
** @param [w] ujwin [UJWin *] UjWin values
** @param [r] jmin [ajint] Minimum
** @param [r] jmax [ajint] Maximum
** @param [r] lwin [ajint] Window length
** @param [r] lseq [ajint] Sequence length
** @param [r] step [ajint] Window step size
** @return [void]
** @@
******************************************************************************/

static void comElabSeq(const char *seq,UJWin *ujwin, ajint jmin, ajint jmax,
		       ajint lwin, ajint lseq,
		       ajint step)
{
    ajint j;
    ajint k = 0;
    ajint bwin;
    ajint ewin;
    ajint nwin = 0;
    char *wind;
    float Uj   = 0.0;

    AJCNEW(wind, lwin+1);

    bwin = 0;
    ewin = lwin-1;

    while(ewin<lseq)
    {
	nwin++;
	comReadWin(seq,bwin,ewin,wind);
	for(j=jmin;j<=jmax;j++)
	{
	    comCalcUj(lwin,j,wind,&Uj);
	    ujwin[nwin-1].Ujwin[k] = Uj;
	    k++;
	}
	bwin = bwin+step;
	ewin = bwin+(lwin-1);
	k = 0;
    }
    AJFREE(wind);

    return;
}




/* @funcstatic comCalcNOfWin **************************************************
**
** Calculate number of windws for complexity
**
** @param [r] lseq [ajint] Sequence length
** @param [r] lwin [ajint] Window length
** @param [r] step [ajint] Window step size
** @return [ajint] Result
** @@
******************************************************************************/

static ajint comCalcNOfWin(ajint lseq, ajint lwin, ajint step)
{
    ajint bwin;
    ajint ewin;
    ajint nwin;

    bwin = 0;
    ewin = lwin-1;
    nwin = 0;
    while(ewin<lseq)
    {
	nwin++;
	bwin=bwin+step;
	ewin=bwin+(lwin-1);
    }

    return nwin;
}




/* @funcstatic comWriteSimValue ***********************************************
**
** Output simulation value for complexity calculation
**
** @param [r] ComplexOfSim [const float *] Calcualted values
** @param [r] nsim [ajint] Number of simulations
** @param [u] fp [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

static void comWriteSimValue(const float *ComplexOfSim, ajint nsim,AjPFile fp)
{
    ajint i;

    ajFmtPrintF(fp,"VALUES FOR EACH SIMULATION\n\n");
    for(i=0;i<nsim;i++)
	ajFmtPrintF(fp,"%4d %7.4f \n",i+1,ComplexOfSim[i]);

    return;
}




/* @funcstatic comWriteValue  *************************************************
**
** Output of values for complexity calculation
**
** @param [r] name [const char *] Sequence name
** @param [r] lseq [ajint] Sequence length
** @param [r] ComplexOfSeq [const float *] Results array
** @param [r] NumOfWin [ajint] Number of windows
** @param [r] l [ajint] Window length
** @param [r] step [ajint] Window step size
** @param [r] jmin [ajint] Minimum
** @param [r] jmax [ajint] Maximum
** @param [r] sim [ajint] Number of simulations
** @param [r] MedValue [float] Mean value
** @param [u] fp [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

static void comWriteValue(const char *name, ajint lseq,
			  const float *ComplexOfSeq,
			  ajint NumOfWin, ajint l, ajint step,
			  ajint jmin, ajint jmax,
			  ajint sim,float MedValue,AjPFile fp)
{
    ajint i;
    ajint bwin ;
    ajint ewin;

    ajFmtPrintF(fp,"Name of sequence:   %s \n",name);
    ajFmtPrintF(fp,"Length of sequence: %d \n",lseq);
    ajFmtPrintF(fp,"Length of window  : %d \n",l);
    ajFmtPrintF(fp,"Step : %d \n",step);
    ajFmtPrintF(fp,"jmin : %d \n",jmin);
    ajFmtPrintF(fp,"jmax : %d \n",jmax);

    if(sim)
	ajFmtPrintF(fp,"sim : %d \n",sim);

    if(sim == 0)
	ajFmtPrintF(fp,"Execution without simulation\n");

    ajFmtPrintF(fp,"-----------------------------------------"
		"-------------------------------------\n");
    ajFmtPrintF(fp,"|                   |                   |"
		"                   |                |\n");
    ajFmtPrintF(fp,"|     begin         |     end           |"
		"     value of      |  average       |\n");
    ajFmtPrintF(fp,"|     window        |     window        |"
		"    complexity     |                |\n");
    ajFmtPrintF(fp,"|                   |                   |"
		"                   |                |\n");
    ajFmtPrintF(fp,"-----------------------------------------"
		"-------------------------------------\n");


    bwin = 0;
    ewin = l-1;
    for(i=0;i<NumOfWin;i++)
    {
	if(i==NumOfWin-1)
	    ajFmtPrintF(fp,"%10d  %20d  %20.4f %20.4f",
			bwin+1,ewin+1,ComplexOfSeq[i],MedValue);

	if(i<NumOfWin-1)
	    ajFmtPrintF(fp,"%10d  %20d  %20.4f\n",
			bwin+1,ewin+1,ComplexOfSeq[i]);

	bwin = bwin+step;
	ewin = bwin+l-1;
    }
    ajFmtPrintF(fp,"\n");

    return;
}




/* @funcstatic comComplexOnSeq ************************************************
**
** Complexity of sequence
**
** @param [r] seqsim [const char *] Simulated sequence
** @param [r] seq [const char *] Sequence
** @param [r] lseq [ajint] Sequence length
** @param [r] lwin [ajint] Window length
** @param [r] jmin [ajint] Minimum
** @param [r] jmax [ajint] Maximum
** @param [r] step [ajuint] Window step size
** @param [w] ComplexOfSeq [float *] Results array
** @return [void]
** @@
******************************************************************************/

static void comComplexOnSeq(const char *seqsim,const char *seq, ajint lseq,
			    ajint lwin,
			    ajint jmin, ajint jmax, ajuint step,
			    float *ComplexOfSeq)
{
    ajint bwin;
    ajint ewin;
    ajint nwin;
    char *wind;
    char *winsim;
    float ComplexOfWin = 0.0;

    AJCNEW(wind, lwin+1);
    AJCNEW(winsim, lwin+1);

    bwin = 0;
    ewin = lwin-1;
    nwin = 0;

    while(ewin<lseq)
    {
	comReadWin(seq,bwin,ewin,wind);
	comReadWin(seqsim,bwin,ewin,winsim);
	comWinComplex(wind,winsim,lwin,&ComplexOfWin,jmin,jmax);
	ComplexOfSeq[nwin] = ComplexOfWin;
	bwin = bwin+step;
	ewin = bwin+lwin-1;
	nwin++;
    }

    AJFREE(wind);
    AJFREE(winsim);

    return;
}




/* @funcstatic comComplexOnSeq2 ***********************************************
**
** Complexity of sequence.
**
** @param [r] seq [const char *] Sequence
** @param [r] lseq [ajint] Sequence length
** @param [r] lwin [ajint] Window length
** @param [r] jmin [ajint] Minimum
** @param [r] jmax [ajint] Maximum
** @param [r] step [ajint] Window step size
** @param [w] ComplexOfSeq [float *] Results array
** @return [void]
** @@
******************************************************************************/

static void comComplexOnSeq2(const char *seq,
			     ajint lseq, ajint lwin,
			     ajint jmin, ajint jmax, ajint step,
			     float *ComplexOfSeq)
{
    ajint bwin;
    ajint ewin;
    ajint nwin;
    char *wind;
    float ComplexOfWin = 0.0;

    AJCNEW(wind, lwin+1);

    bwin = 0;
    ewin = lwin-1;
    nwin = 0;

    while(ewin<lseq)
    {
	comReadWin(seq,bwin,ewin,wind);
	comWinComplex2(wind,lwin,&ComplexOfWin,jmin,jmax);
	ComplexOfSeq[nwin] = ComplexOfWin;
	bwin = bwin+step;
	ewin = bwin+lwin-1;
	nwin++;
    }

    AJFREE(wind);

    return;
}




/* @funcstatic comSimulSeq ****************************************************
**
** Simulation of sequence for complexity calculation
**
** @param [w] seqsim [char *] Simulation sequence
** @param [r] lseq [ajint] Sequence length
** @param [r] Freq [const comtrace*] Frequency values
** @param [r] ACN [const char *] ACN value
** @param [r] freq [ajint] Frequency count
** @return [void]
** @@
******************************************************************************/

static void comSimulSeq(char *seqsim,
			ajint lseq, const comtrace *Freq,
			const char *ACN, ajint freq)

{
    ajint x;
    ajint k;
    float x1;
    float n1;
    float n2;
    float n3;

    if(freq)
    {
	n1 = Freq[0].pc;
	n2 = n1+Freq[1].pc;
	n3 = n2+Freq[2].pc;
    }
    else
    {
	n1 = 250.0;
	n2 = 500.0;
	n3 = 750.0;
    }

    k = 0;
    while(k<lseq)
    {
	x = rand();
	x1 = fmodf((float)(x),(float)(1000));
	if(x1 > 0.0 && x1 <= n1 && freq)
	{
	    seqsim[k] = ACN[Freq[0].ind];
	    k++;
	}

	if(x1 > 0.0 && x1 <= n1 && !freq)
	{
	    seqsim[k] = ACN[0];
	    k++;
	}

	if(x1 > n1 && x1 <= n2 && freq)
	{
	    seqsim[k] = ACN[Freq[1].ind];
	    k++;
	}

	if(x1 > n1 && x1 <= n2 && !freq)
	{
	    seqsim[k] = ACN[1];
	    k++;
	}

	if(x1 > n2 && x1 <= n3 && freq)
	{
	    seqsim[k] = ACN[Freq[2].ind];
	    k++;
	}

	if(x1 > n2 && x1 <= n3 && !freq)
	{
	    seqsim[k] = ACN[2];
	    k++;
	}

	if(x1 > n3 && freq)
	{
	    seqsim[k] = ACN[Freq[3].ind];
	    k++;
	}

	if(x1 > n3 && !freq)
	{
	    seqsim[k] = ACN[3];
	    k++;
	}
    }
    seqsim[k] = '\0';

    return;
}




/* @funcstatic comSortFreq ****************************************************
**
** Sort frequencies for complexity calculation
**
** @param [u] set [comtrace*] Frequency values
** @return [void]
** @@
******************************************************************************/

static void comSortFreq(comtrace *set)
{
    ajint a;
    ajint b;
    comtrace strutt;

    strutt.ind = 0;
    strutt.pc = 0.;

    for(a=1;a<4;++a)
	for(b=3;b>=a;--b)
	    if(set[b-1].pc>set[b].pc)
		strutt=set[b-1];
		set[b-1]=set[b];
		set[b]=strutt;

    return;
}




/* @funcstatic comCalcComplexMed **********************************************
**
** Calculate mean for complexity
**
** @param [r] ComplexOfSeq [const float*] Sequence complexity values
** @param [r] NumOfWin [ajint] Number of windows
** @param [w] MedValue [float *] Mean value result
** @return [void]
** @@
******************************************************************************/

static void comCalcComplexMed(const float *ComplexOfSeq, ajint NumOfWin,
			      float *MedValue)
{
    ajint i;
    float sum = 0.0;

    ajDebug("CalcComplexMed NumOfWin: %d\n", NumOfWin);
    for(i=0;i<NumOfWin;i++)
    {
	sum += ComplexOfSeq[i];
	ajDebug("ComplexOfSeq[%d] %.2f\n", i, ComplexOfSeq[i]);
    }
    *MedValue = sum/(float)(NumOfWin);

    return;
}




/* @funcstatic comReadWin *****************************************************
**
** Read sequence for complexity
**
** @param [r] seq [const char *] Sequence
** @param [r] bwin [ajint] Begin
** @param [r] ewin [ajint] End
** @param [w] win [char *] Sequence in window
** @return [void]
** @@
******************************************************************************/

static void comReadWin(const char *seq, ajint bwin, ajint ewin,char *win)
{
    ajint i;
    ajint k;

    k=0;
    for(i=bwin;i<=ewin;i++)
    {
	win[k] = seq[i];
	k++;
    }
    win[k] = '\0';

    return;
}




/* @funcstatic comRead_j_mer **************************************************
**
** Read jmer for complexity
**
** @param [r] win [const char *] Sequence for window
** @param [r] lwin [ajint] Window length
** @param [r] jlen [ajint] Word length
** @param [w] str [AjPStr*] Words
** @return [void]
** @@
******************************************************************************/

static void comRead_j_mer(const char *win, ajint lwin, ajint jlen,AjPStr *str)
{
    ajint i;
    ajint r;
    ajint k;
    ajint t;
    char *temp;

    AJCNEW(temp, jlen+1);

    t = 0;
    r = 0;
    for(i=0;i<(lwin-jlen+1);i++)
    {
	for(k=t;k<t+jlen;k++)
	{
	    temp[r]=win[k];
	    r ++;
	}
	temp[jlen] = '\0';
	ajStrAssignC(&str[i],temp);
	t++;
	r = 0;
    }

    AJFREE(temp);

    return;
}




/* @funcstatic comWinComplex2 *************************************************
**
** Complexity over window
**
** @param [r] win [const char*] Sequence in window
** @param [r] lwin [ajint] Window length
** @param [w] ComplexOfWin [float*] Result
** @param [r] jmin [ajint] Minimum
** @param [r] jmax [ajint] Maximum
** @return [void]
** @@
******************************************************************************/

static void comWinComplex2(const char *win, ajint lwin,float *ComplexOfWin,
			ajint jmin, ajint jmax)
{
    ajint j;

    float Ujvalue  = 0.0;
    float WinValue = 1.0;


    for(j=jmin;j<=jmax;j++)
    {
	comCalcUj2(lwin,j,win,&Ujvalue);
	WinValue = WinValue*Ujvalue;
    }

    *ComplexOfWin = WinValue;

    return;
}




/* @funcstatic comWinComplex **************************************************
**
** Complexity over window
**
** @param [r] win [const char*] Sequence for window
** @param [r] winsim [const char*] Simulation sequence for window
** @param [r] lwin [ajint] Window length
** @param [w] ComplexOfWin [float*] Result
** @param [r] jmin [ajint] Minimum
** @param [r] jmax [ajint] Maximum
** @return [void]
** @@
******************************************************************************/

static void comWinComplex(const char *win,const char *winsim, ajint lwin,
			  float *ComplexOfWin, ajint jmin, ajint jmax)
{
    ajint j;

    float Ujreal   = 0.0;
    float Ujsim    = 0.0;
    float Ujvalue  = 0.0;
    float WinValue = 1.0;

    for(j=jmin;j<=jmax;j++)
    {
	comCalcUj(lwin,j,win,&Ujreal);
	comCalcUj(lwin,j,winsim,&Ujsim);

	if((Ujreal/Ujsim) > 1.0)
	    Ujvalue = 1.0;
	else
	    Ujvalue = Ujreal/Ujsim;
	WinValue = WinValue*Ujvalue;
    }

    *ComplexOfWin = WinValue;

    return;
}




/* @funcstatic comCalcUj2 *****************************************************
**
** Uj calculation for complexity
**
** @param [r] lwin [ajint] Window length
** @param [r] jlen [ajint] Word length
** @param [r] win [const char *] Sequece for window
** @param [w] Ujvalue [float *] Results
** @return [void]
** @@
******************************************************************************/

static void comCalcUj2(ajint lwin, ajint jlen,const char *win,float *Ujvalue)
{

    ajint unlikej_mer = 0;
    ajint k;
    ajint i;
    float z = 0.0;
    float n = 0.0;
    AjPStr *str;
    ajint njmer;

    njmer = (lwin-jlen+1);
    AJCNEW(str, njmer);

    comRead_j_mer(win,lwin,jlen,str);
    qsort(str,njmer,sizeof(AjPStr),(ajint (*)(const void *str_1,
					      const void *str_2)) ajStrVcmp);
    unlikej_mer = comCounter(str,lwin-jlen+1);

    n=(float)pow((double)4,(double)jlen);

    k=(ajint)(n);
    if(lwin > k+jlen-1)
	z=((float)(unlikej_mer)/n);
    else
	z=((float)(unlikej_mer)/(float)(njmer));

    *Ujvalue = z;

    for(i=0;i<njmer;i++)
	ajStrDel(&str[i]);

    AJFREE(str);

    return;
}




/* @funcstatic comCalcUj ******************************************************
**
** Uj calculation for complexity
**
** @param [r] lwin [ajint] Window length
** @param [r] jlen [ajint] Word length
** @param [r] win [const char *] Sequece for window
** @param [w] Ujvalue [float *] Results
** @return [void]
** @@
******************************************************************************/

static void comCalcUj(ajint lwin, ajint jlen,const char *win,float *Ujvalue)
{
    ajint unlikej_mer = 0;
    float z = 0.0;
    AjPStr *str;
    ajint njmer;

    njmer = (lwin-jlen+1);
    AJCNEW(str, njmer);

    comRead_j_mer(win,lwin,jlen,str);
    qsort(str,njmer,sizeof(AjPStr),(ajint (*)(const void *str_1,
					      const void *str_2))ajStrVcmp);
    unlikej_mer = comCounter(str,lwin-jlen+1);

    z=(float)(unlikej_mer);
    *Ujvalue = z;

    AJFREE(str);

    return;
}




/* @funcstatic comCounter *****************************************************
**
** Counter of matches for complexity
**
** @param [r] str [AjPStr const*] Words
** @param [r] k [ajint] Number of words
** @return [ajint] Counter returned
** @@
******************************************************************************/

static ajint comCounter(AjPStr const * str, ajint k)

{
    ajint i;
    ajint count = 1;


    for(i=0;i<k-1;i++)
	if((ajStrCmpS(str[i],str[i+1])) < 0)
	    count++;

    return count;
}




/* @funcstatic comAmbiguity ***************************************************
**
** Ambiguity replaced randomly for complexity
**
** @param [w] seq [char*]  Sequence
** @return [void]
** @@
******************************************************************************/

static void comAmbiguity(char *seq)
{
    char *ch;

    char R[] = "AG";
    char Y[] = "TC";
    char W[] = "AT";
    char S[] = "GC";
    char M[] = "AC";
    char K[] = "GT";
    char H[] = "ATC";
    char B[] = "GCT";
    char V[] = "GAC";
    char D[] = "GAT";
    char N[] = "ACGT";

    ch  = seq;
    while(*ch)
    {
	switch(*ch)
	{
	case 'R':
	    comReplace(R,ch);
	    break;
	case 'Y':
	    comReplace(Y,ch);
	    break;
	case 'W':
	    comReplace(W,ch);
	    break;
	case 'S':
	    comReplace(S,ch);
	    break;
	case 'M':
	    comReplace(M,ch);
	    break;
	case 'K':
	    comReplace(K,ch);
	    break;
	case 'H':
	    comReplace(H,ch);
	    break;
	case 'B':
	    comReplace(B,ch);
	    break;
	case 'V':
	    comReplace(V,ch);
	    break;
	case 'D':
	    comReplace(D,ch);
	    break;
	case 'N':
	    comReplace(N,ch);
	    break;
	}
	ch++;
    }

    return;
}




/* @funcstatic comReplace *****************************************************
**
** Selection of a random base for complexity simulation
**
** @param [r] vet [const char*] Source
** @param [w] ch [char*] Selected character (passed in as the original
**                        position in the sequence, which is why this
**                        function can perform a replace)
** @return [void]
** @@
******************************************************************************/

static void comReplace(const char *vet,char *ch)
{
    ajint dim;
    ajint x;
    ajint x1;
    time_t   tm;

    dim = (ajint) strlen(vet);
    srand((unsigned) time(&tm));
    x  = rand();
    x1 = (x%dim);

    *ch = vet[x1];

    return;
}




/* @funcstatic comCalcFreqACN *************************************************
**
** ACN Frequency calculation for complexity
**
** @param [r] seq [const char*] Sequence
** @param [r] lseq [ajint] Sequence length
** @param [w] Freq [float*] Result array
** @return [void]
** @@
******************************************************************************/

static void comCalcFreqACN(const char *seq, ajint lseq,float *Freq)
{
    ajint countA = 0;
    ajint countC = 0;
    ajint countG = 0;
    ajint countT = 0;
    const char *ch;

    ch = seq;
    while(*ch!='\0')
    {
	switch(*ch)
	{
	case 'A':
	    countA++;
	    break;
	case 'C':
	    countC++;
	    break;
	case 'G':
	    countG++;
	    break;
	case 'T':
	    countT++;
	    break;
	}
	ch++;
    }

    Freq[0] = ((float)(countA)*(float)(100))/(float)(lseq);
    Freq[1] = ((float)(countC)*(float)(100))/(float)(lseq);
    Freq[2] = ((float)(countG)*(float)(100))/(float)(lseq);
    Freq[3] = ((float)(countT)*(float)(100))/(float)(lseq);

    return;
}




/* @func embComUnused *********************************************************
**
** Unused functions in embcom to avoid compiler warnings
**
** @return [void]
******************************************************************************/

void embComUnused(void)
{
    float cosval      = 0.0;
    ajint nsim     = 0;
    ajint lseq     = 0;
    ajint lwin     = 0;
    ajint jmin     = 0;
    ajint jmax     = 0;
    ajuint step     = 1;
    AjPFile fp     = NULL;
    char* seqsim   = NULL;
    char* seq      = NULL;

    comWriteSimValue(&cosval, nsim, fp);
    comComplexOnSeq(seqsim, seq, lseq, lwin, jmin, jmax, step, &cosval);
    comAmbiguity(seq);

    return;
}

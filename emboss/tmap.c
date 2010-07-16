/* @source tmap application
**
** Predict transmembrane regions
**
** @author Copyright (C) Bengt Persson
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

/*
**
** +-------------------------------+
** | Program       tmap.c          |
** | Edition 46    1994-04-21      |
** | Copyright (c) Bengt Persson   |
** +-------------------------------+------------------------------+
** | This program predicts transmembrane segments in proteins,    |
** | utilising the algorithm described in:                        |
** | "Persson, B. & Argos, P. (1994) Prediction of transmembrane  |
** | segments in proteins utilsing multiple sequence alignments   |
** | J. Mol. Biol. 237, 182-192.                                  |
** +--------------------------------------------------------------+
** | Users of this program are kindly asked to cite the above     |
** | reference in publications (or other types of presentation).  |
** +--------------------------------------------------------------+
** | Questions, suggestions and comments are welcomed             |
** | electronically (see below). Best wishes and good luck!       |
** +--------------------------------------------------------------+
** | E-mail addresses:  Persson@EMBL-Heidelberg.DE                |
** |                    Argos@EMBL-Heidelberg.DE                  |
** +--------------------------------------------------------------+
**
** Converted for EMBOSS by Ian longden il@sanger.ac.uk (18/6/99)
** Report output by Henrikki Almusa and Peter Rice (July 2003)
*/

#include "emboss.h"
#include <limits.h>
#include <float.h>

#define UTGAVA "46"

#define NOLLVARDE FLT_MIN /*-10.000*/

#define N_SPANN 4
#define M_SPANN 21
#define C_SPANN 4
#define N_FORLANGNING 10
#define C_FORLANGNING 10
#define N_FOSFAT 4
#define C_FOSFAT 4

#define FORLC 8
#define FORLN 8

#define START_E_KORR 2
#define STOPP_E_KORR 2

#define M_KORT2_LIMIT 8
#define M_KORT3_LIMIT 16
#define M_LANG1_LIMIT 29

#define ALI_MINIMUM  0.30            /* Proportion of sequences that
					should be present at a
					position in order to utilise
					the data from that
					position.  */
#define E_SPANN_MIN 20
#define E_SPANN_MAX 33
#define E_STST_DIFF 6

#define GAP '-'



static float **profile;


/* P values and spans */

static ajint pp_antal=2;
static ajint glspan[2] = { 15, 4 };

static float P[2][26] =
{
    /* Pm values */
    
    {
	(float) 1.409446,
	(float) 0.000000,
	(float) 1.068500,
	(float) 0.192356,
	(float) 0.174588,
	(float) 1.965858,
	(float) 1.058479,
	(float) 0.587963,
	(float) 1.990336,
	(float) 0.000000,
	(float) 0.180983,
	(float) 1.701726,
	(float) 1.500664,
	(float) 0.433590,
	(float) 0.000000,
	(float) 0.518571,
	(float) 0.344232,
	(float) 0.239737,
	(float) 0.774442,
	(float) 0.828131,
	(float) 0.000000,
	(float) 1.694256,
	(float) 1.314157,
	(float) 0.000000,
	(float) 0.979187,
	(float) 0.000000
    },
	    
	    /* Pe values */
	    
	{
	    (float) 0.887866,
	    (float) 0.000000,
	    (float) 0.842097,
	    (float) 0.739931,
	    (float) 0.804004,
	    (float) 1.102175,
	    (float) 0.919923,
	    (float) 1.117477,
	    (float) 1.103394,
	    (float) 0.000000,
	    (float) 1.178047,
	    (float) 0.997766,
	    (float) 1.171823,
	    (float) 1.103455,
	    (float) 0.000000,
	    (float) 0.881061,
	    (float) 0.889218,
	    (float) 1.519044,
	    (float) 0.919717,
	    (float) 0.881105,
	    (float) 0.000000,
	    (float) 0.869741,
	    (float) 1.450220,
	    (float) 0.000000,
	    (float) 1.314105,
	    (float) 0.000000
	}
};


static char **s = NULL;            /* Sequences  */
static ajint *relc = NULL;
static ajint *reln = NULL;

static ajint glnr;
static ajint glpos;
static ajuint glposs;

static float *norm_skillnad = NULL;

static ajint tm_number;
static ajint **tm_segment = NULL;
static ajint *glnpos = NULL;
static ajint *glcpos = NULL;

static ajint *pred_mode = NULL;
static ajint e_spann_max;

static float mx_limit;

static ajint *ali_ok = NULL;




static void tmap_profile2(ajint prof, ajint antal, ajint poss, ajint span);
static float tmap_length1(ajint nr, ajint start, ajint stopp);
static void tmap_present3p(ajint antal, const ajint *npos, const ajint *cpos,
			   ajint poss,
			   ajint nr, const AjPSeqset seqset,
			   AjPReport outfile);
static ajint tmap_peak1(ajint start, ajint stopp, float *parameter);
static ajint tmap_vec_to_stst(ajint *vec, ajint *start, ajint *stopp,
			      ajint length);
static void tmap_weights(char **, ajint, ajint, float *);
static void tmap_refpos2(ajint, ajint);
static float tmap_summa1(ajint start, ajint stopp, const float *parameter);
static ajint tmap_pred1(float, float, float, ajint);
static ajint tmap_insert_in_vector(ajint *start, ajint *stopp,
				   ajint max,
				   ajint starttmp, ajint stopptmp,
				   ajint *pred, ajint predparameter);
static ajint tmap_tm_in_vector(const ajint *start, const ajint *stopp,
			       ajint max,
			       ajint starttmp, ajint stopptmp);
static void tmap_align_rel(ajint antal, ajint poss, ajint span);
static void tmap_plot2(AjPGraph mult);
static ajint tmap_all_charged(ajint pos, ajint nr);




/* @prog tmap *****************************************************************
**
** Displays membrane spanning regions
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqset seqset;
    AjPGraph mult;
    AjPReport report;

    ajint i;
    ajint j;
    ajuint jj;
    float m_limit;
    float ml_limit;
    float e_limit;

    embInit("tmap", argc, argv);

    seqset = ajAcdGetSeqset("sequences");
    mult   = ajAcdGetGraphxy("graph");
    report = ajAcdGetReport("outfile");

    e_spann_max = E_SPANN_MAX;

    m_limit  = (float) 1.23;
    ml_limit = (float) 1.17;
    e_limit  = (float) 1.07;

    mx_limit = (float) 1.18;
    /* me_limit = 1.10; Unused */


    glnr = ajSeqsetGetSize(seqset);
    glposs = ajSeqsetGetLen(seqset);
    ajDebug("tmap nr: %d poss %d\n", glnr, glposs);

    AJCNEW0(s, glnr);
    ajDebug("tmap s %x \n", s);
    for(i=0;i<glnr;i++)
    {
	AJCNEW0(s[i], glposs+1);
	ajDebug("tmap s[%d] %x \n", i, s[i]);
    }

    AJCNEW0(relc, glposs+1);
    AJCNEW0(reln, glposs+1);
    AJCNEW0(ali_ok, glposs+1);
    AJCNEW0(glnpos, glposs+1);
    AJCNEW0(glcpos, glposs+1);

    AJCNEW0(norm_skillnad, glnr);

    glnr--;

    for(i=0;i<=glnr;i++)
    {
	const char *temp;

	ajSeqsetFmtUpper(seqset);
	temp = ajSeqsetGetseqSeqC(seqset, i);
	for(jj=0;jj<glposs;jj++)
	    s[i][jj+1] =  temp[jj];
    }

    if(!glnr)
	norm_skillnad[0] = 1;
    else
	tmap_weights(s,glposs,glnr,norm_skillnad);

    ajDebug("tmap pp_antal: %d\n", pp_antal);
    AJCNEW0(profile, pp_antal);
    for(i=0;i<pp_antal; i++)
	AJCNEW0(profile[i], glposs+1);

    for(j=0; j<pp_antal; j++)
    {
	tmap_align_rel(glnr,glposs,glspan[j]);
	tmap_profile2(j,glnr, glposs, glspan[j]);
    }

    tm_number = tmap_pred1(m_limit,ml_limit,e_limit,glnr);

    ajDebug("tmap tm_number: %d\n", tm_number);
    AJCNEW0(tm_segment, tm_number+1);
    for(i=0;i<=tm_number;i++)
	AJCNEW0(tm_segment[i], 2);

    tmap_present3p(tm_number, glnpos, glcpos, glposs, glnr, seqset, report);
    ajReportSetSeqsetstats(report, seqset);

    for(j=1; j<=tm_number; j++)
    {
	tm_segment[j][0] = glnpos[j]+N_SPANN;
	tm_segment[j][1] = glcpos[j]-C_SPANN;
    }

    tmap_plot2(mult);

    ajDebug("tmap done .. cleaning up\n");
    ajSeqsetDel(&seqset);
    ajGraphxyDel(&mult);
    ajReportClose(report);
    ajReportDel(&report);

/* note: glnr is one less than it was at the start */

    for(i=0;i<=glnr;i++)
	AJFREE(s[i]);
    AJFREE(s);
    AJFREE(relc);
    AJFREE(reln);
    AJFREE(ali_ok);
    AJFREE(glnpos);
    AJFREE(glcpos);
    AJFREE(norm_skillnad);
    for(i=0;i<pp_antal; i++)
	AJFREE(profile[i]);
    AJFREE(profile);
    for(i=0;i<=tm_number;i++)
	AJFREE(tm_segment[i]);
    AJFREE(tm_segment);

    embExit();

    return 0;
}




/* @funcstatic tmap_refpos2 ***************************************************
**
** tmap_refpos2, utg. 44
** ----------------
** Transforms the positional numbers of alignment into those of a
** reference sequence
** in each alignment.
**
** Global variables:
** -----------------
** relc[] and reln[] - vectors containing the "real" positional numbers
**
**
** @param [r] refnr [ajint] Undocumented
** @param [r] poss [ajint] Undocumented
** @@
******************************************************************************/

static void tmap_refpos2(ajint refnr, ajint poss)
{
    ajint i;
    ajint temp;

    for(i=0; i<poss; i++)
	relc[i]=reln[i]=0;

    temp = 1;
    for(i=1; i<=poss; i++)
    {
	reln[i] = temp;

	if(s[refnr][i]!=GAP)
	    ++temp;
    }

    temp = 0;

    for(i=1; i<=poss; i++)
    {
	if(s[refnr][i]!=GAP)
	    ++temp;
	relc[i]=temp;
    }

    return;
}




/* @funcstatic tmap_all_charged ***********************************************
**
** Calculates if all residues at an alignment position are identical
** and one of DEKRQN
**
** Returns 1 if so, otherwise 0
**
** @param [r] pos [ajint] Undocumented
** @param [r] nr [ajint] Undocumented
** @return [ajint] Undocumented
** @@
******************************************************************************/

static ajint tmap_all_charged(ajint pos, ajint nr)
{
    ajint i;
    ajint likhet;

    for(i=1,likhet=1; i<=nr; i++)
	if(s[0][pos]!=s[i][pos])
	    likhet=0;

    if(likhet==1)
    {
	if((s[0][pos]=='D') || (s[0][pos]=='E') ||
	   (s[0][pos]=='K') || (s[0][pos]=='R'))
	    return 1;
    }

    return 0;
}




/* @funcstatic tmap_length1 ***************************************************
**
** Calculates the real medium length of TM segment
** Eliminates from the calculations sequences with less than 4 positions
**
** @param [r] nr [ajint] Undocumented
** @param [r] start [ajint] Undocumented
** @param [r] stopp [ajint] Undocumented
** @return [float] medium length of TM segment
** @@
******************************************************************************/

static float tmap_length1(ajint nr, ajint start, ajint stopp)
{
    ajint i;
    ajint j;
    ajint l;
    ajint ll;
    ajint *correct_sequence;
    ajint nr_correct;

    AJCNEW0(correct_sequence, nr+1);

    /* First, check for sequences with less than 10 a. a. residues */
    nr_correct = 0;

    for(i=0; i<=nr; i++)
    {
	for(j=start, l=0; j<=stopp; j++)
	    if(s[i][j]!=GAP) l++;

	if(l>=10)
	{
	    correct_sequence[i]=1;
	    nr_correct++;
	}
    }

    /*
    **  Second, check for lengths among the sequences that contain >=10 a. a.
    **  residues
    */
    for(i=0, ll=0; i<=nr; i++)
	if(correct_sequence[i]==1)
	{
	    for(j=start, l=0; j<=stopp; j++)
		if(s[i][j]!=GAP) l++;
	    ll += l;
	}

    AJFREE(correct_sequence);

    if(nr_correct!=0)
	return (float)ll/nr_correct;

    return 0;
}




/* @funcstatic tmap_present3p *************************************************
**
** Presents results from predictions
**
** @param [r] antal [ajint] Undocumented
** @param [r] npos [const ajint*] Undocumented
** @param [r] cpos [const ajint*] Undocumented
** @param [r] poss [ajint] Undocumented
** @param [r] nr [ajint] Undocumented
** @param [r] seqset [const AjPSeqset] Undocumented
** @param [u] report [AjPReport] Undocumented
** @@
******************************************************************************/

static void tmap_present3p(ajint antal, const ajint *npos, const ajint *cpos,
			   ajint poss, ajint nr, const AjPSeqset seqset,
			   AjPReport report)

{
    ajint i;
    ajint j;
    AjPFeattable feat;
    AjPFeature seqf;
    AjPStr tmp = NULL;
    AjPStr hdr = NULL;
    AjPSeq seq;
    const AjPSeq cseq;
    AjPStr cons = NULL;
    ajint calcid;
    ajint calcsim;
    ajint calcgap;
    ajint calclen;

    ajAlignConsStats(seqset, NULL, &cons,
		     &calcid, &calcsim, &calcgap, &calclen);

    ajStrAssignC(&hdr, "");
    ajReportSetHeaderS(report, hdr);

    seq = ajSeqNewRes(ajSeqsetGetSize(seqset));
    ajSeqSetProt(seq);
    ajSeqAssignNameC(seq, "Consensus");
    ajSeqAssignSeqS(seq, cons);
    feat = ajFeattableNewSeq(seq);

    for(i=1; i<=antal; i++)
    {
        seqf=ajFeatNewII(feat,npos[i],cpos[i]);
	ajFmtPrintS(&tmp,"*TM %2d",  i);
	ajFeatTagAdd(seqf,NULL,tmp);
    }

    ajReportWrite(report, feat, seq);
    ajFeattableDel(&feat);
    ajSeqDel(&seq);

    for(j=0; j<=nr; j++)
    {
	cseq = ajSeqsetGetseqSeq(seqset, j);
	feat = ajFeattableNewSeq(cseq);
	tmap_refpos2(j, poss);
	ajStrAssignC(&hdr, "");
	ajReportSetHeaderS(report, hdr);
	for(i=1; i<=antal; i++)
	{
	    seqf=ajFeatNewII(feat,npos[i],cpos[i]);
	    ajFmtPrintS(&tmp,"*TM %2d",i);
	    ajFeatTagAdd(seqf,NULL,tmp);
	}
	ajReportWrite(report, feat, cseq);
	ajFeattableDel(&feat);
    }

    ajStrDel(&cons);
    ajStrDel(&hdr);
    ajStrDel(&tmp);
    return;
}




/* @funcstatic tmap_pred1 *****************************************************
**
** Prediction algorithm
** Returns number of transmembrane segments
**
** @param [r] m_limit [float] Undocumented
** @param [r] ml_limit [float] Undocumented
** @param [r] e_limit [float] Undocumented
** @param [r] nr [ajint] Undocumented
** @return [ajint] number of segments
** @@
******************************************************************************/

static ajint tmap_pred1(float m_limit, float ml_limit, float e_limit, ajint nr)
{
    ajint i;
    ajint j;
    ajuint ii;
    ajuint jj;
    ajint k;
    ajint tm_ant;
    ajint flag;
    ajint length;

    ajint *start = NULL;
    ajint *stopp = NULL;
    ajint *hitposs = NULL;

    ajint avstand;
    ajint mitt;
    ajint start0;

    ajint *start_e_pos = NULL;
    ajint *stopp_e_pos = NULL;

    float sum;

    ajint count;
    ajint count2;
    ajint tempN;
    ajint tempC;

    ajint starttmp;
    ajint stopptmp;
    ajint temp;
    float tf;
    
    AJCNEW0(hitposs, glposs+1);
    AJCNEW0(pred_mode, glposs+1);

    ajDebug("tmap pred_mode tm_number:%d\n", tm_number);

    for(i=0; i<tm_number; i++)
	pred_mode[i] = 0;

    /* Find peak values */
    for(jj=1; jj<=glposs; jj++)
	if(profile[0][jj]>m_limit)
	    hitposs[jj] = 1;
	else
	    hitposs[jj] = 0;


    /* Smoothing: Disregard 1 or 2 consecutive positions in vector hitposs[] */
    for(ii=3; ii<=glposs-1; ii++)
	if((hitposs[ii-2]==1) && (hitposs[ii+1]==1))
	    hitposs[ii] = hitposs[ii-1]=1;
    for(ii=2; ii<=glposs-1; ii++)
	if((hitposs[ii-1]==1) && (hitposs[ii+1]==1))
	    hitposs[ii] = 1;


    AJCNEW0(start, glposs+1);
    AJCNEW0(stopp, glposs+1);
    AJCNEW0(start_e_pos, glposs+1);
    AJCNEW0(stopp_e_pos, glposs+1);

    /* Transform hitposs[] to TM vector */
    for(ii=0; ii<=glposs; ii++)
	glnpos[ii]=glcpos[ii]=0;
    tm_ant = tmap_vec_to_stst(hitposs,start,stopp,glposs);

    for(i=1; i<=tm_ant; i++) {
	ajDebug("tmap pred_mode[%d]\n", i);
	ajDebug("tmap pred_mode[%d] %x\n", i, pred_mode[i]);
	pred_mode[i]=pred_mode[i] | 1;
    }


    /* Remove start[] & stopp[] with length <=M_KORT2_LIMIT */
    for(i=1; i<=tm_ant; i++)
	if(stopp[i]-start[i]<M_KORT2_LIMIT-1)
	{
	    /*
	    **  Correct for if strictly conserved charges are present,
	    **  thus reducing the mean value
	    */
	    count2 = 0;

	    count = stopp[i]-start[i]+1;
	    for(j=stopp[i]+1; j<=(ajint)glposs && profile[0][j]>mx_limit; j++)
		if(profile[0][j]>mx_limit)
		    count++;
	    tempC = j-1;
	    for(j=start[i]-1; j>0 && profile[0][j]>mx_limit; j--)
		if(profile[0][j]>mx_limit)
		    count++;
	    tempN = j+1;

	    mitt = (tempC-tempN)/2 + tempN;

	    count2 = 0;
	    if(count>8)
		if(mitt>8)
		    for(j=mitt-8; j<=mitt+8 && j<=(ajint)glposs; j++)
			if(tmap_all_charged(j,nr))
			    count2++;


	    /* ... this was not the case ... */

	    if(count2==0)
	    {
		for(j=i; j<=tm_ant-1; j++)
		{
		    start[j]=start[j+1];
		    stopp[j]=stopp[j+1];
		    pred_mode[j]=pred_mode[j+1];
		}
		i--;
		tm_ant--;
	    }

	}






    /* 3.
    ** Starting in 'start[]' and 'stopp[]', expand N- and C-terminally
    ** - each step is taken in the direction of highest profile[0] value
    ** - as ajlong 'over_limit' is true
    **  until langd=M_SPAN
    */




    stopp[0] = 0;
    start[tm_ant+1] = glpos;


    for(i=1; i<=tm_ant; i++)
    {
	flag = 1;
	while(flag)
	{
	    if((profile[0][start[i]-1]>ml_limit) ||
		(profile[0][stopp[i]+1]>ml_limit))
	    {
		if(profile[0][start[i]-1] > profile[0][stopp[i]+1])
		{
		    if((start[i]>1) && (profile[0][start[i]-1]>ml_limit) &&
			(start[i]>stopp[i-1]+FORLC))
			start[i]--;
		    else
		    {
			if((stopp[i]<(ajint)glposs) && (profile[0][stopp[i]+1]>
						 ml_limit) &&
			    (stopp[i]<start[i+1]-FORLN))
			    stopp[i]++;
			else
			    flag = 0;
		    }
		}
		else
		    if((stopp[i]<(ajint) glposs) &&
			(profile[0][stopp[i]+1]>ml_limit) &&
			(stopp[i]<start[i+1]-FORLN))
			stopp[i]++;
		    else
		    {
			if((start[i]>1) && (profile[0][start[i]-1]>ml_limit)
			    && (start[i]>stopp[i-1]+FORLC))
			    start[i]--;
			else
			    flag = 0;
		    }
	    }
	    else
		flag = 0;
	}
    }



    /*
    ** 4.
    ** Elongate with N_FOSFAT and C_FOSFAT, respectively, to correct for
    ** that the prediction hitherto
    ** has been focused to find only the hydrophobic portion of the
    ** transmembrane segment
    */

    for(i=1; i<=tm_ant; i++)
    {
	if(start[i]>N_FOSFAT)
	    start[i]-=N_FOSFAT;

	if(stopp[i]<(ajint)glposs-C_FOSFAT)
	    stopp[i]+=C_FOSFAT;
    }




    /*
    ** 5.1.
    ** Search for Pe values
    */

    for(i=1; i<=tm_ant; i++)
    {
	mitt = start[i]+(stopp[i]-start[i])/2;
	start_e_pos[i]=tmap_peak1(mitt-20,mitt,profile[1]);
	stopp_e_pos[i]=tmap_peak1(mitt,mitt+20,profile[1]);


	/* Use the Pe values, if they are good */

	if((profile[1][start_e_pos[i]]>=e_limit) &&
	    (profile[1][stopp_e_pos[i]]>=e_limit))
	{
	    if((tmap_length1(nr,start_e_pos[i],stopp_e_pos[i])>=
		  E_SPANN_MIN) &&
		(tmap_length1(nr,start_e_pos[i],stopp_e_pos[i])<=
		 E_SPANN_MAX))
	    {
		start[i] = start_e_pos[i]-START_E_KORR;
		stopp[i] = stopp_e_pos[i]+STOPP_E_KORR;
	    }

	}
	else
	{
	    if(profile[1][start_e_pos[i]]>=e_limit)
		if(abs(start_e_pos[i]-start[i])<E_STST_DIFF)
		    start[i] = start_e_pos[i]-START_E_KORR;

	    if(profile[1][stopp_e_pos[i]]>=e_limit)
		if(abs(stopp_e_pos[i]-stopp[i])<E_STST_DIFF)
		    stopp[i] = stopp_e_pos[i]+STOPP_E_KORR;
	}



    } /* i, E-varden */



    /* Check for Pe values */

    for(i=20; i<=(ajint) glposs-20; i++)
    {
	starttmp = tmap_peak1(i-19,i,profile[1]);
	stopptmp = tmap_peak1(i+1,i+20,profile[1]);
	temp=0;

	if((profile[1][starttmp]>1.15) && (profile[1][stopptmp]>1.15))
	    if((tmap_length1(nr,starttmp,stopptmp)>=E_SPANN_MIN /* >17 */)
		&&
		(tmap_length1(nr,starttmp,stopptmp)<=e_spann_max /*<30*/))
		for(j=starttmp+2, temp=1; j<=stopptmp-2; j++)
		    if(profile[0][j]<1.08)
			temp = 0;

	if(temp==1)
	    if(!(tmap_tm_in_vector(start,stopp,tm_ant,starttmp,stopptmp)))
		tm_ant = tmap_insert_in_vector(start,stopp,tm_ant,starttmp,
					       stopptmp,pred_mode,16);

    }


    /*
    ** 6.2.
    ** Correction for overlap
    */

    for(i=2; i<=tm_ant; i++)
	if(start[i]<stopp[i-1])
	{
	    stopp[i-1] = stopp[i];
	    for(j=i; j<tm_ant; j++)
	    {
		start[j] = start[j+1];
		stopp[j] = stopp[j+1];
		pred_mode[j] = pred_mode[j+1];
	    }
	    tm_ant--;
	    i--;
	}



    /*
    ** 6.3.
    ** Divide segments long enough to contain several tm segments
    */

    for(i=1; i<=tm_ant; i++)
    {
	tf = tmap_length1(nr,start[i],stopp[i]);
	length = (ajint) tf;
	for(j=10; j>=2; j--)
	{
	    if(length>=j*M_SPANN+(j-1)*(N_SPANN+C_SPANN-1))
	    {
		/*
		**  For att korrigera for turn precis i membrankanten kan
		**  langdkriteriet usteras litet grand:
		*/
		tm_ant += j-1;
		for(k=tm_ant; k>=i+1; k--)
		{
		    start[k] = start[k-(j-1)];
		    stopp[k] = stopp[k-(j-1)];
		    pred_mode[k] = pred_mode[k-(j-1)];
		}

		avstand = length/j;
		start0  = start[i];



		for(k=1; k<=j; k++)
		{
		    mitt=avstand/2+avstand*(k-1);
		    start[i+k-1]=start0+mitt-N_FORLANGNING;
		    stopp[i+k-1]=start0+mitt+C_FORLANGNING;
		    pred_mode[i+k-1]=pred_mode[i+k-1] | 8;
		}

		j = 0;
	    } /* if */
	} /* j */
    } /* i */




    /*
    ** 7.
    ** Remove too short segments
    */

    for(i=1; i<=tm_ant; i++)
	if((tmap_length1(nr,start[i],stopp[i]) < M_KORT3_LIMIT))
	{
	    for(j=i; j<=tm_ant-1; j++)
	    {
		start[j] = start[j+1];
		stopp[j] = stopp[j+1];
		pred_mode[j] = pred_mode[j+1];
	    }
	    i--;
	    tm_ant--;
	}



    /*
    ** 8.
    ** Shorten segments, longer than M_SPANN aa
    */

    for(i=1; i<=tm_ant; i++)
	if((tmap_length1(nr,start[i],stopp[i])>M_LANG1_LIMIT))
	{
	    /* Forst kolla om segmentet tangerar forutvarande */
	    if(i>1)
		if(start[i]<stopp[i-1])
		    start[i] = stopp[i-1];

	    sum=0;
	    start0=start[i];
	    for(j=start[i]; j<=stopp[i]-M_LANG1_LIMIT+1; j++)
		if(tmap_summa1(j, j+M_LANG1_LIMIT-1, profile[0]) > sum)
		{
		    sum = tmap_summa1(j, j+M_LANG1_LIMIT-1, profile[0]);
		    start0 = j;
		}
	    start[i] = start0;
	    stopp[i] = start0+M_LANG1_LIMIT-1;
	    pred_mode[i] = pred_mode[i] | 128;
	}

    for(i=1; i<=tm_ant; i++)
    {
	glnpos[i] = start[i];
	glcpos[i] = stopp[i];
    }

    AJFREE(hitposs);
    AJFREE(pred_mode);
    AJFREE(start);
    AJFREE(stopp);
    AJFREE(start_e_pos);
    AJFREE(stopp_e_pos);

    return tm_ant;
}




/* @funcstatic tmap_peak1 *****************************************************
**
** Finds peak value in the vector 'parameter'
** and returns position of peak value
**
** @param [r] start [ajint] Undocumented
** @param [r] stopp [ajint] Undocumented
** @param [w] parameter [float*] Undocumented
** @return [ajint] peak value
** @@
******************************************************************************/

static ajint tmap_peak1(ajint start, ajint stopp, float *parameter)
{
    ajint i;
    ajint maxpos = 0;
    float maximum;

    maximum = 0;
    for(i=start; i<=stopp; i++)
	if(parameter[i]>maximum)
	{
	    maxpos = i;
	    maximum = parameter[i];
	}

    return maxpos;
}




#define OVERLAPP 2
#define MAX_TM_LANGD 30
#define MIN_TM_LANGD 25




/* @funcstatic tmap_summa1 ****************************************************
**
** Sums the values for 'parameter' in span 'start' to 'stopp'
**
** @param [r] start [ajint] Undocumented
** @param [r] stopp [ajint] Undocumented
** @param [r] parameter [const float*] Undocumented
** @return [float] sum
** @@
******************************************************************************/

static float tmap_summa1(ajint start, ajint stopp, const float *parameter)
{
    float summa = 0;
    ajint i;

    for(i=start; i<=stopp; i++)
	summa += parameter[i];

    return summa;
}




/* @funcstatic tmap_tm_in_vector **********************************************
**
** Checks if segment already in TM vector
**
** @param [r] start [const ajint*] Undocumented
** @param [r] stopp [const ajint*] Undocumented
** @param [r] max [ajint] Undocumented
** @param [r] starttmp [ajint] Undocumented
** @param [r] stopptmp [ajint] Undocumented
** @return [ajint] Undocumented
** @@
******************************************************************************/

static ajint tmap_tm_in_vector(const ajint *start, const ajint *stopp,
			       ajint max,
			       ajint starttmp, ajint stopptmp)
{
    ajint i;
    ajint temp;

    temp = 0;
    for(i=1; i<=max; i++)
	if((abs(start[i]-starttmp)<7) && (abs(stopp[i]-stopptmp)<7))
	    temp = 1;

    return temp;
}




/* @funcstatic tmap_insert_in_vector ******************************************
**
** Insert segment in TM vector
**
** @param [w] start [ajint*] Undocumented
** @param [w] stopp [ajint*] Undocumented
** @param [r] max [ajint] Undocumented
** @param [r] starttmp [ajint] Undocumented
** @param [r] stopptmp [ajint] Undocumented
** @param [w] pred [ajint*] Undocumented
** @param [r] predparameter [ajint] Undocumented
** @return [ajint] Undocumented
** @@
******************************************************************************/

static ajint tmap_insert_in_vector(ajint *start, ajint *stopp, ajint max,
				   ajint starttmp, ajint stopptmp,
				   ajint *pred, ajint predparameter)
{
    ajint i;
    ajint j;

    for(i=1; i<=max-1; i++)
	if(starttmp>start[i])
	    if(starttmp<start[i+1])
	    {
		for(j=max; j>=i+1; j--)
		{
		    start[j+1]=start[j];
		    stopp[j+1]=stopp[j];
		    pred[j+1]=pred[j];
		}
		start[i+1]=starttmp;
		stopp[i+1]=stopptmp;
		pred[i+1]=predparameter;
		return max+1;
	    }

    /* starttmp<=start[i] */

    for(j=max; j>=1; j--)
    {
	start[j+1]=start[j];
	stopp[j+1]=stopp[j];
	pred[j+1]=pred[j];
    }
    start[1] = starttmp;
    stopp[1] = stopptmp;
    pred[1]  = predparameter;

    return max+1;
}




/* @funcstatic tmap_vec_to_stst ***********************************************
**
** Transfers information in vector vec[] to start[] and stopp[]
** Returns number of elements in start[] and stopp[]
**
** @param [w] vec [ajint*] Undocumented
** @param [w] start [ajint*] Undocumented
** @param [w] stopp [ajint*] Undocumented
** @param [r] length [ajint] Undocumented
** @return [ajint] Undocumented
** @@
******************************************************************************/

static ajint tmap_vec_to_stst(ajint *vec, ajint *start, ajint *stopp,
			      ajint length)
{
    ajint flagga;
    ajint i;
    ajint myindex;

    flagga=0;
    myindex=0;

    for(i=1; i<=length; i++)
    {
	if((vec[i]==1) && (flagga==0))
	{
	    flagga = 1;
	    start[++myindex]=i;
	}

	if((vec[i]==0) && (flagga==1))
	{
	    flagga = 0;
	    stopp[myindex]=i-1;
	}
    }

    if(flagga==1)
	stopp[myindex] = length;

    return myindex;
}




/* @funcstatic tmap_weights ***************************************************
**
** Calculates number of differences between sequence 'testnr' and all
**  other sequences (Ref. Vingron & Argos, CABIOS 5 (1989) 115-121).
**
** @param [r] sw [CONST char**] sekvensmatris (sequence matrix)
** @param [r] poss [ajint] antal positioner i sekvenserna
**                         (position in sequence)
** @param [r] nr [ajint] max nr av sekvenserna (max number of sequences)
** @param [w] norm_sk [float*] vektor for vikterna (vector)
** @@
******************************************************************************/

static void tmap_weights(char **sw, ajint poss, ajint nr,
			 float *norm_sk)
{
    ajint i;
    ajint j;
    ajint testnr;
    ajint *skillnad = NULL;
    float summa;

    AJCNEW0(skillnad, nr+1);
    for(testnr=0; testnr<=nr; testnr++)
	for(i=0; i<=nr; i++)
	    if(i!=testnr)
		for(j=1, skillnad[testnr]=0; j<=poss; j++)
		    if(sw[testnr][j]!=sw[i][j])
			skillnad[testnr]++;

    /* Normalize 'skillnad[]' */
    for(i=0, j=poss*nr; i<=nr; i++)
	if(skillnad[i]<j)
	    j = skillnad[i];
    for(i=0; i<=nr; i++)
	norm_sk[i] = (float)skillnad[i]/j;

    /* Satt  Summa av skillnad[] till 1 */

    for(i=0, summa=0; i<=nr; i++)
	summa += norm_sk[i];

    for(i=0; i<=nr; i++)
	norm_sk[i] = norm_sk[i]/summa;

    AJFREE(skillnad);

    return;
}




/* @funcstatic tmap_profile2 **************************************************
**
** Calculates mean values of 'P[]' over 'span' a.a.
** and stores the result in 'profile[]'.
**
** Calculates on all sequences of the alignment
** Ignores gaps
**
** @param [r] prof [ajint] number of profile
** @param [r] antal [ajint] number of sequences in alignment
** @param [r] poss [ajint] number of positions in sequence
** @param [r] span [ajint] length of span for profile to be calculated upon
** @@
******************************************************************************/

static void tmap_profile2(ajint prof, ajint antal, ajint poss, ajint span)
{
    ajint bin=0,count,i,j,nr;
    ajint *flagga = NULL;
    float prof_temp;
    float *summa_vikt = NULL;

    AJCNEW0(flagga, poss+1);
    AJCNEW0(summa_vikt, poss+1);

    for(i=1; i<=poss; i++)
    {
	profile[prof][i] = 0.0;
	flagga[i] = 0;
	summa_vikt[i] = 0;
    }

    for(nr=0; nr<=antal; nr++)
	for(i=1; i<=poss-span+1; i++)
	{
	    if((s[nr][i]>='A') && (s[nr][i]<='Z'))
	    {
		for(j=0, count=0, prof_temp=0; count<span && i+j<=poss; j++)
		    if((s[nr][i+j]>='A') && (s[nr][i+j]<='Z'))
		    {
			prof_temp+=P[prof][s[nr][i+j]-'A'] * norm_skillnad[nr];
			if(count==span/2)
			    bin = i+j;
			count++;
		    }

		if(count==span)
		{
		    flagga[bin] = 1;
		    profile[prof][bin] += prof_temp/count;
		    summa_vikt[bin] += norm_skillnad[nr];
		}
	    }
	}

    for(i=1; i<=poss; i++)
	if((flagga[i]==0) || (ali_ok[i]==0))
	    profile[prof][i] = NOLLVARDE;
	else
	    profile[prof][i] = profile[prof][i]/summa_vikt[i];

    AJFREE(flagga);
    AJFREE(summa_vikt);

    return;
}




/* @funcstatic tmap_align_rel *************************************************
**
** Undocumented.
**
** @param [r] antal [ajint] Undocumented
** @param [r] poss [ajint] Undocumented
** @param [r] span [ajint] Undocumented
** @@
******************************************************************************/

static void tmap_align_rel(ajint antal, ajint poss, ajint span)
{
    ajint nr;
    ajint ok;
    ajint pos;

    for(pos=1; pos<=poss-span+1; pos++)
    {
	for(nr=0,ok=0; nr<=antal; nr++)
	    if(s[nr][pos]!=GAP)
		ok++;

	if((float)ok/(float)nr > ALI_MINIMUM)
	    ali_ok[pos] = 1;
	else
	    ali_ok[pos] = 0;
    }

    return;
}




/* @funcstatic tmap_plot2 *****************************************************
**
** Undocumented.
**
** @param [w] mult [AjPGraph] Undocumented
** @@
******************************************************************************/

static void tmap_plot2(AjPGraph mult)
{
  AjPGraphdata graphdata = NULL;
  float max = -10.0;
  float min = 10.0;
  ajuint i;
  ajint j;

  for(j=0;j<pp_antal;j++)
  {
    for(i=1; i<=glposs; i++)
    {
      if(profile[j][i] != FLT_MIN)
      {
	if(profile[j][i] > max)
	  max = profile[j][i];
	if(profile[j][i] < min)
	  min = profile[j][i];
      }
    }
    graphdata = ajGraphdataNew();
    ajGraphdataSetTypeC(graphdata,"Overlay 2D Plot");

    ajGraphdataCalcXY(graphdata,glposs,0.0,1.0,&profile[j][0]);

    ajGraphdataSetTruescale(graphdata,0.,(float)glposs,min,max);

    ajGraphdataSetXlabelC(graphdata,"Residue no.");
    ajGraphdataSetYlabelC(graphdata,"");
    ajGraphdataSetTitleC(graphdata,"");
    ajGraphdataSetSubtitleC(graphdata,"");

    ajGraphDataAdd(mult,graphdata);
  }
  ajGraphdataSetLinetype(graphdata, 2);
  ajGraphxySetflagGaps(mult,AJTRUE);
  ajGraphxySetflagOverlay(mult,AJTRUE);


  if(min > 0.0)
    min = min*(float)0.9;
  else
    min = min*(float)1.1;

  max = max*(float)1.1;

  ajGraphxySetMinmax(mult,0.0,(float)glposs,min,max);
  ajGraphSetTitleC(mult,"Tmap");

  max = max * (float)0.95;

  for(j=1; j<=tm_number; j++)
    ajGraphAddRect(mult,(float)tm_segment[j][0],max,(float)tm_segment[j][1],
		      max+((max-min)*(float)0.01),BLACK,1);

  ajGraphxyDisplay(mult,AJFALSE);
  ajGraphicsClose();

  return;
}

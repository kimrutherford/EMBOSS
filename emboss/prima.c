/* @source prima.c
** @author Copyright (C) Sinead O'Leary (soleary@hgmp.mrc.ac.uk)
** @@
** Application for selecting forward and reverse primers for PCR and
** DNA amplification.
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
#include <limits.h>

#define SIMLIMIT 30
#define SIMLIMIT2 70

static float* entropy = NULL;
static float* enthalpy = NULL;
static float* energy = NULL;

/* @datastatic AjPPrimer ******************************************************
**
** Definition of the primer object
**
** @alias AjSPrimer
** @alias AjOPrimer
**
** @attr substr [AjPStr] Undocumented
** @attr start [ajint]  Undocumented
** @attr primerlen [ajint]  Undocumented
** @attr primerTm [float]  Undocumented
** @attr primGCcont [float]  Undocumented
** @attr prodTm [float]  Undocumented
** @attr prodGC [float]  Undocumented
** @attr score [ajint]  Undocumented
******************************************************************************/

typedef struct AjSPrimer
{
    AjPStr substr;
    ajint start;
    ajint primerlen;
    float primerTm;
    float primGCcont;
    float prodTm;
    float prodGC;
    ajint   score;
} AjOPrimer;
#define AjPPrimer AjOPrimer*




/* @datastatic AjPPair ********************************************************
**
** Object to hold awesome primer pairs
**
** @alias AjSPair
** @alias AjOPair
**
** @attr f [AjPPrimer] Forward primer
** @attr r [AjPPrimer] Reverse primer
******************************************************************************/

typedef struct AjSPair
{
    AjPPrimer f;
    AjPPrimer r;
} AjOPair;
#define AjPPair AjOPair*




static ajint prima_primalign(const AjPStr a, const AjPStr b);
static void prima_reject_self(AjPList forlist,AjPList revlist,
			      ajint *neric, ajint *nfred);
static void prima_best_primer(AjPList forlist, AjPList revlist,
			      ajint *neric, ajint *nfred);
static void prima_test_multi(AjPList forlist, AjPList revlist,
			     ajint *neric, ajint *nfred,
			     const AjPStr seq, const AjPStr rseq, ajint len);
static ajint prima_seq_align(const char *a, const char *b, ajint len);
static void  prima_PrimerDel(AjPPrimer *p);
static ajint prima_Compare(const void *a, const void *b);
static ajint prima_PosCompare(const void *a, const void *b);
static ajint prima_PosEndCompare(const void *a, const void *b);
/*static float prima_probAlign(AjPStr *seq1, AjPStr *seq2);*/
static void prima_prune_nearby(AjPList pairlist, ajint *npair, ajint range);
static void prima_check_overlap(AjPList pairlist, ajint *npair,
				ajint overlap);
static void prima_TwoSortscorepos(AjPList *pairlist);
static void prima_RevSort(AjPList *alist);


static void prima_testproduct(const AjPStr seqstr,
			      ajint startpos, ajint endpos,
			      ajint primerlen, ajint minprimerlen,
			      ajint maxprimerlen, float minpmGCcont,
			      float maxpmGCcont, ajint minprimerTm,
			      ajint maxprimerTm, ajint minprodlen,
			      ajint maxprodlen, float prodTm,
			      float prodGC, ajint seqlen, AjPPrimer *eric,
			      AjPPrimer *fred, AjPList forlist,
			      AjPList revlist, ajint *neric, ajint *nfred,
			      ajint stepping_value, float saltconc,
			      float dnaconc, AjBool isDNA, ajint begin);

static void prima_testtarget(const AjPStr seqstr, const AjPStr revstr,
			     ajint targetstart,
			     ajint targetend, ajint minprimerlen,
			     ajint maxprimerlen, ajint seqlen,
			     float minprimerTm, float maxprimerTm,
			     float minpmGCcont, float maxpmGCcont,
			     float minprodGCcont, float maxprodGCcont,
			     float saltconc, float dnaconc,
			     AjPList pairlist, ajint *npair);




/* @prog prima ****************************************************************
**
** Selects primers for PCR and DNA amplification
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPFile outf = NULL;

    AjPSeq sequence = NULL;
    AjPStr substr   = NULL;
    AjPStr seqstr = NULL;
    AjPStr revstr = NULL;

    AjPStr p1;
    AjPStr p2;

    AjPPrimer eric = NULL;
    AjPPrimer fred = NULL;

    AjPPrimer f;
    AjPPrimer r;

    AjPPair pair;

    AjPList forlist  = NULL;
    AjPList revlist  = NULL;
    AjPList pairlist = NULL;

    AjBool targetrange;
    AjBool isDNA  = ajTrue;
    AjBool dolist = ajFalse;

    ajint primerlen    = 0;
    ajint minprimerlen = 0;
    ajint maxprimerlen = 0;
    ajint minprodlen   = 0;
    ajint maxprodlen   = 0;
    ajint prodlen      = 0;

    ajint seqlen = 0;
    ajint stepping_value = 1;

    ajint targetstart = 0;
    ajint targetend   = 0;

    ajint limit    = 0;
    ajint limit2   = 0;
    ajint lastpos  = 0;
    ajint startpos = 0;
    ajint endpos   = 0;

    ajint begin;
    ajint end;
    ajint v1;
    ajint v2;

    ajint overlap;

    float minpmGCcont   = 0.;
    float maxpmGCcont   = 0.;
    float minprodGCcont = 0.;
    float maxprodGCcont = 0.;
    float prodTm;
    float prodGC;

    ajint i;
    ajint j;

    ajint neric=0;
    ajint nfred=0;
    ajint npair=0;

    float minprimerTm = 0.0;
    float maxprimerTm = 0.0;

    float saltconc = 0.0;
    float dnaconc  = 0.0;

    embInit ("prima", argc, argv);

    substr = ajStrNew();

    forlist  = ajListNew();
    revlist  = ajListNew();
    pairlist = ajListNew();

    p1 = ajStrNew();
    p2 = ajStrNew();


    sequence = ajAcdGetSeq("sequence");
    outf     = ajAcdGetOutfile("outfile");

    minprimerlen = ajAcdGetInt("minprimerlen");
    maxprimerlen = ajAcdGetInt("maxprimerlen");
    minpmGCcont  = ajAcdGetFloat("minpmGCcont");
    maxpmGCcont  = ajAcdGetFloat("maxpmGCcont");
    minprimerTm  = ajAcdGetFloat("mintmprimer");
    maxprimerTm  = ajAcdGetFloat("maxtmprimer");

    minprodlen    = ajAcdGetInt("minplen");
    maxprodlen    = ajAcdGetInt("maxplen");
    minprodGCcont = ajAcdGetFloat("minpgccont");
    maxprodGCcont = ajAcdGetFloat("maxpgccont");

    saltconc = ajAcdGetFloat("saltconc");
    dnaconc  = ajAcdGetFloat("dnaconc");

    targetrange = ajAcdGetToggle("targetrange");
    targetstart = ajAcdGetInt("targetstart");
    targetend   = ajAcdGetInt("targetend");

    overlap = ajAcdGetInt("overlap");
    dolist  = ajAcdGetBoolean("list");

    seqstr = ajSeqGetSeqCopyS(sequence);
    ajStrFmtUpper(&seqstr);

    begin  = ajSeqGetBegin(sequence);
    end    = ajSeqGetEnd(sequence);
    seqlen = end-begin+1;

    ajStrAssignSubC(&substr,ajStrGetPtr(seqstr),begin-1,end-1);
    revstr = ajStrNewC(ajStrGetPtr(substr));
    ajSeqstrReverse(&revstr);

    AJCNEW0(entropy, seqlen);
    AJCNEW0(enthalpy, seqlen);
    AJCNEW0(energy, seqlen);

    /* Initialise Tm calculation arrays */
    ajTm2(ajStrGetPtr(substr),0,seqlen,saltconc,dnaconc,1,
	  &entropy, &enthalpy, &energy);


    ajFmtPrintF(outf, "\n\nINPUT SUMMARY\n");
    ajFmtPrintF(outf, "*************\n\n");

    if(targetrange)
	ajFmtPrintF
	    (outf, "Prima of %s from positions %d to %d bps\n",
	     ajSeqGetNameC(sequence),targetstart, targetend);
    else
	ajFmtPrintF(outf, "Prima of %s\n", ajSeqGetNameC(sequence));

    ajFmtPrintF(outf, "PRIMER CONSTRAINTS:\n");
    ajFmtPrintF
	(outf, "PRIMA DOES NOT ALLOW PRIMER SEQUENCE AMBIGUITY OR ");
    ajFmtPrintF(outf,"DUPLICATE PRIMER ENDPOINTS\n");
    ajFmtPrintF(outf,
		"Primer size range is %d-%d\n",minprimerlen,maxprimerlen);
    ajFmtPrintF(outf,
		"Primer GC content range is %.2f-%.2f\n",minpmGCcont,
		maxpmGCcont);
    ajFmtPrintF(outf,"Primer melting Temp range is %.2f - %.2f C\n",
		minprimerTm, maxprimerTm);

    ajFmtPrintF (outf, "PRODUCT CONSTRAINTS:\n");

    ajFmtPrintF(outf,"Product GC content range is %.2f-%.2f\n",
		minprodGCcont, maxprodGCcont);

    ajFmtPrintF(outf, "Salt concentration is %.2f (mM)\n", saltconc);
    ajFmtPrintF(outf, "DNA concentration is %.2f (nM)\n", dnaconc);



    if(targetrange)
	ajFmtPrintF(outf, "Targeted range to amplify is from %d to %d\n",
		    targetstart,targetend);
    else
    {
	ajFmtPrintF(outf,"Considering all suitable Primer pairs with ");
	ajFmtPrintF(outf,"Product length ranges %d to %d\n\n\n", minprodlen,
		    maxprodlen);
    }


    ajFmtPrintF(outf, "\n\nPRIMER/PRODUCT PAIR CALCULATIONS & OUTPUT\n");
    ajFmtPrintF(outf, "*****************************************\n\n");


    if(seqlen-minprimerlen < 0)
	ajFatal("Sequence too short");

    if(targetrange)
    {
	ajStrAssignSubC(&p1,ajStrGetPtr(substr),targetstart-begin,targetend-begin);

	prodGC = ajMeltGC(substr,seqlen);
	prodTm = ajProdTm(prodGC,saltconc,seqlen);

	if(prodGC<minprodGCcont || prodGC>maxprodGCcont)
	{
	    ajFmtPrintF(outf,
			"Product GC content [%.2f] outside acceptable range\n",
			prodGC);
	    embExitBad();
	    return 0;
	}

	prima_testtarget(substr, revstr, targetstart-begin, targetend-begin,
			 minprimerlen, maxprimerlen,
			 seqlen, minprimerTm, maxprimerTm, minpmGCcont,
			 maxpmGCcont, minprodGCcont, maxprodGCcont, saltconc,
			 dnaconc, pairlist, &npair);
    }



    if(!targetrange)
    {

    limit   = seqlen-minprimerlen-minprodlen+1;
    lastpos = seqlen-minprodlen;
    limit2  = maxprodlen-minprodlen;

    /* Outer loop selects all possible product start points */
    for(i=minprimerlen; i<limit; ++i)
    {
	startpos = i;
	ajDebug("Position in sequence %d\n",startpos);
	endpos = i+minprodlen-1;
	/* Inner loop selects all possible product lengths  */
	for(j=0; j<limit2; ++j, ++endpos)
	{
	    if(endpos>lastpos)
		break;

	    v1 = endpos-startpos+1;
	    ajStrAssignSubC(&p1,ajStrGetPtr(substr),startpos,endpos);
	    prodGC = ajMeltGC(p1,v1);
	    prodTm = ajProdTm(prodGC,saltconc,v1);

	    if(prodGC<minprodGCcont || prodGC>maxprodGCcont)
		continue;

	    /* Only accept primers with acceptable Tm and GC */
	    neric = 0;
	    nfred = 0;
	    prima_testproduct(substr, startpos, endpos, primerlen,
			      minprimerlen, maxprimerlen,minpmGCcont,
			      maxpmGCcont, minprimerTm, maxprimerTm,
			      minprodlen, maxprodlen, prodTm, prodGC, seqlen,
			      &eric,&fred,forlist,revlist,&neric,&nfred,
			      stepping_value, saltconc,dnaconc, isDNA, begin);
	    if(!neric)
		continue;



	    /* Now reject those primers with self-complementarity */

	    prima_reject_self(forlist,revlist,&neric,&nfred);
	    if(!neric)
		continue;

	    /* Reject any primers that could bind elsewhere in the
               sequence */
	    prima_test_multi(forlist,revlist,&neric,&nfred,substr,revstr,
			     seqlen);



	    /* Now select the least complementary pair (if any) */
	    prima_best_primer(forlist, revlist, &neric, &nfred);
	    if(!neric)
		continue;

	    AJNEW(pair);
	    ajListPop(forlist,(void **)&f);
	    ajListPop(revlist,(void **)&r);
	    pair->f = f;
	    pair->r = r;
	    ++npair;
	    ajListPush(pairlist,(void *)pair);
	}
     }

  }


    if(!targetrange)
    {
	/* Get rid of primer pairs nearby the top scoring ones */
	prima_TwoSortscorepos(&pairlist);
	prima_prune_nearby(pairlist, &npair, maxprimerlen-1);
	ajListSort(pairlist,prima_PosCompare);
	prima_check_overlap(pairlist,&npair,overlap);
    }



    if(npair)
    {
	if(!targetrange)
	    ajFmtPrintF(outf,"%d pairs found\n\n",npair);
	else
	    ajFmtPrintF(outf,
			"Closest primer pair to specified product is:\n\n");

	if((maxprimerlen<26 && seqlen<999999 && !dolist))
	    ajFmtPrintF(outf,"\n\t\tForward\t\t\t\t\tReverse\n\n");
    }



    for(i=0;i<npair;++i)
    {
	if(!targetrange)
	    ajFmtPrintF(outf,"[%d]\n",i+1);

	ajListPop(pairlist,(void **)&pair);


	prodlen = pair->r->start - (pair->f->start + pair->f->primerlen);

	if((maxprimerlen<26 && seqlen<999999 && !dolist))
	{
	    v1 = pair->f->start;
	    v2 = v1 + pair->f->primerlen -1;

	    ajStrAssignSubS(&p1,substr,v1,v2);
	    ajFmtPrintF(outf,"%6d %-25.25s %d\t", v1+begin, ajStrGetPtr(p1),
			v2+begin);


	    v1 = pair->r->start;
	    v2 = v1 + pair->r->primerlen -1;
	    ajStrAssignSubS(&p2,substr,v1,v2);
	    ajSeqstrReverse(&p2);
	    ajFmtPrintF(outf,
			"%6d %-25.25s %d\n", v1+begin, ajStrGetPtr(p2), v2+begin);


	    ajFmtPrintF(outf,"       Tm  %.2f C  (GC %.2f%%)\t\t       ",
			pair->f->primerTm,pair->f->primGCcont*100.);
	    ajFmtPrintF(outf,"Tm  %.2f C  (GC %.2f%%)\n",
			pair->r->primerTm,pair->r->primGCcont*100.);

	    ajFmtPrintF(outf,"             Length: %-32dLength: %d\n",
			pair->f->primerlen,pair->r->primerlen);
	    ajFmtPrintF(outf,"             Tma:    %.2f C\t\t\t",
			ajAnneal(pair->f->primerTm,pair->f->prodTm));
	    ajFmtPrintF(outf,"     Tma:    %.2f C\n\n\n",
			ajAnneal(pair->r->primerTm,pair->f->prodTm));


	    ajFmtPrintF(outf,"       Product GC: %.2f%%\n",
			pair->f->prodGC * 100.0);
	    ajFmtPrintF(outf,"       Product Tm: %.2f C\n",
			pair->f->prodTm);
	    ajFmtPrintF(outf,"       Length:     %d\n\n\n",prodlen);
	}
	else
	{
	    ajFmtPrintF(outf,"    Product from %d to %d\n",pair->f->start+
			pair->f->primerlen+begin,pair->r->start-1+begin);
	    ajFmtPrintF(outf,"                 Tm: %.2f C   GC: %.2f%%\n",
			pair->f->prodTm,pair->f->prodGC*(float)100.);
	    ajFmtPrintF(outf,"                 Length: %d\n\n\n",prodlen);


	    v1 = pair->f->start;
	    v2 = v1 + pair->f->primerlen -1;
	    ajStrAssignSubS(&p1,substr,v1,v2);
	    ajFmtPrintF(outf,"    Forward: 5' %s 3'\n",ajStrGetPtr(p1));
	    ajFmtPrintF(outf,"             Start: %d\n",v1+begin);
	    ajFmtPrintF(outf,"             End:   %d\n",v2+begin);
	    ajFmtPrintF(outf,"             Tm:    %.2f C\n",
			pair->f->primerTm);
	    ajFmtPrintF(outf,"             GC:    %.2f%%\n",
			pair->f->primGCcont*(float)100.);
	    ajFmtPrintF(outf,"             Len:   %d\n",
			pair->f->primerlen);
	    ajFmtPrintF(outf,"             Tma:   %.2f C\n\n\n",
			ajAnneal(pair->f->primerTm,pair->f->prodTm));

	    v1 = pair->r->start;
	    v2 = v1 + pair->r->primerlen -1;
	    ajStrAssignSubS(&p2,substr,v1,v2);
	    ajSeqstrReverse(&p2);
	    ajStrAssignSubS(&p1,substr,v1,v2);
	    ajFmtPrintF(outf,"    Reverse: 5' %s 3'\n",ajStrGetPtr(p1));
	    ajFmtPrintF(outf,"             Start: %d\n",v1+begin);
	    ajFmtPrintF(outf,"             End:   %d\n",v2+begin);
	    ajFmtPrintF(outf,"             Tm:    %.2f C\n",
			pair->r->primerTm);
	    ajFmtPrintF(outf,"             GC:    %.2f%%\n",
			pair->r->primGCcont*(float)100.);
	    ajFmtPrintF(outf,"             Len:   %d\n",
			pair->r->primerlen);
	    ajFmtPrintF(outf,"             Tma:   %.2f C\n\n\n",
			ajAnneal(pair->r->primerTm,pair->f->prodTm));
	}

	prima_PrimerDel(&pair->f);
	prima_PrimerDel(&pair->r);
	AJFREE(pair);
    }



    ajStrDel(&seqstr);
    ajStrDel(&revstr);
    ajStrDel(&substr);
    ajStrDel(&p1);
    ajStrDel(&p2);

    ajListFree(&forlist);
    ajListFree(&revlist);
    ajListFree(&pairlist);

    ajFileClose(&outf);
    ajSeqDel(&sequence);

    AJFREE(entropy);
    AJFREE(enthalpy);
    AJFREE(energy);

    embExit();

    return 0;
}




/* @funcstatic prima_primalign ************************************************
**
** Align two sequences and return match percentage
**
** @param [r] a [const AjPStr] sequence a
** @param [r] b [const AjPStr] sequence b
** @return [ajint] percent match
** @@
******************************************************************************/

static ajint prima_primalign(const AjPStr a, const AjPStr b)
{
    ajint plen;
    ajint qlen;
    ajint limit;
    ajint i;
    ajint n = 0;
    ajint mm = 0;
    ajint j;
    const char *p;
    const char *q;
    ajint alen;
    ajint blen;

    alen = ajStrGetLen(a);
    blen = ajStrGetLen(b);


    if(alen > blen)
    {
        plen = alen;
	qlen = blen;
        p = ajStrGetPtr(a);
        q = ajStrGetPtr(b);
    }
    else
    {
        plen = blen;
        qlen = alen;
        p = ajStrGetPtr(b);
        q = ajStrGetPtr(a);
    }

    limit = plen-qlen+1;

    for(i=0; i<limit; ++i)
    {
        for(j=0; j<qlen; ++j)
	    if(p[j]==q[j])
		++n;

        mm = AJMAX(mm, n);
        ++p;
    }

    return (ajint)(((float)mm/(float)qlen)*100.0);
}




/* @funcstatic prima_testproduct **********************************************
**
** Undocumented.
**
** @param [r] seqstr [const AjPStr] Undocumented
** @param [r] startpos [ajint] Undocumented
** @param [r] endpos [ajint] Undocumented
** @param [r] primerlen [ajint] Undocumented
** @param [r] minprimerlen [ajint] Undocumented
** @param [r] maxprimerlen [ajint] Undocumented
** @param [r] minpmGCcont [float] Undocumented
** @param [r] maxpmGCcont [float] Undocumented
** @param [r] minprimerTm [ajint] Undocumented
** @param [r] maxprimerTm [ajint] Undocumented
** @param [r] minprodlen [ajint] Undocumented
** @param [r] maxprodlen [ajint] Undocumented
** @param [r] prodTm [float] Undocumented
** @param [r] prodGC [float] Undocumented
** @param [r] seqlen [ajint] Undocumented
** @param [w] eric [AjPPrimer*] Undocumented
** @param [w] fred [AjPPrimer*] Undocumented
** @param [u] forlist [AjPList] Undocumented
** @param [u] revlist [AjPList] Undocumented
** @param [w] neric [ajint*] Undocumented
** @param [w] nfred [ajint*] Undocumented
** @param [r] stepping_value [ajint] Undocumented
** @param [r] saltconc [float] Undocumented
** @param [r] dnaconc [float] Undocumented
** @param [r] isDNA [AjBool] Undocumented
** @param [r] begin [ajint] Undocumented
** @@
******************************************************************************/

static void prima_testproduct(const AjPStr seqstr,
			      ajint startpos, ajint endpos,
			      ajint primerlen, ajint minprimerlen,
			      ajint maxprimerlen, float minpmGCcont,
			      float maxpmGCcont, ajint minprimerTm,
			      ajint maxprimerTm, ajint minprodlen,
			      ajint maxprodlen, float prodTm, float prodGC,
			      ajint seqlen, AjPPrimer *eric,
			      AjPPrimer *fred, AjPList forlist,
			      AjPList revlist, ajint *neric,
			      ajint *nfred, ajint stepping_value,
			      float saltconc, float dnaconc,
			      AjBool isDNA, ajint begin)
{
    AjPStr substr = NULL;
    AjPPrimer rubbish = NULL;
    ajint forpstart;
    ajint forpend;
    ajint revpstart;
    ajint revpend;
    ajint i;
    ajint tnum = 0;
    ajint thisplen;

    float primerTm   = 0.0;
    float primGCcont = 0.0;

    (void) primerlen;
    (void) minprodlen;
    (void) maxprodlen;
    (void) stepping_value;

    forpend   = startpos -1;
    revpstart = endpos +1;

    tnum = maxprimerlen-minprimerlen+1;


    substr=ajStrNew();

    /* FORWARD PRIMERS */

    forpstart = forpend-minprimerlen+1;

    for(i=0; i<tnum; ++i,--forpstart)
    {
	if(forpstart<0)
	    break;

	ajStrAssignSubC(&substr,ajStrGetPtr(seqstr),forpstart,forpend);
	thisplen = minprimerlen + i;

	primerTm = ajTm2("",forpstart,thisplen, saltconc,
			 dnaconc, isDNA,
			 &entropy, &enthalpy, &energy);

	/* If temp out of range ignore rest of loop iteration */
	if(primerTm<minprimerTm || primerTm>maxprimerTm)
	    continue;

	primGCcont = ajMeltGC(substr, thisplen);

	/* If GC content out of range ignore rest of loop iteration */
	if(primGCcont<minpmGCcont || primGCcont>maxpmGCcont)
	    continue;

	/*
	**  This is a valid primer as far as Tm & GC is concerned
	**  so push it to the storage list
        */
	AJNEW0(*eric);

	(*eric)->substr     = ajStrNewC(ajStrGetPtr(substr));
	(*eric)->start      = forpstart+begin;
	(*eric)->primerlen  = thisplen;
	(*eric)->primerTm   = primerTm;
	(*eric)->primGCcont = primGCcont;
	(*eric)->prodTm     = prodTm;
	(*eric)->prodGC     = prodGC;
	ajListPush(forlist, (void*)*eric);
	(*neric)++;
    }

    if(!*neric)
    {
	ajStrDel(&substr);
	return;
    }

    /* REVERSE PRIMERS */
    revpend=revpstart + minprimerlen-1;
    for(i=0; i<tnum; ++i,++revpend)
    {
	if(revpend>seqlen)
	    break;

	ajStrAssignSubC(&substr,ajStrGetPtr(seqstr),revpstart,revpend);
	ajSeqstrReverse(&substr);

	thisplen = minprimerlen + i;

	primerTm = ajTm2("",revpstart,thisplen, saltconc,
			 dnaconc, isDNA,
			 &entropy, &enthalpy, &energy);
	/* If temp out of range ignore rest of loop iteration */
	if(primerTm<minprimerTm || primerTm>maxprimerTm)
	    continue;

	primGCcont = ajMeltGC(substr, thisplen);
	/* If GC content out of range ignore rest of loop iteration */
	if(primGCcont<minpmGCcont || primGCcont>maxpmGCcont)
	    continue;

	/*
	**  This is a valid primer as far as Tm & GC is concerned
	**  so push it to the reverse primer storage list
        */
	AJNEW0(*fred);
	(*fred)->substr     = ajStrNewC(ajStrGetPtr(substr));
	(*fred)->start      = revpstart+begin;
	(*fred)->primerlen  = thisplen;
	(*fred)->primerTm   = primerTm;
	(*fred)->primGCcont = primGCcont;
	ajListPush(revlist, (void*)*fred);
	(*nfred)++;
    }

    ajStrDel(&substr);


    if(!*nfred)
    {
	*neric = 0;
	while(ajListPop(forlist,(void**)&rubbish))
	    prima_PrimerDel(&rubbish);
    }

    return;
}




/* @funcstatic prima_reject_self **********************************************
**
** reject self complementary primers
**
** @param [u] forlist [AjPList] Undocumented
** @param [u] revlist [AjPList] Undocumented
** @param [u] neric [ajint*] Undocumented
** @param [u] nfred [ajint*] Undocumented
** @@
******************************************************************************/

static void prima_reject_self(AjPList forlist,AjPList revlist, ajint *neric,
			      ajint *nfred)
{
    ajint count;
    ajint j;
    ajint i;
    AjPPrimer tmp;

    ajint len;
    ajint cut;
    AjPStr str1;
    AjPStr str2;
    ajint x;


    str1 = ajStrNew();
    str2 = ajStrNew();

    /* deal with forwards */
    count = *neric;
    for(i=0;i<*neric;++i)
    {
	ajListPop(forlist,(void **)&tmp);
	len = tmp->primerlen;
	cut = (len/2)-1;
	ajStrAssignSubS(&str1,tmp->substr,0,cut);
	ajStrAssignSubS(&str2,tmp->substr,cut+1,len-1);
	x = prima_primalign(str1,str2);
	if(x<SIMLIMIT)
	    ajListPushAppend(forlist,(void *)tmp);
	else
	{
	    prima_PrimerDel(&tmp);
	    --count;
	}
    }
    *neric = count;

    if (!*neric)
    {
	ajStrDel(&str1);
	ajStrDel(&str2);
	while(ajListPop(revlist,(void**)&tmp))
	    prima_PrimerDel(&tmp);
	*nfred=0;
	return;
    }



    /****** reverses ********/

    count = *nfred;

    for(j=0; j<*nfred; ++j)
    {
	ajListPop(revlist,(void **)&tmp);
	len = tmp ->primerlen;
	cut = (len/2)-1;
	ajStrAssignSubS(&str1,tmp->substr,0,cut);
	ajStrAssignSubS(&str2,tmp->substr,cut+1,len-1);
	x = prima_primalign(str1,str2);

	if(x<SIMLIMIT)
	    ajListPushAppend(revlist,(void *)tmp);
	else
	{
	    --count;
	    prima_PrimerDel(&tmp);
	}
    }
    *nfred = count;

    if(!*nfred)
    {
	while(ajListPop(forlist,(void**)&tmp))
	    prima_PrimerDel(&tmp);
	*neric=0;
    }
    ajStrDel(&str1);
    ajStrDel(&str2);

    return;
}




/* @funcstatic prima_best_primer **********************************************
**
** BEST PRIMER FUNCTION
**
** @param [u] forlist [AjPList] Undocumented
** @param [u] revlist [AjPList] Undocumented
** @param [w] neric [ajint*] Undocumented
** @param [w] nfred [ajint*] Undocumented
** @@
******************************************************************************/

static void prima_best_primer(AjPList forlist, AjPList revlist,
			      ajint *neric, ajint *nfred)
{
    ajint bestf;
    ajint bestr;
    ajint lowx;
    ajint i;
    ajint j;
    ajint x;

    AjPPrimer temp;
    AjPPrimer temp2;

    AjPPrimer hitf;
    AjPPrimer hitr;

    AjBool good;


    lowx = INT_MAX;
    bestf = bestr = 0;


    /* First find the best primer (if any) */
    good = ajFalse;

    for(i=0;i<*neric; ++i)
    {
	ajListPop(forlist, (void**)&temp);

	for(j=0; j<*nfred; ++j)
	{
	    ajListPop(revlist, (void**)&temp2);

	    x=prima_primalign(temp->substr,temp2->substr);

	    if(x<=SIMLIMIT)
		good = ajTrue;

	    if (x < lowx)
	    {
		temp->score=x;
		bestf=i;
		bestr=j;
		lowx=x;
	    }
	    ajListPushAppend(revlist, (void *)temp2);

	}
	ajListPushAppend(forlist, (void *)temp);
    }


    if(!good)
    {
	while(ajListPop(forlist,(void **)&temp))
	    prima_PrimerDel(&temp);

	while(ajListPop(revlist,(void **)&temp))
	    prima_PrimerDel(&temp);
	*neric = 0;
	*nfred = 0;
	return;
    }


    /*
    ** Get the best fwd one in hitf, discard the rest
    * Discard ones before a hit
    */
    for(i=0;i<bestf;++i)
    {
	ajListPop(forlist,(void **)&temp);
	prima_PrimerDel(&temp);
    }

    /* Next on the list is a hit */
    ajListPop(forlist,(void **)&hitf);

    /* Get rid of anything left on the list */
    for(i++;i<*neric;++i)
    {
	ajListPop(forlist,(void **)&temp);
	prima_PrimerDel(&temp);
    }


    /* Get the best rev one in hitr, discard the rest */
    for(i=0;i<bestr;++i)
    {
	ajListPop(revlist,(void **)&temp);
	prima_PrimerDel(&temp);
    }
    ajListPop(revlist,(void **)&hitr);

    for(i++;i<*nfred;++i)
    {
	ajListPop(revlist,(void **)&temp);
	prima_PrimerDel(&temp);
    }

    ajListPushAppend(forlist,(void *)hitf);
    ajListPushAppend(revlist,(void *)hitr);

    *neric = 1;
    *nfred = 1;

    return;
}




/* @funcstatic prima_PrimerDel ************************************************
**
** Free memory from primers
**
** @param [w] p [AjPPrimer*] Undocumented
** @@
******************************************************************************/

static void prima_PrimerDel(AjPPrimer *p)
{
    ajStrDel(&((*p)->substr));
    AJFREE(*p);

    return;
}




/* @funcstatic prima_Compare ********************************************
**
** Undocumented.
**
** @param [r] a [const void*] Undocumented
** @param [r] b [const void*] Undocumented
** @return [ajint] Undocumented
** @@
******************************************************************************/

static ajint prima_Compare(const void *a, const void *b)
{
    return (*(AjPPair const *)a)->f->score -
		   (*(AjPPair const *)b)->f->score;
}




/* @funcstatic prima_PosCompare ***********************************************
**
** Undocumented.
**
** @param [r] a [const void*] Undocumented
** @param [r] b [const void*] Undocumented
** @return [ajint] Undocumented
** @@
******************************************************************************/

static ajint prima_PosCompare(const void *a, const void *b)
{

    return ((*(AjPPair const *)a)->f->start + (*(AjPPair const *)a)->f->primerlen - 1)  -
	   ((*(AjPPair const *)b)->f->start + (*(AjPPair const *)b)->f->primerlen - 1);
}




/* @funcstatic prima_PosEndCompare ********************************************
**
** Undocumented.
**
** @param [r] a [const void*] Undocumented
** @param [r] b [const void*] Undocumented
** @return [ajint] Undocumented
** @@
******************************************************************************/

static ajint prima_PosEndCompare(const void *a, const void *b)
{

    return ((*(AjPPair const *)a)->r->start)  -
	   ((*(AjPPair const *)b)->r->start);
}




/* @funcstatic prima_testtarget ***********************************************
**
** Undocumented.
**
** @param [r] seqstr [const AjPStr] Undocumented
** @param [r] revstr [const AjPStr] Undocumented
** @param [r] targetstart [ajint] Undocumented
** @param [r] targetend [ajint] Undocumented
** @param [r] minprimerlen [ajint] Undocumented
** @param [r] maxprimerlen [ajint] Undocumented
** @param [r] seqlen [ajint] Undocumented
** @param [r] minprimerTm [float] Undocumented
** @param [r] maxprimerTm [float] Undocumented
** @param [r] minpmGCcont [float] Undocumented
** @param [r] maxpmGCcont [float] Undocumented
** @param [r] minprodGCcont [float] Undocumented
** @param [r] maxprodGCcont [float] Undocumented
** @param [r] saltconc [float] Undocumented
** @param [r] dnaconc [float] Undocumented
** @param [u] pairlist [AjPList] Undocumented
** @param [w] npair [ajint*] Undocumented
** @@
******************************************************************************/

static void prima_testtarget(const AjPStr seqstr, const AjPStr revstr,
			     ajint targetstart,
			     ajint targetend, ajint minprimerlen,
			     ajint maxprimerlen, ajint seqlen,
			     float minprimerTm, float maxprimerTm,
			     float minpmGCcont, float maxpmGCcont,
			     float minprodGCcont, float maxprodGCcont,
			     float saltconc, float dnaconc,
			     AjPList pairlist, ajint *npair)
{


    AjPStr fstr;
    AjPStr rstr;

    AjPStr str1;
    AjPStr str2;
    AjPPrimer f;
    AjPPrimer r;

    AjPPair ppair;

    ajint i;
    ajint j;
    ajint forstart = 0;
    ajint forend;
    ajint revstart = 0;
    ajint revend;
    ajint Limit;
    ajint tnum;
    ajint thisplen;
    ajint cut;

    float primerTm = 0.0;
    float primGCcont = 0.0;
    float prodgc = 0.0;

    AjBool found = ajFalse;
    AjBool revfound = ajFalse;
    AjBool isDNA = ajTrue;

    ajint flen = 0;
    ajint rlen = 0;

    float ftm = 0.0;
    float rtm = 0.0;
    float fgc = 0.0;
    float rgc = 0.0;
    ajint fsc = 0;
    ajint rsc = 0;

    const char *s;
    const char *s2;
    const char *p;
    ajint  pv;
    ajint  plimit;
    ajint  pcount;
    ajint  k;

    (void) minprodGCcont;
    (void) maxprodGCcont;

    fstr = ajStrNew();
    rstr = ajStrNew();
    str1 = ajStrNew();
    str2 = ajStrNew();



    tnum=maxprimerlen-minprimerlen+1;

    /******FORWARDS  *******/

    for(i=targetstart-minprimerlen; i>-1; --i)
    {
	forstart = i;
	forend = i+minprimerlen-1;


	for(j=0; j<tnum; ++j,++forend)
	{
	    if(forend==targetstart)
		break;

	    ajStrAssignSubC(&fstr, ajStrGetPtr(seqstr), forstart, forend);

	    thisplen = ajStrGetLen(fstr);
	    primerTm =ajTm2("",forstart,thisplen, saltconc, dnaconc, isDNA,
			    &entropy, &enthalpy, &energy);

	    if(primerTm <minprimerTm || primerTm>maxprimerTm)
		continue;

	    primGCcont= ajMeltGC(fstr, thisplen);
	    if(primGCcont< minpmGCcont || primGCcont >maxpmGCcont)
		continue;


	    /*instead of calling the self-reject function */
	    cut = (thisplen/2)-1;

	    ajStrAssignSubS(&str1, fstr, 0, cut);
	    ajStrAssignSubS(&str2, fstr, cut+1, thisplen-1);

	    if((fsc=prima_primalign(str1, str2)) > SIMLIMIT)
		continue;

	    /* Test for match with rest of sequence */
	    s  = ajStrGetPtr(seqstr);
	    s2 = ajStrGetPtr(revstr);
	    p  = ajStrGetPtr(fstr);
	    pv = thisplen;
	    pcount = 0;
	    plimit = seqlen-pv+1;
	    for(k=0;k<plimit && pcount<2;++k)
	    {
		if(prima_seq_align(s+k,p,pv)>SIMLIMIT2)
		    ++pcount;
		if(prima_seq_align(s2+k,p,pv)>SIMLIMIT2)
		    ++pcount;
	    }

	    if(pcount<2)
	    {
		found = ajTrue;
		flen  = thisplen;
		ftm   = primerTm;
		fgc   = primGCcont;
		break;
	    }
	}

	if(found)
	    break;
    }



    /******* REVERSES IN TARGETRANGE *****/


    Limit = seqlen-minprimerlen;

    if(found)
	for(i=targetend+1; i<Limit; ++i)
	{
	    revstart = i;
	    revend = i+minprimerlen-1;

	    for(j=0; j<tnum; ++j,++revend)
	    {
		if(revend==seqlen)
		    break;

		ajStrAssignSubC(&rstr, ajStrGetPtr(seqstr), revstart, revend);
		ajSeqstrReverse(&rstr);

		thisplen = ajStrGetLen(rstr);
		primerTm = ajTm2("", revstart, thisplen, saltconc, dnaconc, 1,
				 &entropy, &enthalpy, &energy);

		if(primerTm <minprimerTm || primerTm>maxprimerTm)
		    continue;

		primGCcont= ajMeltGC(rstr, thisplen);
		if(primGCcont< minpmGCcont || primGCcont >maxpmGCcont)
		    continue;

		/*instead of calling the self-reject function */
		cut = (thisplen/2)-1;

		ajStrAssignSubS(&str1, rstr, 0, cut);
		ajStrAssignSubS(&str2, rstr, cut+1, thisplen-1);

		if((rsc=prima_primalign(str1, str2)) < SIMLIMIT)
		    continue;

		/* Test for match with rest of sequence */
		s  = ajStrGetPtr(seqstr);
		s2 = ajStrGetPtr(revstr);
		p  = ajStrGetPtr(rstr);
		pv = thisplen;
		pcount = 0;
		plimit = seqlen-pv+1;
		for(k=0;k<plimit && pcount<2;++k)
		{
		    if(prima_seq_align(s+k,p,pv)>SIMLIMIT2)
			++pcount;

		    if(prima_seq_align(s2+k,p,pv)>SIMLIMIT2)
			++pcount;
		}

		if(pcount<2)
		{
		    revfound = ajTrue;
		    rlen     = thisplen;
		    rtm      = primerTm;
		    rgc      = primGCcont;
		    break;
		}
	    }

	    if(revfound)
		break;
	}


    if(found && !revfound)
    {
	found = ajFalse;
	ajWarn("No reverse primers found in targetrange");
	*npair = 0;
	return;
    }



    if(!found)
    {
	ajWarn("No forward primers found in targetrange");
	*npair = 0;
	return;
    }

    ajStrAssignSubC(&str1,ajStrGetPtr(seqstr),forstart+flen,revstart-1);
    prodgc = ajMeltGC(str1,revstart-(forstart+flen));



    AJNEW0(f);
    f->substr     = ajStrNewC(ajStrGetPtr(fstr));
    f->start      = forstart;
    f->primerlen  = flen;
    f->primerTm   = ftm;
    f->primGCcont = fgc;
    f->score      = fsc;
    f->prodGC     = prodgc;
    f->prodTm     = ajProdTm(prodgc,saltconc,revstart-(forstart+flen));


    AJNEW0(r);
    r->substr     = ajStrNewC(ajStrGetPtr(rstr));
    r->start      = revstart;
    r->primerlen  = rlen;
    r->primerTm   = rtm;
    r->primGCcont = rgc;
    r->score      = rsc;


    AJNEW0(ppair);
    ppair->f = f;
    ppair->r = r;
    ajListPush(pairlist,(void *)ppair);
    *npair = 1;

    return;
}




/* @funcstatic prima_test_multi ***********************************************
**
** Undocumented.
**
** @param [u] forlist [AjPList] Undocumented
** @param [u] revlist [AjPList] Undocumented
** @param [w] neric [ajint*] Undocumented
** @param [w] nfred [ajint*] Undocumented
** @param [r] seq [const AjPStr] Undocumented
** @param [r] rseq [const AjPStr] Undocumented
** @param [r] len [ajint] Undocumented
** @@
******************************************************************************/


static void prima_test_multi(AjPList forlist, AjPList revlist, ajint *neric,
			     ajint *nfred, const AjPStr seq,
			     const  AjPStr rseq, ajint len)
{
    AjPPrimer tmp;
    AjPStr st;

    ajint i;
    ajint j;
    ajint v;
    ajint pc;
    ajint count;
    ajint limit;

    const char *s;
    const char *r;
    const char *p;

    st = ajStrNew();

    s = ajStrGetPtr(seq);
    r = ajStrGetPtr(rseq);

    pc = *neric;

    for(i=0;i<*neric;++i)
    {
	ajListPop(forlist,(void **)&tmp);
	count = 0;
	v = tmp->primerlen;
	limit = len-v+1;
	p = ajStrGetPtr(tmp->substr);
	for(j=0;j<limit && count<2;++j)
	{
	    if(prima_seq_align(s+j,p,v)>SIMLIMIT2)
		++count;

	    if(prima_seq_align(r+j,p,v)>SIMLIMIT2)
		++count;
	}


	if(count>1)
	{
	    prima_PrimerDel(&tmp);
	    --pc;
	}
	else
	    ajListPushAppend(forlist,(void *)tmp);
    }

    *neric = pc;
    if(!*neric)
    {
	while(ajListPop(revlist,(void **)&tmp))
	    prima_PrimerDel(&tmp);
	*nfred = 0;
	ajStrDel(&st);
	return;
    }


    pc = *nfred;
    for(i=0;i<*nfred;++i)
    {
	ajListPop(revlist,(void **)&tmp);
	count = 0;
	v = tmp->primerlen;
	limit = len-v+1;
	ajStrAssignC(&st,ajStrGetPtr(tmp->substr));
	ajSeqstrReverse(&st);
	p = ajStrGetPtr(st);
	for(j=0;j<limit && count<2;++j)
	{
	    if(prima_seq_align(s+j,p,v)>SIMLIMIT2)
		++count;

	    if(prima_seq_align(r+j,p,v)>SIMLIMIT2)
		++count;
	}


	if(count>1)
	{
	    prima_PrimerDel(&tmp);
	    --pc;
	}
	else
	    ajListPushAppend(revlist,(void *)tmp);
    }

    *nfred = pc;
    if(!*nfred)
    {
	while(ajListPop(forlist,(void **)&tmp))
	    prima_PrimerDel(&tmp);
	*neric = 0;
    }

    ajStrDel(&st);

    return;
}




/* @funcstatic prima_seq_align ************************************************
**
** Undocumented.
**
** @param [r] a [const char*] Undocumented
** @param [r] b [const char*] Undocumented
** @param [r] len [ajint] Undocumented
** @return [ajint] Undocumented
** @@
******************************************************************************/

static ajint prima_seq_align(const char *a, const char *b, ajint len)
{
    ajint i;
    ajint count;

    count = 0;
    for(i=0;i<len;++i)
	if(a[i]==b[i])
	    ++count;

    return (ajint)(((float)count/(float)len)*(float)100.0);
}




/* @funcstatic prima_prune_nearby *********************************************
**
** Undocumented.
**
** @param [u] pairlist [AjPList] Undocumented
** @param [w] npair [ajint*] Undocumented
** @param [r] range [ajint] Undocumented
** @@
******************************************************************************/

static void prima_prune_nearby(AjPList pairlist, ajint *npair, ajint range)
{
    AjPPair pair;

    ajint count;
    ajint fst;
    ajint fst2;
    ajint blim;
    ajint blim2;
    ajint elim;
    ajint elim2;
    ajint i;
    ajint j;
    ajint len;

    for(i=0;i<*npair;++i)
    {
	for(j=0;j<i;++j)	/* Ignore those already processed */
	{
	    ajListPop(pairlist,(void **)&pair);
	    ajListPushAppend(pairlist,(void *)pair);
	}

	ajListPop(pairlist,(void **)&pair);/* Get next high scoring pair */
	len  = pair->f->primerlen;
	fst  = pair->f->start + len -1;
	blim = fst-range;
	elim = fst+range;

	len   = pair->r->primerlen;
	blim2 = pair->r->start - range;
	elim2 = pair->r->start + range;

	ajListPushAppend(pairlist,(void *)pair);
	count = *npair;
	for(j=i+1;j<*npair;++j)
	{
	    ajListPop(pairlist,(void **)&pair);
	    fst2=pair->f->start+pair->f->primerlen-1;
	    if((fst2<blim || fst2>elim) && (pair->r->start<blim2 ||
					    pair->r->start>elim2))
		ajListPushAppend(pairlist,(void *)pair);
	    else
	    {
		prima_PrimerDel(&pair->f);
		prima_PrimerDel(&pair->r);
		AJFREE(pair);
		--count;
	    }
	}

	*npair = count;
    }

    return;
}




/* @funcstatic prima_check_overlap ********************************************
**
** Undocumented.
**
** @param [u] pairlist [AjPList] Undocumented
** @param [w] npair [ajint*] Undocumented
** @param [r] overlap [ajint] Undocumented
** @@
******************************************************************************/

static void prima_check_overlap(AjPList pairlist, ajint *npair, ajint overlap)
{
    AjPPair pair;

    ajint i;
    ajint j;
    ajint end;
    ajint limit;
    ajint count;

    for(i=0;i<*npair;++i)
    {
	for(j=0;j<i;++j)
	{
	    ajListPop(pairlist,(void **)&pair);
	    ajListPushAppend(pairlist,(void *)pair);
	}

	ajListPop(pairlist,(void **)&pair);

	end   = pair->r->start;
	limit = end-overlap;
	ajListPushAppend(pairlist,(void *)pair);

	count = *npair;
	for(j=i+1;j<*npair;++j)
	{
	    ajListPop(pairlist,(void **)&pair);
	    if(pair->f->start+pair->f->primerlen-1 < limit)
	    {
		prima_PrimerDel(&pair->f);
		prima_PrimerDel(&pair->r);
		AJFREE(pair);
		--count;
	    }
	    else
		ajListPushAppend(pairlist,(void *)pair);
	}

	*npair = count;
    }

    return;
}




/* @funcstatic prima_TwoSortscorepos ******************************************
**
** Sort on basis of score then, within that block on the basis of
** product start then within that block on the basis of product end
** This requires the two functions TwoSortscorepos for the double
** double sort and RevSort, called within that, to sort on the
** primer end position
**
** @param [w] pairlist [AjPList*] Original list - replaced by a new list
** @@
******************************************************************************/

static void prima_TwoSortscorepos(AjPList *pairlist)
{
    AjPPair tmp = NULL;
    AjPList intlist = NULL;
    AjPList filist  = NULL;
    AjPPair save  = NULL;
    float   score = 0.0;


    ajListSort(*pairlist,prima_Compare);
    intlist = ajListNew();
    filist  = ajListNew();

    score = (float) -1.0;

    while(ajListPop(*pairlist,(void **)&tmp))
    {
	if(tmp->f->score == score)
	{
	    ajListPush(intlist,(void *)tmp);
	    continue;
	}

	save = tmp;
	ajListSort(intlist,prima_PosCompare);
	score = tmp->f->score;
	prima_RevSort(&intlist);

	while(ajListPop(intlist,(void **)&tmp))
	    ajListPushAppend(filist,(void *)tmp);
	ajListPush(intlist,(void *)save);
    }

    ajListSort(intlist,prima_PosCompare);
    prima_RevSort(&intlist);

    while(ajListPop(intlist,(void **)&tmp))
	ajListPushAppend(filist,(void *)tmp);

    ajListFree(&intlist);
    ajListFree(pairlist);

    *pairlist = filist;
    return;
}




/* @funcstatic prima_RevSort **************************************************
**
** See TwoSortscorepos
**
** @param [u] alist [AjPList*] Undocumented
** @@
******************************************************************************/

static void prima_RevSort(AjPList *alist)
{
    AjPPair tmp = NULL;
    AjPList intlist = NULL;
    AjPList filist  = NULL;
    AjPPair save = NULL;
    ajint pos = -1;


    intlist = ajListNew();
    filist  = ajListNew();

    pos = -1;

    while(ajListPop(*alist,(void **)&tmp))
    {
	if(tmp->f->start+tmp->f->primerlen == pos)
	{
	    ajListPush(intlist,(void *)tmp);
	    continue;
	}

	save = tmp;
	ajListSort(intlist,prima_PosEndCompare);
	pos = tmp->f->start+tmp->f->primerlen;
	while(ajListPop(intlist,(void **)&tmp))
	    ajListPushAppend(filist,(void *)tmp);
	ajListPush(intlist,(void *)save);
    }

    ajListSort(intlist,prima_PosEndCompare);
    while(ajListPop(intlist,(void **)&tmp))
	ajListPushAppend(filist,(void *)tmp);

    ajListFree(&intlist);
    ajListFree(alist);

    *alist = filist;

    return;
}

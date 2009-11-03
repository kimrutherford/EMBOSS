/* @source etandem application
**
** Tandem searches for tandem repeats
** @author Copyright (C) Richard Durbin (rd@sanger.ac.uk)
** and Jean Thierry-Mieg 1992
** @@
** The original application is part of the ACEDB genome database
** package, written by ** Richard Durbin (MRC LMB, UK)
** rd@mrc-lmba.cam.ac.uk, and Jean Thierry-Mieg (CRBM du CNRS,
** France) mieg@crbm1.cnusc.fr
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




#define DEBUG 0

#define NMAX 1000




/* @datastatic EtandemPCons ***************************************************
**
** Consensus pattern structure
**
** @alias EtandemSCons
** @alias EtandemOCons
**
** @attr tab [ajint*] Undocumented
** @attr max [ajint*] Undocumented
** @attr start [ajint] Undocumented
** @attr score [ajint] Undocumented
** @attr bestScore [ajint] Undocumented
** @attr ibest [ajint] Undocumented
** @attr bestMax [ajint*] Undocumented
** @attr phase [ajint] Undocumented
** @attr repeat [ajint] Undocumented
** @attr next [struct EtandemSCons*] Next node in linked list
** @@
******************************************************************************/

typedef struct EtandemSCons
{
    ajint* tab ;
    ajint* max ;
    ajint start ;
    ajint score ;
    ajint bestScore ;
    ajint ibest ;
    ajint* bestMax ;
    ajint phase ;
    ajint repeat ;
    struct EtandemSCons* next ;
} EtandemOCons, *EtandemPCons ;




static EtandemOCons rootStruct ;
static EtandemPCons root = &rootStruct ;
static ajint *ring ;
static char letter[5]  = "acgtn" ;
static AjBool mismatch = AJFALSE ;
static AjBool uniform  = AJFALSE ;
static ajint thresh = 20 ;
static ajint nbase;
static ajint nmin;
static ajint nmax;
static AjPSeqCvt cvt;

static EtandemPCons etandem_consCreate(void);
static void etandem_consDestroy(EtandemPCons *cons);
static void etandem_basicReport(AjPFeattable tab, AjPFile outfile,
				const EtandemPCons a);
static void etandem_report(EtandemPCons *a);
static void etandem_finalReport(AjPFeattable tab, AjPFile outfile);

#define ATAB(x,y) (a->tab[x+5*y])

static ajint nCons = 0 ;




/* @funcstatic etandem_consCreate *********************************************
**
** Undocumented.
**
** @return [EtandemPCons] Undocumented
** @@
******************************************************************************/

static EtandemPCons etandem_consCreate(void)
{
    static EtandemPCons res;

    AJNEW0(res);
    AJCNEW0(res->max, nmax+1);
    AJCNEW0(res->bestMax, nmax+1);
    AJCNEW0(res->tab, 5*nmax+5);
    ++nCons;

    return res;
}




/* @funcstatic etandem_consDestroy ********************************************
**
** Undocumented.
**
** @param [d] cons [EtandemPCons*] Undocumented
** @@
******************************************************************************/

static void etandem_consDestroy(EtandemPCons *cons)
{

    if(!*cons)
	return;
    --nCons;
    AJFREE((*cons)->max);
    AJFREE((*cons)->bestMax);
    AJFREE((*cons)->tab);
    AJFREE(*cons);

    return;
}




/***************** reporting code *****************/

static EtandemOCons reportRootStruct;
static EtandemPCons reportRoot = &reportRootStruct;




/* @funcstatic etandem_basicReport ********************************************
**
** Undocumented.
**
** @param [u] tab [AjPFeattable] Feature table
** @param [u] outfile [AjPFile] Output file (null unless original output
**                              is needed)
** @param [r] a [const EtandemPCons] Undocumented
** @@
******************************************************************************/

static void etandem_basicReport(AjPFeattable tab, AjPFile outfile,
				const EtandemPCons a)
{
    ajint j;
    ajint copies;
    ajint n;
    float perc;
    AjPStr constr = NULL;
    AjPStr rpthit = NULL;
    AjPStr s = NULL;
    AjPFeature gf;

    ajDebug("basicReport\n");

    n = a->repeat;

    if(!rpthit)
      ajStrAssignC(&rpthit, "repeat_region");

    copies = (a->ibest - a->start + 1) / n;
    perc = (float)100.0 * (a->bestScore + n * (copies + 1)) /
	((float)2.0 * n * copies);
    if(outfile)
      ajFmtPrintF(outfile, "%6d %10d %10d %2d %3d %5.1f ",
		  a->bestScore, a->start+1, a->ibest+1,
		  n, copies, perc);

    gf = ajFeatNew(tab, NULL, rpthit,
		   a->start+1, a->ibest+1,
		   (float) a->bestScore, '+', 0);
    ajFeatTagAddCC(gf, "rpt_type", "TANDEM");
    ajFmtPrintS(&s, "*rpt_size %d", n);
    ajFeatTagAdd(gf, NULL, s);
    ajFmtPrintS(&s, "*rpt_count %d", copies);
    ajFeatTagAdd(gf, NULL, s);
    ajFmtPrintS(&s, "*identity %.1f", perc);
    ajFeatTagAdd(gf, NULL, s);

    /* make the consensus */

    for(j = (a->phase+1) % n; j < n; ++j)
    {
	/*ajDebug("      bestMax[%d] letter[%d] '%c'\n",
		j, a->bestMax[j], letter[a->bestMax[j]]);*/
	ajStrAppendK(&constr, letter[a->bestMax[j]]);
    }

    if((a->phase+1) % n)
	for(j = 0; j <= a->phase; ++j)
	{
	    /*ajDebug("more: bestMax[%d] letter[%d] '%c'\n",
		    j, a->bestMax[j], letter[a->bestMax[j]]);*/
	    ajStrAppendK(&constr, letter[a->bestMax[j]]);
	}

    ajFmtPrintS(&s, "*consensus %S", constr);
    ajFeatTagAdd(gf, NULL, s);

    if(outfile)
      ajFmtPrintF(outfile, "%S\n", constr);

    ajStrDel(&rpthit);
    ajStrDel(&s);
    ajStrDel(&constr);

    return;
}




/* @funcstatic etandem_report *************************************************
**
** Undocumented.
**
** @param [d] a [EtandemPCons*] Undocumented
** @@
******************************************************************************/

static void etandem_report(EtandemPCons *a)
{
    ajint j;
    ajint firstchar;

    if((*a)->bestScore >= thresh)
    {
	if(uniform)
	    goto good;

	/* else check not a single letter pattern */
	firstchar = (*a)->bestMax[0];

	for(j = 1; j < (*a)->repeat; j++)
	    if((*a)->bestMax[j] != firstchar)
		goto good;
    }

    etandem_consDestroy(a);	/* don't destroy, since reporting repeatedly */
    return;

 good:
    (*a)->next = reportRoot->next;
    reportRoot->next = *a;

    return;
}




/* @funcstatic etandem_finalReport ********************************************
**
** Undocumented.
**
** @param [u] tab [AjPFeattable] Feature table
** @param [u] outfile [AjPFile] Output file (null unless original output
**                              is needed)
** @@
******************************************************************************/

static void etandem_finalReport(AjPFeattable tab, AjPFile outfile)
{
    ajint start;
    ajint end;
    EtandemPCons a;
    EtandemPCons top;
    EtandemPCons olda;

    ajDebug("finalReport\n");
    while(reportRoot->next)
    {					/* find top score */
	top = reportRoot;
	for(a = reportRoot->next; a; a = a->next)
	    if(a->bestScore > top->bestScore ||
		(a->bestScore == top->bestScore && a->repeat < top->repeat))
		top = a;

	/* report that */
	etandem_basicReport(tab, outfile, top);
	/* destroy all overlapping entries, including self  */
	start = top->start;
	end = top->ibest;
	olda = reportRoot;
	for(a = olda->next; a; olda = a, a = a->next)
	    if(a->ibest >= start && a->start <= end)
	    {
		olda->next = a->next;
		etandem_consDestroy(&a);
		a = olda;
	    }
    }

    return;
}




/* @prog etandem **************************************************************
**
** Looks for tandem repeats in a nucleotide sequence
**
******************************************************************************/

int main(int argc, char **argv)
{

    ajint ibase;
    ajint base;
    const char *cp;
    AjPSeq sequence = NULL;
    ajint i;
    ajint j;
    ajint x;
    ajint phase;
    ajint n;
    EtandemPCons new;
    EtandemPCons a;
    EtandemPCons b;
    EtandemPCons olda;
    EtandemPCons oldb;
    AjPStr nseq = NULL;
    AjPFeattable tab = NULL;
    AjPReport report = NULL;
    AjPFile outfile  = NULL;
    AjPStr tmpstr    = NULL;

    embInit("etandem", argc, argv);

    nmin     = ajAcdGetInt("minrepeat");
    nmax     = ajAcdGetInt("maxrepeat");
    mismatch = ajAcdGetBool("mismatch");
    thresh   = ajAcdGetInt("threshold");
    uniform  = ajAcdGetBool("uniform");
    report   = ajAcdGetReport("outfile");
    outfile  = ajAcdGetOutfile("origfile");
    sequence = ajAcdGetSeq("sequence");
    nbase    = ajSeqGetLen(sequence);

    tab = ajFeattableNewSeq(sequence);

    ajFmtPrintAppS(&tmpstr, "Threshold: %d\n", thresh);
    ajFmtPrintAppS(&tmpstr, "Minrepeat: %d\n", nmin);
    ajFmtPrintAppS(&tmpstr, "Maxrepeat: %d\n", nmax);
    ajFmtPrintAppS(&tmpstr, "Mismatch: %B\n", mismatch);
    ajFmtPrintAppS(&tmpstr, "Uniform: %B\n", uniform);
    ajReportSetHeader(report, tmpstr);

    cvt = ajSeqcvtNewC("ACGTN");
    ajSeqConvertNum(sequence, cvt, &nseq);

    AJCNEW(ring, nbase);

    reportRoot->bestScore = thresh - 1;

    for(n = nmin; n <= nmax; ++n)
    {
	cp = ajStrGetPtr(nseq);
	for(ibase = 0; ibase < nbase; ++ibase, ++cp)
	{
	    base = *cp - 1;

	    /* set up local ring */
	    phase = ibase % n;
	    ring[phase] = base;

	    if(ibase < n-1)
		continue;

	    /* start new Cons */
	    new = etandem_consCreate();
	    new->start = ibase - n + 1;
	    new->next = root->next;
	    new->score = new->bestScore = -n;
	    new->phase = phase;	/* phase of the last base of pattern */
	    new->repeat = n;
	    root->next = new;

	    /* add last nmer to active Cons's */
	    olda = root;

	    if(DEBUG)
		ajDebug("%d\n", ibase);

	    for(a = olda->next; a; olda = a, a = a->next)
	    {
		if(a->phase == phase)
		{
		    for(i = 0; i < n; ++i)
		    {
			x = ring[i];
			if(x == 4 && mismatch)
			{
			    --a->score;
			    continue;
			}
			++ATAB(x,i);
			if(x == a->max[i])
			    ++a->score;
			else if(ATAB(x,i) > ATAB(a->max[i],i))
			{
			    a->max[i] = x;
			    ++a->score;
			}
			else
			{
			    --a->score;
			    if(ATAB(x,i) == ATAB(a->max[i], i))
				a->max[i] = x;
			}
		    }

		    if(a->score > a->bestScore)
		    {
			a->bestScore = a->score;
			a->ibest = ibase;
			for(j = 0; j < n; ++j)
			    a->bestMax[j] = a->max[j];
		    }
		    else if(a->score < 0)
		    {
			if(DEBUG) ajDebug("D");
			olda->next = a->next;
			etandem_report(&a);
			a = olda;
		    }
		}
	    }

	    /* remove duplicate max tables */
	    olda = root;
	    for(a = olda->next; a; olda = a, a = a->next)
	    {
		if(a->phase == phase)
		{
		    oldb = a;
		    for(b = a->next; b; b = b->next)
		    {			/* all phases */
			for(j = 0; j < n; ++j)
			{
			    if(a->max[j] != b->max[j])
				goto nextb;
			}

			if(a->bestScore > b->bestScore)
			{		/* remove b */
			    oldb->next = b->next;
			    if(DEBUG)
			    {
				ajDebug("B");
				etandem_basicReport(tab, outfile, b);
			    }
			    etandem_consDestroy(&b);
			    b = oldb;
			}
			else
			{		/* remove a */
			    olda->next = a->next;
			    if(DEBUG)
			    {
				ajDebug("A");
				etandem_basicReport(tab, outfile, a);
			    }
			    etandem_consDestroy(&a);
			    a = olda;
			    goto nexta;
			}

		    nextb:
			oldb = b;
		    }
		nexta:
		   ;
		}
	    }

	    if(DEBUG)
		for(a = root->next; a; a = a->next)
		    etandem_basicReport(tab, outfile, a);
	}

	while((a = root->next))
	{
	    root->next = a->next;
	    etandem_report(&a);
	}
    }

    etandem_finalReport(tab, outfile);
    ajReportWrite(report, tab, sequence);
    ajReportDel(&report);
    ajFeattableDel(&tab);
    ajSeqDel(&sequence);
    ajFileClose(&outfile);

    ajSeqcvtDel(&cvt);
    ajStrDel(&nseq);
    ajStrDel(&tmpstr);
    AJFREE(ring);

    embExit();

    return 0;
}

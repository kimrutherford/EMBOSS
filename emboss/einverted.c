/* @source einverted application
**
** Inverted repeats by dynamic programming
**
** @author Copyright (C) J Thierry-Mieg and R Durbin, 1993
** @modified July 4 2006 Jon Ison (ACD seqout added)
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

/*  File: inverted.c
**  Author: Richard Durbin (rd@mrc-lmba.cam.ac.uk)
**  Copyright (C) J Thierry-Mieg and R Durbin, 1993
**-------------------------------------------------------------------
** This file is part of the ACEDB genome database package, written by
** 	Richard Durbin (MRC LMB, UK) rd@mrc-lmba.cam.ac.uk, and
**	Jean Thierry-Mieg (CRBM du CNRS, France) mieg@crbm1.cnusc.fr
**
** Description: looks for inverted repeats using dynamic programming.
**		Ideas for fast implementation taken from James Crook's
**		thesis, fused/replaced by Gene Myers code.
** Exported functions:
** HISTORY:
** Created: Sat Jan 23 15:16:29 1993 (rd)
**-------------------------------------------------------------------
*/

#include "emboss.h"

#define MYERS			/* Gene Myers DP code */

/*
** Comment from Richard Durbin 6 April 2001:
** A critical parameter is the maximal extent of the interval from start to
** end of the repeat.  I don't know whether this is compile time or run
** time - I suspect compile time and set to 4000 bp just now.  You should
** look at the code and find the value and say what it is in the
** documentation.  Ideally this should be a command line configurable
** parameter, but this may mean some non-trivial recoding because currently
** I think arrays are statically not dynamically defined.
**
** Tim Carver 26/04/01:
** maxsave has been made a command line parameter
*/

static ajint match;
static ajint mismatch;
static ajint threshold;
static ajint gap;
static ajint maxsave;
static AjPFile outfile;
static AjPSeqCvt cvt;
static ajint rogue = 1000000;

static char base[] = "acgt-" ;

static const char *sq ;
static ajint *revmatch[5] ;
static ajint seqlength ;
static AjPInt2d matrix=NULL;



/* JISON new listseq, liststart, listend, pos parameters */
static void einverted_report(ajint max, ajint imax, const AjPSeq seq, 
			     AjPList *list, AjPInt *liststart, 
			     AjPInt *listend, ajint *pos);




/* @prog einverted ************************************************************
**
** Finds DNA inverted repeats
**
******************************************************************************/

int main(int argc, char **argv)
{
    ajint i;
    ajint j;
    ajint irel;
    ajint imax;
    ajint jmax = 0;
    ajint *ip;
    ajint *t1Base;
    ajint lastReported = -1;
    const char *cp;
    ajint a;
    ajint c;
    ajint d;
    ajint *t0;
    ajint *t1;
    ajint max;
    AjPInt localMax = NULL;
    AjPInt back = NULL;

    AjPSeqall seqall = NULL;
    AjPSeq sequence = NULL;
    AjPStr nseq = NULL;
    AjBool rev = ajFalse;
    AjBool comp = ajFalse;
    ajint ioffset = 0;

    /* JISON new variables */
    AjPList listseq = NULL;
    AjPInt  liststart = NULL;
    AjPInt  listend = NULL;
    ajint   pos = 0;
    ajint   temppos = 0;
    
    AjPSeqout seqout = NULL;
    AjPStr    tempstr = NULL;
    AjPSeq    tempseq = NULL;
    AjPStr    tempname = 0;
    tempseq = ajSeqNew();
    

    embInit("einverted", argc, argv);

    outfile   = ajAcdGetOutfile("outfile");
    seqall  = ajAcdGetSeqall("sequence");
    threshold = ajAcdGetInt("threshold");
    match     = ajAcdGetInt("match");
    mismatch  = ajAcdGetInt("mismatch");
    gap       = ajAcdGetInt("gap");
    maxsave   = ajAcdGetInt("maxrepeat");
    seqout    = ajAcdGetSeqout("outseq");
    

    cvt    = ajSeqcvtNewEndC("ACGT"); /* 0-3=ACGT 4=other */

    while(ajSeqallNext(seqall, &sequence))
    {
	/* JISON */
	listseq   = ajListstrNew();
	liststart = ajIntNew();
	listend   = ajIntNew();
	
	ajSeqTrim(sequence);
	seqlength = ajSeqGetLen(sequence);
	ajSeqConvertNum(sequence, cvt, &nseq);
	sq = ajStrGetPtr(nseq);
	rev = ajSeqIsReversed(sequence);
	if(rev)
	    ioffset = ajSeqGetOffend(sequence);
	else
	    ioffset = ajSeqGetOffset(sequence);

	ajDebug("sequence length: %d\n", seqlength);

	/*
	 ** build revmatch etc. to be a,t,g,c matched to reverse sequence
	 ** ending in MAXSAVE ROGUE values
	 */
	for(i = 5; i--;)
	{
	    AJCNEW(revmatch[i],(seqlength+maxsave));
	    ip = revmatch[i];

	    for(j = seqlength; j--; )
		*ip++ = mismatch;

	    for(j = maxsave; j--;)
		*ip++ = rogue;
	}
	
	cp = ajStrGetPtr(nseq);
	for(j = seqlength; j--;)		/* reverse order important here */
	    switch(*cp++)
	    {
	    case 0:
		revmatch[3][j] = match;
		break;			/* A */
	    case 1:
		revmatch[2][j] = match;
		break;			/* C */
	    case 2:
		revmatch[1][j] = match;
		break;			/* G */
	    case 3:
		revmatch[0][j] = match;
		break;			/* T */
	    }
	
	back     = ajIntNewRes(maxsave);
	localMax = ajIntNewRes(maxsave);
	matrix   = ajInt2dNewResRes2(maxsave, maxsave);

	for(i=0; i<maxsave; i++)
	{
	    ajIntPut(&back,i,0);
	    ajIntPut(&localMax,i,0);

	    for(j=0; j<maxsave; j++)
		ajInt2dPut(&matrix,i,j,0);
	}
	
	
	for(i = 0; i < seqlength+maxsave; ++i) /* +MAXSAVE to report at end */
	{
	    irel = i % maxsave;

	    /*ajDebug("i: %d irel: %d back[irel] %d\n", i, irel,
	      ajIntGet(back,irel));*/

	    if(ajIntGet(back,irel))	/* something to report */
	    {
		imax = 0;
		for(j = ajIntGet(back,irel); j > i-maxsave; --j)
		    if(ajIntGet(localMax,j%maxsave) > imax)
		    {
			jmax = j;
			imax = ajIntGet(localMax,j%maxsave);
		    }
		/* JISON added &listseq, &liststart, &listend args */
		einverted_report(imax, jmax, sequence, &listseq,
				 &liststart, &listend, &pos);
		lastReported = jmax;

		for(j = jmax; j >= i-maxsave; --j)
		{
		    ajIntPut(&localMax,j%maxsave,0);
		    ajIntPut(&back,j%maxsave,0);
		}
	    }

	    if(i >= seqlength)		/* report only */
		continue;

	    if(i == 0)
		t0 = (matrix->Ptr[maxsave-1]->Ptr) - 1;	/* NB offset by 1 */
	    else
		t0 = (matrix->Ptr[(i-1)%maxsave]->Ptr) - 1; /* NB offset by 1 */

	    t1 = (matrix->Ptr[irel]->Ptr);
	    memcpy(t1, &revmatch[(ajint)sq[i]][seqlength-i],
		   (maxsave-1)*sizeof(ajint));
	    t1[maxsave-2] = t1[maxsave-1] = rogue;

	    /*
	     ** Gene Myers' version of dynamic progamming:
	     ** a is current *t0, d is diagonal sum, c is working *t1 value
	     */

#ifdef TEST
	    ajDebug("\n%2d %c: ", i, base[(ajint)sq[i]]);

	    for(j = seqlength-i; --j;)
		ajDebug("      ");
	    ajDebug(" ");

	    if(*t1 > 0)
		ajDebug("*");
	    else
		ajDebug(" ");
	    ajDebug("%2d  ", *t1);
#endif

	    max    = threshold-1;
	    jmax   = 0;
	    t1Base = t1;

#ifdef MYERS
	    c = *t1;
	    a = -rogue;
	    while(1)			/* inner loop */
	    {
		d = *++t1;
		if(a > 0)
		    d += a;
		a = *++t0;
		if(a > c)
		    c = a;
		c -= gap;
		if(d > c)
		    c = d;
#ifdef TEST
		if(c == d)
		{
		    if(d == *t1)
			ajDebug(".");
		    else
			ajDebug("\\");
		}
		else if(c + gap == a)
		    ajDebug("|");
		else
		    ajDebug("-");

		if(*t1 > 0)
		    ajDebug("*");
		else
		    ajDebug(" ");
		ajDebug("%2d  ", c);
#endif
		*t1 = c;

		if(c > max)
		{
		    if(c >= rogue)
			goto done;
		    max = c;
		    jmax = (ajint) (t1 - t1Base);
		}
	    }
#endif

	done:
	    if(jmax)			/* max was broken */
	    {
		ajIntPut(&localMax,irel,max);
		j = (i-jmax-1) % maxsave;

		if(i-jmax-1 > lastReported &&
		   (!ajIntGet(back,j) ||
		    ajIntGet(localMax,ajIntGet(back,j)%maxsave) < max))
		    ajIntPut(&back,j,i);
	    }
	    else
		ajIntPut(&localMax,irel,0);
	}

	/* JISON new block */
	temppos=0;
	comp = rev;
	while(ajListstrPop(listseq, &tempstr))
	{
	    if(rev)
		ajFmtPrintS(&tempname, "%S_%d_%d_rev",
			    ajSeqGetNameS(sequence),
			    ioffset + seqlength + 1 - ajIntGet(listend, temppos),
			    ioffset + seqlength + 1 - ajIntGet(liststart, temppos));
	    else
		ajFmtPrintS(&tempname, "%S_%d_%d",
			    ajSeqGetNameS(sequence),
			    ioffset+ajIntGet(liststart, temppos),
			    ioffset+ajIntGet(listend, temppos));
	    ajSeqAssignNameS(tempseq, tempname);
	    ajSeqAssignSeqS(tempseq, tempstr);
	    if(seqout)
		ajSeqoutWriteSeq(seqout, tempseq);
	    ajStrDel(&tempstr);
	    temppos++;
	    comp = !comp;
	}	    
	ajListstrFree(&listseq);
	ajIntDel(&liststart);
	ajIntDel(&listend);
	
	ajIntDel(&localMax);
	ajIntDel(&back);
	ajInt2dDel(&matrix);
	
	for(i=0;i<5;i++)
	    AJFREE(revmatch[i]);
	
    }
    /* JISON new block */
    if(seqout)
	ajSeqoutClose(seqout);

    ajSeqDel(&tempseq);
    ajSeqoutDel(&seqout);

    
    ajSeqallDel(&seqall);
    ajSeqDel(&sequence);
    ajFileClose(&outfile);
    ajSeqcvtDel(&cvt);
    
    ajStrDel(&nseq);
    ajStrDel(&tempname);
    
    embExit();
    
    return 0;
}




/* @funcstatic einverted_report ***********************************************
**
** Undocumented.
**
** @param [r] max [ajint] Undocumented
** @param [r] imax [ajint] Undocumented
** @param [r] seq [const AjPSeq] Sequence
** @param [u] listseq [AjPList *] List (for sequence regions)
** @param [u] liststart [AjPInt *] List (for start of regions)
** @param [u] listend [AjPInt *] List (for end of regions)
** @param [u] pos [ajint*] Position
** @@
******************************************************************************/

static void einverted_report(ajint max, ajint imax, const AjPSeq seq, 
			     AjPList *listseq, AjPInt *liststart,
			     AjPInt *listend, ajint *pos)
{
    /* JISON new listseq, liststart, listend, pos parameters*/

    ajint *t1;
    ajint *ip;
    ajint *jp;
    ajint i;
    ajint j;
    ajint *align1;
    ajint *align2;
    ajint nmatch = 0;
    ajint nmis = 0;
    ajint ngap = 0;
    ajint saveMax = max;
    /* JISON new variables */
    AjPStr regionA=NULL;
    AjPStr regionB=NULL;
    AjBool rev = ajFalse;
    ajint ioffset = 0;
    ajint length = 0;
    

    AJCNEW(align1,2*maxsave);
    AJCNEW(align2,2*maxsave);

    ajDebug("report (%d %d)\n", max, imax);

    rev = ajSeqIsReversed(seq);
    if(rev)
    ioffset = ajSeqGetOffend(seq);
    else
	ioffset = ajSeqGetOffset(seq);
    length = ajSeqGetLen(seq);

    /* reconstruct maximum path */
    t1 = (matrix->Ptr[imax % maxsave]->Ptr);
    for(j = 0; j < maxsave; ++j)
	if(t1[j] == max)
	    break;

    i  = imax;
    ip = align1; jp = align2;
    while(max > 0 && j >= 1)		/* original missed blunt joins */
    {
	*ip++ = i;
	*jp++ = i-j;			/* seqpt + 1 */
#ifdef TEST
	ajDebug("i j, max (local): %d %d, %4d (%2d)\n",
		 i, j, max, revmatch[(ajint)sq[i]][length-i+j]);
#endif
	if(t1[j-1] == max + gap)
	{
	    max += gap;
	    ++ngap;
	    --j;
	    continue;
	}

	t1 = (matrix->Ptr[(i-1) % maxsave]->Ptr);
	if(t1[j-1] == max + gap)
	{
	    max += gap;
	    ++ngap;
	    --i;
	    --j;
	    continue;
	}

	max -= revmatch[(ajint)sq[i]][length-i+j];
	if(revmatch[(ajint)sq[i]][length-i+j] == match)
	    ++nmatch;
	else
	    ++nmis;
	--i; j-=2;
    }
    *ip = *jp = 0;
#ifdef TEST
    ajDebug("\n");
#endif

    /* report reconstruction */
    ajFmtPrintF(outfile, "\n%S: Score %d: %d/%d (%3d%%) matches, %d gaps\n",
		ajSeqGetNameS(seq),
		saveMax, nmatch, (nmatch+nmis),
		(100*nmatch)/(nmatch+nmis), ngap);


    /* JISON new block */
    regionA = ajStrNew();
    regionB = ajStrNew();
    ajIntPut(liststart, *pos, *align2);
    
    if(rev)				/* NB *jp is 1+coord */
	ajFmtPrintF(outfile, "%8d ", ioffset + length + 1 - *align2);
    else
	ajFmtPrintF(outfile, "%8d ", ioffset + *align2);/* NB *jp is 1+coord */
    for(jp = align2; *jp; ++jp)
	if(*jp == *(jp+1))
	    ajFmtPrintF(outfile, "-");
	else
	{
	    ajFmtPrintF(outfile, "%c", base[(ajint)sq[*jp-1]]);
	    ajStrAppendK(&regionA, base[(ajint)sq[*jp-1]]); /* JISON */
	}
    
    if(rev)
	ajFmtPrintF(outfile, " %-8d\n", ioffset + length + 1 - *(jp-1));
    else
	ajFmtPrintF(outfile, " %-8d\n", ioffset+*(jp-1));

    /* JISON new block */
    ajIntPut(listend, *pos, *(jp-1)); 
    (*pos)++;  
    

    ajFmtPrintF(outfile, "         ");
    for(ip = align1, jp = align2; *ip; ++ip, ++jp)
	if(*ip == *(ip+1) || *jp == *(jp+1))
	    ajFmtPrintF(outfile, " ");
	else if(sq[*ip] + sq[*jp-1] == 3) /* pmr: was 1 or 5 */
	    ajFmtPrintF(outfile, "|");
	else
	    ajFmtPrintF(outfile, " ");
    ajFmtPrintF(outfile, "\n");

    ajIntPut(listend, *pos, *align1 + 1); /* JISON */

    if(rev)
	ajFmtPrintF(outfile, "%8d ", ioffset + length + 1 - *align1 - 1);
    else
	ajFmtPrintF(outfile, "%8d ", ioffset + *align1 + 1);
    for(ip = align1; *ip; ++ip)
	if(*ip == *(ip+1))
	    ajFmtPrintF(outfile, "-");
	else
	{
	    ajFmtPrintF(outfile, "%c", base[(ajint)sq[*ip]]);
	    ajStrAppendK(&regionB, base[(ajint)sq[*ip]]);  /* JISON */
	}
    /* JISON new block */
    ajStrReverse(&regionB);
    ajListstrPushAppend(*listseq, regionA);
    ajListstrPushAppend(*listseq, regionB);
    
    if(rev)
	ajFmtPrintF(outfile, " %-8d\n", ioffset + length + 1 - *(ip-1) -1);
    else
	ajFmtPrintF(outfile, " %-8d\n", ioffset + *(ip-1)+1);

    /* JISON new block */
    ajIntPut(liststart, *pos, *(ip-1)+1);  
    (*pos)++;
    

    AJFREE(align1);
    AJFREE(align2);


    return;
}

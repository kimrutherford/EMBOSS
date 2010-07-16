/* @source matcher application
**
** MATCHER finds the best local alignments between two sequences
** version 2.0u4 Feb. 1996
** Please cite:
** X. Huang and W. Miller (1991) Adv. Appl. Math. 12:373-381
**
** @author Copyright (C) Ian Longden (il@sanger.ac.uk)
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




/* @macro matchergap **********************************************************
**
** sets k-symbol indel score
**
** @param [r] k [ajint] Symbol
** @return [void]
******************************************************************************/

#define matchergap(k)  ((k) <= 0 ? 0 : q+r*(k))	/* k-symbol indel score */




static ajint tt;

static ajint *sapp;				/* Current script append ptr */
static ajint  last;				/* Last script op appended */

static ajint I, J;				/* current positions of A ,B */
static ajint al_len; 				/* length of alignment */




/* @macro MATCHERDEL **********************************************************
**
** Macro for a "Delete k" operation
**
** @param [r] k [ajint] Undocumented
** @return [void]
******************************************************************************/

#define MATCHERDEL(k)			\
{ I += k;				\
  al_len += k;				\
  if (last < 0)				\
    last = sapp[-1] -= (k);		\
  else					\
    last = *sapp++ = -(k);		\
}




/* @macro MATCHERINS **********************************************************
**
** Macro for an "Insert k" operation
**
** @param [r] k [ajint] Undocumented
** @return [void]
******************************************************************************/

#define MATCHERINS(k)			\
{ J += k;				\
  al_len += k;				\
  if (last < 0)				\
    { sapp[-1] = (k); *sapp++ = last; }	\
  else					\
    last = *sapp++ = (k);		\
}




/* @macro MATCHERREP **********************************************************
**
** Macro for a "Replace" operation
**
** @return [void]
******************************************************************************/

#define MATCHERREP 			\
{ last = *sapp++ = 0; 			\
  al_len += 1;				\
}




/* @macro MATCHERDIAG *********************************************************
**
** assigns value to x if (ii,jj) is never used before
**
** @param [r] ii [ajint] Undocumented
** @param [r] jj [ajint] Undocumented
** @param [r] x [ajint] Undocumented
** @param [r] value [ajint] Undocumented
** @return [void]
******************************************************************************/

#define MATCHERDIAG(ii, jj, x, value)				\
{                                                		\
 for (tt = 1, z = row[(ii)]; z != PAIRNULL; z = z->NEXT)	\
    if (z->COL == (jj))					\
      { tt = 0; break; }					\
  if (tt)							\
    x = (value);						\
}




/* @macro MATCHERORDER ********************************************************
**
** replace (ss1, xx1, yy1) by (ss2, xx2, yy2) if the latter is large
**
** @param [r] ss1 [ajint] Undocumented
** @param [r] xx1 [ajint] Undocumented
** @param [r] yy1 [ajint] Undocumented
** @param [r] ss2 [ajint] Undocumented
** @param [r] xx2 [ajint] Undocumented
** @param [r] yy2 [ajint] Undocumented
** @return [void]
******************************************************************************/

#define MATCHERORDER(ss1, xx1, yy1, ss2, xx2, yy2)	\
{ if (ss1 < ss2)					\
    { ss1 = ss2; xx1 = xx2; yy1 = yy2; }		\
  else							\
    if (ss1 == ss2)					\
      { if (xx1 < xx2)				\
	  { xx1 = xx2; yy1 = yy2; }			\
	else						\
	  if (xx1 == xx2 && yy1 < yy2)		\
	    yy1 = yy2;					\
      }							\
}




static  AjPSeq seq;
static  AjPSeq seq2;
static ajint **sub;




/* @datastatic vertex *********************************************************
**
** Matcher internals
**
** @alias NODE
** @alias vertexptr
**
** @attr SCORE [ajint] Undocumented
** @attr STARI [ajint] Undocumented
** @attr STARJ [ajint] Undocumented
** @attr ENDI [ajint] Undocumented
** @attr ENDJ [ajint] Undocumented
** @attr TOP [ajint] Undocumented
** @attr BOT [ajint] Undocumented
** @attr LEFT [ajint] Undocumented
** @attr RIGHT [ajint] Undocumented
******************************************************************************/

typedef struct NODE
{
    ajint  SCORE;
    ajint  STARI;
    ajint  STARJ;
    ajint  ENDI;
    ajint  ENDJ;
    ajint  TOP;
    ajint  BOT;
    ajint  LEFT;
    ajint  RIGHT;
}  vertex;
#define vertexptr vertex*




vertexptr  *LIST;			/* an array for saving k best scores */
vertexptr  low = 0;			/* lowest score node in LIST */
vertexptr  most = 0;			/* latestly accessed node in LIST */
static ajint *CC, *DD;			/* saving matrix scores */
static ajint *RR, *SS, *EE, *FF; 	/* saving start-points */
static ajint *HH, *WW;		 	/* saving matrix scores */
static ajint *II, *JJ, *XX, *YY; 	/* saving start-points */
static ajint  m1, mm, n1, nn;		/* boundaries of recomputed area */
static ajint  rl, cl;			/* left and top boundaries */
static ajint  lmin;			/* minimum score in LIST */
static ajint flag;			/* indicate if recomputation needed */

static ajint q, r;			/* gap penalties */
static ajint qr;			/* qr = q + r */




/* @datastatic pair ***********************************************************
**
** Matcher internals
**
** @alias ONE
** @alias pairptr
**
** @attr NEXT [struct ONE*] Undocumented
** @attr COL [ajint] Undocumented
** @attr Padding [char[4]] Padding to alignment boundary
******************************************************************************/

typedef struct ONE
{
    struct ONE  *NEXT;
    ajint COL;
    char Padding[4];
} pair;
#define pairptr pair*

pairptr *row; 			/* for saving used aligned pairs */
pairptr z; 			/* for saving used aligned pairs */



#define PAIRNULL (pairptr)NULL

AjPMatrix matrix = NULL;
AjPSeqCvt cvt = NULL;




static void  matcher_Sim(AjPAlign align,
			const char A[], const char B[],
			 const AjPSeq seq0, const AjPSeq seq1, ajuint K,
			ajint Q, ajint R, ajint beg, ajint beg2, ajint nseq);
static ajint matcher_BigPass(const char A[], const char B[],
			     ajint M, ajint N, ajint K,
			    ajint nseq, ajint *numnode);
static ajint matcher_Locate(const char A[], const char B[], ajint nseq,
			    ajint numnode);
static ajint matcher_SmallPass(const char A[], const char B[],
			       ajint count, ajint nseq, ajint *numnode);
static ajint matcher_Diff(const char A[], const char B[],
			  ajint M, ajint N, ajint tb,
			  ajint te);
static ajint matcher_Calcons(char *seqc0, char *seqc1,
			     const ajint *res,
			     ajint min0, ajint min1,
			     ajint max0, ajint max1,
			     ajint *nc, ajint *nident);
static ajint matcher_Addnode(ajint c, ajint ci, ajint cj, ajint i, ajint j,
			     ajint K, ajint cost, ajint *numnode);
static ajint matcher_NoCross(ajint numnode);
static vertexptr matcher_Findmax(ajint *numnode);




/* @prog matcher **************************************************************
**
** Finds the best local alignments between two sequences
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPStr aa0str = 0;
    AjPStr aa1str = 0;
    const char *s1;
    const char *s2;
    ajint gdelval;
    ajint ggapval;
    ajuint i;
    ajint K;

    AjPAlign align = NULL;

    embInit("matcher", argc, argv);

    seq     = ajAcdGetSeq("asequence");
    ajSeqTrim(seq);
    seq2    = ajAcdGetSeq("bsequence");
    ajSeqTrim(seq2);
    matrix  = ajAcdGetMatrix("datafile");
    K       = ajAcdGetInt("alternatives");
    gdelval = ajAcdGetInt("gapopen");
    ggapval = ajAcdGetInt("gapextend");
    align   = ajAcdGetAlign("outfile");


    /*
      create sequence indices. i.e. A->0, B->1 ... Z->25 etc.
      This is done so that ajAZToInt has only to be done once for
      each residue in the sequence
    */

    ajSeqFmtUpper(seq);
    ajSeqFmtUpper(seq2);

    s1 = ajStrGetPtr(ajSeqGetSeqS(seq));
    s2 = ajStrGetPtr(ajSeqGetSeqS(seq2));

    sub = ajMatrixGetMatrix(matrix);
    cvt = ajMatrixGetCvt(matrix);


    aa0str = ajStrNewRes(2+ajSeqGetLen(seq)); /* length + blank + trailing null */
    aa1str = ajStrNewRes(2+ajSeqGetLen(seq2));
    ajStrAppendK(&aa0str,' ');
    ajStrAppendK(&aa1str,' ');

    for(i=0;i<ajSeqGetLen(seq);i++)
	ajStrAppendK(&aa0str,(char)ajSeqcvtGetCodeK(cvt, *s1++));

    for(i=0;i<ajSeqGetLen(seq2);i++)
	ajStrAppendK(&aa1str,ajSeqcvtGetCodeK(cvt, *s2++));

    matcher_Sim(align, ajStrGetPtr(aa0str),ajStrGetPtr(aa1str),
		seq,seq2,
		K,(gdelval-ggapval),ggapval,
		ajSeqGetOffset(seq), ajSeqGetOffset(seq2), 2);

    ajStrDel(&aa0str);
    ajStrDel(&aa1str);

    ajSeqDel(&seq);
    ajSeqDel(&seq2);

    embExit();

    return 0;
}




/* @funcstatic matcher_Sim ****************************************************
**
** Smith Waterman Eggert local alignment
**
** @param [u] align [AjPAlign] Alignment object
** @param [r] A [const char*] Sequence A with trailing blank
** @param [r] B [const char*] Sequence B with trailing blank
** @param [r] seq0 [const AjPSeq] Sequence A
** @param [r] seq1 [const AjPSeq] Sequence B
** @param [r] K [ajuint] Number of best alignments to report
** @param [r] Q [ajint] Gap penalty (minus extension penalty)
** @param [r] R [ajint] Gap extension penalty
** @param [r] beg0 [ajint] Offset of sequence A
** @param [r] beg1 [ajint] Offset of sequence B
** @param [r] nseq [ajint] Number of sequences
** @return [void]
******************************************************************************/

static void matcher_Sim(AjPAlign align,
			const char *A,const char *B,
			const AjPSeq seq0, const AjPSeq seq1,ajuint K,ajint Q,
			ajint R, ajint beg0, ajint beg1, ajint nseq)
{
    ajint endi;
    ajint endj;
    ajint stari;
    ajint starj;		 	/* endpoint and startpoint */
    ajint  score;   			/* the max score in LIST */
    ajint count;			/* maximum size of list */
    register ajuint  i;
    register ajuint j;			/* row and column indices */
    ajint  *S;				/* saving operations for diff */
    ajint nc;
    ajint nident;			/* for display */
    vertexptr cur; 			/* temporary pointer */
    ajint seq0len;
    ajint seq1len;

    AjPSeq res0 = NULL;
    AjPSeq res1 = NULL;

    char *seqc0, *seqc1;   /* aligned sequences */

    ajint numnode;			/* the number of nodes in LIST */

    ajint min0;
    ajint min1;
    ajint max0;
    ajint max1;

    seq0len = ajSeqGetLen(seq0);
    seq1len = ajSeqGetLen(seq1);

    /* allocate space for consensus */
    i = (seq0len+seq1len+1);
    AJCNEW(seqc0,i);
    AJCNEW(seqc1,i);
    /* allocate space for all vectors */

    j = (seq1len + 1)				/* * sizeof(ajint)*/;
    AJCNEW(CC, j);
    AJCNEW(DD, j);
    AJCNEW(RR, j);
    AJCNEW(SS, j);
    AJCNEW(EE, j);
    AJCNEW(FF, j);

    i = (seq0len + 1)				/* * sizeof(ajint)*/;
    AJCNEW(HH, i);
    AJCNEW(WW, i);
    AJCNEW(II, i);
    AJCNEW(JJ, i);
    AJCNEW(XX, i);
    AJCNEW(YY, i);
    AJCNEW(S,  (i+j));
    AJCNEW0(row,(seq0len + 1));

    /* set up list for each row (already zeroed by AJCNEW0 macro) */
    /* for(i = 1; i <= M; i++) row[i]= PAIRNULL;*/

    /*  vv = *sub[0];*/
    q = Q;
    r = R;
    qr = q + r;

    AJCNEW(LIST, K);
    for(i = 0; i < K; i++)
	AJNEW0(LIST[i]);

    numnode = lmin = 0;
    matcher_BigPass(A,B,seq0len,seq1len,K,nseq, &numnode);

    /* Report the K best alignments one by one. After each alignment is
       output, recompute part of the matrix. First determine the size
       of the area to be recomputed, then do the recomputation         */

    for(count = K - 1; count >= 0; count--)
    {
	if(numnode == 0)
	    ajFatal("The number of alignments computed is too large");
	cur = matcher_Findmax(&numnode);
	score = cur->SCORE;
	stari = ++cur->STARI;
	starj = ++cur->STARJ;
	endi  = cur->ENDI;
	endj  = cur->ENDJ;
	m1    = cur->TOP;
	mm    = cur->BOT;
	n1    = cur->LEFT;
	nn    = cur->RIGHT;
	rl    = endi - stari + 1;
	cl    = endj - starj + 1;
	I     = stari - 1;
	J     = starj - 1;
	sapp  = S;
	last  = 0;
	al_len = 0;
	matcher_Diff(&A[stari]-1, &B[starj]-1,rl,cl,q,q);

	min0 = stari-1;
	min1 = starj-1;
	max0 = stari+rl-1;
	max1 = starj+cl-1;
	matcher_Calcons(seqc0,seqc1,
			S, min0, min1, max0, max1, &nc, &nident);
	/* percent = (double)nident*100.0/(double)nc; Unused */

	ajDebug("Matcher min: %d %d max: %d %d beg: %d %d nc: %d\n",
		min0, min1, max0, max1, beg0, beg1, nc);
	ajDebug("Matcher offsets: %d %d end: %d %d len: %d %d alen: %d %d\n",
		ajSeqGetOffset(seq0), ajSeqGetOffset(seq1),
		ajSeqGetOffend(seq0), ajSeqGetOffend(seq1),
		ajSeqGetLen(seq0), ajSeqGetLen(seq1),
		strlen(seqc0), strlen(seqc1));
	ajDebug("Matcher seqc0: %s\n", seqc0);
	ajDebug("Matcher seqc1: %s\n", seqc1);

	res0 = ajSeqNewRangeC(seqc0, min0+beg0, ajSeqGetOffend(seq0) + ajSeqGetLen(seq0) - max0,
			       ajSeqIsReversed(seq0));
	ajSeqAssignUsaS(res0, ajSeqGetUsaS(seq0));
	ajSeqAssignNameS(res0, ajSeqGetNameS(seq0));

	res1 = ajSeqNewRangeC(seqc1, min1+beg1, ajSeqGetOffend(seq1) + ajSeqGetLen(seq1) - max1,
			       ajSeqIsReversed(seq1));
	ajSeqAssignNameS(res1, ajSeqGetNameS(seq1));
	ajSeqAssignUsaS(res1, ajSeqGetUsaS(seq1));

	ajAlignDefineSS(align, res0, res1);
	ajSeqDel(&res0);
	ajSeqDel(&res1);

	ajAlignSetGapI(align, Q+R, R);
	ajAlignSetScoreI(align, score);
	ajAlignSetMatrixInt(align, matrix);
	ajAlignSetStats(align, -1, nc, nident, -1, -1, NULL);

	ajAlignTrace(align);

	if(count)
	{
	    flag = 0;
	    matcher_Locate(A,B,nseq, numnode);
	    if(flag)
		matcher_SmallPass(A,B,count,nseq, &numnode);
	}
    }

    ajAlignWrite(align);

    /* now free all the memory */

    ajAlignClose(align);

    ajAlignDel(&align);

    AJFREE(CC);
    AJFREE(DD);
    AJFREE(RR);
    AJFREE(SS);
    AJFREE(EE);
    AJFREE(FF);
    AJFREE(HH);
    AJFREE(WW);
    AJFREE(II);
    AJFREE(JJ);
    AJFREE(XX);
    AJFREE(YY);
    AJFREE(S);

    for(i=0; i<=ajSeqGetLen(seq);i++)
    {
	pairptr this;
	pairptr next;
	if(row[i])
	{
	    this = row[i];
	    next = this->NEXT;
	    while(next)
	    {
		AJFREE(this);
		this = next;
		next= this->NEXT;
	    }
	    AJFREE(this);
	}
    }

    AJFREE(row);
    for(i = 0; i < K; i++)
	AJFREE(LIST[i]);
    AJFREE(LIST);


    AJFREE(seqc0);
    AJFREE(seqc1);

    return;
}




/* @funcstatic matcher_NoCross ************************************************
**
** return 1 if no node in LIST shares vertices with the area
**
** @param [r] numnode [ajint] Undocumented
** @return [ajint] 1 if no node shares vertices
******************************************************************************/

static ajint matcher_NoCross(ajint numnode)
{
    vertexptr  cur;
    register ajint i;

    for(i = 0; i < numnode; i++)
    {
	cur = LIST[i];
	if(cur->STARI <= mm && cur->STARJ <= nn && cur->BOT >= m1-1 &&
	    cur->RIGHT >= n1-1 && ( cur->STARI < rl || cur->STARJ < cl))
	{
	    if(cur->STARI < rl)
		rl = cur->STARI;

	    if(cur->STARJ < cl)
		cl = cur->STARJ;
	    flag = 1;
	    break;
	}
    }

    if(i == numnode)
	return 1;

    return 0;
}




/* @funcstatic matcher_BigPass ************************************************
**
** Undocumented
**
** @param [r] A [const char*] Undocumented
** @param [r] B [const char*] Undocumented
** @param [r] M [ajint] Undocumented
** @param [r] N [ajint] Undocumented
** @param [r] K [ajint] Undocumented
** @param [r] nseq [ajint] Number of sequences
** @param [w] numnode [ajint*] Number of alignments in list
** @return [ajint] Undocumented
******************************************************************************/

static ajint matcher_BigPass(const char *A,const char *B,
			     ajint M,ajint N,ajint K,
			      ajint nseq, ajint *numnode)
{
    register ajint  i;
    register ajint  j;			/* row and column indices */
    register ajint  c;			/* best score at current point */
    register ajint  f;			/* best score ending with insertion */
    register ajint  d;			/* best score ending with deletion */
    register ajint  p;			/* best score at (i-1, j-1) */
    register ajint  ci;
    register ajint  cj;			/* end-point associated with c */
    register ajint  di;
    register ajint  dj;			/* end-point associated with d */
    register ajint  fi;
    register ajint  fj;			/* end-point associated with f */
    register ajint  pi;
    register ajint  pj;			/* end-point associated with p */
    ajint  *va;				/* pointer to vv(A[i], B[j]) */

    /*
    ** Compute the matrix and save the top K best scores in LIST
    ** CC : the scores of the current row
    ** RR and EE : the starting point that leads to score CC
    ** DD : the scores of the current row, ending with deletion
    ** SS and FF : the starting point that leads to score DD
    */

    /* Initialize the 0 th row */
    for(j = 1; j <= N; j++)
    {
	CC[j] = 0;
	RR[j] = 0;
	EE[j] = j;
	DD[j] = - (q);
	SS[j] = 0;
	FF[j] = j;
    }

    for(i = 1; i <= M; i++)
    {
	c = 0;				/* Initialize column 0 */
	f = - (q);
	ci = fi = i;
	va = sub[(ajint)A[i]];

	if(nseq == 2)
	{
	    p = 0;
	    pi = i - 1;
	    cj = fj = pj = 0;
	}
	else
	{
	    p = CC[i];
	    pi = RR[i];
	    pj = EE[i];
	    cj = fj = i;
	}

	for(j = (nseq == 2 ? 1 : (i+1)); j <= N; j++)
	{
	    f = f - r;
	    c = c - qr;
	    MATCHERORDER(f, fi, fj, c, ci, cj)
	    c = CC[j] - qr;
	    ci = RR[j];
	    cj = EE[j];
	    d = DD[j] - r;
	    di = SS[j];
	    dj = FF[j];
	    MATCHERORDER(d, di, dj, c, ci, cj)
	    c = 0;
	    MATCHERDIAG(i, j, c, p+va[(ajint)B[j]]) /* diagonal */

	    if(c <= 0)
	    {
	        c = 0;
		ci = i;
		cj = j;
	    }
	    else
	    {
	        ci = pi; cj = pj; }
	    MATCHERORDER(c, ci, cj, d, di, dj)
	    MATCHERORDER(c, ci, cj, f, fi, fj)
	    p = CC[j];
	    CC[j] = c;
	    pi    = RR[j];
	    pj    = EE[j];
	    RR[j] = ci;
	    EE[j] = cj;
	    DD[j] = d;
	    SS[j] = di;
	    FF[j] = dj;
	    if(c > lmin)		/* add the score into list */
		lmin = matcher_Addnode(c, ci, cj, i, j, K, lmin, numnode);
	}
    }

    return 0;
}




/* @funcstatic matcher_Locate *************************************************
**
** Undocumented
**
** @param [r] A [const char*] Undocumented
** @param [r] B [const char*] Undocumented
** @param [r] nseq [ajint] Number of sequences
** @param [r] numnode [ajint] Number of alignments in list
** @return [ajint] Undocumented
******************************************************************************/

static ajint matcher_Locate(const char *A,const char *B,ajint nseq,
			    ajint numnode)
{
    register ajint i;
    register ajint j;			/* row and column indices */
    register ajint c;			/* best score at current point */
    register ajint f;			/* best score ending with insertion */
    register ajint d;			/* best score ending with deletion */
    register ajint p;			/* best score at (i-1, j-1) */
    register ajint ci;
    register ajint cj;			/* end-point associated with c */
    register ajint di=0;
    register ajint dj=0;		/* end-point associated with d */
    register ajint fi;
    register ajint fj;			/* end-point associated with f */
    register ajint pi;
    register ajint pj;			/* end-point associated with p */
    ajint cflag;
    ajint rflag;			/* for recomputation */
    ajint  *va;				/* pointer to vv(A[i], B[j]) */
    ajint  limit;			/* the bound on j */

    /*
    ** Reverse pass
    ** rows
    ** CC : the scores on the current row
    ** RR and EE : the endpoints that lead to CC
    ** DD : the deletion scores
    ** SS and FF : the endpoints that lead to DD
    **
    ** columns
    ** HH : the scores on the current columns
    ** II and JJ : the endpoints that lead to HH
    ** WW : the deletion scores
    ** XX and YY : the endpoints that lead to WW
    */

    for(j = nn; j >= n1; j--)
    {
	CC[j] = 0;
	EE[j] = j;
	DD[j] = - (q);
	FF[j] = j;
	if(nseq == 2 || j > mm)
	    RR[j] = SS[j] = mm + 1;
	else
	    RR[j] = SS[j] = j;
    }

    for(i = mm; i >= m1; i--)
    {
	c  = p = 0;
	f  = - (q);
	ci = fi = i;
	pi = i + 1;
	cj = fj = pj = nn + 1;
	va = sub[(ajint)A[i]];

	if(nseq == 2 || n1 > i)
	    limit = n1;
	else
	    limit = i + 1;

	for(j = nn; j >= limit; j--)
	{
	    f = f - r;
	    c = c - qr;
	    MATCHERORDER(f, fi, fj, c, ci, cj)
	    c = CC[j] - qr;
	    ci = RR[j];
	    cj = EE[j];
	    d = DD[j] - r;
	    di = SS[j];
	    dj = FF[j];
	    MATCHERORDER(d, di, dj, c, ci, cj)
	    c = 0;
	    MATCHERDIAG(i, j, c, p+va[(ajint)B[j]]) /* diagonal */

	    if(c <= 0)
	    {
		c  = 0;
	        ci = i;
	        cj = j;
	    }
	    else
	    {
	        ci = pi;
	        cj = pj;
	    }

	    MATCHERORDER(c, ci, cj, d, di, dj)
	    MATCHERORDER(c, ci, cj, f, fi, fj)
	    p = CC[j];
	    CC[j] = c;
	    pi = RR[j];
	    pj = EE[j];
	    RR[j] = ci;
	    EE[j] = cj;
	    DD[j] = d;
	    SS[j] = di;
	    FF[j] = dj;
	    if(c > lmin)
		flag = 1;
	}

	if(nseq == 2 || i < n1)
	{
	    HH[i] = CC[n1];
	    II[i] = RR[n1];
	    JJ[i] = EE[n1];
	    WW[i] = DD[n1];
	    XX[i] = SS[n1];
	    YY[i] = FF[n1];
	}
    }

    for(rl = m1, cl = n1; ;)
    {
	for(rflag = cflag = 1;( rflag && m1 > 1 ) || ( cflag && n1 > 1 );)
	{
	    if(rflag && m1 > 1)	/* Compute one row */
	    {
		rflag = 0;
		m1--;
		c = p = 0;
		f = - (q);
		ci = fi = m1;
		pi = m1 + 1;
		cj = fj = pj = nn + 1;
		va = sub[(ajint)A[m1]];

		for(j = nn; j >= n1; j--)
		{
		    f = f - r;
		    c = c - qr;
		    MATCHERORDER(f, fi, fj, c, ci, cj)
		    c = CC[j] - qr;
		    ci = RR[j];
		    cj = EE[j];
		    d = DD[j] - r;
		    di = SS[j];
		    dj = FF[j];
		    MATCHERORDER(d, di, dj, c, ci, cj)
		    c = 0;
		    MATCHERDIAG(m1, j, c, p+va[(ajint)B[j]]) /* diagonal */

		    if(c <= 0)
		    {
		        c = 0;
		        ci = m1;
		        cj = j;
		    }
		    else
		    {
		        ci = pi;
		        cj = pj;
		    }

		    MATCHERORDER(c, ci, cj, d, di, dj)
		    MATCHERORDER(c, ci, cj, f, fi, fj)
		    p = CC[j];
		    CC[j] = c;
		    pi = RR[j];
		    pj = EE[j];
		    RR[j] = ci;
		    EE[j] = cj;
		    DD[j] = d;
		    SS[j] = di;
		    FF[j] = dj;
		    if(c > lmin)
			flag = 1;
		    if(! rflag && ( (ci > rl && cj > cl) || (di > rl && dj >
							       cl)
				     || (fi > rl && fj > cl )))
			rflag = 1;
		}

		HH[m1] = CC[n1];
		II[m1] = RR[n1];
		JJ[m1] = EE[n1];
		WW[m1] = DD[n1];
		XX[m1] = SS[n1];
		YY[m1] = FF[n1];
		if(! cflag && ( (ci > rl && cj > cl) || (di > rl && dj > cl)
				 || (fi > rl && fj > cl)))
		    cflag = 1;
	    }

	    if(nseq == 1 && n1 == (m1 + 1) && ! rflag)
		cflag = 0;

	    if(cflag && n1 > 1)	/* Compute one column */
	    {
		cflag = 0;
		n1--;
		c = 0;
		f = - (q);
		cj = fj = n1;
		va = sub[(ajint)B[n1]];
		if(nseq == 2 || mm < n1)
		{
		    p = 0;
		    ci = fi = pi = mm + 1;
		    pj = n1 + 1;
		    limit = mm;
		}
		else
		{
		    p = HH[n1];
		    pi = II[n1];
		    pj = JJ[n1];
		    ci = fi = n1;
		    limit = n1 - 1;
		}

		for(i = limit; i >= m1; i--)
		{
		    f = f - r;
		    c = c - qr;
		    MATCHERORDER(f, fi, fj, c, ci, cj)
		    c = HH[i] - qr;
		    ci = II[i];
		    cj = JJ[i];
		    d = WW[i] - r;
		    di = XX[i];
		    dj = YY[i];
		    MATCHERORDER(d, di, dj, c, ci, cj)
		    c = 0;
		    MATCHERDIAG(i, n1, c, p+va[(ajint)A[i]])
		    if(c <= 0)
		    {
		        c = 0;
		        ci = i;
		        cj = n1;
		    }
		    else
		    {
		       ci = pi;
		       cj = pj;
		    }

		    MATCHERORDER(c, ci, cj, d, di, dj)
		    MATCHERORDER(c, ci, cj, f, fi, fj)
		    p = HH[i];
		    HH[i] = c;
		    pi = II[i];
		    pj = JJ[i];
		    II[i] = ci;
		    JJ[i] = cj;
		    WW[i] = d;
		    XX[i] = di;
		    YY[i] = dj;

		    if(c > lmin)
			flag = 1;

		    if(! cflag && ( (ci > rl && cj > cl) || (di > rl && dj >
							       cl)
				     || (fi > rl && fj > cl)))
			cflag = 1;
		}
		CC[n1] = HH[m1];
		RR[n1] = II[m1];
		EE[n1] = JJ[m1];
		DD[n1] = WW[m1];
		SS[n1] = XX[m1];
		FF[n1] = YY[m1];

		if(! rflag && ( (ci > rl && cj > cl) || (di > rl && dj > cl)
				 || (fi > rl && fj > cl)))
		    rflag = 1;
	    }
	}
	if((m1 == 1 && n1 == 1) || matcher_NoCross(numnode))
	    break;
    }
    m1--;
    n1--;

    return 0;
}




/* @funcstatic matcher_SmallPass **********************************************
**
** Undocumented
**
** @param [r] A [const char*] Undocumented
** @param [r] B [const char*] Undocumented
** @param [r] count [ajint] Undocumented
** @param [r] nseq [ajint] Number of sequences
** @param [w] numnode [ajint*] Number of alignments in list
** @return [ajint] Undocumented
******************************************************************************/

static ajint matcher_SmallPass(const char *A,const char *B,
			       ajint count,ajint nseq, ajint *numnode)
{
    register ajint i;
    register ajint j;			/* row and column indices */
    register ajint c;			/* best score at current point */
    register ajint f;			/* best score ending with insertion */
    register ajint d;			/* best score ending with deletion */
    register ajint p;			/* best score at (i-1, j-1) */
    register ajint ci;
    register ajint cj;			/* end-point associated with c */
    register ajint di;
    register ajint dj;			/* end-point associated with d */
    register ajint fi;
    register ajint fj;			/* end-point associated with f */
    register ajint pi;
    register ajint pj;			/* end-point associated with p */
    ajint  *va;				/* pointer to vv(A[i], B[j]) */
    ajint  limit;			/* lower bound on j */

    for(j = n1 + 1; j <= nn; j++)
    {
	CC[j] = 0;
	RR[j] = m1;
	EE[j] = j;
	DD[j] = - (q);
	SS[j] = m1;
	FF[j] = j;
    }

    for(i = m1 + 1; i <= mm; i++)
    {
	c = 0;				/* Initialize column 0 */
	f = - (q);
	ci = fi = i;
	va = sub[(ajint)A[i]];

	if(nseq == 2 || i <= n1)
	{
	    p = 0;
	    pi = i - 1;
	    cj = fj = pj = n1;
	    limit = n1 + 1;
	}
	else
	{
	    p = CC[i];
	    pi = RR[i];
	    pj = EE[i];
	    cj = fj = i;
	    limit = i + 1;
	}

	for(j = limit; j <= nn; j++)
	{
	    f = f - r;
	    c = c - qr;
	    MATCHERORDER(f, fi, fj, c, ci, cj)
	    c = CC[j] - qr;
	    ci = RR[j];
	    cj = EE[j];
	    d = DD[j] - r;
	    di = SS[j];
	    dj = FF[j];
	    MATCHERORDER(d, di, dj, c, ci, cj)
	    c = 0;
	    MATCHERDIAG(i, j, c, p+va[(ajint)B[j]]) /* diagonal */

	    if(c <= 0)
	    {
	      c = 0;
	      ci = i;
	      cj = j;
	    }
	    else
	    {
		ci = pi;
		cj = pj;
	    }

	    MATCHERORDER(c, ci, cj, d, di, dj)
	    MATCHERORDER(c, ci, cj, f, fi, fj)
	    p = CC[j];
	    CC[j] = c;
	    pi = RR[j];
	    pj = EE[j];
	    RR[j] = ci;
	    EE[j] = cj;
	    DD[j] = d;
	    SS[j] = di;
	    FF[j] = dj;
	    if(c > lmin)		/* add the score into list */
		lmin = matcher_Addnode(c, ci, cj, i, j, count, lmin, numnode);
	}
    }

    return 0;
}




/* @funcstatic matcher_Addnode ************************************************
**
** Add a node
**
** @param [r] c [ajint] Undocumented
** @param [r] ci [ajint] Undocumented
** @param [r] cj [ajint] Undocumented
** @param [r] i [ajint] Undocumented
** @param [r] j [ajint] Undocumented
** @param [r] K [ajint] Undocumented
** @param [r] cost [ajint] Undocumented
** @param [w] numnode [ajint*] NUmber of nodes (updated)
** @return [ajint] Undocumented
******************************************************************************/

static ajint matcher_Addnode(ajint c, ajint ci, ajint cj, ajint i, ajint j,
			     ajint K, ajint cost, ajint *numnode)
{
    ajint found;			/* 1 if the node is in LIST */
    register ajint d;

    found = 0;

    if(most != 0 && most->STARI == ci && most->STARJ == cj)
	found = 1;
    else
	for(d = 0; d < *numnode; d++)
	{
	    most = LIST[d];
	    if(most->STARI == ci && most->STARJ == cj)
	    {
		found = 1;
		break;
	    }
        }

    if(found)
    {
	if(most->SCORE < c)
        {
	    most->SCORE = c;
	    most->ENDI = i;
	    most->ENDJ = j;
        }

	if(most->TOP > i)
	    most->TOP = i;

	if(most->BOT < i)
	    most->BOT = i;

	if(most->LEFT > j)
	    most->LEFT = j;

	if(most->RIGHT < j)
	    most->RIGHT = j;
    }
    else
    {
	if(*numnode == K)		/* list full */
	    most = low;
	else
	    most = LIST[(*numnode)++];
	most->SCORE = c;
	most->STARI = ci;
	most->STARJ = cj;
	most->ENDI = i;
	most->ENDJ = j;
	most->TOP = most->BOT = i;
	most->LEFT = most->RIGHT = j;
    }

    if(*numnode == K)
    {
	if(low == most || ! low)
	    for(low = LIST[0], d = 1; d < *numnode; d++)
		if(LIST[d]->SCORE < low->SCORE)
		    low = LIST[d];

	return (low->SCORE);
    }


    return cost;
}




/* @funcstatic matcher_Findmax ************************************************
**
** Find maximum node
**
** @param [u] numnode [ajint*] Maximum node number
** @return [vertexptr] Maximum node
******************************************************************************/

static vertexptr matcher_Findmax(ajint *numnode)
{
    vertexptr  cur;
    register ajint i;
    register ajint j;

    for(j = 0, i = 1; i < *numnode; i++)
	if(LIST[i]->SCORE > LIST[j]->SCORE)
	    j = i;
    cur = LIST[j];

    if(j != --(*numnode))
    {
	LIST[j] = LIST[*numnode];
	LIST[*numnode] =  cur;
    }
    most = LIST[0];
    if(low == cur)
	low = LIST[0];

    return (cur);
}




/* @funcstatic matcher_Diff ***************************************************
**
** Undocumented
**
** @param [r] A [const char*] Undocumented
** @param [r] B [const char*] Undocumented
** @param [r] M [ajint] Undocumented
** @param [r] N [ajint] Undocumented
** @param [r] tb [ajint] Undocumented
** @param [r] te [ajint] Undocumented
** @return [ajint] Undocumented
******************************************************************************/


static ajint matcher_Diff(const char *A,const char *B,
			  ajint M,ajint N,ajint tb,ajint te)
{
    ajint midi;
    ajint midj;
    ajint type;				/* Midpoint, type, and cost */
    ajint midc;
    ajint  zero = 0;			/* ajint type zero        */

    register ajint i;
    register ajint j;
    register ajint c;
    register ajint e;
    register ajint d;
    register ajint s;
    ajint t;
    ajint *va;

    /* Boundary cases: M <= 1 or N == 0 */

    if(N <= 0)
    {
	if(M > 0)
	    MATCHERDEL(M)
	return - matchergap(M);
    }

    if(M <= 1)
    {
	if(M <= 0)
        {
	    MATCHERINS(N);
	    return - matchergap(N);
        }
	if(tb > te)
	    tb = te;
	midc = - (tb + r + matchergap(N));
	midj = 0;
	va = sub[(ajint)A[1]];

	for(j = 1; j <= N; j++)
        {
	    for(tt = 1, z = row[I+1]; z != PAIRNULL; z = z->NEXT)
		if(z->COL == j+J)
		{
		    tt = 0;
		    break;
		}

	    if(tt)
            {
		c = va[(ajint)B[j]] - ( matchergap(j-1) + matchergap(N-j));
		if(c > midc)
		{
		    midc = c;
		    midj = j;
		}
	    }
	}

	if(midj == 0)
	{
	    MATCHERINS(N)
	    MATCHERDEL(1)
	}
	else
        {
	    if(midj > 1) MATCHERINS(midj-1)
		MATCHERREP

	    /* mark(A[I],B[J]) as used: put J into list row[I] */
	    I++; J++;
	    AJNEW0(z);
	    z->COL = J;
	    z->NEXT = row[I];
	    row[I] = z;

	    if(midj < N)
		MATCHERINS(N-midj)
        }
	return midc;
    }

    /* Divide: Find optimum midpoint (midi,midj) of cost midc */

    midi = M/2;		/* Forward phase:                          */
    CC[0] = 0;		/*   Compute C(M/2,k) & D(M/2,k) for all k */

    t = -q;
    for(j = 1; j <= N; j++)
    {
	CC[j] = t = t-r;
	DD[j] = t-q;
    }
    t = -tb;

    for(i = 1; i <= midi; i++)
    {
	s = CC[0];
	CC[0] = c = t = t-r;
	e = t-q;
	va = sub[(ajint)A[i]];
	for(j = 1; j <= N; j++)
        {
	    if((c = c - qr) > (e = e - r))
		e = c;

	    if((c = CC[j] - qr) > (d = DD[j] - r))
		d = c;
	    MATCHERDIAG(i+I, j+J, c, s+va[(ajint)B[j]])

	    if(c < d)
	        c = d;

	    if(c < e)
		c = e;

	    s = CC[j];
	    CC[j] = c;
	    DD[j] = d;
        }
    }
    DD[0] = CC[0];

    RR[N] = 0;		/* Reverse phase:                          */
    t = -q;		/*   Compute R(M/2,k) & S(M/2,k) for all k */

    for(j = N-1; j >= 0; j--)
    {
	RR[j] = t = t-r;
	SS[j] = t-q;
    }
    t = -te;

    for(i = M-1; i >= midi; i--)
    {
	s = RR[N];
	RR[N] = c = t = t-r;
	e = t-q;
	va = sub[(ajint)A[i+1]];

	for(j = N-1; j >= 0; j--)
        {
	    if((c = c - qr) > (e = e - r))
		e = c;

	    if((c = RR[j] - qr) > (d = SS[j] - r))
		d = c;
	    MATCHERDIAG(i+1+I, j+1+J, c, s+va[(ajint)B[j+1]])

	    if(c < d)
		c = d;

	    if(c < e)
		c = e;

	    s = RR[j];
	    RR[j] = c;
	    SS[j] = d;
        }
    }
    SS[N] = RR[N];

    midc = CC[0]+RR[0];			/* Find optimal midpoint */
    midj = 0;
    type = 1;

    for(j = 0; j <= N; j++)
	if((c = CC[j] + RR[j]) >= midc)
	    if(c > midc || (CC[j] != DD[j] && RR[j] == SS[j]))
	    {
		midc = c;
		midj = j;
	    }

    for(j = N; j >= 0; j--)
	if((c = DD[j] + SS[j] + q) > midc)
	{
	    midc = c;
	    midj = j;
	    type = 2;
	}


    /* Conquer: recursively around midpoint */

    if(type == 1)
    {
	matcher_Diff(A,B,midi,midj,tb,q);
	matcher_Diff(A+midi,B+midj,M-midi,N-midj,q,te);
    }
    else
    {
	matcher_Diff(A,B,midi-1,midj,tb,zero);
	MATCHERDEL(2);
	matcher_Diff(A+midi+1,B+midj,M-midi-1,N-midj,zero,te);
    }

    return midc;
}




/* @funcstatic matcher_Calcons ************************************************
**
** Calculate a consensus sequence
**
** @param [w] seqc0 [char*] Sequence A alignment output
** @param [w] seqc1 [char*] Sequence B alignment output
** @param [r] res [const ajint*] Undocumented
** @param [r] min0 [ajint] Undocumented
** @param [r] min1 [ajint] Undocumented
** @param [r] max0 [ajint] Undocumented
** @param [r] max1 [ajint] Undocumented
** @param [w] nc [ajint*] Number of conserved positions
** @param [w] nident [ajint*] Number of identical positions
** @return [ajint] Undocumented
******************************************************************************/

static ajint matcher_Calcons(char *seqc0,char *seqc1,
			     const ajint *res,
			     ajint min0, ajint min1,
			     ajint max0, ajint max1,
			     ajint *nc,ajint *nident)
{
    ajint i0;
    ajint i1;
    ajint op;
    ajint nid;
    ajint lenc;
    ajint nd;
    char *sp0;
    char *sp1;
    const ajint *rp;

    const char *sq1;
    const char *sq2;

    /* now get the middle */

    sp0 = seqc0				/*+mins*/;
    sp1 = seqc1				/*+mins*/;
    rp = res;
    lenc = nid = op = 0;
    i0 = min0;
    i1 = min1;

    sq1 = ajStrGetPtr(ajSeqGetSeqS(seq));
    sq2 = ajStrGetPtr(ajSeqGetSeqS(seq2));

    while(i0 < max0 || i1 < max1)
    {
	if(op == 0 && *rp == 0)
	{
	    op = *rp++;
	    *sp0 = sq1[i0++];
	    *sp1 = sq2[i1++];
	    lenc++;
	    if(*sp0 == *sp1)
		nid++;

	    sp0++; sp1++;
	}
	else
	{
	    if(op==0)
		op = *rp++;

	    if(op>0)
	    {
		*sp0++ = '-';
		*sp1++ =  sq2[i1++];
		op--;
		lenc++;
	    }
	    else
	    {
		*sp0++ = sq1[i0++];
		*sp1++ = '-';
		op++;
		lenc++;
	    }
	}
    }
    *sp0 = '\0';
    *sp1 = '\0';
    *nident = nid;
    *nc = lenc;
    /*	get the right end */
    nd = 0;

    return lenc+nd;
}

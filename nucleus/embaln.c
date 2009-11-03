/* @source embaln.c
**
** General routines for alignment.
** Copyright (c) 1999 Alan Bleasby
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
#include <limits.h>
#include <math.h>




#define GAPO 26
#define GAPE 27

#define DIAG 0
#define LEFT 1
#define DOWN 2


static void alignPathCalcOld(const char *a, const char *b,
			     ajint lena, ajint lenb,
			     float gapopen, float gapextend, float *path,
			     float * const *sub, const AjPSeqCvt cvt,
			     ajint *compass, AjBool show);

/* @func embAlignPathCalc *****************************************************
**
** Create path matrix for Needleman-Wunsch
** Nucleotides or proteins as needed.
**
** @param [r] a [const char *] first sequence
** @param [r] b [const char *] second sequence
** @param [r] lena [ajint] length of first sequence
** @param [r] lenb [ajint] length of second sequence
** @param [r] gapopen [float] gap opening penalty
** @param [r] gapextend [float] gap extension penalty
** @param [w] path [float *] path matrix
** @param [r] sub [float * const *] substitution matrix from AjPMatrixf
** @param [r] cvt [const AjPSeqCvt] Conversion array for AjPMatrixf
** @param [w] compass [ajint *] Path direction pointer array
** @param [r] show [AjBool] Display path matrix
**
** Optimised to keep a maximum value to avoid looping down or left
** to find the maximum. (il 29/07/99)
**
** @return [void]
******************************************************************************/

void embAlignPathCalc(const char *a, const char *b,
		      ajint lena, ajint lenb, float gapopen,
		      float gapextend, float *path,
		      float * const *sub, const AjPSeqCvt cvt,
		      ajint *compass, AjBool show)
{
    ajint xpos;
    ajint i;
    ajint j;

    float match;
    float mscore;
    float fnew;
    float *maxa;
    float *maxb;
    float *oval;
    ajint *cnt;

    static AjPStr outstr = NULL;
    float bx;
    ajint bv;

    ajDebug("embAlignPathCalc\n");

    /* Create stores for the maximum values in a row or column */

    maxa = AJALLOC(lena*sizeof(float));
    maxb = AJALLOC(lenb*sizeof(float));
    oval = AJALLOC(lena*sizeof(float));
    cnt  = AJALLOC(lena*sizeof(ajint));


    /* First initialise the first column and row */
    for(i=0;i<lena;++i)
    {
	path[i*lenb] = sub[ajSeqcvtGetCodeK(cvt,a[i])][ajSeqcvtGetCodeK(cvt,b[0])];
	compass[i*lenb] = 0;
    }

    for(i=0;i<lena;++i)
    {
	maxa[i] = oval[i] = path[i*lenb]-(gapopen);
	cnt[i] = 0;
    }


    for(j=0;j<lenb;++j)
    {
	path[j] = sub[ajSeqcvtGetCodeK(cvt,a[0])][ajSeqcvtGetCodeK(cvt,b[j])];
	compass[j] = 0;
    }

    for(j=0;j<lenb;++j)
	maxb[j] = path[j]-(gapopen);

    /* xpos and ypos are the diagonal steps so start at 1 */
    xpos = 1;
    while(xpos!=lenb)
    {
	i  = 1;
	bx = maxb[xpos-1];
	bv = 0;

	while(i<lena)
	{
	    /* get match for current xpos/ypos */
	    match = sub[ajSeqcvtGetCodeK(cvt,a[i])][ajSeqcvtGetCodeK(cvt,b[xpos])];

	    /* Get diag score */
	    mscore = path[(i-1)*lenb+xpos-1] + match;

	    /*	  ajDebug("Opt %d %6.2f ",i*lenb+xpos,mscore); */

	    /* Set compass to diagonal value 0 */
	    compass[i*lenb+xpos] = 0;
	    path[i*lenb+xpos] = mscore;


	    /* Now parade back along X axis */
	    if(xpos-2>-1)
	    {
		fnew=path[(i-1)*lenb+xpos-2];
		fnew-=gapopen;
		if(maxa[i-1] < fnew)
		{
		    oval[i-1] = maxa[i-1] = fnew;
		    cnt[i-1] = 0;
		}
		++cnt[i-1];

		if( maxa[i-1]+match > mscore)
		{
		    mscore = maxa[i-1]+match;
		    path[i*lenb+xpos] = mscore;
		    compass[i*lenb+xpos] = 1; /* Score comes from left */
		}

	    }

	    /* And then bimble down Y axis */
	    if(i-2>-1)
	    {
		fnew = path[(i-2)*lenb+xpos-1];
		fnew-=gapopen;
		if(fnew>maxb[xpos-1])
		{
		    maxb[xpos-1]=bx=fnew;
		    bv=0;
		}
		++bv;

		if(maxb[xpos-1]+match > mscore)
		{
		    mscore = maxb[xpos-1]+match;
		    path[i*lenb+xpos] = mscore;
		    compass[i*lenb+xpos] = 2; /* Score comes from bottom */
		}
	    }


	    maxa[i-1]= oval[i-1] - ((float)cnt[i-1]*gapextend);
	    maxb[xpos-1]= bx - ((float)bv*gapextend);
	    i++;
	}
	++xpos;

    }
    
    if(show)
    {
	for(i=lena-1;i>-1;--i)
	{
	    ajStrDelStatic(&outstr);
	    for(j=0;j<lenb;++j)
		ajFmtPrintAppS(&outstr, "%6.2f ",path[i*lenb+j]);
	    ajDebug("%S\n", outstr);
	}
    }
    AJFREE(maxa);
    AJFREE(maxb);
    AJFREE(oval);
    AJFREE(cnt);

    ajStrDelStatic(&outstr);

    return;
}




/* @func embAlignPathCalcSW ***************************************************
**
** Create path matrix for Smith-Waterman
** Nucleotides or proteins as needed.
**
** @param [r] a [const char *] first sequence
** @param [r] b [const char *] second sequence
** @param [r] lena [ajint] length of first sequence
** @param [r] lenb [ajint] length of second sequence
** @param [r] gapopen [float] gap opening penalty
** @param [r] gapextend [float] gap extension penalty
** @param [w] path [float *] path matrix
** @param [r] sub [float * const *] substitution matrix from AjPMatrixf
** @param [r] cvt [const AjPSeqCvt] Conversion array for AjPMatrixf
** @param [w] compass [ajint *] Path direction pointer array
** @param [r] show [AjBool] Display path matrix
**
** Optimised to keep a maximum value to avoid looping down or left
** to find the maximum. (il 29/07/99)
**
** @return [void]
******************************************************************************/

void embAlignPathCalcSW(const char *a, const char *b, ajint lena, ajint lenb,
			float gapopen, float gapextend, float *path,
			float * const *sub, const AjPSeqCvt cvt, ajint *compass,
			AjBool show)
{
    ajint xpos;
    ajint i;
    ajint j;

    float match;
    float mscore;
    float result;
    float fnew;
    float *maxa;
    float *maxb;
    float *oval;
    ajint *cnt;

    static AjPStr outstr = NULL;
    float bx;
    ajint bv;

    ajDebug("embAlignPathCalcSW\n");

    /* Create stores for the maximum values in a row or column */

    maxa = AJALLOC(lena*sizeof(float));
    maxb = AJALLOC(lenb*sizeof(float));
    oval = AJALLOC(lena*sizeof(float));
    cnt  = AJALLOC(lena*sizeof(ajint));


    /* First initialise the first column and row */
    for(i=0;i<lena;++i)
    {
	result = sub[ajSeqcvtGetCodeK(cvt,a[i])][ajSeqcvtGetCodeK(cvt,b[0])];
	path[i*lenb] = (result > 0.) ? result : (float)0.;
	compass[i*lenb] = 0;
    }

    for(i=0;i<lena;++i)
    {
	maxa[i] = oval[i] = path[i*lenb]-(gapopen);
	cnt[i] = 0;
    }


    for(j=0;j<lenb;++j)
    {
	result = sub[ajSeqcvtGetCodeK(cvt,a[0])][ajSeqcvtGetCodeK(cvt,b[j])];
	path[j] = (result > 0.) ? result : (float)0.;
	compass[j] = 0;
    }

    for(j=0;j<lenb;++j)
	maxb[j] = path[j]-(gapopen);

    /* xpos and ypos are the diagonal steps so start at 1 */
    xpos = 1;
    while(xpos!=lenb)
    {
	i  = 1;
	bx = maxb[xpos-1];
	bv = 0;

	while(i<lena)
	{
	    /* get match for current xpos/ypos */
	    match = sub[ajSeqcvtGetCodeK(cvt,a[i])][ajSeqcvtGetCodeK(cvt,b[xpos])];

	    /* Get diag score */
	    mscore = path[(i-1)*lenb+xpos-1] + match;

	    /*	  ajDebug("Opt %d %6.2f ",i*lenb+xpos,mscore); */

	    /* Set compass to diagonal value 0 */
	    compass[i*lenb+xpos] = 0;
	    path[i*lenb+xpos] = mscore;


	    /* Now parade back along X axis */
	    if(xpos-2>-1)
	    {
		fnew=path[(i-1)*lenb+xpos-2];
		fnew-=gapopen;
		if(maxa[i-1] < fnew)
		{
		    oval[i-1] = maxa[i-1] = fnew;
		    cnt[i-1] = 0;
		}
		++cnt[i-1];

		if( maxa[i-1]+match > mscore)
		{
		    mscore = maxa[i-1]+match;
		    path[i*lenb+xpos] = mscore;
		    compass[i*lenb+xpos] = 1; /* Score comes from left */
		}

	    }

	    /* And then bimble down Y axis */
	    if(i-2>-1)
	    {
		fnew = path[(i-2)*lenb+xpos-1];
		fnew-=gapopen;
		if(fnew>maxb[xpos-1])
		{
		    maxb[xpos-1]=bx=fnew;
		    bv=0;
		}
		++bv;

		if(maxb[xpos-1]+match > mscore)
		{
		    mscore = maxb[xpos-1]+match;
		    path[i*lenb+xpos] = mscore;
		    compass[i*lenb+xpos] = 2; /* Score comes from bottom */
		}
	    }


	    maxa[i-1]= oval[i-1] - ((float)cnt[i-1]*gapextend);
	    maxb[xpos-1]= bx - ((float)bv*gapextend);

	    result = path[i*lenb+xpos];
	    if(result < 0.)
		path[i*lenb+xpos] = 0.;

	    i++;
	}
	++xpos;

    }

    if(show)
    {
	for(i=lena-1;i>-1;--i)
	{
	    ajStrDelStatic(&outstr);
	    for(j=0;j<lenb;++j)
		ajFmtPrintAppS(&outstr, "%6.2f ",path[i*lenb+j]);
	    ajDebug("%S\n", outstr);
	}
    }
    AJFREE(maxa);
    AJFREE(maxb);
    AJFREE(oval);
    AJFREE(cnt);

    ajStrDelStatic(&outstr);

    return;
}




/* @funcstatic alignPathCalcOld ***********************************************
**
** Create path matrix for Smith-Waterman and Needleman-Wunsch
** Nucleotides or proteins as needed.
**
** @param [r] a [const char *] first sequence
** @param [r] b [const char *] second sequence
** @param [r] lena [ajint] length of first sequence
** @param [r] lenb [ajint] length of second sequence
** @param [r] gapopen [float] gap opening penalty
** @param [r] gapextend [float] gap extension penalty
** @param [w] path [float *] path matrix
** @param [r] sub [float * const*] substitution matrix from AjPMatrixf
** @param [r] cvt [const AjPSeqCvt] Conversion array for AjPMatrixf
** @param [w] compass [ajint *] Path direction pointer array
** @param [r] show [AjBool] Display path matrix
**
** @return [void]
******************************************************************************/

static void alignPathCalcOld(const char *a, const char *b,
			     ajint lena, ajint lenb,
			     float gapopen, float gapextend, float *path,
			     float * const *sub, const AjPSeqCvt cvt,
			     ajint *compass, AjBool show)
{
    ajint xpos;
    ajint ypos;
    ajint i;
    ajint j;

    ajint im;
    ajint jm;

    float match;
    float mscore;
    float tsc;
    float pen;
    static AjPStr outstr = NULL;

    ajDebug("alignPathCalcOld\n");

    /* First initialise the first column and row */
    for(i=0;i<lena;++i)
    {
	path[i*lenb] = sub[ajSeqcvtGetCodeK(cvt,a[i])][ajSeqcvtGetCodeK(cvt,b[0])];
	compass[i*lenb] = 0;
    }

    for(j=0;j<lenb;++j)
    {
	path[j] = sub[ajSeqcvtGetCodeK(cvt,a[0])][ajSeqcvtGetCodeK(cvt,b[j])];
	compass[j] = 0;
    }


    /* xpos and ypos are our diagonal steps so start at 1 */
    xpos = ypos = 1;
    while(xpos<lenb && ypos<lena)
    {
	for(i=ypos;i<lena;++i)
	{
	    /* get match for current xpos/ypos */
	    match = sub[ajSeqcvtGetCodeK(cvt,a[i])][ajSeqcvtGetCodeK(cvt,b[xpos])];

	    /* Get diag score */
	    mscore = path[(i-1)*lenb+xpos-1] + match;

	    /* Set compass to diagonal value 0 */
	    compass[i*lenb+xpos] = 0;
	    path[i*lenb+xpos] = mscore;

	    /* Now parade back along X axis */
	    if(xpos-2>-1)
		for(jm=xpos-2;jm>-1;--jm)
		{
		    tsc = path[(i-1)*lenb+jm];
		    pen = (float) -1.0 * (gapopen + ((xpos-jm-2)*gapextend));
		    tsc += pen + match;
		    if(tsc>mscore)
		    {
			mscore=tsc;
			path[i*lenb+xpos] = tsc;
			compass[i*lenb+xpos] = 1; /* Score comes from left */
		    }
		}

	    /* And then bimble down Y axis */
	    if(i-2>-1)
		for(im=i-2;im>-1;--im)
		{
		    tsc = path[im*lenb+xpos-1];
		    pen = (float) -1.0 * (gapopen + ((i-im-2)*gapextend));
		    tsc += pen + match;
		    if(tsc>mscore)
		    {
			mscore=tsc;
			path[i*lenb+xpos] = tsc;
			compass[i*lenb+xpos] = 2; /* Score comes from bottom */
		    }
		}
	}

	/* move along */
	if(xpos+1 != lenb)
	{
	    for(j=xpos+1;j<lenb;++j)
	    {
		match=sub[ajSeqcvtGetCodeK(cvt,a[ypos])][ajSeqcvtGetCodeK(cvt,b[j])];
		mscore = path[(ypos-1)*lenb+j-1] + match;
		compass[ypos*lenb+j]=0;
		path[ypos*lenb+j]=mscore;

		/* parade once again back X */
		if(j-2>-1)
		    for(jm=j-2;jm>-1;--jm)
		    {
			tsc = path[(ypos-1)*lenb+jm];
			pen = (float) -1.0 * (gapopen + ((j-jm-2)*gapextend));
			tsc += pen+match;
			if(tsc>mscore)
			{
			    mscore = tsc;
			    path[ypos*lenb+j] = tsc; /* Came from left */
			    compass[ypos*lenb+j]=1;
			}
		    }

		/* Re-bimble down Y */
		if(ypos-2>-1)
		{
		    for(im=ypos-2;im>-1;--im)
		    {
			tsc = path[im*lenb+j-1];
			pen = (float) -1.0 * (gapopen+((ypos-im-2)*gapextend));
			tsc += pen+match;
			if(tsc>mscore)
			{
			    mscore=tsc;
			    path[ypos*lenb+j]=tsc;
			    compass[ypos*lenb+j]=2;	/* from bottom */
			}
		    }
		}
	    }
	}

	++xpos;
	++ypos;

    }

    if(show)
    {
	for(i=lena-1;i>-1;--i)
	{
	    ajStrDelStatic(&outstr);
	    for(j=0;j<lenb;++j)
		ajFmtPrintAppS(&outstr,"%6.2f ",path[i*lenb+j]);
	    ajDebug("%S\n", outstr);
	}
    }

    ajStrDelStatic(&outstr);
    return;
}




/* @func embAlignScoreNWMatrix ************************************************
**
** Score a  matrix for Needleman Wunsch.
** Nucleotides or proteins as needed.
**
** @param [r] path [const float *] path matrix
** @param [r] a [const AjPSeq] first sequence
** @param [r] b [const AjPSeq] second sequence
** @param [r] fmatrix [float * const *] substitution matrix from
**                    AjPMatrixf
** @param [r] cvt [const AjPSeqCvt] Conversion array for AjPMatrixf
** @param [r] lena [ajint] length of first sequence
** @param [r] lenb [ajint] length of second sequence
** @param [r] gapopen [float] gap opening coefficient
** @param [r] compass [const ajint*] Path direction pointer array
** @param [r] gapextend [float] gap extension coefficient
** @param [w] start1 [ajint *] start of alignment in first sequence
** @param [w] start2 [ajint *] start of alignment in second sequence
**
** @return [float] Maximum path axis score
** @@
******************************************************************************/

float embAlignScoreNWMatrix(const float *path, const AjPSeq a, const AjPSeq b,
			    float * const *fmatrix,
			    const AjPSeqCvt cvt, ajint lena, ajint lenb,
			    float gapopen, const ajint *compass,
			    float gapextend, ajint *start1, ajint *start2)
{
    ajint i;
    ajint j;

    float pmax;
    float score;
    float match;
    float gapcnt;
    float bimble;
    float wscore;
    float errbounds;

    ajint ix;
    ajint iy;
    ajint t;

    ajint xpos = 0;
    ajint ypos = 0;
    const char *p;
    const char *q;

    ajDebug("embAlignScoreNWMatrix\n");

    errbounds = gapextend;
    errbounds = (float) 0.01;

    /* Get maximum path axis score and save position */
    pmax = (float) (-1*INT_MAX);
    for(i=0;i<lenb;++i)
	if(path[(lena-1)*lenb+i]>=pmax)
	{
	    pmax = path[(lena-1)*lenb+i];
	    xpos = i;
	    ypos = lena-1;
	}

    for(j=0;j<lena;++j)
	if(path[j*lenb+lenb-1]>pmax)
	{
	    pmax=path[j*lenb+lenb-1];
	    xpos=lenb-1;
	    ypos=j;
	}

    p = ajSeqGetSeqC(a);
    q = ajSeqGetSeqC(b);

    wscore = fmatrix[ajSeqcvtGetCodeK(cvt,p[ypos])][ajSeqcvtGetCodeK(cvt,q[xpos])];
    /*    ajDebug("Match %c %c %f\n",p[ypos],q[xpos],wscore);*/

    while(xpos && ypos)
    {

	if(!compass[ypos*lenb+xpos])	/* diagonal */
	{
	    wscore += fmatrix[ajSeqcvtGetCodeK(cvt,p[--ypos])]
		             [ajSeqcvtGetCodeK(cvt,q[--xpos])];
	    /*ajDebug("Match %c %c %f\n",p[ypos],q[xpos],wscore);*/
	}
	else if(compass[ypos*lenb+xpos]==1) /* Left, gap(s) in vertical */
	{
	    score  = path[ypos*lenb+xpos];
	    gapcnt = 0.;
	    ix     = xpos-2;
	    match = fmatrix[ajSeqcvtGetCodeK(cvt,p[ypos])][ajSeqcvtGetCodeK(cvt,q[xpos])];
	    --ypos;
	    t = ix+1;
	    while(1)
	    {
		bimble = path[ypos*lenb+ix]-gapopen-(gapcnt*gapextend)+match;
		if(!ix || fabs((double)score-(double)bimble)< errbounds)
		    break;
		--ix;
		if(ix<0)
		    ajFatal("NW: Error walking left");

		++gapcnt;
	    }

	    t -= (ajint)gapcnt;

	    wscore += fmatrix[ajSeqcvtGetCodeK(cvt,p[ypos])][ajSeqcvtGetCodeK(cvt,q[t-1])];
	    wscore -= (gapopen + (gapextend*gapcnt));

	    xpos = ix;
	    continue;
	}
	else if(compass[ypos*lenb+xpos]==2) /* Down, gap(s) in horizontal */
	{
	    score=path[ypos*lenb+xpos];
	    gapcnt=0.;
	    match = fmatrix[ajSeqcvtGetCodeK(cvt,p[ypos])][ajSeqcvtGetCodeK(cvt,q[xpos])];
	    --xpos;
	    iy=ypos-2;
	    t=iy+1;

	    while(1)
	    {
		bimble = path[iy*lenb+xpos]-gapopen-(gapcnt*gapextend)+match;
		if(!iy || fabs((double)score-(double)bimble)< errbounds)
		    break;
		--iy;
		if(iy<0)
		    ajFatal("NW: Error walking down");

		++gapcnt;
	    }

	    t -= (ajint)gapcnt;

	    wscore += fmatrix[ajSeqcvtGetCodeK(cvt,p[t-1])][ajSeqcvtGetCodeK(cvt,q[xpos])];

	    wscore -= (gapopen + (gapextend*gapcnt));

	    ypos = iy;
	    continue;
	}
	else
	    ajFatal("Walk Error in NW");
    }

    *start1 = ypos;
    *start2 = xpos;

    return wscore;
}




/* @func embAlignScoreSWMatrix ************************************************
**
** Walk down a matrix for Smith Waterman. Form aligned strings.
** Nucleotides or proteins as needed.
**
** @param [r] path [const float*] path matrix
** @param [r] compass [const ajint*] Path direction pointer array
** @param [r] gapopen [float] gap opening penalty
** @param [r] gapextend [float] gap extension penalty
** @param [r] a [const AjPSeq] first sequence
** @param [r] b [const AjPSeq] second sequence
** @param [r] lena [ajint] length of first sequence
** @param [r] lenb [ajint] length of second sequence
** @param [r] sub [float * const *] substitution matrix from AjPMatrixf
** @param [r] cvt [const AjPSeqCvt] Conversion array for AjPMatrixf
** @param [w] start1 [ajint *] start of alignment in first sequence
** @param [w] start2 [ajint *] start of alignment in second sequence
**
** @return [float] Score of best matching segment
******************************************************************************/

float embAlignScoreSWMatrix(const float *path, const ajint *compass,
			    float gapopen,
			    float gapextend,  const AjPSeq a, const AjPSeq b,
			    ajint lena, ajint lenb, float * const *sub,
			    const AjPSeqCvt cvt, ajint *start1, ajint *start2)
{
    ajint i;
    ajint j;
    float pmax;
    float score;
    float match;
    float gapcnt;
    float bimble;
    float wscore;

    ajint ix = 0;
    ajint iy = 0;
    ajint t;

    ajint xpos = 0;
    ajint ypos = 0;
    const char *p;
    const char *q;
    float errbounds;

    ajDebug("embAlignScoreSWMatrix\n");

    errbounds = gapextend;
    errbounds = (float) 0.01;

    /* Get maximum path score and save position */
    pmax = (float) (-1*INT_MAX);
    for(i=0;i<lena;++i)
	for(j=0;j<lenb;++j)
	    if(path[i*lenb+j]>pmax)
	    {
		pmax=path[i*lenb+j];
		xpos=j;
		ypos=i;
	    }

    p = ajSeqGetSeqC(a);
    q = ajSeqGetSeqC(b);

    wscore = sub[ajSeqcvtGetCodeK(cvt,p[ypos])][ajSeqcvtGetCodeK(cvt,q[xpos])];

    while(xpos && ypos)
    {
	if(!compass[ypos*lenb+xpos])	/* diagonal */
	{
	    if(path[(ypos-1)*lenb+xpos-1]<=0.)
	    {
		*start1 = ypos;
		*start2 = xpos;
		ajDebug("break: trace done at ypos:%d xpos:%d\n", ypos, xpos);
		break;
	    }

	    wscore += sub[ajSeqcvtGetCodeK(cvt,p[--ypos])][ajSeqcvtGetCodeK(cvt,q[--xpos])];
	    continue;
	}
	else if(compass[ypos*lenb+xpos]==1) /* Left, gap(s) in vertical */
	{
	    score  = path[ypos*lenb+xpos];
	    gapcnt = 0.;
	    ix     = xpos-2;
	    match = sub[ajSeqcvtGetCodeK(cvt,p[ypos])][ajSeqcvtGetCodeK(cvt,q[xpos])];
	    --ypos;
	    t = ix+1;
	    while(1)
	    {
		bimble =path[ypos*lenb+ix]-gapopen-(gapcnt*gapextend)+match;
		if(!ix || fabs((double)score-(double)bimble)<errbounds)
		{
		    ajDebug("inner break: ix errbounds at "
			    "ypos:%d xpos:%d ix:%d t:%d bimble:%.3f\n",
			    ypos, xpos, ix, t, bimble);
		    break;
		}
		--ix;
		if(ix<0)
		    ajFatal("SW: Error walking left");
		++gapcnt;
	    }

	    if(bimble<=0.0)
	    {
		++ypos;
		ajDebug("break: bimble<0 after ix  ypos:%d xpos:%d\n",
			ypos, xpos);
		break;
	    }

	    t -= (ajint)gapcnt;

	    wscore += sub[ajSeqcvtGetCodeK(cvt,p[ypos])][ajSeqcvtGetCodeK(cvt,q[t-1])];
	    wscore -= (gapopen + (gapextend*gapcnt));

	    xpos = ix;
	    continue;
	}
	else if(compass[ypos*lenb+xpos]==2) /* Down, gap(s) in horizontal */
	{
	    score  = path[ypos*lenb+xpos];
	    gapcnt = 0.;
	    match  = sub[ajSeqcvtGetCodeK(cvt,p[ypos])][ajSeqcvtGetCodeK(cvt,q[xpos])];
	    --xpos;
	    iy = ypos-2;
	    t  = iy+1;

	    while(1)
	    {
		bimble=path[iy*lenb+xpos]-gapopen-(gapcnt*gapextend)+match;

		if(!iy ||  fabs((double)score-(double)bimble) < errbounds)
		{
		    ajDebug("inner break: iy errbounds at "
			    "ypos:%d xpos:%d iy:%d t:%d bimble:%.3f\n",
			    ypos, xpos, iy, t, bimble);
		    break;
		}

		--iy;

		if(iy<0)
		    ajFatal("SW: Error walking down");

		++gapcnt;
	    }

	    if(bimble<=0.0)
	    {
		++xpos;
		ajDebug("Break: bimble < 0 after iy with xpos %d\n", xpos);
		break;
	    }

	    t -= (ajint)gapcnt;

	    wscore += sub[ajSeqcvtGetCodeK(cvt,p[t-1])][ajSeqcvtGetCodeK(cvt,q[xpos])];
	    wscore -= (gapopen + (gapextend*gapcnt));

	    ypos = iy;
	    continue;
	}
	else
	    ajFatal("Walk Error in SW");
    }

    if(ypos > 0)
	*start1 = ypos;
    if(xpos > 0)
	*start2 = xpos;

    ajDebug("ScoreSW ypos(start1):%d xpos(start2):%d iy:%d ix:%d\n",
	    ypos, xpos, iy, ix);

    return wscore;
}




/* @func embAlignWalkSWMatrix *************************************************
**
** Walk down a matrix for Smith Waterman. Form aligned strings.
** Nucleotides or proteins as needed.
**
** @param [r] path [const float*] path matrix
** @param [r] compass [const ajint*] Path direction pointer array
** @param [r] gapopen [float] gap opening penalty
** @param [r] gapextend [float] gap extension penalty
** @param [r] a [const AjPSeq] first sequence
** @param [r] b [const AjPSeq] second sequence
** @param [w] m [AjPStr *] alignment for first sequence
** @param [w] n [AjPStr *] alignment for second sequence
** @param [r] lena [ajint] length of first sequence
** @param [r] lenb [ajint] length of second sequence
** @param [r] sub [float * const *] substitution matrix from AjPMatrixf
** @param [r] cvt [const AjPSeqCvt] Conversion array for AjPMatrixf
** @param [w] start1 [ajint *] start of alignment in first sequence
** @param [w] start2 [ajint *] start of alignment in second sequence
**
** @return [void]
******************************************************************************/

void embAlignWalkSWMatrix(const float *path, const ajint *compass,
			  float gapopen,
			  float gapextend, const AjPSeq a, const AjPSeq b,
			  AjPStr *m,
			  AjPStr *n, ajint lena, ajint lenb,
			  float * const *sub,
			  const AjPSeqCvt cvt, ajint *start1, ajint *start2)
{
    ajint i;
    ajint j;
    float pmax;
    float score;
    float match;
    float gapcnt;
    float bimble;

    ajint ix;
    ajint iy;
    ajint t;

    ajint xpos = 0;
    ajint ypos = 0;
    const char *p;
    const char *q;

    float ic;
    float errbounds;

    ajDebug("embAlignWalkSWMatrix\n");

    errbounds = gapextend;
    errbounds = (float) 0.01;

    /* Get maximum path score and save position */
    pmax = (float) (-1*INT_MAX);
    for(i=0;i<lena;++i)
	for(j=0;j<lenb;++j)
	    if(path[i*lenb+j]>pmax)
	    {
		pmax = path[i*lenb+j];
		xpos = j;
		ypos = i;
	    }

    p = ajSeqGetSeqC(a);
    q = ajSeqGetSeqC(b);

    ajStrAssignK(m,p[ypos]);
    ajStrAssignK(n,q[xpos]);

    while(xpos && ypos)
    {
	if(!compass[ypos*lenb+xpos])	/* diagonal */
	{
	    if(path[(ypos-1)*lenb+xpos-1]<=0.)
		break;
	    ajStrInsertK(m,0,p[--ypos]);
	    ajStrInsertK(n,0,q[--xpos]);
	    continue;
	}
	else if(compass[ypos*lenb+xpos]==1) /* Left, gap(s) in vertical */
	{
	    score  = path[ypos*lenb+xpos];
	    gapcnt = 0.;
	    ix     = xpos-2;
	    match  = sub[ajSeqcvtGetCodeK(cvt,p[ypos])][ajSeqcvtGetCodeK(cvt,q[xpos])];
	    --ypos;
	    t=ix+1;
	    while(1)
	    {
		bimble = path[ypos*lenb+ix]-gapopen-(gapcnt*gapextend)+match;
		if(!ix || fabs((double)score-(double)bimble)<errbounds)
		    break;
		--ix;
		if(ix<0)
		    ajFatal("SW: Error walking left");
		++gapcnt;
	    }

	    if(bimble<=0.0)
	    {
		++ypos;
		break;
	    }

	    for(ic=-1;ic<gapcnt;++ic)
	    {
		ajStrInsertK(m,0,'.');
		ajStrInsertK(n,0,q[t--]);
	    }
	    ajStrInsertK(n,0,q[t]);
	    ajStrInsertK(m,0,p[ypos]);

	    xpos = ix;
	    continue;
	}
	else if(compass[ypos*lenb+xpos]==2) /* Down, gap(s) in horizontal */
	{
	    score  = path[ypos*lenb+xpos];
	    gapcnt = 0.;
	    match  = sub[ajSeqcvtGetCodeK(cvt,p[ypos])][ajSeqcvtGetCodeK(cvt,q[xpos])];
	    --xpos;
	    iy = ypos-2;
	    t  = iy+1;

	    while(1)
	    {
		bimble=path[iy*lenb+xpos]-gapopen-(gapcnt*gapextend)+match;
		if(!iy || fabs((double)score-(double)bimble)<errbounds)
		    break;
		--iy;
		if(iy<0)
		    ajFatal("SW: Error walking down");
		++gapcnt;
	    }
	    if(bimble<=0.0)
	    {
		++xpos;
		break;
	    }

	    for(ic=-1;ic<gapcnt;++ic)
	    {
		ajStrInsertK(n,0,'.');
		ajStrInsertK(m,0,p[t--]);
	    }
	    ajStrInsertK(m,0,p[t]);
	    ajStrInsertK(n,0,q[xpos]);
	    ypos = iy;
	    continue;
	}
	else
	    ajFatal("Walk Error in SW");
    }

    *start1 = ypos;
    *start2 = xpos;
    ajDebug("embAlignWalkSWMatrix done\n");
    return;
}




/* @func embAlignWalkNWMatrix *************************************************
**
** Walk down a matrix for Needleman Wunsch. Form aligned strings.
** Nucleotides or proteins as needed.
**
** @param [r] path [const float*] path matrix
** @param [r] a [const AjPSeq] first sequence
** @param [r] b [const AjPSeq] second sequence
** @param [w] m [AjPStr *] alignment for first sequence
** @param [w] n [AjPStr *] alignment for second sequence
** @param [r] lena [ajint] length of first sequence
** @param [r] lenb [ajint] length of second sequence
** @param [w] start1 [ajint *] start of alignment in first sequence
** @param [w] start2 [ajint *] start of alignment in second sequence
** @param [r] gapopen [float] gap open penalty
** @param [r] gapextend [float] gap extension penalty
** @param [r] cvt [const AjPSeqCvt] Conversion array for AjPMatrixf
** @param [r] compass [const ajint*] Path direction pointer array
** @param [r] sub [float * const*] substitution matrix from AjPMatrixf
**
** @return [void]
******************************************************************************/

void embAlignWalkNWMatrix(const float *path, const AjPSeq a, const AjPSeq b,
			  AjPStr *m,
			  AjPStr *n, ajint lena, ajint lenb, ajint *start1,
			  ajint *start2, float gapopen,
			  float gapextend, const AjPSeqCvt cvt,
			  const ajint *compass,
			  float * const *sub)
{
    ajint i;
    ajint j;
    float pmax;
    float score;
    float match;
    float gapcnt;
    float bimble;

    ajint ix;
    ajint iy;
    ajint t;

    ajint xpos = 0;
    ajint ypos = 0;
    const char *p;
    const char *q;

    float ic;
    float errbounds;

    ajDebug("embAlignWalkNWMatrix\n");

    ajDebug("seqlen a:%d b:%d\n", ajSeqGetLen(a), ajSeqGetLen(b));

    errbounds=gapextend;
    errbounds = (float) 0.01;

    /* Get maximum path axis score and save position */
    pmax = (float) (-1*INT_MAX);
    for(i=0;i<lenb;++i)
	if(path[(lena-1)*lenb+i]>=pmax)
	{
	    pmax = path[(lena-1)*lenb+i];
	    xpos = i;
	    ypos = lena-1;
	}

    for(j=0;j<lena;++j)
	if(path[j*lenb+lenb-1]>pmax)
	{
	    pmax = path[j*lenb+lenb-1];
	    xpos = lenb-1;
	    ypos = j;
	}

    p = ajSeqGetSeqC(a);
    q = ajSeqGetSeqC(b);

    ajStrInsertK(m,0,p[ypos]);
    ajStrInsertK(n,0,q[xpos]);
    while(xpos && ypos)
    {
	if(!compass[ypos*lenb+xpos])	/* diagonal */
	{
	    ajDebug("match %5d %5d '%c' '%c'\n",
		    ypos, xpos, p[ypos-1], q[xpos-1]);
	    ajStrInsertK(m,0,p[--ypos]);
	    ajStrInsertK(n,0,q[--xpos]);
	    continue;
	}
	else if(compass[ypos*lenb+xpos]==1) /* Left, gap(s) in vertical */
	{
	    score  = path[ypos*lenb+xpos];
	    gapcnt = 0.;
	    ix     = xpos-2;
	    match  = sub[ajSeqcvtGetCodeK(cvt,p[ypos])][ajSeqcvtGetCodeK(cvt,q[xpos])];
	    --ypos;
	    t = ix+1;
	    while(1)
	    {
		bimble = path[ypos*lenb+ix]-gapopen-(gapcnt*gapextend)+match;
		if(!ix || fabs((double)score-(double)bimble)< errbounds)
		    break;
		--ix;
		if(ix<0)
		    ajFatal("NW: Error walking left");
		++gapcnt;
	    }

	    for(ic=-1;ic<gapcnt;++ic)
	    {
		ajStrInsertK(m,0,'.');
		ajStrInsertK(n,0,q[t--]);
	    }
	    ajStrInsertK(n,0,q[t]);
	    ajStrInsertK(m,0,p[ypos]);

	    xpos = ix;
	    ajDebug("gapx  %5d %5d '%c' '%c' +%d\n",
		    ypos+1, xpos+1, p[ypos], q[t], 1+(int)gapcnt);
	    continue;
	}
	else if(compass[ypos*lenb+xpos]==2) /* Down, gap(s) in horizontal */
	{
	    score  = path[ypos*lenb+xpos];
	    gapcnt = 0.;
	    match  = sub[ajSeqcvtGetCodeK(cvt,p[ypos])][ajSeqcvtGetCodeK(cvt,q[xpos])];
	    --xpos;
	    iy = ypos-2;
	    t  = iy+1;

	    while(1)
	    {
		bimble = path[iy*lenb+xpos]-gapopen-(gapcnt*gapextend)+match;
		ajDebug("Down: iy: %d score:%.3f bimble:%.3f\n",
			iy, score, bimble);
		if(!iy || fabs((double)score-(double)bimble)< errbounds)
		    break;
		--iy;
		if(iy<0)
		    ajFatal("NW: Error walking down");
		++gapcnt;
	    }

	    for(ic=-1;ic<gapcnt;++ic)
	    {
		ajStrInsertK(n,0,'.');
		ajStrInsertK(m,0,p[t--]);
	    }
	    ajStrInsertK(m,0,p[t]);
	    ajStrInsertK(n,0,q[xpos]);
	    ypos = iy;
	    ajDebug("gapy  %5d %5d '%c' '%c' +%d\n",
		    ypos+1, xpos+1, p[t], q[xpos], 1+(int)gapcnt);
	    continue;
	}
	else
	    ajFatal("Walk Error in NW");

    }

    *start1 = ypos;
    *start2 = xpos;

    return;
}




/* @func embAlignPrintGlobal **************************************************
**
** Print a global alignment
** Nucleotides or proteins as needed.
**
** @param [u] outf [AjPFile] output stream
** @param [r] a [const char *] complete first sequence
** @param [r] b [const char *] complete second sequence
** @param [r] m [const AjPStr] Walk alignment for first sequence
** @param [r] n [const AjPStr] Walk alignment for second sequence
** @param [r] start1 [ajint] start of alignment in first sequence
** @param [r] start2 [ajint] start of alignment in second sequence
** @param [r] score [float] alignment score from AlignScoreX
** @param [r] mark [AjBool] mark matches and conservatives
** @param [r] sub [float * const *] substitution matrix
** @param [r] cvt [const AjPSeqCvt] conversion table for matrix
** @param [r] namea [const char *] name of first sequence
** @param [r] nameb [const char *] name of second sequence
** @param [r] begina [ajint] first sequence offset
** @param [r] beginb [ajint] second sequence offset
**
** @return [void]
******************************************************************************/

void embAlignPrintGlobal(AjPFile outf, const char *a, const char *b,
			 const AjPStr m, const AjPStr n,
			 ajint start1, ajint start2, float score, AjBool mark,
			 float * const *sub, const AjPSeqCvt cvt,
			 const char *namea,
			 const char *nameb, ajint begina, ajint beginb)
{
    AjPStr fa;
    AjPStr fb;
    AjPStr fm;
    AjPStr ap;
    AjPStr bp;
    AjPStr mp;

    ajint i;
    ajint nc;
    ajint olen;
    const char *p;
    const char *q;
    const char *r = NULL;

    float match = 0.0;

    ajint apos;
    ajint bpos;
    ajint alen;
    ajint blen;
    ajint acnt;
    ajint bcnt;
    ajint aend;
    ajint bend;

    ajint len;
    ajint pos;

    fa = ajStrNewC("");
    fb = ajStrNewC("");
    fm = ajStrNewC("");
    ap = ajStrNewC("");
    bp = ajStrNewC("");
    mp = ajStrNewC("");


    if(start1>start2)
    {
	for(i=0;i<start1;++i)
	{
	    ajStrAppendK(&fa,a[i]);
	    if(mark) ajStrAppendC(&fm," ");
	}
	nc=start1-start2;
	for(i=0;i<nc;++i)
	    ajStrAppendC(&fb," ");
	for(++nc;i<start1;++i)
	    ajStrAppendK(&fb,b[i-nc]);
    }
    else if(start2>start1)
    {
	for(i=0;i<start2;++i)
	{
	    ajStrAppendK(&fb,b[i]);
	    if(mark) ajStrAppendC(&fm," ");
	}
	nc=start2-start1;
	for(i=0;i<nc;++i)
	    ajStrAppendC(&fa," ");
	for(++nc;i<start2;++i)
	    ajStrAppendK(&fa,a[i-nc]);
    }

    len = ajStrGetLen(fa);

    /* Now deal with the alignment overlap */
    p = ajStrGetPtr(m);
    q = ajStrGetPtr(n);
    olen = (ajint) strlen(p);
    for(i=0;i<olen;++i)
    {
	ajStrAppendK(&fa,p[i]);
	ajStrAppendK(&fb,q[i]);
	if(mark)
	{
	    if(p[i]=='.' || q[i]=='.')
	    {
		ajStrAppendC(&fm," ");
		continue;
	    }
	    match = sub[ajSeqcvtGetCodeK(cvt,p[i])][ajSeqcvtGetCodeK(cvt,q[i])];
	    if(p[i]==q[i])
	    {
		ajStrAppendC(&fm,"|");
		continue;
	    }

	    if(match>0.0)
		ajStrAppendC(&fm,":");
	    else
		ajStrAppendC(&fm," ");
	}

    }
    /* Set pointers to sequence remainders */
    for(i=0,apos=start1,bpos=start2;i<olen;++i)
    {
	if(p[i]!='.')
	    ++apos;

	if(q[i]!='.')
	    ++bpos;
    }


    alen = (ajint) strlen(&a[apos]);
    blen = (ajint) strlen(&b[bpos]);

    if(alen>blen)
    {
	ajStrAppendC(&fa,&a[apos]);
	for(i=0;i<blen;++i)
	{
	    ajStrAppendK(&fb,b[bpos+i]);
	    if(mark)
		ajStrAppendC(&fm," ");
	}
	nc=alen-blen;

	for(i=0;i<nc;++i)
	{
	    ajStrAppendC(&fb," ");
	    if(mark)
		ajStrAppendC(&fm," ");
	}
    }
    else if(blen>alen)
    {
	ajStrAppendC(&fb,&b[bpos]);
	for(i=0;i<alen;++i)
	{
	    ajStrAppendK(&fa,a[apos+i]);
	    if(mark)
		ajStrAppendC(&fm," ");
	}
	nc=blen-alen;
	for(i=0;i<nc;++i)
	{
	    ajStrAppendC(&fa," ");
	    if(mark)
		ajStrAppendC(&fm," ");
	}
    }
    else
    {
	ajStrAppendC(&fa,&a[apos]);
	ajStrAppendC(&fb,&b[bpos]);
	if(mark)
	    for(i=0;i<alen;++i)
		ajStrAppendC(&fm," ");
    }

    /* Get start residues */
    p = ajStrGetPtr(fa);
    q = ajStrGetPtr(fb);
    for(i=0,acnt=start1,bcnt=start2;i<len;++i)
    {
	if(p[i]!=' ')
	    --acnt;
	if(q[i]!=' ')
	    --bcnt;
    }
    acnt += begina;
    bcnt += beginb;

    len = ajStrGetLen(fa);
    pos = 0;
    if(mark)
	r = ajStrGetPtr(fm);


    /* Add header stuff here */
    ajFmtPrintF(outf,"Global: %s vs %s\n",namea,nameb);
    ajFmtPrintF(outf,"Score: %.2f\n\n",score);

    while(pos<len)
    {
	if(pos+45 < len)
	{
	    ajStrAssignSubC(&ap,p,pos,pos+45-1);
	    ajStrAssignSubC(&bp,q,pos,pos+45-1);
	    if(mark)
		ajStrAssignSubC(&mp,r,pos,pos+45-1);
	    for(i=0,aend=acnt,bend=bcnt;i<45;++i)
	    {
		if(p[pos+i]!=' ' && p[pos+i]!='.')
		    ++aend;
		if(q[pos+i]!=' ' && q[pos+i]!='.')
		    ++bend;
	    }


	    ajFmtPrintF(outf,"%-15.15s ",namea);
	    if(aend!=acnt)
		ajFmtPrintF(outf,"%-8d ",acnt);
	    else
		ajFmtPrintF(outf,"         ");
	    ajFmtPrintF(outf,"%-45S ",ap);
	    if(aend!=acnt)
		ajFmtPrintF(outf,"%-8d\n",aend-1);
	    else
		ajFmtPrintF(outf,"\n");
	    acnt = aend;

	    if(mark)
		ajFmtPrintF(outf,"                         %S\n",mp);

	    ajFmtPrintF(outf,"%-15.15s ",nameb);
	    if(bend!=bcnt)
		ajFmtPrintF(outf,"%-8d ",bcnt);
	    else
		ajFmtPrintF(outf,"         ");
	    ajFmtPrintF(outf,"%-45S ",bp);
	    if(bend!=bcnt)
		ajFmtPrintF(outf,"%-8d\n",bend-1);
	    else
		ajFmtPrintF(outf,"\n");
	    bcnt = bend;

	    ajFmtPrintF(outf,"\n");
	    pos += 45;
	    continue;
	}

	ajStrAssignC(&ap,&p[pos]);
	ajStrAssignC(&bp,&q[pos]);
	if(mark)
	    ajStrAssignC(&mp,&r[pos]);
	for(i=0,aend=acnt,bend=bcnt;i<45 && p[pos+i];++i)
	{
	    if(p[pos+i]!=' ' && p[pos+i]!='.')
		++aend;
	    if(q[pos+i]!=' ' && q[pos+i]!='.')
		++bend;
	}


	ajFmtPrintF(outf,"%-15.15s ",namea);
	if(aend!=acnt)
	    ajFmtPrintF(outf,"%-8d ",acnt);
	else
	    ajFmtPrintF(outf,"         ");
	ajFmtPrintF(outf,"%-45S ",ap);
	if(aend!=acnt)
	    ajFmtPrintF(outf,"%-8d\n",aend-1);
	else
	    ajFmtPrintF(outf,"\n");
	acnt = aend;

	if(mark)
	    ajFmtPrintF(outf,"                         %S\n",mp);

	ajFmtPrintF(outf,"%-15.15s ",nameb);
	if(bend!=bcnt)
	    ajFmtPrintF(outf,"%-8d ",bcnt);
	else
	    ajFmtPrintF(outf,"         ");
	ajFmtPrintF(outf,"%-45S ",bp);
	if(bend!=bcnt)
	    ajFmtPrintF(outf,"%-8d\n",bend-1);
	else
	    ajFmtPrintF(outf,"\n");
	bcnt = bend;

	pos = len;
    }

    ajStrDel(&mp);
    ajStrDel(&bp);
    ajStrDel(&ap);
    ajStrDel(&fa);
    ajStrDel(&fb);
    ajStrDel(&fm);

    return;
}




/* @func embAlignPrintLocal ***************************************************
**
** Print a local alignment
** Nucleotides or proteins as needed.
**
** @param [u] outf [AjPFile] output stream
** @param [r] m [const AjPStr] Walk alignment for first sequence
** @param [r] n [const AjPStr] Walk alignment for second sequence
** @param [r] start1 [ajint] start of alignment in first sequence
** @param [r] start2 [ajint] start of alignment in second sequence
** @param [r] score [float] alignment score from AlignScoreX
** @param [r] mark [AjBool] mark matches and conservatives
** @param [r] sub [float * const *] substitution matrix
** @param [r] cvt [const AjPSeqCvt] conversion table for matrix
** @param [r] namea [const char*] name of first sequence
** @param [r] nameb [const char*] name of second sequence
** @param [r] begina [ajint] first sequence offset
** @param [r] beginb [ajint] second sequence offset
**
** @return [void]
******************************************************************************/

void embAlignPrintLocal(AjPFile outf,
			const AjPStr m, const AjPStr n,
			ajint start1, ajint start2, float score, AjBool mark,
			float * const *sub, const AjPSeqCvt cvt,
			const char *namea,
			const char *nameb, ajint begina, ajint beginb)
{
    const AjPStr fa;
    const AjPStr fb;
    AjPStr fm;
    AjPStr ap;
    AjPStr bp;
    AjPStr mp;

    ajint i;
    ajint olen;
    const char *p;
    const char *q;
    const char *r = NULL;

    float match = 0.0;

    ajint acnt;
    ajint bcnt;
    ajint aend;
    ajint bend;

    ajint len;
    ajint pos;

    fm = ajStrNewC("");
    ap = ajStrNewC("");
    bp = ajStrNewC("");
    mp = ajStrNewC("");


    /* Now deal with the alignment overlap */
    p    = ajStrGetPtr(m);
    q    = ajStrGetPtr(n);
    olen = (ajint) strlen(p);
    fa   = m;
    fb   = n;

    if(mark)
    {
	for(i=0;i<olen;++i)
	{
	    if(p[i]=='.' || q[i]=='.')
	    {
		ajStrAppendC(&fm," ");
		continue;
	    }
	    match = sub[ajSeqcvtGetCodeK(cvt,p[i])][ajSeqcvtGetCodeK(cvt,q[i])];
	    if(p[i]==q[i])
	    {
		ajStrAppendC(&fm,"|");
		continue;
	    }

	    if(match>0.0)
		ajStrAppendC(&fm,":");
	    else
		ajStrAppendC(&fm," ");
	}
    }

    /* Get start residues */
    p    = ajStrGetPtr(fa);
    q    = ajStrGetPtr(fb);
    acnt = begina+start1;
    bcnt = beginb+start2;

    len = ajStrGetLen(fa);
    pos = 0;
    if(mark) r=ajStrGetPtr(fm);


    /* Add header stuff here */
    ajFmtPrintF(outf,"Local: %s vs %s\n",namea,nameb);
    ajFmtPrintF(outf,"Score: %.2f\n\n",score);

    while(pos<len)
    {
	if(pos+45 < len)
	{
	    ajStrAssignSubC(&ap,p,pos,pos+45-1);
	    ajStrAssignSubC(&bp,q,pos,pos+45-1);
	    if(mark)
		ajStrAssignSubC(&mp,r,pos,pos+45-1);
	    for(i=0,aend=acnt,bend=bcnt;i<45;++i)
	    {
		if(p[pos+i]!=' ' && p[pos+i]!='.')
		    ++aend;
		if(q[pos+i]!=' ' && q[pos+i]!='.')
		    ++bend;
	    }


	    ajFmtPrintF(outf,"%-15.15s ",namea);
	    if(aend!=acnt)
		ajFmtPrintF(outf,"%-8d ",acnt);
	    else
		ajFmtPrintF(outf,"         ");
	    ajFmtPrintF(outf,"%-45S ",ap);
	    if(aend!=acnt)
		ajFmtPrintF(outf,"%-8d\n",aend-1);
	    else
		ajFmtPrintF(outf,"\n");
	    acnt = aend;

	    if(mark)
		ajFmtPrintF(outf,"                         %S\n",mp);

	    ajFmtPrintF(outf,"%-15.15s ",nameb);
	    if(bend!=bcnt)
		ajFmtPrintF(outf,"%-8d ",bcnt);
	    else
		ajFmtPrintF(outf,"         ");
	    ajFmtPrintF(outf,"%-45S ",bp);
	    if(bend!=bcnt)
		ajFmtPrintF(outf,"%-8d\n",bend-1);
	    else
		ajFmtPrintF(outf,"\n");
	    bcnt = bend;

	    ajFmtPrintF(outf,"\n");
	    pos += 45;
	    continue;
	}

	ajStrAssignC(&ap,&p[pos]);
	ajStrAssignC(&bp,&q[pos]);
	if(mark)
	    ajStrAssignC(&mp,&r[pos]);
	for(i=0,aend=acnt,bend=bcnt;i<45 && p[pos+i];++i)
	{
	    if(p[pos+i]!=' ' && p[pos+i]!='.')
		++aend;

	    if(q[pos+i]!=' ' && q[pos+i]!='.')
		++bend;
	}


	ajFmtPrintF(outf,"%-15.15s ",namea);
	if(aend!=acnt)
	    ajFmtPrintF(outf,"%-8d ",acnt);
	else
	    ajFmtPrintF(outf,"         ");
	ajFmtPrintF(outf,"%-45S ",ap);
	if(aend!=acnt)
	    ajFmtPrintF(outf,"%-8d\n",aend-1);
	else
	    ajFmtPrintF(outf,"\n");
	acnt = aend;

	if(mark)
	    ajFmtPrintF(outf,"                         %S\n",mp);

	ajFmtPrintF(outf,"%-15.15s ",nameb);
	if(bend!=bcnt)
	    ajFmtPrintF(outf,"%-8d ",bcnt);
	else
	    ajFmtPrintF(outf,"         ");
	ajFmtPrintF(outf,"%-45S ",bp);
	if(bend!=bcnt)
	    ajFmtPrintF(outf,"%-8d\n",bend-1);
	else
	    ajFmtPrintF(outf,"\n");
	bcnt = bend;

	pos = len;
    }

    ajStrDel(&mp);
    ajStrDel(&bp);
    ajStrDel(&ap);
    ajStrDel(&fm);

    return;
}




/* @func embAlignUnused *******************************************************
**
** Calls unused functions to avoid warning messages
**
** @return [void]
******************************************************************************/

void embAlignUnused(void)
{
    char *a    = NULL;
    char *b    = NULL;
    ajint lena = 0;
    ajint lenb = 0;

    float gapopen   = 0.0;
    float gapextend = 0.0;

    float *path   = NULL;
    float **sub   = NULL;
    AjPSeqCvt cvt = NULL;

    ajint *compass = NULL;

    AjBool show = 0;

    alignPathCalcOld(a,b,lena,lenb,gapopen,gapextend,path,sub,cvt,
		       compass,show);

    return;
}




/* @func embAlignPathCalcFast *************************************************
**
** Create path matrix for Smith-Waterman and Needleman-Wunsch
** Nucleotides or proteins as needed.
**
** @param [r] a [const char *] first sequence
** @param [r] b [const char *] second sequence
** @param [r] lena [ajint] length of first sequence
** @param [r] lenb [ajint] length of second sequence
** @param [r] gapopen [float] gap opening penalty
** @param [r] gapextend [float] gap extension penalty
** @param [w] path [float *] path matrix
** @param [r] sub [float * const *] substitution matrix from AjPMatrixf
** @param [r] cvt [const AjPSeqCvt] Conversion array for AjPMatrixf
** @param [w] compass [ajint *] Path direction pointer array
** @param [r] show [AjBool] Display path matrix
** @param [r] pathwidth [ajint] width of path matrix
**
** Optimised to keep a maximum value to avoid looping down or left
** to find the maximum. (il 29/07/99)
**
** Further speeded up by using only width calculations instead of lena.
**
** @return [void]
******************************************************************************/

void embAlignPathCalcFast(const char *a, const char *b, ajint lena, ajint lenb,
			  float gapopen, float gapextend,
			  float *path, float * const *sub, const AjPSeqCvt cvt,
			  ajint *compass, AjBool show, ajint pathwidth)
{
    ajint xpos;
    ajint i;
    ajint j;

    float match;
    float mscore;
    float fnew;
    float *maxa;
    float *maxb;

    ajint jlena;
    ajint jlenb;
    ajint width;

    float max;
    static AjPStr outstr = NULL;

    width = pathwidth;

    if(lena < width)
	width = lena;
    if(lenb < width)
	width = lenb;

    jlena = lena - 10;			/* for debug printout only */
    if(jlena < 0)
	jlena = lena-1;

    jlenb = lenb - 10;			/* for debug printout only */
    if(jlenb < 0)
	jlenb = lenb-1;

    /* ajDebug("embAlignPathCalcFast\n"); */

    /* ajDebug("lena: %d lenb: %d width: %d pathwidth: %d\n", lena, lenb,
	    width, pathwidth); */
    /* ajDebug("a: '%10.10s .. %10.10s' %d\n", a, &a[jlena], lena); */
    /* ajDebug("b: '%10.10s .. %10.10s' %d\n", b, &b[jlenb], lenb); */

    /* Create stores for the maximum values in a row or column */

    maxa = AJALLOC(lena*sizeof(float));
    maxb = AJALLOC(lenb*sizeof(float));

    /* First initialise the first column and row */
    for(i=0;i<lena;++i)
    {
	path[i*width] = sub[ajSeqcvtGetCodeK(cvt,a[i])][ajSeqcvtGetCodeK(cvt,b[0])];
	compass[i*width] = 0;
	/* ajDebug("CalcFast inita [%d] path: %.2f compass: %d\n",
		i*width, path[i*width], compass[i*width]); */
    }

    for(i=0;i<lena;++i)
	maxa[i] = path[i*width]-(gapopen);

    for(j=0;j<width;++j)
    {
	path[j] = sub[ajSeqcvtGetCodeK(cvt,a[0])][ajSeqcvtGetCodeK(cvt,b[j])];
	compass[j] = 0;
	/* ajDebug("CalcFast initb [%d] path: %.2f compass: %d\n",
		j, path[j], compass[j]); */
    }

    for(j=width;j<lenb;++j)
	maxb[j] = -1000.0;

    for(j=0;j<width;++j)
	maxb[j] = path[j]-(gapopen);

    /* ajDebug("2   %d %d\n",lena,lenb);*/

    /* now step through and build the path matrix */
    i = 1;
    while(i<lena)
    {
	xpos = 0;
	while(xpos<width && i+xpos < lenb)
	{
	    /* get match for current xpos/ypos */
	    match = sub[ajSeqcvtGetCodeK(cvt,a[i])][ajSeqcvtGetCodeK(cvt,b[i+xpos])];

	    /* Get diag score */
	    mscore = path[(i-1)*width+xpos] + match;
	    if(mscore < 0.0)
		mscore =0.0;

	    /* Set compass to diagonal value 0 */
	    compass[i*width+xpos] = 0;
	    path[i*width+xpos] = mscore;
	    /* ajDebug("CalcFast initc [%d] path: %.2f compass: %d\n",
		    i*width+xpos, path[i*width+xpos],
		    compass[i*width+xpos]); */

	    /* update the maximum against the previous point */
	    if(xpos > 0)
	    {
		fnew  = path[(i-1)*width+xpos-1];
		fnew -= gapopen;
		if(maxa[i-1] < fnew)
		    maxa[i-1] = fnew;
		else
		    maxa[i-1] -= gapextend;
	    }
	    if(i>1)
	    {
		if(xpos < width-1)
		{
		    fnew=path[(i-2)*width+xpos+1];
		    fnew-=gapopen;
		    if(fnew>maxb[i+xpos-1])
			maxb[i+xpos-1] = fnew;
		    else
			maxb[i+xpos-1] -= gapextend;
		}
		else
		    maxb[i+xpos-1] -= gapextend;
	    }

	    /* Now parade back along X axis */
	    if( maxa[i-1]+match > mscore)
	    {
		mscore = maxa[i-1]+match;
		path[i*width+xpos] = mscore;
		compass[i*width+xpos] = 1; /* Score comes from left */
		/* ajDebug("CalcFast initd [%d] path: %.2f compass: %d\n",
			i*width+xpos, path[i*width+xpos],
			compass[i*width+xpos]); */
	    }


	    /* And then bimble down Y axis */
	    if(maxb[i+xpos-1]+match > mscore)
	    {
		mscore = maxb[i+xpos-1]+match;
		path[i*width+xpos] = mscore;
		compass[i*width+xpos] = 2; /* Score comes from bottom */
		/* ajDebug("CalcFast inite [%d] path: %.2f compass: %d\n",
			i*width+xpos, path[i*width+xpos],
			compass[i*width+xpos]); */
	    }

	    xpos++;
	}
	++i;

    }

    max = -1000.0;
    if(show)
    {
	for(i=0;i<lena;++i)
	{
	    ajStrDelStatic(&outstr);
	    for(j=0;j<width;++j)
	    {
		ajFmtPrintAppS(&outstr,"%6.2f ",path[i*width+j]);
		if(path[i*width+j] > max)
		    max = path[i*width+j];
	    }
	    /* ajDebug("%S\n", outstr); */
	}
    }

    AJFREE(maxa);
    AJFREE(maxb);
    ajStrDelStatic(&outstr);

    return;
}




/* @func embAlignScoreSWMatrixFast ********************************************
**
** Walk down a matrix for Smith Waterman. Form aligned strings.
** Nucleotides or proteins as needed.
**
** @param [r] path [const float*] path matrix
** @param [r] compass [const ajint*] Path direction pointer array
** @param [r] gapopen [float] gap opening penalty
** @param [r] gapextend [float] gap extension penalty
** @param [r] a [const AjPSeq] first sequence
** @param [r] b [const AjPSeq] second sequence
** @param [r] lena [ajint] length of first sequence
** @param [r] lenb [ajint] length of second sequence
** @param [r] sub [float * const *] substitution matrix from AjPMatrixf
** @param [r] cvt [const AjPSeqCvt] Conversion array for AjPMatrixf
** @param [w] start1 [ajint *] start of alignment in first sequence
** @param [w] start2 [ajint *] start of alignment in second sequence
** @param [r] pathwidth [ajint] width of path matrix
**
** @return [float] Score of best matching segment
******************************************************************************/
float embAlignScoreSWMatrixFast(const float *path, const ajint *compass,
				float gapopen,
				float gapextend,
				const AjPSeq a, const AjPSeq b,
				ajint lena, ajint lenb,
				float * const *sub,
				const AjPSeqCvt cvt,
				ajint *start1, ajint *start2,
				ajint pathwidth)
{
    ajint i;
    ajint j;
    float pmax   = -1000.0;
    float score  = 0.;
    float wscore = 0.;
    float match  = 0.;
    float gapcnt = 0.;
    float bimble = 0.;

    ajint ix;
    ajint iy;
    ajint t;

    ajint xpos  = 0;
    ajint xpos2 = 0;
    ajint ypos  = 0;
    const char *p;
    const char *q;
    ajint width;

    ajDebug("embAlignScoreSWMatrixFast\n");

    width = pathwidth;
    if(lena < width)
	width = lena;
    if(lenb < width)
	width = lenb;


    /* ajDebug extra */
    ajDebug("SeqA '%S' %d %d\n", ajSeqGetNameS(a), ajSeqGetLen(a), lena);
    ajDebug("SeqB '%S' %d %d\n", ajSeqGetNameS(b), ajSeqGetLen(b), lenb);
    ajDebug("start1: %d start2: %d width: %d\n", *start1, *start2, width);

    pmax = (float) (-1*INT_MAX);
    for(i=0;i<lena;++i)
	for(j=0;j<width;++j)
	    if(path[i*width+j]>pmax)
	    {
		pmax = path[i*width+j];
		xpos = j;
		ypos = i;
		/*ajDebug("pmax %.2f xpos: %d ypos: %d path[%d]\n",
		  pmax, xpos, ypos, i*width+j);*/
	    }

    p = ajSeqGetSeqC(a);
    q = ajSeqGetSeqC(b);

    p += (*start1);
    q += (*start2);

    xpos2 = ypos+xpos;

    wscore = sub[ajSeqcvtGetCodeK(cvt,p[ypos])][ajSeqcvtGetCodeK(cvt,q[xpos2])];


    while(xpos>=0 && ypos && path[ypos*width+xpos] >0.)
    {
      /*ajDebug("(*) '%c' '%c' xpos: %d ypos: %d path[%d] %.2f\n",
	  p[xpos], q[ypos], xpos, ypos,
	  ypos*width+xpos, path[ypos*width+xpos]);*/

	if(!compass[ypos*width+xpos])	/* diagonal */
        {
	    if(path[(ypos-1)*width+xpos]<=0.0)
		break;
	    if(path[(ypos-1)*width+xpos] +
	       sub[ajSeqcvtGetCodeK(cvt,p[ypos])][ajSeqcvtGetCodeK(cvt,q[xpos2])]
	       != path[(ypos)*width+xpos])
	    {
		/* ajDebug("(*) '%c' '%c' '%c' "
			"xpos: %d xpos2: %d ypos: %d path[%d]"
			" %.2f != path[%d] %.2f + matrix %.2f\n",
			p[xpos], q[xpos2], q[ypos], xpos, xpos2, ypos,
			ypos*width+xpos, path[ypos*width+xpos],
			(ypos-1)*width+xpos, path[(ypos-1)*width+xpos],
		       sub[ajSeqcvtGetCodeK(cvt,p[ypos])][ajSeqcvtGetCodeK(cvt,q[xpos2])]);*/
		ajFatal("SW: Error walking match xpos: %d ypos: %d",
			xpos, ypos);
	    }

	    wscore += sub[ajSeqcvtGetCodeK(cvt,p[--ypos])][ajSeqcvtGetCodeK(cvt,q[--xpos2])];
	    continue;
        }

	else if(compass[ypos*width+xpos]==1) /* Left, gap(s) in vertical */
        {
	    score  = path[ypos*width+xpos];
	    gapcnt = 0.;
	    ix     = xpos-1;
	    match  = sub[ajSeqcvtGetCodeK(cvt,p[ypos])][ajSeqcvtGetCodeK(cvt,q[xpos2])];
	    --ypos;
	    t = xpos2-1;
	    while(1)
            {
	        /*ajDebug("(1) ypos: %d * %d + %d\n",
		  ypos, width, ix);*/
		/*ajDebug("(1) path[%d] = %.2f gapcnt: %.0f\n",
		ypos*width+ix, path[ypos*width+ix], gapcnt);*/

		bimble = path[ypos*width+ix]-gapopen-(gapcnt*gapextend)+match;

		/*ajDebug("(1) fabs(%.2f - %.2f) = %.2f\n",
		  score, bimble, fabs((double)score-(double)bimble));*/

		if(fabs((double)score-(double)bimble)<0.1)
		    break;
		--ix;
		if(ix<0)
		    ajFatal("SW: Error walking left");
		++gapcnt;
            }

	    if(score<=0.0)
		break;
	    t -= (ajint)gapcnt+1;
	    wscore += sub[ajSeqcvtGetCodeK(cvt,p[ypos])][ajSeqcvtGetCodeK(cvt,q[t])];
	    wscore -= (gapopen + (gapextend*gapcnt));

	    xpos2 = t;
	    xpos  = ix;
	    /*ajDebug("xpos => %d\n", xpos);*/
	    continue;
        }

	else if(compass[ypos*width+xpos]==2) /* Down, gap(s) in horizontal */
        {
	    score  = path[ypos*width+xpos];
	    gapcnt = 0.;
	    match  = sub[ajSeqcvtGetCodeK(cvt,p[ypos])][ajSeqcvtGetCodeK(cvt,q[xpos2])];
	    xpos++;
	    iy = ypos-2;
	    t  = iy+1;
	    while(1)
	    {
	        /*ajDebug("(2) %d * %d + xpos: %d\n",
		  iy, width, xpos);*/
		/*ajDebug("(2) path[%d] = %.2f gapcnt: %.0f\n",
		  iy*width+xpos, path[iy*width+xpos], gapcnt);*/

		bimble = path[iy*width+xpos]-gapopen-(gapcnt*gapextend)+match;

		/*ajDebug("(2) fabs(%.2f - %.2f) = %.2f\n",
		  score, bimble, fabs((double)score-(double)bimble));*/

		if(fabs((double)score-(double)bimble)<0.1)
		    break;
		--iy;
		++xpos;

		if(iy<0)
		{
		    ajDebug("SW: Error walking down %d < 0 gapcnt: %d\n",
		      iy, gapcnt);

		    ajFatal("SW: Error walking down");
		}
		++gapcnt;
            }

	    if(score<=0.0)
		break;
	    t -= (ajint)gapcnt;

	    wscore += sub[ajSeqcvtGetCodeK(cvt,p[t])][ajSeqcvtGetCodeK(cvt,q[xpos2])];
	    wscore -= (gapopen + (gapextend*gapcnt));


	    ypos = iy;
	    /*ajDebug("ypos => %d\n", ypos);*/
	    xpos2--;
	    continue;
        }
	else
	    ajFatal("Walk Error in SW");
    }

    return wscore;
}




/* @func embAlignWalkSWMatrixFast *********************************************
**
** Walk down a matrix for Smith Waterman. Form aligned strings.
** Nucleotides or proteins as needed.
**
** @param [r] path [const float*] path matrix
** @param [r] compass [const ajint*] Path direction pointer array
** @param [r] gapopen [float] gap opening penalty
** @param [r] gapextend [float] gap extension penalty
** @param [r] a [const AjPSeq] first sequence
** @param [r] b [const AjPSeq] second sequence
** @param [w] m [AjPStr *] alignment for first sequence
** @param [w] n [AjPStr *] alignment for second sequence
** @param [r] lena [ajint] length of first sequence
** @param [r] lenb [ajint] length of second sequence
** @param [r] sub [float * const *] substitution matrix from AjPMatrixf
** @param [r] cvt [const AjPSeqCvt] Conversion array for AjPMatrixf
** @param [w] start1 [ajint *] start of alignment in first sequence
** @param [w] start2 [ajint *] start of alignment in second sequence
** @param [r] pathwidth [ajint] width of path matrix
**
** @return [void]
******************************************************************************/

void embAlignWalkSWMatrixFast(const float *path, const ajint *compass,
			      float gapopen,
			      float gapextend,
			      const AjPSeq a, const AjPSeq b, AjPStr *m,
			      AjPStr *n, ajint lena, ajint lenb,
			      float * const *sub, const AjPSeqCvt cvt,
			      ajint *start1, ajint *start2,
			      ajint pathwidth)
{
    ajint i;
    ajint j;
    float pmax;
    float score;
    float match;
    float gapcnt;
    float bimble;

    ajint ix;
    ajint iy;
    ajint t;

    ajint xpos  = 0;
    ajint xpos2 = 0;
    ajint ypos  = 0;
    const char *p;
    const char *q;

    float ic;
    ajint width;

    ajDebug("embAlignWalkSWMatrixFast\n");

    width = pathwidth;

    if(lena < width)
	width = lena;
    if(lenb < width)
	width = lenb;

    /* Get maximum path score and save position */
    pmax = (float) (-1*INT_MAX);
    for(i=0;i<lena;++i)
	for(j=0;j<width;++j)
	    if(path[i*width+j]>pmax)
	    {
		pmax = path[i*width+j];
		xpos = j;
		ypos = i;
	    }

    p = ajSeqGetSeqC(a);
    q = ajSeqGetSeqC(b);
    p += (*start1);
    q += (*start2);

    xpos2 = xpos+ypos;

    ajStrAssignK(m,p[ypos]);
    ajStrAssignK(n,q[xpos2]);


    while(xpos>=0 && ypos && path[ypos*width+xpos] >0.)
    {

	if(!compass[ypos*width+xpos])	/* diagonal */
	{
	    if(path[(ypos-1)*width+xpos] +
	       sub[ajSeqcvtGetCodeK(cvt,p[ypos])][ajSeqcvtGetCodeK(cvt,q[xpos2])]
	       != path[(ypos)*width+xpos])
		ajFatal("SW: Error walking match");

	    if(path[(ypos-1)*width+xpos]<=0.0)
		break;
	    ajStrAppendK(m,p[--ypos]);
	    ajStrAppendK(n,q[--xpos2]);
	    continue;
	}
	else if(compass[ypos*width+xpos]==1) /* Left, gap(s) in vertical */
	{
	    score  = path[ypos*width+xpos];
	    gapcnt = 0.;
	    ix     = xpos-1;
	    match  = sub[ajSeqcvtGetCodeK(cvt,p[ypos])][ajSeqcvtGetCodeK(cvt,q[xpos2])];
	    --ypos;
	    t = xpos2-1;
	    while(1)
	    {
		bimble=path[ypos*width+ix]-gapopen-(gapcnt*gapextend)+match;
		if(fabs((double)score-(double)bimble)<0.1)
		    break;
		--ix;
		if(ix<0)
		    ajFatal("SW: Error walking left");

		++gapcnt;
	    }

	    if(score<=0.0)
		break;

	    for(ic=-1;ic<gapcnt;++ic)
	    {
		ajStrAppendK(m,'.');
		ajStrAppendK(n,q[t--]);
	    }
	    ajStrAppendK(n,q[t]);
	    ajStrAppendK(m,p[ypos]);

	    xpos2 = t;			/* change xpos2 */
	    xpos  = ix;
	    continue;
	}
	else if(compass[ypos*width+xpos]==2) /* Down, gap(s) in horizontal */
	{
	    score  = path[ypos*width+xpos];
	    gapcnt = 0.;
	    match  = sub[ajSeqcvtGetCodeK(cvt,p[ypos])][ajSeqcvtGetCodeK(cvt,q[xpos2])];
	    xpos++;
	    iy = ypos-2;
	    t  = iy+1;

	    while(1)
	    {
		bimble=path[iy*width+xpos]-gapopen-(gapcnt*gapextend)+match;
		if(fabs((double)score-(double)bimble)<0.1)
		    break;
		--iy;
		++xpos;
		if(iy<0)
		    ajFatal("SW: Error walking down");

		++gapcnt;
	    }

	    if(score<=0.0)
		break;
	    for(ic=-1;ic<gapcnt;++ic)
	    {
		ajStrAppendK(n,'.');
		ajStrAppendK(m,p[t--]);
	    }
	    ajStrAppendK(m,p[t]);
	    ajStrAppendK(n,q[--xpos2]);
	    ypos = iy;
	    continue;
	}
	else
	    ajFatal("Walk Error in SW");
    }

    ajStrReverse(m);
    ajStrReverse(n);

    *start1 += ypos;
    *start2 += xpos2;

    return;
}




/* @func embAlignProfilePathCalc  *********************************************
**
** Create path matrix for a profile
** Nucleotides or proteins as needed.
**
** @param [r] a [const char *] sequence
** @param [r] proflen [ajint] length of profile
** @param [r] seqlen [ajint] length of sequence
** @param [r] gapopen [float] gap opening coefficient
** @param [r] gapextend [float] gap extension coefficient
** @param [w] path [float *] path matrix
** @param [r] fmatrix [float * const *] profile matrix
** @param [w] compass [ajint *] Path direction pointer array
** @param [r] show [AjBool] Display path matrix
**
** @return [void]
******************************************************************************/

void embAlignProfilePathCalc(const char *a, ajint proflen, ajint seqlen,
			     float gapopen,
			     float gapextend, float *path,
			     float * const *fmatrix,
			     ajint *compass, AjBool show)
{
    ajint row;				/* profile position in path */
    ajint rowx;


    ajint column;		        /* sequence position in path */
    ajint columnx;

    float penalty;
    static AjPStr outstr = NULL;

    float score;
    float fmscore;
    float diagscore;
    float currmax;


    ajDebug("embAlignProfilePathCalc\n");

    /* First initialise the first column and row */
    for(column=0;column<seqlen;++column)
    {
	path[column] = fmatrix[0][ajAZToInt(a[column])];
	compass[column] = DIAG;
    }

    for(row=0;row<proflen;++row)
    {
	path[row*seqlen] = fmatrix[row][ajAZToInt(*a)];
	compass[row*seqlen] = DIAG;
    }


    /* diagonal steps start at 1 */
    column = 1;
    while(column!=seqlen)
    {
	for(row=1;row<proflen;++row)
	{
	    /* get match for current xpos/ypos */
	    fmscore = fmatrix[row][ajAZToInt(a[column])];

	    /* Get diag score */
	    diagscore = path[(row-1)*seqlen+(column-1)];

	    /* Initialise current maximum to diagonal score */
	    currmax = diagscore + fmscore;

	    /* Initialise compass to diagonal value */
	    compass[row*seqlen+column] = DIAG;
	    path[row*seqlen+column]    = currmax;

	    /* Now parade back along X axis */
	    if(column-2>-1)
	    {
		for(columnx=column-2;columnx>-1;--columnx)
		{
		    score = path[(row-1)*seqlen+columnx];
		    score += fmscore;
		    penalty  = -(fmatrix[(row-1)][GAPO] * gapopen +
				 ((column-columnx-2) * gapextend *
				  fmatrix[(row-1)][GAPE]));
		    score += penalty;


		    if(score>currmax)
		    {
			currmax=score;
			path[row*seqlen+column] = currmax;
			compass[row*seqlen+column] = LEFT;
		    }
		}
	    }

	    /* And then bimble down Y axis */
	    if(row-2>-1)
	    {
		for(rowx=row-2;rowx>-1;--rowx)
		{
		    score = path[rowx*seqlen+(column-1)];
		    score += fmscore;

		    penalty  = -(fmatrix[rowx][GAPO] * gapopen +
				 ((float) (row-rowx-2.)
				  * gapextend * fmatrix[rowx][GAPE]));
		    score += penalty;

		    if(score>currmax)
		    {
			currmax = score;
			path[row*seqlen+column] = currmax;
			compass[row*seqlen+column] = DOWN;
		    }
		}
	    }
	}

	++column;
    }





    if(show)
    {
	for(row=proflen-1;row>-1;--row)
	{
	    ajStrDelStatic(&outstr);
	    for(column=0;column<seqlen;++column)
		ajFmtPrintAppS(&outstr,"%6.2f ",
			       path[row*seqlen+column]);
	    /*ajDebug("%S\n", outstr);*/
	}
    }


    ajStrDelStatic(&outstr);
    return;
}




/* @func embAlignWalkProfileMatrix ********************************************
**
** Walk down a profile path matrix for Smith Waterman. Form aligned strings.
** Nucleotides or proteins as needed.
**
** @param [r] path [const float*] path matrix
** @param [r] compass [const ajint*] Path direction pointer array
** @param [r] gapopen [float] gap opening coeff
** @param [r] gapextend [float] gap extension coeff
** @param [r] cons [const AjPStr] consensus sequence
** @param [r] seq [const AjPStr] second sequence
** @param [w] m [AjPStr *] alignment for consensus sequence
** @param [w] n [AjPStr *] alignment for second sequence
** @param [r] proflen [ajint] length of consensus sequence
** @param [r] seqlen [ajint] length of test sequence
** @param [r] fmatrix [float * const *] profile
** @param [w] start1 [ajint *] start of alignment in consensus sequence
** @param [w] start2 [ajint *] start of alignment in test sequence
**
** @return [void]
******************************************************************************/

void embAlignWalkProfileMatrix(const float *path, const ajint *compass,
			       float gapopen,
			       float gapextend,
			       const AjPStr cons, const AjPStr seq,
			       AjPStr *m, AjPStr *n, ajint proflen,
			       ajint seqlen, float * const *fmatrix,
			       ajint *start1, ajint *start2)
{
    ajint i;
    float pathmax;
    float targetscore;
    float currscore;

    float match;
    ajint gapcnt;
    float penalty=0.;

    ajint row    = 0;
    ajint column = 0;

    ajint colstep;
    ajint rowstep;


    ajint direction = 0;

    ajint xpos = 0;
    ajint ypos = 0;

    const char *p;
    const char *q;

    float errbounds;

    ajDebug("embAlignWalkProfileMatrix\n");

    errbounds = (float) 0.01;

    /* Get maximum path score and save position */
    pathmax = -(float) INT_MAX;
    for(row=0;row<proflen;++row)
	for(column=0;column<seqlen;++column)
	    if(path[row*seqlen+column] > pathmax)
	    {
		pathmax = path[row*seqlen+column];
		xpos    = column;
		ypos    = row;
	    }


    column = xpos;
    row = ypos;

    p = ajStrGetPtr(cons);
    q = ajStrGetPtr(seq);

    ajStrAssignK(m,p[row]);
    ajStrAssignK(n,q[column]);

    while(row && column)
    {
	direction = compass[row*seqlen+column];
	if(direction == DIAG)
	{
	    if(path[(row-1)*seqlen+(column-1)]<0.)
		break;
	    ajStrInsertK(m,0,p[--row]);
	    ajStrInsertK(n,0,q[--column]);
	    continue;
	}
	else if(direction == LEFT)
	{
	    targetscore = path[row*seqlen+column];
	    gapcnt  = 0;
	    colstep = column-2;
	    currscore = -(float)INT_MAX;
	    match = fmatrix[row][ajAZToInt(q[column])];

	    while(fabs(targetscore-currscore) > errbounds)
	    {
		currscore = path[(row-1)*seqlen+colstep];
		penalty = -(fmatrix[row-1][GAPO] * gapopen +
			    fmatrix[row-1][GAPE] * (float)gapcnt * gapextend);
		currscore += penalty;
		currscore += match;

		++gapcnt;
		if(currscore-penalty < 0.)
		    break;

		--colstep;
	    }

	    for(i=0;i<gapcnt;++i)
	    {
		ajStrInsertK(m,0,'.');
		ajStrInsertK(n,0,q[--column]);
	    }

	    ajStrInsertK(n,0,q[--column]);
	    ajStrInsertK(m,0,p[--row]);

	    continue;
	}
	else if(direction == DOWN)
	{
	    targetscore = path[row*seqlen+column];
	    gapcnt  = 0;
	    rowstep = row-2;
	    currscore = -(float)(INT_MAX);
	    match = fmatrix[row][ajAZToInt(q[column])];

	    while(fabs(targetscore-currscore) > errbounds)
	    {
		currscore = path[rowstep*seqlen+(column-1)];
		penalty = -(fmatrix[rowstep][GAPO] * gapopen +
			    fmatrix[rowstep][GAPE] * (float)gapcnt *
			    gapextend);
		currscore += penalty;
		currscore += match;

		++gapcnt;

		if(currscore-penalty < 0.)
		    break;

		--rowstep;
	    }

	    for(i=0;i<gapcnt;++i)
	    {
		ajStrInsertK(n,0,'.');
		ajStrInsertK(m,0,p[--row]);
	    }

	    ajStrInsertK(m,0,p[--row]);
	    ajStrInsertK(n,0,q[column]);

	    continue;
	}
	else
	    ajFatal("Walk Error in Profile Walk");
    }

    *start1 = row;
    *start2 = column;

    return;
}




/* @func embAlignPrintProfile *************************************************
**
** Print a profile alignment
** Nucleotides or proteins as needed.
**
** @param [u] outf [AjPFile] output stream
** @param [r] m [const AjPStr] Walk alignment for first sequence
** @param [r] n [const AjPStr] Walk alignment for second sequence
** @param [r] start1 [ajint] start of alignment in first sequence
** @param [r] start2 [ajint] start of alignment in second sequence
** @param [r] score [float] alignment score from AlignScoreX
** @param [r] mark [AjBool] mark matches and conservatives
** @param [r] fmatrix [float * const *] profile
** @param [r] namea [const char *] name of first sequence
** @param [r] nameb [const char *] name of second sequence
** @param [r] begina [ajint] first sequence offset
** @param [r] beginb [ajint] second sequence offset
**
** @return [void]
******************************************************************************/

void embAlignPrintProfile(AjPFile outf,
			  const AjPStr m, const AjPStr n,
			  ajint start1, ajint start2, float score, AjBool mark,
			  float * const *fmatrix, const char *namea,
			  const char *nameb, ajint begina, ajint beginb)
{
    const AjPStr fa;
    const AjPStr fb;
    AjPStr fm;
    AjPStr ap;
    AjPStr bp;
    AjPStr mp;

    ajint i;
    ajint olen;
    const char *p;
    const char *q;
    const char *r = NULL;

    float match = 0.0;

    ajint acnt;
    ajint bcnt;
    ajint aend;
    ajint bend;

    ajint len;
    ajint pos;
    ajint cpos;

    fm = ajStrNewC("");
    ap = ajStrNewC("");
    bp = ajStrNewC("");
    mp = ajStrNewC("");


    /* Now deal with the alignment overlap */
    p    = ajStrGetPtr(m);
    q    = ajStrGetPtr(n);
    olen = (ajint) strlen(p);
    fa   = m;
    fb   = n;


    cpos = start1;
    --cpos;

    if(mark)
    {
	for(i=0;i<olen;++i)
	{
	    if(p[i]!='.')
		++cpos;
	    if(p[i]=='.' || q[i]=='.')
	    {
		ajStrAppendC(&fm," ");
		continue;
	    }
	    match=fmatrix[cpos][ajAZToInt(q[i])];

	    if(p[i]==q[i])
	    {
		ajStrAppendC(&fm,"|");
		continue;
	    }

	    if(match>0.0)
		ajStrAppendC(&fm,":");
	    else
		ajStrAppendC(&fm," ");
	}
    }

    /* Get start residues */
    p    = ajStrGetPtr(fa);
    q    = ajStrGetPtr(fb);
    acnt = begina+start1;
    bcnt = beginb+start2;

    len = ajStrGetLen(fa);
    pos = 0;
    if(mark)
	r = ajStrGetPtr(fm);


    /* Add header stuff here */
    ajFmtPrintF(outf,"Local: %s vs %s\n",namea,nameb);
    ajFmtPrintF(outf,"Score: %.2f\n\n",score);

    while(pos<len)
    {
	if(pos+45 < len)
	{
	    ajStrAssignSubC(&ap,p,pos,pos+45-1);
	    ajStrAssignSubC(&bp,q,pos,pos+45-1);
	    if(mark)
		ajStrAssignSubC(&mp,r,pos,pos+45-1);
	    for(i=0,aend=acnt,bend=bcnt;i<45;++i)
	    {
		if(p[pos+i]!=' ' && p[pos+i]!='.')
		    ++aend;

		if(q[pos+i]!=' ' && q[pos+i]!='.')
		    ++bend;
	    }


	    ajFmtPrintF(outf,"%-15.15s ",namea);
	    if(aend!=acnt)
		ajFmtPrintF(outf,"%-8d ",acnt);
	    else
		ajFmtPrintF(outf,"         ");
	    ajFmtPrintF(outf,"%-45S ",ap);
	    if(aend!=acnt)
		ajFmtPrintF(outf,"%-8d\n",aend-1);
	    else
		ajFmtPrintF(outf,"\n");
	    acnt = aend;

	    if(mark)
		ajFmtPrintF(outf,"                         %S\n",mp);

	    ajFmtPrintF(outf,"%-15.15s ",nameb);
	    if(bend!=bcnt)
		ajFmtPrintF(outf,"%-8d ",bcnt);
	    else
		ajFmtPrintF(outf,"         ");
	    ajFmtPrintF(outf,"%-45S ",bp);
	    if(bend!=bcnt)
		ajFmtPrintF(outf,"%-8d\n",bend-1);
	    else
		ajFmtPrintF(outf,"\n");
	    bcnt = bend;

	    ajFmtPrintF(outf,"\n");
	    pos += 45;
	    continue;
	}

	ajStrAssignC(&ap,&p[pos]);
	ajStrAssignC(&bp,&q[pos]);
	if(mark)
	    ajStrAssignC(&mp,&r[pos]);
	for(i=0,aend=acnt,bend=bcnt;i<45 && p[pos+i];++i)
	{
	    if(p[pos+i]!=' ' && p[pos+i]!='.')
		++aend;

	    if(q[pos+i]!=' ' && q[pos+i]!='.')
		++bend;
	}


	ajFmtPrintF(outf,"%-15.15s ",namea);
	if(aend!=acnt)
	    ajFmtPrintF(outf,"%-8d ",acnt);
	else
	    ajFmtPrintF(outf,"         ");
	ajFmtPrintF(outf,"%-45S ",ap);
	if(aend!=acnt)
	    ajFmtPrintF(outf,"%-8d\n",aend-1);
	else
	    ajFmtPrintF(outf,"\n");
	acnt = aend;

	if(mark)
	    ajFmtPrintF(outf,"                         %S\n",mp);

	ajFmtPrintF(outf,"%-15.15s ",nameb);
	if(bend!=bcnt)
	    ajFmtPrintF(outf,"%-8d ",bcnt);
	else
	    ajFmtPrintF(outf,"         ");
	ajFmtPrintF(outf,"%-45S ",bp);
	if(bend!=bcnt)
	    ajFmtPrintF(outf,"%-8d\n",bend-1);
	else
	    ajFmtPrintF(outf,"\n");
	bcnt = bend;

	pos = len;
    }

    ajStrDel(&mp);
    ajStrDel(&bp);
    ajStrDel(&ap);
    ajStrDel(&fm);

    return;
}




/* @func embAlignCalcSimilarity ***********************************************
**
** Calculate Similarity of two sequences (same length)
** Nucleotides or proteins as needed.
**
** @param [r] m [const AjPStr] Walk alignment for first sequence
** @param [r] n [const AjPStr] Walk alignment for second sequence
** @param [r] sub [float * const *] substitution matrix
** @param [r] cvt [const AjPSeqCvt] conversion table for matrix
** @param [r] lenm [ajint] length of first sequence
** @param [r] lenn [ajint] length of second sequence
** @param [w] id [float *] % identity
** @param [w] sim [float *] % similarity
** @param [w] idx [float *] % identity wrt longest sequence
** @param [w] simx [float *] % similarity wrt longest sequence
**
** @return [void]
******************************************************************************/

void embAlignCalcSimilarity(const AjPStr m, const AjPStr n,
			    float * const *sub, const AjPSeqCvt cvt,
			    ajint lenm, ajint lenn, float *id, float *sim,
			    float *idx, float *simx)
{
    ajint i;
    ajint olen;
    const char  *p    = NULL;
    const char  *q    = NULL;
    float match = 0.;
    ajint max;
    ajint gaps  = 0;

    AjPStr fm = NULL;
    AjPStr fn = NULL;

    ajStrAssignS(&fm, m);
    ajStrAssignS(&fn, n);
    ajStrFmtUpper(&fm);
    ajStrFmtUpper(&fn);

    p = ajStrGetPtr(fm);
    q = ajStrGetPtr(fn);
    olen = (ajint) strlen(p);


    *id = *sim = 0.;


    for(i=0;i<olen;++i)
    {
	if(p[i] =='.' || q[i]=='.')
	{
	    ++gaps;
	    continue;
	}

	match = sub[ajSeqcvtGetCodeK(cvt,toupper((int)p[i]))]
	           [ajSeqcvtGetCodeK(cvt,toupper((int)q[i]))];
	if(p[i]==q[i])
	{
	    ++(*id);
	    ++(*sim);
	    continue;
	}

	if(match>0.0)
	    ++(*sim);
    }

    max = (lenm>lenn) ? lenm : lenn;
    
    *idx  = *id / (float)max * (float)100.;
    *simx = *sim / (float)max * (float)100.;
    *id   *= ((float)100. / (float)(olen-gaps));
    *sim  *= ((float)100. / (float)(olen-gaps));


    ajStrDel(&fm);
    ajStrDel(&fn);
    return;
}




/* @func embAlignScoreProfileMatrix *******************************************
**
** Score a profile path matrix for Smith Waterman.
** Nucleotides or proteins as needed.
**
** @param [r] path [const float*] path matrix
** @param [r] compass [const ajint*] Path direction pointer array
** @param [r] gapopen [float] gap opening coeff
** @param [r] gapextend [float] gap extension coeff
** @param [r] seq [const AjPStr] second sequence
** @param [r] proflen [ajint] length of consensus sequence
** @param [r] seqlen [ajint] length of test sequence
** @param [r] fmatrix [float * const *] profile
** @param [w] start1 [ajint *] start of alignment in consensus sequence
** @param [w] start2 [ajint *] start of alignment in test sequence
**
** @return [float] profile alignment score
******************************************************************************/

float embAlignScoreProfileMatrix(const float *path, const ajint *compass,
				 float gapopen,
				 float gapextend, const AjPStr seq,
				 ajint proflen, ajint seqlen,
				 float * const *fmatrix,
				 ajint *start1, ajint *start2)
{
    ajint i;
    float pathmax;
    float targetscore;
    float currscore;
    float wscore=0.;

    float match;
    ajint gapcnt;
    float penalty = 0.;

    ajint row    = 0;
    ajint column = 0;

    ajint colstep;
    ajint rowstep;


    ajint direction = 0;

    ajint xpos = 0;
    ajint ypos = 0;

    const char *q;

    float errbounds;

    ajDebug("embAlignWalkProfileMatrix\n");

    errbounds = (float) 0.01;

    /* Get maximum path score and save position */
    pathmax = -(float) INT_MAX;
    for(row=0;row<proflen;++row)
	for(column=0;column<seqlen;++column)
	    if(path[row*seqlen+column] > pathmax)
	    {
		pathmax = path[row*seqlen+column];
		xpos    = column;
		ypos    = row;
	    }


    column = xpos;
    row    = ypos;

    q = ajStrGetPtr(seq);

    wscore = fmatrix[row][ajAZToInt(q[column])];

    while(row && column)
    {
	direction = compass[row*seqlen+column];
	if(direction == DIAG)
	{
	    if(path[(row-1)*seqlen+(column-1)]<0.)
		break;
	    wscore += fmatrix[--row][ajAZToInt(q[--column])];
	    continue;
	}
	else if(direction == LEFT)
	{
	    targetscore = path[row*seqlen+column];
	    gapcnt = 0;
	    colstep = column-2;
	    currscore = -(float)INT_MAX;
	    match = fmatrix[row][ajAZToInt(q[column])];

	    while(fabs(targetscore-currscore) > errbounds)
	    {
		currscore = path[(row-1)*seqlen+colstep];
		penalty = -(fmatrix[row-1][GAPO] * gapopen +
			    fmatrix[row-1][GAPE] * (float)gapcnt * gapextend);
		currscore += penalty;
		currscore += match;

		++gapcnt;
		if(currscore-penalty < 0.)
		    break;

		--colstep;
	    }


	    for(i=0;i<gapcnt;++i)
		--column;

	    wscore += fmatrix[--row][ajAZToInt(q[--column])];

	    continue;
	}
	else if(direction == DOWN)
	{
	    targetscore = path[row*seqlen+column];
	    gapcnt = 0;
	    rowstep = row-2;
	    currscore = -(float)(INT_MAX);
	    match = fmatrix[row][ajAZToInt(q[column])];

	    while(fabs(targetscore-currscore) > errbounds)
	    {
		currscore = path[rowstep*seqlen+(column-1)];
		penalty = -(fmatrix[rowstep][GAPO] * gapopen +
			    fmatrix[rowstep][GAPE] * (float)gapcnt *
			    gapextend);
		currscore += penalty;
		currscore += match;

		++gapcnt;

		if(currscore-penalty < 0.)
		    break;

		--rowstep;
	    }

	    for(i=0;i<gapcnt;++i)
		--row;

	    wscore += fmatrix[--row][ajAZToInt(q[--column])];

	    continue;
	}
	else
	    ajFatal("Walk Error in Profile Score");
    }

    *start1 = row;
    *start2 = column;

    return wscore;
}




/* @func embAlignReportGlobal *************************************************
**
** Print a global alignment
** Nucleotides or proteins as needed.
**
** @param [u] align [AjPAlign] Alignment object
** @param [r] seqa [const AjPSeq] Complete first sequence
** @param [r] seqb [const AjPSeq] Complete second sequence
** @param [r] m [const AjPStr] Walk alignment for first sequence
** @param [r] n [const AjPStr] Walk alignment for second sequence
** @param [r] start1 [ajint] start of alignment in first sequence
** @param [r] start2 [ajint] start of alignment in second sequence
** @param [r] gapopen [float] Gap open penalty to report
** @param [r] gapextend [float] Gap extend penalty to report
** @param [r] score [float] alignment score from AlignScoreX
** @param [u] matrix [AjPMatrixf] Floating point matrix
** @param [r] offset1 [ajint] first sequence offset
** @param [r] offset2 [ajint] second sequence offset
**
** @return [void]
******************************************************************************/

void embAlignReportGlobal(AjPAlign align,
			  const AjPSeq seqa, const AjPSeq seqb,
			  const AjPStr m, const AjPStr n,
			  ajint start1, ajint start2,
			  float gapopen, float gapextend,
			  float score, AjPMatrixf matrix,
			  ajint offset1, ajint offset2)
{
    AjPSeq res1 = NULL;
    AjPSeq res2 = NULL;
/*
    ajint end1;
    ajint end2;
*/
    AjPStr fa = NULL;
    AjPStr fb = NULL;
    ajint maxlen;
    ajint i;
    ajint alen;
    ajint blen;
    ajint apos;
    ajint bpos;
    ajint nc;
    const char* a;
    const char* b;

    maxlen = AJMAX(ajStrGetLen(m), ajStrGetLen(n));
    ajDebug("embAlignReportGlobal %d %d\n", start1, start2);
    ajDebug("  start1:%d start2:%d offset1:%d offset2:%d\n",
	    start1, start2, offset1, offset2);

    a = ajSeqGetSeqC(seqa);
    b = ajSeqGetSeqC(seqb);

    /* generate the full aligned sequences */

    ajStrSetRes(&fa, maxlen);
    ajStrSetRes(&fb, maxlen);

    /* pad the start of either sequence */

    if(start1>start2)
    {
	for(i=0;i<start1;++i)
	    ajStrAppendK(&fa,a[i]);

	nc = start1-start2;
	for(i=0;i<nc;++i)
	    ajStrAppendK(&fb,' ');

	ajDebug("start1>start2 start a: seqa 1..%d b: %d spaces seqb 1..%d\n",
		start1, nc, start1-nc);
	for(++nc;i<start1;++i)
	    ajStrAppendK(&fb,b[i-nc]);
    }
    else if(start2>start1)
    {
	for(i=0;i<start2;++i)
	    ajStrAppendK(&fb,b[i]);

	nc = start2-start1;
	for(i=0;i<nc;++i)
	    ajStrAppendK(&fa,' ');

	ajDebug("start1<start2 start a: %d spaces seqb 1..%d b: seqa 1..%d \n",
		 nc, start1-nc, start1);
	for(++nc;i<start2;++i)
	    ajStrAppendK(&fa,a[i-nc]);
    }

    apos = start1 + ajStrGetLen(m) - ajSeqstrCountGaps(m);
    bpos = start2 + ajStrGetLen(n) - ajSeqstrCountGaps(n);

    ajStrAppendS(&fa, m);
    ajStrAppendS(&fb, n);

    ajDebug("append alignment len (ungapped) a: %d (%d) b: %d (%d)\n",
	    ajStrGetLen(m), ajStrGetLen(m) - ajSeqstrCountGaps(m),
	    ajStrGetLen(n), ajStrGetLen(n) - ajSeqstrCountGaps(n));

    alen=ajSeqGetLen(seqa) - apos;
    blen=ajSeqGetLen(seqb) - bpos;

    ajDebug("alen: %d blen: %d apos: %d bpos: %d\n", alen, blen, apos, bpos);

    /* adding gaps at the ends */

    if(alen>blen)
    {
	ajStrAppendC(&fa,&a[apos]);
	for(i=0;i<blen;++i)
	    ajStrAppendK(&fb,b[bpos+i]);

	nc = alen-blen;
	for(i=0;i<nc;++i)
	    ajStrAppendC(&fb," ");
    }
    else if(blen>alen)
    {
	ajStrAppendC(&fb,&b[bpos]);
	for(i=0;i<alen;++i)
	    ajStrAppendK(&fa,a[apos+i]);

	nc = blen-alen;
	for(i=0;i<nc;++i)
	    ajStrAppendC(&fa," ");
    }
    else				/* same length, just copy */
    {
	ajStrAppendC(&fa,&a[apos]);
	ajStrAppendC(&fb,&b[bpos]);
    }

    ajDebug("  res1: %5d '%S' \n", ajStrGetLen(fa), fa);
    ajDebug("  res2: %5d '%S' \n", ajStrGetLen(fb), fb);
    maxlen = AJMAX(ajSeqGetLen(seqa), ajSeqGetLen(seqb));

    res1   = ajSeqNewRangeC(ajStrGetPtr(fa), ajSeqGetOffset(seqa),
			     ajSeqGetOffend(seqa), ajSeqIsReversed(seqa));
    ajSeqAssignNameS(res1, ajSeqGetNameS(seqa));
    ajSeqAssignUsaS(res1, ajSeqGetUsaS(seqa));
    ajSeqAssignAccS(res1, ajSeqGetAccS(seqa));
    ajSeqAssignDescS(res1, ajSeqGetDescS(seqa));

    res2   = ajSeqNewRangeC(ajStrGetPtr(fb), ajSeqGetOffset(seqb),
			     ajSeqGetOffend(seqb), ajSeqIsReversed(seqb));
    ajSeqAssignNameS(res2, ajSeqGetNameS(seqb));
    ajSeqAssignUsaS(res2, ajSeqGetUsaS(seqb));
    ajSeqAssignAccS(res2, ajSeqGetAccS(seqb));
    ajSeqAssignDescS(res2, ajSeqGetDescS(seqb));

    ajSeqGapStandard(res1, '-');
    ajSeqGapStandard(res2, '-');

    ajAlignDefineSS(align, res1, res2);

    ajAlignSetGapR(align, gapopen, gapextend);
    ajAlignSetScoreR(align, score);
    ajAlignSetMatrixFloat(align, matrix);
/*
    end1 = start1 - ajStrCalcCountK(m, '-') + ajStrGetLen(m);
    end2 = start2 - ajStrCalcCountK(n, '-') + ajStrGetLen(n);
*/
    /* ajAlignSetRange(align, start1+1, end1+1, start2+1, end2);*/
    /*
    ajAlignSetRange(align,
		    start1, end1-1,
                    ajSeqGetOffset(seqa)+ajSeqGetLen(res1)+ajSeqGetOffend(seqa)
                           -ajSeqCountGaps(res1),
                    offset1,
		    start2, end2-1,
                    ajSeqGetOffset(seqb)+ajSeqGetLen(res1)+ajSeqGetOffend(seqb)
                           -ajSeqCountGaps(res1),
                    offset2);
*/
    ajStrDel(&fa);
    ajStrDel(&fb);
    ajSeqDel(&res1);
    ajSeqDel(&res2);

    return;
}




/* @func embAlignReportLocal **************************************************
**
** Print a local alignment
** Nucleotides or proteins as needed.
**
** @param [u] align [AjPAlign] Alignment object
** @param [r] seqa [const AjPSeq] complete first sequence
** @param [r] seqb [const AjPSeq] complete second sequence
** @param [r] m [const AjPStr] Walk alignment for first sequence
** @param [r] n [const AjPStr] Walk alignment for second sequence
** @param [r] start1 [ajint] start of alignment in first sequence
** @param [r] start2 [ajint] start of alignment in second sequence
** @param [r] gapopen [float] Gap open penalty to report
** @param [r] gapextend [float] Gap extend penalty to report
** @param [r] score [float] alignment score from AlignScoreX
** @param [u] matrix [AjPMatrixf] Floating point matrix
** @param [r] offset1 [ajint] first sequence offset
** @param [r] offset2 [ajint] second sequence offset
**
** @return [void]
******************************************************************************/

void embAlignReportLocal(AjPAlign align,
			 const AjPSeq seqa, const AjPSeq seqb,
			 const AjPStr m, const AjPStr n,
			 ajint start1, ajint start2,
			 float gapopen, float gapextend,
			 float score, AjPMatrixf matrix,
			 ajint offset1, ajint offset2)
{
    AjPSeq res1 = NULL;
    AjPSeq res2 = NULL;
    ajint offend1;
    ajint offend2;

    offend1 = ajSeqGetLen(seqa) - ajStrGetLen(m) - start1
	+ ajStrCalcCountK(m, '.')
	+ ajStrCalcCountK(m, '-')
	    + ajStrCalcCountK(m, ' ');
    offend2 = ajSeqGetLen(seqb) - ajStrGetLen(n) - start2
	+ ajStrCalcCountK(n, '.')
	+ ajStrCalcCountK(n, '-')
	    + ajStrCalcCountK(n, ' ');
    ajDebug("embAlignReportLocal start: %d %d offset: %d %d offend: %d %d "
	    "len:%d %d seqlen: %d %d Offset:%d %d Offend:%d %d\n",
	    start1, start2,
	    offset1, offset2,
	    offend1, offend2,
	    ajStrGetLen(m), ajStrGetLen(n),
	    ajSeqGetLen(seqa), ajSeqGetLen(seqb),
	    ajSeqGetOffset(seqa), ajSeqGetOffset(seqb),
	    ajSeqGetOffend(seqa), ajSeqGetOffend(seqb));

    res1   = ajSeqNewRangeC(ajStrGetPtr(m), start1+ajSeqGetOffset(seqa),
			     offend1+ajSeqGetOffend(seqa),
			     ajSeqIsReversed(seqa));
    ajSeqAssignNameS(res1, ajSeqGetNameS(seqa));
    ajSeqAssignUsaS(res1, ajSeqGetUsaS(seqa));
    ajSeqAssignAccS(res1, ajSeqGetAccS(seqa));
    ajSeqAssignDescS(res1, ajSeqGetDescS(seqa));

    res2   = ajSeqNewRangeC(ajStrGetPtr(n), start2+ajSeqGetOffset(seqb),
			     offend2+ajSeqGetOffend(seqb),
			     ajSeqIsReversed(seqb));
    ajSeqAssignNameS(res2, ajSeqGetNameS(seqb));
    ajSeqAssignUsaS(res2, ajSeqGetUsaS(seqb));
    ajSeqAssignAccS(res2, ajSeqGetAccS(seqb));
    ajSeqAssignDescS(res2, ajSeqGetDescS(seqb));

    ajSeqGapStandard(res1, '-');
    ajSeqGapStandard(res2, '-');

    ajAlignDefineSS(align, res1, res2);

    ajAlignSetGapR(align, gapopen, gapextend);
    ajAlignSetScoreR(align, score);
    ajAlignSetMatrixFloat(align, matrix);

    ajSeqDel(&res1);
    ajSeqDel(&res2);

    return;
}




/* @func embAlignReportProfile ************************************************
**
** Print a profile alignment
** Nucleotides or proteins as needed.
**
** @param [u] align [AjPAlign] Alignment object
** @param [r] m [const AjPStr] Walk alignment for first sequence
** @param [r] n [const AjPStr] Walk alignment for second sequence
** @param [r] start1 [ajint] start of alignment in first sequence
** @param [r] start2 [ajint] start of alignment in second sequence
** @param [r] score [float] alignment score from AlignScoreX
** @param [r] namea [const char *] name of first sequence
** @param [r] nameb [const char *] name of second sequence
**
** @return [void]
******************************************************************************/

void embAlignReportProfile(AjPAlign align,
			   const AjPStr m, const AjPStr n,
			   ajint start1, ajint start2,
			   float score,
			   const char *namea, const char *nameb)
{
    AjPSeq seqm = NULL;
    AjPSeq seqn = NULL;
    AjPStr seqname = NULL;

    ajuint mlen = ajStrGetLen(m);
    ajuint slen = ajStrGetLen(n);

    ajStrAssignC(&seqname, namea);
    seqm=ajSeqNewNameS(m, seqname);

    ajStrAssignC(&seqname, nameb);
    seqn=ajSeqNewNameS(n, seqname); 

    ajAlignDefineSS(align, seqm, seqn);
    ajAlignSetScoreR(align, score);
    ajAlignSetRange(align, 
		    start1, start1+mlen-1, mlen, 1,
		    start2, start2+slen-1, slen, 1);

    ajSeqDel(&seqm);
    ajSeqDel(&seqn);
    ajStrDel(&seqname);

    return;
}

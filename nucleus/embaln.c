/* @source embaln *************************************************************
**
** General routines for alignment.
**
** @author Copyright (c) 1999 Alan Bleasby
** @version $Revision: 1.102 $
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

#include "embaln.h"
#include "ajmath.h"
#include "ajseq.h"
#include "ajalign.h"
#include "ajfile.h"
#include "ajutil.h"
#include "ajbase.h"

#include <string.h>
#include <limits.h>
#include <float.h>
#include <math.h>



#define GAPO 26
#define GAPE 27

#define DIAG 0
#define LEFT 1
#define DOWN 2




static void printPathMatrix(const float* path, const ajint* compass,
	const char *a, const char *b, ajuint lena, ajuint lenb);

static float embAlignGetScoreNWMatrix(
	const float *ix, const float *iy, const float *m, ajint lena,
        ajint lenb, ajint *xpos, ajint *ypos, AjBool endweight);




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
** @return [float] Maximum score
**
** @release 1.0.0
** @@
** Optimised to keep a maximum value to avoid looping down or left
** to find the maximum. (il 29/07/99)
**
******************************************************************************/

float embAlignPathCalc(const char *a, const char *b,
                       ajint lena, ajint lenb, float gapopen,
                       float gapextend, float *path,
                       float * const *sub, const AjPSeqCvt cvt,
                       ajint *compass, AjBool show)
{
    ajint xpos;
    ajint ypos;
    ajint i;
    ajint j;

    double match;
    double mscore;
    double fnew;
    double *maxa;
    double maxb;

    static AjPStr outstr = NULL;
    char compasschar;

    float ret = -FLT_MAX;

    ajDebug("embAlignPathCalc\n");

    /* Create stores for the maximum values in a row or column */

    maxa = AJALLOC(lena*sizeof(double));


    /* First initialise the first column and row */
    for(i=0;i<lena;++i)
    {
	path[i*lenb] = sub[ajSeqcvtGetCodeK(cvt,a[i])]
                          [ajSeqcvtGetCodeK(cvt,b[0])];
	compass[i*lenb] = 0;
    }

    for(i=0;i<lena;++i)
    {
	maxa[i] = path[i*lenb]-(gapopen);
    }


    for(j=0;j<lenb;++j)
    {
	path[j] = sub[ajSeqcvtGetCodeK(cvt,a[0])]
                     [ajSeqcvtGetCodeK(cvt,b[j])];
	compass[j] = 0;
    }


    /* xpos and ypos are the diagonal steps so start at 1 */
    xpos = 1;

    while(xpos!=lenb)
    {
	ypos  = 1;
	maxb = path[xpos]-(gapopen);

	while(ypos < lena)
	{
	    /* get match for current xpos/ypos */
	    match = sub[ajSeqcvtGetCodeK(cvt,a[ypos])]
                       [ajSeqcvtGetCodeK(cvt,b[xpos])];

	    /* Get diag score */
	    mscore = path[(ypos-1)*lenb+xpos-1] + match;

            /* ajDebug("match %d:%d %c:%c %d %6.2f ",
               xpos, ypos, a[ypos], b[xpos], ypos*lenb+xpos,mscore); */

	    /* Set compass to diagonal value 0 */
	    compass[ypos*lenb+xpos] = 0;
	    path[ypos*lenb+xpos] = (float) mscore;

	    /* Now parade back along X axis */
            maxa[ypos] -= gapextend;
            fnew=path[(ypos)*lenb+xpos-1];
            fnew-=gapopen;

            if(fnew > maxa[ypos])
                maxa[ypos] = fnew;

            if( maxa[ypos] > mscore)
            {
                mscore = maxa[ypos];
                path[ypos*lenb+xpos] = (float) mscore;
                compass[ypos*lenb+xpos] = LEFT; /* Score comes from left */
            }

	    /* And then bimble down Y axis */
            maxb -= gapextend;
            fnew = path[(ypos-1)*lenb+xpos];
            fnew-=gapopen;

            if(fnew > maxb)
                maxb = fnew;

            if(maxb > mscore)
            {
                mscore = maxb;
                path[ypos*lenb+xpos] = (float) mscore;
                compass[ypos*lenb+xpos] = DOWN; /* Score comes from bottom */
            }

            /*ajDebug("\n");*/
 
	    ypos++;
	}
	++xpos;

    }
    
    for(i=0;i<lenb;++i)
	if(path[(lena-1)*lenb+i]>ret)
	    ret = path[(lena-1)*lenb+i];

    for(j=0;j<lena;++j)
	if(path[j*lenb+lenb-1]>ret)
	    ret=path[j*lenb+lenb-1];

    if(show)
    {
	for(i=lena-1;i>-1;--i)
	{
            ajFmtPrintS(&outstr, "%6d ", i);

	    for(j=0;j<lenb;++j)
            {
                if(compass[i*lenb+j] == LEFT)
                    compasschar = '<';
                else if(compass[i*lenb+j] == DOWN)
                    compasschar = 'v';
                else
                    compasschar = ' ';

		ajFmtPrintAppS(&outstr, "%6.2f%c ",
                               path[i*lenb+j],compasschar);
            }

	    ajDebug("%S\n", outstr);
	}

        ajFmtPrintS(&outstr, "       ");

        for(j=0;j<lenb;++j)
            ajFmtPrintAppS(&outstr, "%6d  ", j);

        ajDebug("%S\n", outstr);
    }

    AJFREE(maxa);

    ajStrDelStatic(&outstr);

    return ret;
}




/* @func embAlignPathCalcWithEndGapPenalties **********************************
**
** Create path matrix for Needleman-Wunsch alignment of two sequences.
** Nucleotides or proteins as needed. Supports end gap penalties.
**
** @param [r] a [const char *] first sequence
** @param [r] b [const char *] second sequence
** @param [r] lena [ajint] length of first sequence
** @param [r] lenb [ajint] length of second sequence
** @param [r] gapopen [float] gap opening penalty
** @param [r] gapextend [float] gap extension penalty
** @param [r] endgapopen [float] end gap opening penalty
** @param [r] endgapextend [float] end gap extension penalty
** @param [w] start1 [ajint *] start of alignment in first sequence
** @param [w] start2 [ajint *] start of alignment in second sequence
** @param [r] sub [float * const *] substitution matrix from AjPMatrixf
** @param [r] cvt [const AjPSeqCvt] Conversion array for AjPMatrixf
** @param [w] m [float*] Match scores array, m(i,j) is the best score
**                       up to (i,j) given that a(i) is aligned to b(j)
** @param [w] ix [float*] Gap scores array, ix(i,j) is the best score
**                        given that a(i) is aligned to a gap
**                        (in an insertion with respect to b)
** @param [w] iy [float*] Gap scores array, iy(i,j) is the best score
**                        given that b(i) is in an insertion with respect to a
** @param [w] compass [ajint *] Path direction pointer array
** @param [r] show [AjBool] Display path matrix
** @param [r] endweight [AjBool] Use end gap weights
**
** @return [float] Score
**
** @release 6.2.0
** @@
**
******************************************************************************/

float embAlignPathCalcWithEndGapPenalties(const char *a, const char *b,
                       ajint lena, ajint lenb,
                       float gapopen, float gapextend,
                       float endgapopen, float endgapextend,
                       ajint *start1, ajint *start2,
                       float * const *sub, const AjPSeqCvt cvt,
                       float *m, float *ix, float *iy,
                       ajint *compass, AjBool show,
                       AjBool endweight)
{
    ajint xpos;
    ajint ypos;
    ajint bconvcode;

    float match;
    float ixp;
    float iyp;
    float mp;
    ajint cursor;
    ajint cursorp;
    
    float testog;
    float testeg;
    float score;
    
    ajDebug("embAlignPathCalcWithEndGapPenalties\n");
    
    if (!endweight)
    {
	endgapopen=0;
	endgapextend=0;
	endweight=ajTrue;
    }

    match = sub[ajSeqcvtGetCodeK(cvt, a[0])][ajSeqcvtGetCodeK(cvt, b[0])];

    ix[0] = -endgapopen-gapopen;
    iy[0] = -endgapopen-gapopen;
    m[0] = match;

    cursor =0;

    /* First initialise the first column */
    for (ypos = 1; ypos < lena; ++ypos)
    {
	match = sub[ajSeqcvtGetCodeK(cvt,a[ypos])][ajSeqcvtGetCodeK(cvt,b[0])];
	cursor = ypos * lenb;
	cursorp = (ypos-1) * lenb;

	testog = m[cursorp] - gapopen;
	testeg = iy[cursorp] - gapextend;

	if(testog >= testeg)
	    iy[cursor] = testog;
	else
	    iy[cursor] = testeg;

	m[cursor] = match - (endgapopen + (ypos - 1) * endgapextend);
	ix[cursor] = -endgapopen - ypos * endgapextend - gapopen;
    }

    ix[cursor] -= endgapopen;
    ix[cursor] += gapopen;

    cursor=0;

    /* Now initialise the first row */
    for (xpos = 1; xpos < lenb; ++xpos)
    {
	match = sub[ajSeqcvtGetCodeK(cvt,a[0])][ajSeqcvtGetCodeK(cvt,b[xpos])];
	cursor = xpos;
	cursorp = xpos -1;

	testog = m[cursorp] - gapopen;
        testeg = ix[cursorp] - gapextend;

        if(testog >= testeg)
            ix[cursor] = testog;
        else
            ix[cursor] = testeg;

	m[cursor] = match - (endgapopen + (xpos - 1) * endgapextend);
	iy[cursor] = -endgapopen - xpos * endgapextend -gapopen;
    }

    iy[cursor] -= endgapopen;
    iy[cursor] += gapopen;

    xpos = 1;

    /* Now construct match, ix, and iy matrices */
    while (xpos != lenb)
    {
        ypos = 1;
        bconvcode = ajSeqcvtGetCodeK(cvt, *++b);

        /* coordinates of the cells being processed */
        cursorp = xpos-1;
        cursor = xpos++;

        while (ypos < lena)
        {
            /* get match for current xpos/ypos */
            match = sub[ajSeqcvtGetCodeK(cvt, a[ypos++])][bconvcode];

            cursor += lenb;

            /* match matrix calculations */
            mp = m[cursorp];
            ixp = ix[cursorp];
            iyp = iy[cursorp];

            if(mp > ixp && mp > iyp)
                m[cursor] = mp+match;                     
            else if(ixp > iyp)
                m[cursor] = ixp+match;
            else
                m[cursor] = iyp+match;

            /* iy matrix calculations */
            if(xpos==lenb)
            {
        	testog = m[++cursorp] - endgapopen;
            	
            if (testog<ix[cursorp]-endgapopen)
                testog = ix[cursorp]-endgapopen;
            	
        	testeg = iy[cursorp] - endgapextend;
            }
            else
            {
        	testog = m[++cursorp];
        	
        	if (testog<ix[cursorp])
        	    testog = ix[cursorp];
        	
        	testog -= gapopen;
        	testeg = iy[cursorp] - gapextend;
            }
            
            if(testog > testeg)
                iy[cursor] = testog;
            else
        	iy[cursor] = testeg;

            cursorp += lenb;

            /* ix matrix calculations */
            if(ypos==lena)
            {
        	testog = m[--cursorp] - endgapopen;
            	
            if (testog<iy[cursorp]-endgapopen)
                testog = iy[cursorp]-endgapopen;
            	
        	testeg = ix[cursorp] - endgapextend;
            }
            else
            {
        	testog = m[--cursorp];
        	
        	if (testog<iy[cursorp])
        	    testog = iy[cursorp];
        	
        	testog -= gapopen;
        	testeg = ix[cursorp] - gapextend;
            }
            
            if(testog > testeg )
                ix[cursor] = testog;
            else
        	ix[cursor] = testeg;

        }
    }

    score = embAlignGetScoreNWMatrix(ix, iy, m, lena, lenb,
            start1, start2, endweight);

    xpos = *start2;
    ypos = *start1;

    /* In the following loop the three matrices (m, ix, iy) are traced back
     * and path/alignment decision/selection is made.
     * 0 means match: go up and left in the matrix
     * 1 means: go left in the matrix, i.e. gap in the first sequence(seq a)
     * 2 means: go up in the matrix, i.e. gap in the second sequence(seq b)
     */
    cursorp=0;
    cursor=1;
    
    while (xpos>=0 && ypos>=0)
    {
	cursor = ypos*lenb+xpos;
	mp = m[cursor];

	if(cursorp == LEFT && E_FPEQ((ypos==0||(ypos==lena-1)?
		endgapextend:gapextend), (ix[cursor]-ix[cursor+1]),U_FEPS))
	{
	    compass[cursor] = LEFT;
	    xpos--;
	}
	else if(cursorp== DOWN && E_FPEQ((xpos==0||(xpos==lenb-1)?
		endgapextend:gapextend), (iy[cursor]-iy[cursor+lenb]),U_FEPS))
	{
	    compass[cursor] = DOWN;
	    ypos--;
	}
	else if(mp >= ix[cursor] && mp>= iy[cursor])
	{

	    if(cursorp == LEFT && E_FPEQ(mp,ix[cursor],U_FEPS))
	    {
		compass[cursor] = LEFT;
		xpos--;
	    }
	    else if(cursorp == DOWN && E_FPEQ(mp,iy[cursor],U_FEPS))
	    {
		compass[cursor] = DOWN;
		ypos--;
	    }
	    else
	    {
		compass[cursor] = 0;
		ypos--;
		xpos--;
	    }

	}
	else if(ix[cursor]>=iy[cursor] && xpos>-1)
	{
	    compass[cursor] = LEFT;
	    xpos--;
	}
	else if(ypos>-1)
	{
	    compass[cursor] = DOWN;
	    ypos--;
	}
	else
	{
	    ajErr("something is seriously wrong in the traceback algorithm");
	    ajExitAbort();
	}
	cursorp = compass[cursor];
    }

    if(show)
    {
        printPathMatrix(m, compass, a, b-lenb+1, lena, lenb);
        printPathMatrix(ix, compass, a, b-lenb+1, lena, lenb);
        printPathMatrix(iy, compass, a, b-lenb+1, lena, lenb);
    }

    
    return score;
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
** @return [float] Maximum score
**
** @release 2.8.0
** @@
** Optimised to keep a maximum value to avoid looping down or left
** to find the maximum. (il 29/07/99)
******************************************************************************/

float embAlignPathCalcSW(const char *a, const char *b, ajint lena, ajint lenb,
                         float gapopen, float gapextend, float *path,
                         float * const *sub, const AjPSeqCvt cvt,
                         ajint *compass, AjBool show)
{
    float ret;
    ajlong xpos;
    ajlong ypos;
    ajlong i;
    ajlong j;

    double match;
    double mscore;
    double result;
    double fnew;
    double *maxa;

    static AjPStr outstr = NULL;
    double bx;
    char compasschar;

    ajDebug("embAlignPathCalcSW\n");

    ret= -FLT_MAX;

    /* Create stores for the maximum values in a row or column */

    maxa = AJALLOC(lena*sizeof(double));


    /* First initialise the first column and row */
    for(i=0;i<lena;++i)
    {
	result = sub[ajSeqcvtGetCodeK(cvt,a[i])][ajSeqcvtGetCodeK(cvt,b[0])];

	fnew = i==0 ? 0. :
		path[(i-1)*lenb] -(compass[(i-1)*lenb]==DOWN ?
			gapextend : gapopen);

	if (result > fnew && result>0)
	{
	  path[i*lenb] = (float) result;
	    compass[i*lenb] = 0;
	}
	else if (fnew>0)
	{
	  path[i*lenb] = (float) fnew;
	    compass[i*lenb] = DOWN;
	}
	else
	{
	    path[i*lenb] = 0.;
	    compass[i*lenb] = 0;
	}

	maxa[i] = i==0 ? path[i*lenb]-gapopen :
	path[i*lenb] - (compass[(i-1)*lenb]==DOWN ? gapextend : gapopen);
    }

    for(j=0;j<lenb;++j)
    {
	result = sub[ajSeqcvtGetCodeK(cvt,a[0])][ajSeqcvtGetCodeK(cvt,b[j])];

	fnew = j==0 ? 0. :
		path[j-1] -(compass[j-1]==LEFT ? gapextend : gapopen);

	if (result > fnew && result > 0)
	{
	  path[j] = (float) result;
	    compass[j] = 0;
	}
	else if (fnew >0)
	{
	  path[j] = (float) fnew;
	    compass[j] = LEFT;
	}
	else
	{
	    path[j] = 0.;
	    compass[j] = 0;
	}

    }


    /* xpos and ypos are the diagonal steps so start at 1 */
    xpos = 1;

    while(xpos!=lenb)
    {
	ypos  = 1;
	bx = path[xpos]-gapopen-gapextend;

	while(ypos < lena)
	{
	    /* get match for current xpos/ypos */
	    match = sub[ajSeqcvtGetCodeK(cvt,a[ypos])]
                       [ajSeqcvtGetCodeK(cvt,b[xpos])];

	    /* Get diag score */
	    mscore = path[(ypos-1)*lenb+xpos-1] + match;

	    /* Set compass to diagonal value 0 */
	    compass[ypos*lenb+xpos] = 0;
	    path[ypos*lenb+xpos] = (float) mscore;


	    /* Now parade back along X axis */
            maxa[ypos] -= gapextend;
            fnew=path[(ypos)*lenb+xpos-1];
            fnew-=gapopen;

#ifdef TESTALL
            ajDebug("xpos:%d ypos:%d mscore: %.2f fnew:%.2f maxa: %.2f\n",
        	    xpos, ypos, mscore, fnew, maxa[ypos]);
#endif

            if(fnew > maxa[ypos])
                maxa[ypos] = fnew;

            if( maxa[ypos] > mscore)
            {
                mscore = maxa[ypos];
                path[ypos*lenb+xpos] = (float) mscore;
                compass[ypos*lenb+xpos] = LEFT; /* Score comes from left */
            }

	    /* And then bimble down Y axis */
            bx -= gapextend;
            fnew = path[(ypos-1)*lenb+xpos];
            fnew-=gapopen;

            if(fnew > bx)
                bx = fnew;

            if(bx > mscore)
            {
                mscore = bx;
                path[ypos*lenb+xpos] = (float) mscore;
                compass[ypos*lenb+xpos] = DOWN; /* Score comes from bottom */
            }

            if(mscore > ret)
                ret = (float) mscore;

	    result = path[ypos*lenb+xpos];
	    if(result < 0.)
		path[ypos*lenb+xpos] = 0.;

	    ypos++;
	}
	++xpos;
    }

    if(show)
    {
	for(i=lena-1;i>-1;--i)
	{
            ajFmtPrintS(&outstr, "%6d ", i);

	    for(j=0;j<lenb;++j)
            {
                if(compass[i*lenb+j] == LEFT)
                    compasschar = '<';
                else if(compass[i*lenb+j] == DOWN)
                    compasschar = 'v';
                else
                    compasschar = ' ';

		ajFmtPrintAppS(&outstr, "%6.2f%c ",
                               path[i*lenb+j],compasschar);
            }

	    ajDebug("%S\n", outstr);
	}

        ajFmtPrintS(&outstr, "       ");

        for(j=0;j<lenb;++j)
            ajFmtPrintAppS(&outstr, "%6d  ", j);

        ajDebug("%S\n", outstr);
        ajStrDel(&outstr);
    }

    AJFREE(maxa);

    ajStrDelStatic(&outstr);

    return ret;
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
** @param [w] start1 [ajint *] start of alignment in first sequence
** @param [w] start2 [ajint *] start of alignment in second sequence
**
** @return [void]
**
** @release 1.0.0
******************************************************************************/

void embAlignWalkSWMatrix(const float *path, const ajint *compass,
			  float gapopen, float gapextend,
                          const AjPSeq a, const AjPSeq b,
			  AjPStr *m, AjPStr *n,
                          ajint lena, ajint lenb,
			  ajint *start1, ajint *start2)
{
    ajlong i;
    ajlong j;
    ajlong k;
    ajlong gapcnt;
    double pmax;
    double score;
    double bimble;

    ajlong ix;
    ajlong iy;

    ajlong xpos = 0;
    ajlong ypos = 0;
    const char *p;
    const char *q;

    ajint ic;
    double errbounds;

    ajDebug("embAlignWalkSWMatrix\n");

    /* errbounds = gapextend; */
    errbounds = (double) 0.01;

    /* Get maximum path score and save position */
    pmax = -FLT_MAX;
    k = (ajlong)lena*(ajlong)lenb-1;

    for(i=lena-1; i>=0; --i)
	for(j=lenb-1; j>=0; --j)
	    if((path[k--] > pmax) || E_FPEQ(path[k+1],pmax,U_FEPS))
	    {
		pmax = path[k+1];
		xpos = j;
		ypos = i;
	    }

    ajStrAssignClear(m);
    ajStrAssignClear(n);

    p = ajSeqGetSeqC(a);
    q = ajSeqGetSeqC(b);

    while(xpos>=0 && ypos>=0)
    {
	if(!compass[ypos*lenb+xpos])	/* diagonal */
	{
	    ajStrAppendK(m,p[ypos--]);
	    ajStrAppendK(n,q[xpos--]);

	    if(ypos >= 0 && xpos>=0 && path[(ypos)*lenb+xpos]<=0.)
		break;

	    continue;
	}
	else if(compass[ypos*lenb+xpos]==LEFT) /* Left, gap(s) in vertical */
	{
	    score  = path[ypos*lenb+xpos];
	    gapcnt = 0;
	    ix     = xpos-1;

	    while(1)
	    {
		bimble = path[ypos*lenb+ix]-gapopen-(gapcnt*gapextend);

		if(!ix || fabs((double)score-(double)bimble)<errbounds)
		    break;

		--ix;
		++gapcnt;
	    }

	    if(bimble<=0.0)
		break;

	    for(ic=0;ic<=gapcnt;++ic)
	    {
		ajStrAppendK(m,'.');
		ajStrAppendK(n,q[xpos--]);
	    }

	    continue;
	}
	else if(compass[ypos*lenb+xpos]==DOWN) /* Down, gap(s) in horizontal */
	{
	    score  = path[ypos*lenb+xpos];
	    gapcnt = 0;
	    iy = ypos-1;

	    while(1)
	    {
		bimble=path[iy*lenb+xpos]-gapopen-(gapcnt*gapextend);

		if(!iy || fabs((double)score-(double)bimble)<errbounds)
		    break;

		--iy;

		if(iy<0)
		    ajFatal("SW: Error walking down");

		++gapcnt;
	    }

	    if(bimble<=0.0)
		break;

	    for(ic=0;ic<=gapcnt;++ic)
	    {
		ajStrAppendK(m,p[ypos--]);
		ajStrAppendK(n,'.');
	    }
	    continue;
	}
	else
	    ajFatal("Walk Error in SW");
    }

    *start1 = (ajint) (ypos + 1); /* Potential lossy cast */
    *start2 = (ajint) (xpos + 1); /* Potential lossy cast */

    ajStrReverse(m);            /* written with append, need to reverse */
    ajStrReverse(n);

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
** @param [r] compass [const ajint*] Path direction pointer array
**
** @return [void]
**
** @release 1.0.0
******************************************************************************/

void embAlignWalkNWMatrix(const float *path, const AjPSeq a, const AjPSeq b,
			  AjPStr *m,
			  AjPStr *n, ajint lena, ajint lenb, ajint *start1,
			  ajint *start2, float gapopen,
			  float gapextend,
			  const ajint *compass)
{
    ajint i;
    ajint j;
    ajint gapcnt;
    double pmax;
    double score;
    double bimble;

    ajint ix;
    ajint iy;

    ajint xpos = 0;
    ajint ypos = 0;
    const char *p;
    const char *q;

    ajint ic;
    double errbounds;

    ajDebug("embAlignWalkNWMatrix\n");

    /*errbounds=gapextend;*/
    errbounds = (double) 0.01;

    /* Get maximum path axis score and save position */
    pmax = (double) (-1*INT_MAX);

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

    ajStrAssignClear(m);
    ajStrAssignClear(n);

    p = ajSeqGetSeqC(a);
    q = ajSeqGetSeqC(b);

    while(xpos>=0 && ypos>=0)
    {
	if(!compass[ypos*lenb+xpos])	/* diagonal */
	{
	    ajStrAppendK(m,p[ypos--]);
	    ajStrAppendK(n,q[xpos--]);
	    continue;
	}
	else if(compass[ypos*lenb+xpos]==LEFT) /* Left, gap(s) in vertical */
	{
	    score  = path[ypos*lenb+xpos];
	    gapcnt = 0;
	    ix     = xpos-1;

	    while(1)
	    {
		bimble = path[ypos*lenb+ix]-gapopen-(gapcnt*gapextend);

		if(!ix || fabs((double)score-(double)bimble)< errbounds)
		    break;

		--ix;
		++gapcnt;
	    }

	    for(ic=0;ic<=gapcnt;++ic)
	    {
		ajStrAppendK(m,'.');
		ajStrAppendK(n,q[xpos--]);
	    }
	    continue;
	}
	else if(compass[ypos*lenb+xpos]==DOWN) /* Down, gap(s) in horizontal */
	{
	    score  = path[ypos*lenb+xpos];
	    gapcnt = 0;
	    iy = ypos-1;

	    while(1)
	    {
		bimble = path[iy*lenb+xpos]-gapopen-(gapcnt*gapextend);

		if(!iy || fabs((double)score-(double)bimble)< errbounds)
		    break;

		--iy;
		++gapcnt;
	    }

	    for(ic=0;ic<=gapcnt;++ic)
	    {
		ajStrAppendK(m,p[ypos--]);
		ajStrAppendK(n,'.');
	    }

	    continue;
	}
	else
	    ajFatal("Walk Error in NW");

    }

    *start2 = xpos+1;
    *start1 = ypos+1;

    ajStrReverse(m);            /* written with append, need to reverse */
    ajStrReverse(n);

    return;
}




/* @func embAlignWalkNWMatrixUsingCompass *************************************
**
** Walk down a matrix for Needleman Wunsch which was constructed using end gap
** penalties. Form aligned strings. Nucleotides or proteins as needed.
**
** @param [r] p [const char*] first sequence
** @param [r] q [const char*] second sequence
** @param [w] m [AjPStr *] alignment for first sequence
** @param [w] n [AjPStr *] alignment for second sequence
** @param [r] lena [ajuint] length of first sequence
** @param [r] lenb [ajuint] length of second sequence
** @param [w] start1 [ajint *] start of alignment in first sequence
** @param [w] start2 [ajint *] start of alignment in second sequence
** @param [r] compass [ajint const *] Path direction pointer array
**
** @return [void]
**
** @release 6.2.0
** @@
**
******************************************************************************/

void embAlignWalkNWMatrixUsingCompass(const char* p, const char* q,
                                 AjPStr *m, AjPStr *n,
                                 ajuint lena, ajuint lenb,
                                 ajint *start1, ajint *start2,
                                 ajint const *compass)
{
    ajint xpos = *start2;
    ajint ypos = *start1;
    ajint i;
    ajint j;
    ajuint cursor;

    ajDebug("embAlignWalkNWMatrixUsingCompass\n");

    ajStrAssignClear(m);
    ajStrAssignClear(n);
    
    for (i=lenb-1; i>xpos;)
    {
        ajStrAppendK(n, q[i--]);
        ajStrAppendK(m, '.');
    }

    for (j=lena-1; j>ypos; )
    {
        ajStrAppendK(m, p[j--]);
        ajStrAppendK(n, '.');
    }

    while (xpos >= 0 && ypos >= 0)
    {
        cursor = ypos * lenb + xpos;
        if(!compass[cursor]) /* diagonal */
        {
            ajStrAppendK(m, p[ypos--]);
            ajStrAppendK(n, q[xpos--]);
            continue;
        }
        else if(compass[cursor] == LEFT) /* Left, gap(s) in vertical */
        {
            ajStrAppendK(m, '.');
            ajStrAppendK(n, q[xpos--]);
            continue;
        }
        else if(compass[cursor] == DOWN) /* Down, gap(s) in horizontal */
        {
            ajStrAppendK(m, p[ypos--]);
            ajStrAppendK(n, '.');

            continue;
        } else
            ajFatal("Walk Error in NW");
    }

    for (;xpos>=0;xpos--)
    {
        ajStrAppendK(n, q[xpos]);
        ajStrAppendK(m, '.');
    }

    for (;ypos>=0;ypos--)
    {
        ajStrAppendK(m, p[ypos]);
        ajStrAppendK(n, '.');
    }

    *start2 = xpos+1;
    *start1 = ypos+1;

    ajStrReverse(m); /* written with append, need to reverse */
    ajStrReverse(n);

    ajDebug("first sequence extended with gaps  (m): %S\n", *m);
    ajDebug("second sequence extended with gaps (n): %S\n", *n);
    
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
**
** @release 1.0.0
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

	    if(mark)
                ajStrAppendC(&fm," ");
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




/* @func embAlignUnused *******************************************************
**
** Calls unused functions to avoid warning messages
**
** @return [void]
**
** @release 2.0.0
******************************************************************************/

void embAlignUnused(void)
{
    return;
}




/* @func embAlignPathCalcSWFast ***********************************************
**
** Create path matrix for Needleman-Wunsch with limited width around main
** diagonal.
**
** Nucleotides or proteins as needed.
**
** @param [r] a [const char *] first sequence
** @param [r] b [const char *] second sequence
** @param [r] lena [ajint] length of first sequence
** @param [r] lenb [ajint] length of second sequence
** @param [r] offset [ajint] Diagonal offset. Zero is start of a and b.
**                           Negative is position in a.
**                           Positive is position in b.
** @param [r] pathwidth [ajint] width of path matrix
** @param [r] gapopen [float] gap opening penalty
** @param [r] gapextend [float] gap extension penalty
** @param [w] path [float *] path matrix of size
**                (minlength*pathwidth + (pathwidth/2)*pathwidth)
**                current value: (pathwidth*(lena+1)), lena==lenb
** @param [r] sub [float * const *] substitution matrix from AjPMatrixf
** @param [r] cvt [const AjPSeqCvt] Conversion array for AjPMatrixf
** @param [w] compass [ajint *] Path direction pointer array or
**                              same dimensions as path
** @param [r] show [AjBool] Display path matrix
**
** @return [float] Maximum score
**
** @release 6.0.0
** @@
** Optimised to keep a maximum value to avoid looping down or left
** to find the maximum. (il 29/07/99)
**
** Further speeded up by using only width calculations instead of lena.
**
******************************************************************************/

float embAlignPathCalcSWFast(const char *a, const char *b,
                             ajint lena, ajint lenb,
                             ajint offset, ajint pathwidth,
                             float gapopen, float gapextend,
                             float *path, float * const *sub,
                             const AjPSeqCvt cvt, ajint *compass,
                             AjBool show)
{
    float ret;
    ajint irow;
    ajint icol;

    ajint i;
    ajint j;
    ajint k;
    ajint ip;                   /* position in path matrix */

    float match;
    float mscore;
    float fnew;
    float *maxa;
    float bx = 0.0F;

    ajint width;
    ajint leftwidth;
    ajint rightwidth;

    ajint xmin;                 /* first x position for current row */
    ajint ymin;
    ajint xmax;                 /* last x position for current row */
    ajint ymax;

    char compasschar;

    ret = -FLT_MAX;

    width = pathwidth;
    if(width < 1)
        width = 1;

    if(lena < width)
	width = lena;

    if(lenb < width)
	width = lenb;

    leftwidth = width/2;
    rightwidth = width - leftwidth - 1;


    if(offset >= 0)             /* x (b) offset */
        ymin =  0;
    else
        ymin = -offset;

    ymax = lenb + leftwidth - offset;

    if(ymax > lena)
        ymax = lena;

    if(offset <= 0)             /* y (a) offset) */
        xmin =  -leftwidth;
    else
        xmin = offset - leftwidth;

    xmax = lena + rightwidth + offset;

    if(xmax > lenb)
        xmax = lenb;
   
    ajDebug("embAlignPathCalcSWFast\n");

    ajDebug("lena:%d lenb:%d ymin:%d ymax:%d xmin:%d xmax:%d\n",
            lena, lenb, ymin, ymax, xmin,xmax);
    ajDebug("pathwidth:%d width:%d leftwidth:%d rightwidth:%d\n",
            pathwidth, width, leftwidth, rightwidth);
    ajDebug("a: '%s'\n", a);
    ajDebug("b: '%s'\n", b);
    

    /* Create stores for the maximum values in a row or column */
    maxa = AJALLOC((xmax+1)*sizeof(float));

    /* TODO: in the following 2 loops we miss the cases when it is possible
     * to have gaps just after the first base
     */
    /* initialise the first column */
    for(i=0;i<=lena;++i)
    {
	ip = i*width+leftwidth-i;
	if(i < leftwidth)
	{
            path[ip] = sub[ajSeqcvtGetCodeK(cvt,a[i])]
                               [ajSeqcvtGetCodeK(cvt,b[0])];

            if(path[ip]<0)
                path[ip] = 0;

            fnew = i==0 ? (float) 0. :
	      path[ip-width-1] - (float) (compass[ip-width-1] == DOWN ?
				  gapextend : gapopen);

            if(fnew > path[ip])
            {
        	path[ip] = fnew;
        	compass[ip] = DOWN;
            }
            else
        	compass[ip] = 0;
	}
	else
	{
	    path[ip] = 0;
	    compass[ip] = 0;
	}

	ajDebug("CalcFast init first column; row[%d] path[%d]: %.2f"
		" compass: %d\n",
		i, ip, path[ip], compass[ip]);
    }

    /* initialise the first row */
    for(j=0;j<width;++j)
    {
        if(j >= leftwidth)
        {
            path[j] = sub[ajSeqcvtGetCodeK(cvt,a[0])]
                         [ajSeqcvtGetCodeK(cvt,b[j-leftwidth])];

            if(path[j]<0)
        	path[j] = 0;

            fnew = j==0 ? (float) 0. :
	      path[j-1] - (float) (compass[j-1] == LEFT ?
			           gapextend : gapopen);

            if(fnew > path[j])
            {
        	path[j] = fnew;
        	compass[j] = LEFT;
            }
            else
        	compass[j] = 0;

        }
        else
        {
            path[j] = 0;
            compass[j] = 0;
        }

        maxa[j] = j==0 ? path[j]-gapopen :
        path[j] - (compass[j-1]==LEFT ? gapextend : gapopen);

	ajDebug("CalcFast init first row; col[%d] path: %.2f compass: %d\n",
		j, path[j], compass[j]);
    }

    irow = ymin;
    
    while (++irow < ymax)
    {
        ip = irow*width-1;
        icol = xmin++;

        for(i=0;i<width;i++)
        {
            compass[++ip] = DIAG;

            if(icol++ < 0)
                continue;

            if(icol >= xmax)
                break;

            match = sub[ajSeqcvtGetCodeK(cvt,a[irow])]
                       [ajSeqcvtGetCodeK(cvt,b[icol])];

	    /* Get diag score */
	    mscore = path[ip-width] + match;

            ajDebug("match %4.1f irow:%d icol:%d i:%d ip:%2d a:%c b:%c "
        	    "mscore: %4.2f bx: %4.2f\n",
        	    match, irow, icol, i, ip, a[irow], b[icol], mscore, bx);

            if(mscore < 0.0)
                mscore = 0.0;

	    /* Set compass to diagonal value 0 */
	    path[ip] = (float) mscore;

	    if(i > 0 && i!=width-1)
	    {

		fnew  = path[ip-width+1];
		fnew -= gapopen;

		if (i==width-2)
		    maxa[icol] = fnew;
		else
		{
		    maxa[icol] -= gapextend;

		    if(fnew > maxa[icol])
			maxa[icol] = fnew;
		}

		if( maxa[icol] > mscore)
		{
		    mscore = maxa[icol];
		    path[ip] = mscore;
		    compass[ip] = DOWN; /* Score comes from bottom */
		}
	    }
            
            if(irow > 0 && i!=0)
            {
        	if(i == 1 || icol==1)
        	    bx = path[ip-1]-gapopen;
        	else
        	{
        	    bx -= gapextend;

        	    if(compass[ip-1]!=1)
        	    {
        		fnew=path[ip-1];
        		fnew-=gapopen;

        		if(fnew > bx)
        		    bx = fnew;
        	    }
        	}

        	if(bx > mscore)
        	{
        	    mscore = bx;
        	    path[ip] = mscore;
        	    compass[ip] = LEFT; /* Score comes from left */
        	}
            }

            if(mscore > ret)
                ret = (float) mscore;

            if(path[ip]<0)
        	path[ip] = 0.;
        }
    }
        
            
    if(show)
    {

	for(i=ymax-1;i>=ymin;--i)
	{
            ajDebug("%5d %c", i, a[i]);

	    for(k=0;k<i;++k)
                ajDebug("        ");

	    for(j=0;j<width;++j)
            {
                if(compass[i*width+j] == LEFT)
                    compasschar = '<';
                else if(compass[i*width+j] == DOWN)
                    compasschar = 'v';
                else
                compasschar = ' ';

		ajDebug("%6.1f%c ",
                               path[i*width+j],compasschar);

            }

	    ajDebug("\n");
	}

        ajDebug("       ");

        for(k=0;k<leftwidth;++k)
                 ajDebug("        ");

        for(j=0;j<lenb;++j)
            ajDebug("%5d %c ", j,b[j]);

        ajDebug("\n");
    }

    AJFREE(maxa);

    return ret;
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
** @param [r] offset [ajint] Diagonal offset. Zero is start of a and b.
**                           Negative is position in a.
**                           Positive is position in b.
** @param [r] width [ajint] width of path matrix
** @param [w] start1 [ajint *] start of alignment in first sequence
** @param [w] start2 [ajint *] start of alignment in second sequence
**
** @return [void]
**
** @release 1.0.0
******************************************************************************/

void embAlignWalkSWMatrixFast(const float *path, const ajint *compass,
			      float gapopen, float gapextend,
			      const AjPSeq a, const AjPSeq b, AjPStr *m,
			      AjPStr *n, ajint lena, ajint lenb,
			      ajint offset, ajint width,
                              ajint *start1, ajint *start2)
{
    ajint i;
    ajint j;
    ajint k;
    ajint ip;                   /* position in path matrix */
    ajint gapcnt;
    double pmax;
    double score;
    double bimble = 0.;

    ajint ix;
    ajint iy;

    ajint xpos  = 0;
    ajint xpos2 = 0;
    ajint ypos  = 0;
    const char *p;
    const char *q;

    ajint ic;

    ajint leftwidth;
    ajint rightwidth;

    ajint ymin;
    ajint xmax;                 /* last x position */
    ajint ymax;

    ajDebug("embAlignWalkSWMatrixFast\n");

    if(lena < width)
	width = lena;

    if(lenb < width)
	width = lenb;

    leftwidth = width/2;
    rightwidth = width - leftwidth - 1;

    if(offset >= 0)             /* x (b) offset */
        ymin =  0;
    else
        ymin = -offset;

    ymax = lenb + leftwidth - offset;

    if(ymax > lena)
        ymax = lena;

    xmax = lena + rightwidth-1 + offset;

    if(xmax > lenb)
        xmax = lenb;

    /* Get maximum path score and save position */
    pmax = -FLT_MAX;
    k=0;

    for(i=ymin;i<ymax;++i)
	for(j=0;j<width;++j)
	    if(path[k++]>pmax)
	    {
		pmax = path[k-1];
		xpos = j;
		ypos = i;
	    }

    ajStrAssignClear(m);
    ajStrAssignClear(n);

    p = ajSeqGetSeqC(a);
    q = ajSeqGetSeqC(b);

    p += (*start1);
    q += (*start2);

    xpos2 = xpos+ypos-leftwidth;
    ajDebug("ypos:%d xpos:%d xpos2: %d start1:%d start2:%d width:%d\n",
            ypos, xpos, xpos2, *start1, *start2, width);

    while(xpos2>=0 && ypos>=0 && path[ypos*width+xpos] >0.)
    {
        ip = ypos*width+xpos;

        ajDebug("comp:%d %c %c ypos:%d xpos:%d xpos2:%d path[%d]:%.2f\n",
                compass[ip], p[ypos], q[xpos2], ypos, xpos, xpos2,
                ip, path[ip]);

        if(!compass[ip])	/* diagonal: xpos stays the same */
	{
            ajDebug("comp:%d %c %c ypos:%d xpos:%d xpos2:%d path[%d]:%.2f "
        	    "path[ip-width]: %.2f\n",
                    compass[ip], p[ypos], q[xpos2], ypos, xpos, xpos2,
                    ip, path[ip],
                    (ip>=width ? path[ip-width]:0));
	    ajStrAppendK(m,p[ypos--]);
	    ajStrAppendK(n,q[xpos2--]);

	    if(xpos2>=0 && ypos>=0 && ip>=width && path[ip-width]<=0.0)
		break;

	    continue;
	}
	else if(compass[ip]==LEFT) /* Horizontal gap(s): step through xpos */
	{
	    score  = path[ip];
	    gapcnt = 0;
	    ix     = xpos-1;
            bimble = 0.0;

	    while(ip>0)
	    {
		bimble=path[--ip]-gapopen-(gapcnt*gapextend);
		if(!ix || E_FPEQ(score,bimble,U_FEPS))
		    break;

		--ix;
		++gapcnt;
	    }

	    if(bimble<=0.0)
		break;

	    for(ic=0;ic<=gapcnt;++ic)
	    {
		ajStrAppendK(m,'.');
		ajStrAppendK(n,q[xpos2--]);
                xpos--;
	    }

            ajDebug("comp:%d %c %c ypos:%d xpos:%d xpos2:%d ix:%d "
                    "ip:%d path[%d]:%.2f gapcnt:%d\n",
                    compass[ip], p[ypos], q[xpos2], ypos, xpos, xpos2,
                    ix, ip, ip, path[ip], gapcnt);
	    continue;
	}
	else if(compass[ip]==DOWN) /* Vertical gap(s): step through ypos */
	{
	    score  = path[ip];
	    gapcnt = 0;
	    iy = ypos-1;
            bimble = 0.0;

	    while(iy>=0)
	    {
                ip -= width;
		bimble=path[++ip]-gapopen-(gapcnt*gapextend);
		if(E_FPEQ(score,bimble,U_FEPS))
		    break;

		--iy;
		++gapcnt;
	    }

	    if(bimble<=0.0)
		break;
            
	    for(ic=0;ic<=gapcnt;++ic)
	    {
		ajStrAppendK(m,p[ypos--]);
		ajStrAppendK(n,'.');
                xpos++;
	    }

            ajDebug("comp:%d %c %c ypos:%d xpos:%d xpos2:%d iy:%d "
                    "path[%d]:%f\n",
                    compass[ip], p[ypos], q[xpos2], ypos, xpos, xpos2,
                    iy, ip, path[ip]);
	    continue;
	}
	else
	    ajFatal("Walk Error in SW");
    }

    *start1 += ypos + 1;
    *start2 += xpos2 + 1;

    ajStrReverse(m);            /* written with append, need to reverse */
    ajStrReverse(n);

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
** @param [r] pmatrix [float * const *] profile matrix
** @param [w] compass [ajint *] Path direction pointer array
** @param [r] show [AjBool] Display path matrix
**
** @return [float] Maximum score
**
** @release 1.0.0
******************************************************************************/

float embAlignProfilePathCalc(const char *a, ajint proflen, ajint seqlen,
                              float gapopen,
                              float gapextend, float *path,
                              float * const *pmatrix,
                              ajint *compass, AjBool show)
{
    float ret;
    ajint row;				/* profile position in path */

    ajint column;		        /* sequence position in path */

    static AjPStr outstr = NULL;

    double fmscore;
    double mscore;
    double fnew;
    double maxp;
    double *maxs;

    ret = -FLT_MAX;
    ajDebug("embAlignProfilePathCalc\n");

    maxs = AJALLOC(seqlen*sizeof(double));

    /* First initialise the first column and row */
    for(column=0;column<seqlen;++column)
    {
	path[column] = pmatrix[0][ajBasecodeToInt(a[column])];

        if(path[column]<0.0)
            path[column]=0.0;

	compass[column] = DIAG;
	maxs[column] = path[column]-(pmatrix[0][GAPO]*gapopen);
    }

    for(row=0;row<proflen;++row)
    {
	path[row*seqlen] = pmatrix[row][ajBasecodeToInt(*a)];

        if(path[row*seqlen]<0.0)
            path[row*seqlen]=0.0;

	compass[row*seqlen] = DIAG;
    }


    /* diagonal steps start at 1 */
    for(row=1;row<proflen;++row)
    {
        column = 1;
        maxp = path[row*seqlen] - (pmatrix[row][GAPO]*gapopen);
        /* get match for current xpos/ypos */

        while(column!=seqlen)
        {
	    fmscore = pmatrix[row][ajBasecodeToInt(a[column])];

	    /* Get diag score */
	    mscore = fmscore + path[(row-1)*seqlen+(column-1)];

	    /* Initialise compass to diagonal value */
	    compass[row*seqlen+column] = DIAG;
	    path[row*seqlen+column]    = (float) mscore;

	    /* Now parade back along X axis */
            maxs[column] -= gapextend*pmatrix[row][GAPE];
            fnew=path[row*seqlen+column-1];
            fnew-=gapopen*pmatrix[row][GAPO];

            if(fnew > maxs[column])
                maxs[column] = fnew;

            if( maxs[column] > mscore)
            {
                mscore = maxs[column];
                path[row*seqlen+column] = (float) mscore;
                compass[row*seqlen+column] = 1; /* Score from left */
            }

	    /* And then bimble down Y axis */
            maxp -= gapextend*pmatrix[row-1][GAPE];
            fnew = path[(row-1)*seqlen+column];
            fnew-=gapopen*pmatrix[row-1][GAPO];

            if(fnew > maxp)
                maxp = fnew;

            if(maxp > mscore)
            {
                mscore = maxp;
                path[row*seqlen+column] = (float) mscore;
                compass[row*seqlen+column] = 2; /* Score from bottom */
            }

            if(mscore < 0.0)
            {
                path[row*seqlen+column] = 0.0;
                compass[row*seqlen+column] = DIAG;
                mscore = 0.0;
            }
            
            if(mscore > ret)
                ret = (float) mscore;

            ++column;
	}
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
    AJFREE(maxs);

    return ret;
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
** @param [r] pmatrix [float * const *] profile
** @param [w] start1 [ajint *] start of alignment in consensus sequence
** @param [w] start2 [ajint *] start of alignment in test sequence
**
** @return [void]
**
** @release 1.0.0
******************************************************************************/

void embAlignWalkProfileMatrix(const float *path, const ajint *compass,
			       float gapopen,
			       float gapextend,
			       const AjPStr cons, const AjPStr seq,
			       AjPStr *m, AjPStr *n, ajint proflen,
			       ajint seqlen, float * const *pmatrix,
			       ajint *start1, ajint *start2)
{
    ajint i;
    ajint gapcnt;
    double pathmax;
    double score;

    double bimble;

    ajint ix;
    ajint iy;

    ajint row    = 0;
    ajint column = 0;

    ajint direction = 0;

    ajint xpos = 0;
    ajint ypos = 0;

    const char *p;
    const char *q;

    double errbounds;

    ajDebug("embAlignWalkProfileMatrix\n");

    errbounds = (double) 0.01;

    /* Get maximum path score and save position */
    pathmax = -(double) INT_MAX;

    for(row=0;row<proflen;++row)
	for(column=0;column<seqlen;++column)
	    if(path[row*seqlen+column] > pathmax)
	    {
		pathmax = path[row*seqlen+column];
		xpos    = column;
		ypos    = row;
	    }

    ajDebug("score:%.3f xpos:%d/%d ypos:%d/%d\n",
            pathmax, xpos, seqlen, ypos, proflen);

    column = xpos;
    row = ypos;

    ajStrAssignClear(m);
    ajStrAssignClear(n);

    p = ajStrGetPtr(cons);
    q = ajStrGetPtr(seq);

    while(row>= 0 && column >= 0)
    {
        score = path[row*seqlen+column];
	direction = compass[row*seqlen+column];
        ajDebug("row:%d col:%d dir:%d\n",
                row, column, direction);

	if(direction == DIAG)
	{
	    ajStrAppendK(m,p[row--]);
	    ajStrAppendK(n,q[column--]);

            if(row < 0 || column < 0)
                break;
	    if(path[(row)*seqlen+(column)]<=0.)
		break;

            ajDebug("diagonal row:%d col:%d path:%.2f\n",
                    row, column, path[(row)*seqlen+(column)]);
            continue;
	}
	else if(direction == LEFT)
	{
	    gapcnt  = 0;
	    ix     = column-1;
            bimble = 0.0;

	    while(1)
	    {
		bimble = path[row*seqlen+ix] -
                         pmatrix[row][GAPO]*gapopen -
                         (gapcnt * pmatrix[row][GAPE]*gapextend);

		if(!ix || fabs((double)score-(double)bimble)<errbounds)
		    break;

		--ix;
		++gapcnt;
	    }

	    if(bimble<=0.0)
		break;

	    for(i=0;i<=gapcnt;++i)
	    {
		ajStrAppendK(m,'.');
		ajStrAppendK(n,q[column--]);
	    }

            ajDebug("left row:%d col:%d path:%.2f\n",
                    row, column, path[(row)*seqlen+(column)]);
	    continue;
	}
	else if(direction == DOWN)
	{
	    gapcnt  = 0;
            iy = row -1 ;
            bimble = 0.0;

	    while(1)
	    {
		bimble = path[iy*seqlen+column] -
                         pmatrix[row-1][GAPO]*gapopen -
                         (gapcnt*pmatrix[row-1][GAPE]*gapextend);
                ajDebug("going down row:%d col:%d gapcnt:%d score:%.2f "
                        "bimble:%.2f\n",
                        row, column, gapcnt, score, bimble);

		if(!iy || fabs((double)score-(double)bimble)<errbounds)
		    break;

		--iy;
		++gapcnt;
	    }

	    if(bimble<=0.0)
		break;

	    for(i=0;i<=gapcnt;++i)
	    {
		ajStrAppendK(m,p[row--]);
		ajStrAppendK(n,'.');
	    }

            ajDebug("down row:%d col:%d path:%.2f\n",
                    row, column, path[(row)*seqlen+(column)]);
	    continue;
	}
	else
	    ajFatal("Walk Error in Profile Walk");
    }

    *start1 = row + 1;
    *start2 = column + 1;

    ajStrReverse(m);            /* written with append, need to reverse */
    ajStrReverse(n);

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
** @param [r] pmatrix [float * const *] profile
** @param [r] namea [const char *] name of first sequence
** @param [r] nameb [const char *] name of second sequence
** @param [r] begina [ajint] first sequence offset
** @param [r] beginb [ajint] second sequence offset
**
** @return [void]
**
** @release 1.0.0
******************************************************************************/

void embAlignPrintProfile(AjPFile outf,
			  const AjPStr m, const AjPStr n,
			  ajint start1, ajint start2, float score, AjBool mark,
			  float * const *pmatrix, const char *namea,
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

	    match=pmatrix[cpos][ajBasecodeToInt(q[i])];

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
**
** @release 1.6.3
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




/* @funcstatic embAlignGetScoreNWMatrix ***************************************
**
** Returns score of the optimal global or overlap alignment for
** the specified path matrix for Needleman Wunsch
**
** @param [r] ix [const float*] Gap scores array, ix(i,j) is the best score
**                              given that a(i) is aligned to a gap
**                              (in an insertion with respect to b)
** @param [r] iy [const float*] Gap scores array, iy(i,j) is the best score
**                              given that b(i) is in an insertion
**                              with respect to a
**
** @param [r] m [const float*] Match scores array, m(i,j) is the best score
**                             up to (i,j) given that a(i) is aligned to b(j)
** @param [r] lena [ajint] length of the first sequence
** @param [r] lenb [ajint] length of the second sequence
** @param [w] start1 [ajint *] start of alignment in first sequence
** @param [w] start2 [ajint *] start of alignment in second sequence
** @param [r] endweight [AjBool] whether the matrix was built for
**                               a global or overlap alignment
**
** @return [float] optimal score
**
** @release 6.2.0
******************************************************************************/

static float embAlignGetScoreNWMatrix(
	const float *ix, const float *iy, const float *m,
        ajint lena, ajint lenb,
        ajint *start1, ajint *start2,
        AjBool endweight)
{
    ajint i,j, cursor;
    float score = INT_MIN;
    *start1 = lena-1;
    *start2 = lenb-1;
    
    if(endweight)
    {
        /* when using end gap penalties the score of the optimal global
         * alignment is stored in the final cell of the path matrix */
	cursor = lena * lenb - 1;
	if(m[cursor]>ix[cursor]&&m[cursor]>iy[cursor])
	    score = m[cursor];
	else if(ix[cursor]>iy[cursor])
	    score = ix[cursor];
	else
	    score = iy[cursor];
    }
    else {

        for (i = 0; i < lenb; ++i)
        {
            cursor = (lena - 1) * lenb + i;
            if(m[cursor]>score)
            {
        	*start2 = i;
        	score = m[cursor];
            }
            if(ix[cursor]>score)
            {
        	score = ix[cursor];
        	*start2 = i;
            }
            if(iy[cursor]>score)
            {
        	score = iy[cursor];
        	*start2 = i;
            }
        }

        for (j = 0; j < lena; ++j)
        {
            cursor = j * lenb + lenb - 1;
            if(m[cursor]>score)
            {
        	*start1 = j;
        	*start2 = lenb-1;
        	score = m[cursor];
            }
            if(ix[cursor]>score)
            {
        	score = ix[cursor];
        	*start1 = j;
        	*start2 = lenb-1;
            }
            if(iy[cursor]>score)
            {
        	score = iy[cursor];
        	*start1 = j;
        	*start2 = lenb-1;
            }
        }
    }
    return score;
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
**
** @release 2.1.0
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
**
** @release 2.1.0
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
    const float* qual = NULL;
    ajint offend1;
    ajint offend2;

    offend1 = (ajint) (ajSeqGetLen(seqa) - ajStrGetLen(m) - start1
                       + ajStrCalcCountK(m, '.')
                       + ajStrCalcCountK(m, '-')
                       + ajStrCalcCountK(m, ' '));

    ajDebug("embAlignReportLocal lenseqa: %d lenm: %d start1: %d "
	    ".count:%d -count: %d spacecount:%d\n",
	    ajSeqGetLen(seqa),ajStrGetLen(m),start1,
	    	 ajStrCalcCountK(m, '.'),
	    	ajStrCalcCountK(m, '-'),
	    	    ajStrCalcCountK(m, ' '));

    offend2 = (ajint) (ajSeqGetLen(seqb) - ajStrGetLen(n) - start2
                       + ajStrCalcCountK(n, '.')
                       + ajStrCalcCountK(n, '-')
                       + ajStrCalcCountK(n, ' '));

    ajDebug("embAlignReportLocal lenseqb: %d lenn: %d start2: %d "
	    ".count:%d -count: %d spacecount:%d\n",
	    ajSeqGetLen(seqb),ajStrGetLen(n),start2,
	    	 ajStrCalcCountK(n, '.'),
	    	ajStrCalcCountK(n, '-'),
	    	    ajStrCalcCountK(n, ' '));

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
    qual = ajSeqGetQual(seqa);

    if(qual!=NULL)
	ajSeqAssignQualLen(res1,qual+start1+ajSeqGetOffset(seqa),
                           ajSeqGetLenUngapped(res1));

    res2   = ajSeqNewRangeC(ajStrGetPtr(n), start2+ajSeqGetOffset(seqb),
			     offend2+ajSeqGetOffend(seqb),
			     ajSeqIsReversed(seqb));
    ajSeqAssignNameS(res2, ajSeqGetNameS(seqb));
    ajSeqAssignUsaS(res2, ajSeqGetUsaS(seqb));
    ajSeqAssignAccS(res2, ajSeqGetAccS(seqb));
    ajSeqAssignDescS(res2, ajSeqGetDescS(seqb));
    qual = ajSeqGetQual(seqb);

    if(qual!=NULL)
	ajSeqAssignQualLen(res2,qual+start2+ajSeqGetOffset(seqb),
                           ajSeqGetLenUngapped(res2));

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
**
** @release 2.3.0
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




/* @funcstatic printPathMatrix ************************************************
**
** Prints path matrix for Needleman-Wunsch
**
** @param [r] path [const float*] Alignment path matrix to be printed 
** @param [r] compass [const ajint*] Path direction pointer array
** @param [r] a [const char*] first sequence
** @param [r] b [const char*] second sequence
** @param [r] lena [ajuint] length of first sequence
** @param [r] lenb [ajuint] length of second sequence
** 
** @return [void]
**
** @release 6.2.0
******************************************************************************/

static void printPathMatrix(const float* path, const ajint* compass,
	const char *a, const char *b, ajuint lena, ajuint lenb)
{
    char compasschar;
    ajuint i;
    ajuint j;

    ajDebug("path matrix:\n");

    i = lena;
    while( i--!= 0)
    {
        ajDebug("%4d(%c)", i, a[i]);

        for(j = 0; j < lenb; j++)
        {
            if(compass[i * lenb + j] == LEFT)
                compasschar = '<';
            else if(compass[i * lenb + j] == DOWN)
                compasschar = '^';
            else
                compasschar = ' ';

            ajDebug("%6.2f%c ", path[i * lenb + j], compasschar);
        }
        ajDebug("\n");
    }

    ajDebug("       ");

    for (j = 0; j < lenb; ++j)
        ajDebug("%4d(%c) ", j, b[j]);

    ajDebug("\n");

    return;
}




#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif




#ifdef AJ_COMPILE_DEPRECATED
/* @obsolete embAlignPrintLocal
** @remove not used, see ajAlignWrite
*/

__deprecated void embAlignPrintLocal(AjPFile outf,
                                     const AjPStr m, const AjPStr n,
                                     ajint start1, ajint start2,
                                     float score, AjBool mark,
                                     float * const *sub, const AjPSeqCvt cvt,
                                     const char *namea, const char *nameb,
                                     ajint begina, ajint beginb)
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

    if(mark)
        r=ajStrGetPtr(fm);


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
#endif

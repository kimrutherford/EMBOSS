/* @source prophecy application
**
** Creates profiles and simple freuency matrices
** @author Copyright (C) Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
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
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define AZ 27
#define GRIBSKOV_LENGTH 27
#define HENIKOFF_LENGTH 27




static void prophecy_simple_matrix(const AjPSeqset seqset, AjPFile outf,
				   const AjPStr name,
				   ajint thresh);
static void prophecy_gribskov_profile(const AjPSeqset seqset,
				      AjPFile outf, const AjPStr name,
				      ajint thresh,
				      float gapopen, float gapextend,
				      AjPStr *cons);
static void prophecy_henikoff_profile(const AjPSeqset seqset,
				      const AjPMatrixf imtx,
				      ajint thresh, const AjPSeqCvt cvt,
				      AjPFile outf, const AjPStr name,
				      float gapopen, float gapextend,
				      AjPStr *cons);




/* @prog prophecy *************************************************************
**
** Creates matrices/profiles from multiple alignments
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPSeqset seqset = NULL;
    AjPFile outf = NULL;
    AjPStr name  = NULL;
    AjPStr cons  = NULL;

    ajint thresh;
    AjPStr type;

    AjPMatrixf imtx = NULL;
    AjPSeqCvt cvt = NULL;
    float gapopen;
    float gapextend;

    embInit("prophecy", argc, argv);


    seqset = ajAcdGetSeqset("sequence");
    name   = ajAcdGetString("name");
    thresh = ajAcdGetInt("threshold");
    imtx   = ajAcdGetMatrixf("datafile");
    type   = ajAcdGetListSingle("type");
    outf   = ajAcdGetOutfile("outfile");

    gapopen   = ajAcdGetFloat("open");
    gapextend = ajAcdGetFloat("extension");

    gapopen   = ajRoundFloat(gapopen, 8);
    gapextend = ajRoundFloat(gapextend, 8);

    cons = ajStrNewC("");


    if(ajStrGetCharFirst(type) == 'F')
	prophecy_simple_matrix(seqset,outf,name,thresh);

    if(ajStrGetCharFirst(type) == 'G')
	prophecy_gribskov_profile(seqset,outf,name,thresh,
				 gapopen,gapextend,&cons);

    if(ajStrGetCharFirst(type) == 'H')
	prophecy_henikoff_profile(seqset,imtx,thresh,cvt,outf,name,
				 gapopen,gapextend,&cons);

    ajFileClose(&outf);

    ajSeqsetDel(&seqset);
    ajStrDel(&name);
    ajStrDel(&cons);
    ajMatrixfDel(&imtx);
    ajStrDel(&type);

    embExit();

    return 0;
}




/* @funcstatic prophecy_simple_matrix *****************************************
**
** Undocumented.
**
** @param [r] seqset [const AjPSeqset] Undocumented
** @param [u] outf [AjPFile] Undocumented
** @param [r] name [const AjPStr] Undocumented
** @param [r] thresh [ajint] Undocumented
** @@
******************************************************************************/


static void prophecy_simple_matrix(const AjPSeqset seqset, AjPFile outf,
				   const AjPStr name,
				   ajint thresh)
{
    const char *p;
    ajuint nseqs;
    ajuint mlen;
    ajint len;
    ajuint i;
    ajint j;
    ajint x;
    ajint px;

    ajint maxscore;
    ajint score;
    ajint *matrix[AZ+2];
    AjPStr cons = NULL;
    size_t stlen;
    
    nseqs = ajSeqsetGetSize(seqset);
    if(nseqs<2)
	ajFatal("Insufficient sequences (%d) to create a matrix",nseqs);

    mlen = ajSeqsetGetLen(seqset);

    /* Check sequences are the same length. Warn if not */
    for(i=0;i<nseqs;++i)
    {
	p = ajSeqsetGetseqSeqC(seqset,i);
	if(strlen(p)!=mlen)
	    ajWarn("Sequence lengths are not equal!");
    }

    for(i=0;i<AZ+2;++i)
	AJCNEW0(matrix[i], mlen);

    /* Load matrix */
    for(i=0;i<nseqs;++i)
    {
	p = ajSeqsetGetseqSeqC(seqset,i);
	stlen = strlen(p);
	len = (ajint) stlen;

	for(j=0;j<len;++j)
	{
	    x = toupper((ajint)*p++);
	    ++matrix[ajBasecodeToInt(x)][j];
	}
    }

    /* Get consensus sequence */
    cons = ajStrNew();
    for(i=0;i<mlen;++i)
    {
	px=x=-INT_MAX;

	for(j=0;j<AZ-1;++j)
	    if(matrix[j][i]>x)
	    {
		x=matrix[j][i];
		px=j;
	    }
	ajStrAppendK(&cons,(char)(px+'A'));
    }

    /* Find maximum score for matrix */
    maxscore = 0;
    for(i=0;i<mlen;++i)
    {
	for(j=score=0;j<AZ;++j)
	    score = AJMAX(score,matrix[j][i]);
	maxscore += score;
    }

    ajFmtPrintF(outf,"# Pure Frequency Matrix\n");
    ajFmtPrintF(outf,"# Columns are amino acid counts A->Z\n");
    ajFmtPrintF(outf,"# Rows are alignment positions 1->n\n");
    ajFmtPrintF(outf,"Simple\n");
    ajFmtPrintF(outf,"Name\t\t%s\n",ajStrGetPtr(name));
    ajFmtPrintF(outf,"Length\t\t%d\n",mlen);
    ajFmtPrintF(outf,"Maximum score\t%d\n",maxscore);
    ajFmtPrintF(outf,"Thresh\t\t%d\n",thresh);
    ajFmtPrintF(outf,"Consensus\t%s\n",ajStrGetPtr(cons));


    for(i=0;i<mlen;++i)
    {
	for(j=0;j<AZ;++j)
	    ajFmtPrintF(outf,"%-2d ",matrix[j][i]);
	ajFmtPrintF(outf,"\n");
    }

    for(i=0;i<AZ+2;++i)
	AJFREE(matrix[i]);

    ajStrDel(&cons);

    return;
}




/* @funcstatic prophecy_gribskov_profile **************************************
**
** Undocumented.
**
** @param [r] seqset [const AjPSeqset] Undocumented
** @param [u] outf [AjPFile] Undocumented
** @param [r] name [const AjPStr] Undocumented
** @param [r] thresh [ajint] Undocumented
** @param [r] gapopen [float] Undocumented
** @param [r] gapextend [float] Undocumented
** @param [w] cons [AjPStr*] Undocumented
** @@
******************************************************************************/

static void prophecy_gribskov_profile(const AjPSeqset seqset,
				      AjPFile outf, const AjPStr name,
				      ajint thresh,
				      float gapopen, float gapextend,
				      AjPStr *cons)
{
    AjPMatrixf imtx = 0;
    AjPSeqCvt cvt = 0;
    AjPStr mname = NULL;
    float **sub = NULL;

    float **mat;
    ajuint nseqs;
    ajuint mlen;
    ajuint i;
    ajuint j;
    static const char *valid="ACDEFGHIKLMNPQRSTVWY";
    const char *p;
    const char *q;
    float score;
    float sum;
    ajint gsum;
    float mmax;
    float pmax;
    float psum;
    ajint start;
    ajint end;
    ajint pos;
    float x;
    ajint px;

    float **weights;
    ajint *gaps;


    mname = ajStrNewC("Epprofile");
    imtx = ajMatrixfNewFile(mname);
    ajStrDel(&mname);

    nseqs = ajSeqsetGetSize(seqset);
    mlen  = ajSeqsetGetLen(seqset);

    sub = ajMatrixfGetMatrix(imtx);
    cvt = ajMatrixfGetCvt(imtx);



    /*
    ** Set gaps to be maximum length of gap that can occur
    ** including that position
    */
    AJCNEW(gaps, mlen);
    for(i=0;i<mlen;++i)
    {
	gsum = 0;
	for(j=0;j<nseqs;++j)
	{
	    p=ajSeqsetGetseqSeqC(seqset,j);
	    if(i>=strlen(p))
		continue;

	    if(ajBasecodeToInt(p[i])!=27)	  /* if not a gap */
		continue;
	    pos = i;

	    while(pos>=0 && ajBasecodeToInt(p[pos])==27)
		--pos;
	    start = ++pos;
	    pos = i;

	    while(pos<(ajint)mlen && ajBasecodeToInt(p[pos])==27)
		++pos;
	    end  = pos-1;
	    gsum = AJMAX(gsum,(end-start)+1);
	    ajDebug("Gribskov gaps pos:%d seq:%d %d..%d (%d)\n",
		    j, i, start, end, gsum);
	}
	gaps[i] = gsum;
	ajDebug("Gribskov gaps[%d] %d\n",
		    i, gsum);
    }


    /* get maximum score in scoring matrix */
    mmax = 0.0;
    p = valid;
    while(*p)
    {
	q = valid;
	while(*q)
	{
	    mmax=(mmax>sub[ajSeqcvtGetCodeK(cvt,*p)][ajSeqcvtGetCodeK(cvt,*q)]) ? mmax :
		sub[ajSeqcvtGetCodeK(cvt,*p)][ajSeqcvtGetCodeK(cvt,*q)];
	    ++q;
	}
	++p;
    }


    /* Create the weight matrix and zero it */
    AJCNEW(weights, mlen);
    for(i=0;i<mlen;++i)
	AJCNEW0(weights[i], GRIBSKOV_LENGTH+1);

    /*
    **  count the number of times each residue occurs at each
    **  position in the alignment
    */
    for(i=0;i<mlen;++i)
	for(j=0;j<nseqs;++j)
	{
	    p = ajSeqsetGetseqSeqC(seqset,j);
	    if(i>=strlen(p))
		continue;
	    weights[i][ajBasecodeToInt(p[i])] += ajSeqsetGetseqWeight(seqset,j);
	}


    px = -INT_MAX;
    for(i=0;i<mlen;++i)
    {
	x = (float)-INT_MAX;

	for(j=0;j<AZ-1;++j)
	    if(weights[i][j]>x)
	    {
		x=weights[i][j];
		px=j;
	    }
	ajStrAppendK(cons,(char)(px+'A'));
    }


    /* Now normalise the weights */
    for(i=0;i<mlen;++i)
	for(j=0;j<GRIBSKOV_LENGTH;++j)
	    weights[i][j] /= (float)nseqs;


    /* Create the profile matrix n*GRIBSKOV_LENGTH and zero it */
    AJCNEW(mat, mlen);
    for(i=0;i<mlen;++i)
	AJCNEW0(mat[i],GRIBSKOV_LENGTH);

    /* Fill the profile with aa scores */
    for(i=0;i<mlen;++i)
	for(p=valid;*p;++p)
	{
	    sum = 0.0;
	    q = valid;
	    while(*q)
	    {
		score = weights[i][ajBasecodeToInt(*q)];
		score *= (float)(sub[ajSeqcvtGetCodeK(cvt,*p)][ajSeqcvtGetCodeK(cvt,*q)]);
		sum += score;
		++q;
	    }
	    mat[i][ajBasecodeToInt(*p)] = sum;
	}

    /* Calculate gap penalties */
    for(i=0;i<mlen;++i)
	mat[i][GRIBSKOV_LENGTH-1]= (mmax / (gapopen+gapextend+(float)gaps[i]));


    /* Get maximum matrix score */
    psum = 0.0;
    for(i=0;i<mlen;++i)
    {
	pmax = (float)-INT_MAX;
	for(j=0;j<AZ;++j)
	    pmax=(pmax>mat[i][j]) ? pmax : mat[i][j];
	psum += pmax;

	ajDebug("matrix score [%d] %.3f psum: %.3f\n",
		i, pmax, psum);
    }

    /* Print matrix */

    ajFmtPrintF(outf,"# Gribskov Protein Profile\n");
    ajFmtPrintF(outf,"# Columns are amino acids A->Z\n");
    ajFmtPrintF(outf,"# Last column is indel penalty\n");
    ajFmtPrintF(outf,"# Rows are alignment positions 1->n\n");
    ajFmtPrintF(outf,"Gribskov\n");
    ajFmtPrintF(outf,"Name\t\t%s\n",ajStrGetPtr(name));
    ajFmtPrintF(outf,"Matrix\t\tpprofile\n");
    ajFmtPrintF(outf,"Length\t\t%d\n",mlen);
    ajFmtPrintF(outf,"Max_score\t%.2f\n",psum);
    ajFmtPrintF(outf,"Threshold\t%d\n",thresh);
    ajFmtPrintF(outf,"Gap_open\t%.2f\n",gapopen);
    ajFmtPrintF(outf,"Gap_extend\t%.2f\n",gapextend);
    ajFmtPrintF(outf,"Consensus\t%s\n",ajStrGetPtr(*cons));

    for(i=0;i<mlen;++i)
    {
	for(j=0;j<GRIBSKOV_LENGTH;++j)
	    ajFmtPrintF(outf,"%.2f ",mat[i][j]);
	ajFmtPrintF(outf,"%.2f\n",mat[i][GRIBSKOV_LENGTH-1]);
    }


    for(i=0;i<mlen;++i)
    {
	AJFREE(mat[i]);
	AJFREE(weights[i]);
    }
    AJFREE(mat);
    AJFREE(weights);

    AJFREE(gaps);

    ajMatrixfDel(&imtx);

    return;
}




/* @funcstatic prophecy_henikoff_profile **************************************
**
** Undocumented.
**
** @param [r] seqset [const AjPSeqset] Undocumented
** @param [r] imtx [const AjPMatrixf] Undocumented
** @param [r] thresh [ajint] Undocumented
** @param [r] cvt [const AjPSeqCvt] Undocumented
** @param [u] outf [AjPFile] Undocumented
** @param [r] name [const AjPStr] Undocumented
** @param [r] gapopen [float] Undocumented
** @param [r] gapextend [float] Undocumented
** @param [w] cons [AjPStr*] Undocumented
** @@
******************************************************************************/


static void prophecy_henikoff_profile(const AjPSeqset seqset,
				      const AjPMatrixf imtx,
				      ajint thresh, const AjPSeqCvt cvt,
				      AjPFile outf, const AjPStr name,
				      float gapopen, float gapextend,
				      AjPStr *cons)
{
    float **sub = NULL;
    float **mat;
    ajuint nseqs;
    ajuint mlen;
    ajuint i;
    ajuint j;
    static const char *valid="ACDEFGHIKLMNPQRSTVWY";
    const char *p;
    const char *q;
    float score;
    float sum;
    float psum;
    float pmax;
    ajint gsum;
    ajint mmax;
    ajint start;
    ajint end;
    ajint pos;

    float **weights;
    ajint *gaps;
    ajint *pcnt;

    float x;
    ajint px;


    nseqs = ajSeqsetGetSize(seqset);
    mlen  = ajSeqsetGetLen(seqset);

    sub = ajMatrixfGetMatrix(imtx);
    cvt = ajMatrixfGetCvt(imtx);


    /*
    ** Set gaps to be maximum length of gap that can occur
    ** including that position
    */
    AJCNEW(gaps, mlen);
    for(i=0;i<mlen;++i)
    {
	gsum = 0;
	for(j=0;j<nseqs;++j)
	{
	    p=ajSeqsetGetseqSeqC(seqset,j);
	    if(i>=strlen(p))
		continue;

	    if(ajBasecodeToInt(p[i])!=27)
		continue; /* if not a gap */

	    pos = i;
	    while(pos>=0 && ajBasecodeToInt(p[pos])==27)
		--pos;
	    start = ++pos;

	    pos = i;
	    while(pos<(ajint)mlen && ajBasecodeToInt(p[pos])==27)
		++pos;
	    end = pos-1;
	    gsum = AJMAX(gsum,(end-start)+1);
	}
	gaps[i] = gsum;
    }

    /* get maximum score in scoring matrix */
    mmax = 0;
    p = valid;
    while(*p)
    {
	q=valid;
	while(*q)
	{
	    mmax = (mmax>sub[ajSeqcvtGetCodeK(cvt,*p)][ajSeqcvtGetCodeK(cvt,*q)]) ? mmax :
		(ajint)sub[ajSeqcvtGetCodeK(cvt,*p)][ajSeqcvtGetCodeK(cvt,*q)];
	    ++q;
	}
	++p;
    }


    /* Create the weight matrix and zero it */
    AJCNEW(weights, mlen);
    for(i=0;i<mlen;++i)
	AJCNEW0(weights[i],HENIKOFF_LENGTH+1);

    /*
    **  count the number of times each residue occurs at each
    **  position in the alignment
    */
    for(i=0;i<mlen;++i)
	for(j=0;j<nseqs;++j)
	{
	    p = ajSeqsetGetseqSeqC(seqset,j);
	    if(i>=strlen(p))
		continue;
	    weights[i][ajBasecodeToInt(p[i])] +=
                ajSeqsetGetseqWeight(seqset,j);
	}

    px = -INT_MAX;
    for(i=0;i<mlen;++i)
    {
	x = (float)-INT_MAX;
	for(j=0;j<AZ-1;++j)
	    if(weights[i][j]>x)
	    {
		x = weights[i][j];
		px=j;
	    }
	ajStrAppendK(cons,(char)(px+'A'));
    }



    /* Count the number of different residues at each position */

    AJCNEW0(pcnt, mlen);

    for(i=0;i<mlen;++i)
	for(j=0;j<HENIKOFF_LENGTH-1;++j)
	    if(weights[i][j])
		++pcnt[i];

    /* weights = 1/(num diff res * count of residues at that position */
    for(i=0;i<mlen;++i)
	for(j=0;j<HENIKOFF_LENGTH-1;++j)
	    if(weights[i][j])
		weights[i][j] = (float)1.0/(weights[i][j]*(float)pcnt[i]);


    /* Create the profile matrix n*HENIKOFF_LENGTH */
    AJCNEW(mat, mlen);
    for(i=0;i<mlen;++i)
      AJCNEW0(mat[i],HENIKOFF_LENGTH);

    /* Fill the profile with aa scores */
    for(i=0;i<mlen;++i)
	for(p=valid;*p;++p)
	{
	    sum = 0.0;
	    q   = valid;
	    while(*q)
	    {
		score = weights[i][ajBasecodeToInt(*q)];
		score *= sub[ajSeqcvtGetCodeK(cvt,*p)]
                            [ajSeqcvtGetCodeK(cvt,*q)];
		sum += score;
		++q;
	    }
	    mat[i][ajBasecodeToInt(*p)] = sum;
	}

    /* Calculate gap penalties */
    for(i=0;i<mlen;++i)
	mat[i][HENIKOFF_LENGTH-1]=(mmax / (gapopen+gapextend+
					  (float)gaps[i]));


    /* Get maximum matrix score */
    psum = 0.0;
    for(i=0;i<mlen;++i)
    {
	pmax = (float)-INT_MAX;
	for(j=0;j<HENIKOFF_LENGTH-1;++j)
	    pmax=(pmax>mat[i][j]) ? pmax : mat[i][j];
	psum += pmax;
    }

    /* Print matrix */

    ajFmtPrintF(outf,"# Henikoff Protein Profile\n");
    ajFmtPrintF(outf,"# Columns are amino acids A->Z\n");
    ajFmtPrintF(outf,"# Last column is indel penalty\n");
    ajFmtPrintF(outf,"# Rows are alignment positions 1->n\n");
    ajFmtPrintF(outf,"Henikoff\n");
    ajFmtPrintF(outf,"Name\t\t%s\n",ajStrGetPtr(name));
    ajFmtPrintF(outf,"Matrix\t\t%s\n",ajStrGetPtr(ajMatrixfGetName(imtx)));
    ajFmtPrintF(outf,"Length\t\t%d\n",mlen);
    ajFmtPrintF(outf,"Max_score\t%.2f\n",psum);
    ajFmtPrintF(outf,"Threshold\t%d\n",thresh);
    ajFmtPrintF(outf,"Gap_open\t%.2f\n",gapopen);
    ajFmtPrintF(outf,"Gap_extend\t%.2f\n",gapextend);
    ajFmtPrintF(outf,"Consensus\t%s\n",ajStrGetPtr(*cons));

    for(i=0;i<mlen;++i)
    {
	for(j=0;j<HENIKOFF_LENGTH;++j)
	    ajFmtPrintF(outf,"%.2f ",mat[i][j]);
	ajFmtPrintF(outf,"%.2f\n",mat[i][j-1]);
    }


    for(i=0;i<mlen;++i)
    {
	AJFREE(mat[i]);
	AJFREE(weights[i]);
    }
    AJFREE(mat);
    AJFREE(weights);
    AJFREE(gaps);
    AJFREE(pcnt);

    return;
}

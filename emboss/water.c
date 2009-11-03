/* @source water application
**
** True Smith-Waterman best local alignment
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




/* @prog water ****************************************************************
**
** Smith-Waterman local alignment
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPAlign align;
    AjPSeqall seqall;
    AjPSeq a;
    AjPSeq b;
    AjPStr m;
    AjPStr n;
    AjPStr ss;

    AjBool  show = ajFalse;

    ajuint lena;
    ajuint lenb;

    const char *p;
    const char *q;

    ajint start1 = 0;
    ajint start2 = 0;


    float *path;
    ajint *compass;

    AjPMatrixf matrix;
    AjPSeqCvt cvt = 0;
    float **sub;

    float gapopen;
    float gapextend;

    ajulong maxarr = 1000;
    ajulong len;

    float score;
    ajint begina;
    ajint beginb;

    AjBool dobrief = ajTrue;
    AjPStr tmpstr  = NULL;

    float id   = 0.;
    float sim  = 0.;
    float idx  = 0.;
    float simx = 0.;

    size_t stlen;

    embInit("water", argc, argv);
    
    matrix    = ajAcdGetMatrixf("datafile");
    a         = ajAcdGetSeq("asequence");
    ajSeqTrim(a);
    seqall    = ajAcdGetSeqall("bsequence");
    gapopen   = ajAcdGetFloat("gapopen");
    gapextend = ajAcdGetFloat("gapextend");
    dobrief   = ajAcdGetBoolean("brief");
    align     = ajAcdGetAlign("outfile");
    
    /* obsolete. Can be uncommented in acd file and here to reuse */
    
    /* show      = ajAcdGetBoolean("showinternals"); */
    
    m  = ajStrNew();
    n  = ajStrNew();
    ss = ajStrNew();
    
    gapopen = ajRoundF(gapopen, 8);
    gapextend = ajRoundF(gapextend, 8);
    
    AJCNEW(path, maxarr);
    AJCNEW(compass, maxarr);
    
    sub = ajMatrixfArray(matrix);
    cvt = ajMatrixfCvt(matrix);
    
    begina=ajSeqGetBegin(a)+ajSeqGetOffset(a);
    
    while(ajSeqallNext(seqall,&b))
    {
	lena = ajSeqGetLen(a);
	ajSeqTrim(b);
	lenb = ajSeqGetLen(b);

	if(lenb > (ULONG_MAX/(ajulong)(lena+1)))
	   ajFatal("Sequences too big. Try 'matcher' or 'supermatcher'");

	len = lena*lenb;

	if(len>maxarr)
	{
	    stlen = (size_t) len;
	    AJCRESIZE(path,stlen);
	    if(!path)
		ajFatal("Sequences too big. Try 'matcher' or 'supermatcher'");
	    AJCRESIZE(compass,stlen);
	    if(!compass)
		ajFatal("Sequences too big. Try 'matcher' or 'supermatcher'");
	    maxarr=len;
	}

	beginb=ajSeqGetBegin(b)+ajSeqGetOffset(b);

	p = ajSeqGetSeqC(a);
	q = ajSeqGetSeqC(b);

	ajStrAssignC(&m,"");
	ajStrAssignC(&n,"");

	score = embAlignPathCalcSW(p,q,lena,lenb,gapopen,gapextend,path,sub,cvt,
			   compass,show);

	/*score=embAlignScoreSWMatrix(path,compass,gapopen,gapextend,a,b,lena,
          lenb,sub,cvt,&start1,&start2);*/

	embAlignWalkSWMatrix(path,compass,gapopen,gapextend,a,b,&m,&n,
			     lena,lenb,&start1,&start2);

	ajDebug("ReportLocal call start1:%d begina:%d start2:%d beginb:%d\n",
		start1, begina, start2, beginb);
	embAlignReportLocal(align, a, b, m, n,
			    start1, start2,
			    gapopen, gapextend,
			    score, matrix, begina, beginb);
	if(!dobrief)
	{
	    embAlignCalcSimilarity(m,n,sub,cvt,lena,lenb,&id,&sim,&idx,
				   &simx);
	    ajFmtPrintAppS(&tmpstr,"Longest_Identity = %5.2f%%\n",
			   id);
	    ajFmtPrintAppS(&tmpstr,"Longest_Similarity = %5.2f%%\n",
			   sim);
	    ajFmtPrintAppS(&tmpstr,"Shortest_Identity = %5.2f%%\n",
			   idx);
	    ajFmtPrintAppS(&tmpstr,"Shortest_Similarity = %5.2f%%",
			   simx);
	    ajAlignSetSubHeaderApp(align, tmpstr);
	}
	ajAlignWrite(align);
	ajAlignReset(align);

    }
    
    ajAlignClose(align);
    ajAlignDel(&align);
    
    AJFREE(compass);
    AJFREE(path);
    
    AJFREE(compass);
    AJFREE(path);
    
    ajStrDel(&n);
    ajStrDel(&m);
    ajStrDel(&ss);
    ajStrDel(&tmpstr);
    ajSeqallDel(&seqall);
    ajSeqDel(&a);
    ajSeqDel(&b);

    embExit();

    return 0;
}

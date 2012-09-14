/* @source needle application
**
** True Needleman-Wunsch global alignment
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




/* @prog needle ***************************************************************
**
** Needleman-Wunsch global alignment
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPAlign align;
    AjPSeqall seqall;
    AjPSeq a;
    AjPSeq b;
    AjPStr alga;
    AjPStr algb;
    AjPStr ss;

    ajuint    lena;
    ajuint    lenb;

    const char   *p;
    const char   *q;

    ajint start1 = 0;
    ajint start2 = 0;

    ajint *compass;
    float* ix;
    float* iy;
    float* m;

    AjPMatrixf matrix;
    AjPSeqCvt cvt = 0;
    float **sub;

    float gapopen;
    float gapextend;
    float endgapopen;
    float endgapextend;
    size_t maxarr = 1000; 	/* arbitrary. realloc'd if needed */
    size_t len;

    float score;

    AjBool dobrief = ajTrue;
    AjBool endweight = ajFalse; /* should end gap penalties be applied */

    float id   = 0.;
    float sim  = 0.;
    float idx  = 0.;
    float simx = 0.;

    AjPStr tmpstr = NULL;

    embInit("needle", argc, argv);

    matrix    = ajAcdGetMatrixf("datafile");
    a         = ajAcdGetSeq("asequence");
    ajSeqTrim(a);
    seqall    = ajAcdGetSeqall("bsequence");
    gapopen   = ajAcdGetFloat("gapopen");
    gapextend = ajAcdGetFloat("gapextend");
    endgapopen   = ajAcdGetFloat("endopen");
    endgapextend = ajAcdGetFloat("endextend");
    dobrief   = ajAcdGetBoolean("brief");
    endweight   = ajAcdGetBoolean("endweight");

    align     = ajAcdGetAlign("outfile");

    gapopen = ajRoundFloat(gapopen, 8);
    gapextend = ajRoundFloat(gapextend, 8);

    AJCNEW0(compass, maxarr);
    AJCNEW0(m, maxarr);
    AJCNEW0(ix, maxarr);
    AJCNEW0(iy, maxarr);

    alga  = ajStrNew();
    algb  = ajStrNew();
    ss = ajStrNew();

    sub = ajMatrixfGetMatrix(matrix);
    cvt = ajMatrixfGetCvt(matrix);

    lena = ajSeqGetLen(a);

    while(ajSeqallNext(seqall,&b))
    {
	ajSeqTrim(b);
	lenb = ajSeqGetLen(b);

	if(lenb > (LONG_MAX/(size_t)(lena+1)))
	   ajDie("Sequences too big. Try 'stretcher'");

	len = (size_t)lena*(size_t)lenb;

	if(len>maxarr)
	{
	    AJCRESIZETRY0(compass,(size_t)maxarr,len);
	    if(!compass)
		ajDie("Sequences too big. Try 'stretcher'");
	    AJCRESIZETRY0(m,(size_t)maxarr,len);
	    if(!m)
		ajDie("Sequences too big. Try 'stretcher'");
	    AJCRESIZETRY0(ix,(size_t)maxarr,len);
	    if(!ix)
		ajDie("Sequences too big. Try 'stretcher'");
	    AJCRESIZETRY0(iy,(size_t)maxarr,len);
	    if(!iy)
		ajDie("Sequences too big. Try 'stretcher'");
	    maxarr=len;
	}


	p = ajSeqGetSeqC(a);
	q = ajSeqGetSeqC(b);

	ajStrAssignC(&alga,"");
	ajStrAssignC(&algb,"");

	score = embAlignPathCalcWithEndGapPenalties(p, q, lena, lenb,
	        gapopen, gapextend, endgapopen, endgapextend,
	        &start1, &start2, sub, cvt,
	        m, ix, iy, compass, ajFalse, endweight);



	embAlignWalkNWMatrixUsingCompass(p, q, &alga, &algb,
	        lena, lenb, &start1, &start2,
	        compass);
		
	embAlignReportGlobal(align, a, b, alga, algb,
			     start1, start2,
			     gapopen, gapextend,
			     score, matrix,
			     ajSeqGetOffset(a), ajSeqGetOffset(b));

	if(!dobrief)
	{
	  embAlignCalcSimilarity(alga,algb,sub,cvt,lena,lenb,&id,&sim,&idx,
				 &simx);
	  ajFmtPrintS(&tmpstr,"Longest_Identity = %5.2f%%\n",
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

    ajSeqallDel(&seqall);
    ajSeqDel(&a);
    ajSeqDel(&b);

    AJFREE(compass);
    AJFREE(ix);
    AJFREE(iy);
    AJFREE(m);

    ajStrDel(&alga);
    ajStrDel(&algb);
    ajStrDel(&ss);
    ajStrDel(&tmpstr);

    embExit();

    return 0;
}

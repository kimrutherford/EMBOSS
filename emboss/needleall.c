/* @source needleall application
**
** Many-to-many pairwise alignment
**
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




/* @prog needleall ***********************************************************
**
** Many-to-many pairwise sequence alignment using Needleman-Wunsch algorithm
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPAlign align;
    AjPSeqall seqall;
    AjPSeqset seqset;
    const AjPSeq seqa;
    AjPSeq seqb;
    AjPStr alga;
    AjPStr algb;
    AjPStr ss;
    AjPFile errorf;

    ajuint lena;
    ajuint lenb;
    ajuint k;

    const char *p;
    const char *q;

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
    float minscore;
    size_t maxarr = 1000;  /* arbitrary. realloc'd if needed */
    size_t len;

    float score;

    AjBool dobrief = ajTrue;
    AjBool endweight = ajFalse; /*whether end gap penalties should be applied*/

    float id   = 0.;
    float sim  = 0.;
    float idx  = 0.;
    float simx = 0.;

    AjPStr tmpstr = NULL;

    embInit("needleall", argc, argv);

    matrix    = ajAcdGetMatrixf("datafile");
    seqset    = ajAcdGetSeqset("asequence");
    ajSeqsetTrim(seqset);
    seqall    = ajAcdGetSeqall("bsequence");
    gapopen   = ajAcdGetFloat("gapopen");
    gapextend = ajAcdGetFloat("gapextend");
    endgapopen   = ajAcdGetFloat("endopen");
    endgapextend = ajAcdGetFloat("endextend");
    minscore = ajAcdGetFloat("minscore");
    dobrief   = ajAcdGetBoolean("brief");
    endweight   = ajAcdGetBoolean("endweight");
    align     = ajAcdGetAlign("outfile");
    errorf    = ajAcdGetOutfile("errfile");

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

    while(ajSeqallNext(seqall,&seqb))
    {
        ajSeqTrim(seqb);
        lenb = ajSeqGetLen(seqb);

        for(k=0;k<ajSeqsetGetSize(seqset);k++)
        {
            seqa = ajSeqsetGetseqSeq(seqset, k);
            lena = ajSeqGetLen(seqa);


            if(lenb > (LONG_MAX/(size_t)(lena+1)))
                ajFatal("Sequences too big.");

            len = (size_t)lena*(size_t)lenb;

            if(len>maxarr)
            {
                AJCRESIZETRY0(compass,(size_t)maxarr,len);
                if(!compass)
                    ajDie("Sequences too big, memory allocation failed");
                AJCRESIZETRY0(m,(size_t)maxarr,len);
                if(!m)
                    ajDie("Sequences too big, memory allocation failed");
                AJCRESIZETRY0(ix,(size_t)maxarr,len);
                if(!ix)
                    ajDie("Sequences too big, memory allocation failed");
                AJCRESIZETRY0(iy,(size_t)maxarr,len);
                if(!iy)
                    ajDie("Sequences too big, memory allocation failed");
                maxarr=len;
            }


            p = ajSeqGetSeqC(seqa);
            q = ajSeqGetSeqC(seqb);

            ajStrAssignC(&alga,"");
            ajStrAssignC(&algb,"");

            score = embAlignPathCalcWithEndGapPenalties(p, q, lena, lenb,
                    gapopen, gapextend, endgapopen, endgapextend,
                    &start1, &start2, sub, cvt, m, ix, iy,
                    compass, ajFalse, endweight);

            embAlignWalkNWMatrixUsingCompass(p, q, &alga, &algb,
                    lena, lenb, &start1, &start2, compass);

            if (score > minscore){
                if(!ajAlignFormatShowsSequences(align))
                {
                    ajAlignDefineCC(align, ajStrGetPtr(alga),
                            ajStrGetPtr(algb), ajSeqGetNameC(seqa),
                            ajSeqGetNameC(seqb));
                    ajAlignSetScoreR(align, score);
                }
                else
                {
                    embAlignReportGlobal(align, seqa, seqb, alga, algb,
                            start1, start2,
                            gapopen, gapextend,
                            score, matrix,
                            ajSeqGetOffset(seqa), ajSeqGetOffset(seqb));
                }

                if(!dobrief)
                {
                    embAlignCalcSimilarity(alga,algb,sub,cvt,lena,lenb,&id,
                            &sim, &idx, &simx);
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
            else
                ajFmtPrintF(errorf,
                        "Alignment score (%.1f) is less than minimum score"
                        "(%.1f) for sequences %s vs %s\n",
                        score, minscore, ajSeqGetNameC(seqa),
                        ajSeqGetNameC(seqb));
        }
    }
    

    if(!ajAlignFormatShowsSequences(align))
    {
        ajMatrixfDel(&matrix);        
    }

    ajAlignClose(align);
    ajAlignDel(&align);
    ajFileClose(&errorf);


    ajSeqallDel(&seqall);
    ajSeqsetDel(&seqset);
    ajSeqDel(&seqb);

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

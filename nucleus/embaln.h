/* @include embaln ************************************************************
**
** General routines for alignment.
**
** @author Copyright (c) 1999 Alan Bleasby
** @version $Revision: 1.31 $
** @modified $Date: 2011/10/18 14:24:24 $ by $Author: rice $
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

#ifndef EMBALN_H
#define EMBALN_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajstr.h"
#include "ajseqdata.h"
#include "ajalign.h"
#include "ajfile.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




#define PAZ  26
#define PAZ1 27




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */



/*
** Prototype definitions
*/

void embAlignCalcSimilarity(const AjPStr m, const AjPStr n,
			    const AjFloatArray *sub, const AjPSeqCvt cvt,
			    ajint lenm, ajint lenn, float *id, float *sim,
			    float *idx, float *simx);

float embAlignPathCalc(const char *a, const char *b,
                       ajint lena, ajint lenb, float gapopen,
                       float gapextend, float *path,
                       float * const *sub, const AjPSeqCvt cvt,
                       ajint *compass, AjBool show);

float embAlignPathCalcWithEndGapPenalties(const char *a, const char *b,
                       ajint lena, ajint lenb,
                       float gapopen, float gapextend,
                       float endgapopen, float endgapextend,
                       ajint *start1, ajint *start2,
                       float * const *sub, const AjPSeqCvt cvt,
                       float *m, float *ix, float *iy,
                       ajint *compass, AjBool show,
                       AjBool endweight);

float embAlignPathCalcSW(const char *a, const char *b, ajint lena, ajint lenb,
                         float gapopen, float gapextend, float *path,
                         float * const *sub, const AjPSeqCvt cvt,
                         ajint *compass,
                         AjBool show);

void embAlignPrintGlobal(AjPFile outf, const char *a, const char *b,
			 const AjPStr m, const AjPStr n,
			 ajint start1, ajint start2, float score, AjBool mark,
			 float * const *sub, const AjPSeqCvt cvt,
			 const char *namea,
			 const char *nameb, ajint begina, ajint beginb);
void embAlignPrintProfile(AjPFile outf,
			  const AjPStr m, const AjPStr n,
			  ajint start1, ajint start2, float score, AjBool mark,
			  float * const *fmatrix, const char *namea,
			  const char *nameb, ajint begina, ajint beginb);

float embAlignProfilePathCalc(const char *a,
                              ajint mlen, ajint slen, float gapopen,
                              float gapextend, float *path,
                              float * const *fmatrix,
                              ajint *compass, AjBool show);

void embAlignReportGlobal (AjPAlign align,
			   const AjPSeq seqa, const AjPSeq seqb,
			   const AjPStr m, const AjPStr n,
			   ajint start1, ajint start2,
			   float gapopen, float gapextend,
			   float score, AjPMatrixf matrix,
			   ajint offset1, ajint offset2);
void embAlignReportLocal (AjPAlign align,
			  const AjPSeq seqa, const AjPSeq seqb,
			  const AjPStr m, const AjPStr n,
			  ajint start1, ajint start2,
			  float gapopen, float gapextend,
			  float score, AjPMatrixf matrix,
			  ajint offset1, ajint offset2);
void embAlignReportProfile(AjPAlign align,
			   const AjPStr m, const AjPStr n,
			   ajint start1, ajint start2,
			   float score,
			   const char *namea, const char *nameb);

/*float embAlignScoreProfileMatrix(const float *path, const ajint *compass,
				 float gapopen,
				 float gapextend, const AjPStr b,
				 ajint clen, ajint slen,
				 float * const *fmatrix,
				 ajint *start1, ajint *start2);
*/
void embAlignWalkNWMatrix(const float *path,
                          const AjPSeq a, const AjPSeq b,
			  AjPStr *m, AjPStr *n,
                          ajint lena, ajint lenb,
                          ajint *start1, ajint *start2,
                          float gapopen, float gapextend,
                          const ajint *compass);

void embAlignWalkNWMatrixUsingCompass(const char* p, const char* q,
        AjPStr *m, AjPStr *n,
        ajuint lena, ajuint lenb,
        ajint *start1, ajint *start2,
        ajint const *compass);

void embAlignWalkProfileMatrix(const float *path, const ajint *compass,
			       float gapopen, float gapextend,
			       const AjPStr cons, const AjPStr b,
			       AjPStr *m, AjPStr *n,
                               ajint clen, ajint slen,
			       float * const *pmatrix,
			       ajint *start1, ajint *start2);

void embAlignWalkSWMatrix(const float *path, const ajint *compass,
			  float gapopen, float gapextend,
                          const AjPSeq a, const AjPSeq b,
			  AjPStr *m, AjPStr *n,
                          ajint lena, ajint lenb,
			  ajint *start1, ajint *start2);

float embAlignPathCalcSWFast(const char *a, const char *b,
                             ajint lena, ajint lenb,
                             ajint offset, ajint width,
                             float gapopen, float gapextend, float *path,
                             float * const *sub, const AjPSeqCvt cvt,
                             ajint *compass, AjBool show);

void embAlignWalkSWMatrixFast(const float *path, const ajint *compass,
			      float gapopen, float gapextend,
                              const AjPSeq a, const AjPSeq b,
			      AjPStr *m, AjPStr *n,
                              ajint lena, ajint lenb,
			      ajint offset, ajint width,
                              ajint *start1, ajint *start2);

void embAlignUnused(void);

/*
** End of prototype definitions
*/

#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif
#ifdef AJ_COMPILE_DEPRECATED

__deprecated void embAlignPrintLocal(AjPFile outf,
                                     const AjPStr m, const AjPStr n,
                                     ajint start1, ajint start2,
                                     float score, AjBool mark,
                                     float * const *sub, const AjPSeqCvt cvt,
                                     const char *namea, const char *nameb,
                                     ajint begina, ajint beginb);

#endif

AJ_END_DECLS

#endif  /* !EMBALN_H */

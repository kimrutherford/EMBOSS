#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embaln_h
#define embaln_h

#define PAZ  26
#define PAZ1 27




/*
** Prototype definitions
*/

void embAlignCalcSimilarity(const AjPStr m, const AjPStr n,
			    const AjFloatArray *sub, const AjPSeqCvt cvt,
			    ajint lenm, ajint lenn, float *id, float *sim,
			    float *idx, float *simx);

void embAlignPathCalc(const char *a, const char *b,
		      ajint lena, ajint lenb, float gapopen,
		      float gapextend, float *path,
		      float * const *sub, const AjPSeqCvt cvt,
		      ajint *compass, AjBool show);

void embAlignPathCalcSW(const char *a, const char *b, ajint lena, ajint lenb,
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
void embAlignPrintLocal(AjPFile outf,
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

void embAlignProfilePathCalc(const char *a,
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

float embAlignScoreNWMatrix(const float *path, const AjPSeq a, const AjPSeq b,
			    float * const *fmatrix,
			    const AjPSeqCvt cvt, ajint lena, ajint lenb,
			    float gapopen, const ajint *compass,
			    float gapextend, ajint *start1, ajint *start2);

float embAlignScoreProfileMatrix(const float *path, const ajint *compass,
				 float gapopen,
				 float gapextend, const AjPStr b,
				 ajint clen, ajint slen,
				 float * const *fmatrix,
				 ajint *start1, ajint *start2);
float embAlignScoreSWMatrix(const float *path, const ajint *compass,
			    float gapopen,
			    float gapextend,  const AjPSeq a, const AjPSeq b,
			    ajint lena, ajint lenb, float * const *sub,
			    const AjPSeqCvt cvt, ajint *start1, ajint *start2);

void embAlignWalkNWMatrix(const float *path, const AjPSeq a, const AjPSeq b,
			  AjPStr *m,
			  AjPStr *n, ajint lena, ajint lenb, ajint *start1,
			  ajint *start2, float gapopen,
			  float gapextend, const AjPSeqCvt cvt,
			  const ajint *compass, float * const *sub);
void embAlignWalkProfileMatrix(const float *path, const ajint *compass,
			       float gapopen,
			       float gapextend,
			       const AjPStr cons, const AjPStr b,
			       AjPStr *m, AjPStr *n, ajint clen, ajint slen,
			       float * const *fmatrix,
			       ajint *start1, ajint *start2);
void embAlignWalkSWMatrix(const float *path, const ajint *compass,
			  float gapopen,
			  float gapextend, const AjPSeq a, const AjPSeq b,
			  AjPStr *m,
			  AjPStr *n, ajint lena, ajint lenb,
			  float * const *sub,
			  const AjPSeqCvt cvt, ajint *start1, ajint *start2);

void embAlignPathCalcFast(const char *a, const char *b, ajint lena, ajint lenb,
			  float gapopen, float gapextend, float *path,
			  float * const *sub, const AjPSeqCvt cvt,
			  ajint *compass, AjBool show, ajint width);

float embAlignScoreSWMatrixFast(const float *path, const ajint *compass,
				float gapopen,
				float gapextend,
				const AjPSeq a, const AjPSeq b,
				ajint lena, ajint lenb,
				float * const *sub,
				const AjPSeqCvt cvt,
				ajint *start1, ajint *start2,
				ajint width);

void embAlignWalkSWMatrixFast(const float *path, const ajint *compass,
			      float gapopen,
			      float gapextend, const AjPSeq a, const AjPSeq b,
			      AjPStr *m,
			      AjPStr *n, ajint lena, ajint lenb,
			      float * const *sub,
			      const AjPSeqCvt cvt,
			      ajint *start1, ajint *start2,
			      ajint width);

void embAlignUnused(void);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif

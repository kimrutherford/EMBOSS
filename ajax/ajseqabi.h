#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajseqabi_h
#define ajseqabi_h




/*
** Prototype definitions
*/

ajint      ajSeqABIGetBaseOffset(AjPFile fp);
void       ajSeqABIGetBasePosition(AjPFile fp,ajlong numBases,
				   AjPShort* basePositions);
ajint      ajSeqABIGetBasePosOffset(AjPFile fp);
float      ajSeqABIGetBaseSpace(AjPFile fp);
void       ajSeqABIGetData(AjPFile fp,const ajlong *Offset,ajlong numPoints,
			   AjPInt2d trace);
ajint      ajSeqABIGetFWO(AjPFile fp);
ajint      ajSeqABIGetNBase(AjPFile fp);
ajint      ajSeqABIGetNData(AjPFile fp);
ajint      ajSeqABIGetPrimerOffset(AjPFile fp);
ajint      ajSeqABIGetPrimerPosition(AjPFile fp);
void       ajSeqABIGetSignal(AjPFile fp,ajlong fwo_,
			     ajshort *sigC,ajshort *sigA,
			     ajshort *sigG,ajshort *sigT);
AjBool     ajSeqABIGetTraceOffset(AjPFile fp, ajlong *Offset);
AjBool     ajSeqABIMachineName(AjPFile fp,AjPStr *machine);
AjBool     ajSeqABIReadSeq(AjPFile fp,ajlong baseO,ajlong numBases,
			   AjPStr* nseq);
AjBool     ajSeqABISampleName(AjPFile fp, AjPStr *sample);
AjBool     ajSeqABITest(AjPFile fp);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif

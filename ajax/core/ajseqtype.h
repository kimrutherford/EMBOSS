#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajseqtype_h
#define ajseqtype_h




/*
** Prototype definitions
*/

void         ajSeqGap (AjPSeq thys, char gapc, char padc);
void         ajSeqGapLen (AjPSeq thys, char gapc, char padc, ajint ilen);
void         ajSeqGapS (AjPStr* seq, char gapc);
void         ajSeqPrintType (AjPFile outf, AjBool full);
void         ajSeqSetNuc (AjPSeq thys);
void         ajSeqSetProt (AjPSeq thys);
void         ajSeqsetSetNuc (AjPSeqset thys);
void         ajSeqsetSetProt (AjPSeqset thys);

char         ajSeqTypeAnyprotS (const AjPStr pthys);
char         ajSeqTypeDnaS (const AjPStr pthys);
void         ajSeqTypeExit(void);
char         ajSeqTypeGapanyS (const AjPStr pthys);
char         ajSeqTypeGapdnaS (const AjPStr pthys);
char         ajSeqTypeGapnucS (const AjPStr pthys);
char         ajSeqTypeGaprnaS (const AjPStr pthys);
char         ajSeqTypeNucS (const AjPStr pthys);
char         ajSeqTypeProtS (const AjPStr pthys);
AjBool       ajSeqTypeSummary(const AjPStr type_name,
			      AjPStr* Ptype, AjBool* gaps);
char         ajSeqTypeRnaS (const AjPStr pthys);

AjBool       ajSeqTypeIsAny (const AjPStr type_name);
AjBool       ajSeqTypeIsNuc (const AjPStr type_name);
AjBool       ajSeqTypeIsProt (const AjPStr type_name);


void         ajSeqType (AjPSeq thys);
AjBool       ajSeqTypeCheckIn (AjPSeq thys, const AjPSeqin seqin);
AjBool       ajSeqTypeCheckS (AjPStr* pthys, const AjPStr type_name);

void         ajSeqTypeUnused(void);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif

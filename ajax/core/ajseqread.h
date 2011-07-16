#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajseqread_h
#define ajseqread_h


extern AjPTable seqDbMethods;



/*
** Prototype definitions
*/

AjPTable     ajSeqaccessGetDb(void);
AjBool       ajSeqaccessMethodTest (const AjPStr method);
const char*  ajSeqaccessMethodGetQlinks(const AjPStr method);
ajuint       ajSeqaccessMethodGetScope(const AjPStr method);
AjPSeqall    ajSeqallFile (const AjPStr usa);
AjBool       ajSeqAllRead (AjPSeq thys, AjPSeqin seqin);
AjBool       ajSeqGetFromUsa (const AjPStr thys, AjBool protein, AjPSeq seq);
AjBool       ajSeqGetFromUsaRange(const AjPStr thys, AjBool protein,
			          ajint ibegin, ajint iend, AjPSeq seq);
AjBool       ajSeqFormatTest (const AjPStr format);
void         ajSeqinClear (AjPSeqin thys);
void         ajSeqinClearPos(AjPSeqin thys);
void         ajSeqinDel (AjPSeqin* pthis);
AjPSeqin     ajSeqinNew (void);
void         ajSeqinSetNuc (AjPSeqin seqin);
void         ajSeqinSetProt (AjPSeqin seqin);
void         ajSeqinSetRange (AjPSeqin seqin, ajint ibegin, ajint iend);
void         ajSeqinUsa (AjPSeqin* pthis, const AjPStr Usa);
void         ajSeqinTrace (const AjPSeqin thys);
const char*  ajSeqinTypeGetFields(void);
const char*  ajSeqinTypeGetQlinks(void);

AjBool       ajSeqParseFasta(const AjPStr str, AjPStr* id, AjPStr* acc,
			     AjPStr* sv, AjPStr* desc);
AjBool       ajSeqParseFastq(const AjPStr str, AjPStr* id, AjPStr* desc);
AjBool       ajSeqParseNcbi(const AjPStr str, AjPStr* id, AjPStr* acc,
			    AjPStr* sv, AjPStr* gi, AjPStr* db, AjPStr* desc);
void         ajSeqPrintInFormat (AjPFile outf, AjBool full);
void         ajSeqPrintbookInFormat (AjPFile outf);
void         ajSeqPrinthtmlInFormat (AjPFile outf);
void         ajSeqPrintwikiInFormat (AjPFile outf);
AjBool       ajSeqRead (AjPSeq thys, AjPSeqin seqin);
void         ajSeqReadExit(void);
ajint        ajSeqsetApp (AjPSeqset thys, const AjPSeq seq);
AjBool       ajSeqsetGetFromUsa(const AjPStr thys, AjPSeqset *seq);
AjBool       ajSeqsetFromList (AjPSeqset thys, const AjPList list);
ajint        ajSeqsetFromPair (AjPSeqset thys,
			       const AjPSeq seqa, const AjPSeq seqb);
AjBool       ajSeqsetallRead (AjPList thys, AjPSeqin seqin);
AjBool       ajSeqsetRead (AjPSeqset thys, AjPSeqin seqin);
AjBool       ajSeqUsaGetBase(const AjPStr usa, AjPStr* baseusa);
AjBool       ajSeqinformatTerm(const AjPStr term);
AjBool       ajSeqinformatTest(const AjPStr format);

/*
** End of prototype definitions
*/

__deprecated AjBool       ajSeqMethodTest (const AjPStr method);
__deprecated ajuint       ajSeqMethodGetScope(const AjPStr method);

#endif

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajseqread_h
#define ajseqread_h






/*
** Prototype definitions
*/

AjPSeqall    ajSeqallFile (const AjPStr usa);
AjBool       ajSeqAllRead (AjPSeq thys, AjPSeqin seqin);
AjBool       ajSeqGetFromUsa (const AjPStr thys, AjBool protein, AjPSeq seq);
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

AjBool       ajSeqParseFasta(const AjPStr str, AjPStr* id, AjPStr* acc,
			     AjPStr* sv, AjPStr* desc);
AjBool       ajSeqParseFastq(const AjPStr str, AjPStr* id, AjPStr* acc,
			     AjPStr* sv, AjPStr* desc);
AjBool       ajSeqParseNcbi(const AjPStr str, AjPStr* id, AjPStr* acc,
			    AjPStr* sv, AjPStr* gi, AjPStr* db, AjPStr* desc);
void         ajSeqQueryClear (AjPSeqQuery thys);
void         ajSeqQueryDel (AjPSeqQuery *pthis);
AjBool       ajSeqQueryIs (const AjPSeqQuery qry);
AjPSeqQuery  ajSeqQueryNew (void);
void         ajSeqQueryStarclear (AjPSeqQuery qry);
void         ajSeqQueryTrace (const AjPSeqQuery qry);
AjBool       ajSeqQueryWild (AjPSeqQuery qry);
void         ajSeqPrintInFormat (AjPFile outf, AjBool full);
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

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*
**
** ajfeat.h - AJAX Sequence Feature include file
**            Version 2.0 - June 2001
**
*/

#ifndef ajfeat_h
#define ajfeat_h

#include <stdlib.h>
#include <stdio.h>

#include "ajdefine.h"
#include "ajexcept.h"
#include "ajmem.h"
#include "ajreg.h"
#include "ajstr.h"
#include "ajfile.h"
#include "ajtime.h"
#include "ajfmt.h"
#include "ajfeatdata.h"
#include "ajseqdata.h"

/* ========================================================================= */
/* ========== All functions in (more or less) alphabetical order =========== */
/* ========================================================================= */

/*
** Prototype definitions
*/

AjBool        ajFeatIsChild (const AjPFeature gf);
AjPFeature    ajFeatNewFeat (const AjPFeature orig);
void          ajFeatDel(AjPFeature *pthis) ;
void          ajFeatExit (void);
ajuint        ajFeatGetEnd (const AjPFeature thys);
AjBool        ajFeatGetForward (const AjPFeature thys);
ajint         ajFeatGetFrame (const AjPFeature thys);
ajuint        ajFeatGetLength(const AjPFeature thys);
AjBool        ajFeatGetNoteC(const AjPFeature thys, const char* name,
			     AjPStr* val);
AjBool        ajFeatGetNoteCI(const AjPFeature thys, const char* name,
			      ajint count, AjPStr* val);
AjBool        ajFeatGetNoteSI (const AjPFeature thys, const AjPStr name,
                               ajint count, AjPStr* val);
AjBool        ajFeatGetNoteS (const AjPFeature thys, const AjPStr name,
			     AjPStr* val);
AjBool        ajFeatGetRemoteseq(const AjPFeature thys, const AjPStr usa,
                                 AjPSeq seq);
float         ajFeatGetScore (const AjPFeature thys);
const AjPStr  ajFeatGetSource (const AjPFeature thys);
AjBool        ajFeatGetFlags(const AjPFeature thys,  AjPStr* Pflagstr);
AjBool        ajFeatGetSeq(const AjPFeature feat,
                           const AjPSeq seq, AjPStr* seqstr);
AjBool        ajFeatGetSeqJoin(const AjPFeature thys, const AjPFeattable table,
                               const AjPSeq seq, AjPStr* Pseqstr);
ajuint        ajFeatGetStart (const AjPFeature thys);
char          ajFeatGetStrand (const AjPFeature thys);
AjBool        ajFeatGetTagC(const AjPFeature thys, const char* tname,
			    ajint num, AjPStr* val);
AjBool        ajFeatGetTagS(const AjPFeature thys, const AjPStr name,
			    ajint num, AjPStr* val);
AjBool        ajFeatGetTranslation(const AjPFeature feat, AjPStr* seqstr);
const AjPStr  ajFeatGetType (const AjPFeature thys);
AjBool        ajFeatGetXrefs(const AjPFeature thys, AjPList *Pxreflist);
AjBool        ajFeatIsCompMult (const AjPFeature gf);
AjBool        ajFeatIsLocal (const AjPFeature gf);
AjBool        ajFeatIsLocalRange (const AjPFeature gf,
				  ajuint start, ajuint end);
AjBool        ajFeatIsMultiple (const AjPFeature gf);
AjBool        ajFeatLocMark(const AjPFeature thys, const AjPFeattable table,
                            AjPStr* Pseqstr);
AjPFeature    ajFeatNew (AjPFeattable thys,
			 const AjPStr source, const AjPStr type,
			 ajint Start, ajint End, float score,
			 char strand, ajint frame);
AjPFeature    ajFeatNewII (AjPFeattable thys,
			   ajint Start, ajint End);
AjPFeature    ajFeatNewIIRev (AjPFeattable thys,
			      ajint Start, ajint End);
AjPFeature    ajFeatNewProt (AjPFeattable thys,
			     const AjPStr source, const AjPStr type,
			     ajint Start, ajint End, float score);
AjBool        ajFeatOutFormatDefault (AjPStr* pformat);
void          ajFeatPrintFormat(AjPFile outf, AjBool full);
void          ajFeatPrintbookFormat(AjPFile outf);
void          ajFeatoutPrintbookFormat(AjPFile outf);
void          ajFeatPrintwikiFormat(AjPFile outf);
void          ajFeatoutPrintwikiFormat(AjPFile outf);
void          ajFeatReverse  (AjPFeature thys, ajint ilen) ;
void          ajFeatSetDesc (AjPFeature thys, const AjPStr desc);
void          ajFeatSetDescApp (AjPFeature thys, const AjPStr desc);
void          ajFeatSetScore (AjPFeature thys, float score);
void          ajFeatSetStrand (AjPFeature thys, AjBool rev);
void          ajFeatSortByEnd (AjPFeattable Feattab);
void          ajFeatSortByStart (AjPFeattable Feattab);
void          ajFeatSortByType (AjPFeattable Feattab);
void          ajFeattableAdd (AjPFeattable thys, AjPFeature feature) ;
void          ajFeattableClear (AjPFeattable thys);
void          ajFeattableDel (AjPFeattable *pthis) ;
ajint         ajFeattableGetBegin (const AjPFeattable thys);
ajint         ajFeattableGetEnd (const AjPFeattable thys);
ajint         ajFeattableGetLen (const AjPFeattable thys);
const AjPStr  ajFeattableGetName (const AjPFeattable thys);
ajuint        ajFeattableGetSize(const AjPFeattable thys);
const char*   ajFeattableGetTypeC (const AjPFeattable thys);
const AjPStr  ajFeattableGetTypeS (const AjPFeattable thys);
AjBool        ajFeattableGetXrefs(const AjPFeattable thys, AjPList *Pxreflist,
                                  ajuint *Ptaxid);
AjBool        ajFeattableIsNuc (const AjPFeattable thys);
AjBool        ajFeattableIsProt (const AjPFeattable thys);
AjPFeattable  ajFeattableNew (const AjPStr name);
AjPFeattable  ajFeattableNewDna (const AjPStr name);
AjPFeattable  ajFeattableNewFtable (const AjPFeattable orig);
AjPFeattable  ajFeattableNewFtableLimit (const AjPFeattable orig, ajint limit);
AjPFeattable  ajFeattableNewProt (const AjPStr name);
AjPFeattable  ajFeattableNewRead  (AjPFeattabIn ftin);
AjPFeattable  ajFeattableNewReadUfo (AjPFeattabIn tabin, const AjPStr Ufo);
AjPFeattable  ajFeattableNewSeq (const AjPSeq seq);
ajuint        ajFeattablePos (const AjPFeattable thys, ajint ipos);
ajuint        ajFeattablePosI (const AjPFeattable thys,
			       ajuint imin, ajint ipos);
ajuint        ajFeattablePosII (ajuint ilen, ajuint imin, ajint ipos);
AjBool        ajFeattableWriteDebug(AjPFeattabOut ftout,
                               const AjPFeattable ftable);
AjBool        ajFeattablePrint(const AjPFeattable ftable,
                               AjPFile file);
void          ajFeattableReverse  (AjPFeattable  thys) ;
void          ajFeattableSetDefname(AjPFeattable thys, const AjPStr setname);
void          ajFeattableSetLength(AjPFeattable thys, ajuint len);
void          ajFeattableSetNuc (AjPFeattable thys);
void          ajFeattableSetProt (AjPFeattable thys);
void          ajFeattableSetRange  (AjPFeattable thys,
				     ajint fbegin, ajint fend) ;
void          ajFeattableTrace (const AjPFeattable thys);
AjBool        ajFeattableTrimOff (AjPFeattable thys,
				  ajuint ioffset, ajuint ilen);
AjBool        ajFeattableWriteDasgff(AjPFeattabOut ftout,
                                    const AjPFeattable features);
AjBool        ajFeattableWriteDdbj (AjPFeattabOut ftout,
                                    const AjPFeattable features);
AjBool        ajFeattableWriteEmbl (AjPFeattabOut ftout,
                                    const AjPFeattable features);
AjBool        ajFeattableWriteGenbank (AjPFeattabOut ftout,
                                    const AjPFeattable features);
AjBool        ajFeattableWriteRefseq (AjPFeattabOut ftout,
                                      const AjPFeattable features);
AjBool        ajFeattableWriteRefseqp (AjPFeattabOut ftout,
                                       const AjPFeattable features);
AjBool        ajFeattableWriteGff2 (AjPFeattabOut ftout,
                                    const AjPFeattable features);
AjBool        ajFeattableWriteGff3 (AjPFeattabOut ftout,
                                    const AjPFeattable features);
AjBool        ajFeattableWritePir (AjPFeattabOut ftout,
                                    const AjPFeattable features);
AjBool        ajFeattableWriteSwiss (AjPFeattabOut ftout,
                                    const AjPFeattable features);
void          ajFeattabInClear (AjPFeattabIn thys);
void          ajFeattabInDel (AjPFeattabIn* pthis);
AjPFeattabIn  ajFeattabInNew (void);
AjPFeattabIn  ajFeattabInNewCSF (const char* fmt, const AjPStr name,
				 const char* type, AjPFilebuff buff);
AjPFeattabIn  ajFeattabInNewSS (const AjPStr fmt, const AjPStr name,
				const char* type);
AjPFeattabIn  ajFeattabInNewSSF (const AjPStr fmt, const AjPStr name,
				 const char* type, AjPFilebuff buff);
AjBool        ajFeattabInSetType(AjPFeattabIn thys, const AjPStr type);
AjBool        ajFeattabInSetTypeC(AjPFeattabIn thys, const char* type);
void          ajFeattabOutClear(AjPFeattabOut *thys);
void          ajFeattabOutDel (AjPFeattabOut* pthis);
AjPFile       ajFeattabOutFile (const AjPFeattabOut thys);
AjPStr        ajFeattabOutFilename (const AjPFeattabOut thys);
AjBool        ajFeattabOutIsLocal(const AjPFeattabOut thys);
AjBool        ajFeattabOutIsOpen (const AjPFeattabOut thys);
AjPFeattabOut ajFeattabOutNew (void);
AjPFeattabOut ajFeattabOutNewCSF (const char* fmt, const AjPStr name,
				  const char* type, AjPFile buff);
AjPFeattabOut ajFeattabOutNewSSF (const AjPStr fmt, const AjPStr name,
				  const char* type, AjPFile buff);
AjBool        ajFeattabOutOpen (AjPFeattabOut thys, const AjPStr ufo);
AjBool        ajFeattabOutSet (AjPFeattabOut thys, const AjPStr ufo);
void          ajFeattabOutSetBasename (AjPFeattabOut thys,
				       const AjPStr basename);
AjBool        ajFeattabOutSetSeqname(AjPFeattabOut thys, const AjPStr name);
AjBool        ajFeattabOutSetType(AjPFeattabOut thys, const AjPStr type);
AjBool        ajFeattabOutSetTypeC(AjPFeattabOut thys, const char* type);
AjBool        ajFeatTagAdd (AjPFeature thys,
			    const AjPStr tag, const AjPStr value);
AjBool        ajFeatTagAddC (AjPFeature thys,
			     const char* tag, const AjPStr value);
AjBool        ajFeatTagAddCC (AjPFeature thys,
			      const char* tag, const char* value);
AjIList       ajFeatTagIter (const AjPFeature thys);
AjBool        ajFeatTagSet (AjPFeature thys,
			    const AjPStr tag, const AjPStr value);
AjBool        ajFeatTagSetC (AjPFeature thys,
			     const char* tag, const AjPStr value);
void          ajFeatTagTrace (const AjPFeature thys);
AjBool        ajFeatTagval (AjIList iter, AjPStr* tagnam,
			    AjPStr* tagval);
void          ajFeatTest (void);
void          ajFeatTrace (const AjPFeature thys);
AjBool        ajFeatTrimOffRange (AjPFeature ft, ajuint ioffset,
				  ajuint begin, ajuint end,
				  AjBool dobegin, AjBool doend);
const AjPStr  ajFeatTypeGetCategory(const AjPStr type);
AjBool        ajFeatTypeIsCds(const AjPFeature gf);
AjBool        ajFeatTypeMatchC(const AjPFeature gf, const char* txt);
AjBool        ajFeatTypeMatchS(const AjPFeature gf, const AjPStr str);
AjBool        ajFeatTypeMatchWildS(const AjPFeature gf, const AjPStr str);
const AjPStr  ajFeatTypeNuc(const AjPStr type);
const AjPStr  ajFeatTypeProt(const AjPStr type);
AjBool        ajFeattableWriteUfo (AjPFeattabOut ftout, const AjPFeattable ft,
			      const AjPStr Ufo);
AjBool        ajFeattableWrite (AjPFeattabOut ftout, const AjPFeattable ft) ;
AjBool        ajFeattagIsNote(const AjPStr tag);

void          ajFeatUnused(void);

/*
** End of prototype definitions
*/

__deprecated AjBool        ajFeatGetNoteI (const AjPFeature thys,
                                            const AjPStr name,
                                            ajint count, AjPStr* val);
__deprecated AjPFeature    ajFeatCopy (const AjPFeature orig);
__deprecated AjPFeattable  ajFeattableCopy (const AjPFeattable orig);
__deprecated AjPFeattable  ajFeattableCopyLimit (const AjPFeattable orig,
                                                 ajint limit);
__deprecated AjPFeattable  ajFeatUfoRead (AjPFeattabIn tabin, const AjPStr Ufo);
__deprecated AjBool        ajFeatUfoWrite (const AjPFeattable thys,
                                           AjPFeattabOut tabout,
                                           const AjPStr Ufo);
__deprecated AjBool        ajFeatWrite (AjPFeattabOut ftout,
                                        const AjPFeattable ft) ;
__deprecated AjPFeattable  ajFeatRead  (AjPFeattabIn ftin) ;
__deprecated ajint         ajFeattableBegin (const AjPFeattable thys);
__deprecated ajint         ajFeattableEnd (const AjPFeattable thys);
__deprecated ajint         ajFeattableLen (const AjPFeattable thys);
__deprecated ajint         ajFeattableSize (const AjPFeattable thys);
__deprecated void          ajFeatDefName(AjPFeattable thys,
                                         const AjPStr setname);
__deprecated AjBool        ajFeatGetNote (const AjPFeature thys,
                                          const AjPStr name, AjPStr* val);


/*
//#define       MAJFEATOBJVERIFY(p,c) ajFeatObjAssert((p), (c), \
//      __FILE__, __LINE__)
//#define       MAJFEATSETSCORE(p,s) (((AjPFeature)(p))->Score=(s))
//#define       MAJFEATSCORE(p)    ((p)->Score)
//#define       MAJFEATSOURCE(p)   ((p)->Source)
//#define       MAJFEATTYPE(p)     ((p)->Type)
//#define       MAJFEATTABSETVERSION(p,v) ((p)->Version=(v))
//#define       MAJFEATTABSETDATE(p,d)    ((p)->Date=(d))
//#define       MAJFEATTABDEFFORMAT(p,f)  ((p)->DefFormat=(f))
//#define       MAJFEATTABFORMAT(p)       ((p)->Format)
//#define       MAJFEATTABVERSION(p)      ((p)->Version)
//#define       MAJFEATTABDATE(p)         ((p)->Date)
//#define       MAJFEATTABDICTIONARY(p)   ((p)->Dictionary)
//#define       MAJFEATVOCABREADONLY(p,f) ((p)?(p)->ReadOnly=(f):AjTrue)
*/

#endif /* ajfeat_h */

#ifdef __cplusplus
}
#endif

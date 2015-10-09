/* @include ajfeat ************************************************************
**
** A genome feature (in AJAX program context) is a description of a
** genomic entity which was determined by some 'source' analysis
** (which may be of 'wet lab' experimental or 'in silico'
** computational nature), has a 'primary' descriptor ('Primary_Tag'),
** may have some 'score' asserting the level of analysis confidence in
** its identity (e.g. log likelihood relative to a null hypothesis or
** other similar entity), has a 'Position' in the genome, and may have
** any arbitrary number of descriptor tags associated with it.
**
** @author Copyright (C) 1999 Richard Bruskiewich
** @version $Revision: 1.78 $
** @modified 2000 Ian Longden.
** @modified 2001 Peter Rice.
** @modified $Date: 2012/07/03 16:19:36 $ by $Author: rice $
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
******************************************************************************/

#ifndef AJFEAT_H
#define AJFEAT_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajexcept.h"
#include "ajmem.h"
#include "ajreg.h"
#include "ajstr.h"
#include "ajtagval.h"
#include "ajfile.h"
#include "ajtime.h"
#include "ajfmt.h"
#include "ajfeatdata.h"
#include "ajseqdata.h"
#include <stdlib.h>
#include <stdio.h>

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */




/*
** Prototype definitions
*/

ajint          ajFeatCompByEnd(const void *a, const void *b);
ajint          ajFeatCompByGroup(const void *a, const void *b);
ajint          ajFeatCompByStart(const void *a, const void *b);
ajint          ajFeatCompByType(const void *a, const void *b);

const AjPTable ajFeatVocabGetTags(const char* name);
const AjPTable ajFeatVocabGetTagsNuc(const char* name);
const AjPTable ajFeatVocabGetTagsProt(const char* name);
const AjPTable ajFeatVocabGetTypes(const char* name);
const AjPTable ajFeatVocabGetTypesNuc(const char* name);
const AjPTable ajFeatVocabGetTypesProt(const char* name);
const AjPTable ajFeatVocabInit(const char* name);
const AjPTable ajFeatVocabInitNuc(const char* name);
const AjPTable ajFeatVocabInitProt(const char* name);
AjPFeature    ajFeatNewFeat(const AjPFeature orig);
void          ajFeatDel(AjPFeature *pthis);
void          ajFeatExit(void);
ajuint        ajFeatGetEnd(const AjPFeature thys);
char          ajFeatframeGetFrame(ajint frame);
char          ajFeatframeGetFrameNuc(ajint frame);
char          ajFeatstrandGetStrand(ajint strand);
AjBool        ajFeatGetForward(const AjPFeature thys);
ajint         ajFeatGetFrame(const AjPFeature thys);
ajuint        ajFeatGetLength(const AjPFeature thys);
AjBool        ajFeatGetNoteC(const AjPFeature thys, const char* name,
                             AjPStr* val);
AjBool        ajFeatGetNoteCI(const AjPFeature thys, const char* name,
                              ajint count, AjPStr* val);
AjBool        ajFeatGetNoteSI(const AjPFeature thys, const AjPStr name,
                              ajint count, AjPStr* val);
AjBool        ajFeatGetNoteS(const AjPFeature thys, const AjPStr name,
                             AjPStr* val);
AjBool        ajFeatGetRemoteseq(const AjPFeature thys, const AjPStr usa,
                                 AjPSeq seq);
float         ajFeatGetScore(const AjPFeature thys);
AjBool        ajFeatGetScorerange(const AjPFeature thys,
                                  float *minscore, float *maxscore);
const AjPStr  ajFeatGetSubtype(const AjPFeature thys);
const AjPStr  ajFeatGetSource(const AjPFeature thys);
AjBool        ajFeatGetFlags(const AjPFeature thys,  AjPStr* Pflagstr);
AjBool        ajFeatGetSeq(const AjPFeature feat,
                           const AjPSeq seq, AjPStr* seqstr);
AjBool        ajFeatGetSeqJoin(const AjPFeature thys,
                               const AjPSeq seq, AjPStr* Pseqstr);
ajuint        ajFeatGetStart(const AjPFeature thys);
char          ajFeatGetStrand(const AjPFeature thys);
AjBool        ajFeatGetTagC(const AjPFeature thys, const char* tname,
                            ajint num, AjPStr* val);
AjBool        ajFeatGetTagS(const AjPFeature thys, const AjPStr name,
                            ajint num, AjPStr* val);
AjBool        ajFeatGetTranslation(const AjPFeature feat, AjPStr* seqstr);
const AjPStr  ajFeatGetType(const AjPFeature thys);
AjBool        ajFeatGetXrefs(const AjPFeature thys, AjPList *Pxreflist);
AjBool        ajFeatIsCompMult(const AjPFeature gf);
AjBool        ajFeatIsLocal(const AjPFeature gf);
AjBool        ajFeatIsLocalRange(const AjPFeature gf,
                                 ajuint start, ajuint end);
AjBool        ajFeatIsMultiple(const AjPFeature gf);
AjBool        ajFeatLocMark(const AjPFeature thys, const AjPFeattable table,
                            AjPStr* Pseqstr);
AjPFeature    ajFeatNew(AjPFeattable thys,
                        const AjPStr source, const AjPStr type,
                        ajint Start, ajint End, float score,
                        char strand, ajint frame);
AjPFeature    ajFeatNewII(AjPFeattable thys,
                          ajint Start, ajint End);
AjPFeature    ajFeatNewIIRev(AjPFeattable thys,
                             ajint Start, ajint End);
AjPFeature    ajFeatNewBetween(AjPFeattable thys,
                               ajint Start);
AjPFeature   ajFeatNewNuc(AjPFeattable thys,
                          const AjPStr source, const AjPStr type,
                          ajint Start, ajint End,
                          float score, char  strand,
                          ajint frame, ajint exon,
                          ajint Start2, ajint End2,
                          const AjPStr entryid,  const AjPStr label);
AjPFeature   ajFeatNewNucFlags(AjPFeattable thys,
                               const AjPStr source, const AjPStr type,
                               ajint Start, ajint End,
                               float score, char  strand,
                               ajint frame, ajint exon,
                               ajint Start2, ajint End2,
                               const AjPStr entryid,  const AjPStr label,
                               ajuint flags);
AjPFeature    ajFeatNewProt(AjPFeattable thys,
                            const AjPStr source, const AjPStr type,
                            ajint Start, ajint End,
                            float score);
AjPFeature    ajFeatNewProtFlags(AjPFeattable thys,
                                 const AjPStr source, const AjPStr type,
                                 ajint Start, ajint End,
                                 float score,
                                 ajuint flags);
AjPFeature    ajFeatNewSub(AjPFeattable thys, AjPFeature parent,
                           const AjPStr source, const AjPStr type,
                           ajint Start, ajint End, float score,
                           char strand, ajint frame);
AjPFeature    ajFeatNewIISub(AjPFeattable thys, AjPFeature parent,
                             ajint Start, ajint End);
AjPFeature    ajFeatNewIIRevSub(AjPFeattable thys, AjPFeature parent,
                                ajint Start, ajint End);
AjPFeature   ajFeatNewNucSub(AjPFeattable thys, AjPFeature parent,
                             const AjPStr source, const AjPStr type,
                             ajint Start, ajint End,
                             float score, char  strand,
                             ajint frame, ajint exon,
                             ajint Start2, ajint End2,
                             const AjPStr entryid,  const AjPStr label);
AjPFeature   ajFeatNewNucFlagsSub(AjPFeattable thys, AjPFeature parent,
                                  const AjPStr source, const AjPStr type,
                                  ajint Start, ajint End,
                                  float score, char  strand,
                                  ajint frame, ajint exon,
                                  ajint Start2, ajint End2,
                                  const AjPStr entryid,  const AjPStr label,
                                  ajuint flags);
AjPFeature    ajFeatNewProtSub(AjPFeattable thys, AjPFeature parent,
                               const AjPStr source, const AjPStr type,
                               ajint Start, ajint End,
                               float score);
AjPFeature    ajFeatNewProtFlagsSub(AjPFeattable thys, AjPFeature parent,
                                    const AjPStr source, const AjPStr type,
                                    ajint Start, ajint End,
                                    float score,
                                    ajuint flags);
void          ajFeatReverse(AjPFeature thys, ajint ilen);
void          ajFeatSetDesc(AjPFeature thys, const AjPStr desc);
void          ajFeatSetDescApp(AjPFeature thys, const AjPStr desc);
void          ajFeatSetScore(AjPFeature thys, float score);
void          ajFeatSetSource(AjPFeature thys, const AjPStr source);
void          ajFeatSetStrand(AjPFeature thys, AjBool rev);
void          ajFeatSortByEnd(AjPFeattable Feattab);
void          ajFeatSortByStart(AjPFeattable Feattab);
void          ajFeatSortByType(AjPFeattable Feattab);
void          ajFeattableAdd(AjPFeattable thys, AjPFeature feature);
void          ajFeattableClear(AjPFeattable thys);
void          ajFeattableDel(AjPFeattable *pthis);
ajint         ajFeattableGetBegin(const AjPFeattable thys);
ajint         ajFeattableGetEnd(const AjPFeattable thys);
const AjPStr  ajFeattableGetEntry(const AjPFeattable thys);
ajint         ajFeattableGetLen(const AjPFeattable thys);
const AjPStr  ajFeattableGetName(const AjPFeattable thys);
const char*   ajFeattableGetQryC(const AjPFeattable ftable);
const AjPStr  ajFeattableGetQryS(const AjPFeattable ftable);
AjBool        ajFeattableGetScorerange(const AjPFeattable thys,
                                       float *minscore, float *maxscore);
ajuint        ajFeattableGetSize(const AjPFeattable thys);
const char*   ajFeattableGetTypeC(const AjPFeattable thys);
const AjPStr  ajFeattableGetTypeS(const AjPFeattable thys);
AjBool        ajFeattableGetXrefs(const AjPFeattable thys, AjPList *Pxreflist,
                                  ajuint *Ptaxid);
AjBool        ajFeattableIsCircular(const AjPFeattable thys);
AjBool        ajFeattableIsNuc(const AjPFeattable thys);
AjBool        ajFeattableIsProt(const AjPFeattable thys);
ajuint        ajFeattableMerge(AjPFeattable thys, const AjPFeattable srctable);
AjPFeattable  ajFeattableNew(const AjPStr name);
AjPFeattable  ajFeattableNewDna(const AjPStr name);
AjPFeattable  ajFeattableNewFtable(const AjPFeattable orig);
AjPFeattable  ajFeattableNewFtableLimit(const AjPFeattable orig, ajint limit);
AjPFeattable  ajFeattableNewProt(const AjPStr name);
AjPFeattable  ajFeattableNewSeq(const AjPSeq seq);
ajuint        ajFeattablePos(const AjPFeattable thys, ajint ipos);
ajuint        ajFeattablePosI(const AjPFeattable thys,
                              ajuint imin, ajint ipos);
ajuint        ajFeattablePosII(ajuint ilen, ajuint imin, ajint ipos);
void          ajFeattableReverse(AjPFeattable  thys);
void          ajFeattableSetCircular(AjPFeattable thys);
void          ajFeattableSetDefname(AjPFeattable thys, const AjPStr setname);
void          ajFeattableSetLength(AjPFeattable thys, ajuint len);
void          ajFeattableSetLinear(AjPFeattable thys);
void          ajFeattableSetNuc(AjPFeattable thys);
void          ajFeattableSetProt(AjPFeattable thys);
void          ajFeattableSetRange(AjPFeattable thys,
                                  ajint fbegin, ajint fend);
void          ajFeattableTrace(const AjPFeattable thys);
AjBool        ajFeattableTrim(AjPFeattable thys);
AjBool        ajFeattableTrimOff(AjPFeattable thys,
                                 ajuint ioffset, ajuint ilen);
AjPFeatGfftags ajFeatGfftagsNew(void);
void           ajFeatGfftagsDel(AjPFeatGfftags*Pthys);
AjPFeatGfftags ajFeatGetGfftags(const AjPFeature thys);
void           ajFeatGfftagTrace(const AjPFeature thys);
const AjPStr   ajFeatGetId(const AjPFeature thys);
const AjPStr   ajFeatGetParent(const AjPFeature thys);

ajuint        ajFeatGfftagAddSS(AjPFeature thys,
                                const AjPStr tag, const AjPStr value);
ajuint        ajFeatGfftagAddCS(AjPFeature thys,
                                const char* tag, const AjPStr value);
AjBool        ajFeatGfftagAddTag(AjPFeature thys, const AjPTagval tagval);

AjBool        ajFeatTagAddSS(AjPFeature thys,
                             const AjPStr tag, const AjPStr value);
AjBool        ajFeatTagAddCS(AjPFeature thys,
                             const char* tag, const AjPStr value);
AjBool        ajFeatTagAddCC(AjPFeature thys,
                             const char* tag, const char* value);
AjBool        ajFeatTagAddTag(AjPFeature thys, const AjPTagval tagval);
AjIList       ajFeatSubIter(const AjPFeature thys);
AjIList       ajFeatTagIter(const AjPFeature thys);
AjBool        ajFeatTagSet(AjPFeature thys,
                           const AjPStr tag, const AjPStr value);
AjBool        ajFeatTagSetC(AjPFeature thys,
                            const char* tag, const AjPStr value);
void          ajFeatTagTrace(const AjPFeature thys);
AjBool        ajFeatTagval(AjIList iter, AjPStr* tagnam,
                           AjPStr* tagval);
void          ajFeatTest(void);
void          ajFeatTrace(const AjPFeature thys);
AjBool        ajFeatTrimOffRange(AjPFeature ft, ajuint ioffset,
                                 ajuint begin, ajuint end,
                                 AjBool dobegin, AjBool doend);
const AjPStr  ajFeatTypeGetCategory(const AjPStr type);
AjBool        ajFeatTypeIsCds(const AjPFeature gf);
AjBool        ajFeatTypeMatchC(const AjPFeature gf, const char* txt);
AjBool        ajFeatTypeMatchS(const AjPFeature gf, const AjPStr str);
AjBool        ajFeatTypeMatchWildS(const AjPFeature gf, const AjPStr str);
const AjPStr  ajFeatTypeNuc(const AjPStr type);
const AjPStr  ajFeatTypeProt(const AjPStr type);
AjBool        ajFeattagIsNote(const AjPStr tag);

void          ajFeatUnused(void);
void          ajFeatWarn(const char* fmt, ...);
const AjPStr  ajFeattypeGetExternal(const AjPStr type, const AjPTable table);
const AjPStr  ajFeattypeGetInternal(const AjPStr type);
const AjPStr  ajFeattypeGetInternalEmbl(const AjPStr type);
const AjPStr  ajFeattypeGetInternalNuc(const AjPStr type);
const AjPStr  ajFeattypeGetInternalPir(const AjPStr type);
const AjPStr  ajFeattypeGetInternalProt(const AjPStr type);
const AjPStr  ajFeattypeGetInternalRefseqp(const AjPStr type);
AjBool        ajFeattagSpecial(const AjPStr tag, AjPStr* pval);
AjBool        ajFeattagSpecialGff2(const AjPStr tag, AjPStr* pval);
AjBool        ajFeattagSpecialGff3(const AjPStr tag, AjPStr* pval);
void          ajFeattagFormat(const AjPStr name, const AjPTable table,
                              AjPStr* retstr);
const AjPStr  ajFeattagGetNameC(const char *tag, const AjPTable table,
                                AjBool* known);
const AjPStr  ajFeattagGetNameS(const AjPStr name, const AjPTable table,
                                AjBool* known);
void          ajFeattagGetLimit(const AjPStr name, const AjPTable table,
                                AjPStr* retstr);

/*
** End of prototype definitions
*/

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


#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif /* AJ_COMPILE_DEPRECATED_BOOK */

#ifdef AJ_COMPILE_DEPRECATED

__deprecated AjBool        ajFeatGetNoteI(const AjPFeature thys,
                                          const AjPStr name,
                                          ajint count, AjPStr* val);
__deprecated AjPFeature    ajFeatCopy(const AjPFeature orig);
__deprecated AjPFeattable  ajFeattableCopy(const AjPFeattable orig);
__deprecated AjPFeattable  ajFeattableCopyLimit(const AjPFeattable orig,
                                                ajint limit);
__deprecated AjPFeattable  ajFeatUfoRead(AjPFeattabIn tabin, const AjPStr Ufo);
__deprecated AjPFeattable  ajFeatRead(AjPFeattabIn ftin);
__deprecated ajint         ajFeattableBegin(const AjPFeattable thys);
__deprecated ajint         ajFeattableEnd(const AjPFeattable thys);
__deprecated ajint         ajFeattableLen(const AjPFeattable thys);
__deprecated ajint         ajFeattableSize(const AjPFeattable thys);
__deprecated void          ajFeatDefName(AjPFeattable thys,
                                         const AjPStr setname);
__deprecated AjBool        ajFeatGetNote(const AjPFeature thys,
                                         const AjPStr name, AjPStr* val);

__deprecated ajuint        ajFeatGfftagAdd(AjPFeature thys,
                                           const AjPStr tag,
                                           const AjPStr value);
__deprecated ajuint        ajFeatGfftagAddC(AjPFeature thys,
                                            const char* tag,
                                            const AjPStr value);

__deprecated AjBool        ajFeatTagAdd(AjPFeature thys,
                                        const AjPStr tag, const AjPStr value);
__deprecated AjBool        ajFeatTagAddC(AjPFeature thys,
                                         const char* tag, const AjPStr value);
__deprecated AjBool        ajFeatIsChild(const AjPFeature gf);

#endif /* AJ_COMPILE_DEPRECATED */




AJ_END_DECLS

#endif /* AJFEAT_H */

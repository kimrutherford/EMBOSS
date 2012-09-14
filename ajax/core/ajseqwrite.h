/* @include ajseqwrite ********************************************************
**
** AJAX seqwrite  functions
**
** @author Copyright (C) 2001 Peter Rice
** @version $Revision: 1.52 $
** @modified 2001-2011 Peter Rice
** @modified $Date: 2011/10/18 14:23:40 $ by $Author: rice $
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

#ifndef AJSEQWRITE_H
#define AJSEQWRITE_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajstr.h"
#include "ajfile.h"
#include "ajseqdata.h"

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

AjBool       ajSeqoutWriteSeq(AjPSeqout outseq, const AjPSeq seq);
AjBool       ajSeqoutOpenFilename(AjPSeqout seqout, const AjPStr name);
ajint        ajSeqoutGetCheckgcg(const AjPSeqout outseq);
const AjPStr ajSeqoutGetFilename(const AjPSeqout thys);
void         ajSeqoutClear(AjPSeqout thys);
void         ajSeqoutGetBasecount(const AjPSeqout seqout, ajuint* bases);
AjBool       ajSeqoutSetNameDefaultC(AjPSeqout thys,
                                     AjBool multi, const char* txt);
AjBool       ajSeqoutSetNameDefaultS(AjPSeqout thys,
                                     AjBool multi, const AjPStr str);
void         ajSeqoutDel(AjPSeqout* thys);
AjBool       ajSeqoutstrGetFormatDefault(AjPStr* Pformat);
AjBool       ajSeqoutstrIsFormatExists(const AjPStr format);
AjBool       ajSeqoutstrIsFormatSingle(const AjPStr format);
AjPSeqout    ajSeqoutNew(void);
AjPSeqout    ajSeqoutNewFile(AjPFile file);
AjPSeqout    ajSeqoutNewFormatC(const char* txt);
AjPSeqout    ajSeqoutNewFormatS(const AjPStr str);
AjBool       ajSeqoutOpen(AjPSeqout thys);
AjBool       ajSeqoutSetFormatC(AjPSeqout thys, const char* format);
AjBool       ajSeqoutSetFormatS(AjPSeqout thys, const AjPStr format);
void         ajSeqoutTrace(const AjPSeqout seq);
void         ajSeqoutPrintFormat(AjPFile outf, AjBool full);
void         ajSeqoutPrintbookFormat(AjPFile outf);
void         ajSeqoutPrinthtmlFormat(AjPFile outf);
void         ajSeqoutPrintwikiFormat(AjPFile outf);
void         ajSeqoutClearUsa(AjPSeqout thys, const AjPStr Usa);
AjBool       ajSeqoutWriteSet(AjPSeqout seqout, const AjPSeqset seq);
void         ajSeqoutClose(AjPSeqout outseq);
void         ajSeqoutCloseEmpty(AjPSeqout outseq);
void         ajSeqoutFlush(AjPSeqout seqout);
void         ajSeqoutExit(void);
void         ajSeqoutReset(AjPSeqout seqout);

void         ajSeqoutDumpSwisslike(AjPSeqout outseq,const AjPStr seq,
                                   const char *prefix);

/*
** End of prototype definitions
*/




#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif /* AJ_COMPILE_DEPRECATED_BOOK */

#ifdef AJ_COMPILE_DEPRECATED

__deprecated void         ajSeqoutCount(const AjPSeqout seqout, ajuint* b);
__deprecated void         ajSeqAllWrite(AjPSeqout outseq, const AjPSeq seq);
__deprecated AjBool       ajSeqFileNewOut(AjPSeqout seqout,
                                          const AjPStr name);
__deprecated AjPSeqout    ajSeqoutNewF(AjPFile file);
__deprecated void         ajSeqWrite(AjPSeqout seqout, const AjPSeq seq);
__deprecated void         ajSeqsetWrite(AjPSeqout outseq, const AjPSeqset seq);
__deprecated void         ajSeqWriteClose(AjPSeqout outseq);
__deprecated AjBool       ajSeqOutFormatSingle(AjPStr format);
__deprecated AjBool       ajSeqOutSetFormat(AjPSeqout thys,
                                            const AjPStr format);
__deprecated AjBool       ajSeqOutSetFormatC(AjPSeqout thys,
                                             const char* format);
__deprecated AjBool       ajSeqOutFormatDefault(AjPStr* pformat);

__deprecated void         ajSeqoutUsa(AjPSeqout* pthis, const AjPStr Usa);

__deprecated void         ajSeqPrintOutFormat(AjPFile outf, AjBool full);
__deprecated AjBool       ajSeqFindOutFormat(const AjPStr format,
                                             ajint* iformat);
__deprecated void         ajSeqWriteExit(void);
__deprecated ajint        ajSeqoutCheckGcg(const AjPSeqout outseq);

__deprecated void         ajSeqoutDefName(AjPSeqout thys,
                                          const AjPStr setname, AjBool multi);
__deprecated void         ajSeqWriteXyz(AjPFile outf,
                                        const AjPStr seq, const char *prefix);
__deprecated void         ajSssWriteXyz(AjPFile outf,
                                        const AjPStr seq, const char *prefix);

#endif /* AJ_COMPILE_DEPRECATED */




AJ_END_DECLS

#endif /* !AJSEQWRITE_H */

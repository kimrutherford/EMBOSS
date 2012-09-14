/* @include ajseqread *********************************************************
**
** AJAX sequence reading functions
**
** These functions control all aspects of AJAX sequence reading
**
** @author Copyright (C) 2001 Peter Rice
** @version $Revision: 1.50 $
** @modified 2001-2011 pmr
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

#ifndef AJSEQREAD_H
#define AJSEQREAD_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajstr.h"
#include "ajlist.h"
#include "ajfile.h"
#include "ajseqdata.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */

extern AjPTable seqDbMethods;




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */




/*
** Prototype definitions
*/

AjPTable     ajSeqaccessGetDb(void);
AjBool       ajSeqaccessMethodTest(const AjPStr method);
const char*  ajSeqaccessMethodGetQlinks(const AjPStr method);
ajuint       ajSeqaccessMethodGetScope(const AjPStr method);
AjPSeqall    ajSeqallFile(const AjPStr usa);
AjBool       ajSeqAllRead(AjPSeq thys, AjPSeqin seqin);
AjBool       ajSeqGetFromUsa(const AjPStr thys, AjBool protein, AjPSeq seq);
AjBool       ajSeqGetFromUsaRange(const AjPStr thys, AjBool protein,
                                  ajint ibegin, ajint iend, AjPSeq seq);
AjBool       ajSeqFormatTest(const AjPStr format);
void         ajSeqinClear(AjPSeqin thys);
void         ajSeqinClearPos(AjPSeqin thys);
void         ajSeqinDel(AjPSeqin* pthis);
AjPSeqin     ajSeqinNew(void);
AjPSeqin     ajSeqinNewQueryC(const char* qrytxt);
AjPSeqin     ajSeqinNewQueryS(const AjPStr qry);
void         ajSeqinSetNuc(AjPSeqin seqin);
void         ajSeqinSetProt(AjPSeqin seqin);
void         ajSeqinSetRange(AjPSeqin seqin, ajint ibegin, ajint iend);
void         ajSeqinUsa(AjPSeqin* pthis, const AjPStr Usa);
void         ajSeqinTrace(const AjPSeqin thys);
const char*  ajSeqinTypeGetFields(void);
const char*  ajSeqinTypeGetQlinks(void);

AjBool       ajSeqParseFasta(const AjPStr str, AjPStr* id, AjPStr* acc,
                             AjPStr* sv, AjPStr* desc);
AjBool       ajSeqParseFastq(const AjPStr str, AjPStr* id, AjPStr* desc);
AjBool       ajSeqParseNcbi(const AjPStr str, AjPStr* id, AjPStr* acc,
                            AjPStr* sv, AjPStr* gi, AjPStr* db, AjPStr* desc);
void         ajSeqPrintInFormat(AjPFile outf, AjBool full);
void         ajSeqPrintbookInFormat(AjPFile outf);
void         ajSeqPrinthtmlInFormat(AjPFile outf);
void         ajSeqPrintwikiInFormat(AjPFile outf);
AjBool       ajSeqRead(AjPSeq thys, AjPSeqin seqin);
void         ajSeqReadExit(void);
ajint        ajSeqsetApp(AjPSeqset thys, const AjPSeq seq);
AjBool       ajSeqsetGetFromUsa(const AjPStr thys, AjPSeqset *seq);
AjBool       ajSeqsetFromList(AjPSeqset thys, const AjPList list);
ajint        ajSeqsetFromPair(AjPSeqset thys,
                              const AjPSeq seqa, const AjPSeq seqb);
AjBool       ajSeqsetallRead(AjPList thys, AjPSeqin seqin);
AjBool       ajSeqsetRead(AjPSeqset thys, AjPSeqin seqin);
AjBool       ajSeqUsaGetBase(const AjPStr usa, AjPStr* baseusa);
AjBool       ajSeqinformatTerm(const AjPStr term);
AjBool       ajSeqinformatTest(const AjPStr format);

/*
** End of prototype definitions
*/




#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif /* AJ_COMPILE_DEPRECATED_BOOK */

#ifdef AJ_COMPILE_DEPRECATED

__deprecated AjBool       ajSeqMethodTest(const AjPStr method);
__deprecated ajuint       ajSeqMethodGetScope(const AjPStr method);

#endif /* AJ_COMPILE_DEPRECATED */




AJ_END_DECLS

#endif /* !AJSEQREAD_H */

/* @include ajfeatwrite *******************************************************
**
** AJAX feature writing functions
**
** These functions control all aspects of AJAX feature writing
**
** @author Copyright (C) 1999 Richard Bruskiewich
** @modified 2000 Ian Longden.
** @modified 2001-2011 Peter Rice.
** @version $Revision: 1.6 $
** @modified $Date: 2011/12/19 16:39:37 $ by $Author: rice $
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

#ifndef AJFEATWRITE_H
#define AJFEATWRITE_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajstr.h"
#include "ajfile.h"
#include "ajfeatdata.h"

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

AjBool        ajFeatOutFormatDefault(AjPStr* pformat);
void          ajFeatoutPrintFormat(AjPFile outf, AjBool full);
void          ajFeatoutPrinthtmlFormat(AjPFile outf);
void          ajFeatoutPrintbookFormat(AjPFile outf);
void          ajFeatoutPrintwikiFormat(AjPFile outf);
void          ajFeatwriteExit(void);
AjBool        ajFeattableWriteDebug(AjPFeattabOut ftout,
                                    const AjPFeattable ftable);
AjBool        ajFeattablePrint(const AjPFeattable ftable,
                               AjPFile file);
AjBool        ajFeattableWriteBed(AjPFeattabOut ftout,
                                  const AjPFeattable features);
AjBool        ajFeattableWriteDasgff(AjPFeattabOut ftout,
                                     const AjPFeattable features);
AjBool        ajFeattableWriteDdbj(AjPFeattabOut ftout,
                                   const AjPFeattable features);
AjBool        ajFeattableWriteDraw(AjPFeattabOut ftout,
                                   const AjPFeattable thys);
AjBool        ajFeattableWriteEmbl(AjPFeattabOut ftout,
                                   const AjPFeattable features);
AjBool        ajFeattableWriteGenbank(AjPFeattabOut ftout,
                                      const AjPFeattable features);
AjBool        ajFeattableWriteRefseq(AjPFeattabOut ftout,
                                     const AjPFeattable features);
AjBool        ajFeattableWriteRefseqp(AjPFeattabOut ftout,
                                      const AjPFeattable features);
AjBool        ajFeattableWriteGff2(AjPFeattabOut ftout,
                                   const AjPFeattable features);
AjBool        ajFeattableWriteGff3(AjPFeattabOut ftout,
                                   const AjPFeattable features);
AjBool        ajFeattableWritePir(AjPFeattabOut ftout,
                                  const AjPFeattable features);
AjBool        ajFeattableWriteSwiss(AjPFeattabOut ftout,
                                    const AjPFeattable features);
void          ajFeattabOutClear(AjPFeattabOut *thys);
void          ajFeattabOutDel(AjPFeattabOut* pthis);
AjPFile       ajFeattabOutFile(const AjPFeattabOut thys);
AjPStr        ajFeattabOutFilename(const AjPFeattabOut thys);
AjBool        ajFeattabOutIsLocal(const AjPFeattabOut thys);
AjBool        ajFeattabOutIsOpen(const AjPFeattabOut thys);
AjPFeattabOut ajFeattabOutNew(void);
AjPFeattabOut ajFeattabOutNewCSF(const char* fmt, const AjPStr name,
                                 const char* type, AjPFile buff);
AjPFeattabOut ajFeattabOutNewSSF(const AjPStr fmt, const AjPStr name,
                                 const char* type, AjPFile buff);
AjBool        ajFeattabOutOpen(AjPFeattabOut thys, const AjPStr ufo);
AjBool        ajFeattabOutSet(AjPFeattabOut thys, const AjPStr ufo);
void          ajFeattabOutSetBasename(AjPFeattabOut thys,
                                      const AjPStr basename);
AjBool        ajFeattabOutSetSeqname(AjPFeattabOut thys, const AjPStr name);
AjBool        ajFeattabOutSetType(AjPFeattabOut thys, const AjPStr type);
AjBool        ajFeattabOutSetTypeC(AjPFeattabOut thys, const char* type);
AjBool        ajFeattableWriteUfo(AjPFeattabOut ftout, const AjPFeattable ft,
                                  const AjPStr Ufo);
AjBool        ajFeattableWrite(AjPFeattabOut ftout, const AjPFeattable ft);

/*
** End of prototype definitions
*/




#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif /* AJ_COMPILE_DEPRECATED_BOOK */

#ifdef AJ_COMPILE_DEPRECATED

__deprecated AjBool ajFeatUfoWrite(const AjPFeattable thys,
                                   AjPFeattabOut featout,
                                   const AjPStr ufo);

__deprecated AjBool ajFeatWrite(AjPFeattabOut ftout,
                                const AjPFeattable features);

#endif /* AJ_COMPILE_DEPRECATED */




AJ_END_DECLS

#endif /* AJFEATWRITE_H */

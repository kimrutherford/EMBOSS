/* @include ajfileio **********************************************************
**
** AJAX file routines
**
** @author Copyright (C) 1999 Peter Rice
** @version $Revision: 1.9 $
** @modified Peter Rice pmr@ebi.ac.uk I/O file functions from ajfile.c
** @modified $Date: 2011/12/19 16:35:36 $ by $Author: rice $
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

#ifndef AJFILEIO_H
#define AJFILEIO_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajfile.h"
#include <sys/stat.h>

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

size_t         ajReadbinBinary(AjPFile thys, size_t count,
                               size_t element_size, void* ptr);
size_t         ajReadbinChar(AjPFile file, size_t len, char* buffer);
size_t         ajReadbinCharTrim(AjPFile file, size_t len, char* buffer);
size_t         ajReadbinInt(AjPFile thys, ajint *val);
size_t         ajReadbinIntEndian(AjPFile thys, ajint *val);
size_t         ajReadbinIntLocal(AjPFile thys, ajint *val);
size_t         ajReadbinInt2(AjPFile thys, ajshort *val);
size_t         ajReadbinInt2Endian(AjPFile thys, ajshort *val);
size_t         ajReadbinInt2Local(AjPFile thys, ajshort *val);
size_t         ajReadbinInt4(AjPFile thys, ajint *val);
size_t         ajReadbinInt4Endian(AjPFile thys, ajint *val);
size_t         ajReadbinInt4Local(AjPFile thys, ajint *val);
size_t         ajReadbinInt8(AjPFile thys, ajlong *val);
size_t         ajReadbinInt8Endian(AjPFile thys, ajlong *val);
size_t         ajReadbinInt8Local(AjPFile thys, ajlong *val);
size_t         ajReadbinStr(AjPFile file, size_t len, AjPStr* buffer);
size_t         ajReadbinStrTrim(AjPFile file, size_t len, AjPStr* buffer);
size_t         ajReadbinUint(AjPFile thys, ajuint *val);
size_t         ajReadbinUint2(AjPFile thys, ajushort *val);
size_t         ajReadbinUint4(AjPFile thys, ajuint *val);
size_t         ajReadbinUint8(AjPFile thys, ajulong *val);
size_t         ajReadbinUintEndian(AjPFile thys, ajuint *val);
size_t         ajReadbinUint2Endian(AjPFile thys, ajushort *val);
size_t         ajReadbinUint4Endian(AjPFile thys, ajuint *val);
size_t         ajReadbinUint8Endian(AjPFile thys, ajulong *val);
size_t         ajReadbinUintLocal(AjPFile thys, ajuint *val);
size_t         ajReadbinUint2Local(AjPFile thys, ajushort *val);
size_t         ajReadbinUint4Local(AjPFile thys, ajuint *val);
size_t         ajReadbinUint8Local(AjPFile thys, ajulong *val);

size_t         ajWritebinBinary(AjPFile file, size_t count,
                                size_t size, const void* buffer);
size_t         ajWritebinByte(AjPFile thys, char ch);
size_t         ajWritebinChar(AjPFile thys, const char* txt, size_t len);
size_t         ajWritebinInt2(AjPFile thys, ajshort i);
size_t         ajWritebinInt4(AjPFile thys, ajint i);
size_t         ajWritebinInt8(AjPFile thys, ajlong i);
size_t         ajWritebinNewline(AjPFile thys);
size_t         ajWritebinStr(AjPFile thys, const AjPStr str, size_t len);
size_t         ajWritebinUint2(AjPFile thys, ajushort u);
size_t         ajWritebinUint4(AjPFile thys, ajuint u);
size_t         ajWritebinUint8(AjPFile thys, ajulong iu);
AjBool         ajWriteline(AjPFile file, const AjPStr line);
AjBool         ajWritelineNewline(AjPFile file, const AjPStr line);
AjBool         ajWritelineSpace(AjPFile file, const AjPStr line);

AjBool         ajReadline(AjPFile file, AjPStr *Pdest);
AjBool         ajReadlinePos(AjPFile file, AjPStr *Pdest, ajlong* Ppos);
AjBool         ajReadlineTrim(AjPFile file, AjPStr *Pdest);
AjBool         ajReadlineTrimPos(AjPFile file, AjPStr *Pdest, ajlong* Ppos);
AjBool         ajReadlineAppend(AjPFile file, AjPStr* Pdest);

AjBool         ajBuffreadLine(AjPFilebuff buff, AjPStr *pdest);
AjBool         ajBuffreadLinePos(AjPFilebuff buff, AjPStr *pdest,
                                 ajlong* Ppos);
AjBool         ajBuffreadLineStore(AjPFilebuff buff, AjPStr* pdest,
                                   AjBool store, AjPStr *astr);
AjBool         ajBuffreadLinePosStore(AjPFilebuff buff, AjPStr* pdest,
                                      ajlong* Ppos, AjBool store, AjPStr *astr);
AjBool         ajBuffreadLineTrim(AjPFilebuff buff, AjPStr* pdest);

/*
** End of prototype definitions
*/




#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif /* AJ_COMPILE_DEPRECATED_BOOK */

#ifdef AJ_COMPILE_DEPRECATED

__deprecated void    ajFileBuffLoad(AjPFilebuff thys);
__deprecated AjBool  ajFileBuffGet(AjPFilebuff thys, AjPStr *pdest);
__deprecated AjBool  ajFileBuffGetL(AjPFilebuff thys, AjPStr *pdest,
                                    ajlong* fpos);
__deprecated AjBool  ajFileBuffGetStore(AjPFilebuff thys, AjPStr* pdest,
                                        AjBool store, AjPStr *astr);
__deprecated AjBool  ajFileBuffGetStoreL(AjPFilebuff thys,
                                         AjPStr* pdest, ajlong* fpos,
                                         AjBool store, AjPStr *astr);
__deprecated AjBool  ajFileBuffGetTrim(AjPFilebuff thys, AjPStr* pdest);

__deprecated AjBool  ajFileGets(AjPFile thys, AjPStr *pdest);
__deprecated AjBool  ajFileGetsL(AjPFile thys,
                                 AjPStr *pdest, ajlong* fpos);
__deprecated AjBool  ajFileGetsTrim(AjPFile thys, AjPStr *pdest);
__deprecated AjBool  ajFileGetsTrimL(AjPFile thys,
                                     AjPStr *pdest, ajlong* fpos);
__deprecated AjBool  ajFileReadAppend(AjPFile thys, AjPStr* pbuff);
__deprecated AjBool  ajFileReadLine(AjPFile thys, AjPStr *pdest);
__deprecated ajint   ajFileWriteByte(AjPFile thys, char ch);
__deprecated ajint   ajFileWriteInt2(AjPFile thys, ajshort i);
__deprecated ajint   ajFileWriteInt4(AjPFile thys, ajint i);
__deprecated size_t  ajFileRead(void* ptr, size_t element_size,
                                size_t count, AjPFile thys);
__deprecated size_t  ajFileWrite(AjPFile file, const void* ptr,
                                 size_t element_size, size_t count);
__deprecated ajint   ajFileWriteStr(AjPFile thys,
                                    const AjPStr str, ajuint len);

#endif /* AJ_COMPILE_DEPRECATED */




AJ_END_DECLS

#endif /* !AJFILEIO_H */

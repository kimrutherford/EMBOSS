/* @include ajutil ************************************************************
**
** AJAX utility functions
** @author Copyright (C) 1998 Ian Longden
** @author Copyright (C) 1998 Peter Rice
** @version $Revision: 1.24 $
** @modified $Date: 2011/10/18 14:23:41 $ by $Author: rice $
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

#ifndef AJUTIL_H
#define AJUTIL_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajstr.h"

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

__noreturn void   ajExit(void);
__noreturn void   ajExitAbort(void);
__noreturn void   ajExitBad(void);

void          ajByteRevLen2(ajshort* i);
void          ajByteRevLen4(ajint* i);
void          ajByteRevLen8(ajlong* i);
void          ajByteRevLen2u(ajushort* i);
void          ajByteRevLen4u(ajuint* i);
void          ajByteRevLen8u(ajulong* i);
void          ajByteRevInt(ajint* sval);
void          ajByteRevShort(ajshort* ival);
void          ajByteRevLong(ajlong* lval);
void          ajByteRevUint(ajuint* ival);
void          ajByteRevUlong(ajulong* lval);

void          ajReset(void);

void          ajUtilCatch(void);
AjBool        ajUtilGetBigendian(void);
AjBool        ajUtilGetUid(AjPStr* Puid);
void          ajUtilLoginfo(void);
const AjPStr  ajUtilGetProgram(void);
const AjPStr  ajUtilGetCmdline(void);
const AjPStr  ajUtilGetInputs(void);

size_t ajUtilBase64DecodeC(AjPStr *dest, const char *src);
AjBool ajUtilBase64EncodeC(AjPStr *dest, size_t size, const unsigned char *src);

/*
** End of prototype definitions
*/




#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif /* AJ_COMPILE_DEPRECATED_BOOK */

#ifdef AJ_COMPILE_DEPRECATED

__deprecated void  ajAcdProgramS(AjPStr* pgm);
__deprecated const char*  ajAcdProgram(void);
__deprecated const AjPStr  ajAcdGetProgram(void);
__deprecated const AjPStr  ajAcdGetCmdline(void);
__deprecated const AjPStr  ajAcdGetInputs(void);
__deprecated void   ajLogInfo(void);
__deprecated void   ajUtilRev2(short* i);
__deprecated void   ajUtilRev4(ajint* i);
__deprecated void   ajUtilRev8(ajlong* i);
__deprecated void   ajUtilRevInt(ajint* ival);
__deprecated void   ajUtilRevShort(short* ival);
__deprecated void   ajUtilRevLong(ajlong* lval);
__deprecated void   ajUtilRevUint(ajuint* ival);
__deprecated AjBool ajUtilBigendian(void);
__deprecated AjBool ajUtilUid(AjPStr* dest);

#endif /* AJ_COMPILE_DEPRECATED */




AJ_END_DECLS

#endif /* !AJUTIL_H */

/* @include ajcall ************************************************************
**
** General routines for function callback
**
** @author Copyright (c) 1999 Alan Bleasby
** @version $Revision: 1.11 $
** @modified $Date: 2011/10/23 20:09:49 $ by $Author: mks $
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

#ifndef AJCALL_H
#define AJCALL_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajstr.h"
#include "ajtable.h"
#include <stdarg.h>

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




typedef void* (*CallFunc)(const char *name, va_list args);




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */




/*
** Prototype definitions
*/

void     ajCallRegister(const char *name, CallFunc func);
void     ajCallRegisterOld(const char *name, CallFunc func);
void     ajCallTableRegister(AjPTable table, const char *name, void* func);

AjPTable ajCallTableNew(void);
void*    ajCall(const char *name, ...);
void*    ajCallTableGetC(const AjPTable table, const char *name);
void*    ajCallTableGetS(const AjPTable table, const AjPStr namestr);
void     ajCallExit(void);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !AJCALL_H */

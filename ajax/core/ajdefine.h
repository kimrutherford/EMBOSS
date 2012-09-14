/* @include ajdefine **********************************************************
**
** AJAX master definitions include file
**
** @author Copyright (C) 1998 Peter Rice and Alan Bleasby
** @version $Revision: 1.29 $
** @modified $Date: 2012/04/12 20:35:29 $ by $Author: mks $
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
**
******************************************************************************/

#ifndef AJDEFINE_H
#define AJDEFINE_H

#if defined(__cplusplus)
#define AJ_BEGIN_DECLS extern "C" {
#define AJ_END_DECLS }
#else /* !__cplusplus */
#define AJ_BEGIN_DECLS
#define AJ_END_DECLS
#endif /* !__cplusplus */




/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ajarch.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */

#ifdef __GNUC__
#define __deprecated __attribute__((deprecated))
#define __noreturn __attribute__((noreturn))
#define __warn_unused_result __attribute__((warn_unused_result))
#else /* !__GNUC__ */
#define __deprecated
#define __noreturn
#define __warn_unused_result
#endif /* !__GNUC__ */

#ifndef WIN32
#define SLASH_CHAR   '/'
#define SLASH_STRING "/"
#define CURRENT_DIR  "./"
#define UP_DIR       "../"
#define PATH_SEPARATOR ":"
#define _BLACKHOLE "/dev/null"
#else
#define SLASH_CHAR   '\\'
#define SLASH_STRING "\\"
#define CURRENT_DIR  ".\\"
#define UP_DIR       "..\\"
#define PATH_SEPARATOR ";"
#define _BLACKHOLE "nul:"
#endif

#define AJAXLONGDOUBLE double

#define AJBOOL(b) (b ? "TRUE" : "FALSE")

#define AJFALSE 0
#define AJTRUE 1

#define ajFalse 0
#define ajTrue 1

#define CASE2(a,b) ((a << 8) + b)
#define CASE3(a,b,c) ((a << 16) + (b << 8) + c)
#define CASE4(a,b,c,d) ((a << 24) + (b << 16) + (c << 8) + d)

#define STRCASE2(a) ((a[0] << 8) + a[1])

#define U_FEPS 1.192e-6F          /* 1.0F + E_FEPS != 1.0F */
#define U_DEPS 2.22e-15           /* 1.0 +  E_DEPS != 1.0  */

#define E_FPEQ(a,b,e) (((b - e) < a) && (a < (b + e)))

#define E_FPZERO(a,e) (fabs((double)a) <= (double) e)




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */




/*
** Prototype definitions
*/

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !AJDEFINE_H */

/* @include ajassert **********************************************************
**
** AJAX assert functions
**
** Functions to handle assert statements.
**
** @author Copyright (C) 1998 Ian Longden
** @version $Revision: 1.4 $
** @modified Jun 25 pmr First version
** @modified $Date: 2011/10/02 10:11:48 $ by $Author: mks $
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

#ifndef AJASSERT_H
#define AJASSERT_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"

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

#undef assert
#ifdef NDEBUG
#define assert(e) ((void)0)
#else
#include "ajexcept.h"
extern void assert(ajint e);
#define assert(e) ((void)((e)||(AJRAISE(Assert_Failed),0)))
#endif




AJ_END_DECLS

#endif  /* !AJASSERT_H */

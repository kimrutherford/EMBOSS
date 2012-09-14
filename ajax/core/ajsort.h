/* @include ajsort ************************************************************
**
** AJAX sort functions
**
** @author Copyright (C) 2001 Alan Bleasby
** @version $Revision: 1.9 $
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

#ifndef AJSORT_H
#define AJSORT_H

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




/*
** Prototype definitions
*/

void ajSortFloatDecI(const float *a, ajuint *p, ajuint n);
void ajSortIntDecI(const ajint *a, ajuint *p, ajuint n);
void ajSortUintDecI(const ajuint *a, ajuint *p, ajuint n);
void ajSortFloatIncI(const float *a, ajuint *p, ajuint n);
void ajSortIntIncI(const ajint *a, ajuint *p, ajuint n);
void ajSortUintIncI(const ajuint *a, ajuint *p, ajuint n);
void ajSortFloatDec(float *a, ajuint n);
void ajSortIntDec(ajint *a, ajuint n);
void ajSortUintDec(ajuint *a, ajuint n);
void ajSortFloatInc(float *a, ajuint n);
void ajSortIntInc(ajint *a, ajuint n);
void ajSortUintInc(ajuint *a, ajuint n);
void ajSorttwoIntIncI(ajint *a, ajuint *p, ajuint n);
void ajSorttwoUintIncI(ajuint *a, ajuint *p, ajuint n);

/*
** End of prototype definitions
*/




#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif /* AJ_COMPILE_DEPRECATED_BOOK */

#ifdef AJ_COMPILE_DEPRECATED

__deprecated void ajSortTwoIntIncI(ajint *a, ajuint *p, ajuint n);
__deprecated void ajSortTwoUintIncI(ajuint *a, ajuint *p, ajuint n);

#endif /* AJ_COMPILE_DEPRECATED */




AJ_END_DECLS

#endif /* !AJSORT_H */

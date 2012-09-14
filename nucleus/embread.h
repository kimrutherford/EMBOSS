/* @include embread ***********************************************************
**
** Data file reading routines
**
** @author Copyright (c) 1999 Alan Bleasby
** @version $Revision: 1.6 $
** @modified $Date: 2011/09/27 11:19:27 $ by $Author: rice $
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

#ifndef EMBREAD_H
#define EMBREAD_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




#define AJREADAMINO 28



/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */





/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */



/*
** Prototype definitions
*/

AjBool embReadAminoDataDoubleC (const char *s, double **a, double fill);
AjBool embReadAminoDataFloatC  (const char *s, float **a, float fill);
AjBool embReadAminoDataIntC    (const char *s, ajint **a, ajint fill);

/*
** End of prototype definitions
*/

AJ_END_DECLS

#endif  /* !EMBREAD_H */

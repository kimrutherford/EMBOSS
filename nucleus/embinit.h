/* @include embinit ***********************************************************
**
** General routines for initialisation
**
** @author Copyright (c) 1999 Alan Bleasby
** @version $Revision: 1.7 $
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
******************************************************************************/

#ifndef EMBINIT_H
#define EMBINIT_H




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

void embInit (const char *pgm, ajint argc, char * const argv[]);
void embInitP (const char *pgm, ajint argc, char * const argv[],
	       const char *package);

void embInitPV (const char *pgm, ajint argc, char * const argv[],
	       const char *package, const char *packversion);

/*
** End of prototype definitions
*/

AJ_END_DECLS

#endif  /* !EMBINIT_H */

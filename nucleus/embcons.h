/* @include embcons ***********************************************************
**
** General routines for consensus.
**
** @author Copyright (c) Copyright 2001 (C) Tim Carver (tcarver@hgmp.mrc.ac.uk)
** @version $Revision: 1.7 $
** @modified $Date: 2011/10/18 14:24:24 $ by $Author: rice $
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

#ifndef EMBCONS_H
#define EMBCONS_H




/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajstr.h"
#include "ajseqdata.h"
#include "ajmatrices.h"

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

void embConsCalc (const AjPSeqset seqset, const AjPMatrix cmpmatrix,
	ajint nseqs, ajint mlen,float fplural, float setcase,
	ajint identity, AjBool gaps, AjPStr *cons);

/*
** End of prototype definitions
*/


AJ_END_DECLS

#endif  /* !EMBCONS_H */

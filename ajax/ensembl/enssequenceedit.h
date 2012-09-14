/* @include enssequenceedit ***************************************************
**
** Ensembl Sequence Edit functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.12 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/02/04 10:30:24 $ by $Author: mks $
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

#ifndef ENSSEQUENCEEDIT_H
#define ENSSEQUENCEEDIT_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ensattribute.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */

/* @data EnsPSequenceedit *****************************************************
**
** Ensembl Sequence Edit.
**
** @alias EnsSSequenceedit
** @alias EnsOSequenceedit
**
** @cc Bio::EnsEMBL::Attribute
** @attr Attribute [EnsPAttribute] Ensembl Attribute
** @attr Sequence [AjPStr] Alternative sequence
** @attr Start [ajuint] Start coordinate
** @attr End [ajuint] End coordinate
** @attr Use [ajuint] Use counter
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSSequenceedit
{
    EnsPAttribute Attribute;
    AjPStr Sequence;
    ajuint Start;
    ajuint End;
    ajuint Use;
    ajuint Padding;
} EnsOSequenceedit;

#define EnsPSequenceedit EnsOSequenceedit*




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */

/*
** Prototype definitions
*/

/* Ensembl Sequence Edit */

EnsPSequenceedit ensSequenceeditNewAttribute(EnsPAttribute attribute);

EnsPSequenceedit ensSequenceeditNewCpy(const EnsPSequenceedit se);

EnsPSequenceedit ensSequenceeditNewIni(EnsPAttributetype at,
                                       AjPStr sequence,
                                       ajuint start,
                                       ajuint end);

EnsPSequenceedit ensSequenceeditNewRef(EnsPSequenceedit se);

void ensSequenceeditDel(EnsPSequenceedit *Pse);

EnsPAttribute ensSequenceeditGetAttribute(const EnsPSequenceedit se);

ajuint ensSequenceeditGetEnd(const EnsPSequenceedit se);

AjPStr ensSequenceeditGetSequence(const EnsPSequenceedit se);

ajuint ensSequenceeditGetStart(const EnsPSequenceedit se);

AjBool ensSequenceeditTrace(const EnsPSequenceedit se, ajuint level);

ajint ensSequenceeditCalculateDifference(const EnsPSequenceedit se);

size_t ensSequenceeditCalculateMemsize(const EnsPSequenceedit se);

AjBool ensSequenceeditApplyString(const EnsPSequenceedit se,
                                  ajint offset,
                                  AjPStr *Psequence);

AjBool ensListSequenceeditSortStartAscending(AjPList ses);

AjBool ensListSequenceeditSortStartDescending(AjPList ses);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSSEQUENCEEDIT_H */

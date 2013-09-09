/* @include enstable **********************************************************
**
** Ensembl Table functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.17 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2013/02/17 13:09:10 $ by $Author: mks $
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

#ifndef ENSTABLE_H
#define ENSTABLE_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajax.h"

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

/*
** AJAX Table of AJAX String key data and
** generic value data.
*/

AjBool ensTablestrFromList(
    AjPTable table,
    AjPStr (*FobjectGetKey) (const void *object),
    AjPList list);

AjBool ensTablestrToList(AjPTable table, AjPList list);

/*
** AJAX Table of AJAX unsigned integer key data and
** generic value data.
*/

AjBool ensTableuintFromList(
    AjPTable table,
    ajuint (*FobjectGetIdentifier) (const void *object),
    AjPList list);

AjBool ensTableuintToList(AjPTable table, AjPList list);

/*
** AJAX Table of
** AJAX unsigned integer key data and
** AJAX String-List value data.
*/

AjPTable ensTableuintliststrNewLen(ajuint size);

void ensTableuintliststrClear(AjPTable table);

void ensTableuintliststrDelete(AjPTable *Ptable);

AjBool ensTableuintliststrRegister(AjPTable table,
                                   ajuint identifier,
                                   AjPStr str);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSTABLE_H */

/* @include ajobowrite ********************************************************
**
** AJAX OBO writing functions
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.6 $
** @modified May 5 pmr 2010 First AJAX version
** @modified Sep 8 2010 pmr Added output formats
** @modified $Date: 2011/10/18 14:23:40 $ by $Author: rice $
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

#ifndef AJOBOWRITE_H
#define AJOBOWRITE_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajobodata.h"

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

AjBool        ajObooutWrite(AjPOutfile outf, const AjPObo obo);

void          ajObooutprintBook(AjPFile outf);
void          ajObooutprintHtml(AjPFile outf);
void          ajObooutprintText(AjPFile outf, AjBool full);
void          ajObooutprintWiki(AjPFile outf);

AjBool        ajObooutformatFind(const AjPStr format, ajint* iformat);
AjBool        ajObooutformatTest(const AjPStr format);

void          ajObooutExit(void);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !AJOBOWRITE_H */

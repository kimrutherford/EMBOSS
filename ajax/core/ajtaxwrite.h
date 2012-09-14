/* @include ajtaxwrite ********************************************************
**
** AJAX taxonomy writing functions
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.6 $
** @modified Oct 25 2010 pmr First AJAX version
** @modified $Date: 2011/10/18 14:23:41 $ by $Author: rice $
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

#ifndef AJTAXWRITE_H
#define AJTAXWRITE_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajtaxdata.h"

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

AjBool        ajTaxoutWrite(AjPOutfile outf, const AjPTax tax);

void          ajTaxoutprintBook(AjPFile outf);
void          ajTaxoutprintHtml(AjPFile outf);
void          ajTaxoutprintText(AjPFile outf, AjBool full);
void          ajTaxoutprintWiki(AjPFile outf);

AjBool        ajTaxoutformatFind(const AjPStr format, ajint* iformat);
AjBool        ajTaxoutformatTest(const AjPStr format);

void          ajTaxoutExit(void);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !AJTAXWRITE_H */

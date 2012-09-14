/* @include ajtextwrite *******************************************************
**
* AJAX text data writing functions
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.6 $
** @modified May 5 pmr 2010 First AJAX version
** @modified Sep 8 2010 pmr Added query and reading functions
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

#ifndef AJTEXTWRITE_H
#define AJTEXTWRITE_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajtextdata.h"

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

AjBool        ajTextoutWrite(AjPOutfile outf, const AjPText text);

void          ajTextoutprintBook(AjPFile outf);
void          ajTextoutprintHtml(AjPFile outf);
void          ajTextoutprintText(AjPFile outf, AjBool full);
void          ajTextoutprintWiki(AjPFile outf);

AjBool        ajTextoutformatFind(const AjPStr format, ajint* iformat);
AjBool        ajTextoutformatTest(const AjPStr format);

void          ajTextoutExit(void);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !AJTEXTWRITE_H */

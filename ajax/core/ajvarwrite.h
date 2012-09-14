/* @include ajvarwrite ********************************************************
**
** AJAX variation data writing functions
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.7 $
** @modified Oct 25 2010 pmr First AJAX version
** @modified $Date: 2011/12/19 16:33:15 $ by $Author: rice $
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

#ifndef AJVARWRITE_H
#define AJVARWRITE_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajvardata.h"

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

AjBool        ajVaroutWrite(AjPOutfile outf, const AjPVar var);
AjBool        ajVaroutWriteNext(AjPOutfile outf, const AjPVar var);

void          ajVaroutprintBook(AjPFile outf);
void          ajVaroutprintHtml(AjPFile outf);
void          ajVaroutprintText(AjPFile outf, AjBool full);
void          ajVaroutprintWiki(AjPFile outf);

AjBool        ajVaroutformatFind(const AjPStr format, ajint* iformat);
AjBool        ajVaroutformatTest(const AjPStr format);

void          ajVaroutExit(void);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !AJVARWRITE_H */

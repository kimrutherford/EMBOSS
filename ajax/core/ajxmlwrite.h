/* @include ajxmlwrite *******************************************************
**
** __AJAX or NUCLEUS__  __LibraryArea__ functions
**
** __ShortDescription__
**
** @author Copyright (C) __Year__  __AuthorName__
** @version $Revision: 1.1 $
** @modified __EditDate__  __EditorName__  __Description of Edit__
** @modified $Date: 2012/09/03 14:14:45 $ by $Author: rice $
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

#ifndef AjPXMLWRITE_H
#define AjPXMLWRITE_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajxmldata.h"

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

AjBool        ajXmloutWrite(AjPOutfile outf, const AjPXml xml);

void          ajXmloutprintBook(AjPFile outf);
void          ajXmloutprintHtml(AjPFile outf);
void          ajXmloutprintText(AjPFile outf, AjBool full);
void          ajXmloutprintWiki(AjPFile outf);

AjBool        ajXmloutformatFind(const AjPStr format, ajint* iformat);
AjBool        ajXmloutformatTest(const AjPStr format);

void          ajXmloutExit(void);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !AjPXMLWRITE_H */

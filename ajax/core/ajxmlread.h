/* @include ajxmlread ********************************************************
**
** __AJAX or NUCLEUS__  __LibraryArea__ functions
**
** __ShortDescription__
**
** @author Copyright (C) __Year__  __AuthorName__
** @version $Revision: 1.2 $
** @modified __EditDate__  __EditorName__  __Description of Edit__
** @modified $Date: 2012/11/14 14:21:13 $ by $Author: rice $
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

#ifndef AjPXMLREAD_H
#define AjPXMLREAD_H

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

void             ajXmlallClear(AjPXmlall thys);
void             ajXmlallDel(AjPXmlall* pthis);
AjPXmlall        ajXmlallNew(void);
const AjPStr     ajXmlallGetxmlId(const AjPXmlall thys);

void             ajXmlinClear(AjPXmlin thys);
void             ajXmlinDel(AjPXmlin *pthis);
AjPXmlin         ajXmlinNew(void);
void             ajXmlinQryC(AjPXmlin thys, const char* txt);
void             ajXmlinQryS(AjPXmlin thys, const AjPStr str);
void             ajXmlinTrace(const AjPXmlin thys);

void             ajXmlinprintBook(AjPFile outf);
void             ajXmlinprintHtml(AjPFile outf);
void             ajXmlinprintText(AjPFile outf, AjBool full);
void             ajXmlinprintWiki(AjPFile outf);

AjBool           ajXmlallNext(AjPXmlall thys, AjPXml *Pxml);
AjBool           ajXmlinRead(AjPXmlin xmlin, AjPXml thys);

AjBool           ajXmlinformatTerm(const AjPStr term);
AjBool           ajXmlinformatTest(const AjPStr format);
void             ajXmlinExit(void);
const char*      ajXmlinTypeGetFields(void);
const char*      ajXmlinTypeGetQlinks(void);

AjPTable         ajXmlaccessGetDb(void);
const char*      ajXmlaccessMethodGetQlinks(const AjPStr method);
ajuint           ajXmlaccessMethodGetScope(const AjPStr method);
AjBool           ajXmlaccessMethodTest(const AjPStr method);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !AjPXMLREAD_H */

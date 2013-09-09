/* @include ajxml ************************************************************
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

#ifndef AJXWYZ_H
#define AJXWYZ_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

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

AjPXml      ajXmlNew(void);
void         ajXmlDel(AjPXml *Pxml);
void         ajXmlClear(AjPXml xml);
const AjPStr ajXmlGetDb(const AjPXml xml);
const AjPStr ajXmlGetEntry(const AjPXml xml);
const AjPStr ajXmlGetId(const AjPXml xml);
const char*  ajXmlGetQryC(const AjPXml xml);
const AjPStr ajXmlGetQryS(const AjPXml xml);
void         ajXmlExit(void);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !AjPXML_H */

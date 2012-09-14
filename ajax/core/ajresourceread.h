/* @include ajresourceread ****************************************************
**
** AJAX data resource reading functions
**
** These functions control all aspects of AJAX data resource reading
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.12 $
** @modified Oct 5 pmr First version
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

#ifndef AJRESOURCEREAD_H
#define AJRESOURCEREAD_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajresourcedata.h"

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

void           ajResourceallClear(AjPResourceall thys);
void           ajResourceallDel(AjPResourceall* pthis);
AjPResourceall ajResourceallNew(void);
AjPResourcein  ajResourceinNewDrcat(const AjPStr dbname);
const AjPStr   ajResourceallGetresourceId(const AjPResourceall thys);

void           ajResourceinClear(AjPResourcein thys);
void           ajResourceinDel(AjPResourcein* pthis);
AjPResourcein  ajResourceinNew(void);
void           ajResourceinQryC(AjPResourcein thys, const char* txt);
void           ajResourceinQryS(AjPResourcein thys, const AjPStr str);
void           ajResourceinTrace(const AjPResourcein thys);

void           ajResourceinprintBook(AjPFile outf);
void           ajResourceinprintHtml(AjPFile outf);
void           ajResourceinprintText(AjPFile outf, AjBool full);
void           ajResourceinprintWiki(AjPFile outf);

AjBool         ajResourceallNext(AjPResourceall thys, AjPResource *Presource);
AjBool         ajResourceinRead(AjPResourcein resourcein, AjPResource thys);

AjBool         ajResourceinformatTerm(const AjPStr term);
AjBool         ajResourceinformatTest(const AjPStr format);
void           ajResourceinExit(void);
const char*    ajResourceinTypeGetFields(void);
const char*    ajResourceinTypeGetQlinks(void);

AjPTable       ajResourceaccessGetDb(void);
const char*    ajResourceaccessMethodGetQlinks(const AjPStr method);
ajuint         ajResourceaccessMethodGetScope(const AjPStr method);
AjBool         ajResourceaccessMethodTest(const AjPStr method);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !AJRESOURCEREAD_H */

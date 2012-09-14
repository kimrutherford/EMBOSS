/* @include ajtaxread *********************************************************
**
** AJAX taxonomy reading functions
**
** These functions control all aspects of AJAX taxonomy reading
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.10 $
** @modified Oct 5 pmr First version
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

#ifndef AJTAXREAD_H
#define AJTAXREAD_H

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

void             ajTaxallClear(AjPTaxall thys);
void             ajTaxallDel(AjPTaxall* pthis);
AjPTaxall        ajTaxallNew(void);
const AjPStr     ajTaxallGettaxId(const AjPTaxall thys);

void             ajTaxinClear(AjPTaxin thys);
void             ajTaxinDel(AjPTaxin* pthis);
AjPTaxin         ajTaxinNew(void);
void             ajTaxinQryC(AjPTaxin thys, const char* txt);
void             ajTaxinQryS(AjPTaxin thys, const AjPStr str);
void             ajTaxinTrace(const AjPTaxin thys);

void             ajTaxinprintBook(AjPFile outf);
void             ajTaxinprintHtml(AjPFile outf);
void             ajTaxinprintText(AjPFile outf, AjBool full);
void             ajTaxinprintWiki(AjPFile outf);

AjBool           ajTaxallNext(AjPTaxall thys, AjPTax *Ptax);
AjBool           ajTaxinRead(AjPTaxin taxin, AjPTax thys);

AjBool           ajTaxinformatTest(const AjPStr format);
void             ajTaxinExit(void);
const char*      ajTaxinTypeGetFields(void);
const char*      ajTaxinTypeGetQlinks(void);

AjPTable         ajTaxaccessGetDb(void);
const char*      ajTaxaccessMethodGetQlinks(const AjPStr method);
ajuint           ajTaxaccessMethodGetScope(const AjPStr method);
AjBool           ajTaxaccessMethodTest(const AjPStr method);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !AJTAXREAD_H */

/* @include ajvarread *********************************************************
**
** AJAX variation data reading functions
**
** These functions control all aspects of AJAX variation data reading
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.7 $
** @modified Oct 5 pmr First version
** @modified $Date: 2012/07/02 18:07:01 $ by $Author: rice $
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

#ifndef AJVARREAD_H
#define AJVARREAD_H

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

void             ajVarallClear(AjPVarall thys);
void             ajVarallDel(AjPVarall* pthis);
AjPVarall        ajVarallNew(void);
const AjPStr     ajVarallGetvarId(const AjPVarall thys);

void             ajVarloadClear(AjPVarload thys);
void             ajVarloadDel(AjPVarload* pthis);
AjPVarload       ajVarloadNew(void);
const AjPStr     ajVarloadGetvarId(const AjPVarload thys);

void             ajVarinClear(AjPVarin thys);
void             ajVarinDel(AjPVarin* pthis);
AjPVarin         ajVarinNew(void);
void             ajVarinQryC(AjPVarin thys, const char* txt);
void             ajVarinQryS(AjPVarin thys, const AjPStr str);
void             ajVarinTrace(const AjPVarin thys);

void             ajVarinprintBook(AjPFile outf);
void             ajVarinprintHtml(AjPFile outf);
void             ajVarinprintText(AjPFile outf, AjBool full);
void             ajVarinprintWiki(AjPFile outf);

AjBool           ajVarallNext(AjPVarall thys, AjPVarload *Pvarload);
AjBool           ajVarloadMore(AjPVarload thys, AjPVar *Pvar);
AjBool           ajVarloadNext(AjPVarload thys, AjPVar *Pvar);
AjBool           ajVarinLoad(AjPVarin varin, AjPVar thys);

AjBool           ajVarinformatTerm(const AjPStr term);
AjBool           ajVarinformatTest(const AjPStr format);
void             ajVarinExit(void);
const char*      ajVarinTypeGetFields(void);
const char*      ajVarinTypeGetQlinks(void);

AjPTable         ajVaraccessGetDb(void);
const char*      ajVaraccessMethodGetQlinks(const AjPStr method);
ajuint           ajVaraccessMethodGetScope(const AjPStr method);
AjBool           ajVaraccessMethodTest(const AjPStr method);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !AJVARREAD_H */

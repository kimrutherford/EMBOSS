/* @include ajurlread *********************************************************
**
** AJAX url reading functions
**
** These functions control all aspects of AJAX url reading
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.5 $
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

#ifndef AJURLREAD_H
#define AJURLREAD_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajurldata.h"

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

void             ajUrlallClear(AjPUrlall thys);
void             ajUrlallDel(AjPUrlall* pthis);
AjPUrlall        ajUrlallNew(void);
const AjPStr     ajUrlallGeturlId(const AjPUrlall thys);

void             ajUrlinClear(AjPUrlin thys);
void             ajUrlinDel(AjPUrlin* pthis);
AjPUrlin         ajUrlinNew(void);
void             ajUrlinQryC(AjPUrlin thys, const char* txt);
void             ajUrlinQryS(AjPUrlin thys, const AjPStr str);
void             ajUrlinTrace(const AjPUrlin thys);

void             ajUrlinprintBook(AjPFile outf);
void             ajUrlinprintHtml(AjPFile outf);
void             ajUrlinprintText(AjPFile outf, AjBool full);
void             ajUrlinprintWiki(AjPFile outf);

AjBool           ajUrlallNext(AjPUrlall thys, AjPUrl *Purl);
AjBool           ajUrlinRead(AjPUrlin urlin, AjPUrl thys);

AjBool           ajUrlinformatTerm(const AjPStr term);
AjBool           ajUrlinformatTest(const AjPStr format);
void             ajUrlinExit(void);
const char*      ajUrlinTypeGetFields(void);
const char*      ajUrlinTypeGetQlinks(void);

AjPTable         ajUrlaccessGetDb(void);
const char*      ajUrlaccessMethodGetQlinks(const AjPStr method);
ajuint           ajUrlaccessMethodGetScope(const AjPStr method);
AjBool           ajUrlaccessMethodTest(const AjPStr method);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !AJURLREAD_H */

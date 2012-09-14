/* @include ajoboread *********************************************************
**
** AJAX OBO reading functions
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.10 $
** @modified May 5 pmr 2010 First AJAX version
** @modified Sep 8 2010 pmr Added query and reading functions
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

#ifndef AJOBOREAD_H
#define AJOBOREAD_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajobodata.h"

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

AjPOboData       ajObodataNew(void);
AjPOboData       ajObodataParseObofile(AjPFile infile, const char* validations);
AjPObo           ajObodataFetchId(const AjPOboData thys, const AjPStr query);
AjPObo           ajObodataFetchName(const AjPOboData thys, const AjPStr query);

AjBool           ajObolineCutComment(AjPStr *Pline, AjPStr *Pcomment);
AjBool           ajObolineCutModifier(AjPStr *Pline, AjPStr *Pmodifier);
AjBool           ajObolineCutDbxref(AjPStr *Pline, AjPStr *Pdbxref);
AjBool           ajObolineEscape(AjPStr *Pline);

void             ajOboallClear(AjPOboall thys);
void             ajOboallDel(AjPOboall* pthis);
AjPOboall        ajOboallNew(void);
const AjPStr     ajOboallGetoboId(const AjPOboall thys);
const AjPStr     ajOboallGetQryS(const AjPOboall thys);

void             ajOboinClear(AjPOboin thys);
void             ajOboinDel(AjPOboin* pthis);
const AjPStr     ajOboinGetQryS(const AjPOboin thys);
AjPOboin         ajOboinNew(void);
void             ajOboinQryC(AjPOboin thys, const char* txt);
void             ajOboinQryS(AjPOboin thys, const AjPStr str);
void             ajOboinTrace(const AjPOboin thys);
const char*      ajOboinTypeGetFields(void);
const char*      ajOboinTypeGetQlinks(void);

void             ajOboinprintBook(AjPFile outf);
void             ajOboinprintHtml(AjPFile outf);
void             ajOboinprintText(AjPFile outf, AjBool full);
void             ajOboinprintWiki(AjPFile outf);

AjBool           ajOboallNext(AjPOboall thys, AjPObo *Pobo);
AjBool           ajOboinRead(AjPOboin oboin, AjPObo thys);

ajuint           ajOboqueryGetallObofields(const AjPQuery query,
                                           const AjPStr *id, const AjPStr *acc,
                                           const AjPStr *nam, const AjPStr *des,
                                           const AjPStr *up, const AjPStr *div);

AjBool           ajOboinformatTerm(const AjPStr term);
AjBool           ajOboinformatTest(const AjPStr format);
void             ajOboinExit(void);

AjPTable         ajOboaccessGetDb(void);
const char*      ajOboaccessMethodGetQlinks(const AjPStr method);
ajuint           ajOboaccessMethodGetScope(const AjPStr method);
AjBool           ajOboaccessMethodTest(const AjPStr method);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !AJOBOREAD_H */

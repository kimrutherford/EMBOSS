/* @include ajassemread *******************************************************
**
** AJAX assembly reading functions
**
** These functions control all aspects of AJAX assembly reading
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.10 $
** @modified Oct 5 pmr First version
** @modified $Date: 2012/07/02 16:44:55 $ by $Author: rice $
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

#ifndef AJASSEMREAD_H
#define AJASSEMREAD_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajstr.h"
#include "ajtable.h"
#include "ajfile.h"
#include "ajassemdata.h"

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

void             ajAssemloadClear(AjPAssemload thys);
void             ajAssemloadDel(AjPAssemload* pthis);
AjPAssemload     ajAssemloadNew(void);


void             ajAsseminClear(AjPAssemin thys);
void             ajAsseminDel(AjPAssemin* pthis);
AjPAssemin       ajAsseminNew(void);
void             ajAsseminQryC(AjPAssemin thys, const char* txt);
void             ajAsseminQryS(AjPAssemin thys, const AjPStr str);
void             ajAsseminTrace(const AjPAssemin thys);

void             ajAsseminprintBook(AjPFile outf);
void             ajAsseminprintHtml(AjPFile outf);
void             ajAsseminprintText(AjPFile outf, AjBool full);
void             ajAsseminprintWiki(AjPFile outf);

AjBool           ajAssemloadMore(AjPAssemload thys, AjPAssem *Pvar);
AjBool           ajAssemloadNext(AjPAssemload thys, AjPAssem *Pvar);

AjBool           ajAsseminLoad(AjPAssemin assemin, AjPAssem thys);

AjBool           ajAsseminformatTerm(const AjPStr format);
AjBool           ajAsseminformatTest(const AjPStr format);
void             ajAsseminExit(void);
const char*      ajAsseminTypeGetFields(void);
const char*      ajAsseminTypeGetQlinks(void);

AjPTable         ajAssemaccessGetDb(void);
const char*      ajAssemaccessMethodGetQlinks(const AjPStr method);
ajuint           ajAssemaccessMethodGetScope(const AjPStr method);
AjBool           ajAssemaccessMethodTest(const AjPStr method);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !AJASSEMREAD_H */

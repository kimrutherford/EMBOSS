/* @include ajfeatread ********************************************************
**
** AJAX feature reading functions
**
** These functions control all aspects of AJAX feature reading
**
** @author Copyright (C) 1999 Richard Bruskiewich
** @version $Revision: 1.9 $
** @modified 2000 Ian Longden.
** @modified 2001 Peter Rice.
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

#ifndef AJFEATREAD_H
#define AJFEATREAD_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajstr.h"
#include "ajtable.h"
#include "ajfeatdata.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




extern AjPTable feattabDbMethods;




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */




/*
** Prototype definitions
*/

void          ajFeattaballClear(AjPFeattaball thys);
void          ajFeattaballDel(AjPFeattaball* pthis);
AjPFeattaball ajFeattaballNew(void);
const AjPStr  ajFeattaballGetfeattableId(const AjPFeattaball thys);

void          ajFeattabinClear(AjPFeattabin thys);
void          ajFeattabinDel(AjPFeattabin* pthis);
AjPFeattabin  ajFeattabinNew(void);
AjPFeattabin  ajFeattabinNewCSF(const char* fmt, const AjPStr name,
                                const char* type, AjPFilebuff buff);
AjPFeattabin  ajFeattabinNewSS(const AjPStr fmt, const AjPStr name,
                               const char* type);
AjPFeattabin  ajFeattabinNewSSF(const AjPStr fmt, const AjPStr name,
                                const char* type, AjPFilebuff buff);
void          ajFeattabinSetRange(AjPFeattabin thys,
                                  ajint fbegin, ajint fend);
AjBool        ajFeattabinSetTypeC(AjPFeattabin thys, const char* type);
AjBool        ajFeattabinSetTypeS(AjPFeattabin thys, const AjPStr type);

void          ajFeatinPrintFormat(AjPFile outf, AjBool full);
void          ajFeatinPrinthtmlFormat(AjPFile outf);
void          ajFeatinPrintbookFormat(AjPFile outf);
void          ajFeatinPrintwikiFormat(AjPFile outf);
void          ajFeatreadExit(void);
const char*   ajFeattabinTypeGetFields(void);
const char*   ajFeattabinTypeGetQlinks(void);

AjPTable      ajFeattabaccessGetDb(void);
const char*   ajFeattabaccessMethodGetQlinks(const AjPStr method);
ajuint        ajFeattabaccessMethodGetScope(const AjPStr method);
AjBool        ajFeattabaccessMethodTest(const AjPStr method);
AjBool        ajFeattabinformatTerm(const AjPStr term);
AjBool        ajFeattabinformatTest(const AjPStr format);

void          ajFeattabinQryC(AjPFeattabin thys, const char* txt);
void          ajFeattabinQryS(AjPFeattabin thys, const AjPStr txt);

AjBool        ajFeattabinRead(AjPFeattabin ftin, AjPFeattable ftable);

AjBool        ajFeattaballNext(AjPFeattaball thys, AjPFeattable *Pfeattable);
AjPFeattable  ajFeattableNewRead(AjPFeattabin ftin);
AjPFeattable  ajFeattableNewReadUfo(AjPFeattabin tabin, const AjPStr Ufo);

/*
** End of prototype definitions
*/




#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif
#ifdef AJ_COMPILE_DEPRECATED

__deprecated void          ajFeattabInClear(AjPFeattabIn thys);
__deprecated void          ajFeattabInDel(AjPFeattabIn* pthis);
__deprecated AjPFeattabIn  ajFeattabInNew(void);
__deprecated AjPFeattabIn  ajFeattabInNewCSF(const char* fmt, const AjPStr name,
                                             const char* type, AjPFilebuff buff);
__deprecated AjPFeattabIn  ajFeattabInNewSS(const AjPStr fmt, const AjPStr name,
                                            const char* type);
__deprecated AjPFeattabIn  ajFeattabInNewSSF(const AjPStr fmt, const AjPStr name,
                                             const char* type, AjPFilebuff buff);
__deprecated AjBool        ajFeattabInSetType(AjPFeattabIn thys, const AjPStr type);
__deprecated AjBool        ajFeattabInSetTypeC(AjPFeattabIn thys, const char* type);

#endif




AJ_END_DECLS

#endif /* !AJFEATREAD_H */

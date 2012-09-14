/* @include ajtagval **********************************************************
**
** Handling of tag-value pairs of strings
**
** @author Copyright (C) 2011 Peter Rice
** @version $Revision: 1.2 $
** @modified $Date: 2012/06/26 13:03:59 $ by $Author: rice $
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
******************************************************************************/

#ifndef AJTAGVAL_H
#define AJTAGVAL_H




/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajstr.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* @data AjPTagval ************************************************************
**
** General tag values data structure
**
** @alias AjSTagval
** @alias AjOTagval
**
** @attr Tag [AjPStr] Tag name usually from a controlled internal vocabulary
** @attr Value [AjPStr] Tag value
** @@
******************************************************************************/

typedef struct AjSTagval
{
    AjPStr Tag;
    AjPStr Value;
} AjOTagval;

#define AjPTagval AjOTagval*




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */




/*
** Prototype definitions
*/

AjPTagval    ajTagvalNewC(const char* tag, const char* value);
AjPTagval    ajTagvalNewS(const AjPStr tag, const AjPStr value);
void         ajTagvalDel(AjPTagval *Ptagval);
const AjPStr ajTagvalGetTag(const AjPTagval tagval);
const AjPStr ajTagvalGetValue(const AjPTagval tagval);
void         ajTagvalFmtLower(AjPTagval tagval);
void         ajTagvalFmtUpper(AjPTagval tagval);
void         ajTagvalAppendC(AjPTagval tagval, const char* txt);
void         ajTagvalAppendS(AjPTagval tagval, const AjPStr str);
void         ajTagvalReplaceC(AjPTagval tagval, const char* txt);
void         ajTagvalReplaceS(AjPTagval tagval, const AjPStr str);

#define MAJTAGVALGETTAG(tagval) ((tagval) ? (tagval)->Tag : NULL)
#define MAJTAGVALGETVALUE(tagval) ((tagval) ? (tagval)->Value : NULL)

/*
** End of prototype definitions
*/

#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif /* AJ_COMPILE_DEPRECATED_BOOK */

#ifdef AJ_COMPILE_DEPRECATED

#endif /* AJ_COMPILE_DEPRECATED */




AJ_END_DECLS

#endif /* AJTAGVAL_H */

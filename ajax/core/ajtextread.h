/* @include ajtextread ********************************************************
**
** AJAX text data reading functions
**
** These functions control all aspects of AJAX text data reading
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.14 $
** @modified Oct 5 pmr First version
** @modified $Date: 2013/07/15 20:55:34 $ by $Author: rice $
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

#ifndef AJTEXTREAD_H
#define AJTEXTREAD_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajtextdata.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */

extern AjPTable textDbMethods;




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */




/*
** Prototype definitions
*/

AjPTextin    ajTextinNew(void);
AjPTextin    ajTextinNewDatatype(const AjEDataType datatype);
void         ajTextinDel(AjPTextin* pthis);
void         ajTextinDelNofile(AjPTextin* pthis);
void         ajTextinClear(AjPTextin thys);
void         ajTextinClearNewfile(AjPTextin thys);
void         ajTextinClearNewinput(AjPTextin thys);
void         ajTextinClearNofile(AjPTextin thys);
ajuint       ajTextinGetCount(const AjPTextin thys);
ajlong       ajTextinGetFpos(const AjPTextin thys);
const AjPStr ajTextinGetQryS(const AjPTextin thys);
ajuint       ajTextinGetRecords(const AjPTextin thys);
ajuint       ajTextinGetTotrecords(const AjPTextin thys);
void         ajTextinQryC(AjPTextin thys, const char* txt);
void         ajTextinQryS(AjPTextin thys, const AjPStr str);
void         ajTextinStoreClear(AjPTextin thys,
                           ajint lines, const AjPStr rdline,
                           AjPStr *astr);
AjBool       ajTextinStoreReadline(AjPTextin thys,
                                   AjPStr* pdest, AjPStr *astr);
void         ajTextinStoreReset(AjPTextin thys, AjPStr *astr);
void         ajTextinTrace(const AjPTextin thys);
const char*  ajTextinTypeGetFields(void);
const char*  ajTextinTypeGetQlinks(void);

AjPTable     ajTextaccessGetDb(void);
const char*  ajTextaccessMethodGetQlinks(const AjPStr method);
ajuint       ajTextaccessMethodGetScope(const AjPStr method);
AjBool       ajTextaccessMethodTest(const AjPStr method);

AjBool       ajTextinAccessAsis(AjPTextin textin);
AjBool       ajTextinAccessFile(AjPTextin textin);
AjBool       ajTextinAccessFtp(AjPTextin textin);
AjBool       ajTextinAccessHttp(AjPTextin textin);
AjBool       ajTextinAccessOffset(AjPTextin textin);
AjBool       ajTextinformatTerm(const AjPStr term);
AjBool       ajTextinformatTest(const AjPStr format);
AjBool       ajTextinRead(AjPTextin textin, AjPText text);

void         ajTextinprintBook(AjPFile outf);
void         ajTextinprintHtml(AjPFile outf);
void         ajTextinprintText(AjPFile outf, AjBool full);
void         ajTextinprintWiki(AjPFile outf);

AjBool       ajTextallNext(AjPTextall thys, AjPText *Ptext);

void         ajTextallClear(AjPTextall thys);
void         ajTextallDel(AjPTextall* pthis);
AjPTextall   ajTextallNew(void);

void         ajTextinExit(void);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !AJTEXTREAD_H */

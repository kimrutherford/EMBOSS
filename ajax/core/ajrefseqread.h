/* @include ajrefseqread ******************************************************
**
** AJAX reference sequence reading functions
**
** These functions control all aspects of AJAX refseqdec reading
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.4 $
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

#ifndef AJREFSEQREAD_H
#define AJREFSEQREAD_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajrefseqdata.h"

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

void             ajRefseqallClear(AjPRefseqall thys);
void             ajRefseqallDel(AjPRefseqall* pthis);
AjPRefseqall     ajRefseqallNew(void);
const AjPStr     ajRefseqallGetrefseqId(const AjPRefseqall thys);

void             ajRefseqinClear(AjPRefseqin thys);
void             ajRefseqinDel(AjPRefseqin* pthis);
AjPRefseqin      ajRefseqinNew(void);
void             ajRefseqinQryC(AjPRefseqin thys, const char* txt);
void             ajRefseqinQryS(AjPRefseqin thys, const AjPStr str);
void             ajRefseqinTrace(const AjPRefseqin thys);

void             ajRefseqinprintBook(AjPFile outf);
void             ajRefseqinprintHtml(AjPFile outf);
void             ajRefseqinprintText(AjPFile outf, AjBool full);
void             ajRefseqinprintWiki(AjPFile outf);

AjBool           ajRefseqallNext(AjPRefseqall thys, AjPRefseq *Prefseq);
AjBool           ajRefseqinRead(AjPRefseqin refseqin, AjPRefseq thys);

AjBool           ajRefseqinformatTerm(const AjPStr term);
AjBool           ajRefseqinformatTest(const AjPStr format);
void             ajRefseqinExit(void);
const char*      ajRefseqinTypeGetFields(void);
const char*      ajRefseqinTypeGetQlinks(void);

AjPTable         ajRefseqaccessGetDb(void);
const char*      ajRefseqaccessMethodGetQlinks(const AjPStr method);
ajuint           ajRefseqaccessMethodGetScope(const AjPStr method);
AjBool           ajRefseqaccessMethodTest(const AjPStr method);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !AJREFSEQREAD_H */

/* @include ajtax *************************************************************
**
** AJAX taxonomy functions
**
** These functions control all aspects of AJAX taxonomy
** parsing and include simple utilities.
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.12 $
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

#ifndef AJTAX_H
#define AJTAX_H

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

AjPTax       ajTaxNew(void);
AjPTax       ajTaxNewTax(const AjPTax tax);
void         ajTaxDel(AjPTax *Ptax);
void         ajTaxClear(AjPTax tax);
const AjPStr ajTaxGetDb(const AjPTax tax);
const AjPStr ajTaxGetId(const AjPTax tax);
const AjPStr ajTaxGetName(const AjPTax tax);
ajuint       ajTaxGetParent(const AjPTax tax);
const char*  ajTaxGetQryC(const AjPTax tax);
const AjPStr ajTaxGetQryS(const AjPTax tax);
const AjPStr ajTaxGetRank(const AjPTax tax);
ajuint       ajTaxGetTree(const AjPTax tax, AjPList taxlist);
AjBool       ajTaxIsHidden(const AjPTax tax);
AjBool       ajTaxIsSpecies(const AjPTax tax);

AjPTaxcit    ajTaxcitNew(void);
void         ajTaxcitDel(AjPTaxcit *Pcit);

AjPTaxcode   ajTaxcodeNew(void);

AjPTaxdel    ajTaxdelNew(void);

AjPTaxdiv    ajTaxdivNew(void);

AjPTaxmerge  ajTaxmergeNew(void);

AjPTaxname   ajTaxnameNew(void);
AjPTaxname   ajTaxnameNewName(const AjPTaxname name);
void         ajTaxnameDel(AjPTaxname *Pname);

void         ajTaxExit(void);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !AJTAX_H */

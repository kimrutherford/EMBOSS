/* @include ajquery ***********************************************************
**
** AJAX Query parsing functions
**
** @author Copyright (C) 2011 Peter Rice
** @version $Revision: 1.9 $
** @modified Jul 15 pmr First version with code from all datatypes merged
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

#ifndef AJQUERY_H
#define AJQUERY_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajstr.h"
#include "ajlist.h"
#include "ajquerydata.h"
#include "ajtextdata.h"

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

AjPQuery      ajQueryNew(AjEDataType datatype);
void          ajQueryDel(AjPQuery* pthis);
void          ajQueryTrace(const AjPQuery thys);
void          ajQueryClear(AjPQuery thys);
const AjPStr  ajQuerystrParseFormat(AjPStr *Pqry, AjPTextin textin,
                                    AjBool findformat(const AjPStr format,
                                                      ajint *iformat));
AjBool        ajQuerystrParseListfile(AjPStr *Pqry);
AjBool        ajQuerystrParseRange(AjPStr *Pqry,
                                   ajint *Pbegin, ajint *Pend, AjBool *Prev);
AjBool        ajQuerystrParseRead(AjPStr *Pqry,
                                  AjPTextin textin,
                                  AjBool findformat(const AjPStr format,
                                                    ajint *iformat),
                                  AjBool *Pnontext);
void          ajQueryStarclear(AjPQuery thys);
AjBool        ajQueryKnownFieldC(const AjPQuery qry, const char* fieldtxt);
AjBool        ajQueryKnownFieldS(const AjPQuery qry, const AjPStr field);
AjBool        ajQueryAddFieldAndC(AjPQuery qry, const char* fieldtxt,
                                  const char* wildquerytxt);
AjBool        ajQueryAddFieldAndS(AjPQuery qry, const AjPStr field,
                                  const AjPStr wildquery);
AjBool        ajQueryAddFieldElseC(AjPQuery qry, const char* fieldtxt,
                                   const char* wildquerytxt);
AjBool        ajQueryAddFieldElseS(AjPQuery qry, const AjPStr field,
                                   const AjPStr wildquery);
AjBool        ajQueryAddFieldEorC(AjPQuery qry, const char* fieldtxt,
                                  const char* wildquerytxt);
AjBool        ajQueryAddFieldEorS(AjPQuery qry, const AjPStr field,
                                  const AjPStr wildquery);
AjBool        ajQueryAddFieldNotC(AjPQuery qry, const char* fieldtxt,
                                  const char* wildquerytxt);
AjBool        ajQueryAddFieldNotS(AjPQuery qry, const AjPStr field,
                                  const AjPStr wildquery);
AjBool        ajQueryAddFieldOrC(AjPQuery qry, const char* fieldtxt,
                                 const char* wildquerytxt);
AjBool        ajQueryAddFieldOrS(AjPQuery qry, const AjPStr field,
                                 const AjPStr wildquery);
const AjPList ajQueryGetallFields(const AjPQuery query);
const char*   ajQueryGetDatatype(const AjPQuery query);
const AjPStr  ajQueryGetFormat(const AjPQuery query);
const AjPStr  ajQueryGetId(const AjPQuery query);
AjBool        ajQueryGetQuery(const AjPQuery query, AjPStr* Pdest);
AjBool        ajQueryIsSet(const AjPQuery thys);
AjBool        ajQuerySetWild(AjPQuery thys);

AjPQueryField ajQueryfieldNewC(const char* fieldtxt,
                               const char* wildtxt,
                               AjEQryLink oper);
AjPQueryField ajQueryfieldNewS(const AjPStr field,
                               const AjPStr wild,
                               AjEQryLink oper);
void          ajQueryfieldDel(AjPQueryField *Pthis);
void          ajQuerylistTrace(const AjPList list);

void          ajQueryExit(void);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !AJQUERY_H */

/* @include ajobo *************************************************************
**
** AJAX OBO handling functions
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.21 $
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

#ifndef AJOBO_H
#define AJOBO_H

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

AjPObo           ajOboNew(void);
AjPObo           ajOboNewObo(const AjPObo obo);

void             ajOboClear(AjPObo thys);
void             ajOboDel(AjPObo *Pobo);

AjPObotag        ajObotagNewData(const AjPStr name, const AjPStr value,
                                 const AjPStr modifier, const AjPStr comment,
                                 ajuint linenum);
AjPObotag        ajObotagNewTag(const AjPObotag tag);
void             ajObotagDel(AjPObotag* Ptag);

AjPOboxref       ajOboxrefNewData(const AjPStr name, const AjPStr desc);
AjPOboxref       ajOboxrefNewXref(const AjPOboxref xref);
void             ajOboxrefDel(AjPOboxref* Pxref);

AjBool           ajOboqryGetObo(const AjPStr thys, AjPObo obo);

const AjPStr     ajOboGetDb(const AjPObo obo);
AjBool           ajOboGetDef(const AjPObo obo, AjPStr *Pdefstr, ajuint *nrefs);
const AjPStr     ajOboGetEntry(const AjPObo obo);
const AjPStr     ajOboGetId(const AjPObo obo);
const AjPStr     ajOboGetName(const AjPObo obo);
const AjPStr     ajOboGetNamespace(const AjPObo obo);
ajuint           ajOboGetParents(const AjPObo obo, AjPList uplist);
const char*      ajOboGetQryC(const AjPObo obo);
const AjPStr     ajOboGetQryS(const AjPObo obo);
const AjPStr     ajOboGetReplaced(const AjPObo obo);
ajuint           ajOboGetTree(const AjPObo obo, AjPList obolist);
AjBool           ajOboIsObsolete(const AjPObo obo);

void             ajOboExit(void);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !AJOBO_H */

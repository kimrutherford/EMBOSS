/* @include ajassem ***********************************************************
**
** AJAX assembly functions
**
** These functions control all aspects of AJAX assembly
** parsing and include simple utilities.
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.12 $
** @modified Oct 5 pmr First version
** @modified $Date: 2012/03/12 17:42:29 $ by $Author: rice $
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

#ifndef AJASSEM_H
#define AJASSEM_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajstr.h"
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

AjPAssem         ajAssemNew(void);
void             ajAssemDel(AjPAssem *Passem);
void             ajAssemClear(AjPAssem assem);
void             ajAssemReset(AjPAssem assem);
const AjPStr     ajAssemGetId(const AjPAssem assem);
const char*      ajAssemGetQryC(const AjPAssem assem);
const AjPStr     ajAssemGetQryS(const AjPAssem assem);
void             ajAssemExit(void);


const char*      ajAssemreadgroupGetPlatformname(const AjPAssemReadgroup rg);
AjEAssemPlatform ajAssemplatformGetType(const char* platfname);

const char*       ajAssemGetSortorderC(const AjPAssem assem);
AjEAssemSortOrder ajAssemsortorderGetType(const char* name);

void             ajAssemSetContigattributes(AjPAssem assem,
                                            const AjPTable tags);
void             ajAssemSetReadgroups(AjPAssem assem,
                                      const AjPTable tags);

void    ajAssemreadDel(AjPAssemRead *Passemread);
ajint   ajAssemreadGetAlignmentend(const AjPAssemRead r);
ajint   ajAssemreadGetAlignmentstart(const AjPAssemRead r);
AjBool  ajAssemreadGetFlagFirstofpair(const AjPAssemRead r);
AjBool  ajAssemreadGetFlagNegativestrand(const AjPAssemRead r);
AjBool  ajAssemreadGetFlagPaired(const AjPAssemRead r);
AjBool  ajAssemreadGetFlagUnmapped(const AjPAssemRead r);
AjPList ajAssemreadSetAlignblocks(AjPAssemRead read);


/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !AJASSEM_H */

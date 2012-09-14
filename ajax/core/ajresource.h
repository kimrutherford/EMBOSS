/* @include ajresource ********************************************************
**
** AJAX data resource functions
**
** These functions control all aspects of AJAX data resource
** parsing and include simple utilities.
**
** @author Copyright (C) 2010 Peter Rice
** @version  $Revision: 1.18 $
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

#ifndef AJRESOURCE_H
#define AJRESOURCE_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajresourcedata.h"

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

AjPResource  ajResourceNew(void);
AjPResource  ajResourceNewDrcat(const AjPStr dbname);
AjPResource  ajResourceNewResource(const AjPResource res);
void         ajResourceDel(AjPResource *Presource);
void         ajResourceClear(AjPResource resource);
AjBool       ajResourceGetDbdata(const AjPResource resource, AjPQuery qry,
                                 AjBool findformat(const AjPStr format,
                                                   ajint *iformat));
const AjPStr ajResourceGetEntry(const AjPResource resource);
const AjPStr ajResourceGetId(const AjPResource resource);
const char*  ajResourceGetQryC(const AjPResource resource);
const AjPStr ajResourceGetQryS(const AjPResource resource);
void         ajResourceExit(void);
void         ajResourceTrace(const AjPResource thys);

AjPReslink   ajReslinkNew(void);
AjPReslink   ajReslinkNewReslink(const AjPReslink link);
void         ajReslinkDel(AjPReslink*);
AjBool       ajReslinklistClone(const AjPList src, AjPList dest);

AjPResquery  ajResqueryNew(void);
AjPResquery  ajResqueryNewResquery(const AjPResquery qry);
void         ajResqueryDel(AjPResquery*);
AjBool       ajResquerylistClone(const AjPList src, AjPList dest);

AjPResterm   ajRestermNew(void);
AjPResterm   ajRestermNewResterm(const AjPResterm term);
void         ajRestermDel(AjPResterm*);
AjBool       ajRestermlistClone(const AjPList src, AjPList dest);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !AJRESOURCE_H */

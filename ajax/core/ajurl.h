/* @include ajurl *************************************************************
**
** AJAX url functions
**
** These functions control all aspects of AJAX url
** parsing and include simple utilities.
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.4 $
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

#ifndef AJURL_H
#define AJURL_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajurldata.h"

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

AjPUrl       ajUrlNew(void);
void         ajUrlDel(AjPUrl *Purl);
void         ajUrlClear(AjPUrl url);
const AjPStr ajUrlGetDb(const AjPUrl url);
const AjPStr ajUrlGetId(const AjPUrl url);
const char*  ajUrlGetQryC(const AjPUrl url);
const AjPStr ajUrlGetQryS(const AjPUrl url);
void         ajUrlExit(void);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !AJURL_H */

/* @include ensmetacoordinate *************************************************
**
** Ensembl Meta-Coordinate functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.14 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/02/04 10:30:23 $ by $Author: mks $
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

#ifndef ENSMETACOORDINATE_H
#define ENSMETACOORDINATE_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "enscoordsystem.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */

/* @data EnsPMetacoordinateadaptor ********************************************
**
** Ensembl Meta-Coordinate Adaptor.
**
** @alias EnsSMetacoordinateadaptor
** @alias EnsOMetacoordinateadaptor
**
** @attr Adaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor.
** @attr CacheByName [AjPTable] Ensembl Feature name (i.e. table name) cache.
** @attr CacheByLength [AjPTable] Maximum length cache.
** @@
******************************************************************************/

typedef struct EnsSMetacoordinateadaptor
{
    EnsPDatabaseadaptor Adaptor;
    AjPTable CacheByName;
    AjPTable CacheByLength;
} EnsOMetacoordinateadaptor;

#define EnsPMetacoordinateadaptor EnsOMetacoordinateadaptor*




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */

/*
** Prototype definitions
*/

/* Ensembl Meta-Coordinate Adaptor */

EnsPMetacoordinateadaptor ensRegistryGetMetacoordinateadaptor(
    EnsPDatabaseadaptor dba);

EnsPMetacoordinateadaptor ensMetacoordinateadaptorNew(
    EnsPDatabaseadaptor dba);

void ensMetacoordinateadaptorDel(EnsPMetacoordinateadaptor *Pmca);

AjBool ensMetacoordinateadaptorFetchAllCoordsystems(
    const EnsPMetacoordinateadaptor mca,
    const AjPStr name,
    AjPList css);

ajint ensMetacoordinateadaptorGetMaximumlength(
    const EnsPMetacoordinateadaptor mca,
    const EnsPCoordsystem cs,
    const AjPStr name);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSMETACOORDINATE_H */

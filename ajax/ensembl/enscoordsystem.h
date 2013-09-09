/* @include enscoordsystem ****************************************************
**
** Ensembl Coordinate System functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.22 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2013/02/17 13:04:02 $ by $Author: mks $
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

#ifndef ENSCOORDSYSTEM_H
#define ENSCOORDSYSTEM_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ensdatabaseadaptor.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */

struct EnsSCoordsystem;




/* @data EnsPCoordsystemadaptor ***********************************************
**
** Ensembl Coordinate System Adaptor
**
** @alias EnsSCoordsystemadaptor
** @alias EnsOCoordsystemadaptor
**
** @cc Bio::EnsEMBL::DBSQL::BaseAdaptor
** @attr Adaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** cc Bio::EnsEMBL::DBSQL::CoordSystemAdaptor
** @attr CacheByIdentifier [AjPTable] Database identifier cache
** @attr CacheByName [AjPTable] Name cache
** @attr CacheByRank [AjPTable] Rank cache
** @attr CacheByDefault [AjPTable] Default Ensembl Coordinate System objects
** @attr MappingPaths [AjPTable] Mapping paths between coordinate systems
** @attr ExternalToInternal [AjPTable]
** AJAX Table of AJAX unsigned integer
** (external Ensembl Sequence Region identifier) key data and
** AJAX unsigned integer
** (internal Ensembl Sequence Region identifier) value data
** @attr InternalToExternal [AjPTable]
** AJAX Table of AJAX unsigned integer
** (internal Ensembl Sequence Region identifier) key data and
** AJAX unsigned integer
** (external Ensembl Sequence Region identifier) value data
** @attr Seqlevel [struct EnsSCoordsystem*]
** Sequence-level Ensembl Coordinate System
** @attr Toplevel [struct EnsSCoordsystem*]
** Top-level Ensembl Coordinate System
** @@
******************************************************************************/

typedef struct EnsSCoordsystemadaptor
{
    EnsPDatabaseadaptor Adaptor;
    AjPTable CacheByIdentifier;
    AjPTable CacheByName;
    AjPTable CacheByRank;
    AjPTable CacheByDefault;
    AjPTable MappingPaths;
    AjPTable ExternalToInternal;
    AjPTable InternalToExternal;
    struct EnsSCoordsystem *Seqlevel;
    struct EnsSCoordsystem *Toplevel;
} EnsOCoordsystemadaptor;

#define EnsPCoordsystemadaptor EnsOCoordsystemadaptor*




/* @data EnsPCoordsystem ******************************************************
**
** Ensembl Coordinate System
**
** @alias EnsSCoordsystem
** @alias EnsOCoordsystem
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] Internal SQL database identifier (primary key)
** @attr Adaptor [EnsPCoordsystemadaptor] Ensembl Coordinate System Adaptor
** @cc Bio::EnsEMBL::CoordSystem
** @attr Name [AjPStr] Coordinate System name
** @attr Version [AjPStr] Coordinate System version
** @attr Default [AjBool] Default Coordinate System version of this name
** @attr SequenceLevel [AjBool] Sequence-level attribute
** @attr Toplevel [AjBool] Top-level attribute
** @attr Rank [ajuint] Coordinate System rank
** @@
******************************************************************************/

typedef struct EnsSCoordsystem
{
    ajuint Use;
    ajuint Identifier;
    EnsPCoordsystemadaptor Adaptor;
    AjPStr Name;
    AjPStr Version;
    AjBool Default;
    AjBool SequenceLevel;
    AjBool Toplevel;
    ajuint Rank;
} EnsOCoordsystem;

#define EnsPCoordsystem EnsOCoordsystem*




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */

/*
** Prototype definitions
*/

/* Ensembl Coordinate System */

EnsPCoordsystem ensCoordsystemNewCpy(const EnsPCoordsystem cs);

EnsPCoordsystem ensCoordsystemNewIni(EnsPCoordsystemadaptor csa,
                                     ajuint identifier,
                                     AjPStr name,
                                     AjPStr version,
                                     ajuint rank,
                                     AjBool deflt,
                                     AjBool toplvl,
                                     AjBool seqlvl);

EnsPCoordsystem ensCoordsystemNewRef(EnsPCoordsystem cs);

void ensCoordsystemDel(EnsPCoordsystem *Pcs);

EnsPCoordsystemadaptor ensCoordsystemGetAdaptor(const EnsPCoordsystem cs);

AjBool ensCoordsystemGetDefault(const EnsPCoordsystem cs);

ajuint ensCoordsystemGetIdentifier(const EnsPCoordsystem cs);

const AjPStr ensCoordsystemGetName(const EnsPCoordsystem cs);

ajuint ensCoordsystemGetRank(const EnsPCoordsystem cs);

AjBool ensCoordsystemGetSeqlevel(const EnsPCoordsystem cs);

AjBool ensCoordsystemGetToplevel(const EnsPCoordsystem cs);

const AjPStr ensCoordsystemGetVersion(const EnsPCoordsystem cs);

AjBool ensCoordsystemSetAdaptor(EnsPCoordsystem cs,
                                EnsPCoordsystemadaptor csa);

AjBool ensCoordsystemSetIdentifier(EnsPCoordsystem cs, ajuint identifier);

AjBool ensCoordsystemTrace(const EnsPCoordsystem cs, ajuint level);

size_t ensCoordsystemCalculateMemsize(const EnsPCoordsystem cs);

AjBool ensCoordsystemMatch(const EnsPCoordsystem cs1,
                           const EnsPCoordsystem cs2);

AjPStr ensCoordsystemGetSpecies(const EnsPCoordsystem cs);

AjBool ensListCoordsystemSortIdentifierAscending(AjPList css);

AjBool ensListCoordsystemSortIdentifierDescending(AjPList css);

AjBool ensListCoordsystemSortRankAscending(AjPList css);

AjBool ensListCoordsystemSortRankDescending(AjPList css);

AjBool ensListCoordsystemTrace(const AjPList css, ajuint level);

/* Ensembl Coordinate System Adaptor */

EnsPCoordsystemadaptor ensRegistryGetCoordsystemadaptor(
    EnsPDatabaseadaptor dba);

EnsPCoordsystemadaptor ensCoordsystemadaptorNew(
    EnsPDatabaseadaptor dba);

void ensCoordsystemadaptorDel(EnsPCoordsystemadaptor *Pcsa);

EnsPDatabaseadaptor ensCoordsystemadaptorGetDatabaseadaptor(
    EnsPCoordsystemadaptor csa);

AjBool ensCoordsystemadaptorFetchAll(
    const EnsPCoordsystemadaptor csa,
    AjPList css);

AjBool ensCoordsystemadaptorFetchAllbyName(
    const EnsPCoordsystemadaptor csa,
    const AjPStr name,
    AjPList css);

AjBool ensCoordsystemadaptorFetchByIdentifier(
    const EnsPCoordsystemadaptor csa,
    ajuint identifier,
    EnsPCoordsystem *Pcs);

AjBool ensCoordsystemadaptorFetchByName(
    const EnsPCoordsystemadaptor csa,
    const AjPStr name,
    const AjPStr version,
    EnsPCoordsystem *Pcs);

AjBool ensCoordsystemadaptorFetchByRank(
    const EnsPCoordsystemadaptor csa,
    ajuint rank,
    EnsPCoordsystem *Pcs);

AjBool ensCoordsystemadaptorFetchSeqlevel(
    const EnsPCoordsystemadaptor csa,
    EnsPCoordsystem *Pcs);

AjBool ensCoordsystemadaptorFetchToplevel(
    const EnsPCoordsystemadaptor csa,
    EnsPCoordsystem *Pcs);

const AjPList ensCoordsystemadaptorGetMappingpath(
    const EnsPCoordsystemadaptor csa,
    EnsPCoordsystem cs1,
    EnsPCoordsystem cs2);

ajuint ensCoordsystemadaptorGetSeqregionidentifierExternal(
    const EnsPCoordsystemadaptor csa,
    ajuint srid);

ajuint ensCoordsystemadaptorGetSeqregionidentifierInternal(
    const EnsPCoordsystemadaptor csa,
    ajuint srid);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSCOORDSYSTEM_H */

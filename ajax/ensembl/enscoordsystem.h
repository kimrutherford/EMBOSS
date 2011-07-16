
#ifndef ENSCOORDSYSTEM_H
#define ENSCOORDSYSTEM_H

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ensdatabaseadaptor.h"

AJ_BEGIN_DECLS




/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */




/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */

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
** @attr CacheByDefault [AjPTable] Default Ensembl Coordinate Systems
** @attr MappingPaths [AjPTable] Mapping paths between coordinate systems
** @attr ExternalToInternal [AjPTable] External to internal Sequence Regions
** @attr InternalToExternal [AjPTable] Internal to external Sequence Regions
** @attr Seqlevel [void*] Sequence-level Ensembl Coordinate System
** @attr Toplevel [void*] Top-level Ensembl Coordinate System
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
    void* Seqlevel;
    void* Toplevel;
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




/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */

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

void ensCoordsystemDel(EnsPCoordsystem* Pcs);

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

AjBool ensListCoordsystemTrace(const AjPList css, ajuint level);

/* Ensembl Coordinate System Adaptor */

EnsPCoordsystemadaptor ensRegistryGetCoordsystemadaptor(
    EnsPDatabaseadaptor dba);

EnsPCoordsystemadaptor ensCoordsystemadaptorNew(
    EnsPDatabaseadaptor dba);

void ensCoordsystemadaptorDel(EnsPCoordsystemadaptor* Pcsa);

EnsPDatabaseadaptor ensCoordsystemadaptorGetDatabaseadaptor(
    const EnsPCoordsystemadaptor csa);

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
    EnsPCoordsystem* Pcs);

AjBool ensCoordsystemadaptorFetchByName(
    const EnsPCoordsystemadaptor csa,
    const AjPStr name,
    const AjPStr version,
    EnsPCoordsystem* Pcs);

AjBool ensCoordsystemadaptorFetchByRank(
    const EnsPCoordsystemadaptor csa,
    ajuint rank,
    EnsPCoordsystem* Pcs);

AjBool ensCoordsystemadaptorFetchSeqlevel(
    const EnsPCoordsystemadaptor csa,
    EnsPCoordsystem* Pcs);

AjBool ensCoordsystemadaptorFetchToplevel(
    const EnsPCoordsystemadaptor csa,
    EnsPCoordsystem* Pcs);

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

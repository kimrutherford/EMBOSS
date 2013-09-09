/* @include ensmapper *********************************************************
**
** Ensembl Mapper functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.27 $
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

#ifndef ENSMAPPER_H
#define ENSMAPPER_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "enscoordsystem.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */

/* @enum EnsEMapperunitType ***************************************************
**
** Ensembl Mapper Unit Type enumeration
**
** @value ensEMapperunitTypeNULL Null
** @value ensEMapperunitTypeSource Source
** @value ensEMapperunitTypeTarget Target
** @@
******************************************************************************/

typedef enum EnsOMapperunitType
{
    ensEMapperunitTypeNULL,
    ensEMapperunitTypeSource,
    ensEMapperunitTypeTarget
} EnsEMapperunitType;




/* @enum EnsEMapperresultType *************************************************
**
** Ensembl Mapper Result Type enumeration
**
** @value ensEMapperresultTypeNULL Null
** @value ensEMapperresultTypeCoordinate Coordinate
** @value ensEMapperresultTypeGap Gap
** @value ensEMapperresultTypeInDel Insertion or Deletion
** @@
******************************************************************************/

typedef enum EnsOMapperresultType
{
    ensEMapperresultTypeNULL,
    ensEMapperresultTypeCoordinate,
    ensEMapperresultTypeGap,
    ensEMapperresultTypeInDel
} EnsEMapperresultType;




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */

/* @data EnsPMapperunit *******************************************************
**
** Ensembl Mapper Unit.
**
** One unit of an Ensembl Mapper Pair.
**
** @alias EnsSMapperunit
** @alias EnsOMapperunit
**
** @cc Bio::EnsEMBL::Mapper::Unit
** @attr Objectidentifier [ajuint] Ensembl Object identifier
** @attr Start [ajint] Start coordinate
** @attr End [ajint] End coordinate
** @attr Use [ajuint] Use counter
** @@
******************************************************************************/

typedef struct EnsSMapperunit
{
    ajuint Objectidentifier;
    ajint Start;
    ajint End;
    ajuint Use;
} EnsOMapperunit;

#define EnsPMapperunit EnsOMapperunit*




/* @data EnsPMapperpair *******************************************************
**
** Ensembl Mapper Pair.
**
** An Ensembl Mapper Pair consists of two Ensembl Mapper Unit objects.
**
** @alias EnsSMapperpair
** @alias EnsOMapperpair
**
** @cc Bio::EnsEMBL::Mapper::Pair
** @attr Source [EnsPMapperunit] Source Ensembl Mapper Unit
** @attr Target [EnsPMapperunit] Target Ensembl Mapper Unit
** @attr Orientation [ajint]
** Relative orientation of the Ensembl Mapper Unit objects
** @cc Bio::EnsEMBL::Mapper::IndelPair
** @attr Indel [AjBool] Insertion or deletion attribute
** @attr Use [ajuint] Use counter
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSMapperpair
{
    EnsPMapperunit Source;
    EnsPMapperunit Target;
    ajint Orientation;
    AjBool Indel;
    ajuint Use;
    ajuint Padding;
} EnsOMapperpair;

#define EnsPMapperpair EnsOMapperpair*




/* @data EnsPMapperresult *****************************************************
**
** Ensembl Mapper Result.
**
** Representation of a mapped Ensembl Object returned by an Ensembl Mapper when
** the source region maps to valid sequence.
** Depending on the Ensembl Mapper Result type member,
** this can also represent a gap.
**
** @alias EnsSMapperresult
** @alias EnsOMapperresult
**
** @attr Coordsystem [EnsPCoordsystem] Ensembl Coordinate System
** @attr Type [EnsEMapperresultType] Result type
** @cc Bio::EnsEMBL::Mapper::Coordinate
** @cc Bio::EnsEMBL::Mapper::IndelCoordinate
** @attr Objectidentifier [ajuint] Ensembl Object identifier
** @attr CoordinateStart [ajint] Coordinate start
** @attr CoordinateEnd [ajint] Coordinate end
** @attr CoordinateStrand [ajint] Coordinate strand
** @cc Bio::EnsEMBL::Mapper::Gap
** @cc Bio::EnsEMBL::Mapper::IndelCoordinate
** @attr GapStart [ajint] Start coordinate of a gap
** @attr GapEnd [ajint] End coordinate of a gap
** @attr Rank [ajuint] Rank
** @attr Use [ajuint] Use counter
** @attr Padding [ajuint] Padding to alignment boundary
** @@
**
** NOTE: This object subsumes the following Perl objects returned by the
** Bio::EnsEMBL::Mapper class.
**
** Bio::EnsEMBL::Mapper::Coordinate
** Bio::EnsEMBL::Mapper::Gap
** Bio::EnsEMBL::Mapper::IndelCoordinate
**
** The objects can be distinguished by their Ensembl Mapper Result type member.
******************************************************************************/

typedef struct EnsSMapperresult
{
    EnsPCoordsystem Coordsystem;
    EnsEMapperresultType Type;
    ajuint Objectidentifier;
    ajint CoordinateStart;
    ajint CoordinateEnd;
    ajint CoordinateStrand;
    ajint GapStart;
    ajint GapEnd;
    ajuint Rank;
    ajuint Use;
    ajuint Padding;
} EnsOMapperresult;

#define EnsPMapperresult EnsOMapperresult*




/* @data EnsPMapperrange ******************************************************
**
** Ensembl Mapper Range.
**
** An Ensembl Mapper Range defines start and end ccordinates of genome ranges.
**
** @alias EnsSMapperrange
** @alias EnsOMapperrange
**
** @attr Start [ajint] Start coordinate
** @attr End [ajint] End coordinate
** @attr Use [ajuint] Use counter
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSMapperrange
{
    ajint Start;
    ajint End;
    ajuint Use;
    ajuint Padding;
} EnsOMapperrange;

#define EnsPMapperrange EnsOMapperrange*




/* @data EnsPMapperrangeregistry **********************************************
**
** Ensembl Mapper Range Registry.
**
** An Ensembl Mapper Range Registry maintains a table of registered regions.
**
** @alias EnsSMapperrangeregistry
** @alias EnsOMapperrangeregistry
**
** @attr Registry [AjPTable]
** First-level AJAX Table of AJAX unsigned integer
** (Ensembl Object identifier) key data and
** second-level AJAX List value data of Ensembl Mapper Range objects.
** @attr Use [ajuint] Use counter
** @attr Padding [ajuint] Padding to alignment boundary
** @@
** The Ensembl Mapper Range Registry maintains an internal list of registered
** regions and is used to quickly ascertain if and what regions of a provided
** range need registration.
******************************************************************************/

typedef struct EnsSMapperrangeregistry
{
    AjPTable Registry;
    ajuint Use;
    ajuint Padding;
} EnsOMapperrangeregistry;

#define EnsPMapperrangeregistry EnsOMapperrangeregistry*




/* @data EnsPMapper ***********************************************************
**
** Ensembl Mapper.
**
** Generic mapper to provide coordinate transforms between two
** disjoint Ensembl Coordinate System objects.
**
** @alias EnsSMapper
** @alias EnsOMapper
**
** @cc Bio::EnsEMBL::Mapper
** @attr TypeSource [AjPStr] Source type
** @attr TypeTarget [AjPStr] Target type
** @attr CoordsystemSource [EnsPCoordsystem] Source Ensembl Coordinate System
** @attr CoordsystemTarget [EnsPCoordsystem] Target Ensembl Coordinate System
** @attr Mapperpairs [AjPTable] AJAX Table of AJAX Table objects with
** Ensembl Mapper Pair objects
** @attr Sorted [AjBool] Ensembl Mapper Pair objects are sorted
** @attr Count [ajuint] Number of Ensembl Mapper Pair objects
** @attr Use [ajuint] Use counter
** @attr Padding [ajuint] Padding to alignment boundary
** @@
** The AJAX Table Mapperpairs forms the top hierarchy of an Ensembl Mapper Pair
** cache. The AJAX Table uses the contents of the TypeSource and TypeTarget
** strings as index and holds a second hierarchy of AJAX Table objects, which
** use AJAX unsigned inetger (Ensembl Object identifier) objects as index.
** Those second-level AJAX Table objects then hold a
** third-level of AJAX List objects of Ensembl Mapper Pair objects.
******************************************************************************/

typedef struct EnsSMapper
{
    AjPStr TypeSource;
    AjPStr TypeTarget;
    EnsPCoordsystem CoordsystemSource;
    EnsPCoordsystem CoordsystemTarget;
    AjPTable Mapperpairs;
    AjBool Sorted;
    ajuint Count;
    ajuint Use;
    ajuint Padding;
} EnsOMapper;

#define EnsPMapper EnsOMapper*




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */

/*
** Prototype definitions
*/

/* Ensembl Mapper Unit */

EnsPMapperunit ensMapperunitNewCpy(const EnsPMapperunit mu);

EnsPMapperunit ensMapperunitNewIni(ajuint oid, ajint start, ajint end);

EnsPMapperunit ensMapperunitNewRef(EnsPMapperunit mu);

void ensMapperunitDel(EnsPMapperunit *Pmu);

ajint ensMapperunitGetEnd(const EnsPMapperunit mu);

ajuint ensMapperunitGetObjectidentifier(const EnsPMapperunit mu);

ajint ensMapperunitGetStart(const EnsPMapperunit mu);

AjBool ensMapperunitSetEnd(EnsPMapperunit mu, ajint end);

AjBool ensMapperunitSetObjectidentifier(EnsPMapperunit mu, ajuint oid);

AjBool ensMapperunitSetStart(EnsPMapperunit mu, ajint start);

AjBool ensMapperunitTrace(const EnsPMapperunit mu, ajuint level);

size_t ensMapperunitCalculateMemsize(const EnsPMapperunit mu);

/* Ensembl Mapper Pair */

EnsPMapperpair ensMapperpairNewCpy(const EnsPMapperpair mp);

EnsPMapperpair ensMapperpairNewIni(ajuint srcoid,
                                   ajint srcstart,
                                   ajint srcend,
                                   ajuint trgoid,
                                   ajint trgstart,
                                   ajint trgend,
                                   ajint ori,
                                   AjBool indel);

EnsPMapperpair ensMapperpairNewRef(EnsPMapperpair mp);

EnsPMapperpair ensMapperpairNewUnit(EnsPMapperunit source,
                                    EnsPMapperunit target,
                                    ajint ori,
                                    AjBool indel);

void ensMapperpairDel(EnsPMapperpair *Pmp);

AjBool ensMapperpairGetIndel(const EnsPMapperpair mp);

ajint ensMapperpairGetOrientation(const EnsPMapperpair mp);

EnsPMapperunit ensMapperpairGetSource(const EnsPMapperpair mp);

EnsPMapperunit ensMapperpairGetTarget(const EnsPMapperpair mp);

AjBool ensMapperpairTrace(const EnsPMapperpair mp, ajuint level);

EnsPMapperunit ensMapperpairCalculateMapperunit(const EnsPMapperpair mp,
                                                EnsEMapperunitType type);

size_t ensMapperpairCalculateMemsize(const EnsPMapperpair mp);

AjBool ensListMapperpairSortSourceStartAscending(AjPList mps);

AjBool ensListMapperpairSortTargetStartAscending(AjPList mps);

/* Ensembl Mapper Result */

EnsPMapperresult ensMapperresultNewCoordinate(ajuint oid,
                                              ajint crdstart,
                                              ajint crdend,
                                              ajint crdstrand,
                                              EnsPCoordsystem cs,
                                              ajuint rank);

EnsPMapperresult ensMapperresultNewCpy(const EnsPMapperresult mr);

EnsPMapperresult ensMapperresultNewGap(ajint gapstart,
                                       ajint gapend,
                                       ajuint rank);

EnsPMapperresult ensMapperresultNewIndel(ajuint oid,
                                         ajint crdstart,
                                         ajint crdend,
                                         ajint crdstrand,
                                         EnsPCoordsystem cs,
                                         ajint gapstart,
                                         ajint gapend,
                                         ajuint rank);

EnsPMapperresult ensMapperresultNewIni(EnsEMapperresultType type,
                                       ajuint oid,
                                       ajint crdstart,
                                       ajint crdend,
                                       ajint crdstrand,
                                       EnsPCoordsystem cs,
                                       ajint gapstart,
                                       ajint gapend,
                                       ajuint rank);

EnsPMapperresult ensMapperresultNewRef(EnsPMapperresult mr);

void ensMapperresultDel(EnsPMapperresult *Pmr);

EnsPCoordsystem ensMapperresultGetCoordsystem(const EnsPMapperresult mr);

ajint ensMapperresultGetCoordinateEnd(const EnsPMapperresult mr);

ajint ensMapperresultGetCoordinateStart(const EnsPMapperresult mr);

ajint ensMapperresultGetGapEnd(const EnsPMapperresult mr);

ajint ensMapperresultGetGapStart(const EnsPMapperresult mr);

ajuint ensMapperresultGetObjectidentifier(const EnsPMapperresult mr);

ajuint ensMapperresultGetRank(const EnsPMapperresult mr);

ajint ensMapperresultGetCoordinateStrand(const EnsPMapperresult mr);

EnsEMapperresultType ensMapperresultGetType(const EnsPMapperresult mr);

AjBool ensMapperresultSetCoordinateEnd(EnsPMapperresult mr,
                                       ajint crdend);

AjBool ensMapperresultSetCoordinateStart(EnsPMapperresult mr,
                                         ajint crdstart);

AjBool ensMapperresultSetCoordinateStrand(EnsPMapperresult mr,
                                          ajint crdstrand);

AjBool ensMapperresultSetCoordsystem(EnsPMapperresult mr,
                                     EnsPCoordsystem cs);

AjBool ensMapperresultSetGapEnd(EnsPMapperresult mr,
                                ajint gapend);

AjBool ensMapperresultSetGapStart(EnsPMapperresult mr,
                                  ajint gapstart);

AjBool ensMapperresultSetObjectidentifier(EnsPMapperresult mr,
                                          ajuint oid);

AjBool ensMapperresultSetRank(EnsPMapperresult mr,
                              ajuint rank);

AjBool ensMapperresultSetType(EnsPMapperresult mr,
                              EnsEMapperresultType mrt);

AjBool ensMapperresultTrace(const EnsPMapperresult mr, ajuint level);

ajuint ensMapperresultCalculateLengthCoordinate(const EnsPMapperresult mr);

ajuint ensMapperresultCalculateLengthGap(const EnsPMapperresult mr);

ajuint ensMapperresultCalculateLengthResult(const EnsPMapperresult mr);

size_t ensMapperresultCalculateMemsize(const EnsPMapperresult mr);

/* Ensembl Mapper Range */

EnsPMapperrange ensMapperrangeNewCpy(const EnsPMapperrange mr);

EnsPMapperrange ensMapperrangeNewIni(ajint start, ajint end);

EnsPMapperrange ensMapperrangeNewRef(EnsPMapperrange mr);

void ensMapperrangeDel(EnsPMapperrange *Pmr);

ajint ensMapperrangeGetEnd(const EnsPMapperrange mr);

ajint ensMapperrangeGetStart(const EnsPMapperrange mr);

AjBool ensMapperrangeTrace(const EnsPMapperrange mr, ajuint level);

size_t ensMapperrangeCalculateMemsize(const EnsPMapperrange mr);

/* Ensembl Mapper Range Registry */

EnsPMapperrangeregistry ensMapperrangeregistryNew(void);

EnsPMapperrangeregistry ensMapperrangeregistryNewRef(
    EnsPMapperrangeregistry mrr);

AjBool ensMapperrangeregistryClear(EnsPMapperrangeregistry mrr);

void ensMapperrangeregistryDel(EnsPMapperrangeregistry *Pmrr);

AjBool ensMapperrangeregistryCheck(EnsPMapperrangeregistry mrr,
                                   ajuint oid,
                                   ajint chkstart,
                                   ajint chkend,
                                   ajint regstart,
                                   ajint regend,
                                   AjPList ranges);

const AjPList ensMapperrangeregistryCalculateMapperranges(
    const EnsPMapperrangeregistry mrr,
    ajuint oid);

ajuint ensMapperrangeregistryCalculateOverlap(
    const EnsPMapperrangeregistry mrr,
    ajuint oid,
    ajint start,
    ajint end);

/* Ensembl Mapper */

EnsPMapper ensMapperNewIni(AjPStr srctype,
                           AjPStr trgtype,
                           EnsPCoordsystem srccs,
                           EnsPCoordsystem trgcs);

EnsPMapper ensMapperNewRef(EnsPMapper mapper);

AjBool ensMapperClear(EnsPMapper mapper);

void ensMapperDel(EnsPMapper *Pmapper);

EnsPCoordsystem ensMapperGetCoordsystemSource(const EnsPMapper mapper);

EnsPCoordsystem ensMapperGetCoordsystemTarget(const EnsPMapper mapper);

ajuint ensMapperGetCount(const EnsPMapper mapper);

AjBool ensMapperGetSorted(const EnsPMapper mapper);

AjPStr ensMapperGetTypeSource(const EnsPMapper mapper);

AjPStr ensMapperGetTypeTarget(const EnsPMapper mapper);

AjBool ensMapperAddCoordinates(EnsPMapper mapper,
                               ajuint srcoid,
                               ajint srcstart,
                               ajint srcend,
                               ajint ori,
                               ajuint trgoid,
                               ajint trgstart,
                               ajint trgend);

AjBool ensMapperAddIndel(EnsPMapper mapper,
                         ajuint srcoid,
                         ajint srcstart,
                         ajint srcend,
                         ajint ori,
                         ajuint trgoid,
                         ajint trgstart,
                         ajint trgend);

AjBool ensMapperAddMapperpair(EnsPMapper mapper, EnsPMapperpair mp);

AjBool ensMapperAddMappers(EnsPMapper mapper1, EnsPMapper mapper2);

AjBool ensMapperAddMapperunits(EnsPMapper mapper,
                               EnsPMapperunit srcmu,
                               EnsPMapperunit trgmu,
                               ajint ori,
                               AjBool indel);

AjBool ensMapperList(EnsPMapper mapper,
                     ajuint oid,
                     ajint start,
                     ajint end,
                     const AjPStr type,
                     AjPList mps);

AjBool ensMapperTrace(const EnsPMapper mapper, ajuint level);

size_t ensMapperCalculateMemsize(const EnsPMapper mapper);

AjBool ensMapperFastmap(EnsPMapper mapper,
                        ajuint oid,
                        ajint start,
                        ajint end,
                        ajint strand,
                        const AjPStr type,
                        AjPList mrs);

AjBool ensMapperMap(EnsPMapper mapper,
                    ajuint oid,
                    ajint start,
                    ajint end,
                    ajint strand,
                    const AjPStr type,
                    AjPList mrs);

AjBool ensMapperMapindel(EnsPMapper mapper,
                         ajuint oid,
                         ajint start,
                         ajint end,
                         ajint strand,
                         const AjPStr type,
                         AjPList mrs);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSMAPPER_H */

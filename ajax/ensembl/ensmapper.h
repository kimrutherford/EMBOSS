#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ensmapper_h
#define ensmapper_h

#include "enscoordsystem.h"
#include "enstable.h"




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
** @attr ObjectIdentifier [ajuint] Ensembl Object identifier
** @attr Start [ajint] Start coordinate
** @attr End [ajint] End coordinate
** @attr Use [ajuint] Use counter
** @@
******************************************************************************/

typedef struct EnsSMapperunit
{
    ajuint ObjectIdentifier;
    ajint Start;
    ajint End;
    ajuint Use;
} EnsOMapperunit;

#define EnsPMapperunit EnsOMapperunit*




/* EnsEMapperunitType *********************************************************
**
** Ensembl Mapper Unit Type enumeration.
**
******************************************************************************/

typedef enum EnsOMapperunitType
{
    ensEMapperunitTypeNULL,
    ensEMapperunitTypeSource,
    ensEMapperunitTypeTarget
} EnsEMapperunitType;




/* @data EnsPMapperpair *******************************************************
**
** Ensembl Mapper Pair.
**
** An Ensembl Mapper Pair consists of two Ensembl Mapper Units.
**
** @alias EnsSMapperpair
** @alias EnsOMapperpair
**
** @cc Bio::EnsEMBL::Mapper::Pair
** @attr Source [EnsPMapperunit] Source Ensembl Mapper Unit
** @attr Target [EnsPMapperunit] Target Ensembl Mapper Unit
** @attr Orientation [ajint] Relative orientation of the Ensembl Mapper Units
** @cc Bio::EnsEMBL::Mapper::IndelPair
** @attr InsertionDeletion [AjBool] Insertion-deletion attribute
** @attr Use [ajuint] Use counter
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSMapperpair
{
    EnsPMapperunit Source;
    EnsPMapperunit Target;
    ajint Orientation;
    AjBool InsertionDeletion;
    ajuint Use;
    ajuint Padding;
} EnsOMapperpair;

#define EnsPMapperpair EnsOMapperpair*




/* EnsEMapperresultType *******************************************************
**
** Ensembl Mapper Result Type enumeration.
**
******************************************************************************/

typedef enum EnsOMapperresultType
{
    ensEMapperresultNULL,
    ensEMapperresultCoordinate,
    ensEMapperresultGap,
    ensEMapperresultInDel
} EnsEMapperresultType;




/* @data EnsPMapperresult *****************************************************
**
** Ensembl Mapper Result.
**
** Representation of a mapped Ensembl Object returned by an Ensembl Mapper when
** the source region maps to valid sequence. Depending on the type element
** this can also represent a gap.
**
** @alias EnsSMapperresult
** @alias EnsOMapperresult
**
** @attr Coordsystem [EnsPCoordsystem] Ensembl Coordinate System
** @attr Type [EnsEMapperresultType] Result type
** @cc Bio::EnsEMBL::Mapper::Coordinate
** @cc Bio::EnsEMBL::Mapper::IndelCoordinate
** @attr ObjectIdentifier [ajuint] Ensembl Object identifier
** @attr Start [ajint] Start coordinate
** @attr End [ajint] End coordinate
** @attr Strand [ajint] Strand information
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
** The objects can be distinguished by their type element.
******************************************************************************/

typedef struct EnsSMapperresult
{
    EnsPCoordsystem Coordsystem;
    EnsEMapperresultType Type;
    ajuint ObjectIdentifier;
    ajint Start;
    ajint End;
    ajint Strand;
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
** @attr Registry [AjPTable] Registry Table
** @attr Use [ajuint] Use counter
** @attr Padding [ajuint] Padding to alignment boundary
** @@
** The Ensembl Mapper Range Registry maintains an internal list of registered
** regions and is used to quickly ascertain if and what regions of a provided
** range need registration. It is implemented as a first-level AJAX table with
** Ensembl Object identifers as keys and second-level AJAX Lists of
** Ensembl Mapper Ranges as values.
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
** disjoint Ensembl Coordinate Systems.
**
** @alias EnsSMapper
** @alias EnsOMapper
**
** @cc Bio::EnsEMBL::Mapper
** @attr SourceType [AjPStr] Source type
** @attr TargetType [AjPStr] Target type
** @attr SourceCoordsystem [EnsPCoordsystem] Source Ensembl Coordinate System
** @attr TargetCoordsystem [EnsPCoordsystem] Target Ensembl Coordinate System
** @attr Pairs [AjPTable] AJAX Table of AJAX Tables with Ensembl Mapper Pairs
** @attr IsSorted [AjBool] Ensembl Mapper Pairs in the Mapper are sorted
** @attr PairCount [ajuint] Number of Ensembl Mapper Pairs
** @attr Use [ajuint] Use counter
** @attr Padding [ajuint] Padding to alignment boundary
** @@
** The AJAX Table Pairs forms the top hierarchy of an Ensembl Mapper Pair
** cache. The AJAX Table uses the contents of the SourceType and TargetType
** strings as index and holds a second hierarchy of AJAX Tables, which use
** Ensembl Object identifiers as index. Those second-level AJAX Tables then
** hold a third-level of AJAX Lists of Ensembl Mapper Pairs.
******************************************************************************/

typedef struct EnsSMapper
{
    AjPStr SourceType;
    AjPStr TargetType;
    EnsPCoordsystem SourceCoordsystem;
    EnsPCoordsystem TargetCoordsystem;
    AjPTable Pairs;
    AjBool IsSorted;
    ajuint PairCount;
    ajuint Use;
    ajuint Padding;
} EnsOMapper;

#define EnsPMapper EnsOMapper*




/*
** Prototype definitions
*/

/* Ensembl Mapper Unit */

EnsPMapperunit ensMapperunitNew(ajuint oid, ajint start, ajint end);

EnsPMapperunit ensMapperunitNewObj(const EnsPMapperunit object);

EnsPMapperunit ensMapperunitNewRef(EnsPMapperunit mu);

void ensMapperunitDel(EnsPMapperunit* Pmu);

ajuint ensMapperunitGetObjectIdentifier(const EnsPMapperunit mu);

ajint ensMapperunitGetStart(const EnsPMapperunit mu);

ajint ensMapperunitGetEnd(const EnsPMapperunit mu);

AjBool ensMapperunitSetObjectIdentifier(EnsPMapperunit mu, ajuint oid);

AjBool ensMapperunitSetStart(EnsPMapperunit mu, ajint start);

AjBool ensMapperunitSetEnd(EnsPMapperunit mu, ajint end);

ajulong ensMapperunitGetMemsize(const EnsPMapperunit mu);

AjBool ensMapperunitTrace(const EnsPMapperunit mu, ajuint level);

/* Ensembl Mapper Pair */

EnsPMapperpair ensMapperpairNew(EnsPMapperunit source,
                                EnsPMapperunit target,
                                ajint ori,
                                AjBool indel);

EnsPMapperpair ensMapperpairNewC(ajuint srcoid,
                                 ajint srcstart,
                                 ajint srcend,
                                 ajuint trgoid,
                                 ajint trgstart,
                                 ajint trgend,
                                 ajint ori,
                                 AjBool indel);

EnsPMapperpair ensMapperpairNewObj(const EnsPMapperpair object);

EnsPMapperpair ensMapperpairNewRef(EnsPMapperpair mp);

void ensMapperpairDel(EnsPMapperpair* Pmp);

EnsPMapperunit ensMapperpairGetSource(const EnsPMapperpair mp);

EnsPMapperunit ensMapperpairGetTarget(const EnsPMapperpair mp);

ajint ensMapperpairGetOrientation(const EnsPMapperpair mp);

AjBool ensMapperpairGetInsertionDeletion(const EnsPMapperpair mp);

EnsPMapperunit ensMapperpairGetUnit(const EnsPMapperpair mp,
                                    EnsEMapperunitType type);

ajulong ensMapperpairGetMemsize(const EnsPMapperpair mp);

AjBool ensMapperpairTrace(const EnsPMapperpair mp, ajuint level);

AjBool ensMapperpairSortBySourceStartAscending(AjPList mps);

AjBool ensMapperpairSortByTargetStartAscending(AjPList mps);

/* Ensembl Mapper Result */

EnsPMapperresult ensMapperresultNew(EnsEMapperresultType type,
                                    ajuint oid,
                                    ajint start,
                                    ajint end,
                                    ajint strand,
                                    EnsPCoordsystem cs,
                                    ajint gapstart,
                                    ajint gapend,
                                    ajuint rank);

EnsPMapperresult ensMapperresultNewObj(const EnsPMapperresult object);

EnsPMapperresult ensMapperresultNewRef(EnsPMapperresult mr);

void ensMapperresultDel(EnsPMapperresult* Pmr);

EnsEMapperresultType ensMapperresultGetType(const EnsPMapperresult mr);

ajuint ensMapperresultGetObjectIdentifier(const EnsPMapperresult mr);

ajint ensMapperresultGetStart(const EnsPMapperresult mr);

ajint ensMapperresultGetEnd(const EnsPMapperresult mr);

ajint ensMapperresultGetStrand(const EnsPMapperresult mr);

EnsPCoordsystem ensMapperresultGetCoordsystem(const EnsPMapperresult mr);

ajint ensMapperresultGetGapStart(const EnsPMapperresult mr);

ajint ensMapperresultGetGapEnd(const EnsPMapperresult mr);

ajint ensMapperresultGetRank(const EnsPMapperresult mr);

ajuint ensMapperresultGetCoordinateLength(const EnsPMapperresult mr);

ajuint ensMapperresultGetGapLength(const EnsPMapperresult mr);

ajuint ensMapperresultGetLength(const EnsPMapperresult mr);

AjBool ensMapperresultTrace(const EnsPMapperresult mr, ajuint level);

/* Ensembl Mapper Range */

EnsPMapperrange ensMapperrangeNew(ajint start, ajint end);

EnsPMapperrange ensMapperrangeNewObj(const EnsPMapperrange object);

EnsPMapperrange ensMapperrangeNewRef(EnsPMapperrange mr);

void ensMapperrangeDel(EnsPMapperrange* Pmr);

ajint ensMapperrangeGetStart(const EnsPMapperrange mr);

ajint ensMapperrangeGetEnd(const EnsPMapperrange mr);

/* Ensembl Mapper Range Registry */

EnsPMapperrangeregistry ensMapperrangeregistryNew(void);

EnsPMapperrangeregistry ensMapperrangeregistryNewRef(
    EnsPMapperrangeregistry mrr);

AjBool ensMapperrangeregistryClear(EnsPMapperrangeregistry mrr);

void ensMapperrangeregistryDel(EnsPMapperrangeregistry* Pmrr);

AjBool ensMapperrangeregistryCheckAndRegister(EnsPMapperrangeregistry mrr,
                                              ajuint oid,
                                              ajint chkstart,
                                              ajint chkend,
                                              ajint regstart,
                                              ajint regend,
                                              AjPList ranges);

const AjPList ensMapperrangeregistryGetRanges(
    const EnsPMapperrangeregistry mrr,
    ajuint oid);

ajuint ensMapperrangeregistryOverlapSize(const EnsPMapperrangeregistry mrr,
                                         ajuint oid,
                                         ajint start,
                                         ajint end);

/* Ensembl Mapper */

EnsPMapper ensMapperNew(AjPStr srctype,
                        AjPStr trgtype,
                        EnsPCoordsystem srccs,
                        EnsPCoordsystem trgcs);

EnsPMapper ensMapperNewRef(EnsPMapper mapper);

AjBool ensMapperClear(EnsPMapper mapper);

void ensMapperDel(EnsPMapper* Pmapper);

AjPStr ensMapperGetSourceType(const EnsPMapper mapper);

AjPStr ensMapperGetTargetType(const EnsPMapper mapper);

ajuint ensMapperGetPairCount(const EnsPMapper mapper);

AjBool ensMapperIsSorted(const EnsPMapper mapper);

AjBool ensMapperAddCoordinates(EnsPMapper mapper,
                               ajuint srcoid,
                               ajint srcstart,
                               ajint srcend,
                               ajint ori,
                               ajuint trgoid,
                               ajint trgstart,
                               ajint trgend);

AjBool ensMapperAddMapperunits(EnsPMapper mapper,
                               EnsPMapperunit srcmu,
                               EnsPMapperunit trgmu,
                               ajint ori,
                               AjBool indel);

AjBool ensMapperAddMapperpair(EnsPMapper mapper, EnsPMapperpair mp);

AjBool ensMapperListPairs(EnsPMapper mapper,
                          ajuint oid,
                          ajint start,
                          ajint end,
                          const AjPStr type,
                          AjPList mps);

AjBool ensMapperMapCoordinates(EnsPMapper mapper,
                               ajuint oid,
                               ajint start,
                               ajint end,
                               ajint strand,
                               const AjPStr type,
                               AjPList mrs);

AjBool ensMapperFastMap(EnsPMapper mapper,
                        ajuint oid,
                        ajint start,
                        ajint end,
                        ajint strand,
                        const AjPStr type,
                        AjPList mrs);

AjBool ensMapperAddInDelCoordinates(EnsPMapper mapper,
                                    ajuint srcoid,
                                    ajint srcstart,
                                    ajint srcend,
                                    ajint ori,
                                    ajuint trgoid,
                                    ajint trgstart,
                                    ajint trgend);

AjBool ensMapperMapInDel(EnsPMapper mapper,
                         ajuint oid,
                         ajint start,
                         ajint end,
                         ajint strand,
                         const AjPStr type,
                         AjPList mrs);

AjBool ensMapperAddMapper(EnsPMapper mapper1, EnsPMapper mapper2);

ajulong ensMapperGetMemsize(const EnsPMapper mapper);

AjBool ensMapperTrace(const EnsPMapper mapper, ajuint level);

/*
** End of prototype definitions
*/

#define MENSMAPPERINDELPAIRNEW(source, target, ori)     \
ensMapperpairNew(source, target, ori, AJTRUE);

#define MENSMAPPERINDELPAIRDEL(Pmp)             \
ensMapperpairDel(Pmp);

#define MENSMAPPERCOORDINATENEW(oid, start, end, strand, cs, rank)      \
ensMapperresultNew(ensEMapperresultCoordinate, oid, start, end, strand, cs, \
                   0, 0, rank)

#define MENSMAPPERGAPNEW(start, end, rank)                              \
ensMapperresultNew(ensEMapperresultGap, 0, 0, 0, 0, (EnsPCoordsystem) NULL, \
                   start, end, rank)

#define MENSMAPPERINDELNEW(oid, start, end, strand, cs, gstart, gend)   \
ensMapperresultNew(ensEMapperresultInDel, oid, start, end, strand, cs, \
                   gstart, gend, 0)

#endif /* ensmapper_h */

#ifdef __cplusplus
}
#endif

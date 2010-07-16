#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ensmarker_h
#define ensmarker_h

#include "ensfeature.h"




/* @data EnsPMarkersynonym ****************************************************
**
** Ensembl Marker Synonym
**
** @alias EnsSMarkersynonym
** @alias EnsOMarkersynonym
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Map::MarkerSynonym
** @attr Identifier [ajuint] Internal SQL database identifier (primary key)
** @attr Source [AjPStr] Source
** @attr Name [AjPStr] Name
** @@
******************************************************************************/

typedef struct EnsSMarkersynonym
{
    ajuint Use;
    ajuint Identifier;
    AjPStr Source;
    AjPStr Name;
} EnsOMarkersynonym;

#define EnsPMarkersynonym EnsOMarkersynonym*




/* @data EnsPMarkermaplocation ************************************************
**
** Ensembl Marker Map Location
**
** @alias EnsSMarkermaplocation
** @alias EnsOMarkermaplocation
**
** @cc Bio::EnsEMBL::Map::MapLocation
** @attr Markersynonym [EnsPMarkersynonym] Ensembl Marker Synonym
** @attr MapName [AjPStr] Map name
** @attr ChromosomeName [AjPStr] Chromosome name
** @attr Position [AjPStr] Position
** @attr LODScore [float] LOD score
** @attr Use [ajuint] Use counter
** @@
******************************************************************************/

typedef struct EnsSMarkermaplocation
{
    EnsPMarkersynonym Markersynonym;
    AjPStr MapName;
    AjPStr ChromosomeName;
    AjPStr Position;
    float LODScore;
    ajuint Use;
} EnsOMarkermaplocation;

#define EnsPMarkermaplocation EnsOMarkermaplocation*




/* @data EnsPMarkeradaptor ****************************************************
**
** Ensembl Marker Adaptor
**
** @alias EnsSMarkeradaptor
** @alias EnsOMarkeradaptor
**
** @attr Adaptor [EnsPBaseadaptor] Ensembl Base Adaptor
** @@
******************************************************************************/

typedef struct EnsSMarkeradaptor
{
    EnsPBaseadaptor Adaptor;
} EnsOMarkeradaptor;

#define EnsPMarkeradaptor EnsOMarkeradaptor*




/* EnsEMarkerType *************************************************************
**
** Ensembl Marker Type enumeration.
**
******************************************************************************/

typedef enum EnsOMarkerType
{
    ensEMarkerTypeNULL,
    ensEMarkerTypeEST,
    ensEMarkerTypeMicroSatellite
} EnsEMarkerType;




/* @data EnsPMarker ***********************************************************
**
** Ensembl Marker
**
** @alias EnsSMarker
** @alias EnsOMarker
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] Internal SQL database identifier (primary key)
** @attr Adaptor [EnsPMarkeradaptor] Ensembl Marker Adaptor
** @cc Bio::EnsEMBL::Map::Marker
** @attr DisplaySynonym [EnsPMarkersynonym] Display Ensembl Marker Synonym
** @attr LeftPrimer [AjPStr] Left primer
** @attr RightPrimer [AjPStr] Right primer
** @attr Markersynonyms [AjPList] AJAX List of Ensembl Marker Synonyms
** @attr Markermaplocations [AjPList] AJAX List of Ensembl Marker Map Locations
** @attr MinimumDistance [ajuint] Minimum primer distance
** @attr MaximumDistance [ajuint] Maximum primer distance
** @attr Type [EnsEMarkerType] Type
** @attr Priority [ajint] Priority
** @@
******************************************************************************/

typedef struct EnsSMarker
{
    ajuint Use;
    ajuint Identifier;
    EnsPMarkeradaptor Adaptor;
    EnsPMarkersynonym DisplaySynonym;
    AjPStr LeftPrimer;
    AjPStr RightPrimer;
    AjPList Markersynonyms;
    AjPList Markermaplocations;
    ajuint MinimumDistance;
    ajuint MaximumDistance;
    EnsEMarkerType Type;
    ajint Priority;
} EnsOMarker;

#define EnsPMarker EnsOMarker*




/* @data EnsPMarkerfeatureadaptor *********************************************
**
** Ensembl Marker Feature Adaptor.
**
** @alias EnsSMarkerfeatureadaptor
** @alias EnsOMarkerfeatureadaptor
**
** @cc Bio::EnsEMBL::Map::DBSQL::MarkerFeatureAdaptor
** @attr Adaptor [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @@
******************************************************************************/

typedef struct EnsSMarkerfeatureadaptor
{
    EnsPFeatureadaptor Adaptor;
} EnsOMarkerfeatureadaptor;

#define EnsPMarkerfeatureadaptor EnsOMarkerfeatureadaptor*




/* @data EnsPMarkerfeature ****************************************************
**
** Ensembl Marker Feature
**
** @alias EnsSMarkerfeature
** @alias EnsOMarkerfeature
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] Internal SQL database identifier (primary key)
** @attr Adaptor [EnsPMarkerfeatureadaptor] Ensembl Marker Feature Adaptor
** @cc Bio::EnsEMBL::Feature
** @attr Feature [EnsPFeature] Ensembl Feature
** @cc Bio::EnsEMBL::Map::MarkerFeature
** @attr Marker [EnsPMarker] Ensembl Marker
** @attr MapWeight [ajint] Map weight
** @attr Padding [ajint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSMarkerfeature
{
    ajuint Use;
    ajuint Identifier;
    EnsPMarkerfeatureadaptor Adaptor;
    EnsPFeature Feature;
    EnsPMarker Marker;
    ajint MapWeight;
    ajint Padding;
} EnsOMarkerfeature;

#define EnsPMarkerfeature EnsOMarkerfeature*




/*
** Prototype definitions
*/

/* Ensembl Marker Synonym */

EnsPMarkersynonym ensMarkersynonymNew(ajuint identifier,
                                      AjPStr source,
                                      AjPStr name);

EnsPMarkersynonym ensMarkersynonymNewObj(const EnsPMarkersynonym object);

EnsPMarkersynonym ensMarkersynonymNewRef(EnsPMarkersynonym ms);

void ensMarkersynonymDel(EnsPMarkersynonym* Pms);

ajuint ensMarkersynonymGetIdentifier(const EnsPMarkersynonym ms);

AjPStr ensMarkersynonymGetSource(const EnsPMarkersynonym ms);

AjPStr ensMarkersynonymGetName(const EnsPMarkersynonym ms);

AjBool ensMarkersynonymSetIdentifier(EnsPMarkersynonym ms, ajuint identifier);

AjBool ensMarkersynonymSetSource(EnsPMarkersynonym ms, AjPStr source);

AjBool ensMarkersynonymSetName(EnsPMarkersynonym ms, AjPStr name);

AjBool ensMarkersynonymTrace(const EnsPMarkersynonym ms, ajuint level);

ajulong ensMarkersynonymGetMemsize(const EnsPMarkersynonym ms);

/* Ensembl Marker Synonym Adaptor */

AjBool ensMarkersynonymadaptorFetchByIdentifier(EnsPDatabaseadaptor dba,
                                                ajuint identifier,
                                                EnsPMarkersynonym *Pms);

AjBool ensMarkersynonymadaptorFetchAllByMarkerIdentifier(
    EnsPDatabaseadaptor dba,
    ajuint markerid,
    AjPList mss);

/* Ensembl Marker Map Location */

EnsPMarkermaplocation ensMarkermaplocationNew(EnsPMarkersynonym ms,
                                              AjPStr mapname,
                                              AjPStr chrname,
                                              AjPStr position,
                                              float lodscore);

EnsPMarkermaplocation ensMarkermaplocationNewObj(
    const EnsPMarkermaplocation object);

EnsPMarkermaplocation ensMarkermaplocationNewRef(EnsPMarkermaplocation mml);

void ensMarkermaplocationDel(EnsPMarkermaplocation* Pmml);

EnsPMarkersynonym ensMarkermaplocationGetMarkersynonym(
    const EnsPMarkermaplocation mml);

AjPStr ensMarkermaplocationGetMapName(const EnsPMarkermaplocation mml);

AjPStr ensMarkermaplocationGetChromosomeName(const EnsPMarkermaplocation mml);

AjPStr ensMarkermaplocationGetPosition(const EnsPMarkermaplocation mml);

float ensMarkermaplocationGetLODScore(const EnsPMarkermaplocation mml);

AjBool ensMarkermaplocationSetMarkersynonym(EnsPMarkermaplocation mml,
                                            EnsPMarkersynonym ms);

AjBool ensMarkermaplocationSetMapName(EnsPMarkermaplocation mml,
                                      AjPStr mapname);

AjBool ensMarkermaplocationSetChromosomeName(EnsPMarkermaplocation mml,
                                             AjPStr chrname);

AjBool ensMarkermaplocationSetPosition(EnsPMarkermaplocation mml,
                                       AjPStr position);

AjBool ensMarkermaplocationSetLODScore(EnsPMarkermaplocation mml,
                                       float lodscore);

AjBool ensMarkermaplocationTrace(const EnsPMarkermaplocation mml,
                                 ajuint level);

ajulong ensMarkermaplocationGetMemsize(const EnsPMarkermaplocation mml);

/* Ensembl Marker Map Location Adaptor */

AjBool ensMarkermaplocationadaptorFetchAllByMarkerIdentifier(
    EnsPDatabaseadaptor dba,
    ajuint markerid,
    AjPList mmls);

/* Ensembl Marker */

EnsPMarker ensMarkerNew(EnsPMarkeradaptor adaptor,
                        ajuint identifier,
                        EnsPMarkersynonym display,
                        AjPStr lprimer,
                        AjPStr rprimer,
                        AjPList mss,
                        AjPList mmls,
                        ajuint mindistance,
                        ajuint maxdistance,
                        EnsEMarkerType type,
                        ajint priority);

EnsPMarker ensMarkerNewObj(const EnsPMarker object);

EnsPMarker ensMarkerNewRef(EnsPMarker marker);

void ensMarkerDel(EnsPMarker* Pmarker);

EnsPMarkeradaptor ensMarkerGetAdaptor(const EnsPMarker marker);

ajuint ensMarkerGetIdentifier(const EnsPMarker marker);

EnsPMarkersynonym ensMarkerGetDisplaySynonym(const EnsPMarker marker);

AjPStr ensMarkerGetLeftPrimer(const EnsPMarker marker);

AjPStr ensMarkerGetRightPrimer(const EnsPMarker marker);

const AjPList ensMarkerGetMarkersynonyms(EnsPMarker marker);

const AjPList ensMarkerGetMarkermaplocations(EnsPMarker marker);

ajuint ensMarkerGetMinimumDistance(const EnsPMarker marker);

ajuint ensMarkerGetMaximumDistance(const EnsPMarker marker);

EnsEMarkerType ensMarkerGetType(const EnsPMarker marker);

ajint ensMarkerGetPriority(const EnsPMarker marker);

AjBool ensMarkerSetAdaptor(EnsPMarker marker, EnsPMarkeradaptor ma);

AjBool ensMarkerSetIdentifier(EnsPMarker marker, ajuint identifier);

AjBool ensMarkerSetLeftPrimer(EnsPMarker marker, AjPStr lprimer);

AjBool ensMarkerSetRightPrimer(EnsPMarker marker, AjPStr rprimer);

AjBool ensMarkerSetDisplaySynonym(EnsPMarker marker,
                                  EnsPMarkersynonym display);

AjBool ensMarkerSetMinimumDistance(EnsPMarker marker, ajuint mindistance);

AjBool ensMarkerSetMaximumDistance(EnsPMarker marker, ajuint maxdistance);

AjBool ensMarkerSetType(EnsPMarker marker, EnsEMarkerType type);

AjBool ensMarkerSetPriority(EnsPMarker marker, ajint priority);

AjBool ensMarkerTrace(const EnsPMarker marker, ajuint level);

ajulong ensMarkerGetMemsize(const EnsPMarker marker);

AjBool ensMarkerAddMarkersynonym(EnsPMarker marker, EnsPMarkersynonym ms);

AjBool ensMarkerAddMarkermaplocation(EnsPMarker marker,
                                     EnsPMarkermaplocation mml);

EnsEMarkerType ensMarkerTypeFromStr(const AjPStr type);

const char *ensMarkerTypeToChar(EnsEMarkerType type);

AjBool ensMarkerClearMarkersynonyms(EnsPMarker marker);

AjBool ensMarkerClearMarkermaplocations(EnsPMarker marker);

/* Ensembl Marker Adaptor */

EnsPMarkeradaptor ensRegistryGetMarkeradaptor(
    EnsPDatabaseadaptor dba);

EnsPMarkeradaptor ensMarkeradaptorNew(
    EnsPDatabaseadaptor dba);

void ensMarkeradaptorDel(EnsPMarkeradaptor* Pma);

EnsPBaseadaptor ensMarkeradaptorGetBaseadaptor(const EnsPMarkeradaptor ma);

AjBool ensMarkeradaptorFetchAll(const EnsPMarkeradaptor ma,
                                AjPList markers);

AjBool ensMarkeradaptorFetchByIdentifier(const EnsPMarkeradaptor ma,
                                         ajuint identifier,
                                         EnsPMarker *Pmarker);

AjBool ensMarkeradaptorFetchAllBySynonym(const EnsPMarkeradaptor ma,
                                         const AjPStr name,
                                         const AjPStr source,
                                         AjPList markers);

__deprecated AjBool ensMarkeradaptorFetchAttributes(const EnsPMarkeradaptor ma,
                                                    EnsPMarker marker);

/* Ensembl Marker Feature */

EnsPMarkerfeature ensMarkerfeatureNew(EnsPMarkerfeatureadaptor mfa,
                                      ajuint identifier,
                                      EnsPFeature feature,
                                      EnsPMarker marker,
                                      ajint mapweight);

EnsPMarkerfeature ensMarkerfeatureNewObj(const EnsPMarkerfeature object);

EnsPMarkerfeature ensMarkerfeatureNewRef(EnsPMarkerfeature mf);

void ensMarkerfeatureDel(EnsPMarkerfeature* Pmf);

EnsPMarkerfeatureadaptor ensMarkerfeatureGetAdaptor(
    const EnsPMarkerfeature mf);

ajuint ensMarkerfeatureGetIdentifier(const EnsPMarkerfeature mf);

EnsPFeature ensMarkerfeatureGetFeature(const EnsPMarkerfeature mf);

EnsPMarker ensMarkerfeatureGetMarker(const EnsPMarkerfeature mf);

ajint ensMarkerfeatureGetMapWeight(const EnsPMarkerfeature mf);

AjBool ensMarkerfeatureSetAdaptor(EnsPMarkerfeature mf,
                                  EnsPMarkerfeatureadaptor mfa);

AjBool ensMarkerfeatureSetIdentifier(EnsPMarkerfeature mf, ajuint identifier);

AjBool ensMarkerfeatureSetFeature(EnsPMarkerfeature mf, EnsPFeature feature);

AjBool ensMarkerfeatureSetMarker(EnsPMarkerfeature mf, EnsPMarker marker);

AjBool ensMarkerfeatureSetMapWeight(EnsPMarkerfeature mf, ajint mapweight);

AjBool ensMarkerfeatureTrace(const EnsPMarkerfeature mf, ajuint level);

ajulong ensMarkerfeatureGetMemsize(const EnsPMarkerfeature mf);

AjBool ensMarkerfeatureSortByStartAscending(AjPList mfs);

AjBool ensMarkerfeatureSortByStartDescending(AjPList mfs);

/* Ensembl Marker Feature Adaptor */

EnsPMarkerfeatureadaptor ensRegistryGetMarkerfeatureadaptor(
    EnsPDatabaseadaptor dba);

EnsPMarkerfeatureadaptor ensMarkerfeatureadaptorNew(
    EnsPDatabaseadaptor dba);

void ensMarkerfeatureadaptorDel(EnsPMarkerfeatureadaptor *Pmfa);

AjBool ensMarkerfeatureadaptorFetchAllByMarker(
    const EnsPMarkerfeatureadaptor mfa,
    const EnsPMarker marker,
    AjPList mfs);

AjBool ensMarkerfeatureadaptorFetchAllBySlice(
    const EnsPMarkerfeatureadaptor mfa,
    EnsPSlice slice,
    ajint priority,
    ajuint mapweight,
    const AjPStr anname,
    AjPList mfs);

AjBool ensMarkerfeatureadaptorFetchAllBySliceMarkerName(
    const EnsPMarkerfeatureadaptor mfa,
    EnsPSlice slice,
    const AjPStr name,
    AjPList mfs);

/*
** End of prototype definitions
*/




#endif /* ensmarker_h */

#ifdef __cplusplus
}
#endif

/* @include ensmarker *********************************************************
**
** Ensembl Marker functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.24 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/04/12 20:34:16 $ by $Author: mks $
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

#ifndef ENSMARKER_H
#define ENSMARKER_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ensfeature.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */

/* @const EnsPMarkeradaptor ***************************************************
**
** Ensembl Marker Adaptor.
** Defined as an alias in EnsPBaseadaptor.
**
** #alias EnsPBaseadaptor
**
** #cc Bio::EnsEMBL::Map::DBSQL::MarkerAdaptor
** ##
******************************************************************************/

#define EnsPMarkeradaptor EnsPBaseadaptor




/* @const EnsPMarkerfeatureadaptor ********************************************
**
** Ensembl Marker Feature Adaptor.
** Defined as an alias in EnsPFeatureadaptor.
**
** #alias EnsPFeatureadaptor
**
** #cc Bio::EnsEMBL::Map::DBSQL::MarkerFeatureAdaptor
** ##
******************************************************************************/

#define EnsPMarkerfeatureadaptor EnsPFeatureadaptor




/* @enum EnsEMarkerType *******************************************************
**
** Ensembl Marker Type enumeration
**
** @value ensEMarkerTypeNULL Null
** @value ensEMarkerTypeEST Expressed Sequence Tag
** @value ensEMarkerTypeMicroSatellite Micro-Satellite
** @@
******************************************************************************/

typedef enum EnsOMarkerType
{
    ensEMarkerTypeNULL,
    ensEMarkerTypeEST,
    ensEMarkerTypeMicroSatellite
} EnsEMarkerType;




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */

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
** @attr Mapname [AjPStr] Map name
** @attr Chromosomename [AjPStr] Chromosome name
** @attr Position [AjPStr] Position
** @attr Lodscore [float] LOD score
** @attr Use [ajuint] Use counter
** @@
******************************************************************************/

typedef struct EnsSMarkermaplocation
{
    EnsPMarkersynonym Markersynonym;
    AjPStr Mapname;
    AjPStr Chromosomename;
    AjPStr Position;
    float Lodscore;
    ajuint Use;
} EnsOMarkermaplocation;

#define EnsPMarkermaplocation EnsOMarkermaplocation*




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
** @attr Displaysynonym [EnsPMarkersynonym] Display Ensembl Marker Synonym
** @attr PrimerLeft [AjPStr] Left primer
** @attr PrimerRight [AjPStr] Right primer
** @attr Markersynonyms [AjPList] AJAX List of Ensembl Marker Synonyms
** @attr Markermaplocations [AjPList] AJAX List of Ensembl Marker Map Locations
** @attr DistanceMinimum [ajuint] Minimum primer distance
** @attr DistanceMaximum [ajuint] Maximum primer distance
** @attr Type [EnsEMarkerType] Type
** @attr Priority [ajint] Priority
** @@
******************************************************************************/

typedef struct EnsSMarker
{
    ajuint Use;
    ajuint Identifier;
    EnsPMarkeradaptor Adaptor;
    EnsPMarkersynonym Displaysynonym;
    AjPStr PrimerLeft;
    AjPStr PrimerRight;
    AjPList Markersynonyms;
    AjPList Markermaplocations;
    ajuint DistanceMinimum;
    ajuint DistanceMaximum;
    EnsEMarkerType Type;
    ajint Priority;
} EnsOMarker;

#define EnsPMarker EnsOMarker*




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
** @attr Mapweight [ajint] Map weight
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
    ajint Mapweight;
    ajint Padding;
} EnsOMarkerfeature;

#define EnsPMarkerfeature EnsOMarkerfeature*




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */

/*
** Prototype definitions
*/

/* Ensembl Marker Synonym */

EnsPMarkersynonym ensMarkersynonymNewCpy(const EnsPMarkersynonym ms);

EnsPMarkersynonym ensMarkersynonymNewIni(ajuint identifier,
                                         AjPStr source,
                                         AjPStr name);

EnsPMarkersynonym ensMarkersynonymNewRef(EnsPMarkersynonym ms);

void ensMarkersynonymDel(EnsPMarkersynonym *Pms);

ajuint ensMarkersynonymGetIdentifier(const EnsPMarkersynonym ms);

AjPStr ensMarkersynonymGetName(const EnsPMarkersynonym ms);

AjPStr ensMarkersynonymGetSource(const EnsPMarkersynonym ms);

AjBool ensMarkersynonymSetIdentifier(EnsPMarkersynonym ms, ajuint identifier);

AjBool ensMarkersynonymSetName(EnsPMarkersynonym ms, AjPStr name);

AjBool ensMarkersynonymSetSource(EnsPMarkersynonym ms, AjPStr source);

AjBool ensMarkersynonymTrace(const EnsPMarkersynonym ms, ajuint level);

size_t ensMarkersynonymCalculateMemsize(const EnsPMarkersynonym ms);

/* Ensembl Marker Synonym Adaptor */

AjBool ensMarkersynonymadaptorFetchAllbyMarkeridentifier(
    EnsPDatabaseadaptor dba,
    ajuint markerid,
    AjPList mss);

AjBool ensMarkersynonymadaptorFetchByIdentifier(
    EnsPDatabaseadaptor dba,
    ajuint identifier,
    EnsPMarkersynonym *Pms);

/* Ensembl Marker Map Location */

EnsPMarkermaplocation ensMarkermaplocationNewCpy(
    const EnsPMarkermaplocation mml);

EnsPMarkermaplocation ensMarkermaplocationNewIni(EnsPMarkersynonym ms,
                                                 AjPStr mapname,
                                                 AjPStr chrname,
                                                 AjPStr position,
                                                 float lodscore);

EnsPMarkermaplocation ensMarkermaplocationNewRef(EnsPMarkermaplocation mml);

void ensMarkermaplocationDel(EnsPMarkermaplocation *Pmml);

AjPStr ensMarkermaplocationGetChromosomename(const EnsPMarkermaplocation mml);

float ensMarkermaplocationGetLodscore(const EnsPMarkermaplocation mml);

AjPStr ensMarkermaplocationGetMapname(const EnsPMarkermaplocation mml);

EnsPMarkersynonym ensMarkermaplocationGetMarkersynonym(
    const EnsPMarkermaplocation mml);

AjPStr ensMarkermaplocationGetPosition(const EnsPMarkermaplocation mml);

AjBool ensMarkermaplocationSetChromosomename(EnsPMarkermaplocation mml,
                                             AjPStr chrname);

AjBool ensMarkermaplocationSetLodscore(EnsPMarkermaplocation mml,
                                       float lodscore);

AjBool ensMarkermaplocationSetMapname(EnsPMarkermaplocation mml,
                                      AjPStr mapname);

AjBool ensMarkermaplocationSetMarkersynonym(EnsPMarkermaplocation mml,
                                            EnsPMarkersynonym ms);

AjBool ensMarkermaplocationSetPosition(EnsPMarkermaplocation mml,
                                       AjPStr position);

AjBool ensMarkermaplocationTrace(const EnsPMarkermaplocation mml,
                                 ajuint level);

size_t ensMarkermaplocationCalculateMemsize(const EnsPMarkermaplocation mml);

/* Ensembl Marker Map Location Adaptor */

AjBool ensMarkermaplocationadaptorFetchAllbyMarkeridentifier(
    EnsPDatabaseadaptor dba,
    ajuint markerid,
    AjPList mmls);

/* Ensembl Marker */

EnsPMarker ensMarkerNewCpy(const EnsPMarker marker);

EnsPMarker ensMarkerNewIni(EnsPMarkeradaptor adaptor,
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

EnsPMarker ensMarkerNewRef(EnsPMarker marker);

void ensMarkerDel(EnsPMarker *Pmarker);

EnsPMarkeradaptor ensMarkerGetAdaptor(const EnsPMarker marker);

EnsPMarkersynonym ensMarkerGetDisplaysynonym(const EnsPMarker marker);

ajuint ensMarkerGetDistanceMaximum(const EnsPMarker marker);

ajuint ensMarkerGetDistanceMinimum(const EnsPMarker marker);

ajuint ensMarkerGetIdentifier(const EnsPMarker marker);

AjPStr ensMarkerGetPrimerLeft(const EnsPMarker marker);

AjPStr ensMarkerGetPrimerRight(const EnsPMarker marker);

ajint ensMarkerGetPriority(const EnsPMarker marker);

EnsEMarkerType ensMarkerGetType(const EnsPMarker marker);

const AjPList ensMarkerLoadMarkermaplocations(EnsPMarker marker);

const AjPList ensMarkerLoadMarkersynonyms(EnsPMarker marker);

AjBool ensMarkerSetAdaptor(EnsPMarker marker,
                           EnsPMarkeradaptor ma);

AjBool ensMarkerSetDisplaysynonym(EnsPMarker marker,
                                  EnsPMarkersynonym display);

AjBool ensMarkerSetDistanceMaximum(EnsPMarker marker,
                                   ajuint maxdistance);

AjBool ensMarkerSetDistanceMinimum(EnsPMarker marker,
                                   ajuint mindistance);

AjBool ensMarkerSetIdentifier(EnsPMarker marker,
                              ajuint identifier);

AjBool ensMarkerSetPrimerLeft(EnsPMarker marker,
                              AjPStr lprimer);

AjBool ensMarkerSetPrimerRight(EnsPMarker marker,
                               AjPStr rprimer);

AjBool ensMarkerSetPriority(EnsPMarker marker,
                            ajint priority);

AjBool ensMarkerSetType(EnsPMarker marker,
                        EnsEMarkerType type);

AjBool ensMarkerTrace(const EnsPMarker marker, ajuint level);

size_t ensMarkerCalculateMemsize(const EnsPMarker marker);

AjBool ensMarkerAddMarkermaplocation(EnsPMarker marker,
                                     EnsPMarkermaplocation mml);

AjBool ensMarkerAddMarkersynonym(EnsPMarker marker,
                                 EnsPMarkersynonym ms);

AjBool ensMarkerClearMarkermaplocations(EnsPMarker marker);

AjBool ensMarkerClearMarkersynonyms(EnsPMarker marker);

EnsEMarkerType ensMarkerTypeFromStr(const AjPStr type);

const char *ensMarkerTypeToChar(EnsEMarkerType type);

/* Ensembl Marker Adaptor */

EnsPMarkeradaptor ensRegistryGetMarkeradaptor(
    EnsPDatabaseadaptor dba);

EnsPMarkeradaptor ensMarkeradaptorNew(
    EnsPDatabaseadaptor dba);

void ensMarkeradaptorDel(EnsPMarkeradaptor *Pma);

EnsPBaseadaptor ensMarkeradaptorGetBaseadaptor(EnsPMarkeradaptor ma);

EnsPDatabaseadaptor ensMarkeradaptorGetDatabaseadaptor(EnsPMarkeradaptor ma);

AjBool ensMarkeradaptorFetchAll(EnsPMarkeradaptor ma,
                                AjPList markers);

AjBool ensMarkeradaptorFetchAllbySynonym(EnsPMarkeradaptor ma,
                                         const AjPStr name,
                                         const AjPStr source,
                                         AjPList markers);

AjBool ensMarkeradaptorFetchByIdentifier(EnsPMarkeradaptor ma,
                                         ajuint identifier,
                                         EnsPMarker *Pmarker);

/* Ensembl Marker Feature */

EnsPMarkerfeature ensMarkerfeatureNewCpy(const EnsPMarkerfeature mf);

EnsPMarkerfeature ensMarkerfeatureNewIni(EnsPMarkerfeatureadaptor mfa,
                                         ajuint identifier,
                                         EnsPFeature feature,
                                         EnsPMarker marker,
                                         ajint mapweight);

EnsPMarkerfeature ensMarkerfeatureNewRef(EnsPMarkerfeature mf);

void ensMarkerfeatureDel(EnsPMarkerfeature *Pmf);

EnsPMarkerfeatureadaptor ensMarkerfeatureGetAdaptor(
    const EnsPMarkerfeature mf);

EnsPFeature ensMarkerfeatureGetFeature(const EnsPMarkerfeature mf);

ajuint ensMarkerfeatureGetIdentifier(const EnsPMarkerfeature mf);

ajint ensMarkerfeatureGetMapweight(const EnsPMarkerfeature mf);

EnsPMarker ensMarkerfeatureGetMarker(const EnsPMarkerfeature mf);

AjBool ensMarkerfeatureSetAdaptor(EnsPMarkerfeature mf,
                                  EnsPMarkerfeatureadaptor mfa);

AjBool ensMarkerfeatureSetIdentifier(EnsPMarkerfeature mf, ajuint identifier);

AjBool ensMarkerfeatureSetFeature(EnsPMarkerfeature mf, EnsPFeature feature);

AjBool ensMarkerfeatureSetMarker(EnsPMarkerfeature mf, EnsPMarker marker);

AjBool ensMarkerfeatureSetMapweight(EnsPMarkerfeature mf, ajint mapweight);

AjBool ensMarkerfeatureTrace(const EnsPMarkerfeature mf, ajuint level);

size_t ensMarkerfeatureCalculateMemsize(const EnsPMarkerfeature mf);

/* AJAX List of Ensembl Marker Feature objects */

AjBool ensListMarkerfeatureSortEndAscending(AjPList mfs);

AjBool ensListMarkerfeatureSortEndDescending(AjPList mfs);

AjBool ensListMarkerfeatureSortIdentifierAscending(AjPList mfs);

AjBool ensListMarkerfeatureSortStartAscending(AjPList mfs);

AjBool ensListMarkerfeatureSortStartDescending(AjPList mfs);

/* Ensembl Marker Feature Adaptor */

EnsPMarkerfeatureadaptor ensRegistryGetMarkerfeatureadaptor(
    EnsPDatabaseadaptor dba);

EnsPMarkerfeatureadaptor ensMarkerfeatureadaptorNew(
    EnsPDatabaseadaptor dba);

void ensMarkerfeatureadaptorDel(EnsPMarkerfeatureadaptor *Pmfa);

EnsPDatabaseadaptor ensMarkerfeatureadaptorGetDatabaseadaptor(
    EnsPMarkerfeatureadaptor mfa);

AjBool ensMarkerfeatureadaptorFetchAllbyMarker(
    EnsPMarkerfeatureadaptor mfa,
    const EnsPMarker marker,
    AjPList mfs);

AjBool ensMarkerfeatureadaptorFetchAllbySlice(
    EnsPMarkerfeatureadaptor mfa,
    EnsPSlice slice,
    ajint priority,
    ajuint mapweight,
    const AjPStr anname,
    AjPList mfs);

AjBool ensMarkerfeatureadaptorFetchAllbySlicemarkername(
    EnsPMarkerfeatureadaptor mfa,
    EnsPSlice slice,
    const AjPStr name,
    AjPList mfs);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSMARKER_H */

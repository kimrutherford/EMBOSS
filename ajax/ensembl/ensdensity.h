/* @include ensdensity ********************************************************
**
** Ensembl Density functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.22 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/08/05 10:42:27 $ by $Author: mks $
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

#ifndef ENSDENSITY_H
#define ENSDENSITY_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ensfeature.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */

/* @const EnsPDensityfeatureadaptor *******************************************
**
** Ensembl Density Feature Adaptor.
** Defined as an alias in EnsPFeatureadaptor.
**
** #alias EnsPFeatureadaptor
**
** # Bio::EnsEMBL::DBSQL::DensityFeatureAdaptor
** ##
******************************************************************************/

#define EnsPDensityfeatureadaptor EnsPFeatureadaptor




/* @enum EnsEDensitytypeType **************************************************
**
** Ensembl Density Type type enumeration
**
** @value ensEDensitytypeTypeNULL Null
** @value ensEDensitytypeTypeSum Sum
** @value ensEDensitytypeTypeRatio Ratio
** @@
******************************************************************************/

typedef enum EnsODensitytypeType
{
    ensEDensitytypeTypeNULL,
    ensEDensitytypeTypeSum,
    ensEDensitytypeTypeRatio
} EnsEDensitytypeType;




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */

/* @data EnsPDensitytypeadaptor ***********************************************
**
** Ensembl Density Type Adaptor.
**
** @alias EnsSDensitytypeadaptor
** @alias EnsODensitytypeadaptor
**
** @attr Adaptor [EnsPBaseadaptor] Ensembl Base Adaptor
** @attr CacheByIdentifier [AjPTable] Identifier cache
** @@
******************************************************************************/

typedef struct EnsSDensitytypeadaptor
{
    EnsPBaseadaptor Adaptor;
    AjPTable CacheByIdentifier;
} EnsODensitytypeadaptor;

#define EnsPDensitytypeadaptor EnsODensitytypeadaptor*




/* @data EnsPDensitytype ******************************************************
**
** Ensembl Density Type.
**
** @alias EnsSDensitytype
** @alias EnsODensitytype
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] Internal SQL database identifier (primary key)
** @attr Adaptor [EnsPDensitytypeadaptor] Ensembl Density Type Adaptor
** @cc Bio::EnsEMBL::Densitytype
** @attr Analysis [EnsPAnalysis] Ensembl Analysis
** @attr Type [EnsEDensitytypeType] Type enumeration
** @attr Size [ajuint] Block size
** @attr Features [ajuint] Number of Ensembl Density Features per
**                         Ensembl Sequence Region
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSDensitytype
{
    ajuint Use;
    ajuint Identifier;
    EnsPDensitytypeadaptor Adaptor;
    EnsPAnalysis Analysis;
    EnsEDensitytypeType Type;
    ajuint Size;
    ajuint Features;
    ajuint Padding;
} EnsODensitytype;

#define EnsPDensitytype EnsODensitytype*




/* @data EnsPDensityfeature ***************************************************
**
** Ensembl Density Feature.
**
** @alias EnsSDensityfeature
** @alias EnsODensityfeature
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPDensityfeatureadaptor] Ensembl Density Feature Adaptor
** @cc Bio::EnsEMBL::Feature
** @attr Feature [EnsPFeature] Ensembl Feature
** @cc Bio::EnsEMBL::Densityfeature
** @attr Densitytype [EnsPDensitytype] Ensembl Density Type
** @attr Value [float] Value
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSDensityfeature
{
    ajuint Use;
    ajuint Identifier;
    EnsPDensityfeatureadaptor Adaptor;
    EnsPFeature Feature;
    EnsPDensitytype Densitytype;
    float Value;
    ajuint Padding;
} EnsODensityfeature;

#define EnsPDensityfeature EnsODensityfeature*




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */

/*
** Prototype definitions
*/

/* Ensembl Density Type */

EnsPDensitytype ensDensitytypeNewCpy(const EnsPDensitytype dt);

EnsPDensitytype ensDensitytypeNewIni(EnsPDensitytypeadaptor dta,
                                     ajuint identifier,
                                     EnsPAnalysis analysis,
                                     EnsEDensitytypeType type,
                                     ajuint size,
                                     ajuint features);

EnsPDensitytype ensDensitytypeNewRef(EnsPDensitytype dt);

void ensDensitytypeDel(EnsPDensitytype *Pdt);

EnsPDensitytypeadaptor ensDensitytypeGetAdaptor(const EnsPDensitytype dt);

EnsPAnalysis ensDensitytypeGetAnalysis(const EnsPDensitytype dt);

ajuint ensDensitytypeGetFeatures(const EnsPDensitytype dt);

ajuint ensDensitytypeGetIdentifier(const EnsPDensitytype dt);

ajuint ensDensitytypeGetSize(const EnsPDensitytype dt);

EnsEDensitytypeType ensDensitytypeGetType(const EnsPDensitytype dt);

AjBool ensDensitytypeSetAdaptor(EnsPDensitytype dt,
                                EnsPDensitytypeadaptor dta);

AjBool ensDensitytypeSetIdentifier(EnsPDensitytype dt,
                                   ajuint identifier);

AjBool ensDensitytypeSetAnalysis(EnsPDensitytype dt,
                                 EnsPAnalysis analysis);

AjBool ensDensitytypeSetFeatures(EnsPDensitytype dt,
                                 ajuint features);

AjBool ensDensitytypeSetType(EnsPDensitytype dt,
                             EnsEDensitytypeType type);

AjBool ensDensitytypeSetSize(EnsPDensitytype dt,
                             ajuint size);

AjBool ensDensitytypeTrace(const EnsPDensitytype dt, ajuint level);

size_t ensDensitytypeCalculateMemsize(const EnsPDensitytype dt);

EnsEDensitytypeType ensDensitytypeTypeFromStr(const AjPStr type);

const char *ensDensitytypeTypeToChar(const EnsEDensitytypeType dtt);

/* Ensembl Density Type Adaptor */

EnsPDensitytypeadaptor ensRegistryGetDensitytypeadaptor(
    EnsPDatabaseadaptor dba);

EnsPDensitytypeadaptor ensDensitytypeadaptorNew(
    EnsPDatabaseadaptor dba);

void ensDensitytypeadaptorDel(EnsPDensitytypeadaptor *Pdta);

EnsPBaseadaptor ensDensitytypeadaptorGetBaseadaptor(
    EnsPDensitytypeadaptor dta);

EnsPDatabaseadaptor ensDensitytypeadaptorGetDatabaseadaptor(
    EnsPDensitytypeadaptor dta);

AjBool ensDensitytypeadaptorFetchAll(
    EnsPDensitytypeadaptor dta,
    AjPList dts);

AjBool ensDensitytypeadaptorFetchAllbyAnalysisname(
    EnsPDensitytypeadaptor dta,
    const AjPStr name,
    AjPList dts);

AjBool ensDensitytypeadaptorFetchByIdentifier(
    EnsPDensitytypeadaptor dta,
    ajuint identifier,
    EnsPDensitytype *Pdt);

/* Ensembl Density Feature */

EnsPDensityfeature ensDensityfeatureNewCpy(const EnsPDensityfeature df);

EnsPDensityfeature ensDensityfeatureNewIni(EnsPDensityfeatureadaptor dfa,
                                           ajuint identifier,
                                           EnsPFeature feature,
                                           EnsPDensitytype dt,
                                           float value);

EnsPDensityfeature ensDensityfeatureNewRef(EnsPDensityfeature df);

void ensDensityfeatureDel(EnsPDensityfeature *Pdf);

EnsPDensityfeatureadaptor ensDensityfeatureGetAdaptor(
    const EnsPDensityfeature df);

EnsPDensitytype ensDensityfeatureGetDensitytype(const EnsPDensityfeature df);

EnsPFeature ensDensityfeatureGetFeature(const EnsPDensityfeature df);

ajuint ensDensityfeatureGetIdentifier(const EnsPDensityfeature df);

float ensDensityfeatureGetValue(const EnsPDensityfeature df);

AjBool ensDensityfeatureSetAdaptor(EnsPDensityfeature df,
                                   EnsPDensityfeatureadaptor dfa);

AjBool ensDensityfeatureSetDensitytype(EnsPDensityfeature df,
                                       EnsPDensitytype dt);

AjBool ensDensityfeatureSetFeature(EnsPDensityfeature df,
                                   EnsPFeature feature);

AjBool ensDensityfeatureSetIdentifier(EnsPDensityfeature df,
                                      ajuint identifier);

AjBool ensDensityfeatureSetValue(EnsPDensityfeature df,
                                 float value);

AjBool ensDensityfeatureTrace(const EnsPDensityfeature df, ajuint level);

size_t ensDensityfeatureCalculateMemsize(const EnsPDensityfeature df);

/* AJAX List of Ensembl Density Feature objects */

AjBool ensListDensityfeatureSortEndAscending(AjPList dfs);

AjBool ensListDensityfeatureSortEndDescending(AjPList dfs);

AjBool ensListDensityfeatureSortIdentifierAscending(AjPList dfs);

AjBool ensListDensityfeatureSortStartAscending(AjPList dfs);

AjBool ensListDensityfeatureSortStartDescending(AjPList dfs);

/* Ensembl Density Feature Adaptor */

EnsPDensityfeatureadaptor ensRegistryGetDensityfeatureadaptor(
    EnsPDatabaseadaptor dba);

EnsPDensityfeatureadaptor ensDensityfeatureadaptorNew(
    EnsPDatabaseadaptor dba);

void ensDensityfeatureadaptorDel(EnsPDensityfeatureadaptor *Pdfa);

EnsPBaseadaptor ensDensityfeatureadaptorGetBaseadaptor(
    EnsPDensityfeatureadaptor dfa);

EnsPDatabaseadaptor ensDensityfeatureadaptorGetDatabaseadaptor(
    EnsPDensityfeatureadaptor dfa);

EnsPFeatureadaptor ensDensityfeatureadaptorGetFeatureadaptor(
    EnsPDensityfeatureadaptor dfa);

AjBool ensDensityfeatureadaptorFetchAllbySlice(
    EnsPDensityfeatureadaptor dfa,
    EnsPSlice slice,
    const AjPStr anname,
    ajuint blocks,
    AjBool interpolate,
    float maxratio,
    AjPList dfs);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSDENSITY_H */

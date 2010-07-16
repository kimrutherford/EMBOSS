#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ensdensity_h
#define ensdensity_h

#include "ensfeature.h"




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




/* EnsEDensitytypeValueType ***************************************************
**
** Ensembl Density Type value type enumeration.
**
******************************************************************************/

typedef enum EnsODensitytypeValueType
{
    ensEDensitytypeValueTypeNULL,
    ensEDensitytypeValueTypeSum,
    ensEDensitytypeValueTypeRatio
} EnsEDensitytypeValueType;




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
** @attr ValueType [EnsEDensitytypeValueType] Value type enumeration
** @attr BlockSize [ajuint] Block size
** @attr RegionFeatures [ajuint] Number of Features per Sequence Region in
**                               this Density Type
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSDensitytype
{
    ajuint Use;
    ajuint Identifier;
    EnsPDensitytypeadaptor Adaptor;
    EnsPAnalysis Analysis;
    EnsEDensitytypeValueType ValueType;
    ajuint BlockSize;
    ajuint RegionFeatures;
    ajuint Padding;
} EnsODensitytype;

#define EnsPDensitytype EnsODensitytype*




/* @data EnsPDensityfeatureadaptor ********************************************
**
** Ensembl Density Feature Adaptor.
**
** @alias EnsSDensityfeatureadaptor
** @alias EnsODensityfeatureadaptor
**
** @attr Adaptor [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @@
******************************************************************************/

typedef struct EnsSDensityfeatureadaptor
{
    EnsPFeatureadaptor Adaptor;
} EnsODensityfeatureadaptor;

#define EnsPDensityfeatureadaptor EnsODensityfeatureadaptor*




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
** @attr DensityValue [float] Density value
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
    float DensityValue;
    ajuint Padding;
} EnsODensityfeature;

#define EnsPDensityfeature EnsODensityfeature*




/*
** Prototype definitions
*/

/* Ensembl Density Type */

EnsPDensitytype ensDensitytypeNew(EnsPDensitytypeadaptor dta,
                                  ajuint identifier,
                                  EnsPAnalysis analysis,
                                  EnsEDensitytypeValueType type,
                                  ajuint size,
                                  ajuint features);

EnsPDensitytype ensDensitytypeNewObj(const EnsPDensitytype object);

EnsPDensitytype ensDensitytypeNewRef(EnsPDensitytype dt);

void ensDensitytypeDel(EnsPDensitytype* Pdt);

EnsPDensitytypeadaptor ensDensitytypeGetAdaptor(const EnsPDensitytype dt);

ajuint ensDensitytypeGetIdentifier(const EnsPDensitytype dt);

EnsPAnalysis ensDensitytypeGetAnalysis(const EnsPDensitytype dt);

EnsEDensitytypeValueType ensDensitytypeGetValueType(const EnsPDensitytype dt);

ajuint ensDensitytypeGetBlockSize(const EnsPDensitytype dt);

ajuint ensDensitytypeGetRegionFeatures(const EnsPDensitytype dt);

AjBool ensDensitytypeSetAdaptor(EnsPDensitytype dt,
                                EnsPDensitytypeadaptor dta);

AjBool ensDensitytypeSetIdentifier(EnsPDensitytype dt,
                                   ajuint identifier);

AjBool ensDensitytypeSetAnalysis(EnsPDensitytype dt,
                                 EnsPAnalysis analysis);

AjBool ensDensitytypeSetValueType(EnsPDensitytype dt,
                                  EnsEDensitytypeValueType type);

AjBool ensDensitytypeSetBlockSize(EnsPDensitytype dt,
                                  ajuint size);

AjBool ensDensitytypeSetRegionFeatures(EnsPDensitytype dt,
                                       ajuint features);

AjBool ensDensitytypeTrace(const EnsPDensitytype dt, ajuint level);

ajulong ensDensitytypeGetMemsize(const EnsPDensitytype dt);

EnsEDensitytypeValueType ensDensitytypeValeTypeFromStr(const AjPStr type);

const char* ensDensitytypeValeTypeToChar(const EnsEDensitytypeValueType type);

/* Ensembl Density Type Adaptor */

EnsPDensitytypeadaptor ensRegistryGetDensitytypeadaptor(
    EnsPDatabaseadaptor dba);

EnsPDensitytypeadaptor ensDensitytypeadaptorNew(
    EnsPDatabaseadaptor dba);

void ensDensitytypeadaptorDel(EnsPDensitytypeadaptor* Pdta);

EnsPBaseadaptor ensDensitytypeadaptorGetBaseadaptor(
    const EnsPDensitytypeadaptor dta);

EnsPDatabaseadaptor ensDensitytypeadaptorGetDatabaseadaptor(
    const EnsPDensitytypeadaptor dta);

AjBool ensDensitytypeadaptorFetchAll(
    EnsPDensitytypeadaptor dta,
    AjPList dts);

AjBool ensDensitytypeadaptorFetchByIdentifier(
    EnsPDensitytypeadaptor dta,
    ajuint identifier,
    EnsPDensitytype *Pdt);

AjBool ensDensitytypeadaptorFetchAllByAnalysisName(
    EnsPDensitytypeadaptor dta,
    const AjPStr name,
    AjPList dts);

/* Ensembl Density Feature */

EnsPDensityfeature ensDensityfeatureNew(EnsPDensityfeatureadaptor dfa,
                                        ajuint identifier,
                                        EnsPFeature feature,
                                        EnsPDensitytype dt,
                                        float value);

EnsPDensityfeature ensDensityfeatureNewObj(const EnsPDensityfeature object);

EnsPDensityfeature ensDensityfeatureNewRef(EnsPDensityfeature df);

void ensDensityfeatureDel(EnsPDensityfeature* Pdf);

EnsPDensityfeatureadaptor ensDensityfeatureGetAdaptor(
    const EnsPDensityfeature df);

ajuint ensDensityfeatureGetIdentifier(const EnsPDensityfeature df);

EnsPFeature ensDensityfeatureGetFeature(const EnsPDensityfeature df);

EnsPDensitytype ensDensityfeatureGetDensitytype(const EnsPDensityfeature df);

float ensDensityfeatureGetDensityValue(const EnsPDensityfeature df);

AjBool ensDensityfeatureSetAdaptor(EnsPDensityfeature df,
                                   EnsPDensityfeatureadaptor dfa);

AjBool ensDensityfeatureSetIdentifier(EnsPDensityfeature df,
                                      ajuint identifier);

AjBool ensDensityfeatureSetFeature(EnsPDensityfeature df,
                                   EnsPFeature feature);

AjBool ensDensityfeatureSetDensitytype(EnsPDensityfeature df,
                                       EnsPDensitytype dt);

AjBool ensDensityfeatureSetDensityValue(EnsPDensityfeature df,
                                        float value);

AjBool ensDensityfeatureTrace(const EnsPDensityfeature df, ajuint level);

ajulong ensDensityfeatureGetMemsize(const EnsPDensityfeature df);

AjBool ensDensityfeatureSortByStartAscending(AjPList dfs);

AjBool ensDensityfeatureSortByStartDescending(AjPList dfs);

/* Ensembl Density Feature Adaptor */

EnsPDensityfeatureadaptor ensRegistryGetDensityfeatureadaptor(
    EnsPDatabaseadaptor dba);

EnsPDensityfeatureadaptor ensDensityfeatureadaptorNew(
    EnsPDatabaseadaptor dba);

void ensDensityfeatureadaptorDel(EnsPDensityfeatureadaptor *Pdfa);

AjBool ensDensityfeatureadaptorFetchAllBySlice(
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




#endif /* ensdensity_h */

#ifdef __cplusplus
}
#endif

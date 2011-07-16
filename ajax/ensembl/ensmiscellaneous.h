
#ifndef ENSMISCELLANEOUS_H
#define ENSMISCELLANEOUS_H

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ensfeature.h"
#include "ensattribute.h"

AJ_BEGIN_DECLS




/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */

/* @const EnsPMiscellaneousfeatureadaptor *************************************
**
** Ensembl Miscellaneous Feature Adaptor.
** Defined as an alias in EnsPFeatureadaptor.
**
** #alias EnsPFeatureadaptor
**
** #cc Bio::EnsEMBL::DBSQL::MiscFeatureAdaptor
** ##
******************************************************************************/

#define EnsPMiscellaneousfeatureadaptor EnsPFeatureadaptor




/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */

/* @data EnsPMiscellaneoussetadaptor ******************************************
**
** Ensembl Miscellaneous Set Adaptor.
**
** @alias EnsSMiscellaneoussetadaptor
** @alias EnsOMiscellaneoussetadaptor
**
** @attr Adaptor [EnsPBaseadaptor] Ensembl Base Adaptor
** @attr CacheByIdentifier [AjPTable] Identifier cache
** @attr CacheByCode [AjPTable] Code cache
** @@
******************************************************************************/

typedef struct EnsSMiscellaneoussetadaptor
{
    EnsPBaseadaptor Adaptor;
    AjPTable CacheByIdentifier;
    AjPTable CacheByCode;
} EnsOMiscellaneoussetadaptor;

#define EnsPMiscellaneoussetadaptor EnsOMiscellaneoussetadaptor*




/* @data EnsPMiscellaneousset *************************************************
**
** Ensembl Miscellaneous Set.
**
** @alias EnsSMiscellaneousset
** @alias EnsOMiscellaneousset
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPMiscellaneoussetadaptor] Ensembl Miscellaneous
**                                             Set Adaptor
** @cc Bio::EnsEMBL::MiscSet
** @cc 'misc_set' SQL table
** @attr Code [AjPStr] Code
** @attr Name [AjPStr] Name
** @attr Description [AjPStr] Description
** @attr MaximumLength [ajuint] Maximum Feature length
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSMiscellaneousset
{
    ajuint Use;
    ajuint Identifier;
    EnsPMiscellaneoussetadaptor Adaptor;
    AjPStr Code;
    AjPStr Name;
    AjPStr Description;
    ajuint MaximumLength;
    ajuint Padding;
} EnsOMiscellaneousset;

#define EnsPMiscellaneousset EnsOMiscellaneousset*




/* @data EnsPMiscellaneousfeature *********************************************
**
** Ensembl Miscellaneous Feature.
**
** @alias EnsSMiscellaneousfeature
** @alias EnsOMiscellaneousfeature
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPMiscellaneousfeatureadaptor] Ensembl Miscellaneous
**                                                 Feature Adaptor
** @cc Bio::EnsEMBL::Feature
** @cc 'misc_feature' SQL table
** @attr Feature [EnsPFeature] Ensembl Feature
** @cc Bio::EnsEMBL::MiscFeature
** @attr Attributes [AjPList] AJAX List of Ensembl Attribute objects
** @attr Miscellaneoussets [AjPList] AJAX List of
** Ensembl Miscellaneous Set objects
** @@
******************************************************************************/

typedef struct EnsSMiscellaneousfeature
{
    ajuint Use;
    ajuint Identifier;
    EnsPMiscellaneousfeatureadaptor Adaptor;
    EnsPFeature Feature;
    AjPList Attributes;
    AjPList Miscellaneoussets;
} EnsOMiscellaneousfeature;

#define EnsPMiscellaneousfeature EnsOMiscellaneousfeature*




/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */

/*
** Prototype definitions
*/

/* Ensembl Miscellaneous Set */

EnsPMiscellaneousset ensMiscellaneoussetNewCpy(
    const EnsPMiscellaneousset ms);

EnsPMiscellaneousset ensMiscellaneoussetNewIni(
    EnsPMiscellaneoussetadaptor msa,
    ajuint identifier,
    AjPStr code,
    AjPStr name,
    AjPStr description,
    ajuint maxlen);

EnsPMiscellaneousset ensMiscellaneoussetNewRef(EnsPMiscellaneousset ms);

void ensMiscellaneoussetDel(EnsPMiscellaneousset* Pms);

EnsPMiscellaneoussetadaptor ensMiscellaneoussetGetAdaptor(
    const EnsPMiscellaneousset ms);

AjPStr ensMiscellaneoussetGetCode(const EnsPMiscellaneousset ms);

AjPStr ensMiscellaneoussetGetDescription(const EnsPMiscellaneousset ms);

ajuint ensMiscellaneoussetGetIdentifier(const EnsPMiscellaneousset ms);

ajuint ensMiscellaneoussetGetMaximumlength(const EnsPMiscellaneousset ms);

AjPStr ensMiscellaneoussetGetName(const EnsPMiscellaneousset ms);

AjBool ensMiscellaneoussetSetAdaptor(EnsPMiscellaneousset ms,
                                     EnsPMiscellaneoussetadaptor msa);

AjBool ensMiscellaneoussetSetIdentifier(EnsPMiscellaneousset ms,
                                        ajuint identifier);

AjBool ensMiscellaneoussetSetCode(EnsPMiscellaneousset ms, AjPStr code);

AjBool ensMiscellaneoussetSetName(EnsPMiscellaneousset ms, AjPStr name);

AjBool ensMiscellaneoussetSetDescription(EnsPMiscellaneousset ms,
                                         AjPStr description);

AjBool ensMiscellaneoussetSetMaximumlength(EnsPMiscellaneousset ms,
                                           ajuint maxlen);

size_t ensMiscellaneoussetCalculateMemsize(const EnsPMiscellaneousset ms);

AjBool ensMiscellaneoussetTrace(const EnsPMiscellaneousset ms, ajuint level);

/* Ensembl Miscellaneous Set Adaptor */

EnsPMiscellaneoussetadaptor ensRegistryGetMiscellaneoussetadaptor(
    EnsPDatabaseadaptor dba);

EnsPMiscellaneoussetadaptor ensMiscellaneoussetadaptorNew(
    EnsPDatabaseadaptor dba);

void ensMiscellaneoussetadaptorDel(EnsPMiscellaneoussetadaptor* Pmsa);

AjBool ensMiscellaneoussetadaptorCacheClear(
    EnsPMiscellaneoussetadaptor msa);

EnsPBaseadaptor ensMiscellaneoussetadaptorGetBaseadaptor(
    const EnsPMiscellaneoussetadaptor msa);

EnsPDatabaseadaptor ensMiscellaneoussetadaptorGetDatabaseadaptor(
    const EnsPMiscellaneoussetadaptor msa);

AjBool ensMiscellaneoussetadaptorFetchAll(
    EnsPMiscellaneoussetadaptor msa,
    AjPList mslist);

AjBool ensMiscellaneoussetadaptorFetchByCode(
    EnsPMiscellaneoussetadaptor msa,
    const AjPStr code,
    EnsPMiscellaneousset* Pms);

AjBool ensMiscellaneoussetadaptorFetchByIdentifier(
    EnsPMiscellaneoussetadaptor msa,
    ajuint identifier,
    EnsPMiscellaneousset* Pms);

/* Ensembl Miscellaneous Feature */

EnsPMiscellaneousfeature ensMiscellaneousfeatureNewCpy(
    const EnsPMiscellaneousfeature mf);

EnsPMiscellaneousfeature ensMiscellaneousfeatureNewIni(
    EnsPMiscellaneousfeatureadaptor mfa,
    ajuint identifier,
    EnsPFeature feature);

EnsPMiscellaneousfeature ensMiscellaneousfeatureNewRef(
    EnsPMiscellaneousfeature mf);

void ensMiscellaneousfeatureDel(EnsPMiscellaneousfeature* Pmf);

EnsPMiscellaneousfeatureadaptor ensMiscellaneousfeatureGetAdaptor(
    const EnsPMiscellaneousfeature mf);

const AjPList ensMiscellaneousfeatureGetAttributes(
    const EnsPMiscellaneousfeature mf);

EnsPFeature ensMiscellaneousfeatureGetFeature(
    const EnsPMiscellaneousfeature mf);

ajuint ensMiscellaneousfeatureGetIdentifier(
    const EnsPMiscellaneousfeature mf);

const AjPList ensMiscellaneousfeatureGetMiscellaneoussets(
    const EnsPMiscellaneousfeature mf);

AjBool ensMiscellaneousfeatureSetAdaptor(
    EnsPMiscellaneousfeature mf,
    EnsPMiscellaneousfeatureadaptor mfa);

AjBool ensMiscellaneousfeatureSetFeature(
    EnsPMiscellaneousfeature mf,
    EnsPFeature feature);

AjBool ensMiscellaneousfeatureSetIdentifier(
    EnsPMiscellaneousfeature mf,
    ajuint identifier);

AjBool ensMiscellaneousfeatureAddAttribute(
    EnsPMiscellaneousfeature mf,
    EnsPAttribute attribute);

AjBool ensMiscellaneousfeatureAddMiscellaneousset(
    EnsPMiscellaneousfeature mf,
    EnsPMiscellaneousset ms);

AjBool ensMiscellaneousfeatureTrace(const EnsPMiscellaneousfeature mf,
                                    ajuint level);

size_t ensMiscellaneousfeatureCalculateMemsize(
    const EnsPMiscellaneousfeature mf);

AjBool ensMiscellaneousfeatureFetchAllAttributes(
    EnsPMiscellaneousfeature mf,
    const AjPStr code,
    AjPList attributes);

AjBool ensMiscellaneousfeatureFetchAllMiscellaneoussets(
    EnsPMiscellaneousfeature mf,
    const AjPStr code,
    AjPList mslist);

AjBool ensListMiscellaneousfeatureSortStartAscending(AjPList mfs);

AjBool ensListMiscellaneousfeatureSortStartDescending(AjPList mfs);

/* Ensembl Miscellaneous Feature Adaptor */

EnsPMiscellaneousfeatureadaptor ensRegistryGetMiscellaneousfeatureadaptor(
    EnsPDatabaseadaptor dba);

EnsPMiscellaneousfeatureadaptor ensMiscellaneousfeatureadaptorNew(
    EnsPDatabaseadaptor dba);

void ensMiscellaneousfeatureadaptorDel(EnsPMiscellaneousfeatureadaptor* Pmfa);

EnsPDatabaseadaptor ensMiscellaneousfeatureadaptorGetDatabaseadaptor(
    EnsPMiscellaneousfeatureadaptor mfa);

AjBool ensMiscellaneousfeatureadaptorFetchAllbySlicecodes(
    EnsPMiscellaneousfeatureadaptor mfa,
    EnsPSlice slice,
    const AjPList codes,
    AjPList mfs);

AjBool ensMiscellaneousfeatureadaptorFetchAllbyAttributecodevalue(
    EnsPMiscellaneousfeatureadaptor mfa,
    const AjPStr code,
    const AjPStr value,
    AjPList mfs);

AjBool ensMiscellaneousfeatureadaptorRetrieveAllIdentifiers(
    EnsPMiscellaneousfeatureadaptor mfa,
    AjPList identifiers);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSMISCELLANEOUS_H */

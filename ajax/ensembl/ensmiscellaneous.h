#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ensmiscellaneous_h
#define ensmiscellaneous_h

#include "ensfeature.h"
#include "ensattribute.h"




/* @data EnsPMiscellaneoussetadaptor ******************************************
**
** Ensembl Miscellaneous Set Adaptor.
**
** @alias EnsSMiscellaneoussetadaptor
** @alias EnsOMiscellaneoussetadaptor
**
** @attr Adaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor.
** @attr CacheByIdentifier [AjPTable] Identifier cache.
** @attr CacheByCode [AjPTable] Code cache.
** @@
******************************************************************************/

typedef struct EnsSMiscellaneoussetadaptor
{
    EnsPDatabaseadaptor Adaptor;
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
** @attr Use [ajuint] Use counter.
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier.
** @attr Adaptor [EnsPMiscellaneoussetadaptor] Ensembl Miscellaneous
**                                             Set Adaptor.
** @cc Bio::EnsEMBL::MiscSet
** @cc 'misc_set' SQL table
** @attr Code [AjPStr] Code.
** @attr Name [AjPStr] Name.
** @attr Description [AjPStr] Description.
** @attr MaximumLength [ajuint] Maximum Feature length.
** @attr Padding [ajuint] Padding to alignment boundary.
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




/* @data EnsPMiscellaneousfeatureadaptor **************************************
**
** Ensembl Miscellaneous Feature Adaptor.
**
** @alias EnsSMiscellaneousfeatureadaptor
** @alias EnsOMiscellaneousfeatureadaptor
**
** @attr Adaptor [EnsPFeatureadaptor] Ensembl Feature Adaptor.
** @@
******************************************************************************/

typedef struct EnsSMiscellaneousfeatureadaptor
{
    EnsPFeatureadaptor Adaptor;
} EnsOMiscellaneousfeatureadaptor;

#define EnsPMiscellaneousfeatureadaptor EnsOMiscellaneousfeatureadaptor*




/* @data EnsPMiscellaneousfeature *********************************************
**
** Ensembl Miscellaneous Feature.
**
** @alias EnsSMiscellaneousfeature
** @alias EnsOMiscellaneousfeature
**
** @attr Use [ajuint] Use counter.
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier.
** @attr Adaptor [EnsPMiscellaneousfeatureadaptor] Ensembl Miscellaneous
**                                                 Feature Adaptor.
** @cc Bio::EnsEMBL::Feature
** @cc 'misc_feature' SQL table
** @attr Feature [EnsPFeature] Ensembl Feature.
** @cc Bio::EnsEMBL::MiscFeature
** @attr Attributes [AjPList] AJAX List of Ensembl Attributes.
** @attr Miscellaneoussets [AjPList] AJAX List of Ensembl Miscellaneous Sets.
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




/*
** Prototype definitions
*/

/* Ensembl Miscellaneous Set */

EnsPMiscellaneousset ensMiscellaneoussetNew(EnsPMiscellaneoussetadaptor adaptor,
                                            ajuint identifier,
                                            AjPStr code,
                                            AjPStr name,
                                            AjPStr description,
                                            ajuint maxlen);

EnsPMiscellaneousset ensMiscellaneoussetNewObj(
    const EnsPMiscellaneousset object);

EnsPMiscellaneousset ensMiscellaneoussetNewRef(EnsPMiscellaneousset ms);

void ensMiscellaneoussetDel(EnsPMiscellaneousset* Pms);

EnsPMiscellaneoussetadaptor ensMiscellaneoussetGetAdaptor(
    const EnsPMiscellaneousset ms);

ajuint ensMiscellaneoussetGetIdentifier(const EnsPMiscellaneousset ms);

AjPStr ensMiscellaneoussetGetCode(const EnsPMiscellaneousset ms);

AjPStr ensMiscellaneoussetGetName(const EnsPMiscellaneousset ms);

AjPStr ensMiscellaneoussetGetDescription(const EnsPMiscellaneousset ms);

ajuint ensMiscellaneoussetGetMaximumLength(const EnsPMiscellaneousset ms);

AjBool ensMiscellaneoussetSetAdaptor(EnsPMiscellaneousset ms,
			      EnsPMiscellaneoussetadaptor adaptor);

AjBool ensMiscellaneoussetSetIdentifier(EnsPMiscellaneousset ms,
                                        ajuint identifier);

AjBool ensMiscellaneoussetSetCode(EnsPMiscellaneousset ms, AjPStr code);

AjBool ensMiscellaneoussetSetName(EnsPMiscellaneousset ms, AjPStr name);

AjBool ensMiscellaneoussetSetDescription(EnsPMiscellaneousset ms,
                                         AjPStr description);

AjBool ensMiscellaneoussetSetMaximumLength(EnsPMiscellaneousset ms,
                                           ajuint maxlen);

ajuint ensMiscellaneoussetGetMemSize(const EnsPMiscellaneousset ms);

AjBool ensMiscellaneoussetTrace(const EnsPMiscellaneousset ms, ajuint level);

/* Ensembl Miscellaneous Set Adaptor */

EnsPMiscellaneoussetadaptor ensMiscellaneoussetadaptorNew(
    EnsPDatabaseadaptor dba);

void ensMiscellaneoussetadaptorDel(EnsPMiscellaneoussetadaptor* Padaptor);

const EnsPDatabaseadaptor ensMiscellaneoussetadaptorGetDatabaseadaptor(
    const EnsPMiscellaneoussetadaptor adaptor);

AjBool ensMiscellaneoussetadaptorFetchAll(
    const EnsPMiscellaneoussetadaptor adaptor,
				   AjPList mslist);

AjBool ensMiscellaneoussetadaptorFetchByIdentifier(
    EnsPMiscellaneoussetadaptor adaptor,
    ajuint identifier,
    EnsPMiscellaneousset *Pms);

AjBool ensMiscellaneoussetadaptorFetchByCode(
    EnsPMiscellaneoussetadaptor adaptor,
    const AjPStr code,
    EnsPMiscellaneousset *Pms);

/* Ensembl Miscellaneous Feature */

EnsPMiscellaneousfeature ensMiscellaneousfeatureNew(
    EnsPMiscellaneousfeatureadaptor adaptor,
    ajuint identifier,
    EnsPFeature feature);

EnsPMiscellaneousfeature ensMiscellaneousfeatureNewObj(
    const EnsPMiscellaneousfeature object);

EnsPMiscellaneousfeature ensMiscellaneousfeatureNewRef(
    EnsPMiscellaneousfeature mf);

void ensMiscellaneousfeatureDel(EnsPMiscellaneousfeature* Pmf);

EnsPMiscellaneousfeatureadaptor ensMiscellaneousfeatureGetAdaptor(
    const EnsPMiscellaneousfeature mf);

ajuint ensMiscellaneousfeatureGetIdentifier(const EnsPMiscellaneousfeature mf);

EnsPFeature ensMiscellaneousfeatureGetFeature(
    const EnsPMiscellaneousfeature mf);

const AjPList ensMiscellaneousfeatureGetAttributes(
    const EnsPMiscellaneousfeature mf);

const AjPList ensMiscellaneousfeatureGetMiscellaneoussets(
    const EnsPMiscellaneousfeature mf);

AjBool ensMiscellaneousfeatureSetAdaptor(
    EnsPMiscellaneousfeature mf,
    EnsPMiscellaneousfeatureadaptor adaptor);

AjBool ensMiscellaneousfeatureSetIdentifier(EnsPMiscellaneousfeature mf,
                                            ajuint identifier);

AjBool ensMiscellaneousfeatureSetFeature(EnsPMiscellaneousfeature mf,
                                         EnsPFeature feature);

AjBool ensMiscellaneousfeatureAddAttribute(EnsPMiscellaneousfeature mf,
                                           EnsPAttribute attribute);

AjBool ensMiscellaneousfeatureAddMiscellaneousset(EnsPMiscellaneousfeature mf,
                                                  EnsPMiscellaneousset ms);

AjBool ensMiscellaneousfeatureTrace(const EnsPMiscellaneousfeature mf,
                                    ajuint level);

ajuint ensMiscellaneousfeatureGetMemSize(const EnsPMiscellaneousfeature mf);

AjBool ensMiscellaneousfeatureFetchAllAttributes(EnsPMiscellaneousfeature mf,
                                                 const AjPStr code,
                                                 AjPList attributes);

AjBool ensMiscellaneousfeatureFetchAllMiscellaneoussets(
    EnsPMiscellaneousfeature mf,
    const AjPStr code,
    AjPList mslist);

/* Ensembl Miscellaneous Feature Adaptor */

EnsPMiscellaneousfeatureadaptor ensMiscellaneousfeatureadaptorNew(
    EnsPDatabaseadaptor dba);

void ensMiscellaneousfeatureadaptorDel(EnsPMiscellaneousfeatureadaptor *Pmfa);

AjBool ensMiscellaneousfeatureadaptorFetchAllBySliceAndSetCode(
    const EnsPMiscellaneousfeatureadaptor adaptor,
    EnsPSlice slice,
    const AjPList codes,
    AjPList mflist);

AjBool ensMiscellaneousfeatureadaptorFetchAllByAttributeCodeValue(
    const EnsPMiscellaneousfeatureadaptor adaptor,
    const AjPStr code,
    const AjPStr value,
    AjPList mflist);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif

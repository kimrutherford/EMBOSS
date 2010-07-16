#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ensvariation_h
#define ensvariation_h

#include "ensgvdata.h"
#include "ensgvsample.h"
#include "ensgvpopulation.h"




/* #data EnsPGvalleleadaptor **************************************************
**
** Ensembl Genetic Variation Allele Adaptor.
** Defined as an alias in EnsPDatabaseadaptor
**
** #alias EnsPDatabaseadaptor
** ##
** NOTE: Although the Bio::EnsEMBL::Variation::Allele object is based on the
** Bio::EnsEMBL::Storable object, there is actually no
** Bio::EnsEMBL::Variation::DBSQL::AlleleAdaptor as such in the
** Ensembl Variation API.
******************************************************************************/

#define EnsPGvalleleadaptor EnsPDatabaseadaptor




/* @data EnsPGvallele *********************************************************
**
** Ensembl Genetic Variation Allele.
**
** @alias EnsSGvallele
** @alias EnsOGvallele
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPGvalleleadaptor] Ensembl Genetic Variation
**                                     Allele Adaptor
** @cc Bio::EnsEMBL::Variation::Allele
** @attr Gvpopulation [EnsPGvpopulation] Ensembl Genetic Variation Population
** @attr Allele [AjPStr] Allele
** @attr Frequency [float] Frequency
** @attr SubSNPIdentifier [ajuint] Sub SNP Identifier
** @@
******************************************************************************/

typedef struct EnsSGvallele
{
    ajuint Use;
    ajuint Identifier;
    EnsPGvalleleadaptor Adaptor;
    EnsPGvpopulation Gvpopulation;
    AjPStr Allele;
    float Frequency;
    ajuint SubSNPIdentifier;
} EnsOGvallele;

#define EnsPGvallele EnsOGvallele*




/* #data EnsPGvgenotypeadaptor ************************************************
**
** Ensembl Genetic Variation Genotype Adaptor.
** Defined as an alias in EnsPDatabaseadaptor
**
** #alias EnsPDatabaseadaptor
** ##
******************************************************************************/

#define EnsPGvgenotypeadaptor EnsPDatabaseadaptor




/* @data EnsPGvgenotype *******************************************************
**
** Ensembl Genetic Variation Genotype.
**
** @alias EnsSGvgenotype
** @alias EnsOGvgenotype
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPGvgenotypeadaptor] Ensembl Genetic Variation
**                                       Genotype Adaptor
** @cc Bio::EnsEMBL::Variation::Genotype
** @attr Allele1 [AjPStr] Allele 1
** @attr Allele2 [AjPStr] Allele 2
** @@
******************************************************************************/

typedef struct EnsSGvgenotype
{
    ajuint Use;
    ajuint Identifier;
    EnsPGvgenotypeadaptor Adaptor;
    AjPStr Allele1;
    AjPStr Allele2;
} EnsOGvgenotype;

#define EnsPGvgenotype EnsOGvgenotype*




/* @data EnsPGvsourceadaptor **************************************************
**
** Ensembl Genetic Variation Source Adaptor.
**
** @alias EnsSGvsourceadaptor
** @alias EnsOGvsourceadaptor
**
** @attr Adaptor [EnsPBaseadaptor] Ensembl Base Adaptor
** @attr CacheByIdentifier [AjPTable] Identifier cache
** @attr CacheByName [AjPTable] Name cache
** @@
******************************************************************************/

typedef struct EnsSGvsourceadaptor {
    EnsPBaseadaptor Adaptor;
    AjPTable CacheByIdentifier;
    AjPTable CacheByName;
} EnsOGvsourceadaptor;

#define EnsPGvsourceadaptor EnsOGvsourceadaptor*




/* @data EnsPGvsource *********************************************************
**
** Ensembl Genetic Variation Source.
**
** @alias EnsSGvsource
** @alias EnsOGvsource
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPGvsourceadaptor] Ensembl Genetic Variation
**                                     Source Adaptor
** @cc Bio::EnsEMBL::Variation::??
** @attr Name [AjPStr] Name
** @attr Version [AjPStr] Version
** @attr Description [AjPStr] Description
** @attr URL [AjPStr] Uniform Resource Locator
** @@
******************************************************************************/

typedef struct EnsSGvsource
{
    ajuint Use;
    ajuint Identifier;
    EnsPGvsourceadaptor Adaptor;
    AjPStr Name;
    AjPStr Version;
    AjPStr Description;
    AjPStr URL;
} EnsOGvsource;

#define EnsPGvsource EnsOGvsource*




/* #data EnsPGvvariationadaptor ***********************************************
**
** Ensembl Genetic Variation Variation Adaptor.
** Defined as an alias in EnsPDatabaseadaptor
**
** #alias EnsPDatabaseadaptor
** ##
******************************************************************************/

#define EnsPGvvariationadaptor EnsPDatabaseadaptor




/******************************************************************************
**
** Ensembl Genetic Variation Variation validation state enumeration.
**
******************************************************************************/

typedef enum EnsOGvvariationValidationState
{
    ensEGvvariationValidationStateNULL,
    ensEGvvariationValidationStateCluster,
    ensEGvvariationValidationStateFrequency,
    ensEGvvariationValidationStateSubmitter,
    ensEGvvariationValidationStateDoublehit,
    ensEGvvariationValidationStateHapMap,
    ensEGvvariationValidationStateFailed,
    ensEGvvariationValidationStateNonPolymorphic,
    ensEGvvariationValidationStateObserved,
    ensEGvvariationValidationStateUnknown
} EnsEGvvariationValidationState;




/* @data EnsPGvvariation ******************************************************
**
** Ensembl Genetic Variation Variation.
**
** @alias EnsSGvvariation
** @alias EnsOGvvariation
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPGvvariationadaptor] Ensembl Genetic Variation
**                                        Variation Adaptor
** @cc Bio::EnsEMBL::Variation::Variation
** @attr Gvsource [EnsPGvsource] Ensembl Genetic Variation Source
** @attr Name [AjPStr] Name
** @attr Synonyms [AjPTable] Synonyms
** @attr Handles [AjPTable] Handles
** @attr AncestralAllele [AjPStr] Ancestral allele
** @attr Gvalleles [AjPList] AJAX List of Ensembl Genetic Variation Alleles
** @attr MoleculeType [AjPStr] Molecule type
** @attr FivePrimeFlank [AjPStr] Five prime flanking sequence
** @attr ThreePrimeFlank [AjPStr] Three prime flanking sequence
** @attr FailedDescription [AjPStr] Failed description
** @attr ValidationStates [ajuint] Bit field of validation states
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSGvvariation
{
    ajuint Use;
    ajuint Identifier;
    EnsPGvvariationadaptor Adaptor;
    EnsPGvsource Gvsource;
    AjPStr Name;
    AjPTable Synonyms;
    AjPTable Handles;
    AjPStr AncestralAllele;
    AjPList Gvalleles;
    AjPStr MoleculeType;
    AjPStr FivePrimeFlank;
    AjPStr ThreePrimeFlank;
    AjPStr FailedDescription;
    ajuint ValidationStates;
    ajuint Padding;
} EnsOGvvariation;

#define EnsPGvvariation EnsOGvvariation*




/* @data EnsPGvvariationfeatureadaptor ****************************************
**
** Ensembl Genetic Variation Variation Feature Adaptor.
**
** @alias EnsSGvvariationfeatureadaptor
** @alias EnsOGvvariationfeatureadaptor
**
** @attr Adaptor [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @@
******************************************************************************/

typedef struct EnsSGvvariationfeatureadaptor
{
    EnsPFeatureadaptor Adaptor;
} EnsOGvvariationfeatureadaptor;

#define EnsPGvvariationfeatureadaptor EnsOGvvariationfeatureadaptor*




/* @data EnsPGvvariationfeature ***********************************************
**
** Ensembl Genetic Variation Variation Feature.
**
** @alias EnsSGvvariationfeature
** @alias EnsOGvvariationfeature
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPGvvariationfeatureadaptor] Ensembl Genetic Variation
**                                               Genotype Adaptor
** @cc Bio::EnsEMBL::Feature
** @attr Feature [EnsPFeature] Ensembl Feature
** @cc Bio::EnsEMBL::Variation::VariationFeature
** @attr Gvvariation [EnsPGvvariation] Ensembl Variation
** @attr Name [AjPStr] Name
** @attr Source [AjPStr] Source
** @attr ValidationCode [AjPStr] Validation code
** @attr ConsequenceType [AjPStr] Consequence type
** @attr MapWeight [ajuint] Map weight or the number of times that the
**                          Variation associated with this Feature has hit
**                          the genome. If this was the only Feature associated
**                          with this Variation Feature the map weight would be
**                          1.
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSGvvariationfeature
{
    ajuint Use;
    ajuint Identifier;
    EnsPGvvariationfeatureadaptor Adaptor;
    EnsPFeature Feature;
    EnsPGvvariation Gvvariation;
    AjPStr Name;
    AjPStr Source;
    AjPStr ValidationCode;
    AjPStr ConsequenceType;
    ajuint MapWeight;
    ajuint Padding;
} EnsOGvvariationfeature;

#define EnsPGvvariationfeature EnsOGvvariationfeature*




/*
** Prototype definitions
*/

/* Ensembl Genetic Variation Allele */

EnsPGvallele ensGvalleleNew(EnsPGvalleleadaptor gvaa,
                            ajuint identifier,
                            EnsPGvpopulation gvp,
                            AjPStr allelestr,
                            float frequency,
                            ajuint subsnpid);

EnsPGvallele ensGvalleleNewObj(const EnsPGvallele object);

EnsPGvallele ensGvalleleNewRef(EnsPGvallele gva);

void ensGvalleleDel(EnsPGvallele* Pgva);

EnsPGvalleleadaptor ensGvalleleGetAdaptor(const EnsPGvallele gva);

ajuint ensGvalleleGetIdentifier(const EnsPGvallele gva);

EnsPGvpopulation ensGvalleleGetPopulation(const EnsPGvallele gva);

AjPStr ensGvalleleGetAllele(const EnsPGvallele gva);

float ensGvalleleGetFrequency(const EnsPGvallele gva);

ajuint ensGvalleleGetSubSNPIdentifier(const EnsPGvallele gva);

AjBool ensGvalleleSetAdaptor(EnsPGvallele gva, EnsPGvalleleadaptor gvaa);

AjBool ensGvalleleSetIdentifier(EnsPGvallele gva, ajuint identifier);

AjBool ensGvalleleSetPopulation(EnsPGvallele gva, EnsPGvpopulation gvp);

AjBool ensGvalleleSetAllele(EnsPGvallele gva, AjPStr allelestr);

AjBool ensGvalleleSetFrequency(EnsPGvallele gva, float frequency);

AjBool ensGvalleleSetSubSNPIdentifier(EnsPGvallele gva, ajuint subsnpid);

ajulong ensGvalleleGetMemsize(const EnsPGvallele gva);

AjBool ensGvalleleTrace(const EnsPGvallele gva, ajuint level);

/* Ensembl Genetic Variation Allele Adaptor */

EnsPGvalleleadaptor ensRegistryGetGvalleleadaptor(
    EnsPDatabaseadaptor dba);

/* Ensembl Genetic Variation Genotype */

EnsPGvgenotype ensGvgenotypeNew(EnsPGvgenotypeadaptor gvga,
                                ajuint identifier,
                                AjPStr allele1,
                                AjPStr allele2);

EnsPGvgenotype ensGvgenotypeNewObj(const EnsPGvgenotype object);

EnsPGvgenotype ensGvgenotypeNewRef(EnsPGvgenotype gvg);

void ensGvgenotypeDel(EnsPGvgenotype* Pgvg);

EnsPGvgenotypeadaptor ensGvgenotypeGetAdaptor(const EnsPGvgenotype gvg);

ajuint ensGvgenotypeGetIdentifier(const EnsPGvgenotype gvg);

AjPStr ensGvgenotypeGetAllele1(const EnsPGvgenotype gvg);

AjPStr ensGvgenotypeGetAllele2(const EnsPGvgenotype gvg);

AjBool ensGvgenotypeSetAdaptor(EnsPGvgenotype gvg, EnsPGvgenotypeadaptor gvga);

AjBool ensGvgenotypeSetIdentifier(EnsPGvgenotype gvg, ajuint identifier);

AjBool ensGvgenotypeSetAllele1(EnsPGvgenotype gvg, AjPStr allele1);

AjBool ensGvgenotypeSetAllele2(EnsPGvgenotype gvg, AjPStr allele2);

ajulong ensGvgenotypeGetMemsize(const EnsPGvgenotype gvg);

AjBool ensGvgenotypeTrace(const EnsPGvgenotype gvg, ajuint level);

/* Ensembl Genetic Variation Genotype Adaptor */

EnsPGvgenotypeadaptor ensRegistryGetGvgenotypeadaptor(
    EnsPDatabaseadaptor dba);

/* Ensembl Genetic Variation Source */

EnsPGvsource ensGvsourceNew(EnsPGvsourceadaptor gvsa,
                            ajuint identifier,
                            AjPStr name,
                            AjPStr version,
                            AjPStr description,
                            AjPStr url);

EnsPGvsource ensGvsourceNewObj(const EnsPGvsource object);

EnsPGvsource ensGvsourceNewRef(EnsPGvsource gvs);

void ensGvsourceDel(EnsPGvsource *Pgvs);

EnsPGvsourceadaptor ensGvsourceGetAdaptor(const EnsPGvsource gvs);

ajuint ensGvsourceGetIdentifier(const EnsPGvsource gvs);

AjPStr ensGvsourceGetName(const EnsPGvsource gvs);

AjPStr ensGvsourceGetVersion(const EnsPGvsource gvs);

AjPStr ensGvsourceGetDescription(const EnsPGvsource gvs);

AjPStr ensGvsourceGetURL(const EnsPGvsource gvs);

AjBool ensGvsourceSetAdaptor(EnsPGvsource gvs, EnsPGvsourceadaptor gvsa);

AjBool ensGvsourceSetIdentifier(EnsPGvsource gvs, ajuint identifier);

AjBool ensGvsourceSetName(EnsPGvsource gvs, AjPStr name);

AjBool ensGvsourceSetVersion(EnsPGvsource gvs, AjPStr version);

AjBool ensGvsourceSetDescription(EnsPGvsource gvs, AjPStr description);

AjBool ensGvsourceSetURL(EnsPGvsource gvs, AjPStr url);

ajulong ensGvsourceGetMemsize(const EnsPGvsource gvs);

AjBool ensGvsourceTrace(const EnsPGvsource gvs, ajuint level);

/* Ensembl Genetic Variation Source Adaptor */

EnsPGvsourceadaptor ensRegistryGetGvsourceadaptor(
    EnsPDatabaseadaptor dba);

EnsPGvsourceadaptor ensGvsourceadaptorNew(EnsPDatabaseadaptor dba);

void ensGvsourceadaptorDel(EnsPGvsourceadaptor* Pgvsa);

EnsPBaseadaptor ensGvsourceadaptorGetBaseadaptor(
    const EnsPGvsourceadaptor gvsa);

EnsPDatabaseadaptor ensGvsourceadaptorGetDatabaseadaptor(
    const EnsPGvsourceadaptor gvsa);

AjBool ensGvsourceadaptorFetchAll(EnsPGvsourceadaptor gvsa,
                                  AjPList gvss);

AjBool ensGvsourceadaptorFetchByIdentifier(EnsPGvsourceadaptor gvsa,
                                           ajuint identifier,
                                           EnsPGvsource *Pgvs);

AjBool ensGvsourceadaptorFetchByName(EnsPGvsourceadaptor gvsa,
                                     const AjPStr name,
                                     EnsPGvsource *Pgvs);

/* Ensembl Genetic Variation Variation */

EnsPGvvariation ensGvvariationNew(EnsPGvvariationadaptor gvva,
                                  ajuint identifier,
                                  EnsPGvsource gvs,
                                  AjPStr name,
                                  AjPStr ancestralallele,
                                  AjPTable synonyms,
                                  AjPList alleles,
                                  AjPStr validationstates,
                                  AjPStr moltype,
                                  AjPStr fiveflank,
                                  AjPStr threeflank,
                                  AjPStr faileddescription);

EnsPGvvariation ensGvvariationNewObj(const EnsPGvvariation object);

EnsPGvvariation ensGvvariationNewRef(EnsPGvvariation gvv);

void ensGvvariationDel(EnsPGvvariation* Pgvv);

EnsPGvvariationadaptor ensGvvariationGetAdaptor(const EnsPGvvariation gvv);

ajuint ensGvvariationGetIdentifier(const EnsPGvvariation gvv);

EnsPGvsource ensGvvariationGetGvsource(const EnsPGvvariation gvv);

AjPStr ensGvvariationGetName(const EnsPGvvariation gvv);

const AjPTable ensGvvariationGetSynonyms(const EnsPGvvariation gvv);

AjPStr ensGvvariationGetAncestralAllele(const EnsPGvvariation gvv);

const AjPList ensGvvariationGetGvalleles(const EnsPGvvariation gvv);

ajuint ensGvvariationGetValidationStates(const EnsPGvvariation gvv);

AjPStr ensGvvariationGetMoleculeType(const EnsPGvvariation gvv);

AjPStr ensGvvariationGetFivePrimeFlank(const EnsPGvvariation gvv);

AjPStr ensGvvariationGetThreePrimeFlank(const EnsPGvvariation gvv);

AjPStr ensGvvariationGetFailedDescription(const EnsPGvvariation gvv);

AjBool ensGvvariationSetAdaptor(EnsPGvvariation gvv,
                                EnsPGvvariationadaptor gvva);

AjBool ensGvvariationSetIdentifier(EnsPGvvariation gvv,
                                   ajuint identifier);

AjBool ensGvvariationSetGvsource(EnsPGvvariation gvv,
                                 EnsPGvsource gvs);

AjBool ensGvvariationSetName(EnsPGvvariation gvv,
                             AjPStr name);

AjBool ensGvvariationSetMoleculeType(EnsPGvvariation gvv,
                                     AjPStr moltype);

AjBool ensGvvariationSetAncestralAllele(EnsPGvvariation gvv,
                                        AjPStr ancestralallele);

AjBool ensGvvariationSetFivePrimeFlank(EnsPGvvariation gvv,
                                       AjPStr fiveflank);

AjBool ensGvvariationSetThreePrimeFlank(EnsPGvvariation gvv,
                                        AjPStr threeflank);

AjBool ensGvvariationSetFailedDescription(EnsPGvvariation gvv,
                                          AjPStr faileddescription);

ajulong ensGvvariationGetMemsize(const EnsPGvvariation gvv);

AjBool ensGvvariationTrace(const EnsPGvvariation gvv, ajuint level);

AjBool ensGvvariationAddSynonym(EnsPGvvariation gvv,
                                AjPStr source,
                                AjPStr synonym,
                                AjPStr handle);

AjBool ensGvvariationAddGvallele(EnsPGvvariation gvv, EnsPGvallele gva);

AjBool ensGvvariationAddValidationState(EnsPGvvariation gvv,
                                        EnsEGvvariationValidationState state);

EnsEGvvariationValidationState ensGvvariationValidationStateFromStr(
    const AjPStr state);

const char *ensGvvariationValidationStateToChar(
    EnsEGvvariationValidationState state);

ajuint ensGvvariationValidationStatesFromSet(const AjPStr set);

AjBool ensGvvariationValidationStatesToSet(ajuint states, AjPStr *Pstr);

AjBool ensGvvariationFetchAllSynonyms(const EnsPGvvariation gvv,
                                      const AjPStr source,
                                      AjPList synonyms);

AjBool ensGvvariationFetchAllSynonymSources(const EnsPGvvariation gvv,
                                            AjPList sources);

AjBool ensGvvariationFetchHandleBySynonym(EnsPGvvariation gvv,
                                          const AjPStr synonym,
                                          AjPStr *Phandle);

/* Ensembl Genetic Variation Variation Adaptor */

EnsPGvvariationadaptor ensRegistryGetGvvariationadaptor(
    EnsPDatabaseadaptor dba);

AjBool ensGvvariationadaptorFetchByIdentifier(EnsPGvvariationadaptor gvva,
                                              ajuint identifier,
                                              EnsPGvvariation *Pgvv);

AjBool ensGvvariationadaptorFetchByName(EnsPGvvariationadaptor gvva,
                                        const AjPStr name,
                                        const AjPStr source,
                                        EnsPGvvariation *Pgvv);

AjBool ensGvvariationadaptorFetchAllBySource(EnsPGvvariationadaptor gvva,
                                             const AjPStr source,
                                             AjBool primary,
                                             AjPList gvvs);

AjBool ensGvvariationadaptorFetchAllByGvpopulation(EnsPGvvariationadaptor gvva,
                                                   const EnsPGvpopulation gvp,
                                                   AjPList gvvs);

AjBool ensGvvariationadaptorFetchAllSources(EnsPGvvariationadaptor gvva,
                                            AjPList sources);

AjBool ensGvvariationadaptorFetchSourceVersion(EnsPGvvariationadaptor gvva,
                                               const AjPStr source,
                                               AjPStr *Pversion);

AjBool ensGvvariationadaptorFetchDefaultSource(EnsPGvvariationadaptor gvva,
                                               AjPStr *Psource);

AjBool ensGvvariationadaptorFetchFlankingSequence(EnsPGvvariationadaptor gvva,
                                                  ajuint variationid,
                                                  AjPStr *Pfiveseq,
                                                  AjPStr *Pthreeseq);

/* Ensembl Genetic Variation Variation Feature */

EnsPGvvariationfeature ensGvvariationfeatureNew(
    EnsPGvvariationfeatureadaptor adaptor,
    ajuint identifier,
    EnsPFeature feature,
    EnsPGvvariation gvv,
    AjPStr name,
    AjPStr source,
    AjPStr validation,
    AjPStr consequence,
    ajuint mapweight);

EnsPGvvariationfeature ensGvvariationfeatureNewObj(
    const EnsPGvvariationfeature object);

EnsPGvvariationfeature ensGvvariationfeatureNewRef(EnsPGvvariationfeature gvvf);

void ensGvvariationfeatureDel(EnsPGvvariationfeature *Pgvvf);

EnsPGvvariationfeatureadaptor ensGvvariationfeatureGetAdaptor(
    const EnsPGvvariationfeature gvvf);

ajuint ensGvvariationfeatureGetIdentifier(const EnsPGvvariationfeature gvvf);

EnsPFeature ensGvvariationfeatureGetFeature(const EnsPGvvariationfeature gvvf);

EnsPGvvariation ensGvvariationfeatureGetGvvariation(
    const EnsPGvvariationfeature gvvf);

AjPStr ensGvvariationfeatureGetName(const EnsPGvvariationfeature gvvf);

AjPStr ensGvvariationfeatureGetSource(const EnsPGvvariationfeature gvvf);

AjPStr ensGvvariationfeatureGetValidationCode(
    const EnsPGvvariationfeature gvvf);

AjPStr ensGvvariationfeatureGetConsequenceType(
    const EnsPGvvariationfeature gvvf);

ajuint ensGvvariationfeatureGetMapWeight(const EnsPGvvariationfeature gvvf);

AjBool ensGvvariationfeatureSetAdaptor(EnsPGvvariationfeature gvvf,
                                       EnsPGvvariationfeatureadaptor gvvfa);

AjBool ensGvvariationfeatureSetIdentifier(EnsPGvvariationfeature gvvf,
                                          ajuint identifier);

AjBool ensGvvariationfeatureSetFeature(EnsPGvvariationfeature gvvf,
                                       EnsPFeature feature);

AjBool ensGvvariationfeatureSetGvvariation(EnsPGvvariationfeature gvvf,
                                           EnsPGvvariation gvv);

AjBool ensGvvariationfeatureSetName(EnsPGvvariationfeature gvvf, AjPStr name);

AjBool ensGvvariationfeatureSetSource(EnsPGvvariationfeature gvvf,
                                      AjPStr source);

AjBool ensGvvariationfeatureSetValidationCode(EnsPGvvariationfeature gvvf,
                                              AjPStr validationcode);

AjBool ensGvvariationfeatureSetConsequenceType(EnsPGvvariationfeature gvvf,
                                               AjPStr consequencetype);

AjBool ensGvvariationfeatureSetMapWeight(EnsPGvvariationfeature gvvf,
                                         ajuint mapweight);

ajulong ensGvvariationfeatureGetMemsize(const EnsPGvvariationfeature gvvf);

AjBool ensGvvariationfeatureTrace(const EnsPGvvariationfeature gvvf,
                                  ajuint level);

AjBool ensGvvariationfeatureSortByStartAscending(AjPList gvvfs);

AjBool ensGvvariationfeatureSortByStartDescending(AjPList gvvfs);

/*
** End of prototype definitions
*/




#endif /* ensvariation_h */

#ifdef __cplusplus
}
#endif

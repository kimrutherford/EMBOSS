#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ensvariation_h
#define ensvariation_h

#include "ensgvdata.h"
#include "ensgvsample.h"
#include "ensgvpopulation.h"




/* @data EnsPGvalleleadaptor **************************************************
**
** Ensembl Genetic Variation Allele Adaptor.
**
** @alias EnsPDatabaseadaptor
** @@
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




/* @data EnsPGvgenotypeadaptor ************************************************
**
** Ensembl Genetic Variation Genotype Adaptor.
**
** @alias EnsPDatabaseadaptor
** @@
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




/* @data EnsPGvvariationadaptor ***********************************************
**
** Ensembl Genetic Variation Variation Adaptor.
**
** @alias EnsPDatabaseadaptor
** @@
******************************************************************************/

#define EnsPGvvariationadaptor EnsPDatabaseadaptor




/******************************************************************************
**
** Ensembl Genetic Variation Variation validation state enumeration.
**
******************************************************************************/

enum EnsEGvvariationValidationState
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
};




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
** @attr Name [AjPStr] Name
** @attr Source [AjPStr] Source
** @attr Synonyms [AjPTable] Synonyms
** @attr Handles [AjPTable] Handles
** @attr AncestralAllele [AjPStr] Ancestral allele
** @attr Gvalleles [AjPList] AJAX List of Ensembl Genetic Variation Alleles
** @attr ValidationStates [AjPList] AJAX List of AJAX Strings of validation
**                                  states
** @attr MoleculeType [AjPStr] Molecule type
** @attr FivePrimeFlank [AjPStr] Five prime flanking sequence
** @attr ThreePrimeFlank [AjPStr] Three prime flanking sequence
** @attr FailedDescription [AjPStr] Failed description
** @@
******************************************************************************/

typedef struct EnsSGvvariation
{
    ajuint Use;
    ajuint Identifier;
    EnsPGvvariationadaptor Adaptor;
    AjPStr Name;
    AjPStr Source;
    AjPTable Synonyms;
    AjPTable Handles;
    AjPStr AncestralAllele;
    AjPList Gvalleles;
    AjPList ValidationStates;
    AjPStr MoleculeType;
    AjPStr FivePrimeFlank;
    AjPStr ThreePrimeFlank;
    AjPStr FailedDescription;
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

ajuint ensGvalleleGetMemSize(const EnsPGvallele gva);

AjBool ensGvalleleTrace(const EnsPGvallele gva, ajuint level);

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

ajuint ensGvgenotypeGetMemSize(const EnsPGvgenotype gvg);

AjBool ensGvgenotypeTrace(const EnsPGvgenotype gvg, ajuint level);

/* Ensembl Genetic Variation Variation */

EnsPGvvariation ensGvvariationNew(EnsPGvvariationadaptor adaptor,
                                  ajuint identifier,
                                  AjPStr name,
                                  AjPStr source,
                                  AjPStr ancestralallele,
                                  AjPTable synonyms,
                                  AjPList alleles,
                                  AjPList validationstates,
                                  AjPStr moltype,
                                  AjPStr fiveflank,
                                  AjPStr threeflank,
                                  AjPStr faileddescription);

EnsPGvvariation ensGvvariationNewObj(const EnsPGvvariation object);

EnsPGvvariation ensGvvariationNewRef(EnsPGvvariation gvv);

void ensGvvariationDel(EnsPGvvariation* Pgvv);

EnsPGvvariationadaptor ensGvvariationGetAdaptor(const EnsPGvvariation gvv);

ajuint ensGvvariationGetIdentifier(const EnsPGvvariation gvv);

AjPStr ensGvvariationGetName(const EnsPGvvariation gvv);

AjPStr ensGvvariationGetSource(const EnsPGvvariation gvv);

const AjPTable ensGvvariationGetSynonyms(const EnsPGvvariation gvv);

AjPStr ensGvvariationGetAncestralAllele(const EnsPGvvariation gvv);

const AjPList ensGvvariationGetGvalleles(const EnsPGvvariation gvv);

const AjPList ensGvvariationGetValidationStates(const EnsPGvvariation gvv);

AjPStr ensGvvariationGetMoleculeType(const EnsPGvvariation gvv);

AjPStr ensGvvariationGetFivePrimeFlank(const EnsPGvvariation gvv);

AjPStr ensGvvariationGetThreePrimeFlank(const EnsPGvvariation gvv);

AjPStr ensGvvariationGetFailedDescription(const EnsPGvvariation gvv);

AjBool ensGvvariationSetAdaptor(EnsPGvvariation gvv,
                                EnsPGvvariationadaptor gvva);

AjBool ensGvvariationSetIdentifier(EnsPGvvariation gvv, ajuint identifier);

AjBool ensGvvariationSetName(EnsPGvvariation gvv, AjPStr name);

AjBool ensGvvariationSetSource(EnsPGvvariation gvv, AjPStr source);

AjBool ensGvvariationSetMoleculeType(EnsPGvvariation gvv, AjPStr moltype);

AjBool ensGvvariationSetAncestralAllele(EnsPGvvariation gvv,
                                        AjPStr ancestralallele);

AjBool ensGvvariationSetFivePrimeFlank(EnsPGvvariation gvv,
                                       AjPStr fiveflank);

AjBool ensGvvariationSetThreePrimeFlank(EnsPGvvariation gvv,
                                        AjPStr threeflank);

AjBool ensGvvariationSetFailedDescription(EnsPGvvariation gvv,
                                          AjPStr faileddescription);

ajuint ensGvvariationGetMemSize(const EnsPGvvariation gvv);

AjBool ensGvvariationTrace(const EnsPGvvariation gvv, ajuint level);

AjBool ensGvvariationAddSynonym(EnsPGvvariation gvv,
                                AjPStr source,
                                AjPStr synonym,
                                AjPStr handle);

AjBool ensGvvariationAddGvallele(EnsPGvvariation gvv, EnsPGvallele gva);

AjEnum ensGvvariationValidationStateFromStr(const AjPStr state);

const char *ensGvvariationValidationStateToChar(const AjEnum state);

AjBool ensGvvariationFetchAllSynonyms(const EnsPGvvariation gvv,
                                      const AjPStr source,
                                      AjPList synonyms);

AjBool ensGvvariationFetchAllSynonymSources(const EnsPGvvariation gvv,
                                            AjPList sources);

AjBool ensGvvariationFetchHandleBySynonym(EnsPGvvariation gvv,
                                          const AjPStr synonym,
                                          AjPStr *Phandle);

/* Ensembl Genetic Variation Variation Adaptor */

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

AjBool ensGvvariationAdaptorFetchAllByGvpopulation(EnsPGvvariationadaptor gvva,
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

ajuint ensGvvariationfeatureGetMemSize(const EnsPGvvariationfeature gvvf);

AjBool ensGvvariationfeatureTrace(const EnsPGvvariationfeature gvvf,
                                  ajuint level);

/*
** End of prototype definitions
*/




#endif

#ifdef __cplusplus
}
#endif

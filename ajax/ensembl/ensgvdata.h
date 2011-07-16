
#ifndef ENSGVDATA_H
#define ENSGVDATA_H

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ensfeature.h"

AJ_BEGIN_DECLS




/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */

/* @const EnsPGvgenotypeadaptor ***********************************************
**
** Ensembl Genetic Variation Genotype Adaptor.
** Defined as an alias in EnsPDatabaseadaptor
**
** #alias EnsPDatabaseadaptor
** ##
******************************************************************************/

#define EnsPGvgenotypeadaptor EnsPDatabaseadaptor




/* @const EnsPGvindividualadaptor *********************************************
**
** Ensembl Genetic Variation Individual Adaptor.
** Defined as an alias in EnsPBaseadaptor
**
** #alias EnsPBaseadaptor
** ##
******************************************************************************/

#define EnsPGvindividualadaptor EnsPBaseadaptor




/* @const EnsPGvpopulationadaptor *********************************************
**
** Ensembl Genetic Variation Population Adaptor.
** Defined as an alias in EnsPBaseadaptor
**
** #alias EnsPBaseadaptor
** ##
******************************************************************************/

#define EnsPGvpopulationadaptor EnsPBaseadaptor




/* @const EnsPGvpopulationgenotypeadaptor *************************************
**
** Ensembl Genetic Variation Population Genotype Adaptor.
** Defined as an alias in EnsPGvbaseadaptor
**
** #alias EnsPGvbaseadaptor
** ##
******************************************************************************/

#define EnsPGvpopulationgenotypeadaptor EnsPGvbaseadaptor




/* @const EnsPGvsampleadaptor *************************************************
**
** Ensembl Genetic Variation Sample Adaptor.
** Defined as an alias in EnsPBaseadaptor
**
** #alias EnsPBaseadaptor
** ##
******************************************************************************/

#define EnsPGvsampleadaptor EnsPBaseadaptor




/* @const EnsPGvsynonymadaptor ************************************************
**
** Ensembl Genetic Variation Synonym Adaptor.
** Defined as an alias in EnsPDatabaseadaptor
**
** #alias EnsPDatabaseadaptor
** ##
******************************************************************************/

#define EnsPGvsynonymadaptor EnsPDatabaseadaptor




/* @const EnsPGvvariationadaptor **********************************************
**
** Ensembl Genetic Variation Variation Adaptor.
** Defined as an alias in EnsPGvdatabaseadaptor
**
** #alias EnsPGvdatabaseadaptor
** ##
******************************************************************************/

#define EnsPGvvariationadaptor EnsPGvdatabaseadaptor




/* @const EnsPGvvariationsetadaptor *******************************************
**
** Ensembl Genetic Variation Variation Set Adaptor.
** Defined as an alias in EnsPBaseadaptor
**
** #alias EnsPBaseadaptor
** ##
******************************************************************************/

#define EnsPGvvariationsetadaptor EnsPBaseadaptor




/* @const EnsEGvindividualGender **********************************************
**
** Ensembl Genetic Variation Individual Gender enumeration.
**
******************************************************************************/

typedef enum EnsOGvindividualGender
{
    ensEGvindividualGenderNULL,
    ensEGvindividualGenderMale,
    ensEGvindividualGenderFemale,
    ensEGvindividualGenderUnknown
} EnsEGvindividualGender;




/* @const EnsEGvindividualType ************************************************
**
** Ensembl Genetic Variation Individual Type enumeration.
**
******************************************************************************/

typedef enum EnsOGvindividualType
{
    ensEGvindividualTypeNULL,
    ensEGvindividualTypeFullyInbred,
    ensEGvindividualTypePartlyInbred,
    ensEGvindividualTypeOutbred,
    ensEGvindividualTypeMutant,
    ensEGvindividualTypeUnknown
} EnsEGvindividualType;




/* @const EnsEGvsampleDisplay *************************************************
**
** Ensembl Genetic Variation Sample Display enumeration.
**
******************************************************************************/

typedef enum EnsOGvsampleDisplay
{
    ensEGvsampleDisplayNULL,
    ensEGvsampleDisplayReference,
    ensEGvsampleDisplayDefault,
    ensEGvsampleDisplayDisplayable,
    ensEGvsampleDisplayUndisplayable,
    ensEGvsampleDisplayLD
} EnsEGvsampleDisplay;




/* @const EnsEGvsourceSomatic *************************************************
**
** Ensembl Genetic Variation Source Somatic enumeration.
**
******************************************************************************/

typedef enum EnsOGvsourceSomatic
{
    ensEGvsourceSomaticNULL,
    ensEGvsourceSomaticGermline,
    ensEGvsourceSomaticSomatic,
    ensEGvsourceSomaticMixed
} EnsEGvsourceSomatic;




/* @const EnsEGvsourceType ****************************************************
**
** Ensembl Genetic Variation Source Type enumeration.
**
******************************************************************************/

typedef enum EnsOGvsourceType
{
    ensEGvsourceTypeNULL,
    ensEGvsourceTypeChip
} EnsEGvsourceType;




/* @const EnsEGvvariationClass ************************************************
**
** Ensembl Genetic Variation Variation Class enumeration.
**
******************************************************************************/

typedef enum EnsOGvvariationClass
{
    ensEGvvariationClassNULL,
    ensEGvvariationClassSO0001483, /* SNV */
    ensEGvvariationClassSO1000002, /* substitution */
    ensEGvvariationClassSO0000667, /* insertion */
    ensEGvvariationClassSO0000159, /* deletion */
    ensEGvvariationClassSO0000705, /* tandem_repeat */
    ensEGvvariationClassSO1000032, /* indel */
    ensEGvvariationClassSO0001059, /* sequence_alteration */
    ensEGvvariationClassSO0001019  /* copy_number_variation */
} EnsEGvvariationClass;




/* @const EnsEGvvariationValidation *******************************************
**
** Ensembl Genetic Variation Variation Validation enumeration.
**
******************************************************************************/

typedef enum EnsOGvvariationValidation
{
    ensEGvvariationValidationNULL,
    ensEGvvariationValidationCluster,
    ensEGvvariationValidationFrequency,
    ensEGvvariationValidationSubmitter,
    ensEGvvariationValidationDoublehit,
    ensEGvvariationValidationHapMap,
    ensEGvvariationValidation1000Genomes,
    ensEGvvariationValidationFailed,
    ensEGvvariationValidationPrecious
} EnsEGvvariationValidation;




/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */

/* @data EnsPGvdatabaseadaptor ************************************************
**
** Ensembl Genetic Variation Database Adaptor.
**
** @alias EnsSGvdatabaseadaptor
** @alias EnsOGvdatabaseadaptor
**
** @alias EnsPGvvariationadaptor
**
** @cc Bio::EnsEMBL::Variation::DBSQL::DBAdaptor
** @attr Adaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @attr Failedvariations [AjBool] Failed variation
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSGvdatabaseadaptor
{
    EnsPDatabaseadaptor Adaptor;
    AjBool Failedvariations;
    ajuint Padding;
} EnsOGvdatabaseadaptor;

#define EnsPGvdatabaseadaptor EnsOGvdatabaseadaptor*




/* @data EnsPGvbaseadaptor ****************************************************
**
** Ensembl Genetic Variation Base Adaptor.
**
** @alias EnsSGvbaseadaptor
** @alias EnsOGvbaseadaptor
**
** @alias EnsPGvpopulationgenotypeadaptor
** @alias EnsPGvtranscriptvariationadaptor
**
** @cc Bio::EnsEMBL::Variation::DBSQL::BaseAdaptor
** @attr Adaptor [EnsPGvdatabaseadaptor]
** Ensembl Genetic Variation Database Adaptor
** @attr Baseadaptor [EnsPBaseadaptor]
** Ensembl Base Adaptor
** @@
******************************************************************************/

typedef struct EnsSGvbaseadaptor
{
    EnsPGvdatabaseadaptor Adaptor;
    EnsPBaseadaptor Baseadaptor;
} EnsOGvbaseadaptor;

#define EnsPGvbaseadaptor EnsOGvbaseadaptor*




/* @data EnsPGvattributeadaptor ***********************************************
**
** Ensembl Genetic Variation Attribute Adaptor
**
** @alias EnsSGvattributeadaptor
** @alias EnsOGvattributeadaptor
**
** @cc Bio::EnsEMBL::Variation::DBSQL::AttributeAdaptor
** @attr Adaptor [EnsPBaseadaptor] Ensembl Base Adaptor
** @attr CacheByIdentifier [AjPTable] Identifier cache
** @attr CacheBySet [AjPVoid] Cache by attribute sets
** @@
******************************************************************************/

typedef struct EnsSGvattributeadaptor
{
    EnsPBaseadaptor Adaptor;
    AjPTable CacheByIdentifier;
    AjPVoid CacheBySet;
} EnsOGvattributeadaptor;

#define EnsPGvattributeadaptor EnsOGvattributeadaptor*




/* @data EnsPGvattribute ******************************************************
**
** Ensembl Genetic Variation Attribute
**
** @alias EnsSGvattribute
** @alias EnsOGvattribute
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPGvattributeadaptor]
** Ensembl Genetic Variation Attribute Adaptor
** @attr Attributetype [EnsPAttributetype] Ensembl Attribute Type
** @attr Value [AjPStr] Value
** @@
******************************************************************************/

typedef struct EnsSGvattribute
{
    ajuint Use;
    ajuint Identifier;
    EnsPGvattributeadaptor Adaptor;
    EnsPAttributetype Attributetype;
    AjPStr Value;
} EnsOGvattribute;

#define EnsPGvattribute EnsOGvattribute*




/* @data EnsPGvsample *********************************************************
**
** Ensembl Genetic Variation Sample.
**
** @alias EnsSGvsample
** @alias EnsOGvsample
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPGvsampleadaptor] Ensembl Genetic Variation
**                                     Sample Adaptor
** @cc Bio::EnsEMBL::Variation::Sample
** @attr Name [AjPStr] Name
** @attr Description [AjPStr] Description
** @attr Display [EnsEGvsampleDisplay] Display
** @attr Size [ajuint] Size
** @@
******************************************************************************/

typedef struct EnsSGvsample
{
    ajuint Use;
    ajuint Identifier;
    EnsPGvsampleadaptor Adaptor;
    AjPStr Name;
    AjPStr Description;
    EnsEGvsampleDisplay Display;
    ajuint Size;
} EnsOGvsample;

#define EnsPGvsample EnsOGvsample*




/* @data EnsPGvindividual *****************************************************
**
** Ensembl Genetic Variation Individual.
**
** @alias EnsSGvindividual
** @alias EnsOGvindividual
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPGvindividualadaptor] Ensembl Genetic Variation
**                                         Individual Adaptor
** @cc Bio::EnsEMBL::Variation::Sample
** @attr Gvsample [EnsPGvsample] Sample
** @cc Bio::EnsEMBL::Variation::Individual
** @attr Father [struct EnsSGvindividual*] Father Individual
** @attr Mother [struct EnsSGvindividual*] Mother Individual
** @attr Gender [EnsEGvindividualGender] Gender
** @attr Type [EnsEGvindividualType] Type
** @attr Description [AjPStr] Description
** @@
******************************************************************************/

typedef struct EnsSGvindividual
{
    ajuint Use;
    ajuint Identifier;
    EnsPGvindividualadaptor Adaptor;
    EnsPGvsample Gvsample;
    struct EnsSGvindividual* Father;
    struct EnsSGvindividual* Mother;
    EnsEGvindividualGender Gender;
    EnsEGvindividualType Type;
    AjPStr Description;
} EnsOGvindividual;

#define EnsPGvindividual EnsOGvindividual*




/* @data EnsPGvpopulation *****************************************************
**
** Ensembl Genetic Variation Population.
**
** @alias EnsSGvpopulation
** @alias EnsOGvpopulation
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPGvpopulationadaptor] Ensembl Genetic Variation
**                                         Population Adaptor
** @cc Bio::EnsEMBL::Variation::Sample
** @attr Gvsample [EnsPGvsample] Sample
** @cc Bio::EnsEMBL::Variation::Population
** @attr Subgvpopulations [AjPList] AJAX List of Ensembl Genetic Variation
**                                  (Sub-) Population objects
** @@
******************************************************************************/

typedef struct EnsSGvpopulation
{
    ajuint Use;
    ajuint Identifier;
    EnsPGvpopulationadaptor Adaptor;
    EnsPGvsample Gvsample;
    AjPList Subgvpopulations;
} EnsOGvpopulation;

#define EnsPGvpopulation EnsOGvpopulation*




/* @data EnsPGvalleleadaptor **************************************************
**
** Ensembl Genetic Variation Allele Adaptor.
**
** @alias EnsSGvalleleadaptor
** @alias EnsOGvalleleadaptor
**
** @cc Bio::EnsEMBL::Variation::DBSQL::AlleleAdaptor
** @attr Excludeadaptor [EnsPGvbaseadaptor]
** Ensembl Genetic Variation Base Adaptor excluding the failed_variation table
** @attr Includeadaptor [EnsPGvbaseadaptor]
** Ensembl Genetic Variation Base Adaptor including the failed_variation table
** @@
******************************************************************************/

typedef struct EnsSGvalleleadaptor
{
    EnsPGvbaseadaptor Excludeadaptor;
    EnsPGvbaseadaptor Includeadaptor;
} EnsOGvalleleadaptor;

#define EnsPGvalleleadaptor EnsOGvalleleadaptor*




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
** @attr SubSNPHandle [AjPStr] Sub SNP handle
** @attr Faileddescriptions [AjPList]
** AJAX List of AJAX String (failed description) objects
** @attr Counter [ajuint] Counter
** @attr Gvvariationidentifier [ajuint]
** Ensembl Genetic Variation Variation identifier
** @attr SubSNPIdentifier [ajuint] Sub SNP Identifier
** @attr Frequency [float] Frequency
** @@
******************************************************************************/

typedef struct EnsSGvallele
{
    ajuint Use;
    ajuint Identifier;
    EnsPGvalleleadaptor Adaptor;
    EnsPGvpopulation Gvpopulation;
    AjPStr Allele;
    AjPStr SubSNPHandle;
    AjPList Faileddescriptions;
    ajuint Counter;
    ajuint Gvvariationidentifier;
    ajuint SubSNPIdentifier;
    float Frequency;
} EnsOGvallele;

#define EnsPGvallele EnsOGvallele*




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
** @attr DefaultGvsource [void*] Default Ensembl Genetic Variation Source
** @@
******************************************************************************/

typedef struct EnsSGvsourceadaptor
{
    EnsPBaseadaptor Adaptor;
    AjPTable CacheByIdentifier;
    AjPTable CacheByName;
    void* DefaultGvsource;
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
** @cc Bio::EnsEMBL::Variation::???
** @attr Name [AjPStr] Name
** @attr Version [AjPStr] Version
** @attr Description [AjPStr] Description
** @attr URL [AjPStr] Uniform Resource Locator
** @attr Somatic [EnsEGvsourceSomatic]
** Ensembl Genetic Variation Source Somatic enumeration
** @attr Type [EnsEGvsourceType]
** Ensembl Genetic Variation Source Type enumeration
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
    EnsEGvsourceSomatic Somatic;
    EnsEGvsourceType Type;
} EnsOGvsource;

#define EnsPGvsource EnsOGvsource*




/* @data EnsPGvsynonym ********************************************************
**
** Ensembl Genetic Variation Synonym.
**
** @alias EnsSGvsynonym
** @alias EnsOGvsynonym
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPGvsynonymadaptor] Ensembl Genetic Variation
**                                      Synonym Adaptor
** @cc Bio::EnsEMBL::Variation::??
** @attr Gvsource [EnsPGvsource] Ensembl Genetic Variation Source
** @attr Name [AjPStr] Name
** @attr Moleculetype [AjPStr] Molecule type
** @attr Gvvariationidentifier [ajuint] Ensembl Genetic Variation identifier
** @attr Subidentifier [ajuint] Sub Identifier
** @@
******************************************************************************/

typedef struct EnsSGvsynonym
{
    ajuint Use;
    ajuint Identifier;
    EnsPGvsynonymadaptor Adaptor;
    EnsPGvsource Gvsource;
    AjPStr Name;
    AjPStr Moleculetype;
    ajuint Gvvariationidentifier;
    ajuint Subidentifier;
} EnsOGvsynonym;

#define EnsPGvsynonym EnsOGvsynonym*




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
** @attr Class [EnsPGvattribute]
** Class Ensembl Genetic Variation Attribute
** @attr Gvsource [EnsPGvsource] Ensembl Genetic Variation Source
** @attr Name [AjPStr] Name
** @attr Handles [AjPTable] Handles
** @attr Ancestralallele [AjPStr] Ancestral allele
** @attr Gvalleles [AjPList] AJAX List of
** Ensembl Genetic Variation Allele objects
** @attr Gvsynonyms [AjPList] AJAX List of
** Ensembl Genetic Variation Synonym objects
** @attr Faileddescriptions [AjPList] AJAX List of
** AJAX String (failed description) objects
** @attr Moleculetype [AjPStr] Molecule type
** @attr FlankFive [AjPStr] Five-prime flanking sequence
** @attr FlankThree [AjPStr] Three-prime flanking sequence
** @attr FlankExists [AjBool] A five-prime or three-prime flank exists
** @attr Somatic [AjBool] Somatic or germline flag
** @attr Validations [ajuint] Bit field of validation states
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSGvvariation
{
    ajuint Use;
    ajuint Identifier;
    EnsPGvvariationadaptor Adaptor;
    EnsPGvattribute Class;
    EnsPGvsource Gvsource;
    AjPStr Name;
    AjPTable Handles;
    AjPStr Ancestralallele;
    AjPList Gvalleles;
    AjPList Gvsynonyms;
    AjPList Faileddescriptions;
    AjPStr Moleculetype;
    AjPStr FlankFive;
    AjPStr FlankThree;
    AjBool FlankExists;
    AjBool Somatic;
    ajuint Validations;
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
** @attr Adaptor [EnsPGvdatabaseadaptor]
** Ensembl Genetic Variation Database Adaptor
** @attr Featureadaptor [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @@
******************************************************************************/

typedef struct EnsSGvvariationfeatureadaptor
{
    EnsPGvdatabaseadaptor Adaptor;
    EnsPFeatureadaptor Featureadaptor;
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
** @attr Gvsource [EnsPGvsource] Ensembl Genetic Variation Source
** @attr Gvvariation [EnsPGvvariation] Ensembl Genetic Variation Variation
** @attr Allele [AjPStr] Allele, denormalisation from allele.allele
** @attr Name [AjPStr] Name, denormalisation from variation.name
** @attr Validationcode [AjPStr] Validation code
** @attr Consequencetype [AjPStr] Consequence type
** @attr Mapweight [ajuint] Map weight or the number of times that the
**                          Variation associated with this Feature has hit
**                          the genome. If this was the only Feature associated
**                          with this Variation Feature the map weight would be
**                          1.
** @attr Gvvariationidentifier [ajuint] Ensembl Genetic Variation Variation
**                                      identifier
** @attr GvvariationClass [EnsEGvvariationClass]
** Ensembl Genetic Variation Variation Class enumeration
** @attr Padding [ajuint] Padding to alignment boundary
** @cc The somatic member is part of the EnsSGvsource data structure
** @@
******************************************************************************/

typedef struct EnsSGvvariationfeature
{
    ajuint Use;
    ajuint Identifier;
    EnsPGvvariationfeatureadaptor Adaptor;
    EnsPFeature Feature;
    EnsPGvsource Gvsource;
    EnsPGvvariation Gvvariation;
    AjPStr Allele;
    AjPStr Name;
    AjPStr Validationcode;
    AjPStr Consequencetype;
    ajuint Mapweight;
    ajuint Gvvariationidentifier;
    EnsEGvvariationClass GvvariationClass;
    ajuint Padding;
} EnsOGvvariationfeature;

#define EnsPGvvariationfeature EnsOGvvariationfeature*




/* @data EnsPGvvariationset ***************************************************
**
** Ensembl Genetic Variation Variation Set.
**
** @alias EnsSGvvariationset
** @alias EnsOGvvariationset
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPGvvariationsetadaptor] Ensembl Genetic Variation
**                                           Set Adaptor
** @cc Bio::EnsEMBL::Variation::VariationSet
** @attr Name [AjPStr] Name
** @attr Description [AjPStr] Description
** @@
******************************************************************************/

typedef struct EnsSGvvariationset
{
    ajuint Use;
    ajuint Identifier;
    EnsPGvvariationsetadaptor Adaptor;
    AjPStr Name;
    AjPStr Description;
} EnsOGvvariationset;

#define EnsPGvvariationset EnsOGvvariationset*




/* @data EnsPGvpopulationgenotype *********************************************
**
** Ensembl Genetic Variation Population Genotype.
**
** @alias EnsSGvpopulationgenotype
** @alias EnsOGvpopulationgenotype
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPGvpopulationgenotypeadaptor] Ensembl Genetic
**                                       Variation Population Genotype Adaptor
** @cc Bio::EnsEMBL::Variation::PopulationGenotype
** @attr Gvpopulation [EnsPGvpopulation] Ensembl Genetic Variation Population
** @attr Gvvariation [EnsPGvvariation] Ensembl Genetic Variation Variation
** @attr Allele1 [AjPStr] Allele1
** @attr Allele2 [AjPStr] Allele2
** @attr Counter [ajuint] Counter
** @attr Subidentifier [ajuint] Sub-Identifier
** @attr Frequency [float] Frequency
** @@
******************************************************************************/

typedef struct EnsSGvpopulationgenotype
{
    ajuint Use;
    ajuint Identifier;
    EnsPGvpopulationgenotypeadaptor Adaptor;
    EnsPGvpopulation Gvpopulation;
    EnsPGvvariation Gvvariation;
    AjPStr Allele1;
    AjPStr Allele2;
    ajuint Counter;
    ajuint Subidentifier;
    float Frequency;
} EnsOGvpopulationgenotype;

#define EnsPGvpopulationgenotype EnsOGvpopulationgenotype*




/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */




AJ_END_DECLS

#endif /* !ENSGVDATA_H */

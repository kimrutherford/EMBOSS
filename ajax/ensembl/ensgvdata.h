/* @include ensgvdata *********************************************************
**
** Ensembl Genetic Variation Data functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.22 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2013/02/17 13:06:34 $ by $Author: mks $
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

#ifndef ENSGVDATA_H
#define ENSGVDATA_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ensfeature.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */

/* @const EnsPGvgenotypeadaptor ***********************************************
**
** Ensembl Genetic Variation Genotype Adaptor.
** Defined as an alias in EnsPDatabaseadaptor
**
** #alias EnsPDatabaseadaptor
** ##
******************************************************************************/

#define EnsPGvgenotypeadaptor EnsPDatabaseadaptor




/* @const EnsPGvgenotypecodeadaptor *******************************************
**
** Ensembl Genetic Variation Genotype Code Adaptor.
** Defined as an alias in EnsPGvbaseadaptor
**
** #alias EnsPGvbaseadaptor
** ##
******************************************************************************/

#define EnsPGvgenotypecodeadaptor EnsPGvbaseadaptor




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




/* @const EnsPGvvariationsetadaptor *******************************************
**
** Ensembl Genetic Variation Variation Set Adaptor.
** Defined as an alias in EnsPBaseadaptor
**
** #alias EnsPBaseadaptor
** ##
******************************************************************************/

#define EnsPGvvariationsetadaptor EnsPBaseadaptor




/* @enum EnsEGvindividualGender ***********************************************
**
** Ensembl Genetic Variation Individual Gender enumeration
**
** @value ensEGvindividualGenderNULL Null
** @value ensEGvindividualGenderMale Male
** @value ensEGvindividualGenderFemale Female
** @value ensEGvindividualGenderUnknown Unknown
** @@
******************************************************************************/

typedef enum EnsOGvindividualGender
{
    ensEGvindividualGenderNULL,
    ensEGvindividualGenderMale,
    ensEGvindividualGenderFemale,
    ensEGvindividualGenderUnknown
} EnsEGvindividualGender;




/* @enum EnsEGvindividualType *************************************************
**
** Ensembl Genetic Variation Individual Type enumeration
**
** @value ensEGvindividualTypeNULL Null
** @value ensEGvindividualTypeFullyInbred Fully inbred
** @value ensEGvindividualTypePartlyInbred Partly inbred
** @value ensEGvindividualTypeOutbred Outbred
** @value ensEGvindividualTypeMutant Mutant
** @value ensEGvindividualTypeUnknown Unknown
** @@
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




/* @enum EnsEGvsampleDisplay **************************************************
**
** Ensembl Genetic Variation Sample Display enumeration
**
** @value ensEGvsampleDisplayNULL Null
** @value ensEGvsampleDisplayReference Reference
** @value ensEGvsampleDisplayDefault Default
** @value ensEGvsampleDisplayDisplayable Displayable
** @value ensEGvsampleDisplayUndisplayable Undisplayable
** @value ensEGvsampleDisplayLD Linkage Disequilibrium
** @value ensEGvsampleMartdisplayable Displayable in BioMart
** @@
******************************************************************************/

typedef enum EnsOGvsampleDisplay
{
    ensEGvsampleDisplayNULL,
    ensEGvsampleDisplayReference,
    ensEGvsampleDisplayDefault,
    ensEGvsampleDisplayDisplayable,
    ensEGvsampleDisplayUndisplayable,
    ensEGvsampleDisplayLD,
    ensEGvsampleMartdisplayable
} EnsEGvsampleDisplay;




/* @enum EnsEGvsourceSomatic **************************************************
**
** Ensembl Genetic Variation Source Somatic enumeration
**
** @value ensEGvsourceSomaticNULL Null
** @value ensEGvsourceSomaticGermline Germline
** @value ensEGvsourceSomaticSomatic Somatic
** @value ensEGvsourceSomaticMixed Mixed
** @@
******************************************************************************/

typedef enum EnsOGvsourceSomatic
{
    ensEGvsourceSomaticNULL,
    ensEGvsourceSomaticGermline,
    ensEGvsourceSomaticSomatic,
    ensEGvsourceSomaticMixed
} EnsEGvsourceSomatic;




/* @enum EnsEGvsourceType *****************************************************
**
** Ensembl Genetic Variation Source Type enumeration
**
** @value ensEGvsourceTypeNULL Null
** @value ensEGvsourceTypeChip Chip
** @@
******************************************************************************/

typedef enum EnsOGvsourceType
{
    ensEGvsourceTypeNULL,
    ensEGvsourceTypeChip
} EnsEGvsourceType;




/* @enum EnsEGvvariationClass *************************************************
**
** Ensembl Genetic Variation Variation Class enumeration
**
** @value ensEGvvariationClassNULL      Null
** @value ensEGvvariationClassSO0001483 SNV
** @value ensEGvvariationClassSO1000002 substitution
** @value ensEGvvariationClassSO0000667 insertion
** @value ensEGvvariationClassSO0000159 deletion
** @value ensEGvvariationClassSO0000705 tandem_repeat
** @value ensEGvvariationClassSO1000032 indel
** @value ensEGvvariationClassSO0001059 sequence_alteration
** @value ensEGvvariationClassSO0001019 copy_number_variation
** @@
******************************************************************************/

typedef enum EnsOGvvariationClass
{
    ensEGvvariationClassNULL,
    ensEGvvariationClassSO0001483,
    ensEGvvariationClassSO1000002,
    ensEGvvariationClassSO0000667,
    ensEGvvariationClassSO0000159,
    ensEGvvariationClassSO0000705,
    ensEGvvariationClassSO1000032,
    ensEGvvariationClassSO0001059,
    ensEGvvariationClassSO0001019
} EnsEGvvariationClass;




/* @enum EnsEGvvariationValidation ********************************************
**
** Ensembl Genetic Variation Variation Validation enumeration
**
** @value ensEGvvariationValidationNULL Null
** @value ensEGvvariationValidationCluster Cluster
** @value ensEGvvariationValidationFrequency Frequency
** @value ensEGvvariationValidationSubmitter Submitter
** @value ensEGvvariationValidationDoublehit Double-hit
** @value ensEGvvariationValidationHapMap HAP Map Project
** @value ensEGvvariationValidation1000Genomes 1000 Genomes Project
** @value ensEGvvariationValidationFailed Failed
** @value ensEGvvariationValidationPrecious Precious
** @@
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




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */

struct EnsSGvsource;




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




/* @data EnsPGvbaseadaptorLeftjoin ********************************************
**
** Ensembl Genetic Variation Base Adaptor SQL LEFT JOIN condition
**
** @alias EnsSGvbaseadaptorLeftjoin
** @alias EnsOGvbaseadaptorLeftjoin
**
** @attr Tablename [char*] SQL table name
** @attr Condition [char*] SQL LEFT JOIN condition
** @@
** FIXME: Could this just become an alias of EnsPBaseadaptorLeftjoin
******************************************************************************/

typedef struct EnsSGvbaseadaptorLeftjoin
{
    char *Tablename;
    char *Condition;
} EnsOGvbaseadaptorLeftjoin;

#define EnsPGvbaseadaptorLeftjoin EnsOGvbaseadaptorLeftjoin*




/* @data EnsPGvbaseadaptor ****************************************************
**
** Ensembl Genetic Variation Base Adaptor.
**
** @alias EnsSGvbaseadaptor
** @alias EnsOGvbaseadaptor
**
** @alias EnsPGvgenotypecodeadaptor
** @alias EnsPGvpopulationgenotypeadaptor
** @alias EnsPGvtranscriptvariationadaptor
**
** @cc Bio::EnsEMBL::Variation::DBSQL::BaseAdaptor
** @attr Adaptor [EnsPGvdatabaseadaptor]
** Ensembl Genetic Variation Database Adaptor
** @attr Baseadaptor [EnsPBaseadaptor]
** Ensembl Base Adaptor
** @attr Ploidy [ajuint] Ploidy
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSGvbaseadaptor
{
    EnsPGvdatabaseadaptor Adaptor;
    EnsPBaseadaptor Baseadaptor;
    ajuint Ploidy;
    ajuint Padding;
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
** @attr Adaptor [EnsPGvsampleadaptor]
** Ensembl Genetic Variation Sample Adaptor
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
** @attr Adaptor [EnsPGvindividualadaptor]
** Ensembl Genetic Variation Individual Adaptor
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
    struct EnsSGvindividual *Father;
    struct EnsSGvindividual *Mother;
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
** @attr Adaptor [EnsPGvpopulationadaptor]
** Ensembl Genetic Variation Population Adaptor
** @cc Bio::EnsEMBL::Variation::Sample
** @attr Gvsample [EnsPGvsample] Sample
** @cc Bio::EnsEMBL::Variation::Population
** @attr Subgvpopulations [AjPList]
** AJAX List of (Sub-) Ensembl Genetic Variation Population objects
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
** @attr Subhandle [AjPStr] Subhandle
** @attr Faileddescriptions [AjPList]
** AJAX List of AJAX String (failed description) objects
** @attr Counter [ajuint] Counter
** @attr Gvvariationidentifier [ajuint]
** Ensembl Genetic Variation Variation identifier
** @attr Subidentifier [ajuint] Subidentifier
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
    AjPStr Subhandle;
    AjPList Faileddescriptions;
    ajuint Counter;
    ajuint Gvvariationidentifier;
    ajuint Subidentifier;
    float Frequency;
} EnsOGvallele;

#define EnsPGvallele EnsOGvallele*




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
** @attr DefaultGvsource [struct EnsSGvsource*]
** Default Ensembl Genetic Variation Source
** @@
******************************************************************************/

typedef struct EnsSGvsourceadaptor
{
    EnsPBaseadaptor Adaptor;
    AjPTable CacheByIdentifier;
    AjPTable CacheByName;
    struct EnsSGvsource *DefaultGvsource;
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
** @attr Subidentifier [ajuint] Subidentifier
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




/* @data EnsPGvvariationadaptor ***********************************************
**
** Ensembl Genetic Variation Variation Adaptor.
**
** @alias EnsOGvvariationadaptor
** @alias EnsSGvvariationadaptor
**
** @attr Adaptor [EnsPGvbaseadaptor] Ensembl Genetic Variation Base Adaptor
** @attr Tablenames [char**] SQL table name array
** @attr Columnnames [char**] SQL column name array
** @attr Condition [char*] SQL SELECT default condition,
**                         which is dynamically assigned.
** @attr Leftjoins [EnsPGvbaseadaptorLeftjoin] SQL LEFT JOIN conditions
** @attr Defaultcondition [char*] SQL SELECT default condition
** @@
******************************************************************************/

typedef struct EnsSGvvariationadaptor
{
    EnsPGvbaseadaptor Adaptor;
    char **Tablenames;
    char **Columnnames;
    char *Condition;
    EnsPGvbaseadaptorLeftjoin Leftjoins;
    char *Defaultcondition;
} EnsOGvvariationadaptor;

#define EnsPGvvariationadaptor EnsOGvvariationadaptor*




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
** @attr Clinical [EnsPGvattribute]
** Clinical Significance Ensembl Genetic Variation Attribute
** @attr Gvsource [EnsPGvsource] Ensembl Genetic Variation Source
** @attr Name [AjPStr] Name
** @attr Handles [AjPTable] Handles
** @attr Ancestralallele [AjPStr] Ancestral allele
** @attr MinoralleleAllele [AjPStr] Minor allele allele
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
** @attr Flipped [AjBool] Flipped
** @attr Somatic [AjBool] Somatic or germline flag
** @attr Validations [ajuint] Bit field of validation states
** @attr MinoralleleCount [ajuint] Minor allele count
** @attr MinoralleleFrequency [float] Minor allele frequency
** @@
******************************************************************************/

typedef struct EnsSGvvariation
{
    ajuint Use;
    ajuint Identifier;
    EnsPGvvariationadaptor Adaptor;
    EnsPGvattribute Class;
    EnsPGvattribute Clinical;
    EnsPGvsource Gvsource;
    AjPStr Name;
    AjPTable Handles;
    AjPStr Ancestralallele;
    AjPStr MinoralleleAllele;
    AjPList Gvalleles;
    AjPList Gvsynonyms;
    AjPList Faileddescriptions;
    AjPStr Moleculetype;
    AjPStr FlankFive;
    AjPStr FlankThree;
    AjBool FlankExists;
    AjBool Flipped;
    AjBool Somatic;
    ajuint Validations;
    ajuint MinoralleleCount;
    float MinoralleleFrequency;
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
** @attr Shortname [AjPStr] Short name
** @@
******************************************************************************/

typedef struct EnsSGvvariationset
{
    ajuint Use;
    ajuint Identifier;
    EnsPGvvariationsetadaptor Adaptor;
    AjPStr Name;
    AjPStr Description;
    AjPStr Shortname;
} EnsOGvvariationset;

#define EnsPGvvariationset EnsOGvvariationset*




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
** @attr Gvvariation [EnsPGvvariation] Ensembl Genetic Variation Variation
** @attr Alleles [AjPList] AJAX List of AJAX String objects
** @attr Subhandle [AjPStr] Subhandle
** @attr Subidentifier [ajuint] Subidentifier
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSGvgenotype
{
    ajuint Use;
    ajuint Identifier;
    EnsPGvgenotypeadaptor Adaptor;
    EnsPGvvariation Gvvariation;
    AjPList Alleles;
    AjPStr Subhandle;
    ajuint Subidentifier;
    ajuint Padding;
} EnsOGvgenotype;

#define EnsPGvgenotype EnsOGvgenotype*




/* @data EnsPGvgenotypecode ***************************************************
**
** Ensembl Genetic Variation Genotype Code.
**
** @alias EnsSGvgenotypecode
** @alias EnsOGvgenotypecode
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPGvgenotypecodeadaptor]
** Ensembl Genetic Variation Genotype Code Adaptor
** @cc Bio::EnsEMBL::Variation::GenotypeCode
** @attr Alleles [AjPList] AJAX List of AJAX String (allele) objects
** @@
******************************************************************************/

typedef struct EnsSGvgenotypecode
{
    ajuint Use;
    ajuint Identifier;
    EnsPGvgenotypecodeadaptor Adaptor;
    AjPList Alleles;
} EnsOGvgenotypecode;

#define EnsPGvgenotypecode EnsOGvgenotypecode*




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
** @attr Gvgenotype [EnsPGvgenotype] Ensembl Genetic Variation Genotype
** @attr Counter [ajuint] Counter
** @attr Frequency [float] Frequency
** @@
******************************************************************************/

typedef struct EnsSGvpopulationgenotype
{
    ajuint Use;
    ajuint Identifier;
    EnsPGvpopulationgenotypeadaptor Adaptor;
    EnsPGvpopulation Gvpopulation;
    EnsPGvgenotype Gvgenotype;
    ajuint Counter;
    float Frequency;
} EnsOGvpopulationgenotype;

#define EnsPGvpopulationgenotype EnsOGvpopulationgenotype*




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */

/*
** Prototype definitions
*/

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSGVDATA_H */

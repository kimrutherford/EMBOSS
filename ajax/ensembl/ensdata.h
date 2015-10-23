/* @include ensdata ***********************************************************
**
** Ensembl Data functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.53 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2013/02/17 13:04:31 $ by $Author: mks $
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

#ifndef ENSDATA_H
#define ENSDATA_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "enscache.h"
#include "enscoordsystem.h"
#include "ensmapper.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */

/* @const EnsPAttributeadaptor ************************************************
**
** Ensembl Attribute Adaptor
**
** Defined as an alias in EnsPDatabaseadaptor.
**
** #alias EnsPDatabaseadaptor
**
** #cc Bio::EnsEMBL::DBSQL::AttributeAdaptor
** ##
******************************************************************************/

#define EnsPAttributeadaptor EnsPDatabaseadaptor




/* @const EnsPDatabaseentryadaptor ********************************************
**
** Ensembl Database Entry Adaptor
**
** Defined as an alias in EnsPDatabaseadaptor.
**
** #alias EnsPDatabaseadaptor
**
** #cc Bio::EnsEMBL::DBSQL::DBEntryAdaptor
** ##
******************************************************************************/

#define EnsPDatabaseentryadaptor EnsPDatabaseadaptor




/* @const EnsPGeneadaptor *****************************************************
**
** Ensembl Gene Adaptor
**
** Defined as an alias in EnsPFeatureadaptor.
**
** #alias EnsPFeatureadaptor
**
** #cc Bio::EnsEMBL::DBSQL::GeneAdaptor
** ##
******************************************************************************/

#define EnsPGeneadaptor EnsPFeatureadaptor




/* @const EnsPIntronsupportingevidenceadaptor *********************************
**
** Ensembl Intron Supporting Evidence Adaptor
**
** Defined as an alias in EnsPFeatureadaptor.
**
** #alias EnsPFeatureadaptor
**
** #cc Bio::EnsEMBL::DBSQL::IntronsupportingevidenceAdaptor
** ##
******************************************************************************/

#define EnsPIntronsupportingevidenceadaptor EnsPFeatureadaptor




/* @const EnsPOperonadaptor ***************************************************
**
** Ensembl Operon Adaptor
**
** Defined as an alias in EnsPFeatureadaptor.
**
** #alias EnsPFeatureadaptor
**
** #cc Bio::EnsEMBL::DBSQL::OperonAdaptor
** ##
******************************************************************************/

#define EnsPOperonadaptor EnsPFeatureadaptor




/* @const EnsPOperontranscriptadaptor *****************************************
**
** Ensembl Operon Transcript Adaptor
**
** Defined as an alias in EnsPFeatureadaptor.
**
** #alias EnsPFeatureadaptor
**
** #cc Bio::EnsEMBL::DBSQL::OperonTranscriptAdaptor
** ##
******************************************************************************/

#define EnsPOperontranscriptadaptor EnsPFeatureadaptor




/* @const EnsPSeqregionsynonymadaptor *****************************************
**
** Ensembl Sequence Region Synonym Adaptor
**
** Defined as an alias in EnsPBaseadaptor.
**
** #alias EnsPBaseadaptor
**
** #cc Bio::EnsEMBL::DBSQL::SeqRegionSynonymAdaptor
** ##
******************************************************************************/

#define EnsPSeqregionsynonymadaptor EnsPBaseadaptor




/* @const EnsPTranscriptadaptor ***********************************************
**
** Ensembl Transcript Adaptor
**
** Defined as an alias in EnsPFeatureadaptor.
**
** #alias EnsPFeatureadaptor
**
** #cc Bio::EnsEMBL::DBSQL::TranscriptAdaptor
** ##
******************************************************************************/

#define EnsPTranscriptadaptor EnsPFeatureadaptor




/* @const EnsPTranslationadaptor **********************************************
**
** Ensembl Translation Adaptor
**
** Defined as an alias in EnsPBaseadaptor.
**
** #alias EnsPBaseadaptor
**
** #cc Bio::EnsEMBL::DBSQL::TranslationAdaptor
** ##
******************************************************************************/

#define EnsPTranslationadaptor EnsPBaseadaptor




/* @enum EnsEExternaldatabaseStatus *******************************************
**
** Ensembl External Database Status enumeration
**
** @value ensEExternaldatabaseStatusNULL Null
** @value ensEExternaldatabaseStatusKnownXref Known external reference
** @value ensEExternaldatabaseStatusKnown Known
** @value ensEExternaldatabaseStatusXref External reference
** @value ensEExternaldatabaseStatusPred Predicted
** @value ensEExternaldatabaseStatusOrth Orthologue
** @value ensEExternaldatabaseStatusPseudo Pseudo
** @@
******************************************************************************/

typedef enum EnsOExternaldatabaseStatus
{
    ensEExternaldatabaseStatusNULL,
    ensEExternaldatabaseStatusKnownXref,
    ensEExternaldatabaseStatusKnown,
    ensEExternaldatabaseStatusXref,
    ensEExternaldatabaseStatusPred,
    ensEExternaldatabaseStatusOrth,
    ensEExternaldatabaseStatusPseudo
} EnsEExternaldatabaseStatus;




/* @enum EnsEExternaldatabaseType *********************************************
**
** Ensembl External Database Type enumeration
**
** @value ensEExternaldatabaseTypeNULL Null
** @value ensEExternaldatabaseTypeArray Array
** @value ensEExternaldatabaseTypeAltTrans Alternative Translation
** @value ensEExternaldatabaseTypeAltGene Alternative Gene
** @value ensEExternaldatabaseTypeMisc Miscellaneous
** @value ensEExternaldatabaseTypeLit Literature
** @value ensEExternaldatabaseTypePrimaryDbSynonym Primary database synonym
** @value ensEExternaldatabaseTypeEnsembl Ensembl
** @value ensEExternaldatabaseTypeIgnore Ignore
** @@
******************************************************************************/

typedef enum EnsOExternaldatabaseType
{
    ensEExternaldatabaseTypeNULL,
    ensEExternaldatabaseTypeArray,
    ensEExternaldatabaseTypeAltTrans,
    ensEExternaldatabaseTypeAltGene,
    ensEExternaldatabaseTypeMisc,
    ensEExternaldatabaseTypeLit,
    ensEExternaldatabaseTypePrimaryDbSynonym,
    ensEExternaldatabaseTypeEnsembl,
    ensEExternaldatabaseTypeIgnore
} EnsEExternaldatabaseType;




/* @enum EnsEExternalreferenceInfotype ****************************************
**
** Ensembl External Reference Information Type enumeration
**
** @value ensEExternalreferenceInfotypeNULL Null
** @value ensEExternalreferenceInfotypeProjection Projection
** @value ensEExternalreferenceInfotypeMisc Miscellaneous
** @value ensEExternalreferenceInfotypeDependent Dependent
** @value ensEExternalreferenceInfotypeDirect Direct
** @value ensEExternalreferenceInfotypeSequenceMatch Sequence Match
** @value ensEExternalreferenceInfotypeInferredPair Inferred Pair
** @value ensEExternalreferenceInfotypeProbe Probe
** @value ensEExternalreferenceInfotypeUnmapped Unmapped
** @value ensEExternalreferenceInfotypeCoordinateOverlap Coordinate Overlap
** @@
******************************************************************************/

typedef enum EnsOExternalreferenceInfotype
{
    ensEExternalreferenceInfotypeNULL,
    ensEExternalreferenceInfotypeProjection,
    ensEExternalreferenceInfotypeMisc,
    ensEExternalreferenceInfotypeDependent,
    ensEExternalreferenceInfotypeDirect,
    ensEExternalreferenceInfotypeSequenceMatch,
    ensEExternalreferenceInfotypeInferredPair,
    ensEExternalreferenceInfotypeProbe,
    ensEExternalreferenceInfotypeUnmapped,
    ensEExternalreferenceInfotypeCoordinateOverlap
} EnsEExternalreferenceInfotype;




/* @enum EnsEExternalreferenceObjecttype **************************************
**
** Ensembl External Reference Object Type enumeration
**
** @value ensEExternalreferenceObjecttypeNULL Null
** @value ensEExternalreferenceObjecttypeRawContig Raw Contig
** @value ensEExternalreferenceObjectypeTranscript Transcript
** @value ensEExternalreferenceObjecttypeGene Gene
** @value ensEExternalreferenceObjecttypeTranslation Translation
** @@
******************************************************************************/

typedef enum EnsOExternalreferenceObjecttype
{
    ensEExternalreferenceObjecttypeNULL,
    ensEExternalreferenceObjecttypeRawContig,
    ensEExternalreferenceObjectypeTranscript,
    ensEExternalreferenceObjecttypeGene,
    ensEExternalreferenceObjecttypeTranslation
} EnsEExternalreferenceObjecttype;




/* @enum EnsEGeneStatus *******************************************************
**
** Ensembl Gene Status enumeration
**
** @value ensEGeneStatusNULL Null
** @value ensEGeneStatusKnown Known
** @value ensEGeneStatusNovel Novel
** @value ensEGeneStatusPutative Putative
** @value ensEGeneStatusPredicted Predicted
** @value ensEGeneStatusKnownByProjection Known by projection
** @value ensEGeneStatusUnknown Unknown
** @@
******************************************************************************/

typedef enum EnsOGeneStatus
{
    ensEGeneStatusNULL,
    ensEGeneStatusKnown,
    ensEGeneStatusNovel,
    ensEGeneStatusPutative,
    ensEGeneStatusPredicted,
    ensEGeneStatusKnownByProjection,
    ensEGeneStatusUnknown
} EnsEGeneStatus;




/* @enum EnsEIntronsupportingevidenceType *************************************
**
** Ensembl Intron Supporting Evidence Type enumeration
**
** @value ensEIntronsupportingevidenceTypeNULL Null
** @value ensEIntronsupportingevidenceTypeNone None
** @value ensEIntronsupportingevidenceTypeDepth Depth
** @@
******************************************************************************/

typedef enum EnsOIntronsupportingevidenceType
{
    ensEIntronsupportingevidenceTypeNULL,
    ensEIntronsupportingevidenceTypeNone,
    ensEIntronsupportingevidenceTypeDepth
} EnsEIntronsupportingevidenceType;




/* @enum EnsESliceTopology ****************************************************
**
** Ensembl Slice Topology enumeration
**
** @value ensESliceTopologyNULL Null
** @value ensESliceTopologyLinear Linear
** @value ensESliceTopologyCircular Circular
** @@
******************************************************************************/

typedef enum EnsOSliceTopology
{
    ensESliceTopologyNULL,
    ensESliceTopologyLinear,
    ensESliceTopologyCircular
} EnsESliceTopology;




/* @enum EnsESliceType ********************************************************
**
** Ensembl Slice Type enumeration
**
** @value ensESliceTypeNULL Null
** @value ensESliceTypeLinear Linear
** @value ensESliceTypeCircular Circular
** @value ensESliceTypeLrg Locus Reference Genome
** @@
******************************************************************************/

typedef enum EnsOSliceType
{
    ensESliceTypeNULL,
    ensESliceTypeLinear,
    ensESliceTypeCircular,
    ensESliceTypeLrg
} EnsESliceType;




/* @enum EnsETranscriptStatus *************************************************
**
** Ensembl Transcript Status enumeration
**
** @value ensETranscriptStatusNULL Null
** @value ensETranscriptStatusKnown Known
** @value ensETranscriptStatusNovel Novel
** @value ensETranscriptStatusPutative Putative
** @value ensETranscriptStatusPredicted Predicted
** @value ensETranscriptStatusKnownByProjection Known by projection
** @value ensETranscriptStatusUnknown Unknown
** @value ensETranscriptStatusAnnotated Annotated
** @@
******************************************************************************/

typedef enum EnsOTranscriptStatus
{
    ensETranscriptStatusNULL,
    ensETranscriptStatusKnown,
    ensETranscriptStatusNovel,
    ensETranscriptStatusPutative,
    ensETranscriptStatusPredicted,
    ensETranscriptStatusKnownByProjection,
    ensETranscriptStatusUnknown,
    ensETranscriptStatusAnnotated
} EnsETranscriptStatus;




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */

/* @data EnsPSeqregionadaptor *************************************************
**
** Ensembl Sequence Region Adaptor
**
** @attr Adaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @attr CacheByIdentifier [EnsPCache] Ensembl Cache to store Ensembl Sequence
** Region objects with AJAX unsigned integer (SQL-database identifier) objects
** as index
** @attr CacheByName [AjPTable] AJAX Table to cache Ensembl Sequence Region
** objects with "Sequence Region Name:Coordinate System Identifier" AJAX String
** objects as index
** @attr CacheLocusReferenceGenomic [AjPList] AJAX List to cache Ensembl
** Sequence Region objects, which are associated with an Ensembl Attribute of
** code "LRG" and correspond to Locus Reference Genomic entries.
** See http://www.lrg-sequence.org/ for details.
** @attr CacheNonReference [AjPList] AJAX List to cache Ensembl Sequence
** Region objects, which are associated with an Ensembl Attribute of code
** "non_ref" that is usually set for haplotype assembly paths.
** @@
******************************************************************************/

typedef struct EnsSSeqregionadaptor
{
    EnsPDatabaseadaptor Adaptor;
    EnsPCache CacheByIdentifier;
    AjPTable CacheByName;
    AjPList CacheLocusReferenceGenomic;
    AjPList CacheNonReference;
} EnsOSeqregionadaptor;

#define EnsPSeqregionadaptor EnsOSeqregionadaptor*




/* @data EnsPSeqregion ********************************************************
**
** Ensembl Sequence Region
**
** @attr Use [ajuint] Usage counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] Internal SQL database identifier (primary key)
** @attr Adaptor [EnsPSeqregionadaptor] Ensembl Sequence Region Adaptor
** @cc Bio::EnsEMBL::???
** @attr Coordsystem [EnsPCoordsystem] Ensembl Coordinate System
** @attr Name [AjPStr] Name
** @attr Attributes [AjPList] AJAX List of Ensembl Attribute objects
** @attr Seqregionsynonyms [AjPList]
** AJAX List of Ensembl Sequence Region Synonym objects
** @attr Length [ajint] Length
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSSeqregion
{
    ajuint Use;
    ajuint Identifier;
    EnsPSeqregionadaptor Adaptor;
    EnsPCoordsystem Coordsystem;
    AjPStr Name;
    AjPList Attributes;
    AjPList Seqregionsynonyms;
    ajint Length;
    ajuint Padding;
} EnsOSeqregion;

#define EnsPSeqregion EnsOSeqregion*




/* @data EnsPSliceadaptor *****************************************************
**
** Ensembl Slice Adaptor
**
** @attr Adaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @attr CacheByIdentifier [EnsPCache] Caches Ensembl Slice objects by their
** Ensembl Sequence Region identifier, start, end and strand coordinate
** @attr CacheByName [AjPTable] Caches Ensembl Slice objects by their
** Ensembl Sequence Region name, start, end and strand coordinate
** @@
******************************************************************************/

typedef struct EnsSSliceadaptor
{
    EnsPDatabaseadaptor Adaptor;
    EnsPCache CacheByIdentifier;
    AjPTable CacheByName;
} EnsOSliceadaptor;

#define EnsPSliceadaptor EnsOSliceadaptor*




/* @data EnsPSlice ************************************************************
**
** Ensembl Slice
**
** Holds information about a genome sequence slice.
**
** @alias EnsSSlice
** @alias EnsOSlice
**
** @attr Adaptor [EnsPSliceadaptor] Ensembl Slice Adaptor
** @attr Seqregion [EnsPSeqregion] Ensembl Sequence Region
** @attr Sequence [AjPStr] Sequence
** @attr Topology [EnsESliceTopology] Slice Topology enumeration
** @attr Type [EnsESliceType] Ensembl Slice Type enumeration
** @attr Start [ajint] Start coordinate
** @attr End [ajint] End coordinate
** @attr Strand [ajint] Strand information (+1|-1)
** @attr Use [ajuint] Use counter
** @@
******************************************************************************/

typedef struct EnsSSlice
{
    EnsPSliceadaptor Adaptor;
    EnsPSeqregion Seqregion;
    AjPStr Sequence;
    EnsESliceTopology Topology;
    EnsESliceType Type;
    ajint Start;
    ajint End;
    ajint Strand;
    ajuint Use;
} EnsOSlice;

#define EnsPSlice EnsOSlice*




/* @data EnsPAssemblymapperadaptor ********************************************
**
** Ensembl Assembly Mapper Adaptor
**
** The Ensembl Assembly Mapper Adaptor is used to retrieve Ensembl Mapper
** objects between any two Ensembl Coordinate System objects which makeup is
** described by the "assembly" table. Currently, one-step (explicit) and
** two-step (implict) pairwise mapping is supported. In one-step mapping an
** explicit relationship between the Coordinate System objects is defined in
** the "assembly" table. In two-step "chained" mapping no explicit mapping is
** present, but the Coordinate System objects must share a common mapping to
** an intermediate Coordinate System.
**
** @alias EnsSAssemblymapperadaptor
** @alias EnsOAssemblymapperadaptor
**
** @attr Adaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @attr CacheByIdentifiers [AjPTable]
** AJAX Table of AJAX String (Ensembl Coordinate System identifiers
** concatenated by ':' characters) key data and
** Ensembl Assembly Mapper value data
** @attr MultipleMappings [AjPTable]
** AJAX Table of AJAX unsigned integer (Ensembl Sequence Region identifier)
** key data and Ensembl Sequence Region value data for objects that have more
** than one entry in the Ensembl Core "assembly" table, i. e. they map to
** more than one location.
** @@
******************************************************************************/

typedef struct EnsSAssemblymapperadaptor
{
    EnsPDatabaseadaptor Adaptor;
    AjPTable CacheByIdentifiers;
    AjPTable MultipleMappings;
} EnsOAssemblymapperadaptor;

#define EnsPAssemblymapperadaptor EnsOAssemblymapperadaptor*




/* @data EnsPGenericassemblymapper ********************************************
**
** Ensembl Generic Assembly Mapper
**
** The Ensembl Generic Assembly Mapper handles mappings between two Ensembl
** Coordinate System objects using the information stored in the "assembly"
** SQL table of an Ensembl Core database.
**
** The Ensembl Generic Assembly Mapper is a database aware mapper, which
** facilitates conversion of coordinates between any two Ensembl Coordinate
** System objects with a relationship explicitly defined in the "assembly"
** SQL table. In future, it may be possible to perform multiple step (implicit)
** mapping between Ensembl Coordinate System objects.
**
** It is implemented using the Ensembl Mapper object, which is a generic mapper
** object between disjoint Ensembl Coordinate System objects.
**
** @alias EnsSGenericassemblymapper
** @alias EnsOGenericassemblymapper
**
** @attr Adaptor [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
** @attr CoordsystemAssembled [EnsPCoordsystem] Assembled Ensembl Coordinate
** System
** @attr CoordsystemComponent [EnsPCoordsystem] Component Ensembl Coordinate
** System
** @attr RegisterAssembled [AjPTable] Assembled Register
** @attr RegisterComponent [AjPTable] Component Register
** @attr Mapper [EnsPMapper] Ensembl Mapper
** @attr Maximum [ajuint] Maximum Ensembl Mapper Pair count
** @attr Use [ajuint] Use counter
** @@
******************************************************************************/

typedef struct EnsSGenericassemblymapper
{
    EnsPAssemblymapperadaptor Adaptor;
    EnsPCoordsystem CoordsystemAssembled;
    EnsPCoordsystem CoordsystemComponent;
    AjPTable RegisterAssembled;
    AjPTable RegisterComponent;
    EnsPMapper Mapper;
    ajuint Maximum;
    ajuint Use;
} EnsOGenericassemblymapper;

#define EnsPGenericassemblymapper EnsOGenericassemblymapper*




/* @data EnsPChainedassemblymapper ********************************************
**
** Ensembl Chained Assembly Mapper
**
** The Ensembl Chained Assembly Mapper is an extension of the regular
** Ensembl Generic Assembly Mapper that allows for mappings between
** Ensembl Coordinate System objects that require multi-step mapping.
** For example if explicit mappings are defined between the following
** Coordinate System objects, chromosome/contig and contig/clone,
** the Ensembl Chained Assembly Mapper would be able to perform implicit
** mapping between the "chromosome" and "clone" Ensembl Coordinate System
** objects. This should be transparent to the user of this module, and users
** should not even realise that they are using an Ensembl Chained Assembly
** Mapper as opposed to an Ensembl Generic Assembly Mapper.
**
** @alias EnsSChainedassemblymapper
** @alias EnsOChainedassemblymapper
**
** @attr Adaptor [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
** @attr CoordsystemSource [EnsPCoordsystem] Source Ensembl Coordinate System
** @attr CoordsystemMiddle [EnsPCoordsystem] Middle Ensembl Coordinate System
** @attr CoordsystemTarget [EnsPCoordsystem] Target Ensembl Coordinate System
** @attr MapperSourceMiddle [EnsPMapper] Source to middle Ensembl Mapper
** @attr MapperTargetMiddle [EnsPMapper] Target to middle Ensembl Mapper
** @attr MapperSourceTarget [EnsPMapper] Source to target Ensembl Mapper
** @attr RegistrySource [EnsPMapperrangeregistry]
** Source Ensembl Mapper Range Registry
** @attr RegistryTarget [EnsPMapperrangeregistry]
** Target Ensembl Mapper Range Registry
** @attr Maximum [ajuint] Maximum Ensembl Mapper Pair count
** @attr Use [ajuint] Use counter
** @@
******************************************************************************/

typedef struct EnsSChainedassemblymapper
{
    EnsPAssemblymapperadaptor Adaptor;
    EnsPCoordsystem CoordsystemSource;
    EnsPCoordsystem CoordsystemMiddle;
    EnsPCoordsystem CoordsystemTarget;
    EnsPMapper MapperSourceMiddle;
    EnsPMapper MapperTargetMiddle;
    EnsPMapper MapperSourceTarget;
    EnsPMapperrangeregistry RegistrySource;
    EnsPMapperrangeregistry RegistryTarget;
    ajuint Maximum;
    ajuint Use;
} EnsOChainedassemblymapper;

#define EnsPChainedassemblymapper EnsOChainedassemblymapper*




/* @data EnsPToplevelassemblymapper *******************************************
**
** Ensembl Top-Level Assembly Mapper
**
** The Ensembl Top-Level Assembly Mapper performs mapping between a provided
** Ensembl Coordinate System and the top-level pseudo Cooordinate System.
** The top-level Coordinate System is not a real Coordinate System, but
** represents the highest Coordinate System that can be mapped to in a given
** Sequence Region. It is only possible to perform unidirectional mapping
** using this Mapper, because it does not make sense to map from the top-level
** Coordinate System to another Coordinate System.
**
** @alias EnsSToplevelassemblymapper
** @alias EnsOToplevelassemblymapper
**
** @attr Adaptor [EnsPAssemblymapperadaptor]
** Ensembl Assembly Mapper Adaptor
** @attr Coordsystems [AjPList]
** AJAX List of Ensembl Coordinate System objects
** @attr CoordsystemAssembled [EnsPCoordsystem]
** Top-level Ensembl Coordinate System
** @attr CoordsystemComponent [EnsPCoordsystem]
** Other Ensembl Coordinate System
** @attr Use [ajuint] Use counter
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSToplevelassemblymapper
{
    EnsPAssemblymapperadaptor Adaptor;
    AjPList Coordsystems;
    EnsPCoordsystem CoordsystemAssembled;
    EnsPCoordsystem CoordsystemComponent;
    ajuint Use;
    ajuint Padding;
} EnsOToplevelassemblymapper;

#define EnsPToplevelassemblymapper EnsOToplevelassemblymapper*




/* @data EnsPAssemblymapper ***************************************************
**
** Ensembl Assembly Mapper
**
** The Ensembl Assembly Mapper is a wrapper for specialised
** Ensembl Assembly Mapper objects.
**
** @alias EnsSAssemblymapper
** @alias EnsOAssemblymapper
**
** @attr Adaptor [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
** @attr Generic [EnsPGenericassemblymapper] Ensembl Generic Assembly Mapper
** @attr Chained [EnsPChainedassemblymapper] Ensembl Chained Assembly Mapper
** @attr Toplevel [EnsPToplevelassemblymapper]
** Ensembl Top-Level Assembly Mapper
** @attr Use [ajuint] Use counter
** @attr Padding [ajuint] Padding to alignment boundary
** @@
** NOTE: This object subsumes the following Perl objects all returned by the
** Bio::EnsEMBL::DBSQL::AssemblyMapperAdaptor
**
** Bio::EnsEMBL::AssemblyMapper (here Ensembl Generic Assembly Mapper)
** Bio::EnsEMBL::ChainedAssemblyMapper
** Bio::EnsEMBL::TopLevelAssemblyMapper
**
** The objects can be distinguished by their Type member.
******************************************************************************/

typedef struct EnsSAssemblymapper
{
    EnsPAssemblymapperadaptor Adaptor;
    EnsPGenericassemblymapper Generic;
    EnsPChainedassemblymapper Chained;
    EnsPToplevelassemblymapper Toplevel;
    ajuint Use;
    ajuint Padding;
} EnsOAssemblymapper;

#define EnsPAssemblymapper EnsOAssemblymapper*




/* @data EnsPBaseadaptorLeftjoin **********************************************
**
** Ensembl Base Adaptor SQL LEFT JOIN condition
**
** @alias EnsSBaseadaptorLeftjoin
** @alias EnsOBaseadaptorLeftjoin
**
** @attr Tablename [const char*] SQL table name
** @attr Condition [const char*] SQL LEFT JOIN condition
** @@
******************************************************************************/

typedef struct EnsSBaseadaptorLeftjoin
{
    const char *Tablename;
    const char *Condition;
} EnsOBaseadaptorLeftjoin;

#define EnsPBaseadaptorLeftjoin EnsOBaseadaptorLeftjoin*




/* @data EnsPBaseadaptor ******************************************************
**
** Ensembl Base Adaptor
**
** @alias EnsSBaseadaptor
** @alias EnsOBaseadaptor
**
** @alias EnsPDitagadaptor
** @alias EnsPGvindividualadaptor
** @alias EnsPGvpopulationadaptor
** @alias EnsPGvsampleadaptor
** @alias EnsPGvvariationsetadaptor
** @alias EnsPMarkeradaptor
** @alias EnsPOntologytermadaptor
** @alias EnsPProteinfeatureadaptor
** @alias EnsPQcalignmentadaptor
** @alias EnsPQcdasfeatureadaptor
** @alias EnsPQcsubmissionadaptor
** @alias EnsPQcvariationadaptor
** @alias EnsPRepeatconsensusadaptor
** @alias EnsPSeqregionsynonymadaptor
** @alias EnsPTranslationadaptor
**
** @attr Adaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @attr Tablenames [const char* const*] SQL table name array
** @attr Columnnames [const char* const*] SQL column name array
** @attr Leftjoins [const EnsPBaseadaptorLeftjoin]
** SQL LEFT JOIN condition array
** @attr Defaultcondition [const char*] SQL SELECT default condition
** @attr Finalcondition [const char*] SQL SELECT final condition
** @attr Mysqlstraightjoin [AjBool]
** Set the MySQL-specific SQL SELECT STRAIGHT_JOIN option
** @attr Padding [ajuint] Padding to alignment boundary
** @attr Fstatement [AjBool function] Statement function address
** @@
** NOTE: In constrast to the Perl API, this implementation does not feature a
** species_id member, which is already set in the Ensembl Database Adaptor.
******************************************************************************/

typedef struct EnsSBaseadaptor
{
    EnsPDatabaseadaptor Adaptor;
    const char *const *Tablenames;
    const char *const *Columnnames;
    const EnsPBaseadaptorLeftjoin Leftjoins;
    const char *Defaultcondition;
    const char *Finalcondition;
    AjBool Mysqlstraightjoin;
    ajuint Padding;
    AjBool (*Fstatement) (struct EnsSBaseadaptor *dba,
                          const AjPStr sql,
                          EnsPAssemblymapper am,
                          EnsPSlice slice,
                          AjPList objects);
} EnsOBaseadaptor;

#define EnsPBaseadaptor EnsOBaseadaptor*




/* @data EnsPAnalysisadaptor **************************************************
**
** Ensembl Analysis Adaptor
**
** @alias EnsSAnalysisadaptor
** @alias EnsOAnalysisadaptor
**
** @attr Adaptor [EnsPBaseadaptor] Ensembl Base Adaptor
** @attr CacheByIdentifier [AjPTable] Identifier cache
** @attr CacheByName [AjPTable] Name cache
** @@
******************************************************************************/

typedef struct EnsSAnalysisadaptor
{
    EnsPBaseadaptor Adaptor;
    AjPTable CacheByIdentifier;
    AjPTable CacheByName;
} EnsOAnalysisadaptor;

#define EnsPAnalysisadaptor EnsOAnalysisadaptor*




/* @data EnsPAnalysis *********************************************************
**
** Ensembl Analysis
**
** An Ensembl Analysis object stores details of an analysis within the
** Ensembl genome analysis and annotation pipeline.
**
** @alias EnsSAnalysis
** @alias EnsOAnalysis
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPAnalysisadaptor] Ensembl Analysis Adaptor
** @cc Bio::EnsEMBL::Analysis
** @cc "analysis" SQL table
** @attr DateCreation [AjPStr] Creation date
** @attr Name [AjPStr] Name
** @attr Databasename [AjPStr] Database name
** @attr Databaseversion [AjPStr] Database version
** @attr Databasefile [AjPStr] Database file
** @attr Programname [AjPStr] Program name
** @attr Programversion [AjPStr] Program version
** @attr Programfile [AjPStr] Program file
** @attr Parameters [AjPStr] Parameters
** @attr Modulename [AjPStr] Module name
** @attr Moduleversion [AjPStr] Module version
** @attr Gffsource [AjPStr] GFF source
** @attr Gfffeature [AjPStr] GFF feature
** @cc "analysis_description" SQL table
** @attr Description [AjPStr] Description
** @attr Displaylabel [AjPStr] Display label for the Ensembl website
** @attr Webdata [AjPStr] Configuration information for the Ensembl web site
** @attr Displayable [AjBool] Displayable on the Ensembl web site
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSAnalysis
{
    ajuint Use;
    ajuint Identifier;
    EnsPAnalysisadaptor Adaptor;
    AjPStr DateCreation;
    AjPStr Name;
    AjPStr Databasename;
    AjPStr Databaseversion;
    AjPStr Databasefile;
    AjPStr Programname;
    AjPStr Programversion;
    AjPStr Programfile;
    AjPStr Parameters;
    AjPStr Modulename;
    AjPStr Moduleversion;
    AjPStr Gffsource;
    AjPStr Gfffeature;
    AjPStr Description;
    AjPStr Displaylabel;
    AjPStr Webdata;
    AjBool Displayable;
    ajuint Padding;
} EnsOAnalysis;

#define EnsPAnalysis EnsOAnalysis*




/* @data EnsPExternaldatabaseadaptor ******************************************
**
** Ensembl External Database Adaptor
**
** @alias EnsSExternaldatabaseadaptor
** @alias EnsOExternaldatabaseadaptor
**
** @attr Adaptor [EnsPBaseadaptor] Ensembl Base Adaptor
** @attr CacheByIdentifier [AjPTable] Identifier cache
** @attr CacheByName [AjPTable] Name cache
** @@
******************************************************************************/

typedef struct EnsSExternaldatabaseadaptor
{
    EnsPBaseadaptor Adaptor;
    AjPTable CacheByIdentifier;
    AjPTable CacheByName;
} EnsOExternaldatabaseadaptor;

#define EnsPExternaldatabaseadaptor EnsOExternaldatabaseadaptor*




/* @data EnsPAttributetypeadaptor *********************************************
**
** Ensembl Attribute Type Adaptor
**
** @alias EnsSAttributetypeadaptor
** @alias EnsOAttributetypeadaptor
**
** @attr Adaptor [EnsPBaseadaptor] Ensembl Base Adaptor
** @attr CacheByIdentifier [AjPTable] Identifier cache
** @attr CacheByCode [AjPTable] Code cache
** @@
******************************************************************************/

typedef struct EnsSAttributetypeadaptor
{
    EnsPBaseadaptor Adaptor;
    AjPTable CacheByIdentifier;
    AjPTable CacheByCode;
} EnsOAttributetypeadaptor;

#define EnsPAttributetypeadaptor EnsOAttributetypeadaptor*




/* @data EnsPAttributetype ****************************************************
**
** Ensembl Attribute Type
**
** @alias EnsSAttributetype
** @alias EnsOAttributetype
**
** @attr Use [ajuint] Usage counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] Internal SQL database identifier (primary key)
** @attr Adaptor [EnsPAttributetypeadaptor] Ensembl Attribute Type Adaptor
** @cc Bio::EnsEMBL::???
** @attr Code [AjPStr] Code
** @attr Name [AjPStr] Name
** @attr Description [AjPStr] Description
** @@
******************************************************************************/

typedef struct EnsSAttributetype
{
    ajuint Use;
    ajuint Identifier;
    EnsPAttributetypeadaptor Adaptor;
    AjPStr Code;
    AjPStr Name;
    AjPStr Description;
} EnsOAttributetype;

#define EnsPAttributetype EnsOAttributetype*




/* @data EnsPAttribute ********************************************************
**
** Ensembl Attribute
**
** @alias EnsSAttribute
** @alias EnsOAttribute
**
** @cc Bio::EnsEMBL::Attribute
** @attr Attributetype [EnsPAttributetype] Ensembl Attribute Type
** @attr Value [AjPStr] Value
** @attr Use [ajuint] Use counter
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSAttribute
{
    EnsPAttributetype Attributetype;
    AjPStr Value;
    ajuint Use;
    ajuint Padding;
} EnsOAttribute;

#define EnsPAttribute EnsOAttribute*




/* @data EnsPExternaldatabase *************************************************
**
** Ensembl External Database
**
** @alias EnsSExternaldatabase
** @alias EnsOExternaldatabase
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] Internal SQL database identifier (primary key)
** @attr Adaptor [EnsPExternaldatabaseadaptor]
** Ensembl External Database Adaptor
** @cc Bio::EnsEMBL::???
** @cc "external_db" SQL table
** @attr Name [AjPStr] Database name
** @attr Release [AjPStr] Database release
** @attr Displayname [AjPStr] Database display name
** @attr Secondaryname [AjPStr] Secondary database name
** @attr Secondarytable [AjPStr] Secondary database table
** @attr Description [AjPStr] Description
** @attr Status [EnsEExternaldatabaseStatus] Status
** @attr Type [EnsEExternaldatabaseType] Type
** @attr Priority [ajint] Priority
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSExternaldatabase
{
    ajuint Use;
    ajuint Identifier;
    EnsPExternaldatabaseadaptor Adaptor;
    AjPStr Name;
    AjPStr Release;
    AjPStr Displayname;
    AjPStr Secondaryname;
    AjPStr Secondarytable;
    AjPStr Description;
    EnsEExternaldatabaseStatus Status;
    EnsEExternaldatabaseType Type;
    ajint Priority;
    ajuint Padding;
} EnsOExternaldatabase;

#define EnsPExternaldatabase EnsOExternaldatabase*




/* @data EnsPExternalreference ************************************************
**
** Ensembl External Reference
**
** @alias EnsSExternalreference
** @alias EnsOExternalreference
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] Internal SQL database identifier (primary key)
** @attr Analysis [EnsPAnalysis] Ensembl Analysis
** @cc "xref" SQL table
** @attr Externaldatabase [EnsPExternaldatabase] Ensembl External Database
** @attr Primaryidentifier [AjPStr] Primary identifier
** @attr Displayidentifier [AjPStr] Display identifier
** @attr Version [AjPStr] Version
** @attr Description [AjPStr] Description
** @attr Linkageannotation [AjPStr] Linkage annotation
** @attr Infotext [AjPStr] Information text
** @attr Infotype [EnsEExternalreferenceInfotype]
** Ensembl External Reference Information Type enumeration
** @attr Objecttype [EnsEExternalreferenceObjecttype]
** Ensembl External Reference Object Type enumeration
** @attr Objectidentifier [ajuint] Ensembl Object identifier
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSExternalreference
{
    ajuint Use;
    ajuint Identifier;
    EnsPAnalysis Analysis;
    EnsPExternaldatabase Externaldatabase;
    AjPStr Primaryidentifier;
    AjPStr Displayidentifier;
    AjPStr Version;
    AjPStr Description;
    AjPStr Linkageannotation;
    AjPStr Infotext;
    EnsEExternalreferenceInfotype Infotype;
    EnsEExternalreferenceObjecttype Objecttype;
    ajuint Objectidentifier;
    ajuint Padding;
} EnsOExternalreference;

#define EnsPExternalreference EnsOExternalreference*




/* @data EnsPIdentityreference ************************************************
**
** Ensembl Identity Reference
**
** @alias EnsSIdentityreference
** @alias EnsOIdentityreference
**
** @cc Bio::EnsEMBL::IdentityXref
** @cc "identity_xref" SQL table
** @attr Cigar [AjPStr] Cigar line (See exonerate(1))
** @attr QueryStart [ajint] Query start
** @attr QueryEnd [ajint] Query end
** @attr QueryIdentity [ajint] Query sequence identity
** @attr TargetStart [ajint] Target start
** @attr TargetEnd [ajint] Target end
** @attr TargetIdentity [ajint] Target sequence identity
** @attr Use [ajuint] Use counter
** @attr Padding [ajuint] Padding to alignment boundary
** @attr Evalue [double] e-Value
** @attr Score [double] Score
** @@
******************************************************************************/

typedef struct EnsSIdentityreference
{
    AjPStr Cigar;
    ajint QueryStart;
    ajint QueryEnd;
    ajint QueryIdentity;
    ajint TargetStart;
    ajint TargetEnd;
    ajint TargetIdentity;
    ajuint Use;
    ajuint Padding;
    double Evalue;
    double Score;
} EnsOIdentityreference;

#define EnsPIdentityreference EnsOIdentityreference*




/* @data EnsPDatabaseentry ****************************************************
**
** Ensembl Database Entry
**
** @alias EnsSDatabaseentry
** @alias EnsODatabaseentry
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] Internal SQL database identifier (primary key)
** @attr Adaptor [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
** @cc Bio::EnsEMBL::DBEntry
** @cc "xref" SQL table
** @attr Externalreference [EnsPExternalreference] Ensembl External Reference
** @cc Bio::EnsEMBL::IdentityXref
** @attr Identityreference [EnsPIdentityreference] Ensembl Identity Reference
** @cc "external_synonym" SQL table
** @attr Synonyms [AjPList] Synonyms
** @attr Ontologylinkages [AjPList] AJAX List of
** Ensembl Ontology Linkage objects
** @@
******************************************************************************/

typedef struct EnsSDatabaseentry
{
    ajuint Use;
    ajuint Identifier;
    EnsPDatabaseentryadaptor Adaptor;
    EnsPExternalreference Externalreference;
    EnsPIdentityreference Identityreference;
    AjPList Synonyms;
    AjPList Ontologylinkages;
} EnsODatabaseentry;

#define EnsPDatabaseentry EnsODatabaseentry*




/* @data EnsPOntologylinkage **************************************************
**
** Ensembl Ontology Linkage
**
** @alias EnsSOntologylinkage
** @alias EnsOOntologylinkage
**
** @cc "ontology_xref" SQL table
** @attr LinkageType [AjPStr] Likage type (Ontology Evidence Code)
** @attr Source [EnsPDatabaseentry] Source Ensembl Database Entry
** @attr Use [ajuint] Use counter
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSOntologylinkage
{
    AjPStr LinkageType;
    EnsPDatabaseentry Source;
    ajuint Use;
    ajuint Padding;
} EnsOOntologylinkage;

#define EnsPOntologylinkage EnsOOntologylinkage*




/* @data EnsPFeature **********************************************************
**
** Ensembl Feature
**
** @alias EnsSFeature
** @alias EnsOFeature
**
** @cc Bio::EnsEMBL::Feature
** @attr Analysis [EnsPAnalysis] Ensembl Analysis
** @attr Slice [EnsPSlice] Ensembl Slice
** @attr Sequencename [AjPStr] Sequence name
** @attr Start [ajint] Start coordinate
** @attr End [ajint] End coordinate
** @attr Strand [ajint] Strand orientation
** @attr Use [ajuint] Use counter
** @@
******************************************************************************/

typedef struct EnsSFeature
{
    EnsPAnalysis Analysis;
    EnsPSlice Slice;
    AjPStr Sequencename;
    ajint Start;
    ajint End;
    ajint Strand;
    ajuint Use;
} EnsOFeature;

#define EnsPFeature EnsOFeature*




/* @data EnsPFeatureadaptor ***************************************************
**
** Ensembl Feature Adaptor
**
** @alias EnsSFeatureadaptor
** @alias EnsOFeatureadaptor
**
** @alias EnsPExonadaptor
** @alias EnsPDnaalignfeatureadaptor
** @alias EnsPDensityfeatureadaptor
** @alias EnsPDitagfeatureadaptor
** @alias EnsPGeneadaptor
** @alias EnsPIntronsupportingevidenceadaptor
** @alias EnsPKaryotypebandadaptor
** @alias EnsPMarkerfeatureadaptor
** @alias EnsPMiscellaneousfeatureadaptor
** @alias EnsPOperonadaptor
** @alias EnsPOperontranscriptadaptor
** @alias EnsPPredictionexonadaptor
** @alias EnsPPredictiontranscriptadaptor
** @alias EnsPProteinalignfeatureadaptor
** @alias EnsPRepeatfeatureadaptor
** @alias EnsPSimplefeatureadaptor
**
** @attr Adaptor [EnsPBaseadaptor] Ensembl Base Adaptor
** @attr Cache [EnsPCache] Ensembl LRU Cache
** @attr Tablenames [char**] SQL table name array, which is used for
**                           multi-species databases only.
**                           The array is dynamically allocated to extend for
**                           "seq_region" and "coord_system" table names.
**                           The array is then set in the Ensembl Base Adaptor
**                           object.
** @attr Defaultcondition [char*] SQL SELECT default condition, which is used
**                         for multi-species databases only. This character
**                         string is dynamically allocated to extend for
**                         "seq_region" and "coord_system" conditions.
**                         The string is then set in the Ensembl Base Adaptor
**                         object.
** @attr FobjectGetFeature [EnsPFeature function]
** Ensembl Object-specific objectGetFeature member function
** @attr Freference [void* function]
** Ensembl Object-specific referencing function
** @attr Fdelete [void function]
** Ensembl Object-specific deletion function
** @attr Startequalsend [AjBool] Simplified SQL in featureadaptorSliceFetch
** @attr Maximumlength [ajint] Maximum length of a particular Feature type
** @@
******************************************************************************/

typedef struct EnsSFeatureadaptor
{
    EnsPBaseadaptor Adaptor;
    EnsPCache Cache;
    char **Tablenames;
    char *Defaultcondition;
    EnsPFeature (*FobjectGetFeature) (const void *object);
    void *(*Freference) (void *value);
    void (*Fdelete) (void **Pvalue);
    AjBool Startequalsend;
    ajint Maximumlength;
} EnsOFeatureadaptor;

#define EnsPFeatureadaptor EnsOFeatureadaptor*




/* @data EnsPExonadaptor ******************************************************
**
** Ensembl Exon Adaptor
**
** @alias EnsSExonadaptor
** @alias EnsOExonadaptor
**
**
** @attr Exonadaptor [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @attr Exontranscriptadaptor [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @@
******************************************************************************/

typedef struct EnsSExonadaptor
{
    EnsPFeatureadaptor Exonadaptor;
    EnsPFeatureadaptor Exontranscriptadaptor;
} EnsOExonadaptor;

#define EnsPExonadaptor EnsOExonadaptor*




/* @data EnsPExon *************************************************************
**
** Ensembl Exon
**
** @alias EnsSExon
** @alias EnsOExon
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] Internal SQL database identifier (primary key)
** @attr Adaptor [EnsPExonadaptor] Ensembl Exon Adaptor
** @cc Bio::EnsEMBL::Feature
** @attr Feature [EnsPFeature] Ensembl Feature
** @cc Bio::EnsEMBL::Exon
** @cc "exon" SQL table
** @attr PhaseStart [ajint] Phase of Exon start
** @attr PhaseEnd [ajint] Phase of Exon end
** @attr Current [AjBool] Current
** @attr Constitutive [AjBool] Consitutive
** @cc "exon_stable_id" SQL table
** @attr Stableidentifier [AjPStr] Stable identifier
** @attr Version [ajuint] Version
** @attr Padding [ajuint] Padding to alignment boundary
** @attr DateCreation [AjPStr] Creation date
** @attr DateModification [AjPStr] Modification date
** @cc Additional members not in SQL tables
** @attr SequenceCache [AjPStr] Sequence Cache
** @attr Supportingfeatures [AjPList]
** AJAX List of Ensembl Base Align Feature objects
** @@
******************************************************************************/

typedef struct EnsSExon
{
    ajuint Use;
    ajuint Identifier;
    EnsPExonadaptor Adaptor;
    EnsPFeature Feature;
    ajint PhaseStart;
    ajint PhaseEnd;
    AjBool Current;
    AjBool Constitutive;
    AjPStr Stableidentifier;
    ajuint Version;
    ajuint Padding;
    AjPStr DateCreation;
    AjPStr DateModification;
    AjPStr SequenceCache;
    AjPList Supportingfeatures;
} EnsOExon;

#define EnsPExon EnsOExon*




/* @data EnsPIntron ***********************************************************
**
** Ensembl Intron
**
** @alias EnsSIntron
** @alias EnsOIntron
**
** @cc Bio::EnsEMBL::Feature
** @attr Feature [EnsPFeature] Ensembl Feature
** @cc Bio::EnsEMBL::Intron
** @attr PreviousExon [EnsPExon] Previous Ensembl Exon
** @attr NextExon [EnsPExon] Next Ensembl Exon
** @attr Use [ajuint] Use counter
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSIntron
{
    EnsPFeature Feature;
    EnsPExon PreviousExon;
    EnsPExon NextExon;
    ajuint Use;
    ajuint Padding;
} EnsOIntron;

#define EnsPIntron EnsOIntron*




/* @data EnsPIntronsupportingevidence *****************************************
**
** Ensembl Intron Supporting Evidence
**
** @alias EnsSIntronsupportingevidence
** @alias EnsOIntronsupportingevidence
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPIntronsupportingevidenceadaptor]
** Ensembl Intron Supporting Evidence Adaptor
** @cc Bio::EnsEMBL::Feature
** @attr Feature [EnsPFeature] Ensembl Feature
** @cc Bio::EnsEMBL::IntronSupportingEvidence
** @attr Intron [EnsPIntron] Ensembl Intron
** @attr Hitname [AjPStr] Hit name
** @attr Canonical [AjBool] Canonical splice sites
** @attr Score [double] Socre
** @attr Type [EnsEIntronsupportingevidenceType]
** Ensembl Intron Supporting Evidence Type enumeration
** @@
******************************************************************************/

typedef struct EnsSIntronsupportingevidence
{
    ajuint Use;
    ajuint Identifier;
    EnsPIntronsupportingevidenceadaptor Adaptor;
    EnsPFeature Feature;
    EnsPIntron Intron;
    AjPStr Hitname;
    AjBool Canonical;
    double Score;
    EnsEIntronsupportingevidenceType Type;
} EnsOIntronsupportingevidence;

#define EnsPIntronsupportingevidence EnsOIntronsupportingevidence*




/* @data EnsPTranslation ******************************************************
**
** Ensembl Translation
**
** @alias EnsSTranslation
** @alias EnsOTranslation
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] Internal SQL database identifier (primary key)
** @attr Adaptor [EnsPTranslationadaptor] Ensembl Translation Adaptor
** @cc Bio::EnsEMBL::Translation
** @cc "translation" SQL table
** @attr Startexon [EnsPExon]
** Ensembl Exon in which the Translation start coordinate is annotated
** @attr Endexon [EnsPExon]
** Ensembl Exon in which the Translation end coordinate is annotated
** @attr Start [ajuint] Start coordinate relative to the start Ensembl Exon
** @attr End [ajuint] End coordinate relative to the end Ensembl Exon
** @cc "translation_stable_id" SQL table
** @attr Stableidentifier [AjPStr] Stable identifier
** @attr DateCreation [AjPStr] Creation date
** @attr DateModification [AjPStr] Modification date
** @attr Version [ajuint] Version
** @attr Padding [ajuint] Padding to alignment boundary
** @cc Additional members not in SQL tables
** @attr Attributes [AjPList] AJAX List of Ensembl Attribute objects
** @attr Databaseentries [AjPList] AJAX List of Ensembl Database Entry objects
** @attr Proteinfeatures [AjPList] AJAX List of Ensembl Protein Feature objects
** @attr Sequence [AjPStr] Sequence
** @attr TranscriptStart [ajuint]
** Start coordinate relative to the Ensembl Transcript (cDNA)
** @attr TranscriptEnd [ajuint]
** End coordinate relative to the Ensembl Transcript (cDNA)
** @attr SliceStart [ajint] Start coordinate relative to the Ensembl Slice
** @attr SliceEnd [ajint] End coordinate relative to the Ensembl Slice
** @@
******************************************************************************/

typedef struct EnsSTranslation
{
    ajuint Use;
    ajuint Identifier;
    EnsPTranslationadaptor Adaptor;
    EnsPExon Startexon;
    EnsPExon Endexon;
    ajuint Start;
    ajuint End;
    AjPStr Stableidentifier;
    AjPStr DateCreation;
    AjPStr DateModification;
    ajuint Version;
    ajuint Padding;
    AjPList Attributes;
    AjPList Databaseentries;
    AjPList Proteinfeatures;
    AjPStr Sequence;
    ajuint TranscriptStart;
    ajuint TranscriptEnd;
    ajint SliceStart;
    ajint SliceEnd;
} EnsOTranslation;

#define EnsPTranslation EnsOTranslation*




/* @data EnsPTranscript *******************************************************
**
** Ensembl Transcript
**
** @alias EnsSTranscript
** @alias EnsOTranscript
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] Internal SQL database identifier (primary key)
** @attr Adaptor [EnsPTranscriptadaptor] Ensembl Transcript Adaptor
** @cc Bio::EnsEMBL::Feature
** @attr Feature [EnsPFeature] Ensembl Feature
** @cc Bio::EnsEMBL::Transcript
** @cc "transcript" SQL table
** @attr Displayreference [EnsPDatabaseentry] Display External Reference
** @attr Description [AjPStr] Description
** @attr Biotype [AjPStr] Biological type
** @attr Status [EnsETranscriptStatus] Status
** @attr Current [AjBool] Current attribute
** @cc "transcript_stable_id" SQL table
** @attr Stableidentifier [AjPStr] Stable identifier
** @attr DateCreation [AjPStr] Creation date
** @attr DateModification [AjPStr] Modification date
** @attr Version [ajuint] Version
** @cc Additional members not in SQL tables
** @attr Geneidentifier [ajuint] Ensembl Gene identifier
** @attr Alternativetranslations [AjPList]
** AJAX List of alternative Ensembl Translation objects
** @attr Attributes [AjPList]
** AJAX List of Ensembl Attribute objects
** @attr Databaseentries [AjPList]
** AJAX List of Ensembl Database Entry objects
** @attr Exons [AjPList]
** AJAX List of Ensembl Exon objects
** @attr Intronsupportingevidences [AjPList]
** AJAX List of Ensembl Intron Supporting Evidence objects
** @attr Supportingfeatures [AjPList]
** AJAX List of Ensembl Base Align Feature objects
** @attr Translation [EnsPTranslation] Ensembl Translation
** @attr Sequenceedits [AjBool] Enable Ensembl Sequence Edit objects
** @attr Padding [ajuint] Padding to alignment boundary
** @attr Canonical [AjBool*] Canonical attribute
** @cc Bio::EnsEMBL::TranscriptMapper
** @attr ExonCoordMapper [EnsPMapper] Ensembl Mapper
** @@
******************************************************************************/

typedef struct EnsSTranscript
{
    ajuint Use;
    ajuint Identifier;
    EnsPTranscriptadaptor Adaptor;
    EnsPFeature Feature;
    EnsPDatabaseentry Displayreference;
    AjPStr Description;
    AjPStr Biotype;
    EnsETranscriptStatus Status;
    AjBool Current;
    AjPStr Stableidentifier;
    AjPStr DateCreation;
    AjPStr DateModification;
    ajuint Version;
    ajuint Geneidentifier;
    AjPList Alternativetranslations;
    AjPList Attributes;
    AjPList Databaseentries;
    AjPList Exons;
    AjPList Intronsupportingevidences;
    AjPList Supportingfeatures;
    EnsPTranslation Translation;
    AjBool Sequenceedits;
    ajuint Padding;
    AjBool *Canonical;
    EnsPMapper ExonCoordMapper;
} EnsOTranscript;

#define EnsPTranscript EnsOTranscript*




/* @data EnsPGene *************************************************************
**
** Ensembl Gene
**
** @alias EnsSGene
** @alias EnsOGene
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] Internal SQL database identifier (primary key)
** @attr Adaptor [EnsPGeneadaptor] Ensembl Gene Adaptor
** @cc Bio::EnsEMBL::Feature
** @attr Feature [EnsPFeature] Ensembl Feature
** @cc Bio::EnsEMBL::Gene
** @cc "gene" SQL table
** @attr Displayreference [EnsPDatabaseentry] Display External Reference
** @attr Description [AjPStr] Description
** @attr Source [AjPStr] Source
** @attr Biotype [AjPStr] Biological type
** @attr Status [EnsEGeneStatus] Status
** @attr Current [AjBool] Current attribute
** @attr Canonicalannotation [AjPStr] Canonical annotation
** @attr Canonicaltranscriptidentifier [ajuint]
** Canonical Ensembl Transcript identifier
** @attr Version [ajuint] Version
** @cc "gene_stable_id" SQL table
** @attr Stableidentifier [AjPStr] Stable identifier
** @attr DateCreation [AjPStr] Creation date
** @attr DateModification [AjPStr] Modification date
** @cc Additional members not in SQL tables
** @attr Attributes [AjPList] AJAX List of Ensembl Attribute objects
** @attr Databaseentries [AjPList] AJAX List of Ensembl Database Entry objects
** @attr Transcripts [AjPList] AJAX List of Ensembl Transcript objects
** @@
******************************************************************************/

typedef struct EnsSGene
{
    ajuint Use;
    ajuint Identifier;
    EnsPGeneadaptor Adaptor;
    EnsPFeature Feature;
    EnsPDatabaseentry Displayreference;
    AjPStr Description;
    AjPStr Source;
    AjPStr Biotype;
    EnsEGeneStatus Status;
    AjBool Current;
    AjPStr Canonicalannotation;
    ajuint Canonicaltranscriptidentifier;
    ajuint Version;
    AjPStr Stableidentifier;
    AjPStr DateCreation;
    AjPStr DateModification;
    AjPList Attributes;
    AjPList Databaseentries;
    AjPList Transcripts;
} EnsOGene;

#define EnsPGene EnsOGene*




/* @data EnsPOperon ***********************************************************
**
** Ensembl Operon
**
** @alias EnsSOperon
** @alias EnsOOperon
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] Internal SQL database identifier (primary key)
** @attr Adaptor [EnsPOperonadaptor] Ensembl Operon Adaptor
** @cc Bio::EnsEMBL::Feature
** @attr Feature [EnsPFeature] Ensembl Feature
** @cc Bio::EnsEMBL::Operon
** @cc "operon" SQL table
** @attr Displaylabel [AjPStr] Display Label
** @attr Stableidentifier [AjPStr] Stable identifier
** @attr DateCreation [AjPStr] Creation date
** @attr DateModification [AjPStr] Modification date
** @attr Version [ajuint] Version
** @attr Padding [ajuint] Padding to alignment boundary
** @attr Attributes [AjPList] AJAX List of Ensembl Attribute objects
** @attr Databaseentries [AjPList] AJAX List of Ensembl Database Entry objects
** @attr Operontranscripts [AjPList]
** AJAX List of Ensembl Operon Transcript objects
** @@
******************************************************************************/

typedef struct EnsSOperon
{
    ajuint Use;
    ajuint Identifier;
    EnsPOperonadaptor Adaptor;
    EnsPFeature Feature;
    AjPStr Displaylabel;
    AjPStr Stableidentifier;
    AjPStr DateCreation;
    AjPStr DateModification;
    ajuint Version;
    ajuint Padding;
    AjPList Attributes;
    AjPList Databaseentries;
    AjPList Operontranscripts;
} EnsOOperon;

#define EnsPOperon EnsOOperon*




/* @data EnsPOperontranscript *************************************************
**
** Ensembl Operon Transcript
**
** @alias EnsSOperontranscript
** @alias EnsOOperontranscript
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] Internal SQL database identifier (primary key)
** @attr Adaptor [EnsPOperontranscriptadaptor]
** Ensembl Operon Transcript Adaptor
** @cc Bio::EnsEMBL::Feature
** @attr Feature [EnsPFeature] Ensembl Feature
** @cc Bio::EnsEMBL::OperonTranscript
** @cc "operon_transcript" SQL table
** @attr Displaylabel [AjPStr] Display Label
** @attr Stableidentifier [AjPStr] Stable identifier
** @attr DateCreation [AjPStr] Creation date
** @attr DateModification [AjPStr] Modification date
** @attr Version [ajuint] Version
** @attr Padding [ajuint] Padding to alignment boundary
** @attr Attributes [AjPList] AJAX List of Ensembl Attribute objects
** @attr Databaseentries [AjPList] AJAX List of Ensembl Database Entry objects
** @attr Genes [AjPList] AJAX List of Ensembl Gene objects
** @@
******************************************************************************/

typedef struct EnsSOperontranscript
{
    ajuint Use;
    ajuint Identifier;
    EnsPOperontranscriptadaptor Adaptor;
    EnsPFeature Feature;
    AjPStr Displaylabel;
    AjPStr Stableidentifier;
    AjPStr DateCreation;
    AjPStr DateModification;
    ajuint Version;
    ajuint Padding;
    AjPList Attributes;
    AjPList Databaseentries;
    AjPList Genes;
} EnsOOperontranscript;

#define EnsPOperontranscript EnsOOperontranscript*




/* @data EnsPSeqregionsynonym *************************************************
**
** Ensembl Sequence Region Synonym
**
** @attr Use [ajuint] Usage counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] Internal SQL database identifier (primary key)
** @attr Adaptor [EnsPSeqregionsynonymadaptor]
** Ensembl Sequence Region Synonym Adaptor
** @cc Bio::EnsEMBL::SeqRegionSynonym
** @attr Externaldatabase [EnsPExternaldatabase] Ensembl External Database
** @attr Name [AjPStr] Name
** @attr Seqregionidentifier [ajuint] Ensembl Sequence Region identifier
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSSeqregionsynonym
{
    ajuint Use;
    ajuint Identifier;
    EnsPSeqregionsynonymadaptor Adaptor;
    EnsPExternaldatabase Externaldatabase;
    AjPStr Name;
    ajuint Seqregionidentifier;
    ajuint Padding;
} EnsOSeqregionsynonym;

#define EnsPSeqregionsynonym EnsOSeqregionsynonym*




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

#endif /* !ENSDATA_H */

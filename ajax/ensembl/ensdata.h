#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ensdata_h
#define ensdata_h

#include "enscache.h"
#include "enscoordsystem.h"
#include "ensmapper.h"




/* @data EnsPAttribute ********************************************************
**
** Ensembl Attribute.
**
** @alias EnsSAttribute
** @alias EnsOAttribute
**
** @cc Bio::EnsEMBL::Attribute
** @attr Code [AjPStr] Ensembl Code
** @attr Name [AjPStr] Ensembl Name
** @attr Description [AjPStr] Description
** @attr Value [AjPStr] Value
** @attr Use [ajuint] Use counter
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSAttribute {
    AjPStr Code;
    AjPStr Name;
    AjPStr Description;
    AjPStr Value;
    ajuint Use;
    ajuint Padding;
} EnsOAttribute;

#define EnsPAttribute EnsOAttribute*




/* @data EnsPSeqregionadaptor *************************************************
**
** Ensembl Sequence Region Adaptor.
**
** @attr Adaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @attr CacheByIdentifier [EnsPCache] Ensembl Cache to store Ensembl Sequence
**                              Regions with SQL-database identifiers as index
** @attr CacheByName [AjPTable] AJAX Table to cache Ensembl Sequence Regions
**                              with 'SequenceRegionName:CoordinatesystemId'
**                              AJAX Strings as index
** @@
******************************************************************************/

typedef struct EnsSSeqregionadaptor {
    EnsPDatabaseadaptor Adaptor;
    EnsPCache CacheByIdentifier;
    AjPTable CacheByName;
} EnsOSeqregionadaptor;

#define EnsPSeqregionadaptor EnsOSeqregionadaptor*




/* @data EnsPSeqregion ********************************************************
**
** Ensembl Sequence Region.
**
** @attr Use [ajuint] Usage counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] Internal SQL database identifier (primary key)
** @attr Adaptor [EnsPSeqregionadaptor] Ensembl Sequence Region Adaptor
** @cc Bio::EnsEMBL::???
** @attr Coordsystem [EnsPCoordsystem] Ensembl Coordinate System
** @attr Name [AjPStr] Name
** @attr Attributes [AjPList] AJAX List of Ensembl Attributes
** @attr Length [ajint] Length
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

/*
** FIXME: The SQL table definition for seq_region.length specifies SIGNED.
** Why? This should be safe to be UNSIGNED.
*/

typedef struct EnsSSeqregion {
    ajuint Use;
    ajuint Identifier;
    EnsPSeqregionadaptor Adaptor;
    EnsPCoordsystem Coordsystem;
    AjPStr Name;
    AjPList Attributes;
    ajint Length;
    ajuint Padding;
} EnsOSeqregion;

#define EnsPSeqregion EnsOSeqregion*




/* @data EnsPSliceadaptor *****************************************************
**
** Ensembl Slice Adaptor.
**
** @attr Adaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @attr CacheByIdentifier [EnsPCache] Cache Ensembl Slice by their
**                                     Ensembl Sequence Region identifier,
**                                     start, end and strand coordinate.
** @attr CacheByName [AjPTable] Cache Ensembl Slices by their
**                              Ensembl Sequence Region name, start, end and
**                              strand coordinate.
** @@
******************************************************************************/

typedef struct EnsSSliceadaptor {
    EnsPDatabaseadaptor Adaptor;
    EnsPCache CacheByIdentifier;
    AjPTable CacheByName;
} EnsOSliceadaptor;

#define EnsPSliceadaptor EnsOSliceadaptor*




/* @data EnsPSlice ************************************************************
**
** Ensembl Slice.
**
** Holds information about a genome sequence slice.
**
** @alias EnsSSlice
** @alias EnsOSlice
**
** @attr Adaptor [EnsPSliceadaptor] Ensembl Slice Adaptor
** @attr Seqregion [EnsPSeqregion] Ensembl Sequence Region
** @attr Sequence [AjPStr] Sequence
** @attr Start [ajint] Start coordinate
** @attr End [ajint] End coordinate
** @attr Strand [ajint] Strand information (+1|-1)
** @attr Use [ajuint] Use counter
** @@
******************************************************************************/

typedef struct EnsSSlice {
    EnsPSliceadaptor Adaptor;
    EnsPSeqregion Seqregion;
    AjPStr Sequence;
    ajint Start;
    ajint End;
    ajint Strand;
    ajuint Use;
} EnsOSlice;

#define EnsPSlice EnsOSlice*




/* @data EnsPAssemblymapperadaptor ********************************************
**
** Ensembl Assembly Mapper Adaptor.
**
** The Ensembl Assembly Mapper Adaptor is used to retrieve Ensembl Mappers
** between any two Ensembl Coordinate Systems whose makeup is described by
** the 'assembly' table. Currently, one-step (explicit) and two-step (implict)
** pairwise mapping is supported. In one-step mapping an explicit relationship
** between the Coordinate Systems is defined in the 'assembly' table.
** In two-step 'chained' mapping no explicit mapping is present, but the
** Coordinate Systems must share a common mapping to an intermediate
** Coordinate System.
**
** @alias EnsSAssemblymapperadaptor
** @alias EnsOAssemblymapperadaptor
**
** @attr Adaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @attr AsmMapperCache [AjPTable] Ensembl Assembly Mapper cache
** @attr MultipleMappings [AjPTable] Sequence Regions that have more than one
**                                   entry in the Ensembl Core 'assembly'
**                                   table, i. e. they map to more than one
**                                   location.
** @@
** The Ensembl Assembly Mapper cache is implemented as an AJAX Table with
** Ensembl Coordinate System identifiers as AJAX String keys and
** Ensembl Assembly Mapper objects as values.
******************************************************************************/

typedef struct EnsSAssemblymapperadaptor {
    EnsPDatabaseadaptor Adaptor;
    AjPTable AsmMapperCache;
    AjPTable MultipleMappings;
} EnsOAssemblymapperadaptor;

#define EnsPAssemblymapperadaptor EnsOAssemblymapperadaptor*




/* @data EnsPGenericassemblymapper ********************************************
**
** Ensembl Generic Assembly Mapper.
**
** The Ensembl Generic Assembly Mapper handles mappings between two Ensembl
** Coordinate Systems using the information stored in the 'assembly' table of
** an Ensembl Core database.
**
** The Ensembl Generic Assembly Mapper is a database aware mapper, which
** facilitates conversion of coordinates between any two coordinate systems
** with a relationship explicitly defined in the assembly table. In future,
** it may be possible to perform multiple step (implicit) mapping between
** coordinate systems.
**
** It is implemented using the Ensembl Mapper object, which is a generic mapper
** object between disjoint coordinate systems.
**
** @alias EnsSGenericassemblymapper
** @alias EnsOGenericassemblymapper
**
** @attr Adaptor [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
** @attr AsmCoordsystem [EnsPCoordsystem] Assembled Ensembl Coordinate System
** @attr CmpCoordsystem [EnsPCoordsystem] Component Ensembl Coordinate System
** @attr AsmRegister [AjPTable] Assembled Register
** @attr CmpRegister [AjPTable] Component Register
** @attr Mapper [EnsPMapper] Ensembl Mapper
** @attr MaxPairCount [ajuint] Maximum Ensembl Mapper Pair count
** @attr Use [ajuint] Use counter
** @@
******************************************************************************/

typedef struct EnsSGenericassemblymapper {
    EnsPAssemblymapperadaptor Adaptor;
    EnsPCoordsystem AsmCoordsystem;
    EnsPCoordsystem CmpCoordsystem;
    AjPTable AsmRegister;
    AjPTable CmpRegister;
    EnsPMapper Mapper;
    ajuint MaxPairCount;
    ajuint Use;
} EnsOGenericassemblymapper;

#define EnsPGenericassemblymapper EnsOGenericassemblymapper*




/* @data EnsPChainedassemblymapper ********************************************
**
** Ensembl Chained Assembly Mapper.
**
** The Ensembl Chained Assembly Mapper is an extension of the regular
** Ensembl Generic Assembly Mapper that allows for mappings between
** Ensembl Coordinate Systems that require multi-step mapping.
** For example if explicit mappings are defined between the following
** Coordinate Systems,
**   chromosome ... contig
**   contig     ... clone
** the Ensembl Chained Assembly Mapper would be able to perform implicit
** mapping between the chromosome and clone coordinate systems. This should be
** transparent to the user of this module, and users should not even realise
** that they are using a Chained Assembly Mapper as opposed to a
** Generic Assembly Mapper.
**
** @alias EnsSChainedassemblymapper
** @alias EnsOChainedassemblymapper
**
** @attr Adaptor [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor
** @attr SourceCoordsystem [EnsPCoordsystem] Source Ensembl Coordinate System
** @attr MiddleCoordsystem [EnsPCoordsystem] Middle Ensembl Coordinate System
** @attr TargetCoordsystem [EnsPCoordsystem] Target Ensembl Coordinate System
** @attr SourceMiddleMapper [EnsPMapper] Source to middle Ensembl Mapper
** @attr TargetMiddleMapper [EnsPMapper] Target to middle Ensembl Mapper
** @attr SourceTargetMapper [EnsPMapper] Source to target Ensembl Mapper
** @attr SourceRegistry [EnsPMapperrangeregistry] Source Ensembl Mapper
**                                                Range Registry
** @attr TargetRegistry [EnsPMapperrangeregistry] Target Ensembl Mapper
**                                                Range Registry
** @attr MaxPairCount [ajuint] Maximum Ensembl Mapper Pair count
** @attr Use [ajuint] Use counter
** @@
******************************************************************************/

typedef struct EnsSChainedassemblymapper {
    EnsPAssemblymapperadaptor Adaptor;
    EnsPCoordsystem SourceCoordsystem;
    EnsPCoordsystem MiddleCoordsystem;
    EnsPCoordsystem TargetCoordsystem;
    EnsPMapper SourceMiddleMapper;
    EnsPMapper TargetMiddleMapper;
    EnsPMapper SourceTargetMapper;
    EnsPMapperrangeregistry SourceRegistry;
    EnsPMapperrangeregistry TargetRegistry;
    ajuint MaxPairCount;
    ajuint Use;
} EnsOChainedassemblymapper;

#define EnsPChainedassemblymapper EnsOChainedassemblymapper*




/* @data EnsPToplevelassemblymapper *******************************************
**
** Ensembl Top-Level Assembly Mapper.
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
** @attr Adaptor [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor.
** @attr Coordsystems [AjPList] AJAX List of Ensembl Coordinate Systems.
** @attr TopLevelCoordsystem [EnsPCoordsystem]
**                            Top-level Ensembl Coordinate System.
** @attr OtherCoordsystem [EnsPCoordsystem] Other Ensembl Coordinate System.
** @attr Use [ajuint] Use counter.
** @attr Padding [ajuint] Padding to alignment boundary.
** @@
******************************************************************************/

typedef struct EnsSToplevelassemblymapper {
    EnsPAssemblymapperadaptor Adaptor;
    AjPList Coordsystems;
    EnsPCoordsystem TopLevelCoordsystem;
    EnsPCoordsystem OtherCoordsystem;
    ajuint Use;
    ajuint Padding;
} EnsOToplevelassemblymapper;

#define EnsPToplevelassemblymapper EnsOToplevelassemblymapper*




/* @data EnsPAssemblymapper ***************************************************
**
** Ensembl Assembly Mapper.
**
** The Ensembl Assembly Mapper is a wrapper for specialised Assembly Mappers.
**
** @alias EnsSAssemblymapper
** @alias EnsOAssemblymapper
**
** @attr Adaptor [EnsPAssemblymapperadaptor] Ensembl Assembly Mapper Adaptor.
** @attr Generic [EnsPGenericassemblymapper] Ensembl Generic Assembly Mapper.
** @attr Chained [EnsPChainedassemblymapper] Ensembl Chained Assembly Mapper.
** @attr TopLevel [EnsPToplevelassemblymapper] Ensembl Top-Level
**                                            Assembly Mapper.
** @attr Use [ajuint] Use counter.
** @attr Padding [ajuint] Padding to alignment boundary.
** @@
** This object subsumes the following Perl objects all returned by the
** Bio::EnsEMBL::DBSQL::Assemblymapperadaptor
**
** Bio::EnsEMBL::Assemblymapper (here Ensembl Generic Assembly Mapper)
** Bio::EnsEMBL::Chainedassemblymapper
** Bio::EnsEMBL::Toplevelassemblymapper
**
** The objects can be distinguished by their Type element.
******************************************************************************/

typedef struct EnsSAssemblymapper {
    EnsPAssemblymapperadaptor Adaptor;
    EnsPGenericassemblymapper Generic;
    EnsPChainedassemblymapper Chained;
    EnsPToplevelassemblymapper TopLevel;
    ajuint Use;
    ajuint Padding;
} EnsOAssemblymapper;

#define EnsPAssemblymapper EnsOAssemblymapper*




/* @data EnsPBaseadaptorLeftJoin **********************************************
**
** Ensembl Base Adaptor Left Join Statement.
**
** @alias EnsSBaseadaptorLeftJoin
** @alias EnsOBaseadaptorLeftJoin
**
** @attr Table [const char*] SQL Table
** @attr Condition [const char*] SQL Condition for a LEFT JOIN statement
** @@
******************************************************************************/

typedef struct EnsSBaseadaptorLeftJoin {
    const char *Table;
    const char *Condition;
} EnsOBaseadaptorLeftJoin;

#define EnsPBaseadaptorLeftJoin EnsOBaseadaptorLeftJoin*




/* @data EnsPBaseadaptor ******************************************************
**
** Ensembl Base Adaptor.
**
** @alias EnsSBaseadaptor
** @alias EnsOBaseadaptor
**
** @attr Adaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor.
** @attr Tables [const char**] One dimensional array of table name character
**                              strings
** @attr Columns [const char**] One dimensional array of column name character
**                               strings
** @attr LeftJoin [EnsOBaseadaptorLeftJoin*] Address of an array of
**                                   Ensembl Base Adaptor Left Join Statements
** @attr DefaultCondition [const char*] Default condition in a
**                                       SQL SELECT statement
** @attr FinalCondition [const char*] Final condition in a
**                                     SQL SELECT statement
** @attr StraightJoin [AjBool] Set the MySQL-specific STRAIGHT_JOIN option in a
**                             SQL SELECT statement
** @attr Padding [ajuint] Padding to alignment boundary
** @attr Query [(AjBool*)] Function pointer to an Ensembl object-spcific Query
**                        function, consolidating SQL results into Ensembl
**                        objects
** @@
** NOTE: In constrast to the Perl API, this implementation does not have a
** species_id element, which is already available from the Database Adaptor.
******************************************************************************/

typedef struct EnsSBaseadaptor {
    EnsPDatabaseadaptor Adaptor;
    const char **Tables;
    const char **Columns;
    EnsOBaseadaptorLeftJoin *LeftJoin;
    const char *DefaultCondition;
    const char *FinalCondition;
    AjBool StraightJoin;
    ajuint Padding;
    AjBool (*Query)(EnsPDatabaseadaptor dba,
		    const AjPStr sql,
		    EnsPAssemblymapper mapper,
		    EnsPSlice slice,
		    AjPList objects);
} EnsOBaseadaptor;

#define EnsPBaseadaptor EnsOBaseadaptor*




/* @data EnsPAnalysisadaptor **************************************************
**
** Ensembl Analysis Adaptor.
**
** @alias EnsSAnalysisadaptor
** @alias EnsOAnalysisadaptor
**
** @attr Adaptor [EnsPBaseadaptor] Ensembl Base Adaptor
** @attr CacheByIdentifier [AjPTable] Identifier cache
** @attr CacheByName [AjPTable] Name cache
** @@
******************************************************************************/

typedef struct EnsSAnalysisadaptor {
    EnsPBaseadaptor Adaptor;
    AjPTable CacheByIdentifier;
    AjPTable CacheByName;
} EnsOAnalysisadaptor;

#define EnsPAnalysisadaptor EnsOAnalysisadaptor*




/* @data EnsPAnalysis *********************************************************
**
** Ensembl Analysis.
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
** @attr CreationDate [AjPStr] Creation date
** @attr Name [AjPStr] Name
** @attr DatabaseName [AjPStr] Database name
** @attr DatabaseVersion [AjPStr] Database version
** @attr DatabaseFile [AjPStr] Database file
** @attr ProgramName [AjPStr] Program name
** @attr ProgramVersion [AjPStr] Program version
** @attr ProgramFile [AjPStr] Program file
** @attr Parameters [AjPStr] Parameters
** @attr ModuleName [AjPStr] Module name
** @attr ModuleVersion [AjPStr] Module version
** @attr GFFSource [AjPStr] GFF source
** @attr GFFFeature [AjPStr] GFF feature
** @cc 'analysis_description' SQL table
** @attr Description [AjPStr] Description
** @attr DisplayLabel [AjPStr] Display label for the Ensembl website
** @attr WebData [AjPStr] Configuration information for the Ensembl web site
** @attr Displayable [AjBool] Displayable on the Ensembl web site
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSAnalysis {
    ajuint Use;
    ajuint Identifier;
    EnsPAnalysisadaptor Adaptor;
    AjPStr CreationDate;
    AjPStr Name;
    AjPStr DatabaseName;
    AjPStr DatabaseVersion;
    AjPStr DatabaseFile;
    AjPStr ProgramName;
    AjPStr ProgramVersion;
    AjPStr ProgramFile;
    AjPStr Parameters;
    AjPStr ModuleName;
    AjPStr ModuleVersion;
    AjPStr GFFSource;
    AjPStr GFFFeature;
    AjPStr Description;
    AjPStr DisplayLabel;
    AjPStr WebData;
    AjBool Displayable;
    ajuint Padding;
} EnsOAnalysis;

#define EnsPAnalysis EnsOAnalysis*




/* @data EnsPExternaldatabaseadaptor ******************************************
**
** Ensembl External Database Adaptor.
**
** @alias EnsSExternaldatabaseadaptor
** @alias EnsOExternaldatabaseadaptor
**
** @attr Adaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @attr CacheByIdentifier [AjPTable] Identifier cache
** @attr CacheByName [AjPTable] Name cache
** @@
******************************************************************************/

typedef struct EnsSExternaldatabaseadaptor {
    EnsPDatabaseadaptor Adaptor;
    AjPTable CacheByIdentifier;
    AjPTable CacheByName;
} EnsOExternaldatabaseadaptor;

#define EnsPExternaldatabaseadaptor EnsOExternaldatabaseadaptor*




/******************************************************************************
**
** Ensembl External Database Status enumeration.
**
******************************************************************************/

enum EnsEExternaldatabaseStatus
{
    ensEExternaldatabaseStatusNULL,
    ensEExternaldatabaseStatusKnownXref,
    ensEExternaldatabaseStatusKnown,
    ensEExternaldatabaseStatusXref,
    ensEExternaldatabaseStatusPred,
    ensEExternaldatabaseStatusOrth,
    ensEExternaldatabaseStatusPseudo
};




/******************************************************************************
**
** Ensembl External Database Type enumeration.
**
******************************************************************************/

enum EnsEExternaldatabaseType
{
    ensEExternaldatabaseTypeNULL,
    ensEExternaldatabaseTypeArray,
    ensEExternaldatabaseTypeAltTrans,
    ensEExternaldatabaseTypeMisc,
    ensEExternaldatabaseTypeLit,
    ensEExternaldatabaseTypePrimaryDbSynonym,
    ensEExternaldatabaseTypeEnsembl
};




/* @data EnsPExternaldatabase *************************************************
**
** Ensembl External Database.
**
** @alias EnsSExternaldatabase
** @alias EnsOExternaldatabase
**
** @attr Use [ajuint] Use counter.
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] Internal SQL database identifier (primary key).
** @attr Adaptor [EnsPExternaldatabaseadaptor] Ensembl External
**                                             Database Adaptor
** @cc 'external_db' table
** @attr Name [AjPStr] Database name
** @attr Release [AjPStr] Database release
** @attr DisplayName [AjPStr] Database display name
** @attr SecondaryName [AjPStr] Secondary database name
** @attr SecondaryTable [AjPStr] Secondary database table
** @attr Description [AjPStr] Description
** @attr PrimaryIdIsLinkable [AjBool] Primary identifier is linkable
** @attr DisplayIdIsLinkable [AjBool] Display identifier is linkable
** @attr Status [AjEnum] Status
** @attr Type [AjEnum] Type
** @attr Priority [ajint] Priority
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSExternaldatabase {
    ajuint Use;
    ajuint Identifier;
    EnsPExternaldatabaseadaptor Adaptor;
    AjPStr Name;
    AjPStr Release;
    AjPStr DisplayName;
    AjPStr SecondaryName;
    AjPStr SecondaryTable;
    AjPStr Description;
    AjBool PrimaryIdIsLinkable;
    AjBool DisplayIdIsLinkable;
    AjEnum Status;
    AjEnum Type;
    ajint Priority;
    ajuint Padding;
} EnsOExternaldatabase;

#define EnsPExternaldatabase EnsOExternaldatabase*




/******************************************************************************
**
** Ensembl External Reference InfoType enumeration.
**
******************************************************************************/

enum EnsEExternalreferenceInfoType
{
    ensEExternalreferenceInfoTypeNULL,
    ensEExternalreferenceInfoTypeProjection,
    ensEExternalreferenceInfoTypeMisc,
    ensEExternalreferenceInfoTypeDependent,
    ensEExternalreferenceInfoTypeDirect,
    ensEExternalreferenceInfoTypeSequenceMatch,
    ensEExternalreferenceInfoTypeInferrredPair,
    ensEExternalreferenceInfoTypeProbe,
    ensEExternalreferenceInfoTypeUnmapped,
    ensEExternalreferenceInfoTypeCoordinateOverlap
};




/* @data EnsPExternalreference ************************************************
**
** Ensembl External Reference.
**
** @alias EnsSExternalreference
** @alias EnsOExternalreference
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] Internal SQL database identifier (primary key)
** @attr Analysis [EnsPAnalysis] Ensembl Analysis
** @cc 'xref' table
** @attr Externaldatabase [EnsPExternaldatabase] Ensembl External Database
** @attr PrimaryIdentifier [AjPStr] Primary identifier
** @attr DisplayIdentifier [AjPStr] Display identifier
** @attr Version [AjPStr] Version
** @attr Description [AjPStr] Description
** @attr LinkageAnnotation [AjPStr] Linkage annotation
** @attr InfoText [AjPStr] Information text
** @attr InfoType [AjEnum] Information type
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSExternalreference {
    ajuint Use;
    ajuint Identifier;
    EnsPAnalysis Analysis;
    EnsPExternaldatabase Externaldatabase;
    AjPStr PrimaryIdentifier;
    AjPStr DisplayIdentifier;
    AjPStr Version;
    AjPStr Description;
    AjPStr LinkageAnnotation;
    AjPStr InfoText;
    AjEnum InfoType;
    ajuint Padding;
} EnsOExternalreference;

#define EnsPExternalreference EnsOExternalreference*
/*  EnsPExternalreferenceadaptor Adaptor; */




/* @data EnsPIdentityreference ************************************************
**
** Ensembl Identity Reference.
**
** @alias EnsSIdentityreference
** @alias EnsOIdentityreference
**
** @cc Bio::EnsEMBL::IdentityXref
** @cc 'identity_xref' table
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

typedef struct EnsSIdentityreference {
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




/* @data EnsPDatabaseentryadaptor *********************************************
**
** Ensembl Database Entry Adaptor.
**
** @alias EnsSDatabaseentryadaptor
** @alias EnsODatabaseentryadaptor
**
** @attr Adaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @@
******************************************************************************/

typedef struct EnsSDatabaseentryadaptor {
    EnsPDatabaseadaptor Adaptor;
} EnsODatabaseentryadaptor;

#define EnsPDatabaseentryadaptor EnsODatabaseentryadaptor*




/* @data EnsPDatabaseentry ****************************************************
**
** Ensembl Database Entry.
**
** @alias EnsSDatabaseentry
** @alias EnsODatabaseentry
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] Internal SQL database identifier (primary key)
** @attr Adaptor [EnsPDatabaseentryadaptor] Ensembl Database Entry Adaptor
** @cc Bio::EnsEMBL::DbEntry
** @cc 'xref' table
** @attr Externalreference [EnsPExternalreference] Ensembl External Reference
** @cc Bio::EnsEMBL::IdentityXref
** @attr Identityreference [EnsPIdentityreference] Ensembl Identity Reference
** @cc 'external_synonym' table
** @attr Synonyms [AjPList] Synonyms
** @attr GoLinkageTypes [AjPList] Gene Ontology linkage types
** @@
******************************************************************************/

typedef struct EnsSDatabaseentry {
    ajuint Use;
    ajuint Identifier;
    EnsPDatabaseentryadaptor Adaptor;
    EnsPExternalreference Externalreference;
    EnsPIdentityreference Identityreference;
    AjPList Synonyms;
    AjPList GoLinkageTypes;
} EnsODatabaseentry;

#define EnsPDatabaseentry EnsODatabaseentry*




/* @data EnsPGeneontologylinkage **********************************************
**
** Ensembl Gene Ontology Linkage.
**
** @alias EnsSGeneontologylinkage
** @alias EnsOGeneontologylinkage
**
** @cc 'go_xref' table
** @attr LinkageType [AjPStr] Likage type (Gene Ontology Evidence Code)
** @attr Source [EnsPDatabaseentry] Source Ensembl Database Entry
** @attr Use [ajuint] Use counter
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSGeneontologylinkage {
    AjPStr LinkageType;
    EnsPDatabaseentry Source;
    ajuint Use;
    ajuint Padding;
} EnsOGeneontologylinkage;

#define EnsPGeneontologylinkage EnsOGeneontologylinkage*




/* @data EnsPFeature **********************************************************
**
** Ensembl Feature.
**
** @alias EnsSFeature
** @alias EnsOFeature
**
** @cc Bio::EnsEMBL::Feature
** @attr Analysis [EnsPAnalysis] Ensembl Analysis
** @attr Slice [EnsPSlice] Ensembl Slice
** @attr SequenceName [AjPStr] Sequence name
** @attr Start [ajint] Start coordinate
** @attr End [ajint] End coordinate
** @attr Strand [ajint] Strand orientation
** @attr Use [ajuint] Use counter
** @@
******************************************************************************/

typedef struct EnsSFeature {
    EnsPAnalysis Analysis;
    EnsPSlice Slice;
    AjPStr SequenceName;
    ajint Start;
    ajint End;
    ajint Strand;
    ajuint Use;
} EnsOFeature;

#define EnsPFeature EnsOFeature*




/* @data EnsPFeatureadaptor ***************************************************
**
** Ensembl Feature Adaptor.
**
** @alias EnsSFeatureadaptor
** @alias EnsOFeatureadaptor
**
** @attr Adaptor [EnsPBaseadaptor] Ensembl Base Adaptor
** @attr Cache [EnsPCache] Ensembl LRU Cache
** @attr Tables [char**] Tables
** @attr Condition [char*] Default SQL condition
** @attr GetFeature [(EnsPFeature*)] Ensembl Object-specific 'GetFeature'
**                                  element function
** @attr Reference [(void**)] Ensembl Object-specific referencing function
** @attr Delete [(void*)] Ensembl Object-specific deletion function
** @attr MaxFeatureLength [ajuint] Maximum length of a particular Feature type
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSFeatureadaptor {
    EnsPBaseadaptor Adaptor;
    EnsPCache Cache;
    char **Tables;
    char *Condition;
    EnsPFeature (*GetFeature)(const void *object);
    void* (*Reference)(void* value);
    void (*Delete)(void** value);
    ajuint MaxFeatureLength;
    ajuint Padding;
} EnsOFeatureadaptor;

#define EnsPFeatureadaptor EnsOFeatureadaptor*




/* @data EnsPExonadaptor ******************************************************
**
** Ensembl Exon Adaptor.
**
** @alias EnsSExonadaptor
** @alias EnsOExonadaptor
**
** @cc Bio::EnsEMBL::DBSQL::BaseFeatureadaptor
** @attr Adaptor [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @@
******************************************************************************/

typedef struct EnsSExonadaptor {
    EnsPFeatureadaptor Adaptor;
} EnsOExonadaptor;

#define EnsPExonadaptor EnsOExonadaptor*




/* @data EnsPExon *************************************************************
**
** Ensembl Exon.
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
** @cc 'exon' SQL table
** @attr StartPhase [ajint] Exon start phase
** @attr EndPhase [ajint] Exon end phase
** @attr Current [AjBool] Current
** @attr Constitutive [AjBool] Consitutive
** @cc 'exon_stable_id' SQL table
** @attr StableIdentifier [AjPStr] Stable identifier
** @attr Version [ajuint] Version
** @attr Padding [ajuint] Padding to alignment boundary
** @attr CreationDate [AjPStr] Creation date
** @attr ModificationDate [AjPStr] Modification date
** @cc Additional elements
** @attr SequenceCache [AjPStr] Sequence Cache
** @attr Supportingfeatures [AjPList] AJAX List of Ensembl Base Align Features
** @attr Coordinates [AjPTable] Exon Coordinates indexed by
**                              Ensembl Transcript identifiers
** @@
******************************************************************************/

typedef struct EnsSExon {
    ajuint Use;
    ajuint Identifier;
    EnsPExonadaptor Adaptor;
    EnsPFeature Feature;
    ajint StartPhase;
    ajint EndPhase;
    AjBool Current;
    AjBool Constitutive;
    AjPStr StableIdentifier;
    ajuint Version;
    ajuint Padding;
    AjPStr CreationDate;
    AjPStr ModificationDate;
    AjPStr SequenceCache;
    AjPList Supportingfeatures;
    AjPTable Coordinates;
} EnsOExon;

#define EnsPExon EnsOExon*




/* @data EnsPIntron ***********************************************************
**
** Ensembl Intron.
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

typedef struct EnsSIntron {
    EnsPFeature Feature;
    EnsPExon PreviousExon;
    EnsPExon NextExon;
    ajuint Use;
    ajuint Padding;
} EnsOIntron;

#define EnsPIntron EnsOIntron*




/* @data EnsPTranscriptadaptor ************************************************
**
** Ensembl Transcript Adaptor.
**
** @alias EnsSTranscriptadaptor
** @alias EnsOTranscriptadaptor
**
** @cc Bio::EnsEMBL::DBSQL::BaseFeatureadaptor
** @attr Adaptor [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @@
******************************************************************************/

typedef struct EnsSTranscriptadaptor {
    EnsPFeatureadaptor Adaptor;
} EnsOTranscriptadaptor;

#define EnsPTranscriptadaptor EnsOTranscriptadaptor*



/******************************************************************************
**
** Ensembl Transcript Status enumeration.
**
******************************************************************************/

enum EnsETranscriptStatus
{
    ensETranscriptStatusNULL,
    ensETranscriptStatusKnown,
    ensETranscriptStatusNovel,
    ensETranscriptStatusPutative,
    ensETranscriptStatusPredicted,
    ensETranscriptStatusKnownByProjection,
    ensETranscriptStatusUnknown
};




/* @data EnsPTranscript *******************************************************
**
** Ensembl Transcript.
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
** @cc 'transcript' SQL table
** @attr DisplayReference [EnsPDatabaseentry] Display External Reference
** @attr Description [AjPStr] Description
** @attr BioType [AjPStr] Biological type
** @attr Status [AjEnum] Status
** @attr Current [AjBool] Current attribute
** @cc 'transcript_stable_id' SQL table
** @attr StableIdentifier [AjPStr] Stable identifier
** @attr CreationDate [AjPStr] Creation date
** @attr ModificationDate [AjPStr] Modification date
** @attr Version [ajuint] Version
** @cc Additional elements
** @attr GeneIdentifier [ajuint] Ensembl Gene identifier
** @attr Attributes [AjPList] AJAX List of Ensembl Attributes
** @attr DatabaseEntries [AjPList] AJAX List of Ensembl Database Entries
** @attr Exons [AjPList] AJAX List of Ensembl Exons
** @attr Supportingfeatures [AjPList] AJAX List of Ensembl Base Align Features
** @attr Translations [AjPList] Ensembl Translations (weak references)
** @attr SliceCodingStart [ajuint] Coding start in Slice coordinates
** @attr SliceCodingEnd [ajuint] Coding end in Slice coordinates
** @attr TranscriptCodingStart [ajuint] Coding start in Transcript coordinates
** @attr TranscriptCodingEnd [ajuint] Coding end in Transcript coordinates
** @attr EditsEnabled [AjBool] Sequence Edits are enabled
** @attr StartPhase [ajint] Start phase
** @cc Bio::EnsEMBL::TranscriptMapper
** @attr ExonCoordMapper [EnsPMapper] Ensembl Mapper
** @@
** The Attribute AJAX Table is a first-level Table of Ensembl Attribute code
** AJAX String keys and AjPTable second-level AJAX Lists as value.
******************************************************************************/

typedef struct EnsSTranscript {
    ajuint Use;
    ajuint Identifier;
    EnsPTranscriptadaptor Adaptor;
    EnsPFeature Feature;
    EnsPDatabaseentry DisplayReference;
    AjPStr Description;
    AjPStr BioType;
    AjEnum Status;
    AjBool Current;
    AjPStr StableIdentifier;
    AjPStr CreationDate;
    AjPStr ModificationDate;
    ajuint Version;
    ajuint GeneIdentifier;
    AjPList Attributes;
    AjPList DatabaseEntries;
    AjPList Exons;
    AjPList Supportingfeatures;
    AjPList Translations;
    ajuint SliceCodingStart;
    ajuint SliceCodingEnd;
    ajuint TranscriptCodingStart;
    ajuint TranscriptCodingEnd;
    AjBool EditsEnabled;
    ajint StartPhase;
    EnsPMapper ExonCoordMapper;
} EnsOTranscript;

#define EnsPTranscript EnsOTranscript*




/* @data EnsPTranslationadaptor ***********************************************
**
** Ensembl Translation Adaptor.
**
** @alias EnsSTranslationadaptor
** @alias EnsOTranslationadaptor
**
** @cc Bio::EnsEMBL::DBSQL::Baseadaptor
** @attr Adaptor [EnsPBaseadaptor] Ensembl Base Adaptor
** @@
******************************************************************************/

typedef struct EnsSTranslationadaptor {
    EnsPBaseadaptor Adaptor;
} EnsOTranslationadaptor;

#define EnsPTranslationadaptor EnsOTranslationadaptor*




/* @data EnsPTranslation ******************************************************
**
** Ensembl Translation.
**
** @alias EnsSTranslation
** @alias EnsOTranslation
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] Internal SQL database identifier (primary key)
** @attr Adaptor [EnsPTranslationadaptor] Ensembl Translation Adaptor
** @cc Bio::EnsEMBL::Translation
** @cc SQL table 'translation' elements
** @attr Transcript [EnsPTranscript] Ensembl Transcript
** @attr StartExon [EnsPExon] Ensembl Exon in which the Translation
**                            start coordinate is annotated
** @attr EndExon [EnsPExon] Ensembl Exon in which the Translation
**                          end coordinate is annotated
** @attr Start [ajuint] Start coordinate relative to start Ensembl Exon
** @attr End [ajuint] End coordinate relative to end Ensembl Exon
** @cc  SQL table 'translation_stable_id' elements
** @attr StableIdentifier [AjPStr] Stable identifier
** @attr CreationDate [AjPStr] Creation date
** @attr ModificationDate [AjPStr] Modification date
** @attr Version [ajuint] Version
** @attr Padding [ajuint] Padding to alignment boundary
** @cc Additional elements
** @attr Attributes [AjPList] AJAX List of Ensembl Attributes
** @attr DatabaseEntries [AjPList] AJAX List of Ensembl Database Entries
** @attr Proteinfeatures [AjPList] AJAX List of Ensembl Protein Features
** @attr Sequence [AjPStr] Translation sequence
** @attr TranscriptStart [ajuint] Start coordinate relative to
**                                Ensembl Transcript (cDNA)
** @attr TranscriptEnd [ajuint] End coordinate relative to
**                              Ensembl Transcript (cDNA)
** @attr SliceStart [ajuint] Start coordinate relative to the Slice
** @attr SliceEnd [ajuint] End coordinate relative to the Slice
** @@
******************************************************************************/

typedef struct EnsSTranslation {
    ajuint Use;
    ajuint Identifier;
    EnsPTranslationadaptor Adaptor;
    EnsPTranscript Transcript;
    EnsPExon StartExon;
    EnsPExon EndExon;
    ajuint Start;
    ajuint End;
    AjPStr StableIdentifier;
    AjPStr CreationDate;
    AjPStr ModificationDate;
    ajuint Version;
    ajuint Padding;
    AjPList Attributes;
    AjPList DatabaseEntries;
    AjPList Proteinfeatures;
    AjPStr Sequence;
    ajuint TranscriptStart;
    ajuint TranscriptEnd;
    ajuint SliceStart;
    ajuint SliceEnd;
} EnsOTranslation;

#define EnsPTranslation EnsOTranslation*




/* @data EnsPGeneadaptor ******************************************************
**
** Ensembl Gene Adaptor.
**
** @alias EnsSGeneadaptor
** @alias EnsOGeneadaptor
**
** @cc Bio::EnsEMBL::DBSQL::BaseFeatureadaptor
** @attr Adaptor [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @@
******************************************************************************/

typedef struct EnsSGeneadaptor {
    EnsPFeatureadaptor Adaptor;
} EnsOGeneadaptor;

#define EnsPGeneadaptor EnsOGeneadaptor*




/******************************************************************************
**
** Ensembl Gene Status enumeration.
**
******************************************************************************/

enum EnsEGeneStatus
{
    ensEGeneStatusNULL,
    ensEGeneStatusKnown,
    ensEGeneStatusNovel,
    ensEGeneStatusPutative,
    ensEGeneStatusPredicted,
    ensEGeneStatusKnownByProjection,
    ensEGeneStatusUnknown
};




/* @data EnsPGene *************************************************************
**
** Ensembl Gene.
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
** @cc 'gene' SQL table
** @attr DisplayReference [EnsPDatabaseentry] Display External Reference
** @attr Description [AjPStr] Description
** @attr Source [AjPStr] Source
** @attr BioType [AjPStr] Biological type
** @attr Status [AjEnum] Status
** @attr Current [AjBool] Current attribute
** @cc 'gene_stable_id' SQL table
** @attr StableIdentifier [AjPStr] Stable identifier
** @attr CreationDate [AjPStr] Creation date
** @attr ModificationDate [AjPStr] Modification date
** @attr Version [ajuint] Version
** @attr Padding [ajuint] Padding to alignment boundary
** @cc
** @attr Attributes [AjPList] AJAX List of Ensembl Attributes
** @attr DatabaseEntries [AjPList] AJAX List of Ensembl Database Entries
** @attr Transcripts [AjPList] AJAX List of Ensembl Transcripts
** @@
******************************************************************************/

typedef struct EnsSGene {
    ajuint Use;
    ajuint Identifier;
    EnsPGeneadaptor Adaptor;
    EnsPFeature Feature;
    EnsPDatabaseentry DisplayReference;
    AjPStr Description;
    AjPStr Source;
    AjPStr BioType;
    AjEnum Status;
    AjBool Current;
    AjPStr StableIdentifier;
    AjPStr CreationDate;
    AjPStr ModificationDate;
    ajuint Version;
    ajuint Padding;
    AjPList Attributes;
    AjPList DatabaseEntries;
    AjPList Transcripts;
} EnsOGene;

#define EnsPGene EnsOGene*




#endif

#ifdef __cplusplus
}
#endif

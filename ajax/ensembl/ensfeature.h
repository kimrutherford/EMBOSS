#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ensfeature_h
#define ensfeature_h

#include "ensdata.h"
#include "ensassemblyexception.h"
#include "ensdatabaseentry.h"
#include "ensmetainformation.h"
#include "ensmetacoordinate.h"
#include "ensslice.h"




/* @data EnsPFeaturepair ******************************************************
**
** Ensembl Feature Pair.
**
** @alias EnsSFeaturepair
** @alias EnsOFeaturepair
**
** @cc Bio::EnsEMBL::Featurepair
** @attr SourceFeature [EnsPFeature] Source Ensembl Feature (Query)
** @attr TargetFeature [EnsPFeature] Target Ensembl Feature (Target)
** @attr Externaldatabase [EnsPExternaldatabase] Ensembl External Database
** @attr ExtraData [AjPStr] Extra data
** @attr SourceSpecies [AjPStr] Source species name
** @attr TargetSpecies [AjPStr] Target species name
** @attr GroupIdentifier [ajuint] Group identifier
** @attr LevelIdentifier [ajuint] Level identifier
** @attr Evalue [double] e- or p-value
** @attr Score [double] Score
** @attr SourceCoverage [float] Source coverage in percent
** @attr TargetCoverage [float] Target coverage in percent
** @attr SequenceIdentity [float] Sequence identity in percent
** @attr Use [ajuint] Use counter
** @@
******************************************************************************/

typedef struct EnsSFeaturepair
{
    EnsPFeature SourceFeature;
    EnsPFeature TargetFeature;
    EnsPExternaldatabase Externaldatabase;
    AjPStr ExtraData;
    AjPStr SourceSpecies;
    AjPStr TargetSpecies;
    ajuint GroupIdentifier;
    ajuint LevelIdentifier;
    double Evalue;
    double Score;
    float SourceCoverage;
    float TargetCoverage;
    float SequenceIdentity;
    ajuint Use;
} EnsOFeaturepair;

#define EnsPFeaturepair EnsOFeaturepair*




/* @data EnsPDNAAlignFeatureadaptor *******************************************
**
** Ensembl DNA Align Feature Adaptor.
**
** @alias EnsSDNAAlignFeatureadaptor
** @alias EnsODNAAlignFeatureadaptor
**
** @attr Adaptor [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @@
******************************************************************************/

typedef struct EnsSDNAAlignFeatureadaptor
{
    EnsPFeatureadaptor Adaptor;
} EnsODNAAlignFeatureadaptor;

#define EnsPDNAAlignFeatureadaptor EnsODNAAlignFeatureadaptor*




/* @data EnsPProteinalignfeatureadaptor ***************************************
**
** Ensembl Protein Align Feature Adaptor.
**
** @alias EnsSProteinalignfeatureadaptor
** @alias EnsOProteinalignfeatureadaptor
**
** @attr Adaptor [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @@
******************************************************************************/

typedef struct EnsSProteinalignfeatureadaptor
{
    EnsPFeatureadaptor Adaptor;
} EnsOProteinalignfeatureadaptor;

#define EnsPProteinalignfeatureadaptor EnsOProteinalignfeatureadaptor*




/******************************************************************************
**
** Ensembl Base Align Feature Type enumeration.
**
******************************************************************************/

enum EnsEBasealignfeatureType
{
    ensEBasealignfeatureTypeNULL,
    ensEBasealignfeatureTypeDNA,
    ensEBasealignfeatureTypeProtein
};




/* @data EnsPBasealignfeature *************************************************
**
** Ensembl Base Align Feature.
**
** @alias EnsSBasealignfeature
** @alias EnsOBasealignfeature
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr DNAAlignFeatureadaptor [EnsPDNAAlignFeatureadaptor]
**                              Ensembl DNA Align Feature Adaptor
** @attr Proteinalignfeatureadaptor [EnsPProteinalignfeatureadaptor]
**                                  Ensembl Protein Align Feature Adaptor
** @cc Bio::EnsEMBL::Featurepair
** @attr Featurepair [EnsPFeaturepair] Ensembl Feature Pair
** @cc Bio::EnsEMBL::Basealignfeature
** @attr GetFeaturepair [(EnsPFeaturepair*)] Get Ensembl Feature Pair function
** @attr Cigar [AjPStr] CIGAR line
** @attr Type [AjEnum] Type (ensEBasealignfeatureTypeDNA, ...)
** @attr AlignmentLength [ajuint] Target component alignment length
** @attr PairDNAAlignFeatureIdentifier [ajuint] Pair DNA Align Feature
**                                              identifier
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSBasealignfeature
{
    ajuint Use;
    ajuint Identifier;
    EnsPDNAAlignFeatureadaptor DNAAlignFeatureadaptor;
    EnsPProteinalignfeatureadaptor Proteinalignfeatureadaptor;
    EnsPFeaturepair Featurepair;
    EnsPFeaturepair (*GetFeaturepair)(void* object);
    AjPStr Cigar;
    AjEnum Type;
    ajuint AlignmentLength;
    ajuint PairDNAAlignFeatureIdentifier;
    ajuint Padding;
} EnsOBasealignfeature;

#define EnsPBasealignfeature EnsOBasealignfeature*




/* @data EnsPProteinfeatureadaptor ********************************************
**
** Ensembl Protein Feature Adaptor.
**
** @alias EnsSProteinfeatureadaptor
** @alias EnsOProteinfeatureadaptor
**
** @attr Adaptor [EnsPBaseadaptor] Ensembl Base Adaptor
** @@
******************************************************************************/

typedef struct EnsSProteinfeatureadaptor
{
    EnsPBaseadaptor Adaptor;
} EnsOProteinfeatureadaptor;

#define EnsPProteinfeatureadaptor EnsOProteinfeatureadaptor*




/* @data EnsPProteinfeature ***************************************************
**
** Ensembl Protein Feature.
**
** @alias EnsSProteinfeature
** @alias EnsOProteinfeature
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPProteinfeatureadaptor] Ensembl Protein Feature Adaptor
** @cc Bio::EnsEMBL::Featurepair
** @attr Featurepair [EnsPFeaturepair] Ensembl Feature Pair
** @cc Bio::EnsEMBL::Proteinfeature
** @attr Accession [AjPStr] (InterPro) Accession
** @attr Description [AjPStr] (InterPro) Description
** @@
******************************************************************************/

typedef struct EnsSProteinfeature
{
    ajuint Use;
    ajuint Identifier;
    EnsPProteinfeatureadaptor Adaptor;
    EnsPFeaturepair Featurepair;
    AjPStr Accession;
    AjPStr Description;
} EnsOProteinfeature;

#define EnsPProteinfeature EnsOProteinfeature*




/* @data EnsPSimplefeatureadaptor *********************************************
**
** Ensembl Simple Feature Adaptor.
**
** @alias EnsSSimplefeatureadaptor
** @alias EnsOSimplefeatureadaptor
**
** @attr Adaptor [EnsPFeatureadaptor] Ensembl Feature Adaptor
** @@
******************************************************************************/

typedef struct EnsSSimplefeatureadaptor
{
    EnsPFeatureadaptor Adaptor;
} EnsOSimplefeatureadaptor;

#define EnsPSimplefeatureadaptor EnsOSimplefeatureadaptor*




/* @data EnsPSimplefeature ****************************************************
**
** Ensembl Simple Feature.
**
** @alias EnsSSimplefeature
** @alias EnsOSimplefeature
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPSimplefeatureadaptor] Ensembl Simple Feature Adaptor
** @cc Bio::EnsEMBL::Feature
** @attr Feature [EnsPFeature] Ensembl Feature
** @cc Bio::EnsEMBL::Simplefeature
** @attr DisplayLabel [AjPStr] Display label
** @attr Score [double] Score
** @@
******************************************************************************/

typedef struct EnsSSimplefeature
{
    ajuint Use;
    ajuint Identifier;
    EnsPSimplefeatureadaptor Adaptor;
    EnsPFeature Feature;
    AjPStr DisplayLabel;
    double Score;
} EnsOSimplefeature;

#define EnsPSimplefeature EnsOSimplefeature*




/* @data EnsPAssemblyexceptionfeatureadaptor **********************************
**
** Ensembl Assembly Exception Feature Adaptor.
**
** @alias EnsSAssemblyexceptionfeatureadaptor
** @alias EnsOAssemblyexceptionfeatureadaptor
**
** @attr Adaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @attr Cache [AjPList] AJAX List of Ensembl Assembly Exception Features
** @attr CacheByIdentifier [AjPTable] AJAX Table of Ensembl Assembly Exception
**                                    Features
** @attr CacheBySlice [EnsPCache] Ensembl Cache by Ensembl Slice names
** @@
******************************************************************************/

typedef struct EnsSAssemblyexceptionfeatureadaptor
{
    EnsPDatabaseadaptor Adaptor;
    AjPList Cache;
    AjPTable CacheByIdentifier;
    EnsPCache CacheBySlice;
} EnsOAssemblyexceptionfeatureadaptor;

#define EnsPAssemblyexceptionfeatureadaptor EnsOAssemblyexceptionfeatureadaptor*




/* @data EnsPAssemblyexceptionfeature *****************************************
**
** Ensembl Assembly Exception Feature.
**
** @alias EnsSAssemblyexceptionfeature
** @alias EnsOAssemblyexceptionfeature
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPAssemblyexceptionfeatureadaptor]
**                Ensembl Assembly Exception Feature Adaptor
** @cc Bio::EnsEMBL::Feature
** @attr Feature [EnsPFeature] Ensembl Feature
** @cc Bio::EnsEMBL::Assemblyexceptionfeature
** @attr AlternateSlice [EnsPSlice] Alternate Ensembl Slice
** @attr Type [AjEnum] Ensembl Assembly Exception Feature type
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSAssemblyexceptionfeature
{
    ajuint Use;
    ajuint Identifier;
    EnsPAssemblyexceptionfeatureadaptor Adaptor;
    EnsPFeature Feature;
    EnsPSlice AlternateSlice;
    AjEnum Type;
    ajuint Padding;
} EnsOAssemblyexceptionfeature;

#define EnsPAssemblyexceptionfeature EnsOAssemblyexceptionfeature*




/*
** Prototype definitions
*/

/* Ensembl Feature */

EnsPFeature ensFeatureNewS(EnsPAnalysis analysis,
                           EnsPSlice slice,
                           ajint start,
                           ajint end,
                           ajint strand);

EnsPFeature ensFeatureNewN(EnsPAnalysis analysis,
                           AjPStr seqname,
                           ajint start,
                           ajint end,
                           ajint strand);

EnsPFeature ensFeatureNewObj(const EnsPFeature object);

EnsPFeature ensFeatureNewRef(EnsPFeature feature);

void ensFeatureDel(EnsPFeature* Pfeature);

EnsPAnalysis ensFeatureGetAnalysis(const EnsPFeature feature);

EnsPSlice ensFeatureGetSlice(const EnsPFeature feature);

AjPStr ensFeatureGetSequenceName(const EnsPFeature feature);

ajint ensFeatureGetStart(const EnsPFeature feature);

ajint ensFeatureGetEnd(const EnsPFeature feature);

ajint ensFeatureGetStrand(const EnsPFeature feature);

AjBool ensFeatureSetAnalysis(EnsPFeature feature, EnsPAnalysis analysis);

AjBool ensFeatureSetSlice(EnsPFeature feature, EnsPSlice slice);

AjBool ensFeatureSetSequenceName(EnsPFeature, AjPStr seqname);

AjBool ensFeatureSetStart(EnsPFeature feature, ajint start);

AjBool ensFeatureSetEnd(EnsPFeature feature, ajint end);

AjBool ensFeatureSetStrand(EnsPFeature feature, ajint strand);

AjBool ensFeatureTrace(const EnsPFeature feature, ajuint level);

ajuint ensFeatureGetLength(const EnsPFeature feature);

ajuint ensFeatureGetMemSize(const EnsPFeature feature);

const EnsPSeqregion ensFeatureGetSeqregion(const EnsPFeature feature);

ajint ensFeatureGetSeqregionStart(const EnsPFeature feature);

ajint ensFeatureGetSeqregionEnd(const EnsPFeature feature);

ajint ensFeatureGetSeqregionStrand(const EnsPFeature feature);

const AjPStr ensFeatureGetSeqregionName(const EnsPFeature feature);

ajuint ensFeatureGetSeqregionLength(const EnsPFeature feature);

AjBool ensFeatureMove(EnsPFeature feature,
                      ajint start,
                      ajint end,
                      ajint strand);

AjBool ensFeatureProject(const EnsPFeature feature,
                         const AjPStr csname,
                         const AjPStr csversion,
                         AjPList pslist);

EnsPFeature ensFeatureTransform(EnsPFeature feature,
                                const AjPStr csname,
                                const AjPStr csversion);

EnsPFeature ensFeatureTransfer(EnsPFeature feature, EnsPSlice slice);

AjBool ensFeatureFetchAllAlternativeLocations(EnsPFeature feature,
                                              AjBool all,
                                              AjPList features);

AjBool ensFeatureOverlap(const EnsPFeature feature1,
                         const EnsPFeature feature2);

AjBool ensFeatureFetchSeqregionName(const EnsPFeature feature, AjPStr* Pname);

/* Ensembl Feature Adaptor */

EnsPFeatureadaptor ensFeatureadaptorNew(
    EnsPDatabaseadaptor dba,
    const char **Ptables,
    const char **Pcolumns,
    EnsOBaseadaptorLeftJoin *left_join,
    const char *condition,
    const char *final,
    AjBool Fquery(EnsPDatabaseadaptor dba,
                  const AjPStr statement,
                  EnsPAssemblymapper mapper,
                  EnsPSlice slice,
                  AjPList objects),
    void* Fread(const void* key),
    void* Freference(void* value),
    AjBool Fwrite(const void* value),
    void Fdelete(void** value),
    ajuint Fsize(const void* value),
    EnsPFeature Fgetfeature(const void *object),
    const char *label);

void ensFeatureadaptorDel(EnsPFeatureadaptor *Padaptor);

EnsPBaseadaptor ensFeatureadaptorGetBaseadaptor(
    const EnsPFeatureadaptor adaptor);

EnsPCache ensFeatureadaptorGetCache(const EnsPFeatureadaptor adaptor);

ajuint ensFeatureadaptorGetMaxFeatureLength(const EnsPFeatureadaptor adaptor);

EnsPDatabaseadaptor ensFeatureadaptorGetDatabaseadaptor(
    const EnsPFeatureadaptor adaptor);

AjBool ensFeatureadaptorSetMaximumFeatureLength(EnsPFeatureadaptor adaptor,
                                                ajuint length);

AjBool ensFeatureadaptorSetTables(EnsPFeatureadaptor adaptor,
                                  const char **Ptables);

AjBool ensFeatureadaptorSetColumns(EnsPFeatureadaptor adaptor,
                                   const char **Pcolumns);

AjBool ensFeatureadaptorSetDefaultCondition(EnsPFeatureadaptor adaptor,
                                            const char *condition);

AjBool ensFeatureadaptorSetFinalCondition(EnsPFeatureadaptor adaptor,
                                          const char *final);

AjBool ensFeatureadaptorEscapeC(EnsPFeatureadaptor adaptor,
                                 char **Ptxt,
                                 const AjPStr str);

AjBool ensFeatureadaptorEscapeS(EnsPFeatureadaptor adaptor,
                                 AjPStr *Pstr,
                                 const AjPStr str);

AjBool ensFeatureadaptorFetchAllBySliceConstraint(EnsPFeatureadaptor adaptor,
                                                  EnsPSlice slice,
                                                  const AjPStr constraint,
                                                  const AjPStr anname,
                                                  AjPList objects);

AjBool ensFeatureadaptorFetchAllBySlice(EnsPFeatureadaptor adaptor,
                                        EnsPSlice slice,
                                        const AjPStr anname,
                                        AjPList objects);

AjBool ensFeatureadaptorFetchAllBySliceScore(EnsPFeatureadaptor adaptor,
                                             EnsPSlice slice,
                                             double score,
                                             const AjPStr anname,
                                             AjPList objects);

AjBool ensFeatureadaptorFetchAllByAnalysisName(EnsPFeatureadaptor adaptor,
                                               const AjPStr anname,
                                               AjPList objects);

/* Ensembl Feature Pair */

EnsPFeaturepair ensFeaturepairNew(EnsPFeature srcfeature,
                                  EnsPFeature trgfeature,
                                  EnsPExternaldatabase edb,
                                  AjPStr extra,
                                  AjPStr srcspecies,
                                  AjPStr trgspecies,
                                  ajuint groupid,
                                  ajuint levelid,
                                  double evalue,
                                  double score,
                                  float srccoverage,
                                  float trgcoverage,
                                  float identity);

EnsPFeaturepair ensFeaturepairNewObj(const EnsPFeaturepair object);

EnsPFeaturepair ensFeaturepairNewRef(EnsPFeaturepair fp);

void ensFeaturepairDel(EnsPFeaturepair* Pfp);

EnsPFeature ensFeaturepairGetSourceFeature(const EnsPFeaturepair fp);

EnsPFeature ensFeaturepairGetTargetFeature(const EnsPFeaturepair fp);

EnsPExternaldatabase ensFeaturepairGetExternaldatabase(
    const EnsPFeaturepair fp);

AjPStr ensFeaturepairGetExtraData(const EnsPFeaturepair fp);

AjPStr ensFeaturepairGetSourceSpecies(const EnsPFeaturepair fp);

AjPStr ensFeaturepairGetTargetSpecies(const EnsPFeaturepair fp);

ajuint ensFeaturepairGetGroupIdentifier(const EnsPFeaturepair fp);

ajuint ensFeaturepairGetLevelIdentifier(const EnsPFeaturepair fp);

double ensFeaturepairGetEvalue(const EnsPFeaturepair fp);

double ensFeaturepairGetScore(const EnsPFeaturepair fp);

float ensFeaturepairGetSourceCoverage(const EnsPFeaturepair fp);

float ensFeaturepairGetTargetCoverage(const EnsPFeaturepair fp);

float ensFeaturepairGetSequenceIdentity(const EnsPFeaturepair fp);

AjBool ensFeaturepairSetSourceFeature(EnsPFeaturepair fp, EnsPFeature feature);

AjBool ensFeaturepairSetTargetFeature(EnsPFeaturepair fp, EnsPFeature feature);

AjBool ensFeaturepairSetExternaldatabase(EnsPFeaturepair fp,
                                         EnsPExternaldatabase edb);

AjBool ensFeaturepairSetExtraData(EnsPFeaturepair fp, AjPStr extra);

AjBool ensFeaturepairSetSourceSpecies(EnsPFeaturepair fp, AjPStr species);

AjBool ensFeaturepairSetTargetSpecies(EnsPFeaturepair fp, AjPStr species);

AjBool ensFeaturepairSetGroupIdentifier(EnsPFeaturepair fp, ajuint groupid);

AjBool ensFeaturepairSetLevelIdentifier(EnsPFeaturepair fp, ajuint levelid);

AjBool ensFeaturepairSetEvalue(EnsPFeaturepair fp, double evalue);

AjBool ensFeaturepairSetScore(EnsPFeaturepair fp, double score);

AjBool ensFeaturepairSetSourceCoverage(EnsPFeaturepair fp, float coverage);

AjBool ensFeaturepairSetTargetCoverage(EnsPFeaturepair fp, float coverage);

AjBool ensFeaturepairTrace(const EnsPFeaturepair fp, ajuint level);

AjBool ensFeaturepairSetSequenceIdentity(EnsPFeaturepair fp, float identity);

ajuint ensFeaturepairGetMemSize(const EnsPFeaturepair fp);

EnsPFeaturepair ensFeaturepairTransform(const EnsPFeaturepair fp,
                                        const AjPStr csname,
                                        const AjPStr csversion);

EnsPFeaturepair ensFeaturepairTransfer(EnsPFeaturepair fp, EnsPSlice slice);

/* Ensembl Base Align Feature */

EnsPBasealignfeature ensBasealignfeatureNewD(
    EnsPDNAAlignFeatureadaptor adaptor,
    ajuint identifier,
    EnsPFeaturepair fp,
    AjPStr cigar,
    ajuint pair);

EnsPBasealignfeature ensBasealignfeatureNewP(
    EnsPProteinalignfeatureadaptor adaptor,
    ajuint identifier,
    EnsPFeaturepair fp,
    AjPStr cigar);

EnsPBasealignfeature ensBasealignfeatureNew(
    EnsPFeaturepair fp,
    AjPStr cigar,
    AjPList fps,
    AjEnum type,
    ajuint pair);

EnsPBasealignfeature ensBasealignfeatureNewObj(
    const EnsPBasealignfeature object);

EnsPBasealignfeature ensBasealignfeatureNewRef(EnsPBasealignfeature baf);

void ensBasealignfeatureDel(EnsPBasealignfeature *Pbaf);

EnsPDNAAlignFeatureadaptor ensBasealignfeatureGetDNAAlignFeatureadaptor(
    const EnsPBasealignfeature baf);

EnsPProteinalignfeatureadaptor ensBasealignfeatureGetProteinalignfeatureadaptor(
    const EnsPBasealignfeature baf);

ajuint ensBasealignfeatureGetIdentifier(const EnsPBasealignfeature baf);

EnsPFeaturepair ensBasealignfeatureGetFeaturepair(
    const EnsPBasealignfeature baf);

AjBool ensBasealignfeatureSetFeaturepair(EnsPBasealignfeature baf,
                                         EnsPFeaturepair fp);

EnsPFeature ensBasealignfeatureGetFeature(const EnsPBasealignfeature baf);

ajuint ensBasealignfeatureGetAlignmentLength(EnsPBasealignfeature baf);

void *ensBasealignfeatureGetAdaptor(const EnsPBasealignfeature baf);

ajuint ensBasealignfeatureGetSourceUnit(const EnsPBasealignfeature baf);

ajuint ensBasealignfeatureGetTargetUnit(const EnsPBasealignfeature baf);

ajuint ensBasealignfeatureGetMemSize(const EnsPBasealignfeature baf);

EnsPBasealignfeature ensBasealignfeatureTransform(
    const EnsPBasealignfeature baf,
    const AjPStr csname,
    const AjPStr csversion);

EnsPBasealignfeature ensBasealignfeatureTransfer(EnsPBasealignfeature baf,
                                                 EnsPSlice slice);

AjBool ensBasealignfeatureTrace(const EnsPBasealignfeature baf, ajuint level);

AjBool ensBasealignfeatureFetchAllFeaturepairs(const EnsPBasealignfeature baf,
                                               AjPList fps);

/* Ensembl DNA Align Feature Adaptor */

EnsPDNAAlignFeatureadaptor ensDNAAlignFeatureadaptorNew(
    EnsPDatabaseadaptor dba);

void ensDNAAlignFeatureadaptorDel(EnsPDNAAlignFeatureadaptor *Padaptor);

AjBool ensDNAAlignFeatureadaptorFetchByIdentifier(
    EnsPDNAAlignFeatureadaptor adaptor,
    ajuint identifier,
    EnsPBasealignfeature *Pbaf);

AjBool ensDNAAlignFeatureadaptorFetchAllBySliceIdentity(
    EnsPDNAAlignFeatureadaptor adaptor,
    EnsPSlice slice,
    float identity,
    const AjPStr anname,
    AjPList bafs);

AjBool ensDNAAlignFeatureadaptorFetchAllBySliceCoverage(
    EnsPDNAAlignFeatureadaptor adaptor,
    EnsPSlice slice,
    float coverage,
    const AjPStr anname,
    AjPList bafs);

AjBool ensDNAAlignFeatureadaptorFetchAllBySliceExternaldatabase(
    EnsPDNAAlignFeatureadaptor adaptor,
    EnsPSlice slice,
    const AjPStr edbname,
    const AjPStr anname,
    AjPList bafs);

AjBool ensDNAAlignFeatureadaptorFetchAllByHitName(
    EnsPDNAAlignFeatureadaptor adaptor,
    const AjPStr hitname,
    const AjPStr anname,
    AjPList bafs);

AjBool ensDNAAlignFeatureadaptorFetchAllByHitUnversioned(
    EnsPDNAAlignFeatureadaptor adaptor,
    const AjPStr hitname,
    const AjPStr anname,
    AjPList bafs);

AjBool ensDNAAlignFeatureadaptorFetchAllIdentifiers(
    EnsPDNAAlignFeatureadaptor adaptor,
    AjPList idlist);

/* Ensembl Protein Align Feature Adaptor */

EnsPProteinalignfeatureadaptor ensProteinalignfeatureadaptorNew(
    EnsPDatabaseadaptor dba);

void ensProteinalignfeatureadaptorDel(EnsPProteinalignfeatureadaptor *Padaptor);

AjBool ensProteinalignfeatureadaptorFetchByIdentifier(
    EnsPProteinalignfeatureadaptor adaptor,
    ajuint identifier,
    EnsPBasealignfeature *Pbaf);

AjBool ensProteinalignfeatureadaptorFetchAllBySliceIdentity(
    EnsPProteinalignfeatureadaptor adaptor,
    EnsPSlice slice,
    float identity,
    const AjPStr anname,
    AjPList bafs);

AjBool ensProteinalignfeatureadaptorFetchAllBySliceCoverage(
    EnsPProteinalignfeatureadaptor adaptor,
    EnsPSlice slice,
    float coverage,
    const AjPStr anname,
    AjPList bafs);

AjBool ensProteinalignfeatureadaptorFetchAllBySliceExternaldatabase(
    EnsPProteinalignfeatureadaptor adaptor,
    EnsPSlice slice,
    const AjPStr edbname,
    const AjPStr anname,
    AjPList bafs);

AjBool ensProteinalignfeatureadaptorFetchAllByHitName(
    EnsPProteinalignfeatureadaptor adaptor,
    const AjPStr hitname,
    const AjPStr anname,
    AjPList bafs);

AjBool ensProteinalignfeatureadaptorFetchAllByHitUnversioned(
    EnsPProteinalignfeatureadaptor adaptor,
    const AjPStr hitname,
    const AjPStr anname,
    AjPList bafs);

AjBool ensProteinalignfeatureadaptorFetchAllIdentifiers(
    EnsPProteinalignfeatureadaptor adaptor,
    AjPList idlist);

/* Ensembl Protein Feature */

EnsPProteinfeature ensProteinfeatureNew(
    EnsPProteinfeatureadaptor adaptor,
    ajuint identifier,
    EnsPFeaturepair fp,
    AjPStr accession,
    AjPStr description);

EnsPProteinfeature ensProteinfeatureNewObj(const EnsPProteinfeature object);

EnsPProteinfeature ensProteinfeatureNewRef(EnsPProteinfeature pf);

void ensProteinfeatureDel(EnsPProteinfeature *Ppf);

EnsPProteinfeatureadaptor ensProteinfeatureGetAdaptor(
    const EnsPProteinfeature pf);

ajuint ensProteinfeatureGetIdentifier(const EnsPProteinfeature pf);

EnsPFeaturepair ensProteinfeatureGetFeaturepair(const EnsPProteinfeature pf);

AjPStr ensProteinfeatureGetAccession(const EnsPProteinfeature pf);

AjPStr ensProteinfeatureGetDescription(const EnsPProteinfeature pf);

AjBool ensProteinfeatureSetAdaptor(EnsPProteinfeature pf,
                                   EnsPProteinfeatureadaptor adaptor);

AjBool ensProteinfeatureSetIdentifier(EnsPProteinfeature pf, ajuint identifier);

AjBool ensProteinfeatureSetFeaturepair(EnsPProteinfeature pf,
                                       EnsPFeaturepair fp);

AjBool ensProteinfeatureSetAccession(EnsPProteinfeature pf, AjPStr accession);

AjBool ensProteinfeatureSetDescription(EnsPProteinfeature pf,
                                       AjPStr description);

AjBool ensProteinfeatureTrace(const EnsPProteinfeature pf, ajuint level);

ajuint ensProteinfeatureGetMemSize(const EnsPProteinfeature pf);

/* Ensembl Protein Feature Adaptor */

EnsPProteinfeatureadaptor ensProteinfeatureadaptorNew(EnsPDatabaseadaptor dba);

void ensProteinfeatureadaptorDel(EnsPProteinfeatureadaptor *Padaptor);

AjBool ensProteinfeatureadaptorFetchAllByTranslationIdentifier(
    EnsPProteinfeatureadaptor adaptor,
    ajuint tlid,
    AjPList pfs);

AjBool ensProteinfeatureadaptorFetchByIdentifier(
    EnsPProteinfeatureadaptor adaptor,
    ajuint identifier,
    EnsPProteinfeature *Ppf);

/* Ensembl Simple Feature */

EnsPSimplefeature ensSimplefeatureNew(EnsPSimplefeatureadaptor adaptor,
                                      ajuint identifier,
                                      EnsPFeature feature,
                                      AjPStr label,
                                      double score);

EnsPSimplefeature ensSimplefeatureNewObj(const EnsPSimplefeature object);

EnsPSimplefeature ensSimplefeatureNewRef(EnsPSimplefeature sf);

void ensSimplefeatureDel(EnsPSimplefeature* Psf);

EnsPSimplefeatureadaptor ensSimplefeatureGetAdaptor(const EnsPSimplefeature sf);

ajuint ensSimplefeatureGetIdentifier(const EnsPSimplefeature sf);

EnsPFeature ensSimplefeatureGetFeature(const EnsPSimplefeature sf);

AjPStr ensSimplefeatureGetDisplayLabel(const EnsPSimplefeature sf);

double ensSimplefeatureGetScore(const EnsPSimplefeature sf);

AjBool ensSimplefeatureSetAdaptor(EnsPSimplefeature sf,
                                  EnsPSimplefeatureadaptor adaptor);

AjBool ensSimplefeatureSetIdentifier(EnsPSimplefeature sf, ajuint identifier);

AjBool ensSimplefeatureSetFeature(EnsPSimplefeature sf, EnsPFeature feature);

AjBool ensSimplefeatureSetDisplayLabel(EnsPSimplefeature sf, AjPStr label);

AjBool ensSimplefeatureSetScore(EnsPSimplefeature sf, double score);

AjBool ensSimplefeatureTrace(const EnsPSimplefeature sf, ajuint level);

ajuint ensSimplefeatureGetMemSize(const EnsPSimplefeature sf);

/* Ensembl Simple Feature Adaptor */

EnsPSimplefeatureadaptor ensSimplefeatureadaptorNew(EnsPDatabaseadaptor dba);

void ensSimplefeatureadaptorDel(EnsPSimplefeatureadaptor *Padaptor);

AjBool ensSimplefeatureadaptorFetchAllBySlice(EnsPSimplefeatureadaptor adaptor,
                                              EnsPSlice slice,
                                              const AjPStr anname,
                                              AjPList sfs);

AjBool ensSimplefeatureadaptorFetchAllBySliceScore(
    EnsPSimplefeatureadaptor adaptor,
    EnsPSlice slice,
    double score,
    const AjPStr anname,
    AjPList sfs);

AjBool ensSimplefeatureadaptorFetchAllByAnalysisName(
    EnsPSimplefeatureadaptor adaptor,
    const AjPStr anname,
    AjPList sfs);

/* Ensembl Assembly Exception Feature */

EnsPAssemblyexceptionfeature ensAssemblyexceptionfeatureNew(
    EnsPAssemblyexceptionfeatureadaptor adaptor,
    ajuint identifier,
    EnsPFeature feature,
    EnsPSlice slice,
    AjEnum type);

EnsPAssemblyexceptionfeature ensAssemblyexceptionfeatureNewObj(
    const EnsPAssemblyexceptionfeature object);

EnsPAssemblyexceptionfeature ensAssemblyexceptionfeatureNewRef(
    EnsPAssemblyexceptionfeature aef);

void ensAssemblyexceptionfeatureDel(EnsPAssemblyexceptionfeature* Paef);

EnsPAssemblyexceptionfeatureadaptor ensAssemblyexceptionfeatureGetAdaptor(
    const EnsPAssemblyexceptionfeature aef);

ajuint ensAssemblyexceptionfeatureGetIdentifier(
    const EnsPAssemblyexceptionfeature aef);

EnsPFeature ensAssemblyexceptionfeatureGetFeature(
    const EnsPAssemblyexceptionfeature aef);

EnsPSlice ensAssemblyexceptionfeatureGetAlternateSlice(
    const EnsPAssemblyexceptionfeature aef);

AjEnum ensAssemblyexceptionfeatureGetType(
    const EnsPAssemblyexceptionfeature aef);

AjBool ensAssemblyexceptionfeatureSetAdaptor(
    EnsPAssemblyexceptionfeature aef,
    EnsPAssemblyexceptionfeatureadaptor adaptor);

AjBool ensAssemblyexceptionfeatureSetIdentifier(
    EnsPAssemblyexceptionfeature aef,
    ajuint identifier);

AjBool ensAssemblyexceptionfeatureSetFeature(EnsPAssemblyexceptionfeature aef,
                                             EnsPFeature feature);

AjBool ensAssemblyexceptionfeatureSetAlternateSlice(
    EnsPAssemblyexceptionfeature aef,
    EnsPSlice altslice);

AjBool ensAssemblyexceptionfeatureSetType(EnsPAssemblyexceptionfeature aef,
                                          AjEnum type);

AjBool ensAssemblyexceptionfeatureFetchDisplayIdentifier(
    const EnsPAssemblyexceptionfeature aef,
    AjPStr *Pidentifier);

AjBool ensAssemblyexceptionfeatureTrace(const EnsPAssemblyexceptionfeature aef,
                                        ajuint level);

/* Ensembl Assembly Exception Feature Adaptor */

EnsPAssemblyexceptionfeatureadaptor ensAssemblyexceptionfeatureadaptorNew(
    EnsPDatabaseadaptor dba);

void ensAssemblyexceptionfeatureadaptorDel(
    EnsPAssemblyexceptionfeatureadaptor* Padaptor);

AjBool ensAssemblyexceptionfeatureadaptorFetchAll(
    EnsPAssemblyexceptionfeatureadaptor adaptor,
    AjPList aefs);

AjBool ensAssemblyexceptionfeatureadaptorFetchByIdentifier(
    EnsPAssemblyexceptionfeatureadaptor adaptor,
    ajuint identifier,
    EnsPAssemblyexceptionfeature* Paef);

AjBool ensAssemblyexceptionfeatureadaptorFetchAllBySlice(
    EnsPAssemblyexceptionfeatureadaptor adaptor,
    EnsPSlice slice,
    AjPList aefs);

/*
** End of prototype definitions
*/




#endif

#ifdef __cplusplus
}
#endif

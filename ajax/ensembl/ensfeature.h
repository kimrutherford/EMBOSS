
#ifndef ENSFEATURE_H
#define ENSFEATURE_H

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ensanalysis.h"
#include "ensassemblyexception.h"
#include "ensdata.h"
#include "ensslice.h"

AJ_BEGIN_DECLS




/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */




/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */

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
** @attr Extradata [AjPStr] Extra data
** @attr SourceSpecies [AjPStr] Source species name
** @attr TargetSpecies [AjPStr] Target species name
** @attr Groupidentifier [ajuint] Group identifier
** @attr Levelidentifier [ajuint] Level identifier
** @attr Evalue [double] e- or p-value
** @attr Score [double] Score
** @attr SourceCoverage [float] Source coverage in percent
** @attr TargetCoverage [float] Target coverage in percent
** @attr Identity [float] Sequence identity in percent
** @attr Use [ajuint] Use counter
** @@
******************************************************************************/

typedef struct EnsSFeaturepair
{
    EnsPFeature SourceFeature;
    EnsPFeature TargetFeature;
    EnsPExternaldatabase Externaldatabase;
    AjPStr Extradata;
    AjPStr SourceSpecies;
    AjPStr TargetSpecies;
    ajuint Groupidentifier;
    ajuint Levelidentifier;
    double Evalue;
    double Score;
    float SourceCoverage;
    float TargetCoverage;
    float Identity;
    ajuint Use;
} EnsOFeaturepair;

#define EnsPFeaturepair EnsOFeaturepair*




/* @data EnsPAssemblyexceptionfeatureadaptor **********************************
**
** Ensembl Assembly Exception Feature Adaptor.
**
** @alias EnsSAssemblyexceptionfeatureadaptor
** @alias EnsOAssemblyexceptionfeatureadaptor
**
** @attr Adaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @attr Cache [AjPList]
** AJAX List of Ensembl Assembly Exception Feature objects
** @attr CacheByIdentifier [AjPTable]
** AJAX Table of Ensembl Assembly Exception Feature objects
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
** @attr ExceptionSlice [EnsPSlice] Exception Ensembl Slice
** @attr Type [EnsEAssemblyexceptionType] Ensembl Assembly Exception type
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSAssemblyexceptionfeature
{
    ajuint Use;
    ajuint Identifier;
    EnsPAssemblyexceptionfeatureadaptor Adaptor;
    EnsPFeature Feature;
    EnsPSlice ExceptionSlice;
    EnsEAssemblyexceptionType Type;
    ajuint Padding;
} EnsOAssemblyexceptionfeature;

#define EnsPAssemblyexceptionfeature EnsOAssemblyexceptionfeature*




/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */

/*
** Prototype definitions
*/

/* Ensembl Feature */

EnsPFeature ensFeatureNewCpy(const EnsPFeature feature);

EnsPFeature ensFeatureNewIniN(EnsPAnalysis analysis,
                              AjPStr seqname,
                              ajint start,
                              ajint end,
                              ajint strand);

EnsPFeature ensFeatureNewIniS(EnsPAnalysis analysis,
                              EnsPSlice slice,
                              ajint start,
                              ajint end,
                              ajint strand);

EnsPFeature ensFeatureNewRef(EnsPFeature feature);

void ensFeatureDel(EnsPFeature* Pfeature);

EnsPAnalysis ensFeatureGetAnalysis(const EnsPFeature feature);

ajint ensFeatureGetEnd(const EnsPFeature feature);

AjPStr ensFeatureGetSequencename(const EnsPFeature feature);

EnsPSlice ensFeatureGetSlice(const EnsPFeature feature);

ajint ensFeatureGetStart(const EnsPFeature feature);

ajint ensFeatureGetStrand(const EnsPFeature feature);

AjBool ensFeatureSetAnalysis(EnsPFeature feature, EnsPAnalysis analysis);

AjBool ensFeatureSetEnd(EnsPFeature feature, ajint end);

AjBool ensFeatureSetSequencename(EnsPFeature, AjPStr seqname);

AjBool ensFeatureSetSlice(EnsPFeature feature, EnsPSlice slice);

AjBool ensFeatureSetStart(EnsPFeature feature, ajint start);

AjBool ensFeatureSetStrand(EnsPFeature feature, ajint strand);

AjBool ensFeatureTrace(const EnsPFeature feature, ajuint level);

ajuint ensFeatureCalculateLength(const EnsPFeature feature);

size_t ensFeatureCalculateMemsize(const EnsPFeature feature);

ajint ensFeatureGetSeqregionEnd(const EnsPFeature feature);

ajuint ensFeatureGetSeqregionLength(const EnsPFeature feature);

const AjPStr ensFeatureGetSeqregionName(const EnsPFeature feature);

const EnsPSeqregion ensFeatureGetSeqregionObject(const EnsPFeature feature);

ajint ensFeatureGetSeqregionStart(const EnsPFeature feature);

ajint ensFeatureGetSeqregionStrand(const EnsPFeature feature);

AjBool ensFeatureFetchAllAlternativelocations(EnsPFeature feature,
                                              AjBool all,
                                              AjPList features);

AjBool ensFeatureFetchSequencename(const EnsPFeature feature, AjPStr* Pname);

AjBool ensFeatureMove(EnsPFeature feature,
                      ajint start,
                      ajint end,
                      ajint strand);

AjBool ensFeatureProject(const EnsPFeature feature,
                         const AjPStr csname,
                         const AjPStr csversion,
                         AjPList pss);

AjBool ensFeatureProjectslice(const EnsPFeature feature,
                              EnsPSlice slice,
                              AjPList pss);

EnsPFeature ensFeatureTransform(EnsPFeature feature,
                                const AjPStr csname,
                                const AjPStr csversion,
                                const EnsPSlice slice);

EnsPFeature ensFeatureTransfer(EnsPFeature feature, EnsPSlice slice);

AjBool ensFeatureMatch(const EnsPFeature feature1,
                       const EnsPFeature feature2);

AjBool ensFeatureOverlap(const EnsPFeature feature1,
                         const EnsPFeature feature2);

AjBool ensFeatureSimilarity(const EnsPFeature feature1,
                            const EnsPFeature feature2);

int ensFeatureCompareStartAscending(const EnsPFeature feature1,
                                    const EnsPFeature feature2);

int ensFeatureCompareStartDescending(const EnsPFeature feature1,
                                     const EnsPFeature feature2);

AjBool ensListFeatureSortStartAscending(AjPList features);

AjBool ensListFeatureSortStartDescending(AjPList features);

/* Ensembl Feature Adaptor */

EnsPFeatureadaptor ensFeatureadaptorNew(
    EnsPDatabaseadaptor dba,
    const char* const* Ptables,
    const char* const* Pcolumns,
    EnsPBaseadaptorLeftjoin leftjoin,
    const char* condition,
    const char* final,
    AjBool Fstatement(EnsPDatabaseadaptor dba,
                      const AjPStr statement,
                      EnsPAssemblymapper am,
                      EnsPSlice slice,
                      AjPList objects),
    void* Fread(const void* key),
    void* Freference(void* value),
    AjBool Fwrite(const void* value),
    void Fdelete(void** value),
    size_t Fsize(const void* value),
    EnsPFeature Fgetfeature(const void* object),
    const char* label);

void ensFeatureadaptorDel(EnsPFeatureadaptor* Pfa);

EnsPBaseadaptor ensFeatureadaptorGetBaseadaptor(
    const EnsPFeatureadaptor fa);

EnsPCache ensFeatureadaptorGetCache(
    const EnsPFeatureadaptor fa);

EnsPDatabaseadaptor ensFeatureadaptorGetDatabaseadaptor(
    const EnsPFeatureadaptor fa);

ajint ensFeatureadaptorGetMaximumlength(
    const EnsPFeatureadaptor fa);

AjBool ensFeatureadaptorSetColumns(EnsPFeatureadaptor fa,
                                   const char* const* Pcolumns);

AjBool ensFeatureadaptorSetDefaultcondition(EnsPFeatureadaptor fa,
                                            const char* condition);

AjBool ensFeatureadaptorSetFinalcondition(EnsPFeatureadaptor fa,
                                          const char* final);

AjBool ensFeatureadaptorSetMaximumlength(EnsPFeatureadaptor fa,
                                         ajint length);

AjBool ensFeatureadaptorSetTables(EnsPFeatureadaptor fa,
                                  const char* const* Ptables);

AjBool ensFeatureadaptorEscapeC(EnsPFeatureadaptor fa,
                                char** Ptxt,
                                const AjPStr str);

AjBool ensFeatureadaptorEscapeS(EnsPFeatureadaptor fa,
                                AjPStr* Pstr,
                                const AjPStr str);

AjBool ensFeatureadaptorConstraintAppendAnalysisname(
    const EnsPFeatureadaptor fa,
    AjPStr* Pconstraint,
    const AjPStr anname);

AjBool ensFeatureadaptorFetchAllbyAnalysisname(EnsPFeatureadaptor fa,
                                               const AjPStr anname,
                                               AjPList objects);

AjBool ensFeatureadaptorFetchAllbySlice(EnsPFeatureadaptor fa,
                                        EnsPSlice slice,
                                        const AjPStr constraint,
                                        const AjPStr anname,
                                        AjPList objects);

AjBool ensFeatureadaptorFetchAllbySlicescore(EnsPFeatureadaptor fa,
                                             EnsPSlice slice,
                                             double score,
                                             const AjPStr anname,
                                             AjPList objects);

/* Ensembl Feature Pair */

EnsPFeaturepair ensFeaturepairNewCpy(const EnsPFeaturepair fp);

EnsPFeaturepair ensFeaturepairNewIni(EnsPFeature srcfeature,
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

EnsPFeaturepair ensFeaturepairNewRef(EnsPFeaturepair fp);

void ensFeaturepairDel(EnsPFeaturepair* Pfp);

double ensFeaturepairGetEvalue(
    const EnsPFeaturepair fp);

EnsPExternaldatabase ensFeaturepairGetExternaldatabase(
    const EnsPFeaturepair fp);

AjPStr ensFeaturepairGetExtradata(
    const EnsPFeaturepair fp);

ajuint ensFeaturepairGetGroupidentifier(
    const EnsPFeaturepair fp);

float ensFeaturepairGetIdentity(
    const EnsPFeaturepair fp);

ajuint ensFeaturepairGetLevelidentifier(
    const EnsPFeaturepair fp);

double ensFeaturepairGetScore(
    const EnsPFeaturepair fp);

float ensFeaturepairGetSourceCoverage(
    const EnsPFeaturepair fp);

EnsPFeature ensFeaturepairGetSourceFeature(
    const EnsPFeaturepair fp);

AjPStr ensFeaturepairGetSourceSpecies(
    const EnsPFeaturepair fp);

float ensFeaturepairGetTargetCoverage(
    const EnsPFeaturepair fp);

EnsPFeature ensFeaturepairGetTargetFeature(
    const EnsPFeaturepair fp);

AjPStr ensFeaturepairGetTargetSpecies(
    const EnsPFeaturepair fp);

AjBool ensFeaturepairSetEvalue(EnsPFeaturepair fp,
                               double evalue);

AjBool ensFeaturepairSetExternaldatabase(EnsPFeaturepair fp,
                                         EnsPExternaldatabase edb);

AjBool ensFeaturepairSetExtradata(EnsPFeaturepair fp,
                                  AjPStr extra);

AjBool ensFeaturepairSetGroupidentifier(EnsPFeaturepair fp,
                                        ajuint groupid);

AjBool ensFeaturepairSetLevelidentifier(EnsPFeaturepair fp,
                                        ajuint levelid);

AjBool ensFeaturepairSetScore(EnsPFeaturepair fp,
                              double score);

AjBool ensFeaturepairSetSourceCoverage(EnsPFeaturepair fp,
                                       float coverage);

AjBool ensFeaturepairSetSourceFeature(EnsPFeaturepair fp,
                                      EnsPFeature feature);

AjBool ensFeaturepairSetSourceSpecies(EnsPFeaturepair fp,
                                      AjPStr species);

AjBool ensFeaturepairSetTargetCoverage(EnsPFeaturepair fp,
                                       float coverage);

AjBool ensFeaturepairSetTargetFeature(EnsPFeaturepair fp,
                                      EnsPFeature feature);

AjBool ensFeaturepairSetTargetSpecies(EnsPFeaturepair fp,
                                      AjPStr species);

AjBool ensFeaturepairTrace(const EnsPFeaturepair fp, ajuint level);

AjBool ensFeaturepairSetIdentity(EnsPFeaturepair fp, float identity);

size_t ensFeaturepairCalculateMemsize(const EnsPFeaturepair fp);

EnsPFeaturepair ensFeaturepairTransfer(EnsPFeaturepair fp,
                                       EnsPSlice slice);

EnsPFeaturepair ensFeaturepairTransform(const EnsPFeaturepair fp,
                                        const AjPStr csname,
                                        const AjPStr csversion);

int ensFeaturepairCompareSourceStartAscending(const EnsPFeaturepair fp1,
                                              const EnsPFeaturepair fp2);

int ensFeaturepairCompareSourceStartDescending(const EnsPFeaturepair fp1,
                                               const EnsPFeaturepair fp2);

AjBool ensListFeaturepairSortSourceStartAscending(AjPList fps);

AjBool ensListFeaturepairSortSourceStartDescending(AjPList fps);

/* Ensembl Assembly Exception Feature */

EnsPAssemblyexceptionfeature ensAssemblyexceptionfeatureNewCpy(
    const EnsPAssemblyexceptionfeature aef);

EnsPAssemblyexceptionfeature ensAssemblyexceptionfeatureNewIni(
    EnsPAssemblyexceptionfeatureadaptor aefa,
    ajuint identifier,
    EnsPFeature feature,
    EnsPSlice slice,
    EnsEAssemblyexceptionType type);

EnsPAssemblyexceptionfeature ensAssemblyexceptionfeatureNewRef(
    EnsPAssemblyexceptionfeature aef);

void ensAssemblyexceptionfeatureDel(EnsPAssemblyexceptionfeature* Paef);

EnsPAssemblyexceptionfeatureadaptor ensAssemblyexceptionfeatureGetAdaptor(
    const EnsPAssemblyexceptionfeature aef);

EnsPSlice ensAssemblyexceptionfeatureGetExceptionSlice(
    const EnsPAssemblyexceptionfeature aef);

EnsPFeature ensAssemblyexceptionfeatureGetFeature(
    const EnsPAssemblyexceptionfeature aef);

ajuint ensAssemblyexceptionfeatureGetIdentifier(
    const EnsPAssemblyexceptionfeature aef);

EnsEAssemblyexceptionType ensAssemblyexceptionfeatureGetType(
    const EnsPAssemblyexceptionfeature aef);

AjBool ensAssemblyexceptionfeatureSetAdaptor(
    EnsPAssemblyexceptionfeature aef,
    EnsPAssemblyexceptionfeatureadaptor aefa);

AjBool ensAssemblyexceptionfeatureSetExceptionSlice(
    EnsPAssemblyexceptionfeature aef,
    EnsPSlice slice);

AjBool ensAssemblyexceptionfeatureSetFeature(EnsPAssemblyexceptionfeature aef,
                                             EnsPFeature feature);

AjBool ensAssemblyexceptionfeatureSetIdentifier(
    EnsPAssemblyexceptionfeature aef,
    ajuint identifier);

AjBool ensAssemblyexceptionfeatureSetType(EnsPAssemblyexceptionfeature aef,
                                          EnsEAssemblyexceptionType type);

size_t ensAssemblyexceptionfeatureCalculateMemsize(
    const EnsPAssemblyexceptionfeature aef);

AjBool ensAssemblyexceptionfeatureFetchDisplayidentifier(
    const EnsPAssemblyexceptionfeature aef,
    AjPStr* Pidentifier);

AjBool ensAssemblyexceptionfeatureTrace(const EnsPAssemblyexceptionfeature aef,
                                        ajuint level);

AjBool ensTableAssemblyexceptionfeatureClear(AjPTable table);

AjBool ensTableAssemblyexceptionfeatureDelete(AjPTable* Ptable);

/* Ensembl Assembly Exception Feature Adaptor */

EnsPAssemblyexceptionfeatureadaptor ensRegistryGetAssemblyexceptionfeatureadaptor(
    EnsPDatabaseadaptor dba);

EnsPAssemblyexceptionfeatureadaptor ensAssemblyexceptionfeatureadaptorNew(
    EnsPDatabaseadaptor dba);

void ensAssemblyexceptionfeatureadaptorDel(
    EnsPAssemblyexceptionfeatureadaptor* Paefa);

AjBool ensAssemblyexceptionfeatureadaptorFetchAll(
    EnsPAssemblyexceptionfeatureadaptor aefa,
    AjPList aefs);

AjBool ensAssemblyexceptionfeatureadaptorFetchAllbySlice(
    EnsPAssemblyexceptionfeatureadaptor aefa,
    EnsPSlice slice,
    AjPList aefs);

AjBool ensAssemblyexceptionfeatureadaptorFetchByIdentifier(
    EnsPAssemblyexceptionfeatureadaptor aefa,
    ajuint identifier,
    EnsPAssemblyexceptionfeature* Paef);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSFEATURE_H */

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ensqc_h
#define ensqc_h

#include "ensanalysis.h"
#include "ensqcalignment.h"
#include "ensqcsequence.h"




/******************************************************************************
**
** Ensembl QC DAS Feature Adaptor
**
******************************************************************************/

#define EnsPQcdasfeatureadaptor EnsPBaseadaptor




/******************************************************************************
**
** Ensembl QC DAS Feature Category enumeration
**
******************************************************************************/

enum EnsEQcdasfeatureCategory
{
    ensEQcdasfeatureCategoryNULL,
    ensEQcdasfeatureCategoryUnknown,
    ensEQcdasfeatureCategoryTranscriptPerfect,
    ensEQcdasfeatureCategoryTranscriptTolerance,
    ensEQcdasfeatureCategoryTranscriptPartial,
    ensEQcdasfeatureCategoryTranscriptMissing,
    ensEQcdasfeatureCategoryTranscript,
    ensEQcdasfeatureCategoryTranslationPerfect,
    ensEQcdasfeatureCategoryTranslationTolerance,
    ensEQcdasfeatureCategoryTranslationPartial,
    ensEQcdasfeatureCategoryTranslationMissing,
    ensEQcdasfeatureCategoryTranslation
};




/******************************************************************************
**
** Ensembl QC DAS Feature Type enumeration
**
******************************************************************************/

enum EnsEQcdasfeatureType
{
    ensEQcdasfeatureTypeNULL,
    ensEQcdasfeatureTypeUnknown,
    ensEQcdasfeatureTypeExonPerfect,
    ensEQcdasfeatureTypeExonPartial,
    ensEQcdasfeatureTypeExonMissing,
    ensEQcdasfeatureTypeExonFrameshift,
    ensEQcdasfeatureTypeExonGap,
    ensEQcdasfeatureTypeExon
};




/* @data EnsPQcdasfeature *****************************************************
**
** Ensembl QC DAS Feature
**
** @alias EnsSQcdasfeature
** @alias EnsOQcdasfeature
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPQcdasfeatureadaptor] Ensembl QC DAS Feature Adaptor
** @cc Bio::EnsEMBL::QC::DASFeature
** @cc 'das_feature' SQL table
** @attr Qcalignment [EnsPQcalignment] Ensembl QC Alignment
** @attr Analysis [EnsPAnalysis] Ensembl Analysis
** @attr SegmentSequence [EnsPQcsequence] Segment Ensembl QC Sequence
** @attr FeatureSequence [EnsPQcsequence] Feature Ensembl QC Sequence
** @attr SegmentStart [ajuint] Segment start
** @attr SegmentEnd [ajuint] Segment end
** @attr SegmentStrand [ajint] Segment strand
** @attr FeatureStart [ajuint] Feature start
** @attr FeatureEnd [ajuint] Feature end
** @attr Phase [ajint] Phase
** @attr Category [AjEnum] Category
** @attr Type [AjEnum] Type
** @@
******************************************************************************/

typedef struct EnsSQcdasfeature
{
    ajuint Use;
    ajuint Identifier;
    EnsPQcdasfeatureadaptor Adaptor;
    EnsPQcalignment Qcalignment;
    EnsPAnalysis Analysis;
    EnsPQcsequence SegmentSequence;
    EnsPQcsequence FeatureSequence;
    ajuint SegmentStart;
    ajuint SegmentEnd;
    ajint SegmentStrand;
    ajuint FeatureStart;
    ajuint FeatureEnd;
    ajint Phase;
    AjEnum Category;
    AjEnum Type;
} EnsOQcdasfeature;

#define EnsPQcdasfeature EnsOQcdasfeature*




/******************************************************************************
**
** Ensembl QC Variation Adaptor
**
******************************************************************************/

#define EnsPQcvariationadaptor EnsPBaseadaptor




/******************************************************************************
**
** Ensembl QC Variation Class enumeration
**
******************************************************************************/

enum EnsEQcvariationClass
{
    ensEQcvariationClassNULL,
    ensEQcvariationClassNone,
    ensEQcvariationClassSimple,
    ensEQcvariationClassSplice,
    ensEQcvariationClassExon
};




/******************************************************************************
**
** Ensembl QC Variation Type enumeration
**
******************************************************************************/

enum EnsEQcvariationType
{
    ensEQcvariationTypeNULL,
    ensEQcvariationTypeNone,
    ensEQcvariationTypeSingle,
    ensEQcvariationTypeMulti
};




/******************************************************************************
**
** Ensembl QC Variation State enumeration
**
******************************************************************************/

enum EnsEQcvariationState
{
    ensEQcvariationStateNULL,
    ensEQcvariationStateNone,
    ensEQcvariationStateMatch,
    ensEQcvariationStateGap,
    ensEQcvariationStateFrameshift,
    ensEQcvariationState5ss,
    ensEQcvariationState3ss,
    EnsEQcvariationStateSplit
};




/* @data EnsPQcvariation ******************************************************
**
** Ensembl QC Variation
**
** @alias EnsSQcvariation
** @alias EnsOQcvariation
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPQcvariationadaptor] Ensembl QC Variation Adaptor
** @cc Bio::EnsEMBL::QC::Variation
** @cc 'variation' SQL table
** @attr Analysis [EnsPAnalysis] Ensembl Analysis
** @attr Qcalignment [EnsPQcalignment] Ensembl QC Alignment
** @attr QuerySequence [EnsPQcsequence] Query Ensembl QC Sequence
** @attr QueryStart [ajuint] Query start
** @attr QueryEnd [ajuint] Query end
** @attr QueryString [AjPStr] Query string
** @attr TargetSequence [EnsPQcsequence] Target Ensembl QC Sequence
** @attr TargetStart [ajuint] Target start
** @attr TargetEnd [ajuint] Target end
** @attr TargetString [AjPStr] Target string
** @attr Class [AjEnum] Class
** @attr Type [AjEnum] Type
** @attr State [AjEnum] State
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSQcvariation
{
    ajuint Use;
    ajuint Identifier;
    EnsPQcvariationadaptor Adaptor;
    EnsPAnalysis Analysis;
    EnsPQcalignment Qcalignment;
    EnsPQcsequence QuerySequence;
    ajuint QueryStart;
    ajuint QueryEnd;
    AjPStr QueryString;
    EnsPQcsequence TargetSequence;
    ajuint TargetStart;
    ajuint TargetEnd;
    AjPStr TargetString;
    AjEnum Class;
    AjEnum Type;
    AjEnum State;
    char Padding[4];
} EnsOQcvariation;

#define EnsPQcvariation EnsOQcvariation*




/******************************************************************************
**
** Ensembl QC Submission Adaptor
**
******************************************************************************/

#define EnsPQcsubmissionadaptor EnsPBaseadaptor




/* @data EnsPQcsubmission *****************************************************
**
** Ensembl QC Submission
**
** @alias EnsSQcsubmission
** @alias EnsOQcsubmission
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPQcsubmissionadaptor] Ensembl QC Submission Adaptor
** @cc Bio::EnsEMBL::QC::Submission
** @cc 'submission' SQL table
** @attr Analysis [EnsPAnalysis] Ensembl Analysis
** @attr QuerySequence [EnsPQcsequence] Query Ensembl QC Sequence
** @attr TargetSequence [EnsPQcsequence] Target Ensembl QC Sequence
** @attr QueryStart [ajuint] Query start
** @attr QueryEnd [ajuint] Query end
** @attr QueryStrand [ajint] Query strand
** @attr TargetStart [ajuint] Target start
** @attr TargetEnd [ajuint] Target end
** @attr TargetStrand [ajint] Target strand
** @attr AnalysisJobIdentifier [ajuint] Ensembl Hive Analysis Job identifier
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSQcsubmission
{
    ajuint Use;
    ajuint Identifier;
    EnsPQcsubmissionadaptor Adaptor;
    EnsPAnalysis Analysis;
    EnsPQcsequence QuerySequence;
    EnsPQcsequence TargetSequence;
    ajuint QueryStart;
    ajuint QueryEnd;
    ajint QueryStrand;
    ajuint TargetStart;
    ajuint TargetEnd;
    ajint TargetStrand;
    ajuint AnalysisJobIdentifier;
    ajuint Padding;
} EnsOQcsubmission;

#define EnsPQcsubmission EnsOQcsubmission*




/*
** Prototype definitions
*/

/* Ensembl Quality Check DAS Feature */

EnsPQcdasfeature ensQcdasfeatureNew(EnsPQcdasfeatureadaptor adaptor,
                                    ajuint identifier,
                                    EnsPQcalignment qca,
                                    EnsPAnalysis analysis,
                                    EnsPQcsequence segment,
                                    ajuint sstart,
                                    ajuint send,
                                    ajint sstrand,
                                    EnsPQcsequence feature,
                                    ajuint fstart,
                                    ajuint fend,
                                    ajint phase,
                                    AjEnum category,
                                    AjEnum type);

EnsPQcdasfeature ensQcdasfeatureNewObj(const EnsPQcdasfeature object);

EnsPQcdasfeature ensQcdasfeatureNewRef(EnsPQcdasfeature qcdasf);

void ensQcdasfeatureDel(EnsPQcdasfeature* Pqcdasf);

EnsPQcdasfeatureadaptor ensQcdasfeatureGetAdaptor(
    const EnsPQcdasfeature qcdasf);

ajuint ensQcdasfeatureGetIdentifier(const EnsPQcdasfeature qcdasf);

EnsPQcalignment ensQcdasfeatureGetQcalignment(const EnsPQcdasfeature qcdasf);

EnsPAnalysis ensQcdasfeatureGetAnalysis(const EnsPQcdasfeature qcdasf);

EnsPQcsequence ensQcdasfeatureGetSegmentSequence(const EnsPQcdasfeature qcdasf);

EnsPQcsequence ensQcdasfeatureGetFeatureSequence(const EnsPQcdasfeature qcdasf);

ajuint ensQcdasfeatureGetSegmentStart(const EnsPQcdasfeature qcdasf);

ajuint ensQcdasfeatureGetSegmentEnd(const EnsPQcdasfeature qcdasf);

ajint ensQcdasfeatureGetSegmentStrand(const EnsPQcdasfeature qcdasf);

ajuint ensQcdasfeatureGetFeatureStart(const EnsPQcdasfeature qcdasf);

ajuint ensQcdasfeatureGetFeatureEnd(const EnsPQcdasfeature qcdasf);

ajint ensQcdasfeatureGetPhase(const EnsPQcdasfeature qcdasf);

AjEnum ensQcdasfeatureGetCategory(const EnsPQcdasfeature qcdasf);

AjEnum ensQcdasfeatureGetType(const EnsPQcdasfeature qcdasf);

AjBool ensQcdasfeatureSetAdaptor(EnsPQcdasfeature qcdasf,
                                 EnsPQcdasfeatureadaptor adaptor);

AjBool ensQcdasfeatureSetIdentifier(EnsPQcdasfeature qcdasf, ajuint identifier);

AjBool ensQcdasfeatureSetQcalignment(EnsPQcdasfeature qcdasf,
                                     EnsPQcalignment qca);

AjBool ensQcdasfeatureSetAnalysis(EnsPQcdasfeature qcdasf,
                                  EnsPAnalysis analysis);

AjBool ensQcdasfeatureSetSegmentSequence(EnsPQcdasfeature qcdasf,
                                         EnsPQcsequence qcs);

AjBool ensQcdasfeatureSetFeatureSequence(EnsPQcdasfeature qcdasf,
                                         EnsPQcsequence qcs);

AjBool ensQcdasfeatureSetSegmentStart(EnsPQcdasfeature qcdasf, ajuint start);

AjBool ensQcdasfeatureSetSegmentEnd(EnsPQcdasfeature qcdasf, ajuint end);

AjBool ensQcdasfeatureSetSegmentStrand(EnsPQcdasfeature qcdasf, ajint strand);

AjBool ensQcdasfeatureSetFeatureStart(EnsPQcdasfeature qcdasf, ajuint start);

AjBool ensQcdasfeatureSetFeatureEnd(EnsPQcdasfeature qcdasf, ajuint end);

AjBool ensQcdasfeatureSetPhase(EnsPQcdasfeature qcdasf, ajint phase);

AjBool ensQcdasfeatureSetCategory(EnsPQcdasfeature qcdasf, AjEnum category);

AjBool ensQcdasfeatureSetType(EnsPQcdasfeature qcdasf, AjEnum type);

ajuint ensQcdasfeatureGetMemSize(const EnsPQcdasfeature qcdasf);

AjBool ensQcdasfeatureTrace(const EnsPQcdasfeature qcdasf, ajuint level);

AjEnum ensQcdasfeatureCategoryFromStr(const AjPStr category);

AjEnum ensQcdasfeatureTypeFromStr(const AjPStr type);

const char *ensQcdasfeatureCategoryToChar(const AjEnum category);

const char *ensQcdasfeatureTypeToChar(const AjEnum type);

/* Ensembl Quality Check DAS Feature Adaptor */

EnsPQcdasfeatureadaptor ensQcdasfeatureadaptorNew(EnsPDatabaseadaptor dba);

void ensQcdasfeatureadaptorDel(EnsPQcdasfeatureadaptor *Pqcdasfa);

AjBool ensQcdasfeatureadaptorFetchByIdentifier(EnsPQcdasfeatureadaptor adaptor,
                                               ajuint identifier,
                                               EnsPQcdasfeature *Pqcdasf);

AjBool ensQcdasfeatureadaptorFetchAllByQcalignment(
    EnsPQcdasfeatureadaptor qcdasfa,
    const EnsPQcalignment qca,
    AjPList qcdasfs);

AjBool ensQcdasfeatureadaptorFetchAllByFeature(EnsPQcdasfeatureadaptor qcdasfa,
                                               const EnsPAnalysis analysis,
                                               const EnsPQcsequence feature,
                                               AjPList qcdasfs);

AjBool ensQcdasfeatureadaptorFetchAllBySegment(EnsPQcdasfeatureadaptor qcdasfa,
                                               const EnsPAnalysis analysis,
                                               const EnsPQcsequence segment,
                                               AjPList qcdasfs);

AjBool ensQcdasfeatureadaptorFetchAllByAFS(EnsPQcdasfeatureadaptor qcdasfa,
                                           const EnsPAnalysis analysis,
                                           const EnsPQcsequence feature,
                                           const EnsPQcsequence segment,
                                           AjPList qcdasfs);

AjBool ensQcdasfeatureadaptorFetchAllByRegion(EnsPQcdasfeatureadaptor qcdasfa,
                                              const EnsPAnalysis analysis,
                                              const EnsPQcsequence segment,
                                              ajuint start,
                                              ajuint end,
                                              AjPList qcdasfs);

AjBool ensQcdasfeatureadaptorStore(EnsPQcdasfeatureadaptor qcdasfa,
                                   EnsPQcdasfeature qcdasf);

AjBool ensQcdasfeatureadaptorUpdate(EnsPQcdasfeatureadaptor qcdasfa,
                                    const EnsPQcdasfeature qcdasf);

AjBool ensQcdasfeatureadaptorDelete(EnsPQcdasfeatureadaptor qcdasfa,
                                    const EnsPQcdasfeature qcdasf);

/* Ensembl Quality Check Variation */

EnsPQcvariation ensQcvariationNew(EnsPQcvariationadaptor adaptor,
                                  ajuint identifier,
                                  EnsPQcalignment qca,
                                  EnsPAnalysis analysis,
                                  EnsPQcsequence qsequence,
                                  ajuint qstart,
                                  ajuint qend,
                                  AjPStr qstring,
                                  EnsPQcsequence tsequence,
                                  ajuint tstart,
                                  ajuint tend,
                                  AjPStr tstring,
                                  AjEnum class,
                                  AjEnum type,
                                  AjEnum state);

EnsPQcvariation ensQcvariationNewObj(const EnsPQcvariation object);

EnsPQcvariation ensQcvariationNewRef(EnsPQcvariation qcv);

void ensQcvariationDel(EnsPQcvariation* Pqcv);

EnsPQcvariationadaptor ensQcvariationGetAdaptor(const EnsPQcvariation qcv);

ajuint ensQcvariationGetIdentifier(const EnsPQcvariation qcv);

EnsPQcalignment ensQcvariationGetQcalignment(const EnsPQcvariation qcv);

EnsPAnalysis ensQcvariationGetAnalysis(const EnsPQcvariation qcv);

EnsPQcsequence ensQcvariationGetQuerySequence(const EnsPQcvariation qcv);

ajuint ensQcvariationGetQueryStart(const EnsPQcvariation qcv);

ajuint ensQcvariationGetQueryEnd(const EnsPQcvariation qcv);

AjPStr ensQcvariationGetQueryString(const EnsPQcvariation qcv);

EnsPQcsequence ensQcvariationGetTargetSequence(const EnsPQcvariation qcv);

ajuint ensQcvariationGetTargetStart(const EnsPQcvariation qcv);

ajuint ensQcvariationGetTargetEnd(const EnsPQcvariation qcv);

AjPStr ensQcvariationGetTargetString(const EnsPQcvariation qcv);

AjEnum ensQcvariationGetClass(const EnsPQcvariation qcv);

AjEnum ensQcvariationGetType(const EnsPQcvariation qcv);

AjEnum ensQcvariationGetState(const EnsPQcvariation qcv);

AjBool ensQcvariationSetAdaptor(EnsPQcvariation qcv,
                                EnsPQcvariationadaptor qcva);

AjBool ensQcvariationSetIdentifier(EnsPQcvariation qcv, ajuint identifier);

AjBool ensQcvariationSetQcalignment(EnsPQcvariation qcv, EnsPQcalignment qca);

AjBool ensQcvariationSetAnalysis(EnsPQcvariation qcv, EnsPAnalysis analysis);

AjBool ensQcvariationSetQuerySequence(EnsPQcvariation qcv,
                                      EnsPQcsequence qsequence);

AjBool ensQcvariationSetQueryStart(EnsPQcvariation qcv, ajuint qstart);

AjBool ensQcvariationSetQueryEnd(EnsPQcvariation qcv, ajuint qend);

AjBool ensQcvariationSetQueryString(EnsPQcvariation qcv, AjPStr qstring);

AjBool ensQcvariationSetTargetSequence(EnsPQcvariation qcv,
                                       EnsPQcsequence tsequence);

AjBool ensQcvariationSetTargetStart(EnsPQcvariation qcv, ajuint tstart);

AjBool ensQcvariationSetTargetEnd(EnsPQcvariation qcv, ajuint tend);

AjBool ensQcvariationSetTargetString(EnsPQcvariation qcv, AjPStr tstring);

AjBool ensQcvariationSetClass(EnsPQcvariation qcv, AjEnum class);

AjBool ensQcvariationSetType(EnsPQcvariation qcv, AjEnum type);

AjBool ensQcvariationSetState(EnsPQcvariation qcv, AjEnum state);

ajuint ensQcvariationGetMemSize(const EnsPQcvariation qcv);

AjBool ensQcvariationTrace(const EnsPQcvariation qcv, ajuint level);

AjEnum ensQcvariationClassFromStr(const AjPStr vclass);

AjEnum ensQcvariationTypeFromStr(const AjPStr type);

AjEnum ensQcvariationStateFromStr(const AjPStr state);

const char *ensQcvariationClassToChar(const AjEnum vclass);

const char *ensQcvariationTypeToChar(const AjEnum type);

const char *ensQcvariationStateToChar(const AjEnum state);

/* Ensembl Quality Check Variation Adaptor */

EnsPQcvariationadaptor ensQcvariationadaptorNew(EnsPDatabaseadaptor dba);

void ensQcvariationadaptorDel(EnsPQcvariationadaptor* Pqcva);

AjBool ensQcvariationadaptorFetchByIdentifier(EnsPQcvariationadaptor adaptor,
                                              ajuint identifier,
                                              EnsPQcvariation *Pqcvf);

AjBool ensQcvariationadaptorFetchAllByQcalignment(EnsPQcvariationadaptor qcva,
                                                  const EnsPQcalignment qca,
                                                  AjPList qcvs);

AjBool ensQcvariationadaptorFetchAllByAnalysisQueryTarget(
    EnsPQcvariationadaptor qcva,
    const EnsPAnalysis analysis,
    const EnsPQcdatabase qdb,
    const EnsPQcdatabase tdb,
    AjPList qcvs);

AjBool ensQcvariationadaptorFetchAllByQuery(EnsPQcvariationadaptor qcva,
                                            const EnsPAnalysis analysis,
                                            const EnsPQcdatabase qdb,
                                            AjPList qcvs);

AjBool ensQcvariationadaptorFetchAllByTarget(EnsPQcvariationadaptor qcva,
                                             const EnsPAnalysis analysis,
                                             const EnsPQcdatabase tdb,
                                             AjPList qcvs);

AjBool ensQcvariationadaptorStore(EnsPQcvariationadaptor qcva,
                                  EnsPQcvariation qcv);

AjBool ensQcvariationadaptorUpdate(EnsPQcvariationadaptor qcva,
                                   const EnsPQcvariation qcv);

AjBool ensQcvariationadaptorDelete(EnsPQcvariationadaptor qcva,
                                   const EnsPQcvariation qcv);

/* Ensembl Quality Check Submission */

EnsPQcsubmission ensQcsubmissionNew(EnsPQcsubmissionadaptor adaptor,
                                    ajuint identifier,
                                    EnsPAnalysis analysis,
                                    EnsPQcsequence qsequence,
                                    EnsPQcsequence tsequence,
                                    ajuint tstart,
                                    ajuint tend,
                                    ajint tstrand,
                                    ajuint analysisjobid);

EnsPQcsubmission ensQcsubmissionNewObj(const EnsPQcsubmission object);

EnsPQcsubmission ensQcsubmissionNewRef(EnsPQcsubmission qcs);

void ensQcsubmissionDel(EnsPQcsubmission* Pqcs);

EnsPQcsubmissionadaptor ensQcsubmissionGetAdaptor(const EnsPQcsubmission qcs);

ajuint ensQcsubmissionGetIdentifier(const EnsPQcsubmission qcs);

EnsPAnalysis ensQcsubmissionGetAnalysis(const EnsPQcsubmission qcs);

EnsPQcsequence ensQcsubmissionGetQuerySequence(const EnsPQcsubmission qcs);

ajuint ensQcsubmissionGetQueryStart(const EnsPQcsubmission qcs);

ajuint ensQcsubmissionGetQueryEnd(const EnsPQcsubmission qcs);

ajint ensQcsubmissionGetQueryStrand(const EnsPQcsubmission qcs);

EnsPQcsequence ensQcsubmissionGetTargetSequence(const EnsPQcsubmission qcs);

ajuint ensQcsubmissionGetTargetStart(const EnsPQcsubmission qcs);

ajuint ensQcsubmissionGetTargetEnd(const EnsPQcsubmission qcs);

ajint ensQcsubmissionGetTargetStrand(const EnsPQcsubmission qcs);

ajuint ensQcsubmissionGetAnalysisJobIdentifier(const EnsPQcsubmission qcs);

AjBool ensQcsubmissionSetAdaptor(EnsPQcsubmission qcs,
                                 EnsPQcsubmissionadaptor qcsa);

AjBool ensQcsubmissionSetIdentifier(EnsPQcsubmission qcs, ajuint identifier);

AjBool ensQcsubmissionSetAnalysis(EnsPQcsubmission qcs, EnsPAnalysis analysis);

AjBool ensQcsubmissionSetQuerySequence(EnsPQcsubmission qcs,
                                       EnsPQcsequence qcseq);

AjBool ensQcsubmissionSetQueryStart(EnsPQcsubmission qcs, ajuint qstart);

AjBool ensQcsubmissionSetQueryEnd(EnsPQcsubmission qcs, ajuint qend);

AjBool ensQcsubmissionSetQueryStrand(EnsPQcsubmission qcs, ajint qstrand);

AjBool ensQcsubmissionSetTargetSequence(EnsPQcsubmission qcs,
                                        EnsPQcsequence qcseq);

AjBool ensQcsubmissionSetTargetStart(EnsPQcsubmission qcs, ajuint tstart);

AjBool ensQcsubmissionSetTargetEnd(EnsPQcsubmission qcs, ajuint tend);

AjBool ensQcsubmissionSetTargetStrand(EnsPQcsubmission qcs, ajint tstrand);

AjBool ensQcsubmissionSetAnalysisJobIdentifier(EnsPQcsubmission qcs,
                                               ajuint jobid);

ajuint ensQcsubmissionGetMemSize(const EnsPQcsubmission qcs);

AjBool ensQcsubmissionTrace(const EnsPQcsubmission qcs, ajuint level);

/* Ensembl Quality Check Submission Adaptor */

EnsPQcsubmissionadaptor ensQcsubmissionadaptorNew(EnsPDatabaseadaptor dba);

void ensQcsubmissionadaptorDel(EnsPQcsubmissionadaptor* Pqcsa);

AjBool ensQcsubmissionadaptorFetchByIdentifier(EnsPQcsubmissionadaptor adaptor,
                                               ajuint identifier,
                                               EnsPQcsubmission *Pqcs);

AjBool ensQcsubmissionadaptorFetchAllByAnalysisQueryTarget(
    EnsPQcsubmissionadaptor qcsa,
    const EnsPAnalysis analysis,
    const EnsPQcdatabase qdb,
    const EnsPQcdatabase tdb,
    AjPList qcss);

AjBool ensQcsubmissionadaptorFetchAllByANQIDTDB(EnsPQcsubmissionadaptor qcsa,
                                                const EnsPAnalysis analysis,
                                                const EnsPQcsequence qseqeunce,
                                                const EnsPQcdatabase tdb,
                                                const EnsPQcsequence tsequence,
                                                ajuint tstart,
                                                ajuint tend,
                                                ajint tstrand,
                                                AjPList qcss);

AjBool ensQcsubmissionadaptorFetchAllByQuery(EnsPQcsubmissionadaptor qcsa,
                                             const EnsPAnalysis analysis,
                                             const EnsPQcdatabase qdb,
                                             AjPList qcss);

AjBool ensQcsubmissionadaptorFetchAllByTarget(EnsPQcsubmissionadaptor qcsa,
                                              const EnsPAnalysis analysis,
                                              const EnsPQcdatabase tdb,
                                              AjPList qcss);

AjBool ensQcsubmissionadaptorStore(EnsPQcsubmissionadaptor qcsa,
                                   EnsPQcsubmission qcs);

AjBool ensQcsubmissionadaptorUpdate(EnsPQcsubmissionadaptor qcsa,
                                    const EnsPQcsubmission qcs);

AjBool ensQcsubmissionadaptorDelete(EnsPQcsubmissionadaptor qcsa,
                                    const EnsPQcsubmission qcs);

/*
** End of prototype definitions
*/




#endif

#ifdef __cplusplus
}
#endif

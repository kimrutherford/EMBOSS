#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ensqcalignment_h
#define ensqcalignment_h

#include "ensanalysis.h"
#include "ensqcsequence.h"




/* EnsPQcalignmentadaptor *****************************************************
**
** Ensembl QC Alignment Adaptor
**
******************************************************************************/

#define EnsPQcalignmentadaptor EnsPBaseadaptor




/* @data EnsPQcalignment ******************************************************
**
** Ensembl QC Alignment
**
** @alias EnsSQcalignment
** @alias EnsOQcalignment
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPQcalignmentadaptor] Ensembl Alignment Adaptor
** @cc Bio::EnsEMBL::QC::Alignment
** @cc 'alignment' SQL table
** @attr Analysis [EnsPAnalysis] Ensembl Analysis
** @attr QuerySequence [EnsPQcsequence] Query Ensembl QC Sequence
** @attr TargetSequence [EnsPQcsequence] Target Ensembl QC Sequence
** @attr VULGAR [AjPStr] VULGAR line
** @attr QueryStart [ajuint] Query start
** @attr QueryEnd [ajuint] Query end
** @attr QueryStrand [ajint] Query strand
** @attr TargetStart [ajuint] Target start
** @attr TargetEnd [ajuint] Target end
** @attr TargetStrand [ajint] Target strand
** @attr SpliceStrand [ajint] Splice strand
** @attr Coverage [ajuint] Coverage score
** @attr Score [double] Score
** @attr Identity [float] Identity
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EnsSQcalignment
{
    ajuint Use;
    ajuint Identifier;
    EnsPQcalignmentadaptor Adaptor;
    EnsPAnalysis Analysis;
    EnsPQcsequence QuerySequence;
    EnsPQcsequence TargetSequence;
    AjPStr VULGAR;
    ajuint QueryStart;
    ajuint QueryEnd;
    ajint QueryStrand;
    ajuint TargetStart;
    ajuint TargetEnd;
    ajint TargetStrand;
    ajint SpliceStrand;
    ajuint Coverage;
    double Score;
    float Identity;
    char Padding[4];
} EnsOQcalignment;

#define EnsPQcalignment EnsOQcalignment*




/*
** Prototype definitions
*/

/* Ensembl Quality Check Alignment */

EnsPQcalignment ensQcalignmentNew(EnsPQcalignmentadaptor qcaa,
                                  ajuint identifier,
                                  EnsPAnalysis analysis,
                                  EnsPQcsequence qsequence,
                                  ajuint qstart,
                                  ajuint qend,
                                  ajint qstrand,
                                  EnsPQcsequence tsequence,
                                  ajuint tstart,
                                  ajuint tend,
                                  ajint tstrand,
                                  ajint sstrand,
                                  ajuint coverage,
                                  double score,
                                  float identity,
                                  AjPStr vulgar);

EnsPQcalignment ensQcalignmentNewObj(const EnsPQcalignment object);

EnsPQcalignment ensQcalignmentNewRef(EnsPQcalignment qca);

void ensQcalignmentDel(EnsPQcalignment* Pqca);

EnsPQcalignmentadaptor ensQcalignmentGetAdaptor(const EnsPQcalignment qca);

ajuint ensQcalignmentGetIdentifier(const EnsPQcalignment qca);

const EnsPAnalysis ensQcalignmentGetAnalysis(const EnsPQcalignment qca);

EnsPQcsequence ensQcalignmentGetQuerySequence(const EnsPQcalignment qca);

ajuint ensQcalignmentGetQueryStart(const EnsPQcalignment qca);

ajuint ensQcalignmentGetQueryEnd(const EnsPQcalignment qca);

ajint ensQcalignmentGetQueryStrand(const EnsPQcalignment qca);

EnsPQcsequence ensQcalignmentGetTargetSequence(const EnsPQcalignment qca);

ajuint ensQcalignmentGetTargetStart(const EnsPQcalignment qca);

ajuint ensQcalignmentGetTargetEnd(const EnsPQcalignment qca);

ajint ensQcalignmentGetTargetStrand(const EnsPQcalignment qca);

ajint ensQcalignmentGetSpliceStrand(const EnsPQcalignment qca);

ajuint ensQcalignmentGetCoverage(const EnsPQcalignment qca);

double ensQcalignmentGetScore(const EnsPQcalignment qca);

float ensQcalignmentGetIdentity(const EnsPQcalignment qca);

AjPStr ensQcalignmentGetVULGAR(const EnsPQcalignment qca);

AjBool ensQcalignmentSetAdaptor(EnsPQcalignment qca,
                                EnsPQcalignmentadaptor qcaa);

AjBool ensQcalignmentSetIdentifier(EnsPQcalignment qca,
                                   ajuint identifier);

AjBool ensQcalignmentSetAnalysis(EnsPQcalignment qca,
                                 EnsPAnalysis analysis);

AjBool ensQcalignmentSetQuerySequence(EnsPQcalignment qca,
                                      EnsPQcsequence qsequence);

AjBool ensQcalignmentSetQueryStart(EnsPQcalignment qca,
                                   ajuint qstart);

AjBool ensQcalignmentSetQueryEnd(EnsPQcalignment qca,
                                 ajuint qend);

AjBool ensQcalignmentSetQueryStrand(EnsPQcalignment qca,
                                    ajint qstrand);

AjBool ensQcalignmentSetTargetSequence(EnsPQcalignment qca,
                                       EnsPQcsequence qcs);

AjBool ensQcalignmentSetTargetStart(EnsPQcalignment qca,
                                    ajuint tstart);

AjBool ensQcalignmentSetTargetEnd(EnsPQcalignment qca,
                                  ajuint tend);

AjBool ensQcalignmentSetTargetStrand(EnsPQcalignment qca,
                                     ajint tstrand);

AjBool ensQcalignmentSetSpliceStrand(EnsPQcalignment qca,
                                     ajint sstrand);

AjBool ensQcalignmentSetCoverage(EnsPQcalignment qca,
                                 ajuint coverage);

AjBool ensQcalignmentSetScore(EnsPQcalignment qca,
                              double score);

AjBool ensQcalignmentSetIdentity(EnsPQcalignment qca,
                                 float identity);

AjBool ensQcalignmentSetVULGAR(EnsPQcalignment qca,
                               AjPStr vulgar);

ajulong ensQcalignmentGetMemsize(const EnsPQcalignment qca);

AjBool ensQcalignmentTrace(const EnsPQcalignment qca, ajuint level);

AjBool ensQcalignmentGetQueryCoordinates(const EnsPQcalignment qca,
                                         ajint *Pstart,
                                         ajint *Pend,
                                         ajint *Pstrand,
                                         ajuint *Plength);

AjBool ensQcalignmentGetTargetCoordinates(const EnsPQcalignment qca,
                                          ajint *Pstart,
                                          ajint *Pend,
                                          ajint *Pstrand,
                                          ajuint *Plength);

AjBool ensQcalignmentOverlapOnTarget(const EnsPQcalignment qca1,
                                     const EnsPQcalignment qca2);

AjBool ensQcalignmentCalculateQueryCoverage(
    EnsPQcalignment qca,
    float identity,
    ajuint edge);

AjBool ensQcalignmentCalculateProteinToProteinQueryCoverage(
    EnsPQcalignment qca,
    float identity);

AjBool ensQcalignmentCalculateProteinToGenomeQueryCoverage(
    EnsPQcalignment qca,
    float identity,
    ajuint edge);

AjBool ensQcalignmentCalculateDNAToDNAQueryCoverage(
    EnsPQcalignment qca,
    float identity,
    ajuint edge);

AjBool ensQcalignmentCalculateDNAToGenomeQueryCoverage(
    EnsPQcalignment qca,
    float identity,
    ajuint edge);

ajuint ensQcalignmentCalculateQueryToQueryCoverage(
    const EnsPQcalignment qca1,
    const EnsPQcalignment qca2);

AjBool ensQcalignmentReport(const EnsPQcalignment qca,
                            AjPStr *Pstr,
                            AjBool internalquery,
                            AjBool internaltarget);

/* Ensembl Quality Check Alignment Adaptor */

EnsPQcalignmentadaptor ensRegistryGetQcalignmentadaptor(
    EnsPDatabaseadaptor dba);

EnsPQcalignmentadaptor ensQcalignmentadaptorNew(
    EnsPDatabaseadaptor dba);

void ensQcalignmentadaptorDel(EnsPQcalignmentadaptor* Pqcaa);

AjBool ensQcalignmentadaptorFetchByIdentifier(
    EnsPQcalignmentadaptor qcaa,
    ajuint identifier,
    EnsPQcalignment *Pqca);

AjBool ensQcalignmentadaptorFetchAllByAnalysisQueryTarget(
    EnsPQcalignmentadaptor qcaa,
    const EnsPAnalysis analysis,
    const EnsPQcdatabase qdb,
    const EnsPQcdatabase tdb,
    AjPList qcas);


AjBool ensQcalignmentadaptorFetchAllByQuery(
    EnsPQcalignmentadaptor qcaa,
    const EnsPAnalysis analysis,
    const EnsPQcdatabase qdb,
    AjPList qcas);

AjBool ensQcalignmentadaptorFetchAllByTarget(
    EnsPQcalignmentadaptor qcaa,
    const EnsPAnalysis analysis,
    const EnsPQcdatabase tdb,
    AjPList qcas);

AjBool ensQcalignmentadaptorFetchAllByCoverage(
    EnsPQcalignmentadaptor qcaa,
    ajuint lower,
    ajuint upper,
    AjPList qcas);

AjBool ensQcalignmentadaptorFetchAllByTargetLocation(
    EnsPQcalignmentadaptor qcaa,
    const EnsPAnalysis analysis,
    const EnsPQcsequence tsequence,
    ajuint tstart,
    ajuint tend,
    AjPList qcas);

AjBool ensQcalignmentadaptorStore(EnsPQcalignmentadaptor qcaa,
                                  EnsPQcalignment qca);

AjBool ensQcalignmentadaptorUpdate(EnsPQcalignmentadaptor qcaa,
                                   const EnsPQcalignment qca);

AjBool ensQcalignmentadaptorDelete(EnsPQcalignmentadaptor qcaa,
                                   EnsPQcalignment qca);

void ensQcalignmentDummyFunction(void);

/*
** End of prototype definitions
*/




#endif /* ensqcalignment_h */

#ifdef __cplusplus
}
#endif

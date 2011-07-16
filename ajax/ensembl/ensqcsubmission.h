
#ifndef ENSQCSUBMISSION_H
#define ENSQCSUBMISSION_H

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ensanalysis.h"
#include "ensqcsequence.h"

AJ_BEGIN_DECLS




/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */

/* @const EnsPQcsubmissionadaptor *********************************************
**
** Ensembl Quality Check Submission Adaptor
**
******************************************************************************/

#define EnsPQcsubmissionadaptor EnsPBaseadaptor




/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */

/* @data EnsPQcsubmission *****************************************************
**
** Ensembl Quality Check Submission
**
** @alias EnsSQcsubmission
** @alias EnsOQcsubmission
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] SQL database-internal identifier
** @attr Adaptor [EnsPQcsubmissionadaptor]
** Ensembl Quality Check Submission Adaptor
** @cc Bio::EnsEMBL::QC::Submission
** @cc 'submission' SQL table
** @attr Analysis [EnsPAnalysis] Ensembl Analysis
** @attr QuerySequence [EnsPQcsequence] Query Ensembl Quality Check Sequence
** @attr TargetSequence [EnsPQcsequence] Target Ensembl Quality Check Sequence
** @attr QueryStart [ajuint] Query start
** @attr QueryEnd [ajuint] Query end
** @attr QueryStrand [ajint] Query strand
** @attr TargetStart [ajuint] Target start
** @attr TargetEnd [ajuint] Target end
** @attr TargetStrand [ajint] Target strand
** @attr Analysisjobidentifier [ajuint] Ensembl Hive Analysis Job identifier
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
    ajuint Analysisjobidentifier;
    ajuint Padding;
} EnsOQcsubmission;

#define EnsPQcsubmission EnsOQcsubmission*




/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */

/*
** Prototype definitions
*/

/* Ensembl Quality Check Submission */

EnsPQcsubmission ensQcsubmissionNewCpy(const EnsPQcsubmission qcsb);

EnsPQcsubmission ensQcsubmissionNewIni(EnsPQcsubmissionadaptor adaptor,
                                       ajuint identifier,
                                       EnsPAnalysis analysis,
                                       EnsPQcsequence qsequence,
                                       EnsPQcsequence tsequence,
                                       ajuint tstart,
                                       ajuint tend,
                                       ajint tstrand,
                                       ajuint analysisjobid);

EnsPQcsubmission ensQcsubmissionNewRef(EnsPQcsubmission qcsb);

void ensQcsubmissionDel(EnsPQcsubmission* Pqcsb);

EnsPQcsubmissionadaptor ensQcsubmissionGetAdaptor(const EnsPQcsubmission qcsb);

EnsPAnalysis ensQcsubmissionGetAnalysis(const EnsPQcsubmission qcsb);

ajuint ensQcsubmissionGetAnalysisjobidentifier(const EnsPQcsubmission qcsb);

ajuint ensQcsubmissionGetIdentifier(const EnsPQcsubmission qcsb);

ajuint ensQcsubmissionGetQueryEnd(const EnsPQcsubmission qcsb);

EnsPQcsequence ensQcsubmissionGetQuerySequence(const EnsPQcsubmission qcsb);

ajuint ensQcsubmissionGetQueryStart(const EnsPQcsubmission qcsb);

ajint ensQcsubmissionGetQueryStrand(const EnsPQcsubmission qcsb);

ajuint ensQcsubmissionGetTargetEnd(const EnsPQcsubmission qcsb);

EnsPQcsequence ensQcsubmissionGetTargetSequence(const EnsPQcsubmission qcsb);

ajuint ensQcsubmissionGetTargetStart(const EnsPQcsubmission qcsb);

ajint ensQcsubmissionGetTargetStrand(const EnsPQcsubmission qcsb);

AjBool ensQcsubmissionSetAdaptor(EnsPQcsubmission qcsb,
                                 EnsPQcsubmissionadaptor qcsba);

AjBool ensQcsubmissionSetAnalysis(EnsPQcsubmission qcsb,
                                  EnsPAnalysis analysis);

AjBool ensQcsubmissionSetAnalysisjobidentifier(EnsPQcsubmission qcsb,
                                               ajuint jobid);

AjBool ensQcsubmissionSetIdentifier(EnsPQcsubmission qcsb,
                                    ajuint identifier);

AjBool ensQcsubmissionSetQueryEnd(EnsPQcsubmission qcsb,
                                  ajuint qend);

AjBool ensQcsubmissionSetQuerySequence(EnsPQcsubmission qcsb,
                                       EnsPQcsequence qsequence);

AjBool ensQcsubmissionSetQueryStart(EnsPQcsubmission qcsb,
                                    ajuint qstart);

AjBool ensQcsubmissionSetQueryStrand(EnsPQcsubmission qcsb,
                                     ajint qstrand);

AjBool ensQcsubmissionSetTargetEnd(EnsPQcsubmission qcsb,
                                   ajuint tend);

AjBool ensQcsubmissionSetTargetSequence(EnsPQcsubmission qcsb,
                                        EnsPQcsequence tsequence);

AjBool ensQcsubmissionSetTargetStart(EnsPQcsubmission qcsb,
                                     ajuint tstart);

AjBool ensQcsubmissionSetTargetStrand(EnsPQcsubmission qcsb,
                                      ajint tstrand);

AjBool ensQcsubmissionTrace(const EnsPQcsubmission qcsb, ajuint level);

size_t ensQcsubmissionCalculateMemsize(const EnsPQcsubmission qcsb);

/* Ensembl Quality Check Submission Adaptor */

EnsPQcsubmissionadaptor ensRegistryGetQcsubmissionadaptor(
    EnsPDatabaseadaptor dba);

EnsPQcsubmissionadaptor ensQcsubmissionadaptorNew(
    EnsPDatabaseadaptor dba);

void ensQcsubmissionadaptorDel(EnsPQcsubmissionadaptor* Pqcsba);

EnsPBaseadaptor ensQcsubmissionadaptorGetBaseadaptor(
    EnsPQcsubmissionadaptor qcsba);

EnsPDatabaseadaptor ensQcsubmissionadaptorGetDatabaseadaptor(
    EnsPQcsubmissionadaptor qcsba);

AjBool ensQcsubmissionadaptorFetchAllbyQcdatabasePair(
    EnsPQcsubmissionadaptor qcsba,
    const EnsPAnalysis analysis,
    const EnsPQcdatabase qdb,
    const EnsPQcdatabase tdb,
    AjPList qcsbs);

AjBool ensQcsubmissionadaptorFetchAllbyQcdatabaseQuery(
    EnsPQcsubmissionadaptor qcsba,
    const EnsPAnalysis analysis,
    const EnsPQcdatabase qdb,
    AjPList qcsbs);

AjBool ensQcsubmissionadaptorFetchAllbyQcdatabaseTarget(
    EnsPQcsubmissionadaptor qcsba,
    const EnsPAnalysis analysis,
    const EnsPQcdatabase tdb,
    AjPList qcsbs);

AjBool ensQcsubmissionadaptorFetchAllbyRegion(
    EnsPQcsubmissionadaptor qcsba,
    const EnsPAnalysis analysis,
    const EnsPQcsequence qsequence,
    const EnsPQcdatabase tdb,
    const EnsPQcsequence tsequence,
    ajuint tstart,
    ajuint tend,
    ajint tstrand,
    AjPList qcsbs);

AjBool ensQcsubmissionadaptorFetchByIdentifier(
    EnsPQcsubmissionadaptor adaptor,
    ajuint identifier,
    EnsPQcsubmission* Pqcsb);

AjBool ensQcsubmissionadaptorDelete(EnsPQcsubmissionadaptor qcsba,
                                    EnsPQcsubmission qcsb);

AjBool ensQcsubmissionadaptorStore(EnsPQcsubmissionadaptor qcsba,
                                   EnsPQcsubmission qcsb);

AjBool ensQcsubmissionadaptorUpdate(EnsPQcsubmissionadaptor qcsba,
                                    const EnsPQcsubmission qcsb);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSQCSUBMISSION_H */

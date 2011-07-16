
#ifndef ENSANALYSIS_H
#define ENSANALYSIS_H

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ensbaseadaptor.h"

AJ_BEGIN_DECLS




/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */




/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */

/*
** Prototype definitions
*/

/* Ensembl Analysis */

EnsPAnalysis ensAnalysisNewCpy(const EnsPAnalysis analysis);

EnsPAnalysis ensAnalysisNewIni(EnsPAnalysisadaptor aa,
                               ajuint identifier,
                               AjPStr cdate,
                               AjPStr name,
                               AjPStr databasename,
                               AjPStr databaseversion,
                               AjPStr databasefile,
                               AjPStr programname,
                               AjPStr programversion,
                               AjPStr programfile,
                               AjPStr parameters,
                               AjPStr modulename,
                               AjPStr moduleversion,
                               AjPStr gffsource,
                               AjPStr gfffeature,
                               AjPStr description,
                               AjPStr displaylabel,
                               AjPStr webdata,
                               AjBool displayable);

EnsPAnalysis ensAnalysisNewRef(EnsPAnalysis analysis);

void ensAnalysisDel(EnsPAnalysis* Panalysis);

EnsPAnalysisadaptor ensAnalysisGetAdaptor(const EnsPAnalysis analysis);

ajuint ensAnalysisGetIdentifier(const EnsPAnalysis analysis);

const AjPStr ensAnalysisGetName(const EnsPAnalysis analysis);

const AjPStr ensAnalysisGetDatabasefile(const EnsPAnalysis analysis);

const AjPStr ensAnalysisGetDatabasename(const EnsPAnalysis analysis);

const AjPStr ensAnalysisGetDatabaseversion(const EnsPAnalysis analysis);

const AjPStr ensAnalysisGetDateCreation(const EnsPAnalysis analysis);

const AjPStr ensAnalysisGetProgramname(const EnsPAnalysis analysis);

const AjPStr ensAnalysisGetProgramversion(const EnsPAnalysis analysis);

const AjPStr ensAnalysisGetProgramfile(const EnsPAnalysis analysis);

const AjPStr ensAnalysisGetParameters(const EnsPAnalysis analysis);

const AjPStr ensAnalysisGetModulename(const EnsPAnalysis analysis);

const AjPStr ensAnalysisGetModuleversion(const EnsPAnalysis analysis);

const AjPStr ensAnalysisGetGffsource(const EnsPAnalysis analysis);

const AjPStr ensAnalysisGetGfffeature(const EnsPAnalysis analysis);

const AjPStr ensAnalysisGetDescription(const EnsPAnalysis analysis);

const AjPStr ensAnalysisGetDisplaylabel(const EnsPAnalysis analysis);

const AjPStr ensAnalysisGetWebdata(const EnsPAnalysis analysis);

AjBool ensAnalysisGetDisplayable(const EnsPAnalysis analysis);

AjBool ensAnalysisSetAdaptor(EnsPAnalysis analysis,
                             EnsPAnalysisadaptor aa);

AjBool ensAnalysisSetDatabasefile(EnsPAnalysis analysis,
                                  AjPStr databasefile);

AjBool ensAnalysisSetDatabasename(EnsPAnalysis analysis,
                                  AjPStr databasename);

AjBool ensAnalysisSetDateCreation(EnsPAnalysis analysis,
                                  AjPStr cdate);

AjBool ensAnalysisSetDescription(EnsPAnalysis analysis,
                                 AjPStr description);

AjBool ensAnalysisSetDisplayable(EnsPAnalysis analysis,
                                 AjBool displayable);

AjBool ensAnalysisSetDisplaylabel(EnsPAnalysis analysis,
                                  AjPStr displaylabel);

AjBool ensAnalysisSetDatabaseversion(EnsPAnalysis analysis,
                                     AjPStr databaseversion);

AjBool ensAnalysisSetIdentifier(EnsPAnalysis analysis,
                                ajuint identifier);

AjBool ensAnalysisSetGfffeature(EnsPAnalysis analysis,
                                AjPStr gfffeature);

AjBool ensAnalysisSetGffsource(EnsPAnalysis analysis,
                               AjPStr gffsource);

AjBool ensAnalysisSetModulename(EnsPAnalysis analysis,
                                AjPStr modulename);

AjBool ensAnalysisSetModuleversion(EnsPAnalysis analysis,
                                   AjPStr moduleversion);

AjBool ensAnalysisSetName(EnsPAnalysis analysis,
                          AjPStr name);

AjBool ensAnalysisSetParameters(EnsPAnalysis analysis,
                                AjPStr parameters);

AjBool ensAnalysisSetProgramfile(EnsPAnalysis analysis,
                                 AjPStr programfile);

AjBool ensAnalysisSetProgramname(EnsPAnalysis analysis,
                                 AjPStr programname);

AjBool ensAnalysisSetProgramversion(EnsPAnalysis analysis,
                                    AjPStr programversion);

AjBool ensAnalysisSetWebdata(EnsPAnalysis analysis,
                             AjPStr webdata);

AjBool ensAnalysisTrace(const EnsPAnalysis analysis, ajuint level);

size_t ensAnalysisCalculateMemsize(const EnsPAnalysis analysis);

AjBool ensAnalysisIsDatabase(const EnsPAnalysis analysis);

AjBool ensAnalysisMatch(const EnsPAnalysis analysis1,
                        const EnsPAnalysis analysis2);

/* Ensembl Analysis Adaptor */

EnsPAnalysisadaptor ensRegistryGetAnalysisadaptor(
    EnsPDatabaseadaptor dba);

EnsPAnalysisadaptor ensAnalysisadaptorNew(
    EnsPDatabaseadaptor dba);

AjBool ensAnalysisadaptorCacheClear(EnsPAnalysisadaptor aa);

void ensAnalysisadaptorDel(EnsPAnalysisadaptor* Paa);

EnsPBaseadaptor ensAnalysisadaptorGetBaseadaptor(
    const EnsPAnalysisadaptor aa);

EnsPDatabaseadaptor ensAnalysisadaptorGetDatabaseadaptor(
    const EnsPAnalysisadaptor aa);

AjBool ensAnalysisadaptorFetchAll(EnsPAnalysisadaptor aa,
                                  AjPList analyses);

AjBool ensAnalysisadaptorFetchAllbyFeatureclass(EnsPAnalysisadaptor aa,
                                                const AjPStr class,
                                                AjPList analyses);

AjBool ensAnalysisadaptorFetchByIdentifier(EnsPAnalysisadaptor aa,
                                           ajuint identifier,
                                           EnsPAnalysis* Panalysis);

AjBool ensAnalysisadaptorFetchByName(EnsPAnalysisadaptor aa,
                                     const AjPStr name,
                                     EnsPAnalysis* Panalysis);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSANALYSIS_H */

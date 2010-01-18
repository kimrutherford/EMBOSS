#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ensanalysis_h
#define ensanalysis_h

#include "ensbaseadaptor.h"




/*
** Prototype definitions
*/

/* Ensembl Analysis */

EnsPAnalysis ensAnalysisNewData(EnsPAnalysisadaptor adaptor,
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
                                AjBool displayable,
                                AjPStr webdata);

EnsPAnalysis ensAnalysisNewObj(const EnsPAnalysis object);

EnsPAnalysis ensAnalysisNewRef(EnsPAnalysis analysis);

void ensAnalysisDel(EnsPAnalysis* Panalysis);

EnsPAnalysisadaptor ensAnalysisGetAdaptor(const EnsPAnalysis analysis);

ajuint ensAnalysisGetIdentifier(const EnsPAnalysis analysis);

const AjPStr ensAnalysisGetCreationDate(const EnsPAnalysis analysis);

const AjPStr ensAnalysisGetName(const EnsPAnalysis analysis);

const AjPStr ensAnalysisGetDatabaseName(const EnsPAnalysis analysis);

const AjPStr ensAnalysisGetDatabaseVersion(const EnsPAnalysis analysis);

const AjPStr ensAnalysisGetDatabaseFile(const EnsPAnalysis analysis);

const AjPStr ensAnalysisGetProgramName(const EnsPAnalysis analysis);

const AjPStr ensAnalysisGetProgramVersion(const EnsPAnalysis analysis);

const AjPStr ensAnalysisGetProgramFile(const EnsPAnalysis analysis);

const AjPStr ensAnalysisGetParameters(const EnsPAnalysis analysis);

const AjPStr ensAnalysisGetModuleName(const EnsPAnalysis analysis);

const AjPStr ensAnalysisGetModuleVersion(const EnsPAnalysis analysis);

const AjPStr ensAnalysisGetGFFSource(const EnsPAnalysis analysis);

const AjPStr ensAnalysisGetGFFFeature(const EnsPAnalysis analysis);

const AjPStr ensAnalysisGetDescription(const EnsPAnalysis analysis);

const AjPStr ensAnalysisGetDisplayLabel(const EnsPAnalysis analysis);

extern const AjPStr ensAnalysisGetWebData(const EnsPAnalysis analysis);

AjBool ensAnalysisGetDisplayable(const EnsPAnalysis analysis);

AjBool ensAnalysisSetAdaptor(EnsPAnalysis analysis,
                             EnsPAnalysisadaptor adaptor);

AjBool ensAnalysisSetIdentifier(EnsPAnalysis analysis, ajuint identifier);

AjBool ensAnalysisSetCreationDate(EnsPAnalysis analysis, AjPStr cdate);

AjBool ensAnalysisSetName(EnsPAnalysis analysis, AjPStr name);

AjBool ensAnalysisSetDatabaseName(EnsPAnalysis analysis, AjPStr databasename);

AjBool ensAnalysisSetDatabaseVersion(EnsPAnalysis analysis,
                                     AjPStr databaseversion);

AjBool ensAnalysisSetDatabaseFile(EnsPAnalysis analysis, AjPStr databasefile);

AjBool ensAnalysisSetProgramName(EnsPAnalysis analysis, AjPStr programname);

AjBool ensAnalysisSetProgramVersion(EnsPAnalysis analysis,
                                    AjPStr programversion);

AjBool ensAnalysisSetProgramFile(EnsPAnalysis analysis, AjPStr programfile);

AjBool ensAnalysisSetParameters(EnsPAnalysis analysis, AjPStr parameters);

AjBool ensAnalysisSetModuleName(EnsPAnalysis analysis, AjPStr modulename);

AjBool ensAnalysisSetModuleVersion(EnsPAnalysis analysis,
                                   AjPStr moduleversion);

AjBool ensAnalysisSetGFFSource(EnsPAnalysis analysis, AjPStr gffsource);

AjBool ensAnalysisSetGFFFeature(EnsPAnalysis analysis, AjPStr gfffeature);

AjBool ensAnalysisSetDescription(EnsPAnalysis analysis, AjPStr description);

AjBool ensAnalysisSetDisplayLabel(EnsPAnalysis analysis, AjPStr displaylabel);

AjBool ensAnalysisSetWebData(EnsPAnalysis analysis, AjPStr webdata);

AjBool ensAnalysisSetDisplayable(EnsPAnalysis analysis, AjBool displayable);

AjBool ensAnalysisTrace(const EnsPAnalysis analysis, ajuint level);

AjBool ensAnalysisIsDatabase(const EnsPAnalysis analysis);

AjBool ensAnalysisMatch(const EnsPAnalysis analysis1,
                        const EnsPAnalysis analysis2);

ajuint ensAnalysisGetMemSize(const EnsPAnalysis analysis);

/* Ensembl Analysis Adaptor */

EnsPAnalysisadaptor ensAnalysisadaptorNew(EnsPDatabaseadaptor dba);

void ensAnalysisadaptorDel(EnsPAnalysisadaptor* Padaptor);

EnsPBaseadaptor ensAnalysisadaptorGetBaseadaptor(
    const EnsPAnalysisadaptor adaptor);

EnsPDatabaseadaptor ensAnalysisadaptorGetDatabaseadaptor(
    const EnsPAnalysisadaptor adaptor);

AjBool ensAnalysisadaptorFetchAll(EnsPAnalysisadaptor adaptor,
                                  AjPList analyses);

AjBool ensAnalysisadaptorFetchByIdentifier(EnsPAnalysisadaptor adaptor,
                                           ajuint identifier,
                                           EnsPAnalysis *Panalysis);

AjBool ensAnalysisadaptorFetchByName(EnsPAnalysisadaptor adaptor,
                                     const AjPStr name,
                                     EnsPAnalysis *Panalysis);

AjBool ensAnalysisadaptorFetchAllByFeatureClass(EnsPAnalysisadaptor adaptor,
                                                const AjPStr class,
                                                AjPList analyses);

/*
** End of prototype definitions
*/




#endif

#ifdef __cplusplus
}
#endif

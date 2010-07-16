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

EnsPAnalysis ensAnalysisNewData(EnsPAnalysisadaptor aa,
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

const AjPStr ensAnalysisGetCreationdate(const EnsPAnalysis analysis);

const AjPStr ensAnalysisGetName(const EnsPAnalysis analysis);

const AjPStr ensAnalysisGetDatabasename(const EnsPAnalysis analysis);

const AjPStr ensAnalysisGetDatabaseversion(const EnsPAnalysis analysis);

const AjPStr ensAnalysisGetDatabasefile(const EnsPAnalysis analysis);

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

AjBool ensAnalysisSetIdentifier(EnsPAnalysis analysis, ajuint identifier);

AjBool ensAnalysisSetCreationdate(EnsPAnalysis analysis, AjPStr cdate);

AjBool ensAnalysisSetName(EnsPAnalysis analysis, AjPStr name);

AjBool ensAnalysisSetDatabasename(EnsPAnalysis analysis, AjPStr databasename);

AjBool ensAnalysisSetDatabaseversion(EnsPAnalysis analysis,
                                     AjPStr databaseversion);

AjBool ensAnalysisSetDatabasefile(EnsPAnalysis analysis, AjPStr databasefile);

AjBool ensAnalysisSetProgramname(EnsPAnalysis analysis, AjPStr programname);

AjBool ensAnalysisSetProgramversion(EnsPAnalysis analysis,
                                    AjPStr programversion);

AjBool ensAnalysisSetProgramfile(EnsPAnalysis analysis, AjPStr programfile);

AjBool ensAnalysisSetParameters(EnsPAnalysis analysis, AjPStr parameters);

AjBool ensAnalysisSetModulename(EnsPAnalysis analysis, AjPStr modulename);

AjBool ensAnalysisSetModuleversion(EnsPAnalysis analysis,
                                   AjPStr moduleversion);

AjBool ensAnalysisSetGffsource(EnsPAnalysis analysis, AjPStr gffsource);

AjBool ensAnalysisSetGfffeature(EnsPAnalysis analysis, AjPStr gfffeature);

AjBool ensAnalysisSetDescription(EnsPAnalysis analysis, AjPStr description);

AjBool ensAnalysisSetDisplaylabel(EnsPAnalysis analysis, AjPStr displaylabel);

AjBool ensAnalysisSetWebdata(EnsPAnalysis analysis, AjPStr webdata);

AjBool ensAnalysisSetDisplayable(EnsPAnalysis analysis, AjBool displayable);

AjBool ensAnalysisTrace(const EnsPAnalysis analysis, ajuint level);

AjBool ensAnalysisIsDatabase(const EnsPAnalysis analysis);

AjBool ensAnalysisMatch(const EnsPAnalysis analysis1,
                        const EnsPAnalysis analysis2);

ajulong ensAnalysisGetMemsize(const EnsPAnalysis analysis);

/* Ensembl Analysis Adaptor */

EnsPAnalysisadaptor ensRegistryGetAnalysisadaptor(
    EnsPDatabaseadaptor dba);

EnsPAnalysisadaptor ensAnalysisadaptorNew(
    EnsPDatabaseadaptor dba);

void ensAnalysisadaptorDel(EnsPAnalysisadaptor* Paa);

EnsPBaseadaptor ensAnalysisadaptorGetBaseadaptor(
    const EnsPAnalysisadaptor aa);

EnsPDatabaseadaptor ensAnalysisadaptorGetDatabaseadaptor(
    const EnsPAnalysisadaptor aa);

AjBool ensAnalysisadaptorFetchAll(EnsPAnalysisadaptor aa,
                                  AjPList analyses);

AjBool ensAnalysisadaptorFetchByIdentifier(EnsPAnalysisadaptor aa,
                                           ajuint identifier,
                                           EnsPAnalysis *Panalysis);

AjBool ensAnalysisadaptorFetchByName(EnsPAnalysisadaptor aa,
                                     const AjPStr name,
                                     EnsPAnalysis *Panalysis);

AjBool ensAnalysisadaptorFetchAllByFeatureclass(EnsPAnalysisadaptor aa,
                                                const AjPStr class,
                                                AjPList analyses);

/*
** End of prototype definitions
*/




#endif /* ensanalysis_h */

#ifdef __cplusplus
}
#endif

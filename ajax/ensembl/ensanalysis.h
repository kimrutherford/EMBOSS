/* @include ensanalysis *******************************************************
**
** Ensembl Analysis functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.18 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/02/20 22:04:34 $ by $Author: mks $
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

#ifndef ENSANALYSIS_H
#define ENSANALYSIS_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ensbaseadaptor.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */

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

void ensAnalysisDel(EnsPAnalysis *Panalysis);

EnsPAnalysisadaptor ensAnalysisGetAdaptor(const EnsPAnalysis analysis);

AjPStr ensAnalysisGetDatabasefile(const EnsPAnalysis analysis);

AjPStr ensAnalysisGetDatabasename(const EnsPAnalysis analysis);

AjPStr ensAnalysisGetDatabaseversion(const EnsPAnalysis analysis);

AjPStr ensAnalysisGetDateCreation(const EnsPAnalysis analysis);

AjPStr ensAnalysisGetDescription(const EnsPAnalysis analysis);

AjBool ensAnalysisGetDisplayable(const EnsPAnalysis analysis);

AjPStr ensAnalysisGetDisplaylabel(const EnsPAnalysis analysis);

AjPStr ensAnalysisGetGfffeature(const EnsPAnalysis analysis);

AjPStr ensAnalysisGetGffsource(const EnsPAnalysis analysis);

ajuint ensAnalysisGetIdentifier(const EnsPAnalysis analysis);

AjPStr ensAnalysisGetModulename(const EnsPAnalysis analysis);

AjPStr ensAnalysisGetModuleversion(const EnsPAnalysis analysis);

AjPStr ensAnalysisGetName(const EnsPAnalysis analysis);

AjPStr ensAnalysisGetParameters(const EnsPAnalysis analysis);

AjPStr ensAnalysisGetProgramfile(const EnsPAnalysis analysis);

AjPStr ensAnalysisGetProgramname(const EnsPAnalysis analysis);

AjPStr ensAnalysisGetProgramversion(const EnsPAnalysis analysis);

AjPStr ensAnalysisGetWebdata(const EnsPAnalysis analysis);

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

AjBool ensListAnalysisSortIdentifierAscending(AjPList analyses);

AjBool ensListAnalysisSortIdentifierDescending(AjPList analyses);

AjBool ensListAnalysisSortNameAscending(AjPList analyses);

AjBool ensListAnalysisSortNameDescending(AjPList analyses);

/* Ensembl Analysis Adaptor */

EnsPAnalysisadaptor ensRegistryGetAnalysisadaptor(
    EnsPDatabaseadaptor dba);

EnsPAnalysisadaptor ensAnalysisadaptorNew(
    EnsPDatabaseadaptor dba);

AjBool ensAnalysisadaptorCacheClear(EnsPAnalysisadaptor aa);

void ensAnalysisadaptorDel(EnsPAnalysisadaptor *Paa);

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
                                           EnsPAnalysis *Panalysis);

AjBool ensAnalysisadaptorFetchByName(EnsPAnalysisadaptor aa,
                                     const AjPStr name,
                                     EnsPAnalysis *Panalysis);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSANALYSIS_H */

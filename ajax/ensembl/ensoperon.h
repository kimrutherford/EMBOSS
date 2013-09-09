/* @include ensoperon *********************************************************
**
** Ensembl Operon functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.1 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/08/05 11:18:13 $ by $Author: mks $
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

#ifndef ENSOPERON_H
#define ENSOPERON_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ensfeature.h"

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

/* Ensembl Operon */

EnsPOperon ensOperonNewCpy(const EnsPOperon operon);

EnsPOperon ensOperonNewIni(EnsPOperonadaptor opa,
                           ajuint identifier,
                           EnsPFeature feature,
                           AjPStr displaylabel,
                           AjPStr stableid,
                           ajuint version,
                           AjPStr cdate,
                           AjPStr mdate);

EnsPOperon ensOperonNewRef(EnsPOperon operon);

void ensOperonDel(EnsPOperon *Poperon);

EnsPOperonadaptor ensOperonGetAdaptor(
    const EnsPOperon operon);

AjPStr ensOperonGetDateCreation(
    const EnsPOperon operon);

AjPStr ensOperonGetDateModification(
    const EnsPOperon operon);

AjPStr ensOperonGetDisplaylabel(
    const EnsPOperon operon);

EnsPFeature ensOperonGetFeature(
    const EnsPOperon operon);

ajuint ensOperonGetIdentifier(
    const EnsPOperon operon);

AjPStr ensOperonGetStableidentifier(
    const EnsPOperon operon);

ajuint ensOperonGetVersion(
    const EnsPOperon operon);

const AjPList ensOperonLoadAttributes(EnsPOperon operon);

const AjPList ensOperonLoadDatabaseentries(EnsPOperon operon);

const AjPList ensOperonLoadOperontranscripts(EnsPOperon operon);

AjBool ensOperonSetAdaptor(EnsPOperon operon, EnsPOperonadaptor opa);

AjBool ensOperonSetDateCreation(EnsPOperon operon, AjPStr cdate);

AjBool ensOperonSetDateModification(EnsPOperon operon, AjPStr mdate);

AjBool ensOperonSetDisplaylabel(EnsPOperon operon, AjPStr displaylabel);

AjBool ensOperonSetFeature(EnsPOperon operon, EnsPFeature feature);

AjBool ensOperonSetIdentifier(EnsPOperon operon, ajuint identifier);

AjBool ensOperonSetStableidentifier(EnsPOperon operon, AjPStr stableid);

AjBool ensOperonSetVersion(EnsPOperon operon, ajuint version);

AjBool ensOperonTrace(const EnsPOperon operon, ajuint level);

size_t ensOperonCalculateMemsize(const EnsPOperon operon);

AjBool ensOperonAddAttribute(EnsPOperon operon,
                             EnsPAttribute attribute);

AjBool ensOperonAddDatabaseentry(EnsPOperon operon,
                                 EnsPDatabaseentry dbe);

AjBool ensOperonAddOperontranscript(EnsPOperon operon,
                                    EnsPOperontranscript ot);

AjBool ensOperonFetchAllAttributes(EnsPOperon operon,
                                   const AjPStr code,
                                   AjPList attributes);

AjBool ensOperonFetchAllDatabaseentries(EnsPOperon operon,
                                        const AjPStr name,
                                        EnsEExternaldatabaseType type,
                                        AjPList dbes);

/* Ensembl Operon Adaptor */

EnsPOperonadaptor ensRegistryGetOperonadaptor(
    EnsPDatabaseadaptor dba);

EnsPOperonadaptor ensOperonadaptorNew(
    EnsPDatabaseadaptor dba);

void ensOperonadaptorDel(EnsPOperonadaptor *Popa);

EnsPBaseadaptor ensOperonadaptorGetBaseadaptor(
    EnsPOperonadaptor opa);

EnsPDatabaseadaptor ensOperonadaptorGetDatabaseadaptor(
    EnsPOperonadaptor opa);

EnsPFeatureadaptor ensOperonadaptorGetFeatureadaptor(
    EnsPOperonadaptor opa);

AjBool ensOperonadaptorFetchAll(
    EnsPOperonadaptor opa,
    AjPList operons);

AjBool ensOperonadaptorFetchByDisplaylabel(
    EnsPOperonadaptor opa,
    const AjPStr displaylabel,
    EnsPOperon *Poperon);

AjBool ensOperonadaptorFetchByStableidentifier(
    EnsPOperonadaptor opa,
    const AjPStr stableid,
    ajuint version,
    EnsPOperon *Poperon);

AjBool ensOperonadaptorRetrieveAllIdentifiers(
    EnsPOperonadaptor opa,
    AjPList identifiers);

AjBool ensOperonadaptorRetrieveAllStableidentifiers(
    EnsPOperonadaptor opa,
    AjPList stableids);

/* Ensembl Operon Transcript */

EnsPOperontranscript ensOperontranscriptNewCpy(const EnsPOperontranscript ot);

EnsPOperontranscript ensOperontranscriptNewIni(
    EnsPOperontranscriptadaptor ota,
    ajuint identifier,
    EnsPFeature feature,
    AjPStr displaylabel,
    AjPStr stableid,
    ajuint version,
    AjPStr cdate,
    AjPStr mdate);

EnsPOperontranscript ensOperontranscriptNewRef(EnsPOperontranscript ot);

void ensOperontranscriptDel(EnsPOperontranscript *Pot);

EnsPOperontranscriptadaptor ensOperontranscriptGetAdaptor(
    const EnsPOperontranscript ot);

AjPStr ensOperontranscriptGetDateCreation(
    const EnsPOperontranscript ot);

AjPStr ensOperontranscriptGetDateModification(
    const EnsPOperontranscript ot);

AjPStr ensOperontranscriptGetDisplaylabel(
    const EnsPOperontranscript ot);

EnsPFeature ensOperontranscriptGetFeature(
    const EnsPOperontranscript ot);

ajuint ensOperontranscriptGetIdentifier(
    const EnsPOperontranscript ot);

AjPStr ensOperontranscriptGetStableidentifier(
    const EnsPOperontranscript ot);

ajuint ensOperontranscriptGetVersion(
    const EnsPOperontranscript ot);

const AjPList ensOperontranscriptLoadAttributes(EnsPOperontranscript ot);

const AjPList ensOperontranscriptLoadDatabaseentries(EnsPOperontranscript ot);

const AjPList ensOperontranscriptLoadGenes(EnsPOperontranscript ot);

AjBool ensOperontranscriptSetAdaptor(EnsPOperontranscript ot,
                                     EnsPOperontranscriptadaptor ota);

AjBool ensOperontranscriptSetDateCreation(EnsPOperontranscript ot,
                                          AjPStr cdate);

AjBool ensOperontranscriptSetDateModification(EnsPOperontranscript ot,
                                              AjPStr mdate);

AjBool ensOperontranscriptSetDisplaylabel(EnsPOperontranscript ot,
                                          AjPStr displaylabel);

AjBool ensOperontranscriptSetFeature(EnsPOperontranscript ot,
                                     EnsPFeature feature);

AjBool ensOperontranscriptSetIdentifier(EnsPOperontranscript ot,
                                        ajuint identifier);

AjBool ensOperontranscriptSetStableidentifier(EnsPOperontranscript ot,
                                              AjPStr stableid);

AjBool ensOperontranscriptSetVersion(EnsPOperontranscript ot,
                                     ajuint version);

AjBool ensOperontranscriptTrace(const EnsPOperontranscript ot, ajuint level);

size_t ensOperontranscriptCalculateMemsize(const EnsPOperontranscript ot);

AjBool ensOperontranscriptAddAttribute(EnsPOperontranscript ot,
                                       EnsPAttribute attribute);

AjBool ensOperontranscriptAddDatabaseentry(EnsPOperontranscript ot,
                                           EnsPDatabaseentry dbe);

AjBool ensOperontranscriptAddGene(EnsPOperontranscript ot,
                                  EnsPGene gene);

AjBool ensOperontranscriptFetchAllAttributes(
    EnsPOperontranscript ot,
    const AjPStr code,
    AjPList attributes);

AjBool ensOperontranscriptFetchAllDatabaseentries(
    EnsPOperontranscript ot,
    const AjPStr name,
    EnsEExternaldatabaseType type,
    AjPList dbes);

AjBool ensOperontranscriptFetchOperon(
    EnsPOperontranscript ot,
    EnsPOperon *Poperon);

/* Ensembl Operon Transcript Adaptor */

EnsPOperontranscriptadaptor ensRegistryGetOperontranscriptadaptor(
    EnsPDatabaseadaptor dba);

EnsPOperontranscriptadaptor ensOperontranscriptadaptorNew(
    EnsPDatabaseadaptor dba);

void ensOperontranscriptadaptorDel(EnsPOperontranscriptadaptor *Pota);

EnsPBaseadaptor ensOperontranscriptadaptorGetBaseadaptor(
    EnsPOperontranscriptadaptor ota);

EnsPDatabaseadaptor ensOperontranscriptadaptorGetDatabaseadaptor(
    EnsPOperontranscriptadaptor ota);

EnsPFeatureadaptor ensOperontranscriptadaptorGetFeatureadaptor(
    EnsPOperontranscriptadaptor ota);

AjBool ensOperontranscriptadaptorFetchAll(
    EnsPOperontranscriptadaptor ota,
    AjPList ots);

AjBool ensOperontranscriptadaptorFetchAllbyGene(
    EnsPOperontranscriptadaptor ota,
    const EnsPGene gene,
    AjPList ots);

AjBool ensOperontranscriptadaptorFetchAllbyOperon(
    EnsPOperontranscriptadaptor ota,
    const EnsPOperon operon,
    AjPList ots);

AjBool ensOperontranscriptadaptorFetchByDisplaylabel(
    EnsPOperontranscriptadaptor ota,
    const AjPStr label,
    EnsPOperontranscript *Pot);

AjBool ensOperontranscriptadaptorFetchByIdentifier(
    EnsPOperontranscriptadaptor ota,
    ajuint identifier,
    EnsPOperontranscript *Pot);

AjBool ensOperonadaptorFetchByOperontranscriptidentifier(
    EnsPOperonadaptor opa,
    ajuint otidentifier,
    EnsPOperon *Poperon);

AjBool ensOperonadaptorFetchByOperontranscriptstableidentifier(
    EnsPOperonadaptor opa,
    const AjPStr stableid,
    EnsPOperon *Poperon);

AjBool ensOperontranscriptadaptorFetchByStableidentifier(
    EnsPOperontranscriptadaptor ota,
    const AjPStr stableid,
    ajuint version,
    EnsPOperontranscript *Pot);

AjBool ensOperontranscriptadaptorRetrieveAllGenesByOperontranscriptidentifier(
    EnsPOperontranscriptadaptor ota,
    ajuint identifier,
    AjPList genes);

AjBool ensOperontranscriptadaptorRetrieveAllIdentifiers(
    EnsPOperontranscriptadaptor ota,
    AjPList identifiers);

AjBool ensOperontranscriptadaptorRetrieveAllStableidentifiers(
    EnsPOperontranscriptadaptor ota,
    AjPList stableids);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSOPERON_H */

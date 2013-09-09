/* @include ensintron *********************************************************
**
** Ensembl Intron functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.13 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2013/02/17 13:07:53 $ by $Author: mks $
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

#ifndef ENSINTRON_H
#define ENSINTRON_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ensdata.h"

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

/* Ensembl Intron */

EnsPIntron ensIntronNewCpy(const EnsPIntron intron);

EnsPIntron ensIntronNewIni(EnsPExon exon1,
                           EnsPExon exon2,
                           EnsPAnalysis analysis);

EnsPIntron ensIntronNewRef(EnsPIntron intron);

void ensIntronDel(EnsPIntron *Pintron);

EnsPFeature ensIntronGetFeature(const EnsPIntron intron);

EnsPExon ensIntronGetNextexon(const EnsPIntron intron);

EnsPExon ensIntronGetPreviousexon(const EnsPIntron intron);

AjBool ensIntronSetFeature(EnsPIntron intron,
                           EnsPFeature feature);

AjBool ensIntronSetNextexon(EnsPIntron intron,
                            EnsPExon exon);

AjBool ensIntronSetPreviousexon(EnsPIntron intron,
                                EnsPExon exon);

AjBool ensIntronTrace(const EnsPIntron intron, ajuint level);

ajuint ensIntronCalculateLength(const EnsPIntron intron);

size_t ensIntronCalculateMemsize(const EnsPIntron intron);

AjBool ensIntronFetchSplicesequences(const EnsPIntron intron,
                                     AjPStr *Pdonor,
                                     AjPStr *Pacceptor);

AjBool ensIntronIsCanonical(const EnsPIntron intron, AjBool *Presult);

AjBool ensIntronMatch(const EnsPIntron intron1,
                      const EnsPIntron intron2);

EnsPIntron ensIntronTransfer(EnsPIntron intron,
                             EnsPSlice slice);

EnsPIntron ensIntronTransform(EnsPIntron intron,
                              const AjPStr csname,
                              const AjPStr csversion);

/* Ensembl Intron Supporting Evidence */

EnsPIntronsupportingevidence ensIntronsupportingevidenceNewCpy(
    const EnsPIntronsupportingevidence ise);

EnsPIntronsupportingevidence ensIntronsupportingevidenceNewIni(
    EnsPIntronsupportingevidenceadaptor isea,
    ajuint identifier,
    EnsPFeature feature,
    EnsPIntron intron,
    AjPStr hitname,
    AjBool canonical,
    double score,
    EnsEIntronsupportingevidenceType type);

EnsPIntronsupportingevidence ensIntronsupportingevidenceNewRef(
    EnsPIntronsupportingevidence ise);

void ensIntronsupportingevidenceDel(EnsPIntronsupportingevidence *Pise);

EnsPIntronsupportingevidenceadaptor ensIntronsupportingevidenceGetAdaptor(
    const EnsPIntronsupportingevidence ise);

AjBool ensIntronsupportingevidenceGetCanonical(
    const EnsPIntronsupportingevidence ise);

EnsPFeature ensIntronsupportingevidenceGetFeature(
    const EnsPIntronsupportingevidence ise);

AjPStr ensIntronsupportingevidenceGetHitname(
    const EnsPIntronsupportingevidence ise);

ajuint ensIntronsupportingevidenceGetIdentifier(
    const EnsPIntronsupportingevidence ise);

EnsPIntron ensIntronsupportingevidenceGetIntron(
    const EnsPIntronsupportingevidence ise);

double ensIntronsupportingevidenceGetScore(
    const EnsPIntronsupportingevidence ise);

EnsEIntronsupportingevidenceType ensIntronsupportingevidenceGetType(
    const EnsPIntronsupportingevidence ise);

AjBool ensIntronsupportingevidenceSetFeature(
    EnsPIntronsupportingevidence ise,
    EnsPFeature feature);

AjBool ensIntronsupportingevidenceTrace(
    const EnsPIntronsupportingevidence ise,
    ajuint level);

size_t ensIntronsupportingevidenceCalculateMemsize(
    const EnsPIntronsupportingevidence ise);

AjBool ensIntronsupportingevidenceFetchExons(
    const EnsPIntronsupportingevidence ise,
    EnsPTranscript transcript,
    EnsPExon *Previous,
    EnsPExon *Pnext);

AjBool ensIntronsupportingevidenceMatch(
    const EnsPIntronsupportingevidence ise1,
    const EnsPIntronsupportingevidence ise2);

EnsPIntronsupportingevidence ensIntronsupportingevidenceTransfer(
    EnsPIntronsupportingevidence ise,
    EnsPSlice slice);

EnsPIntronsupportingevidence ensIntronsupportingevidenceTransform(
    EnsPIntronsupportingevidence ise,
    const AjPStr csname,
    const AjPStr csversion);

AjBool ensIntronsupportingevidenceHasTranscripts(
    const EnsPIntronsupportingevidence ise,
    AjBool *Presult);

EnsEIntronsupportingevidenceType ensIntronsupportingevidenceTypeFromStr(
    const AjPStr type);

const char* ensIntronsupportingevidenceTypeToChar(
    EnsEIntronsupportingevidenceType iset);

/* Ensembl Intron Supporting Evidence Adaptor */

EnsPIntronsupportingevidenceadaptor ensRegistryGetIntronsupportingevidenceadaptor(
    EnsPDatabaseadaptor dba);

EnsPIntronsupportingevidenceadaptor ensIntronsupportingevidenceadaptorNew(
    EnsPDatabaseadaptor dba);

void ensIntronsupportingevidenceadaptorDel(
    EnsPIntronsupportingevidenceadaptor *Pisea);

EnsPBaseadaptor ensIntronsupportingevidenceadaptorGetBaseadaptor(
    EnsPIntronsupportingevidenceadaptor isea);

EnsPDatabaseadaptor ensIntronsupportingevidenceadaptorGetDatabaseadaptor(
    EnsPIntronsupportingevidenceadaptor isea);

EnsPFeatureadaptor ensIntronsupportingevidenceadaptorGetFeatureadaptor(
    EnsPIntronsupportingevidenceadaptor isea);

AjBool ensIntronsupportingevidenceadaptorFetchAllbyTranscript(
    EnsPIntronsupportingevidenceadaptor isea,
    const EnsPTranscript transcript,
    AjPList ises);

AjBool ensIntronsupportingevidenceadaptorFetchByIdentifier(
    EnsPIntronsupportingevidenceadaptor isea,
    ajuint identifier,
    EnsPIntronsupportingevidence *Pise);

AjBool ensIntronsupportingevidenceadaptorRetrieveExonidentifiers(
    EnsPIntronsupportingevidenceadaptor isea,
    const EnsPIntronsupportingevidence ise,
    const EnsPTranscript transcript,
    ajuint *Previous,
    ajuint *Pnext);

AjBool ensIntronsupportingevidenceadaptorRetrieveAllTranscriptidentifiers(
    EnsPIntronsupportingevidenceadaptor isea,
    const EnsPIntronsupportingevidence ise,
    AjPList transcriptids);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSINTRON_H */

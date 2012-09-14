/* @include ensgene ***********************************************************
**
** Ensembl Gene functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.30 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/04/12 20:34:16 $ by $Author: mks $
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

#ifndef ENSGENE_H
#define ENSGENE_H

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

/* Ensembl Gene */

EnsPGene ensGeneNewCpy(const EnsPGene gene);

EnsPGene ensGeneNewIni(EnsPGeneadaptor ga,
                       ajuint identifier,
                       EnsPFeature feature,
                       EnsPDatabaseentry displaydbe,
                       AjPStr description,
                       AjPStr source,
                       AjPStr biotype,
                       EnsEGeneStatus status,
                       AjBool current,
                       ajuint cantrcid,
                       AjPStr canann,
                       AjPStr stableid,
                       ajuint version,
                       AjPStr cdate,
                       AjPStr mdate,
                       AjPList transcripts);

EnsPGene ensGeneNewRef(EnsPGene gene);

void ensGeneDel(EnsPGene *Pgene);

EnsPGeneadaptor ensGeneGetAdaptor(const EnsPGene gene);

AjPStr ensGeneGetBiotype(const EnsPGene gene);

AjPStr ensGeneGetCanonicalannotation(const EnsPGene gene);

ajuint ensGeneGetCanonicaltranscriptidentifier(const EnsPGene gene);

AjBool ensGeneGetCurrent(const EnsPGene gene);

AjPStr ensGeneGetDateCreation(const EnsPGene gene);

AjPStr ensGeneGetDateModification(const EnsPGene gene);

AjPStr ensGeneGetDescription(const EnsPGene gene);

EnsPDatabaseentry ensGeneGetDisplayreference(const EnsPGene gene);

EnsPFeature ensGeneGetFeature(const EnsPGene gene);

ajuint ensGeneGetIdentifier(const EnsPGene gene);

AjPStr ensGeneGetSource(const EnsPGene gene);

AjPStr ensGeneGetStableidentifier(const EnsPGene gene);

EnsEGeneStatus ensGeneGetStatus(const EnsPGene gene);

ajuint ensGeneGetVersion(const EnsPGene gene);

const AjPList ensGeneLoadAttributes(EnsPGene gene);

const AjPList ensGeneLoadDatabaseentries(EnsPGene gene);

const AjPList ensGeneLoadTranscripts(EnsPGene gene);

AjBool ensGeneSetAdaptor(EnsPGene gene, EnsPGeneadaptor ga);

AjBool ensGeneSetBiotype(EnsPGene gene, AjPStr biotype);

AjBool ensGeneSetCanonicalannotation(EnsPGene gene, AjPStr canann);

AjBool ensGeneSetCanonicaltranscriptidentifier(EnsPGene gene, ajuint cantrcid);

AjBool ensGeneSetCurrent(EnsPGene gene, AjBool current);

AjBool ensGeneSetDateCreation(EnsPGene gene, AjPStr cdate);

AjBool ensGeneSetDateModification(EnsPGene gene, AjPStr mdate);

AjBool ensGeneSetDescription(EnsPGene gene, AjPStr description);

AjBool ensGeneSetDisplayreference(EnsPGene gene, EnsPDatabaseentry dbe);

AjBool ensGeneSetFeature(EnsPGene gene, EnsPFeature feature);

AjBool ensGeneSetIdentifier(EnsPGene gene, ajuint identifier);

AjBool ensGeneSetSource(EnsPGene gene, AjPStr source);

AjBool ensGeneSetStableidentifier(EnsPGene gene, AjPStr stableid);

AjBool ensGeneSetStatus(EnsPGene gene, EnsEGeneStatus status);

AjBool ensGeneSetVersion(EnsPGene gene, ajuint version);

AjBool ensGeneAddAttribute(EnsPGene gene, EnsPAttribute attribute);

AjBool ensGeneAddDatabaseentry(EnsPGene gene, EnsPDatabaseentry dbe);

AjBool ensGeneAddTranscript(EnsPGene gene, EnsPTranscript transcript);

AjBool ensGeneTrace(const EnsPGene gene, ajuint level);

size_t ensGeneCalculateMemsize(const EnsPGene gene);

AjBool ensGeneCalculateCoordinates(EnsPGene gene);

AjBool ensGeneFetchAllAttributes(EnsPGene gene,
                                 const AjPStr code,
                                 AjPList attributes);

AjBool ensGeneFetchAllDatabaseentries(EnsPGene gene,
                                      const AjPStr name,
                                      EnsEExternaldatabaseType type,
                                      AjPList dbes);

AjBool ensGeneFetchAllExons(EnsPGene gene, AjPList exons);

AjBool ensGeneFetchCanonicaltranscript(EnsPGene gene,
                                       EnsPTranscript *Ptranscript);

AjBool ensGeneFetchDisplayidentifier(const EnsPGene gene,
                                     AjPStr *Pidentifier);

AjBool ensGeneSimilarity(EnsPGene gene1, EnsPGene gene2);

EnsPGene ensGeneTransfer(EnsPGene gene, EnsPSlice slice);

EnsPGene ensGeneTransform(EnsPGene gene,
                          const AjPStr csname,
                          const AjPStr csversion);

AjBool ensGeneCheckReference(const EnsPGene gene, AjBool *Presult);

EnsEGeneStatus ensGeneStatusFromStr(const AjPStr status);

const char *ensGeneStatusToChar(EnsEGeneStatus status);

/* AJAX List of Ensembl Gene objects */

AjBool ensListGeneSortEndAscending(AjPList genes);

AjBool ensListGeneSortEndDescending(AjPList genes);

AjBool ensListGeneSortIdentifierAscending(AjPList genes);

AjBool ensListGeneSortStartAscending(AjPList genes);

AjBool ensListGeneSortStartDescending(AjPList genes);

AjBool ensSequenceAddFeatureGene(AjPSeq seq,
                                 EnsPGene gene,
                                 AjPFeature *Pfeature);

/* Ensembl Gene Adaptor */

EnsPGeneadaptor ensRegistryGetGeneadaptor(
    EnsPDatabaseadaptor dba);

EnsPGeneadaptor ensGeneadaptorNew(
    EnsPDatabaseadaptor dba);

void ensGeneadaptorDel(EnsPGeneadaptor *Pga);

EnsPDatabaseadaptor ensGeneadaptorGetDatabaseadaptor(const EnsPGeneadaptor ga);

EnsPFeatureadaptor ensGeneadaptorGetFeatureadaptor(const EnsPGeneadaptor ga);

AjBool ensGeneadaptorFetchAll(
    EnsPGeneadaptor ga,
    AjPList genes);

AjBool ensGeneadaptorFetchAllbyBiotype(
    EnsPGeneadaptor ga,
    const AjPStr biotype,
    AjPList genes);

AjBool ensGeneadaptorFetchAllbySlice(
    EnsPGeneadaptor ga,
    EnsPSlice slice,
    const AjPStr anname,
    const AjPStr source,
    const AjPStr biotype,
    AjBool loadtranscripts,
    AjPList genes);

AjBool ensGeneadaptorFetchAllbyStableidentifier(
    EnsPGeneadaptor ga,
    const AjPStr stableid,
    AjPList genes);

AjBool ensGeneadaptorFetchByDisplaylabel(
    EnsPGeneadaptor ga,
    const AjPStr label,
    EnsPGene *Pgene);

AjBool ensGeneadaptorFetchByExonidentifier(
    EnsPGeneadaptor ga,
    ajuint identifier,
    EnsPGene *Pgene);

AjBool ensGeneadaptorFetchByExonstableidentifier(
    EnsPGeneadaptor ga,
    const AjPStr stableid,
    EnsPGene *Pgene);

AjBool ensGeneadaptorFetchByIdentifier(
    EnsPGeneadaptor ga,
    ajuint identifier,
    EnsPGene *Pgene);

AjBool ensGeneadaptorFetchByStableidentifier(
    EnsPGeneadaptor ga,
    const AjPStr stableid,
    ajuint version,
    EnsPGene *Pgene);

AjBool ensGeneadaptorFetchByTranscriptidentifier(
    EnsPGeneadaptor ga,
    ajuint identifier,
    EnsPGene *Pgene);

AjBool ensGeneadaptorFetchByTranscriptstableidentifier(
    EnsPGeneadaptor ga,
    const AjPStr stableid,
    EnsPGene *Pgene);

AjBool ensGeneadaptorFetchByTranslationstableidentifier(
    EnsPGeneadaptor ga,
    const AjPStr stableid,
    EnsPGene *Pgene);

AjBool ensGeneadaptorRetrieveAllIdentifiers(
    EnsPGeneadaptor ga,
    AjPList identifiers);

AjBool ensGeneadaptorRetrieveAllStableidentifiers(
    EnsPGeneadaptor ga,
    AjPList identifiers);

AjBool ensGeneadaptorCheckReference(
    EnsPGeneadaptor ga,
    const EnsPGene gene,
    AjBool *Presult);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSGENE_H */

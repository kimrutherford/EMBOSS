/* @include enstranslation ****************************************************
**
** Ensembl Translation functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.28 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/02/04 10:30:24 $ by $Author: mks $
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

#ifndef ENSTRANSLATION_H
#define ENSTRANSLATION_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ensdata.h"
#include "ensprotein.h"

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

void ensTranslationInit(void);

void ensTranslationExit(void);

const AjPTrn ensTranslationCacheGet(ajint codontable);

/* Ensembl Translation */

EnsPTranslation ensTranslationNewCpy(const EnsPTranslation translation);

EnsPTranslation ensTranslationNewIni(EnsPTranslationadaptor tla,
                                     ajuint identifier,
                                     EnsPExon startexon,
                                     EnsPExon endexon,
                                     ajuint start,
                                     ajuint end,
                                     AjPStr sequence,
                                     AjPStr stableid,
                                     ajuint version,
                                     AjPStr cdate,
                                     AjPStr mdate);

EnsPTranslation ensTranslationNewRef(EnsPTranslation translation);

void ensTranslationDel(EnsPTranslation *Ptranslation);

EnsPTranslationadaptor ensTranslationGetAdaptor(
    const EnsPTranslation translation);

AjPStr ensTranslationGetDateCreation(const EnsPTranslation translation);

AjPStr ensTranslationGetDateModification(const EnsPTranslation translation);

ajuint ensTranslationGetEnd(const EnsPTranslation translation);

EnsPExon ensTranslationGetEndexon(const EnsPTranslation translation);

ajuint ensTranslationGetIdentifier(const EnsPTranslation translation);

AjPStr ensTranslationGetStableidentifier(const EnsPTranslation translation);

ajuint ensTranslationGetStart(const EnsPTranslation translation);

EnsPExon ensTranslationGetStartexon(const EnsPTranslation translation);

ajuint ensTranslationGetVersion(const EnsPTranslation translation);

const AjPList ensTranslationLoadAttributes(EnsPTranslation translation);

const AjPList ensTranslationLoadDatabaseentries(EnsPTranslation translation);

const AjPList ensTranslationLoadProteinfeatures(EnsPTranslation translation);

AjBool ensTranslationSetAdaptor(EnsPTranslation translation,
                                EnsPTranslationadaptor tla);

AjBool ensTranslationSetIdentifier(EnsPTranslation translation,
                                   ajuint identifier);

AjBool ensTranslationSetStartexon(EnsPTranslation translation,
                                  EnsPExon exon);

AjBool ensTranslationSetEndexon(EnsPTranslation translation,
                                EnsPExon exon);

AjBool ensTranslationSetStart(EnsPTranslation translation,
                              ajuint start);

AjBool ensTranslationSetEnd(EnsPTranslation translation,
                            ajuint end);

AjBool ensTranslationSetStableidentifier(EnsPTranslation translation,
                                         AjPStr stableid);

AjBool ensTranslationSetVersion(EnsPTranslation translation,
                                ajuint version);

AjBool ensTranslationSetDateCreation(EnsPTranslation translation,
                                     AjPStr cdate);

AjBool ensTranslationSetDateModification(EnsPTranslation translation,
                                         AjPStr mdate);

AjBool ensTranslationAddAttribute(EnsPTranslation translation,
                                  EnsPAttribute attribute);

AjBool ensTranslationAddDatabaseentry(EnsPTranslation translation,
                                      EnsPDatabaseentry dbe);

AjBool ensTranslationAddProteinfeature(EnsPTranslation translation,
                                       EnsPProteinfeature pf);

size_t ensTranslationCalculateMemsize(const EnsPTranslation translation);

ajint ensTranslationCalculateSliceEnd(EnsPTranslation translation);

ajint ensTranslationCalculateSliceStart(EnsPTranslation translation);

ajuint ensTranslationCalculateTranscriptEnd(EnsPTranslation translation);

ajuint ensTranslationCalculateTranscriptStart(EnsPTranslation translation);

AjBool ensTranslationFetchAllAttributes(EnsPTranslation translation,
                                        const AjPStr code,
                                        AjPList attributes);

AjBool ensTranslationFetchAllDatabaseentries(EnsPTranslation translation,
                                             const AjPStr name,
                                             EnsEExternaldatabaseType type,
                                             AjPList dbes);

AjBool ensTranslationFetchAllDomains(EnsPTranslation translation,
                                     AjPList pfs);

AjBool ensTranslationFetchAllProteinfeatures(EnsPTranslation translation,
                                             const AjPStr name,
                                             AjPList pfs);

AjBool ensTranslationFetchAllSequenceedits(EnsPTranslation translation,
                                           AjPList ses);

AjBool ensTranslationFetchDisplayidentifier(const EnsPTranslation translation,
                                            AjPStr *Pidentifier);

AjBool ensTranslationTrace(const EnsPTranslation translation, ajuint level);

AjBool ensTranslationFetchSequenceSeq(EnsPTranslation translation,
                                      AjPSeq *Psequence);

AjBool ensTranslationFetchSequenceStr(EnsPTranslation translation,
                                      AjPStr *Psequence);

/* Ensembl Translation Adaptor */

EnsPTranslationadaptor ensRegistryGetTranslationadaptor(
    EnsPDatabaseadaptor dba);

EnsPTranslationadaptor ensTranslationadaptorNew(
    EnsPDatabaseadaptor dba);

void ensTranslationadaptorDel(EnsPTranslationadaptor *Ptla);

EnsPDatabaseadaptor ensTranslationadaptorGetDatabaseadaptor(
    EnsPTranslationadaptor tla);

AjBool ensTranslationadaptorFetchAllbyExternalname(
    EnsPTranslationadaptor tla,
    const AjPStr name,
    const AjPStr dbname,
    AjPList translations);

AjBool ensTranslationadaptorFetchAllbyTranscript(EnsPTranslationadaptor tla,
                                                 EnsPTranscript transcript);

AjBool ensTranslationadaptorFetchByIdentifier(EnsPTranslationadaptor tla,
                                              ajuint identifier,
                                              EnsPTranslation *Ptranslation);

AjBool ensTranslationadaptorFetchByStableidentifier(
    EnsPTranslationadaptor tla,
    const AjPStr stableid,
    ajuint version,
    EnsPTranslation *Ptranslation);

AjBool ensTranslationadaptorFetchByTranscript(EnsPTranslationadaptor tla,
                                              EnsPTranscript transcript);

AjBool ensTranslationadaptorFetchAllbyTranscriptsList(
    EnsPTranslationadaptor tla,
    AjPList transcripts);

AjBool ensTranslationadaptorFetchAllbyTranscriptsTable(
    EnsPTranslationadaptor tla,
    AjPTable transcripts);

AjBool ensTranslationadaptorRetrieveAllIdentifiers(
    EnsPTranslationadaptor tla,
    AjPList identifiers);

AjBool ensTranslationadaptorRetrieveAllStableidentifiers(
    EnsPTranslationadaptor tla,
    AjPList identifiers);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSTRANSLATION_H */

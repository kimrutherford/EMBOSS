/* @include enstranscript *****************************************************
**
** Ensembl Transcript functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.38 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2013/02/17 13:09:28 $ by $Author: mks $
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

#ifndef ENSTRANSCRIPT_H
#define ENSTRANSCRIPT_H

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

/* Ensembl Transcript */

EnsPTranscript ensTranscriptNewCpy(const EnsPTranscript transcript);

EnsPTranscript ensTranscriptNewIni(EnsPTranscriptadaptor tca,
                                   ajuint identifier,
                                   EnsPFeature feature,
                                   EnsPDatabaseentry displaydbe,
                                   AjPStr description,
                                   AjPStr biotype,
                                   EnsETranscriptStatus status,
                                   AjBool current,
                                   AjPStr stableid,
                                   ajuint version,
                                   AjPStr cdate,
                                   AjPStr mdate,
                                   AjPList exons);

EnsPTranscript ensTranscriptNewRef(EnsPTranscript transcript);

AjBool ensTranscriptClearIntronsupportingevidences(EnsPTranscript transcript);

void ensTranscriptDel(EnsPTranscript *Ptranscript);

EnsPTranscriptadaptor ensTranscriptGetAdaptor(const EnsPTranscript transcript);

AjPStr ensTranscriptGetBiotype(const EnsPTranscript transcript);

const AjBool* ensTranscriptGetCanonical(const EnsPTranscript transcript);

AjBool ensTranscriptGetCurrent(const EnsPTranscript transcript);

AjPStr ensTranscriptGetDateCreation(const EnsPTranscript transcript);

AjPStr ensTranscriptGetDateModification(const EnsPTranscript transcript);

AjPStr ensTranscriptGetDescription(const EnsPTranscript transcript);

EnsPDatabaseentry ensTranscriptGetDisplayreference(
    const EnsPTranscript transcript);

EnsPFeature ensTranscriptGetFeature(const EnsPTranscript transcript);

ajuint ensTranscriptGetGeneidentifier(const EnsPTranscript transcript);

ajuint ensTranscriptGetIdentifier(const EnsPTranscript transcript);

AjBool ensTranscriptGetSequenceedits(const EnsPTranscript transcript);

AjPStr ensTranscriptGetStableidentifier(const EnsPTranscript transcript);

EnsETranscriptStatus ensTranscriptGetStatus(const EnsPTranscript transcript);

ajuint ensTranscriptGetVersion(const EnsPTranscript transcript);

const AjPList ensTranscriptLoadAlternativetranslations(
    EnsPTranscript transcript);

const AjPList ensTranscriptLoadAttributes(
    EnsPTranscript transcript);

const AjBool* ensTranscriptLoadCanonical(
    EnsPTranscript transcript);

const AjPList ensTranscriptLoadDatabaseentries(
    EnsPTranscript transcript);

const AjPList ensTranscriptLoadExons(
    EnsPTranscript transcript);

const AjPList ensTranscriptLoadIntronsupportingevidences(
    EnsPTranscript transcript);

const AjPList ensTranscriptLoadSupportingfeatures(
    EnsPTranscript transcript);

EnsPTranslation ensTranscriptLoadTranslation(
    EnsPTranscript transcript);

AjBool ensTranscriptSetAdaptor(EnsPTranscript transcript,
                               EnsPTranscriptadaptor tca);

AjBool ensTranscriptSetBiotype(EnsPTranscript transcript,
                               AjPStr biotype);

AjBool ensTranscriptSetCanonical(EnsPTranscript transcript,
                                 AjBool canonical);

AjBool ensTranscriptSetCurrent(EnsPTranscript transcript,
                               AjBool current);

AjBool ensTranscriptSetDateCreation(EnsPTranscript transcript,
                                    AjPStr cdate);

AjBool ensTranscriptSetDateModification(EnsPTranscript transcript,
                                        AjPStr mdate);

AjBool ensTranscriptSetDescription(EnsPTranscript transcript,
                                   AjPStr description);

AjBool ensTranscriptSetDisplayreference(EnsPTranscript transcript,
                                        EnsPDatabaseentry displaydbe);

AjBool ensTranscriptSetFeature(EnsPTranscript transcript,
                               EnsPFeature feature);

AjBool ensTranscriptSetGeneidentifier(EnsPTranscript transcript,
                                      ajuint geneid);

AjBool ensTranscriptSetIdentifier(EnsPTranscript transcript,
                                  ajuint identifier);

AjBool ensTranscriptSetSequenceedits(EnsPTranscript transcript,
                                     AjBool enablese);

AjBool ensTranscriptSetStableidentifier(EnsPTranscript transcript,
                                        AjPStr stableid);

AjBool ensTranscriptSetStatus(EnsPTranscript transcript,
                              EnsETranscriptStatus status);

AjBool ensTranscriptSetTranslation(EnsPTranscript transcript,
                                   EnsPTranslation translation);

AjBool ensTranscriptSetVersion(EnsPTranscript transcript,
                               ajuint version);

AjBool ensTranscriptAddAlternativetranslation(
    EnsPTranscript transcript,
    EnsPTranslation translation);

AjBool ensTranscriptAddAttribute(
    EnsPTranscript transcript,
    EnsPAttribute attribute);

AjBool ensTranscriptAddDatabaseentry(
    EnsPTranscript transcript,
    EnsPDatabaseentry dbe);

AjBool ensTranscriptAddExon(
    EnsPTranscript transcript,
    EnsPExon exon,
    ajint rank);

AjBool ensTranscriptAddIntronsupportingevidence(
    EnsPTranscript transcript,
    EnsPIntronsupportingevidence ise);

AjBool ensTranscriptTrace(const EnsPTranscript transcript, ajuint level);

AjBool ensTranscriptCalculateCoordinates(EnsPTranscript transcript);

ajuint ensTranscriptCalculateLength(EnsPTranscript transcript);

size_t ensTranscriptCalculateMemsize(const EnsPTranscript transcript);

ajint ensTranscriptCalculatePhaseStart(EnsPTranscript transcript);

ajint ensTranscriptCalculateSliceCodingEnd(
    EnsPTranscript transcript,
    EnsPTranslation translation);

ajint ensTranscriptCalculateSliceCodingStart(
    EnsPTranscript transcript,
    EnsPTranslation translation);

ajuint ensTranscriptCalculateTranscriptCodingEnd(
    EnsPTranscript transcript,
    EnsPTranslation translation);

ajuint ensTranscriptCalculateTranscriptCodingStart(
    EnsPTranscript transcript,
    EnsPTranslation translation);

EnsETranscriptStatus ensTranscriptStatusFromStr(const AjPStr status);

const char *ensTranscriptStatusToChar(EnsETranscriptStatus status);

AjBool ensTranscriptSimilarity(EnsPTranscript transcript1,
                               EnsPTranscript transcript2);

EnsPTranscript ensTranscriptTransfer(EnsPTranscript transcript,
                                     EnsPSlice slice);

EnsPTranscript ensTranscriptTransform(EnsPTranscript transcript,
                                      const AjPStr csname,
                                      const AjPStr csversion);

AjBool ensTranscriptFetchAllAttributes(EnsPTranscript transcript,
                                       const AjPStr code,
                                       AjPList attributes);

AjBool ensTranscriptFetchAllConstitutiveexons(EnsPTranscript transcript,
                                              AjPList exons);

AjBool ensTranscriptFetchAllDatabaseentries(EnsPTranscript transcript,
                                            const AjPStr name,
                                            EnsEExternaldatabaseType type,
                                            AjPList dbes);

AjBool ensTranscriptFetchAllIntrons(EnsPTranscript transcript,
                                    AjPList introns);

AjBool ensTranscriptFetchAllSequenceedits(EnsPTranscript transcript,
                                          AjPList ses);

AjBool ensTranscriptFetchDisplayidentifier(const EnsPTranscript transcript,
                                           AjPStr *Pidentifier);

AjBool ensTranscriptFetchFeatureUtrfive(EnsPTranscript transcript,
                                        EnsPTranslation translation,
                                        EnsPFeature *Pfeature);

AjBool ensTranscriptFetchFeatureUtrthree(EnsPTranscript transcript,
                                         EnsPTranslation translation,
                                         EnsPFeature *Pfeature);

AjBool ensTranscriptFetchGene(const EnsPTranscript transcript,
                              EnsPGene *Pgene);

AjBool ensTranscriptFetchSequenceCodingStr(EnsPTranscript transcript,
                                           EnsPTranslation translation,
                                           AjPStr *Psequence);

AjBool ensTranscriptFetchSequenceTranscriptSeq(EnsPTranscript transcript,
                                               AjPSeq *Psequence);

AjBool ensTranscriptFetchSequenceTranscriptStr(EnsPTranscript transcript,
                                               AjPStr *Psequence);

AjBool ensTranscriptFetchSequenceTranslationSeq(EnsPTranscript transcript,
                                                EnsPTranslation translation,
                                                AjPSeq *Psequence);

AjBool ensTranscriptFetchSequenceTranslationStr(EnsPTranscript transcript,
                                                EnsPTranslation translation,
                                                AjPStr *Psequence);

AjBool ensTranscriptFetchSequenceUtrfiveStr(EnsPTranscript transcript,
                                            EnsPTranslation translation,
                                            AjPStr *Psequence);

AjBool ensTranscriptFetchSequenceUtrthreeStr(EnsPTranscript transcript,
                                             EnsPTranslation translation,
                                             AjPStr *Psequence);

AjBool ensTranscriptMapperInit(EnsPTranscript transcript, AjBool force);

AjBool ensTranscriptMapperCodingToslice(EnsPTranscript transcript,
                                        EnsPTranslation translation,
                                        ajint start,
                                        ajint end,
                                        AjPList mrs);

AjBool ensTranscriptMapperSliceTocoding(EnsPTranscript transcript,
                                        EnsPTranslation translation,
                                        ajint start,
                                        ajint end,
                                        ajint strand,
                                        AjPList mrs);

AjBool ensTranscriptMapperSliceTotranscript(EnsPTranscript transcript,
                                            ajint start,
                                            ajint end,
                                            ajint strand,
                                            AjPList mrs);

AjBool ensTranscriptMapperSliceTotranslation(EnsPTranscript transcript,
                                             EnsPTranslation translation,
                                             ajint start,
                                             ajint end,
                                             ajint strand,
                                             AjPList mrs);

AjBool ensTranscriptMapperTranscriptToslice(EnsPTranscript transcript,
                                            ajuint start,
                                            ajuint end,
                                            AjPList mrs);

AjBool ensTranscriptMapperTranslationToslice(EnsPTranscript transcript,
                                             EnsPTranslation translation,
                                             ajuint start,
                                             ajuint end,
                                             AjPList mrs);

/* AJAX LIst of Ensembl Transcript objects */

AjBool ensListTranscriptSortEndAscending(AjPList transcripts);

AjBool ensListTranscriptSortEndDescending(AjPList transcripts);

AjBool ensListTranscriptSortIdentifierAscending(AjPList transcripts);

AjBool ensListTranscriptSortStartAscending(AjPList transcripts);

AjBool ensListTranscriptSortStartDescending(AjPList transcripts);

AjBool ensSequenceAddFeatureTranscript(AjPSeq seq,
                                       EnsPTranscript transcript,
                                       AjPFeature *Pfeature);

/* Ensembl Transcript Adaptor */

EnsPTranscriptadaptor ensRegistryGetTranscriptadaptor(
    EnsPDatabaseadaptor dba);

EnsPTranscriptadaptor ensTranscriptadaptorNew(
    EnsPDatabaseadaptor dba);

void ensTranscriptadaptorDel(EnsPTranscriptadaptor *Ptca);

EnsPBaseadaptor ensTranscriptadaptorGetBaseadaptor(
    EnsPTranscriptadaptor tca);

EnsPDatabaseadaptor ensTranscriptadaptorGetDatabaseadaptor(
    EnsPTranscriptadaptor tca);

EnsPFeatureadaptor ensTranscriptadaptorGetFeatureadaptor(
    EnsPTranscriptadaptor tca);

AjBool ensTranscriptadaptorFetchAll(
    EnsPTranscriptadaptor tca,
    AjPList transcripts);

AjBool ensTranscriptadaptorFetchAllbyBiotype(
    EnsPTranscriptadaptor tca,
    const AjPStr biotype,
    AjPList transcripts);

AjBool ensTranscriptadaptorFetchAllbyExonidentifier(
    EnsPTranscriptadaptor tca,
    ajuint identifier,
    AjPList transcripts);

AjBool ensTranscriptadaptorFetchAllbyExonstableidentifier(
    EnsPTranscriptadaptor tca,
    const AjPStr stableid,
    AjPList transcripts);

AjBool ensTranscriptadaptorFetchAllbyExternaldatabasename(
    EnsPTranscriptadaptor tca,
    const AjPStr dbname,
    AjPList transcripts);

AjBool ensTranscriptadaptorFetchAllbyExternalname(
    EnsPTranscriptadaptor tca,
    const AjPStr name,
    const AjPStr dbname,
    AjBool override,
    AjPList transcripts);

AjBool ensTranscriptadaptorFetchAllbyGene(
    EnsPTranscriptadaptor tca,
    EnsPGene gene,
    AjPList transcripts);

AjBool ensTranscriptadaptorFetchAllbyIdentifiers(
    EnsPTranscriptadaptor tca,
    AjPTable transcripts);

AjBool ensTranscriptadaptorFetchAllbySlice(
    EnsPTranscriptadaptor tca,
    EnsPSlice slice,
    const AjPStr anname,
    const AjPStr constraint,
    AjBool loadexons,
    AjPList transcripts);

AjBool ensTranscriptadaptorFetchAllbyStableidentifier(
    EnsPTranscriptadaptor tca,
    const AjPStr stableid,
    AjPList transcripts);

AjBool ensTranscriptadaptorFetchByDisplaylabel(
    EnsPTranscriptadaptor tca,
    const AjPStr label,
    EnsPTranscript *Ptranscript);

AjBool ensTranscriptadaptorFetchByIdentifier(
    EnsPTranscriptadaptor tca,
    ajuint identifier,
    EnsPTranscript *Ptranscript);

AjBool ensTranscriptadaptorFetchByStableidentifier(
    EnsPTranscriptadaptor tca,
    const AjPStr stableid,
    ajuint version,
    EnsPTranscript *Ptranscript);

AjBool ensTranscriptadaptorFetchByTranslationidentifier(
    EnsPTranscriptadaptor tca,
    ajuint identifier,
    EnsPTranscript *Ptranscript);

AjBool ensTranscriptadaptorFetchByTranslationstableidentifier(
    EnsPTranscriptadaptor tca,
    const AjPStr stableid,
    EnsPTranscript *Ptranscript);

AjBool ensTranscriptadaptorRetrieveAllIdentifiers(
    EnsPTranscriptadaptor tca,
    AjPList identifiers);

AjBool ensTranscriptadaptorRetrieveAllStableidentifiers(
    EnsPTranscriptadaptor tca,
    AjPList stableids);

AjBool ensTranscriptadaptorRetrieveCanonical(
    EnsPTranscriptadaptor tca,
    EnsPTranscript transcript,
    AjBool *Pcanonical);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSTRANSCRIPT_H */

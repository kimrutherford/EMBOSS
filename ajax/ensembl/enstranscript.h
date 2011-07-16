
#ifndef ENSTRANSCRIPT_H
#define ENSTRANSCRIPT_H

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ensdata.h"

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

void ensTranscriptDel(EnsPTranscript* Ptranscript);

EnsPTranscriptadaptor ensTranscriptGetAdaptor(const EnsPTranscript transcript);

AjPStr ensTranscriptGetBiotype(const EnsPTranscript transcript);

AjBool ensTranscriptGetCanonical(const EnsPTranscript transcript);

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

const AjPList ensTranscriptLoadAttributes(EnsPTranscript transcript);

const AjPList ensTranscriptLoadDatabaseentries(EnsPTranscript transcript);

const AjPList ensTranscriptLoadExons(EnsPTranscript transcript);

const AjPList ensTranscriptLoadSupportingfeatures(EnsPTranscript transcript);

EnsPTranslation ensTranscriptLoadTranslation(EnsPTranscript transcript);

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

AjBool ensTranscriptSetFeature(EnsPTranscript transcript, EnsPFeature feature);

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

AjBool ensTranscriptAddAlternativetranslation(EnsPTranscript transcript,
                                              EnsPTranslation translation);

AjBool ensTranscriptAddAttribute(EnsPTranscript transcript,
                                 EnsPAttribute attribute);

AjBool ensTranscriptAddDatabaseentry(EnsPTranscript transcript,
                                     EnsPDatabaseentry dbe);

AjBool ensTranscriptAddExon(EnsPTranscript transcript,
                            EnsPExon exon,
                            ajint rank);

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

const char* ensTranscriptStatusToChar(EnsETranscriptStatus status);

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
                                           AjPStr* Pidentifier);

AjBool ensTranscriptFetchSequenceCodingStr(EnsPTranscript transcript,
                                           EnsPTranslation translation,
                                           AjPStr* Psequence);

AjBool ensTranscriptFetchSequenceTranscriptSeq(EnsPTranscript transcript,
                                               AjPSeq* Psequence);

AjBool ensTranscriptFetchSequenceTranscriptStr(EnsPTranscript transcript,
                                               AjPStr* Psequence);

AjBool ensTranscriptFetchSequenceTranslationSeq(EnsPTranscript transcript,
                                                EnsPTranslation translation,
                                                AjPSeq* Psequence);

AjBool ensTranscriptFetchSequenceTranslationStr(EnsPTranscript transcript,
                                                EnsPTranslation translation,
                                                AjPStr* Psequence);

AjBool ensTranscriptMapperInit(EnsPTranscript transcript, AjBool force);

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

AjBool ensListTranscriptSortStartAscending(AjPList transcripts);

AjBool ensListTranscriptSortStartDescending(AjPList transcripts);

AjBool ensTableTranscriptClear(AjPTable table);

AjBool ensTableTranscriptDelete(AjPTable* Ptable);

/* Ensembl Supporting Feature Adaptor */

AjBool ensSupportingfeatureadaptorFetchAllbyTranscript(
    EnsPDatabaseadaptor dba,
    EnsPTranscript transcript,
    AjPList bafs);

/* Ensembl Transcript Adaptor */

EnsPTranscriptadaptor ensRegistryGetTranscriptadaptor(
    EnsPDatabaseadaptor dba);

EnsPTranscriptadaptor ensTranscriptadaptorNew(
    EnsPDatabaseadaptor dba);

void ensTranscriptadaptorDel(EnsPTranscriptadaptor* Ptca);

EnsPDatabaseadaptor ensTranscriptadaptorGetDatabaseadaptor(
    EnsPTranscriptadaptor tca);

EnsPFeatureadaptor ensTranscriptadaptorGetFeatureadaptor(
    EnsPTranscriptadaptor tca);

AjBool ensTranscriptadaptorFetchAll(
    EnsPTranscriptadaptor tca,
    AjPList transcripts);

AjBool ensTranscriptadaptorFetchAllbyGene(
    EnsPTranscriptadaptor tca,
    EnsPGene gene,
    AjPList transcripts);

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
    EnsPTranscript* Ptranscript);

AjBool ensTranscriptadaptorFetchByExonidentifier(
    EnsPTranscriptadaptor tca,
    ajuint identifier,
    EnsPTranscript* Ptranscript);

AjBool ensTranscriptadaptorFetchByExonstableidentifier(
    EnsPTranscriptadaptor tca,
    const AjPStr stableid,
    EnsPTranscript* Ptranscript);

AjBool ensTranscriptadaptorFetchByIdentifier(
    EnsPTranscriptadaptor tca,
    ajuint identifier,
    EnsPTranscript* Ptranscript);

AjBool ensTranscriptadaptorFetchByStableidentifier(
    EnsPTranscriptadaptor tca,
    const AjPStr stableid,
    ajuint version,
    EnsPTranscript* Ptranscript);

AjBool ensTranscriptadaptorFetchByTranslationidentifier(
    EnsPTranscriptadaptor tca,
    ajuint identifier,
    EnsPTranscript* Ptranscript);

AjBool ensTranscriptadaptorFetchByTranslationstableidentifier(
    EnsPTranscriptadaptor tca,
    const AjPStr stableid,
    EnsPTranscript* Ptranscript);

AjBool ensTranscriptadaptorRetrieveAllIdentifiers(
    EnsPTranscriptadaptor tca,
    AjPList identifiers);

AjBool ensTranscriptadaptorRetrieveAllStableidentifiers(
    EnsPTranscriptadaptor tca,
    AjPList identifiers);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSTRANSCRIPT_H */

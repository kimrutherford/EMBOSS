
#ifndef ENSEXON_H
#define ENSEXON_H

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ensfeature.h"

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

/* Ensembl Exon */

EnsPExon ensExonNewCpy(const EnsPExon object);

EnsPExon ensExonNewIni(EnsPExonadaptor ea,
                       ajuint identifier,
                       EnsPFeature feature,
                       ajint sphase,
                       ajint ephase,
                       AjBool current,
                       AjBool constitutive,
                       AjPStr stableid,
                       ajuint version,
                       AjPStr cdate,
                       AjPStr mdate);

EnsPExon ensExonNewRef(EnsPExon exon);

void ensExonDel(EnsPExon* Pexon);

EnsPExonadaptor ensExonGetAdaptor(const EnsPExon exon);

AjBool ensExonGetConstitutive(const EnsPExon exon);

AjBool ensExonGetCurrent(const EnsPExon exon);

AjPStr ensExonGetDateCreation(const EnsPExon exon);

AjPStr ensExonGetDateModification(const EnsPExon exon);

EnsPFeature ensExonGetFeature(const EnsPExon exon);

ajuint ensExonGetIdentifier(const EnsPExon exon);

ajint ensExonGetPhaseEnd(const EnsPExon exon);

ajint ensExonGetPhaseStart(const EnsPExon exon);

AjPStr ensExonGetStableidentifier(const EnsPExon exon);

ajuint ensExonGetVersion(const EnsPExon exon);

const AjPList ensExonLoadSupportingfeatures(EnsPExon exon);

AjBool ensExonSetAdaptor(EnsPExon exon, EnsPExonadaptor ea);

AjBool ensExonSetConstitutive(EnsPExon exon, AjBool constitutive);

AjBool ensExonSetCurrent(EnsPExon exon, AjBool current);

AjBool ensExonSetDateCreation(EnsPExon exon, AjPStr cdate);

AjBool ensExonSetDateModification(EnsPExon exon, AjPStr mdate);

AjBool ensExonSetFeature(EnsPExon exon, EnsPFeature feature);

AjBool ensExonSetIdentifier(EnsPExon exon, ajuint identifier);

AjBool ensExonSetPhaseEnd(EnsPExon exon, ajint ephase);

AjBool ensExonSetPhaseStart(EnsPExon exon, ajint sphase);

AjBool ensExonSetStableidentifier(EnsPExon exon, AjPStr stableid);

AjBool ensExonSetVersion(EnsPExon exon, ajuint version);

AjBool ensExonTrace(const EnsPExon exon, ajuint level);

ajint ensExonCalculateFrame(const EnsPExon exon);

size_t ensExonCalculateMemsize(const EnsPExon exon);

ajint ensExonCalculateSliceCodingEnd(EnsPExon exon,
                                     EnsPTranscript transcript,
                                     EnsPTranslation translation);

ajint ensExonCalculateSliceCodingStart(EnsPExon exon,
                                       EnsPTranscript transcript,
                                       EnsPTranslation translation);

ajuint ensExonCalculateTranscriptCodingEnd(EnsPExon exon,
                                           EnsPTranscript transcript,
                                           EnsPTranslation translation);

ajuint ensExonCalculateTranscriptCodingStart(EnsPExon exon,
                                             EnsPTranscript transcript,
                                             EnsPTranslation translation);

ajuint ensExonCalculateTranscriptStart(EnsPExon exon,
                                       EnsPTranscript transcript);

ajuint ensExonCalculateTranscriptEnd(EnsPExon exon,
                                     EnsPTranscript transcript);

EnsPExon ensExonTransfer(EnsPExon exon, EnsPSlice slice);

EnsPExon ensExonTransform(EnsPExon exon,
                          const AjPStr csname,
                          const AjPStr csversion);

AjBool ensExonFetchDisplayidentifier(const EnsPExon exon, AjPStr* Pidentifier);

AjBool ensExonFetchSequenceSliceSeq(EnsPExon exon, AjPSeq* Psequence);

AjBool ensExonFetchSequenceSliceStr(EnsPExon exon, AjPStr* Psequence);

AjBool ensExonFetchSequenceTranslationSeq(EnsPExon exon,
                                          EnsPTranscript transcript,
                                          EnsPTranslation translation,
                                          AjPSeq* Psequence);

AjBool ensExonFetchSequenceTranslationStr(EnsPExon exon,
                                          EnsPTranscript transcript,
                                          EnsPTranslation translation,
                                          AjPStr* Psequence);

AjBool ensExonMatch(const EnsPExon exon1, const EnsPExon exon2);

AjBool ensExonSimilarity(const EnsPExon exon1, const EnsPExon exon2);

AjBool ensListExonSortStartAscending(AjPList exons);

AjBool ensListExonSortStartDescending(AjPList exons);

/* Ensembl Exon Adaptor */

EnsPExonadaptor ensRegistryGetExonadaptor(
    EnsPDatabaseadaptor dba);

EnsPExonadaptor ensExonadaptorNew(
    EnsPDatabaseadaptor dba);

void ensExonadaptorDel(EnsPExonadaptor* Pea);

EnsPDatabaseadaptor ensExonadaptorGetDatabaseadaptor(EnsPExonadaptor ea);

EnsPFeatureadaptor ensExonadaptorGetFeatureadaptor(EnsPExonadaptor ea);

AjBool ensExonadaptorFetchAll(EnsPExonadaptor ea,
                              AjPList exons);

AjBool ensExonadaptorFetchAllbySlice(EnsPExonadaptor ea,
                                     EnsPSlice slice,
                                     const AjPStr constraint,
                                     AjPList exons);

AjBool ensExonadaptorFetchAllbyStableidentifier(EnsPExonadaptor ea,
                                                const AjPStr stableid,
                                                AjPList exons);

AjBool ensExonadaptorFetchAllbyTranscript(EnsPExonadaptor ea,
                                          const EnsPTranscript transcript,
                                          AjPList exons);

AjBool ensExonadaptorFetchByIdentifier(EnsPExonadaptor ea,
                                       ajuint identifier,
                                       EnsPExon* Pexon);

AjBool ensExonadaptorFetchByStableidentifier(EnsPExonadaptor ea,
                                             const AjPStr stableid,
                                             ajuint version,
                                             EnsPExon* Pexon);

AjBool ensExonadaptorRetrieveAllIdentifiers(EnsPExonadaptor ea,
                                            AjPList identifiers);

AjBool ensExonadaptorRetrieveAllStableidentifiers(EnsPExonadaptor ea,
                                                  AjPList identifiers);

/* Ensembl Supporting Feature Adaptor */

AjBool ensSupportingfeatureadaptorFetchAllbyExon(EnsPDatabaseadaptor dba,
                                                 EnsPExon exon,
                                                 AjPList bafs);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSEXON_H */

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef enstranslation_h
#define enstranslation_h

#include "ensdata.h"
#include "ensfeature.h"




/*
** Prototype definitions
*/

void ensTranslationInit(void);

void ensTranslationExit(void);

const AjPTrn ensTranslationCacheGetTranslation(ajint codontable);

/* Ensembl Translation */

EnsPTranslation ensTranslationNew(EnsPTranslationadaptor tla,
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

EnsPTranslation ensTranslationNewObj(const EnsPTranslation object);

EnsPTranslation ensTranslationNewRef(EnsPTranslation translation);

void ensTranslationDel(EnsPTranslation* Ptranslation);

const EnsPTranslationadaptor ensTranslationGetAdaptor(
    const EnsPTranslation translation);

ajuint ensTranslationGetIdentifier(const EnsPTranslation translation);

EnsPExon ensTranslationGetStartExon(const EnsPTranslation translation);

EnsPExon ensTranslationGetEndExon(const EnsPTranslation translation);

ajuint ensTranslationGetStart(const EnsPTranslation translation);

ajuint ensTranslationGetEnd(const EnsPTranslation translation);

AjPStr ensTranslationGetStableIdentifier(const EnsPTranslation translation);

ajuint ensTranslationGetVersion(const EnsPTranslation translation);

AjPStr ensTranslationGetCreationDate(const EnsPTranslation translation);

AjPStr ensTranslationGetModificationDate(const EnsPTranslation translation);

const AjPList ensTranslationGetAttributes(EnsPTranslation translation);

const AjPList ensTranslationGetDatabaseEntries(EnsPTranslation translation);

const AjPList ensTranslationGetProteinfeatures(EnsPTranslation translation);

ajuint ensTranslationGetTranscriptStart(EnsPTranslation translation);

ajuint ensTranslationGetTranscriptEnd(EnsPTranslation translation);

ajuint ensTranslationGetSliceStart(EnsPTranslation translation);

ajuint ensTranslationGetSliceEnd(EnsPTranslation translation);

ajulong ensTranslationGetMemsize(const EnsPTranslation translation);

AjBool ensTranslationSetAdaptor(EnsPTranslation translation,
                                EnsPTranslationadaptor tla);

AjBool ensTranslationSetIdentifier(EnsPTranslation translation,
                                   ajuint identifier);

AjBool ensTranslationSetStartExon(EnsPTranslation translation, EnsPExon exon);

AjBool ensTranslationSetEndExon(EnsPTranslation translation, EnsPExon exon);

AjBool ensTranslationSetStart(EnsPTranslation translation, ajuint start);

AjBool ensTranslationSetEnd(EnsPTranslation translation, ajuint end);

AjBool ensTranslationSetStableIdentifier(EnsPTranslation translation,
                                         AjPStr stableid);

AjBool ensTranslationSetVersion(EnsPTranslation translation, ajuint version);

AjBool ensTranslationSetCreationDate(EnsPTranslation translation, AjPStr cdate);

AjBool ensTranslationSetModificationDate(EnsPTranslation translation,
                                         AjPStr mdate);

AjBool ensTranslationAddAttribute(EnsPTranslation translation,
                                  EnsPAttribute attribute);

AjBool ensTranslationAddDatabaseentry(EnsPTranslation translation,
                                      EnsPDatabaseentry dbe);

AjBool ensTranslationAddProteinfeature(EnsPTranslation translation,
                                       EnsPProteinfeature pf);

AjBool ensTranslationFetchAllAttributes( EnsPTranslation translation,
                                         const AjPStr code,
                                         AjPList attributes);

AjBool ensTranslationFetchAllDatabaseEntries(EnsPTranslation translation,
                                             const AjPStr name,
                                             EnsEExternaldatabaseType type,
                                             AjPList dbes);

AjBool ensTranslationFetchAllProteinfeatures(EnsPTranslation translation,
                                             const AjPStr name,
                                             AjPList pfs);

AjBool ensTranslationFetchAllProteinfeaturesDomain(EnsPTranslation translation,
                                                   AjPList pfs);

AjBool ensTranslationFetchAllSequenceEdits(EnsPTranslation translation,
                                           AjPList ses);

AjBool ensTranslationFetchDisplayIdentifier(const EnsPTranslation translation,
                                            AjPStr* Pidentifier);

AjBool ensTranslationTrace(const EnsPTranslation translation, ajuint level);

AjBool ensTranslationFetchSequenceStr(EnsPTranslation translation,
                                      AjPStr *Psequence);

AjBool ensTranslationFetchSequenceSeq(EnsPTranslation translation,
                                      AjPSeq *Psequence);

/* Ensembl Translation Adaptor */

EnsPTranslationadaptor ensRegistryGetTranslationadaptor(
    EnsPDatabaseadaptor dba);

EnsPTranslationadaptor ensTranslationadaptorNew(
    EnsPDatabaseadaptor dba);

void ensTranslationadaptorDel(EnsPTranslationadaptor *Ptla);

EnsPDatabaseadaptor ensTranslationadaptorGetDatabaseadaptor(
    const EnsPTranslationadaptor tla);

AjBool ensTranslationadaptorFetchByIdentifier(EnsPTranslationadaptor tla,
                                              ajuint identifier,
                                              EnsPTranslation *Ptranslation);

AjBool ensTranslationadaptorFetchByStableIdentifier(
    EnsPTranslationadaptor tla,
    const AjPStr stableid,
    ajuint version,
    EnsPTranslation *Ptranslation);

AjBool ensTranslationadaptorFetchByTranscript(EnsPTranslationadaptor tla,
                                              EnsPTranscript transcript);

AjBool ensTranslationadaptorFetchAllByTranscript(EnsPTranslationadaptor tla,
                                                 EnsPTranscript transcript);

AjBool ensTranslationadaptorFetchAllByExternalName(
    EnsPTranslationadaptor tla,
    AjPStr externalname,
    AjPStr externaldbname,
    AjPList translations);

AjBool ensTranslationadaptorFetchAllByTranscriptList(
    EnsPTranslationadaptor tla,
    AjPList transcripts);

AjBool ensTranslationadaptorFetchAllByTranscriptTable(
    EnsPTranslationadaptor tla,
    AjPTable transcripts);

AjBool ensTranslationadaptorFetchAllIdentifiers(
    const EnsPTranslationadaptor tla,
    AjPList identifiers);

AjBool ensTranslationadaptorFetchAllStableIdentifiers(
    const EnsPTranslationadaptor tla,
    AjPList identifiers);

/*
** End of prototype definitions
*/




#endif /* enstranslation_h */

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ensdatabaseentry_h
#define ensdatabaseentry_h

#include "ensanalysis.h"
#include "ensexternaldatabase.h"




/*
** Prototype definitions
*/

/* Ensembl External Reference */

EnsPExternalreference ensExternalreferenceNew(
    ajuint identifier,
    EnsPAnalysis analysis,
    EnsPExternaldatabase edb,
    AjPStr primaryid,
    AjPStr displayid,
    AjPStr version,
    AjPStr description,
    AjPStr linkageannotation,
    AjPStr infotext,
    EnsEExternalreferenceInfoType infotype);

EnsPExternalreference ensExternalreferenceNewObj(
    const EnsPExternalreference object);

EnsPExternalreference ensExternalreferenceNewRef(
    EnsPExternalreference er);

void ensExternalreferenceDel(
    EnsPExternalreference *Per);

ajuint ensExternalreferenceGetIdentifier(
    const EnsPExternalreference er);

EnsPAnalysis ensExternalreferenceGetAnalysis(
    const EnsPExternalreference er);

EnsPExternaldatabase ensExternalreferenceGetExternaldatabase(
    const EnsPExternalreference er);

AjPStr ensExternalreferenceGetPrimaryIdentifier(
    const EnsPExternalreference er);

AjPStr ensExternalreferenceGetDisplayIdentifier(
    const EnsPExternalreference er);

AjPStr ensExternalreferenceGetVersion(
    const EnsPExternalreference er);

AjPStr ensExternalreferenceGetDescription(
    const EnsPExternalreference er);

AjPStr ensExternalreferenceGetLinkageAnnotation(
    const EnsPExternalreference er);

AjPStr ensExternalreferenceGetInfoText(
    const EnsPExternalreference er);

EnsEExternalreferenceInfoType ensExternalreferenceGetInfoType(
    const EnsPExternalreference er);

AjBool ensExternalreferenceSetIdentifier(EnsPExternalreference er,
                                         ajuint identifier);

AjBool ensExternalreferenceSetAnalysis(EnsPExternalreference er,
                                       EnsPAnalysis analysis);

AjBool ensExternalreferenceSetExternaldatabase(EnsPExternalreference er,
                                               EnsPExternaldatabase edb);

AjBool ensExternalreferenceSetPrimaryIdentifier(EnsPExternalreference er,
                                                AjPStr primaryid);

AjBool ensExternalreferenceSetDisplayIdentifier(EnsPExternalreference er,
                                                AjPStr displayid);

AjBool ensExternalreferenceSetVersion(EnsPExternalreference er,
                                      AjPStr version);

AjBool ensExternalreferenceSetDescription(EnsPExternalreference er,
                                          AjPStr description);

AjBool ensExternalreferenceSetLinkageAnnotation(EnsPExternalreference er,
                                                AjPStr linkageannotation);

AjBool ensExternalreferenceSetInfoText(EnsPExternalreference er,
                                       AjPStr infotext);

AjBool ensExternalreferenceSetInfoType(EnsPExternalreference er,
                                       EnsEExternalreferenceInfoType infotype);

AjBool ensExternalreferenceTrace(const EnsPExternalreference er, ajuint level);

EnsEExternalreferenceInfoType ensExternalreferenceInfoTypeFromStr(
    const AjPStr infotype);

const char *ensExternalreferenceInfoTypeToChar(
    EnsEExternalreferenceInfoType type);

ajulong ensExternalreferenceGetMemsize(const EnsPExternalreference er);

/* Ensembl Identity Reference */

EnsPIdentityreference ensIdentityreferenceNew(AjPStr cigar,
                                              ajint qstart,
                                              ajint qend,
                                              ajint qidentity,
                                              ajint tstart,
                                              ajint tend,
                                              ajint tidentity,
                                              double score,
                                              double evalue);

EnsPIdentityreference ensIdentityreferenceNewObj(
    const EnsPIdentityreference object);

void ensIdentityreferenceDel(EnsPIdentityreference *Pir);

AjPStr ensIdentityreferenceGetCigar(const EnsPIdentityreference ir);

ajint ensIdentityreferenceGetQueryStart(const EnsPIdentityreference ir);

ajint ensIdentityreferenceGetQueryEnd(const EnsPIdentityreference ir);

ajint ensIdentityreferenceGetQueryIdentity(const EnsPIdentityreference ir);

ajint ensIdentityreferenceGetTargetStart(const EnsPIdentityreference ir);

ajint ensIdentityreferenceGetTargetEnd(const EnsPIdentityreference ir);

ajint ensIdentityreferenceGetTargetIdentity(const EnsPIdentityreference ir);

double ensIdentityreferenceGetEvalue(const EnsPIdentityreference ir);

double ensIdentityreferenceGetScore(const EnsPIdentityreference ir);

AjBool ensIdentityreferenceSetCigar(EnsPIdentityreference ir,
                                    AjPStr cigar);

AjBool ensIdentityreferenceSetQueryStart(EnsPIdentityreference ir,
                                         ajint qstart);

AjBool ensIdentityreferenceSetQueryEnd(EnsPIdentityreference ir,
                                       ajint qend);

AjBool ensIdentityreferenceSetQueryIdentity(EnsPIdentityreference ir,
                                            ajint qidentity);

AjBool ensIdentityreferenceSetTargetStart(EnsPIdentityreference ir,
                                          ajint tstart);

AjBool ensIdentityreferenceSetTargetEnd(EnsPIdentityreference ir,
                                        ajint tend);

AjBool ensIdentityreferenceSetTargetIdentity(EnsPIdentityreference ir,
                                             ajint tidentity);

AjBool ensIdentityreferenceSetEvalue(EnsPIdentityreference ir,
                                     double evalue);

AjBool ensIdentityreferenceSetScore(EnsPIdentityreference ir,
                                    double score);

AjBool ensIdentityreferenceTrace(const EnsPIdentityreference ir, ajuint level);

ajulong ensIdentityreferenceGetMemsize(const EnsPIdentityreference ir);

/* Ensembl Gene Ontology Linkage */

EnsPGeneontologylinkage ensGeneontologylinkageNew(AjPStr linkagetype,
                                                  EnsPDatabaseentry source);

EnsPGeneontologylinkage ensGeneontologylinkageNewObj(
    const EnsPGeneontologylinkage object);

EnsPGeneontologylinkage ensGeneontologylinkageNewRef(
    EnsPGeneontologylinkage gol);

void ensGeneontologylinkageDel(EnsPGeneontologylinkage *Pgol);

/* Ensembl Database Entry */

EnsPDatabaseentry ensDatabaseentryNew(EnsPDatabaseentryadaptor dbea,
                                      ajuint identifier,
                                      EnsPAnalysis analysis,
                                      EnsPExternaldatabase edb,
                                      AjPStr primaryid,
                                      AjPStr displayid,
                                      AjPStr version,
                                      AjPStr description,
                                      AjPStr linkageannotation,
                                      EnsEExternalreferenceInfoType infotype,
                                      AjPStr infotext);

EnsPDatabaseentry ensDatabaseentryNewObj(const EnsPDatabaseentry object);

EnsPDatabaseentry ensDatabaseentryNewRef(EnsPDatabaseentry dbe);

void ensDatabaseentryDel(EnsPDatabaseentry *Pdbe);

EnsPDatabaseentryadaptor ensDatabaseentryGetAdaptor(
    const EnsPDatabaseentry dbe);

ajuint ensDatabaseentryGetIdentifier(const EnsPDatabaseentry dbe);

AjPStr ensDatabaseentryGetPrimaryIdentifier(const EnsPDatabaseentry dbe);

AjPStr ensDatabaseentryGetDisplayIdentifier(const EnsPDatabaseentry dbe);

AjPStr ensDatabaseentryGetVersion(const EnsPDatabaseentry dbe);

AjPStr ensDatabaseentryGetDescription(const EnsPDatabaseentry dbe);

AjPStr ensDatabaseentryGetLinkageAnnotation(const EnsPDatabaseentry dbe);

AjPStr ensDatabaseentryGetInfoText(const EnsPDatabaseentry dbe);

EnsEExternalreferenceInfoType ensDatabaseentryGetInfoType(
    const EnsPDatabaseentry dbe);

AjPStr ensDatabaseentryGetDbName(const EnsPDatabaseentry dbe);

AjPStr ensDatabaseentryGetDbRelease(const EnsPDatabaseentry dbe);

AjPStr ensDatabaseentryGetDbDisplayName(const EnsPDatabaseentry dbe);

AjBool ensDatabaseentryGetPrimaryIdIsLinkable(const EnsPDatabaseentry dbe);

AjBool ensDatabaseentryGetDisplayIdIsLinkable(const EnsPDatabaseentry dbe);

ajint ensDatabaseentryGetPriority(const EnsPDatabaseentry dbe);

EnsEExternaldatabaseStatus ensDatabaseentryGetStatus(
    const EnsPDatabaseentry dbe);

EnsEExternaldatabaseType ensDatabaseentryGetType(const EnsPDatabaseentry dbe);

AjPList ensDatabaseentryGetSynonyms(const EnsPDatabaseentry dbe);

AjBool ensDatabaseentryTrace(const EnsPDatabaseentry dbe, ajuint level);

ajulong ensDatabaseentryGetMemsize(const EnsPDatabaseentry dbe);

AjBool ensDatabaseentryAddGeneontologylinkage(EnsPDatabaseentry dbe,
                                              AjPStr linkagetype,
                                              EnsPDatabaseentry source);

const AjPList ensDatabaseentryGetGeneontologylinkages(
    const EnsPDatabaseentry dbe);

AjBool ensDatabaseentryGetAllLinkageTypes(const EnsPDatabaseentry dbe,
                                          AjPList types);

AjBool EnsDatabaseentryClearGeneontologylinkageTypes(EnsPDatabaseentry dbe);

/* Ensembl Database Entry Adaptor */

EnsPDatabaseentryadaptor ensRegistryGetDatabaseentryadaptor(
    EnsPDatabaseadaptor dba);

EnsPDatabaseentryadaptor ensDatabaseentryadaptorNew(
    EnsPDatabaseadaptor dba);

void ensDatabaseentryadaptorDel(EnsPDatabaseentryadaptor *Pdbea);

EnsPDatabaseadaptor ensDatabaseentryadaptorGetDatabaseadaptor(
    const EnsPDatabaseentryadaptor dbea);

AjBool ensDatabaseentryadaptorFetchByIdentifier(
    EnsPDatabaseentryadaptor dbea,
    ajuint identifier,
    EnsPDatabaseentry *Pdbe);

AjBool ensDatabaseentryadaptorFetchByDbNameAccession(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr dbname,
    const AjPStr accession,
    EnsPDatabaseentry *Pdbe);

AjBool ensDatabaseentryadaptorFetchAllByObjectType(
    EnsPDatabaseentryadaptor dbea,
    ajuint objid,
    const AjPStr objtype,
    const AjPStr dbname,
    EnsEExternaldatabaseType dbtype,
    AjPList dbes);

AjBool ensDatabaseentryadaptorFetchAllByGene(
    EnsPDatabaseentryadaptor dbea,
    const EnsPGene gene,
    const AjPStr dbname,
    EnsEExternaldatabaseType dbtype,
    AjPList dbes);

AjBool ensDatabaseentryadaptorFetchAllByTranscript(
    EnsPDatabaseentryadaptor dbea,
    const EnsPTranscript transcript,
    const AjPStr dbname,
    EnsEExternaldatabaseType dbtype,
    AjPList dbes);

AjBool ensDatabaseentryadaptorFetchAllByTranslation(
    EnsPDatabaseentryadaptor dbea,
    const EnsPTranslation translation,
    const AjPStr dbname,
    EnsEExternaldatabaseType dbtype,
    AjPList dbes);

AjBool ensDatabaseentryadaptorFetchAllGeneIdentifiersByExternalName(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr name,
    const AjPStr dbname,
    AjPList idlist);

AjBool ensDatabaseentryadaptorFetchAllTranscriptIdentifiersByExternalName(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr name,
    const AjPStr dbname,
    AjPList idlist);

AjBool ensDatabaseentryadaptorFetchAllTranslationIdentifiersByExternalName(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr name,
    const AjPStr dbname,
    AjPList idlist);

AjBool ensDatabaseentryadaptorFetchAllGeneIdentifiersByExternaldatabaseName(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr dbname,
    AjPList idlist);

AjBool ensDatabaseentryadaptorFetchAllTranscriptIdentifiersByExternaldatabaseName(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr dbname,
    AjPList idlist);

AjBool ensDatabaseentryadaptorFetchAllTranslationIdentifiersByExternaldatabaseName(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr dbname,
    AjPList idlist);

AjBool ensDatabaseentryadaptorFetchAllByDescription(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr description,
    const AjPStr dbname,
    AjPList dbes);

AjBool ensDatabaseentryadaptorFetchAllBySource(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr source,
    AjPList dbes);

/*
** End of prototype definitions
*/




#endif /* ensdatabaseentry_h */

#ifdef __cplusplus
}
#endif

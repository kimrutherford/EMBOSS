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

EnsPExternalreference ensExternalreferenceNew(ajuint identifier,
                                              EnsPAnalysis analysis,
                                              EnsPExternaldatabase edb,
                                              AjPStr primaryid,
                                              AjPStr displayid,
                                              AjPStr version,
                                              AjPStr description,
                                              AjPStr linkageannotation,
                                              AjPStr infotext,
                                              AjEnum infotype);

EnsPExternalreference ensExternalreferenceNewObj(
    const EnsPExternalreference object);

EnsPExternalreference ensExternalreferenceNewRef(EnsPExternalreference er);

void ensExternalreferenceDel(EnsPExternalreference *Per);

ajuint ensExternalreferenceGetIdentifier(const EnsPExternalreference er);

EnsPAnalysis ensExternalreferenceGetAnalysis(const EnsPExternalreference er);

EnsPExternaldatabase ensExternalreferenceGetExternaldatabase(
    const EnsPExternalreference er);

AjPStr ensExternalreferenceGetPrimaryIdentifier(const EnsPExternalreference er);

AjPStr ensExternalreferenceGetDisplayIdentifier(const EnsPExternalreference er);

AjPStr ensExternalreferenceGetVersion(const EnsPExternalreference er);

AjPStr ensExternalreferenceGetDescription(const EnsPExternalreference er);

AjPStr ensExternalreferenceGetLinkageAnnotation(const EnsPExternalreference er);

AjPStr ensExternalreferenceGetInfoText(const EnsPExternalreference er);

AjEnum ensExternalreferenceGetInfoType(const EnsPExternalreference er);

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

AjBool ensExternalreferenceSetVersion(EnsPExternalreference er, AjPStr version);

AjBool ensExternalreferenceSetDescription(EnsPExternalreference er,
                                          AjPStr description);

AjBool ensExternalreferenceSetLinkageAnnotation(EnsPExternalreference er,
                                                AjPStr linkageannotation);

AjBool ensExternalreferenceSetInfoText(EnsPExternalreference er,
                                       AjPStr infotext);

AjBool ensExternalreferenceSetInfoType(EnsPExternalreference er,
                                       AjEnum infotype);

AjBool ensExternalreferenceTrace(const EnsPExternalreference er, ajuint level);

AjEnum ensExternalreferenceInfoTypeFromStr(const AjPStr infotype);

const char *ensExternalreferenceInfoTypeToChar(const AjEnum type);

ajuint ensExternalreferenceGetMemSize(const EnsPExternalreference er);

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

AjBool ensIdentityreferenceSetCigar(EnsPIdentityreference ir, AjPStr cigar);

AjBool ensIdentityreferenceSetQueryStart(EnsPIdentityreference ir,
                                         ajint qstart);

AjBool ensIdentityreferenceSetQueryEnd(EnsPIdentityreference ir, ajint qend);

AjBool ensIdentityreferenceSetQueryIdentity(EnsPIdentityreference ir,
                                            ajint qidentity);

AjBool ensIdentityreferenceSetTargetStart(EnsPIdentityreference ir,
                                          ajint tstart);

AjBool ensIdentityreferenceSetTargetEnd(EnsPIdentityreference ir, ajint tend);

AjBool ensIdentityreferenceSetTargetIdentity(EnsPIdentityreference ir,
                                             ajint tidentity);

AjBool ensIdentityreferenceSetEvalue(EnsPIdentityreference ir, double evalue);

AjBool ensIdentityreferenceSetScore(EnsPIdentityreference ir, double score);

AjBool ensIdentityreferenceTrace(const EnsPIdentityreference ir, ajuint level);

ajuint ensIdentityreferenceGetMemSize(const EnsPIdentityreference ir);

/* Ensembl Gene Ontology Linkage */

EnsPGeneontologylinkage ensGeneontologylinkageNew(AjPStr linkagetype,
                                                  EnsPDatabaseentry source);

EnsPGeneontologylinkage ensGeneontologylinkageNewObj(
    const EnsPGeneontologylinkage object);

EnsPGeneontologylinkage ensGeneontologylinkageNewRef(
    EnsPGeneontologylinkage gol);

void ensGeneontologylinkageDel(EnsPGeneontologylinkage *Pgol);

/* Ensembl Database Entry */

EnsPDatabaseentry ensDatabaseentryNew(EnsPDatabaseentryadaptor adaptor,
                                      ajuint identifier,
                                      EnsPAnalysis analysis,
                                      EnsPExternaldatabase edb,
                                      AjPStr primaryid,
                                      AjPStr displayid,
                                      AjPStr version,
                                      AjPStr description,
                                      AjPStr linkageannotation,
                                      AjEnum infotype,
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

AjPStr ensDatabaseentryGetInfoText(const EnsPDatabaseentry dbe);

AjEnum ensDatabaseentryGetInfoType(const EnsPDatabaseentry dbe);

AjPStr ensDatabaseentryGetDbName(const EnsPDatabaseentry dbe);

AjPStr ensDatabaseentryGetDbRelease(const EnsPDatabaseentry dbe);

AjPStr ensDatabaseentryGetDbDisplayName(const EnsPDatabaseentry dbe);

AjBool ensDatabaseentryGetPrimaryIdIsLinkable(const EnsPDatabaseentry dbe);

AjBool ensDatabaseentryGetDisplayIdIsLinkable(const EnsPDatabaseentry dbe);

ajint ensDatabaseentryGetPriority(const EnsPDatabaseentry dbe);

AjEnum ensDatabaseentryGetStatus(const EnsPDatabaseentry dbe);

AjEnum ensDatabaseentryGetType(const EnsPDatabaseentry dbe);

AjPList ensDatabaseentryGetSynonyms(const EnsPDatabaseentry dbe);

AjBool ensDatabaseentryTrace(const EnsPDatabaseentry dbe, ajuint level);

ajuint ensDatabaseentryGetMemSize(const EnsPDatabaseentry dbe);

AjBool ensDatabaseentryAddGeneontologylinkage(EnsPDatabaseentry dbe,
                                              AjPStr linkagetype,
                                              EnsPDatabaseentry source);

const AjPList ensDatabaseentryGetGeneontologylinkages(const EnsPDatabaseentry dbe);

AjBool ensDatabaseentryGetAllLinkageTypes(const EnsPDatabaseentry dbe, AjPList types);

AjBool EnsDatabaseentryClearGeneontologylinkageTypes(EnsPDatabaseentry dbe);

/* Ensembl Database Entry Adaptor */

EnsPDatabaseentryadaptor ensDatabaseentryadaptorNew(EnsPDatabaseadaptor dba);

void ensDatabaseentryadaptorDel(EnsPDatabaseentryadaptor *Pdbea);

EnsPDatabaseadaptor ensDatabaseentryadaptorGetDatabaseadaptor(
    const EnsPDatabaseentryadaptor dbea);

AjBool ensDatabaseentryadaptorFetchByIdentifier(EnsPDatabaseentryadaptor dbea,
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
    AjPStr objtype,
    AjPStr dbname,
    AjEnum dbtype,
    AjPList dbes);

AjBool ensDatabaseentryadaptorFetchAllGeneIdentifiersByExternalName(
    EnsPDatabaseentryadaptor dbea,
    AjPStr name,
    AjPStr dbname,
    AjPList idlist);

AjBool ensDatabaseentryadaptorFetchAllTranscriptIdentifiersByExternalName(
    EnsPDatabaseentryadaptor dbea,
    AjPStr name,
    AjPStr dbname,
    AjPList idlist);

AjBool ensDatabaseentryadaptorFetchAllTranslationIdentifiersByExternalName(
    EnsPDatabaseentryadaptor dbea,
    AjPStr name,
    AjPStr dbname,
    AjPList idlist);

AjBool ensDatabaseentryadaptorFetchAllGeneIdentifiersByExternaldatabaseName(
    EnsPDatabaseentryadaptor dbea,
    AjPStr dbname,
    AjPList idlist);

/*
** End of prototype definitions
*/




#endif

#ifdef __cplusplus
}
#endif

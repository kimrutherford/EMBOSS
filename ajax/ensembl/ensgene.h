#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ensgene_h
#define ensgene_h

#include "ensdata.h"
#include "ensfeature.h"




/*
** Prototype definitions
*/

/* Ensembl Gene */

EnsPGene ensGeneNew(EnsPGeneadaptor ga,
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

EnsPGene ensGeneNewObj(const EnsPGene object);

EnsPGene ensGeneNewRef(EnsPGene gene);

void ensGeneDel(EnsPGene* Pgene);

EnsPGeneadaptor ensGeneGetAdaptor(const EnsPGene gene);

ajuint ensGeneGetIdentifier(const EnsPGene gene);

EnsPFeature ensGeneGetFeature(const EnsPGene gene);

EnsPDatabaseentry ensGeneGetDisplayReference(const EnsPGene gene);

AjPStr ensGeneGetDescription(const EnsPGene gene);

AjPStr ensGeneGetSource(const EnsPGene gene);

AjPStr ensGeneGetBioType(const EnsPGene gene);

EnsEGeneStatus ensGeneGetStatus(const EnsPGene gene);

AjBool ensGeneGetCurrent(const EnsPGene gene);

AjPStr ensGeneGetCanonicalAnnotation(const EnsPGene gene);

ajuint ensGeneGetCanonicalTranscriptIdentifier(const EnsPGene gene);

AjPStr ensGeneGetStableIdentifier(const EnsPGene gene);

ajuint ensGeneGetVersion(const EnsPGene gene);

AjPStr ensGeneGetCreationDate(const EnsPGene gene);

AjPStr ensGeneGetModificationDate(const EnsPGene gene);

const AjPList ensGeneGetAttributes(EnsPGene gene);

const AjPList ensGeneGetDatabaseEntries(EnsPGene gene);

const AjPList ensGeneGetTranscripts(EnsPGene gene);

AjBool ensGeneSetAdaptor(EnsPGene gene, EnsPGeneadaptor ga);

AjBool ensGeneSetIdentifier(EnsPGene gene, ajuint identifier);

AjBool ensGeneSetFeature(EnsPGene gene, EnsPFeature feature);

AjBool ensGeneSetDisplayReference(EnsPGene gene, EnsPDatabaseentry dbe);

AjBool ensGeneSetDescription(EnsPGene gene, AjPStr description);

AjBool ensGeneSetSource(EnsPGene gene, AjPStr source);

AjBool ensGeneSetBioType(EnsPGene gene, AjPStr biotype);

AjBool ensGeneSetStatus(EnsPGene gene, EnsEGeneStatus status);

AjBool ensGeneSetCurrent(EnsPGene gene, AjBool current);

AjBool ensGeneSetCanonicalTranscript(EnsPGene gene, ajuint cantrcid);

AjBool ensGeneSetCanonicalAnnotation(EnsPGene gene, AjPStr canann);

AjBool ensGeneSetStableIdentifier(EnsPGene gene, AjPStr stableid);

AjBool ensGeneSetVersion(EnsPGene gene, ajuint version);

AjBool ensGeneSetCreationDate(EnsPGene gene, AjPStr cdate);

AjBool ensGeneSetModificationDate(EnsPGene gene, AjPStr mdate);

AjBool ensGeneTrace(const EnsPGene gene, ajuint level);

ajulong ensGeneGetMemsize(const EnsPGene gene);

AjBool ensGeneAddAttribute(EnsPGene gene, EnsPAttribute attribute);

AjBool ensGeneAddDatabaseentry(EnsPGene gene, EnsPDatabaseentry dbe);

AjBool ensGeneAddTranscript(EnsPGene gene, EnsPTranscript transcript);

EnsEGeneStatus ensGeneStatusFromStr(const AjPStr status);

const char* ensGeneStatusToChar(EnsEGeneStatus status);

AjBool ensGeneCalculateCoordinates(EnsPGene gene);

AjBool ensGeneFetchAllAttributes(EnsPGene gene,
                                 const AjPStr code,
                                 AjPList attributes);

AjBool ensGeneFetchAllDatabaseEntries(EnsPGene gene,
                                      const AjPStr name,
                                      EnsEExternaldatabaseType type,
                                      AjPList dbes);

AjBool ensGeneFetchAllExons(EnsPGene gene, AjPList exons);

AjBool ensGeneFetchCanonicalTranscript(EnsPGene gene,
                                       EnsPTranscript *Ptranscript);

EnsPGene ensGeneTransform(EnsPGene gene,
                          const AjPStr csname,
                          const AjPStr csversion);

EnsPGene ensGeneTransfer(EnsPGene gene, EnsPSlice slice);

AjBool ensGeneSortByStartAscending(AjPList genes);

AjBool ensGeneSortByStartDescending(AjPList genes);

/* Ensembl Gene Adaptor */

EnsPGeneadaptor ensRegistryGetGeneadaptor(
    EnsPDatabaseadaptor dba);

EnsPGeneadaptor ensGeneadaptorNew(
    EnsPDatabaseadaptor dba);

void ensGeneadaptorDel(EnsPGeneadaptor *Pga);

EnsPFeatureadaptor ensGeneadaptorGetFeatureadaptor(const EnsPGeneadaptor ga);

EnsPDatabaseadaptor ensGeneadaptorGetDatabaseadaptor(const EnsPGeneadaptor ga);

AjBool ensGeneadaptorFetchAll(const EnsPGeneadaptor ga,
                              AjPList genes);

AjBool ensGeneadaptorFetchAllByBiotype(EnsPGeneadaptor ga,
                                       const AjPStr biotype,
                                       AjPList genes);

AjBool ensGeneadaptorFetchAllBySlice(EnsPGeneadaptor ga,
                                     EnsPSlice slice,
                                     const AjPStr anname,
                                     const AjPStr source,
                                     const AjPStr biotype,
                                     AjBool loadtranscripts,
                                     AjPList genes);

AjBool ensGeneadaptorFetchAllByStableIdentifier(EnsPGeneadaptor ga,
                                                const AjPStr stableid,
                                                AjPList genes);

AjBool ensGeneadaptorFetchByIdentifier(EnsPGeneadaptor ga,
                                       ajuint identifier,
                                       EnsPGene *Pgene);

AjBool ensGeneadaptorFetchByStableIdentifier(EnsPGeneadaptor ga,
                                             const AjPStr stableid,
                                             ajuint version,
                                             EnsPGene *Pgene);

AjBool ensGeneadaptorFetchByDisplayLabel(EnsPGeneadaptor ga,
                                         const AjPStr label,
                                         EnsPGene *Pgene);

AjBool ensGeneadaptorFetchAllIdentifiers(const EnsPGeneadaptor ga,
                                         AjPList identifiers);

AjBool ensGeneadaptorFetchAllStableIdentifiers(const EnsPGeneadaptor ga,
                                               AjPList identifiers);

/*
** End of prototype definitions
*/




#endif /* ensgene_h */

#ifdef __cplusplus
}
#endif

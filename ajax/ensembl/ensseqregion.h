#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ensseqregion_h
#define ensseqregion_h

#include "enscache.h"
#include "enscoordsystem.h"
#include "ensdata.h"




/*
** Prototype definitions
*/

/* Ensembl Sequence Region */

EnsPSeqregion ensSeqregionNew(EnsPSeqregionadaptor sra,
                              ajuint identifier,
                              EnsPCoordsystem cs,
                              AjPStr name,
                              ajint length);

EnsPSeqregion ensSeqregionNewObj(EnsPSeqregion object);

EnsPSeqregion ensSeqregionNewRef(EnsPSeqregion sr);

void ensSeqregionDel(EnsPSeqregion *Psr);

const EnsPSeqregionadaptor ensSeqregionGetAdaptor(const EnsPSeqregion sr);

ajuint ensSeqregionGetIdentifier(const EnsPSeqregion sr);

EnsPCoordsystem ensSeqregionGetCoordsystem(const EnsPSeqregion sr);

AjPStr ensSeqregionGetName(const EnsPSeqregion sr);

ajint ensSeqregionGetLength(const EnsPSeqregion sr);

const AjPList ensSeqregionGetAttributes(EnsPSeqregion sr);

AjBool ensSeqregionSetAdaptor(EnsPSeqregion sr, EnsPSeqregionadaptor sra);

AjBool ensSeqregionSetIdentifier(EnsPSeqregion sr, ajuint identifier);

AjBool ensSeqregionSetCoordsystem(EnsPSeqregion sr, EnsPCoordsystem cs);

AjBool ensSeqregionSetName(EnsPSeqregion sr, AjPStr name);

AjBool ensSeqregionSetLength(EnsPSeqregion sr, ajint length);

AjBool ensSeqregionAddAttribute(EnsPSeqregion sr, EnsPAttribute attribute);

AjBool ensSeqregionTrace(const EnsPSeqregion sr, ajuint level);

ajulong ensSeqregionGetMemsize(const EnsPSeqregion sr);

AjBool ensSeqregionMatch(const EnsPSeqregion sr1, const EnsPSeqregion sr2);

AjBool ensSeqregionFetchAllAttributes(EnsPSeqregion sr,
                                      const AjPStr code,
                                      AjPList attributes);

AjBool ensSeqregionIsNonReference(EnsPSeqregion sr, AjBool *Presult);

AjBool ensSeqregionIsTopLevel(EnsPSeqregion sr, AjBool *Presult);

/* Ensembl Sequence Region Adaptor */

EnsPSeqregionadaptor ensRegistryGetSeqregionadaptor(
    EnsPDatabaseadaptor dba);

EnsPSeqregionadaptor ensSeqregionadaptorNew(
    EnsPDatabaseadaptor dba);

void ensSeqregionadaptorDel(EnsPSeqregionadaptor *Psra);

EnsPDatabaseadaptor ensSeqregionadaptorGetDatabaseadaptor(
    const EnsPSeqregionadaptor sra);

AjBool ensSeqregionadaptorCacheInsert(EnsPSeqregionadaptor sra,
                                      EnsPSeqregion *Psr);

AjBool ensSeqregionadaptorCacheRemove(EnsPSeqregionadaptor sra,
                                      const EnsPSeqregion sr);

AjBool ensSeqregionadaptorFetchByIdentifier(EnsPSeqregionadaptor sra,
                                            ajuint identifier,
                                            EnsPSeqregion *Psr);

AjBool ensSeqregionadaptorFetchByName(EnsPSeqregionadaptor sra,
                                      const EnsPCoordsystem cs,
                                      const AjPStr name,
                                      EnsPSeqregion *Psr);

AjBool ensSeqregionadaptorFetchByNameFuzzy(EnsPSeqregionadaptor sra,
                                           const EnsPCoordsystem cs,
                                           const AjPStr name,
                                           EnsPSeqregion *Psr);

AjBool ensSeqregionadaptorFetchAllByCoordsystem(EnsPSeqregionadaptor sra,
                                                const EnsPCoordsystem cs,
                                                AjPList srs);

AjBool ensSeqregionadaptorFetchAllByAttributeCodeValue(
    EnsPSeqregionadaptor sra,
    const AjPStr code,
    const AjPStr value,
    AjPList srs);

AjBool ensSeqregionadaptorIsNonReference(EnsPSeqregionadaptor sra,
                                         const EnsPSeqregion sr,
                                         AjBool *Presult);

/*
** End of prototype definitions
*/




#endif /* ensseqregion_h */

#ifdef __cplusplus
}
#endif

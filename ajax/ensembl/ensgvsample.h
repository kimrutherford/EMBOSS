#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ensgvsample_h
#define ensgvsample_h

#include "ensgvdata.h"




/*
** Prototype definitions
*/

/* Ensembl Genetic Variation Sample */

EnsPGvsample ensGvsampleNew(EnsPGvsampleadaptor gvsa,
                            ajuint identifier,
                            AjPStr name,
                            AjPStr description,
                            EnsEGvsampleDisplay display,
                            ajuint size);

EnsPGvsample ensGvsampleNewObj(const EnsPGvsample object);

EnsPGvsample ensGvsampleNewRef(EnsPGvsample gvs);

void ensGvsampleDel(EnsPGvsample* Pgvs);

EnsPGvsampleadaptor ensGvsampleGetAdaptor(const EnsPGvsample gvs);

ajuint ensGvsampleGetIdentifier(const EnsPGvsample gvs);

AjPStr ensGvsampleGetName(const EnsPGvsample gvs);

AjPStr ensGvsampleGetDescription(const EnsPGvsample gvs);

EnsEGvsampleDisplay ensGvsampleGetDisplay(const EnsPGvsample gvs);

ajuint ensGvsampleGetSize(const EnsPGvsample gvs);

AjBool ensGvsampleSetAdaptor(EnsPGvsample gvs, EnsPGvsampleadaptor adaptor);

AjBool ensGvsampleSetIdentifier(EnsPGvsample gvs, ajuint identifier);

AjBool ensGvsampleSetName(EnsPGvsample gvs, AjPStr name);

AjBool ensGvsampleSetDescription(EnsPGvsample gvs, AjPStr description);

AjBool ensGvsampleSetDisplay(EnsPGvsample gvs, EnsEGvsampleDisplay display);

AjBool ensGvsampleSetSize(EnsPGvsample gvs, ajuint size);

ajulong ensGvsampleGetMemsize(const EnsPGvsample gvs);

AjBool ensGvsampleTrace(const EnsPGvsample gvs, ajuint level);

EnsEGvsampleDisplay ensGvsampleDisplayFromStr(const AjPStr display);

const char* ensGvsampleDisplayToChar(EnsEGvsampleDisplay display);

/* Ensembl Genetic Variation Sample Adaptor */

EnsPGvsampleadaptor ensRegistryGetGvsampleadaptor(
    EnsPDatabaseadaptor dba);

EnsPGvsampleadaptor ensGvsampleadaptorNew(
    EnsPDatabaseadaptor dba);

void ensGvsampleadaptorDel(EnsPGvsampleadaptor *Pgvsa);

EnsPBaseadaptor ensGvsampleadaptorGetAdaptor(EnsPGvsampleadaptor gvsa);

AjBool ensGvsampleadaptorFetchAllByDisplay(EnsPGvsampleadaptor gvsa,
                                           EnsEGvsampleDisplay display,
                                           AjPList gvss);

AjBool ensGvsampleadaptorFetchAllSynonymsByIdentifier(
    const EnsPGvsampleadaptor gvsa,
    ajuint identifier,
    const AjPStr source,
    AjPList synonyms);

AjBool ensGvsampleadaptorFetchAllIdentifiersBySynonym(
    const EnsPGvsampleadaptor gvsa,
    const AjPStr synonym,
    const AjPStr source,
    AjPList idlist);

AjBool ensGvsampleadaptorFetchByIdentifier(EnsPGvsampleadaptor gvsa,
                                           ajuint identifier,
                                           EnsPGvsample *Pgvs);

/*
** End of prototype definitions
*/




#endif /* ensgvsample_h */

#ifdef __cplusplus
}
#endif

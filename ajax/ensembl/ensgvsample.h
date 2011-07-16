
#ifndef ENSGVSAMPLE_H
#define ENSGVSAMPLE_H

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ensgvdata.h"

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

/* Ensembl Genetic Variation Sample */

EnsPGvsample ensGvsampleNewCpy(const EnsPGvsample gvs);

EnsPGvsample ensGvsampleNewIni(EnsPGvsampleadaptor gvsa,
                               ajuint identifier,
                               AjPStr name,
                               AjPStr description,
                               EnsEGvsampleDisplay display,
                               ajuint size);

EnsPGvsample ensGvsampleNewRef(EnsPGvsample gvs);

void ensGvsampleDel(EnsPGvsample* Pgvs);

EnsPGvsampleadaptor ensGvsampleGetAdaptor(const EnsPGvsample gvs);

AjPStr ensGvsampleGetDescription(const EnsPGvsample gvs);

EnsEGvsampleDisplay ensGvsampleGetDisplay(const EnsPGvsample gvs);

ajuint ensGvsampleGetIdentifier(const EnsPGvsample gvs);

AjPStr ensGvsampleGetName(const EnsPGvsample gvs);

ajuint ensGvsampleGetSize(const EnsPGvsample gvs);

AjBool ensGvsampleSetAdaptor(EnsPGvsample gvs, EnsPGvsampleadaptor adaptor);

AjBool ensGvsampleSetDescription(EnsPGvsample gvs, AjPStr description);

AjBool ensGvsampleSetDisplay(EnsPGvsample gvs, EnsEGvsampleDisplay display);

AjBool ensGvsampleSetIdentifier(EnsPGvsample gvs, ajuint identifier);

AjBool ensGvsampleSetName(EnsPGvsample gvs, AjPStr name);

AjBool ensGvsampleSetSize(EnsPGvsample gvs, ajuint size);

size_t ensGvsampleCalculateMemsize(const EnsPGvsample gvs);

AjBool ensGvsampleTrace(const EnsPGvsample gvs, ajuint level);

EnsEGvsampleDisplay ensGvsampleDisplayFromStr(const AjPStr display);

const char* ensGvsampleDisplayToChar(EnsEGvsampleDisplay gvsd);

AjBool ensTableGvsampleClear(AjPTable table);

AjBool ensTableGvsampleDelete(AjPTable* Ptable);

/* Ensembl Genetic Variation Sample Adaptor */

EnsPGvsampleadaptor ensRegistryGetGvsampleadaptor(
    EnsPDatabaseadaptor dba);

EnsPGvsampleadaptor ensGvsampleadaptorNew(
    EnsPDatabaseadaptor dba);

void ensGvsampleadaptorDel(EnsPGvsampleadaptor* Pgvsa);

EnsPBaseadaptor ensGvsampleadaptorGetBaseadaptor(
    EnsPGvsampleadaptor gvsa);

EnsPDatabaseadaptor ensGvsampleadaptorGetDatabaseadaptor(
    EnsPGvsampleadaptor gvsa);

AjBool ensGvsampleadaptorFetchAllbyDisplay(
    EnsPGvsampleadaptor gvsa,
    EnsEGvsampleDisplay gvsd,
    AjPList gvss);

AjBool ensGvsampleadaptorFetchAllbyIdentifiers(
    EnsPGvsampleadaptor gvsa,
    AjPTable gvss);

AjBool ensGvsampleadaptorFetchByIdentifier(
    EnsPGvsampleadaptor gvsa,
    ajuint identifier,
    EnsPGvsample* Pgvs);

AjBool ensGvsampleadaptorRetrieveAllIdentifiersBySynonym(
    EnsPGvsampleadaptor gvsa,
    const AjPStr synonym,
    const AjPStr source,
    AjPList idlist);

AjBool ensGvsampleadaptorRetrieveAllSynonymsByIdentifier(
    EnsPGvsampleadaptor gvsa,
    ajuint identifier,
    const AjPStr source,
    AjPList synonyms);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSGVSAMPLE_H */

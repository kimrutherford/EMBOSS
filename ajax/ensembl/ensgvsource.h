
#ifndef ENSGVSOURCE_H
#define ENSGVSOURCE_H

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

/* Ensembl Genetic Variation Source */

EnsPGvsource ensGvsourceNewCpy(const EnsPGvsource gvs);

EnsPGvsource ensGvsourceNewIni(EnsPGvsourceadaptor gvsa,
                               ajuint identifier,
                               AjPStr name,
                               AjPStr version,
                               AjPStr description,
                               AjPStr url,
                               EnsEGvsourceSomatic gvss,
                               EnsEGvsourceType gvst);

EnsPGvsource ensGvsourceNewRef(EnsPGvsource gvs);

void ensGvsourceDel(EnsPGvsource* Pgvs);

EnsPGvsourceadaptor ensGvsourceGetAdaptor(const EnsPGvsource gvs);

AjPStr ensGvsourceGetDescription(const EnsPGvsource gvs);

ajuint ensGvsourceGetIdentifier(const EnsPGvsource gvs);

AjPStr ensGvsourceGetName(const EnsPGvsource gvs);

EnsEGvsourceSomatic ensGvsourceGetSomatic(const EnsPGvsource gvs);

EnsEGvsourceType ensGvsourceGetType(const EnsPGvsource gvs);

AjPStr ensGvsourceGetUrl(const EnsPGvsource gvs);

AjPStr ensGvsourceGetVersion(const EnsPGvsource gvs);

AjBool ensGvsourceSetAdaptor(EnsPGvsource gvs, EnsPGvsourceadaptor gvsa);

AjBool ensGvsourceSetDescription(EnsPGvsource gvs, AjPStr description);

AjBool ensGvsourceSetIdentifier(EnsPGvsource gvs, ajuint identifier);

AjBool ensGvsourceSetName(EnsPGvsource gvs, AjPStr name);

AjBool ensGvsourceSetSomatic(EnsPGvsource gvs, EnsEGvsourceSomatic gvss);

AjBool ensGvsourceSetType(EnsPGvsource gvs, EnsEGvsourceType gvst);

AjBool ensGvsourceSetUrl(EnsPGvsource gvs, AjPStr url);

AjBool ensGvsourceSetVersion(EnsPGvsource gvs, AjPStr version);

AjBool ensGvsourceTrace(const EnsPGvsource gvs, ajuint level);

size_t ensGvsourceCalculateMemsize(const EnsPGvsource gvs);

AjBool ensGvsourceMatch(const EnsPGvsource gvs1, const EnsPGvsource gvs2);

EnsEGvsourceSomatic ensGvsourceSomaticFromStr(
    const AjPStr somatic);

const char* ensGvsourceSomaticToChar(
    EnsEGvsourceSomatic gvss);

EnsEGvsourceType ensGvsourceTypeFromStr(
    const AjPStr type);

const char* ensGvsourceTypeToChar(
    EnsEGvsourceType gvst);

/* Ensembl Genetic Variation Source Adaptor */

EnsPGvsourceadaptor ensRegistryGetGvsourceadaptor(
    EnsPDatabaseadaptor dba);

EnsPGvsourceadaptor ensGvsourceadaptorNew(EnsPDatabaseadaptor dba);

void ensGvsourceadaptorDel(EnsPGvsourceadaptor* Pgvsa);

EnsPBaseadaptor ensGvsourceadaptorGetBaseadaptor(
    const EnsPGvsourceadaptor gvsa);

EnsPDatabaseadaptor ensGvsourceadaptorGetDatabaseadaptor(
    const EnsPGvsourceadaptor gvsa);

AjBool ensGvsourceadaptorFetchAll(
    EnsPGvsourceadaptor gvsa,
    AjPList gvss);

AjBool ensGvsourceadaptorFetchAllbyGvsourceType(
    EnsPGvsourceadaptor gvsa,
    EnsEGvsourceType gvst,
    AjPList gvss);

AjBool ensGvsourceadaptorFetchByIdentifier(
    EnsPGvsourceadaptor gvsa,
    ajuint identifier,
    EnsPGvsource* Pgvs);

AjBool ensGvsourceadaptorFetchByName(
    EnsPGvsourceadaptor gvsa,
    const AjPStr name,
    EnsPGvsource* Pgvs);

AjBool ensGvsourceadaptorFetchDefault(
    EnsPGvsourceadaptor gvsa,
    EnsPGvsource* Pgvs);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSGVSOURCE_H */

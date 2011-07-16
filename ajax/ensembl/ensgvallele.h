
#ifndef ENSGVALLELE_H
#define ENSGVALLELE_H

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

/* Ensembl Genetic Variation Allele */

EnsPGvallele ensGvalleleNewCpy(const EnsPGvallele gva);

EnsPGvallele ensGvalleleNewIni(EnsPGvalleleadaptor gvaa,
                               ajuint identifier,
                               EnsPGvpopulation gvp,
                               AjPStr allele,
                               float frequency,
                               ajuint subsnpid,
                               ajuint counter,
                               ajuint gvvid);

EnsPGvallele ensGvalleleNewRef(EnsPGvallele gva);

void ensGvalleleDel(EnsPGvallele* Pgva);

EnsPGvalleleadaptor ensGvalleleGetAdaptor(const EnsPGvallele gva);

AjPStr ensGvalleleGetAllele(const EnsPGvallele gva);

ajuint ensGvalleleGetCounter(const EnsPGvallele gva);

EnsPGvpopulation ensGvalleleGetGvpopulation(const EnsPGvallele gva);

ajuint ensGvalleleGetGvvariationidentifier(const EnsPGvallele gva);

ajuint ensGvalleleGetIdentifier(const EnsPGvallele gva);

float ensGvalleleGetFrequency(const EnsPGvallele gva);

ajuint ensGvalleleGetSubsnpidentifier(const EnsPGvallele gva);

const AjPList ensGvalleleLoadAllFaileddescriptions(EnsPGvallele gva);

AjPStr ensGvalleleLoadSubsnphandle(EnsPGvallele gva);

AjBool ensGvalleleSetAdaptor(EnsPGvallele gva, EnsPGvalleleadaptor gvaa);

AjBool ensGvalleleSetCounter(EnsPGvallele gva, ajuint counter);

AjBool ensGvalleleSetIdentifier(EnsPGvallele gva, ajuint identifier);

AjBool ensGvalleleSetGvpopulation(EnsPGvallele gva, EnsPGvpopulation gvp);

AjBool ensGvalleleSetGvvariationidentifier(EnsPGvallele gva, ajuint gvvid);

AjBool ensGvalleleSetAllele(EnsPGvallele gva, AjPStr allelestr);

AjBool ensGvalleleSetFrequency(EnsPGvallele gva, float frequency);

AjBool ensGvalleleSetSubsnphandle(EnsPGvallele gva, AjPStr subsnphandle);

AjBool ensGvalleleSetSubsnpidentifier(EnsPGvallele gva, ajuint subsnpid);

AjBool ensGvalleleTrace(const EnsPGvallele gva, ajuint level);

size_t ensGvalleleCalculateMemsize(const EnsPGvallele gva);

AjBool ensGvalleleIsFailed(EnsPGvallele gva, AjBool* Presult);

AjBool ensTableGvalleleClear(AjPTable table);

AjBool ensTableGvalleleDelete(AjPTable* Ptable);

/* Ensembl Genetic Variation Allele Adaptor */

EnsPGvalleleadaptor ensRegistryGetGvalleleadaptor(
    EnsPDatabaseadaptor dba);

EnsPGvalleleadaptor ensGvalleleadaptorNew(
    EnsPDatabaseadaptor dba);

void ensGvalleleadaptorDel(EnsPGvalleleadaptor* Pgvaa);

EnsPDatabaseadaptor ensGvalleleadaptorGetDatabaseadaptor(
    const EnsPGvalleleadaptor gvaa);

EnsPGvdatabaseadaptor ensGvalleleadaptorGetGvdatabaseadaptor(
    const EnsPGvalleleadaptor gvaa);

AjBool ensGvalleleadaptorFetchAllbyGvvariation(EnsPGvalleleadaptor gvaa,
                                               const EnsPGvvariation gvv,
                                               const EnsPGvpopulation gvp,
                                               AjPList gvas);

AjBool ensGvalleleadaptorFetchAllbySubsnpidentifier(
    EnsPGvalleleadaptor gvaa,
    ajuint subsnpid,
    AjPList gvas);

AjBool ensGvalleleadaptorRetrieveAllFaileddescriptions(
    EnsPGvalleleadaptor gvaa,
    const EnsPGvallele gva,
    AjPList descriptions);

AjBool ensGvalleleadaptorRetrieveSubsnphandle(
    EnsPGvalleleadaptor gvaa,
    const EnsPGvallele gva,
    AjPStr* Phandle);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSGVALLELE_H */

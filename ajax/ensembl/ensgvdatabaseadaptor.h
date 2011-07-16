
#ifndef ENSGVDATABASEADAPTOR_H
#define ENSGVDATABASEADAPTOR_H

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

/* Ensembl Genetic Variation Database Adaptor */

EnsPGvdatabaseadaptor ensRegistryGetGvdatabaseadaptor(
    EnsPDatabaseadaptor dba);

EnsPGvdatabaseadaptor ensGvdatabaseadaptorNewIni(EnsPDatabaseadaptor dba);

void ensGvdatabaseadaptorDel(EnsPGvdatabaseadaptor* Pgvdba);

EnsPDatabaseadaptor ensGvdatabaseadaptorGetDatabaseadaptor(
    const EnsPGvdatabaseadaptor gvdba);

AjBool ensGvdatabaseadaptorGetFailedvariations(
    const EnsPGvdatabaseadaptor gvdba);

AjBool ensGvdatabaseadaptorSetDatabaseadaptor(EnsPGvdatabaseadaptor gvdba,
                                              EnsPDatabaseadaptor dba);

AjBool ensGvdatabaseadaptorSetFailedvariations(EnsPGvdatabaseadaptor gvdba,
                                               AjBool fv);

AjBool ensGvdatabaseadaptorTrace(const EnsPGvdatabaseadaptor gvdba,
                                 ajuint level);

AjBool ensGvdatabaseadaptorFailedallelesconstraint(
    EnsPGvdatabaseadaptor gvdba,
    const AjPStr tablename,
    AjPStr* Pconstraint);

AjBool ensGvdatabaseadaptorFailedvariationsconstraint(
    EnsPGvdatabaseadaptor gvdba,
    const AjPStr tablename,
    AjPStr* Pconstraint);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSGVDATABASEADAPTOR_H */

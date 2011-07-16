
#ifndef ENSGVBASEADAPTOR_H
#define ENSGVBASEADAPTOR_H

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

/* Ensembl Genetic Variation Base Adaptor */

EnsPGvbaseadaptor ensGvbaseadaptorNew(
    EnsPGvdatabaseadaptor gvdba,
    const char* const* Ptables,
    const char* const* Pcolumns,
    const EnsPBaseadaptorLeftjoin leftjoin,
    const char* condition,
    const char* final,
    AjBool Fstatement(EnsPDatabaseadaptor dba,
                      const AjPStr statement,
                      EnsPAssemblymapper am,
                      EnsPSlice slice,
                      AjPList objects));

void ensGvbaseadaptorDel(EnsPGvbaseadaptor* Pgvba);

EnsPBaseadaptor ensGvbaseadaptorGetBaseadaptor(
    const EnsPGvbaseadaptor gvba);

EnsPDatabaseadaptor ensGvbaseadaptorGetDatabaseadaptor(
    const EnsPGvbaseadaptor gvba);

AjBool ensGvbaseadaptorGetFailedvariations(
    const EnsPGvbaseadaptor gvba);

EnsPGvdatabaseadaptor ensGvbaseadaptorGetGvdatabaseadaptor(
    const EnsPGvbaseadaptor gvba);

AjBool ensGvbaseadaptorFetchAll(EnsPGvbaseadaptor gvba,
                                AjPList objects);

AjBool ensGvbaseadaptorFetchAllsomatic(EnsPGvbaseadaptor gvba,
                                       AjPList objects);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSGVBASEADAPTOR_H */

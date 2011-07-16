
#ifndef ENSMETACOORDINATE_H
#define ENSMETACOORDINATE_H

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "enscoordsystem.h"

AJ_BEGIN_DECLS




/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */




/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */

/* @data EnsPMetacoordinateadaptor ********************************************
**
** Ensembl Meta-Coordinate Adaptor.
**
** @alias EnsSMetacoordinateadaptor
** @alias EnsOMetacoordinateadaptor
**
** @attr Adaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor.
** @attr CacheByName [AjPTable] Ensembl Feature name (i.e. table name) cache.
** @attr CacheByLength [AjPTable] Maximum length cache.
** @@
******************************************************************************/

typedef struct EnsSMetacoordinateadaptor
{
    EnsPDatabaseadaptor Adaptor;
    AjPTable CacheByName;
    AjPTable CacheByLength;
} EnsOMetacoordinateadaptor;

#define EnsPMetacoordinateadaptor EnsOMetacoordinateadaptor*




/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */

/*
** Prototype definitions
*/

/* Ensembl Meta-Coordinate Adaptor */

EnsPMetacoordinateadaptor ensRegistryGetMetacoordinateadaptor(
    EnsPDatabaseadaptor dba);

EnsPMetacoordinateadaptor ensMetacoordinateadaptorNew(
    EnsPDatabaseadaptor dba);

void ensMetacoordinateadaptorDel(EnsPMetacoordinateadaptor* Pmca);

AjBool ensMetacoordinateadaptorFetchAllCoordsystems(
    const EnsPMetacoordinateadaptor mca,
    const AjPStr name,
    AjPList css);

ajint ensMetacoordinateadaptorGetMaximumlength(
    const EnsPMetacoordinateadaptor mca,
    const EnsPCoordsystem cs,
    const AjPStr name);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSMETACOORDINATE_H */

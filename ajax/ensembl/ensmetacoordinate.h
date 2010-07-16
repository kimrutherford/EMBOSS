#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ensmetacoordinate_h
#define ensmetacoordinate_h

#include "enstable.h"
#include "enscoordsystem.h"




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




/*
** Prototype definitions
*/

/* Ensembl Meta-Coordinate Adaptor */

EnsPMetacoordinateadaptor ensRegistryGetMetacoordinateadaptor(
    EnsPDatabaseadaptor dba);

EnsPMetacoordinateadaptor ensMetacoordinateadaptorNew(
    EnsPDatabaseadaptor dba);

void ensMetacoordinateadaptorDel(EnsPMetacoordinateadaptor *Pmca);

AjBool ensMetacoordinateadaptorFetchAllCoordsystems(
    const EnsPMetacoordinateadaptor mca,
    const AjPStr name,
    AjPList css);

ajuint ensMetacoordinateadaptorGetMaximumlength(
    const EnsPMetacoordinateadaptor mca,
    const EnsPCoordsystem cs,
    const AjPStr name);

/*
** End of prototype definitions
*/




#endif /* ensmetacoordinate_h */

#ifdef __cplusplus
}
#endif

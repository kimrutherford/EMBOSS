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

EnsPMetacoordinateadaptor ensMetacoordinateadaptorNew(EnsPDatabaseadaptor dba);

void ensMetacoordinateadaptorDel(EnsPMetacoordinateadaptor *Padaptor);

AjBool ensMetacoordinateadaptorFetchAllCoordsystems(
    EnsPMetacoordinateadaptor adaptor,
    const AjPStr name,
    AjPList cslist);

ajuint ensMetacoordinateadaptorGetMaximumLength(
    EnsPMetacoordinateadaptor adaptor,
    const EnsPCoordsystem cs,
    const AjPStr name);

/*
** End of prototype definitions
*/




#endif

#ifdef __cplusplus
}
#endif

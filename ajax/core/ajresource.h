#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajresource_h
#define ajresource_h

#include "ajresourcedata.h"
#include "ajax.h"

/*
** Prototype definitions
*/


AjPResource  ajResourceNew(void);
AjPResource  ajResourceNewDrcat(const AjPStr dbname);
AjPResource  ajResourceNewResource(const AjPResource res);
void         ajResourceDel(AjPResource *Presource);
void         ajResourceClear(AjPResource resource);
AjBool       ajResourceGetDbdata(const AjPResource resource, AjPQuery qry,
                                 AjBool findformat(const AjPStr format,
                                                   ajint *iformat));
const AjPStr ajResourceGetEntry(const AjPResource resource);
const AjPStr ajResourceGetId(const AjPResource resource);
const char*  ajResourceGetQryC(const AjPResource resource);
const AjPStr ajResourceGetQryS(const AjPResource resource);
void         ajResourceExit(void);
void         ajResourceTrace(const AjPResource thys);

AjPReslink   ajReslinkNew(void);
AjPReslink   ajReslinkNewReslink(const AjPReslink link);
void         ajReslinkDel(AjPReslink*);
AjBool       ajReslinklistClone(const AjPList src, AjPList dest);

AjPResquery  ajResqueryNew(void);
AjPResquery  ajResqueryNewResquery(const AjPResquery qry);
void         ajResqueryDel(AjPResquery*);
AjBool       ajResquerylistClone(const AjPList src, AjPList dest);

AjPResterm   ajRestermNew(void);
AjPResterm   ajRestermNewResterm(const AjPResterm term);
void         ajRestermDel(AjPResterm*);
AjBool       ajRestermlistClone(const AjPList src, AjPList dest);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif

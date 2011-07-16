#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajurl_h
#define ajurl_h

#include "ajurldata.h"
#include "ajax.h"

/*
** Prototype definitions
*/


AjPUrl      ajUrlNew(void);
void         ajUrlDel(AjPUrl *Purl);
void         ajUrlClear(AjPUrl url);
const AjPStr ajUrlGetDb(const AjPUrl url);
const AjPStr ajUrlGetId(const AjPUrl url);
const char*  ajUrlGetQryC(const AjPUrl url);
const AjPStr ajUrlGetQryS(const AjPUrl url);
void         ajUrlExit(void);


/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif

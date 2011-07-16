#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajtext_h
#define ajtext_h

#include "ajtextdata.h"
#include "ajax.h"

/*
** Prototype definitions
*/


AjPText      ajTextNew(void);
void         ajTextDel(AjPText *Ptext);
void         ajTextClear(AjPText text);
const char*  ajTextGetQryC(const AjPText text);
const AjPStr ajTextGetQryS(const AjPText text);
void         ajTextExit(void);


/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif

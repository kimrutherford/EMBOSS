#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajassem_h
#define ajassem_h

#include "ajassemdata.h"
#include "ajax.h"

/*
** Prototype definitions
*/


AjPAssem     ajAssemNew(void);
void         ajAssemDel(AjPAssem *Passem);
void         ajAssemClear(AjPAssem assem);
const char*  ajAssemGetQryC(const AjPAssem assem);
const AjPStr ajAssemGetQryS(const AjPAssem assem);
void         ajAssemExit(void);


/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif

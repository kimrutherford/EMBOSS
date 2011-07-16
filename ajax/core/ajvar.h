#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajvar_h
#define ajvar_h

#include "ajvardata.h"
#include "ajax.h"

/*
** Prototype definitions
*/


AjPVar      ajVarNew(void);
void         ajVarDel(AjPVar *Pvar);
void         ajVarClear(AjPVar var);
const AjPStr ajVarGetDb(const AjPVar var);
const AjPStr ajVarGetId(const AjPVar var);
const char*  ajVarGetQryC(const AjPVar var);
const AjPStr ajVarGetQryS(const AjPVar var);
void         ajVarExit(void);


/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif

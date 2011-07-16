#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajcall_h
#define ajcall_h

#include "ajax.h"
typedef void* (*CallFunc)(const char *name, va_list args);




/*
** Prototype definitions
*/

void     ajCallRegister(const char *name, CallFunc func);
void     ajCallRegisterOld(const char *name, CallFunc func);
void     ajCallTableRegister(AjPTable table, const char *name, void* func);

AjPTable ajCallTableNew(void);
void*    ajCall(const char *name, ...);
void*    ajCallTableGetC(const AjPTable table, const char *name);
void*    ajCallTableGetS(const AjPTable table, const AjPStr namestr);
void     ajCallExit(void);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif

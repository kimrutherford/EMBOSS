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

void  ajCallRegister(const char *name, CallFunc func);

void* ajCall(const char *name, ...);
void  ajCallExit(void);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajassert_h
#define ajassert_h

#undef assert
#ifdef NDEBUG
#define assert(e) ((void)0)
#else
#include "ajexcept.h"
extern void assert(ajint e);
#define assert(e) ((void)((e)||(AJRAISE(Assert_Failed),0)))
#endif
#endif

#ifdef __cplusplus
}
#endif

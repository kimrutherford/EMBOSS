#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajmem_h
#define ajmem_h

#include "ajdefine.h"
#include "ajexcept.h"

#ifndef HAVE_MEMMOVE
#define ajMemMove(d,s,l) bcopy (s,d,l)
#else
#define ajMemMove(d,s,l) memmove(d,s,l)
#endif




/*
** Prototype definitions
*/

void *ajMemAlloc (size_t nbytes,
	const char *file, ajint line, AjBool nofail);
void *ajMemCalloc(size_t count, size_t nbytes,
	const char *file, ajint line, AjBool nofail);
void *ajMemCallocZero(size_t count, size_t nbytes,
	const char *file, ajint line, AjBool nofail);
void ajMemFree(void *ptr);
void *ajMemResize(void *ptr, size_t nbytes,
	const char *file, ajint line, AjBool nofail);
void *ajMemResizeZero(void *ptr, size_t oldbytes, size_t nbytes,
	const char *file, ajint line, AjBool nofail);
void ajMemSetZero(void* ptr, size_t count, size_t nbytes);

ajint *ajMemArrB (size_t size);
ajint *ajMemArrI (size_t size);
float *ajMemArrF (size_t size);
void ajMemStat (const char* title);
void ajMemExit (void);
void ajMemCheck(int istat);
  void ajMemCheckSetLimit(ajint maxfail);
void ajMemProbe(void* ptr, const char* file, ajint line);

/*
** End of prototype definitions
*/


__deprecated void *ajMemCalloc0(size_t count, size_t nbytes,
				const char *file, ajint line, AjBool nofail);

#define AJALLOC(nbytes) \
	ajMemAlloc((nbytes), __FILE__, __LINE__, AJFALSE)
#define AJALLOC0(nbytes) \
	ajMemCallocZero(1, (nbytes), __FILE__, __LINE__, AJFALSE)
#define AJCALLOC(count, nbytes) \
	ajMemCalloc((count), (nbytes), __FILE__, __LINE__, AJFALSE)
#define AJCALLOC0(count, nbytes) \
	ajMemCallocZero((count), (nbytes), __FILE__, __LINE__, AJFALSE)

#define AJNEW(p) ((p) = AJALLOC(sizeof *(p)))
#define AJCNEW(p,c) ((p) = AJCALLOC((size_t)c, sizeof *(p)))
#define AJNEW0(p) ((p) = AJCALLOC0((size_t)1, sizeof *(p)))
#define AJCNEW0(p,c) ((p) = AJCALLOC0((size_t)c, sizeof *(p)))
#define AJSET0(p) (ajMemSetZero((p), (size_t)1, sizeof *(p)))
#define AJCSET0(p,c) (ajMemSetZero((p), (size_t)c, sizeof *(p)))

#define AJFREE(ptr) ((void)(ajMemFree(ptr), (ptr) = 0))
#define AJRESIZE(ptr, nbytes) 	((ptr) = ajMemResize((ptr), \
	(nbytes), __FILE__, __LINE__, AJFALSE))
#define AJRESIZE0(ptr, oldbytes, nbytes)  (ptr) = ajMemResizeZero((ptr), \
	(oldbytes), (nbytes), __FILE__, __LINE__, AJFALSE))
#define AJCRESIZE(ptr, nbytes) 	((ptr) = ajMemResize((ptr), \
	(nbytes)*sizeof *(ptr), __FILE__, __LINE__, AJFALSE))
#define AJCRESIZE0(ptr, oldbytes, nbytes)   ((ptr) = ajMemResizeZero((ptr), \
	(oldbytes)*sizeof *(ptr), (nbytes)*sizeof *(ptr), \
	__FILE__, __LINE__, AJFALSE))
#define AJCRESIZETRY(ptr, nbytes) 	((ptr) = ajMemResize((ptr), \
	(nbytes)*sizeof *(ptr), __FILE__, __LINE__, AJTRUE))
#define AJMPROBE(ptr) \
	ajMemProbe(ptr, __FILE__, __LINE__)

#endif

#ifdef __cplusplus
}
#endif

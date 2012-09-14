/* @include ajmem *************************************************************
**
** AJAX memory functions
**
** @author Copyright (C) 1999 Peter Rice
** @version $Revision: 1.15 $
** @modified Peter Rice pmr@ebi.ac.uk
** @modified $Date: 2011/10/18 14:23:40 $ by $Author: rice $
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public
** License along with this library; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA  02110-1301,  USA.
**
******************************************************************************/

#ifndef AJMEM_H
#define AJMEM_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */

#ifdef HAVE_MEMMOVE
#define ajMemMove(d,s,l) memmove(d,s,l)
#else /* !HAVE_MEMMOVE */
#define ajMemMove(d,s,l) bcopy(s,d,l)
#endif /* !HAVE_MEMMOVE */




/*
** Prototype definitions
*/

void *ajMemAlloc(size_t nbytes,
                 const char *file, ajint line, AjBool nofail);
void *ajMemCalloc(size_t count, size_t nbytes,
                  const char *file, ajint line, AjBool nofail);
void *ajMemCallocZero(size_t count, size_t nbytes,
                      const char *file, ajint line, AjBool nofail);
void ajMemFree(void **ptr);
void *ajMemResize(void *ptr, size_t nbytes,
                  const char *file, ajint line, AjBool nofail);
void *ajMemResizeZero(void *ptr, size_t oldbytes, size_t nbytes,
                      const char *file, ajint line, AjBool nofail);
void ajMemSetZero(void* ptr, size_t count, size_t nbytes);

ajint *ajMemArrB(size_t size);
ajint *ajMemArrI(size_t size);
float *ajMemArrF(size_t size);
void ajMemStat(const char* title);
void ajMemExit(void);
void ajMemCheck(int istat);
void ajMemCheckSetLimit(ajint maxfail);
void ajMemProbe(void* ptr, const char* file, ajint line);

/*
** End of prototype definitions
*/




#define AJALLOC(nbytes)                                 \
    ajMemAlloc((nbytes), __FILE__, __LINE__, AJFALSE)
#define AJALLOC0(nbytes)                                        \
    ajMemCallocZero(1, (nbytes), __FILE__, __LINE__, AJFALSE)
#define AJCALLOC(count, nbytes)                                 \
    ajMemCalloc((count), (nbytes), __FILE__, __LINE__, AJFALSE)
#define AJCALLOC0(count, nbytes)                                        \
    ajMemCallocZero((count), (nbytes), __FILE__, __LINE__, AJFALSE)

#define AJNEW(p) ((p) = AJALLOC(sizeof *(p)))
#define AJCNEW(p,c) ((p) = AJCALLOC((size_t)c, sizeof *(p)))
#define AJNEW0(p) ((p) = AJCALLOC0((size_t)1, sizeof *(p)))
#define AJCNEW0(p,c) ((p) = AJCALLOC0((size_t)c, sizeof *(p)))
#define AJSET0(p) (ajMemSetZero((p), (size_t)1, sizeof *(p)))
#define AJCSET0(p,c) (ajMemSetZero((p), (size_t)c, sizeof *(p)))

#define AJFREE(ptr) ((void)(ajMemFree((void**)&ptr)))
#define AJRESIZE(ptr, nbytes)                                           \
    ((ptr) = ajMemResize((ptr), (nbytes), __FILE__, __LINE__, AJFALSE))
#define AJRESIZE0(ptr, oldbytes, nbytes)                        \
    ((ptr) = ajMemResizeZero((ptr), (oldbytes), (nbytes),       \
                             __FILE__, __LINE__, AJFALSE))
#define AJCRESIZE(ptr, nbytes)                          \
    ((ptr) = ajMemResize((ptr), (nbytes)*sizeof *(ptr), \
                         __FILE__, __LINE__, AJFALSE))
#define AJCRESIZE0(ptr, oldbytes, nbytes)                       \
    ((ptr) = ajMemResizeZero((ptr), (oldbytes)*sizeof *(ptr),   \
                             (nbytes)*sizeof *(ptr),            \
                             __FILE__, __LINE__, AJFALSE))
#define AJCRESIZETRY(ptr, nbytes)                       \
    ((ptr) = ajMemResize((ptr), (nbytes)*sizeof *(ptr), \
                         __FILE__, __LINE__, AJTRUE))
#define AJCRESIZETRY0(ptr, oldbytes, nbytes)                    \
    ((ptr) = ajMemResizeZero((ptr), (oldbytes)*sizeof *(ptr),   \
                             (nbytes)*sizeof *(ptr),            \
                             __FILE__, __LINE__, AJTRUE))
#define AJMPROBE(ptr)                           \
    ajMemProbe(ptr, __FILE__, __LINE__)




#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif /* AJ_COMPILE_DEPRECATED_BOOK */

#ifdef AJ_COMPILE_DEPRECATED

__deprecated void *ajMemCalloc0(size_t count, size_t nbytes,
                                const char *file, ajint line, AjBool nofail);

#endif /* AJ_COMPILE_DEPRECATED */




AJ_END_DECLS

#endif /* !AJMEM_H */

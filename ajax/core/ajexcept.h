/* @include ajexcept **********************************************************
**
** AJAX exception handling functions
**
** These functions control exception handling
**
** @author Copyright (C) 1998 Peter Rice
** @version $Revision: 1.13 $
** @modified Dec 31 pmr First version
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

#ifndef AJEXCEPT_H
#define AJEXCEPT_H




/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"

#include <setjmp.h>

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* @data Except_T *************************************************************
**
** Exception data structure
**
** @attr reason [const char*] Reason for the exception
** @@
******************************************************************************/

typedef struct Except_T
{
    const char *reason;
} Except_T;




/* @data Except_Frame *********************************************************
**
** Exception frame data structure
**
** @attr env [jmp_buf] Environment jump buffer
** @attr prev [struct Except_Frame*] Previous exception if any
** @attr file [const char*] source filename usually from __FILE__
** @attr exception [const Except_T*] Exception data (reason)
** @attr line [ajint] source line number usually from __LINE__
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct Except_Frame
{
    jmp_buf env;
    struct Except_Frame *prev;
    const char *file;
    const Except_T *exception;
    ajint line;
    char Padding[4];
} Except_Frame;

enum { Except_entered=0, Except_raised,
       Except_handled,   Except_finalized };
extern Except_Frame *Except_stack;


extern const Except_T Assert_Failed;




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */




/*
** Prototype definitions
*/

void ajExceptRaise(const Except_T *e, const char *file, ajint line);

/*
** End of prototype definitions
*/




#define AJRAISE(e) ajExceptRaise(&(e), __FILE__, __LINE__)
#define AJRERAISE ajExceptRaise(Except_frame.exception,                 \
                                Except_frame.file, Except_frame.line)
#define AJRETURN switch (Except_stack = Except_stack->prev,0) default: return
#define AJTRY do {                              \
    volatile ajint Except_flag;                 \
    Except_Frame Except_frame;                  \
    Except_frame.prev = Except_stack;           \
    Except_stack = &Except_frame;               \
    Except_flag = setjmp(Except_frame.env);     \
    if (Except_flag == Except_entered) {
#define AJEXCEPT(e)                                                     \
    if (Except_flag == Except_entered) Except_stack = Except_stack->prev; \
    } else if (Except_frame.exception == &(e)) {                        \
    Except_flag = Except_handled;
#define ELSE                                                            \
    if (Except_flag == Except_entered) Except_stack = Except_stack->prev; \
    } else {                                                            \
    Except_flag = Except_handled;
#define FINALLY                                                         \
    if (Except_flag == Except_entered) Except_stack = Except_stack->prev; \
    } {                                                                 \
    if (Except_flag == Except_entered)                                  \
        Except_flag = Except_finalized;
#define END_TRY                                                         \
    if (Except_flag == Except_entered) Except_stack = Except_stack->prev; \
    } if (Except_flag == Except_raised) AJRERAISE;                      \
    } while (0)
#undef T




AJ_END_DECLS

#endif  /* !AJEXCEPT_H */

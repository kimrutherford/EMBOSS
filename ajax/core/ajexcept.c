/* @source ajexcept ***********************************************************
**
** AJAX exception handling functions
**
** These functions control exception handling
**
** @author Copyright (C) 1998 Peter Rice
** @version $Revision: 1.14 $
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

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#include "ajexcept.h"
#include "ajassert.h"
#include "ajmess.h"
#include "ajutil.h"

#define T Except_T

Except_Frame *Except_stack = NULL;




/* @const Assert_Failed *******************************************************
**
** Exception for a failed assert
**
******************************************************************************/

const Except_T Assert_Failed = { "Assertion failed" };




/* @func ajExceptRaise ********************************************************
**
** Check for exception. If not caught abort. Else call exception routine.
**
** @param [r] e [const T*] Exception code
** @param [r] file [const char*] file exceptions called from
** @param [r] line [ajint] line number exception called from
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

__noreturn void  ajExceptRaise(const T* e, const char* file,
		   ajint line)
{
    Except_Frame *p;

    p = Except_stack;
    
    assert(e);

    if(p == NULL)
    {
	ajMessOut("Uncaught exception: ");

	if(e->reason)
	    ajMessOut(" %s,", e->reason);
	else
	    ajMessOut(" at 0x%p,", e);

	if(file && line > 0)
	    ajMessOut(" raised at %s:%d\n", file, line);

        ajUtilCatch();
	exit(EXIT_FAILURE);
    }

    p->exception = e;
    p->file      = file;
    p->line      = line;
    Except_stack = Except_stack->prev;

    longjmp(p->env, Except_raised);
}

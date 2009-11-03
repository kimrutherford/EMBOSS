/******************************************************************************
** @source AJAX assert functions
**
** Functions to handle assert statements.
**
** @author Copyright (C) 1998 Ian Longden
** @version 1.0
** @modified Jun 25 pmr First version
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA  02111-1307, USA.
******************************************************************************/

#include "ajassert.h"




#ifndef WIN32

/* @func assert ***************************************************************
**
** The assert() macro inserts diagnostics into programs.
**
** @param [r] e [ajint] test for assertion.
** @return [void]
** @@
******************************************************************************/

void (assert)(ajint e)
{
    assert(e);
}

#endif

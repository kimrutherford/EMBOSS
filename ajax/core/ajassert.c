/* @source ajassert ***********************************************************
**
** AJAX assert functions
**
** Functions to handle assert statements.
**
** @author Copyright (C) 1998 Ian Longden
** @version $Revision: 1.13 $
** @modified Jun 25 pmr First version
** @modified $Date: 2011/10/18 14:23:40 $ by $Author: rice $
** @@
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

#include "ajassert.h"




#ifndef WIN32
/* @func assert ***************************************************************
**
** The assert() macro inserts diagnostics into programs.
**
** @param [r] e [ajint] test for assertion.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void (assert) (ajint e)
{
    assert(e);

    return;
}

#endif

/* @source ajvardb ************************************************************
**
** AJAX variation database functions
**
** These functions control all aspects of AJAX variation database access
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.8 $
** @modified Oct 2010 pmr first version
** @modified $Date: 2012/04/26 17:36:15 $ by $Author: mks $
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


#include "ajlib.h"

#include "ajvardb.h"
#include "ajvarread.h"
#include "ajcall.h"

#include <limits.h>
#include <stdarg.h>
#include <sys/types.h>

#include <errno.h>
#include <signal.h>

#ifndef WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <netdb.h>

#include <dirent.h>
#include <unistd.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#endif




/* @funclist varAccess ********************************************************
**
** Functions to access each database or variation access method
**
******************************************************************************/

static AjOVarAccess varAccess[] =
{
  /*  Name      AccessFunction   FreeFunction
      Qlink    Description
      Alias    Entry    Query    All      Chunk   Padding */
    {
      NULL, NULL, NULL,
      NULL, NULL,
      AJFALSE, AJFALSE, AJFALSE, AJFALSE, AJFALSE, AJFALSE
    },
    {
      NULL, NULL, NULL,
      NULL, NULL,
      AJFALSE, AJFALSE, AJFALSE, AJFALSE, AJFALSE, AJFALSE
    }
};




/* @func ajVardbInit **********************************************************
**
** Initialise variation database internals
**
** @return [void]
**
** @release 6.4.0
******************************************************************************/

void ajVardbInit(void)
{
    AjPTable table;
    ajuint i = 0;

    table = ajVaraccessGetDb();

    while(varAccess[i].Name)
    {
        ajCallTableRegister(table, varAccess[i].Name,
                            (void*) &varAccess[i]);
	i++;
    }

    return;
}




/* @func ajVardbPrintAccess ***************************************************
**
** Reports the internal data structures
**
** @param [u] outf [AjPFile] Output file
** @param [r] full [AjBool] Full report (usually ajFalse)
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajVardbPrintAccess(AjPFile outf, AjBool full)
{
    ajint i = 0;

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# Variation access methods\n");
    ajFmtPrintF(outf, "# Name       Alias Entry Query   All Description\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "method {\n");

    for(i=0; varAccess[i].Name; i++)
	if(full || !varAccess[i].Alias)
	    ajFmtPrintF(outf, "  %-10s %5B %5B %5B %5B \"%s\"\n",
			varAccess[i].Name,  varAccess[i].Alias,
			varAccess[i].Entry, varAccess[i].Query,
			varAccess[i].All,   varAccess[i].Desc);

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajVardbExit **********************************************************
**
** Cleans up variation database processing internal memory
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajVardbExit(void)
{
    return;
}

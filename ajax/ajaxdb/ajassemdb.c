/******************************************************************************
** @source AJAX ASSEMDB (Assemby database) functions
**
** These functions control all aspects of AJAX assembly database access
**
** @author Copyright (C) 2010 Peter Rice
** @version 1.0
** @modified Oct 2010 pmr first version
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

#include "ajax.h"
#include "ajassemdb.h"

#include <limits.h>
#include <stdarg.h>
#include <sys/types.h>
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
#include <errno.h>
#include <signal.h>




/* @funclist assemAccess ******************************************************
**
** Functions to access each database or assembly access method
**
******************************************************************************/

static AjOAssemAccess assemAccess[] =
{
  /* Name      AccessFunction   FreeFunction
     Qlink    Description
     Alias    Entry    Query    All      Chunk */
    {NULL, NULL, NULL,
     NULL, NULL,
     AJFALSE, AJFALSE, AJFALSE, AJFALSE, AJFALSE},

    {NULL, NULL, NULL,
     NULL, NULL,
     AJFALSE, AJFALSE, AJFALSE, AJFALSE, AJFALSE}
};




/* @func ajAssemdbInit ********************************************************
**
** Initialise assembly database internals
**
** @return [void]
******************************************************************************/

void ajAssemdbInit(void)
{
    AjPTable table;
    ajuint i = 0;

    table = ajAssemaccessGetDb();

    while(assemAccess[i].Name)
    {
        ajCallTableRegister(table, assemAccess[i].Name,
                            (void*) &assemAccess[i]);
	i++;
    }

    return;
}




/* @func ajAssemdbPrintAccess **************************************************
**
** Reports the internal data structures
**
** @param [u] outf [AjPFile] Output file
** @param [r] full [AjBool] Full report (usually ajFalse)
** @return [void]
** @@
******************************************************************************/

void ajAssemdbPrintAccess(AjPFile outf, AjBool full)
{
    ajint i = 0;

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# Assembly access methods\n");
    ajFmtPrintF(outf, "# Name       Alias Entry Query   All Description\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "method {\n");

    for(i=0; assemAccess[i].Name; i++)
	if(full || !assemAccess[i].Alias)
	    ajFmtPrintF(outf, "  %-10s %5B %5B %5B %5B \"%s\"\n",
			assemAccess[i].Name,  assemAccess[i].Alias,
			assemAccess[i].Entry, assemAccess[i].Query,
			assemAccess[i].All,   assemAccess[i].Desc);

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajAssemdbExit ********************************************************
**
** Cleans up assembly database processing internal memory
**
** @return [void]
** @@
******************************************************************************/

void ajAssemdbExit(void)
{
    return;
}

/******************************************************************************
** @source AJAX OBODB (OBO database) functions
**
** These functions control all aspects of AJAX obo database access
**
** @author Copyright (C) 2010 Peter Rice
** @version 1.0
** @modified Sep 2010 pmr first version
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
#include "ajobodb.h"

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




/* @funclist oboAccess ********************************************************
**
** Functions to access each database or obo access method
**
******************************************************************************/

static AjOOboAccess oboAccess[] =
{
  /* Name      AccessFunction   FreeFunction
     Qlink,   Description
     Alias    Entry    Query    All      Chunk */
    {NULL, NULL, NULL,
     NULL, NULL,
     AJFALSE, AJFALSE, AJFALSE, AJFALSE, AJFALSE},

    {NULL, NULL, NULL,
     NULL, NULL,
     AJFALSE, AJFALSE, AJFALSE, AJFALSE, AJFALSE}
};




/* @func ajObodbInit **********************************************************
**
** Initialise obo database internals
**
** @return [void]
******************************************************************************/

void ajObodbInit(void)
{
    AjPTable table;
    ajuint i = 0;

    table = ajOboaccessGetDb();

    while(oboAccess[i].Name)
    {
        ajCallTableRegister(table, oboAccess[i].Name, (void*) &oboAccess[i]);
	i++;
    }

    return;
}




/* @func ajObodbPrintAccess ***************************************************
**
** Reports the internal data structures
**
** @param [u] outf [AjPFile] Output file
** @param [r] full [AjBool] Full report (usually ajFalse)
** @return [void]
** @@
******************************************************************************/

void ajObodbPrintAccess(AjPFile outf, AjBool full)
{
    ajint i = 0;

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# Obo term access methods\n");
    ajFmtPrintF(outf, "# Name       Alias Entry Query   All Description\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "method {\n");

    for(i=0; oboAccess[i].Name; i++)
	if(full || !oboAccess[i].Alias)
	    ajFmtPrintF(outf, "  %-10s %5B %5B %5B %5B \"%s\"\n",
			oboAccess[i].Name,  oboAccess[i].Alias,
			oboAccess[i].Entry, oboAccess[i].Query,
			oboAccess[i].All,   oboAccess[i].Desc);

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajObodbExit **********************************************************
**
** Cleans up obo database processing internal memory
**
** @return [void]
** @@
******************************************************************************/

void ajObodbExit(void)
{
    return;
}

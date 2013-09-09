/* @source ajxmldb ***********************************************************
**
** AJAX xml database functions
**
** These functions control all aspects of AJAX xml database access
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.2 $
** @modified Oct 2010 pmr first version
** @modified $Date: 2012/11/14 14:18:37 $ by $Author: rice $
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

#include "ajxml.h"
#include "ajxmldb.h"
#include "ajxmlread.h"
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




/* @funclist xmlAccess ******************************************************
**
** Functions to access each database or xml access method
**
******************************************************************************/

static AjOXmlAccess xmlAccess[] =
{
  /* Name      AccessFunction   FreeFunction
     Qlink    Description
     Alias    Entry    Query    All      Chunk */
    {NULL, NULL, NULL,
     NULL, NULL,
     AJFALSE, AJFALSE, AJFALSE, AJFALSE, AJFALSE},
    {NULL, NULL, NULL,
     NULL, NULL,
     AJFALSE, AJFALSE, AJFALSE, AJFALSE, AJFALSE},
};




/* @func ajXmldbInit ********************************************************
**
** Initialise xml database internals
**
** @return [void]
******************************************************************************/

void ajXmldbInit(void)
{
    AjPTable table;
    ajuint i = 0;

    table = ajXmlaccessGetDb();

    while(xmlAccess[i].Name)
    {
        ajCallTableRegister(table, xmlAccess[i].Name,
                            (void*) &xmlAccess[i]);
	i++;
    }

    return;
}




/* @func ajXmldbPrintAccess **************************************************
**
** Reports the internal data structures
**
** @param [u] outf [AjPFile] Output file
** @param [r] full [AjBool] Full report (usually ajFalse)
** @return [void]
** @@
******************************************************************************/

void ajXmldbPrintAccess(AjPFile outf, AjBool full)
{
    ajint i = 0;

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# Xml access methods\n");
    ajFmtPrintF(outf, "# Name       Alias Entry Query   All Description\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "method {\n");

    for(i=0; xmlAccess[i].Name; i++)
	if(full || !xmlAccess[i].Alias)
	    ajFmtPrintF(outf, "  %-10s %5B %5B %5B %5B \"%s\"\n",
			xmlAccess[i].Name,  xmlAccess[i].Alias,
			xmlAccess[i].Entry, xmlAccess[i].Query,
			xmlAccess[i].All,   xmlAccess[i].Desc);

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajXmldbExit ********************************************************
**
** Cleans up xml database processing internal memory
**
** @return [void]
** @@
******************************************************************************/

void ajXmldbExit(void)
{
    return;
}

/* @source cachedas application
 **
 ** Prepares EMBOSS cache file for DAS servers or for the DAS registry.
 **
 **
 ** This program is free software; you can redistribute it and/or
 ** modify it under the terms of the GNU General Public License
 ** as published by the Free Software Foundation; either version 2
 ** of the License, or (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program; if not, write to the Free Software
 ** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 ******************************************************************************/

#include "emboss.h"
#include "ajdas.h"
#include "ajfeatread.h"




static AjPTable     cachedasGetTitleCount(const AjPDasServer server);




/* @prog cachedas *************************************************************
 **
 ** Prepares EMBOSS cache file for DAS servers or for the DAS registry.
 **
 *****************************************************************************/

int main(int argc, char **argv)
{
    AjPDasServer server   = NULL;
    AjPDasSource source   = NULL;

    AjPStr host  = NULL;
    AjPStr path  = NULL;
    AjPFile outf = NULL;
    AjPFile cachef = NULL;

    ajint port = 80;

    AjBool sequencesourcesonly;
    AjBool quickexit = ajFalse;

    AjIList iter     = NULL;
    AjPFilebuff buff = NULL;
    AjPStr url       = NULL;

    AjPStr dbhttpver = ajStrNew();
    AjPStr dbname    = ajStrNew();
    AjPStr dbproxy   = ajStrNew();

    AjPStr servername   = NULL;
    AjPTable titlecount = NULL;

    embInit("cachedas", argc, argv);

    host = ajAcdGetString("host");
    path = ajAcdGetString("path");
    port = ajAcdGetInt("port");
    sequencesourcesonly = ajAcdGetBoolean("sequencesourcesonly");

    servername = ajAcdGetString("servername");

    outf   = ajAcdGetOutfile("outfile");
    cachef = ajAcdGetOutfile("cachefile");

    server = ajDasServerNew();

    if(cachef)
    {
	url = ajStrNew();

	if(!ajNamServer(servername))
	{
	    ajWarn("following das server is required to be defined "
		    "for test queries...");
	    ajWarn("\nSERVER %S [\n"
		    "   type: \"sequence\"\n"
		    "   method: \"das\"\n"
		    "   url: \"http://%S%S\"\n"
		    "]\n",servername, host,path);
	    ajWarn("ignoring -runtestqueries option...");
	}
	else
	{
	    ajNamSvrGetUrl(servername, &url);
	    ajHttpUrlDeconstruct(url, &port, &host, &path);

	    ajStrDel(&url);
	}
    }

    ajDasServerSethostS(server,host);
    ajDasServerSetport(server,port);


    if(ajStrGetCharLast(path)!='/')
	ajStrAppendK(&path,'/');

    ajStrAppendC(&path,"sources");

    ajDasServerSetpathS(server,path);

    ajFmtPrintF(outf,"host = %S\npath = %S\nport = %d\n",
	    server->host, server->path, server->port);

    buff = ajHttpRead(dbhttpver, dbname, dbproxy, host, port, path);

    if(!buff)
	ajExitBad();

    ajFilebuffHtmlNoheader(buff);

    ajDasParseRegistry(buff, server->sources);
    ajFmtPrintF(outf,"DAS sources and descriptions\n\n");

    titlecount = cachedasGetTitleCount(server);

    if(cachef)
	ajDasPrintCachefile(server, cachef);

    iter = ajListIterNew(server->sources);

    while(!ajListIterDone(iter) && !quickexit)
    {
	source = ajListIterGet(iter);


	if ((sequencesourcesonly && !source->sequence))
	    continue;

	ajFmtPrintF(outf,"%-30S %-50S\n%S\n",source->uri,source->title,
		source->description);
    }

    ajListIterDel(&iter);

    ajDasServerDel(&server);
    ajFilebuffDel(&buff);

    ajStrDel(&host);
    ajStrDel(&path);
    ajStrDel(&servername);

    ajStrDel(&dbhttpver);
    ajStrDel(&dbname);
    ajStrDel(&dbproxy);

    ajFileClose(&outf);
    ajFileClose(&cachef);

    ajTableDelValdel(&titlecount, ajMemFree);

    embExit();

    return 0;
}




/* @funcstatic cachedasGetTitleCount ******************************************
**
** Returns record of in how many different sources titles are used.
**
** Note: if a title is used more than once 'source' 'uri' attributes
** are also used to get DB names to make sure they are unique.
**
** @param [r] server [const AjPDasServer] DAS server obj
** @return [AjPTable] table of usage count for each title
** @@
******************************************************************************/

static AjPTable cachedasGetTitleCount(const AjPDasServer server)
{
    AjIList iter  = NULL;

    AjPTable titlecount = NULL;
    AjPDasSource source = NULL;

    AjPStr title = NULL;

    ajuint* count = NULL;

    titlecount = ajTablestrNewCaseConst(ajListGetLength(server->sources)+20);


    iter = ajListIterNew(server->sources);

    while(!ajListIterDone(iter))
    {
	source = ajListIterGet(iter);

	title = source->title;

	count = ajTableFetchmodS(titlecount,title);

	if (count != NULL)
        {
	    (*count)++;
        }
	else
	{
	    AJNEW(count);
	    *count = 1;
            ajTablePut(titlecount, title, (void*)count);
	}

    }

    ajListIterDel(&iter);

    return titlecount;
}

/* @source  application
**
** Displays information on the currently available servers
**
** @author Copyright (C) Peter Rice (pmr@ebi.ac.uk)
** @@
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



static void showserverWidth (const AjPStr svrname,
                             const AjPStr scope, const AjPStr type,
                             const AjPStr methods, ajuint count,
                             const AjPStr defined, const AjPStr version,
                             const AjPStr cachefile, const AjPStr url,
                             ajuint *maxname, ajuint *maxscope,
                             ajuint *maxtype, ajuint *maxmethod,
                             ajuint *maxcount, ajuint *maxfield,
                             ajuint* maxdefined, ajuint *maxversion,
                             ajuint *maxcachefile, ajuint *maxurl);
static void showserverHead (AjPFile outfile, AjBool html,
                            AjBool doscope, AjBool dotype,
                            AjBool doid, AjBool doqry, AjBool doall,
                            AjBool domethod, AjBool docount,
                            AjBool dofields, AjBool dodefined,
                            AjBool docomment, AjBool doversion,
                            AjBool docachefile, AjBool dourl,
                            ajuint maxname, ajuint maxscope, ajuint maxtype,
                            ajuint maxmethod, ajuint maxcount, ajuint maxfield,
                            ajuint maxdefined, ajuint maxversion,
                            ajuint maxcachefile, ajuint maxurl);

static void   showserverOut(AjPFile outfile,
                            const AjPStr svrname,
                            const AjPStr scope, const AjPStr type,
                            AjBool id, AjBool qry, AjBool all,
                            const AjPStr methods, ajuint count,
                            const AjPStr defined,
                            const AjPStr comment, const AjPStr version,
                            const AjPStr cachefile, const AjPStr url,
                            AjBool html, AjBool doscope, AjBool dotype,
                            AjBool doid, AjBool doqry, AjBool doall,
                            AjBool domethod, AjBool docount,
                            AjBool dofields, AjBool dodefined,
                            AjBool docomment, AjBool doversion,
                            AjBool docachefile, AjBool dourl,
                            ajuint maxname, ajuint maxscope,
                            ajuint maxtype, ajuint maxmethod, ajuint maxcount,
                            ajuint maxfield, ajuint maxdefined,
                            ajuint maxversion, ajuint maxcachefile,
                            ajuint maxurl);

static void   showserverGetFields(const AjPStr svrname, AjPStr* fields);
static int    showserverSortDefined(const void* str1, const void* str2);
static int    showserverSortType(const void* str1, const void* str2);




/* @prog showserver ***********************************************************
**
** Displays information on the currently available servers
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPStr sortby;
    AjPStr *showtypes;
    AjBool html;
    AjBool doheader;
    AjBool doscope;
    AjBool dotype;
    AjBool doid;
    AjBool doqry;
    AjBool doall;
    AjBool domethod;
    AjBool docount;
    AjBool dofields;
    AjBool dodefined;
    AjBool doversion;
    AjBool docachefile;
    AjBool dourl;
    AjBool docomment;

    AjPFile outfile = NULL;
    AjPStr onesvrname   = NULL;		/* user-specified single server */
    AjPStr nextsvrname   = NULL;	/* the next server name to look at */
    AjPStr type     = NULL;
    AjPStr scope    = NULL;
    AjBool id;
    AjBool qry;
    AjBool all;
    AjPStr methods = NULL;
    AjPStr defined = NULL;
    AjPStr version = NULL;
    AjPStr cachefile = NULL;
    AjPStr cachedir  = NULL;
    AjPStr url = NULL;
    AjPStr comment = NULL;
    ajuint count = 0;

    AjPList svrnames = NULL;
    AjIList iter = NULL;

    ajuint maxname = 14;
    ajuint maxmethod = 6;
    ajuint maxcount = 5;
    ajuint maxfield = 6;
    ajuint maxscope = 5;
    ajuint maxtype = 4;
    ajuint maxdefined = 7;
    ajuint maxversion = 7;
    ajuint maxcachefile = 9;
    ajuint maxurl = 3;

    AjPTable showtable = NULL;
    AjBool showtype = ajFalse;
    AjBool doshow = ajTrue;

    int (*sortfunc) (const void* str1, const void* str2);

    AjPStrTok handle = NULL;
    AjPStr nexttype = NULL;

    ajuint i;

    ajNamSetControl("namvalid");	/* validate server defs */

    embInit("showserver", argc, argv);

    onesvrname  = ajAcdGetString("servername");
    outfile = ajAcdGetOutfile("outfile");

    html      = ajAcdGetBoolean("html");
    showtypes = ajAcdGetList("showtypes");
    sortby    = ajAcdGetListSingle("sortby");

    doheader  = ajAcdGetBoolean("heading");
    doscope   = ajAcdGetBoolean("scope");
    dotype    = ajAcdGetBoolean("type");
    doid      = ajAcdGetBoolean("id");
    doqry     = ajAcdGetBoolean("query");
    doall     = ajAcdGetBoolean("all");
    dofields  = ajAcdGetBoolean("fields");
    dodefined = ajAcdGetBoolean("defined");
    domethod  = ajAcdGetBoolean("methods");
    docount   = ajAcdGetBoolean("count");
    docomment = ajAcdGetBoolean("comment");
    doversion = ajAcdGetBoolean("serverversion");
    docachefile = ajAcdGetBoolean("cachefile");
    dourl     = ajAcdGetBoolean("url");
    
    svrnames  = ajListstrNew();
    
    if(ajStrMatchC(sortby, "defined"))
        sortfunc = &showserverSortDefined;
    else if(ajStrMatchC(sortby, "type"))
        sortfunc = &showserverSortType;
    else
        sortfunc = NULL;

    showtable = ajTablestrNew(10);
    for(i=0; showtypes[i]; i++)
        ajTablePut(showtable, ajStrNewS(showtypes[i]), &doshow);

/* start the HTML table */
    if(html)
	ajFmtPrintF(outfile, "<table border cellpadding=4 bgcolor="
		    "\"#FFFFF0\">\n");
    
    
    /* Just one specified name to get details on? */
    if(ajStrGetLen(onesvrname))
    {
	if(ajNamSvrDetails(onesvrname, &type, &scope, &id, &qry, &all, &comment,
                           &version, &methods, &defined, &cachedir,
                           &cachefile, &url))
        {
            count = ajNamSvrCount(onesvrname);
	    showserverWidth(onesvrname, scope, type, methods, count,
                            defined, version, cachefile, url,
                            &maxname, &maxscope, &maxtype, &maxmethod,
                            &maxcount, &maxfield, &maxdefined, &maxversion,
                            &maxcachefile, &maxurl);
        }

	/* print the header information */
        if(doheader)
        {
            showserverHead(outfile, html, doscope, dotype, doid, doqry,
                           doall, domethod, docount, dofields, dodefined,
                           docomment, doversion, docachefile, dourl,
                           maxname, maxscope, maxtype, maxmethod,
                           maxcount, maxfield,
                           maxdefined, maxversion, maxcachefile, maxurl);
    
            showserverOut(outfile, onesvrname, scope, type, id, qry, all,
                          methods, count, defined,
                          comment, version, cachefile, url, html,
                          doscope, dotype, doid, doqry, doall, 
                          domethod, docount, dofields, dodefined,
                          docomment, doversion, docachefile, dourl,
                          maxname, maxscope, maxtype,
                          maxmethod, maxcount, maxfield, maxdefined,
                          maxversion, maxcachefile, maxurl);
	}
	else
	    ajFatal("The server '%S' does not exist", onesvrname);
    }
    else
    {
	/* get the list of server names */
	ajNamListListServers(svrnames);

	/* sort it */
	/*ajListSort(svrnames, &ajStrCmp);*/
        if(sortfunc)
            ajListSort(svrnames, sortfunc);

	/* iterate through the svrnames list */
	iter = ajListIterNewread(svrnames);

	maxname = 14;
	maxmethod = 6;
	maxfield = 6;
	maxtype = 4;
	maxversion = 7;

	/* find the field widths */
	while((nextsvrname = ajListIterGet(iter)) != NULL)
	{
	    if(ajNamSvrDetails(nextsvrname, &type, &scope,
                               &id, &qry, &all, &comment,
                               &version, &methods, &defined,
                               &cachedir, &cachefile, &url))
            {
                count = ajNamSvrCount(nextsvrname);
                showserverWidth(nextsvrname, scope, type, methods, count,
                                defined, version, cachefile, url,
                                &maxname, &maxscope, &maxtype, &maxmethod,
                                &maxcount, &maxfield, &maxdefined,
                                &maxversion, &maxcachefile, &maxurl);
            }
	}

	/* print the header information */
	if(doheader)
	    showserverHead(outfile, html, doscope, dotype, doid, doqry,
                           doall, domethod, docount, dofields, dodefined,
                           docomment, doversion, docachefile, dourl,
                           maxname, maxscope, maxtype, maxmethod,
                           maxcount, maxfield,
                           maxdefined, maxversion, maxcachefile, maxurl);
    
	ajListIterDel(&iter);

	/* iterate through the svrnames list */
	iter = ajListIterNewread(svrnames);

	/* write out servers */
	while((nextsvrname = ajListIterGet(iter)) != NULL)
	    if(ajNamSvrDetails(nextsvrname, &type,  &scope,
                               &id, &qry, &all, &comment,
                               &version, &methods, &defined,
                               &cachedir, &cachefile, &url))
	    {
                showtype = ajFalse;
                count = ajNamSvrCount(nextsvrname);
                ajStrTokenAssignC(&handle, type, " ,;");

                while(ajStrTokenNextParse(&handle, &nexttype))
                    if(ajTableMatchS(showtable,nexttype))
                        showtype = ajTrue;

                ajStrDel(&nexttype);
                ajStrTokenDel(&handle);
                
                if(showtype)
                    showserverOut(outfile, nextsvrname, scope, type,
                                  id, qry, all,
                                  methods, count, defined,
                                  comment, version, cachefile, url,
                                  html, doscope, dotype, doid,
                                  doqry, doall, domethod,
                                  docount, dofields, dodefined,
                                  docomment, doversion, docachefile, dourl,
                                  maxname, maxscope, maxtype, maxmethod,
                                  maxcount, maxfield, maxdefined,
                                  maxversion, maxcachefile, maxurl);
	    }
	    else
		ajFatal("The server '%S' does not exist", nextsvrname);


	/* reset the iterator */
	ajListIterDel(&iter);

    }
    
    /* end the HTML table */
    if(html)
	ajFmtPrintF(outfile, "</table>\n");
    
    ajFileClose(&outfile);
    ajStrDel(&onesvrname);
    ajStrDel(&type);
    ajStrDel(&scope);
    ajStrDel(&methods);
    ajStrDel(&defined);
    ajStrDel(&cachedir);
    ajStrDel(&cachefile);
    ajStrDel(&url);
    ajStrDel(&version);
    ajStrDel(&comment);
    ajStrDel(&sortby);

    ajListstrFree(&svrnames);

    ajTablestrFreeKey(&showtable);

    ajStrDelarray(&showtypes);

    embExit();

    return 0;
}




/* @funcstatic showserverWidth ************************************************
**
** Update maximum width for variable length text
**
** @param [r] svrname [const AjPStr] server name
** @param [r] scope [const AjPStr] server scope
** @param [r] type [const AjPStr] server type
** @param [r] methods [const AjPStr] server access method(s)
** @param [r] count [ajuint] Database count
** @param [r] defined [const AjPStr] server definition file short name
** @param [r] version [const AjPStr] server version number
** @param [r] cachefile [const AjPStr] server cachefile name
** @param [r] url [const AjPStr] server URL
** @param [u] maxname [ajuint*] Maximum width for name
** @param [u] maxtype [ajuint*] Maximum width for type
** @param [u] maxscope [ajuint*] Maximum width for list of scopes
** @param [u] maxmethod [ajuint*] Maximum width for list of access methods
** @param [u] maxcount [ajuint*] Maximum width for database count
** @param [u] maxfield [ajuint*] Maximum width for list of fields
** @param [u] maxdefined [ajuint*] Maximum width for definition file
** @param [u] maxversion [ajuint*] Maximum width for version number
** @param [u] maxcachefile [ajuint*] Maximum width for cachefile name
** @param [u] maxurl [ajuint*] Maximum width for URL
******************************************************************************/

static void showserverWidth (const AjPStr svrname, const AjPStr scope, 
                             const AjPStr type, const AjPStr methods,
                             ajuint count,
                             const AjPStr defined, const AjPStr version,
                             const AjPStr cachefile, const AjPStr url,
                             ajuint *maxname, ajuint* maxtype,
                             ajuint* maxscope, ajuint *maxmethod,
                             ajuint *maxcount,
                             ajuint *maxfield, ajuint* maxdefined,
                             ajuint *maxversion, ajuint *maxcachefile,
                             ajuint *maxurl)
{
    AjPStr fields = NULL;
    ajuint i;
    ajuint testcount;
    ajuint testwidth = 0;

    if (ajStrGetLen(svrname) > *maxname)
	*maxname = ajStrGetLen(svrname);

    if (ajStrGetLen(scope) > *maxscope)
	*maxscope = ajStrGetLen(scope);

    if (ajStrGetLen(type) > *maxtype)
	*maxtype = ajStrGetLen(type);

    if (ajStrGetLen(methods) > *maxmethod)
	*maxmethod = ajStrGetLen(methods);

    testcount = count;
    if(!count)
        testcount++;
    for(testcount = count; testcount > 0; testcount /= 10)
        testwidth++;
    if(testwidth > *maxcount)
        *maxcount = testwidth;

    if (ajStrGetLen(defined) > *maxdefined)
	*maxdefined = ajStrGetLen(defined);

    if (ajStrGetLen(version) > *maxversion)
	*maxversion = ajStrGetLen(version);

    if (ajStrGetLen(cachefile) > *maxcachefile)
	*maxcachefile = ajStrGetLen(cachefile);

    if (ajStrGetLen(url) > *maxurl)
	*maxurl = ajStrGetLen(url);

    showserverGetFields(svrname, &fields);
    i = ajStrGetLen(fields);

    if (i > *maxfield)
	*maxfield = i;

    ajStrDel(&fields);

    return;
}




/* @funcstatic showserverHead *************************************************
**
** Output header for server information
**
** @param [w] outfile [AjPFile] outfile
** @param [r] html [AjBool] do html
** @param [r] doscope [AjBool] show scope
** @param [r] dotype [AjBool] show type
** @param [r] doid [AjBool] show id
** @param [r] doqry [AjBool] show query status
** @param [r] doall [AjBool] show everything
** @param [r] domethod [AjBool] show access method(s)
** @param [r] docount [AjBool] show database count
** @param [r] dofields [AjBool] show query fields
** @param [r] dodefined [AjBool] show definition file
** @param [r] docomment [AjBool] show comment
** @param [r] doversion [AjBool] show version
** @param [r] docachefile [AjBool] show cachefile name
** @param [r] dourl [AjBool] show URL
** @param [r] maxname [ajuint] Maximum width for name
** @param [r] maxscope [ajuint] Maximum width for scope
** @param [r] maxtype [ajuint] Maximum width for type
** @param [r] maxmethod [ajuint] Maximum width for access method
** @param [r] maxcount [ajuint] Maximum width for databas count
** @param [r] maxfield [ajuint] Maximum width for list of fields
** @param [r] maxdefined [ajuint] Maximum width for definition file
** @param [r] maxversion [ajuint] Maximum width for version number
** @param [r] maxcachefile [ajuint] Maximum width for cachefile name
** @param [r] maxurl [ajuint] Maximum width for URL
******************************************************************************/

static void showserverHead (AjPFile outfile, AjBool html,
                            AjBool doscope, AjBool dotype,
                            AjBool doid, AjBool doqry, AjBool doall,
                            AjBool domethod, AjBool docount,
                            AjBool dofields, AjBool dodefined,
                            AjBool docomment, AjBool doversion,
                            AjBool docachefile, AjBool dourl,
                            ajuint maxname, ajuint maxscope,
                            ajuint maxtype, ajuint maxmethod, ajuint maxcount,
                            ajuint maxfield, ajuint maxdefined,
                            ajuint maxversion, ajuint maxcachefile,
                            ajuint maxurl)
{

    if(html)
	/* start the HTML table title line and output the Name header */
	ajFmtPrintF(outfile, "<tr><th>Name</th>");
    else
	ajFmtPrintF(outfile, "# Name%*s", maxname-5, " ");

    if(doscope)
    {
	if(html)
	    ajFmtPrintF(outfile, "<th>Scope</th>");
	else
	    ajFmtPrintF(outfile, "Scope%*s", maxscope-4, " ");
    }

    if(dotype)
    {
	if(html)
	    ajFmtPrintF(outfile, "<th>Type</th>");
	else
	    ajFmtPrintF(outfile, "Type%*s", maxtype-3, " ");
    }

    if(doid)
    {
	if(html)
	    ajFmtPrintF(outfile, "<th>ID</th>");
	else
	    ajFmtPrintF(outfile, "ID  ");
    }

    if(doqry)
    {
	if(html)
	    ajFmtPrintF(outfile, "<th>Qry</th>");
	else
	    ajFmtPrintF(outfile, "Qry ");
    }

    if(doall)
    {
	if(html)
	    ajFmtPrintF(outfile, "<th>All</th>");
	else
	    ajFmtPrintF(outfile, "All ");
    }

    if(domethod)
    {
	if(html)
	    ajFmtPrintF(outfile, "<th>Method</th>");
	else
	    ajFmtPrintF(outfile, "Method%*s", maxmethod-5, " ");
    }

    if(docount)
    {
	if(html)
	    ajFmtPrintF(outfile, "<th>Count</th>");
	else
	    ajFmtPrintF(outfile, "Count%*s", maxcount-4, " ");
    }

    if(docachefile)
    {
	if(html)
	    ajFmtPrintF(outfile, "<th>Cachefile</th>");
	else
	    ajFmtPrintF(outfile, "Cachefile%*s", maxcachefile-8, " ");
    }

    if(dourl)
    {
	if(html)
	    ajFmtPrintF(outfile, "<th>URL</th>");
	else
	    ajFmtPrintF(outfile, "URL%*s", maxurl-2, " ");
    }

    if(dofields)
    {
	if(html)
	    ajFmtPrintF(outfile, "<th>Fields</th>");
	else
	    ajFmtPrintF(outfile, "Fields%*s", maxfield-5, " ");
    }

    if(dodefined)
    {
	if(html)
	    ajFmtPrintF(outfile, "<th>Defined</th>");
	else
	    ajFmtPrintF(outfile, "Defined%*s", maxdefined-6, " ");
    }

    if(doversion)
    {
	if(html)
	    ajFmtPrintF(outfile, "<th>Version</th>");
	else
	    ajFmtPrintF(outfile, "Version%*s", maxversion-6, " ");
    }

    if(docomment)
    {
	if(html)
	    ajFmtPrintF(outfile, "<th>Comment</th>");
	else
	    ajFmtPrintF(outfile, "Comment");
    }

    if(html)
	/* end the HTML table title line */
	ajFmtPrintF(outfile, "</tr>\n");
    else
    {
	ajFmtPrintF(outfile, "\n");
	
	ajFmtPrintF(outfile, "# %.*s ", maxname-2,
		    "=====================================================");

	if(dotype)
	    ajFmtPrintF(outfile, "%.*s ", maxtype,
			"==================================================");

	if(doscope)
	    ajFmtPrintF(outfile, "%.*s ", maxscope,
			"==================================================");

	if(doid)
	    ajFmtPrintF(outfile, "==  ");

	if(doqry)
	    ajFmtPrintF(outfile, "=== ");

	if(doall)
	    ajFmtPrintF(outfile, "=== ");

	if(domethod)
	    ajFmtPrintF(outfile, "%.*s ", maxmethod,
			"==================================================");

	if(docount)
	    ajFmtPrintF(outfile, "%.*s ", maxcount,
			"==================================================");

	if(docachefile)
	    ajFmtPrintF(outfile, "%.*s ", maxcachefile,
			"==================================================");

	if(dourl)
	    ajFmtPrintF(outfile, "%.*s ", maxurl,
			"=================================================="
                        "==================================================");

	if(dofields)
	    ajFmtPrintF(outfile, "%.*s ", maxfield,
			"==================================================");

	if(dodefined)
	    ajFmtPrintF(outfile, "%.*s ", maxdefined,
			"==================================================");

	if(doversion)
	    ajFmtPrintF(outfile, "%.*s ", maxversion,
			"==================================================");

	if(docomment)
	    ajFmtPrintF(outfile, "=======");

	ajFmtPrintF(outfile, "\n");
    }
    return;
}




/* @funcstatic showserverOut **************************************************
**
** Output server information
**
** @param [w] outfile [AjPFile] outfile
** @param [r] svrname [const AjPStr] server name
** @param [r] scope [const AjPStr] scope
** @param [r] type [const AjPStr] type
** @param [r] id [AjBool] id
** @param [r] qry [AjBool] queryable
** @param [r] all [AjBool] all info
** @param [r] methods [const AjPStr] server access method(s)
** @param [r] count [ajuint] database count
** @param [r] defined [const AjPStr] server definition file short name
** @param [r] comment [const AjPStr] server comment
** @param [r] version [const AjPStr] server version
** @param [r] cachefile [const AjPStr] server cachefile name
** @param [r] url [const AjPStr] server URL
** @param [r] html [AjBool] do html
** @param [r] doscope [AjBool] show scope
** @param [r] dotype [AjBool] show type
** @param [r] doid [AjBool] show id
** @param [r] doqry [AjBool] show query status
** @param [r] doall [AjBool] show everything
** @param [r] domethod [AjBool] show access method(s)
** @param [r] docount [AjBool] show database count
** @param [r] dofields [AjBool] show query fields
** @param [r] dodefined [AjBool] show access method(s)
** @param [r] docomment [AjBool] show comment
** @param [r] doversion [AjBool] show version
** @param [r] docachefile [AjBool] show cachefile
** @param [r] dourl [AjBool] show URL
** @param [r] maxname [ajuint] Maximum width for name
** @param [r] maxscope [ajuint] Maximum width for scope
** @param [r] maxtype [ajuint] Maximum width for type
** @param [r] maxmethod [ajuint] Maximum width for list of access methods
** @param [r] maxcount [ajuint] Maximum width for database count
** @param [r] maxfield [ajuint] Maximum width for list of fields
** @param [r] maxdefined [ajuint] Maximum width for definition file
** @param [r] maxversion [ajuint] Maximum width for version number
** @param [r] maxcachefile [ajuint] Maximum width for cachefile name
** @param [r] maxurl [ajuint] Maximum width for URL
** @@
******************************************************************************/

static void showserverOut(AjPFile outfile,
                          const AjPStr svrname, const AjPStr scope,
                          const AjPStr type,
                          AjBool id, AjBool qry, AjBool all,
                          const AjPStr methods, ajuint count,
                          const AjPStr defined,
                          const AjPStr comment, const AjPStr version,
                          const AjPStr cachefile, const AjPStr url,
                          AjBool html, AjBool doscope, AjBool dotype,
                          AjBool doid, AjBool doqry, AjBool doall,
                          AjBool domethod, AjBool docount,
                          AjBool dofields, AjBool dodefined,
                          AjBool docomment, AjBool doversion,
                          AjBool docachefile, AjBool dourl,
                          ajuint maxname, ajuint maxscope,
                          ajuint maxtype, ajuint maxmethod, ajuint maxcount,
                          ajuint maxfield, ajuint maxdefined,
                          ajuint maxversion, ajuint maxcachefile,
                          ajuint maxurl)
{
    AjPStr fields = NULL;

    if(html)
	/* start table line and output name */
	ajFmtPrintF(outfile, "<tr><td>%S</td>", svrname);
    else
	ajFmtPrintF(outfile, "%-*S ", maxname, svrname);

    if(doscope)
    {
	if(html)
	    ajFmtPrintF(outfile, "<td>%S</td>", scope);
	else
	    ajFmtPrintF(outfile, "%-*S ", maxscope, scope);
    }

    if(dotype)
    {
	if(html)
	    ajFmtPrintF(outfile, "<td>%S</td>", type);
	else
	    ajFmtPrintF(outfile, "%-*S ", maxtype, type);
    }

    if(doid)
    {
	if(html)
	    ajFmtPrintF(outfile, "<td>");

	if(id)
	    ajFmtPrintF(outfile, "%s", "OK  ");
	else
	    ajFmtPrintF(outfile, "%s", "-   ");

	if(html)
	    ajFmtPrintF(outfile, "</td>");
    }

    if(doqry)
    {
	if(html)
	    ajFmtPrintF(outfile, "<td>");

	if(qry)
	    ajFmtPrintF(outfile, "%s", "OK  ");
	else
	    ajFmtPrintF(outfile, "%s", "-   ");

	if(html)
	    ajFmtPrintF(outfile, "</td>");
    }

    if(doall)
    {
	if(html)
	    ajFmtPrintF(outfile, "<td>");

	if(all)
	    ajFmtPrintF(outfile, "%s", "OK  ");
	else
	    ajFmtPrintF(outfile, "%s", "-   ");

	if(html)
	    ajFmtPrintF(outfile, "</td>");
    }

    if(domethod)
    {
	if(html)
	    ajFmtPrintF(outfile, "<td>");

        ajFmtPrintF(outfile, "%-*S ", maxmethod, methods);

	if(html)
	    ajFmtPrintF(outfile, "</td>");
    }

    if(docount)
    {
	if(html)
	    ajFmtPrintF(outfile, "<td>");

        ajFmtPrintF(outfile, "%*d ", maxcount, count);

	if(html)
	    ajFmtPrintF(outfile, "</td>");
    }

    if(docachefile)
    {
	if(html)
	    ajFmtPrintF(outfile, "<td>");

	if(cachefile != NULL)
	    ajFmtPrintF(outfile, "%-*S ", maxcachefile, cachefile);
	else
	    ajFmtPrintF(outfile, "%-*s ", maxcachefile, "");

	if(html)
	    ajFmtPrintF(outfile, "</td>");
    }


    if(dourl)
    {
	if(html)
	    ajFmtPrintF(outfile, "<td>");

	if(cachefile != NULL)
	    ajFmtPrintF(outfile, "%-*S ", maxurl, url);
	else
	    ajFmtPrintF(outfile, "%-*s ", maxurl, "");

	if(html)
	    ajFmtPrintF(outfile, "</td>");
    }


    if(dofields)
    {
	if(html)
	    ajFmtPrintF(outfile, "<td>");

        showserverGetFields(svrname, &fields);
	ajFmtPrintF(outfile, "%-*S ", maxfield, fields);

	if(html)
	    ajFmtPrintF(outfile, "</td>");
    }

    if(dodefined)
    {
	if(html)
	    ajFmtPrintF(outfile, "<td>");

        ajFmtPrintF(outfile, "%-*S ", maxdefined, defined);

	if(html)
	    ajFmtPrintF(outfile, "</td>");
    }

    if(doversion)
    {
	if(html)
	    ajFmtPrintF(outfile, "<td>");

	if(version != NULL)
	    ajFmtPrintF(outfile, "%-*S ", maxversion, version);
	else
	    ajFmtPrintF(outfile, "%-*s ", maxversion, "");

	if(html)
	    ajFmtPrintF(outfile, "</td>");
    }


    if(docomment)
    {
	if(html)
	    ajFmtPrintF(outfile, "<td>");

	if(comment != NULL)
	    ajFmtPrintF(outfile, "%S", comment);
	else
	    ajFmtPrintF(outfile, "-");

	if(html)
	    ajFmtPrintF(outfile, "</td>");
    }

    if(html)
	ajFmtPrintF(outfile, "</tr>\n");	/* end table line */
    else
	ajFmtPrintF(outfile, "\n");

    ajStrDel(&fields);

    return;
}




/* @funcstatic showserverGetFields ********************************************
**
** Get a server's valid query fields (apart from the default 'id' and 'acc')
**
** @param [r] svrname [const AjPStr] server name
** @param [w] fields [AjPStr*] the available search fields
** @return [void]
** @@
******************************************************************************/

static void showserverGetFields(const AjPStr svrname, AjPStr* fields)
{
    AjPQuery query;

    query = ajQueryNew(AJDATATYPE_UNKNOWN);

    ajStrAssignS(&query->SvrName, svrname);
    ajNamSvrData(query,0);
    ajStrAssignS(fields, query->DbFields);

    /* if there are no query fields, then change to a '_' */
    if(!ajStrGetLen(*fields))
  	ajStrAssignC(fields, "-     ");
    else
	/* change spaces to commas to make the result one word */
	ajStrExchangeSetCC(fields, " ", ",");

    ajQueryDel(&query);
    return;
}




/* @funcstatic showserverSortDefined *****************************************
**
** Compares the value of two strings for use in sorting (e.g. ajListSort)
**
** @param [r] str1 [const void*] First string
** @param [r] str2 [const void*] Second string
** @return [int] -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
** @category use [AjPStr] String compare
** @@
******************************************************************************/

static int showserverSortDefined(const void* str1, const void* str2)
{
    const AjPStr svr1 = *(AjPStr const *) str1;
    const AjPStr svr2 = *(AjPStr const *) str2;

    AjPStr methods = NULL;
    AjPStr version = NULL;
    AjPStr cachefile = NULL;
    AjPStr cachedir  = NULL;
    AjPStr url = NULL;
    AjPStr comment = NULL;
    AjPStr type    = NULL;
    AjPStr scope    = NULL;
    AjBool id;
    AjBool qry;
    AjBool all;
    AjPStr defined1 = NULL;
    AjPStr defined2 = NULL;
    AjBool ok;

    int ret;

    ok = ajNamSvrDetails(svr1, &type, &scope,
                         &id, &qry, &all, &comment,
                         &version, &methods, &defined1, &cachedir,
                         &cachefile, &url);
    if(ok)
	ok = ajNamSvrDetails(svr2, &type, &scope,
                             &id, &qry, &all, &comment,
                             &version, &methods, &defined2, &cachedir,
                             &cachefile, &url);
    ajStrDel(&type);
    ajStrDel(&scope);
    ajStrDel(&comment);
    ajStrDel(&version);
    ajStrDel(&methods);
    ajStrDel(&url);
    ajStrDel(&cachedir);
    
    if(ok)
    {
	ret = ajStrCmpS(defined1, defined2);
	ajDebug("Sorting1 %S:%S %S:%S %d\n",
                svr1, svr2, defined1, defined2, ret);
	ajStrDel(&defined1);
	ajStrDel(&defined2);

	if(ret)
            return ret;
    }

    ajStrDel(&defined1);
    ajStrDel(&defined2);

    ret = ajStrCmpS(svr1, svr2);
    ajDebug("Sorting2 %S:%S %d\n", svr1, svr2, ret);

    return ret;
}




/* @funcstatic showserverSortType *********************************************
**
** Compares the value of two strings for use in sorting (e.g. ajListSort)
**
** @param [r] str1 [const void*] First string
** @param [r] str2 [const void*] Second string
** @return [int] -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
** @category use [AjPStr] String compare
** @@
******************************************************************************/

static int showserverSortType(const void* str1, const void* str2)
{
    const AjPStr svr1 = NULL;
    const AjPStr svr2 = NULL;

    AjPStr methods   = NULL;
    AjPStr version   = NULL;
    AjPStr cachedir  = NULL;
    AjPStr cachefile = NULL;
    AjPStr comment   = NULL;
    AjPStr url = NULL;
    AjPStr type1  = NULL;
    AjPStr type2  = NULL;
    AjPStr scope1 = NULL;
    AjPStr scope2 = NULL;
    AjBool id;
    AjBool qry;
    AjBool all;
    AjPStr defined = NULL;
    AjBool ok;

    int ret;

    svr1 = *(AjPStr const *) str1;
    svr2 = *(AjPStr const *) str2;
    
    ok = ajNamSvrDetails(svr1, &type1, &scope1,
                         &id, &qry, &all, &comment,
                         &version, &methods, &defined, &cachedir,
                         &cachefile, &url);
    if(ok)
	ok = ajNamSvrDetails(svr2, &type2, &scope2,
                             &id, &qry, &all, &comment,
                             &version, &methods, &defined, &cachedir,
                             &cachefile, &url);

    ajStrDel(&defined);
    ajStrDel(&comment);
    ajStrDel(&version);
    ajStrDel(&methods);
    ajStrDel(&cachedir);
    ajStrDel(&cachefile);
    ajStrDel(&url);

    if(ok)
    {
	ret = ajStrCmpS(scope1, scope2);
	ajDebug("Sorting1 %S:%S %S:%S %d\n",
                svr1, svr2, scope1, scope2, ret);

	if(!ret)
        {
            ret = ajStrCmpS(type1, type2);
            ajDebug("Sorting1 %S:%S %S:%S %d\n",
                    svr1, svr2, type1, type2, ret);
        }

	if(ret)
        {
            ajStrDel(&scope1);
            ajStrDel(&scope2);
            ajStrDel(&type1);
            ajStrDel(&type2);

            return ret;
        }
    }

    ajStrDel(&scope1);
    ajStrDel(&scope2);
    ajStrDel(&type1);
    ajStrDel(&type2);

    ret = ajStrCmpS(svr1, svr2);
    ajDebug("Sorting2 %S:%S %d\n", svr1, svr2, ret);

    return ret;
}

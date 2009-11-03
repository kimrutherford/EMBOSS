/* @source showdb application
**
** Displays information on the currently available databases
**
** @author Copyright (C) Gary Williams (gwilliam@hgmp.mrc.ac.uk)
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



static void showdbDBWidth (const AjPStr dbname, const AjPStr type,
			   const AjPStr methods, 
			   const AjPStr defined, const AjPStr release,
			   ajuint *maxname, ajuint *maxtype,
			   ajuint *maxmethod, ajuint *maxfield,
			   ajuint* maxdefined, ajuint *maxrelease);
static void showdbDBHead (AjPFile outfile, AjBool html, AjBool dotype,
			  AjBool doid, AjBool doqry, AjBool doall,
			  AjBool domethod, AjBool dofields, AjBool dodefined,
			  AjBool docomment, AjBool dorelease,
			  ajuint maxname, ajuint maxtype,
			  ajuint maxmethod, 
			  ajuint maxfield,
			  ajuint maxdefined, ajuint maxrelease);

static void   showdbDBOut(AjPFile outfile,
			  const AjPStr dbname, const AjPStr type,
			  AjBool id, AjBool qry, AjBool all,
			  const AjPStr methods, const AjPStr defined,
			  const AjPStr comment, const AjPStr release,
			  AjBool html, AjBool dotype,
			  AjBool doid, AjBool doqry, AjBool doall,
			  AjBool domethod, AjBool dofields, AjBool dodefined,
			  AjBool docomment, AjBool dorelease,
			  ajuint maxname, ajuint maxtype, ajuint maxmethod,
			  ajuint maxfield, ajuint maxdefined,
			  ajuint maxrelease);

static void   showdbGetFields(const AjPStr dbname, AjPStr* fields);
static int    showdbDBSortDefined(const void* str1, const void* str2);




/* @prog showdb ***************************************************************
**
** Displays information on the currently available databases
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjBool html;
    AjBool protein;
    AjBool nucleic;
    AjBool doheader;
    AjBool dotype;
    AjBool doid;
    AjBool doqry;
    AjBool doall;
    AjBool domethod;
    AjBool dofields;
    AjBool dodefined;
    AjBool dorelease;
    AjBool docomment;

    AjPFile outfile = NULL;
    AjPStr onedbname   = NULL;		/* user-specified single database */
    AjPStr nextdbname   = NULL;	/* the next database name to look at */
    AjPStr type     = NULL;
    AjBool id;
    AjBool qry;
    AjBool all;
    AjPStr methods = NULL;
    AjPStr defined = NULL;
    AjPStr release = NULL;
    AjPStr comment = NULL;

    AjPList dbnames;
    AjIList iter = NULL;

    ajuint maxname = 14;
    ajuint maxmethod = 6;
    ajuint maxfield = 6;
    ajuint maxtype = 4;
    ajuint maxdefined = 7;
    ajuint maxrelease = 7;

    ajNamSetControl("namvalid");	/* validate database/resource defs */

    embInit("showdb", argc, argv);

    onedbname  = ajAcdGetString("database");
    outfile = ajAcdGetOutfile("outfile");

    html    = ajAcdGetBoolean("html");
    protein = ajAcdGetBoolean("protein");
    nucleic = ajAcdGetBoolean("nucleic");

    doheader  = ajAcdGetBoolean("heading");
    dotype    = ajAcdGetBoolean("type");
    doid      = ajAcdGetBoolean("id");
    doqry     = ajAcdGetBoolean("query");
    doall     = ajAcdGetBoolean("all");
    dofields  = ajAcdGetBoolean("fields");
    dodefined = ajAcdGetBoolean("defined");
    domethod  = ajAcdGetBoolean("methods");
    docomment = ajAcdGetBoolean("comment");
    dorelease = ajAcdGetBoolean("release");
    
    dbnames = ajListstrNew();
    
    
    /* start the HTML table */
    if(html)
	ajFmtPrintF(outfile, "<table border cellpadding=4 bgcolor="
		    "\"#FFFFF0\">\n");
    
    
    /* Just one specified name to get details on? */
    if(ajStrGetLen(onedbname))
    {
	if(ajNamDbDetails(onedbname, &type, &id, &qry, &all, &comment,
			  &release, &methods, &defined))
	    showdbDBWidth(onedbname, type, methods, defined, release,
			   &maxname, &maxtype, &maxmethod,
			   &maxfield, &maxdefined, &maxrelease);

	/* print the header information */
	if(doheader)
	{
	    showdbDBHead(outfile, html, dotype, doid, doqry,
			  doall, domethod, dofields, dodefined,
			  docomment, dorelease,
			  maxname, maxtype, maxmethod, maxfield,
			  maxdefined, maxrelease);
    
    	    showdbDBOut(outfile, onedbname, type, id, qry, all,
			 methods, defined,
			 comment, release, html,
			 dotype, doid, doqry, doall, 
			 domethod, dofields, dodefined,
			 docomment, dorelease,
			 maxname, maxtype,
			 maxmethod, maxfield, maxdefined, maxrelease);
	}
	else
	    ajFatal("The database '%S' does not exist", onedbname);
    }
    else
    {
	/* get the list of database names */
	ajNamListListDatabases(dbnames);

	/* sort it */
	/*ajListSort(dbnames, ajStrCmp);*/
	ajListSort(dbnames, showdbDBSortDefined);

	/* iterate through the dbnames list */
	iter = ajListIterNewread(dbnames);

	maxname = 14;
	maxmethod = 6;
	maxfield = 6;
	maxtype = 4;
	maxrelease = 7;

	/* find the field widths */
	while((nextdbname = ajListIterGet(iter)) != NULL)
	{
	    if(ajNamDbDetails(nextdbname, &type, &id, &qry, &all, &comment,
			      &release, &methods, &defined))
	    showdbDBWidth(nextdbname, type, methods, defined, release,
			   &maxname, &maxtype, &maxmethod,
			   &maxfield, &maxdefined, &maxrelease);
	}

	/* print the header information */
	if(doheader)
	    showdbDBHead(outfile, html, dotype, doid, doqry,
			  doall, domethod, dofields, dodefined,
			  docomment, dorelease,
			  maxname, maxtype, maxmethod, maxfield,
			  maxdefined, maxrelease);
    
	ajListIterDel(&iter);

	/* iterate through the dbnames list */
	iter = ajListIterNewread(dbnames);

	/* write out protein databases */
	while((nextdbname = ajListIterGet(iter)) != NULL)
	    if(ajNamDbDetails(nextdbname, &type, &id, &qry, &all, &comment,
			      &release, &methods, &defined))
	    {
		if(protein &&
		   (ajStrMatchC(type, "P") || ajStrMatchC(type, "Protein")))
		   showdbDBOut(outfile, nextdbname, type, id, qry, all,
				 methods, defined,
				 comment, release, html, dotype, doid,
				 doqry, doall, domethod, dofields, dodefined,
				 docomment, 
				 dorelease, maxname,maxtype,  maxmethod,
				 maxfield, maxdefined, maxrelease);
	    }
	    else
		ajFatal("The database '%S' does not exist", nextdbname);


	/* reset the iterator */
	ajListIterDel(&iter);
	iter = ajListIterNewread(dbnames);

	/* now write out nucleic databases */
	while((nextdbname = ajListIterGet(iter)) != NULL)
	{
	    if(ajNamDbDetails(nextdbname, &type, &id, &qry, &all, &comment,
			      &release, &methods, &defined))
	    {
		if( nucleic &&
		   (ajStrMatchC(type, "N") || ajStrMatchC(type, "Nucleotide")))
		    showdbDBOut(outfile, nextdbname, type, id, qry, all,
				 methods, defined,
				 comment, release, html,
				 dotype, doid,
				 doqry, doall, domethod, dofields, dodefined,
				 docomment, dorelease,
				 maxname, maxtype, maxmethod,
				 maxfield, maxdefined, maxrelease);
	    }
	    else
		ajFatal("The database '%S' does not exist", nextdbname);
	}

	ajListIterDel(&iter);
	ajListFree(&dbnames);
    }
    
    /* end the HTML table */
    if(html)
	ajFmtPrintF(outfile, "</table>\n");
    
    ajFileClose(&outfile);
    ajStrDel(&onedbname);
    /*ajStrDel(&nextdbname);*/		/* points to name from list */
    ajStrDel(&type);
    ajStrDel(&methods);
    ajStrDel(&defined);
    ajStrDel(&release);
    ajStrDel(&comment);

    embExit();

    return 0;
}


/* @funcstatic showdbDBWidth **************************************************
**
** Update maximum width for variable length text
**
** @param [r] dbname [const AjPStr] database name
** @param [r] type [const AjPStr] database type
** @param [r] methods [const AjPStr] database access method(s)
** @param [r] defined [const AjPStr] database definition file short name
** @param [r] release [const AjPStr] database release number
** @param [u] maxname [ajuint*] Maximum width for name
** @param [u] maxtype [ajuint*] Maximum width for type
** @param [u] maxmethod [ajuint*] Maximum width for list of access methods
** @param [u] maxfield [ajuint*] Maximum width for list of fields
** @param [u] maxdefined [ajuint*] Maximum width for definition file
** @param [u] maxrelease [ajuint*] Maximum width for release number
******************************************************************************/

static void showdbDBWidth (const AjPStr dbname,
			   const AjPStr type, const AjPStr methods,
			   const AjPStr defined, const AjPStr release,
			   ajuint *maxname, ajuint* maxtype, ajuint *maxmethod,
			   ajuint *maxfield, ajuint* maxdefined,
			   ajuint *maxrelease)
{
    AjPStr fields = NULL;
    ajuint i;

    if (ajStrGetLen(dbname) > *maxname)
	*maxname = ajStrGetLen(dbname);

    if (ajStrGetLen(type) > *maxtype)
	*maxtype = ajStrGetLen(type);

    if (ajStrGetLen(methods) > *maxmethod)
	*maxmethod = ajStrGetLen(methods);

    if (ajStrGetLen(defined) > *maxdefined)
	*maxdefined = ajStrGetLen(defined);

    if (ajStrGetLen(release) > *maxrelease)
	*maxrelease = ajStrGetLen(release);

    showdbGetFields(dbname, &fields);
    i = ajStrGetLen(fields);
    if (i > *maxfield)
	*maxfield = i;
    ajStrDel(&fields);

    return;
}

/* @funcstatic showdbDBHead ***************************************************
**
** Output header for db information
**
** @param [w] outfile [AjPFile] outfile
** @param [r] html [AjBool] do html
** @param [r] dotype [AjBool] show type
** @param [r] doid [AjBool] show id
** @param [r] doqry [AjBool] show query status
** @param [r] doall [AjBool] show everything
** @param [r] domethod [AjBool] show access method(s)
** @param [r] dofields [AjBool] show query fields
** @param [r] dodefined [AjBool] show definition file
** @param [r] docomment [AjBool] show comment
** @param [r] dorelease [AjBool] show release
** @param [r] maxname [ajuint] Maximum width for name
** @param [r] maxtype [ajuint] Maximum width for type
** @param [r] maxmethod [ajuint] Maximum width for access method
** @param [r] maxfield [ajuint] Maximum width for list of fields
** @param [r] maxdefined [ajuint] Maximum width for definition file
** @param [r] maxrelease [ajuint] Maximum width for release number
******************************************************************************/

static void showdbDBHead (AjPFile outfile, AjBool html, AjBool dotype,
			  AjBool doid, AjBool doqry, AjBool doall,
			  AjBool domethod, AjBool dofields, AjBool dodefined,
			  AjBool docomment, AjBool dorelease,
			  ajuint maxname, ajuint maxtype, ajuint maxmethod,
			  ajuint maxfield, ajuint maxdefined,
			  ajuint maxrelease)
{

    if(html)
	/* start the HTML table title line and output the Name header */
	ajFmtPrintF(outfile, "<tr><th>Name</th>");
    else
	ajFmtPrintF(outfile, "# Name%*s ", maxname-6, " ");

    if(dotype)
    {
	if(html)
	    ajFmtPrintF(outfile, "<th>Type</th>");
	else
	    ajFmtPrintF(outfile, "Type%*s ", maxtype-4, " ");
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
	    ajFmtPrintF(outfile, "Method%*s ", maxmethod-6, " ");
    }

    if(dofields)
    {
	if(html)
	    ajFmtPrintF(outfile, "<th>Fields</th>");
	else
	    ajFmtPrintF(outfile, "Fields%*s ", maxfield-6, " ");
    }

    if(dodefined)
    {
	if(html)
	    ajFmtPrintF(outfile, "<th>Defined</th>");
	else
	    ajFmtPrintF(outfile, "Defined%*s ", maxdefined-7, " ");
    }

    if(dorelease)
    {
	if(html)
	    ajFmtPrintF(outfile, "<th>Release</th>");
	else
	    ajFmtPrintF(outfile, "Release%*s ", maxrelease-7, " ");
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

	if(doid)
	    ajFmtPrintF(outfile, "==  ");

	if(doqry)
	    ajFmtPrintF(outfile, "=== ");

	if(doall)
	    ajFmtPrintF(outfile, "=== ");

	if(domethod)
	    ajFmtPrintF(outfile, "%.*s ", maxmethod,
			"==================================================");

	if(dofields)
	    ajFmtPrintF(outfile, "%.*s ", maxfield,
			"==================================================");

	if(dodefined)
	    ajFmtPrintF(outfile, "%.*s ", maxdefined,
			"==================================================");

	if(dorelease)
	    ajFmtPrintF(outfile, "%.*s ", maxrelease,
			"==================================================");

	if(docomment)
	    ajFmtPrintF(outfile, "=======");

	ajFmtPrintF(outfile, "\n");
    }
    return;
}

/* @funcstatic showdbDBOut ****************************************************
**
** Output db information
**
** @param [w] outfile [AjPFile] outfile
** @param [r] dbname [const AjPStr] database name
** @param [r] type [const AjPStr] type
** @param [r] id [AjBool] id
** @param [r] qry [AjBool] queryable
** @param [r] all [AjBool] all info
** @param [r] methods [const AjPStr] db access method(s)
** @param [r] defined [const AjPStr] db definition file short name
** @param [r] comment [const AjPStr] db comment
** @param [r] release [const AjPStr] db release
** @param [r] html [AjBool] do html
** @param [r] dotype [AjBool] show type
** @param [r] doid [AjBool] show id
** @param [r] doqry [AjBool] show query status
** @param [r] doall [AjBool] show everything
** @param [r] domethod [AjBool] show access method(s)
** @param [r] dofields [AjBool] show query fields
** @param [r] dodefined [AjBool] show access method(s)
** @param [r] docomment [AjBool] show comment
** @param [r] dorelease [AjBool] show release
** @param [r] maxname [ajuint] Maximum width for name
** @param [r] maxtype [ajuint] Maximum width for type
** @param [r] maxmethod [ajuint] Maximum width for list of access methods
** @param [r] maxfield [ajuint] Maximum width for list of fields
** @param [r] maxdefined [ajuint] Maximum width for definition file
** @param [r] maxrelease [ajuint] Maximum width for release number
** @@
******************************************************************************/

static void showdbDBOut(AjPFile outfile,
			const AjPStr dbname, const AjPStr type,
			AjBool id, AjBool qry, AjBool all,
			const AjPStr methods, const AjPStr defined,
			const AjPStr comment, const AjPStr release,
			AjBool html, AjBool dotype,
			AjBool doid, AjBool doqry, AjBool doall,
			AjBool domethod, AjBool dofields, AjBool dodefined,
			AjBool docomment, AjBool dorelease,
			ajuint maxname, ajuint maxtype, ajuint maxmethod,
			ajuint maxfield, ajuint maxdefined, ajuint maxrelease)
{
    AjPStr fields = NULL;

    if(html)
	/* start table line and output name */
	ajFmtPrintF(outfile, "<tr><td>%S</td>", dbname);
    else
    {
	ajFmtPrintF(outfile, "%-*S ", maxname, dbname);
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

    if(dofields)
    {
	if(html)
	    ajFmtPrintF(outfile, "<td>");

        showdbGetFields(dbname, &fields);
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

    if(dorelease)
    {
	if(html)
	    ajFmtPrintF(outfile, "<td>");

	if(release != NULL)
	    ajFmtPrintF(outfile, "%-*S ", maxrelease, release);
	else
	    ajFmtPrintF(outfile, "%-*s ", maxrelease, "");

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




/* @funcstatic showdbGetFields ***********************************************
**
** Get a database's valid query fields (apart from the default 'id' and 'acc')
**
** @param [r] dbname [const AjPStr] database name
** @param [w] fields [AjPStr*] the available search fields
** @return [void]
** @@
******************************************************************************/

static void showdbGetFields(const AjPStr dbname, AjPStr* fields)
{
    AjPSeqQuery query;

    query = ajSeqQueryNew();

    ajStrAssignS(&query->DbName, dbname);
    ajNamDbData(query);
    ajStrAssignS(fields, query->DbFields);

    /* if there are no query fields, then change to a '_' */
    if(!ajStrGetLen(*fields))
  	ajStrAssignC(fields, "-     ");
    else
	/* change spaces to commas to make the result one word */
	ajStrExchangeSetCC(fields, " ", ",");

    ajSeqQueryDel(&query);
    return;
}


/* @funcstatic showdbDBSortDefined ********************************************
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

static int showdbDBSortDefined(const void* str1, const void* str2)
{
    const AjPStr db1 = *(AjPStr const *) str1;
    const AjPStr db2 = *(AjPStr const *) str2;

    AjPStr methods = NULL;
    AjPStr release = NULL;
    AjPStr comment = NULL;
    AjPStr type    = NULL;
    AjBool id;
    AjBool qry;
    AjBool all;
    AjPStr defined1 = NULL;
    AjPStr defined2 = NULL;
    AjBool ok;

    int ret;

    ok = ajNamDbDetails(db1, &type, &id, &qry, &all, &comment,
			&release, &methods, &defined1);
    if(ok)
	ok = ajNamDbDetails(db2, &type, &id, &qry, &all, &comment,
			    &release, &methods, &defined2);
    ajStrDel(&type);
    ajStrDel(&comment);
    ajStrDel(&release);
    ajStrDel(&methods);

    if(ok)
    {
	ret = ajStrCmpS(defined1, defined2);
	ajDebug("Sorting1 %S:%S %S:%S %d\n", db1, db2, defined1, defined2, ret);
	ajStrDel(&defined1);
	ajStrDel(&defined2);
	if (ret) return ret;
    }

    ajStrDel(&defined1);
    ajStrDel(&defined2);

    ret = ajStrCmpS(db1, db2);
    ajDebug("Sorting2 %S:%S %d\n", db1, db2, ret);

    return ret;
}


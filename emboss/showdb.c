/* @source  application
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
                           AjBool dofields, AjBool doaliases,
                           AjBool doexamples, AjBool dotaxons,
			   ajuint *maxname, ajuint *maxtype,
			   ajuint *maxmethod, ajuint *maxfield,
                           ajuint *maxalias, ajuint *maxexample,
                           ajuint *maxtax,
			   ajuint* maxdefined, ajuint *maxrelease);
static void showdbDBHead (AjPFile outfile, AjBool html, AjBool dotype,
			  AjBool doid, AjBool doqry, AjBool doall,
			  AjBool domethod,
                          AjBool donumfields, AjBool dofields,
                          AjBool donumaliases, AjBool doaliases,
                          AjBool donumexamples, AjBool doexamples,
                          AjBool dotaxons, AjBool dodefined,
			  AjBool docomment, AjBool dorelease,
			  ajuint maxname, ajuint maxtype, ajuint maxmethod, 
			  ajuint maxfield, ajuint maxalias, ajuint maxexample,
			  ajuint maxtax,
                          ajuint maxdefined, ajuint maxrelease);

static void   showdbDBOut(AjPFile outfile,
			  const AjPStr dbname, const AjPStr type,
			  AjBool id, AjBool qry, AjBool all,
			  const AjPStr methods, const AjPStr defined,
			  const AjPStr comment, const AjPStr release,
			  AjBool html, AjBool dotype,
			  AjBool doid, AjBool doqry, AjBool doall,
			  AjBool domethod,
                          AjBool donumfields, AjBool dofields,
                          AjBool donumaliases, AjBool doaliases,
                          AjBool donumexamples, AjBool doexamples,
                          AjBool dotaxons, AjBool dodefined,
			  AjBool docomment, AjBool dorelease,
			  ajuint maxname, ajuint maxtype, ajuint maxmethod,
			  ajuint maxfield, ajuint maxalias, ajuint maxexample,
                          ajuint maxtax,
                          ajuint maxdefined, ajuint maxrelease);

static void   showdbGetFields(const AjPStr dbname,
                              AjPStr* fields, ajuint *numfields);
static void   showdbGetExamples(const AjPStr dbname,
                                AjPStr* examples, ajuint *numexamples);
static void   showdbGetAliases(const AjPStr dbname,
                               AjPStr *aliases, ajuint *numaliases);
static void   showdbGetTaxons(const AjPStr dbname,
                              AjPStr *taxons, ajuint *numtaxon);
static int    showdbDBSortDefined(const void* str1, const void* str2);




/* @prog showdb ***************************************************************
**
** Displays information on the currently available databases
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjBool html;
    AjBool showprotein;
    AjBool shownucleic;
    AjBool showobo;
    AjBool showresource;
    AjBool showtext;
    AjBool showxml;
    AjBool showtax;
    AjBool showsequence;
    AjBool showfeature;
    AjBool doheader;
    AjBool dotype;
    AjBool doid;
    AjBool doqry;
    AjBool doall;
    AjBool doaccess;
    AjBool domethod;
    AjBool dotaxons;
    AjBool dofields;
    AjBool doaliases;
    AjBool doexamples;
    AjBool dodefined;
    AjBool dorelease;
    AjBool docomment;
    AjBool showstandard;
    AjBool showuser;
    AjBool showinclude;
    AjBool donumfields;
    AjBool donumexamples;
    AjBool donumaliases;

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

    AjPList dbnames = NULL;
    AjIList iter = NULL;

    ajuint maxname = 14;
    ajuint maxmethod = 6;
    ajuint maxfield = 6;
    ajuint maxalias = 7;
    ajuint maxexample = 8;
    ajuint maxtax = 6;
    ajuint maxtype = 4;
    ajuint maxdefined = 7;
    ajuint maxrelease = 7;
    AjPTable dbtable = NULL;

    ajNamSetControl("namvalid");	/* validate database/resource defs */

    embInit("showdb", argc, argv);

    onedbname  = ajAcdGetString("database");
    outfile = ajAcdGetOutfile("outfile");

    html    = ajAcdGetBoolean("html");
    showprotein = ajAcdGetBoolean("protein");
    shownucleic = ajAcdGetBoolean("nucleic");
    showobo     = ajAcdGetBoolean("obo");
    showfeature = ajAcdGetBoolean("feature");
    showtext    = ajAcdGetBoolean("text");
    showxml     = ajAcdGetBoolean("xml");
    showtax     = ajAcdGetBoolean("taxonomy");
    showstandard = ajAcdGetBoolean("standard");
    showuser     = ajAcdGetBoolean("user");
    showinclude  = ajAcdGetBoolean("include");
    showresource = ajAcdGetBoolean("resource");
/*
    showassembly= ajAcdGetBoolean("assembly");
*/
    showsequence= ajAcdGetBoolean("sequence");

    doheader  = ajAcdGetBoolean("heading");
    dotype    = ajAcdGetBoolean("type");
    doid      = ajAcdGetBoolean("id");
    doqry     = ajAcdGetBoolean("query");
    doall     = ajAcdGetBoolean("all");
    dofields  = ajAcdGetBoolean("fields");
    doaliases  = ajAcdGetBoolean("aliases");
    doexamples= ajAcdGetBoolean("examples");
    donumfields  = ajAcdGetBoolean("numfields");
    donumaliases = ajAcdGetBoolean("numaliases");
    donumexamples= ajAcdGetBoolean("numexamples");
    dodefined = ajAcdGetBoolean("defined");
    domethod  = ajAcdGetBoolean("methods");
    dotaxons  = ajAcdGetBoolean("taxscope");
    doaccess  = ajAcdGetBoolean("access");
    docomment = ajAcdGetBoolean("comment");
    dorelease = ajAcdGetBoolean("release");
    
    dbnames = ajListstrNew();

    if(!doaccess)
    {
        doid = ajFalse;
        doqry = ajFalse;
        doall = ajFalse;
    }

    /* start the HTML table */
    if(html)
	ajFmtPrintF(outfile, "<table border cellpadding=4 bgcolor="
		    "\"#FFFFF0\">\n");

    dbtable = ajTablestrNewConst(100);
    
    /* Just one specified name to get details on? */
    if(ajStrGetLen(onedbname))
    {
	if(ajNamDbDetails(onedbname, &type, &id, &qry, &all, &comment,
			  &release, &methods, &defined))
        {
	    showdbDBWidth(onedbname, type, methods, defined, release,
                          dofields, doaliases, doexamples, dotaxons,
                          &maxname, &maxtype, &maxmethod,
                          &maxfield, &maxalias, &maxexample, &maxtax,
                          &maxdefined, &maxrelease);

            /* print the header information */
            if(doheader)
                showdbDBHead(outfile, html, dotype, doid, doqry,
                             doall, domethod,
                             donumfields, dofields,
                             donumaliases, doaliases,
                             donumexamples, doexamples,
                             dotaxons, dodefined,
                             docomment, dorelease,
                             maxname, maxtype, maxmethod, maxfield,
                             maxalias, maxexample, maxtax,
                             maxdefined, maxrelease);

    	    showdbDBOut(outfile, onedbname, type, id, qry, all,
                        methods, defined,
                        comment, release, html,
                        dotype, doid, doqry, doall, 
                        domethod,
                        donumfields, dofields,
                        donumaliases, doaliases,
                        donumexamples, doexamples,
                        dotaxons, dodefined,
                        docomment, dorelease,
                        maxname, maxtype,
                        maxmethod, maxfield, maxalias, maxexample, maxtax,
                        maxdefined, maxrelease);
	}
	else
	    ajFatal("The database '%S' does not exist", onedbname);
    }
    else
    {
	/* get the list of database names */
	ajNamListListDatabases(dbnames);

	/* sort it */
	/*ajListSort(dbnames, &ajStrCmp);*/
	ajListSort(dbnames, &showdbDBSortDefined);

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
            if(!ajNamDbDetails(nextdbname, &type, &id, &qry, &all, &comment,
			      &release, &methods, &defined))
                continue;

            if(!showstandard && ajStrMatchC(defined, "standard"))
                continue;

	    if(!showuser && ajStrMatchC(defined, "user"))
                    continue;

            if(!showinclude &&
               (!ajStrMatchC(defined, "user") &&
                !ajStrMatchC(defined, "standard")))
                    continue;

            
            showdbDBWidth(nextdbname, type, methods, defined, release,
                          dofields, doaliases, doexamples, dotaxons,
                          &maxname, &maxtype, &maxmethod,
                          &maxfield, &maxalias, &maxexample, &maxtax,
                          &maxdefined, &maxrelease);
	}

	/* print the header information */
	if(doheader)
	    showdbDBHead(outfile, html, dotype, doid, doqry,
                         doall, domethod,
                         donumfields, dofields,
                         donumaliases, doaliases,
                         donumexamples, doexamples,
                         dotaxons, dodefined,
                         docomment, dorelease,
                         maxname, maxtype, maxmethod, maxfield,
                         maxalias, maxexample, maxtax,
                         maxdefined, maxrelease);
    
	ajListIterDel(&iter);

	/* iterate through the dbnames list */
	iter = ajListIterNewread(dbnames);

	/* write out protein databases */
	while((nextdbname = ajListIterGet(iter)) != NULL)
	    if(ajNamDbDetails(nextdbname, &type, &id, &qry, &all, &comment,
			      &release, &methods, &defined))
	    {
		if(!showstandard && ajStrMatchC(defined, "standard"))
                    continue;

                if(!showuser && ajStrMatchC(defined, "user"))
                    continue;

                if(!showinclude &&
                   (!ajStrMatchC(defined, "user") &&
                    !ajStrMatchC(defined, "standard")))
                    continue;

                if(showprotein &&
                   !ajTableMatchS(dbtable, nextdbname) &&
                   (ajStrFindC(type, "Protein") >= 0 ||
                    ajStrFindC(type, "Protfeatures") >= 0))
                {
                    showdbDBOut(outfile, nextdbname, type, id, qry, all,
                                methods, defined,
                                comment, release, html, dotype, doid,
                                doqry, doall, domethod,
                                donumfields, dofields,
                                donumaliases, doaliases,
                                donumexamples, doexamples,
                                dotaxons, dodefined,
                                docomment, 
                                dorelease, maxname,maxtype,  maxmethod,
                                maxfield, maxalias, maxexample, maxtax,
                                maxdefined, maxrelease);
                    ajTablePut(dbtable, nextdbname, NULL);
                }
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
		if(!showstandard && ajStrMatchC(defined, "standard"))
                    continue;

                if(!showuser && ajStrMatchC(defined, "user"))
                    continue;

                if(!showinclude &&
                   (!ajStrMatchC(defined, "user") &&
                    !ajStrMatchC(defined, "standard")))
                    continue;

                if(shownucleic &&
		   !ajTableMatchS(dbtable, nextdbname) &&
                   (ajStrFindC(type, "Nucleotide") >= 0 ||
                    ajStrFindC(type, "Nucfeatures") >= 0 ||
                    ajStrFindC(type, "Refseq") >= 0))
                {
                    showdbDBOut(outfile, nextdbname, type, id, qry, all,
                                methods, defined,
                                comment, release, html,
                                dotype, doid,
                                doqry, doall, domethod,
                                donumfields, dofields,
                                donumaliases, doaliases,
                                donumexamples, doexamples,
                                dotaxons, dodefined,
                                docomment, dorelease,
                                maxname, maxtype, maxmethod,
                                maxfield, maxalias, maxexample, maxtax,
                                maxdefined, maxrelease);
                    ajTablePut(dbtable, nextdbname, NULL);
                }
	    }
	    else
		ajFatal("The database '%S' does not exist", nextdbname);
	}

	/* reset the iterator */
	ajListIterDel(&iter);
	iter = ajListIterNewread(dbnames);

	/* now write out sequence databases */
	while((nextdbname = ajListIterGet(iter)) != NULL)
	{
	    if(ajNamDbDetails(nextdbname, &type, &id, &qry, &all, &comment,
			      &release, &methods, &defined))
	    {
		if(!showstandard && ajStrMatchC(defined, "standard"))
                    continue;

                if(!showuser && ajStrMatchC(defined, "user"))
                    continue;

                if(!showinclude &&
                   (!ajStrMatchC(defined, "user") &&
                    !ajStrMatchC(defined, "standard")))
                    continue;

                if(showsequence &&
		   !ajTableMatchS(dbtable, nextdbname) &&
                   (ajStrFindC(type, "Sequence") >= 0))
                {
                    showdbDBOut(outfile, nextdbname, type, id, qry, all,
                                methods, defined,
                                comment, release, html,
                                dotype, doid,
                                doqry, doall, domethod,
                                donumfields, dofields,
                                donumaliases, doaliases,
                                donumexamples, doexamples,
                                dotaxons, dodefined,
                                docomment, dorelease,
                                maxname, maxtype, maxmethod,
                                maxfield, maxalias, maxexample, maxtax,
                                maxdefined, maxrelease);
                    ajTablePut(dbtable, nextdbname, NULL);
                }
	    }
	    else
		ajFatal("The database '%S' does not exist", nextdbname);
	}

	ajListIterDel(&iter);
	iter = ajListIterNewread(dbnames);

	/* now write out feature databases */
	while((nextdbname = ajListIterGet(iter)) != NULL)
	{
	    if(ajNamDbDetails(nextdbname, &type, &id, &qry, &all, &comment,
			      &release, &methods, &defined))
	    {
		if(!showstandard && ajStrMatchC(defined, "standard"))
                    continue;

                if(!showuser && ajStrMatchC(defined, "user"))
                    continue;

                if(!showinclude &&
                   (!ajStrMatchC(defined, "user") &&
                    !ajStrMatchC(defined, "standard")))
                    continue;

                if(showfeature &&
		   !ajTableMatchS(dbtable, nextdbname) &&
                   (ajStrFindC(type, "Features") >= 0 ||
                    (shownucleic && ajStrFindC(type, "Nucfeatures") >= 0) ||
                    (showprotein && ajStrFindC(type, "Protfeatures") >= 0)))
                {
                    showdbDBOut(outfile, nextdbname, type, id, qry, all,
                                methods, defined,
                                comment, release, html,
                                dotype, doid,
                                doqry, doall, domethod,
                                donumfields, dofields,
                                donumaliases, doaliases,
                                donumexamples, doexamples,
                                dotaxons, dodefined,
                                docomment, dorelease,
                                maxname, maxtype, maxmethod,
                                maxfield, maxalias, maxexample, maxtax,
                                maxdefined, maxrelease);
                    ajTablePut(dbtable, nextdbname, NULL);
                }
	    }
	    else
		ajFatal("The database '%S' does not exist", nextdbname);
	}

	ajListIterDel(&iter);
	iter = ajListIterNewread(dbnames);

	/* now write out taxonomy databases */
	while((nextdbname = ajListIterGet(iter)) != NULL)
	{
	    if(ajNamDbDetails(nextdbname, &type, &id, &qry, &all, &comment,
			      &release, &methods, &defined))
	    {
		if(!showstandard && ajStrMatchC(defined, "standard"))
                    continue;

                if(!showuser && ajStrMatchC(defined, "user"))
                    continue;

                if(!showinclude &&
                   (!ajStrMatchC(defined, "user") &&
                    !ajStrMatchC(defined, "standard")))
                    continue;

                if(showtax &&
		   !ajTableMatchS(dbtable, nextdbname) &&
                   (ajStrFindC(type, "Taxonomy") >= 0))
                {
                    showdbDBOut(outfile, nextdbname, type, id, qry, all,
                                methods, defined,
                                comment, release, html,
                                dotype, doid,
                                doqry, doall, domethod,
                                donumfields, dofields,
                                donumaliases, doaliases,
                                donumexamples, doexamples,
                                dotaxons, dodefined,
                                docomment, dorelease,
                                maxname, maxtype, maxmethod,
                                maxfield, maxalias, maxexample, maxtax,
                                maxdefined, maxrelease);
                    ajTablePut(dbtable, nextdbname, NULL);
                }
	    }
	    else
		ajFatal("The database '%S' does not exist", nextdbname);
	}

	ajListIterDel(&iter);
	iter = ajListIterNewread(dbnames);

	/* now write out resource databases */
	while((nextdbname = ajListIterGet(iter)) != NULL)
	{
	    if(ajNamDbDetails(nextdbname, &type, &id, &qry, &all, &comment,
			      &release, &methods, &defined))
	    {
		if(!showstandard && ajStrMatchC(defined, "standard"))
                    continue;

                if(!showuser && ajStrMatchC(defined, "user"))
                    continue;

                if(!showinclude &&
                   (!ajStrMatchC(defined, "user") &&
                    !ajStrMatchC(defined, "standard")))
                    continue;

                if(showresource &&
		   !ajTableMatchS(dbtable, nextdbname) &&
                   (ajStrFindC(type, "Resource") >= 0))
                {
                    showdbDBOut(outfile, nextdbname, type, id, qry, all,
                                methods, defined,
                                comment, release, html,
                                dotype, doid,
                                doqry, doall, domethod,
                                donumfields, dofields,
                                donumaliases, doaliases,
                                donumexamples, doexamples,
                                dotaxons, dodefined,
                                docomment, dorelease,
                                maxname, maxtype, maxmethod,
                                maxfield, maxalias, maxexample, maxtax,
                                maxdefined, maxrelease);
                    ajTablePut(dbtable, nextdbname, NULL);
                }
	    }
	    else
		ajFatal("The database '%S' does not exist", nextdbname);
	}

	ajListIterDel(&iter);
	iter = ajListIterNewread(dbnames);

	/* now write out OBO databases */
	while((nextdbname = ajListIterGet(iter)) != NULL)
	{
	    if(ajNamDbDetails(nextdbname, &type, &id, &qry, &all, &comment,
			      &release, &methods, &defined))
	    {
		if(!showstandard && ajStrMatchC(defined, "standard"))
                    continue;

                if(!showuser && ajStrMatchC(defined, "user"))
                    continue;

                if(!showinclude &&
                   (!ajStrMatchC(defined, "user") &&
                    !ajStrMatchC(defined, "standard")))
                    continue;

                if(showobo &&
		   !ajTableMatchS(dbtable, nextdbname) &&
                   (ajStrFindC(type, "Obo") >= 0))
                {
                    showdbDBOut(outfile, nextdbname, type, id, qry, all,
                                methods, defined,
                                comment, release, html,
                                dotype, doid,
                                doqry, doall, domethod,
                                donumfields, dofields,
                                donumaliases, doaliases,
                                donumexamples, doexamples,
                                dotaxons, dodefined,
                                docomment, dorelease,
                                maxname, maxtype, maxmethod,
                                maxfield, maxalias, maxexample, maxtax,
                                maxdefined, maxrelease);
                    ajTablePut(dbtable, nextdbname, NULL);
                }
	    }
	    else
		ajFatal("The database '%S' does not exist", nextdbname);
	}

	ajListIterDel(&iter);
	iter = ajListIterNewread(dbnames);

	/* now write out XML databases */
	while((nextdbname = ajListIterGet(iter)) != NULL)
	{
	    if(ajNamDbDetails(nextdbname, &type, &id, &qry, &all, &comment,
			      &release, &methods, &defined))
	    {
		if(!showstandard && ajStrMatchC(defined, "standard"))
                    continue;

                if(!showuser && ajStrMatchC(defined, "user"))
                    continue;

                if(!showinclude &&
                   (!ajStrMatchC(defined, "user") &&
                    !ajStrMatchC(defined, "standard")))
                    continue;

                if(showxml &&
                   !ajTableMatchS(dbtable, nextdbname) &&
                   (ajStrFindC(type, "Xml") >= 0))
                {
                    showdbDBOut(outfile, nextdbname, type, id, qry, all,
                                methods, defined,
                                comment, release, html,
                                dotype, doid,
                                doqry, doall, domethod,
                                donumfields, dofields,
                                donumaliases, doaliases,
                                donumexamples, doexamples,
                                dotaxons, dodefined,
                                docomment, dorelease,
                                maxname, maxtype, maxmethod,
                                maxfield, maxalias, maxexample, maxtax,
                                maxdefined, maxrelease);
                    ajTablePut(dbtable, nextdbname, NULL);
                }
	    }
	    else
		ajFatal("The database '%S' does not exist", nextdbname);
	}

	ajListIterDel(&iter);
	iter = ajListIterNewread(dbnames);

	/* now write out text databases */
	while((nextdbname = ajListIterGet(iter)) != NULL)
	{
	    if(ajNamDbDetails(nextdbname, &type, &id, &qry, &all, &comment,
			      &release, &methods, &defined))
	    {
		if(!showstandard && ajStrMatchC(defined, "standard"))
                    continue;

                if(!showuser && ajStrMatchC(defined, "user"))
                    continue;

                if(!showinclude &&
                   (!ajStrMatchC(defined, "user") &&
                    !ajStrMatchC(defined, "standard")))
                    continue;

                if(showtext &&
                   !ajTableMatchS(dbtable, nextdbname) &&
                   (ajStrFindC(type, "Text") >= 0 ||
                    ajStrFindC(type, "Html") >= 0))
                {
                    showdbDBOut(outfile, nextdbname, type, id, qry, all,
                                methods, defined,
                                comment, release, html,
                                dotype, doid,
                                doqry, doall, domethod,
                                donumfields, dofields,
                                donumaliases, doaliases,
                                donumexamples, doexamples,
                                dotaxons, dodefined,
                                docomment, dorelease,
                                maxname, maxtype, maxmethod,
                                maxfield, maxalias, maxexample, maxtax,
                                maxdefined, maxrelease);
                    ajTablePut(dbtable, nextdbname, NULL);
                }
	    }
	    else
		ajFatal("The database '%S' does not exist", nextdbname);
	}

	ajListIterDel(&iter);
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
    ajListFree(&dbnames);
    ajTableFree(&dbtable);

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
** @param [r] dofields [AjBool] Show fields
** @param [r] doaliases [AjBool] Show aliases
** @param [r] doexamples [AjBool] Show examples
** @param [r] dotaxons [AjBool] Show taxons
** @param [u] maxname [ajuint*] Maximum width for name
** @param [u] maxtype [ajuint*] Maximum width for type
** @param [u] maxmethod [ajuint*] Maximum width for list of access methods
** @param [u] maxfield [ajuint*] Maximum width for list of fields
** @param [u] maxalias [ajuint*] Maximum width for list of aliases
** @param [u] maxexample [ajuint*] Maximum width for list of examples
** @param [u] maxtax [ajuint*] Maximum width for of taxons
** @param [u] maxdefined [ajuint*] Maximum width for definition file
** @param [u] maxrelease [ajuint*] Maximum width for release number
******************************************************************************/

static void showdbDBWidth (const AjPStr dbname,
			   const AjPStr type, const AjPStr methods,
			   const AjPStr defined, const AjPStr release,
                           AjBool dofields, AjBool doaliases,
                           AjBool doexamples, AjBool dotaxons,
			   ajuint *maxname, ajuint* maxtype,
                           ajuint *maxmethod, ajuint *maxfield,
                           ajuint *maxalias, ajuint *maxexample,
                           ajuint *maxtax,
                           ajuint* maxdefined, ajuint *maxrelease)
{
    AjPStr fields = NULL;
    ajuint i;
    ajuint num;

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

    if(dofields)
    {
        showdbGetFields(dbname, &fields, &num);
        i = ajStrGetLen(fields);
        if (i > *maxfield)
            *maxfield = i;
    }

    if(doaliases)
    {
        showdbGetAliases(dbname, &fields, &num);
        i = ajStrGetLen(fields);
        if (i > *maxalias)
            *maxalias = i;
    }

    if(doexamples)
    {
        showdbGetExamples(dbname, &fields, &num);
        i = ajStrGetLen(fields);
        if (i > *maxexample)
            *maxexample = i;
    }

    if(dotaxons)
    {
        showdbGetTaxons(dbname, &fields, &num);
        i = ajStrGetLen(fields);
        if (i > *maxtax)
            *maxtax = i;
    }

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
** @param [r] domethod [AjBool] show access method(s)*
** @param [r] donumfields [AjBool] show number of query fields
** @param [r] dofields [AjBool] show query fields*
** @param [r] donumaliases [AjBool] show number of aliases
** @param [r] doaliases [AjBool] show aliases
** @param [r] donumexamples [AjBool] show number of examples
** @param [r] doexamples [AjBool] show examples
** @param [r] dotaxons [AjBool] show taxons
** @param [r] dodefined [AjBool] show definition file
** @param [r] docomment [AjBool] show comment
** @param [r] dorelease [AjBool] show release
** @param [r] maxname [ajuint] Maximum width for name
** @param [r] maxtype [ajuint] Maximum width for type
** @param [r] maxmethod [ajuint] Maximum width for access method
** @param [r] maxfield [ajuint] Maximum width for list of fields
** @param [r] maxalias [ajuint] Maximum width for list of aliases
** @param [r] maxexample [ajuint] Maximum width for list of examples
** @param [r] maxtax [ajuint] Maximum width for list of taxons
** @param [r] maxdefined [ajuint] Maximum width for definition file
** @param [r] maxrelease [ajuint] Maximum width for release number
******************************************************************************/

static void showdbDBHead (AjPFile outfile, AjBool html, AjBool dotype,
			  AjBool doid, AjBool doqry, AjBool doall,
			  AjBool domethod,
                          AjBool donumfields, AjBool dofields,
                          AjBool donumaliases, AjBool doaliases,
                          AjBool donumexamples, AjBool doexamples,
                          AjBool dotaxons, AjBool dodefined,
			  AjBool docomment, AjBool dorelease,
			  ajuint maxname, ajuint maxtype, ajuint maxmethod,
			  ajuint maxfield, ajuint maxalias, ajuint maxexample,
                          ajuint maxtax,
                          ajuint maxdefined, ajuint maxrelease)
{
    AjPStr understr = NULL;
    ajuint maxlen = maxname;
    ajuint imaxfield = maxfield;
    ajuint imaxalias = maxalias;
    ajuint imaxexample = maxexample;

    if(donumfields && dofields)
        imaxfield += 7;

    if(donumaliases && doaliases)
        imaxalias += 8;

    if(donumexamples && doexamples)
        imaxexample += 9;

    maxlen = AJMAX(maxlen, maxtype);
    maxlen = AJMAX(maxlen, maxmethod);
    maxlen = AJMAX(maxlen, imaxfield);
    maxlen = AJMAX(maxlen, imaxalias);
    maxlen = AJMAX(maxlen, imaxexample);
    maxlen = AJMAX(maxlen, maxtax);
    maxlen = AJMAX(maxlen, maxdefined);
    maxlen = AJMAX(maxlen, maxrelease);

    ajStrAppendCountK(&understr, '=', maxlen);

    if(html)
	/* start the HTML table title line and output the Name header */
	ajFmtPrintF(outfile, "<tr><th>Name</th>");
    else
	ajFmtPrintF(outfile, "# Name%*s", maxname-5, " ");

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

    if(dotaxons)
    {
	if(html)
	    ajFmtPrintF(outfile, "<th>Taxon</th>");
	else
	    ajFmtPrintF(outfile, "Taxon%*s", maxtax-4, " ");
    }

    if(donumfields || dofields)
    {
	if(html)
	    ajFmtPrintF(outfile, "<th>Fields</th>");
	else
	    ajFmtPrintF(outfile, "Fields%*s", imaxfield-5, " ");
    }

    if(donumaliases || doaliases)
    {
	if(html)
	    ajFmtPrintF(outfile, "<th>Aliases</th>");
	else
	    ajFmtPrintF(outfile, "Aliases%*s", imaxalias-6, " ");
    }

    if(donumexamples || doexamples)
    {
	if(html)
	    ajFmtPrintF(outfile, "<th>Examples</th>");
	else
	    ajFmtPrintF(outfile, "Examples%*s", imaxexample-7, " ");
    }

    if(dodefined)
    {
	if(html)
	    ajFmtPrintF(outfile, "<th>Defined</th>");
	else
	    ajFmtPrintF(outfile, "Defined%*s", maxdefined-6, " ");
    }

    if(dorelease)
    {
	if(html)
	    ajFmtPrintF(outfile, "<th>Release</th>");
	else
	    ajFmtPrintF(outfile, "Release%*s", maxrelease-6, " ");
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
	
	ajFmtPrintF(outfile, "# %.*S ", maxname-2, understr);

	if(dotype)
	    ajFmtPrintF(outfile, "%.*S ", maxtype, understr);

	if(doid)
	    ajFmtPrintF(outfile, "==  ");

	if(doqry)
	    ajFmtPrintF(outfile, "=== ");

	if(doall)
	    ajFmtPrintF(outfile, "=== ");

	if(domethod)
	    ajFmtPrintF(outfile, "%.*S ", maxmethod, understr);

	if(dotaxons)
	    ajFmtPrintF(outfile, "%.*S ", maxtax, understr);

	if(donumfields || dofields)
	    ajFmtPrintF(outfile, "%.*S ", imaxfield, understr);

	if(donumaliases || doaliases)
	    ajFmtPrintF(outfile, "%.*S ", imaxalias, understr);

	if(donumexamples || doexamples)
	    ajFmtPrintF(outfile, "%.*S ", imaxexample, understr);

	if(dodefined)
	    ajFmtPrintF(outfile, "%.*S ", maxdefined, understr);

	if(dorelease)
	    ajFmtPrintF(outfile, "%.*S ", maxrelease, understr);

	if(docomment)
	    ajFmtPrintF(outfile, "=======");

	ajFmtPrintF(outfile, "\n");
    }

    ajStrDel(&understr);

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
** @param [r] donumfields [AjBool] show query fields
** @param [r] dofields [AjBool] show query fields
** @param [r] donumaliases [AjBool] show number of aliases
** @param [r] doaliases [AjBool] show aliases
** @param [r] donumexamples [AjBool] show number of examples
** @param [r] doexamples [AjBool] show examples
** @param [r] dotaxons [AjBool] show taxons
** @param [r] dodefined [AjBool] show access method(s)
** @param [r] docomment [AjBool] show comment
** @param [r] dorelease [AjBool] show release
** @param [r] maxname [ajuint] Maximum width for name
** @param [r] maxtype [ajuint] Maximum width for type
** @param [r] maxmethod [ajuint] Maximum width for list of access methods
** @param [r] maxfield [ajuint] Maximum width for list of fields
** @param [r] maxalias [ajuint] Maximum width for list of aliases
** @param [r] maxexample [ajuint] Maximum width for list of examples
** @param [r] maxtax [ajuint] Maximum width for list of taxons
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
			AjBool domethod,
                        AjBool donumfields, AjBool dofields,
                        AjBool donumaliases, AjBool doaliases,
                        AjBool donumexamples, AjBool doexamples,
			AjBool dotaxons, AjBool dodefined,
                        AjBool docomment, AjBool dorelease,
                        ajuint maxname, ajuint maxtype, ajuint maxmethod,
			ajuint maxfield, ajuint maxalias, ajuint maxexample,
                        ajuint maxtax, ajuint maxdefined, ajuint maxrelease)
{
    AjPStr fields = NULL;
    ajuint numfields;

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
        {
            ajStrAssignS(&fields, type);
            ajStrExchangeKK(&fields, ' ', ',');
	    ajFmtPrintF(outfile, "%-*S ", maxtype, fields);
        }
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
	    ajFmtPrintF(outfile, "<td>%S</td>", methods);
        else
            ajFmtPrintF(outfile, "%-*S ", maxmethod, methods);

    }

    if(dotaxons)
    {
	if(html)
	    ajFmtPrintF(outfile, "<td>");

        showdbGetTaxons(dbname, &fields, &numfields);
        ajFmtPrintF(outfile, "%-*S ", maxtax, fields);

	if(html)
	    ajFmtPrintF(outfile, "</td>");
    }

    if(dofields || donumfields)
    {
	if(html)
	    ajFmtPrintF(outfile, "<td>");

        showdbGetFields(dbname, &fields, &numfields);
        if(donumfields)
            ajFmtPrintF(outfile, "%6u ",numfields);
        if(dofields)
            ajFmtPrintF(outfile, "%-*S ", maxfield, fields);

	if(html)
	    ajFmtPrintF(outfile, "</td>");
    }

    if(doaliases || donumaliases)
    {
	if(html)
	    ajFmtPrintF(outfile, "<td>");

        showdbGetAliases(dbname, &fields, &numfields);
        if(donumaliases)
            ajFmtPrintF(outfile, "%7u ", numfields);
        if(doaliases)
            ajFmtPrintF(outfile, "%-*S ", maxalias, fields);

	if(html)
	    ajFmtPrintF(outfile, "</td>");
    }

    if(doexamples || donumexamples)
    {
	if(html)
	    ajFmtPrintF(outfile, "<td>");

        showdbGetExamples(dbname, &fields, &numfields);
        if(donumexamples)
            ajFmtPrintF(outfile, "%8u ", numfields);
        if(doexamples)
            ajFmtPrintF(outfile, "%-*S ", maxexample, fields);

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




/* @funcstatic showdbGetAliases ***********************************************
**
** Get a database's alias names
**
** @param [r] dbname [const AjPStr] database name
** @param [w] aliases [AjPStr*] the defined alias names
** @param [w] numaliases [ajuint*] count of alias names
** @return [void]
** @@
******************************************************************************/

static void showdbGetAliases(const AjPStr dbname,
                             AjPStr* aliases, ajuint *numaliases)
{
    AjPList alist = ajListNew();
    AjIList iter = NULL;
    const AjPStr aname = NULL;

    ajDebug("showdbGetAliases '%S'\n", dbname);

    ajStrSetClear(aliases);

    /* ajNamDbData will make no changes if the database does not exist */
    ajNamListFindAliases(dbname, alist);

    *numaliases = (ajuint) ajListGetLength(alist);
    iter = ajListIterNewread(alist);

    while(!ajListIterDone(iter))
    {
        aname = ajListIterGet(iter);
        if(ajStrGetLen(*aliases))
            ajStrAppendK(aliases, ',');
        ajStrAppendS(aliases, aname);
    }
    

    /* if there are no query fields, then change to a '_' */
    if(!ajStrGetLen(*aliases))
  	ajStrAssignC(aliases, "-");

    ajListFree(&alist);
    ajListIterDel(&iter);

    return;
}




/* @funcstatic showdbGetExamples **********************************************
**
** Get a database's valid example fields
**
** @param [r] dbname [const AjPStr] database name
** @param [w] examples [AjPStr*] the defined examples
** @param [w] numexamples [ajuint*] count of examples
** @return [void]
** @@
******************************************************************************/

static void showdbGetExamples(const AjPStr dbname,
                              AjPStr* examples, ajuint *numexamples)
{
    AjPQuery query;

    ajDebug("showdbGetExamples '%S'\n", dbname);

    ajStrSetClear(examples);

    *numexamples = 0;

    query = ajQueryNew(AJDATATYPE_UNKNOWN);

    ajStrAssignS(&query->DbName, dbname);

    /* ajNamDbData will make no changes if the database does not exist */
    if(!ajNamDbData(query,1, "example", examples))
        ajWarn("Database '%S' unknown", dbname);

    /* if there are no query fields, then change to a '_' */
    if(!ajStrGetLen(*examples))
  	ajStrAssignC(examples, "-");
    else
    {
	/* change spaces to semicolons to make the result one word */
	ajStrExchangeSetCC(examples, " ", ";");
        *numexamples = 1 + ajStrCalcCountK(*examples, ';');
    }

    ajQueryDel(&query);
    return;
}




/* @funcstatic showdbGetFields ***********************************************
**
** Get a database's valid query fields (apart from the default 'id' and 'acc')
**
** @param [r] dbname [const AjPStr] database name
** @param [w] fields [AjPStr*] the available search fields
** @param [w] numfields [ajuint*] count of defined fields
** @return [void]
** @@
******************************************************************************/

static void showdbGetFields(const AjPStr dbname,
                            AjPStr* fields, ajuint *numfields)
{
    AjPQuery query;
    AjBool hasid = ajFalse;
    AjBool hasacc = ajFalse;

    ajStrSetClear(fields);

    ajDebug("showdbGetFields '%S'\n", dbname);

    query = ajQueryNew(AJDATATYPE_UNKNOWN);

    ajStrAssignS(&query->DbName, dbname);

    /* ajNamDbData will make no changes if the database does not exist */
    if(!ajNamDbData(query,0))
        ajWarn("Database '%S' unknown", dbname);

    ajStrAssignS(fields, query->DbFields);

    *numfields = 0;

    if(!ajQueryDefinedFieldC(query, "id"))
    {
        hasid = ajTrue;
        ++(*numfields);
    }

    if(query->HasAcc && !ajQueryDefinedFieldC(query, "acc"))
    {
        hasacc = ajTrue;
        ++(*numfields);
    }

    /* if there are no query fields, then change to a '_' */
    if(!ajStrGetLen(*fields))
    {
        if(hasid)
        {
            ajStrAppendC(fields, "id");
            if(hasacc)
            {
                ajStrAppendK(fields, ',');
                ajStrAppendC(fields, "acc");
            }
            else if (hasacc)
            {
                ajStrAppendC(fields, "acc");
            }
            else
            {
                ajStrAssignC(fields, "-");
            }
        }
    }
    else
    {
	/* change spaces to commas to make the result one word */
	ajStrExchangeSetCC(fields, " ", ",");
	ajStrExchangeSetCC(fields, ";", ",");
        *numfields += 1 + ajStrCalcCountK(*fields, ',');
    }

    ajQueryDel(&query);
    return;
}




/* @funcstatic showdbGetTaxons ************************************************
**
** Get a database's taxon fields
**
** @param [r] dbname [const AjPStr] database name
** @param [w] taxons [AjPStr*] the defined taxon scopes
** @param [w] numtaxons [ajuint*] count of taxons
** @return [void]
** @@
******************************************************************************/

static void showdbGetTaxons(const AjPStr dbname,
                            AjPStr* taxons, ajuint *numtaxons)
{
    AjPQuery query;

    ajDebug("showdbGetTaxons '%S'\n", dbname);

    ajStrSetClear(taxons);

    *numtaxons = 0;

    query = ajQueryNew(AJDATATYPE_UNKNOWN);

    ajStrAssignS(&query->DbName, dbname);

    /* ajNamDbData will make no changes if the database does not exist */
    if(!ajNamDbData(query,1, "taxon", taxons))
        ajWarn("Database '%S' unknown", dbname);

    /* if there are no query fields, then change to a '_' */
    if(!ajStrGetLen(*taxons))
  	ajStrAssignC(taxons, "-");
    else
    {
	/* change spaces to underscores to make the result one word */
	ajStrExchangeSetCC(taxons, " ", "_");
        *numtaxons = 1 + ajStrCalcCountK(*taxons, ';');
    }

    ajQueryDel(&query);
    return;
}




/* @funcstatic showdbDBSortDefined ********************************************
**
** Compares the value of two database definitions for use in sorting
** (e.g. ajListSort). Sorts first by where they are defined, then by name.
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


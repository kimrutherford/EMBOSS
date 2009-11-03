/******************************************************************************
** @source AJAX nam functions
** Creates a hash table of initial values and allow access to this
** via the routines ajNamDatabase and ajNamGetValueS.
**
** @author Copyright (C) 1998 Ian Longden
** @version 1.0
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

#ifndef WIN32
#include <dirent.h>
#include <unistd.h>
#else
#include "win32.h"
#include <winsock2.h>
#include <stdlib.h>
#endif

#ifdef AJ_MEMPROBE
#include <mcheck.h>
#endif

enum NamEType
{
    TYPE_UNKNOWN,			/* no type set */
    TYPE_ENV,				/* env or set variable */
    TYPE_DB,				/* database definition */
    TYPE_RESOURCE,			/* resource definition */
    TYPE_IFILE				/* include filename */
};

#define NAM_INCLUDE_ESTIMATE 5	/* estimate of maximum number of include */
                                /* statements in emboss.default          */

/* Scope values for entry methods returned from nameMethod2Scope */
#define METHOD_ENTRY 1
#define METHOD_QUERY 2
#define METHOD_ALL   4
#define SLOW_ENTRY 8
#define SLOW_QUERY 16

static AjBool namDoDebug  = AJFALSE;
static AjBool namDoValid  = AJFALSE;
static AjBool namDoHomeRc = AJTRUE;
static AjPStr namRootStr  = NULL;
static AjPStr namValNameTmp  = NULL;

static AjBool namListParseOK = AJFALSE;

const char* namTypes[] = { "unknown", "SET", "DBNAME", "RESOURCE", "INCLUDE" };

/* source directory where control and data files can be found */

#ifdef AJAX_FIXED_ROOT
static char namFixedRoot[] = AJAX_FIXED_ROOT;
#else
static char namFixedRoot[] = "/nfs/WWW/data/EMBOSS";
#endif

/* install target directory where installed control and data files can
   be found  - this is the --prefix= from ./configure */

#ifdef PREFIX
static char namInstallRoot[] = PREFIX;
#else
#ifndef WIN32
static char namInstallRoot[] = "/usr/local";
#else
static char namInstallRoot[MAX_PATH];
#endif
#endif

/* package name from the makefile */

#ifdef PACKAGE
static char namPackage[] = PACKAGE;
#else
static char namPackage[] = "EMBOSS";
#endif

#ifdef VERSION
static char namVersion[] = VERSION;
#else
static char namVersion[] = "6.x";
#endif

/* string versions of char* constants set in ajNamInit */
static AjPStr namFixedBaseStr     = NULL;
static AjPStr namFixedRootStr     = NULL;
static AjPStr namFixedInstallStr  = NULL;
static AjPStr namFixedPackageStr  = NULL;
static AjPStr namFixedVersionStr  = NULL;
static AjPStr namPrefixStr        = NULL;
static AjPStr namFileOrig         = NULL;

static AjPTable namVarMasterTable = NULL;
static AjPTable namDbMasterTable = NULL;
static AjPTable namResMasterTable = NULL;
static ajint namParseType      = 0;
static AjPStr namFileName      = NULL;
static ajint namLine           = 0;
static ajint namErrorCount     = 0;

static AjPRegexp namNameExp = 0;
static AjPRegexp namVarExp  = NULL;




/* @datastatic NamPAttr *******************************************************
**
** Resource attribute definition structure
**
** @alias NamSAttr
** @alias NamOAttr
**
** @attr Name [const char*] Attribute name
** @attr Defval [const char*] Default value, usually an empty string
** @attr Comment [const char*] Comment for documentation purposes
** @@
******************************************************************************/

typedef struct NamSAttr
{
    const char* Name;
    const char* Defval;
    const char* Comment;
} NamOAttr;

#define NamPAttr NamOAttr*




/* @datastatic NamPValid ******************************************************
**
** Resource attribute validation structure
**
** @alias NamSValid
** @alias NamOValid
**
** @attr Name [const char*] Attribute name
** @attr Comment [const char*] Comment for documentation purposes
** @@
******************************************************************************/

typedef struct NamSValid
{
    const char* Name;
    const char* Comment;
} NamOValid;

#define NamPValid NamOValid*




NamOAttr namDbAttrs[] =
{
    {"format", "", "database entry format (required, at some level)"},
    {"method", "", "access method (required, at some level)"},
    {"type", "", "database type 'Nucleotide', 'Protein', etc (required)"},

    {"app", "", "external application commandline (APP, EXTERNAL)"},
    {"appall", "", "external commandline for 'methodall' (APP, EXTERNAL)"},
    {"appentry", "", "external commandline for 'methodentry' (APP, EXTERNAL)"},
    {"appquery", "", "external commandline for 'methodquery' (APP, EXTERNAL)"},

    {"caseidmatch", "N", "match exact case of entry identifier"},
    {"command", "", "command line to return entry/ies"},
    {"comment", "", "text comment for the DB definition"},
    {"dbalias", "", "database name to be used by access method if different"},
    {"description", "", "short database description"},
    {"directory", "", "data directory"},
    {"exclude", "", "wildcard filenames to exclude from 'filename'"},
    {"fields", "", "extra database fields available, ID and ACC are standard"},
    {"filename", "", "(wildcard) database filename"},

    {"formatall", "", "database entry format for 'methodall' access"},
    {"formatentry", "", "database entry format for 'methodentry' access"},
    {"formatquery", "", "database query format for 'methodquery' access"},

    {"hasaccession", "Y", "database has an acc field as an alternate id"},
    {"httpversion", "", "HTTP version for GET requests (URL, SRSWWW)"},
    {"identifier", "", "standard identifier (defaults to name)"},
    {"indexdirectory", "", "Index directory, defaults to data 'directory'"},

    {"methodall", "", "access method for all entries"},
    {"methodentry", "","access method for single entry"},
    {"methodquery", "", "access method for query (several entries)"},

    {"proxy", "", "http proxy server, or ':' to cancel a global proxy "
	          "(URL, SRSWWW)"},
    {"release", "", "release of the database, comment only"},
    {"url", "", "URL skeleton for entry level access (URL, SRSWWW)"},
    {NULL, NULL, NULL}
};

NamOAttr namRsAttrs[] =
{
    {"type", "", "resource type (required)"},

    {"identifier", "", "standard identifier (defaults to name)"},
    {"release", "", "release of the resource"},

    {"idlen", "",  "maximum ID length"},
    {"acclen", "", "maximum AC length"},
    {"svlen", "",  "maximum SV length"},
    {"keylen", "", "maximum KW length"},
    {"deslen", "", "maximum DE length"},
    {"orglen", "", "maximum TX length"},

    {"idpagesize",   "", "ID pagesize"},
    {"accpagesize",  "", "AC pagesize"},
    {"svpagesize",   "", "SV pagesize"},
    {"keypagesize",  "", "KW pagesize"},
    {"despagesize",  "", "DE pagesize"},
    {"orgpagesize",  "", "TX pagesize"},

    {"idcachesize",  "", "ID cachesize"},
    {"acccachesize", "", "AC cachesize"},
    {"svcachesize",  "", "SV cachesize"},
    {"keycachesize", "", "KW cachesize"},
    {"descachesize", "", "DE cachesize"},
    {"orgcachesize", "", "TX cachesize"},

    {"value", "", "value appropriate to the resource type"},
    {NULL, NULL, NULL}
};

NamOValid namDbTypes[] =
{
    {"N", "Nucleotide (obsolete short name)"},
    {"P", "Protein (obsolete short name)"},
    {"Nucleotide", "Nucleotide sequence data"},
    {"Protein", "Protein sequence data"},
    {"Pattern", "Pattern data"},
    {NULL, NULL}
};

NamOValid namRsTypes[] =
{
    {"Blast", "Blast database file"},
    {"Index", "EMBOSS database B+tree index"},
    {"List", "List of possible values"},
    {NULL, NULL}
};




/* @datastatic NamPEntry ******************************************************
**
** Internal database standard structure
**
** @alias NamSEntry
** @alias NamOEntry
**
** @attr name [AjPStr] token name
** @attr value [AjPStr] token value for variables
** @attr file [AjPStr] Short name for definitions file
** @attr data [void*] Attribute names and values for databases
** @@
******************************************************************************/

typedef struct NamSEntry
{
    AjPStr name;
    AjPStr value;
    AjPStr file;
    void* data;
} NamOEntry;

#define NamPEntry NamOEntry*




static ajint  namDbAttr(const AjPStr thys);
static ajint  namDbAttrC(const char* str);
static AjBool namDbSetAttrBool(const AjPStr* dbattr, const char* attrib,
			       AjBool* qrybool);
static AjBool namDbSetAttrStr(const AjPStr* dbattr, const char* attrib,
			      AjPStr* qrystr);
static void   namDebugDatabase(const AjPStr* dbattr);
static void   namDebugResource(const AjPStr* dbattr);
static void   namDebugVariables(void);
static void   namDebugMaster(const AjPTable table, ajint which);
static void   namEntryDelete(NamPEntry* pentry, ajint which);
static void   namError(const char* fmt, ...);
static void   namListParse(AjPList listwords, AjPList listcount,
			   AjPFile file, const AjPStr shortname);
static void   namListMaster(const AjPTable table, ajint which);
static void   namListMasterDelete(AjPTable table, ajint which);
static ajint  namMethod2Scope(const AjPStr method);
static void   namNoColon(AjPStr *thys);
static void   namPrintDatabase(const AjPStr* dbattr);
static void   namPrintResource(const AjPStr* rsattr);
static AjBool namProcessFile(AjPFile file, const AjPStr shortname);
static ajint  namRsAttr(const AjPStr thys);
static ajint  namRsAttrC(const char* str);
static void   namUser(const char *fmt, ...);
static AjBool namValid(const NamPEntry entry, ajint entrytype);
static AjBool namValidDatabase(const NamPEntry entry);
static AjBool namValidResource(const NamPEntry entry);
static AjBool namValidVariable(const NamPEntry entry);
static AjBool namVarResolve(AjPStr* var);




/* @funcstatic namEntryDelete *************************************************
**
** Deletes a variable, database, or resource entry from the internal table.
**
** @param [d] pentry [NamPEntry*] The entry to be deleted.
** @param [r] which [ajint] Internal table entry type
** @return [void]
** @@
******************************************************************************/

static void namEntryDelete(NamPEntry* pentry, ajint which)	
{

    ajint j;
    AjPStr* attrs;
    NamPEntry entry;

    entry = *pentry;

    ajStrDel(&entry->name);
    ajStrDel(&entry->value);
    ajStrDel(&entry->file);

    if(which == TYPE_DB)
    {
	attrs = (AjPStr *) entry->data;

	for(j=0; namDbAttrs[j].Name; j++)
	    ajStrDel(&attrs[j]);

	AJFREE(entry->data);
    }

    else if(which == TYPE_RESOURCE)
    {
	attrs = (AjPStr *) entry->data;

	for(j=0; namRsAttrs[j].Name; j++)
	    ajStrDel(&attrs[j]);

	AJFREE(entry->data);
    }

    else if(which == TYPE_ENV)
    {
    }

    AJFREE(entry);

    return;
}




/* @funcstatic namListMasterDelete *****************************************
**
** Deletes all databases in the internal table. The table is converted to
** an array, and each entry in turn is passed to namEntryDelete.
**
** @param [u] table [AjPTable] Table object
** @param [r] which [ajint] Internal table entry type
** @return [void]
** @@
******************************************************************************/

static void namListMasterDelete(AjPTable table, ajint which)
{
    ajint i;
    NamPEntry fnew = 0;
    void **keyarray = NULL;
    void **valarray = NULL;

    if(!table) return;

    ajTableToarray(table, &keyarray, &valarray);

    for(i = 0; keyarray[i]; i++)
    {
	AJFREE(keyarray[i]);		/* the key */
	fnew = (NamPEntry) valarray[i];
	namEntryDelete(&fnew, which);
    }

    AJFREE(keyarray);
    AJFREE(valarray);

    return;
}




/* @funcstatic namPrintDatabase ***********************************************
**
** Prints a report of defined attributes for a database definition.
**
** @param [r] dbattr [const AjPStr*] Attribute list from a database entry.
** @return [void]
** @@
******************************************************************************/

static void namPrintDatabase(const AjPStr* dbattr)
{
    ajint i;

    for(i=0; namDbAttrs[i].Name; i++)
	if(ajStrGetLen(dbattr[i]))
	    ajUser("\t%s: %S", namDbAttrs[i].Name, dbattr[i]);

    return;
}




/* @funcstatic namPrintResource ***********************************************
**
** Prints a report of defined attributes for a resource definition.
**
** @param [r] rsattr [const AjPStr*] Attribute list from a resource entry.
** @return [void]
** @@
******************************************************************************/

static void namPrintResource(const AjPStr* rsattr)
{
    ajint i;

    for(i=0; namRsAttrs[i].Name; i++) 
	if(ajStrGetLen(rsattr[i]))
	    ajUser("\t%s: %S", namRsAttrs[i].Name, rsattr[i]);

    return;
}




/* @func ajNamPrintDbAttr *****************************************************
**
** Prints a report of the database attributes available (for entrails)
**
** @param [u] outf [AjPFile] Output file
** @param [r] full [AjBool] Full output if AjTrue
** @return [void]
******************************************************************************/

void ajNamPrintDbAttr(AjPFile outf, AjBool full)
{
    ajint i;
    AjPStr tmpstr = NULL;
    ajuint maxtmp = 0;

    (void) full;			/* no extra detail to report */

    ajFmtPrintF(outf, "# Database attributes\n");
    ajFmtPrintF(outf, "# %-15s %-12s %s\n", "Attribute", "Default", "Comment");
    ajFmtPrintF(outf, "namDbAttrs {\n");

    for(i=0; namDbAttrs[i].Name; i++)
    {
	ajFmtPrintF(outf, "  %-15s", namDbAttrs[i].Name);
	ajFmtPrintS(&tmpstr, "\"%s\"", namDbAttrs[i].Defval);

	if(ajStrGetLen(tmpstr) > maxtmp)
	    maxtmp = ajStrGetLen(tmpstr);

	ajFmtPrintF(outf, " %-12S", tmpstr);
	ajFmtPrintF(outf, " \"%s\"\n", namDbAttrs[i].Comment);
    }

    if(maxtmp > 12)
        ajWarn("ajNamPrintDbAttr max tmpstr len %d",
               maxtmp);	      

    ajFmtPrintF(outf, "}\n\n");
    ajStrDel(&tmpstr);

    return;
}




/* @func ajNamPrintRsAttr *****************************************************
**
** Prints a report of the resource attributes available (for entrails)
**
** @param [u] outf [AjPFile] Output file
** @param [r] full [AjBool] Full output if AjTrue
** @return [void]
******************************************************************************/

void ajNamPrintRsAttr(AjPFile outf, AjBool full)
{
    ajint i;
    AjPStr tmpstr = NULL;
    ajuint maxtmp = 0;

    (void) full;			/* no extra detail to report */

    ajFmtPrintF(outf, "# Resource attributes\n");
    ajFmtPrintF(outf, "# %-15s %-12s %s\n", "Attribute", "Default", "Comment");
    ajFmtPrintF(outf, "namRsAttrs {\n");

    for(i=0; namRsAttrs[i].Name; i++)
    {
	ajFmtPrintF(outf, "  %-15s", namRsAttrs[i].Name);
	ajFmtPrintS(&tmpstr, "\"%s\"", namRsAttrs[i].Defval);

	if(ajStrGetLen(tmpstr) > maxtmp)
	    maxtmp = ajStrGetLen(tmpstr);

	ajFmtPrintF(outf, " %-12S", tmpstr);
	ajFmtPrintF(outf, " \"%s\"\n", namRsAttrs[i].Comment);
    }

    ajFmtPrintF(outf, "}\n\n");

    if(maxtmp > 12)
        ajWarn("ajNamPrintRsAttr max tmpstr len %d",
               maxtmp);	      

    ajStrDel(&tmpstr);

    return;
}




/* @funcstatic namDebugDatabase ***********************************************
**
** Prints a report of defined attributes for a database definition.
**
** @param [r] dbattr [const AjPStr*] Attribute list from a database entry.
** @return [void]
** @@
******************************************************************************/

static void namDebugDatabase(const AjPStr* dbattr)
{
    ajint i;

    for(i=0; namDbAttrs[i].Name; i++)
	if(ajStrGetLen(dbattr[i]))
	    ajDebug("\t%s: %S\n", namDbAttrs[i].Name, dbattr[i]);

    return;
}




/* @funcstatic namDebugResource ***********************************************
**
** Prints a report of defined attributes for a resource definition.
**
** @param [r] rsattr [const AjPStr*] Attribute list from a database entry.
** @return [void]
** @@
******************************************************************************/

static void namDebugResource(const AjPStr* rsattr)
{
    ajint i;

    for(i=0; namRsAttrs[i].Name; i++)
	if(ajStrGetLen(rsattr[i]))
	    ajDebug("\t%s: %S\n", namRsAttrs[i].Name, rsattr[i]);

    return;
}




/* @funcstatic namListMaster **************************************************
**
** Lists databases or variables defined in the internal table.
**
** @param [r] table [const AjPTable] internal table
** @param [r] which [ajint] Variable type, either TYPE_ENV for environment
**                        variables or TYPE_DB for databases or
**                        TYPE_RESOURCE for resources.
** @return [void]
** @@
******************************************************************************/

static void namListMaster(const AjPTable table, ajint which)
{
    ajint i;
    NamPEntry fnew;
    void **keyarray = NULL;
    void **valarray = NULL;
    char *key;

    ajTableToarray(table, &keyarray, &valarray);

    for(i = 0; keyarray[i]; i++)
    {
	key = (char*) keyarray[i];
	fnew =(NamPEntry) valarray[i];

	if(TYPE_DB == which)
	{
	    ajUser("DB %S\t *%s*", fnew->name, key);
	    namPrintDatabase(fnew->data);
	    ajUserDumpC("");
	}
	else if(TYPE_RESOURCE == which) 
	{
	    ajUser("RES %S\t *%s*", fnew->name, key);
	    namPrintResource(fnew->data);
	    ajUserDumpC("");
	}
	else if(TYPE_ENV == which)
	    ajUser("ENV %S\t%S\t *%s*",fnew->name,fnew->value,key);
    }

    AJFREE(keyarray);
    AJFREE(valarray);

    return;
}




/* @funcstatic namDebugMaster **********************************************
**
** Lists databases or variables defined in the internal table.
**
** @param [r] table [const AjPTable] Table object
** @param [r] which [ajint] Variable type, either TYPE_ENV for environment
**                        variables or TYPE_DB for databases or
**                        TYPE_RESOURCE for resources.
** @return [void]
** @@
******************************************************************************/

static void namDebugMaster(const AjPTable table, ajint which)
{
    ajint i;
    NamPEntry fnew;
    void **keyarray = NULL;
    void **valarray = NULL;
    char *key;

    ajTableToarray(table, &keyarray, &valarray);

    for(i = 0; keyarray[i]; i++)
    {
	key = (char*) keyarray[i];
	fnew = (NamPEntry) valarray[i];

	if(TYPE_DB == which)
	{
	    ajDebug("DB %S\t *%s*\n", fnew->name, key);
	    namDebugDatabase(fnew->data);
	    ajDebug("\n");
	}

	else if(TYPE_RESOURCE == which)
	{
	    ajDebug("RES %S\t *%s*\n", fnew->name, key);
	    namDebugResource(fnew->data);
	    ajDebug("\n");
	}

	else if(TYPE_ENV == which)
	    ajDebug("ENV %S\t%S\t *%s*\n",fnew->name,fnew->value,key);
    }

    AJFREE(keyarray);
    AJFREE(valarray);

    return;
}




/* @func ajNamDbDetails *******************************************************
**
** Returns database access method information
**
** @param [r] name [const AjPStr] Database name
** @param [w] type [AjPStr*] sequence type - 'P' or 'N'
** @param [w] id [AjBool*] ajTrue = can access single entries
** @param [w] qry [AjBool*] ajTrue = can access wild/query entries
** @param [w] all [AjBool*] ajTrue = can access all entries
** @param [w] comment [AjPStr*] comment about database
** @param [w] release [AjPStr*] database release date
** @param [w] methods [AjPStr*] database access methods formatted
** @param [w] defined [AjPStr*] database definition file short name
** @return [AjBool] ajTrue if database details were found
** @@
******************************************************************************/

AjBool ajNamDbDetails(const AjPStr name, AjPStr* type, AjBool* id,
		      AjBool* qry, AjBool* all,
		      AjPStr* comment, AjPStr* release,
		      AjPStr* methods, AjPStr* defined)
{
    NamPEntry fnew = 0;
    AjPStr* dbattr = NULL;
    ajint i;
    ajint scope;
    
    *id = *qry = *all = ajFalse;
    
    ajStrDelStatic(type);
    ajStrDelStatic(comment);
    ajStrDelStatic(release);
    ajStrDelStatic(methods);
    ajStrDelStatic(defined);
    
    fnew = ajTableFetch(namDbMasterTable, ajStrGetPtr(name));

    if(fnew)
    {
	ajDebug("ajNamDbDetails '%S' found\n", name);

	ajStrAssignS(defined, fnew->file);

	dbattr = (AjPStr *) fnew->data;

	for(i=0; namDbAttrs[i].Name; i++)
	{
	    ajDebug("Attribute name = %s, value = %S\n",
                    namDbAttrs[i].Name, dbattr[i]);

	    if(ajStrGetLen(dbattr[i]))
	    {
		if(!strcmp("type", namDbAttrs[i].Name))
		    ajStrAssignS(type, dbattr[i]);

		if(!strcmp("method", namDbAttrs[i].Name))
		{
		    scope = namMethod2Scope(dbattr[i]);

		    if(scope & METHOD_ENTRY)
                        *id = ajTrue;

		    if(scope & METHOD_QUERY)
                        *qry = ajTrue;

		    if(scope & METHOD_ALL)
                        *all = ajTrue;

		    ajStrAppendS(methods, dbattr[i]);
		}

		if(!strcmp("methodentry", namDbAttrs[i].Name))
		{
		    scope = namMethod2Scope(dbattr[i]);

		    if(scope & METHOD_ENTRY)
                        *id = ajTrue;

		    if(ajStrGetLen(*methods))
			ajStrAppendC(methods, ",");

		    ajStrAppendS(methods, dbattr[i]);
		    ajStrAppendC(methods, "(id)");
		}

		if(!strcmp("methodquery", namDbAttrs[i].Name))
		{
		    scope = namMethod2Scope(dbattr[i]);

		    if(scope & METHOD_ENTRY)
                        *id = ajTrue;

		    if(scope & METHOD_QUERY)
                        *qry = ajTrue;

		    if(ajStrGetLen(*methods))
			ajStrAppendC(methods, ",");

		    ajStrAppendS(methods, dbattr[i]);
		    ajStrAppendC(methods, "(qry)");
		}

		if(!strcmp("methodall", namDbAttrs[i].Name))
		{
		    scope = namMethod2Scope(dbattr[i]);

		    if(scope & METHOD_ALL)
                        *all = ajTrue;

		    if(ajStrGetLen(*methods))
			ajStrAppendC(methods, ",");

		    ajStrAppendS(methods, dbattr[i]);
		    ajStrAppendC(methods, "(all)");
		}

		if(!strcmp("comment", namDbAttrs[i].Name))
		    ajStrAssignS(comment, dbattr[i]);

		if(!strcmp("release", namDbAttrs[i].Name))
		    ajStrAssignS(release, dbattr[i]);
	    }
	}
	
	if(!ajStrGetLen(*type))
	{
	    ajWarn("Bad database definition for %S: No type. 'P' assumed",
		   name);
	    ajStrAssignC(type, "P");
	}

	if(!*id && !*qry && !*all)
	    ajWarn("Bad database definition for %S: No method(s) for access",
		   name);
	
	return ajTrue;
    }
    
    ajDebug("  '%S' not found\n", name);

    return ajFalse;
}




/* @funcstatic namMethod2Scope ************************************************
**
** Returns OR'ed values of METHOD_ENTRY, METHOD_QUERY and METHOD_ALL
** for the various types of access method for databases.
**
** @param [r] method [const AjPStr] Access method string
** @return [ajint] OR'ed values for the valid scope of the access method given
** @@
******************************************************************************/

static ajint namMethod2Scope(const AjPStr method)
{

    ajint result = 0;

    if(!ajStrCmpC(method, "dbfetch"))
	result = METHOD_ENTRY;
    else if(!ajStrCmpC(method, "emboss"))
	result = (METHOD_ENTRY | METHOD_QUERY | METHOD_ALL);
    else if(!ajStrCmpC(method, "emblcd"))
	result = (METHOD_ENTRY | METHOD_QUERY | METHOD_ALL);
    else if(!ajStrCmpC(method, "srs"))
	result = (METHOD_ENTRY | METHOD_QUERY);
    else if(!ajStrCmpC(method, "mrs"))
	result = (METHOD_ENTRY | METHOD_QUERY);
    else if(!ajStrCmpC(method, "mrs3"))
	result = (METHOD_ENTRY | METHOD_QUERY);
    else if(!ajStrCmpC(method, "srsfasta"))
	result = (METHOD_ENTRY | METHOD_QUERY);
    else if(!ajStrCmpC(method, "srswww"))
	result = METHOD_ENTRY;
    else if(!ajStrCmpC(method, "mrs"))
	result = (METHOD_ENTRY | METHOD_QUERY | METHOD_ALL);
    else if(!ajStrCmpC(method, "entrez"))
	result = (METHOD_ENTRY | METHOD_QUERY);
    else if(!ajStrCmpC(method, "seqhound"))
	result = (METHOD_ENTRY | METHOD_QUERY);
    else if(!ajStrCmpC(method, "url"))
	result = METHOD_ENTRY;
    else if(!ajStrCmpC(method, "app"))
	result = (METHOD_ENTRY | METHOD_QUERY | METHOD_ALL);
    else if(!ajStrCmpC(method, "external"))
	result = (METHOD_ENTRY | METHOD_QUERY | METHOD_ALL);
    else if(!ajStrCmpC(method, "direct"))
	result = (METHOD_ALL | SLOW_QUERY | SLOW_ENTRY );
    else if(!ajStrCmpC(method, "gcg"))
	result = (METHOD_ENTRY | METHOD_QUERY | METHOD_ALL);
    else if(!ajStrCmpC(method, "embossgcg"))
	result = (METHOD_ENTRY | METHOD_QUERY | METHOD_ALL);
    else if(!ajStrCmpC(method, "blast"))
	result = (METHOD_ENTRY | METHOD_QUERY | METHOD_ALL);
    /* not in ajseqdb seqAccess list */
    /*
       else if(!ajStrCmpC(method, "corba"))
       result = (METHOD_ENTRY | METHOD_QUERY | METHOD_ALL);
       */

    return result;
}




/* @func ajNamListOrigin ******************************************************
**
** Writes a simple list of where the internal tables came from..
**
** @return [void]
** @@
******************************************************************************/

void ajNamListOrigin(void)
{
    ajUserDumpC("SOURCE---------->");
    ajUserDumpS(namFileOrig);
    ajUserDumpC("SOURCE---------->");
    ajUserDumpC("");

    return;
}




/* @func ajNamDebugOrigin *****************************************************
**
** Writes a simple list of where the internal tables came from..
**
** @return [void]
** @@
******************************************************************************/

void ajNamDebugOrigin(void)
{
    ajDebug("Defaults and .rc files: %S\n", namFileOrig);

    return;
}




/* @func ajNamListDatabases ***************************************************
**
** Writes a simple list of all databases in the internal table.
**
** @return [void]
** @@
******************************************************************************/

void ajNamListDatabases(void)
{
    ajUserDumpC("DB---------->");
    namListMaster(namDbMasterTable, TYPE_DB);
    ajUserDumpC("DB---------->");
    ajUserDumpC("");

    return;
}




/* @func ajNamDebugDatabases **************************************************
**
** Writes a simple debug report of all databases in the internal table.
**
** @return [void]
** @@
******************************************************************************/

void ajNamDebugDatabases(void)
{
    ajDebug("DB databases\n");
    ajDebug("============\n");
    namDebugMaster(namDbMasterTable, TYPE_DB);
    ajDebug("[DB done]\n\n");

    return;
}




/* @func ajNamDebugResources **************************************************
**
** Writes a simple debug report of all databases in the internal table.
**
** @return [void]
** @@
******************************************************************************/

void ajNamDebugResources(void)
{
    ajDebug("RES resources\n");
    ajDebug("=============\n");
    namDebugMaster(namResMasterTable, TYPE_RESOURCE);
    ajDebug("[RES done]\n\n");

    return;
}




/* @func ajNamDebugVariables *********************************************
**
** Writes a simple debug report of all envornment variables
** in the internal table.
**
** @return [void]
** @@
******************************************************************************/

void ajNamDebugVariables(void)
{
    ajDebug("ENV variables\n");
    ajDebug("=============\n");
    namDebugMaster(namVarMasterTable, TYPE_ENV);
    ajDebug("[ENV done]\n\n");

    return;
}




/* @func ajNamListListDatabases ***********************************************
**
** Creates a AjPList list of all databases in the internal table.
**
** @param [w] dbnames [AjPList] Str List of names to be populated
** @return [void]
** @@
******************************************************************************/

void ajNamListListDatabases(AjPList dbnames)
{
    ajint i;
    NamPEntry fnew;
    void **keyarray =  NULL;
    void **valarray =  NULL;

    ajTableToarray(namDbMasterTable, &keyarray, &valarray);
    ajDebug("ajNamListListDatabases\n");

    for(i = 0; valarray[i]; i++)
    {
	fnew = (NamPEntry) valarray[i];
	ajDebug("DB: %S\n", fnew->name);
	ajListstrPushAppend(dbnames, fnew->name);
    }

    AJFREE(keyarray);
    AJFREE(valarray);

    return;
}




/* @func ajNamListListResources ***********************************************
**
** Creates a AjPList list of all databases in the internal table.
**
** @param [w] rsnames [AjPList] Str List of names to be populated
** @return [void]
** @@
******************************************************************************/

void ajNamListListResources(AjPList rsnames)
{
    ajint i;
    NamPEntry fnew;
    void **keyarray = NULL;
    void **valarray = NULL;

    ajTableToarray(namResMasterTable, &keyarray, &valarray);

    for(i = 0; valarray[i]; i++)
    {
	fnew = (NamPEntry) valarray[i];
	ajDebug("RES: %S\n", fnew->name);
	ajListstrPushAppend(rsnames, fnew->name);
    }

    AJFREE(keyarray);
    AJFREE(valarray);

    return;
}




/* @func ajNamVariables **************************************************
**
** Writes a simple list of all variables in the internal table.
**
** @return [void]
** @@
******************************************************************************/

void ajNamVariables(void)
{
    ajUserDumpC("ENV---------->");
    namListMaster(namDbMasterTable, TYPE_ENV);
    ajUserDumpC("ENV---------->");
    ajUserDumpC("");

    return;
}




/* @funcstatic namDebugVariables *****************************************
**
** Writes a simple list of all variables in the internal table.
**
** @return [void]
** @@
******************************************************************************/

static void namDebugVariables(void)
{
    namUser("ENV---------->\n");
    namDebugMaster(namDbMasterTable, TYPE_ENV);
    namUser("ENV---------->\n");
    namUser("\n");

    return;
}




/* @funcstatic namListParse ***************************************************
**
** Parse the text in a list of tokens read from the input file.
** Derive environment variable and database definitions. Store
** all these in the internal tables.
**
** @param [u] listwords [AjPList] String list of word tokens to parse
** @param [u] listcount [AjPList] List of word counts per line for
**                                generating error messages
** @param [u] file [AjPFile] Input file only for name in messages
** @param [r] shortname [const AjPStr] Definition file short name
** @return [void]
** @@
******************************************************************************/

static void namListParse(AjPList listwords, AjPList listcount,
			 AjPFile file, const AjPStr shortname)
{
    static char* tabname   = 0;
    static AjPStr name     = 0;
    static AjPStr value    = 0;
    static char quoteopen  = 0;
    static char quoteclose = 0;
    static AjPStr* dbattr  = 0;
    static AjPStr* rsattr  = 0;
    static ajint  db_input = -1;
    static ajint  rs_input = -1;
    AjPTable saveTable = NULL;

    NamPEntry fnew  = NULL;
    NamPEntry entry = NULL;

    AjBool dbsave = ajFalse;
    AjBool rssave = ajFalse;
    AjBool saveit = ajFalse;
    ajint ndbattr = 0;
    ajint nrsattr = 0;

    AjPStr includefn = NULL;
    AjPFile iinf     = NULL;
    AjPStr key       = NULL;
    AjPStr val       = NULL;

    static AjPTable Ifiles = NULL;
    AjPStr curword;

    ajint wordcount = 0;
    ajint linecount = 0;
    ajint lineword  = 0;
    ajint *iword    = NULL;
    AjBool namstatus;
    AjPStr saveshortname = NULL;

    /* ndbattr = count database attributes */
    if(!ndbattr)
	for(ndbattr=0; namDbAttrs[ndbattr].Name; ndbattr++);

    /* nrsattr = count resource attributes */
    if(!nrsattr)
	for(nrsattr=0; namRsAttrs[nrsattr].Name; nrsattr++);

    ajStrAssignS(&saveshortname, shortname);
    ajStrDel(&name);
    ajStrDel(&value);
    quoteopen  = 0;
    quoteclose = 0;
    
    namLine = 1;
    namUser("namListParse of %F '%S' words: %d lines: %d\n", 
	    file, name, ajListGetLength(listwords), ajListGetLength(listcount));
    
    while(ajListstrPop(listwords, &curword))
    {
	while(ajListGetLength(listcount) && (lineword < wordcount))
	{
	    namUser("ajListPop %d < %d list %d\n",
		    lineword, wordcount, ajListGetLength(listcount));
	    ajListPop(listcount, (void**) &iword);
	    lineword = *iword;
	    linecount++;
	    namLine = linecount-1;
	    AJFREE(iword);
	}

	wordcount++;
	namUser("namListParse word: %d line: %d (%d) <%S>\n",
		wordcount, namLine, lineword, curword);
	
	if(!namParseType)
	{
	    namNoColon(&curword);
	    ajStrFmtLower(&curword);

	    if(ajCharPrefixS("env", curword))
		namParseType = TYPE_ENV;

	    if(ajCharPrefixS("setenv", curword))
		namParseType = TYPE_ENV;
	    else if(ajCharPrefixS("dbname",curword))
		namParseType = TYPE_DB;
	    else if(ajCharPrefixS("resource",curword))
		namParseType = TYPE_RESOURCE;
	    else if(ajCharPrefixS("include",curword))
		namParseType = TYPE_IFILE;

	    if(!namParseType)		/* test: badtype.rc */
		namError("Invalid definition type '%S'", curword);

	    namUser("type set to %s curword '%S'\n",
		    namTypes[namParseType], curword);
	}
	else if(quoteopen && ajStrMatchC(curword, "]"))
	{				/* test; dbnoquote.rc */
	    namError("']' found, unclosed quotes in '%S'\n", value);
	    quoteopen    = 0;
	    namParseType = 0;
	}
	else if(quoteopen)
	{
	    /*
	    ** quote is open, so append word until close quote is found,
	    ** and set the appropriate save flag
	    */
	    namUser("<%c>..<%c> quote processing\n", quoteopen, quoteclose);
	    ajStrAppendC(&value," ");
	    ajStrAppendS(&value,curword);

	    /* close quote here ?? */
	    if(ajStrGetCharLast(curword) == quoteclose)
	    {
		namUser("close quotes\n");
		ajStrCutEnd(&value, 1);
		quoteopen = quoteclose = 0;

		if(namParseType == TYPE_ENV) /* set save flag, value found */
		    saveit = ajTrue;
		else if(namParseType == TYPE_DB)
		    dbsave = ajTrue;
		else if(namParseType == TYPE_RESOURCE)
		    rssave = ajTrue;
	    }
	}
	else if(namParseType == TYPE_ENV)
	{
	    if(name && value)
		saveit= ajTrue;
	    else if(name)
	    {
		/* if name already got then it must be the value */
		if(ajStrGetCharFirst(curword) == '\'')
		    quoteopen = quoteclose = '\'';
		else if(ajStrGetCharFirst(curword) == '\"')
		    quoteopen = quoteclose = '\"';

		ajStrAssignS(&value, curword);

		if(quoteopen)
		{
		    /* trim the opening quote */
		    ajStrCutStart(&value, 1);

		    if(!ajStrGetLen(value))
			ajErr("Bare quote %c found in namListParse",
			       quoteopen);
		}
		else
		    saveit = ajTrue;

		if(ajStrGetCharLast(curword) == quoteclose)
		{
		    /* end of quote on same word */
		    quoteopen = quoteclose = 0;
		    saveit= ajTrue;
		    /* remove quote at the end */
		    ajStrCutEnd(&value, 1);
		}
		namUser("save value '%S'\n", value);
	    }
	    else
	    {
		ajStrAssignS(&name, curword);
		namUser("save name '%S'\n", name);
	    }
	}
	
	
	else if(namParseType == TYPE_DB)
	{
	    if(ajStrMatchC(curword, "[")) /* [ therefore new database */
		dbattr = AJCALLOC0(ndbattr, sizeof(AjPStr)); /* new db obj */
	    else if(ajStrMatchC(curword, "]"))	/* ] therefore end of db */
		saveit = ajTrue;
	    else if(name)
	    {
		if(ajStrGetCharLast(curword) == ':')
		{
		    /* if last character is : then its a keyword */
		    ajStrFmtLower(&curword); /* make it lower case */
		    namNoColon(&curword);
		    db_input = namDbAttr(curword);

		    if(db_input < 0)
			ajWarn("%S: bad attribute '%S' for database '%S'\n",
				namRootStr, curword, name);
		}
		else if(db_input >= 0)
		{
		    /* So if keyword type has been set */
		    if(ajStrGetCharFirst(curword) == '\'')
		    {
			/* is there a quote? If so expect the */
			/* same at the end. No ()[] etc here*/
			quoteopen = quoteclose = '\'';
		    }
		    else if(ajStrGetCharFirst(curword) == '\"')
			quoteopen = quoteclose = '\"';

		    ajStrAssignS(&value, curword);

		    if(quoteopen)
		    {
			ajStrCutStart(&value, 1); /* trim opening quote */

			if(!ajStrGetLen(value))
			    ajErr("Bare quote %c found in namListParse",
				   quoteopen);
		    }
		    else
			dbsave = ajTrue; /* we are done - simple word */

		    if(ajStrGetCharLast(curword) == quoteclose)
		    {
			quoteopen = quoteclose = 0;
			ajStrCutEnd(&value,1); /* trim closing quote */
			dbsave = ajTrue;
		    }

		    if(!quoteopen)     /* if we just reset it above */
			dbsave = ajTrue;
		}
	    }
	    else
	    {
		ajStrAssignS(&name, curword);

		if(!ajNamIsDbname(name))
		    ajErr("Invalid database name '%S'", name);

		namUser("saving db name '%S'\n", name);
	    }
	}
	
	
	else if(namParseType == TYPE_RESOURCE)
	{
	    if(ajStrMatchC(curword, "[")) /* [ therefore new resource */
		rsattr = AJCALLOC0(nrsattr, sizeof(AjPStr)); /* new resource*/
	    else if(ajStrMatchC(curword, "]"))	/* ] end of resource */
		saveit = ajTrue;
	    else if(name)
	    {
                /* if last character is : */
		if(ajStrGetCharLast(curword) == ':')
		{		     	     /* then it is a keyword */
		    ajStrFmtLower(&curword); /* make it lower case */
		    namNoColon(&curword);
		    rs_input = namRsAttr(curword);

		    if(rs_input < 0)	/* test: badresattr.rc */
			namError("Bad attribute '%S' for resource '%S'",
				  curword, name);
		}
		else if(rs_input >= 0)
		{		 /* So if keyword type has been set */
		    if(ajStrGetCharFirst(curword) == '\'')
		    {	      /* is there a quote? If so expect the */
			/* same at the end. No ()[] etc here */
			quoteopen = quoteclose = '\'';
		    }
		    else if(ajStrGetCharFirst(curword) == '\"')
			quoteopen = quoteclose = '\"';

		    ajStrAssignS(&value, curword);

		    if(quoteopen)
		    {
			ajStrCutStart(&value, 1); /* trim opening quote */

			if(!ajStrGetLen(value))
			    ajErr("Bare quote %c found in namListParse",
				   quoteopen);
		    }
		    else
			rssave = ajTrue;

		    if(ajStrGetCharLast(curword) == quoteclose)
		    {
			quoteopen = quoteclose = 0;
			ajStrCutEnd(&value,1); /* ignore quote if */
					             /* one at end */
			rssave = ajTrue;
		    }

		    if(!quoteopen)
			rssave = ajTrue;
		}
	    }
	    else
	    {
		ajStrAssignS(&name, curword);
		namUser("saving resource name '%S'\n", name);
	    }
	}
	
	
	else if(namParseType == TYPE_IFILE)
	{
	    if(!Ifiles)
		Ifiles = ajTablestrNewLen(NAM_INCLUDE_ESTIMATE);
	    namParseType = 0;

	    if(ajTableFetch(Ifiles,curword)) /* test: includeagain.rc */
		namError("%S already read .. skipping\n", curword);
	    else
	    {
		includefn = ajStrNew();
		ajStrAssignS(&includefn,curword);

		if(namFileOrig)
		    ajStrAppendC(&namFileOrig,", ");
		ajStrAppendS(&namFileOrig,includefn);

		key = ajStrNewC(ajStrGetPtr(includefn));
		val = ajStrNewC(ajStrGetPtr(includefn));
		ajTablePut(Ifiles,key,val);

                /* test: badinclude.rc */
		if(!(iinf = ajFileNewInNameS(includefn)))
		{
		    namError("Failed to open include file '%S'\n", includefn);
		    ajStrAppendC(&namFileOrig,"(Failed)");
		}
		else
		{
		    ajStrAppendC(&namFileOrig,"(OK)");
                    /* replaces namFile */
		    namstatus = namProcessFile(iinf, name);
		    ajFmtPrintS(&namFileName, "%F",file);/* reset saved name */
		    namLine = linecount-1;

		    if(!namstatus)	/* test: badsummary.rc */
			namError("Error(s) found in included file %F", iinf);

		    ajFileClose(&iinf);
		}

		ajStrDel(&includefn);
	    }

	    namListParseOK = ajTrue;
	}
	
	
	if(dbsave)
	{
	    /* Save the keyword value */
	    ajStrAssignS(&dbattr[db_input], value);
	    db_input =-1;
	    ajStrDel(&value);
	    dbsave = ajFalse;
	}
	
	if(rssave)
	{
	    /* Save the keyword value */
	    ajStrAssignS(&rsattr[rs_input], value);
	    rs_input =-1;
	    ajStrDel(&value);
	    rssave = ajFalse;
	}
	
	namListParseOK = saveit;
	
	if(saveit)
	{
	    namUser("saving type %d name '%S' value '%S' line:%d\n",
		    namParseType, name, value, namLine);
	    AJNEW0(fnew);
	    tabname = ajCharNewS(name);
	    fnew->name = name;
	    name = 0;
	    fnew->value = value;
	    value = 0;
	    fnew->file = ajStrNewRef(saveshortname);

	    if(namParseType == TYPE_DB)
	    {
		fnew->data = (AjPStr *) dbattr;
		saveTable = namDbMasterTable;
	    }
	    else if(namParseType == TYPE_RESOURCE)
	    {
		fnew->data = (AjPStr *) rsattr;
		saveTable = namResMasterTable;
	    }
	    else if(namParseType == TYPE_ENV)
	    {
		fnew->data = NULL;
		saveTable = namVarMasterTable;
	    }
	    else
	    {
		fnew->data = NULL;
	    }

	    /* Validate the master table entry */

	    if(namDoValid)
		namValid(fnew, namParseType);

	    /*
	    ** Add new one to table 
	    ** be very careful that everything in the table 
	    ** is not about to be deallocated - so do not use "name" here
	    */

	    entry = ajTablePut(saveTable, tabname, fnew);
	    if(entry)
	    {
		/* it existed so over wrote previous table entry
		** Only a namUser message - redefining EMBOSSRC in
		** QA testing can give too many warnings
		*/
		namUser("%S: replaced %s %S definition from %S\n",
			shortname,
			namTypes[namParseType],
			entry->name,
			entry->file);
		namEntryDelete(&entry, namParseType); /* previous entry */
		AJFREE(tabname);        /* ajTablePut reused the old key */
	    }
	    
	    saveit = ajFalse;
	    namParseType = 0;
	    db_input = -1;
	    dbattr = 0;
	}

	ajStrDel(&curword);
    }
    
    if(namParseType)
    {
	/* test: badset.rc baddb.rc  */
	namError("Unexpected end of file in %s definition",
		 namTypes[namParseType]);
	namParseType = 0;
    }
    
    if(ajListGetLength(listcount))	/* cleanup the wordcount list */
    {
	namUser("** remaining wordcount items: %d\n",
                ajListGetLength(listcount));

	while(ajListGetLength(listcount))
	{
	    ajListPop(listcount, (void**) &iword);
	    AJFREE(iword);
	}
    }

    if(value)
    {
	namUser("++ namListParse value %x '%S'", value, value);
    }

    ajStrDel(&saveshortname);
    ajTablestrFree(&Ifiles);

    return;
}




/* @func ajNamIsDbname ********************************************************
**
** Returns true if the name is a valid database name.
** 
** Database names must start with a letter, and have 1 or more letters,
** numbers or underscores. No other characters are permitted.
**
** @param [r] name [const AjPStr] character string to find in getenv list
** @return [AjBool] True if name was defined.
** @@
**
******************************************************************************/

AjBool ajNamIsDbname(const AjPStr name)
{
    const char* cp = ajStrGetPtr(name);

    if (!*cp)
	return ajFalse;

    if (!isalpha((int)*cp++))
	return ajFalse;

    if (!*cp)
	return ajFalse;

    while (*cp)
    {
	if(!isalnum((int)*cp) && (*cp != '_'))
	    return ajFalse;

	cp++;
    }

    return ajTrue;
}




/* @func ajNamGetenvC *********************************************************
**
** Looks for name as an environment variable.
** the AjPStr for this in "value". If not found returns value as NULL;
**
** @param [r] name [const char*] character string to find in getenv list
** @param [w] value [AjPStr*] String for the value.
** @return [AjBool] True if name was defined.
** @@
**
******************************************************************************/

AjBool ajNamGetenvC(const char* name,
		    AjPStr* value)
{
    char *envval;

    envval = getenv(name);

    if(envval)
    {
	ajStrAssignC(value, envval);

	return ajTrue;
    }

    ajStrDel(value);

    return ajFalse;
}




/* @func ajNamGetenvS **********************************************************
**
** Looks for name as an environment variable.
** the AjPStr for this in "value". If not found returns NULL;
**
** @param [r] name [const AjPStr] character string to find in getenv list
** @param [w] value [AjPStr*] String for the value.
** @return [AjBool] True if name was defined.
** @@
**
******************************************************************************/

AjBool ajNamGetenvS(const AjPStr name,
		    AjPStr* value)
{
    return ajNamGetenvC(ajStrGetPtr(name), value);
}




/* @obsolete ajNamGetenv
** @remove Use ajNamGetenvS
*/
__deprecated AjBool ajNamGetenv(const AjPStr name,
		    AjPStr* value)
{
    return ajNamGetenvS(name, value);
}




/* @func ajNamGetValueC *******************************************************
**
** Looks for name as an (upper case) environment variable,
** and then as-is in the hash table and if found returns
** the AjPStr for this in "value". If not found returns NULL;
**
** @param [r] name [const char*] character string find in hash table.
** @param [w] value [AjPStr*] Str for the value.
** @return [AjBool] True if name was defined.
** @@
**
******************************************************************************/

AjBool ajNamGetValueC(const char* name, AjPStr* value)
{
    NamPEntry fnew       = 0;
    AjBool hadPrefix     = ajFalse;
    AjBool ret           = ajFalse;
    
    if(ajCharPrefixS(name, namPrefixStr)) /* may already have the prefix */
    {
	ajStrAssignC(&namValNameTmp, name);
	hadPrefix = ajTrue;
    }
    else
    {
	ajStrAssignS(&namValNameTmp, namPrefixStr);
	ajStrAppendC(&namValNameTmp, name);
    }

    /* upper case for ENV, otherwise don't care */
    ajStrFmtUpper(&namValNameTmp);
    
    /* first test for an ENV variable */
    
    ret = ajNamGetenvS(namValNameTmp, value);
    if(ret)
	return ajTrue;

    /* then test the table definitions - with the prefix */
    
    fnew = ajTableFetch(namVarMasterTable, ajStrGetPtr(namValNameTmp));

    if(fnew)
    {
	ajStrAssignS(value, fnew->value);

	return ajTrue;
    }
    
    if(!hadPrefix)
    {

	/* then test the table definitions - as originally specified */

	fnew = ajTableFetch(namVarMasterTable, name);

	if(fnew)
	{
	    ajStrAssignS(value, fnew->value);

	    return ajTrue;
	}
    }
    
    return ajFalse;
}




/* @func ajNamGetValueS ********************************************************
**
** Looks for name as an (upper case) environment variable,
** and then as-is in the hash table and if found returns
** the AjPStr for this in "value". If not found returns NULL;
**
** @param [r] name [const AjPStr] character string find in hash table.
** @param [w] value [AjPStr*] String for the value.
** @return [AjBool] True if name was defined.
** @@
**
******************************************************************************/

AjBool ajNamGetValueS(const AjPStr name, AjPStr* value)
{
    return ajNamGetValueC(ajStrGetPtr(name), value);
}




/* @obsolete ajNamGetValue
** @remove Use ajNamGetValusS
*/

__deprecated AjBool ajNamGetValue(const AjPStr name, AjPStr* value)
{
    return ajNamGetValueC(ajStrGetPtr(name), value);
}




/* @func ajNamDatabase ********************************************************
**
** Looks for name in the hash table and if found returns
** the attribute for this. If not found returns  NULL;
**
** @param [r] name [const AjPStr] character string find in hash table.
** @return [AjBool] true if database name is valid.
** @error  NULL if name not found in the table
** @@
**
******************************************************************************/

AjBool ajNamDatabase(const AjPStr name)
{
    NamPEntry fnew = 0;

    /* ajDebug("ajNamDatabase '%S'\n", name); */

    fnew = ajTableFetch(namDbMasterTable, ajStrGetPtr(name));

    if(fnew)
    {
	/* ajDebug("  '%S' found\n", name); */
	return ajTrue;
    }

    /* ajDebug("  '%S' not found\n", name); */
    return ajFalse;
}




/* @funcstatic namProcessFile *************************************************
**
** Read the definitions file and append each token to the list.
**
** @param [u] file [AjPFile] Input file object
** @param [r] shortname [const AjPStr] Definitions file short name
** @return [AjBool] ajTrue if no error were found
** @@
******************************************************************************/

static AjBool namProcessFile(AjPFile file, const AjPStr shortname)
{
    AjPStr rdline = NULL;
    AjPStr word   = NULL;
    const char *ptr;
    ajint i = 0;
    ajint len;
    char quote = '\0';
    AjPList listwords;
    AjPList listcount;
    AjPStr wordptr;
    ajint iline = 0;
    ajint *k    = NULL;
    
    ajint preverrorcount = namErrorCount;
    
    listwords = ajListstrNew();
    listcount = ajListNew();
    word      = ajStrNewRes(128);
    
    ajFmtPrintS(&namFileName, "%F", file);
    namUser("namProcessFile '%F'\n", file);
    
    /* Read in the settings. */
    while(ajReadlineTrim(file, &rdline))
    {
	
	AJNEW0(k);
	*k = ajListGetLength(listwords);
	ajListPushAppend(listcount, k);
	
	/* namUser("%S\n",rdline); */
	len = ajStrGetLen(rdline);
	
        /* Ignore if the line is a comment */
	if(!ajStrCutCommentsStart(&rdline))
	    continue;
	
	/* now create a linked list of the "words" */
	if(len)
	{
	    ptr = ajStrGetPtr(rdline);
	    i = 0;

	    while(*ptr && i < len)
	    {
		if(*ptr == ' ' || *ptr == '\t')
		{
		    if(ajStrGetLen(word))
		    {
			wordptr = ajStrNewS(word);
			ajListstrPushAppend(listwords, wordptr);
			ajStrAssignClear(&word);
		    }
		    i++;
		    ptr++;
		    continue;
		}
		else if(*ptr == '\'' || *ptr == '\"')
		{
		    ajStrAppendK(&word,*ptr);

		    if(quote)
		    {
			if(quote == *ptr)
			    quote = '\0';
		    }
		    else
			quote = *ptr;
		}
		else if(!quote && ajStrGetLen(word) && *ptr == ']')
		{
		    wordptr = ajStrNewS(word);
		    ajListstrPushAppend(listwords, wordptr);
		    ajStrAssignClear(&word);
		    wordptr = ajStrNewC("]");
		    ajListstrPushAppend(listwords, wordptr);
		    ajStrAssignClear(&word);
		}
		else
		    ajStrAppendK(&word,*ptr);
		i++;ptr++;
	    }

	    if(ajStrGetLen(word))
	    {
		wordptr = ajStrNewS(word);
		ajListstrPushAppend(listwords, wordptr);
		ajStrAssignClear(&word);
	    }
	    
	}
    }

    ajStrDel(&rdline);
    
    AJNEW0(k);
    *k = ajListGetLength(listwords);
    ajListPushAppend(listcount, k);
    
    namListParseOK = ajTrue;
    
    namUser("ready to parse\n");
    namListParse(listwords, listcount, file, shortname);
    
    if(!namListParseOK)
	namUser("Unexpected end of file in %S at line %d\n",
		namRootStr, iline);
    
    namUser("file read\n");
    ajListstrFreeData(&listwords); /* Delete the linked list structure */
    namUser("wordlist free\n");
    ajListFree(&listcount);	/* Delete the linked list structure */
    namUser("countlist free\n");
    
    namDebugVariables();
    ajStrDel(&word);
    
    namUser("namProcessFile done '%F'\n", file);

    if(namErrorCount > preverrorcount)
	return ajFalse;

    return ajTrue;
}




/* @func ajNamInit ************************************************************
**
** Initialise the variable and database definitions. Find the definition
** files and read them.
**
** @param [r] prefix [const char*] Default prefix for all file
**                                 and variable names.
** @return [void]
** @@
******************************************************************************/

void ajNamInit(const char* prefix)
{
    const char *prefixRoot;
    AjPFile prefixRootFile;
    AjPStr prefixRootStr = NULL;
    AjPStr prefixStr     = NULL;
    AjPStr prefixCap     = NULL;
    AjPStr debugStr      = NULL;
    AjPStr debugVal      = NULL;
    AjPStr homercVal     = NULL;
    AjPStr basename      = NULL;
    AjBool root_defined;
    AjBool is_windows = ajFalse;
    
    /*
    ** The Windows socket interface must be initialised before any
    ** socket calls are made elsewhere in the AJAX library.
    ** This must only be done once so here is as good a place
    ** as any to do it.
    ** Set the winsock version to 1.1
    **
    ** Also note that EMBOSS_ROOT must be set for ajNamInit to
    ** work with Windows. This will be done by the Windows
    ** installer but developers must set it manually e,g,
    **    set EMBOSS_ROOT C:\emboss\win32
    */
#ifdef WIN32    
    WSADATA wsaData;
#endif

    if(namVarMasterTable && namDbMasterTable && namResMasterTable)
	return;

#ifdef AJ_MPROBE
    /*
    ** mcheck turns on checking of all malloc/calloc/realloc/free calls
    **
    ** it *must* be called before any other malloc by the main program
    **
    ** ajMemCheck reports on status. If called via ajMemProbe (AJMPROBE)
    ** it can also report the source file line it was invoked from
    **
    ** This is all specific to glibc and must be turned on with ./configure
    */
    if(mcheck(ajMemCheck))
	ajWarn("ajNamInit called after first malloc - ajMemCheck ignored");
#endif

#ifdef WIN32
    WSAStartup(MAKEWORD(1, 1), &wsaData);
    is_windows = ajTrue;
#endif


    /*
    ** static namPrefixStr is the prefix for all variable names
    */
    
    ajStrAssignC(&namPrefixStr, prefix);
    
    ajStrAppendC(&namPrefixStr, "_");
    
    ajClockReset();
    ajTimeReset();

    /* create new tables to hold the values */

    namVarMasterTable = ajTablecharNewCase();
    namDbMasterTable = ajTablecharNewCase();
    namResMasterTable = ajTablecharNewCase();
    
    /*
    ** for each type of file read it and save the values 
    ** Start at system level then go to user
    */

    /*
    ** local prefixRoot is the root directory 
    ** it is the value of (PREFIX)_ROOT (if set) or namFixedRoot
    */
    
    ajStrAssignC(&debugStr, prefix);
    
    ajStrAppendC(&debugStr, "_namdebug");
    ajStrFmtUpper(&debugStr);
    
    if(ajNamGetenvS(debugStr, &debugVal))
	ajStrToBool(debugVal, &namDoDebug);
    
    ajStrAssignC(&debugStr, prefix);
    
    ajStrAppendC(&debugStr, "_namvalid");
    ajStrFmtUpper(&debugStr);
    
    if(ajNamGetenvS(debugStr, &debugVal))
	ajStrToBool(debugVal, &namDoValid);
    
    ajStrDel(&debugStr);
    ajStrDel(&debugVal);
    
    ajStrAssignC(&prefixStr, prefix);
    
    ajStrAppendC(&prefixStr, "_ROOT");
    ajStrFmtUpper(&prefixStr);
    
    ajStrAppendC(&prefixCap, prefix);
    ajStrFmtUpper(&prefixCap);

    root_defined = ajNamGetenvS(prefixStr, &prefixRootStr);

    if(!root_defined && is_windows)
	ajDie("EMBOSS_ROOT must be defined for Windows");

    if(root_defined)
    {
	prefixRoot = ajStrGetPtr(prefixRootStr);
#ifdef WIN32
	strcpy(namInstallRoot,prefixRoot);
#endif
    }
    else
	prefixRoot = namFixedRoot;
    
    /* namFixedBaseStr is the directory above the source root */
    
    ajStrAssignC(&namFixedRootStr, prefixRoot);
    ajStrAssignS(&namFixedBaseStr, namFixedRootStr);
    ajDirnameUp(&namFixedBaseStr);
    
    ajStrAssignC(&namFixedPackageStr, namPackage);
    ajStrAssignC(&namFixedVersionStr, namVersion);
    ajStrAssignC(&namFixedInstallStr, namInstallRoot);

    /*
    ** look for default file in the install directory as
       <install-prefix>/share/PREFIX/emboss.default
    **
    ** Note that this will fail with Windows on many levels
    */
    
    ajFmtPrintS(&namRootStr, "%s/share/%S/%s.default",
		 namInstallRoot, prefixCap, prefix);
    prefixRootFile = ajFileNewInNameS(namRootStr);
    ajStrAssignC(&basename, "global");
    
    /* look for $(PREFIX)_ROOT/../emboss.default */
    
    if(!prefixRootFile)
    {
	/* try original directory */
	    ajFmtPrintS(&namRootStr, "%s%s%s.default", prefixRoot,
			SLASH_STRING,prefix);

	prefixRootFile = ajFileNewInNameS(namRootStr);
	ajStrAssignC(&basename, "source");
    }
    
    if(namFileOrig)
	ajStrAppendC(&namFileOrig, ", ");
    ajStrAppendS(&namFileOrig, namRootStr);
    
    if(prefixRootFile)
    {
	ajStrAppendC(&namFileOrig, "(OK)");
	namProcessFile(prefixRootFile, basename);
	ajFileClose(&prefixRootFile);
    }
    else
	ajStrAppendC(&namFileOrig, "(failed)");
    
    
    
    /* look for .embossrc in an arbitrary directory */
    
    prefixRoot= getenv("EMBOSSRC");
    
    if(prefixRoot)
    {
	ajStrAssignC(&namRootStr, prefixRoot);
	ajStrAppendC(&namRootStr, SLASH_STRING);
	ajStrAppendC(&namRootStr, ".");
	ajStrAppendC(&namRootStr, prefix);
	ajStrAppendC(&namRootStr, "rc");

	if(namFileOrig)
	    ajStrAppendC(&namFileOrig, ", ");
	ajStrAppendS(&namFileOrig, namRootStr);

	prefixRootFile = ajFileNewInNameS(namRootStr);

	if(prefixRootFile)
	{
	    ajStrAssignC(&basename, "special");
	    ajStrAppendC(&namFileOrig, "(OK)");
	    namProcessFile(prefixRootFile, basename);
	    ajFileClose(&prefixRootFile);
	}
	else
	    ajStrAppendC(&namFileOrig, "(failed)");
    }
    

    /*
    ** look for $HOME/.embossrc
    **
    ** Note that this will not work with Windows as there is
    ** no concept of HOME
    */
    
    prefixRoot= getenv("HOME");
    
    ajStrAssignC(&prefixStr, prefix);
    
    ajStrAppendC(&prefixStr, "_RCHOME");
    ajStrFmtUpper(&prefixStr);
    
    if(ajNamGetenvS(prefixStr, &homercVal))
	ajStrToBool(homercVal, &namDoHomeRc);

    ajStrDel(&homercVal);
    
    if(namDoHomeRc && prefixRoot)
    {
	ajStrAssignC(&namRootStr, prefixRoot);
	ajStrAppendC(&namRootStr, "/.");
	ajStrAppendC(&namRootStr, prefix);
	ajStrAppendC(&namRootStr, "rc");

	if(namFileOrig)
	    ajStrAppendC(&namFileOrig, ", ");
	ajStrAppendS(&namFileOrig, namRootStr);

	ajStrAssignC(&basename, "user");
	prefixRootFile = ajFileNewInNameS(namRootStr);

	if(prefixRootFile)
	{
	    ajStrAppendC(&namFileOrig, "(OK)");
	    namProcessFile(prefixRootFile, basename);
	    ajFileClose(&prefixRootFile);
	}
	else
	    ajStrAppendC(&namFileOrig, "(failed)");
    }
    
    namUser("Files processed: %S\n", namFileOrig);

    ajStrDel(&prefixRootStr);
    ajStrDel(&basename);
    ajStrDel(&prefixStr);
    ajStrDel(&prefixCap);

    if(!namFixedVersionStr)
        namFixedVersionStr = ajStrNewC(namVersion);
    
    if(namErrorCount)		/* test: badsummary.rc */
	ajDie("Error(s) in configuration files");

    return;
}




/* @funcstatic namNoColon *****************************************************
**
** Remove any trailing colon ':' in the input string.
**
** @param [u] thys [AjPStr*] String.
** @return [void]
** @@
******************************************************************************/

static void namNoColon(AjPStr* thys)
{
    if(ajStrGetCharLast(*thys) == ':')
	ajStrCutEnd(thys, 1);

    return;
}




/* @funcstatic namDbAttr ******************************************************
**
** Return the index for a database attribute name.
**
** @param [r] thys [const AjPStr] Attribute name.
** @return [ajint] Index in namDbAttrs, or -1 on failure.
** @@
******************************************************************************/

static ajint namDbAttr(const AjPStr thys)
{
    return namDbAttrC(ajStrGetPtr(thys));
}




/* @funcstatic namDbAttrC *****************************************************
**
** Return the index for a database attribute name.
**
** @param [r] str [const char*] Attribute name.
** @return [ajint] Index in namDbAttrs, or -1 on failure.
** @@
******************************************************************************/

static ajint namDbAttrC(const char* str)
{
    ajint i = 0;
    ajint j = 0;
    ajint ifound = 0;

    for(i=0; namDbAttrs[i].Name; i++)
    {
	if(!strcmp(str, namDbAttrs[i].Name))
	    return i;

	if(ajCharPrefixC(namDbAttrs[i].Name, str))
	{
	    ifound++;
	    j = i;
	}
    }

    if(ifound == 1)
	return j;

    return -1;
}




/* @funcstatic namRsAttr ******************************************************
**
** Return the index for a resource attribute name.
**
** @param [r] thys [const AjPStr] Attribute name.
** @return [ajint] Index in namRsAttrs, or -1 on failure.
** @@
******************************************************************************/

static ajint namRsAttr(const AjPStr thys)
{
    return namRsAttrC(ajStrGetPtr(thys));
}




/* @funcstatic namRsAttrC *****************************************************
**
** Return the index for a resource attribute name.
**
** @param [r] str [const char*] Attribute name.
** @return [ajint] Index in namRsAttrs, or -1 on failure.
** @@
******************************************************************************/

static ajint namRsAttrC(const char* str)
{
    ajint i = 0;
    ajint j = 0;
    ajint ifound = 0;

    for(i=0; namRsAttrs[i].Name; i++)
    {
	if(!strcmp(str, namRsAttrs[i].Name))
	    return i;

	if(ajCharPrefixC(namRsAttrs[i].Name, str))
	{
	    ifound++;
	    j = i;
	}
    }

    if(ifound == 1)
	return j;

    return -1;
}




/* @func ajNamExit ************************************************************
**
** Delete the initialisation values in the table.
** @return [void]
** @@
******************************************************************************/

void ajNamExit(void)
{

    namListMasterDelete(namVarMasterTable, TYPE_ENV); /* Delete elements */
    ajTableFree(&namVarMasterTable); /* free table and database structures */
    namListMasterDelete(namDbMasterTable, TYPE_DB); /* Delete elements */
    ajTableFree(&namDbMasterTable); /* free table and database structures */
    namListMasterDelete(namResMasterTable, TYPE_RESOURCE);/* Delete elements */
    ajTableFree(&namResMasterTable); /* free table and database structures */

    ajStrDel(&namFixedBaseStr);    /* allocated in ajNamInit */
    ajStrDel(&namFixedRootStr);    /* allocated in ajNamInit */
    ajStrDel(&namFixedInstallStr); /* allocated in ajNamInit */
    ajStrDel(&namFixedPackageStr); /* allocated in ajNamInit */
    ajStrDel(&namFixedVersionStr); /* allocated in ajNamInit */
    ajStrDel(&namPrefixStr);       /* allocated in ajNamInit */
    ajStrDel(&namFileOrig);        /* allocated in ajNamInit */
    ajStrDel(&namRootStr);         /* allocated in ajNamInit */
    ajStrDel(&namFixedVersionStr); /* allocated in ajNamInit */
    
    ajStrDel(&namFileName);		/* allocated in ajNamProcessFile */
    ajStrDel(&namValNameTmp);


    ajRegFree(&namNameExp);
    ajRegFree(&namVarExp);

    ajDebug("ajNamExit done\n");

    return;
}




/* @func ajNamDbTest **********************************************************
**
** Looks for a database name in the known definitions.
**
** @param [r] dbname [const AjPStr] Database name.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajNamDbTest(const AjPStr dbname)
{
    NamPEntry data;

    data = ajTableFetch(namDbMasterTable, ajStrGetPtr(dbname));

    if(!data)
	return ajFalse;

    return ajTrue;
}




/* @func ajNamDbGetUrl ********************************************************
**
** Gets the URL definition for a database definition.
**
** @param [r] dbname [const AjPStr] Database name.
** @param [w] url [AjPStr*] URL returned.
** @return [AjBool] ajTrue if success.
** @@
******************************************************************************/

AjBool ajNamDbGetUrl(const AjPStr dbname, AjPStr* url)
{

    NamPEntry data;
    AjPStr* dbattr;
    static ajint calls = 0;
    static ajint iurl  = 0;

    if(!calls)
    {
	iurl = namDbAttrC("url");
	calls = 1;
    }
    data = ajTableFetch(namDbMasterTable, ajStrGetPtr(dbname));

    if(!data)
	ajFatal("%S is not a known database\n", dbname);

    dbattr = (AjPStr *) data->data;

    if(ajStrGetLen(dbattr[iurl]))
    {
	ajStrAssignS(url, dbattr[iurl]);

	return ajTrue;
    }

    return ajFalse;
}




/* @func ajNamDbGetDbalias ****************************************************
**
** Gets an alias name for a database.
**
** @param [r] dbname [const AjPStr] Database name.
** @param [w] dbalias [AjPStr*] Alias returned.
** @return [AjBool] ajTrue if success.
** @@
******************************************************************************/

AjBool ajNamDbGetDbalias(const AjPStr dbname, AjPStr* dbalias)
{

    NamPEntry data;
    AjPStr* dbattr;
    static ajint calls = 0;
    static ajint idba  = 0;

    if(!calls)
    {
	idba = namDbAttrC("dbalias");
	calls = 1;
    }

    data = ajTableFetch(namDbMasterTable, ajStrGetPtr(dbname));

    if(!data)
	ajFatal("%S is not a known database\n", dbname);

    dbattr = (AjPStr *) data->data;

    if(ajStrGetLen(dbattr[idba]))
    {
	ajStrAssignS(dbalias, dbattr[idba]);

	return ajTrue;
    }

    return ajFalse;
}




/* @func ajNamDbData **********************************************************
**
** Given a query with database name and search fields,
** fill in the common fields. The query fields are set later.
**
** This part of the database definition is required (specifically
** the "fields" definition) for setting the query details.
**
** See also ajNamDbQuery, which calls this function if the common
** query data is not yet set.
**
** @param [u] qry [AjPSeqQuery] Query structure with at least
**                                    dbname filled in
** @return [AjBool] ajTrue if success.
** @@
******************************************************************************/

AjBool ajNamDbData(AjPSeqQuery qry)
{

    NamPEntry data;

    const AjPStr* dbattr;

    data = ajTableFetch(namDbMasterTable, ajStrGetPtr(qry->DbName));

    if(!data)
	ajFatal("database %S unknown\n", qry->DbName);

    dbattr = (const AjPStr *) data->data;

    /* general defaults */

    namDbSetAttrStr(dbattr, "type", &qry->DbType);
    namDbSetAttrStr(dbattr, "method", &qry->Method);
    namDbSetAttrStr(dbattr, "format", &qry->Formatstr);
    namDbSetAttrStr(dbattr, "app", &qry->Application);
    namDbSetAttrStr(dbattr, "directory", &qry->IndexDir);
    namDbSetAttrStr(dbattr, "indexdirectory", &qry->IndexDir);
    namDbSetAttrStr(dbattr, "indexdirectory", &qry->Directory);
    namDbSetAttrStr(dbattr, "directory", &qry->Directory);
    namDbSetAttrStr(dbattr, "exclude", &qry->Exclude);
    namDbSetAttrStr(dbattr, "filename", &qry->Filename);
    namDbSetAttrStr(dbattr, "fields", &qry->DbFields);
    namDbSetAttrStr(dbattr, "proxy", &qry->DbProxy);
    namDbSetAttrStr(dbattr, "httpversion", &qry->DbHttpVer);
    namDbSetAttrBool(dbattr, "caseidmatch", &qry->CaseId);
    namDbSetAttrBool(dbattr, "hasaccession", &qry->HasAcc);
    /*
       ajDebug("ajNamDbQuery DbName '%S'\n", qry->DbName);
       ajDebug("    Id '%S' Acc '%S' Des '%S'\n",
                qry->Id, qry->Acc, qry->Des);
       ajDebug("    Method      '%S'\n", qry->Method);
       ajDebug("    Formatstr   '%S'\n", qry->Formatstr);
       ajDebug("    Application '%S'\n", qry->Application);
       ajDebug("    IndexDir    '%S'\n", qry->IndexDir);
       ajDebug("    Directory   '%S'\n", qry->Directory);
       ajDebug("    Filename    '%S'\n", qry->Filename);
       */

    return ajTrue;
}




/* @func ajNamDbQuery *********************************************************
**
** Given a query with database name and search fields,
** fill in the access method and some common fields according
** to the query level.
**
** @param [u] qry [AjPSeqQuery] Query structure with at least
**                                    dbname filled in
** @return [AjBool] ajTrue if success.
** @@
******************************************************************************/

AjBool ajNamDbQuery(AjPSeqQuery qry)
{

    NamPEntry data;

    const AjPStr* dbattr;

    data = ajTableFetch(namDbMasterTable, ajStrGetPtr(qry->DbName));

    if(!data)
	ajFatal("database %S unknown\n", qry->DbName);

    dbattr = (const AjPStr *) data->data;

    if(!ajStrGetLen(qry->DbType))
	ajNamDbData(qry);

    if(!ajSeqQueryIs(qry))   /* must have a method for all entries */
    {

	namDbSetAttrStr(dbattr, "methodall", &qry->Method);
	namDbSetAttrStr(dbattr, "formatall", &qry->Formatstr);
	namDbSetAttrStr(dbattr, "appall", &qry->Application);
	qry->Type = QRY_ALL;
    }
    else		      /* must be able to query the database */
    {
	namDbSetAttrStr(dbattr, "methodquery", &qry->Method);
	namDbSetAttrStr(dbattr, "formatquery", &qry->Formatstr);
	namDbSetAttrStr(dbattr, "appquery", &qry->Application);

	if(!ajSeqQueryWild(qry)) /* ID - single entry may be available */
	{
	    namDbSetAttrStr(dbattr, "methodentry", &qry->Method);
	    namDbSetAttrStr(dbattr, "formatentry", &qry->Formatstr);
	    namDbSetAttrStr(dbattr, "appentry", &qry->Application);
	    qry->Type = QRY_ENTRY;
	}
	else
	    qry->Type = QRY_QUERY;
    }


    if(!ajStrGetLen(qry->Formatstr))
    {
	ajErr("No format defined for database '%S'", qry->DbName);

	return ajFalse;
    }

    if(!ajStrGetLen(qry->Method))
    {
	ajErr("No access method for database '%S'", qry->DbName);

	return ajFalse;
    }

    return ajTrue;
}




/* @funcstatic namDbSetAttrStr ************************************************
**
** Sets a named string attribute value from an attribute list.
**
** @param [r] dbattr [const AjPStr*] Attribute definitions.
** @param [r] attrib [const char*] Attribute name.
** @param [w] qrystr [AjPStr*] Returned attribute value.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool namDbSetAttrStr(const AjPStr* dbattr, const char* attrib,
			      AjPStr* qrystr)
{
    ajint i;

    i = namDbAttrC(attrib);

    if(i < 0)
	ajFatal("unknown attribute '%s' requested",  attrib);

    if(!ajStrGetLen(dbattr[i]))
	return ajFalse;

    ajStrAssignS(qrystr, dbattr[i]);
    /* ajDebug("namDbSetAttr('%S')\n", *qrystr); */

    namVarResolve(qrystr);

    return ajTrue;
}




/* @funcstatic namDbSetAttrBool ***********************************************
**
** Sets a named boolean attribute value from an attribute list.
**
** @param [r] dbattr [const AjPStr*] Attribute definitions.
** @param [r] attrib [const char*] Attribute name.
** @param [w] qrybool [AjBool*] Returned attribute value.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool namDbSetAttrBool(const AjPStr* dbattr, const char* attrib,
			       AjBool* qrybool)
{
    AjBool ret = ajTrue;
    ajint i;
    AjPStr tmpstr = NULL;

    i = namDbAttrC(attrib);

    if(i < 0)
	ajFatal("unknown attribute '%s' requested",  attrib);

    if(!ajStrGetLen(dbattr[i]))
    {
	ajStrAssignC(&tmpstr, namDbAttrs[i].Defval);
	ret = ajFalse;
    }
    else
    {
	ajStrAssignS(&tmpstr, dbattr[i]);
	ret = ajTrue;
    }

    /* ajDebug("namDbSetAttr('%S')\n", *qrystr); */

    namVarResolve(&tmpstr);
    ajStrToBool(tmpstr, qrybool);
    ajStrDel(&tmpstr);

    return ret;
}




/* @funcstatic namVarResolve **************************************************
**
** Resolves any variable or function references in a string.
** Yet to be implemented, but called in the right places.
**
** @param [u] var [AjPStr*] String value
** @return [AjBool] Always ajTrue so far
** @@
******************************************************************************/

static AjBool namVarResolve(AjPStr* var)
{

    AjPStr varname = NULL;
    AjPStr newvar  = NULL;
    AjPStr restvar = NULL;

    if(!namVarExp)
	namVarExp = ajRegCompC("^\\$([a-zA-Z0-9_.]+)");

    while(ajRegExec(namVarExp, *var))
    {
	ajRegSubI(namVarExp, 1, &varname); /* variable name */

	ajNamGetValueS(varname, &newvar);

	ajDebug("namVarResolve '%S' = '%S'\n", varname, newvar);

	if(ajRegPost(namVarExp, &restvar)) /* any more? */
	    ajStrAppendS(&newvar, restvar);

	ajStrAssignS(var, newvar);
    }

    ajStrDel(&varname);
    ajStrDel(&newvar);
    ajStrDel(&restvar);

    return ajFalse;
}




/* @funcstatic namUser ********************************************************
**
** Formatted write as an error message.
**
** @param [r] fmt [const char*] Format string
** @param [v] [...] Format arguments.
** @return [void]
** @@
******************************************************************************/

static void namUser(const char* fmt, ...)
{
    va_list args;

    if(!namDoDebug)
	return;

    va_start(args, fmt);
    ajFmtVError(fmt, args);
    va_end(args);

    return;
}




/* @funcstatic namError *******************************************************
**
** Formatted write as an error message.
**
** @param [r] fmt [const char*] Format string
** @param [v] [...] Format arguments.
** @return [void]
** @@
******************************************************************************/

static void namError(const char* fmt, ...)
{
    va_list args;
    AjPStr errstr = NULL;
  
    namErrorCount++;

    va_start(args, fmt);
    ajFmtVPrintS(&errstr, fmt, args);
    va_end(args);

    ajErr("File %S line %d: %S", namFileName, namLine, errstr);
    ajStrDel(&errstr);

    return;
}



/* @func ajNamValueInstalldir *************************************************
**
** Returns the install directory root for all file searches
** (package level)
**
** @return [const AjPStr] Install directory root
** @@
******************************************************************************/

const AjPStr ajNamValueInstalldir(void)
{
    return namFixedInstallStr;
}




/* @obsolete ajNamRootInstall
** @remove Use ajNamValueinstalldir
*/
__deprecated AjBool ajNamRootInstall(AjPStr* root)
{
    ajStrAssignS(root, namFixedInstallStr);
    if(!ajStrGetLen(*root)) return ajFalse;
    return ajTrue;
}




/* @func ajNamValuePackage *****************************************************
**
** Returns the package name for the library
**
** @return [const AjPStr] Package name
** @@
******************************************************************************/

const AjPStr ajNamValuePackage(void)
{
    return namFixedPackageStr;
}


/* @obsolete ajNamRootPack
** @remove Use ajNamValuePackage
*/
__deprecated AjBool ajNamRootPack(AjPStr* root)
{
    ajStrAssignS(root, namFixedPackageStr);

    if(!ajStrGetLen(*root))
        return ajFalse;

    return ajTrue;
}




/* @func ajNamValueVersion *****************************************************
**
** Returns the version number for the library
**
** @return [const AjPStr] Version number
** @@
******************************************************************************/

const AjPStr ajNamValueVersion(void)
{
    return namFixedVersionStr;
}




/* @obsolete ajNamRootVersion
** @remove Use ajNamValueVersion
*/
__deprecated AjBool ajNamRootVersion(AjPStr* version)
{
    ajStrAssignS(version, namFixedVersionStr);

    return ajTrue;
}




/* @func ajNamValueRootdir *****************************************************
**
** Returns the directory for all file searches
** (package level)
**
** @return [const AjPStr] Package level root directory
** @@
******************************************************************************/

const AjPStr ajNamValueRootdir(void)
{
    return namFixedRootStr;
}




/* @obsolete ajNamRoot
** @remove Use ajNamValueRootdir
*/
__deprecated AjBool ajNamRoot(AjPStr* root)
{
    ajStrAssignS(root, namFixedRootStr);

    return ajTrue;
}




/* @func ajNamValueBasedir *****************************************************
**
** Returns the base directory for all for all file searches
** (above package level).
**
** @return [const AjPStr] Base directory
******************************************************************************/

const AjPStr ajNamValueBasedir(void)
{
    return namFixedBaseStr;
}




/* @obsolete ajNamRootBase
** @remove Use ajNamValueBasedir
*/
__deprecated AjBool ajNamRootBase(AjPStr* rootbase)
{
    ajStrAssignS(rootbase, namFixedBaseStr);

    return ajTrue;
}




/* @func ajNamResolve *********************************************************
**
** Resolves a variable name if the input string starts with a dollar sign.
**
** @param [w] name [AjPStr*] String
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajNamResolve(AjPStr* name)
{

    AjPStr varname  = NULL;
    AjPStr varvalue = NULL;
    AjPStr restname = NULL;
    AjBool ret;

    if(!namNameExp)
	namNameExp = ajRegCompC("^\\$([A-Za-z0-9_]+)");

    namUser("ajNamResolve of '%S'\n", *name);
    ret = ajRegExec(namNameExp, *name);

    if(ret)
    {
	ajRegSubI(namNameExp, 1, &varname);
	namUser("variable '%S' found\n", varname);
	ajRegPost(namNameExp, &restname);
	ret = ajNamGetValueS(varname, &varvalue);

	if(ret)
	{
	    ajStrAssignS(name, varvalue);
	    ajStrAppendS(name, restname);
	    namUser("converted to '%S'\n", *name);
	}
	else
	{
	    namUser("Variable unknown '$%S'\n", varname);
	    ajWarn("Variable unknown in '%S'", *name);
	}

	ajStrDel(&varname);
	ajStrDel(&varvalue);
	ajStrDel(&restname);
    }

    return ret;
}




/* @funcstatic namValid *******************************************************
**
** Validation of a master table entry
**
** @param [r] entry [const NamPEntry] Internal table entry
** @param [r] entrytype [ajint] Internal table entry type
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool namValid(const NamPEntry entry, ajint entrytype)
{
    if(entrytype == TYPE_ENV)
	return namValidVariable(entry);
    else if(entrytype == TYPE_DB)
	return namValidDatabase(entry);
    else if(entrytype == TYPE_RESOURCE)
	return namValidResource(entry);

    /* fatal: cannot test - should not happen */
    namError("Unknown definition type number %d",
	      entrytype);

    return ajFalse;
}




/* @funcstatic namValidDatabase ***********************************************
**
** Validation of a master table database entry
**
** @param [r] entry [const NamPEntry] Internal table entry
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool namValidDatabase(const NamPEntry entry)
{
    ajint iattr = 0;
    ajint j;
    ajint k;
    AjBool ok;
    AjBool oktype;
    AjPStr* attrs;
    AjBool hasformat = ajFalse;
    AjBool hasmethod = ajFalse;
    AjBool hastype   = ajFalse;
    
    attrs = (AjPStr *) entry->data;

    if(!attrs)
    {			 /* fatal - should be set for all databases */
	namError("Database '%S' has no list of valid attributes",
		  entry->name);
	return ajFalse;
    }
    
    for(j=0; namDbAttrs[j].Name; j++)
    {
	if(attrs[j])
	{
	    iattr++;

	    if(ajCharPrefixC(namDbAttrs[j].Name, "format"))
	    {
		hasformat=ajTrue;

		if(!ajSeqFormatTest(attrs[j]))	/* test: dbunknowns.rc */
		    namError("Database '%S' %s: '%S' unknown\n",
			     entry->name, namDbAttrs[j].Name, attrs[j]);
	    }

	    if(ajCharPrefixC(namDbAttrs[j].Name, "method"))
	    {
		hasmethod=ajTrue;

		if(!ajSeqMethodTest(attrs[j]))	/* test: dbunknowns.rc */
		    namError("Database '%S' %s: '%S' unknown\n",
			     entry->name, namDbAttrs[j].Name, attrs[j]);
	    }

	    if(ajCharPrefixC(namDbAttrs[j].Name, "type"))
	    {
		hastype=ajTrue;
		oktype = ajFalse;

		for(k=0; namDbTypes[k].Name; k++)
		{
		    if(ajStrMatchCaseC(attrs[j], namDbTypes[k].Name)) 
			oktype = ajTrue;
		}

		if(!oktype)		/* test: dbunknowns.rc */
		    namError("Database '%S' %s: '%S' unknown\n",
			      entry->name, namDbAttrs[j].Name, attrs[j]);
	    }
	}
    }

    ok = ajTrue;
    if(!iattr)
    {					/* test: dbempty.rc */
	namError("Database '%S' has no attributes", entry->name);
	ok = ajFalse;
    }

    if(!hasformat)		/* test: dbempty.rc */
    {
	namError("Database '%S' has no format definition", entry->name);
	ok = ajFalse;
    }

    if(!hastype)			/* test: dbempty.rc */
    {
	namError("Database '%S' has no type definition", entry->name);
	ok = ajFalse;
    }

    if(!hasmethod)		/* test: dbempty.rc */
    {
	namError("Database '%S' has no access method definition",
		  entry->name);
	ok = ajFalse;
    }

    return ok;
}




/* @funcstatic namValidResource ***********************************************
**
** Validation of a master table resource entry
**
** @param [r] entry [const NamPEntry] Internal table entry
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool namValidResource(const NamPEntry entry)
{
    ajint iattr = 0;
    ajint j;
    AjPStr* attrs;
    AjBool ok;

    attrs = (AjPStr *) entry->data;

    if(!attrs)
    {			 /* fatal - should be set for all databases */
	namError("Resource '%S' has no list of valid attributes",
		  entry->name);
	return ajFalse;
    }

    for(j=0; namRsAttrs[j].Name; j++)
	if(attrs[j])
	{
	    iattr++;
	}

    ok = ajTrue;

    if(!iattr)
    {					/* test: dbempty.rc */
	namError("Resource '%S' has no attributes", entry->name);
	ok =  ajFalse;
    }

    return ok;
}




/* @funcstatic namValidVariable ***********************************************
**
** Validation of a master table variable entry
**
** @param [r] entry [const NamPEntry] Internal table entry
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool namValidVariable(const NamPEntry entry)
{
    AjPStr* attrs;

    attrs = (AjPStr *) entry->data;

    if(attrs)
    {			 /* strange - should be nothing for variables */
	namError("Variable '%S' has a list of attributes",
		  entry->name);

	return ajFalse;
    }
    
    return ajTrue;
}




/* @func ajNamSetControl ******************************************************
**
** Sets special internal variables to reflect their presence.
**
** Currently these are "namdebug, namvalid"
**
** @param [r] optionName [const char*] option name
** @return [AjBool] ajTrue if option was recognised
** @@
******************************************************************************/

AjBool ajNamSetControl(const char* optionName)
{

    if(!ajCharCmpCase(optionName, "namdebug"))
    {
	namDoDebug = ajTrue;

	return ajTrue;
    }

    if(!ajCharCmpCase(optionName, "namvalid"))
    {
	namDoValid = ajTrue;

	return ajTrue;
    }

    ajDie("Unknown ajNamSetControl control option '%s'", optionName);

    return ajFalse;
}




/* @func ajNamRsAttrValue **************************************************
**
** Return the value for a resource attribute
**
** @param [r] name [const AjPStr] resource name
** @param [r] attribute [const AjPStr] resource attribute
** @param [w] value [AjPStr *] resource value

**
** @return [AjBool] true if found
** @@
******************************************************************************/

AjBool ajNamRsAttrValue(const AjPStr name, const AjPStr attribute,
			 AjPStr *value)
{
    ajint j;
    NamPEntry fnew = NULL;
    AjPStr *rsattr = NULL;

    fnew = ajTableFetch(namResMasterTable, ajStrGetPtr(name));

    rsattr = (AjPStr *) fnew->data;
    j = namRsAttr(attribute);

    if(j < 0)
	ajFatal("unknown attribute '%s' requested for resource '%s'",
                attribute, name);

    if(ajStrGetLen(rsattr[j]))
    {
	ajStrAssignS(value,rsattr[j]);

	return ajTrue;
    }

    return ajFalse;
}




/* @func ajNamRsAttrValueC **************************************************
**
** Return the value for a resource attribute
**
** @param [r] name [const char *] resource name
** @param [r] attribute [const char *] resource attribute
** @param [w] value [AjPStr *] resource value

**
** @return [AjBool] true if found
** @@
******************************************************************************/

AjBool ajNamRsAttrValueC(const char *name, const char *attribute,
			 AjPStr *value)
{
    ajint j;
    NamPEntry fnew = NULL;
    const AjPStr* rsattr;

    fnew = ajTableFetch(namResMasterTable, name);

    if(!fnew)
	return ajFalse;
    
    rsattr = (const AjPStr *) fnew->data;
    j = namRsAttrC(attribute);

    if(j < 0)
	ajFatal("unknown attribute '%s' requested for resource '%s'",
                attribute, name);

    if(ajStrGetLen(rsattr[j]))
    {
	ajStrAssignS(value,rsattr[j]);

	return ajTrue;
    }

    return ajFalse;
}




/* @func ajNamRsListValue **************************************************
**
** Return the value for a resource attribute of type 'list'
**
** @param [r] name [const AjPStr] resource name
** @param [w] value [AjPStr *] resource value
**
** @return [AjBool] true if found
** @@
******************************************************************************/

AjBool ajNamRsListValue(const AjPStr name, AjPStr *value)
{
    ajint j;
    NamPEntry fnew = NULL;
    const AjPStr *rsattr = NULL;

    fnew = ajTableFetch(namResMasterTable, ajStrGetPtr(name));

    rsattr = (const AjPStr *) fnew->data;
    j = namRsAttrC("type");

    if(!ajStrMatchCaseC(rsattr[j], "list"))
	return ajFalse;

    j = namRsAttrC("value");

    if(ajStrGetLen(rsattr[j]))
    {
	ajStrAssignS(value,rsattr[j]);

	return ajTrue;
    }

    return ajFalse;
}

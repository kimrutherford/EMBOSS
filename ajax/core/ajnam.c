/* @source ajnam **************************************************************
**
** AJAX nam functions
**
** Creates a hash table of initial values and allow access to this
** via the routines ajNamDatabase and ajNamGetValueS.
**
** @author Copyright (C) 1998 Ian Longden
** @version $Revision: 1.184 $
** @modified 2000-2011 Peter Rice
** @modified $Date: 2012/07/15 18:36:14 $ by $Author: rice $
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

#include "ajnam.h"
#include "ajsys.h"
#include "ajfileio.h"
#include "ajquery.h"
#include "ajreg.h"
#include "ajtagval.h"
#include "ajassemread.h"
#include "ajfeatread.h"
#include "ajoboread.h"
#include "ajrefseqread.h"
#include "ajresourceread.h"
#include "ajseqread.h"
#include "ajtaxread.h"
#include "ajtextread.h"
#include "ajurlread.h"
#include "ajvarread.h"

#ifndef WIN32
#include <dirent.h>
#include <unistd.h>
#else /* !WIN32 */
#include "win32.h"
#include <winsock2.h>
#include <stdlib.h>
#endif /* WIN32 */

#ifdef HAVE_MCHECK
#include <mcheck.h>
#endif /* ! HAVE_MCHECK */


enum NamEType
{
    TYPE_UNKNOWN,			/* no type set */
    TYPE_ENV,				/* env or set variable */
    TYPE_SVR,				/* server definition */
    TYPE_DB,				/* database definition */
    TYPE_RESOURCE,			/* resource definition */
    TYPE_IFILE,				/* include filename */
    TYPE_ALIAS,				/* database alias */
    TYPE_SVRDB,				/* server:database table */
    TYPE_SVRALI,                        /* server:alias table */
    TYPE_IF,
    TYPE_IFDEF,
    TYPE_ELSE,
    TYPE_ENDIF,
    TYPE_MAX
};

enum NamEAttrtype
{
    ATTR_UNKNOWN,			/* no type set */
    ATTR_STR,				/* string */
    ATTR_BOOL,				/* boolean */
    ATTR_LIST,				/* list of strings */
};

const char* namTypes[] = { "unknown", "SET",
                           "SERVER", "DBNAME", "RESOURCE",
                           "INCLUDE", "ALIAS",
                           "SERVER:DB", "SERVER:ALIAS",
                           "IF", "ELSE", "ENDIF",
                           "MAX"
};

#define NAM_INCLUDE_ESTIMATE 5	/* estimate of maximum number of include */
                                /* statements in emboss.default          */

/* Scope values for entry methods returned from name*Method2Scope */
#define SLOW_ENTRY 8
#define SLOW_QUERY 16

static AjBool namDoDebug  = AJFALSE;
static AjBool namDoValid  = AJFALSE;
static AjBool namDoHomeRc = AJTRUE;
static AjPStr namRootStr  = NULL;
static AjPStr namStandardFilename = NULL;
static AjPStr namStandardDir = NULL;
static AjPStr namUserDir = NULL;
static AjPStr namUserFilename = NULL;
static AjPStr namValNameTmp  = NULL;

static AjBool namListParseOK = AJFALSE;

static ajuint namIfBlock = 0;
static AjBool namIfNow = AJTRUE;

static AjPList namIfList = NULL;
static AjBool *namIfValue;
static AjBool namTrue = AJTRUE;
static AjBool namFalse = AJFALSE;
    
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
static char namVersion[] = "6.x.x";
#endif

#ifdef AJAX_SYSTEM
static char namSystem[] = AJAX_SYSTEM;
#else
#ifdef WIN32
static char namSystem[] = "windows";
#else
static char namSystem[] = "unknown";
#endif
#endif

/* string versions of char* constants set in ajNamInit */
static AjPStr namFixedBaseStr     = NULL;
static AjPStr namFixedRootStr     = NULL;
static AjPStr namFixedInstallStr  = NULL;
static AjPStr namFixedPackageStr  = NULL;
static AjPStr namFixedSystemStr  = NULL;
static AjPStr namFixedVersionStr  = NULL;
static AjPStr namPrefixStr        = NULL;
static AjPStr namFileOrig         = NULL;

static AjPTable namDbTypeTable = NULL;
static AjPTable namAliasMasterTable = NULL;
static AjPTable namVarMasterTable = NULL;
static AjPTable namSvrMasterTable = NULL;
static AjPTable namDbMasterTable = NULL;
static AjPTable namResMasterTable = NULL;
static AjPTable namSvrAttrTable = NULL;
static AjPTable namDbAttrTable = NULL;
static AjPTable namResAttrTable = NULL;
static AjPTable namSvrDatabaseTable = NULL;
static AjPTable namSvrAliasTable = NULL;
static ajint namParseType      = 0;
static AjPStr namFileName      = NULL;
static ajint namLine           = 0;
static ajint namErrorCount     = 0;

static AjPRegexp namNameExp = NULL;
static AjPRegexp namVarExp  = NULL;


static AjPStr namCmpStr = NULL;

/* @datastatic NamPAttr *******************************************************
**
** Resource attribute definition structure
**
** @alias NamSAttr
** @alias NamOAttr
**
** @attr Name [const char*] Attribute name
** @attr Type [enum NamEAttrtype] Type of value
** @attr Defval [const char*] Default value, usually an empty string
** @attr Comment [const char*] Comment for documentation purposes
** @@
******************************************************************************/

typedef struct NamSAttr
{
    const char* Name;
    enum NamEAttrtype Type;
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




/* @datastatic NamPType ******************************************************
**
** Resource attribute validation structure
**
** @alias NamSType
** @alias NamOType
**
** @attr Name [const char*] Type name
** @attr Scope [const char*] Type scope
** @attr Comment [const char*] Comment for documentation purposes
** @attr DataType [ajuint] Enumerated datatype matching scope
** @@
******************************************************************************/

typedef struct NamSType
{
    const char* Name;
    const char* Scope;
    const char* Comment;
    ajuint DataType;
} NamOType;

#define NamPType NamOType*




NamOAttr namSvrAttrs[] =
{
    {"method", ATTR_STR, "",
     "access method (required, at some level)"},
    {"type", ATTR_STR, "",
     "database type(s) 'Nucleotide', 'Protein', etc (required)"},
    {"cachedirectory", ATTR_STR, "",
     "cache directory name"},
    {"cachefile", ATTR_STR, "", "cache file name"},

    {"accession", ATTR_STR, "",
     "secondary identifier field"},
    {"app", ATTR_STR, "",
     "external application commandline (APP, EXTERNAL)"},
    {"appall", ATTR_STR, "",
     "external commandline for 'methodall' (APP, EXTERNAL)"},
    {"appentry", ATTR_STR, "",
     "external commandline for 'methodentry' (APP, EXTERNAL)"},
    {"appquery", ATTR_STR, "",
     "external commandline for 'methodquery' (APP, EXTERNAL)"},

    {"caseidmatch", ATTR_BOOL, "N",
     "match exact case of entry identifier"},
    {"comment", ATTR_STR, "",
     "text comment for the server definition"},
    {"directory", ATTR_STR, "",
     "data directory"},
/*    {"exclude", ATTR_STR, "",
      "wildcard filenames to exclude from 'filename'"},*/
    {"field", ATTR_LIST, "",
     "database query field, with altnames and description"},
    {"filters", ATTR_STR, "",
     "database query filters to apply to all retrievals"},
/*    {"filename", "",
      "(wildcard) database filename"},*/

    {"format", ATTR_STR, "",
     "database entry format(s)"},
    {"formatall", ATTR_STR, "",
     "database entry format(s) for 'methodall' access"},
    {"formatentry", ATTR_STR, "",
     "database entry format(s) for 'methodentry' access"},
    {"formatquery", ATTR_STR, "",
     "database query format(s) for 'methodquery' access"},

    {"hasaccession", ATTR_BOOL, "Y",
     "database has an acc field as an alternate id"},
    {"httpversion", ATTR_STR, "",
     "HTTP version for GET requests"},
    {"identifier", ATTR_STR, "",
     "standard identifier field"},
    {"indexdirectory", ATTR_STR, "",
     "Index directory, defaults to data 'directory'"},

    {"methodall", ATTR_STR, "",
     "access method for all entries"},
    {"methodentry", ATTR_STR, "",
     "access method for single entry"},
    {"methodquery", ATTR_STR, "",
     "access method for query (several entries)"},

    {"proxy", ATTR_STR, "",
     "http proxy server, or ':' to cancel a global proxy "},
    {"query", ATTR_STR, "",
     "database query (SQL, SPARQL, etc.)"},
/*    {"release", ATTR_STR, "",
      "release of the database, comment only"},*/
    {"return", ATTR_STR, "",
     "names fields to be returned"},
    {"sequence", ATTR_STR, "",
     "sequence field to be returned"},
    {"serverversion", ATTR_STR, "",
     "Version of database server"},
    {"special", ATTR_LIST, "",
     "name:value attributes for access method"},
    {"url", ATTR_STR, "",
     "Basic URL for data access"},
    {"view", ATTR_STR, "",
     "Access method view"},

    {"edamdat", ATTR_LIST, "",
     "EDAM datatype term references"},
    {"edamfmt", ATTR_LIST, "",
     "EDAM format term references"},
    {"edamid", ATTR_LIST, "",
     "EDAM identifier term references"},
    {"edamtpc", ATTR_LIST, "",
     "EDAM topic term references"},
    {"taxon", ATTR_LIST, "",
     "taxon id and name"},

    {NULL, ATTR_UNKNOWN, NULL,
     NULL}
};

NamOAttr namDbAttrs[] =
{
    {"format", ATTR_STR, "",
     "database entry format(s) (required, at some level)"},
    {"method", ATTR_STR, "",
     "access method (required, at some level)"},
    {"type", ATTR_STR, "",
     "database type(s) 'Nucleotide', 'Protein', etc (required)"},

    {"accession", ATTR_STR, "",
     "secondary identifier field"},
    {"app", ATTR_STR, "",
     "external application commandline (APP, EXTERNAL)"},
    {"appall", ATTR_STR, "",
     "external commandline for 'methodall' (APP, EXTERNAL)"},
    {"appentry", ATTR_STR, "",
     "external commandline for 'methodentry' (APP, EXTERNAL)"},
    {"appquery", ATTR_STR, "",
     "external commandline for 'methodquery' (APP, EXTERNAL)"},

    {"caseidmatch", ATTR_BOOL, "N",
     "match exact case of entry identifier"},
    {"comment", ATTR_STR, "",
     "text comment for the DB definition"},
    {"dbalias", ATTR_STR, "",
     "database name(s) to be used by access method if different"},
    {"directory", ATTR_STR, "",
     "data directory"},
    {"example", ATTR_STR, "",
     "example identifier"},
    {"exclude", ATTR_STR, "",
     "wildcard filenames to exclude from 'filename'"},
    {"field", ATTR_LIST, "",
     "database query field, with altnames and description"},
    {"fields", ATTR_STR, "",
     "extra database query fields, ID and ACC are standard"},
    {"filters", ATTR_STR, "",
     "database query filters to apply to all retrievals"},
    {"filename", ATTR_STR, "",
     "(wildcard) database filename"},

    {"formatall", ATTR_STR, "",
     "database entry format(s) for 'methodall' access"},
    {"formatentry", ATTR_STR, "",
     "database entry format(s) for 'methodentry' access"},
    {"formatquery", ATTR_STR, "",
     "database query format(s) for 'methodquery' access"},

    {"hasaccession", ATTR_BOOL, "Y",
     "database has an acc field as an alternate id"},
    {"httpversion", ATTR_STR, "", "HTTP version for GET requests"},
    {"identifier", ATTR_STR, "", "standard identifier field"},
    {"indexdirectory", ATTR_STR, "",
     "Index directory, defaults to data 'directory'"},

    {"methodall", ATTR_STR, "",
     "access method for all entries"},
    {"methodentry", ATTR_STR, "",
     "access method for single entry"},
    {"methodquery", ATTR_STR, "",
     "access method for query (several entries)"},

    {"namespace", ATTR_STR, "",
     "namespace query to restrict ontology searches"},
    {"organisms", ATTR_STR, "",
     "organism/taxonomy query to restrict biological searches"},
    {"proxy", ATTR_STR, "",
     "http proxy server, or ':' to cancel a global proxy"},
    {"query", ATTR_STR, "",
     "database query (SQL, SPARQL, etc.)"},
    {"release", ATTR_STR, "",
     "release of the database, comment only"},
    {"return", ATTR_STR, "",
     "names of fields to be returned"},
    {"sequence", ATTR_STR, "",
     "sequence field to be returned"},
    {"serverversion", ATTR_STR, "",
     "version of database server"},
    {"special", ATTR_LIST, "",
     "name:value attributes for access method"},
    {"url", ATTR_STR, "",
     "Basic URL for data access"},
    {"view", ATTR_STR, "",
     "Access method view"},

    {"edamdat", ATTR_LIST, "",
     "EDAM datatype term references"},
    {"edamfmt", ATTR_LIST, "",
     "EDAM format term references"},
    {"edamid", ATTR_LIST, "",
     "EDAM identifier term references"},
    {"edamtpc", ATTR_LIST, "",
     "EDAM topic term references"},
    {"taxon", ATTR_LIST, "",
     "taxon id and name"},

    {NULL, ATTR_UNKNOWN, NULL,
     NULL}
};

NamOAttr namRsAttrs[] =
{
    {"type", ATTR_STR, "",
     "resource type (required)"},

/*    {"identifier",ATTR_STR,  "",
      "standard identifier (defaults to name)"},*/
    {"release", ATTR_STR, "",
     "release of the resource"},
    {"pagesize", ATTR_STR, "",
     "default index pagesize"},
    {"secpagesize", ATTR_STR, "",
     "default index secondary pagesize"},
    {"cachesize", ATTR_STR, "",
     "default index cache size"},
    {"seccachesize", ATTR_STR, "",
     "default index secondary cache size"},

    {"fields", ATTR_STR,
     "id,acc,sv,key,des,org,nam,isa,xref",
                    "known database query fields"},

    {"value", ATTR_STR, "",
     "value appropriate to the resource type"},
    {NULL, ATTR_UNKNOWN, NULL,
     NULL}
};

NamOType namDbTypes[] =
{
    {"N",            "sequence",
     "Nucleotide (obsolete short name)",
     AJDATATYPE_SEQUENCE},
    {"P",            "sequence",
     "Protein (obsolete short name)",
     AJDATATYPE_SEQUENCE},
    {"Nucleotide",   "sequence",
     "Nucleotide sequence data",
     AJDATATYPE_SEQUENCE},
    {"Protein",      "sequence",
     "Protein sequence data",
     AJDATATYPE_SEQUENCE},
    {"Sequence",     "sequence",
     "Both nucleotide and protein sequence data",
     AJDATATYPE_SEQUENCE},
    {"Nucfeatures",  "features",
     "Nucleotide features data",
     AJDATATYPE_FEATURES},
    {"Protfeatures", "features",
     "Protein features data",
     AJDATATYPE_FEATURES},
    {"Features",     "features",
     "Both nucleotide and protein features data",
     AJDATATYPE_FEATURES},
/*    {"Pattern",      "pattern",
      "Pattern data"},*/
    {"Assembly",     "assembly",
     "Assembly of sequence reads",
     AJDATATYPE_ASSEMBLY},
    {"Obo",          "obo",
     "OBO ontology",
     AJDATATYPE_OBO},
    {"Refseq",       "refseq",
     "Reference sequence",
     AJDATATYPE_REFSEQ},
    {"Resource" ,    "resource",
     "Data resource data",
     AJDATATYPE_RESOURCE},
    {"Taxonomy",     "taxon",
     "NCBI taxonomy",
     AJDATATYPE_TAXON},
    {"Html",         "text",
     "HTML data",
     AJDATATYPE_TEXT},
    {"Xml",          "text",
     "XML data",
     AJDATATYPE_TEXT},
    {"Text",         "text",
     "Text data",
     AJDATATYPE_TEXT},
    {"Url",          "url",
     "Url",
     AJDATATYPE_URL},
    {"Variation",    "variation",
     "Variation",
     AJDATATYPE_URL},
    {"Unknown",      "text",
     "Unknown type",
     AJDATATYPE_UNKNOWN},
    {NULL, NULL,
     NULL,
     AJDATATYPE_UNKNOWN}
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




static const AjPTable namQueryDbdata(const AjPQuery qry);
static const AjPTable namQuerySvrdata(const AjPQuery qry);
static AjBool namSvrCacheParseList(AjPList listwords,
                                   AjPList listcount,  AjPFile file,
                                   AjPTable dbtable, AjPTable alitable);
static AjBool namSvrCacheParse(AjPFile cachefile, AjPTable dbtable,
                               AjPTable alitable);
static AjPFile namSvrCacheOpen(const AjPStr cachename);
static AjBool namSvrCacheRead(const AjPStr server, const AjPStr cachename);
static ajint  namSvrAttrC(const char* str);
static ajint  namSvrAttrS(const AjPStr thys);
static AjBool namSvrSetAttrBoolC(const AjPTable dbtable, const char* attrib,
                                AjBool* qrybool);
static AjBool namSvrSetAttrStrC(const AjPTable dbtable, const char* attrib,
                               AjPStr* qrystr);

static const char* namDatatype2Fields(ajint datatype);
static const char* namDatatype2Qlinks(ajint datatype);
static ajint  namDbAttrC(const char* str);
static ajint  namDbAttrS(const AjPStr thys);
static AjBool namDbSetAttrBoolC(const AjPTable dbtable, const char* attrib,
                                AjBool* qrybool);
static AjBool namDbSetAttrStrC(const AjPTable dbtable, const char* attrib,
                               AjPStr* qrystr);

static void   namDebugServer(const AjPTable dbtable);
static void   namDebugDatabase(const AjPTable dbtable);
static void   namDebugResource(const AjPTable dbtable);
static void   namDebugVariables(void);
static void   namDebugAliases(void);
static void   namDebugMaster(const AjPTable table, ajint which);

static void   namEntryDelete(NamPEntry* pentry, ajint which);
static void   namError(const char* fmt, ...);
static void   namListParse(AjPList listwords, AjPList listcount,
			   AjPFile file, const AjPStr shortname);
static void   namListMasterDelete(AjPTable table, ajint which);
static AjBool namAccessTest(const AjPStr method, const AjPStr dbtype);
static AjBool namInformatTest(const AjPStr format, const AjPStr dbtype);
static const char* namMethod2Qlinks(const AjPStr method, ajint datatype);
static ajuint namMethod2Scope(const AjPStr method, const AjPStr dbtype);
static void   namNoColon(AjPStr *thys);
static AjBool namProcessFile(AjPFile file, const AjPStr shortname);
static ajint  namRsAttrC(const char* str);
static ajint  namRsAttrS(const AjPStr thys);
static ajint  namRsAttrFieldC(const AjPTable rstable, const char* str);
static ajint  namRsAttrFieldS(const AjPTable rstable, const AjPStr str);
static void   namUser(const char *fmt, ...);
static AjBool namValid(const NamPEntry entry, ajint entrytype);
static AjBool namValidAlias(const NamPEntry entry);
static AjBool namValidDatabase(const NamPEntry entry);
static AjBool namValidResource(const NamPEntry entry);
static AjBool namValidServer(const NamPEntry entry);
static AjBool namValidVariable(const NamPEntry entry);
static AjBool namVarResolve(AjPStr* var);
static AjBool namDbtablePutAttrS(AjPTable dbtable, AjPStr *Pattribute,
                                 AjPStr *Pvalue);
static AjBool namRstablePutAttrS(AjPTable rstable, AjPStr *Pattribute,
                                 AjPStr *Pvalue);
static AjBool namSvrtablePutAttrS(AjPTable svrtable, AjPStr *Pattribute,
                                  AjPStr *Pvalue);
static void namDbAttrtableFree(AjPTable* Ptable);
static void namRsAttrtableFree(AjPTable* Ptable);
static void namSvrAttrtableFree(AjPTable* Ptable);




/* @funcstatic namEntryDelete *************************************************
**
** Deletes a variable, database, or resource entry from the internal table.
**
** @param [d] Pentry [NamPEntry*] The entry to be deleted.
** @param [r] which [ajint] Internal table entry type
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void namEntryDelete(NamPEntry* Pentry, ajint which)
{

    AjPTable attrtable;
    NamPEntry entry;

    entry = *Pentry;

    ajStrDel(&entry->name);
    ajStrDel(&entry->value);
    ajStrDel(&entry->file);

    if(which == TYPE_DB)
    {
	attrtable = (AjPTable) entry->data;
        namDbAttrtableFree(&attrtable);
    }

    else if(which == TYPE_SVR)
    {
	attrtable = (AjPTable) entry->data;
        namSvrAttrtableFree(&attrtable);
    }

    else if(which == TYPE_RESOURCE)
    {
	attrtable = (AjPTable) entry->data;
        namRsAttrtableFree(&attrtable);
    }

    else if(which == TYPE_SVRDB)
    {
	attrtable = (AjPTable) entry->data;
        namListMasterDelete(attrtable, TYPE_DB);
        ajTableFree(&attrtable);
    }

    else if(which == TYPE_SVRALI)
    {
	attrtable = (AjPTable) entry->data;
        namListMasterDelete(attrtable, TYPE_ENV);
        ajTableFree(&attrtable);
    }

    else if(which == TYPE_ENV)
    {
    }

    else if(which == TYPE_ALIAS)
    {
    }

    AJFREE(entry);

    return;
}




/* @funcstatic namDbAttrtableFree *********************************************
**
** Free a database attribute table
**
** @param [d] Ptable [AjPTable*] Attribute table
**
**
** @release 6.4.0
******************************************************************************/

static void namDbAttrtableFree(AjPTable* Ptable)
{
    AjPStr *keys = NULL;
    void** values = NULL;
    ajulong n;
    ajulong i;
    const NamPAttr attr = NULL;

    n = ajTableToarrayKeysValues(*Ptable, (void***)&keys, &values);

    for(i=0UL; i < n; i++)
    {
        attr = ajTableFetchS(namDbAttrTable, keys[i]);

        if(!attr)
            ajStrDel((AjPStr*) &values[i]);
        else
        {
            switch(attr->Type)
            {
                case ATTR_LIST:
                    ajListstrFreeData((AjPList*) &values[i]);
                    break;
                default:
                    ajStrDel((AjPStr*) &values[i]);
                    break;
            }
        }

        ajStrDel(&keys[i]);
    }
    
    ajTableFree(Ptable);

    AJFREE(keys);
    AJFREE(values);

    return;
}




/* @funcstatic namRsAttrtableFree *********************************************
**
** Free a resource attribute table
**
** @param [d] Ptable [AjPTable*] Attribute table
**
**
** @release 6.4.0
******************************************************************************/

static void namRsAttrtableFree(AjPTable* Ptable)
{
    AjPStr *keys = NULL;
    void** values = NULL;
    ajulong n;
    ajulong i;
    const NamPAttr attr = NULL;

    n = ajTableToarrayKeysValues(*Ptable, (void***) &keys, &values);

    for(i=0UL; i < n; i++)
    {
        attr = ajTableFetchS(namResAttrTable, keys[i]);

        if(!attr)
            ajStrDel((AjPStr*) &values[i]);
        else
        {
            switch(attr->Type)
            {
                case ATTR_LIST:
                    ajListstrFreeData((AjPList*) &values[i]);
                    break;
                default:
                    ajStrDel((AjPStr*) &values[i]);
                    break;
            }
        }

        ajStrDel(&keys[i]);
    }

    ajTableFree(Ptable);

    AJFREE(keys);
    AJFREE(values);

    return;
}




/* @funcstatic namSvrAttrtableFree ********************************************
**
** Free a server attribute table
**
** @param [d] Ptable [AjPTable*] Attribute table
**
**
** @release 6.4.0
******************************************************************************/

static void namSvrAttrtableFree(AjPTable* Ptable)
{
    AjPStr *keys = NULL;
    void** values = NULL;
    ajulong n;
    ajulong i;
    const NamPAttr attr = NULL;

    n = ajTableToarrayKeysValues(*Ptable, (void***) &keys, &values);

    for(i=0UL; i < n; i++)
    {
        attr = ajTableFetchS(namSvrAttrTable, keys[i]);

        if(!attr)
            ajStrDel((AjPStr*) &values[i]);
        else
        {
            switch(attr->Type)
            {
                case ATTR_LIST:
                    ajListstrFreeData((AjPList*) &values[i]);
                    break;
                default:
                    ajStrDel((AjPStr*) &values[i]);
                    break;
            }
        }

        ajStrDel(&keys[i]);
    }
    
    ajTableFree(Ptable);

    AJFREE(keys);
    AJFREE(values);

    return;
}




/* @funcstatic namListMasterDelete ********************************************
**
** Deletes all databases in the internal table. The table is converted to
** an array, and each entry in turn is passed to namEntryDelete.
**
** @param [u] table [AjPTable] Table object
** @param [r] which [ajint] Internal table entry type
** @return [void]
**
** @release 2.7.0
** @@
******************************************************************************/

static void namListMasterDelete(AjPTable table, ajint which)
{
    ajint i;
    NamPEntry fnew = NULL;
    void **keyarray = NULL;
    void **valarray = NULL;

    if(!table)
        return;

    ajTableToarrayKeysValues(table, &keyarray, &valarray);

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




/* @func ajNamPrintSvrAttr ****************************************************
**
** Prints a report of the server attributes available (for entrails)
**
** @param [u] outf [AjPFile] Output file
** @param [r] full [AjBool] Full output if AjTrue
** @return [void]
**
** @release 6.4.0
******************************************************************************/

void ajNamPrintSvrAttr(AjPFile outf, AjBool full)
{
    ajint i;
    AjPStr tmpstr = NULL;
    ajuint maxtmp = 0;

    (void) full;			/* no extra detail to report */

    ajFmtPrintF(outf, "# Server attributes\n");
    ajFmtPrintF(outf, "# %-15s %-12s %s\n", "Attribute", "Default", "Comment");
    ajFmtPrintF(outf, "namSvrAttrs {\n");

    for(i=0; namSvrAttrs[i].Name; i++)
    {
	ajFmtPrintF(outf, "  %-15s", namSvrAttrs[i].Name);
	ajFmtPrintS(&tmpstr, "\"%s\"", namSvrAttrs[i].Defval);

	if(ajStrGetLen(tmpstr) > maxtmp)
	    maxtmp = ajStrGetLen(tmpstr);

	ajFmtPrintF(outf, " %-12S", tmpstr);
	ajFmtPrintF(outf, " \"%s\"\n", namSvrAttrs[i].Comment);
    }

    if(maxtmp > 12)
        ajWarn("ajNamPrintSvrAttr max tmpstr len %u",
               maxtmp);	      

    ajFmtPrintF(outf, "}\n\n");
    ajStrDel(&tmpstr);

    return;
}




/* @func ajNamPrintDbAttr *****************************************************
**
** Prints a report of the database attributes available (for entrails)
**
** @param [u] outf [AjPFile] Output file
** @param [r] full [AjBool] Full output if AjTrue
** @return [void]
**
** @release 2.5.0
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
        ajWarn("ajNamPrintDbAttr max tmpstr len %u",
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
**
** @release 2.7.0
******************************************************************************/

void ajNamPrintRsAttr(AjPFile outf, AjBool full)
{
    ajint i;
    AjPStr tmpstr = NULL;
    ajuint maxtmp = 0;

    (void) full;			/* no extra detail to report */

    ajFmtPrintF(outf, "# Resource attributes\n");
    ajFmtPrintF(outf, "# %-15s %-26s %s\n", "Attribute", "Default", "Comment");
    ajFmtPrintF(outf, "namRsAttrs {\n");

    for(i=0; namRsAttrs[i].Name; i++)
    {
	ajFmtPrintF(outf, "  %-15s", namRsAttrs[i].Name);
	ajFmtPrintS(&tmpstr, "\"%s\"", namRsAttrs[i].Defval);

	if(ajStrGetLen(tmpstr) > maxtmp)
	    maxtmp = ajStrGetLen(tmpstr);

	ajFmtPrintF(outf, " %-36S", tmpstr);
	ajFmtPrintF(outf, " \"%s\"\n", namRsAttrs[i].Comment);
    }

    ajFmtPrintF(outf, "}\n\n");

    if(maxtmp > 36)
        ajWarn("ajNamPrintRsAttr max tmpstr len %u",
               maxtmp);	      

    ajStrDel(&tmpstr);

    return;
}




/* @funcstatic namDebugDatabase ***********************************************
**
** Prints a report of defined attributes for a database definition.
**
** @param [r] dbtable [const AjPTable] Attribute table from a database entry.
** @return [void]
**
** @release 1.13.0
** @@
******************************************************************************/

static void namDebugDatabase(const AjPTable dbtable)
{
    ajulong i;
    ajulong nkeys;
    AjPStr *names = NULL;
    AjPStr *values = NULL;

    nkeys = ajTableToarrayKeysValues(dbtable,
                                     (void***) &names, (void***) &values);
    
   for(i=0UL; i < nkeys; i++) 
	if(ajStrGetLen(values[i]))
	    namUser("\t%S: %S\n", names[i], values[i]);

    AJFREE(names);
    AJFREE(values);

    return;
}




/* @funcstatic namDebugServer *************************************************
**
** Prints a report of defined attributes for a server definition.
**
** @param [r] svrtable [const AjPTable] Attribute table from a server entry.
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void namDebugServer(const AjPTable svrtable)
{
    ajulong i;
    ajulong nkeys;
    AjPStr *names = NULL;
    AjPStr *values = NULL;

    nkeys = ajTableToarrayKeysValues(svrtable,
                                     (void***) &names, (void***) &values);
    
   for(i=0UL; i < nkeys; i++) 
	if(ajStrGetLen(values[i]))
	    namUser("\t%S: %S\n", names[i], values[i]);

    AJFREE(names);
    AJFREE(values);

    return;
}




/* @funcstatic namDebugResource ***********************************************
**
** Prints a report of defined attributes for a resource definition.
**
** @param [r] rstable [const AjPTable] Attribute table from a database entry.
** @return [void]
**
** @release 2.7.0
** @@
******************************************************************************/

static void namDebugResource(const AjPTable rstable)
{
    ajulong i;
    ajulong nkeys;
    AjPStr *names = NULL;
    AjPStr *values = NULL;

    nkeys = ajTableToarrayKeysValues(rstable,
                                     (void***) &names, (void***) &values);
    
   for(i=0UL; i < nkeys; i++) 
	if(ajStrGetLen(values[i]))
	    namUser("\t%S: %S\n", names[i], values[i]);

    AJFREE(names);
    AJFREE(values);

    return;
}




/* @funcstatic namDebugMaster *************************************************
**
** Lists databases or variables defined in the internal table.
**
** @param [r] table [const AjPTable] Table object
** @param [r] which [ajint] Variable type, either TYPE_ENV for environment
**                        variables or TYPE_DB for databases or
**                        TYPE_RESOURCE for resources.
** @return [void]
**
** @release 2.7.0
** @@
******************************************************************************/

static void namDebugMaster(const AjPTable table, ajint which)
{
    ajint i;
    NamPEntry fnew;
    void **keyarray = NULL;
    void **valarray = NULL;
    char *key;

    ajTableToarrayKeysValues(table, &keyarray, &valarray);

    for(i = 0; keyarray[i]; i++)
    {
	key  = (char*) keyarray[i];
	fnew = (NamPEntry) valarray[i];

	if(TYPE_SVR == which)
	{
	    namUser("SVR %S\t *%s*\n", fnew->name, key);
	    namDebugServer(fnew->data);
	    namUser("\n");
	}

	else if(TYPE_DB == which)
	{
	    namUser("DB %S\t *%s*\n", fnew->name, key);
	    namDebugDatabase(fnew->data);
	    namUser("\n");
	}

	else if(TYPE_RESOURCE == which)
	{
	    namUser("RES %S\t *%s*\n", fnew->name, key);
	    namDebugResource(fnew->data);
	    namUser("\n");
	}

	else if(TYPE_ENV == which)
	    namUser("ENV %S\t%S\t *%s*\n",fnew->name,fnew->value,key);

	else if(TYPE_ALIAS == which)
	    namUser("ALIAS %S\t%S\t *%s*\n",fnew->name,fnew->value,key);
    }

    AJFREE(keyarray);
    AJFREE(valarray);

    return;
}




/* @func ajNamSvrGetAttrC *****************************************************
**
** Return the value for a server attribute
**
** @param [r] name [const AjPStr] server name
** @param [r] attribute [const char *] server attribute name
** @param [w] value [AjPStr *] attribute value
**
** @return [AjBool] true if found
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajNamSvrGetAttrC(const AjPStr name, const char *attribute,
                        AjPStr *value)
{
    ajint j;
    NamPEntry fnew = NULL;
    AjPTable svrtable;
    const AjPStr svrvalue;

    ajDebug("ajNamSvrGetAttrC '%S' '%s'\n", name, attribute);

    fnew = ajTableFetchmodS(namSvrMasterTable, name);

    if(!fnew)
    {
        ajWarn("unknown server '%S'",
               name);
	return ajFalse;
    }

    svrtable = (AjPTable) fnew->data;
    j = namSvrAttrC(attribute);

    if(j < 0)
    {
	ajWarn("unknown attribute '%s' requested for server '%S'",
               attribute, name);
        return ajFalse;
    }

    svrvalue = ajTableFetchmodC(svrtable, attribute);

    if(ajStrGetLen(svrvalue))
    {
	ajStrAssignS(value,svrvalue);

	return ajTrue;
    }

    return ajFalse;
}




/* @func ajNamSvrGetAttrS *****************************************************
**
** Return the value for a server attribute
**
** @param [r] name [const AjPStr] server name
** @param [r] attribute [const AjPStr] server attribute name
** @param [w] value [AjPStr *] attribute value
**
** @return [AjBool] true if found
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajNamSvrGetAttrS(const AjPStr name, const AjPStr attribute,
                       AjPStr *value)
{
    return ajNamSvrGetAttrC(name, MAJSTRGETPTR(attribute), value);
}




/* @func ajNamSvrGetAttrSpecialC **********************************************
**
** Return the value for the first occurrence of a named special attribute
**
** @param [r] name [const AjPStr] server name
** @param [r] attribute [const char*] special attribute name
** @param [w] value [AjPStr*] value
**
** @return [ajuint] Number of matching special values
**
** @release 6.5.0
** @@
******************************************************************************/

ajuint ajNamSvrGetAttrSpecialC(const AjPStr name, const char *attribute,
                               AjPStr* value)
{
    ajuint ret = 0;
    ajint j;
    NamPEntry fnew = NULL;
    AjPTable svrtable;
    AjPList svrvalue;
    AjIList iter;
    const AjPStr tmpstr = NULL;
    ajuint attrlen = 0;

    ajDebug("ajNamSvrGetAttrSpecial '%S' '%s'\n", name, attribute);

    attrlen = strlen(attribute);

    fnew = ajTableFetchmodS(namSvrMasterTable, name);

    if(!fnew)
    {
        ajWarn("unknown database '%S'",
               name);
	return ajFalse;
    }

    svrtable = (AjPTable) fnew->data;
    j = namSvrAttrC("special");

    if(j < 0)
    {
	ajWarn("unknown attribute '%s' requested for server '%S'",
               "special", name);
        return ajFalse;
    }

    svrvalue = ajTableFetchmodC(svrtable, "special");

    if(ajListGetLength(svrvalue))
    {
        iter = ajListIterNewread(svrvalue);

        while(!ajListIterDone(iter))
        {
            tmpstr = ajListIterGet(iter);
            if(ajStrPrefixC(tmpstr, attribute) &&
               ajStrGetCharPos(tmpstr, attrlen) == '=')
            {
                if(!ret++)
                {
                    ajStrAssignS(value, tmpstr);
                    ajStrCutStart(value, attrlen+1);
                }
            }
        }

        ajListIterDel(&iter);
    }

    return ret;
}




/* @func ajNamSvrGetAttrSpecialS **********************************************
**
** Return the value for the first occurrence of a named special attribute
**
** @param [r] name [const AjPStr] server name
** @param [r] attribute [const AjPStr] special attribute name
** @param [w] value [AjPStr*] value
**
** @return [ajuint] Number of matching special values
**
** @release 6.5.0
** @@
******************************************************************************/

ajuint ajNamSvrGetAttrSpecialS(const AjPStr name, const AjPStr attribute,
                               AjPStr* value)
{
    return ajNamSvrGetAttrSpecialC(name, MAJSTRGETPTR(attribute), value);
}




/* @func ajNamSvrGetAttrlist **************************************************
**
** Return a list of names and values for all of a server defined attributes
**
** @param [r] name [const AjPStr] server name
**
** @return [AjPList] Tag-value list
**
** @release 6.4.0
** @@
******************************************************************************/

AjPList ajNamSvrGetAttrlist(const AjPStr name)
{
    AjPList ret = NULL;
    AjPTagval tagval = NULL;
    NamPEntry fnew = NULL;
    AjPTable svrtable = NULL;
    const char* svrattr = NULL;
    NamOAttr attr;
    void* svrvalue;
    const AjPStr tmpstr;
    ajuint i;
    AjIList iter;

    fnew = ajTableFetchmodS(namSvrMasterTable, name);

    if(!fnew)
	return NULL;

    ret = ajListNew();
    svrtable = (AjPTable) fnew->data;

    for(i=0; namSvrAttrs[i].Name; i++)
    {
        attr = namSvrAttrs[i];
        svrattr = attr.Name;

        svrvalue = ajTableFetchmodC(svrtable, svrattr);
        if(svrvalue)
        {
            switch(attr.Type)
            {
                case ATTR_LIST:
                    iter = ajListIterNewread(svrvalue);

                    while(!ajListIterDone(iter))
                    {
                        tmpstr = ajListIterGet(iter);
                        tagval = ajTagvalNewC(svrattr,MAJSTRGETPTR(tmpstr));
                        ajListPushAppend(ret, tagval);
                    }

                    ajListIterDel(&iter);
                    break;
                default:
                    tagval = ajTagvalNewC(svrattr,ajStrGetPtr(svrvalue));
                    ajListPushAppend(ret, tagval);
                   break;
            }
        }
    }

    return ret;
  
}




/* @func ajNamSvrGetdbAttrC ***************************************************
**
** Return the value for a server database attribute
**
** @param [r] name [const AjPStr] server name
** @param [r] dbname [const AjPStr] database name
** @param [r] attribute [const char *] database attribute name
** @param [w] value [AjPStr *] attribute value
**
** @return [AjBool] true if found
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajNamSvrGetdbAttrC(const AjPStr name, const AjPStr dbname,
                          const char *attribute,
                          AjPStr *value)
{
    ajint j;
    NamPEntry dbdata = NULL;
    NamPEntry fnew = NULL;
    AjPTable sdbtable;
    AjPTable dbtable;
    const AjPStr dbvalue;

    ajDebug("ajNamSvrGetdbAttrC '%S' '%S' '%s'\n",
            name, dbname, attribute);

    if(!ajNamDatabaseServer(dbname, name))
        return ajFalse;

    dbdata = ajTableFetchmodS(namSvrDatabaseTable, name);
    sdbtable = (AjPTable) dbdata->data;

    fnew = ajTableFetchmodS(sdbtable, dbname);

    dbtable = (AjPTable) fnew->data;
    j = namDbAttrC(attribute);

    if(j < 0)
    {
	ajWarn("unknown attribute '%s' requested for database '%S'",
               attribute, name);
        return ajFalse;
    }

    dbvalue = ajTableFetchmodC(dbtable, attribute);

    if(ajStrGetLen(dbvalue))
    {
	ajStrAssignS(value,dbvalue);

	return ajTrue;
    }

    return ajFalse;
}




/* @func ajNamSvrGetdbAttrS ***************************************************
**
** Return the value for a database attribute
**
** @param [r] name [const AjPStr] server name
** @param [r] dbname [const AjPStr] database name
** @param [r] attribute [const AjPStr] database attribute name
** @param [w] value [AjPStr *] attribute value
**
** @return [AjBool] true if found
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajNamSvrGetdbAttrS(const AjPStr name, const AjPStr dbname,
                          const AjPStr attribute,
                          AjPStr *value)
{
    return ajNamSvrGetdbAttrC(name, dbname, MAJSTRGETPTR(attribute), value);
}




/* @func ajNamSvrGetdbAttrSpecialC ********************************************
**
** Return the value for a server database special attribute
**
** @param [r] name [const AjPStr] server name
** @param [r] dbname [const AjPStr] database name
** @param [r] attribute [const char *] database special attribute name
** @param [w] value [AjPStr *] attribute value
**
** @return [ajuint] Number of matching special values
**
** @release 6.5.0
** @@
******************************************************************************/

ajuint ajNamSvrGetdbAttrSpecialC(const AjPStr name, const AjPStr dbname,
                                 const char *attribute,
                                 AjPStr *value)
{
    ajuint ret = 0;
    ajint j;
    NamPEntry fnew = NULL;
    NamPEntry dbdata = NULL;
    AjPTable sdbtable;
    AjPTable dbtable;
    AjPList dbvalue;
    AjIList iter;
    const AjPStr tmpstr = NULL;
    ajuint attrlen = 0;

    ajDebug("ajNamSvrGetdbAttrSpecialC '%S' '%S' '%s'\n",
            name, dbname, attribute);

    if(!ajNamDatabaseServer(dbname, name))
        return ajFalse;

    attrlen = strlen(attribute);
    
    dbdata = ajTableFetchmodS(namSvrDatabaseTable, name);
    sdbtable = (AjPTable) dbdata->data;

    fnew = ajTableFetchmodS(sdbtable, dbname);

    dbtable = (AjPTable) fnew->data;
    j = namDbAttrC("special");

    if(j < 0)
    {
	ajWarn("unknown attribute '%s' requested for database '%S'",
               "special", name);
        return ajFalse;
    }

    dbvalue = ajTableFetchmodC(dbtable, "special");

    if(ajListGetLength(dbvalue))
    {
        iter = ajListIterNewread(dbvalue);

        while(!ajListIterDone(iter))
        {
            tmpstr = ajListIterGet(iter);
            if(ajStrPrefixC(tmpstr, attribute) &&
               ajStrGetCharPos(tmpstr, attrlen) == '=')
            {
                if(!ret++)
                {
                    ajStrAssignS(value, tmpstr);
                    ajStrCutStart(value, attrlen+1);
                }
            }
        }

        ajListIterDel(&iter);
    }

    return ret;
}




/* @func ajNamSvrGetdbAttrSpecialS ********************************************
**
** Return the value for a database special attribute
**
** @param [r] name [const AjPStr] server name
** @param [r] dbname [const AjPStr] database name
** @param [r] attribute [const AjPStr] database special attribute name
** @param [w] value [AjPStr *] attribute value
**
** @return [ajuint] Number of matching special values
**
** @release 6.5.0
** @@
******************************************************************************/

ajuint ajNamSvrGetdbAttrSpecialS(const AjPStr name, const AjPStr dbname,
                                 const AjPStr attribute,
                                 AjPStr *value)
{
    return ajNamSvrGetdbAttrSpecialC(name, dbname, MAJSTRGETPTR(attribute),
                                     value);
}




/* @func ajNamSvrDetails ******************************************************
**
** Returns server access method information
**
** @param [r] name [const AjPStr] Server name
** @param [w] type [AjPStr*] datatype - 'Protein', 'Nucleic' etc.
** @param [w] scope [AjPStr*] datatype scope Sequence Features etc.
** @param [w] id [AjBool*] ajTrue = can access single entries
** @param [w] qry [AjBool*] ajTrue = can access wild/query entries
** @param [w] all [AjBool*] ajTrue = can access all entries
** @param [w] comment [AjPStr*] comment about server
** @param [w] svrversion [AjPStr*] Server version
** @param [w] methods [AjPStr*] server access methods formatted
** @param [w] defined [AjPStr*] server definition file short name
** @param [w] cachedirectory [AjPStr*] cache directory name
** @param [w] cachefile [AjPStr*] cache file name
** @param [w] url [AjPStr*] server URL
** @return [AjBool] ajTrue if server details were found
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajNamSvrDetails(const AjPStr name, AjPStr* type, AjPStr *scope,
                       AjBool* id, AjBool* qry, AjBool* all,
                       AjPStr* comment, AjPStr* svrversion,
                       AjPStr* methods, AjPStr* defined,
                       AjPStr *cachedirectory, AjPStr* cachefile,
                       AjPStr *url)
{
    NamPType namtype = NULL;
    NamPEntry fnew = NULL;
    AjPTable svrtable = NULL;
    ajint i;
    ajint qryscope;
    AjPStr attrname  = NULL;
    const AjPStr attrval  = NULL;
    const AjPStr svrtype  = NULL;
    AjBool typetested = ajFalse;
    AjPStrTok handle = NULL;
    AjPStr nexttype = NULL;

    AjPFile cacheopen = NULL;

    AjBool *datatype;

    *id = *qry = *all = ajFalse;
    
    ajStrDelStatic(scope);
    ajStrDelStatic(type);
    ajStrDelStatic(comment);
    ajStrDelStatic(methods);
    ajStrDelStatic(defined);
    ajStrDelStatic(cachedirectory);
    ajStrDelStatic(cachefile);
    ajStrDelStatic(svrversion);
    ajStrDelStatic(url);
    
    fnew = ajTableFetchmodS(namSvrMasterTable, name);

    if(fnew)
    {
	ajDebug("ajNamSvrDetails '%S' found\n", name);

	ajStrAssignS(defined, fnew->file);

	svrtable = (AjPTable) fnew->data;
        svrtype = ajTableFetchmodC(svrtable, "type");

	for(i=0; namSvrAttrs[i].Name; i++)
	{
	    if(namSvrAttrs[i].Type != ATTR_STR)
                continue;

            ajStrAssignC(&attrname, namSvrAttrs[i].Name);
            attrval = ajTableFetchmodS(svrtable, attrname);
                ajDebug("Attribute name = %S, value = %S\n",
                        attrname, attrval);

	    if(ajStrGetLen(attrval))
	    {
                if(ajStrMatchC(attrname, "type"))
                {
                    ajStrTokenAssignC(&handle, attrval, " ,;");
                    ajStrAssignC(type, "");
                    ajStrAssignC(scope, "");
                    AJCNEW0(datatype, AJDATATYPE_MAX);

                    while(ajStrTokenNextParse(&handle, &nexttype))
                    {
                        if(ajStrGetLen(*type))
                            ajStrAppendK(type, ' ');

                        if(ajStrPrefixCaseC(attrval, "N"))
                            ajStrAppendC(type, "Nucleotide");
                        else if(ajStrPrefixCaseC(attrval, "P"))
                            ajStrAppendC(type, "Protein");
                        else 
                        {
                            ajStrFmtTitle(&nexttype);
                            ajStrAppendS(type, nexttype);
                        }

                        typetested = ajTrue;
                        namtype = ajTableFetchmodS(namDbTypeTable, nexttype);

                        if(namtype)
                        {
                            if(!datatype[namtype->DataType]++)
                            {
                                if(ajStrGetLen(*scope))
                                    ajStrAppendK(scope, ' ');

                                ajStrAppendC(scope, namtype->Scope);
                            }
                        }
                        else
                            ajWarn("Server '%S' type '%S' unknown scope",
                                   name, nexttype);
                    }
                    ajStrTokenDel(&handle);
                    AJFREE(datatype);
                    ajStrDel(&nexttype);
                }

		if(ajStrMatchC(attrname, "method"))
		{
		    qryscope = namMethod2Scope(attrval, svrtype);

		    if(qryscope & AJMETHOD_ENTRY)
                        *id = ajTrue;

		    if(qryscope & AJMETHOD_QUERY)
                        *qry = ajTrue;

		    if(qryscope & AJMETHOD_ALL)
                        *all = ajTrue;

		    ajStrAppendS(methods, attrval);
		}

		if(ajStrMatchC(attrname, "methodentry"))
		{
                    qryscope = namMethod2Scope(attrval, svrtype);

		    if(qryscope & AJMETHOD_ENTRY)
                        *id = ajTrue;

		    if(ajStrGetLen(*methods))
			ajStrAppendC(methods, ",");

		    ajStrAppendS(methods, attrval);
		    ajStrAppendC(methods, "(id)");
		}

		if(ajStrMatchC(attrname, "methodquery"))
		{
		    qryscope = namMethod2Scope(attrval, svrtype);

		    if(qryscope & AJMETHOD_ENTRY)
                        *id = ajTrue;

		    if(qryscope & AJMETHOD_QUERY)
                        *qry = ajTrue;

		    if(ajStrGetLen(*methods))
			ajStrAppendC(methods, ",");

		    ajStrAppendS(methods, attrval);
		    ajStrAppendC(methods, "(qry)");
		}

		if(ajStrMatchC(attrname, "methodall"))
		{
		    qryscope = namMethod2Scope(attrval, svrtype);

		    if(qryscope & AJMETHOD_ALL)
                        *all = ajTrue;

		    if(ajStrGetLen(*methods))
			ajStrAppendC(methods, ",");

		    ajStrAppendS(methods, attrval);
		    ajStrAppendC(methods, "(all)");
		}

                if(ajStrMatchC(attrname, "comment"))
		    ajStrAssignS(comment, attrval);

                if(ajStrMatchC(attrname, "serverversion"))
		    ajStrAssignS(svrversion, attrval);

                if(ajStrMatchC(attrname, "cachedirectory"))
		    ajStrAssignS(cachedirectory, attrval);

                if(ajStrMatchC(attrname, "cachefile"))
		    ajStrAssignS(cachefile, attrval);

                if(ajStrMatchC(attrname, "url"))
		    ajStrAssignS(url, attrval);
	    }
	}
	
	if(!typetested)
	{
	    ajDebug("Bad server definition for %S: No type. 'P' assumed\n",
		   name);
	    ajWarn("Bad server definition for %S: No type. 'P' assumed",
		   name);
	    ajStrAssignC(type, "P");
	}

	if(!*id && !*qry && !*all)
        {
	    ajDebug("Bad server definition for %S: No method(s) for access\n",
		   name);
	    ajWarn("Bad server definition for %S: No method(s) for access",
		   name);
	}

        ajStrDel(&attrname);

        if(ajStrGetLen(*cachefile))
        {
            cacheopen = namSvrCacheOpen(*cachefile);
            if(cacheopen)
            {
                ajFmtPrintS(cachefile, "%F", cacheopen);
                ajFileClose(&cacheopen);
            }
        }

	return ajTrue;
    }
    
    ajDebug("  '%S' not found\n", name);

    ajStrDel(&attrname);

    return ajFalse;
}




/* @func ajNamDbGetAttrC ******************************************************
**
** Return the value for a database attribute
**
** @param [r] name [const AjPStr] database name
** @param [r] attribute [const char *] database attribute name
** @param [w] value [AjPStr *] attribute value
**
** @return [AjBool] true if found
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajNamDbGetAttrC(const AjPStr name, const char *attribute,
                       AjPStr *value)
{
    ajint j;
    NamPEntry fnew = NULL;
    AjPTable dbtable;
    const AjPStr dbvalue;

    ajDebug("ajNamDbGetAttrC '%S' '%s'\n", name, attribute);

    fnew = ajTableFetchmodS(namDbMasterTable, name);

    if(!fnew)
    {
        ajWarn("unknown database '%S'",
               name);
	return ajFalse;
    }

    dbtable = (AjPTable) fnew->data;
    j = namDbAttrC(attribute);

    if(j < 0)
    {
	ajWarn("unknown attribute '%s' requested for database '%S'",
               attribute, name);
        return ajFalse;
    }

    dbvalue = ajTableFetchmodC(dbtable, attribute);

    if(ajStrGetLen(dbvalue))
    {
	ajStrAssignS(value,dbvalue);

	return ajTrue;
    }

    return ajFalse;
}




/* @func ajNamDbGetAttrS ******************************************************
**
** Return the value for a database attribute
**
** @param [r] name [const AjPStr] database name
** @param [r] attribute [const AjPStr] database attribute name
** @param [w] value [AjPStr *] attribute value
**
** @return [AjBool] true if found
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajNamDbGetAttrS(const AjPStr name, const AjPStr attribute,
                       AjPStr *value)
{
    return ajNamDbGetAttrC(name, MAJSTRGETPTR(attribute), value);
}




/* @func ajNamDbGetAttrSpecialC ***********************************************
**
** Return the value for the first occurrence of a named special attribute
**
** @param [r] name [const AjPStr] database name
** @param [r] attribute [const char*] special attribute name
** @param [w] value [AjPStr*] value
**
** @return [ajuint] Number of matching special values
**
** @release 6.5.0
** @@
******************************************************************************/

ajuint ajNamDbGetAttrSpecialC(const AjPStr name, const char *attribute,
                              AjPStr* value)
{
    ajuint ret = 0;
    ajint j;
    NamPEntry fnew = NULL;
    AjPTable dbtable;
    AjPList dbvalue;
    AjIList iter;
    const AjPStr tmpstr = NULL;
    ajuint attrlen = 0;

    ajDebug("ajNamDbGetAttrSpecial '%S' '%s'\n", name, attribute);

    attrlen = strlen(attribute);

    fnew = ajTableFetchmodS(namDbMasterTable, name);

    if(!fnew)
    {
        ajWarn("unknown database '%S'",
               name);
	return ajFalse;
    }

    dbtable = (AjPTable) fnew->data;
    j = namDbAttrC("special");

    if(j < 0)
    {
	ajWarn("unknown attribute '%s' requested for database '%S'",
               "special", name);
        return ajFalse;
    }

    dbvalue = ajTableFetchmodC(dbtable, "special");

    if(ajListGetLength(dbvalue))
    {
        iter = ajListIterNewread(dbvalue);

        while(!ajListIterDone(iter))
        {
            tmpstr = ajListIterGet(iter);
            if(ajStrPrefixC(tmpstr, attribute) &&
               ajStrGetCharPos(tmpstr, attrlen) == '=')
            {
                if(!ret++)
                {
                    ajStrAssignS(value, tmpstr);
                    ajStrCutStart(value, attrlen+1);
                }
            }
        }

        ajListIterDel(&iter);
    }

    return ret;
}




/* @func ajNamDbGetAttrSpecialS ***********************************************
**
** Return the value for the first occurrence of a named special attribute
**
** @param [r] name [const AjPStr] database name
** @param [r] attribute [const AjPStr] special attribute name
** @param [w] value [AjPStr*] value
**
** @return [ajuint] Number of matching special values
**
** @release 6.5.0
** @@
******************************************************************************/

ajuint ajNamDbGetAttrSpecialS(const AjPStr name, const AjPStr attribute,
                              AjPStr* value)
{
    return ajNamDbGetAttrSpecialC(name, MAJSTRGETPTR(attribute), value);
}




/* @func ajNamDbGetAttrlist ***************************************************
**
** Return a list of names and values for all of a database defined attributes
**
** @param [r] name [const AjPStr] database name
**
** @return [AjPList] Tag-value list
**
** @release 6.4.0
** @@
******************************************************************************/

AjPList ajNamDbGetAttrlist(const AjPStr name)
{
    AjPList ret = NULL;
    AjPTagval tagval = NULL;
    NamPEntry fnew = NULL;
    AjPTable dbtable = NULL;
    const char* dbattr = NULL;
    NamOAttr attr;
    void* dbvalue;
    const AjPStr tmpstr;
    ajuint i;
    AjIList iter;

    fnew = ajTableFetchmodS(namDbMasterTable, name);

    if(!fnew)
    {
	return NULL;
    }

    ret = ajListNew();
    dbtable = (AjPTable) fnew->data;

    for(i=0; namDbAttrs[i].Name; i++)
    {
        attr = namDbAttrs[i];
        dbattr = attr.Name;

        dbvalue = ajTableFetchmodC(dbtable, dbattr);

        if(dbvalue)
        {
            switch(attr.Type)
            {
                case ATTR_LIST:
                    iter = ajListIterNewread(dbvalue);

                    while(!ajListIterDone(iter))
                    {
                        tmpstr = ajListIterGet(iter);
                        tagval = ajTagvalNewC(dbattr, MAJSTRGETPTR(tmpstr));
                        ajListPushAppend(ret, tagval);
                    }

                    ajListIterDel(&iter);
                    break;
                default:
                    tagval = ajTagvalNewC(dbattr, ajStrGetPtr(dbvalue));
                    ajListPushAppend(ret, tagval);
                   break;
            }
        }
    }

    return ret;
  
}




/* @funcstatic namDbtablePutAttrS *********************************************
**
** Store the value for a database attribute
**
** @param [u] dbtable [AjPTable] Database attribute table
** @param [u] Pattribute [AjPStr *] database attribute name
** @param [u] Pvalue [AjPStr*] attribute value
**
** @return [AjBool] true if found
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool namDbtablePutAttrS(AjPTable dbtable, AjPStr *Pattribute,
                                 AjPStr *Pvalue)
{
    void *oldval;
    NamPAttr attr;
    AjPList fdlist = NULL;

    attr  = ajTableFetchmodS(namDbAttrTable, *Pattribute);
    namUser("store db attr '%S' => '%S'\n", *Pattribute, *Pvalue);

    switch(attr->Type) 
    {
        case ATTR_LIST:
            fdlist = ajTableFetchmodS(dbtable, *Pattribute);

            if(!fdlist)
            {
                fdlist = ajListstrNew();
                ajTablePut(dbtable, (AjPStr) *Pattribute, (AjPList) fdlist);
                *Pattribute = NULL;
            }
            else
                ajStrDel(Pattribute);

            ajListstrPushAppend(fdlist, *Pvalue);
            break;

        default:
            oldval = ajTablePut(dbtable, (AjPStr) *Pattribute,
                                (AjPStr) *Pvalue);
            if(oldval)
                ajWarn("duplicate db keyword value for '%S' = '%S'",
                       *Pattribute, (AjPStr) oldval);

            *Pattribute = NULL;
            break;
    }
    
    *Pvalue = NULL;

    return ajTrue;
}




/* @funcstatic namRstablePutAttrS *********************************************
**
** Store the value for a resource attribute
**
** @param [u] rstable [AjPTable] Resource attribute table
** @param [u] Pattribute [AjPStr*] resource attribute name
** @param [u] Pvalue [AjPStr*] attribute value
**
** @return [AjBool] true if found
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool namRstablePutAttrS(AjPTable rstable, AjPStr *Pattribute,
                                 AjPStr *Pvalue)
{
    void *oldval;
    NamPAttr attr;
    AjPList fdlist = NULL;

    attr  = ajTableFetchmodS(namResAttrTable, *Pattribute);
    namUser("store resource attr '%S' => '%S\n", *Pattribute, *Pvalue);

    if(!attr)
        ajTablePut(rstable, (AjPStr) *Pattribute,
                   (AjPStr) *Pvalue);
    else
    {
        switch(attr->Type) 
        {
            case ATTR_LIST:
                fdlist = ajTableFetchmodS(rstable, *Pattribute);

                if(!fdlist)
                {
                    fdlist = ajListstrNew();
                    ajTablePut(rstable, (AjPStr) *Pattribute, (AjPList) fdlist);
                    *Pattribute = NULL;
                }
                else
                    ajStrDel(Pattribute);

                ajListstrPushAppend(fdlist, *Pvalue);
            break;

            default:
                oldval = ajTablePut(rstable, (AjPStr) *Pattribute,
                                    (AjPStr) *Pvalue);
                if(oldval)
                    ajWarn("duplicate resource keyword value for '%S' = '%S'",
                           *Pattribute, (AjPStr) oldval);

                *Pattribute = NULL;
                break;
        }
    }

    *Pvalue = NULL;

    return ajTrue;
}




/* @funcstatic namSvrtablePutAttrS ********************************************
**
** Store the value for a server attribute
**
** @param [u] svrtable [AjPTable] Server attribute table
** @param [u] Pattribute [AjPStr*] server attribute name
** @param [u] Pvalue [AjPStr*] attribute value
**
** @return [AjBool] true if found
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool namSvrtablePutAttrS(AjPTable svrtable, AjPStr *Pattribute,
                                  AjPStr *Pvalue)
{
    void *oldval;
    NamPAttr attr;
    AjPList fdlist = NULL;

    attr  = ajTableFetchmodS(namSvrAttrTable, *Pattribute);
    namUser("store server attr '%S' => '%S\n", *Pattribute, *Pvalue);

    switch(attr->Type) 
    {
        case ATTR_LIST:
            fdlist = ajTableFetchmodS(svrtable, *Pattribute);

            if(!fdlist)
            {
                fdlist = ajListstrNew();
                ajTablePut(svrtable, (AjPStr) *Pattribute, (AjPList) fdlist);
                *Pattribute = NULL;
            }
            else
                ajStrDel(Pattribute);

            ajListstrPushAppend(fdlist, *Pvalue);
            break;

        default:
            oldval = ajTablePut(svrtable, (AjPStr) *Pattribute,
                                (AjPStr) *Pvalue);
            if(oldval)
                ajWarn("duplicate server keyword value for '%S' = '%S'",
                       *Pattribute, (AjPStr) oldval);

            *Pattribute = NULL;
            break;
    }
    
    *Pvalue = NULL;

    return ajTrue;
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
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajNamDbDetails(const AjPStr name, AjPStr* type, AjBool* id,
		      AjBool* qry, AjBool* all,
		      AjPStr* comment, AjPStr* release,
		      AjPStr* methods, AjPStr* defined)
{
    NamPEntry fnew = NULL;
    AjPTable dbtable = NULL;
    ajint i;
    ajint scope;
    AjPStr attrname  = NULL;
    const AjPStr attrval  = NULL;
    const AjPStr dbtype  = NULL;
    AjBool typetested = ajFalse;

    *id = *qry = *all = ajFalse;
    
    ajStrDelStatic(type);
    ajStrDelStatic(comment);
    ajStrDelStatic(release);
    ajStrDelStatic(methods);
    ajStrDelStatic(defined);
    
    fnew = ajTableFetchmodS(namDbMasterTable, name);

    if(fnew)
    {
	ajDebug("ajNamDbDetails: '%S' found\n", name);

	ajStrAssignS(defined, fnew->file);

	dbtable = (AjPTable) fnew->data;
        dbtype = ajTableFetchmodC(dbtable, "type");

	for(i=0; namDbAttrs[i].Name; i++)
	{
            if(namDbAttrs[i].Type != ATTR_STR)
                continue;

            ajStrAssignC(&attrname, namDbAttrs[i].Name);

            attrval = ajTableFetchmodS(dbtable, attrname);
	    ajDebug("Attribute name = %S, value = %S\n",
                    attrname, attrval);

	    if(ajStrGetLen(attrval))
	    {
		if(ajStrMatchC(attrname, "type"))
                {
                    if(ajStrPrefixCaseC(attrval, "N"))
                        ajStrAssignC(type, "Nucleotide");
                    else if(ajStrPrefixCaseC(attrval, "P"))
                        ajStrAssignC(type, "Protein");
                    else 
                    {
                        ajStrAssignS(type, attrval);
                        ajStrFmtTitle(type);
                    }

                    typetested = ajTrue;
                }

		if(ajStrMatchC(attrname, "method"))
		{
		    scope = namMethod2Scope(attrval, dbtype);

		    if(scope & AJMETHOD_ENTRY)
                        *id = ajTrue;

		    if(scope & AJMETHOD_QUERY)
                        *qry = ajTrue;

		    if(scope & AJMETHOD_ALL)
                        *all = ajTrue;

		    ajStrAppendS(methods, attrval);
		}

		if(ajStrMatchC(attrname, "methodentry"))
		{
                    scope = namMethod2Scope(attrval, dbtype);

		    if(scope & AJMETHOD_ENTRY)
                        *id = ajTrue;

		    if(ajStrGetLen(*methods))
			ajStrAppendC(methods, ",");

		    ajStrAppendS(methods, attrval);
		    ajStrAppendC(methods, "(id)");
		}

		if(ajStrMatchC(attrname, "methodquery"))
		{
		    scope = namMethod2Scope(attrval, dbtype);

		    if(scope & AJMETHOD_ENTRY)
                        *id = ajTrue;

		    if(scope & AJMETHOD_QUERY)
                        *qry = ajTrue;

		    if(ajStrGetLen(*methods))
			ajStrAppendC(methods, ",");

		    ajStrAppendS(methods, attrval);
		    ajStrAppendC(methods, "(qry)");
		}

		if(ajStrMatchC(attrname, "methodall"))
		{
		    scope = namMethod2Scope(attrval, dbtype);

		    if(scope & AJMETHOD_ALL)
                        *all = ajTrue;

		    if(ajStrGetLen(*methods))
			ajStrAppendC(methods, ",");

		    ajStrAppendS(methods, attrval);
		    ajStrAppendC(methods, "(all)");
		}

                if(ajStrMatchC(attrname, "comment"))
		    ajStrAssignS(comment, attrval);

		if(ajStrMatchC(attrname, "release"))
		    ajStrAssignS(release, attrval);
	    }
	}
	
	if(!typetested)
	{
	    ajDebug("Bad database definition for %S: No type. 'P' assumed\n",
		   name);
	    ajWarn("Bad database definition for %S: No type. 'P' assumed",
		   name);
	    ajStrAssignC(type, "P");
	}

	if(!*id && !*qry && !*all)
        {
	    ajDebug("Bad database definition for %S: No method(s) for access\n",
		   name);
	    ajWarn("Bad database definition for %S: No method(s) for access",
		   name);
	}

        ajStrDel(&attrname);

	return ajTrue;
    }
    
    ajDebug("ajNamDbDetails: FAILED '%S' not found\n", name);

    ajStrDel(&attrname);

    return ajFalse;
}




/* @funcstatic namAccessTest **************************************************
**
** Tests whether a named access method is known for a given data type
**
** @param [r] method [const AjPStr] Access method name
** @param [r] dbtype [const AjPStr] Database type
** @return [AjBool] True if found
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool namAccessTest(const AjPStr method, const AjPStr dbtype)
{
    AjBool result;
    NamPType namtype;

    result = ajTextaccessMethodTest(method);

    if(!result)
    {
        namtype = ajTableFetchmodS(namDbTypeTable, dbtype);

        if(!namtype)
            return result;

        if(namtype->DataType == AJDATATYPE_ASSEMBLY)
            result = ajAssemaccessMethodTest(method);
        else if(namtype->DataType == AJDATATYPE_FEATURES)
            result = ajFeattabaccessMethodTest(method);
        else if(namtype->DataType == AJDATATYPE_OBO)
            result = ajOboaccessMethodTest(method);
        else if(namtype->DataType == AJDATATYPE_RESOURCE)
            result = ajResourceaccessMethodTest(method);
        else if(namtype->DataType == AJDATATYPE_SEQUENCE)
            result = ajSeqaccessMethodTest(method);
        else if(namtype->DataType == AJDATATYPE_TAXON)
            result = ajTaxaccessMethodTest(method);
        else if(namtype->DataType == AJDATATYPE_URL)
            result = ajUrlaccessMethodTest(method);
        else if(namtype->DataType == AJDATATYPE_VARIATION)
            result = ajVaraccessMethodTest(method);
        else if(namtype->DataType == AJDATATYPE_REFSEQ)
            result = ajRefseqaccessMethodTest(method);
    }

    return result;
}




/* @funcstatic namInformatTest ************************************************
**
** Tests whether a named data input format is known for a given data type
**
** @param [r] format [const AjPStr] Format name
** @param [r] dbtype [const AjPStr] Database type
** @return [AjBool] True if found
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool namInformatTest(const AjPStr format, const AjPStr dbtype)
{
    AjBool result = ajFalse;
    NamPType namtype;

    namtype = ajTableFetchmodS(namDbTypeTable, dbtype);

    if(!namtype) 
        return ajFalse;

    ajDebug("namInformatTest format '%S' dbtype '%S' namtype '%s'\n",
            format, dbtype, namtype->Name);

    if(namtype->DataType == AJDATATYPE_ASSEMBLY)
        result = ajAsseminformatTest(format);
    else if(namtype->DataType == AJDATATYPE_FEATURES)
        result = ajFeattabinformatTest(format);
    else if(namtype->DataType == AJDATATYPE_OBO)
        result = ajOboinformatTest(format);
    else if(namtype->DataType == AJDATATYPE_RESOURCE)
        result = ajResourceinformatTest(format);
    else if(namtype->DataType == AJDATATYPE_SEQUENCE)
        result = ajSeqinformatTest(format);
    else if(namtype->DataType == AJDATATYPE_TAXON)
        result = ajTaxinformatTest(format);
    else if(namtype->DataType == AJDATATYPE_URL)
        result = ajUrlinformatTest(format);
    else if(namtype->DataType == AJDATATYPE_VARIATION)
        result = ajVarinformatTest(format);
    else if(namtype->DataType == AJDATATYPE_TEXT)
        result = ajTextinformatTest(format);
    else if(namtype->DataType == AJDATATYPE_REFSEQ)
        result = ajRefseqinformatTest(format);

    return result;
}




/* @funcstatic namMethod2Qlinks ***********************************************
**
** Returns known query link operators
** for the various types of access method for databases.
**
** @param [r] method [const AjPStr] Access method string
** @param [r] datatype [ajint] Enumerated database type
** @return [const char*] Query link operators
**
** @release 6.4.0
** @@
******************************************************************************/

static const char* namMethod2Qlinks(const AjPStr method, ajint datatype)
{
    const char* result = NULL;

    result = ajTextaccessMethodGetQlinks(method);

    if(ajCharMatchC(result, "DATA"))
       result = namDatatype2Qlinks(datatype);

    ajDebug("namMethod2Qlinks method: '%S' datatype: %d text: '%s'\n",
           method, datatype, result);

    if(datatype == AJDATATYPE_TEXT)
        return result;

    if(!result)
    {
        if(datatype == AJDATATYPE_ASSEMBLY)
            result = ajAssemaccessMethodGetQlinks(method);
        else if(datatype == AJDATATYPE_FEATURES)
            result = ajFeattabaccessMethodGetQlinks(method);
        else if(datatype == AJDATATYPE_OBO)
            result = ajOboaccessMethodGetQlinks(method);
        else if(datatype == AJDATATYPE_RESOURCE)
            result = ajResourceaccessMethodGetQlinks(method);
        else if(datatype == AJDATATYPE_SEQUENCE)
            result = ajSeqaccessMethodGetQlinks(method);
        else if(datatype == AJDATATYPE_TAXON)
            result = ajTaxaccessMethodGetQlinks(method);
        else if(datatype == AJDATATYPE_URL)
            result = ajUrlaccessMethodGetQlinks(method);
        else if(datatype == AJDATATYPE_VARIATION)
            result = ajVaraccessMethodGetQlinks(method);
    }

    return result;
}




/* @funcstatic namMethod2Scope ************************************************
**
** Returns OR'ed values of AJMETHOD_ENTRY, AJMETHOD_QUERY and AJMETHOD_ALL
** for the various types of access method for databases.
**
** @param [r] method [const AjPStr] Access method string
** @param [r] dbtype [const AjPStr] Database type
** @return [ajuint] OR'ed values for the valid scope of the access method given
**
** @release 1.0.0
** @@
******************************************************************************/

static ajuint namMethod2Scope(const AjPStr method, const AjPStr dbtype)
{
    ajuint result = 0;
    NamPType namtype;
    AjPStrTok handle = NULL;
    AjPStr nexttype = NULL;

    result = ajTextaccessMethodGetScope(method);

    if(!result)
    {
        ajStrTokenAssignC(&handle, dbtype, " ,;");

        while(ajStrTokenNextParse(&handle, &nexttype))
        {
            namtype = ajTableFetchmodS(namDbTypeTable, nexttype);

            if(!namtype) 
                return result;

            if(namtype->DataType == AJDATATYPE_ASSEMBLY)
                result = ajAssemaccessMethodGetScope(method);
            else if(namtype->DataType == AJDATATYPE_FEATURES)
                result = ajFeattabaccessMethodGetScope(method);
            else if(namtype->DataType == AJDATATYPE_OBO)
                result = ajOboaccessMethodGetScope(method);
            else if(namtype->DataType == AJDATATYPE_RESOURCE)
                result = ajResourceaccessMethodGetScope(method);
            else if(namtype->DataType == AJDATATYPE_SEQUENCE)
                result = ajSeqaccessMethodGetScope(method);
            else if(namtype->DataType == AJDATATYPE_TAXON)
                result = ajTaxaccessMethodGetScope(method);
             else if(namtype->DataType == AJDATATYPE_URL)
                result = ajUrlaccessMethodGetScope(method);
             else if(namtype->DataType == AJDATATYPE_VARIATION)
                result = ajVaraccessMethodGetScope(method);
             else if(namtype->DataType == AJDATATYPE_REFSEQ)
                result = ajRefseqaccessMethodGetScope(method);
        }
    }

    ajStrDel(&nexttype);
    ajStrTokenDel(&handle);

    return result;
}




/* @funcstatic namDatatype2Fields *********************************************
**
** Returns the list of fields that can be processed by the "Read"
** file parsing function for a specified datatype
**
** @param [r] datatype [ajint] Enumerated data type
** @return [const char*] Field name list
**
** @release 6.4.0
** @@
******************************************************************************/

static const char* namDatatype2Fields(ajint datatype)
{
    const char* result = NULL;

    if(datatype == AJDATATYPE_ASSEMBLY)
        result = ajAsseminTypeGetFields();
    else if(datatype == AJDATATYPE_FEATURES)
        result = ajFeattabinTypeGetFields();
    else if(datatype == AJDATATYPE_OBO)
        result = ajOboinTypeGetFields();
    else if(datatype == AJDATATYPE_RESOURCE)
        result = ajResourceinTypeGetFields();
    else if(datatype == AJDATATYPE_SEQUENCE)
        result = ajSeqinTypeGetFields();
    else if(datatype == AJDATATYPE_TAXON)
        result = ajTaxinTypeGetFields();
    else if(datatype == AJDATATYPE_URL)
        result = ajUrlinTypeGetFields();
    else if(datatype == AJDATATYPE_VARIATION)
        result = ajVarinTypeGetFields();

    if(!result)
        result = ajTextinTypeGetFields();

    return result;
}




/* @funcstatic namDatatype2Qlinks *********************************************
**
** Returns the list of query links that can be processed by the "Read"
** file parsing function for a specified datatype
**
** @param [r] datatype [ajint] Enumerated data type
** @return [const char*] Known query link operators
**
** @release 6.4.0
** @@
******************************************************************************/

static const char* namDatatype2Qlinks(ajint datatype)
{
    const char* result = NULL;

    if(datatype == AJDATATYPE_ASSEMBLY)
        result = ajAsseminTypeGetQlinks();
    else if(datatype == AJDATATYPE_FEATURES)
        result = ajFeattabinTypeGetQlinks();
    else if(datatype == AJDATATYPE_OBO)
        result = ajOboinTypeGetQlinks();
    else if(datatype == AJDATATYPE_RESOURCE)
        result = ajResourceinTypeGetQlinks();
    else if(datatype == AJDATATYPE_SEQUENCE)
        result = ajSeqinTypeGetQlinks();
    else if(datatype == AJDATATYPE_TAXON)
        result = ajTaxinTypeGetQlinks();
    else if(datatype == AJDATATYPE_URL)
        result = ajUrlinTypeGetQlinks();
    else if(datatype == AJDATATYPE_VARIATION)
        result = ajVarinTypeGetQlinks();

    if(!result)
        result = ajTextinTypeGetQlinks();

    return result;
}




/* @func ajNamDebugOrigin *****************************************************
**
** Writes a simple list of where the internal tables came from..
**
** @return [void]
**
** @release 1.13.0
** @@
******************************************************************************/

void ajNamDebugOrigin(void)
{
    ajDebug("Defaults and .rc files: %S\n", namFileOrig);

    return;
}




/* @func ajNamDebugServers ****************************************************
**
** Writes a simple debug report of all servers in the internal table.
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajNamDebugServers(void)
{
    ajDebug("SERVER servers\n");
    ajDebug("==============\n");
    namDebugMaster(namSvrMasterTable, TYPE_SVR);
    ajDebug("[SVR done]\n\n");

    return;
}




/* @func ajNamDebugDatabases **************************************************
**
** Writes a simple debug report of all databases in the internal table.
**
** @return [void]
**
** @release 1.13.0
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
**
** @release 2.7.0
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




/* @func ajNamDebugVariables **************************************************
**
** Writes a simple debug report of all environment variables
** in the internal table.
**
** @return [void]
**
** @release 2.7.0
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




/* @func ajNamDebugAliases ****************************************************
**
** Writes a simple debug report of all aliases
** in the internal table.
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajNamDebugAliases(void)
{
    ajDebug("ALIAS names\n");
    ajDebug("=============\n");
    namDebugMaster(namAliasMasterTable, TYPE_ALIAS);
    ajDebug("[ALIAS done]\n\n");

    return;
}




/* @func ajNamListListServers *************************************************
**
** Creates a AjPList list of all servers in the internal table.
**
** @param [w] svrnames [AjPList] Str List of names to be populated
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajNamListListServers(AjPList svrnames)
{
    ajint i;
    NamPEntry fnew;
    void **valarray =  NULL;

    ajTableToarrayValues(namSvrMasterTable, &valarray);
    ajDebug("ajNamListListSevers\n");

    for(i = 0; valarray[i]; i++)
    {
	fnew = (NamPEntry) valarray[i];
	ajDebug("SVR: %S\n", fnew->name);
	ajListstrPushAppend(svrnames, fnew->name);
    }

    AJFREE(valarray);

    return;
}




/* @func ajNamListListDatabases ***********************************************
**
** Creates a AjPList list of all databases in the internal table.
**
** @param [w] dbnames [AjPList] Str List of names to be populated
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajNamListListDatabases(AjPList dbnames)
{
    ajint i;
    NamPEntry fnew;
    void **valarray =  NULL;

    ajTableToarrayValues(namDbMasterTable, &valarray);
    ajDebug("ajNamListListDatabases\n");

    for(i = 0; valarray[i]; i++)
    {
	fnew = (NamPEntry) valarray[i];
	ajDebug("DB: %S\n", fnew->name);
	ajListstrPushAppend(dbnames, fnew->name);
    }

    AJFREE(valarray);

    return;
}




/* @func ajNamListListResources ***********************************************
**
** Creates a AjPList list of all databases in the internal table.
**
** @param [w] rsnames [AjPList] Str List of names to be populated
** @return [void]
**
** @release 2.7.0
** @@
******************************************************************************/

void ajNamListListResources(AjPList rsnames)
{
    ajint i;
    NamPEntry fnew;
    void **valarray = NULL;

    ajTableToarrayValues(namResMasterTable, &valarray);

    for(i = 0; valarray[i]; i++)
    {
	fnew = (NamPEntry) valarray[i];
	ajDebug("RES: %S\n", fnew->name);
	ajListstrPushAppend(rsnames, fnew->name);
    }

    AJFREE(valarray);

    return;
}




/* @funcstatic namDebugVariables **********************************************
**
** Writes a simple list of all variables in the internal table.
**
** @return [void]
**
** @release 2.7.0
** @@
******************************************************************************/

static void namDebugVariables(void)
{
    namUser("ENV---------->\n");
    namDebugMaster(namVarMasterTable, TYPE_ENV);
    namUser("ENV---------->\n");
    namUser("\n");

    return;
}




/* @funcstatic namDebugAliases ************************************************
**
** Writes a simple list of all aliases in the internal table.
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void namDebugAliases(void)
{
    namUser("ALIAS---------->\n");
    namDebugMaster(namAliasMasterTable, TYPE_ALIAS);
    namUser("ALIAS---------->\n");
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
**
** @release 1.0.0
** @@
******************************************************************************/

static void namListParse(AjPList listwords, AjPList listcount,
			 AjPFile file, const AjPStr shortname)
{
    static char* tabname   = NULL;
    static AjPStr name     = NULL;
    AjPStr value    = NULL;
    static NamPAttr attrvalue  = NULL;
    static char quoteopen  = '\0';
    static char quoteclose = '\0';
    static AjPTable svattr  = NULL;
    static AjPTable dbattr  = NULL;
    static AjPTable rsattr  = NULL;
    static ajint  svr_input = -1;
    static ajint  db_input = -1;
    static ajint  rs_input = -1;
    AjPStr rs_name = NULL;
    AjPTable saveTable = NULL;

    NamPEntry fnew  = NULL;
    NamPEntry entry = NULL;

    AjBool svrsave = ajFalse;
    AjBool dbsave = ajFalse;
    AjBool rssave = ajFalse;
    AjBool saveit = ajFalse;
    AjBool saveifblock = ajFalse;
    static ajint nsvrattr = 0;
    static ajint ndbattr = 0;
    static ajint ndbtype = 0;
    static ajint nrsattr = 0;

    AjPStr includefn = NULL;
    AjPFile iinf     = NULL;
    AjPStr key       = NULL;
    AjPStr val       = NULL;
    AjPStr oldval    = NULL;

    static AjPTable Ifiles = NULL;
    AjPStr curword = NULL;

    ajint wordcount = 0;
    ajint linecount = 0;
    ajint lineword  = 0;
    ajint *iword    = NULL;
    AjBool namstatus;
    AjPStr saveshortname = NULL;
    AjPStr saveword = NULL;
    AjPStr teststr = NULL;

    namUser("namListParse shortname: '%S'\n", shortname);

    /* ndbtype = count database types*/
    if(!ndbtype)
    {
        if(!namDbTypeTable)
            namDbTypeTable = ajTablestrNewCase(30);

	for(ndbtype=0; namDbTypes[ndbtype].Name; ndbtype++)
        {
            name = ajStrNewC(namDbTypes[ndbtype].Name);
            ajTablePut(namDbTypeTable, name, &namDbTypes[ndbtype]);
         }

        name = NULL;
    }

    /* nsvrattr = count server attributes */
    if(!nsvrattr)
    {
        if(!namSvrAttrTable)
            namSvrAttrTable = ajTablestrNew(30);

	for(nsvrattr=0; namSvrAttrs[nsvrattr].Name; nsvrattr++)
        {
            name = ajStrNewC(namSvrAttrs[nsvrattr].Name);
            attrvalue = &namSvrAttrs[nsvrattr];
            oldval = ajTablePut(namSvrAttrTable, name, attrvalue);
            namUser("store server default '%S' => '%s\n",
                    name, attrvalue->Defval);

            if(oldval)
            {
                ajWarn("duplicate server attr value for '%S' = '%S'",
                       name, oldval);
                ajStrDel(&oldval);
            }
        }

        name = NULL;
        value = NULL;
    }

    /* ndbattr = count database attributes */
    if(!ndbattr)
    {
        if(!namDbAttrTable)
            namDbAttrTable = ajTablestrNew(30);

	for(ndbattr=0; namDbAttrs[ndbattr].Name; ndbattr++)
        {
            name = ajStrNewC(namDbAttrs[ndbattr].Name);
            attrvalue = &namDbAttrs[ndbattr];
            oldval = ajTablePut(namDbAttrTable, name, attrvalue);
            namUser("store database default '%S' => '%s\n",
                    name, attrvalue->Defval);

            if(oldval)
            {
                ajWarn("duplicate db attr value for '%S' = '%S'",
                       name, oldval);
                ajStrDel(&oldval);
            }
        }

        name = NULL;
        value = NULL;
    }

    /* nrsattr = count resource attributes */
    if(!nrsattr)
    {
        if(!namResAttrTable)
            namResAttrTable = ajTablestrNew(30);

	for(nrsattr=0; namRsAttrs[nrsattr].Name; nrsattr++)
        {
            name = ajStrNewC(namRsAttrs[nrsattr].Name);
            attrvalue = &namRsAttrs[nrsattr];
            oldval = ajTablePut(namResAttrTable, name, attrvalue);
            namUser("store resource default '%S' => '%s\n",
                    name, attrvalue->Defval);

            if(oldval)
            {
                ajWarn("duplicate res attr value for '%S' = '%S'",
                       name, oldval);
                ajStrDel(&oldval);
            }
        }

        name = NULL;
        value = NULL;
    }

    ajStrAssignS(&saveshortname, shortname);
    ajStrDel(&name);
    ajStrDel(&value);
    quoteopen  = '\0';
    quoteclose = '\0';
    
    namLine = 1;
    namUser("namListParse of %F '%S' words: %Lu lines: %Lu\n", 
	    file, name, ajListGetLength(listwords), ajListGetLength(listcount));
    
    while(ajListstrPop(listwords, &curword))
    {
	while(ajListGetLength(listcount) && (lineword < wordcount))
	{
	    namUser("ajListPop %d < %d list %Lu\n",
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
	    else if(ajCharPrefixS("setenv", curword))
		namParseType = TYPE_ENV;
	    else if(ajCharPrefixS("server",curword))
		namParseType = TYPE_SVR;
	    else if(ajCharPrefixS("dbname",curword))
		namParseType = TYPE_DB;
	    else if(ajCharPrefixS("resource",curword))
		namParseType = TYPE_RESOURCE;
	    else if(ajCharPrefixS("alias",curword))
		namParseType = TYPE_ALIAS;
	    else if(ajCharPrefixS("include",curword))
		namParseType = TYPE_IFILE;
	    else if(ajCharPrefixS("if",curword))
            {
                namParseType = TYPE_IF;
                saveifblock = ajFalse;
            }
	    else if(ajCharPrefixS("ifdef",curword))
            {
                namParseType = TYPE_IFDEF;
                saveifblock = ajFalse;
            }
	    else if(ajCharPrefixS("else",curword))
		namParseType = TYPE_ELSE;
	    else if(ajCharPrefixS("endif",curword))
		namParseType = TYPE_ENDIF;

	    if(!namParseType)		/* test: badtype.rc */
		namError("Invalid definition type '%S'", curword);

	    namUser("type set to %s curword '%S'\n",
		    namTypes[namParseType], curword);
	}
	else if(quoteopen && ajStrMatchC(curword, "]"))
	{				/* test; dbnoquote.rc */
	    namError("']' found, unclosed quotes in '%S'", value);
	    quoteopen    = '\0';
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
		quoteopen = quoteclose = '\0';

		if(namParseType == TYPE_ENV) /* set save flag, value found */
		    saveit = ajTrue;
		else if(namParseType == TYPE_SVR)
		    svrsave = ajTrue;
		else if(namParseType == TYPE_DB)
		    dbsave = ajTrue;
		else if(namParseType == TYPE_RESOURCE)
		    rssave = ajTrue;
		else if(namParseType == TYPE_ALIAS)
		    saveit = ajTrue;
		else if(namParseType == TYPE_IFDEF)
		    saveifblock = ajTrue;
		else if(namParseType == TYPE_IF)
		    saveifblock = ajTrue;
	    }
	}
	else if(namParseType == TYPE_IF || namParseType == TYPE_IFDEF )
	{
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
                saveifblock = ajTrue;
            
            if(ajStrGetCharLast(curword) == quoteclose)
            {
                /* end of quote on same word */
                quoteopen = quoteclose = '\0';
                /* remove quote at the end */
                ajStrCutEnd(&value, 1);
                saveifblock = ajTrue;
            }

            namUser("if: save value '%S' saveif: %B\n", value, saveifblock);
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
		    quoteopen = quoteclose = '\0';
		    saveit= ajTrue;
		    /* remove quote at the end */
		    ajStrCutEnd(&value, 1);
		}

                if(saveit)
                    ajNamResolve(&value);
		namUser("save value '%S' saveit: %B\n", value, saveit);
	    }
	    else
	    {
		ajStrAssignS(&name, curword);
		namUser("save name '%S'\n", name);
	    }
	}

        else if(namParseType == TYPE_ALIAS)
	{
            namUser("ALIAS saveit: %B name: '%S' value '%S' "
                    "quoteopen: '%c'\n",
                    saveit, name, value, quoteopen);
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
		    quoteopen = quoteclose = '\0';
		    saveit= ajTrue;
		    /* remove quote at the end */
		    ajStrCutEnd(&value, 1);
		}

                if(saveit)
                    ajNamResolve(&value);
		namUser("save alias value '%S' saveit: %B\n", value, saveit);
	    }
	    else
	    {
		ajStrAssignS(&name, curword);
		namUser("save alias name '%S'\n", name);
	    }
	}
	
	
	else if(namParseType == TYPE_SVR)
	{
	    if(ajStrMatchC(curword, "[")) /* [ therefore new server */
		svattr = ajTablestrNew(100); /* new server obj */
	    else if(ajStrMatchC(curword, "]"))	/* ] therefore end of server */
		saveit = ajTrue;
	    else if(name)
	    {
		if(svr_input >= 0)
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
			svrsave = ajTrue; /* we are done - simple word */

		    if(ajStrGetCharLast(curword) == quoteclose)
		    {
			quoteopen = quoteclose = '\0';
			ajStrCutEnd(&value,1); /* trim closing quote */
			svrsave = ajTrue;
		    }

		    if(!quoteopen)     /* if we just reset it above */
			svrsave = ajTrue;

                    if(svrsave)
                        ajNamResolve(&value);
		}
		else if(ajStrGetCharLast(curword) == ':')
		{
		    /* if last character is : then its a keyword */
		    ajStrFmtLower(&curword); /* make it lower case */
		    namNoColon(&curword);
		    svr_input = namSvrAttrS(curword);

		    if(svr_input < 0)
			ajWarn("%F: bad attribute '%S' for server '%S'",
				file, curword, name);
		}
                else
                {
                    ajWarn("%F: unexpected token '%S' for server '%S'",
                           file, curword, name);
                }
	    }
	    else
	    {
		ajStrAssignS(&name, curword);

		if(!ajNamIsDbname(name))
		    ajErr("Invalid server name '%S'", name);

		namUser("saving svr name '%S'\n", name);
	    }
	}
	
	else if(namParseType == TYPE_DB)
	{
	    if(ajStrMatchC(curword, "[")) /* [ therefore new database */
		dbattr = ajTablestrNew(100); /* new db obj */
	    else if(ajStrMatchC(curword, "]"))	/* ] therefore end of db */
		saveit = ajTrue;
	    else if(name)
	    {
		if(db_input >= 0)
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
			quoteopen = quoteclose = '\0';
			ajStrCutEnd(&value,1); /* trim closing quote */
			dbsave = ajTrue;
		    }

		    if(!quoteopen)     /* if we just reset it above */
			dbsave = ajTrue;

                    if(dbsave)
                        ajNamResolve(&value);
		}
		else if(ajStrGetCharLast(curword) == ':')
                {
                    /* if last character is : then its a keyword */
                    ajStrFmtLower(&curword); /* make it lower case */
                    namNoColon(&curword);
                    db_input = namDbAttrS(curword);
                        
                    if(db_input < 0)
                        ajWarn("%F: bad attribute '%S' for database '%S'",
                               file, curword, name);
                }
                else
                {
                    ajWarn("%F: unexpected token '%S' for database '%S'",
                           file, curword, name);
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
		rsattr = ajTablestrNew(100); /* new resource*/
	    else if(ajStrMatchC(curword, "]"))	/* ] end of resource */
		saveit = ajTrue;
	    else if(name)
	    {
                /* if last character is : */
		if(ajStrGetCharLast(curword) == ':')
		{		     	     /* then it is a keyword */
		    ajStrFmtLower(&curword); /* make it lower case */
		    namNoColon(&curword);
		    rs_input = namRsAttrS(curword);
                    ajStrDel(&rs_name);

		    if(rs_input < 0)	/* test: badresattr.rc */
                    {
                        if(namRsAttrFieldS(rsattr, curword))
                        {
                            rs_name = ajStrNewRef(curword);
                        }
                        else
                            namError("Bad attribute '%S' for resource '%S'",
                                     curword, name);
                    }
		}
		else if(rs_name || rs_input >= 0)
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
			quoteopen = quoteclose = '\0';
			ajStrCutEnd(&value,1); /* ignore quote if */
					             /* one at end */
			rssave = ajTrue;
		    }

		    if(!quoteopen)
			rssave = ajTrue;
		}

                if(rssave)
                    ajNamResolve(&value);
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
		Ifiles = ajTablestrNew(NAM_INCLUDE_ESTIMATE);
	    namParseType = 0;

            ajStrAssignS(&saveword, curword);

            ajNamResolve(&curword);

	    if(ajTableFetchmodS(Ifiles,curword)) /* test: includeagain.rc */
            {
		if(ajStrMatchS(curword, saveword))
                    namError("%S already read ... skipping", curword);
                else
                    namError("%S (%S) already read ... skipping",
                             saveword, curword);
            }
	    else
	    {
		includefn = ajStrNew();
		ajStrAssignS(&includefn,curword);

		if(namFileOrig)
		    ajStrAppendC(&namFileOrig,", ");
		ajStrAppendS(&namFileOrig,includefn);

		key = ajStrNewS(includefn);
		val = ajStrNewS(includefn);
		oldval = ajTablePut(Ifiles,key,val);
                namUser("store file '%S' => '%S\n", key, val);

                if(oldval)
                {
                    ajWarn("duplicate include value for '%S' = '%S'",
                           key, oldval);
                    ajStrDel(&key);
                    ajStrDel(&oldval);
                }

                /* test: badinclude.rc */
		if(!(iinf = ajFileNewInNameS(includefn)))
		{
                    if(ajStrMatchS(includefn, saveword))
                        namError("Failed to open include file '%S'",
                                 includefn);
                    else
                        namError("Failed to open include file '%S' (%S)",
                                 saveword, includefn);
		    ajStrAppendC(&namFileOrig,"(Failed)");
		}
		else
		{
		    ajStrAppendC(&namFileOrig,"(OK)");
                    /* replaces namFile */
                    ajStrAssignS(&name, includefn);
                    ajFilenameTrimPath(&name);
		    namstatus = namProcessFile(iinf, name);
		    ajFmtPrintS(&namFileName, "%F",file);/* reset saved name */
		    namLine = linecount-1;

		    if(!namstatus)	/* test: badsummary.rc */
                    {
                        if(ajStrMatchS(includefn, saveword))
                            namError("Error(s) found in included file %F",
                                     iinf);
                        else
                            namError("Error(s) found in included file %S (%F)",
                                     saveword, iinf);
                    }
                    

		    ajFileClose(&iinf);
		}

		ajStrDel(&includefn);
	    }

	    namListParseOK = ajTrue;
	}
	
	
	if(svrsave)
	{
	    /* Save the keyword value */
            key = ajStrNewC(namSvrAttrs[svr_input].Name);
            namUser("store svr attr '%S' => '%S\n", key, value);
            namSvrtablePutAttrS(svattr, &key, &value);

	    svr_input =-1;
	    svrsave = ajFalse;
	}

	if(dbsave)
	{
	    /* Save the keyword value */
            key = ajStrNewC(namDbAttrs[db_input].Name);
            namDbtablePutAttrS(dbattr, &key, &value);

	    db_input =-1;
	    dbsave = ajFalse;
	}

	if(rssave)
	{
            if(rs_input >= 0)
                /* Save the keyword value */
                key = ajStrNewC(namRsAttrs[rs_input].Name);
            else
                key = ajStrNewS(rs_name);
            namUser("store resource attr '%S' => '%S\n", key, value);
            namRstablePutAttrS(rsattr, &key, &value);

	    rs_input =-1;
            ajStrDel(&rs_name);
	    rssave = ajFalse;
	}
	
	namListParseOK = saveit;
	
        if(namParseType == TYPE_ENDIF)
        {
            if(!namIfBlock)
            {
                namError("endif not in if-block");
            }
            else
            {
                --namIfBlock;
                ajListPop(namIfList, (void**) &namIfValue);
                namUser("endif restore %B (block %u)\n",
                       namIfValue, namIfBlock);

                if(*namIfValue)
                    namIfNow = ajTrue;
                else
                    namIfNow = ajFalse;
            }
            namUser("endif %B (block %u)\n", namIfNow, /* FIXME teststr, */ namIfBlock);
            ajStrDel(&teststr);
            namParseType = 0;
	    namListParseOK = ajTrue;
        }
        else if(namParseType == TYPE_ELSE)
        {
            namIfNow = !namIfNow;
            namUser("else %B (block %u)\n", namIfNow, namIfBlock);
            namParseType = 0;
	    namListParseOK = ajTrue;
        }

	if(saveifblock)
	{
            if(namParseType == TYPE_IF || namParseType == TYPE_IFDEF )
            {
                ajNamResolve(&value);

                namUser("if: '%S'\n", value);

                if(!namIfList)
                    namIfList = ajListNew();

                if(namIfNow)
                    ajListPush(namIfList, (void*) &namTrue);
                else
                    ajListPush(namIfList, (void*) &namFalse);

                ++namIfBlock;

                if(namParseType == TYPE_IF)
                {
                    ajNamGetValueS(value, &teststr);
                    namUser("if '%S' '%S'\n", value, teststr);
                    if(!ajStrToBool(teststr, &namIfNow))
                        namIfNow = ajFalse;
                }
                else 
                {
                    namUser("ifdef '%S'\n", value);
                    if(ajNamGetValueS(value, &teststr))
                        namIfNow = ajTrue;
                    else
                        namIfNow = ajFalse;
                }

                namUser("if value '%S' '%S' %B (block %u)\n",
                       value, teststr, namIfNow, namIfBlock);
                namParseType = 0;
                ajStrDel(&teststr);
                namListParseOK = ajTrue;
                ajStrDel(&value);
            }
            saveifblock = ajFalse;
        }

	else if(saveit)
	{
            if(!namIfNow)
            {
                namUser("skipping type %d name '%S' value '%S' line:%d\n",
                        namParseType, name, value, namLine);
                ajStrDel(&name);
                saveTable = NULL;

		namSvrAttrtableFree(&svattr);
                namDbAttrtableFree(&dbattr);
                namRsAttrtableFree(&rsattr);
            }
            else
            {
                namUser("saving type %d name '%S' value '%S' line:%d\n",
                        namParseType, name, value, namLine);
                AJNEW0(fnew);
                tabname = ajCharNewS(name);
                fnew->name = name;
                name = NULL;
                fnew->value = value;
                value = NULL;
                fnew->file = ajStrNewRef(saveshortname);

                if(namParseType == TYPE_SVR)
                {
                    fnew->data = (AjPTable) svattr;
                    saveTable = namSvrMasterTable;
                    svattr = NULL;
                }
                else if(namParseType == TYPE_DB)
                {
                    fnew->data = (AjPTable) dbattr;
                    saveTable = namDbMasterTable;
                    dbattr = NULL;
                }
                else if(namParseType == TYPE_RESOURCE)
                {
                    fnew->data = (AjPTable) rsattr;
                    saveTable = namResMasterTable;
                    rsattr = NULL;
                }
                else if(namParseType == TYPE_ENV)
                {
                    fnew->data = NULL;
                    saveTable = namVarMasterTable;
                }
                else if(namParseType == TYPE_ALIAS)
                {
                    fnew->data = NULL;
                    saveTable = namAliasMasterTable;
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
                namUser("store entry '%s' '%S' (%S)\n",
                        tabname, fnew->name, fnew->file);
                if(entry)
                {
                    /* it existed so over wrote previous table entry
                    ** Only a namUser message - redefining EMBOSSRC in
                    ** QA testing can give too many warnings
                    */
                    namUser("namTypes: [%d] '%s'\n",
                            namParseType, namTypes[namParseType]);
                    namUser("shortname: '%S'\n",
                            shortname);
                    namUser("entry: '%S' in file '%S'\n",
                            entry->name, entry->file);
                    namUser("%S: replaced %s %S definition from %S\n",
                            shortname,
                            namTypes[namParseType],
                            entry->name,
                            entry->file);
                    namEntryDelete(&entry, namParseType); /* previous entry */
                }
	    }

	    saveit = ajFalse;
	    namParseType = 0;
            saveTable = NULL;
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
	namUser("** remaining wordcount items: %lu\n",
                ajListGetLength(listcount));

	while(ajListGetLength(listcount))
	{
	    ajListPop(listcount, (void**) &iword);
	    AJFREE(iword);
	}
    }

    if(value)
	namUser("++ namListParse value %x '%S'", value, value);

    ajStrDel(&saveword);
    ajStrDel(&saveshortname);
    ajTablestrFree(&Ifiles);

    return;
}




/* @func ajNamIsDbname ********************************************************
**
** Returns true if the name is a valid database (or server) name.
** 
** Database names must start with a letter, and have 1 or more letters,
** numbers or underscores. No other characters are permitted.
**
** @param [r] name [const AjPStr] character string to find in getenv list
** @return [AjBool] True if name was defined.
**
** @release 3.0.0
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
**
** @release 2.9.0
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




/* @func ajNamGetenvS *********************************************************
**
** Looks for name as an environment variable.
** the AjPStr for this in "value". If not found returns NULL;
**
** @param [r] name [const AjPStr] character string to find in getenv list
** @param [w] value [AjPStr*] String for the value.
** @return [AjBool] True if name was defined.
**
** @release 6.1.0
** @@
**
******************************************************************************/

AjBool ajNamGetenvS(const AjPStr name,
		    AjPStr* value)
{
    return ajNamGetenvC(ajStrGetPtr(name), value);
}




/* @func ajNamGetAliasC *******************************************************
**
** Looks for name as an alias in the hash table and if found returns
** the AjPStr for this in "value". If not found returns NULL;
**
** @param [r] name [const char*] character string find in hash table.
** @param [w] value [AjPStr*] Str for the value.
** @return [AjBool] True if name was defined.
**
** @release 6.4.0
** @@
**
******************************************************************************/

AjBool ajNamGetAliasC(const char* name, AjPStr* value)
{
    NamPEntry fnew       = NULL;
    
    fnew = ajTableFetchmodC(namAliasMasterTable, name);

    if(fnew)
    {
        ajDebug("Found alias '%s' value '%S' (%S)\n",
                name, fnew->value, fnew->file);
	ajStrAssignS(value, fnew->value);

	return ajTrue;
    }

    return ajFalse;
}




/* @func ajNamGetAliasS *******************************************************
**
** Looks for name as an alias in the hash table and if found returns
** the AjPStr for this in "value". If not found returns NULL;
**
** @param [r] namestr [const AjPStr] character string find in hash table.
** @param [w] value [AjPStr*] Str for the value.
** @return [AjBool] True if name was defined.
**
** @release 6.4.0
** @@
**
******************************************************************************/

AjBool ajNamGetAliasS(const AjPStr namestr, AjPStr* value)
{
    NamPEntry fnew       = NULL;
    
    fnew = ajTableFetchmodS(namAliasMasterTable, namestr);

    if(fnew)
    {
        ajDebug("Found alias '%S' value '%S' (%S)\n",
                namestr, fnew->value, fnew->file);
	ajStrAssignS(value, fnew->value);

	return ajTrue;
    }

    return ajFalse;
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
**
** @release 1.0.0
** @@
**
******************************************************************************/

AjBool ajNamGetValueC(const char* name, AjPStr* value)
{
    NamPEntry fnew       = NULL;
    AjBool hadPrefix     = ajFalse;
    AjBool ret           = ajFalse;

    ajDebug("ajNamGetValueC '%s'\n", name);

    if(ajCharPrefixCaseS(name, namPrefixStr)) /* may already have the prefix */
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
    
    ajDebug("ajNamGetValueC search for '%S': ", namValNameTmp);

    /* first test for an ENV variable */
    
    ret = ajNamGetenvS(namValNameTmp, value);

    if(ret)
    {
        ajDebug("Found environment variable value '%S'\n", *value);
	return ajTrue;
    }

    /* then test the table definitions - with the prefix */
    
    fnew = ajTableFetchmodS(namVarMasterTable, namValNameTmp);

    if(fnew)
    {
        ajDebug("Found %S prefix set variable '%S' value '%S' (%S)\n",
                namPrefixStr, namValNameTmp, fnew->value, fnew->file);
	ajStrAssignS(value, fnew->value);

	return ajTrue;
    }
    
    if(!hadPrefix)
    {

	/* then test the table definitions - as originally specified */

	fnew = ajTableFetchmodC(namVarMasterTable, name);

	if(fnew)
	{
            ajDebug("Found %S noprefix set variable '%s' value '%S' (%S)\n",
                    namPrefixStr, name, fnew->value, fnew->file);
	    ajStrAssignS(value, fnew->value);

	    return ajTrue;
	}
    }

    if(ajStrMatchC(namValNameTmp, "EMBOSS_INSTALLDIRECTORY"))
    {
        ajStrAssignS(value, ajNamValueInstalldir());
        ajDebug("Found builtin variable '%S' value '%S'\n",
                namValNameTmp, *value);
        return ajTrue;
    }
    
    if(ajStrMatchC(namValNameTmp, "EMBOSS_ROOTDIRECTORY"))
    {
        ajStrAssignS(value, ajNamValueRootdir());
        ajDebug("Found builtin variable '%S' value '%S'\n",
                namValNameTmp, *value);
        return ajTrue;
    }
    
    if(ajStrMatchC(namValNameTmp, "EMBOSS_BASEDIRECTORY"))
    {
        ajStrAssignS(value, ajNamValueBasedir());
        ajDebug("Found builtin variable '%S' value '%S'\n",
                namValNameTmp, *value);
        return ajTrue;
    }

    ajDebug("%S not found\n", namValNameTmp);

    return ajFalse;
}




/* @func ajNamGetValueS *******************************************************
**
** Looks for name as an (upper case) environment variable,
** and then as-is in the hash table and if found returns
** the AjPStr for this in "value". If not found returns NULL;
**
** @param [r] name [const AjPStr] character string find in hash table.
** @param [w] value [AjPStr*] String for the value.
** @return [AjBool] True if name was defined.
**
** @release 6.1.0
** @@
**
******************************************************************************/

AjBool ajNamGetValueS(const AjPStr name, AjPStr* value)
{
    return ajNamGetValueC(ajStrGetPtr(name), value);
}




/* @func ajNamServer **********************************************************
**
** Looks for name in the server hash table and if found returns
** the attribute for this. If not found returns  NULL;
**
** @param [r] name [const AjPStr] character string find in hash table.
** @return [AjBool] true if server name is valid.
** @error  NULL if name not found in the table
**
** @release 6.4.0
** @@
**
******************************************************************************/

AjBool ajNamServer(const AjPStr name)
{
    const NamPEntry fnew = NULL;

    /* ajDebug("ajNamServer '%S'\n", name); */

    fnew = ajTableFetchS(namSvrMasterTable, name);

    if(fnew)
    {
	/* ajDebug("  '%S' found\n", name); */
	return ajTrue;
    }

    /* ajDebug("  '%S' not found\n", name); */
    return ajFalse;
}




/* @func ajNamDatabase ********************************************************
**
** Looks for name in the database hash table and if found returns
** true.
**
** @param [r] name [const AjPStr] character string find in hash table.
** @return [AjBool] true if database name is valid.
** @error  NULL if name not found in the table
**
** @release 1.0.0
** @@
**
******************************************************************************/

AjBool ajNamDatabase(const AjPStr name)
{
    const NamPEntry fnew = NULL;

    /* ajDebug("ajNamDatabase '%S'\n", name); */

    fnew = ajTableFetchS(namDbMasterTable, name);

    if(fnew)
    {
	/* ajDebug("  '%S' found\n", name); */
	return ajTrue;
    }

    /* ajDebug("  '%S' not found\n", name); */
    return ajFalse;
}




/* @func ajNamAliasDatabase ***************************************************
**
** Looks for name in the database hash table and the database hash
** table and if found returns the true name for this. If not found
** returns NULL.
**
** @param [u] Pname [AjPStr*] character string to find in hash table.
** @return [AjBool] true if database name is valid.
**
** @release 6.4.0
** @@
**
******************************************************************************/

AjBool ajNamAliasDatabase(AjPStr *Pname)
{
    const NamPEntry fnew = NULL;
    AjPStr alias = NULL;
    const AjPStr tryname = *Pname;

    /* ajDebug("ajNamAliasDatabase '%S'\n", *Pname); */

    fnew = ajTableFetchS(namDbMasterTable, *Pname);

    if(fnew)
    {
	/* ajDebug("  '%S' found\n", *Pname); */
	return ajTrue;
    }

    while(ajNamGetAliasS(tryname, &alias))
    {
        fnew = ajTableFetchS(namDbMasterTable, alias);
        if(fnew)
        {
            ajStrAssignS(Pname, alias);
            /*ajDebug("  '%S' found as '%S'\n", name, alias);*/
            ajStrDel(&alias);
            return ajTrue;
        }
        tryname = alias;
    }

    ajStrDel(&alias);
    /* ajDebug("  '%S' not found\n", name); */
    return ajFalse;
}




/* @func ajNamDatabaseServer **************************************************
**
** Looks for a database name for a specific server.
** Reads the server cache file to find the database.
**
** @param [r] name [const AjPStr] Database name to find for server
** @param [r] server [const AjPStr] Server name
** @return [AjBool] true if database name is valid.
** @error  NULL if name not found in the table
**
** @release 6.4.0
** @@
**
******************************************************************************/

AjBool ajNamDatabaseServer(const AjPStr name, const AjPStr server)
{
    AjBool ret = ajTrue;
    const NamPEntry svdata = NULL;
    const NamPEntry dbdata = NULL;
    const NamPEntry sdbdata = NULL;
    AjPTable svrtable;
    AjPTable dbtable;
    const AjPStr svrval = NULL;
    AjPStr cachefile = NULL;

    ajDebug("ajNamDatabaseServer '%S' for server '%S'\n",
            name, server);

    svdata = ajTableFetchS(namSvrMasterTable, server);

    if(!svdata)
    {
	ajErr("Server '%S' not found, unable to search for database '%S'",
              server, name);
	return ajFalse;
    }

    dbdata = ajTableFetchS(namSvrDatabaseTable,server);

    if(!dbdata)
    {

        /* pick up the cache file name for this server */
        svrtable = (AjPTable) svdata->data;
        svrval = ajTableFetchmodC(svrtable, "cachefile");

        if(ajStrGetLen(svrval))
            cachefile = ajStrNewS(svrval);
        else
        {
            cachefile = ajStrNewC("server.");
            ajStrAppendS(&cachefile, server);
        }

        namSvrCacheRead(server, cachefile);
        dbdata = ajTableFetchS(namSvrDatabaseTable, server);

        ajStrDel(&cachefile);
    }

    if(!dbdata)
    {
        ajWarn("Server '%S' has no databases", server);
        return ajFalse;
    }

    dbtable = (AjPTable) dbdata->data;
    sdbdata = ajTableFetchS(dbtable, name);

    if(!sdbdata)
    {
        ajWarn("Server '%S' has no database '%S'", server, name);
        return ajFalse;
    }

    ajDebug("Found database '%S' on server '%S'\n", name, server);

    /* ajDebug("  '%S' not found\n", name); */
    return ret;
}




/* @func ajNamAliasServer *****************************************************
**
** Looks for a database name for a specific server.
** Reads the server cache file to find the database.
**
** @param [u] Pname [AjPStr*] character string to find in hash table.
** @param [r] server [const AjPStr] Server name
** @return [AjBool] true if database name is valid.
**
** @release 6.4.0
** @@
**
******************************************************************************/

AjBool ajNamAliasServer(AjPStr *Pname, const AjPStr server)
{
    const NamPEntry svdata = NULL;
    const NamPEntry dbdata = NULL;
    const NamPEntry sdbdata = NULL;
    const NamPEntry alidata = NULL;
    const NamPEntry salidata = NULL;
    AjPTable svrtable;
    AjPTable dbtable;
    AjPTable alitable;
    const AjPStr svrval = NULL;
    AjPStr cachefile = NULL;
    AjPStr alias = NULL;
    AjBool hasAlias = ajFalse;

    ajDebug("ajNamAliasServer '%S' for server '%S'\n",
            *Pname, server);

    svdata = ajTableFetchS(namSvrMasterTable, server);

    if(!svdata)
    {
	ajErr("Server '%S' not found, unable to search for database '%S'",
              server, *Pname);
	return ajFalse;
    }

    dbdata = ajTableFetchS(namSvrDatabaseTable,server);
    alidata = ajTableFetchS(namSvrAliasTable,server);

    if(!dbdata)
    {

        /* pick up the cache file name for this server */
        svrtable = (AjPTable) svdata->data;
        svrval = ajTableFetchmodC(svrtable, "cachefile");

        if(ajStrGetLen(svrval))
        {
            cachefile = ajStrNewS(svrval);
        }
        else
        {
            cachefile = ajStrNewC("server.");
            ajStrAppendS(&cachefile, server);
        }

        namSvrCacheRead(server, cachefile);
        dbdata = ajTableFetchS(namSvrDatabaseTable, server);
        alidata = ajTableFetchS(namSvrAliasTable, server);

        ajStrDel(&cachefile);
    }

    if(!dbdata)
    {
        ajWarn("Server '%S' has no databases", server);
        return ajFalse;
    }

   ajStrAssignS(&alias, *Pname);

    alitable = (AjPTable) alidata->data;
    salidata = ajTableFetchS(alitable, alias);

    while(salidata)
    {
        hasAlias = ajTrue;
        ajStrAssignS(&alias,salidata->value);
        salidata = ajTableFetchS(alitable, alias);
    }

    dbtable = (AjPTable) dbdata->data;
    sdbdata = ajTableFetchS(dbtable, alias);
    if(!sdbdata)
    {
        if(hasAlias)
            ajWarn("Server '%S' has no database '%S' (alias '%S')",
                   server, *Pname, alias);
        else
            ajWarn("Server '%S' has no database '%S'", server, *Pname);

        ajStrDel(&alias);
        return ajFalse;
    }

    ajDebug("Found database '%S' (alias '%S') on server '%S'\n",
            *Pname, alias, server);

    ajStrAssignS(Pname, alias);

    ajStrDel(&alias);

    return ajTrue;
}




/* @funcstatic namSvrCacheParse ***********************************************
**
** Parses a database cachefile for a server
**
** @param [u] cachefile [AjPFile] Cache file
** @param [u] dbtable [AjPTable] Database table for server
** @param [u] alitable [AjPTable] Alias table for server
** @return [AjBool] True on success
**
**
** @release 6.4.0
******************************************************************************/

static AjBool namSvrCacheParse(AjPFile cachefile, AjPTable dbtable,
                               AjPTable alitable)
{
    AjBool ret = ajTrue;
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

    ajDebug("namSvrCacheParse '%F'\n", cachefile);

    ajFmtPrintS(&namFileName, "%F",cachefile);

    listwords = ajListstrNew();
    listcount = ajListNew();
    word      = ajStrNewRes(128);

    /* Read in the settings. */
    while(ajReadlineTrim(cachefile, &rdline))
    {
	iline++;
 	AJNEW0(k);
	*k = (ajuint) ajListGetLength(listwords);
	ajListPushAppend(listcount, k);

	if(!ajStrCutCommentsStart(&rdline))
	    continue;

	len = ajStrGetLen(rdline);

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

		i++;
                ptr++;
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
    *k = (ajuint) ajListGetLength(listwords);
    ajListPushAppend(listcount, k);
    
    if(!namSvrCacheParseList(listwords, listcount, cachefile,
                             dbtable, alitable))
    {
	ajErr("%F: Unexpected end of file in %S at line %d\n",
              cachefile, iline); /* FIXME */
        ret = ajFalse;
    }

    ajListFree(&listwords);
    ajListFree(&listcount);
    ajStrDel(&word);

    return ret;
}




/* @funcstatic namSvrCacheParseList *******************************************
**
** Parse the text in a list of tokens read from the server cache file.
** Derive server database definitions. Store all these in the server's
** internal table.
**
** @param [u] listwords [AjPList] String list of word tokens to parse
** @param [u] listcount [AjPList] List of word counts per line for
**                                generating error messages
** @param [u] cachefile [AjPFile] Input file only for name in messages
** @param [u] dbtable [AjPTable] Database table for server
** @param [u] alitable [AjPTable] Alias table for server
** @return [AjBool] True on success
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool namSvrCacheParseList(AjPList listwords, AjPList listcount,
                                   AjPFile cachefile,
                                   AjPTable dbtable, AjPTable alitable)
{
    static char* tabname   = NULL;
    static AjPStr name     = NULL;
    static AjPStr value    = NULL;
    static char quoteopen  = '\0';
    static char quoteclose = '\0';
    static AjPTable dbattr  = NULL;
    static ajint  db_input = -1;
    AjPTable saveTable = NULL;

    NamPEntry fnew  = NULL;
    NamPEntry entry = NULL;

    AjPStr includefn = NULL;
    AjPFile iinf     = NULL;
    AjPStr key       = NULL;
    AjPStr val       = NULL;
    AjPStr oldval    = NULL;
    static AjPTable Ifiles = NULL;
    AjPStr curword = NULL;

    ajint wordcount = 0;
    ajint linecount = 0;
    ajint lineword  = 0;
    ajint *iword    = NULL;
    AjBool namstatus;
    AjPStr saveshortname = NULL;
    AjPStr saveword = NULL;

    AjBool dbsave = ajFalse;
    AjBool saveit = ajFalse;

    ajFmtPrintS(&saveshortname, "%F", cachefile);
    quoteopen  = '\0';
    quoteclose = '\0';

    namLine = 1;

    while(ajListstrPop(listwords, &curword))
    {
	while(ajListGetLength(listcount) && (lineword < wordcount))
	{
	    namUser("ajListPop %d < %d list %Lu\n",
		    lineword, wordcount, ajListGetLength(listcount));
	    ajListPop(listcount, (void**) &iword);
	    lineword = *iword;
	    linecount++;
	    namLine = linecount-1;
	    AJFREE(iword);
	}

	wordcount++;

	if(!namParseType)
	{
	    namNoColon(&curword);
	    ajStrFmtLower(&curword);

	    if(ajCharPrefixS("dbname",curword))
		namParseType = TYPE_DB;
	    if(ajCharPrefixS("alias",curword))
		namParseType = TYPE_ALIAS;
	    else if(ajCharPrefixS("include",curword))
		namParseType = TYPE_IFILE;

	    if(!namParseType)		/* test: badtype.rc */
		namError("Invalid definition type '%S'", curword);
	}
	else if(quoteopen && ajStrMatchC(curword, "]"))
	{				/* test; dbnoquote.rc */
	    namError("']' found, unclosed quotes in '%S'", value);
	    quoteopen    = '\0';
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
		quoteopen = quoteclose = '\0';

		if(namParseType == TYPE_DB)
		    dbsave = ajTrue;
		if(namParseType == TYPE_ALIAS)
		    dbsave = ajTrue;
	    }
	}
	else if(namParseType == TYPE_ALIAS)
	{
	    namUser("CACHE ALIAS saveit: %B name: '%S' value '%S' "
                    "quoteopen: '%c'\n",
                    saveit, name, value, quoteopen);
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
		    quoteopen = quoteclose = '\0';
		    saveit= ajTrue;
		    /* remove quote at the end */
		    ajStrCutEnd(&value, 1);
		}

                if(saveit)
                    ajNamResolve(&value);
		namUser("save alias value '%S'\n", value);
	    }
	    else
	    {
		ajStrAssignS(&name, curword);
		namUser("save alias name '%S'\n", name);
	    }
        }
	else if(namParseType == TYPE_DB)
	{
	    if(ajStrMatchC(curword, "[")) /* [ therefore new database */
		dbattr = ajTablestrNew(100); /* new db obj */
	    else if(ajStrMatchC(curword, "]"))	/* ] therefore end of db */
		saveit = ajTrue;
	    else if(name)
	    {
		if(db_input >= 0)
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
			quoteopen = quoteclose = '\0';
			ajStrCutEnd(&value,1); /* trim closing quote */
			dbsave = ajTrue;
		    }

		    if(!quoteopen)     /* if we just reset it above */
			dbsave = ajTrue;

                    if(dbsave)
                        ajNamResolve(&value);
		}
		else if(ajStrGetCharLast(curword) == ':')
		{
		    /* if last character is : then its a keyword */
		    ajStrFmtLower(&curword); /* make it lower case */
		    namNoColon(&curword);
		    db_input = namDbAttrS(curword);

		    if(db_input < 0)
			ajWarn("%F: bad attribute '%S' for database '%S'",
				cachefile, curword, name);
		}
                else
                {
                    ajWarn("%F: unexpected token '%S' for database '%S'",
                           cachefile, curword, name);
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
	
	
	else if(namParseType == TYPE_IFILE)
	{
	    if(!Ifiles)
		Ifiles = ajTablestrNew(NAM_INCLUDE_ESTIMATE);

	    namParseType = 0;

            ajStrAssignS(&saveword, curword);

            ajNamResolve(&curword);

	    if(ajTableFetchS(Ifiles,curword)) /* test: includeagain.rc */
            {
		if(ajStrMatchS(curword, saveword))
                    namError("%S already read ... skipping", curword);
                else
                    namError("%S (%S) already read ... skipping",
                             saveword, curword);
            }
	    else
	    {
		includefn = ajStrNew();
		ajStrAssignS(&includefn,curword);

		if(namFileOrig)
		    ajStrAppendC(&namFileOrig,", ");

		ajStrAppendS(&namFileOrig,includefn);

		key = ajStrNewS(includefn);
		val = ajStrNewS(includefn);
		oldval = ajTablePut(Ifiles,key,val);
                namUser("store file '%S' => '%S\n", key, val);

                if(oldval)
                {
                    ajWarn("duplicate include value for '%S' = '%S'",
                           key, oldval);
                    ajStrDel(&key);
                    ajStrDel(&oldval);
                }

                /* test: badinclude.rc */
		if(!(iinf = ajFileNewInNameS(includefn)))
		{
                    if(ajStrMatchS(includefn, saveword))
                        namError("Failed to open include file '%S'",
                                 includefn);
                    else
                        namError("Failed to open include file '%S' (%S)",
                                 saveword, includefn);
		    ajStrAppendC(&namFileOrig,"(Failed)");
		}
		else
		{
		    ajStrAppendC(&namFileOrig,"(OK)");
                    /* replaces namFile */
                    ajStrAssignS(&name, includefn);
                    ajFilenameTrimPath(&name);
		    namstatus = namProcessFile(iinf, name);
		    ajFmtPrintS(&namFileName, "%F",
                                cachefile);/* reset saved name */
		    namLine = linecount-1;

		    if(!namstatus)	/* test: badsummary.rc */
                    {
                        if(ajStrMatchS(includefn, saveword))
                            namError("Error(s) found in included file %F",
                                     iinf);
                        else
                            namError("Error(s) found in included file %S (%F)",
                                     saveword, iinf);
                    }
                    

		    ajFileClose(&iinf);
		}

		ajStrDel(&includefn);
	    }

	    namListParseOK = ajTrue;
	}
	
	
	if(dbsave)
	{
	    /* Save the keyword value */
            key = ajStrNewC(namDbAttrs[db_input].Name);
            namDbtablePutAttrS(dbattr, &key, &value);

	    db_input =-1;
	    dbsave = ajFalse;
	}

	if(saveit)
	{
	    namUser("saving type %d name '%S' value '%S' line:%d\n",
		    namParseType, name, value, namLine);
	    AJNEW0(fnew);
	    tabname = ajCharNewS(name);
	    fnew->name = name;
	    name = NULL;
	    fnew->value = value;
	    value = NULL;
	    fnew->file = ajStrNewRef(saveshortname);

            if(namParseType == TYPE_DB)
	    {
		fnew->data = (AjPTable) dbattr;
                dbattr = NULL;
		saveTable = dbtable;
	    }
	    else if(namParseType == TYPE_ALIAS)
	    {
		fnew->data = NULL;
		saveTable = alitable;
	    }
	    else
	    {
		fnew->data = NULL;
	    }
	    entry = ajTablePut(saveTable, tabname, fnew);
            namUser("store entry '%s' '%S' (%S)\n",
                    tabname, fnew->name, fnew->file);

	    if(entry)
	    {
		/* it existed so over wrote previous table entry
		** Only a namUser message - redefining EMBOSSRC in
		** QA testing can give too many warnings
		*/
		ajErr("%F: replaced %s %S definition from %S\n",
                      cachefile,
                      namTypes[namParseType],
                      entry->name,
                      entry->file);
		namEntryDelete(&entry, namParseType); /* previous entry */
	    }

	    saveit = ajFalse;
	    namParseType = 0;
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
	namUser("** remaining wordcount items: %Lu\n",
                ajListGetLength(listcount));

	while(ajListGetLength(listcount))
	{
	    ajListPop(listcount, (void**) &iword);
	    AJFREE(iword);
	}
    }

    if(value)
	namUser("++ namListParse value %x '%S'", value, value);

    ajStrDel(&saveword);
    ajStrDel(&saveshortname);
    ajTablestrFree(&Ifiles);

    return ajTrue;
}




/* @func ajNamSvrCount ********************************************************
**
** Reads the database cachefile for a server and counts the number of databases
**
** Searches emboss_standard and the user directory ~/.embossdata/
** testing the file data in each file found and parsing the most recent file.
**
** cache files should all start with a comment line
** containing the file name and the creation date.
**
** @param [r] server [const AjPStr] Server name
** @return [ajuint] Number of databases
**
**
** @release 6.4.0
******************************************************************************/

ajuint ajNamSvrCount(const AjPStr server)
{
    const NamPEntry svdata = NULL;
    const NamPEntry dbdata = NULL;
    AjPTable svrtable;
    AjPTable dbtable;
    const AjPStr svrcache = NULL;
    AjPStr cachefile = NULL;

    dbdata = ajTableFetchS(namSvrDatabaseTable, server);

    if(!dbdata)
    {
        svdata = ajTableFetchS(namSvrMasterTable, server);

        if(!svdata)
        {
            ajErr("Server '%S' not found, unable to count databases",
                  server);
            return 0;
        }

        svrtable = (AjPTable) svdata->data;
        svrcache = ajTableFetchC(svrtable, "cachefile");

        if(ajStrGetLen(svrcache))
            cachefile = ajStrNewS(svrcache);
        else
        {
            cachefile = ajStrNewC("server.");
            ajStrAppendS(&cachefile, server);
        }

        namSvrCacheRead(server, cachefile);
        dbdata = ajTableFetchS(namSvrDatabaseTable, server);

        if(!dbdata)
        {
            ajDebug("ajNamSvrCount: server '%S' has no databases\n", server);
            ajStrDel(&cachefile);
            return 0;
        }
    }

    dbtable = (AjPTable) dbdata->data;

    ajStrDel(&cachefile);
    return (ajuint) ajTableGetLength(dbtable);
}




/* @func ajNamSvrListListDatabases ********************************************
**
** Reads the database cachefile for a server and lists the databases.
**
** Searches emboss_standard and the user directory ~/.embossdata/
** testing the file data in each file found and parsing the most recent file.
**
** cache files should all start with a comment line
** containing the file name and the creation date.
**
** @param [r] server [const AjPStr] Server name
** @param [w] dbnames [AjPList] Str List of names to be populated
** @return [void]
**
**
** @release 6.4.0
******************************************************************************/

void ajNamSvrListListDatabases(const AjPStr server, AjPList dbnames)
{
    void **valarray =  NULL;
    register ajint i = 0;

    const NamPEntry svdata = NULL;
    const NamPEntry dbdata = NULL;
    NamPEntry fnew   = NULL;
    AjPTable svrtable;
    AjPTable dbtable;
    const AjPStr svrcache = NULL;
    AjPStr cachefile = NULL;
    
    /*
    ** TODO: Should server files be read by a separate function?
    ** This function is largely a copy of ajNamSvrCount and
    ** ajNamListListDatabases so that server files are read by both functions.
    */
    
    dbdata = ajTableFetchS(namSvrDatabaseTable, server);
    
    if(!dbdata)
    {
        svdata = ajTableFetchS(namSvrMasterTable, server);
        
        if(!svdata)
        {
            ajErr("Server '%S' not found, unable to count databases",
                  server);
            return;
        }
        
        svrtable = (AjPTable) svdata->data;
        svrcache = ajTableFetchC(svrtable, "cachefile");
        
        if(ajStrGetLen(svrcache))
            cachefile = ajStrNewS(svrcache);
        else
        {
            cachefile = ajStrNewC("server.");
            ajStrAppendS(&cachefile, server);
        }
        
        namSvrCacheRead(server, cachefile);
        dbdata = ajTableFetchS(namSvrDatabaseTable, server);
        
        ajStrDel(&cachefile);
        
        if(!dbdata)
        {
            ajWarn("Server '%S' has no databases", server);
            return;
        }
    }
    
    dbtable = (AjPTable) dbdata->data;
    
    ajTableToarrayValues(dbtable, &valarray);
    ajDebug("ajNamSvrListListDatabases\n");
    
    for(i = 0; valarray[i]; i++)
    {
	fnew = (NamPEntry) valarray[i];
	ajDebug("DB: %S\n", fnew->name);
	ajListstrPushAppend(dbnames, fnew->name);
    }
    
    AJFREE(valarray);
    
    return;
}




/* @funcstatic namSvrCacheOpen ************************************************
**
** Opens the database cachefile for a server.
**
** Searches emboss_standard and the user directory ~/.embossdata/
** testing the file data in each file found and opening the most recent file.
**
** cache files must all start with a comment line
** containing the file name and the creation date.
**
** @param [r] cachename [const AjPStr] Cache file name
** @return [AjPFile] Filename of selected cache file
**
**
** @release 6.4.0
******************************************************************************/

static AjPFile namSvrCacheOpen(const AjPStr cachename)
{
    AjPFile ret = NULL;
    AjPFile userfile = NULL;
    AjPFile systemfile = NULL;

    AjPStr readline = NULL;
    AjPStrTok handle = NULL;
    AjPStr token = NULL;
    AjPTime usertime = NULL;
    AjPTime systemtime = NULL;

    systemfile = ajFileNewInNamePathS(cachename, namStandardDir);
    ajDebug("system file '%F' name: '%S' path: '%S'\n",
            systemfile, cachename, namStandardDir);

    if(systemfile)
    {
        ajReadlineTrim(systemfile, &readline);
        ajStrTokenAssignC(&handle, readline, "# \t");
        ajStrTokenNextParse(&handle, &token);

        if(!ajStrMatchS(token, cachename))
            ajWarn("%F: Cache file name '%S' expected, '%S' found",
                   systemfile, cachename, token);

        ajStrTokenNextParseC(&handle, "", &token); /* time */
        systemtime = ajTimeNew();
        ajTimeSetS(systemtime, token);
    }

    userfile = ajFileNewInNamePathS(cachename, namUserDir);
    ajDebug("user file '%F' name: '%S' path: '%S'\n",
            userfile, cachename, namStandardDir);

    if(userfile)
    {
        ajReadlineTrim(userfile, &readline);
        ajStrTokenAssignC(&handle, readline, "# \t");
        ajStrTokenNextParse(&handle, &token);

        if(!ajStrMatchS(token, cachename))
            ajWarn("%F: Cache file name '%S' expected, '%S' found",
                   userfile, cachename, token);

        ajStrTokenNextParseC(&handle, "", &token); /* time */
        usertime = ajTimeNew();
        ajTimeSetS(usertime, token);
    }

    if(userfile && (!systemfile || ajTimeDiff(systemtime, usertime) > 0.0))
    {
        ajFileClose(&systemfile);
        ret = userfile;
        ajDebug("user file '%F' %D selected\n", userfile, usertime);
    }
    else
    {
        ajFileClose(&userfile);
        ret = systemfile;
        ajDebug("system file '%F' %D selected\n", systemfile, systemtime);
    }

    ajStrTokenDel(&handle);
    ajStrDel(&readline);
    ajStrDel(&token);
    ajTimeDel(&systemtime);
    ajTimeDel(&usertime);

    return ret;
}




/* @funcstatic namSvrCacheRead ************************************************
**
** Reads the database cachefile for a server
**
** Searches emboss_standard and the user directory ~/.embossdata/
** testing the file data in each file found and parsing the most recent file.
**
** cache files must all start with a comment line
** containing the file name and the creation date.
**
** @param [r] server [const AjPStr] Server name
** @param [r] cachename [const AjPStr] Cache file name
** @return [AjBool] True on success
**
**
** @release 6.4.0
******************************************************************************/

static AjBool namSvrCacheRead(const AjPStr server, const AjPStr cachename)
{
    AjPFile cachefile;
    AjPTable dbtable;
    AjPTable alitable;
    NamPEntry fnew = NULL;

    cachefile = namSvrCacheOpen(cachename);
    if(!cachefile)
        return ajFalse;

    dbtable = ajTablecharNewCase(30);
    alitable = ajTablecharNewCase(30);

    namSvrCacheParse(cachefile, dbtable, alitable);
    ajFileClose(&cachefile);

    AJNEW0(fnew);
    fnew->name = ajStrNewS(server);
    fnew->value = ajStrNewS(cachename);
    fnew->data = dbtable;
    ajTablePut(namSvrDatabaseTable, ajCharNewS(server), fnew);
    fnew = NULL;

    AJNEW0(fnew);
    fnew->name = ajStrNewS(server);
    fnew->value = ajStrNewS(cachename);
    fnew->data = alitable;
    ajTablePut(namSvrAliasTable, ajCharNewS(server), fnew);
    fnew = NULL;


    return ajTrue;
}




/* @funcstatic namProcessFile *************************************************
**
** Read the definitions file and append each token to the list.
**
** @param [u] file [AjPFile] Input file object
** @param [r] shortname [const AjPStr] Definitions file short name
** @return [AjBool] ajTrue if no error were found
**
** @release 1.0.0
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
	iline++;
	AJNEW0(k);
	*k = (ajuint) ajListGetLength(listwords);
	ajListPushAppend(listcount, k);
	
        /* Ignore if the line is a comment */
	if(!ajStrCutCommentsStart(&rdline))
	    continue;
	
	/* namUser("%S\n",rdline); */
	len = ajStrGetLen(rdline);
	
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
    *k = (ajuint) ajListGetLength(listwords);
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
    namDebugAliases();
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
**
** @release 1.0.0
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
    NamPEntry entryAuto  = NULL;

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
    AjPStr tmpstr        = NULL;
#endif

    if(namVarMasterTable &&
       namAliasMasterTable &&
       namSvrMasterTable &&
       namDbMasterTable &&
       namResMasterTable)
	return;

#ifdef HAVE_MCHECK
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
#endif /* ! HAVE_MCHECK */

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

    namVarMasterTable = ajTablecharNewCase(100);
    namAliasMasterTable = ajTablecharNewCase(100);
    namSvrMasterTable = ajTablecharNewCase(100);
    namDbMasterTable = ajTablecharNewCase(100);
    namResMasterTable = ajTablecharNewCase(100);
    namSvrDatabaseTable = ajTablecharNewCase(100);
    namSvrAliasTable = ajTablecharNewCase(100);

#ifdef HAVE_AXIS2C
    AJNEW0(entryAuto);
    entryAuto->name = ajStrNewC("emboss_axis2c");
    entryAuto->value = ajStrNewC("1");
    entryAuto->file = ajStrNewC("auto");
    ajTablePut(namVarMasterTable, ajCharNewC("emboss_axis2c"),
               entryAuto);
    entryAuto = NULL;
#endif

#if defined(HAVE_MYSQL) || defined(HAVE_POSTGRESQL)
    AJNEW0(entryAuto);
    entryAuto->name = ajStrNewC("emboss_sql");
    entryAuto->value = ajStrNewC("1");
    entryAuto->file = ajStrNewC("auto");
    ajTablePut(namVarMasterTable, ajCharNewC("emboss_sql"),
               entryAuto);
    entryAuto = NULL;

#ifdef HAVE_MYSQL
    AJNEW0(entryAuto);
    entryAuto->name = ajStrNewC("emboss_mysql");
    entryAuto->value = ajStrNewC("1");
    entryAuto->file = ajStrNewC("auto");
    ajTablePut(namVarMasterTable, ajCharNewC("emboss_mysql"),
               entryAuto);
    entryAuto = NULL;
#endif
#ifdef HAVE_POSTGRESQL
    AJNEW0(entryAuto);
    entryAuto->name = ajStrNewC("emboss_postgresql");
    entryAuto->value = ajStrNewC("1");
    entryAuto->file = ajStrNewC("auto");
    ajTablePut(namVarMasterTable, ajCharNewC("emboss_postgresql"),
               entryAuto);
    entryAuto = NULL;
#endif
#endif

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
        /* namInstallRoot undefined for Windows ... use required EMBOSS_ROOT */

        ajStrAssignS(&tmpstr, prefixRootStr);
	ajStrAppendC(&tmpstr, "\\apps\\Release");

        if(ajFilenameExistsDir(tmpstr))
            strcpy(namInstallRoot, ajStrGetPtr(tmpstr));
        else
            strcpy(namInstallRoot,prefixRoot);

        ajStrDel(&tmpstr);
#endif
    }
    else
	prefixRoot = namFixedRoot;
    
    /* namFixedBaseStr is the directory above the source root */
    
    ajStrAssignC(&namFixedRootStr, prefixRoot);
    ajStrAssignS(&namFixedBaseStr, namFixedRootStr);
#ifndef WIN32
    ajDirnameUp(&namFixedBaseStr);
#endif

    ajStrAssignC(&namFixedPackageStr, namPackage);
    ajStrAssignC(&namFixedSystemStr, namSystem);
    ajStrAssignC(&namFixedVersionStr, namVersion);
    if(ajStrGetLen(namFixedVersionStr) < 7)
        ajStrAppendC(&namFixedVersionStr, ".0");

    ajStrAssignC(&namFixedInstallStr, namInstallRoot);

    if(is_windows)
    {
        ajFmtPrintS(&namStandardDir, "%s",
                    namInstallRoot);
    }
    else
    {
        ajFmtPrintS(&namStandardDir, "%s%sshare%s%S",
                    namInstallRoot, SLASH_STRING, SLASH_STRING, prefixCap);
    }
    ajFmtPrintS(&namStandardFilename, "%S%s%s.standard",
                namStandardDir, SLASH_STRING, prefix);
    prefixRootFile = ajFileNewInNameS(namStandardFilename);
    ajStrAssignC(&basename, "standard");

    if(!prefixRootFile)
    {
	/* try original directory */
        ajStrAssignC(&namStandardDir, prefixRoot);
	    ajFmtPrintS(&namStandardFilename, "%s%s%s.standard", prefixRoot,
			SLASH_STRING,prefix);

	prefixRootFile = ajFileNewInNameS(namStandardFilename);
    }

    if(prefixRootFile)
    {
        AJNEW0(entryAuto);
        entryAuto->name = ajStrNewC("emboss_standard");
        entryAuto->value = ajStrNewS(namStandardDir);
        entryAuto->file = ajStrNewRef(namStandardFilename);
        ajTablePut(namVarMasterTable, ajCharNewC("emboss_standard"),
                   entryAuto);
        entryAuto = NULL;

	ajStrAppendC(&namFileOrig, "(OK)");
	namProcessFile(prefixRootFile, basename);
	ajFileClose(&prefixRootFile);

        if(namFileOrig)
            ajStrAppendC(&namFileOrig, ", ");

        ajStrAppendS(&namFileOrig, namStandardFilename);
    }
    else
	ajStrAppendC(&namFileOrig, "(failed)");

    /*
    ** EMBOSS is distributed with an emboss.standard file
    ** Looks for this file and set EMBOSS_STANDARD variable to its directory
    */

    /*
    ** look for the site's default file in the install directory as
       <install-prefix>/share/PREFIX/emboss.default
    **
    */
    
    ajFmtPrintS(&namRootStr, "%s%sshare%s%S%s%s.default",
		 namInstallRoot, SLASH_STRING, SLASH_STRING,
                prefixCap, SLASH_STRING, prefix);
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
    
    prefixRoot = ajSysGetHomedir();
    
    ajStrAssignC(&namUserDir, prefixRoot);
    ajStrAppendC(&namUserDir, SLASH_STRING);
    ajStrAppendC(&namUserDir, ".embossdata");
    ajStrAppendC(&namUserDir, SLASH_STRING);
    
    ajStrAssignC(&prefixStr, prefix);
    ajStrAppendC(&prefixStr, "_RCHOME");
    ajStrFmtUpper(&prefixStr);
    
    if(ajNamGetenvS(prefixStr, &homercVal))
	ajStrToBool(homercVal, &namDoHomeRc);

    ajStrDel(&homercVal);
    
    if(namDoHomeRc && prefixRoot)
    {
	ajStrAssignC(&namRootStr, prefixRoot);
	ajStrAppendC(&namRootStr, SLASH_STRING);
	ajStrAppendC(&namRootStr, ".");
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

    if(!namFixedSystemStr)
        namFixedSystemStr = ajStrNewC(namSystem);
    
    if(!namFixedVersionStr)
    {
        namFixedVersionStr = ajStrNewC(namVersion);
        if(ajStrGetLen(namFixedVersionStr) < 7)
            ajStrAppendC(&namFixedVersionStr, ".0");
    }

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
**
** @release 1.0.0
** @@
******************************************************************************/

static void namNoColon(AjPStr* thys)
{
    if(ajStrGetCharLast(*thys) == ':')
	ajStrCutEnd(thys, 1);

    return;
}




/* @funcstatic namDbAttrC *****************************************************
**
** Return the index for a database attribute name.
**
** @param [r] str [const char*] Attribute name.
** @return [ajint] Index in namDbAttrs, or -1 on failure.
**
** @release 1.0.0
** @@
******************************************************************************/

static ajint namDbAttrC(const char* str)
{
    ajint i = 0;
    ajint j = 0;
    ajint ifound = 0;

    ajStrAssignC(&namCmpStr, str);
    ajStrFmtLower(&namCmpStr);

    for(i=0; namDbAttrs[i].Name; i++)
    {
	if(ajStrMatchC(namCmpStr, namDbAttrs[i].Name))
	    return i;

	if(ajCharPrefixS(namDbAttrs[i].Name, namCmpStr))
	{
	    ifound++;
	    j = i;
	}
    }

    if(ifound == 1)
	return j;

    return -1;
}




/* @funcstatic namDbAttrS *****************************************************
**
** Return the index for a database attribute name.
**
** @param [r] thys [const AjPStr] Attribute name.
** @return [ajint] Index in namDbAttrs, or -1 on failure.
**
** @release 6.4.0
** @@
******************************************************************************/

static ajint namDbAttrS(const AjPStr thys)
{
    return namDbAttrC(ajStrGetPtr(thys));
}




/* @funcstatic namSvrAttrC ****************************************************
**
** Return the index for a server attribute name.
**
** @param [r] str [const char*] Attribute name.
** @return [ajint] Index in namSvrAttrs, or -1 on failure.
**
** @release 6.4.0
** @@
******************************************************************************/

static ajint namSvrAttrC(const char* str)
{
    ajint i = 0;
    ajint j = 0;
    ajint ifound = 0;

    ajStrAssignC(&namCmpStr, str);
    ajStrFmtLower(&namCmpStr);

    for(i=0; namSvrAttrs[i].Name; i++)
    {
	if(ajStrMatchC(namCmpStr, namSvrAttrs[i].Name))
	    return i;

	if(ajCharPrefixS(namSvrAttrs[i].Name, namCmpStr))
	{
	    ifound++;
	    j = i;
	}
    }

    if(ifound == 1)
	return j;

    return -1;
}




/* @funcstatic namSvrAttrS ****************************************************
**
** Return the index for a server attribute name.
**
** @param [r] thys [const AjPStr] Attribute name.
** @return [ajint] Index in namSvrAttrs, or -1 on failure.
**
** @release 6.4.0
** @@
******************************************************************************/

static ajint namSvrAttrS(const AjPStr thys)
{
    return namSvrAttrC(ajStrGetPtr(thys));
}




/* @funcstatic namRsAttrC *****************************************************
**
** Return the index for a resource attribute name.
**
** @param [r] str [const char*] Attribute name.
** @return [ajint] Index in namRsAttrs, or -1 on failure.
**
** @release 2.7.0
** @@
******************************************************************************/

static ajint namRsAttrC(const char* str)
{
    ajint i = 0;
    ajint j = 0;
    ajint ifound = 0;

    ajStrAssignC(&namCmpStr, str);
    ajStrFmtLower(&namCmpStr);


    for(i=0; namRsAttrs[i].Name; i++)
    {
	if(ajStrMatchC(namCmpStr, namRsAttrs[i].Name))
	    return i;

	if(ajCharPrefixS(namRsAttrs[i].Name, namCmpStr))
	{
	    ifound++;
	    j = i;
	}
    }

    if(ifound == 1)
	return j;

    return -1;
}




/* @funcstatic namRsAttrS *****************************************************
**
** Return the index for a resource attribute name.
**
** @param [r] thys [const AjPStr] Attribute name.
** @return [ajint] Index in namRsAttrs, or -1 on failure.
**
** @release 6.4.0
** @@
******************************************************************************/

static ajint namRsAttrS(const AjPStr thys)
{
    return namRsAttrC(ajStrGetPtr(thys));
}




/* @funcstatic namRsAttrFieldC ************************************************
**
** Return the validity of a resource attribute name using a known field prefix
**
** @param [r] rstable [const AjPTable] Resource table
** @param [r] str [const char*] Attribute name.
** @return [AjBool] Attribute name matches a defined field and property
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool namRsAttrFieldC(const AjPTable rstable, const char* str)
{
    AjBool ret = ajFalse;
    AjPStr prefix = NULL;
    const AjPStr fields = NULL;
    AjPStrTok handle = NULL;
    AjPStr field = NULL;
    const AjPStr type = NULL;
    const AjPList fdlist = NULL;
    AjIList iter = NULL;
    const NamPAttr attr = NULL;

    type = ajTableFetchmodC(rstable, "type");

    if(!ajStrMatchC(type, "Index"))
        return ajFalse;

    if(ajCharSuffixC(str, "len"))
        ajStrAssignSubC(&prefix, str, 0, -4);
    else if(ajCharSuffixC(str, "seccachesize"))
        ajStrAssignSubC(&prefix, str, 0, -13);
    else if(ajCharSuffixC(str, "cachesize"))
        ajStrAssignSubC(&prefix, str, 0, -10);
    else if(ajCharSuffixC(str, "secpagesize"))
        ajStrAssignSubC(&prefix, str, 0, -12);
    else if(ajCharSuffixC(str, "pagesize"))
        ajStrAssignSubC(&prefix, str, 0, -9);
    else
        return ajFalse;

    if(ajStrGetLen(prefix))
    {
        fields = ajTableFetchmodC(rstable, "fields");

        if(fields)
        {
            handle = ajStrTokenNewC(fields, " ,;");

            while(ajStrTokenNextParse(&handle, &field))
                if(ajStrMatchS(prefix, field))
                    ret = ajTrue;
        }
        else
        {
            fdlist = ajTableFetchC(rstable, "field");

            if(fdlist)
            {
                iter = ajListIterNewread(fdlist);

                while(!ajListIterDone(iter))
                {
                    fields = ajListIterGet(iter);
                    ajStrTokenAssignC(&handle, fields, " ,;");

                    while(ajStrTokenNextParse(&handle, &field))
                    {
                        if(ajStrMatchC(field, "!"))
                            break;

                        if(ajStrMatchS(prefix, field))
                            ret = ajTrue;
                    }
                }

                ajListIterDel(&iter);
            }
            else 
            {
                attr = ajTableFetchC(namResAttrTable, "fields");
                handle = ajStrTokenNewcharC(attr->Defval, " ,;");

                while(ajStrTokenNextParse(&handle, &field))
                    if(ajStrMatchS(prefix, field))
                        ret = ajTrue;
            }
        }

        ajStrTokenDel(&handle);
        ajStrDel(&field);
    }

    ajStrDel(&prefix);

    return ret;
}




/* @funcstatic namRsAttrFieldS ************************************************
**
** Return the validity of a resource attribute name using a known field prefix
**
** @param [r] rstable [const AjPTable] Resource table
** @param [r] str [const AjPStr] Attribute name.
** @return [AjBool] Attribute name matches a defined field and property
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool namRsAttrFieldS(const AjPTable rstable, const AjPStr str)
{
    AjBool ret = ajFalse;
    AjPStr prefix = NULL;
    const AjPStr fields = NULL;
    AjPStrTok handle = NULL;
    AjPStr field = NULL;
    const AjPStr type = NULL;
    AjPList fdlist = NULL;
    AjIList iter = NULL;
    const NamPAttr attr = NULL;

    type = ajTableFetchC(rstable, "type");

    if(!ajStrMatchC(type, "Index"))
        return ajFalse;

    if(ajStrSuffixC(str, "len"))
        ajStrAssignSubS(&prefix, str, 0, -4);
    else if(ajStrSuffixC(str, "seccachesize"))
        ajStrAssignSubS(&prefix, str, 0, -13);
    else if(ajStrSuffixC(str, "cachesize"))
        ajStrAssignSubS(&prefix, str, 0, -10);
    else if(ajStrSuffixC(str, "secpagesize"))
        ajStrAssignSubS(&prefix, str, 0, -12);
    else if(ajStrSuffixC(str, "pagesize"))
        ajStrAssignSubS(&prefix, str, 0, -9);
    else
        return ajFalse;

    if(ajStrGetLen(prefix))
    {
        fields = ajTableFetchC(rstable, "fields");

        if(fields)
        {
            handle = ajStrTokenNewC(fields, " ,;");

            while(ajStrTokenNextParse(&handle, &field))
                if(ajStrMatchS(prefix, field))
                    ret = ajTrue;
        }
        else
        {
            fdlist = ajTableFetchmodC(rstable, "field");

            if(fdlist)
            {
                iter = ajListIterNewread(fdlist);

                while(!ajListIterDone(iter))
                {
                    fields = ajListIterGet(iter);
                    ajStrTokenAssignC(&handle, fields, " ,;");

                    while(ajStrTokenNextParse(&handle, &field))
                    {
                        if(ajStrMatchC(field, "!"))
                            break;

                        if(ajStrMatchS(prefix, field))
                            ret = ajTrue;
                    }
                }

                ajListIterDel(&iter);
            }
            else
            {
                attr = ajTableFetchC(namResAttrTable, "fields");

                if(attr)
                {
                    handle = ajStrTokenNewcharC(attr->Defval, " ,;");

                    while(ajStrTokenNextParse(&handle, &field))
                        if(ajStrMatchS(prefix, field))
                            ret = ajTrue;
                }
                
            }
            
        }
        


        ajStrTokenDel(&handle);
        ajStrDel(&field);
    }

    ajStrDel(&prefix);

    return ret;
}




/* @func ajNamExit ************************************************************
**
** Delete the initialisation values in the table.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajNamExit(void)
{
    namListMasterDelete(namVarMasterTable, TYPE_ENV); /* Delete elements */
    ajTableFree(&namVarMasterTable); /* free table and database structures */
    namListMasterDelete(namAliasMasterTable, TYPE_ALIAS); /* Delete elements */
    ajTableFree(&namAliasMasterTable); /* free table and database structures */
    namListMasterDelete(namSvrMasterTable, TYPE_SVR); /* Delete elements */
    ajTableFree(&namSvrMasterTable); /* free table and database structures */
    namListMasterDelete(namDbMasterTable, TYPE_DB); /* Delete elements */
    ajTableFree(&namDbMasterTable); /* free table and database structures */
    namListMasterDelete(namResMasterTable, TYPE_RESOURCE);/* Delete elements */
    ajTableFree(&namResMasterTable); /* free table and database structures */
    namListMasterDelete(namSvrDatabaseTable, TYPE_SVRDB);/* Delete elements */
    ajTableFree(&namSvrDatabaseTable); /* free table and database structures */
    namListMasterDelete(namSvrAliasTable, TYPE_SVRALI);/* Delete elements */
    ajTableFree(&namSvrAliasTable); /* free table and database structures */

    ajTablestrFreeKey(&namDbTypeTable); /* free table and strings */

    ajTablestrFree(&namSvrAttrTable); /* free table and strings */
    ajTablestrFree(&namDbAttrTable); /* free table and strings */
    ajTablestrFree(&namResAttrTable); /* free table and strings */

    ajStrDel(&namFixedBaseStr);    /* allocated in ajNamInit */
    ajStrDel(&namFixedRootStr);    /* allocated in ajNamInit */
    ajStrDel(&namFixedInstallStr); /* allocated in ajNamInit */
    ajStrDel(&namFixedPackageStr); /* allocated in ajNamInit */
    ajStrDel(&namFixedSystemStr);  /* allocated in ajNamInit */
    ajStrDel(&namFixedVersionStr); /* allocated in ajNamInit */
    ajStrDel(&namPrefixStr);       /* allocated in ajNamInit */
    ajStrDel(&namFileOrig);        /* allocated in ajNamInit */
    ajStrDel(&namRootStr);         /* allocated in ajNamInit */
    
    ajStrDel(&namFileName);		/* allocated in ajNamProcessFile */
    ajStrDel(&namValNameTmp);
    ajStrDel(&namCmpStr);
    ajStrDel(&namStandardDir);
    ajStrDel(&namStandardFilename);
    ajStrDel(&namUserDir);
    ajStrDel(&namUserFilename);

    ajRegFree(&namNameExp);
    ajRegFree(&namVarExp);

    ajListFree(&namIfList);

    ajDebug("ajNamExit done\n");

    return;
}




/* @func ajNamSvrTest *********************************************************
**
** Looks for a server name in the known definitions.
**
** @param [r] svrname [const AjPStr] Server name.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajNamSvrTest(const AjPStr svrname)
{
    const NamPEntry data;

    data = ajTableFetchS(namSvrMasterTable, svrname);

    if(!data)
	return ajFalse;

    return ajTrue;
}




/* @func ajNamSvrGetUrl *******************************************************
**
** Gets the URL definition for a server definition.
**
** @param [r] svrname [const AjPStr] Server name.
** @param [w] url [AjPStr*] URL returned.
** @return [AjBool] ajTrue if success.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajNamSvrGetUrl(const AjPStr svrname, AjPStr* url)
{

    const NamPEntry data;
    AjPTable svrtable;
    const AjPStr svrval = NULL;

    data = ajTableFetchS(namSvrMasterTable, svrname);

    if(!data)
	ajFatal("%S is not a known server", svrname);

    svrtable = (AjPTable) data->data;

    svrval = ajTableFetchC(svrtable, "url");

    if(ajStrGetLen(svrval))
    {
	ajStrAssignS(url, svrval);

	return ajTrue;
    }

    return ajFalse;
}




/* @func ajNamQueryGetDatatypeC ***********************************************
**
** Gets the datatype matching a query, and updates the datatype in the query
**
** @param [r] qry [const AjPQuery] Database query
** @return [const char*] Database type
**
** @release 6.4.0
** @@
******************************************************************************/

const char* ajNamQueryGetDatatypeC(const AjPQuery qry)
{
    ajuint i;
    const char* ret = NULL;

    if(ajStrGetLen(qry->DbType))
        return MAJSTRGETPTR(qry->DbType);

    for(i=0; namDbTypes[i].Name; i++)
        if(namDbTypes[i].DataType == qry->DataType)
            ret = namDbTypes[i].Name;

    return ret;
}




/* @func ajNamQueryGetUrl *****************************************************
**
** Gets the URL definition for a server definition.
**
** @param [r] qry [const AjPQuery] Database query
** @return [const AjPStr] URL value
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPStr ajNamQueryGetUrl(const AjPQuery qry)
{
    const AjPStr ret = NULL;
    const AjPStr svrval = NULL;
    const AjPTable dbtable;

    dbtable = namQuerySvrdata(qry);

    svrval = ajTableFetchC(dbtable, "url");

    if(svrval)
        ret = svrval;

    dbtable = namQueryDbdata(qry);

    svrval = ajTableFetchC(dbtable, "url");

    if(svrval)
        ret = svrval;

    return ret;
}




/* @funcstatic namQuerySvrdata ************************************************
**
** Gets the URL definition for a server definition.
**
** @param [r] qry [const AjPQuery] Query.
** @return [const AjPTable] DatabaseServer attributes table
**
** @release 6.4.0
** @@
******************************************************************************/

static const AjPTable namQuerySvrdata(const AjPQuery qry)
{
    const NamPEntry data;

    if(!ajStrGetLen(qry->SvrName))
        return NULL;

    data = ajTableFetchS(namSvrMasterTable, qry->SvrName);

    if(!data)
	ajFatal("%S is not a known server", qry->SvrName);

    return (const AjPTable) data->data;

}




/* @funcstatic namQueryDbdata *************************************************
**
** Gets the data table for a query database.
**
** @param [r] qry [const AjPQuery] Database query
** @return [const AjPTable] Database attributes table
**
** @release 6.4.0
** @@
******************************************************************************/

static const AjPTable namQueryDbdata(const AjPQuery qry)
{
    const NamPEntry data;
    const NamPEntry sdata;
    const AjPTable sdbtable;

    if(ajStrGetLen(qry->SvrName))
    {
        sdata = ajTableFetchS(namSvrDatabaseTable, qry->SvrName);
        sdbtable = (const AjPTable) sdata->data;
        data = ajTableFetchS(sdbtable, qry->DbName);

        if(!data)
            ajFatal("%S is not a known database for server %S",
                    qry->DbName, qry->SvrName);
    }
    else
    {
        data = ajTableFetchS(namDbMasterTable, qry->DbName);

        if(!data)
            ajFatal("%S is not a known database", qry->DbName);
    }

    return (const AjPTable) data->data;
}




/* @func ajNamSvrGetDbalias ***************************************************
**
** Gets alias names for a server's database.
**
** @param [r] svrname [const AjPStr] Server name.
** @param [w] dbalias [AjPStr*] Alias returned.
** @return [AjBool] ajTrue if success.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajNamSvrGetDbalias(const AjPStr svrname, AjPStr* dbalias)
{
    const NamPEntry data;
    const AjPTable svrtable;
    const AjPStr svrval = NULL;

    data = ajTableFetchS(namSvrMasterTable, svrname);

    if(!data)
	ajFatal("%S is not a known server", svrname);

    svrtable = (AjPTable) data->data;

    svrval = ajTableFetchC(svrtable, "dbalias");

    if(ajStrGetLen(svrval))
	ajStrAssignS(dbalias, svrval);
    else
    {
        ajStrAssignS(dbalias, svrname);
        ajStrFmtLower(dbalias);
    }

    return ajTrue;
}




/* @func ajNamSvrData *********************************************************
**
** Given a query with server name, database name and search fields,
** fill in the common fields. The query fields are set later.
**
** This part of the database definition is required (specifically
** the "fields" definition) for setting the query details.
**
** See also ajNamSvrQuery, which calls this function if the common
** query data is not yet set.
**
** @param [u] qry [AjPQuery] Query structure with at least
**                                    dbname filled in
** @param [r] argc [ajuint] Number of additional attribute name/value pairs
** @param [v] [...] Variable length argument list
** @return [AjBool] ajTrue if success.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajNamSvrData(AjPQuery qry, ajuint argc, ...)
{

    const NamPEntry data;
    const NamPEntry dbdata;

    const AjPTable svrtable;
    const AjPTable sdbtable;
    const AjPTable dbtable;
    AjPStrTok handle = NULL;
    AjPStr liststr = NULL;
    AjPStr token = NULL;
    ajuint i;
    const char* varAttrName;
    AjPStr* varAttrValue;
    va_list ap;
    const NamPType namtype;
    AjBool ok = ajTrue;
    AjEDataType testDataType = AJDATATYPE_UNKNOWN;
    AjPStr saveType = NULL;
    AjBool donecaseid = ajFalse;
    AjBool donehasacc = ajFalse;
    AjBool caseid = ajFalse;
    AjBool hasacc = ajFalse;

    if(qry->SetServer)
        return ajTrue;

    qry->SetServer = ajTrue;

    data = ajTableFetchS(namSvrMasterTable, qry->SvrName);

    if(!data)
	ajFatal("server %S unknown", qry->SvrName);

    svrtable = (const AjPTable) data->data;

    /* general defaults */

    namSvrSetAttrStrC(svrtable, "type", &liststr);
    ajStrTokenAssignC(&handle, liststr, " ,;");
    ajStrAssignC(&qry->DbType, "");

    ajDebug("Server type: '%S'\n", liststr);

    while(ajStrTokenNextParse(&handle, &token))
    {
        if(!ajStrSuffixCaseC(token,"features"))
        {
            if(ajStrPrefixCaseC(token, "n"))
        	ajStrAssignC(&token, "Nucleotide");
            else if(ajStrPrefixCaseC(token, "p"))
        	ajStrAssignC(&token, "Protein");
        }

        namtype = ajTableFetchS(namDbTypeTable, token);

        ajDebug("testing type '%S' namtype: '%s' \n",
		token, namtype->Name);

        if(!namtype)
        {
            ajErr("Server %S type '%S' unknown", qry->SvrName, token);
            continue;
        }

        ajDebug("test server type '%S' qry datatype %u nam "
                "datatype %u unk %u\n",
                token, qry->DataType, namtype->DataType, AJDATATYPE_UNKNOWN);

        if(qry->DataType == AJDATATYPE_UNKNOWN) /* take first available */
        {
            if(testDataType == AJDATATYPE_UNKNOWN ||
               namtype->DataType == AJDATATYPE_TEXT)
            {
                testDataType = namtype->DataType;
                ajStrAssignS(&saveType, token);
                ajDebug("Save type %d '%S'\n", testDataType, saveType);
            }
        }

        if(namtype->DataType == qry->DataType)
        {
            ajStrAssignS(&qry->DbType, token);
            break;
        }
    }
    
    if(ajStrGetLen(qry->Formatstr))
    {
        if(ajStrGetLen(qry->DbType) &&
           !ajStrMatchC(qry->DbType, "unknown") &&
           !namInformatTest(qry->Formatstr, qry->DbType))
        {
            ajErr("Query format '%S' does not support datatype '%S'",
                  qry->Formatstr, qry->DbType);
            ajStrDel(&saveType);
            return ajFalse;
        }
    }
    else
    {
        if(namSvrSetAttrStrC(svrtable, "format", &liststr))
        {
            ok = ajFalse;
            ajStrTokenAssignC(&handle, liststr, " ,;");

            while(ajStrTokenNextParse(&handle, &token))
            {
                if(namInformatTest(token, qry->DbType))
                {
                    ok = ajTrue;
                    ajStrAssignS(&qry->Formatstr, token);
                }
                if(saveType && namInformatTest(token, saveType))
                {
                    ok = ajTrue;
                    ajStrAssignS(&qry->Formatstr, token);
                }
            }

            if(!ok)
            {
                ajErr("Server %S format(s) '%S' do not support "
                      "datatype '%S'",
                      qry->SvrName, liststr, qry->DbType);
                ajStrDel(&saveType);
                return ajFalse;
            }
        }
    }
    
    namSvrSetAttrStrC(svrtable, "method", &qry->Method);
    namSvrSetAttrStrC(svrtable, "indexdirectory", &qry->IndexDir);
    namSvrSetAttrStrC(svrtable, "indexdirectory", &qry->Directory);
    namSvrSetAttrStrC(svrtable, "field", &qry->DbFields);
    namSvrSetAttrStrC(svrtable, "url", &qry->DbUrl);
    namSvrSetAttrStrC(svrtable, "proxy", &qry->DbProxy);
    namSvrSetAttrStrC(svrtable, "httpversion", &qry->DbHttpVer);
    namSvrSetAttrStrC(svrtable, "serverversion", &qry->ServerVer);
    if(namSvrSetAttrBoolC(svrtable, "caseidmatch", &caseid))
    {
        donecaseid = ajTrue;
        qry->CaseId = caseid;
    }
    if(namSvrSetAttrBoolC(svrtable, "hasaccession", &hasacc))
    {
        donehasacc = ajTrue;
        qry->HasAcc = hasacc;
    }

#ifdef WIN32
    ajStrExchangeKK(&qry->Directory, '/', '\\');
    if(ajStrPrefixC(qry->Directory, ".\\"))
        ajStrCutStart(&qry->Directory, 2);
    ajStrExchangeKK(&qry->IndexDir, '/', '\\');
    if(ajStrPrefixC(qry->IndexDir, ".\\"))
        ajStrCutStart(&qry->IndexDir, 2);
#endif

    if(argc)
    {
        va_start(ap, argc);

        for(i=0; i < argc; i++)
        {
            varAttrName = va_arg(ap, const char*);
            varAttrValue = va_arg(ap, AjPStr*);
            namSvrSetAttrStrC(svrtable, varAttrName, varAttrValue);
        }

        va_end(ap);
    }

    ajDebug("ajNamSvrQuery SvrName '%S'\n", qry->SvrName);
    ajDebug("    DbName      '%S'\n", qry->DbName);
    ajDebug("    DbType      '%S'\n", qry->DbType);
    ajDebug("    Method      '%S'\n", qry->Method);
    ajDebug("    Formatstr   '%S'\n", qry->Formatstr);
    ajDebug("    IndexDir    '%S'\n", qry->IndexDir);
    ajDebug("    Directory   '%S'\n", qry->Directory);

    if(ajStrGetLen(qry->DbName))
    {
        data = ajTableFetchS(namSvrDatabaseTable, qry->SvrName);

        if(!data)
        {
            ajWarn("server %S has no databases", qry->SvrName);
            ajStrDel(&saveType);
            return ajFalse;
        }
    
        sdbtable = (const AjPTable) data->data;
        dbdata = ajTableFetchS(sdbtable, qry->DbName);

        if(!dbdata)
            ajFatal("server %S has no database %S", qry->SvrName, qry->DbName);

        dbtable = (const AjPTable) dbdata->data;

        /* general defaults */

        if(namDbSetAttrStrC(dbtable, "type", &liststr))
        {
            ajDebug("db type '%S'\n", liststr);

            ajStrTokenAssignC(&handle, liststr, " ,;");
            ajStrAssignC(&qry->DbType, "");

            while(ajStrTokenNextParse(&handle, &token))
            {
                if(!ajStrSuffixCaseC(token,"features"))
                {
                    if(ajStrPrefixCaseC(token, "n"))
                        ajStrAssignC(&token, "Nucleotide");
                    else if(ajStrPrefixCaseC(token, "p"))
                        ajStrAssignC(&token, "Protein");
                }

                ajDebug("test token '%S' list '%S'\n", token, liststr);

                namtype = ajTableFetchS(namDbTypeTable, token);

                if(!namtype)
                {
                    ajErr("Database %S type '%S' unknown", qry->DbName, token);
                    continue;
                }

                ajDebug("qry->DataType %u namtype->DataType %u type '%s'\n",
                        qry->DataType, namtype->DataType, namtype->Name);

                if(qry->DataType == AJDATATYPE_UNKNOWN) /* take first type */
                {
                    if(testDataType == AJDATATYPE_UNKNOWN ||
                       namtype->DataType == AJDATATYPE_TEXT)
                    {
                        testDataType = namtype->DataType;
                        ajStrAssignS(&saveType, token);
                        ajDebug("Save type %d '%S'\n", testDataType, saveType);
                    }
                }

                if(namtype->DataType == qry->DataType)
                {
                    ajStrAssignS(&qry->DbType, token);
                    break;
                }
            }

        }

        if(qry->DataType == AJDATATYPE_UNKNOWN)
        {
            qry->DataType = testDataType;
            ajStrAssignS(&qry->DbType, saveType);
        }

        if(!ajStrGetLen(qry->DbType))
        {
            ajErr("Server %S database '%S' type(s) '%S' "
                  "no match to query type '%s'",
                  qry->SvrName, qry->DbName,
                  liststr, ajQueryGetDatatype(qry));
            ajStrDel(&saveType);
            return ajFalse;
        }

        if(ajStrGetLen(qry->Formatstr))
        {
            if(ajStrGetLen(qry->DbType) &&
               !ajStrMatchC(qry->DbType, "unknown") &&
               !namInformatTest(qry->Formatstr, qry->DbType))
            {
                ajErr("Query format '%S' does not support datatype '%S'",
                      qry->Formatstr, qry->DbType);
                ajStrDel(&saveType);
                return ajFalse;
            }
        }
        else if (ajStrGetLen(qry->DbType))
        {
            if(namDbSetAttrStrC(dbtable, "format", &liststr))
            {
                ok = ajFalse;
                ajStrTokenAssignC(&handle, liststr, " ,;");

                while(ajStrTokenNextParse(&handle, &token))
                {
                    if(namInformatTest(token, qry->DbType))
                    {
                        ok = ajTrue;
                        ajStrAssignS(&qry->Formatstr, token);
                        break;
                    }
                }

                if(!ok)
                {
                    ajErr("Server %S database %S format(s) '%S' "
			  "do not support datatype '%S'",
                          qry->SvrName, qry->DbName, liststr, qry->DbType);
                    ajStrDel(&saveType);
                    return ajFalse;
                }
            }
        }

        namDbSetAttrStrC(dbtable, "method", &qry->Method);
        namDbSetAttrStrC(dbtable, "app", &qry->Application);
        namDbSetAttrStrC(dbtable, "directory", &qry->IndexDir);
        namDbSetAttrStrC(dbtable, "indexdirectory", &qry->IndexDir);
        namDbSetAttrStrC(dbtable, "indexdirectory", &qry->Directory);
        namDbSetAttrStrC(dbtable, "directory", &qry->Directory);
        namDbSetAttrStrC(dbtable, "namespace", &qry->Namespace);
        namDbSetAttrStrC(dbtable, "organisms", &qry->Organisms);
        namDbSetAttrStrC(dbtable, "exclude", &qry->Exclude);
        namDbSetAttrStrC(dbtable, "filename", &qry->Filename);
        if(!namDbSetAttrStrC(dbtable, "field", &qry->DbFields))
            namDbSetAttrStrC(dbtable, "fields", &qry->DbFields);
        namDbSetAttrStrC(dbtable, "url", &qry->DbUrl);
        namDbSetAttrStrC(dbtable, "proxy", &qry->DbProxy);
        namDbSetAttrStrC(dbtable, "httpversion", &qry->DbHttpVer);
        namDbSetAttrStrC(dbtable, "serverversion", &qry->ServerVer);
        if(namDbSetAttrBoolC(dbtable, "caseidmatch", &caseid))
        {
            donecaseid = ajTrue;
            qry->CaseId = caseid;
        }
        if(namDbSetAttrBoolC(dbtable, "hasaccession", &hasacc))
        {
            donehasacc = ajTrue;
            qry->HasAcc = hasacc;
        }

        namDbSetAttrStrC(dbtable, "identifier", &qry->DbIdentifier);
        namDbSetAttrStrC(dbtable, "accession", &qry->DbAccession);
        namDbSetAttrStrC(dbtable, "filters", &qry->DbFilter);
        namDbSetAttrStrC(dbtable, "return", &qry->DbReturn);

        if(!donecaseid)
            qry->CaseId = caseid;
        if(!donehasacc)
            qry->HasAcc = hasacc;

#ifdef WIN32
	ajStrExchangeKK(&qry->Directory, '/', '\\');
	if(ajStrPrefixC(qry->Directory, ".\\"))
	  ajStrCutStart(&qry->Directory, 2);
	ajStrExchangeKK(&qry->IndexDir, '/', '\\');
	if(ajStrPrefixC(qry->IndexDir, ".\\"))
	  ajStrCutStart(&qry->IndexDir, 2);
#endif

        if(argc)
        {
            va_start(ap, argc);

            for(i=0; i < argc; i++)
            {
                varAttrName = va_arg(ap, const char*);
                varAttrValue = va_arg(ap, AjPStr*);
                namDbSetAttrStrC(dbtable, varAttrName, varAttrValue);
            }

            va_end(ap);
        }
    }

    ajDebug("ajNamSvrData '%S:%S' method:%S type:%S format:%S\n",
            qry->SvrName, qry->DbName, qry->Method, qry->DbType,
            qry->Formatstr);

    ajStrDel(&liststr);
    ajStrDel(&token);

    ajStrTokenDel(&handle);
    ajStrDel(&saveType);

    return ajTrue;
}




/* @func ajNamSvrQuery ********************************************************
**
** Given a query with server name and search fields,
** fill in the access method and some common fields according
** to the query level.
**
** @param [u] qry [AjPQuery] Query structure with at least
**                                    svrname filled in
** @return [AjBool] ajTrue if success.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajNamSvrQuery(AjPQuery qry)
{
    const NamPEntry data;
    const AjPTable svrtable;

    data = ajTableFetchS(namSvrMasterTable, qry->SvrName);

    if(!data)
	ajFatal("server %S unknown", qry->SvrName);

    svrtable = (const AjPTable) data->data;

    if(!ajStrGetLen(qry->DbType))
	ajNamSvrData(qry, 0);

    if((qry->QueryType == AJQUERY_ALL) ||
       ((qry->QueryType == AJQUERY_UNKNOWN) && !ajQueryIsSet(qry)))
    {
        /* must have a method for all entries */
	namSvrSetAttrStrC(svrtable, "methodall", &qry->Method);
	namSvrSetAttrStrC(svrtable, "formatall", &qry->Formatstr);
	namSvrSetAttrStrC(svrtable, "appall", &qry->Application);
	qry->QueryType = AJQUERY_ALL;
    }
    else		      /* must be able to query the database */
    {
	namSvrSetAttrStrC(svrtable, "methodquery", &qry->Method);
	namSvrSetAttrStrC(svrtable, "formatquery", &qry->Formatstr);
	namSvrSetAttrStrC(svrtable, "appquery", &qry->Application);

	if((qry->QueryType != AJQUERY_QUERY) &&
           !ajQuerySetWild(qry)) /* ID - single entry may be available */
	{
	    namSvrSetAttrStrC(svrtable, "methodentry", &qry->Method);
	    namSvrSetAttrStrC(svrtable, "formatentry", &qry->Formatstr);
	    namSvrSetAttrStrC(svrtable, "appentry", &qry->Application);
	    qry->QueryType = AJQUERY_ENTRY;
	}
	else
	    qry->QueryType = AJQUERY_QUERY;
    }


    if(!ajStrGetLen(qry->Method))
    {
	ajErr("No access method for server '%S'",
              qry->SvrName);

	return ajFalse;
    }

    return ajTrue;
}




/* @funcstatic namSvrSetAttrStrC **********************************************
**
** Sets a named string attribute value from an attribute list.
**
** @param [r] svrtable [const AjPTable] Attribute definitions table.
** @param [r] attrib [const char*] Attribute name.
** @param [w] qrystr [AjPStr*] Returned attribute value.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool namSvrSetAttrStrC(const AjPTable svrtable, const char* attrib,
                                AjPStr* qrystr)
{
    const void* svrval = NULL;
    const AjPStr strval = NULL;
    const NamPAttr attr = NULL;
    const AjPList fdlist = NULL;
    AjIList iter = NULL;
    AjPStrTok handle = NULL;
    AjPStr word = NULL;

    attr = ajTableFetchC(namSvrAttrTable, attrib);

    if(!attr)
        ajFatal("unknown attribute '%s' requested",  attrib);

    svrval = ajTableFetchC(svrtable, attrib);

    if(svrval)
    {
        switch(attr->Type)
        {
            case ATTR_LIST:
                ajStrAssignC(qrystr, "");
                fdlist = (const AjPList) svrval;
                iter = ajListIterNewread(fdlist);

                while(!ajListIterDone(iter))
                {
                    strval = ajListIterGet(iter);
                    ajStrTokenAssignC(&handle, strval, " ");

                    while(ajStrTokenNextParse(&handle, &word))
                    {
                        if(ajStrMatchC(word, "!"))
                            break;

                        if(ajStrGetLen(*qrystr))
                            ajStrAppendK(qrystr, ' ');

                        ajStrAppendS(qrystr, word);
                    }
                }

                ajStrTokenDel(&handle);
                ajStrDel(&word);
                ajListIterDel(&iter);
                break;

            default:
                if(!ajStrGetLen((const AjPStr) svrval))
                    return ajFalse;

                ajStrAssignS(qrystr, (const AjPStr) svrval);
                break;
        }
    }
    else 
    {
        if(!strlen(attr->Defval))
            return ajFalse;

        ajStrAssignC(qrystr, attr->Defval);
    }

    namVarResolve(qrystr);

    return ajTrue;
}




/* @funcstatic namSvrSetAttrBoolC *********************************************
**
** Sets a named boolean attribute value from an attribute list.
**
** @param [r] svrtable [const AjPTable] Attribute definitions table
** @param [r] attrib [const char*] Attribute name.
** @param [w] qrybool [AjBool*] Returned attribute value.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool namSvrSetAttrBoolC(const AjPTable svrtable, const char* attrib,
                                 AjBool* qrybool)
{
    AjBool ret = ajTrue;
    AjPStr tmpstr = NULL;
    const AjPStr svrval = NULL;
    const char* txtval = NULL;
    const NamPAttr attrval = NULL;

    svrval = ajTableFetchC(svrtable, attrib);

    if(svrval)
        txtval = ajStrGetPtr(svrval);
    else 
    {
        attrval = ajTableFetchC(namSvrAttrTable, attrib);

        if(!attrval)
            ajFatal("unknown attribute '%s' requested",  attrib);

        txtval = attrval->Defval;
    }

    if(ajStrGetLen(svrval))
    {
	ajStrAssignS(&tmpstr, svrval);
	ret = ajTrue;
    }
    else
    {
	ajStrAssignC(&tmpstr, txtval);
	ret = ajFalse;
    }

    /* ajDebug("namSvrSetAttr('%S')\n", *qrystr); */

    namVarResolve(&tmpstr);
    ajStrToBool(tmpstr, qrybool);
    ajStrDel(&tmpstr);

    return ret;
}




/* @func ajNamDbTest **********************************************************
**
** Looks for a database name in the known definitions.
**
** @param [r] dbname [const AjPStr] Database name.
** @return [AjBool] ajTrue on success.
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajNamDbTest(const AjPStr dbname)
{
    const NamPEntry data;

    data = ajTableFetchS(namDbMasterTable, dbname);

    if(!data)
	return ajFalse;

    return ajTrue;
}




/* @func ajNamDbGetType *******************************************************
**
** Gets the enumerated type for a database definition.
**
** @param [r] dbname [const AjPStr] Database name.
** @param [w] itype [ajuint*] Database enumerated type
** @return [AjBool] ajTrue if success.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajNamDbGetType(const AjPStr dbname, ajuint *itype)
{
    AjBool ret = ajFalse;
    const NamPEntry data;
    const AjPTable dbtable;
    const AjPStr dbval = NULL;
    AjPStrTok handle = NULL;
    AjPStr token = NULL;
    const NamPType namtype;

    data = ajTableFetchS(namDbMasterTable, dbname);

    if(!data)
	ajFatal("%S is not a known database", dbname);

    dbtable = (const AjPTable) data->data;

    dbval = ajTableFetchC(dbtable, "type");
    ajStrTokenAssignC(&handle, dbval, " ,;");

    while(ajStrTokenNextParse(&handle, &token))
    {
        if(!ajStrSuffixCaseC(token,"features"))
        {
            if(ajStrPrefixCaseC(token, "n"))
        	ajStrAssignC(&token, "Nucleotide");
            else if(ajStrPrefixCaseC(token, "p"))
        	ajStrAssignC(&token, "Protein");
        }

        namtype = ajTableFetchS(namDbTypeTable, token);
        ajDebug("testing type '%S' namtype: '%s' \n",
		token, namtype->Name);

        *itype =  namtype->DataType;
        ret = ajTrue;
        break;
    }

    ajStrTokenDel(&handle);
    ajStrDel(&token);
    
    return ret;
}




/* @func ajNamDbGetUrl ********************************************************
**
** Gets the URL definition for a database definition.
**
** @param [r] dbname [const AjPStr] Database name.
** @param [w] url [AjPStr*] URL returned.
** @return [AjBool] ajTrue if success.
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajNamDbGetUrl(const AjPStr dbname, AjPStr* url)
{

    const NamPEntry data;
    const AjPTable dbtable;
    const AjPStr dbval = NULL;

    data = ajTableFetchS(namDbMasterTable, dbname);

    if(!data)
	ajFatal("%S is not a known database", dbname);

    dbtable = (const AjPTable) data->data;

    dbval = ajTableFetchC(dbtable, "url");

    if(ajStrGetLen(dbval))
    {
	ajStrAssignS(url, dbval);

	return ajTrue;
    }

    if(ajStrGetLen(*url))
        return ajTrue;

    return ajFalse;
}




/* @func ajNamDbGetDbalias ****************************************************
**
** Gets an alias name for a database.
**
** @param [r] dbname [const AjPStr] Database name.
** @param [w] dbalias [AjPStr*] Alias returned.
** @return [AjBool] ajTrue if success.
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajNamDbGetDbalias(const AjPStr dbname, AjPStr* dbalias)
{

    const NamPEntry data;
    const AjPTable dbtable;
    const AjPStr dbval = NULL;

    data = ajTableFetchS(namDbMasterTable, dbname);

    if(!data)
	ajFatal("%S is not a known database", dbname);

    dbtable = (const AjPTable) data->data;

    dbval = ajTableFetchC(dbtable, "dbalias");

    if(ajStrGetLen(dbval))
	ajStrAssignS(dbalias, dbval);
    else
    {
        ajStrAssignS(dbalias, dbname);
        ajStrFmtLower(dbalias);
    }

    return ajTrue;
}




/* @func ajNamDbGetIndexdir ***************************************************
**
** Gets the index directory for a database.
**
** @param [r] dbname [const AjPStr] Database name.
** @param [w] indexdir [AjPStr*] Directory path returned.
** @return [AjBool] ajTrue if success.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajNamDbGetIndexdir(const AjPStr dbname, AjPStr* indexdir)
{

   const  NamPEntry data;
    const AjPTable dbtable;
    const AjPStr dbval = NULL;

    data = ajTableFetchS(namDbMasterTable, dbname);

    if(!data)
	ajFatal("%S is not a known database", dbname);

    dbtable = (const AjPTable) data->data;

    dbval = ajTableFetchC(dbtable, "indexdirectory");

    if(ajStrGetLen(dbval))
    {
	ajStrAssignS(indexdir, dbval);
        return ajTrue;
    }

    dbval = ajTableFetchC(dbtable, "directory");

    if(ajStrGetLen(dbval))
    {
	ajStrAssignS(indexdir, dbval);
        return ajTrue;
    }

    return ajFalse;
}




/* @func ajNamQuerySetDbalias *************************************************
**
** Sets an alias name for a database query.
**
** @param [u] qry [AjPQuery] Database query.
** @return [AjBool] ajTrue if success.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajNamQuerySetDbalias(AjPQuery qry)
{
    const NamPEntry data;
    const NamPEntry sdata;
    const AjPTable dbtable;
    const AjPTable sdbtable;
    const AjPStr dbval = NULL;

    if(ajStrGetLen(qry->SvrName))
    {
	if(!ajStrGetLen(qry->DbType))
            ajNamSvrData(qry, 0);

        ajNamSvrQuery(qry);
        sdata = ajTableFetchS(namSvrDatabaseTable, qry->SvrName);
        sdbtable = (const AjPTable) sdata->data;
        data = ajTableFetchS(sdbtable, qry->DbName);

        if(!data)
            ajFatal("%S is not a known database for server %S",
                    qry->DbName, qry->SvrName);
    }
    else
    {
        data = ajTableFetchS(namDbMasterTable, qry->DbName);

        if(!data)
            ajFatal("%S is not a known database", qry->DbName);
    }

    dbtable = (const AjPTable) data->data;

    dbval = ajTableFetchC(dbtable, "dbalias");

    if(ajStrGetLen(dbval))
	ajStrAssignS(&qry->DbAlias, dbval);
    else
    {
        ajStrAssignS(&qry->DbAlias, qry->DbName);
        ajStrFmtLower(&qry->DbAlias);
    }

    return ajTrue;
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
** @param [u] qry [AjPQuery] Query structure with at least
**                                    dbname filled in
** @param [r] argc [ajuint] Number of additional attribute name/value pairs
** @param [v] [...] Variable length argument list
** @return [AjBool] ajTrue if success.
**
** @release 2.4.0
** @@
******************************************************************************/

AjBool ajNamDbData(AjPQuery qry, ajuint argc, ...)
{
    const NamPEntry data;
    const AjPTable dbtable;
    AjPStrTok handle = NULL;
    AjPStr liststr = NULL;
    AjPStr token = NULL;
    ajuint i;
    const char* varAttrName;
    AjPStr* varAttrValue;
    va_list ap;
    const NamPType namtype;
    AjBool ok = ajTrue;

    if(qry->SetDatabase)
        return ajTrue;

    qry->SetDatabase = ajTrue;

    data = ajTableFetchS(namDbMasterTable, qry->DbName);

    if(!data)
    {
	ajDebug("ajNamDbData: database %S unknown\n", qry->DbName);
        return ajFalse;
    }

    dbtable = (const AjPTable) data->data;

    /* general defaults */

    if(namDbSetAttrStrC(dbtable, "type", &liststr))
    {
        ajStrTokenAssignC(&handle, liststr, " ,;");
        ajStrAssignC(&qry->DbType, "");

        while(ajStrTokenNextParse(&handle, &token))
        {
            if(ajStrMatchCaseC(token, "n"))
                ajStrAssignC(&token, "Nucleotide");
            else if(ajStrMatchCaseC(token, "p"))
                ajStrAssignC(&token, "Protein");
            else if(ajStrMatchCaseC(token, "prot"))
                ajStrAssignC(&token, "Protein");

            namtype = ajTableFetchS(namDbTypeTable, token);

            if(!namtype)
            {
                ajErr("Database %S type '%S' unknown", qry->DbName, token);
                continue;
            }

            if(qry->DataType == AJDATATYPE_UNKNOWN) /* take first available */
                qry->DataType = namtype->DataType;

            if(namtype->DataType == qry->DataType)
            {
                ajStrAssignS(&qry->DbType, token);
                break;
            }
        }

        if(!ajStrGetLen(qry->DbType))
        {
            ajErr("Database %S type(s) '%S' do not match query type '%s'",
                  qry->DbName, liststr, ajQueryGetDatatype(qry));
            return ajFalse;
        }
    }

    if(ajStrGetLen(qry->Formatstr))
    {
        if(!namInformatTest(qry->Formatstr, qry->DbType))
        {
            ajErr("Query format '%S' does not support datatype '%S'",
                  qry->Formatstr, qry->DbType);
            return ajFalse;
        }
    }
    else
    {
        if(namDbSetAttrStrC(dbtable, "format", &liststr))
        {
            ok = ajFalse;
            ajStrTokenAssignC(&handle, liststr, " ,;");

            while(ajStrTokenNextParse(&handle, &token))
            {
                if(namInformatTest(token, qry->DbType))
                {
                    ok = ajTrue;
                    ajStrAssignS(&qry->Formatstr, token);
                }
            }

            if(!ok)
            {
                ajErr("Database %S format(s) '%S' do not support "
                      "datatype '%S'",
                      qry->DbName, liststr, qry->DbType);
                return ajFalse;
            }
        }
    }

    ajStrDel(&liststr);
    ajStrDel(&token);
    ajStrTokenDel(&handle);

    namDbSetAttrStrC(dbtable, "method", &qry->Method);
    namDbSetAttrStrC(dbtable, "app", &qry->Application);
    namDbSetAttrStrC(dbtable, "directory", &qry->IndexDir);
    namDbSetAttrStrC(dbtable, "indexdirectory", &qry->IndexDir);
    namDbSetAttrStrC(dbtable, "indexdirectory", &qry->Directory);
    namDbSetAttrStrC(dbtable, "directory", &qry->Directory);
    namDbSetAttrStrC(dbtable, "namespace", &qry->Namespace);
    namDbSetAttrStrC(dbtable, "organisms", &qry->Organisms);
    namDbSetAttrStrC(dbtable, "exclude", &qry->Exclude);
    namDbSetAttrStrC(dbtable, "filename", &qry->Filename);

    if(!namDbSetAttrStrC(dbtable, "field", &qry->DbFields))
        namDbSetAttrStrC(dbtable, "fields", &qry->DbFields);

    namDbSetAttrStrC(dbtable, "url", &qry->DbUrl);
    namDbSetAttrStrC(dbtable, "proxy", &qry->DbProxy);
    namDbSetAttrStrC(dbtable, "httpversion", &qry->DbHttpVer);
    namDbSetAttrStrC(dbtable, "serverversion", &qry->ServerVer);
    namDbSetAttrBoolC(dbtable, "caseidmatch", &qry->CaseId);
    namDbSetAttrBoolC(dbtable, "hasaccession", &qry->HasAcc);

    namDbSetAttrStrC(dbtable, "identifier", &qry->DbIdentifier);
    namDbSetAttrStrC(dbtable, "accession", &qry->DbAccession);
    namDbSetAttrStrC(dbtable, "filters", &qry->DbFilter);
    namDbSetAttrStrC(dbtable, "return", &qry->DbReturn);

#ifdef WIN32
    ajStrExchangeKK(&qry->Directory, '/', '\\');
    if(ajStrPrefixC(qry->Directory, ".\\"))
        ajStrCutStart(&qry->Directory, 2);
    ajStrExchangeKK(&qry->IndexDir, '/', '\\');
    if(ajStrPrefixC(qry->IndexDir, ".\\"))
        ajStrCutStart(&qry->IndexDir, 2);
#endif


    if(argc)
    {
        va_start(ap, argc);

        for(i=0; i < argc; i++)
        {
            varAttrName = va_arg(ap, const char*);
            varAttrValue = va_arg(ap, AjPStr*);
            namDbSetAttrStrC(dbtable, varAttrName, varAttrValue);
        }

        va_end(ap);
    }

    return ajTrue;
}




/* @func ajNamDbQuery *********************************************************
**
** Given a query with database name and search fields,
** fill in the access method and some common fields according
** to the query level.
**
** @param [u] qry [AjPQuery] Query structure with at least
**                                    dbname filled in
** @return [AjBool] ajTrue if success.
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajNamDbQuery(AjPQuery qry)
{

    const NamPEntry data;
    const NamPEntry sdata;

    const AjPTable sdbtable;

    const AjPTable dbtable;
    const char* qlinks;

    ajDebug("ajNamDbQuery SetQuery:%B QueryType:%d\n",
            qry->SetQuery, qry->QueryType);

    if(qry->SetQuery)
        return ajTrue;

    qry->SetQuery = ajTrue;

    if(qry->InDrcat)
        return ajTrue;

    if(ajStrGetLen(qry->SvrName))
    {
        if(ajStrGetLen(qry->DbType))
            ajNamSvrData(qry, 0);

        ajNamSvrQuery(qry);
        sdata = ajTableFetchS(namSvrDatabaseTable, qry->SvrName);
        sdbtable = (const AjPTable) sdata->data;
        data = ajTableFetchS(sdbtable, qry->DbName);
    }
    else
        data = ajTableFetchS(namDbMasterTable, qry->DbName);

    if(!data)
        ajFatal("database %S unknown", qry->DbName);

    dbtable = (const AjPTable) data->data;

    if(!ajStrGetLen(qry->DbType))
    {
	if(!ajNamDbData(qry, 0))
            return ajFalse;
    }

    if((qry->QueryType == AJQUERY_ALL) ||
       ((qry->QueryType == AJQUERY_UNKNOWN) && !ajQueryIsSet(qry))) 
    {
        /* must have a method for all entries */
	namDbSetAttrStrC(dbtable, "methodall", &qry->Method);
	namDbSetAttrStrC(dbtable, "formatall", &qry->Formatstr);
	namDbSetAttrStrC(dbtable, "appall", &qry->Application);
	qry->QueryType = AJQUERY_ALL;
    }
    else		      /* must be able to query the database */
    {
	namDbSetAttrStrC(dbtable, "methodquery", &qry->Method);
	namDbSetAttrStrC(dbtable, "formatquery", &qry->Formatstr);
	namDbSetAttrStrC(dbtable, "appquery", &qry->Application);

	if((qry->QueryType != AJQUERY_QUERY) &&
           !ajQuerySetWild(qry)) /* ID - single entry may be available */
	{
	    namDbSetAttrStrC(dbtable, "methodentry", &qry->Method);
	    namDbSetAttrStrC(dbtable, "formatentry", &qry->Formatstr);
	    namDbSetAttrStrC(dbtable, "appentry", &qry->Application);
	    qry->QueryType = AJQUERY_ENTRY;
	}
	else
	    qry->QueryType = AJQUERY_QUERY;
    }

    namDbSetAttrStrC(dbtable, "return", &qry->DbReturn);


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

    qlinks = namMethod2Qlinks(qry->Method, qry->DataType);

    if(!qlinks)
    {
        ajErr("Unknown access method '%S' for database '%S'",
              qry->Method, qry->DbName);
        return ajFalse;
    }

    ajStrAssignC(&qry->Qlinks, qlinks);

    return ajTrue;
}




/* @func ajNamFileQuery *******************************************************
**
** Given a query with no database name and search fields, check
** datatype specific fields according to the query level.
**
** @param [u] qry [AjPQuery] Query structure with no database name
** @return [AjBool] ajTrue if success.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajNamFileQuery(AjPQuery qry)
{
    const char* fields = NULL;
    const char* qlinks = NULL;

    if(qry->SetQuery)
        return ajTrue;

    qry->SetQuery = ajTrue;

    if(ajStrGetLen(qry->SvrName))
	ajFatal("ajNamFileQuery called with server '%S'", qry->SvrName);
    else if(ajStrGetLen(qry->DbName))
	ajFatal("ajNamFileQuery called with database '%S'", qry->DbName);

    switch(qry->QueryType)
    {
        case AJQUERY_ALL:
            break;

        case AJQUERY_QUERY:
            break;

        case AJQUERY_ENTRY:
            break;
            
        case AJQUERY_UNKNOWN:
        default:
            break;
    }

    /*
    if(!ajStrGetLen(qry->Formatstr))
    {
	ajErr("No format defined for file access query");

	return ajFalse;
    }
    */

    qlinks = namDatatype2Qlinks(qry->DataType);

    if(qlinks)
        ajStrAssignC(&qry->Qlinks, qlinks);
    else
    {
        ajWarn("no query links defined for type; '%s'",
               namDbTypes[qry->DataType].Name);
        ajStrAssignC(&qry->Qlinks, "|");
    }

    fields = namDatatype2Fields(qry->DataType);

    if(fields)
        ajStrAssignC(&qry->DbFields, fields);
    else
    {
        ajWarn("no fields defined for type; '%s'",
               namDbTypes[qry->DataType].Name);
        ajStrAssignC(&qry->DbFields, "");
    }

    return ajTrue;
}




/* @funcstatic namDbSetAttrStrC ***********************************************
**
** Sets a named string attribute value from an attribute list.
**
** @param [r] dbtable [const AjPTable] Attribute definitions table.
** @param [r] attrib [const char*] Attribute name.
** @param [w] qrystr [AjPStr*] Returned attribute value.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool namDbSetAttrStrC(const AjPTable dbtable, const char* attrib,
                               AjPStr* qrystr)
{
    const void* dbval = NULL;
    const AjPStr strval = NULL;
    const NamPAttr attr = NULL;
    const AjPList fdlist = NULL;
    AjIList iter = NULL;
    AjPStrTok handle = NULL;
    AjPStr word = NULL;

    attr = ajTableFetchC(namDbAttrTable, attrib);

    if(!attr)
        ajFatal("unknown attribute '%s' requested",  attrib);

    dbval = ajTableFetchC(dbtable, attrib);

    if(dbval)
    {
        switch(attr->Type)
        {
            case ATTR_LIST:
                ajStrAssignC(qrystr, "");
                fdlist = (const AjPList) dbval;
                iter = ajListIterNewread(fdlist);

                while(!ajListIterDone(iter))
                {
                    strval = ajListIterGet(iter);
                    ajStrTokenAssignC(&handle, strval, " ");

                    while(ajStrTokenNextParse(&handle, &word))
                    {
                        if(ajStrMatchC(word, "!"))
                            break;

                        if(ajStrGetLen(*qrystr))
                            ajStrAppendK(qrystr, ' ');

                        ajStrAppendS(qrystr, word);
                    }
                }

                ajStrTokenDel(&handle);
                ajStrDel(&word);
                ajListIterDel(&iter);
                break;

            default:
                if(!ajStrGetLen((const AjPStr) dbval))
                    return ajFalse;
                ajStrAssignS(qrystr, (const AjPStr) dbval);

                break;
        }
    }
    else 
    {
        if(!strlen(attr->Defval))
            return ajFalse;

        ajStrAssignC(qrystr, attr->Defval);
    }

    namVarResolve(qrystr);

    return ajTrue;
}




/* @funcstatic namDbSetAttrBoolC **********************************************
**
** Sets a named boolean attribute value from an attribute list.
**
** @param [r] dbtable [const AjPTable] Attribute definitions table
** @param [r] attrib [const char*] Attribute name.
** @param [w] qrybool [AjBool*] Returned attribute value.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool namDbSetAttrBoolC(const AjPTable dbtable, const char* attrib,
                                AjBool* qrybool)
{
    AjBool ret = ajTrue;
    AjPStr tmpstr = NULL;
    const AjPStr dbval = NULL;
    const char* txtval = NULL;
    const NamPAttr attr = NULL;

    dbval = ajTableFetchC(dbtable, attrib);

    if(dbval)
        txtval = ajStrGetPtr(dbval);
    else 
    {
        attr = ajTableFetchC(namDbAttrTable, attrib);

        if(!attr)
            ajFatal("unknown attribute '%s' requested",  attrib);

        txtval = attr->Defval;
    }

    if(!ajStrGetLen(dbval))
    {
	ajStrAssignC(&tmpstr, txtval);
	ret = ajFalse;
    }
    else
    {
	ajStrAssignS(&tmpstr, dbval);
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
**
** @param [u] var [AjPStr*] String value
** @return [AjBool] Always ajTrue so far
**
** @release 1.0.0
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
**
** @release 1.0.0
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
**
** @release 2.7.0
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
**
** @release 6.0.0
** @@
******************************************************************************/

const AjPStr ajNamValueInstalldir(void)
{
    return namFixedInstallStr;
}




/* @func ajNamValuePackage ****************************************************
**
** Returns the package name for the library
**
** @return [const AjPStr] Package name
**
** @release 6.0.0
** @@
******************************************************************************/

const AjPStr ajNamValuePackage(void)
{
    return namFixedPackageStr;
}




/* @func ajNamValueSystem *****************************************************
**
** Returns the system information for the library
**
** @return [const AjPStr] Version number
**
** @release 6.2.0
** @@
******************************************************************************/

const AjPStr ajNamValueSystem(void)
{
    return namFixedSystemStr;
}




/* @func ajNamValueVersion ****************************************************
**
** Returns the version number for the library
**
** @return [const AjPStr] Version number
**
** @release 6.0.0
** @@
******************************************************************************/

const AjPStr ajNamValueVersion(void)
{
    return namFixedVersionStr;
}




/* @func ajNamValueRootdir ****************************************************
**
** Returns the directory for all file searches
** (package level)
**
** @return [const AjPStr] Package level root directory
**
** @release 6.0.0
** @@
******************************************************************************/

const AjPStr ajNamValueRootdir(void)
{
    return namFixedRootStr;
}




/* @func ajNamValueBasedir ****************************************************
**
** Returns the base directory for all for all file searches
** (above package level).
**
** @return [const AjPStr] Base directory
**
** @release 6.0.0
******************************************************************************/

const AjPStr ajNamValueBasedir(void)
{
    return namFixedBaseStr;
}




/* @func ajNamResolve *********************************************************
**
** Resolves a variable name if the input string starts with a dollar sign.
**
** @param [w] name [AjPStr*] String
** @return [AjBool] ajTrue on success.
**
** @release 1.0.0
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
	    ajStrAssignS(name, restname);
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
**
** @release 2.7.0
** @@
******************************************************************************/

static AjBool namValid(const NamPEntry entry, ajint entrytype)
{
    if(entrytype == TYPE_ENV)
	return namValidVariable(entry);
    else if(entrytype == TYPE_SVR)
	return namValidServer(entry);
    else if(entrytype == TYPE_DB)
	return namValidDatabase(entry);
    else if(entrytype == TYPE_RESOURCE)
	return namValidResource(entry);
    else if(entrytype == TYPE_ALIAS)
	return namValidAlias(entry);

    /* fatal: cannot test - should not happen */
    namError("Unknown definition type number %d",
	      entrytype);

    return ajFalse;
}




/* @funcstatic namValidAlias **************************************************
**
** Validation of a master table alias entry
**
** @param [r] entry [const NamPEntry] Internal table entry
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool namValidAlias(const NamPEntry entry)
{
    AjPTable attrtable;

    attrtable = (AjPTable) entry->data;

    if(attrtable)
    {			 /* strange - should be nothing for aliases */
	namError("Alias '%S' has a list of attributes",
		  entry->name);

	return ajFalse;
    }
    
    return ajTrue;
}




/* @funcstatic namValidServer *************************************************
**
** Validation of a master table server entry
**
** @param [r] entry [const NamPEntry] Internal table entry
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool namValidServer(const NamPEntry entry)
{
    ajint iattr = 0;
    ajint j;
    ajint k;
    AjBool ok;
    AjBool oktype;
    AjPTable attrtable;
    AjBool hasmethod = ajFalse;
    AjBool hastype   = ajFalse;
    AjPStr name = NULL;
    AjPStrTok handle = NULL;
    AjPStr token = NULL;
    const AjPStr value = NULL;
    const AjPStr dbtype = NULL;

    attrtable = (AjPTable) entry->data;

    if(!attrtable)
    {			 /* fatal - should be set for all servers */
	namError("Server '%S' has no list of valid attributes",
		  entry->name);
	return ajFalse;
    }
    
    dbtype = ajTableFetchC(attrtable, "type");

    for(j=0; namSvrAttrs[j].Name; j++)
    {
        if(namSvrAttrs[j].Type != ATTR_STR)
            continue;

        ajStrAssignC(&name, namSvrAttrs[j].Name);
        value = ajTableFetchS(attrtable, name);

	if(value)
	{
	    iattr++;

	    if(ajStrPrefixC(name, "format"))
            {
                if(!namInformatTest(value, dbtype))
                        namError("Database '%S' %S: '%S' unknown",
                                 entry->name, name, value);
            }

            if(ajStrPrefixC(name, "method"))
            {
        	handle = NULL;
        	token = ajStrNew();
        	hasmethod=ajTrue;

        	ajStrTokenAssignC(&handle, dbtype, " ,;");

        	while(ajStrTokenNextParse(&handle, &token))
        	    if(!namAccessTest(value, token))
        		namError("Server '%S' %S: '%S' unknown",
        		         entry->name, name, value);

        	ajStrTokenDel(&handle);
        	ajStrDel(&token);
            }

	    if(ajStrPrefixC(name, "type"))
	    {
		hastype=ajTrue;
		oktype = ajFalse;

		for(k=0; namDbTypes[k].Name; k++)
		    if(ajStrFindAnyC(value, namDbTypes[k].Name) != -1)
			oktype = ajTrue;

		if(!oktype)
		    namError("Server '%S' %S: '%S' unknown",
			      entry->name, name, value);
	    }
	}
    }

    ok = ajTrue;

    if(!iattr)
    {
	namError("Server '%S' has no attributes", entry->name);
	ok = ajFalse;
    }

    if(!hastype)
    {
	namError("Database '%S' has no type definition", entry->name);
	ok = ajFalse;
    }

    if(!hasmethod)
    {
	namError("Database '%S' has no access method definition",
		  entry->name);
	ok = ajFalse;
    }

    ajStrDel(&name);

    return ok;
}




/* @funcstatic namValidDatabase ***********************************************
**
** Validation of a master table database entry
**
** @param [r] entry [const NamPEntry] Internal table entry
** @return [AjBool] ajTrue on success
**
** @release 2.7.0
** @@
******************************************************************************/

static AjBool namValidDatabase(const NamPEntry entry)
{
    ajint iattr = 0;
    ajint j;
    ajint k;
    AjBool ok;
    AjBool oktype;
    AjPTable attrtable;
    AjBool hasformat = ajFalse;
    AjBool hasmethod = ajFalse;
    AjBool hastype   = ajFalse;
    AjPStr name = NULL;
    AjPStr token = NULL;
    AjPStr typetoken = NULL;
    AjPStrTok handle = NULL;
    AjPStrTok typehandle = NULL;
    const AjPStr value = NULL;
    const AjPStr dbtype = NULL;

    attrtable = (AjPTable) entry->data;

    if(!attrtable)
    {			 /* fatal - should be set for all databases */
	namError("Database '%S' has no list of valid attributes",
		  entry->name);
	return ajFalse;
    }
    
    dbtype = ajTableFetchC(attrtable, "type");

    for(j=0; namDbAttrs[j].Name; j++)
    {
        if(namDbAttrs[j].Type != ATTR_STR)
            continue;

        ajStrAssignC(&name, namDbAttrs[j].Name);
        value = ajTableFetchS(attrtable, name);

	if(value)
	{
	    iattr++;

	    if(ajStrPrefixC(name, "format"))
	    {
		hasformat=ajTrue;

                ajStrTokenAssignC(&typehandle, dbtype, " ,;");

                while(ajStrTokenNextParse(&typehandle, &typetoken))
                {
                    ok = ajFalse;
                    ajStrTokenAssignC(&handle, value, " ,;");
                    while(ajStrTokenNextParse(&handle, &token))
                    {
                        if(namInformatTest(token, typetoken))
                            ok = ajTrue;
                    }

                    if(!ok) /* test: dbunknowns.rc */
                        namError("Database '%S' %S: '%S' unknown for type '%S'",
                                 entry->name, name, value, typetoken);
                }

            }

            if(ajStrPrefixC(name, "method"))
	    {
		hasmethod=ajTrue;

                ajStrTokenAssignC(&typehandle, dbtype, " ,;");
 
                while(ajStrTokenNextParse(&typehandle, &typetoken))
                {
                    ok = ajFalse;
                    ajStrTokenAssignC(&handle, value, " ,;");
                    while(ajStrTokenNextParse(&handle, &token))
                    {
                        if(namAccessTest(value, typetoken))
                            ok = ajTrue;
                    }

                    if(!ok)  /* test: dbunknowns.rc */
                        namError("Database '%S' %S: '%S' unknown for type '%S'",
                                 entry->name, name, value, typetoken);
                }
            }

	    if(ajStrPrefixC(name, "type"))
	    {
		hastype=ajTrue;
		oktype = ajFalse;

                ajStrTokenAssignC(&typehandle, value, " ,;");
                while(ajStrTokenNextParse(&typehandle, &typetoken))
                {
                    for(k=0; namDbTypes[k].Name; k++)
                        if(ajStrMatchCaseC(typetoken, namDbTypes[k].Name)) 
                            oktype = ajTrue;
                }

		if(!oktype)		/* test: dbunknowns.rc */
		    namError("Database '%S' %S: '%S' unknown",
			      entry->name, name, value);
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

    ajStrDel(&name);
    ajStrTokenDel(&handle);
    ajStrTokenDel(&typehandle);
    ajStrDel(&token);
    ajStrDel(&typetoken);

    return ok;
}




/* @funcstatic namValidResource ***********************************************
**
** Validation of a master table resource entry
**
** @param [r] entry [const NamPEntry] Internal table entry
** @return [AjBool] ajTrue on success
**
** @release 2.7.0
** @@
******************************************************************************/

static AjBool namValidResource(const NamPEntry entry)
{
    ajint iattr = 0;
    ajint j;
    AjPTable rstable;
    AjBool ok;

    rstable = (AjPTable) entry->data;

    if(!rstable)
    {			 /* fatal - should be set for all databases */
	namError("Resource '%S' has no list of valid attributes",
		  entry->name);
	return ajFalse;
    }

    for(j=0; namRsAttrs[j].Name; j++)
	if(ajTableFetchC(rstable,  namRsAttrs[j].Name))
	    iattr++;
    
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
**
** @release 2.7.0
** @@
******************************************************************************/

static AjBool namValidVariable(const NamPEntry entry)
{
    AjPTable attrtable;

    attrtable = (AjPTable) entry->data;

    if(attrtable)
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
**
** @release 2.7.0
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




/* @func ajNamRsAttrValueC ****************************************************
**
** Return the value for a resource attribute
**
** @param [r] name [const char *] resource name
** @param [r] attribute [const char *] resource attribute
** @param [w] value [AjPStr *] resource value

**
** @return [AjBool] true if found
**
** @release 3.0.0
** @@
******************************************************************************/

AjBool ajNamRsAttrValueC(const char *name, const char *attribute,
			 AjPStr *value)
{
    ajint j;
    const NamPEntry fnew = NULL;
    const AjPTable rstable;
    const AjPStr rsvalue;

    ajDebug("ajNamRsAttrValueC '%s' '%s'\n", name, attribute);

    fnew = ajTableFetchC(namResMasterTable, name);

    if(!fnew)
	return ajFalse;
    
    rstable = (const AjPTable) fnew->data;
    j = namRsAttrC(attribute);

    if(j < 0)
    {
        if(namRsAttrFieldC(rstable, attribute))
            return ajFalse;
    
	ajFatal("unknown attribute '%s' requested for resource '%s'",
                attribute, name);
    }

    rsvalue = ajTableFetchC(rstable, attribute);

    if(ajStrGetLen(rsvalue))
    {
	ajStrAssignS(value,rsvalue);

	return ajTrue;
    }

    return ajFalse;
}




/* @func ajNamRsAttrValueS ****************************************************
**
** Return the value for a resource attribute
**
** @param [r] name [const AjPStr] resource name
** @param [r] attribute [const AjPStr] resource attribute
** @param [w] value [AjPStr *] resource value

**
** @return [AjBool] true if found
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajNamRsAttrValueS(const AjPStr name, const AjPStr attribute,
			 AjPStr *value)
{
    ajint j;
    const NamPEntry fnew = NULL;
    const AjPTable rstable;
    const AjPStr rsvalue;

    ajDebug("ajNamRsAttrValueS '%S' '%S'\n", name, attribute);

    fnew = ajTableFetchS(namResMasterTable, name);

    if(!fnew)
	ajFatal("unknown resource '%S'",
                name);

    j = namRsAttrS(attribute);

    rstable = (const AjPTable) fnew->data;
    rsvalue = ajTableFetchS(rstable, attribute);

    ajDebug("resource '%S' found file:%S attribute '%S' j:%d value: '%S'\n",
            name, fnew->file, attribute, j, rsvalue);

    if(ajStrGetLen(rsvalue))
    {
	ajStrAssignS(value,rsvalue);

	return ajTrue;
    }

    if(j < 0)
    {
        if(namRsAttrFieldS(rstable, attribute))
            return ajFalse;
    
	ajFatal("unknown attribute '%S' requested for resource '%S'",
                attribute, name);
    }

    return ajFalse;
}




/* @func ajNamRsListValue *****************************************************
**
** Return the value for a resource attribute of type 'list'
**
** @param [r] name [const AjPStr] resource name
** @param [w] value [AjPStr *] resource value
**
** @return [AjBool] true if found
**
** @release 3.0.0
** @@
******************************************************************************/

AjBool ajNamRsListValue(const AjPStr name, AjPStr *value)
{
    const NamPEntry fnew = NULL;
    const AjPTable rstable = NULL;
    const AjPStr rsvalue = NULL;

    fnew = ajTableFetchS(namResMasterTable, name);

    rstable = (const AjPTable) fnew->data;
    rsvalue = ajTableFetchC(rstable, "type");

    if(!ajStrMatchCaseC(rsvalue, "list"))
	return ajFalse;

    rsvalue = ajTableFetchC(rstable, "value");

    if(ajStrGetLen(rsvalue))
    {
	ajStrAssignS(value,rsvalue);

	return ajTrue;
    }

    return ajFalse;
}




#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif




#ifdef AJ_COMPILE_DEPRECATED
/* @obsolete ajNamGetenv
** @rename ajNamGetenvS
*/
__deprecated AjBool ajNamGetenv(const AjPStr name,
		    AjPStr* value)
{
    return ajNamGetenvS(name, value);
}




/* @obsolete ajNamGetValue
** @rename ajNamGetValusS
*/

__deprecated AjBool ajNamGetValue(const AjPStr name, AjPStr* value)
{
    return ajNamGetValueC(ajStrGetPtr(name), value);
}




/* @obsolete ajNamRootInstall
** @remove Use ajNamValueinstalldir
*/
__deprecated AjBool ajNamRootInstall(AjPStr* root)
{
    ajStrAssignS(root, namFixedInstallStr);

    if(!ajStrGetLen(*root))
        return ajFalse;

    return ajTrue;
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




/* @obsolete ajNamRootVersion
** @remove Use ajNamValueVersion
*/
__deprecated AjBool ajNamRootVersion(AjPStr* version)
{
    ajStrAssignS(version, namFixedVersionStr);

    return ajTrue;
}




/* @obsolete ajNamRoot
** @remove Use ajNamValueRootdir
*/
__deprecated AjBool ajNamRoot(AjPStr* root)
{
    ajStrAssignS(root, namFixedRootStr);

    return ajTrue;
}




/* @obsolete ajNamRootBase
** @remove Use ajNamValueBasedir
*/
__deprecated AjBool ajNamRootBase(AjPStr* rootbase)
{
    ajStrAssignS(rootbase, namFixedBaseStr);

    return ajTrue;
}




/* @obsolete ajNamRsAttrValue
** @rename ajNamRsAttrValueS
*/
__deprecated AjBool ajNamRsAttrValue(const AjPStr name, const AjPStr attribute,
			 AjPStr *value)
{
    return ajNamRsAttrValueS(name, attribute, value);
}
#endif

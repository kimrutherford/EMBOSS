/* @source ajoboread **********************************************************
**
** AJAX OBO reading functions
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.42 $
** @modified May 5 pmr 2010 First AJAX version
** @modified Sep 8 2010 pmr Added query and reading functions
** @modified $Date: 2012/07/17 15:04:04 $ by $Author: rice $
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

#include "ajoboread.h"
#include "ajobo.h"
#include "ajcall.h"
#include "ajlist.h"
#include "ajquery.h"
#include "ajtextread.h"
#include "ajnam.h"
#include "ajfileio.h"

#include <ctype.h>
#include <string.h>


AjPTable oboDbMethods = NULL;


static AjPStr oboinReadLine     = NULL;




/******************************************************************************
**
** To Do:
**
** alt_id needs to become a stored ID somehow and resolve to the same list
**
** validate instance, annotation and formula stanzas
**
** do something (store) header tags
**
** more syntax validation where useful
**
** escape quotes somewhere - after processing the quoted string,
** return it escaped.
**
** validate number of times a term is found -
**   count all possible terms and compare to the
**     minima 0:may-be-absent 1:required
**     maxima 0:unlimited 1:unique 2:pair-at-least
**
** Report all tags that have the obsolete value set
**
******************************************************************************/

#if 0
static const char *oboBuiltin[] =
{
    "OBO:TYPE", 
    "OBO:TERM", 
    "OBO:TERM_OR_TYPE", 
    "OBO:INSTANCE", 
    "xsd:simpleType",
    "xsd:string",
    "xsd:integer",
    "xsd:decimal",
    "xsd:negativeInteger",
    "xsd:positiveInteger",
    "xsd:nonNegativeInteger",
    "xsd:nonPositiveInteger",
    "xsd:boolean",
    "xsd:date",
    NULL
};
#endif /* 0 */



static const char *oboStanza[] =
{
    "Typedef", 
    "Term",
    "Instance", 
    "Annotation",               /* new in 1.3 */
    "Formula",                  /* new in 1.3 */
    NULL
};




/* @enumstatic tagtype ********************************************************
**
** OBO tag value type
**
** @value TAG_ANY Any
** @value TAG_BOOL Boolean
** @value TAG_QTEXT Quoted text
** @value TAG_DBX Database cross-reference
** @value TAG_QXREF Quoted cross-reference 
** @value TAG_ID Identifier
** @value TAG_NAME Name

******************************************************************************/

enum tagtype {
    TAG_ANY, TAG_BOOL, TAG_QTEXT, TAG_DBX, TAG_QXREF, TAG_ID, TAG_NAME
};




/*
** dbxrefs are comma-separated ids and possible "comment"
** need to parse by ',' with quotes honoured.
** watch out for \" within quotes - perhaps these should be escaped later?
** or just accept as [dbxref-list]
*/

/*
** should parse out trailing modifiers from any tag
** {name=value ...}
*/

/* should parse out any dbxref list from any tag? */




/* @datastatic OboOTagdef *****************************************************
**
** Tag definitions for OBO format
**
** @attr Tag [const char*] Tag name
** @attr Type [ajint] Enumerated type
** @attr Min [ajint] Minimum unmber
** @attr Count [ajint] Maximum number
** @attr Padding [ajint] Padding
** @attr Obsolete [const char*] Obsolete form of tag name
******************************************************************************/

typedef struct OboSTagdef 
{
    const char* Tag;
    ajint       Type;
    ajint       Min;
    ajint       Count;
    ajint       Padding;
    const char* Obsolete;
} OboOTagdef;

#define OboPTagdef OboOTagdef*




static OboOTagdef oboTermTags[] =
{
    {"id",              TAG_ID,   1, 1, 0, NULL}, 
    {"is_anonymous",    TAG_BOOL, 0, 1, 0, NULL}, 
    {"name",            TAG_NAME, 0, 1, 0, NULL}, /* optional in 1.4 */
    {"namespace",       TAG_ANY, 0, 1, 0, NULL}, 
    {"alt_id",          TAG_ANY, 0, 0, 0, NULL}, 
    {"def",             TAG_DBX, 0, 1, 0, NULL}, 
    {"comment",         TAG_ANY, 0, 1, 0, NULL}, 
    {"subset",          TAG_QXREF, 0, 0, 0, NULL}, 
    {"synonym",         TAG_NAME, 0, 0, 0, NULL},
    {"exact_synonym",   TAG_NAME, 0, 0, 0, "synonym EXACT"},
    {"narrow_synonym",  TAG_NAME, 0, 0, 0, "synonym NARROW"},
    {"broad_synonym",   TAG_NAME, 0, 0, 0, "synonym BROAD"},
    {"xref",            TAG_ID, 0, 0, 0, NULL}, 
    {"xref_analog",     TAG_ID, 0, 0, 0, "xref"}, 
    {"xref_unk",        TAG_ID, 0, 0, 0, "xref"}, 
    {"example",         TAG_ANY, 0, 1, 0, NULL}, 
    {"regex",           TAG_ANY, 0, 1, 0, NULL}, 
    {"seealso",         TAG_ANY, 0, 0, 0, NULL}, 
    {"is_a",            TAG_ID, 0, 0, 0, NULL},  
    {"intersection_of", TAG_ANY, 0, 2, 0, NULL},
    {"union_of",        TAG_ANY, 0, 2, 0, NULL}, 
    {"disjoint_from",   TAG_ID, 0, 0, 0, NULL}, 
    {"relationship",    TAG_ANY, 0, 0, 0, NULL}, 
    {"is_obsolete",     TAG_BOOL, 0, 1, 0, NULL}, 
    {"replaced_by",     TAG_ID, 0, 0, 0, NULL},          /* for is_obsolete */
    {"consider",        TAG_ANY, 0, 0, 0, NULL},         /* for is_obsolete */
    {"use_term",        TAG_ANY, 0, 0, 0, "consider"}, 
    {"builtin",         TAG_BOOL, 0, 1, 0, NULL},
    {"created_by",      TAG_ANY, 0, 1, 0, NULL},               /* new in 1.3 */
    {"creation_date",   TAG_ANY, 0, 1, 0, NULL},            /* new in 1.3 */
    {"spec",            TAG_ANY, 0, 1, 0, NULL},            /* EDAM only */
    {NULL, 0, 0, 0, 0, NULL}
};




static const char *oboTypedefTags[] =
{
    "id", 
    "is_anonymous", 
    "name", 
    "namespace", 
    "alt_id", 
    "def", 
    "comment", 
    "subset", 
    "synonym",
    "exact_synonym",
    "narrow_synonym",
    "broad_synonym",
    "xref", 
    "xref_analog", 
    "xref_unk", 
    "domain", 
    "range", 
    "is_anti_symmetric", 
    "is_cyclic", 
    "is_reflexive", 
    "is_symmetric", 
    "is_transitive", 
    "is_a", 
    "intersection_of",              /* new in 1.3 */
    "union_of",                     /* new in 1.3 */
    "disjoint_from",                /* new in 1.3 */
    "inverse_of", 
    "inverse_of_at_instance_level", /* new in 1.3 */
    "transitive_over",
    "relationship",
    "is_obsolete", 
    "replaced_by",                  /* for is_obsolete */
    "consider",                     /* for is_obsolete */
    "is_metadata_tag", 
    "created_by",               /* new in 1.3 */
    "creation_date",            /* new in 1.3 */
    NULL
};



#if 0
static const char *oboInstanceTags[] =
{
    "id", 
    "is_anonymous", 
    "name", 
    "namespace", 
    "alt_id", 
    "comment",
    "synonym",
    "xref",
    "instance_of",
    "property_value",
    "is_obsolete",              /* for is_obsolete */
    "replaced_by",              /* for is_obsolete */
    "consider", 
    NULL
};




static const char *oboAnnotationTags[] =             /* new in 1.3 */
{
    "id", 
    "is_anonymous", 
    "name", 
    "namespace", 
    "alt_id", 
    "comment",
    "subset",
    "synonym",
    "xref",
    "is_a",
    "created_by",
    "creation_date",
    "is_obsolete", 
    "replaced_by",              /* for is_obsolete */
    "consider",                 /* for is_obsolete */
    "subject",
    "relation",
    "is_negated",
    "object",
    "source",
    "assigned_by",
    "evidence",
    "secondary_taxon",          /* order undefined in 1.3 draft */
     NULL
};




static const char *oboFormulaTags[] =      /* new and undocumented in 1.3 */
{
    "id", 
    "is_anonymous", 
    "name", 
    "namespace", 
    "alt_id", 
    "comment",
    "formula",
    NULL
};
#endif /* 0 */



static const char *oboHeaderTags[] =
{
    "format-version",
    "data-version",
    "version",
    "date",
    "saved-by",
    "auto-generated-by",
    "import",
    "subsetdef",
    "synonymtypedef",
    "default-namespace",
    "idspace",
    "default-relationship-id-prefix",
    "id-mapping",
    "remark",
    "treat-xrefs-as-equivalent",                        /* new in 1.3 */
    "treat-xrefs-as-genus-differentia",                 /* new in 1.3 */
    "treat-xrefs-as-relationship",                      /* new in 1.3 */
    "treat-xrefs-as-is_a",                              /* new in 1.3 */
    "relax-unique-identifier-assumption-for-namespace", /* new in 1.3 */
    "relax-unique-label-assumption-for-namespace",      /* new in 1.3 */
    "next_id",                                          /* used by EDAM */
    "",
    NULL
};

static AjBool oboinReadObo(AjPOboin thys, AjPObo obo);




/* @datastatic OboPInFormat ***************************************************
**
** Obo input formats data structure
**
** @alias OboSInFormat
** @alias OboOInFormat
**
** @attr Name [const char*] Format name
** @attr Obo  [const char*] Ontology term id from EDAM
** @attr Desc [const char*] Format description
** @attr Alias [AjBool] Name is an alias for an identical definition
** @attr Try [AjBool] If true, try for an unknown input. Duplicate names
**                    and read-anything formats are set false
** @attr Read [AjBool function] Input function, returns ajTrue on success
** @@
******************************************************************************/

typedef struct OboSInFormat
{
    const char *Name;
    const char *Obo;
    const char *Desc;
    AjBool Alias;
    AjBool Try;
    AjBool (*Read) (AjPOboin thys, AjPObo obo);
} OboOInFormat;

#define OboPInFormat OboOInFormat*




static OboOInFormat oboinFormatDef[] =
{
/* "Name",        "Description" */
/*     Alias,   Try,     */
/*     ReadFunction */
  {"unknown",     "0000", "Unknown format",
       AJFALSE, AJFALSE,
       &oboinReadObo}, /* alias for obo */
  {"obo",         "2196", "OBO format",
       AJFALSE, AJTRUE,
       &oboinReadObo},
  {NULL, NULL, NULL, 0, 0, NULL}
};

static ajuint     oboErrorCount = 0;



static void   oboinWarn(const AjPFile obofile, ajuint linecnt,
                        const AjPStr id, const char* fmt, ...);

static void   oboinDebug(const AjPFile obofile, ajuint linecnt,
                         const char* fmt, ...);

static AjBool oboDefine(AjPObo thys, AjPOboin oboin);
static AjBool oboQueryMatch(const AjPQuery thys, const AjPObo obo);
static AjBool oboinQryProcess(AjPOboin oboin, AjPObo obo);
static AjBool oboinRead(AjPOboin thys, AjPObo obo);
static AjBool oboinformatFind(const AjPStr format, ajint* iformat);
static AjBool oboinFormatSet(AjPOboin oboin, AjPObo obo);
static AjBool oboinListProcess(AjPOboin oboin, AjPObo obo,
                               const AjPStr listfile);
static void oboinListNoComment(AjPStr* text);
static void oboinQryRestore(AjPOboin oboin, const AjPQueryList node);
static void oboinQrySave(AjPQueryList node, const AjPOboin oboin);
static AjBool obolineParseDef(AjPObo obo, const AjPStr line);




/* @filesection ajobo ********************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
******************************************************************************/





/* @datasection [AjPOboin] Obo Input ******************************************
**
** Function is for manipulating obo input objects
**
** @nam2rule Oboin Obo input objects
**
******************************************************************************/




/* @section Obo Input Constructors ********************************************
**
** All constructors return a new obo input object by pointer. It
** is the responsibility of the user to first destroy any previous
** obo term input object. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so
** anyway.
**
** @fdata [AjPOboin]
**
** @nam3rule New Constructor
**
** @valrule * [AjPOboin] Obo input object
**
** @fcategory new
**
******************************************************************************/




/* @func ajOboinNew ***********************************************************
**
** Creates a new obo input object.
**
** @return [AjPOboin] New obo input object.
** @category new [AjPOboin] Default constructor
**
** @release 6.4.0
** @@
******************************************************************************/

AjPOboin ajOboinNew(void)
{
    AjPOboin pthis;

    AJNEW0(pthis);

    pthis->Input = ajTextinNewDatatype(AJDATATYPE_OBO);

    pthis->OboData      = NULL;

    return pthis;
}





/* ==================================================================== */
/* ========================== destructors ============================= */
/* ==================================================================== */




/* @section Obo Input Destructors ****************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the obo input object.
**
** @fdata [AjPOboin]
**
** @nam3rule Del Destructor
**
** @argrule Del pthis [AjPOboin*] Obo input
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajOboinDel ***********************************************************
**
** Deletes an obo input object.
**
** @param [d] pthis [AjPOboin*] Obo input
** @return [void]
** @category delete [AjPOboin] Default destructor
**
** @release 6.4.0
** @@
******************************************************************************/

void ajOboinDel(AjPOboin* pthis)
{
    AjPOboin thys;

    if(!pthis)
        return;

    thys = *pthis;

    if(!thys)
        return;

    ajDebug("ajOboinDel called qry:'%S'\n", thys->Input->Qry);

    ajTextinDel(&thys->Input);

    AJFREE(*pthis);

    return;
}




/* ==================================================================== */
/* =========================== Modifiers ============================== */
/* ==================================================================== */




/* @section obo input modifiers ******************************************
**
** These functions use the contents of an obo term input object and
** update them.
**
** @fdata [AjPOboin]
**
** @nam3rule Clear Clear all values
** @nam3rule Qry Reset using a query string
** @suffix C Character string input
** @suffix S String input
**
** @argrule * thys [AjPOboin] Obo input object
** @argrule C txt [const char*] Query text
** @argrule S str [const AjPStr] query string
**
** @valrule * [void]
**
** @fcategory modify
**
******************************************************************************/




/* @func ajOboinClear *********************************************************
**
** Clears an obo term input object back to "as new" condition, except
** for the query list which must be preserved.
**
** @param [w] thys [AjPOboin] obo term input
** @return [void]
** @category modify [AjPOboin] Resets ready for reuse.
**
** @release 6.4.0
** @@
******************************************************************************/

void ajOboinClear(AjPOboin thys)
{

    ajDebug("ajOboinClear called\n");

    if(!thys)
        return;

    ajTextinClear(thys->Input);

    thys->OboData = NULL;

    return;
}




/* @func ajOboinQryC **********************************************************
**
** Resets an obo term input object using a new Universal
** obo term Address
**
** @param [u] thys [AjPOboin] obo term input object.
** @param [r] txt [const char*] Query
** @return [void]
** @category modify [AjPOboin] Resets using a new query
**
** @release 6.4.0
** @@
******************************************************************************/

void ajOboinQryC(AjPOboin thys, const char* txt)
{
    ajOboinClear(thys);
    ajTextinQryC(thys->Input, txt);

    return;
}





/* @func ajOboinQryS **********************************************************
**
** Resets an obo term input object using a new Universal
** obo term Address
**
** @param [u] thys [AjPOboin] obo term input object.
** @param [r] str [const AjPStr] Query
** @return [void]
** @category modify [AjPOboin] Resets using a new query
**
** @release 6.4.0
** @@
******************************************************************************/

void ajOboinQryS(AjPOboin thys, const AjPStr str)
{
    ajOboinClear(thys);
    ajTextinQryS(thys->Input, str);

    return;
}




/* @section casts *************************************************************
**
** Return values
**
** @fdata [AjPOboin]
**
** @nam3rule Get Get obo input stream values
** @nam4rule GetQry Get obo query
** @nam3rule Trace Write debugging output
** @suffix S Return as a string object
**
** @argrule * thys [const AjPOboin] Obo input object
**
** @valrule * [void]
** @valrule *S [const AjPStr] String value
**
** @fcategory cast
**
******************************************************************************/




/* @func ajOboinGetQryS *******************************************************
**
** Returns the query of an obo term input object
**
** @param [r] thys [const AjPOboin] obo term input object.
** @return [const AjPStr] Query string
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPStr ajOboinGetQryS(const AjPOboin thys)
{
    return ajTextinGetQryS(thys->Input);

}




/* @func ajOboinTrace *********************************************************
**
** Debug calls to trace the data in an obo term input object.
**
** @param [r] thys [const AjPOboin] obo term input object.
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajOboinTrace(const AjPOboin thys)
{
    ajDebug("obo term input trace\n");
    ajDebug("====================\n\n");

    ajTextinTrace(thys->Input);

    if(thys->OboData)
	ajDebug( "  OboData: exists\n");

    return;
}




/* @section obo term inputs **********************************************
**
** These functions read the obo term provided by the first argument
**
** @fdata [AjPOboin]
**
** @nam3rule Read Read obo term(s)
**
** @argrule Read oboin [AjPOboin] Obo input object
** @argrule Read obo [AjPObo] Obo term
**
** @valrule * [AjBool] true on success
**
** @fcategory input
**
******************************************************************************/




/* @func ajOboinRead **********************************************************
**
** If the file is not yet open, calls oboinQryProcess to convert the query into
** an open file stream.
**
** Uses oboinRead for the actual file reading.
**
** Returns the results in the AjPObo object.
**
** @param [u] oboin [AjPOboin] obo term input definitions
** @param [w] obo [AjPObo] obo term returned.
** @return [AjBool] ajTrue on success.
** @category input [AjPObo] Master obo term input, calls specific functions
**                  for file access type and obo term format.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajOboinRead(AjPOboin oboin, AjPObo obo)
{
    AjBool ret       = ajFalse;
    AjPQueryList node = NULL;
    AjBool listdata  = ajFalse;

    if(oboin->Input->Filebuff)
    {
	/* (a) if file still open, keep reading */
	ajDebug("ajOboinRead: input file '%F' still there, try again at %Lu\n",
		oboin->Input->Filebuff->File, oboin->Input->Filebuff->Fpos);
	ret = oboinRead(oboin, obo);
	ajDebug("ajOboinRead: open buffer  qry: '%S' returns: %B\n",
		oboin->Input->Qry, ret);
    }
    else
    {
	/* (b) if we have a list, try the next query in the list */
	if(ajListGetLength(oboin->Input->List))
	{
	    listdata = ajTrue;
	    ajListPop(oboin->Input->List, (void**) &node);

	    ajDebug("++pop from list '%S'\n", node->Qry);
	    ajOboinQryS(oboin, node->Qry);
	    ajDebug("++SAVE OBOIN '%S' '%S' %d\n",
		    oboin->Input->Qry,
		    oboin->Input->Formatstr, oboin->Input->Format);

            oboinQryRestore(oboin, node);

	    ajStrDel(&node->Qry);
	    ajStrDel(&node->Formatstr);
	    AJFREE(node);

	    ajDebug("ajOboinRead: open list, try '%S'\n", oboin->Input->Qry);

	    if(!oboinQryProcess(oboin, obo) &&
               !ajListGetLength(oboin->Input->List))
		return ajFalse;

	    ret = oboinRead(oboin, obo);
	    ajDebug("ajOboinRead: list qry: '%S' returns: %B\n",
		    oboin->Input->Qry, ret);
	}
	else
	{
	    ajDebug("ajOboinRead: no file yet - test query '%S'\n",
                    oboin->Input->Qry);

	    /* (c) Must be a query - decode it */
	    if(!oboinQryProcess(oboin, obo) &&
               !ajListGetLength(oboin->Input->List))
		return ajFalse;

	    if(ajListGetLength(oboin->Input->List)) /* could be a new list */
		listdata = ajTrue;

	    ret = oboinRead(oboin, obo);
	    ajDebug("ajOboinRead: new qry: '%S' returns: %B\n",
		    oboin->Input->Qry, ret);
	}
    }

    /* Now read whatever we got */

    while(!ret && ajListGetLength(oboin->Input->List))
    {
	/* Failed, but we have a list still - keep trying it */
        if(listdata)
	    ajErr("Failed to read obo term '%S'", oboin->Input->Qry);

	listdata = ajTrue;
	ajListPop(oboin->Input->List,(void**) &node);
	ajDebug("++try again: pop from list '%S'\n", node->Qry);
	ajOboinQryS(oboin, node->Qry);
	ajDebug("++SAVE (AGAIN) OBOIN '%S' '%S' %d\n",
		oboin->Input->Qry,
		oboin->Input->Formatstr, oboin->Input->Format);

	oboinQryRestore(oboin, node);

	ajStrDel(&node->Qry);
	ajStrDel(&node->Formatstr);
	AJFREE(node);

	if(!oboinQryProcess(oboin, obo))
	    continue;

	ret = oboinRead(oboin, obo);
	ajDebug("ajOboinRead: list retry qry: '%S' returns: %B\n",
		oboin->Input->Qry, ret);
    }

    if(!ret)
    {
	if(listdata)
	    ajErr("Failed to read obo term '%S'", oboin->Input->Qry);

	return ajFalse;
    }


    oboDefine(obo, oboin);

    return ajTrue;
}




/* @funcstatic oboinReadFmt ***************************************************
**
** Tests whether an obo term can be read using the specified format.
** Then tests whether the obo term matches obo term query criteria
** and checks any specified type. Applies upper and lower case.
**
** @param [u] oboin [AjPOboin] obo term input object
** @param [w] obo [AjPObo] obo term object
** @param [r] format [ajuint] input format code
** @return [ajuint] 0 if successful.
**                  1 if the query match failed.
**                  2 if the obo term type failed
**                  3 if it failed to read an obo term
**
** @release 6.4.0
** @@
** This is the only function that calls the appropriate Read function
** oboinReadXxxxxx where Xxxxxxx is the supported obo term format.
**
** Some of the oboReadXxxxxx functions fail to reset the buffer correctly,
** which is a very serious problem when cycling through all of them to
** identify an unknown format. The extra ajFileBuffReset call at the end is
** intended to address this problem. The individual functions should still
** reset the buffer in case they are called from elsewhere.
**
******************************************************************************/

static ajuint oboinReadFmt(AjPOboin oboin, AjPObo obo,
                           ajuint format)
{
    ajDebug("++oboinReadFmt format %d (%s) '%S'\n",
	    format, oboinFormatDef[format].Name,
	    oboin->Input->Qry);

    oboin->Input->Records = 0;

    /* Calling funclist oboinFormatDef() */
    if((*oboinFormatDef[format].Read)(oboin, obo))
    {
	ajDebug("oboinReadFmt success with format %d (%s) '%S'\n",
		format, oboinFormatDef[format].Name, obo->Name);
	oboin->Input->Format = format;
	ajStrAssignC(&oboin->Input->Formatstr, oboinFormatDef[format].Name);
	ajStrAssignC(&obo->Formatstr, oboinFormatDef[format].Name);
	ajStrAssignEmptyS(&obo->Db, oboin->Input->Db);
	ajStrAssignS(&obo->Filename, oboin->Input->Filename);

	if(oboQueryMatch(oboin->Input->Query, obo))
	{
            /* ajOboinTrace(oboin); */

            return FMT_OK;
        }

	ajDebug("query match failed, continuing ...\n");
	ajOboClear(obo);

	return FMT_NOMATCH;
    }
    else
    {
	ajDebug("Testing input buffer: IsBuff: %B Eof: %B\n",
		ajFilebuffIsBuffered(oboin->Input->Filebuff),
		ajFilebuffIsEof(oboin->Input->Filebuff));

	if (!ajFilebuffIsBuffered(oboin->Input->Filebuff) &&
	    ajFilebuffIsEof(oboin->Input->Filebuff))
	    return FMT_EOF;

	ajFilebuffResetStore(oboin->Input->Filebuff,
                             oboin->Input->Text, &obo->TextPtr);
	ajDebug("Format %d (%s) failed, file buffer reset by oboinReadFmt\n",
		format, oboinFormatDef[format].Name);
	/* ajFilebuffTraceFull(oboin->Input->Filebuff, 10, 10);*/
    }

    ajDebug("++oboinReadFmt failed - nothing read\n");

    return FMT_FAIL;
}




/* @funcstatic oboinRead ******************************************************
**
** Given data in a oboin structure, tries to read everything needed
** using the specified format or by trial and error.
**
** @param [u] oboin [AjPOboin] obo term input object
** @param [w] obo [AjPObo] obo term object
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool oboinRead(AjPOboin oboin, AjPObo obo)
{
    ajuint i;
    ajuint istat = 0;
    ajuint jstat = 0;

    AjPFilebuff buff = oboin->Input->Filebuff;
/*    AjBool regfile = ajFalse; */
    AjBool ok;

    AjPTextAccess textaccess = oboin->Input->Query->TextAccess;
    AjPOboAccess  oboaccess  = oboin->Input->Query->Access;

    ajOboClear(obo);
    ajDebug("oboinRead: cleared\n");

    if(oboin->Input->Single && oboin->Input->Count)
    {
	/*
	** One obo term at a time is read.
	** The first obo term was read by ACD
	** for the following ones we need to reset the AjPOboin
	**
	** Single is set by the access method
	*/

	ajDebug("oboinRead: single access - count %d - call access"
		" routine again\n",
		oboin->Input->Count);
	/* Calling funclist oboinAccess() */
	if(textaccess)
        {
            if(!(*textaccess->Access)(oboin->Input))
            {
                ajDebug("oboinRead: (*textaccess->Access)(oboin->Input) "
                        "*failed*\n");

                return ajFalse;
            }
        }

	if(oboaccess)
        {
            if(!(*oboaccess->Access)(oboin))
            {
                ajDebug("oboinRead: (*oboaccess->Access)(oboin) "
                        "*failed*\n");

                return ajFalse;
            }
        }

        buff = oboin->Input->Filebuff;
    }

    ajDebug("oboinRead: oboin format %d '%S'\n", oboin->Input->Format,
	    oboin->Input->Formatstr);

    oboin->Input->Count++;

    if(!oboin->Input->Filebuff)
	return ajFalse;

    ok = ajFilebuffIsBuffered(oboin->Input->Filebuff);

    while(ok)
    {				/* skip blank lines */
        ok = ajBuffreadLine(oboin->Input->Filebuff, &oboinReadLine);

        if(!ajStrIsWhite(oboinReadLine))
        {
            ajFilebuffClear(oboin->Input->Filebuff,1);
            break;
        }
    }

    if(!oboin->Input->Format)
    {			   /* no format specified, try all defaults */

/*        regfile = ajFileIsFile(ajFilebuffGetFile(oboin->Input->Filebuff)); */

	for(i = 1; oboinFormatDef[i].Name; i++)
	{
	    if(!oboinFormatDef[i].Try)	/* skip if Try is ajFalse */
		continue;

	    ajDebug("oboinRead:try format %d (%s)\n",
		    i, oboinFormatDef[i].Name);

	    istat = oboinReadFmt(oboin, obo, i);

	    switch(istat)
	    {
	    case FMT_OK:
		ajDebug("++oboinRead OK, set format %d\n",
                        oboin->Input->Format);
		oboDefine(obo, oboin);

		return ajTrue;
	    case FMT_BADTYPE:
		ajDebug("oboinRead: (a1) oboinReadFmt stat == BADTYPE "
                        "*failed*\n");

		return ajFalse;
	    case FMT_FAIL:
		ajDebug("oboinRead: (b1) oboinReadFmt stat == FAIL "
                        "*failed*\n");
		break;			/* we can try next format */
	    case FMT_NOMATCH:
		ajDebug("oboinRead: (c1) oboinReadFmt stat==NOMATCH "
                        "try again\n");
		break;
	    case FMT_EOF:
		ajDebug("oboinRead: (d1) oboinReadFmt stat == EOF "
                        "*failed*\n");
		return ajFalse;			/* EOF and unbuffered */
	    case FMT_EMPTY:
		ajWarn("obo term '%S' has zero length, ignored",
		       ajOboGetQryS(obo));
		ajDebug("oboinRead: (e1) oboinReadFmt stat==EMPTY "
                        "try again\n");
		break;
	    default:
		ajDebug("unknown code %d from oboinReadFmt\n", stat);
	    }

	    ajOboClear(obo);

	    if(oboin->Input->Format)
		break;			/* we read something */

            /*ajFilebuffTrace(oboin->Input->Filebuff);*/
	}

	if(!oboin->Input->Format)
	{		     /* all default formats failed, give up */
	    ajDebug("oboinRead:all default formats failed, give up\n");

	    return ajFalse;
	}

	ajDebug("++oboinRead set format %d\n", oboin->Input->Format);
    }
    else
    {					/* one format specified */
	ajDebug("oboinRead: one format specified\n");
	ajFilebuffSetUnbuffered(oboin->Input->Filebuff);

	ajDebug("++oboinRead known format %d\n", oboin->Input->Format);
	istat = oboinReadFmt(oboin, obo, oboin->Input->Format);

	switch(istat)
	{
	case FMT_OK:
	    oboDefine(obo, oboin);

	    return ajTrue;
	case FMT_BADTYPE:
	    ajDebug("oboinRead: (a2) oboinReadFmt stat == BADTYPE "
                    "*failed*\n");

	    return ajFalse;

        case FMT_FAIL:
	    ajDebug("oboinRead: (b2) oboinReadFmt stat == FAIL "
                    "*failed*\n");

	    return ajFalse;

        case FMT_NOMATCH:
	    ajDebug("oboinRead: (c2) oboinReadFmt stat == NOMATCH "
                    "*try again*\n");
	    break;
	case FMT_EOF:
	    ajDebug("oboinRead: (d2) oboinReadFmt stat == EOF "
                    "*try again*\n");
            if(oboin->Input->Records)
                ajErr("Error reading file '%F' with format '%s': "
                      "end-of-file before end of data "
                      "(read %u records)",
                      ajFilebuffGetFile(oboin->Input->Filebuff),
                      oboinFormatDef[oboin->Input->Format].Name,
                      oboin->Input->Records);
	    break;		     /* simply end-of-file */
	case FMT_EMPTY:
	    ajWarn("obo term '%S' has zero length, ignored",
		   ajOboGetQryS(obo));
	    ajDebug("oboinRead: (e2) oboinReadFmt stat == EMPTY "
                    "*try again*\n");
	    break;
	default:
	    ajDebug("unknown code %d from oboinReadFmt\n", stat);
	}

	ajOboClear(obo); /* 1 : read, failed to match id/acc/query */
    }

    /* failed - probably entry/accession query failed. Can we try again? */

    ajDebug("oboinRead failed - try again with format %d '%s' code %d\n",
	    oboin->Input->Format,
            oboinFormatDef[oboin->Input->Format].Name, istat);

    ajDebug("Search:%B Chunk:%B Data:%x ajFileBuffEmpty:%B\n",
	    oboin->Input->Search, oboin->Input->ChunkEntries,
            oboin->Input->TextData, ajFilebuffIsEmpty(buff));

    if(ajFilebuffIsEmpty(buff) && oboin->Input->ChunkEntries)
    {
	if(textaccess && !(*textaccess->Access)(oboin->Input))
            return ajFalse;
	else if(oboaccess && !(*oboaccess->Access)(oboin))
            return ajFalse;
        buff = oboin->Input->Filebuff;
    }


    /* need to check end-of-file to avoid repeats */
    while(oboin->Input->Search &&
          (oboin->Input->TextData || !ajFilebuffIsEmpty(buff)))
    {
	jstat = oboinReadFmt(oboin, obo, oboin->Input->Format);

	switch(jstat)
	{
	case FMT_OK:
	    oboDefine(obo, oboin);

	    return ajTrue;

        case FMT_BADTYPE:
	    ajDebug("oboinRead: (a3) oboinReadFmt stat == BADTYPE "
                    "*failed*\n");

	    return ajFalse;

        case FMT_FAIL:
	    ajDebug("oboinRead: (b3) oboinReadFmt stat == FAIL "
                    "*failed*\n");

	    return ajFalse;
            
	case FMT_NOMATCH:
	    ajDebug("oboinRead: (c3) oboinReadFmt stat == NOMATCH "
                    "*try again*\n");
	    break;
	case FMT_EOF:
	    ajDebug("oboinRead: (d3) oboinReadFmt stat == EOF "
                    "*failed*\n");

	    return ajFalse;			/* we already tried again */

        case FMT_EMPTY:
	    if(istat != FMT_EMPTY)
                ajWarn("obo term '%S' has zero length, ignored",
                       ajOboGetQryS(obo));
	    ajDebug("oboinRead: (e3) oboinReadFmt stat == EMPTY "
                    "*try again*\n");
	    break;

        default:
	    ajDebug("unknown code %d from oboinReadFmt\n", stat);
	}

	ajOboClear(obo); /* 1 : read, failed to match id/acc/query */
    }

    if(oboin->Input->Format)
	ajDebug("oboinRead: *failed* to read obo term %S using format %s\n",
		oboin->Input->Qry, oboinFormatDef[oboin->Input->Format].Name);
    else
	ajDebug("oboinRead: *failed* to read obo term %S using any format\n",
		oboin->Input->Qry);

    return ajFalse;
}




/* @funcstatic oboinReadObo ***************************************************
**
** Given data in an obo structure, tries to read everything needed
** using the OBO format.
**
** @param [u] oboin [AjPOboin] Obo input object
** @param [w] obo [AjPObo] Obo object
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool oboinReadObo(AjPOboin oboin, AjPObo obo)
{
    AjPFilebuff buff;

    AjPFile obofile;
    ajlong fpos     = 0;
    ajlong spos     = 0;
    ajuint linecnt  = 0;
    AjBool incomplete = ajFalse;
    AjBool isterm = ajFalse;
    AjPStr token = NULL;
    AjPStr saveline = NULL;
    AjPStr rest  = NULL;
    AjPStr name   = NULL;
    AjPStr modifier = NULL;
    AjPStr comment  = NULL;
    AjPObotag obotag   = NULL;
    AjBool ok = ajFalse;

    ajDebug("oboinReadObo\n");
    ajOboClear(obo);
    buff = oboin->Input->Filebuff;
    obofile = ajFilebuffGetFile(buff);

    /* ajFilebuffTrace(buff); */
    ok = ajBuffreadLine(buff, &oboinReadLine);
    while(ok && !ajStrPrefixC(oboinReadLine, "[Term]"))
    {
        ok = ajBuffreadLine(buff, &oboinReadLine);
    }

    if(!ok)
        return ajFalse;

    isterm = ajTrue;

    ajFilebuffClear(buff, 1);

    ok = ajBuffreadLinePosStore(buff, &oboinReadLine, &fpos,
                                oboin->Input->Text, &obo->TextPtr);
    if(ok)
        ok = ajBuffreadLinePosStore(buff, &oboinReadLine, &fpos,
                                    oboin->Input->Text, &obo->TextPtr);

    while (ok)
    {
        ajStrTrimWhite(&oboinReadLine);

        if(!ajStrGetLen(oboinReadLine))
            break;

        if(ajStrGetCharFirst(oboinReadLine) == '[') /* new stanza */
            break;

        linecnt++;

/*        ajDebug("line %u:%S\n", linecnt, oboinReadLine);*/

        ajObolineCutComment(&oboinReadLine, &comment);
        ajObolineEscape(&oboinReadLine);

        if((ajStrGetCharLast(oboinReadLine) == '\\') &&
           (ajStrGetCharPos(oboinReadLine, -2) != '\\'))
        {
            incomplete = ajTrue;
            ajStrAppendS(&saveline, oboinReadLine);
            ok = ajBuffreadLinePosStore(buff, &oboinReadLine, &fpos,
                                        oboin->Input->Text, &obo->TextPtr);
            continue;
        }
        
        if(incomplete)          /* continued line */
        {
            ajStrAppendS(&saveline, oboinReadLine);
            ajStrAssignS(&oboinReadLine, saveline);
            ajStrAssignC(&saveline, "");
            incomplete = ajFalse;
        }

        if (!ajStrGetLen(oboinReadLine)) /* empty line */
        {
            if(!ajStrGetLen(comment))
                break;

            ok = ajBuffreadLinePosStore(buff, &oboinReadLine, &fpos,
                                        oboin->Input->Text, &obo->TextPtr);
            continue;
        }

        /* normal name:value record */

       if(ajObolineCutModifier(&oboinReadLine, &modifier))
            oboinDebug(obofile, linecnt,
                       "modifier found '%S'",
                       modifier);

        /* parse name: value ! comment */
        ajStrExtractWord(oboinReadLine, &rest, &token);

        if(!ajStrGetLen(rest))
            oboinWarn(obofile, linecnt, obo->Id,
                      "missing value for tag '%S' in Term '%S'",
                      token, obo->Id);
        ajStrAssignS(&name, token);

        if(ajStrGetCharLast(name) != ':')
            oboinWarn(obofile, linecnt, obo->Id,
                      "bad name '%S'",
                      token);
        ajStrCutEnd(&name, 1);

        obotag = ajObotagNewData(name, rest, modifier, comment, linecnt);

        if(ajStrMatchC(name, "id"))
        {
            if(MAJSTRGETLEN(obo->Id))
                oboinWarn(obofile, linecnt, obo->Id,
                        "Multiple '%S' tags for ID '%S'",
                        name, obo->Id);
            ajStrAssignS(&obo->Fullid, obotag->Value);
            spos = ajStrFindlastK(obotag->Value, ':');
            if(spos < 0)
                ajStrAssignS(&obo->Id, obotag->Value);
            else
            {
                ajStrAssignSubS(&obo->Db, obotag->Value, 0, spos-1);
                ajStrAssignSubS(&obo->Id, obotag->Value, spos+1, -1);
            }
        }
        else if(ajStrMatchC(name, "name"))
        {
            if(MAJSTRGETLEN(obo->Name))
                oboinWarn(obofile, linecnt, obo->Id,
                        "Multiple '%S' tags for ID '%S'",
                        name, obo->Id);
            ajStrAssignS(&obo->Name, obotag->Value);
        }
        else if(ajStrMatchC(name, "synonym"))
        {
            ajListPushAppend(obo->Taglist, obotag);
            obotag = NULL;
        }
        else if(ajStrMatchC(name, "namespace"))
        {
            if(MAJSTRGETLEN(obo->Namespace))
                oboinWarn(obofile, linecnt, obo->Id,
                        "Multiple '%S' tags for ID '%S'",
                        name, obo->Id);
            ajStrAssignS(&obo->Namespace, obotag->Value);
        }
        else if(ajStrMatchC(name, "def"))
        {
            if(MAJSTRGETLEN(obo->Def))
                oboinWarn(obofile, linecnt, obo->Id,
                        "Multiple '%S' tags for ID '%S'",
                        name, obo->Id);
            ajStrAssignS(&obo->Def, obotag->Value);
            obolineParseDef(obo, obotag->Value);
        }
        else if(ajStrMatchC(name, "comment"))
        {
            if(MAJSTRGETLEN(obo->Comment))
                oboinWarn(obofile, linecnt, obo->Id,
                        "Multiple '%S' tags for ID '%S'",
                        name, obo->Id);
            ajStrAssignS(&obo->Comment, obotag->Value);
        }
        else if(ajStrMatchC(name, "is_obsolete"))
        {
            if(obo->Obsolete)
                oboinWarn(obofile, linecnt, obo->Id,
                        "Multiple '%S' tags for ID '%S'",
                        name, obo->Id);
            if(ajStrPrefixC(obotag->Value, "true"))
                obo->Obsolete = ajTrue;
            else if(ajStrPrefixC(obotag->Value, "false"))
                obo->Obsolete = ajFalse;
            else
                oboinWarn(obofile, linecnt, obo->Id,
                        "Bad value '%S' boolean tag for ID '%S'",
                        name, obo->Id);
            ajListPushAppend(obo->Taglist, obotag);
            obotag = NULL;
        }
        else if(ajStrMatchC(name, "builtin"))
        {
            if(obo->Obsolete)
                oboinWarn(obofile, linecnt, obo->Id,
                        "Multiple '%S' tags for ID '%S'",
                        name, obo->Id);
            if(ajStrPrefixC(obotag->Value, "true"))
                obo->Builtin = ajTrue;
            else if(ajStrPrefixC(obotag->Value, "false"))
                obo->Builtin = ajFalse;
            else
                oboinWarn(obofile, linecnt, obo->Id,
                        "Bad value '%S' boolean tag for ID '%S'",
                        name, obo->Id);
            ajListPushAppend(obo->Taglist, obotag);
            obotag = NULL;
        }
        else if(ajStrMatchC(name, "replaced_by"))
        {
            /*
            if(MAJSTRGETLEN(obo->Replaced))
                oboinWarn(obofile, linecnt, obo->Id,
                        "Multiple '%S' tags for ID '%S'",
                        name, obo->Id);
            */
            ajStrAssignS(&obo->Replaced, obotag->Value);
            ajListPushAppend(obo->Taglist, obotag);
            obotag = NULL;
        }
        else if(ajStrMatchC(name, "consider"))
        {
            ajStrAssignS(&obo->Replaced, obotag->Value);
            ajListPushAppend(obo->Taglist, obotag);
            obotag = NULL;
        }
        else 
        {
            ajListPushAppend(obo->Taglist, obotag);
            obotag = NULL;
        }
        ajObotagDel(&obotag);
        ok = ajBuffreadLinePosStore(buff, &oboinReadLine, &fpos,
                               oboin->Input->Text, &obo->TextPtr);
    }

    if(!isterm)
    {
        oboinWarn(obofile, linecnt, obo->Id,
                "Bad OBO term '%S' '%S'",
                name, obo->Id);
        return ajFalse;
    }

    /* ajDebug("First line: %S\n", oboinReadLine); */

    if(ok)
        ajFilebuffClearStore(buff, 1,
                             oboinReadLine,
                             oboin->Input->Text, &obo->TextPtr);
    else
        ajFilebuffClear(buff, 0);

    ajStrDel(&token);
    ajStrDel(&rest);
    ajStrDel(&saveline);
    ajStrDel(&name);
    ajStrDel(&modifier);
    ajStrDel(&comment);

    return ajTrue;
}




/* @datasection [AjPOboall] Obo Input Stream **********************************
**
** Function is for manipulating obo input stream objects
**
** @nam2rule Oboall Obo input stream objects
**
******************************************************************************/




/* @section Obo Input Constructors ********************************************
**
** All constructors return a new obo input stream object by pointer. It
** is the responsibility of the user to first destroy any previous
** obo term input object. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so
** anyway.
**
** @fdata [AjPOboall]
**
** @nam3rule New Constructor
**
** @valrule * [AjPOboall] Obo input stream object
**
** @fcategory new
**
******************************************************************************/




/* @func ajOboallNew **********************************************************
**
** Creates a new obo input stream object.
**
** @return [AjPOboall] New obo input stream object.
**
** @release 6.4.0
** @@
******************************************************************************/

AjPOboall ajOboallNew(void)
{
    AjPOboall pthis;

    AJNEW0(pthis);

    pthis->Oboin = ajOboinNew();
    pthis->Obo   = ajOboNew();

    return pthis;
}





/* ==================================================================== */
/* ========================== destructors ============================= */
/* ==================================================================== */




/* @section OBO Input Stream Destructors **************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the obo input stream object.
**
** @fdata [AjPOboall]
**
** @nam3rule Del Destructor
**
** @argrule Del pthis [AjPOboall*] Obo input stream
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajOboallDel **********************************************************
**
** Deletes an obo input stream object.
**
** @param [d] pthis [AjPOboall*] Obo input stream
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajOboallDel(AjPOboall* pthis)
{
    AjPOboall thys;

    if(!pthis)
        return;

    thys = *pthis;

    if(!thys)
        return;

    ajOboinDel(&thys->Oboin);
    if(!thys->Returned)
        ajOboDel(&thys->Obo);

    AJFREE(*pthis);

    return;
}




/* ==================================================================== */
/* =========================== Modifiers ============================== */
/* ==================================================================== */




/* @section obo input stream modifiers ****************************************
**
** These functions use the contents of an obo term input stream object and
** update them.
**
** @fdata [AjPOboall]
**
** @nam3rule Clear Clear all values
**
** @argrule * thys [AjPOboall] Obo input stream object
**
** @valrule * [void]
**
** @fcategory modify
**
******************************************************************************/




/* @func ajOboallClear ********************************************************
**
** Clears an obo term input stream object back to "as new" condition, except
** for the query list which must be preserved.
**
** @param [w] thys [AjPOboall] obo term input stream
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajOboallClear(AjPOboall thys)
{

    ajDebug("ajOboallClear called\n");

    if(!thys)
        return;

    ajOboinClear(thys->Oboin);

    ajOboClear(thys->Obo);

    thys->Returned = ajFalse;

    return;
}




/* @section obo input stream casts ********************************************
**
** These functions return the contents of an obo term input stream object
**
** @fdata [AjPOboall]
**
** @nam3rule Get Get obo input stream values
** @nam3rule Getobo Get obo term values
** @nam4rule GetoboId Get obo term identifier
** @nam4rule GetQry Get obo query
** @suffix S Return as a string object
**
** @argrule * thys [const AjPOboall] Obo input stream object
**
** @valrule * [const AjPStr] String value
**
** @fcategory cast
**
******************************************************************************/




/* @func ajOboallGetQryS ******************************************************
**
** Returns the query of an input stream
**
** @param [r] thys [const AjPOboall] obo term input stream
** @return [const AjPStr] Identifier
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPStr ajOboallGetQryS(const AjPOboall thys)
{
    if(!thys)
        return NULL;

    ajDebug("ajOboallGetQryS called\n");

    return ajOboinGetQryS(thys->Oboin);
}




/* @func ajOboallGetoboId *****************************************************
**
** Returns the identifier of the current obo term in an input stream
**
** @param [r] thys [const AjPOboall] obo term input stream
** @return [const AjPStr] Identifier
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPStr ajOboallGetoboId(const AjPOboall thys)
{
    if(!thys)
        return NULL;

    ajDebug("ajOboallGetoboId called\n");

    return ajOboGetId(thys->Obo);
}




/* @section obo input *********************************************************
**
** These functions use an obo term input stream object to read data
**
** @fdata [AjPOboall]
**
** @nam3rule Next Read next obo term
**
** @argrule * thys [AjPOboall] Obo input stream object
** @argrule * Pobo [AjPObo*] Obo term object
**
** @valrule * [AjBool] True on success
**
** @fcategory input
**
******************************************************************************/




/* @func ajOboallNext *********************************************************
**
** Parse an OBO query into format, access, file and entry
**
** Split at delimiters. Check for the first part as a valid format
** Check for the remaining first part as a database name or as a file
** that can be opened.
** Anything left is an entryname spec.
**
** Return the results in the AjPObo object but leave the file open for
** future calls.
**
** @param [w] thys [AjPOboall] Obo term input stream
** @param [u] Pobo [AjPObo*] Obo term returned
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajOboallNext(AjPOboall thys, AjPObo *Pobo)
{
    if(!thys->Count)
    {
	thys->Count = 1;

	thys->Totterms++;

	*Pobo = thys->Obo;
	thys->Returned = ajTrue;

	return ajTrue;
    }


    if(ajOboinRead(thys->Oboin, thys->Obo))
    {
	thys->Count++;

	thys->Totterms++;

	*Pobo = thys->Obo;
	thys->Returned = ajTrue;

	ajDebug("ajOboallNext success\n");

	return ajTrue;
    }

    *Pobo = NULL;

    ajDebug("ajOboallNext failed\n");

    ajOboallClear(thys);

    return ajFalse;
}




/* @datasection [none] Input formats ******************************************
**
** Input formats internals
**
** @nam2rule Oboinformat Obo term input format specific
**
******************************************************************************/




/* @section cast **************************************************************
**
** Values for input formats
**
** @fdata [none]
**
** @nam3rule Find Return index to named format
** @nam3rule Term Test format EDAM term
** @nam3rule Test Test format value
**
** @argrule Find format [const AjPStr] Format name
** @argrule Term term [const AjPStr] Format EDAM term
** @argrule Test format [const AjPStr] Format name
** @argrule Find iformat [ajint*] Index matching format name
**
** @valrule * [AjBool] True if found
**
** @fcategory cast
**
******************************************************************************/




/* @funcstatic oboinformatFind ************************************************
**
** Looks for the specified format(s) in the internal definitions and
** returns the index.
**
** Sets iformat as the recognised format, and returns ajTrue.
**
** @param [r] format [const AjPStr] Format required.
** @param [w] iformat [ajint*] Index
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool oboinformatFind(const AjPStr format, ajint* iformat)
{
    AjPStr tmpformat = NULL;
    ajuint i = 0;

    /* ajDebug("oboinformatFind '%S'\n", format); */
    if(!ajStrGetLen(format))
	return ajFalse;

    ajStrAssignS(&tmpformat, format);
    ajStrFmtLower(&tmpformat);

    for(i=0; oboinFormatDef[i].Name; i++)
    {
	/* ajDebug("test %d '%s' \n",
           i, oboinFormatDef[i].Name,
           oboinFormatDef[i].Obo,
           oboinFormatDef[i].Desc); */
	if(ajStrMatchC(tmpformat, oboinFormatDef[i].Name) ||
           ajStrMatchC(format, oboinFormatDef[i].Obo))
	{
	    *iformat = i;
	    ajStrDel(&tmpformat);
	    /* ajDebug("found '%s' at %d\n", oboinFormatDef[i].Name, i); */
	    return ajTrue;
	}
    }

    ajErr("Unknown input format '%S'", format);

    ajStrDel(&tmpformat);

    return ajFalse;
}




/* @func ajOboinformatTerm ****************************************************
**
** tests whether an OBO input format term is known
**
** @param [r] term [const AjPStr] Format term EDAM ID
** @return [AjBool] ajTrue if term was accepted
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajOboinformatTerm(const AjPStr term)
{
    ajuint i;

    for(i=0; oboinFormatDef[i].Name; i++)
	if(ajStrMatchC(term, oboinFormatDef[i].Obo))
	    return ajTrue;

    return ajFalse;
}




/* @func ajOboinformatTest ****************************************************
**
** tests whether a named OBO input format is known
**
** @param [r] format [const AjPStr] Format
** @return [AjBool] ajTrue if format was accepted
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajOboinformatTest(const AjPStr format)
{
    ajuint i;

    for(i=0; oboinFormatDef[i].Name; i++)
    {
	if(ajStrMatchCaseC(format, oboinFormatDef[i].Name))
	    return ajTrue;
	if(ajStrMatchC(format, oboinFormatDef[i].Obo))
	    return ajTrue;
    }

    return ajFalse;
}




/* @datasection [AjPStr] Query string *****************************************
**
** Query string processing
**
** @nam2rule Oboqry Query handling for obo terms
**
******************************************************************************/




/* @section Casts *************************************************************
**
** returns attributes of a query string
**
** @fdata [AjPStr]
**
** @nam3rule Get Return a value
** @nam4rule Obo return matching obo term
**
** @argrule Get qry [const AjPStr] Query string
** @argrule Base Pbaseqry [AjPStr*] Base part of query
** @argrule Obo obo [AjPObo] Obo term with values updated
**
** @valrule * [AjBool] True on success
**
** @fcategory cast
**
******************************************************************************/




/* @func ajOboqryGetObo *******************************************************
**
** Returns an obo term given a query
**
** @param [r] qry [const AjPStr] Query
** @param [u] obo [AjPObo] obo term
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajOboqryGetObo(const AjPStr qry, AjPObo obo)
{
    AjPOboin oboin;
    AjBool ok;

    oboin        = ajOboinNew();

    ajOboinQryS(oboin, qry);
    ok = ajOboinRead(oboin, obo);
    ajOboinDel(&oboin);

    if(!ok)
	return ajFalse;

    return ajTrue;
}




/* @funcstatic oboinQryProcess ************************************************
**
** Converts an obo query into an open file.
**
** Tests for "format::" and sets this if it is found
**
** Then tests for "list:" or "@" and processes as a list file
** using oboinListProcess which in turn invokes oboinQryProcess
** until a valid query is found.
**
** Then tests for dbname:query and opens the file (at the correct position
** if the database definition defines it)
**
** If there is no database, looks for file:query and opens the file.
** In this case the file position is not known and obo term reading
** will have to scan for the entry/entries we need.
**
** @param [u] oboin [AjPOboin] obo term input structure.
** @param [u] obo [AjPObo] obo term to be read. The format will be replaced
**                         if defined in the query string.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool oboinQryProcess(AjPOboin oboin, AjPObo obo)
{
    AjBool ret = ajTrue;
    AjPStr qrystr = NULL;
    AjBool obomethod = ajFalse;
    const AjPStr fmtstr = NULL;
    AjPTextin textin;
    AjPQuery qry;
    AjPOboAccess oboaccess = NULL;

    textin = oboin->Input;
    qry = textin->Query;

    /* pick up the original query string */
    qrystr = ajStrNewS(textin->Qry);

    ajDebug("oboinQryProcess '%S'\n", qrystr);

    /* look for a format:: prefix */
    fmtstr = ajQuerystrParseFormat(&qrystr, textin, oboinformatFind);
    ajDebug("oboinQryProcess ... fmtstr '%S' '%S'\n", fmtstr, qrystr);

    /* (seq/feat) DO NOT look for a [range] suffix */

    /* look for a list:: or @:: listfile of queries  - process and return */
    if(ajQuerystrParseListfile(&qrystr))
    {
        ajDebug("oboinQryProcess ... listfile '%S'\n", qrystr);
        ret = oboinListProcess(oboin, obo, qrystr);
        ajStrDel(&qrystr);
        return ret;
    }

    /* try general text access methods (file, asis, text database access */
    ajDebug("oboinQryProcess ... no listfile '%S'\n", qrystr);
    if(!ajQuerystrParseRead(&qrystr, textin, oboinformatFind, &obomethod))
    {
        ajStrDel(&qrystr);
        return ajFalse;
    }
    
    oboinFormatSet(oboin, obo);

    ajDebug("oboinQryProcess ... read nontext: %B '%S'\n",
            obomethod, qrystr);
    ajStrDel(&qrystr);

    /* we found a non-text method */
    if(obomethod)
    {
        ajDebug("oboinQryProcess ... call method '%S'\n", qry->Method);
        ajDebug("oboinQryProcess ... textin format %d '%S'\n",
                textin->Format, textin->Formatstr);
        ajDebug("oboinQryProcess ...  query format  '%S'\n",
                qry->Formatstr);
        qry->Access = ajCallTableGetS(oboDbMethods,qry->Method);
        oboaccess = qry->Access;
        return (*oboaccess->Access)(oboin);
    }

    ajDebug("oboinQryProcess text method '%S' success\n", qry->Method);

    return ajTrue;
}




/* @datasection [AjPList] Query field list ************************************
**
** Query fields lists are handled internally. Only static functions
** should appear here
**
******************************************************************************/




/* @funcstatic oboinListProcess ***********************************************
**
** Processes a file of queries.
** This function is called by, and calls, oboinQryProcess. There is
** a depth check to avoid infinite loops, for example where a list file
** refers to itself.
**
** This function produces a list (AjPList) of queries with all list references
** expanded into lists of queries.
**
** Because queries in a list can have their own format
** the prior settings are stored with each query in the list node so that they
** can be restored after.
**
** @param [u] oboin [AjPOboin] obo term input
** @param [u] obo [AjPObo] obo term
** @param [r] listfile [const AjPStr] Name of list file.,
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool oboinListProcess(AjPOboin oboin, AjPObo obo,
                               const AjPStr listfile)
{
    AjPList list  = NULL;
    AjPFile file  = NULL;
    AjPStr token  = NULL;
    AjPStrTok handle = NULL;
    AjBool ret       = ajFalse;
    AjPQueryList node = NULL;

    static ajint depth    = 0;
    static ajint MAXDEPTH = 16;

    depth++;
    ajDebug("++oboListProcess %S depth %d\n",
	    listfile, depth);

    if(depth > MAXDEPTH)
	ajFatal("Query list too deep");

    if(!oboin->Input->List)
	oboin->Input->List = ajListNew();

    list = ajListNew();

    file = ajFileNewInNameS(listfile);

    if(!file)
    {
	ajErr("Failed to open list file '%S'", listfile);
	depth--;

	return ret;
    }

    while(ajReadlineTrim(file, &oboinReadLine))
    {
	oboinListNoComment(&oboinReadLine);

	if(ajStrGetLen(oboinReadLine))
	{
	    ajStrTokenAssignC(&handle, oboinReadLine, " \t\n\r");
	    ajStrTokenNextParse(&handle, &token);
	    /* ajDebug("Line  '%S'\n");*/
	    /* ajDebug("token '%S'\n", oboinReadLine, token); */

	    if(ajStrGetLen(token))
	    {
	        ajDebug("++Add to list: '%S'\n", token);
	        AJNEW0(node);
	        ajStrAssignS(&node->Qry, token);
	        oboinQrySave(node, oboin);
	        ajListPushAppend(list, node);
	    }

	    ajStrDel(&token);
	    token = NULL;
	}
    }

    ajFileClose(&file);
    ajStrDel(&token);

    /*
    ajDebug("Trace oboin->Input->List\n");
    ajQuerylistTrace(oboin->Input->List);
    ajDebug("Trace new list\n");
    ajQuerylistTrace(list);
    */

    ajListPushlist(oboin->Input->List, &list);

    ajDebug("Trace combined oboin->Input->List\n");
    ajQuerylistTrace(oboin->Input->List);

    /*
     ** now try the first item on the list
     ** this can descend recursively if it is also a list
     ** which is why we check the depth above
     */

    if(ajListPop(oboin->Input->List, (void**) &node))
    {
        ajDebug("++pop first item '%S'\n", node->Qry);
	ajOboinQryS(oboin, node->Qry);
	oboinQryRestore(oboin, node);
	ajStrDel(&node->Qry);
	ajStrDel(&node->Formatstr);
	AJFREE(node);
	ajDebug("descending with query '%S'\n", oboin->Input->Qry);
	ret = oboinQryProcess(oboin, obo);
    }

    ajStrTokenDel(&handle);
    depth--;
    ajDebug("++oboListProcess depth: %d returns: %B\n", depth, ret);

    return ret;
}




/* @funcstatic oboinListNoComment *********************************************
**
** Strips comments from a character string (a line from an ACD file).
** Comments are blank lines or any text following a "#" character.
**
** @param [u] text [AjPStr*] Line of text from input file.
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void oboinListNoComment(AjPStr* text)
{
    ajuint i;
    char *cp;

    i = ajStrGetLen(*text);

    if(!i)				/* empty string */
	return;

    MAJSTRGETUNIQUESTR(text);
    
    cp = strchr(ajStrGetPtr(*text), '#');

    if(cp)
    {					/* comment found */
	*cp = '\0';
	ajStrSetValid(text);
    }

    return;
}




/* @funcstatic oboinFormatSet *************************************************
**
** Sets the input format for an obo term using the obo term input object's
** defined format, or a default from variable 'EMBOSS_FORMAT'.
**
** @param [u] oboin [AjPOboin] obo term input.
** @param [u] obo [AjPObo] obo term.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool oboinFormatSet(AjPOboin oboin, AjPObo obo)
{

    if(ajStrGetLen(oboin->Input->Formatstr))
    {
	ajDebug("... input format value '%S'\n", oboin->Input->Formatstr);

	if(oboinformatFind(oboin->Input->Formatstr, &oboin->Input->Format))
	{
	    ajStrAssignS(&obo->Formatstr, oboin->Input->Formatstr);
	    obo->Format = oboin->Input->Format;
	    ajDebug("...format OK '%S' = %d\n", oboin->Input->Formatstr,
		    oboin->Input->Format);
	}
	else
	    ajDebug("...format unknown '%S'\n", oboin->Input->Formatstr);

	return ajTrue;
    }
    else
	ajDebug("...input format not set\n");


    return ajFalse;
}




/* @datasection [AjPQuery] Obo query objects *******************************
**
** Obo query object functions
**
** @nam2rule Oboquery
**
******************************************************************************/




/* @section Query Casts ***************************************************
**
** These functions examine the contents of query object
** and return some derived information. Some of them provide access to
** the internal components of a query object. They are
** provided for programming convenience but should be used with
** caution.
**
** @fdata [AjPQuery]
**
** @nam3rule Getall Return all values
** @nam4rule Obofields Return a set of standard obo query fields
**
** @suffix C Character string input
**
** @argrule * query [const AjPQuery] Query object
**
** @argrule Obofields id [const AjPStr*] Id query
** @argrule Obofields acc [const AjPStr*] Alt id query
** @argrule Obofields nam [const AjPStr*] Name query
** @argrule Obofields des [const AjPStr*] Description query
** @argrule Obofields up [const AjPStr*] Parent query
** @argrule Obofields space [const AjPStr*] Namespace query
**
** @valrule *Obofields [ajuint] Number of fields found
**
** @fcategory cast
**
******************************************************************************/




/* @func ajOboqueryGetallObofields ********************************************
**
** Returns the standard obo query string from a query
**
** @param [r] query [const AjPQuery] Query
** @param [w] id   [const AjPStr*] Id query
** @param [w] acc  [const AjPStr*] Accession query
** @param [w] nam  [const AjPStr*] Name query
** @param [w] des  [const AjPStr*] Description query
** @param [w] up   [const AjPStr*] Parent query
** @param [w] space [const AjPStr*] Namespace query
** @return [ajuint] Number of fields found
**
** @release 6.4.0
******************************************************************************/

ajuint ajOboqueryGetallObofields(const AjPQuery query,
                                 const AjPStr *id, const AjPStr *acc,
                                 const AjPStr *nam, const AjPStr *des,
                                 const AjPStr *up, const AjPStr *space)
{
    ajuint ret = 0;
    AjIList iter = NULL;
    AjPQueryField field = NULL;

    iter = ajListIterNewread(query->QueryFields);
    while(!ajListIterDone(iter))
    {
        field = ajListIterGet(iter);
        ret++;
        if(ajStrMatchC(field->Field, "id"))
            *id = field->Wildquery;
        else if(ajStrMatchC(field->Field, "acc"))
            *acc = field->Wildquery;
        else if(ajStrMatchC(field->Field, "nam"))
            *nam = field->Wildquery;
        else if(ajStrMatchC(field->Field, "des"))
            *des = field->Wildquery;
        else if(ajStrMatchC(field->Field, "up"))
            *up = field->Wildquery;
        else if(ajStrMatchC(field->Field, "div"))
            *space = field->Wildquery;
        else
            ret--;
    }
    ajListIterDel(&iter);

    return ret;
}




/* @funcstatic oboQueryMatch **************************************************
**
** Compares an obo term to a query and returns true if they match.
**
** @param [r] thys [const AjPQuery] query.
** @param [r] obo [const AjPObo] obo term.
** @return [AjBool] ajTrue if the obo term matches the query.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool oboQueryMatch(const AjPQuery thys, const AjPObo obo)
{
    AjBool tested = ajFalse;
    AjIList iter  = NULL;
    AjIList iterfield  = NULL;
    AjPQueryField field = NULL;
    AjBool ok = ajFalse;

    ajDebug("oboQueryMatch '%S' fields: %Lu Case %B Done %B\n",
	    obo->Name, ajListGetLength(thys->QueryFields),
            thys->CaseId, thys->QryDone);

    if(!thys)			   /* no query to test, that's fine */
	return ajTrue;

    if(thys->QryDone)			/* do we need to test here? */
	return ajTrue;

    /* test the query field(s) */

    iterfield = ajListIterNewread(thys->QueryFields);
    while(!ajListIterDone(iterfield))
    {
        field = ajListIterGet(iterfield);

        ajDebug("  field: '%S' Query: '%S'\n",
                field->Field, field->Wildquery);
        if(ajStrMatchC(field->Field, "id"))
        {
            ajDebug("  id test: '%S'\n",
                    obo->Id);
            if(thys->CaseId)
            {
                if(ajStrMatchWildS(obo->Id, field->Wildquery))
                {
                    ajListIterDel(&iterfield);
                    return ajTrue;
                }
            }
            else
            {
                if(ajStrMatchWildCaseS(obo->Id, field->Wildquery))
                {
                    ajListIterDel(&iterfield);
                    return ajTrue;
                }
            }
            
            ajDebug("id test failed\n");
            tested = ajTrue;
            ok = ajFalse;
        }

        if(ajStrMatchC(field->Field, "nam")) /* test Sv and Gi */
        {
            ajDebug(" nam test: '%S'\n",
                    obo->Name);
            if(ajStrMatchWildCaseS(obo->Name, field->Wildquery))
            {
                ajListIterDel(&iterfield);
                return ajTrue;
            }

            ajDebug("sv test failed\n");
            tested = ajTrue;
            ok = ajFalse;
        }

        if(ajStrMatchC(field->Field, "acc")) /* test id, use trueid */
        {
            if(ajStrMatchWildCaseS(obo->Id, field->Wildquery))
            {
                ajListIterDel(&iterfield);
                return ajTrue;
            }
        }

        if(ajStrMatchC(field->Field, "des"))
        {
            ajDebug("  des test: '%S'\n",
                    obo->Def);
            if(ajStrGetLen(obo->Def))
            {            /* description test - check the string */
                ajDebug("... try description '%S' '%S'\n", obo->Def,
                        field->Wildquery);

                if(ajStrMatchWildWordCaseS(obo->Def, field->Wildquery))
                {
                    ajListIterDel(&iterfield);
                    return ajTrue;
                }

                tested = ajTrue;
                ajDebug("des test failed\n");
                ajListIterDel(&iter);
            }
            else
            {
                ajDebug("des test failed - nothing to test\n");
                ajListIterDel(&iterfield);
                return ajFalse;
            }
        }
    }
        
    ajListIterDel(&iterfield);

    if(!tested)		    /* nothing to test, so accept it anyway */
    {
        ajDebug("  no tests: assume OK\n");
	return ajTrue;
    }
    
    ajDebug("result: %B\n", ok);

    return ok;
}




/* @funcstatic oboDefine ******************************************************
**
** Make sure all obo term object attributes are defined
** using values from the obo input object if needed
**
** @param [w] thys [AjPObo] Obo term returned.
** @param [u] oboin [AjPOboin] Obo term input definitions
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool oboDefine(AjPObo thys, AjPOboin oboin)
{

    /* if values are missing in the OBO object, we can use defaults
       from oboin or calculate where possible */

    /*ajDebug("oboDefine: thys->Db '%S', oboin->Db '%S'\n",
      thys->Db, oboin->Db);*/
    /*ajDebug("oboDefine: thys->Name '%S' type: %S\n",
      thys->Name, thys->Type);*/
    /*ajDebug("oboDefine: thys->Entryname '%S', oboin->Entryname '%S'\n",
      thys->Entryname, oboin->Entryname);*/

    /* assign the dbname and entryname if defined in the oboin object */
    if(ajStrGetLen(oboin->Input->Db))
      ajStrAssignEmptyS(&thys->Db, oboin->Input->Db);

    /*ajDebug("oboDefine: returns thys->Name '%S' type: %S\n",
      thys->Name, thys->Type);*/

    return ajTrue;
}





/* @funcstatic oboinQryRestore ************************************************
**
** Restores an obo input specification from an AjPQueryList node
**
** @param [w] oboin [AjPOboin] Obo input object
** @param [r] node [const AjPQueryList] Query list node
** @return [void]
**
** @release 6.4.0
******************************************************************************/

static void oboinQryRestore(AjPOboin oboin, const AjPQueryList node)
{
    oboin->Input->Format = node->Format;
    oboin->Input->Fpos   = node->Fpos;
    ajStrAssignS(&oboin->Input->Formatstr, node->Formatstr);
    ajStrAssignS(&oboin->Input->QryFields, node->QryFields);

    return;
}




/* @funcstatic oboinQrySave ***************************************************
**
** Saves an obo input specification in an OboPListQry node
**
** @param [w] node [AjPQueryList] Query list node
** @param [r] oboin [const AjPOboin] Obo input object
** @return [void]
**
** @release 6.4.0
******************************************************************************/

static void oboinQrySave(AjPQueryList node, const AjPOboin oboin)
{
    node->Format   = oboin->Input->Format;
    node->Fpos     = oboin->Input->Fpos;
    ajStrAssignS(&node->Formatstr, oboin->Input->Formatstr);
    ajStrAssignS(&node->QryFields, oboin->Input->QryFields);

    return;
}




/* @datasection [AjPOboData] Obo Data *****************************************
**
** Function is for manipulating obo internal data objects
**
** @nam2rule Obodata Obo internal data
**
******************************************************************************/





/* @section constructors ******************************************************
**
** Constructors
**
** @fdata [AjPOboData]
**
** @nam3rule New Constructor
** @nam3rule Parse Read from a file
** @nam4rule Obofile Read from a file
**
** @argrule Obofile obofile [AjPFile] Input file
** @argrule Parse validations [const char*] Validation options to turn on/off
**
** @valrule * [AjPOboData] Obo data internals object
**
** @fcategory new
**
******************************************************************************/




/* @func ajObodataNew *********************************************************
**
** Obo data internals constructor
**
** @return [AjPOboData] New object
**
** @release 6.4.0
** @@
******************************************************************************/

AjPOboData ajObodataNew(void)
{
    AjPOboData ret;

    AJNEW0(ret);

    return ret;
}




/* @func ajObodataParseObofile ************************************************
**
** Parse an OBO format file
**
** @param [u] obofile [AjPFile] OBO format input file
** @param [r] validations [const char*] Validations to turn on or off
** @return [AjPOboData] Ontology data object
**
** @release 6.4.0
******************************************************************************/

AjPOboData ajObodataParseObofile(AjPFile obofile, const char* validations)
{
    AjPOboData ret;
    AjPList idprefixlist = NULL;
    ajuint linecnt = 0;
    AjPStr line     = NULL;
    AjPStr saveline = NULL;
    AjBool isstanza   = ajFalse;
    AjBool incomplete = ajFalse;
    AjPStr token = NULL;
    AjPStr rest  = NULL;
    AjPStr stanzatype     = NULL;
    AjPStr laststanzatype = NULL;
    AjPStr id     = NULL;
    AjPStr altid  = NULL;
    AjPStr lastid = NULL;
    AjPStr name   = NULL;
    AjPStr modifier = NULL;
    AjPStr comment  = NULL;
    AjBool ok = ajFalse;
    ajulong i;
    ajulong j;
    ajint icmp;
    ajlong icolon;
    AjBool found;
    AjBool isterm = AJFALSE;
    AjPObo term    = NULL;
    AjPObo altterm = NULL;
    AjPObo oldterm = NULL;
    AjPList stanzalist = NULL;
    AjPList oldlist    = NULL;
    AjPObotag obotag   = NULL;
    ajuint ntypes = 0;
    AjPStr *typeids    = NULL;
    AjPList *typetags  = NULL;
    ajuint nterms = 0;
    AjPStr *termids    = NULL;
    AjPObo *terms  = NULL;
    AjIList iter       = NULL;
    AjIList iterpref   = NULL;
    ajulong s;
    ajlong k;
    AjPStr t;
    AjPList u;
    AjPObo uterm;
    ajulong jsave;
    ajuint jj;
    AjPStr idprefix       = NULL;
    AjPStr tmpstr         = NULL;
    AjPStr tmppref        = NULL;
    AjPStrTok idsplit     = NULL;
    AjPStrTok validsplit  = NULL;
    AjPStr validstr       = NULL;
    AjBool obovalid          = ajTrue;
    AjBool obovalididorder   = ajTrue;
    AjBool obovalididunk     = ajTrue;
    AjBool obovalidmultisyn  = ajTrue;
    AjPStr namekey = NULL;
    AjPStr namestr = NULL;
    AjPList namelist = NULL;
    const AjPObo isaterm = NULL;
    AjPStr tmpid = NULL;
    AjPStr tmpname = NULL;
    ajlong ipos;

    ajDebug("ajObodataParseObofile '%F', validations '%s'\n",
            obofile, validations);

    ret = ajObodataNew();
    namelist = ajListNew();

    if(validations)
    {
        validstr = ajStrNewC(validations);
        ajStrTokenAssignC(&validsplit, validstr, ",");

        while(ajStrTokenNextParse(&validsplit, &tmpstr))
        {
            if(ajStrMatchC(tmpstr, "none"))
            {
                obovalid = ajFalse;
                obovalididorder = ajFalse;
/*                obovalididunk = ajFalse;*/
                obovalidmultisyn = ajFalse;
            }
            else if(ajStrMatchC(tmpstr, "noidorder"))
                obovalididorder = ajFalse;
            else if(ajStrMatchC(tmpstr, "nounkid"))
                obovalididunk = ajFalse;
            else if(ajStrMatchC(tmpstr, "nomultisynonym"))
                obovalidmultisyn = ajFalse;
            else
                ajWarn("ajObodataParseObofile: unknown validation '%S'",
                       tmpstr);
        }
        
    }
    
    ret->Termtable = ajTablestrNew(35000);
    ret->Termnametable = ajTablestrNew(35000);
    ret->Typedeftable = ajTablestrNew(1000);
    ret->Instancetable = ajTablestrNew(1000);

    idprefixlist = ajListstrNew();

    while(ajReadlineTrim(obofile, &line))
    {
        linecnt++;

        if(ajStrGetLen(line) && isspace((int) ajStrGetCharLast(line)))
           oboinWarn(obofile, linecnt, term->Id,
                   "trailing whitespace on line");

        ajObolineCutComment(&line, &comment);
        ajObolineEscape(&line);

        if((ajStrGetCharLast(line) == '\\') &&
           (ajStrGetCharPos(line, -2) != '\\'))
        {
            incomplete = ajTrue;
            ajStrAppendS(&saveline, line);
            continue;
        }
        
        if(incomplete)          /* continued line */
        {
            ajStrAppendS(&saveline, line);
            ajStrAssignS(&line, saveline);
            ajStrAssignC(&saveline, "");
            incomplete = ajFalse;
        }

        if (!ajStrGetLen(line)) /* empty line */
        {
            continue;
        }
        else if(ajStrGetCharFirst(line) == '[')
        {
            if(!isstanza) 
            {
                /* validate header */
                isstanza = ajTrue;
            }
            else
            {
                if(stanzalist)
                {
                    if(isterm)
                    {
                        ajListPushlist(term->Taglist, &stanzalist);
                        oldterm = ajTablePut(ret->Termtable, id, term);

                        if(oldterm)
                        {
                            oboinWarn(obofile, linecnt, term->Id,
                                    "Duplicate %S id '%S'",
                                    stanzatype, id);
                            oldterm = NULL;
                        }

                        while(ajListGetLength(namelist))
                        {
                            namekey = NULL;
                            ajListstrPop(namelist, &namestr);

                            if(ajStrGetLen(term->Namespace))
                                ajFmtPrintS(&namekey, "%S:%S",
                                            term->Namespace,
                                            namestr);
                            else
                                namekey = ajStrNewS(namestr);

                            if(term->Obsolete)
                                ajStrAppendC(&namekey, " (obsolete)");

                            oldterm = ajTablePut(ret->Termnametable,
                                                 namekey, term);
                            if(oldterm)
                            {
                                if(!ajStrMatchS(oldterm->Id, term->Id))
                                    oboinWarn(obofile, linecnt, term->Id,
                                              "%S Name/Synonym %S "
                                              "duplicates '%S'",
                                              term->Id, namekey, oldterm->Id);
                                else if(obovalidmultisyn)
                                    oboinWarn(obofile, linecnt, term->Id,
                                              "%S Name/Synonym %S "
                                              "multiply defined",
                                              term->Id, namekey);
                                oldterm = NULL;
                            }
                        }
                    }
                    else
                    {
                        if(ajStrMatchC(stanzatype, "Typedef"))
                            oldlist = ajTablePut(ret->Typedeftable,
                                                 id, stanzalist);
                        else if(ajStrMatchC(stanzatype, "Instance"))
                            oldlist = ajTablePut(ret->Instancetable,
                                                 id, stanzalist);
                        else if(ajStrMatchC(stanzatype, "Annotation"))
                            oldlist = ajTablePut(ret->Annotable,
                                                 id, stanzalist);
                        else if(ajStrMatchC(stanzatype, "Formula"))
                            oldlist = ajTablePut(ret->Formulatable,
                                                 id, stanzalist);
                        else 
                            oldlist = ajTablePut(ret->Misctable,
                                                 id, stanzalist);
                        if(oldlist)
                        {
                            oboinWarn(obofile, linecnt, term->Id,
                                    "Duplicate %S id '%S'",
                                    stanzatype, id);
                            oldlist = NULL;
                        }
                    }
                }                
            }

            id = NULL;
            stanzalist = ajListNew();
            
            ajStrExtractWord(line, &rest, &token);
            if(ajStrGetLen(rest))
                oboinWarn(obofile, linecnt, term->Id,
                        "unexpected characters in stanza line: '%S'",
                        rest);

            if(ajStrGetCharLast(token) != ']')
                oboinWarn(obofile, linecnt, term->Id,
                        "unexpected characters in stanza type: '%S'",
                        token);

            ajStrAssignS(&laststanzatype, stanzatype);
            ajStrAssignS(&stanzatype, token);
            ajStrCutEnd(&stanzatype,1);
            ajStrCutStart(&stanzatype,1);

            if(ajStrMatchC(stanzatype, "Term"))
            {
                isterm = ajTrue;
                term = ajOboNew();
            }
            else
                isterm = ajFalse;

            if(ajStrGetLen(laststanzatype) &&
               !ajStrMatchS(laststanzatype, stanzatype))
            {
                ok = ajFalse;

                for(i=0; oboStanza[i]; i++)
                {
                    if(ajStrMatchC(laststanzatype, oboStanza[i]))
                    {
                        for(j = i + 1; oboStanza[j]; j++)
                        {
                            if(ajStrMatchC(stanzatype, oboStanza[j]))
                            {
                                ok = ajTrue;
                                break;
                            }
                        }
                        break;
                    }
                }

                if(!ok && obovalid)
                    oboinWarn(obofile, linecnt, term->Id,
                            "stanza '%S' after '%S'",
                            stanzatype, laststanzatype);
            }
            
            
            ok = ajFalse;

            for(i=0;oboStanza[i];i++)
            {
                if(ajStrMatchC(stanzatype, oboStanza[i]))
                    ok = ajTrue;
            }

            if(!ok)
                oboinWarn(obofile, linecnt, term->Id,
                        "unknown stanza type '%S'",
                        token);
        }
        else 
        {
            if(ajObolineCutModifier(&line, &modifier))
            {
                if(term)
                    ajDebug("%F %u: (%S) modifier found '%S'\n",
                            obofile, linecnt, term->Id,
                            modifier);
                else
                    ajDebug("%F %u: header modifier found '%S'\n",
                            obofile, linecnt,
                            modifier);
            }
            /* parse name: value ! comment */
            ajStrExtractWord(line, &rest, &token);

            if(!ajStrGetLen(rest))
            {
                if(isterm)
                    oboinWarn(obofile, linecnt, term->Id,
                              "missing value for tag '%S' in %S '%S'",
                              token, stanzatype, id);
                else
                    oboinWarn(obofile, linecnt, NULL,
                              "missing value for tag '%S' in %S '%S'",
                              token, stanzatype, id);
                    }
            
            ajStrAssignS(&name, token);

            if(ajStrGetCharLast(name) != ':')
                oboinWarn(obofile, linecnt, term->Id,
                        "bad name '%S'",
                        token);
            ajStrCutEnd(&name, 1);

            if(ajStrMatchC(name, "id"))
            {
                ajStrAssignS(&id, rest);
                icolon = ajStrFindAnyK(id, ':');

                if(icolon > 0)
                {
                    ajStrAssignSubS(&tmpstr, id, 0, icolon);

                    if(!ajStrMatchS(idprefix, tmpstr))
                    {
                        iterpref = ajListIterNew(idprefixlist);
                        found = ajFalse;

                        while(!ajListIterDone(iterpref))
                        {
                            tmppref = ajListstrIterGet(iterpref);

                            if(ajStrMatchS(tmppref, tmpstr))
                            {
                                found = ajTrue;
                                break;
                            }
                        }

                        ajListIterDel(&iterpref);

                        if(!found)
                        {
                            ajStrAssignS(&idprefix, tmpstr);
                            ajListstrPush(idprefixlist, tmpstr);
                            tmpstr = NULL;
                        }
                    }
                }
                
                if(ajStrGetLen(lastid))
                {
                    if(ajStrMatchS(stanzatype, laststanzatype))
                    {
                        icmp = MAJSTRCMPS(id, lastid);

                        if(!icmp)
                            oboinWarn(obofile, linecnt, term->Id,
                                    "duplicate of previous ID '%S'",
                                    id);
                        else if(obovalididorder && (icmp < 0))
                            oboinWarn(obofile, linecnt, term->Id,
                                    "ID out of order '%S' after '%S",
                                    id, lastid);
                    }
                }
                ajStrAssignS(&lastid, id);
            }

            if(isstanza)
            {
                obotag = ajObotagNewData(name, rest, modifier, comment,
                                         linecnt);
                ajListPushAppend(stanzalist, obotag);
                obotag = NULL;

                if(isterm)
                {
                    if(ajStrMatchC(name, "id"))
                    {
                        if(term->Id)
                            oboinWarn(obofile, linecnt, term->Id,
                                    "Multiple '%S' tags for ID '%S'",
                                    name, id);
                        ajStrAssignS(&term->Id, rest);
                    }
                    else if(ajStrMatchC(name, "name"))
                    {
                        if(term->Name)
                            oboinWarn(obofile, linecnt, term->Id,
                                    "Multiple '%S' tags for ID '%S'",
                                    name, id);

                        ajStrAssignS(&term->Name, rest);
                        namekey = ajStrNewS(rest);
                        ajListstrPushAppend(namelist, namekey);
                    }
                    else if(ajStrMatchC(name, "synonym"))
                    {
                        if(ajStrGetCharFirst(rest) == '"')
                        {
                            i = ajStrFindlastK(rest, '"');

                            if(i > 1)
                            {
                                namekey = ajStrNewRes((size_t) i);
                                ajStrAssignSubS(&namekey, rest,
                                                1, (ajlong) i-1);
                                ajListstrPushAppend(namelist, namekey);
                            }
                        }
                    }
                    else if(ajStrMatchC(name, "namespace"))
                    {
                        if(term->Namespace)
                            oboinWarn(obofile, linecnt, term->Id,
                                    "Multiple '%S' tags for ID '%S'",
                                    name, id);

                        ajStrAssignS(&term->Namespace, rest);
                    }
                    else if(ajStrMatchC(name, "def"))
                    {
                        if(term->Def)
                            oboinWarn(obofile, linecnt, term->Id,
                                    "Multiple '%S' tags for ID '%S'",
                                    name, id);

                        ajStrAssignS(&term->Def, rest);
                    }
                    else if(ajStrMatchC(name, "comment"))
                    {
                        if(term->Comment)
                            oboinWarn(obofile, linecnt, term->Id,
                                    "Multiple '%S' tags for ID '%S'",
                                    name, id);

                        ajStrAssignS(&term->Comment, rest);
                    }
                    else if(ajStrMatchC(name, "is_obsolete"))
                    {
                        if(term->Obsolete)
                            oboinWarn(obofile, linecnt, term->Id,
                                    "Multiple '%S' tags for ID '%S'",
                                    name, id);

                        if(ajStrPrefixC(rest, "true"))
                            term->Obsolete = ajTrue;
                        else if(ajStrPrefixC(rest, "false"))
                            term->Obsolete = ajFalse;
                        else
                            oboinWarn(obofile, linecnt, term->Id,
                                    "Bad value '%S' boolean tag for ID '%S'",
                                    name, id);
                    }
                    else if(ajStrMatchC(name, "replaced_by"))
                    {
                        if(term->Replaced)
                            oboinWarn(obofile, linecnt, term->Id,
                                    "Multiple '%S' tags for ID '%S'",
                                    name, id);

                        ajStrAssignS(&term->Replaced, rest);
                    }
                    else if(ajStrMatchC(name, "consider"))
                    {
                        ajStrAssignS(&term->Replaced, rest);
                    }
                    else if(ajStrMatchC(name, "builtin"))
                    {
                        if(term->Obsolete)
                            oboinWarn(obofile, linecnt, term->Id,
                                    "Multiple '%S' tags for ID '%S'",
                                    name, id);

                        if(ajStrPrefixC(rest, "true"))
                            term->Builtin = ajTrue;
                        else if(ajStrPrefixC(rest, "false"))
                            term->Builtin = ajFalse;
                        else
                            oboinWarn(obofile, linecnt, term->Id,
                                    "Bad value '%S' boolean tag for ID '%S'",
                                    name, id);
                    }
                    else if(ajStrMatchC(name, "alt_id"))
                    {
                        altterm = ajOboNew();
                        ajStrAssignS(&altterm->Id, rest);
                        ajStrAssignS(&altterm->Trueid, id);
                        altid = ajStrNewS(rest);
                        oldterm = ajTablePut(ret->Termtable, altid, altterm);
                        altid = NULL;

                        if(oldterm)
                        {
                            oboinWarn(obofile, linecnt, term->Id,
                                    "Duplicate %S alt_id '%S'",
                                    stanzatype, id);

                            oldterm = NULL;
                        }
                    }
                }
            }
            else                /* processing header tags */
            {
                ok =  ajFalse;

                for(i=0;oboHeaderTags[i];i++)
                {
                    if(ajStrMatchC(name, oboHeaderTags[i]))
                    {
                        ok = ajTrue;
                        break;
                    }
                }

                if(!ok)
                {
                    if(term)
                        oboinWarn(obofile, linecnt, term->Id,
                                  "unknown header name '%S'",
                                  token);
                    else
                        oboinWarn(obofile, linecnt, NULL,
                                  "unknown header name noid",
                                  token);
                }
            }

            ajStrTrimWhite(&rest);
        }
    }

    /*
    ** Apply the same tests to the last term
    ** if that was the final stanza in the input file
    */

    if(stanzalist)
    {
        if(isterm)
        {
            if(!term->Obsolete)
            {
                ajListPushlist(term->Taglist, &stanzalist);
                oldterm = ajTablePut(ret->Termtable, id, term);            

                if(oldterm)
                {
                    oboinWarn(obofile, linecnt, term->Id,
                            "Duplicate %S id '%S'",
                            stanzatype, id);
                    ajOboDel(&oldterm);
                }

                while(ajListGetLength(namelist))
                {
                    namekey = NULL;
                    ajListstrPop(namelist, &namestr);

                    if(ajStrGetLen(term->Namespace))
                        ajFmtPrintS(&namekey, "%S:%S",
                                    term->Namespace,
                                    namestr);
                    else
                        namekey = ajStrNewS(namestr);

                    if(term->Obsolete)
                        ajStrAppendC(&namekey, " (obsolete)");
                            
                    oldterm = ajTablePut(ret->Termnametable,
                                         namekey, term);
                    if(oldterm)
                    {
                        if(!ajStrMatchS(oldterm->Id, term->Id))
                            oboinWarn(obofile, linecnt, term->Id,
                                      "%S Name/Synonym %S "
                                      "duplicates '%S'",
                                      term->Id, namekey, oldterm->Id);
                        else if(obovalidmultisyn)
                            oboinWarn(obofile, linecnt, term->Id,
                                      "%S Name/Synonym %S "
                                      "multiply defined",
                                      term->Id, namekey);
                         oldterm = NULL;
                    }
                }

                term = NULL;
            }
        }
        else 
        {
            if(ajStrMatchC(stanzatype, "Typedef"))
                oldlist = ajTablePut(ret->Typedeftable, id, stanzalist);
            else if(ajStrMatchC(stanzatype, "Instance"))
                oldlist = ajTablePut(ret->Instancetable, id, stanzalist);
            else if(ajStrMatchC(stanzatype, "Annotation"))
                oldlist = ajTablePut(ret->Annotable, id, stanzalist);
            else if(ajStrMatchC(stanzatype, "Formula"))
                oldlist = ajTablePut(ret->Formulatable, id, stanzalist);
            else 
                oldlist = ajTablePut(ret->Misctable, id, stanzalist);

            if(oldlist)
            {
                oboinWarn(obofile, linecnt, term->Id,
                        "Duplicate %S id '%S'",
                        stanzatype, id);
                oldlist = NULL;
            }
        }
    }

    id = NULL;
    stanzalist = ajListNew();

    ajDebug("typedefs:%Lu terms:%Lu instances:%Lu annotations:%Lu "
            "formulas:%Lu other:%Lu\n",
            ajTableGetLength(ret->Typedeftable),
            ajTableGetLength(ret->Termtable),
            ajTableGetLength(ret->Instancetable),
            ajTableGetLength(ret->Annotable),
            ajTableGetLength(ret->Formulatable),
            ajTableGetLength(ret->Misctable));

    /*
    ** Typedefs first
    ** fetch and sort them
    */

    ntypes = (ajuint) ajTableToarrayKeysValues(ret->Typedeftable,
					       (void***) &typeids,
					       (void***) &typetags);
    for(s=ntypes/2; s>0; s /= 2)
	for(i=s; i<ntypes; ++i)
	  for(k = (ajlong)(i-s);k>=0 && MAJSTRCMPS(typeids[k],typeids[k+s])>0;
	      k -= (ajlong)s)
	    {
		t = typeids[k];
		typeids[k] = typeids[k+s];
		typeids[k+s] = t;

		u = typetags[k];
		typetags[k] = typetags[k+s];
		typetags[k+s] = u;
	    }

    for(i=0; i<ntypes; i++)
    {
        id = typeids[i];
        iter = ajListIterNewread(typetags[i]);
        j = 0;
        jsave = 0;

        while(!ajListIterDone(iter))
        {
            obotag = ajListIterGet(iter);

            while(oboTypedefTags[j] &&
                  !ajStrMatchC(obotag->Name, oboTypedefTags[j]))
                j++;

            if(oboTypedefTags[j])
                jsave = j;
            else
            {
                for(jj=0; jj < jsave; jj++)
                {
                    if(obovalididorder &&
                       ajStrMatchC(obotag->Name, oboTypedefTags[jj]))
                    {
                        ajWarn("type tag out of order '%S' after '%s' "
                               "for id '%S'",
                               obotag->Name, oboTypedefTags[jsave], id);
                        j = jj;
                        break;
                    }
                }
                
            }

            ajStrTokenAssignC(&idsplit, obotag->Value, " \t,;[]{}()'\"");

            while (ajStrTokenNextParse(&idsplit, &tmpstr))
            {
                if(ajStrPrefixS(tmpstr, idprefix))
                {
                    if(obovalididunk &&
                       !ajTableMatchS(ret->Termtable, tmpstr))
                        oboinWarn(obofile, obotag->Linenumber, NULL,
                                  "Unknown id '%S' for tag '%S' "
                                  "in typedef '%S'",
                                  tmpstr, obotag->Name, id);
                }
            }
        }
    }

    /*
    ** Validate terms
    */

    nterms = (ajuint) ajTableToarrayKeysValues(ret->Termtable,
					       (void***) &termids,
					       (void***) &terms);
    for(s=nterms/2; s>0; s /= 2)
	for(i=s; i<nterms; ++i)
	  for(k = (ajlong)(i-s); k>=0 && MAJSTRCMPS(termids[k],termids[k+s])>0;
	      k -= (ajlong)s)
	    {
		t = termids[k];
		termids[k] = termids[k+s];
		termids[k+s] = t;

		uterm = terms[k];
		terms[k] = terms[k+s];
		terms[k+s] = uterm;
	    }

    for(i=0; i<nterms; i++)
    {
        id = termids[i];

        if(ajStrGetLen(terms[i]->Trueid))
        {
            ajStrTokenAssignC(&idsplit, terms[i]->Trueid, " \t,;[]{}()'\"");

            while (ajStrTokenNextParse(&idsplit, &tmpstr))
            {
                ajStrTrimEndC(&tmpstr, ".");

                if(ajStrPrefixS(tmpstr, idprefix))
                    if(obovalididunk && !ajTableMatchS(ret->Termtable, tmpstr))
                        oboinWarn(obofile, 0, NULL,
                                "Unknown id '%S' for tag alt_id in term '%S'",
                                tmpstr, terms[i]->Trueid);
            }

            continue;
        }

        /* valid values for major tags*/

        if(!terms[i]->Name)
        {
            oboinWarn(obofile, 0, NULL,
                    "no 'name' for term '%S' ... OWL data only  in OBO 1.3",
                    id);
        }
        
        if(terms[i]->Def)
        {
            if(ajStrGetCharFirst(terms[i]->Def) != '"')
                oboinWarn(obofile, obotag->Linenumber, NULL,
                        "bad 'def' tag value for term '%S'",
                        id);
        }
        
        iter = ajListIterNewread(terms[i]->Taglist);
        j = 0;
        jj = 0;
        jsave = 0;

        while(!ajListIterDone(iter))
        {
            obotag = ajListIterGet(iter);

            j = 0;

            while(oboTermTags[j].Tag &&
                  !ajStrMatchC(obotag->Name, oboTermTags[j].Tag))
                j++;

            if(oboTermTags[j].Tag)
                jsave = j;
            else
            {
                jj = 0;

                while(typeids[jj] &&
                      !ajStrMatchS(obotag->Name, typeids[jj]))
                    jj++;

                if(!typeids[jj])
                    oboinWarn(obofile, obotag->Linenumber, NULL,
                            "term tag not found '%S' for id '%S'",
                            obotag->Name, id);
            }
            
            if(ajStrMatchC(obotag->Name, "is_a") ||
               ajStrMatchC(obotag->Name, "consider"))
            {
                ajStrAssignS(&tmpname, obotag->Comment);
                ipos = ajStrFindAnyK(tmpname, '!');
                if(ipos >= 0)
                {
                    ajStrKeepRange(&tmpname, 0, ipos-1);
                    ajStrTrimEndC(&tmpname, " ");
                }

                isaterm = ajTableFetchS(ret->Termtable, obotag->Value);
                if(!isaterm)
                    oboinWarn(obofile, obotag->Linenumber, NULL,
                              "Unknown id '%S' for tag '%S' "
                              "in term '%S'",
                              obotag->Value, obotag->Name, id);

                else if(!ajStrMatchS(isaterm->Name, tmpname))
                    oboinWarn(obofile, obotag->Linenumber, NULL,
                              "Bad name '%S' for id '%S' "
                              "for tag '%S' "
                              "in term '%S' expected '%S'",
                              tmpname, obotag->Value,
                              obotag->Name, id, isaterm->Name);
            }
            else if(ajStrMatchC(obotag->Name, "relationship"))
            {
                ajStrTokenAssignC(&idsplit, obotag->Value, " \t,;[]{}()'\"");
                ajStrTokenNextParse(&idsplit, &tmpstr);
                ajStrTokenNextParse(&idsplit, &tmpid);

                ajStrAssignS(&tmpname, obotag->Comment);
                ipos = ajStrFindAnyK(tmpname, '!');
                if(ipos >= 0)
                {
                    ajStrKeepRange(&tmpname, 0, ipos-1);
                    ajStrTrimEndC(&tmpname, " ");
                }

                isaterm = ajTableFetchS(ret->Termtable, tmpid);
                if(!isaterm)
                    oboinWarn(obofile, obotag->Linenumber, NULL,
                              "Unknown id '%S' for tag '%S' relation '%S'"
                              "in term '%S'",
                              tmpid, obotag->Name, tmpstr, id);

                else if(!ajStrMatchS(isaterm->Name, tmpname))
                    oboinWarn(obofile, obotag->Linenumber, NULL,
                              "Bad name '%S' for id '%S' "
                              "for tag '%S' "
                              "in term '%S' expected '%S'",
                              tmpname, tmpid,
                              obotag->Name, id, isaterm->Name);
            }
            else 
            {
                /* check for IDs */
                ajStrTokenAssignC(&idsplit, obotag->Value, " \t,;[]{}()'\"");

                while (ajStrTokenNextParse(&idsplit, &tmpstr))
                {
                    ajStrTrimEndC(&tmpstr, ".");

                    if(ajStrPrefixS(tmpstr, idprefix))
                        if(obovalididunk &&
                           !ajTableMatchS(ret->Termtable, tmpstr))
                            oboinWarn(obofile, obotag->Linenumber, NULL,
                                      "Unknown id '%S' for tag '%S' "
                                      "in term '%S'",
                                      tmpstr, obotag->Name, id);
                }
            }
        }
    }

    ajDebug("%Lu id prefixes current '%S'\n",
            ajListGetLength(idprefixlist), idprefix);

    iterpref = ajListIterNew(idprefixlist);
    found = ajFalse;

    while(!ajListIterDone(iterpref))
    {
        tmppref = ajListstrIterGet(iterpref);
        ajDebug("   '%S'\n", tmppref);
    }

    ajListIterDel(&iterpref);

    ajStrDel(&tmpid);
    ajStrDel(&tmpname);
    return ret; 
}




/* @section casts *************************************************************
**
** Retrieve data from an obo data opbject
**
** @fdata [AjPOboData]
**
** @nam3rule Fetch Fetch data
** @nam4rule Id  Fetch an obo term by id
** @nam4rule Name Fetch an obo term by name
**
** @argrule * thys [const AjPOboData] Obo data from parsed ontology
** @argrule * query [const AjPStr] Obo query string
**
** @valrule * [AjPObo] Obo term
**
** @fcategory cast
**
******************************************************************************/




/* @func ajObodataFetchId *****************************************************
**
** Retrieves an OBO term by ID
**
** @param [r] thys [const AjPOboData] Parsed ontology
** @param [r] query [const AjPStr] OBO identifier
** @return [AjPObo] OBO term
**
** @release 6.4.0
******************************************************************************/

AjPObo ajObodataFetchId(const AjPOboData thys, const AjPStr query)
{
    AjPObo ret;
    ajuint irecurs = 0;
    AjPStr tmpqry = NULL;
    AjBool isid = ajFalse;

    if(!thys)
        return NULL;

    if(ajStrPrefixC(query, "EDAM_identifier"))
    {
        isid = ajTrue;
        ajStrAssignS(&tmpqry, query);
        ajStrExchangeCC(&tmpqry, "EDAM_identifier:", "EDAM_data:");
        ret = ajTableFetchmodS(thys->Termtable, tmpqry);
        ajStrDel(&tmpqry);
    }
    else 
    {
        ret = ajTableFetchmodS(thys->Termtable, query);
    }

    while(ret && ajStrGetLen(ret->Trueid))
    {
        if(++irecurs > 256)
            return NULL;

        ret = ajTableFetchmodS(thys->Termtable, ret->Trueid);
    }

    if(isid)
    {
        ajStrExchangeCC(&ret->Id, "EDAM_data:", "EDAM_identifier:");
    }

    return ret;
}




/* @func ajObodataFetchName ***************************************************
**
** Retrieves an OBO term by name
**
** The name must be an exact match to a name or synonym for the term
**
** @param [r] thys [const AjPOboData] Parsed ontology
** @param [r] query [const AjPStr] OBO name
** @return [AjPObo] OBO term
**
** @release 6.4.0
******************************************************************************/

AjPObo ajObodataFetchName(const AjPOboData thys, const AjPStr query)
{
    AjPObo ret;
    ajuint irecurs = 0;

    if(!thys)
        return NULL;

    ret = ajTableFetchmodS(thys->Termnametable, query);

    while(ret && ajStrGetLen(ret->Trueid))
    {
        if(++irecurs > 256)
            return NULL;

        ret = ajTableFetchmodS(thys->Termtable, ret->Trueid);
    }

    return ret;
}




/* @datasection [AjPStr] Obo file line ****************************************
**
** Processing a line from an OBO format file
**
** @nam2rule Oboline
**
******************************************************************************/




/* @section Modifiers *********************************************************
**
** Clean up[ the input line
**
** @fdata [AjPStr]
**
** @nam3rule Cut Remove characters
** @nam4rule Comment Remove comment
** @nam4rule Dbxref Remove OBO cross reference
** @nam4rule Modifier Remove training modifiers
** @nam3rule Escape Process escape characters
**
** @argrule * Pline [AjPStr*] Input line
** @argrule Comment Pcomment [AjPStr*] Removed comment text
** @argrule Dbxref Pdbxref [AjPStr*] Removed dbxref text
** @argrule Modifier Pmodifier [AjPStr*] Removed modifier text
**
** @valrule * [AjBool] True if modified
**
** @fcategory modify
**
******************************************************************************/




/* @func ajObolineCutComment **************************************************
**
** Remove comments starting with an unescaped '!'
**
** @param [u] Pline [AjPStr*] Line with possible comment text
** @param [u] Pcomment [AjPStr*] Comment text
** @return [AjBool] True if a comment was found
**
** @release 6.4.0
******************************************************************************/

AjBool ajObolineCutComment(AjPStr *Pline, AjPStr *Pcomment)
{
    const char* cp;
    ajuint i      = 0;
    ajuint spaces = 0;
    ajuint ilen   = 0;
    AjBool doescape = ajFalse;

    ilen = ajStrGetLen(*Pline);
    cp = ajStrGetPtr(*Pline);

    while (*cp)
    {
        if(*cp == '!')
        {
            if(!doescape)
            {
                if((i+1) < ilen)
                {
                    ajStrAssignSubS(Pcomment, *Pline, i+1, -1);
                    ajStrTrimWhite(Pcomment);
                }
                else
                    ajStrAssignC(Pcomment, "");

                ajStrCutEnd(Pline, ilen-i+spaces);

                return ajTrue;
            }
        }
        
        if(*cp == '\\')
            doescape = ajTrue;
        else
        {
            doescape = ajFalse;

            if(isspace((int)*cp))
                spaces++;
            else
                spaces = 0;
        }
        i++;
        cp++;
    }
    
    ajStrAssignC(Pcomment, "");

    return ajFalse;
}




/* @func ajObolineCutDbxref ***************************************************
**
** Remove trailing dbxrefs in square braces
**
** @param [u] Pline [AjPStr*] Line with possible trailing dbxref text
** @param [u] Pdbxref [AjPStr*] Dbxref text
** @return [AjBool] True if a trailing dbxref was found
**
** @release 6.4.0
******************************************************************************/

AjBool ajObolineCutDbxref(AjPStr *Pline, AjPStr *Pdbxref)
{
    ajlong i = 0;
    ajulong ilen;

    ilen = ajStrGetLen(*Pline);

    if(ajStrGetCharLast(*Pline) != ']')
        return ajFalse;

    i = ajStrFindlastK(*Pline, '[');

    if(i < 0)
    {
        ajWarn("Failed to complete trailing dbxref: '%S'",
               *Pline);
        return ajFalse;
    }

    ajStrAssignSubS(Pdbxref, *Pline, i, -1);
    ajStrTrimWhite(Pdbxref);

    if(i)
    {
      ajStrCutEnd(Pline, (size_t) (ilen-i));
        ajStrTrimWhite(Pline);
    }
    else
        ajStrAssignC(Pline, "");
 
    return ajTrue;
}




/* @func ajObolineCutModifier *************************************************
**
** Remove trailing modifiers in curly braces
**
** @param [u] Pline [AjPStr*] Line with possible trailing modifier text
** @param [u] Pmodifier [AjPStr*] Modifier text
** @return [AjBool] True if a trailing modifier was found
**
** @release 6.4.0
******************************************************************************/

AjBool ajObolineCutModifier(AjPStr *Pline, AjPStr *Pmodifier)
{
    ajlong i = 0;
    ajulong ilen = 0;

    ajStrAssignC(Pmodifier, "");

    ilen = ajStrGetLen(*Pline);

    if(ajStrGetCharLast(*Pline) != '}')
        return ajFalse;

    i = ajStrFindlastK(*Pline, '{');

    if(i < 0)
    {
        ajWarn("Failed to complete trailing modifier: '%S'",
               *Pline);
        return ajFalse;
    }

    ajStrAssignSubS(Pmodifier, *Pline, i, -1);
    ajStrTrimWhite(Pmodifier);

    if(i)
    {
      ajStrCutEnd(Pline, (size_t) (ilen-i));
        ajStrTrimWhite(Pline);
    }
    else
        ajStrAssignC(Pline, "");
 
    return ajTrue;
}




/* @func ajObolineEscape ******************************************************
**
** Clean up escaped characters
**
** @param [u] Pline [AjPStr*] Line with possible comment text
** @return [AjBool] True if an escaped character was found
**
** @release 6.4.0
******************************************************************************/

AjBool ajObolineEscape(AjPStr *Pline)
{
    const char* cp;
    ajuint ilen = 0;

    AjPStr tmpstr = NULL;
    char lastchar = '\0';

    AjBool ret = ajFalse;

    ilen = ajStrGetLen(*Pline);
    cp = ajStrGetPtr(*Pline);
    tmpstr = ajStrNewRes(ilen+1);

    while (*cp)
    {
        if(lastchar == '\\')
        {
            ret = ajTrue;

            switch (*cp)
            {
                case '\n':      /* ignore the newline */
                    break;
                 case 'n':
                    ajStrAppendK(&tmpstr, '\n');
                    break;
                case 't':
                    ajStrAppendK(&tmpstr, '\t');
                    break;
                case 'W':
                    ajStrAppendK(&tmpstr, ' ');
                    break;
                default:        /* usually just copy the character */
                    ajStrAppendK(&tmpstr, *cp);
                    break;
            }
            
        }
        else if(*cp != '\\')
            ajStrAppendK(&tmpstr, *cp);

        lastchar = *cp++;
    }

    if(lastchar == '\\')        /* \ at the end means ignore the newline */
        ret = ajTrue;

    if(ret)
        ajStrAssignS(Pline, tmpstr);

    ajStrDel(&tmpstr);

    return ret;
}




/* @funcstatic obolineParseDef ************************************************
**
** Parse definition line and store results in Obo object. The raw text
** may include backslash escaped characters.
**
** The expected format is
**
** "definition text" [db:xref "optional desc", db:xref "desc2"]
** with name=value pairs possible after each dbxref or at the end
** (these are to be ignored).
**
** @param [u] obo [AjPObo] OBO term object
** @param [r] line [const AjPStr] Original definition line
** @return [AjBool] True if an escaped character was found
**
** @release 6.4.0
******************************************************************************/

static AjBool obolineParseDef(AjPObo obo, const AjPStr line)
{
    const char* cp;
    ajuint ilen = 0;

    AjPStr tmpstr = NULL;
    AjBool done;

    ilen = ajStrGetLen(line);
    cp = ajStrGetPtr(line);

    while (*cp && *cp != '"')
        cp++;

    if(!*cp)
        return ajFalse;

    cp++;

    tmpstr = ajStrNewRes(ilen+1);

    while (*cp && *cp != '"')
    {
        if(*cp == '\\')
        {
            cp++;
            switch (*cp)
            {
                case '\0':
                case '\n':      /* ignore the newline */
                    break;
                 case 'n':
                    ajStrAppendK(&tmpstr, '\n');
                    break;
                case 't':
                    ajStrAppendK(&tmpstr, '\t');
                    break;
                case 'W':
                    ajStrAppendK(&tmpstr, ' ');
                    break;
                default:        /* usually just copy the character */
                    ajStrAppendK(&tmpstr, *cp);
                    break;
            }
        }
        else
            ajStrAppendK(&tmpstr, *cp);

        cp++;
    }

    if(!*cp)
    {
        ajStrDel(&tmpstr);
        return ajFalse;
    }

    cp++;
    while (*cp && *cp == ' ')
        cp++;

    ajStrAssignS(&obo->Def, tmpstr);

    if(*cp == '[')
        cp++;

    done = ajFalse;
    while(!done)
    {
        ajStrAssignClear(&tmpstr);
        while (*cp && *cp != ' ')
        {
            if(*cp == '=')      /* name=value modifier */
            {
                done = ajTrue;
                break;
            }
            ajStrAssignK(&tmpstr, *cp++);
        }
        if(!*cp)
            done = ajTrue;
        else
            cp++;
    }
    
    ajStrDel(&tmpstr);

    return ajTrue;
}




/* @datasection [none] Miscellaneous functions ********************************
**
** Functions to initialise and clean up internals
**
** @nam2rule Oboin Obo input internals
**
******************************************************************************/




/* @section Printing **********************************************************
**
** Printing details of the internals to a file
**
** @fdata [none]
**
** @nam2rule Oboinprint
**
** @fcategory output
**
******************************************************************************/




/* @section Print *************************************************************
**
** Printing to a file
**
** @fdata [none]
**
** @nam3rule Book Print as docbook table
** @nam3rule Html Print as html table
** @nam3rule Wiki Print as wiki table
** @nam3rule Text Print as text
**
** @argrule * outf [AjPFile] output file
** @argrule Text full [AjBool] Print all details
**
** @valrule * [void]
**
** @fcategory cast
**
******************************************************************************/




/* @func ajOboinprintBook *****************************************************
**
** Reports the internal data structures as a Docbook table
**
** @param [u] outf [AjPFile] Output file
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajOboinprintBook(AjPFile outf)
{
    ajuint i = 0;
    ajuint j = 0;
    AjPStr namestr = NULL;
    AjPList fmtlist;
    AjPStr* names;

    fmtlist = ajListstrNew();

    ajFmtPrintF(outf, "<para>The supported obo term formats are summarised "
                "in the table below. "
                "The columns are as follows: "
                "<emphasis>Input format</emphasis> (format name), "
                "<emphasis>Try</emphasis> (indicates whether the "
                "format can be detected automatically on input), and "
                "<emphasis>Description</emphasis> (short description of "
                "the format).</para>\n\n");

    ajFmtPrintF(outf, "<table frame=\"box\" rules=\"cols\">\n");
    ajFmtPrintF(outf, "  <caption>Input obo term formats</caption>\n");
    ajFmtPrintF(outf, "  <thead>\n");
    ajFmtPrintF(outf, "    <tr align=\"center\">\n");
    ajFmtPrintF(outf, "      <th>Input Format</th>\n");
    ajFmtPrintF(outf, "      <th>Try</th>\n");
    ajFmtPrintF(outf, "      <th>Description</th>\n");
    ajFmtPrintF(outf, "    </tr>\n");
    ajFmtPrintF(outf, "  </thead>\n");
    ajFmtPrintF(outf, "  <tbody>\n");

    for(i=1; oboinFormatDef[i].Name; i++)
    {
	if(!oboinFormatDef[i].Alias)
        {
            namestr = ajStrNewC(oboinFormatDef[i].Name);
            ajListPushAppend(fmtlist, namestr);
            namestr = NULL;
        }
    }

    ajListSort(fmtlist, &ajStrVcmp);
    ajListstrToarray(fmtlist, &names);

    for(i=0; names[i]; i++)
    {
        for(j=0; oboinFormatDef[j].Name; j++)
        {
            if(ajStrMatchC(names[i],oboinFormatDef[j].Name))
            {
                ajFmtPrintF(outf, "    <tr>\n");
                ajFmtPrintF(outf, "      <td>%s</td>\n",
                            oboinFormatDef[j].Name);
                ajFmtPrintF(outf, "      <td>%B</td>\n",
                            oboinFormatDef[j].Try);
                ajFmtPrintF(outf, "      <td>%s</td>\n",
                            oboinFormatDef[j].Desc);
                ajFmtPrintF(outf, "    </tr>\n");
            }
        }
    }
        

    ajFmtPrintF(outf, "  </tbody>\n");
    ajFmtPrintF(outf, "</table>\n");
    ajStrDel(&namestr);

    names = NULL;
    ajListstrFreeData(&fmtlist);

    return;
}




/* @func ajOboinprintHtml *****************************************************
**
** Reports the internal data structures as an HTML table
**
** @param [u] outf [AjPFile] Output file
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajOboinprintHtml(AjPFile outf)
{
    ajuint i = 0;
    ajuint j = 0;

    AjPStr namestr = NULL;

    ajFmtPrintF(outf, "<table border=3>");
    ajFmtPrintF(outf, "<tr><th>Input Format</th><th>Auto</th>\n");
    ajFmtPrintF(outf, "<th>Multi</th><th>Description</th></tr>\n");

    for(i=1; oboinFormatDef[i].Name; i++)
    {
        ajStrAssignC(&namestr, oboinFormatDef[i].Name);

	if(!oboinFormatDef[i].Alias)
        {
            for(j=i+1; oboinFormatDef[j].Name; j++)
            {
                if(oboinFormatDef[j].Read == oboinFormatDef[i].Read)
                {
                    ajFmtPrintAppS(&namestr, " %s", oboinFormatDef[j].Name);
                    if(!oboinFormatDef[j].Alias) 
                    {
                        ajWarn("Input format '%s' same as '%s' but not alias",
                               oboinFormatDef[j].Name, oboinFormatDef[i].Name);
                    }
                }
            }

	    ajFmtPrintF(outf, "<tr><td>\n%S\n</td><td>%B</td>\n",
                        namestr,
			oboinFormatDef[i].Try);
            ajFmtPrintF(outf, "<td>\n%s\n</td></tr>\n",
			oboinFormatDef[i].Desc);
        }

    }

    ajFmtPrintF(outf, "</table>\n");
    ajStrDel(&namestr);

    return;
}




/* @func ajOboinprintText *****************************************************
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

void ajOboinprintText(AjPFile outf, AjBool full)
{
    ajuint i = 0;

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# Obo term input formats\n");
    ajFmtPrintF(outf, "# Name  Format name (or alias)\n");
    ajFmtPrintF(outf, "# Alias Alias name\n");
    ajFmtPrintF(outf, "# Try   Test for unknown input files\n");
    ajFmtPrintF(outf, "# Name         Alias Try "
		"Description");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "InFormat {\n");

    for(i=0; oboinFormatDef[i].Name; i++)
	if(full || !oboinFormatDef[i].Alias)
	    ajFmtPrintF(outf,
			"  %-12s %5B %3B \"%s\"\n",
			oboinFormatDef[i].Name,
			oboinFormatDef[i].Alias,
			oboinFormatDef[i].Try,
			oboinFormatDef[i].Desc);

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajOboinprintWiki *****************************************************
**
** Reports the internal data structures as a wiki table
**
** @param [u] outf [AjPFile] Output file
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajOboinprintWiki(AjPFile outf)
{
    ajuint i = 0;
    ajuint j = 0;

    AjPStr namestr = NULL;

    ajFmtPrintF(outf, "{| class=\"wikitable sortable\" border=\"2\"\n");
    ajFmtPrintF(outf, "|-\n");
    ajFmtPrintF(outf, "!Format!!Try!!"
                "class=\"unsortable\"|Description\n");

    for(i=1; oboinFormatDef[i].Name; i++)
    {
        ajStrAssignC(&namestr, oboinFormatDef[i].Name);

	if(!oboinFormatDef[i].Alias)
        {
            for(j=i+1; oboinFormatDef[j].Name; j++)
            {
                if(oboinFormatDef[j].Read == oboinFormatDef[i].Read)
                {
                    ajFmtPrintAppS(&namestr, "<br>%s", oboinFormatDef[j].Name);
                    if(!oboinFormatDef[j].Alias) 
                    {
                        ajWarn("Input format '%s' same as '%s' but not alias",
                               oboinFormatDef[j].Name, oboinFormatDef[i].Name);
                    }
                }
            }

            ajFmtPrintF(outf, "|-\n");
	    ajFmtPrintF(outf,
			"|%S||%B||%s\n",
			namestr,
			oboinFormatDef[i].Try,
			oboinFormatDef[i].Desc);
        }

    }

    ajFmtPrintF(outf, "|}\n\n");
    ajStrDel(&namestr);

    return;
}




/* @section Miscellaneous *****************************************************
**
** Functions to initialise and clean up internals
**
** @fdata [none]
**
** @nam3rule Exit Clean up and exit
**
** @valrule * [void]
**
** @fcategory misc
**
******************************************************************************/




/* @func ajOboinExit **********************************************************
**
** Cleans up obo term input internal memory
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajOboinExit(void)
{
    /* Query processing regular expressions */

    ajStrDel(&oboinReadLine);

    ajTableDel(&oboDbMethods);

    return;
}




/* @funcstatic oboinDebug *****************************************************
**
** Formatted write as a debug message.
**
** @param [r] obofile [const AjPFile] Input OBO file
** @param [r] linecnt [ajuint] Current line number
** @param [r] fmt [const char*] Format string
** @param [v] [...] Format arguments.
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void oboinDebug(const AjPFile obofile, ajuint linecnt,
                       const char* fmt, ...)
{
    va_list args;
    AjPStr errstr = NULL;

    oboErrorCount++;

    va_start(args, fmt) ;
    ajFmtVPrintS(&errstr, fmt, args);
    va_end(args) ;

    ajDebug("File %F line %u: %S\n", obofile, linecnt, errstr);
    ajStrDel(&errstr);

    return;
}




/* @funcstatic oboinWarn ******************************************************
**
** Formatted write as an error message.
**
** @param [r] obofile [const AjPFile] Input OBO file
** @param [r] linecnt [ajuint] Current line number
** @param [r] id [const AjPStr] Current term identifier
** @param [r] fmt [const char*] Format string
** @param [v] [...] Format arguments.
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void oboinWarn(const AjPFile obofile, ajuint linecnt, const AjPStr id,
                      const char* fmt, ...)
{
    va_list args;
    AjPStr errstr = NULL;

    oboErrorCount++;

    va_start(args, fmt) ;
    ajFmtVPrintS(&errstr, fmt, args);
    va_end(args) ;

    if(!ajStrGetLen(id))
        ajWarn("File %F line %u: %S", obofile, linecnt, errstr);
    else
        ajWarn("File %F term %S line %u: %S", obofile, id, linecnt, errstr);

    ajStrDel(&errstr);

    return;
}




/* @section Internals *********************************************************
**
** Functions to return internal values
**
** @fdata [none]
**
** @nam3rule Type Internals for obo term datatype
** @nam4rule Get  Return a value
** @nam5rule Fields  Known query fields for ajOboinRead
** @nam5rule Qlinks  Known query link operators for ajOboinRead
**
** @valrule * [const char*] Internal value
**
** @fcategory misc
**
******************************************************************************/




/* @func ajOboinTypeGetFields *************************************************
**
** Returns the listof known field names for ajOboinRead
**
** @return [const char*] List of field names
**
** @release 6.4.0
** @@
******************************************************************************/

const char* ajOboinTypeGetFields(void)
{
    return "id acc nam des";
}




/* @func ajOboinTypeGetQlinks *************************************************
**
** Returns the listof known query link operators for ajOboinRead
**
** @return [const char*] List of field names
**
** @release 6.4.0
** @@
******************************************************************************/

const char* ajOboinTypeGetQlinks(void)
{
    return "|";
}




/* @datasection [AjPTable] Internal call register table ***********************
**
** Functions to manage the internal call register table that links the
** ajaxdb library functions with code in the core AJAX library.
**
** @nam2rule Oboaccess Functions to manage obodb call tables.
**
******************************************************************************/




/* @section Cast **************************************************************
**
** Return a reference to the call table
**
** @fdata [AjPTable] Obodb functions call table
**
** @nam3rule Get Return a value
** @nam4rule Db Database access functions table
** @nam3rule Method Lookup an access method by name
** @nam4rule Test Return true if the access method exists
** @nam4rule MethodGet Return a method value
** @nam5rule Qlinks Return known query links for a named method
** @nam5rule Scope Return scope (entry, query or all) for a named method
**
** @argrule Method method [const AjPStr] Method name
**
** @valrule *Db [AjPTable] Call table of function names and references
** @valrule *Qlinks [const char*] Query link operators
** @valrule *Scope [ajuint] Scope flags
** @valrule *Test [AjBool] True if found
**
** @fcategory cast
**
******************************************************************************/




/* @func ajOboaccessGetDb *****************************************************
**
** returns the table in which obo database access details are registered
**
** @return [AjPTable] Access functions hash table
**
** @release 6.4.0
** @@
******************************************************************************/

AjPTable ajOboaccessGetDb(void)
{
    if(!oboDbMethods)
        oboDbMethods = ajCallTableNew();

    return oboDbMethods;
}





/* @func ajOboaccessMethodGetQlinks *******************************************
**
** Tests for a named method for obo term reading and returns the 
** known query link operators
**
** @param [r] method [const AjPStr] Method required.
** @return [const char*] Known link operators
**
** @release 6.4.0
** @@
******************************************************************************/

const char* ajOboaccessMethodGetQlinks(const AjPStr method)
{
    AjPOboAccess methoddata; 

    methoddata = ajCallTableGetS(oboDbMethods, method);
    if(!methoddata)
        return NULL;

    return methoddata->Qlink;
}




/* @func ajOboaccessMethodGetScope ********************************************
**
** Tests for a named method for obo term reading and returns the scope
** (entry, query or all).
*
** @param [r] method [const AjPStr] Method required.
** @return [ajuint] Scope flags
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ajOboaccessMethodGetScope(const AjPStr method)
{
    AjPOboAccess methoddata; 
    ajuint ret = 0;

    methoddata = ajCallTableGetS(oboDbMethods, method);
    if(!methoddata)
        return 0;

    if(methoddata->Entry)
        ret |= AJMETHOD_ENTRY;
    if(methoddata->Query)
        ret |= AJMETHOD_QUERY;
    if(methoddata->All)
        ret |= AJMETHOD_ALL;

    return ret;
}




/* @func ajOboaccessMethodTest ************************************************
**
** Tests for a named method for obo reading.
**
** @param [r] method [const AjPStr] Method required.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajOboaccessMethodTest(const AjPStr method)
{
    if(ajCallTableGetS(oboDbMethods, method))
      return ajTrue;

    return ajFalse;
}

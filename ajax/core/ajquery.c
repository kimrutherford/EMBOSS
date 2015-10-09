/* @source ajquery ************************************************************
**
** AJAX Query parsing functions
**
** @author Copyright (C) 2011 Peter Rice
** @version $Revision: 1.46 $
** @modified Jul 15 pmr First version with code from all datatypes merged
** @modified $Date: 2012/07/10 09:27:41 $ by $Author: rice $
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

#include "ajquery.h"
#include "ajcall.h"
#include "ajreg.h"
#include "ajtextread.h"
#include "ajnam.h"
#include "ajresource.h"

static AjBool    queryRegInitDone = AJFALSE;

static AjPStr queryFormat    = NULL;
static AjPStr queryList      = NULL;
static AjPStr querySvr       = NULL;
static AjPStr queryDb        = NULL;
static AjPStr queryChr       = NULL;
static AjPStr queryTest      = NULL;

static AjPRegexp queryRegAsis     = NULL;
static AjPRegexp queryRegHttp     = NULL;
static AjPRegexp queryRegFtp      = NULL;
static AjPRegexp queryRegSvr      = NULL;
static AjPRegexp queryRegDb       = NULL;
static AjPRegexp queryRegQryId    = NULL;
static AjPRegexp queryRegQryField = NULL;
static AjPRegexp queryRegFmt      = NULL;
static AjPRegexp queryRegFieldId  = NULL;
static AjPRegexp queryRegId       = NULL;
static AjPRegexp queryRegList     = NULL;
static AjPRegexp queryRegRange    = NULL;
static AjPRegexp queryRegRefrange = NULL;
static AjPRegexp queryRegWild     = NULL;

static void queryWildComp(void);
static void queryRegInit(void);
static const AjPStr queryGetFieldC(const AjPQuery query,
                                   const char* txt);
static AjBool queryDbfind(AjPStr *Pdb, const AjPStr querySvr,
                          AjBool findformat(const AjPStr format,
                                            ajint *iformat),
                          AjPQuery qry, AjBool* dbstat, AjBool* drcatstat);


static const char* queryDatatypeName[] =
{
    "unknown", "sequence", "features", "assembly",
    "obo", "resource", "taxon",
    "text", NULL
};




/* @filesection ajquery *******************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
******************************************************************************/





/* @datasection [AjPQuery] Query object ***************************************
**
** Function is for manipulating query objects
**
** @nam2rule Query
**
******************************************************************************/




/* @section Query Constructors ************************************************
**
** All constructors return a new query object by pointer. It
** is the responsibility of the user to first destroy any previous
** query object. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so
** anyway.
**
** @fdata [AjPQuery]
**
** @nam3rule New Constructor
** @argrule New datatype [const AjEDataType] Enumerated datatype
**
** @valrule * [AjPQuery]
**
** @fcategory new
**
******************************************************************************/




/* @func ajQueryNew ***********************************************************
**
** Creates a new query object for a specific datatype from the
** AJDATATYPE enumerated types
**
** @param [r] datatype [const AjEDataType] Enumerated datatype
** @return [AjPQuery] New query object.
**
** @release 6.4.0
** @@
******************************************************************************/

AjPQuery ajQueryNew(const AjEDataType datatype)
{
    AjPQuery pthis;

    AJNEW0(pthis);

    pthis->SvrName = ajStrNew();
    pthis->DbName = ajStrNew();
    pthis->DbAlias= ajStrNew();
    pthis->QueryFields = ajListNew();
    pthis->ResultsList = ajListNew();
    pthis->ResultsTable = ajTablestrNewConst(100);

    pthis->DbIdentifier = ajStrNew();
    pthis->DbAccession = ajStrNew();

    pthis->Wild   = ajFalse;

    pthis->Method      = ajStrNew();
    pthis->Formatstr   = ajStrNew();
    pthis->IndexDir    = ajStrNew();
    pthis->Directory   = ajStrNew();
    pthis->Filename    = ajStrNew();
    pthis->Application = ajStrNew();
    pthis->SingleField = ajStrNew();

    pthis->DbFilter = ajStrNew();
    pthis->DbReturn = ajStrNew();

    pthis->QueryType    = AJQUERY_UNKNOWN;
    pthis->Access  = NULL;
    pthis->DataType     = datatype;
    pthis->QryData = NULL;
    pthis->Fpos    = NULLFPOS;
    pthis->QryDone = ajFalse;
    pthis->HasAcc  = ajTrue;

    return pthis;
}




/* @section Query Destructors *************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the query object.
**
** @fdata [AjPQuery]
**
** @nam3rule Del destructor
**
** @argrule Del pthis [AjPQuery*] Query
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajQueryDel ***********************************************************
**
** Deletes a query object
**
** @param [d] pthis [AjPQuery*] Address of query object
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajQueryDel(AjPQuery* pthis)
{
    AjPQuery thys;
    AjPQueryField field = NULL;
    AjPTextAccess textaccess;

    if(!pthis)
        return;

    if(!*pthis)
        return;

    ajDebug("ajQueryDel db:'%S' svr: '%S' fields:%Lu\n",
            (*pthis)->DbName, (*pthis)->SvrName,
            ajListGetLength((*pthis)->QueryFields));

    thys = *pthis;

    ajStrDel(&thys->SvrName);
    ajStrDel(&thys->DbName);
    ajStrDel(&thys->DbAlias);
    ajStrDel(&thys->DbType);

    ajStrDel(&thys->DbIdentifier);
    ajStrDel(&thys->DbAccession);
    ajStrDel(&thys->DbFilter);
    ajStrDel(&thys->DbReturn);

    while(ajListPop(thys->QueryFields, (void **) &field))
        ajQueryfieldDel(&field);

    ajListFree(&thys->QueryFields);

    ajListFreeData(&thys->ResultsList);

    ajTableFree(&thys->ResultsTable);

    ajStrDel(&thys->Method);
    ajStrDel(&thys->Qlinks);
    ajStrDel(&thys->Formatstr);
    ajStrDel(&thys->IndexDir);
    ajStrDel(&thys->Directory);
    ajStrDel(&thys->Filename);
    ajStrDel(&thys->Exclude);
    ajStrDel(&thys->Namespace);
    ajStrDel(&thys->Organisms);
    ajStrDel(&thys->DbFields);
    ajStrDel(&thys->DbUrl);
    ajStrDel(&thys->DbProxy);
    ajStrDel(&thys->DbHttpVer);
    ajStrDel(&thys->ServerVer);
    ajStrDel(&thys->SingleField);
    ajStrDel(&thys->QryString);
    ajStrDel(&thys->QryFields);
    ajStrDel(&thys->Application);

    if(thys->QryData)
    {
        textaccess = thys->TextAccess;

        if(textaccess)
        {
            if(textaccess->AccessFree)
                (*textaccess->AccessFree)(thys);
        }
        
        
	AJFREE(thys->QryData);
    }

    AJFREE(*pthis);

    return;
}




/* @section Query Casts ***************************************************
**
** These functions examine the contents of a query object
** and return some derived information. Some of them provide access to
** the internal components of a query object. They are
** provided for programming convenience but should be used with
** caution.
**
** @fdata [AjPQuery]
**
** @nam3rule Get      Return a value
** @nam4rule Datatype Return name of datatype
** @nam4rule Id       Return id query term
** @nam4rule Format   Return format name
** @nam4rule Query    Return a report of the query string
** @nam3rule Getall   Return all values
** @nam4rule Fields   Return a set of standard query fields
**
** @argrule * query [const AjPQuery] Query object
** @argrule GetQuery Pdest [AjPStr*] Returned query string
**
**
** @valrule * [AjBool] True on success
** @valrule *Fields [const AjPList] List of field objects found
** @valrule *Format [const AjPStr] Format name
** @valrule *Id [const AjPStr] Wildcard ID
** @valrule *Datatype [const char*] Datatype standard name
**
** @fcategory cast
**
******************************************************************************/




/* @func ajQueryGetDatatype ***************************************************
**
** Returns the query datatype from a query
**
** @param [r] query [const AjPQuery] Query
** @return [const char*] Standard name for query datatype
**
** @release 6.4.0
******************************************************************************/

const char* ajQueryGetDatatype(const AjPQuery query)
{
    if(query->DataType > AJDATATYPE_MAX)
        return "invalid";

    return queryDatatypeName[query->DataType];
}




/* @func ajQueryGetFormat *****************************************************
**
** Returns the format name from a query
**
** @param [r] query [const AjPQuery] Query
** @return [const AjPStr] Format name
**
** @release 6.4.0
******************************************************************************/

const AjPStr ajQueryGetFormat(const AjPQuery query)
{
    return query->Formatstr;
}




/* @func ajQueryGetId *********************************************************
**
** Returns the ID query string from a query
**
** @param [r] query [const AjPQuery] Query
** @return [const AjPStr] Wildcard ID query string
**
** @release 6.4.0
******************************************************************************/

const AjPStr ajQueryGetId(const AjPQuery query)
{
    return queryGetFieldC(query, "id");
}




/* @func ajQueryGetQuery ******************************************************
**
** Returns a report of the query string from a query
**
** @param [r] query [const AjPQuery] Query
** @param [w] Pdest [AjPStr*] Query string
** @return [AjBool] True on success
**
** @release 6.4.0
******************************************************************************/

AjBool ajQueryGetQuery(const AjPQuery query, AjPStr *Pdest)
{
    AjIList iter = NULL;
    AjPQueryField field = NULL;

    const char* queryLinkNames[] = {"", "OR", "AND", "EOR", "NOT", "ELSE"};

    if(!Pdest)
        return ajFalse;

    ajStrAssignClear(Pdest);

    iter = ajListIterNewread(query->QueryFields);

    while(!ajListIterDone(iter))
    {
        field = ajListIterGet(iter);

        if(field->Link != AJQLINK_INIT)
            ajFmtPrintAppS(Pdest, " %s ", queryLinkNames[field->Link]);

        ajFmtPrintAppS(Pdest, "%S:%S", field->Field, field->Wildquery);
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @func ajQueryGetallFields **************************************************
**
** Returns the standard sequence query string from a query
**
** @param [r] query  [const AjPQuery] Query
** @return [const AjPList] List of field objects
**
** @release 6.4.0
******************************************************************************/

const AjPList ajQueryGetallFields(const AjPQuery query)
{
    return query->QueryFields;
}




/* @funcstatic queryGetFieldC *************************************************
**
** Returns the query string from a query for a named field
**
** @param [r] query [const AjPQuery] Query
** @param [r] txt [const char*] Field name
** @return [const AjPStr] Wildcard ID query string
**
** @release 6.4.0
******************************************************************************/

static const AjPStr queryGetFieldC(const AjPQuery query,
                                   const char* txt)
{
    AjPStr ret = NULL;
    AjIList iter = NULL;
    AjPQueryField field = NULL;

    iter = ajListIterNewread(query->QueryFields);

    while(!ajListIterDone(iter))
    {
        field = ajListIterGet(iter);

        if(ajStrMatchC(field->Field, txt))
        {
            ret = field->Wildquery;
            break;
        }
    }

    ajListIterDel(&iter);

    return ret;
}




/* @section Query Casts *******************************************************
**
** These functions use the contents of a query object but do
** not make any changes.
**
** @fdata [AjPQuery]
**
** @nam3rule Is Test for a property
** @nam3rule Known Test a property matches known options
** @nam4rule Set Query has some query field(s) defined
** @nam4rule KnownField Test field name is defined for the data source
** @suffix C Character string input
** @suffix S String input
**
** @argrule * thys [const AjPQuery] query
** @argrule C fieldtxt [const char*] Field name
** @argrule S field    [const AjPStr] Field name
**
** @valrule * [AjBool] True if property exists
**
** @fcategory cast
**
******************************************************************************/




/* @func ajQueryIsSet *********************************************************
**
** Tests whether any element of a query has been set.
**
** @param [r] thys [const AjPQuery] Query object.
** @return [AjBool] ajTrue if query should be made. ajFalse if the query
**                  includes all entries.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajQueryIsSet(const AjPQuery thys)
{

    ajDebug("ajQueryIsSet list:%Lu\n", ajListGetLength(thys->QueryFields));

    if(ajListGetLength(thys->QueryFields))
	return ajTrue;

    return ajFalse;
}




/* @func ajQueryKnownFieldC ***************************************************
**
** Checks whether a query field is defined for a database as a "fields:"
** string in the database definition.
**
** @param [r] thys [const AjPQuery] Query object
** @param [r] fieldtxt [const char*] field name
** @return [AjBool] ajTrue if the field is defined
**
** @release 6.4.0
******************************************************************************/

AjBool ajQueryKnownFieldC(const AjPQuery thys, const char* fieldtxt)
{

    AjPStrTok handle = NULL;
    AjPStr token     = NULL;

    ajDebug("ajQueryKnownFieldC qry '%s' fields '%S'\n",
            fieldtxt, thys->DbFields);

    ajStrTokenAssignC(&handle, thys->DbFields, "\t ,;\n\r");

    while(ajStrTokenNextParse(&handle, &token))
    {
	if(ajStrMatchCaseC(token, fieldtxt))
	{
	    ajDebug("ajQueryKnownFieldC match '%S'\n", token);
	    ajStrTokenDel(&handle);
	    ajStrDel(&token);

	    return ajTrue;
	}
    }

    ajStrTokenDel(&handle);
    ajStrDel(&token);

    if(ajCharMatchCaseC(fieldtxt, "id"))
        return ajTrue;

    if(thys->HasAcc && ajCharMatchCaseC(fieldtxt, "acc"))
        return ajTrue;

    return ajFalse;
}




/* @func ajQueryKnownFieldS ***************************************************
**
** Checks whether a query field is defined for a database as a "fields:"
** string in the database definition.
**
** @param [r] thys [const AjPQuery] Query object
** @param [r] field [const AjPStr] field name
** @return [AjBool] ajTrue if the field is defined
**
** @release 6.4.0
******************************************************************************/

AjBool ajQueryKnownFieldS(const AjPQuery thys, const AjPStr field)
{
    return ajQueryKnownFieldC(thys, ajStrGetPtr(field));
}




/* @section Modifiers *********************************************************
**
** Modify values in a query object
**
** @fdata [AjPQuery]
**
** @nam3rule Add       Add content
** @nam4rule AddField  Add a query field
** @nam5rule And       Add an AND query field
** @nam5rule Else      Add an ELSE query field
** @nam5rule Eor       Add an EOR query field
** @nam5rule Not       Add a NOT query field
** @nam5rule Or        Add an OR query field
** @nam3rule Set       Set internals
** @nam4rule Wild      Set wildcard property
** @nam3rule Clear     Reset for reuse
** @nam3rule Starclear Reset all fields that are just '*' for 'match all'
** @suffix   C         Character string data
** @suffix   S         String data
**
** @argrule * thys         [AjPQuery]     Query object
** @argrule C fieldtxt     [const char*]  Field name
** @argrule C wildquerytxt [const char*]  Query string
** @argrule S field        [const AjPStr] Field name
** @argrule S wildquery    [const AjPStr] Query string
**
** @valrule * [void]
** @valrule *AddField [AjBool] True on success
** @valrule *Wild [AjBool] True if query had wildcard(s)
**
** @fcategory modify
**
******************************************************************************/




/* @func ajQueryAddFieldAndC **************************************************
**
** Adds a query with an 'AND' operator
**
** @param [u] thys [AjPQuery] Query object
** @param [r] fieldtxt [const char*] field name
** @param [r] wildquerytxt [const char*] wildcard query string
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
******************************************************************************/

AjBool ajQueryAddFieldAndC(AjPQuery thys, const char* fieldtxt,
                           const char* wildquerytxt)
{
    AjPQueryField qryfield;

    qryfield = ajQueryfieldNewC(fieldtxt, wildquerytxt, AJQLINK_AND);

    if(!thys->QueryFields)
        thys->QueryFields = ajListNew();

    ajListPushAppend(thys->QueryFields, qryfield);
    qryfield = NULL;

    return ajTrue;
}




/* @func ajQueryAddFieldAndS **************************************************
**
** Adds a query with an 'AND' operator
**
** @param [u] thys [AjPQuery] Query object
** @param [r] field [const AjPStr] field name
** @param [r] wildquery [const AjPStr] wildcard query string
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
******************************************************************************/

AjBool ajQueryAddFieldAndS(AjPQuery thys, const AjPStr field,
                           const AjPStr wildquery)
{
    AjPQueryField qryfield;

    qryfield = ajQueryfieldNewS(field, wildquery, AJQLINK_AND);

    if(!thys->QueryFields)
        thys->QueryFields = ajListNew();

    ajListPushAppend(thys->QueryFields, qryfield);
    qryfield = NULL;

    return ajTrue;
}





/* @func ajQueryAddFieldElseC *************************************************
**
** Adds a query with an 'ELSE' operator
**
** @param [u] thys [AjPQuery] Query object
** @param [r] fieldtxt [const char*] field name
** @param [r] wildquerytxt [const char*] wildcard query string
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
******************************************************************************/

AjBool ajQueryAddFieldElseC(AjPQuery thys, const char* fieldtxt,
                            const char* wildquerytxt)
{
    AjPQueryField qryfield;

    qryfield = ajQueryfieldNewC(fieldtxt, wildquerytxt, AJQLINK_ELSE);

    if(!thys->QueryFields)
        thys->QueryFields = ajListNew();

    ajListPushAppend(thys->QueryFields, qryfield);
    qryfield = NULL;

    return ajTrue;
}




/* @func ajQueryAddFieldElseS *************************************************
**
** Adds a query with an 'ELSE' operator
**
** @param [u] thys [AjPQuery] Query object
** @param [r] field [const AjPStr] field name
** @param [r] wildquery [const AjPStr] wildcard query string
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
******************************************************************************/

AjBool ajQueryAddFieldElseS(AjPQuery thys, const AjPStr field,
                            const AjPStr wildquery)
{
    AjPQueryField qryfield;

    qryfield = ajQueryfieldNewS(field, wildquery, AJQLINK_ELSE);

    if(!thys->QueryFields)
        thys->QueryFields = ajListNew();

    ajListPushAppend(thys->QueryFields, qryfield);
    qryfield = NULL;

    return ajTrue;
}





/* @func ajQueryAddFieldEorC **************************************************
**
** Adds a query with an 'EOR' operator
**
** @param [u] thys [AjPQuery] Query object
** @param [r] fieldtxt [const char*] field name
** @param [r] wildquerytxt [const char*] wildcard query string
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
******************************************************************************/

AjBool ajQueryAddFieldEorC(AjPQuery thys, const char* fieldtxt,
                            const char* wildquerytxt)
{
    AjPQueryField qryfield;

    qryfield = ajQueryfieldNewC(fieldtxt, wildquerytxt, AJQLINK_EOR);

    if(!thys->QueryFields)
        thys->QueryFields = ajListNew();

    ajListPushAppend(thys->QueryFields, qryfield);
    qryfield = NULL;

    return ajTrue;
}




/* @func ajQueryAddFieldEorS **************************************************
**
** Adds a query with an 'EOR' operator
**
** @param [u] thys [AjPQuery] Query object
** @param [r] field [const AjPStr] field name
** @param [r] wildquery [const AjPStr] wildcard query string
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
******************************************************************************/

AjBool ajQueryAddFieldEorS(AjPQuery thys, const AjPStr field,
                            const AjPStr wildquery)
{
    AjPQueryField qryfield;

    qryfield = ajQueryfieldNewS(field, wildquery, AJQLINK_EOR);

    if(!thys->QueryFields)
        thys->QueryFields = ajListNew();

    ajListPushAppend(thys->QueryFields, qryfield);
    qryfield = NULL;

    return ajTrue;
}





/* @func ajQueryAddFieldNotC **************************************************
**
** Adds a query with a 'NOT' operator
**
** @param [u] thys [AjPQuery] Query object
** @param [r] fieldtxt [const char*] field name
** @param [r] wildquerytxt [const char*] wildcard query string
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
******************************************************************************/

AjBool ajQueryAddFieldNotC(AjPQuery thys, const char* fieldtxt,
                           const char* wildquerytxt)
{
    AjPQueryField qryfield;

    qryfield = ajQueryfieldNewC(fieldtxt, wildquerytxt, AJQLINK_NOT);

    if(!thys->QueryFields)
        thys->QueryFields = ajListNew();

    ajListPushAppend(thys->QueryFields, qryfield);
    qryfield = NULL;

    return ajTrue;
}




/* @func ajQueryAddFieldNotS **************************************************
**
** Adds a query with a 'NOT' operator
**
** @param [u] thys [AjPQuery] Query object
** @param [r] field [const AjPStr] field name
** @param [r] wildquery [const AjPStr] wildcard query string
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
******************************************************************************/

AjBool ajQueryAddFieldNotS(AjPQuery thys, const AjPStr field,
                           const AjPStr wildquery)
{
    AjPQueryField qryfield;

    qryfield = ajQueryfieldNewS(field, wildquery, AJQLINK_NOT);

    if(!thys->QueryFields)
        thys->QueryFields = ajListNew();

    ajListPushAppend(thys->QueryFields, qryfield);
    qryfield = NULL;

    return ajTrue;
}





/* @func ajQueryAddFieldOrC ***************************************************
**
** Adds a query with an 'OR' operator
**
** @param [u] thys [AjPQuery] Query object
** @param [r] fieldtxt [const char*] field name
** @param [r] wildquerytxt [const char*] wildcard query string
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
******************************************************************************/

AjBool ajQueryAddFieldOrC(AjPQuery thys, const char* fieldtxt,
                        const char* wildquerytxt)
{
    AjPQueryField qryfield;

    
    if(!thys->QueryFields)
        thys->QueryFields = ajListNew();
    
    if(!ajListGetLength(thys->QueryFields))
        qryfield = ajQueryfieldNewC(fieldtxt, wildquerytxt, AJQLINK_INIT);
    else
        qryfield = ajQueryfieldNewC(fieldtxt, wildquerytxt, AJQLINK_OR);

    ajListPushAppend(thys->QueryFields, qryfield);
    qryfield = NULL;

    return ajTrue;
}




/* @func ajQueryAddFieldOrS ***************************************************
**
** Adds a query with an 'OR' operator
**
** @param [u] thys [AjPQuery] Query object
** @param [r] field [const AjPStr] field name
** @param [r] wildquery [const AjPStr] wildcard query string
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
******************************************************************************/

AjBool ajQueryAddFieldOrS(AjPQuery thys, const AjPStr field,
                          const AjPStr wildquery)
{
    AjPQueryField qryfield;

    if(!thys->QueryFields)
        thys->QueryFields = ajListNew();

    if(!ajListGetLength(thys->QueryFields))
        qryfield = ajQueryfieldNewS(field, wildquery, AJQLINK_INIT);
    else
        qryfield = ajQueryfieldNewS(field, wildquery, AJQLINK_OR);

    ajListPushAppend(thys->QueryFields, qryfield);
    qryfield = NULL;

    return ajTrue;
}





/* @func ajQueryClear *********************************************************
**
** Resets a query object to a clean state for reuse.
** Keep the begin, end and reverse values.
**
** @param [u] thys [AjPQuery] query object
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajQueryClear(AjPQuery thys)
{
    AjPQueryField field = NULL;
    void *result;

    ajStrSetClear(&thys->SvrName);
    ajStrSetClear(&thys->DbName);
    ajStrSetClear(&thys->DbAlias);
    ajStrSetClear(&thys->DbType);

    while(ajListPop(thys->QueryFields, (void **) &field))
        ajQueryfieldDel(&field);

    while(ajListPop(thys->ResultsList, (void **) &result))
        continue;

    ajTableClear(thys->ResultsTable);
    ajStrSetClear(&thys->Method);
    ajStrSetClear(&thys->Qlinks);
    ajStrSetClear(&thys->Formatstr);
    ajStrSetClear(&thys->IndexDir);
    ajStrSetClear(&thys->Directory);
    ajStrSetClear(&thys->Filename);
    ajStrSetClear(&thys->Exclude);
    ajStrSetClear(&thys->Namespace);
    ajStrSetClear(&thys->Organisms);
    ajStrSetClear(&thys->DbFields);
    ajStrSetClear(&thys->DbUrl);
    ajStrSetClear(&thys->DbProxy);
    ajStrSetClear(&thys->DbHttpVer);
    ajStrSetClear(&thys->ServerVer);
    ajStrSetClear(&thys->SingleField);
    ajStrSetClear(&thys->QryString);
    ajStrSetClear(&thys->QryFields);
    ajStrSetClear(&thys->Application);

    ajStrSetClear(&thys->DbIdentifier);
    ajStrSetClear(&thys->DbAccession);
    ajStrSetClear(&thys->DbFilter);
    ajStrSetClear(&thys->DbReturn);

    thys->Fpos = 0L;

    thys->TextAccess = NULL;
    thys->Access = NULL;

    if(thys->QryData)
	AJFREE(thys->QryData);

    thys->QueryType   = AJQUERY_UNKNOWN;
    /*thys->DataType   = AJDATATYPE_UNKNOWN;*/

    thys->QryDone = ajFalse;
    thys->SetServer = ajFalse;
    thys->SetDatabase = ajFalse;
    thys->SetQuery = ajFalse;
    thys->InDrcat = ajFalse;

    thys->Wild = ajFalse;
    thys->CaseId = ajFalse;
    thys->HasAcc = ajTrue;      /* default true in ajQueryNew */

    return;
}




/* @func ajQuerySetWild *******************************************************
**
** Tests whether a query includes wild cards in any element,
** or can return more than one entry (keyword and some other search terms
** will find multiple entries)
**
** @param [u] thys [AjPQuery] Query object.
** @return [AjBool] ajTrue if query had wild cards.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajQuerySetWild(AjPQuery thys)
{
    AjIList iter = NULL;
    AjPQueryField field = NULL;
    AjBool ret = ajFalse;

    ajuint ifield = 0;

    if(!queryRegWild)
	queryWildComp();

    ajDebug("ajQuerySetWild fields: %Lu\n",
	    ajListGetLength(thys->QueryFields));

    /* first test for ID query */
    iter = ajListIterNewread(thys->QueryFields);

    while(!ajListIterDone(iter))
    {
        field = ajListIterGet(iter);
        ajDebug("test1 query %S '%S' %u\n",
                field->Field, field->Wildquery, field->Link);

        if(ajStrMatchC(field->Field, "id"))
        {
            if(ajRegExec(queryRegWild, field->Wildquery))
            {
                thys->Wild = ajTrue;
                ajDebug("wild id query %S '%S'\n",
                        field->Field, field->Wildquery);

                ajListIterDel(&iter);
                return ajTrue;
            }

            break;
        }
    }

    ajListIterRewind(iter);

    /* now test the other fields */
    while(!ajListIterDone(iter))
    {
        field = ajListIterGet(iter);
        ajDebug("test2 query %S '%S'\n",
                field->Field, field->Wildquery);

        if(ifield++ && field->Link != AJQLINK_ELSE) /* field [|&!^] field */
        {
            ajListIterDel(&iter);
            return ajTrue;
        }
        
        if(ajStrMatchC(field->Field, "id"))
            continue;

        if(ajRegExec(queryRegWild, field->Wildquery))
        {
            thys->Wild = ajTrue;
            ajDebug("wild query %S '%S'\n",
                    field->Field, field->Wildquery);

            ajListIterDel(&iter);
            return ajTrue;
        }

        if(!ajStrMatchC(field->Field, "acc"))
            ret = ajTrue;
    }

    ajListIterDel(&iter);

    ajDebug("wildcard in stored query: %B\n", ret);

    return ret;
}




/* @func ajQueryStarclear *****************************************************
**
** Clears elements of a query object if they are simply "*" because this
** is equivalent to a null string.
**
** @param [u] thys [AjPQuery] Query object.
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajQueryStarclear(AjPQuery thys)
{

    AjIList iter = NULL;
    AjPQueryField field = NULL;

    iter = ajListIterNew(thys->QueryFields);

    while(!ajListIterDone(iter))
    {
        field = ajListIterGet(iter);

        if(ajStrMatchC(field->Wildquery, "*"))
        {
            ajDebug("ajQueryStarclear keep field %S '%S'\n",
                    field->Field, field->Wildquery);
            /*
            ajQueryfieldDel(&field);
	    ajListIterRemove(iter);
            */
        }
    }

    ajListIterDel(&iter);

    return;
}




/* @section Debug *************************************************************
**
** Reports sequence contents for debugging purposes
**
** @fdata [AjPQuery]
** @fcategory misc
**
** @nam3rule Trace    Print report to debug file (if any)
** @nam4rule TraceTitle  Print report to debug file (if any) with title
**
** @argrule * thys [const AjPQuery] query object.
**
** @valrule * [void]
**
******************************************************************************/




/* @func ajQueryTrace *********************************************************
**
** Debug calls to trace the data in a query object.
**
** @param [r] thys [const AjPQuery] query object.
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajQueryTrace(const AjPQuery thys)
{
    AjIList iter = NULL;
    AjPQueryField field = NULL;

    const char* operators[] =
    {
        "init",
        "OR", "AND", "EOR", "NOT",
        "ELSE"
    };
    
    ajDebug( "  Query Trace\n");

    if(ajStrGetLen(thys->SvrName))
	ajDebug( "   SvrName: '%S'\n", thys->SvrName);

    if(ajStrGetLen(thys->DbName))
	ajDebug( "    DbName: '%S'\n", thys->DbName);

    if(ajStrGetLen(thys->DbAlias))
	ajDebug( "    DbAlias: '%S'\n", thys->DbAlias);

    if(ajStrGetLen(thys->DbType))
	ajDebug( "    DbType: '%S' (%d)\n", thys->DbType, thys->QueryType);

    iter = ajListIterNewread(thys->QueryFields);

    while(!ajListIterDone(iter))
    {
        field = ajListIterGet(iter);

        if(field->Link < AJQLINK_MAX)
            ajDebug( "    %S: '%S' %d (%s)\n",
                     field->Field, field->Wildquery,
                     field->Link, operators[field->Link]);
        else
            ajDebug( "    %S: '%S' %d\n",
                     field->Field, field->Wildquery,
                     field->Link);
    }

    ajListIterDel(&iter);

    ajDebug( "    Case-sensitive Id: '%B'\n", thys->CaseId);
    ajDebug( "   Has accession: %B\n", thys->HasAcc);

    if(ajStrGetLen(thys->Method))
	ajDebug( "    Method: '%S'\n", thys->Method);

    if(ajStrGetLen(thys->Formatstr))
	ajDebug( "    Formatstr: '%S'\n", thys->Formatstr);

    if(ajStrGetLen(thys->IndexDir))
	ajDebug( "    IndexDir: '%S'\n", thys->IndexDir);

    if(ajStrGetLen(thys->Directory))
	ajDebug( "    Directory: '%S'\n", thys->Directory);

    if(ajStrGetLen(thys->Filename))
	ajDebug( "    Filename: '%S'\n", thys->Filename);

    if(ajStrGetLen(thys->Exclude))
	ajDebug( "    Exclude: '%S'\n", thys->Exclude);

    if(ajStrGetLen(thys->Namespace))
	ajDebug( "    Namespace: '%S'\n", thys->Namespace);

    if(ajStrGetLen(thys->Organisms))
	ajDebug( "    Organisms: '%S'\n", thys->Organisms);

    if(ajStrGetLen(thys->DbFields))
	ajDebug( "    DbFields: '%S'\n", thys->DbFields);

    if(ajStrGetLen(thys->DbIdentifier))
	ajDebug( "    DbIdentifier: '%S'\n", thys->DbIdentifier);

    if(ajStrGetLen(thys->DbAccession))
	ajDebug( "    DbAccession: '%S'\n", thys->DbAccession);

    if(ajStrGetLen(thys->DbFilter))
	ajDebug( "    DbFilter: '%S'\n", thys->DbFilter);

    if(ajStrGetLen(thys->DbReturn))
	ajDebug( "    DbReturn: '%S'\n", thys->DbReturn);

    if(ajStrGetLen(thys->DbUrl))
	ajDebug( "    DbUrl: '%S'\n", thys->DbUrl);

    if(ajStrGetLen(thys->DbProxy))
	ajDebug( "    DbProxy: '%S'\n", thys->DbProxy);

    if(ajStrGetLen(thys->DbHttpVer))
	ajDebug( "    DbHttpVer: '%S'\n", thys->DbHttpVer);

    if(ajStrGetLen(thys->SingleField))
	ajDebug( "    SingleField: '%S'\n", thys->SingleField);

    if(ajStrGetLen(thys->QryString))
	ajDebug( "    QryString: '%S'\n", thys->QryString);

    if(ajStrGetLen(thys->QryFields))
	ajDebug( "    QryFields: '%S'\n", thys->QryFields);

    if(ajStrGetLen(thys->Application))
	ajDebug( "    Application: '%S'\n", thys->Application);

    ajDebug( "   Fpos: %Ld\n", thys->Fpos);
    ajDebug( "   QryDone: %B\n", thys->QryDone);
    ajDebug( "   Wildcard in query: %B\n", thys->Wild);

    if(thys->Access)
	ajDebug( "    Access: exists\n");

    if(thys->QryData)
	ajDebug( "    QryData: exists\n");

    return;
}




/* @section exit **************************************************************
**
** Functions called on exit from the program by ajExit to do
** any necessary cleanup and to report internal statistics to the debug file
**
** @fdata      [AjPQuery]
** @fnote     general exit functions, no arguments
**
** @nam3rule Exit Cleanup and report on exit
**
** @valrule * [void]
**
** @fcategory misc
******************************************************************************/




/* @func ajQueryExit **********************************************************
**
** Cleans up query processing internal memory
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajQueryExit(void)
{
    ajStrDel(&queryFormat);
    ajStrDel(&queryList);
    ajStrDel(&querySvr);
    ajStrDel(&queryDb);
    ajStrDel(&queryChr);
    ajStrDel(&queryTest);

    ajRegFree(&queryRegAsis);
    ajRegFree(&queryRegHttp);
    ajRegFree(&queryRegFtp);
    ajRegFree(&queryRegSvr);
    ajRegFree(&queryRegDb);
    ajRegFree(&queryRegFmt);
    ajRegFree(&queryRegFieldId);
    ajRegFree(&queryRegId);
    ajRegFree(&queryRegList);
    ajRegFree(&queryRegQryId);
    ajRegFree(&queryRegQryField);
    ajRegFree(&queryRegRange);
    ajRegFree(&queryRegRefrange);

    ajRegFree(&queryRegWild);

    return;
}




/* @datasection [AjPQueryField] Query fields **********************************
**
** Query field processing
**
** @nam2rule Queryfield
**
******************************************************************************/




/* @section constructors ******************************************************
**
** Constructors
**
** @fdata [AjPQueryField]
**
** @nam3rule New Constructor
** @suffix C Character string data
** @suffix S String data
**
** @argrule C fieldtxt [const char*]  Field name
** @argrule C wildtxt  [const char*]  Wildcard query string
** @argrule S field    [const AjPStr] Field name
** @argrule S wild     [const AjPStr] Wildcard query string
** @argrule New oper [AjEQryLink] Operator
**
** @valrule * [AjPQueryField] Query field object
**
** @fcategory new
**
******************************************************************************/




/* @func ajQueryfieldNewC *****************************************************
**
** Constructor for a query field
**
** @param [r] fieldtxt [const char*] Field name
** @param [r] wildtxt [const char*] Wildcard query string
** @param [r] oper [AjEQryLink] Operator
**
** @return [AjPQueryField] Query field
**
** @release 6.4.0
******************************************************************************/

AjPQueryField ajQueryfieldNewC(const char* fieldtxt,
                               const char* wildtxt,
                               AjEQryLink oper)
{
    AjPQueryField ret;

    AJNEW0(ret);

    ajDebug("ajQueryfieldNewC '%s' '%s'\n", fieldtxt, wildtxt);

    ret->Field = ajStrNewC(fieldtxt);
    ret->Wildquery = ajStrNewC(wildtxt);
    ret->Link = oper;

    ajStrFmtLower(&ret->Field);

    return ret;
}




/* @func ajQueryfieldNewS *****************************************************
**
** Constructor for a query field
**
** @param [r] field [const AjPStr] Field name
** @param [r] wild [const AjPStr] Wildcard query string
** @param [r] oper [AjEQryLink] Operator
**
** @return [AjPQueryField] Query field
**
** @release 6.4.0
******************************************************************************/

AjPQueryField ajQueryfieldNewS(const AjPStr field,
                               const AjPStr wild,
                               AjEQryLink oper)
{
    AjPQueryField ret;

    AJNEW0(ret);

    ajDebug("ajQueryfieldNewS '%S' '%S'\n", field, wild);

    ret->Field = ajStrNewS(field);
    ret->Wildquery = ajStrNewS(wild);
    ret->Link = oper;

    ajStrFmtLower(&ret->Field);

    return ret;
}




/* @section Query Field Destructors *******************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the query field object.
**
** @fdata [AjPQueryField]
**
** @nam3rule Del destructor
**
** @argrule Del Pthis [AjPQueryField*] Query field
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajQueryfieldDel ******************************************************
**
** Destructor for a query field
**
** @param [d] Pthis [AjPQueryField*] Query field object to be deleted
**
** @return [void]
**
** @release 6.4.0
******************************************************************************/

void ajQueryfieldDel(AjPQueryField *Pthis)
{
    AjPQueryField thys;

    if(!Pthis) return;
    if(!*Pthis) return;

    thys = *Pthis;
    ajStrDel(&thys->Field);
    ajStrDel(&thys->Wildquery);

    AJFREE(*Pthis);

    return;
}




/* @datasection [AjPList] Query field list ************************************
**
** Query fields lists are handled internally. Only static functions
** should appear here
**
** @nam2rule Querylist Query list processing
**
******************************************************************************/




/* @section Debug *************************************************************
**
** Reports sequence contents for debugging purposes
**
** @fdata [AjPList]
** @fcategory misc
**
** @nam3rule Trace    Print report to debug file (if any)
** @nam4rule TraceTitle  Print report to debug file (if any) with title
**
** @argrule * list [const AjPList] query list
**
** @valrule * [void]
**
******************************************************************************/




/* @func ajQuerylistTrace *****************************************************
**
** Traces the nodes in a query list
**
** @param [r] list [const AjPList] The query list
** @return [void]
**
** @release 6.4.0
******************************************************************************/

void ajQuerylistTrace(const AjPList list)
{
    AjIList iter;
    AjPQueryList node;
    ajuint i = 0;

    iter = ajListIterNewread(list);

    ajDebug("ajQuerylistTrace %Lu nodes\n", ajListGetLength(list));

    while(!ajListIterDone(iter))
    {
	node = (AjPQueryList) ajListIterGet(iter);
	ajDebug("%3d: '%S' '%S' %d\n",
		++i, node->Qry,
		node->Formatstr, node->Format);
    }

    ajListIterDel(&iter);
    ajDebug("...Done...\n");

    return;
}




/* @funcstatic queryWildComp **************************************************
**
** Compiles the regular expressions for testing wild cards in queries.
** These are held in static storage and built once only if needed.
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void queryWildComp(void)
{
    if(!queryRegWild)
	queryRegWild = ajRegCompC("[*?]");

    return;
}




/* @datasection [AjPStr] Query string *****************************************
**
** Function is for manipulating query strings
**
** @nam2rule Querystr
**
******************************************************************************/




/* @section Query string parsing **********************************************
**
** Parses a query string and removes the processed string trokens
**
** @fdata [AjPStr]
** @fcategory modify
**
** @nam3rule Parse Parse the string
** @nam4rule ParseFormat Parse the format prefix
** @nam4rule ParseListfile Parse the listfile prefix
** @nam4rule ParseRange Parse the range suffix
** @nam4rule ParseRead Parse a text query and read data
**
** @argrule Parse Pqry [AjPStr*]  Query string
** @argrule Format textin [AjPTextin]  Text input object
** @argrule Format findformat [AjBool function] Function to validate
**                                              format name
** @argrule Range Pbegin [ajint*]  Begin position
** @argrule Range Pend [ajint*]  End position
** @argrule Range Prev [AjBool*]  Reverse orientation
** @argrule Read textin [AjPTextin]  Text input object
** @argrule Read findformat [AjBool function] Function to validate format name
** @argrule Read Pnontext [AjBool*] True if access is a non-text method
**                                  to be processed by the caller
**
** @valrule * [AjBool] True if found
** @valrule *Format [const AjPStr] Format name if found
**
******************************************************************************/




/* @func ajQuerystrParseFormat ************************************************
**
** Parses a query (USA, UFO or general query).
**
** Then tests for "format::" and returns this if it is found,
** removing the format part of the original query.
**
** @param [u] Pqry [AjPStr*]  Query string
** @param [u] textin [AjPTextin]  Text input object
** @param [f] findformat [AjBool function] Function to validate format name
** @return [const AjPStr] Format name if found
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPStr ajQuerystrParseFormat(AjPStr *Pqry, AjPTextin textin,
                                   AjBool findformat(const AjPStr format,
                                                     ajint *iformat))
{
    AjBool fmtstat   = ajFalse;	 /* status returns from regex tests */
    AjBool asisstat  = ajFalse;
    AjBool liststat  = ajFalse;
    AjPQuery qry = textin->Query;

    ajDebug("++ajQuerystrParseFormat '%S'\n", *Pqry);

    if(!queryRegInitDone)
        queryRegInit();

    /* Strip any leading spaces */
    ajStrTrimC(Pqry," \t\n");

    asisstat = ajRegExec(queryRegAsis, *Pqry);
    liststat = ajRegExec(queryRegList, *Pqry);

    if(liststat || asisstat)
        return NULL;

    fmtstat = ajRegExec(queryRegFmt, *Pqry);

    if(!fmtstat)
    {
        if(ajStrGetLen(textin->Formatstr))
        {
            if(findformat(textin->Formatstr, &textin->Format))
                ajStrAssignS(&qry->Formatstr, textin->Formatstr);
        }
	return NULL;
    }

    ajRegSubI(queryRegFmt, 1, &queryFormat);
    ajRegSubI(queryRegFmt, 2, Pqry);
    ajDebug("found format %S rest '%S'\n", queryFormat, *Pqry);

    ajStrAssignEmptyC(&queryFormat, "unknown");

    if(findformat(queryFormat, &textin->Format))
    {
        ajStrAssignS(&qry->Formatstr, queryFormat);
        ajStrAssignS(&textin->Formatstr, queryFormat);
    }
    else
        ajErr("Unknown input format '%S'\n", queryFormat);

    return queryFormat;
}




/* @func ajQuerystrParseListfile **********************************************
**
** Parses a query (USA, UFO or general query).
**
** Then tests for "list::" or "@" and returns true if found,
** removing the filename only in the origiunal query.
**
** @param [u] Pqry [AjPStr*]  Query string
** @return [AjBool] True if found
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajQuerystrParseListfile(AjPStr *Pqry)
{
    AjBool liststat   = ajFalse;	 /* status returns from regex tests */

    ajDebug("++ajQuerystrParseListfile '%S'\n", *Pqry);

    if(!queryRegInitDone)
        queryRegInit();

    /* Strip any leading spaces */
    ajStrTrimC(Pqry," \t\n");

    ajStrAssignS(&queryTest, *Pqry);

    liststat = ajRegExec(queryRegList, queryTest);

    if(liststat)
    {
	ajRegSubI(queryRegList, 2, Pqry);
	ajDebug("list found @%S\n", *Pqry);
    }

    return liststat;
}




/* @func ajQuerystrParseRange *************************************************
**
** Parses a query (USA, UFO or general query).
**
** Tests for "[n:n:r]" range and sets this if it is found,
** removing the range part of the original query.
**
** @param [u] Pqry [AjPStr*]  Query string
** @param [u] Pbegin [ajint*]  Begin position
** @param [u] Pend [ajint*]  End position
** @param [u] Prev [AjBool*]  Reverse orientation
** @return [AjBool] True if range was found.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajQuerystrParseRange(AjPStr *Pqry, ajint *Pbegin, ajint *Pend,
                            AjBool *Prev)
{
    AjPStr tmpstr  = NULL;
    AjBool rangestat = ajFalse;

    if(!queryRegInitDone)
        queryRegInit();

    ajDebug("ajQuerystrParseRange: '%S'\n", *Pqry); 

    rangestat = ajRegExec(queryRegRange, *Pqry);

    if(rangestat)
    {
	ajRegSubI(queryRegRange, 2, &tmpstr);

	if(ajStrGetLen(tmpstr))
	    ajStrToInt(tmpstr, Pbegin);

	ajRegSubI(queryRegRange, 3, &tmpstr);

	if(ajStrGetLen(tmpstr))
	    ajStrToInt(tmpstr, Pend);

	ajRegSubI(queryRegRange, 5, &tmpstr);

	if(ajStrGetLen(tmpstr))
	    *Prev = ajTrue;

	ajStrDel(&tmpstr);
	ajRegSubI(queryRegRange, 1, Pqry);
	ajDebug("range found [%d:%d:%b]\n",
		*Pbegin, *Pend, *Prev);

        return ajTrue;
    }

    rangestat = ajRegExec(queryRegRefrange, *Pqry);

    if(rangestat)
    {
	ajRegSubI(queryRegRefrange, 2, &tmpstr);

	if(ajStrGetLen(tmpstr))
	    ajStrToInt(tmpstr, Pbegin);

	ajRegSubI(queryRegRefrange, 4, &tmpstr);

	if(ajStrGetLen(tmpstr))
	    ajStrToInt(tmpstr, Pend);

	ajRegSubI(queryRegRefrange, 6, &tmpstr);

	if(ajStrGetLen(tmpstr))
	    *Prev = ajTrue;

	ajStrDel(&tmpstr);
	ajRegSubI(queryRegRefrange, 1, Pqry);
	ajDebug("refrange found [%d:%d:%b]\n",
		*Pbegin, *Pend, *Prev);

        return ajTrue;
    }

    return ajFalse;
}




/* @func ajQuerystrParseRead **************************************************
**
** Parses a query (USA, UFO or general query).
**
** First tests for "asis" input and sets the rest of the query as input data.
**
** Then tests for server:dbname:query or dbname:query
** and tests whether the access method is a text method.
**
** For text access, calls the access method to opens the file
** (and set the file position).
**
** If there is no database, looks for file:query and opens the file.
**
** If an offset is provided by %offset this sets the file position
**
** If the file does not exist, tests again for a database of that name
** from any known server.
**
** If no text data was found, returns the filled in datatype-specific details
**
** @param [u] Pqry [AjPStr*]  Query string
** @param [u] textin [AjPTextin] Text input structure.
** @param [f] findformat [AjBool function] Function to validate format name
** @param [w] Pnontext [AjBool*] True if access is a non-text method
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajQuerystrParseRead(AjPStr *Pqry, AjPTextin textin,
                           AjBool findformat(const AjPStr format,
                                             ajint *iformat),
                           AjBool *Pnontext)
{
    AjPQuery qry;

    AjBool svrstat   = ajFalse;
    AjBool dbstat   = ajFalse;
    AjBool regstat   = ajFalse;
    AjBool accstat   = ajFalse;	/* return status from reading something */
    AjBool asisstat  = ajFalse;
    AjBool ftpstat   = ajFalse;
    AjBool httpstat  = ajFalse;
    AjBool drcatstat = ajFalse;

    AjBool isspecialfile = ajFalse;

    AjBool inbraces  = ajFalse;

    AjPStr braceopen  = NULL;
    AjPStr braceclose = NULL;
    AjPStr idstr = NULL;
    AjPStr lastoper= NULL;
    AjPStr operstr = NULL;
    AjPStr qrystr = NULL;

    ajlong i;
    ajuint iquery = 0;

    AjPStrTok handle = NULL;
    AjPTextAccess textaccess = NULL;

    ajint iformat = 0;

#ifdef __CYGWIN__
    char cygd;
#endif

    *Pnontext = ajFalse;
    qry = textin->Query;

    if(qry->QryDone)
	return ajFalse;

    ajStrAssignS(&qry->QryString, *Pqry);

    ajStrDel(&qry->SingleField);    /* clear it. we test this for regstat */

    if(!queryRegInitDone)
        queryRegInit();
    
    ajDebug("ajQuerystrParseRead qry '%S' iquery '%S' ioffset '%Ld' "
            "format: '%S' (%d)\n",
            qry->QryString, textin->QryFields, textin->Fpos,
            textin->Formatstr, textin->Format);

    ajStrAssignS(&qry->QryFields, textin->QryFields);
    qry->Fpos = textin->Fpos;

    /* Strip any leading spaces */
    ajStrTrimC(Pqry," \t\n");


    /*
    ** First we test some basic access methods
    */

    /*
    ** ASIS
    ** asis::datastream
    */

    asisstat = ajRegExec(queryRegAsis, *Pqry);

    if(asisstat)
    {
        ajStrAssignC(&queryFormat, "asis");

        if(!findformat(queryFormat, &textin->Format))
        {
            ajErr("asis:: not supported by datatype for '%S'",
                  *Pqry);
            return ajFalse;
        }
        
        ajStrAssignS(&qry->Formatstr, queryFormat);
        ajStrAssignS(&textin->Formatstr, queryFormat);

	ajRegSubI(queryRegAsis, 1, &qry->Filename);
	ajDebug("asis data '%S'\n", qry->Filename);

        isspecialfile = ajTrue;
    }

    /*
    ** FTP URL
    ** ftp://host.address/path/to/file
    ** supports -ioffset -iformat
    */

    ftpstat = ajRegExec(queryRegFtp, *Pqry);

    if(ftpstat)
    {
	ajRegSubI(queryRegFtp, 1, &qry->Filename);
	ajDebug("ftp data '%S'\n", qry->Filename);

        isspecialfile = ajTrue;
    }

    /*
    ** HTTP URL
    ** http://host.address/path/to/file
    ** supports -ioffset -iformat
    */

    httpstat = ajRegExec(queryRegHttp, *Pqry);

    if(httpstat)
    {
	ajRegSubI(queryRegHttp, 1, &qry->Filename);
	ajDebug("http data '%S'\n", qry->Filename);

        isspecialfile = ajTrue;
    }

#ifdef __CYGWIN__

    /*
    ** replace C: drive with /cygdrive/c/
    */

    if(ajStrGetCharPos(*Pqry,1) == ':')
    {
        cygd = ajStrGetCharFirst(*Pqry);

        if(isupper((int) cygd))
            ajStrPasteCountK(Pqry, 0, (char) tolower((int)cygd), 1);

        ajStrPasteCountK(Pqry, 1, '/', 1);
        ajStrInsertC(Pqry, 0, "/cygdrive/");
    }
#endif

    ajDebug("query to test: '%S'\n\n", *Pqry);


    /*
    ** test a name: prefix for a possible server name
    */

    svrstat = ajRegExec(queryRegSvr, *Pqry);
    ajDebug("server dbexp: %B '%S'\n", svrstat, *Pqry);

    if(svrstat)
    {
	ajRegSubI(queryRegSvr, 1, &querySvr);

	/* clear it if this was really a file */	

	if(ajNamServer(querySvr))
	{
            ajRegPost(queryRegSvr, Pqry);
	    ajDebug("found server %S, rest '%S'\n",
                    querySvr, *Pqry);
        }
        else
        {
	    ajDebug("unknown server %S, try dbname\n", querySvr);
            ajStrDel(&querySvr);
	    svrstat = ajFalse;
	}
    }

    regstat = ajFalse;

    if(!isspecialfile)
    {
        regstat = ajRegExec(queryRegFieldId, *Pqry);
        ajDebug("entry-id regexp: %B\n", regstat);
        ajDebug("filetest regexp queryRegFieldId: %B '%S'\n",
                regstat, *Pqry);

        if(regstat)
        {
            ajRegSubI(queryRegFieldId, 1, &queryDb);
            ajDebug("queryRegFieldId '%S'\n", queryDb);
            if(queryDbfind(&queryDb, querySvr, findformat,
                           qry, &dbstat, &drcatstat))
            {
                ajRegSubI(queryRegFieldId, 3, &qry->SingleField);
                ajRegSubI(queryRegFieldId, 4, &braceopen);
                ajRegSubI(queryRegFieldId, 5, &qry->QryFields);
                ajRegSubI(queryRegFieldId, 6, &braceclose);
                ajStrTrimStartC(&braceopen, ":");
            }
            else
            {
                ajDebug("failed: not a database or file '%S'\n", queryDb);
                regstat = ajFalse;
            }
        }

        if(!regstat)
        {
            regstat = ajRegExec(queryRegId, *Pqry);
            ajDebug("entry-id regexp: %B\n", regstat);
            ajDebug("filetest regexp queryRegId: %B '%S'\n", regstat, *Pqry);

            if(regstat)
            {
                ajRegSubI(queryRegId, 1, &queryDb);
                ajDebug("queryRegId '%S'\n", queryDb);
                ajRegSubI(queryRegId, 3, &queryChr);
                ajRegSubI(queryRegId, 5, &braceopen);
                ajRegSubI(queryRegId, 6, &qry->QryFields);
                ajRegSubI(queryRegId, 7, &braceclose);
                
                regstat = queryDbfind(&queryDb, querySvr, findformat,
                                      qry, &dbstat, &drcatstat);
                if(!regstat)
                    ajDebug("failed: not a database or file '%S'\n", queryDb);
            }
        }

        if(!regstat)
        {
            ajStrAssignS(&queryDb, *Pqry);
        }

    }

    if(svrstat && !dbstat)
    {
        ajErr("Server '%S:' but no database name in '%S'", querySvr, *Pqry);
        return ajFalse;
    }

    if(regstat && !dbstat && ajStrMatchC(queryChr, "%"))
    {
        ajStrToLong(qry->QryFields, &qry->Fpos);
        regstat = ajFalse;
    }

    /*
    ** no query found, but test -iquery input
    */

    if(!regstat && ajStrGetLen(textin->QryFields))
    {
        regstat = ajRegExec(queryRegQryField, textin->QryFields);
        if(regstat)
        {
            ajDebug("queryRegQryField\n");
            ajRegSubI(queryRegQryField, 1, &qry->SingleField);
            ajRegSubI(queryRegQryField, 3, &queryChr);
            ajRegSubI(queryRegQryField, 5, &braceopen);
            ajRegSubI(queryRegQryField, 6, &qry->QryFields);
            ajRegSubI(queryRegQryField, 7, &braceclose);
        }
    }
    
    if(!regstat && ajStrGetLen(textin->QryFields))
    {
        regstat = ajRegExec(queryRegQryId, textin->QryFields);
        ajDebug("entry-id regexp: %B\n", regstat);
        ajDebug("filetest regexp queryRegQryId: %B '%S'\n",
                regstat, textin->QryFields);

        if(regstat)
        {
            ajDebug("queryRegQryId\n");
            ajRegSubI(queryRegQryId, 1, &braceopen);
            ajRegSubI(queryRegQryId, 2, &qry->QryFields);
            ajRegSubI(queryRegQryId, 3, &braceclose);
        }
    }

    ajDebug("regstat: %B qry '%S' iquery '%S' queryDB: '%S' dbstat: %B "
           "open: '%S' close: '%S' field: '%S' qryfields: '%S'\n",
           regstat, *Pqry, textin->QryFields, queryDb, dbstat, 
           braceopen, braceclose, qry->SingleField, qry->QryFields);

    ajQueryStarclear(qry);

    if(dbstat)                  /* we have a database */
    {
        ajStrAssignS(&qry->DbName, queryDb);

        if(ajStrGetLen(querySvr))
        {
            ajStrAssignS(&qry->SvrName, querySvr);
            ajDebug("found server '%S' dbname '%S' level: '%S' "
                    "qryfields: '%S'\n",
                    qry->SvrName, qry->DbName,
                    qry->SingleField, qry->QryFields);
            dbstat = ajNamSvrData(qry, 0);

            if(!dbstat)
            {
                ajStrDel(&qry->QryFields);

                return ajFalse;
            }
        }
        else
        {
            ajDebug("found dbname '%S' level: '%S' qryfields: '%S'\n",
                    qry->DbName, qry->SingleField, qry->QryFields);
            if(drcatstat)
                dbstat = ajTrue;
            else
            {
                dbstat = ajNamDbData(qry, 0);

                if(!dbstat)
                {
                    ajStrDel(&qry->QryFields);

                    return ajFalse;
                }
            }
        }

	if(!dbstat)
	{
	    ajErr("no access method available for '%S'", *Pqry);

	    return ajFalse;
	}

        if(findformat(qry->Formatstr, &iformat))
            ajStrAssignS(&textin->Formatstr, qry->Formatstr);
        else
        {
            ajErr("unknown format '%S' for '%S' type '%S'\n",
                  qry->Formatstr, *Pqry, qry->DbType);

            return ajFalse;
        }
    }
    
    /*
    ** process the query fields
    */

    ajStrTrimWhite(&qry->QryFields);

    if(*MAJSTRGETPTR(braceopen))
    {
        if(*MAJSTRGETPTR(braceclose))
            inbraces = ajTrue;
        else
        {
            ajErr("Bad query syntax: unclosed braces '%S'", *Pqry);
            ajStrDel(&qry->QryFields);

            return ajFalse;
        }
    }
    else
    {
        if(*MAJSTRGETPTR(braceclose))
        {
            ajErr("Bad query syntax: unopened braces '%S'", *Pqry);
            ajStrDel(&qry->QryFields);

            return ajFalse;
        }
    }

    ajDebug("Query qryfields: '%S' open: '%S' close: '%S' "
            "inbraces: %B\n",
            qry->QryFields, braceopen, braceclose, inbraces);

    ajStrDel(&braceopen);
    ajStrDel(&braceclose);

    if(iformat >= 0)
    {
        textin->Format = iformat;
        textin->TextFormat = 0;
    }
    else
    {
        textin->TextFormat = - iformat;
        textin->Format = 0;
    }

    if(dbstat)
        ajDebug("database type: '%S' format '%S' %u textformat %u \n",
                qry->DbType, qry->Formatstr,
                textin->Format, textin->TextFormat);
    else
        ajDebug("file: '%S' format '%S' %u textformat %u \n",
                queryDb, qry->Formatstr,
                textin->Format, textin->TextFormat);

    if(regstat)
    {
        if(ajStrGetLen(qry->QryFields))
        {
            ajDebug("file Qry '%S' Field '%S' hasAcc:%B queryChr '%S'\n",
                    qry->QryFields, qry->SingleField,
                    qry->HasAcc, queryChr);

            if(ajStrGetCharFirst(qry->QryFields) == '!')
                ajStrInsertK(&qry->QryFields, 0, '*');

            ajStrAssignC(&lastoper, "|");

            if(ajStrGetLen(qry->SingleField)) /* one specified field */
            {
                ajDebug("query processing: single field '%S' query '%S'\n",
                       qry->SingleField, qry->QryFields);

                qry->QueryType = AJQUERY_QUERY;
                if(dbstat)
                    ajNamDbQuery(qry);
                else
                    ajNamFileQuery(qry);

                /* ajDebug("    qry->SingleField %S\n",
                   qry->SingleField); */
                
                if(!ajQueryKnownFieldS(qry, qry->SingleField))
                {
                    if(dbstat)
                        ajErr("Query '%S' query field '%S' not defined"
                          " for database '%S'",
                          *Pqry, qry->SingleField, qry->DbName);
                    else
                         ajErr("Query '%S' query field '%S' not defined"
                          " for datatype '%s'",
                               *Pqry, qry->SingleField,
                               ajQueryGetDatatype(qry));

                    return ajFalse;
                }

                /*
                ** in braces we treat spaces as delimiters
                ** in simple queries they are real
                */

                if(inbraces)
                    ajStrTokenAssignC(&handle, qry->QryFields, "|&!^ \t\n\r");
                else
                    ajStrTokenAssignC(&handle, qry->QryFields, "|&!^\t\n\r");

                while(ajStrTokenNextParseDelimiters(&handle, &idstr,
                                                    &operstr))
                {
                    if(iquery && (!ajStrGetLen(qry->Qlinks) ||
                                  (ajStrFindAnyS(qry->Qlinks, lastoper) == -1)))
                    {
                        if(dbstat)
                            ajErr("Query link operator '%S' not supported "
                              "by access method '%S' for database '%S'"
                              " allowed links are '%S'",
                              lastoper, qry->Method, qry->DbName,
                              qry->Qlinks);
                        else
                            ajErr("Query link operator '%S' not supported "
                                  "by access method '%S' for datatype '%s'"
                                  " allowed links are '%S'",
                                  lastoper, qry->Method,
                                  ajQueryGetDatatype(qry),
                                  qry->Qlinks);

                        return ajFalse;
                    }

                    iquery++;

                    switch(ajStrGetCharFirst(lastoper))
                    {
                        case '|':
                            ajQueryAddFieldOrS(qry,
                                               qry->SingleField, idstr);
                            ajDebug("ajQueryAddFieldOrS.a '%S' '%S'\n",
                                    qry->SingleField, idstr);
                            break;

                        case '&':
                            ajQueryAddFieldAndS(qry,
                                                qry->SingleField, idstr);
                            ajDebug("ajQueryAddFieldAndS.a '%S' '%S'\n",
                                    qry->SingleField, idstr);
                            break;

                        case '!':
                            ajQueryAddFieldNotS(qry,
                                                qry->SingleField, idstr);
                            ajDebug("ajQueryAddFieldNotS.a '%S' '%S'\n",
                                    qry->SingleField, idstr);
                            break;

                        case '^':
                            ajQueryAddFieldEorS(qry,
                                                qry->SingleField, idstr);
                            ajDebug("ajQueryAddFieldEorS.a '%S' '%S'\n",
                                    qry->SingleField, idstr);
                            break;

                        default:
                            ajErr("bad query syntax: "
                                  "unknown operator '%S'",
                                  lastoper);
                            break;
                    }

                    ajStrAssignS(&lastoper, operstr);
                    ajStrTrimWhite(&lastoper);
                }
            }
            else if(ajStrFindAnyK(qry->QryFields, ':') > 0) /* 1+ fields */
            {
                ajDebug("query processing: full query '%S'\n",
                       qry->QryFields);

                qry->QueryType = AJQUERY_QUERY;
                if(dbstat)
                    ajNamDbQuery(qry);
                else
                    ajNamFileQuery(qry);

                ajStrTokenAssignC(&handle, qry->QryFields, "|&^!");

                while(ajStrTokenNextParseDelimiters(&handle, &qrystr,
                                                    &operstr))
                {
                    if(ajStrGetLen(operstr))
                    {
                        if(!ajStrGetLen(qry->Qlinks) ||
                           (ajStrFindAnyS(qry->Qlinks, operstr) == -1))
                        {
                            if(dbstat)
                                ajErr("Query link operator '%S' not supported "
                                      "by access method '%S' for database '%S'"
                                      " allowed links are '%S'",
                                      operstr, qry->Method, qry->DbName,
                                      qry->Qlinks);
                            else
                                ajErr("Query link operator '%S' not supported "
                                      "by access method '%S' for datatype '%s'"
                                      " allowed links are '%S'",
                                      operstr, qry->Method,
                                      ajQueryGetDatatype(qry),
                                      qry->Qlinks);

                            return ajFalse;
                        }
                    }

                    ajStrTrimWhite(&qrystr);

                    i = ajStrFindAnyK(qrystr, ':');

                    if(i>0)
                    {
                        ajStrAssignSubS(&qry->SingleField, qrystr, 0, i-1);
                        ajStrAssignSubS(&idstr, qrystr, i+1, -1);
                        ajDebug("qrystr: '%S' field: '%S' qry: '%S'\n",
                                qrystr, qry->SingleField, idstr);
                    }

                    if(!ajQueryKnownFieldS(qry, qry->SingleField))
                    {
                        if(dbstat)
                            ajErr("Query '%S' query field '%S' not defined"
                                  " for database '%S'",
                                  *Pqry, qry->SingleField, qry->DbName);
                        else
                            ajErr("Query '%S' query field '%S' not defined"
                                  " for datatype '%s'",
                                  *Pqry, qry->SingleField,
                                  ajQueryGetDatatype(qry));
                        ajStrDel(&qry->QryFields);

                        return ajFalse;
                    }

                    if(iquery && (!ajStrGetLen(qry->Qlinks) ||
                                  (ajStrFindAnyS(qry->Qlinks, lastoper) == -1)))
                    {
                        if(dbstat)
                            ajErr("Query link operator '%S' not supported "
                                  "by access method '%S' for database '%S'"
                                  " allowed links are '%S'",
                                  lastoper, qry->Method, qry->DbName,
                                  qry->Qlinks);
                        else
                            ajErr("Query link operator '%S' not supported "
                                  "by access method '%S' for datatype '%s'"
                                  " allowed links are '%S'",
                                  lastoper, qry->Method,
                                  ajQueryGetDatatype(qry),
                                  qry->Qlinks);

                        return ajFalse;
                    }

                    iquery++;

                    switch(ajStrGetCharFirst(lastoper))
                    {
                        case '|':
                            ajQueryAddFieldOrS(qry, qry->SingleField, idstr);
                            ajDebug("ajQueryAddFieldOrS.b '%S' '%S'\n",
                                    qry->SingleField, idstr);
                            break;

                        case '&':
                            ajQueryAddFieldAndS(qry, qry->SingleField, idstr);
                            ajDebug("ajQueryAddFieldAndS.b '%S' '%S'\n",
                                    qry->SingleField, idstr);
                            break;

                        case '!':
                            ajQueryAddFieldNotS(qry, qry->SingleField, idstr);
                            ajDebug("ajQueryAddFieldNotS.b '%S' '%S'\n",
                                    qry->SingleField, idstr);
                            break;

                        case '^':
                            ajQueryAddFieldEorS(qry, qry->SingleField, idstr);
                            ajDebug("ajQueryAddFieldEorS.b '%S' '%S'\n",
                                    qry->SingleField, idstr);
                            break;

                        default:
                            ajErr("bad query syntax: "
                                  "unknown operator '%S'",
                                  lastoper);
                            break;
                    }
                    ajStrAssignS(&lastoper, operstr);
                    ajStrTrimWhite(&lastoper);
                }

                ajStrDel(&qrystr);
            }
            else if(ajStrFindAnyC(qry->QryFields, " \t\n\r,;|") >= 0)
            {
                ajDebug("query processing: idlist '%S'\n",
                       qry->QryFields);

                qry->QueryType = AJQUERY_QUERY;
                if(dbstat)
                    ajNamDbQuery(qry);
                else
                    ajNamFileQuery(qry);

                ajStrTokenAssignC(&handle, qry->QryFields, " \t\n\r,;|");

                while(ajStrTokenNextParse(&handle, &idstr))
                {
                    ajQueryAddFieldOrC(qry, "id",
                                       MAJSTRGETPTR(idstr));
                    ajDebug("ajQueryAddFieldOrC.c '%s' '%S'\n",
                            "id", idstr);

                    if(qry->HasAcc && ajQueryKnownFieldC(qry, "acc"))
                    {
                        ajQueryAddFieldElseC(qry, "acc",
                                             MAJSTRGETPTR(idstr));
                        ajDebug("ajQueryAddFieldElseC.c '%s' '%S'\n",
                                "acc", idstr);
                    }

                    if(ajQueryKnownFieldC(qry, "sv"))
                    {
                        ajQueryAddFieldElseC(qry, "sv",
                                             MAJSTRGETPTR(idstr));
                        ajDebug("ajQueryAddFieldElseC.c '%s' '%S'\n",
                                "sv", idstr);
                    }
                }
            }
            else                /* just a simple id */
            {
                ajDebug("query processing: simple id '%S'\n",
                       qry->QryFields);

                qry->QueryType = AJQUERY_ENTRY;
                if(dbstat)
                    ajNamDbQuery(qry);
                else
                    ajNamFileQuery(qry);

                ajQueryAddFieldOrC(qry, "id",
                                   MAJSTRGETPTR(qry->QryFields));
                ajDebug("ajQueryAddFieldOrC.d '%s' '%S'\n",
                        "id", qry->QryFields);

                if(qry->HasAcc && ajQueryKnownFieldC(qry, "acc"))
                {
                    ajQueryAddFieldElseC(qry, "acc",
                                         MAJSTRGETPTR(qry->QryFields));
                    ajDebug("ajQueryAddFieldElseC.d '%s' '%S'\n",
                            "acc", qry->QryFields);
                }

                if(ajQueryKnownFieldC(qry, "sv"))
                {
                    ajQueryAddFieldElseC(qry, "sv",
                                         MAJSTRGETPTR(qry->QryFields));
                    ajDebug("ajQueryAddFieldElseC.d '%s' '%S'\n",
                            "sv", qry->QryFields);
                }
            }
            ajStrAssignS(&lastoper, operstr);
            ajStrTrimWhite(&lastoper);
        }
        else
        {
            qry->QueryType = AJQUERY_ALL;
        }

        ajStrTokenDel(&handle);
        ajStrDel(&idstr);
        ajStrDel(&qrystr);
        ajStrDel(&operstr);
        ajStrDel(&lastoper);
    }
    else
    {
        qry->QueryType = AJQUERY_ALL;
    }

    if(ajQuerySetWild(qry))
    {
        if(qry->QueryType == AJQUERY_ENTRY)
            qry->QueryType = AJQUERY_QUERY;
    }

    if(qry->QueryType == AJQUERY_ALL && ajStrGetLen(qry->Organisms))
    {
        qry->QueryType = AJQUERY_QUERY;
        if(ajQueryKnownFieldC(qry, "org"))
        {
	    ajQueryAddFieldOrC(qry, "org",
			       MAJSTRGETPTR(qry->Organisms));
        }
        else if(ajQueryKnownFieldC(qry, "tax"))
        {
	    ajQueryAddFieldOrC(qry, "tax",
                               MAJSTRGETPTR(qry->Organisms));
        }
        else {
            if(dbstat)
                ajErr("Query '%S' query field 'org' or 'tax' not defined"
                      " for database '%S'",
                      *Pqry, qry->DbName);
            else
                ajErr("Query '%S' query field 'org' or 'tax' not defined"
                      " for datatype '%s'",
                      *Pqry, ajQueryGetDatatype(qry));
        }
    }

    if(qry->QueryType == AJQUERY_ALL && ajStrGetLen(qry->Namespace))
    {
        qry->QueryType = AJQUERY_QUERY;
        if(ajQueryKnownFieldC(qry, "namespace"))
        {            ajQueryAddFieldOrC(qry, "namespace",
                               MAJSTRGETPTR(qry->Namespace));
        }
        else if(ajQueryKnownFieldC(qry, "ns"))
        {            ajQueryAddFieldOrC(qry, "ns",
                               MAJSTRGETPTR(qry->Namespace));
        }
        else {
            if(dbstat)
                ajErr("Query '%S' query field 'ns' or 'namespace' not defined"
                      " for database '%S'",
                      *Pqry, qry->DbName);
            else
                ajErr("Query '%S' query field 'ns' or 'namespace' not defined"
                      " for datatype '%s'",
                      *Pqry, ajQueryGetDatatype(qry));
        }
    }

    ajQueryTrace(qry);

    if(isspecialfile)
    {
        if(asisstat)
            return ajTextinAccessAsis(textin);
        if(ftpstat)
            return ajTextinAccessFtp(textin);
        if(httpstat)
            return ajTextinAccessHttp(textin);
    }

    else if(dbstat)
    {
        ajDebug("try TEXT access by method '%S'\n", qry->Method);
        qry->TextAccess = ajCallTableGetS(textDbMethods,qry->Method);

        if(!qry->TextAccess)
        {
            *Pnontext = ajTrue;

            return ajTrue;
        }

        textaccess = qry->TextAccess;
        accstat = (*textaccess->Access)(textin);

        if(accstat)
            return ajTrue;

        ajDebug("Database '%S' : access method '%S' failed\n",
                qry->DbName, qry->Method);

        return ajFalse;
    }

    else if(ajFilenameExists(queryDb))
    {
        ajDebug("found filename %S\n", queryDb);
        ajStrAssignS(&qry->Filename, queryDb);

        if(qry->Fpos)
        {
            accstat = ajTextinAccessOffset(textin);

            ajQueryTrace(qry);

            if(accstat)
            {
                ajStrDel(&qry->QryFields);

                return ajTrue;
            }
        }
        accstat = ajTextinAccessFile(textin);

        if(accstat)
            return ajTrue;

        ajErr("Failed to open filename '%S'", qry->Filename);

        return ajFalse;

    }
    else 
    {
        ajErr("Failed to open filename '%S'", queryDb);
    }

    ajDebug("no valid filename specified\n");

    ajDebug("\n");

    return ajFalse;
    
    
}




/* @funcstatic queryDbfind ****************************************************
**
** Test a possible database name
**
** @param [u] Pdb [AjPStr*] Database name
** @param [r] svr [const AjPStr] Server name
** @param [f] findformat [AjBool function] Function to validate format name
** @param [u] qry [AjPQuery] Query object
** @param [w] dbstat [AjBool*] True if database is valid
** @param [w] drcatstat [AjBool*] True if database is found via DRCAT
** @return [AjBool] ajTrue if string is a known file or database name
**
******************************************************************************/

static AjBool queryDbfind(AjPStr *Pdb, const AjPStr svr,
                          AjBool findformat(const AjPStr format,
                                            ajint *iformat),
                          AjPQuery qry, AjBool* dbstat, AjBool* drcatstat)
{
    AjBool ret = ajFalse;
    AjPResource drcat = NULL;

    *dbstat = ajFalse;
    *drcatstat = ajFalse;

    ajDebug("queryDbfind db: '%S' svr: '%S'\n", *Pdb, svr);

    if(ajRegExec(queryRegDb, *Pdb))
    {
        *dbstat = ajTrue;
        if(ajStrGetLen(svr)) /* is it a server:dbname */
        {
            if(!ajNamAliasServer(Pdb, svr))
            {
                ajDebug("unknown dbname %S for server %S\n",
                        *Pdb, svr);
                *dbstat = ajFalse;
            }
        }
        else if(!ajNamAliasDatabase(Pdb)) /* is it a defined dbname */
        {
            ajDebug("not a db '%S'\n", *Pdb);
            if(ajFilenameExists(*Pdb)) /* existing filename */
            {
                ajDebug("is a file '%S'\n", *Pdb);
                ret = ajTrue;
                *dbstat = ajFalse;
            }
            else                /* search DRCAT */
            {
                drcat = ajResourceNewDrcat(*Pdb);
                
                if(drcat)
                {
                    ajDebug("database '%S' found via DRCAT datatype %u\n",
                            *Pdb, qry->DataType);

                    if(!ajResourceGetDbdata(drcat, qry, findformat))
                        *dbstat = ajFalse;
                    else
                    {
                        if(qry->DataType == AJDATATYPE_URL)
                        {
                            qry->QryData = drcat;
                            drcat = NULL;
                        }
                        *drcatstat = ajTrue;
                    }

                    ajResourceDel(&drcat);
                }
                else
                {
                    ajDebug("unknown dbname %S, try filename\n", *Pdb);
                    *dbstat = ajFalse;
                }
                ajDebug("drcat search: %B '%S'\n", *drcatstat, *Pdb);
            }
        }
    }

    if(*dbstat || ret)
        return ajTrue;

    if(ajFilenameExists(*Pdb)) /* existing filename */
    {
        ajDebug("not a dbname but is a file '%S'\n", *Pdb);
        ret = ajTrue;
        *dbstat = ajFalse;
    }

    return ret;
}




/* @funcstatic queryRegInit ***************************************************
**
** Initialised regular expressions for parsing queries
**
** @return [void]
**
** @release 6.4.0
******************************************************************************/

static void queryRegInit(void)
{
    if(queryRegInitDone)
        return;

    if(!queryRegFmt)
	queryRegFmt = ajRegCompC("^([A-Za-z0-9-]*)::(.*)$");
    /* \1 format letters and numbers only */
    /* \2 remainder (filename, etc.)*/

    if(!queryRegSvr)
	queryRegSvr = ajRegCompC("^([A-Za-z][A-Za-z0-9_.]+):");
    /* \1 svrname (start with a letter, then alphanumeric) */

    if(!queryRegDb)
	queryRegDb = ajRegCompC("^[A-Za-z][A-Za-z0-9_.]*[A-Za-z0-9]$");

    if(!queryRegQryField)
	queryRegQryField = ajRegCompC("^([A-Za-z]+)?"
                                   "(([:]?)(([{]?)([^}]*)(}?)))?$");
    if(!queryRegQryId)
	queryRegQryId = ajRegCompC("^([{]?)([^}]*)(}?)$");

    /* \1 dbname (start with a letter, then alphanumeric) */
    /* \2 -id or -acc etc. */
    /* \3 qry->SingleField (id or acc etc.) */
    /* \4 :qrystring or {qrystring}*/
    /* \5 qrystring */

    if(!queryRegFieldId)
#ifndef WIN32
        /* \1 is filename \3 is the qry->SingleField \6 is the qrystring */
	queryRegFieldId = ajRegCompC("^(([^{|]+[|])|[^{:]+)"
                                     "[:-]([^{:-]+)"
                                     "([:{]+)([^}]+)(}?)$");
#else
	/* Windows file names can start with e.g.: 'C:\' */
	/* But allow e.g. 'C:/...', for Staden spin */

        /* \1 is filename \3 is the qry->SingleField \6 is the qrystring */
	queryRegFieldId = ajRegCompC ("^(([a-zA-Z]:[\\\\/])?[^{:]+)"
                                      "[:-]([^{:-]+)"
                                      "([:{]+)([^}]+)(}?)$");
#endif


    if(!queryRegId)
#ifndef WIN32
        /* \1 is filename \3 is the qry->SingleField \6 is the qry->QryString */
	queryRegId = ajRegCompC("^(([^|{]+[|])|[^:{%]*[^:{%-])"
                                "([:%-]?)(({?)([^}]*)(}?))?$");
#else
	/* Windows file names can start with e.g.: 'C:\' */
	/* But allow e.g. 'C:/...', for Staden spin */

        /* \1 is filename \6 is the qry->SingleField \7 is the qry->QryString */
	queryRegId = ajRegCompC ("^(([a-zA-Z]:[\\\\/])?[^:{%]*[^:{%-])"
                                "([:%-]?)(({?)([^}]*)(}?))?$");
#endif


    if(!queryRegList)	 /* \1 is filename \3 is the qry->QryString */
	queryRegList = ajRegCompC("^(@|[Ll][Ii][Ss][Tt]:+)(.+)$");

    if(!queryRegAsis)	 /* \1 is filename \3 is the qry->QryString */
	queryRegAsis = ajRegCompC("^[Aa][Ss][Ii][Ss]:+(.+)$");

    if(!queryRegFtp)	 /* \1 is filename \3 is the qry->QryString */
	queryRegFtp = ajRegCompC("^([Ff][Tt][Pp]:/*.+)$");

    if(!queryRegHttp)	 /* \1 is filename \3 is the qry->QryString */
	queryRegHttp = ajRegCompC("^([Hh][Tt][Tt][Pp]:/*.+)$");

    if(!queryRegWild)
	queryRegWild = ajRegCompC("(.*[*].*)");
    /* \1 wildcard query */

    if(!queryRegRange)    /* \1 is rest of USA \2 start \3 end \5 reverse */
	queryRegRange = ajRegCompC("(.*)[[](-?[0-9]*):(-?[0-9]*)"
                                   "(:([Rr])?)?[]]$");

    if(!queryRegRefrange)    /* \1 is rest of USA \2 start \4 end \6 rev */
	queryRegRefrange = ajRegCompC("(.*):([0-9]*)(:|\\.\\.)([0-9]*)"
                                   "(:([Rr]))?$");

    queryRegInitDone = ajTrue;

    return;
}

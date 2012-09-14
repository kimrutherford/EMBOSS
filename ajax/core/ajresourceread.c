/* @source ajresourceread *****************************************************
**
** AJAX data resource reading functions
**
** These functions control all aspects of AJAX data resource reading
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.35 $
** @modified Oct 5 pmr First version
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

#include "ajresourceread.h"
#include "ajresource.h"
#include "ajcall.h"
#include "ajlist.h"
#include "ajquery.h"
#include "ajtextread.h"
#include "ajnam.h"
#include "ajfileio.h"


#include <string.h>


AjPTable resourceDbMethods = NULL;

static AjPStr   resourceinReadLine = NULL;

const char* resourceStatus[] = {
    "Supported",
    "Referenced",
    "Unavailable",
    "None",
    NULL
};

const char* resourceDataFormat[] = {
    "Unknown",
    "HTML",
    "Text",
    "Text (EMBL)",
    "Text (fasta)",
    "Text (gff)",
    NULL
};
    
const char* resourceTags[] = {
    "ID",
    "IDalt",
    "Acc",
    "Name",
    "Desc",
    "URL",
    "URLlink",
    "URLrest",
    "URLsoap",
    "Cat",
    "EDAMtpc",
    "EDAMdat",
    "EDAMid",
    "EDAMfmt",
    "Xref",
    "Query",
    "Example",
    "Contact",
    "Email",
    "CCxref",
    "CCmisc",
    "CCrest",
    "CCsoap", 
    "Status",
    NULL
};



static AjBool resourceinReadDrcat(AjPResourcein thys, AjPResource resource);




/* @datastatic ResourcePInFormat **********************************************
**
** Data input formats data structure
**
** @alias ResourceSInFormat
** @alias ResourceOInFormat
**
** @attr Name [const char*] Format name
** @attr Desc [const char*] Format description
** @attr Alias [AjBool] Name is an alias for an identical definition
** @attr Try [AjBool] If true, try for an unknown input. Duplicate names
**                    and read-anything formats are set false
** @attr Read [AjBool function] Input function, returns ajTrue on success
** @@
******************************************************************************/

typedef struct ResourceSInFormat
{
    const char *Name;
    const char *Desc;
    AjBool Alias;
    AjBool Try;
    AjBool (*Read) (AjPResourcein thys, AjPResource resource);
} ResourceOInFormat;

#define ResourcePInFormat ResourceOInFormat*

static ResourceOInFormat resourceinFormatDef[] =
{
/* "Name",        "Description" */
/*     Alias,   Try,     */
/*     ReadFunction */
  {"unknown",     "Unknown format",
       AJFALSE, AJFALSE,
       &resourceinReadDrcat}, /* default to first format */
  {"drcat",       "Data resource catalogue format",
       AJFALSE, AJFALSE,
       &resourceinReadDrcat},
  {NULL, NULL, 0, 0, NULL}
};



static ajuint resourceinReadFmt(AjPResourcein resourcein,
                                AjPResource resource,
                                ajuint format);
static AjBool resourceinRead(AjPResourcein resourcein,
                             AjPResource resource);
static AjBool resourceinformatFind(const AjPStr format,
                                   ajint* iformat);
static AjBool resourceinFormatSet(AjPResourcein resourcein,
                                  AjPResource resource);
static AjBool resourceinListProcess(AjPResourcein resourcein,
                                    AjPResource resource,
                                    const AjPStr listfile);
static void   resourceinListNoComment(AjPStr* text);
static void   resourceinQryRestore(AjPResourcein resourcein,
                                   const AjPQueryList node);
static void   resourceinQrySave(AjPQueryList node,
                                const AjPResourcein resourcein);
static AjBool resourceDefine(AjPResource thys,
                             AjPResourcein resourcein);
static AjBool resourceinQryProcess(AjPResourcein resourcein,
                                   AjPResource resource);
static AjBool resourceinQueryMatch(const AjPQuery thys,
                                   const AjPResource resource);




/* @filesection ajresourceread ************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
*/




/* @datasection [AjPResourcein] Data resource input objects *******************
**
** Function is for manipulating data input objects
**
** @nam2rule Resourcein
******************************************************************************/




/* @section Data input constructors *******************************************
**
** All constructors return a new data input object by pointer. It
** is the responsibility of the user to first destroy any previous
** data input object. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so
** anyway.
**
** @fdata [AjPResourcein]
**
** @nam3rule  New     Construct a new data input object
** @nam4rule  Drcat   Construct using a DRCAT identifier
**
** @argrule Drcat dbname [const AjPStr] DRCAT identifier
** @valrule   *  [AjPResourcein] New data input object
**
** @fcategory new
**
******************************************************************************/




/* @func ajResourceinNew ******************************************************
**
** Creates a new data input object.
**
** @return [AjPResourcein] New data input object.
** @category new [AjPResourcein] Default constructor
**
** @release 6.4.0
** @@
******************************************************************************/

AjPResourcein ajResourceinNew(void)
{
    AjPResourcein pthis;

    AJNEW0(pthis);

    pthis->Input = ajTextinNewDatatype(AJDATATYPE_RESOURCE);

    pthis->ResourceData      = NULL;

    return pthis;
}





/* @func ajResourceinNewDrcat *************************************************
**
** Creates a new data input object with a resolved DRCAT query
**
** @param [r] dbname [const AjPStr] DRCAT identifier
**
** @return [AjPResourcein] New data input object.
** @category new [AjPResourcein] Default constructor
**
** @release 6.4.0
** @@
******************************************************************************/

AjPResourcein ajResourceinNewDrcat(const AjPStr dbname)
{
    AjPResourcein pthis;
    AjPQuery qry;
    AjPTextin textin;
    AjPTextAccess textaccess;

    AjBool dbstat;

    AJNEW0(pthis);

    pthis->Input = ajTextinNewDatatype(AJDATATYPE_RESOURCE);
    pthis->ResourceData      = NULL;

    textin = pthis->Input;
    qry = textin->Query;

    ajStrAssignC(&qry->DbName, "drcat");
    dbstat = ajNamDbData(qry, 0);
    if(!dbstat)
        ajDie("ajResourceinNewDrcat '%S' failed to find drcat", dbname);
    ajFmtPrintS(&textin->Qry, "drcat:%S", dbname);
    ajStrAssignC(&qry->SingleField, "id");
    ajStrAssignS(&qry->QryString, dbname);
    ajQueryAddFieldOrS(qry, qry->SingleField, qry->QryString);
    ajStrAssignC(&qry->SingleField, "acc");
    ajQueryAddFieldOrS(qry, qry->SingleField, qry->QryString);
    ajStrAssignC(&qry->Formatstr, "drcat");
    ajStrAssignC(&textin->Formatstr, "drcat");
    resourceinformatFind(textin->Formatstr, &textin->Format);
    ajStrAssignC(&qry->Method, "emboss");
    dbstat = ajNamDbQuery(qry);
    if(!dbstat)
        ajDie("ajResourceinNewDrcat '%S' failed to set querytype", dbname);
    qry->TextAccess = ajCallTableGetS(textDbMethods,qry->Method);
    textaccess = qry->TextAccess;

    (*textaccess->Access)(textin);

    return pthis;
}





/* @section data input destructors ********************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the data input object.
**
** @fdata [AjPResourcein]
**
** @nam3rule Del Destructor
**
** @argrule Del pthis [AjPResourcein*] Data resource input object
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajResourceinDel ******************************************************
**
** Deletes a data input object.
**
** @param [d] pthis [AjPResourcein*] Data resource input
** @return [void]
** @category delete [AjPResourcein] Default destructor
**
** @release 6.4.0
** @@
******************************************************************************/

void ajResourceinDel(AjPResourcein* pthis)
{
    AjPResourcein thys;

    if(!pthis)
        return;

    thys = *pthis;

    if(!thys)
        return;

    ajDebug("ajResourceinDel called qry:'%S'\n", thys->Input->Qry);

    ajTextinDel(&thys->Input);

    AJFREE(*pthis);

    return;
}




/* @section data input modifiers **********************************************
**
** These functions use the contents of a data input object and
** update them.
**
** @fdata [AjPResourcein]
**
** @nam3rule Clear Clear all values
** @nam3rule Qry Reset using a query string
** @suffix C Character string input
** @suffix S String input
**
** @argrule * thys [AjPResourcein] Data resource input object
** @argrule C txt [const char*] Query text
** @argrule S str [const AjPStr] query string
**
** @valrule * [void]
**
** @fcategory modify
**
******************************************************************************/




/* @func ajResourceinClear ****************************************************
**
** Clears a data input object back to "as new" condition, except
** for the query list which must be preserved.
**
** @param [w] thys [AjPResourcein] Data resource input
** @return [void]
** @category modify [AjPResourcein] Resets ready for reuse.
**
** @release 6.4.0
** @@
******************************************************************************/

void ajResourceinClear(AjPResourcein thys)
{

    ajDebug("ajResourceinClear called\n");

    if(!thys)
        return;

    ajTextinClear(thys->Input);

    thys->ResourceData = NULL;

    return;
}




/* @func ajResourceinQryC *****************************************************
**
** Resets a data input object using a new Universal
** Query Address
**
** @param [u] thys [AjPResourcein] Data resource input object.
** @param [r] txt [const char*] Query
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajResourceinQryC(AjPResourcein thys, const char* txt)
{
    ajResourceinClear(thys);
    ajStrAssignC(&thys->Input->Qry, txt);

    return;
}





/* @func ajResourceinQryS *****************************************************
**
** Resets a data input object using a new Universal
** Query Address
**
** @param [u] thys [AjPResourcein] Data resource input object.
** @param [r] str [const AjPStr] Query
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajResourceinQryS(AjPResourcein thys, const AjPStr str)
{
    if(!thys)
        return;

    ajResourceinClear(thys);
    ajStrAssignS(&thys->Input->Qry, str);

    return;
}




/* @section casts *************************************************************
**
** Return values
**
** @fdata [AjPResourcein]
**
** @nam3rule Trace Write debugging output
**
** @argrule * thys [const AjPResourcein] Data resource input object
**
** @valrule * [void]
**
** @fcategory cast
**
******************************************************************************/




/* @func ajResourceinTrace ****************************************************
**
** Debug calls to trace the data in a data input object.
**
** @param [r] thys [const AjPResourcein] Data resource input object.
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajResourceinTrace(const AjPResourcein thys)
{
    ajDebug("data input trace\n");
    ajDebug("====================\n\n");

    ajTextinTrace(thys->Input);

    if(thys->ResourceData)
	ajDebug( "  ResourceData: exists\n");

    return;
}




/* @section Data data inputs **********************************************
**
** These functions read the wxyxdesc data provided by the first argument
**
** @fdata [AjPResourcein]
**
** @nam3rule Read Read data data
**
** @argrule Read resourcein [AjPResourcein] Data resource input object
** @argrule Read resource [AjPResource] Data resource data
**
** @valrule * [AjBool] true on success
**
** @fcategory input
**
******************************************************************************/




/* @func ajResourceinRead *****************************************************
**
** If the file is not yet open, calls resourceinQryProcess to convert the query
** into an open file stream.
**
** Uses resourceinRead for the actual file reading.
**
** Returns the results in the AjPResource object.
**
** @param [u] resourcein [AjPResourcein] Data resource data input definitions
** @param [w] resource [AjPResource] Data resource data returned.
** @return [AjBool] ajTrue on success.
** @category input [AjPResource] Master data data input,
**                  calls specific functions for file access type
**                  and data data format.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajResourceinRead(AjPResourcein resourcein, AjPResource resource)
{
    AjBool ret       = ajFalse;
    AjPQueryList node = NULL;
    AjBool listdata  = ajFalse;

    if(resourcein->Input->Filebuff)
    {
	/* (a) if file still open, keep reading */
	ret = resourceinRead(resourcein, resource);
    }
    else
    {
	/* (b) if we have a list, try the next query in the list */
	if(ajListGetLength(resourcein->Input->List))
	{
	    listdata = ajTrue;
	    ajListPop(resourcein->Input->List, (void**) &node);

	    ajResourceinQryS(resourcein, node->Qry);

            resourceinQryRestore(resourcein, node);

	    ajStrDel(&node->Qry);
	    ajStrDel(&node->Formatstr);
	    AJFREE(node);

	    if(!resourceinQryProcess(resourcein, resource) &&
               !ajListGetLength(resourcein->Input->List))
		return ajFalse;

	    ret = resourceinRead(resourcein, resource);
	}
	else
	{
	    /* (c) Must be a query - decode it */
	    if(!resourceinQryProcess(resourcein, resource) &&
               !ajListGetLength(resourcein->Input->List))
		return ajFalse;

	    if(ajListGetLength(resourcein->Input->List)) /* maybe a new list */
		listdata = ajTrue;

	    ret = resourceinRead(resourcein, resource);
	}
    }

    /* Now read whatever we got */

    while(!ret && ajListGetLength(resourcein->Input->List))
    {
	/* Failed, but we have a list still - keep trying it */
        if(listdata)
	    ajErr("Failed to read data data '%S'",
                  resourcein->Input->Qry);

	listdata = ajTrue;
	ajListPop(resourcein->Input->List,(void**) &node);
	ajResourceinQryS(resourcein, node->Qry);

	resourceinQryRestore(resourcein, node);

	ajStrDel(&node->Qry);
	ajStrDel(&node->Formatstr);
	AJFREE(node);

	if(!resourceinQryProcess(resourcein, resource))
	    continue;

	ret = resourceinRead(resourcein, resource);
    }

    if(!ret)
    {
	if(listdata)
	    ajErr("Failed to read data data '%S'",
                  resourcein->Input->Qry);

	return ajFalse;
    }


    resourceDefine(resource, resourcein);

    return ajTrue;
}




/* @funcstatic resourceinQueryMatch *******************************************
**
** Compares a data data item to a query and returns true if they match.
**
** @param [r] thys [const AjPQuery] query.
** @param [r] resource [const AjPResource] Data resource data.
** @return [AjBool] ajTrue if the data data matches the query.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool resourceinQueryMatch(const AjPQuery thys,
                                   const AjPResource resource)
{
    AjBool tested = ajFalse;
    AjIList iterfield  = NULL;
    AjPQueryField field = NULL;
    AjBool ok = ajFalse;

    ajDebug("resourceinQueryMatch '%S' fields: %Lu Case %B Done %B\n",
	    resource->Id, ajListGetLength(thys->QueryFields),
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
                    resource->Id);
            if(thys->CaseId)
            {
                if(ajStrMatchWildS(resource->Id, field->Wildquery))
                {
                    ajListIterDel(&iterfield);
                    return ajTrue;
                }
            }
            else
            {
                if(ajStrMatchWildCaseS(resource->Id, field->Wildquery))
                {
                    ajListIterDel(&iterfield);
                    return ajTrue;
                }
            }
            
            ajDebug("id test failed\n");
            tested = ajTrue;
            ok = ajFalse;
        }

        if(ajStrMatchC(field->Field, "acc")) /* test id, use trueid */
        {
            if(ajStrMatchWildCaseS(resource->Id, field->Wildquery))
            {
                ajListIterDel(&iterfield);
                return ajTrue;
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




/* @funcstatic resourceDefine *************************************************
**
** Make sure all data data object attributes are defined
** using values from the data input object if needed
**
** @param [w] thys [AjPResource] Data resource data returned.
** @param [u] resourcein [AjPResourcein] Data resource data input definitions
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool resourceDefine(AjPResource thys, AjPResourcein resourcein)
{

    /* if values are missing in the data object, we can use defaults
       from resourcein or calculate where possible */

    /* assign the dbname if defined in the resourcein object */
    if(ajStrGetLen(resourcein->Input->Db))
      ajStrAssignS(&thys->Db, resourcein->Input->Db);

    return ajTrue;
}





/* @funcstatic resourceinReadFmt **********************************************
**
** Tests whether data data can be read using the specified format.
** Then tests whether the data data matches data data query criteria
** and checks any specified type. Applies upper and lower case.
**
** @param [u] resourcein [AjPResourcein] Data resource data input object
** @param [w] resource [AjPResource] Data resource data object
** @param [r] format [ajuint] input format code
** @return [ajuint] 0 if successful.
**                  1 if the query match failed.
**                  2 if the data data type failed
**                  3 if it failed to read any data data
**
** @release 6.4.0
** @@
** This is the only function that calls the appropriate Read function
** resourceinReadXxxxxx where Xxxxxxx is the supported data data format.
**
** Some of the resourceReadXxxxxx functions fail to reset the buffer correctly,
** which is a very serious problem when cycling through all of them to
** identify an unknown format. The extra ajFileBuffReset call at the end is
** intended to address this problem. The individual functions should still
** reset the buffer in case they are called from elsewhere.
**
******************************************************************************/

static ajuint resourceinReadFmt(AjPResourcein resourcein, AjPResource resource,
                           ajuint format)
{
    ajDebug("++resourceinReadFmt format %d (%s) '%S'\n",
	    format, resourceinFormatDef[format].Name,
	    resourcein->Input->Qry);

    resourcein->Input->Records = 0;

    /* Calling funclist resourceinFormatDef() */
    if((*resourceinFormatDef[format].Read)(resourcein, resource))
    {
	ajDebug("resourceinReadFmt success with format %d (%s)\n",
		format, resourceinFormatDef[format].Name);
        ajDebug("id: '%S'\n",
                resource->Id);
	resourcein->Input->Format = format;
	ajStrAssignC(&resourcein->Input->Formatstr,
                     resourceinFormatDef[format].Name);
	ajStrAssignC(&resource->Formatstr, resourceinFormatDef[format].Name);
	ajStrAssignEmptyS(&resource->Db, resourcein->Input->Db);
	ajStrAssignS(&resource->Filename, resourcein->Input->Filename);

	if(resourceinQueryMatch(resourcein->Input->Query, resource))
	{
            /* ajResourceinTrace(resourcein); */

            return FMT_OK;
        }

	ajDebug("query match failed, continuing ...\n");
	ajResourceClear(resource);

	return FMT_NOMATCH;
    }
    else
    {
	ajDebug("Testing input buffer: IsBuff: %B Eof: %B\n",
		ajFilebuffIsBuffered(resourcein->Input->Filebuff),
		ajFilebuffIsEof(resourcein->Input->Filebuff));

	if (!ajFilebuffIsBuffered(resourcein->Input->Filebuff) &&
	    ajFilebuffIsEof(resourcein->Input->Filebuff))
	    return FMT_EOF;

	ajFilebuffReset(resourcein->Input->Filebuff);
	ajDebug("Format %d (%s) failed, "
                "file buffer reset by resourceinReadFmt\n",
		format, resourceinFormatDef[format].Name);
	/* ajFilebuffTraceFull(resourcein->Filebuff, 10, 10);*/
    }

    ajDebug("++resourceinReadFmt failed - nothing read\n");

    return FMT_FAIL;
}




/* @funcstatic resourceinRead *************************************************
**
** Given data in a resourcein structure, tries to read everything needed
** using the specified format or by trial and error.
**
** @param [u] resourcein [AjPResourcein] Data resource data input object
** @param [w] resource [AjPResource] Data resource data object
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool resourceinRead(AjPResourcein resourcein, AjPResource resource)
{
    ajuint i;
    ajuint istat = 0;
    ajuint jstat = 0;

    AjPFilebuff buff = resourcein->Input->Filebuff;
    AjBool ok;

    AjPTextAccess  textaccess  = resourcein->Input->Query->TextAccess;
    AjPResourceAccess resourceaccess = resourcein->Input->Query->Access;

    ajResourceClear(resource);
    ajDebug("resourceinRead: cleared\n");

    if(resourcein->Input->Single && resourcein->Input->Count)
    {
	/*
	** One data data item at a time is read.
	** The first data data item was read by ACD
	** for the following ones we need to reset the AjPResourcein
	**
	** Single is set by the access method
	*/

	ajDebug("resourceinRead: single access - count %d - call access"
		" routine again\n",
		resourcein->Input->Count);
	/* Calling funclist resourceinAccess() */
	if(textaccess)
        {
            if(!(*textaccess->Access)(resourcein->Input))
            {
                ajDebug("resourceinRead: (*textaccess->Access)"
                        "(resourcein->Input) *failed*\n");

                return ajFalse;
            }
        }

	if(resourceaccess)
        {
            if(!(*resourceaccess->Access)(resourcein))
            {
                ajDebug("resourceinRead: (*resourceaccess->Access)(resourcein) "
                        "*failed*\n");

                return ajFalse;
            }
        }

        buff = resourcein->Input->Filebuff;
    }

    ajDebug("resourceinRead: resourcein format %d '%S'\n",
            resourcein->Input->Format,
	    resourcein->Input->Formatstr);

    resourcein->Input->Count++;

    if(!resourcein->Input->Filebuff)
	return ajFalse;

    ok = ajFilebuffIsBuffered(resourcein->Input->Filebuff);

    while(ok)
    {				/* skip blank lines */
        ok = ajBuffreadLine(resourcein->Input->Filebuff, &resourceinReadLine);

        if(!ajStrIsWhite(resourceinReadLine))
        {
            ajFilebuffClear(resourcein->Input->Filebuff,1);
            break;
        }
    }

    if(!resourcein->Input->Format)
    {			   /* no format specified, try all defaults */
	for(i = 1; resourceinFormatDef[i].Name; i++)
	{
	    if(!resourceinFormatDef[i].Try)	/* skip if Try is ajFalse */
		continue;

	    ajDebug("resourceinRead:try format %d (%s)\n",
		    i, resourceinFormatDef[i].Name);

	    istat = resourceinReadFmt(resourcein, resource, i);

	    switch(istat)
	    {
	    case FMT_OK:
		ajDebug("++resourceinRead OK, set format %d\n",
                        resourcein->Input->Format);
		resourceDefine(resource, resourcein);

		return ajTrue;
	    case FMT_BADTYPE:
		ajDebug("resourceinRead: (a1) "
                        "resourceinReadFmt stat == BADTYPE *failed*\n");

		return ajFalse;
	    case FMT_FAIL:
		ajDebug("resourceinRead: (b1) "
                        "resourceinReadFmt stat == FAIL *failed*\n");
		break;			/* we can try next format */
	    case FMT_NOMATCH:
		ajDebug("resourceinRead: (c1) "
                        "resourceinReadFmt stat==NOMATCH try again\n");
		break;
	    case FMT_EOF:
		ajDebug("resourceinRead: (d1) "
                        "resourceinReadFmt stat == EOF *failed*\n");
		return ajFalse;			/* EOF and unbuffered */
	    case FMT_EMPTY:
		ajWarn("data data '%S' has zero length, ignored",
		       ajResourceGetQryS(resource));
		ajDebug("resourceinRead: (e1) "
                        "resourceinReadFmt stat==EMPTY try again\n");
		break;
	    default:
		ajDebug("unknown code %d from resourceinReadFmt\n", stat);
	    }

	    ajResourceClear(resource);

	    if(resourcein->Input->Format)
		break;			/* we read something */

            ajFilebuffTrace(resourcein->Input->Filebuff);
	}

	if(!resourcein->Input->Format)
	{		     /* all default formats failed, give up */
	    ajDebug("resourceinRead:all default formats failed, give up\n");

	    return ajFalse;
	}

	ajDebug("++resourceinRead set format %d\n",
                resourcein->Input->Format);
    }
    else
    {					/* one format specified */
	ajDebug("resourceinRead: one format specified\n");
	ajFilebuffSetUnbuffered(resourcein->Input->Filebuff);

	ajDebug("++resourceinRead known format %d\n",
                resourcein->Input->Format);
	istat = resourceinReadFmt(resourcein, resource,
                                  resourcein->Input->Format);

	switch(istat)
	{
	case FMT_OK:
	    resourceDefine(resource, resourcein);

	    return ajTrue;
	case FMT_BADTYPE:
	    ajDebug("resourceinRead: (a2) "
                    "resourceinReadFmt stat == BADTYPE *failed*\n");

	    return ajFalse;

        case FMT_FAIL:
	    ajDebug("resourceinRead: (b2) "
                    "resourceinReadFmt stat == FAIL *failed*\n");

	    return ajFalse;

        case FMT_NOMATCH:
	    ajDebug("resourceinRead: (c2) "
                    "resourceinReadFmt stat == NOMATCH *try again*\n");
	    break;
	case FMT_EOF:
	    ajDebug("resourceinRead: (d2) "
                    "resourceinReadFmt stat == EOF *try again*\n");
            if(resourcein->Input->Records)
                ajErr("Error reading file '%F' with format '%s': "
                      "end-of-file before end of data "
                      "(read %u records)",
                      ajFilebuffGetFile(resourcein->Input->Filebuff),
                      resourceinFormatDef[resourcein->Input->Format].Name,
                      resourcein->Input->Records);
	    break;		     /* simply end-of-file */
	case FMT_EMPTY:
	    ajWarn("resource data '%S' has zero length, ignored",
		   ajResourceGetQryS(resource));
	    ajDebug("resourceinRead: (e2) "
                    "resourceinReadFmt stat == EMPTY *try again*\n");
	    break;
	default:
	    ajDebug("unknown code %d from resourceinReadFmt\n", stat);
	}

	ajResourceClear(resource); /* 1 : read, failed to match id/acc/query */
    }

    /* failed - probably entry/accession query failed. Can we try again? */

    ajDebug("resourceinRead failed - try again with format %d '%s' code %d\n",
	    resourcein->Input->Format,
            resourceinFormatDef[resourcein->Input->Format].Name, istat);

    ajDebug("Search:%B Chunk:%B Data:%x ajFileBuffEmpty:%B\n",
	    resourcein->Input->Search, resourcein->Input->ChunkEntries,
            resourcein->Input->TextData, ajFilebuffIsEmpty(buff));

    if(ajFilebuffIsEmpty(buff) && resourcein->Input->ChunkEntries)
    {
	if(textaccess && !(*textaccess->Access)(resourcein->Input))
            return ajFalse;
	else if(resourceaccess && !(*resourceaccess->Access)(resourcein))
            return ajFalse;
        buff = resourcein->Input->Filebuff;
    }


    /* need to check end-of-file to avoid repeats */
    while(resourcein->Input->Search &&
          (resourcein->Input->TextData || !ajFilebuffIsEmpty(buff)))
    {
	jstat = resourceinReadFmt(resourcein, resource,
                                  resourcein->Input->Format);

	switch(jstat)
	{
	case FMT_OK:
	    resourceDefine(resource, resourcein);

	    return ajTrue;

        case FMT_BADTYPE:
	    ajDebug("resourceinRead: (a3) "
                    "resourceinReadFmt stat == BADTYPE *failed*\n");

	    return ajFalse;

        case FMT_FAIL:
	    ajDebug("resourceinRead: (b3) "
                    "resourceinReadFmt stat == FAIL *failed*\n");

	    return ajFalse;
            
	case FMT_NOMATCH:
	    ajDebug("resourceinRead: (c3) "
                    "resourceinReadFmt stat == NOMATCH *try again*\n");
	    break;
	case FMT_EOF:
	    ajDebug("resourceinRead: (d3) "
                    "resourceinReadFmt stat == EOF *failed*\n");

	    return ajFalse;			/* we already tried again */

        case FMT_EMPTY:
	    if(istat != FMT_EMPTY)
                ajWarn("assmebly data '%S' has zero length, ignored",
                       ajResourceGetQryS(resource));
	    ajDebug("resourceinRead: (e3) "
                    "resourceinReadFmt stat == EMPTY *try again*\n");
	    break;

        default:
	    ajDebug("unknown code %d from resourceinReadFmt\n", stat);
	}

	ajResourceClear(resource); /* 1 : read, failed to match id/acc/query */
    }

    if(resourcein->Input->Format)
	ajDebug("resourceinRead: *failed* to read data data %S "
                "using format %s\n",
		resourcein->Input->Qry,
                resourceinFormatDef[resourcein->Input->Format].Name);
    else
	ajDebug("resourceinRead: *failed* to read data data %S "
                "using any format\n",
		resourcein->Input->Qry);

    return ajFalse;
}




/* @funcstatic resourceinReadDrcat ********************************************
**
** Given data in a data structure, tries to read everything needed
** using drcat format.
**
** @param [u] resourcein [AjPResourcein] Resource input object
** @param [w] resource [AjPResource] resource object
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool resourceinReadDrcat(AjPResourcein resourcein,
                                  AjPResource resource)
{
    AjPFilebuff buff;

    AjPStr name = NULL;
    AjPStr rest = NULL;

    ajlong fpos     = 0;
    ajuint linecnt = 0;
    AjBool ok = ajTrue;

    AjPStrTok handle = NULL;
    AjPResquery resqry = NULL;
    AjPReslink reslink = NULL;
    AjPResterm resterm = NULL;
    AjPStr token = NULL;
    AjIList iter = NULL;

    ajDebug("resourceinReadDrcat\n");
    ajResourceClear(resource);
    buff = resourcein->Input->Filebuff;

    /* ajFilebuffTrace(buff); */

    ok = ajBuffreadLine(buff, &resourceinReadLine);

    while (ok && !ajStrPrefixC(resourceinReadLine, "ID      "))
    {
        ok = ajBuffreadLine(buff, &resourceinReadLine);
    }

    if(!ok)
        return ajFalse;

    ajFilebuffClear(buff, 1);

    ok = ajBuffreadLinePosStore(buff, &resourceinReadLine, &fpos,
                                resourcein->Input->Text,
                                &resource->TextPtr);

    ajStrExtractFirst(resourceinReadLine, &rest, &name);
    ajStrTrimWhite(&rest);
    ajStrAssignS(&resource->Id, rest);

    if(ok)
        ok = ajBuffreadLinePosStore(buff, &resourceinReadLine, &fpos,
                                    resourcein->Input->Text,
                                    &resource->TextPtr);

    while(ok && ajStrGetLen(resourceinReadLine) > 1)
    {
        linecnt++;
        ajStrExtractFirst(resourceinReadLine, &rest, &name);
        ajStrTrimWhite(&rest);

        if(linecnt > 1 && ajStrMatchC(name, "ID"))
        {
            break;
        }
        else if(ajStrMatchC(name, "IDalt"))
        {
            if(!ajStrMatchC(rest, "None"))
                ajListstrPushAppend(resource->Idalt, ajStrNewS(rest));
        }
        else if(ajStrMatchC(name, "Acc"))
        {
            if(!ajStrMatchC(rest, "None"))
                ajStrAssignS(&resource->Acc, rest);
        }
        else if(ajStrMatchC(name, "Name"))
        {
            if(!ajStrMatchC(rest, "None"))
                ajStrAssignS(&resource->Name, rest);
        }
        else if(ajStrMatchC(name, "Desc"))
        {
            if(!ajStrMatchC(rest, "None"))
                ajStrAssignS(&resource->Desc, rest);
        }
        else if(ajStrMatchC(name, "URL"))
        {
            if(!ajStrMatchC(rest, "None"))
                ajStrAssignS(&resource->Url, rest);
        }
        else if(ajStrMatchC(name, "URLlink"))
        {
            if(!ajStrMatchC(rest, "None"))
                ajStrAssignS(&resource->Urllink, rest);
        }
        else if(ajStrMatchC(name, "URLrest"))
        {
            if(!ajStrMatchC(rest, "None"))
                ajStrAssignS(&resource->Urlrest, rest);
        }
        else if(ajStrMatchC(name, "URLsoap"))
        {
            if(!ajStrMatchC(rest, "None"))
                ajStrAssignS(&resource->Urlsoap, rest);
        }
        else if(ajStrMatchC(name, "Cat"))
        {
            if(!ajStrMatchC(rest, "None"))
            {
                ajStrTokenAssignC(&handle, rest, ";");
                while(ajStrTokenNextParse(&handle, &token))
                {
                    ajStrRemoveWhiteExcess(&token);
                    ajListstrPushAppend(resource->Cat, ajStrNewS(token));
                }
            }
        }
        else if(ajStrMatchC(name, "EDAMtpc"))
        {
            if(!ajStrMatchC(rest, "None"))
            {
                resterm = ajRestermNew();

                ajStrTokenAssignC(&handle, rest, "|");

                ajStrTokenNextParse(&handle, &token);
                ajStrRemoveWhiteExcess(&token);
                ajStrAssignS(&resterm->Id, token);

                ajStrTokenRestParse(&handle, &token);
                ajStrRemoveWhiteExcess(&token);
                ajStrAssignS(&resterm->Name, token);

                ajListPushAppend(resource->Edamtpc, resterm);
                resterm = NULL;
            }
        }
        else if(ajStrMatchC(name, "EDAMdat"))
        {
            if(!ajStrMatchC(rest, "None"))
            {
                resterm = ajRestermNew();

                ajStrTokenAssignC(&handle, rest, "|");

                ajStrTokenNextParse(&handle, &token);
                ajStrRemoveWhiteExcess(&token);
                ajStrAssignS(&resterm->Id, token);

                ajStrTokenRestParse(&handle, &token);
                ajStrRemoveWhiteExcess(&token);
                ajStrAssignS(&resterm->Name, token);

                ajListPushAppend(resource->Edamdat, resterm);
                resterm = NULL;
            }
        }
        else if(ajStrMatchC(name, "EDAMid"))
        {
            if(!ajStrMatchC(rest, "None"))
            {
                resterm = ajRestermNew();

                ajStrTokenAssignC(&handle, rest, "|");

                ajStrTokenNextParse(&handle, &token);
                ajStrRemoveWhiteExcess(&token);
                ajStrAssignS(&resterm->Id, token);

                ajStrTokenRestParse(&handle, &token);
                ajStrRemoveWhiteExcess(&token);
                ajStrAssignS(&resterm->Name, token);

                ajListPushAppend(resource->Edamid, resterm);
                resterm = NULL;
            }
        }
        else if(ajStrMatchC(name, "EDAMfmt"))
        {
            if(!ajStrMatchC(rest, "None"))
            {
                resterm = ajRestermNew();

                ajStrTokenAssignC(&handle, rest, "|");

                ajStrTokenNextParse(&handle, &token);
                ajStrRemoveWhiteExcess(&token);
                ajStrAssignS(&resterm->Id, token);

                ajStrTokenRestParse(&handle, &token);
                ajStrRemoveWhiteExcess(&token);
                ajStrAssignS(&resterm->Name, token);

                ajListPushAppend(resource->Edamfmt, resterm);
                resterm = NULL;
            }
        }
        else if(ajStrMatchC(name, "Xref"))
        {
            if(!ajStrMatchC(rest, "None"))
            {
                reslink = ajReslinkNew();

                ajStrTokenAssignC(&handle, rest, "|");

                ajStrTokenNextParse(&handle, &token);
                ajStrRemoveWhiteExcess(&token);
                ajStrAssignS(&reslink->Source, token);

                ajStrTokenRestParse(&handle, &token);
                ajStrRemoveWhiteExcess(&token);
                ajStrAssignS(&reslink->Term, token);
                reslink->Nterms = (ajint) ajStrCalcCountK(token, ';');

                ajListPushAppend(resource->Xref, reslink);
                reslink = NULL;
            }
        }
        else if(ajStrMatchC(name, "Query"))
        {
            if(!ajStrMatchC(rest, "None"))
            {
                resqry = ajResqueryNew();

                ajStrTokenAssignC(&handle, rest, "|");
                ajStrTokenNextParse(&handle, &token);
                ajStrRemoveWhiteExcess(&token);
                ajStrAssignS(&resqry->Datatype, token);

                ajStrTokenNextParse(&handle, &token);
                ajStrRemoveWhiteExcess(&token);
                ajStrAssignS(&resqry->Format, token);

                iter = ajListIterNew(resource->Edamfmt);
                while(!ajListIterDone(iter))
                {
                    resterm = ajListIterGet(iter);
                    if(ajStrMatchS(resqry->Format, resterm->Name))
                        ajStrAssignS(&resqry->FormatTerm, resterm->Id);
                }
                ajListIterDel(&iter);

                ajStrTokenNextParse(&handle, &token);
                ajStrRemoveWhiteExcess(&token);
                ajStrAssignS(&resqry->Term, token);
                resqry->Nterms = (ajint) ajStrCalcCountK(token, ';');
    
                ajStrTokenRestParse(&handle, &token);
                ajStrRemoveWhiteExcess(&token);
                ajStrAssignS(&resqry->Url, token);

                ajListPushAppend(resource->Query, resqry);
                resqry = NULL;
            }
        }
        else if(ajStrMatchC(name, "Example"))
        {
            if(!ajStrMatchC(rest, "None"))
                ajListstrPushAppend(resource->Example, ajStrNewS(rest));
        }
        else if(ajStrMatchC(name, "Taxon"))
        {
            if(!ajStrMatchC(rest, "None"))
            {
                resterm = ajRestermNew();

                ajStrTokenAssignC(&handle, rest, "|");

                ajStrTokenNextParse(&handle, &token);
                ajStrRemoveWhiteExcess(&token);
                ajStrAssignS(&resterm->Id, token);

                ajStrTokenRestParse(&handle, &token);
                ajStrRemoveWhiteExcess(&token);
                ajStrAssignS(&resterm->Name, token);

                ajListPushAppend(resource->Taxon, resterm);
                resterm = NULL;
            }
        }
        ok = ajBuffreadLinePosStore(buff, &resourceinReadLine, &fpos,
                                    resourcein->Input->Text,
                                    &resource->TextPtr);
    }

    if(ok)
        ajFilebuffClearStore(buff, 1,
                             resourceinReadLine,
                             resourcein->Input->Text,
                             &resource->TextPtr);
    else
        ajFilebuffClear(buff, 0);

    ajStrDel(&name);
    ajStrDel(&rest);
    ajStrDel(&token);
    ajStrTokenDel(&handle);

    return ajTrue;
}




/* @datasection [none] Miscellaneous ******************************************
**
** Data input internals
**
** @nam2rule Resourcein Data input
**
******************************************************************************/




/* @section Printing **********************************************************
**
** Printing details of the internals to a file
**
** @fdata [none]
**
** @nam2rule Resourceinprint
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




/* @func ajResourceinprintBook ************************************************
**
** Reports the internal data structures as a Docbook table
**
** @param [u] outf [AjPFile] Output file
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajResourceinprintBook(AjPFile outf)
{
    ajuint i = 0;
    ajuint j = 0;
    AjPStr namestr = NULL;
    AjPList fmtlist;
    AjPStr* names;

    fmtlist = ajListstrNew();

    ajFmtPrintF(outf, "<para>The supported data resource formats"
                "are summarised in the table below. "
                "The columns are as follows: "
                "<emphasis>Input format</emphasis> (format name), "
                "<emphasis>Try</emphasis> (indicates whether the "
                "format can be detected automatically on input), and "
                "<emphasis>Description</emphasis> (short description of "
                "the format).</para>\n\n");

    ajFmtPrintF(outf, "<table frame=\"box\" rules=\"cols\">\n");
    ajFmtPrintF(outf, "  <caption>Input data resource formats</caption>\n");
    ajFmtPrintF(outf, "  <thead>\n");
    ajFmtPrintF(outf, "    <tr align=\"center\">\n");
    ajFmtPrintF(outf, "      <th>Input Format</th>\n");
    ajFmtPrintF(outf, "      <th>Try</th>\n");
    ajFmtPrintF(outf, "      <th>Description</th>\n");
    ajFmtPrintF(outf, "    </tr>\n");
    ajFmtPrintF(outf, "  </thead>\n");
    ajFmtPrintF(outf, "  <tbody>\n");

    for(i=1; resourceinFormatDef[i].Name; i++)
    {
	if(!resourceinFormatDef[i].Alias)
        {
            namestr = ajStrNewC(resourceinFormatDef[i].Name);
            ajListPushAppend(fmtlist, namestr);
            namestr = NULL;
        }
    }

    ajListSort(fmtlist, &ajStrVcmp);
    ajListstrToarray(fmtlist, &names);

    for(i=0; names[i]; i++)
    {
        for(j=0; resourceinFormatDef[j].Name; j++)
        {
            if(ajStrMatchC(names[i],resourceinFormatDef[j].Name))
            {
                ajFmtPrintF(outf, "    <tr>\n");
                ajFmtPrintF(outf, "      <td>%s</td>\n",
                            resourceinFormatDef[j].Name);
                ajFmtPrintF(outf, "      <td>%B</td>\n",
                            resourceinFormatDef[j].Try);
                ajFmtPrintF(outf, "      <td>%s</td>\n",
                            resourceinFormatDef[j].Desc);
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




/* @func ajResourceinprintHtml ************************************************
**
** Reports the internal data structures as an HTML table
**
** @param [u] outf [AjPFile] Output file
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajResourceinprintHtml(AjPFile outf)
{
    ajuint i = 0;
    ajuint j = 0;

    AjPStr namestr = NULL;

    ajFmtPrintF(outf, "<table border=3>");
    ajFmtPrintF(outf, "<tr><th>Input Format</th><th>Auto</th>\n");
    ajFmtPrintF(outf, "<th>Multi</th><th>Description</th></tr>\n");

    for(i=1; resourceinFormatDef[i].Name; i++)
    {
        ajStrAssignC(&namestr, resourceinFormatDef[i].Name);

	if(!resourceinFormatDef[i].Alias)
        {
            for(j=i+1; resourceinFormatDef[j].Name; j++)
            {
                if(resourceinFormatDef[j].Read == resourceinFormatDef[i].Read)
                {
                    ajFmtPrintAppS(&namestr, " %s",
                                   resourceinFormatDef[j].Name);
                    if(!resourceinFormatDef[j].Alias) 
                    {
                        ajWarn("Input format '%s' same as '%s' but not alias",
                               resourceinFormatDef[j].Name,
                               resourceinFormatDef[i].Name);
                    }
                }
            }

	    ajFmtPrintF(outf, "<tr><td>\n%S\n</td><td>%B</td>\n",
                        namestr,
			resourceinFormatDef[i].Try);
            ajFmtPrintF(outf, "<td>\n%s\n</td></tr>\n",
			resourceinFormatDef[i].Desc);
        }

    }

    ajFmtPrintF(outf, "</table>\n");
    ajStrDel(&namestr);

    return;
}




/* @func ajResourceinprintText ************************************************
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

void ajResourceinprintText(AjPFile outf, AjBool full)
{
    ajuint i = 0;

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# Data resource input formats\n");
    ajFmtPrintF(outf, "# Name  Format name (or alias)\n");
    ajFmtPrintF(outf, "# Alias Alias name\n");
    ajFmtPrintF(outf, "# Try   Test for unknown input files\n");
    ajFmtPrintF(outf, "# Name         Alias Try "
		"Description");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "InFormat {\n");

    for(i=0; resourceinFormatDef[i].Name; i++)
	if(full || !resourceinFormatDef[i].Alias)
	    ajFmtPrintF(outf,
			"  %-12s %5B %3B \"%s\"\n",
			resourceinFormatDef[i].Name,
			resourceinFormatDef[i].Alias,
			resourceinFormatDef[i].Try,
			resourceinFormatDef[i].Desc);

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajResourceinprintWiki ************************************************
**
** Reports the internal data structures as a wiki table
**
** @param [u] outf [AjPFile] Output file
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajResourceinprintWiki(AjPFile outf)
{
    ajuint i = 0;
    ajuint j = 0;

    AjPStr namestr = NULL;

    ajFmtPrintF(outf, "{| class=\"wikitable sortable\" border=\"2\"\n");
    ajFmtPrintF(outf, "|-\n");
    ajFmtPrintF(outf, "!Format!!Try!!"
                "class=\"unsortable\"|Description\n");

    for(i=1; resourceinFormatDef[i].Name; i++)
    {
        ajStrAssignC(&namestr, resourceinFormatDef[i].Name);

	if(!resourceinFormatDef[i].Alias)
        {
            for(j=i+1; resourceinFormatDef[j].Name; j++)
            {
                if(resourceinFormatDef[j].Read == resourceinFormatDef[i].Read)
                {
                    ajFmtPrintAppS(&namestr, "<br>%s",
                                   resourceinFormatDef[j].Name);
                    if(!resourceinFormatDef[j].Alias) 
                    {
                        ajWarn("Input format '%s' same as '%s' but not alias",
                               resourceinFormatDef[j].Name,
                               resourceinFormatDef[i].Name);
                    }
                }
            }

            ajFmtPrintF(outf, "|-\n");
	    ajFmtPrintF(outf,
			"|%S||%B||%s\n",
			namestr,
			resourceinFormatDef[i].Try,
			resourceinFormatDef[i].Desc);
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




/* @func ajResourceinExit *****************************************************
**
** Cleans up data input internal memory
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajResourceinExit(void)
{
    ajStrDel(&resourceinReadLine);

    ajTableDel(&resourceDbMethods);

    return;
}




/* @section Internals *********************************************************
**
** Functions to return internal values
**
** @fdata [none]
**
** @nam3rule Type Internals for data resource datatype
** @nam4rule Get  Return a value
** @nam5rule Fields  Known query fields for ajResourceinRead
** @nam5rule Qlinks  Known query link operators for ajResourceinRead
**
** @valrule * [const char*] Internal value
**
** @fcategory misc
**
******************************************************************************/




/* @func ajResourceinTypeGetFields ********************************************
**
** Returns the listof known field names for ajResourceinRead
**
** @return [const char*] List of field names
**
** @release 6.4.0
** @@
******************************************************************************/

const char* ajResourceinTypeGetFields(void)
{
    return "id acc";
}




/* @func ajResourceinTypeGetQlinks ********************************************
**
** Returns the listof known query link operators for ajResourceinRead
**
** @return [const char*] List of field names
**
** @release 6.4.0
** @@
******************************************************************************/

const char* ajResourceinTypeGetQlinks(void)
{
    return "|";
}




/* @datasection [AjPTable] Internal call register table ***********************
**
** Functions to manage the internal call register table that links the
** ajaxdb library functions with code in the core AJAX library.
**
** @nam2rule Resourceaccess Functions to manage resourcedb call tables.
**
******************************************************************************/




/* @section Cast **************************************************************
**
** Return a reference to the call table
**
** @fdata [AjPTable] resourcedb functions call table
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




/* @func ajResourceaccessGetDb ************************************************
**
** Returns the table in which data database access details are registered
**
** @return [AjPTable] Access functions hash table
**
** @release 6.4.0
** @@
******************************************************************************/

AjPTable ajResourceaccessGetDb(void)
{
    if(!resourceDbMethods)
        resourceDbMethods = ajCallTableNew();
    return resourceDbMethods;
    
}




/* @func ajResourceaccessMethodGetQlinks **************************************
**
** Tests for a named method for data data reading and returns the 
** known query link operators
**
** @param [r] method [const AjPStr] Method required.
** @return [const char*] Known link operators
**
** @release 6.4.0
** @@
******************************************************************************/

const char* ajResourceaccessMethodGetQlinks(const AjPStr method)
{
    AjPResourceAccess methoddata; 

    methoddata = ajCallTableGetS(resourceDbMethods, method);
    if(!methoddata)
        return NULL;

    return methoddata->Qlink;
}




/* @func ajResourceaccessMethodGetScope ***************************************
**
** Tests for a named method for data data reading and returns the scope
** (entry, query or all).
*
** @param [r] method [const AjPStr] Method required.
** @return [ajuint] Scope flags
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ajResourceaccessMethodGetScope(const AjPStr method)
{
    AjPResourceAccess methoddata; 
    ajuint ret = 0;

    methoddata = ajCallTableGetS(resourceDbMethods, method);
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




/* @func ajResourceaccessMethodTest *******************************************
**
** Tests for a named method for data data reading.
**
** @param [r] method [const AjPStr] Method required.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajResourceaccessMethodTest(const AjPStr method)
{
    if(ajCallTableGetS(resourceDbMethods, method))
      return ajTrue;

    return ajFalse;
}




/* @funcstatic resourceinQryRestore *******************************************
**
** Restores a data input specification from an AjPQueryList node
**
** @param [w] resourcein [AjPResourcein] Data resource input object
** @param [r] node [const AjPQueryList] Query list node
** @return [void]
**
** @release 6.4.0
******************************************************************************/

static void resourceinQryRestore(AjPResourcein resourcein,
                                 const AjPQueryList node)
{
    resourcein->Input->Format = node->Format;
    ajStrAssignS(&resourcein->Input->Formatstr, node->Formatstr);

    return;
}




/* @funcstatic resourceinQrySave **********************************************
**
** Saves a data input specification in an AjPQueryList node
**
** @param [w] node [AjPQueryList] Query list node
** @param [r] resourcein [const AjPResourcein] Data resource input object
** @return [void]
**
** @release 6.4.0
******************************************************************************/

static void resourceinQrySave(AjPQueryList node,
                              const AjPResourcein resourcein)
{
    node->Format   = resourcein->Input->Format;
    ajStrAssignS(&node->Formatstr, resourcein->Input->Formatstr);

    return;
}




/* @funcstatic resourceinQryProcess *******************************************
**
** Converts a data data query into an open file.
**
** Tests for "format::" and sets this if it is found
**
** Then tests for "list:" or "@" and processes as a list file
** using resourceinListProcess which in turn invokes resourceinQryProcess
** until a valid query is found.
**
** Then tests for dbname:query and opens the file (at the correct position
** if the database definition defines it)
**
** If there is no database, looks for file:query and opens the file.
** In this case the file position is not known and data data reading
** will have to scan for the entry/entries we need.
**
** @param [u] resourcein [AjPResourcein] Data resource data input structure.
** @param [u] resource [AjPResource] Data resource data to be read.
**                         The format will be replaced
**                         if defined in the query string.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool resourceinQryProcess(AjPResourcein resourcein,
                                   AjPResource resource)
{
    AjBool ret = ajTrue;
    AjPStr qrystr = NULL;
    AjBool resourcemethod = ajFalse;
    const AjPStr fmtstr = NULL;
    AjPTextin textin;
    AjPQuery qry;
    AjPResourceAccess resourceaccess = NULL;

    textin = resourcein->Input;
    qry = textin->Query;

    /* pick up the original query string */
    qrystr = ajStrNewS(textin->Qry);

    ajDebug("resourceinQryProcess '%S'\n", qrystr);

    /* look for a format:: prefix */
    fmtstr = ajQuerystrParseFormat(&qrystr, textin, resourceinformatFind);
    ajDebug("resourceinQryProcess ... fmtstr '%S' '%S'\n", fmtstr, qrystr);

    /* (seq/feat) DO NOT look for a [range] suffix */

    /* look for a list:: or @:: listfile of queries  - process and return */
    if(ajQuerystrParseListfile(&qrystr))
    {
        ajDebug("resourceinQryProcess ... listfile '%S'\n", qrystr);
        ret = resourceinListProcess(resourcein, resource, qrystr);
        ajStrDel(&qrystr);
        return ret;
    }

    /* try general text access methods (file, asis, text database access */
    ajDebug("resourceinQryProcess ... no listfile '%S'\n", qrystr);
    if(!ajQuerystrParseRead(&qrystr, textin, resourceinformatFind,
                            &resourcemethod))
    {
        ajStrDel(&qrystr);
        return ajFalse;
    }
    
    resourceinFormatSet(resourcein, resource);

    ajDebug("resourceinQryProcess ... read nontext: %B '%S'\n",
            resourcemethod, qrystr);
    ajStrDel(&qrystr);

    /* we found a non-text method */
    if(resourcemethod)
    {
        ajDebug("resourceinQryProcess ... call method '%S'\n", qry->Method);
        ajDebug("resourceinQryProcess ... textin format %d '%S'\n",
                textin->Format, textin->Formatstr);
        ajDebug("resourceinQryProcess ...  query format  '%S'\n",
                qry->Formatstr);
        qry->Access = ajCallTableGetS(resourceDbMethods,qry->Method);
        resourceaccess = qry->Access;
        return (*resourceaccess->Access)(resourcein);
    }

    ajDebug("resourceinQryProcess text method '%S' success\n", qry->Method);

    return ajTrue;
}





/* @datasection [AjPList] Query field list ************************************
**
** Query fields lists are handled internally. Only static functions
** should appear here
**
******************************************************************************/




/* @funcstatic resourceinListProcess ******************************************
**
** Processes a file of queries.
** This function is called by, and calls, resourceinQryProcess. There is
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
** @param [u] resourcein [AjPResourcein] Data resource data input
** @param [u] resource [AjPResource] Data resource data
** @param [r] listfile [const AjPStr] Name of list file.,
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool resourceinListProcess(AjPResourcein resourcein,
                                    AjPResource resource,
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
    ajDebug("++resourceinListProcess %S depth %d\n",
	    listfile, depth);

    if(depth > MAXDEPTH)
	ajFatal("Query list too deep");

    if(!resourcein->Input->List)
	resourcein->Input->List = ajListNew();

    list = ajListNew();

    file = ajFileNewInNameS(listfile);

    if(!file)
    {
	ajErr("Failed to open list file '%S'", listfile);
	depth--;

	return ret;
    }

    while(ajReadlineTrim(file, &resourceinReadLine))
    {
	resourceinListNoComment(&resourceinReadLine);

	if(ajStrGetLen(resourceinReadLine))
	{
	    ajStrTokenAssignC(&handle, resourceinReadLine, " \t\n\r");
	    ajStrTokenNextParse(&handle, &token);
	    /* ajDebug("Line  '%S'\n");*/
	    /* ajDebug("token '%S'\n", resourceinReadLine, token); */

	    if(ajStrGetLen(token))
	    {
	        ajDebug("++Add to list: '%S'\n", token);
	        AJNEW0(node);
	        ajStrAssignS(&node->Qry, token);
	        resourceinQrySave(node, resourcein);
	        ajListPushAppend(list, node);
	    }

	    ajStrDel(&token);
	    token = NULL;
	}
    }

    ajFileClose(&file);
    ajStrDel(&token);

    ajDebug("Trace resourcein->Input->List\n");
    ajQuerylistTrace(resourcein->Input->List);
    ajDebug("Trace new list\n");
    ajQuerylistTrace(list);
    ajListPushlist(resourcein->Input->List, &list);

    ajDebug("Trace combined resourcein->Input->List\n");
    ajQuerylistTrace(resourcein->Input->List);

    /*
     ** now try the first item on the list
     ** this can descend recursively if it is also a list
     ** which is why we check the depth above
     */

    if(ajListPop(resourcein->Input->List, (void**) &node))
    {
        ajDebug("++pop first item '%S'\n", node->Qry);
	ajResourceinQryS(resourcein, node->Qry);
	resourceinQryRestore(resourcein, node);
	ajStrDel(&node->Qry);
	ajStrDel(&node->Formatstr);
	AJFREE(node);
	ajDebug("descending with query '%S'\n", resourcein->Input->Qry);
	ret = resourceinQryProcess(resourcein, resource);
    }

    ajStrTokenDel(&handle);
    depth--;
    ajDebug("++resourceinListProcess depth: %d returns: %B\n", depth, ret);

    return ret;
}




/* @funcstatic resourceinListNoComment ****************************************
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

static void resourceinListNoComment(AjPStr* text)
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




/* @funcstatic resourceinFormatSet ********************************************
**
** Sets the input format for data data using the data data
** input object's defined format
**
** @param [u] resourcein [AjPResourcein] Data resource data input.
** @param [u] resource [AjPResource] Data resource data
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool resourceinFormatSet(AjPResourcein resourcein,
                                  AjPResource resource)
{

    if(ajStrGetLen(resourcein->Input->Formatstr))
    {
	ajDebug("... input format value '%S'\n",
                resourcein->Input->Formatstr);

	if(resourceinformatFind(resourcein->Input->Formatstr,
                             &resourcein->Input->Format))
	{
	    ajStrAssignS(&resource->Formatstr,
                         resourcein->Input->Formatstr);
	    resource->Format = resourcein->Input->Format;
	    ajDebug("...format OK '%S' = %d\n",
                    resourcein->Input->Formatstr,
		    resourcein->Input->Format);
	}
	else
	    ajDebug("...format unknown '%S'\n",
                    resourcein->Input->Formatstr);

	return ajTrue;
    }
    else
	ajDebug("...input format not set\n");


    return ajFalse;
}




/* @datasection [AjPResourceall] Data Resource Input Stream ********************
**
** Function is for manipulating data resource input stream objects
**
** @nam2rule Resourceall Data resource input stream objects
**
******************************************************************************/




/* @section Data Resource Input Constructors **********************************
**
** All constructors return a new data resource input stream object by pointer.
** It is the responsibility of the user to first destroy any previous
** data resource input object. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so
** anyway.
**
** @fdata [AjPResourceall]
**
** @nam3rule New Constructor
**
** @valrule * [AjPResourceall] Data resource input stream object
**
** @fcategory new
**
******************************************************************************/




/* @func ajResourceallNew *****************************************************
**
** Creates a new data resource input stream object.
**
** @return [AjPResourceall] New data resource input stream object.
**
** @release 6.4.0
** @@
******************************************************************************/

AjPResourceall ajResourceallNew(void)
{
    AjPResourceall pthis;

    AJNEW0(pthis);

    pthis->Resourcein = ajResourceinNew();
    pthis->Resource   = ajResourceNew();

    return pthis;
}





/* ==================================================================== */
/* ========================== destructors ============================= */
/* ==================================================================== */




/* @section Data Resource Input Stream Destructors ****************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the data resource input stream object.
**
** @fdata [AjPResourceall]
**
** @nam3rule Del Destructor
**
** @argrule Del pthis [AjPResourceall*] Data resource input stream
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajResourceallDel *****************************************************
**
** Deletes a data resource input stream object.
**
** @param [d] pthis [AjPResourceall*] Data resource input stream
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajResourceallDel(AjPResourceall* pthis)
{
    AjPResourceall thys;

    if(!pthis)
        return;

    thys = *pthis;

    if(!thys)
        return;

    ajResourceinDel(&thys->Resourcein);
    if(!thys->Returned)
        ajResourceDel(&thys->Resource);

    AJFREE(*pthis);

    return;
}




/* ==================================================================== */
/* =========================== Modifiers ============================== */
/* ==================================================================== */




/* @section data resource input stream modifiers *******************************
**
** These functions use the contents of an data resource input stream object and
** update them.
**
** @fdata [AjPResourceall]
**
** @nam3rule Clear Clear all values
**
** @argrule * thys [AjPResourceall] Data resource input stream object
**
** @valrule * [void]
**
** @fcategory modify
**
******************************************************************************/




/* @func ajResourceallClear ***************************************************
**
** Clears a data resource term input stream object back to "as new" condition,
** except for the query list which must be preserved.
**
** @param [w] thys [AjPResourceall] Data resource input stream
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajResourceallClear(AjPResourceall thys)
{

    ajDebug("ajResourceallClear called\n");

    if(!thys)
        return;

    ajResourceinClear(thys->Resourcein);

    ajResourceClear(thys->Resource);

    thys->Returned = ajFalse;

    return;
}




/* @section Data resource input stream casts **********************************
**
** These functions return the contents of a data resource input stream object
**
** @fdata [AjPResourceall]
**
** @nam3rule Get Get data resource input stream values
** @nam3rule Getresource Get data resource values
** @nam4rule Id Return identifier
**
** @argrule * thys [const AjPResourceall] Data resource input stream object
**
** @valrule * [const AjPStr] String value
**
** @fcategory cast
**
******************************************************************************/




/* @func ajResourceallGetresourceId *******************************************
**
** Returns the identifier of the current data resource in an input stream
**
** @param [r] thys [const AjPResourceall] Data resource term input stream
** @return [const AjPStr] Identifier
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPStr ajResourceallGetresourceId(const AjPResourceall thys)
{
    if(!thys)
        return NULL;

    ajDebug("ajResourceallGetresourceId called\n");

    return ajResourceGetId(thys->Resource);
}




/* @section resource input ****************************************************
**
** These functions use a data resource input stream object to read data
**
** @fdata [AjPResourceall]
**
** @nam3rule Next Read next data resource
**
** @argrule * thys [AjPResourceall] Data resource input stream object
** @argrule * Presource [AjPResource*] Data resourec object
**
** @valrule * [AjBool] True on success
**
** @fcategory input
**
******************************************************************************/




/* @func ajResourceallNext ****************************************************
**
** Parse a data resource query into format, access, file and entry
**
** Split at delimiters. Check for the first part as a valid format
** Check for the remaining first part as a database name or as a file
** that can be opened.
** Anything left is an entryname spec.
**
** Return the results in the AjPResource object but leave the file open for
** future calls.
**
** @param [w] thys [AjPResourceall] Data resource input stream
** @param [u] Presource [AjPResource*] Data resource returned
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajResourceallNext(AjPResourceall thys, AjPResource *Presource)
{
    if(!thys->Count)
    {
	thys->Count = 1;

	thys->Totresources++;

	*Presource = thys->Resource;
	thys->Returned = ajTrue;

	return ajTrue;
    }


    if(ajResourceinRead(thys->Resourcein, thys->Resource))
    {
	thys->Count++;

	thys->Totresources++;

	*Presource = thys->Resource;
	thys->Returned = ajTrue;

	ajDebug("ajResourceallNext success\n");

	return ajTrue;
    }

    *Presource = NULL;

    ajDebug("ajResourceallNext failed\n");

    ajResourceallClear(thys);

    return ajFalse;
}




/* @datasection [none] Input formats ******************************************
**
** Input formats internals
**
** @nam2rule Resourceinformat Data data input format specific
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




/* @funcstatic resourceinformatFind *******************************************
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

static AjBool resourceinformatFind(const AjPStr format, ajint* iformat)
{
    AjPStr tmpformat = NULL;
    ajuint i = 0;

    /* ajDebug("resourceinformatFind '%S'\n", format); */
    if(!ajStrGetLen(format))
	return ajFalse;

    ajStrAssignS(&tmpformat, format);
    ajStrFmtLower(&tmpformat);

    for(i=0; resourceinFormatDef[i].Name; i++)
    {
	/* ajDebug("test %d '%s'\n",
           i, resourceinFormatDef[i].Name); */
	if(ajStrMatchC(tmpformat, resourceinFormatDef[i].Name))
	{
	    *iformat = i;
	    ajStrDel(&tmpformat);
	    /* ajDebug("found '%s' at %d\n", resourceinFormatDef[i].Name, i); */
	    return ajTrue;
	}
    }

    ajErr("Unknown input format '%S'", format);

    ajStrDel(&tmpformat);

    return ajFalse;
}




/* @func ajResourceinformatTerm ***********************************************
**
** Tests whether a data resource data input format term is known
**
** @param [r] term [const AjPStr] Format term EDAM ID
** @return [AjBool] ajTrue if term was accepted
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajResourceinformatTerm(const AjPStr term)
{
    ajuint i;

    for(i=0; resourceinFormatDef[i].Name; i++)
	if(ajStrMatchCaseC(term, resourceinFormatDef[i].Name))
	    return ajTrue;

    return ajFalse;
}




/* @func ajResourceinformatTest ***********************************************
**
** Tests whether a named data data input format is known
**
** @param [r] format [const AjPStr] Format term EDAM ID
** @return [AjBool] ajTrue if format was accepted
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajResourceinformatTest(const AjPStr format)
{
    ajuint i;

    for(i=0; resourceinFormatDef[i].Name; i++)
	if(ajStrMatchCaseC(format, resourceinFormatDef[i].Name))
	    return ajTrue;

    return ajFalse;
}

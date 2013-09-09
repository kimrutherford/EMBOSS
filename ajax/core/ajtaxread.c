/* @source ajtaxread **********************************************************
**
** AJAX taxonomy reading functions
**
** These functions control all aspects of AJAX taxonomy reading
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.31 $
** @modified Oct 5 pmr First version
** @modified $Date: 2012/12/07 10:10:52 $ by $Author: rice $
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

#include "ajtaxread.h"
#include "ajtax.h"
#include "ajcall.h"
#include "ajlist.h"
#include "ajquery.h"
#include "ajtextread.h"
#include "ajnam.h"
#include "ajfileio.h"

#include <string.h>


AjPTable taxDbMethods = NULL;

static AjPStr taxinReadLine     = NULL;

#define TAXFLAG_INHERITDIV    0x01
#define TAXFLAG_INHERITCODE   0x02
#define TAXFLAG_INHERITMITO   0x04
#define TAXFLAG_HIDDENGENBANK 0x10
#define TAXFLAG_HIDDENSUBTREE 0x20


static AjBool taxinReadEbi(AjPTaxin thys, AjPTax tax);
static AjBool taxinReadNcbi(AjPTaxin thys, AjPTax tax);




/* @datastatic TaxPInFormat *************************************************
**
** Taxonomy input formats data structure
**
** @alias TaxSInFormat
** @alias TaxOInFormat
**
** @attr Name [const char*] Format name
** @attr Desc [const char*] Format description
** @attr Alias [AjBool] Name is an alias for an identical definition
** @attr Try [AjBool] If true, try for an unknown input. Duplicate names
**                    and read-anything formats are set false
** @attr Read [AjBool function] Input function, returns ajTrue on success
** @@
******************************************************************************/

typedef struct TaxSInFormat
{
    const char *Name;
    const char *Desc;
    AjBool Alias;
    AjBool Try;
    AjBool (*Read) (AjPTaxin thys, AjPTax tax);
} TaxOInFormat;

#define TaxPInFormat TaxOInFormat*

static TaxOInFormat taxinFormatDef[] =
{
/* "Name",        "Description" */
/*     Alias,   Try,     */
/*     ReadFunction */
  {"unknown",     "Unknown format",
       AJFALSE, AJFALSE,
       &taxinReadNcbi}, /* default to first format */
  {"ncbi",          "NCBI taxonomy format",
       AJFALSE, AJTRUE,
       &taxinReadNcbi},
  {"taxonomy",      "EBI taxonomy format",
       AJFALSE, AJTRUE,
       &taxinReadEbi},
  {NULL, NULL, 0, 0, NULL}
};



static ajuint taxinReadFmt(AjPTaxin taxin, AjPTax tax,
                           ajuint format);
static AjBool taxinRead(AjPTaxin taxin, AjPTax tax);
static AjBool taxinformatFind(const AjPStr format, ajint* iformat);
static AjBool taxinFormatSet(AjPTaxin taxin, AjPTax tax);
static AjBool taxinListProcess(AjPTaxin taxin, AjPTax tax,
                               const AjPStr listfile);
static void taxinListNoComment(AjPStr* text);
static void taxinQryRestore(AjPTaxin taxin, const AjPQueryList node);
static void taxinQrySave(AjPQueryList node, const AjPTaxin taxin);
static AjBool taxDefine(AjPTax thys, AjPTaxin taxin);
static AjBool taxinQryProcess(AjPTaxin taxin, AjPTax tax);
static AjBool taxinQueryMatch(const AjPQuery thys, const AjPTax tax);




/* @filesection ajtaxread ****************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
*/




/* @datasection [AjPTaxin] Taxonomy input objects ***************************
**
** Function is for manipulating taxonomy input objects
**
** @nam2rule Taxin
******************************************************************************/




/* @section Taxonomy input constructors ***************************************
**
** All constructors return a new taxonomy input object by pointer. It
** is the responsibility of the user to first destroy any previous
** taxonomy input object. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so
** anyway.
**
** @fdata [AjPTaxin]
**
** @nam3rule  New     Construct a new taxonomy input object
**
** @valrule   *  [AjPTaxin] New taxonomy input object
**
** @fcategory new
**
******************************************************************************/




/* @func ajTaxinNew ***********************************************************
**
** Creates a new taxonomy input object.
**
** @return [AjPTaxin] New taxonomy input object.
** @category new [AjPTaxin] Default constructor
**
** @release 6.4.0
** @@
******************************************************************************/

AjPTaxin ajTaxinNew(void)
{
    AjPTaxin pthis;

    AJNEW0(pthis);

    pthis->Input = ajTextinNewDatatype(AJDATATYPE_TAXON);

    pthis->TaxData      = NULL;

    return pthis;
}





/* @section taxonomy input destructors ****************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the taxonomy input object.
**
** @fdata [AjPTaxin]
**
** @nam3rule Del Destructor
**
** @argrule Del pthis [AjPTaxin*] Taxonomy input object
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajTaxinDel ***********************************************************
**
** Deletes a taxonomy input object.
**
** @param [d] pthis [AjPTaxin*] Taxonomy input
** @return [void]
** @category delete [AjPTaxin] Default destructor
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTaxinDel(AjPTaxin* pthis)
{
    AjPTaxin thys;

    if(!pthis)
        return;

    thys = *pthis;

    if(!thys)
        return;

    ajDebug("ajTaxinDel called qry:'%S'\n", thys->Input->Qry);

    ajTextinDel(&thys->Input);

    AJFREE(*pthis);

    return;
}




/* @section taxonomy input modifiers ******************************************
**
** These functions use the contents of a taxonomy input object and
** update them.
**
** @fdata [AjPTaxin]
**
** @nam3rule Clear Clear all values
** @nam3rule Qry Reset using a query string
** @suffix C Character string input
** @suffix S String input
**
** @argrule * thys [AjPTaxin] Taxonomy input object
** @argrule C txt [const char*] Query text
** @argrule S str [const AjPStr] query string
**
** @valrule * [void]
**
** @fcategory modify
**
******************************************************************************/




/* @func ajTaxinClear *********************************************************
**
** Clears a taxonomy input object back to "as new" condition, except
** for the query list which must be preserved.
**
** @param [w] thys [AjPTaxin] Taxonomy input
** @return [void]
** @category modify [AjPTaxin] Resets ready for reuse.
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTaxinClear(AjPTaxin thys)
{

    ajDebug("ajTaxinClear called\n");

    ajTextinClear(thys->Input);

    thys->TaxData = NULL;

    return;
}




/* @func ajTaxinQryC **********************************************************
**
** Resets a taxonomy input object using a new Universal
** Query Address
**
** @param [u] thys [AjPTaxin] Taxonomy input object.
** @param [r] txt [const char*] Query
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTaxinQryC(AjPTaxin thys, const char* txt)
{
    ajTaxinClear(thys);
    ajStrAssignC(&thys->Input->Qry, txt);

    return;
}





/* @func ajTaxinQryS **********************************************************
**
** Resets a taxonomy input object using a new Universal
** Query Address
**
** @param [u] thys [AjPTaxin] Taxonomy input object.
** @param [r] str [const AjPStr] Query
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTaxinQryS(AjPTaxin thys, const AjPStr str)
{
    ajTaxinClear(thys);
    ajStrAssignS(&thys->Input->Qry, str);

    return;
}




/* @section casts *************************************************************
**
** Return values
**
** @fdata [AjPTaxin]
**
** @nam3rule Trace Write debugging output
**
** @argrule * thys [const AjPTaxin] Taxonomy input object
**
** @valrule * [void]
**
** @fcategory cast
**
******************************************************************************/




/* @func ajTaxinTrace *********************************************************
**
** Debug calls to trace the data in a taxonomy input object.
**
** @param [r] thys [const AjPTaxin] Taxonomy input object.
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTaxinTrace(const AjPTaxin thys)
{
    ajDebug("taxonomy input trace\n");
    ajDebug("====================\n\n");

    ajTextinTrace(thys->Input);

    if(thys->TaxData)
	ajDebug( "  TaxData: exists\n");

    return;
}




/* @section Taxonomy data inputs **********************************************
**
** These functions read the wxyxdesc data provided by the first argument
**
** @fdata [AjPTaxin]
**
** @nam3rule Read Read taxonomy data
**
** @argrule Read taxin [AjPTaxin] Taxonomy input object
** @argrule Read tax [AjPTax] Taxonomy data
**
** @valrule * [AjBool] true on success
**
** @fcategory input
**
******************************************************************************/




/* @func ajTaxinRead **********************************************************
**
** If the file is not yet open, calls taxinQryProcess to convert the query
** into an open file stream.
**
** Uses taxinRead for the actual file reading.
**
** Returns the results in the AjPTax object.
**
** @param [u] taxin [AjPTaxin] Taxonomy data input definitions
** @param [w] tax [AjPTax] Taxonomy data returned.
** @return [AjBool] ajTrue on success.
** @category input [AjPTax] Master taxonomy data input,
**                  calls specific functions for file access type
**                  and taxonomy data format.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajTaxinRead(AjPTaxin taxin, AjPTax tax)
{
    AjBool ret       = ajFalse;
    AjPQueryList node = NULL;
    AjBool listdata  = ajFalse;

    ajDebug("ajTaxinRead: Filebuff:%x\n",
            taxin->Input->Filebuff);

    if(taxin->Input->Filebuff)
    {
	/* (a) if file still open, keep reading */
	ajDebug("ajTaxinRead: input file '%F' still there, try again\n",
		taxin->Input->Filebuff->File);
	ret = taxinRead(taxin, tax);
	ajDebug("ajTaxinRead: open buffer  qry: '%S' returns: %B\n",
		taxin->Input->Qry, ret);
    }
    else
    {
	/* (b) if we have a list, try the next query in the list */
	if(ajListGetLength(taxin->Input->List))
	{
	    listdata = ajTrue;
	    ajListPop(taxin->Input->List, (void**) &node);

	    ajDebug("++pop from list '%S'\n", node->Qry);
	    ajTaxinQryS(taxin, node->Qry);
	    ajDebug("++SAVE TAXIN '%S' '%S' %d\n",
		    taxin->Input->Qry,
		    taxin->Input->Formatstr, taxin->Input->Format);

            taxinQryRestore(taxin, node);

	    ajStrDel(&node->Qry);
	    ajStrDel(&node->Formatstr);
	    AJFREE(node);

	    ajDebug("ajTaxinRead: open list, try '%S'\n", taxin->Input->Qry);

	    if(!taxinQryProcess(taxin, tax) &&
               !ajListGetLength(taxin->Input->List))
		return ajFalse;

	    ret = taxinRead(taxin, tax);
	    ajDebug("ajTaxinRead: list qry: '%S' returns: %B\n",
		    taxin->Input->Qry, ret);
	}
	else
	{
	    ajDebug("ajTaxinRead: no file yet - test query '%S'\n",
                    taxin->Input->Qry);

	    /* (c) Must be a query - decode it */
	    if(!taxinQryProcess(taxin, tax) &&
               !ajListGetLength(taxin->Input->List))
		return ajFalse;

	    if(ajListGetLength(taxin->Input->List)) /* could be a new list */
		listdata = ajTrue;

	    ret = taxinRead(taxin, tax);
	    ajDebug("ajTaxinRead: new qry: '%S' returns: %B\n",
		    taxin->Input->Qry, ret);
	}
    }

    /* Now read whatever we got */

    while(!ret && ajListGetLength(taxin->Input->List))
    {
	/* Failed, but we have a list still - keep trying it */
        if(listdata)
	    ajErr("Failed to read taxons '%S'", taxin->Input->Qry);

	listdata = ajTrue;
	ajListPop(taxin->Input->List,(void**) &node);
	ajDebug("++try again: pop from list '%S'\n", node->Qry);
	ajTaxinQryS(taxin, node->Qry);
	ajDebug("++SAVE (AGAIN) TAXIN '%S' '%S' %d\n",
		taxin->Input->Qry,
		taxin->Input->Formatstr, taxin->Input->Format);

	taxinQryRestore(taxin, node);

	ajStrDel(&node->Qry);
	ajStrDel(&node->Formatstr);
	AJFREE(node);

	if(!taxinQryProcess(taxin, tax))
	    continue;

	ret = taxinRead(taxin, tax);
	ajDebug("ajTaxinRead: list retry qry: '%S' returns: %B\n",
		taxin->Input->Qry, ret);
    }

    if(!ret)
    {
	if(listdata)
	    ajErr("Failed to read taxon '%S'", taxin->Input->Qry);

	return ajFalse;
    }


    taxDefine(tax, taxin);

    return ajTrue;
}




/* @funcstatic taxinQueryMatch ************************************************
**
** Compares a taxonomy data item to a query and returns true if they match.
**
** @param [r] thys [const AjPQuery] query.
** @param [r] tax [const AjPTax] Taxonomy data.
** @return [AjBool] ajTrue if the taxonomy data matches the query.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool taxinQueryMatch(const AjPQuery thys, const AjPTax tax)
{
    AjBool tested = ajFalse;
    AjIList iterfield  = NULL;
    AjPQueryField field = NULL;
    AjBool ok = ajFalse;

    ajDebug("taxinQueryMatch '%S' fields: %Lu Case %B Done %B\n",
	    tax->Id, ajListGetLength(thys->QueryFields),
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
                    tax->Id);
            if(thys->CaseId)
            {
                if(ajStrMatchWildS(tax->Id, field->Wildquery))
                {
                    ajListIterDel(&iterfield);
                    return ajTrue;
                }
            }
            else
            {
                if(ajStrMatchWildCaseS(tax->Id, field->Wildquery))
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
            if(ajStrMatchWildCaseS(tax->Id, field->Wildquery))
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




/* @funcstatic taxDefine ******************************************************
**
** Make sure all taxonomy data object attributes are defined
** using values from the taxonomy input object if needed
**
** @param [w] thys [AjPTax] Taxonomy data returned.
** @param [u] taxin [AjPTaxin] Taxonomy data input definitions
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool taxDefine(AjPTax thys, AjPTaxin taxin)
{

    /* if values are missing in the taxonomy object, we can use defaults
       from taxin or calculate where possible */

    /* assign the dbname if defined in the taxin object */
    if(ajStrGetLen(taxin->Input->Db))
      ajStrAssignS(&thys->Db, taxin->Input->Db);

    return ajTrue;
}





/* @funcstatic taxinReadFmt ***************************************************
**
** Tests whether taxonomy data can be read using the specified format.
** Then tests whether the taxonomy data matches taxonomy data query criteria
** and checks any specified type. Applies upper and lower case.
**
** @param [u] taxin [AjPTaxin] Taxonomy data input object
** @param [w] tax [AjPTax] Taxonomy data object
** @param [r] format [ajuint] input format code
** @return [ajuint] 0 if successful.
**                  1 if the query match failed.
**                  2 if the taxonomy data type failed
**                  3 if it failed to read any taxonomy data
**
** @release 6.4.0
** @@
** This is the only function that calls the appropriate Read function
** taxinReadXxxxxx where Xxxxxxx is the supported taxonomy data format.
**
** Some of the taxReadXxxxxx functions fail to reset the buffer correctly,
** which is a very serious problem when cycling through all of them to
** identify an unknown format. The extra ajFileBuffReset call at the end is
** intended to address this problem. The individual functions should still
** reset the buffer in case they are called from elsewhere.
**
******************************************************************************/

static ajuint taxinReadFmt(AjPTaxin taxin, AjPTax tax,
                           ajuint format)
{
    ajDebug("++taxinReadFmt format %d (%s) '%S'\n",
	    format, taxinFormatDef[format].Name,
	    taxin->Input->Qry);

    taxin->Input->Records = 0;

    /* Calling funclist taxinFormatDef() */
    if((*taxinFormatDef[format].Read)(taxin, tax))
    {
	ajDebug("taxinReadFmt success with format %d (%s)\n",
		format, taxinFormatDef[format].Name);
        ajDebug("id: '%S'\n",
                tax->Id);
	taxin->Input->Format = format;
	ajStrAssignC(&taxin->Input->Formatstr, taxinFormatDef[format].Name);
	ajStrAssignC(&tax->Formatstr, taxinFormatDef[format].Name);
	ajStrAssignEmptyS(&tax->Db, taxin->Input->Db);
	ajStrAssignS(&tax->Filename, taxin->Input->Filename);

	if(taxinQueryMatch(taxin->Input->Query, tax))
	{
            /* ajTaxinTrace(taxin); */

            return FMT_OK;
        }

	ajDebug("query match failed, continuing ...\n");
	ajTaxClear(tax);

	return FMT_NOMATCH;
    }
    else
    {
	ajDebug("Testing input buffer: IsBuff: %B Eof: %B\n",
		ajFilebuffIsBuffered(taxin->Input->Filebuff),
		ajFilebuffIsEof(taxin->Input->Filebuff));

	if(!ajFilebuffIsBuffered(taxin->Input->Filebuff) &&
	    ajFilebuffIsEof(taxin->Input->Filebuff))
	    return FMT_EOF;

	ajFilebuffReset(taxin->Input->Filebuff);
	ajDebug("Format %d (%s) failed, file buffer reset by taxinReadFmt\n",
		format, taxinFormatDef[format].Name);
	/* ajFilebuffTraceFull(taxin->Filebuff, 10, 10);*/
    }

    ajDebug("++taxinReadFmt failed - nothing read\n");

    return FMT_FAIL;
}




/* @funcstatic taxinRead ******************************************************
**
** Given data in a taxin structure, tries to read everything needed
** using the specified format or by trial and error.
**
** @param [u] taxin [AjPTaxin] Taxonomy data input object
** @param [w] tax [AjPTax] Taxonomy data object
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool taxinRead(AjPTaxin taxin, AjPTax tax)
{
    ajuint i;
    ajuint istat = 0;
    ajuint jstat = 0;

    AjPFilebuff buff = taxin->Input->Filebuff;
    AjBool ok;

    AjPTextAccess  textaccess  = taxin->Input->Query->TextAccess;
    AjPTaxAccess taxaccess = taxin->Input->Query->Access;

    ajTaxClear(tax);
    ajDebug("taxinRead: cleared\n");

    if(taxin->Input->Single && taxin->Input->Count)
    {
	/*
	** One taxonomy data item at a time is read.
	** The first taxonomy data item was read by ACD
	** for the following ones we need to reset the AjPTaxin
	**
	** Single is set by the access method
	*/

	ajDebug("taxinRead: single access - count %d - call access"
		" routine again\n",
		taxin->Input->Count);
	/* Calling funclist taxinAccess() */
	if(textaccess)
        {
            if(!(*textaccess->Access)(taxin->Input))
            {
                ajDebug("taxinRead: (*textaccess->Access)(taxin->Input) "
                        "*failed*\n");

                return ajFalse;
            }
        }

	if(taxaccess)
        {
            if(!(*taxaccess->Access)(taxin))
            {
                ajDebug("taxinRead: (*taxaccess->Access)(taxin) "
                        "*failed*\n");

                return ajFalse;
            }
        }

        buff = taxin->Input->Filebuff;
    }

    ajDebug("taxinRead: taxin format %d '%S'\n", taxin->Input->Format,
	    taxin->Input->Formatstr);

    taxin->Input->Count++;

    if(!taxin->Input->Filebuff)
	return ajFalse;

    ok = ajFilebuffIsBuffered(taxin->Input->Filebuff);

    while(ok)
    {				/* skip blank lines */
        ok = ajBuffreadLine(taxin->Input->Filebuff, &taxinReadLine);

        if(!ajStrIsWhite(taxinReadLine))
        {
            ajFilebuffClear(taxin->Input->Filebuff,1);
            break;
        }
    }

    if(!taxin->Input->Format)
    {			   /* no format specified, try all defaults */
	for(i = 1; taxinFormatDef[i].Name; i++)
	{
	    if(!taxinFormatDef[i].Try)	/* skip if Try is ajFalse */
		continue;

	    ajDebug("taxinRead:try format %d (%s)\n",
		    i, taxinFormatDef[i].Name);

	    istat = taxinReadFmt(taxin, tax, i);

	    switch(istat)
	    {
	    case FMT_OK:
		ajDebug("++taxinRead OK, set format %d\n",
                        taxin->Input->Format);
		taxDefine(tax, taxin);

		return ajTrue;
	    case FMT_BADTYPE:
		ajDebug("taxinRead: (a1) "
                        "taxinReadFmt stat == BADTYPE *failed*\n");

		return ajFalse;
	    case FMT_FAIL:
		ajDebug("taxinRead: (b1) "
                        "taxinReadFmt stat == FAIL *failed*\n");
		break;			/* we can try next format */
	    case FMT_NOMATCH:
		ajDebug("taxinRead: (c1) "
                        "taxinReadFmt stat==NOMATCH try again\n");
		break;
	    case FMT_EOF:
		ajDebug("taxinRead: (d1) "
                        "taxinReadFmt stat == EOF *failed*\n");
		return ajFalse;			/* EOF and unbuffered */
	    case FMT_EMPTY:
		ajWarn("taxonomy data '%S' has zero length, ignored",
		       ajTaxGetQryS(tax));
		ajDebug("taxinRead: (e1) "
                        "taxinReadFmt stat==EMPTY try again\n");
		break;
	    default:
		ajDebug("unknown code %d from taxinReadFmt\n", stat);
	    }

	    ajTaxClear(tax);

	    if(taxin->Input->Format)
		break;			/* we read something */

            ajFilebuffTrace(taxin->Input->Filebuff);
	}

	if(!taxin->Input->Format)
	{		     /* all default formats failed, give up */
	    ajDebug("taxinRead:all default formats failed, give up\n");

	    return ajFalse;
	}

	ajDebug("++taxinRead set format %d\n",
                taxin->Input->Format);
    }
    else
    {					/* one format specified */
	ajDebug("taxinRead: one format specified\n");
	ajFilebuffSetUnbuffered(taxin->Input->Filebuff);

	ajDebug("++taxinRead known format %d\n",
                taxin->Input->Format);
	istat = taxinReadFmt(taxin, tax, taxin->Input->Format);

	switch(istat)
	{
	case FMT_OK:
	    taxDefine(tax, taxin);

	    return ajTrue;
	case FMT_BADTYPE:
	    ajDebug("taxinRead: (a2) "
                    "taxinReadFmt stat == BADTYPE *failed*\n");

	    return ajFalse;

        case FMT_FAIL:
	    ajDebug("taxinRead: (b2) "
                    "taxinReadFmt stat == FAIL *failed*\n");

	    return ajFalse;

        case FMT_NOMATCH:
	    ajDebug("taxinRead: (c2) "
                    "taxinReadFmt stat == NOMATCH *try again*\n");
	    break;
	case FMT_EOF:
	    ajDebug("taxinRead: (d2) "
                    "taxinReadFmt stat == EOF *try again*\n");
            if(taxin->Input->Records)
                ajErr("Error reading file '%F' with format '%s': "
                      "end-of-file before end of data "
                      "(read %u records)",
                      ajFilebuffGetFile(taxin->Input->Filebuff),
                      taxinFormatDef[taxin->Input->Format].Name,
                      taxin->Input->Records);
	    break;		     /* simply end-of-file */
	case FMT_EMPTY:
	    ajWarn("assmebly data '%S' has zero length, ignored",
		   ajTaxGetQryS(tax));
	    ajDebug("taxinRead: (e2) "
                    "taxinReadFmt stat == EMPTY *try again*\n");
	    break;
	default:
	    ajDebug("unknown code %d from taxinReadFmt\n", stat);
	}

	ajTaxClear(tax); /* 1 : read, failed to match id/acc/query */
    }

    /* failed - probably entry/accession query failed. Can we try again? */

    ajDebug("taxinRead failed - try again with format %d '%s' code %d\n",
	    taxin->Input->Format,
            taxinFormatDef[taxin->Input->Format].Name, istat);

    ajDebug("Search:%B Chunk:%B Data:%x ajFileBuffEmpty:%B\n",
	    taxin->Input->Search, taxin->Input->ChunkEntries,
            taxin->Input->TextData, ajFilebuffIsEmpty(buff));

    if(ajFilebuffIsEmpty(buff) && taxin->Input->ChunkEntries)
    {
	if(textaccess && !(*textaccess->Access)(taxin->Input))
            return ajFalse;
	else if(taxaccess && !(*taxaccess->Access)(taxin))
            return ajFalse;
        buff = taxin->Input->Filebuff;
    }


    /* need to check end-of-file to avoid repeats */
    while(taxin->Input->Search &&
          (taxin->Input->TextData || !ajFilebuffIsEmpty(buff)))
    {
	jstat = taxinReadFmt(taxin, tax, taxin->Input->Format);

	switch(jstat)
	{
	case FMT_OK:
	    taxDefine(tax, taxin);

	    return ajTrue;

        case FMT_BADTYPE:
	    ajDebug("taxinRead: (a3) "
                    "taxinReadFmt stat == BADTYPE *failed*\n");

	    return ajFalse;

        case FMT_FAIL:
	    ajDebug("taxinRead: (b3) "
                    "taxinReadFmt stat == FAIL *failed*\n");

	    return ajFalse;
            
	case FMT_NOMATCH:
	    ajDebug("taxinRead: (c3) "
                    "taxinReadFmt stat == NOMATCH *try again*\n");
	    break;
	case FMT_EOF:
	    ajDebug("taxinRead: (d3) "
                    "taxinReadFmt stat == EOF *failed*\n");

	    return ajFalse;			/* we already tried again */

        case FMT_EMPTY:
	    if(istat != FMT_EMPTY)
                ajWarn("assmebly data '%S' has zero length, ignored",
                       ajTaxGetQryS(tax));
	    ajDebug("taxinRead: (e3) "
                    "taxinReadFmt stat == EMPTY *try again*\n");
	    break;

        default:
	    ajDebug("unknown code %d from taxinReadFmt\n", stat);
	}

	ajTaxClear(tax); /* 1 : read, failed to match id/acc/query */
    }

    if(taxin->Input->Format)
	ajDebug("taxinRead: *failed* to read taxonomy data %S "
                "using format %s\n",
		taxin->Input->Qry,
                taxinFormatDef[taxin->Input->Format].Name);
    else
	ajDebug("taxinRead: *failed* to read taxonomy data %S "
                "using any format\n",
		taxin->Input->Qry);

    return ajFalse;
}




/* @funcstatic taxinReadEbi ***************************************************
**
** Given data in a taxonomy structure, tries to read everything needed
** using EBI format.
**
** @param [u] taxin [AjPTaxin] Tax input object
** @param [w] tax [AjPTax] tax object
** @return [AjBool] ajTrue on success
**
** @release 6.6.0
** @@
******************************************************************************/

static AjBool taxinReadEbi(AjPTaxin taxin, AjPTax tax)
{
    AjPFilebuff buff;

    ajlong fpos     = 0;
    ajuint linecnt = 0;

    AjPStrTok handle = NULL;
    AjPStr tmpstr = NULL;

    ajDebug("taxinReadEbi\n");
    ajTaxClear(tax);
    buff = taxin->Input->Filebuff;

    /* ajFilebuffTrace(buff); */

    while(ajBuffreadLinePos(buff, &taxinReadLine, &fpos))
    {
        linecnt++;
    }

    ajStrTokenDel(&handle);
    ajStrDel(&tmpstr);

    return ajTrue;
}




/* @funcstatic taxinReadNcbi **************************************************
**
** Given data in a taxonomy structure, tries to read everything needed
** using NCBI format.
**
** @param [u] taxin [AjPTaxin] Tax input object
** @param [w] tax [AjPTax] tax object
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool taxinReadNcbi(AjPTaxin taxin, AjPTax tax)
{
    AjPFilebuff buff;

    ajlong fpos     = 0;
    ajuint linecnt = 0;
    ajuint icode;

    AjPStrTok handle = NULL;
    AjPStr tmpstr = NULL;

    AjPTaxname name = NULL;

    ajDebug("taxinReadNcbi\n");
    ajTaxClear(tax);
    buff = taxin->Input->Filebuff;

    /* ajFilebuffTrace(buff); */

    while(ajBuffreadLinePos(buff, &taxinReadLine, &fpos))
    {
        ajStrTokenAssignC(&handle, taxinReadLine, "|");

        linecnt++;
        if(linecnt == 1)
        {
            ajStrTokenNextParse(handle, &tax->Id);         
            ajStrTrimWhite(&tax->Id);
            ajStrToUint(tax->Id, &tax->Taxid);

            ajStrTokenNextParse(handle, &tmpstr);         
            ajStrTrimWhite(&tmpstr);
            ajStrToUint(tmpstr, &tax->Parent);

            ajStrTokenNextParse(handle, &tax->Rank);         
            ajStrTrimWhite(&tax->Rank);

            ajStrTokenNextParse(handle, &tax->Emblcode);         
            ajStrTrimWhite(&tax->Emblcode);

            ajStrTokenNextParse(handle, &tmpstr);         
            ajStrTrimWhite(&tmpstr);
            ajStrToUint(tmpstr, &icode);
            tax->Divid = icode;

            ajStrTokenNextParse(handle, &tmpstr);         
            ajStrTrimWhite(&tmpstr);
            if(ajStrMatchC(tmpstr, "1"))
                tax->Flags |= 1;

            ajStrTokenNextParse(handle, &tmpstr);         
            ajStrTrimWhite(&tmpstr);
            ajStrToUint(tmpstr, &icode);
            tax->Gencode = icode;

            ajStrTokenNextParse(handle, &tmpstr);         
            ajStrTrimWhite(&tmpstr);
            if(ajStrMatchC(tmpstr, "1"))
                tax->Flags |= 2;

            ajStrTokenNextParse(handle, &tmpstr);         
            ajStrTrimWhite(&tmpstr);
            ajStrToUint(tmpstr, &icode);
            tax->Mitocode = icode;

            ajStrTokenNextParse(handle, &tmpstr);         
            ajStrTrimWhite(&tmpstr);
            if(ajStrMatchC(tmpstr, "1"))
                tax->Flags |= 4;

            ajStrTokenNextParse(handle, &tmpstr);         
            ajStrTrimWhite(&tmpstr);
            if(ajStrMatchC(tmpstr, "1"))
                tax->Flags |= 8;

            ajStrTokenNextParse(handle, &tmpstr);         
            ajStrTrimWhite(&tmpstr);
            if(ajStrMatchC(tmpstr, "1"))
                tax->Flags |= 16;

            ajStrTokenNextParse(handle, &tax->Comment);         
            ajStrTrimWhite(&tax->Comment);
        }
        else
        {
            ajStrTokenNextParse(handle, &tmpstr);         
            ajStrTrimWhite(&tmpstr);
            if(!ajStrMatchS(tax->Id, tmpstr))
            {
                ajDebug("names.dmp id '%S' != nodes.dmp id '%S'\n",
                       tax->Id, tmpstr);
                break;
            }

            name = ajTaxnameNew();

            ajStrTokenNextParse(handle, &name->Name);         
            ajStrTrimWhite(&name->Name);

            ajStrTokenNextParse(handle, &name->UniqueName);         
            ajStrTrimWhite(&name->UniqueName);

            ajStrTokenNextParse(handle, &name->NameClass);         
            ajStrTrimWhite(&name->NameClass);

            if(ajStrMatchC(name->NameClass, "scientific name"))
                ajStrAssignEmptyS(&tax->Name, name->Name);

            ajListPushAppend(tax->Namelist, name);
        }

        ajDebug("line %u:%S\n", linecnt, taxinReadLine);

        /* add line to AjPTax object */
    }

    ajStrTokenDel(&handle);
    ajStrDel(&tmpstr);

    return ajTrue;
}




/* @datasection [none] Miscellaneous ******************************************
**
** Taxonomy input internals
**
** @nam2rule Taxin Taxonomy input
**
******************************************************************************/




/* @section Printing **********************************************************
**
** Printing details of the internals to a file
**
** @fdata [none]
**
** @nam2rule Taxinprint
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




/* @func ajTaxinprintBook *****************************************************
**
** Reports the internal data structures as a Docbook table
**
** @param [u] outf [AjPFile] Output file
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTaxinprintBook(AjPFile outf)
{
    ajuint i = 0;
    ajuint j = 0;
    AjPStr namestr = NULL;
    AjPList fmtlist;
    AjPStr* names;

    fmtlist = ajListstrNew();

    ajFmtPrintF(outf, "<para>The supported taxonomy formats are summarised "
                "in the table below. "
                "The columns are as follows: "
                "<emphasis>Input format</emphasis> (format name), "
                "<emphasis>Try</emphasis> (indicates whether the "
                "format can be detected automatically on input), and "
                "<emphasis>Description</emphasis> (short description of "
                "the format).</para>\n\n");

    ajFmtPrintF(outf, "<table frame=\"box\" rules=\"cols\">\n");
    ajFmtPrintF(outf, "  <caption>Input taxonomy formats</caption>\n");
    ajFmtPrintF(outf, "  <thead>\n");
    ajFmtPrintF(outf, "    <tr align=\"center\">\n");
    ajFmtPrintF(outf, "      <th>Input Format</th>\n");
    ajFmtPrintF(outf, "      <th>Try</th>\n");
    ajFmtPrintF(outf, "      <th>Description</th>\n");
    ajFmtPrintF(outf, "    </tr>\n");
    ajFmtPrintF(outf, "  </thead>\n");
    ajFmtPrintF(outf, "  <tbody>\n");

    for(i=1; taxinFormatDef[i].Name; i++)
    {
	if(!taxinFormatDef[i].Alias)
        {
            namestr = ajStrNewC(taxinFormatDef[i].Name);
            ajListPushAppend(fmtlist, namestr);
            namestr = NULL;
        }
    }

    ajListSort(fmtlist, &ajStrVcmp);
    ajListstrToarray(fmtlist, &names);

    for(i=0; names[i]; i++)
    {
        for(j=0; taxinFormatDef[j].Name; j++)
        {
            if(ajStrMatchC(names[i],taxinFormatDef[j].Name))
            {
                ajFmtPrintF(outf, "    <tr>\n");
                ajFmtPrintF(outf, "      <td>%s</td>\n",
                            taxinFormatDef[j].Name);
                ajFmtPrintF(outf, "      <td>%B</td>\n",
                            taxinFormatDef[j].Try);
                ajFmtPrintF(outf, "      <td>%s</td>\n",
                            taxinFormatDef[j].Desc);
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




/* @func ajTaxinprintHtml *****************************************************
**
** Reports the internal data structures as an HTML table
**
** @param [u] outf [AjPFile] Output file
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTaxinprintHtml(AjPFile outf)
{
    ajuint i = 0;
    ajuint j = 0;

    AjPStr namestr = NULL;

    ajFmtPrintF(outf, "<table border=3>");
    ajFmtPrintF(outf, "<tr><th>Input Format</th><th>Auto</th>\n");
    ajFmtPrintF(outf, "<th>Multi</th><th>Description</th></tr>\n");

    for(i=1; taxinFormatDef[i].Name; i++)
    {
        ajStrAssignC(&namestr, taxinFormatDef[i].Name);

	if(!taxinFormatDef[i].Alias)
        {
            for(j=i+1; taxinFormatDef[j].Name; j++)
            {
                if(taxinFormatDef[j].Read == taxinFormatDef[i].Read)
                {
                    ajFmtPrintAppS(&namestr, " %s", taxinFormatDef[j].Name);
                    if(!taxinFormatDef[j].Alias) 
                    {
                        ajWarn("Input format '%s' same as '%s' but not alias",
                               taxinFormatDef[j].Name,
                               taxinFormatDef[i].Name);
                    }
                }
            }

	    ajFmtPrintF(outf, "<tr><td>\n%S\n</td><td>%B</td>\n",
                        namestr,
			taxinFormatDef[i].Try);
            ajFmtPrintF(outf, "<td>\n%s\n</td></tr>\n",
			taxinFormatDef[i].Desc);
        }

    }

    ajFmtPrintF(outf, "</table>\n");
    ajStrDel(&namestr);

    return;
}




/* @func ajTaxinprintText *****************************************************
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

void ajTaxinprintText(AjPFile outf, AjBool full)
{
    ajuint i = 0;

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# Taxonomy input formats\n");
    ajFmtPrintF(outf, "# Name  Format name (or alias)\n");
    ajFmtPrintF(outf, "# Alias Alias name\n");
    ajFmtPrintF(outf, "# Try   Test for unknown input files\n");
    ajFmtPrintF(outf, "# Name         Alias Try "
		"Description");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "InFormat {\n");

    for(i=0; taxinFormatDef[i].Name; i++)
	if(full || !taxinFormatDef[i].Alias)
	    ajFmtPrintF(outf,
			"  %-12s %5B %3B \"%s\"\n",
			taxinFormatDef[i].Name,
			taxinFormatDef[i].Alias,
			taxinFormatDef[i].Try,
			taxinFormatDef[i].Desc);

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajTaxinprintWiki *****************************************************
**
** Reports the internal data structures as a wiki table
**
** @param [u] outf [AjPFile] Output file
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTaxinprintWiki(AjPFile outf)
{
    ajuint i = 0;
    ajuint j = 0;

    AjPStr namestr = NULL;

    ajFmtPrintF(outf, "{| class=\"wikitable sortable\" border=\"2\"\n");
    ajFmtPrintF(outf, "|-\n");
    ajFmtPrintF(outf, "!Format!!Try!!"
                "class=\"unsortable\"|Description\n");

    for(i=1; taxinFormatDef[i].Name; i++)
    {
        ajStrAssignC(&namestr, taxinFormatDef[i].Name);

	if(!taxinFormatDef[i].Alias)
        {
            for(j=i+1; taxinFormatDef[j].Name; j++)
            {
                if(taxinFormatDef[j].Read == taxinFormatDef[i].Read)
                {
                    ajFmtPrintAppS(&namestr, "<br>%s",
                                   taxinFormatDef[j].Name);
                    if(!taxinFormatDef[j].Alias) 
                    {
                        ajWarn("Input format '%s' same as '%s' but not alias",
                               taxinFormatDef[j].Name,
                               taxinFormatDef[i].Name);
                    }
                }
            }

            ajFmtPrintF(outf, "|-\n");
	    ajFmtPrintF(outf,
			"|%S||%B||%s\n",
			namestr,
			taxinFormatDef[i].Try,
			taxinFormatDef[i].Desc);
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




/* @func ajTaxinExit **********************************************************
**
** Cleans up taxonomy input internal memory
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTaxinExit(void)
{
    /* Query processing regular expressions */

    ajStrDel(&taxinReadLine);

    ajTableDel(&taxDbMethods);

    return;
}




/* @section Internals *********************************************************
**
** Functions to return internal values
**
** @fdata [none]
**
** @nam3rule Type Internals for taxon datatype
** @nam4rule Get  Return a value
** @nam5rule Fields  Known query fields for ajTaxinRead
** @nam5rule Qlinks  Known query link operators for ajTaxinRead
**
** @valrule * [const char*] Internal value
**
** @fcategory misc
**
******************************************************************************/




/* @func ajTaxinTypeGetFields *************************************************
**
** Returns the list of known field names for ajTaxinRead
**
** @return [const char*] List of field names
**
** @release 6.4.0
** @@
******************************************************************************/

const char* ajTaxinTypeGetFields(void)
{
    return "id acc";
}




/* @func ajTaxinTypeGetQlinks *************************************************
**
** Returns the listof known query link operators for ajTaxinRead
**
** @return [const char*] List of field names
**
** @release 6.4.0
** @@
******************************************************************************/

const char* ajTaxinTypeGetQlinks(void)
{
    return "|";
}




/* @datasection [AjPTable] Internal call register table ***********************
**
** Functions to manage the internal call register table that links the
** ajaxdb library functions with code in the core AJAX library.
**
** @nam2rule Taxaccess Functions to manage taxdb call tables.
**
******************************************************************************/




/* @section Cast **************************************************************
**
** Return a reference to the call table
**
** @fdata [AjPTable] taxdb functions call table
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




/* @func ajTaxaccessGetDb *****************************************************
**
** Returns the table in which taxonomy database access details are registered
**
** @return [AjPTable] Access functions hash table
**
** @release 6.4.0
** @@
******************************************************************************/

AjPTable ajTaxaccessGetDb(void)
{
    if(!taxDbMethods)
        taxDbMethods = ajCallTableNew();
    return taxDbMethods;
    
}




/* @func ajTaxaccessMethodGetQlinks *******************************************
**
** Tests for a named method for taxonomy data reading returns the 
** known query link operators
**
** @param [r] method [const AjPStr] Method required.
** @return [const char*] Known link operators
**
** @release 6.4.0
** @@
******************************************************************************/

const char* ajTaxaccessMethodGetQlinks(const AjPStr method)
{
    AjPTaxAccess methoddata; 

    methoddata = ajCallTableGetS(taxDbMethods, method);
    if(!methoddata)
        return NULL;

    return methoddata->Qlink;
}




/* @func ajTaxaccessMethodGetScope ********************************************
**
** Tests for a named method for taxonomy data reading and returns the scope
** (entry, query or all).
*
** @param [r] method [const AjPStr] Method required.
** @return [ajuint] Scope flags
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ajTaxaccessMethodGetScope(const AjPStr method)
{
    AjPTaxAccess methoddata; 
    ajuint ret = 0;

    methoddata = ajCallTableGetS(taxDbMethods, method);
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




/* @func ajTaxaccessMethodTest ************************************************
**
** Tests for a named method for taxonomy data reading.
**
** @param [r] method [const AjPStr] Method required.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajTaxaccessMethodTest(const AjPStr method)
{
    if(ajCallTableGetS(taxDbMethods, method))
      return ajTrue;

    return ajFalse;
}




/* @funcstatic taxinQryRestore ************************************************
**
** Restores a taxonomy input specification from an AjPQueryList node
**
** @param [w] taxin [AjPTaxin] Taxonomy input object
** @param [r] node [const AjPQueryList] Query list node
** @return [void]
**
** @release 6.4.0
******************************************************************************/

static void taxinQryRestore(AjPTaxin taxin, const AjPQueryList node)
{
    taxin->Input->Format = node->Format;
    taxin->Input->Fpos   = node->Fpos;
    ajStrAssignS(&taxin->Input->Formatstr, node->Formatstr);
    ajStrAssignS(&taxin->Input->QryFields, node->QryFields);

    return;
}




/* @funcstatic taxinQrySave ***************************************************
**
** Saves a taxonomy input specification in an AjPQueryList node
**
** @param [w] node [AjPQueryList] Query list node
** @param [r] taxin [const AjPTaxin] Taxonomy input object
** @return [void]
**
** @release 6.4.0
******************************************************************************/

static void taxinQrySave(AjPQueryList node, const AjPTaxin taxin)
{
    node->Format   = taxin->Input->Format;
    node->Fpos     = taxin->Input->Fpos;
    ajStrAssignS(&node->Formatstr, taxin->Input->Formatstr);
    ajStrAssignS(&node->QryFields, taxin->Input->QryFields);

    return;
}




/* @funcstatic taxinQryProcess ************************************************
**
** Converts a taxonomy data query into an open file.
**
** Tests for "format::" and sets this if it is found
**
** Then tests for "list:" or "@" and processes as a list file
** using taxinListProcess which in turn invokes taxinQryProcess
** until a valid query is found.
**
** Then tests for dbname:query and opens the file (at the correct position
** if the database definition defines it)
**
** If there is no database, looks for file:query and opens the file.
** In this case the file position is not known and taxonomy data reading
** will have to scan for the entry/entries we need.
**
** @param [u] taxin [AjPTaxin] Taxonomy data input structure.
** @param [u] tax [AjPTax] Taxonomy data to be read.
**                         The format will be replaced
**                         if defined in the query string.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool taxinQryProcess(AjPTaxin taxin, AjPTax tax)
{
    AjBool ret = ajTrue;
    AjPStr qrystr = NULL;
    AjBool taxmethod = ajFalse;
    const AjPStr fmtstr = NULL;
    AjPTextin textin;
    AjPQuery qry;
    AjPTaxAccess taxaccess = NULL;

    textin = taxin->Input;
    qry = textin->Query;

    /* pick up the original query string */
    qrystr = ajStrNewS(textin->Qry);

    ajDebug("taxinQryProcess '%S'\n", qrystr);

    /* look for a format:: prefix */
    fmtstr = ajQuerystrParseFormat(&qrystr, textin, taxinformatFind);
    ajDebug("taxinQryProcess ... fmtstr '%S' '%S'\n", fmtstr, qrystr);

    /* (seq/feat) DO NOT look for a [range] suffix */

    /* look for a list:: or @:: listfile of queries  - process and return */
    if(ajQuerystrParseListfile(&qrystr))
    {
        ajDebug("taxinQryProcess ... listfile '%S'\n", qrystr);
        ret = taxinListProcess(taxin, tax, qrystr);
        ajStrDel(&qrystr);
        return ret;
    }

    /* try general text access methods (file, asis, text database access */
    ajDebug("taxinQryProcess ... no listfile '%S'\n", qrystr);
    if(!ajQuerystrParseRead(&qrystr, textin, taxinformatFind, &taxmethod))
    {
        ajStrDel(&qrystr);
        return ajFalse;
    }
    
    taxinFormatSet(taxin, tax);

    ajDebug("taxinQryProcess ... read nontext: %B '%S'\n",
            taxmethod, qrystr);
    ajStrDel(&qrystr);

    /* we found a non-text method */
    if(taxmethod)
    {
        ajDebug("taxinQryProcess ... call method '%S'\n", qry->Method);
        ajDebug("taxinQryProcess ... textin format %d '%S'\n",
                textin->Format, textin->Formatstr);
        ajDebug("taxinQryProcess ...  query format  '%S'\n",
                qry->Formatstr);
        qry->Access = ajCallTableGetS(taxDbMethods,qry->Method);
        taxaccess = qry->Access;
        return (*taxaccess->Access)(taxin);
    }

    ajDebug("taxinQryProcess text method '%S' success\n", qry->Method);

    return ajTrue;
}





/* @datasection [AjPList] Query field list ************************************
**
** Query fields lists are handled internally. Only static functions
** should appear here
**
******************************************************************************/




/* @funcstatic taxinListProcess ***********************************************
**
** Processes a file of queries.
** This function is called by, and calls, taxinQryProcess. There is
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
** @param [u] taxin [AjPTaxin] Taxonomy data input
** @param [u] tax [AjPTax] Taxonomy data
** @param [r] listfile [const AjPStr] Name of list file.,
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool taxinListProcess(AjPTaxin taxin, AjPTax tax,
                               const AjPStr listfile)
{
    AjPList list  = NULL;
    AjPFile file  = NULL;
    AjPStr token  = NULL;
    AjPStr rest  = NULL;
    AjBool ret       = ajFalse;
    AjPQueryList node = NULL;

    ajuint recnum = 0;
    static ajint depth    = 0;
    static ajint MAXDEPTH = 16;

    depth++;
    ajDebug("++taxinListProcess %S depth %d\n",
	    listfile, depth);

    if(depth > MAXDEPTH)
	ajFatal("Query list too deep");

    if(!taxin->Input->List)
	taxin->Input->List = ajListNew();

    list = ajListNew();

    file = ajFileNewInNameS(listfile);

    if(!file)
    {
	ajErr("Failed to open list file '%S'", listfile);
	depth--;

	return ret;
    }

    while(ajReadlineTrim(file, &taxinReadLine))
    {
        ++recnum;
	taxinListNoComment(&taxinReadLine);

        if(ajStrExtractWord(taxinReadLine, &rest, &token))
        {
            
            if(ajStrGetLen(rest)) 
            {
                ajErr("Bad record %u in list file '%S'\n'%S'",
                      recnum, listfile, taxinReadLine);
            }
            else if(ajStrGetLen(token))
            {
                ajDebug("++Add to list: '%S'\n", token);
                AJNEW0(node);
                ajStrAssignS(&node->Qry, token);
                taxinQrySave(node, taxin);
                ajListPushAppend(list, node);
            }
        }
    }

    ajFileClose(&file);
    ajStrDel(&token);
    ajStrDel(&rest);

    ajDebug("Trace taxin->Input->List\n");
    ajQuerylistTrace(taxin->Input->List);
    ajDebug("Trace new list\n");
    ajQuerylistTrace(list);
    ajListPushlist(taxin->Input->List, &list);

    ajDebug("Trace combined taxin->Input->List\n");
    ajQuerylistTrace(taxin->Input->List);

    /*
     ** now try the first item on the list
     ** this can descend recursively if it is also a list
     ** which is why we check the depth above
     */

    if(ajListPop(taxin->Input->List, (void**) &node))
    {
        ajDebug("++pop first item '%S'\n", node->Qry);
	ajTaxinQryS(taxin, node->Qry);
	taxinQryRestore(taxin, node);
	ajStrDel(&node->Qry);
	ajStrDel(&node->Formatstr);
	AJFREE(node);
	ajDebug("descending with query '%S'\n", taxin->Input->Qry);
	ret = taxinQryProcess(taxin, tax);
    }

    depth--;
    ajDebug("++taxinListProcess depth: %d returns: %B\n", depth, ret);

    return ret;
}




/* @funcstatic taxinListNoComment *********************************************
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

static void taxinListNoComment(AjPStr* text)
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




/* @funcstatic taxinFormatSet *************************************************
**
** Sets the input format for taxonomy data using the taxonomy data
** input object's defined format
**
** @param [u] taxin [AjPTaxin] Taxonomy data input.
** @param [u] tax [AjPTax] Taxonomy data
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool taxinFormatSet(AjPTaxin taxin, AjPTax tax)
{

    if(ajStrGetLen(taxin->Input->Formatstr))
    {
	ajDebug("... input format value '%S'\n",
                taxin->Input->Formatstr);

	if(taxinformatFind(taxin->Input->Formatstr,
                             &taxin->Input->Format))
	{
	    ajStrAssignS(&tax->Formatstr,
                         taxin->Input->Formatstr);
	    tax->Format = taxin->Input->Format;
	    ajDebug("...format OK '%S' = %d\n",
                    taxin->Input->Formatstr,
		    taxin->Input->Format);
	}
	else
	    ajDebug("...format unknown '%S'\n",
                    taxin->Input->Formatstr);

	return ajTrue;
    }
    else
	ajDebug("...input format not set\n");


    return ajFalse;
}




/* @datasection [AjPTaxall] Taxon Input Stream ********************************
**
** Function is for manipulating taxon input stream objects
**
** @nam2rule Taxall Taxon input stream objects
**
******************************************************************************/




/* @section Taxon Input Constructors ******************************************
**
** All constructors return a new taxon input stream object by pointer. It
** is the responsibility of the user to first destroy any previous
** taxon input object. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so
** anyway.
**
** @fdata [AjPTaxall]
**
** @nam3rule New Constructor
**
** @valrule * [AjPTaxall] Taxon input stream object
**
** @fcategory new
**
******************************************************************************/




/* @func ajTaxallNew **********************************************************
**
** Creates a new taxon input stream object.
**
** @return [AjPTaxall] New taxon input stream object.
**
** @release 6.4.0
** @@
******************************************************************************/

AjPTaxall ajTaxallNew(void)
{
    AjPTaxall pthis;

    AJNEW0(pthis);

    pthis->Taxin = ajTaxinNew();
    pthis->Tax   = ajTaxNew();

    return pthis;
}





/* ==================================================================== */
/* ========================== destructors ============================= */
/* ==================================================================== */




/* @section Taxon Input Stream Destructors ************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the taxon input stream object.
**
** @fdata [AjPTaxall]
**
** @nam3rule Del Destructor
**
** @argrule Del pthis [AjPTaxall*] Taxon input stream
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajTaxallDel **********************************************************
**
** Deletes a taxon input stream object.
**
** @param [d] pthis [AjPTaxall*] taxon input stream
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTaxallDel(AjPTaxall* pthis)
{
    AjPTaxall thys;

    if(!pthis)
        return;

    thys = *pthis;

    if(!thys)
        return;

    ajTaxinDel(&thys->Taxin);
    if(!thys->Returned)
        ajTaxDel(&thys->Tax);

    AJFREE(*pthis);

    return;
}




/* ==================================================================== */
/* =========================== Modifiers ============================== */
/* ==================================================================== */




/* @section taxon input stream modifiers **************************************
**
** These functions use the contents of a taxon input stream object and
** update them.
**
** @fdata [AjPTaxall]
**
** @nam3rule Clear Clear all values
**
** @argrule * thys [AjPTaxall] Taxon input stream object
**
** @valrule * [void]
**
** @fcategory modify
**
******************************************************************************/




/* @func ajTaxallClear ********************************************************
**
** Clears a taxon input stream object back to "as new" condition, except
** for the query list which must be preserved.
**
** @param [w] thys [AjPTaxall] Taxon input stream
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTaxallClear(AjPTaxall thys)
{

    ajDebug("ajTaxallClear called\n");

    ajTaxinClear(thys->Taxin);

    ajTaxClear(thys->Tax);

    thys->Returned = ajFalse;

    return;
}




/* @section taxon input stream casts ******************************************
**
** These functions return the contents of a taxon input stream object
**
** @fdata [AjPTaxall]
**
** @nam3rule Get Get taxon input stream values
** @nam3rule Gettax Get taxon values
** @nam4rule GettaxId Get taxon identifier
**
** @argrule * thys [const AjPTaxall] Taxon input stream object
**
** @valrule * [const AjPStr] String value
**
** @fcategory cast
**
******************************************************************************/




/* @func ajTaxallGettaxId *****************************************************
**
** Returns the identifier of the current taxon in an input stream
**
** @param [r] thys [const AjPTaxall] Taxon input stream
** @return [const AjPStr] Identifier
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPStr ajTaxallGettaxId(const AjPTaxall thys)
{
    if(!thys)
        return NULL;

    ajDebug("ajTaxallGettaxId called\n");

    return ajTaxGetId(thys->Tax);
}




/* @section taxon input *******************************************************
**
** These functions use a taxon input stream object to read data
**
** @fdata [AjPTaxall]
**
** @nam3rule Next Read next taxon
**
** @argrule * thys [AjPTaxall] Taxon input stream object
** @argrule * Ptax [AjPTax*] Taxon object
**
** @valrule * [AjBool] True on success
**
** @fcategory input
**
******************************************************************************/




/* @func ajTaxallNext *********************************************************
**
** Parse a taxon query into format, access, file and entry
**
** Split at delimiters. Check for the first part as a valid format
** Check for the remaining first part as a database name or as a file
** that can be opened.
** Anything left is an entryname spec.
**
** Return the results in the AjPTax object but leave the file open for
** future calls.
**
** @param [w] thys [AjPTaxall] Taxon input stream
** @param [u] Ptax [AjPTax*] Taxon returned
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajTaxallNext(AjPTaxall thys, AjPTax *Ptax)
{
    ajDebug("ajTaxallNext count:%u\n", thys->Count);

    if(!thys->Count)
    {
	thys->Count = 1;

	thys->Totterms++;

	*Ptax = thys->Tax;
	thys->Returned = ajTrue;

	return ajTrue;
    }


    if(ajTaxinRead(thys->Taxin, thys->Tax))
    {
	thys->Count++;

	thys->Totterms++;

	*Ptax = thys->Tax;
	thys->Returned = ajTrue;

	ajDebug("ajTaxallNext success\n");

	return ajTrue;
    }

    *Ptax = NULL;

    ajDebug("ajTaxallNext failed\n");

    ajTaxallClear(thys);

    return ajFalse;
}




/* @datasection [none] Input formats ******************************************
**
** Input formats internals
**
** @nam2rule Taxinformat Taxonomy data input format specific
**
******************************************************************************/




/* @section cast **************************************************************
**
** Values for input formats
**
** @fdata [none]
**
** @nam3rule Find Return index to named format
** @nam3rule Test Test format value
**
** @argrule * format [const AjPStr] Format name
** @argrule Find iformat [ajint*] Index matching format name
**
** @valrule * [AjBool] True if found
**
** @fcategory cast
**
******************************************************************************/




/* @funcstatic taxinformatFind ************************************************
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

static AjBool taxinformatFind(const AjPStr format, ajint* iformat)
{
    AjPStr tmpformat = NULL;
    ajuint i = 0;

    /* ajDebug("taxinformatFind '%S'\n", format); */
    if(!ajStrGetLen(format))
	return ajFalse;

    ajStrAssignS(&tmpformat, format);
    ajStrFmtLower(&tmpformat);

    for(i=0; taxinFormatDef[i].Name; i++)
    {
	/* ajDebug("test %d '%s'\n",
           i, taxinFormatDef[i].Name); */
	if(ajStrMatchC(tmpformat, taxinFormatDef[i].Name))
	{
	    *iformat = i;
	    ajStrDel(&tmpformat);
	    /* ajDebug("found '%s' at %d\n", taxinFormatDef[i].Name, i); */
	    return ajTrue;
	}
    }

    ajErr("Unknown input format '%S'", format);

    ajStrDel(&tmpformat);

    return ajFalse;
}




/* @func ajTaxinformatTest ****************************************************
**
** Tests whether a named taxonomy data input format is known
**
** @param [r] format [const AjPStr] Format
** @return [AjBool] ajTrue if formats was accepted
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajTaxinformatTest(const AjPStr format)
{
    ajuint i;

    for(i=0; taxinFormatDef[i].Name; i++)
	if(ajStrMatchCaseC(format, taxinFormatDef[i].Name))
	    return ajTrue;

    return ajFalse;
}

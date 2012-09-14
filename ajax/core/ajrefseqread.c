/* @source ajrefseqread *******************************************************
**
** AJAX reference sequence reading functions
**
** These functions control all aspects of AJAX refseqdec reading
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.14 $
** @modified Oct 5 pmr First version
** @modified $Date: 2012/07/12 12:09:15 $ by $Author: rice $
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

#include "ajrefseqread.h"
#include "ajrefseq.h"
#include "ajcall.h"
#include "ajlist.h"
#include "ajquery.h"
#include "ajtextread.h"
#include "ajnam.h"
#include "ajfileio.h"
#include "ajseq.h"
#include "ajseqread.h"

#include <string.h>


AjPTable refseqDbMethods = NULL;

static AjPStr refseqinReadLine     = NULL;


static AjBool refseqinReadEmblcon(AjPRefseqin thys, AjPRefseq refseq);




/* @datastatic RefseqPInFormat *************************************************
**
** Reference sequence input formats data structure
**
** @alias RefseqSInFormat
** @alias RefseqOInFormat
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

typedef struct RefseqSInFormat
{
    const char *Name;
    const char *Obo;
    const char *Desc;
    AjBool Alias;
    AjBool Try;
    AjBool (*Read) (AjPRefseqin thys, AjPRefseq refseq);
} RefseqOInFormat;

#define RefseqPInFormat RefseqOInFormat*

static RefseqOInFormat refseqinFormatDef[] =
{
/* "Name",        "OBOterm", "Description" */
/*     Alias,   Try,     */
/*     ReadFunction */
  {"unknown",     "0000", "Unknown format",
       AJFALSE, AJFALSE,
       &refseqinReadEmblcon}, /* default to first format */
  {"emblcon",     "0000", "EMBL CON format",
       AJFALSE, AJTRUE,
       &refseqinReadEmblcon},
  {"embl",     "0000", "EMBL CON format",
       AJTRUE,  AJFALSE,
       &refseqinReadEmblcon},
  {NULL, NULL, NULL, 0, 0, NULL}
};



static ajuint refseqinReadFmt(AjPRefseqin refseqin, AjPRefseq refseq,
                           ajuint format);
static AjBool refseqinRead(AjPRefseqin refseqin, AjPRefseq refseq);
static AjBool refseqinformatFind(const AjPStr format, ajint* iformat);
static AjBool refseqinFormatSet(AjPRefseqin refseqin, AjPRefseq refseq);
static AjBool refseqinListProcess(AjPRefseqin refseqin, AjPRefseq refseq,
                               const AjPStr listfile);
static void refseqinListNoComment(AjPStr* text);
static void refseqinQryRestore(AjPRefseqin refseqin, const AjPQueryList node);
static void refseqinQrySave(AjPQueryList node, const AjPRefseqin refseqin);
static AjBool refseqDefine(AjPRefseq thys, AjPRefseqin refseqin);
static AjBool refseqinQryProcess(AjPRefseqin refseqin, AjPRefseq refseq);
static AjBool refseqinQueryMatch(const AjPQuery thys, const AjPRefseq refseq);




/* @filesection ajrefseqread **************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
*/




/* @datasection [AjPRefseqin] Reference sequence input objects *****************
**
** Function is for manipulating reference sequence input objects
**
** @nam2rule Refseqin
******************************************************************************/




/* @section Reference sequence input constructors ******************************
**
** All constructors return a new reference sequence input object by pointer. It
** is the responsibility of the user to first destroy any previous
** reference sequence input object. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so
** anyway.
**
** @fdata [AjPRefseqin]
**
** @nam3rule  New     Construct a new reference sequence input object
**
** @valrule   *  [AjPRefseqin] New reference sequence input object
**
** @fcategory new
**
******************************************************************************/




/* @func ajRefseqinNew *********************************************************
**
** Creates a new reference sequence input object.
**
** @return [AjPRefseqin] New reference sequence input object.
** @category new [AjPRefseqin] Default constructor
** @@
******************************************************************************/

AjPRefseqin ajRefseqinNew(void)
{
    AjPRefseqin pthis;

    AJNEW0(pthis);

    pthis->Input = ajTextinNewDatatype(AJDATATYPE_REFSEQ);

    pthis->RefseqData      = NULL;

    return pthis;
}





/* @section reference sequence input destructors *******************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the reference sequence input object.
**
** @fdata [AjPRefseqin]
**
** @nam3rule Del Destructor
**
** @argrule Del pthis [AjPRefseqin*] Reference sequence input object
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajRefseqinDel *********************************************************
**
** Deletes a reference sequence input object.
**
** @param [d] pthis [AjPRefseqin*] Reference sequence input
** @return [void]
** @category delete [AjPRefseqin] Default destructor
** @@
******************************************************************************/

void ajRefseqinDel(AjPRefseqin* pthis)
{
    AjPRefseqin thys;

    if(!pthis)
        return;

    thys = *pthis;

    if(!thys)
        return;

    ajDebug("ajRefseqinDel called qry:'%S'\n", thys->Input->Qry);

    ajTextinDel(&thys->Input);

    AJFREE(*pthis);

    return;
}




/* @section reference sequence input modifiers *********************************
**
** These functions use the contents of a reference sequence input object and
** update them.
**
** @fdata [AjPRefseqin]
**
** @nam3rule Clear Clear all values
** @nam3rule Qry Reset using a query string
** @suffix C Character string input
** @suffix S String input
**
** @argrule * thys [AjPRefseqin] Reference sequence input object
** @argrule C txt [const char*] Query text
** @argrule S str [const AjPStr] query string
**
** @valrule * [void]
**
** @fcategory modify
**
******************************************************************************/




/* @func ajRefseqinClear *******************************************************
**
** Clears a reference sequence input object back to "as new" condition, except
** for the query list which must be preserved.
**
** @param [w] thys [AjPRefseqin] Reference sequence input
** @return [void]
** @category modify [AjPRefseqin] Resets ready for reuse.
** @@
******************************************************************************/

void ajRefseqinClear(AjPRefseqin thys)
{

    ajDebug("ajRefseqinClear called\n");

    ajTextinClear(thys->Input);

    thys->RefseqData = NULL;

    return;
}




/* @func ajRefseqinQryC ********************************************************
**
** Resets a reference sequence input object using a new Universal
** Query Address
**
** @param [u] thys [AjPRefseqin] Reference sequence input object.
** @param [r] txt [const char*] Query
** @return [void]
** @@
******************************************************************************/

void ajRefseqinQryC(AjPRefseqin thys, const char* txt)
{
    ajRefseqinClear(thys);
    ajStrAssignC(&thys->Input->Qry, txt);

    return;
}





/* @func ajRefseqinQryS ********************************************************
**
** Resets a reference sequence input object using a new Universal
** Query Address
**
** @param [u] thys [AjPRefseqin] Reference sequence input object.
** @param [r] str [const AjPStr] Query
** @return [void]
** @@
******************************************************************************/

void ajRefseqinQryS(AjPRefseqin thys, const AjPStr str)
{
    ajRefseqinClear(thys);
    ajStrAssignS(&thys->Input->Qry, str);

    return;
}




/* @section casts *************************************************************
**
** Return values
**
** @fdata [AjPRefseqin]
**
** @nam3rule Trace Write debugging output
**
** @argrule * thys [const AjPRefseqin] Reference sequence input object
**
** @valrule * [void]
**
** @fcategory cast
**
******************************************************************************/




/* @func ajRefseqinTrace *******************************************************
**
** Debug calls to trace the data in a reference sequence input object.
**
** @param [r] thys [const AjPRefseqin] Reference sequence input object.
** @return [void]
** @@
******************************************************************************/

void ajRefseqinTrace(const AjPRefseqin thys)
{
    ajDebug("reference sequence input trace\n");
    ajDebug("====================\n\n");

    ajTextinTrace(thys->Input);

    if(thys->RefseqData)
	ajDebug( "  RefseqData: exists\n");

    return;
}




/* @section Reference sequence data inputs ************************************
**
** These functions read the wxyxdesc data provided by the first argument
**
** @fdata [AjPRefseqin]
**
** @nam3rule Read Read reference sequence data
**
** @argrule Read refseqin [AjPRefseqin] Reference sequence input object
** @argrule Read refseq [AjPRefseq] Reference sequence data
**
** @valrule * [AjBool] true on success
**
** @fcategory input
**
******************************************************************************/




/* @func ajRefseqinRead ********************************************************
**
** If the file is not yet open, calls refseqinQryProcess to convert the query
** into an open file stream.
**
** Uses refseqinRead for the actual file reading.
**
** Returns the results in the AjPRefseq object.
**
** @param [u] refseqin [AjPRefseqin] Reference sequence data input definitions
** @param [w] refseq [AjPRefseq] Reference sequence data returned.
** @return [AjBool] ajTrue on success.
** @category input [AjPRefseq] Master reference sequence data input,
**                  calls specific functions for file access type
**                  and reference sequence data format.
** @@
******************************************************************************/

AjBool ajRefseqinRead(AjPRefseqin refseqin, AjPRefseq refseq)
{
    AjBool ret       = ajFalse;
    AjPQueryList node = NULL;
    AjBool listdata  = ajFalse;

    if(refseqin->Input->Filebuff)
    {
	/* (a) if file still open, keep reading */
	ajDebug("ajRefseqinRead: input file '%F' still there, try again\n",
		refseqin->Input->Filebuff->File);
	ret = refseqinRead(refseqin, refseq);
	ajDebug("ajRefseqinRead: open buffer  qry: '%S' returns: %B\n",
		refseqin->Input->Qry, ret);
    }
    else
    {
	/* (b) if we have a list, try the next query in the list */
	if(ajListGetLength(refseqin->Input->List))
	{
	    listdata = ajTrue;
	    ajListPop(refseqin->Input->List, (void**) &node);

	    ajDebug("++pop from list '%S'\n", node->Qry);
	    ajRefseqinQryS(refseqin, node->Qry);
	    ajDebug("++SAVE WXYZIN '%S' '%S' %d\n",
		    refseqin->Input->Qry,
		    refseqin->Input->Formatstr, refseqin->Input->Format);

            refseqinQryRestore(refseqin, node);

	    ajStrDel(&node->Qry);
	    ajStrDel(&node->Formatstr);
	    AJFREE(node);

	    ajDebug("ajRefseqinRead: open list, try '%S'\n",
                    refseqin->Input->Qry);

	    if(!refseqinQryProcess(refseqin, refseq) &&
               !ajListGetLength(refseqin->Input->List))
		return ajFalse;

	    ret = refseqinRead(refseqin, refseq);
	    ajDebug("ajRefseqinRead: list qry: '%S' returns: %B\n",
		    refseqin->Input->Qry, ret);
	}
	else
	{
	    ajDebug("ajRefseqinRead: no file yet - test query '%S'\n",
                    refseqin->Input->Qry);

	    /* (c) Must be a query - decode it */
	    if(!refseqinQryProcess(refseqin, refseq) &&
               !ajListGetLength(refseqin->Input->List))
		return ajFalse;

	    if(ajListGetLength(refseqin->Input->List)) /* could be a new list */
		listdata = ajTrue;

	    ret = refseqinRead(refseqin, refseq);
	    ajDebug("ajRefseqinRead: new qry: '%S' returns: %B\n",
		    refseqin->Input->Qry, ret);
	}
    }

    /* Now read whatever we got */

    while(!ret && ajListGetLength(refseqin->Input->List))
    {
	/* Failed, but we have a list still - keep trying it */
        if(listdata)
	    ajErr("Failed to read reference sequence data '%S'",
                  refseqin->Input->Qry);

	listdata = ajTrue;
	ajListPop(refseqin->Input->List,(void**) &node);
	ajDebug("++try again: pop from list '%S'\n", node->Qry);
	ajRefseqinQryS(refseqin, node->Qry);
	ajDebug("++SAVE (AGAIN) WXYZIN '%S' '%S' %d\n",
		refseqin->Input->Qry,
		refseqin->Input->Formatstr, refseqin->Input->Format);

	refseqinQryRestore(refseqin, node);

	ajStrDel(&node->Qry);
	ajStrDel(&node->Formatstr);
	AJFREE(node);

	if(!refseqinQryProcess(refseqin, refseq))
	    continue;

	ret = refseqinRead(refseqin, refseq);
	ajDebug("ajRefseqinRead: list retry qry: '%S' returns: %B\n",
		refseqin->Input->Qry, ret);
    }

    if(!ret)
    {
	if(listdata)
	    ajErr("Failed to read reference sequence data '%S'",
                  refseqin->Input->Qry);

	return ajFalse;
    }


    refseqDefine(refseq, refseqin);

    return ajTrue;
}




/* @funcstatic refseqinQueryMatch *********************************************
**
** Compares a reference sequence data item to a query and returns true
** if they match.
**
** @param [r] thys [const AjPQuery] query.
** @param [r] refseq [const AjPRefseq] Reference sequence data.
** @return [AjBool] ajTrue if the reference sequence data matches the query.
** @@
******************************************************************************/

static AjBool refseqinQueryMatch(const AjPQuery thys, const AjPRefseq refseq)
{
    AjBool tested = ajFalse;
    AjIList iterfield  = NULL;
    AjPQueryField field = NULL;
    AjBool ok = ajFalse;

    ajDebug("refseqinQueryMatch '%S' fields: %Lu Case %B Done %B\n",
	    refseq->Id, ajListGetLength(thys->QueryFields),
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
                    refseq->Id);
            if(thys->CaseId)
            {
                if(ajStrMatchWildS(refseq->Id, field->Wildquery))
                {
                    ajListIterDel(&iterfield);
                    return ajTrue;
                }
            }
            else
            {
                if(ajStrMatchWildCaseS(refseq->Id, field->Wildquery))
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
            if(ajStrMatchWildCaseS(refseq->Id, field->Wildquery))
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




/* @funcstatic refseqDefine ****************************************************
**
** Make sure all reference sequence data object attributes are defined
** using values from the reference sequence input object if needed
**
** @param [w] thys [AjPRefseq] Reference sequence data returned.
** @param [u] refseqin [AjPRefseqin] Reference sequence data input definitions
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool refseqDefine(AjPRefseq thys, AjPRefseqin refseqin)
{

    /* if values are missing in the reference sequence object,
       we can use defaults from refseqin or calculate where possible */

    /* assign the dbname if defined in the refseqin object */
    if(ajStrGetLen(refseqin->Input->Db))
      ajStrAssignS(&thys->Db, refseqin->Input->Db);

    return ajTrue;
}





/* @funcstatic refseqinReadFmt *************************************************
**
** Tests whether reference sequence data can be read using the
** specified format.  Then tests whether the reference sequence data
** matches reference sequence data query criteria and checks any
** specified type. Applies upper and lower case.
**
** @param [u] refseqin [AjPRefseqin] Reference sequence data input object
** @param [w] refseq [AjPRefseq] Reference sequence data object
** @param [r] format [ajuint] input format code
** @return [ajuint] 0 if successful.
**                  1 if the query match failed.
**                  2 if the reference sequence data type failed
**                  3 if it failed to read any reference sequence data
** @@
**
** This is the only function that calls the appropriate Read function
** refseqinReadXxxxxx where Xxxxxxx is the supported reference
** sequence data format.
**
** Some of the refseqReadXxxxxx functions fail to reset the buffer correctly,
** which is a very serious problem when cycling through all of them to
** identify an unknown format. The extra ajFileBuffReset call at the end is
** intended to address this problem. The individual functions should still
** reset the buffer in case they are called from elsewhere.
**
******************************************************************************/

static ajuint refseqinReadFmt(AjPRefseqin refseqin, AjPRefseq refseq,
                              ajuint format)
{
    ajDebug("++refseqinReadFmt format %d (%s) '%S'\n",
	    format, refseqinFormatDef[format].Name,
	    refseqin->Input->Qry);

    refseqin->Input->Records = 0;

    /* Calling funclist refseqinFormatDef() */
    if((*refseqinFormatDef[format].Read)(refseqin, refseq))
    {
	ajDebug("refseqinReadFmt success with format %d (%s)\n",
		format, refseqinFormatDef[format].Name);
        ajDebug("id: '%S'\n",
                refseq->Id);
	refseqin->Input->Format = format;
	ajStrAssignC(&refseqin->Input->Formatstr,
	             refseqinFormatDef[format].Name);
	ajStrAssignC(&refseq->Formatstr,
	             refseqinFormatDef[format].Name);
	ajStrAssignEmptyS(&refseq->Db, refseqin->Input->Db);
	ajStrAssignS(&refseq->Filename, refseqin->Input->Filename);

	if(refseqinQueryMatch(refseqin->Input->Query, refseq))
	{
            /* ajRefseqinTrace(refseqin); */

            return FMT_OK;
        }

	ajDebug("query match failed, continuing ...\n");
	ajRefseqClear(refseq);

	return FMT_NOMATCH;
    }
    else
    {
	ajDebug("Testing input buffer: IsBuff: %B Eof: %B\n",
		ajFilebuffIsBuffered(refseqin->Input->Filebuff),
		ajFilebuffIsEof(refseqin->Input->Filebuff));

	if (!ajFilebuffIsBuffered(refseqin->Input->Filebuff) &&
	    ajFilebuffIsEof(refseqin->Input->Filebuff))
	    return FMT_EOF;

	ajFilebuffReset(refseqin->Input->Filebuff);
	ajDebug("Format %d (%s) failed, file buffer reset by refseqinReadFmt\n",
		format, refseqinFormatDef[format].Name);
	/* ajFilebuffTraceFull(refseqin->Filebuff, 10, 10);*/
    }

    ajDebug("++refseqinReadFmt failed - nothing read\n");

    return FMT_FAIL;
}




/* @funcstatic refseqinRead ****************************************************
**
** Given data in a refseqin structure, tries to read everything needed
** using the specified format or by trial and error.
**
** @param [u] refseqin [AjPRefseqin] Reference sequence data input object
** @param [w] refseq [AjPRefseq] Reference sequence data object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool refseqinRead(AjPRefseqin refseqin, AjPRefseq refseq)
{
    ajuint i;
    ajuint istat = 0;
    ajuint jstat = 0;

    AjPFilebuff buff = refseqin->Input->Filebuff;
    /*AjBool regfile = ajFalse;*/
    AjBool ok;

    AjPTextAccess  textaccess  = refseqin->Input->Query->TextAccess;
    AjPRefseqAccess refseqaccess = refseqin->Input->Query->Access;

    ajRefseqClear(refseq);
    ajDebug("refseqinRead: cleared\n");

    if(refseqin->Input->Single && refseqin->Input->Count)
    {
	/*
	** One reference sequence data item at a time is read.
	** The first reference sequence data item was read by ACD
	** for the following ones we need to reset the AjPRefseqin
	**
	** Single is set by the access method
	*/

	ajDebug("refseqinRead: single access - count %d - call access"
		" routine again\n",
		refseqin->Input->Count);
	/* Calling funclist refseqinAccess() */
	if(textaccess)
        {
            if(!(*textaccess->Access)(refseqin->Input))
            {
                ajDebug("refseqinRead: (*textaccess->Access)(refseqin->Input) "
                        "*failed*\n");

                return ajFalse;
            }
        }

	if(refseqaccess)
        {
            if(!(*refseqaccess->Access)(refseqin))
            {
                ajDebug("refseqinRead: (*refseqaccess->Access)(refseqin) "
                        "*failed*\n");

                return ajFalse;
            }
        }

        buff = refseqin->Input->Filebuff;
    }

    ajDebug("refseqinRead: refseqin format %d '%S'\n", refseqin->Input->Format,
	    refseqin->Input->Formatstr);

    refseqin->Input->Count++;

    if(!refseqin->Input->Filebuff)
	return ajFalse;

    ok = ajFilebuffIsBuffered(refseqin->Input->Filebuff);

    while(ok)
    {				/* skip blank lines */
        ok = ajBuffreadLine(refseqin->Input->Filebuff, &refseqinReadLine);

        if(!ajStrIsWhite(refseqinReadLine))
        {
            ajFilebuffClear(refseqin->Input->Filebuff,1);
            break;
        }
    }

    if(!refseqin->Input->Format)
    {			   /* no format specified, try all defaults */

     /*regfile = ajFileIsFile(ajFilebuffGetFile(refseqin->Input->Filebuff));*/

	for(i = 1; refseqinFormatDef[i].Name; i++)
	{
	    if(!refseqinFormatDef[i].Try)	/* skip if Try is ajFalse */
		continue;

	    ajDebug("refseqinRead:try format %d (%s)\n",
		    i, refseqinFormatDef[i].Name);

	    istat = refseqinReadFmt(refseqin, refseq, i);

	    switch(istat)
	    {
	    case FMT_OK:
		ajDebug("++refseqinRead OK, set format %d\n",
                        refseqin->Input->Format);
		refseqDefine(refseq, refseqin);

		return ajTrue;
	    case FMT_BADTYPE:
		ajDebug("refseqinRead: (a1) "
                        "refseqinReadFmt stat == BADTYPE *failed*\n");

		return ajFalse;
	    case FMT_FAIL:
		ajDebug("refseqinRead: (b1) "
                        "refseqinReadFmt stat == FAIL *failed*\n");
		break;			/* we can try next format */
	    case FMT_NOMATCH:
		ajDebug("refseqinRead: (c1) "
                        "refseqinReadFmt stat==NOMATCH try again\n");
		break;
	    case FMT_EOF:
		ajDebug("refseqinRead: (d1) "
                        "refseqinReadFmt stat == EOF *failed*\n");
		return ajFalse;			/* EOF and unbuffered */
	    case FMT_EMPTY:
		ajWarn("reference sequence data '%S' has zero length, ignored",
		       ajRefseqGetQryS(refseq));
		ajDebug("refseqinRead: (e1) "
                        "refseqinReadFmt stat==EMPTY try again\n");
		break;
	    default:
		ajDebug("unknown code %d from refseqinReadFmt\n", stat);
	    }

	    ajRefseqClear(refseq);

	    if(refseqin->Input->Format)
		break;			/* we read something */

            ajFilebuffTrace(refseqin->Input->Filebuff);
	}

	if(!refseqin->Input->Format)
	{		     /* all default formats failed, give up */
	    ajDebug("refseqinRead:all default formats failed, give up\n");

	    return ajFalse;
	}

	ajDebug("++refseqinRead set format %d\n",
                refseqin->Input->Format);
    }
    else
    {					/* one format specified */
	ajDebug("refseqinRead: one format specified\n");
	ajFilebuffSetUnbuffered(refseqin->Input->Filebuff);

	ajDebug("++refseqinRead known format %d\n",
                refseqin->Input->Format);
	istat = refseqinReadFmt(refseqin, refseq, refseqin->Input->Format);

	switch(istat)
	{
	case FMT_OK:
	    refseqDefine(refseq, refseqin);

	    return ajTrue;
	case FMT_BADTYPE:
	    ajDebug("refseqinRead: (a2) "
                    "refseqinReadFmt stat == BADTYPE *failed*\n");

	    return ajFalse;

        case FMT_FAIL:
	    ajDebug("refseqinRead: (b2) "
                    "refseqinReadFmt stat == FAIL *failed*\n");

	    return ajFalse;

        case FMT_NOMATCH:
	    ajDebug("refseqinRead: (c2) "
                    "refseqinReadFmt stat == NOMATCH *try again*\n");
	    break;
	case FMT_EOF:
	    ajDebug("refseqinRead: (d2) "
                    "refseqinReadFmt stat == EOF *try again*\n");
            if(refseqin->Input->Records)
                ajErr("Error reading file '%F' with format '%s': "
                      "end-of-file before end of data "
                      "(read %u records)",
                      ajFilebuffGetFile(refseqin->Input->Filebuff),
                      refseqinFormatDef[refseqin->Input->Format].Name,
                      refseqin->Input->Records);
	    break;		     /* simply end-of-file */
	case FMT_EMPTY:
	    ajWarn("assmebly data '%S' has zero length, ignored",
		   ajRefseqGetQryS(refseq));
	    ajDebug("refseqinRead: (e2) "
                    "refseqinReadFmt stat == EMPTY *try again*\n");
	    break;
	default:
	    ajDebug("unknown code %d from refseqinReadFmt\n", stat);
	}

	ajRefseqClear(refseq); /* 1 : read, failed to match id/acc/query */
    }

    /* failed - probably entry/accession query failed. Can we try again? */

    ajDebug("refseqinRead failed - try again with format %d '%s' code %d\n",
	    refseqin->Input->Format,
            refseqinFormatDef[refseqin->Input->Format].Name, istat);

    ajDebug("Search:%B Chunk:%B Data:%x ajFileBuffEmpty:%B\n",
	    refseqin->Input->Search, refseqin->Input->ChunkEntries,
            refseqin->Input->TextData, ajFilebuffIsEmpty(buff));

    if(ajFilebuffIsEmpty(buff) && refseqin->Input->ChunkEntries)
    {
	if(textaccess && !(*textaccess->Access)(refseqin->Input))
            return ajFalse;
	else if(refseqaccess && !(*refseqaccess->Access)(refseqin))
            return ajFalse;
        buff = refseqin->Input->Filebuff;
    }


    /* need to check end-of-file to avoid repeats */
    while(refseqin->Input->Search &&
          (refseqin->Input->TextData || !ajFilebuffIsEmpty(buff)))
    {
	jstat = refseqinReadFmt(refseqin, refseq, refseqin->Input->Format);

	switch(jstat)
	{
	case FMT_OK:
	    refseqDefine(refseq, refseqin);

	    return ajTrue;

        case FMT_BADTYPE:
	    ajDebug("refseqinRead: (a3) "
                    "refseqinReadFmt stat == BADTYPE *failed*\n");

	    return ajFalse;

        case FMT_FAIL:
	    ajDebug("refseqinRead: (b3) "
                    "refseqinReadFmt stat == FAIL *failed*\n");

	    return ajFalse;
            
	case FMT_NOMATCH:
	    ajDebug("refseqinRead: (c3) "
                    "refseqinReadFmt stat == NOMATCH *try again*\n");
	    break;
	case FMT_EOF:
	    ajDebug("refseqinRead: (d3) "
                    "refseqinReadFmt stat == EOF *failed*\n");

	    return ajFalse;			/* we already tried again */

        case FMT_EMPTY:
	    if(istat != FMT_EMPTY)
                ajWarn("assmebly data '%S' has zero length, ignored",
                       ajRefseqGetQryS(refseq));
	    ajDebug("refseqinRead: (e3) "
                    "refseqinReadFmt stat == EMPTY *try again*\n");
	    break;

        default:
	    ajDebug("unknown code %d from refseqinReadFmt\n", stat);
	}

	ajRefseqClear(refseq); /* 1 : read, failed to match id/acc/query */
    }

    if(refseqin->Input->Format)
	ajDebug("refseqinRead: *failed* to read reference sequence data %S "
                "using format %s\n",
		refseqin->Input->Qry,
                refseqinFormatDef[refseqin->Input->Format].Name);
    else
	ajDebug("refseqinRead: *failed* to read reference sequence data %S "
                "using any format\n",
		refseqin->Input->Qry);

    return ajFalse;
}




/* @funcstatic refseqinReadEmblcon *********************************************
**
** Given data in a reference sequence structure, tries to read
** everything needed using EMBL CON format.
**
** @param [u] refseqin [AjPRefseqin] Refseq input object
** @param [w] refseq [AjPRefseq] refseq object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool refseqinReadEmblcon(AjPRefseqin refseqin, AjPRefseq refseq)
{
    AjPFilebuff buff;
    AjPTextin input;
    AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    AjBool ok;
    AjIList iter = NULL;
    AjPSeqRange conrange = NULL;
    ajuint fraglen = 0;
    ajuint gaplen = 0;
    ajuint conlen = 0;
    AjBool conrev = ajFalse;
    AjPSeqin conseqin = NULL;
    AjPSeq conseq = NULL;
    AjPStr conqry = NULL;
    AjPStr constr = NULL;
    AjPStr condb = NULL;
    AjPStr confield = NULL;
    AjPStr tmpstr = NULL;
    AjPStr seqFtFmtEmbl = NULL;
    AjBool dofeat        = ajFalse;
    AjPStr liststr;			/* for lists, do not delete */
    AjPStr datestr = NULL;
    AjPStr relstr = NULL;
    AjPStr cmtstr = NULL;		/* stored in AjPSeq - do not delete */
    ajuint icount;
    AjPSeqRef seqref = NULL;
    AjPSeqXref xref  = NULL;
    ajuint refnum;
    ajuint seqlen=1024;
    ajuint tmplen;
    ajuint itmp;
    ajuint i;
    ajint dotpos;
    ajint colonpos;
    AjPStr numstr = NULL;
    ajuint start = 0;
    ajuint end = 0;
    ajint istat = 0;

    /*ajuint taxid = 0;*/

    ajDebug("refseqinReadEmblcon\n");
    ajRefseqClear(refseq);
    input = refseqin->Input;
    buff = input->Filebuff;
    /*textfile = ajFilebuffGetFile(buff);*/

    if(!seqFtFmtEmbl)
	ajStrAssignC(&seqFtFmtEmbl, "embl");

    if(!ajBuffreadLineStore(buff, &refseqinReadLine,
                            input->Text, &refseq->TextPtr))
	return ajFalse;

    /* for GCG formatted databases */

    while(ajStrPrefixC(refseqinReadLine, "WP "))
    {
	if(!ajBuffreadLineStore(buff, &refseqinReadLine,
                                input->Text, &refseq->TextPtr))
	    return ajFalse;
        input->Records++;
    }

    /* extra blank lines */

    while(ajStrIsWhite(refseqinReadLine))
    {
	if(!ajBuffreadLineStore(buff, &refseqinReadLine,
                                input->Text, &refseq->TextPtr))
	    return ajFalse;
    }

    ajDebug("seqReadEmbl first line '%S'\n", refseqinReadLine);

    if(!ajStrPrefixC(refseqinReadLine, "ID   "))
    {
	ajFilebuffResetStore(buff, input->Text, &refseq->TextPtr);

	return ajFalse;
    }
    input->Records++;

    if(input->Text)
	ajStrAssignC(&refseq->TextPtr,ajStrGetPtr(refseqinReadLine));

    ajDebug("seqReadEmbl ID line found\n");
    ajStrTokenAssignC(&handle, refseqinReadLine, " ;\t\n\r");
    ajStrTokenNextParse(&handle, &token);	/* 'ID' */
    ajStrTokenNextParse(&handle, &refseq->Id);	/* entry name */

    /* seqSetName(thys, token); */

    ajStrTokenNextParse(&handle, &token);	/* SV for new syntax */

    if(ajStrMatchC(token, "SV"))	/* new post-2006 EMBL line */
    {
	ajStrTokenNextParse(&handle, &token);	/* SV */
	ajStrInsertK(&token, 0, '.');
	/*ajStrInsertS(&token, 0, refseq->Name);*/
	/* seqSvSave(thys, token); */

	ajStrTokenNextParse(&handle, &token); /* linear or circular */

	ajStrTokenNextParseC(&handle, ";\t\n\r", &token);
	ajStrTrimWhite(&token);
	/*ajSeqmolSetEmbl(&thys->Molecule, token);*/

	ajStrTokenNextParse(&handle, &token);
	ajStrTrimWhite(&token);
	/*ajStrAssignS(&thys->Class, token);*/

	ajStrTokenNextParse(&handle, &token);
	ajStrTrimWhite(&token);
	/*ajStrAssignS(&thys->Division, token);*/

	ajStrTokenNextParse(&handle, &token);
	ajStrTrimEndC(&token, "BP.");
	ajStrTrimWhite(&token);
	ajStrToUint(token, &seqlen);
    }
    else		     /* test for a SwissProt/SpTrEMBL entry */
    {
	if(ajStrFindC(refseqinReadLine, " PRT; ")>= 0  ||
	   ajStrFindC(refseqinReadLine, " Unreviewed; ") >= 0 ||
	   ajStrFindC(refseqinReadLine, " Reviewed; ") >= 0 ||
	   ajStrFindC(refseqinReadLine, " Preliminary; ") >= 0 
	   )
	{
	    ajFilebuffResetStore(buff, input->Text, &refseq->TextPtr);
	    ajStrTokenDel(&handle);
	    ajStrDel(&token);

	    return ajFalse;
	}
    }

    ok = ajBuffreadLineStore(buff, &refseqinReadLine,
                             input->Text, &refseq->TextPtr);

    while(ok &&
          !ajStrPrefixC(refseqinReadLine, "CO") &&
          !ajStrPrefixC(refseqinReadLine, "SQ"))
    {
	input->Records++;

	/* check for Staden Experiment format instead */
	if(ajStrPrefixC(refseqinReadLine, "EN   ") ||
	   ajStrPrefixC(refseqinReadLine, "TN   ") ||
	   ajStrPrefixC(refseqinReadLine, "EX   ") )
	{
	    ajFilebuffResetStore(buff, input->Text, &refseq->TextPtr);
	    ajStrDel(&token);

	    return ajFalse;;
	}

	else if(ajStrPrefixC(refseqinReadLine, "FH   ") ||
		ajStrPrefixC(refseqinReadLine, "AH   "))
	    ok = ajTrue;		/* ignore these lines */

	else if(ajStrPrefixC(refseqinReadLine, "AC   ") ||
	   ajStrPrefixC(refseqinReadLine, "PA   ") ) /* emblcds database format */
	{
	    ajStrTokenAssignC(&handle, refseqinReadLine, " ;\n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'AC' */

	    /* while(ajStrTokenNextParse(&handle, &token))
		seqAccSave(thys, token); */
	}

	else if(ajStrPrefixC(refseqinReadLine, "SV   ") ||
	   ajStrPrefixC(refseqinReadLine, "IV   ") ) /* emblcds database format */
	{
	    ajStrTokenAssignC(&handle, refseqinReadLine, " \n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'SV' */
	    ajStrTokenNextParse(&handle, &token); /* version */
	    /* seqSvSave(thys, token); */
	}

	else if(ajStrPrefixC(refseqinReadLine, "DE   "))
	{
	    ajStrTokenAssignC(&handle, refseqinReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'DE' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* desc */

	    if(ajStrGetLen(refseq->Desc))
	    {
		ajStrAppendC(&refseq->Desc, " ");
		ajStrAppendS(&refseq->Desc, token);
	    }
	    else
		ajStrAssignS(&refseq->Desc, token);
	}

	if(ajStrPrefixC(refseqinReadLine, "KW   "))
	{
	    ajStrTokenAssignC(&handle, refseqinReadLine, " \n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'KW' */

            while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
	    {
		liststr = ajStrNewS(token);
		ajStrTrimWhite(&liststr);
		/*ajSeqAddKey(thys, liststr);*/
	    }
	}

	else if(ajStrPrefixC(refseqinReadLine, "OS   "))
	{
	    ajStrTokenAssignC(&handle, refseqinReadLine, " \n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'OS' */

	    /* maybe better remove . from this, and trim from end */
	    while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
	    {
		ajStrAssignS(&tmpstr, token);
		ajStrTrimWhite(&tmpstr);
		/*seqTaxSave(thys, tmpstr, 1);*/
		ajStrDel(&tmpstr);
	    }
	}

	else if(ajStrPrefixC(refseqinReadLine, "OC   "))
	{
	    ajStrTokenAssignC(&handle, refseqinReadLine, " \n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'OC' */

	    /* maybe better remove . from this, and trim from end */
	    while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
	    {
		ajStrAssignS(&tmpstr, token);
		ajStrTrimWhite(&tmpstr);
		/*seqTaxSave(thys, tmpstr, 0);*/
		ajStrDel(&tmpstr);
	    }
	}

	else if(ajStrPrefixC(refseqinReadLine, "OG   "))
	{
	    ajStrTokenAssignC(&handle, refseqinReadLine, " \n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'OG' */

	    /* maybe better remove . from this, and trim from end */
	    while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
	    {
		ajStrAssignS(&tmpstr, token);
		ajStrTrimWhite(&tmpstr);
		/*seqTaxSave(thys, tmpstr, 2);*/
		ajStrDel(&tmpstr);
	    }
	}

	else if(ajStrPrefixC(refseqinReadLine, "CC   "))
	{
	    ajStrTokenAssignC(&handle, refseqinReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'CC' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* comment */

	    if(ajStrGetLen(cmtstr))
		ajStrAppendC(&cmtstr, "\n");
	    ajStrAppendS(&cmtstr, token);

/* trying to keep comments in one long string with embedded returns
** probably fails for long comments - and also fails for contact details
** which have very short comment lines
** switch to just keeping original lines */

/*
	    if(ajStrGetLen(cmtstr))
	    {
		if(ajStrGetLen(token))
		{
		    if(ajStrGetCharLast(cmtstr) != '\n')
			ajStrAppendK(&cmtstr, ' ');
		    ajStrAppendS(&cmtstr, token);
		}
		else
		{
		    if(ajStrGetCharLast(cmtstr) != '\n')
			ajStrAppendK(&cmtstr, '\n');
		    ajStrAppendC(&cmtstr, " \n");
		}
	    }
	    else
		ajStrAssignS(&cmtstr, token);
	    if(ajStrGetCharLast(token) == '.')
		ajStrAppendK(&cmtstr, '\n');
*/
	}

	else if(ajStrPrefixC(refseqinReadLine, "DR   "))
	{
            AJNEW0(xref);
	    ajStrTokenAssignC(&handle, refseqinReadLine, " ;\n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'DR' */
	    ajStrTokenNextParseC(&handle, ";\n\r", &token); /* dbname */
	    ajStrAssignS(&xref->Db, token);
            ajStrTrimWhite(&token);
	    ajStrTokenNextParse(&handle, &token); /* primary */
            ajStrTrimWhite(&token);
	    ajStrAssignS(&xref->Id, token);
	    ajStrTokenNextParse(&handle, &token); /* secondary*/

            if(!ajStrGetLen(token))
            {
                if(ajStrGetCharLast(xref->Id) == '.')
                    ajStrCutEnd(&xref->Id, 1);
            }
            else
            {
                if(ajStrGetCharLast(token) == '.')
                    ajStrCutEnd(&token, 1);
                ajStrTrimWhite(&token);
                ajStrAssignS(&xref->Secid, token);

                ajStrTokenNextParse(&handle, &token); /* secondary*/

                if(!ajStrGetLen(token))
                {
                    if(ajStrGetCharLast(xref->Secid) == '.')
                        ajStrCutEnd(&xref->Secid, 1);
                }
                else
                {
                    if(ajStrGetCharLast(token) == '.')
                        ajStrCutEnd(&token, 1);
                    ajStrTrimWhite(&token);
                    ajStrAssignS(&xref->Terid, token);

                    ajStrTokenNextParse(&handle, &token); /* secondary*/

                    if(!ajStrGetLen(token))
                    {
                        if(ajStrGetCharLast(xref->Terid) == '.')
                            ajStrCutEnd(&xref->Terid, 1);
                    }
                    else
                    {
                        if(ajStrGetCharLast(token) == '.')
                            ajStrCutEnd(&token, 1);
                        ajStrTrimWhite(&token);
                        ajStrAssignS(&xref->Quatid, token);
                    }
                }
            }
            xref->Type = XREF_DR;
	    /*ajSeqAddXref(thys, xref);*/
	}

	else if(ajStrPrefixC(refseqinReadLine, "RN   "))
	{
	    if(!seqref)
		seqref = ajSeqrefNew();
	    ajStrTokenAssignC(&handle, refseqinReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'RN' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* [num] */
	    ajStrAssignSubS(&tmpstr, token, 1, -2);
	    ajStrToUint(tmpstr, &refnum);
	    ajSeqrefSetnumNumber(seqref, refnum);
	}

	else if(ajStrPrefixC(refseqinReadLine, "RG   "))
	{
	    if(!seqref)
		seqref = ajSeqrefNew();

	    ajStrTokenAssignC(&handle, refseqinReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'RG' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* groupname */
	    ajSeqrefAppendGroupname(seqref, token);
	}

	else if(ajStrPrefixC(refseqinReadLine, "RX   "))
	{
	    if(!seqref)
		seqref = ajSeqrefNew();

	    ajStrTokenAssignC(&handle, refseqinReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'RX' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* xref */
	    ajSeqrefAppendXref(seqref, token);
	}

	else if(ajStrPrefixC(refseqinReadLine, "RP   "))
	{
	    if(!seqref)
		seqref = ajSeqrefNew();

	    ajStrTokenAssignC(&handle, refseqinReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'RP' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* position */
	    ajSeqrefAppendPosition(seqref, token);
	}

	else if(ajStrPrefixC(refseqinReadLine, "RA   "))
	{
	    if(!seqref)
		seqref = ajSeqrefNew();

	    ajStrTokenAssignC(&handle, refseqinReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'RA' */
	    ajStrTokenNextParseC(&handle, "\n\r;", &token); /* authors */
	    ajSeqrefAppendAuthors(seqref, token);
	}

	else if(ajStrPrefixC(refseqinReadLine, "RT   "))
	{
	    if(!seqref)
		seqref = ajSeqrefNew();

	    ajStrTokenAssignC(&handle, refseqinReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'RT' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* title */

	    if(!ajStrMatchC(token, ";"))
		ajSeqrefAppendTitle(seqref, token);
	}

	else if(ajStrPrefixC(refseqinReadLine, "RL   "))
	{
	    if(!seqref)
		seqref = ajSeqrefNew();

	    ajStrTokenAssignC(&handle, refseqinReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'RL' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* authors */
	    ajSeqrefAppendLocation(seqref, token);
	}

	else if(ajStrPrefixC(refseqinReadLine, "RC   "))
	{
	    if(!seqref)
		seqref = ajSeqrefNew();

	    ajStrTokenAssignC(&handle, refseqinReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'RC' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* comment */
	    ajSeqrefAppendComment(seqref, token);
	}

	else if(ajStrPrefixC(refseqinReadLine, "FT   "))
	{
            if(!dofeat)
            {
                dofeat = ajTrue;
                /*seqin->Ftquery = ajFeattabinNewSS(seqFtFmtEmbl,
                  thys->Name, "N"); */
            }

            /* ajFilebuffLoadS(seqin->Ftquery->Input->Filebuff,
               refseqinReadLine);*/
	}

	else if(ajStrPrefixC(refseqinReadLine, "DT   "))
	{
	    /*if(!thys->Date)
              thys->Date = ajSeqdateNew();*/

	    ajStrTokenAssignC(&handle, refseqinReadLine, " (),");
	    icount = 0;

	    while(ajStrTokenNextParse(&handle, &token))
	    {
		icount++;

		if(icount==2)
		    ajStrAssignS(&datestr, token);
		else if(icount==4)
		    ajStrAssignS(&relstr, token);
		else if(icount==5)
		{
		    if(ajStrMatchC(token, "Created"))
		    {
                        /*
			ajSeqdateSetCreateS(thys->Date, datestr);
			ajStrAssignS(&thys->Date->CreRel, relstr);
                        */
		    }
		}
		else if(icount==8)
		{
                    /*
                      ajSeqdateSetModifyS(thys->Date, datestr);
                      ajStrAssignS(&thys->Date->ModRel, relstr);
                      ajStrAssignS(&thys->Date->ModVer, token);
                    */
		}
	    }
	}


	else if(ajStrPrefixC(refseqinReadLine, "XX"))
	{
	    if(seqref)
	    {
                ajSeqrefStandard(seqref);
		/*ajSeqAddRef(thys, seqref);*/
		/*seqref = NULL;*/
	    }
	    if(ajStrGetLen(cmtstr))
	    {
                /*ajSeqAddCmt(thys, cmtstr);*/
		/*cmtstr = NULL;*/
	    }

	}

/* ignored line types */

/* other line types */
/*
	if(ajStrPrefixC(refseqinReadLine, "RN   "))
	if(ajStrPrefixC(refseqinReadLine, "RC   "))
	if(ajStrPrefixC(refseqinReadLine, "RP   "))
	if(ajStrPrefixC(refseqinReadLine, "RX   "))
	if(ajStrPrefixC(refseqinReadLine, "RG   "))
	if(ajStrPrefixC(refseqinReadLine, "RA   "))
	if(ajStrPrefixC(refseqinReadLine, "RT   "))
	if(ajStrPrefixC(refseqinReadLine, "RL   "))
	if(ajStrPrefixC(refseqinReadLine, "AS   "))
	if(ajStrPrefixC(refseqinReadLine, "CO   "))
	if(ajStrPrefixC(refseqinReadLine, "CC   "))
*/

	ok = ajBuffreadLineStore(buff, &refseqinReadLine,
				input->Text, &refseq->TextPtr);
    }

    if(dofeat)
    {
	/* ajDebug("EMBL FEAT TabIn %x\n", seqin->Ftquery); */
/*	ajFeattableDel(&thys->Fttable);
	thys->Fttable = ajFeattableNewRead(seqin->Ftquery);
	ajFeattabinClear(seqin->Ftquery);*/
    }

    if(ajStrPrefixC(refseqinReadLine, "SQ"))
    {
        ajStrTokenAssignC(&handle, refseqinReadLine, " ");
        ajStrTokenNextParse(&handle, &token); /* 'SQ' */
        ajStrTokenNextParse(&handle, &token); /* 'Sequence' */
        ajStrTokenNextParse(&handle, &token); /* len */
        ajStrToUint(token, &tmplen);

        if(tmplen > seqlen)
            seqlen = tmplen;

        ajStrTokenNextParse(&handle, &token); /* BP; */
        tmplen = 0;

        for(i=0;i<4;i++)
        {
            ajStrTokenNextParse(&handle, &token); /* count */
            ajStrToUint(token, &itmp);
            ajStrTokenNextParse(&handle, &token); /* 'A' 'C' 'G' 'T' 'other' */
            tmplen += itmp;
        }

        if(tmplen > seqlen)
            seqlen = tmplen;

        /* read the sequence and terminator */
        ok = ajBuffreadLineStore(buff, &refseqinReadLine,
                                 input->Text, &refseq->TextPtr);
        /*ajStrSetRes(&thys->Seq, seqlen+1);*/

        while(ok && !ajStrPrefixC(refseqinReadLine, "//"))
        {
            /*seqAppend(&thys->Seq, refseqinReadLine);*/
            input->Records++;
            ok = ajBuffreadLineStore(buff, &refseqinReadLine,
                                     input->Text, &refseq->TextPtr);
        }
    }
    else if (ajStrPrefixC(refseqinReadLine, "CO"))
    {
        if(!constr)
            constr = ajStrNewRes(4096);

        while(ok && ajStrPrefixC(refseqinReadLine, "CO"))
        {
            ajStrTrimWhiteEnd(&refseqinReadLine);
            ajStrAppendSubS(&constr, refseqinReadLine, 5, -1);
            ok = ajBuffreadLineStore(buff, &refseqinReadLine,
                                     input->Text,  &refseq->TextPtr);
        }

        if(ajStrPrefixC(constr, "join(") && ajStrSuffixC(constr, ")"))
        {
            ajStrCutEnd(&constr, 1);
            ajStrCutStart(&constr, 5);
        }

        if(!refseq->Seqlist)
            refseq->Seqlist = ajListstrNew();
        ajStrTokenAssignC(&handle, constr, ",");

        conlen = 0;

        while(ajStrTokenNextParse(&handle, &token))
        {
            if(ajStrPrefixC(token, "gap("))
            {
                ajDebug("%Lu: '%S'\n",
                       ajListGetLength(refseq->Seqlist), token);
                ajStrCutEnd(&token, 1);
                ajStrCutStart(&token, 4);
                if(ajStrToUint(token, &gaplen))
                {
                    ajDebug("gap %u bases total %u\n",
                           gaplen, ajStrGetLen(refseq->Seq));
                }
                else
                    ajWarn("Unknown gap length in '%S'", constr);

                conrange = ajSeqrangeNewValues(conlen, gaplen, NULL);
                ajListPushAppend(refseq->Seqlist, conrange);
                conrange = NULL;
                conlen += gaplen;
            }
            else
            {
                if(ajStrPrefixC(token, "complement("))
                {
                    ajStrCutEnd(&token, 1);
                    ajStrCutStart(&token, 11);
                    conrev = ajTrue;
                }

                if(!condb)
                {
                    if(!ajNamDbGetAttrSpecialC(input->Db, "ConDatabase",
                                               &condb))
                        ajStrAssignS(&condb, input->Db);
                    if(!ajNamDbGetAttrSpecialC(input->Db, "ConField",
                                               &confield))
                        ajStrAssignC(&confield, "sv");
                }

                dotpos   = (ajint) ajStrFindAnyK(token, '.');
                colonpos = (ajint) ajStrFindAnyK(token, ':');
                ajStrAssignSubS(&numstr, token, colonpos+1, -1);
                istat = ajFmtScanS(numstr, "%u..%u", &start, &end);
                if(istat != 2)
                {
                    ajWarn("EMBLCON badly formed fragment '%S'", token);
                    start = 1;
                    end = 0;
                }

                fraglen = 1 + end - start;
                if(ajStrMatchC(confield, "sv"))
                {
                    ajFmtPrintS(&conqry, "%S-sv:%S", input->Db, token);
                    if(conrev)
                        ajStrAppendC(&conqry, ":r");
                }
                else
                {
                    if((dotpos > 0) && (dotpos < colonpos))
                    {
                        ajStrCutRange(&token, dotpos, colonpos-1);
                        ajFmtPrintS(&conqry, "%S-%S:%S",
                                    condb, confield, token);
                        if(conrev)
                            ajStrAppendC(&conqry, ":r");
                    }
                }
                ajDebug("%Lu: '%S' '%S' rev:%B\n",
                        ajListGetLength(refseq->Seqlist),
                        token, conqry, conrev);

                conrange = ajSeqrangeNewValues(conlen,
                                               fraglen,
                                               conqry);
                ajListPushAppend(refseq->Seqlist, conrange);
                conrange = NULL;
                conlen += fraglen;
            }
        }

        ajDebug("conlen: %u seqlen: %u\n",
                conlen, seqlen);
        ajStrTokenDel(&handle);
    }

    if(ok &&!ajStrPrefixC(refseqinReadLine, "//"))
        ajWarn("Unexpected line after sequence data: %S", refseqinReadLine);
    if(!ok)
        ajWarn("Unexpected end of file after sequence data");

    conseqin = ajSeqinNew();
    conseq = ajSeqNew();
    iter = ajListIterNewread(refseq->Seqlist);

    while(!ajListIterDone(iter))
    {
        conrange = ajListIterGet(iter);
        if(conrange->Start != ajStrGetLen(refseq->Seq))
            ajWarn("EMBLCON entry '%S' reading at %u expected %Lu",
                   refseq->Id, ajStrGetLen(refseq->Seq), conrange->Start);
        if(conrange->Query)
        {
            ajSeqinUsa(&conseqin, conrange->Query);

            if(!ajSeqRead(conseq, conseqin))
                ajErr("EMBLCON entry '%S' failed to read '%S'",
                      refseq->Id, conrange->Query);
            else
            {
                ajSeqTrim(conseq);
                if(conrev)
                    ajSeqReverseDo(conseq);
                ajStrAppendS(&refseq->Seq,ajSeqGetSeqS(conseq));
                ajDebug("Read %u bases total %u\n",
                        ajSeqGetLen(conseq), ajStrGetLen(refseq->Seq));
            }
        }
        else
        {
            ajStrAppendCountK(&refseq->Seq, 'N', conrange->Length);
        }
    }

    ajListIterDel(&iter);
    ajSeqDel(&conseq);
    ajSeqinDel(&conseqin);

    conrange = NULL;

    ajFilebuffClear(buff, 0);

    ajSeqrefDel(&seqref);
    ajSeqxrefDel(&xref);
    ajStrDel(&tmpstr);
    ajStrDel(&cmtstr);
    ajStrDel(&liststr);
    ajStrDel(&constr);
    ajStrDel(&conqry);
    ajStrDel(&condb);
    ajStrDel(&confield);
    ajStrDel(&token);
    ajStrDel(&datestr);
    ajStrDel(&relstr);
    ajStrDel(&numstr);
    ajStrDel(&seqFtFmtEmbl);
    ajStrTokenDel(&handle);

    return ajTrue;
}




/* @datasection [none] Miscellaneous ******************************************
**
** Reference sequence input internals
**
** @nam2rule Refseqin Reference sequence input
**
******************************************************************************/




/* @section Printing **********************************************************
**
** Printing details of the internals to a file
**
** @fdata [none]
**
** @nam2rule Refseqinprint
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




/* @func ajRefseqinprintBook ***************************************************
**
** Reports the internal data structures as a Docbook table
**
** @param [u] outf [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

void ajRefseqinprintBook(AjPFile outf)
{
    ajuint i = 0;
    ajuint j = 0;
    AjPStr namestr = NULL;
    AjPList fmtlist;
    AjPStr* names;

    fmtlist = ajListstrNew();

    ajFmtPrintF(outf, "<para>The supported reference sequence formats are "
                "summarised in the table below. "
                "The columns are as follows: "
                "<emphasis>Input format</emphasis> (format name), "
                "<emphasis>Try</emphasis> (indicates whether the "
                "format can be detected automatically on input), and "
                "<emphasis>Description</emphasis> (short description of "
                "the format).</para>\n\n");

    ajFmtPrintF(outf, "<table frame=\"box\" rules=\"cols\">\n");
    ajFmtPrintF(outf,
                "  <caption>Input reference sequence formats</caption>\n");
    ajFmtPrintF(outf, "  <thead>\n");
    ajFmtPrintF(outf, "    <tr align=\"center\">\n");
    ajFmtPrintF(outf, "      <th>Input Format</th>\n");
    ajFmtPrintF(outf, "      <th>Try</th>\n");
    ajFmtPrintF(outf, "      <th>Description</th>\n");
    ajFmtPrintF(outf, "    </tr>\n");
    ajFmtPrintF(outf, "  </thead>\n");
    ajFmtPrintF(outf, "  <tbody>\n");

    for(i=1; refseqinFormatDef[i].Name; i++)
    {
	if(!refseqinFormatDef[i].Alias)
        {
            namestr = ajStrNewC(refseqinFormatDef[i].Name);
            ajListPushAppend(fmtlist, namestr);
            namestr = NULL;
        }
    }

    ajListSort(fmtlist, ajStrVcmp);
    ajListstrToarray(fmtlist, &names);

    for(i=0; names[i]; i++)
    {
        for(j=0; refseqinFormatDef[j].Name; j++)
        {
            if(ajStrMatchC(names[i],refseqinFormatDef[j].Name))
            {
                ajFmtPrintF(outf, "    <tr>\n");
                ajFmtPrintF(outf, "      <td>%s</td>\n",
                            refseqinFormatDef[j].Name);
                ajFmtPrintF(outf, "      <td>%B</td>\n",
                            refseqinFormatDef[j].Try);
                ajFmtPrintF(outf, "      <td>%s</td>\n",
                            refseqinFormatDef[j].Desc);
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




/* @func ajRefseqinprintHtml ***************************************************
**
** Reports the internal data structures as an HTML table
**
** @param [u] outf [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

void ajRefseqinprintHtml(AjPFile outf)
{
    ajuint i = 0;
    ajuint j = 0;

    AjPStr namestr = NULL;

    ajFmtPrintF(outf, "<table border=3>");
    ajFmtPrintF(outf, "<tr><th>Input Format</th><th>Auto</th>\n");
    ajFmtPrintF(outf, "<th>Multi</th><th>Description</th></tr>\n");

    for(i=1; refseqinFormatDef[i].Name; i++)
    {
        ajStrAssignC(&namestr, refseqinFormatDef[i].Name);

	if(!refseqinFormatDef[i].Alias)
        {
            for(j=i+1; refseqinFormatDef[j].Name; j++)
            {
                if(refseqinFormatDef[j].Read == refseqinFormatDef[i].Read)
                {
                    ajFmtPrintAppS(&namestr, " %s",
                                   refseqinFormatDef[j].Name);
                    if(!refseqinFormatDef[j].Alias) 
                    {
                        ajWarn("Input format '%s' same as '%s' but not alias",
                               refseqinFormatDef[j].Name,
                               refseqinFormatDef[i].Name);
                    }
                }
            }

	    ajFmtPrintF(outf, "<tr><td>\n%S\n</td><td>%B</td>\n",
                        namestr,
			refseqinFormatDef[i].Try);
            ajFmtPrintF(outf, "<td>\n%s\n</td></tr>\n",
			refseqinFormatDef[i].Desc);
        }

    }

    ajFmtPrintF(outf, "</table>\n");
    ajStrDel(&namestr);

    return;
}




/* @func ajRefseqinprintText ***************************************************
**
** Reports the internal data structures
**
** @param [u] outf [AjPFile] Output file
** @param [r] full [AjBool] Full report (usually ajFalse)
** @return [void]
** @@
******************************************************************************/

void ajRefseqinprintText(AjPFile outf, AjBool full)
{
    ajuint i = 0;

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# Reference sequence input formats\n");
    ajFmtPrintF(outf, "# Name  Format name (or alias)\n");
    ajFmtPrintF(outf, "# Alias Alias name\n");
    ajFmtPrintF(outf, "# Try   Test for unknown input files\n");
    ajFmtPrintF(outf, "# Name         Alias Try "
		"Description");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "InFormat {\n");

    for(i=0; refseqinFormatDef[i].Name; i++)
	if(full || !refseqinFormatDef[i].Alias)
	    ajFmtPrintF(outf,
			"  %-12s %5B %3B \"%s\"\n",
			refseqinFormatDef[i].Name,
			refseqinFormatDef[i].Alias,
			refseqinFormatDef[i].Try,
			refseqinFormatDef[i].Desc);

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajRefseqinprintWiki ***************************************************
**
** Reports the internal data structures as a wiki table
**
** @param [u] outf [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

void ajRefseqinprintWiki(AjPFile outf)
{
    ajuint i = 0;
    ajuint j = 0;

    AjPStr namestr = NULL;

    ajFmtPrintF(outf, "{| class=\"wikitable sortable\" border=\"2\"\n");
    ajFmtPrintF(outf, "|-\n");
    ajFmtPrintF(outf, "!Format!!Try!!"
                "class=\"unsortable\"|Description\n");

    for(i=1; refseqinFormatDef[i].Name; i++)
    {
        ajStrAssignC(&namestr, refseqinFormatDef[i].Name);

	if(!refseqinFormatDef[i].Alias)
        {
            for(j=i+1; refseqinFormatDef[j].Name; j++)
            {
                if(refseqinFormatDef[j].Read == refseqinFormatDef[i].Read)
                {
                    ajFmtPrintAppS(&namestr, "<br>%s",
                                   refseqinFormatDef[j].Name);
                    if(!refseqinFormatDef[j].Alias) 
                    {
                        ajWarn("Input format '%s' same as '%s' but not alias",
                               refseqinFormatDef[j].Name,
                               refseqinFormatDef[i].Name);
                    }
                }
            }

            ajFmtPrintF(outf, "|-\n");
	    ajFmtPrintF(outf,
			"|%S||%B||%s\n",
			namestr,
			refseqinFormatDef[i].Try,
			refseqinFormatDef[i].Desc);
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




/* @func ajRefseqinExit ********************************************************
**
** Cleans up reference sequence input internal memory
**
** @return [void]
** @@
******************************************************************************/

void ajRefseqinExit(void)
{
    ajStrDel(&refseqinReadLine);

    ajTableDel(&refseqDbMethods);

    return;
}




/* @section Internals *********************************************************
**
** Functions to return internal values
**
** @fdata [none]
**
** @nam3rule Type Internals for refseq datatype
** @nam4rule Get  Return a value
** @nam5rule Fields  Known query fields for ajRefseqinRead
** @nam5rule Qlinks  Known query link operators for ajRefseqinRead
**
** @valrule * [const char*] Internal value
**
** @fcategory misc
**
******************************************************************************/




/* @func ajRefseqinTypeGetFields ***********************************************
**
** Returns the list of known field names for ajRefseqinRead
**
** @return [const char*] List of field names
** @@
******************************************************************************/

const char* ajRefseqinTypeGetFields(void)
{
    return "id acc";
}




/* @func ajRefseqinTypeGetQlinks ***********************************************
**
** Returns the listof known query link operators for ajRefseqinRead
**
** @return [const char*] List of field names
** @@
******************************************************************************/

const char* ajRefseqinTypeGetQlinks(void)
{
    return "|";
}




/* @datasection [AjPTable] Internal call register table ***********************
**
** Functions to manage the internal call register table that links the
** ajaxdb library functions with code in the core AJAX library.
**
** @nam2rule Refseqaccess Functions to manage refseqdb call tables.
**
******************************************************************************/




/* @section Cast **************************************************************
**
** Return a reference to the call table
**
** @fdata [AjPTable] refseqdb functions call table
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




/* @func ajRefseqaccessGetDb ***************************************************
**
** Returns the table in which reference sequence database access
** details are registered
**
** @return [AjPTable] Access functions hash table
** @@
******************************************************************************/

AjPTable ajRefseqaccessGetDb(void)
{
    if(!refseqDbMethods)
        refseqDbMethods = ajCallTableNew();
    return refseqDbMethods;
    
}




/* @func ajRefseqaccessMethodGetQlinks ****************************************
**
** Tests for a named method for refseq data reading returns the 
** known query link operators
**
** @param [r] method [const AjPStr] Method required.
** @return [const char*] Known link operators
** @@
******************************************************************************/

const char* ajRefseqaccessMethodGetQlinks(const AjPStr method)
{
    AjPRefseqAccess methoddata; 

    methoddata = ajCallTableGetS(refseqDbMethods, method);
    if(!methoddata)
        return NULL;

    return methoddata->Qlink;
}




/* @func ajRefseqaccessMethodGetScope ******************************************
**
** Tests for a named method for reference sequence data reading and
** returns the scope (entry, query or all).
*
** @param [r] method [const AjPStr] Method required.
** @return [ajuint] Scope flags
** @@
******************************************************************************/

ajuint ajRefseqaccessMethodGetScope(const AjPStr method)
{
    AjPRefseqAccess methoddata; 
    ajuint ret = 0;

    methoddata = ajCallTableGetS(refseqDbMethods, method);
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




/* @func ajRefseqaccessMethodTest **********************************************
**
** Tests for a named method for reference sequence data reading.
**
** @param [r] method [const AjPStr] Method required.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajRefseqaccessMethodTest(const AjPStr method)
{
    if(ajCallTableGetS(refseqDbMethods, method))
      return ajTrue;

    return ajFalse;
}




/* @funcstatic refseqinQryRestore **********************************************
**
** Restores a reference sequence input specification from an AjPQueryList node
**
** @param [w] refseqin [AjPRefseqin] Reference sequence input object
** @param [r] node [const AjPQueryList] Query list node
** @return [void]
******************************************************************************/

static void refseqinQryRestore(AjPRefseqin refseqin, const AjPQueryList node)
{
    refseqin->Input->Format = node->Format;
    refseqin->Input->Fpos   = node->Fpos;
    ajStrAssignS(&refseqin->Input->Formatstr, node->Formatstr);
    ajStrAssignS(&refseqin->Input->QryFields, node->QryFields);

    return;
}




/* @funcstatic refseqinQrySave *************************************************
**
** Saves a reference sequence input specification in an AjPQueryList node
**
** @param [w] node [AjPQueryList] Query list node
** @param [r] refseqin [const AjPRefseqin] Reference sequence input object
** @return [void]
******************************************************************************/

static void refseqinQrySave(AjPQueryList node, const AjPRefseqin refseqin)
{
    node->Format   = refseqin->Input->Format;
    node->Fpos     = refseqin->Input->Fpos;
    ajStrAssignS(&node->Formatstr, refseqin->Input->Formatstr);
    ajStrAssignS(&node->QryFields, refseqin->Input->QryFields);

    return;
}




/* @funcstatic refseqinQryProcess **********************************************
**
** Converts a reference sequence data query into an open file.
**
** Tests for "format::" and sets this if it is found
**
** Then tests for "list:" or "@" and processes as a list file
** using refseqinListProcess which in turn invokes refseqinQryProcess
** until a valid query is found.
**
** Then tests for dbname:query and opens the file (at the correct position
** if the database definition defines it)
**
** If there is no database, looks for file:query and opens the file.
** In this case the file position is not known and reference sequence
** data reading will have to scan for the entry/entries we need.
**
** @param [u] refseqin [AjPRefseqin] Reference sequence data input structure.
** @param [u] refseq [AjPRefseq] Reference sequence data to be read.
**                         The format will be replaced
**                         if defined in the query string.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool refseqinQryProcess(AjPRefseqin refseqin, AjPRefseq refseq)
{
    AjBool ret = ajTrue;
    AjPStr qrystr = NULL;
    AjBool refseqmethod = ajFalse;
    const AjPStr fmtstr = NULL;
    AjPTextin textin;
    AjPQuery qry;
    AjPRefseqAccess refseqaccess = NULL;

    textin = refseqin->Input;
    qry = textin->Query;

    /* pick up the original query string */
    qrystr = ajStrNewS(textin->Qry);

    ajDebug("refseqinQryProcess '%S'\n", qrystr);

    /* look for a format:: prefix */
    fmtstr = ajQuerystrParseFormat(&qrystr, textin, refseqinformatFind);
    ajDebug("refseqinQryProcess ... fmtstr '%S' '%S'\n", fmtstr, qrystr);

    /* (seq/feat) DO NOT look for a [range] suffix */

    /* look for a list:: or @:: listfile of queries  - process and return */
    if(ajQuerystrParseListfile(&qrystr))
    {
        ajDebug("refseqinQryProcess ... listfile '%S'\n", qrystr);
        ret = refseqinListProcess(refseqin, refseq, qrystr);
        ajStrDel(&qrystr);
        return ret;
    }

    /* try general text access methods (file, asis, text database access */
    ajDebug("refseqinQryProcess ... no listfile '%S'\n", qrystr);
    if(!ajQuerystrParseRead(&qrystr, textin, refseqinformatFind, &refseqmethod))
    {
        ajStrDel(&qrystr);
        return ajFalse;
    }
    
    refseqinFormatSet(refseqin, refseq);

    ajDebug("refseqinQryProcess ... read nontext: %B '%S'\n",
            refseqmethod, qrystr);
    ajStrDel(&qrystr);

    /* we found a non-text method */
    if(refseqmethod)
    {
        ajDebug("refseqinQryProcess ... call method '%S'\n", qry->Method);
        ajDebug("refseqinQryProcess ... textin format %d '%S'\n",
                textin->Format, textin->Formatstr);
        ajDebug("refseqinQryProcess ...  query format  '%S'\n",
                qry->Formatstr);
        qry->Access = ajCallTableGetS(refseqDbMethods,qry->Method);
        refseqaccess = qry->Access;
        return (*refseqaccess->Access)(refseqin);
    }

    ajDebug("refseqinQryProcess text method '%S' success\n", qry->Method);

    return ajTrue;
}





/* @datasection [AjPList] Query field list ************************************
**
** Query fields lists are handled internally. Only static functions
** should appear here
**
******************************************************************************/




/* @funcstatic refseqinListProcess *********************************************
**
** Processes a file of queries.
** This function is called by, and calls, refseqinQryProcess. There is
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
** @param [u] refseqin [AjPRefseqin] Reference sequence data input
** @param [u] refseq [AjPRefseq] Reference sequence data
** @param [r] listfile [const AjPStr] Name of list file.,
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool refseqinListProcess(AjPRefseqin refseqin, AjPRefseq refseq,
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
    ajDebug("++refseqinListProcess %S depth %d\n",
	    listfile, depth);

    if(depth > MAXDEPTH)
	ajFatal("Query list too deep");

    if(!refseqin->Input->List)
	refseqin->Input->List = ajListNew();

    list = ajListNew();

    file = ajFileNewInNameS(listfile);

    if(!file)
    {
	ajErr("Failed to open list file '%S'", listfile);
	depth--;

	return ret;
    }

    while(ajReadlineTrim(file, &refseqinReadLine))
    {
	refseqinListNoComment(&refseqinReadLine);

	if(ajStrGetLen(refseqinReadLine))
	{
	    ajStrTokenAssignC(&handle, refseqinReadLine, " \t\n\r");
	    ajStrTokenNextParse(&handle, &token);
	    /* ajDebug("Line  '%S'\n");*/
	    /* ajDebug("token '%S'\n", refseqinReadLine, token); */

	    if(ajStrGetLen(token))
	    {
	        ajDebug("++Add to list: '%S'\n", token);
	        AJNEW0(node);
	        ajStrAssignS(&node->Qry, token);
	        refseqinQrySave(node, refseqin);
	        ajListPushAppend(list, node);
	    }

	    ajStrDel(&token);
	    token = NULL;
	}
    }

    ajFileClose(&file);
    ajStrDel(&token);

    ajDebug("Trace refseqin->Input->List\n");
    ajQuerylistTrace(refseqin->Input->List);
    ajDebug("Trace new list\n");
    ajQuerylistTrace(list);
    ajListPushlist(refseqin->Input->List, &list);

    ajDebug("Trace combined refseqin->Input->List\n");
    ajQuerylistTrace(refseqin->Input->List);

    /*
     ** now try the first item on the list
     ** this can descend recursively if it is also a list
     ** which is why we check the depth above
     */

    if(ajListPop(refseqin->Input->List, (void**) &node))
    {
        ajDebug("++pop first item '%S'\n", node->Qry);
	ajRefseqinQryS(refseqin, node->Qry);
	refseqinQryRestore(refseqin, node);
	ajStrDel(&node->Qry);
	ajStrDel(&node->Formatstr);
	AJFREE(node);
	ajDebug("descending with query '%S'\n", refseqin->Input->Qry);
	ret = refseqinQryProcess(refseqin, refseq);
    }

    ajStrTokenDel(&handle);
    depth--;
    ajDebug("++refseqinListProcess depth: %d returns: %B\n", depth, ret);

    return ret;
}




/* @funcstatic refseqinListNoComment *******************************************
**
** Strips comments from a character string (a line from an ACD file).
** Comments are blank lines or any text following a "#" character.
**
** @param [u] text [AjPStr*] Line of text from input file.
** @return [void]
** @@
******************************************************************************/

static void refseqinListNoComment(AjPStr* text)
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




/* @funcstatic refseqinFormatSet ***********************************************
**
** Sets the input format for reference sequence data using the
** reference sequence data input object's defined format
**
** @param [u] refseqin [AjPRefseqin] Reference sequence data input.
** @param [u] refseq [AjPRefseq] Reference sequence data
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool refseqinFormatSet(AjPRefseqin refseqin, AjPRefseq refseq)
{

    if(ajStrGetLen(refseqin->Input->Formatstr))
    {
	ajDebug("... input format value '%S'\n",
                refseqin->Input->Formatstr);

	if(refseqinformatFind(refseqin->Input->Formatstr,
                             &refseqin->Input->Format))
	{
	    ajStrAssignS(&refseq->Formatstr,
                         refseqin->Input->Formatstr);
	    refseq->Format = refseqin->Input->Format;
	    ajDebug("...format OK '%S' = %d\n",
                    refseqin->Input->Formatstr,
		    refseqin->Input->Format);
	}
	else
	    ajDebug("...format unknown '%S'\n",
                    refseqin->Input->Formatstr);

	return ajTrue;
    }
    else
	ajDebug("...input format not set\n");


    return ajFalse;
}




/* @datasection [AjPRefseqall] Refseq Input Stream *****************************
**
** Function is for manipulating refseq input stream objects
**
** @nam2rule Refseqall Refseq input stream objects
**
******************************************************************************/




/* @section Refseq Input Constructors ******************************************
**
** All constructors return a new refseq input stream object by pointer. It
** is the responsibility of the user to first destroy any previous
** refseq input object. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so
** anyway.
**
** @fdata [AjPRefseqall]
**
** @nam3rule New Constructor
**
** @valrule * [AjPRefseqall] Refseq input stream object
**
** @fcategory new
**
******************************************************************************/




/* @func ajRefseqallNew ********************************************************
**
** Creates a new refseq input stream object.
**
** @return [AjPRefseqall] New refseq input stream object.
** @@
******************************************************************************/

AjPRefseqall ajRefseqallNew(void)
{
    AjPRefseqall pthis;

    AJNEW0(pthis);

    pthis->Refseqin = ajRefseqinNew();
    pthis->Refseq   = ajRefseqNew();

    return pthis;
}





/* ==================================================================== */
/* ========================== destructors ============================= */
/* ==================================================================== */




/* @section Refseq Input Stream Destructors ************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the refseq input stream object.
**
** @fdata [AjPRefseqall]
**
** @nam3rule Del Destructor
**
** @argrule Del pthis [AjPRefseqall*] Refseq input stream
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajRefseqallDel ********************************************************
**
** Deletes a refseq input stream object.
**
** @param [d] pthis [AjPRefseqall*] Refseq input stream
** @return [void]
** @@
******************************************************************************/

void ajRefseqallDel(AjPRefseqall* pthis)
{
    AjPRefseqall thys;

    if(!pthis)
        return;

    thys = *pthis;

    if(!thys)
        return;

    ajRefseqinDel(&thys->Refseqin);
    if(!thys->Returned)
        ajRefseqDel(&thys->Refseq);

    AJFREE(*pthis);

    return;
}




/* ==================================================================== */
/* =========================== Modifiers ============================== */
/* ==================================================================== */




/* @section refseq input stream modifiers **************************************
**
** These functions use the contents of a refseq input stream object and
** update them.
**
** @fdata [AjPRefseqall]
**
** @nam3rule Clear Clear all values
**
** @argrule * thys [AjPRefseqall] Refseq input stream object
**
** @valrule * [void]
**
** @fcategory modify
**
******************************************************************************/




/* @func ajRefseqallClear ******************************************************
**
** Clears a refseq input stream object back to "as new" condition, except
** for the query list which must be preserved.
**
** @param [w] thys [AjPRefseqall] Refseq input stream
** @return [void]
** @@
******************************************************************************/

void ajRefseqallClear(AjPRefseqall thys)
{

    ajDebug("ajRefseqallClear called\n");

    ajRefseqinClear(thys->Refseqin);

    ajRefseqClear(thys->Refseq);

    thys->Returned = ajFalse;

    return;
}




/* @section refseq input stream casts ******************************************
**
** These functions return the contents of a refseq input stream object
**
** @fdata [AjPRefseqall]
**
** @nam3rule Get Get refseq input stream values
** @nam3rule Getrefseq Get refseq values
** @nam4rule GetrefseqId Get refseq identifier
**
** @argrule * thys [const AjPRefseqall] Refseq input stream object
**
** @valrule * [const AjPStr] String value
**
** @fcategory cast
**
******************************************************************************/




/* @func ajRefseqallGetrefseqId ************************************************
**
** Returns the identifier of the current refseq in an input stream
**
** @param [r] thys [const AjPRefseqall] Refseq input stream
** @return [const AjPStr] Identifier
** @@
******************************************************************************/

const AjPStr ajRefseqallGetrefseqId(const AjPRefseqall thys)
{
    if(!thys)
        return NULL;

    ajDebug("ajRefseqallGetrefseqId called\n");

    return ajRefseqGetId(thys->Refseq);
}




/* @section refseq input *******************************************************
**
** These functions use a refseq input stream object to read data
**
** @fdata [AjPRefseqall]
**
** @nam3rule Next Read next refseq
**
** @argrule * thys [AjPRefseqall] Refseq input stream object
** @argrule * Prefseq [AjPRefseq*] Refseq object
**
** @valrule * [AjBool] True on success
**
** @fcategory input
**
******************************************************************************/




/* @func ajRefseqallNext *******************************************************
**
** Parse a refseq query into format, access, file and entry
**
** Split at delimiters. Check for the first part as a valid format
** Check for the remaining first part as a database name or as a file
** that can be opened.
** Anything left is an entryname spec.
**
** Return the results in the AjPRefseq object but leave the file open for
** future calls.
**
** @param [w] thys [AjPRefseqall] Refseq input stream
** @param [u] Prefseq [AjPRefseq*] Refseq returned
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajRefseqallNext(AjPRefseqall thys, AjPRefseq *Prefseq)
{
    ajDebug("ajRefseqallNext count:%u\n", thys->Count);

    if(!thys->Count)
    {
	thys->Count = 1;

	thys->Totterms++;

	*Prefseq = thys->Refseq;
	thys->Returned = ajTrue;

	return ajTrue;
    }


    if(ajRefseqinRead(thys->Refseqin, thys->Refseq))
    {
	thys->Count++;

	thys->Totterms++;

	*Prefseq = thys->Refseq;
	thys->Returned = ajTrue;

	ajDebug("ajRefseqallNext success\n");

	return ajTrue;
    }

    *Prefseq = NULL;

    ajDebug("ajRefseqallNext failed\n");

    ajRefseqallClear(thys);

    return ajFalse;
}




/* @datasection [none] Input formats ******************************************
**
** Input formats internals
**
** @nam2rule Refseqinformat Reference sequence data input format specific
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




/* @funcstatic refseqinformatFind **********************************************
**
** Looks for the specified format(s) in the internal definitions and
** returns the index.
**
** Sets iformat as the recognised format, and returns ajTrue.
**
** @param [r] format [const AjPStr] Format required.
** @param [w] iformat [ajint*] Index
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool refseqinformatFind(const AjPStr format, ajint* iformat)
{
    AjPStr tmpformat = NULL;
    ajuint i = 0;

    /* ajDebug("refseqinformatFind '%S'\n", format); */
    if(!ajStrGetLen(format))
	return ajFalse;

    ajStrAssignS(&tmpformat, format);
    ajStrFmtLower(&tmpformat);

    for(i=0; refseqinFormatDef[i].Name; i++)
    {
	/* ajDebug("test %d '%s' \n", i, refseqinFormatDef[i].Name); */
	if(ajStrMatchCaseC(tmpformat, refseqinFormatDef[i].Name))
	{
	    *iformat = i;
	    ajStrDel(&tmpformat);
	    /* ajDebug("found '%s' at %d\n", refseqinFormatDef[i].Name, i); */
	    return ajTrue;
	}
    }

    ajErr("Unknown input format '%S'", format);

    ajStrDel(&tmpformat);

    return ajFalse;
}




/* @func ajRefseqinformatTerm **************************************************
**
** Tests whether a reference sequence data input format term is known
**
** @param [r] term [const AjPStr] Format term EDAM ID
** @return [AjBool] ajTrue if term was accepted
** @@
******************************************************************************/

AjBool ajRefseqinformatTerm(const AjPStr term)
{
    ajuint i;

    for(i=0; refseqinFormatDef[i].Name; i++)
	if(ajStrMatchCaseC(term, refseqinFormatDef[i].Obo))
	    return ajTrue;

    return ajFalse;
}




/* @func ajRefseqinformatTest **************************************************
**
** Tests whether a named reference sequence data input format is known
**
** @param [r] format [const AjPStr] Format
** @return [AjBool] ajTrue if term was accepted
** @@
******************************************************************************/

AjBool ajRefseqinformatTest(const AjPStr format)
{
    ajuint i;

    for(i=0; refseqinFormatDef[i].Name; i++)
	if(ajStrMatchCaseC(format, refseqinFormatDef[i].Name))
	    return ajTrue;

    return ajFalse;
}

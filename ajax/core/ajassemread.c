/*
** This is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU Library General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
******************************************************************************/

#include "ajax.h"

AjPTable assemDbMethods = NULL;

static AjPStr asseminReadLine = NULL;

static AjBool asseminReadAbc(AjPAssemin thys, AjPAssem assem);




/* @datastatic AssemPInFormat *************************************************
**
** Assembly input formats data structure
**
** @alias AssemSInFormat
** @alias AssemOInFormat
**
** @attr Name [const char*] Format name
** @attr Obo  [const char*] Ontology term id from EDAM
** @attr Desc [const char*] Format description
** @attr Alias [AjBool] Name is an alias for an identical definition
** @attr Try [AjBool] If true, try for an unknown input. Duplicate names
**                    and read-anything formats are set false
** @attr Read [(AjBool*)] Input function, returns ajTrue on success
** @@
******************************************************************************/

typedef struct AssemSInFormat
{
    const char *Name;
    const char *Obo;
    const char *Desc;
    AjBool Alias;
    AjBool Try;
    AjBool (*Read) (AjPAssemin thys, AjPAssem assem);
} AssemOInFormat;

#define AssemPInFormat AssemOInFormat*

static AssemOInFormat asseminFormatDef[] =
{
/* "Name",        "Description" */
/*     Alias,   Try,     */
/*     ReadFunction */
  {"unknown",     "0000000", "Unknown format",
       AJFALSE, AJFALSE,
       asseminReadAbc}, /* default to first format */
  {"abc",          "0000000", "Abc format",
       AJFALSE, AJTRUE,
       asseminReadAbc},
  {NULL, NULL, NULL, 0, 0, NULL}
};



static ajuint asseminReadFmt(AjPAssemin assemin, AjPAssem assem,
                           ajuint format);
static AjBool asseminRead(AjPAssemin assemin, AjPAssem assem);
static AjBool asseminformatFind(const AjPStr format, ajint* iformat);
static AjBool asseminFormatSet(AjPAssemin assemin, AjPAssem assem);
static AjBool asseminListProcess(AjPAssemin assemin, AjPAssem assem,
                               const AjPStr listfile);
static void asseminListNoComment(AjPStr* text);
static void asseminQryRestore(AjPAssemin assemin, const AjPQueryList node);
static void asseminQrySave(AjPQueryList node, const AjPAssemin assemin);
static AjBool assemDefine(AjPAssem thys, AjPAssemin assemin);
static AjBool asseminQryProcess(AjPAssemin assemin, AjPAssem assem);
static AjBool asseminQueryMatch(const AjPQuery thys, const AjPAssem assem);




/* @filesection ajassemread **************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
*/




/* @datasection [AjPAssemin] Assembly input objects ***************************
**
** Function is for manipulating assembly input objects
**
** @nam2rule Assemin
******************************************************************************/




/* @section Assembly Input Constructors ***************************************
**
** All constructors return a new assembly input object by pointer. It
** is the responsibility of the user to first destroy any previous
** assembly input object. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so
** anyway.
**
** @fdata [AjPAssemin]
**
** @nam3rule  New     Construct a new assembly input object
**
** @valrule   *  [AjPAssemin] New assembly input object
**
** @fcategory new
**
******************************************************************************/




/* @func ajAsseminNew *********************************************************
**
** Creates a new assembly input object.
**
** @return [AjPAssemin] New assembly input object.
** @category new [AjPAssemin] Default constructor
** @@
******************************************************************************/

AjPAssemin ajAsseminNew(void)
{
    AjPAssemin pthis;

    AJNEW0(pthis);

    pthis->Input = ajTextinNewDatatype(AJDATATYPE_ASSEMBLY);

    pthis->AssemData      = NULL;

    return pthis;
}





/* @section Assembly Input Destructors ****************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the assembly input object.
**
** @fdata [AjPAssemin]
**
** @nam3rule Del Destructor
**
** @argrule Del pthis [AjPAssemin*] Assembly input
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajAsseminDel *********************************************************
**
** Deletes an assembly input object.
**
** @param [d] pthis [AjPAssemin*] Assembly input
** @return [void]
** @category delete [AjPAssemin] Default destructor
** @@
******************************************************************************/

void ajAsseminDel(AjPAssemin* pthis)
{
    AjPAssemin thys;

    if(!pthis)
        return;

    thys = *pthis;

    if(!thys)
        return;

    ajDebug("ajAsseminDel called qry:'%S'\n", thys->Input->Qry);

    ajTextinDel(&thys->Input);

    AJFREE(*pthis);

    return;
}




/* @section assembly input modifiers ******************************************
**
** These functions use the contents of an assembly input object and
** update them.
**
** @fdata [AjPAssemin]
**
** @nam3rule Clear Clear all values
** @nam3rule Qry Reset using a query string
** @suffix C Character string input
** @suffix S String input
**
** @argrule * thys [AjPAssemin] Assembly input object
** @argrule C txt [const char*] Query text
** @argrule S str [const AjPStr] query string
**
** @valrule * [void]
**
** @fcategory modify
**
******************************************************************************/




/* @func ajAsseminClear *******************************************************
**
** Clears an assembly input object back to "as new" condition, except
** for the query list which must be preserved.
**
** @param [w] thys [AjPAssemin] Assembly input
** @return [void]
** @category modify [AjPAssemin] Resets ready for reuse.
** @@
******************************************************************************/

void ajAsseminClear(AjPAssemin thys)
{

    ajDebug("ajAsseminClear called\n");

    if(!thys)
        return;

    ajTextinClear(thys->Input);

    thys->AssemData = NULL;

    return;
}




/* @func ajAsseminQryC ********************************************************
**
** Resets an assembly input object using a new Universal
** Query Address
**
** @param [u] thys [AjPAssemin] Assembly input object.
** @param [r] txt [const char*] Query
** @return [void]
** @@
******************************************************************************/

void ajAsseminQryC(AjPAssemin thys, const char* txt)
{
    ajAsseminClear(thys);
    ajStrAssignC(&thys->Input->Qry, txt);

    return;
}





/* @func ajAsseminQryS ********************************************************
**
** Resets an assembly input object using a new Universal
** Query Address
**
** @param [u] thys [AjPAssemin] Assembly input object.
** @param [r] str [const AjPStr] Query
** @return [void]
** @@
******************************************************************************/

void ajAsseminQryS(AjPAssemin thys, const AjPStr str)
{
    ajAsseminClear(thys);
    ajStrAssignS(&thys->Input->Qry, str);

    return;
}




/* @section casts *************************************************************
**
** Return values
**
** @fdata [AjPAssemin]
**
** @nam3rule Trace Write debugging output
**
** @argrule * thys [const AjPAssemin] Assembly input object
**
** @valrule * [void]
**
** @fcategory cast
**
******************************************************************************/




/* @func ajAsseminTrace *******************************************************
**
** Debug calls to trace the data in an assembly input object.
**
** @param [r] thys [const AjPAssemin] Assembly input object.
** @return [void]
** @@
******************************************************************************/

void ajAsseminTrace(const AjPAssemin thys)
{
    ajDebug("assembly input trace\n");
    ajDebug("====================\n\n");

    ajTextinTrace(thys->Input);

    if(thys->AssemData)
	ajDebug( "  AssemData: exists\n");

    return;
}




/* @section Assembly data inputs **********************************************
**
** These functions read the assembly data provided by the first argument
**
** @fdata [AjPAssemin]
**
** @nam3rule Read Read assembly data
**
** @argrule Read assemin [AjPAssemin] Assembly input object
** @argrule Read assem [AjPAssem] Assembly data
**
** @valrule * [AjBool] true on success
**
** @fcategory input
**
******************************************************************************/




/* @func ajAsseminRead ********************************************************
**
** If the file is not yet open, calls asseminQryProcess to convert the query
** into an open file stream.
**
** Uses asseminRead for the actual file reading.
**
** Returns the results in the AjPAssem object.
**
** @param [u] assemin [AjPAssemin] assembly data input definitions
** @param [w] assem [AjPAssem] assembly data returned.
** @return [AjBool] ajTrue on success.
** @category input [AjPAssem] Master assembly data input,
**                  calls specific functions for file access type
**                  and assembly data format.
** @@
******************************************************************************/

AjBool ajAsseminRead(AjPAssemin assemin, AjPAssem assem)
{
    AjBool ret       = ajFalse;
    AjPQueryList node = NULL;
    AjBool listdata  = ajFalse;

    if(assemin->Input->Filebuff)
    {
	/* (a) if file still open, keep reading */
	ajDebug("ajAsseminRead: input file '%F' still there, try again\n",
		assemin->Input->Filebuff->File);
	ret = asseminRead(assemin, assem);
	ajDebug("ajAsseminRead: open buffer  qry: '%S' returns: %B\n",
		assemin->Input->Qry, ret);
    }
    else
    {
	/* (b) if we have a list, try the next query in the list */
	if(ajListGetLength(assemin->Input->List))
	{
	    listdata = ajTrue;
	    ajListPop(assemin->Input->List, (void**) &node);

	    ajDebug("++pop from list '%S'\n", node->Qry);
	    ajAsseminQryS(assemin, node->Qry);
	    ajDebug("++SAVE ASSEMIN '%S' '%S' %d\n",
		    assemin->Input->Qry,
		    assemin->Input->Formatstr, assemin->Input->Format);

            asseminQryRestore(assemin, node);

	    ajStrDel(&node->Qry);
	    ajStrDel(&node->Formatstr);
	    AJFREE(node);

	    ajDebug("ajAsseminRead: open list, try '%S'\n",
                    assemin->Input->Qry);

	    if(!asseminQryProcess(assemin, assem) &&
               !ajListGetLength(assemin->Input->List))
		return ajFalse;

	    ret = asseminRead(assemin, assem);
	    ajDebug("ajAsseminRead: list qry: '%S' returns: %B\n",
		    assemin->Input->Qry, ret);
	}
	else
	{
	    ajDebug("ajAsseminRead: no file yet - test query '%S'\n",
                    assemin->Input->Qry);

	    /* (c) Must be a query - decode it */
	    if(!asseminQryProcess(assemin, assem) &&
               !ajListGetLength(assemin->Input->List))
		return ajFalse;

	    if(ajListGetLength(assemin->Input->List)) /* could be a new list */
		listdata = ajTrue;

	    ret = asseminRead(assemin, assem);
	    ajDebug("ajAsseminRead: new qry: '%S' returns: %B\n",
		    assemin->Input->Qry, ret);
	}
    }

    /* Now read whatever we got */

    while(!ret && ajListGetLength(assemin->Input->List))
    {
	/* Failed, but we have a list still - keep trying it */
        if(listdata)
	    ajErr("Failed to read assembly data '%S'",
                  assemin->Input->Qry);

	listdata = ajTrue;
	ajListPop(assemin->Input->List,(void**) &node);
	ajDebug("++try again: pop from list '%S'\n", node->Qry);
	ajAsseminQryS(assemin, node->Qry);
	ajDebug("++SAVE (AGAIN) ASSEMIN '%S' '%S' %d\n",
		assemin->Input->Qry,
		assemin->Input->Formatstr, assemin->Input->Format);

	asseminQryRestore(assemin, node);

	ajStrDel(&node->Qry);
	ajStrDel(&node->Formatstr);
	AJFREE(node);

	if(!asseminQryProcess(assemin, assem))
	    continue;

	ret = asseminRead(assemin, assem);
	ajDebug("ajAsseminRead: list retry qry: '%S' returns: %B\n",
		assemin->Input->Qry, ret);
    }

    if(!ret)
    {
	if(listdata)
	    ajErr("Failed to read assembly data '%S'",
                  assemin->Input->Qry);

	return ajFalse;
    }


    assemDefine(assem, assemin);

    return ajTrue;
}




/* @funcstatic asseminQueryMatch **********************************************
**
** Compares an assembly data item to a query and returns true if they match.
**
** @param [r] thys [const AjPQuery] query.
** @param [r] assem [const AjPAssem] Assembly data.
** @return [AjBool] ajTrue if the assembly data matches the query.
** @@
******************************************************************************/

static AjBool asseminQueryMatch(const AjPQuery thys, const AjPAssem assem)
{
    AjBool tested = ajFalse;
    AjIList iterfield  = NULL;
    AjPQueryField field = NULL;
    AjBool ok = ajFalse;

    ajDebug("asseminQueryMatch '%S' fields: %u Case %B Done %B\n",
	    assem->Id, ajListGetLength(thys->QueryFields),
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
                    assem->Id);
            if(thys->CaseId)
            {
                if(ajStrMatchWildS(assem->Id, field->Wildquery))
                {
                    ajListIterDel(&iterfield);
                    return ajTrue;
                }
            }
            else
            {
                if(ajStrMatchWildCaseS(assem->Id, field->Wildquery))
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
            if(ajStrMatchWildCaseS(assem->Id, field->Wildquery))
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




/* @funcstatic assemDefine ****************************************************
**
** Make sure all assembly data object attributes are defined
** using values from the assembly input object if needed
**
** @param [w] thys [AjPAssem] Assembly data returned.
** @param [u] assemin [AjPAssemin] Assembly data input definitions
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool assemDefine(AjPAssem thys, AjPAssemin assemin)
{

    /* if values are missing in the assembly object, we can use defaults
       from assemin or calculate where possible */

    /* assign the dbname if defined in the assemin object */
    if(ajStrGetLen(assemin->Input->Db))
      ajStrAssignS(&thys->Db, assemin->Input->Db);

    return ajTrue;
}





/* @funcstatic asseminReadFmt *************************************************
**
** Tests whether assembly data can be read using the specified format.
** Then tests whether the assembly data matches assembly data query criteria
** and checks any specified type. Applies upper and lower case.
**
** @param [u] assemin [AjPAssemin] Assembly data input object
** @param [w] assem [AjPAssem] Assembly data object
** @param [r] format [ajuint] input format code
** @return [ajuint] 0 if successful.
**                  1 if the query match failed.
**                  2 if the assembly data type failed
**                  3 if it failed to read a assembly data
** @@
** This is the only function that calls the appropriate Read function
** asseminReadXxxxxx where Xxxxxxx is the supported assembly data format.
**
** Some of the assemReadXxxxxx functions fail to reset the buffer correctly,
** which is a very serious problem when cycling through all of them to
** identify an unknown format. The extra ajFileBuffReset call at the end is
** intended to address this problem. The individual functions should still
** reset the buffer in case they are called from elsewhere.
**
******************************************************************************/

static ajuint asseminReadFmt(AjPAssemin assemin, AjPAssem assem,
                           ajuint format)
{
    ajDebug("++asseminReadFmt format %d (%s) '%S'\n",
	    format, asseminFormatDef[format].Name,
	    assemin->Input->Qry);

    assemin->Input->Records = 0;

    /* Calling funclist asseminFormatDef() */
    if(asseminFormatDef[format].Read(assemin, assem))
    {
	ajDebug("asseminReadFmt success with format %d (%s)\n",
		format, asseminFormatDef[format].Name);
        ajDebug("id: '%S'\n",
                assem->Id);
	assemin->Input->Format = format;
	ajStrAssignC(&assemin->Input->Formatstr, asseminFormatDef[format].Name);
	ajStrAssignC(&assem->Formatstr, asseminFormatDef[format].Name);
	ajStrAssignEmptyS(&assem->Db, assemin->Input->Db);
	ajStrAssignS(&assem->Filename, assemin->Input->Filename);

	if(asseminQueryMatch(assemin->Input->Query, assem))
	{
            /* ajAsseminTrace(assemin); */

            return FMT_OK;
        }

	ajDebug("query match failed, continuing ...\n");
	ajAssemClear(assem);

	return FMT_NOMATCH;
    }
    else
    {
	ajDebug("Testing input buffer: IsBuff: %B Eof: %B\n",
		ajFilebuffIsBuffered(assemin->Input->Filebuff),
		ajFilebuffIsEof(assemin->Input->Filebuff));

	if (!ajFilebuffIsBuffered(assemin->Input->Filebuff) &&
	    ajFilebuffIsEof(assemin->Input->Filebuff))
	    return FMT_EOF;

	ajFilebuffReset(assemin->Input->Filebuff);
	ajDebug("Format %d (%s) failed, file buffer reset by asseminReadFmt\n",
		format, asseminFormatDef[format].Name);
	/* ajFilebuffTraceFull(assemin->Filebuff, 10, 10);*/
    }

    ajDebug("++asseminReadFmt failed - nothing read\n");

    return FMT_FAIL;
}




/* @funcstatic asseminRead ****************************************************
**
** Given data in a assemin structure, tries to read everything needed
** using the specified format or by trial and error.
**
** @param [u] assemin [AjPAssemin] Assembly data input object
** @param [w] assem [AjPAssem] Assembly data object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool asseminRead(AjPAssemin assemin, AjPAssem assem)
{
    ajuint i;
    ajuint istat = 0;
    ajuint jstat = 0;

    AjPFilebuff buff = assemin->Input->Filebuff;
/*    AjBool regfile = ajFalse; */
    AjBool ok;

    AjPTextAccess  textaccess  = assemin->Input->Query->TextAccess;
    AjPAssemAccess assemaccess = assemin->Input->Query->Access;

    ajAssemClear(assem);
    ajDebug("asseminRead: cleared\n");

    if(assemin->Input->Single && assemin->Input->Count)
    {
	/*
	** One assembly data item at a time is read.
	** The first assembly data item was read by ACD
	** for the following ones we need to reset the AjPAssemin
	**
	** Single is set by the access method
	*/

	ajDebug("asseminRead: single access - count %d - call access"
		" routine again\n",
		assemin->Input->Count);
	/* Calling funclist asseminAccess() */
	if(textaccess)
        {
            if(!textaccess->Access(assemin->Input))
            {
                ajDebug("asseminRead: textaccess->Access(assemin->Input) "
                        "*failed*\n");

                return ajFalse;
            }
        }

	if(assemaccess)
        {
            if(!assemaccess->Access(assemin))
            {
                ajDebug("asseminRead: assemaccess->Access(assemin) "
                        "*failed*\n");

                return ajFalse;
            }
        }

        buff = assemin->Input->Filebuff;
    }

    ajDebug("asseminRead: assemin format %d '%S'\n", assemin->Input->Format,
	    assemin->Input->Formatstr);

    assemin->Input->Count++;

    if(!assemin->Input->Filebuff)
	return ajFalse;

    ok = ajFilebuffIsBuffered(assemin->Input->Filebuff);

    while(ok)
    {				/* skip blank lines */
        ok = ajBuffreadLine(assemin->Input->Filebuff, &asseminReadLine);

        if(!ajStrIsWhite(asseminReadLine))
        {
            ajFilebuffClear(assemin->Input->Filebuff,1);
            break;
        }
    }

    if(!assemin->Input->Format)
    {			   /* no format specified, try all defaults */

        /*regfile = ajFileIsFile(ajFilebuffGetFile(assemin->Input->Filebuff));*/

	for(i = 1; asseminFormatDef[i].Name; i++)
	{
	    if(!asseminFormatDef[i].Try)	/* skip if Try is ajFalse */
		continue;

	    ajDebug("asseminRead:try format %d (%s)\n",
		    i, asseminFormatDef[i].Name);

	    istat = asseminReadFmt(assemin, assem, i);

	    switch(istat)
	    {
	    case FMT_OK:
		ajDebug("++asseminRead OK, set format %d\n",
                        assemin->Input->Format);
		assemDefine(assem, assemin);

		return ajTrue;
	    case FMT_BADTYPE:
		ajDebug("asseminRead: (a1) "
                        "asseminReadFmt stat == BADTYPE *failed*\n");

		return ajFalse;
	    case FMT_FAIL:
		ajDebug("asseminRead: (b1) "
                        "asseminReadFmt stat == FAIL *failed*\n");
		break;			/* we can try next format */
	    case FMT_NOMATCH:
		ajDebug("asseminRead: (c1) "
                        "asseminReadFmt stat==NOMATCH try again\n");
		break;
	    case FMT_EOF:
		ajDebug("asseminRead: (d1) "
                        "asseminReadFmt stat == EOF *failed*\n");
		return ajFalse;			/* EOF and unbuffered */
	    case FMT_EMPTY:
		ajWarn("assembly data '%S' has zero length, ignored",
		       ajAssemGetQryS(assem));
		ajDebug("asseminRead: (e1) "
                        "asseminReadFmt stat==EMPTY try again\n");
		break;
	    default:
		ajDebug("unknown code %d from asseminReadFmt\n", stat);
	    }

	    ajAssemClear(assem);

	    if(assemin->Input->Format)
		break;			/* we read something */

            ajFilebuffTrace(assemin->Input->Filebuff);
	}

	if(!assemin->Input->Format)
	{		     /* all default formats failed, give up */
	    ajDebug("asseminRead:all default formats failed, give up\n");

	    return ajFalse;
	}

	ajDebug("++asseminRead set format %d\n",
                assemin->Input->Format);
    }
    else
    {					/* one format specified */
	ajDebug("asseminRead: one format specified\n");
	ajFilebuffSetUnbuffered(assemin->Input->Filebuff);

	ajDebug("++asseminRead known format %d\n",
                assemin->Input->Format);
	istat = asseminReadFmt(assemin, assem, assemin->Input->Format);

	switch(istat)
	{
	case FMT_OK:
	    assemDefine(assem, assemin);

	    return ajTrue;
	case FMT_BADTYPE:
	    ajDebug("asseminRead: (a2) "
                    "asseminReadFmt stat == BADTYPE *failed*\n");

	    return ajFalse;

        case FMT_FAIL:
	    ajDebug("asseminRead: (b2) "
                    "asseminReadFmt stat == FAIL *failed*\n");

	    return ajFalse;

        case FMT_NOMATCH:
	    ajDebug("asseminRead: (c2) "
                    "asseminReadFmt stat == NOMATCH *try again*\n");
	    break;
	case FMT_EOF:
	    ajDebug("asseminRead: (d2) "
                    "asseminReadFmt stat == EOF *try again*\n");
            if(assemin->Input->Records)
                ajErr("Error reading file '%F' with format '%s': "
                      "end-of-file before end of data "
                      "(read %u records)",
                      ajFilebuffGetFile(assemin->Input->Filebuff),
                      asseminFormatDef[assemin->Input->Format].Name,
                      assemin->Input->Records);
	    break;		     /* simply end-of-file */
	case FMT_EMPTY:
	    ajWarn("assmebly data '%S' has zero length, ignored",
		   ajAssemGetQryS(assem));
	    ajDebug("asseminRead: (e2) "
                    "asseminReadFmt stat == EMPTY *try again*\n");
	    break;
	default:
	    ajDebug("unknown code %d from asseminReadFmt\n", stat);
	}

	ajAssemClear(assem); /* 1 : read, failed to match id/acc/query */
    }

    /* failed - probably entry/accession query failed. Can we try again? */

    ajDebug("asseminRead failed - try again with format %d '%s' code %d\n",
	    assemin->Input->Format,
            asseminFormatDef[assemin->Input->Format].Name, istat);

    ajDebug("Search:%B Chunk:%B Data:%x ajFileBuffEmpty:%B\n",
	    assemin->Input->Search, assemin->Input->ChunkEntries,
            assemin->Input->TextData, ajFilebuffIsEmpty(buff));

    if(ajFilebuffIsEmpty(buff) && assemin->Input->ChunkEntries)
    {
	if(textaccess && !textaccess->Access(assemin->Input))
            return ajFalse;
	else if(assemaccess && !assemaccess->Access(assemin))
            return ajFalse;
        buff = assemin->Input->Filebuff;
    }


    /* need to check end-of-file to avoid repeats */
    while(assemin->Input->Search &&
          (assemin->Input->TextData || !ajFilebuffIsEmpty(buff)))
    {
	jstat = asseminReadFmt(assemin, assem, assemin->Input->Format);

	switch(jstat)
	{
	case FMT_OK:
	    assemDefine(assem, assemin);

	    return ajTrue;

        case FMT_BADTYPE:
	    ajDebug("asseminRead: (a3) "
                    "asseminReadFmt stat == BADTYPE *failed*\n");

	    return ajFalse;

        case FMT_FAIL:
	    ajDebug("asseminRead: (b3) "
                    "asseminReadFmt stat == FAIL *failed*\n");

	    return ajFalse;
            
	case FMT_NOMATCH:
	    ajDebug("asseminRead: (c3) "
                    "asseminReadFmt stat == NOMATCH *try again*\n");
	    break;
	case FMT_EOF:
	    ajDebug("asseminRead: (d3) "
                    "asseminReadFmt stat == EOF *failed*\n");

	    return ajFalse;			/* we already tried again */

        case FMT_EMPTY:
	    if(istat != FMT_EMPTY)
                ajWarn("assmebly data '%S' has zero length, ignored",
                       ajAssemGetQryS(assem));
	    ajDebug("asseminRead: (e3) "
                    "asseminReadFmt stat == EMPTY *try again*\n");
	    break;

        default:
	    ajDebug("unknown code %d from asseminReadFmt\n", stat);
	}

	ajAssemClear(assem); /* 1 : read, failed to match id/acc/query */
    }

    if(assemin->Input->Format)
	ajDebug("asseminRead: *failed* to read assembly data %S "
                "using format %s\n",
		assemin->Input->Qry,
                asseminFormatDef[assemin->Input->Format].Name);
    else
	ajDebug("asseminRead: *failed* to read assembly data %S "
                "using any format\n",
		assemin->Input->Qry);

    return ajFalse;
}




/* @funcstatic asseminReadAbc *************************************************
**
** Given data in an assembly structure, tries to read everything needed
** using Abc format.
**
** @param [u] assemin [AjPAssemin] Assembly input object
** @param [w] assem [AjPAssem] Assembly object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool asseminReadAbc(AjPAssemin assemin, AjPAssem assem)
{
    AjPFilebuff buff;

/*    AjPFile textfile; */
    ajlong fpos     = 0;
    ajuint linecnt = 0;

    ajDebug("asseminReadAbc\n");
    ajAssemClear(assem);
    buff = assemin->Input->Filebuff;
/*    textfile = ajFilebuffGetFile(buff); */

    /* ajFilebuffTrace(buff); */

    while (ajBuffreadLinePos(buff, &asseminReadLine, &fpos))
    {
        linecnt++;
        if(ajStrGetCharLast(asseminReadLine) == '\n')
            ajStrCutEnd(&asseminReadLine, 1);
        if(ajStrGetCharLast(asseminReadLine) == '\r')
            ajStrCutEnd(&asseminReadLine, 1);

        ajDebug("line %u:%S\n", linecnt, asseminReadLine);

        /* add line to AjPAssem object */
    }

    return ajTrue;
}




/* @datasection [none] Miscellaneous ******************************************
**
** Assembly input internals
**
** @nam2rule Assemin Assembly input
**
******************************************************************************/




/* @section Printing **********************************************************
**
** Printing details of the internals to a file
**
** @fdata [none]
**
** @nam2rule Asseminprint
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




/* @func ajAsseminprintBook ***************************************************
**
** Reports the internal data structures as a Docbook table
**
** @param [u] outf [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

void ajAsseminprintBook(AjPFile outf)
{
    ajuint i = 0;
    ajuint j = 0;
    AjPStr namestr = NULL;
    AjPList fmtlist;
    AjPStr* names;

    fmtlist = ajListstrNew();

    ajFmtPrintF(outf, "<para>The supported assembly formats are summarised "
                "in the table below. "
                "The columns are as follows: "
                "<emphasis>Input format</emphasis> (format name), "
                "<emphasis>Try</emphasis> (indicates whether the "
                "format can be detected automatically on input), and "
                "<emphasis>Description</emphasis> (short description of "
                "the format).</para>\n\n");

    ajFmtPrintF(outf, "<table frame=\"box\" rules=\"cols\">\n");
    ajFmtPrintF(outf, "  <caption>Input assembly formats</caption>\n");
    ajFmtPrintF(outf, "  <thead>\n");
    ajFmtPrintF(outf, "    <tr align=\"center\">\n");
    ajFmtPrintF(outf, "      <th>Input Format</th>\n");
    ajFmtPrintF(outf, "      <th>Try</th>\n");
    ajFmtPrintF(outf, "      <th>Description</th>\n");
    ajFmtPrintF(outf, "    </tr>\n");
    ajFmtPrintF(outf, "  </thead>\n");
    ajFmtPrintF(outf, "  <tbody>\n");

    for(i=1; asseminFormatDef[i].Name; i++)
    {
	if(!asseminFormatDef[i].Alias)
        {
            namestr = ajStrNewC(asseminFormatDef[i].Name);
            ajListPushAppend(fmtlist, namestr);
            namestr = NULL;
        }
    }

    ajListSort(fmtlist, ajStrVcmp);
    ajListstrToarray(fmtlist, &names);

    for(i=0; names[i]; i++)
    {
        for(j=0; asseminFormatDef[j].Name; j++)
        {
            if(ajStrMatchC(names[i],asseminFormatDef[j].Name))
            {
                ajFmtPrintF(outf, "    <tr>\n");
                ajFmtPrintF(outf, "      <td>%s</td>\n",
                            asseminFormatDef[j].Name);
                ajFmtPrintF(outf, "      <td>%B</td>\n",
                            asseminFormatDef[j].Try);
                ajFmtPrintF(outf, "      <td>%s</td>\n",
                            asseminFormatDef[j].Desc);
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




/* @func ajAsseminprintHtml ***************************************************
**
** Reports the internal data structures as an HTML table
**
** @param [u] outf [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

void ajAsseminprintHtml(AjPFile outf)
{
    ajuint i = 0;
    ajuint j = 0;

    AjPStr namestr = NULL;

    ajFmtPrintF(outf, "<table border=3>");
    ajFmtPrintF(outf, "<tr><th>Input Format</th><th>Auto</th>\n");
    ajFmtPrintF(outf, "<th>Multi</th><th>Description</th></tr>\n");

    for(i=1; asseminFormatDef[i].Name; i++)
    {
        ajStrAssignC(&namestr, asseminFormatDef[i].Name);

	if(!asseminFormatDef[i].Alias)
        {
            for(j=i+1; asseminFormatDef[j].Name; j++)
            {
                if(asseminFormatDef[j].Read == asseminFormatDef[i].Read)
                {
                    ajFmtPrintAppS(&namestr, " %s", asseminFormatDef[j].Name);
                    if(!asseminFormatDef[j].Alias) 
                    {
                        ajWarn("Input format '%s' same as '%s' but not alias",
                               asseminFormatDef[j].Name,
                               asseminFormatDef[i].Name);
                    }
                }
            }

	    ajFmtPrintF(outf, "<tr><td>\n%S\n</td><td>%B</td>\n",
                        namestr,
			asseminFormatDef[i].Try);
            ajFmtPrintF(outf, "<td>\n%s\n</td></tr>\n",
			asseminFormatDef[i].Desc);
        }

    }

    ajFmtPrintF(outf, "</table>\n");
    ajStrDel(&namestr);

    return;
}




/* @func ajAsseminprintText ***************************************************
**
** Reports the internal data structures
**
** @param [u] outf [AjPFile] Output file
** @param [r] full [AjBool] Full report (usually ajFalse)
** @return [void]
** @@
******************************************************************************/

void ajAsseminprintText(AjPFile outf, AjBool full)
{
    ajuint i = 0;

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# Assembly input formats\n");
    ajFmtPrintF(outf, "# Name  Format name (or alias)\n");
    ajFmtPrintF(outf, "# Alias Alias name\n");
    ajFmtPrintF(outf, "# Try   Test for unknown input files\n");
    ajFmtPrintF(outf, "# Name         Alias Try "
		"Description");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "InFormat {\n");

    for(i=0; asseminFormatDef[i].Name; i++)
	if(full || !asseminFormatDef[i].Alias)
	    ajFmtPrintF(outf,
			"  %-12s %5B %3B \"%s\"\n",
			asseminFormatDef[i].Name,
			asseminFormatDef[i].Alias,
			asseminFormatDef[i].Try,
			asseminFormatDef[i].Desc);

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajAsseminprintWiki ***************************************************
**
** Reports the internal data structures as a wiki table
**
** @param [u] outf [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

void ajAsseminprintWiki(AjPFile outf)
{
    ajuint i = 0;
    ajuint j = 0;

    AjPStr namestr = NULL;

    ajFmtPrintF(outf, "{| class=\"wikitable sortable\" border=\"2\"\n");
    ajFmtPrintF(outf, "|-\n");
    ajFmtPrintF(outf, "!Format!!Try!!"
                "class=\"unsortable\"|Description\n");

    for(i=1; asseminFormatDef[i].Name; i++)
    {
        ajStrAssignC(&namestr, asseminFormatDef[i].Name);

	if(!asseminFormatDef[i].Alias)
        {
            for(j=i+1; asseminFormatDef[j].Name; j++)
            {
                if(asseminFormatDef[j].Read == asseminFormatDef[i].Read)
                {
                    ajFmtPrintAppS(&namestr, "<br>%s",
                                   asseminFormatDef[j].Name);
                    if(!asseminFormatDef[j].Alias) 
                    {
                        ajWarn("Input format '%s' same as '%s' but not alias",
                               asseminFormatDef[j].Name,
                               asseminFormatDef[i].Name);
                    }
                }
            }

            ajFmtPrintF(outf, "|-\n");
	    ajFmtPrintF(outf,
			"|%S||%B||%s\n",
			namestr,
			asseminFormatDef[i].Try,
			asseminFormatDef[i].Desc);
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




/* @func ajAsseminExit ********************************************************
**
** Cleans up assembly input internal memory
**
** @return [void]
** @@
******************************************************************************/

void ajAsseminExit(void)
{
    /* Query processing regular expressions */

    ajTableDel(&assemDbMethods);

    ajStrDel(&asseminReadLine);

    return;
}




/* @section Internals *********************************************************
**
** Functions to return internal values
**
** @fdata [none]
**
** @nam3rule Type Internals for assembly datatype
** @nam4rule Get  Return a value
** @nam5rule Fields  Known query fields for ajAsseminRead
** @nam5rule Qlinks  Known query link operators for ajAsseminRead
**
** @valrule * [const char*] Internal value
**
** @fcategory misc
**
******************************************************************************/




/* @func ajAsseminTypeGetFields ***********************************************
**
** Returns the listof known field names for ajAsseminRead
**
** @return [const char*] List of field names
** @@
******************************************************************************/

const char* ajAsseminTypeGetFields(void)
{
    return "id acc";
}




/* @func ajAsseminTypeGetQlinks **********************************************
**
** Returns the listof known query link operators for ajAsseminRead
**
** @return [const char*] List of field names
** @@
******************************************************************************/

const char* ajAsseminTypeGetQlinks(void)
{
    return "";
}




/* @datasection [AjPTable] Internal call register table ***********************
**
** Functions to manage the internal call register table that links the
** ajaxdb library functions with code in the core AJAX library.
**
** @nam2rule Assemaccess Functions to manage assemdb call tables.
**
******************************************************************************/




/* @section Cast **************************************************************
**
** Return a reference to the call table
**
** @fdata [AjPTable] assemdb functions call table
**
** @nam3rule Get Return a value
** @nam4rule Db Database access functions table
** @nam3rule Method Lookup an access method by name
** @nam4rule Test Return true if the access method exists
** @nam4rule MethodGet Return a method value
** @nam5rule Qlinks Return query link operators for a named method
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




/* @func ajAssemaccessGetDb ***************************************************
**
** Returns the table in which assembly database access details are registered
**
** @return [AjPTable] Access functions hash table
** @@
******************************************************************************/

AjPTable ajAssemaccessGetDb(void)
{
    if(!assemDbMethods)
        assemDbMethods = ajCallTableNew();
    return assemDbMethods;
    
}




/* @func ajAssemaccessMethodGetQlinks *****************************************
**
** Tests for a named method for assembly data reading returns the 
** known query link operators
**
** @param [r] method [const AjPStr] Method required.
** @return [const char*] Known link operators
** @@
******************************************************************************/

const char* ajAssemaccessMethodGetQlinks(const AjPStr method)
{
    const AjPAssemAccess methoddata; 

    methoddata = ajCallTableGetS(assemDbMethods, method);
    if(!methoddata)
        return NULL;

    return methoddata->Qlink;
}




/* @func ajAssemaccessMethodGetScope ******************************************
**
** Tests for a named method for assembly data reading and returns the scope
** (entry, query or all).
*
** @param [r] method [const AjPStr] Method required.
** @return [ajuint] Scope flags
** @@
******************************************************************************/

ajuint ajAssemaccessMethodGetScope(const AjPStr method)
{
    const AjPAssemAccess methoddata; 
    ajuint ret = 0;

    methoddata = ajCallTableGetS(assemDbMethods, method);
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




/* @func ajAssemaccessMethodTest **********************************************
**
** Tests for a named method for assembly data reading.
**
** @param [r] method [const AjPStr] Method required.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajAssemaccessMethodTest(const AjPStr method)
{
    if(ajCallTableGetS(assemDbMethods, method))
      return ajTrue;

    return ajFalse;
}




/* @funcstatic asseminQryRestore **********************************************
**
** Restores an assembly input specification from an AjPQueryList node
**
** @param [w] assemin [AjPAssemin] Assembly input object
** @param [r] node [const AjPQueryList] Query list node
** @return [void]
******************************************************************************/

static void asseminQryRestore(AjPAssemin assemin, const AjPQueryList node)
{
    assemin->Input->Format = node->Format;
    ajStrAssignS(&assemin->Input->Formatstr, node->Formatstr);

    return;
}




/* @funcstatic asseminQrySave *************************************************
**
** Saves an assembly input specification in an AjPQueryList node
**
** @param [w] node [AjPQueryList] Query list node
** @param [r] assemin [const AjPAssemin] Assembly input object
** @return [void]
******************************************************************************/

static void asseminQrySave(AjPQueryList node, const AjPAssemin assemin)
{
    node->Format   = assemin->Input->Format;
    ajStrAssignS(&node->Formatstr, assemin->Input->Formatstr);

    return;
}




/* @funcstatic asseminQryProcess **********************************************
**
** Converts an assembly data query into an open file.
**
** Tests for "format::" and sets this if it is found
**
** Then tests for "list:" or "@" and processes as a list file
** using asseminListProcess which in turn invokes asseminQryProcess
** until a valid query is found.
**
** Then tests for dbname:query and opens the file (at the correct position
** if the database definition defines it)
**
** If there is no database, looks for file:query and opens the file.
** In this case the file position is not known and assembly data reading
** will have to scan for the entry/entries we need.
**
** @param [u] assemin [AjPAssemin] Assembly data input structure.
** @param [u] assem [AjPAssem] Assembly data to be read.
**                         The format will be replaced
**                         if defined in the query string.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool asseminQryProcess(AjPAssemin assemin, AjPAssem assem)
{
    AjBool ret = ajTrue;
    AjPStr qrystr = NULL;
    AjBool assemmethod = ajFalse;
    const AjPStr fmtstr = NULL;
    AjPTextin textin;
    AjPQuery qry;
    AjPAssemAccess assemaccess = NULL;

    textin = assemin->Input;
    qry = textin->Query;

    /* pick up the original query string */
    qrystr = ajStrNewS(textin->Qry);

    ajDebug("asseminQryProcess '%S'\n", qrystr);

    /* look for a format:: prefix */
    fmtstr = ajQuerystrParseFormat(&qrystr, textin, asseminformatFind);
    ajDebug("asseminQryProcess ... fmtstr '%S' '%S'\n", fmtstr, qrystr);

    /* (seq/feat) DO NOT look for a [range] suffix */

    /* look for a list:: or @:: listfile of queries  - process and return */
    if(ajQuerystrParseListfile(&qrystr))
    {
        ajDebug("asseminQryProcess ... listfile '%S'\n", qrystr);
        ret = asseminListProcess(assemin, assem, qrystr);
        ajStrDel(&qrystr);
        return ret;
    }

    /* try general text access methods (file, asis, text database access */
    ajDebug("asseminQryProcess ... no listfile '%S'\n", qrystr);
    if(!ajQuerystrParseRead(&qrystr, textin, asseminformatFind, &assemmethod))
    {
        ajStrDel(&qrystr);
        return ajFalse;
    }
    
    asseminFormatSet(assemin, assem);

    ajDebug("asseminQryProcess ... read nontext: %B '%S'\n",
            assemmethod, qrystr);
    ajStrDel(&qrystr);

    /* we found a non-text method */
    if(assemmethod)
    {
        ajDebug("asseminQryProcess ... call method '%S'\n", qry->Method);
        ajDebug("asseminQryProcess ... textin format %d '%S'\n",
                textin->Format, textin->Formatstr);
        ajDebug("asseminQryProcess ...  query format  '%S'\n",
                qry->Formatstr);
        qry->Access = ajCallTableGetS(assemDbMethods,qry->Method);
        assemaccess = qry->Access;
        return assemaccess->Access(assemin);
    }

    ajDebug("asseminQryProcess text method '%S' success\n", qry->Method);

    return ajTrue;
}





/* @datasection [AjPList] Query field list ************************************
**
** Query fields lists are handled internally. Only static functions
** should appear here
**
******************************************************************************/




/* @funcstatic asseminListProcess *********************************************
**
** Processes a file of queries.
** This function is called by, and calls, asseminQryProcess. There is
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
** @param [u] assemin [AjPAssemin] Assembly data input
** @param [u] assem [AjPAssem] Assembly data
** @param [r] listfile [const AjPStr] Name of list file.,
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool asseminListProcess(AjPAssemin assemin, AjPAssem assem,
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
    ajDebug("++asseminListProcess %S depth %d\n",
	    listfile, depth);

    if(depth > MAXDEPTH)
	ajFatal("Query list too deep");

    if(!assemin->Input->List)
	assemin->Input->List = ajListNew();

    list = ajListNew();

    file = ajFileNewInNameS(listfile);

    if(!file)
    {
	ajErr("Failed to open list file '%S'", listfile);
	depth--;

	return ret;
    }

    while(ajReadlineTrim(file, &asseminReadLine))
    {
	asseminListNoComment(&asseminReadLine);

	if(ajStrGetLen(asseminReadLine))
	{
	    ajStrTokenAssignC(&handle, asseminReadLine, " \t\n\r");
	    ajStrTokenNextParse(&handle, &token);
	    /* ajDebug("Line  '%S'\n");*/
	    /* ajDebug("token '%S'\n", asseminReadLine, token); */

	    if(ajStrGetLen(token))
	    {
	        ajDebug("++Add to list: '%S'\n", token);
	        AJNEW0(node);
	        ajStrAssignS(&node->Qry, token);
	        asseminQrySave(node, assemin);
	        ajListPushAppend(list, node);
	    }

	    ajStrDel(&token);
	    token = NULL;
	}
    }

    ajFileClose(&file);
    ajStrDel(&token);

    ajDebug("Trace assemin->Input->List\n");
    ajQuerylistTrace(assemin->Input->List);
    ajDebug("Trace new list\n");
    ajQuerylistTrace(list);
    ajListPushlist(assemin->Input->List, &list);

    ajDebug("Trace combined assemin->Input->List\n");
    ajQuerylistTrace(assemin->Input->List);

    /*
     ** now try the first item on the list
     ** this can descend recursively if it is also a list
     ** which is why we check the depth above
     */

    if(ajListPop(assemin->Input->List, (void**) &node))
    {
        ajDebug("++pop first item '%S'\n", node->Qry);
	ajAsseminQryS(assemin, node->Qry);
	asseminQryRestore(assemin, node);
	ajStrDel(&node->Qry);
	ajStrDel(&node->Formatstr);
	AJFREE(node);
	ajDebug("descending with query '%S'\n", assemin->Input->Qry);
	ret = asseminQryProcess(assemin, assem);
    }

    ajStrTokenDel(&handle);
    depth--;
    ajDebug("++asseminListProcess depth: %d returns: %B\n", depth, ret);

    return ret;
}




/* @funcstatic asseminListNoComment *******************************************
**
** Strips comments from a character string (a line from an ACD file).
** Comments are blank lines or any text following a "#" character.
**
** @param [u] text [AjPStr*] Line of text from input file.
** @return [void]
** @@
******************************************************************************/

static void asseminListNoComment(AjPStr* text)
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




/* @funcstatic asseminFormatSet ***********************************************
**
** Sets the input format for assembly data using the assembly data
** input object's defined format
**
** @param [u] assemin [AjPAssemin] Assembly data input.
** @param [u] assem [AjPAssem] Assembly data
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool asseminFormatSet(AjPAssemin assemin, AjPAssem assem)
{

    if(ajStrGetLen(assemin->Input->Formatstr))
    {
	ajDebug("... input format value '%S'\n",
                assemin->Input->Formatstr);

	if(asseminformatFind(assemin->Input->Formatstr,
                             &assemin->Input->Format))
	{
	    ajStrAssignS(&assem->Formatstr,
                         assemin->Input->Formatstr);
	    assem->Format = assemin->Input->Format;
	    ajDebug("...format OK '%S' = %d\n",
                    assemin->Input->Formatstr,
		    assemin->Input->Format);
	}
	else
	    ajDebug("...format unknown '%S'\n",
                    assemin->Input->Formatstr);

	return ajTrue;
    }
    else
	ajDebug("...input format not set\n");


    return ajFalse;
}




/* @datasection [none] Input formats ******************************************
**
** Input formats internals
**
** @nam2rule Asseminformat Assembly data input format specific
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




/* @funcstatic asseminformatFind **********************************************
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

static AjBool asseminformatFind(const AjPStr format, ajint* iformat)
{
    AjPStr tmpformat = NULL;
    ajuint i = 0;

    /* ajDebug("asseminformatFind '%S'\n", format); */
    if(!ajStrGetLen(format))
	return ajFalse;

    ajStrAssignS(&tmpformat, format);
    ajStrFmtLower(&tmpformat);

    for(i=0; asseminFormatDef[i].Name; i++)
    {
	/* ajDebug("test %d '%s' \n", i, asseminFormatDef[i].Name); */
	if(ajStrMatchCaseC(tmpformat, asseminFormatDef[i].Name))
	{
	    *iformat = i;
	    ajStrDel(&tmpformat);
	    /* ajDebug("found '%s' at %d\n", asseminFormatDef[i].Name, i); */
	    return ajTrue;
	}
    }

    ajErr("Unknown input format '%S'", format);

    ajStrDel(&tmpformat);

    return ajFalse;
}




/* @func ajAsseminformatTerm **************************************************
**
** Tests whether a assembly data input format term is known
**
** @param [r] term [const AjPStr] Format term EDAM ID
** @return [AjBool] ajTrue if formats was accepted
** @@
******************************************************************************/

AjBool ajAsseminformatTerm(const AjPStr term)
{
    ajuint i;

    for(i=0; asseminFormatDef[i].Name; i++)
	if(ajStrMatchCaseC(term, asseminFormatDef[i].Obo))
	    return ajTrue;

    return ajFalse;
}




/* @func ajAsseminformatTest **************************************************
**
** Tests whether a named assembly data input format is known
**
** @param [r] format [const AjPStr] Format
** @return [AjBool] ajTrue if format was accepted
** @@
******************************************************************************/

AjBool ajAsseminformatTest(const AjPStr format)
{
    ajuint i;

    for(i=0; asseminFormatDef[i].Name; i++)
	if(ajStrMatchCaseC(format, asseminFormatDef[i].Name))
	    return ajTrue;

    return ajFalse;
}

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

AjPTable varDbMethods = NULL;

static AjPStr varinReadLine     = NULL;

static AjBool varinReadAbc(AjPVarin thys, AjPVar var);




/* @datastatic VarPInFormat *************************************************
**
** Variation input formats data structure
**
** @alias VarSInFormat
** @alias VarOInFormat
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

typedef struct VarSInFormat
{
    const char *Name;
    const char *Obo;
    const char *Desc;
    AjBool Alias;
    AjBool Try;
    AjBool (*Read) (AjPVarin thys, AjPVar var);
} VarOInFormat;

#define VarPInFormat VarOInFormat*

static VarOInFormat varinFormatDef[] =
{
/* "Name",        "OBOterm", "Description" */
/*     Alias,   Try,     */
/*     ReadFunction */
  {"unknown",     "0000000", "Unknown format",
       AJFALSE, AJFALSE,
       varinReadAbc}, /* default to first format */
  {"abc",          "0000000", "Abc format",
       AJFALSE, AJTRUE,
       varinReadAbc},
  {NULL, NULL, NULL, 0, 0, NULL}
};



static ajuint varinReadFmt(AjPVarin varin, AjPVar var,
                           ajuint format);
static AjBool varinRead(AjPVarin varin, AjPVar var);
static AjBool varinformatFind(const AjPStr format, ajint* iformat);
static AjBool varinFormatSet(AjPVarin varin, AjPVar var);
static AjBool varinListProcess(AjPVarin varin, AjPVar var,
                               const AjPStr listfile);
static void varinListNoComment(AjPStr* text);
static void varinQryRestore(AjPVarin varin, const AjPQueryList node);
static void varinQrySave(AjPQueryList node, const AjPVarin varin);
static AjBool varDefine(AjPVar thys, AjPVarin varin);
static AjBool varinQryProcess(AjPVarin varin, AjPVar var);
static AjBool varinQueryMatch(const AjPQuery thys, const AjPVar var);




/* @filesection ajvarread ****************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
*/




/* @datasection [AjPVarin] Variation input objects ***************************
**
** Function is for manipulating variation input objects
**
** @nam2rule Varin
******************************************************************************/




/* @section Variation input constructors ***************************************
**
** All constructors return a new variation input object by pointer. It
** is the responsibility of the user to first destroy any previous
** variation input object. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so
** anyway.
**
** @fdata [AjPVarin]
**
** @nam3rule  New     Construct a new variation input object
**
** @valrule   *  [AjPVarin] New variation input object
**
** @fcategory new
**
******************************************************************************/




/* @func ajVarinNew **********************************************************
**
** Creates a new variation input object.
**
** @return [AjPVarin] New variation input object.
** @category new [AjPVarin] Default constructor
** @@
******************************************************************************/

AjPVarin ajVarinNew(void)
{
    AjPVarin pthis;

    AJNEW0(pthis);

    pthis->Input = ajTextinNew();

    pthis->VarData      = NULL;

    return pthis;
}





/* @section variation input destructors ****************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the variation input object.
**
** @fdata [AjPVarin]
**
** @nam3rule Del Destructor
**
** @argrule Del pthis [AjPVarin*] Variation input object
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajVarinDel **********************************************************
**
** Deletes a variation input object.
**
** @param [d] pthis [AjPVarin*] Variation input
** @return [void]
** @category delete [AjPVarin] Default destructor
** @@
******************************************************************************/

void ajVarinDel(AjPVarin* pthis)
{
    AjPVarin thys;

    if(!pthis)
        return;

    thys = *pthis;

    if(!thys)
        return;

    ajDebug("ajVarinDel called qry:'%S'\n", thys->Input->Qry);

    ajTextinDel(&thys->Input);

    AJFREE(*pthis);

    return;
}




/* @section variation input modifiers ******************************************
**
** These functions use the contents of a variation input object and
** update them.
**
** @fdata [AjPVarin]
**
** @nam3rule Clear Clear all values
** @nam3rule Qry Reset using a query string
** @suffix C Character string input
** @suffix S String input
**
** @argrule * thys [AjPVarin] Variation input object
** @argrule C txt [const char*] Query text
** @argrule S str [const AjPStr] query string
**
** @valrule * [void]
**
** @fcategory modify
**
******************************************************************************/




/* @func ajVarinClear ********************************************************
**
** Clears a variation input object back to "as new" condition, except
** for the query list which must be preserved.
**
** @param [w] thys [AjPVarin] Variation input
** @return [void]
** @category modify [AjPVarin] Resets ready for reuse.
** @@
******************************************************************************/

void ajVarinClear(AjPVarin thys)
{

    ajDebug("ajVarinClear called\n");

    ajTextinClear(thys->Input);

    thys->VarData = NULL;

    return;
}




/* @func ajVarinQryC *********************************************************
**
** Resets a variation input object using a new Universal
** Query Address
**
** @param [u] thys [AjPVarin] Variation input object.
** @param [r] txt [const char*] Query
** @return [void]
** @@
******************************************************************************/

void ajVarinQryC(AjPVarin thys, const char* txt)
{
    ajVarinClear(thys);
    ajStrAssignC(&thys->Input->Qry, txt);

    return;
}





/* @func ajVarinQryS ********************************************************
**
** Resets a variation input object using a new Universal
** Query Address
**
** @param [u] thys [AjPVarin] Variation input object.
** @param [r] str [const AjPStr] Query
** @return [void]
** @@
******************************************************************************/

void ajVarinQryS(AjPVarin thys, const AjPStr str)
{
    ajVarinClear(thys);
    ajStrAssignS(&thys->Input->Qry, str);

    return;
}




/* @section casts *************************************************************
**
** Return values
**
** @fdata [AjPVarin]
**
** @nam3rule Trace Write debugging output
**
** @argrule * thys [const AjPVarin] Variation input object
**
** @valrule * [void]
**
** @fcategory cast
**
******************************************************************************/




/* @func ajVarinTrace ********************************************************
**
** Debug calls to trace the data in a variation input object.
**
** @param [r] thys [const AjPVarin] Variation input object.
** @return [void]
** @@
******************************************************************************/

void ajVarinTrace(const AjPVarin thys)
{
    ajDebug("variation input trace\n");
    ajDebug("====================\n\n");

    ajTextinTrace(thys->Input);

    if(thys->VarData)
	ajDebug( "  VarData: exists\n");

    return;
}




/* @section Variation data inputs **********************************************
**
** These functions read the wxyxdesc data provided by the first argument
**
** @fdata [AjPVarin]
**
** @nam3rule Read Read variation data
**
** @argrule Read varin [AjPVarin] Variation input object
** @argrule Read var [AjPVar] Variation data
**
** @valrule * [AjBool] true on success
**
** @fcategory input
**
******************************************************************************/




/* @func ajVarinRead ********************************************************
**
** If the file is not yet open, calls varinQryProcess to convert the query
** into an open file stream.
**
** Uses varinRead for the actual file reading.
**
** Returns the results in the AjPVar object.
**
** @param [u] varin [AjPVarin] Variation data input definitions
** @param [w] var [AjPVar] Variation data returned.
** @return [AjBool] ajTrue on success.
** @category input [AjPVar] Master variation data input,
**                  calls specific functions for file access type
**                  and variation data format.
** @@
******************************************************************************/

AjBool ajVarinRead(AjPVarin varin, AjPVar var)
{
    AjBool ret       = ajFalse;
    AjPQueryList node = NULL;
    AjBool listdata  = ajFalse;

    if(varin->Input->Filebuff)
    {
	/* (a) if file still open, keep reading */
	ajDebug("ajVarinRead: input file '%F' still there, try again\n",
		varin->Input->Filebuff->File);
	ret = varinRead(varin, var);
	ajDebug("ajVarinRead: open buffer  qry: '%S' returns: %B\n",
		varin->Input->Qry, ret);
    }
    else
    {
	/* (b) if we have a list, try the next query in the list */
	if(ajListGetLength(varin->Input->List))
	{
	    listdata = ajTrue;
	    ajListPop(varin->Input->List, (void**) &node);

	    ajDebug("++pop from list '%S'\n", node->Qry);
	    ajVarinQryS(varin, node->Qry);
	    ajDebug("++SAVE WXYZIN '%S' '%S' %d\n",
		    varin->Input->Qry,
		    varin->Input->Formatstr, varin->Input->Format);

            varinQryRestore(varin, node);

	    ajStrDel(&node->Qry);
	    ajStrDel(&node->Formatstr);
	    AJFREE(node);

	    ajDebug("ajVarinRead: open list, try '%S'\n",
                    varin->Input->Qry);

	    if(!varinQryProcess(varin, var) &&
               !ajListGetLength(varin->Input->List))
		return ajFalse;

	    ret = varinRead(varin, var);
	    ajDebug("ajVarinRead: list qry: '%S' returns: %B\n",
		    varin->Input->Qry, ret);
	}
	else
	{
	    ajDebug("ajVarinRead: no file yet - test query '%S'\n",
                    varin->Input->Qry);

	    /* (c) Must be a query - decode it */
	    if(!varinQryProcess(varin, var) &&
               !ajListGetLength(varin->Input->List))
		return ajFalse;

	    if(ajListGetLength(varin->Input->List)) /* could be a new list */
		listdata = ajTrue;

	    ret = varinRead(varin, var);
	    ajDebug("ajVarinRead: new qry: '%S' returns: %B\n",
		    varin->Input->Qry, ret);
	}
    }

    /* Now read whatever we got */

    while(!ret && ajListGetLength(varin->Input->List))
    {
	/* Failed, but we have a list still - keep trying it */
        if(listdata)
	    ajErr("Failed to read variation data '%S'",
                  varin->Input->Qry);

	listdata = ajTrue;
	ajListPop(varin->Input->List,(void**) &node);
	ajDebug("++try again: pop from list '%S'\n", node->Qry);
	ajVarinQryS(varin, node->Qry);
	ajDebug("++SAVE (AGAIN) WXYZIN '%S' '%S' %d\n",
		varin->Input->Qry,
		varin->Input->Formatstr, varin->Input->Format);

	varinQryRestore(varin, node);

	ajStrDel(&node->Qry);
	ajStrDel(&node->Formatstr);
	AJFREE(node);

	if(!varinQryProcess(varin, var))
	    continue;

	ret = varinRead(varin, var);
	ajDebug("ajVarinRead: list retry qry: '%S' returns: %B\n",
		varin->Input->Qry, ret);
    }

    if(!ret)
    {
	if(listdata)
	    ajErr("Failed to read variation data '%S'",
                  varin->Input->Qry);

	return ajFalse;
    }


    varDefine(var, varin);

    return ajTrue;
}




/* @funcstatic varinQueryMatch ***********************************************
**
** Compares a variation data item to a query and returns true if they match.
**
** @param [r] thys [const AjPQuery] query.
** @param [r] var [const AjPVar] Variation data.
** @return [AjBool] ajTrue if the variation data matches the query.
** @@
******************************************************************************/

static AjBool varinQueryMatch(const AjPQuery thys, const AjPVar var)
{
    AjBool tested = ajFalse;
    AjIList iterfield  = NULL;
    AjPQueryField field = NULL;
    AjBool ok = ajFalse;

    ajDebug("varinQueryMatch '%S' fields: %u Case %B Done %B\n",
	    var->Id, ajListGetLength(thys->QueryFields),
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
                    var->Id);
            if(thys->CaseId)
            {
                if(ajStrMatchWildS(var->Id, field->Wildquery))
                {
                    ajListIterDel(&iterfield);
                    return ajTrue;
                }
            }
            else
            {
                if(ajStrMatchWildCaseS(var->Id, field->Wildquery))
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
            if(ajStrMatchWildCaseS(var->Id, field->Wildquery))
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




/* @funcstatic varDefine ****************************************************
**
** Make sure all variation data object attributes are defined
** using values from the variation input object if needed
**
** @param [w] thys [AjPVar] Variation data returned.
** @param [u] varin [AjPVarin] Variation data input definitions
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool varDefine(AjPVar thys, AjPVarin varin)
{

    /* if values are missing in the variation object, we can use defaults
       from varin or calculate where possible */

    /* assign the dbname if defined in the varin object */
    if(ajStrGetLen(varin->Input->Db))
      ajStrAssignS(&thys->Db, varin->Input->Db);

    return ajTrue;
}





/* @funcstatic varinReadFmt *************************************************
**
** Tests whether variation data can be read using the specified format.
** Then tests whether the variation data matches variation data query criteria
** and checks any specified type. Applies upper and lower case.
**
** @param [u] varin [AjPVarin] Variation data input object
** @param [w] var [AjPVar] Variation data object
** @param [r] format [ajuint] input format code
** @return [ajuint] 0 if successful.
**                  1 if the query match failed.
**                  2 if the variation data type failed
**                  3 if it failed to read any variation data
** @@
** This is the only function that calls the appropriate Read function
** varinReadXxxxxx where Xxxxxxx is the supported variation data format.
**
** Some of the varReadXxxxxx functions fail to reset the buffer correctly,
** which is a very serious problem when cycling through all of them to
** identify an unknown format. The extra ajFileBuffReset call at the end is
** intended to address this problem. The individual functions should still
** reset the buffer in case they are called from elsewhere.
**
******************************************************************************/

static ajuint varinReadFmt(AjPVarin varin, AjPVar var,
                           ajuint format)
{
    ajDebug("++varinReadFmt format %d (%s) '%S'\n",
	    format, varinFormatDef[format].Name,
	    varin->Input->Qry);

    varin->Input->Records = 0;

    /* Calling funclist varinFormatDef() */
    if(varinFormatDef[format].Read(varin, var))
    {
	ajDebug("varinReadFmt success with format %d (%s)\n",
		format, varinFormatDef[format].Name);
        ajDebug("id: '%S'\n",
                var->Id);
	varin->Input->Format = format;
	ajStrAssignC(&varin->Input->Formatstr,
	             varinFormatDef[format].Name);
	ajStrAssignC(&var->Formatstr,
	             varinFormatDef[format].Name);
	ajStrAssignEmptyS(&var->Db, varin->Input->Db);
	ajStrAssignS(&var->Filename, varin->Input->Filename);

	if(varinQueryMatch(varin->Input->Query, var))
	{
            /* ajVarinTrace(varin); */

            return FMT_OK;
        }

	ajDebug("query match failed, continuing ...\n");
	ajVarClear(var);

	return FMT_NOMATCH;
    }
    else
    {
	ajDebug("Testing input buffer: IsBuff: %B Eof: %B\n",
		ajFilebuffIsBuffered(varin->Input->Filebuff),
		ajFilebuffIsEof(varin->Input->Filebuff));

	if (!ajFilebuffIsBuffered(varin->Input->Filebuff) &&
	    ajFilebuffIsEof(varin->Input->Filebuff))
	    return FMT_EOF;

	ajFilebuffReset(varin->Input->Filebuff);
	ajDebug("Format %d (%s) failed, file buffer reset by varinReadFmt\n",
		format, varinFormatDef[format].Name);
	/* ajFilebuffTraceFull(varin->Filebuff, 10, 10);*/
    }

    ajDebug("++varinReadFmt failed - nothing read\n");

    return FMT_FAIL;
}




/* @funcstatic varinRead *****************************************************
**
** Given data in a varin structure, tries to read everything needed
** using the specified format or by trial and error.
**
** @param [u] varin [AjPVarin] Variation data input object
** @param [w] var [AjPVar] Variation data object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool varinRead(AjPVarin varin, AjPVar var)
{
    ajuint i;
    ajuint istat = 0;
    ajuint jstat = 0;

    AjPFilebuff buff = varin->Input->Filebuff;
    AjBool ok;

    AjPTextAccess  textaccess  = varin->Input->Query->TextAccess;
    AjPVarAccess varaccess = varin->Input->Query->Access;

    ajVarClear(var);
    ajDebug("varinRead: cleared\n");

    if(varin->Input->Single && varin->Input->Count)
    {
	/*
	** One variation data item at a time is read.
	** The first variation data item was read by ACD
	** for the following ones we need to reset the AjPVarin
	**
	** Single is set by the access method
	*/

	ajDebug("varinRead: single access - count %d - call access"
		" routine again\n",
		varin->Input->Count);
	/* Calling funclist varinAccess() */
	if(textaccess)
        {
            if(!textaccess->Access(varin->Input))
            {
                ajDebug("varinRead: textaccess->Access(varin->Input) "
                        "*failed*\n");

                return ajFalse;
            }
        }

	if(varaccess)
        {
            if(!varaccess->Access(varin))
            {
                ajDebug("varinRead: varaccess->Access(varin) "
                        "*failed*\n");

                return ajFalse;
            }
        }

        buff = varin->Input->Filebuff;
    }

    ajDebug("varinRead: varin format %d '%S'\n", varin->Input->Format,
	    varin->Input->Formatstr);

    varin->Input->Count++;

    if(!varin->Input->Filebuff)
	return ajFalse;

    ok = ajFilebuffIsBuffered(varin->Input->Filebuff);

    while(ok)
    {				/* skip blank lines */
        ok = ajBuffreadLine(varin->Input->Filebuff, &varinReadLine);

        if(!ajStrIsWhite(varinReadLine))
        {
            ajFilebuffClear(varin->Input->Filebuff,1);
            break;
        }
    }

    if(!varin->Input->Format)
    {			   /* no format specified, try all defaults */

	for(i = 1; varinFormatDef[i].Name; i++)
	{
	    if(!varinFormatDef[i].Try)	/* skip if Try is ajFalse */
		continue;

	    ajDebug("varinRead:try format %d (%s)\n",
		    i, varinFormatDef[i].Name);

	    istat = varinReadFmt(varin, var, i);

	    switch(istat)
	    {
	    case FMT_OK:
		ajDebug("++varinRead OK, set format %d\n",
                        varin->Input->Format);
		varDefine(var, varin);

		return ajTrue;
	    case FMT_BADTYPE:
		ajDebug("varinRead: (a1) "
                        "varinReadFmt stat == BADTYPE *failed*\n");

		return ajFalse;
	    case FMT_FAIL:
		ajDebug("varinRead: (b1) "
                        "varinReadFmt stat == FAIL *failed*\n");
		break;			/* we can try next format */
	    case FMT_NOMATCH:
		ajDebug("varinRead: (c1) "
                        "varinReadFmt stat==NOMATCH try again\n");
		break;
	    case FMT_EOF:
		ajDebug("varinRead: (d1) "
                        "varinReadFmt stat == EOF *failed*\n");
		return ajFalse;			/* EOF and unbuffered */
	    case FMT_EMPTY:
		ajWarn("variation data '%S' has zero length, ignored",
		       ajVarGetQryS(var));
		ajDebug("varinRead: (e1) "
                        "varinReadFmt stat==EMPTY try again\n");
		break;
	    default:
		ajDebug("unknown code %d from varinReadFmt\n", stat);
	    }

	    ajVarClear(var);

	    if(varin->Input->Format)
		break;			/* we read something */

            ajFilebuffTrace(varin->Input->Filebuff);
	}

	if(!varin->Input->Format)
	{		     /* all default formats failed, give up */
	    ajDebug("varinRead:all default formats failed, give up\n");

	    return ajFalse;
	}

	ajDebug("++varinRead set format %d\n",
                varin->Input->Format);
    }
    else
    {					/* one format specified */
	ajDebug("varinRead: one format specified\n");
	ajFilebuffSetUnbuffered(varin->Input->Filebuff);

	ajDebug("++varinRead known format %d\n",
                varin->Input->Format);
	istat = varinReadFmt(varin, var, varin->Input->Format);

	switch(istat)
	{
	case FMT_OK:
	    varDefine(var, varin);

	    return ajTrue;
	case FMT_BADTYPE:
	    ajDebug("varinRead: (a2) "
                    "varinReadFmt stat == BADTYPE *failed*\n");

	    return ajFalse;

        case FMT_FAIL:
	    ajDebug("varinRead: (b2) "
                    "varinReadFmt stat == FAIL *failed*\n");

	    return ajFalse;

        case FMT_NOMATCH:
	    ajDebug("varinRead: (c2) "
                    "varinReadFmt stat == NOMATCH *try again*\n");
	    break;
	case FMT_EOF:
	    ajDebug("varinRead: (d2) "
                    "varinReadFmt stat == EOF *try again*\n");
            if(varin->Input->Records)
                ajErr("Error reading file '%F' with format '%s': "
                      "end-of-file before end of data "
                      "(read %u records)",
                      ajFilebuffGetFile(varin->Input->Filebuff),
                      varinFormatDef[varin->Input->Format].Name,
                      varin->Input->Records);
	    break;		     /* simply end-of-file */
	case FMT_EMPTY:
	    ajWarn("assmebly data '%S' has zero length, ignored",
		   ajVarGetQryS(var));
	    ajDebug("varinRead: (e2) "
                    "varinReadFmt stat == EMPTY *try again*\n");
	    break;
	default:
	    ajDebug("unknown code %d from varinReadFmt\n", stat);
	}

	ajVarClear(var); /* 1 : read, failed to match id/acc/query */
    }

    /* failed - probably entry/accession query failed. Can we try again? */

    ajDebug("varinRead failed - try again with format %d '%s' code %d\n",
	    varin->Input->Format,
            varinFormatDef[varin->Input->Format].Name, istat);

    ajDebug("Search:%B Chunk:%B Data:%x ajFileBuffEmpty:%B\n",
	    varin->Input->Search, varin->Input->ChunkEntries,
            varin->Input->TextData, ajFilebuffIsEmpty(buff));

    if(ajFilebuffIsEmpty(buff) && varin->Input->ChunkEntries)
    {
	if(textaccess && !textaccess->Access(varin->Input))
            return ajFalse;
	else if(varaccess && !varaccess->Access(varin))
            return ajFalse;
        buff = varin->Input->Filebuff;
    }


    /* need to check end-of-file to avoid repeats */
    while(varin->Input->Search &&
          (varin->Input->TextData || !ajFilebuffIsEmpty(buff)))
    {
	jstat = varinReadFmt(varin, var, varin->Input->Format);

	switch(jstat)
	{
	case FMT_OK:
	    varDefine(var, varin);

	    return ajTrue;

        case FMT_BADTYPE:
	    ajDebug("varinRead: (a3) "
                    "varinReadFmt stat == BADTYPE *failed*\n");

	    return ajFalse;

        case FMT_FAIL:
	    ajDebug("varinRead: (b3) "
                    "varinReadFmt stat == FAIL *failed*\n");

	    return ajFalse;
            
	case FMT_NOMATCH:
	    ajDebug("varinRead: (c3) "
                    "varinReadFmt stat == NOMATCH *try again*\n");
	    break;
	case FMT_EOF:
	    ajDebug("varinRead: (d3) "
                    "varinReadFmt stat == EOF *failed*\n");

	    return ajFalse;			/* we already tried again */

        case FMT_EMPTY:
	    if(istat != FMT_EMPTY)
                ajWarn("assmebly data '%S' has zero length, ignored",
                       ajVarGetQryS(var));
	    ajDebug("varinRead: (e3) "
                    "varinReadFmt stat == EMPTY *try again*\n");
	    break;

        default:
	    ajDebug("unknown code %d from varinReadFmt\n", stat);
	}

	ajVarClear(var); /* 1 : read, failed to match id/acc/query */
    }

    if(varin->Input->Format)
	ajDebug("varinRead: *failed* to read variation data %S "
                "using format %s\n",
		varin->Input->Qry,
                varinFormatDef[varin->Input->Format].Name);
    else
	ajDebug("varinRead: *failed* to read variation data %S "
                "using any format\n",
		varin->Input->Qry);

    return ajFalse;
}




/* @funcstatic varinReadAbc **************************************************
**
** Given data in a variation structure, tries to read everything needed
** using Abc format.
**
** @param [u] varin [AjPVarin] Var input object
** @param [w] var [AjPVar] var object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool varinReadAbc(AjPVarin varin, AjPVar var)
{
    AjPFilebuff buff;

    ajlong fpos     = 0;
    ajuint linecnt = 0;

    ajDebug("varinReadAbc\n");
    ajVarClear(var);
    buff = varin->Input->Filebuff;

    /* ajFilebuffTrace(buff); */

    while (ajBuffreadLinePos(buff, &varinReadLine, &fpos))
    {
        linecnt++;

        if(ajStrGetCharLast(varinReadLine) == '\n')
            ajStrCutEnd(&varinReadLine, 1);

        if(ajStrGetCharLast(varinReadLine) == '\r')
            ajStrCutEnd(&varinReadLine, 1);

        ajDebug("line %u:%S\n", linecnt, varinReadLine);

        /* add line to AjPVar object */
    }

    return ajTrue;
}




/* @datasection [none] Miscellaneous ******************************************
**
** Variation input internals
**
** @nam2rule Varin Variation input
**
******************************************************************************/




/* @section Printing **********************************************************
**
** Printing details of the internals to a file
**
** @fdata [none]
**
** @nam2rule Varinprint
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




/* @func ajVarinprintBook ****************************************************
**
** Reports the internal data structures as a Docbook table
**
** @param [u] outf [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

void ajVarinprintBook(AjPFile outf)
{
    ajuint i = 0;
    ajuint j = 0;
    AjPStr namestr = NULL;
    AjPList fmtlist;
    AjPStr* names;

    fmtlist = ajListstrNew();

    ajFmtPrintF(outf, "<para>The supported variation formats are summarised "
                "in the table below. "
                "The columns are as follows: "
                "<emphasis>Input format</emphasis> (format name), "
                "<emphasis>Try</emphasis> (indicates whether the "
                "format can be detected automatically on input), and "
                "<emphasis>Description</emphasis> (short description of "
                "the format).</para>\n\n");

    ajFmtPrintF(outf, "<table frame=\"box\" rules=\"cols\">\n");
    ajFmtPrintF(outf, "  <caption>Input variation formats</caption>\n");
    ajFmtPrintF(outf, "  <thead>\n");
    ajFmtPrintF(outf, "    <tr align=\"center\">\n");
    ajFmtPrintF(outf, "      <th>Input Format</th>\n");
    ajFmtPrintF(outf, "      <th>Try</th>\n");
    ajFmtPrintF(outf, "      <th>Description</th>\n");
    ajFmtPrintF(outf, "    </tr>\n");
    ajFmtPrintF(outf, "  </thead>\n");
    ajFmtPrintF(outf, "  <tbody>\n");

    for(i=1; varinFormatDef[i].Name; i++)
    {
	if(!varinFormatDef[i].Alias)
        {
            namestr = ajStrNewC(varinFormatDef[i].Name);
            ajListPushAppend(fmtlist, namestr);
            namestr = NULL;
        }
    }

    ajListSort(fmtlist, ajStrVcmp);
    ajListstrToarray(fmtlist, &names);

    for(i=0; names[i]; i++)
    {
        for(j=0; varinFormatDef[j].Name; j++)
        {
            if(ajStrMatchC(names[i],varinFormatDef[j].Name))
            {
                ajFmtPrintF(outf, "    <tr>\n");
                ajFmtPrintF(outf, "      <td>%s</td>\n",
                            varinFormatDef[j].Name);
                ajFmtPrintF(outf, "      <td>%B</td>\n",
                            varinFormatDef[j].Try);
                ajFmtPrintF(outf, "      <td>%s</td>\n",
                            varinFormatDef[j].Desc);
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




/* @func ajVarinprintHtml ***************************************************
**
** Reports the internal data structures as an HTML table
**
** @param [u] outf [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

void ajVarinprintHtml(AjPFile outf)
{
    ajuint i = 0;
    ajuint j = 0;

    AjPStr namestr = NULL;

    ajFmtPrintF(outf, "<table border=3>");
    ajFmtPrintF(outf, "<tr><th>Input Format</th><th>Auto</th>\n");
    ajFmtPrintF(outf, "<th>Multi</th><th>Description</th></tr>\n");

    for(i=1; varinFormatDef[i].Name; i++)
    {
        ajStrAssignC(&namestr, varinFormatDef[i].Name);

	if(!varinFormatDef[i].Alias)
        {
            for(j=i+1; varinFormatDef[j].Name; j++)
            {
                if(varinFormatDef[j].Read == varinFormatDef[i].Read)
                {
                    ajFmtPrintAppS(&namestr, " %s",
                                   varinFormatDef[j].Name);
                    if(!varinFormatDef[j].Alias) 
                    {
                        ajWarn("Input format '%s' same as '%s' but not alias",
                               varinFormatDef[j].Name,
                               varinFormatDef[i].Name);
                    }
                }
            }

	    ajFmtPrintF(outf, "<tr><td>\n%S\n</td><td>%B</td>\n",
                        namestr,
			varinFormatDef[i].Try);
            ajFmtPrintF(outf, "<td>\n%s\n</td></tr>\n",
			varinFormatDef[i].Desc);
        }

    }

    ajFmtPrintF(outf, "</table>\n");
    ajStrDel(&namestr);

    return;
}




/* @func ajVarinprintText ***************************************************
**
** Reports the internal data structures
**
** @param [u] outf [AjPFile] Output file
** @param [r] full [AjBool] Full report (usually ajFalse)
** @return [void]
** @@
******************************************************************************/

void ajVarinprintText(AjPFile outf, AjBool full)
{
    ajuint i = 0;

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# Variation input formats\n");
    ajFmtPrintF(outf, "# Name  Format name (or alias)\n");
    ajFmtPrintF(outf, "# Alias Alias name\n");
    ajFmtPrintF(outf, "# Try   Test for unknown input files\n");
    ajFmtPrintF(outf, "# Name         Alias Try "
		"Description");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "InFormat {\n");

    for(i=0; varinFormatDef[i].Name; i++)
	if(full || !varinFormatDef[i].Alias)
	    ajFmtPrintF(outf,
			"  %-12s %5B %3B \"%s\"\n",
			varinFormatDef[i].Name,
			varinFormatDef[i].Alias,
			varinFormatDef[i].Try,
			varinFormatDef[i].Desc);

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajVarinprintWiki ***************************************************
**
** Reports the internal data structures as a wiki table
**
** @param [u] outf [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

void ajVarinprintWiki(AjPFile outf)
{
    ajuint i = 0;
    ajuint j = 0;

    AjPStr namestr = NULL;

    ajFmtPrintF(outf, "{| class=\"wikitable sortable\" border=\"2\"\n");
    ajFmtPrintF(outf, "|-\n");
    ajFmtPrintF(outf, "!Format!!Try!!"
                "class=\"unsortable\"|Description\n");

    for(i=1; varinFormatDef[i].Name; i++)
    {
        ajStrAssignC(&namestr, varinFormatDef[i].Name);

	if(!varinFormatDef[i].Alias)
        {
            for(j=i+1; varinFormatDef[j].Name; j++)
            {
                if(varinFormatDef[j].Read == varinFormatDef[i].Read)
                {
                    ajFmtPrintAppS(&namestr, "<br>%s",
                                   varinFormatDef[j].Name);
                    if(!varinFormatDef[j].Alias) 
                    {
                        ajWarn("Input format '%s' same as '%s' but not alias",
                               varinFormatDef[j].Name,
                               varinFormatDef[i].Name);
                    }
                }
            }

            ajFmtPrintF(outf, "|-\n");
	    ajFmtPrintF(outf,
			"|%S||%B||%s\n",
			namestr,
			varinFormatDef[i].Try,
			varinFormatDef[i].Desc);
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




/* @func ajVarinExit *********************************************************
**
** Cleans up variation input internal memory
**
** @return [void]
** @@
******************************************************************************/

void ajVarinExit(void)
{
    ajStrDel(&varinReadLine);

    ajTableDel(&varDbMethods);

    return;
}




/* @section Internals *********************************************************
**
** Functions to return internal values
**
** @fdata [none]
**
** @nam3rule Type Internals for var datatype
** @nam4rule Get  Return a value
** @nam5rule Fields  Known query fields for ajVarinRead
** @nam5rule Qlinks  Known query link operators for ajVarinRead
**
** @valrule * [const char*] Internal value
**
** @fcategory misc
**
******************************************************************************/




/* @func ajVarinTypeGetFields ************************************************
**
** Returns the list of known field names for ajVarinRead
**
** @return [const char*] List of field names
** @@
******************************************************************************/

const char* ajVarinTypeGetFields(void)
{
    return "id acc";
}




/* @func ajVarinTypeGetQlinks ************************************************
**
** Returns the listof known query link operators for ajVarinRead
**
** @return [const char*] List of field names
** @@
******************************************************************************/

const char* ajVarinTypeGetQlinks(void)
{
    return "|";
}




/* @datasection [AjPTable] Internal call register table ***********************
**
** Functions to manage the internal call register table that links the
** ajaxdb library functions with code in the core AJAX library.
**
** @nam2rule Varaccess Functions to manage vardb call tables.
**
******************************************************************************/




/* @section Cast **************************************************************
**
** Return a reference to the call table
**
** @fdata [AjPTable] vardb functions call table
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




/* @func ajVaraccessGetDb ***************************************************
**
** Returns the table in which variation database access details are registered
**
** @return [AjPTable] Access functions hash table
** @@
******************************************************************************/

AjPTable ajVaraccessGetDb(void)
{
    if(!varDbMethods)
        varDbMethods = ajCallTableNew();
    return varDbMethods;
    
}




/* @func ajVaraccessMethodGetQlinks ******************************************
**
** Tests for a named method for var data reading returns the 
** known query link operators
**
** @param [r] method [const AjPStr] Method required.
** @return [const char*] Known link operators
** @@
******************************************************************************/

const char* ajVaraccessMethodGetQlinks(const AjPStr method)
{
    AjPVarAccess methoddata; 

    methoddata = ajCallTableGetS(varDbMethods, method);
    if(!methoddata)
        return NULL;

    return methoddata->Qlink;
}




/* @func ajVaraccessMethodGetScope ******************************************
**
** Tests for a named method for variation data reading and returns the scope
** (entry, query or all).
*
** @param [r] method [const AjPStr] Method required.
** @return [ajuint] Scope flags
** @@
******************************************************************************/

ajuint ajVaraccessMethodGetScope(const AjPStr method)
{
    AjPVarAccess methoddata; 
    ajuint ret = 0;

    methoddata = ajCallTableGetS(varDbMethods, method);
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




/* @func ajVaraccessMethodTest **********************************************
**
** Tests for a named method for variation data reading.
**
** @param [r] method [const AjPStr] Method required.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajVaraccessMethodTest(const AjPStr method)
{
    if(ajCallTableGetS(varDbMethods, method))
      return ajTrue;

    return ajFalse;
}




/* @funcstatic varinQryRestore **********************************************
**
** Restores a variation input specification from an AjPQueryList node
**
** @param [w] varin [AjPVarin] Variation input object
** @param [r] node [const AjPQueryList] Query list node
** @return [void]
******************************************************************************/

static void varinQryRestore(AjPVarin varin, const AjPQueryList node)
{
    varin->Input->Format = node->Format;
    ajStrAssignS(&varin->Input->Formatstr, node->Formatstr);

    return;
}




/* @funcstatic varinQrySave *************************************************
**
** Saves a variation input specification in an AjPQueryList node
**
** @param [w] node [AjPQueryList] Query list node
** @param [r] varin [const AjPVarin] Variation input object
** @return [void]
******************************************************************************/

static void varinQrySave(AjPQueryList node, const AjPVarin varin)
{
    node->Format   = varin->Input->Format;
    ajStrAssignS(&node->Formatstr, varin->Input->Formatstr);

    return;
}




/* @funcstatic varinQryProcess **********************************************
**
** Converts a variation data query into an open file.
**
** Tests for "format::" and sets this if it is found
**
** Then tests for "list:" or "@" and processes as a list file
** using varinListProcess which in turn invokes varinQryProcess
** until a valid query is found.
**
** Then tests for dbname:query and opens the file (at the correct position
** if the database definition defines it)
**
** If there is no database, looks for file:query and opens the file.
** In this case the file position is not known and variation data reading
** will have to scan for the entry/entries we need.
**
** @param [u] varin [AjPVarin] Variation data input structure.
** @param [u] var [AjPVar] Variation data to be read.
**                         The format will be replaced
**                         if defined in the query string.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool varinQryProcess(AjPVarin varin, AjPVar var)
{
    AjBool ret = ajTrue;
    AjPStr qrystr = NULL;
    AjBool varmethod = ajFalse;
    const AjPStr fmtstr = NULL;
    AjPTextin textin;
    AjPQuery qry;
    AjPVarAccess varaccess = NULL;

    textin = varin->Input;
    qry = textin->Query;

    /* pick up the original query string */
    qrystr = ajStrNewS(textin->Qry);

    ajDebug("varinQryProcess '%S'\n", qrystr);

    /* look for a format:: prefix */
    fmtstr = ajQuerystrParseFormat(&qrystr, textin, varinformatFind);
    ajDebug("varinQryProcess ... fmtstr '%S' '%S'\n", fmtstr, qrystr);

    /* (seq/feat) DO NOT look for a [range] suffix */

    /* look for a list:: or @:: listfile of queries  - process and return */
    if(ajQuerystrParseListfile(&qrystr))
    {
        ajDebug("varinQryProcess ... listfile '%S'\n", qrystr);
        ret = varinListProcess(varin, var, qrystr);
        ajStrDel(&qrystr);
        return ret;
    }

    /* try general text access methods (file, asis, text database access */
    ajDebug("varinQryProcess ... no listfile '%S'\n", qrystr);
    if(!ajQuerystrParseRead(&qrystr, textin, varinformatFind, &varmethod))
    {
        ajStrDel(&qrystr);
        return ajFalse;
    }
    
    varinFormatSet(varin, var);

    ajDebug("varinQryProcess ... read nontext: %B '%S'\n",
            varmethod, qrystr);
    ajStrDel(&qrystr);

    /* we found a non-text method */
    if(varmethod)
    {
        ajDebug("varinQryProcess ... call method '%S'\n", qry->Method);
        ajDebug("varinQryProcess ... textin format %d '%S'\n",
                textin->Format, textin->Formatstr);
        ajDebug("varinQryProcess ...  query format  '%S'\n",
                qry->Formatstr);
        qry->Access = ajCallTableGetS(varDbMethods,qry->Method);
        varaccess = qry->Access;
        return varaccess->Access(varin);
    }

    ajDebug("varinQryProcess text method '%S' success\n", qry->Method);

    return ajTrue;
}





/* @datasection [AjPList] Query field list ************************************
**
** Query fields lists are handled internally. Only static functions
** should appear here
**
******************************************************************************/




/* @funcstatic varinListProcess **********************************************
**
** Processes a file of queries.
** This function is called by, and calls, varinQryProcess. There is
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
** @param [u] varin [AjPVarin] Variation data input
** @param [u] var [AjPVar] Variation data
** @param [r] listfile [const AjPStr] Name of list file.,
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool varinListProcess(AjPVarin varin, AjPVar var,
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
    ajDebug("++varinListProcess %S depth %d\n",
	    listfile, depth);

    if(depth > MAXDEPTH)
	ajFatal("Query list too deep");

    if(!varin->Input->List)
	varin->Input->List = ajListNew();

    list = ajListNew();

    file = ajFileNewInNameS(listfile);

    if(!file)
    {
	ajErr("Failed to open list file '%S'", listfile);
	depth--;

	return ret;
    }

    while(ajReadlineTrim(file, &varinReadLine))
    {
	varinListNoComment(&varinReadLine);

	if(ajStrGetLen(varinReadLine))
	{
	    ajStrTokenAssignC(&handle, varinReadLine, " \t\n\r");
	    ajStrTokenNextParse(&handle, &token);
	    /* ajDebug("Line  '%S'\n");*/
	    /* ajDebug("token '%S'\n", varinReadLine, token); */

	    if(ajStrGetLen(token))
	    {
	        ajDebug("++Add to list: '%S'\n", token);
	        AJNEW0(node);
	        ajStrAssignS(&node->Qry, token);
	        varinQrySave(node, varin);
	        ajListPushAppend(list, node);
	    }

	    ajStrDel(&token);
	    token = NULL;
	}
    }

    ajFileClose(&file);
    ajStrDel(&token);

    ajDebug("Trace varin->Input->List\n");
    ajQuerylistTrace(varin->Input->List);
    ajDebug("Trace new list\n");
    ajQuerylistTrace(list);
    ajListPushlist(varin->Input->List, &list);

    ajDebug("Trace combined varin->Input->List\n");
    ajQuerylistTrace(varin->Input->List);

    /*
     ** now try the first item on the list
     ** this can descend recursively if it is also a list
     ** which is why we check the depth above
     */

    if(ajListPop(varin->Input->List, (void**) &node))
    {
        ajDebug("++pop first item '%S'\n", node->Qry);
	ajVarinQryS(varin, node->Qry);
	varinQryRestore(varin, node);
	ajStrDel(&node->Qry);
	ajStrDel(&node->Formatstr);
	AJFREE(node);
	ajDebug("descending with query '%S'\n", varin->Input->Qry);
	ret = varinQryProcess(varin, var);
    }

    ajStrTokenDel(&handle);
    depth--;
    ajDebug("++varinListProcess depth: %d returns: %B\n", depth, ret);

    return ret;
}




/* @funcstatic varinListNoComment ********************************************
**
** Strips comments from a character string (a line from an ACD file).
** Comments are blank lines or any text following a "#" character.
**
** @param [u] text [AjPStr*] Line of text from input file.
** @return [void]
** @@
******************************************************************************/

static void varinListNoComment(AjPStr* text)
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




/* @funcstatic varinFormatSet ************************************************
**
** Sets the input format for variation data using the variation data
** input object's defined format
**
** @param [u] varin [AjPVarin] Variation data input.
** @param [u] var [AjPVar] Variation data
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool varinFormatSet(AjPVarin varin, AjPVar var)
{

    if(ajStrGetLen(varin->Input->Formatstr))
    {
	ajDebug("... input format value '%S'\n",
                varin->Input->Formatstr);

	if(varinformatFind(varin->Input->Formatstr,
                             &varin->Input->Format))
	{
	    ajStrAssignS(&var->Formatstr,
                         varin->Input->Formatstr);
	    var->Format = varin->Input->Format;
	    ajDebug("...format OK '%S' = %d\n",
                    varin->Input->Formatstr,
		    varin->Input->Format);
	}
	else
	    ajDebug("...format unknown '%S'\n",
                    varin->Input->Formatstr);

	return ajTrue;
    }
    else
	ajDebug("...input format not set\n");


    return ajFalse;
}




/* @datasection [AjPVarall] Variation Input Stream ****************************
**
** Function is for manipulating variation input stream objects
**
** @nam2rule Varall Variation input stream objects
**
******************************************************************************/




/* @section Variation Input Constructors **************************************
**
** All constructors return a new variation input stream object by pointer. It
** is the responsibility of the user to first destroy any previous
** variation input object. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so
** anyway.
**
** @fdata [AjPVarall]
**
** @nam3rule New Constructor
**
** @valrule * [AjPVarall] Variation input stream object
**
** @fcategory new
**
******************************************************************************/




/* @func ajVarallNew ***********************************************************
**
** Creates a new variation input stream object.
**
** @return [AjPVarall] New variation input stream object.
** @@
******************************************************************************/

AjPVarall ajVarallNew(void)
{
    AjPVarall pthis;

    AJNEW0(pthis);

    pthis->Varin = ajVarinNew();
    pthis->Var   = ajVarNew();

    return pthis;
}





/* ==================================================================== */
/* ========================== destructors ============================= */
/* ==================================================================== */




/* @section Variation Input Stream Destructors ********************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the variation input stream object.
**
** @fdata [AjPVarall]
**
** @nam3rule Del Destructor
**
** @argrule Del pthis [AjPVarall*] Variation input stream
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajVarallDel ***********************************************************
**
** Deletes a variation input stream object.
**
** @param [d] pthis [AjPVarall*] Variation input stream
** @return [void]
** @@
******************************************************************************/

void ajVarallDel(AjPVarall* pthis)
{
    AjPVarall thys;

    if(!pthis)
        return;

    thys = *pthis;

    if(!thys)
        return;

    ajVarinDel(&thys->Varin);
    if(!thys->Returned)
        ajVarDel(&thys->Var);

    AJFREE(*pthis);

    return;
}




/* ==================================================================== */
/* =========================== Modifiers ============================== */
/* ==================================================================== */




/* @section Variation input stream modifiers **********************************
**
** These functions use the contents of a variation input stream object and
** update them.
**
** @fdata [AjPVarall]
**
** @nam3rule Clear Clear all values
**
** @argrule * thys [AjPVarall] Variation input stream object
**
** @valrule * [void]
**
** @fcategory modify
**
******************************************************************************/




/* @func ajVarallClear ********************************************************
**
** Clears a variation input stream object back to "as new" condition, except
** for the query list which must be preserved.
**
** @param [w] thys [AjPVarall] Variation input stream
** @return [void]
** @@
******************************************************************************/

void ajVarallClear(AjPVarall thys)
{

    ajDebug("ajVarallClear called\n");

    ajVarinClear(thys->Varin);

    ajVarClear(thys->Var);

    thys->Returned = ajFalse;

    return;
}




/* @section Variation input stream casts **************************************
**
** These functions return the contents of a variation input stream object
**
** @fdata [AjPVarall]
**
** @nam3rule Get Get variation input stream values
** @nam3rule Getvar Get variation values
** @nam4rule GetvarId Get variation identifier
**
** @argrule * thys [const AjPVarall] Variation input stream object
**
** @valrule * [const AjPStr] String value
**
** @fcategory cast
**
******************************************************************************/




/* @func ajVarallGetvarId *****************************************************
**
** Returns the identifier of the current variation in an input stream
**
** @param [r] thys [const AjPVarall] Variation input stream
** @return [const AjPStr] Identifier
** @@
******************************************************************************/

const AjPStr ajVarallGetvarId(const AjPVarall thys)
{
    if(!thys)
        return NULL;

    ajDebug("ajVarallGetvarId called\n");

    return ajVarGetId(thys->Var);
}




/* @section Variation input ***************************************************
**
** These functions use a variation input stream object to read data
**
** @fdata [AjPVarall]
**
** @nam3rule Next Read next variation
**
** @argrule * thys [AjPVarall] Variation input stream object
** @argrule * Pvar [AjPVar*] Variation object
**
** @valrule * [AjBool] True on success
**
** @fcategory use
**
******************************************************************************/




/* @func ajVarallNext *********************************************************
**
** Parse a variation query into format, access, file and entry
**
** Split at delimiters. Check for the first part as a valid format
** Check for the remaining first part as a database name or as a file
** that can be opened.
** Anything left is an entryname spec.
**
** Return the results in the AjPVar object but leave the file open for
** future calls.
**
** @param [w] thys [AjPVarall] Variation input stream
** @param [u] Pvar [AjPVar*] Variation returned
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajVarallNext(AjPVarall thys, AjPVar *Pvar)
{
    ajDebug("ajVarallNext count:%u\n", thys->Count);

    if(!thys->Count)
    {
	thys->Count = 1;

	thys->Totterms++;

	*Pvar = thys->Var;
	thys->Returned = ajTrue;

	return ajTrue;
    }


    if(ajVarinRead(thys->Varin, thys->Var))
    {
	thys->Count++;

	thys->Totterms++;

	*Pvar = thys->Var;
	thys->Returned = ajTrue;

	ajDebug("ajVarallNext success\n");

	return ajTrue;
    }

    *Pvar = NULL;

    ajDebug("ajVarallNext failed\n");

    ajVarallClear(thys);

    return ajFalse;
}




/* @datasection [none] Input formats ******************************************
**
** Input formats internals
**
** @nam2rule Varinformat Variation data input format specific
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




/* @funcstatic varinformatFind ***********************************************
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

static AjBool varinformatFind(const AjPStr format, ajint* iformat)
{
    AjPStr tmpformat = NULL;
    ajuint i = 0;

    /* ajDebug("varinformatFind '%S'\n", format); */
    if(!ajStrGetLen(format))
	return ajFalse;

    ajStrAssignS(&tmpformat, format);
    ajStrFmtLower(&tmpformat);

    for(i=0; varinFormatDef[i].Name; i++)
    {
	/* ajDebug("test %d '%s' \n", i, varinFormatDef[i].Name); */
	if(ajStrMatchCaseC(tmpformat, varinFormatDef[i].Name))
	{
	    *iformat = i;
	    ajStrDel(&tmpformat);
	    /* ajDebug("found '%s' at %d\n", varinFormatDef[i].Name, i); */
	    return ajTrue;
	}
    }

    ajErr("Unknown input format '%S'", format);

    ajStrDel(&tmpformat);

    return ajFalse;
}




/* @func ajVarinformatTerm ***************************************************
**
** Tests whether a variation data input format term is known
**
** @param [r] term [const AjPStr] Format term EDAM ID
** @return [AjBool] ajTrue if term was accepted
** @@
******************************************************************************/

AjBool ajVarinformatTerm(const AjPStr term)
{
    ajuint i;

    for(i=0; varinFormatDef[i].Name; i++)
	if(ajStrMatchCaseC(term, varinFormatDef[i].Obo))
	    return ajTrue;

    return ajFalse;
}




/* @func ajVarinformatTest ***************************************************
**
** Tests whether a named variation data input format is known
**
** @param [r] format [const AjPStr] Format
** @return [AjBool] ajTrue if format was accepted
** @@
******************************************************************************/

AjBool ajVarinformatTest(const AjPStr format)
{
    ajuint i;

    for(i=0; varinFormatDef[i].Name; i++)
	if(ajStrMatchCaseC(format, varinFormatDef[i].Name))
	    return ajTrue;

    return ajFalse;
}

/* @source ajtextread *********************************************************
**
** AJAX text data reading functions
**
** These functions control all aspects of AJAX text data reading
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.37 $
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

#include "ajtextread.h"
#include "ajtext.h"
#include "ajcall.h"
#include "ajlist.h"
#include "ajquery.h"
#include "ajnam.h"
#include "ajfileio.h"
#include "ajhttp.h"
#include "ajftp.h"


#include <string.h>

#ifndef WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#endif


AjPTable textDbMethods = NULL;

static AjPStr textinReadLine     = NULL;

static AjBool    textinReadText(AjPTextin thys, AjPText text);
static AjBool    textinReadXml(AjPTextin thys, AjPText text);
static AjBool    textinReadObo(AjPTextin thys, AjPText text);
static AjBool    textinReadEmbl(AjPTextin thys, AjPText text);
static AjBool    textinReadPdb(AjPTextin thys, AjPText text);




/* @datastatic TextPInFormat **************************************************
**
** Text input formats data structure
**
** @alias TextSInFormat
** @alias TextOInFormat
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

typedef struct TextSInFormat
{
    const char *Name;
    const char *Obo;
    const char *Desc;
    AjBool Alias;
    AjBool Try;
    AjBool (*Read) (AjPTextin thys, AjPText text);
} TextOInFormat;

#define TextPInFormat TextOInFormat*

static TextOInFormat textinFormatDef[] =
{
/* "Name",        "Description" */
/*     Alias,   Try,     */
/*     ReadFunction */
    {"unknown",     "0000", "Unknown format",
       AJFALSE, AJFALSE,
       &textinReadText}, /* alias for text */
    {"text",        "2330", "Plain text format",
       AJFALSE, AJTRUE,
       &textinReadText},
    {"xml",         "2332", "XML data",
       AJFALSE, AJTRUE,
       &textinReadXml},
    {"obo",         "2196", "OBO data",
       AJFALSE, AJTRUE,
       &textinReadObo},
    {"embl",        "1927", "EMBL data",
       AJFALSE, AJFALSE,
       &textinReadEmbl},
    {"swissprot",   "1963", "SwissProt data",
       AJTRUE, AJFALSE,
       &textinReadEmbl},
    {"swiss",       "1963", "SwissProt data",
       AJTRUE, AJFALSE,
       &textinReadEmbl},
    {"uniprot",     "2188", "UniProt data",
       AJTRUE, AJFALSE,
       &textinReadEmbl},
    {"uniprotkb",   "2187", "UniProt-like data",
       AJTRUE, AJFALSE,
       &textinReadEmbl},
    {"ipi",         "2189", "UniProt-like data",
       AJTRUE, AJFALSE,
       &textinReadEmbl},
    {"uniprotxml",  "0000", "Uniprot XML data",
       AJTRUE, AJFALSE,
       &textinReadXml},
    {"pdb",         "1476", "PDB data",
       AJFALSE, AJFALSE,
       &textinReadPdb},
  {NULL, NULL, NULL, 0, 0, NULL}
};



static ajuint textinReadFmt(AjPTextin textin, AjPText text,
                           ajuint format);
static AjBool textinRead(AjPTextin textin, AjPText text);
static AjBool textinformatFind(const AjPStr format, ajint* iformat);
static AjBool textinFormatSet(AjPTextin textin, AjPText text);
static AjBool textinListProcess(AjPTextin textin, AjPText text,
                               const AjPStr listfile);
static void textinListNoComment(AjPStr* text);
static void textinQryRestore(AjPTextin textin, const AjPQueryList node);
static void textinQrySave(AjPQueryList node, const AjPTextin textin);
static AjBool textDefine(AjPText thys, AjPTextin textin);
static AjBool textinQryProcess(AjPTextin textin, AjPText text);
static AjBool textinQueryMatch(const AjPQuery thys, const AjPText text);




/* @filesection ajtextread ****************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
*/




/* @datasection [AjPTextin] Text input objects ********************************
**
** Function is for manipulating text input objects
**
** @nam2rule Textin
******************************************************************************/




/* @section Text Input Constructors *******************************************
**
** All constructors return a new text input object by pointer. It
** is the responsibility of the user to first destroy any previous
** text input object. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so
** anyway.
**
** @fdata [AjPTextin]
**
** @nam3rule  New     Construct a new text input object
** @nam4rule  Datatype     Construct a new text input object for a
**                         specific datatype
**
** @argrule   Datatype datatype [const AjEDataType] Enumerated datatype
**
** @valrule   *  [AjPTextin] New text input object
**
** @fcategory new
**
******************************************************************************/




/* @func ajTextinNew **********************************************************
**
** Creates a new text input object.
**
** @return [AjPTextin] New text input object.
** @category new [AjPTextin] Default constructor
**
** @release 6.4.0
** @@
******************************************************************************/

AjPTextin ajTextinNew(void)
{
    return ajTextinNewDatatype(AJDATATYPE_UNKNOWN);
}




/* @func ajTextinNewDatatype **************************************************
**
** Creates a new text input object for assembly data
**
** @param [r] datatype [const AjEDataType] Enumerated datatype
** @return [AjPTextin] New text input object.
** @category new [AjPTextin] Default constructor
**
** @release 6.4.0
** @@
******************************************************************************/

AjPTextin ajTextinNewDatatype(const AjEDataType datatype)
{
    AjPTextin pthis;

    AJNEW0(pthis);

    pthis->Db  = ajStrNew();
    pthis->Qry  = ajStrNew();
    pthis->Formatstr = ajStrNew();
    pthis->QryFields = ajStrNew();
    pthis->Filename  = ajStrNew();

    pthis->Query     = ajQueryNew(datatype);

    pthis->Search    = ajTrue;

    return pthis;
}




/* @section Text Input Destructors ********************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the text input object.
**
** @fdata [AjPTextin]
**
** @nam3rule Del Destructor
** @nam4rule Nofile File buffer is a copy, do not delete
**
** @argrule Del pthis [AjPTextin*] Text input
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajTextinDel **********************************************************
**
** Deletes a text input object.
**
** @param [d] pthis [AjPTextin*] Text input
** @return [void]
** @category delete [AjPTextin] Default destructor
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTextinDel(AjPTextin* pthis)
{
    AjPTextin thys;
    AjPQueryList node = NULL;

    if(!pthis)
        return;

    thys = *pthis;

    if(!thys)
        return;

    ajDebug("ajTextinDel called qry:'%S' filebuff: %x\n",
            thys->Qry, thys->Filebuff);

    ajStrDel(&thys->Db);
    ajStrDel(&thys->Qry);
    ajStrDel(&thys->Formatstr);
    ajStrDel(&thys->QryFields);
    ajStrDel(&thys->Filename);

    while(ajListGetLength(thys->List))
    {
	ajListPop(thys->List, (void**) &node);
	ajStrDel(&node->Qry);
	ajStrDel(&node->Formatstr);
	AJFREE(node);
    }

    ajListFree(&thys->List);

    ajQueryDel(&thys->Query);

    if(thys->Filebuff)
	ajFilebuffDel(&thys->Filebuff);

    AJFREE(*pthis);

    return;
}




/* @func ajTextinDelNofile ****************************************************
**
** Deletes a text input object but do not close the file as it is a
** copy of a file buffer elsewhere
**
** @param [d] pthis [AjPTextin*] Text input
** @return [void]
** @category delete [AjPTextin] Default destructor
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTextinDelNofile(AjPTextin* pthis)
{
    AjPTextin thys;
    AjPQueryList node = NULL;

    if(!pthis)
        return;

    thys = *pthis;

    if(!thys)
        return;

    ajDebug("ajTextinDel called qry:'%S'\n", thys->Qry);

    ajStrDel(&thys->Db);
    ajStrDel(&thys->Qry);
    ajStrDel(&thys->Formatstr);
    ajStrDel(&thys->QryFields);
    ajStrDel(&thys->Filename);

    while(ajListGetLength(thys->List))
    {
	ajListPop(thys->List, (void**) &node);
	ajStrDel(&node->Qry);
	ajStrDel(&node->Formatstr);
	AJFREE(node);
    }

    ajListFree(&thys->List);

    ajQueryDel(&thys->Query);

    AJFREE(*pthis);

    return;
}




/* @section text input modifiers **********************************************
**
** These functions use the contents of a text input object and
** update them.
**
** @fdata [AjPTextin]
**
** @nam3rule Clear Clear all values
** @nam3rule Qry Reset using a query string
** @nam4rule Nofile File buffer is a copy, do not delete
** @suffix C Character string input
** @suffix S String input
**
** @argrule * thys [AjPTextin] Text input object
** @argrule C txt [const char*] Query text
** @argrule S str [const AjPStr] query string
**
** @valrule * [void]
**
** @fcategory modify
**
******************************************************************************/




/* @func ajTextinClear ********************************************************
**
** Clears a text input object back to "as new" condition, except
** for the query list which must be preserved.
**
** @param [w] thys [AjPTextin] Text input
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTextinClear(AjPTextin thys)
{

    ajDebug("ajTextinClear called\n");

    if(!thys)
        return;

    ajStrSetClear(&thys->Db);
    ajStrSetClear(&thys->Qry);
    ajStrSetClear(&thys->Formatstr);
    ajStrSetClear(&thys->QryFields);
    ajStrSetClear(&thys->Filename);

    /* preserve thys->List */

    if(thys->Filebuff)
	ajFilebuffDel(&thys->Filebuff);

    if(thys->Filebuff)
	ajFatal("ajTextinClear did not delete Filebuff");

    ajQueryClear(thys->Query);
    thys->TextData = NULL;

    thys->Search = ajTrue;
    thys->Single = ajFalse;
/*    thys->CaseId= ajFalse;*/
/*    thys->Multi = ajFalse;*/
/*    thys->Text = ajFalse;*/
    thys->ChunkEntries = ajFalse;

    thys->Count     = 0;
    thys->Dataread = ajFalse;
    thys->Datadone = ajFalse;
    thys->Datacount = 0;

    /* preserve thys->Filecount */
    /* preserve thys->Entrycount */

    thys->Records = 0;
    thys->Fpos = 0L;

    return;
}




/* @func ajTextinClearNofile **************************************************
**
** Clears a text input object back to "as new" condition, except
** for the query list which must be preserved.
**
** @param [w] thys [AjPTextin] Text input
** @return [void]
** @category modify [AjPTextin] Resets ready for reuse.
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTextinClearNofile(AjPTextin thys)
{

    ajDebug("ajTextinClearNofile called\n");

    ajStrSetClear(&thys->Db);
    ajStrSetClear(&thys->Qry);
    ajStrSetClear(&thys->Formatstr);
    ajStrSetClear(&thys->QryFields);
    ajStrSetClear(&thys->Filename);

    /* preserve thys->List */

    ajQueryClear(thys->Query);
    thys->TextData = NULL;

    thys->Search = ajTrue;
    thys->Single = ajFalse;
/*    thys->CaseId= ajFalse;*/
/*    thys->Multi = ajFalse;*/
/*    thys->Text = ajFalse;*/
    thys->ChunkEntries = ajFalse;

    thys->Count     = 0;

    /* preserve thys->Filecount */
    /* preserve thys->Entrycount */

    thys->Records = 0;
    thys->Fpos = 0L;

    return;
}




/* @func ajTextinQryC *********************************************************
**
** Resets a text input object using a new Universal
** Query Address
**
** @param [u] thys [AjPTextin] text input object.
** @param [r] txt [const char*] Query
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTextinQryC(AjPTextin thys, const char* txt)
{
    ajTextinClear(thys);
    ajStrAssignC(&thys->Qry, txt);

    return;
}





/* @func ajTextinQryS *********************************************************
**
** Resets a text input object using a new Universal
** Query Address
**
** @param [u] thys [AjPTextin] Text input object.
** @param [r] str [const AjPStr] Query
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTextinQryS(AjPTextin thys, const AjPStr str)
{
    ajTextinClear(thys);
    ajStrAssignS(&thys->Qry, str);

    return;
}




/* @section casts *************************************************************
**
** Return values
**
** @fdata [AjPTextin]
**
** @nam3rule Get Get obo input stream values
** @nam4rule GetQry Get obo query
** @nam3rule Trace Write debugging output
** @suffix S Return as a string object
**
** @argrule * thys [const AjPTextin] Text input object
**
** @valrule * [void]
** @valrule *S [const AjPStr] String value
**
** @fcategory cast
**
******************************************************************************/




/* @func ajTextinGetQryS ******************************************************
**
** Returns the query of a text input object
**
** @param [r] thys [const AjPTextin] Text input object.
** @return [const AjPStr] Query string
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPStr ajTextinGetQryS(const AjPTextin thys)
{
    return thys->Qry;
}




/* @func ajTextinTrace ********************************************************
**
** Debug calls to trace the data in a text input object.
**
** @param [r] thys [const AjPTextin] Text input object.
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTextinTrace(const AjPTextin thys)
{
    ajDebug("text input trace\n");
    ajDebug( "====================\n\n");

    if(ajStrGetLen(thys->Db))
	ajDebug( "  Db: '%S'\n", thys->Db);

    if(ajStrGetLen(thys->Formatstr))
	ajDebug( "  Format: '%S' (%u)\n", thys->Formatstr, thys->Format);

    if(ajStrGetLen(thys->QryFields))
	ajDebug( "  Fields: '%S'\n", thys->QryFields);

    if(ajStrGetLen(thys->Qry))
	ajDebug( "  Query: '%S'\n", thys->Qry);

    if(ajStrGetLen(thys->Filename))
	ajDebug( "  Filename: '%S'\n", thys->Filename);

    if(ajListGetLength(thys->List))
	ajDebug( "  List: (%Lu)\n", ajListGetLength(thys->List));

    if(thys->Filebuff)
	ajDebug( "  Filebuff: %F (%Ld)\n",
		ajFilebuffGetFile(thys->Filebuff),
		ajFileResetPos(ajFilebuffGetFile(thys->Filebuff)));

    if(thys->Search)
	ajDebug( "  Search: %B\n", thys->Search);

    if(thys->Single)
	ajDebug( "  Single: %B\n", thys->Single);

    if(thys->Multi)
	ajDebug( "  Multi: %B\n", thys->Multi);

    if(thys->CaseId)
	ajDebug( "  CaseId: %B\n", thys->CaseId);

    if(thys->Text)
	ajDebug( "  Savetext: %B\n", thys->Text);

    if(thys->Count)
	ajDebug( "  Count: %u\n", thys->Count);

    if(thys->Filecount)
	ajDebug( "  File count: %u\n", thys->Filecount);

    if(thys->Entrycount)
	ajDebug( "  Entry count: %u\n", thys->Entrycount);

    if(thys->Fpos)
	ajDebug( "  Fpos: %Ld\n", thys->Fpos);

    if(thys->Query)
	ajQueryTrace(thys->Query);

    if(thys->TextData)
	ajDebug( "  TextData: exists\n");

    return;
}




/* @section Text data inputs **************************************************
**
** These functions read the text data provided by the first argument
**
** @fdata [AjPTextin]
**
** @nam3rule Read Read text data
**
** @argrule Read textin [AjPTextin] Text input object
** @argrule Read text [AjPText] Text data
**
** @valrule * [AjBool] true on success
**
** @fcategory input
**
******************************************************************************/




/* @func ajTextinRead *********************************************************
**
** If the file is not yet open, calls textinQryProcess to convert the
** query into an open file stream.
**
** Uses textinRead for the actual file reading.
**
** Returns the results in the AjPText object.
**
** @param [u] textin [AjPTextin] text data input definitions
** @param [w] text [AjPText] text data returned.
** @return [AjBool] ajTrue on success.
** @category input [AjPText] Master text data input, calls specific functions
**                  for file access type and text data format.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajTextinRead(AjPTextin textin, AjPText text)
{
    AjBool ret       = ajFalse;
    AjPQueryList node = NULL;
    AjBool listdata  = ajFalse;

    if(textin->Filebuff)
    {
	/* (a) if file still open, keep reading */
	ajDebug("ajTextinRead: input file '%F' still there, try again\n",
		textin->Filebuff->File);
	ret = textinRead(textin, text);
	ajDebug("ajTextinRead: open buffer  qry: '%S' returns: %B\n",
		textin->Qry, ret);
    }
    else
    {
	/* (b) if we have a list, try the next query in the list */
	if(ajListGetLength(textin->List))
	{
	    listdata = ajTrue;
	    ajListPop(textin->List, (void**) &node);

	    ajDebug("++pop from list '%S'\n", node->Qry);
	    ajTextinQryS(textin, node->Qry);
	    ajDebug("++SAVE TEXTIN '%S' '%S' %d\n",
		    textin->Qry,
		    textin->Formatstr, textin->Format);

            textinQryRestore(textin, node);

	    ajStrDel(&node->Qry);
	    ajStrDel(&node->Formatstr);
	    AJFREE(node);

	    ajDebug("ajTextinRead: open list, try '%S'\n", textin->Qry);

	    if(!textinQryProcess(textin, text) &&
               !ajListGetLength(textin->List))
		return ajFalse;

	    ret = textinRead(textin, text);
	    ajDebug("ajTextinRead: list qry: '%S' returns: %B\n",
		    textin->Qry, ret);
	}
	else
	{
	    ajDebug("ajTextinRead: no file yet - test query '%S'\n",
                    textin->Qry);

	    /* (c) Must be a query - decode it */
	    if(!textinQryProcess(textin, text) &&
               !ajListGetLength(textin->List))
		return ajFalse;

	    if(ajListGetLength(textin->List)) /* could be a new list */
		listdata = ajTrue;

	    ret = textinRead(textin, text);
	    ajDebug("ajTextinRead: new qry: '%S' returns: %B\n",
		    textin->Qry, ret);
	}
    }

    /* Now read whatever we got */

    while(!ret && ajListGetLength(textin->List))
    {
	/* Failed, but we have a list still - keep trying it */
        if(listdata)
	    ajErr("Failed to read text data '%S'", textin->Qry);

	listdata = ajTrue;
	ajListPop(textin->List,(void**) &node);
	ajDebug("++try again: pop from list '%S'\n", node->Qry);
	ajTextinQryS(textin, node->Qry);
	ajDebug("++SAVE (AGAIN) TEXTIN '%S' '%S' %d\n",
		textin->Qry,
		textin->Formatstr, textin->Format);

	textinQryRestore(textin, node);

	ajStrDel(&node->Qry);
	ajStrDel(&node->Formatstr);
	AJFREE(node);

	if(!textinQryProcess(textin, text))
	    continue;

	ret = textinRead(textin, text);
	ajDebug("ajTextinRead: list retry qry: '%S' returns: %B\n",
		textin->Qry, ret);
    }

    if(!ret)
    {
	if(listdata)
	    ajErr("Failed to read text data '%S'", textin->Qry);

	return ajFalse;
    }


    textDefine(text, textin);

    return ajTrue;
}




/* @funcstatic textinQueryMatch ***********************************************
**
** Compares a text data item to a query and returns true if they match.
**
** @param [r] thys [const AjPQuery] query.
** @param [r] text [const AjPText] Text data.
** @return [AjBool] ajTrue if the text data matches the query.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textinQueryMatch(const AjPQuery thys, const AjPText text)
{
    AjBool tested = ajFalse;
    AjIList iterfield  = NULL;
    AjPQueryField field = NULL;
    AjBool ok = ajFalse;

    ajDebug("textinQueryMatch '%S' fields: %Lu Case %B Done %B\n",
	    text->Id, ajListGetLength(thys->QueryFields),
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
                    text->Id);
            if(thys->CaseId)
            {
                if(ajStrMatchWildS(text->Id, field->Wildquery))
                {
                    ajListIterDel(&iterfield);
                    return ajTrue;
                }
            }
            else
            {
                if(ajStrMatchWildCaseS(text->Id, field->Wildquery))
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
            if(ajStrMatchWildCaseS(text->Id, field->Wildquery))
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




/* @funcstatic textDefine *****************************************************
**
** Make sure all text data object attributes are defined
** using values from the text input object if needed
**
** @param [w] thys [AjPText] Text data returned.
** @param [u] textin [AjPTextin] Text data input definitions
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textDefine(AjPText thys, AjPTextin textin)
{

    /* if values are missing in the text object, we can use defaults
       from textin or calculate where possible */

    /* assign the dbname if defined in the textin object */
    if(ajStrGetLen(textin->Db))
      ajStrAssignS(&thys->Db, textin->Db);

    return ajTrue;
}





/* @funcstatic textinReadFmt **************************************************
**
** Tests whether an text data can be read using the specified format.
** Then tests whether the text data matches text data query criteria
** and checks any specified type. Applies upper and lower case.
**
** @param [u] textin [AjPTextin] text data input object
** @param [w] text [AjPText] text data object
** @param [r] format [ajuint] input format code
** @return [ajuint] 0 if successful.
**                  1 if the query match failed.
**                  2 if the text data type failed
**                  3 if it failed to read an text data
**
** @release 6.4.0
** @@
** This is the only function that calls the appropriate Read function
** textinReadXxxxxx where Xxxxxxx is the supported text data format.
**
** Some of the textReadXxxxxx functions fail to reset the buffer correctly,
** which is a very serious problem when cycling through all of them to
** identify an unknown format. The extra ajFileBuffReset call at the end is
** intended to address this problem. The individual functions should still
** reset the buffer in case they are called from elsewhere.
**
******************************************************************************/

static ajuint textinReadFmt(AjPTextin textin, AjPText text,
                            ajuint format)
{
    ajDebug("++textinReadFmt format %d (%s) '%S'\n",
	    format, textinFormatDef[format].Name,
	    textin->Qry);

    textin->Records = 0;

    /* Calling funclist textinFormatDef() */
    if((*textinFormatDef[format].Read)(textin, text))
    {
	ajDebug("textinReadFmt success with format %d (%s)\n",
		format, textinFormatDef[format].Name);
        ajDebug("id: '%S'\n",
                text->Id);
	textin->Format = format;
	ajStrAssignC(&textin->Formatstr, textinFormatDef[format].Name);
	ajStrAssignC(&text->Formatstr, textinFormatDef[format].Name);
	ajStrAssignEmptyS(&text->Db, textin->Db);
	ajStrAssignS(&text->Filename, textin->Filename);
        if(!ajStrGetLen(text->Id))
        {
            ajStrAssignS(&text->Id,
                     ajFileGetPrintnameS(ajFilebuffGetFile(textin->Filebuff)));
            ajFilenameTrimAll(&text->Id);
            ajDebug("filename as id: '%S'\n",
                    text->Id);
        }

	if(textinQueryMatch(textin->Query, text))
	{
            /* ajTextinTrace(textin); */

            return FMT_OK;
        }

	ajDebug("query match failed, continuing ...\n");
	ajTextClear(text);

	return FMT_NOMATCH;
    }
    else
    {
	ajDebug("Testing input buffer: IsBuff: %B Eof: %B\n",
		ajFilebuffIsBuffered(textin->Filebuff),
		ajFilebuffIsEof(textin->Filebuff));

	if (!ajFilebuffIsBuffered(textin->Filebuff) &&
	    ajFilebuffIsEof(textin->Filebuff))
	    return FMT_EOF;

	ajFilebuffReset(textin->Filebuff);
	ajDebug("Format %d (%s) failed, file buffer reset by textinReadFmt\n",
		format, textinFormatDef[format].Name);
	/* ajFilebuffTraceFull(textin->Filebuff, 10, 10);*/
    }

    ajDebug("++textinReadFmt failed - nothing read\n");

    return FMT_FAIL;
}




/* @funcstatic textinRead *****************************************************
**
** Given data in a textin structure, tries to read everything needed
** using the specified format or by trial and error.
**
** @param [u] textin [AjPTextin] text data input object
** @param [w] text [AjPText] text data object
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textinRead(AjPTextin textin, AjPText text)
{
    ajuint i;
    ajuint istat = 0;
    ajuint jstat = 0;

    AjPFilebuff buff = textin->Filebuff;
    AjBool ok;

    AjPTextAccess textaccess = textin->Query->TextAccess;
    AjPTextAccess textonlyaccess = textin->Query->Access;

    ajTextClear(text);
    ajDebug("textinRead: cleared\n");

    if(textin->Single && textin->Count)
    {
	/*
	** One text data at a time is read.
	** The first text data was read by ACD
	** for the following ones we need to reset the AjPTextin
	**
	** Single is set by the access method
	*/

	ajDebug("textinRead: single access - count %d - call access"
		" routine again\n",
		textin->Count);
	/* Calling funclist textinAccess() */
	if(textaccess)
        {
            if(!(*textaccess->Access)(textin))
            {
                ajDebug("textinRead: (*textaccess->Access)(textin) "
                        "*failed*\n");

                return ajFalse;
            }
        }

	if(textonlyaccess)
        {
            if(!(*textonlyaccess->Access)(textin))
            {
                ajDebug("textinRead: (*textonlyaccess->Access)(textin) "
                        "*failed*\n");

                return ajFalse;
            }
        }

        buff = textin->Filebuff;
    }

    ajDebug("textinRead: textin format %d '%S'\n", textin->Format,
	    textin->Formatstr);

    textin->Count++;

    if(!textin->Filebuff)
	return ajFalse;

    ok = ajFilebuffIsBuffered(textin->Filebuff);

    while(ok)
    {				/* skip blank lines */
        ok = ajBuffreadLine(textin->Filebuff, &textinReadLine);

        if(!ajStrIsWhite(textinReadLine))
        {
            ajFilebuffClear(textin->Filebuff,1);
            break;
        }
    }

    if(!textin->Format)
    {			   /* no format specified, try all defaults */
	for(i = 1; textinFormatDef[i].Name; i++)
	{
	    if(!textinFormatDef[i].Try)	/* skip if Try is ajFalse */
		continue;

	    ajDebug("textinRead:try format %d (%s)\n",
		    i, textinFormatDef[i].Name);

	    istat = textinReadFmt(textin, text, i);

	    switch(istat)
	    {
	    case FMT_OK:
		ajDebug("++textinRead OK, set format %d\n", textin->Format);
		textDefine(text, textin);

		return ajTrue;
	    case FMT_BADTYPE:
		ajDebug("textinRead: (a1) textinReadFmt stat == BADTYPE "
                        "*failed*\n");

		return ajFalse;
	    case FMT_FAIL:
		ajDebug("textinRead: (b1) textinReadFmt stat == FAIL "
                        "*failed*\n");
		break;			/* we can try next format */
	    case FMT_NOMATCH:
		ajDebug("textinRead: (c1) textinReadFmt stat==NOMATCH "
                        "try again\n");
		break;
	    case FMT_EOF:
		ajDebug("textinRead: (d1) textinReadFmt stat == EOF "
                        "*failed*\n");
		return ajFalse;			/* EOF and unbuffered */
	    case FMT_EMPTY:
		ajWarn("text data '%S' has zero length, ignored",
		       ajTextGetQryS(text));
		ajDebug("textinRead: (e1) textinReadFmt stat==EMPTY "
                        "try again\n");
		break;
	    default:
		ajDebug("unknown code %d from textinReadFmt\n", stat);
	    }

	    ajTextClear(text);

	    if(textin->Format)
		break;			/* we read something */

            ajFilebuffTrace(textin->Filebuff);
	}

	if(!textin->Format)
	{		     /* all default formats failed, give up */
	    ajDebug("textinRead:all default formats failed, give up\n");

	    return ajFalse;
	}

	ajDebug("++textinRead set format %d\n", textin->Format);
    }
    else
    {					/* one format specified */
	ajDebug("textinRead: one format specified\n");
	ajFilebuffSetUnbuffered(textin->Filebuff);

	ajDebug("++textinRead known format %d\n", textin->Format);
	istat = textinReadFmt(textin, text, textin->Format);

	switch(istat)
	{
	case FMT_OK:
	    textDefine(text, textin);

	    return ajTrue;
	case FMT_BADTYPE:
	    ajDebug("textinRead: (a2) textinReadFmt stat == BADTYPE "
                    "*failed*\n");

	    return ajFalse;

        case FMT_FAIL:
	    ajDebug("textinRead: (b2) textinReadFmt stat == FAIL "
                    "*failed*\n");

	    return ajFalse;

        case FMT_NOMATCH:
	    ajDebug("textinRead: (c2) textinReadFmt stat == NOMATCH "
                    "*try again*\n");
	    break;
	case FMT_EOF:
	    ajDebug("textinRead: (d2) textinReadFmt stat == EOF "
                    "*try again*\n");
            if(textin->Records)
                ajErr("Error reading file '%F' with format '%s': "
                      "end-of-file before end of data "
                      "(read %u records)",
                      ajFilebuffGetFile(textin->Filebuff),
                      textinFormatDef[textin->Format].Name,
                      textin->Records);
	    break;		     /* simply end-of-file */
	case FMT_EMPTY:
	    ajWarn("text data '%S' has zero length, ignored",
		   ajTextGetQryS(text));
	    ajDebug("textinRead: (e2) textinReadFmt stat == EMPTY "
                    "*try again*\n");
	    break;
	default:
	    ajDebug("unknown code %d from textinReadFmt\n", stat);
	}

	ajTextClear(text); /* 1 : read, failed to match id/acc/query */
    }

    /* failed - probably entry/accession query failed. Can we try again? */

    ajDebug("textinRead failed - try again with format %d '%s' code %d\n",
	    textin->Format,
            textinFormatDef[textin->Format].Name, istat);

    ajDebug("Search:%B Chunk:%B Data:%x ajFileBuffEmpty:%B\n",
	    textin->Search, textin->ChunkEntries,
            textin->TextData, ajFilebuffIsEmpty(buff));

    if(ajFilebuffIsEmpty(buff) && textin->ChunkEntries)
    {
	if(textaccess && !(*textaccess->Access)(textin))
            return ajFalse;
	else if(textonlyaccess && !(*textonlyaccess->Access)(textin))
            return ajFalse;
        buff = textin->Filebuff;
    }


    /* need to check end-of-file to avoid repeats */
    while(textin->Search &&
          (textin->TextData || !ajFilebuffIsEmpty(buff)))
    {
	jstat = textinReadFmt(textin, text, textin->Format);

	switch(jstat)
	{
	case FMT_OK:
	    textDefine(text, textin);

	    return ajTrue;

        case FMT_BADTYPE:
	    ajDebug("textinRead: (a3) textinReadFmt stat == BADTYPE "
                    "*failed*\n");

	    return ajFalse;

        case FMT_FAIL:
	    ajDebug("textinRead: (b3) textinReadFmt stat == FAIL "
                    "*failed*\n");

	    return ajFalse;
            
	case FMT_NOMATCH:
	    ajDebug("textinRead: (c3) textinReadFmt stat == NOMATCH "
                    "*try again*\n");
	    break;
	case FMT_EOF:
	    ajDebug("textinRead: (d3) textinReadFmt stat == EOF "
                    "*failed*\n");

	    return ajFalse;			/* we already tried again */

        case FMT_EMPTY:
	    if(istat != FMT_EMPTY)
                ajWarn("text data '%S' has zero length, ignored",
                       ajTextGetQryS(text));
	    ajDebug("textinRead: (e3) textinReadFmt stat == EMPTY "
                    "*try again*\n");
	    break;

        default:
	    ajDebug("unknown code %d from textinReadFmt\n", stat);
	}

	ajTextClear(text); /* 1 : read, failed to match id/acc/query */
    }

    if(textin->Format)
	ajDebug("textinRead: *failed* to read text data %S using format %s\n",
		textin->Qry, textinFormatDef[textin->Format].Name);
    else
	ajDebug("textinRead: *failed* to read text data %S using any format\n",
		textin->Qry);

    return ajFalse;
}




/* @funcstatic textinReadText *************************************************
**
** Given data in a text structure, tries to read everything needed
** using the TEXT format.
**
** @param [u] textin [AjPTextin] Text input object
** @param [w] text [AjPText] Text object
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textinReadText(AjPTextin textin, AjPText text)
{
    AjPFilebuff buff;

    ajlong fpos     = 0;
    ajuint linecnt = 0;

    ajDebug("textinReadText\n");
    ajTextClear(text);
    buff = textin->Filebuff;

    /* ajFilebuffTrace(buff); */

    while (ajBuffreadLinePos(buff, &textinReadLine, &fpos))
    {
        linecnt++;
        ajStrTrimEndC(&textinReadLine, "\r\n");

        ajDebug("line %u:%S\n", linecnt, textinReadLine);

        /* add line to AjPText object */
        ajListPushAppend(text->Lines, ajStrNewS(textinReadLine));
    }
    ajDebug("textinReadText read %u lines\n", linecnt);

    if(!linecnt)
        return ajFalse;

    return ajTrue;
}




/* @funcstatic textinReadXml **************************************************
**
** Given data in a text structure, tries to read everything needed
** using the XML format.
**
** @param [u] textin [AjPTextin] Text input object
** @param [w] text [AjPText] Text object
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textinReadXml(AjPTextin textin, AjPText text)
{
    AjPFilebuff buff;

    ajlong fpos     = 0;
    ajuint linecnt = 0;

    ajDebug("textinReadXml\n");
    ajTextClear(text);
    buff = textin->Filebuff;

    /* ajFilebuffTrace(buff); */

    while (ajBuffreadLinePos(buff, &textinReadLine, &fpos))
    {
        linecnt++;
        ajStrTrimEndC(&textinReadLine, "\r\n");

        ajDebug("line %u:%S\n", linecnt, textinReadLine);

        /* add line to AjPText object */
        ajListPushAppend(text->Lines, ajStrNewS(textinReadLine));
    }
    ajDebug("textinReadXml read %u lines\n", linecnt);

    if(!linecnt)
        return ajFalse;

    return ajTrue;
}




/* @funcstatic textinReadEmbl *************************************************
**
** Given data in an obo structure, tries to read everything needed
** using the EMBL format.
**
** @param [u] textin [AjPTextin] Text input object
** @param [w] text [AjPText] Text object
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textinReadEmbl(AjPTextin textin, AjPText text)
{
    AjPFilebuff buff;
    ajuint linecnt  = 0;
    AjBool ok = ajFalse;

    ajDebug("textinReadObo\n");
    ajTextClear(text);
    buff = textin->Filebuff;

    /* ajFilebuffTrace(buff); */
    ok = ajBuffreadLine(buff, &textinReadLine);
    while(ok && !ajStrPrefixC(textinReadLine, "ID   "))
        ok = ajBuffreadLine(buff, &textinReadLine);

    if(!ok)
        return ajFalse;

    ajStrTrimEndC(&textinReadLine, "\r\n");

    ajDebug("line %u:%S\n", linecnt, textinReadLine);

    /* add line to AjPText object */
    ajListPushAppend(text->Lines, ajStrNewS(textinReadLine));

    ajFilebuffClear(buff, 1);
    ok = ajBuffreadLine(buff, &textinReadLine);
    if(ok)
        ok = ajBuffreadLine(buff, &textinReadLine);

    while (ok)
    {
        ajStrTrimWhite(&textinReadLine);

        if(!ajStrGetLen(textinReadLine))
            break;


        linecnt++;
        ajStrTrimEndC(&textinReadLine, "\r\n");

        ajDebug("line %u:%S\n", linecnt, textinReadLine);

        /* add line to AjPText object */
        ajListPushAppend(text->Lines, ajStrNewS(textinReadLine));

        if(ajStrMatchC(textinReadLine, "//"))
            break;

        ok = ajBuffreadLine(buff, &textinReadLine);
    }

    return ajTrue;
}




/* @funcstatic textinReadObo **************************************************
**
** Given data in an obo structure, tries to read everything needed
** using the OBO format.
**
** @param [u] textin [AjPTextin] Text input object
** @param [w] text [AjPText] Text object
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textinReadObo(AjPTextin textin, AjPText text)
{
    AjPFilebuff buff;
    ajuint linecnt  = 0;
    AjBool ok = ajFalse;

    ajDebug("textinReadObo\n");
    ajTextClear(text);
    buff = textin->Filebuff;

    /* ajFilebuffTrace(buff); */
    ok = ajBuffreadLine(buff, &textinReadLine);
    while(ok && !ajStrPrefixC(textinReadLine, "[Term]"))
        ok = ajBuffreadLine(buff, &textinReadLine);

    if(!ok)
        return ajFalse;

    ajStrTrimEndC(&textinReadLine, "\r\n");

    ajDebug("line %u:%S\n", linecnt, textinReadLine);

    /* add line to AjPText object */
    ajListPushAppend(text->Lines, ajStrNewS(textinReadLine));

    ajFilebuffClear(buff, 1);
    ok = ajBuffreadLine(buff, &textinReadLine);
    if(ok)
        ok = ajBuffreadLine(buff, &textinReadLine);

    while (ok)
    {
        ajStrTrimWhite(&textinReadLine);

        if(!ajStrGetLen(textinReadLine))
            break;

        if(ajStrGetCharFirst(textinReadLine) == '[') /* new stanza */
            break;

        linecnt++;
        ajStrTrimEndC(&textinReadLine, "\r\n");

        ajDebug("line %u:%S\n", linecnt, textinReadLine);

        /* add line to AjPText object */
        ajListPushAppend(text->Lines, ajStrNewS(textinReadLine));

        ok = ajBuffreadLine(buff, &textinReadLine);
    }

    return ajTrue;
}




/* @funcstatic textinReadPdb * ************************************************
**
** Given data in an obo structure, tries to read everything needed
** using the PDB format.
**
** @param [u] textin [AjPTextin] Text input object
** @param [w] text [AjPText] Text object
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textinReadPdb(AjPTextin textin, AjPText text)
{
    AjPFilebuff buff;
    ajuint linecnt  = 0;
    AjBool ok = ajFalse;

    ajDebug("textinReadPdb\n");
    ajTextClear(text);
    buff = textin->Filebuff;

    /* ajFilebuffTrace(buff); */
    ok = ajBuffreadLine(buff, &textinReadLine);
    while(ok && !ajStrPrefixC(textinReadLine, "HEADER  "))
        ok = ajBuffreadLine(buff, &textinReadLine);

    if(!ok)
        return ajFalse;

    ajStrTrimEndC(&textinReadLine, "\r\n");

    ajDebug("line %u:%S\n", linecnt, textinReadLine);

    /* add line to AjPText object */
    ajListPushAppend(text->Lines, ajStrNewS(textinReadLine));

    ajFilebuffClear(buff, 1);
    ok = ajBuffreadLine(buff, &textinReadLine);
    if(ok)
        ok = ajBuffreadLine(buff, &textinReadLine);

    while (ok)
    {
        ajStrTrimWhite(&textinReadLine);

        if(!ajStrGetLen(textinReadLine))
            break;


        linecnt++;
        ajStrTrimEndC(&textinReadLine, "\r\n");

        ajDebug("line %u:%S\n", linecnt, textinReadLine);

        /* add line to AjPText object */
        ajListPushAppend(text->Lines, ajStrNewS(textinReadLine));

        if(ajStrMatchC(textinReadLine, "END"))
            break;

        ok = ajBuffreadLine(buff, &textinReadLine);
    }

    return ajTrue;
}




/* @section File Access *******************************************************
**
** These functions manage the text file access methods.
**
** @fdata [AjPTextin]
**
** @nam3rule Access Access method
** @nam4rule Asis Reads text using the 'filename' as the single input line
** @nam4rule File Reading an input file
** @nam4rule Ftp  Reads text using the 'filename' as an FTP URL
** @nam4rule Http Reads text using the 'filename' as an HTTP URL
** @nam4rule Offset Reading an input file starting at a given offset position
**                  within the text input query
**
** @argrule Access textin [AjPTextin] Text input object
** @valrule * [AjBool] True on success
**
** @fcategory input
**
******************************************************************************/




/* @func ajTextinAccessAsis ***************************************************
**
** Reads text using the 'filename' as the single input line
**
** @param [u] textin [AjPTextin] Text input.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajTextinAccessAsis(AjPTextin textin)
{
    AjPQuery qry;

    qry = textin->Query;

    if(!ajStrGetLen(qry->Filename))
    {
	ajErr("ASIS access: no text");

	return ajFalse;
    }

    ajDebug("ajTextinAccessAsis %S\n", qry->Filename);

    ajFilebuffDel(&textin->Filebuff);
    textin->Filebuff = ajFilebuffNewLine(qry->Filename);

    if(!textin->Filebuff)
    {
	ajDebug("Asis access: unable to use text '%S'\n", qry->Filename);

	return ajFalse;
    }

    ajStrAssignC(&textin->Filename, "asis");
    /*ajFilebuffTrace(textin->Filebuff);*/

    return ajTrue;
}




/* @func ajTextinAccessFile ***************************************************
**
** Reads data from a named file.
**
** @param [u] textin [AjPTextin] Text input.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajTextinAccessFile(AjPTextin textin)
{
    AjPQuery qry;

    qry = textin->Query;

    if(!ajStrGetLen(qry->Filename))
    {
	ajErr("FILE access: no filename");

	return ajFalse;
    }

    ajDebug("ajTextinAccessFile %S\n", qry->Filename);

    /* ajStrTraceT(qry->Filename, "qry->Filename (before):"); */

    ajFilebuffDel(&textin->Filebuff);
    textin->Filebuff = ajFilebuffNewNameS(qry->Filename);

    if(!textin->Filebuff)
    {
	ajDebug("FILE access: unable to open file '%S'\n", qry->Filename);

	return ajFalse;
    }

    /* ajStrTraceT(textin->Filename, "textin->Filename:"); */
    /* ajStrTraceT(qry->Filename, "qry->Filename (after):"); */

    ajStrAssignS(&textin->Filename, qry->Filename);

    return ajTrue;
}




/* @func ajTextinAccessFtp ****************************************************
**
** Reads data from an FTP URL
**
** @param [u] textin [AjPTextin] Text input.
** @return [AjBool] ajTrue on success.
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ajTextinAccessFtp(AjPTextin textin)
{
    AjPQuery qry;
    AjPStr url = NULL;

    AjPStr host  = NULL;
    ajint iport;

    AjPStr urlget    = NULL;
    AjPUrlref urlref = NULL;

    iport = 21;
    qry = textin->Query;

    if(!ajStrGetLen(qry->Filename))
    {
	ajErr("FILE access: no filename");

	return ajFalse;
    }

    ajDebug("ajTextinAccessFtp %S\n", qry->Filename);

    /* ajStrTraceT(qry->Filename, "qry->Filename (before):"); */

    ajStrAssignS(&url, qry->Filename);

    urlref = ajHttpUrlrefNew();
    ajHttpUrlrefParseS(&urlref, url);
    ajHttpUrlrefSplitPort(urlref);
    ajStrAssignS(&host,urlref->Host);
    if(ajStrGetLen(urlref->Port))
        ajStrToInt(urlref->Port, &iport);
    ajFmtPrintS(&urlget,"/%S",urlref->Absolute);
    ajHttpUrlrefDel(&urlref);

    ajFilebuffDel(&textin->Filebuff);
    textin->Filebuff = ajFtpRead(NULL, host, iport, textin->Fpos, urlget);

    ajStrDel(&host);
    ajStrDel(&urlget);

    if(!textin->Filebuff)
    {
	ajDebug("FTP access: unable to open file '%S'\n", qry->Filename);

	return ajFalse;
    }

    /* ajStrTraceT(textin->Filename, "textin->Filename:"); */
    /* ajStrTraceT(qry->Filename, "qry->Filename (after):"); */

    ajStrAssignS(&textin->Filename, qry->Filename);

    ajDebug("FTP access: opened file '%S'\n", qry->Filename);

    ajStrDel(&url);

    return ajTrue;
}




/* @func ajTextinAccessHttp ***************************************************
**
** Reads data from an HTTP URL. No HTML is stripped.
**
** @param [u] textin [AjPTextin] Text input.
** @return [AjBool] ajTrue on success.
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ajTextinAccessHttp(AjPTextin textin)
{
    AjPQuery qry;
    AjPStr url = NULL;

    AjPStr host  = NULL;
    ajint iport;

    AjPStr urlget    = NULL;
    AjPUrlref urlref = NULL;
    AjPStr version10 = NULL;
    AjBool ok;

    iport = 80;
    qry = textin->Query;

    if(!ajStrGetLen(qry->Filename))
    {
	ajErr("HTTP access: no filename");

	return ajFalse;
    }

    ajDebug("ajTextinAccessHttp %S\n", qry->Filename);

    ajStrAssignS(&url, qry->Filename);

    urlref = ajHttpUrlrefNew();
    ajHttpUrlrefParseS(&urlref, url);
    ajHttpUrlrefSplitPort(urlref);
    ajStrAssignS(&host,urlref->Host);
    if(ajStrGetLen(urlref->Port))
        ajStrToInt(urlref->Port, &iport);
    ajFmtPrintS(&urlget,"/%S",urlref->Absolute);
    ajHttpUrlrefDel(&urlref);
   
    version10 = ajStrNewC("1.0");

    ajFilebuffDel(&textin->Filebuff);
    textin->Filebuff = ajHttpReadPos(version10, url,
                                     NULL, host, iport, urlget, qry->Fpos);
    ajStrDel(&version10);

    if(!textin->Filebuff)
    {
        if(iport == 80)
            ajErr("Cannot open HTTP connection 'http://%S%S'",
                  host, urlget);
        else
            ajErr("Cannot open HTTP connection 'http://%S:%d%S'",
                  host, iport, urlget);
        return ajFalse;
    }

    /* skip past the header */

    ok = ajBuffreadLine(textin->Filebuff, &textinReadLine);

    switch(ajStrGetCharPos(textinReadLine, 9))
    {
        case '4':
            return ajFalse;
        default:
            break;        
    }

    while(ok && ajStrFindRestC(textinReadLine, "\r\n") >= 0)
        ajBuffreadLine(textin->Filebuff, &textinReadLine);

    ajFilebuffClear(textin->Filebuff,1);



    ajStrDel(&host);
    ajStrDel(&urlget);

    if(!textin->Filebuff)
    {
	ajDebug("HTTP access: unable to open file '%S'\n", qry->Filename);

	return ajFalse;
    }

    /* ajStrTraceT(textin->Filename, "textin->Filename:"); */
    /* ajStrTraceT(qry->Filename, "qry->Filename (after):"); */

    ajStrAssignS(&textin->Filename, url);

    ajStrDel(&url);

    return ajTrue;
}




/* @func ajTextinAccessOffset *************************************************
**
** Reads a text from a named file, at a given offset within the file.
**
** @param [u] textin [AjPTextin] Text input.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajTextinAccessOffset(AjPTextin textin)
{
    AjPQuery qry;

    qry = textin->Query;

    if(!ajStrGetLen(qry->Filename))
    {
	ajErr("FILE access: no filename");

	return ajFalse;
    }

    ajDebug("ajTextinAccessOffset %S %Ld\n", qry->Filename, qry->Fpos);

    /* ajStrTraceT(qry->Filename, "qry->Filename (before):"); */

    ajFilebuffDel(&textin->Filebuff);
    textin->Filebuff = ajFilebuffNewNameS(qry->Filename);

    if(!textin->Filebuff)
    {
	ajDebug("OFFSET access: unable to open file '%S'\n", qry->Filename);

	return ajFalse;
    }

    ajFileSeek(ajFilebuffGetFile(textin->Filebuff), qry->Fpos, 0);
    /* ajStrTraceT(textin->Filename, "textin->Filename:"); */
    /* ajStrTraceT(qry->Filename, "qry->Filename (after):"); */
    ajStrAssignS(&textin->Filename, qry->Filename);

    return ajTrue;
}





/* @datasection [none] Miscellaneous ******************************************
**
** Text input internals
**
** @nam2rule Textin Text input
**
******************************************************************************/




/* @section Printing **********************************************************
**
** Printing details of the internals to a file
**
** @fdata [none]
**
** @nam2rule Textinprint
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




/* @func ajTextinprintBook ****************************************************
**
** Reports the internal data structures as a Docbook table
**
** @param [u] outf [AjPFile] Output file
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTextinprintBook(AjPFile outf)
{
    ajuint i = 0;
    ajuint j = 0;
    AjPStr namestr = NULL;
    AjPList fmtlist;
    AjPStr* names;

    fmtlist = ajListstrNew();

    ajFmtPrintF(outf, "<para>The supported text formats are summarised "
                "in the table below. "
                "The columns are as follows: "
                "<emphasis>Input format</emphasis> (format name), "
                "<emphasis>Try</emphasis> (indicates whether the "
                "format can be detected automatically on input), and "
                "<emphasis>Description</emphasis> (short description of "
                "the format).</para>\n\n");

    ajFmtPrintF(outf, "<table frame=\"box\" rules=\"cols\">\n");
    ajFmtPrintF(outf, "  <caption>Input text formats</caption>\n");
    ajFmtPrintF(outf, "  <thead>\n");
    ajFmtPrintF(outf, "    <tr align=\"center\">\n");
    ajFmtPrintF(outf, "      <th>Input Format</th>\n");
    ajFmtPrintF(outf, "      <th>Try</th>\n");
    ajFmtPrintF(outf, "      <th>Description</th>\n");
    ajFmtPrintF(outf, "    </tr>\n");
    ajFmtPrintF(outf, "  </thead>\n");
    ajFmtPrintF(outf, "  <tbody>\n");

    for(i=1; textinFormatDef[i].Name; i++)
    {
	if(!textinFormatDef[i].Alias)
        {
            namestr = ajStrNewC(textinFormatDef[i].Name);
            ajListPushAppend(fmtlist, namestr);
            namestr = NULL;
        }
    }

    ajListSort(fmtlist, &ajStrVcmp);
    ajListstrToarray(fmtlist, &names);

    for(i=0; names[i]; i++)
    {
        for(j=0; textinFormatDef[j].Name; j++)
        {
            if(ajStrMatchC(names[i],textinFormatDef[j].Name))
            {
                ajFmtPrintF(outf, "    <tr>\n");
                ajFmtPrintF(outf, "      <td>%s</td>\n",
                            textinFormatDef[j].Name);
                ajFmtPrintF(outf, "      <td>%B</td>\n",
                            textinFormatDef[j].Try);
                ajFmtPrintF(outf, "      <td>%s</td>\n",
                            textinFormatDef[j].Desc);
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




/* @func ajTextinprintHtml ****************************************************
**
** Reports the internal data structures as an HTML table
**
** @param [u] outf [AjPFile] Output file
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTextinprintHtml(AjPFile outf)
{
    ajuint i = 0;
    ajuint j = 0;

    AjPStr namestr = NULL;

    ajFmtPrintF(outf, "<table border=3>");
    ajFmtPrintF(outf, "<tr><th>Input Format</th><th>Auto</th>\n");
    ajFmtPrintF(outf, "<th>Multi</th><th>Description</th></tr>\n");

    for(i=1; textinFormatDef[i].Name; i++)
    {
        ajStrAssignC(&namestr, textinFormatDef[i].Name);

	if(!textinFormatDef[i].Alias)
        {
            for(j=i+1; textinFormatDef[j].Name; j++)
            {
                if(textinFormatDef[j].Read == textinFormatDef[i].Read)
                {
                    ajFmtPrintAppS(&namestr, " %s", textinFormatDef[j].Name);
                    if(!textinFormatDef[j].Alias) 
                    {
                        ajWarn("Input format '%s' same as '%s' but not alias",
                               textinFormatDef[j].Name,
                               textinFormatDef[i].Name);
                    }
                }
            }

	    ajFmtPrintF(outf, "<tr><td>\n%S\n</td><td>%B</td>\n",
                        namestr,
			textinFormatDef[i].Try);
            ajFmtPrintF(outf, "<td>\n%s\n</td></tr>\n",
			textinFormatDef[i].Desc);
        }

    }

    ajFmtPrintF(outf, "</table>\n");
    ajStrDel(&namestr);

    return;
}




/* @func ajTextinprintText ****************************************************
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

void ajTextinprintText(AjPFile outf, AjBool full)
{
    ajuint i = 0;

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# Text input formats\n");
    ajFmtPrintF(outf, "# Name  Format name (or alias)\n");
    ajFmtPrintF(outf, "# Alias Alias name\n");
    ajFmtPrintF(outf, "# Try   Test for unknown input files\n");
    ajFmtPrintF(outf, "# Name         Alias Try "
		"Description");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "InFormat {\n");

    for(i=0; textinFormatDef[i].Name; i++)
	if(full || !textinFormatDef[i].Alias)
	    ajFmtPrintF(outf,
			"  %-12s %5B %3B \"%s\"\n",
			textinFormatDef[i].Name,
			textinFormatDef[i].Alias,
			textinFormatDef[i].Try,
			textinFormatDef[i].Desc);

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajTextinprintWiki ****************************************************
**
** Reports the internal data structures as a wiki table
**
** @param [u] outf [AjPFile] Output file
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTextinprintWiki(AjPFile outf)
{
    ajuint i = 0;
    ajuint j = 0;

    AjPStr namestr = NULL;

    ajFmtPrintF(outf, "{| class=\"wikitable sortable\" border=\"2\"\n");
    ajFmtPrintF(outf, "|-\n");
    ajFmtPrintF(outf, "!Format!!Try!!"
                "class=\"unsortable\"|Description\n");

    for(i=1; textinFormatDef[i].Name; i++)
    {
        ajStrAssignC(&namestr, textinFormatDef[i].Name);

	if(!textinFormatDef[i].Alias)
        {
            for(j=i+1; textinFormatDef[j].Name; j++)
            {
                if(textinFormatDef[j].Read == textinFormatDef[i].Read)
                {
                    ajFmtPrintAppS(&namestr, "<br>%s",
                                   textinFormatDef[j].Name);
                    if(!textinFormatDef[j].Alias) 
                    {
                        ajWarn("Input format '%s' same as '%s' but not alias",
                               textinFormatDef[j].Name,
                               textinFormatDef[i].Name);
                    }
                }
            }

            ajFmtPrintF(outf, "|-\n");
	    ajFmtPrintF(outf,
			"|%S||%B||%s\n",
			namestr,
			textinFormatDef[i].Try,
			textinFormatDef[i].Desc);
        }

    }

    ajFmtPrintF(outf, "|}\n\n");
    ajStrDel(&namestr);

    return;
}




/* @datasection [none] Miscellaneous ******************************************
**
** Text internals
**
** @nam2rule Textin Text input
**
******************************************************************************/




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




/* @func ajTextinExit *********************************************************
**
** Cleans up text data input internal memory
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTextinExit(void)
{
    /* Query processing regular expressions */

    ajStrDel(&textinReadLine);

    ajTableDel(&textDbMethods);

    return;
}




/* @section Internals *********************************************************
**
** Functions to return internal values
**
** @fdata [none]
**
** @nam3rule Type Internals for text datatype
** @nam4rule Get  Return a value
** @nam5rule Fields  Known query fields for ajTextinRead
** @nam5rule Qlinks  Known query link operators for ajTextinRead
**
** @valrule * [const char*] Internal value
**
** @fcategory misc
**
******************************************************************************/




/* @func ajTextinTypeGetFields ************************************************
**
** Returns the listof known field names for ajTextinRead
**
** @return [const char*] List of field names
**
** @release 6.4.0
** @@
******************************************************************************/

const char* ajTextinTypeGetFields(void)
{
    return "id";
}




/* @func ajTextinTypeGetQlinks ************************************************
**
** Returns the listof known query link operators for ajTextinRead
**
** @return [const char*] List of field names
**
** @release 6.4.0
** @@
******************************************************************************/

const char* ajTextinTypeGetQlinks(void)
{
    return "|";
}




/* @datasection [AjPTable] Internal call register table ***********************
**
** Functions to manage the internal call register table that links the
** ajaxdb library functions with code in the core AJAX library.
**
** @nam2rule Textaccess Functions to manage textdb call tables.
**
******************************************************************************/




/* @section Cast **************************************************************
**
** Return a reference to the call table
**
** @fdata [AjPTable] textdb functions call table
**
** @nam3rule Get Return a value
** @nam4rule Db Database access functions table
** @nam3rule Method Lookup an access method by name
** @nam4rule Test Return true if the access method exists
** @nam4rule MethodGet Return a method value
** @nam5rule Qlinks Return query link operators
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




/* @func ajTextaccessGetDb ****************************************************
**
** Returns the table in which text database access details are registered
**
** @return [AjPTable] Access functions hash table
**
** @release 6.4.0
** @@
******************************************************************************/

AjPTable ajTextaccessGetDb(void)
{
    if(!textDbMethods)
        textDbMethods = ajCallTableNew();
    return textDbMethods;
    
}




/* @func ajTextaccessMethodGetQlinks ******************************************
**
** Tests for a named method for text file reading and returns the 
** known query link operators
**
** @param [r] method [const AjPStr] Method required.
** @return [const char*] Known link operators
**
** @release 6.4.0
** @@
******************************************************************************/

const char* ajTextaccessMethodGetQlinks(const AjPStr method)
{
    AjPTextAccess methoddata; 

    methoddata = ajCallTableGetS(textDbMethods, method);
    if(!methoddata)
        return NULL;

    return methoddata->Qlink;
}




/* @func ajTextaccessMethodGetScope *******************************************
**
** Tests for a named method for text file reading and returns the scope
** (entry, query or all).
*
** @param [r] method [const AjPStr] Method required.
** @return [ajuint] Scope flags
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ajTextaccessMethodGetScope(const AjPStr method)
{
    AjPTextAccess methoddata; 
    ajuint ret = 0;

    methoddata = ajCallTableGetS(textDbMethods, method);
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




/* @func ajTextaccessMethodTest ***********************************************
** Tests for a named method for text reading.
**
** @param [r] method [const AjPStr] Method required.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajTextaccessMethodTest(const AjPStr method)
{
    if(ajCallTableGetS(textDbMethods, method))
      return ajTrue;

    return ajFalse;
}




/* @funcstatic textinQryRestore ***********************************************
**
** Restores an text input specification from an AjPQueryList node
**
** @param [w] textin [AjPTextin] Text input object
** @param [r] node [const AjPQueryList] Query list node
** @return [void]
**
** @release 6.4.0
******************************************************************************/

static void textinQryRestore(AjPTextin textin, const AjPQueryList node)
{
    textin->Format = node->Format;
    textin->Fpos   = node->Fpos;
    ajStrAssignS(&textin->Formatstr, node->Formatstr);
    ajStrAssignS(&textin->QryFields, node->QryFields);

    return;
}




/* @funcstatic textinQrySave **************************************************
**
** Saves an text input specification in an AjPQueryList node
**
** @param [w] node [AjPQueryList] Query list node
** @param [r] textin [const AjPTextin] Text input object
** @return [void]
**
** @release 6.4.0
******************************************************************************/

static void textinQrySave(AjPQueryList node, const AjPTextin textin)
{
    node->Format   = textin->Format;
    node->Fpos     = textin->Fpos;
    ajStrAssignS(&node->Formatstr, textin->Formatstr);
    ajStrAssignS(&node->QryFields, textin->QryFields);

    return;
}




/* @funcstatic textinQryProcess ***********************************************
**
** Converts an text query into an open file.
**
** Tests for "format::" and sets this if it is found
**
** Then tests for "list:" or "@" and processes as a list file
** using textinListProcess which in turn invokes textinQryProcess
** until a valid query is found.
**
** Then tests for dbname:query and opens the file (at the correct position
** if the database definition defines it)
**
** If there is no database, looks for file:query and opens the file.
** In this case the file position is not known and text data reading
** will have to scan for the entry/entries we need.
**
** @param [u] textin [AjPTextin] text data input structure.
** @param [u] text [AjPText] text data to be read. The format will be replaced
**                         if defined in the query string.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textinQryProcess(AjPTextin textin, AjPText text)
{
    AjBool ret = ajTrue;
    AjPStr qrystr = NULL;
    AjBool nontextmethod = ajFalse;
    const AjPStr fmtstr = NULL;
    AjPQuery qry;

    qry = textin->Query;

    /* pick up the original query string */
    qrystr = ajStrNewS(textin->Qry);

    ajDebug("++textinQryProcess '%S' \n", qrystr);

    /* look for a format:: prefix */
    fmtstr = ajQuerystrParseFormat(&qrystr, textin, textinformatFind);
    ajDebug("textinQryProcess ... fmtstr '%S' '%S'\n", fmtstr, qrystr);

    /* (seq/feat only) DO NOT look for a [range] suffix */
    /* look for a list:: or @:: listfile of queries  - process and return */
    if(ajQuerystrParseListfile(&qrystr))
    {
        ajDebug("textinQryProcess ... listfile '%S'\n", qrystr);
        ret = textinListProcess(textin, text, qrystr);
        ajStrDel(&qrystr);
        return ret;
    }

    /* try general text access methods (file, asis, text database access */
    ajDebug("textinQryProcess ... no listfile '%S'\n", qrystr);
    if(!ajQuerystrParseRead(&qrystr, textin, textinformatFind, &nontextmethod))
    {
        ajStrDel(&qrystr);
        return ajFalse;
    }
    
    textinFormatSet(textin, text);

    ajDebug("textinQryProcess ... read nontext: %B '%S'\n",
            nontextmethod, qrystr);
    ajStrDel(&qrystr);

    if(nontextmethod)
    {
        ajDebug("textinQryProcess ... call method '%S'\n", qry->Method);
        ajDebug("textinQryProcess ... textin format %d '%S'\n",
                textin->Format, textin->Formatstr);
        ajDebug("textinQryProcess ...  query format  '%S'\n",
                qry->Formatstr);
/*
** skip this for text .... we already tried text access methods!
        qry->Access = ajCallTableGetS(xxxDbMethods,qry->Method);
        xxxaccess = qry->Access;
        return (*xxxaccess->Access)(oboin);
*/    }

    ajDebug("seqinUsaProcess text method '%S' success\n", qry->Method);

    return ajTrue;
}




/* @datasection [AjPList] Query field list ************************************
**
** Query fields lists are handled internally. Only static functions
** should appear here
**
******************************************************************************/




/* @funcstatic textinListProcess **********************************************
**
** Processes a file of queries.
** This function is called by, and calls, textinQryProcess. There is
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
** @param [u] textin [AjPTextin] text data input
** @param [u] text [AjPText] text data
** @param [r] listfile [const AjPStr] Name of list file.,
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textinListProcess(AjPTextin textin, AjPText text,
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
    ajDebug("++textListProcess %S depth %d\n",
	    listfile, depth);

    if(depth > MAXDEPTH)
	ajFatal("Query list too deep");

    if(!textin->List)
	textin->List = ajListNew();

    list = ajListNew();

    file = ajFileNewInNameS(listfile);

    if(!file)
    {
	ajErr("Failed to open list file '%S'", listfile);
	depth--;

	return ret;
    }

    while(ajReadlineTrim(file, &textinReadLine))
    {
	textinListNoComment(&textinReadLine);

	if(ajStrGetLen(textinReadLine))
	{
	    ajStrTokenAssignC(&handle, textinReadLine, " \t\n\r");
	    ajStrTokenNextParse(&handle, &token);
	    /* ajDebug("Line  '%S'\n");*/
	    /* ajDebug("token '%S'\n", textinReadLine, token); */

	    if(ajStrGetLen(token))
	    {
	        ajDebug("++Add to list: '%S'\n", token);
	        AJNEW0(node);
	        ajStrAssignS(&node->Qry, token);
	        textinQrySave(node, textin);
	        ajListPushAppend(list, node);
	    }

	    ajStrDel(&token);
	    token = NULL;
	}
    }

    ajFileClose(&file);
    ajStrDel(&token);

    ajDebug("Trace textin->List\n");
    ajQuerylistTrace(textin->List);
    ajDebug("Trace new list\n");
    ajQuerylistTrace(list);
    ajListPushlist(textin->List, &list);

    ajDebug("Trace combined textin->List\n");
    ajQuerylistTrace(textin->List);

    /*
     ** now try the first item on the list
     ** this can descend recursively if it is also a list
     ** which is why we check the depth above
     */

    if(ajListPop(textin->List, (void**) &node))
    {
        ajDebug("++pop first item '%S'\n", node->Qry);
	ajTextinQryS(textin, node->Qry);
	textinQryRestore(textin, node);
	ajStrDel(&node->Qry);
	ajStrDel(&node->Formatstr);
	AJFREE(node);
	ajDebug("descending with query '%S'\n", textin->Qry);
	ret = textinQryProcess(textin, text);
    }

    ajStrTokenDel(&handle);
    depth--;
    ajDebug("++textListProcess depth: %d returns: %B\n", depth, ret);

    return ret;
}




/* @funcstatic textinListNoComment ********************************************
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

static void textinListNoComment(AjPStr* text)
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




/* @funcstatic textinFormatSet ************************************************
**
** Sets the input format for text data using the text data input object's
** defined format
**
** @param [u] textin [AjPTextin] text term input.
** @param [u] text [AjPText] text term.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textinFormatSet(AjPTextin textin, AjPText text)
{

    if(ajStrGetLen(textin->Formatstr))
    {
	ajDebug("... input format value '%S'\n", textin->Formatstr);

	if(textinformatFind(textin->Formatstr, &textin->Format))
	{
	    ajStrAssignS(&text->Formatstr, textin->Formatstr);
	    text->Format = textin->Format;
	    ajDebug("...format OK '%S' = %d\n", textin->Formatstr,
		    textin->Format);
	}
	else
	    ajDebug("...format unknown '%S'\n", textin->Formatstr);

	return ajTrue;
    }
    else
	ajDebug("...input format not set\n");


    return ajFalse;
}




/* @datasection [AjPTextall] Text Input Stream ********************************
**
** Function is for manipulating text block input stream objects
**
** @nam2rule Textall Text input stream objects
**
******************************************************************************/




/* @section Text Input Constructors *******************************************
**
** All constructors return a new text input stream object by pointer. It
** is the responsibility of the user to first destroy any previous
** text input object. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so
** anyway.
**
** @fdata [AjPTextall]
**
** @nam3rule New Constructor
**
** @valrule * [AjPTextall] Text input stream object
**
** @fcategory new
**
******************************************************************************/




/* @func ajTextallNew *********************************************************
**
** Creates a new text input stream object.
**
** @return [AjPTextall] New text input stream object.
**
** @release 6.4.0
** @@
******************************************************************************/

AjPTextall ajTextallNew(void)
{
    AjPTextall pthis;

    AJNEW0(pthis);

    pthis->Textin = ajTextinNew();
    pthis->Text   = ajTextNew();

    return pthis;
}





/* ==================================================================== */
/* ========================== destructors ============================= */
/* ==================================================================== */




/* @section Text Input Stream Destructors *************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the text input stream object.
**
** @fdata [AjPTextall]
**
** @nam3rule Del Destructor
**
** @argrule Del pthis [AjPTextall*] Text input stream
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajTextallDel *********************************************************
**
** Deletes a text input stream object.
**
** @param [d] pthis [AjPTextall*] Text input stream
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTextallDel(AjPTextall* pthis)
{
    AjPTextall thys;

    if(!pthis)
        return;

    thys = *pthis;

    if(!thys)
        return;

    ajTextinDel(&thys->Textin);
    if(!thys->Returned)
        ajTextDel(&thys->Text);

    AJFREE(*pthis);

    return;
}




/* ==================================================================== */
/* =========================== Modifiers ============================== */
/* ==================================================================== */




/* @section Text input stream modifiers ***************************************
**
** These functions use the contents of a text input stream object and
** update them.
**
** @fdata [AjPTextall]
**
** @nam3rule Clear Clear all values
**
** @argrule * thys [AjPTextall] Text input stream object
**
** @valrule * [void]
**
** @fcategory modify
**
******************************************************************************/




/* @func ajTextallClear *******************************************************
**
** Clears a text input stream object back to "as new" condition, except
** for the query list which must be preserved.
**
** @param [w] thys [AjPTextall] Text input stream
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTextallClear(AjPTextall thys)
{

    ajDebug("ajTextallClear called\n");

    if(!thys)
        return;

    ajTextinClear(thys->Textin);

    ajTextClear(thys->Text);

    thys->Returned = ajFalse;

    return;
}




/* @section Text input ********************************************************
**
** These functions use a text input stream object to read data
**
** @fdata [AjPTextall]
**
** @nam3rule Next Read next text block
**
** @argrule * thys [AjPTextall] Text input stream object
** @argrule * Ptext [AjPText*] Text object
**
** @valrule * [AjBool] True on success
**
** @fcategory input
**
******************************************************************************/




/* @func ajTextallNext ********************************************************
**
** Parse a text query into format, access, file and entry
**
** Split at delimiters. Check for the first part as a valid format
** Check for the remaining first part as a database name or as a file
** that can be opened.
** Anything left is an entryname spec.
**
** Return the results in the AjPText object but leave the file open for
** future calls.
**
** @param [w] thys [AjPTextall] Text input stream
** @param [u] Ptext [AjPText*] Text block returned
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajTextallNext(AjPTextall thys, AjPText *Ptext)
{
    if(!thys->Count)
    {
	thys->Count = 1;

	thys->Totterms++;

	*Ptext = thys->Text;
	thys->Returned = ajTrue;

	return ajTrue;
    }


    if(ajTextinRead(thys->Textin, thys->Text))
    {
	thys->Count++;

	thys->Totterms++;

	*Ptext = thys->Text;
	thys->Returned = ajTrue;

	ajDebug("ajTextallNext success\n");

	return ajTrue;
    }

    *Ptext = NULL;

    ajDebug("ajTextallNext failed\n");

    ajTextallClear(thys);

    return ajFalse;
}




/* @datasection [none] Input formats ******************************************
**
** Input formats internals
**
** @nam2rule Textinformat Text data input format specific
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
** @argrule Find iformat [ajuint*] Index matching format name
**
** @valrule * [AjBool] True if found
**
** @fcategory cast
**
******************************************************************************/




/* @funcstatic textinformatFind ***********************************************
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

static AjBool textinformatFind(const AjPStr format, ajint* iformat)
{
    AjPStr tmpformat = NULL;
    ajuint i = 0;

    ajDebug("textinformatFind '%S'\n", format);
    if(!ajStrGetLen(format))
	return ajFalse;

    ajStrAssignS(&tmpformat, format);
    ajStrFmtLower(&tmpformat);

    for(i=0; textinFormatDef[i].Name; i++)
    {
	ajDebug("test %d '%s' '%s' '%s'\n",
                i, textinFormatDef[i].Name,
                textinFormatDef[i].Obo,
                textinFormatDef[i].Desc);
	if(ajStrMatchC(tmpformat, textinFormatDef[i].Name) ||
           ajStrMatchC(format, textinFormatDef[i].Obo))
	{
	    *iformat = i;
	    ajStrDel(&tmpformat);
	    ajDebug("found '%s' at %d\n", textinFormatDef[i].Name, i);
	    return ajTrue;
	}
    }

    ajStrDel(&tmpformat);

    return ajFalse;
}




/* @func ajTextinformatTerm ***************************************************
**
** tests whether a text input format term is known
**
** @param [r] term [const AjPStr] Format term EDAM ID
** @return [AjBool] ajTrue if term was accepted
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajTextinformatTerm(const AjPStr term)
{
    ajuint i;

    for(i=0; textinFormatDef[i].Name; i++)
	if(ajStrMatchC(term, textinFormatDef[i].Obo))
	    return ajTrue;

    return ajFalse;
}




/* @func ajTextinformatTest ***************************************************
**
** tests whether a named text input format is known
**
** @param [r] format [const AjPStr] Format
** @return [AjBool] ajTrue if format was accepted
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajTextinformatTest(const AjPStr format)
{
    ajuint i;

    for(i=0; textinFormatDef[i].Name; i++)
    {
	if(ajStrMatchCaseC(format, textinFormatDef[i].Name))
	    return ajTrue;
	if(ajStrMatchC(format, textinFormatDef[i].Obo))
	    return ajTrue;
    }

    return ajFalse;
}

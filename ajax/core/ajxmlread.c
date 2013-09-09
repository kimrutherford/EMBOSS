/* @source ajxmlread *********************************************************
**
** AJAX xml reading functions
**
** These functions control all aspects of AJAX xmldec reading
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.2 $
** @modified Oct 5 pmr First version
** @modified $Date: 2012/10/25 13:17:28 $ by $Author: rice $
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

#include "ajxmlread.h"
#include "ajxml.h"
#include "ajcall.h"
#include "ajlist.h"
#include "ajquery.h"
#include "ajtextread.h"
#include "ajnam.h"
#include "ajfileio.h"

#include <string.h>


AjPTable xmlDbMethods = NULL;

static AjPStr xmlinReadLine     = NULL;


static AjBool xmlinReadXml(AjPXmlin thys, AjPXml xml);




/* @datastatic XmlPInFormat ***************************************************
**
** Xml input formats data structure
**
** @alias XmlSInFormat
** @alias XmlOInFormat
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

typedef struct XmlSInFormat
{
    const char *Name;
    const char *Obo;
    const char *Desc;
    AjBool Alias;
    AjBool Try;
    AjBool (*Read) (AjPXmlin thys, AjPXml xml);
} XmlOInFormat;

#define XmlPInFormat XmlOInFormat*

static XmlOInFormat xmlinFormatDef[] =
{
/* "Name",        "OBOterm", "Description" */
/*     Alias,   Try,     */
/*     ReadFunction */
  {"unknown",     "0000", "Unknown format",
       AJFALSE, AJFALSE,
       &xmlinReadXml}, /* default to first format */
  {"xml",          "2332", "General XML format",
       AJFALSE, AJTRUE,
       &xmlinReadXml},
  {"emblxml",      "2332", "EMBL XML format",
       AJTRUE, AJFALSE,
       &xmlinReadXml},
  {"emblxml-1.2",  "2332", "EMBL XML format 1.2",
       AJTRUE, AJFALSE,
       &xmlinReadXml},
  {"insdxml",      "2332", "International nucleotide sequence XML format",
       AJTRUE, AJFALSE,
       &xmlinReadXml},
  {"seqxml",       "2332", "Sequence XML format",
       AJTRUE, AJFALSE,
       &xmlinReadXml},
  {"medlinexml",   "0000", "Medline XML format",
       AJTRUE, AJFALSE,
       &xmlinReadXml},
  {"modsxml",      "0000", "Medline MODS XML format",
       AJTRUE, AJFALSE,
       &xmlinReadXml},
  {"wordbibxml",   "0000", "WordBib XML format",
       AJTRUE, AJFALSE,
       &xmlinReadXml},
  {"iprmcxml",     "0000", "IPRMC XML format",
       AJTRUE, AJFALSE,
       &xmlinReadXml},
  {"enataxonomyxml", "0000", "ENA taxonomy XML format",
       AJTRUE, AJFALSE,
       &xmlinReadXml},
  {"uniprotxml",   "0000", "Uniprot XML data",
       AJTRUE, AJFALSE,
       &xmlinReadXml},
  {"uniprotrdfxml", "0000", "Uniprot RDF XML data",
       AJTRUE, AJFALSE,
       &xmlinReadXml},
  {"uniprottaxonomyrdfxml",   "0000", "Uniprot taxonomy RDF XML data",
       AJTRUE, AJFALSE,
       &xmlinReadXml},
  {"interproxml",  "0000", "Interpro XML data",
       AJTRUE, AJFALSE,
       &xmlinReadXml},
  {"sgtxml",     "0000", "SGT XML data",
       AJTRUE, AJFALSE,
       &xmlinReadXml},
  {"pdbml",     "0000", "PDBML data",
       AJTRUE, AJFALSE,
       &xmlinReadXml},
  {"tracexml",     "0000", "Sequence trace XML data",
       AJTRUE, AJFALSE,
       &xmlinReadXml},
  {NULL, NULL, NULL, 0, 0, NULL}
};



static ajuint xmlinReadFmt(AjPXmlin xmlin, AjPXml xml,
                           ajuint format);
static AjBool xmlinRead(AjPXmlin xmlin, AjPXml xml);
static AjBool xmlinformatFind(const AjPStr format, ajint* iformat);
static AjBool xmlinFormatSet(AjPXmlin xmlin, AjPXml xml);
static AjBool xmlinListProcess(AjPXmlin xmlin, AjPXml xml,
                               const AjPStr listfile);
static void xmlinListNoComment(AjPStr* text);
static void xmlinQryRestore(AjPXmlin xmlin, const AjPQueryList node);
static void xmlinQrySave(AjPQueryList node, const AjPXmlin xmlin);
static AjBool xmlDefine(AjPXml thys, AjPXmlin xmlin);
static AjBool xmlinQryProcess(AjPXmlin xmlin, AjPXml xml);
static AjBool xmlinQueryMatch(const AjPQuery thys, const AjPXml xml);




/* @filesection ajxmlread ****************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
*/




/* @datasection [AjPXmlin] Xml input objects ***************************
**
** Function is for manipulating xml input objects
**
** @nam2rule Xmlin
******************************************************************************/




/* @section Xml input constructors ***************************************
**
** All constructors return a new xml input object by pointer. It
** is the responsibility of the user to first destroy any previous
** xml input object. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so
** anyway.
**
** @fdata [AjPXmlin]
**
** @nam3rule  New     Construct a new xml input object
**
** @valrule   *  [AjPXmlin] New xml input object
**
** @fcategory new
**
******************************************************************************/




/* @func ajXmlinNew **********************************************************
**
** Creates a new xml input object.
**
** @return [AjPXmlin] New xml input object.
** @category new [AjPXmlin] Default constructor
** @@
******************************************************************************/

AjPXmlin ajXmlinNew(void)
{
    AjPXmlin pthis;

    AJNEW0(pthis);

    pthis->Input = ajTextinNewDatatype(AJDATATYPE_XML);

    pthis->XmlData      = NULL;

    return pthis;
}





/* @section xml input destructors ****************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the xml input object.
**
** @fdata [AjPXmlin]
**
** @nam3rule Del Destructor
**
** @argrule Del pthis [AjPXmlin*] Xml input object
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajXmlinDel **********************************************************
**
** Deletes a xml input object.
**
** @param [d] pthis [AjPXmlin*] Xml input
** @return [void]
** @category delete [AjPXmlin] Default destructor
** @@
******************************************************************************/

void ajXmlinDel(AjPXmlin* pthis)
{
    AjPXmlin thys;

    if(!pthis)
        return;

    thys = *pthis;

    if(!thys)
        return;

    ajDebug("ajXmlinDel called qry:'%S'\n", thys->Input->Qry);

    ajTextinDel(&thys->Input);

    AJFREE(*pthis);

    return;
}




/* @section xml input modifiers ***********************************************
**
** These functions use the contents of a xml input object and
** update them.
**
** @fdata [AjPXmlin]
**
** @nam3rule Clear Clear all values
** @nam3rule Qry Reset using a query string
** @suffix C Character string input
** @suffix S String input
**
** @argrule * thys [AjPXmlin] Xml input object
** @argrule C txt [const char*] Query text
** @argrule S str [const AjPStr] query string
**
** @valrule * [void]
**
** @fcategory modify
**
******************************************************************************/




/* @func ajXmlinClear ********************************************************
**
** Clears a xml input object back to "as new" condition, except
** for the query list which must be preserved.
**
** @param [w] thys [AjPXmlin] Xml input
** @return [void]
** @category modify [AjPXmlin] Resets ready for reuse.
** @@
******************************************************************************/

void ajXmlinClear(AjPXmlin thys)
{

    ajDebug("ajXmlinClear called\n");

    ajTextinClear(thys->Input);

    thys->XmlData = NULL;

    return;
}




/* @func ajXmlinQryC *********************************************************
**
** Resets a xml input object using a new Universal
** Query Address
**
** @param [u] thys [AjPXmlin] Xml input object.
** @param [r] txt [const char*] Query
** @return [void]
** @@
******************************************************************************/

void ajXmlinQryC(AjPXmlin thys, const char* txt)
{
    ajXmlinClear(thys);
    ajStrAssignC(&thys->Input->Qry, txt);

    return;
}





/* @func ajXmlinQryS ********************************************************
**
** Resets a xml input object using a new Universal
** Query Address
**
** @param [u] thys [AjPXmlin] Xml input object.
** @param [r] str [const AjPStr] Query
** @return [void]
** @@
******************************************************************************/

void ajXmlinQryS(AjPXmlin thys, const AjPStr str)
{
    ajXmlinClear(thys);
    ajStrAssignS(&thys->Input->Qry, str);

    return;
}




/* @section casts *************************************************************
**
** Return values
**
** @fdata [AjPXmlin]
**
** @nam3rule Trace Write debugging output
**
** @argrule * thys [const AjPXmlin] Xml input object
**
** @valrule * [void]
**
** @fcategory cast
**
******************************************************************************/




/* @func ajXmlinTrace ********************************************************
**
** Debug calls to trace the data in a xml input object.
**
** @param [r] thys [const AjPXmlin] Xml input object.
** @return [void]
** @@
******************************************************************************/

void ajXmlinTrace(const AjPXmlin thys)
{
    ajDebug("xml input trace\n");
    ajDebug("====================\n\n");

    ajTextinTrace(thys->Input);

    if(thys->XmlData)
	ajDebug( "  XmlData: exists\n");

    return;
}




/* @section Xml data inputs **********************************************
**
** These functions read the wxyxdesc data provided by the first argument
**
** @fdata [AjPXmlin]
**
** @nam3rule Read Read xml data
**
** @argrule Read xmlin [AjPXmlin] Xml input object
** @argrule Read xml [AjPXml] Xml data
**
** @valrule * [AjBool] true on success
**
** @fcategory input
**
******************************************************************************/




/* @func ajXmlinRead ********************************************************
**
** If the file is not yet open, calls xmlinQryProcess to convert the query
** into an open file stream.
**
** Uses xmlinRead for the actual file reading.
**
** Returns the results in the AjPXml object.
**
** @param [u] xmlin [AjPXmlin] Xml data input definitions
** @param [w] xml [AjPXml] Xml data returned.
** @return [AjBool] ajTrue on success.
** @category input [AjPXml] Master xml data input,
**                  calls specific functions for file access type
**                  and xml data format.
** @@
******************************************************************************/

AjBool ajXmlinRead(AjPXmlin xmlin, AjPXml xml)
{
    AjBool ret       = ajFalse;
    AjPQueryList node = NULL;
    AjBool listdata  = ajFalse;

    if(xmlin->Input->Filebuff)
    {
	/* (a) if file still open, keep reading */
	ajDebug("ajXmlinRead: input file '%F' still there, try again\n",
		xmlin->Input->Filebuff->File);
	ret = xmlinRead(xmlin, xml);
	ajDebug("ajXmlinRead: open buffer  qry: '%S' returns: %B\n",
		xmlin->Input->Qry, ret);
    }
    else
    {
	/* (b) if we have a list, try the next query in the list */
	if(ajListGetLength(xmlin->Input->List))
	{
	    listdata = ajTrue;
	    ajListPop(xmlin->Input->List, (void**) &node);

	    ajDebug("++pop from list '%S'\n", node->Qry);
	    ajXmlinQryS(xmlin, node->Qry);
	    ajDebug("++SAVE XMLIN '%S' '%S' %d\n",
		    xmlin->Input->Qry,
		    xmlin->Input->Formatstr, xmlin->Input->Format);

            xmlinQryRestore(xmlin, node);

	    ajStrDel(&node->Qry);
	    ajStrDel(&node->Formatstr);
	    AJFREE(node);

	    ajDebug("ajXmlinRead: open list, try '%S'\n",
                    xmlin->Input->Qry);

	    if(!xmlinQryProcess(xmlin, xml) &&
               !ajListGetLength(xmlin->Input->List))
		return ajFalse;

	    ret = xmlinRead(xmlin, xml);
	    ajDebug("ajXmlinRead: list qry: '%S' returns: %B\n",
		    xmlin->Input->Qry, ret);
	}
	else
	{
	    ajDebug("ajXmlinRead: no file yet - test query '%S'\n",
                    xmlin->Input->Qry);

	    /* (c) Must be a query - decode it */
	    if(!xmlinQryProcess(xmlin, xml) &&
               !ajListGetLength(xmlin->Input->List))
		return ajFalse;

	    if(ajListGetLength(xmlin->Input->List)) /* could be a new list */
		listdata = ajTrue;

	    ret = xmlinRead(xmlin, xml);
	    ajDebug("ajXmlinRead: new qry: '%S' returns: %B\n",
		    xmlin->Input->Qry, ret);
	}
    }

    /* Now read whatever we got */

    while(!ret && ajListGetLength(xmlin->Input->List))
    {
	/* Failed, but we have a list still - keep trying it */
        if(listdata)
	    ajErr("Failed to read xml data '%S'",
                  xmlin->Input->Qry);

	listdata = ajTrue;
	ajListPop(xmlin->Input->List,(void**) &node);
	ajDebug("++try again: pop from list '%S'\n", node->Qry);
	ajXmlinQryS(xmlin, node->Qry);
	ajDebug("++SAVE (AGAIN) XMLIN '%S' '%S' %d\n",
		xmlin->Input->Qry,
		xmlin->Input->Formatstr, xmlin->Input->Format);

	xmlinQryRestore(xmlin, node);

	ajStrDel(&node->Qry);
	ajStrDel(&node->Formatstr);
	AJFREE(node);

	if(!xmlinQryProcess(xmlin, xml))
	    continue;

	ret = xmlinRead(xmlin, xml);
	ajDebug("ajXmlinRead: list retry qry: '%S' returns: %B\n",
		xmlin->Input->Qry, ret);
    }

    if(!ret)
    {
	if(listdata)
	    ajErr("Failed to read xml data '%S'",
                  xmlin->Input->Qry);

	return ajFalse;
    }


    xmlDefine(xml, xmlin);

    return ajTrue;
}




/* @funcstatic xmlinQueryMatch ***********************************************
**
** Compares a xml data item to a query and returns true if they match.
**
** @param [r] thys [const AjPQuery] query.
** @param [r] xml [const AjPXml] Xml data.
** @return [AjBool] ajTrue if the xml data matches the query.
** @@
******************************************************************************/

static AjBool xmlinQueryMatch(const AjPQuery thys, const AjPXml xml)
{
    AjBool tested = ajFalse;
    AjIList iterfield  = NULL;
    AjPQueryField field = NULL;
    AjBool ok = ajFalse;

    ajDebug("xmlinQueryMatch '%S' fields: %u Case %B Done %B\n",
	    xml->Id, ajListGetLength(thys->QueryFields),
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
                    xml->Id);
            if(thys->CaseId)
            {
                if(ajStrMatchWildS(xml->Id, field->Wildquery))
                {
                    ajListIterDel(&iterfield);
                    return ajTrue;
                }
            }
            else
            {
                if(ajStrMatchWildCaseS(xml->Id, field->Wildquery))
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
            if(ajStrMatchWildCaseS(xml->Id, field->Wildquery))
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




/* @funcstatic xmlDefine ****************************************************
**
** Make sure all xml data object attributes are defined
** using values from the xml input object if needed
**
** @param [w] thys [AjPXml] Xml data returned.
** @param [u] xmlin [AjPXmlin] Xml data input definitions
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool xmlDefine(AjPXml thys, AjPXmlin xmlin)
{

    /* if values are missing in the xml object, we can use defaults
       from xmlin or calculate where possible */

    /* assign the dbname if defined in the xmlin object */
    if(ajStrGetLen(xmlin->Input->Db))
      ajStrAssignS(&thys->Db, xmlin->Input->Db);

    return ajTrue;
}





/* @funcstatic xmlinReadFmt *************************************************
**
** Tests whether xml data can be read using the specified format.
** Then tests whether the xml data matches xml data query criteria
** and checks any specified type. Applies upper and lower case.
**
** @param [u] xmlin [AjPXmlin] Xml data input object
** @param [w] xml [AjPXml] Xml data object
** @param [r] format [ajuint] input format code
** @return [ajuint] 0 if successful.
**                  1 if the query match failed.
**                  2 if the xml data type failed
**                  3 if it failed to read any xml data
** @@
** This is the only function that calls the appropriate Read function
** xmlinReadXxxxxx where Xxxxxxx is the supported xml data format.
**
** Some of the xmlReadXxxxxx functions fail to reset the buffer correctly,
** which is a very serious problem when cycling through all of them to
** identify an unknown format. The extra ajFileBuffReset call at the end is
** intended to address this problem. The individual functions should still
** reset the buffer in case they are called from elsewhere.
**
******************************************************************************/

static ajuint xmlinReadFmt(AjPXmlin xmlin, AjPXml xml,
                           ajuint format)
{
    ajDebug("++xmlinReadFmt format %d (%s) '%S'\n",
	    format, xmlinFormatDef[format].Name,
	    xmlin->Input->Qry);

    xmlin->Input->Records = 0;

    /* Calling funclist xmlinFormatDef() */
    if((*xmlinFormatDef[format].Read)(xmlin, xml))
    {
	ajDebug("xmlinReadFmt success with format %d (%s)\n",
		format, xmlinFormatDef[format].Name);
        ajDebug("id: '%S'\n",
                xml->Id);
	xmlin->Input->Format = format;
	ajStrAssignC(&xmlin->Input->Formatstr,
	             xmlinFormatDef[format].Name);
	ajStrAssignC(&xml->Formatstr,
	             xmlinFormatDef[format].Name);
	ajStrAssignEmptyS(&xml->Db, xmlin->Input->Db);
	ajStrAssignS(&xml->Filename, xmlin->Input->Filename);

	if(xmlinQueryMatch(xmlin->Input->Query, xml))
	{
            /* ajXmlinTrace(xmlin); */

            return FMT_OK;
        }

	ajDebug("query match failed, continuing ...\n");
	ajXmlClear(xml);

	return FMT_NOMATCH;
    }
    else
    {
	ajDebug("Testing input buffer: IsBuff: %B Eof: %B\n",
		ajFilebuffIsBuffered(xmlin->Input->Filebuff),
		ajFilebuffIsEof(xmlin->Input->Filebuff));

	if (!ajFilebuffIsBuffered(xmlin->Input->Filebuff) &&
	    ajFilebuffIsEof(xmlin->Input->Filebuff))
	    return FMT_EOF;

	ajFilebuffReset(xmlin->Input->Filebuff);
	ajDebug("Format %d (%s) failed, file buffer reset by xmlinReadFmt\n",
		format, xmlinFormatDef[format].Name);
	/* ajFilebuffTraceFull(xmlin->Filebuff, 10, 10);*/
    }

    ajDebug("++xmlinReadFmt failed - nothing read\n");

    return FMT_FAIL;
}




/* @funcstatic xmlinRead *****************************************************
**
** Given data in a xmlin structure, tries to read everything needed
** using the specified format or by trial and error.
**
** @param [u] xmlin [AjPXmlin] Xml data input object
** @param [w] xml [AjPXml] Xml data object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool xmlinRead(AjPXmlin xmlin, AjPXml xml)
{
    ajuint i;
    ajuint istat = 0;
    ajuint jstat = 0;

    AjPFilebuff buff = xmlin->Input->Filebuff;
    /*AjBool regfile = ajFalse;*/
    AjBool ok;

    AjPTextAccess  textaccess  = xmlin->Input->Query->TextAccess;
    AjPXmlAccess xmlaccess = xmlin->Input->Query->Access;

    ajXmlClear(xml);
    ajDebug("xmlinRead: cleared\n");

    if(xmlin->Input->Single && xmlin->Input->Count)
    {
	/*
	** One xml data item at a time is read.
	** The first xml data item was read by ACD
	** for the following ones we need to reset the AjPXmlin
	**
	** Single is set by the access method
	*/

	ajDebug("xmlinRead: single access - count %d - call access"
		" routine again\n",
		xmlin->Input->Count);
	/* Calling funclist xmlinAccess() */
	if(textaccess)
        {
            if(!(*textaccess->Access)(xmlin->Input))
            {
                ajDebug("xmlinRead: (*textaccess->Access)(xmlin->Input) "
                        "*failed*\n");

                return ajFalse;
            }
        }

	if(xmlaccess)
        {
            if(!(*xmlaccess->Access)(xmlin))
            {
                ajDebug("xmlinRead: (*xmlaccess->Access)(xmlin) "
                        "*failed*\n");

                return ajFalse;
            }
        }

        buff = xmlin->Input->Filebuff;
    }

    ajDebug("xmlinRead: xmlin format %d '%S'\n", xmlin->Input->Format,
	    xmlin->Input->Formatstr);

    xmlin->Input->Count++;

    if(!xmlin->Input->Filebuff)
	return ajFalse;

    ok = ajFilebuffIsBuffered(xmlin->Input->Filebuff);

    while(ok)
    {				/* skip blank lines */
        ok = ajBuffreadLine(xmlin->Input->Filebuff, &xmlinReadLine);

        if(!ajStrIsWhite(xmlinReadLine))
        {
            ajFilebuffClear(xmlin->Input->Filebuff,1);
            break;
        }
    }

    if(!xmlin->Input->Format)
    {			   /* no format specified, try all defaults */
        /*regfile = ajFileIsFile(ajFilebuffGetFile(xmlin->Input->Filebuff));*/

	for(i = 1; xmlinFormatDef[i].Name; i++)
	{
	    if(!xmlinFormatDef[i].Try)	/* skip if Try is ajFalse */
		continue;

	    ajDebug("xmlinRead:try format %d (%s)\n",
		    i, xmlinFormatDef[i].Name);

	    istat = xmlinReadFmt(xmlin, xml, i);

	    switch(istat)
	    {
	    case FMT_OK:
		ajDebug("++xmlinRead OK, set format %d\n",
                        xmlin->Input->Format);
		xmlDefine(xml, xmlin);

		return ajTrue;
	    case FMT_BADTYPE:
		ajDebug("xmlinRead: (a1) "
                        "xmlinReadFmt stat == BADTYPE *failed*\n");

		return ajFalse;
	    case FMT_FAIL:
		ajDebug("xmlinRead: (b1) "
                        "xmlinReadFmt stat == FAIL *failed*\n");
		break;			/* we can try next format */
	    case FMT_NOMATCH:
		ajDebug("xmlinRead: (c1) "
                        "xmlinReadFmt stat==NOMATCH try again\n");
		break;
	    case FMT_EOF:
		ajDebug("xmlinRead: (d1) "
                        "xmlinReadFmt stat == EOF *failed*\n");
		return ajFalse;			/* EOF and unbuffered */
	    case FMT_EMPTY:
		ajWarn("xml data '%S' has zero length, ignored",
		       ajXmlGetQryS(xml));
		ajDebug("xmlinRead: (e1) "
                        "xmlinReadFmt stat==EMPTY try again\n");
		break;
	    default:
		ajDebug("unknown code %d from xmlinReadFmt\n", stat);
	    }

	    ajXmlClear(xml);

	    if(xmlin->Input->Format)
		break;			/* we read something */

            ajFilebuffTrace(xmlin->Input->Filebuff);
	}

	if(!xmlin->Input->Format)
	{		     /* all default formats failed, give up */
	    ajDebug("xmlinRead:all default formats failed, give up\n");

	    return ajFalse;
	}

	ajDebug("++xmlinRead set format %d\n",
                xmlin->Input->Format);
    }
    else
    {					/* one format specified */
	ajDebug("xmlinRead: one format specified\n");
	ajFilebuffSetUnbuffered(xmlin->Input->Filebuff);

	ajDebug("++xmlinRead known format %d\n",
                xmlin->Input->Format);
	istat = xmlinReadFmt(xmlin, xml, xmlin->Input->Format);

	switch(istat)
	{
	case FMT_OK:
	    xmlDefine(xml, xmlin);

	    return ajTrue;
	case FMT_BADTYPE:
	    ajDebug("xmlinRead: (a2) "
                    "xmlinReadFmt stat == BADTYPE *failed*\n");

	    return ajFalse;

        case FMT_FAIL:
	    ajDebug("xmlinRead: (b2) "
                    "xmlinReadFmt stat == FAIL *failed*\n");

	    return ajFalse;

        case FMT_NOMATCH:
	    ajDebug("xmlinRead: (c2) "
                    "xmlinReadFmt stat == NOMATCH *try again*\n");
	    break;
	case FMT_EOF:
	    ajDebug("xmlinRead: (d2) "
                    "xmlinReadFmt stat == EOF *try again*\n");
            if(xmlin->Input->Records)
                ajErr("Error reading file '%F' with format '%s': "
                      "end-of-file before end of data "
                      "(read %u records)",
                      ajFilebuffGetFile(xmlin->Input->Filebuff),
                      xmlinFormatDef[xmlin->Input->Format].Name,
                      xmlin->Input->Records);
	    break;		     /* simply end-of-file */
	case FMT_EMPTY:
	    ajWarn("assmebly data '%S' has zero length, ignored",
		   ajXmlGetQryS(xml));
	    ajDebug("xmlinRead: (e2) "
                    "xmlinReadFmt stat == EMPTY *try again*\n");
	    break;
	default:
	    ajDebug("unknown code %d from xmlinReadFmt\n", stat);
	}

	ajXmlClear(xml); /* 1 : read, failed to match id/acc/query */
    }

    /* failed - probably entry/accession query failed. Can we try again? */

    ajDebug("xmlinRead failed - try again with format %d '%s' code %d\n",
	    xmlin->Input->Format,
            xmlinFormatDef[xmlin->Input->Format].Name, istat);

    ajDebug("Search:%B Chunk:%B Data:%x ajFileBuffEmpty:%B\n",
	    xmlin->Input->Search, xmlin->Input->ChunkEntries,
            xmlin->Input->TextData, ajFilebuffIsEmpty(buff));

    if(ajFilebuffIsEmpty(buff) && xmlin->Input->ChunkEntries)
    {
	if(textaccess && !(*textaccess->Access)(xmlin->Input))
            return ajFalse;
	else if(xmlaccess && !(*xmlaccess->Access)(xmlin))
            return ajFalse;
        buff = xmlin->Input->Filebuff;
    }


    /* need to check end-of-file to avoid repeats */
    while(xmlin->Input->Search &&
          (xmlin->Input->TextData || !ajFilebuffIsEmpty(buff)))
    {
	jstat = xmlinReadFmt(xmlin, xml, xmlin->Input->Format);

	switch(jstat)
	{
	case FMT_OK:
	    xmlDefine(xml, xmlin);

	    return ajTrue;

        case FMT_BADTYPE:
	    ajDebug("xmlinRead: (a3) "
                    "xmlinReadFmt stat == BADTYPE *failed*\n");

	    return ajFalse;

        case FMT_FAIL:
	    ajDebug("xmlinRead: (b3) "
                    "xmlinReadFmt stat == FAIL *failed*\n");

	    return ajFalse;
            
	case FMT_NOMATCH:
	    ajDebug("xmlinRead: (c3) "
                    "xmlinReadFmt stat == NOMATCH *try again*\n");
	    break;
	case FMT_EOF:
	    ajDebug("xmlinRead: (d3) "
                    "xmlinReadFmt stat == EOF *failed*\n");

	    return ajFalse;			/* we already tried again */

        case FMT_EMPTY:
	    if(istat != FMT_EMPTY)
                ajWarn("assmebly data '%S' has zero length, ignored",
                       ajXmlGetQryS(xml));
	    ajDebug("xmlinRead: (e3) "
                    "xmlinReadFmt stat == EMPTY *try again*\n");
	    break;

        default:
	    ajDebug("unknown code %d from xmlinReadFmt\n", stat);
	}

	ajXmlClear(xml); /* 1 : read, failed to match id/acc/query */
    }

    if(xmlin->Input->Format)
	ajDebug("xmlinRead: *failed* to read xml data %S "
                "using format %s\n",
		xmlin->Input->Qry,
                xmlinFormatDef[xmlin->Input->Format].Name);
    else
	ajDebug("xmlinRead: *failed* to read xml data %S "
                "using any format\n",
		xmlin->Input->Qry);

    return ajFalse;
}




/* @funcstatic xmlinReadXml ***************************************************
**
** Given data in a xml structure, tries to read everything needed
** using general XML format.
**
** @param [u] xmlin [AjPXmlin] Xml input object
** @param [w] xml [AjPXml] xml object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool xmlinReadXml(AjPXmlin xmlin, AjPXml xml)
{
    AjPFilebuff buff;

    ajDebug("xmlinReadXml\n");
    ajXmlClear(xml);
    buff = xmlin->Input->Filebuff;
    /*textfile = ajFilebuffGetFile(buff);*/

    /* ajFilebuffTrace(buff); */

    xml->Doc = ajDomImplementationCreateDocument(NULL,NULL,NULL);

    if (ajDomReadFilebuffText(xml->Doc,buff,
                              xmlin->Input->Text, &xml->TextPtr) == -1)
    {
	ajDomDocumentDestroyNode(xml->Doc, &xml->Doc);
        xml->Doc = NULL;
	return AJFALSE;
    }

    ajFilebuffClear(buff, 0);

    return ajTrue;
}




/* @datasection [none] Miscellaneous ******************************************
**
** Xml input internals
**
** @nam2rule Xmlin Xml input
**
******************************************************************************/




/* @section Printing **********************************************************
**
** Printing details of the internals to a file
**
** @fdata [none]
**
** @nam2rule Xmlinprint
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




/* @func ajXmlinprintBook ****************************************************
**
** Reports the internal data structures as a Docbook table
**
** @param [u] outf [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

void ajXmlinprintBook(AjPFile outf)
{
    ajuint i = 0;
    ajuint j = 0;
    AjPStr namestr = NULL;
    AjPList fmtlist;
    AjPStr* names;

    fmtlist = ajListstrNew();

    ajFmtPrintF(outf, "<para>The supported xml formats are summarised "
                "in the table below. "
                "The columns are as follows: "
                "<emphasis>Input format</emphasis> (format name), "
                "<emphasis>Try</emphasis> (indicates whether the "
                "format can be detected automatically on input), and "
                "<emphasis>Description</emphasis> (short description of "
                "the format).</para>\n\n");

    ajFmtPrintF(outf, "<table frame=\"box\" rules=\"cols\">\n");
    ajFmtPrintF(outf, "  <caption>Input xml formats</caption>\n");
    ajFmtPrintF(outf, "  <thead>\n");
    ajFmtPrintF(outf, "    <tr align=\"center\">\n");
    ajFmtPrintF(outf, "      <th>Input Format</th>\n");
    ajFmtPrintF(outf, "      <th>Try</th>\n");
    ajFmtPrintF(outf, "      <th>Description</th>\n");
    ajFmtPrintF(outf, "    </tr>\n");
    ajFmtPrintF(outf, "  </thead>\n");
    ajFmtPrintF(outf, "  <tbody>\n");

    for(i=1; xmlinFormatDef[i].Name; i++)
    {
	if(!xmlinFormatDef[i].Alias)
        {
            namestr = ajStrNewC(xmlinFormatDef[i].Name);
            ajListPushAppend(fmtlist, namestr);
            namestr = NULL;
        }
    }

    ajListSort(fmtlist, &ajStrVcmp);
    ajListstrToarray(fmtlist, &names);

    for(i=0; names[i]; i++)
    {
        for(j=0; xmlinFormatDef[j].Name; j++)
        {
            if(ajStrMatchC(names[i],xmlinFormatDef[j].Name))
            {
                ajFmtPrintF(outf, "    <tr>\n");
                ajFmtPrintF(outf, "      <td>%s</td>\n",
                            xmlinFormatDef[j].Name);
                ajFmtPrintF(outf, "      <td>%B</td>\n",
                            xmlinFormatDef[j].Try);
                ajFmtPrintF(outf, "      <td>%s</td>\n",
                            xmlinFormatDef[j].Desc);
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




/* @func ajXmlinprintHtml ***************************************************
**
** Reports the internal data structures as an HTML table
**
** @param [u] outf [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

void ajXmlinprintHtml(AjPFile outf)
{
    ajuint i = 0;
    ajuint j = 0;

    AjPStr namestr = NULL;

    ajFmtPrintF(outf, "<table border=3>");
    ajFmtPrintF(outf, "<tr><th>Input Format</th><th>Auto</th>\n");
    ajFmtPrintF(outf, "<th>Multi</th><th>Description</th></tr>\n");

    for(i=1; xmlinFormatDef[i].Name; i++)
    {
        ajStrAssignC(&namestr, xmlinFormatDef[i].Name);

	if(!xmlinFormatDef[i].Alias)
        {
            for(j=i+1; xmlinFormatDef[j].Name; j++)
            {
                if(xmlinFormatDef[j].Read == xmlinFormatDef[i].Read)
                {
                    ajFmtPrintAppS(&namestr, " %s",
                                   xmlinFormatDef[j].Name);
                    if(!xmlinFormatDef[j].Alias) 
                    {
                        ajWarn("Input format '%s' same as '%s' but not alias",
                               xmlinFormatDef[j].Name,
                               xmlinFormatDef[i].Name);
                    }
                }
            }

	    ajFmtPrintF(outf, "<tr><td>\n%S\n</td><td>%B</td>\n",
                        namestr,
			xmlinFormatDef[i].Try);
            ajFmtPrintF(outf, "<td>\n%s\n</td></tr>\n",
			xmlinFormatDef[i].Desc);
        }

    }

    ajFmtPrintF(outf, "</table>\n");
    ajStrDel(&namestr);

    return;
}




/* @func ajXmlinprintText ***************************************************
**
** Reports the internal data structures
**
** @param [u] outf [AjPFile] Output file
** @param [r] full [AjBool] Full report (usually ajFalse)
** @return [void]
** @@
******************************************************************************/

void ajXmlinprintText(AjPFile outf, AjBool full)
{
    ajuint i = 0;

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# Xml input formats\n");
    ajFmtPrintF(outf, "# Name  Format name (or alias)\n");
    ajFmtPrintF(outf, "# Alias Alias name\n");
    ajFmtPrintF(outf, "# Try   Test for unknown input files\n");
    ajFmtPrintF(outf, "# Name         Alias Try "
		"Description");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "InFormat {\n");

    for(i=0; xmlinFormatDef[i].Name; i++)
	if(full || !xmlinFormatDef[i].Alias)
	    ajFmtPrintF(outf,
			"  %-12s %5B %3B \"%s\"\n",
			xmlinFormatDef[i].Name,
			xmlinFormatDef[i].Alias,
			xmlinFormatDef[i].Try,
			xmlinFormatDef[i].Desc);

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajXmlinprintWiki ***************************************************
**
** Reports the internal data structures as a wiki table
**
** @param [u] outf [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

void ajXmlinprintWiki(AjPFile outf)
{
    ajuint i = 0;
    ajuint j = 0;

    AjPStr namestr = NULL;

    ajFmtPrintF(outf, "{| class=\"wikitable sortable\" border=\"2\"\n");
    ajFmtPrintF(outf, "|-\n");
    ajFmtPrintF(outf, "!Format!!Try!!"
                "class=\"unsortable\"|Description\n");

    for(i=1; xmlinFormatDef[i].Name; i++)
    {
        ajStrAssignC(&namestr, xmlinFormatDef[i].Name);

	if(!xmlinFormatDef[i].Alias)
        {
            for(j=i+1; xmlinFormatDef[j].Name; j++)
            {
                if(xmlinFormatDef[j].Read == xmlinFormatDef[i].Read)
                {
                    ajFmtPrintAppS(&namestr, "<br>%s",
                                   xmlinFormatDef[j].Name);
                    if(!xmlinFormatDef[j].Alias) 
                    {
                        ajWarn("Input format '%s' same as '%s' but not alias",
                               xmlinFormatDef[j].Name,
                               xmlinFormatDef[i].Name);
                    }
                }
            }

            ajFmtPrintF(outf, "|-\n");
	    ajFmtPrintF(outf,
			"|%S||%B||%s\n",
			namestr,
			xmlinFormatDef[i].Try,
			xmlinFormatDef[i].Desc);
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




/* @func ajXmlinExit *********************************************************
**
** Cleans up xml input internal memory
**
** @return [void]
** @@
******************************************************************************/

void ajXmlinExit(void)
{
    ajStrDel(&xmlinReadLine);

    ajTableDel(&xmlDbMethods);

    return;
}




/* @section Internals *********************************************************
**
** Functions to return internal values
**
** @fdata [none]
**
** @nam3rule Type Internals for xml datatype
** @nam4rule Get  Return a value
** @nam5rule Fields  Known query fields for ajXmlinRead
** @nam5rule Qlinks  Known query link operators for ajXmlinRead
**
** @valrule * [const char*] Internal value
**
** @fcategory misc
**
******************************************************************************/




/* @func ajXmlinTypeGetFields ************************************************
**
** Returns the list of known field names for ajXmlinRead
**
** @return [const char*] List of field names
** @@
******************************************************************************/

const char* ajXmlinTypeGetFields(void)
{
    return "id acc";
}




/* @func ajXmlinTypeGetQlinks ************************************************
**
** Returns the listof known query link operators for ajXmlinRead
**
** @return [const char*] List of field names
** @@
******************************************************************************/

const char* ajXmlinTypeGetQlinks(void)
{
    return "|";
}




/* @datasection [AjPTable] Internal call register table ***********************
**
** Functions to manage the internal call register table that links the
** ajaxdb library functions with code in the core AJAX library.
**
** @nam2rule Xmlaccess Functions to manage xmldb call tables.
**
******************************************************************************/




/* @section Cast **************************************************************
**
** Return a reference to the call table
**
** @fdata [AjPTable] xmldb functions call table
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




/* @func ajXmlaccessGetDb ***************************************************
**
** Returns the table in which xml database access details are registered
**
** @return [AjPTable] Access functions hash table
** @@
******************************************************************************/

AjPTable ajXmlaccessGetDb(void)
{
    if(!xmlDbMethods)
        xmlDbMethods = ajCallTableNew();
    return xmlDbMethods;
    
}




/* @func ajXmlaccessMethodGetQlinks *******************************************
**
** Tests for a named method for xml data reading returns the 
** known query link operators
**
** @param [r] method [const AjPStr] Method required.
** @return [const char*] Known link operators
** @@
******************************************************************************/

const char* ajXmlaccessMethodGetQlinks(const AjPStr method)
{
    AjPXmlAccess methoddata; 

    methoddata = ajCallTableGetS(xmlDbMethods, method);
    if(!methoddata)
        return NULL;

    return methoddata->Qlink;
}




/* @func ajXmlaccessMethodGetScope ******************************************
**
** Tests for a named method for xml data reading and returns the scope
** (entry, query or all).
*
** @param [r] method [const AjPStr] Method required.
** @return [ajuint] Scope flags
** @@
******************************************************************************/

ajuint ajXmlaccessMethodGetScope(const AjPStr method)
{
    AjPXmlAccess methoddata; 
    ajuint ret = 0;

    methoddata = ajCallTableGetS(xmlDbMethods, method);
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




/* @func ajXmlaccessMethodTest **********************************************
**
** Tests for a named method for xml data reading.
**
** @param [r] method [const AjPStr] Method required.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajXmlaccessMethodTest(const AjPStr method)
{
    if(ajCallTableGetS(xmlDbMethods, method))
      return ajTrue;

    return ajFalse;
}




/* @funcstatic xmlinQryRestore **********************************************
**
** Restores a xml input specification from an AjPQueryList node
**
** @param [w] xmlin [AjPXmlin] Xml input object
** @param [r] node [const AjPQueryList] Query list node
** @return [void]
******************************************************************************/

static void xmlinQryRestore(AjPXmlin xmlin, const AjPQueryList node)
{
    xmlin->Input->Format = node->Format;
    ajStrAssignS(&xmlin->Input->Formatstr, node->Formatstr);

    return;
}




/* @funcstatic xmlinQrySave *************************************************
**
** Saves a xml input specification in an AjPQueryList node
**
** @param [w] node [AjPQueryList] Query list node
** @param [r] xmlin [const AjPXmlin] Xml input object
** @return [void]
******************************************************************************/

static void xmlinQrySave(AjPQueryList node, const AjPXmlin xmlin)
{
    node->Format   = xmlin->Input->Format;
    ajStrAssignS(&node->Formatstr, xmlin->Input->Formatstr);

    return;
}




/* @funcstatic xmlinQryProcess **********************************************
**
** Converts a xml data query into an open file.
**
** Tests for "format::" and sets this if it is found
**
** Then tests for "list:" or "@" and processes as a list file
** using xmlinListProcess which in turn invokes xmlinQryProcess
** until a valid query is found.
**
** Then tests for dbname:query and opens the file (at the correct position
** if the database definition defines it)
**
** If there is no database, looks for file:query and opens the file.
** In this case the file position is not known and xml data reading
** will have to scan for the entry/entries we need.
**
** @param [u] xmlin [AjPXmlin] Xml data input structure.
** @param [u] xml [AjPXml] Xml data to be read.
**                         The format will be replaced
**                         if defined in the query string.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool xmlinQryProcess(AjPXmlin xmlin, AjPXml xml)
{
    AjBool ret = ajTrue;
    AjPStr qrystr = NULL;
    AjBool xmlmethod = ajFalse;
    const AjPStr fmtstr = NULL;
    AjPTextin textin;
    AjPQuery qry;
    AjPXmlAccess xmlaccess = NULL;

    textin = xmlin->Input;
    qry = textin->Query;

    /* pick up the original query string */
    qrystr = ajStrNewS(textin->Qry);

    ajDebug("xmlinQryProcess '%S'\n", qrystr);

    /* look for a format:: prefix */
    fmtstr = ajQuerystrParseFormat(&qrystr, textin, xmlinformatFind);
    ajDebug("xmlinQryProcess ... fmtstr '%S' '%S'\n", fmtstr, qrystr);

    /* (seq/feat) DO NOT look for a [range] suffix */

    /* look for a list:: or @:: listfile of queries  - process and return */
    if(ajQuerystrParseListfile(&qrystr))
    {
        ajDebug("xmlinQryProcess ... listfile '%S'\n", qrystr);
        ret = xmlinListProcess(xmlin, xml, qrystr);
        ajStrDel(&qrystr);
        return ret;
    }

    /* try general text access methods (file, asis, text database access */
    ajDebug("xmlinQryProcess ... no listfile '%S'\n", qrystr);
    if(!ajQuerystrParseRead(&qrystr, textin, xmlinformatFind, &xmlmethod))
    {
        ajStrDel(&qrystr);
        return ajFalse;
    }
    
    xmlinFormatSet(xmlin, xml);

    ajDebug("xmlinQryProcess ... read nontext: %B '%S'\n",
            xmlmethod, qrystr);
    ajStrDel(&qrystr);

    /* we found a non-text method */
    if(xmlmethod)
    {
        ajDebug("xmlinQryProcess ... call method '%S'\n", qry->Method);
        ajDebug("xmlinQryProcess ... textin format %d '%S'\n",
                textin->Format, textin->Formatstr);
        ajDebug("xmlinQryProcess ...  query format  '%S'\n",
                qry->Formatstr);
        qry->Access = ajCallTableGetS(xmlDbMethods,qry->Method);
        xmlaccess = qry->Access;
        return (*xmlaccess->Access)(xmlin);
    }

    ajDebug("xmlinQryProcess text method '%S' success\n", qry->Method);

    return ajTrue;
}





/* @datasection [AjPList] Query field list ************************************
**
** Query fields lists are handled internally. Only static functions
** should appear here
**
******************************************************************************/




/* @funcstatic xmlinListProcess **********************************************
**
** Processes a file of queries.
** This function is called by, and calls, xmlinQryProcess. There is
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
** @param [u] xmlin [AjPXmlin] Xml data input
** @param [u] xml [AjPXml] Xml data
** @param [r] listfile [const AjPStr] Name of list file.,
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool xmlinListProcess(AjPXmlin xmlin, AjPXml xml,
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
    ajDebug("++xmlinListProcess %S depth %d\n",
	    listfile, depth);

    if(depth > MAXDEPTH)
	ajFatal("Query list too deep");

    if(!xmlin->Input->List)
	xmlin->Input->List = ajListNew();

    list = ajListNew();

    file = ajFileNewInNameS(listfile);

    if(!file)
    {
	ajErr("Failed to open list file '%S'", listfile);
	depth--;

	return ret;
    }

    while(ajReadlineTrim(file, &xmlinReadLine))
    {
        ++recnum;
	xmlinListNoComment(&xmlinReadLine);

        if(ajStrExtractWord(xmlinReadLine, &rest, &token))
        {
            if(ajStrGetLen(rest)) 
            {
                ajErr("Bad record %u in list file '%S'\n'%S'",
                      recnum, listfile, xmlinReadLine);
            }
            else if(ajStrGetLen(token))
            {
                ajDebug("++Add to list: '%S'\n", token);
                AJNEW0(node);
                ajStrAssignS(&node->Qry, token);
                xmlinQrySave(node, xmlin);
                ajListPushAppend(list, node);
            }
        }
    }

    ajFileClose(&file);
    ajStrDel(&token);
    ajStrDel(&rest);

    ajDebug("Trace xmlin->Input->List\n");
    ajQuerylistTrace(xmlin->Input->List);
    ajDebug("Trace new list\n");
    ajQuerylistTrace(list);
    ajListPushlist(xmlin->Input->List, &list);

    ajDebug("Trace combined xmlin->Input->List\n");
    ajQuerylistTrace(xmlin->Input->List);

    /*
     ** now try the first item on the list
     ** this can descend recursively if it is also a list
     ** which is why we check the depth above
     */

    if(ajListPop(xmlin->Input->List, (void**) &node))
    {
        ajDebug("++pop first item '%S'\n", node->Qry);
	ajXmlinQryS(xmlin, node->Qry);
	xmlinQryRestore(xmlin, node);
	ajStrDel(&node->Qry);
	ajStrDel(&node->Formatstr);
	AJFREE(node);
	ajDebug("descending with query '%S'\n", xmlin->Input->Qry);
	ret = xmlinQryProcess(xmlin, xml);
    }

    depth--;
    ajDebug("++xmlinListProcess depth: %d returns: %B\n", depth, ret);

    return ret;
}




/* @funcstatic xmlinListNoComment ********************************************
**
** Strips comments from a character string (a line from an ACD file).
** Comments are blank lines or any text following a "#" character.
**
** @param [u] text [AjPStr*] Line of text from input file.
** @return [void]
** @@
******************************************************************************/

static void xmlinListNoComment(AjPStr* text)
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




/* @funcstatic xmlinFormatSet ************************************************
**
** Sets the input format for xml data using the xml data
** input object's defined format
**
** @param [u] xmlin [AjPXmlin] Xml data input.
** @param [u] xml [AjPXml] Xml data
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool xmlinFormatSet(AjPXmlin xmlin, AjPXml xml)
{

    if(ajStrGetLen(xmlin->Input->Formatstr))
    {
	ajDebug("... input format value '%S'\n",
                xmlin->Input->Formatstr);

	if(xmlinformatFind(xmlin->Input->Formatstr,
                             &xmlin->Input->Format))
	{
	    ajStrAssignS(&xml->Formatstr,
                         xmlin->Input->Formatstr);
	    xml->Format = xmlin->Input->Format;
	    ajDebug("...format OK '%S' = %d\n",
                    xmlin->Input->Formatstr,
		    xmlin->Input->Format);
	}
	else
	    ajDebug("...format unknown '%S'\n",
                    xmlin->Input->Formatstr);

	return ajTrue;
    }
    else
	ajDebug("...input format not set\n");


    return ajFalse;
}




/* @datasection [AjPXmlall] Xml Input Stream **********************************
**
** Function is for manipulating xml input stream objects
**
** @nam2rule Xmlall Xml input stream objects
**
******************************************************************************/




/* @section Xml Input Constructors ********************************************
**
** All constructors return a new xml input stream object by pointer. It
** is the responsibility of the user to first destroy any previous
** xml input object. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so
** anyway.
**
** @fdata [AjPXmlall]
**
** @nam3rule New Constructor
**
** @valrule * [AjPXmlall] Xml input stream object
**
** @fcategory new
**
******************************************************************************/




/* @func ajXmlallNew **********************************************************
**
** Creates a new xml input stream object.
**
** @return [AjPXmlall] New xml input stream object.
** @@
******************************************************************************/

AjPXmlall ajXmlallNew(void)
{
    AjPXmlall pthis;

    AJNEW0(pthis);

    pthis->Xmlin = ajXmlinNew();
    pthis->Xml   = ajXmlNew();

    return pthis;
}





/* ==================================================================== */
/* ========================== destructors ============================= */
/* ==================================================================== */




/* @section Xml Input Stream Destructors ************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the xml input stream object.
**
** @fdata [AjPXmlall]
**
** @nam3rule Del Destructor
**
** @argrule Del pthis [AjPXmlall*] Xml input stream
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajXmlallDel *********************************************************
**
** Deletes a xml input stream object.
**
** @param [d] pthis [AjPXmlall*] Xml input stream
** @return [void]
** @@
******************************************************************************/

void ajXmlallDel(AjPXmlall* pthis)
{
    AjPXmlall thys;

    if(!pthis)
        return;

    thys = *pthis;

    if(!thys)
        return;

    ajXmlinDel(&thys->Xmlin);
    if(!thys->Returned)
        ajXmlDel(&thys->Xml);

    AJFREE(*pthis);

    return;
}




/* ==================================================================== */
/* =========================== Modifiers ============================== */
/* ==================================================================== */




/* @section xml input stream modifiers **************************************
**
** These functions use the contents of a xml input stream object and
** update them.
**
** @fdata [AjPXmlall]
**
** @nam3rule Clear Clear all values
**
** @argrule * thys [AjPXmlall] Xml input stream object
**
** @valrule * [void]
**
** @fcategory modify
**
******************************************************************************/




/* @func ajXmlallClear *******************************************************
**
** Clears a xml input stream object back to "as new" condition, except
** for the query list which must be preserved.
**
** @param [w] thys [AjPXmlall] Xml input stream
** @return [void]
** @@
******************************************************************************/

void ajXmlallClear(AjPXmlall thys)
{

    ajDebug("ajXmlallClear called\n");

    ajXmlinClear(thys->Xmlin);

    ajXmlClear(thys->Xml);

    thys->Returned = ajFalse;

    return;
}




/* @section xml input stream casts ******************************************
**
** These functions return the contents of a xml input stream object
**
** @fdata [AjPXmlall]
**
** @nam3rule Get Get xml input stream values
** @nam3rule Getxml Get xml values
** @nam4rule GetxmlId Get xml identifier
**
** @argrule * thys [const AjPXmlall] Xml input stream object
**
** @valrule * [const AjPStr] String value
**
** @fcategory cast
**
******************************************************************************/




/* @func ajXmlallGetxmlId ***************************************************
**
** Returns the identifier of the current xml in an input stream
**
** @param [r] thys [const AjPXmlall] Xml input stream
** @return [const AjPStr] Identifier
** @@
******************************************************************************/

const AjPStr ajXmlallGetxmlId(const AjPXmlall thys)
{
    if(!thys)
        return NULL;

    ajDebug("ajXmlallGetxmlId called\n");

    return ajXmlGetId(thys->Xml);
}




/* @section xml input *********************************************************
**
** These functions use a xml input stream object to read data
**
** @fdata [AjPXmlall]
**
** @nam3rule Next Read next xml
**
** @argrule * thys [AjPXmlall] Xml input stream object
** @argrule * Pxml [AjPXml*] Xml object
**
** @valrule * [AjBool] True on success
**
** @fcategory input
**
******************************************************************************/




/* @func ajXmlallNext ********************************************************
**
** Parse a xml query into format, access, file and entry
**
** Split at delimiters. Check for the first part as a valid format
** Check for the remaining first part as a database name or as a file
** that can be opened.
** Anything left is an entryname spec.
**
** Return the results in the AjPXml object but leave the file open for
** future calls.
**
** @param [w] thys [AjPXmlall] Xml input stream
** @param [u] Pxml [AjPXml*] Xml returned
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajXmlallNext(AjPXmlall thys, AjPXml *Pxml)
{
    ajDebug("ajXmlallNext count:%u\n", thys->Count);

    if(!thys->Count)
    {
	thys->Count = 1;

	thys->Totterms++;

	*Pxml = thys->Xml;
	thys->Returned = ajTrue;

	return ajTrue;
    }


    if(ajXmlinRead(thys->Xmlin, thys->Xml))
    {
	thys->Count++;

	thys->Totterms++;

	*Pxml = thys->Xml;
	thys->Returned = ajTrue;

	ajDebug("ajXmlallNext success\n");

	return ajTrue;
    }

    *Pxml = NULL;

    ajDebug("ajXmlallNext failed\n");

    ajXmlallClear(thys);

    return ajFalse;
}




/* @datasection [none] Input formats ******************************************
**
** Input formats internals
**
** @nam2rule Xmlinformat Xml data input format specific
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




/* @funcstatic xmlinformatFind ***********************************************
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

static AjBool xmlinformatFind(const AjPStr format, ajint* iformat)
{
    AjPStr tmpformat = NULL;
    ajuint i = 0;

    /* ajDebug("xmlinformatFind '%S'\n", format); */
    if(!ajStrGetLen(format))
	return ajFalse;

    ajStrAssignS(&tmpformat, format);
    ajStrFmtLower(&tmpformat);

    for(i=0; xmlinFormatDef[i].Name; i++)
    {
	/* ajDebug("test %d '%s' \n",
	 i, xmlinFormatDef[i].Name,
	 xmlinFormatDef[i].Obo,
	 xmlinFormatDef[i].Desc); */
	if(ajStrMatchC(tmpformat, xmlinFormatDef[i].Name) ||
	   ajStrMatchC(format, xmlinFormatDef[i].Obo))
	{
	    *iformat = i;
	    ajStrDel(&tmpformat);
	    /* ajDebug("found '%s' at %d\n", xmlinFormatDef[i].Name, i); */
	    return ajTrue;
	}
    }

    ajErr("Unknown input format '%S'", format);

    ajStrDel(&tmpformat);

    return ajFalse;
}




/* @func ajXmlinformatTerm ***************************************************
**
** Tests whether a xml data input format term is known
**
** @param [r] term [const AjPStr] Format term EDAM ID
** @return [AjBool] ajTrue if term was accepted
** @@
******************************************************************************/

AjBool ajXmlinformatTerm(const AjPStr term)
{
    ajuint i;

    for(i=0; xmlinFormatDef[i].Name; i++)
	if(ajStrMatchC(term, xmlinFormatDef[i].Obo))
	    return ajTrue;

    return ajFalse;
}




/* @func ajXmlinformatTest ***************************************************
**
** Tests whether a named xml data input format is known
**
** @param [r] format [const AjPStr] Format
** @return [AjBool] ajTrue if term was accepted
** @@
******************************************************************************/

AjBool ajXmlinformatTest(const AjPStr format)
{
    ajuint i;

    for(i=0; xmlinFormatDef[i].Name; i++)
    {
	if(ajStrMatchCaseC(format, xmlinFormatDef[i].Name))	
	    return ajTrue;
	if(ajStrMatchC(format, xmlinFormatDef[i].Obo))	
	    return ajTrue;
    }

    return ajFalse;
}

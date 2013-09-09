/* @source ajxml *************************************************************
**
** AJAX xml functions
**
** These functions control all aspects of AJAX xml
** parsing and include simple utilities.
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.3 $
** @modified Oct 5 pmr First version
** @modified $Date: 2012/11/14 14:20:29 $ by $Author: rice $
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA  02111-1307, USA.
******************************************************************************/

#include "ajlib.h"

#include "ajxml.h"
#include "ajlist.h"
#include "ajxmlread.h"
#include "ajxmlwrite.h"


static AjPStr xmlTempQry = NULL;

static void xmlMakeQry(const AjPXml thys, AjPStr* qry);




/* @filesection ajxml *******************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
******************************************************************************/





/* @datasection [AjPXml] Xml data ***************************************
**
** Function is for manipulating xml data objects
**
** @nam2rule Xml Xml data objects
**
******************************************************************************/




/* @section constructors ******************************************************
**
** Constructors
**
** @fdata [AjPXml]
**
** @nam3rule New Constructor
**
** @valrule * [AjPXml] Xml data object
**
** @fcategory new
**
******************************************************************************/




/* @func ajXmlNew ************************************************************
**
** Xml data constructor
**
** @return [AjPXml] New object
** @@
******************************************************************************/

AjPXml ajXmlNew(void)
{
    AjPXml ret;

    AJNEW0(ret);

    return ret;
}




/* @section Xml data destructors **********************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the xml data object.
**
** @fdata [AjPXml]
**
** @nam3rule Del Destructor
**
** @argrule Del Pxml [AjPXml*] Xml data
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajXmlDel ************************************************************
**
** Xml data destructor
**
** @param [d] Pxml       [AjPXml*] Xml data object to delete
** @return [void] 
** @@
******************************************************************************/

void ajXmlDel(AjPXml *Pxml)
{
    AjPXml xml;

    if(!Pxml) return;
    if(!(*Pxml)) return;

    xml = *Pxml;

    ajStrDel(&xml->Id);
    ajStrDel(&xml->Db);
    ajStrDel(&xml->Setdb);
    ajStrDel(&xml->Full);
    ajStrDel(&xml->Qry);
    ajStrDel(&xml->Formatstr);
    ajStrDel(&xml->Filename);

    ajStrDel(&xml->TextPtr);

    if(xml->Doc)
        ajDomDocumentDestroyNode(xml->Doc, &xml->Doc);

    AJFREE(*Pxml);
    *Pxml = NULL;

    return;
}




/* @section Casts *************************************************************
**
** Return values from a xml data object
**
** @fdata [AjPXml]
**
** @nam3rule Get Return a value
** @nam4rule Db Source database name
** @nam4rule Entry Full entry text
** @nam4rule Id Identifier string
** @nam4rule Qry Return a query field
** @suffix C Character string result
** @suffix S String object result
**
** @argrule * xml [const AjPXml] Xml data object.
**
** @valrule *C [const char*] Query as a character string.
** @valrule *S [const AjPStr] Query as a string object.
** @valrule *GetDb [const AjPStr] Database name
** @valrule *GetEntry [const AjPStr] Full entry text
** @valrule *GetId [const AjPStr] Identifier string
**
** @fcategory cast
**
******************************************************************************/




/* @func ajXmlGetDb **********************************************************
**
** Return the database name
**
** @param [r] xml [const AjPXml] Xml
**
** @return [const AjPStr] Database name
**
******************************************************************************/

const AjPStr ajXmlGetDb(const AjPXml xml)
{
    return xml->Db;
}




/* @func ajXmlGetEntry ********************************************************
**
** Return the full text
**
** @param [r] xml [const AjPXml] XML entry
**
** @return [const AjPStr] Returned full text
**
**
** @release 6.6.0
******************************************************************************/

const AjPStr ajXmlGetEntry(const AjPXml xml)
{
    if(!xml)
        return NULL;

    if(xml->TextPtr)
        return xml->TextPtr;

    return ajStrConstEmpty();
}




/* @func ajXmlGetId **********************************************************
**
** Return the identifier
**
** @param [r] xml [const AjPXml] Xml
**
** @return [const AjPStr] Returned id
**
******************************************************************************/

const AjPStr ajXmlGetId(const AjPXml xml)
{
    return xml->Id;
}




/* @func ajXmlGetQryC ********************************************************
**
** Returns the query string of a xml data object.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] xml [const AjPXml] Xml data object.
** @return [const char*] Query as a character string.
** @@
******************************************************************************/

const char* ajXmlGetQryC(const AjPXml xml)
{
    return MAJSTRGETPTR(ajXmlGetQryS(xml));
}




/* @func ajXmlGetQryS ********************************************************
**
** Returns the query string of a xml data object.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] xml [const AjPXml] Xml data object.
** @return [const AjPStr] Query as a string.
** @@
******************************************************************************/

const AjPStr ajXmlGetQryS(const AjPXml xml)
{
    ajDebug("ajXmlGetQryS '%S'\n", xml->Qry);

    if(ajStrGetLen(xml->Qry))
	return xml->Qry;

    xmlMakeQry(xml, &xmlTempQry);

    return xmlTempQry;
}




/* @funcstatic xmlMakeQry ***************************************************
**
** Sets the query for a xml data object.
**
** @param [r] thys [const AjPXml] Xml data object
** @param [w] qry [AjPStr*] Query string in full
** @return [void]
** @@
******************************************************************************/

static void xmlMakeQry(const AjPXml thys, AjPStr* qry)
{
    ajDebug("xmlMakeQry (Id <%S> Formatstr <%S> Db <%S> "
	    "Filename <%S>)\n",
	    thys->Id, thys->Formatstr, thys->Db,
	    thys->Filename);

    /* ajXmlTrace(thys); */

    if(ajStrGetLen(thys->Db))
	ajFmtPrintS(qry, "%S-id:%S", thys->Db, thys->Id);
    else
    {
	ajFmtPrintS(qry, "%S::%S:%S", thys->Formatstr,
                    thys->Filename,thys->Id);
    }

    ajDebug("      result: <%S>\n",
	    *qry);

    return;
}




/* @section xml data modifiers *******************************************
**
** Xml data modifiers
**
** @fdata [AjPXml]
**
** @nam3rule Clear clear internal values
**
** @argrule * xml [AjPXml] Xml data object
**
** @valrule * [void]
**
** @fcategory modify
**
******************************************************************************/




/* @func ajXmlClear **********************************************************
**
** Resets all data for a xml data object so that it can be reused.
**
** @param [u] xml [AjPXml] Xml data object
** @return [void]
** @@
******************************************************************************/

void ajXmlClear(AjPXml xml)
{
    if(MAJSTRGETLEN(xml->Id))
       ajStrSetClear(&xml->Id);

    if(MAJSTRGETLEN(xml->Db))
       ajStrSetClear(&xml->Db);

    if(MAJSTRGETLEN(xml->Setdb))
       ajStrSetClear(&xml->Setdb);

    if(MAJSTRGETLEN(xml->Full))
       ajStrSetClear(&xml->Full);

    if(MAJSTRGETLEN(xml->Qry))
       ajStrSetClear(&xml->Qry);

    if(MAJSTRGETLEN(xml->Formatstr))
       ajStrSetClear(&xml->Formatstr);

    if(MAJSTRGETLEN(xml->Filename))
       ajStrSetClear(&xml->Filename);

    ajStrDel(&xml->TextPtr);

    xml->Count = 0;
    xml->Fpos = 0L;
    xml->Format = 0;

    if(xml->Doc)
        ajDomDocumentDestroyNode(xml->Doc, &xml->Doc);

    return;
}




/* @datasection [none] Miscellaneous functions ********************************
**
** Functions to initialise and clean up internals
**
** @nam2rule Xml Xml internals
**
******************************************************************************/




/* @section exit **************************************************************
**
** Functions called on exit from the program by ajExit to do
** any necessary cleanup and to report internal statistics to the debug file
**
** @fdata      [none]
** @fnote     general exit functions, no arguments
**
** @nam3rule Exit Cleanup and report on exit
**
** @valrule * [void]
**
** @fcategory misc
******************************************************************************/




/* @func ajXmlExit ***********************************************************
**
** Cleans up xml processing internal memory
**
** @return [void]
** @@
******************************************************************************/

void ajXmlExit(void)
{
    ajXmlinExit();
    ajXmloutExit();

    return;
}




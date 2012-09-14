/* @source ajurl **************************************************************
**
** AJAX url functions
**
** These functions control all aspects of AJAX url
** parsing and include simple utilities.
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.6 $
** @modified Oct 5 pmr First version
** @modified $Date: 2011/10/18 14:23:41 $ by $Author: rice $
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

#include "ajurl.h"
#include "ajlist.h"
#include "ajurlread.h"
#include "ajurlwrite.h"
#include "ajresource.h"

static AjPStr urlTempQry = NULL;

static void urlMakeQry(const AjPUrl thys, AjPStr* qry);




/* @filesection ajurl *******************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
******************************************************************************/





/* @datasection [AjPUrl] Url data ***************************************
**
** Function is for manipulating url data objects
**
** @nam2rule Url Url data objects
**
******************************************************************************/




/* @section constructors ******************************************************
**
** Constructors
**
** @fdata [AjPUrl]
**
** @nam3rule New Constructor
**
** @valrule * [AjPUrl] Url data object
**
** @fcategory new
**
******************************************************************************/




/* @func ajUrlNew *************************************************************
**
** Url data constructor
**
** @return [AjPUrl] New object
**
** @release 6.4.0
** @@
******************************************************************************/

AjPUrl ajUrlNew(void)
{
    AjPUrl ret;

    AJNEW0(ret);

    return ret;
}




/* @section Url data destructors *****************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the url data object.
**
** @fdata [AjPUrl]
**
** @nam3rule Del Destructor
**
** @argrule Del Purl [AjPUrl*] Url data
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajUrlDel *************************************************************
**
** Url data destructor
**
** @param [d] Purl       [AjPUrl*] Url data object to delete
** @return [void] 
**
** @release 6.4.0
** @@
******************************************************************************/

void ajUrlDel(AjPUrl *Purl)
{
    AjPUrl url;

    if(!Purl) return;
    if(!(*Purl)) return;

    url = *Purl;

    ajStrDel(&url->Id);
    ajStrDel(&url->Db);
    ajStrDel(&url->Setdb);
    ajStrDel(&url->Full);
    ajStrDel(&url->Qry);
    ajStrDel(&url->Formatstr);
    ajStrDel(&url->Filename);

    ajResqueryDel(&url->Resqry);

    ajStrDelarray(&url->Lines);

    AJFREE(*Purl);
    *Purl = NULL;

    return;
}




/* @section Casts *************************************************************
**
** Return values from an url data object
**
** @fdata [AjPUrl]
**
** @nam3rule Get Return a value
** @nam4rule Db Source database name
** @nam4rule Id Identifier string
** @nam4rule Qry Return a query field
** @suffix C Character string result
** @suffix S String object result
**
** @argrule * url [const AjPUrl] Url data object.
**
** @valrule *C [const char*] Query as a character string.
** @valrule *S [const AjPStr] Query as a string object.
** @valrule *GetDb [const AjPStr] Database name
** @valrule *GetId [const AjPStr] Identifier string
**
** @fcategory cast
**
******************************************************************************/




/* @func ajUrlGetDb ***********************************************************
**
** Return the database name
**
** @param [r] url [const AjPUrl] Url
**
** @return [const AjPStr] Database name
**
**
** @release 6.4.0
******************************************************************************/

const AjPStr ajUrlGetDb(const AjPUrl url)
{
    return url->Db;
}




/* @func ajUrlGetId ***********************************************************
**
** Return the identifier
**
** @param [r] url [const AjPUrl] Url
**
** @return [const AjPStr] Returned id
**
**
** @release 6.4.0
******************************************************************************/

const AjPStr ajUrlGetId(const AjPUrl url)
{
    return url->Id;
}




/* @func ajUrlGetQryC *********************************************************
**
** Returns the query string of an url data object.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] url [const AjPUrl] Url data object.
** @return [const char*] Query as a character string.
**
** @release 6.4.0
** @@
******************************************************************************/

const char* ajUrlGetQryC(const AjPUrl url)
{
    return MAJSTRGETPTR(ajUrlGetQryS(url));
}




/* @func ajUrlGetQryS *********************************************************
**
** Returns the query string of an url data object.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] url [const AjPUrl] Url data object.
** @return [const AjPStr] Query as a string.
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPStr ajUrlGetQryS(const AjPUrl url)
{
    ajDebug("ajUrlGetQryS '%S'\n", url->Qry);

    if(ajStrGetLen(url->Qry))
	return url->Qry;

    urlMakeQry(url, &urlTempQry);

    return urlTempQry;
}




/* @funcstatic urlMakeQry *****************************************************
**
** Sets the query for an url data object.
**
** @param [r] thys [const AjPUrl] Url data object
** @param [w] qry [AjPStr*] Query string in full
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void urlMakeQry(const AjPUrl thys, AjPStr* qry)
{
    ajDebug("urlMakeQry (Id <%S> Formatstr <%S> Db <%S> "
	    "Filename <%S>)\n",
	    thys->Id, thys->Formatstr, thys->Db,
	    thys->Filename);

    /* ajUrlTrace(thys); */

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




/* @section url data modifiers *******************************************
**
** Url data modifiers
**
** @fdata [AjPUrl]
**
** @nam3rule Clear clear internal values
**
** @argrule * url [AjPUrl] Url data object
**
** @valrule * [void]
**
** @fcategory modify
**
******************************************************************************/




/* @func ajUrlClear ***********************************************************
**
** Resets all data for an url data object so that it can be reused.
**
** @param [u] url [AjPUrl] Url data object
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajUrlClear(AjPUrl url)
{
    if(MAJSTRGETLEN(url->Id))
       ajStrSetClear(&url->Id);

    if(MAJSTRGETLEN(url->Db))
       ajStrSetClear(&url->Db);

    if(MAJSTRGETLEN(url->Setdb))
       ajStrSetClear(&url->Setdb);

    if(MAJSTRGETLEN(url->Full))
       ajStrSetClear(&url->Full);

    if(MAJSTRGETLEN(url->Qry))
       ajStrSetClear(&url->Qry);

    if(MAJSTRGETLEN(url->Formatstr))
       ajStrSetClear(&url->Formatstr);

    if(MAJSTRGETLEN(url->Filename))
       ajStrSetClear(&url->Filename);

    ajResqueryDel(&url->Resqry);

    ajStrDelarray(&url->Lines);

    url->Count = 0;
    url->Fpos = 0L;
    url->Format = 0;

    return;
}




/* @datasection [none] Miscellaneous functions ********************************
**
** Functions to initialise and clean up internals
**
** @nam2rule Url Url internals
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




/* @func ajUrlExit ************************************************************
**
** Cleans up url processing internal memory
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajUrlExit(void)
{
    ajUrlinExit();
    ajUrloutExit();

    return;
}




/* @source ajurlread **********************************************************
**
** AJAX url reading functions
**
** These functions control all aspects of AJAX url reading
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.17 $
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

#include "ajurlread.h"
#include "ajurl.h"
#include "ajcall.h"
#include "ajlist.h"
#include "ajquery.h"
#include "ajtextread.h"
#include "ajnam.h"
#include "ajfileio.h"
#include "ajresource.h"

#include <string.h>

AjPTable urlDbMethods = NULL;

static AjPStr urlinReadLine     = NULL;


static AjBool urlinReadHtml(AjPUrlin thys, AjPUrl url);




/* @datastatic UrlPInFormat *************************************************
**
** Url input formats data structure
**
** @alias UrlSInFormat
** @alias UrlOInFormat
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

typedef struct UrlSInFormat
{
    const char *Name;
    const char *Obo;
    const char *Desc;
    AjBool Alias;
    AjBool Try;
    AjBool (*Read) (AjPUrlin thys, AjPUrl url);
} UrlOInFormat;

#define UrlPInFormat UrlOInFormat*

static UrlOInFormat urlinFormatDef[] =
{
/* "Name",        "OBOterm", "Description" */
/*     Alias,   Try,     */
/*     ReadFunction */
  {"unknown",     "0000", "Unknown format",
       AJFALSE, AJFALSE,
       &urlinReadHtml}, /* default to first format */
  {"html",        "2331", "Html format",
       AJFALSE, AJTRUE,
       &urlinReadHtml},
  {"HTML",        "2331", "Html format",
       AJTRUE, AJFALSE,
       &urlinReadHtml},
  {NULL, NULL, NULL, 0, 0, NULL}
};



static AjBool urlinRead(AjPUrlin urlin, AjPUrl url);
static AjBool urlinformatFind(const AjPStr format, ajint* iformat);
static AjBool urlinFormatSet(AjPUrlin urlin, AjPUrl url);
static AjBool urlinListProcess(AjPUrlin urlin, AjPUrl url,
                               const AjPStr listfile);
static void urlinListNoComment(AjPStr* text);
static void urlinQryRestore(AjPUrlin urlin, const AjPQueryList node);
static void urlinQrySave(AjPQueryList node, const AjPUrlin urlin);
static AjBool urlDefine(AjPUrl thys, AjPUrlin urlin);
static AjBool urlinQryProcess(AjPUrlin urlin, AjPUrl url);




/* @filesection ajurlread ****************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
*/




/* @datasection [AjPUrlin] Url input objects ***************************
**
** Function is for manipulating url input objects
**
** @nam2rule Urlin
******************************************************************************/




/* @section Url input constructors ***************************************
**
** All constructors return a new url input object by pointer. It
** is the responsibility of the user to first destroy any previous
** url input object. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so
** anyway.
**
** @fdata [AjPUrlin]
**
** @nam3rule  New     Construct a new url input object
**
** @valrule   *  [AjPUrlin] New url input object
**
** @fcategory new
**
******************************************************************************/




/* @func ajUrlinNew ***********************************************************
**
** Creates a new url input object.
**
** @return [AjPUrlin] New url input object.
** @category new [AjPUrlin] Default constructor
**
** @release 6.4.0
** @@
******************************************************************************/

AjPUrlin ajUrlinNew(void)
{
    AjPUrlin pthis;

    AJNEW0(pthis);

    pthis->Input = ajTextinNewDatatype(AJDATATYPE_URL);

    pthis->Resource = NULL;
    pthis->UrlData  = NULL;

    return pthis;
}





/* @section url input destructors *********************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the url input object.
**
** @fdata [AjPUrlin]
**
** @nam3rule Del Destructor
**
** @argrule Del pthis [AjPUrlin*] Url input object
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajUrlinDel ***********************************************************
**
** Deletes an url input object.
**
** @param [d] pthis [AjPUrlin*] Url input
** @return [void]
** @category delete [AjPUrlin] Default destructor
**
** @release 6.4.0
** @@
******************************************************************************/

void ajUrlinDel(AjPUrlin* pthis)
{
    AjPUrlin thys;
    AjPResquery resqry;

    if(!pthis)
        return;

    thys = *pthis;

    if(!thys)
        return;

    ajDebug("ajUrlinDel called qry:'%S'\n", thys->Input->Qry);

    ajTextinDel(&thys->Input);
    ajResourceDel(&thys->Resource);

    ajStrDel(&thys->Identifiers);
    ajStrDel(&thys->Accession);
    ajStrDel(&thys->IdTypes);

    ajListstrFree(&thys->UrlList);

    while(ajListPop(thys->QryList, (void**)&resqry))
        ajResqueryDel(&resqry);
    ajListFree(&thys->QryList);

    AJFREE(*pthis);

    return;
}




/* @section url input modifiers ******************************************
**
** These functions use the contents of an url input object and
** update them.
**
** @fdata [AjPUrlin]
**
** @nam3rule Clear Clear all values
** @nam3rule Qry Reset using a query string
** @suffix C Character string input
** @suffix S String input
**
** @argrule * thys [AjPUrlin] Url input object
** @argrule C txt [const char*] Query text
** @argrule S str [const AjPStr] query string
**
** @valrule * [void]
**
** @fcategory modify
**
******************************************************************************/




/* @func ajUrlinClear *********************************************************
**
** Clears an url input object back to "as new" condition, except
** for the query list which must be preserved.
**
** @param [w] thys [AjPUrlin] Url input
** @return [void]
** @category modify [AjPUrlin] Resets ready for reuse.
**
** @release 6.4.0
** @@
******************************************************************************/

void ajUrlinClear(AjPUrlin thys)
{

    ajDebug("ajUrlinClear called\n");

    ajTextinClear(thys->Input);
    ajResourceDel(&thys->Resource);
    ajStrDel(&thys->Identifiers);
    ajStrDel(&thys->Accession);
    ajStrDel(&thys->IdTypes);
    ajListstrFree(&thys->UrlList);

    thys->IsSwiss = ajFalse;
    thys->IsEmbl = ajFalse;

    thys->UrlData = NULL;

    return;
}




/* @func ajUrlinQryC **********************************************************
**
** Resets an url input object using a new Universal
** Query Address
**
** @param [u] thys [AjPUrlin] Url input object.
** @param [r] txt [const char*] Query
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajUrlinQryC(AjPUrlin thys, const char* txt)
{
    ajUrlinClear(thys);
    ajStrAssignC(&thys->Input->Qry, txt);

    return;
}





/* @func ajUrlinQryS **********************************************************
**
** Resets an url input object using a new Universal
** Query Address
**
** @param [u] thys [AjPUrlin] Url input object.
** @param [r] str [const AjPStr] Query
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajUrlinQryS(AjPUrlin thys, const AjPStr str)
{
    ajUrlinClear(thys);
    ajStrAssignS(&thys->Input->Qry, str);

    return;
}




/* @section casts *************************************************************
**
** Return values
**
** @fdata [AjPUrlin]
**
** @nam3rule Trace Write debugging output
**
** @argrule * thys [const AjPUrlin] Url input object
**
** @valrule * [void]
**
** @fcategory cast
**
******************************************************************************/




/* @func ajUrlinTrace *********************************************************
**
** Debug calls to trace the data in an url input object.
**
** @param [r] thys [const AjPUrlin] Url input object.
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajUrlinTrace(const AjPUrlin thys)
{
    ajDebug("url input trace\n");
    ajDebug("====================\n\n");

    ajTextinTrace(thys->Input);

    if(thys->UrlData)
	ajDebug( "  UrlData: exists\n");

    return;
}




/* @section Url data inputs **********************************************
**
** These functions read the wxyxdesc data provided by the first argument
**
** @fdata [AjPUrlin]
**
** @nam3rule Read Read url data
**
** @argrule Read urlin [AjPUrlin] Url input object
** @argrule Read url [AjPUrl] Url data
**
** @valrule * [AjBool] true on success
**
** @fcategory input
**
******************************************************************************/




/* @func ajUrlinRead **********************************************************
**
** If the file is not yet open, calls urlinQryProcess to convert the query
** into an open file stream.
**
** Uses urlinRead for the actual file reading.
**
** Returns the results in the AjPUrl object.
**
** @param [u] urlin [AjPUrlin] Url data input definitions
** @param [w] url [AjPUrl] Url data returned.
** @return [AjBool] ajTrue on success.
** @category input [AjPUrl] Master url data input,
**                  calls specific functions for file access type
**                  and url data format.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajUrlinRead(AjPUrlin urlin, AjPUrl url)
{
    AjBool ret       = ajFalse;
    AjPQueryList node = NULL;
    AjBool listdata  = ajFalse;

    if(urlin->Resource)
    {
	/* (a) if file still open, keep reading */
	ajDebug("ajUrlinRead: input resource '%S' still there, try again\n",
		urlin->Resource->Id);
	ret = urlinRead(urlin, url);
	ajDebug("ajUrlinRead: open buffer  qry: '%S' returns: %B\n",
		urlin->Input->Qry, ret);
    }
    else
    {
	/* (b) if we have a list, try the next query in the list */
	if(ajListGetLength(urlin->Input->List))
	{
	    listdata = ajTrue;
	    ajListPop(urlin->Input->List, (void**) &node);

	    ajDebug("++pop from list '%S'\n", node->Qry);
	    ajUrlinQryS(urlin, node->Qry);
	    ajDebug("++SAVE WXYZIN '%S' '%S' %d\n",
		    urlin->Input->Qry,
		    urlin->Input->Formatstr, urlin->Input->Format);

            urlinQryRestore(urlin, node);

	    ajStrDel(&node->Qry);
	    ajStrDel(&node->Formatstr);
	    AJFREE(node);

	    ajDebug("ajUrlinRead: open list, try '%S'\n",
                    urlin->Input->Qry);

	    if(!urlinQryProcess(urlin, url) &&
               !ajListGetLength(urlin->Input->List))
		return ajFalse;

	    ret = urlinRead(urlin, url);
	    ajDebug("ajUrlinRead: list qry: '%S' returns: %B\n",
		    urlin->Input->Qry, ret);
	}
	else
	{
	    ajDebug("ajUrlinRead: no file yet - test query '%S'\n",
                    urlin->Input->Qry);

	    /* (c) Must be a query - decode it */
	    if(!urlinQryProcess(urlin, url) &&
               !ajListGetLength(urlin->Input->List))
		return ajFalse;

	    if(ajListGetLength(urlin->Input->List)) /* could be a new list */
		listdata = ajTrue;

	    ret = urlinRead(urlin, url);
	    ajDebug("ajUrlinRead: new qry: '%S' returns: %B\n",
		    urlin->Input->Qry, ret);
	}
    }

    /* Now read whatever we got */

    while(!ret && ajListGetLength(urlin->Input->List))
    {
	/* Failed, but we have a list still - keep trying it */
        if(listdata)
	    ajErr("Failed to read url data '%S'",
                  urlin->Input->Qry);

	listdata = ajTrue;
	ajListPop(urlin->Input->List,(void**) &node);
	ajDebug("++try again: pop from list '%S'\n", node->Qry);
	ajUrlinQryS(urlin, node->Qry);
	ajDebug("++SAVE (AGAIN) WXYZIN '%S' '%S' %d\n",
		urlin->Input->Qry,
		urlin->Input->Formatstr, urlin->Input->Format);

	urlinQryRestore(urlin, node);

	ajStrDel(&node->Qry);
	ajStrDel(&node->Formatstr);
	AJFREE(node);

	if(!urlinQryProcess(urlin, url))
	    continue;

	ret = urlinRead(urlin, url);
	ajDebug("ajUrlinRead: list retry qry: '%S' returns: %B\n",
		urlin->Input->Qry, ret);
    }

    if(!ret)
    {
	if(listdata)
	    ajErr("Failed to read url data '%S'",
                  urlin->Input->Qry);

	return ajFalse;
    }


    urlDefine(url, urlin);

    return ajTrue;
}




/* @funcstatic urlDefine ******************************************************
**
** Make sure all url data object attributes are defined
** using values from the url input object if needed
**
** @param [w] thys [AjPUrl] Url data returned.
** @param [u] urlin [AjPUrlin] Url data input definitions
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool urlDefine(AjPUrl thys, AjPUrlin urlin)
{

    /* if values are missing in the url object, we can use defaults
       from urlin or calculate where possible */

    /* assign the dbname if defined in the urlin object */
    if(ajStrGetLen(urlin->Input->Db))
      ajStrAssignS(&thys->Db, urlin->Input->Db);

    return ajTrue;
}





/* @funcstatic urlinRead ******************************************************
**
** Given data in an urlin structure, tries to read everything needed
** using the specified format or by trial and error.
**
** @param [u] urlin [AjPUrlin] Url data input object
** @param [w] url [AjPUrl] Url data object
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool urlinRead(AjPUrlin urlin, AjPUrl url)
{
    AjPResource drcat = NULL;
    AjPResquery resqry = NULL;
    AjPReslink reslnk = NULL;
    AjPStr urlstr = NULL;
    AjIList iter;
/*    AjPTextin textin = urlin->Input;*/
    AjPQuery qry = urlin->Input->Query;
    AjPUrlAccess urlaccess = qry->Access;
    AjBool ret = ajFalse;
    ajuint nids = 0;
    ajuint ntids = 0;
    ajuint i;
    AjPStrTok handle = NULL;
    AjPStr snstr = NULL;
    AjPStr idstr = NULL;
    AjPStr tmpstr = NULL;

    ajUrlClear(url);

    ajDebug("urlinRead: swiss %B embl %B acc  '%S' idtype '%S' id '%S'\n",
            urlin->IsSwiss, urlin->IsEmbl,
            urlin->Accession, urlin->IdTypes,
            urlin->Identifiers);

    if(!urlin->Resource)
    {
        if(!(*urlaccess->Access)(urlin))
            return ajFalse;
    }

    drcat = urlin->Resource;
    if(!urlin->UrlList)
    {
        urlin->UrlList = ajListstrNew();
        urlin->QryList = ajListNew();

        if(urlin->IsSwiss || urlin->IsEmbl)
        {
            iter = ajListIterNew(drcat->Xref);
            while(!ajListIterDone(iter))
            {
                reslnk = ajListIterGet(iter);
                ajDebug("test reslnk '%S' : '%S'\n",
                        reslnk->Source, reslnk->Term);
                if(urlin->IsSwiss && ajStrPrefixC(reslnk->Source, "SP_"))
                {
                    ajStrAssignS(&urlin->IdTypes, reslnk->Term);
                    ajStrCutBraces(&urlin->IdTypes);
                    ajDebug("idtypes %S: '%S'\n",
                            reslnk->Source, urlin->IdTypes);
                }
                if(urlin->IsEmbl  && ajStrPrefixC(reslnk->Source, "EMBL_"))
                {
                    ajStrAssignS(&urlin->IdTypes, reslnk->Term);
                    ajStrCutBraces(&urlin->IdTypes);
                    ajDebug("idtypes %S: '%S'\n",
                            reslnk->Source, urlin->IdTypes);
                }
            }
            ajListIterDel(&iter);
        }
        if(ajStrGetLen(urlin->Accession))
        {
        }
        if(ajStrGetLen(urlin->IdTypes))
        {
            ajStrCutBraces(&urlin->IdTypes);
        }

        nids = 1 + (ajuint) ajStrCalcCountK(urlin->Identifiers, ';');

        iter = ajListIterNewread(drcat->Query);

        while(!ajListIterDone(iter)) 
        {
            resqry = ajListIterGet(iter);

            ajDebug("test qry %S | %S | %S\n",
                    resqry->Datatype, resqry->Format, resqry->Term);

            ntids = 1 + (ajuint) ajStrCalcCountK(resqry->Term, ';');
            if(ntids != nids)
                continue;

            if(!ajStrMatchC(resqry->Format, "HTML"))
                continue;

            if(ajStrGetLen(urlin->IdTypes))
            {
                if(!ajStrPrefixS(resqry->Term, urlin->IdTypes))
                    continue;
                if(!ajStrMatchS(resqry->Term, urlin->IdTypes))
                {
                    ajStrAssignS(&tmpstr, resqry->Term);
                    ajStrCutBraces(&tmpstr);
                    if(!ajStrMatchS(tmpstr, urlin->IdTypes))
                       continue;
                }
            }
            
            urlstr = ajStrNewS(resqry->Url);
            if(nids > 1)
            {
                handle = ajStrTokenNewC(urlin->Identifiers, ";");
                for(i=0; i<nids; i++)
                {
                    ajStrTokenNextParse(&handle, &idstr);
                    ajFmtPrintS(&snstr, "%%s%u", (i+1));
                    ajStrExchangeSS(&urlstr, snstr, idstr);
                }
                ajStrTokenDel(&handle);
                ajStrDel(&snstr);
                ajStrDel(&idstr);
            }
            ajStrExchangeCS(&urlstr, "%s", urlin->Identifiers);
            if(ajStrGetLen(urlin->Accession))
                ajStrExchangeCS(&urlstr, "%u", urlin->Accession);

            ajDebug("save query '%S'\n", urlstr);

            ajListPushAppend(urlin->UrlList, urlstr);
            urlstr = NULL;
            ajListPushAppend(urlin->QryList, ajResqueryNewResquery(resqry));
        }

        ajListIterDel(&iter);
    }

    if(ajListGetLength(urlin->UrlList))
    {
        ajListstrPop(urlin->UrlList, &url->Full);
        ajListPop(urlin->QryList, (void**) &url->Resqry);
        ajStrAssignS(&url->Id, urlin->Identifiers);
        ret = ajTrue;
    }

    if(!urlin->UrlList)
    {
        ajListstrFree(&urlin->UrlList);
    }

    ajStrDel(&tmpstr);
    return ret;
}




/* @funcstatic urlinReadHtml **************************************************
**
** Given data in an url structure, tries to read everything needed
** using HTML format.
**
** @param [u] urlin [AjPUrlin] Url input object
** @param [w] url [AjPUrl] url object
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool urlinReadHtml(AjPUrlin urlin, AjPUrl url)
{
    AjPFilebuff buff;

    ajlong fpos     = 0;
    ajuint linecnt = 0;

    ajDebug("urlinReadHtml\n");
    ajUrlClear(url);
    buff = urlin->Input->Filebuff;

    /* ajFilebuffTrace(buff); */

    while (ajBuffreadLinePos(buff, &urlinReadLine, &fpos))
    {
        linecnt++;

        if(ajStrGetCharLast(urlinReadLine) == '\n')
            ajStrCutEnd(&urlinReadLine, 1);

        if(ajStrGetCharLast(urlinReadLine) == '\r')
            ajStrCutEnd(&urlinReadLine, 1);

        ajDebug("line %u:%S\n", linecnt, urlinReadLine);

        /* add line to AjPUrl object */
    }

    return ajTrue;
}




/* @datasection [none] Miscellaneous ******************************************
**
** Url input internals
**
** @nam2rule Urlin Url input
**
******************************************************************************/




/* @section Printing **********************************************************
**
** Printing details of the internals to a file
**
** @fdata [none]
**
** @nam2rule Urlinprint
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




/* @func ajUrlinprintBook *****************************************************
**
** Reports the internal data structures as a Docbook table
**
** @param [u] outf [AjPFile] Output file
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajUrlinprintBook(AjPFile outf)
{
    ajuint i = 0;
    ajuint j = 0;
    AjPStr namestr = NULL;
    AjPList fmtlist;
    AjPStr* names;

    fmtlist = ajListstrNew();

    ajFmtPrintF(outf, "<para>The supported url formats are summarised "
                "in the table below. "
                "The columns are as follows: "
                "<emphasis>Input format</emphasis> (format name), "
                "<emphasis>Try</emphasis> (indicates whether the "
                "format can be detected automatically on input), and "
                "<emphasis>Description</emphasis> (short description of "
                "the format).</para>\n\n");

    ajFmtPrintF(outf, "<table frame=\"box\" rules=\"cols\">\n");
    ajFmtPrintF(outf, "  <caption>Input url formats</caption>\n");
    ajFmtPrintF(outf, "  <thead>\n");
    ajFmtPrintF(outf, "    <tr align=\"center\">\n");
    ajFmtPrintF(outf, "      <th>Input Format</th>\n");
    ajFmtPrintF(outf, "      <th>Try</th>\n");
    ajFmtPrintF(outf, "      <th>Description</th>\n");
    ajFmtPrintF(outf, "    </tr>\n");
    ajFmtPrintF(outf, "  </thead>\n");
    ajFmtPrintF(outf, "  <tbody>\n");

    for(i=1; urlinFormatDef[i].Name; i++)
    {
	if(!urlinFormatDef[i].Alias)
        {
            namestr = ajStrNewC(urlinFormatDef[i].Name);
            ajListPushAppend(fmtlist, namestr);
            namestr = NULL;
        }
    }

    ajListSort(fmtlist, &ajStrVcmp);
    ajListstrToarray(fmtlist, &names);

    for(i=0; names[i]; i++)
    {
        for(j=0; urlinFormatDef[j].Name; j++)
        {
            if(ajStrMatchC(names[i],urlinFormatDef[j].Name))
            {
                ajFmtPrintF(outf, "    <tr>\n");
                ajFmtPrintF(outf, "      <td>%s</td>\n",
                            urlinFormatDef[j].Name);
                ajFmtPrintF(outf, "      <td>%B</td>\n",
                            urlinFormatDef[j].Try);
                ajFmtPrintF(outf, "      <td>%s</td>\n",
                            urlinFormatDef[j].Desc);
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




/* @func ajUrlinprintHtml *****************************************************
**
** Reports the internal data structures as an HTML table
**
** @param [u] outf [AjPFile] Output file
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajUrlinprintHtml(AjPFile outf)
{
    ajuint i = 0;
    ajuint j = 0;

    AjPStr namestr = NULL;

    ajFmtPrintF(outf, "<table border=3>");
    ajFmtPrintF(outf, "<tr><th>Input Format</th><th>Auto</th>\n");
    ajFmtPrintF(outf, "<th>Multi</th><th>Description</th></tr>\n");

    for(i=1; urlinFormatDef[i].Name; i++)
    {
        ajStrAssignC(&namestr, urlinFormatDef[i].Name);

	if(!urlinFormatDef[i].Alias)
        {
            for(j=i+1; urlinFormatDef[j].Name; j++)
            {
                if(urlinFormatDef[j].Read == urlinFormatDef[i].Read)
                {
                    ajFmtPrintAppS(&namestr, " %s",
                                   urlinFormatDef[j].Name);
                    if(!urlinFormatDef[j].Alias) 
                    {
                        ajWarn("Input format '%s' same as '%s' but not alias",
                               urlinFormatDef[j].Name,
                               urlinFormatDef[i].Name);
                    }
                }
            }

	    ajFmtPrintF(outf, "<tr><td>\n%S\n</td><td>%B</td>\n",
                        namestr,
			urlinFormatDef[i].Try);
            ajFmtPrintF(outf, "<td>\n%s\n</td></tr>\n",
			urlinFormatDef[i].Desc);
        }

    }

    ajFmtPrintF(outf, "</table>\n");
    ajStrDel(&namestr);

    return;
}




/* @func ajUrlinprintText *****************************************************
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

void ajUrlinprintText(AjPFile outf, AjBool full)
{
    ajuint i = 0;

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# Url input formats\n");
    ajFmtPrintF(outf, "# Name  Format name (or alias)\n");
    ajFmtPrintF(outf, "# Alias Alias name\n");
    ajFmtPrintF(outf, "# Try   Test for unknown input files\n");
    ajFmtPrintF(outf, "# Name         Alias Try "
		"Description");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "InFormat {\n");

    for(i=0; urlinFormatDef[i].Name; i++)
	if(full || !urlinFormatDef[i].Alias)
	    ajFmtPrintF(outf,
			"  %-12s %5B %3B \"%s\"\n",
			urlinFormatDef[i].Name,
			urlinFormatDef[i].Alias,
			urlinFormatDef[i].Try,
			urlinFormatDef[i].Desc);

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajUrlinprintWiki *****************************************************
**
** Reports the internal data structures as a wiki table
**
** @param [u] outf [AjPFile] Output file
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajUrlinprintWiki(AjPFile outf)
{
    ajuint i = 0;
    ajuint j = 0;

    AjPStr namestr = NULL;

    ajFmtPrintF(outf, "{| class=\"wikitable sortable\" border=\"2\"\n");
    ajFmtPrintF(outf, "|-\n");
    ajFmtPrintF(outf, "!Format!!Try!!"
                "class=\"unsortable\"|Description\n");

    for(i=1; urlinFormatDef[i].Name; i++)
    {
        ajStrAssignC(&namestr, urlinFormatDef[i].Name);

	if(!urlinFormatDef[i].Alias)
        {
            for(j=i+1; urlinFormatDef[j].Name; j++)
            {
                if(urlinFormatDef[j].Read == urlinFormatDef[i].Read)
                {
                    ajFmtPrintAppS(&namestr, "<br>%s",
                                   urlinFormatDef[j].Name);
                    if(!urlinFormatDef[j].Alias) 
                    {
                        ajWarn("Input format '%s' same as '%s' but not alias",
                               urlinFormatDef[j].Name,
                               urlinFormatDef[i].Name);
                    }
                }
            }

            ajFmtPrintF(outf, "|-\n");
	    ajFmtPrintF(outf,
			"|%S||%B||%s\n",
			namestr,
			urlinFormatDef[i].Try,
			urlinFormatDef[i].Desc);
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




/* @func ajUrlinExit **********************************************************
**
** Cleans up url input internal memory
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajUrlinExit(void)
{
    ajStrDel(&urlinReadLine);

    ajTableDel(&urlDbMethods);

    return;
}




/* @section Internals *********************************************************
**
** Functions to return internal values
**
** @fdata [none]
**
** @nam3rule Type Internals for url datatype
** @nam4rule Get  Return a value
** @nam5rule Fields  Known query fields for ajUrlinRead
** @nam5rule Qlinks  Known query link operators for ajUrlinRead
**
** @valrule * [const char*] Internal value
**
** @fcategory misc
**
******************************************************************************/




/* @func ajUrlinTypeGetFields *************************************************
**
** Returns the listof known field names for ajUrlinRead
**
** @return [const char*] List of field names
**
** @release 6.4.0
** @@
******************************************************************************/

const char* ajUrlinTypeGetFields(void)
{
    return "id acc";
}




/* @func ajUrlinTypeGetQlinks *************************************************
**
** Returns the list of known query link operators for ajUrlinRead
**
** @return [const char*] List of field names
**
** @release 6.4.0
** @@
******************************************************************************/

const char* ajUrlinTypeGetQlinks(void)
{
    return "|";
}




/* @datasection [AjPTable] Internal call register table ***********************
**
** Functions to manage the internal call register table that links the
** ajaxdb library functions with code in the core AJAX library.
**
** @nam2rule Urlaccess Functions to manage urldb call tables.
**
******************************************************************************/




/* @section Cast **************************************************************
**
** Return a reference to the call table
**
** @fdata [AjPTable] urldb functions call table
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




/* @func ajUrlaccessGetDb *****************************************************
**
** Returns the table in which url database access details are registered
**
** @return [AjPTable] Access functions hash table
**
** @release 6.4.0
** @@
******************************************************************************/

AjPTable ajUrlaccessGetDb(void)
{
    if(!urlDbMethods)
        urlDbMethods = ajCallTableNew();
    return urlDbMethods;
    
}




/* @func ajUrlaccessMethodGetQlinks *******************************************
**
** Tests for a named method for url data reading returns the 
** known query link operators
**
** @param [r] method [const AjPStr] Method required.
** @return [const char*] Known link operators
**
** @release 6.4.0
** @@
******************************************************************************/

const char* ajUrlaccessMethodGetQlinks(const AjPStr method)
{
    AjPUrlAccess methoddata; 

    methoddata = ajCallTableGetS(urlDbMethods, method);
    if(!methoddata)
        return NULL;

    return methoddata->Qlink;
}




/* @func ajUrlaccessMethodGetScope ********************************************
**
** Tests for a named method for url data reading and returns the scope
** (entry, query or all).
*
** @param [r] method [const AjPStr] Method required.
** @return [ajuint] Scope flags
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ajUrlaccessMethodGetScope(const AjPStr method)
{
    AjPUrlAccess methoddata; 
    ajuint ret = 0;

    methoddata = ajCallTableGetS(urlDbMethods, method);
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




/* @func ajUrlaccessMethodTest ************************************************
**
** Tests for a named method for url data reading.
**
** @param [r] method [const AjPStr] Method required.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajUrlaccessMethodTest(const AjPStr method)
{
    if(ajCallTableGetS(urlDbMethods, method))
      return ajTrue;

    return ajFalse;
}




/* @funcstatic urlinQryRestore ************************************************
**
** Restores an url input specification from an AjPQueryList node
**
** @param [w] urlin [AjPUrlin] Url input object
** @param [r] node [const AjPQueryList] Query list node
** @return [void]
**
** @release 6.4.0
******************************************************************************/

static void urlinQryRestore(AjPUrlin urlin, const AjPQueryList node)
{
    urlin->Input->Format = node->Format;
    ajStrAssignS(&urlin->Input->Formatstr, node->Formatstr);

    return;
}




/* @funcstatic urlinQrySave ***************************************************
**
** Saves an url input specification in an AjPQueryList node
**
** @param [w] node [AjPQueryList] Query list node
** @param [r] urlin [const AjPUrlin] Url input object
** @return [void]
**
** @release 6.4.0
******************************************************************************/

static void urlinQrySave(AjPQueryList node, const AjPUrlin urlin)
{
    node->Format   = urlin->Input->Format;
    ajStrAssignS(&node->Formatstr, urlin->Input->Formatstr);

    return;
}




/* @funcstatic urlinQryProcess ************************************************
**
** Converts an url data query into an open file.
**
** Tests for "format::" and sets this if it is found
**
** Then tests for "list:" or "@" and processes as a list file
** using urlinListProcess which in turn invokes urlinQryProcess
** until a valid query is found.
**
** Then tests for dbname:query and opens the file (at the correct position
** if the database definition defines it)
**
** If there is no database, looks for file:query and opens the file.
** In this case the file position is not known and url data reading
** will have to scan for the entry/entries we need.
**
** @param [u] urlin [AjPUrlin] Url data input structure.
** @param [u] url [AjPUrl] Url data to be read.
**                         The format will be replaced
**                         if defined in the query string.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool urlinQryProcess(AjPUrlin urlin, AjPUrl url)
{
    AjBool ret = ajTrue;
    AjPStr qrystr = NULL;
    AjBool urlmethod = ajFalse;
    const AjPStr fmtstr = NULL;
    AjPTextin textin;
    AjPQuery qry;
    AjPUrlAccess urlaccess = NULL;

    textin = urlin->Input;
    qry = textin->Query;

    /* pick up the original query string */
    qrystr = ajStrNewS(textin->Qry);

    ajDebug("urlinQryProcess '%S'\n", qrystr);

    /* look for a format:: prefix */
    fmtstr = ajQuerystrParseFormat(&qrystr, textin, urlinformatFind);
    ajDebug("urlinQryProcess ... fmtstr '%S' '%S'\n", fmtstr, qrystr);

    /* (seq/feat) DO NOT look for a [range] suffix */

    /* look for a list:: or @:: listfile of queries  - process and return */
    if(ajQuerystrParseListfile(&qrystr))
    {
        ajDebug("urlinQryProcess ... listfile '%S'\n", qrystr);
        ret = urlinListProcess(urlin, url, qrystr);
        ajStrDel(&qrystr);
        return ret;
    }

    /* try general text access methods (file, asis, text database access */
    ajDebug("urlinQryProcess ... no listfile '%S'\n", qrystr);
    if(!ajQuerystrParseRead(&qrystr, textin, urlinformatFind, &urlmethod))
    {
        ajStrDel(&qrystr);
        return ajFalse;
    }
    
    urlinFormatSet(urlin, url);

    ajDebug("urlinQryProcess ... read nontext: %B '%S'\n",
            urlmethod, qrystr);
    ajStrDel(&qrystr);

    /* we found a non-text method */
    if(urlmethod)
    {
        ajDebug("urlinQryProcess ... call method '%S'\n", qry->Method);
        ajDebug("urlinQryProcess ... textin format %d '%S'\n",
                textin->Format, textin->Formatstr);
        ajDebug("urlinQryProcess ...  query format  '%S'\n",
                qry->Formatstr);
        qry->Access = ajCallTableGetS(urlDbMethods,qry->Method);
        urlaccess = qry->Access;
        return (*urlaccess->Access)(urlin);
    }

    ajDebug("urlinQryProcess text method '%S' success\n", qry->Method);

    return ajTrue;
}





/* @datasection [AjPList] Query field list ************************************
**
** Query fields lists are handled internally. Only static functions
** should appear here
**
******************************************************************************/




/* @funcstatic urlinListProcess ***********************************************
**
** Processes a file of queries.
** This function is called by, and calls, urlinQryProcess. There is
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
** @param [u] urlin [AjPUrlin] Url data input
** @param [u] url [AjPUrl] Url data
** @param [r] listfile [const AjPStr] Name of list file.,
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool urlinListProcess(AjPUrlin urlin, AjPUrl url,
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
    ajDebug("++urlinListProcess %S depth %d\n",
	    listfile, depth);

    if(depth > MAXDEPTH)
	ajFatal("Query list too deep");

    if(!urlin->Input->List)
	urlin->Input->List = ajListNew();

    list = ajListNew();

    file = ajFileNewInNameS(listfile);

    if(!file)
    {
	ajErr("Failed to open list file '%S'", listfile);
	depth--;

	return ret;
    }

    while(ajReadlineTrim(file, &urlinReadLine))
    {
	urlinListNoComment(&urlinReadLine);

	if(ajStrGetLen(urlinReadLine))
	{
	    ajStrTokenAssignC(&handle, urlinReadLine, " \t\n\r");
	    ajStrTokenNextParse(&handle, &token);
	    /* ajDebug("Line  '%S'\n");*/
	    /* ajDebug("token '%S'\n", urlinReadLine, token); */

	    if(ajStrGetLen(token))
	    {
	        ajDebug("++Add to list: '%S'\n", token);
	        AJNEW0(node);
	        ajStrAssignS(&node->Qry, token);
	        urlinQrySave(node, urlin);
	        ajListPushAppend(list, node);
	    }

	    ajStrDel(&token);
	    token = NULL;
	}
    }

    ajFileClose(&file);
    ajStrDel(&token);

    ajDebug("Trace urlin->Input->List\n");
    ajQuerylistTrace(urlin->Input->List);
    ajDebug("Trace new list\n");
    ajQuerylistTrace(list);
    ajListPushlist(urlin->Input->List, &list);

    ajDebug("Trace combined urlin->Input->List\n");
    ajQuerylistTrace(urlin->Input->List);

    /*
     ** now try the first item on the list
     ** this can descend recursively if it is also a list
     ** which is why we check the depth above
     */

    if(ajListPop(urlin->Input->List, (void**) &node))
    {
        ajDebug("++pop first item '%S'\n", node->Qry);
	ajUrlinQryS(urlin, node->Qry);
	urlinQryRestore(urlin, node);
	ajStrDel(&node->Qry);
	ajStrDel(&node->Formatstr);
	AJFREE(node);
	ajDebug("descending with query '%S'\n", urlin->Input->Qry);
	ret = urlinQryProcess(urlin, url);
    }

    ajStrTokenDel(&handle);
    depth--;
    ajDebug("++urlinListProcess depth: %d returns: %B\n", depth, ret);

    return ret;
}




/* @funcstatic urlinListNoComment *********************************************
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

static void urlinListNoComment(AjPStr* text)
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




/* @funcstatic urlinFormatSet *************************************************
**
** Sets the input format for url data using the url data
** input object's defined format
**
** @param [u] urlin [AjPUrlin] Url data input.
** @param [u] url [AjPUrl] Url data
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool urlinFormatSet(AjPUrlin urlin, AjPUrl url)
{

    if(ajStrGetLen(urlin->Input->Formatstr))
    {
	ajDebug("... input format value '%S'\n",
                urlin->Input->Formatstr);

	if(urlinformatFind(urlin->Input->Formatstr,
                             &urlin->Input->Format))
	{
	    ajStrAssignS(&url->Formatstr,
                         urlin->Input->Formatstr);
	    url->Format = urlin->Input->Format;
	    ajDebug("...format OK '%S' = %d\n",
                    urlin->Input->Formatstr,
		    urlin->Input->Format);
	}
	else
	    ajDebug("...format unknown '%S'\n",
                    urlin->Input->Formatstr);

	return ajTrue;
    }
    else
	ajDebug("...input format not set\n");


    return ajFalse;
}




/* @datasection [AjPUrlall] Url Input Stream ********************************
**
** Function is for manipulating url input stream objects
**
** @nam2rule Urlall Url input stream objects
**
******************************************************************************/




/* @section Url Input Constructors ******************************************
**
** All constructors return a new url input stream object by pointer. It
** is the responsibility of the user to first destroy any previous
** url input object. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so
** anyway.
**
** @fdata [AjPUrlall]
**
** @nam3rule New Constructor
**
** @valrule * [AjPUrlall] Url input stream object
**
** @fcategory new
**
******************************************************************************/




/* @func ajUrlallNew **********************************************************
**
** Creates a new url input stream object.
**
** @return [AjPUrlall] New url input stream object.
**
** @release 6.4.0
** @@
******************************************************************************/

AjPUrlall ajUrlallNew(void)
{
    AjPUrlall pthis;

    AJNEW0(pthis);

    pthis->Urlin = ajUrlinNew();
    pthis->Url   = ajUrlNew();

    return pthis;
}





/* ==================================================================== */
/* ========================== destructors ============================= */
/* ==================================================================== */




/* @section Url Input Stream Destructors ************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the url input stream object.
**
** @fdata [AjPUrlall]
**
** @nam3rule Del Destructor
**
** @argrule Del pthis [AjPUrlall*] Url input stream
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajUrlallDel **********************************************************
**
** Deletes a url input stream object.
**
** @param [d] pthis [AjPUrlall*] Url input stream
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajUrlallDel(AjPUrlall* pthis)
{
    AjPUrlall thys;

    if(!pthis)
        return;

    thys = *pthis;

    if(!thys)
        return;

    ajUrlinDel(&thys->Urlin);
    if(!thys->Returned)
        ajUrlDel(&thys->Url);

    AJFREE(*pthis);

    return;
}




/* ==================================================================== */
/* =========================== Modifiers ============================== */
/* ==================================================================== */




/* @section Url input stream modifiers **************************************
**
** These functions use the contents of a url input stream object and
** update them.
**
** @fdata [AjPUrlall]
**
** @nam3rule Clear Clear all values
**
** @argrule * thys [AjPUrlall] Url input stream object
**
** @valrule * [void]
**
** @fcategory modify
**
******************************************************************************/




/* @func ajUrlallClear ********************************************************
**
** Clears a url input stream object back to "as new" condition, except
** for the query list which must be preserved.
**
** @param [w] thys [AjPUrlall] Url input stream
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajUrlallClear(AjPUrlall thys)
{

    ajDebug("ajUrlallClear called\n");

    ajUrlinClear(thys->Urlin);

    ajUrlClear(thys->Url);

    thys->Returned = ajFalse;

    return;
}




/* @section Url input stream casts ******************************************
**
** These functions return the contents of a url input stream object
**
** @fdata [AjPUrlall]
**
** @nam3rule Get Get url input stream values
** @nam3rule Geturl Get url values
** @nam4rule GeturlId Get url identifier
**
** @argrule * thys [const AjPUrlall] Url input stream object
**
** @valrule * [const AjPStr] String value
**
** @fcategory cast
**
******************************************************************************/




/* @func ajUrlallGeturlId *****************************************************
**
** Returns the identifier of the current url in an input stream
**
** @param [r] thys [const AjPUrlall] Url input stream
** @return [const AjPStr] Identifier
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPStr ajUrlallGeturlId(const AjPUrlall thys)
{
    if(!thys)
        return NULL;

    ajDebug("ajUrlallGeturlId called\n");

    return ajUrlGetId(thys->Url);
}




/* @section url input *******************************************************
**
** These functions use a url input stream object to read data
**
** @fdata [AjPUrlall]
**
** @nam3rule Next Read next url
**
** @argrule * thys [AjPUrlall] Url input stream object
** @argrule * Purl [AjPUrl*] Url object
**
** @valrule * [AjBool] True on success
**
** @fcategory input
**
******************************************************************************/




/* @func ajUrlallNext *********************************************************
**
** Parse a url query into format, access, file and entry
**
** Split at delimiters. Check for the first part as a valid format
** Check for the remaining first part as a database name or as a file
** that can be opened.
** Anything left is an entryname spec.
**
** Return the results in the AjPUrl object but leave the file open for
** future calls.
**
** @param [w] thys [AjPUrlall] Url input stream
** @param [u] Purl [AjPUrl*] Url returned
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajUrlallNext(AjPUrlall thys, AjPUrl *Purl)
{
    ajDebug("ajUrlallNext count:%u\n", thys->Count);

    if(!thys->Count)
    {
	thys->Count = 1;

	thys->Totterms++;

	*Purl = thys->Url;
	thys->Returned = ajTrue;

	return ajTrue;
    }


    if(ajUrlinRead(thys->Urlin, thys->Url))
    {
	thys->Count++;

	thys->Totterms++;

	*Purl = thys->Url;
	thys->Returned = ajTrue;

	ajDebug("ajUrlallNext success\n");

	return ajTrue;
    }

    *Purl = NULL;

    ajDebug("ajUrlallNext failed\n");

    ajUrlallClear(thys);

    return ajFalse;
}




/* @datasection [none] Input formats ******************************************
**
** Input formats internals
**
** @nam2rule Urlinformat Url data input format specific
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




/* @funcstatic urlinformatFind ************************************************
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

static AjBool urlinformatFind(const AjPStr format, ajint* iformat)
{
    AjPStr tmpformat = NULL;
    ajuint i = 0;

    /* ajDebug("urlinformatFind '%S'\n", format); */
    if(!ajStrGetLen(format))
	return ajFalse;

    ajStrAssignS(&tmpformat, format);
    ajStrFmtLower(&tmpformat);

    for(i=0; urlinFormatDef[i].Name; i++)
    {
	/* ajDebug("test %d '%s' '%s' '%s'\n",
           i, urlinFormatDef[i].Name,
           urlinFormatDef[i].Obo,
           urlinFormatDef[i].Desc); */
	if(ajStrMatchCaseC(tmpformat, urlinFormatDef[i].Name) ||
           ajStrMatchC(format, urlinFormatDef[i].Obo))
	{
	    *iformat = i;
	    ajStrDel(&tmpformat);
	    /* ajDebug("found '%s' at %d\n", urlinFormatDef[i].Name, i); */
	    return ajTrue;
	}
    }

    ajErr("Unknown input format '%S'", format);

    ajStrDel(&tmpformat);

    return ajFalse;
}




/* @func ajUrlinformatTerm ****************************************************
**
** Tests whether a url data input format term is known
**
** @param [r] term [const AjPStr] Format term EDAM ID
** @return [AjBool] ajTrue if term was accepted
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajUrlinformatTerm(const AjPStr term)
{
    ajuint i;

    for(i=0; urlinFormatDef[i].Name; i++)
	if(ajStrMatchC(term, urlinFormatDef[i].Obo))
	    return ajTrue;

    return ajFalse;
}




/* @func ajUrlinformatTest ****************************************************
**
** Tests whether a named url data input format is known
**
** @param [r] format [const AjPStr] Format
** @return [AjBool] ajTrue if format was accepted
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajUrlinformatTest(const AjPStr format)
{
    ajuint i;

    for(i=0; urlinFormatDef[i].Name; i++)
    {
	if(ajStrMatchCaseC(format, urlinFormatDef[i].Name))
	    return ajTrue;
	if(ajStrMatchC(format, urlinFormatDef[i].Obo))
	    return ajTrue;
    }

    return ajFalse;
}

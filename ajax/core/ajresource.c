/* @source ajresource *********************************************************
**
** AJAX data resource functions
**
** These functions control all aspects of AJAX data resource
** parsing and include simple utilities.
**
** @author Copyright (C) 2010 Peter Rice
** @version  $Revision: 1.32 $
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

#include "ajresource.h"
#include "ajlist.h"
#include "ajresourceread.h"
#include "ajresourcewrite.h"
#include "ajnam.h"

static AjPStr resourceTempQry = NULL;
static void resourceMakeQry(const AjPResource thys, AjPStr* qry);





/* @filesection ajresource ****************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
******************************************************************************/





/* @datasection [AjPResource] Data resource data ******************************
**
** Function is for manipulating data resource data objects
**
** @nam2rule Resource Data resource data objects
**
******************************************************************************/




/* @section constructors ******************************************************
**
** Constructors
**
** @fdata [AjPResource]
**
** @nam3rule New Constructor
** @nam4rule NewDrcat Constructor by retrieval from DRCAT
** @nam4rule NewResource Copy constructor
**
** @argrule NewDrcat dbname [const AjPStr] DRCAT identifier
** @argrule NewResource res [const AjPResource] Original resource object
**
** @valrule * [AjPResource] Data resource data object
**
** @fcategory new
**
******************************************************************************/




/* @func ajResourceNew ********************************************************
**
** Resource data constructor
**
** @return [AjPResource] New object
**
** @release 6.4.0
** @@
******************************************************************************/

AjPResource ajResourceNew(void)
{
    AjPResource ret;

    AJNEW0(ret);

    ret->Id       = ajStrNew();
    ret->Idalt    = ajListstrNew();
    ret->Acc      = ajStrNew();
    ret->Name     = ajStrNew();
    ret->Desc     = ajStrNew();
    ret->Url      = ajStrNew();
    ret->Urllink  = ajStrNew();
    ret->Urlrest  = ajStrNew();
    ret->Urlsoap  = ajStrNew();
    ret->Cat      = ajListstrNew();
    ret->Taxon    = ajListNew();
    ret->Edamdat  = ajListNew();
    ret->Edamfmt  = ajListNew();
    ret->Edamid   = ajListNew();
    ret->Edamtpc  = ajListNew();
    ret->Xref     = ajListNew();
    ret->Query    = ajListNew();
    ret->Example  = ajListstrNew();

    return ret;
}




/* @func ajResourceNewDrcat ***************************************************
**
** Resource data constructor for an entry in DRCAT
**
** @param [r] dbname [const AjPStr] DRCAT identifier
** @return [AjPResource] New object
**
** @release 6.4.0
** @@
******************************************************************************/

AjPResource ajResourceNewDrcat(const AjPStr dbname)
{
    AjPResource drcat = NULL;
    AjPResourcein drcatin = NULL;

    drcat = ajResourceNew();
    drcatin = ajResourceinNewDrcat(dbname);
    if(!ajResourceinRead(drcatin, drcat))
        ajResourceDel(&drcat);

    ajResourceinDel(&drcatin);

    return drcat;
}




/* @func ajResourceNewResource ************************************************
**
** Copy constructor for a public resource object
**
** @param [r] res [const AjPResource] Resource object
** @return [AjPResource] Resource object
**
**
** @release 6.4.0
******************************************************************************/

AjPResource ajResourceNewResource(const AjPResource res)
{
    AjPResource ret;

    AJNEW0(ret);

    ret->Id       = ajStrNewS(res->Id);
    ret->Acc      = ajStrNewS(res->Acc);
    ret->Name     = ajStrNewS(res->Name);
    ret->Desc     = ajStrNewS(res->Desc);
    ret->Url      = ajStrNewS(res->Url);

    ret->Cat     = ajListstrNewList(res->Cat);
    ret->Idalt   = ajListstrNewList(res->Idalt);
    ret->Taxon   = ajListNew();
    ret->Edamdat = ajListNew();
    ret->Edamfmt = ajListNew();
    ret->Edamid  = ajListNew();
    ret->Edamtpc = ajListNew();
    ajRestermlistClone(res->Taxon,   ret->Taxon);
    ajRestermlistClone(res->Edamdat, ret->Edamdat);
    ajRestermlistClone(res->Edamfmt, ret->Edamfmt);
    ajRestermlistClone(res->Edamid,  ret->Edamid);
    ajRestermlistClone(res->Edamtpc, ret->Edamtpc);
    ret->Xref    = ajListNew();
    ajReslinklistClone(res->Xref,    ret->Xref);
    ret->Query   = ajListNew();
    ajResquerylistClone(res->Query,  ret->Query);
    ret->Example = ajListstrNewList(res->Example);

    return ret;
}




/* @section Data resource data destructors ************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the data resource data object.
**
** @fdata [AjPResource]
**
** @nam3rule Del Destructor
**
** @argrule Del Presource [AjPResource*] Data resource data
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajResourceDel ********************************************************
**
** Data resource data destructor
**
** @param [d] Presource  [AjPResource*] Data resource data object to delete
** @return [void] 
**
** @release 6.4.0
** @@
******************************************************************************/

void ajResourceDel(AjPResource *Presource)
{
    AjPResource thys;
    AjPReslink  reslink;
    AjPResquery resqry;
    AjPResterm  resterm;
    AjIList iter;

    if(!Presource) return;
    if(!(*Presource)) return;

    thys = *Presource;

    ajStrDel(&thys->Id);
    ajListstrFreeData(&thys->Idalt);
    ajStrDel(&thys->Acc);
    ajStrDel(&thys->Name);
    ajStrDel(&thys->Desc);
    ajStrDel(&thys->Url);
    ajStrDel(&thys->Urllink);
    ajStrDel(&thys->Urlrest);
    ajStrDel(&thys->Urlsoap);
    ajListstrFreeData(&thys->Cat);

    iter = ajListIterNewread(thys->Taxon);
    while(!ajListIterDone(iter))
    {
        resterm = ajListIterGet(iter);
        ajRestermDel(&resterm);
    }
    ajListIterDel(&iter);
    ajListFree(&thys->Taxon);

    iter = ajListIterNewread(thys->Edamdat);
    while(!ajListIterDone(iter))
    {
        resterm = ajListIterGet(iter);
        ajRestermDel(&resterm);
    }
    ajListIterDel(&iter);
    ajListFree(&thys->Edamdat);

    iter = ajListIterNewread(thys->Edamfmt);
    while(!ajListIterDone(iter))
    {
        resterm = ajListIterGet(iter);
        ajRestermDel(&resterm);
    }
    ajListIterDel(&iter);
    ajListFree(&thys->Edamfmt);

    iter = ajListIterNewread(thys->Edamid);
    while(!ajListIterDone(iter))
    {
        resterm = ajListIterGet(iter);
        ajRestermDel(&resterm);
    }
    ajListIterDel(&iter);
    ajListFree(&thys->Edamid);

    iter = ajListIterNewread(thys->Edamtpc);
    while(!ajListIterDone(iter))
    {
        resterm = ajListIterGet(iter);
        ajRestermDel(&resterm);
    }
    ajListIterDel(&iter);
    ajListFree(&thys->Edamtpc);

    iter = ajListIterNewread(thys->Xref);
    while(!ajListIterDone(iter))
    {
        reslink = ajListIterGet(iter);
        ajReslinkDel(&reslink);
    }
    ajListIterDel(&iter);
    ajListFree(&thys->Xref);

    iter = ajListIterNewread(thys->Query);
    while(!ajListIterDone(iter))
    {
        resqry = ajListIterGet(iter);
        ajResqueryDel(&resqry);
    }
    ajListIterDel(&iter);
    ajListFree(&thys->Query);

    ajListstrFreeData(&thys->Example);
    ajStrDel(&thys->Db);
    ajStrDel(&thys->Setdb);
    ajStrDel(&thys->Full);
    ajStrDel(&thys->Qry);
    ajStrDel(&thys->Formatstr);
    ajStrDel(&thys->Filename);

    ajStrDel(&thys->TextPtr);

    AJFREE(*Presource);
    *Presource = NULL;

    return;
}




/* @section Casts *************************************************************
**
** Return values from a data resource data object
**
** @fdata [AjPResource]
**
** @nam3rule Get Return a value
** @nam3rule Valid Validate a resource object
** @nam4rule Dbdata Return query database definition
** @nam4rule Entry Return complete entry
** @nam4rule Id Return identifier
** @nam4rule Qry Return a query field
** @suffix C Character string result
** @suffix S String object result
**
** @argrule * resource [const AjPResource] Data resource data object.
** @argrule Dbdata qry [AjPQuery] Query object
** @argrule Dbdata findformat [AjBool function] Findformat function for
**                                              the query datatype
**
** @valrule * [AjBool] True on success
** @valrule *C [const char*] Query as a character string.
** @valrule *S [const AjPStr] Query as a string object.
** @valrule *Entry [const AjPStr] Entry text
** @valrule *Id [const AjPStr] Identifier
**
** @fcategory cast
**
******************************************************************************/




/* @func ajResourceGetDbdata **************************************************
**
** Returns a query database definition using a data resource data object.
**
** @param [r] resource [const AjPResource] Data resource data object.
** @param [w] qry [AjPQuery] Query object.
** @param [f] findformat [AjBool function] Find format function for datatype
** @return [AjBool] True on success
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajResourceGetDbdata(const AjPResource resource, AjPQuery qry,
                           AjBool findformat(const AjPStr format,
                                             ajint *iformat))
{
    AjIList iter;
    AjPResquery resqry = NULL;
    ajint format;
    AjBool ret = ajFalse;

    if(!resource)
        return ajFalse;

    qry->InDrcat = ajTrue;

    ajStrAssignC(&qry->DbType, ajNamQueryGetDatatypeC(qry));

    ajDebug("ajResourceGetDbdata dbtype %S %d\n", qry->DbType, qry->DataType);

    if(qry->DataType == AJDATATYPE_URL)
    {
        ajStrAssignC(&qry->Method, "urlonly");
        qry->QueryType = AJQUERY_ENTRY;
        qry->HasAcc = ajFalse;

        iter = ajListIterNewread(resource->Query);
        while(!ajListIterDone(iter)) 
        {
            resqry = ajListIterGet(iter);

            ajDebug("ajResourceGetDbdata test fmt: '%S' edam: '%S'\n",
                    resqry->Format, resqry->FormatTerm);

            if(ajStrMatchC(resqry->FormatTerm, "2331"))
            {
                ajDebug("     OK fmt: '%S' edam: '%S' url '%S'\n",
                        resqry->Format, resqry->FormatTerm, resqry->Url);
                ajStrAssignS(&qry->Formatstr, resqry->FormatTerm);
                ajStrAssignS(&qry->DbUrl, resqry->Url);
                ret = ajTrue;
            }
        }
        ajListIterDel(&iter);
    }
    else 
    {
        ajStrAssignC(&qry->Method, "url");
        qry->QueryType = AJQUERY_ENTRY;
        qry->HasAcc = ajFalse;

        iter = ajListIterNewread(resource->Query);
        while(!ajListIterDone(iter)) 
        {
            resqry = ajListIterGet(iter);

            ajDebug("ajResourceGetDbdata test fmt: '%S' edam: '%S'\n",
                    resqry->Format, resqry->FormatTerm);

            if(findformat(resqry->Format, &format))
            {
                ajDebug("     OK fmt: '%S' url '%S'\n",
                        resqry->Format, resqry->Url);
                ajStrAssignS(&qry->Formatstr, resqry->Format);
                ajStrAssignS(&qry->DbUrl, resqry->Url);
                ret = ajTrue;
            }
            if(ajStrGetLen(resqry->FormatTerm) &&
               findformat(resqry->FormatTerm, &format))
            {
                ajDebug("     OK edam: '%S' url '%S'\n",
                        resqry->FormatTerm, resqry->Url);
                ajStrAssignS(&qry->Formatstr, resqry->FormatTerm);
                ajStrAssignS(&qry->DbUrl, resqry->Url);
                ret = ajTrue;
            }
        }
        ajListIterDel(&iter);
    }

    return ret;
}




/* @func ajResourceGetEntry ***************************************************
**
** Return the complete text
**
** @param [r] resource [const AjPResource] Data resource
**
** @return [const AjPStr] Returned entry text
**
**
** @release 6.4.0
******************************************************************************/

const AjPStr ajResourceGetEntry(const AjPResource resource)
{
    if(resource->TextPtr)
        return resource->TextPtr;

    return ajStrConstEmpty();
}




/* @func ajResourceGetId ******************************************************
**
** Return the identifier
**
** @param [r] resource [const AjPResource] Data resource
**
** @return [const AjPStr] Returned id
**
**
** @release 6.4.0
******************************************************************************/

const AjPStr ajResourceGetId(const AjPResource resource)
{
    return resource->Id;
}




/* @func ajResourceGetQryC ****************************************************
**
** Returns the query string of a data resource data object.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] resource [const AjPResource] Data resource data object.
** @return [const char*] Query as a character string.
**
** @release 6.4.0
** @@
******************************************************************************/

const char* ajResourceGetQryC(const AjPResource resource)
{
    return MAJSTRGETPTR(ajResourceGetQryS(resource));
}




/* @func ajResourceGetQryS ****************************************************
**
** Returns the query string of a data resource data object.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] resource [const AjPResource] Data resource data object.
** @return [const AjPStr] Query as a string.
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPStr ajResourceGetQryS(const AjPResource resource)
{
    ajDebug("ajResourceGetQryS '%S'\n", resource->Qry);

    if(ajStrGetLen(resource->Qry))
	return resource->Qry;

    resourceMakeQry(resource, &resourceTempQry);

    return resourceTempQry;
}




/* @funcstatic resourceMakeQry ************************************************
**
** Sets the query for a data resource data object.
**
** @param [r] thys [const AjPResource] Data resource data object
** @param [w] qry [AjPStr*] Query string in full
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void resourceMakeQry(const AjPResource thys, AjPStr* qry)
{
    ajDebug("resourceMakeQry (Id <%S> Formatstr <%S> Db <%S> "
	    "Filename <%S>)\n",
	    thys->Id, thys->Formatstr, thys->Db,
	    thys->Filename);

    /* ajResourceTrace(thys); */

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




/* @section data resource data modifiers **************************************
**
** Data resource data modifiers
**
** @fdata [AjPResource]
**
** @nam3rule Clear clear internal values
**
** @argrule * resource [AjPResource] Data resource data object
**
** @valrule * [void]
**
** @fcategory modify
**
******************************************************************************/




/* @func ajResourceClear ******************************************************
**
** Resets all data for a data resource data object so that it can be reused.
**
** @param [u] resource [AjPResource] Data resource data object
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajResourceClear(AjPResource resource)
{
    AjPReslink lnk = NULL;
    AjPResquery qry = NULL;
    AjPResterm resterm = NULL;
    AjPStr ptr = NULL;

    if(MAJSTRGETLEN(resource->Id))
       ajStrSetClear(&resource->Id);

    if(ajListGetLength(resource->Idalt)) 
        while(ajListstrPop(resource->Idalt,&ptr))
            ajStrDel(&ptr);

    if(MAJSTRGETLEN(resource->Acc))
       ajStrSetClear(&resource->Acc);

    if(MAJSTRGETLEN(resource->Name))
       ajStrSetClear(&resource->Name);

    if(MAJSTRGETLEN(resource->Desc))
       ajStrSetClear(&resource->Desc);

    if(MAJSTRGETLEN(resource->Url))
       ajStrSetClear(&resource->Url);

    if(MAJSTRGETLEN(resource->Urllink))
       ajStrSetClear(&resource->Urllink);

    if(MAJSTRGETLEN(resource->Urlrest))
       ajStrSetClear(&resource->Urlrest);

    if(MAJSTRGETLEN(resource->Urlsoap))
       ajStrSetClear(&resource->Urlsoap);

    if(ajListGetLength(resource->Cat))
        while(ajListstrPop(resource->Cat,&ptr))
            ajStrDel(&ptr);

    if(ajListGetLength(resource->Taxon))
        while(ajListPop(resource->Taxon,(void**)&resterm))
            ajRestermDel(&resterm);

    if(ajListGetLength(resource->Edamdat))
        while(ajListPop(resource->Edamdat,(void**)&resterm))
            ajRestermDel(&resterm);

    if(ajListGetLength(resource->Edamfmt))
        while(ajListPop(resource->Edamfmt,(void**)&resterm))
            ajRestermDel(&resterm);

    if(ajListGetLength(resource->Edamid))
        while(ajListPop(resource->Edamid,(void**)&resterm))
            ajRestermDel(&resterm);

    if(ajListGetLength(resource->Edamtpc))
        while(ajListPop(resource->Edamtpc,(void**)&resterm))
            ajRestermDel(&resterm);

    if(ajListGetLength(resource->Xref))
        while(ajListPop(resource->Xref,(void**)&lnk))
            ajReslinkDel(&lnk);

    if(ajListGetLength(resource->Query))
        while(ajListPop(resource->Query,(void**)&qry))
            ajResqueryDel(&qry);

    if(ajListGetLength(resource->Example))
        while(ajListstrPop(resource->Example,&ptr))
            ajStrDel(&ptr);

    if(MAJSTRGETLEN(resource->Db))
       ajStrSetClear(&resource->Db);

    if(MAJSTRGETLEN(resource->Setdb))
       ajStrSetClear(&resource->Setdb);

    if(MAJSTRGETLEN(resource->Full))
       ajStrSetClear(&resource->Full);

    if(MAJSTRGETLEN(resource->Qry))
       ajStrSetClear(&resource->Qry);

    if(MAJSTRGETLEN(resource->Formatstr))
       ajStrSetClear(&resource->Formatstr);

    if(MAJSTRGETLEN(resource->Filename))
       ajStrSetClear(&resource->Filename);

    ajStrDel(&resource->TextPtr);

    resource->Count = 0;
    resource->Fpos = 0L;
    resource->Format = 0;

    return;
}




/* @section casts *************************************************************
**
** Return values
**
** @fdata [AjPResource]
**
** @nam3rule Trace Write debugging output
**
** @argrule * thys [const AjPResource] Data resource object
**
** @valrule * [void]
**
** @fcategory cast
**
******************************************************************************/




/* @func ajResourceTrace ******************************************************
**
** Write debug report of a resource object
**
** @param [r] thys [const AjPResource] Resource object
** @return [void]
**
**
** @release 6.4.0
******************************************************************************/

void ajResourceTrace(const AjPResource thys)
{
    AjIList iter;
    AjPStr tmpstr;
    ajuint i;
    AjPReslink reslink = NULL;
    AjPResquery resqry = NULL;
    AjPResterm resterm = NULL;
    if(!thys)
    {
        ajDebug("ajResourceTrace NULL\n");
        return;
    }

    ajDebug("\najResourceTrace\n");
    ajDebug("        Id: %S\n", thys->Id);
    ajDebug("       Acc: %S\n", thys->Acc);
    ajDebug("      Name: %S\n", thys->Name);
    ajDebug("      Desc: %S\n", thys->Desc);
    ajDebug("       Url: %S\n", thys->Url);

    ajDebug("     Idalt: %Lu\n", ajListGetLength(thys->Idalt));
    if(ajListGetLength(thys->Idalt))
    {
        i=0;
        iter = ajListIterNewread(thys->Idalt);
        while(!ajListIterDone(iter))
        {
            tmpstr = ajListIterGet(iter);
            ajDebug("%15d: %S\n", ++i, tmpstr);
        }
        ajListIterDel(&iter);
    }
    
    ajDebug("       Cat: %Lu\n", ajListGetLength(thys->Cat));
    if(ajListGetLength(thys->Cat))
    {
        i = 0;
        iter = ajListIterNewread(thys->Cat);
        while(!ajListIterDone(iter))
        {
            tmpstr = ajListIterGet(iter);
            ajDebug("%15d: %S\n", ++i, tmpstr);
        }
        ajListIterDel(&iter);
    }
    
    ajDebug("     Taxon: %Lu\n", ajListGetLength(thys->Taxon));
    if(ajListGetLength(thys->Taxon))
    {
        i = 0;
        iter = ajListIterNewread(thys->Taxon);
        while(!ajListIterDone(iter))
        {
            resterm = ajListIterGet(iter);
            ajDebug("%15d: %S | %S\n", ++i,  resterm->Id, resterm->Name);
        }
        ajListIterDel(&iter);
    }
    
    ajDebug("   Edamtpc: %Lu\n", ajListGetLength(thys->Edamtpc));
    if(ajListGetLength(thys->Edamtpc))
    {
        i = 0;
        iter = ajListIterNewread(thys->Edamtpc);
        while(!ajListIterDone(iter))
        {
            resterm = ajListIterGet(iter);
            ajDebug("%15d: %S | %S\n", ++i,  resterm->Id, resterm->Name);
        }
        ajListIterDel(&iter);
    }
    
    ajDebug("   Edamdat: %Lu\n", ajListGetLength(thys->Edamdat));
    if(ajListGetLength(thys->Edamdat))
    {
        i = 0;
        iter = ajListIterNewread(thys->Edamdat);
        while(!ajListIterDone(iter))
        {
            resterm = ajListIterGet(iter);
            ajDebug("%15d: %S | %S\n", ++i, resterm->Id, resterm->Name);
        }
        ajListIterDel(&iter);
    }
    
    ajDebug("    Edamid: %Lu\n", ajListGetLength(thys->Edamid));
    if(ajListGetLength(thys->Edamid))
    {
        i = 0;
        iter = ajListIterNewread(thys->Edamid);
        while(!ajListIterDone(iter))
        {
            resterm = ajListIterGet(iter);
            ajDebug("%15d: %S | %S\n", ++i,  resterm->Id, resterm->Name);
        }
        ajListIterDel(&iter);
    }
    
    ajDebug("   Edamfmt: %Lu\n", ajListGetLength(thys->Edamfmt));
    if(ajListGetLength(thys->Edamfmt))
    {
        i = 0;
        iter = ajListIterNewread(thys->Edamfmt);
        while(!ajListIterDone(iter))
        {
            resterm = ajListIterGet(iter);
            ajDebug("%15d: %S | %S\n", ++i,  resterm->Id, resterm->Name);
        }
        ajListIterDel(&iter);
    }
    
    ajDebug("      Xref: %Lu\n", ajListGetLength(thys->Xref));
    if(ajListGetLength(thys->Xref))
    {
        i = 0;
        iter = ajListIterNewread(thys->Xref);
        while(!ajListIterDone(iter))
        {
            reslink = ajListIterGet(iter);
            ajDebug("%15d: %S | %S\n", ++i, reslink->Source, reslink->Term);
        }
        ajListIterDel(&iter);
    }
    
    ajDebug("     Query: %Lu\n", ajListGetLength(thys->Query));
    if(ajListGetLength(thys->Query))
    {
        i = 0;
        iter = ajListIterNewread(thys->Query);
        while(!ajListIterDone(iter))
        {
            resqry = ajListIterGet(iter);
            ajDebug("%15d: %S | %S | %S | %S\n",
                    ++i, resqry->Datatype, resqry->Format,
                    resqry->Term, resqry->Url);
        }
        ajListIterDel(&iter);
    }
    
    ajDebug("  Example: %Lu\n", ajListGetLength(thys->Example));
    if(ajListGetLength(thys->Example))
    {
        i = 0;
        iter = ajListIterNewread(thys->Example);
        while(!ajListIterDone(iter))
        {
            tmpstr = ajListIterGet(iter);
            ajDebug("%15d: %S\n", ++i, tmpstr);
        }
        ajListIterDel(&iter);
    }
    
    return;
}




/* @datasection [AjPReslink] Data resource link data **************************
**
** Function is for manipulating data resource link data objects
**
** @nam2rule Reslink Data resource link data objects
**
******************************************************************************/




/* @section constructors ******************************************************
**
** Constructors
**
** @fdata [AjPReslink]
**
** @nam3rule New Constructor
** @nam4rule Reslink Copy constructor
**
** @argrule NewReslink reslink [const AjPReslink] Source resource link object
**
** @valrule * [AjPReslink] Data resource link data object
**
** @fcategory new
**
******************************************************************************/




/* @func ajReslinkNew *********************************************************
**
** Constructor for a public resource link object
**
** @return [AjPReslink] Resource link object
**
**
** @release 6.4.0
******************************************************************************/

AjPReslink ajReslinkNew(void)
{
    AjPReslink ret;

    AJNEW0(ret);

    return ret;
}




/* @func ajReslinkNewReslink **************************************************
**
** Constructor for a public resource link object
**
** @param [r] reslink [const AjPReslink] Source resource link object
** @return [AjPReslink] Resource link object
**
**
** @release 6.4.0
******************************************************************************/

AjPReslink ajReslinkNewReslink(const AjPReslink reslink)
{
    AjPReslink ret;

    AJNEW0(ret);

    if(!reslink)
	return ret;

    ajStrAssignS(&ret->Source, reslink->Source);
    ajStrAssignS(&ret->Term,   reslink->Term);

    ret->Nterms = reslink->Nterms;

    return ret;
}




/* @section Data resource link data destructors *******************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the data resource link data object.
**
** @fdata [AjPReslink]
**
** @nam3rule Del Destructor
**
** @argrule Del Preslink [AjPReslink*] Data resource link data
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajReslinkDel *********************************************************
**
** Destructor for a public resource link object
**
** @param [d] Preslink [AjPReslink*] Resource link object
** @return [void]
**
** @release 6.4.0
******************************************************************************/

void ajReslinkDel(AjPReslink *Preslink)
{
    AjPReslink thys;

    if(!Preslink) return;
    if(!*Preslink) return;

    thys = *Preslink;

    ajStrDel(&thys->Source);
    ajStrDel(&thys->Term);

    AJFREE(*Preslink);

    return;
}




/* @datasection [AjPList] Resource link list operations *********************
**
** Manipulating lists of resource links
**
** @nam2rule Reslinklist
**
******************************************************************************/




/* @section Resource link list operations *************************************
**
** Manipulating lists of resource links
**
** @fdata [AjPList]
** @fcategory use
**
** @nam3rule Clone Clone list of resource links
**
** @argrule * src [const AjPList] List of resource link objects
** @argrule Clone dest [AjPList] Empty list to hold resource link
**                               objects
**
** @valrule * [AjBool] True on success
**
******************************************************************************/




/* @func ajReslinklistClone ***************************************************
**
** Copy a list of resource links to another list
**
** @param [r] src [const AjPList] Source list of resource links
** @param [w] dest [AjPList] Destination list of resource links
** @return [AjBool] True on success
**
** @release 6.4.0
******************************************************************************/

AjBool ajReslinklistClone(const AjPList src, AjPList dest)
{
    AjIList iter;
    AjPReslink linkout = NULL;
    AjPReslink linkin = NULL;

    if(ajListGetLength(dest))
	return ajFalse;

    iter = ajListIterNewread(src);

    while ((linkin = (AjPReslink) ajListIterGet(iter)))
    {
	linkout = ajReslinkNewReslink(linkin);
	ajListPushAppend(dest, linkout);
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @datasection [AjPResquery] Data resource query data ************************
**
** Function is for manipulating data resource query data objects
**
** @nam2rule Resquery Data resource query data objects
**
******************************************************************************/




/* @section constructors ******************************************************
**
** Constructors
**
** @fdata [AjPResquery]
**
** @nam3rule New Constructor
** @nam4rule Resquery Copy constructor
**
** @argrule NewResquery qry [const AjPResquery] Data resource query data object
**
** @valrule * [AjPResquery] Data resource query data object
**
** @fcategory new
**
******************************************************************************/




/* @func ajResqueryNew ********************************************************
**
** Constructor for a public resource query object
**
** @return [AjPResquery] Resource query object
**
**
** @release 6.4.0
******************************************************************************/

AjPResquery ajResqueryNew(void)
{
    AjPResquery ret;

    AJNEW0(ret);

    return ret;
}




/* @func ajResqueryNewResquery ************************************************
**
** Constructor for a public resource query object
**
** @param [r] qry [const AjPResquery] Source resource query object
** @return [AjPResquery] Resource query object
**
**
** @release 6.4.0
******************************************************************************/

AjPResquery ajResqueryNewResquery(const AjPResquery qry)
{
    AjPResquery ret;

    AJNEW0(ret);

    if(!qry)
	return ret;

    ajStrAssignS(&ret->Datatype,   qry->Datatype);
    ajStrAssignS(&ret->Format,     qry->Format);
    ajStrAssignS(&ret->FormatTerm, qry->FormatTerm);
    ajStrAssignS(&ret->Term,       qry->Term);
    ajStrAssignS(&ret->Url,        qry->Url);

    ret->Nterms = qry->Nterms;

    return ret;
}




/* @section Data resource query data destructors ******************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the data resource query data object.
**
** @fdata [AjPResquery]
**
** @nam3rule Del Destructor
**
** @argrule Del Presquery [AjPResquery*] Data resource query data
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajResqueryDel ********************************************************
**
** Destructor for a public resource query object
**
** @param [d] Presquery [AjPResquery*] Resource query object
** @return [void]
**
** @release 6.4.0
******************************************************************************/

void ajResqueryDel(AjPResquery *Presquery)
{
    AjPResquery thys;

    if(!Presquery) return;
    if(!*Presquery) return;

    thys = *Presquery;

    ajStrDel(&thys->Datatype);
    ajStrDel(&thys->Format);
    ajStrDel(&thys->FormatTerm);
    ajStrDel(&thys->Term);
    ajStrDel(&thys->Url);

    AJFREE(*Presquery);

    return;
}




/* @datasection [AjPList] Resource query list operations *********************
**
** Manipulating lists of resource queries
**
** @nam2rule Resquerylist
**
******************************************************************************/




/* @section Resource query list operations ************************************
**
** Manipulating lists of resource queries
**
** @fdata [AjPList]
** @fcategory use
**
** @nam3rule Clone Clone list of resource queries
**
** @argrule * src [const AjPList] List of resource query objects
** @argrule Clone dest [AjPList] Empty list to hold resource query
**                               objects
**
** @valrule * [AjBool] True on success
**
******************************************************************************/




/* @func ajResquerylistClone **************************************************
**
** Copy a list of resource queries to another list
**
** @param [r] src [const AjPList] Source list of resource queries
** @param [w] dest [AjPList] Destination list of resource queries
** @return [AjBool] True on success
**
** @release 6.4.0
******************************************************************************/

AjBool ajResquerylistClone(const AjPList src, AjPList dest)
{
    AjIList iter;
    AjPResquery qryout = NULL;
    AjPResquery qryin = NULL;

    if(ajListGetLength(dest))
	return ajFalse;

    iter = ajListIterNewread(src);

    while ((qryin = (AjPResquery) ajListIterGet(iter)))
    {
	qryout = ajResqueryNewResquery(qryin);
	ajListPushAppend(dest, qryout);
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @datasection [AjPResterm] Data resource term data **************************
**
** Function is for manipulating data resource term data objects
**
** @nam2rule Resterm Data resource term data objects
**
******************************************************************************/




/* @section constructors ******************************************************
**
** Constructors
**
** @fdata [AjPResterm]
**
** @nam3rule New Constructor
** @nam4rule Resterm Copy constructor
**
** @argrule NewResterm term [const AjPResterm] Data resource term data object
**
** @valrule * [AjPResterm] Data resource term data object
**
** @fcategory new
**
******************************************************************************/




/* @func ajRestermNew *********************************************************
**
** Constructor for a public resource term object
**
** @return [AjPResterm] Resource term object
**
**
** @release 6.4.0
******************************************************************************/

AjPResterm ajRestermNew(void)
{
    AjPResterm ret;

    AJNEW0(ret);

    return ret;
}




/* @func ajRestermNewResterm **************************************************
**
** Constructor for a public resource term object
**
** @param [r] term [const AjPResterm] Source resource term object
** @return [AjPResterm] Resource term object
**
**
** @release 6.4.0
******************************************************************************/

AjPResterm ajRestermNewResterm(const AjPResterm term)
{
    AjPResterm ret;

    AJNEW0(ret);

    if(!term)
	return ret;

    ajStrAssignS(&ret->Id, term->Id);
    ajStrAssignS(&ret->Name, term->Name);

    return ret;
}




/* @section Data resource term data destructors *******************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the data resource term data object.
**
** @fdata [AjPResterm]
**
** @nam3rule Del Destructor
**
** @argrule Del Presterm [AjPResterm*] Data resource term data
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajRestermDel *********************************************************
**
** Destructor for a public resource term object
**
** @param [d] Presterm [AjPResterm*] Resource term object
** @return [void]
**
** @release 6.4.0
******************************************************************************/

void ajRestermDel(AjPResterm *Presterm)
{
    AjPResterm thys;

    if(!Presterm) return;
    if(!*Presterm) return;

    thys = *Presterm;

    ajStrDel(&thys->Id);
    ajStrDel(&thys->Name);

    AJFREE(*Presterm);

    return;
}




/* @datasection [AjPList] Resource term list operations ***********************
**
** Manipulating lists of resource terms
**
** @nam2rule Restermlist
**
******************************************************************************/




/* @section Resource term list operations *************************************
**
** Manipulating lists of resource terms
**
** @fdata [AjPList]
** @fcategory use
**
** @nam3rule Clone Clone list of resource terms
**
** @argrule * src [const AjPList] List of resource term objects
** @argrule Clone dest [AjPList] Empty list to hold resource term
**                               objects
**
** @valrule * [AjBool] True on success
**
******************************************************************************/




/* @func ajRestermlistClone ***************************************************
**
** Copy a list of resource terms to another list
**
** @param [r] src [const AjPList] Source list of resource terms
** @param [w] dest [AjPList] Destination list of resource terms
** @return [AjBool] True on success
**
** @release 6.4.0
******************************************************************************/

AjBool ajRestermlistClone(const AjPList src, AjPList dest)
{
    AjIList iter;
    AjPResterm termout = NULL;
    AjPResterm termin = NULL;

    if(ajListGetLength(dest))
	return ajFalse;

    iter = ajListIterNewread(src);

    while ((termin = (AjPResterm) ajListIterGet(iter)))
    {
	termout = ajRestermNewResterm(termin);
	ajListPushAppend(dest, termout);
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @datasection [none] Miscellaneous functions ********************************
**
** Functions to initialise and clean up internals
**
** @nam2rule Resource Data resource internals
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




/* @func ajResourceExit *******************************************************
**
** Cleans up data processing internal memory
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajResourceExit(void)
{
    ajResourceinExit();
    ajResourceoutExit();

    return;
}




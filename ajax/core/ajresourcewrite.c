/* @source ajresourcewrite ****************************************************
**
** AJAX data resource handling functions
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.19 $
** @modified Oct 25 2010 pmr First AJAX version
** @modified $Date: 2012/12/07 10:18:08 $ by $Author: rice $
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

#include "ajresourcewrite.h"
#include "ajfile.h"


static AjBool resourceoutWriteBasic(AjPFile outf,
                                    const AjPResource resource);
static AjBool resourceoutWriteDrcat(AjPFile outf,
                                    const AjPResource resource);
static AjBool resourceoutWriteList(AjPFile outf,
                                    const AjPResource resource);
static AjBool resourceoutWriteWebpage(AjPFile outf,
                                      const AjPResource resource);
static AjBool resourceoutWriteWsbasic(AjPFile outf,
                                      const AjPResource resource);





/* @datastatic ResourcePOutFormat *********************************************
**
** Data output formats data structure
**
** @alias ResourceSoutFormat
** @alias ResourceOOutFormat
**
** @attr Name [const char*] Format name
** @attr Desc [const char*] Format description
** @attr Write [AjBool function] Output function, returns ajTrue on success
** @@
******************************************************************************/

typedef struct ResourceSOutFormat
{
    const char *Name;
    const char *Desc;
    AjBool (*Write) (AjPFile outf, const AjPResource resource);
} ResourceOOutFormat;

#define ResourcePOutFormat ResourceOOutFormat*


static ResourceOOutFormat resourceoutFormatDef[] =
{
/* "Name",        "Description" */
/*     WriteFunction */

  {"drcat",         "Data resource catalogue entry format",
       &resourceoutWriteDrcat},

  {"basic",         "Basic data resource catalogue entry format",
       &resourceoutWriteBasic},

  {"wsbasic",       "Webservice annotated data resource catalogue entry format",
       &resourceoutWriteWsbasic},

  {"list",          "List of resource ids",
       &resourceoutWriteList},

  {"webpage",       "DRCAT web page",
       &resourceoutWriteWebpage},

  {NULL, NULL, NULL}
};




/* @filesection ajresource ****************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
******************************************************************************/





/* @datasection [AjPOutfile] Data resource data output ************************
**
** Function is for manipulating data resource data objects
**
** @nam2rule Resourceout Data resource data output
**
******************************************************************************/




/* @section data resource data outputs ****************************************
**
** These functions write the data resource data provided by the first argument
**
** @fdata [AjPOutfile]
**
** @nam3rule Write Write data resource data
**
** @argrule * outf [AjPOutfile] Output file
** @argrule * resource [const AjPResource] Data resource data
**
** @valrule * [AjBool] true on success
**
** @fcategory output
**
******************************************************************************/




/* @func ajResourceoutWrite ***************************************************
**
** Write a data resource in a named format
**
** @param [u] outf [AjPOutfile] Output file
** @param [r] resource [const AjPResource] Data resource object
**
** @return [AjBool] True on success
**
**
** @release 6.4.0
******************************************************************************/

AjBool ajResourceoutWrite(AjPOutfile outf, const AjPResource resource)
{
    ajuint i = ajOutfileGetFormatindex(outf);
    AjPFile outfile = ajOutfileGetFile(outf);

    return (*resourceoutFormatDef[i].Write)(outfile, resource);
}




/* @funcstatic resourceoutWriteDrcat ******************************************
**
** Write resource object to file in EMBOSS drcat.dat format.
**
** All elements / fields are written.
**
** @param [u] outf [AjPFile] Output resource file
** @param [r] resource [const AjPResource] Resource object
** @return [AjBool] True on success
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool resourceoutWriteDrcat(AjPFile outf, const AjPResource resource)
{
    AjPReslink tmplink = NULL;
    AjPResquery tmpqry = NULL;
    AjPResterm resterm = NULL;
    AjPStr  tmpstr  = NULL;
    AjIList iter    = NULL;

    if(!outf)
        return ajFalse;

    ajFmtPrintF(outf, "%-8s%S\n", "ID", resource->Id);

    iter = ajListIterNew(resource->Idalt);
    while((tmpstr = ajListstrIterGet(iter)))
        ajFmtPrintF(outf, "%-8s%S\n", "IDalt", tmpstr);
    ajListIterDel(&iter);

    if(ajStrGetLen(resource->Acc))
        ajFmtPrintF(outf, "%-8s%S\n", "Acc", resource->Acc);
    ajFmtPrintF(outf, "%-8s%S\n", "Name", resource->Name);
    ajFmtPrintF(outf, "%-8s%S\n", "Desc", resource->Desc);

    if(ajStrGetLen(resource->Url))
        ajFmtPrintF(outf, "%-8s%S\n", "URL", resource->Url);
    if(ajStrGetLen(resource->Urllink))
        ajFmtPrintF(outf, "%-8s%S\n", "URLlink", resource->Urllink);
    if(ajStrGetLen(resource->Urlrest))
        ajFmtPrintF(outf, "%-8s%S\n", "URLrest", resource->Urlrest);
    if(ajStrGetLen(resource->Urlsoap))
        ajFmtPrintF(outf, "%-8s%S\n", "URLsoap", resource->Urlsoap);

    iter = ajListIterNew(resource->Cat);
    while((tmpstr = ajListstrIterGet(iter)))
        ajFmtPrintF(outf, "%-8s%S\n", "Cat", tmpstr);
    ajListIterDel(&iter);
    
    iter = ajListIterNew(resource->Taxon);
    while((resterm = ajListIterGet(iter)))
        ajFmtPrintF(outf, "%-8s%S | %S\n", "Taxon",
                    resterm->Id, resterm->Name);
    ajListIterDel(&iter);
    
    iter = ajListIterNew(resource->Edamtpc);
    while((resterm = ajListIterGet(iter)))
        ajFmtPrintF(outf, "%-8s%S | %S\n", "EDAMtpc",
                    resterm->Id, resterm->Name);
    ajListIterDel(&iter);
    
    iter = ajListIterNew(resource->Edamdat);
    while((resterm = ajListIterGet(iter)))
        ajFmtPrintF(outf, "%-8s%S | %S\n", "EDAMdat",
                    resterm->Id, resterm->Name);
    ajListIterDel(&iter);

    iter = ajListIterNew(resource->Edamid);
    while((resterm = ajListIterGet(iter)))
        ajFmtPrintF(outf, "%-8s%S | %S\n", "EDAMid",
                    resterm->Id, resterm->Name);
    ajListIterDel(&iter);
    
    iter = ajListIterNew(resource->Edamfmt);
    while((resterm = ajListIterGet(iter)))
        ajFmtPrintF(outf, "%-8s%S | %S\n", "EDAMfmt",
                    resterm->Id, resterm->Name);
    ajListIterDel(&iter);

    iter = ajListIterNew(resource->Xref);
    while((tmplink = ajListIterGet(iter)))
        ajFmtPrintF(outf, "%-8s%S | %S\n",
                    "Xref", tmplink->Source,
                    tmplink->Term);
    ajListIterDel(&iter);
    
    iter = ajListIterNew(resource->Query);
    while((tmpqry = ajListIterGet(iter)))
        ajFmtPrintF(outf, "%-8s %S | %S | %S | %S\n",
                    "Query", tmpqry->Datatype, tmpqry->Format,
                    tmpqry->Term, tmpqry->Url);
    ajListIterDel(&iter);

    iter = ajListIterNew(resource->Example);
    while((tmpstr = ajListstrIterGet(iter)))
        ajFmtPrintF(outf, "%-8s%S\n", "Example", tmpstr);
    ajListIterDel(&iter);
    
    ajFmtPrintF(outf, "\n");

    return ajTrue;
}




/* @funcstatic resourceoutWriteBasic ******************************************
**
** Write resource object to file in EMBOSS drcat.dat format.
**
** Only the most important elements / fields are written:
** Recommended or official unique identifier ('ID')
** The full (verbose english) name ('Name')
** URL of the database server ('URL')
**
** @param [u] outf [AjPFile] Output resource file
** @param [r] resource [const AjPResource] Resource object
** @return [AjBool] True on success
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool resourceoutWriteBasic(AjPFile outf, const AjPResource resource)
{
    if(!outf)
        return ajFalse;

    ajFmtPrintF(outf, "%-8s%S\n", "ID", resource->Id);
    ajFmtPrintF(outf, "%-8s%S\n", "Name", resource->Name);
    if(ajStrGetLen(resource->Url))
        ajFmtPrintF(outf, "%-8s%S\n", "URL", resource->Url);

    ajFmtPrintF(outf, "\n");

    return ajTrue;
}




/* @funcstatic resourceoutWriteList *******************************************
**
** Write resource object id to file
**
** @param [u] outf [AjPFile] Output resource file
** @param [r] resource [const AjPResource] Resource object
** @return [AjBool] True on success
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool resourceoutWriteList(AjPFile outf, const AjPResource resource)
{
    if(!outf)
        return ajFalse;
    if(!resource)
        return ajFalse;

    if(ajStrGetLen(resource->Db))
        ajFmtPrintF(outf, "%S:%S\n", resource->Db, resource->Id);
    else
        ajFmtPrintF(outf, "%S\n", resource->Id);

    return ajTrue;
}




/* @funcstatic resourceoutWriteWebpage ****************************************
**
** Write resource object to file in DRCAT web pageformat.
**
** All elements / fields are written.
**
** @param [u] outf [AjPFile] Output resource file
** @param [r] resource [const AjPResource] Resource object
** @return [AjBool] True on success
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool resourceoutWriteWebpage(AjPFile outf, const AjPResource resource)
{
    AjPResquery tmpqry = NULL;
    AjPResterm resterm = NULL;
    AjPStr  snstr   = NULL;
    AjPStr  tmpstr   = NULL;
    AjPStr  tmpurl   = NULL;
    AjPStr  tmpid    = NULL;
    AjPStr  tmpname  = NULL;
    AjPStr  tmpqid   = NULL;
    AjPStr  tmpqname = NULL;
    AjPStr  tmpqterm = NULL;
    AjIList iter     = NULL;
    AjIList iterq    = NULL;
    ajuint i = 0;
    AjPStrTok handle = NULL;
    AjPStrTok namehandle = NULL;
    AjPStrTok termhandle = NULL;
    const char* biourl = "http://bioportal.bioontology.org/ontologies/45846";

    if(!outf)
        return ajFalse;

    ajFmtPrintF(outf,
                "<!DOCTYPE HTML PUBLIC "
                "\"-//W3C//DTD HTML 4.01 Transitional//EN\">\n");
    ajFmtPrintF(outf,
                "<html>\n");

    ajFmtPrintF(outf,
                "<head>\n");
    ajFmtPrintF(outf,
                "<meta http-equiv=\"Content-Type\" "
                "content=\"text/html;charset=UTF-8\" />\n");
    ajFmtPrintF(outf,
                "<title>Bioinformatics Data Resource Catalogue</title>\n");
    ajFmtPrintF(outf,
                "</head>\n\n\n");

    ajFmtPrintF(outf,
                "<body bgcolor=\"#ffffff\">\n");

    ajFmtPrintF(outf,
                "<!-- ID -->\n");

    if(ajStrGetLen(resource->Url))
       ajFmtPrintF(outf,
                   "<center><h1><a href=\"%S\">%S</a></h1></center>\n",
                   resource->Url, resource->Id);
    else
       ajFmtPrintF(outf,
                   "<center><h1>%S</h1></center>\n",
                   resource->Id);

    ajFmtPrintF(outf,
                "<!-- Name -->\n");

    ajFmtPrintF(outf,
                "<center><h2>%S</h2> </center>\n",
                resource->Name);

    ajFmtPrintF(outf,
                "<!-- Desc -->\n");
    ajFmtPrintF(outf,
                "<p><i>%S</i></p>\n",
                resource->Desc);

    ajFmtPrintF(outf,
                "<table>\n");

    ajFmtPrintF(outf,
                "<!-- ID -->\n");

    ajFmtPrintF(outf,
                "  <tr align=\"left\"><th>ID</th><td>%S</td></tr>\n",
                resource->Id);

    ajFmtPrintF(outf,
                "<!-- IDalt or omit -->\n");

    if(ajListGetLength(resource->Idalt))
    {
        ajFmtPrintF(outf,
                    "  <tr align=\"left\"><th>IDalt</th><td>");

        i = 0;
        iter = ajListIterNew(resource->Idalt);
        while((tmpstr = ajListstrIterGet(iter)))
        {
            if(i++)
                ajFmtPrintF(outf, ", ");
            ajFmtPrintF(outf, "%S", tmpstr);
        }

        ajListIterDel(&iter);

        ajFmtPrintF(outf,
                    "</td></tr>\n");
    }
 
    if(ajStrGetLen(resource->Acc))
    {
        ajFmtPrintF(outf,
                    "<!-- ACC or omit -->\n");
        
        ajFmtPrintF(outf,
                    "  <tr align=\"left\"><th>Acc</th><td>%S</td></tr>\n",
                    resource->Acc);
    }


    if(ajStrGetLen(resource->Url))
        ajFmtPrintF(outf,
                    "  <tr align=\"left\"><th>Home</th><td>"
                    "<a href=\"%S\">%S</a></td></tr>\n",
                    resource->Url, resource->Url);

    if(ajListGetLength(resource->Edamtpc))
    {
        ajFmtPrintF(outf,
                "<!-- EDAMtpc -->\n");

        iter = ajListIterNew(resource->Edamtpc);

        while((resterm = ajListIterGet(iter)))
        {
            ajStrAssignS(&tmpid, resterm->Id);
            ajStrCutBraces(&tmpid);
            ajStrFmtPercentEncodeC(&tmpid, " /()");
            ajFmtPrintF(outf,
                        "  <tr align=\"left\"><th>EDAM topic</th><td>"
                        "<a href=\"%s?p=terms&conceptid=EDAM%%3A%S\">%S</a>"
                        "</td></tr>\n",
                        biourl, tmpid, resterm->Name);
        }
        ajListIterDel(&iter);
    }

    ajFmtPrintF(outf,
                "<!-- Cat or omit -->\n");

    if(ajListGetLength(resource->Cat))
    {
        iter = ajListIterNew(resource->Cat);
        while((tmpstr = ajListstrIterGet(iter)))
            ajFmtPrintF(outf,
                        "  <tr align=\"left\"><th>Category</th>"
                        "<td>%S</td></tr>\n",
                        tmpstr);
        ajListIterDel(&iter);
    }
    
    ajFmtPrintF(outf,
                "<!-- Taxon (comma-separated list) -->\n");

    ajFmtPrintF(outf,
                "  <tr align=\"left\"><th>Taxon</th><td>");
    i = 0;
    iter = ajListIterNew(resource->Taxon);
    while((resterm = ajListIterGet(iter)))
    {
        if(i++)
            ajFmtPrintF(outf, ", ");
        ajFmtPrintF(outf,
                    "<a href=\"http://www.ncbi.nlm.nih.gov/Taxonomy/Browser/"
                    "wwwtax.cgi?id=%S\">%S</a>",
                    resterm->Id, resterm->Name);
    }

    ajListIterDel(&iter);
    ajFmtPrintF(outf,
                "</td></tr>\n");
    

    ajFmtPrintF(outf,
                "<!-- URLlink or omit -->\n");

    if(ajStrGetLen(resource->Urllink))
         ajFmtPrintF(outf,
                     "<tr align=\"left\"><th>Linking</th><td>"
                     "<a href=\"%S\">%S</a></td></tr>\n",
                     resource->Urllink, resource->Urllink);

    ajFmtPrintF(outf,
                "<!-- URLrest or omit -->\n");

    if(ajStrGetLen(resource->Urlrest))
         ajFmtPrintF(outf,
                     "<tr align=\"left\"><th>REST</th><td>"
                     "<a href=\"%S\">%S</a></td></tr>\n",
                     resource->Urlrest, resource->Urlrest);

    ajFmtPrintF(outf,
                "<!-- URLsoap or omit -->\n");

    if(ajStrGetLen(resource->Urlsoap))
         ajFmtPrintF(outf,
                     "<tr align=\"left\"><th>SOAP</th><td>"
                     "<a href=\"%S\">%S</a></td></tr>\n",
                     resource->Urlsoap, resource->Urlsoap);

    ajFmtPrintF(outf,
                "<!-- Xref - omit ! -->\n");

    ajFmtPrintF(outf,
                "</table>\n");


    ajFmtPrintF(outf,
                "<!-- Query or omit, free-text comments given in {} --> \n");
    if(ajListGetLength(resource->Query))
    {
        ajFmtPrintF(outf,
                    "<h3>Available data</h3>\n");
        ajFmtPrintF(outf,
                    "<table>\n");
        ajFmtPrintF(outf,
                    "   <thead>\n");
        ajFmtPrintF(outf,
                    "      <tr align=\"left\">\n");
        ajFmtPrintF(outf,
                    "         <th>Data</th>\n");
        ajFmtPrintF(outf,
                    "         <th>Format</th>\n");
        ajFmtPrintF(outf,
                    "         <th>Query</th>\n");
        ajFmtPrintF(outf,
                    "         <th>Link</th>\n");
        ajFmtPrintF(outf,
                    "      </tr>\n");
        ajFmtPrintF(outf,
                    "   </thead>\n");
        ajFmtPrintF(outf,
                    "   <tbody>\n");

        iter = ajListIterNew(resource->Query);
        while((tmpqry = ajListIterGet(iter)))
        {
            ajStrAssignS(&tmpname, tmpqry->Datatype);
            ajStrCutBraces(&tmpname);
            ajStrFmtPercentEncodeC(&tmpname, " /()");
            ajFmtPrintF(outf,
                        "     <tr>\n");
            ajFmtPrintF(outf,
                        "       <td>"
                        "<a href=\"%s?"
                        "p=terms&conceptid=EDAM%%3A%S\">%S</a></td>\n",
                        biourl, tmpname, tmpqry->Datatype);

            ajStrAssignS(&tmpname, tmpqry->Format);
            ajStrCutBraces(&tmpname);
            ajStrFmtPercentEncodeC(&tmpname, " /()");
            ajFmtPrintF(outf,
                        "       <td>"
                        "<a href=\"%s?"
                        "p=terms&conceptid=EDAM%%3A%S\">%S</a></td>\n",
                        biourl, tmpname, tmpqry->Format);

            ajStrAssignS(&tmpname, tmpqry->Term);
            ajStrCutBraces(&tmpname);
            ajStrFmtPercentEncodeC(&tmpname, " /()");
            ajFmtPrintF(outf,
                        "       <td>"
                        "<a href=\"%s?"
                        "p=terms&conceptid=EDAM%%3A%S\">%S</a></td>\n",
                        biourl, tmpname, tmpqry->Term);
            ajFmtPrintF(outf,
                        "       <td>%S</td>\n",
                        tmpqry->Url);
            ajFmtPrintF(outf,
                        "     </tr>\n\n");
        }
        ajListIterDel(&iter);

        ajFmtPrintF(outf,
                    "   </tbody>\n");
        ajFmtPrintF(outf,
                    "</table>\n");
    }

    ajFmtPrintF(outf,
                "<!-- Example or omit -->\n");

    if(ajListGetLength(resource->Example))
    {
        ajFmtPrintF(outf,
                    "<h3>Example queries</h3>\n");

        ajFmtPrintF(outf,
                    "<table>\n");
        ajFmtPrintF(outf,
                    "   <thead>\n");
        ajFmtPrintF(outf,
                    "      <tr align=\"left\">\n");
        ajFmtPrintF(outf,
                    "         <th>Data</th>\n");
        ajFmtPrintF(outf,
                    "         <th>Format</th>\n");
        ajFmtPrintF(outf,
                    "         <th>Query</th>\n");
        ajFmtPrintF(outf,
                    "         <th>Example</th>\n");
        ajFmtPrintF(outf,
                    "      </tr>\n");
        ajFmtPrintF(outf,
                    "   </thead>\n");
        ajFmtPrintF(outf,
                    "   <tbody>\n");

        iter = ajListIterNew(resource->Example);
        while((tmpstr = ajListstrIterGet(iter)))
        {
            ajStrTokenAssignC(&handle, tmpstr, "|");
            ajStrTokenNextParse(handle, &tmpid);
            ajStrRemoveWhiteExcess(&tmpid);
            ajStrTokenNextParse(handle, &tmpname);
            ajStrRemoveWhiteExcess(&tmpname);
            
            iterq = ajListIterNew(resource->Query);
            while((tmpqry = ajListIterGet(iterq)))
            {
                if(!ajStrMatchS(tmpid, tmpqry->Term))
                   continue;

                ajStrAssignS(&tmpqid, tmpqry->Datatype);
                ajStrCutBraces(&tmpqid);
                ajStrFmtPercentEncodeC(&tmpqid, " /()");

                ajFmtPrintF(outf,
                            "     <tr>\n");
                ajFmtPrintF(outf,
                            "       <td>"
                            "<a href=\"%s?"
                            "p=terms&conceptid=EDAM%%3A%S\">%S</a></td>\n",
                            biourl, tmpqid, tmpqry->Datatype); /* datatype */

                ajStrAssignS(&tmpqid, tmpqry->Format);
                ajStrCutBraces(&tmpqid);
                ajStrFmtPercentEncodeC(&tmpqid, " /()");

                ajFmtPrintF(outf,
                            "       <td>"
                            "<a href=\"%s?"
                            "p=terms&conceptid=EDAM%%3A%S\">%S</a></td>\n",
                            biourl, tmpqid, tmpqry->Format); /* format */

                ajStrAssignS(&tmpurl, tmpqry->Url);
                ajStrAssignS(&tmpqid, tmpqry->Term);
                ajStrCutBraces(&tmpqid);
                ajStrFmtPercentEncodeC(&tmpqid, " /()");

                i = 0;
                ajStrTokenAssignC(&handle, tmpid, ";");
                ajStrTokenAssignC(&namehandle, tmpname, ";");
                ajStrTokenAssignC(&termhandle, tmpqry->Term, ";");
                ajFmtPrintF(outf,
                            "       <td>");

                while(ajStrTokenNextParse(handle, &tmpqid))
                {
                    ajStrTokenNextParse(namehandle, &tmpqname);
                    ajStrTokenNextParse(termhandle, &tmpqterm);

                    if(i)
                        ajFmtPrintF(outf,
                                    "; ");
                                    
                    ajStrRemoveWhiteExcess(&tmpqid);
                    ajStrCutBraces(&tmpqid);

                    ajFmtPrintF(outf,
                                "<a href=\"%s?"
                                "p=terms&conceptid=EDAM%%3A%S\">%S</a>",
                                biourl, tmpqid, tmpqterm); /* id term */

                    ajFmtPrintS(&snstr, "%%s%u", (i+1));
                    ajStrExchangeSS(&tmpurl, snstr, tmpqname);
                        
                    i++;

                }
                ajFmtPrintF(outf,
                                "</td>\n");

                ajFmtPrintS(&snstr, "%%s");
                ajStrExchangeSS(&tmpurl, snstr, tmpname);

                ajFmtPrintF(outf,
                            "       <td><a href=\"%S\">%S</a></td>\n",
                            tmpurl, tmpname); /* url, exampleid */

                ajFmtPrintF(outf,
                                "     </tr>\n");
            }
            ajListIterDel(&iterq);
        }

        ajListIterDel(&iter);

        ajFmtPrintF(outf,
                    "   </tbody>\n");
        ajFmtPrintF(outf,
                    "</table>\n");
    }

    ajFmtPrintF(outf, "\n\n</body></html>\n");

    ajStrDel(&tmpid);
    ajStrDel(&tmpname);
    ajStrDel(&tmpurl);
    ajStrDel(&tmpqid);
    ajStrDel(&tmpqname);
    ajStrDel(&tmpqterm);
    ajStrDel(&snstr);

    ajStrTokenDel(&handle);
    ajStrTokenDel(&namehandle);
    ajStrTokenDel(&termhandle);

    return ajTrue;
}




/* @funcstatic resourceoutWriteWsbasic ****************************************
**
** Write resource object to file in EMBOSS db.dat format.
**
** An extension of ajResourceoutWriteBasic, the following fields / elements
** are written:
**
** Recommended or official unique identifier ('ID')
** The full (verbose english) name ('Name')
** URL of the database server ('URL')
**
** and also:
** URL of documentation on SOAP-based interfaces ('URLsoap')
** URL of documentation on REST-based interfaces ('URLrest')
** 
** @param [u] outf [AjPFile] Output resource file
** @param [r] resource [const AjPResource] Resource object
** @return [AjBool] True on success
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool resourceoutWriteWsbasic(AjPFile outf,
                                      const AjPResource resource)
{
    if(!outf)
        return ajFalse;

    (void) resource;
    (void) outf;

    return ajTrue;
}




/* @datasection [none] Miscellaneous functions ********************************
**
** Functions to initialise and clean up internals
**
** @nam2rule Resourceout Data output internals
**
******************************************************************************/




/* @section Printing **********************************************************
**
** Printing details of the internals to a file
**
** @fdata [none]
**
** @nam2rule Resourceoutprint
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




/* @func ajResourceoutprintBook ***********************************************
**
** Reports the resource format internals as Docbook text
**
** @param [u] outf [AjPFile] Output file
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajResourceoutprintBook(AjPFile outf)
{
    ajint i = 0;

    ajFmtPrintF(outf, "<para>The supported data resource output "
                "formats are summarised "
                "in the table below. The columns are as follows: "
                "<emphasis>Output format</emphasis> (format name), "
                 "<emphasis>Description</emphasis> (short description of "
                "the format).</para> \n\n");

    ajFmtPrintF(outf, "<table frame=\"box\" rules=\"cols\">\n");
    ajFmtPrintF(outf, "  <caption>Data resource output formats</caption>\n");
    ajFmtPrintF(outf, "  <thead>\n");
    ajFmtPrintF(outf, "    <tr align=\"center\">\n");
    ajFmtPrintF(outf, "      <th>Output Format</th>\n");
    ajFmtPrintF(outf, "      <th>Description</th>\n");
    ajFmtPrintF(outf, "    </tr>\n");
    ajFmtPrintF(outf, "  </thead>\n");
    ajFmtPrintF(outf, "  <tbody>\n");

    for(i=0; resourceoutFormatDef[i].Name; i++)
    {
        ajFmtPrintF(outf, "    <tr>\n");
        ajFmtPrintF(outf, "      <td>%s</td>\n",
                    resourceoutFormatDef[i].Name);
        ajFmtPrintF(outf, "      <td>%s</td>\n",
                    resourceoutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "  </tbody>\n");
    ajFmtPrintF(outf, "</table>\n");

    return;
}




/* @func ajResourceoutprintHtml ***********************************************
**
** Reports the internal data structures
**
** @param [u] outf [AjPFile] Output file
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajResourceoutprintHtml(AjPFile outf)
{
    ajint i = 0;

    ajFmtPrintF(outf, "<table border=3>");
    ajFmtPrintF(outf, "<tr><th>Data resource Format</th>\n");
    ajFmtPrintF(outf, "<th>Description</th></tr>\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# Data resource output formats\n");
    ajFmtPrintF(outf, "# Name    Format name (or alias)\n");
    ajFmtPrintF(outf, "# Desc    Format description\n");
    ajFmtPrintF(outf, "# Name         Description\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "ResourceFormat {\n");

    for(i=0; resourceoutFormatDef[i].Name; i++)
    {
        ajFmtPrintF(outf, "<tr><td>\n%-12s\n"
                        "<td>\"%s\"</td></tr>\n",
			resourceoutFormatDef[i].Name,
			resourceoutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajResourceoutprintText ***********************************************
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

void ajResourceoutprintText(AjPFile outf, AjBool full)
{
    ajint i = 0;

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# Data resource output formats\n");
    ajFmtPrintF(outf, "# Name    Format name (or alias)\n");
    ajFmtPrintF(outf, "# Desc    Format description\n");
    ajFmtPrintF(outf, "# Name         Description\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "ResourceFormat {\n");

    for(i=0; resourceoutFormatDef[i].Name; i++)
    {
	if(full)
	    ajFmtPrintF(outf, "  %-12s \"%s\"\n",
			resourceoutFormatDef[i].Name,
			resourceoutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajResourceoutprintWiki ***********************************************
**
** Reports the data resource output format internals as wikitext
**
** @param [u] outf [AjPFile] Output file
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajResourceoutprintWiki(AjPFile outf)
{
    ajint i = 0;
    ajint j = 0;
    AjPStr namestr = NULL;

    ajFmtPrintF(outf, "{| class=\"wikitable sortable\" border=\"2\"\n");
    ajFmtPrintF(outf, "|-\n");
    ajFmtPrintF(outf, "!Format!!"
                "class=\"unsortable\"|Description\n");

    for(i=0; resourceoutFormatDef[i].Name; i++)
    {
        ajFmtPrintF(outf, "|-\n");
        ajStrAssignC(&namestr, resourceoutFormatDef[i].Name);

        for(j=i+1; resourceoutFormatDef[j].Name; j++)
        {
            if(resourceoutFormatDef[j].Write == resourceoutFormatDef[i].Write)
            {
                ajFmtPrintAppS(&namestr, "<br>%s",
                               resourceoutFormatDef[j].Name);
                ajWarn("Data resource output format '%s' same as '%s' "
                       "but not alias",
                       resourceoutFormatDef[j].Name,
                       resourceoutFormatDef[i].Name);
            }
        }
        ajFmtPrintF(outf, "|%S||%s\n",
		    namestr,
		    resourceoutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "|}\n");

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




/* @func ajResourceoutExit ****************************************************
**
** Cleans up data output internal memory
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajResourceoutExit(void)
{
    return;
}




/* @datasection [none] Output formats *****************************************
**
** Output formats internals
**
** @nam2rule Resourceoutformat Data resource output format specific
**
******************************************************************************/




/* @section cast **************************************************************
**
** Values for output formats
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




/* @func ajResourceoutformatFind **********************************************
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

AjBool ajResourceoutformatFind(const AjPStr format, ajint* iformat)
{
    AjPStr tmpformat = NULL;
    ajuint i = 0;

    /* ajDebug("ajResourceoutformatFind '%S'\n", format); */
    if(!ajStrGetLen(format))
	return ajFalse;

    ajStrAssignS(&tmpformat, format);
    ajStrFmtLower(&tmpformat);

    for(i=0; resourceoutFormatDef[i].Name; i++)
    {
	/* ajDebug("test %d '%s' \n", i, resourceoutFormatDef[i].Name); */
	if(ajStrMatchCaseC(tmpformat, resourceoutFormatDef[i].Name))
	{
	    *iformat = i;
	    ajStrDel(&tmpformat);
	    /* ajDebug("found '%s' at %d\n", resourceoutFormatDef[i].Name, i); */
	    return ajTrue;
	}
    }

    ajErr("Unknown output format '%S'", format);

    ajStrDel(&tmpformat);

    *iformat = 0;

    return ajFalse;
}




/* @func ajResourceoutformatTest **********************************************
**
** Tests whether a named data resource output format is known
**
** @param [r] format [const AjPStr] Format
** @return [AjBool] ajTrue if formats was accepted
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajResourceoutformatTest(const AjPStr format)
{
    ajint i;

    for (i=0; resourceoutFormatDef[i].Name; i++)
	if(ajStrMatchCaseC(format, resourceoutFormatDef[i].Name))
	    return ajTrue;

    return ajFalse;
}

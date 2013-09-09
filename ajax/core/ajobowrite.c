/* @source ajobowrite *********************************************************
**
** AJAX OBO writing functions
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.15 $
** @modified May 5 pmr 2010 First AJAX version
** @modified Sep 8 2010 pmr Added output formats
** @modified $Date: 2012/08/21 10:54:04 $ by $Author: rice $
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

#include "ajobowrite.h"
#include "ajobo.h"
#include "ajfileio.h"


static AjBool oboWriteExcel(AjPFile outf, const AjPObo obo);
static AjBool oboWriteHtml(AjPFile outf, const AjPObo obo);
static AjBool oboWriteJson(AjPFile outf, const AjPObo obo);
static AjBool oboWriteList(AjPFile outf, const AjPObo obo);
static AjBool oboWriteObo(AjPFile outf, const AjPObo obo);
static AjBool oboWriteObobrief(AjPFile outf, const AjPObo obo);
static AjBool oboWriteXml(AjPFile outf, const AjPObo obo);





/* @datastatic OboPOutFormat **************************************************
**
** Obo output formats data structure
**
** @alias OboSoutFormat
** @alias OboOOutFormat
**
** @attr Name [const char*] Format name
** @attr Obo  [const char*] Ontology term id from EDAM
** @attr Desc [const char*] Format description
** @attr Write [AjBool function] Output function, returns ajTrue on success
** @@
******************************************************************************/

typedef struct OboSOutFormat
{
    const char *Name;
    const char *Obo;
    const char *Desc;
    AjBool (*Write) (AjPFile outf, const AjPObo obo);
} OboOOutFormat;

#define OboPOutFormat OboOOutFormat*


static OboOOutFormat obooutFormatDef[] =
{
/*   "Name",        OBO     "Description" */
/*     WriteFunction */
    {"obo",         "2196", "OBO format",
       &oboWriteObo},
    {"brief",       "0000", "OBO format id, name and namespace only",
       &oboWriteObobrief},
    {"list",        "0000", "Identifier only",
       &oboWriteList},
    {"html",        "2331", "HTML with markup",
       &oboWriteHtml},
    {"xml",         "2332", "XML format",
     &oboWriteXml},
    {"json",        "0000", "JSON format",
       &oboWriteJson},
    {"excel",       "0000", "Tab-delimited file for import to Microsoft Excel",
       &oboWriteExcel},
    {NULL, NULL, NULL, NULL}
};




/* @filesection ajobo ********************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
******************************************************************************/





/* @datasection [AjPFile] Obo Term output *************************************
**
** Function is for manipulating obo term objects
**
** @nam2rule Oboout Obo term output
**
******************************************************************************/




/* @section obo term outputs **********************************************
**
** These functions write the obo term provided by the first argument
**
** @fdata [AjPFile]
**
** @nam3rule Write Write an obo term
** @nam4rule Format Use a nmaed format
** @nam4rule Html Write as HTML with markup
** @nam4rule Obo Write as standard OBO format
**
** @argrule Write outf [AjPOutfile] Output file
** @argrule Write obo [const AjPObo] Obo term
** @argrule Format fmt [const AjPStr] Format name
**
** @valrule * [AjBool] true on success
**
** @fcategory output
**
******************************************************************************/




/* @func ajObooutWrite ********************************************************
**
** Write an OBO term in a named format
**
** @param [u] outf [AjPOutfile] Output file
** @param [r] obo [const AjPObo] Obo term object
**
** @return [AjBool] True on success
**
**
** @release 6.4.0
******************************************************************************/

AjBool ajObooutWrite(AjPOutfile outf, const AjPObo obo)
{
    ajuint i = ajOutfileGetFormatindex(outf);
    AjPFile outfile = ajOutfileGetFile(outf);

    return (*obooutFormatDef[i].Write)(outfile, obo);
}





/* @funcstatic oboWriteExcel **************************************************
**
** Write an OBO term in an tab-delimited format
**
** @param [u] outf [AjPFile] Output file
** @param [r] obo [const AjPObo] Obo term object
**
** @return [AjBool] True on success
**
**
** @release 6.4.0
******************************************************************************/

static AjBool oboWriteExcel(AjPFile outf, const AjPObo obo)
{
    if(!outf) return ajFalse;
    if(!obo) return ajFalse;

    /*ajFmtPrintF(outf, "id\tnamespace\tname\n");*/
    if(ajStrGetLen(obo->Trueid))
        ajFmtPrintF(outf, "%S\t", obo->Trueid);
    else
        ajFmtPrintF(outf, "%S\t", obo->Id);
    ajFmtPrintF(outf, "%S\t%S\n", obo->Namespace, obo->Name);

    return ajTrue;
}




/* @funcstatic oboWriteHtml ***************************************************
**
** Write an OBO term in an HTML format
**
** @param [u] outf [AjPFile] Output file
** @param [r] obo [const AjPObo] Obo term object
**
** @return [AjBool] True on success
**
**
** @release 6.4.0
******************************************************************************/

static AjBool oboWriteHtml(AjPFile outf, const AjPObo obo)
{
    AjIList iter = NULL;
    AjPObotag obotag;

    if(!outf) return ajFalse;
    if(!obo) return ajFalse;

    ajFmtPrintF(outf, "[Term]\n");

    if(ajStrGetLen(obo->Trueid))
        ajFmtPrintF(outf, "id: %S\n", obo->Trueid);
    else
        ajFmtPrintF(outf, "id: %S\n", obo->Id);

    if(ajStrGetLen(obo->Name))
        ajFmtPrintF(outf, "name: %S\n", obo->Name);
    if(ajStrGetLen(obo->Namespace))
        ajFmtPrintF(outf, "namespace: %S\n", obo->Namespace);

    if(ajStrGetLen(obo->Trueid))
        ajFmtPrintF(outf, "alt_id: %S\n", obo->Id);

    if(ajStrGetLen(obo->Def))
        ajFmtPrintF(outf, "def: %S\n", obo->Def);

    if(ajStrGetLen(obo->Comment))
        ajFmtPrintF(outf, "comment: %S\n", obo->Comment);

    iter = ajListIterNewread(obo->Taglist);
    while(!ajListIterDone(iter))
    {
        obotag = (AjPObotag) ajListIterGet(iter);

     /* pick out ONTO:id for markup - based on each ontology */
     /* EDAM: http://srs.ebi.ac.uk/srsbin/cgi-bin/wgetz?[edam-ID:0000346]+-e */
        
        ajFmtPrintF(outf, "%S: %S", obotag->Name, obotag->Value);
        if(ajStrGetLen(obotag->Modifier))
            ajFmtPrintF(outf, " %S",
                        obotag->Modifier);
        if(ajStrGetLen(obotag->Comment))
            ajFmtPrintF(outf, " ! %S",
                        obotag->Comment);
        ajFmtPrintF(outf, "\n");
    }
    ajListIterDel(&iter);
    
    return ajTrue;
}




/* @funcstatic oboWriteJson ***************************************************
**
** Write an OBO term in JSON format
**
** @param [u] outf [AjPFile] Output file
** @param [r] obo [const AjPObo] Obo term object
**
** @return [AjBool] True on success
**
**
** @release 6.4.0
******************************************************************************/

static AjBool oboWriteJson(AjPFile outf, const AjPObo obo)
{
    AjIList iter = NULL;
    AjPObotag obotag;
    AjPStr defstr = NULL;
    AjPStr xrefacc = NULL;
    AjPStr xrefdb = NULL;
    AjPStr synscope = NULL;
    AjPStr synstr = NULL;
    AjPStr reltype = NULL;
    AjPStr relto = NULL;

    if(!outf) return ajFalse;
    if(!obo) return ajFalse;

    ajFmtPrintF(outf, "{\n");

    if(ajStrGetLen(obo->Trueid))
        ajFmtPrintF(outf, "  \"id\": \"%S\",\n", obo->Trueid);
    else
        ajFmtPrintF(outf, "  \"id\": \"%S\",\n", obo->Id);

    if(ajStrGetLen(obo->Name))
        ajFmtPrintF(outf, "  \"name\": \"%S\"\n", obo->Name);
    if(ajStrGetLen(obo->Namespace))
        ajFmtPrintF(outf, "  \"namespace\": \"%S\",\n", obo->Namespace);

    /*
    if(ajStrGetLen(obo->Trueid))
        ajFmtPrintF(outf, "alt_id: %S\n", obo->Id);
    */
    if(ajStrGetLen(obo->Def))
    {
        ajFmtPrintF(outf, "  \"def\":\n  {\n");
        ajFmtPrintF(outf, "    \"defstr\": \"%S\",\n", defstr);
        ajFmtPrintF(outf, "    \"dbxref\":\n");
        ajFmtPrintF(outf, "    [\n");
        ajFmtPrintF(outf, "      {\n");
        ajFmtPrintF(outf, "        \"acc\": \"%S\",\n", xrefacc);
        ajFmtPrintF(outf, "        \"dbname\": \"%S\",\n", xrefdb);
        ajFmtPrintF(outf, "      }\n"); /* comma if there are more */
        ajFmtPrintF(outf, "    ]\n");
        ajFmtPrintF(outf, "  },\n");
    }
    
    if(ajStrGetLen(obo->Comment))
        ajFmtPrintF(outf, "  \"comment\": \"%S\",\n", obo->Comment);

    iter = ajListIterNewread(obo->Taglist);
    while(!ajListIterDone(iter))
    {
        obotag = (AjPObotag) ajListIterGet(iter);
        if(ajStrMatchC(obotag->Name, "is_obsolete") ||
           ajStrMatchC(obotag->Name, "built_in") ||
           ajStrMatchC(obotag->Name, "is_anonymous")) 
        {
            if(ajStrMatchC(obotag->Name, "true"))
               ajFmtPrintF(outf, "  \"%S\": 1,\n",
                            obotag->Name);
        }
        else if (ajStrMatchC(obotag->Name, "consider") ||
                 ajStrMatchC(obotag->Name, "is_a") ||
                 ajStrMatchC(obotag->Name, "intersection_of") ||
                 ajStrMatchC(obotag->Name, "union_of") ||
                 ajStrMatchC(obotag->Name, "disjoint_from") ||
                 ajStrMatchC(obotag->Name, "subset") ||
                 ajStrMatchC(obotag->Name, "replaced_by") ||
                 ajStrMatchC(obotag->Name, "alt_id")) 
        {
            ajFmtPrintF(outf, "  \"%S\": \"%S\",\n",
                        obotag->Name, obotag->Value);
        }
        else if(ajStrMatchC(obotag->Name, "synonym")) 
        {
            ajFmtPrintF(outf, "  \"synonym\":\n");
            ajFmtPrintF(outf, "  {\n");
            ajFmtPrintF(outf, "    \"scope\": \"%S\",\n", synscope);
            ajFmtPrintF(outf, "    \"synonym_text\": \"%S\",\n", synstr);
            ajFmtPrintF(outf, "  },\n>");
        }
        
        else if(ajStrMatchC(obotag->Name, "relationship")) 
        {
            ajFmtPrintF(outf, "  \"relationship\":\n");
            ajFmtPrintF(outf, "  [\n");
            ajFmtPrintF(outf, "    {\n");
            ajFmtPrintF(outf, "      \"type\": \"%S\",\n", reltype);
            ajFmtPrintF(outf, "      \"to\": \"%S\",\n",  relto);
            ajFmtPrintF(outf, "    }\n"); /* comma if there are more */
            ajFmtPrintF(outf, "  ]\n");
        }
        else 
        {
            ajFmtPrintF(outf, "  \"%S\": \"%S\",\n",
                        obotag->Name, obotag->Value);
            ajWarn("ontology term '%S' has tag '%S:' unknown in JSON output",
                   obo->Id, obotag->Name);
        }
        
    }

    ajFmtPrintF(outf, "}\n");

    ajListIterDel(&iter);

    ajStrDel(&defstr);
    ajStrDel(&xrefacc);
    ajStrDel(&xrefdb);
    ajStrDel(&synscope);
    ajStrDel(&synstr);
    ajStrDel(&reltype);
    ajStrDel(&relto);

    return ajTrue;
}




/* @funcstatic oboWriteList ***************************************************
**
** Write an OBO term as the simple id with the ontology name prefix
**
** @param [u] outf [AjPFile] Output file
** @param [r] obo [const AjPObo] Obo term object
**
** @return [AjBool] True on success
**
**
** @release 6.4.0
******************************************************************************/

static AjBool oboWriteList(AjPFile outf, const AjPObo obo)
{
    if(!outf) return ajFalse;
    if(!obo) return ajFalse;

    ajWritelineNewline(outf, obo->Fullid);

    return ajTrue;
}




/* @funcstatic oboWriteObo ****************************************************
**
** Write an OBO term in OBO format
**
** @param [u] outf [AjPFile] Output file
** @param [r] obo [const AjPObo] Obo term object
**
** @return [AjBool] True on success
**
**
** @release 6.4.0
******************************************************************************/

static AjBool oboWriteObo(AjPFile outf, const AjPObo obo)
{
    AjIList iter = NULL;
    AjPObotag obotag;

    if(!outf) return ajFalse;
    if(!obo) return ajFalse;

    ajFmtPrintF(outf, "[Term]\n");

    if(ajStrGetLen(obo->Db))
    {
        if(ajStrGetLen(obo->Trueid))
            ajFmtPrintF(outf, "id: %S:%S\n", obo->Db, obo->Trueid);
        else
            ajFmtPrintF(outf, "id: %S:%S\n", obo->Db, obo->Id);
    }
    else
    {
        if(ajStrGetLen(obo->Trueid))
            ajFmtPrintF(outf, "id: %S\n", obo->Trueid);
        else
            ajFmtPrintF(outf, "id: %S\n", obo->Id);
    }
    
    if(ajStrGetLen(obo->Name))
        ajFmtPrintF(outf, "name: %S\n", obo->Name);
    if(ajStrGetLen(obo->Namespace))
        ajFmtPrintF(outf, "namespace: %S\n", obo->Namespace);

    if(ajStrGetLen(obo->Trueid))
        ajFmtPrintF(outf, "alt_id: %S\n", obo->Id);

    if(ajStrGetLen(obo->Def))
        ajFmtPrintF(outf, "def: %S\n", obo->Def);

    if(ajStrGetLen(obo->Comment))
        ajFmtPrintF(outf, "comment: %S\n", obo->Comment);

    iter = ajListIterNewread(obo->Taglist);
    while(!ajListIterDone(iter))
    {
        obotag = (AjPObotag) ajListIterGet(iter);
        ajFmtPrintF(outf, "%S: %S", obotag->Name, obotag->Value);
        if(ajStrGetLen(obotag->Modifier))
            ajFmtPrintF(outf, " %S",
                        obotag->Modifier);
        if(ajStrGetLen(obotag->Comment))
            ajFmtPrintF(outf, " ! %S",
                        obotag->Comment);
        ajFmtPrintF(outf, "\n");
    }
    ajListIterDel(&iter);
    
    ajFmtPrintF(outf, "\n");

    return ajTrue;
}





/* @funcstatic oboWriteObobrief ***********************************************
**
** Write an OBO term in brief OBO format (id, name and namespace only)
**
** @param [u] outf [AjPFile] Output file
** @param [r] obo [const AjPObo] Obo term object
**
** @return [AjBool] True on success
**
**
** @release 6.4.0
******************************************************************************/

static AjBool oboWriteObobrief(AjPFile outf, const AjPObo obo)
{
    if(!outf) return ajFalse;
    if(!obo) return ajFalse;

    ajFmtPrintF(outf, "[Term]\n");

    if(ajStrGetLen(obo->Trueid))
        ajFmtPrintF(outf, "id: %S\n", obo->Trueid);
    else
        ajFmtPrintF(outf, "id: %S\n", obo->Id);

    if(ajStrGetLen(obo->Name))
        ajFmtPrintF(outf, "name: %S\n", obo->Name);
    if(ajStrGetLen(obo->Namespace))
        ajFmtPrintF(outf, "namespace: %S\n", obo->Namespace);

    return ajFalse;
}




/* @funcstatic oboWriteXml ****************************************************
**
** Write an OBO term in OBO-XML format
**
** @param [u] outf [AjPFile] Output file
** @param [r] obo [const AjPObo] Obo term object
**
** @return [AjBool] True on success
**
**
** @release 6.4.0
******************************************************************************/

static AjBool oboWriteXml(AjPFile outf, const AjPObo obo)
{
    AjIList iter = NULL;
    AjPObotag obotag;
    AjPStr defstr = NULL;
    AjPStr xrefacc = NULL;
    AjPStr xrefdb = NULL;
    AjPStr synscope = NULL;
    AjPStr synstr = NULL;
    AjPStr reltype = NULL;
    AjPStr relto = NULL;
    ajuint nrefs = 0;
    ajuint i;

    if(!outf) return ajFalse;
    if(!obo) return ajFalse;

    ajFmtPrintF(outf, "<term>\n");

    if(ajStrGetLen(obo->Trueid))
        ajFmtPrintF(outf, "  <id>%S</id>\n", obo->Trueid);
    else
        ajFmtPrintF(outf, "  <id>%S</id>\n", obo->Id);

    if(ajStrGetLen(obo->Name))
        ajFmtPrintF(outf, "  <name>%S</name>\n", obo->Name);
    if(ajStrGetLen(obo->Namespace))
        ajFmtPrintF(outf, "  <namespace>%S</namespace>\n", obo->Namespace);

    /*
    if(ajStrGetLen(obo->Trueid))
        ajFmtPrintF(outf, "alt_id: %S\n", obo->Id);
    */
    if(ajStrGetLen(obo->Def))
    {
        ajOboGetDef(obo, &defstr, &nrefs);
        ajFmtPrintF(outf, "  <def>\n");
        ajFmtPrintF(outf, "    <defstr>%S</defstr>\n", defstr);
        for(i=1; i<= nrefs; i++)
        {
            /*ajOboGetDefXref(obo, i, &xrefdb, &xrefacc);*/
            ajFmtPrintF(outf, "    <dbxref>\n");
            ajFmtPrintF(outf, "      <acc>%S</acc>\n", xrefacc);
            ajFmtPrintF(outf, "      <dbname>%S</dbname>\n", xrefdb);
            ajFmtPrintF(outf, "    </dbxref>\n");
        }
        ajFmtPrintF(outf, "  </def>\n");
    }
    
    if(ajStrGetLen(obo->Comment))
        ajFmtPrintF(outf, "  <comment>%S</comment>\n", obo->Comment);

    iter = ajListIterNewread(obo->Taglist);
    while(!ajListIterDone(iter))
    {
        obotag = (AjPObotag) ajListIterGet(iter);
        if(ajStrMatchC(obotag->Name, "is_obsolete") ||
           ajStrMatchC(obotag->Name, "built_in") ||
           ajStrMatchC(obotag->Name, "is_anonymous")) 
        {
            if(ajStrMatchC(obotag->Name, "true"))
                ajFmtPrintF(outf, "  <%S>1</%S>\n",
                            obotag->Name, obotag->Name);
        }
        else if (ajStrMatchC(obotag->Name, "consider") ||
                 ajStrMatchC(obotag->Name, "is_a") ||
                 ajStrMatchC(obotag->Name, "intersection_of") ||
                 ajStrMatchC(obotag->Name, "union_of") ||
                 ajStrMatchC(obotag->Name, "disjoint_from") ||
                 ajStrMatchC(obotag->Name, "subset") ||
                 ajStrMatchC(obotag->Name, "replaced_by") ||
                 ajStrMatchC(obotag->Name, "alt_id")) 
        {
            ajFmtPrintF(outf, "  <%S>%S</%S>\n",
                        obotag->Name, obotag->Value, obotag->Name);
        }
        else if(ajStrMatchC(obotag->Name, "synonym")) 
        {
            ajFmtPrintF(outf, "  <synonym score=\"%S\">\n", synscope);
            ajFmtPrintF(outf, "    <synonym_text>%S</synonym_text>\n",
                        synstr);
            ajFmtPrintF(outf, "  </synonym\n>");
        }
        
        else if(ajStrMatchC(obotag->Name, "relationship")) 
        {
            ajFmtPrintF(outf, "  <relationship>\n");
            ajFmtPrintF(outf, "    <type>%S</type>\n",  reltype);
            ajFmtPrintF(outf, "    <to>%S</to>\n",  relto);
            ajFmtPrintF(outf, "  </relationship>\n");
        }
        
        else 
        {
            ajFmtPrintF(outf, "  <%S>%S</%S>\n",
                        obotag->Name, obotag->Value, obotag->Name);
            ajWarn("ontology term '%S' has tag '%S:' unknown in OBO-XML output",
                   obo->Id, obotag->Name);
        }
    }

    ajListIterDel(&iter);

    ajStrDel(&defstr);
    ajStrDel(&xrefacc);
    ajStrDel(&xrefdb);
    ajStrDel(&synscope);
    ajStrDel(&synstr);
    ajStrDel(&reltype);
    ajStrDel(&relto);

    return ajTrue;
}




/* @datasection [none] Miscellaneous functions ********************************
**
** Functions to initialise and clean up internals
**
** @nam2rule Oboout Obo term output internals
**
******************************************************************************/




/* @section Printing **********************************************************
**
** Printing details of the internals to a file
**
** @fdata [none]
**
** @nam2rule Obooutprint
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




/* @func ajObooutprintBook ****************************************************
**
** Reports the obo term format internals as Docbook text
**
** @param [u] outf [AjPFile] Output file
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajObooutprintBook(AjPFile outf)
{
    ajint i = 0;

    ajFmtPrintF(outf, "<para>The supported obo term output "
                "formats are summarised "
                "in the table below. The columns are as follows: "
                "<emphasis>Output format</emphasis> (format name), "
                 "<emphasis>Description</emphasis> (short description of "
                "the format).</para> \n\n");

    ajFmtPrintF(outf, "<table frame=\"box\" rules=\"cols\">\n");
    ajFmtPrintF(outf, "  <caption>Obo term output formats</caption>\n");
    ajFmtPrintF(outf, "  <thead>\n");
    ajFmtPrintF(outf, "    <tr align=\"center\">\n");
    ajFmtPrintF(outf, "      <th>Output Format</th>\n");
    ajFmtPrintF(outf, "      <th>Description</th>\n");
    ajFmtPrintF(outf, "    </tr>\n");
    ajFmtPrintF(outf, "  </thead>\n");
    ajFmtPrintF(outf, "  <tbody>\n");

    for(i=0; obooutFormatDef[i].Name; i++)
    {
        ajFmtPrintF(outf, "    <tr>\n");
        ajFmtPrintF(outf, "      <td>%s</td>\n",
                    obooutFormatDef[i].Name);
        ajFmtPrintF(outf, "      <td>%s</td>\n",
                    obooutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "  </tbody>\n");
    ajFmtPrintF(outf, "</table>\n");

    return;
}




/* @func ajObooutprintHtml ****************************************************
**
** Reports the internal data structures
**
** @param [u] outf [AjPFile] Output file
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajObooutprintHtml(AjPFile outf)
{
    ajint i = 0;

    ajFmtPrintF(outf, "<table border=3>");
    ajFmtPrintF(outf, "<tr><th>Obo term Format</th>\n");
    ajFmtPrintF(outf, "<th>Description</th></tr>\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# Obo term output formats\n");
    ajFmtPrintF(outf, "# Name    Format name (or alias)\n");
    ajFmtPrintF(outf, "# Desc    Format description\n");
    ajFmtPrintF(outf, "# Name         Description\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "OboFormat {\n");

    for(i=0; obooutFormatDef[i].Name; i++)
    {
        ajFmtPrintF(outf, "<tr><td>\n%-12s\n"
                        "<td>\"%s\"</td></tr>\n",
			obooutFormatDef[i].Name,
			obooutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajObooutprintText ****************************************************
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

void ajObooutprintText(AjPFile outf, AjBool full)
{
    ajint i = 0;

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# Obo term output formats\n");
    ajFmtPrintF(outf, "# Name    Format name (or alias)\n");
    ajFmtPrintF(outf, "# Desc    Format description\n");
    ajFmtPrintF(outf, "# Name         Description\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "OboFormat {\n");

    for(i=0; obooutFormatDef[i].Name; i++)
    {
	if(full)
	    ajFmtPrintF(outf, "  %-12s \"%s\"\n",
			obooutFormatDef[i].Name,
			obooutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajObooutprintWiki ****************************************************
**
** Reports the obo term output format internals as wikitext
**
** @param [u] outf [AjPFile] Output file
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajObooutprintWiki(AjPFile outf)
{
    ajint i = 0;
    ajint j = 0;
    AjPStr namestr = NULL;

    ajFmtPrintF(outf, "{| class=\"wikitable sortable\" border=\"2\"\n");
    ajFmtPrintF(outf, "|-\n");
    ajFmtPrintF(outf, "!Format!!"
                "class=\"unsortable\"|Description\n");

    for(i=0; obooutFormatDef[i].Name; i++)
    {
        ajFmtPrintF(outf, "|-\n");
        ajStrAssignC(&namestr, obooutFormatDef[i].Name);

        for(j=i+1; obooutFormatDef[j].Name; j++)
        {
            if(obooutFormatDef[j].Write == obooutFormatDef[i].Write)
            {
                ajFmtPrintAppS(&namestr, "<br>%s",
                               obooutFormatDef[j].Name);
                ajWarn("Obo term output format '%s' same as '%s' "
                       "but not alias",
                       obooutFormatDef[j].Name,
                       obooutFormatDef[i].Name);
            }
        }
        ajFmtPrintF(outf, "|%S||%s\n",
		    namestr,
		    obooutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "|}\n");

    ajStrDel(&namestr);

    return;
}




/* @datasection [none] Output formats *****************************************
**
** Output formats internals
**
** @nam2rule Obooutformat Obo term output format specific
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




/* @func ajObooutformatFind ***************************************************
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

AjBool ajObooutformatFind(const AjPStr format, ajint* iformat)
{
    AjPStr tmpformat = NULL;
    ajuint i = 0;

    /* ajDebug("obooutformatFind '%S'\n", format); */
    if(!ajStrGetLen(format))
	return ajFalse;

    ajStrAssignS(&tmpformat, format);
    ajStrFmtLower(&tmpformat);

    for(i=0; obooutFormatDef[i].Name; i++)
    {
	/* ajDebug("test %d '%s' \n", i, obooutFormatDef[i].Name); */
	if(ajStrMatchCaseC(tmpformat, obooutFormatDef[i].Name))
	{
	    *iformat = i;
	    ajStrDel(&tmpformat);
	    /* ajDebug("found '%s' at %d\n", obooutFormatDef[i].Name, i); */
	    return ajTrue;
	}
    }

    ajErr("Unknown output format '%S'", format);

    ajStrDel(&tmpformat);

    *iformat = 0;

    return ajFalse;
}




/* @func ajObooutformatTest ***************************************************
**
** Tests whether a named OBO output format is known
**
** @param [r] format [const AjPStr] Format
** @return [AjBool] ajTrue if formats was accepted
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajObooutformatTest(const AjPStr format)
{
    ajint i;

    for (i=0; obooutFormatDef[i].Name; i++)
	if(ajStrMatchCaseC(format, obooutFormatDef[i].Name))
	    return ajTrue;

    return ajFalse;
}




/* @datasection [none] Miscellaneous functions ********************************
**
** Functions to initialise and clean up internals
**
** @nam2rule Oboout Obo output internals
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




/* @func ajObooutExit *********************************************************
**
** Cleans up obo term outputinternal memory
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajObooutExit(void)
{
    return;
}

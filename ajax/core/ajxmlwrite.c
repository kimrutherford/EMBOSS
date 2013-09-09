/* @source ajxmlwrite ********************************************************
**
** AJAX xml writing functions
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.1 $
** @modified Oct 25 2010 pmr First AJAX version
** @modified $Date: 2012/09/03 18:19:48 $ by $Author: rice $
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

#include "ajxmlwrite.h"
#include "ajfile.h"

static AjBool xmloutWriteXml(AjPFile outf, const AjPXml xml);
static AjBool xmloutWriteXmlpretty(AjPFile outf, const AjPXml xml);





/* @datastatic XmlPOutFormat ************************************************
**
** Xml output formats data structure
**
** @alias XmlSoutFormat
** @alias XmlOOutFormat
**
** @attr Name [const char*] Format name
** @attr Desc [const char*] Format description
** @attr Write [AjBool function] Output function, returns ajTrue on success
** @@
******************************************************************************/

typedef struct XmlSOutFormat
{
    const char *Name;
    const char *Desc;
    AjBool (*Write) (AjPFile outf, const AjPXml xml);
} XmlOOutFormat;

#define XmlPOutFormat XmlOOutFormat*


static XmlOOutFormat xmloutFormatDef[] =
{
/* "Name",        "Description" */
/*     WriteFunction */

  {"xml",         "General XML format",
       &xmloutWriteXml},

  {"xmlpretty",   "Indented XML format",
       &xmloutWriteXmlpretty},

  {NULL, NULL, NULL}
};




/* @filesection ajxml *******************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
******************************************************************************/





/* @datasection [AjPOutfile] Xml data output *****************************
**
** Function is for manipulating xml data objects
**
** @nam2rule Xmlout Xml data output
**
******************************************************************************/




/* @section xml data outputs *********************************************
**
** These functions write the xml data provided by the first argument
**
** @fdata [AjPOutfile]
**
** @nam3rule Write Write xml data
** @nam4rule Format Use a named format
**
** @argrule Write outf [AjPOutfile] Output file
** @argrule Write xml [const AjPXml] xml data
** @argrule Format fmt [const AjPStr] Format name
**
** @valrule * [AjBool] true on success
**
** @fcategory output
**
******************************************************************************/




/* @func ajXmloutWrite *******************************************************
**
** Write xml data in a named format
**
** @param [u] outf [AjPOutfile] Output file
** @param [r] xml [const AjPXml] Xml data object
**
** @return [AjBool] True on success
**
******************************************************************************/

AjBool ajXmloutWrite(AjPOutfile outf, const AjPXml xml)
{
    ajuint i = ajOutfileGetFormatindex(outf);
    AjPFile outfile = ajOutfileGetFile(outf);

    return (*xmloutFormatDef[i].Write)(outfile, xml);
}




/* @funcstatic xmloutWriteXml ***********************************************
**
** Write xml data in general XML format
**
** @param [u] outf [AjPFile] Output file
** @param [r] xml [const AjPXml] Xml data object
**
** @return [AjBool] True on success
**
******************************************************************************/

static AjBool xmloutWriteXml(AjPFile outf, const AjPXml xml)
{
    if(!outf) return ajFalse;
    if(!xml) return ajFalse;

    ajDomWrite(xml->Doc, outf);

    return ajTrue;
}




/* @funcstatic xmloutWriteXmlpretty *******************************************
**
** Write xml data in indented general XML format
**
** @param [u] outf [AjPFile] Output file
** @param [r] xml [const AjPXml] Xml data object
**
** @return [AjBool] True on success
**
******************************************************************************/

static AjBool xmloutWriteXmlpretty(AjPFile outf, const AjPXml xml)
{
    if(!outf) return ajFalse;
    if(!xml) return ajFalse;

    ajDomWriteIndent(xml->Doc, outf, 0);

    return ajTrue;
}




/* @datasection [none] Miscellaneous functions ********************************
**
** Functions to initialise and clean up internals
**
** @nam2rule Xmlout Xml output internals
**
******************************************************************************/




/* @section Printing **********************************************************
**
** Printing details of the internals to a file
**
** @fdata [none]
**
** @nam2rule Xmloutprint
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




/* @func ajXmloutprintBook ***************************************************
**
** Reports the xml format internals as Docbook text
**
** @param [u] outf [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

void ajXmloutprintBook(AjPFile outf)
{
    ajint i = 0;

    ajFmtPrintF(outf, "<para>The supported xml output "
                "formats are summarised "
                "in the table below. The columns are as follows: "
                "<emphasis>Output format</emphasis> (format name), "
                 "<emphasis>Description</emphasis> (short description of "
                "the format).</para> \n\n");

    ajFmtPrintF(outf, "<table frame=\"box\" rules=\"cols\">\n");
    ajFmtPrintF(outf, "  <caption>Xml output formats</caption>\n");
    ajFmtPrintF(outf, "  <thead>\n");
    ajFmtPrintF(outf, "    <tr align=\"center\">\n");
    ajFmtPrintF(outf, "      <th>Output Format</th>\n");
    ajFmtPrintF(outf, "      <th>Description</th>\n");
    ajFmtPrintF(outf, "    </tr>\n");
    ajFmtPrintF(outf, "  </thead>\n");
    ajFmtPrintF(outf, "  <tbody>\n");

    for(i=0; xmloutFormatDef[i].Name; i++)
    {
        ajFmtPrintF(outf, "    <tr>\n");
        ajFmtPrintF(outf, "      <td>%s</td>\n",
                    xmloutFormatDef[i].Name);
        ajFmtPrintF(outf, "      <td>%s</td>\n",
                    xmloutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "  </tbody>\n");
    ajFmtPrintF(outf, "</table>\n");

    return;
}




/* @func ajXmloutprintHtml ***************************************************
**
** Reports the internal data structures
**
** @param [u] outf [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

void ajXmloutprintHtml(AjPFile outf)
{
    ajint i = 0;

    ajFmtPrintF(outf, "<table border=3>");
    ajFmtPrintF(outf, "<tr><th>Xml Format</th>\n");
    ajFmtPrintF(outf, "<th>Description</th></tr>\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# Xml output formats\n");
    ajFmtPrintF(outf, "# Name    Format name (or alias)\n");
    ajFmtPrintF(outf, "# Desc    Format description\n");
    ajFmtPrintF(outf, "# Name         Description\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "XmlFormat {\n");

    for(i=0; xmloutFormatDef[i].Name; i++)
    {
        ajFmtPrintF(outf, "<tr><td>\n%-12s\n"
                        "<td>\"%s\"</td></tr>\n",
			xmloutFormatDef[i].Name,
			xmloutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajXmloutprintText ***************************************************
**
** Reports the internal data structures
**
** @param [u] outf [AjPFile] Output file
** @param [r] full [AjBool] Full report (usually ajFalse)
** @return [void]
** @@
******************************************************************************/

void ajXmloutprintText(AjPFile outf, AjBool full)
{
    ajint i = 0;

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# Xml output formats\n");
    ajFmtPrintF(outf, "# Name    Format name (or alias)\n");
    ajFmtPrintF(outf, "# Desc    Format description\n");
    ajFmtPrintF(outf, "# Name         Description\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "XmlFormat {\n");

    for(i=0; xmloutFormatDef[i].Name; i++)
    {
	if(full)
	    ajFmtPrintF(outf, "  %-12s \"%s\"\n",
			xmloutFormatDef[i].Name,
			xmloutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajXmloutprintWiki ***************************************************
**
** Reports the xml output format internals as wikitext
**
** @param [u] outf [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

void ajXmloutprintWiki(AjPFile outf)
{
    ajint i = 0;
    ajint j = 0;
    AjPStr namestr = NULL;

    ajFmtPrintF(outf, "{| class=\"wikitable sortable\" border=\"2\"\n");
    ajFmtPrintF(outf, "|-\n");
    ajFmtPrintF(outf, "!Format!!"
                "class=\"unsortable\"|Description\n");

    for(i=0; xmloutFormatDef[i].Name; i++)
    {
        ajFmtPrintF(outf, "|-\n");
        ajStrAssignC(&namestr, xmloutFormatDef[i].Name);

        for(j=i+1; xmloutFormatDef[j].Name; j++)
        {
            if(xmloutFormatDef[j].Write == xmloutFormatDef[i].Write)
            {
                ajFmtPrintAppS(&namestr, "<br>%s",
                               xmloutFormatDef[j].Name);
                ajWarn("Xml output format '%s' same as '%s' "
                       "but not alias",
                       xmloutFormatDef[j].Name,
                       xmloutFormatDef[i].Name);
            }
        }
        ajFmtPrintF(outf, "|%S||%s\n",
		    namestr,
		    xmloutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "|}\n");

    ajStrDel(&namestr);

    return;
}




/* @datasection [none] Output formats *****************************************
**
** Output formats internals
**
** @nam2rule Xmloutformat Xml output format specific
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




/* @func ajXmloutformatFind ***************************************************
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

AjBool ajXmloutformatFind(const AjPStr format, ajint* iformat)
{
    AjPStr tmpformat = NULL;
    ajuint i = 0;

    /* ajDebug("ajXmloutformatFind '%S'\n", format); */
    if(!ajStrGetLen(format))
	return ajFalse;

    ajStrAssignS(&tmpformat, format);
    ajStrFmtLower(&tmpformat);

    for(i=0; xmloutFormatDef[i].Name; i++)
    {
	/* ajDebug("test %d '%s' \n", i, xmloutFormatDef[i].Name); */
	if(ajStrMatchCaseC(tmpformat, xmloutFormatDef[i].Name))
	{
	    *iformat = i;
	    ajStrDel(&tmpformat);
	    /* ajDebug("found '%s' at %d\n", xmloutFormatDef[i].Name, i); */
	    return ajTrue;
	}
    }

    ajErr("Unknown output format '%S'", format);

    ajStrDel(&tmpformat);

    *iformat = 0;

    return ajFalse;
}




/* @func ajXmloutformatTest **************************************************
**
** Tests whether a named xml output format is known
**
** @param [r] format [const AjPStr] Format
** @return [AjBool] ajTrue if formats was accepted
** @@
******************************************************************************/

AjBool ajXmloutformatTest(const AjPStr format)
{
    ajint i;

    for (i=0; xmloutFormatDef[i].Name; i++)
	if(ajStrMatchCaseC(format, xmloutFormatDef[i].Name))
	    return ajTrue;

    return ajFalse;
}




/* @datasection [none] Miscellaneous functions ********************************
**
** Functions to initialise and clean up internals
**
** @nam2rule Xmlout Xml output internals
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




/* @func ajXmloutExit ********************************************************
**
** Cleans up xml output internal memory
**
** @return [void]
** @@
******************************************************************************/

void ajXmloutExit(void)
{
    return;
}

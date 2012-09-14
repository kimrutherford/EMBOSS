/* @source ajurlwrite *********************************************************
**
** AJAX url writing functions
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.12 $
** @modified Oct 25 2010 pmr First AJAX version
** @modified $Date: 2011/10/19 14:52:22 $ by $Author: rice $
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

#include "ajurlwrite.h"
#include "ajfile.h"

static AjBool urloutWriteHtml(AjPFile outf, const AjPUrl url);
static AjBool urloutWriteUrl(AjPFile outf, const AjPUrl url);





/* @datastatic UrlPOutFormat ************************************************
**
** Url output formats data structure
**
** @alias UrlSoutFormat
** @alias UrlOOutFormat
**
** @attr Name [const char*] Format name
** @attr Desc [const char*] Format description
** @attr Write [AjBool function] Output function, returns ajTrue on success
** @@
******************************************************************************/

typedef struct UrlSOutFormat
{
    const char *Name;
    const char *Desc;
    AjBool (*Write) (AjPFile outf, const AjPUrl url);
} UrlOOutFormat;

#define UrlPOutFormat UrlOOutFormat*


static UrlOOutFormat urloutFormatDef[] =
{
/* "Name",        "Description" */
/*     WriteFunction */

  {"url",         "URL",
       &urloutWriteUrl},

  {"html",        "HTML markup of URLs",
       &urloutWriteHtml},

  {NULL, NULL, NULL}
};




/* @filesection ajurl *******************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
******************************************************************************/





/* @datasection [AjPOutfile] Url data output ********************************
**
** Function is for manipulating url data objects
**
** @nam2rule Urlout Url data output
**
******************************************************************************/




/* @section url data outputs *********************************************
**
** These functions write the url data provided by the first argument
**
** @fdata [AjPOutfile]
**
** @nam3rule Write Write url data
** @nam4rule Format Use a named format
**
** @argrule Write outf [AjPOutfile] Output file
** @argrule Write url [const AjPUrl] url data
** @argrule Format fmt [const AjPStr] Format name
**
** @valrule * [AjBool] true on success
**
** @fcategory output
**
******************************************************************************/




/* @func ajUrloutWrite ********************************************************
**
** Write url data in a named format
**
** @param [u] outf [AjPOutfile] Output file
** @param [r] url [const AjPUrl] Url data object
**
** @return [AjBool] True on success
**
**
** @release 6.4.0
******************************************************************************/

AjBool ajUrloutWrite(AjPOutfile outf, const AjPUrl url)
{
    ajuint i = ajOutfileGetFormatindex(outf);
    AjPFile outfile = ajOutfileGetFile(outf);

    return (*urloutFormatDef[i].Write)(outfile, url);
}





/* @funcstatic urloutWriteHtml ************************************************
**
** Write url data in HTML markup format
**
** @param [u] outf [AjPFile] Output file
** @param [r] url [const AjPUrl] Url data object
**
** @return [AjBool] True on success
**
**
** @release 6.4.0
******************************************************************************/

static AjBool urloutWriteHtml(AjPFile outf, const AjPUrl url)
{
    if(!outf) return ajFalse;
    if(!url) return ajFalse;

    ajFmtPrintF(outf, "<a href=\"%S\">%S: %S</a>\n",
                url->Full, url->Resqry->Term, url->Id); 
    return ajTrue;
}




/* @funcstatic urloutWriteUrl *************************************************
**
** Write url data in simple URL format
**
** @param [u] outf [AjPFile] Output file
** @param [r] url [const AjPUrl] Url data object
**
** @return [AjBool] True on success
**
**
** @release 6.4.0
******************************************************************************/

static AjBool urloutWriteUrl(AjPFile outf, const AjPUrl url)
{
    if(!outf) return ajFalse;
    if(!url) return ajFalse;

    ajFmtPrintF(outf, "%S        %S: %S\n",
                url->Full, url->Resqry->Term, url->Id); 
    return ajTrue;
}




/* @datasection [none] Miscellaneous functions ********************************
**
** Functions to initialise and clean up internals
**
** @nam2rule Urlout Url output internals
**
******************************************************************************/




/* @section Printing **********************************************************
**
** Printing details of the internals to a file
**
** @fdata [none]
**
** @nam2rule Urloutprint
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




/* @func ajUrloutprintBook ****************************************************
**
** Reports the url format internals as Docbook text
**
** @param [u] outf [AjPFile] Output file
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajUrloutprintBook(AjPFile outf)
{
    ajint i = 0;

    ajFmtPrintF(outf, "<para>The supported url output "
                "formats are summarised "
                "in the table below. The columns are as follows: "
                "<emphasis>Output format</emphasis> (format name), "
                 "<emphasis>Description</emphasis> (short description of "
                "the format).</para> \n\n");

    ajFmtPrintF(outf, "<table frame=\"box\" rules=\"cols\">\n");
    ajFmtPrintF(outf, "  <caption>Url output formats</caption>\n");
    ajFmtPrintF(outf, "  <thead>\n");
    ajFmtPrintF(outf, "    <tr align=\"center\">\n");
    ajFmtPrintF(outf, "      <th>Output Format</th>\n");
    ajFmtPrintF(outf, "      <th>Description</th>\n");
    ajFmtPrintF(outf, "    </tr>\n");
    ajFmtPrintF(outf, "  </thead>\n");
    ajFmtPrintF(outf, "  <tbody>\n");

    for(i=0; urloutFormatDef[i].Name; i++)
    {
        ajFmtPrintF(outf, "    <tr>\n");
        ajFmtPrintF(outf, "      <td>%s</td>\n",
                    urloutFormatDef[i].Name);
        ajFmtPrintF(outf, "      <td>%s</td>\n",
                    urloutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "  </tbody>\n");
    ajFmtPrintF(outf, "</table>\n");

    return;
}




/* @func ajUrloutprintHtml ****************************************************
**
** Reports the internal data structures
**
** @param [u] outf [AjPFile] Output file
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajUrloutprintHtml(AjPFile outf)
{
    ajint i = 0;

    ajFmtPrintF(outf, "<table border=3>");
    ajFmtPrintF(outf, "<tr><th>Url Format</th>\n");
    ajFmtPrintF(outf, "<th>Description</th></tr>\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# Url output formats\n");
    ajFmtPrintF(outf, "# Name    Format name (or alias)\n");
    ajFmtPrintF(outf, "# Desc    Format description\n");
    ajFmtPrintF(outf, "# Name         Description\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "UrlFormat {\n");

    for(i=0; urloutFormatDef[i].Name; i++)
    {
        ajFmtPrintF(outf, "<tr><td>\n%-12s\n"
                        "<td>\"%s\"</td></tr>\n",
			urloutFormatDef[i].Name,
			urloutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajUrloutprintText ****************************************************
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

void ajUrloutprintText(AjPFile outf, AjBool full)
{
    ajint i = 0;

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# Url output formats\n");
    ajFmtPrintF(outf, "# Name    Format name (or alias)\n");
    ajFmtPrintF(outf, "# Desc    Format description\n");
    ajFmtPrintF(outf, "# Name         Description\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "UrlFormat {\n");

    for(i=0; urloutFormatDef[i].Name; i++)
    {
	if(full)
	    ajFmtPrintF(outf, "  %-12s \"%s\"\n",
			urloutFormatDef[i].Name,
			urloutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajUrloutprintWiki ****************************************************
**
** Reports the url output format internals as wikitext
**
** @param [u] outf [AjPFile] Output file
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajUrloutprintWiki(AjPFile outf)
{
    ajint i = 0;
    ajint j = 0;
    AjPStr namestr = NULL;

    ajFmtPrintF(outf, "{| class=\"wikitable sortable\" border=\"2\"\n");
    ajFmtPrintF(outf, "|-\n");
    ajFmtPrintF(outf, "!Format!!"
                "class=\"unsortable\"|Description\n");

    for(i=0; urloutFormatDef[i].Name; i++)
    {
        ajFmtPrintF(outf, "|-\n");
        ajStrAssignC(&namestr, urloutFormatDef[i].Name);

        for(j=i+1; urloutFormatDef[j].Name; j++)
        {
            if(urloutFormatDef[j].Write == urloutFormatDef[i].Write)
            {
                ajFmtPrintAppS(&namestr, "<br>%s",
                               urloutFormatDef[j].Name);
                ajWarn("Url output format '%s' same as '%s' "
                       "but not alias",
                       urloutFormatDef[j].Name,
                       urloutFormatDef[i].Name);
            }
        }
        ajFmtPrintF(outf, "|%S||%s\n",
		    namestr,
		    urloutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "|}\n");

    ajStrDel(&namestr);

    return;
}




/* @datasection [none] Output formats *****************************************
**
** Output formats internals
**
** @nam2rule Urloutformat URL output format specific
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




/* @func ajUrloutformatFind ***************************************************
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

AjBool ajUrloutformatFind(const AjPStr format, ajint* iformat)
{
    AjPStr tmpformat = NULL;
    ajuint i = 0;

    /* ajDebug("ajUrloutformatFind '%S'\n", format); */
    if(!ajStrGetLen(format))
	return ajFalse;

    ajStrAssignS(&tmpformat, format);
    ajStrFmtLower(&tmpformat);

    for(i=0; urloutFormatDef[i].Name; i++)
    {
	/* ajDebug("test %d '%s' \n", i, urloutFormatDef[i].Name); */
	if(ajStrMatchCaseC(tmpformat, urloutFormatDef[i].Name))
	{
	    *iformat = i;
	    ajStrDel(&tmpformat);
	    /* ajDebug("found '%s' at %d\n", urloutFormatDef[i].Name, i); */
	    return ajTrue;
	}
    }

    ajErr("Unknown output format '%S'", format);

    ajStrDel(&tmpformat);

    *iformat = 0;

    return ajFalse;
}




/* @func ajUrloutformatTest ***************************************************
**
** Tests whether a named URL output format is known
**
** @param [r] format [const AjPStr] Format
** @return [AjBool] ajTrue if formats was accepted
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajUrloutformatTest(const AjPStr format)
{
    ajint i;

    for (i=0; urloutFormatDef[i].Name; i++)
	if(ajStrMatchCaseC(format, urloutFormatDef[i].Name))
	    return ajTrue;

    return ajFalse;
}




/* @datasection [none] Miscellaneous functions ********************************
**
** Functions to initialise and clean up internals
**
** @nam2rule Urlout URL output internals
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




/* @func ajUrloutExit *********************************************************
**
** Cleans up url output internal memory
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajUrloutExit(void)
{
    return;
}

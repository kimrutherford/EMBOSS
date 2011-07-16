/******************************************************************************
** @source AJAX TEXT handling functions
**
** @author Copyright (C) 2010 Peter Rice
** @version 1.0
** @modified May 5 pmr 2010 First AJAX version
** @modified Sep 8 2010 pmr Added query and reading functions
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

#include "ajax.h"


static AjBool textoutWriteList(AjPFile outf, const AjPText text);
static AjBool textoutWriteText(AjPFile outf, const AjPText text);
static AjBool textoutWriteHtml(AjPFile outf, const AjPText text);
static AjBool textoutWriteJson(AjPFile outf, const AjPText text);
static AjBool textoutWriteXml(AjPFile outf, const AjPText text);




/* @datastatic TextPOutFormat **************************************************
**
** Text output formats data structure
**
** @alias TextSoutFormat
** @alias TextOOutFormat
**
** @attr Name [const char*] Format name
** @attr Desc [const char*] Format description
** @attr Write [(AjBool*)] Output function, returns ajTrue on success
** @@
******************************************************************************/

typedef struct TextSOutFormat
{
    const char *Name;
    const char *Desc;
    AjBool (*Write) (AjPFile outf, const AjPText text);
} TextOOutFormat;

#define TextPOutFormat TextOOutFormat*


static TextOOutFormat textoutFormatDef[] =
{
/* "Name",        "Description" */
/*     WriteFunction */
  {"text",         "Plain text",
       textoutWriteText},
  {"html",        "HTML with markup",
       textoutWriteHtml},
  {"xml",         "XML format",
       textoutWriteXml},
  {"json",        "JSON format",
       textoutWriteJson},
  {"list",        "Identifier only",
       textoutWriteList},
  {NULL, NULL, NULL}
};




/* @filesection ajtext ********************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
******************************************************************************/





/* @datasection [AjPFile] Text data output *************************************
**
** Function is for manipulating text data objects
**
** @nam2rule Textout Text data output
**
******************************************************************************/




/* @section text data outputs **********************************************
**
** These functions write the text data provided by the first argument
**
** @fdata [AjPFile]
**
** @nam3rule Write Write text data
** @nam4rule Format Use a named format
**
** @argrule Write outf [AjPOutfile] Output file
** @argrule Write text [const AjPText] Text data
** @argrule Format fmt [const AjPStr] Format name
**
** @valrule * [AjBool] true on success
**
** @fcategory output
**
******************************************************************************/




/* @func ajTextoutWrite ********************************************************
**
** Write a text in a named format
**
** @param [u] outf [AjPOutfile] Output file
** @param [r] text [const AjPText] Text object
**
** @return [AjBool] True on success
**
******************************************************************************/

AjBool ajTextoutWrite(AjPOutfile outf, const AjPText text)
{
    ajuint i = ajOutfileGetFormatindex(outf);
    AjPFile outfile = ajOutfileGetFile(outf);

    return textoutFormatDef[i].Write(outfile, text);
}





/* @funcstatic textoutWriteHtml ***********************************************
**
** Write text data in an HTML format
**
** @param [u] outf [AjPFile] Output file
** @param [r] text [const AjPText] Text data object
**
** @return [AjBool] True on success
**
******************************************************************************/

static AjBool textoutWriteHtml(AjPFile outf, const AjPText text)
{
    AjIList iter = NULL;
    AjPStr line = NULL;

    iter = ajListIterNewread(text->Lines);
    while(!ajListIterDone(iter))
    {
        line = ajListIterGet(iter);
        ajWritelineNewline(outf, line);
    }
    ajListIterDel(&iter);
    
    return ajTrue;
}




/* @funcstatic textoutWriteJson ***********************************************
**
** Write text data in JSON format
**
** @param [u] outf [AjPFile] Output file
** @param [r] text [const AjPText] Text data object
**
** @return [AjBool] True on success
**
******************************************************************************/

static AjBool textoutWriteJson(AjPFile outf, const AjPText text)
{
    AjIList iter = NULL;
    AjPStr line = NULL;

    iter = ajListIterNewread(text->Lines);
    while(!ajListIterDone(iter))
    {
        line = ajListIterGet(iter);
        ajWritelineNewline(outf, line);
    }
    ajListIterDel(&iter);
    
    return ajTrue;
}




/* @funcstatic textoutWriteList ***********************************************
**
** Write text data in listfile format
**
** @param [u] outf [AjPFile] Output file
** @param [r] text [const AjPText] Text data object
**
** @return [AjBool] True on success
**
******************************************************************************/

static AjBool textoutWriteList(AjPFile outf, const AjPText text)
{
    if(!outf) return ajFalse;
    if(!text) return ajFalse;

    if(ajStrGetLen(text->Db))
        ajFmtPrintF(outf, "%S:%S\n", text->Db, text->Id);
    else
        ajFmtPrintF(outf, "%S\n", text->Id);
    
    return ajTrue;
}




/* @funcstatic textoutWriteText ***********************************************
**
** Write text data in TEXT format
**
** @param [u] outf [AjPFile] Output file
** @param [r] text [const AjPText] Text data object
**
** @return [AjBool] True on success
**
******************************************************************************/

static AjBool textoutWriteText(AjPFile outf, const AjPText text)
{
    AjIList iter = NULL;
    AjPStr line = NULL;

    iter = ajListIterNewread(text->Lines);
    while(!ajListIterDone(iter))
    {
        line = ajListIterGet(iter);
        ajWritelineNewline(outf, line);
    }
    ajListIterDel(&iter);
    
    ajWritebinNewline(outf);

    return ajTrue;
}





/* @funcstatic textoutWriteXml ************************************************
**
** Write text data in TEXT-XML format
**
** @param [u] outf [AjPFile] Output file
** @param [r] text [const AjPText] Text data object
**
** @return [AjBool] True on success
**
******************************************************************************/

static AjBool textoutWriteXml(AjPFile outf, const AjPText text)
{
    AjIList iter = NULL;
    AjPStr line = NULL;

    iter = ajListIterNewread(text->Lines);
    while(!ajListIterDone(iter))
    {
        line = ajListIterGet(iter);
        ajWritelineNewline(outf, line);
    }
    ajListIterDel(&iter);
    
    ajWritebinNewline(outf);

    return ajTrue;
}




/* @datasection [none] Miscellaneous functions ********************************
**
** Functions to initialise and clean up internals
**
** @nam2rule Textout Text output internals
**
******************************************************************************/




/* @section Printing **********************************************************
**
** Printing details of the internals to a file
**
** @fdata [none]
**
** @nam2rule Textoutprint
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




/* @func ajTextoutprintBook ***************************************************
**
** Reports the text format internals as Docbook text
**
** @param [u] outf [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

void ajTextoutprintBook(AjPFile outf)
{
    ajint i = 0;

    ajFmtPrintF(outf, "<para>The supported text output "
                "formats are summarised "
                "in the table below. The columns are as follows: "
                "<emphasis>Output format</emphasis> (format name), "
                 "<emphasis>Description</emphasis> (short description of "
                "the format).</para> \n\n");

    ajFmtPrintF(outf, "<table frame=\"box\" rules=\"cols\">\n");
    ajFmtPrintF(outf, "  <caption>Text output formats</caption>\n");
    ajFmtPrintF(outf, "  <thead>\n");
    ajFmtPrintF(outf, "    <tr align=\"center\">\n");
    ajFmtPrintF(outf, "      <th>Output Format</th>\n");
    ajFmtPrintF(outf, "      <th>Description</th>\n");
    ajFmtPrintF(outf, "    </tr>\n");
    ajFmtPrintF(outf, "  </thead>\n");
    ajFmtPrintF(outf, "  <tbody>\n");

    for(i=0; textoutFormatDef[i].Name; i++)
    {
        ajFmtPrintF(outf, "    <tr>\n");
        ajFmtPrintF(outf, "      <td>%s</td>\n",
                    textoutFormatDef[i].Name);
        ajFmtPrintF(outf, "      <td>%s</td>\n",
                    textoutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "  </tbody>\n");
    ajFmtPrintF(outf, "</table>\n");

    return;
}




/* @func ajTextoutprintHtml ***************************************************
**
** Reports the internal data structures
**
** @param [u] outf [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

void ajTextoutprintHtml(AjPFile outf)
{
    ajint i = 0;

    ajFmtPrintF(outf, "<table border=3>");
    ajFmtPrintF(outf, "<tr><th>Text Format</th>\n");
    ajFmtPrintF(outf, "<th>Description</th></tr>\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# text output formats\n");
    ajFmtPrintF(outf, "# Name    Format name (or alias)\n");
    ajFmtPrintF(outf, "# Desc    Format description\n");
    ajFmtPrintF(outf, "# Name         Description\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "TextFormat {\n");

    for(i=0; textoutFormatDef[i].Name; i++)
    {
        ajFmtPrintF(outf, "<tr><td>\n%-12s\n"
                        "<td>\"%s\"</td></tr>\n",
			textoutFormatDef[i].Name,
			textoutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajTextoutprintText ***************************************************
**
** Reports the internal data structures
**
** @param [u] outf [AjPFile] Output file
** @param [r] full [AjBool] Full report (usually ajFalse)
** @return [void]
** @@
******************************************************************************/

void ajTextoutprintText(AjPFile outf, AjBool full)
{
    ajint i = 0;

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# Text output formats\n");
    ajFmtPrintF(outf, "# Name    Format name (or alias)\n");
    ajFmtPrintF(outf, "# Desc    Format description\n");
    ajFmtPrintF(outf, "# Name         Description\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "TextFormat {\n");

    for(i=0; textoutFormatDef[i].Name; i++)
    {
	if(full)
	    ajFmtPrintF(outf, "  %-12s \"%s\"\n",
			textoutFormatDef[i].Name,
			textoutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajTextoutprintWiki ***************************************************
**
** Reports the text output format internals as wikitext
**
** @param [u] outf [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

void ajTextoutprintWiki(AjPFile outf)
{
    ajint i = 0;
    ajint j = 0;
    AjPStr namestr = NULL;

    ajFmtPrintF(outf, "{| class=\"wikitable sortable\" border=\"2\"\n");
    ajFmtPrintF(outf, "|-\n");
    ajFmtPrintF(outf, "!Format!!"
                "class=\"unsortable\"|Description\n");

    for(i=0; textoutFormatDef[i].Name; i++)
    {
        ajFmtPrintF(outf, "|-\n");
        ajStrAssignC(&namestr, textoutFormatDef[i].Name);

        for(j=i+1; textoutFormatDef[j].Name; j++)
        {
            if(textoutFormatDef[j].Write == textoutFormatDef[i].Write)
            {
                ajFmtPrintAppS(&namestr, "<br>%s",
                               textoutFormatDef[j].Name);
                ajWarn("Text output format '%s' same as '%s' "
                       "but not alias",
                       textoutFormatDef[j].Name,
                       textoutFormatDef[i].Name);
            }
        }
        ajFmtPrintF(outf, "|%S||%s\n",
		    namestr,
		    textoutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "|}\n");

    ajStrDel(&namestr);

    return;
}




/* @datasection [none] Output formats *****************************************
**
** Output formats internals
**
** @nam2rule Textoutformat Text output format specific
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




/* @func ajTextoutformatFind **************************************************
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

AjBool ajTextoutformatFind(const AjPStr format, ajint* iformat)
{
    AjPStr tmpformat = NULL;
    ajuint i = 0;

    /* ajDebug("textoutformatFind '%S'\n", format); */
    if(!ajStrGetLen(format))
	return ajFalse;

    ajStrAssignS(&tmpformat, format);
    ajStrFmtLower(&tmpformat);

    for(i=0; textoutFormatDef[i].Name; i++)
    {
	/* ajDebug("test %d '%s' \n", i, textoutFormatDef[i].Name); */
	if(ajStrMatchCaseC(tmpformat, textoutFormatDef[i].Name))
	{
	    *iformat = i;
	    ajStrDel(&tmpformat);
	    /* ajDebug("found '%s' at %d\n", textoutFormatDef[i].Name, i); */
	    return ajTrue;
	}
    }

    ajErr("Unknown output format '%S'", format);

    ajStrDel(&tmpformat);

    *iformat = 0;

    return ajFalse;
}




/* @func ajTextoutformatTest ***************************************************
**
** Tests whether a named text output format is known
**
** @param [r] format [const AjPStr] Format
** @return [AjBool] ajTrue if formats was accepted
** @@
******************************************************************************/

AjBool ajTextoutformatTest(const AjPStr format)
{
    ajint i;

    for (i=0; textoutFormatDef[i].Name; i++)
	if(ajStrMatchCaseC(format, textoutFormatDef[i].Name))
	    return ajTrue;

    return ajFalse;
}




/* @datasection [none] Miscellaneous functions ********************************
**
** Functions to initialise and clean up internals
**
** @nam2rule Textout Text output internals
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




/* @func ajTextoutExit *********************************************************
**
** Cleans up text data output internal memory
**
** @return [void]
** @@
******************************************************************************/

void ajTextoutExit(void)
{
    return;
}

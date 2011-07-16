/******************************************************************************
** @source AJAX variation handling functions
**
** @author Copyright (C) 2010 Peter Rice
** @version 1.0
** @modified Oct 25 2010 pmr First AJAX version
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

static AjBool varoutWriteAbc(AjPFile outf, const AjPVar var);





/* @datastatic VarPOutFormat ************************************************
**
** Variation output formats data structure
**
** @alias VarSoutFormat
** @alias VarOOutFormat
**
** @attr Name [const char*] Format name
** @attr Desc [const char*] Format description
** @attr Write [(AjBool*)] Output function, returns ajTrue on success
** @@
******************************************************************************/

typedef struct VarSOutFormat
{
    const char *Name;
    const char *Desc;
    AjBool (*Write) (AjPFile outf, const AjPVar var);
} VarOOutFormat;

#define VarPOutFormat VarOOutFormat*


static VarOOutFormat varoutFormatDef[] =
{
/* "Name",        "Description" */
/*     WriteFunction */

  {"abc",         "Abc format description",
       varoutWriteAbc},

  {NULL, NULL, NULL},

  {NULL, NULL, NULL}
};




/* @filesection ajvar *******************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
******************************************************************************/





/* @datasection [AjPoutfile] Variation data output ****************************
**
** Function is for manipulating variation data objects
**
** @nam2rule Varout Variation data output
**
******************************************************************************/




/* @section variation data outputs *********************************************
**
** These functions write the variation data provided by the first argument
**
** @fdata [AjPoutfile]
**
** @nam3rule Write Write variation data
** @nam4rule Format Use a named format
**
** @argrule Write outf [AjPOutfile] Outpur file
** @argrule Write var [const AjPVar] variation data
** @argrule Format fmt [const AjPStr] Format name
**
** @valrule * [AjBool] true on success
**
** @fcategory output
**
******************************************************************************/




/* @func ajVaroutWrite ********************************************************
**
** Write variation data in a named format
**
** @param [u] outf [AjPOutfile] Output file
** @param [r] var [const AjPVar] Variation object
**
** @return [AjBool] True on success
**
******************************************************************************/

AjBool ajVaroutWrite(AjPOutfile outf, const AjPVar var)
{
    ajuint i = ajOutfileGetFormatindex(outf);
    AjPFile outfile = ajOutfileGetFile(outf);

    return varoutFormatDef[i].Write(outfile, var);
}





/* @funcstatic varoutWriteAbc ***********************************************
**
** Write variation data in abc format
**
** @param [u] outf [AjPFile] Output file
** @param [r] var [const AjPVar] Variation data object
**
** @return [AjBool] True on success
**
******************************************************************************/

static AjBool varoutWriteAbc(AjPFile outf, const AjPVar var)
{
    if(!outf) return ajFalse;
    if(!var) return ajFalse;

    return ajTrue;
}




/* @datasection [none] Miscellaneous functions ********************************
**
** Functions to initialise and clean up internals
**
** @nam2rule Varout Variation output internals
**
******************************************************************************/




/* @section Printing **********************************************************
**
** Printing details of the internals to a file
**
** @fdata [none]
**
** @nam2rule Varoutprint
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




/* @func ajVaroutprintBook ***************************************************
**
** Reports the variation format internals as Docbook text
**
** @param [u] outf [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

void ajVaroutprintBook(AjPFile outf)
{
    ajint i = 0;

    ajFmtPrintF(outf, "<para>The supported variation output "
                "formats are summarised "
                "in the table below. The columns are as follows: "
                "<emphasis>Output format</emphasis> (format name), "
                 "<emphasis>Description</emphasis> (short description of "
                "the format).</para> \n\n");

    ajFmtPrintF(outf, "<table frame=\"box\" rules=\"cols\">\n");
    ajFmtPrintF(outf, "  <caption>Variation output formats</caption>\n");
    ajFmtPrintF(outf, "  <thead>\n");
    ajFmtPrintF(outf, "    <tr align=\"center\">\n");
    ajFmtPrintF(outf, "      <th>Output Format</th>\n");
    ajFmtPrintF(outf, "      <th>Description</th>\n");
    ajFmtPrintF(outf, "    </tr>\n");
    ajFmtPrintF(outf, "  </thead>\n");
    ajFmtPrintF(outf, "  <tbody>\n");

    for(i=0; varoutFormatDef[i].Name; i++)
    {
        ajFmtPrintF(outf, "    <tr>\n");
        ajFmtPrintF(outf, "      <td>%s</td>\n",
                    varoutFormatDef[i].Name);
        ajFmtPrintF(outf, "      <td>%s</td>\n",
                    varoutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "  </tbody>\n");
    ajFmtPrintF(outf, "</table>\n");

    return;
}




/* @func ajVaroutprintHtml ***************************************************
**
** Reports the internal data structures
**
** @param [u] outf [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

void ajVaroutprintHtml(AjPFile outf)
{
    ajint i = 0;

    ajFmtPrintF(outf, "<table border=3>");
    ajFmtPrintF(outf, "<tr><th>Variation Format</th>\n");
    ajFmtPrintF(outf, "<th>Description</th></tr>\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# variation output formats\n");
    ajFmtPrintF(outf, "# Name    Format name (or alias)\n");
    ajFmtPrintF(outf, "# Desc    Format description\n");
    ajFmtPrintF(outf, "# Name         Description\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "VarFormat {\n");

    for(i=0; varoutFormatDef[i].Name; i++)
    {
        ajFmtPrintF(outf, "<tr><td>\n%-12s\n"
                        "<td>\"%s\"</td></tr>\n",
			varoutFormatDef[i].Name,
			varoutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajVaroutprintText ***************************************************
**
** Reports the internal data structures
**
** @param [u] outf [AjPFile] Output file
** @param [r] full [AjBool] Full report (usually ajFalse)
** @return [void]
** @@
******************************************************************************/

void ajVaroutprintText(AjPFile outf, AjBool full)
{
    ajint i = 0;

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# Variation output formats\n");
    ajFmtPrintF(outf, "# Name    Format name (or alias)\n");
    ajFmtPrintF(outf, "# Desc    Format description\n");
    ajFmtPrintF(outf, "# Name         Description\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "VarFormat {\n");

    for(i=0; varoutFormatDef[i].Name; i++)
    {
	if(full)
	    ajFmtPrintF(outf, "  %-12s \"%s\"\n",
			varoutFormatDef[i].Name,
			varoutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajVaroutprintWiki ***************************************************
**
** Reports the variation output format internals as wikitext
**
** @param [u] outf [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

void ajVaroutprintWiki(AjPFile outf)
{
    ajint i = 0;
    ajint j = 0;
    AjPStr namestr = NULL;

    ajFmtPrintF(outf, "{| class=\"wikitable sortable\" border=\"2\"\n");
    ajFmtPrintF(outf, "|-\n");
    ajFmtPrintF(outf, "!Format!!"
                "class=\"unsortable\"|Description\n");

    for(i=0; varoutFormatDef[i].Name; i++)
    {
        ajFmtPrintF(outf, "|-\n");
        ajStrAssignC(&namestr, varoutFormatDef[i].Name);

        for(j=i+1; varoutFormatDef[j].Name; j++)
        {
            if(varoutFormatDef[j].Write == varoutFormatDef[i].Write)
            {
                ajFmtPrintAppS(&namestr, "<br>%s",
                               varoutFormatDef[j].Name);
                ajWarn("Variation output format '%s' same as '%s' "
                       "but not alias",
                       varoutFormatDef[j].Name,
                       varoutFormatDef[i].Name);
            }
        }
        ajFmtPrintF(outf, "|%S||%s\n",
		    namestr,
		    varoutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "|}\n");

    ajStrDel(&namestr);

    return;
}




/* @datasection [none] Output formats *****************************************
**
** Output formats internals
**
** @nam2rule Varoutformat Variation output format specific
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




/* @func ajVaroutformatFind ***************************************************
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

AjBool ajVaroutformatFind(const AjPStr format, ajint* iformat)
{
    AjPStr tmpformat = NULL;
    ajuint i = 0;

    /* ajDebug("ajVaroutformatFind '%S'\n", format); */
    if(!ajStrGetLen(format))
	return ajFalse;

    ajStrAssignS(&tmpformat, format);
    ajStrFmtLower(&tmpformat);

    for(i=0; varoutFormatDef[i].Name; i++)
    {
	/* ajDebug("test %d '%s' \n", i, varoutFormatDef[i].Name); */
	if(ajStrMatchCaseC(tmpformat, varoutFormatDef[i].Name))
	{
	    *iformat = i;
	    ajStrDel(&tmpformat);
	    /* ajDebug("found '%s' at %d\n", varoutFormatDef[i].Name, i); */
	    return ajTrue;
	}
    }

    ajErr("Unknown output format '%S'", format);

    ajStrDel(&tmpformat);

    *iformat = 0;

    return ajFalse;
}




/* @func ajVaroutformatTest ***************************************************
**
** Tests whether a named variation output format is known
**
** @param [r] format [const AjPStr] Format
** @return [AjBool] ajTrue if formats was accepted
** @@
******************************************************************************/

AjBool ajVaroutformatTest(const AjPStr format)
{
    ajint i;

    for (i=0; varoutFormatDef[i].Name; i++)
	if(ajStrMatchCaseC(format, varoutFormatDef[i].Name))
	    return ajTrue;

    return ajFalse;
}




/* @datasection [none] Miscellaneous functions ********************************
**
** Functions to initialise and clean up internals
**
** @nam2rule Varout Variation output internals
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




/* @func ajVaroutExit ********************************************************
**
** Cleans up variation output internal memory
**
** @return [void]
** @@
******************************************************************************/

void ajVaroutExit(void)
{
    return;
}

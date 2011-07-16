/******************************************************************************
** @source AJAX assembly handling functions
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

static AjBool assemoutWriteAbc(AjPFile outf, const AjPAssem assem);
static AjBool assemoutWriteList(AjPFile outf, const AjPAssem assem);





/* @datastatic AssemPOutFormat ************************************************
**
** Assembly output formats data structure
**
** @alias AssemSoutFormat
** @alias AssemOOutFormat
**
** @attr Name [const char*] Format name
** @attr Desc [const char*] Format description
** @attr Write [(AjBool*)] Output function, returns ajTrue on success
** @@
******************************************************************************/

typedef struct AssemSOutFormat
{
    const char *Name;
    const char *Desc;
    AjBool (*Write) (AjPFile outf, const AjPAssem assem);
} AssemOOutFormat;

#define AssemPOutFormat AssemOOutFormat*


static AssemOOutFormat assemoutFormatDef[] =
{
/* "Name",        "Description" */
/*     WriteFunction */

  {"abc",         "Abc format description",
       assemoutWriteAbc},
  {"list",        "Identifier only",
       assemoutWriteList},

  {NULL, NULL, NULL}
};




/* @filesection ajassem *******************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
******************************************************************************/





/* @datasection [AjPFile] Assembly data output ********************************
**
** Function is for manipulating assembly data objects
**
** @nam2rule Assemout Assembly data output
**
******************************************************************************/




/* @section assembly data outputs *********************************************
**
** These functions write the assembly data provided by the first argument
**
** @fdata [AjPFile]
**
** @nam3rule Write Write assembly data
** @nam4rule Format Use a named format
**
** @argrule Write outf [AjPFile] Output file
** @argrule Write assem [const AjPAssem] Assembly data
** @argrule Format fmt [const AjPStr] Format name
**
** @valrule * [AjBool] true on success
**
** @fcategory output
**
******************************************************************************/




/* @func ajAssemoutWriteFormat ************************************************
**
** Write assembly data in a named format
**
** @param [u] outf [AjPFile] Output file
** @param [r] assem [const AjPAssem] Assembly data object
** @param [r] fmt [const AjPStr] Format name
**
** @return [AjBool] True on success
**
******************************************************************************/

AjBool ajAssemoutWriteFormat(AjPFile outf, const AjPAssem assem,
                             const AjPStr fmt)
{
    ajuint i;
    AjPStr fmtname = ajStrNewS(fmt);

    ajStrFmtLower(&fmtname);

    for (i=0; assemoutFormatDef[i].Name; i++)
    {

        if(ajStrMatchC(fmtname, assemoutFormatDef[i].Name))
        {
            ajStrDel(&fmtname);
            return assemoutFormatDef[i].Write(outf, assem);
        }
    }

    ajStrDel(&fmtname);

    ajErr("Unknown format '%S' for assembly data '%S'", fmt, assem->Id);
    return ajFalse;
}





/* @funcstatic assemoutWriteAbc ***********************************************
**
** Write assembly data in abc format
**
** @param [u] outf [AjPFile] Output file
** @param [r] assem [const AjPAssem] Assembly data object
**
** @return [AjBool] True on success
**
******************************************************************************/

static AjBool assemoutWriteAbc(AjPFile outf, const AjPAssem assem)
{
    if(!outf) return ajFalse;
    if(!assem) return ajFalse;

    return ajTrue;
}




/* @funcstatic assemoutWriteList **********************************************
**
** Write an OBO term as the simple id with the ontology name prefix
**
** @param [u] outf [AjPFile] Output file
** @param [r] assem [const AjPAssem] Assembly object
**
** @return [AjBool] True on success
**
******************************************************************************/

static AjBool assemoutWriteList(AjPFile outf, const AjPAssem assem)
{
    if(!outf) return ajFalse;
    if(!assem) return ajFalse;

    if(ajStrGetLen(assem->Db))
        ajFmtPrintF(outf, "%S:%S\n", assem->Db, assem->Id);
    else
        ajFmtPrintF(outf, "%S\n", assem->Id);
    
    return ajTrue;
}




/* @datasection [none] Miscellaneous functions ********************************
**
** Functions to initialise and clean up internals
**
** @nam2rule Assemout Assembly output internals
**
******************************************************************************/




/* @section Printing **********************************************************
**
** Printing details of the internals to a file
**
** @fdata [none]
**
** @nam2rule Assemoutprint
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




/* @func ajAssemoutprintBook **************************************************
**
** Reports the assembly format internals as Docbook text
**
** @param [u] outf [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

void ajAssemoutprintBook(AjPFile outf)
{
    ajint i = 0;

    ajFmtPrintF(outf, "<para>The supported assembly output "
                "formats are summarised "
                "in the table below. The columns are as follows: "
                "<emphasis>Output format</emphasis> (format name), "
                 "<emphasis>Description</emphasis> (short description of "
                "the format).</para> \n\n");

    ajFmtPrintF(outf, "<table frame=\"box\" rules=\"cols\">\n");
    ajFmtPrintF(outf, "  <caption>Assembly output formats</caption>\n");
    ajFmtPrintF(outf, "  <thead>\n");
    ajFmtPrintF(outf, "    <tr align=\"center\">\n");
    ajFmtPrintF(outf, "      <th>Output Format</th>\n");
    ajFmtPrintF(outf, "      <th>Description</th>\n");
    ajFmtPrintF(outf, "    </tr>\n");
    ajFmtPrintF(outf, "  </thead>\n");
    ajFmtPrintF(outf, "  <tbody>\n");

    for(i=0; assemoutFormatDef[i].Name; i++)
    {
        ajFmtPrintF(outf, "    <tr>\n");
        ajFmtPrintF(outf, "      <td>%s</td>\n",
                    assemoutFormatDef[i].Name);
        ajFmtPrintF(outf, "      <td>%s</td>\n",
                    assemoutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "  </tbody>\n");
    ajFmtPrintF(outf, "</table>\n");

    return;
}




/* @func ajAssemoutprintHtml **************************************************
**
** Reports the internal data structures
**
** @param [u] outf [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

void ajAssemoutprintHtml(AjPFile outf)
{
    ajint i = 0;

    ajFmtPrintF(outf, "<table border=3>");
    ajFmtPrintF(outf, "<tr><th>Assembly Format</th>\n");
    ajFmtPrintF(outf, "<th>Description</th></tr>\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# Assembly output formats\n");
    ajFmtPrintF(outf, "# Name    Format name (or alias)\n");
    ajFmtPrintF(outf, "# Desc    Format description\n");
    ajFmtPrintF(outf, "# Name         Description\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "AssemFormat {\n");

    for(i=0; assemoutFormatDef[i].Name; i++)
    {
        ajFmtPrintF(outf, "<tr><td>\n%-12s\n"
                        "<td>\"%s\"</td></tr>\n",
			assemoutFormatDef[i].Name,
			assemoutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajAssemoutprintText **************************************************
**
** Reports the internal data structures
**
** @param [u] outf [AjPFile] Output file
** @param [r] full [AjBool] Full report (usually ajFalse)
** @return [void]
** @@
******************************************************************************/

void ajAssemoutprintText(AjPFile outf, AjBool full)
{
    ajint i = 0;

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# Assembly output formats\n");
    ajFmtPrintF(outf, "# Name    Format name (or alias)\n");
    ajFmtPrintF(outf, "# Desc    Format description\n");
    ajFmtPrintF(outf, "# Name         Description\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "AssemFormat {\n");

    for(i=0; assemoutFormatDef[i].Name; i++)
    {
	if(full)
	    ajFmtPrintF(outf, "  %-12s \"%s\"\n",
			assemoutFormatDef[i].Name,
			assemoutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajAssemoutprintWiki **************************************************
**
** Reports the asembly output format internals as wikitext
**
** @param [u] outf [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

void ajAssemoutprintWiki(AjPFile outf)
{
    ajint i = 0;
    ajint j = 0;
    AjPStr namestr = NULL;

    ajFmtPrintF(outf, "{| class=\"wikitable sortable\" border=\"2\"\n");
    ajFmtPrintF(outf, "|-\n");
    ajFmtPrintF(outf, "!Format!!"
                "class=\"unsortable\"|Description\n");

    for(i=0; assemoutFormatDef[i].Name; i++)
    {
        ajFmtPrintF(outf, "|-\n");
        ajStrAssignC(&namestr, assemoutFormatDef[i].Name);

        for(j=i+1; assemoutFormatDef[j].Name; j++)
        {
            if(assemoutFormatDef[j].Write == assemoutFormatDef[i].Write)
            {
                ajFmtPrintAppS(&namestr, "<br>%s",
                               assemoutFormatDef[j].Name);
                ajWarn("Assembly output format '%s' same as '%s' "
                       "but not alias",
                       assemoutFormatDef[j].Name,
                       assemoutFormatDef[i].Name);
            }
        }
        ajFmtPrintF(outf, "|%S||%s\n",
                    namestr,
                    assemoutFormatDef[i].Desc);
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




/* @func ajAssemoutExit *******************************************************
**
** Cleans up assembly output internal memory
**
** @return [void]
** @@
******************************************************************************/

void ajAssemoutExit(void)
{
    return;
}

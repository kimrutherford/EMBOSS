/* @source ajtaxwrite *********************************************************
**
** AJAX taxonomy writing functions
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.13 $
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

#include "ajtaxwrite.h"
#include "ajfile.h"

static AjBool taxoutWriteEbi(AjPFile outf, const AjPTax tax);
static AjBool taxoutWriteExcel(AjPFile outf, const AjPTax tax);
static AjBool taxoutWriteNcbi(AjPFile outf, const AjPTax tax);
static AjBool taxoutWriteTax(AjPFile outf, const AjPTax tax);





/* @datastatic TaxPOutFormat ************************************************
**
** Taxonomy output formats data structure
**
** @alias TaxSoutFormat
** @alias TaxOOutFormat
**
** @attr Name [const char*] Format name
** @attr Desc [const char*] Format description
** @attr Write [AjBool function] Output function, returns ajTrue on success
** @@
******************************************************************************/

typedef struct TaxSOutFormat
{
    const char *Name;
    const char *Desc;
    AjBool (*Write) (AjPFile outf, const AjPTax tax);
} TaxOOutFormat;

#define TaxPOutFormat TaxOOutFormat*


static TaxOOutFormat taxoutFormatDef[] =
{
/* "Name",        "Description" */
/*     WriteFunction */

  {"ncbi",         "NCBI taxonomy format",
       &taxoutWriteNcbi},

  {"ebi",          "EBI taxonomy.dat format",
       &taxoutWriteEbi},

  {"tax",          "EMBOSS taxon format",
       &taxoutWriteTax},

  {"excel",       "Tab-delimited file for import to Microsoft Excel",
       &taxoutWriteExcel},
  {NULL, NULL, NULL}
};




/* @filesection ajtax *******************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
******************************************************************************/





/* @datasection [AjPFile] Taxonomy data output ********************************
**
** Function is for manipulating taxonomy data objects
**
** @nam2rule Taxout Taxonomy data output
**
******************************************************************************/




/* @section taxonomy data outputs *********************************************
**
** These functions write the taxonomy data provided by the first argument
**
** @fdata [AjPFile]
**
** @nam3rule Write Write taxonomy data
** @nam4rule Format Use a named format
**
** @argrule Write outf [AjPOutfile] Output file
** @argrule Write tax [const AjPTax] taxonomy data
** @argrule Format fmt [const AjPStr] Format name
**
** @valrule * [AjBool] true on success
**
** @fcategory output
**
******************************************************************************/




/* @func ajTaxoutWrite ********************************************************
**
** Write a taxon in a named format
**
** @param [u] outf [AjPOutfile] Output file
** @param [r] tax [const AjPTax] Taxon object
**
** @return [AjBool] True on success
**
**
** @release 6.4.0
******************************************************************************/

AjBool ajTaxoutWrite(AjPOutfile outf, const AjPTax tax)
{
    ajuint i = ajOutfileGetFormatindex(outf);
    AjPFile outfile = ajOutfileGetFile(outf);

    return (*taxoutFormatDef[i].Write)(outfile, tax);
}





/* @funcstatic taxoutWriteEbi *************************************************
**
** Write taxonomy data in EBI taxonomy.dat format
**
** @param [u] outf [AjPFile] Output file
** @param [r] tax [const AjPTax] Taxonomy data object
**
** @return [AjBool] True on success
**
**
** @release 6.4.0
******************************************************************************/

static AjBool taxoutWriteEbi(AjPFile outf, const AjPTax tax)
{
    AjIList iter = NULL;
    AjPTaxname name = NULL;

    ajFmtPrintF(outf, "%-25s : %S\n", "ID", tax->Id);
    ajFmtPrintF(outf, "%-25s : %u\n", "PARENT ID", tax->Parent);
    ajFmtPrintF(outf, "%-25s : %S\n", "RANK", tax->Rank);
    ajFmtPrintF(outf, "%-25s : %u\n", "GC ID", (ajuint) tax->Gencode);
    if(tax->Mitocode)
        ajFmtPrintF(outf, "%-25s : %u\n", "MGC ID", (ajuint) tax->Mitocode);

    iter = ajListIterNewread(tax->Namelist);
    while(!ajListIterDone(iter))
    {
        name = (AjPTaxname) ajListIterGet(iter);
        if(ajStrGetLen(name->UniqueName))
           ajFmtPrintF(outf, "%-25US : %S\n", name->NameClass,
                       name->UniqueName);
        else
           ajFmtPrintF(outf, "%-25US : %S\n", name->NameClass, name->Name);
    }

    ajFmtPrintF(outf, "//\n");

    ajListIterDel(&iter);

    return ajTrue;
}




/* @funcstatic taxoutWriteExcel ***********************************************
**
** Write a taxon in an tab-delimited format
**
** @param [u] outf [AjPFile] Output file
** @param [r] tax [const AjPTax] Taxon object
**
** @return [AjBool] True on success
**
**
** @release 6.4.0
******************************************************************************/

static AjBool taxoutWriteExcel(AjPFile outf, const AjPTax tax)
{
    if(!outf) return ajFalse;
    if(!tax) return ajFalse;

    /*ajFmtPrintF(outf, "id\tnamespace\tname\n");*/
    ajFmtPrintF(outf, "%S\t", tax->Id);
    ajFmtPrintF(outf, "%u\t", tax->Parent);
    ajFmtPrintF(outf, "%S\t", tax->Rank);
    ajFmtPrintF(outf, "%b\t", tax->Flags & 8);
    ajFmtPrintF(outf, "%S\n", tax->Name);

    return ajTrue;
}




/* @funcstatic taxoutWriteNcbi ************************************************
**
** Write taxonomy data in NCBI taxonomy format
**
** @param [u] outf [AjPFile] Output file
** @param [r] tax [const AjPTax] Taxonomy data object
**
** @return [AjBool] True on success
**
**
** @release 6.4.0
******************************************************************************/

static AjBool taxoutWriteNcbi(AjPFile outf, const AjPTax tax)
{
    AjIList iter = NULL;
    AjPTaxname name = NULL;

    ajFmtPrintF(outf, "%S\t|", tax->Id);
    ajFmtPrintF(outf, "\t%u\t|", tax->Parent);
    ajFmtPrintF(outf, "\t%S\t|", tax->Rank);
    ajFmtPrintF(outf, "\t%S\t|", tax->Emblcode);
    ajFmtPrintF(outf, "\t%u\t|", (ajuint) tax->Divid);
    ajFmtPrintF(outf, "\t%u\t|", (tax->Flags && 1));
    ajFmtPrintF(outf, "\t%u\t|", (ajuint) tax->Gencode);
    ajFmtPrintF(outf, "\t%u\t|", (tax->Flags && 2));
    ajFmtPrintF(outf, "\t%u\t|", (ajuint) tax->Mitocode);
    ajFmtPrintF(outf, "\t%u\t|", (tax->Flags && 4));
    ajFmtPrintF(outf, "\t%u\t|", (tax->Flags && 8));
    ajFmtPrintF(outf, "\t%u\t|", (tax->Flags && 16));

    if(ajStrGetLen(tax->Comment))
       ajFmtPrintF(outf, "\t%S", tax->Comment);

    ajFmtPrintF(outf, "\n");

    iter = ajListIterNewread(tax->Namelist);
    while(!ajListIterDone(iter))
    {
        name = (AjPTaxname) ajListIterGet(iter);
        ajFmtPrintF(outf, "%S\t|", tax->Id);
        ajFmtPrintF(outf, "\t%S\t|", name->Name);
        ajFmtPrintF(outf, "\t%S\t|", name->UniqueName);
        ajFmtPrintF(outf, "\t%S\n", name->NameClass);
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @funcstatic taxoutWriteTax *************************************************
**
** Write taxonomy data in EMBOSS taxon format
**
** @param [u] outf [AjPFile] Output file
** @param [r] tax [const AjPTax] Taxonomy data object
**
** @return [AjBool] True on success
**
**
** @release 6.4.0
******************************************************************************/

static AjBool taxoutWriteTax(AjPFile outf, const AjPTax tax)
{
    AjIList iter = NULL;
    AjPTaxname name = NULL;

    ajFmtPrintF(outf, "id: %S\n", tax->Id);
    ajFmtPrintF(outf, "parent: %u\n", tax->Parent);
    ajFmtPrintF(outf, "rank: %S\n", tax->Rank);
    ajFmtPrintF(outf, "emblprefix: %S\n", tax->Emblcode);
    ajFmtPrintF(outf, "division: %u\n", (ajuint) tax->Divid);
    ajFmtPrintF(outf, "divflag: %u\n", (tax->Flags && 1));
    ajFmtPrintF(outf, "gencode: %u\n", (ajuint) tax->Gencode);
    ajFmtPrintF(outf, "gcflag: %u\n", (tax->Flags && 2));
    ajFmtPrintF(outf, "mitocode: %u\n", (ajuint) tax->Mitocode);
    ajFmtPrintF(outf, "mgcflag %u\n", (tax->Flags && 4));
    ajFmtPrintF(outf, "hidden: %u\n", (tax->Flags && 8));
    ajFmtPrintF(outf, "nosequence: %u\n", (tax->Flags && 16));

    if(ajStrGetLen(tax->Comment))
       ajFmtPrintF(outf, "comment: %S\n", tax->Comment);

    iter = ajListIterNewread(tax->Namelist);
    while(!ajListIterDone(iter))
    {
        name = (AjPTaxname) ajListIterGet(iter);
        if(ajStrGetLen(name->UniqueName))
            ajFmtPrintF(outf, "%S: %S\n", name->NameClass, name->UniqueName);
        else
            ajFmtPrintF(outf, "%S: %S\n", name->NameClass, name->Name);
    }

    ajFmtPrintF(outf, "\n");

    ajListIterDel(&iter);

    return ajTrue;
}




/* @datasection [none] Miscellaneous functions ********************************
**
** Functions to initialise and clean up internals
**
** @nam2rule Taxout Taxonomy output internals
**
******************************************************************************/




/* @section Printing **********************************************************
**
** Printing details of the internals to a file
**
** @fdata [none]
**
** @nam2rule Taxoutprint
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




/* @func ajTaxoutprintBook ****************************************************
**
** Reports the taxonomy format internals as Docbook text
**
** @param [u] outf [AjPFile] Output file
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTaxoutprintBook(AjPFile outf)
{
    ajint i = 0;

    ajFmtPrintF(outf, "<para>The supported taxonomy output "
                "formats are summarised "
                "in the table below. The columns are as follows: "
                "<emphasis>Output format</emphasis> (format name), "
                 "<emphasis>Description</emphasis> (short description of "
                "the format).</para> \n\n");

    ajFmtPrintF(outf, "<table frame=\"box\" rules=\"cols\">\n");
    ajFmtPrintF(outf, "  <caption>Taxonomy output formats</caption>\n");
    ajFmtPrintF(outf, "  <thead>\n");
    ajFmtPrintF(outf, "    <tr align=\"center\">\n");
    ajFmtPrintF(outf, "      <th>Output Format</th>\n");
    ajFmtPrintF(outf, "      <th>Description</th>\n");
    ajFmtPrintF(outf, "    </tr>\n");
    ajFmtPrintF(outf, "  </thead>\n");
    ajFmtPrintF(outf, "  <tbody>\n");

    for(i=0; taxoutFormatDef[i].Name; i++)
    {
        ajFmtPrintF(outf, "    <tr>\n");
        ajFmtPrintF(outf, "      <td>%s</td>\n",
                    taxoutFormatDef[i].Name);
        ajFmtPrintF(outf, "      <td>%s</td>\n",
                    taxoutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "  </tbody>\n");
    ajFmtPrintF(outf, "</table>\n");

    return;
}




/* @func ajTaxoutprintHtml ****************************************************
**
** Reports the internal data structures
**
** @param [u] outf [AjPFile] Output file
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTaxoutprintHtml(AjPFile outf)
{
    ajint i = 0;

    ajFmtPrintF(outf, "<table border=3>");
    ajFmtPrintF(outf, "<tr><th>Taxonomy Format</th>\n");
    ajFmtPrintF(outf, "<th>Description</th></tr>\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# taxonomy output formats\n");
    ajFmtPrintF(outf, "# Name    Format name (or alias)\n");
    ajFmtPrintF(outf, "# Desc    Format description\n");
    ajFmtPrintF(outf, "# Name         Description\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "TaxFormat {\n");

    for(i=0; taxoutFormatDef[i].Name; i++)
    {
        ajFmtPrintF(outf, "<tr><td>\n%-12s\n"
                        "<td>\"%s\"</td></tr>\n",
			taxoutFormatDef[i].Name,
			taxoutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajTaxoutprintText ****************************************************
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

void ajTaxoutprintText(AjPFile outf, AjBool full)
{
    ajint i = 0;

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# Taxonomy output formats\n");
    ajFmtPrintF(outf, "# Name    Format name (or alias)\n");
    ajFmtPrintF(outf, "# Desc    Format description\n");
    ajFmtPrintF(outf, "# Name         Description\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "TaxFormat {\n");

    for(i=0; taxoutFormatDef[i].Name; i++)
    {
	if(full)
	    ajFmtPrintF(outf, "  %-12s \"%s\"\n",
			taxoutFormatDef[i].Name,
			taxoutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajTaxoutprintWiki ****************************************************
**
** Reports the taxonomy output format internals as wikitext
**
** @param [u] outf [AjPFile] Output file
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTaxoutprintWiki(AjPFile outf)
{
    ajint i = 0;
    ajint j = 0;
    AjPStr namestr = NULL;

    ajFmtPrintF(outf, "{| class=\"wikitable sortable\" border=\"2\"\n");
    ajFmtPrintF(outf, "|-\n");
    ajFmtPrintF(outf, "!Format!!"
                "class=\"unsortable\"|Description\n");

    for(i=0; taxoutFormatDef[i].Name; i++)
    {
        ajFmtPrintF(outf, "|-\n");
        ajStrAssignC(&namestr, taxoutFormatDef[i].Name);

        for(j=i+1; taxoutFormatDef[j].Name; j++)
        {
            if(taxoutFormatDef[j].Write == taxoutFormatDef[i].Write)
            {
                ajFmtPrintAppS(&namestr, "<br>%s",
                               taxoutFormatDef[j].Name);
                ajWarn("Taxonomy output format '%s' same as '%s' "
                       "but not alias",
                       taxoutFormatDef[j].Name,
                       taxoutFormatDef[i].Name);
            }
        }
        ajFmtPrintF(outf, "|%S||%s\n",
		    namestr,
		    taxoutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "|}\n");

    ajStrDel(&namestr);

    return;
}




/* @datasection [none] Output formats *****************************************
**
** Output formats internals
**
** @nam2rule Taxoutformat Taxon output format specific
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




/* @func ajTaxoutformatFind ***************************************************
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

AjBool ajTaxoutformatFind(const AjPStr format, ajint* iformat)
{
    AjPStr tmpformat = NULL;
    ajuint i = 0;

    /* ajDebug("ajTaxoutformatFind '%S'\n", format); */
    if(!ajStrGetLen(format))
	return ajFalse;

    ajStrAssignS(&tmpformat, format);
    ajStrFmtLower(&tmpformat);

    for(i=0; taxoutFormatDef[i].Name; i++)
    {
	/* ajDebug("test %d '%s' \n", i, taxoutFormatDef[i].Name); */
	if(ajStrMatchCaseC(tmpformat, taxoutFormatDef[i].Name))
	{
	    *iformat = i;
	    ajStrDel(&tmpformat);
	    /* ajDebug("found '%s' at %d\n", taxoutFormatDef[i].Name, i); */
	    return ajTrue;
	}
    }

    ajErr("Unknown output format '%S'", format);

    ajStrDel(&tmpformat);

    *iformat = 0;

    return ajFalse;
}




/* @func ajTaxoutformatTest ***************************************************
**
** Tests whether a named taxon output format is known
**
** @param [r] format [const AjPStr] Format
** @return [AjBool] ajTrue if formats was accepted
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajTaxoutformatTest(const AjPStr format)
{
    ajint i;

    for (i=0; taxoutFormatDef[i].Name; i++)
	if(ajStrMatchCaseC(format, taxoutFormatDef[i].Name))
	    return ajTrue;

    return ajFalse;
}




/* @datasection [none] Miscellaneous functions ********************************
**
** Functions to initialise and clean up internals
**
** @nam2rule Taxout Taxon output internals
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




/* @func ajTaxoutExit *********************************************************
**
** Cleans up taxonomy output internal memory
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTaxoutExit(void)
{
    return;
}

/* @source ajrefseqwrite ******************************************************
**
** AJAX reference sequence writing functions
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.8 $
** @modified Oct 25 2010 pmr First AJAX version
** @modified $Date: 2011/10/19 14:52:21 $ by $Author: rice $
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

#include "ajrefseqwrite.h"
#include "ajfile.h"
#include "ajseq.h"
#include "ajseqwrite.h"


static AjBool refseqoutWriteEmbl(AjPFile outf, const AjPRefseq refseq);
static AjBool refseqoutWriteList(AjPFile outf, const AjPRefseq refseq);





/* @datastatic RefseqPOutFormat ************************************************
**
** Reference sequence output formats data structure
**
** @alias RefseqSoutFormat
** @alias RefseqOOutFormat
**
** @attr Name [const char*] Format name
** @attr Desc [const char*] Format description
** @attr Write [AjBool function] Output function, returns ajTrue on success
** @@
******************************************************************************/

typedef struct RefseqSOutFormat
{
    const char *Name;
    const char *Desc;
    AjBool (*Write) (AjPFile outf, const AjPRefseq refseq);
} RefseqOOutFormat;

#define RefseqPOutFormat RefseqOOutFormat*


static RefseqOOutFormat refseqoutFormatDef[] =
{
/* "Name",        "Description" */
/*     WriteFunction */

  {"embl",         "EMBL format",
       &refseqoutWriteEmbl},

  {"list",          "List of reference sequence ids",
       &refseqoutWriteList},

  {NULL, NULL, NULL}
};




/* @filesection ajrefseq *******************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
******************************************************************************/





/* @datasection [AjPOutfile] Reference sequence data output ********************
**
** Function is for manipulating reference sequence data objects
**
** @nam2rule Refseqout Reference sequence data output
**
******************************************************************************/




/* @section reference sequence data outputs ************************************
**
** These functions write the reference sequence data provided by the
** first argument
**
** @fdata [AjPOutfile]
**
** @nam3rule Write Write reference sequence data
** @nam4rule Format Use a named format
**
** @argrule Write outf [AjPOutfile] Output file
** @argrule Write refseq [const AjPRefseq] reference sequence data
** @argrule Format fmt [const AjPStr] Format name
**
** @valrule * [AjBool] true on success
**
** @fcategory output
**
******************************************************************************/




/* @func ajRefseqoutWrite ******************************************************
**
** Write refseq data in a named format
**
** @param [u] outf [AjPOutfile] Output file
** @param [r] refseq [const AjPRefseq] Refseq data object
**
** @return [AjBool] True on success
**
******************************************************************************/

AjBool ajRefseqoutWrite(AjPOutfile outf, const AjPRefseq refseq)
{
    ajuint i = ajOutfileGetFormatindex(outf);
    AjPFile outfile = ajOutfileGetFile(outf);

    return (*refseqoutFormatDef[i].Write)(outfile, refseq);
}





/* @funcstatic refseqoutWriteEmbl *********************************************
**
** Write reference sequence data in embl format
**
** @param [u] outf [AjPFile] Output file
** @param [r] refseq [const AjPRefseq] Reference sequence data object
**
** @return [AjBool] True on success
**
******************************************************************************/

static AjBool refseqoutWriteEmbl(AjPFile outf, const AjPRefseq refseq)
{
    AjPSeq seq = NULL;
    AjPSeqout seqout = NULL;

    if(!outf) return ajFalse;
    if(!refseq) return ajFalse;

    seq = ajSeqNewRefseq(refseq);
    seqout = ajSeqoutNewFile(outf);
    ajSeqoutSetFormatC(seqout, "embl");
    ajSeqoutWriteSeq(seqout, seq);

    ajSeqDel(&seq);
    ajSeqoutDel(&seqout);
    
    return ajTrue;
}




/* @funcstatic refseqoutWriteList *********************************************
**
** Write reference sequence object id to file
**
** @param [u] outf [AjPFile] Output reference sequence file
** @param [r] refseq [const AjPRefseq] Reference sequence object
** @return [AjBool] True on success
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool refseqoutWriteList(AjPFile outf, const AjPRefseq refseq)
{
    if(!outf)
        return ajFalse;
    if(!refseq)
        return ajFalse;

    if(ajStrGetLen(refseq->Db))
        ajFmtPrintF(outf, "%S:%S\n", refseq->Db, refseq->Id);
    else
        ajFmtPrintF(outf, "%S\n", refseq->Id);

    return ajTrue;
}




/* @datasection [none] Miscellaneous functions ********************************
**
** Functions to initialise and clean up internals
**
** @nam2rule Refseqout Reference sequence output internals
**
******************************************************************************/




/* @section Printing **********************************************************
**
** Printing details of the internals to a file
**
** @fdata [none]
**
** @nam2rule Refseqoutprint
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




/* @func ajRefseqoutprintBook **************************************************
**
** Reports the reference sequence format internals as Docbook text
**
** @param [u] outf [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

void ajRefseqoutprintBook(AjPFile outf)
{
    ajint i = 0;

    ajFmtPrintF(outf, "<para>The supported reference sequence output "
                "formats are summarised "
                "in the table below. The columns are as follows: "
                "<emphasis>Output format</emphasis> (format name), "
                 "<emphasis>Description</emphasis> (short description of "
                "the format).</para> \n\n");

    ajFmtPrintF(outf, "<table frame=\"box\" rules=\"cols\">\n");
    ajFmtPrintF(outf,
                "  <caption>Reference sequence output formats</caption>\n");
    ajFmtPrintF(outf, "  <thead>\n");
    ajFmtPrintF(outf, "    <tr align=\"center\">\n");
    ajFmtPrintF(outf, "      <th>Output Format</th>\n");
    ajFmtPrintF(outf, "      <th>Description</th>\n");
    ajFmtPrintF(outf, "    </tr>\n");
    ajFmtPrintF(outf, "  </thead>\n");
    ajFmtPrintF(outf, "  <tbody>\n");

    for(i=0; refseqoutFormatDef[i].Name; i++)
    {
        ajFmtPrintF(outf, "    <tr>\n");
        ajFmtPrintF(outf, "      <td>%s</td>\n",
                    refseqoutFormatDef[i].Name);
        ajFmtPrintF(outf, "      <td>%s</td>\n",
                    refseqoutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "  </tbody>\n");
    ajFmtPrintF(outf, "</table>\n");

    return;
}




/* @func ajRefseqoutprintHtml **************************************************
**
** Reports the internal data structures
**
** @param [u] outf [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

void ajRefseqoutprintHtml(AjPFile outf)
{
    ajint i = 0;

    ajFmtPrintF(outf, "<table border=3>");
    ajFmtPrintF(outf, "<tr><th>Reference sequence Format</th>\n");
    ajFmtPrintF(outf, "<th>Description</th></tr>\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# Reference sequence output formats\n");
    ajFmtPrintF(outf, "# Name    Format name (or alias)\n");
    ajFmtPrintF(outf, "# Desc    Format description\n");
    ajFmtPrintF(outf, "# Name         Description\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "RefseqFormat {\n");

    for(i=0; refseqoutFormatDef[i].Name; i++)
    {
        ajFmtPrintF(outf, "<tr><td>\n%-12s\n"
                        "<td>\"%s\"</td></tr>\n",
			refseqoutFormatDef[i].Name,
			refseqoutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajRefseqoutprintText **************************************************
**
** Reports the internal data structures
**
** @param [u] outf [AjPFile] Output file
** @param [r] full [AjBool] Full report (usually ajFalse)
** @return [void]
** @@
******************************************************************************/

void ajRefseqoutprintText(AjPFile outf, AjBool full)
{
    ajint i = 0;

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# Reference sequence output formats\n");
    ajFmtPrintF(outf, "# Name    Format name (or alias)\n");
    ajFmtPrintF(outf, "# Desc    Format description\n");
    ajFmtPrintF(outf, "# Name         Description\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "RefseqFormat {\n");

    for(i=0; refseqoutFormatDef[i].Name; i++)
    {
	if(full)
	    ajFmtPrintF(outf, "  %-12s \"%s\"\n",
			refseqoutFormatDef[i].Name,
			refseqoutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajRefseqoutprintWiki **************************************************
**
** Reports the reference sequence output format internals as wikitext
**
** @param [u] outf [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

void ajRefseqoutprintWiki(AjPFile outf)
{
    ajint i = 0;
    ajint j = 0;
    AjPStr namestr = NULL;

    ajFmtPrintF(outf, "{| class=\"wikitable sortable\" border=\"2\"\n");
    ajFmtPrintF(outf, "|-\n");
    ajFmtPrintF(outf, "!Format!!"
                "class=\"unsortable\"|Description\n");

    for(i=0; refseqoutFormatDef[i].Name; i++)
    {
        ajFmtPrintF(outf, "|-\n");
        ajStrAssignC(&namestr, refseqoutFormatDef[i].Name);

        for(j=i+1; refseqoutFormatDef[j].Name; j++)
        {
            if(refseqoutFormatDef[j].Write == refseqoutFormatDef[i].Write)
            {
                ajFmtPrintAppS(&namestr, "<br>%s",
                               refseqoutFormatDef[j].Name);
                ajWarn("Reference sequence output format '%s' same as '%s' "
                       "but not alias",
                       refseqoutFormatDef[j].Name,
                       refseqoutFormatDef[i].Name);
            }
        }
        ajFmtPrintF(outf, "|%S||%s\n",
		    namestr,
		    refseqoutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "|}\n");

    ajStrDel(&namestr);

    return;
}




/* @datasection [none] Output formats *****************************************
**
** Output formats internals
**
** @nam2rule Refseqoutformat Refseq output format specific
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




/* @func ajRefseqoutformatFind ************************************************
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

AjBool ajRefseqoutformatFind(const AjPStr format, ajint* iformat)
{
    AjPStr tmpformat = NULL;
    ajuint i = 0;

    /* ajDebug("ajRefseqoutformatFind '%S'\n", format); */
    if(!ajStrGetLen(format))
	return ajFalse;

    ajStrAssignS(&tmpformat, format);
    ajStrFmtLower(&tmpformat);

    for(i=0; refseqoutFormatDef[i].Name; i++)
    {
	/* ajDebug("test %d '%s' \n", i, refseqoutFormatDef[i].Name); */
	if(ajStrMatchCaseC(tmpformat, refseqoutFormatDef[i].Name))
	{
	    *iformat = i;
	    ajStrDel(&tmpformat);
	    /* ajDebug("found '%s' at %d\n", refseqoutFormatDef[i].Name, i); */
	    return ajTrue;
	}
    }

    ajErr("Unknown output format '%S'", format);

    ajStrDel(&tmpformat);

    *iformat = 0;

    return ajFalse;
}




/* @func ajRefseqoutformatTest ************************************************
**
** Tests whether a named refseq output format is known
**
** @param [r] format [const AjPStr] Format
** @return [AjBool] ajTrue if formats was accepted
** @@
******************************************************************************/

AjBool ajRefseqoutformatTest(const AjPStr format)
{
    ajint i;

    for (i=0; refseqoutFormatDef[i].Name; i++)
	if(ajStrMatchCaseC(format, refseqoutFormatDef[i].Name))
	    return ajTrue;

    return ajFalse;
}




/* @datasection [none] Miscellaneous functions ********************************
**
** Functions to initialise and clean up internals
**
** @nam2rule Refseqout Refseq output internals
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




/* @func ajRefseqoutExit ******************************************************
**
** Cleans up reference sequence output internal memory
**
** @return [void]
** @@
******************************************************************************/

void ajRefseqoutExit(void)
{
    return;
}

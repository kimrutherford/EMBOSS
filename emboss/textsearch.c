/* @source textsearch application
**
** Search sequence documentation text. SRS or Entrez is faster.
**
** @author Copyright (C) Gary Williams (gwilliam@hgmp.mrc.ac.uk)
** @@
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
******************************************************************************/

#include "emboss.h"




/* @prog textsearch ***********************************************************
**
** Search sequence documentation text. SRS and Entrez are faster!
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPSeqall seqall;
    AjPSeq seq;
    AjPStr pattern;
    AjBool html;
    AjBool doheader;
    AjBool casesensitive;
    AjBool doname;
    AjBool dodesc;
    AjBool dousa;
    AjBool doacc;

    AjPFile outfile;

    const AjPStr usa;
    const AjPStr name;
    const AjPStr acc;
    AjPStr altusa;
    AjPStr altname;
    AjPStr altacc;

    const AjPStr desc = NULL;

    AjPRegexp exp = NULL;


    embInit("textsearch", argc, argv);

    seqall        = ajAcdGetSeqall("sequence");
    pattern       = ajAcdGetString("pattern");
    outfile       = ajAcdGetOutfile("outfile");
    html          = ajAcdGetBoolean("html");
    doheader      = ajAcdGetBoolean("heading");
    casesensitive = ajAcdGetBoolean("casesensitive");
    dousa         = ajAcdGetBoolean("usa");
    doname        = ajAcdGetBoolean("name");
    doacc         = ajAcdGetBoolean("accession");
    dodesc        = ajAcdGetBoolean("description");


    altusa  = ajStrNewC("-");
    altname = ajStrNewC("-");
    altacc  = ajStrNewC("-     ");


    /* compile the regular expression with or without case-sensitivity */
    if(casesensitive)
	exp = ajRegComp(pattern);
    else
	exp = ajRegCompCase(pattern);


    /* start the HTML table */
    if(html)
	ajFmtPrintF(outfile, "<table border cellpadding=4 "
		    "bgcolor=\"#FFFFF0\">\n");


    /* print the header information */
    if(doheader)
    {
	if(html)
	    ajFmtPrintF(outfile, "<tr>Search for: %S</tr>\n", pattern);
	else
	    ajFmtPrintF(outfile, "# Search for: %S\n", pattern);
    }

    while(ajSeqallNext(seqall, &seq))
    {
	ajSeqTrace(seq);
	name = ajSeqGetNameS(seq);
	desc = ajSeqGetDescS(seq);

	if(ajStrGetLen(desc) && ajRegExec(exp, desc))
	{
	    /* get the usa ('-' if unknown) */
	    usa = ajSeqGetUsaS(seq);
	    if(ajStrGetLen(usa) == 0)
		usa = altusa;

	    /* get the name ('-' if unknown) */
	    name = ajSeqGetNameS(seq);
	    if(ajStrGetLen(name) == 0)
		name = altname;

	    /* get the accession number ('-' if unknown) */
	    acc = ajSeqGetAccS(seq);
	    if(ajStrGetLen(acc) == 0)
		acc = altacc;

	    /* start table line */
	    if(html)
		ajFmtPrintF(outfile, "<tr>");

	    if(dousa)
	    {
		if(html)
		    ajFmtPrintF(outfile, "<td>%S</td>", usa);
		else
		{
		    /*
		    **  Format:
		    **
		    **  If this is the last item, don't put spaces or TABs
		    **  after it. Try to fit the name in 18 spaces,
		    **  else just add a TAB after it
		    */
		    if(ajStrGetLen(usa) < 18)
		    {
			if(doname || doacc || dodesc)
			    ajFmtPrintF(outfile, "%-18.17S", usa);
			else
			    ajFmtPrintF(outfile, "%S", usa);
		    }
		    else
		    {
			ajFmtPrintF(outfile, "%S", usa);
			if(doname || doacc || dodesc)
			    ajFmtPrintF(outfile, "\t");
		    }
		}
	    }

	    if(doname)
	    {
		if(html)
		    ajFmtPrintF(outfile, "<td>%S</td>", name);
		else
		{
		    /*
		    ** Make the formatting nice:
		    **
		    **  If this is the last item, don't put spaces or TABs
		    **  after it. Try to fit the name in 14 space, else
		    **  just add a TAB after it
		    */
		    if(ajStrGetLen(name) < 14)
		    {
			if(doacc || dodesc)
			    ajFmtPrintF(outfile, "%-14.13S", name);
			else
			    ajFmtPrintF(outfile, "%S", name);
		    }
		    else
		    {
			ajFmtPrintF(outfile, "%S", name);

			if(doacc || dodesc)
			    ajFmtPrintF(outfile, "\t");
		    }
		}
	    }


	    if(doacc)
	    {
		if(html)
		    ajFmtPrintF(outfile, "<td>%S</td>", acc);
		else
		    ajFmtPrintF(outfile, "%S", acc);
		if(dodesc)
		    ajFmtPrintF(outfile, "\t");
	    }


	    if(dodesc)
	    {
		if(html)
		    ajFmtPrintF(outfile, "<td>%S</td>", desc);
		else
		    ajFmtPrintF(outfile, "%S", desc);
	    }

	    /* end table line */
	    if(html)
		ajFmtPrintF(outfile, "</tr>\n");
	    else
		ajFmtPrintF(outfile, "\n");
	}

    }


    /* end the HTML table */
    if(html)
	ajFmtPrintF(outfile, "</table>\n");

    ajFileClose(&outfile);

    ajStrDel(&altusa);
    ajStrDel(&altname);
    ajStrDel(&altacc);
    ajRegFree(&exp);
    ajSeqallDel(&seqall);
    ajSeqDel(&seq);
    ajStrDel(&pattern);

    embExit();

    return 0;
}

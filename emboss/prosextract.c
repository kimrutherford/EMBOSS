/* @source prosextract.c
** @author Copyright (C) Sinead O'Leary (soleary@hgmp.mrc.ac.uk)
**
** Application for extracting relevent lines from the Prosite motif database.
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

#define DATANAME "PROSITE/prosite.lines"




/* @prog prosextract **********************************************************
**
** Builds the PROSITE motif database for patmatmotifs to search
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPFile infdat = NULL;
    AjPFile infdoc = NULL;
    AjPFile outf   = NULL;
    AjPFile outs   = NULL;

    AjBool  haspattern;

    const char   *p;


    AjPStr line  = NULL;
    AjPStr text  = NULL;
    AjPStr dirname  = NULL;
    AjPStr filename = NULL;
    AjPStr id    = NULL;
    AjPStr ac    = NULL;
    AjPStr de    = NULL;
    AjPStr pa    = NULL;
    AjPStr ps    = NULL;
    AjPStr fn    = NULL;
    AjPStr re    = NULL;
    AjPStr fname = NULL;
    AjBool flag;
    AjBool isopen;
    AjBool goback;

    ajlong storepos = 0L;


    embInit("prosextract", argc, argv);

    dirname = ajAcdGetDirectoryName("prositedir");

    line = ajStrNew();
    text = ajStrNew();

    id = ajStrNew();
    ac = ajStrNew();
    de = ajStrNew();
    pa = ajStrNew();
    ps = ajStrNew();



    fn=ajStrNew();
    ajStrAssignS(&fn,dirname);
    ajStrAppendC(&fn,"prosite.dat");
    if(!(infdat=ajFileNewIn(fn)))
	ajFatal("Cannot open file %S",fn);
    ajStrDel(&fn);



    fn=ajStrNewC("PROSITE/prosite.lines");
    ajFileDataNewWrite(fn,&outf);
    ajStrDel(&fn);



    haspattern = ajFalse;

    while(ajFileReadLine(infdat, &line) )
    {
	if(ajStrPrefixC(line, "ID"))
	{
	    if(ajStrSuffixC(line,"PATTERN."))
	    {
		haspattern = ajTrue;
		/*save id*/
		p = ajStrGetPtr(line);
		p = ajSysFuncStrtok(p," \t;");
		p = ajSysFuncStrtok(NULL," \t;");
		ajStrAssignC(&id,p);
		ajFmtPrintF(outf, "%S ", id);
		continue;
	    }
	    else
	    {
		haspattern = ajFalse;
		continue;
	    }
	}

	if(!haspattern)
	    continue;


	if(ajStrPrefixC(line, "AC") )
	{
	    p = ajStrGetPtr(line);
	    p = ajSysFuncStrtok(p, " \t;");
	    p = ajSysFuncStrtok(NULL, " \t;");
	    ajStrAssignC(&ac,p);
	    ajFmtPrintF(outf, "%S\n ", ac);
	    continue;
	}

    	if(ajStrPrefixC(line, "DE") )
	{
	    p = ajStrGetPtr(line);
	    p = ajSysFuncStrtok(p, " \t.");
	    p = ajSysFuncStrtok(NULL, " \t.");
	    ajStrAssignC(&de,p);
	    ajFmtPrintF(outf, "%S\n ", de);
	    continue;
	}


	if(ajStrPrefixC(line, "PA"))
	{
	    ajStrAssignC(&pa,"");

	    while(ajStrPrefixC(line,"PA"))
	    {
		p = ajStrGetPtr(line);
		p = ajSysFuncStrtok(p, " \t.");
		p = ajSysFuncStrtok(NULL, " \t.");
		ajStrAppendC(&pa,p);
		ajFileReadLine(infdat, &line);
	    }

	    ajFmtPrintF(outf, "%S\n", pa);
	    re = embPatPrositeToRegExp(pa);
	    ajFmtPrintF(outf, "^%S\n\n", re);
	    ajStrDel(&re);
	    continue;
	}
    }


  /* Finished processing prosite.dat so look at prosite.doc */


    fn = ajStrNew();
    ajStrAssignS(&fn,dirname);
    ajStrAppendC(&fn,"prosite.doc");
    if(!(infdoc=ajFileNewIn(fn)))
	ajFatal("Cannot open file %S",fn);
    ajStrDel(&fn);



    fname  = ajStrNewC("PROSITE/");
    flag   = ajFalse;
    isopen = ajFalse;
    goback = ajFalse;


    while(ajFileReadLine(infdoc, &text))
    {
	if(ajStrPrefixC(text, "{PS") && isopen && !goback)
	    goback = ajTrue;

	if(ajStrPrefixC(text, "{PS") && !isopen)
	{
	    storepos = ajFileTell(infdoc);
	    /* save out the documentation text to acc numbered outfiles . */
	    p = ajStrGetPtr(text)+1;
	    p = ajSysFuncStrtok(p, ";");
	    ajStrAssignS(&filename, fname);
	    ajStrAppendC(&filename, p);

	    ajFileDataNewWrite(filename, &outs);
	    flag   = ajTrue;
	    isopen = ajTrue;
	    continue;
	}


	if(ajStrPrefixC(text, "{BEGIN}") && flag)
	{
	    while(ajFileReadLine(infdoc, &text))
	    {
		if(ajStrPrefixC(text,"{END}"))
		    break;

		ajFmtPrintF(outs, "%S\n", text);
	    }
	    ajFileClose(&outs);
	    isopen = ajFalse;

	    if(goback)
	    {
		goback = ajFalse;
		ajFileSeek(infdoc,storepos,0);
	    }

	}
    }

    ajStrDel(&line);
    ajStrDel(&text);
    ajStrDel(&dirname);
    ajStrDel(&filename);

    ajStrDel(&id);
    ajStrDel(&ac);
    ajStrDel(&de);
    ajStrDel(&pa);
    ajStrDel(&re);
    ajStrDel(&ps);
    ajStrDel(&fname);


    ajFileClose(&infdat);
    ajFileClose(&infdoc);
    ajFileClose(&outf);

    embExit();

    return 0;
}

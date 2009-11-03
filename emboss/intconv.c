/* @source intconv application
**
** Convert ints to ajints and longs to ajlongs
**
** @author Copyright (C) Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
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
#include <string.h>




static void intconv_modLineInt(AjPStr *line, ajint pos);
static void intconv_modLineLong(AjPStr *line, ajint pos);




/* @prog intconv **************************************************************
**
** int/ajint long/ajlong conversion
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPFile inf  = NULL;
    AjPFile outf = NULL;
    AjPStr  line = NULL;
    ajint pos;
    const char *p;
    const char *q;
    char c;
    char d = '\0';

    embInit("intconv", argc, argv);

    inf  = ajAcdGetInfile("infile");
    outf = ajAcdGetOutfile("outfile");

    line = ajStrNew();

    while(ajReadlineTrim(inf, &line))
    {
	if(ajStrFindC(line,"int") == -1 && ajStrFindC(line,"long") == -1)
	{
	    ajFmtPrintF(outf,"%S\n",line);
	    continue;
	}

	pos = 0;
	p   = ajStrGetPtr(line);

	while(strstr(p+pos,"int"))
	{
	    if((q=strstr(p+pos,"[int]")))
	    {
		pos = q-p;
		intconv_modLineInt(&line,pos+1);
		p = ajStrGetPtr(line);
		pos += 5;
	    }
	    else if((q=strstr(p+pos,"(int)")))
	    {
		pos = q-p;
		intconv_modLineInt(&line,pos+1);
		p = ajStrGetPtr(line);
		pos += 5;
	    }
	    else if((q=strstr(p+pos,"int")))
	    {
		pos = q-p;
		c = *(q+3);
		if(!pos && (c==' ' || c=='\t' || c=='\n' || c=='\r' || c=='*'))
		{
		    intconv_modLineInt(&line,pos);
		    p = ajStrGetPtr(line);
		}

		if(pos)
		    d = *(q-1);

		if(pos && (d==' ' || d=='\t' || d=='\r' || d=='(') &&
		   (c==' ' || c=='\t' || c=='\n' || c=='\r' || c=='*'))
		{
		    intconv_modLineInt(&line,pos);
		    p = ajStrGetPtr(line);
		}

		pos += 3;
	    }
	}



	pos = 0;
	p   = ajStrGetPtr(line);

	while(strstr(p+pos,"long"))
	{
	    if((q=strstr(p+pos,"[long]")))
	    {
		pos = q-p;
		intconv_modLineLong(&line,pos+1);
		p = ajStrGetPtr(line);
		pos += 6;
	    }
	    else if((q=strstr(p+pos,"(long)")))
	    {
		pos = q-p;
		intconv_modLineLong(&line,pos+1);
		p = ajStrGetPtr(line);
		pos += 6;
	    }
	    else if((q=strstr(p+pos,"long")))
	    {
		pos = q-p;
		c = *(q+4);
		if(!pos && (c==' ' || c=='\t' || c=='\n' || c=='\r' || c=='*'))
		{
		    intconv_modLineLong(&line,pos);
		    p = ajStrGetPtr(line);
		}

		if(pos)
		    d = *(q-1);

		if(pos && (d==' ' || d=='\t' || d=='\r' || d=='(') &&
		   (c==' ' || c=='\t' || c=='\n' || c=='\r' || c=='*'))
		{
		    intconv_modLineLong(&line,pos);
		    p = ajStrGetPtr(line);
		}

		pos += 4;
	    }
	}

	ajFmtPrintF(outf,"%S\n",line);
    }

    ajStrDel(&line);
    ajFileClose(&inf);
    ajFileClose(&outf);

    embExit();

    return 0;
}




/* @funcstatic intconv_modLineInt *********************************************
**
** Undocumented.
**
** @param [w] line [AjPStr*] Undocumented
** @param [r] pos [ajint] Undocumented
** @@
******************************************************************************/

static void intconv_modLineInt(AjPStr *line, ajint pos)
{
    AjPStr t;
    const char *p;

    t = ajStrNew();
    p = ajStrGetPtr(*line);

    if(pos)
	ajStrAssignSubC(&t,p,0,pos-1);
    else
	ajStrAssignC(&t,"");

    ajStrAppendC(&t,"ajint");
    p += (pos+3);
    ajStrAppendC(&t,p);

    ajStrAssignC(line,ajStrGetPtr(t));

    ajStrDel(&t);

    return;
}




/* @funcstatic intconv_modLineLong ********************************************
**
** Undocumented.
**
** @param [w] line [AjPStr*] Undocumented
** @param [r] pos [ajint] Undocumented
** @@
******************************************************************************/

static void intconv_modLineLong(AjPStr *line, ajint pos)
{
    AjPStr t;
    const char   *p;

    t = ajStrNew();
    p = ajStrGetPtr(*line);


    if(pos)
	ajStrAssignSubC(&t,p,0,pos-1);
    else
	ajStrAssignC(&t,"");

    ajStrAppendC(&t,"ajlong");
    p += (pos+4);
    ajStrAppendC(&t,p);

    ajStrAssignC(line,ajStrGetPtr(t));

    ajStrDel(&t);

    return;
}


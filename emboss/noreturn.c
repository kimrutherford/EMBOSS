/* @source noreturn application
**
** Remove trailing carriage returns from otherwise ASCII files
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




/* @prog noreturn *************************************************************
**
** Removes carriage return from ASCII files
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPFile inf  = NULL;
    AjPFile outf = NULL;
    AjPStr  line = NULL;
    ajint   len;
    ajint i;
    char    *p;
    ajint iline=0;

    AjPStr osystem = NULL;
    AjBool pcoutput = ajFalse;
    AjBool macoutput = ajFalse;

    embInit("noreturn", argc, argv);

    inf  = ajAcdGetInfile("infile");
    outf = ajAcdGetOutfile("outfile");
    osystem = ajAcdGetListSingle("system");
    if(ajStrMatchC(osystem, "pc"))
	pcoutput = ajTrue;
    if(ajStrMatchC(osystem, "mac"))
	macoutput = ajTrue;

    line = ajStrNew();

    ajDebug("Characters \\n %%%02x \\r %%%02x\n", '\n', '\r');
    while(ajFileReadLine(inf,&line))
    {
	iline++;
	p   = MAJSTRGETPTR(line);
	len = ajStrGetLen(line);
	if(len)
	{
	    ajDebug("line %d: Length %d\n", iline, len);
	    if(p[len-1] == 0x0a)
	    {
		ajDebug("line %d: newline at end removed\n", iline);
		p[len-1] = 0x00;
		len--;
		if(len && p[len-1] == 0x0d)
		{
		    ajDebug("line %d: return at end removed\n", iline);
		    p[len-1] = 0x00;
		}
	    }

	    /* for files with 0d but no newline, (e.g. from Macintosh sources)
	    ** there's everything in one line
	    ** so replace the rest directly
	    */

	    if(!macoutput)
	    {
		for(i=0;i<len;i++)
		{
		    if(p[i] ==  0x0d)
		    {
			p[i] = 0x0a;
			ajDebug("line %d: return inserted at position %d\n",
				iline, i);
		    }
		}
	    }
	}
	
	if (pcoutput)
	    ajFmtPrintF(outf,"%s\r\n",p);
	else if (macoutput)
	    ajFmtPrintF(outf,"%s\r",p);
	else
	    ajFmtPrintF(outf,"%s\n",p);
    }
    
    ajStrDel(&line);
    ajStrDel(&osystem);
    ajFileClose(&inf);
    ajFileClose(&outf);

    embExit();

    return 0;
}

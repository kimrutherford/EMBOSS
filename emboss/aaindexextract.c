/* @source aaindexextract application
**
** Extracts amino acid indices from AAINDEX
** @author Copyright (C) Peter Rice (peter.rice@uk.lionbioscience.com)
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
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>




/* @prog aaindexextract *******************************************************
**
** Extract data from AAINDEX.
**
** Each amino acid index is written as a separate file to the standard
** EMBOSS data directory.
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPFile inf   = NULL;
    AjPFile outf  = NULL;

    AjPStr  line;
    AjPStr  outdir   = NULL;
    AjPStr  outfname = NULL;
    AjPStr  id       = NULL;
    AjBool  done;

    AjPRegexp idexp  = NULL;
    AjPRegexp endexp = NULL;

    idexp = ajRegCompC("^H ([^ \n\r\t]+)$");
    endexp = ajRegCompC("^//$");
    ajStrAssignC(&outdir, "AAINDEX");

    embInit("aaindexextract",argc,argv);

    inf = ajAcdGetInfile("infile");


    line = ajStrNew();

    done = ajTrue;
    while(ajFileGetsTrim(inf, &line))
    {
	/* process the ID for a new index */
	if(ajRegExec(idexp, line))
	{
	    if(!done)
		ajFatal("bad aaindex1 format new ID at: %S", line);
	    ajRegSubI(idexp, 1, &id);
	    ajStrFmtLower(&id);
	    ajFmtPrintS(&outfname, "%S/%S", outdir, id);
	    ajFileDataNewWrite(outfname,&outf);
	    done = ajFalse;
	}
	else
	{
	    if(done)
		ajFatal("bad aaindex1 format expected ID at: %S", line);
	    done = ajFalse;
	}

	/* write the current line */
	ajFmtPrintF(outf, "%S\n", line);

	/* close the file at end of index */
	if(ajRegExec(endexp, line))
	{
	    done = ajTrue;
	    ajFileClose (&outf);
	}
    }
    ajFileClose (&inf);

    ajStrDel(&id);
    ajStrDel(&line);
    ajStrDel(&outdir);
    ajStrDel(&outfname);

    ajRegFree(&idexp);
    ajRegFree(&endexp);

    embExit();

    return 0;
}

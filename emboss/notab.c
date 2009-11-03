/* @source notab application
**
** Replace tabs with spaces in an ASCII text file
** @author Copyright (C) Jon Ison (jison@ebi.ac.uk)
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

/* @prog notab ****************************************************************
**
** Replace tabs with spaces in an ASCII text file
**
******************************************************************************/

int main(int argc, char **argv)
{
    /* Variable Declarations */
    AjPFile inf     = NULL;
    AjPFile outf    = NULL;

    AjPStr  line    = NULL;  /* Line from inf    */
    AjPStr  outline    = NULL;  /* Line to write    */
    AjPStr tmpstr = NULL;
    
    AjPRegexp tabexp = NULL;
    ajuint ilen;
    ajuint irest;

    /* ACD File Processing */
    embInit("notab", argc, argv);
    inf  = ajAcdGetInfile("infile");
    outf = ajAcdGetOutfile("outfile");
    tabexp = ajRegCompC("\t");

    /* Application logic */
    line = ajStrNew();
    while(ajReadline(inf,&line))
    {
        ilen = 0;
        ajStrAssignClear(&outline);
        while(ajRegExec(tabexp, line))
        {
            ajRegPre(tabexp, &tmpstr);
            ajStrAppendS(&outline, tmpstr);
            ilen += ajStrGetLen(tmpstr);
            irest = 8 - (ilen - 8*(ilen/8));
            ajStrAppendCountK(&outline, ' ', irest);
            ilen += irest;
            ajRegPost(tabexp, &tmpstr);
            ajStrAssignS(&line, tmpstr);
            ajDebug("ilen: %u irest: %u\n'%S'\n",
                   ilen, irest, outline);
        }
        ajStrAppendS(&outline, line);
        
	ajFmtPrintF(outf, "%S", outline);
    }

    /* Memory management and exit */
    ajFileClose(&inf);
    ajFileClose(&outf);

    ajStrDel(&line);

    embExit();

    return 0;
}


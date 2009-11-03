/* @source nospace application
**
** Remove extraneous whitespace from an ASCII text file
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

/* @prog trimspace ************************************************************
**
** Remove extra whitespace from an ASCII text file
**
******************************************************************************/

int main(int argc, char **argv)
{
    /* Variable Declarations */
    AjPFile   inf     = NULL;
    AjPFile   outf    = NULL; 

    AjPStr    line    = NULL;  /* Line from inf     */


    /* ACD File Processing */
    embInit("trimspace", argc, argv);
    inf  = ajAcdGetInfile("infile");
    outf = ajAcdGetOutfile("outfile");


    /* Application logic */
    line    = ajStrNew();

    while(ajReadline(inf,&line))
      {
	ajStrRemoveWhiteExcess(&line);
	ajFmtPrintF(outf, "%S\n", line);
      }

    /* Memory management and exit */
    ajFileClose(&inf);
    ajFileClose(&outf);

    ajStrDel(&line);

    embExit();

    return 0;
}


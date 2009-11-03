/* @source infobase application
**
** Return information on a given nucleotide base
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

/* @prog infobase *************************************************************
**
** Return information on nucleotide bases
**
******************************************************************************/



int main(int argc, char **argv)
{
    /* Variable Declarations */
    AjPFile outf;
    AjPStr code = NULL;
    char    code1;
    ajuint i;
    ajuint iend;

    /* ACD File Processing */
    embInit("infobase", argc, argv);
    code = ajAcdGetString("code");
    outf = ajAcdGetOutfile("outfile");


    /* Application logic */

    ajStrFmtUpper(&code);
    iend = ajStrGetLen(code);
    ajFmtPrintF(outf, "%4s %-10s %-10s %s\n",
                "Code", "Ambiguity", "Complement", "Mnemonic");
    for(i=0;i<iend;i++)
    {
        code1=ajStrGetCharPos(code,i);
        if(ajBaseExistsChar(code1))
        {
            ajFmtPrintF(outf, "%-4c %-10S %-10c %S\n",
                        code1, ajBaseGetCodes(code1),
                        ajBaseAlphacharComp(code1),
                        ajBaseGetMnemonic(code1));
        }
        else
        {
            ajFmtPrintF(outf, "%-4c %-10s %-10c %s\n",
                        code1, ".",
                        '.',
                        "invalid");

        }
        
    }
    
    

    /* Memory management and exit */
    ajStrDel(&code);
    ajFileClose(&outf);

    embExit();

    return 0;
}


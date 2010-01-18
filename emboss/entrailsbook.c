/* @source entrails application
**
** Show EMBOSS internals
**
** @author Copyright (C) Peter Rice
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




/* @prog entrailsbook *********************************************************
**
** Reports internal data structures in EMBOSS book format
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPFile outf;
    AjBool doall = ajFalse;
    AjPStr *tables = NULL;
    ajuint i;

    embInit("entrailsbook", argc, argv);

    outf = ajAcdGetOutfile("outfile");
    tables = ajAcdGetList("tables");

    for(i=0;tables[i];i++)
    {
        if(ajStrMatchC(tables[i], "all"))
            doall = ajTrue;

        if(doall || ajStrMatchC(tables[i], "inseq"))
            ajSeqPrintbookInFormat(outf);

        if(doall || ajStrMatchC(tables[i], "outseq"))
            ajSeqoutPrintbookFormat(outf);

        if(doall || ajStrMatchC(tables[i], "infeat"))
            ajFeatPrintbookFormat(outf);

        if(doall || ajStrMatchC(tables[i], "outfeat"))
            ajFeatoutPrintbookFormat(outf);

        if(doall || ajStrMatchC(tables[i], "report"))
            ajReportPrintbookFormat(outf);

        if(doall || ajStrMatchC(tables[i], "align"))
            ajAlignPrintbookFormat(outf);
    }
    
    ajFileClose(&outf);

    embExit();

    return 0;
}

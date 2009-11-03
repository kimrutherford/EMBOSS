/* @source yank application
**
** Return a USA for a sequence range
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




/* @prog yank *****************************************************************
**
** Reads a sequence range, appends the full USA to a list file
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeq seq;
    AjPFile outf;

    embInit("yank", argc, argv);

    seq  = ajAcdGetSeq("sequence");
    outf = ajAcdGetOutfile("outfile");

    ajFmtPrintF(outf, "%S\n", ajSeqGetUsaS(seq));
    ajFileClose(&outf);

    ajSeqDel(&seq);
    ajFileClose(&outf);

    embExit();

    return 0;
}

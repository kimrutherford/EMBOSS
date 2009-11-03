/* @source geecee application
**
** Get GC content
**
** @author Copyright (C) Richard Bruskievich
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




/* @prog geecee ***************************************************************
**
** Calculates the fractional GC content of nucleic acid sequences
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPSeq seq;
    AjPSeqall seqall;
    const AjPStr seqstr;
    AjPFile outf;
    ajint len;
    float pgc;

    embInit("geecee", argc, argv);

    seqall = ajAcdGetSeqall("sequence");
    outf   = ajAcdGetOutfile("outfile");

    ajFmtPrintF(outf, "#Sequence   GC content\n");
    while(ajSeqallNext(seqall, &seq))
    {
	ajSeqTrim(seq);
	seqstr = ajSeqGetSeqS(seq);
	len    = ajSeqGetLen(seq);
	pgc    = ajMeltGC(seqstr,len); /* forward strand for now... */

	ajFmtPrintF(outf, "%-12s %5.2f\n", ajSeqGetNameC(seq), pgc);
    }

    ajSeqallDel(&seqall);
    ajSeqDel(&seq);

    ajFileClose(&outf);

    embExit();

    return 0;
}

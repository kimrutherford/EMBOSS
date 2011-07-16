/* @source seqxref application
**
** Return a sequence
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




/* @prog seqxref **************************************************************
**
** Reads sequences and reports all their cross-references
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPFile outf;
    AjPSeq seq = NULL;
    AjPList xrefs = NULL;
    ajuint nrefs;
    AjPSeqXref xref = NULL;

    embInit("seqxref", argc, argv);

    outf = ajAcdGetOutfile("outfile");
    seqall = ajAcdGetSeqall("sequence");

    xrefs = ajListNew();

    while(ajSeqallNext(seqall, &seq))
    {
        nrefs = ajSeqGetXrefs(seq, xrefs);
        ajSeqxreflistSort(xrefs);

        ajFmtPrintF(outf, "#%S: %u\n", ajSeqGetUsaS(seq), nrefs);

        while(ajListPop(xrefs, (void**)&xref))
        {
            ajFmtPrintF(outf, "%S:%S\n", xref->Db, xref->Id);
            ajSeqxrefDel(&xref);
        }
    }

    ajListFree(&xrefs);
    ajSeqDel(&seq);
    
    ajFileClose(&outf);
    ajSeqallDel(&seqall);

    embExit();

    return 0;
}


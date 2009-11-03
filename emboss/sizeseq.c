/* @source sizeseq application
**
** Filter input sequences on basis of size
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


/* @prog sizeseq **************************************************************
**
** Filter input sequences on basis of size
**
******************************************************************************/

int main(int argc, char **argv)
{
    /* Variable Declarations */
    AjPSeqset seqset  = NULL;
    AjPSeqout seqout  = NULL;
    AjBool    bigfirst;
    ajuint nseqs;
    ajuint i;

    /* ACD File Processing */
    embInit("sizeseq", argc, argv);
    seqset      = ajAcdGetSeqset("sequences");
    bigfirst    = ajAcdGetBoolean("descending");
    seqout      = ajAcdGetSeqoutall("outseq");

    /* Application logic */
    ajSeqsetSortLen(seqset);
    nseqs = ajSeqsetGetSize(seqset);
    
    if(bigfirst)
    {
        for(i=nseqs; i>0; i--)
            ajSeqoutWriteSeq(seqout, ajSeqsetGetseqSeq(seqset,i-1));
    }
    else
    {
        for(i=0; i<nseqs; i++)
            ajSeqoutWriteSeq(seqout, ajSeqsetGetseqSeq(seqset,i));
    }
    

    /* Memory management and exit */
    ajSeqsetDel(&seqset);
    ajSeqoutClose(seqout);
    ajSeqoutDel(&seqout);

    embExit();

    return 0;
}

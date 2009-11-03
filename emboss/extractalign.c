/* @source extractalign application
**
** Extract regions from a sequence alignment
**
** @author Copyright (C) Gary Williams (gwilliam@hgmp.mrc.ac.uk)
** Fri Apr 16 16:47:32 BST 1999 (ajb)
** 7 Sept 1999 - GWW rewrote to use ajRange routines.
** 15 March 2000 - GWW added '-separate' option
** 22 May 2002 - GWW changed to only read one sequence, not a seqall
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




/* @prog extractalign ********************************************************
**
** Extract regions from a sequence
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqset seqset;
    const AjPSeq seq = NULL;
    AjPSeqout seqout;
    AjPRange regions;
    AjPStr newstr = NULL;
    ajint i;
    ajint numseqs;
    AjPSeq newseq = NULL;

    embInit("extractalign", argc, argv);

    seqset   = ajAcdGetSeqset("sequence");
    regions  = ajAcdGetRange("regions");
    seqout   = ajAcdGetSeqoutall("outseq");

    numseqs = ajSeqsetGetSize(seqset);
    for(i=0;i<numseqs;i++)
    {
      /*
      **  concatenate all regions from the sequence into the same
      **  sequence
      */
      seq = ajSeqsetGetseqSeq(seqset, i);
      newseq = ajSeqNewSeq(seq);
      ajRangeStrExtract(regions, ajSeqGetSeqS(newseq), &newstr);
      ajSeqAssignSeqS(newseq, newstr);
      ajStrSetClear(&newstr);
      ajSeqoutWriteSeq(seqout, newseq);
      ajSeqDel(&newseq);
      
    }
    ajSeqoutClose(seqout);

    ajSeqoutDel(&seqout);

    ajRangeDel(&regions);
    ajStrDel(&newstr);

    ajSeqsetDel(&seqset);
    ajSeqDel(&newseq);

    embExit();

    return 0;
}

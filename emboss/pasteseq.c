/* @source pasteseq application
**
** Insert one sequence into another
**
** @author Copyright (C) Gary Williams (gwilliam@hgmp.mrc.ac.uk)
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




/* @prog pasteseq *************************************************************
**
** Insert one sequence into another
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeq seq = NULL;
    AjPSeq insertseq = NULL;
    AjPSeqout seqout = NULL;
    /* position to insert after - can be 0 to sequence.length */
    ajint pos  = 0;
    AjPStr str = NULL;

    AjPSeq newseq = NULL;			/* the new sequence */


    embInit("pasteseq", argc, argv);

    seq       = ajAcdGetSeq("asequence");
    insertseq = ajAcdGetSeq("bsequence");
    pos       = ajAcdGetInt("pos");
    seqout    = ajAcdGetSeqout("outseq");

    /* trim to -sbegin and -send positions */
    ajSeqTrim(seq);
    ajSeqTrim(insertseq);

    /*
    **  create the new sequence - set the size to the size of the two input
    **  sequences plus 1 for the terminating NULL
    */
    newseq = ajSeqNewRes(ajSeqGetLen(seq) + ajSeqGetLen(insertseq) + 1);

    /* create a name for the sequence */
    ajSeqAssignNameS(newseq, ajSeqGetNameS(seq));

    /* set the description of the sequence */
    ajSeqAssignDescS(newseq, ajSeqGetDescS(seq));

    /* do the insertion here */
    str = ajSeqGetSeqCopyS(seq);
    ajStrInsertS(&str, pos, ajSeqGetSeqS(insertseq));
    ajSeqAssignSeqS(newseq, str);

    /* set the type of the output sequence (is this required?) */
    ajSeqType(newseq);

    /* output the sequence */
    ajSeqoutWriteSeq(seqout, newseq);
    ajSeqoutClose(seqout);

    ajSeqDel(&seq);
    ajSeqDel(&insertseq);
    ajSeqDel(&newseq);
    ajSeqoutDel(&seqout);
    ajStrDel(&str);

    embExit();

    return 0;
}

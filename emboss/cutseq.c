/* @source cutseq application
**
** Removes a specified section from a sequence
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




/* @prog cutseq ***************************************************************
**
** Removes a specified section from a sequence
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeq seq;
    AjPSeqout seqout;
    ajint from;
    ajint to;
    AjPStr str = NULL;
    ajint beg;
    ajint end;

    embInit("cutseq", argc, argv);

    seq    = ajAcdGetSeq("sequence");
    from   = ajAcdGetInt("from")-1;
    to     = ajAcdGetInt("to")-1;
    seqout = ajAcdGetSeqout("outseq");

    beg = ajSeqGetBegin(seq)-1;
    end = ajSeqGetEnd(seq)-1;

    str = ajStrNew();

    /* get a COPY of the sequence string */
    ajStrAssignSubS(&str, ajSeqGetSeqS(seq), beg, end);
    ajStrCutRange(&str, from-beg, to-beg);

    ajSeqAssignSeqS(seq, str);

    ajSeqoutWriteSeq(seqout, seq);
    ajSeqoutClose(seqout);

    ajStrDel(&str);
    ajSeqDel(&seq);
    ajSeqoutDel(&seqout);

    embExit();
    return 0;
}

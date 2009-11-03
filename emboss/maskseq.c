/* @source maskseq application
**
** Mask off regions of a sequence
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




/* @prog maskseq **************************************************************
**
** Mask off regions of a sequence
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeq seq = NULL;
    AjPSeqout seqout = NULL;
    AjPRange regions = NULL;
    AjPStr maskchar = NULL;
    AjBool dolower = ajFalse;
    AjPStr str = NULL;
    ajint beg;
    ajint end;

    embInit("maskseq", argc, argv);

    seq      = ajAcdGetSeq("sequence");
    seqout   = ajAcdGetSeqout("outseq");
    regions  = ajAcdGetRange("regions");
    maskchar = ajAcdGetString("maskchar");
    dolower  = ajAcdGetToggle("tolower");

    beg = ajSeqGetBegin(seq)-1;
    end = ajSeqGetEnd(seq)-1;


    /* get the copy of the sequence and the regions */
    ajStrAssignSubS(&str, ajSeqGetSeqS(seq), beg, end);
    ajRangeBegin(regions, beg+1);

    /*
    ** if the mask character is null or space or 'tower' is True, then
    ** ToLower the regions, else replace with maskseq
    */
    if(dolower || ajStrGetLen(maskchar) == 0 || ajStrMatchC(maskchar, " "))
    	ajRangeStrToLower(regions, &str);
    else
        ajRangeStrMask(regions, maskchar, &str);
    
    ajSeqAssignSeqS(seq, str);
    ajSeqoutWriteSeq(seqout, seq);
    ajSeqoutClose(seqout);

    ajSeqDel(&seq);
    ajSeqoutDel(&seqout);
    ajRangeDel(&regions);
    ajStrDel(&maskchar);
    ajStrDel(&str);

    embExit();

    return 0;
}

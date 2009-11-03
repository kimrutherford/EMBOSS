/* @source splitter application
**
** Split a sequence into (overlapping) smaller sequences
**
** @author Copyright (C) Gary Williams (gwilliam@hgmp.mrc.ac.uk)
** @modified: Rewritten for more intuitive overlaps (ableasby@hgmp.mrc.ac.uk)
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




static void origsplitter_write(AjPSeqout seqout, AjPSeq subseq, ajint start,
			   ajint end, const AjPSeq seq);




/* @prog origsplitter *********************************************************
**
** Split a sequence into (overlapping) smaller sequences
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPSeqall seqall;
    AjPSeqout seqout;
    AjPSeq seq;
    AjPSeq subseq = NULL;
    AjPStr str = NULL;
    ajint size;
    ajint overlap;
    ajint len;
    ajint pos;
    AjBool addover;

    embInit("origsplitter", argc, argv);

    seqout  = ajAcdGetSeqoutall("outseq");
    seqall  = ajAcdGetSeqall("sequence");
    size    = ajAcdGetInt("size");
    overlap = ajAcdGetInt("overlap");
    addover = ajAcdGetBoolean("addoverlap");

    subseq = ajSeqNew();
    str    = ajStrNew();

    while(ajSeqallNext(seqall, &seq))
    {
	ajSeqTrim(seq);

	len = ajSeqGetLen(seq);
	pos = 0;

	if(!addover)
	{
	    while(pos+size <= len-1)
	    {
		ajStrAssignSubC(&str,ajSeqGetSeqC(seq),pos,pos+size-1);
		ajSeqAssignSeqS(subseq, str);
		origsplitter_write(seqout,subseq,pos,pos+size-1,seq);
		pos += size-overlap;
	    }

	    ajStrAssignSubC(&str,ajSeqGetSeqC(seq),pos,len-1);
	    ajSeqAssignSeqS(subseq, str);
	    origsplitter_write(seqout,subseq,pos,len-1,seq);
	}
	else
	{
	    while(pos+size+overlap < len-1)
	    {
		ajStrAssignSubC(&str,ajSeqGetSeqC(seq),pos,pos+size+overlap-1);
		ajSeqAssignSeqS(subseq,str);
		origsplitter_write(seqout,subseq,pos,pos+size+overlap-1,seq);
		pos += size;
	    }

	    ajStrAssignSubC(&str,ajSeqGetSeqC(seq),pos,len-1);
	    ajSeqAssignSeqS(subseq, str);
	    origsplitter_write(seqout,subseq,pos,len-1,seq);
	}

    }

    ajSeqoutClose(seqout);
    ajSeqoutDel(&seqout);
    ajSeqDel(&subseq);
    ajStrDel(&str);
    ajSeqallDel(&seqall);
    ajSeqDel(&seq);

    embExit();

    return 0;
}




/* @funcstatic origsplitter_write  ********************************************
**
** Write out split sequence
**
** @param [w] seqout [AjPSeqout] Output object
** @param [w] subseq [AjPSeq] sequence to write
** @param [r] start [ajint] start offset
** @param [r] end [ajint] end offset
** @param [r] seq [const AjPSeq] original trimmed sequence
** @return [void]
** @@
******************************************************************************/

static void origsplitter_write(AjPSeqout seqout, AjPSeq subseq, ajint start,
			   ajint end, const AjPSeq seq)
{
    AjPStr name  = NULL;
    AjPStr value = NULL;

    name  = ajStrNew();
    value = ajStrNew();


    /* create a name for the subsequence */
    ajStrAssignS(&name, ajSeqGetNameS(seq));
    ajStrAppendC(&name, "_");
    ajStrFromInt(&value, ajSeqGetBegin(seq)+start);
    ajStrAppendS(&name, value);
    ajStrAppendC(&name, "-");
    ajStrFromInt(&value, ajSeqGetBegin(seq)+end);
    ajStrAppendS(&name, value);
    ajSeqAssignNameS(subseq, name);

    /* set the description of the subsequence */
    ajSeqAssignDescS(subseq, ajSeqGetDescS(seq));

    /* set the type of the subsequence */
    ajSeqType(subseq);

    ajSeqoutWriteSeq(seqout, subseq);


    ajStrDel(&name);
    ajStrDel(&value);

    return;
}

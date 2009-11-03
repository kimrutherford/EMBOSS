/* @source cons application
**
** Calculates a consensus
** @author Copyright (C) Tim Carver (tcarver@hgmp.mrc.ac.uk)
** @@
**
**
** -plurality  	- defines no. of +ve scoring matches below
**                which there is no consensus.
**
** -identity   	- defines the number of identical symbols
**                requires in an alignment column for it to
**                included in the consensus.
**
** -setcase   	- upper/lower case given if score above/below
**                user defined +ve matching threshold.
**
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




/* @prog cons *****************************************************************
**
** Creates a consensus from multiple alignments
**
******************************************************************************/

int main(int argc, char **argv)
{
    ajint   nseqs;
    ajuint   mlen;
    ajint   i;
    ajint   identity;
    float fplural;
    float setcase;
    const char  *p;
    AjPSeqset seqset;
    AjPSeqout seqout;
    AjPSeq    seqo;
    AjPStr    name = NULL;
    AjPStr    cons;
    AjPMatrix cmpmatrix = 0;


    embInit ("cons", argc, argv);

    seqset    = ajAcdGetSeqset ("sequence");
    cmpmatrix = ajAcdGetMatrix("datafile");
    fplural   = ajAcdGetFloat("plurality");
    setcase   = ajAcdGetFloat("setcase");
    identity  = ajAcdGetInt("identity");
    seqout    = ajAcdGetSeqout("outseq");
    name      = ajAcdGetString ("name");

    nseqs = ajSeqsetGetSize(seqset);
    if(nseqs<2)
	ajFatal("Insufficient sequences (%d) to create a matrix",nseqs);

    mlen = ajSeqsetGetLen(seqset);
    for(i=0;i<nseqs;++i)	/* check sequences are same length */
    {
	p = ajSeqsetGetseqSeqC(seqset,i);
	if(strlen(p)!=mlen)
	{
	    ajWarn("Sequence lengths are not equal: expect %d '%S' is %d",
		   mlen, ajSeqsetGetseqNameS(seqset, i), p);
	    break;
	}
    }

    ajSeqsetFmtUpper(seqset);

    cons = ajStrNewRes(mlen+1);
    embConsCalc (seqset, cmpmatrix, nseqs, mlen,
                 fplural, setcase, identity, ajFalse, &cons);

     /* write out consensus sequence */
    seqo = ajSeqNew();
    ajSeqAssignSeqS(seqo,cons);

    if (ajSeqsetIsNuc(seqset))
      ajSeqSetNuc(seqo);
    else
      ajSeqSetProt(seqo);

    if(name == NULL)
	ajSeqAssignNameS(seqo,ajSeqsetGetNameS(seqset));
    else
	ajSeqAssignNameS(seqo,name);

    ajSeqoutWriteSeq(seqout,seqo);
    ajSeqoutClose(seqout);

    ajStrDel(&cons);
    ajSeqDel(&seqo);
    ajSeqsetDel(&seqset);
    ajMatrixDel(&cmpmatrix);
    ajSeqoutDel(&seqout);
    ajStrDel(&name);

    embExit();

    return 0;
}

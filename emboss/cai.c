/* @source cai application
**
** Calculate codon usage statistics
**
** @author Copyright (C) Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
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




/* @prog cai ******************************************************************
**
** CAI codon adaptation index
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall  seqall;
    AjPSeq     seq;
    AjPFile    outf;
    AjPCod     codon;
    AjPStr     substr;

    ajint beg;
    ajint end;

    float cai;


    embInit("cai", argc, argv);

    seqall = ajAcdGetSeqall("seqall");
    codon  = ajAcdGetCodon("cfile");
    outf   = ajAcdGetOutfile("outfile");

    substr=ajStrNew();

    while(ajSeqallNext(seqall, &seq))
    {
	beg = ajSeqallGetseqBegin(seqall);
	end = ajSeqallGetseqEnd(seqall);
	ajStrAssignSubS(&substr,ajSeqGetSeqS(seq),beg-1,end-1);
	ajStrFmtUpper(&substr);
	cai = (float)ajCodCai(codon,substr);
	ajFmtPrintF(outf,"Sequence: %S CAI: %.3f\n",ajSeqGetNameS(seq), cai);
    }

    ajFileClose(&outf);

    ajCodDel(&codon);
    ajSeqallDel(&seqall);
    ajSeqDel(&seq);
    ajStrDel(&substr);

    embExit();

    return 0;
}


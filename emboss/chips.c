/* @source chips application
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




/* @prog chips ****************************************************************
**
** Codon usage statistics
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall  seqall;
    AjPSeq     seq;
    AjPFile    outf;
    AjPCod     codon;
    AjPStr     substr;
    AjBool     sum;
    
    ajint ccnt;
    ajint beg;
    ajint end;

    float Nc;
    double td;
    

    embInit("chips", argc, argv);

    seqall = ajAcdGetSeqall("seqall");
    outf   = ajAcdGetOutfile("outfile");
    sum    = ajAcdGetBoolean("sum");
    
    codon  = ajCodNewCode(0);

    ccnt = 0;
    substr = ajStrNew();

    while(ajSeqallNext(seqall, &seq))
    {
	beg = ajSeqallGetseqBegin(seqall);
	end = ajSeqallGetseqEnd(seqall);
	ajStrAssignSubS(&substr,ajSeqGetSeqS(seq),beg-1,end-1);
	ajStrFmtUpper(&substr);
	ajCodCountTriplets(codon,substr,&ccnt);
	if(!sum)
	{
	    ajCodCalculateUsage(codon,ccnt);
	    td = ajCodCalcNc(codon);
	    Nc = (float) td;
	    
	    ajFmtPrintF(outf,"%-20s Nc = %.3f\n",ajSeqGetNameC(seq),Nc);
	    ajCodClearData(codon);
	}
    }

    if(sum)
    {
	ajCodCalculateUsage(codon,ccnt);
	td = ajCodCalcNc(codon);
	Nc = (float) td;
	
	ajFmtPrintF(outf,"# CHIPS codon usage statistics\n\n");
	ajFmtPrintF(outf,"Nc = %.3f\n",Nc);
    }
    
    ajFileClose(&outf);
    ajCodDel(&codon);
    ajSeqallDel(&seqall);
    ajSeqDel(&seq);
    ajStrDel(&substr);

    embExit();

    return 0;
}

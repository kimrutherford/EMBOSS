/* @source complex application
**
** Reports complexity of DNA
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




/* @prog complex **************************************************************
**
** Find the linguistic complexity in nucleotide sequences
**
******************************************************************************/

int main(int argc, char **argv)
{
    const char *charseq;
    const char *name;

    AjPSeq    seq;
    AjPSeqall seqall;
    AjPSeqout seqout;
    AjPFile   outfile;
    AjPFile   pfile;

    ajint len;
    ajint Num_seq = 0;

    ajint lwin;
    ajint step;
    ajint sim;
    ajint jmin;
    ajint jmax;

    AjBool print;
    AjBool freq;
    AjBool omnia;

    float MedValue = 0.0;

    embInit("complex",argc,argv);

    lwin  = ajAcdGetInt("lwin");
    step  = ajAcdGetInt("step");
    sim   = ajAcdGetInt("sim");
    jmin  = ajAcdGetInt("jmin");
    jmax  = ajAcdGetInt("jmax");

    omnia = ajAcdGetToggle("omnia");
    freq  = ajAcdGetBoolean("freq");
    print = ajAcdGetBoolean("print");

    seqall = ajAcdGetSeqall("sequence");

    seqout = ajAcdGetSeqoutall("outseq");
    outfile = ajAcdGetOutfile("outfile");
    pfile = ajAcdGetOutfile("ujtablefile");

    ajDebug("Output file: %F \n",outfile);

    if(omnia)
    {
	embComWriteFile(outfile,jmin,jmax,lwin,step,sim);

	while(ajSeqallNext(seqall, &seq))
	{
	    ajSeqoutWriteSeq(seqout,seq);
	    len = ajSeqGetLen(seq);
	    name = ajSeqGetNameC(seq);

	    charseq = ajSeqGetSeqC(seq);
	    if(len >= lwin)
	    {
		Num_seq++;
		embComComplexity(charseq,name,len,jmin,jmax,lwin,
				 step,sim,freq,omnia,
				 outfile,pfile,print,&MedValue);
		embComWriteValueOfSeq(outfile,Num_seq,name,len,MedValue);
	    }
	}
	ajSeqoutClose(seqout);
    }
    else
    {
	Num_seq = 0;
	while((ajSeqallNext(seqall, &seq) && Num_seq < 1))
	{
	    len = ajSeqGetLen(seq);
	    name = ajSeqGetNameC(seq);
	    charseq = ajSeqGetSeqC(seq);
	    Num_seq ++;
	    embComComplexity(charseq,name,len,jmin,jmax,lwin,
			     step,sim,freq,omnia,
			     outfile, pfile,print,&MedValue);
	}
    }


    ajFileClose(&pfile);
    ajFileClose(&outfile);

    ajSeqallDel(&seqall);
    ajSeqoutDel(&seqout);
    ajSeqDel(&seq);

    embExit();

    return 0;
}

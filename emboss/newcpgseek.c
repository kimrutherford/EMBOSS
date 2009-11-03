/* @source newcpgseek application
**
** Reports ALL cpg rich regions in a sequence
** @author Copyright (C) Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
** @author Modified by Rodrigo Lopez (rls@ebi.ac.uk)
** @@
**
** Original program "CPGREPORT" by Rodrigo Lopez (EGCG 1995)
**  CpG island finder. Larsen et al Genomics 13 1992 p1095-1107
**  "usually defined as >200bp with %GC > 50% and obs/exp CpG >
**  0.6". Here use running sum rather than window to score: if not CpG
**  at position i, then decrement runSum counter, but if CpG then runSum
**  += CPGSCORE.     Spans > threshold are searched
**  for recursively.
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
#include <math.h>
#include <stdlib.h>




static void newcpgseek_cpgsearch(AjPFile *outf, ajint s, ajint len,
				 const char *seq,
				 const char *name, ajint begin, ajint score);
static void newcpgseek_calcgc(ajint from, ajint to, const char *p, ajint *dcg,
			      ajint *dgc, ajint *gc);




/* @prog newcpgseek ***********************************************************
**
** Reports CpG rich regions
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPSeq seq    = NULL;
    AjPFile outf  = NULL;
    AjPStr strand = NULL;
    AjPStr substr = NULL;

    ajint begin;
    ajint end;
    ajint len;
    ajint score;

    embInit("newcpgseek",argc,argv);

    seqall = ajAcdGetSeqall("sequence");
    score  = ajAcdGetInt("score");
    outf   = ajAcdGetOutfile("outfile");


    substr = ajStrNew();


    while(ajSeqallNext(seqall, &seq))
    {
	begin = ajSeqallGetseqBegin(seqall);
	end   = ajSeqallGetseqEnd(seqall);

	strand = ajSeqGetSeqCopyS(seq);
	ajStrFmtUpper(&strand);

	ajStrAssignSubC(&substr,ajStrGetPtr(strand),begin-1,end-1);

	len=ajStrGetLen(substr);

	ajFmtPrintF(outf,"\n\nNEWCPGSEEK of %s from %d to %d\n",
		    ajSeqGetNameC(seq),begin,begin+len-1);
	ajFmtPrintF(outf,"with score > %d \n\n",score);

	ajFmtPrintF(outf," Begin    End  Score");
	ajFmtPrintF(outf,"        CpG  %%CG  CG/GC\n");

	newcpgseek_cpgsearch(&outf,0,len,ajStrGetPtr(substr),ajSeqGetNameC(seq),
			     begin,score);
	ajFmtPrintF(outf,"-------------------------------------------\n");

	ajStrDel(&strand);
    }


    ajSeqDel(&seq);
    ajStrDel(&substr);
    ajFileClose(&outf);

    ajSeqallDel(&seqall);
    ajFileClose(&outf);

    embExit();

    return 0;
}




/* @funcstatic newcpgseek_cpgsearch *******************************************
**
** Perform cpg search
**
** @param [u] outf [AjPFile*] outfile
** @param [r] from [ajint] start pos
** @param [r] to [ajint] end pos
** @param [r] p [const char*] sequence
** @param [r] name [const char*] sequence name
** @param [r] begin [ajint] start in sequence
** @param [r] score [ajint] score
** @@
******************************************************************************/

static void newcpgseek_cpgsearch(AjPFile *outf, ajint from, ajint to,
				 const char *p, const char *name, ajint begin,
				 ajint score)
{
    ajint i;
    ajint c;
    ajint z;

    ajint sum;
    ajint ssum;
    ajint lsum;
    ajint t;
    ajint top;

    ajint dcg;
    ajint dgc;
    ajint gc;


    for(i=from,c=to-1,sum=ssum=t=top=0,lsum=-1,z=begin-1;i<to;++i,ssum=sum)
    {
	if(p[i]=='C' && p[i+1]=='G' && c-i)
	    sum+=score+1;
	--sum;

	if(sum<0)
	    sum=0;

	if(!sum && ssum)
	{
	    newcpgseek_calcgc(lsum+1,t+2,p,&dcg,&dgc,&gc);
	    if(dgc)
	    {
		ajFmtPrintF(*outf,"%6d %6d %5d ",lsum+2+z,
			    t+2+z,top);
		ajFmtPrintF(*outf,"     %5d %5.1f %6.2f\n",
			    dcg,(float)gc*100.0/(float)(t+1-lsum),
			    (float)(dcg/dgc));
	    }

	    newcpgseek_cpgsearch(outf,t+2,i,p,name,begin,score);
	    sum = ssum = lsum = t = top =0;
	}

	if(sum>top)
	{
	    t = i;
	    top=sum;
	}

	if(!sum)
	    lsum = i;
    }


    if(sum)
    {
	newcpgseek_calcgc(lsum+1,t+2,p,&dcg,&dgc,&gc);
	if(dgc)
	{
	    ajFmtPrintF(*outf,"*%6d %6d %5d ",lsum+2+z,t+2+z,
			top);
	    ajFmtPrintF(*outf,"     %5d %5.1f %6.2f\n",
			dcg,(float)gc*100.0/(float)(t+1-lsum),
			((float)dcg/(float)dgc));
	}

	newcpgseek_cpgsearch(outf,t+2,to,p,name,begin,score);
    }

    return;
}




/* @funcstatic newcpgseek_calcgc **********************************************
**
** Calculate gc content
**
** @param [r] from [ajint] from
** @param [r] to [ajint] to
** @param [r] p [const char*] sequence
** @param [w] dcg [ajint*] number cg
** @param [w] dgc [ajint*] number gc
** @param [w] gc [ajint*] number c or g
** @@
******************************************************************************/

static void newcpgseek_calcgc(ajint from, ajint to, const char *p, ajint *dcg,
			      ajint *dgc, ajint *gc)
{

    ajint i;
    ajint c;

    c = to-1;

    for(i=from,*gc=*dgc=*dcg=0;i<=c;++i)
    {
	if(p[i]=='G' || p[i]=='C')
	    ++*gc;

	if(p[i]=='C' && p[i+1]=='G' && c-i)
	    ++*dcg ;

	if(p[i]=='G' && p[i+1]=='C' && c-i )
	    ++*dgc ;
    }

    return;
}

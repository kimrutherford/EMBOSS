/* @source plotorf application
**
** Plot potential open reading frames
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
#include <string.h>




static void plotorf_norfs(const char *seq, const char *rev,
			  ajint n, float **x, float **y,
			  AjPInt *cnt, ajint beg, AjPStr const *starts,
			  ajint nstarts, AjPStr const *stops, ajint nstops);
static AjBool plotorf_isin(const char *p, AjPStr const *str, ajint n);





/* @prog plotorf **************************************************************
**
** Plot potential open reading frames
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeq seq;
    AjPStr str;
    AjPStr rev;
    AjPStr *starts = NULL;
    AjPStr *stops  = NULL;
    AjPStr start;
    AjPStr stop;
    ajint nstarts;
    ajint nstops;

    AjPGraph graph;
    AjPGraphdata data;

    float *x[6] = {NULL,NULL,NULL,NULL,NULL,NULL};
    float *y[6] = {NULL,NULL,NULL,NULL,NULL,NULL};
    AjPInt cnt;
    ajint beg;
    ajint end;

    ajint i;
    ajint j;

    const char *ftit[6]=
    {
	"F1","F2","F3","R1","R2","R3"
    };


    embInit("plotorf", argc, argv);
    ajGraphicsSetPagesize(960, 960);

    seq       = ajAcdGetSeq("sequence");
    graph     = ajAcdGetGraphxy("graph");
    start     = ajAcdGetString("start");
    stop      = ajAcdGetString("stop");

    ajStrFmtUpper(&start);
    ajStrFmtUpper(&stop);

    nstarts = ajArrCommaList(start,&starts);
    nstops  = ajArrCommaList(stop,&stops);

    beg = ajSeqGetBegin(seq);
    end = ajSeqGetEnd(seq);

    str = ajStrNew();
    cnt = ajIntNew();

    ajSeqFmtUpper(seq);
    ajStrAssignSubC(&str,ajSeqGetSeqC(seq),beg-1,end-1);

    rev = ajStrNewC(ajStrGetPtr(str));
    ajSeqstrReverse(&rev);

    for(i=0;i<6;++i)
    {
	plotorf_norfs(ajStrGetPtr(str),ajStrGetPtr(rev),i,x,y,&cnt,beg,starts,
		      nstarts,stops,nstops);
	data = ajGraphdataNewI(2);
	data->numofpoints = 0;


	ajGraphDataAdd(graph,data);
	ajGraphxySetflagOverlay(graph,ajFalse);
	ajGraphxyShowYtick(graph, ajFalse);
	ajGraphdataSetTruescale(data,(float)beg,(float)end,0.0,1.0);
	ajGraphdataSetTypeC(data,"Multi 2D Plot Small");
	ajGraphdataSetYlabelC(data,"Orf");
	ajGraphdataSetXlabelC(data,"Sequence");
	ajGraphdataSetTitleC(data,ftit[i]);

	for(j=0;j<ajIntGet(cnt,i);++j)
	    ajGraphdataAddposRect(data,y[i][j],0.0,
					      x[i][j],1.0,4,1);
    }


    ajGraphShowTitle(graph, ajTrue);
    ajGraphxySetMinmax(graph,(float)beg,(float)end,0.0,1.0);

    ajGraphxySetYstartF(graph,0.0);
    ajGraphxySetYendF(graph,2.0);
    ajGraphSetTitleC(graph,"Potential codons (rectangles)");
    ajGraphxyDisplay(graph,ajTrue);
    ajGraphxyDel(&graph);

    ajStrDel(&str);
    ajStrDel(&rev);
    ajStrDel(&start);
    ajStrDel(&stop);
    ajIntDel(&cnt);

    for(i=0;i<nstarts;++i)
	ajStrDel(&starts[i]);
    AJFREE(starts);
    for(i=0;i<nstops;++i)
	ajStrDel(&stops[i]);
    AJFREE(stops);
    for(i=0;i<6;++i)
    {
	AJFREE(x[i]);
	AJFREE(y[i]);
    }

    ajSeqDel(&seq);

    embExit();

    return 0;
}




/* @funcstatic plotorf_norfs **************************************************
**
** Undocumented.
**
** @param [r] seq [const char*] nucleic sequence
** @param [r] rev [const char*] reverse sequence
** @param [r] n [ajint] length
** @param [w] x [float**] xpos
** @param [w] y [float**] ypos
** @param [w] cnt [AjPInt*] orf count
** @param [r] beg [ajint] sequence strat
** @param [r] starts [AjPStr const *] start posns
** @param [r] nstarts [ajint] number of starts
** @param [r] stops [AjPStr const *] stop posns
** @param [r] nstops [ajint] number of stops
** @@
******************************************************************************/

static void plotorf_norfs(const char *seq, const char *rev,
			  ajint n, float **x, float **y,
			  AjPInt *cnt, ajint beg, AjPStr const *starts,
			  ajint nstarts, AjPStr const *stops, ajint nstops)
{
    ajint len;
    ajint i;
    ajint count;
    AjBool inframe;
    ajint po;
    const char *p;
    size_t stlen;

    
    stlen = strlen(seq);
    len   = (ajint) stlen;
    
    if(n<3)
    {
	p  = seq;
	po = n%3;
    }
    else
    {
	p  = rev;
	po = len%3;
	po -= n%3;
	if(po<0)
	    po += 3;
    }


    inframe = ajFalse;
    count = 0;

    for(i=po;i<len-2;i+=3)
    {
	if(plotorf_isin(&p[i],starts,nstarts))
	{
	    if(!inframe)
	    {
		++count;
		inframe=ajTrue;
		continue;
	    }
	}

	if(plotorf_isin(&p[i],stops,nstops))
	    if(inframe)
		inframe=ajFalse;
    }

    if(count)
    {
	AJCNEW(x[n], count);
	AJCNEW(y[n], count);
    }
    ajIntPut(cnt,n,count);


    count = 0;
    inframe = ajFalse;

    for(i=po;i<len-2;i+=3)
    {
	if(plotorf_isin(&p[i],starts,nstarts))
	    if(!inframe)
	    {
		if(ajIntGet(*cnt,n))
		{
		    if(n<3)
			x[n][count]=(float)(i+beg);
		    else
			x[n][count]=(float)((len-i-1)+beg);
		}
		++count;
		inframe = ajTrue;
		continue;
	    }


	if(plotorf_isin(&p[i],stops,nstops))
	    if(inframe)
	    {
		if(ajIntGet(*cnt,n))
		{
		    if(n<3)
			y[n][count-1]=(float)(i+beg);
		    else
			y[n][count-1]=(float)((len-i-1)+beg);
		}
		inframe = ajFalse;
	    }
    }

    if(inframe)
	if(ajIntGet(*cnt,n))
	{
	    if(n<3)
		y[n][count-1]=(float)(len+beg-1);
	    else
		y[n][count-1]=(float) beg;
	}

    return;
}




/* @funcstatic plotorf_isin ***************************************************
**
** True if codon at p occurs in string str
**
** @param [r] p [const char*] codon
** @param [r] str [AjPStr const *] sequence
** @param [r] n [ajint] str length
** @return [AjBool] true if found
** @@
******************************************************************************/

static AjBool plotorf_isin(const char *p, AjPStr const *str, ajint n)
{
    ajint i;
    AjBool ret;

    ret = ajFalse;

    for(i=0;i<n && !ret;++i)
	if(!strncmp(p,ajStrGetPtr(str[i]),3))
	    ret = ajTrue;

    return ret;
}

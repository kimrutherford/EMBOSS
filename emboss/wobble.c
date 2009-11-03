/* @source wobble application
**
** Plot wobble base percentage
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
#include <stdlib.h>




static void wobble_checkstring(AjPStr *str);
static void wobble_calcpc(const char *seq, const char *rev,
			  ajint n, float **x, float **y,
			  ajint *count, ajint beg,
			  const char *gc, ajint window);
static float wobble_get_mean(const char *bases, const char *s);






/* @prog wobble ***************************************************************
**
** Wobble base plot
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPFile outf;
    AjPSeq seq;
    AjPStr gc;
    AjPStr fwd;
    AjPStr rev;


    AjPGraph   graph;
    AjPGraphPlpData data;

    float *x[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
    float *y[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
    float xt;
    float mean;
    float ymin;
    float ymax;

    ajint count[6];

    ajint window;


    ajint beg;
    ajint end;

    ajint i;
    ajint j;

    const char *ftit[6]=
    {
	"F1","F2","F3","R1","R2","R3"
    };




    ajGraphInit("wobble", argc, argv);
    ajGraphSetPage(960, 960);

    seq    = ajAcdGetSeq("sequence");
    graph  = ajAcdGetGraphxy("graph");
    gc     = ajAcdGetString("bases");
    window = ajAcdGetInt("window");
    outf   = ajAcdGetOutfile("outfile");
    
    ajSeqFmtUpper(seq);
    ajStrFmtUpper(&gc);
    
    beg = ajSeqGetBegin(seq);
    end = ajSeqGetEnd(seq);
    
    fwd = ajStrNew();
    rev = ajStrNew();
    
    ajStrAssignSubC(&fwd,ajSeqGetSeqC(seq),beg-1,end-1);
    rev = ajStrNewC(ajStrGetPtr(fwd));
    ajSeqstrReverse(&rev);
    
    
    wobble_checkstring(&gc);
    mean = wobble_get_mean(ajStrGetPtr(gc),ajStrGetPtr(fwd));
    ajFmtPrintF(outf,"Expected %s content in third position = %.2f\n",
		ajStrGetPtr(gc),mean);
    
    for(i=0;i<6;++i)
    {
	wobble_calcpc(ajStrGetPtr(fwd),ajStrGetPtr(rev),i,x,y,
		      count,beg,ajStrGetPtr(gc),
		      window);

	data = ajGraphPlpDataNewI(count[i]);

	ajGraphxySetOverLap(graph,ajFalse);

	for(j=0;j<count[i];++j)
	{
	    xt = x[i][j];
	    if(!xt)
	    {
		x[i][j] = x[i][j-1];
		y[i][j] = y[i][j-1];
	    }
	    data->x[j] = x[i][j];
	    data->y[j] = y[i][j];
	}

	ajGraphArrayMaxMin(data->y,count[i],&ymin,&ymax);
	ajGraphPlpDataSetMaxima(data,(float)beg,(float)end,ymin,ymax);

	ajGraphPlpDataSetTypeC(data,"2D Plot");
	ajGraphDataAdd(graph,data);

	ajGraphxySetYTick(graph, ajTrue);

	ajGraphPlpDataSetYTitleC(data,ajStrGetPtr(gc));
	ajGraphPlpDataSetXTitleC(data,"Sequence");
	ajGraphPlpDataSetTitleC(data,ftit[i]);
	ajGraphPlpDataAddLine(data,(float)beg,mean,(float)end,mean,4);
    }
    
    
    ajGraphSetTitleDo(graph, ajTrue);
    ajGraphxySetMaxMin(graph,(float)beg,(float)end,0.0,100.0);
    
    ajGraphxySetYStart(graph,0.0);
    ajGraphxySetYEnd(graph,100.0);
    ajGraphSetTitleC(graph,"Wobble bases");
    
    ajGraphSetCharScale((float)0.7);
    
    ajGraphxyDisplay(graph,ajTrue);
    
    
    ajStrDel(&gc);
    ajStrDel(&fwd);
    ajStrDel(&rev);
    ajSeqDel(&seq);
    ajFileClose(&outf);
    ajGraphxyDel(&graph);
    for(i=0;i<6;i++)
    {
	AJFREE(x[i]);
	AJFREE(y[i]);
    }

    embExit();

    return 0;
}




/* @funcstatic wobble_calcpc **************************************************
**
** Calculate percentages
**
** @param [r] seq [const char*] sequence
** @param [r] rev [const char*] reverse sequence
** @param [r] n [ajint] frame
** @param [w] x [float**] x-axis
** @param [w] y [float**] y-axis
** @param [w] count [ajint*] Number of codons
** @param [r] beg [ajint] sequence start position
** @param [r] gc [const char*] bases to match
** @param [r] window [ajint] window size
** @@
******************************************************************************/

static void wobble_calcpc(const char *seq, const char *rev,
			  ajint n, float **x, float **y,
			  ajint *count, ajint beg, const char *gc,
			  ajint window)
{
    ajint len;
    ajint limit;

    ajint i;
    ajint j;

    ajint po;
    const char *p;
    float sum;

    ajint nb;
    ajint cds;
    ajint z;
    size_t stlen;
    
    limit = window*3;

    stlen = strlen(seq);
    len = (ajint) stlen;
    
    if(n<3)
    {
	p = seq;
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
    nb = len-po;
    cds = nb/3 - window + 1;

    if(cds<1)
	ajFatal("Sequence too short for window");

    if(!n)
	for(i=0;i<6;++i)
	{
	    AJCNEW(x[i], cds);
	    AJCNEW(y[i], cds);
	    count[i]=cds;
	}


    for(i=0;i<cds;++i)
    {
	sum = (float)0.0;
	for(j=0;j<limit;j+=3)
	{
	    z = (i*3)+j+po+2;
	    if(z<len)
		if(strchr(gc,(ajint)p[z]))
		    ++sum;
	}

	x[n][i] = (float) ((i*3)+beg+(limit/2));
	y[n][i] = (float)((sum/(float)window)*(float)100.0);
    }

    return;
}




/* @funcstatic wobble_checkstring *********************************************
**
** Check that a sensible -bases option has been given. Modify if necessary
**
** @param [w] str [AjPStr*] Undocumented
** @@
******************************************************************************/

static void wobble_checkstring(AjPStr *str)
{
    AjPStr tmp;
    static const char *bases="GCAT";
    ajint i;

    tmp = ajStrNewC("");

    for(i=0;i<4;++i)
	if(strchr(ajStrGetPtr(*str),(ajint)bases[i]))
	    ajStrAppendK(&tmp,bases[i]);
    ajStrAssignC(str,ajStrGetPtr(tmp));

    if(ajStrGetLen(tmp) >3)
	ajFatal("Specifying ACG&T is meaningless");

    if(ajStrGetLen(tmp)<1)
	ajFatal("No bases specified");

    ajStrDel(&tmp);

    return;
}




/* @funcstatic wobble_get_mean ************************************************
**
** Get pc of -bases specified
**
** @param [r] bases [const char*] bases to use
** @param [r] s [const char*] sequence
** @return [float] base percentage
** @@
******************************************************************************/

static float wobble_get_mean(const char *bases, const char *s)
{
    ajint na;
    ajint nc;
    ajint ng;
    ajint nt;
    float tot;
    float sum;
    const char  *p;
    ajint c;

    na = nc = ng = nt = 0;
    ajCodComp(&na,&nc,&ng,&nt,s);
    sum = (float) 0.0;
    p = bases;

    while((c=*p))
    {
	if(c=='A')
	    sum+=na;
	else if(c=='C')
	    sum+=nc;
	else if(c=='G')
	    sum+=ng;
	else if(c=='T')
	    sum+=nt;

	++p;
    }

    tot = (float) na+nc+ng+nt;

    return (sum/tot) * (float) 100.0;
}

/* @source density application
**
** Calculate nucleic acid density within a sliding window
**
** @author Copyright (C) Alan Bleasby (ajb@ebi.ac.uk)
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




/* @datastatic PNucDensity ****************************************************
**
** Nucleic acid composition
**
** @attr pos [float*] window position
** @attr a [float*] A
** @attr c [float*] C
** @attr g [float*] G
** @attr t [float*] T
** @attr at [float*] AT
** @attr gc [float*] GC
** @@
******************************************************************************/

typedef struct SNucDensity
{
    float     *pos;
    float     *a;
    float     *c;
    float     *g;
    float     *t;
    float     *at;
    float     *gc;
} ONucDensity;
#define PNucDensity ONucDensity*




static void density_ReportHits(AjPFeattable TabRpt, const PNucDensity density,
                               ajint limit);

static void density_addquadgraph(AjPGraph qgraph, ajint limit,
				 const PNucDensity density,
				 float ymin, float ymax,
				 ajint window);

static void density_adddualgraph(AjPGraph dgraph, ajint limit,
				 const PNucDensity density,
				 float ymin, float ymax,
				 ajint window);




/* @prog density ***************************************************************
**
** Nucleic acid density
**
******************************************************************************/

int main(int argc, char **argv)
{
    ajint i;
    ajint j;

    AjPSeqall  seqall;
    AjPSeq     seq;
    AjPStr     str;
    AjPStr     hdr;
    
    AjPReport report = NULL;
    AjBool dual = ajFalse;
    AjBool quad = ajFalse;
    AjPGraph graph = NULL; 

    PNucDensity density = NULL;

    ajint beg;
    ajint end;
    ajint window;
    ajint len;
    ajint limit;
    
    const char *p;

    float asum;
    float csum;
    float gsum;
    float tsum;

    float ymin;
    float ymax;

    char c;
    
    AjPFeattable ftable = NULL;
    AjPStr display = NULL;
    

    embInit("density", argc, argv);

    seqall    = ajAcdGetSeqall("seqall");
    display   = ajAcdGetListSingle("display");
    window    = ajAcdGetInt("window");
    
    report  = ajAcdGetReport("outfile");
    graph  = ajAcdGetGraphxy("graph");

    if(ajStrGetCharFirst(display) == 'D')
        dual = ajTrue;
    if(ajStrGetCharFirst(display) == 'Q')
        quad = ajTrue;

    str = ajStrNew();
    hdr = ajStrNew();

    if(report)
    {
	ajFmtPrintS(&hdr, "Window size = %d\n",window);
	ajReportSetHeaderS(report,hdr);
    }

    AJNEW0(density);

    if(graph)
        ajGraphAppendTitleS(graph, ajSeqallGetUsa(seqall));

    while(ajSeqallNext(seqall, &seq))
    {
	beg = ajSeqallGetseqBegin(seqall);
	end = ajSeqallGetseqEnd(seqall);
	len = end-beg+1;

	limit = len - window + 1;

	if(!ftable)
	    ftable = ajFeattableNewSeq(seq);

	ajStrAssignSubC(&str,ajSeqGetSeqC(seq),--beg,--end);
	ajStrFmtUpper(&str);
	p = ajStrGetPtr(str);

	if(limit>0)
	{
	    AJCNEW0(density->pos,limit);
	    AJCNEW0(density->a,limit);
	    AJCNEW0(density->c,limit);
	    AJCNEW0(density->g,limit);
	    AJCNEW0(density->t,limit);
	    AJCNEW0(density->at,limit);
	    AJCNEW0(density->gc,limit);
	}

	for(i=0;i<limit;++i)
	{
	    density->pos[i] = (float)i+1+beg;
	    asum=0.;
	    csum=0.;
	    gsum=0.;
	    tsum=0.;
	    for(j=0;j < window; ++j)
	    {
		c = p[i+j];
		
		if(c == 'A')
		    ++asum;
		else if(c == 'C')
		    ++csum;
		else if(c == 'G')
		    ++gsum;
		else if(c == 'T')
		    ++tsum;
	    }

	    density->a[i] = asum / (float) window;
	    density->c[i] = csum / (float) window;
	    density->g[i] = gsum / (float) window;
	    density->t[i] = tsum / (float) window;
	    
	    density->at[i] = density->a[i] + density->t[i];
	    density->gc[i] = density->g[i] + density->c[i];
	}



	ymin = 0.;
	ymax = 0.;
	
	if(quad)
	    for(i=0;i<limit;++i)
	    {
		ymax = ymax > density->a[i] ? ymax : density->a[i];
		ymax = ymax > density->c[i] ? ymax : density->c[i];
		ymax = ymax > density->g[i] ? ymax : density->g[i];
		ymax = ymax > density->t[i] ? ymax : density->t[i];
	    }

	if(dual)
	    for(i=0;i<limit;++i)
	    {
		ymax = ymax > density->at[i] ? ymax : density->at[i];
		ymax = ymax > density->gc[i] ? ymax : density->gc[i];
	    }


	ymax += (float) 0.1;


	if(report)
	{
	    density_ReportHits(ftable, density, limit);
	    ajReportWrite(report,ftable,seq);
	}
	

        if(graph)
        {
            ajGraphxySetflagOverlay(graph,ajTrue);
            ajGraphSetXlabelC(graph,"Position");
            ajGraphSetYlabelC(graph,"Density");
        }

        
	if(quad)
	    density_addquadgraph(graph, limit, density, ymin, ymax,
				 window);


	if(dual)
	    density_adddualgraph(graph, limit, density, ymin, ymax,
				 window);


        if(graph)
            if(limit > 1)
                ajGraphxyDisplay(graph,ajFalse);

        

	if(limit>0)
	{
	    AJFREE(density->pos);
	    AJFREE(density->a);
	    AJFREE(density->c);
	    AJFREE(density->g);
	    AJFREE(density->t);
	    AJFREE(density->at);
	    AJFREE(density->gc);
	}

	ajFeattableClear(ftable);
    }

    if(graph)
        ajGraphicsClose();

    AJFREE(density);

    ajSeqDel(&seq);
    ajSeqallDel(&seqall);

    if(graph)
        ajGraphxyDel(&graph);

    ajReportClose(report);
    ajReportDel(&report);
    
    ajFeattableDel(&ftable);

    ajStrDel(&str);
    ajStrDel(&hdr);
    ajStrDel(&display);
    
    embExit();

    return 0;
}




/* @funcstatic density_addquadgraph *******************************************
**
** Add data points for A+C+G+T graphs
**
** @param [u] qgraph [AjPGraph] graph object
** @param [r] limit [ajint] range
** @param [r] density [const PNucDensity] co-ords
** @param [r] ymin [float] min y value
** @param [r] ymax [float] max y value
** @param [r] window [ajint] window
** @@
******************************************************************************/

static void density_addquadgraph(AjPGraph qgraph, ajint limit,
				 const PNucDensity density,
				 float ymin, float ymax,
				 ajint window)
{
    ajint i;

    AjPGraphdata data;
    AjPStr st = NULL;

    if(limit<1)
	return;

    st = ajStrNew();

    ajFmtPrintS(&st,"Window = %d. A=Black C=Red G=Green T=Blue",window);
    ajGraphSetSubtitleS(qgraph,st);
    
    data = ajGraphdataNewI(limit);

    for(i=0;i<limit;++i)
    {
	data->x[i] = density->pos[i];
	data->y[i] = density->a[i];
    }

    ajGraphdataSetColour(data,BLACK);
    ajGraphdataSetMinmax(data,density->pos[0],density->pos[limit-1],ymin,
			    ymax);
    ajGraphdataSetTruescale(data,density->pos[0],density->pos[limit-1],ymin,
			    ymax);

    ajGraphdataSetTypeC(data,"2D Plot Float");

    ajGraphDataAdd(qgraph,data);



    data = ajGraphdataNewI(limit);

    for(i=0;i<limit;++i)
    {
	data->x[i] = density->pos[i];
	data->y[i] = density->c[i];
    }

    ajGraphdataSetColour(data,RED);
    ajGraphdataSetMinmax(data,density->pos[0],density->pos[limit-1],ymin,
			    ymax);
    ajGraphdataSetTruescale(data,density->pos[0],density->pos[limit-1],ymin,
			    ymax);

    ajGraphdataSetTypeC(data,"2D Plot Float");

    ajGraphDataAdd(qgraph,data);



    data = ajGraphdataNewI(limit);

    for(i=0;i<limit;++i)
    {
	data->x[i] = density->pos[i];
	data->y[i] = density->g[i];
    }

    ajGraphdataSetColour(data,GREEN);
    ajGraphdataSetMinmax(data,density->pos[0],density->pos[limit-1],ymin,
			    ymax);
    ajGraphdataSetTruescale(data,density->pos[0],density->pos[limit-1],ymin,
			    ymax);

    ajGraphdataSetTypeC(data,"2D Plot Float");

    ajGraphDataAdd(qgraph,data);



    data = ajGraphdataNewI(limit);

    for(i=0;i<limit;++i)
    {
	data->x[i] = density->pos[i];
	data->y[i] = density->t[i];
    }

    ajGraphdataSetColour(data,BLUE);
    ajGraphdataSetMinmax(data,density->pos[0],density->pos[limit-1],ymin,
			    ymax);
    ajGraphdataSetTruescale(data,density->pos[0],density->pos[limit-1],ymin,
			    ymax);

    ajGraphdataSetTypeC(data,"2D Plot Float");

    ajGraphDataAdd(qgraph,data);


    ajStrDel(&st);

    return;
}




/* @funcstatic density_adddualgraph *******************************************
**
** Add data points for AT + GC graphs
**
** @param [u] dgraph [AjPGraph] graph object
** @param [r] limit [ajint] range
** @param [r] density [const PNucDensity] co-ords
** @param [r] ymin [float] min y value
** @param [r] ymax [float] max y value
** @param [r] window [ajint] window
** @@
******************************************************************************/

static void density_adddualgraph(AjPGraph dgraph, ajint limit,
				 const PNucDensity density,
				 float ymin, float ymax,
				 ajint window)
{
    ajint i;

    AjPGraphdata data;
    AjPStr st = NULL;

    if(limit<1)
	return;

    st = ajStrNew();

    ajFmtPrintS(&st,"Window = %d. AT=Green GC=Blue",window);
    ajGraphSetSubtitleS(dgraph,st);

    data = ajGraphdataNewI(limit);

    for(i=0;i<limit;++i)
    {
	data->x[i] = density->pos[i];
	data->y[i] = density->at[i];
    }

    ajGraphdataSetColour(data,GREEN);
    ajGraphdataSetMinmax(data,density->pos[0],density->pos[limit-1],ymin,
			    ymax);
    ajGraphdataSetTruescale(data,density->pos[0],density->pos[limit-1],ymin,
			    ymax);

    ajGraphdataSetTypeC(data,"2D Plot Float");

    ajGraphDataAdd(dgraph,data);



    data = ajGraphdataNewI(limit);

    for(i=0;i<limit;++i)
    {
	data->x[i] = density->pos[i];
	data->y[i] = density->gc[i];
    }

    ajGraphdataSetColour(data,BLUE);
    ajGraphdataSetMinmax(data,density->pos[0],density->pos[limit-1],ymin,
			    ymax);
    ajGraphdataSetTruescale(data,density->pos[0],density->pos[limit-1],ymin,
			    ymax);

    ajGraphdataSetTypeC(data,"2D Plot Float");

    ajGraphDataAdd(dgraph,data);

    ajStrDel(&st);

    return;
}




/* @funcstatic density_ReportHits ********************************************
**
** Report density
**
** @param [u] TabRpt [AjPFeattable] report object
** @param [r] density [const PNucDensity] density
** @param [r] limit [ajint] limit
**
** @return [void]
** @@
******************************************************************************/

static void density_ReportHits(AjPFeattable TabRpt, const PNucDensity density,
                               ajint limit)
{
    AjPFeature feat = NULL;
    AjPStr str;
    
    ajint i;

    str = ajStrNew();

    for(i=0; i < limit; ++i)
    {
	feat = ajFeatNewII(TabRpt,(ajint)density->pos[i],
			   (ajint)density->pos[i]);	

        ajFmtPrintS(&str,"*a %.3f",density->a[i]);
        ajFeatTagAdd(feat, NULL, str);

        ajFmtPrintS(&str,"*c %.3f",density->c[i]);
        ajFeatTagAdd(feat, NULL, str);

        ajFmtPrintS(&str,"*g %.3f",density->g[i]);
        ajFeatTagAdd(feat, NULL, str);

        ajFmtPrintS(&str,"*t %.3f",density->t[i]);
        ajFeatTagAdd(feat, NULL, str);

        ajFmtPrintS(&str,"*at %.3f",density->at[i]);
        ajFeatTagAdd(feat, NULL, str);

        ajFmtPrintS(&str,"*gc %.3f",density->gc[i]);
        ajFeatTagAdd(feat, NULL, str);
    }

    ajStrDel(&str);
    
    return;
}

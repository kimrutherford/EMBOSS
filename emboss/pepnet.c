/* @source pepnet application
**
** Displays proteins as a helical net
** @author Copyright (C) Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
** @@
**
** Original program "PEPNET" in EGCG
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
#include <string.h>




static void pepnet_plotresidue(char c, float x, float y, const char *squares,
			       const char *circles, const char *diamonds);
static void pepnet_drawocta(float x, float y, float size);




/* @prog pepnet ***************************************************************
**
** Displays proteins as a helical net
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeq    seq = NULL;
    AjPStr    strand   = NULL;
    AjPStr    substr   = NULL;
    AjPStr    squares  = NULL;
    AjPStr    diamonds = NULL;
    AjPStr    octags   = NULL;
    AjBool    amphipathic;
    AjPStr    txt   = NULL;
    AjPGraph  graph = NULL;

    ajint begin;
    ajint end;

    ajint lc;

    ajint i;
    ajint j;
    ajint r;

    ajint count;
    ajint pstart;
    ajint pstop;

    float xmin =   0.0;
    float xmax = 150.0;
    float ymin =   0.0;
    float ymax = 112.5;

    float xstart;
    float ystart;
    float ch = (float)1.8;
    float xinc;
    float yinc;
    AjPStr fstr = NULL;

    float x;
    float y;


    embInit("pepnet", argc, argv);


    seq         = ajAcdGetSeq("sequence");
    graph       = ajAcdGetGraph("graph");
    octags      = ajAcdGetString("octags");
    squares     = ajAcdGetString("squares");
    diamonds    = ajAcdGetString("diamonds");
    amphipathic = ajAcdGetToggle("amphipathic");

    ajStrFmtUpper(&octags);
    ajStrFmtUpper(&squares);
    ajStrFmtUpper(&diamonds);

    if(amphipathic)
    {
	ajStrAssignC(&squares,"ACFGILMVWY");
	ajStrAssignC(&diamonds,"");
	ajStrAssignC(&octags,"");
    }


    substr = ajStrNew();
    txt    = ajStrNew();
    fstr   = ajStrNew();




    begin = ajSeqGetBegin(seq);
    end   = ajSeqGetEnd(seq);

    strand = ajSeqGetSeqCopyS(seq);

    ajStrFmtUpper(&strand);
    ajStrAssignSubC(&substr,ajStrGetPtr(strand),begin-1,end-1);

    ajGraphAppendTitleS(graph, ajSeqGetUsaS(seq));

    ajGraphOpenWin(graph, xmin,xmax,ymin,ymax);

    for(count=begin-1,r=0;count<end;count+=231)
    {
	if (count > begin)
	    ajGraphNewpage(graph, ajFalse);
	pstart=count;
	pstop = AJMIN(end-1, count+230);

	ajGraphicsSetCharscale(0.75);

	xstart = 145.0;
	ystart =  80.0;

	yinc = ch * (float)2.5;
	xinc = yinc / (float)2.5;

	x = xstart;

	for(i=pstart;i<=pstop;i+=7)
	{
	    lc = i;
	    if(x < 10.0*xinc)
	    {
		x = xstart;
		ystart -= (float)7.5*yinc;
	    }
	    y=ystart;

	    ajFmtPrintS(&txt,"%d",i+1);

	    ajGraphicsSetFgcolour(RED);
	    ajGraphicsDrawposTextJustify(x-xinc,y-yinc-1,ajStrGetPtr(txt),0.5);

	    for(j=0;j<4;++j)
	    {
		x -= xinc;
		y += yinc;
		if(lc <= pstop)
		    pepnet_plotresidue(*(ajStrGetPtr(substr)+r),x,y,
				       ajStrGetPtr(squares),ajStrGetPtr(octags),
				       ajStrGetPtr(diamonds));
		++r;
		++lc;
	    }
	    y=ystart+yinc/(float)2.0;

	    for(j=4;j<7;++j)
	    {
		x -= xinc;
		y += yinc;
		if(lc <= pstop)
		    pepnet_plotresidue(*(ajStrGetPtr(substr)+r),x,y,
				       ajStrGetPtr(squares),ajStrGetPtr(octags),
				       ajStrGetPtr(diamonds));
		++r;
		++lc;
	    }
	}
    }

    ajGraphicsClose();
    ajGraphxyDel(&graph);

    ajStrDel(&strand);
    ajStrDel(&fstr);

    ajSeqDel(&seq);
    ajStrDel(&substr);

    ajStrDel(&squares);
    ajStrDel(&diamonds);
    ajStrDel(&octags);
    ajStrDel(&txt);

    embExit();

    return 0;
}




/* @funcstatic pepnet_drawocta ************************************************
**
** Draw an octagon
**
** @param [r] x [float] xpos
** @param [r] y [float] xpos
** @param [r] size [float] size
** @@
******************************************************************************/

static void pepnet_drawocta(float x, float y, float size)
{
    static float polyx[]=
    {
	(float)-0.05, (float)0.05, (float)0.1, (float)0.1, (float)0.05,
	(float)-0.05, (float)-0.1, (float)-0.1, (float)-0.05
    };
    static float polyy[]=
    {
	(float)0.1, (float)0.1, (float)0.05, (float)-0.05, (float)-0.1,
	(float)-0.1, (float)-0.05, (float)0.05, (float)0.1
    };

    ajint i;

    for(i=0;i<8;++i)
	ajGraphicsDrawposLine(x+polyx[i]*size,y+polyy[i]*size,x+polyx[i+1]*size,
                              y+polyy[i+1]*size);

    return;
}




/* @funcstatic pepnet_plotresidue *********************************************
**
** Plot a residue
**
** @param [r] c [char] char to plot
** @param [r] x [float] x coordinate
** @param [r] y [float] y coordinate
** @param [r] squares [const char*] residues for squares
** @param [r] octags [const char*] residues for octagons
** @param [r] diamonds [const char*] residues for diamonds
** @@
******************************************************************************/

static void pepnet_plotresidue(char c, float x, float y, const char *squares,
			       const char *octags, const char *diamonds)
{
    static char cs[2];

    cs[1] = '\0';
    *cs=c;


    ajGraphicsSetFgcolour(GREEN);

    if(strstr(squares,cs))
    {
	ajGraphicsSetFgcolour(BLUE);
	ajGraphicsDrawposBox(x-(float)1.5,y-(float)1.32,(float)3.0);
    }

    if(strstr(octags,cs))
    {
	ajGraphicsSetFgcolour(BLUEVIOLET);
	pepnet_drawocta(x,y+(float)0.225,(float)20.0);
    }

    if(strstr(diamonds,cs))
    {
	ajGraphicsSetFgcolour(RED);
	ajGraphicsDrawposDia(x-(float)2.5,y-(float)2.25,(float)5.0);
    }

    ajGraphicsDrawposTextJustify(x,y,cs,0.5);
    ajGraphicsSetFgcolour(GREEN);

    return;
}

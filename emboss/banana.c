/* @source banana application
**
** Banana displays Bending and Curvature Calculations.
**
** @author Copyright (C) Ian Longden (il@sanger.ac.uk)
** @@
** please reference the following report in any publication resulting from
** use of this program.
** Goodsell, D.S. & Dickerson, R.E. (1994) "Bending and Curvature Calculations
** in B-DNA" Nucl. Acids. Res. 22, 5497-5503.
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


#define sinfban(x) (float)sin((double)x)
#define cosfban(x) (float)cos((double)x)




/* @prog banana ***************************************************************
**
** Bending and curvature plot in B-DNA
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeq seq;
    AjPGraph graph = 0;
    AjPFile   outf = NULL;
    AjPFile file   = NULL;
    AjPStr buffer  = NULL;
    float twist[4][4][4];
    float roll[4][4][4];
    float tilt[4][4][4];
    float rbend;
    float rcurve;
    float bendscale;
    float curvescale;
    float twistsum = (float) 0.0;
    float pi       = (float) 3.14159;
    float pifac    = (pi/(float) 180.0);
    float pi2      = pi/(float) 2.0;
    ajint *iseq    = NULL;
    float *x;
    float *y;
    float *xave;
    float *yave;
    float *curve;
    float *bend;
    const char *ptr;
    ajint i;
    ajuint ii;
    ajint k;
    ajint j;
    char residue[2];
    float maxbend;
    float minbend;
    float bendfactor;
    float maxcurve;
    float mincurve;
    float curvefactor;

    float fxp;
    float fyp;
    float yincr;
    float yy1;
    ajint ixlen;
    ajint iylen;
    ajint ixoff;
    ajint iyoff;
    float ystart;
    float defheight;
    float currentheight;
    ajint count;
    ajint portrait = 0;
    ajint title    = 0;
    ajint numres;
    ajint ibeg;
    ajint iend;
    ajint ilen;
    AjPStr sstr = NULL;
    ajint ipos;
    float dx;
    float dy;
    float rxsum;
    float rysum;
    float yp1;
    float yp2;
    double td;

    ajGraphInit("banana", argc, argv);
    seq    = ajAcdGetSeq("sequence");
    file   = ajAcdGetDatafile("anglesfile");
    outf   = ajAcdGetOutfile("outfile");
    graph  = ajAcdGetGraph("graph");
    numres = ajAcdGetInt("residuesperline");

    ibeg = ajSeqGetBegin(seq);
    iend = ajSeqGetEnd(seq);

    ajStrAssignSubS(&sstr, ajSeqGetSeqS(seq), ibeg-1, iend-1);
    ilen = ajStrGetLen(sstr);

    AJCNEW0(iseq,ilen+1);
    AJCNEW0(x,ilen+1);
    AJCNEW0(y,ilen+1);
    AJCNEW0(xave,ilen+1);
    AJCNEW0(yave,ilen+1);
    AJCNEW0(curve,ilen+1);
    AJCNEW0(bend,ilen+1);

    ptr= ajStrGetPtr(sstr);

    for(ii=0;ii<ajStrGetLen(sstr);ii++)
    {
	if(*ptr=='A' || *ptr=='a')
	    iseq[ii+1] = 0;
	else if(*ptr=='T' || *ptr=='t')
	    iseq[ii+1] = 1;
	else if(*ptr=='G' || *ptr=='g')
	    iseq[ii+1] = 2;
	else if(*ptr=='C' || *ptr=='c')
	    iseq[ii+1] = 3;
	else
	    ajErr("%c is not an ATCG hence not valid",*ptr);

	ptr++;
    }


    if(!file)
	ajErr("Banana failed to open angle file");

    ajFileGets(file,&buffer);		/* 3 junk lines */
    ajFileGets(file,&buffer);
    ajFileGets(file,&buffer);

    for(k=0;k<4;k++)
	for(ii=0;ii<4;ii++)
	{
	    if(ajFileGets(file,&buffer))
	    {
		sscanf(ajStrGetPtr(buffer),"%f,%f,%f,%f",
		       &twist[ii][0][k],&twist[ii][1][k],&twist[ii][2][k],
		       &twist[ii][3][k]);
	    }
	    else
		ajErr("Error reading angle file");

	    for(j=0;j<4;j++)
		twist[ii][j][k] *= pifac;
	}


    for(k=0;k<4;k++)
	for(ii=0;ii<4;ii++)
	    if(ajFileGets(file,&buffer))
	    {
		sscanf(ajStrGetPtr(buffer),"%f,%f,%f,%f",&roll[ii][0][k],
		       &roll[ii][1][k],&roll[ii][2][k],&roll[ii][3][k]);
	    }
	    else
		ajErr("Error reading angle file");


    for(k=0;k<4;k++)
	for(ii=0;ii<4;ii++)
	    if(ajFileGets(file,&buffer))
		sscanf(ajStrGetPtr(buffer),"%f,%f,%f,%f",&tilt[ii][0][k],
		       &tilt[ii][1][k],&tilt[ii][2][k],&tilt[ii][3][k]);
	    else
		ajErr("Error reading angle file");


    if(ajFileGets(file,&buffer))
	sscanf(ajStrGetPtr(buffer),"%f,%f,%f,%f",&rbend,&rcurve,
	       &bendscale,&curvescale);
    else
	ajErr("Error reading angle file");

    ajFileClose(&file);
    ajStrDel(&buffer);


    for(ii=1;ii<ajStrGetLen(sstr)-1;ii++)
    {
	twistsum += twist[iseq[ii]][iseq[ii+1]][iseq[ii+2]];
	dx = (roll[iseq[ii]][iseq[ii+1]][iseq[ii+2]]*sinfban(twistsum)) +
	    (tilt[iseq[ii]][iseq[ii+1]][iseq[ii+2]]*sinfban(twistsum-pi2));
	dy = roll[iseq[ii]][iseq[ii+1]][iseq[ii+2]]*cosfban(twistsum) +
	    tilt[iseq[ii]][iseq[ii+1]][iseq[ii+2]]*cosfban(twistsum-pi2);

	x[ii+1] = x[ii]+dx;
	y[ii+1] = y[ii]+dy;

    }

    for(ii=6;ii<ajStrGetLen(sstr)-6;ii++)
    {
	rxsum = 0.0;
	rysum = 0.0;
	for(k=-4;k<=4;k++)
	{
	    rxsum+=x[ii+k];
	    rysum+=y[ii+k];
	}
	rxsum+=(x[ii+5]*(float)0.5);
	rysum+=(y[ii+5]*(float)0.5);
	rxsum+=(x[ii-5]*(float)0.5);
	rysum+=(y[ii-5]*(float)0.5);

	xave[ii] = rxsum*(float)0.1;
	yave[ii] = rysum*(float)0.1;
    }

    for(i=(ajint)rbend+1;i<=ilen-(ajint)rbend-1;i++)
    {
	td = sqrt(((x[i+(ajint)rbend]-x[i-(ajint)rbend])*
		   (x[i+(ajint)rbend]-x[i-(ajint)rbend])) +
		  ((y[i+(ajint)rbend]-y[i-(ajint)rbend])*
		   (y[i+(ajint)rbend]-y[i-(ajint)rbend])));
	bend[i] = (float) td;
	bend[i]*=bendscale;
    }

    for(i=(ajint)rcurve+6;i<=ilen-(ajint)rcurve-6;i++)
    {
	td = sqrt(((xave[i+(ajint)rcurve]-
		    xave[i-(ajint)rcurve])*(xave[i+(ajint)rcurve]-
					    xave[i-(ajint)rcurve]))+
		  ((yave[i+(ajint)rcurve]-yave[i-(ajint)rcurve])*
		   (yave[i+(ajint)rcurve]-yave[i-(ajint)rcurve])));
	curve[i] = (float) td;
    }
    

    if(outf)
    {
	ajFmtPrintF(outf,"Base   Bend      Curve\n");
	ptr = ajStrGetPtr(sstr);
	for(ii=1;ii<=ajStrGetLen(sstr);ii++)
	{
	    ajFmtPrintF(outf,"%c    %6.1f   %6.1f\n",
			*ptr, bend[ii], curve[ii]);
	    ptr++;
	}
	ajFileClose(&outf);
    }

    if(graph)
    {
	maxbend  = minbend  = 0.0;
	maxcurve = mincurve = 0.0;
	for(ii=1;ii<=ajStrGetLen(sstr);ii++)
	{
	    if(bend[ii] > maxbend)
		maxbend = bend[ii];
	    if(bend[ii] < minbend)
		minbend = bend[ii];
	    if(curve[ii] > maxcurve)
		maxcurve = curve[ii];
	    if(curve[ii] < mincurve)
		mincurve = curve[ii];
	}

	ystart = 75.0;

	ajGraphSetTitlePlus(graph, ajSeqGetUsaS(seq));

	ajGraphOpenWin(graph,(float)-1.0, (float)numres+(float)10.0,
		       (float)0.0, ystart+(float)5.0);

	ajGraphGetOut(&fxp,&fyp,&ixlen,&iylen,&ixoff,&iyoff);

	if(ixlen == 0)
	{
	    /* for postscript these are 0.0 ????? */
	    if(portrait)
	    {
		ixlen = 768;
		iylen = 960;
	    }
	    else
	    {
		ixlen = 960;
		iylen = 768;
	    }
	}

	ajGraphGetCharSize(&defheight,&currentheight);
	if(!currentheight)
	{
	    defheight = currentheight = (float) 4.440072;
	    currentheight = defheight *
		((float)ixlen/ ((float)(numres)*(currentheight+(float)1.0)))
		    /currentheight;
	}
	ajGraphSetCharScale(((float)ixlen/((float)(numres)*
					  (currentheight+(float)1.0)))/
			    currentheight);
	ajGraphGetCharSize(&defheight,&currentheight);

	yincr = (currentheight + (float)3.0)*(float)0.3;

	if(!title)
	    yy1 = ystart;
	else
	    yy1 = ystart-(float)5.0;

	count = 1;

	residue[1]='\0';

	bendfactor = (3*yincr)/maxbend;
	curvefactor = (3*yincr)/maxcurve;

	ptr = ajStrGetPtr(sstr);

	yy1 = yy1-(yincr*((float)5.0));
	for(ii=1;ii<=ajStrGetLen(sstr);ii++)
	{
	    if(count > numres)
	    {
		yy1 = yy1-(yincr*((float)5.0));
		if(yy1<1.0)
		{
		    if(!title)
			yy1=ystart;
		    else
			yy1 = ystart-(float)5.0;

		    yy1 = yy1-(yincr*((float)5.0));
		    ajGraphNewPage(graph,AJFALSE);
		}
		count = 1;
	    }
	    residue[0] = *ptr;

	    ajGraphTextEnd((float)(count)+(float)2.0,yy1,residue);

	    if(ii>1 && ii < ajStrGetLen(sstr))
	    {
		yp1 = yy1+yincr + (bend[ii]*bendfactor);
		yp2 = yy1+yincr + (bend[ii+1]*bendfactor);
		ajGraphLine((float)count+(float)1.5,yp1,
			    (float)(count)+(float)2.5,yp2);
	    }

	    ipos = ajStrGetLen(sstr)-(ajint)rcurve-7;
	    if(ipos < 0)
		ipos = 0;

	    if(ii>(ajuint)rcurve+5 && ii< (ajuint) ipos)
	    {
		yp1 = yy1+yincr + (curve[ii]*curvefactor);
		yp2 = yy1+yincr + (curve[ii+1]*curvefactor);
		ajGraphLine((float)count+(float)1.7,yp1,
			    (float)(count)+(float)2.3,yp2);
	    }

	    ajGraphLine((float)count+(float)1.5,yy1+yincr,
			(float)(count)+(float)2.5,yy1+yincr);

	    count++;
	    ptr++;
	}

	ajGraphCloseWin();
    }

    AJFREE(iseq);
    AJFREE(x);
    AJFREE(y);
    AJFREE(xave);
    AJFREE(yave);
    AJFREE(curve);
    AJFREE(bend);

    ajStrDel(&sstr);

    ajSeqDel(&seq);
    ajFileClose(&file);
    ajFileClose(&outf);
    ajGraphxyDel(&graph);

    embExit();

    return 0;
}

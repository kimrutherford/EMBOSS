/* @source pepwindowall application
**
** Displays protein hydropathy.
** @author Copyright (C) Ian Longden (il@sanger.ac.uk)
** @@
** Original program by Jack Kyte and Russell F. Doolittle.
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

#include "limits.h"
#include <float.h>
#include "emboss.h"

#define AZ 28




static AjBool pepwindowall_getnakaidata(AjPFile file, float matrix[],
                                        AjBool normal);




/* @prog pepwindowall *********************************************************
**
** Displays protein hydropathy of a set of sequences
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPFile datafile;
    AjBool normal;
    AjPStr aa0str = NULL;
    AjPSeqset seqset;
    AjPGraphPlpData graphdata;
    AjPGraph mult;
    const char *seq;
    const char *s1;
    ajint *position;
    ajuint i;
    ajuint j;
    ajuint k;
    ajuint w;
    ajint a;
    ajuint midpoint,llen, maxlen, maxsize;
    float total;
    float matrix[AZ];
    float min = 555.5;
    float max = -555.5;
    float v   = 0.;
    float v1  = 0.;
    float ymin = 64000.;
    float ymax = -64000.;
    ajint beg;
    ajint end;
    float fstart;
    float fend;
    ajuint ilen;
    ajuint istart;
    ajuint iend;
    
    ajGraphInit("pepwindowall", argc, argv);

    seqset   = ajAcdGetSeqset("sequences");
    mult     = ajAcdGetGraphxy("graph");
    datafile = ajAcdGetDatafile("datafile");
    llen     = ajAcdGetInt("length");
    normal   = ajAcdGetBoolean("normalize");

    if(!pepwindowall_getnakaidata(datafile,&matrix[0], normal))
	ajExitBad();

    maxsize   = ajSeqsetGetSize(seqset);
    maxlen   = ajSeqsetGetLen(seqset);
    aa0str   = ajStrNewRes(maxlen);
    midpoint = (ajint)((llen+1)/2);

    AJCNEW(position, maxlen);

    istart = ajSeqsetGetBegin(seqset) - 1;

    iend = ajSeqsetGetEnd(seqset);
    if((iend-istart) > llen)
        ilen = iend-istart+1-llen;
    else
    {
        ilen = 1;
        llen = iend-istart;
    }

    fstart = (float) istart;
    fend = (float) iend;

    for(i=0;i<maxsize;i++)
    {
	seq = ajSeqsetGetseqSeqC(seqset, i);
	ajStrSetClear(&aa0str);

	graphdata = ajGraphPlpDataNewI(maxlen);
	ajGraphPlpDataSetTypeC(graphdata,"Overlay 2D Plot");
	ymin = 64000.;
	ymax = -64000.;


	for(k=0;k<iend ;k++)
	    graphdata->x[k] = FLT_MIN;

	s1 = seq + istart;
	k = 0;
	w = istart;
	while(*s1 != '\0' && w <= iend)
	{
	    if(ajBasecodeToInt(*s1) != 27 )
	    {
		ajStrAppendK(&aa0str,(char)ajBasecodeToInt(*s1));
		position[k++]= w+midpoint;
	    }
	    w++;
	    s1++;
	}

	s1 = ajStrGetPtr(aa0str);
	for(j=0;j<=k-llen;j++)
	{
	    total = 0;
	    for(w=0;w<llen;w++)
		total = total + matrix[(ajint)s1[w]];

	    total = total/(float)llen;
	    v1 = (float)position[j];
	    graphdata->x[position[j]] = v1;
	    v    = graphdata->y[position[j]] = total;
	    ymin = (ymin<v) ? ymin : v;
	    ymax = (ymax>v) ? ymax : v;

	    if(total > max)
		max = total;
	    if(total < min)
		min = total;
	    s1++;
	}

	beg = 0;
	while(graphdata->x[beg]<0.00001)
	    ++beg;
	graphdata->numofpoints -= beg;

	for(a=0;a<graphdata->numofpoints;++a)
	{
	    graphdata->x[a] = graphdata->x[a+beg];
	    graphdata->y[a] = graphdata->y[a+beg];
	}

	end = graphdata->numofpoints-1;

	while(graphdata->x[end--]<0.00001)
	    --graphdata->numofpoints;

	end = maxlen-1;
	while(!graphdata->x[end])
	    --end;

	ajGraphPlpDataSetMaxima(graphdata,(float)graphdata->x[0],
			       (float)graphdata->x[graphdata->numofpoints-1],
			       ymin,ymax);

	ajGraphPlpDataSetYTitleC(graphdata,"Hydropathy");
	ajGraphPlpDataSetXTitleC(graphdata,"Sequence");

	ajGraphDataAdd(mult,graphdata);
    }

    min = min * (float) 1.1;
    max = max * (float) 1.1;

    ajGraphxySetGaps(mult,AJTRUE);
    ajGraphxySetOverLap(mult,AJTRUE);

    if(min == max)
    {
        min--;
        max++;
    }

    ajGraphxySetMaxMin(mult,fstart,fend,min,max);
    ajGraphSetTitleC(mult,"Pepwindowall");


    ajGraphxyDisplay(mult,AJTRUE);
    ajGraphxyDel(&mult);

    ajSeqsetDel(&seqset);
    ajFileClose(&datafile);
    ajStrDel(&aa0str);

    AJFREE(position);

    embExit();

    return 0;
}


/* @funcstatic pepwindowall_getnakaidata **************************************
**
** Read the NAKAI (AAINDEX) data file
**
** @param [u] file [AjPFile] Input file
** @param [w] matrix [float[]] Data values for each amino acid
** @param [r] normal [AjBool] If true, normalize data to mean 0.0 and SD 1.0
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/
static AjBool pepwindowall_getnakaidata(AjPFile file, float matrix[],
                                        AjBool normal)
{
    AjPStr buffer = NULL;
    AjPStr buf2   = NULL;
    AjPStr delim  = NULL;
    AjBool description = ajFalse;
    AjPStrTok token;
    ajint line = 0;
    const char *ptr;
    ajint cols;
    ajint i;

    if(!file)
	return 0;


    delim  = ajStrNewC(" :\t\n");
    buffer = ajStrNew();
    buf2   = ajStrNew();


    for (i=0;i<26;i++) {
	matrix[i] = FLT_MIN;
    }

    while(ajReadline(file,&buffer))
    {
	ptr = ajStrGetPtr(buffer);
	if(*ptr == 'D')			/* description */
	    description = ajTrue;
	else if(*ptr == 'I')
	    line = 1;
	else if(line == 1)
	{
	    line++;
            ajStrRemoveWhiteExcess(&buffer);
	    token = ajStrTokenNewS(buffer,delim);
	    cols = ajStrParseCountS(buffer,delim);
	    ajDebug("num of cols = %d\n",cols);

	    ajStrTokenNextParseS(&token,delim,&buf2);
	    ajStrToFloat(buf2,&matrix[0]);

	    ajStrTokenNextParseS(&token,delim,&buf2);
	    ajStrToFloat(buf2,&matrix[17]);

	    ajStrTokenNextParseS(&token,delim,&buf2);
	    ajStrToFloat(buf2,&matrix[13]);

	    ajStrTokenNextParseS(&token,delim,&buf2);
	    ajStrToFloat(buf2,&matrix[3]);

	    ajStrTokenNextParseS(&token,delim,&buf2);
	    ajStrToFloat(buf2,&matrix[2]);

	    ajStrTokenNextParseS(&token,delim,&buf2);
	    ajStrToFloat(buf2,&matrix[16]);

	    ajStrTokenNextParseS(&token,delim,&buf2);
	    ajStrToFloat(buf2,&matrix[4]);

	    ajStrTokenNextParseS(&token,delim,&buf2);
	    ajStrToFloat(buf2,&matrix[6]);

	    ajStrTokenNextParseS(&token,delim,&buf2);
	    ajStrToFloat(buf2,&matrix[7]);

	    ajStrTokenNextParseS(&token,delim,&buf2);
	    ajStrToFloat(buf2,&matrix[8]);

	    ajStrTokenDel(&token);
	}
	else if(line == 2)
	{
	    line++;
	    ajStrRemoveWhiteExcess(&buffer);
	    token = ajStrTokenNewS(buffer,delim);
	    cols  = ajStrParseCountS(buffer,delim);
	    ajStrTokenNextParseS(&token,delim,&buf2);
	    ajStrToFloat(buf2,&matrix[11]);

	    ajStrTokenNextParseS(&token,delim,&buf2);
	    ajStrToFloat(buf2,&matrix[10]);

	    ajStrTokenNextParseS(&token,delim,&buf2);
	    ajStrToFloat(buf2,&matrix[12]);

	    ajStrTokenNextParseS(&token,delim,&buf2);
	    ajStrToFloat(buf2,&matrix[5]);

	    ajStrTokenNextParseS(&token,delim,&buf2);
	    ajStrToFloat(buf2,&matrix[15]);

	    ajStrTokenNextParseS(&token,delim,&buf2);
	    ajStrToFloat(buf2,&matrix[18]);

	    ajStrTokenNextParseS(&token,delim,&buf2);
	    ajStrToFloat(buf2,&matrix[19]);

	    ajStrTokenNextParseS(&token,delim,&buf2);
	    ajStrToFloat(buf2,&matrix[22]);

	    ajStrTokenNextParseS(&token,delim,&buf2);
	    ajStrToFloat(buf2,&matrix[24]);

	    ajStrTokenNextParseS(&token,delim,&buf2);
	    ajStrToFloat(buf2,&matrix[21]);

	    ajStrTokenDel(&token);
	}
    }

    if(normal)
        embPropNormalF(matrix, FLT_MIN);

    embPropFixF(matrix, FLT_MIN);

    ajStrDel(&buffer);
    ajStrDel(&buf2);
    ajStrDel(&delim);

    return description;
}

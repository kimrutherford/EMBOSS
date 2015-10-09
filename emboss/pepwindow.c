/* @source pepwindow application
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

#include <limits.h>
#include <float.h>
#include "emboss.h"

#define AZ 28




static AjBool pepwindow_getnakaidata(AjPFile file, float matrix[],
                                     AjBool normal);




/* @prog pepwindow ************************************************************
**
** Displays protein hydropathy
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPFile datafile;
    AjPStr aa0str = 0;
    const char *s1;
    AjPSeq seq;
    ajuint llen;
    AjBool normal;
    float matrix[AZ];
    ajuint i;
    ajuint midpoint;
    ajuint j;
    ajuint k;
    AjPGraphdata graphdata;
    AjPGraph mult;
    float min = 555.5;
    float max = -555.5;
    float total;
    float fstart;
    float fend;
    ajuint ilen;
    ajuint istart;
    ajuint iend;

    embInit("pepwindow", argc, argv);

    seq  = ajAcdGetSeq("sequence");
    ilen = ajSeqGetLen(seq);
    
    mult     = ajAcdGetGraphxy("graph");
    datafile = ajAcdGetDatafile("datafile");
    llen     = ajAcdGetInt("window");
    normal   = ajAcdGetBoolean("normalize");

    s1 = ajStrGetPtr(ajSeqGetSeqS(seq));

    aa0str = ajStrNewRes(ilen+1);

    midpoint = (ajint)((llen+1)/2);

    istart = ajSeqGetBegin(seq) - 1;

    iend = ajSeqGetEnd(seq);
    if((iend-istart) > llen)
        ilen = iend-istart+1-llen;
    else
        ilen = 1;

    graphdata = ajGraphdataNewI(ilen);

    ajGraphdataSetTypeC(graphdata,"2D Plot");

    ajGraphDataAdd(mult,graphdata);

    for(i=0;i<iend;i++)
	ajStrAppendK(&aa0str,(char)ajBasecodeToInt(*s1++));


    if(!pepwindow_getnakaidata(datafile,&matrix[0], normal))
	ajExitBad();

    s1 = ajStrGetPtr(aa0str) + istart;

    k=0;
    for(i=istart;i<=iend-llen;i++)
    {
	total = 0;
	for(j=0;j<llen;j++)
	    total += matrix[(ajint)s1[j]];

	total /= (float)llen;
	graphdata->x[k] = (float)i+midpoint;
	graphdata->y[k] = total;
	if(total > max)
	    max= total;
	if(total < min)
	    min = total;
        k++;
	s1++;
    }
    fstart = (float) istart;
    fend = (float) iend;
    ajGraphdataSetTruescale(graphdata,fstart,fend,min,max);

    min = min*(float)1.1;
    max = max*(float)1.1;

    ajGraphdataSetMinmax(graphdata,fstart,fend,min,max);
    ajGraphxySetMinmax(mult,fstart,fend,min,max);

    ajGraphxyDisplay(mult,AJTRUE);
    ajGraphxyDel(&mult);

    ajFileClose(&datafile);
    ajSeqDel(&seq);

    ajStrDel(&aa0str);

    embExit();

    return 0;
}




/* @funcstatic pepwindow_getnakaidata *****************************************
**
** Read the NAKAI (AAINDEX) data file
**
** @param [u] file [AjPFile] Input file
** @param [w] matrix [float[]] Data values for each amino acid
** @param [r] normal [AjBool] If true, normalize data to mean 0.0 and SD 1.0
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/


static AjBool pepwindow_getnakaidata(AjPFile file, float matrix[],
                                     AjBool normal)
{
    AjPStr buffer = NULL;
    AjPStr buf2   = NULL;
    AjPStr delim  = NULL;
    AjPStr description = NULL;
    AjPStrTok token;
    ajint line = 0;
    const char *ptr;
    ajint i;

    if(!file)
	return 0;

    delim  = ajStrNewC(" :\t\n");
    buffer = ajStrNew();
    buf2   = ajStrNew();
    description = ajStrNew();

    for (i=0;i<26;i++) {
	matrix[i] = FLT_MIN;
    }

    while(ajReadline(file,&buffer))
    {
	ptr = ajStrGetPtr(buffer);
	if(*ptr == 'D')			/* save description */
	    ajStrAssignS(&description, buffer);
	else if(*ptr == 'I')
	    line = 1;
	else if(line == 1)
	{
	    line++;
	    ajStrRemoveWhiteExcess(&buffer);

	    token = ajStrTokenNewS(buffer,delim);

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
    ajStrDel(&description);
    ajStrDel(&buf2);
    ajStrDel(&delim);

    return ajTrue;
}

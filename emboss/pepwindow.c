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




static AjBool pepwindow_getnakaidata(AjPFile file, float matrix[]);




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
    ajint llen;
    float matrix[AZ];
    ajuint i;
    ajint midpoint;
    ajint j;
    AjPGraphPlpData graphdata;
    AjPGraph mult;
    float min = 555.5;
    float max = -555.5;
    float total;
    float flen;
    ajuint ilen;
    ajuint tui;
    
    ajGraphInit("pepwindow", argc, argv);

    seq  = ajAcdGetSeq("sequence");
    ilen = ajSeqGetLen(seq);
    tui  = ajSeqGetLen(seq);
    flen = (float) tui;
    
    mult     = ajAcdGetGraphxy("graph");
    datafile = ajAcdGetDatafile("datafile");
    llen     = ajAcdGetInt("length");

    s1 = ajStrGetPtr(ajSeqGetSeqS(seq));

    aa0str = ajStrNewRes(ilen+1);

    graphdata = ajGraphPlpDataNewI(ilen-llen);

    midpoint = (ajint)((llen+1)/2);

    ajGraphPlpDataSetTypeC(graphdata,"2D Plot");

    ajGraphDataAdd(mult,graphdata);

    for(i=0;i<ilen;i++)
	ajStrAppendK(&aa0str,(char)ajAZToInt(*s1++));


    if(!pepwindow_getnakaidata(datafile,&matrix[0]))
	ajExitBad();

    s1 = ajStrGetPtr(aa0str);

    for(i=0;i<ilen-llen;i++)
    {
	total = 0;
	for(j=0;j<llen;j++)
	    total += matrix[(ajint)s1[j]];


	total /= (float)llen;
	graphdata->x[i] = (float)i+midpoint;
	graphdata->y[i] = total;
	if(total > max)
	    max= total;
	if(total < min)
	    min = total;

	s1++;
    }

    ajGraphPlpDataSetMaxima(graphdata,0.,flen,min,max);

    min = min*(float)1.1;
    max = max*(float)1.1;

    ajGraphPlpDataSetMaxMin(graphdata,0.0,flen,min,max);
    ajGraphxySetMaxMin(mult,0.0,flen,min,max);

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
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/


static AjBool pepwindow_getnakaidata(AjPFile file, float matrix[])
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

    while(ajFileGets(file,&buffer))
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

    embPropFixF(matrix, FLT_MIN);

    ajStrDel(&buffer);
    ajStrDel(&description);
    ajStrDel(&buf2);
    ajStrDel(&delim);

    return ajTrue;
}

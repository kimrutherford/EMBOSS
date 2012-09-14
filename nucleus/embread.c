/* @source embread ************************************************************
**
** Data file reading routines
**
** @author Copyright (c) 1999 Alan Bleasby
** @version $Revision: 1.18 $
** @modified $Date: 2011/11/08 15:12:52 $ by $Author: rice $
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public
** License along with this library; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA  02110-1301,  USA.
**
******************************************************************************/


#include "ajlib.h"

#include "embread.h"
#include "ajfiledata.h"
#include "ajfileio.h"
#include "ajsys.h"
#include "ajbase.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>




/* @func embReadAminoDataDoubleC **********************************************
**
** Read amino acid properties from amino.dat
**
** @param [r] s [const char*] datafile name
** @param [w] a [double**] array for amino acid values
** @param [r] fill [double] initialisation value
**
** @return [AjBool] ajTrue on success
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool embReadAminoDataDoubleC(const char *s, double **a, double fill)
{
    AjPFile inf;
    AjPStr  line;

    const char *p;
    ajint  idx;
    ajint  i;

    inf = ajDatafileNewInNameC(s);

    if(!inf)
    {
	ajWarn("File [%s] not found",s);

	return ajFalse;
    }

    *a = AJALLOC(AJREADAMINO*sizeof(double));

    for(i=0;i<AJREADAMINO;++i)
	(*a)[i] = fill;

    line = ajStrNew();

    while(ajReadlineTrim(inf,&line))
    {
	p = ajStrGetPtr(line);

	if(*p=='#' || *p=='!' || !*p)
	    continue;

	p = ajSysFuncStrtok(p," \t\r");

	if(!p || *(p+1))
	{
	    ajWarn("First token is not a single letter");
	    ajFileClose(&inf);
	    ajStrDel(&line);
	    AJFREE(*a);

	    return ajFalse;
	}

	idx = ajBasecodeToInt(*p);
	p = ajSysFuncStrtok(NULL," \t\r");

	if(!p)
	{
	    ajWarn("Missing second token");
	    ajFileClose(&inf);
	    ajStrDel(&line);
	    AJFREE(*a);

	    return ajFalse;
	}

	if(sscanf(p,"%lf",&(*a)[idx])!=1)
	{
	    ajWarn("Bad numeric conversion [%s]",p);
	    ajFileClose(&inf);
	    ajStrDel(&line);
	    AJFREE(*a);

	    return ajFalse;
	}
    }

    ajFileClose(&inf);
    ajStrDel(&line);

    return ajTrue;
}




/* @func embReadAminoDataFloatC ***********************************************
**
** Read amino acid properties from amino.dat
**
** @param [r] s [const char*] datafile name
** @param [w] a [float**] array for amino acid values
** @param [r] fill [float] initialisation value
**
** @return [AjBool] ajTrue on success
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool embReadAminoDataFloatC(const char *s, float **a, float fill)
{
    AjPFile inf;
    AjPStr  line;

    const char *p;
    ajint  idx;
    ajint  i;

    inf = ajDatafileNewInNameC(s);

    if(!inf)
    {
	ajWarn("File [%s] not found",s);

	return ajFalse;
    }

    *a = AJALLOC(AJREADAMINO*sizeof(float));

    for(i=0;i<AJREADAMINO;++i)
	(*a)[i] = fill;

    line = ajStrNew();

    while(ajReadlineTrim(inf,&line))
    {
	p = ajStrGetPtr(line);

	if(*p=='#' || *p=='!' || !*p)
	    continue;

	p = ajSysFuncStrtok(p," \t\r");

	if(!p || *(p+1))
	{
	    ajWarn("First token is not a single letter");
	    ajFileClose(&inf);
	    ajStrDel(&line);
	    AJFREE(*a);

	    return ajFalse;
	}

	idx = ajBasecodeToInt(*p);
	p   = ajSysFuncStrtok(NULL," \t\r");

	if(!p)
	{
	    ajWarn("Missing second token");
	    ajFileClose(&inf);
	    ajStrDel(&line);
	    AJFREE(*a);

	    return ajFalse;
	}

	if(sscanf(p,"%f",&(*a)[idx])!=1)
	{
	    ajWarn("Bad numeric conversion [%s]",p);
	    ajFileClose(&inf);
	    ajStrDel(&line);
	    AJFREE(*a);

	    return ajFalse;
	}
    }

    ajFileClose(&inf);
    ajStrDel(&line);

    return ajTrue;
}




/* @func embReadAminoDataIntC *************************************************
**
** Read amino acid properties from amino.dat
**
** @param [r] s [const char*] datafile name
** @param [w] a [ajint**] array for amino acid values
** @param [r] fill [ajint] initialisation value
**
** @return [AjBool] ajTrue on success
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool embReadAminoDataIntC(const char *s, ajint **a, ajint fill)
{
    AjPFile inf;
    AjPStr  line;

    const char *p;
    ajint  idx;
    ajint  i;

    inf = ajDatafileNewInNameC(s);

    if(!inf)
    {
	ajWarn("File [%s] not found",s);

	return ajFalse;
    }

    *a = AJALLOC(AJREADAMINO*sizeof(ajint));

    for(i=0;i<AJREADAMINO;++i)
	(*a)[i] = fill;


    line = ajStrNew();

    while(ajReadlineTrim(inf,&line))
    {
	p = ajStrGetPtr(line);

	if(*p=='#' || *p=='!' || !*p)
	    continue;

	p = ajSysFuncStrtok(p," \t\r");

	if(!p || *(p+1))
	{
	    ajWarn("First token is not a single letter");
	    ajFileClose(&inf);
	    ajStrDel(&line);
	    AJFREE(*a);

	    return ajFalse;
	}

	idx = ajBasecodeToInt(*p);
	p   = ajSysFuncStrtok(NULL," \t\r");

	if(!p)
	{
	    ajWarn("Missing second token");
	    ajFileClose(&inf);
	    ajStrDel(&line);
	    AJFREE(*a);

	    return ajFalse;
	}

	if(sscanf(p,"%d",&(*a)[idx])!=1)
	{
	    ajWarn("Bad numeric conversion [%s]",p);
	    ajFileClose(&inf);
	    ajStrDel(&line);
	    AJFREE(*a);

	    return ajFalse;
	}
    }

    ajFileClose(&inf);
    ajStrDel(&line);

    return ajTrue;
}

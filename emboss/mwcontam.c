/* @source mwcontam application
**
** Shows contamination molwts (those that are the same in a comma-separated
** set of molwt filenames).
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


#define MILLION 1000000.




static void mwcontam_readdata(AjPList files, AjPList **lists,
			      ajint *n);
static void mwcontam_complists(AjPList one, AjPList *two, float tolerance);




/* @prog mwcontam *************************************************************
**
** Show contaminating molecular weights
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPList files = NULL;
    AjPFile outf = NULL;
    float tolerance = 0.0;
    AjPList *lists = NULL;
    double  *ptr = NULL;

    ajint n;
    ajint i;

    embInit("mwcontam", argc, argv);

    files     = ajAcdGetFilelist("files");
    tolerance = ajAcdGetFloat("tolerance");
    outf      = ajAcdGetOutfile("outfile");


    mwcontam_readdata(files,&lists,&n);

    if(n>1)
    {
	for(i=0;i<n-1;++i)
	    mwcontam_complists(lists[i],&lists[i+1],tolerance);
	while(ajListPop(lists[n-1],(void **)&ptr))
	{
	    ajFmtPrintF(outf,"%.3lf\n",*ptr);
	    AJFREE(ptr);
	}
    }


    for(i=0;i<n;++i)
    {
	while(ajListPop(lists[i],(void **)&ptr))
	    AJFREE(ptr);
	ajListFree(&lists[i]);
    }

    AJFREE(lists);

    ajListFree(&files);
    ajFileClose(&outf);

    embExit();

    return 0;
}




/* @funcstatic mwcontam_readdata **********************************************
**
** Read molecular weight files.
**
** @param [u] files [AjPList] List of files
** @param [w] lists [AjPList**] Array of lists for molwts
** @param [w] n [ajint*] number of files/lists
** @@
******************************************************************************/
static void mwcontam_readdata(AjPList files, AjPList **lists,
			      ajint *n)
{
    AjPFile inf   = NULL;
    AjPStr  line  = NULL;
    AjPStr  thysf = NULL;
    ajint   nfiles;
    ajint   i;
    double  *ptr = NULL;
    double  val = 0.;
    char    c;

    nfiles = *n = ajListGetLength(files);

    if(!nfiles)
	ajFatal("No input files were specified");

    AJCNEW0(*lists,nfiles);

    line = ajStrNew();

    for(i=0;i<nfiles;++i)
    {
	(*lists)[i] = ajListNew();
	ajListPop(files,(void **)&thysf);
	inf = ajFileNewIn(thysf);

	if(!inf)
	    ajFatal("Cannot open file %S",thysf);

	while(ajFileReadLine(inf,&line))
	{
	    c = *ajStrGetPtr(line);

	    if(c=='#' || !c || c=='\n')
		continue;

	    if(sscanf(ajStrGetPtr(line),"%lf",&val)!=1)
		continue;
	    AJNEW(ptr);
	    *ptr = val;
	    ajListPushAppend((*lists)[i],(void *)ptr);
	}
	ajFileClose(&inf);
	ajStrDel(&thysf);
    }

    ajStrDel(&line);

    return;
}




/* @funcstatic mwcontam_complists *********************************************
**
** Compare two lists for matching molwts leaving the result in
** the second list.
**
** @param [u] one [AjPList] First list
** @param [d] two [AjPList*] Second list, deleted at the end
** @param [r] tolerance [float] mw tolerance (ppm)
** @@
******************************************************************************/

static void mwcontam_complists(AjPList one, AjPList *two, float tolerance)
{
    ajint len1;
    ajint len2;
    AjPList result;
    double tol;
    double mwmin;
    double mwmax;
    double *ptr;
    double oval;
    double tval;
    double ppmval;
    ajint  i;
    ajint  j;

    tol = (double) tolerance;

    result = ajListNew();


    len1 = ajListGetLength(one);
    len2 = ajListGetLength(*two);

    for(i=0;i<len1;++i)
    {
	ajListPop(one,(void **)&ptr);
	oval = *ptr;
	ajListPushAppend(one,(void *)ptr);

	ppmval = oval * tol / MILLION;
	mwmin  = oval - ppmval;
	mwmax  = oval + ppmval;

	for(j=0;j<len2;++j)
	{
	    ajListPop(*two,(void **)&ptr);
	    tval = *ptr;
	    ajListPushAppend(*two,(void *)ptr);

	    if(tval>=mwmin && tval<=mwmax)
	    {
		AJNEW0(ptr);
		*ptr = oval;
		ajListPush(result,(void *)ptr);
		j = len2-1;
	    }
	}
    }

    while(ajListPop(*two,(void **)&ptr))
	AJFREE(ptr);
    ajListFree(two);
    *two = result;

    return;
}

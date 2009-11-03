/* @source demolist application
**
** Demomnstration of how the list functions should be used.
** @author Copyright (C) Peter Rice (pmr@sanger.ac.uk)
** @@
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




/* @datastatic DemolistPgff ***************************************************
**
** demolist struct
**
** @alias DemolistSgff
** @alias DemolistOgff
**
** @attr clone [AjPStr] Demolist example struct
** @attr source [AjPStr]  Demolist example struct
** @attr type [AjPStr]  Demolist example struct
** @attr start [ajint]  Demolist example struct
** @attr end [ajint]  Demolist example struct
** @attr score [ajint]  Demolist example struct
******************************************************************************/

typedef struct DemolistSgff
{
    AjPStr clone;
    AjPStr source;
    AjPStr type;
    ajint  start;
    ajint  end;
    ajint  score;
} DemolistOgff;
#define DemolistPgff DemolistOgff*




static DemolistPgff demolist_creategff(const AjPStr line);
static ajint demolist_typecomp(const void *a, const void *b);
static ajint demolist_startcomp(const void *a, const void *b);
static void  demolist_dumpOut(void **x, void *cl);
static void  demolist_freegff (void **x, void *cl);



/* @prog demolist *************************************************************
**
** Read in a gff file and output sorting by user option before outputting
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPList list = NULL;
    AjPFile gfffile;
    AjPStr  line = NULL;
    DemolistPgff  gffnew;
    AjIList iter = NULL;
    void **array = NULL;
    ajint i;
    ajint ia;

    embInit ("demolist", argc, argv);

    /*open file */
    gfffile = ajAcdGetInfile("infile");

    /* create a new list */
    list = ajListNew();

    while(ajFileReadLine(gfffile, &line))
    {
	/* create new gff */
	gffnew = demolist_creategff(line);

	/* add it to the list if okay */
	if(gffnew)
	    ajListPush(list,(void *)gffnew);
    }


    ajUser("\nOutput via the ajListIter method \nSorted by source");

    /* Print out the list using the iterator */
    iter = ajListIterNewread(list);
    while(!ajListIterDone(iter))
    {
	gffnew = (DemolistPgff) ajListIterGet(iter) ;
	ajUser("%S\t%S\t%S\t%d\t%d\t%d",gffnew->clone,gffnew->source,
	       gffnew->type,gffnew->start,gffnew->end,gffnew->score);
    }

    ajListIterDel(&iter);



    ajListSort(list, demolist_startcomp);
    ajUser("\nOutput via the ajListMap function \nSorted by start pos");
    ajListMap(list,demolist_dumpOut,NULL);


    /* printout the list but use the array method */
    ajListSort(list, demolist_typecomp);
    ajUser("\nOutput via the array method \nSorted by type");
    ia = ajListToarray(list, &array);
    for (i = 0; i < ia; i++)
    {
	gffnew = (DemolistPgff) array[i];
	ajUser("%S\t%S\t%S\t%d\t%d\t%d",gffnew->clone,gffnew->source,
	       gffnew->type,gffnew->start,gffnew->end,gffnew->score);
    }

    /* free the objects in the list */
    ajListMap(list,demolist_freegff,NULL);

    ajListFree(&list);
    ajStrDel(&line);
    ajFileClose(&gfffile);

    AJFREE(array);
    
    embExit();

    return 0;
}




/* @funcstatic demolist_typecomp **********************************************
**
** Undocumented.
**
** @param [r] a [const void*] Undocumented
** @param [r] b [const void*] Undocumented
** @return [ajint] Undocumented
** @@
******************************************************************************/

static ajint demolist_typecomp(const void *a, const void *b)
{
    const DemolistPgff gfa;
    const DemolistPgff gfb;

    gfa = *(DemolistPgff const *) a;
    gfb = *(DemolistPgff const *) b;

    return ajStrVcmp(&gfa->type,&gfb->type);
}




/* @funcstatic demolist_startcomp *********************************************
**
** Undocumented.
**
** @param [r] a [const void*] Undocumented
** @param [r] b [const void*] Undocumented
** @return [ajint] Undocumented
** @@
******************************************************************************/

static ajint demolist_startcomp(const void *a, const void *b)
{
    const DemolistPgff gfa;
    const DemolistPgff gfb;

    gfa = *(DemolistPgff const *) a;
    gfb = *(DemolistPgff const *) b;

    if(gfa->start > gfb->start)
	return 1;
    else if (gfa->start == gfb->start)
	return 0;

    return -1;
}




/* @funcstatic demolist_dumpOut ***********************************************
 **
 ** Undocumented.
 **
 ** @param [r] x [void**] Undocumented
 ** @param [r] cl [void*] Undocumented
 ** @return [void]
 ** @@
 ******************************************************************************/

static void  demolist_dumpOut(void **x, void *cl)
{
    DemolistPgff gffnew;

    (void) cl;				/* make it used */

    gffnew = (DemolistPgff)*x;

    ajUser("%S\t%S\t%S\t%d\t%d\t%d",gffnew->clone,gffnew->source,gffnew->type,
	   gffnew->start,gffnew->end,gffnew->score);

    return;
}




/* @funcstatic demolist_freegff ***********************************************
**
** Undocumented.
**
** @param [r] x [void**] Undocumented
** @param [r] cl [void*] Undocumented
** @return [void]
** @@
******************************************************************************/

static void  demolist_freegff (void **x, void *cl)
{
    DemolistPgff gffnew;

    (void) cl;				/* make it used */

    gffnew = (DemolistPgff)*x;

    ajStrDel(&gffnew->clone);
    ajStrDel(&gffnew->type);
    ajStrDel(&gffnew->source);
    AJFREE(gffnew);

    return;
}




/* @funcstatic demolist_creategff *********************************************
**
** Not important to understand for demo but this function
** merely passes back a gff struct
**
** @param [r] line [const AjPStr] Undocumented
** @return [DemolistPgff] Undocumented
** @@
******************************************************************************/

static DemolistPgff demolist_creategff(const AjPStr line)
{
    AjPRegexp gffexp = NULL;
    DemolistPgff gffnew = NULL;
    AjPStr temp   = NULL;

    if(!gffexp)
	gffexp =
	    ajRegCompC(
	       "([^\t]+)\t([^\t]+)\t([^\t]+)\t([^\t]+)\t([^\t]+)\t([^\t]+)");

    if(ajRegExec(gffexp,line))
    {
	AJNEW(gffnew);
	gffnew->clone=gffnew->source=gffnew->type = NULL;
	ajRegSubI(gffexp,1,&gffnew->clone);
	ajRegSubI(gffexp,2,&gffnew->source);
	ajRegSubI(gffexp,3,&gffnew->type);
	ajRegSubI(gffexp,4,&temp);
	ajStrToInt(temp,&gffnew->start);
	ajRegSubI(gffexp,5,&temp);
	ajStrToInt(temp,&gffnew->end);
	ajRegSubI(gffexp,6,&temp);
	ajStrToInt(temp,&gffnew->score);
	ajStrDel(&temp);
    }

    ajRegFree(&gffexp);

    return gffnew;
}

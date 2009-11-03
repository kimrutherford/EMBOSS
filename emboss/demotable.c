/* @source demotable application
**
** Demomnstration of how the table functions should be used.
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




static AjPStr demotable_getsubfromstring(const AjPStr line, ajint which);
static void demotable_typePrint (const void* key, void** value, void* cl);
static void demotable_freetype (void** key, void** value, void* cl);


static AjPRegexp gffexp = NULL;


/* @prog demotable ************************************************************
**
** Testing
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPStr temp;
    AjPFile gfffile;
    AjPStr  line = NULL;
    AjPTable type;
    ajint *intptr;
    AjPList list;
    AjPStr savetemp = NULL;
    AjPStr tmpkey = NULL;
    ajint* tmpval;
    AjPStr trukey = NULL;
    AjIList iter;

    embInit("demotable", argc, argv);


    /*open file */
    gfffile = ajAcdGetInfile("infile");

    /*
    **  create new table using ajStrTableCmpCase as the comparison function
    **  and ajStrTableHashCase as the hash function. Initial size of 50
    **  is used
    */
    type   = ajTableNewFunctionLen(50, ajStrTableCmpCase, ajStrTableHashCase);
    list = ajListstrNew();
    while(ajFileReadLine(gfffile, &line))
    {
	temp = demotable_getsubfromstring(line,3); /* get the string to test */

	if(temp)
	{
	    /* does the key "temp" already exist in the table */
	    intptr = ajTableFetch(type, temp);

	    if(!intptr)
	    {				/* if not i.e. no key returned */
		AJNEW(intptr);
		*intptr = 1;
		savetemp = ajStrNewS(temp);
		ajTablePut(type, temp, intptr); /* add it*/
		ajListPush(list, savetemp);
		temp = NULL;
		savetemp = NULL;
	    }
	    else
	    {
		ajStrDel(&temp);
		(*intptr)++;		/* else increment the counter */
	    }
	}
    }
    ajUser("%d types found",ajTableGetLength(type));

    /* use the map function to print out the results */
    ajTableMap(type, demotable_typePrint, NULL);


    /* clean up the table using a list of known keys */

    iter = ajListIterNewread(list);
    while (!ajListIterDone(iter))
    {
	tmpkey = (AjPStr) ajListIterGet(iter);
	tmpval = ajTableRemoveKey(type, tmpkey, (void**)&trukey);
	if(tmpval) {
	    ajUser("Deleting '%S' %d", trukey, *tmpval);
	    ajStrDel(&trukey);
	    AJFREE(tmpval);
	}
    }

    /* Backup plan - use the map function to free all memory */
    /* not needed here because the loop above already removed the entries */
    ajTableMapDel(type, demotable_freetype, NULL);
    ajTableFree(&type);

    ajFileClose(&gfffile);
    ajStrDel(&line);
    ajRegFree(&gffexp);
    ajListIterDel(&iter);
    ajListstrFreeData(&list);

    embExit();
    return 0;
}




/* @funcstatic demotable_getsubfromstring *************************************
**
** Undocumented.
**
** @param [r] line [const AjPStr] Undocumented
** @param [r] which [ajint] Undocumented
** @return [AjPStr] Undocumented
** @@
******************************************************************************/

static AjPStr demotable_getsubfromstring(const AjPStr line, ajint which)
{
    AjPStr temp = NULL;

    if(!gffexp)
	gffexp = ajRegCompC("([^\t]+)\t([^\t]+)\t([^\t]+)");

    if(ajRegExec(gffexp,line))
	ajRegSubI(gffexp,which,&temp);

    return temp;
}




/* @funcstatic demotable_typePrint ********************************************
**
** Undocumented.
**
** @param [r] key [const void*] Undocumented
** @param [r] value [void**] Undocumented
** @param [r] cl [void*] Undocumented
** @return [void]
** @@
******************************************************************************/

static void demotable_typePrint(const void* key, void** value, void* cl)
{
    const AjPStr keystr;
    ajint *valptr;

    (void) cl;

    keystr = (const AjPStr) key;
    valptr = (ajint *) *value;

    ajUser("type '%S' found %d times", keystr, *valptr);

    return;
}




/* @funcstatic demotable_freetype *********************************************
**
** Undocumented.
**
** @param [r] key [void**] Undocumented
** @param [r] value [void**] Undocumented
** @param [r] cl [void*] Undocumented
** @return [void]
** @@
******************************************************************************/

static void demotable_freetype(void** key, void** value, void* cl)
{
    AjPStr keystr;
    ajint *valptr;

    (void) cl;

    keystr = (AjPStr) *key;
    valptr = (ajint *) *value;

    ajStrDel(&keystr);
    AJFREE(valptr);

    *key = NULL;
    *value = NULL;

    return;
}

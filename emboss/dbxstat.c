/* @source dbxstat application
**
** Statistics for a dbx database
**
** @author Copyright (C) 2007 Alan Bleasby (ajb@ebi.ac.uk)
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



/* @datastatic DbxstatNames *************************************************
**
** Index file names
**
** @alias DbxstatSNames
** @alias DbxstatONames
**
** @attr name [const char*] Field name
** @attr iname [const char*] Field index name
** @@
******************************************************************************/

typedef struct DbxflatSNames
{
    const char* name;
    const char* iname;
} DbxflatONames;
#define DbxflatPNames DbxflatONames*




static DbxflatONames inxnames[] =
{
    {"id", "xid"},
    {"ac", "xac"},
    {"sv", "xsv"},
/* To be done if requested
    {"de", "xde"},
    {"kw", "xkw"},
    {"tx", "xtx"},
*/
    {NULL, NULL}
};




/* @prog dbxstat **************************************************************
**
** Statistics for a dbx index
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPStr   dbname = NULL;
    AjPStr   idir   = NULL;
    AjPStr  *itype  = NULL;
    ajint    imin;
    ajint    imax;
    AjPFile  outf = NULL;

    const char *basenam;
    const char *baseidir;
    
    ajint i;

    ajint order;
    ajint nperbucket;
    ajint pagesize;
    ajint level;
    ajint cachesize;
    ajint sorder;
    ajint snperbucket;
    ajint kwlimit;
    ajlong count;

    AjPBtcache cache = NULL;
    
    embInit("dbxstat", argc, argv);
    
    dbname = ajAcdGetString("dbname");
    idir   = ajAcdGetDirectoryName("indexdir");
    itype  = ajAcdGetList("idtype");
    imin   = ajAcdGetInt("minimum");
    imax   = ajAcdGetInt("maximum");
    outf   = ajAcdGetOutfile("outfile");
    
    basenam  = ajStrGetPtr(dbname);
    baseidir = ajStrGetPtr(idir);
    
    i = 0;
    while(inxnames[i].name)
    {
	if(ajStrMatchC(*itype,inxnames[i].name))
	    break;
	++i;
    }
    if(!inxnames[i].name)
	ajFatal("Unrecognised index type");
    

    ajBtreeReadParams(basenam,inxnames[i].iname,baseidir,
		      &order,&nperbucket,&pagesize,&level,
		      &cachesize,&sorder,&snperbucket,
		      &count,&kwlimit);
    
    cache = ajBtreeSecCacheNewC(basenam,inxnames[i].iname,baseidir,"r+",
				pagesize, order, nperbucket, level,
				cachesize, sorder, 0, snperbucket,
				count, kwlimit);
    if(!cache)
	ajFatal("Cannot open index file for reading");
    

    ajBtreeDumpHybKeys(cache,imin,imax,outf);


    ajStrDel(&dbname);
    ajStrDel(&idir);

    ajBtreeCacheDel(&cache);
    ajFileClose(&outf);
    
    embExit();
    
    return 0;
}

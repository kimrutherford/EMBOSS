/* @source dbxcompress application
**
** Compress an uncompressed dbx index
**
** @author Copyright (C) 2011 Peter Rice (pmr@ebi.ac.uk)
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




/* @prog dbxcompress ********************************************************
**
** Compress an uncompressed dbx index
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPStr   dbname = NULL;
    AjPStr   idir   = NULL;
    AjPStr   fieldname  = NULL;
    AjPFile  outf = NULL;

    const AjPStr fieldext;
    AjPBtcache cache = NULL;
    AjBool secondary;
    AjBool compressed;
    ajuint kwlimit;
    ajuint idlimit;
    ajuint refcount;
    ajuint pripagesize;
    ajuint secpagesize;
    ajuint pricachesize;
    ajuint seccachesize;
    ajulong pripagecount;
    ajulong secpagecount;
    ajuint order;
    ajuint nperbucket;
    ajuint level;
    ajuint sorder;
    ajuint snperbucket;
    ajulong count;
    ajulong countall;
    
    embInit("dbxcompress", argc, argv);
    
    dbname = ajAcdGetString("dbname");
    idir   = ajAcdGetDirectoryName("indexdir");
    fieldname  = ajAcdGetString("field");
    outf   = ajAcdGetOutfile("outfile");
    
    if(!ajStrGetLen(idir))
    {
/* if not forced by the user, find the index directory for the database */
        if(!ajNamDbGetIndexdir(dbname, &idir))
            ajDie("Database '%S' has no indexdirectory defined", dbname);
    }

    fieldext = ajBtreeFieldGetExtensionS(fieldname);

    if(!ajBtreeReadParamsS(dbname, fieldext,
                           idir, &secondary, &compressed,
                           &kwlimit, &idlimit, &refcount,
                           &pripagesize, &secpagesize,
                           &pricachesize, &seccachesize,
                           &pripagecount, &secpagecount,
                           &order, &nperbucket,
                           &level, &sorder, &snperbucket, &count, &countall))
    {
        ajDie("Cannot find index file '%S' for database '%S",
              fieldname, dbname);
    }

    if(compressed)
        ajDie("index '%S' for database '%S' is already compressed",
              fieldname, dbname);

    ajFmtPrintF(outf, "Compressing index '%S' for database '%S'\n",
                fieldname, dbname);

    cache = ajBtreeCacheNewUpdateS(dbname,
                                   fieldext,
                                   idir);

    if(!cache)
	ajDie("Cannot open index file '%S' for update for database '%S'",
              fieldname, dbname);
    
    ajFmtPrintF(outf, "Index '%S' opened, compressing\n", fieldname);

    cache->compressed = ajTrue;

    ajBtreeWriteParamsS(cache, dbname, fieldext, idir);

    ajBtreeCacheDel(&cache);    /* close cache and compress index */

    ajFmtPrintF(outf, "Completed\n");

    ajStrDel(&dbname);
    ajStrDel(&fieldname);
    ajStrDel(&idir);

    ajFileClose(&outf);
    
    embExit();
    
    return 0;
}

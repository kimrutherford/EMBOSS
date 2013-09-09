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




/* @prog dbxstat **************************************************************
**
** Statistics for a dbx index
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPStr   dbname = NULL;
    AjPStr   idir   = NULL;
    AjPStr   fieldname  = NULL;
    ajint    imin;
    ajint    imax;
    AjPFile  outf = NULL;
    AjPStr   dbfilename = NULL;

    const AjPStr fieldext;
    AjPBtcache cache = NULL;
    
    embInit("dbxstat", argc, argv);
    
    dbname = ajAcdGetString("dbname");
    idir   = ajAcdGetDirectoryName("indexdir");
    fieldname  = ajAcdGetString("field");
    imin   = ajAcdGetInt("minimum");
    imax   = ajAcdGetInt("maximum");
    outf   = ajAcdGetOutfile("outfile");
    
    if(!ajNamDbGetDbaliasTest(dbname, &dbfilename))
        ajStrAssignS(&dbfilename, dbname);

    if(!ajStrGetLen(idir))
    {
/* if not forced by the user, find the index directory for the database */
        if(!ajNamDbGetIndexdir(dbname, &idir))
            ajDie("Database '%S' has no indexdirectory defined", dbname);
    }
    fieldext = ajBtreeFieldGetExtensionS(fieldname);

    cache = ajBtreeCacheNewReadS(dbfilename, fieldext, idir);

    if(!cache)
	ajFatal("Cannot open index file '%S' for reading",
                fieldname);
    

    ajBtreeDumpKeywords(cache,imin,imax,outf);

    ajStrDel(&dbfilename);

    ajStrDel(&dbname);
    ajStrDel(&fieldname);
    ajStrDel(&idir);

    ajBtreeCacheDel(&cache);

    ajFileClose(&outf);
    
    embExit();
    
    return 0;
}

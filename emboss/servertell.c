/** @source servertell ********************************************************
**
** Display information about a single EMBOSS server
**
** @author Copyright (C) 2011 Peter Rice / EMBOSS
** @version 1  First version
** @modified April 2011  Peter Rice  First version
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
********************************************************************/


/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "emboss.h"

/* Inclusion of system and local header files goes here */



/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */

/* #define and enum statements go here */



/* ==================================================================== */
/* ======================== global variables ========================== */
/* ==================================================================== */

/* Global variables definitions go here */



/* ==================================================================== */
/* ============================== data ================================ */
/* ==================================================================== */

/* Definition of datatypes go here */



/* ==================================================================== */
/* ==================== function prototypes =========================== */
/* ==================================================================== */

/* Function prototypes for public (external) functions go here */




/* @prog servertell ***********************************************************
**
** Display information about a public server
**
******************************************************************************/

int main(int argc, char **argv)
{
    /* Variable declarations */
    AjPStr   svrname = NULL;
    AjPFile  outfile = NULL;    
    AjPStr type     = NULL;
    AjBool id;
    AjBool qry;
    AjBool all;
    AjPStr scope = NULL;
    AjPStr methods = NULL;
    AjPStr release = NULL;
    AjPStr comment = NULL;
    AjPStr defined = NULL;
    AjPStr cachedir = NULL;
    AjPStr cachefile = NULL;
    AjPStr url = NULL;
    AjPList list = NULL;
    AjPTagval tagval = NULL;
    AjIList iter = NULL;
    ajuint space = 0;
    ajuint count = 0;

    /* ACD processing */
    embInit("servertell", argc, argv);

    svrname   = ajAcdGetString("server");
/*    verbose  = ajAcdGetBoolean("full"); */
    outfile  = ajAcdGetOutfile("outfile");
    

    /* Application logic */
    /* Check EMBOSS server information.
       Write output file */

    count = ajNamSvrCount(svrname);
    if(ajNamSvrDetails(svrname, &type, &scope, &id, &qry, &all, &comment,
                       &release, &methods, &defined,
                       &cachedir, &cachefile, &url))
    {
        ajFmtPrintF(outfile, "# %S is defined in %S\n", svrname, defined);
        ajFmtPrintF(outfile, "# access levels id: %B query: %B all: %B\n",
                    id, qry, all);
        ajFmtPrintF(outfile, "# scope: %S\n", scope);
        if(ajStrGetLen(cachedir))
           ajFmtPrintF(outfile, "# cache directory: %S file: %S\n",
                       cachedir, cachefile);
        else
           ajFmtPrintF(outfile, "# cache file: %S\n",
                       cachefile);
        ajFmtPrintF(outfile, "# databases: %u\n\n", count);
        ajFmtPrintF(outfile, "SERVER %S [\n", svrname);

        list = ajNamSvrGetAttrlist(svrname);
        iter = ajListIterNewread(list);
        while(!ajListIterDone(iter))
        {
            tagval = ajListIterGet(iter);
            space = 15 - ajStrGetLen(ajTagvalGetTag(tagval));
            ajFmtPrintF(outfile, "   %S:%.*s\"%S\"\n",
                        ajTagvalGetTag(tagval),
                        space, "                    ",
                        ajTagvalGetValue(tagval));
            ajTagvalDel(&tagval);
        }
        ajListIterDel(&iter);
        ajListFree(&list);

        ajFmtPrintF(outfile, "]\n");

        ajStrDel(&type);
        ajStrDel(&methods);
        ajStrDel(&release);
        ajStrDel(&comment);
        ajStrDel(&defined);
        ajStrDel(&scope);
        ajStrDel(&cachedir);
        ajStrDel(&cachefile);
        ajStrDel(&url);
    }

    /* Memory clean-up and exit */
    ajFileClose(&outfile);
    ajStrDel(&svrname);

    embExit();

    return 0;
}


/* ==================================================================== */
/* ============================ functions ============================= */
/* ==================================================================== */


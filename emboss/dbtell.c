/** @source dbtell
**
** Display information about a single EMBOSS database
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




/* @prog dbtell ***************************************************************
**
** Display information about a public database
**
******************************************************************************/

int main(int argc, char **argv)
{
    /* Variable declarations */
    AjPStr   dbname = NULL;
    AjPFile  outfile = NULL;    
    AjPResource resource = NULL;
    AjPResourcein resourcein = NULL;
    AjPStr resourceqry = NULL;
    AjPStr type     = NULL;
    AjBool id;
    AjBool qry;
    AjBool all;
    AjPStr methods = NULL;
    AjPStr release = NULL;
    AjPStr comment = NULL;
    AjPStr defined = NULL;
    AjPList list = NULL;
    AjPTagval tagval = NULL;
    AjIList iter = NULL;
    ajuint space = 0;

    /* ACD processing */
    embInit("dbtell", argc, argv);

    dbname   = ajAcdGetString("database");
/*    verbose  = ajAcdGetBoolean("full"); */
    outfile  = ajAcdGetOutfile("outfile");
    

    /* Application logic */
    /* Check EMBOSS database information.
       Write output file */

    if(ajNamDbDetails(dbname, &type, &id, &qry, &all, &comment,
                      &release, &methods, &defined))
    {
        ajFmtPrintF(outfile, "# %S is defined in %S\n", dbname, defined);
        ajFmtPrintF(outfile, "# access levels id: %B query: %B all: %B\n\n",
                    id, qry, all);
        ajFmtPrintF(outfile, "DBNAME %S [\n", dbname);

        list = ajNamDbGetAttrlist(dbname);
        iter = ajListIterNewread(list);
        while(!ajListIterDone(iter))
        {
            tagval = ajListIterGet(iter);
            space = 15 - ajStrGetLen(tagval->Tag);
            ajFmtPrintF(outfile, "   %S:%.*s\"%S\"\n",
                        tagval->Tag,
                        space, "                    ",
                        tagval->Value);
            ajStrDel(&tagval->Tag);
            ajStrDel(&tagval->Value);
            AJFREE(tagval);
        }
        ajListIterDel(&iter);
        ajListFree(&list);

        ajFmtPrintF(outfile, "]\n");

        ajStrDel(&type);
        ajStrDel(&methods);
        ajStrDel(&release);
        ajStrDel(&comment);
        ajStrDel(&defined);
    }
    else
    {
/* try looking in DRCAT */
        resourcein = ajResourceinNew();
        resource = ajResourceNew();

        ajFmtPrintS(&resourceqry, "drcat:%S", dbname);
        ajResourceinQryS(resourcein, resourceqry);
        if(ajResourceinRead(resourcein, resource))
        {
            ajFmtPrintF(outfile, "DBNAME %S [\n", dbname);
            ajFmtPrintF(outfile, "   comment: \"defined in DRCAT\"\n");
            ajFmtPrintF(outfile, "]\n");
         }
        ajResourceinDel(&resourcein);
        ajResourceDel(&resource);
        ajStrDel(&resourceqry);
    }

    /* Memory clean-up and exit */

    ajFileClose(&outfile);
    ajStrDel(&dbname);

    embExit();

    return 0;
}


/* ==================================================================== */
/* ============================ functions ============================= */
/* ==================================================================== */


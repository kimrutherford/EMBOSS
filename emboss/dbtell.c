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
    AjPStr   svrname = NULL;
    AjPFile  outfile = NULL;    
    AjPResource resource = NULL;
    AjPResourcein resourcein = NULL;
    AjPStr resourceqry = NULL;
    AjPStr type     = NULL;
    AjBool id;
    AjBool qry;
    AjBool all;
    AjBool verbose;
    AjPStr methods = NULL;
    AjPStr release = NULL;
    AjPStr comment = NULL;
    AjPStr defined = NULL;
    AjPList list = NULL;
    AjPTagval tagval = NULL;
    AjIList iter = NULL;
    ajuint space = 0;
    AjPList aliaslist = NULL;
    AjIList aliter = NULL;
    const AjPStr alias = NULL;
    ajuint maxlen;
    AjPStr truedbname = NULL;
    AjBool isalias = ajFalse;

    /* ACD processing */
    embInit("dbtell", argc, argv);

    dbname   = ajAcdGetString("database");
    svrname   = ajAcdGetString("server");
    verbose  = ajAcdGetBoolean("full");
    outfile  = ajAcdGetOutfile("outfile");
    
    ajStrAssignS(&truedbname, dbname);
    ajNamAliasDatabase(&truedbname);
    ajStrFmtLower(&truedbname);

    if(!ajStrMatchS(dbname, truedbname))
        isalias = ajTrue;

    /* Application logic */
    /* Check EMBOSS database information.
       Write output file */

    if(ajNamDbDetailsSvr(truedbname, svrname, &type, &id, &qry, &all,
                         &comment, &release, &methods, &defined))
    {
        if(isalias)
            ajFmtPrintF(outfile, "# %S is an alias for %S defined in %S\n",
                        dbname, truedbname, defined);
        else
            ajFmtPrintF(outfile, "# %S is defined in %S\n",
                        truedbname, defined);

        ajFmtPrintF(outfile, "# access levels id: %B query: %B all: %B\n\n",
                    id, qry, all);
        ajFmtPrintF(outfile, "DBNAME %S [\n", truedbname);

        if(ajStrGetLen(svrname))
            list = ajNamDbGetAttrlistSvr(truedbname, svrname);
        else
            list = ajNamDbGetAttrlist(truedbname);

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

        if(verbose)
        {
            aliaslist = ajListNew();
            ajNamListFindAliases(truedbname, aliaslist);

            if(ajListGetLength(aliaslist))
            {
                ajFmtPrintF(outfile, "\n");
                    
                aliter = ajListIterNewread(aliaslist);

                maxlen = 1;
                while(!ajListIterDone(aliter))
                {
                    alias = ajListIterGet(aliter);
                    if(MAJSTRGETLEN(alias) > maxlen)
                        maxlen = MAJSTRGETLEN(alias);
                }
                
                ajListIterDel(&aliter);
                aliter = ajListIterNewread(aliaslist);

                while(!ajListIterDone(aliter))
                {
                    alias = ajListIterGet(aliter);

                    if(ajStrFindK(alias, ':') < 0)
                        ajFmtPrintF(outfile, "ALIAS %-*S %S\n",
                                    maxlen, alias, truedbname);
                }

                ajListIterDel(&aliter);
            }

            ajListstrFree(&aliaslist);
        }

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
    ajStrDel(&truedbname);
    ajStrDel(&dbname);
    ajStrDel(&svrname);

    embExit();

    return 0;
}


/* ==================================================================== */
/* ============================ functions ============================= */
/* ==================================================================== */


/** @source ontogetcommon
**
** Find common ancestor or a term
**
** @author Copyright (C) 2010 Jon Ison / EMBOSS
** @version 1  First version</replaceable>
** @modified July 2010  Jon Ison First version</replaceable>
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




/* @prog ontogetcommon ********************************************************
**
** Find common ancestor of a term
**
******************************************************************************/

int main(int argc, char **argv)
{
    /* Variable declarations */
    AjPOutfile  outfile = NULL;    
    AjBool obsolete = ajTrue;

    AjPObo obo = NULL;
    AjPObo oboanc = NULL;
    AjPObo oboparent = NULL;
    AjPOboall oboall = NULL;
    AjPOboin oboin = NULL;

    AjPStr oboqryup = NULL;
    AjPStr oboqryanc = NULL;
    AjPTable alltable = NULL;
    AjPTable newtable = NULL;

    AjPStr up = NULL;
    AjPList uplist = NULL;

    ajuint iterms = 0;
    AjPStr topid = NULL;
    AjPStr obodb = NULL;
    AjBool saved = ajFalse;

   /* ACD processing */
    embInit("ontogetcommon", argc, argv);

    oboall   = ajAcdGetOboall("oboterms");
    outfile  = ajAcdGetOutobo("outfile");
    obsolete = ajAcdGetBoolean("obsolete");

    oboin = ajOboinNew();
    oboparent = ajOboNew();
    oboanc = ajOboNew();
    uplist = ajListNew();

    alltable = ajTablestrNew(600);
    newtable = ajTablestrNew(600);

    while(ajOboallNext(oboall, &obo))
    {
        saved = ajFalse;
        if(!obsolete && ajOboIsObsolete(obo))
            continue;

        if(!iterms)
        {
            ajDebug("store id '%S'\n", ajOboGetId(obo));
            ajStrAssignS(&obodb, ajOboGetDb(obo));
            ajTablePut(alltable, ajStrNewS(ajOboGetId(obo)), NULL);
            saved = ajTrue;
        }
        else 
        {
            ajDebug("test id '%S'\n", ajOboGetId(obo));
            if(ajTableMatchS(alltable, ajOboGetId(obo)))
            {
                ajDebug("keep id '%S'\n", ajOboGetId(obo));
                ajTablePut(newtable, ajStrNewS(ajOboGetId(obo)), NULL);
                saved = ajTrue;
            }
        }

        if(saved)
            ajStrAssignS(&topid, ajOboGetId(obo));
        
        if(ajOboGetParents(obo, uplist)) /* that was the root */
        {
            while(ajListstrPop(uplist, &up))
            {
                ajDebug("up: '%S'\n", up);
                ajFmtPrintS(&oboqryup, "%S-id:%S", ajOboGetDb(obo), up);
                ajOboinQryS(oboin, oboqryup);
                while(ajOboinRead(oboin, oboparent))
                {
                    if(!obsolete && ajOboIsObsolete(oboparent))
                        continue;

                    if(!iterms)
                    {
                        ajDebug("store parent '%S'\n", ajOboGetId(oboparent));
                        ajTablePut(alltable, ajStrNewS(ajOboGetId(oboparent)),
                                   NULL);
                    }
                    else 
                    {
                        ajDebug("test parent '%S'\n", ajOboGetId(oboparent));
                        if(ajTableMatchS(alltable, ajOboGetId(oboparent)))
                        {
                            ajDebug("keep parent '%S'\n",
                                    ajOboGetId(oboparent));
                            ajTablePut(newtable,
                                       ajStrNewS(ajOboGetId(oboparent)),
                                       NULL);
                            if(!saved)
                            {
                                ajStrAssignS(&topid, ajOboGetId(oboparent));
                                saved = ajTrue;
                            }
                        }
                    }
                    if(!ajOboGetParents(oboparent, uplist))
                        continue;
                }
                ajStrDel(&up);
            }
        }
        
        if(iterms)
            ajTableMergeAnd(alltable, newtable);

        ajDebug("id: '%S' saved %u\n",
               ajOboGetId(obo),
               ajTableGetLength(alltable));

        if(!ajTableGetLength(alltable))
            ajDie("Query '%S' no matching ancestor found for obo term '%S:%S'",
                  ajOboallGetQryS(oboall), ajOboGetDb(obo), ajOboGetId(obo));
        iterms++;
    }

    ajFmtPrintS(&oboqryanc, "%S-id:%S", obodb, topid);
    ajOboinQryS(oboin, oboqryanc);
    while(ajOboinRead(oboin, oboanc))
    {
        ajObooutWrite(outfile, oboanc);
    }


    /* Memory clean-up and exit */

    ajOboallDel(&oboall);
    ajOboinDel(&oboin);
    ajOboDel(&oboanc);
    ajOboDel(&oboparent);

    ajListFree(&uplist);

    ajTablestrFreeKey(&alltable);
    ajTablestrFreeKey(&newtable);

    ajStrDel(&oboqryup);
    ajStrDel(&oboqryanc);
    ajStrDel(&obodb);
    ajStrDel(&topid);

    ajOutfileClose(&outfile);
    
    embExit();

    return 0;
}


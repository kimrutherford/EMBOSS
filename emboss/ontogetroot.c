/** @source ontogetroot
**
** Find ontology terms by child identifier
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




/* @prog ontogetroot **********************************************************
**
** Find EDAM ontology root term
**
******************************************************************************/

int main(int argc, char **argv)
{
    /* Variable declarations */
    AjPOutfile  outfile = NULL;    
    AjBool obsolete = ajTrue;

    AjPObo obo = NULL;
    AjPObo oboparent = NULL;
    AjPObo oboroot = NULL;
    AjPOboall oboall = NULL;
    AjPOboin oboinparent = NULL;

    AjPStr oboqryup = NULL;
    AjPTable foundtable = NULL;

    AjPStr up = NULL;
    AjPList uplist = NULL;
    AjPList rootlist = NULL;

    ajuint ifound = 0;
    ajuint ikeep  = 0;

    /* ACD processing */
    embInit("ontogetroot", argc, argv);

    oboall   = ajAcdGetOboall("oboterms");
    outfile  = ajAcdGetOutobo("outfile");
    obsolete = ajAcdGetBoolean("obsolete");
    
    oboinparent = ajOboinNew();
    oboparent = ajOboNew();
    uplist = ajListNew();
    rootlist = ajListNew();

    foundtable = ajTablestrNew(600);

    while(ajOboallNext(oboall, &obo))
    {
        if(!obsolete && ajOboIsObsolete(obo))
            continue;

        ifound++;

        if(!ajOboGetParents(obo, uplist)) /* that was the root */
        {
            ajListPushAppend(rootlist, ajOboNewObo(obo));
            continue;
        }

        while(ajListstrPop(uplist, &up))
        {
            ajFmtPrintS(&oboqryup, "%S-id:%S", ajOboGetDb(obo), up);
            ajOboinQryS(oboinparent, oboqryup);
            while(ajOboinRead(oboinparent, oboparent))
            {
                if(!obsolete && ajOboIsObsolete(oboparent))
                    continue;

                if(!ajOboGetParents(oboparent, uplist))
                {
                    ajListPushAppend(rootlist, ajOboNewObo(oboparent));
                    continue;
                }
            }
            ajStrDel(&up);
        }
    }

    while(ajListPop(rootlist, (void**) &oboroot))
    {
        if(!ajTableMatchS(foundtable, oboroot->Id))
        {
            ajObooutWrite(outfile, oboroot);
            ajTablePut(foundtable,
                       ajStrNewS(oboroot->Id),
                       (void *) 1);
            ikeep++;
        }
        ajOboDel(&oboroot);
    }

    if(!ifound)
        ajErr("No matching terms");

    /* Memory clean-up and exit */

    ajOboallDel(&oboall);
    ajOboinDel(&oboinparent);
    ajOboDel(&obo);
    ajOboDel(&oboparent);

    ajListFree(&uplist);
    ajListFree(&rootlist);

    ajTablestrFreeKey(&foundtable);

    ajStrDel(&oboqryup);

    ajOutfileClose(&outfile);
    
    embExit();

    return 0;
}


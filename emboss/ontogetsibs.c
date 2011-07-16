/** @source ontogetsibs
**
** Find ontology terms by common parent
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




/* @prog ontogetsibs **********************************************************
**
** Find ontology terms by common parent
**
******************************************************************************/

int main(int argc, char **argv)
{
    /* Variable declarations */
    AjPOutfile  outfile = NULL;    
    AjBool obsolete = ajTrue;

    AjPObo obo = NULL;
    AjPObo oboparent = NULL;
    AjPOboall oboall = NULL;
    AjPObo obochild = NULL;
    AjPOboin oboinparent = NULL;
    AjPOboin oboinchild = NULL;

    AjPStr oboqryup = NULL;
    AjPStr obochildqry = NULL;
    AjPStr qrystr = NULL;
    AjPTable obotable = NULL;

    AjPStrTok handle = NULL;

    AjPStr up = NULL;
    AjPList uplist = NULL;

    ajuint ifound = 0;
    ajuint ikeep = 0;

    /* ACD processing */
    embInit("ontogetsibs", argc, argv);

    oboall    = ajAcdGetOboall("oboterms");
    outfile   = ajAcdGetOutobo("outfile");
    obsolete = ajAcdGetBoolean("obsolete");
    
    oboinparent = ajOboinNew();
    oboparent = ajOboNew();

    oboinchild = ajOboinNew();
    obochild = ajOboNew();

    uplist = ajListNew();

    obotable = ajTablestrNew(600);

    while(ajOboallNext(oboall, &obo))
    {
        if(!obsolete && ajOboIsObsolete(obo))
            continue;

        ifound++;

        ajOboGetParents(obo, uplist);

        while(ajListstrPop(uplist, &up))
        {
            ajFmtPrintS(&oboqryup, "%S-id:%S", ajOboGetDb(obo), up);
            ajOboinQryS(oboinparent, oboqryup);
            while(ajOboinRead(oboinparent, oboparent))
            {
                if(!obsolete && ajOboIsObsolete(oboparent))
                    continue;

                ajFmtPrintS(&obochildqry, "%S-isa:%S",
                            ajOboGetDb(obo), oboparent->Id);
                ajOboinQryS(oboinchild, obochildqry);
                while(ajOboinRead(oboinchild, obochild))
                {
                    if(!ajTableMatchS(obotable, obochild->Id))
                    {
                        ajObooutWrite(outfile, obochild);
                        ajTablePut(obotable, ajStrNewS(obochild->Id),
                                   (void *) 1);
                        ikeep++;
                    }
                }
            }
            ajStrDel(&up);
        }
    }

    if(!ifound)
        ajErr("No matching terms");

    /* Memory clean-up and exit */

    ajOboallDel(&oboall);
    ajOboinDel(&oboinchild);
    ajOboinDel(&oboinparent);

    ajOboDel(&obo);
    ajOboDel(&obochild);
    ajOboDel(&oboparent);

    ajStrTokenDel(&handle);

    ajStrDel(&qrystr);
    ajStrDel(&oboqryup);
    ajStrDel(&obochildqry);

    ajListFree(&uplist);

    ajTablestrFreeKey(&obotable);

    ajOutfileClose(&outfile);
    
    embExit();

    return 0;
}


/* ==================================================================== */
/* ============================ functions ============================= */
/* ==================================================================== */


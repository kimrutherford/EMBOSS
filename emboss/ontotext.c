/** @source ontotext
**
** Get ontology term(s) original text entries
**
** @author Copyright (C) 2011 Peter Rice / EMBOSS
** @version 1  First version</replaceable>
** @modified May 2011  Peter Rice First version</replaceable>
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




/* @prog ontotext *************************************************************
**
** Get ontology term(s) complete text
**
******************************************************************************/

int main(int argc, char **argv)
{
    /* Variable declarations */
    AjPFile outf = NULL;    
    AjPOboall oboall = NULL;
    AjBool subclasses = ajFalse;
    AjBool obsolete = ajFalse;

    AjPObo obo     = NULL;
    AjPObo obotest = NULL;

    AjPList obolist = ajListNew();

    AjPTable foundtable = NULL;
    ajuint ifound = 0;
    ajuint ikeep = 0;

    /* ACD processing */
    embInit("ontotext", argc, argv);

    oboall    = ajAcdGetOboall("oboterms");
    outf   = ajAcdGetOutfile("outfile");
    subclasses = ajAcdGetBoolean("subclasses");
    obsolete = ajAcdGetBoolean("obsolete");
    
    foundtable = ajTablestrNew(600);

    while(ajOboallNext(oboall, &obo))
    {
        ajListPushAppend(obolist, ajOboNewObo(obo));
        if(subclasses)
            ajOboGetTree(obo, obolist);

        while(ajListGetLength(obolist))
        {
            ajListPop(obolist, (void**) &obotest);
            if(!obsolete && ajOboIsObsolete(obotest))
            {
                ajOboDel(&obotest);
                continue;
            }

            ifound++;
            if(!ajTableMatchS(foundtable, obotest->Id))
            {
                ajFmtPrintF(outf,"%S",ajOboGetEntry(obotest));
                ajTablePut(foundtable, ajStrNewS(obotest->Id),
                           (void *) 1);
                ikeep++;
            }
            ajOboDel(&obotest);
        }
    }

    if(!ifound)
        ajErr("No matching terms");

    /* Memory clean-up and exit */

    ajListFree(&obolist);

    ajOboallDel(&oboall);
    ajOboDel(&obo);

    ajFileClose(&outf);
    ajTablestrFreeKey(&foundtable);
   
    embExit();

    return 0;
}


/* ==================================================================== */
/* ============================ functions ============================= */
/* ==================================================================== */


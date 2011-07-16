/** @source ontogetdown
**
** Find ontology terms by parent identifier
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




/* @prog ontogetdown **********************************************************
**
** Find EDAM ontology 'data' (identifier) terms by keyword
**
******************************************************************************/

int main(int argc, char **argv)
{
    /* Variable declarations */
    AjPOboall   oboall = NULL;
    AjPOutfile  outfile = NULL;    
    AjBool obsolete = ajFalse;

    AjPObo obo = NULL;
    AjPObo obochild = NULL;
    AjPOboin obochildin = NULL;
    AjPStr obochildqry = NULL;
    AjPTable obotable = NULL;

    ajuint ifound = 0;
    ajuint ichild = 0;
    ajuint ikeep = 0;

    AjPList obolist = NULL;

    /* ACD processing */
    embInit("ontogetdown", argc, argv);

    oboall    = ajAcdGetOboall("oboterms");
    outfile   = ajAcdGetOutobo("outfile");
    obsolete = ajAcdGetBoolean("obsolete");
    
    /* Application logic */
    /* Check EDAM ontology (edam.obo) is installed indexed.
       Loop through queryable fields
        :Return list of EDAM ids with fields matching keyword(s)
       Merge lists of matching entries
       Write output file */

    obochildin = ajOboinNew();

    obochild = ajOboNew();

    obolist = ajListNew();

    obotable = ajTablestrNew(600);

    while(ajOboallNext(oboall, &obo))
    {
        if(!obsolete && ajOboIsObsolete(obo))
            continue;

        ifound++;

        ajFmtPrintS(&obochildqry, "%S-isa:%S", ajOboGetDb(obo), obo->Id);
        ajOboinQryS(obochildin, obochildqry);
        while(ajOboinRead(obochildin, obochild))
        {
            ichild++;
            if(!ajTableMatchS(obotable, obochild->Id))
            {
                ajObooutWrite(outfile, obochild);
                ajTablePut(obotable, ajStrNewS(obochild->Id),
                           (void *) 1);
                ikeep++;
            }
        }
    }

    if(!ifound)
        ajErr("No matching terms");
    else if(!ichild)
        ajErr("No child terms");

    /* Memory clean-up and exit */

    ajStrDel(&obochildqry);
    ajTablestrFreeKey(&obotable);

    ajOboallDel(&oboall);
    ajOboinDel(&obochildin);

    ajOboDel(&obo);
    ajOboDel(&obochild);

    ajListFree(&obolist);

    ajOutfileClose(&outfile);
    
    embExit();

    return 0;
}


/* ==================================================================== */
/* ============================ functions ============================= */
/* ==================================================================== */


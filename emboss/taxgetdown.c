/** @source taxgetdown
**
** Get taxon(s)
**
** @author Copyright (C) 2011 Peter Rice / EMBOSS
** @version 1  First version</replaceable>
** @modified January 2011  Peter Rice First version</replaceable>
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




/* @prog taxgetdown ***********************************************************
**
** Get next rank below taxon(s)
**
******************************************************************************/

int main(int argc, char **argv)
{
    /* Variable declarations */
    AjPOutfile outfile = NULL;    
    AjPTaxall taxall = NULL;

    AjPTax tax     = NULL;

    AjPList taxlist = ajListNew();

    AjPTax taxchild = NULL;
    AjPTaxin taxchildin = NULL;
    AjPStr taxchildqry = NULL;
    AjPTable foundtable = NULL;
    ajuint ifound = 0;
    ajuint ichild = 0;
    ajuint ikeep = 0;

    /* ACD processing */
    embInit("taxgetdown", argc, argv);

    taxall    = ajAcdGetTaxonall("taxons");
    outfile   = ajAcdGetOuttaxon("outfile");

    taxchildin = ajTaxinNew();

    taxchild = ajTaxNew();

    foundtable = ajTablestrNew(600);

    while(ajTaxallNext(taxall, &tax))
    {
        ifound++;
        ajFmtPrintS(&taxchildqry, "%S-up:%S",
                    ajTaxGetDb(tax), ajTaxGetId(tax));
        ajTaxinQryS(taxchildin, taxchildqry);
        while(ajTaxinRead(taxchildin, taxchild))
        {
            ichild++;
            if(!ajTableMatchS(foundtable, taxchild->Id))
            {
                ajTaxoutWrite(outfile, taxchild);
                ajTablePut(foundtable, ajStrNewS(taxchild->Id),
                           (void *) 1);
                ikeep++;
            }
        }
    }

    if(!ifound)
        ajErr("No matching terms");

    /* Memory clean-up and exit */

    ajStrDel(&taxchildqry);

    ajListFree(&taxlist);

    ajTaxallDel(&taxall);
    ajTaxDel(&tax);
    ajTaxinDel(&taxchildin);
    ajTaxDel(&taxchild);

    ajOutfileClose(&outfile);

    ajTablestrFreeKey(&foundtable);
   
    embExit();

    return 0;
}


/* ==================================================================== */
/* ============================ functions ============================= */
/* ==================================================================== */


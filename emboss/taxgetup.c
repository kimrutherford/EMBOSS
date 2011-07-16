/** @source taxgetup
**
** Find taxons at higher ranks
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




/* @prog taxgetup ************************************************************
**
** Find NCBI taxons above search term
**
******************************************************************************/

int main(int argc, char **argv)
{
    /* Variable declarations */
    AjPOutfile  outfile = NULL;    
    AjBool hidden = ajFalse;

    AjPTax tax = NULL;
    AjPTax taxparent = NULL;
    AjPTaxall taxall = NULL;
    AjPTaxin taxinparent = NULL;

    AjPStr taxqryup = NULL;
    AjPTable foundtable = NULL;

    ajuint up;
    AjPList uplist = NULL;

    /* ACD processing */
    embInit("taxgetup", argc, argv);

    taxall   = ajAcdGetTaxonall("taxons");
    outfile  = ajAcdGetOuttaxon("outfile");
    hidden = ajAcdGetBoolean("hidden");
    
    taxinparent = ajTaxinNew();
    taxparent = ajTaxNew();
    uplist = ajListNew();

    foundtable = ajTablestrNew(600);

    while(ajTaxallNext(taxall, &tax))
    {
        up = ajTaxGetParent(tax);
        while (up > 1)
        {
            ajFmtPrintS(&taxqryup, "%S-id:%u", ajTaxGetDb(tax), up);
            ajTaxinQryS(taxinparent, taxqryup);

            if(!ajTaxinRead(taxinparent, taxparent))
                break;

            if(hidden || !ajTaxIsHidden(taxparent))
            {
                if(!ajTableMatchS(foundtable, taxparent->Id))
                {
                    ajTaxoutWrite(outfile, taxparent);
                    ajTablePut(foundtable,
                               ajStrNewS(taxparent->Id),
                               (void *) 1);
                }
            }

            up = ajTaxGetParent(taxparent);
        }
    }

    /* Memory clean-up and exit */

    ajTaxallDel(&taxall);
    ajTaxinDel(&taxinparent);
    ajTaxDel(&tax);
    ajTaxDel(&taxparent);

    ajListFree(&uplist);

    ajTablestrFreeKey(&foundtable);

    ajStrDel(&taxqryup);

    ajOutfileClose(&outfile);
    
    embExit();

    return 0;
}


/* ==================================================================== */
/* ============================ functions ============================= */
/* ==================================================================== */


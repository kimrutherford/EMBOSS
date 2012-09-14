/** @source variationget
**
** Get sequence variations
**
** @author Copyright (C) 2011 Peter Rice / EMBOSS
** @version $Revision: 1.3 $
** @modified $Date: 2011/12/19 16:33:01 $ by $Author: rice $
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




/* @prog variationget *********************************************************
**
** Get sequence variations
**
******************************************************************************/

int main(int argc, char **argv)
{
    /* Variable declarations */
    AjPOutfile outfile = NULL;    
    AjPVarload varload = NULL;
    AjPVar variation = NULL;

    /* ACD processing */
    embInit("variationget", argc, argv);

    varload   = ajAcdGetVariation("variation");
    outfile   = ajAcdGetOutvariation("outfile");
   
    while(ajVarloadNext(varload, &variation))
    {
        while(ajVarloadMore(varload, &variation))
        {
            ajVaroutWriteNext(outfile, variation);
        }
        ajOutfileReset(outfile);
    }

    /* Memory clean-up and exit */

    ajVarloadDel(&varload);
    ajVarDel(&variation);

    ajOutfileClose(&outfile);
   
    embExit();

    return 0;
}


/* ==================================================================== */
/* ============================ functions ============================= */
/* ==================================================================== */


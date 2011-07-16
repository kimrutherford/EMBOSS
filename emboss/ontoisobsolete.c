/** @source ontoisobsolete
**
** Test ontology term is obsolete
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




/* @prog ontoisobsolete *******************************************************
**
** Test whether ontology term is obsolete
**
******************************************************************************/

int main(int argc, char **argv)
{
    /* Variable declarations */
    AjPFile outf   = NULL;    
    AjPObo obo     = NULL;

    /* ACD processing */
    embInit("ontoisobsolete", argc, argv);

    obo       = ajAcdGetObo("oboterms");
    outf      = ajAcdGetOutfile("outfile");
    
    if(ajOboIsObsolete(obo))
        ajFmtPrintF(outf, "Obsolete\n");
    else
        ajFmtPrintF(outf, "%S:%S OK\n", ajOboGetDb(obo), ajOboGetId(obo));

    /* Memory clean-up and exit */

    ajOboDel(&obo);
    ajFileClose(&outf);
   
    embExit();

    return 0;
}


/* ==================================================================== */
/* ============================ functions ============================= */
/* ==================================================================== */


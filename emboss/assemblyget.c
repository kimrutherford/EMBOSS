/** @source assemblyget
**
** Get assembly of sequence reads
**
** @author Copyright (C) 2011 Peter Rice / EMBOSS
** @version $Revision: 1.4 $
** @modified $Date: 2012/03/13 13:27:40 $ by $Author: rice $
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




/* @prog assemblyget **********************************************************
**
** Get assembly of sequence reads
**
******************************************************************************/

int main(int argc, char **argv)
{
    /* Variable declarations */
    AjPOutfile outfile = NULL;    
    AjPAssemload assemload = NULL;
    AjPAssem assembly = NULL;

    /* ACD processing */
    embInit("assemblyget", argc, argv);

    assemload = ajAcdGetAssembly("assembly");
    outfile   = ajAcdGetOutassembly("outassembly");


    while(ajAssemloadMore(assemload, &assembly))
    {
        ajAssemoutWriteNext(outfile, assembly);
    }

    ajOutfileReset(outfile);

    /* Memory clean-up and exit */

    ajAssemDel(&assembly);
    ajAssemloadDel(&assemload);

    ajOutfileClose(&outfile);
   
    embExit();

    return 0;
}


/* ==================================================================== */
/* ============================ functions ============================= */
/* ==================================================================== */


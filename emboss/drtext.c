/** @source drtext
**
** Get data resource entries original text
**
** @author Copyright (C) 2011 Peter Rice / EMBOSS
** @version 1  First version</replaceable>
** @modified May 2011 Peter Rice First version</replaceable>
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




/* @prog drtext ***************************************************************
**
** Get complete text of data resource entries
**
******************************************************************************/

int main(int argc, char **argv)
{
    /* Variable declarations */
    AjPFile outfile = NULL;    
    AjPResourceall resourceall = NULL;

    AjPResource resource = NULL;

    /* ACD processing */
    embInit("drtext", argc, argv);

    resourceall = ajAcdGetResourceall("resources");
    outfile   = ajAcdGetOutfile("outfile");
    
    while(ajResourceallNext(resourceall, &resource))
    {
        ajFmtPrintF(outfile,"%S",ajResourceGetEntry(resource));
    }

    ajResourceallDel(&resourceall);
    ajResourceDel(&resource);
    ajFileClose(&outfile);
   
    embExit();

    return 0;
}


/* ==================================================================== */
/* ============================ functions ============================= */
/* ==================================================================== */


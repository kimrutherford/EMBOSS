/* @source embinit ************************************************************
**
** General routines for initialisation
**
** @author Copyright (c) 1999 Alan Bleasby
** @version $Revision: 1.21 $
** @modified $Date: 2011/10/18 14:24:24 $ by $Author: rice $
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public
** License along with this library; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA  02110-1301,  USA.
******************************************************************************/

#include "embinit.h"
#include "ajassemdb.h"
#include "ajfeatdb.h"
#include "ajobodb.h"
#include "ajrefseqdb.h"
#include "ajresourcedb.h"
#include "ajseqdb.h"
#include "ajtaxdb.h"
#include "ajtextdb.h"
#include "ajurldb.h"
#include "ajvardb.h"
#include "ajacd.h"
#include "ajnam.h"

static void initDball(void);




/* @funcstatic initDball ******************************************************
**
** Initialises database access functions for all datatypes
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void initDball (void)
{
    ajTextdbInit();
    ajSeqdbInit();
    ajFeatdbInit();
    ajObodbInit();
    ajAssemdbInit();
    ajRefseqdbInit();
    ajTaxdbInit();
    ajUrldbInit();
    ajVardbInit();
    ajResourcedbInit();

    return;
}




/* @func embInit **************************************************************
**
** Initialises everything. Reads an ACD (AJAX Command Definition) file
** prompts the user for any missing information, reads all sequences
** and other input into local structures which applications can request.
** Must be called in each EMBOSS program first.
**
** @param [r] pgm [const char*] Application name, used as the name of
**                              the ACD file
** @param [r] argc [ajint] Number of arguments provided on the command line,
**        usually passed as-is by the calling application.
** @param [r] argv [char* const[]] Actual arguments as an array of text.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void embInit (const char *pgm, ajint argc, char * const argv[])
{
    initDball();
    ajNamInit("emboss");

    ajAcdInit (pgm, argc, argv);

    return;
}




/* @func embInitP *************************************************************
**
** Initialises everything. Reads an ACD (AJAX Command Definition) file
** prompts the user for any missing information, reads all sequences
** and other input into local structures which applications can request.
** Must be called in each EMBOSS program first.
**
** @param [r] pgm [const char*] Application name, used as the name
**                              of the ACD file
** @param [r] argc [ajint] Number of arguments provided on the command line,
**        usually passed as-is by the calling application.
** @param [r] argv [char* const[]] Actual arguments as an array of text.
** @param [r] package [const char*] Package name, used to find the ACD file
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void embInitP (const char *pgm, ajint argc, char * const argv[],
               const char *package)
{
    initDball();
    ajNamInit("emboss");

    ajAcdInitPV (pgm, argc, argv, package, "");

    return;
}




/* @func embInitPV ************************************************************
**
** Initialises everything. Reads an ACD (AJAX Command Definition) file
** prompts the user for any missing information, reads all sequences
** and other input into local structures which applications can request.
** Must be called in each EMBOSS program first.
**
** @param [r] pgm [const char*] Application name, used as the name
**                              of the ACD file
** @param [r] argc [ajint] Number of arguments provided on the command line,
**        usually passed as-is by the calling application.
** @param [r] argv [char* const[]] Actual arguments as an array of text.
** @param [r] package [const char*] Package name, used to find the ACD file
** @param [r] packversion [const char*] Package version
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void embInitPV (const char *pgm, ajint argc, char * const argv[],
                const char *package, const char *packversion)
{
    initDball();
    ajNamInit("emboss");

    ajAcdInitPV (pgm, argc, argv, package, packversion);

    return;
}

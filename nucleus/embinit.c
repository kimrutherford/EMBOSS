/* @source init.c
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
******************************************************************************/

#include "emboss.h"




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
** @@
******************************************************************************/

void  embInit (const char *pgm, ajint argc, char * const argv[])
{
    ajSeqdbInit();
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
** @@
******************************************************************************/

void embInitP (const char *pgm, ajint argc, char * const argv[],
               const char *package)
{
    ajSeqdbInit();
    ajNamInit("emboss");

    ajAcdInitPV (pgm, argc, argv, package, "");

    return;
}




/* @func embInitPV *************************************************************
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
** @@
******************************************************************************/

void embInitPV (const char *pgm, ajint argc, char * const argv[],
               const char *package, const char *packversion)
{
    ajSeqdbInit();
    ajNamInit("emboss");

    ajAcdInitPV (pgm, argc, argv, package, packversion);

    return;
}

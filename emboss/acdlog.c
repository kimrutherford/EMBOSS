/* @source acdlog application
**
** Log processing of acd files
**
** @author Peter Rice
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
******************************************************************************/


#include "emboss.h"




/* @prog acdlog ***************************************************************
**
** ACD file compiler and logging.
**
** Unlike other ACD report programs (acdpretty and acdtable) acdlog will run
** through command line checking and prompting the user, while logging the
** internal data.
**
******************************************************************************/

int main(int argc, char **argv)
{
    if(argc < 2)
	ajFatal("Error - must specify an application to compile\n");

    ajAcdSetControl("acdlog");
    ajAcdSetControl("acdnocommandline");

    ajGraphInit(argv[1], argc-1, &argv[1]);

    ajAcdExit(ajTrue);		/* turn off the 'never used' ACD warnings */

    embExit();

    return 0;
}


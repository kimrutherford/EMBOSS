/* @source wossname application
**
** Finds programs by keywords in their one-line documentation
**
** @author Copyright (C) Gary Williams (gwilliam@hgmp.mrc.ac.uk)
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




/* @prog wossname *************************************************************
**
** Finds programs by keywords in their one-line documentation
**
******************************************************************************/

int main(int argc, char **argv, char **env)
{

    AjPList newlist = NULL;
    AjPList glist;    /* list of groups pointing to lists of programs */
    AjPList alpha;    /* alphabetical list of all programs */
    AjPFile outfile = NULL;
    AjPStr search   = NULL;
    AjBool matchall;
    AjBool showkey;
    AjBool html;
    AjBool groups;
    AjBool alphabetic;
    AjBool emboss;
    AjBool embassy;
    AjBool explode;
    AjBool colon;
    AjBool gui;
    AjPStr showembassy;

    embInit("wossname", argc, argv);
    
    search     = ajAcdGetString("search");
    outfile    = ajAcdGetOutfile("outfile");
    html       = ajAcdGetToggle("html");
    matchall   = ajAcdGetBool("allmatch");
    showkey    = ajAcdGetBool("showkeywords");
    groups     = ajAcdGetBool("groups");
    alphabetic = ajAcdGetBool("alphabetic");
    emboss     = ajAcdGetBool("emboss");
    embassy    = ajAcdGetBool("embassy");
    showembassy = ajAcdGetString("showembassy");
    explode    = ajAcdGetBool("explode");
    colon      = ajAcdGetBool("colon");
    gui        = ajAcdGetBool("gui");
    
    
    glist = ajListNew();
    alpha = ajListNew();
    
    /* get the groups and program information */
    embGrpGetProgGroups(glist, alpha, env, emboss, embassy, showembassy,
			explode, colon, gui);
    
    
    /* is a search string specified */
    if(ajStrGetLen(search))
    {
	newlist = ajListNew();
	embGrpKeySearchProgs(newlist, alpha, search, matchall);
	embGrpOutputGroupsList(outfile, newlist, !groups, html,
			       showkey, showembassy);
	embGrpGroupsListDel(&newlist);
    }
    else if(alphabetic)
	/* list all programs in alphabetic order */
	embGrpOutputGroupsList(outfile, alpha, !groups, html,
			       showkey, showembassy);
    else
	/* just show the grouped sets of programs */
	embGrpOutputGroupsList(outfile, glist, !groups, html,
			       showkey, showembassy);
    
    ajFileClose(&outfile);
    
    embGrpGroupsListDel(&glist);
    embGrpGroupsListDel(&alpha);
    ajFileClose(&outfile);
    ajStrDel(&search);
    ajStrDel(&showembassy);

    embExit();
    
    return 0;
}


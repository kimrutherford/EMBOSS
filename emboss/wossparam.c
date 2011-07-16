/* @source wossparam application
**
** Finds programs by EDAM parameter
**
** @author Copyright (C) Peter Rice (pmr@ebi.ac.uk)
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




/* @prog wossparam ************************************************************
**
** Finds programs by EDAM parameter
**
******************************************************************************/

int main(int argc, char **argv, char **env)
{

    AjPList newlist = NULL;
    AjPList glist;    /* list of groups pointing to lists of programs */
    AjPList alpha;    /* alphabetical list of all programs */
    AjPFile outfile = NULL;
    AjPStr search   = NULL;
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
    AjBool sensitive = ajFalse;
    AjBool subclasses = ajTrue;
    AjBool obsolete = ajFalse;

    embInit("wossparam", argc, argv);
    
    search     = ajAcdGetString("identifier");
    outfile    = ajAcdGetOutfile("outfile");
    html       = ajAcdGetToggle("html");
    showkey    = ajAcdGetBoolean("showkeywords");
    groups     = ajAcdGetBoolean("groups");
    alphabetic = ajAcdGetBoolean("alphabetic");
    emboss     = ajAcdGetBoolean("emboss");
    embassy    = ajAcdGetBoolean("embassy");
    showembassy = ajAcdGetString("showembassy");
    explode    = ajAcdGetBoolean("explode");
    colon      = ajAcdGetBoolean("colon");
    gui        = ajAcdGetBoolean("gui");
    sensitive = ajAcdGetBoolean("sensitive");
    subclasses = ajAcdGetBoolean("subclasses");
    obsolete = ajAcdGetBoolean("obsolete");
    
    
    glist = ajListNew();
    alpha = ajListNew();
    
    /* get the groups and program information */
    embGrpGetProgGroups(glist, alpha, env, emboss, embassy, showembassy,
			explode, colon, gui);
    
    
    /* is a search string specified */
    newlist = ajListNew();
    if(alphabetic)
        embGrpSearchProgsEdam(newlist, alpha, search, "param",
                              sensitive, subclasses, obsolete);
    else
        embGrpSearchProgsEdam(newlist, glist, search, "param",
                              sensitive, subclasses, obsolete);
        

    embGrpOutputGroupsList(outfile, newlist, !groups, html,
                           showkey, showembassy);
    
    ajFileClose(&outfile);
    
    embGrpGroupsListDel(&newlist);
    embGrpGroupsListDel(&glist);
    embGrpGroupsListDel(&alpha);

    ajFileClose(&outfile);
    ajStrDel(&search);
    ajStrDel(&showembassy);

    embExit();
    
    return 0;
}


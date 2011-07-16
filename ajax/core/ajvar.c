/******************************************************************************
** @source AJAX variation functions
**
** These functions control all aspects of AJAX variation
** parsing and include simple utilities.
**
** @author Copyright (C) 2010 Peter Rice
** @version 1.0
** @modified Oct 5 pmr First version
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA  02111-1307, USA.
******************************************************************************/

#include "ajax.h"

static AjPStr varTempQry = NULL;

static void varMakeQry(const AjPVar thys, AjPStr* qry);




/* @filesection ajvar *******************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
******************************************************************************/





/* @datasection [AjPVar] Variation data ***************************************
**
** Function is for manipulating variation data objects
**
** @nam2rule Var Variation data objects
**
******************************************************************************/




/* @section constructors ******************************************************
**
** Constructors
**
** @fdata [AjPVar]
**
** @nam3rule New Constructor
**
** @valrule * [AjPVar] Variation data object
**
** @fcategory new
**
******************************************************************************/




/* @func ajVarNew ************************************************************
**
** Var data constructor
**
** @return [AjPVar] New object
** @@
******************************************************************************/

AjPVar ajVarNew(void)
{
    AjPVar ret;

    AJNEW0(ret);

    return ret;
}




/* @section Variation data destructors *****************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the variation data object.
**
** @fdata [AjPVar]
**
** @nam3rule Del Destructor
**
** @argrule Del Pvar [AjPVar*] Variation data
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajVarDel ************************************************************
**
** Variation data destructor
**
** @param [d] Pvar       [AjPVar*] Variation data object to delete
** @return [void] 
** @@
******************************************************************************/

void ajVarDel(AjPVar *Pvar)
{
    AjPVar var;

    if(!Pvar) return;
    if(!(*Pvar)) return;

    var = *Pvar;

    ajStrDel(&var->Id);
    ajStrDel(&var->Db);
    ajStrDel(&var->Setdb);
    ajStrDel(&var->Full);
    ajStrDel(&var->Qry);
    ajStrDel(&var->Formatstr);
    ajStrDel(&var->Filename);

    ajStrDelarray(&var->Lines);

    AJFREE(*Pvar);
    *Pvar = NULL;

    return;
}




/* @section Casts *************************************************************
**
** Return values from a variation data object
**
** @fdata [AjPVar]
**
** @nam3rule Get Return a value
** @nam4rule Db Source database name
** @nam4rule Id Identifier string
** @nam4rule Qry Return a query field
** @suffix C Character string result
** @suffix S String object result
**
** @argrule * var [const AjPVar] Variation data object.
**
** @valrule *C [const char*] Query as a character string.
** @valrule *S [const AjPStr] Query as a string object.
** @valrule *GetDb [const AjPStr] Database name
** @valrule *GetId [const AjPStr] Identifier string
**
** @fcategory cast
**
******************************************************************************/




/* @func ajVarGetDb ***********************************************************
**
** Return the database name
**
** @param [r] var [const AjPVar] Variation
**
** @return [const AjPStr] Database name
**
******************************************************************************/

const AjPStr ajVarGetDb(const AjPVar var)
{
    return var->Db;
}




/* @func ajVarGetId ***********************************************************
**
** Return the identifier
**
** @param [r] var [const AjPVar] Variation
**
** @return [const AjPStr] Returned id
**
******************************************************************************/

const AjPStr ajVarGetId(const AjPVar var)
{
    return var->Id;
}




/* @func ajVarGetQryC ********************************************************
**
** Returns the query string of a variation data object.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] var [const AjPVar] Variation data object.
** @return [const char*] Query as a character string.
** @@
******************************************************************************/

const char* ajVarGetQryC(const AjPVar var)
{
    return MAJSTRGETPTR(ajVarGetQryS(var));
}




/* @func ajVarGetQryS ********************************************************
**
** Returns the query string of a variation data object.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] var [const AjPVar] Variation data object.
** @return [const AjPStr] Query as a string.
** @@
******************************************************************************/

const AjPStr ajVarGetQryS(const AjPVar var)
{
    ajDebug("ajVarGetQryS '%S'\n", var->Qry);

    if(ajStrGetLen(var->Qry))
	return var->Qry;

    varMakeQry(var, &varTempQry);

    return varTempQry;
}




/* @funcstatic varMakeQry ***************************************************
**
** Sets the query for a variation data object.
**
** @param [r] thys [const AjPVar] Variation data object
** @param [w] qry [AjPStr*] Query string in full
** @return [void]
** @@
******************************************************************************/

static void varMakeQry(const AjPVar thys, AjPStr* qry)
{
    ajDebug("varMakeQry (Id <%S> Formatstr <%S> Db <%S> "
	    "Filename <%S>)\n",
	    thys->Id, thys->Formatstr, thys->Db,
	    thys->Filename);

    /* ajVarTrace(thys); */

    if(ajStrGetLen(thys->Db))
	ajFmtPrintS(qry, "%S-id:%S", thys->Db, thys->Id);
    else
    {
	ajFmtPrintS(qry, "%S::%S:%S", thys->Formatstr,
                    thys->Filename,thys->Id);
    }

    ajDebug("      result: <%S>\n",
	    *qry);

    return;
}




/* @section variation data modifiers *******************************************
**
** Variation data modifiers
**
** @fdata [AjPVar]
**
** @nam3rule Clear clear internal values
**
** @argrule * var [AjPVar] Variation data object
**
** @valrule * [void]
**
** @fcategory modify
**
******************************************************************************/




/* @func ajVarClear **********************************************************
**
** Resets all data for a variation data object so that it can be reused.
**
** @param [u] var [AjPVar] Variation data object
** @return [void]
** @@
******************************************************************************/

void ajVarClear(AjPVar var)
{
    if(MAJSTRGETLEN(var->Id))
       ajStrSetClear(&var->Id);

    if(MAJSTRGETLEN(var->Db))
       ajStrSetClear(&var->Db);

    if(MAJSTRGETLEN(var->Setdb))
       ajStrSetClear(&var->Setdb);

    if(MAJSTRGETLEN(var->Full))
       ajStrSetClear(&var->Full);

    if(MAJSTRGETLEN(var->Qry))
       ajStrSetClear(&var->Qry);

    if(MAJSTRGETLEN(var->Formatstr))
       ajStrSetClear(&var->Formatstr);

    if(MAJSTRGETLEN(var->Filename))
       ajStrSetClear(&var->Filename);

    ajStrDelarray(&var->Lines);

    var->Count = 0;
    var->Fpos = 0L;
    var->Format = 0;

    return;
}




/* @datasection [none] Miscellaneous functions ********************************
**
** Functions to initialise and clean up internals
**
** @nam2rule Var Variation internals
**
******************************************************************************/




/* @section exit **************************************************************
**
** Functions called on exit from the program by ajExit to do
** any necessary cleanup and to report internal statistics to the debug file
**
** @fdata      [none]
** @fnote     general exit functions, no arguments
**
** @nam3rule Exit Cleanup and report on exit
**
** @valrule * [void]
**
** @fcategory misc
******************************************************************************/




/* @func ajVarExit ***********************************************************
**
** Cleans up variation processing internal memory
**
** @return [void]
** @@
******************************************************************************/

void ajVarExit(void)
{
    ajVarinExit();
    ajVaroutExit();

    return;
}




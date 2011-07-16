/******************************************************************************
** @source AJAX assembly functions
**
** These functions control all aspects of AJAX assembly
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

static AjPStr assemTempQry = NULL;

static void assemMakeQry(const AjPAssem thys, AjPStr* qry);




/* @filesection ajassem *******************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
******************************************************************************/





/* @datasection [AjPAssem] Assembly data **************************************
**
** Function is for manipulating assembly data objects
**
** @nam2rule Assem Assem data objects
**
******************************************************************************/




/* @section constructors ******************************************************
**
** Constructors
**
** @fdata [AjPAssem]
**
** @nam3rule New Constructor
**
** @valrule * [AjPAssem] Assem data object
**
** @fcategory new
**
******************************************************************************/




/* @func ajAssemNew ***********************************************************
**
** Assem data constructor
**
** @return [AjPAssem] New object
** @@
******************************************************************************/

AjPAssem ajAssemNew(void)
{
    AjPAssem ret;

    AJNEW0(ret);

    return ret;
}




/* @section Assem data destructors ********************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the assembly data object.
**
** @fdata [AjPAssem]
**
** @nam3rule Del Destructor
**
** @argrule Del Passem [AjPAssem*] Assem data
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajAssemDel ***********************************************************
**
** Assem data destructor
**
** @param [d] Passem       [AjPAssem*]  Assem data object to delete
** @return [void] 
** @@
******************************************************************************/

void ajAssemDel(AjPAssem *Passem)
{
    AjPAssem assem;

    if(!Passem) return;
    if(!(*Passem)) return;

    assem = *Passem;

    ajStrDel(&assem->Id);
    ajStrDel(&assem->Db);
    ajStrDel(&assem->Setdb);
    ajStrDel(&assem->Full);
    ajStrDel(&assem->Qry);
    ajStrDel(&assem->Formatstr);
    ajStrDel(&assem->Filename);
    ajStrDel(&assem->Textptr);

    AJFREE(*Passem);
    *Passem = NULL;

    return;
}




/* @section Casts *************************************************************
**
** Return values from an assembly data object
**
** @fdata [AjPAssem]
**
** @nam3rule Get Return a value
** @nam4rule Qry Return a query field
** @suffix C Character string result
** @suffix S String object result
**
** @argrule * assem [const AjPAssem] Assem data object.
**
** @valrule *C [const char*] Query as a character string.
** @valrule *S [const AjPStr] Query as a string object.
**
** @fcategory cast
**
******************************************************************************/




/* @func ajAssemGetQryC *******************************************************
**
** Returns the query string of an assembly data object.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] assem [const AjPAssem] Assem data object.
** @return [const char*] Query as a character string.
** @@
******************************************************************************/

const char* ajAssemGetQryC(const AjPAssem assem)
{
    return MAJSTRGETPTR(ajAssemGetQryS(assem));
}




/* @func ajAssemGetQryS *******************************************************
**
** Returns the query string of an assembly data object.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] assem [const AjPAssem] Assem data object.
** @return [const AjPStr] Query as a string.
** @@
******************************************************************************/

const AjPStr ajAssemGetQryS(const AjPAssem assem)
{
    ajDebug("ajAssemGetQryS '%S'\n", assem->Qry);

    if(ajStrGetLen(assem->Qry))
	return assem->Qry;

    assemMakeQry(assem, &assemTempQry);

    return assemTempQry;
}




/* @funcstatic assemMakeQry ***************************************************
**
** Sets the query for an assembly data object.
**
** @param [r] thys [const AjPAssem] Assem data object
** @param [w] qry [AjPStr*] Query string in full
** @return [void]
** @@
******************************************************************************/

static void assemMakeQry(const AjPAssem thys, AjPStr* qry)
{
    ajDebug("assemMakeQry (Id <%S> Formatstr <%S> Db <%S> "
	    "Filename <%S>)\n",
	    thys->Id, thys->Formatstr, thys->Db,
	    thys->Filename);

    /* ajAssemTrace(thys); */

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




/* @section assembly data modifiers *******************************************
**
** Assem data modifiers
**
** @fdata [AjPAssem]
**
** @nam3rule Clear clear internal values
**
** @argrule * assem [AjPAssem] Assem data object
**
** @valrule * [void]
**
** @fcategory modify
**
******************************************************************************/




/* @func ajAssemClear *********************************************************
**
** Resets all data for an assembly data object so that it can be reused.
**
** @param [u] assem [AjPAssem] assem data
** @return [void]
** @@
******************************************************************************/

void ajAssemClear(AjPAssem assem)
{
    if(MAJSTRGETLEN(assem->Id))
       ajStrSetClear(&assem->Id);

    if(MAJSTRGETLEN(assem->Db))
       ajStrSetClear(&assem->Db);

    if(MAJSTRGETLEN(assem->Setdb))
       ajStrSetClear(&assem->Setdb);

    if(MAJSTRGETLEN(assem->Full))
       ajStrSetClear(&assem->Full);

    if(MAJSTRGETLEN(assem->Qry))
       ajStrSetClear(&assem->Qry);

    if(MAJSTRGETLEN(assem->Formatstr))
       ajStrSetClear(&assem->Formatstr);

    if(MAJSTRGETLEN(assem->Filename))
       ajStrSetClear(&assem->Filename);

    ajStrDel(&assem->Textptr);

    assem->Count = 0;
    assem->Fpos = 0L;
    assem->Format = 0;

    return;
}




/* @datasection [none] Miscellaneous functions ********************************
**
** Functions to initialise and clean up internals
**
** @nam2rule Assem Assem internals
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




/* @func ajAssemExit **********************************************************
**
** Cleans up assembly processing internal memory
**
** @return [void]
** @@
******************************************************************************/

void ajAssemExit(void)
{
    ajAsseminExit();
    ajAssemoutExit();

    return;
}




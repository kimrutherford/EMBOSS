/******************************************************************************
** @source AJAX TEXT functions
**
** These functions control all aspects of AJAX text
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

static AjPStr textTempQry = NULL;

static void textMakeQry(const AjPText thys, AjPStr* qry);




/* @filesection ajtext ********************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
******************************************************************************/




/* @datasection [AjPText] Text data ********************************************
**
** Function is for manipulating text data objects
**
** @nam2rule Text Text data objects
**
******************************************************************************/




/* @section constructors ******************************************************
**
** Constructors
**
** @fdata [AjPText]
**
** @nam3rule New Constructor
**
** @valrule * [AjPText] Text data object
**
** @fcategory new
**
******************************************************************************/




/* @func ajTextNew ************************************************************
**
** Text data constructor
**
** @return [AjPText] New object
** @@
******************************************************************************/

AjPText ajTextNew(void)
{
    AjPText ret;

    AJNEW0(ret);

    ret->Lines = ajListstrNew();

    return ret;
}




/* @section Text data destructors *********************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the text data object.
**
** @fdata [AjPText]
**
** @nam3rule Del Destructor
**
** @argrule Del Ptext [AjPText*] Text data
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajTextDel ************************************************************
**
** Text data destructor
**
** @param [d] Ptext       [AjPText*]  Text data object to delete
** @return [void] 
** @@
******************************************************************************/

void ajTextDel(AjPText *Ptext)
{
    AjPText text;
    AjPStr tmpstr = NULL;

    if(!Ptext) return;
    if(!(*Ptext)) return;

    text = *Ptext;

    ajStrDel(&text->Id);
    ajStrDel(&text->Db);
    ajStrDel(&text->Setdb);
    ajStrDel(&text->Full);
    ajStrDel(&text->Qry);
    ajStrDel(&text->Formatstr);
    ajStrDel(&text->Filename);

    while(ajListstrPop(text->Lines, &tmpstr))
        ajStrDel(&tmpstr);
    ajListFree(&text->Lines);

    AJFREE(*Ptext);
    *Ptext = NULL;

    return;
}




/* @section Casts *************************************************************
**
** Return values from a text data object
**
** @fdata [AjPText]
**
** @nam3rule Get Return a value
** @nam4rule Qry Return a query field
** @suffix C Character string result
** @suffix S String object result
**
** @argrule * text [const AjPText] Text data object.
**
** @valrule *C [const char*] Query as a character string.
** @valrule *S [const AjPStr] Query as a string object.
**
** @fcategory cast
**
******************************************************************************/




/* @func ajTextGetQryC ********************************************************
**
** Returns the query string of a text data object.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] text [const AjPText] Text data object.
** @return [const char*] Query as a character string.
** @@
******************************************************************************/

const char* ajTextGetQryC(const AjPText text)
{
    return MAJSTRGETPTR(ajTextGetQryS(text));
}




/* @func ajTextGetQryS ********************************************************
**
** Returns the query string of a text data object.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] text [const AjPText] Text data object.
** @return [const AjPStr] Query as a string.
** @@
******************************************************************************/

const AjPStr ajTextGetQryS(const AjPText text)
{
    ajDebug("ajTextGetQryS '%S'\n", text->Qry);

    if(ajStrGetLen(text->Qry))
	return text->Qry;

    textMakeQry(text, &textTempQry);

    return textTempQry;
}




/* @funcstatic textMakeQry ***************************************************
**
** Sets the query for a text data object.
**
** @param [r] thys [const AjPText] Text data object
** @param [w] qry [AjPStr*] Query string in full
** @return [void]
** @@
******************************************************************************/

static void textMakeQry(const AjPText thys, AjPStr* qry)
{
    ajDebug("textMakeQry (Id <%S> Formatstr <%S> Db <%S> "
	    "Filename <%S>)\n",
	    thys->Id, thys->Formatstr, thys->Db,
	    thys->Filename);

    /* ajTextTrace(thys); */

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




/* @section text data modifiers ***********************************************
**
** Text data modifiers
**
** @fdata [AjPText]
**
** @nam3rule Clear clear internal values
**
** @argrule * text [AjPText] Text data object
**
** @valrule * [void]
**
** @fcategory modify
**
******************************************************************************/




/* @func ajTextClear **********************************************************
**
** Resets all data for a text data object so that it can be reused.
**
** @param [u] text [AjPText] text data
** @return [void]
** @@
******************************************************************************/

void ajTextClear(AjPText text)
{
    AjPStr tmpstr = NULL;

    if(MAJSTRGETLEN(text->Id))
       ajStrSetClear(&text->Id);

    if(MAJSTRGETLEN(text->Db))
       ajStrSetClear(&text->Db);

    if(MAJSTRGETLEN(text->Setdb))
       ajStrSetClear(&text->Setdb);

    if(MAJSTRGETLEN(text->Full))
       ajStrSetClear(&text->Full);

    if(MAJSTRGETLEN(text->Qry))
       ajStrSetClear(&text->Qry);

    if(MAJSTRGETLEN(text->Formatstr))
       ajStrSetClear(&text->Formatstr);

    if(MAJSTRGETLEN(text->Filename))
       ajStrSetClear(&text->Filename);

    while(ajListstrPop(text->Lines, &tmpstr))
        ajStrDel(&tmpstr);

    text->Count = 0;
    text->Fpos = 0L;
    text->Format = 0;

    return;
}




/* @datasection [none] Miscellaneous functions ********************************
**
** Functions to initialise and clean up internals
**
** @nam2rule Text Text internals
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




/* @func ajTextExit ***********************************************************
**
** Cleans up text processing internal memory
**
** @return [void]
** @@
******************************************************************************/

void ajTextExit(void)
{
    ajTextinExit();
    ajTextoutExit();

    return;
}

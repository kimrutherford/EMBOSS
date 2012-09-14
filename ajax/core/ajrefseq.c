/* @source ajrefseq ***********************************************************
**
** AJAX reference sequence functions
**
** These functions control all aspects of AJAX reference sequence
** parsing and include simple utilities.
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.5 $
** @modified Oct 5 pmr First version
** @modified $Date: 2011/10/18 14:23:40 $ by $Author: rice $
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

#include "ajlib.h"

#include "ajrefseq.h"
#include "ajlist.h"
#include "ajrefseqread.h"
#include "ajrefseqwrite.h"
#include "ajseq.h"

static AjPStr refseqTempQry = NULL;

static void refseqMakeQry(const AjPRefseq thys, AjPStr* qry);




/* @filesection ajrefseq *******************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
******************************************************************************/





/* @datasection [AjPRefseq] Reference sequence data ***************************
**
** Function is for manipulating reference sequence data objects
**
** @nam2rule Refseq Reference sequence data objects
**
******************************************************************************/




/* @section constructors ******************************************************
**
** Constructors
**
** @fdata [AjPRefseq]
**
** @nam3rule New Constructor
**
** @valrule * [AjPRefseq] Reference sequence data object
**
** @fcategory new
**
******************************************************************************/




/* @func ajRefseqNew ************************************************************
**
** Refseq data constructor
**
** @return [AjPRefseq] New object
** @@
******************************************************************************/

AjPRefseq ajRefseqNew(void)
{
    AjPRefseq ret;

    AJNEW0(ret);

    return ret;
}




/* @section Reference sequence data destructors *******************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the reference sequence data object.
**
** @fdata [AjPRefseq]
**
** @nam3rule Del Destructor
**
** @argrule Del Prefseq [AjPRefseq*] Reference sequence data
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajRefseqDel **********************************************************
**
** Reference sequence data destructor
**
** @param [d] Prefseq       [AjPRefseq*] Reference sequence data object
**                                       to delete
** @return [void] 
** @@
******************************************************************************/

void ajRefseqDel(AjPRefseq *Prefseq)
{
    AjPRefseq refseq;
    AjPSeqRange tmprange;

    if(!Prefseq) return;
    if(!(*Prefseq)) return;

    refseq = *Prefseq;

    ajStrDel(&refseq->Id);
    ajStrDel(&refseq->Db);
    ajStrDel(&refseq->Setdb);
    ajStrDel(&refseq->Full);
    ajStrDel(&refseq->Qry);
    ajStrDel(&refseq->Formatstr);
    ajStrDel(&refseq->Filename);

    ajStrDel(&refseq->TextPtr);

    ajStrDel(&refseq->Desc);
    ajStrDel(&refseq->Seq);

    while(ajListPop(refseq->Seqlist,(void **)&tmprange))
	ajSeqrangeDel(&tmprange);

    ajListFree(&refseq->Seqlist);

    AJFREE(*Prefseq);
    *Prefseq = NULL;

    return;
}




/* @section Casts *************************************************************
**
** Return values from a reference sequence data object
**
** @fdata [AjPRefseq]
**
** @nam3rule Get Return a value
** @nam4rule Db Source database name
** @nam4rule Id Identifier string
** @nam4rule Qry Return a query field
** @suffix C Character string result
** @suffix S String object result
**
** @argrule * refseq [const AjPRefseq] Reference sequence data object.
**
** @valrule *C [const char*] Query as a character string.
** @valrule *S [const AjPStr] Query as a string object.
** @valrule *GetDb [const AjPStr] Database name
** @valrule *GetId [const AjPStr] Identifier string
**
** @fcategory cast
**
******************************************************************************/




/* @func ajRefseqGetDb ********************************************************
**
** Return the database name
**
** @param [r] refseq [const AjPRefseq] Refseq
**
** @return [const AjPStr] Database name
**
******************************************************************************/

const AjPStr ajRefseqGetDb(const AjPRefseq refseq)
{
    return refseq->Db;
}




/* @func ajRefseqGetId ********************************************************
**
** Return the identifier
**
** @param [r] refseq [const AjPRefseq] Refseq
**
** @return [const AjPStr] Returned id
**
******************************************************************************/

const AjPStr ajRefseqGetId(const AjPRefseq refseq)
{
    return refseq->Id;
}




/* @func ajRefseqGetQryC ******************************************************
**
** Returns the query string of a reference sequence data object.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] refseq [const AjPRefseq] Reference sequence data object.
** @return [const char*] Query as a character string.
** @@
******************************************************************************/

const char* ajRefseqGetQryC(const AjPRefseq refseq)
{
    return MAJSTRGETPTR(ajRefseqGetQryS(refseq));
}




/* @func ajRefseqGetQryS ******************************************************
**
** Returns the query string of a reference sequence data object.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] refseq [const AjPRefseq] Reference sequence data object.
** @return [const AjPStr] Query as a string.
** @@
******************************************************************************/

const AjPStr ajRefseqGetQryS(const AjPRefseq refseq)
{
    ajDebug("ajRefseqGetQryS '%S'\n", refseq->Qry);

    if(ajStrGetLen(refseq->Qry))
	return refseq->Qry;

    refseqMakeQry(refseq, &refseqTempQry);

    return refseqTempQry;
}




/* @funcstatic refseqMakeQry ***************************************************
**
** Sets the query for a reference sequence data object.
**
** @param [r] thys [const AjPRefseq] Reference sequence data object
** @param [w] qry [AjPStr*] Query string in full
** @return [void]
** @@
******************************************************************************/

static void refseqMakeQry(const AjPRefseq thys, AjPStr* qry)
{
    ajDebug("refseqMakeQry (Id <%S> Formatstr <%S> Db <%S> "
	    "Filename <%S>)\n",
	    thys->Id, thys->Formatstr, thys->Db,
	    thys->Filename);

    /* ajRefseqTrace(thys); */

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




/* @section reference sequence data modifiers *********************************
**
** Reference sequence data modifiers
**
** @fdata [AjPRefseq]
**
** @nam3rule Clear clear internal values
**
** @argrule * refseq [AjPRefseq] Reference sequence data object
**
** @valrule * [void]
**
** @fcategory modify
**
******************************************************************************/




/* @func ajRefseqClear *********************************************************
**
** Resets all data for a reference sequence data object so that it can
** be reused.
**
** @param [u] refseq [AjPRefseq] Reference sequence data object
** @return [void]
** @@
******************************************************************************/

void ajRefseqClear(AjPRefseq refseq)
{
    AjPSeqRange tmprange = NULL;

    if(MAJSTRGETLEN(refseq->Id))
       ajStrSetClear(&refseq->Id);

    if(MAJSTRGETLEN(refseq->Db))
       ajStrSetClear(&refseq->Db);

    if(MAJSTRGETLEN(refseq->Setdb))
       ajStrSetClear(&refseq->Setdb);

    if(MAJSTRGETLEN(refseq->Full))
       ajStrSetClear(&refseq->Full);

    if(MAJSTRGETLEN(refseq->Qry))
       ajStrSetClear(&refseq->Qry);

    if(MAJSTRGETLEN(refseq->Formatstr))
       ajStrSetClear(&refseq->Formatstr);

    if(MAJSTRGETLEN(refseq->Filename))
       ajStrSetClear(&refseq->Filename);

    ajStrDel(&refseq->TextPtr);

    ajStrDel(&refseq->Desc);
    ajStrDel(&refseq->Seq);

    while(ajListPop(refseq->Seqlist,(void **)&tmprange))
	ajSeqrangeDel(&tmprange);

    ajListFree(&refseq->Seqlist);

    refseq->Count = 0;
    refseq->Fpos = 0L;
    refseq->Format = 0;

    return;
}




/* @datasection [none] Miscellaneous functions ********************************
**
** Functions to initialise and clean up internals
**
** @nam2rule Refseq Reference sequence internals
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




/* @func ajRefseqExit **********************************************************
**
** Cleans up reference sequence processing internal memory
**
** @return [void]
** @@
******************************************************************************/

void ajRefseqExit(void)
{
    ajRefseqinExit();
    ajRefseqoutExit();

    return;
}




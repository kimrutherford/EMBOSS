/* @source enstable ***********************************************************
**
** Ensembl Table functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.34 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2013/02/17 13:08:58 $ by $Author: mks $
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
**
******************************************************************************/

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "enstable.h"




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== global variables ============================ */
/* ========================================================================= */




/* ========================================================================= */
/* ============================= private data ============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private constants =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */

static void tablestrToList(void **Pkey,
                           void **Pvalue,
                           void *cl);

static void tableuintToList(void **Pkey,
                            void **Pvalue,
                            void *cl);

static AjBool tableuintliststrMatch(AjPStr *Pstr, void *cl);




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




/* @filesection enstable ******************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [AjPTable] AJAX Table *****************************************
**
** @nam2rule Table Functions for manipulating AJAX Table objects
**
******************************************************************************/




/* @datasection [AjPTable] AJAX Table *****************************************
**
** @nam2rule Tablestr Functions for manipulating AJAX Table objects of
**                    AJAX String key data and
**                    generic value data
**
******************************************************************************/




/* @section cast **************************************************************
**
** Cast AJAX Table objects into AJAX List objects and vice versa.
**
** @fdata [AjPTable]
**
** @nam3rule From Convert from an AJAX object
** @nam3rule To Convert to an AJAX object
** @nam4rule List Convert between an AJAX List
**
** @argrule * table [AjPTable] AJAX Table
** @argrule FromList FobjectGetKey [AjPStr function]
** Get AJAX String key function address
** @argrule * list [AjPList] AJAX List
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory new
******************************************************************************/




/* @func ensTablestrFromList **************************************************
**
** Moves generic objects from an AJAX List into an AJAX Table of
** AJAX String key data and generic value data.
**
** Empty nodes of the AJAX List are ignored.
**
** This function relies on ajTablePutClean and expects keydel and valdel
** functions set in the AJAX Table to delete duplicate key and value data.
** The last key and value pair will win.
**
** @param [u] table [AjPTable] AJAX Table of AJAX unsigned integer key data
** @param [f] FobjectGetKey [AjPStr function]
** Get AJAX String key function address
** @param [u] list [AjPList] AJAX List
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.6.0
** @@
******************************************************************************/

AjBool ensTablestrFromList(
    AjPTable table,
    AjPStr (*FobjectGetKey) (const void *object),
    AjPList list)
{
    void *object = NULL;

    AjPStr key = NULL;

    if (!table)
        return ajFalse;

    if (!FobjectGetKey)
        return ajFalse;

    if (!list)
        return ajFalse;

    while (ajListPop(list, (void **) &object))
    {
        if (!object)
            continue;

        key = ajStrNewS((*FobjectGetKey) (object));

        ajTablePutClean(table,
                        (void *) key,
                        (void *) object,
                        (void (*)(void **)) NULL,
                        (void (*)(void **)) NULL);
    }

    return ajTrue;
}




/* @funcstatic tablestrToList *************************************************
**
** An ajTableMapDel "apply" function to clear an AJAX Table of
** AJAX String key data and generic value data.
**
** This function deletes the AJAX String key data
** and moves the generic value data onto the AJAX List.
**
** @param [d] Pkey [void**] AJAX String key data address
** @param [d] Pvalue [void**] Generic value data address
** @param [u] cl [void*] AJAX List, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
**
** @release 6.6.0
** @@
******************************************************************************/

static void tablestrToList(void **Pkey,
                           void **Pvalue,
                           void *cl)
{
    if (!Pkey)
        return;

    if (!Pvalue)
        return;

    if (!cl)
        return;

    ajStrDel((AjPStr *) Pkey);

    ajListPushAppend(cl, *Pvalue);

    *Pvalue = NULL;

    return;
}




/* @func ensTablestrToList ****************************************************
**
** Moves generic value data from an AJAX Table of
** AJAX String key data and
** generic value data onto an AJAX List.
**
** @param [u] table [AjPTable] AJAX Table
** @param [u] list [AjPList] AJAX List
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.6.0
** @@
**
******************************************************************************/

AjBool ensTablestrToList(AjPTable table, AjPList list)
{
    if (!table)
        return ajFalse;

    if (!list)
        return ajFalse;

    ajTableMapDel(table, &tablestrToList, (void *) list);

    return ajTrue;
}




/* @datasection [AjPTable] AJAX Table *****************************************
**
** @nam2rule Tableuint Functions for manipulating AJAX Table objects of
**                     AJAX unsigned integer key data and
**                     generic value data
**
******************************************************************************/




/* @section cast **************************************************************
**
** Cast AJAX Table objects into AJAX List objects and vice versa.
**
** @fdata [AjPTable]
**
** @nam3rule From Convert from an AJAX object
** @nam3rule To Convert to an AJAX object
** @nam4rule List Convert between an AJAX List
**
** @argrule * table [AjPTable] AJAX Table
** @argrule FromList FobjectGetIdentifier [ajuint function]
** Get AJAX unsigned integer key function address
** @argrule * list [AjPList] AJAX List
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory new
******************************************************************************/




/* @func ensTableuintFromList *************************************************
**
** Moves data from an AJAX List into an AJAX Table of
** AJAX unsigned integer key data and generic value data.
**
** Empty nodes of the AJAX List are ignored.
**
** This function relies on ajTablePutClean and expects keydel and valdel
** functions set in the AJAX Table to delete duplicate key and value data.
** The last key and value pair will win.
**
** @param [u] table [AjPTable]
** AJAX Table of AJAX unsigned integer key data and generic value data
** @param [f] FobjectGetIdentifier [ajuint function]
** Get AJAX unsigned integer key function address
** @param [u] list [AjPList] AJAX List
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.6.0
** @@
******************************************************************************/

AjBool ensTableuintFromList(
    AjPTable table,
    ajuint (*FobjectGetIdentifier) (const void *object),
    AjPList list)
{
    void *object = NULL;

    ajuint *Pidentifier = NULL;

    if (!table)
        return ajFalse;

    if (!FobjectGetIdentifier)
        return ajFalse;

    if (!list)
        return ajFalse;

    while (ajListPop(list, (void **) &object))
    {
        if (!object)
            continue;

        AJNEW0(Pidentifier);

        *Pidentifier = (*FobjectGetIdentifier) (object);

        ajTablePutClean(table,
                        (void *) Pidentifier,
                        (void *) object,
                        (void (*)(void **)) NULL,
                        (void (*)(void **)) NULL);
    }

    return ajTrue;
}




/* @funcstatic tableuintToList ************************************************
**
** An ajTableMapDel "apply" function to clear an AJAX Table of
** AJAX unsigned integer key data and generic value data.
**
** This function deletes the AJAX unsigned integer key data
** and moves the generic value data onto the AJAX List.
**
** @param [d] Pkey [void**] AJAX unsigned integer key data address
** @param [d] Pvalue [void**] Generic value data address
** @param [u] cl [void*] AJAX List, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
**
** @release 6.6.0
** @@
******************************************************************************/

static void tableuintToList(void **Pkey,
                            void **Pvalue,
                            void *cl)
{
    if (!Pkey)
        return;

    if (!Pvalue)
        return;

    if (!cl)
        return;

    ajMemFree(Pkey);

    ajListPushAppend(cl, *Pvalue);

    *Pvalue = NULL;

    return;
}




/* @func ensTableuintToList ***************************************************
**
** Moves generic value data from an AJAX Table of
** AJAX unsigned integer key data and
** generic value data onto an AJAX List.
**
** @param [u] table [AjPTable]
** AJAX Table od AJAX unsigned integer key data and generic value data
** @param [u] list [AjPList] AJAX List
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.6.0
** @@
**
******************************************************************************/

AjBool ensTableuintToList(AjPTable table, AjPList list)
{
    if (!table)
        return ajFalse;

    if (!list)
        return ajFalse;

    ajTableMapDel(table, &tableuintToList, (void *) list);

    return ajTrue;
}




/* @datasection [AjPTable] AJAX Table *****************************************
**
** @nam2rule Tableuintliststr Functions for manipulating AJAX Table objects of
**                            AJAX unsigned integer key data and
**                            AJAX String-List value data
**
******************************************************************************/




/* @section constructors ******************************************************
**
** These constructors return a new AJAX Table by pointer.
** It is the responsibility of the user to first destroy any previous
** Table. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [AjPTable]
**
** @nam3rule New Constructor
** @nam4rule Len Constructor with an initial length
**
** @argrule Len size [ajuint] Number of key-value pairs
**
** @valrule * [AjPTable] AJAX Table or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensTableuintliststrNewLen ********************************************
**
** Creates, initialises, and returns a new, empty AJAX Table that can hold an
** arbitrary number of key-value pairs.
** The AJAX Table is initialised with comparison, hash and key deletion
** functions suitable for AJAX unsigned integer keys.
**
** @param [r] size [ajuint] Number of key-value pairs
**
** @return [AjPTable] AJAX Table or NULL
**
** @release 6.4.0
** @@
**
******************************************************************************/

AjPTable ensTableuintliststrNewLen(ajuint size)
{
    AjPTable table = NULL;

    table = ajTableuintNew(size);

    ajTableSetDestroyvalue(table, (void (*)(void **)) &ajListstrFreeData);

    return table;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an AJAX Table object.
**
** @fdata [AjPTable]
**
** @nam3rule Clear  Clear
** @nam3rule Delete Destroy (free) an AJAX Table of
**                  AJAX unsigned integer key data and
**                  AJAX String-List value data
**
** @argrule Clear table [AjPTable] AJAX Table
** @argrule Delete Ptable [AjPTable*] AJAX Table address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensTableuintliststrClear *********************************************
**
** Utility function to clear an AJAX Table of
** AJAX unsigned integer key data and
** AJAX String-List value data.
**
** @param [u] table [AjPTable] AJAX Table
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ensTableuintliststrClear(AjPTable table)
{
    ajTableClearDelete(table);

    return;
}




/* @func ensTableuintliststrDelete ********************************************
**
** Utility function to clear and delete an AJAX Table of
** AJAX unsigned integer key data and
** AJAX String-List value data.
**
** @param [d] Ptable [AjPTable*] AJAX Table address
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ensTableuintliststrDelete(AjPTable *Ptable)
{
    ajTableDel(Ptable);

    return;
}




/* @section register **********************************************************
**
** Registers AJAX String objects in an AJAX Table of
** AJAX unsigned integer key data and
** AJAX String-List value data.
**
** @fdata [AjPTable]
**
** @nam3rule Register Register an AJAX String
**
** @argrule Register table [AjPTable] AJAX Table
** @argrule Register identifier [ajuint] AJAX unsigned integer identifier
** @argrule Register str [AjPStr] AJAX String
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @funcstatic tableuintliststrMatch ******************************************
**
** An ajListstrMapfind "apply" function to match AJAX String objects in an
** AJAX String-List.
**
** @param [u] Pstr [AjPStr*] AJAX String address
** @param [u] cl [void*] AJAX String
** @see ajListstrMapfind
**
** @return [AjBool] ajTrue:  A particular AJAX String has already been indexed.
**                  ajFalse: A particular AJAX String has not been indexed
**                           before, but has been added now.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool tableuintliststrMatch(AjPStr *Pstr, void *cl)
{
    if (!Pstr)
        return ajFalse;

    if (!cl)
        return ajFalse;

    return ajStrMatchS(*Pstr, (AjPStr) cl);
}




/* @func ensTableuintliststrRegister ******************************************
**
** For an AJAX Table of AJAX unsigned integer key data and AJAX String-List
** value data check, whether an AJAX String has already been indexed for a
** particular AJAX unsigned integer.
**
** @param [u] table [AjPTable] AJAX Table
** @param [rN] identifier [ajuint] AJAX unsigned integer identifier
** @param [u] str [AjPStr] AJAX String
**
** @return [AjBool] ajTrue:  A particular AJAX String has already been indexed.
**                  ajFalse: A particular AJAX String has not been indexed
**                           before, but has been added now.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensTableuintliststrRegister(AjPTable table,
                                   ajuint identifier,
                                   AjPStr str)
{
    ajuint *Pidentifier = NULL;

    AjPList list = NULL;

    if (ajDebugTest("ensTableuintliststrRegister"))
        ajDebug("ensTableuintliststrRegister\n"
                "  table %p\n"
                "  identifier %u\n"
                "  str '%S'\n",
                table,
                identifier,
                str);

    if (!table)
        return ajFalse;

    if (!str)
        return ajFalse;

    list = (AjPList) ajTableFetchmodV(table, (const void *) &identifier);

    if (list)
    {
        if (ajListstrMapfind(list, &tableuintliststrMatch, (void *) str))
            return ajTrue;
        else
        {
            ajListstrPushAppend(list, ajStrNewS(str));

            return ajFalse;
        }
    }
    else
    {
        AJNEW0(Pidentifier);

        *Pidentifier = identifier;

        list = ajListstrNew();

        ajTablePut(table, (void *) Pidentifier, (void *) list);

        ajListstrPushAppend(list, ajStrNewS(str));

        return ajFalse;
    }
}

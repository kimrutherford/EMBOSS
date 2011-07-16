/* @source Ensembl Table functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2011/05/25 19:55:04 $ by $Author: mks $
** @version $Revision: 1.20 $
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

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "enstable.h"




/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== global variables ========================== */
/* ==================================================================== */




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private constants ========================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private variables ========================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static void tableClear(void** key, void** value, void* cl);

static void tableuintliststrClear(void** key, void** value, void* cl);

static AjBool tableuintliststrMatch(AjPStr* x, void* cl);




/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */




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




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an AJAX Table object.
**
** @fdata [AjPTable]
**
** @nam3rule Clear  Clear
** @nam3rule Delete Destroy (free) an AJAX Table object of
**                  void key data and
**                  void value data
**
** @argrule Clear table [AjPTable] AJAX Table
** @argrule Delete Ptable [AjPTable*] AJAX Table object address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory delete
******************************************************************************/




/* @funcstatic tableClear *****************************************************
**
** An ajTableMapDel "apply" function to clear an AJAX Table of
** void key data and
** void value data.
**
** @param [u] key [void**] Void address
** @param [u] value [void**] Void address
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void tableClear(void** key, void** value, void* cl)
{
    if(!key)
        return;

    if(!*key)
        return;

    if(!value)
        return;

    if(!*value)
        return;

    (void) cl;

    AJFREE(*key);
    AJFREE(*value);

    *key   = NULL;
    *value = NULL;

    return;
}




/* @func ensTableClear ********************************************************
**
** Utility function to clear an AJAX Table of
** void key data and
** void value data.
**
** @param [u] table [AjPTable] AJAX Table
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTableClear(AjPTable table)
{
    if(!table)
        return ajFalse;

    ajTableMapDel(table, tableClear, NULL);

    return ajTrue;
}




/* @func ensTableDelete *******************************************************
**
** Utility function to clear and delete an AJAX Table of
** void key data and
** void value data.
**
** @param [d] Ptable [AjPTable*] AJAX Table object address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTableDelete(AjPTable* Ptable)
{
    AjPTable pthis = NULL;

    if(!Ptable)
        return ajFalse;

    if(!*Ptable)
        return ajFalse;

    pthis = *Ptable;

    ensTableClear(pthis);

    ajTableFree(&pthis);

    *Ptable = NULL;

    return ajTrue;
}




/* @datasection [AjPTable] AJAX Table *****************************************
**
** @nam2rule Tablestr Functions for manipulating AJAX Table objects of
**                    AJAX String key data
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
** @argrule Len size [ajuint] number of key-value pairs
**
** @valrule * [AjPTable] AJAX Table or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensTablestrNewLen ****************************************************
**
** Creates, initialises, and returns a new, empty table that can hold an
** arbitrary number of key-value pairs.
** The AJAX Table is initialised with comparison, hash and key deletion
** functions suitable for AJAX String keys.
**
** @param [r] size [ajuint] number of key-value pairs
**
** @return [AjPTable] AJAX Table or NULL
** @category new [AjPTable] Creates a table.
** @@
**
******************************************************************************/

AjPTable ensTablestrNewLen(ajuint size)
{
    return ajTableNewFunctionLen(size,
                                 ajTablestrCmp,
                                 ajTablestrHash,
                                 NULL,
                                 NULL);
}




/* @datasection [AjPTable] AJAX Table *****************************************
**
** @nam2rule Tableuint Functions for manipulating AJAX Table objects of
**                     AJAX unsigned integer key data
**
******************************************************************************/




/* @section functions *********************************************************
**
** @fdata [AjPTable]
**
** @nam3rule Cmp Comparison function
** @nam3rule Hash Hash function
** @nam3rule Keydel Key deletion function
**
** @argrule Cmp x [const void*] Unsigned integer first key
** @argrule Cmp y [const void*] Unsigned integer second key
** @argrule Hash key [const void*] Unsigned integer key
** @argrule Hash hashsize [ajuint] Hash table size
** @argrule Keydel key [void**] AJAX unsigned integer key address
**
** @valrule Cmp [ajint] 0 for a match, -1 or +1 for one key greater
** @valrule Hash [ajuint] Unsigned integer hash value
** @valrule Keydel [void]
**
** @fcategory misc
******************************************************************************/




/* @func ensTableuintCmp ******************************************************
**
** AJAX Table function to compare AJAX unsigned integer (ajuint)
** hash key values.
**
** @param [r] x [const void*] AJAX unsigned integer value address
** @param [r] y [const void*] AJAX unsigned integer value address
** @see ajTableNewFunctionLen
**
** @return [ajint] 0 if the values are identical, 1 if they are not
** @@
******************************************************************************/

ajint ensTableuintCmp(const void* x, const void* y)
{
    const ajuint* a = NULL;
    const ajuint* b = NULL;

    a = (const ajuint*) x;
    b = (const ajuint*) y;

#if AJFALSE
    if(ajDebugTest("ensTableuintCmp"))
        ajDebug("ensTableuintCmp *a %u *b %u result %d\n", *a, *b, (*a != *b));
#endif

    return (*a != *b);
}




/* @func ensTableuintHash *****************************************************
**
** AJAX Table function to handle AJAX unsigned integer (ajuint)
** hash key values.
**
** @param [r] key [const void*] AJAX unsigned integer key value address
** @param [r] hashsize [ajuint] Hash size (maximum hash value)
** @see ajTableNewFunctionLen
**
** @return [ajuint] Hash value
** @@
******************************************************************************/

ajuint ensTableuintHash(const void* key, ajuint hashsize)
{
    const ajuint* a = NULL;

    if(!key)
        return 0;

    if(!hashsize)
        return 0;

    a = (const ajuint*) key;

#if AJFALSE
    if(ajDebugTest("ensTableuintHash"))
        ajDebug("ensTableuintHash result %u\n", ((*a >> 2) % hashsize));
#endif

    return ((*a >> 2) % hashsize);
}




/* @func ensTableuintKeydel ***************************************************
**
** AJAX Table function to delete AJAX unsigned integer (ajuint)
** hash key values.
**
** @param [r] key [void**] AJAX unsigned integer value address
** @see ajTableNewFunctionLen
**
** @return [void]
** @@
******************************************************************************/

void ensTableuintKeydel(void** key)
{
    if(!key)
        return;

    if(!*key)
        return;

    AJFREE(*key);

    *key = NULL;

    return;
}




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
** @argrule Len size [ajuint] number of key-value pairs
**
** @valrule * [AjPTable] AJAX Table or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensTableuintNewLen ***************************************************
**
** Creates, initialises, and returns a new, empty table that can hold an
** arbitrary number of key-value pairs.
** The AJAX Table is initialised with comparison, hash and key deletion
** functions suitable for AJAX unsigned integer keys.
**
** @param [r] size [ajuint] number of key-value pairs
**
** @return [AjPTable] AJAX Table or NULL
** @category new [AjPTable] Creates a table.
** @@
**
******************************************************************************/

AjPTable ensTableuintNewLen(ajuint size)
{
    return ajTableNewFunctionLen(size,
                                 ensTableuintCmp,
                                 ensTableuintHash,
                                 NULL,
                                 NULL);
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
** @argrule Len size [ajuint] number of key-value pairs
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
** @param [r] size [ajuint] number of key-value pairs
**
** @return [AjPTable] AJAX Table or NULL
** @@
**
******************************************************************************/

AjPTable ensTableuintliststrNewLen(ajuint size)
{
    return ajTableNewFunctionLen(size,
                                 ensTableuintCmp,
                                 ensTableuintHash,
                                 NULL,
                                 NULL);
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an AJAX Table object.
**
** @fdata [AjPTable]
**
** @nam3rule Clear  Clear
** @nam3rule Delete Destroy (free) an AJAX Table object of
**                  AJAX unsigned integer key data and
**                  AJAX String-List value data
**
** @argrule Clear table [AjPTable] AJAX Table
** @argrule Delete Ptable [AjPTable*] AJAX Table object address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory delete
******************************************************************************/




/* @funcstatic tableuintliststrClear ******************************************
**
** An ajTableMapDel "apply" function to clear an AJAX Table of
** AJAX unsigned integer key data and
** AJAX String-List value data.
**
** @param [u] key [void**] AJAX unsigned integer address
** @param [u] value [void**] AJAX String-List address
** @param [u] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void tableuintliststrClear(void** key, void** value, void* cl)
{
    if(!key)
        return;

    if(!*key)
        return;

    if(!value)
        return;

    if(!*value)
        return;

    (void) cl;

    AJFREE(*key);

    ajListstrFreeData((AjPList*) value);

    *key   = NULL;
    *value = NULL;

    return;
}




/* @func ensTableuintliststrClear *********************************************
**
** Utility function to clear an AJAX Table of
** AJAX unsigned integer key data and
** AJAX String-List value data.
**
** @param [u] table [AjPTable] AJAX Table
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTableuintliststrClear(AjPTable table)
{
    if(!table)
        return ajFalse;

    ajTableMapDel(table, tableuintliststrClear, NULL);

    return ajTrue;
}




/* @func ensTableuintliststrDelete ********************************************
**
** Utility function to clear and delete an AJAX Table of
** AJAX unsigned integer key data and
** AJAX String-List value data.
**
** @param [d] Ptable [AjPTable*] AJAX Table object address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensTableuintliststrDelete(AjPTable* Ptable)
{
    AjPTable pthis = NULL;

    if(!Ptable)
        return ajFalse;

    if(!*Ptable)
        return ajFalse;

    pthis = *Ptable;

    ensTableuintliststrClear(pthis);

    ajTableFree(&pthis);

    *Ptable = NULL;

    return ajTrue;
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
** @param [u] x [AjPStr*] AJAX String address
** @param [u] cl [void*] AJAX String
** @see ajListstrMapfind
**
** @return [AjBool] ajTrue:  A particular AJAX String has already been indexed.
**                  ajFalse: A particular AJAX String has not been indexed
**                           before, but has been added now.
** @@
******************************************************************************/

static AjBool tableuintliststrMatch(AjPStr* x, void* cl)
{
    if(!x)
        return ajFalse;

    if(!cl)
        return ajFalse;

    return ajStrMatchS(*x, (AjPStr) cl);
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
** @@
******************************************************************************/

AjBool ensTableuintliststrRegister(AjPTable table,
                                   ajuint identifier,
                                   AjPStr str)
{
    ajuint* Pidentifier = NULL;

    AjPList list = NULL;

    if(!table)
        return ajFalse;

    if(!str)
        return ajFalse;

    list = (AjPList) ajTableFetchmodV(table, (const void*) &identifier);

    if(list)
    {
        if(ajListstrMapfind(list, tableuintliststrMatch, (void*) str))
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

        ajTablePut(table, (void*) Pidentifier, (void*) list);

        ajListstrPushAppend(list, ajStrNewS(str));

        return ajFalse;
    }
}

/* @source ajtable ************************************************************
**
** AJAX table functions
**
** Hash table functions.
**
** @author Copyright (C) 1998 Ian Longden
** @version $Revision: 1.74 $
** @modified 2011 pmr Auto-resizing, destructors, table merges
** @modified $Date: 2013/02/17 13:39:44 $ by $Author: mks $
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


#include "ajlib.h"

#include "ajtable.h"
#include "ajassert.h"

#include <limits.h>
#include <string.h>
#include <ctype.h>




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

#ifdef AJ_SAVESTATS
static ajuint tableNewCnt = 0U;
static ajint tableDelCnt  = 0;
static ajuint tableMaxNum = 0U;
static size_t tableMaxMem = 0;
#endif

static AjPStr tableTmpkeyStr = NULL;

static ajulong tableFreeNext = 0UL;
static ajulong tableFreeMax  = 0UL;
static AjPTableNode* tableFreeSet = NULL;




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */

static ajint  tableCmpAtom(const void* key1, const void* key2);
static void   tableDel(void** Pkey, void** Pvalue, void* cl);
static void   tableDelKey(void** Pkey, void** Pvalue, void* cl);
static void   tableFreeSetExpand (void);
static ajulong tableHashAtom(const void* key, ajulong hashsize);
static void   tableStrDel(void** Pkey, void** Pvalue, void* cl);
static void   tableStrDelKey(void** Pkey, void** Pvalue, void* cl);

static void   tableDelStr(void** Pstr);
static const char* tableType(const AjPTable table);
static void   tableStrFromKey(const AjPTable table, const void* key,
                              AjPStr* Pstr);




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




/* @filesection ajtable *******************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
** @suffix C [char*] C character string
** @suffix S [AjPStr] string object
** @suffix Len [ajuint] size of hash table
**
******************************************************************************/




/* @datasection [AjPTable] Hash tables ****************************************
**
** Function is for manipulating hash tables with any value type.
**
** Some functions are specially designed to understand string (AjPStr) values.
**
** @nam2rule Table
**
******************************************************************************/




/* @section Constructors ******************************************************
**
** Constructors for hash tables
**
** @fdata [AjPTable]
**
** @fcategory new
**
** @nam3rule New Constructor
** @nam4rule Function Functions defined by caller
**
** @argrule New size [ajulong] Number of key values
** @argrule Function cmp [ajint function] Comparison function returning
**                                        +1, zero or -1
** @argrule Function hash [ajulong function] Hash function for keys
** @argrule Function keydel [void function] key destructor function
** @argrule Function valdel [void function] value destructor function
**
** @valrule * [AjPTable] New hash table
**
******************************************************************************/




/* @func ajTableNew ***********************************************************
**
** creates, initialises, and returns a new, empty table that expects
** a specified number of key-value pairs.
**
** Current method defines the table size as the number of entries divided by 4
** to avoid a huge table.
**
** @param [r] size [ajulong] number of key-value pairs
**
** @return [AjPTable] new table.
**
**
** @release 1.0.0
** @@
**
******************************************************************************/

AjPTable ajTableNew(ajulong size)
{
    AjPTable table = NULL;

    table = ajTableNewFunctionLen(size, &tableCmpAtom, &tableHashAtom,
                                  NULL, NULL);
    table->Type = ajETableTypeUser;

    return table;
}




/* @func ajTableNewFunctionLen ************************************************
**
** creates, initialises, and returns a new, empty table that expects
** a specified number of key-value pairs.
**
** Current method defines the table size as the number of entries divided by 4
** to avoid a huge table.
**
** @param [r] size [ajulong] number of key-value pairs
** @param [fN] cmp  [ajint function] function for comparing
** @param [fN] hash [ajulong function] function for hashing keys
** @param [fN] keydel [void function] key destructor function
** @param [fN] valdel [void function] value destructor function
**
** @return [AjPTable] new table.
**
**
** @release 5.0.0
** @@
**
******************************************************************************/

AjPTable ajTableNewFunctionLen(ajulong size,
                               ajint (*cmp)(const void* key1,
					    const void* key2),
                               ajulong (*hash)(const void* key,
					       ajulong hashsize),
                               void (*keydel)(void** Pkey),
                               void (*valdel)(void** Pvalue))
{
    ajulong hint  = 0UL;
    ajulong i     = 0UL;
    ajulong iprime = 0UL;

    AjPTable table = NULL;

    /* largest primes just under 2**8 to 2**31 */

    static ajuint primes[] =
        {
            3, 7, 13, 31, 61, 127,
            251, 509,
            1021, 2039, 4093, 8191,
            16381, 32749, 65521,
            131071, 262139, 524287,
            1048573, 2097143, 4194301, 8388593,
            16777213, 33554393, 67108859,
            134217689, 268435399, 536870909,
            1073741789, 2147483647,
            UINT_MAX
        };

    hint = size/4;

    for(i = 1U; primes[i] <= hint; i++); /* else use default primes[0] */

    iprime = primes[i - 1];

    /*ajDebug("ajTableNewFunctionLen hint %d size %d\n", hint, iprime);*/

    AJNEW0(table);
    table->Size    = iprime;
    table->Fcmp    = cmp;
    table->Fhash   = hash;
    table->Fkeydel = keydel;
    table->Fvaldel = valdel;

    AJCNEW(table->Buckets, iprime);

    for(i = 0UL; i < table->Size; i++)
        table->Buckets[i] = NULL;

    table->Length    = 0UL;
    table->Timestamp = 0U;

#ifdef AJ_SAVESTATS
    tableNewCnt++;

    if(iprime > tableMaxNum)
        tableMaxNum = iprime;

    if(sizeof(*table) > tableMaxMem)
        tableMaxMem = sizeof(*table);
#endif

    table->Use = 1;
    table->Type = ajETableTypeUser;

    return table;
}




/* @section Destructors *******************************************************
**
** @fdata [AjPTable]
**
** Destructors know they are dealing with strings and can
** clean up keys and values
**
** @nam3rule Del Delete table
** @nam4rule Keydel Delete keys using destructor
** @nam4rule Valdel Delete values using destructor
** @nam5rule KeydelValdel Delete keys and values using destructors
** @nam3rule Free Delete table, ignore keys and values
**
** @argrule * Ptable [AjPTable*] Hash table
** @argrule Keydel keydel [void function] Key destructor function
** @argrule Valdel valdel [void function] Value destructor function
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajTableDel ***********************************************************
**
** Deallocates and clears a hash table, and any keys or values.
**
** @param [d] Ptable [AjPTable*] Table (by reference)
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTableDel(AjPTable* Ptable)
{
    AjPTable thys;

    if(!Ptable)
        return;
    if(!*Ptable)
        return;

    thys = *Ptable;

    if(!thys->Use)
        ajErr("trying to delete unused table");

    --thys->Use;

    if(!thys->Use)
    {                                   /* any other references? */
        ajTableClearDelete(thys);

        AJFREE(thys->Buckets);
        AJFREE(*Ptable);
    }

    *Ptable = NULL;

    return;
}




/* @func ajTableDelKeydelValdel ***********************************************
**
** Deallocates and clears a hash table, and any keys or values.
**
** @param [d] Ptable [AjPTable*] Table (by reference)
** @param [f] keydel [void function] key destructor
** @param [f] valdel [void function] value destructor
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTableDelKeydelValdel(AjPTable* Ptable,
                            void (*keydel)(void** Pkey),
                            void (*valdel)(void** Pvalue))
{
    AjPTable thys;

    if(!Ptable)
        return;
    if(!*Ptable)
        return;

    thys = *Ptable;

    if(!thys->Use)
        ajErr("trying to delete unused table");

    --thys->Use;

    if(!thys->Use)
    {                                   /* any other references? */
        thys->Fkeydel = keydel;
        thys->Fvaldel = valdel;

        ajTableClearDelete(thys);

        AJFREE(thys->Buckets);
        AJFREE(*Ptable);
    }

    *Ptable = NULL;

    return;
}




/* @func ajTableDelValdel *****************************************************
**
** Deallocates and clears a hash table, and any keys or values.
**
** @param [d] Ptable [AjPTable*] Table (by reference)
** @param [f] valdel [void function] value destructor
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTableDelValdel(AjPTable* Ptable,
                      void (*valdel)(void** Pvalue))
{
    AjPTable thys;

    if(!Ptable)
        return;
    if(!*Ptable)
        return;

    thys = *Ptable;

    if(!thys->Use)
        ajErr("trying to delete unused table");

    --thys->Use;

    if(!thys->Use)
    {                                   /* any other references? */
        thys->Fvaldel = valdel;

        ajTableClearDelete(thys);

        AJFREE(thys->Buckets);
        AJFREE(*Ptable);
    }

    *Ptable = NULL;

    return;
}




/* @func ajTableFree **********************************************************
**
** Deallocates and clears a hash table. Does not clear keys or values.
**
** @param [d] Ptable [AjPTable*] Table (by reference)
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajTableFree(AjPTable* Ptable)
{
    AjPTable thys;

    if(!Ptable)
        return;
    if(!*Ptable)
        return;

    thys = *Ptable;

    if(!thys->Use)
        ajErr("trying to delete unused table");

    --thys->Use;

    if(!thys->Use)
    {                                   /* any other references? */
        ajTableClear(thys);

        AJFREE(thys->Buckets);
        AJFREE(*Ptable);
    }

    *Ptable = NULL;

    return;
}




/* @funcstatic tableFreeSetExpand *********************************************
**
** Expand the list of free AJAX Table Node objects.
**
** @return [void]
**
** @release 6.0.0
******************************************************************************/

static void tableFreeSetExpand (void)
{
    ajulong newsize;

    if(!tableFreeSet)
    {
        tableFreeMax = 1024;
        AJCNEW0(tableFreeSet,tableFreeMax);

        return;
    }

    if(tableFreeMax >= 65536)
        return;

    newsize = tableFreeMax + tableFreeMax;
    AJCRESIZE0(tableFreeSet, (size_t) tableFreeMax, (size_t) newsize);
    tableFreeMax = newsize;

    return;
}




/* @funcstatic tableDel *******************************************************
**
** Delete an entry in a table by freeing the key and value pointers
**
** @param [d] Pkey [void**] Standard argument. Table key.
** @param [d] Pvalue [void**] Standard argument. Table item.
** @param [u] cl [void*] Standard argument. Usually NULL.
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void tableDel(void** Pkey, void** Pvalue, void* cl)
{
    AJFREE(*Pkey);
    AJFREE(*Pvalue);

    (void) cl;

    return;
}




/* @funcstatic tableDelKey ****************************************************
**
** Delete an entry in a table by freeing the key pointers
**
** @param [d] Pkey [void**] Standard argument. Table key.
** @param [d] Pvalue [void**] Standard argument. Table item.
** @param [u] cl [void*] Standard argument. Usually NULL.
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void tableDelKey(void** Pkey, void** Pvalue, void* cl)
{
    AJFREE(*Pkey);
    *Pvalue = NULL;

    (void) cl;

    return;
}




/* @section Retrieval *********************************************************
**
** @fdata [AjPTable]
**
** Retrieves values from a hash table
**
** @nam3rule Fetch Retrieval function for read-only value
** @nam3rule Fetchmod Retrieval function for modifiable value
** @nam3rule Fetchkey Key retrieval function
** @nam3rule Get return attribute
** @nam4rule GetLength Table number of entries
** @nam4rule GetSize Table hash array size
** @nam4rule Trace Debug report on table internals
** @nam3rule Match Test key matches in table
** @nam3rule Toarray Return keys and values as arrays
** @suffix Keys Return keys as array
** @suffix Values Return values as array
** @suffix C      Pass key as C string
** @suffix S      Pass key as string object
** @suffix V      Pass key as void
**
** @argrule * table [const AjPTable] Hash table
** @argrule C txtkey [const char*] Key
** @argrule S key [const AjPStr] Key
** @argrule V key [const void*] Key
** @argrule Keys keyarray [void***] Array of keys, ending with NULL
** @argrule Values valarray [void***] Array of values, ending with NULL
**
** @valrule Fetch [const void*] Value
** @valrule Fetchmod [void*] Value
** @valrule *Fetchkey [const void*] Key
** @valrule *Length [ajulong] Number of unique keys and values
** @valrule *Match [AjBool] True if a match was found
** @valrule *Size [ajulong] Hash array size
** @valrule *Toarray [ajulong] Array size (not counting trailing NULL)
** @fcategory cast
**
******************************************************************************/




/* @func ajTableFetchC ********************************************************
**
** returns the value associated with key in table, or null
** if table does not hold key.
**
** @param [r] table [const AjPTable] table to search
** @param [r] txtkey [const char*] key to find.
** @return [const void*]  value associated with key
** @error NULL if key not found in table.
**
** @release 6.4.0
** @@
******************************************************************************/

const void* ajTableFetchC(const AjPTable table, const char* txtkey)
{
    ajulong i = 0UL;

    AjPTableNode node = NULL;

    if(!table)
        return NULL;

    if(!txtkey)
        return NULL;

    if(table->Type == ajETableTypeStr)
    {
        ajStrAssignC(&tableTmpkeyStr, txtkey);
        return ajTableFetchS(table, tableTmpkeyStr);
    }

    if(table->Type != ajETableTypeChar)
        ajFatal("ajTableFetchC called for %s table", tableType(table));

    i = (*table->Fhash)(txtkey, table->Size);

    for(node = table->Buckets[i]; node; node = node->Link)
        if((*table->Fcmp)(txtkey, node->Key) == 0)
            break;

    return node ? node->Value : NULL;
}




/* @func ajTableFetchS ********************************************************
**
** returns the value associated with key in table, or null
** if table does not hold key.
**
** @param [r] table [const AjPTable] table to search
** @param [r] key   [const AjPStr] key to find.
** @return [const void*]  value associated with key
** @error NULL if key not found in table.
**
** @release 6.4.0
** @@
******************************************************************************/

const void* ajTableFetchS(const AjPTable table, const AjPStr key)
{
    ajulong i = 0UL;

    AjPTableNode node = NULL;

    if(!table)
        return NULL;

    if(!key)
        return NULL;

    if(table->Type == ajETableTypeChar)
        return ajTableFetchC(table, MAJSTRGETPTR(key));

    if(table->Type != ajETableTypeStr)
        ajFatal("ajTableFetchS called for %s table", tableType(table));

    i = (*table->Fhash)(key, table->Size);

    for(node = table->Buckets[i]; node; node = node->Link)
        if((*table->Fcmp)(key, node->Key) == 0)
            break;

    return node ? node->Value : NULL;
}




/* @func ajTableFetchV ********************************************************
**
** Returns the value associated with key in table, or null
** if table does not hold key.
**
** @param [r] table [const AjPTable] table to search
** @param [r] key   [const void*] key to find.
** @return [const void*]  value associated with key
** @error NULL if key not found in table.
**
** @release 6.4.0
** @@
******************************************************************************/

const void* ajTableFetchV(const AjPTable table, const void* key)
{
    ajulong i = 0UL;

    AjPTableNode node = NULL;

    if(!table)
        return NULL;

    if(!key)
        return NULL;

    i = (*table->Fhash)(key, table->Size);

    for(node = table->Buckets[i]; node; node = node->Link)
        if((*table->Fcmp)(key, node->Key) == 0)
            break;

    return node ? node->Value : NULL;
}




/* @func ajTableFetchmodC *****************************************************
**
** returns the value associated with key in table, or null
** if table does not hold key.
**
** @param [r] table [const AjPTable] table to search
** @param [r] txtkey [const char*] key to find.
** @return [void*]  value associated with key
** @error NULL if key not found in table.
**
** @release 6.4.0
** @@
******************************************************************************/

void* ajTableFetchmodC(const AjPTable table, const char* txtkey)
{
    ajulong i = 0UL;

    AjPTableNode node = NULL;

    if(!table)
        return NULL;

    if(!txtkey)
        return NULL;

    if(table->Type == ajETableTypeStr)
    {
        ajStrAssignC(&tableTmpkeyStr, txtkey);
        return ajTableFetchmodS(table, tableTmpkeyStr);
    }

    if(table->Type != ajETableTypeChar)
        ajFatal("ajTableFetchmodC called for %s table", tableType(table));

    i = (*table->Fhash)(txtkey, table->Size);

    for(node = table->Buckets[i]; node; node = node->Link)
        if((*table->Fcmp)(txtkey, node->Key) == 0)
            break;

    return node ? node->Value : NULL;
}




/* @func ajTableFetchmodS *****************************************************
**
** returns the value associated with key in table, or null
** if table does not hold key.
**
** @param [r] table [const AjPTable] table to search
** @param [r] key   [const AjPStr] key to find.
** @return [void*]  value associated with key
** @error NULL if key not found in table.
**
** @release 6.4.0
** @@
******************************************************************************/

void* ajTableFetchmodS(const AjPTable table, const AjPStr key)
{
    ajulong i = 0UL;

    AjPTableNode node = NULL;

    if(!table)
        return NULL;

    if(!key)
        return NULL;

    if(table->Type == ajETableTypeChar)
        return ajTableFetchmodC(table, MAJSTRGETPTR(key));

    if(table->Type != ajETableTypeStr)
        ajFatal("ajTableFetchmodS called for %s table", tableType(table));

    i = (*table->Fhash)(key, table->Size);

    for(node = table->Buckets[i]; node; node = node->Link)
        if((*table->Fcmp)(key, node->Key) == 0)
            break;

    return node ? node->Value : NULL;
}




/* @func ajTableFetchmodV *****************************************************
**
** Returns the value associated with key in table, or null
** if table does not hold key.
**
** @param [r] table [const AjPTable] table to search
** @param [r] key   [const void*] key to find.
** @return [void*]  value associated with key
** @error NULL if key not found in table.
**
** @release 6.4.0
** @@
******************************************************************************/

void* ajTableFetchmodV(const AjPTable table, const void* key)
{
    ajulong i = 0UL;

    AjPTableNode node = NULL;

    if(!table)
        return NULL;

    if(!key)
        return NULL;

    i = (*table->Fhash)(key, table->Size);

    for(node = table->Buckets[i]; node; node = node->Link)
        if((*table->Fcmp)(key, node->Key) == 0)
            break;

    return node ? node->Value : NULL;
}




/* @func ajTableFetchmodTraceV ************************************************
**
** Returns the value associated with key in table, or null
** if table does not hold key.
**
** Trace internals with debug calls.
**
** @param [r] table [const AjPTable] table to search
** @param [r] key   [const void*] key to find.
** @return [void*]  value associated with key
** @error NULL if key not found in table.
**
** @release 6.4.0
** @@
******************************************************************************/

void* ajTableFetchmodTraceV(const AjPTable table, const void* key)
{
    ajulong i = 0UL;
    ajulong j = 0UL;

    AjPTableNode node = NULL;

    if(!table)
        return NULL;

    if(!key)
        return NULL;

    ajDebug("ajTableFetchmodTraceV length %Lu key %x (%Lu)\n",
            table->Length, key, key);

    i = (*table->Fhash)(key, table->Size);

    ajDebug("...hash to bucket %Lu\n", i);

    for(node = table->Buckets[i]; node; node = node->Link)
    {
        if((*table->Fcmp)(key, node->Key) == 0)
            break;
        j++;
    }

    if(node)
    {
        ajDebug("...tested %Lu keys found %x (%Lu)\n",
                j, node->Value, node->Value);
    }
    else
        ajDebug("...tested %Lu keys no match\n", j);

    return node ? node->Value : NULL;
}




/* @func ajTableGetLength *****************************************************
**
** returns the number of key-value pairs in table.
**
** @param [r] table [const AjPTable] Table to be applied.
** @return [ajulong] number of key-value pairs.
**
** @release 5.0.0
** @@
******************************************************************************/

ajulong ajTableGetLength(const AjPTable table)
{
    if(!table)
        return 0UL;

    return table->Length;
}




/* @func ajTableGetSize *******************************************************
**
** returns the size of the hash array in a table.
**
** @param [r] table [const AjPTable] Table to be applied.
** @return [ajulong] number of hash array positions
**
** @release 6.4.0
** @@
******************************************************************************/

ajulong ajTableGetSize(const AjPTable table)
{
    if(!table)
        return 0UL;

    return table->Size;
}




/* @func ajTableMatchC ********************************************************
**
** Returns true if the key is found in the table
**
** Used in place of ajTableFetchS where the value may be NULL
**
** @param [r] table [const AjPTable] table to search
** @param [r] txtkey [const char*] key to find.
** @return [AjBool] True if key was found[
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajTableMatchC(const AjPTable table, const char* txtkey)
{
    ajulong i = 0UL;

    AjPTableNode node = NULL;

    if(!table)
        return ajFalse;

    if(!txtkey)
        return ajFalse;

    if(table->Type == ajETableTypeStr)
    {
        ajStrAssignC(&tableTmpkeyStr, txtkey);
        return ajTableMatchS(table, tableTmpkeyStr);
    }

    if(table->Type != ajETableTypeChar)
        ajFatal("ajTableMatchC called for %s table", tableType(table));

    i = (*table->Fhash)(txtkey, table->Size);

    for(node = table->Buckets[i]; node; node = node->Link)
        if((*table->Fcmp)(txtkey, node->Key) == 0)
            break;

    return node ? ajTrue : ajFalse;
}




/* @func ajTableMatchS ********************************************************
**
** Returns true if the key is found in the table
**
** Used in place of ajTableFetchS where the value may be NULL
**
** @param [r] table [const AjPTable] table to search
** @param [r] key   [const AjPStr] key to find.
** @return [AjBool] True if key was found
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajTableMatchS(const AjPTable table, const AjPStr key)
{
    ajulong i = 0UL;

    AjPTableNode node = NULL;

    if(!table)
        return ajFalse;

    if(!key)
        return ajFalse;

    if(table->Type == ajETableTypeChar)
        return ajTableMatchC(table, MAJSTRGETPTR(key));

    if(table->Type != ajETableTypeStr)
        ajFatal("ajTableMatchS called for %s table", tableType(table));

    i = (*table->Fhash)(key, table->Size);

    for(node = table->Buckets[i]; node; node = node->Link)
        if((*table->Fcmp)(key, node->Key) == 0)
            break;

    return node ? ajTrue : ajFalse;
}




/* @func ajTableMatchV ********************************************************
**
** Returns true if the key is found in the table
**
** Used in place of ajTableFetchS where the value may be NULL
**
** @param [r] table [const AjPTable] table to search
** @param [r] key [const void*] key to find.
** @return [AjBool] True if key was found
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajTableMatchV(const AjPTable table, const void* key)
{
    ajulong i = 0UL;

    AjPTableNode node = NULL;

    if(!table)
        return ajFalse;

    if(!key)
        return ajFalse;

    i = (*table->Fhash)(key, table->Size);

    for(node = table->Buckets[i]; node; node = node->Link)
        if((*table->Fcmp)(key, node->Key) == 0)
            break;

    return node ? ajTrue : ajFalse;
}




/* @func ajTableToarrayKeys ***************************************************
**
** creates two N+1 element arrays that holds the N keys
** in table in an unspecified order and returns the number of elements.
** The final element of the array is NULL.
**
** @param [r] table [const AjPTable] Table
** @param [w] keyarray [void***] NULL terminated array of keys.
** @return [ajulong] size of array returned
**
** @release 6.2.0
** @@
******************************************************************************/

ajulong ajTableToarrayKeys(const AjPTable table,
                          void*** keyarray)
{
    ajulong i = 0UL;
    ajulong j = 0UL;

    AjPTableNode node = NULL;

    if(*keyarray)
        AJFREE(*keyarray);

    if(!table)
        return 0UL;

    *keyarray = AJALLOC((size_t)(table->Length + 1) * sizeof(keyarray));

    for(i = 0UL; i < table->Size; i++)
        for(node = table->Buckets[i]; node; node = node->Link)
            (*keyarray)[j++] = node->Key;

    (*keyarray)[j] = NULL;

    return table->Length;
}




/* @func ajTableToarrayKeysValues *********************************************
**
** creates two N+1 element arrays that holds the N key-value pairs
** in table in an unspecified order and returns the number of elements.
** The final element of the array is NULL.
**
** @param [r] table [const AjPTable] Table
** @param [w] keyarray [void***] NULL terminated array of keys.
** @param [w] valarray [void***] NULL terminated array of values.
** @return [ajulong] size of arrays returned
**
** @release 6.2.0
** @@
******************************************************************************/

ajulong ajTableToarrayKeysValues(const AjPTable table,
                                void*** keyarray, void*** valarray)
{
    ajulong i = 0UL;
    ajulong j = 0UL;

    AjPTableNode node = NULL;

    if(*keyarray)
        AJFREE(*keyarray);

    if(*valarray)
        AJFREE(*valarray);

    if(!table)
        return 0UL;

    *keyarray = AJALLOC((size_t)(table->Length + 1) * sizeof(keyarray));
    *valarray = AJALLOC((size_t)(table->Length + 1) * sizeof(valarray));

    for(i = 0UL; i < table->Size; i++)
        for(node = table->Buckets[i]; node; node = node->Link)
        {
            (*keyarray)[j]   = node->Key;
            (*valarray)[j++] = node->Value;
        }

    (*keyarray)[j] = NULL;
    (*valarray)[j] = NULL;

    return table->Length;
}




/* @func ajTableToarrayValues *************************************************
**
** creates two N+1 element arrays that holds the N values
** in table in an unspecified order and returns the number of elements.
** The final element of the array is NULL.
**
** @param [r] table [const AjPTable] Table
** @param [w] valarray [void***] NULL terminated array of values.
** @return [ajulong] size of array returned
**
** @release 6.2.0
** @@
******************************************************************************/

ajulong ajTableToarrayValues(const AjPTable table,
			     void*** valarray)
{
    ajulong i = 0UL;
    ajulong j = 0UL;

    AjPTableNode node = NULL;

    if(*valarray)
        AJFREE(*valarray);

    if(!table)
        return 0;

    *valarray = AJALLOC((size_t)(table->Length + 1) * sizeof(valarray));

    for(i = 0UL; i < table->Size; i++)
        for(node = table->Buckets[i]; node; node = node->Link)
        {
            (*valarray)[j++] = node->Value;
        }

    (*valarray)[j] = NULL;

    return table->Length;
}




/* @section Trace functions ***************************************************
**
** @fdata [AjPTable]
**
** @nam3rule Trace Trace contents
** @argrule Trace table [const AjPTable] Hash table
** @valrule * [void]
**
** @fcategory misc
**
******************************************************************************/




/* @func ajTableTrace *********************************************************
**
** Writes debug messages to trace the contents of a table.
**
** @param [r] table [const AjPTable] Table
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajTableTrace(const AjPTable table)
{
    ajulong i = 0UL;
    ajulong j = 0UL;
    ajulong k = 0UL;

    AjPTableNode node = NULL;

    if(!table)
        return;

    ajDebug("table trace: ");
    ajDebug(" length: %Lu", table->Length);
    ajDebug(" size: %Lu", table->Size);
    ajDebug(" timestamp: %u", table->Timestamp);

    for(i = 0UL; i < table->Size; i++)
        if(table->Buckets[i])
        {
            j = 0UL;

            for(node = table->Buckets[i]; node; node = node->Link)
                j++;

            k += j;
        }

    ajDebug(" links: %Lu\n", k);

    return;
}




/* @section Adding values *****************************************************
**
** @fdata [AjPTable]
**
** @nam3rule Put Add new key and value
** @nam4rule PutClean Add new key and value, removing any existing key and value
** @nam3rule Remove Remove one key and value
** @nam4rule RemoveKey Remove one key and value, return key for deletion
** @nam4rule Trace Debug report on table internals
**
** @argrule * table [AjPTable] Hash table
** @argrule Put key [void*] Key
** @argrule Put value [void*] Value
** @argrule Remove key [const void*] Key
** @argrule RemoveKey truekey [void**] Removed key pointer - ready to be freed
** @argrule Clean keydel [void function] key destructor function
** @argrule Clean valdel [void function] value destructor function
**
** @valrule * [void*] Previous value for key, or NULL
** @valrule *Clean [AjBool] Previous value found and destroyed
**
** @fcategory modify
******************************************************************************/




/* @func ajTablePut ***********************************************************
**
** change the value associated with key in table to value and returns
** the previous value, or adds key and value if table does not hold key,
** and returns null.
**
** @param [u] table [AjPTable] Table to add to
** @param [o] key [void*] key
** @param [u] value [void*] value of key
** @return [void*] previous value if key exists, NULL if not.
**
** @release 1.0.0
** @@
******************************************************************************/

void* ajTablePut(AjPTable table, void* key, void* value)
{
    ajulong i       = 0UL;
    ajulong minsize = 0UL;

    AjPTableNode node = NULL;

    void* prev = NULL;

    if(!table)
        return NULL;

    if(!key)
        return NULL;

    minsize = table->Length / 8UL;
    if(table->Size < minsize)
        ajTableResizeCount(table, 6*minsize);

    i = (*table->Fhash)(key, table->Size);

    for(node = table->Buckets[i]; node; node = node->Link)
        if((*table->Fcmp)(key, node->Key) == 0)
            break;

    if(node == NULL)
    {
        if(tableFreeNext)
            node = tableFreeSet[--tableFreeNext];
        else
            AJNEW0(node);

        node->Key = key;
        node->Link = table->Buckets[i];
        table->Buckets[i] = node;
        table->Length++;
        prev = NULL;
    }
    else
    {
        if(table->Fkeydel)
        {
            (*table->Fkeydel)(&node->Key);
            node->Key = key;
        }
        prev = node->Value;
    }

    node->Value = value;
    table->Timestamp++;

    return prev;
}




/* @func ajTablePutClean ******************************************************
**
** change the value associated with key in table to value and returns
** the previous value, applying destructors to the given key and value,
** or adds key and value if table does not hold key,
** and returns null.
**
**
** @param [u] table [AjPTable] Table to add to
** @param [o] key [void*] key
** @param [u] value [void*] value of key
** @param [f] keydel [void function] key destructor
** @param [f] valdel [void function] value destructor
** @return [AjBool] previous value was found and deleted.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajTablePutClean(AjPTable table, void* key, void* value,
                       void (*keydel)(void** Pkey),
                       void (*valdel)(void** Pvalue))
{
    ajulong i       = 0UL;
    ajulong minsize = 0UL;

    AjPTableNode node = NULL;

    AjBool result = ajFalse;

    if(!table)
        return ajFalse;

    if(!key)
        return ajFalse;

    minsize = table->Length / 8UL;
    if(table->Size < minsize)
        ajTableResizeCount(table, 6*minsize);

    i = (*table->Fhash)(key, table->Size);

    for(node = table->Buckets[i]; node; node = node->Link)
        if((*table->Fcmp)(key, node->Key) == 0)
            break;

    if(node == NULL)
    {
        if(tableFreeNext)
            node = tableFreeSet[--tableFreeNext];
        else
            AJNEW0(node);

        node->Key = key;
        node->Link = table->Buckets[i];
        table->Buckets[i] = node;
        table->Length++;
    }
    else
    {
/*
  {
  AjPStr tmpkey = NULL;
  AjPStr newkey = NULL;
  tableStrFromKey(table, node->Key, &tmpkey);
  tableStrFromKey(table, key, &newkey);
  ajDebug("ajTablePutClean duplicate key %x (%S) == %x (%S)\n",
  node->Key, tmpkey, key, newkey);
  ajStrDel(&tmpkey);
  ajStrDel(&newkey);
  }
*/
        if (keydel)
            (*keydel) (&node->Key);
        else if (table->Fkeydel)
            (*table->Fkeydel) (&node->Key);

        node->Key = key;

        if (valdel)
            (*valdel) (&node->Value);
        else if (table->Fvaldel)
            (*table->Fvaldel) (&node->Value);

        result = ajTrue;
    }

    node->Value = value;
    table->Timestamp++;

    return result;
}




/* @funcstatic tableStrFromKey ************************************************
**
** Tries to print the value of a key by checking the internal hash functions
** against the hash function for the table to identify a known key type
**
** Returns a string describing the key type and its value
**
** @param [r] table [const AjPTable] Table object
** @param [r] key [const void*] Key
** @param [w] Pstr [AjPStr*] Output string
**
** @return [void]
**
** @release 6.4.0
******************************************************************************/

static void tableStrFromKey(const AjPTable table, const void* key,
                            AjPStr* Pstr)
{
    if(table->Fhash == &ajTablecharHash)
    {
        ajFmtPrintS(Pstr, "key '%s' (char*)",
                    (const char*) key);
    }
    else if(table->Fhash == &ajTableintHash)
    {
        ajFmtPrintS(Pstr, "key '%d' (int)",
                    *((const ajint*) key));
    }
    else if(table->Fhash == &ajTablelongHash)
    {
        ajFmtPrintS(Pstr, "key '%Ld' (int)",
                    *((const ajlong*) key));
    }
    else if(table->Fhash == &ajTablestrHash)
    {
        ajFmtPrintS(Pstr, "key '%S' (string)",
                    (const AjPStr) key);
    }
    else if(table->Fhash == &ajTableuintHash)
    {
        ajFmtPrintS(Pstr, "key '%u' (unsigned int)",
                    *((const ajuint*) key));
    }
    else if(table->Fhash == &ajTableulongHash)
    {
        ajFmtPrintS(Pstr, "key '%Lu' (unsigned long)",
                    *((const ajulong*) key));
    }
    else
    {
        ajFmtPrintS(Pstr, "key '%x' (unknown)",
                    (const void*) key);
    }

    return;
}




/* @func ajTablePutTrace ******************************************************
**
** change the value associated with key in table to value and returns
** the previous value, or adds key and value if table does not hold key,
** and returns null. Trace the internals as the value is inserted.
**
** @param [u] table [AjPTable] Table to add to
** @param [o] key [void*] key
** @param [u] value [void*] value of key
** @return [void*] previous value if key exists, NULL if not.
**
** @release 6.4.0
** @@
******************************************************************************/

void* ajTablePutTrace(AjPTable table, void* key, void* value)
{
    ajulong i       = 0UL;
    ajulong j       = 0UL;
    ajulong minsize = 0UL;

    AjPTableNode node = NULL;

    void* prev = NULL;

    if(!table)
        return NULL;

    if(!key)
        return NULL;

    if(!value)
        return NULL;

    minsize = table->Length / 6UL;
    if(table->Size < minsize)
        ajTableResizeCount(table, 4 * table->Length);

    ajDebug("ajTablePut length %Lu key %x (%Lu) value %x (%Lu)\n",
            table->Length, key, key, value, value);

    i = (*table->Fhash)(key, table->Size);

    ajDebug("...hash to bucket %Lu\n", i);

    for(node = table->Buckets[i]; node; node = node->Link)
    {
        if((*table->Fcmp)(key, node->Key) == 0)
            break;
        j++;
    }

    ajDebug("...tested %Lu keys\n", j);

    if(node == NULL)
    {
        if(tableFreeNext)
            node = tableFreeSet[--tableFreeNext];
        else
            AJNEW0(node);

        node->Key = key;
        node->Link = table->Buckets[i];
        table->Buckets[i] = node;
        table->Length++;
        prev = NULL;
    }
    else
    {
        ajDebug("...existing value %x %Lu\n", node->Value, node->Value);
        prev = node->Value;
    }

    node->Value = value;
    table->Timestamp++;

    return prev;
}




/* @func ajTableRemove ********************************************************
**
** Removes the key-value pair from table and returns the removed
** value. If table does not hold key, ajTableRemove has no effect
** and returns null.
**
** @param [u] table [AjPTable] Table
** @param [r] key [const void*] key to be removed
** @return [void*] removed value.
** @error NULL if key not found.
**
** @release 1.0.0
** @@
******************************************************************************/

void* ajTableRemove(AjPTable table, const void* key)
{
    ajulong i = 0UL;

    AjPTableNode* Pnode = NULL;

    if(!table)
        return NULL;

    if(!key)
        return NULL;

    {
        AjPStr newkey = NULL;
        tableStrFromKey(table, key, &newkey);
        /*ajDebug("ajTableRemove key %x (%S)\n", key, newkey);*/
        ajStrDel(&newkey);
    }

    table->Timestamp++;
    i = (*table->Fhash)(key, table->Size);

    for(Pnode = &table->Buckets[i]; *Pnode; Pnode = &(*Pnode)->Link)
        if((*table->Fcmp)(key, (*Pnode)->Key) == 0)
        {
            AjPTableNode node = *Pnode;
            void* value = node->Value;
            *Pnode = node->Link;

            if(table->Fkeydel)
            {
                AjPStr tmpkey = NULL;
                AjPStr newkey = NULL;
                tableStrFromKey(table, node->Key, &tmpkey);
                tableStrFromKey(table, key, &newkey);
                ajDebug("ajTableRemove key %x (%S) matched key %x (%S)\n",
                        key, newkey, node->Key, tmpkey);
                (*table->Fkeydel)(&node->Key);
                ajStrDel(&tmpkey);
                ajStrDel(&newkey);
            }

            if(tableFreeNext >= tableFreeMax)
                AJFREE(node);
            else
                tableFreeSet[tableFreeNext++] = node;

            table->Length--;

            return value;
        }

    return NULL;
}




/* @func ajTableRemoveKey *****************************************************
**
** Removes the key-value pair from table and returns the removed
** value. If table does not hold key, ajTableRemove has no effect
** and returns null.
**
** @param [u] table [AjPTable] Table
** @param [r] key [const void*] key to be removed
** @param [w] truekey [void**] true internal key returned, now owned by caller
** @return [void*] removed value.
** @error NULL if key not found.
**
** @release 5.0.0
** @@
******************************************************************************/

void* ajTableRemoveKey(AjPTable table, const void* key, void** truekey)
{
    ajulong i = 0UL;

    AjPTableNode* Pnode = NULL;

    if(!table)
        return NULL;

    if(!key)
        return NULL;

    table->Timestamp++;
    i = (*table->Fhash)(key, table->Size);

    for(Pnode = &table->Buckets[i]; *Pnode; Pnode = &(*Pnode)->Link)
        if((*table->Fcmp)(key, (*Pnode)->Key) == 0)
        {
            AjPTableNode node = *Pnode;
            void* value = node->Value;
            *truekey = node->Key;
            *Pnode   = node->Link;

            if(tableFreeNext >= tableFreeMax)
                AJFREE(node);
            else
                tableFreeSet[tableFreeNext++] = node;

            table->Length--;

            return value;
        }

    return NULL;
}




/* @section Map function to each value ****************************************
**
** @fdata [AjPTable]
**
** @nam3rule Map Map function to each key value pair
** @nam4rule MapDel Map function to delete each key value pair
**
** @argrule Map table [AjPTable] Hash table
** @argrule Map apply [void function] function to be applied
** @argrule Map cl [void*] Standard. Usually NULL. To be passed to apply
**
** @valrule * [void]
**
** @fcategory modify
******************************************************************************/




/* @func ajTableMap ***********************************************************
**
** calls function 'apply' for each key-value in table
** in an unspecified order. The table keys should not be modified by
** function 'apply' although values can be updated.
**
** See ajTableMapDel for a function that can delete.
**
** Note: because of the properties of C it is difficult to check these
**       are being called correctly. This is because the apply function
**       uses void* arguments.
**
** @param [u] table [AjPTable] Table.
** @param [f] apply [void function] function to be applied
** @param [u] cl [void*] Standard. Usually NULL. To be passed to apply
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajTableMap(AjPTable table,
                void (*apply)(const void* key, void** Pvalue, void* cl),
                void* cl)
{
    ajulong i     = 0UL;
    ajuint stamp = 0U;

    AjPTableNode node = NULL;

    if(!table)
        return;

    stamp = table->Timestamp;

    for(i = 0UL; i < table->Size; i++)
        for(node = table->Buckets[i]; node; node = node->Link)
        {
            (*apply)(node->Key, &node->Value, cl);
            assert(table->Timestamp == stamp);
        }

    return;
}




/* @func ajTableMapDel ********************************************************
**
** calls function 'apply' for each key-value in table
** in an unspecified order.
**
** Keys in the table can be deleted - for example a function to delete
** a table entry. See ajTableMap for a function that is read-only
**
** @param [u] table [AjPTable] Table.
** @param [f] apply [void function] function to be applied
** @param [u] cl [void*] Standard. Usually NULL. To be passed to apply
** @return [void]
**
** @release 2.9.0
** @@
******************************************************************************/

void ajTableMapDel(AjPTable table,
                   void (*apply)(void** Pkey, void** Pvalue, void* cl),
                   void* cl)
{
    ajulong i    = 0UL;
    ajuint stamp = 0U;

    AjPTableNode node1 = NULL;
    AjPTableNode node2 = NULL;

    if(!table)
        return;

    stamp = table->Timestamp;

    for(i = 0UL; i < table->Size; i++)
    {
        for(node1 = table->Buckets[i]; node1; node1 = node2)
        {
            node2 = node1->Link;

            (*apply)(&node1->Key, &node1->Value, cl);
            assert(table->Timestamp == stamp);
            table->Length--;
            if(tableFreeNext >= tableFreeMax)
                tableFreeSetExpand();
            if(tableFreeNext >= tableFreeMax)
                AJFREE(node1);
            else
                tableFreeSet[tableFreeNext++] = node1;
        }
        table->Buckets[i] = NULL;
    }

    return;
}




/* @section Modify ************************************************************
**
** @fdata [AjPTable]
**
** Updates values from a hash table
**
** @nam3rule Clear Reset table
** @nam4rule ClearDelete Reset table and delete keys and values
** @nam3rule Resize Resize table
** @nam4rule ResizeCount Resize table to fit a given number of entries
** @nam4rule ResizeHashsize Resize table to a minimum hash size
** @nam3rule Set Set a table property
** @nam4rule SetDestroy Set key and value destructors
** @nam4rule SetDestroyboth Set value destructor same as key
** @nam4rule SetDestroykey Set key destructor
** @nam4rule SetDestroyvalue Set value destructor
** @nam3rule Settype Reset table using a new key type
** @nam4rule SettypeChar C character string type
** @nam4rule SettypeDefault Default key type
** @nam4rule SettypeInt Integer key type
** @nam4rule SettypeLong Long integer key type
** @nam4rule SettypeString String key type
** @nam4rule SettypeUint Unsigned integer key type
** @nam4rule SettypeUlong Unsigned long integer key type
** @nam4rule SettypeUser User-defined key type
** @nam5rule Case Case-insensitive string comparison
**
** @argrule * table [AjPTable] Hash table
** @argrule ResizeCount size [ajulong] Expected number of key-value pairs
** @argrule ResizeHashsize hashsize [ajulong] Required minimum hash array size
** @argrule SettypeUser cmp [ajint function] Comparison function returning
**                                            +1, zero or -1
** @argrule SettypeUser hash [ajulong function] Hash function for keys
** @argrule Destroy keydel [void function] key destructor function
** @argrule Destroykey keydel [void function] key destructor function
** @argrule Destroy valdel [void function] value destructor function
** @argrule Destroyvalue valdel [void function] value destructor function
**
** @valrule * [void]
** @fcategory modify
**
******************************************************************************/




/* @func ajTableClear *********************************************************
**
** Clears a hash table. Does not clear keys or values.
**
** @param [u] table [AjPTable] Table
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTableClear(AjPTable table)
{
    ajulong i     = 0UL;
    ajulong ikeys = 0UL;

    if(!table)
        return;

    /*ajDebug("ajTableClear length: %Lu size: %Lu type: %u\n",
      table->Length, table->Size, table->Type);*/

    if(table->Length > 0)
    {
        AjPTableNode node1 = NULL;
        AjPTableNode node2 = NULL;

        for(i = 0UL; i < table->Size; i++)
        {
            ikeys = 0UL;
            for(node1 = table->Buckets[i]; node1; node1 = node2)
            {
                ikeys++;

                node2 = node1->Link;

                if(tableFreeNext >= tableFreeMax)
                    tableFreeSetExpand();
                if(tableFreeNext >= tableFreeMax)
                    AJFREE(node1);
                else
                    tableFreeSet[tableFreeNext++] = node1;
            }
            table->Buckets[i] = NULL;
        }
    }

    table->Length = 0UL;

    return;
}




/* @func ajTableClearDelete ***************************************************
**
** Clears a hash table. Deletes all keys or values.
**
** @param [u] table [AjPTable] Table
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTableClearDelete(AjPTable table)
{
    ajulong i = 0UL;

    AjPTableNode node1 = NULL;
    AjPTableNode node2 = NULL;

    if(!table)
        return;

    if(table->Length > 0)
    {

        for(i = 0UL; i < table->Size; i++)
        {
            for(node1 = table->Buckets[i]; node1; node1 = node2)
            {
                node2 = node1->Link;

                if(table->Fkeydel)
                    (*table->Fkeydel)(&node1->Key);

                if(table->Fvaldel)
                    (*table->Fvaldel)(&node1->Value);

                if(tableFreeNext >= tableFreeMax)
                    tableFreeSetExpand();
                if(tableFreeNext >= tableFreeMax)
                    AJFREE(node1);
                else
                    tableFreeSet[tableFreeNext++] = node1;
            }
            table->Buckets[i] = NULL;
        }
    }

    table->Length = 0UL;

    return;
}




/* @func ajTableResizeCount ***************************************************
**
** Resizes a hash table to a new number of expected values.
**
** @param [u] table [AjPTable] Table
** @param [r] size [ajulong] Expected number of key-value pairs
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTableResizeCount(AjPTable table, ajulong size)
{
    ajTableResizeHashsize(table, size/4);

    return;
}




/* @func ajTableResizeHashsize ************************************************
**
** Resizes a hash table to a new size.
**
** @param [u] table [AjPTable] Table
** @param [r] hashsize [ajulong] Expected number of key-value pairs
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTableResizeHashsize(AjPTable table, ajulong hashsize)
{
    ajulong n         = 0UL;
    void** keyarray   = NULL;
    void** valarray   = NULL;
    ajulong hint      = 0UL;
    ajulong iprime    = 0UL;
    ajulong ibucket   = 0UL;
    ajulong i         = 0UL;
    AjPTableNode node = NULL;
    ajulong savelength = 0UL;

    /* largest primes just under 2**8 to 2**31 */

    static ajulong primes[] =
        {
            3UL,
            7UL,
            13UL,
            31UL,
            61UL,
            127UL,
            251UL,
            509UL,
            1021UL,
            2039UL,
            4093UL,
            8191UL,
            16381UL,
            32749UL,
            65521UL,
            131071UL,
            262139UL,
            524287UL,
            1048573UL,
            2097143UL,
            4194301UL,
            8388593UL,
            16777213UL,
            33554393UL,
            67108859UL,
            134217689UL,
            268435399UL,
            536870909UL,
            1073741789UL,
            2147483647UL,
            UINT_MAX
        };

    if(!table)
        return;

    /* check the size required */

    hint = hashsize;

    for(i = 1; primes[i] <= hint; i++); /* else use default i=0 */

    iprime = primes[i - 1];

    /*ajDebug("ajTableResizeHashsize test %x entries %Lu newhashsize %Lu "
      "hashsize %Lu newsize %Lu\n",
      table, table->Length, hashsize, table->Size, iprime);*/

    if(iprime == table->Size)
        return;

    /* save the current keys and values */

    n = ajTableToarrayKeysValues(table, &keyarray, &valarray);

    AJCRESIZE0(table->Buckets, (size_t) table->Size, (size_t) iprime);
    table->Size = iprime;

    /* empty the old buckets */

    savelength = table->Length;
    ajTableClear(table);
    table->Length = savelength;

    /*ajDebug("ajTableResizeHashsize yes %x length %Lu nkeys %Lu hashsize %Lu\n",
      table, table->Length, n, table->Size);*/

    /* repopulate the new bucket array */

    for(i=0; i < n; i++)
    {
        ibucket = (*table->Fhash)(keyarray[i], table->Size);
        for(node = table->Buckets[ibucket]; node; node = node->Link)
            if((*table->Fcmp)(keyarray[i], node->Key) == 0)
                break;

        if(node == NULL)
        {
            if(tableFreeNext)
                node = tableFreeSet[--tableFreeNext];
            else
                AJNEW0(node);

            node->Key = keyarray[i];
            node->Link = table->Buckets[ibucket];
            table->Buckets[ibucket] = node;
        }

        node->Value = valarray[i];
    }

    AJFREE(keyarray);
    AJFREE(valarray);
    table->Timestamp++;

    return;
}




/* @func ajTableSetDestroy ****************************************************
**
** Sets the destructor functions for keys and values
**
** @param [u] table [AjPTable] Table
** @param [fN] keydel [void function] key destructor function
** @param [fN] valdel [void function] value destructor function
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTableSetDestroy(AjPTable table,
                       void (*keydel)(void** Pkey),
                       void (*valdel)(void** Pvalue))
{
    if(!table)
        return;

    table->Fkeydel = keydel;
    table->Fvaldel = valdel;

    return;
}




/* @func ajTableSetDestroyboth ************************************************
**
** Sets the destructor functions values to be the same as for keys
**
** @param [u] table [AjPTable] Table
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTableSetDestroyboth(AjPTable table)
{
    if(!table)
        return;

    table->Fvaldel = table->Fkeydel;

    return;
}




/* @func ajTableSetDestroykey *************************************************
**
** Sets the destructor functions for keys
**
** @param [u] table [AjPTable] Table
** @param [fN] keydel [void function] key destructor function
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTableSetDestroykey(AjPTable table,
                          void (*keydel)(void** Pkey))
{
    if(!table)
        return;

    table->Fkeydel = keydel;

    return;
}




/* @func ajTableSetDestroyvalue ***********************************************
**
** Sets the destructor functions for values
**
** @param [u] table [AjPTable] Table
** @param [fN] valdel [void function] value destructor function
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTableSetDestroyvalue(AjPTable table,
                            void (*valdel)(void** Pvalue))
{
    if(!table)
        return;

    table->Fvaldel = valdel;

    return;
}




/* @func ajTableSettypeChar ***************************************************
**
** Resets a hash table to use C character string keys
**
** @param [u] table [AjPTable] Hash table
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTableSettypeChar(AjPTable table)
{
    void** keyarray = NULL;
    void** valarray = NULL;

    ajulong n = 0UL;
    ajulong i = 0UL;

    n = ajTableToarrayKeysValues(table, &keyarray, &valarray);

    ajTableClear(table);

    table->Fcmp    = &ajTablecharCmp;
    table->Fhash   = &ajTablecharHash;
    table->Fkeydel = &ajMemFree;
    table->Type    = ajETableTypeChar;

    if(n)
    {
        for(i = 0UL; i < n; i++)
            ajTablePut(table, keyarray[i], valarray[i]);
    }

    AJFREE(keyarray);
    AJFREE(valarray);

    return;
}




/* @func ajTableSettypeCharCase ***********************************************
**
** Resets a hash table to use case-insensitive C character string keys
**
** @param [u] table [AjPTable] Hash table
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTableSettypeCharCase(AjPTable table)
{
    void** keyarray = NULL;
    void** valarray = NULL;

    ajulong n = 0UL;
    ajulong i = 0UL;

    n = ajTableToarrayKeysValues(table, &keyarray, &valarray);

    ajTableClear(table);

    table->Fcmp    = &ajTablecharCmpCase;
    table->Fhash   = &ajTablecharHashCase;
    table->Fkeydel = &ajMemFree;
    table->Type    = ajETableTypeChar;

    if(n)
    {
        for(i = 0UL; i < n; i++)
            ajTablePut(table, keyarray[i], valarray[i]);
    }

    AJFREE(keyarray);
    AJFREE(valarray);

    return;
}




/* @func ajTableSettypeDefault ************************************************
**
** Resets a hash table to use default keys
**
** @param [u] table [AjPTable] Hash table
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTableSettypeDefault(AjPTable table)
{
    void** keyarray = NULL;
    void** valarray = NULL;

    ajulong n = 0UL;
    ajulong i = 0UL;

    n = ajTableToarrayKeysValues(table, &keyarray, &valarray);

    ajTableClear(table);

    table->Fcmp    = &tableCmpAtom;
    table->Fhash   = &tableHashAtom;
    table->Fkeydel = NULL;
    table->Type    = ajETableTypeUnknown;

    if(n)
    {
        for(i = 0UL; i < n; i++)
            ajTablePut(table, keyarray[i], valarray[i]);
    }

    AJFREE(keyarray);
    AJFREE(valarray);

    return;
}




/* @func ajTableSettypeInt ****************************************************
**
** Resets a hash table to use integer keys
**
** @param [u] table [AjPTable] Hash table
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTableSettypeInt(AjPTable table)
{
    void** keyarray = NULL;
    void** valarray = NULL;

    ajulong n = 0UL;
    ajulong i = 0UL;

    n = ajTableToarrayKeysValues(table, &keyarray, &valarray);

    ajTableClear(table);

    table->Fcmp    = &ajTableintCmp;
    table->Fhash   = &ajTableintHash;
    table->Fkeydel = &ajMemFree;
    table->Type    = ajETableTypeInt;

    if(n)
    {
        for(i = 0Ul; i < n; i++)
            ajTablePut(table, keyarray[i], valarray[i]);
    }

    AJFREE(keyarray);
    AJFREE(valarray);

    return;
}




/* @func ajTableSettypeLong ***************************************************
**
** Resets a hash table to use long integer keys
**
** @param [u] table [AjPTable] Hash table
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTableSettypeLong(AjPTable table)
{
    void** keyarray = NULL;
    void** valarray = NULL;

    ajulong n = 0UL;
    ajulong i = 0UL;

    n = ajTableToarrayKeysValues(table, &keyarray, &valarray);

    ajTableClear(table);

    table->Fcmp    = &ajTablelongCmp;
    table->Fhash   = &ajTablelongHash;
    table->Fkeydel = &ajMemFree;
    table->Type    = ajETableTypeLong;

    if(n)
    {
        for(i = 0UL; i < n; i++)
            ajTablePut(table, keyarray[i], valarray[i]);
    }

    AJFREE(keyarray);
    AJFREE(valarray);

    return;
}




/* @func ajTableSettypeString *************************************************
**
** Resets a hash table to use string keys
**
** @param [u] table [AjPTable] Hash table
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTableSettypeString(AjPTable table)
{
    void** keyarray = NULL;
    void** valarray = NULL;

    ajulong n = 0UL;
    ajulong i = 0UL;

    n = ajTableToarrayKeysValues(table, &keyarray, &valarray);

    ajTableClear(table);

    table->Fcmp    = &ajTablestrCmp;
    table->Fhash   = &ajTablestrHash;
    table->Fkeydel = &tableDelStr;
    table->Type    = ajETableTypeStr;

    if(n)
    {
        for(i = 0UL; i < n; i++)
            ajTablePut(table, keyarray[i], valarray[i]);
    }

    AJFREE(keyarray);
    AJFREE(valarray);

    return;
}




/* @func ajTableSettypeStringCase *********************************************
**
** Resets a hash table to use case-insensitive string keys
**
** @param [u] table [AjPTable] Hash table
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTableSettypeStringCase(AjPTable table)
{
    void** keyarray = NULL;
    void** valarray = NULL;

    ajulong n = 0UL;
    ajulong i = 0UL;

    n = ajTableToarrayKeysValues(table, &keyarray, &valarray);

    ajTableClear(table);

    table->Fcmp    = &ajTablestrCmpCase;
    table->Fhash   = &ajTablestrHashCase;
    table->Fkeydel = &tableDelStr;
    table->Type    = ajETableTypeStr;

    if(n)
    {
        for(i = 0UL; i < n; i++)
            ajTablePut(table, keyarray[i], valarray[i]);
    }

    AJFREE(keyarray);
    AJFREE(valarray);

    return;
}




/* @func ajTableSettypeUint ***************************************************
**
** Resets a hash table to use unsigned integer keys
**
** @param [u] table [AjPTable] Hash table
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTableSettypeUint(AjPTable table)
{
    void** keyarray = NULL;
    void** valarray = NULL;

    ajulong n = 0UL;
    ajulong i = 0UL;

    n = ajTableToarrayKeysValues(table, &keyarray, &valarray);

    ajTableClear(table);

    table->Fcmp    = &ajTableuintCmp;
    table->Fhash   = &ajTableuintHash;
    table->Fkeydel = &ajMemFree;
    table->Type    = ajETableTypeUint;

    if(n)
    {
        for(i = 0UL; i < n; i++)
            ajTablePut(table, keyarray[i], valarray[i]);
    }

    AJFREE(keyarray);
    AJFREE(valarray);

    return;
}




/* @func ajTableSettypeUlong **************************************************
**
** Resets a hash table to use unsigned long integer keys
**
** @param [u] table [AjPTable] Hash table
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTableSettypeUlong(AjPTable table)
{
    void** keyarray = NULL;
    void** valarray = NULL;

    ajulong n = 0UL;
    ajulong i = 0UL;

    n = ajTableToarrayKeysValues(table, &keyarray, &valarray);

    ajTableClear(table);

    table->Fcmp    = &ajTableulongCmp;
    table->Fhash   = &ajTableulongHash;
    table->Fkeydel = &ajMemFree;
    table->Type    = ajETableTypeUlong;

    if(n)
    {
        for(i = 0UL; i < n; i++)
            ajTablePut(table, keyarray[i], valarray[i]);
    }

    AJFREE(keyarray);
    AJFREE(valarray);

    return;
}




/* @func ajTableSettypeUser ***************************************************
**
** Resets a hash table to use user-defined keys
**
**
** @param [u] table [AjPTable] Hash table
** @param [fN] cmp  [ajint function] function for comparing
** @param [fN] hash [ajulong function] function for hashing keys
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTableSettypeUser(AjPTable table,
                        ajint (*cmp)(const void* P1, const void* P2),
                        ajulong (*hash)(const void* key, ajulong hashsize))
{
    void** keyarray = NULL;
    void** valarray = NULL;

    ajulong n = 0UL;
    ajulong i = 0UL;

    n = ajTableToarrayKeysValues(table, &keyarray, &valarray);

    ajTableClear(table);

    table->Fcmp    = cmp;
    table->Fhash   = hash;
    table->Fkeydel = NULL;
    table->Type    = ajETableTypeUser;

    if(n)
    {
        for(i = 0UL; i < n; i++)
            ajTablePut(table, keyarray[i], valarray[i]);
    }

    AJFREE(keyarray);
    AJFREE(valarray);

    return;
}




/* @section Merging ***********************************************************
**
** Merging two tables with matchng key and value types
**
** @fdata [AjPTable]
**
** @fcategory modify
**
** @nam3rule Merge Merge two tables
** @nam4rule And   Keep keys in both tables
** @nam4rule Eor   Keep keys in one but not both tables
** @nam4rule Not   Keep keys in first table but not in second table
** @nam4rule Or    Keep all keys, removing duplicates
**
** @argrule * thys [AjPTable] Current table
** @argrule * table [AjPTable] Table to be merged
**
** @valrule * [AjBool] true on success
******************************************************************************/




/* @func ajTableMergeAnd ******************************************************
**
** Merge two tables, keeping all keys that are in both tables
**
** @param [u] thys [AjPTable] Current table
** @param [u] table [AjPTable] Table to be merged
**
** @return [AjBool] True on success
**
**
** @release 6.4.0
******************************************************************************/

AjBool ajTableMergeAnd(AjPTable thys, AjPTable table)
{
    ajulong i = 0UL;

    AjBool ismatch = ajFalse;

    AjPTableNode p       = NULL;
    AjPTableNode q       = NULL;
    AjPTableNode tmpnode = NULL;
    AjPTableNode pprev   = NULL;
    AjPTableNode pfirst  = NULL;
    AjPTableNode qfirst  = NULL;

    /* first we make both tables have the same hashsize */

    if(table->Size > thys->Size)
        ajTableResizeHashsize(thys, table->Size);
    else if(thys->Size > table->Size)
        ajTableResizeHashsize(table, thys->Size);

    /* compare keys and find matches */

    for(i = 0UL; i < thys->Size; i++)
    {
        pfirst = thys->Buckets[i];
        qfirst = table->Buckets[i];

        pprev = NULL;
        p = pfirst;

        while(p)
        {
            ismatch = ajFalse;

            for(q = qfirst; q; q = q->Link)
            {
                if((*thys->Fcmp)(p->Key, q->Key) == 0)
                {
                    ismatch = ajTrue;
                    break;
                }
            }

            if(ismatch)
            {
                pprev = p;
                p = p->Link;
            }
            else
            {
                tmpnode = p->Link;

                if(thys->Fkeydel)
                    (*thys->Fkeydel)(&p->Key);
                if(thys->Fvaldel)
                    (*thys->Fvaldel)(&p->Value);

                if(tableFreeNext >= tableFreeMax)
                    tableFreeSetExpand();
                if(tableFreeNext >= tableFreeMax)
                    AJFREE(p);
                else
                    tableFreeSet[tableFreeNext++] = p;

                p = tmpnode;

                if(pprev)
                    pprev->Link = tmpnode;
                else
                    thys->Buckets[i] = tmpnode;

                --thys->Length;
            }
        }

        for(q = qfirst; q; q = tmpnode)
        {
            tmpnode = q->Link;

            if(table->Fkeydel)
                (*table->Fkeydel)(&q->Key);

            if(table->Fvaldel)
                (*table->Fvaldel)(&q->Value);

            if(tableFreeNext >= tableFreeMax)
                tableFreeSetExpand();
            if(tableFreeNext >= tableFreeMax)
                AJFREE(q);
            else
                tableFreeSet[tableFreeNext++] = q;

            --table->Length;
        }
        table->Buckets[i] = NULL;
    }

    return ajTrue;
}




/* @func ajTableMergeEor ******************************************************
**
** Merge two tables, keeping all keys that are in only one table
**
** @param [u] thys [AjPTable] Current table
** @param [u] table [AjPTable] Table to be merged
**
** @return [AjBool] True on success
**
**
** @release 6.4.0
******************************************************************************/

AjBool ajTableMergeEor(AjPTable thys, AjPTable table)
{
    ajulong i = 0UL;

    AjBool ismatch = ajFalse;

    AjPTableNode p       = NULL;
    AjPTableNode q       = NULL;
    AjPTableNode tmpnode = NULL;
    AjPTableNode pprev   = NULL;
    AjPTableNode qprev   = NULL;
    AjPTableNode pfirst  = NULL;
    AjPTableNode qfirst  = NULL;
    AjPTableNode qtmp    = NULL;

    /* first we make both tables have the same hashsize */

    if(table->Size > thys->Size)
        ajTableResizeHashsize(thys, table->Size);
    else if(thys->Size > table->Size)
        ajTableResizeHashsize(table, thys->Size);

    /* compare keys and find matches */

    for(i=0; i < thys->Size; i++)
    {
        pfirst = thys->Buckets[i];
        qfirst = table->Buckets[i];

        pprev = NULL;
        p = pfirst;
        while(p)
        {
            ismatch = ajFalse;

            qprev = NULL;
            for(q = qfirst; q; q = q->Link)
            {
                if((*thys->Fcmp)(p->Key, q->Key) == 0)
                {
                    ismatch = ajTrue;
                    break;
                }
                qprev = q;
            }

            if(!ismatch)
            {
                pprev = p;
                p = p->Link;
            }
            else
            {
                tmpnode = p->Link;

                if(thys->Fkeydel)
                    (*thys->Fkeydel)(&p->Key);
                if(thys->Fvaldel)
                    (*thys->Fvaldel)(&p->Value);

                if(tableFreeNext >= tableFreeMax)
                    tableFreeSetExpand();
                if(tableFreeNext >= tableFreeMax)
                    AJFREE(p);
                else
                    tableFreeSet[tableFreeNext++] = p;

                p = tmpnode;

                if(pprev)
                    pprev->Link = tmpnode;
                else
                    thys->Buckets[i] = tmpnode;

                --thys->Length;

                tmpnode = q->Link;

                if(table->Fkeydel)
                    (*table->Fkeydel)(&q->Key);

                if(table->Fvaldel)
                    (*table->Fvaldel)(&q->Value);

                if(tableFreeNext >= tableFreeMax)
                    tableFreeSetExpand();
                if(tableFreeNext >= tableFreeMax)
                    AJFREE(q);
                else
                    tableFreeSet[tableFreeNext++] = q;


                if(qprev)
                    qprev->Link = tmpnode;
                else
                    qfirst = table->Buckets[i] = tmpnode;

                --table->Length;
            }
        }

        for(q = table->Buckets[i]; q; q = qtmp)
        {
            qtmp = q->Link;
            tmpnode = thys->Buckets[i];
            thys->Buckets[i] = q;
            q->Link = tmpnode;

            --table->Length;
            ++thys->Length;
        }
        table->Buckets[i] = NULL;
    }

    return ajTrue;
}




/* @func ajTableMergeNot ******************************************************
**
** Merge two tables, removing all keys that are not in the new table
**
** @param [u] thys [AjPTable] Current table
** @param [u] table [AjPTable] Table to be merged
**
** @return [AjBool] True on success
**
**
** @release 6.4.0
******************************************************************************/

AjBool ajTableMergeNot(AjPTable thys, AjPTable table)
{
    ajulong i = 0UL;

    AjBool ismatch = ajFalse;

    AjPTableNode p       = NULL;
    AjPTableNode q       = NULL;
    AjPTableNode tmpnode = NULL;
    AjPTableNode pprev   = NULL;
    AjPTableNode pfirst  = NULL;
    AjPTableNode qfirst  = NULL;

    /* first we make both tables have the same hashsize */

    if(table->Size > thys->Size)
        ajTableResizeHashsize(thys, table->Size);
    else if(thys->Size > table->Size)
        ajTableResizeHashsize(table, thys->Size);

    /* compare keys and find matches */

    for(i=0UL; i < thys->Size; i++)
    {
        pfirst = thys->Buckets[i];
        qfirst = table->Buckets[i];

        pprev = NULL;
        p = pfirst;

        while(p)
        {
            ismatch = ajFalse;

            for(q = qfirst; q; q = q->Link)
            {
                if((*thys->Fcmp)(p->Key, q->Key) == 0)
                {
                    ismatch = ajTrue;
                    break;
                }
            }

            if(!ismatch)
            {
                pprev = p;
                p = p->Link;
            }
            else
            {
                tmpnode = p->Link;

                if(thys->Fkeydel)
                    (*thys->Fkeydel)(&p->Key);
                if(thys->Fvaldel)
                    (*thys->Fvaldel)(&p->Value);

                if(tableFreeNext >= tableFreeMax)
                    tableFreeSetExpand();
                if(tableFreeNext >= tableFreeMax)
                    AJFREE(p);
                else
                    tableFreeSet[tableFreeNext++] = p;

                p = tmpnode;

                if(pprev)
                    pprev->Link = tmpnode;
                else
                    thys->Buckets[i] = tmpnode;

                --thys->Length;
            }
        }
        for(q = qfirst; q; q = tmpnode)
        {
            tmpnode = q->Link;

            if(table->Fkeydel)
                (*table->Fkeydel)(&q->Key);

            if(table->Fvaldel)
                (*table->Fvaldel)(&q->Value);

            if(tableFreeNext >= tableFreeMax)
                tableFreeSetExpand();
            if(tableFreeNext >= tableFreeMax)
                AJFREE(q);
            else
                tableFreeSet[tableFreeNext++] = q;

            --table->Length;
        }
        table->Buckets[i] = NULL;
    }

    return ajTrue;
}




/* @func ajTableMergeOr *******************************************************
**
** Merge two tables, adding all unique keys from the new table
**
** @param [u] thys [AjPTable] Current table
** @param [u] table [AjPTable] Table to be merged
**
** @return [AjBool] True on success
**
**
** @release 6.4.0
******************************************************************************/

AjBool ajTableMergeOr(AjPTable thys, AjPTable table)
{
    ajulong i = 0UL;

    AjBool ismatch = ajFalse;

    AjPTableNode p       = NULL;
    AjPTableNode q       = NULL;
    AjPTableNode tmpnode = NULL;
    AjPTableNode qprev   = NULL;
    AjPTableNode pfirst  = NULL;
    AjPTableNode qfirst  = NULL;
    AjPTableNode qtmp    = NULL;

    /* first we make both tables have the same hashsize */

    if(table->Size > thys->Size)
        ajTableResizeHashsize(thys, table->Size);
    else if(thys->Size > table->Size)
        ajTableResizeHashsize(table, thys->Size);

    /* compare keys and find matches */

    for(i=0UL; i < thys->Size; i++)
    {
        pfirst = thys->Buckets[i];
        qfirst = table->Buckets[i];

        p = pfirst;

        while(p)
        {
            ismatch = ajFalse;

            qprev = NULL;
            for(q = qfirst; q; q = q->Link)
            {
                if((*thys->Fcmp)(p->Key, q->Key) == 0)
                {
                    ismatch = ajTrue;
                    break;
                }
                qprev = q;
            }

            p = p->Link;

            if(ismatch)
            {
                tmpnode = q->Link;

                if(table->Fkeydel)
                    (*table->Fkeydel)(&q->Key);
                if(table->Fvaldel)
                    (*table->Fvaldel)(&q->Value);

                if(tableFreeNext >= tableFreeMax)
                    tableFreeSetExpand();
                if(tableFreeNext >= tableFreeMax)
                    AJFREE(q);
                else
                    tableFreeSet[tableFreeNext++] = q;

                q = tmpnode;

                if(qprev)
                    qprev->Link = tmpnode;
                else
                    qfirst = table->Buckets[i] = tmpnode;

                --table->Length;
            }
        }

        for(q = table->Buckets[i]; q; q = qtmp)
        {
            qtmp = q->Link;
            tmpnode = thys->Buckets[i];
            thys->Buckets[i] = q;
            q->Link = tmpnode;

            --table->Length;
            ++thys->Length;
        }
        table->Buckets[i] = NULL;
    }

    return ajTrue;
}




/* @section Comparison functions **********************************************
**
** @fdata [AjPTable]
**
** Comparison functions for table keys
**
** @nam3rule Cmp Comparison
** @nam3rule Hash Hashing keys
**
** @argrule Cmp key1 [const void*] First key
** @argrule Cmp key2 [const void*] Second key
** @argrule Hash key [const void*] Key
** @argrule Hash hashsize [ajulong] Hash table size
** @suffix Case Case insensitive keys
**
** @valrule Cmp [ajint] Comparison result 0 for a match,
**                      -1 or +1 for a mismatch
** @valrule Hash [ajulong] hash value
**
** @fcategory misc
**
******************************************************************************/




/* @funcstatic tableCmpAtom ***************************************************
**
** Default comparison function for key comparison
**
** @param [r] key1 [const void*] First key
** @param [r] key2 [const void*] Second key
** @return [ajint] 0 for success, 1 for different keys
**
** @release 2.0.0
** @@
******************************************************************************/


static ajint tableCmpAtom(const void* key1, const void* key2)
{
    return (ajint) (key1 != key2);
}




/* @funcstatic tableHashAtom **************************************************
**
** Default hash function for key indexing
**
** @param [r] key [const void*] Key
** @param [r] hashsize [ajulong] Hash size (maximum hash value)
** @return [ajulong] Hash value in range 0 to hashsize-1
**
** @release 2.0.0
** @@
******************************************************************************/

static ajulong tableHashAtom(const void* key, ajulong hashsize)
{
    return((unsigned long)key>>2) % hashsize;
}




/* @section exit **************************************************************
**
** Functions called on exit from the program by ajExit to do
** any necessary cleanup and to report internal statistics to the debug file
**
** @fdata      [AjPTable]
** @fnote     general exit functions, no arguments
**
** @nam3rule Exit Cleanup and report on exit
**
** @valrule * [void]
**
** @fcategory misc
******************************************************************************/




/* @func ajTableExit **********************************************************
**
** Prints a summary of table usage with debug calls
**
** @return [void]
**
** @release 2.8.0
** @@
******************************************************************************/

void ajTableExit(void)
{
    ajulong i = 0UL;

#ifdef AJ_SAVESTATS
    ajDebug("Table usage : %d opened, %d closed, %d maxsize, %d maxmem\n",
            tableNewCnt, tableDelCnt, tableMaxNum, tableMaxMem);
#endif

    if(tableFreeNext)
        for(i = 0UL; i < tableFreeNext; i++)
            AJFREE(tableFreeSet[i]);

    if(tableFreeSet)
        AJFREE(tableFreeSet);

    ajStrDel(&tableTmpkeyStr);

    tableFreeNext = 0UL;
    tableFreeMax  = 0UL;

    return;
}




/* @datasection [AjPTable] Character hash tables ******************************
**
** @nam2rule Tablechar
**
******************************************************************************/




/* @section Constructors ******************************************************
**
** Constructors for hash tables
**
** @fdata [AjPTable]
**
** @fcategory new
**
** @nam3rule New Constructor
** @nam4rule Case Case-sensitive keys
** @suffix Const Constant keys with no destructor
**
** @argrule New size [ajulong] Number of key values
**
** @valrule * [AjPTable] New hash table
**
******************************************************************************/




/* @func ajTablecharNew *******************************************************
**
** Creates a table with a character string key.
**
** @param [r] size [ajulong] number of key-value pairs
** @return [AjPTable] New table object with a character string key.
**
** @release 5.0.0
** @@
******************************************************************************/

AjPTable ajTablecharNew(ajulong size)
{
    AjPTable table = NULL;

    table = ajTableNewFunctionLen(size, &ajTablecharCmp, &ajTablecharHash,
                                  &ajMemFree, NULL);
    table->Type = ajETableTypeChar;

    return table;
}




/* @func ajTablecharNewCase ***************************************************
**
** Creates a table with a character string key and case insensitive searching.
**
** @param [r] size [ajulong] Hash size estimate.
** @return [AjPTable] New table object with a character string key.
**
** @release 5.0.0
** @@
******************************************************************************/

AjPTable ajTablecharNewCase(ajulong size)
{
    AjPTable table = NULL;

    table = ajTableNewFunctionLen(size, &ajTablecharCmpCase,
                                  &ajTablecharHashCase, &ajMemFree, NULL);
    table->Type = ajETableTypeChar;

    return table;
}




/* @func ajTablecharNewCaseConst **********************************************
**
** Creates a table with a character string key and case insensitive searching.
**
** @param [r] size [ajulong] Hash size estimate.
** @return [AjPTable] New table object with a character string key.
**
** @release 6.4.0
** @@
******************************************************************************/

AjPTable ajTablecharNewCaseConst(ajulong size)
{
    AjPTable table = NULL;

    table = ajTableNewFunctionLen(size, &ajTablecharCmpCase,
                                  &ajTablecharHashCase,
                                  NULL, NULL);
    table->Type = ajETableTypeChar;

    return table;
}




/* @func ajTablecharNewConst **************************************************
**
** Creates a table with a character string key.
**
** @param [r] size [ajulong] number of key-value pairs
** @return [AjPTable] New table object with a character string key.
**
** @release 6.4.0
** @@
******************************************************************************/

AjPTable ajTablecharNewConst(ajulong size)
{
    AjPTable table = NULL;

    table = ajTableNewFunctionLen(size, &ajTablecharCmp, &ajTablecharHash,
                                  NULL, NULL);
    table->Type = ajETableTypeChar;

    return table;
}




/* @section Trace functions ***************************************************
**
** @fdata [AjPTable]
**
** @nam3rule Print Trace contents to standard error
** @nam3rule Trace Trace contents to debug file
**
** @argrule * table [const AjPTable] Hash table
**
** @valrule * [void]
**
** @fcategory misc
**
******************************************************************************/




/* @func ajTablecharPrint *****************************************************
**
** Print function for a table with a character string key.
**
** @param [r] table [const AjPTable] Table.
** @return [void]
**
** @release 5.0.0
** @@
******************************************************************************/

void ajTablecharPrint(const AjPTable table)
{
    ajulong i = 0UL;

    AjPTableNode node = NULL;

    if(!table)
        return;

    for(i = 0UL; i < table->Size; i++)
        for(node = table->Buckets[i]; node; node = node->Link)
        {
            ajUser("key '%s' value '%s'",
                   (const char*) node->Key, (char*) node->Value);
        }

    return;
}




/* @section Comparison functions **********************************************
**
** @fdata [AjPTable]
**
** Comparison functions for table keys
**
** @nam3rule Cmp Comparison
** @nam3rule Hash Hashing keys
**
** @argrule Cmp key1 [const void*] First key
** @argrule Cmp key2 [const void*] Second key
** @argrule Hash key [const void*] Key
** @argrule Hash hashsize [ajulong] Hash table size
** @suffix Case Case insensitive keys
**
** @valrule Cmp [ajint] Comparison result 0 for a match,
**                      -1 or +1 for a mismatch
** @valrule Hash [ajulong] hash value
**
** @fcategory misc
**
******************************************************************************/




/* @func ajTablecharCmp *******************************************************
**
** Comparison function for a table with a character string key
**
** @param [r] key1 [const void*] First key.
** @param [r] key2 [const void*] Second key.
** @return [ajint] Comparison result. Zero if equal, non-zero if different.
**
** @release 5.0.0
** @@
******************************************************************************/

ajint ajTablecharCmp(const void* key1, const void* key2)
{
    return (ajint) strcmp((const char*) key1, (const char*) key2);
}




/* @func ajTablecharCmpCase ***************************************************
**
** Comparison function for a table with a character string key
** and case insensitivity.
**
** @param [r] key1 [const void*] First key.
** @param [r] key2 [const void*] Second key.
** @return [ajint] Comparison result. Zero if equal, non-zero if different.
**
** @release 5.0.0
** @@
******************************************************************************/

ajint ajTablecharCmpCase(const void* key1, const void* key2)
{
    return (ajint) ajCharCmpCase((const char*) key1, (const char*) key2);
}




/* @func ajTablecharHash ******************************************************
**
** Hash function for a table with a character string key
**
** @param [r] key [const void*] Standard argument. Table key.
** @param [r] hashsize [ajulong] Standard argument. Estimated Hash size.
** @return [ajulong] Hash value.
**
** @release 5.0.0
** @@
******************************************************************************/

ajulong ajTablecharHash(const void* key, ajulong hashsize)
{
    ajulong hash = 0UL;

    const char* s = NULL;

    s = (const char*) key;

    for(hash = 0UL; *s; s++)
        hash = (hash * 127 + *s) % hashsize;

    return hash;
}




/* @func ajTablecharHashCase **************************************************
**
** Hash function for a table with a character string key and
** case insensitivity.
**
** @param [r] key [const void*] Standard argument. Table key.
** @param [r] hashsize [ajulong] Standard argument. Estimated Hash size.
** @return [ajulong] Hash value.
**
** @release 5.0.0
** @@
******************************************************************************/

ajulong ajTablecharHashCase(const void* key, ajulong hashsize)
{
    ajulong hash = 0UL;

    const char* s = NULL;

    s = (const char*) key;

    for(hash = 0UL; *s; s++)
        hash = (hash * 127 + toupper((ajint)*s)) % hashsize;

    return hash;
}




/* @datasection [AjPTable] Integer hash tables ********************************
**
** @nam2rule Tableint Integer hash tables
**
******************************************************************************/




/* @section Constructors ******************************************************
**
** Constructors for hash tables
**
** @fdata [AjPTable]
**
** @fcategory new
**
** @nam3rule New Constructor
** @nam4rule Case Case-sensitive keys
** @suffix Const Constant keys with no destructor
**
** @argrule New size [ajulong] Number of key values
**
** @valrule * [AjPTable] New hash table
**
******************************************************************************/




/* @func ajTableintNew ********************************************************
**
** Creates, initialises, and returns a new, empty table that can hold a
** specified number of integer key-value pairs.
**
** @param [r] size [ajulong] estimate of number of unique keys
**
** @return [AjPTable] new table.
**
** @release 6.4.0
** @@
**
******************************************************************************/

AjPTable ajTableintNew(ajulong size)
{
    AjPTable table = NULL;

    table = ajTableNewFunctionLen(size, &ajTableintCmp, &ajTableintHash,
                                  &ajMemFree, NULL);
    table->Type = ajETableTypeInt;

    return table;
}




/* @func ajTableintNewConst ***************************************************
**
** Creates, initialises, and returns a new, empty table that can hold a
** specified number of integer key-value pairs.
**
** @param [r] size [ajulong] estimate of number of unique keys
**
** @return [AjPTable] new table.
**
** @release 6.4.0
** @@
**
******************************************************************************/

AjPTable ajTableintNewConst(ajulong size)
{
    AjPTable table = NULL;

    table = ajTableNewFunctionLen(size, &ajTableintCmp, &ajTableintHash,
                                  NULL, NULL);
    table->Type = ajETableTypeInt;

    return table;
}




/* @section Destructors *******************************************************
**
** @fdata [AjPTable]
**
** Destructors know they are dealing with strings and can
** clean up keys and values
**
** @nam3rule Free Delete table, keys and values
** @nam4rule FreeKey Delete table, keys and values
**
** @argrule * Ptable [AjPTable*] Hash table
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajTableintFree *******************************************************
**
** Free keys and value strings in a table and free the table.  Use
** only where the keys and data in the table are real, and not just
** copies of pointers. Otherwise a call to ajTableFree is enough.
**
** @param [d] Ptable [AjPTable*] Table
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTableintFree(AjPTable* Ptable)
{
    if(!Ptable)
        return;

    if(!*Ptable)
        return;

    ajTableMapDel(*Ptable, &tableDel, NULL);

    ajTableFree(Ptable);

    return;
}




/* @func ajTableintFreeKey ****************************************************
**
** Free keys and value strings in a table and free the table.
** Use only where the keys
** in the table are real, and not just copies of pointers. Otherwise
** a call to ajTableFree is enough.
**
** @param [d] Ptable [AjPTable*] Table
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTableintFreeKey(AjPTable* Ptable)
{
    if(!Ptable)
        return;

    if(!*Ptable)
        return;

    ajTableMapDel(*Ptable, &tableDelKey, NULL);

    ajTableFree(Ptable);

    return;
}




/* @section Retrieval *********************************************************
**
** @fdata [AjPTable]
**
** Retrieves values from a hash table
**
** @nam3rule Fetch Retrieval function
**
** @argrule Fetch table [const AjPTable] Hash table
** @argrule Fetch intkey [const ajint*] Key
**
** @valrule Fetch [const ajint*] Value
**
** @fcategory cast
**
******************************************************************************/




/* @func ajTableintFetch ******************************************************
**
** returns the value associated with key in table, or null
** if table does not hold key.
**
** @param [r] table [const AjPTable] table to search
** @param [r] intkey [const ajint*] key to find.
** @return [const ajint*]  value associated with key
** @error NULL if key not found in table.
**
** @release 6.4.0
** @@
******************************************************************************/

const ajint* ajTableintFetch(const AjPTable table, const ajint* intkey)
{
    ajulong i = 0UL;

    AjPTableNode node = NULL;

    if(!table)
        return NULL;

    if(!intkey)
        return NULL;

    i = (*table->Fhash)(intkey, table->Size);

    for(node = table->Buckets[i]; node; node = node->Link)
        if((*table->Fcmp)(intkey, node->Key) == 0)
            break;

    return node ? (const ajint*) node->Value : NULL;
}




/* @section Modify ************************************************************
**
** @fdata [AjPTable]
**
** Updates values from a hash table
**
** @nam3rule Fetchmod Retrieval function
**
** @argrule * table [AjPTable] Hash table
** @argrule Fetchmod intkey [const ajint*] Key
**
** @valrule Fetchmod [ajint*] Value
**
** @fcategory modify
**
******************************************************************************/




/* @func ajTableintFetchmod ***************************************************
**
** returns the value associated with key in table, or null
** if table does not hold key.
**
** @param [u] table [AjPTable] table to search
** @param [r] intkey   [const ajint*] key to find.
** @return [ajint*]  value associated with key
** @error NULL if key not found in table.
**
** @release 6.4.0
** @@
******************************************************************************/

ajint* ajTableintFetchmod(AjPTable table, const ajint* intkey)
{
    ajulong i = 0UL;

    AjPTableNode node = NULL;

    if(!table)
        return NULL;

    if(!intkey)
        return NULL;

    i = (*table->Fhash)(intkey, table->Size);

    for(node = table->Buckets[i]; node; node = node->Link)
        if((*table->Fcmp)(intkey, node->Key) == 0)
            break;

    return node ? (ajint*) (&node->Value) : NULL;
}




/* @section Comparison functions **********************************************
**
** @fdata [AjPTable]
**
** Comparison functions for table keys
**
** @nam3rule Cmp Comparison
** @nam3rule Hash Hashing keys
**
** @argrule Cmp key1 [const void*] First key
** @argrule Cmp key2 [const void*] Second key
** @argrule Hash key [const void*] Key
** @argrule Hash hashsize [ajulong] Hash table size
** @suffix Case Case insensitive keys
**
** @valrule Cmp [ajint] Comparison result 0 for a match,
**                      -1 or +1 for a mismatch
** @valrule Hash [ajulong] hash value
**
** @fcategory misc
**
******************************************************************************/




/* @func ajTableintCmp ********************************************************
**
** Comparison function for a table with an integer key
**
** @param [r] key1 [const void*] First key.
** @param [r] key2 [const void*] Second key.
** @return [ajint] Comparison result. Zero if equal, non-zero if different.
**
** @release 6.4.0
** @@
******************************************************************************/

ajint ajTableintCmp(const void* key1, const void* key2)
{
    return (ajint) (*((const ajint*) key1) != *((const ajint*) key2));
}




/* @func ajTableintHash *******************************************************
**
** Hash function for a table with an integer key
**
** @param [r] key [const void*] Standard argument. Table key.
** @param [r] hashsize [ajulong] Standard argument. Estimated Hash size.
** @return [ajulong] Hash value in range 0 to hashsize-1
**
** @release 6.4.0
** @@
******************************************************************************/

ajulong ajTableintHash(const void* key, ajulong hashsize)
{
    ajulong hash = 0UL;

    const ajint* ia = NULL;

    if(!key)
        return 0;

    if(!hashsize)
        return 0;

    ia = (const ajint*) key;

    hash = (*ia >> 2) % hashsize;

    return hash;
}




/* @datasection [AjPTable] Long integer hash tables ***************************
**
** @nam2rule Tablelong Long integer hash tables
**
******************************************************************************/




/* @section Constructors ******************************************************
**
** Constructors for hash tables
**
** @fdata [AjPTable]
**
** @fcategory new
**
** @nam3rule New Constructor
** @suffix Const Constant keys with no destructor
**
** @argrule New size [ajulong] Number of key values
**
** @valrule * [AjPTable] New hash table
**
******************************************************************************/




/* @func ajTablelongNew *******************************************************
**
** Creates, initialises, and returns a new, empty table that can hold a
** specified number of long integer key-value pairs.
**
** @param [r] size [ajulong] estimate of number of unique keys
**
** @return [AjPTable] new table.
**
** @release 6.4.0
** @@
**
******************************************************************************/

AjPTable ajTablelongNew(ajulong size)
{
    AjPTable table = NULL;

    table = ajTableNewFunctionLen(size, &ajTablelongCmp, &ajTablelongHash,
                                  &ajMemFree, NULL);
    table->Type = ajETableTypeLong;

    return table;
}




/* @func ajTablelongNewConst **************************************************
**
** Creates, initialises, and returns a new, empty table that can hold a
** specified number of long integer key-value pairs.
**
** @param [r] size [ajulong] estimate of number of unique keys
**
** @return [AjPTable] new table.
**
** @release 6.4.0
** @@
**
******************************************************************************/

AjPTable ajTablelongNewConst(ajulong size)
{
    AjPTable table = NULL;

    table = ajTableNewFunctionLen(size, &ajTablelongCmp, &ajTablelongHash,
                                  NULL, NULL);
    table->Type = ajETableTypeLong;

    return table;
}




/* @section Destructors *******************************************************
**
** @fdata [AjPTable]
**
** Destructors know they are dealing with strings and can
** clean up keys and values
**
** @nam3rule Free Delete table, keys and values
** @nam4rule FreeKey Delete table, keys and values
**
** @argrule * Ptable [AjPTable*] Hash table
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajTablelongFree ******************************************************
**
** Free keys and value strings in a table and free the table.  Use
** only where the keys and data in the table are real, and not just
** copies of pointers. Otherwise a call to ajTableFree is enough.
**
** @param [d] Ptable [AjPTable*] Table
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTablelongFree(AjPTable* Ptable)
{
    if(!Ptable)
        return;

    if(!*Ptable)
        return;

    ajTableMapDel(*Ptable, &tableDel, NULL);

    ajTableFree(Ptable);

    return;
}




/* @func ajTablelongFreeKey ***************************************************
**
** Free keys and value strings in a table and free the table.  Use
** only where the keys in the table are real, and not just copies of
** pointers. Otherwise a call to ajTableFree is enough.
**
** @param [d] Ptable [AjPTable*] Table
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTablelongFreeKey(AjPTable* Ptable)
{
    if(!Ptable)
        return;

    if(!*Ptable)
        return;

    ajTableMapDel(*Ptable, &tableDelKey, NULL);

    ajTableFree(Ptable);

    return;
}




/* @section Retrieval *********************************************************
**
** @fdata [AjPTable]
**
** Retrieves values from a hash table
**
** @nam3rule Fetch Retrieval function
**
** @argrule * table [const AjPTable] Hash table
** @argrule Fetch longkey [const ajlong*] Key
**
** @valrule Fetch [const ajlong*] Value
** @fcategory cast
**
******************************************************************************/




/* @func ajTablelongFetch *****************************************************
**
** returns the value associated with key in table, or null
** if table does not hold key.
**
** @param [r] table [const AjPTable] table to search
** @param [r] longkey [const ajlong*] key to find.
** @return [const ajlong*] value associated with key
** @error NULL if key not found in table.
**
** @release 6.4.0
** @@
******************************************************************************/

const ajlong* ajTablelongFetch(const AjPTable table, const ajlong* longkey)
{
    ajulong i = 0UL;

    AjPTableNode node = NULL;

    if(!table)
        return NULL;

    if(!longkey)
        return NULL;

    i = (*table->Fhash)(longkey, table->Size);

    for(node = table->Buckets[i]; node; node = node->Link)
        if((*table->Fcmp)(longkey, node->Key) == 0)
            break;

    return node ? (const ajlong*) node->Value : NULL;
}




/* @section Modify ************************************************************
**
** @fdata [AjPTable]
**
** Updates values from a hash table
**
** @nam3rule Fetchmod Retrieval function
**
** @argrule * table [AjPTable] Hash table
** @argrule Fetchmod longkey [const ajlong*] Key
**
** @valrule Fetchmod [ajlong*] Value
**
** @fcategory modify
**
******************************************************************************/




/* @func ajTablelongFetchmod **************************************************
**
** returns the value associated with key in table, or null
** if table does not hold key.
**
** @param [u] table [AjPTable] table to search
** @param [r] longkey   [const ajlong*] key to find.
** @return [ajlong*]  value associated with key
** @error NULL if key not found in table.
**
** @release 6.4.0
** @@
******************************************************************************/

ajlong* ajTablelongFetchmod(AjPTable table, const ajlong* longkey)
{
    ajulong i = 0UL;

    AjPTableNode node = NULL;

    if(!table)
        return NULL;

    if(!longkey)
        return NULL;

    i = (*table->Fhash)(longkey, table->Size);

    for(node = table->Buckets[i]; node; node = node->Link)
        if((*table->Fcmp)(longkey, node->Key) == 0)
            break;

    return node ? (ajlong*) (&node->Value) : NULL;
}




/* @section Comparison functions **********************************************
**
** @fdata [AjPTable]
**
** Comparison functions for table keys
**
** @nam3rule Cmp Comparison
** @nam3rule Hash Hashing keys
**
** @argrule Cmp key1 [const void*] First key
** @argrule Cmp key2 [const void*] Second key
** @argrule Hash key [const void*] Key
** @argrule Hash hashsize [ajulong] Hash table size
** @suffix Case Case insensitive keys
**
** @valrule Cmp [ajint] Comparison result 0 for a match,
**                      -1 or +1 for a mismatch
** @valrule Hash [ajulong] hash value
**
** @fcategory misc
**
******************************************************************************/




/* @func ajTablelongCmp *******************************************************
**
** Comparison function for a table with a long integer key
**
** @param [r] key1 [const void*] First key.
** @param [r] key2 [const void*] Second key.
** @return [ajint] Comparison result. Zero if equal, non-zero if different.
**
** @release 6.4.0
** @@
******************************************************************************/

ajint ajTablelongCmp(const void* key1, const void* key2)
{
    return (ajint) (*((const ajlong*) key1) != *((const ajlong*) key2));
}




/* @func ajTablelongHash ******************************************************
**
** Hash function for a table with a long integer key
**
** @param [r] key [const void*] Standard argument. Table key.
** @param [r] hashsize [ajulong] Standard argument. Estimated Hash size.
** @return [ajulong] Hash value in range 0 to hashsize-1
**
** @release 6.4.0
** @@
******************************************************************************/

ajulong ajTablelongHash(const void* key, ajulong hashsize)
{
    ajulong hash = 0UL;

    const ajlong* ia = NULL;

    if(!key)
        return 0;

    if(!hashsize)
        return 0;

    ia = (const ajlong*) key;

    hash = (*ia >> 2) % hashsize;

    return hash;
}




/* @datasection [AjPTable] String hash tables *********************************
**
** @nam2rule Tablestr String hash tables
**
******************************************************************************/




/* @section Constructors ******************************************************
**
** Constructors for hash tables
**
** @fdata [AjPTable]
**
** @fcategory new
**
** @nam3rule New Constructor
** @nam4rule Case Case-sensitive keys
** @suffix Const Constant keys with no destructor
**
** @argrule New size [ajulong] Number of key values
**
** @valrule * [AjPTable] New hash table
**
******************************************************************************/




/* @func ajTablestrNew ********************************************************
**
** Creates, initialises, and returns a new, empty table that can hold a
** specified number of key-value pairs.
**
** @param [r] size [ajulong] estimate of number of unique keys
**
** @return [AjPTable] new table.
**
** @release 5.0.0
** @@
**
******************************************************************************/

AjPTable ajTablestrNew(ajulong size)
{
    AjPTable table = NULL;

    table = ajTableNewFunctionLen(size, &ajTablestrCmp, &ajTablestrHash,
                                  &tableDelStr, NULL);
    table->Type = ajETableTypeStr;

    return table;
}




/* @func ajTablestrNewCase ****************************************************
**
** Creates, initialises, and returns a new, empty table that can hold a
** specified number of key-value pairs.
**
** @param [r] size [ajulong] estimate of number of unique keys
**
** @return [AjPTable] new table.
**
** @release 5.0.0
** @@
**
******************************************************************************/

AjPTable ajTablestrNewCase(ajulong size)
{
    AjPTable table = NULL;

    table = ajTableNewFunctionLen(size, &ajTablestrCmpCase, &ajTablestrHashCase,
                                  &tableDelStr, NULL);
    table->Type = ajETableTypeStr;

    return table;
}




/* @func ajTablestrNewCaseConst ***********************************************
**
** Creates, initialises, and returns a new, empty table that can hold a
** specified number of key-value pairs.
**
** @param [r] size [ajulong] estimate of number of unique keys
**
** @return [AjPTable] new table.
**
** @release 6.4.0
** @@
**
******************************************************************************/

AjPTable ajTablestrNewCaseConst(ajulong size)
{
    AjPTable table = NULL;

    table = ajTableNewFunctionLen(size, &ajTablestrCmpCase, &ajTablestrHashCase,
                                  NULL, NULL);
    table->Type = ajETableTypeStr;

    return table;
}




/* @func ajTablestrNewConst ***************************************************
**
** Creates, initialises, and returns a new, empty table that can hold a
** specified number of key-value pairs.
**
** @param [r] size [ajulong] estimate of number of unique keys
**
** @return [AjPTable] new table.
**
** @release 6.4.0
** @@
**
******************************************************************************/

AjPTable ajTablestrNewConst(ajulong size)
{
    AjPTable table = NULL;

    table = ajTableNewFunctionLen(size, &ajTablestrCmp, &ajTablestrHash,
                                  NULL, NULL);
    table->Type = ajETableTypeStr;

    return table;
}




/* @section Destructors *******************************************************
**
** @fdata [AjPTable]
**
** Destructors know they are dealing with strings and can
** clean up keys and values
**
** @nam3rule Free Delete table, keys and values
** @nam4rule FreeKey Delete table, keys and values
**
** @argrule * Ptable [AjPTable*] Hash table
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajTablestrFree *******************************************************
**
** Free keys and value strings in a table and free the table.
** Use only where the keys and strings
** in the table are real, and not just copies of pointers. Otherwise
** a call to ajTableFree is enough.
**
** @param [d] Ptable [AjPTable*] Table
** @return [void]
**
** @release 5.0.0
** @@
******************************************************************************/

void ajTablestrFree(AjPTable* Ptable)
{
    if(!Ptable)
        return;

    if(!*Ptable)
        return;

    ajTableMapDel(*Ptable, &tableStrDel, NULL);

    ajTableFree(Ptable);

    return;
}




/* @func ajTablestrFreeKey ****************************************************
**
** Free string keys in a table and free the table. Do not free the values.
** Use only where the keys
** in the table are real strings, and not just copies of pointers. Otherwise
** a call to ajTableFree is enough. The data is simply freed.
**
** @param [d] Ptable [AjPTable*] Table
** @return [void]
**
** @release 5.0.0
** @@
******************************************************************************/

void ajTablestrFreeKey(AjPTable* Ptable)
{
    if(!*Ptable)
        return;

    ajTableMapDel(*Ptable, &tableStrDelKey, NULL);

    ajTableFree(Ptable);

    return;
}




/* @funcstatic tableStrDel ****************************************************
**
** Delete an entry in a string table.
**
** @param [d] Pkey [void**] AJAX String key address.
** @param [d] Pvalue [void**] AJAX String value address.
** @param [u] cl [void*] Standard argument. Usually NULL.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void tableStrDel(void** Pkey, void** Pvalue, void* cl)
{
    (void) cl;

    ajStrDel((AjPStr*) Pvalue);
    ajStrDel((AjPStr*) Pkey);

    *Pkey   = NULL;
    *Pvalue = NULL;

    return;
}




/* @funcstatic tableStrDelKey *************************************************
**
** Delete an entry in a table with a string key and ignore the value.
**
** @param [d] Pkey [void**] AJAX String key address.
** @param [d] Pvalue [void**] Value address.
** @param [u] cl [void*] Standard argument. Usually NULL.
** @return [void]
**
** @release 5.0.0
** @@
******************************************************************************/

static void tableStrDelKey(void** Pkey, void** Pvalue, void* cl)
{
    (void) cl;

    ajStrDel((AjPStr*) Pkey);

    *Pkey   = NULL;
    *Pvalue = NULL;

    return;
}




/* @section Retrieval *********************************************************
**
** @fdata [AjPTable]
**
** Retrieves values from a hash table
**
** @nam3rule Fetch Retrieval function for string values
** @nam3rule Fetchkey Retrieval function for string keys
** @suffix C [char*] C character string
** @suffix S [AjPStr] string object
**
** @argrule * table [const AjPTable] Hash table
** @argrule *C txtkey [const char*] Key
** @argrule *S key [const AjPStr] Key
**
** @valrule Fetch [void*] Value
** @valrule Fetchkey [const AjPStr] Value
**
** @fcategory cast
**
******************************************************************************/




/* @func ajTablestrFetchC *****************************************************
**
** returns the value associated with key in table, or null
** if table does not hold key.
**
** @param [r] table [const AjPTable] table to search
** @param [r] txtkey [const char*] key to find.
** @return [void*]  value associated with key
** @error NULL if key not found in table.
**
** @release 6.4.0
** @@
******************************************************************************/

void* ajTablestrFetchC(const AjPTable table, const char* txtkey)
{
    ajulong i = 0UL;

    AjPTableNode node = NULL;

    ajStrAssignC(&tableTmpkeyStr, txtkey);

    if(!table)
        return NULL;

    if(!txtkey)
        return NULL;

    i = (*table->Fhash)(tableTmpkeyStr, table->Size);

    for(node = table->Buckets[i]; node; node = node->Link)
        if((*table->Fcmp)(tableTmpkeyStr, node->Key) == 0)
            break;

    return node ? node->Value : NULL;
}




/* @func ajTablestrFetchS *****************************************************
**
** returns the value associated with key in table, or null
** if table does not hold key.
**
** @param [r] table [const AjPTable] table to search
** @param [r] key   [const AjPStr] key to find.
** @return [void*]  value associated with key
** @error NULL if key not found in table.
**
** @release 6.4.0
** @@
******************************************************************************/

void* ajTablestrFetchS(const AjPTable table, const AjPStr key)
{
    ajulong i = 0UL;

    AjPTableNode node = NULL;

    if(!table)
        return NULL;

    if(!key)
        return NULL;

    i = (*table->Fhash)(key, table->Size);

    for(node = table->Buckets[i]; node; node = node->Link)
        if((*table->Fcmp)(key, node->Key) == 0)
            break;

    return node ? node->Value : NULL;
}




/* @func ajTablestrFetchkeyC **************************************************
**
** returns the key value associated with key in table, or null
** if table does not hold key.
**
** Intended for case-insensitive keys, to return the true key
**
** @param [r] table [const AjPTable] table to search
** @param [r] txtkey   [const char*] key to find.
** @return [const AjPStr] key value as stored in the table
** @error NULL if key not found in table.
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPStr ajTablestrFetchkeyC(const AjPTable table, const char* txtkey)
{
    ajulong i = 0UL;

    const AjPTableNode node = NULL;

    if(!table)
        return NULL;

    if(!txtkey)
        return NULL;

    if(table->Type == ajETableTypeStr)
    {
        ajStrAssignC(&tableTmpkeyStr, txtkey);
        return ajTablestrFetchkeyS(table, tableTmpkeyStr);
    }

    if(table->Type != ajETableTypeChar)
        ajFatal("ajTablestrFetchkeyC called for %s table", tableType(table));

    i = (*table->Fhash)(txtkey, table->Size);

    for(node = table->Buckets[i]; node; node = node->Link)
        if((*table->Fcmp)(txtkey, node->Key) == 0)
            break;

    return node ? (const void*) node->Key : NULL;
}




/* @func ajTablestrFetchkeyS **************************************************
**
** returns the key value associated with key in table, or null
** if table does not hold key.
**
** Intended for case-insensitive keys, to return the true key
**
** @param [r] table [const AjPTable] table to search
** @param [r] key   [const AjPStr] key to find.
** @return [const AjPStr] key value as stored in the table
** @error NULL if key not found in table.
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPStr ajTablestrFetchkeyS(const AjPTable table, const AjPStr key)
{
    ajulong i = 0UL;

    const AjPTableNode node = NULL;

    if(!table)
        return NULL;

    if(!key)
        return NULL;

    if(table->Type == ajETableTypeChar)
    {
        return ajTablestrFetchkeyC(table, MAJSTRGETPTR(key));
    }

    if(table->Type != ajETableTypeStr)
        ajFatal("ajTablestrFetchkeyS called for %s table", tableType(table));

    i = (*table->Fhash)(key, table->Size);

    for(node = table->Buckets[i]; node; node = node->Link)
        if((*table->Fcmp)(key, node->Key) == 0)
            break;

    return node ? (const void*) node->Key : NULL;
}




/* @funcstatic tableType ******************************************************
**
** returns the name of a table type.
**
** @param [r] table [const AjPTable] table type
** @return [const char*] Name of table type
**
** @release 6.4.0
** @@
******************************************************************************/

static const char* tableType(const AjPTable table)
{
    const char* typenames[] = {"unknown", "char", "string",
                               "int", "uint", "long", "ulong", "user", NULL};
    ajint i = table->Type;

    if(i < ajETableTypeUnknown || i >= ajETableTypeMax)
        return "unknown";

    return typenames[i];
}




/* @section Modify ************************************************************
**
** @fdata [AjPTable]
**
** Updates values from a hash table
**
** @nam3rule Fetchmod Retrieval function
**
** @argrule * table [AjPTable] Hash table
** @argrule Fetchmod key [const AjPStr] Key
**
** @valrule Fetchmod [AjPStr*] Value
**
** @fcategory modify
**
******************************************************************************/




/* @func ajTablestrFetchmod ***************************************************
**
** returns the value associated with key in table, or null
** if table does not hold key.
**
** @param [u] table [AjPTable] table to search
** @param [r] key   [const AjPStr] key to find.
** @return [AjPStr*]  value associated with key
** @error NULL if key not found in table.
**
** @release 6.0.0
** @@
******************************************************************************/

AjPStr* ajTablestrFetchmod(AjPTable table, const AjPStr key)
{
    ajulong i = 0UL;

    AjPTableNode node = NULL;

    if(!table)
        return NULL;

    if(!key)
        return NULL;

    i = (*table->Fhash)(key, table->Size);

    for(node = table->Buckets[i]; node; node = node->Link)
        if((*table->Fcmp)(key, node->Key) == 0)
            break;

    return node ? (AjPStr*) (&node->Value) : NULL;
}




/* @section Comparison functions **********************************************
**
** @fdata [AjPTable]
**
** Comparison functions for table keys
**
** @nam3rule Cmp Comparison
** @nam3rule Hash Hashing keys
**
** @argrule Cmp key1 [const void*] First key
** @argrule Cmp key2 [const void*] Second key
** @argrule Hash key [const void*] Key
** @argrule Hash hashsize [ajulong] Hash table size
** @suffix Case Case insensitive keys
**
** @valrule Cmp [ajint] Comparison result 0 for a match,
**                      -1 or +1 for a mismatch
** @valrule Hash [ajulong] hash value
**
** @fcategory misc
**
******************************************************************************/




/* @func ajTablestrCmp ********************************************************
**
** Comparison function for a table with a string key
**
** @param [r] key1 [const void*] First key.
** @param [r] key2 [const void*] Second key.
** @return [ajint] Comparison result. Zero if equal, non-zero if different.
**
** @release 5.0.0
** @@
******************************************************************************/

ajint ajTablestrCmp(const void* key1, const void* key2)
{
    return (ajint) ajStrCmpS((const AjPStr) key1, (const AjPStr) key2);
}




/* @func ajTablestrCmpCase ****************************************************
**
** Comparison function for a table with a string key
** and case insensitivity.
**
** @param [r] key1 [const void*] First key.
** @param [r] key2 [const void*] Second key.
** @return [ajint] Comparison result. Zero if equal, non-zero if different.
**
** @release 5.0.0
** @@
******************************************************************************/

ajint ajTablestrCmpCase(const void* key1, const void* key2)
{
    return (ajint) ajStrCmpCaseS((const AjPStr) key1, (const AjPStr) key2);
}




/* @func ajTablestrHash *******************************************************
**
** Hash function for a table with a string key
**
** @param [r] key [const void*] Standard argument. Table key.
** @param [r] hashsize [ajulong] Standard argument. Estimated Hash size.
** @return [ajulong] Hash value in range 0 to hashsize-1
**
** @release 5.0.0
** @@
******************************************************************************/

ajulong ajTablestrHash(const void* key, ajulong hashsize)
{
    ajulong hash = 0UL;

    const char* s = NULL;

    const AjPStr str = NULL;

    str = (const AjPStr) key;
    s   = ajStrGetPtr(str);

    for(hash = 0; *s; s++)
        hash = (hash * 127 + *s) % hashsize;

    return hash;
}




/* @func ajTablestrHashCase ***************************************************
**
** Hash function for a table with a string key and
** case insensitivity.
**
** @param [r] key [const void*] Standard argument. Table key.
** @param [r] hashsize [ajulong] Standard argument. Estimated Hash size.
** @return [ajulong] Hash value in range 0 to hashsize-1
**
** @release 5.0.0
** @@
******************************************************************************/

ajulong ajTablestrHashCase(const void* key, ajulong hashsize)
{
    ajulong hash = 0UL;

    const char* s = NULL;

    const AjPStr str = NULL;

    str = (const AjPStr) key;
    s   = ajStrGetPtr(str);

    for(hash = 0; *s; s++)
        hash = (hash * 127 + toupper((ajint)*s)) % hashsize;

    return hash;
}




/* @section Trace functions ***************************************************
**
** @fdata [AjPTable]
**
** @nam3rule Print Trace contents to standard error
** @nam3rule Trace Trace contents to debug file
** @nam3rule Tracekeys Trace keys only to debug file
**
** @argrule * table [const AjPTable] Hash table
**
** @valrule * [void]
**
** @fcategory misc
**
******************************************************************************/




/* @section Trace functions ***************************************************
**
** @fdata [AjPTable]
**
** @nam3rule Print Trace contents to standard error
** @nam3rule Trace Trace contents to debug file
** @nam3rule Tracekeys Trace keys only to debug file
**
** @argrule * table [const AjPTable] Hash table
**
** @valrule * [void]
**
** @fcategory misc
**
******************************************************************************/




/* @func ajTablestrPrint ******************************************************
**
** Print function for a table with a string key.
**
** @param [r] table [const AjPTable] Table.
** @return [void]
**
** @release 5.0.0
** @@
******************************************************************************/

void ajTablestrPrint(const AjPTable table)
{
    ajulong i = 0UL;

    AjPTableNode node = NULL;

    if(!table)
        return;

    for(i = 0; i < table->Size; i++)
        for(node = table->Buckets[i]; node; node = node->Link)
            ajUser("key '%S' value '%S'",
                   (const AjPStr) node->Key, (AjPStr) node->Value);

    return;
}




/* @func ajTablestrTrace ******************************************************
**
** Writes debug messages to trace the contents of a table,
** assuming all keys and values are strings.
**
** @param [r] table [const AjPTable] Table
** @return [void]
**
** @release 5.0.0
** @@
******************************************************************************/

void ajTablestrTrace(const AjPTable table)
{
    ajulong i = 0UL;
    ajulong j = 0UL;
    ajulong k = 0UL;

    AjPTableNode node = NULL;

    if(!table)
        return;

    ajDebug("(string) table trace: ");
    ajDebug(" length: %Lu", table->Length);
    ajDebug(" size: %Lu", table->Size);
    ajDebug(" timestamp: %u\n", table->Timestamp);

    for(i = 0UL; i < table->Size; i++)
        if(table->Buckets[i])
        {
            j = 0UL;
            ajDebug("buckets[%Lu]\n", i);

            for(node = table->Buckets[i]; node; node = node->Link)
            {
                ajDebug("   '%S' => '%S'\n",
                        (const AjPStr) node->Key, (AjPStr) node->Value);
                j++;
            }

            k += j;
        }

    ajDebug(" links: %Lu\n", k);

    return;
}




/* @func ajTablestrTracekeys **************************************************
**
** Writes debug messages to trace the keys of a string table,
** assuming all keys and values are strings.
**
** @param [r] table [const AjPTable] Table
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTablestrTracekeys(const AjPTable table)
{
    ajulong i = 0UL;
    ajulong j = 0UL;
    ajulong k = 0UL;

    AjPTableNode node = NULL;

    if(!table)
        return;

    ajDebug("(string) table trace: ");
    ajDebug(" length: %Lu", table->Length);
    ajDebug(" size: %Lu", table->Size);
    ajDebug(" timestamp: %u\n", table->Timestamp);

    for(i = 0UL; i < table->Size; i++)
        if(table->Buckets[i])
        {
            j = 0UL;
            ajDebug("buckets[%Lu]\n", i);

            for(node = table->Buckets[i]; node; node = node->Link)
            {
                ajDebug("   '%S' => '%xS\n",
                        (const AjPStr) node->Key, node->Value);
                j++;
            }

            k += j;
        }

    ajDebug(" links: %Lu\n", k);

    return;
}




/* @datasection [AjPTable] Unsigned integer hash tables ***********************
**
** @nam2rule Tableuint Unsigned integer hash tables
**
******************************************************************************/




/* @section Constructors ******************************************************
**
** Constructors for hash tables
**
** @fdata [AjPTable]
**
** @fcategory new
**
** @nam3rule New Constructor
** @suffix Const Constant keys with no destructor
**
** @argrule New size [ajulong] Number of key values
**
** @valrule * [AjPTable] New hash table
**
******************************************************************************/




/* @func ajTableuintNew *******************************************************
**
** Creates, initialises, and returns a new, empty table that can hold a
** specified number of unsigned integer key-value pairs.
**
** @param [r] size [ajulong] estimate of number of unique keys
**
** @return [AjPTable] new table.
**
** @release 6.4.0
** @@
**
******************************************************************************/

AjPTable ajTableuintNew(ajulong size)
{
    AjPTable table = NULL;

    table = ajTableNewFunctionLen(size, &ajTableuintCmp, &ajTableuintHash,
                                  &ajMemFree, NULL);
    table->Type = ajETableTypeUint;

    return table;
}




/* @func ajTableuintNewConst **************************************************
**
** Creates, initialises, and returns a new, empty table that can hold a
** specified number of unsigned integer key-value pairs.
**
** @param [r] size [ajulong] estimate of number of unique keys
**
** @return [AjPTable] new table.
**
** @release 6.4.0
** @@
**
******************************************************************************/

AjPTable ajTableuintNewConst(ajulong size)
{
    AjPTable table = NULL;

    table = ajTableNewFunctionLen(size, &ajTableuintCmp, &ajTableuintHash,
                                  NULL, NULL);
    table->Type = ajETableTypeUint;

    return table;
}




/* @section Destructors *******************************************************
**
** @fdata [AjPTable]
**
** Destructors know they are dealing with strings and can clean up
** keys and values
**
** @nam3rule Free Delete table, keys and values
** @nam4rule FreeKey Delete table, keys and values
**
** @argrule * Ptable [AjPTable*] Hash table
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajTableuintFree ******************************************************
**
** Free keys and value strings in a table and free the table.
** Use only where the keys and data
** in the table are real, and not just copies of pointers. Otherwise
** a call to ajTableFree is enough.
**
** @param [d] Ptable [AjPTable*] Table
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTableuintFree(AjPTable* Ptable)
{
    if(!Ptable)
        return;

    if(!*Ptable)
        return;

    ajTableMapDel(*Ptable, &tableDel, NULL);

    ajTableFree(Ptable);

    return;
}




/* @func ajTableuintFreeKey ***************************************************
**
** Free keys and value strings in a table and free the table.
** Use only where the keys
** in the table are real, and not just copies of pointers. Otherwise
** a call to ajTableFree is enough.
**
** @param [d] Ptable [AjPTable*] Table
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTableuintFreeKey(AjPTable* Ptable)
{
    if(!Ptable)
        return;

    if(!*Ptable)
        return;

    ajTableMapDel(*Ptable, &tableDelKey, NULL);

    ajTableFree(Ptable);

    return;
}




/* @section Retrieval *********************************************************
**
** @fdata [AjPTable]
**
** Retrieves values from a hash table
**
** @nam3rule Fetch Retrieval function
**
** @argrule * table [const AjPTable] Hash table
** @argrule Fetch uintkey [const ajuint*] Key
**
** @valrule Fetch [const ajuint*] Value
** @fcategory cast
**
******************************************************************************/




/* @func ajTableuintFetch *****************************************************
**
** returns the value associated with key in table, or null
** if table does not hold key.
**
** @param [r] table [const AjPTable] table to search
** @param [r] uintkey [const ajuint*] key to find.
** @return [const ajuint*]  value associated with key
** @error NULL if key not found in table.
**
** @release 6.4.0
** @@
******************************************************************************/

const ajuint* ajTableuintFetch(const AjPTable table, const ajuint* uintkey)
{
    ajulong i = 0UL;

    AjPTableNode node = NULL;

    if(!table)
        return NULL;

    if(!uintkey)
        return NULL;

    i = (*table->Fhash)(uintkey, table->Size);

    for(node = table->Buckets[i]; node; node = node->Link)
        if((*table->Fcmp)(uintkey, node->Key) == 0)
            break;

    return node ? (const ajuint*) node->Value : NULL;
}




/* @section Modify ************************************************************
**
** @fdata [AjPTable]
**
** Updates values from a hash table
**
** @nam3rule Fetchmod Retrieval function
**
** @argrule * table [AjPTable] Hash table
** @argrule Fetchmod uintkey [const ajuint*] Key
**
** @valrule Fetchmod [ajuint*] Value
**
** @fcategory modify
**
******************************************************************************/




/* @func ajTableuintFetchmod **************************************************
**
** returns the value associated with key in table, or null
** if table does not hold key.
**
** @param [u] table [AjPTable] table to search
** @param [r] uintkey   [const ajuint*] key to find.
** @return [ajuint*]  value associated with key
** @error NULL if key not found in table.
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint* ajTableuintFetchmod(AjPTable table, const ajuint* uintkey)
{
    ajulong i = 0UL;

    AjPTableNode node = NULL;

    if(!table)
        return NULL;

    if(!uintkey)
        return NULL;

    i = (*table->Fhash)(uintkey, table->Size);

    for(node = table->Buckets[i]; node; node = node->Link)
        if((*table->Fcmp)(uintkey, node->Key) == 0)
            break;

    return node ? (ajuint*) (&node->Value) : NULL;
}




/* @section Comparison functions **********************************************
**
** @fdata [AjPTable]
**
** Comparison functions for table keys
**
** @nam3rule Cmp Comparison
** @nam3rule Hash Hashing keys
**
** @argrule Cmp key1 [const void*] First key
** @argrule Cmp key2 [const void*] Second key
** @argrule Hash key [const void*] Key
** @argrule Hash hashsize [ajulong] Hash table size
** @suffix Case Case insensitive keys
**
** @valrule Cmp [ajint] Comparison result 0 for a match,
**                      -1 or +1 for a mismatch
** @valrule Hash [ajulong] hash value
**
** @fcategory misc
**
******************************************************************************/




/* @func ajTableuintCmp *******************************************************
**
** Comparison function for a table with an unsigned integer key
**
** @param [r] key1 [const void*] First key.
** @param [r] key2 [const void*] Second key.
** @return [ajint] Comparison result. Zero if equal, non-zero if different.
**
** @release 6.4.0
** @@
******************************************************************************/

ajint ajTableuintCmp(const void* key1, const void* key2)
{
    return (ajint) (*((const ajuint*) key1) != *((const ajuint*) key2));
}




/* @func ajTableuintHash ******************************************************
**
** Hash function for a table with an unsigned integer key
**
** @param [r] key [const void*] Standard argument. Table key.
** @param [r] hashsize [ajulong] Standard argument. Estimated Hash size.
** @return [ajulong] Hash value in range 0 to hashsize-1
**
** @release 6.4.0
** @@
******************************************************************************/

ajulong ajTableuintHash(const void* key, ajulong hashsize)
{
    ajulong hash = 0UL;

    const ajuint* ia = NULL;

    if(!key)
        return 0;

    if(!hashsize)
        return 0;

    ia = (const ajuint *) key;

    hash = (*ia >> 2) % hashsize;

    return hash;
}




/* @datasection [AjPTable] Unsigned long hash tables **************************
**
** @nam2rule Tableulong Unsigned long integer hash tables
**
******************************************************************************/




/* @section Constructors ******************************************************
**
** Constructors for hash tables
**
** @fdata [AjPTable]
**
** @fcategory new
**
** @nam3rule New Constructor
** @suffix Const Constant keys with no destructor
**
** @argrule New size [ajulong] Number of key values
**
** @valrule * [AjPTable] New hash table
**
******************************************************************************/




/* @func ajTableulongNew ******************************************************
**
** Creates, initialises, and returns a new, empty table that can hold an
** arbitrary number of unsigned long integer key-value pairs
**
** @param [r] size [ajulong] estimate of number of unique keys
** @return [AjPTable] new table.
**
** @release 6.4.0
** @@
**
******************************************************************************/

AjPTable ajTableulongNew(ajulong size)
{
    AjPTable table = NULL;

    table = ajTableNewFunctionLen(size, &ajTableulongCmp, &ajTableulongHash,
                                  &ajMemFree, NULL);
    table->Type = ajETableTypeUlong;

    return table;
}





/* @func ajTableulongNewConst *************************************************
**
** Creates, initialises, and returns a new, empty table that can hold an
** arbitrary number of unsigned long integer key-value pairs
**
** @param [r] size [ajulong] estimate of number of unique keys
** @return [AjPTable] new table.
**
** @release 6.4.0
** @@
**
******************************************************************************/

AjPTable ajTableulongNewConst(ajulong size)
{
    AjPTable table = NULL;

    table = ajTableNewFunctionLen(size, &ajTableulongCmp, &ajTableulongHash,
                                  NULL, NULL);
    table->Type = ajETableTypeUlong;

    return table;
}





/* @section Destructors *******************************************************
**
** @fdata [AjPTable]
**
** Destructors know they are dealing with strings and can
** clean up keys and values
**
** @nam3rule Free Delete table, keys and values
** @nam4rule FreeKey Delete table, keys and values
**
** @argrule * Ptable [AjPTable*] Hash table
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajTableulongFree *****************************************************
**
** Free keys and value strings in a table and free the table.
** Use only where the keys and data
** in the table are real, and not just copies of pointers. Otherwise
** a call to ajTableFree is enough.
**
** @param [d] Ptable [AjPTable*] Table
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTableulongFree(AjPTable* Ptable)
{
    if(!Ptable)
        return;

    if(!*Ptable)
        return;

    ajTableMapDel(*Ptable, &tableDel, NULL);

    ajTableFree(Ptable);

    return;
}




/* @func ajTableulongFreeKey **************************************************
**
** Free keys and value strings in a table and free the table.
** Use only where the keys
** in the table are real, and not just copies of pointers. Otherwise
** a call to ajTableFree is enough.
**
** @param [d] Ptable [AjPTable*] Table
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTableulongFreeKey(AjPTable* Ptable)
{
    if(!Ptable)
        return;

    if(!*Ptable)
        return;

    ajTableMapDel(*Ptable, &tableDelKey, NULL);

    ajTableFree(Ptable);

    return;
}




/* @section Retrieval *********************************************************
**
** @fdata [AjPTable]
**
** Retrieves values from a hash table
**
** @nam3rule Fetch Retrieval function
**
** @argrule * table [const AjPTable] Hash table
** @argrule Fetch ulongkey [const ajulong*] Key
**
** @valrule Fetch [const ajulong*] Value
**
** @fcategory cast
**
******************************************************************************/




/* @func ajTableulongFetch ****************************************************
**
** returns the value associated with key in table, or null
** if table does not hold key.
**
** @param [r] table [const AjPTable] table to search
** @param [r] ulongkey [const ajulong*] key to find.
** @return [const ajulong*] value associated with key
** @error NULL if key not found in table.
**
** @release 6.4.0
** @@
******************************************************************************/

const ajulong* ajTableulongFetch(const AjPTable table, const ajulong* ulongkey)
{
    ajulong i = 0UL;

    AjPTableNode node = NULL;

    if(!table)
        return NULL;

    if(!ulongkey)
        return NULL;

    i = (*table->Fhash)(ulongkey, table->Size);

    for(node = table->Buckets[i]; node; node = node->Link)
        if((*table->Fcmp)(ulongkey, node->Key) == 0)
            break;

    return node ? (const ajulong*) node->Value : NULL;
}




/* @section Modify ************************************************************
**
** @fdata [AjPTable]
**
** Updates values from a hash table
**
** @nam3rule Fetchmod Retrieval function
**
** @argrule * table [AjPTable] Hash table
** @argrule Fetchmod ulongkey [const ajulong*] Key
**
** @valrule Fetchmod [ajulong*] Value
**
** @fcategory modify
**
******************************************************************************/




/* @func ajTableulongFetchmod *************************************************
**
** Returns the value associated with key in table, or null if table
** does not hold key.
**
** @param [u] table [AjPTable] table to search
** @param [r] ulongkey   [const ajulong*] key to find.
** @return [ajulong*]  value associated with key
** @error NULL if key not found in table.
**
** @release 6.4.0
** @@
******************************************************************************/

ajulong* ajTableulongFetchmod(AjPTable table, const ajulong* ulongkey)
{
    ajulong i = 0UL;

    AjPTableNode node = NULL;

    if(!table)
        return NULL;

    if(!ulongkey)
        return NULL;

    i = (*table->Fhash)(ulongkey, table->Size);

    for(node = table->Buckets[i]; node; node = node->Link)
        if((*table->Fcmp)(ulongkey, node->Key) == 0)
            break;

    return node ? (ajulong*) (&node->Value) : NULL;
}




/* @section Comparison functions **********************************************
**
** @fdata [AjPTable]
**
** Comparison functions for table keys
**
** @nam3rule Cmp Comparison
** @nam3rule Hash Hashing keys
**
** @argrule Cmp key1 [const void*] First key
** @argrule Cmp key2 [const void*] Second key
** @argrule Hash key [const void*] Key
** @argrule Hash hashsize [ajulong] Hash table size
** @suffix Case Case insensitive keys
**
** @valrule Cmp [ajint] Comparison result 0 for a match,
**                      -1 or +1 for a mismatch
** @valrule Hash [ajulong] hash value
**
** @fcategory misc
**
******************************************************************************/




/* @func ajTableulongCmp ******************************************************
**
** Comparison function for a table with an unsigned long integer key
**
** @param [r] key1 [const void*] First key.
** @param [r] key2 [const void*] Second key.
** @return [ajint] Comparison result. Zero if equal, non-zero if different.
**
** @release 6.4.0
** @@
******************************************************************************/

ajint ajTableulongCmp(const void* key1, const void* key2)
{
    return (ajint) (*((const ajulong*) key1) != *((const ajulong*) key2));
}




/* @func ajTableulongHash *****************************************************
**
** Hash function for a table with an unsigned long integer key
**
** @param [r] key [const void*] Standard argument. Table key.
** @param [r] hashsize [ajulong] Standard argument. Estimated Hash size.
** @return [ajulong] Hash value in range 0 to hashsize-1
**
** @release 6.4.0
** @@
******************************************************************************/

ajulong ajTableulongHash(const void* key, ajulong hashsize)
{
    ajulong hash = 0UL;

    const ajulong* ia = NULL;

    if(!key)
        return 0;

    if(!hashsize)
        return 0;

    ia = (const ajulong*) key;

    hash = (*ia >> 2) % hashsize;

    return hash;
}




/* @funcstatic tableDelStr ****************************************************
**
** Delete a string object value
**
** @param [d] Pstr [void**] AJAX String object
** @return [void]
**
** @release 6.4.0
******************************************************************************/

static void tableDelStr(void** Pstr)
{
    if(!Pstr)
        return;
    if(!*Pstr)
        return;

    ajStrDel((AjPStr*) Pstr);

    return;
}




#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif




#ifdef AJ_COMPILE_DEPRECATED
/* @obsolete ajTableNewLen
** @rename ajTableNew
*/
__deprecated AjPTable ajTableNewLen(ajuint size)
{
    AjPTable table = NULL;

    table = ajTableNewFunctionLen(size, &tableCmpAtom, &tableHashAtom,
                                  NULL, NULL);
    table->Type = ajETableTypeUser;

    return table;
}




/* @obsolete ajTableNewL
** @rename ajTableNewFunctionLen
*/

__deprecated AjPTable ajTableNewL(
    ajuint size,
    ajint (*cmp)(const void* key1, const void* key2),
    ajuint (*hash)(const void* key,
                   ajuint hashsize))
{
    /*
    ** Since the interface of ajTableNewFunctionLen has changed after
    ** deprecation of ajTableNewL, the hash function pointer needs
    ** additional casting.
    */
    return ajTableNewFunctionLen(size, cmp,
                                 (ajulong (*)(const void*, ajulong)) hash,
                                 NULL, NULL);
}




/* @obsolete ajTableFetch
** @rename ajTableFetchmodV
*/

__deprecated void* ajTableFetch(const AjPTable table, const void* key)
{
    ajuint i = 0U;

    AjPTableNode node = NULL;

    if(!table)
        return NULL;

    if (!key)
        return NULL;

    i = (*table->Fhash)(key, table->Size);

    for(node = table->Buckets[i]; node; node = node->Link)
        if((*table->Fcmp)(key, node->Key) == 0)
            break;

    return node ? node->Value : NULL;
}




/* @obsolete ajTablestrFetch
** @rename ajTablestrFetchS
*/
__deprecated const AjPStr ajTablestrFetch(const AjPTable table,
                                          const AjPStr key)
{
    return ajTablestrFetchS(table, key);
}




/* @obsolete ajTableGet
** @rename ajTableFetchmodV
*/

__deprecated void* ajTableGet(const AjPTable table, const void* key)
{
    return ajTableFetchmodV(table, key);
}




/* @obsolete ajTableFetchKey
** @rename ajTablestrFetchKeyS
*/

__deprecated const void* ajTableFetchKey(const AjPTable table,
                                         const void* key)
{
    ajuint i = 0U;

    const AjPTableNode node = NULL;

    if (!table)
        return NULL;

    if (!key)
        return NULL;

    i = (*table->Fhash)(key, table->Size);

    for(node = table->Buckets[i]; node; node = node->Link)
        if((*table->Fcmp)(key, node->Key) == 0)
            break;

    return node ? (const void*) node->Key : NULL;
}




/* @obsolete ajTableLength
** @rename ajTableGetLength
*/

__deprecated ajint ajTableLength(const AjPTable table)
{
    return (ajuint) ajTableGetLength(table);
}




/* @obsolete ajTableToarray
** @rename ajTableToarrayKeysValues
*/

__deprecated ajuint ajTableToarray(const AjPTable table,
                                   void*** keyarray, void*** valarray)
{

    return ajTableToarrayKeysValues(table, keyarray, valarray);
}




/* @obsolete ajTablecharNewCaseLen
** @rename ajTablecharNewCase
*/

__deprecated AjPTable ajTablecharNewCaseLen(ajuint hint)
{
    return ajTablecharNewCase(hint);
}




/* @obsolete ajStrTableNewCase
** @rename ajTablecharNewCase
*/

__deprecated AjPTable ajStrTableNewCase(ajuint hint)
{
    return ajTablecharNewCase(hint);
}




/* @obsolete ajStrTableNewCaseC
** @rename ajTablecharNewCase
*/

__deprecated AjPTable ajStrTableNewCaseC(ajuint hint)
{
    return ajTablecharNewCase(hint);
}




/* @obsolete ajStrTableNewC
** @rename ajTablecharNew
*/

__deprecated AjPTable ajStrTableNewC(ajuint hint)
{
    return ajTablecharNew(hint);
}




/* @obsolete ajStrTableNew
** @rename ajTablestrNew
*/

__deprecated AjPTable ajStrTableNew(ajuint hint)
{
    return ajTablestrNew(hint);
}




/* @obsolete ajStrTablePrintC
** @rename ajTablecharPrint
*/

__deprecated void ajStrTablePrintC(const AjPTable table)
{
    ajTablecharPrint(table);

    return;
}




/* @obsolete ajStrTableCmpC
** @rename ajTablecharCmp
*/

__deprecated ajint ajStrTableCmpC(const void* key1, const void* key2)
{
    return ajTablecharCmp(key1, key2);
}




/* @obsolete ajStrTableCmpCaseC
** @rename ajTablecharCmpCase
*/

__deprecated ajint ajStrTableCmpCaseC(const void* key1, const void* key2)
{
    return ajTablecharCmpCase(key1, key2);
}




/* @obsolete ajStrTableHashC
** @rename ajTablecharHash
*/

__deprecated ajuint ajStrTableHashC(const void* key, ajuint hashsize)
{
    return ajTablecharHash(key, hashsize);
}




/* @obsolete ajStrTableHashCaseC
** @rename ajTablecharHashCase
*/

__deprecated ajuint ajStrTableHashCaseC(const void* key, ajuint hashsize)
{
    return ajTablecharHashCase(key, hashsize);
}




/* @obsolete ajTablestrNewLen
** @rename ajTablestrNew
*/

__deprecated AjPTable ajTablestrNewLen(ajuint size)
{
    AjPTable table = NULL;

    table = ajTableNewFunctionLen(size, &ajTablestrCmp, &ajTablestrHash,
                                  NULL, NULL);
    table->Type = ajETableTypeStr;

    return table;
}




/* @obsolete ajTablestrNewCaseLen
** @rename ajTablestrNewCase
*/
__deprecated AjPTable ajTablestrNewCaseLen(ajuint size)
{
    AjPTable table = NULL;

    table = ajTableNewFunctionLen(size, &ajTablestrCmpCase, &ajTablestrHashCase,
                                  NULL, NULL);
    table->Type = ajETableTypeStr;

    return table;
}




/* @obsolete ajStrTableFree
** @rename ajTablestrFree
*/

__deprecated void ajStrTableFree(AjPTable* ptable)
{
    ajTablestrFree(ptable);

    return;
}




/* @obsolete ajStrTableFreeKey
** @rename ajTablestrFreeKey
*/

__deprecated void ajStrTableFreeKey(AjPTable* ptable)
{
    ajTablestrFreeKey(ptable);

    return;
}




/* @obsolete ajTableKey
** @rename ajTablestrFetchKeyS
*/

__deprecated const void* ajTableKey(const AjPTable table, const void* key)
{
    return ajTablestrFetchkeyS(table, key);
}




/* @obsolete ajStrTableCmp
** @rename ajTablestrCmp
*/

__deprecated ajint ajStrTableCmp(const void* key1, const void* key2)
{
    return ajTablestrCmp(key1, key2);
}




/* @obsolete ajStrTableCmpCase
** @rename ajTablestrCmpCase
*/

__deprecated ajint ajStrTableCmpCase(const void* key1, const void* key2)
{
    return ajTablestrCmpCase(key1, key2);
}




/* @obsolete ajStrTableHash
** @rename ajTablestrHash
*/

__deprecated ajuint ajStrTableHash(const void* key, ajuint hashsize)
{
    return ajTablestrHash(key, hashsize);
}




/* @obsolete ajStrTableHashCase
** @rename ajTablestrHashCase
*/

__deprecated ajuint ajStrTableHashCase(const void* key, ajuint hashsize)
{
    return ajTablestrHashCase(key, hashsize);
}




/* @obsolete ajStrTablePrint
** @rename ajTablestrPrint
*/

__deprecated void ajStrTablePrint(const AjPTable table)
{
    ajTablestrPrint(table);
}




/* @obsolete ajStrTableTrace
** @rename ajTablestrTrace
*/

__deprecated void ajStrTableTrace(const AjPTable table)
{
    ajTablestrTrace(table);

    return;
}
#endif

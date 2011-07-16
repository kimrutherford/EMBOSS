/******************************************************************************
** @source AJAX table functions
**
** Hash table functions.
**
** @author Copyright (C) 1998 Ian Longden
** @version 1.0
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

#include <limits.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#include "ajax.h"


enum TableEType
{
    TABLE_UNKNOWN,			/* no type set */
    TABLE_CHAR,				/* char* type */
    TABLE_STR,				/* AjPStr type */
    TABLE_INT,				/* int type */
    TABLE_UINT,                         /* unsigned int type */
    TABLE_LONG,				/* long int type */
    TABLE_ULONG,                        /* unsigned long int type */
    TABLE_USER,				/* user-defined type */
    TABLE_MAX
};



#ifdef AJ_SAVESTATS
static ajuint tableNewCnt = 0;
static ajint tableDelCnt = 0;
static ajuint tableMaxNum = 0;
static size_t tableMaxMem = 0;
#endif

static AjPStr tableTmpkeyStr = NULL;

static ajint tableFreeNext = 0;
static ajint tableFreeMax = 0;
static struct binding ** tableFreeSet = NULL;

static ajint  tableCmpAtom(const void *x, const void *y);
static void   tableDel(void** key, void** value, void* cl);
static void   tableDelKey(void** key, void** value, void* cl);
static void   tableFreeSetExpand (void);
static ajuint tableHashAtom(const void *key, ajuint hashsize);
static void   tableStrDel(void** key, void** value, void* cl);
static void   tableStrDelKey(void** key, void** value, void* cl);


static void   tableDelStr(void** Pdata);
static const char* tableType(const AjPTable table);
static void tableStrFromKey(const AjPTable table, const void *key,
                            AjPStr* Pstr);




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
** @argrule New size [ajuint] Number of key values
** @argrule Function cmp [ajint function] Comparison function returning
**                                        +1, zero or -1
** @argrule Function hash [ajuint function] Hash function for keys
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
** @param [r] size [ajuint] number of key-value pairs
**
** @return [AjPTable] new table.
**
** @@
**
******************************************************************************/

AjPTable ajTableNew(ajuint size)
{
    AjPTable ret;

    ret = ajTableNewFunctionLen(size, tableCmpAtom, tableHashAtom,
                                NULL, NULL);
    ret->Type = TABLE_USER;

    return ret;
}




/* @obsolete ajTableNewLen
** @rename ajTableNew
*/
__deprecated AjPTable ajTableNewLen(ajuint size)
{
    AjPTable ret;

    ret = ajTableNewFunctionLen(size, tableCmpAtom, tableHashAtom,
                                NULL, NULL);
    ret->Type = TABLE_USER;

    return ret;
}




/* @func ajTableNewFunctionLen ************************************************
**
** creates, initialises, and returns a new, empty table that expects
** a specified number of key-value pairs.
**
** Current method defines the table size as the number of entries divided by 4
** to avoid a huge table.
**
** @param [r] size [ajuint] number of key-value pairs
** @param [fN] cmp  [ajint function] function for comparing
** @param [fN] hash [ajuint function] function for hashing keys
** @param [fN] keydel [void function] key destructor function
** @param [fN] valdel [void function] value destructor function
**
** @return [AjPTable] new table.
**
** @@
**
******************************************************************************/

AjPTable ajTableNewFunctionLen(ajuint size,
			       ajint cmp(const void *x, const void *y),
			       ajuint hash(const void *key, ajuint hashsize),
                               void keydel(void** key),
                               void valdel(void** value))
{
    ajuint hint;
    ajuint i;
    ajuint iprime;
    AjPTable table;

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

    for(i = 1; primes[i] <= hint; i++); /* else use default primes[0] */

    iprime = primes[i-1];

    /*ajDebug("ajTableNewFunctionLen hint %d size %d\n", hint, iprime);*/

    AJNEW0(table);
    table->size = iprime;
    table->cmp  = cmp;
    table->hash = hash;
    table->defdel = keydel;
    table->keydel = keydel;
    table->valdel = valdel;
    AJCNEW(table->buckets, iprime);

    for(i = 0; i < table->size; i++)
	table->buckets[i] = NULL;

    table->length = 0;
    table->timestamp = 0;

#ifdef AJ_SAVESTATS
    tableNewCnt++;

    if(iprime > tableMaxNum)
	tableMaxNum = iprime;

    if(sizeof(*table) > tableMaxMem)
	tableMaxMem = sizeof(*table);
#endif

    table->Use = 1;
    table->Type = TABLE_USER;

    return table;
}




/* @obsolete ajTableNewL
** @rename ajTableNewFunctionLen
*/

__deprecated AjPTable ajTableNewL(ajuint size,
				  ajint cmp(const void *x, const void *y),
				  ajuint hash(const void *key, ajuint hashsize))
{

    return ajTableNewFunctionLen(size, cmp, hash, NULL, NULL);
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
    {					/* any other references? */
        ajTableClearDelete(thys);

        AJFREE(thys->buckets);
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
** @@
******************************************************************************/

void ajTableDelKeydelValdel(AjPTable* Ptable,
                            void keydel(void** Pkey),
                            void valdel(void** Pvalue))
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
    {					/* any other references? */
        thys->keydel = keydel;
        thys->valdel = valdel;

        ajTableClearDelete(thys);

        AJFREE(thys->buckets);
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
** @@
******************************************************************************/

void ajTableDelValdel(AjPTable* Ptable,
                      void valdel(void** Pvalue))
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
    {					/* any other references? */
        thys->valdel = valdel;

        ajTableClearDelete(thys);

        AJFREE(thys->buckets);
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
    {					/* any other references? */
        ajTableClear(thys);

        AJFREE(thys->buckets);
        AJFREE(*Ptable);
    }
    
    *Ptable = NULL;

    return;
}




/* @funcstatic tableFreeSetExpand *********************************************
**
** Expand the list of free structure bindings
**
** @return [void]
******************************************************************************/

static void tableFreeSetExpand (void)
{
    ajint newsize;

    if(!tableFreeSet)
    {
        tableFreeMax = 1024;
        AJCNEW0(tableFreeSet,tableFreeMax);

        return;
    }

    if(tableFreeMax >= 65536)
        return;

    newsize = tableFreeMax + tableFreeMax;
    AJCRESIZE0(tableFreeSet, tableFreeMax, newsize);
    tableFreeMax = newsize;

    return;
}




/* @funcstatic tableDel *******************************************************
**
** Delete an entry in a table by freeing the key and value pointers
**
** @param [d] key [void**] Standard argument. Table key.
** @param [d] value [void**] Standard argument. Table item.
** @param [u] cl [void*] Standard argument. Usually NULL.
** @return [void]
** @@
******************************************************************************/

static void tableDel(void** key, void** value, void* cl)
{
    AJFREE(*key);
    AJFREE(*value);
    
    if(!cl)
	return;

    return;
}




/* @funcstatic tableDelKey ****************************************************
**
** Delete an entry in a table by freeing the key pointers
**
** @param [d] key [void**] Standard argument. Table key.
** @param [d] value [void**] Standard argument. Table item.
** @param [u] cl [void*] Standard argument. Usually NULL.
** @return [void]
** @@
******************************************************************************/

static void tableDelKey(void** key, void** value, void* cl)
{
    AJFREE(*key);
    *value = NULL;
    
    if(!cl)
	return;

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
** @valrule *Length [ajuint] Number of unique keys and values
** @valrule *Match [AjBool] True if a match was found
** @valrule *Size [ajuint] Hash array size
** @valrule *Toarray [ajuint] Array size (not counting trailing NULL)
** @fcategory cast
**
******************************************************************************/




/* @func ajTableFetchC *****************************************************
**
** returns the value associated with key in table, or null
** if table does not hold key.
**
** @param [r] table [const AjPTable] table to search
** @param [r] txtkey [const char*] key to find.
** @return [const void*]  value associated with key
** @error NULL if key not found in table.
** @@
******************************************************************************/

const void* ajTableFetchC(const AjPTable table, const char* txtkey)
{
    ajint i;
    struct binding *p;

    if(!table)
	return NULL;

    if (!txtkey)
	return NULL;

    if(table->Type == TABLE_STR)
    {
        ajStrAssignC(&tableTmpkeyStr, txtkey);
        return ajTableFetchS(table, tableTmpkeyStr);
    }

    if(table->Type != TABLE_CHAR)
        ajFatal("ajTableFetchC called for %s table", tableType(table));

    i = (*table->hash)(txtkey, table->size);

    for(p = table->buckets[i]; p; p = p->link)
	if((*table->cmp)(txtkey, p->key) == 0)
	    break;

    return p ? p->value : NULL;
}




/* @func ajTableFetchS *****************************************************
**
** returns the value associated with key in table, or null
** if table does not hold key.
**
** @param [r] table [const AjPTable] table to search
** @param [r] key   [const AjPStr] key to find.
** @return [const void*]  value associated with key
** @error NULL if key not found in table.
** @@
******************************************************************************/

const void* ajTableFetchS(const AjPTable table, const AjPStr key)
{
    ajint i;
    struct binding *p;

    if(!table)
	return NULL;

    if (!key)
	return NULL;

    if(table->Type == TABLE_CHAR)
        return ajTableFetchC(table, MAJSTRGETPTR(key));

    if(table->Type != TABLE_STR)
        ajFatal("ajTableFetchS called for %s table", tableType(table));

    i = (*table->hash)(key, table->size);

    for(p = table->buckets[i]; p; p = p->link)
	if((*table->cmp)(key, p->key) == 0)
	    break;

    return p ? p->value : NULL;
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
** @@
******************************************************************************/

const void * ajTableFetchV(const AjPTable table, const void *key)
{
    ajint i;
    struct binding *p;

    if(!table)
	return NULL;

    if (!key)
	return NULL;

    i = (*table->hash)(key, table->size);

    for(p = table->buckets[i]; p; p = p->link)
	if((*table->cmp)(key, p->key) == 0)
	    break;

    return p ? p->value : NULL;
}




/* @obsolete ajTableFetch
** @rename ajTableFetchmodV
*/

__deprecated void * ajTableFetch(const AjPTable table, const void *key)
{
    ajint i;
    struct binding *p;

    if(!table)
	return NULL;

    if (!key)
	return NULL;

    i = (*table->hash)(key, table->size);

    for(p = table->buckets[i]; p; p = p->link)
	if((*table->cmp)(key, p->key) == 0)
	    break;

    return p ? p->value : NULL;
}




/* @func ajTableFetchmodC **************************************************
**
** returns the value associated with key in table, or null
** if table does not hold key.
**
** @param [r] table [const AjPTable] table to search
** @param [r] txtkey [const char*] key to find.
** @return [void*]  value associated with key
** @error NULL if key not found in table.
** @@
******************************************************************************/

void* ajTableFetchmodC(const AjPTable table, const char* txtkey)
{
    ajint i;
    struct binding *p;


    if(!table)
	return NULL;

    if (!txtkey)
	return NULL;

    if(table->Type == TABLE_STR)
    {
        ajStrAssignC(&tableTmpkeyStr, txtkey);
        return ajTableFetchmodS(table, tableTmpkeyStr);
    }

    if(table->Type != TABLE_CHAR)
        ajFatal("ajTableFetchmodC called for %s table", tableType(table));

    i = (*table->hash)(txtkey, table->size);

    for(p = table->buckets[i]; p; p = p->link)
	if((*table->cmp)(txtkey, p->key) == 0)
	    break;

    return p ? p->value : NULL;
}




/* @func ajTableFetchmodS **************************************************
**
** returns the value associated with key in table, or null
** if table does not hold key.
**
** @param [r] table [const AjPTable] table to search
** @param [r] key   [const AjPStr] key to find.
** @return [void*]  value associated with key
** @error NULL if key not found in table.
** @@
******************************************************************************/

void* ajTableFetchmodS(const AjPTable table, const AjPStr key)
{
    ajint i;
    struct binding *p;

    if(!table)
	return NULL;

    if (!key)
	return NULL;

    if(table->Type == TABLE_CHAR)
        return ajTableFetchmodC(table, MAJSTRGETPTR(key));

    if(table->Type != TABLE_STR)
        ajFatal("ajTableFetchmodS called for %s table", tableType(table));

    i = (*table->hash)(key, table->size);

    for(p = table->buckets[i]; p; p = p->link)
	if((*table->cmp)(key, p->key) == 0)
	    break;

    return p ? p->value : NULL;
}




/* @obsolete ajTablestrFetch
** @rename ajTablestrFetchS
*/
__deprecated const AjPStr ajTablestrFetch(const AjPTable table,
                                          const AjPStr key)
{
    return ajTablestrFetchS(table, key);
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
** @@
******************************************************************************/

void * ajTableFetchmodV(const AjPTable table, const void *key)
{
    ajint i;
    struct binding *p;

    if(!table)
	return NULL;

    if (!key)
	return NULL;

    i = (*table->hash)(key, table->size);

    for(p = table->buckets[i]; p; p = p->link)
	if((*table->cmp)(key, p->key) == 0)
	    break;

    return p ? p->value : NULL;
}




/* @obsolete ajTableGet
** @rename ajTableFetchmodV
*/

__deprecated void * ajTableGet(const AjPTable table, const void *key)
{
    return ajTableFetchmodV(table, key);
}




/* @obsolete ajTableFetchKey
** @rename ajTablestrFetchKeyS
*/

__deprecated const void * ajTableFetchKey(const AjPTable table,
                                          const void *key)
{
    ajint i;
    const struct binding *p;

    if (!table)
	return NULL;

    if (!key)
	return NULL;

    i = (*table->hash)(key, table->size);

    for(p = table->buckets[i]; p; p = p->link)
	if((*table->cmp)(key, p->key) == 0)
	    break;

    return p ? (const void*)p->key : NULL;
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
** @@
******************************************************************************/

void * ajTableFetchmodTraceV(const AjPTable table, const void *key)
{
    ajuint i;
    ajuint j = 0;
    struct binding *p;

    if(!table)
	return NULL;

    if (!key)
	return NULL;

    ajDebug("ajTableFetch length %u key %x (%Lu)\n",
            table->length, key, key);

    i = (*table->hash)(key, table->size);

    ajDebug("...hash to bucket %u\n", i);

    for(p = table->buckets[i]; p; p = p->link)
    {
	if((*table->cmp)(key, p->key) == 0)
	    break;
        j++;
    }

    if(p)
    {
        ajDebug("...tested %u keys found %x (%Lu)\n", j, p->value, p->value);
    }
    else
        ajDebug("...tested %u keys no match\n", j);

    return p ? p->value : NULL;
}




/* @func ajTableGetLength *****************************************************
**
** returns the number of key-value pairs in table.
**
** @param [r] table [const AjPTable] Table to be applied.
** @return [ajuint] number of key-value pairs.
** @@
******************************************************************************/

ajuint ajTableGetLength(const AjPTable table)
{
    if(!table)
	return 0;

    return table->length;
}




/* @obsolete ajTableLength
** @rename ajTableGetLength
*/

__deprecated ajint ajTableLength(const AjPTable table)
{
    return (ajuint) ajTableGetLength(table);
}




/* @func ajTableGetSize *******************************************************
**
** returns the size of the hash array in a table.
**
** @param [r] table [const AjPTable] Table to be applied.
** @return [ajuint] number of hash array positions
** @@
******************************************************************************/

ajuint ajTableGetSize(const AjPTable table)
{
    if(!table)
	return 0;

    return table->size;
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
** @@
******************************************************************************/

AjBool ajTableMatchC(const AjPTable table, const char* txtkey)
{
    ajint i;
    struct binding *p;

    if(!table)
	return ajFalse;

    if (!txtkey)
	return ajFalse;

    if(table->Type == TABLE_STR)
    {
        ajStrAssignC(&tableTmpkeyStr, txtkey);
        return ajTableMatchS(table, tableTmpkeyStr);
    }

    if(table->Type != TABLE_CHAR)
        ajFatal("ajTableMatchC called for %s table", tableType(table));

    i = (*table->hash)(txtkey, table->size);

    for(p = table->buckets[i]; p; p = p->link)
	if((*table->cmp)(txtkey, p->key) == 0)
	    break;

    return p ? ajTrue : ajFalse;
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
** @@
******************************************************************************/

AjBool ajTableMatchS(const AjPTable table, const AjPStr key)
{
    ajint i;
    struct binding *p;

    if(!table)
	return ajFalse;

    if (!key)
	return ajFalse;

    if(table->Type == TABLE_CHAR)
        return ajTableMatchC(table, MAJSTRGETPTR(key));

    if(table->Type != TABLE_STR)
        ajFatal("ajTableMatchS called for %s table", tableType(table));

    i = (*table->hash)(key, table->size);

    for(p = table->buckets[i]; p; p = p->link)
	if((*table->cmp)(key, p->key) == 0)
	    break;

    return p ? ajTrue : ajFalse;
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
** @@
******************************************************************************/

AjBool ajTableMatchV(const AjPTable table, const void* key)
{
    ajint i;
    struct binding *p;

    if(!table)
	return ajFalse;

    if (!key)
	return ajFalse;

    i = (*table->hash)(key, table->size);

    for(p = table->buckets[i]; p; p = p->link)
	if((*table->cmp)(key, p->key) == 0)
	    break;

    return p ? ajTrue : ajFalse;
}




/* @func ajTableToarrayKeys ***************************************************
**
** creates two N+1 element arrays that holds the N keys
** in table in an unspecified order and returns the number of elements.
** The final element of the array is NULL.
**
** @param [r] table [const AjPTable] Table
** @param [w] keyarray [void***] NULL terminated array of keys.
** @return [ajuint] size of array returned
** @@
******************************************************************************/

ajuint ajTableToarrayKeys(const AjPTable table,
                          void*** keyarray)
{
    ajuint i;
    ajint j = 0;
    struct binding *p;

    if (*keyarray)
	AJFREE(*keyarray);

    if(!table)
	return 0;
 
    *keyarray = AJALLOC((table->length + 1)*sizeof(keyarray));

    for(i = 0; i < table->size; i++)
	for(p = table->buckets[i]; p; p = p->link)
	{
	    (*keyarray)[j++] = p->key;
	}

    (*keyarray)[j] = NULL;

    return table->length;
}




/* @obsolete ajTableToarray
** @rename ajTableToarrayKeysValues
*/

__deprecated ajuint ajTableToarray(const AjPTable table,
                                   void*** keyarray, void*** valarray)
{

    return ajTableToarrayKeysValues(table, keyarray, valarray);
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
** @return [ajuint] size of arrays returned
** @@
******************************************************************************/

ajuint ajTableToarrayKeysValues(const AjPTable table,
                                void*** keyarray, void*** valarray)
{
    ajuint i;
    ajint j = 0;
    struct binding *p;

    if (*keyarray)
	AJFREE(*keyarray);

    if (*valarray)
	AJFREE(*valarray);

    if(!table)
	return 0;
 
    *keyarray = AJALLOC((table->length + 1)*sizeof(keyarray));
    *valarray = AJALLOC((table->length + 1)*sizeof(valarray));

    for(i = 0; i < table->size; i++)
	for(p = table->buckets[i]; p; p = p->link)
	{
	    (*keyarray)[j] = p->key;
	    (*valarray)[j++] = p->value;
	}

    (*keyarray)[j] = NULL;
    (*valarray)[j] = NULL;

    return table->length;
}




/* @func ajTableToarrayValues *************************************************
**
** creates two N+1 element arrays that holds the N values
** in table in an unspecified order and returns the number of elements.
** The final element of the array is NULL.
**
** @param [r] table [const AjPTable] Table
** @param [w] valarray [void***] NULL terminated array of values.
** @return [ajuint] size of array returned
** @@
******************************************************************************/

ajuint ajTableToarrayValues(const AjPTable table,
                            void*** valarray)
{
    ajuint i;
    ajint j = 0;
    struct binding *p;

    if (*valarray)
	AJFREE(*valarray);

    if(!table)
	return 0;
 
    *valarray = AJALLOC((table->length + 1)*sizeof(valarray));

    for(i = 0; i < table->size; i++)
	for(p = table->buckets[i]; p; p = p->link)
	{
	    (*valarray)[j++] = p->value;
	}

    (*valarray)[j] = NULL;

    return table->length;
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
** @@
******************************************************************************/

void ajTableTrace(const AjPTable table)
{
    ajuint i;
    ajint j;
    ajint k = 0;
    struct binding *p;

    if(!table)
	return;

    ajDebug("table trace: ");
    ajDebug(" length: %d", table->length);
    ajDebug(" size: %d", table->size);
    ajDebug(" timestamp: %u", table->timestamp);

    for(i = 0; i < table->size; i++)
	if(table->buckets[i])
	{
	    j = 0;

	    for(p = table->buckets[i]; p; p = p->link)
		j++;

	    k += j;
	}

    ajDebug(" links: %d\n", k);

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
** @@
******************************************************************************/

void * ajTablePut(AjPTable table, void *key, void *value)
{
    ajint i;
    struct binding *p;
    void *prev;
    ajuint minsize;

    if(!table)
	return NULL;

    if(!key)
	return NULL;

    minsize = table->length/8;
    if(table->size < minsize)
        ajTableResizeCount(table, 6*minsize);

    i = (*table->hash)(key, table->size);

    for(p = table->buckets[i]; p; p = p->link)
	if((*table->cmp)(key, p->key) == 0)
	    break;

    if(p == NULL)
    {
        if(tableFreeNext)
            p = tableFreeSet[--tableFreeNext];
        else
            AJNEW0(p);

	p->key = key;
	p->link = table->buckets[i];
	table->buckets[i] = p;
	table->length++;
	prev = NULL;
    }
    else
    {
        if(table->keydel)
        {
            table->keydel(&(p->key));
            p->key = key;
        }
	prev = p->value;
    }

    p->value = value;
    table->timestamp++;

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
** @@
******************************************************************************/

AjBool ajTablePutClean(AjPTable table, void *key, void *value,
                       void keydel(void** Pkey),
                       void valdel(void** Pvalue))
{
    ajint i;
    struct binding *p;
    ajuint minsize;
    AjBool ret = ajFalse;

    if(!table)
	return ajFalse;

    if(!key)
	return ajFalse;

    minsize = table->length/8;
    if(table->size < minsize)
        ajTableResizeCount(table, 6*minsize);

    i = (*table->hash)(key, table->size);

    for(p = table->buckets[i]; p; p = p->link)
	if((*table->cmp)(key, p->key) == 0)
	    break;

    if(p == NULL)
    {
        if(tableFreeNext)
            p = tableFreeSet[--tableFreeNext];
        else
            AJNEW0(p);

	p->key = key;
	p->link = table->buckets[i];
	table->buckets[i] = p;
	table->length++;
    }
    else
    {
/*
        {
            AjPStr tmpkey = NULL;
            AjPStr newkey = NULL;
            tableStrFromKey(table, p->key, &tmpkey);
            tableStrFromKey(table, key, &newkey);
            ajDebug("ajTablePutClean duplicate key %x (%S) == %x (%S)\n",
                    p->key, tmpkey, key, newkey);
            ajStrDel(&tmpkey);
            ajStrDel(&newkey);
        }
*/
        if((void *) keydel)
        {
            keydel(&(p->key));
        }

        else if(table->keydel)
        {
            table->keydel(&(p->key));
        }

        p->key = key;

        if((void *) valdel)
            valdel(&(p->value));

        ret = ajTrue;
    }

    p->value = value;
    table->timestamp++;

    return ret;
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
******************************************************************************/
static void tableStrFromKey(const AjPTable table, const void *key,
                            AjPStr* Pstr)
{
    if(table->hash == ajTablecharHash)
    {
        ajFmtPrintS(Pstr, "key '%s' (char*)",
                    (const char*) key);
    }
    else if(table->hash == ajTableintHash)
    {
        ajFmtPrintS(Pstr, "key '%d' (int)",
                    *((const ajint*) key));
    }
    else if(table->hash == ajTablelongHash)
    {
        ajFmtPrintS(Pstr, "key '%Ld' (int)",
                    *((const ajlong*) key));
    }
    else if(table->hash == ajTablestrHash)
    {
        ajFmtPrintS(Pstr, "key '%S' (string)",
                    (const AjPStr) key);
    }
    else if(table->hash == ajTableuintHash)
    {
        ajFmtPrintS(Pstr, "key '%u' (unsigned int)",
                    *((const ajuint*) key));
    }
    else if(table->hash == ajTableulongHash)
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
** @@
******************************************************************************/

void * ajTablePutTrace(AjPTable table, void *key, void *value)
{
    ajuint i;
    ajuint j = 0;
    struct binding *p;
    void *prev;
    ajuint minsize;

    if(!table)
	return NULL;

    if(!key)
	return NULL;

    if(!value)
	return NULL;

    minsize = table->length/6;
    if(table->size < minsize)
        ajTableResizeCount(table, 4*table->length);

    ajDebug("ajTablePut length %u key %x (%Lu) value %x (%Lu)\n",
            table->length, key, key, value, value);

    i = (*table->hash)(key, table->size);

    ajDebug("...hash to bucket %u\n", i);

    for(p = table->buckets[i]; p; p = p->link)
    {
        if((*table->cmp)(key, p->key) == 0)
	    break;
        j++;
    }
    
    ajDebug("...tested %u keys\n", j);

    if(p == NULL)
    {
        if(tableFreeNext)
            p = tableFreeSet[--tableFreeNext];
        else
            AJNEW0(p);

	p->key = key;
	p->link = table->buckets[i];
	table->buckets[i] = p;
	table->length++;
	prev = NULL;
    }
    else
    {
        ajDebug("...existing value %x %Lu\n", p->value, p->value);
	prev = p->value;
    }

    p->value = value;
    table->timestamp++;

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
** @@
******************************************************************************/

void * ajTableRemove(AjPTable table, const void *key)
{
    ajint i;
    struct binding **pp;

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

    table->timestamp++;
    i = (*table->hash)(key, table->size);

    for(pp = &table->buckets[i]; *pp; pp = &(*pp)->link)
	if((*table->cmp)(key, (*pp)->key) == 0)
	{
	    struct binding *p = *pp;
	    void *value = p->value;
	    *pp = p->link;

            if(table->keydel)
            {
                AjPStr tmpkey = NULL;
                AjPStr newkey = NULL;
                tableStrFromKey(table, p->key, &tmpkey);
                tableStrFromKey(table, key, &newkey);
                ajDebug("ajTableRemove key %x (%S) matched key %x (%S)\n",
                        key, newkey, p->key, tmpkey);
                table->keydel(&p->key);
                ajStrDel(&tmpkey);
                ajStrDel(&newkey);
            }

            if(tableFreeNext >= tableFreeMax)
                AJFREE(p);
            else
                tableFreeSet[tableFreeNext++] = p;

	    table->length--;

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
** @@
******************************************************************************/

void * ajTableRemoveKey(AjPTable table, const void *key, void** truekey)
{
    ajint i;
    struct binding **pp;

    if(!table)
	return NULL;

    if(!key)
	return NULL;

    table->timestamp++;
    i = (*table->hash)(key, table->size);

    for(pp = &table->buckets[i]; *pp; pp = &(*pp)->link)
	if((*table->cmp)(key, (*pp)->key) == 0)
	{
	    struct binding *p = *pp;
	    void *value = p->value;
	    *truekey = p->key;
	    *pp = p->link;

            if(tableFreeNext >= tableFreeMax)
                AJFREE(p);
            else
                tableFreeSet[tableFreeNext++] = p;

	    table->length--;

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
** @@
******************************************************************************/

void ajTableMap(AjPTable table,
		void apply(const void *key, void **value, void *cl),
		void *cl)
{
    ajuint i;
    ajuint stamp;
    struct binding *p;

    if(!table)
	return;

    stamp = table->timestamp;

    for(i = 0; i < table->size; i++)
	for(p = table->buckets[i]; p; p = p->link)
	{
	    apply(p->key, &p->value, cl);
	    assert(table->timestamp == stamp);
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
** @@
******************************************************************************/

void ajTableMapDel(AjPTable table,
                   void apply(void **key, void **value, void *cl),
                   void *cl)
{
    ajuint i;
    ajuint stamp;
    struct binding *p, *q;

    if(!table)
	return;

    stamp = table->timestamp;

    for(i = 0; i < table->size; i++)
    {
	for(p = table->buckets[i]; p; p = q)
	{
            q = p->link;

	    apply(&p->key, &p->value, cl);
	    assert(table->timestamp == stamp);
            table->length--;
            if(tableFreeNext >= tableFreeMax)
                tableFreeSetExpand();
            if(tableFreeNext >= tableFreeMax)
                AJFREE(p);
            else
                tableFreeSet[tableFreeNext++] = p;
	}
        table->buckets[i] = NULL;
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
** @argrule ResizeCount size [ajuint] Expected number of key-value pairs
** @argrule ResizeHashsize hashsize [ajuint] Required minimum hash array size
** @argrule SettypeUser cmp [ajint function] Comparison function returning
**                                        +1, zero or -1
** @argrule SettypeUser hash [ajuint function] Hash function for keys
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
** @@
******************************************************************************/

void ajTableClear(AjPTable table)
{
    ajuint i;
    ajuint ikeys;

    if (!table)
	return;

    /*ajDebug("ajTableClear length: %u size: %u type: %u\n",
      table->length, table->size, table->Type);*/

    if(table->length > 0)
    {
	struct binding *p, *q;

	for(i = 0; i < table->size; i++)
	{
            ikeys = 0;
            for(p = table->buckets[i]; p; p = q)
	    {
                ikeys++;

		q = p->link;

                if(tableFreeNext >= tableFreeMax)
                    tableFreeSetExpand();
                if(tableFreeNext >= tableFreeMax)
                    AJFREE(p);
                else
                    tableFreeSet[tableFreeNext++] = p;
            }
            table->buckets[i] = NULL;
        }
    }

    table->length = 0;

    return;
}




/* @func ajTableClearDelete ***************************************************
**
** Clears a hash table. Deletes all keys or values.
**
** @param [u] table [AjPTable] Table
** @return [void]
** @@
******************************************************************************/

void ajTableClearDelete(AjPTable table)
{
    ajuint i;
    struct binding *p, *q;

    if (!table)
	return;

    if(table->length > 0)
    {

	for(i = 0; i < table->size; i++)
	{
            for(p = table->buckets[i]; p; p = q)
	    {
		q = p->link;

                if(table->keydel)
                    table->keydel(&p->key);

                if(table->valdel)
                    table->valdel(&p->value);

                if(tableFreeNext >= tableFreeMax)
                    tableFreeSetExpand();
                if(tableFreeNext >= tableFreeMax)
                    AJFREE(p);
                else
                    tableFreeSet[tableFreeNext++] = p;
            }
            table->buckets[i] = NULL;
        }
    }

    table->length = 0;

    return;
}




/* @func ajTableResizeCount ***************************************************
**
** Resizes a hash table to a new number of expected values.
**
** @param [u] table [AjPTable] Table
** @param [r] size [ajuint] Expected number of key-value pairs
** @return [void]
** @@
******************************************************************************/

void ajTableResizeCount(AjPTable table, ajuint size)
{
    ajTableResizeHashsize(table, size/4);

    return;
}




/* @func ajTableResizeHashsize ************************************************
**
** Resizes a hash table to a new size.
**
** @param [u] table [AjPTable] Table
** @param [r] hashsize [ajuint] Expected number of key-value pairs
** @return [void]
** @@
******************************************************************************/

void ajTableResizeHashsize(AjPTable table, ajuint hashsize)
{
    ajuint n;
    void** keys = NULL;
    void** values = NULL;
    ajuint hint;
    ajuint iprime;
    ajuint ibucket;
    ajuint i;
    struct binding *p;
    ajuint savelength;

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

    if (!table)
	return;

    /* check the size required */

    hint = hashsize;

    for(i = 1; primes[i] <= hint; i++); /* else use default i=0 */

    iprime = primes[i-1];

    /*ajDebug("ajTableResizeHashsize test %x entries %u newhashsize %u "
            "hashsize %u newsize %u\n",
            table, table->length, hashsize, table->size, iprime);*/

    if(iprime == table->size)
        return;

    /* save the current keys and values */

    n = ajTableToarrayKeysValues(table, &keys, &values);

    AJCRESIZE0(table->buckets, table->size, iprime);
    table->size = iprime;

    /* empty the old buckets */

    savelength = table->length;
    ajTableClear(table);
    table->length = savelength;

    /*ajDebug("ajTableResizeHashsize yes %x length %u nkeys %u hashsize %u\n",
      table, table->length, n, table->size);*/

    /* repopulate the new bucket array */

    for(i=0; i < n; i++)
    {
        ibucket = (*table->hash)(keys[i], table->size);
        for(p = table->buckets[ibucket]; p; p = p->link)
            if((*table->cmp)(keys[i], p->key) == 0)
                break;

        if(p == NULL)
        {
            if(tableFreeNext)
                p = tableFreeSet[--tableFreeNext];
            else
                AJNEW0(p);

            p->key = keys[i];
            p->link = table->buckets[ibucket];
            table->buckets[ibucket] = p;
        }

        p->value = values[i];
    }

    AJFREE(keys);
    AJFREE(values);
    table->timestamp++;

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
** @@
******************************************************************************/

void ajTableSetDestroy(AjPTable table,
                       void keydel(void** key),
                       void valdel(void** value))
{
    if(!table)
        return;

    table->keydel = keydel;
    table->valdel = valdel;

    return;
}




/* @func ajTableSetDestroyboth ************************************************
**
** Sets the destructor functions values to be the same as for keys
**
** @param [u] table [AjPTable] Table
** @return [void]
** @@
******************************************************************************/

void ajTableSetDestroyboth(AjPTable table)
{
    if(!table)
        return;

       table->valdel = table->keydel;

    return;
}




/* @func ajTableSetDestroykey *************************************************
**
** Sets the destructor functions for keys
**
** @param [u] table [AjPTable] Table
** @param [fN] keydel [void function] key destructor function
** @return [void]
** @@
******************************************************************************/

void ajTableSetDestroykey(AjPTable table,
                          void keydel(void** key))
{
    if(!table)
        return;

    table->keydel = keydel;

    return;
}




/* @func ajTableSetDestroyvalue ***********************************************
**
** Sets the destructor functions for values
**
** @param [u] table [AjPTable] Table
** @param [fN] valdel [void function] value destructor function
** @return [void]
** @@
******************************************************************************/

void ajTableSetDestroyvalue(AjPTable table,
                            void valdel(void** value))
{
    if(!table)
        return;

    table->valdel = valdel;

    return;
}




/* @func ajTableSettypeChar ***************************************************
**
** Resets a hash table to use C character string keys
**
** @param [u] table [AjPTable] Hash table
** @return [void]
** @@
******************************************************************************/

void ajTableSettypeChar(AjPTable table)
{
    void** keys = NULL;
    void** values = NULL;
    ajuint n = 0;
    ajuint i = 0;

    n = ajTableToarrayKeysValues(table, &keys, &values);

    ajTableClear(table);
    
    table->cmp = ajTablecharCmp;
    table->hash = ajTablecharHash;
    table->keydel = ajMemFree;
    table->Type = TABLE_CHAR;

    if(n)
    {
        for(i=0; i<n; i++)
            ajTablePut(table, keys[i], values[i]);
    }

    AJFREE(keys);
    AJFREE(values);

    return;
}




/* @func ajTableSettypeCharCase ***********************************************
**
** Resets a hash table to use case-insensitive C character string keys
**
** @param [u] table [AjPTable] Hash table
** @return [void]
** @@
******************************************************************************/

void ajTableSettypeCharCase(AjPTable table)
{
    void** keys = NULL;
    void** values = NULL;
    ajuint n = 0;
    ajuint i = 0;

    n = ajTableToarrayKeysValues(table, &keys, &values);

    ajTableClear(table);
    
    table->cmp = ajTablecharCmpCase;
    table->hash = ajTablecharHashCase;
    table->keydel = ajMemFree;
    table->Type = TABLE_CHAR;

    if(n)
    {
        for(i=0; i<n; i++)
            ajTablePut(table, keys[i], values[i]);
    }

    AJFREE(keys);
    AJFREE(values);

    return;
}




/* @func ajTableSettypeDefault ************************************************
**
** Resets a hash table to use default keys
**
** @param [u] table [AjPTable] Hash table
** @return [void]
** @@
******************************************************************************/

void ajTableSettypeDefault(AjPTable table)
{
    void** keys = NULL;
    void** values = NULL;
    ajuint n = 0;
    ajuint i = 0;

    n = ajTableToarrayKeysValues(table, &keys, &values);

    ajTableClear(table);
    
    table->cmp = tableCmpAtom;
    table->hash = tableHashAtom;
    table->keydel = NULL;
    table->Type = TABLE_UNKNOWN;

    if(n)
    {
        for(i=0; i<n; i++)
            ajTablePut(table, keys[i], values[i]);
    }

    AJFREE(keys);
    AJFREE(values);

    return;
}




/* @func ajTableSettypeInt ****************************************************
**
** Resets a hash table to use integer keys
**
** @param [u] table [AjPTable] Hash table
** @return [void]
** @@
******************************************************************************/

void ajTableSettypeInt(AjPTable table)
{
    void** keys = NULL;
    void** values = NULL;
    ajuint n = 0;
    ajuint i = 0;

    n = ajTableToarrayKeysValues(table, &keys, &values);

    ajTableClear(table);
    
    table->cmp = ajTableintCmp;
    table->hash = ajTableintHash;
    table->keydel = ajMemFree;
    table->Type = TABLE_INT;

    if(n)
    {
        for(i=0; i<n; i++)
            ajTablePut(table, keys[i], values[i]);
    }

    AJFREE(keys);
    AJFREE(values);

    return;
}




/* @func ajTableSettypeLong ***************************************************
**
** Resets a hash table to use long integer keys
**
** @param [u] table [AjPTable] Hash table
** @return [void]
** @@
******************************************************************************/

void ajTableSettypeLong(AjPTable table)
{
    void** keys = NULL;
    void** values = NULL;
    ajuint n = 0;
    ajuint i = 0;

    n = ajTableToarrayKeysValues(table, &keys, &values);

    ajTableClear(table);
    
    table->cmp = ajTablelongCmp;
    table->hash = ajTablelongHash;
    table->keydel = ajMemFree;
    table->Type = TABLE_LONG;

    if(n)
    {
        for(i=0; i<n; i++)
            ajTablePut(table, keys[i], values[i]);
    }

    AJFREE(keys);
    AJFREE(values);

    return;
}




/* @func ajTableSettypeString *************************************************
**
** Resets a hash table to use string keys
**
** @param [u] table [AjPTable] Hash table
** @return [void]
** @@
******************************************************************************/

void ajTableSettypeString(AjPTable table)
{
    void** keys = NULL;
    void** values = NULL;
    ajuint n = 0;
    ajuint i = 0;

    n = ajTableToarrayKeysValues(table, &keys, &values);

    ajTableClear(table);
    
    table->cmp = ajTablestrCmp;
    table->hash = ajTablestrHash;
    table->keydel = tableDelStr;
    table->Type = TABLE_STR;

    if(n)
    {
        for(i=0; i<n; i++)
            ajTablePut(table, keys[i], values[i]);
    }

    AJFREE(keys);
    AJFREE(values);

    return;
}




/* @func ajTableSettypeStringCase *********************************************
**
** Resets a hash table to use case-insensitive string keys
**
** @param [u] table [AjPTable] Hash table
** @return [void]
** @@
******************************************************************************/

void ajTableSettypeStringCase(AjPTable table)
{
    void** keys = NULL;
    void** values = NULL;
    ajuint n = 0;
    ajuint i = 0;

    n = ajTableToarrayKeysValues(table, &keys, &values);

    ajTableClear(table);
    
    table->cmp = ajTablestrCmpCase;
    table->hash = ajTablestrHashCase;
    table->keydel = tableDelStr;
    table->Type = TABLE_STR;

    if(n)
    {
        for(i=0; i<n; i++)
            ajTablePut(table, keys[i], values[i]);
    }

    AJFREE(keys);
    AJFREE(values);

    return;
}




/* @func ajTableSettypeUint ***************************************************
**
** Resets a hash table to use unsigned integer keys
**
** @param [u] table [AjPTable] Hash table
** @return [void]
** @@
******************************************************************************/

void ajTableSettypeUint(AjPTable table)
{
    void** keys = NULL;
    void** values = NULL;
    ajuint n = 0;
    ajuint i = 0;

    n = ajTableToarrayKeysValues(table, &keys, &values);

    ajTableClear(table);
    
    table->cmp = ajTableuintCmp;
    table->hash = ajTableuintHash;
    table->keydel = ajMemFree;
    table->Type = TABLE_UINT;

    if(n)
    {
        for(i=0; i<n; i++)
            ajTablePut(table, keys[i], values[i]);
    }

    AJFREE(keys);
    AJFREE(values);

    return;
}




/* @func ajTableSettypeUlong **************************************************
**
** Resets a hash table to use unsigned long integer keys
**
** @param [u] table [AjPTable] Hash table
** @return [void]
** @@
******************************************************************************/

void ajTableSettypeUlong(AjPTable table)
{
    void** keys = NULL;
    void** values = NULL;
    ajuint n = 0;
    ajuint i = 0;

    n = ajTableToarrayKeysValues(table, &keys, &values);

    ajTableClear(table);
    
    table->cmp = ajTableulongCmp;
    table->hash = ajTableulongHash;
    table->keydel = ajMemFree;
    table->Type = TABLE_ULONG;

    if(n)
    {
        for(i=0; i<n; i++)
            ajTablePut(table, keys[i], values[i]);
    }

    AJFREE(keys);
    AJFREE(values);

    return;
}




/* @func ajTableSettypeUser ***************************************************
**
** Resets a hash table to use user-defined keys
**
**
** @param [u] table [AjPTable] Hash table
** @param [fN] cmp  [ajint function] function for comparing
** @param [fN] hash [ajuint function] function for hashing keys
** @return [void]
** @@
******************************************************************************/

void ajTableSettypeUser(AjPTable table,
                        ajint cmp(const void *x, const void *y),
                        ajuint hash(const void *key, ajuint hashsize))
{
    void** keys = NULL;
    void** values = NULL;
    ajuint n = 0;
    ajuint i = 0;

    n = ajTableToarrayKeysValues(table, &keys, &values);

    ajTableClear(table);
    
    table->cmp = cmp;
    table->hash = hash;
    table->keydel = NULL;
    table->Type = TABLE_USER;

    if(n)
    {
        for(i=0; i<n; i++)
            ajTablePut(table, keys[i], values[i]);
    }

    AJFREE(keys);
    AJFREE(values);

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
******************************************************************************/

AjBool ajTableMergeAnd(AjPTable thys, AjPTable table)
{
    ajuint i;
    struct binding *p;
    struct binding *q;
    struct binding *tmpnode;
    struct binding *pprev;
    struct binding *pfirst;
    struct binding *qfirst;

    AjBool ismatch;

    /* first we make both tables have the same hashsize */

    if(table->size > thys->size)
        ajTableResizeHashsize(thys, table->size);
    else if (thys->size > table->size)
        ajTableResizeHashsize(table, thys->size);

    /* compare keys and find matches */

    for(i=0; i < thys->size; i++)
    {
        pfirst = thys->buckets[i];
        qfirst = table->buckets[i];

        pprev = NULL;
        p = pfirst;

        while(p)
        {
            ismatch = ajFalse;

            for(q = qfirst; q; q = q->link)
            {
                if(!thys->cmp(p->key, q->key))
                {
                    ismatch = ajTrue;
                    break;
                }
            }
            
            if(ismatch)
            {
                pprev = p;
                p = p->link;
            }
            else
            {
                tmpnode = p->link;

                if(thys->keydel)
                    thys->keydel(&p->key);
                if(thys->valdel)
                    thys->valdel(&p->value);

                if(tableFreeNext >= tableFreeMax)
                    tableFreeSetExpand();
                if(tableFreeNext >= tableFreeMax)
                    AJFREE(p);
                else
                    tableFreeSet[tableFreeNext++] = p;

                p = tmpnode;

                if(pprev)
                    pprev->link = tmpnode;
                else
                    thys->buckets[i] = tmpnode;

                --thys->length;
            }
        }

        for(q = qfirst; q; q = tmpnode)
        {
            tmpnode = q->link;

            if(table->keydel)
                table->keydel(&q->key);

            if(table->valdel)
                table->valdel(&q->value);

            if(tableFreeNext >= tableFreeMax)
                tableFreeSetExpand();
            if(tableFreeNext >= tableFreeMax)
                AJFREE(q);
            else
                tableFreeSet[tableFreeNext++] = q;

            --table->length;
        }
        table->buckets[i] = NULL;
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
******************************************************************************/

AjBool ajTableMergeEor(AjPTable thys, AjPTable table)
{
    ajuint i;
    struct binding *p;
    struct binding *q;
    struct binding *tmpnode;
    struct binding *pprev;
    struct binding *qprev;
    struct binding *pfirst;
    struct binding *qfirst;
    struct binding *qtmp;

    AjBool ismatch;

    /* first we make both tables have the same hashsize */

    if(table->size > thys->size)
        ajTableResizeHashsize(thys, table->size);
    else if (thys->size > table->size)
        ajTableResizeHashsize(table, thys->size);

    /* compare keys and find matches */

    for(i=0; i < thys->size; i++)
    {
        pfirst = thys->buckets[i];
        qfirst = table->buckets[i];

        pprev = NULL;
        p = pfirst;
        while(p)
        {
            ismatch = ajFalse;

            qprev = NULL;
            for(q = qfirst; q; q = q->link)
            {
                if(!thys->cmp(p->key, q->key))
                {
                    ismatch = ajTrue;
                    break;
                }
                qprev = q;
            }
            
            if(!ismatch)
            {
                pprev = p;
                p = p->link;
            }
            else
            {
                tmpnode = p->link;

                if(thys->keydel)
                    thys->keydel(&p->key);
                if(thys->valdel)
                    thys->valdel(&p->value);

                if(tableFreeNext >= tableFreeMax)
                    tableFreeSetExpand();
                if(tableFreeNext >= tableFreeMax)
                    AJFREE(p);
                else
                    tableFreeSet[tableFreeNext++] = p;

                p = tmpnode;

                if(pprev)
                    pprev->link = tmpnode;
                else
                    thys->buckets[i] = tmpnode;

                --thys->length;

                tmpnode = q->link;

                if(table->keydel)
                    table->keydel(&q->key);

                if(table->valdel)
                    table->valdel(&q->value);

                if(tableFreeNext >= tableFreeMax)
                    tableFreeSetExpand();
                if(tableFreeNext >= tableFreeMax)
                    AJFREE(q);
                else
                    tableFreeSet[tableFreeNext++] = q;


                if(qprev)
                    qprev->link = tmpnode;
                else
                    qfirst = table->buckets[i] = tmpnode;

                --table->length;
            }
        }

        for(q = table->buckets[i]; q; q = qtmp)
        {
            qtmp = q->link;
            tmpnode = thys->buckets[i];
            thys->buckets[i] = q;
            q->link = tmpnode;

            --table->length;
            ++thys->length;
        }
        table->buckets[i] = NULL;
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
******************************************************************************/

AjBool ajTableMergeNot(AjPTable thys, AjPTable table)
{
    ajuint i;
    struct binding *p;
    struct binding *q;
    struct binding *tmpnode;
    struct binding *pprev;
    struct binding *pfirst;
    struct binding *qfirst;

    AjBool ismatch;

    /* first we make both tables have the same hashsize */

    if(table->size > thys->size)
        ajTableResizeHashsize(thys, table->size);
    else if (thys->size > table->size)
        ajTableResizeHashsize(table, thys->size);

    /* compare keys and find matches */

    for(i=0; i < thys->size; i++)
    {
        pfirst = thys->buckets[i];
        qfirst = table->buckets[i];

        pprev = NULL;
        p = pfirst;

        while(p)
        {
            ismatch = ajFalse;

            for(q = qfirst; q; q = q->link)
            {
                if(!thys->cmp(p->key, q->key))
                {
                    ismatch = ajTrue;
                    break;
                }
            }
            
            if(!ismatch)
            {
                pprev = p;
                p = p->link;
            }
            else
            {
                tmpnode = p->link;

                if(thys->keydel)
                    thys->keydel(&p->key);
                if(thys->valdel)
                    thys->valdel(&p->value);

                if(tableFreeNext >= tableFreeMax)
                    tableFreeSetExpand();
                if(tableFreeNext >= tableFreeMax)
                    AJFREE(p);
                else
                    tableFreeSet[tableFreeNext++] = p;

                p = tmpnode;

                if(pprev)
                    pprev->link = tmpnode;
                else
                    thys->buckets[i] = tmpnode;

                --thys->length;
            }
        }
        for(q = qfirst; q; q = tmpnode)
        {
            tmpnode = q->link;

            if(table->keydel)
                table->keydel(&q->key);

            if(table->valdel)
                table->valdel(&q->value);

            if(tableFreeNext >= tableFreeMax)
                tableFreeSetExpand();
            if(tableFreeNext >= tableFreeMax)
                AJFREE(q);
            else
                tableFreeSet[tableFreeNext++] = q;

            --table->length;
        }
        table->buckets[i] = NULL;
    }

    return ajTrue;
}




/* @func ajTableMergeOr ******************************************************
**
** Merge two tables, adding all unique keys from the new table
**
** @param [u] thys [AjPTable] Current table
** @param [u] table [AjPTable] Table to be merged
**
** @return [AjBool] True on success
**
******************************************************************************/

AjBool ajTableMergeOr(AjPTable thys, AjPTable table)
{
    ajuint i;
    struct binding *p;
    struct binding *q;
    struct binding *tmpnode;
    struct binding *qprev;
    struct binding *pfirst;
    struct binding *qfirst;
    struct binding *qtmp;

    AjBool ismatch;

     /* first we make both tables have the same hashsize */

    if(table->size > thys->size)
        ajTableResizeHashsize(thys, table->size);
    else if (thys->size > table->size)
        ajTableResizeHashsize(table, thys->size);

    /* compare keys and find matches */

    for(i=0; i < thys->size; i++)
    {
        pfirst = thys->buckets[i];
        qfirst = table->buckets[i];

        p = pfirst;

        while(p)
        {
            ismatch = ajFalse;

            qprev = NULL;
            for(q = qfirst; q; q = q->link)
            {
                if(!thys->cmp(p->key, q->key))
                {
                    ismatch = ajTrue;
                    break;
                }
                qprev = q;
            }
            
            p = p->link;

            if(ismatch)
            {
                tmpnode = q->link;

                if(table->keydel)
                    table->keydel(&q->key);
                if(table->valdel)
                    table->valdel(&q->value);

                if(tableFreeNext >= tableFreeMax)
                    tableFreeSetExpand();
                if(tableFreeNext >= tableFreeMax)
                    AJFREE(q);
                else
                    tableFreeSet[tableFreeNext++] = q;

                q = tmpnode;

                if(qprev)
                    qprev->link = tmpnode;
                else
                    qfirst = table->buckets[i] = tmpnode;

                --table->length;
            }
        }

        for(q = table->buckets[i]; q; q = qtmp)
        {
            qtmp = q->link;
            tmpnode = thys->buckets[i];
            thys->buckets[i] = q;
            q->link = tmpnode;

            --table->length;
            ++thys->length;
        }
        table->buckets[i] = NULL;
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
** @argrule Cmp x [const void*] First key
** @argrule Cmp y [const void*] Second key
** @argrule Hash key [const void*] Key
** @argrule Hash hashsize [ajuint] Hash table size
** @suffix Case Case insensitive keys
**
** @valrule Cmp [ajint] Comparison result 0 for a match, -1 or +1 for a mismatch
** @valrule Hash [ajuint] hash value
**
** @fcategory misc
**
******************************************************************************/




/* @funcstatic tableCmpAtom ***************************************************
**
** Default comparison function for key comparison
**
** @param [r] x [const void*] First key
** @param [r] y [const void*] Second key
** @return [ajint] 0 for success, 1 for different keys
** @@
******************************************************************************/


static ajint tableCmpAtom(const void *x, const void *y)
{
    return x != y;
}




/* @funcstatic tableHashAtom **************************************************
**
** Default hash function for key indexing
**
** @param [r] key [const void*] Key
** @param [r] hashsize [ajuint] Hash size (maximum hash value)
** @return [ajuint] Hash value in range 0 to hashsize-1
** @@
******************************************************************************/

static ajuint tableHashAtom(const void *key, ajuint hashsize)
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
*/




/* @func ajTableExit **********************************************************
**
** Prints a summary of table usage with debug calls
**
** @return [void]
** @@
******************************************************************************/

void ajTableExit(void)
{
    ajint i;

#ifdef AJ_SAVESTATS
    ajDebug("Table usage : %d opened, %d closed, %d maxsize, %d maxmem\n",
	    tableNewCnt, tableDelCnt, tableMaxNum, tableMaxMem);
#endif

    if(tableFreeNext)
        for(i=0;i<tableFreeNext;i++)
            AJFREE(tableFreeSet[i]);

            
    if(tableFreeSet) 
        AJFREE(tableFreeSet);

    ajStrDel(&tableTmpkeyStr);

    tableFreeNext = 0;
    tableFreeMax = 0;
    
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
** @argrule New size [ajuint] Number of key values
**
** @valrule * [AjPTable] New hash table
**
******************************************************************************/




/* @func ajTablecharNew *******************************************************
**
** Creates a table with a character string key.
**
** @param [r] size [ajuint] number of key-value pairs
** @return [AjPTable] New table object with a character string key.
** @@
******************************************************************************/

AjPTable ajTablecharNew(ajuint size)
{
    AjPTable ret;

    ret = ajTableNewFunctionLen(size, ajTablecharCmp, ajTablecharHash,
                                ajMemFree, NULL);
    ret->Type = TABLE_CHAR;

    return ret;
}




/* @func ajTablecharNewCase ***************************************************
**
** Creates a table with a character string key and case insensitive searching.
**
** @param [r] size [ajuint] Hash size estimate.
** @return [AjPTable] New table object with a character string key.
** @@
******************************************************************************/

AjPTable ajTablecharNewCase(ajuint size)
{
    AjPTable ret;

    ret = ajTableNewFunctionLen(size, ajTablecharCmpCase, ajTablecharHashCase,
                                ajMemFree, NULL);
    ret->Type = TABLE_CHAR;

    return ret;
}




/* @func ajTablecharNewCaseConst **********************************************
**
** Creates a table with a character string key and case insensitive searching.
**
** @param [r] size [ajuint] Hash size estimate.
** @return [AjPTable] New table object with a character string key.
** @@
******************************************************************************/

AjPTable ajTablecharNewCaseConst(ajuint size)
{
    AjPTable ret;

    ret = ajTableNewFunctionLen(size, ajTablecharCmpCase, ajTablecharHashCase,
                                NULL, NULL);
    ret->Type = TABLE_CHAR;

    return ret;
}




/* @func ajTablecharNewConst **************************************************
**
** Creates a table with a character string key.
**
** @param [r] size [ajuint] number of key-value pairs
** @return [AjPTable] New table object with a character string key.
** @@
******************************************************************************/

AjPTable ajTablecharNewConst(ajuint size)
{
    AjPTable ret;

    ret = ajTableNewFunctionLen(size, ajTablecharCmp, ajTablecharHash,
                                NULL, NULL);
    ret->Type = TABLE_CHAR;

    return ret;
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
** @@
******************************************************************************/

void ajTablecharPrint(const AjPTable table)
{
    ajuint i;
    struct binding *p;

    if(!table)
	return;

    for(i = 0; i < table->size; i++)
	for(p = table->buckets[i]; p; p = p->link)
	{
	    ajUser("key '%s' value '%s'",
		   (const char*) p->key, (char*) p->value);
	}

    return;
}




/* @obsolete ajStrTablePrintC
** @rename ajTablecharPrint
*/

__deprecated void ajStrTablePrintC(const AjPTable table)
{
    ajTablecharPrint(table);

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
** @argrule Cmp x [const void*] First key
** @argrule Cmp y [const void*] Second key
** @argrule Hash key [const void*] Key
** @argrule Hash hashsize [ajuint] Hash table size
** @suffix Case Case insensitive keys
**
** @valrule Cmp [ajint] Comparison result 0 for a match, -1 or +1 for a mismatch
** @valrule Hash [ajuint] hash value
**
** @fcategory misc
**
******************************************************************************/




/* @func ajTablecharCmp *******************************************************
**
** Comparison function for a table with a character string key
**
** @param [r] x [const void*] Standard argument. Item value.
** @param [r] y [const void*] Standard argument. Comparison item value.
** @return [ajint] Comparison result. Zero if equal, non-zero if different.
** @@
******************************************************************************/

ajint ajTablecharCmp(const void* x, const void* y)
{
    const char* sx;
    const char* sy;

    sx = (const char*) x;
    sy = (const char*) y;

    return (ajint)strcmp(sx, sy);
}




/* @obsolete ajStrTableCmpC
** @rename ajTablecharCmp
*/

__deprecated ajint ajStrTableCmpC(const void* x, const void* y)
{
    return ajTablecharCmp(x, y);
}




/* @func ajTablecharCmpCase ***************************************************
**
** Comparison function for a table with a character string key
** and case insensitivity.
**
** @param [r] x [const void*] Standard argument. Item value.
** @param [r] y [const void*] Standard argument. Comparison item value.
** @return [ajint] Comparison result. Zero if equal, non-zero if different.
** @@
******************************************************************************/

ajint ajTablecharCmpCase(const void* x, const void* y)
{
    const char* sx;
    const char* sy;

    sx = (const char*) x;
    sy = (const char*) y;

    return (ajint)ajCharCmpCase(sx, sy);
}




/* @obsolete ajStrTableCmpCaseC
** @rename ajTablecharCmpCase
*/

__deprecated ajint ajStrTableCmpCaseC(const void* x, const void* y)
{
    return ajTablecharCmpCase(x, y);
}




/* @func ajTablecharHash ******************************************************
**
** Hash function for a table with a character string key
**
** @param [r] key [const void*] Standard argument. Table key.
** @param [r] hashsize [ajuint] Standard argument. Estimated Hash size.
** @return [ajuint] Hash value.
** @@
******************************************************************************/

ajuint ajTablecharHash(const void* key, ajuint hashsize)
{
    ajuint hash;
    const char* s;

    s = (const char*) key;

    for(hash = 0; *s; s++)
	hash = (hash * 127 + *s) % hashsize;

    return hash;
}




/* @obsolete ajStrTableHashC
** @rename ajTablecharHash
*/

__deprecated ajuint ajStrTableHashC(const void* key, ajuint hashsize)
{
    return ajTablecharHash(key, hashsize);
}




/* @func ajTablecharHashCase **************************************************
**
** Hash function for a table with a character string key and
** case insensitivity.
**
** @param [r] key [const void*] Standard argument. Table key.
** @param [r] hashsize [ajuint] Standard argument. Estimated Hash size.
** @return [ajuint] Hash value.
** @@
******************************************************************************/

ajuint ajTablecharHashCase(const void* key, ajuint hashsize)
{
    ajuint hash;
    const char* s;

    s = (const char*) key;

    for(hash = 0; *s; s++)
	hash = (hash * 127 + toupper((ajint)*s)) % hashsize;

    return hash;
}




/* @obsolete ajStrTableHashCaseC
** @rename ajTablecharHashCase
*/

__deprecated ajuint ajStrTableHashCaseC(const void* key, ajuint hashsize)
{
    return ajTablecharHashCase(key, hashsize);
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
** @argrule New size [ajuint] Number of key values
**
** @valrule * [AjPTable] New hash table
**
******************************************************************************/




/* @func ajTableintNew ********************************************************
**
** Creates, initialises, and returns a new, empty table that can hold a
** specified number of integer key-value pairs.
**
** @param [r] size [ajuint] estimate of number of unique keys
**
** @return [AjPTable] new table.
** @@
**
******************************************************************************/

AjPTable ajTableintNew(ajuint size)
{
    AjPTable ret;

    ret = ajTableNewFunctionLen(size, ajTableintCmp, ajTableintHash,
                                ajMemFree, NULL);
    ret->Type = TABLE_INT;

    return ret;
}




/* @func ajTableintNewConst ***************************************************
**
** Creates, initialises, and returns a new, empty table that can hold a
** specified number of integer key-value pairs.
**
** @param [r] size [ajuint] estimate of number of unique keys
**
** @return [AjPTable] new table.
** @@
**
******************************************************************************/

AjPTable ajTableintNewConst(ajuint size)
{
    AjPTable ret;

    ret = ajTableNewFunctionLen(size, ajTableintCmp, ajTableintHash,
                                NULL, NULL);
    ret->Type = TABLE_INT;

    return ret;
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
** @@
******************************************************************************/

void ajTableintFree(AjPTable* Ptable)
{
    if(!Ptable)
        return;

    if(!*Ptable)
	return;

    ajTableMapDel(*Ptable, tableDel, NULL);

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
** @@
******************************************************************************/

void ajTableintFreeKey(AjPTable* Ptable)
{
    if(!Ptable)
        return;

    if(!*Ptable)
	return;

    ajTableMapDel(*Ptable, tableDelKey, NULL);

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
** @@
******************************************************************************/

const ajint* ajTableintFetch(const AjPTable table, const ajint *intkey)
{
    ajint i;
    struct binding *p;

    if(!table)
	return NULL;

    if (!intkey)
	return NULL;

    i = (*table->hash)(intkey, table->size);

    for(p = table->buckets[i]; p; p = p->link)
	if((*table->cmp)(intkey, p->key) == 0)
	    break;

    return p ? (const ajint*) p->value : NULL;
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
** @param [r] intkey   [const ajint *] key to find.
** @return [ajint*]  value associated with key
** @error NULL if key not found in table.
** @@
******************************************************************************/

ajint* ajTableintFetchmod(AjPTable table, const ajint *intkey)
{
    ajint i;
    struct binding *p;

    if(!table)
	return NULL;

    if (!intkey)
	return NULL;

    i = (*table->hash)(intkey, table->size);

    for(p = table->buckets[i]; p; p = p->link)
	if((*table->cmp)(intkey, p->key) == 0)
	    break;

    return p ? (ajint*) (&p->value) : NULL;
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
** @argrule Cmp x [const void*] First key
** @argrule Cmp y [const void*] Second key
** @argrule Hash key [const void*] Key
** @argrule Hash hashsize [ajuint] Hash table size
** @suffix Case Case insensitive keys
**
** @valrule Cmp [ajint] Comparison result 0 for a match, -1 or +1 for a mismatch
** @valrule Hash [ajuint] hash value
**
** @fcategory misc
**
******************************************************************************/




/* @func ajTableintCmp ********************************************************
**
** Comparison function for a table with an integer key
**
** @param [r] x [const void*] Standard argument. Item value.
** @param [r] y [const void*] Standard argument. Comparison item value.
** @return [ajint] Comparison result. Zero if equal, non-zero if different.
** @@
******************************************************************************/

ajint ajTableintCmp(const void* x, const void* y)
{
    const ajint *ix;
    const ajint *iy;

    ix = (const ajint*) x;
    iy = (const ajint*) y;

    return (*ix != *iy);
}




/* @func ajTableintHash *******************************************************
**
** Hash function for a table with an integer key
**
** @param [r] key [const void*] Standard argument. Table key.
** @param [r] hashsize [ajuint] Standard argument. Estimated Hash size.
** @return [ajuint] Hash value in range 0 to hashsize-1
** @@
******************************************************************************/

ajuint ajTableintHash(const void* key, ajuint hashsize)
{
    const ajint *ia;
    ajuint hash;

    if(!key)
        return 0;

    if(!hashsize)
        return 0;

    ia = (const ajint *) key;

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
** @argrule New size [ajuint] Number of key values
**
** @valrule * [AjPTable] New hash table
**
******************************************************************************/




/* @func ajTablelongNew *******************************************************
**
** Creates, initialises, and returns a new, empty table that can hold a
** specified number of long integer key-value pairs.
**
** @param [r] size [ajuint] estimate of number of unique keys
**
** @return [AjPTable] new table.
** @@
**
******************************************************************************/

AjPTable ajTablelongNew(ajuint size)
{
    AjPTable ret;

    ret = ajTableNewFunctionLen(size, ajTablelongCmp, ajTablelongHash,
                                ajMemFree, NULL);
    ret->Type = TABLE_LONG;

    return ret;
}




/* @func ajTablelongNewConst **************************************************
**
** Creates, initialises, and returns a new, empty table that can hold a
** specified number of long integer key-value pairs.
**
** @param [r] size [ajuint] estimate of number of unique keys
**
** @return [AjPTable] new table.
** @@
**
******************************************************************************/

AjPTable ajTablelongNewConst(ajuint size)
{
    AjPTable ret;

    ret = ajTableNewFunctionLen(size, ajTablelongCmp, ajTablelongHash,
                                NULL, NULL);
    ret->Type = TABLE_LONG;

    return ret;
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
** @@
******************************************************************************/

void ajTablelongFree(AjPTable* Ptable)
{
    if(!Ptable)
        return;

    if(!*Ptable)
	return;

    ajTableMapDel(*Ptable, tableDel, NULL);

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
** @@
******************************************************************************/

void ajTablelongFreeKey(AjPTable* Ptable)
{
    if(!Ptable)
        return;

    if(!*Ptable)
	return;

    ajTableMapDel(*Ptable, tableDelKey, NULL);

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
** @@
******************************************************************************/

const ajlong* ajTablelongFetch(const AjPTable table, const ajlong *longkey)
{
    ajint i;
    struct binding *p;

    if(!table)
	return NULL;

    if (!longkey)
	return NULL;

    i = (*table->hash)(longkey, table->size);

    for(p = table->buckets[i]; p; p = p->link)
	if((*table->cmp)(longkey, p->key) == 0)
	    break;

    return p ? (const ajlong*) p->value : NULL;
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
** @@
******************************************************************************/

ajlong* ajTablelongFetchmod(AjPTable table, const ajlong *longkey)
{
    ajint i;
    struct binding *p;

    if(!table)
	return NULL;

    if (!longkey)
	return NULL;

    i = (*table->hash)(longkey, table->size);

    for(p = table->buckets[i]; p; p = p->link)
	if((*table->cmp)(longkey, p->key) == 0)
	    break;

    return p ? (ajlong*) (&p->value) : NULL;
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
** @argrule Cmp x [const void*] First key
** @argrule Cmp y [const void*] Second key
** @argrule Hash key [const void*] Key
** @argrule Hash hashsize [ajuint] Hash table size
** @suffix Case Case insensitive keys
**
** @valrule Cmp [ajint] Comparison result 0 for a match, -1 or +1 for a mismatch
** @valrule Hash [ajuint] hash value
**
** @fcategory misc
**
******************************************************************************/




/* @func ajTablelongCmp *******************************************************
**
** Comparison function for a table with a long integer key
**
** @param [r] x [const void*] Standard argument. Item value.
** @param [r] y [const void*] Standard argument. Comparison item value.
** @return [ajint] Comparison result. Zero if equal, non-zero if different.
** @@
******************************************************************************/

ajint ajTablelongCmp(const void* x, const void* y)
{
    const ajlong *ix;
    const ajlong *iy;

    ix = (const ajlong*) x;
    iy = (const ajlong*) y;

    return (*ix != *iy);
}




/* @func ajTablelongHash ******************************************************
**
** Hash function for a table with a long integer key
**
** @param [r] key [const void*] Standard argument. Table key.
** @param [r] hashsize [ajuint] Standard argument. Estimated Hash size.
** @return [ajuint] Hash value in range 0 to hashsize-1
** @@
******************************************************************************/

ajuint ajTablelongHash(const void* key, ajuint hashsize)
{
    const ajlong *ia;
    ajuint hash;

    if(!key)
        return 0;

    if(!hashsize)
        return 0;

    ia = (const ajlong *) key;

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
** @argrule New size [ajuint] Number of key values
**
** @valrule * [AjPTable] New hash table
**
******************************************************************************/




/* @func ajTablestrNew *****************************************************
**
** Creates, initialises, and returns a new, empty table that can hold a
** specified number of key-value pairs.
**
** @param [r] size [ajuint] estimate of number of unique keys
**
** @return [AjPTable] new table.
** @@
**
******************************************************************************/

AjPTable ajTablestrNew(ajuint size)
{
    AjPTable ret;

    ret = ajTableNewFunctionLen(size, ajTablestrCmp, ajTablestrHash,
                                tableDelStr, NULL);
    ret->Type = TABLE_STR;

    return ret;
}




/* @obsolete ajTablestrNewLen
** @rename ajTablestrNew
*/

__deprecated AjPTable ajTablestrNewLen(ajuint size)
{
    AjPTable ret;

    ret = ajTableNewFunctionLen(size, ajTablestrCmp, ajTablestrHash,
                                NULL, NULL);
    ret->Type = TABLE_STR;

    return ret;
}




/* @func ajTablestrNewCase ****************************************************
**
** Creates, initialises, and returns a new, empty table that can hold a
** specified number of key-value pairs.
**
** @param [r] size [ajuint] estimate of number of unique keys
**
** @return [AjPTable] new table.
** @@
**
******************************************************************************/

AjPTable ajTablestrNewCase(ajuint size)
{
    AjPTable ret;

    ret = ajTableNewFunctionLen(size, ajTablestrCmpCase, ajTablestrHashCase,
                                tableDelStr, NULL);
    ret->Type = TABLE_STR;

    return ret;
}




/* @obsolete ajTablestrNewCaseLen
** @rename ajTablestrNewCase
*/
__deprecated AjPTable ajTablestrNewCaseLen(ajuint size)
{
    AjPTable ret;

    ret = ajTableNewFunctionLen(size, ajTablestrCmpCase, ajTablestrHashCase,
                                NULL, NULL);
    ret->Type = TABLE_STR;

    return ret;
}




/* @func ajTablestrNewCaseConst ***********************************************
**
** Creates, initialises, and returns a new, empty table that can hold a
** specified number of key-value pairs.
**
** @param [r] size [ajuint] estimate of number of unique keys
**
** @return [AjPTable] new table.
** @@
**
******************************************************************************/

AjPTable ajTablestrNewCaseConst(ajuint size)
{
    AjPTable ret;

    ret = ajTableNewFunctionLen(size, ajTablestrCmpCase, ajTablestrHashCase,
                                NULL, NULL);
    ret->Type = TABLE_STR;

    return ret;
}




/* @func ajTablestrNewConst ***************************************************
**
** Creates, initialises, and returns a new, empty table that can hold a
** specified number of key-value pairs.
**
** @param [r] size [ajuint] estimate of number of unique keys
**
** @return [AjPTable] new table.
** @@
**
******************************************************************************/

AjPTable ajTablestrNewConst(ajuint size)
{
    AjPTable ret;

    ret = ajTableNewFunctionLen(size, ajTablestrCmp, ajTablestrHash,
                                NULL, NULL);
    ret->Type = TABLE_STR;

    return ret;
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
** @@
******************************************************************************/

void ajTablestrFree(AjPTable* Ptable)
{
    if(!Ptable)
        return;

    if(!*Ptable)
	return;

    ajTableMapDel(*Ptable, tableStrDel, NULL);

    ajTableFree(Ptable);

    return;
}




/* @obsolete ajStrTableFree
** @rename ajTablestrFree
*/

__deprecated void ajStrTableFree(AjPTable* ptable)
{
    ajTablestrFree(ptable);

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
** @@
******************************************************************************/

void ajTablestrFreeKey(AjPTable* Ptable)
{
    if(!*Ptable)
	return;

    ajTableMapDel(*Ptable, tableStrDelKey, NULL);

    ajTableFree(Ptable);

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




/* @funcstatic tableStrDel ****************************************************
**
** Delete an entry in a string table.
**
** @param [d] key [void**] Standard argument. Table key.
** @param [d] value [void**] Standard argument. Table item.
** @param [u] cl [void*] Standard argument. Usually NULL.
** @return [void]
** @@
******************************************************************************/

static void tableStrDel(void** key, void** value, void* cl)
{
    AjPStr p;
    AjPStr q;

    p = (AjPStr) *value;
    q = (AjPStr) *key;

    ajStrDel(&p);
    ajStrDel(&q);

    *key = NULL;
    *value = NULL;

    if(!cl)
	return;

    return;
}




/* @funcstatic tableStrDelKey *************************************************
**
** Delete an entry in a table with a string key and ignore the value.
**
** @param [d] key [void**] Standard argument. Table key.
** @param [d] value [void**] Standard argument. Table item.
** @param [u] cl [void*] Standard argument. Usually NULL.
** @return [void]
** @@
******************************************************************************/

static void tableStrDelKey(void** key, void** value, void* cl)
{
    AjPStr q;

    q = (AjPStr) *key;

    ajStrDel(&q);

    *key = NULL;
    *value = NULL;

    if(!cl)
	return;

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
** @@
******************************************************************************/

void* ajTablestrFetchC(const AjPTable table, const char* txtkey)
{
    ajint i;
    struct binding *p;

    ajStrAssignC(&tableTmpkeyStr, txtkey);

    if(!table)
	return NULL;

    if (!txtkey)
	return NULL;

    i = (*table->hash)(tableTmpkeyStr, table->size);

    for(p = table->buckets[i]; p; p = p->link)
	if((*table->cmp)(tableTmpkeyStr, p->key) == 0)
	    break;

    return p ? p->value : NULL;
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
** @@
******************************************************************************/

void* ajTablestrFetchS(const AjPTable table, const AjPStr key)
{
    ajint i;
    struct binding *p;

    if(!table)
	return NULL;

    if (!key)
	return NULL;

    i = (*table->hash)(key, table->size);

    for(p = table->buckets[i]; p; p = p->link)
	if((*table->cmp)(key, p->key) == 0)
	    break;

    return p ? p->value : NULL;
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
** @@
******************************************************************************/

const AjPStr ajTablestrFetchkeyC(const AjPTable table, const char *txtkey)
{
    ajint i;
    const struct binding *p;

    if (!table)
	return NULL;

    if (!txtkey)
	return NULL;

    if(table->Type == TABLE_STR)
    {
        ajStrAssignC(&tableTmpkeyStr, txtkey);
        return ajTablestrFetchkeyS(table, tableTmpkeyStr);
    }

    if(table->Type != TABLE_CHAR)
        ajFatal("ajTablestrFetchkeyC called for %s table", tableType(table));

    i = (*table->hash)(txtkey, table->size);

    for(p = table->buckets[i]; p; p = p->link)
	if((*table->cmp)(txtkey, p->key) == 0)
	    break;

    return p ? (const void*)p->key : NULL;
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
** @@
******************************************************************************/

const AjPStr ajTablestrFetchkeyS(const AjPTable table, const AjPStr key)
{
    ajint i;
    const struct binding *p;

    if (!table)
	return NULL;

    if (!key)
	return NULL;

    if(table->Type == TABLE_CHAR)
    {
        return ajTablestrFetchkeyC(table, MAJSTRGETPTR(key));
    }

    if(table->Type != TABLE_STR)
        ajFatal("ajTablestrFetchkeyS called for %s table", tableType(table));

    i = (*table->hash)(key, table->size);

    for(p = table->buckets[i]; p; p = p->link)
	if((*table->cmp)(key, p->key) == 0)
	    break;

    return p ? (const void*)p->key : NULL;
}




/* @obsolete ajTableKey
** @rename ajTablestrFetchKeyS
*/

__deprecated const void * ajTableKey(const AjPTable table, const void *key)
{
    return ajTablestrFetchkeyS(table, key);
}




/* @funcstatic tableType ******************************************************
**
** returns the name of a table type.
**
** @param [r] table [const AjPTable] table type
** @return [const char*] Name of table type
** @@
******************************************************************************/

static const char* tableType(const AjPTable table)
{
    const char* typenames[] = {"unknown", "char", "string",
                               "int", "uint", "long", "ulong", "user", NULL};
    ajint i = table->Type;

    if(i < TABLE_UNKNOWN || i >= TABLE_MAX)
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
** @@
******************************************************************************/

AjPStr* ajTablestrFetchmod(AjPTable table, const AjPStr key)
{
    ajint i;
    struct binding *p;

    if(!table)
	return NULL;

    if (!key)
	return NULL;

    i = (*table->hash)(key, table->size);

    for(p = table->buckets[i]; p; p = p->link)
	if((*table->cmp)(key, p->key) == 0)
	    break;

    return p ? (AjPStr*) (&p->value) : NULL;
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
** @argrule Cmp x [const void*] First key
** @argrule Cmp y [const void*] Second key
** @argrule Hash key [const void*] Key
** @argrule Hash hashsize [ajuint] Hash table size
** @suffix Case Case insensitive keys
**
** @valrule Cmp [ajint] Comparison result 0 for a match, -1 or +1 for a mismatch
** @valrule Hash [ajuint] hash value
**
** @fcategory misc
**
******************************************************************************/




/* @func ajTablestrCmp ********************************************************
**
** Comparison function for a table with a string key
**
** @param [r] x [const void*] Standard argument. Item value.
** @param [r] y [const void*] Standard argument. Comparison item value.
** @return [ajint] Comparison result. Zero if equal, non-zero if different.
** @@
******************************************************************************/

ajint ajTablestrCmp(const void* x, const void* y)
{
    const AjPStr sx;
    const AjPStr sy;

    sx = (const AjPStr) x;
    sy = (const AjPStr) y;

    return (ajint)ajStrCmpS(sx, sy);
}




/* @obsolete ajStrTableCmp
** @rename ajTablestrCmp
*/

__deprecated ajint ajStrTableCmp(const void* x, const void* y)
{
    return ajTablestrCmp(x, y);
}




/* @func ajTablestrCmpCase ****************************************************
**
** Comparison function for a table with a string key
** and case insensitivity.
**
** @param [r] x [const void*] Standard argument. Item value.
** @param [r] y [const void*] Standard argument. Comparison item value.
** @return [ajint] Comparison result. Zero if equal, non-zero if different.
** @@
******************************************************************************/

ajint ajTablestrCmpCase(const void* x, const void* y)
{
    const AjPStr sx;
    const AjPStr sy;

    sx = (const AjPStr) x;
    sy = (const AjPStr) y;

    return (ajint)ajStrCmpCaseS(sx, sy);
}




/* @obsolete ajStrTableCmpCase
** @rename ajTablestrCmpCase
*/

__deprecated ajint ajStrTableCmpCase(const void* x, const void* y)
{
    return ajTablestrCmpCase(x, y);
}




/* @func ajTablestrHash *******************************************************
**
** Hash function for a table with a string key
**
** @param [r] key [const void*] Standard argument. Table key.
** @param [r] hashsize [ajuint] Standard argument. Estimated Hash size.
** @return [ajuint] Hash value in range 0 to hashsize-1
** @@
******************************************************************************/

ajuint ajTablestrHash(const void* key, ajuint hashsize)
{
    const AjPStr str;
    const char* s;
    ajuint hash;

    str = (const AjPStr) key;
    s   = ajStrGetPtr(str);

    for(hash = 0; *s; s++)
	hash = (hash * 127 + *s) % hashsize;

    return hash;
}




/* @obsolete ajStrTableHash
** @rename ajTablestrHash
*/

__deprecated ajuint ajStrTableHash(const void* key, ajuint hashsize)
{
    return ajTablestrHash(key, hashsize);
}




/* @func ajTablestrHashCase ***************************************************
**
** Hash function for a table with a string key and
** case insensitivity.
**
** @param [r] key [const void*] Standard argument. Table key.
** @param [r] hashsize [ajuint] Standard argument. Estimated Hash size.
** @return [ajuint] Hash value in range 0 to hashsize-1
** @@
******************************************************************************/

ajuint ajTablestrHashCase(const void* key, ajuint hashsize)
{
    const AjPStr str;
    const char* s;
    ajuint hash;

    str = (const AjPStr) key;
    s   = ajStrGetPtr(str);

    for(hash = 0; *s; s++)
	hash = (hash * 127 + toupper((ajint)*s)) % hashsize;

    return hash;
}




/* @obsolete ajStrTableHashCase
** @rename ajTablestrHashCase
*/

__deprecated ajuint ajStrTableHashCase(const void* key, ajuint hashsize)
{
    return ajTablestrHashCase(key, hashsize);
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
** @@
******************************************************************************/

void ajTablestrPrint(const AjPTable table)
{
    ajuint i;
    struct binding *p;

    if(!table)
	return;

    for(i = 0; i < table->size; i++)
	for(p = table->buckets[i]; p; p = p->link)
	    ajUser("key '%S' value '%S'",
		   (const AjPStr) p->key, (AjPStr) p->value);

    return;
}




/* @obsolete ajStrTablePrint
** @rename ajTablestrPrint
*/

__deprecated void ajStrTablePrint(const AjPTable table)
{
    ajTablestrPrint(table);
}




/* @func ajTablestrTrace ******************************************************
**
** Writes debug messages to trace the contents of a table,
** assuming all keys and values are strings.
**
** @param [r] table [const AjPTable] Table
** @return [void]
** @@
******************************************************************************/

void ajTablestrTrace(const AjPTable table)
{
    ajuint i;
    ajint j;
    ajint k = 0;
    struct binding *p;

    if(!table)
	return;

    ajDebug("(string) table trace: ");
    ajDebug(" length: %d", table->length);
    ajDebug(" size: %d", table->size);
    ajDebug(" timestamp: %u\n", table->timestamp);

    for(i = 0; i < table->size; i++)
	if(table->buckets[i])
	{
	    j = 0;
	    ajDebug("buckets[%d]\n", i);

	    for(p = table->buckets[i]; p; p = p->link)
	    {
		ajDebug("   '%S' => '%S'\n",
			(const AjPStr) p->key, (AjPStr) p->value);
		j++;
	    }

	    k += j;
	}

    ajDebug(" links: %d\n", k);

    return;
}




/* @obsolete ajStrTableTrace
** @rename ajTablestrTrace
*/

__deprecated void ajStrTableTrace(const AjPTable table)
{
    ajTablestrTrace(table);

    return;
}




/* @func ajTablestrTracekeys **************************************************
**
** Writes debug messages to trace the keys of a string table,
** assuming all keys and values are strings.
**
** @param [r] table [const AjPTable] Table
** @return [void]
** @@
******************************************************************************/

void ajTablestrTracekeys(const AjPTable table)
{
    ajuint i;
    ajint j;
    ajint k = 0;
    struct binding *p;

    if(!table)
	return;

    ajDebug("(string) table trace: ");
    ajDebug(" length: %d", table->length);
    ajDebug(" size: %d", table->size);
    ajDebug(" timestamp: %u\n", table->timestamp);

    for(i = 0; i < table->size; i++)
	if(table->buckets[i])
	{
	    j = 0;
	    ajDebug("buckets[%d]\n", i);

	    for(p = table->buckets[i]; p; p = p->link)
	    {
		ajDebug("   '%S' => '%xS\n",
			(const AjPStr) p->key, p->value);
		j++;
	    }

	    k += j;
	}

    ajDebug(" links: %d\n", k);

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
** @argrule New size [ajuint] Number of key values
**
** @valrule * [AjPTable] New hash table
**
******************************************************************************/




/* @func ajTableuintNew *******************************************************
**
** Creates, initialises, and returns a new, empty table that can hold a
** specified number of unsigned integer key-value pairs.
**
** @param [r] size [ajuint] estimate of number of unique keys
**
** @return [AjPTable] new table.
** @@
**
******************************************************************************/

AjPTable ajTableuintNew(ajuint size)
{
    AjPTable ret;

    ret = ajTableNewFunctionLen(size, ajTableuintCmp, ajTableuintHash,
                                ajMemFree, NULL);
    ret->Type = TABLE_UINT;

    return ret;
}




/* @func ajTableuintNewConst **************************************************
**
** Creates, initialises, and returns a new, empty table that can hold a
** specified number of unsigned integer key-value pairs.
**
** @param [r] size [ajuint] estimate of number of unique keys
**
** @return [AjPTable] new table.
** @@
**
******************************************************************************/

AjPTable ajTableuintNewConst(ajuint size)
{
    AjPTable ret;

    ret = ajTableNewFunctionLen(size, ajTableuintCmp, ajTableuintHash,
                                NULL, NULL);
    ret->Type = TABLE_UINT;

    return ret;
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
** @@
******************************************************************************/

void ajTableuintFree(AjPTable* Ptable)
{
    if(!Ptable)
        return;

    if(!*Ptable)
	return;

    ajTableMapDel(*Ptable, tableDel, NULL);

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
** @@
******************************************************************************/

void ajTableuintFreeKey(AjPTable* Ptable)
{
    if(!Ptable)
        return;

    if(!*Ptable)
	return;

    ajTableMapDel(*Ptable, tableDelKey, NULL);

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
** @@
******************************************************************************/

const ajuint* ajTableuintFetch(const AjPTable table, const ajuint *uintkey)
{
    ajint i;
    struct binding *p;

    if(!table)
	return NULL;

    if (!uintkey)
	return NULL;

    i = (*table->hash)(uintkey, table->size);

    for(p = table->buckets[i]; p; p = p->link)
	if((*table->cmp)(uintkey, p->key) == 0)
	    break;

    return p ? (const ajuint*) p->value : NULL;
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
** @param [r] uintkey   [const ajuint *] key to find.
** @return [ajuint*]  value associated with key
** @error NULL if key not found in table.
** @@
******************************************************************************/

ajuint* ajTableuintFetchmod(AjPTable table, const ajuint *uintkey)
{
    ajint i;
    struct binding *p;

    if(!table)
	return NULL;

    if (!uintkey)
	return NULL;

    i = (*table->hash)(uintkey, table->size);

    for(p = table->buckets[i]; p; p = p->link)
	if((*table->cmp)(uintkey, p->key) == 0)
	    break;

    return p ? (ajuint*) (&p->value) : NULL;
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
** @argrule Cmp x [const void*] First key
** @argrule Cmp y [const void*] Second key
** @argrule Hash key [const void*] Key
** @argrule Hash hashsize [ajuint] Hash table size
** @suffix Case Case insensitive keys
**
** @valrule Cmp [ajint] Comparison result 0 for a match, -1 or +1 for a mismatch
** @valrule Hash [ajuint] hash value
**
** @fcategory misc
**
******************************************************************************/




/* @func ajTableuintCmp *******************************************************
**
** Comparison function for a table with an unsigned integer key
**
** @param [r] x [const void*] Standard argument. Item value.
** @param [r] y [const void*] Standard argument. Comparison item value.
** @return [ajint] Comparison result. Zero if equal, non-zero if different.
** @@
******************************************************************************/

ajint ajTableuintCmp(const void* x, const void* y)
{
    const ajuint *ix;
    const ajuint *iy;

    ix = (const ajuint*) x;
    iy = (const ajuint*) y;

    return (*ix != *iy);
}




/* @func ajTableuintHash ******************************************************
**
** Hash function for a table with an unsigned integer key
**
** @param [r] key [const void*] Standard argument. Table key.
** @param [r] hashsize [ajuint] Standard argument. Estimated Hash size.
** @return [ajuint] Hash value in range 0 to hashsize-1
** @@
******************************************************************************/

ajuint ajTableuintHash(const void* key, ajuint hashsize)
{
    const ajuint *ia;
    ajuint hash;

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
** @argrule New size [ajuint] Number of key values
**
** @valrule * [AjPTable] New hash table
**
******************************************************************************/




/* @func ajTableulongNew ******************************************************
**
** Creates, initialises, and returns a new, empty table that can hold an
** arbitrary number of unsigned long integer key-value pairs
**
** @param [r] size [ajuint] estimate of number of unique keys
** @return [AjPTable] new table.
** @@
**
******************************************************************************/

AjPTable ajTableulongNew(ajuint size)
{
    AjPTable ret;

    ret = ajTableNewFunctionLen(size, ajTableulongCmp, ajTableulongHash,
                                ajMemFree, NULL);
    ret->Type = TABLE_ULONG;

    return ret;
}





/* @func ajTableulongNewConst *************************************************
**
** Creates, initialises, and returns a new, empty table that can hold an
** arbitrary number of unsigned long integer key-value pairs
**
** @param [r] size [ajuint] estimate of number of unique keys
** @return [AjPTable] new table.
** @@
**
******************************************************************************/

AjPTable ajTableulongNewConst(ajuint size)
{
    AjPTable ret;

    ret = ajTableNewFunctionLen(size, ajTableulongCmp, ajTableulongHash,
                                NULL, NULL);
    ret->Type = TABLE_ULONG;

    return ret;
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
** @@
******************************************************************************/

void ajTableulongFree(AjPTable* Ptable)
{
    if(!Ptable)
        return;

    if(!*Ptable)
	return;

    ajTableMapDel(*Ptable, tableDel, NULL);

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
** @@
******************************************************************************/

void ajTableulongFreeKey(AjPTable* Ptable)
{
    if(!Ptable)
        return;

    if(!*Ptable)
	return;

    ajTableMapDel(*Ptable, tableDelKey, NULL);

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
** @@
******************************************************************************/

const ajulong* ajTableulongFetch(const AjPTable table, const ajulong *ulongkey)
{
    ajint i;
    struct binding *p;

    if(!table)
	return NULL;

    if (!ulongkey)
	return NULL;

    i = (*table->hash)(ulongkey, table->size);

    for(p = table->buckets[i]; p; p = p->link)
	if((*table->cmp)(ulongkey, p->key) == 0)
	    break;

    return p ? (const ajulong*) p->value : NULL;
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
** @@
******************************************************************************/

ajulong* ajTableulongFetchmod(AjPTable table, const ajulong *ulongkey)
{
    ajint i;
    struct binding *p;

    if(!table)
	return NULL;

    if (!ulongkey)
	return NULL;

    i = (*table->hash)(ulongkey, table->size);

    for(p = table->buckets[i]; p; p = p->link)
	if((*table->cmp)(ulongkey, p->key) == 0)
	    break;

    return p ? (ajulong*) (&p->value) : NULL;
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
** @argrule Cmp x [const void*] First key
** @argrule Cmp y [const void*] Second key
** @argrule Hash key [const void*] Key
** @argrule Hash hashsize [ajuint] Hash table size
** @suffix Case Case insensitive keys
**
** @valrule Cmp [ajint] Comparison result 0 for a match, -1 or +1 for a mismatch
** @valrule Hash [ajuint] hash value
**
** @fcategory misc
**
******************************************************************************/




/* @func ajTableulongCmp ******************************************************
**
** Comparison function for a table with an unsigned long integer key
**
** @param [r] x [const void*] Standard argument. Item value.
** @param [r] y [const void*] Standard argument. Comparison item value.
** @return [ajint] Comparison result. Zero if equal, non-zero if different.
** @@
******************************************************************************/

ajint ajTableulongCmp(const void* x, const void* y)
{
    const ajulong ix = *(const ajulong*)x;
    const ajulong iy = *(const ajulong*)y;

    if(ix == iy)
        return 0;

    return 1;
}




/* @func ajTableulongHash *****************************************************
**
** Hash function for a table with an unsigned long integer key
**
** @param [r] key [const void*] Standard argument. Table key.
** @param [r] hashsize [ajuint] Standard argument. Estimated Hash size.
** @return [ajuint] Hash value in range 0 to hashsize-1
** @@
******************************************************************************/

ajuint ajTableulongHash(const void* key, ajuint hashsize)
{
    const ajulong *ia;
    ajuint hash;

    if(!key)
        return 0;

    if(!hashsize)
        return 0;

    ia = (const ajulong *) key;

    hash = (*ia >> 2) % hashsize;

    return hash;
}




/* @funcstatic tableDelStr ****************************************************
**
** Delete a string object value
**
** @param [d] Pdata [void**] Object to be deleted
** @return [void]
******************************************************************************/

static void tableDelStr(void** Pdata)
{
    AjPStr s;

    if(!Pdata)
        return;
    if(!*Pdata)
        return;

    s = (AjPStr) *Pdata;

    ajStrDel(&s);

    *Pdata = NULL;

    return;
}

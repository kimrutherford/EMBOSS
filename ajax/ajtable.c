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




#ifdef AJ_SAVESTATS
static ajuint tableNewCnt = 0;
static ajint tableDelCnt = 0;
static ajuint tableMaxNum = 0;
static size_t tableMaxMem = 0;
#endif

static ajint tableFreeNext = 0;
static ajint tableFreeMax = 0;
static struct binding ** tableFreeSet = NULL;

static void tableStrDel(void** key, void** value, void* cl);
static void tableStrDelKey(void** key, void** value, void* cl);


/* @filesection ajtable ********************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
** @suffix C [char*] C character string
** @suffix S [AjPStr] string object
** @suffix Len [ajuint] size of hash table
*/




/* @datasection [AjPTable] Hash tables ****************************************
**
** Function is for manipulating hash tables with any value type.
**
** Some functions are specially designed to understand string (AjPStr) values.
**
** @nam2rule Table
*/



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
** @argrule Len size [ajuint] Number of key values
** @argrule Function cmp [ajint function] Comparison function returning
**                                        +1, zero or -1
** @argrule Function hash [ajuint function] Hash function for keys
**
** @valrule * [AjPTable] New hash table
**
******************************************************************************/

/* @func ajTableNewFunctionLen *************************************************
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
**
** @return [AjPTable] new table.
**
** @@
**
******************************************************************************/

AjPTable ajTableNewFunctionLen(ajuint size,
			       ajint cmp(const void *x, const void *y),
			       ajuint hash(const void *key, ajuint hashsize))
{
    ajuint hint;
    ajuint i;
    ajuint iprime;
    AjPTable table;

    /* largest primes just under 2**8 to 2**31 */

    static ajuint primes[] =
    {
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

    for(i = 1; primes[i] < hint; i++); /* else use default i=0 */

    iprime = primes[i-1];
    ajDebug("ajTableNewFunctionLen hint %d size %d\n", hint, iprime);

    table = AJALLOC(sizeof(*table) +
		    iprime*sizeof(table->buckets[0]));
    table->size = iprime;
    table->cmp  = cmp;
    table->hash = hash;
    table->buckets = (struct binding **)(table + 1);

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

    return table;
}

/* @obsolete ajTableNewL
** @rename ajTableNewFunctionLen
*/

__deprecated AjPTable ajTableNewL(ajuint size,
				  ajint cmp(const void *x, const void *y),
				  ajuint hash(const void *key, ajuint hashsize))
{

    return ajTableNewFunctionLen(size, cmp, hash);
}




/* @obsolete ajTableNew
** @rename ajTableNewFunctionLen
*/

__deprecated AjPTable ajTableNew(ajuint hint,
		    ajint cmp(const void *x, const void *y),
		    ajuint hash(const void *key, ajuint hashsize))
{
    return ajTableNewFunctionLen(4*hint, cmp, hash);
}


/* @func ajTableNewLen *********************************************************
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

AjPTable ajTableNewLen(ajuint size)
{
    return ajTableNewFunctionLen(size, tableCmpAtom, tableHashAtom);
}

/* @section Retrieval **********************************************************
**
** @fdata [AjPTable]
**
** Retrieves values from a hash table
**
** @nam3rule Fetch Retrieval fuction
** @nam4rule FetchKey Key retrieval fuction
** @nam3rule Get return attribute
** @nam4rule GetLength Table size
** @nam3rule Toarray Return keys and values as arrays
**
** @argrule * table [const AjPTable] Hash table
** @argrule Fetch key [const void*] Key
** @argrule Toarray keyarray [void***] Array of keys, ending with NULL
** @argrule Toarray valarray [void***] Array of values, ending with NULL
**
** @valrule Fetch [void*] Value
** @valrule *Key [const void*] Key
** @valrule *Length [ajuint] Number of unique keys and values
** @valrule Toarray [ajuint] Array size (not counting trailing NULL)
** @fcategory cast
**
******************************************************************************/

/* @func ajTableFetch *********************************************************
**
** Returns the value associated with key in table, or null
** if table does not hold key.
**
** @param [r] table [const AjPTable] table to search
** @param [r] key   [const void*] key to find.
** @return [void*]  value associated with key
** @error NULL if key not found in table.
** @category cast [AjPTable] Returns the value for a given
**                key.
** @@
******************************************************************************/

void * ajTableFetch(const AjPTable table, const void *key)
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
** @rename ajTableFetch
*/

__deprecated void * ajTableGet(const AjPTable table, const void *key)
{
    return ajTableFetch(table, key);
}


/* @func ajTableFetchKey ******************************************************
**
** returns the key value associated with key in table, or null
** if table does not hold key.
**
** Intended for case-insensitive keys, to return the true key
**
** @param [r] table [const AjPTable] table to search
** @param [r] key   [const void*] key to find.
** @return [const void*] key value as stored in the table
** @error NULL if key not found in table.
** @@
******************************************************************************/

const void * ajTableFetchKey(const AjPTable table, const void *key)
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

/* @obsolete ajTableKey
** @rename ajTableGetKey
*/

__deprecated const void * ajTableKey(const AjPTable table, const void *key)
{
    return ajTableFetchKey(table, key);
}

/* @func ajTableGetLength ****************************************************
**
** returns the number of key-value pairs in table.
**
** @param [r] table [const AjPTable] Table to be applied.
** @return [ajuint] number of key-value pairs.
** @category cast [AjPTable] Returns the number of keys in a
**                table.
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

/* @func ajTableToarray *******************************************************
**
** creates a 2N+1 element array that holds the N key-value pairs
** in table in an unspecified order and returns a pointer to the
** first element. The keys appear in the even-numbered array
** elements and the corresponding values appear in the following
** odd-numbered elements; element 2N is end.
**
** @param [r] table [const AjPTable] Table
** @param [w] keyarray [void***] NULL terminated array of keys.
** @param [w] valarray [void***] NULL terminated array of s.
** @return [ajuint] size of arrays returned
** @category cast [AjPTable] Creates an array to hold each key
**                value pair in pairs of array elements. The last
**                element is null.
** @@
******************************************************************************/

ajuint ajTableToarray(const AjPTable table,
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
** @category output [AjPTable] Writes debug messages to trace the
**                contents of a table.
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
	    {
		j++;
	    }
	    k += j;
	}

    ajDebug(" links: %d\n", k);

    return;
}



/* @section Adding values ******************************************************
**
** @fdata [AjPTable]
**
** @nam3rule Put Add new key and value
** @nam3rule Remove Remove one key and value
** @nam4rule RemoveKey Remove one key and value, return key for deletion
**
** @argrule * table [AjPTable] Hash table
** @argrule Put key [void*] Key
** @argrule Put value [void*] Value
** @argrule Remove key [const void*] Key
** @argrule RemoveKey truekey [void**] Removed key poiner - ready to be freed
**
** @valrule * [void*] Previous value for key, or NULL
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
** @category modify [AjPTable] Adds or updates a value for a given
**                key.
** @@
******************************************************************************/

void * ajTablePut(AjPTable table, void *key, void *value)
{
    ajint i;
    struct binding *p;
    void *prev;

    if(!table)
	return NULL;
    if(!key)
	return NULL;

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
** @category modify [AjPTable] Removes a key/value pair from a
**                table, and returns the value.
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

    table->timestamp++;
    i = (*table->hash)(key, table->size);
    for(pp = &table->buckets[i]; *pp; pp = &(*pp)->link)
	if((*table->cmp)(key, (*pp)->key) == 0)
	{
	    struct binding *p = *pp;
	    void *value = p->value;
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
** @category modify [AjPTable] Removes a key/value pair from a
**                table, and returns the value.
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


/* @func ajTableMap **********************************************************
**
** calls function 'apply' for each key-value in table
** in an unspecified order. The table keys should not be modified by
** function 'apply' although values canbe updated.
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
** @category modify [AjPTable] Calls a function for each key/value
**                in a table.
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




/* @func ajTableMapDel *****************************************************
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
** @category modify [AjPTable] Calls a function for each key/value
**                in a table.
** @@
******************************************************************************/

void ajTableMapDel(AjPTable table,
		void apply(void **key, void **value, void *cl),
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
	    apply(&p->key, &p->value, cl);
	    assert(table->timestamp == stamp);
	}

    return;
}




/* @section Destructors *******************************************************
**
** @fdata [AjPTable]
**
** Destructors know they are dealing with strings and can
** clean up keys and values
**
** @nam3rule Free Delete table, keys and values
**
** @argrule * Ptable [AjPTable*] Hash table
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/

/* @funcstatic tableFreeSetExpand **********************************************
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

    
    
/* @func ajTableFree **********************************************************
**
** Deallocates and clears a hash table. Does not clear keys or values.
**
** @param [d] Ptable [AjPTable*] Table (by reference)
** @return [void]
** @category delete [AjPTable] Deallocates and clears a
**                table.
** @@
******************************************************************************/

void ajTableFree(AjPTable* Ptable)
{
    ajuint i;

    if (!Ptable)
	return;
    if (!*Ptable)
	return;

    if((*Ptable)->length > 0)
    {
	struct binding *p, *q;
	for(i = 0; i < (*Ptable)->size; i++)
	    for(p = (*Ptable)->buckets[i]; p; p = q)
	    {
		q = p->link;
                if(tableFreeNext >= tableFreeMax)
                    tableFreeSetExpand();
                if(tableFreeNext >= tableFreeMax)
                    AJFREE(p);
                else
                    tableFreeSet[tableFreeNext++] = p;
            }
    }

    AJFREE(*Ptable);

    return;
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

    tableFreeNext = 0;
    tableFreeMax = 0;
    
    return;
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
** @argrule Len size [ajuint] Number of key values
**
** @valrule * [AjPTable] New hash table
**
******************************************************************************/

/* @func ajTablestrNew ********************************************************
**
** Creates, initialises, and returns a new, empty table that can hold an
** arbitrary number of key-value pairs. NOTE if cmp=null or hash = null
** ajTableNew uses a function suitable for void keys.
**
** @return [AjPTable] new table.
** @category new [AjPTable] Creates a table.
** @@
**
******************************************************************************/

AjPTable ajTablestrNew(void)
{
    return ajTableNewFunctionLen(100, ajTablestrCmp, ajTablestrHash);
}




/* @func ajTablestrNewCase ****************************************************
**
** Creates, initialises, and returns a new, empty table that can hold an
** arbitrary number of key-value pairs. NOTE if cmp=null or hash = null
** ajTableNew uses a function suitable for void keys.
**
** @return [AjPTable] new table.
** @category new [AjPTable] Creates a table.
** @@
**
******************************************************************************/

AjPTable ajTablestrNewCase(void)
{
    return ajTableNewFunctionLen(100, ajTablestrCmpCase, ajTablestrHashCase);
}





/* @func ajTablestrNewCaseLen *************************************************
**
** Creates, initialises, and returns a new, empty table that can hold a
** specified number of key-value pairs.
**
** The table can grow, but will be slower if it more than doubles in size.
**
** @param [r] size [ajuint] estimate of number of unique keys
**
** @return [AjPTable] new table.
** @@
**
******************************************************************************/

AjPTable ajTablestrNewCaseLen(ajuint size)
{
    return ajTableNewFunctionLen(size, ajTablestrCmpCase, ajTablestrHashCase);
}


/* @func ajTablestrNewLen *****************************************************
**
** Creates, initialises, and returns a new, empty table that can hold a
** specified number of key-value pairs.
**
** The table can grow, but will be slower if it more than doubles in size.
**
** @param [r] size [ajuint] estimate of number of unique keys
**
** @return [AjPTable] new table.
** @@
**
******************************************************************************/

AjPTable ajTablestrNewLen(ajuint size)
{
    return ajTableNewFunctionLen(size, ajTablestrCmp, ajTablestrHash);
}




/* @section Retrieval **********************************************************
**
** @fdata [AjPTable]
**
** Retrieves values from a hash table
**
** @nam3rule Fetch Retrieval fuction
**
** @argrule Fetch table [const AjPTable] Hash table
** @argrule Fetch key [const AjPStr] Key
**
** @valrule Fetch [const AjPStr] Value
**
** @fcategory cast
**
******************************************************************************/

/* @func ajTablestrFetch ******************************************************
**
** returns the value associated with key in table, or null
** if table does not hold key.
**
** @param [r] table [const AjPTable] table to search
** @param [r] key   [const AjPStr] key to find.
** @return [const AjPStr]  value associated with key
** @error NULL if key not found in table.
** @@
******************************************************************************/

const AjPStr ajTablestrFetch(const AjPTable table, const AjPStr key)
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

    return p ? (const AjPStr) p->value : NULL;
}

/* @section Modify ************************************************************
**
** @fdata [AjPTable]
**
** Updates values from a hash table
**
** @nam3rule Fetchmod Retrieval fuction
**
** @argrule Fetchmod table [AjPTable] Hash table
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
** @return [ajint] Comparison result. 0 if equal, +1 if x is higher.
**               -1 if x is lower.
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
** @return [ajint] Comparison result. 0 if equal, +1 if x is higher.
**               -1 if x is lower.
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
	{
	    ajUser("key '%S' value '%S'",
		   (const AjPStr) p->key, (AjPStr) p->value);
	}

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
    ajDebug(" timestamp: %u", table->timestamp);

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

/* @datasection [AjPTable] Character hash tables *******************************
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
** @argrule Len size [ajuint] Number of key values
**
** @valrule * [AjPTable] New hash table
**
******************************************************************************/



/* @func ajTablecharNew ***************************************************
**
** Creates a table with a character string key.
**
** @return [AjPTable] New table object with a character string key.
** @@
******************************************************************************/

AjPTable ajTablecharNew(void)
{
    return ajTableNewFunctionLen(100, ajTablecharCmp, ajTablecharHash);
}

/* @func ajTablecharNewCase ***************************************************
**
** Creates a table with a character string key and case insensitive searching.
**
** @return [AjPTable] New table object with a character string key.
** @@
******************************************************************************/

AjPTable ajTablecharNewCase(void)
{
    return ajTableNewFunctionLen(100, ajTablecharCmpCase, ajTablecharHashCase);
}

/* @func ajTablecharNewCaseLen ************************************************
**
** Creates a table with a character string key and case insensitive searching.
**
** @param [r] size [ajuint] Hash size estimate.
** @return [AjPTable] New table object with a character string key.
** @@
******************************************************************************/

AjPTable ajTablecharNewCaseLen(ajuint size)
{
    return ajTableNewFunctionLen(size, ajTablecharCmpCase, ajTablecharHashCase);
}

/* @obsolete ajStrTableNewCaseC
** @rename ajTablecharNewCaseLen
*/

__deprecated AjPTable ajStrTableNewCaseC(ajuint hint)
{
    return ajTablecharNewCaseLen(hint);
}

/* @obsolete ajStrTableNewCase
** @rename ajTablestrNewCaseLen
*/

__deprecated AjPTable ajStrTableNewCase(ajuint hint)
{
    return ajTablestrNewCaseLen(hint);
}


/* @func ajTablecharNewLen ****************************************************
**
** Creates a table with a character string key.
**
** @param [r] size [ajuint] Hash size estimate.
** @return [AjPTable] New table object with a character string key.
** @@
******************************************************************************/

AjPTable ajTablecharNewLen(ajuint size)
{
    return ajTableNewFunctionLen(size, ajTablecharCmp, ajTablecharHash);
}



/* @obsolete ajStrTableNewC
** @rename ajTablecharNewLen
*/

__deprecated AjPTable ajStrTableNewC(ajuint hint)
{
    return ajTablecharNewLen(hint);
}


/* @obsolete ajStrTableNew
** @rename ajTablestrNewLen
*/

__deprecated AjPTable ajStrTableNew(ajuint hint)
{
    return ajTablestrNewLen(hint);
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
** @return [ajint] Comparison result. 0 if equal, +1 if x is higher.
**               -1 if x is lower.
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
** @return [ajint] Comparison result. 0 if equal, +1 if x is higher.
**               -1 if x is lower.
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


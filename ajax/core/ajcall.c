#include "ajax.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/* ajCall Routines are used to allow access to different library levels.
** Originally set up for plplot graphics to be optional.
** So in ajgraph.c you used to see the register calls which list all the calls
** needed by ajacd.c. this is now a higher level library to these are obsolete
**
** ajCall routines are now used for sequence database access methods which
** search for data and package it as files and buffers.
** These methods may in turn need to call relational databases,
** web services and other ways to get data into the system.
*/

static AjPTable callTable = NULL;
static AjPTable oldcallTable = NULL;
static AjPTable oldcallCount = NULL;

static ajint callCmpStr(const void *x, const void *y);
static unsigned callStrHash(const void *key, unsigned hashsize);
static void callItemDel(void** key, void** value, void* cl);
static void callCountDel(void** key, void** value, void* cl);




/* @funcstatic callCmpStr *****************************************************
**
** Compare two words.
**
** @param [r] x [const void *] First word
** @param [r] y [const void *] Second word
** @return [ajint] difference
** @@
******************************************************************************/

static ajint callCmpStr(const void *x, const void *y)
{
    return strcmp((const char *)x, (const char *)y);
}




/* @funcstatic callStrHash ****************************************************
**
** Create hash value from key.
**
** @param [r] key [const void *] key.
** @param [r] hashsize [unsigned] Hash size
** @return [unsigned] hash value
** @@
******************************************************************************/

static unsigned callStrHash(const void *key, unsigned hashsize)
{
    unsigned hashval;
    const char *s;
    ajint j;

    ajint i;

    s = (const char *) key;
    j = strlen(s);

    for(i=0, hashval = 0; i < j; i++, s++)
	hashval = *s + 31 *hashval;

    return hashval % hashsize;
}




/* @func ajCallTableNew *******************************************************
**
** Create new function hash table
**
** @return [AjPTable] Hash table with string keys
** @@
******************************************************************************/

AjPTable ajCallTableNew(void)
{
    return ajTableNewFunctionLen(50, callCmpStr,callStrHash);
}





/* @func ajCallTableRegister **************************************************
**
** Create hash value pair using the name and function.
**
** @param [w] table [AjPTable] name which is used later.
** @param [r] name [const char*] name which is used later.
** @param [f] func [void*] function to be called on name being called.
** @return [void]
** @@
******************************************************************************/

void ajCallTableRegister(AjPTable table, const char *name, void *func)
{
    void *rec;
    char* keyname = NULL;
 
    if(!table)
      ajFatal("ajCallTableRegister called for undefined table with name '%s'",
	      name);

    rec = ajTableFetch(table, name);	/* does it exist already */

    if(!rec)
    {
	keyname = ajCharNewC(name);
	ajTablePut(table, keyname, (void *) func);
    }
    else
    {
        ajWarn("ajCallTableRegister duplicate name '%s'", name);
    }

    return;
}




/* @func ajCallRegister *******************************************************
**
** Create hash value pair using the name and function.
**
** @param [r] name [const char*] name which is used later..
** @param [f] func [CallFunc] function to be called on name being called.
** @return [void]
** @@
******************************************************************************/

void ajCallRegister(const char *name, CallFunc func)
{
    void *rec;
    char* keyname = NULL;

 
    if(!callTable)
	callTable = ajTableNewFunctionLen(50, callCmpStr,callStrHash);

    rec = ajTableFetch(callTable, name);	/* does it exist already */

    if(!rec)
    {
	keyname = ajCharNewC(name);
	ajTablePut(callTable, keyname, (void *) func);
    }

    return;
}




/* @func ajCallRegisterOld *****************************************************
**
** Create hash value pair using an obsolete name and function.
**
** @param [r] name [const char*] name which is used later.
** @param [f] func [CallFunc] function to be called on name being called.
** @return [void]
** @@
******************************************************************************/

void ajCallRegisterOld(const char *name, CallFunc func)
{
    void *rec;
    char* keyname = NULL;
    ajuint *i;
 
    if(!oldcallTable)
    {
	oldcallTable = ajTableNewFunctionLen(50, callCmpStr,callStrHash);
	oldcallCount = ajTableNewFunctionLen(50, callCmpStr,callStrHash);
    }
    
    rec = ajTableFetch(oldcallTable, name);	/* does it exist already */

    if(!rec)
    {
	keyname = ajCharNewC(name);
	ajTablePut(oldcallTable, keyname, (void *) func);
	keyname = ajCharNewC(name);
        AJNEW0(i);
	ajTablePut(oldcallCount, keyname, i);
    }

    return;
}




/* @func ajCall ***************************************************************
**
** Call a function by its name. If it does not exist then give
** an error message saying so.
**
** @param [r] name [const char*] name of the function to call.
** @param [v] [...] Optional arguments
** @return [void*] NULL if function call not found.
** @@
******************************************************************************/
void* ajCall(const char *name, ...)
{
    va_list args;
    CallFunc rec;
    CallFunc recold;
    void *retval = NULL;
    ajuint *icount;

    if(!callTable)
    {
	ajMessCrash("Calls to %s not registered. For graphics devices use "
		    "ajGraphInit in main function first",name);
	return retval;
    }

    rec = (CallFunc) ajTableFetch(callTable, name);
    recold = (CallFunc) ajTableFetch(oldcallTable, name);

    if(rec)
    {
	va_start(args, name);
	retval = (*(rec))(name, args);
	va_end(args);
    }
    else if(recold)
    {
        icount = (ajuint*) ajTableFetch(oldcallCount, name);
        if(!(*icount)++)
            ajWarn("Obsolete graphics call '%s' called via ajCall", name);
	va_start(args, name);
	retval = (*(recold))(name, args);
	va_end(args);
    }
    else
    {
	ajMessCrash("Graphics call %s not found. "
		    "Use ajGraphInit in main function first",name);
    }

    return retval;
}




/* @func ajCallTableGetC ******************************************************
**
** Returns a named function by its name. If it does not exist then give
** an error message saying so.
**
** @param [r] table [const AjPTable] Function hash table
** @param [r] name [const char*] name of the function
** @return [void*] NULL if function call not found.
** @@
******************************************************************************/
void* ajCallTableGetC(const AjPTable table, const char *name)
{
    void *rec;

    if(!table)
    {
        ajMessCrash("ajCallTableGet no call table for '%S'",
		    name);
	return NULL;
    }

    rec = ajTableFetch(table, name);

    if(!rec)
	return NULL;

    return rec;
}




/* @func ajCallTableGetS ******************************************************
**
** Returns a named function by its name. If it does not exist then give
** an error message saying so.
**
** @param [r] table [const AjPTable]  Function hash table
** @param [r] namestr [const AjPStr] name of the function
** @return [void*] NULL if function call not found.
** @@
******************************************************************************/
void* ajCallTableGetS(const AjPTable table, const AjPStr namestr)
{
    return ajCallTableGetC(table, MAJSTRGETPTR(namestr));
}




/* @funcstatic callItemDel ****************************************************
**
** Delete an entry in the call table.
**
** @param [d] key [void**] Standard argument. Table key (character string)
** @param [d] value [void**] Standard argument. Table item (function reference)
** @param [u] cl [void*] Standard argument. Usually NULL.
** @return [void]
** @@
******************************************************************************/

static void callItemDel(void** key, void** value, void* cl)
{
    char *p;

    (void) value;
    (void) cl;

    p = (char*) *key;

    AJFREE(p);
 
    *key = NULL;
    *value = NULL;

    return;
}




/* @funcstatic callCountDel ***************************************************
**
** Delete an entry in the call count table.
**
** @param [d] key [void**] Standard argument. Table key (character string)
** @param [d] value [void**] Standard argument. Table item (unsigned integer)
** @param [u] cl [void*] Standard argument. Usually NULL.
** @return [void]
** @@
******************************************************************************/

static void callCountDel(void** key, void** value, void* cl)
{
    char *p;
    ajuint *cnt;

    (void) cl;

    p = (char*) *key;
    cnt = (ajuint*) *value;

    AJFREE(p);
    AJFREE(cnt);
 
    *key = NULL;

    return;
}

/* @func ajCallTableDel *******************************************************
**
** Destructor for a function table
**
** @param [d] Ptable [AjPTable*] Table to be freed
** @return [void]
******************************************************************************/

void ajCallTableDel(AjPTable* Ptable)
{
    if(!*Ptable) return;

    ajTableMapDel(*Ptable, callItemDel, NULL);

    return;
}


/* @func ajCallExit ***********************************************************
**
** Cleans up calls register internal memory
**
** @return [void]
** @@
******************************************************************************/

void ajCallExit(void)
{
    ajTableMapDel(callTable, callItemDel, NULL);
    ajTableMapDel(oldcallTable, callItemDel, NULL);
    ajTableMapDel(oldcallCount, callCountDel, NULL);

    ajTableFree(&callTable);
    ajTableFree(&oldcallTable);
    ajTableFree(&oldcallCount);

    return;
}

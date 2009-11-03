#include "ajax.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/* ajCall Routines are used to allow access to different graphics packages.
** Okay so at the moment only plplot is used. Also if you want a light weight
** version of EMBOSS no graphics can be stated.
** So in ajgraph.c you will see the register calls which list all the calls
** needed by ajacd.c.
** At the start of ajgraph.c all the calls that call plplot are done first.
** These are the ones that will need to be replaced if the graphics
** are changed.
*/

static AjPTable callTable = NULL;

static ajint callCmpStr(const void *x, const void *y);
static unsigned callStrHash(const void *key, unsigned hashsize);
static void callItemDel(void** key, void** value, void* cl);




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
    void *retval = NULL;

    if(!callTable)
    {
	ajMessCrash("Calls to %s not registered. For graphics devices use "
		    "ajGraphInit in main function first",name);
	return retval;
    }

    rec = (CallFunc) ajTableFetch(callTable, name);

    if(rec)
    {
	va_start(args, name);
	retval = (*(rec))(name, args);
	va_end(args);
    }
    else
	ajMessCrash("Graphics call %s not found. "
		    "Use ajGraphInit in main function first",name);

    return retval;
}


/* @funcstatic callItemDel ****************************************************
**
** Delete an entry in the call table.
**
** @param [d] key [void**] Standard argument. Table key.
** @param [d] value [void**] Standard argument. Table item.
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

    ajTableFree(&callTable);
    return;
}

/* @source ajcall *************************************************************
**
** General routines for function callback
**
** @author Copyright (c) 1999 Alan Bleasby
** @version $Revision: 1.28 $
** @modified $Date: 2011/10/23 20:09:49 $ by $Author: mks $
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
#include "ajcall.h"

#include <stdio.h>
#include <string.h>




/*
** ajCall Routines are used to allow access to different library levels.
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




/* @func ajCallTableNew *******************************************************
**
** Create new function hash table
**
** @return [AjPTable] Hash table with string keys
**
** @release 6.2.0
** @@
******************************************************************************/

AjPTable ajCallTableNew(void)
{
    return ajTablecharNew(50);
}




/* @func ajCallTableRegister **************************************************
**
** Create hash value pair using the name and function.
**
** @param [w] table [AjPTable] name which is used later.
** @param [r] name [const char*] name which is used later.
** @param [f] func [void*] Access structure for function to be invoked by name
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ajCallTableRegister(AjPTable table, const char *name, void *func)
{
    const void *rec;
    char* keyname = NULL;
 
    if(!table)
      ajFatal("ajCallTableRegister called for undefined table with name '%s'",
	      name);

    rec = ajTableFetchC(table, name);	/* does it exist already */

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
**
** @release 4.0.0
** @@
******************************************************************************/

void ajCallRegister(const char *name, CallFunc func)
{
    const void *rec;
    char* keyname = NULL;

 
    if(!callTable)
	callTable = ajTablecharNew(50);

    rec = ajTableFetchC(callTable, name);	/* does it exist already */

    if(!rec)
    {
	keyname = ajCharNewC(name);
	ajTablePut(callTable, keyname, (void *) func);
    }

    return;
}




/* @func ajCallRegisterOld ****************************************************
**
** Create hash value pair using an obsolete name and function.
**
** @param [r] name [const char*] name which is used later.
** @param [f] func [CallFunc] function to be called on name being called.
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ajCallRegisterOld(const char *name, CallFunc func)
{
    const void *rec;
    char* keyname = NULL;
    ajuint *i;
 
    if(!oldcallTable)
    {
	oldcallTable = ajTablecharNew(50);
	oldcallCount = ajTablecharNew(50);
    }
    
    rec = ajTableFetchC(oldcallTable, name);	/* does it exist already */

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
**
** @release 4.0.0
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

    rec = (CallFunc) ajTableFetchmodC(callTable, name);
    recold = (CallFunc) ajTableFetchmodC(oldcallTable, name);

    if(rec)
    {
	va_start(args, name);
	retval = (*(rec))(name, args);
	va_end(args);
    }
    else if(recold)
    {
        icount = (ajuint*) ajTableFetchmodC(oldcallCount, name);
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
** Returns an access structure defining the named function. If it does
** not exist then gives an error message saying so.
**
** @param [r] table [const AjPTable] Function hash table
** @param [r] name [const char*] name of the function
** @return [void*] Access structure defining the named function.
**                 NULL if not found
**
** @release 6.2.0
** @@
******************************************************************************/
void* ajCallTableGetC(const AjPTable table, const char *name)
{
    void *rec;

    if(!table)
    {
        ajMessCrash("ajCallTableGet no call table for '%s'",
		    name);
	return NULL;
    }

    rec = ajTableFetchmodC(table, name);

    if(!rec)
	return NULL;

    return rec;
}




/* @func ajCallTableGetS ******************************************************
**
** Returns an access structure defining the named function. If it does
** not exist then gives an error message saying so.
**
** @param [r] table [const AjPTable]  Function hash table
** @param [r] namestr [const AjPStr] name of the function
** @return [void*] Access structure defining the named function.
**                 NULL if not found
**
** @release 6.2.0
** @@
******************************************************************************/
void* ajCallTableGetS(const AjPTable table, const AjPStr namestr)
{
    void *rec;

    if(!table)
    {
        ajMessCrash("ajCallTableGet no call table for '%S'",
		    namestr);
	return NULL;
    }

    rec = ajTableFetchmodS(table, namestr);

    if(!rec)
	return NULL;

    return rec;
}




/* @func ajCallExit ***********************************************************
**
** Cleans up calls register internal memory
**
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajCallExit(void)
{
    ajTableDel(&callTable);
    ajTableDel(&oldcallTable);
    ajTableDel(&oldcallCount);

    return;
}

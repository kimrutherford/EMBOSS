/******************************************************************************
** @source AJAX utility functions
**
** @author Copyright (C) 1998 Ian Longden
** @author Copyright (C) 1998 Peter Rice
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

#include "ajax.h"
#include <stdarg.h>
#ifdef WIN32
#include "win32.h"
#include <winsock2.h>
#include <lmcons.h> /* for UNLEN */
#else
#include <pwd.h>
#include <unistd.h>
#endif



static AjBool utilBigendian;
static ajint utilBigendCalled = 0;




/* @filesection ajutil *******************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
*/

/* @datasection [none] Exit functions ****************************************
**
** @nam2rule Exit Functions for exiting cleanly
**
*/

/* @section exit  *************************************************************
**
** Functions for exiting cleanly.
**
** @fdata [none]
**
** @nam3rule  Abort  Exits without flushing any files. 
** @nam3rule  Bad    Calls 'exit' with an unsuccessful code (EXIT_FAILURE
**                   defined in stdlib.h).
** 
** @valrule   *  [void] All functions do not return
** 
** @fcategory misc
**
******************************************************************************/

/* @func ajExit ***************************************************************
**
** Calls 'exit' with a successful code (zero), but first calls ajReset to
** call memory clean up and debug reporting functions.
**
** No cleanup or reporting routines are called. Simply crashes.
**
** @return [void]
** @@
******************************************************************************/

__noreturn void ajExit(void)
{
    ajReset();
    exit(0);
}




/* @func ajExitAbort **********************************************************
**
** Exits without flushing any files. Needed for exit from, for example,
** a failed system call (ajFileNewInPipe, and so on) where the parent
** process has open output files, and the child process needs to exit
** without affecting them. Failure to exit this way can mean the output
** buffer is flushed twice.
**
** Calls '_exit' with an unsuccessful code (EXIT_FAILURE defined in stdlib.h).
**
** No cleanup or reporting routines are called. Simply crashes.
**
** @return [void]
** @@
******************************************************************************/

__noreturn void  ajExitAbort(void)
{
    _exit(EXIT_FAILURE);
}




/* @func ajExitBad ************************************************************
**
** Calls 'exit' with an unsuccessful code (EXIT_FAILURE defined in stdlib.h).
**
** No cleanup or reporting routines are called. Simply crashes.
**
** @return [void]
** @@
******************************************************************************/

__noreturn void  ajExitBad(void)
{
    exit(EXIT_FAILURE);
}




/* @datasection [none] Memory cleanup functions ********************************
**
** @nam2rule  Reset  Resets internal memory and returns.
**
*/

/* @section reset **************************************************************
**
** Functions for memory cleanup
**
** @fdata [none]
**
** 
** @valrule   *  [void] No return value
** 
** @fcategory misc
**
******************************************************************************/

/* @func ajReset **************************************************************
**
** Cleans up all internal memory by calling cleanup routines which
** can report on resource usage etc.
**
** Intended to be called at the end of processing by exit functions.
**
** @return [void]
** @@
******************************************************************************/

void ajReset(void)
{
#ifdef WIN32
    WSACleanup();
#endif
    ajDebug("\nFinal Summary\n=============\n\n");
    ajUtilLoginfo();
    /*    ajBtreeExit(); */
    ajTreeExit();
    ajPdbExit();
    ajDmxExit();
    ajDomainExit();
    ajFeatExit();
    ajSeqExit();
    ajPhyloExit();
    ajAlignExit();
    ajReportExit();
    ajAcdExit(ajFalse);
    ajNamExit();
    ajSysExit();
    ajCallExit();
    ajBaseExit();
    ajCodExit();
    ajTrnExit();
    ajMeltExit();
    ajTimeExit();
    ajRegExit();
    ajArrExit();
    ajDatafileExit();
    ajFileExit();
    ajListExit();
    ajTableExit();
    ajStrExit();
    ajMemExit();
    ajMessExit();     /* clears data for ajDebug - do this last!!!  */

    return;
}




/* @datasection [none] Byte manipulation functions ****************************
**
** @nam2rule  Byte          Manipulate a byte of data.
**
*/



/* @section byte manipulation functions  **************************************
**
** Functions for manipulating bytes.
**
** @fdata [none]
**
** @nam3rule  Rev       Reverse the byte order.
** @nam4rule  RevLen2      Reverse the byte order in a 2 byte integer.
** @nam4rule  RevLen4      Reverse the byte order in a 4 byte integer.
** @nam4rule  RevLen8      Reverse the byte order in a 8 byte integer.
** @nam4rule  RevInt    Reverse the byte order in an integer.
** @nam4rule  RevShort  Reverse the byte order in a short integer.
** @nam4rule  RevLong   Reverse the byte order in a long.
** @nam4rule  RevUint    Reverse the byte order in an unsigned integer.
**
** @argrule   RevLen2  sval [short*] Short to be reversed
** @argrule   RevLen4  ival [ajint*] Integer to be reversed
** @argrule   RevLen8  lval [ajlong*] Long integer to be reversed
** @argrule   RevShort sval [short*] Short to be reversed
** @argrule   RevInt   ival [ajint*] Integer to be reversed
** @argrule   RevLong  lval [ajlong*] Long integer to be reversed
** @argrule   RevUint  ival [ajuint*] Unsigned integer to be reversed
** 
** @valrule   *  [void]
** 
** @fcategory misc
**
******************************************************************************/


/* @func ajByteRevInt *********************************************************
**
** Reverses the byte order in an integer.
**
** @param [u] ival [ajint*] Integer in wrong byte order.
**                        Returned in correct order.
** @return [void]
** @@
******************************************************************************/

void ajByteRevInt(ajint* ival)
{
    union lbytes
    {
	char chars[8];
	ajint i;
    } data, revdata;

    char* cs;
    char* cd;
    ajuint i;

    data.i = *ival;
    cs     = data.chars;
    cd     = &revdata.chars[sizeof(ajint)-1];

    for(i=0; i < sizeof(ajint); i++)
    {
	*cd = *cs++;
	--cd;
    }

    *ival = revdata.i;

    return;
}




/* @obsolete ajUtilRevInt
** @rename ajByteRevInt
*/

__deprecated void ajUtilRevInt(ajint* ival)
{
    ajByteRevInt(ival);

    return;
}




/* @func ajByteRevLen2 ********************************************************
**
** Reverses the byte order in a 2 byte integer.
**
** Intended for cases where the number of bytes is known, for
** example when reading a binary file.
**
** @param [u] sval [short*] Short integer in wrong byte order.
**                          Returned in correct order.
** @return [void]
** @@
******************************************************************************/

void ajByteRevLen2(short* sval)
{
    union lbytes
    {
	char chars[2];
	short s;
    } data, revdata;

    char* cs;
    char* cd;
    ajint i;

    data.s = *sval;
    cs     = data.chars;
    cd     = &revdata.chars[1];

    for(i=0; i < 2; i++)
    {
	*cd = *cs++;
	--cd;
    }

    *sval = revdata.s;

    return;
}




/* @obsolete ajUtilRev2
** @rename ajByteRev2
*/

__deprecated void ajUtilRev2(short* sval)
{
    ajByteRevLen2(sval);

    return;
}




/* @func ajByteRevLen4 ********************************************************
**
** Reverses the byte order in a 4 byte integer.
**
** Intended for cases where the number of bytes is known, for
** example when reading a binary file.
**
** @param [u] ival [ajint*] Integer in wrong byte order.
**                        Returned in correct order.
** @return [void]
** @@
******************************************************************************/

void ajByteRevLen4(ajint* ival)
{
    union lbytes
    {
	char chars[4];
	ajint i;
    } data, revdata;

    char* cs;
    char* cd;
    ajint i;

    data.i = *ival;
    cs     = data.chars;
    cd     = &revdata.chars[3];

    for(i=0; i < 4; i++)
    {
	*cd = *cs++;
	--cd;
    }

    *ival = revdata.i;

    return;
}




/* @obsolete ajUtilRev4
** @rename ajByteRevLen4
*/

__deprecated void ajUtilRev4(ajint* ival)
{
    ajByteRevLen4(ival);

    return;
}




/* @func ajByteRevLen8 ********************************************************
**
** Reverses the byte order in an 8 byte long.
**
** Intended for cases where the number of bytes is known, for
** example when reading a binary file.
**
** @param [u] lval [ajlong*] Integer in wrong byte order.
**                           Returned in correct order.
** @return [void]
** @@
******************************************************************************/

void ajByteRevLen8(ajlong* lval)
{
    union lbytes
    {
	char chars[8];
	ajlong l;
    } data, revdata;

    char* cs;
    char* cd;
    ajint i;
    
    data.l = *lval;
    cs     = data.chars;
    cd     = &revdata.chars[7];

    for(i=0; i < 8; i++)
    {
	*cd = *cs++;
	--cd;
    }
    
    *lval = revdata.l;
    
    return;
}




/* @obsolete ajUtilRev8
** @rename ajByteRevLen8
*/

__deprecated void ajUtilRev8(ajlong* lval)
{
    ajByteRevLen8(lval);
}




/* @func ajByteRevLong *******************************************************
**
** Reverses the byte order in a long.
**
** @param [u] lval [ajlong*] Integer in wrong byte order.
**                           Returned in correct order.
** @return [void]
** @@
******************************************************************************/

void ajByteRevLong(ajlong* lval)
{
    union lbytes
    {
	char chars[8];
	ajlong l;
    } data, revdata;

    char* cs;
    char* cd;
    ajuint i;
    
    data.l = *lval;
    cs     = data.chars;
    cd     = &revdata.chars[sizeof(ajlong)-1];

    for(i=0; i < sizeof(ajlong); i++)
    {
	*cd = *cs++;
	--cd;
    }
    
    *lval = revdata.l;
    
    return;
}




/* @obsolete ajUtilRevLong
** @rename ajByteRevLong
*/

__deprecated void ajUtilRevLong(ajlong* lval)
{
    ajByteRevLong(lval);

    return;
}




/* @func ajByteRevShort ******************************************************
**
** Reverses the byte order in a short integer.
**
** @param [u] sval [short*] Short integer in wrong byte order.
**                          Returned in correct order.
** @return [void]
** @@
******************************************************************************/

void ajByteRevShort(short* sval)
{
    union lbytes
    {
	char chars[8];
	short s;
    } data, revdata;

    char* cs;
    char* cd;
    ajuint i;

    data.s = *sval;
    cs     = data.chars;
    cd     = &revdata.chars[sizeof(short)-1];

    for(i=0; i < sizeof(short); i++)
    {
	*cd = *cs++;
	--cd;
    }

    *sval = revdata.s;

    return;
}




/* @obsolete ajUtilRevShort
** @rename ajByteRevShort
*/

__deprecated void ajUtilRevShort(short* sval)
{
    ajByteRevShort(sval);
}




/* @func ajByteRevUint ********************************************************
**
** Reverses the byte order in an unsigned integer.
**
** @param [u] ival [ajuint*] Unsigned integer in wrong byte order.
**                        Returned in correct order.
** @return [void]
** @@
******************************************************************************/

void ajByteRevUint(ajuint* ival)
{
    union lbytes
    {
	char chars[8];
	ajuint i;
    } data, revdata;

    char* cs;
    char* cd;
    ajuint i;

    data.i = *ival;
    cs     = data.chars;
    cd     = &revdata.chars[sizeof(ajuint)-1];

    for(i=0; i < sizeof(ajuint); i++)
    {
	*cd = *cs++;
	--cd;
    }

    *ival = revdata.i;

    return;
}




/* @obsolete ajUtilRevUint
** @rename ajByteRevUint
*/

__deprecated void ajUtilRevUint(ajuint* ival)
{
    ajByteRevUint(ival);
}




/* @datasection [none]  Miscellaneous utility functions ***********************
**
** Miscellaneous utility functions.
**
**
** @nam2rule  Util           Miscellaneous utility functions.
*/




/* @section Miscellaneous utility functions ***********************************
**
** Miscellaneous utility functions.
**
** @fdata [none]
**
** @nam3rule  Catch      Dummy function to be called in special cases so 
**                       it can be used when debugging in GDB.
** @nam3rule  Get        Retrieve system information
** @nam4rule  GetBigendian  Tests whether the host system uses big endian 
**                          byte order.
** @nam4rule  GetUid        Returns the user's userid.
** 
** @nam3rule  Loginfo    If a log file is in use, writes run details to 
**                       end of file.
** @argrule   GetUid  Puid [AjPStr*] User's userid
** 
** @valrule   *  [void] 
** @valrule   *Get  [AjBool] True if operation was successful.
** 
** @fcategory misc
**
******************************************************************************/


/* @func ajUtilCatch **********************************************************
**
** Dummy function to be called in special cases so it can be used when
** debugging in GDB.
**
** To use, simply put a call to ajUtilCatch() into your code, and use
** "break ajUtilCatch" in gdb to get a traceback.
**
** @return [void]
** @@
******************************************************************************/

void ajUtilCatch(void)
{
    static ajint calls = 0;

    calls = calls + 1;

    return;
}




/* @func ajUtilGetBigendian ***************************************************
**
** Tests whether the host system uses big endian byte order.
**
** @return [AjBool] ajTrue if host is big endian.
** @@
******************************************************************************/

AjBool ajUtilGetBigendian(void)
{
    static union lbytes
    {
	char chars[sizeof(ajint)];
	ajint i;
    } data;

    if(!utilBigendCalled)
    {
	utilBigendCalled = 1;
	data.i           = 0;
	data.chars[0]    = '\1';

	if(data.i == 1)
	    utilBigendian = ajFalse;
	else
	    utilBigendian = ajTrue;
    }

    return utilBigendian;
}




/* @obsolete ajUtilBigendian
** @rename ajUtilGetBigendian
*/
__deprecated AjBool ajUtilBigendian(void)
{
    return ajUtilGetBigendian();
}




/* @func ajUtilGetUid *********************************************************
**
** Returns the user's userid
**
** @param [w] Puid [AjPStr*] String to return result
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajUtilGetUid(AjPStr* Puid)
{
#ifndef WIN32
    ajint uid;
    struct passwd* pwd;

    ajDebug("ajUtilUid\n");

    uid = getuid();

    if(!uid)
    {
	ajStrAssignClear(Puid);

	return ajFalse;
    }

    ajDebug("  uid: %d\n", uid);
    pwd = getpwuid(uid);

    if(!pwd)
    {
	ajStrAssignClear(Puid);

	return ajFalse;
    }

    ajDebug("  pwd: '%s'\n", pwd->pw_name);

    ajStrAssignC(Puid, pwd->pw_name);

    return ajTrue;

#else	/* WIN32 */
    char nameBuf[UNLEN+1];
    DWORD nameLen = UNLEN+1;

    ajDebug("ajUtilUid\n");

    if (GetUserName(nameBuf, &nameLen))
    {
	ajDebug("  pwd: '%s'\n", nameBuf);
	ajStrAssignC(Puid, nameBuf);

	return ajTrue;
    }

    ajStrAssignC(Puid, "");

    return ajFalse;
#endif
}




/* @obsolete ajUtilUid
** @rename ajUtilGetUid
*/

__deprecated AjBool ajUtilUid(AjPStr* dest)
{
    return ajUtilGetUid(dest);
}




/* @func ajUtilLoginfo ********************************************************
**
** If a log file is in use, writes run details to end of file.
**
** @return [void]
** @@
******************************************************************************/

void ajUtilLoginfo(void)
{
    AjPFile logf;
    AjPStr logfname = NULL;
    AjPStr uids    = NULL;
    AjPTime today = NULL;
    double walltime;
    double cputime;

    today = ajTimeNewTodayFmt("log");

    if(ajNamGetValueC("logfile", &logfname))
    {
	logf = ajFileNewOutappendNameS(logfname);

	if(!logf)
	    return;

        walltime = ajTimeDiff(ajTimeRefToday(),today);
        cputime = ajClockSeconds();

        if(walltime < cputime)
            walltime = cputime; /* avoid reporting 0.0 if cpu time appears */

	ajUtilGetUid(&uids),
	ajFmtPrintF(logf, "%S\t%S\t%D\t%.1f\t%.1f\n",
		    ajAcdGetProgram(),
		    uids,
		    today,
                    cputime, walltime);
	ajStrDel(&uids);
	ajStrDel(&logfname);
	ajFileClose(&logf);
    }


    ajTimeDel(&today);

    return;
}




/* @obsolete ajLogInfo
** @rename ajUtilLoginfo
*/

__deprecated void ajLogInfo(void)
{
    ajUtilLoginfo();

    return;
}

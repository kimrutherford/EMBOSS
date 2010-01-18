/******************************************************************************
** @source AJAX message functions
**
** @author Richard Durbin and Ed Griffiths from ACEdb (messubs.c)
** @version 1.0
** @modified Ian Longden for EMBOSS
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

#include <string.h>

#include "ajax.h"
#include <errno.h>
#include <limits.h>


#ifdef __CYGWIN__
#define fopen(a,b) ajSysFuncFopen(a,b)
#endif


/* next six moved from acd for library splitting */

AjBool acdDebugSet    = 0;
AjBool acdDebugBuffer = 0;
AjBool acdDebug       = 0;
AjPStr acdProgram     = NULL;
AjPStr acdArgSave     = NULL;
AjPStr acdInputSave   = NULL;



AjOError AjErrorLevel =
{
    AJTRUE, AJTRUE, AJTRUE, AJTRUE
};

/***************************************************/



AjPTable messErrorTable = 0;

static ajint errorCount = 0;

static char *messErrorFile;

static AjBool messDebug      = 0;
static AjPFile messDebugFile = NULL;
static AjPFile messDebugTestFile = NULL;
static AjPStr messDebugName  = NULL;
static char* messErrMess = NULL;
static AjBool messDebugTestInit = AJFALSE;

static char* messGetFilename(const char *path);
static void messTableDelete(AjPTable* table);

static AjPTable messDebugTestTable = NULL;


/*============================================================================
**======================== Macros ============================================
=============================================================================*/




/* @macro ajFatal *************************************************************
**
** Fatal error message to standard error.
** Includes filename and line number in the source code that invokes it.
** Newline is added automatically at the end of the format string.
**
** @param [r] format [const char*] Format
** @param [v] [...] Variable length argument list
** @return [void]
** @@
******************************************************************************/

/* @macro ajMessCrash *********************************************************
**
** Crash error message to standard error.
** Includes filename and line number in the source code that invokes it.
** Newline is added automatically at the end of the format string.
**
** @param [r] format [const char*] Format
** @param [v] [...] Variable length argument list
** @return [void]
** @@
******************************************************************************/




/* @func ajMessInvokeDebugger *************************************************
**
** Used to trace in a debugger as a breakpoint
**
** @return [void]
** @@
******************************************************************************/

void ajMessInvokeDebugger(void)
{
    static AjBool reentrant = AJFALSE;

    if(!reentrant)
    {
	reentrant = AJTRUE;
	reentrant = AJFALSE;
    }

    return;
}




/******************************************************************************
** Constraints on message buffer size.
**
** BUFSIZE:  size of message buffers (messbuf, a global buffer for general
**           message stuff and a private one in ajMessDump).
** PREFIX:   length of message prefix (used to report details such as the
**           file/line info. for where the error occurred.
** MAINTEXT: space left in buffer is the rest after the prefix and string
**           terminator (NULL) are subtracted.
******************************************************************************/

enum {BUFSIZE = 32768,
      PREFIXSIZE = 1024,
      MAINTEXTSIZE = BUFSIZE - PREFIXSIZE - 1};

/******************************************************************************
** This buffer is used by just about all of the below routines and has the
** obvious problem that strings can run over the end of it, we can only
** detect this after the event with vsprintf.
******************************************************************************/

static char messbuf[BUFSIZE];

/******************************************************************************
** Format strings using va_xx calls.
** Arguments to the macro must have the following types:
**   FORMAT_ARGS:   va_list used to get the variable argument list.
**        FORMAT:   char *  to a string containing the printf format string.
**    TARGET_PTR:   char *  the formatted string will be returned in this
**                          string pointer, N.B. do not put &TARGET_PTR
**        PREFIX:   char *  to a string to be used as a prefix to the rest
**                          of the string, or NULL.
******************************************************************************/

#define AJAXFORMATSTRING(FORMAT_ARGS, FORMAT, TARGET_PTR, PREFIX)    \
va_start(FORMAT_ARGS, FORMAT);                                       \
TARGET_PTR = messFormat(FORMAT_ARGS, FORMAT, PREFIX);                \
va_end(FORMAT_ARGS);

#define AJAXVFORMATSTRING(FORMAT_ARGS, FORMAT, TARGET_PTR, PREFIX)   \
TARGET_PTR = messFormat(FORMAT_ARGS, FORMAT, PREFIX);

static char *messFormat(va_list args, const char *format, const char *prefix);
static void messDump(const char *message);

/* Some standard defines for titles/text for messages:                       */
/*                                                                           */

#define MESG_TITLE "EMBOSS"
#define ERROR_PREFIX "Error: "
#define WARNING_PREFIX "Warning: "
#define EXIT_PREFIX "   An error spotted (non-EMBOSS): "
#define DIE_PREFIX "Died: "
#define CRASH_PREFIX_FORMAT "\n   %s An error in %s at line %d:\n"
#define FULL_CRASH_PREFIX_FORMAT "\n   %s Program cannot continue " \
                                 "(%s, in file %s, at line %d):\n"
#define SYSERR_FORMAT "Something wrong with a system call (%d - %s)"
#define SYSERR_OK "Successful system call (%d - %s)"

/******************************************************************************
** ajMessCrash now reports the file/line no. where ajMessCrash was issued
** as an aid to debugging. We do this using a static structure which holds
** the information and a macro version of ajMessCrash (see regular.h), the
** structure elements are retrieved using access functions.
******************************************************************************/

/* @datastatic MessPErrorInfo *************************************************
**
** Message error information
**
** @alias MessSErrorInfo
** @alias MessOErrorInfo
**
** @attr progname [char*] Name of executable reporting error
** @attr filename [char*] Filename where error was reported
** @attr line_num [ajint] line number of file where error was reported.
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct MessSErrorInfo
{
    char* progname;
    char* filename;
    ajint line_num;
    char Padding[4];
} MessOErrorInfo;

#define MessPErrorInfo MessOErrorInfo*

static MessOErrorInfo messageG = {NULL, NULL, 0, ""};

static ajint messGetErrorLine(void);
static char *messGetErrorFile(void);
static char *messGetErrorProgram(void);

/***************************************************************/
/********* call backs and functions to register them ***********/

static AjMessVoidRoutine beepRoutine    = 0;
static AjMessOutRoutine	 outRoutine     = 0;
static AjMessOutRoutine	 dumpRoutine    = 0;
static AjMessOutRoutine	 errorRoutine   = 0;
static AjMessOutRoutine	 exitRoutine    = 0;
static AjMessOutRoutine	 crashRoutine   = 0;
static AjMessOutRoutine	 warningRoutine = 0;




/* @func ajMessRegBeep ********************************************************
**
** Sets a function to process beeps
**
** @param [f] func [AjMessVoidRoutine] Function to be registered
** @return [AjMessVoidRoutine] Previously defined function
** @@
******************************************************************************/

AjMessVoidRoutine ajMessRegBeep(AjMessVoidRoutine func)
{
    AjMessVoidRoutine old;

    old = beepRoutine;
    beepRoutine = func;

    return old;
}




/* @func ajMessRegOut *********************************************************
**
** Sets a function to write messages
**
** @param [f] func [AjMessOutRoutine] Function to be registered
** @return [AjMessOutRoutine] Previously defined function
** @@
******************************************************************************/

AjMessOutRoutine ajMessRegOut(AjMessOutRoutine func)
{
    AjMessOutRoutine old;

    old = outRoutine;
    outRoutine = func;

    return old;
}




/* @func ajMessRegDump ********************************************************
**
** Sets a function to dump data
**
** @param [f] func [AjMessOutRoutine] Function to be registered
** @return [AjMessOutRoutine] Previously defined function
** @@
******************************************************************************/

AjMessOutRoutine ajMessRegDump(AjMessOutRoutine func)
{
    AjMessOutRoutine old;

    old = dumpRoutine;
    dumpRoutine = func;

    return old;
}




/* @func ajMessRegErr *********************************************************
**
** Sets a function to report errors
**
** @param [f] func [AjMessOutRoutine] Function to be registered
** @return [AjMessOutRoutine] Previously defined function
** @@
******************************************************************************/

AjMessOutRoutine ajMessRegErr(AjMessOutRoutine func)
{
    AjMessOutRoutine old;

    old = errorRoutine;
    errorRoutine = func;

    return old;
}




/* @func ajMessRegExit ********************************************************
**
** Sets a function to exit
**
** @param [f] func [AjMessOutRoutine] Function to be registered
** @return [AjMessOutRoutine] Previously defined function
** @@
******************************************************************************/

AjMessOutRoutine ajMessRegExit(AjMessOutRoutine func)
{
    AjMessOutRoutine old;

    old = exitRoutine;
    exitRoutine = func;

    return old;
}




/* @func ajMessRegCrash *******************************************************
**
** Sets a function to crash
**
** @param [f] func [AjMessOutRoutine] Function to be registered
** @return [AjMessOutRoutine] Previously defined function
** @@
******************************************************************************/

AjMessOutRoutine ajMessRegCrash(AjMessOutRoutine func)
{
    AjMessOutRoutine old;

    old = crashRoutine;
    crashRoutine = func;

    return old;
}




/* @func ajMessRegWarn ********************************************************
**
** Sets a function to print warnings
**
** @param [f] func [AjMessOutRoutine] Function to be registered
** @return [AjMessOutRoutine] Previously defined function
** @@
******************************************************************************/

AjMessOutRoutine ajMessRegWarn(AjMessOutRoutine func)
{
    AjMessOutRoutine old;

    old = warningRoutine;
    warningRoutine = func;

    return old;
}




/* @func ajMessBeep ***********************************************************
**
** Calls the defined beep function, if any. Otherwise prints ASCII 7 to
** standard error.
**
** @return [void]
** @@
******************************************************************************/

void ajMessBeep(void)
{
    if(beepRoutine)
	(*beepRoutine)();
    else
    {
	printf("%c",0x07);
	fflush(stdout);
    }

    return;
}




/* @func ajUser ***************************************************************
**
** Formats a message. Calls the defined output function (if any).
** Otherwise prints the message to standard error with an extra newline.
**
** @param [r] format [const char*] Format string
** @param [v] [...] Variable length argument list
** @return [void]
** @@
******************************************************************************/

void ajUser(const char *format,...)
{
    va_list args;
    const char *mesg_buf;

    AJAXFORMATSTRING(args, format, mesg_buf, NULL);

    if(outRoutine)
	(*outRoutine)(mesg_buf);
    else
	fprintf(stderr, "%s\n", mesg_buf);

    return;
}




/* @func ajUserDumpC ***********************************************************
**
** Prints a string unchanged. Calls the defined output function (if any).
** Otherwise prints the message to standard error with an extra newline.
**
** @param [r] txt [const char*] String to print unchanged
** @return [void]
** @@
******************************************************************************/

void ajUserDumpC(const char* txt)
{
    if(outRoutine)
	(*outRoutine)(txt);
    else
	fprintf(stderr, "%s\n", txt);

    return;
}




/* @func ajUserDumpS ***********************************************************
**
** Prints a string unchanged. Calls the defined output function (if any).
** Otherwise prints the message to standard error with an extra newline.
**
** @param [r] str [const AjPStr] String to print unchanged
** @return [void]
** @@
******************************************************************************/

void ajUserDumpS(const AjPStr str)
{
    const char *mesg_buf = ajStrGetPtr(str);

    if(outRoutine)
	(*outRoutine)(mesg_buf);
    else
	fprintf(stderr, "%s\n", mesg_buf);

    return;
}




/* @func ajMessOut ************************************************************
**
** Formats a message. Calls the defined output function (if any).
** Otherwise prints the message to standard error with no newline.
**
** @param [r] format [const char*] Format string
** @param [v] [...] Variable length argument list
** @return [void]
** @@
******************************************************************************/

void ajMessOut(const char *format,...)
{
    va_list args;
    char *mesg_buf;

    AJAXFORMATSTRING(args, format, mesg_buf, NULL);

    if(outRoutine)
	(*outRoutine)(mesg_buf);
    else
	fprintf(stderr, "%s", mesg_buf);

    return;
}




/* @func ajVUser **************************************************************
**
** Formats a message. Calls the defined output function (if any).
** Otherwise prints the message to standard error.
**
** @param [r] format [const char*] Format string
** @param [v] args [va_list] Variable length argument list
** @return [void]
** @@
******************************************************************************/

void ajVUser(const char *format, va_list args)
{
    char *mesg_buf;

    AJAXVFORMATSTRING(args, format, mesg_buf, NULL);

    if(outRoutine)
	(*outRoutine)(mesg_buf);
    else
	fprintf(stderr, "%s\n", mesg_buf);

    return;
}




/* @func ajMessDump ***********************************************************
**
** Formats a message. Calls the dump function (if any).
** Otherwise no further action.
**
** @param [r] format [const char*] format string.
** @param [v] [...] Variable length argument list.
** @return [void]
** @@
******************************************************************************/

void ajMessDump(const char *format,...)
{
    static char dumpbuf[BUFSIZE];   /* BEWARE limited buffer size. */
    char *mesg_buf;
    va_list args;

    mesg_buf = &dumpbuf[0];

    AJAXFORMATSTRING(args, format, mesg_buf, NULL);

    strcat(mesg_buf, "\n"); /* assume we are writing to a file */

    if(dumpRoutine)
	(*dumpRoutine)(mesg_buf);

    return;
}




/* @funcstatic messDump *******************************************************
**
** Calls the dump function (if any) to dump text followed by a newline.
**
** @param [r] message [const char*] Message text
** @return [void]
** @@
******************************************************************************/

static void messDump(const char *message)
{
    if(dumpRoutine)
    {
	(*dumpRoutine)(message);
	(*dumpRoutine)("\n");
    }

    return;
}




/* @func ajMessGetCountError ***************************************************
**
** Returns the number of times the error routines have been called.
**
** @return [ajint] Error function call count.
** @@
******************************************************************************/

ajint ajMessGetCountError(void)
{
    return errorCount;
}




/* @obsolete ajMessErrorCount
** @rename ajMessGetCountError
*/

__deprecated ajint ajMessErrorCount(void)
{
    return errorCount;
}




/* @func ajErr ****************************************************************
**
** Formats an error message. Calls the error function (if any).
** Otherwise prints the message to standard error with a trailing newline.
**
** The error message count is incremented by 1 for each call.
**
** @param [r] format [const char*] Format
** @param [v] [...] Variable length argument list
** @return [void]
** @@
******************************************************************************/

void ajErr(const char *format, ...)
{
    const char *prefix   = ERROR_PREFIX;
    const char *mesg_buf = NULL;
    va_list args;

    ++errorCount;

    if(AjErrorLevel.error)
    {
	AJAXFORMATSTRING(args, format, mesg_buf, prefix);

	messDump(mesg_buf);

	if(errorRoutine)
	    (*errorRoutine)(mesg_buf);
	else
	    fprintf(stderr, "%s\n", mesg_buf);

	ajMessInvokeDebugger();
        ajUtilCatch();
    }

    return;
}




/* @func ajVErr ***************************************************************
**
** Formats an error message. Calls the error function (if any).
** Otherwise prints the message to standard error with a trailing newline.
**
** The error message count is incremented by 1 for each call.
**
** @param [r] format [const char*] Format
** @param [v] args [va_list] Variable length argument list
** @return [void]
** @@
******************************************************************************/

void ajVErr(const char *format, va_list args)
{
    const char *prefix   = ERROR_PREFIX;
    const char *mesg_buf = NULL;

    ++errorCount;

    AJAXVFORMATSTRING(args, format, mesg_buf, prefix);

    messDump(mesg_buf);

    if(errorRoutine)
	(*errorRoutine)(mesg_buf);
    else
    {
	if(AjErrorLevel.error)
	    fprintf(stderr, "%s\n", mesg_buf);
    }
    ajMessInvokeDebugger();

    return;
}




/* @func ajDie ****************************************************************
**
** Formats an error message. Calls the error function (if any).
** Otherwise prints the message to standard error with a trailing newline.
** Then kills the application.
**
** The error message count is incremented by 1 for each call.
**
** @param [r] format [const char*] Format
** @param [v] [...] Variable length argument list
** @return [void]
** @@
******************************************************************************/

__noreturn void  ajDie(const char *format, ...)
{
    const char *prefix   = DIE_PREFIX;
    const char *mesg_buf = NULL;
    va_list args;

    ++errorCount;

    if(AjErrorLevel.die)
    {
	AJAXFORMATSTRING(args, format, mesg_buf, prefix);

	messDump(mesg_buf);

	if(errorRoutine)
	    (*errorRoutine)(mesg_buf);
	else
	    fprintf(stderr, "%s\n", mesg_buf);

	ajMessInvokeDebugger();
    }


    exit(EXIT_FAILURE);
}




/* @func ajVDie ***********************************************************
**
** Formats an error message. Calls the error function (if any).
** Otherwise prints the message to standard error with a trailing newline.
** Then kills the application.
**
** The error message count is incremented by 1 for each call.
**
** @param [r] format [const char*] Format
** @param [v] args [va_list] Variable length argument list
** @return [void]
** @@
******************************************************************************/

void ajVDie(const char *format, va_list args)
{
    const char *prefix   = DIE_PREFIX;
    const char *mesg_buf = NULL;

    ++errorCount;

    AJAXVFORMATSTRING(args, format, mesg_buf, prefix);

    messDump(mesg_buf);

    if(errorRoutine)
	(*errorRoutine)(mesg_buf);
    else
	ajMessCrash(mesg_buf);

    ajMessInvokeDebugger();

    return;
}




/* @func ajWarn ***************************************************************
**
** Formats a warning message. Calls the warning function (if any).
** Otherwise prints the message to standard error with a trailing newline.
**
** @param [r] format [const char*] Format
** @param [v] [...] Variable length argument list
** @return [void]
** @@
******************************************************************************/

void ajWarn(const char *format, ...)
{
    const char *prefix   = WARNING_PREFIX;
    const char *mesg_buf = NULL;
    va_list args;

    if(AjErrorLevel.warning)
    {
	AJAXFORMATSTRING(args, format, mesg_buf, prefix);

	messDump(mesg_buf);

	if(warningRoutine)
	    (*warningRoutine)(mesg_buf);
	else
	    fprintf(stderr, "%s\n", mesg_buf);

	ajMessInvokeDebugger();
    }

    return;
}




/* @func ajVWarn **************************************************************
**
** Formats a warning message. Calls the warning function (if any).
** Otherwise prints the message to standard error with a trailing newline.
**
** @param [r] format [const char*] Format
** @param [v] args [va_list] Variable length argument list
** @return [void]
** @@
******************************************************************************/

void ajVWarn(const char *format, va_list args)
{
    const char *prefix   = WARNING_PREFIX;
    const char *mesg_buf = NULL;

    AJAXVFORMATSTRING(args, format, mesg_buf, prefix);

    messDump(mesg_buf);

    if(warningRoutine)
	(*warningRoutine)(mesg_buf);
    else
	fprintf(stderr, "%s\n", mesg_buf);

    ajMessInvokeDebugger();

    return;
}




/* @func ajMessExitmsg ********************************************************
**
** Formats an exit message and calls the exit function (if any).
** Otherwise prints the message to standard error with a trailing newline
** and exist with code EXIT_FAILURE.
**
** Use this function for errors that while being unrecoverable are not a
** problem with the AJAX code.
**
** Note that there errors are logged but that this routine will exit without
** any chance to interrupt it (see the crash routine in ajMessCrashFL), this
** could be changed to allow the application to register an exit handler.
**
** @param [r] format [const char*] Format
** @param [v] [...] Variable length argument list
** @return [void]
** @@
******************************************************************************/

__noreturn void  ajMessExitmsg(const char *format, ...)
{
    const char *prefix   = EXIT_PREFIX;
    const char *mesg_buf = NULL;
    va_list args;

    AJAXFORMATSTRING(args, format, mesg_buf, prefix);

    messDump(mesg_buf);

    if(exitRoutine)
	(*exitRoutine)(mesg_buf);
    else
	fprintf(stderr, "%s\n", mesg_buf);

    exit(EXIT_FAILURE);
}




/* @func ajMessCrashFL ********************************************************
**
** This is the routine called by the ajFatal macro and others.
**
** This routine may encounter errors itself, in which case it will attempt
** to call itself to report the error. To avoid infinite recursion we limit
** this to just one reporting of an internal error and then we abort.
**
** @param [r] format [const char*] Format
** @param [v] [...] Variable length argument list
** @return [void]
** @@
******************************************************************************/

__noreturn void  ajMessCrashFL(const char *format, ...)
{
    enum {MAXERRORS = 1};
    static ajint internalErrors = 0;
    static char prefix[1024];
    ajint rc;
    const char *mesg_buf = NULL;
    va_list args;


    if(internalErrors > MAXERRORS)
	abort();
    else
	internalErrors++;

    /* Construct the message prefix, adding the program name if possible. */

    if(messGetErrorProgram() == NULL)
	rc = sprintf(prefix, CRASH_PREFIX_FORMAT, MESG_TITLE,
		     messGetErrorFile(), messGetErrorLine());
    else
	rc = sprintf(prefix, FULL_CRASH_PREFIX_FORMAT, MESG_TITLE,
		     messGetErrorProgram(), messGetErrorFile(),
		     messGetErrorLine());
    if(rc < 0)
	ajMessCrash("sprintf failed");

    if(AjErrorLevel.fatal)
    {
	AJAXFORMATSTRING(args, format, mesg_buf, prefix);

	messDump(mesg_buf);

	if(crashRoutine)
	    (*crashRoutine)(mesg_buf);
	else
	    fprintf(stderr, "%s\n", mesg_buf);

	ajMessInvokeDebugger();
    }


    exit(EXIT_FAILURE);
}




/* @func ajMessVCrashFL *******************************************************
**
** This is the routine called by the ajVFatal macro and others.
**
** This routine may encounter errors itself, in which case it will attempt
** to call itself to report the error. To avoid infinite recursion we limit
** this to just one reporting of an internal error and then we abort.
**
** @param [r] format [const char*] Format
** @param [v] args [va_list] Variable length argument list
** @return [void]
** @@
******************************************************************************/

__noreturn void  ajMessVCrashFL(const char *format, va_list args)
{
    enum {MAXERRORS = 1};
    static ajint internalErrors = 0;
    static char prefix[1024];
    ajint rc;
    char *mesg_buf = NULL;
    
    if(internalErrors > MAXERRORS)
	abort();
    else
	internalErrors++;
    
    /* Construct the message prefix, adding the program name if possible. */
    
    if(messGetErrorProgram() == NULL)
	rc = sprintf(prefix, CRASH_PREFIX_FORMAT, MESG_TITLE,
		     messGetErrorFile(), messGetErrorLine());
    else
	rc = sprintf(prefix, FULL_CRASH_PREFIX_FORMAT, MESG_TITLE,
		     messGetErrorProgram(), messGetErrorFile(),
		     messGetErrorLine());
    if(rc < 0)
	ajMessCrash("sprintf failed");
    
    
    AJAXVFORMATSTRING(args, format, mesg_buf, prefix);
    
    messDump(mesg_buf);
    
    if(crashRoutine)
	(*crashRoutine)(mesg_buf);
    else
	fprintf(stderr, "%s\n", mesg_buf);
    
    ajMessInvokeDebugger();
    
    exit(EXIT_FAILURE);
}



/* @func ajMessGetMessageC **************************************************
**
** Returns the current message text.
**
** @return [const char*] Message text
** @@
******************************************************************************/

const char* ajMessGetMessageC(void)
{
    return messbuf;
}



/* @obsolete ajMessCaughtMessage
** @rename ajMessGetMessageC
*/

__deprecated char* ajMessCaughtMessage(void)
{
    return messbuf;
}




/* @func ajMessGetSysmessageC **************************************************
**
** Returns the system message text from 'strerror' from the standard C
** library.
**
** @return [const char*] System error message.
** @@
******************************************************************************/

const char* ajMessGetSysmessageC(void)
{
    char *mess;

    if(errno)
	mess = ajFmtString(SYSERR_FORMAT, errno, strerror(errno));
    else
	mess = ajFmtString(SYSERR_OK, errno, strerror(errno));
      
    /* must make copy - will be used when mess* calls itself */
    AJFREE(messErrMess);
    messErrMess = ajSysFuncStrdup(mess);

    AJFREE(mess);

    return messErrMess;
}


/* @obsolete ajMessSysErrorText
** @rename ajMessGetSysmessageC
*/

__deprecated char* ajMessSysErrorText(void)
{
    char *mess;

    if(errno)
	mess = ajFmtString(SYSERR_FORMAT, errno, strerror(errno));
    else
	mess = ajFmtString(SYSERR_OK, errno, strerror(errno));
      
    /* must make copy - will be used when mess* calls itself */
    AJFREE(messErrMess);
    messErrMess = ajSysFuncStrdup(mess);

    AJFREE(mess);

    return messErrMess;
}

/************************* message formatting ********************************/
/* This routine does the formatting of the message string using vsprintf,    */
/* it copes with the format string accidentally being our internal buffer.   */
/*                                                                           */
/* This routine does its best to check that the vsprintf is successful, if   */
/* not the routine bombs out with an error message. Note that num_bytes is   */
/* the return value from vsprintf.                                           */
/* Failures trapped:                                                         */
/*      num_bytes less than 0  =  vsprintf failed, reason is reported.      */
/* num_bytes + 1 more than BUFSIZE  =  our internal buffer size was         */
/*                                      exceeded.                            */
/*                                 (vsprintf returns number of bytes written */
/*                                  _minus_ terminating NULL)                */
/*                                                                           */




/* @funcstatic messFormat *****************************************************
**
** Used by the AJAXFORMAT macros to format messages.
**
** This routine does the formatting of the message string using vsprintf,
** it copes with the format string accidentally being our internal buffer.
**
** This routine does its best to check that the vsprintf is successful, if
** not the routine bombs out with an error message. Note that num_bytes is
** the return value from vsprintf.
**
** Failures trapped:
**  num_bytes less than 0  =  vsprintf failed, reason is reported.
**  num_bytes + 1 more than BUFSIZE  =  our internal buffer size was exceeded.
**                                 (vsprintf returns number of bytes written
**                                  _minus_ terminating NULL)
**
** @param [v] args [va_list] Variable length argument list
** @param [r] format [const char*] Format string
** @param [r] prefix [const char*] Message prefix
** @return [char*] Formatted message text
** @@
******************************************************************************/

static char* messFormat(va_list args, const char *format, const char *prefix)
{
    static char *new_buf = NULL;
    char *buf_ptr;
    ajint num_bytes;
    ajint prefix_len;


    if(format == NULL)
	ajMessCrash("invalid call, no format string.");

    if(prefix == NULL)
	prefix_len = 0;
    else
    {
	prefix_len = strlen(prefix);
	if((prefix_len + 1) > PREFIXSIZE)
	    ajMessCrash("prefix string is too long.");
    }


    /* If they supply the internal buffer as an argument, e.g. because they */
    /* used ajFmtString as an arg, then make a copy, otherwise use internal */
    /* buffer.                                                              */

    if(format == messbuf)
    {
	if(new_buf != NULL)
	    AJFREE(new_buf);
	buf_ptr = new_buf = ajSysFuncStrdup(format);
    }
    else
	buf_ptr = messbuf;

    /* Add the prefix if there is one. */
    if(prefix != NULL)
    {
	if(!strcpy(buf_ptr, prefix))
	    ajMessCrash("strcpy failed");
    }


    num_bytes = prefix_len + 1;
    num_bytes += ajFmtVPrintCL((buf_ptr + prefix_len),BUFSIZE, format, args);

    /*
    **  Check the result. This should never happen using the
    **  ajFmtVPrintCL routine instead of the vsprintf routine
    */

    if(num_bytes < 0)
	ajMessCrash("vsprintf failed: %s", ajMessGetSysmessageC());
    else if(num_bytes > BUFSIZE)
	ajMessCrash("messubs internal buffer size (%d) exceeded, "
		    "a total of %d bytes were written",
		    BUFSIZE, num_bytes);

    return(buf_ptr);
}




/* @funcstatic messGetFilename ************************************************
**
** Converts a filename into a base file name. Used for filenames passed
** by macros from __FILE__ which could include part or all of the path
** depending on how the source code was compiled.
**
** @param [r] path [const char*] File name, possibly with full path.
** @return [char*] Base file name
** @@
******************************************************************************/

static char* messGetFilename(const char *path)
{
    static char *path_copy = NULL;
    const char *path_delim = SUBDIR_DELIMITER_STR;
    char *result = NULL;
    char *tmp;

    if(path != NULL)
    {
	if(strcmp((path + strlen(path) - 1), path_delim) != 0)
	{				/* Last char = "/" ?? */
	    if(path_copy != NULL)
		AJFREE(path_copy);

	    path_copy = ajSysFuncStrdup(path);

	    tmp = ajSysFuncStrtok(path_copy, path_delim);

	    while(tmp != NULL)
	    {
		result = tmp;	 /* Keep results of previous strtok */

		tmp = ajSysFuncStrtok(NULL, path_delim);
	    }
	}
    }

    return(result);
}




/*
** When AJAX needs to crash because there has been an unrecoverable
** error the file and line number of the code that detected the error
** need to be outputted. Here are the functions to do it.
**
** Applications can optionally initialise the error handling section of the
** message package, currently the program name can be set (argv[0] in the
** main routine) as there is no easy way to get at this at run time except
** from the main.
** */




/* @func ajMessErrorInit ******************************************************
**
** Initialises the stored program name.
**
** @param [r] progname [const char*] Program name.
** @return [void]
** @@
******************************************************************************/

void ajMessErrorInit(const char *progname)
{
    if(progname != NULL)
	messageG.progname = ajSysFuncStrdup(messGetFilename(progname));

    return;
}




/* @func ajMessSetErr *********************************************************
**
** Stores the source file name (converted to a base name)
** and the source line number to be
** reported by the crash routines.
**
** Invoked automatically by a macro (e.g. ajFatal) where needed.
**
** @param [r] filename [const char*] source filename, __FILE__
** @param [r] line_num [ajint] source line number, __LINE__
** @return [void]
** @@
******************************************************************************/

void ajMessSetErr(const char *filename, ajint line_num)
{
    assert(filename != NULL && line_num != 0);

    /*
    ** We take the basename here because __FILE__ can be a path rather
    ** than just a filename, depending on how a module was compiled.
    */

    messageG.filename = ajSysFuncStrdup(messGetFilename(filename));

    messageG.line_num = line_num;

    ajUtilCatch();

    return;
}




/* Access functions for message error data.                                  */
/* @funcstatic messGetErrorProgram ********************************************
**
** Returns the stored program name.
**
** @return [char*] Program name
** @@
******************************************************************************/

static char* messGetErrorProgram(void)
{
    return(messageG.progname);
}




/* @funcstatic messGetErrorFile ***********************************************
**
** Returns the stored error file name.
**
** @return [char*] Error file name
** @@
******************************************************************************/

static char* messGetErrorFile(void)
{
    return(messageG.filename);
}




/* @funcstatic messGetErrorLine ***********************************************
**
** Returns the stored error source line number.
**
** @return [ajint] Original source code line number
** @@
******************************************************************************/

static ajint messGetErrorLine(void)
{
    return(messageG.line_num);
}




/* set a file to read for all the messages. NB if this is not set
Then a default one will be read */

/* @func ajMessErrorSetFile ***************************************************
**
** Opens a file and sets this to be the error file.
**
** @param [r] errfile [const char*] Error file name
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajMessErrorSetFile(const char *errfile)
{
    FILE *fp = 0;

    if(errfile)
	if((fp = fopen(errfile,"r")))
	{
	    messErrorFile = ajSysFuncStrdup(errfile);
	    fclose(fp);

	    return ajTrue;
	}

    return ajFalse;
}




/* @funcstatic ajMessReadErrorFile ********************************************
**
** Reads the error message file (with a default of
** $EMBOSS_ROOT/messages/messages.english)
** and loads the results into an internal table.
**
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool ajMessReadErrorFile(void)
{
    char line[512];
    char name[12];
    char message[200];
    FILE *fp=0;
    char *mess;
    char *cp;
    char *namestore;
    char *messstore;

    if(messErrorFile)
	fp = fopen(messErrorFile,"r");

    if(!fp)
    {
	messErrorFile = ajFmtString("%s/messages/messages.english",
				    getenv("EMBOSS_ROOT"));
	fp = fopen(messErrorFile,"r");
    }


    if(!fp)
	return ajFalse;

    messErrorTable = ajTablecharNew();

    while(fgets(line, 512, fp))
    {
	if(sscanf(line,"%s %s",name,message)!=2)
	    ajFatal("Library sscanf1");

	cp = strchr(line,'"');
	cp++;
	mess = &message[0];

	while(*cp != '"')
	{
	    *mess = *cp;
	    cp++;
	    mess++;
	}

	*mess = '\0';
	namestore = ajFmtString("%s",name);
	messstore = ajFmtString("%s",message);
	mess = (char *) ajTableFetch(messErrorTable, namestore);

	if(mess)
	    ajErr("%s is listed more than once in file %s",
			name,messErrorFile);
	else
	    ajTablePut(messErrorTable, namestore, messstore);
    }

    return ajTrue;
}




/* @func ajMessOutCode ********************************************************
**
** Writes an output message for a given message code.
**
** @param [r] code [const char*] Message code
** @return [void]
** @@
******************************************************************************/

void ajMessOutCode(const char *code)
{
    char *mess=0;

    if(messErrorTable)
    {
	mess = ajTableFetch(messErrorTable, code);

	if(mess)
	    ajMessOut(mess);
	else
	    ajMessOut("could not find error code %s",code);
    }
    else
    {
	if(ajMessReadErrorFile())
	{
	    mess = ajTableFetch(messErrorTable, code);

	    if(mess)
		ajMessOut(mess);
	    else
		ajMessOut("could not find error code %s",code);
	}
	else
	    ajMessOut("Could not read the error file hence no reference to %s",
		      code);
    }

    return;
}




/* @func ajMessErrorCode ******************************************************
**
** Writes an error message for a given message code.
**
** @param [r] code [const char*] Error code
** @return [void]
** @@
******************************************************************************/

void ajMessErrorCode(const char *code)
{
    char *mess = 0;

    if(messErrorTable)
    {
	mess = ajTableFetch(messErrorTable, code);

	if(mess)
	    ajErr(mess);
	else
	    ajErr("could not find error code %s",code);
    }
    else
    {
	if(ajMessReadErrorFile())
	{
	    mess = ajTableFetch(messErrorTable, code);

	    if(mess)
		ajErr(mess);
	    else
		ajErr("could not find error code %s",code);
	}
	else
	    ajErr("Could not read the error file, "
		  "hence no reference to %s",
		  code);
    }

    return;
}




/* @func ajMessCrashCodeFL ****************************************************
**
** Writes an error message for a given message code and crashes.
**
** @param [r] code [const char*] Error code
** @return [void]
** @@
******************************************************************************/

__noreturn void  ajMessCrashCodeFL(const char *code)
{
    char *mess = 0;

    if(messErrorTable)
    {
	mess = ajTableFetch(messErrorTable, code);

	if(mess)
	    ajMessCrashFL(mess);
	else
	    ajMessCrashFL("could not find error code %s",code);
    }
    else
    {
	if(ajMessReadErrorFile())
	{
	    mess = ajTableFetch(messErrorTable, code);

	    if(mess)
		ajMessCrashFL(mess);
	    else
		ajMessCrashFL("could not find error code %s",code);
	}
	else
	    ajMessCrashFL("Could not read the error file "
			  "hence no reference to %s",
			  code);
    }
}




/* @func ajMessCodesDelete ****************************************************
**
** Deletes the message codes table.
**
** @return [void]
** @@
******************************************************************************/

void ajMessCodesDelete(void)
{
    messTableDelete(&messErrorTable);
}

/* @funcstatic messTableDelete ************************************************
**
** Delete a table, simply freeing the key and value
**
** @param [d] table [AjPTable*] Table
** @return [void]
******************************************************************************/

static void messTableDelete(AjPTable* table) 
{
    void **keyarray = NULL;
    void **valarray = NULL;
    ajint i;

    if(!table)
	return;
    if(!*table)
	return;

    ajTableToarrayKeysValues(*table, &keyarray, &valarray);

    for(i = 0; keyarray[i]; i++)
    {
	AJFREE(keyarray[i]);
	AJFREE(valarray[i]);
    }

    AJFREE(keyarray);
    AJFREE(valarray);

    ajTableFree(table);
    *table = NULL;

    return;
}


/* @func ajDebug **************************************************************
**
** Writes a debug message to the debug file if debugging is on.
** Typically, debugging is turned on by adding '-debug' to the command line
** or by defining a variable prefix_DEBUG
**
** Avoid using this call in any code which can be invoked before the command
** line processing is complete as it can be a problem to find a reasonable
** file name for debug output under these circumstances.
**
** @param [r] fmt [const char*] Format.
** @param [v] [...] Variable argument list.
** @return [void]
** @@
******************************************************************************/

void ajDebug(const char* fmt, ...)
{
    va_list args;
    static ajint debugset = 0;
    static ajint depth    = 0;
    AjPStr bufstr         = NULL;
    
    if(depth)
    {				   /* recursive call, get out quick */
	if(messDebugFile)
	{
	    va_start(args, fmt);
	    ajFmtVPrintF(messDebugFile, fmt, args);
	    va_end(args);
	}

	return;
    }

    depth++;

    if(!debugset && acdDebugSet)
    {
	messDebug = acdDebug;

	if(messDebug)
	{
	    ajFmtPrintS(&messDebugName, "%s.dbg", ajStrGetPtr(acdProgram));
	    messDebugFile = ajFileNewOutNameS(messDebugName);

	    if(!messDebugFile)
		ajFatal("Cannot open debug file %S",messDebugName);

	    if(ajNamGetValueC("debugbuffer", &bufstr))
	    {
		ajStrToBool(bufstr, &acdDebugBuffer);
	    }

	    if(!acdDebugBuffer)
		ajFileSetUnbuffer(messDebugFile);

	    ajFmtPrintF(messDebugFile, "Debug file %F buffered:%B\n",
			 messDebugFile, acdDebugBuffer);
	    ajStrDel(&bufstr);
	}

	debugset = 1;
    }

    if(messDebug)
    {
	va_start(args, fmt);
	ajFmtVPrintF(messDebugFile, fmt, args);
	va_end(args);
    }

    depth--;

    return;
}




/* @func ajDebugTest **********************************************************
**
** Tests a token string and returns true if the user has requested debug output
**
** @param [r] token [const char*] Token name
** @return [AjBool] True if token has debugging requested
** @@
******************************************************************************/

AjBool ajDebugTest(const char* token)
{
    AjPStr filename = NULL;
    const char* debugtestname = ".debugtest";
    char* ctoken = NULL;
    AjPStr line = NULL;
    AjPStr strtoken = NULL;
    AjPStr rest = NULL;
    static ajint depth    = 0;

    struct 
    {
        ajuint count;
        ajuint max;
    } *stats;
    
    if(depth)
        return ajFalse;

    depth++;

    if(!messDebugTestInit)
    {
        filename = ajStrNewC(debugtestname);

        if(ajFilenameExists(filename))
        {
            messDebugTestFile = ajFileNewInNameS(filename);
        }
        else
        {
            ajFmtPrintS(&filename, "%s%s%s",
                        getenv("HOME"), SLASH_STRING, debugtestname); 
            if(ajFilenameExists(filename))
                messDebugTestFile = ajFileNewInNameS(filename);
        }
        ajStrDel(&filename);

        if(messDebugTestFile) 
        {
            messDebugTestTable = ajTablecharNewLen(256);

            while(ajReadlineTrim(messDebugTestFile, &line))
            {
                if(ajStrExtractFirst(line, &rest, &strtoken))
                {
                    AJNEW0(stats);
                    ctoken = ajCharNewS(strtoken);
                    if(ajStrIsInt(rest))
                        ajStrToUint(rest, &stats->max);
                    else
                        stats->max = UINT_MAX;
                    ajTablePut(messDebugTestTable, ctoken, stats);
                    ctoken = NULL;
                    stats = NULL;
                }
            }

            ajStrDel(&line);
            ajStrDel(&strtoken);
            ajStrDel(&rest);
            ajFileClose(&messDebugTestFile);
        }
        messDebugTestInit = ajTrue;
     }

    depth--;
    
    if(!messDebugTestTable)
        return ajFalse;

    depth++;
    stats = ajTableFetch(messDebugTestTable, token);
    depth--;
    

    if(!stats)
        return ajFalse;

    if(!stats->max)
        return ajTrue;

    if(stats->count++ >= stats->max)
        return ajFalse;

    return ajTrue;
}




/* @func ajMessGetDebugfile ****************************************************
**
** Returns the file used for debug output, or NULL if no debug file is open.
**
** @return [FILE*] C runtime library file handle for debug output.
** @@
******************************************************************************/

FILE* ajMessGetDebugfile(void)
{
    if(!messDebugFile)
	return NULL;

    return ajFileGetFileptr(messDebugFile);
}



/* @obsolete ajDebugFile
** @rename ajMessGetDebugfile
*/

__deprecated FILE* ajDebugFile(void)
{
    return ajMessGetDebugfile();
}



/* @func ajUserGet ************************************************************
**
** Writes a prompt to the terminal and reads one line from the user.
**
** @param [w] pthis [AjPStr*] Buffer for the user response.
** @param [r] fmt [const char*] Format string
** @param [v] [...] Variable argument list.
** @return [ajint] Length of response string.
** @@
******************************************************************************/

ajint ajUserGet(AjPStr* pthis, const char* fmt, ...)
{
    AjPStr thys;
    const char *cp;
    char *buff;
    va_list args;
    ajint ipos;
    ajint isize;
    ajint ilen;
    ajint jlen;
    ajint fileBuffSize = ajFileValueBuffsize();

    va_start(args, fmt);
    ajFmtVError(fmt, args);
    va_end(args);

    if(ajFileValueRedirectStdin())
    {
	ajUser("(Standard input in use: using default)");
	ajStrAssignC(pthis, "");

	return ajStrGetLen(*pthis);
    }

    ajStrSetRes(pthis, fileBuffSize);
    buff  = ajStrGetuniquePtr(pthis);
    thys = *pthis;
    isize = ajStrGetRes(thys);
    ilen  = 0;
    ipos  = 0;
    

    /*ajDebug("ajUserGet buffer len: %d res: %d ptr: %x\n",
	     ajStrGetLen(thys), ajStrGetRes(thys), thys->Ptr);*/

    while(buff)
    {

#ifndef __ppc__
	cp = fgets(&buff[ipos], isize, stdin);
#else
	cp = ajSysFuncFgets(&buff[ipos], isize, stdin);
#endif

        if(!cp && !ipos)
	{
	    if(feof(stdin))
	    {
		ajErr("Unable to get reply from user - end of standard input");
		ajExitBad();
	    }
	    else
		ajFatal("Error reading from user: '%s'\n",
			strerror(errno));
	}

	jlen = strlen(&buff[ipos]);
	ilen += jlen;

	/*
	 ** We need to read again if:
	 ** We have read the entire buffer
	 ** and we don't have a newline at the end
	 ** (must be careful about that - we may just have read enough)
	 */
	ajStrSetValidLen(pthis, ilen);
	thys = *pthis;

	if((jlen == (isize-1)) &&
	   (ajStrGetCharLast(thys) != '\n'))
	{
	    ajStrSetRes(pthis, ajStrGetRes(thys)+fileBuffSize);
	    thys = *pthis;
	    /*ajDebug("more to do: jlen: %d ipos: %d isize: %d ilen: %d "
		    "Size: %d\n",
		    jlen, ipos, isize, ilen, ajStrGetRes(thys));*/
	    ipos += jlen;
	    buff = ajStrGetuniquePtr(pthis);
	    isize = ajStrGetRes(thys) - ipos;
	    /* ajDebug("expand to: ipos: %d isize: %d Size: %d\n",
		    ipos, isize, ajStrGetRes(thys)); */

	}
	else
	    buff = NULL;
    }
    
    ajStrSetValidLen(pthis, ilen);

    if(ajStrGetCharLast(*pthis) == '\n')
	ajStrCutEnd(pthis, 1);

    /* PC files have \r\n Macintosh files have just \r : this fixes both */

    if(ajStrGetCharLast(*pthis) == '\r')
    {
	/*ajDebug("Remove carriage-return characters from PC-style files\n");*/
	ajStrCutEnd(pthis, 1);
    }

    ajStrTrimWhite(pthis);

    return ajStrGetLen(*pthis);
}




/* @func ajMessExit ***********************************************************
**
** Delete any static initialised values
**
** @return [void]
** @@
******************************************************************************/

void ajMessExit(void)
{
    AJFREE(messErrMess);
    messTableDelete(&messErrorTable);
    messTableDelete(&messDebugTestTable);

    return;
}

/* @func ajMessExitDebug *******************************************************
**
** Delete any static initialised values for ajDebug calls
**
** @return [void]
** @@
******************************************************************************/

void ajMessExitDebug(void)
{
    ajFileClose(&messDebugFile);
    ajStrDel(&messDebugName);
    ajFileClose(&messDebugTestFile);
    ajStrDel(&messDebugName);

    return;
}

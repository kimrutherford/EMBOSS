/* @include ajmess ************************************************************
**
** AJAX message functions
**
** @author Richard Durbin and Ed Griffiths from ACEdb (messubs.c)
** @version $Revision: 1.29 $
** @modified Ian Longden for EMBOSS
** @modified $Date: 2011/10/18 14:23:40 $ by $Author: rice $
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

#ifndef AJMESS_H
#define AJMESS_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajstr.h"

#include <stdarg.h>
#include <setjmp.h>

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




#ifndef WIN32
extern AjBool acdDebugSet;
extern AjBool acdDebugBuffer;
extern AjBool acdDebug;
extern AjPStr acdProgram;
extern AjPStr acdAppldoc;
extern AjPStr acdArgSave;
extern AjPStr acdInputSave;
#else /* WIN32 */

#ifdef AJAXDLL_EXPORTS
__declspec(dllexport) AjBool acdDebugSet;
__declspec(dllexport) AjBool acdDebugBuffer;
__declspec(dllexport) AjBool acdDebug;
__declspec(dllexport) AjPStr acdProgram;
__declspec(dllexport) AjPStr acdAppldoc;
__declspec(dllexport) AjPStr acdArgSave;
__declspec(dllexport) AjPStr acdInputSave;
#else /* !AJAXDLL_EXPORTS */
__declspec(dllimport) AjBool acdDebugSet;
__declspec(dllimport) AjBool acdDebugBuffer;
__declspec(dllimport) AjBool acdDebug;
__declspec(dllimport) AjPStr acdProgram;
__declspec(dllimport) AjPStr acdAppldoc;
__declspec(dllimport) AjPStr acdArgSave;
__declspec(dllimport) AjPStr acdInputSave;
#endif /* AJAXDLL_EXPORTS */

#endif /* !WIN32 */




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* @data AjPError *************************************************************
**
** Ajax error message levels object
**
** @alias AjSError
** @alias AjOError
**
** @attr warning [AjBool] Display ajWarn messages
** @attr error [AjBool] Display ajErr messages
** @attr fatal [AjBool] Display ajFatal messages
** @attr die [AjBool] Display ajDie messages
** @@
******************************************************************************/

typedef struct AjSError
{
    AjBool warning;
    AjBool error;
    AjBool fatal;
    AjBool die;
} AjOError;

#define AjPError AjOError*

#ifndef WIN32
extern AjOError AjErrorLevel;
#else /* WIN32 */

#ifdef AJAXDLL_EXPORTS
__declspec(dllexport) AjOError AjErrorLevel;
#else /* !AJAXDLL_EXPORTS */
__declspec(dllimport) AjOError AjErrorLevel;
#endif /* AJAXDLL_EXPORTS */

#endif /* !WIN32 */

#define SUBDIR_DELIMITER_STR "\\"



typedef void (*AjMessVoidRoutine)(void);
typedef void (*AjMessOutRoutine)(const char*);




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */




/*
** Prototype definitions
*/

AjMessVoidRoutine ajMessRegBeep(AjMessVoidRoutine func);
AjMessOutRoutine  ajMessRegCrash(AjMessOutRoutine func);
AjMessOutRoutine  ajMessRegDump(AjMessOutRoutine func);
AjMessOutRoutine  ajMessRegErr(AjMessOutRoutine func);
AjMessOutRoutine  ajMessRegExit(AjMessOutRoutine func);
AjMessOutRoutine  ajMessRegOut(AjMessOutRoutine func);
AjMessOutRoutine  ajMessRegWarn(AjMessOutRoutine func);

void              ajMessSetErr(const char *filename, ajint line_num);
void              ajMessCrashFL(const char *format, ...);
void              ajMessVCrashFL(const char *format, va_list args);
void              ajMessCrashCodeFL(const char *code);

/* External Interface.                                                       */
/* Note: ajMesscrash is a macro and makes use of the ',' operator            */
/* in C. This means that the ajMesscrash macro will only produce a single C  */
/* statement and hence can be used within brackets etc. and will not break   */
/* existing code, e.g.                                                       */
/*                     funcblah(ajMesscrash("hello"));                       */
/* will become:                                                              */
/* funcblah(uMessSetErrorOrigin(__FILE__, __LINE__), uMessCrash("hello"));   */
/*                                                                           */

void              ajMessErrorInit(const char *progname); /* Record
                                                            the application's
                                                            name for use in
                                                            error messages. */

void              ajMessBeep(void); /* make a beep */

AjBool            ajMessErrorSetFile(const char *errfile);  /* set file to
                                                               read codes +
                                                               messages from */
void              ajMessCodesDelete(void);  /* Delete the code/message pairs */

void              ajMessOut(const char *format, ...);  /* simple message,
                                                          no newline */
void              ajMessOutCode(const char *name);       /* as above but uses
                                                            codes to get
                                                            message */
void              ajMessDump(const char *format, ...); /* write to
                                                          log file */
void              ajMessErrorCode(const char *name);      /* as above but
                                                             uses code to
                                                             get message */
void              ajMessExit(void);
void              ajMessExitDebug(void);
void              ajMessExitmsg(const char *format, ...);  /* error message,
                                                              write to log
                                                              file & exit */
void              ajDebug(const char *fmt, ...);
AjBool            ajDebugOn(void);
AjBool            ajDebugTest(const char *token);
FILE*             ajMessGetDebugfile(void);
void              ajDie(const char *format, ...);
void              ajErr(const char *format, ...); /* error message and
                                                     write to
                                                     log file */
ajint             ajUserGet(AjPStr *pthis, const char *fmt, ...);
void              ajUser(const char *format, ...);  /* simple msg with
                                                       newline */
void              ajUserDumpC(const char* txt);  /* simple msg with string
                                                    and newline */
void              ajUserDumpS(const AjPStr str);  /* simple msg with string
                                                     and newline */
void              ajVDie(const char *format, va_list args); /* error message
                                                               and
                                                               write to log
                                                               file */
void              ajVErr(const char *format, va_list args); /* error message
                                                               and
                                                               write to log
                                                               file */
void              ajVUser(const char *format, va_list args);
void              ajVWarn(const char *format, va_list args); /* warning
                                                                message */
void              ajWarn(const char *format, ...); /* warning message */

/* abort - but see below */

/* ask for data satisfying format get results via freecard() */

const char*       ajMessGetSysmessageC(void);
/* wrapped system error message for use in ajMesserror/crash() */

ajint             ajMessGetCountError(void);
/* return numbers of error so far */

/**** routines to catch crashes if necessary, e.g. when dumping ****/



const char*       ajMessGetMessageC(void);

void              ajMessInvokeDebugger(void);


#define ajMessCrash   ajMessSetErr(__FILE__, __LINE__), ajMessCrashFL
#define ajMessCrashCode ajMessSetErr(__FILE__, __LINE__), ajMessCrashCodeFL
#define ajFatal   ajMessSetErr(__FILE__, __LINE__), ajMessCrashFL
#define ajVFatal   ajMessSetErr(__FILE__, __LINE__), ajMessVCrashFL


/*
** End of prototype definitions
*/


/* if a setjmp() stack context is set using ajMessCatch*() then rather than
** exiting or giving an error message, ajMessCrash() and messError() will
** longjmp() back to the context.
** ajMessCatch*() return the previous value. Use argument = 0 to reset.
** ajMessCaughtMessage() can be called from the jumped-to routine to get
** the error message that would have been printed.
*/

#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif /* AJ_COMPILE_DEPRECATED_BOOK */

#ifdef AJ_COMPILE_DEPRECATED

__deprecated ajint             ajMessErrorCount(void);
__deprecated char*             ajMessCaughtMessage(void);
__deprecated char*             ajMessSysErrorText(void);
__deprecated FILE*             ajDebugFile(void);

#endif /* AJ_COMPILE_DEPRECATED */




AJ_END_DECLS

#endif /* AJMESS_H */

#ifdef __cplusplus
extern "C"
{
#endif

/*  File: ajmess.h
 *  Original author: Richard Durbin (rd@mrc-lmb.cam.ac.uk) and Ed Griffiths.
 *  as part of the ACEDB package (messubs.h)
 *  Modified: by I Longden for the EMBOSS package.
 */

#ifndef ajmess_h
#define ajmess_h

#include <stdarg.h>
#include <setjmp.h>

#ifndef WIN32
extern AjBool acdDebugSet;
extern AjBool acdDebugBuffer;
extern AjBool acdDebug;
extern AjPStr acdProgram;
extern AjPStr acdArgSave;
extern AjPStr acdInputSave;
#else
#ifdef AJAXDLL_EXPORTS
__declspec(dllexport) AjBool acdDebugSet;
__declspec(dllexport) AjBool acdDebugBuffer;
__declspec(dllexport) AjBool acdDebug;
__declspec(dllexport) AjPStr acdProgram;
__declspec(dllexport) AjPStr acdArgSave;
__declspec(dllexport) AjPStr acdInputSave;
#else
__declspec(dllimport) AjBool acdDebugSet;
__declspec(dllimport) AjBool acdDebugBuffer;
__declspec(dllimport) AjBool acdDebug;
__declspec(dllimport) AjPStr acdProgram;
__declspec(dllimport) AjPStr acdArgSave;
__declspec(dllimport) AjPStr acdInputSave;
#endif
#endif




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
#else
#ifdef AJAXDLL_EXPORTS
 __declspec(dllexport) AjOError AjErrorLevel;
#else
 __declspec(dllimport) AjOError AjErrorLevel;
#endif
#endif

#define SUBDIR_DELIMITER_STR "\\"



typedef void (*AjMessVoidRoutine)(void) ;
typedef void (*AjMessOutRoutine)(const char*) ;




/*
** Prototype definitions
*/

AjMessVoidRoutine ajMessRegBeep (AjMessVoidRoutine func) ;
AjMessOutRoutine  ajMessRegCrash (AjMessOutRoutine func) ;
AjMessOutRoutine  ajMessRegDump (AjMessOutRoutine func) ;
AjMessOutRoutine  ajMessRegErr (AjMessOutRoutine func) ;
AjMessOutRoutine  ajMessRegExit (AjMessOutRoutine func) ;
AjMessOutRoutine  ajMessRegOut (AjMessOutRoutine func) ;
AjMessOutRoutine  ajMessRegWarn (AjMessOutRoutine func) ;

void              ajMessSetErr(const char *filename, ajint line_num);
void              ajMessCrashFL (const char *format, ...);
void              ajMessVCrashFL (const char *format, va_list args);
void              ajMessCrashCodeFL (const char *code);

/* External Interface.                                                       */
/* Note: ajMesscrash is a macro and makes use of the ',' operator    */
/* in C. This means that the ajMesscrash macro will only produce a single C  */
/* statement and hence can be used within brackets etc. and will not break   */
/* existing code, e.g.                                                       */
/*                     funcblah(ajMesscrash("hello")) ;                      */
/* will become:                                                              */
/* funcblah(uMessSetErrorOrigin(__FILE__, __LINE__), uMessCrash("hello")) ;  */
/*                                                                           */

void              ajMessErrorInit(const char *progname) ; /* Record
							     the application's
							     name for use in
							     error messages. */

void              ajMessBeep (void) ; /* make a beep */

AjBool            ajMessErrorSetFile(const char *errfile);  /* set file to
							       read codes +
							       messages from */
void              ajMessCodesDelete(void);  /* Delete the code/message pairs */

void              ajMessOut (const char *format, ...) ;  /* simple message,
							    no newline */
void              ajMessOutCode(const char *name);       /* as above but uses
							    codes to get
							    message */
void              ajMessDump (const char *format, ...) ; /* write to
							    log file */
void              ajMessErrorCode(const char *name);      /* as above but
							     uses code to
							     get message */
void              ajMessExit (void);
void              ajMessExitDebug (void);
void              ajMessExitmsg(const char *format, ...) ;  /* error message,
							       write to log
							       file & exit */
void              ajDebug (const char *fmt, ...);
AjBool            ajDebugTest (const char *token);
FILE*             ajMessGetDebugfile (void);
void              ajDie (const char *format, ...);
void              ajErr (const char *format, ...) ; /* error message and
						       write to
						       log file */
ajint             ajUserGet (AjPStr *pthis, const char *fmt, ...);
void              ajUser (const char *format, ...) ;  /* simple msg with
							 newline */
void              ajUserDumpC (const char* txt) ;  /* simple msg with string
						   and newline */
void              ajUserDumpS (const AjPStr str) ;  /* simple msg with string
						   and newline */
void              ajVDie (const char *format, va_list args) ; /* error message
								 and
								 write to log
								 file */
void              ajVErr (const char *format, va_list args) ; /* error message
								 and
								 write to log
								 file */
void              ajVUser(const char *format, va_list args);
void              ajVWarn (const char *format, va_list args) ; /* warning
								  message */
void              ajWarn (const char *format, ...); /* warning message */

                                                  /* abort - but see below */

/* ask for data satisfying format get results via freecard() */

const char*       ajMessGetSysmessageC (void) ;
/* wrapped system error message for use in ajMesserror/crash() */

ajint             ajMessGetCountError (void);
/* return numbers of error so far */

/**** routines to catch crashes if necessary, e.g. when dumping ****/



const char*       ajMessGetMessageC (void) ;

void              ajMessInvokeDebugger(void);

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

__deprecated ajint             ajMessErrorCount (void);
__deprecated char*             ajMessCaughtMessage (void) ;
__deprecated char*             ajMessSysErrorText (void) ;
__deprecated FILE*             ajDebugFile (void);

#define ajMessCrash   ajMessSetErr(__FILE__, __LINE__), ajMessCrashFL
#define ajMessCrashCode ajMessSetErr(__FILE__, __LINE__), ajMessCrashCodeFL
#define ajFatal   ajMessSetErr(__FILE__, __LINE__), ajMessCrashFL
#define ajVFatal   ajMessSetErr(__FILE__, __LINE__), ajMessVCrashFL


#endif /* defined(DEF_REGULAR_H) */

#ifdef __cplusplus
}
#endif

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

extern AjBool acdDebugSet;
extern AjBool acdDebugBuffer;
extern AjBool acdDebug;
extern AjPStr acdProgram;

/* @data AjPError *************************************************************
**
** Ajax error message levels object
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

extern AjOError AjErrorLevel;

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
void              ajMessExitmsg(const char *format, ...) ;  /* error message,
							       write to log
							       file & exit */
void              ajDebug (const char *fmt, ...);
FILE*             ajDebugFile (void);
void              ajDie (const char *format, ...);
void              ajErr (const char *format, ...) ; /* error message and
						       write to
						       log file */
ajint             ajUserGet (AjPStr *pthis, const char *fmt, ...);
void              ajUser (const char *format, ...) ;  /* simple msg with
							 newline */
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

char*             ajMessSysErrorText (void) ;
/* wrapped system error message for use in ajMesserror/crash() */

ajint             ajMessErrorCount (void);
/* return numbers of error so far */

/**** routines to catch crashes if necessary, e.g. when dumping ****/



char*             ajMessCaughtMessage (void) ;

void              ajMessInvokeDebugger(void);

/* if a setjmp() stack context is set using ajMessCatch*() then rather than
** exiting or giving an error message, ajMessCrash() and messError() will
** longjmp() back to the context.
** ajMessCatch*() return the previous value. Use argument = 0 to reset.
** ajMessCaughtMessage() can be called from the jumped-to routine to get
** the error message that would have been printed.
*/

/*
** End of prototype definitions
*/




#define ajMessCrash   ajMessSetErr(__FILE__, __LINE__), ajMessCrashFL
#define ajMessCrashCode ajMessSetErr(__FILE__, __LINE__), ajMessCrashCodeFL
#define ajFatal   ajMessSetErr(__FILE__, __LINE__), ajMessCrashFL
#define ajVFatal   ajMessSetErr(__FILE__, __LINE__), ajMessVCrashFL


#endif /* defined(DEF_REGULAR_H) */

#ifdef __cplusplus
}
#endif

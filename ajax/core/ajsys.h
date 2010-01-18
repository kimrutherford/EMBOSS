#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajsys_h
#define ajsys_h

#include "ajax.h"
#include <sys/types.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifndef __VMS
#ifndef WIN32
#include <sys/param.h>
#endif
#endif
#include <sys/stat.h>




/*
** Prototype definitions
*/

AjBool        ajSysArglistBuild (const AjPStr cmdline,
				 char** Pname, char*** PParglist);
void          ajSysArglistFree (char*** arglist);
void          ajSysCanon(AjBool state);
char          ajSysCastItoc(ajint v);
unsigned char ajSysCastItouc(ajint v);
void          ajSysExit(void);
AjBool        ajSysFileUnlink(const AjPStr s);
AjBool        ajSysFileWhich(AjPStr *Pfilename);
AjBool        ajSysFileWhichEnv(AjPStr *Pfilename, char * const env[]);
FILE*         ajSysFuncFdopen(ajint filedes, const char *mode);
char*         ajSysFuncFgets(char *buf, int size, FILE *fp);
FILE*         ajSysFuncFopen(const char *name, const char *flags);
char*         ajSysFuncStrtokR(const char *s, const char *t,
			       const char **ptrptr,
			       AjPStr *buf);
char*         ajSysFuncStrtok(const char *s, const char *t);
char*         ajSysFuncStrdup(const char *s);
void          ajSysSystem(const AjPStr cl);
void          ajSysSystemEnv(const AjPStr cl, char * const env[]);
void          ajSysSystemOut(const AjPStr cl, const AjPStr outfname);

/*
** End of prototype definitions
*/


__deprecated void          ajSystem(const AjPStr cl);
__deprecated void          ajSystemEnv(const AjPStr cl, char * const env[]);
__deprecated void          ajSystemOut(const AjPStr cl, const AjPStr outfname);
__deprecated char         *ajSysStrdup(const char *s);
__deprecated char         *ajSysStrtok(const char *s, const char *t);
__deprecated char         *ajSysStrtokR(const char *s, const char *t,
					const char **ptrptr, AjPStr *buf);
__deprecated char         *ajSysFgets(char *buf, int size, FILE *fp);
__deprecated FILE         *ajSysFopen(const char *name, const char *flags);
__deprecated FILE         *ajSysFdopen(ajint filedes, const char *mode);
__deprecated AjBool        ajSysWhich(AjPStr *exefile);
__deprecated AjBool        ajSysWhichEnv(AjPStr *exefile, char * const env[]);
__deprecated void          ajSysBasename(AjPStr *filename);
__deprecated void          ajSysArgListFree (char*** arglist);
__deprecated AjBool        ajSysIsDirectory(const char *s);
__deprecated AjBool        ajSysIsRegular(const char *s);
__deprecated AjBool        ajSysArglist (const AjPStr cmdline,
					 char** Pname, char*** PParglist);
__deprecated char          ajSysItoC(ajint v);
__deprecated unsigned char ajSysItoUC(ajint v);
__deprecated AjBool        ajSysUnlink(const AjPStr s);

/*
** S_IFREG is non-ANSI therefore define it here
** At least keeps all the very dirty stuff in one place
*/

#ifndef S_IFREG
#define S_IFREG 0100000
#endif
#ifndef S_IFDIR
#define S_IFDIR 0x4000
#endif

#ifndef WIN32
#define AJ_FILE_REG S_IFREG
#define AJ_FILE_DIR S_IFDIR
#else
#define AJ_FILE_REG _S_IFREG
#define AJ_FILE_DIR _S_IFDIR
#endif

#endif

#ifdef __cplusplus
}
#endif

/* @include ajsys *************************************************************
**
** AJAX system functions
**
** Copyright (c) Alan Bleasby 1999
** @version $Revision: 1.38 $
** @modified $Date: 2011/11/23 09:52:54 $ by $Author: rice $
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

#ifndef AJSYS_H
#define AJSYS_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajstr.h"
#include "ajfile.h"

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

#ifndef WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <dirent.h>
#include <unistd.h>
#else
#include <winsock2.h>
#include <io.h>
#include <fcntl.h>
#endif

#include <signal.h>

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




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




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/*
** Structure to hold either a UNIX socket or a Windows socket.
** Allows calling functions to use sockets without needing ifdefs.
** Now implemented as an EMBOSS-style typedef though it is really a
** system thing
*/




/* @data AjPSysSocket *********************************************************
**
** Socket data for Unix or Windows systems
**
** @attr sock [ajint] Socket number for Unix
** @cc attr sock [SOCKET] Socket data for Windows
**
******************************************************************************/

typedef struct AjSSysSocket
{
#ifndef WIN32
    ajint sock;
#else
    SOCKET sock;
#endif
} AjOSysSocket;
#define AjPSysSocket AjOSysSocket*




/*
** Structure for use with alarm timeouts
** UNIX uses SIGALRM, but Windows does not have that signal.
** Windows requires setting up a timer. This structure is primarily
** to allow a handle to the timer to be passed back for use
** with the ajSysTimeoutUnset() timer cancellation function.
** On the UNIX side it holds the sigaction structure to allow
** The action handler to be set to SIG_IGN, for extra safety.
** Now implemented as an EMBOSS-style typedef, though it is really a
** system thing.
*/




/* @data AjPSysTimeout ********************************************************
**
** Timeout object for Unix and Windows
**
** @attr sa [struct sigaction] sigaction data for Unix
** @attr Padding [ajint] Padding to alignment boundary
** @cc attr thandle [HANDLE] Timeout handle for Windows
** @cc attr wtime [LARGE_INTEGER] Wide time for Windows
** @attr seconds [ajint] Time limit in seconds
**
******************************************************************************/

typedef struct AjSSysTimeout
{
#ifndef WIN32
    struct sigaction sa;
    ajint Padding;
#else
    HANDLE thandle;
    LARGE_INTEGER wtime;
#endif
    ajint seconds;
} AjOSysTimeout;

#define AjPSysTimeout AjOSysTimeout*




#ifndef WIN32
#define AJBADSOCK -1
#define SOCKRET int
#else
#define AJBADSOCK INVALID_SOCKET
#define SOCKRET SOCKET
#endif




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */




/*
** Prototype definitions
*/

AjBool        ajSysArglistBuildC(const char* cmdline,
                                 char** Pname, char*** PParglist);
AjBool        ajSysArglistBuildS(const AjPStr cmdline,
                                 char** Pname, char*** PParglist);
void          ajSysArglistFree(char*** arglist);
void          ajSysCanon(AjBool state);
char          ajSysCastItoc(ajint v);
unsigned char ajSysCastItouc(ajint v);
void          ajSysExit(void);
AjBool        ajSysFileUnlinkC(const char* s);
AjBool        ajSysFileUnlinkS(const AjPStr s);
FILE*         ajSysFdFromSocket(const AjOSysSocket sock, const char *mode);
AjBool        ajSysFileRmrfC(const char *path);
AjBool        ajSysFileRmrfS(const AjPStr path);
AjBool        ajSysFileWhich(AjPStr *Pfilename);
AjBool        ajSysFileWhichEnv(AjPStr *Pfilename, char * const env[]);
FILE*         ajSysFuncFdopen(ajint filedes, const char *mode);
char*         ajSysFuncFgets(char *buf, int size, FILE *fp);
FILE*         ajSysFuncFopen(const char *name, const char *flags);
SOCKRET       ajSysFuncSocket(int domain, int type, int protocol);
char*         ajSysFuncStrtokR(const char *s, const char *t,
                               const char **ptrptr,
                               AjPStr *buf);
char*         ajSysFuncStrtok(const char *s, const char *t);
char*         ajSysFuncStrdup(const char *s);
AjBool        ajSysCommandCopyC(const char* filename,
                                const char* newfilename);
AjBool        ajSysCommandCopyS(const AjPStr filename,
                                const AjPStr newfilename);
AjBool        ajSysCommandMakedirC(const char* dirname);
AjBool        ajSysCommandMakedirS(const AjPStr dirname);
AjBool        ajSysCommandRemoveC(const char* filename);
AjBool        ajSysCommandRemoveS(const AjPStr filename);
AjBool        ajSysCommandRemovedirC(const char* dirname);
AjBool        ajSysCommandRemovedirS(const AjPStr dirname);
AjBool        ajSysCommandRenameC(const char* filename,
                                  const char* newfilename);
AjBool        ajSysCommandRenameS(const AjPStr filename,
                                  const AjPStr newfilename);
ajint         ajSysExecC(const char* cl);
ajint         ajSysExecS(const AjPStr clstr);
ajint         ajSysExecLocaleC(const char* cl, const char* localetxt);
ajint         ajSysExecLocaleS(const AjPStr clstr, const AjPStr localestr);
ajint         ajSysExecPathC(const char* cl);
ajint         ajSysExecPathS(const AjPStr clstr);
ajint         ajSysExecEnvC(const char* cls, char * const env[]);
ajint         ajSysExecEnvS(const AjPStr clstr, char * const env[]);
ajint         ajSysExecprogNowaitC(const char *prog,
                                   char * const arg[],
                                   char * const env[]);
ajint         ajSysExecprogNowaitS(const AjPStr progstr,
                                   char * const arg[],
                                   char * const env[]);
ajint         ajSysExecOutnameC(const char* cl, const char* outfnametxt);
ajint         ajSysExecOutnameS(const AjPStr clstr, const AjPStr outfname);
ajint         ajSysExecOutnameAppendC(const char* cl,
                                      const char* outfnametxt);
ajint         ajSysExecOutnameAppendS(const AjPStr clstr,
                                      const AjPStr outfname);
ajint         ajSysExecOutnameErrC(const char* cl, const char* outfnametxt);
ajint         ajSysExecOutnameErrS(const AjPStr clstr, const AjPStr outfname);
ajint         ajSysExecOutnameErrAppendC(const char* cl,
                                         const char* outfnametxt);
ajint         ajSysExecOutnameErrAppendS(const AjPStr clstr,
                                         const AjPStr outfname);
void          ajSysSocketclose(AjOSysSocket sock);

int           ajSysTimeoutSet(AjPSysTimeout ts);
int           ajSysTimeoutUnset(AjPSysTimeout ts);

AjPFile       ajSysCreateInpipeC(const char* commandtxt);
AjPFile       ajSysCreateInpipeS(const AjPStr command);
AjBool        ajSysExecRedirectC(const char *command, int **pipeto,
                                 int **pipefrom);

char          *ajSysGetHomedirFromName(const char *username);
char          *ajSysGetHomedir(void);

/*
** End of prototype definitions
*/




#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif /* AJ_COMPILE_DEPRECATED_BOOK */

#ifdef AJ_COMPILE_DEPRECATED

__deprecated ajint         ajSysExecProgArgEnvNowaitC(const char *prog,
                                                      char * const arg[],
                                                      char * const env[]);
__deprecated ajint         ajSysExecProgArgEnvNowaitS(const AjPStr progstr,
                                                      char * const arg[],
                                                      char * const env[]);
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
__deprecated void          ajSysArgListFree(char*** arglist);
__deprecated AjBool        ajSysIsDirectory(const char *s);
__deprecated AjBool        ajSysIsRegular(const char *s);
__deprecated AjBool        ajSysArglist(const AjPStr cmdline,
                                        char** Pname, char*** PParglist);
__deprecated AjBool        ajSysArglistBuild(const AjPStr cmdline,
                                             char** Pname, char*** PParglist);
__deprecated char          ajSysItoC(ajint v);
__deprecated unsigned char ajSysItoUC(ajint v);
__deprecated AjBool        ajSysUnlink(const AjPStr s);
__deprecated AjBool        ajSysFileUnlink(const AjPStr s);

__deprecated void          ajSysSystem(const AjPStr cl);
__deprecated void          ajSysSystemEnv(const AjPStr cl, char * const env[]);
__deprecated void          ajSysSystemOut(const AjPStr cl,
                                          const AjPStr outfname);

#endif /* AJ_COMPILE_DEPRECATED */




AJ_END_DECLS

#endif /* !AJSYS_H */

/******************************************************************************
** @source AJAX system functions
**
** Copyright (c) Alan Bleasby 1999
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

#ifndef WIN32
#ifndef __VMS
#include <termios.h>
#endif
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>
#else
#include "win32.h"
#endif


static AjPStr sysTname = NULL;
static AjPStr sysFname = NULL;
static AjPStr sysTokRets = NULL;
static AjPStr sysTokSou  = NULL;
static const char *sysTokp = NULL;
static AjPStr sysUserPath = NULL;

/* @filesection ajutil ********************************************************
**
** @nam1rule aj   Function belongs to the AJAX library.
**
*/

/* @datasection [none] System utility functions *******************************
**
** @nam2rule Sys  Function belongs to the AJAX ajsys library.
**
*/

/* @section Argument list manipulation functions ******************************
**
** Function for manipulating argument list.
**
** @fdata [none]
**
** @nam3rule  Arglist      Function for manipulating argument list.
** @nam4rule  ArglistBuild Generates a program name and argument list from a 
**                         command line string.
** @nam4rule  ArglistFree   Free memory in an argument list allocated by 
**                         ajSysArglistBuild.
** 
** @argrule Build   cmdline   [const AjPStr] Original command line
** @argrule Build   Pname     [char**] Returned program name
** @argrule Arglist PParglist [char***] Returns argument array
**
** @valrule Build [AjBool] True on success
** @valrule Free [void]
**
** @fcategory misc
**
******************************************************************************/

/* @func ajSysArglistBuild ****************************************************
**
** Generates a program name and argument list from a command line string.
**
** @param [r] cmdline [const AjPStr] Command line.
** @param [w] Pname [char**] Program name.
** @param [w] PParglist [char***] Argument list.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajSysArglistBuild(const AjPStr cmdline, char** Pname, char*** PParglist)
{    
    static AjPRegexp argexp = NULL;
    AjPStr tmpline          = NULL;
    const char* cp;
    ajint ipos = 0;
    ajint iarg = 0;
    ajint ilen = 0;
    ajint i;
    char** al;
    AjPStr argstr = NULL;
    
    if(!argexp)
	argexp = ajRegCompC("^[ \t]*(\"([^\"]*)\"|'([^']*)'|([^ \t]+))");
    
    ajDebug("ajSysArglistBuild '%S'\n", cmdline);
    
    ajStrAssignS(&tmpline, cmdline);
    
    cp   = ajStrGetPtr(cmdline);
    ipos = 0;
    while(ajRegExecC(argexp, &cp[ipos]))
    {
	ipos += ajRegLenI(argexp, 0);
	iarg++;
    }
    
    AJCNEW(*PParglist, iarg+1);
    al   = *PParglist;
    ipos = 0;
    iarg = 0;
    while(ajRegExecC(argexp, &cp[ipos]))
    {
	ilen = ajRegLenI(argexp, 0);
	ajStrDelStatic(&argstr);
	for(i=2;i<5;i++)
	{
	    if(ajRegLenI(argexp, i))
	    {
		ajRegSubI(argexp, i, &argstr);
		/*ajDebug("parsed [%d] '%S'\n", i, argstr);*/
		break;
	    }
	}
	ipos += ilen;

	if(!iarg)
	    *Pname = ajCharNewS(argstr);

	al[iarg] = ajCharNewS(argstr);
	iarg++;
    }

    al[iarg] = NULL;
    
    ajRegFree(&argexp);
    argexp = NULL;
    ajStrDel(&tmpline);
    ajStrDel(&argstr);
    
    ajDebug("ajSysArglistBuild %d args for '%s'\n", iarg, *Pname);

    return ajTrue;
}


/* @obsolete ajSysArglist
** @rename ajSysArglistBuild
*/

__deprecated AjBool ajSysArglist(const AjPStr cmdline,
				 char** Pname, char*** PParglist)
{    
    return ajSysArglistBuild(cmdline, Pname, PParglist);
}


/* @func ajSysArglistFree *****************************************************
**
** Free memory in an argument list allocated by ajSysArgList
**
** @param [w] PParglist [char***] Argument list.
** @return [void]
** @@
******************************************************************************/

void ajSysArglistFree(char*** PParglist)
{
    char** ca;
    ajint i;

    ca = *PParglist;

    i = 0;
    while(ca[i])
    {
	AJFREE(ca[i]);
	++i;
    }

    AJFREE(*PParglist);

    return;
}


/* @obsolete ajSysArgListFree
** @rename ajSysArglistFree
*/

__deprecated void ajSysArgListFree(char*** PParglist)
{
    ajSysArglistFree(PParglist);
    return;
}

/* @section System cast functions *********************************************
**
** Function for casting one datatype to another.
**
** @fdata [none]
** @fcategory misc
**
** @nam3rule  Cast         Function for casting one datatype to another.
** @nam4rule  CastItoc     Convert Int to Char (for fussy compilers)
** @nam4rule  CastItouc    Convert Int to Unsigned Char (for fussy compilers). 
**
** @argrule * v [ajint] Character as an integer
**
** @valrule *Itoc [char] Character
** @valrule *Itouc [unsigned char] Unsigned character
**
******************************************************************************/

/* @func ajSysCastItoc ********************************************************
**
** Convert Int to Char
** Needed for very fussy compilers i.e. Digital C
**
** @param [r] v [ajint] integer
** @return [char] Character cast
** @@
******************************************************************************/

char ajSysCastItoc(ajint v)
{
    char c;

    c = (char) v;
    return c;
}


/* @obsolete ajSysItoC
** @rename ajSysCastItoC
*/

__deprecated char ajSysItoC(ajint v)
{
    return ajSysCastItoc(v);
}



/* @func ajSysCastItouc *******************************************************
**
** Convert Int to Unsigned Char
** Needed for very fussy compilers i.e. Digital C
**
** @param [r] v [ajint] integer
** @return [unsigned char] Unisigned character cast
** @@
******************************************************************************/

unsigned char ajSysCastItouc(ajint v)
{
    char c;

    c = (unsigned char) v;
    return c;
}


/* @obsolete ajSysItoUC
** @rename ajSysCastItouc
*/

__deprecated unsigned char ajSysItoUC(ajint v)
{
    return ajSysCastItouc(v);
}

/* @section System functions for files ****************************************
**
** System functions for files.
**
** @fdata [none]
** @fcategory misc
**
** @nam3rule  File           System functions for files.
** @nam4rule  FileWhich      Searches $PATH sequentially for a user-EXECUTABLE 
**                           file.
** @nam5rule  FileWhichEnv   Uses environment to extract the PATH list.
** @nam4rule  FileUnlink     Deletes a file or link
** 
** @argrule Unlink filename [const AjPStr] File name
** @argrule Which Pfilename [AjPStr*] File name (updated when found)
** @argrule Env env [char* const[]] File name (updated when found)
** @valrule   *  [AjBool]  True if operation is successful.
**
******************************************************************************/


/* @func ajSysFileUnlink ******************************************************
**
** Deletes a file or link
**
** @param [r] filename [const AjPStr] Filename in AjStr.
** @return [AjBool] true if deleted false otherwise
** @@
******************************************************************************/

AjBool ajSysFileUnlink(const AjPStr filename)
{
    ajDebug("ajSysFileUnlink '%S'\n", filename);

#ifndef WIN32
    if(!unlink(ajStrGetPtr(filename)))
	return ajTrue;
#else
    if(DeleteFile(ajStrGetPtr(filename)))
	return ajTrue;
#endif
    ajDebug("ajSysUnlink failed to delete '%S'\n", filename);
    return ajFalse;
}

/* @obsolete ajSysUnlink
** @rename ajSysFileUnlink
*/

__deprecated AjBool ajSysUnlink(const AjPStr s)
{
    return ajSysFileUnlink(s);
}






/* @func ajSysFileWhich *******************************************************
**
** Gets the Basename of a file then searches $PATH sequentially until it
** finds a user-EXECUTABLE file of the same name.
**
** @param [u] Pfilename [AjPStr*] Filename in AjStr, replaced by full pathname
** @return [AjBool] True if executable found, false otherwise
** @@
******************************************************************************/

AjBool ajSysFileWhich(AjPStr *Pfilename)
{
    char *p;

    if(!ajStrGetLen(sysUserPath))
        ajStrAssignC(&sysUserPath, getenv("PATH"));

    if(!ajStrGetLen(sysUserPath))
        return ajFalse;

    p = ajStrGetuniquePtr(&sysUserPath);

    if(!ajNamGetValueS(*Pfilename, &sysTname))
        ajStrAssignS(&sysTname, *Pfilename);

    if(ajFilenameExistsExec(sysTname))
    {
        ajStrAssignS(Pfilename,sysTname);
        ajStrDelStatic(&sysTname);
        return ajTrue;
    }
    
    if(!sysFname)
	sysFname = ajStrNew();

    p=ajSysFuncStrtok(p,PATH_SEPARATOR);

    if(p==NULL)
    {
	ajStrDelStatic(&sysFname);
	ajStrDelStatic(&sysTname);
	return ajFalse;
    }


    while(1)
    {
	ajFmtPrintS(&sysFname,"%s%s%S",p,SLASH_STRING,sysTname);

	if(ajFilenameExistsExec(sysFname))
	{
	    ajStrAssignS(Pfilename,sysFname);
	    break;
	}

	if((p = ajSysFuncStrtok(NULL,PATH_SEPARATOR))==NULL)
        {
	    ajStrDelStatic(&sysFname);
	    ajStrDelStatic(&sysTname);
	    return ajFalse;
        }
    }

    ajStrDelStatic(&sysFname);
    ajStrDelStatic(&sysTname);

    return ajTrue;
}


/* @obsolete ajSysWhich
** @rename ajSysFileWhich
*/

__deprecated AjBool ajSysWhich(AjPStr *s)
{
    return ajSysFileWhich(s);
}


/* @func ajSysFileWhichEnv ****************************************************
**
** Gets the Basename of a file then searches $PATH sequentially until it
** finds a user-EXECUTABLE file of the same name. Reentrant.
**
** @param [u] Pfilename [AjPStr*] Filename in AjStr, replaced by full pathname
** @param [r] env [char* const[]] Environment
** @return [AjBool] True if executable found, false otherwise
** @@
******************************************************************************/

AjBool ajSysFileWhichEnv(AjPStr *Pfilename, char * const env[])
{
    ajint count;
    char *p = NULL;
    const char *cp;
    AjPStr tname = NULL;
    AjPStr fname = NULL;
    AjPStr path  = NULL;
    const char   *save = NULL;
    AjPStr buf   = NULL;
    AjPStr tmp   = NULL;
    
    
    buf   = ajStrNew();
    tname = ajStrNew();
    tmp   = ajStrNew();
    ajStrAssignS(&tname,*Pfilename);
    
    fname = ajStrNew();
    path  = ajStrNew();
    
    ajFilenameTrimPath(&tname);

#ifdef WIN32
    ajStrAppendC(&tname,".exe");
#endif

    ajDebug("ajSysFileWhichEnv '%S' => %S\n", *Pfilename, tname);

    count = 0;
    while(env[count]!=NULL)
    {
	if(!(*env[count]))
	    break;

	/*ajDebug("  env[%d] '%s'\n", count, env[count]);*/

#ifndef WIN32
	if(!strncmp("PATH=",env[count],5))
#else
	if(!strnicmp("PATH=",env[count],5))
#endif
	    break;

	++count;
    }
    
   /* ajDebug("PATH  env[%d] '%s'\n", count, env[count]);*/

    if(env[count]==NULL || !(*env[count]))
    {
	ajStrDel(&fname);
	ajStrDel(&tname);
	ajStrDel(&path);
	ajStrDel(&buf);
	ajStrDel(&tmp);
	return ajFalse;
    }
    
    ajStrAssignC(&path, env[count]);
    cp = ajStrGetPtr(path);
    cp += 5;
    ajStrAssignC(&tmp,cp);

    /*ajDebug("tmp '%S' save '%S' buf '%S'\n", tmp, save, buf);*/
 
    p = ajSysFuncStrtokR(ajStrGetuniquePtr(&tmp),PATH_SEPARATOR,&save,&buf);

    if(p==NULL)
    {
	ajStrDel(&fname);
	ajStrDel(&tname);
	ajStrDel(&path);
	ajStrDel(&buf);
	ajStrDel(&tmp);
	return ajFalse;
    }
    

    ajFmtPrintS(&fname,"%s%s%S",p,SLASH_STRING,tname);

    while(!ajFilenameExistsExec(fname))
    {
	if((p = ajSysFuncStrtokR(NULL,PATH_SEPARATOR,&save,&buf))==NULL)
	{
	    ajStrDel(&fname);
	    ajStrDel(&tname);
	    ajStrDel(&path);
	    ajStrDel(&buf);
	    ajStrDel(&tmp);
	    return ajFalse;
	}

	ajFmtPrintS(&fname,"%s%s%S",p,SLASH_STRING,tname);
    }
    
    
    ajStrAssignS(Pfilename,fname);
    ajDebug("ajSysFileWhichEnv returns '%S'\n", *Pfilename);

    ajStrDel(&fname);
    ajStrDel(&tname);
    ajStrDel(&path);
    ajStrDel(&buf);
    ajStrDel(&tmp);
    
    return ajTrue;
}



/* @obsolete ajSysWhichEnv
** @rename ajSysFileWhichEnv
*/

__deprecated AjBool ajSysWhichEnv(AjPStr *Pfilename, char * const env[])
{
    return ajSysFileWhichEnv(Pfilename, env);
}


/* @section Wrappers to C functions *******************************************
**
** Functions for calling or substituting C-functions.
**
** @fdata [none]
** @fcategory misc
**
** @nam3rule  Func         Replacement for C-function.
** @nam4rule  FuncStrtok   strtok that doesn't corrupt the source string
** @nam5rule  FuncStrtokR  Reentrant version.
** @nam4rule  FuncFgets    An fgets replacement that will cope with Mac OSX
**                          files
** @nam4rule  FuncFopen    An fopen replacement to cope with cygwin and windows
** @nam4rule  FuncFdopen   Calls non-ANSI fdopen.
** @nam4rule  FuncStrdup   Duplicate BSD strdup function for very strict ANSI 
** @nam3rule  System       Execute a command line as if from the C shell
** @nam4rule  SystemEnv    Execute command line and pass the environment 
**                         received from main to extract the PATH list
** @nam4rule  SystemOut    Execute command line and write standard output to
**                         a named file
**
** @argrule Fdopen filedes [ajint] file descriptor
** @argrule Fdopen mode [const char*] file mode
** @argrule Fgets buf [char*] buffer
** @argrule Fgets size [int] maximum length to read
** @argrule Fgets fp [FILE*] stream
** @argrule Fopen name [const char*] Name of file to open
** @argrule Fopen flags [const char*] Read/write/append flags
** @argrule Strdup dupstr [const char*] String to duplicate
** @argrule Strtok srcstr [const char*] source string
** @argrule Strtok delimstr [const char*] delimiter string
** @argrule R ptrptr [const char**] Saved pointer
** @argrule R buf [AjPStr*] Independent buffer provided by caller
** @argrule System cmdline [const AjPStr] The command line
** @argrule SystemEnv env [char* const[]] Environment variables and values
** @argrule SystemOut outfname [const AjPStr] The output file name
**
** @valrule Fdopen [FILE*] C open file
** @valrule Fgets [char*] Buffer on success
** @valrule Fopen [FILE*] C open file
** @valrule Strdup [char*] New string
** @valrule Strtok [char*] New string
** @valrule System [void]
**
******************************************************************************/


/* @func ajSysFuncFdopen ******************************************************
**
** Place non-ANSI fdopen here
**
** @param [r] filedes [ajint] file descriptor
** @param [r] mode [const char *] file mode
** @return [FILE*] file pointer
** @@
******************************************************************************/

FILE* ajSysFuncFdopen(ajint filedes, const char *mode)
{
    FILE *ret;
    
    ret = fdopen(filedes,mode);
    if(ret)
        errno = 0;              /* set to "Illegal seek" on some systems */

    return ret;
}


/* @obsolete ajSysFdopen
** @rename ajSysFuncFdopen
*/

__deprecated FILE* ajSysFdopen(ajint filedes, const char *mode)
{
    return ajSysFuncFdopen(filedes, mode);
}

/* @func ajSysFuncFgets *******************************************************
**
** An fgets replacement that will cope with Mac OSX <CR> files
**
** @param [w] buf [char*] buffer
** @param [r] size [int] maximum length to read
** @param [u] fp [FILE*] stream
**
** @return [char*] buf or NULL
** @@
******************************************************************************/

char* ajSysFuncFgets(char *buf, int size, FILE *fp)
{
#ifdef __ppc__
    int c = 0;
    char *p;
    int cnt;

    p = buf;
    if(!size || size<0)
        return NULL;

    cnt = 0;

    while(cnt!=size-1)
    {
	c = getc(fp);
	if(c==EOF || c=='\r' || c=='\n')
	    break;
        *(p++) = c;
        ++cnt;
    }


    *p ='\0';

    if(c==EOF && !cnt)
	return NULL;

    if(cnt == size-1)
        return buf;

    if(c=='\r' || c=='\n')
    {
	if(c=='\r' && cnt<size-2)
	{
	    if((c=getc(fp)) == '\n')
		*(p++) = '\r';
	    else
		ungetc(c,fp);
	}
	*(p++) = '\n';
    }

    *p = '\0';

    return buf;
#else
    return fgets(buf,size,fp);
#endif
}

/* @obsolete ajSysFgets
** @rename ajSysFuncFgets
*/

__deprecated char* ajSysFgets(char *buf, int size, FILE *fp)
{
    return ajSysFuncFgets(buf, size, fp);
}

/* @func ajSysFuncFopen *******************************************************
**
** An fopen replacement to cope with cygwin and windows
**
** @param [r] name [const char *] file to open
** @param [r] flags [const char*] r/w/a flags
**
** @return [FILE*] file or NULL
** @@
******************************************************************************/

FILE* ajSysFuncFopen(const char *name, const char *flags)
{
    FILE   *ret  = NULL;
#ifdef __CYGWIN__
    AjPStr fname = NULL;
#endif
    
#ifdef __CYGWIN__
    if(*(name+1) == ':')
    {
	fname = ajStrNew();
	ajFmtPrintS(&fname,"/cygdrive/%c/%s",*name,name+2);
	ret = fopen(ajStrGetPtr(fname),flags);
	ajStrDel(&fname);
    }
    else
      ret = fopen(name,flags);
#else
	ret = fopen(name,flags);
#endif

	return ret;
}


/* @obsolete ajSysFopen
** @rename ajSysFuncFopen
*/

__deprecated FILE* ajSysFopen(const char *name, const char *flags)
{
    return ajSysFuncFopen(name, flags);
}


/* @func ajSysFuncStrdup ******************************************************
**
** Duplicate BSD strdup function for very strict ANSI compilers
**
** @param [r] dupstr [const char *] string to duplicate
** @return [char*] Text string as for strdup
** @@
******************************************************************************/

char* ajSysFuncStrdup(const char *dupstr)
{
    static char *p;

    AJCNEW(p, strlen(dupstr)+1);
    strcpy(p,dupstr);

    return p;
}

/* @obsolete ajSysStrdup
** @rename ajSysFuncStrdup
*/
__deprecated char* ajSysStrdup(const char *s)
{
    return ajSysFuncStrdup(s);
}

/* @func ajSysFuncStrtok ******************************************************
**
** strtok that doesn't corrupt the source string
**
** @param [r] srcstr [const char *] source string
** @param [r] delimstr [const char *] delimiter string
**
** @return [char*] pointer or NULL when nothing is found
** @@
******************************************************************************/

char* ajSysFuncStrtok(const char *srcstr, const char *delimstr)
{
    ajint len;

    if(srcstr)
    {
	if(!sysTokRets)
	{
	    sysTokSou  = ajStrNew();
	    sysTokRets = ajStrNew();
	}
	ajStrAssignC(&sysTokSou,srcstr);
	sysTokp = ajStrGetPtr(sysTokSou);
    }

    if(!*sysTokp)
	return NULL;

    len = strspn(sysTokp,delimstr);		/* skip over delimiters */
    sysTokp += len;
    if(!*sysTokp)
	return NULL;

    len = strcspn(sysTokp,delimstr);		/* count non-delimiters */
    ajStrAssignSubC(&sysTokRets,sysTokp,0,len-1);
    sysTokp += len;		  /* skip over first delimiter only */

    return ajStrGetuniquePtr(&sysTokRets);
}


/* @obsolete ajSysStrtok
** @rename ajSysFuncStrtok
*/

__deprecated char* ajSysStrtok(const char *s, const char *t)
{
    return ajSysFuncStrtok(s, t);
}

/* @func ajSysFuncStrtokR *****************************************************
**
** Reentrant strtok that doesn't corrupt the source string.
** This function uses a string buffer provided by the caller.
**
** @param [u] srcstr  [const char *] source string
** @param [r] delimstr [const char *] delimiter string
** @param [u] ptrptr [const char **] ptr save
** @param [w] buf [AjPStr *] result buffer
**
** @return [char*] pointer or NULL
** @@
******************************************************************************/

char* ajSysFuncStrtokR(const char *srcstr, const char *delimstr,
		       const char **ptrptr,AjPStr *buf)
{
    const char *p;
    ajint len;

    if(!*buf)
	*buf = ajStrNew();

    if(srcstr!=NULL)
	p = srcstr;
    else
	p = *ptrptr;

    if(!*p)
	return NULL;

    len = strspn(p,delimstr);			/* skip over delimiters */
    p += len;
    if(!*p)
	return NULL;

    len = strcspn(p,delimstr);			/* count non-delimiters */
    ajStrAssignSubC(buf,p,0,len-1);
    p += len;			       /* skip to first delimiter */

    *ptrptr = p;

    return ajStrGetuniquePtr(buf);
}

/* @obsolete ajSysStrtokR
** @rename ajSysFuncStrtokR
*/


__deprecated char* ajSysStrtokR(const char *s, const char *t,
				const char **ptrptr,
				AjPStr *buf)
{
    return ajSysFuncStrtokR(s, t, ptrptr, buf);
}

/* @func ajSysSystem **********************************************************
**
** Exec a command line as if from the C shell
**
** The exec'd program is passed a new argv array in argptr
**
** @param [r] cmdline [const AjPStr] The command line
** @return [void]
** @@
******************************************************************************/

void ajSysSystem(const AjPStr cmdline)
{
#ifndef WIN32
    pid_t pid;
    pid_t retval;
    ajint status;
    char *pgm;
    char **argptr;
    ajint i;

    AjPStr pname = NULL;

    if(!ajSysArglistBuild(cmdline, &pgm, &argptr))
	return;

    pname = ajStrNew();

    ajStrAssignC(&pname, pgm);

    if(!ajSysFileWhich(&pname))
	ajFatal("cannot find program '%S'", pname);

    pid=fork();

    if(pid==-1)
	ajFatal("System fork failed");

    if(pid)
    {
	while((retval=waitpid(pid,&status,0))!=pid)
	{
	    if(retval == -1)
		if(errno != EINTR)
		    break;
	}
    }
    else
    {
	execv(ajStrGetPtr(pname), argptr);
	ajExitAbort();			/* just in case */
    }

    ajStrDel(&pname);

    i = 0;
    while(argptr[i])
    {
	AJFREE(argptr[i]);
	++i;
    }
    AJFREE(argptr);

    AJFREE(pgm);

#endif
    return;
}


/* @obsolete ajSystem
** @rename ajSysSystem
*/

__deprecated void ajSystem(const AjPStr cl)
{
    ajSysSystem(cl);
    return;
}

/* @func ajSysSystemEnv *******************************************************
**
** Exec a command line as if from the C shell
**
** This routine must be passed the environment received from
** main(ajint argc, char **argv, char **env)
** The environment is used to extract the PATH list (see ajWhich)
**
** Note that the environment is passed through unaltered. The exec'd
** program is passed a new argv array
**
** @param [r] cmdline [const AjPStr] The command line
** @param [r] env [char* const[]] The environment
** @return [void]
** @@
******************************************************************************/

void ajSysSystemEnv(const AjPStr cmdline, char * const env[])
{
#ifndef WIN32
    pid_t pid;
    pid_t retval;
    ajint status;
    char *pgm = NULL;
    char **argptr = NULL;
    ajint i;

    AjPStr pname = NULL;

    if(!ajSysArglistBuild(cmdline, &pgm, &argptr))
	return;

    pname = ajStrNew();

    ajDebug("ajSysSystemEnv '%s' %S \n", pgm, cmdline);
    ajStrAssignC(&pname, pgm);
    if(!ajSysFileWhichEnv(&pname, env))
	ajFatal("cannot find program '%S'", pname);

    ajDebug("ajSysSystemEnv %S = %S\n", pname, cmdline);
    for (i=0;argptr[i]; i++)
    {
	ajDebug("%4d '%s'\n", i, argptr[i]);
    }

    pid = fork();
    if(pid==-1)
	ajFatal("System fork failed");

    if(pid)
    {
	while((retval=waitpid(pid,&status,0))!=pid)
	{
	    if(retval == -1)
		if(errno != EINTR)
		    break;
	}
    }
    else
    {
	execve(ajStrGetPtr(pname), argptr, env);
	ajExitAbort();			/* just in case */
    }

    ajStrDel(&pname);

    i = 0;
    while(argptr[i])
    {
	AJFREE(argptr[i]);
	++i;
    }
    AJFREE(argptr);

    AJFREE(pgm);

#endif

    return;
}

/* @obsolete ajSystemEnv
** @rename ajSysSystemEnv
*/

__deprecated void ajSystemEnv(const AjPStr cl, char * const env[])
{
    ajSysSystemEnv(cl, env);
    return;
}


/* @func ajSysSystemOut *******************************************************
**
** Exec a command line as if from the C shell with standard output redirected
** to a named file
**
** The exec'd program is passed a new argv array in argptr
**
** @param [r] cmdline [const AjPStr] The command line
** @param [r] outfname [const AjPStr] The output file name
** @return [void]
** @@
******************************************************************************/

void ajSysSystemOut(const AjPStr cmdline, const AjPStr outfname)
{
#ifndef WIN32
    pid_t pid;
    pid_t retval;
    ajint status;
    char *pgm;
    char **argptr;
    ajint i;

    AjPStr pname = NULL;

    if(!ajSysArglistBuild(cmdline, &pgm, &argptr))
	return;

    pname = ajStrNew();

    ajStrAssignC(&pname, pgm);

    if(!ajSysFileWhich(&pname))
	ajFatal("cannot find program '%S'", pname);

    fflush(stdout);

    pid=fork();

    if(pid==-1)
	ajFatal("System fork failed");

    if(pid)
    {
	while((retval=waitpid(pid,&status,0))!=pid)
	{
	    if(retval == -1)
		if(errno != EINTR)
		    break;
	}
    }
    else
    {
	/* this is the child process */

	if(!freopen(MAJSTRGETPTR(outfname), "wb", stdout))
	    ajErr("Failed to redirect standard output to '%S'", outfname);
	execv(ajStrGetPtr(pname), argptr);
	ajExitAbort();			/* just in case */
    }

    ajStrDel(&pname);

    i = 0;
    while(argptr[i])
    {
	AJFREE(argptr[i]);
	++i;
    }
    AJFREE(argptr);

    AJFREE(pgm);

#endif
    return;
}

/* @obsolete ajSystemOut
** @rename ajSysSystemOut
*/

__deprecated void ajSystemOut(const AjPStr cl, const AjPStr outfname)
{
    ajSysSystemOut(cl, outfname);
    return;
}

/* @section Miscellaneous system functions ************************************
**
** Miscellaneous system functions
**
** @fdata [none]
** @fcategory misc
**
** @nam3rule  Canon   Sets or unsets TTY canonical mode
** @nam3rule  Exit    Cleans up system internals memory
** 
** @argrule Canon state [AjBool] Canonical mode set if true
** @valrule   *  [void]
**
******************************************************************************/

/* @func ajSysCanon  **********************************************************
**
** Sets or unsets TTY canonical mode
**
** @param [r] state [AjBool] state=true sets canon state=false sets noncanon
** @return [void]
** @@
******************************************************************************/

void ajSysCanon(AjBool state)
{
#ifndef WIN32
#ifndef __VMS
    static struct termios tty;


    tcgetattr(1, &tty);
    tty.c_cc[VMIN]  = '\1';
    tty.c_cc[VTIME] = '\0';
    tcsetattr(1,TCSANOW,&tty);

    if(state)
	tty.c_lflag |= ICANON;
    else
	tty.c_lflag &= ~(ICANON);

    tcsetattr(1, TCSANOW, &tty);
#endif
#endif
    return;
}

/* @func ajSysExit ************************************************************
**
** Cleans up system internals memory
**
** @return [void]
** @@
******************************************************************************/

void ajSysExit(void)
{
    ajStrDel(&sysFname);
    ajStrDel(&sysTname);
    ajStrDel(&sysTokSou);
    ajStrDel(&sysTokRets);
    ajStrDel(&sysUserPath);

    return;
}





/* @obsolete ajSysBasename
** @rename ajFileNameTrim
*/

__deprecated void ajSysBasename(AjPStr *s)
{
    ajFilenameTrimPath(s);

    return;
}




/* @obsolete ajSysIsDirectory
** @remove use ajFileDir instead
*/

__deprecated AjBool ajSysIsDirectory(const char *s)
{
    AjBool ret;
    AjPStr tmpstr = NULL;
    tmpstr = ajStrNewC(s);

    ret = ajDirnameFixExists(&tmpstr);
    ajStrDel(&tmpstr);

    return ret;
}




/* @obsolete ajSysIsRegular
** @remove use ajFileNameValid instead
*/

__deprecated AjBool ajSysIsRegular(const char *s)
{
    AjBool ret;
    AjPStr tmpstr;

    tmpstr = ajStrNewC(s);

    ret = ajFilenameExistsRead(tmpstr);
    ajStrDel(&tmpstr);

    return ret;
}





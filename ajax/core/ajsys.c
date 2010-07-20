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
#include <sys/file.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>
#include <locale.h>
#include <pwd.h>
#include <fcntl.h>
#else
#include "win32.h"
#include <direct.h>
#define open     _open
#define close    _close
#define read     _read
#define write    _write
#define strnicmp _strnicmp
#define fdopen   _fdopen
#define rmdir    _rmdir

#pragma warning(disable:4142)    /* benign redefinition of type */
#include <ShlObj.h>
#include <Sddl.h>
#endif


static AjPStr sysTname = NULL;
static AjPStr sysFname = NULL;
static AjPStr sysTokRets = NULL;
static AjPStr sysTokSou  = NULL;
static const char *sysTokp = NULL;
static AjPStr sysUserPath = NULL;

#ifndef WIN32
static void sysTimeoutAbort(int sig);
#else
static void CALLBACK sysTimeoutAbort(LPVOID arg, DWORD low, DWORD high);
#endif




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
** @suffix    C            Accept C character string parameters
** @suffix    S            Accept string object parameters
** 
** @argrule C   cmdline   [const char*] Original command line
** @argrule S   cmdline   [const AjPStr] Original command line
** @argrule Build   Pname     [char**] Returned program name
** @argrule Arglist PParglist [char***] Returns argument array
**
** @valrule Build [AjBool] True on success
** @valrule Free [void]
**
** @fcategory misc
**
******************************************************************************/




/* @func ajSysArglistBuildC ****************************************************
**
** Generates a program name and argument list from a command line string.
**
** @param [r] cmdline [const char*] Command line.
** @param [w] Pname [char**] Program name.
** @param [w] PParglist [char***] Argument list.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajSysArglistBuildC(const char* cmdline, char** Pname, char*** PParglist)
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
    
    ajDebug("ajSysArglistBuildC '%s'\n", cmdline);
    
    ajStrAssignC(&tmpline, cmdline);
    
    cp   = cmdline;
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
		ajDebug("parsed [%d] '%S'\n", i, argstr);
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
    
    ajDebug("ajSysArglistBuildC %d args for '%s'\n", iarg, *Pname);

    return ajTrue;
}




/* @func ajSysArglistBuildS ****************************************************
**
** Generates a program name and argument list from a command line string.
**
** @param [r] cmdline [const AjPStr] Command line.
** @param [w] Pname [char**] Program name.
** @param [w] PParglist [char***] Argument list.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajSysArglistBuildS(const AjPStr cmdline, char** Pname, char*** PParglist)
{
    return ajSysArglistBuildC(MAJSTRGETPTR(cmdline), Pname, PParglist);
}




/* @obsolete ajSysArglistBuild
** @rename ajSysArglistBuildS
*/

__deprecated AjBool ajSysArglistBuild(const AjPStr cmdline,
                                      char** Pname, char*** PParglist)
{    
    return ajSysArglistBuildS(cmdline, Pname, PParglist);
}




/* @obsolete ajSysArglist
** @rename ajSysArglistBuildS
*/

__deprecated AjBool ajSysArglist(const AjPStr cmdline,
				 char** Pname, char*** PParglist)
{    
    return ajSysArglistBuildS(cmdline, Pname, PParglist);
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
** @return [unsigned char] Unsigned character cast
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
** @nam4rule  FileRmrf       Recursively deletes a directory tree 
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




/* @func ajSysFileRmrfC ******************************************************
**
** Forcibly delete a directory tree
**
** @param [r] path [const char*] Directory path
** @return [AjBool] true if deleted false otherwise
** @@
******************************************************************************/

AjBool ajSysFileRmrfC(const char *path)
{
    AjPList flist = NULL;

    AjPStr wild  = NULL;
    AjPStr fname = NULL;
    AjPStr dirpath = NULL;
    const char *pdir = NULL;
    
    AjBool ret;


    if(ajCharMatchC(path,".") || ajCharMatchC(path,".."))
        return ajFalse;
    
    flist =  ajListNew();
    wild  =  ajStrNewC("*");

    dirpath = ajStrNewC(path);

    ret = ajTrue;
    
    if(!ajFilenameExistsDir(dirpath))
    {
        ajListFree(&flist);
        ajStrDel(&wild);
        ajStrDel(&dirpath);

        return ajFalse;
    }

    ajFilelistAddPathWildDir(flist, dirpath, wild);
    
    while(ajListPop(flist, (void **) &fname))
    {
        if(ajFilenameExistsDir(fname))
        {
            pdir = ajStrGetPtr(fname);
            ret = ajSysFileRmrfC(pdir);

            if(!ret)
                break;

        }
        else
        {
            ret = ajSysFileUnlinkS(fname);

            if(!ret)
                break;
        }

        ajStrDel(&fname);
    }

    if(!(ajCharMatchC(path,".") || ajCharMatchC(path,"..")))
        if(rmdir(path))
            ret  = ajFalse;
    
    while(ajListPop(flist, (void **) &fname))
        ajStrDel(&fname);

    ajStrDel(&wild);
    ajStrDel(&dirpath);

    ajListFree(&flist);
    
    return ret;
}




/* @func ajSysFileUnlinkC ******************************************************
**
** Deletes a file or link
**
** @param [r] filename [const char*] Filename in AjStr.
** @return [AjBool] true if deleted false otherwise
** @@
******************************************************************************/

AjBool ajSysFileUnlinkC(const char* filename)
{
    ajDebug("ajSysFileUnlinkC '%s'\n", filename);

#ifndef WIN32
    if(!unlink(filename))
	return ajTrue;

    ajErr("File '%s' remove failed, error:%d '%s'", filename,
          errno, strerror(errno));
#else
    if(DeleteFile(filename))
	return ajTrue;
#endif
    ajDebug("ajSysFileUnlinkC failed to delete '%s'\n", filename);

    return ajFalse;
}




/* @func ajSysFileUnlinkS ******************************************************
**
** Deletes a file or link
**
** @param [r] filename [const AjPStr] Filename in AjStr.
** @return [AjBool] true if deleted false otherwise
** @@
******************************************************************************/

AjBool ajSysFileUnlinkS(const AjPStr filename)
{
    return ajSysFileUnlinkC(MAJSTRGETPTR(filename));
}




/* @obsolete ajSysFileUnlink
** @rename ajSysFileUnlinkS
*/

__deprecated AjBool ajSysFileUnlink(const AjPStr s)
{
    return ajSysFileUnlinkS(s);
}




/* @obsolete ajSysUnlink
** @rename ajSysFileUnlinkS
*/

__deprecated AjBool ajSysUnlink(const AjPStr s)
{
    return ajSysFileUnlinkS(s);
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
#if !defined(WIN32) && !defined(__CYGWIN__)
	ajFmtPrintS(&sysFname,"%s%s%S",p,SLASH_STRING,sysTname);
#else
	ajFmtPrintS(&sysFname,"%s%s%S.exe",p,SLASH_STRING,sysTname);
#endif
        
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
** @nam3rule  Command          Execute the equivalent of a command
** @nam4rule  CommandClear     Execute a clear screen command
** @nam4rule  CommandCopy      Execute a copy command
** @nam4rule  CommandMakedir   Execute a make directory command
** @nam4rule  CommandRemove    Execute a remove file command
** @nam4rule  CommandRemovedir Execute a remove directory command
** @nam4rule  CommandRename    Execute a fle rename command
** @nam3rule  Func         Replacement for C-function.
** @nam4rule  FuncSocket   A socket function for UNIX and Windows
** @nam4rule  FuncStrtok   strtok that doesn't corrupt the source string
** @nam5rule  FuncStrtokR  Reentrant version.
** @nam4rule  FuncFgets    An fgets replacement that will cope with Mac OSX
**                          files
** @nam4rule  FuncFopen    An fopen replacement to cope with cygwin and windows
** @nam4rule  FuncFdopen   Calls non-ANSI fdopen.
** @nam4rule  FuncSocket   A socket function fore UNIX and Windows
** @nam4rule  FuncStrdup   Duplicate BSD strdup function for very strict ANSI 
** @nam3rule  System       Execute a command line as if from the C shell
** @nam4rule  SystemEnv    Execute command line and pass the environment 
**                         received from main to extract the PATH list
** @nam4rule  SystemOut    Execute command line and write standard output to
**                         a named file
** @suffix    C            Accept CC character string parameters
** @suffix    S            Accept string object parameters
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




/* @func ajSysCommandCopyC **************************************************
**
** Copy a file
**
** @param [r] name  [const char*] Source filename
** @param [r] name2 [const char*] Target filename
** @return [AjBool] True on success
**
******************************************************************************/

AjBool ajSysCommandCopyC(const char* name, const char* name2)
{
    int from;
    int to;
    int n;
    char buf[1024];

    from = open(name, O_RDONLY);
    if(from < 0)
    {
        ajErr("Unable to copy '%s' error %d: %s",
              name, errno, strerror(errno));
        return ajFalse;
    }
    
    to = open(name2, O_WRONLY|O_CREAT, 0644);
    if(to < 0)
    {
        ajErr("Unable to copy to '%s' error %d: %s",
              name2, errno, strerror(errno));
        return ajFalse;
    }

    while((n = read(from, buf, sizeof(buf))) > 0)
        write(to, buf, n);

    close(from);
    close(to);

    return ajTrue;
}




/* @func ajSysCommandCopyS **************************************************
**
** Copy a file
**
** @param [r] strname  [const AjPStr] Source filename
** @param [r] strname2 [const AjPStr] Target filename
** @return [AjBool] True on success
**
******************************************************************************/

AjBool ajSysCommandCopyS(const AjPStr strname, const AjPStr strname2)
{
    return ajSysCommandCopyC(MAJSTRGETPTR(strname), MAJSTRGETPTR(strname2));
}




/* @func ajSysCommandMakedirC *************************************************
**
** Delete a file
**
** @param [r] name [const char*] Directory
** @return [AjBool] True on success
**
******************************************************************************/

AjBool ajSysCommandMakedirC(const char* name)
{
#ifndef WIN32
    if(!mkdir(name, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH))
        return ajTrue;
#else
    if(!_mkdir(name))
        return ajTrue;
#endif

    ajErr("Unable to make directory '%s' (%d): %s",
          name, errno, strerror(errno));

    return ajFalse;
}




/* @func ajSysCommandMakedirS *************************************************
**
** Delete a file
**
** @param [r] strname [const AjPStr] Directory
** @return [AjBool] True on success
**
******************************************************************************/

AjBool ajSysCommandMakedirS(const AjPStr strname)
{
    return ajSysCommandMakedirC(MAJSTRGETPTR(strname));
}




/* @func ajSysCommandRemoveC **************************************************
**
** Delete a file
**
** @param [r] name [const char*] Filename
** @return [AjBool] True on success
**
******************************************************************************/

AjBool ajSysCommandRemoveC(const char* name)
{
    return ajSysFileUnlinkC(name);
}




/* @func ajSysCommandRemoveS **************************************************
**
** Delete a file
**
** @param [r] strname [const AjPStr] Filename
** @return [AjBool] True on success
**
******************************************************************************/

AjBool ajSysCommandRemoveS(const AjPStr strname)
{
    return ajSysFileUnlinkC(MAJSTRGETPTR(strname));
}




/* @func ajSysCommandRemovedirC ***********************************************
**
** Delete a file
**
** @param [r] name [const char*] Directory
** @return [AjBool] True on success
**
******************************************************************************/

AjBool ajSysCommandRemovedirC(const char* name)
{
    AjPStr cmdstr = NULL;
    AjBool ret;

    ret = ajTrue;
    
    cmdstr = ajStrNewC(name);
    if(!ajFilenameExistsDir(cmdstr))
    {
        if(!ajFilenameExists(cmdstr))
            ajErr("Unable to remove directory '%S' not found", cmdstr);
        else
            ajErr("Unable to remove directory '%S' not a directory", cmdstr);

        return ajFalse;
    }
    
    ret = ajSysFileRmrfC(name);

    ajStrDel(&cmdstr);

    return ret;
}




/* @func ajSysCommandRemovedirS ***********************************************
**
** Delete a file
**
** @param [r] strname [const AjPStr] Directory
** @return [AjBool] True on success
**
******************************************************************************/

AjBool ajSysCommandRemovedirS(const AjPStr strname)
{
    return ajSysCommandRemovedirC(MAJSTRGETPTR(strname));
}




/* @func ajSysCommandRenameC ************************************************
**
** Rename a file
**
** @param [r] name  [const char*] Source filename
** @param [r] name2 [const char*] Target filename
** @return [AjBool] True on success
**
******************************************************************************/

AjBool ajSysCommandRenameC(const char* name, const char* name2)
{
    if(!rename(name, name2))
        return ajTrue;

    ajErr("File rename failed (%d): %s",
          errno, strerror(errno));

    return ajFalse;
}




/* @func ajSysCommandRenameS **************************************************
**
** Rename a file
**
** @param [r] strname  [const AjPStr] Source filename
** @param [r] strname2 [const AjPStr] Target filename
** @return [AjBool] True on success
**
******************************************************************************/

AjBool ajSysCommandRenameS(const AjPStr strname, const AjPStr strname2)
{
    return ajSysCommandRenameC(MAJSTRGETPTR(strname), MAJSTRGETPTR(strname2));
}




/* @func ajSysFuncFdopen ******************************************************
**
** Place non-ANSI fdopen here
**
** @param [r] filedes [ajint] file descriptor
** @param [r] mode [const char*] file mode
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
** @param [r] name [const char*] file to open
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




/* @func ajSysFuncSocket ******************************************************
**
** Socket function coping with UNIX and WIN32
**
** @param [r] domain [int] Domain
** @param [r] type [int] Type
** @param [r] protocol [int] Protocol
** @return [SOCKRET] Universal (UNIX/WIN32) socket value
** @@
******************************************************************************/

SOCKRET ajSysFuncSocket(int domain, int type, int protocol)
{
    SOCKRET ret;

#ifndef WIN32
    ret = socket(domain, type, protocol);
#else
    ret = WSASocket(domain, type, protocol,NULL,0,0);
#endif

    return ret;
}




/* @func ajSysFuncStrdup ******************************************************
**
** Duplicate BSD strdup function for very strict ANSI compilers
**
** @param [r] dupstr [const char*] string to duplicate
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
** @param [r] srcstr [const char*] source string
** @param [r] delimstr [const char*] delimiter string
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
** @param [u] srcstr  [const char*] source string
** @param [r] delimstr [const char*] delimiter string
** @param [u] ptrptr [const char**] ptr save
** @param [w] buf [AjPStr*] result buffer
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




/* @section executing commands ************************************************
**
** Functions for executing commands
**
** @fdata [none]
** @fcategory misc
**
** @nam3rule  Exec          Execute the equivalent of a command
**
******************************************************************************/




/* @func ajSysExecC ***********************************************************
**
** Exec a command line as if from the C shell
**
** The exec'd program is passed a new argv array in argptr
**
** @param [r] cmdlinetxt [const char*] The command line
** @return [ajint] Exit status
** @@
******************************************************************************/

ajint ajSysExecC(const char* cmdlinetxt)
{
#ifndef WIN32
    pid_t pid;
    pid_t retval;
    ajint status = 0;
    char *pgm;
    char **argptr;
    ajint i;

    AjPStr pname = NULL;

    ajDebug("ajSysExecC '%s'\n", cmdlinetxt);

    if(!ajSysArglistBuildC(cmdlinetxt, &pgm, &argptr))
	return -1;

    pname = ajStrNewC(pgm);

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

#else
    PROCESS_INFORMATION procInfo;
    STARTUPINFO startInfo;
    ajint status = 0;

    ajDebug ("Launching process '%s'\n", cmdlinetxt);
    
    ZeroMemory(&startInfo, sizeof(startInfo));
    startInfo.cb = sizeof(startInfo);
    
    if(!CreateProcess(NULL, (char *)cmdlinetxt, NULL, NULL, FALSE,
                      CREATE_NO_WINDOW, NULL, NULL, &startInfo, &procInfo))
	ajFatal("CreateProcess failed");

    if(!WaitForSingleObject(procInfo.hProcess, INFINITE))
        status = 0;
    else
        status = -1;

    CloseHandle(procInfo.hProcess);
    CloseHandle(procInfo.hThread);

#endif

    return status;
}




/* @func ajSysExecS ***********************************************************
**
** Exec a command line as if from the C shell
**
** The exec'd program is passed a new argv array in argptr
**
** @param [r] cmdline [const AjPStr] The command line
** @return [ajint] Exit status
** @@
******************************************************************************/

ajint ajSysExecS(const AjPStr cmdline)
{
    return ajSysExecC(MAJSTRGETPTR(cmdline));
}




/* @func ajSysExecEnvC ********************************************************
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
** @param [r] cmdlinetxt [const char*] The command line
** @param [r] env [char* const[]] The environment
** @return [ajint] Exit status
** @@
******************************************************************************/

ajint ajSysExecEnvC(const char* cmdlinetxt, char * const env[])
{
    ajint status = 0;

#ifndef WIN32
    pid_t pid;
    pid_t retval;
    char *pgm = NULL;
    char **argptr = NULL;
    ajint i;

    AjPStr pname = NULL;

    if(!ajSysArglistBuildC(cmdlinetxt, &pgm, &argptr))
	return -1;

    pname = ajStrNew();

    ajDebug("ajSysSystemEnv '%s' %s \n", pgm, cmdlinetxt);
    ajStrAssignC(&pname, pgm);
    if(!ajSysFileWhichEnv(&pname, env))
	ajFatal("cannot find program '%S'", pname);

    ajDebug("ajSysSystemEnv %S = %s\n", pname, cmdlinetxt);
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

    return status;
}




/* @func ajSysExecEnvS ********************************************************
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
** @return [ajint] Exit status
** @@
******************************************************************************/

ajint ajSysExecEnvS(const AjPStr cmdline, char * const env[])
{
    return ajSysExecEnvC(MAJSTRGETPTR(cmdline), env);
}




/* @obsolete ajSystemEnv
** @rename ajSysExecEnvS
*/

__deprecated void ajSystemEnv(const AjPStr cl, char * const env[])
{
    ajSysExecEnvS(cl, env);
    return;
}




/* @obsolete ajSysSystemEnv
** @rename ajSysExecEnvS
*/

__deprecated void ajSysSystemEnv(const AjPStr cmdline, char * const env[])
{
    ajSysExecEnvS(cmdline, env);

    return;
}




/* @func ajSysExecLocaleC *****************************************************
**
** Exec a command line as if from the C shell with a defined locale
** variable.
**
** The exec'd program is passed a new argv array in argptr
**
** @param [r] cmdlinetxt [const char*] The command line
** @param [r] localetxt [const char*] The locale value
** @return [ajint] Exit status
** @@
******************************************************************************/

ajint ajSysExecLocaleC(const char* cmdlinetxt, const char* localetxt)
{
#ifndef WIN32
    pid_t pid;
    pid_t retval;
    ajint status = 0;
    char *pgm;
    char **argptr;
    ajint i;

    AjPStr pname = NULL;

    ajDebug("ajSysExecLocaleC '%s' '%s'\n", cmdlinetxt, localetxt);

    if(!ajSysArglistBuildC(cmdlinetxt, &pgm, &argptr))
	return -1;

    pname = ajStrNewC(pgm);

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
        setlocale(LC_ALL, localetxt);
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

#else
    PROCESS_INFORMATION procInfo;
    STARTUPINFO startInfo;
    ajint status = 0;

    ajDebug ("Launching process '%s'\n", cmdlinetxt);
    
    ZeroMemory(&startInfo, sizeof(startInfo));
    startInfo.cb = sizeof(startInfo);
    
    if(!CreateProcess(NULL, (char *)cmdlinetxt, NULL, NULL, FALSE,
                      CREATE_NO_WINDOW, NULL, NULL, &startInfo, &procInfo))
	ajFatal("CreateProcess failed");

    if(!WaitForSingleObject(procInfo.hProcess, INFINITE))
        status = 0;
    else
        status = -1;

    CloseHandle(procInfo.hProcess);
    CloseHandle(procInfo.hThread);

#endif

    return status;
}




/* @func ajSysExecLocaleS *****************************************************
**
** Exec a command line as if from the C shell with a defined locale
** variable.
**
** The exec'd program is passed a new argv array in argptr
**
** @param [r] cmdline [const AjPStr] The command line
** @param [r] localestr [const AjPStr] The locale value
** @return [ajint] Exit status
** @@
******************************************************************************/

ajint ajSysExecLocaleS(const AjPStr cmdline, const AjPStr localestr)
{
    return ajSysExecLocaleC(MAJSTRGETPTR(cmdline), MAJSTRGETPTR(localestr));
}




/* @func ajSysExecOutnameC *************************************************
**
** Exec a command line as if from the C shell with standard output redirected
** to and overwriting a named file
**
** The exec'd program is passed a new argv array in argptr
**
** @param [r] cmdlinetxt [const char*] The command line
** @param [r] outfnametxt [const char*] The output file name
** @return [ajint] Exit status
** @@
******************************************************************************/

ajint ajSysExecOutnameC(const char* cmdlinetxt, const char* outfnametxt)
{
#ifndef WIN32
    pid_t pid;
    pid_t retval;
    ajint status;
    char *pgm;
    char **argptr;
    ajint i;

    AjPStr pname = NULL;

    if(!ajSysArglistBuildC(cmdlinetxt, &pgm, &argptr))
	return -1;

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

	if(!freopen(outfnametxt, "wb", stdout))
	    ajErr("Failed to redirect standard output to '%s'", outfnametxt);
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

#else

    PROCESS_INFORMATION pinf;
    STARTUPINFO si;
    HANDLE fp;
    SECURITY_ATTRIBUTES sa;
    ajint status = 0;

    ajDebug ("Launching process '%s'\n", cmdlinetxt);

    fflush(stdout);
    
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags |= STARTF_USESTDHANDLES;


    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    fp = CreateFile(TEXT(outfnametxt), GENERIC_WRITE, 0 , &sa,
		    CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if(fp == INVALID_HANDLE_VALUE)
        ajFatal("Cannot open file %s\n",outfnametxt);


    si.hStdOutput = fp;
    si.hStdInput  = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdError  = GetStdHandle(STD_ERROR_HANDLE);

    
    if(!CreateProcess(NULL, (char *) cmdlinetxt, NULL, NULL, TRUE,
                      CREATE_NO_WINDOW, NULL, NULL, &si, &pinf))
    {
	ajFatal("CreateProcess failed");
    }
    
    if(!WaitForSingleObject(pinf.hProcess, INFINITE))
        status = 0;
    else
        status = -1;

    CloseHandle(pinf.hProcess);
    CloseHandle(pinf.hThread);
#endif

    return status;
}




/* @func ajSysExecOutnameS ****************************************************
**
** Exec a command line as if from the C shell with standard output redirected
** to and overwriting a named file
**
** The exec'd program is passed a new argv array in argptr
**
** @param [r] cmdline [const AjPStr] The command line
** @param [r] outfname [const AjPStr] The output file name
** @return [ajint] Exit status
** @@
******************************************************************************/

ajint ajSysExecOutnameS(const AjPStr cmdline, const AjPStr outfname)
{
    return ajSysExecOutnameC(MAJSTRGETPTR(cmdline), MAJSTRGETPTR(outfname));
}




/* @func ajSysExecOutnameAppendC **********************************************
**
** Exec a command line as if from the C shell with standard output redirected
** and appended to a named file
**
** The exec'd program is passed a new argv array in argptr
**
** @param [r] cmdlinetxt [const char*] The command line
** @param [r] outfnametxt [const char*] The output file name
** @return [ajint] Exit status
** @@
******************************************************************************/

ajint ajSysExecOutnameAppendC(const char* cmdlinetxt, const char* outfnametxt)
{
#ifndef WIN32
    pid_t pid;
    pid_t retval;
    ajint status;
    char *pgm;
    char **argptr;
    ajint i;

    AjPStr pname = NULL;

    if(!ajSysArglistBuildC(cmdlinetxt, &pgm, &argptr))
	return -1;

    fflush(stdout);
    
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

	if(!freopen(outfnametxt, "ab", stdout))
	    ajErr("Failed to redirect standard output to '%s'", outfnametxt);
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

#else

    PROCESS_INFORMATION pinf;
    STARTUPINFO si;
    HANDLE fp;
    SECURITY_ATTRIBUTES sa;
    ajint status = 0;

    ajDebug ("Launching process '%s'\n", cmdlinetxt);

    fflush(stdout);
    
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags |= STARTF_USESTDHANDLES;


    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    fp = CreateFile(TEXT(outfnametxt), GENERIC_WRITE, 0 , &sa,
		    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if(fp == INVALID_HANDLE_VALUE)
        ajFatal("Cannot open file %s\n",outfnametxt);

    SetFilePointer(fp, 0, NULL, FILE_END);

    si.hStdOutput = fp;
    si.hStdInput  = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdError  = GetStdHandle(STD_ERROR_HANDLE);

    
    if(!CreateProcess(NULL, (char *) cmdlinetxt, NULL, NULL, TRUE,
                      CREATE_NO_WINDOW, NULL, NULL, &si, &pinf))
    {
	ajFatal("CreateProcess failed");
    }
    
    if(!WaitForSingleObject(pinf.hProcess, INFINITE))
        status = 0;
    else
        status = -1;

    CloseHandle(pinf.hProcess);
    CloseHandle(pinf.hThread);

#endif

    return status;
}




/* @func ajSysExecOutnameAppendS **********************************************
**
** Exec a command line as if from the C shell with standard output redirected
** and appended to a named file
**
** The exec'd program is passed a new argv array in argptr
**
** @param [r] cmdline [const AjPStr] The command line
** @param [r] outfname [const AjPStr] The output file name
** @return [ajint] Exit status
** @@
******************************************************************************/

ajint ajSysExecOutnameAppendS(const AjPStr cmdline, const AjPStr outfname)
{
    return ajSysExecOutnameAppendC(MAJSTRGETPTR(cmdline),
                                   MAJSTRGETPTR(outfname));
}




/* @func ajSysExecOutnameErrC *************************************************
**
** Exec a command line as if from the C shell with standard output and
** standard error redirected to and overwriting a named file
**
** The exec'd program is passed a new argv array in argptr
**
** @param [r] cmdlinetxt [const char*] The command line
** @param [r] outfnametxt [const char*] The output file name
** @return [ajint] Exit status
** @@
******************************************************************************/

ajint ajSysExecOutnameErrC(const char* cmdlinetxt, const char* outfnametxt)
{
#ifndef WIN32
    pid_t pid;
    pid_t retval;
    ajint status;
    char *pgm;
    char **argptr;
    ajint i;

    AjPStr pname = NULL;

    if(!ajSysArglistBuildC(cmdlinetxt, &pgm, &argptr))
	return -1;

    pname = ajStrNew();

    ajStrAssignC(&pname, pgm);

    if(!ajSysFileWhich(&pname))
	ajFatal("cannot find program '%S'", pname);

    fflush(stdout);
    fflush(stderr);

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

	if(!freopen(outfnametxt, "wb", stdout))
	    ajErr("Failed to redirect standard output to '%s'", outfnametxt);
        close(STDERR_FILENO);
        dup(fileno(stdout));
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

#else

    PROCESS_INFORMATION pinf;
    STARTUPINFO si;
    HANDLE fp;
    SECURITY_ATTRIBUTES sa;
    ajint status = 0;

    ajDebug ("Launching process '%s'\n", cmdlinetxt);

    fflush(stdout);
    fflush(stderr);
    
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags |= STARTF_USESTDHANDLES;


    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    fp = CreateFile(TEXT(outfnametxt), GENERIC_WRITE, 0 , &sa,
		    CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if(fp == INVALID_HANDLE_VALUE)
        ajFatal("Cannot open file %s\n",outfnametxt);


    si.hStdOutput = fp;
    si.hStdInput  = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdError  = fp;

    
    if(!CreateProcess(NULL, (char *) cmdlinetxt, NULL, NULL, TRUE,
                      CREATE_NO_WINDOW, NULL, NULL, &si, &pinf))
    {
	ajFatal("CreateProcess failed");
    }
    
    if(!WaitForSingleObject(pinf.hProcess, INFINITE))
        status = 0;
    else
        status = -1;

    CloseHandle(pinf.hProcess);
    CloseHandle(pinf.hThread);
#endif

    return status;
}




/* @func ajSysExecOutnameErrS *************************************************
**
** Exec a command line as if from the C shell with standard output and
** standard error redirected to and overwriting a named file
**
** The exec'd program is passed a new argv array in argptr
**
** @param [r] cmdline [const AjPStr] The command line
** @param [r] outfname [const AjPStr] The output file name
** @return [ajint] Exit status
** @@
******************************************************************************/

ajint ajSysExecOutnameErrS(const AjPStr cmdline, const AjPStr outfname)
{
    return ajSysExecOutnameErrC(MAJSTRGETPTR(cmdline), MAJSTRGETPTR(outfname));
}




/* @obsolete ajSystemOut
** @rename ajSysExecOutnameS
*/

__deprecated void ajSystemOut(const AjPStr cl, const AjPStr outfname)
{
    ajSysExecOutnameS(cl, outfname);
    return;
}




/* @obsolete ajSysSystemOut
** @rename ajSysExecOutnameS
*/

__deprecated void ajSysSystemOut(const AjPStr cmdline, const AjPStr outfname)
{
    ajSysExecOutnameS(cmdline, outfname);
    return;
}




/* @func ajSysExecOutnameErrAppendC *******************************************
**
** Exec a command line as if from the C shell with standard output and
** standard error redirected and appended to a named file
**
** The exec'd program is passed a new argv array in argptr
**
** @param [r] cmdlinetxt [const char*] The command line
** @param [r] outfnametxt [const char*] The output file name
** @return [ajint] Exit status
** @@
******************************************************************************/

ajint ajSysExecOutnameErrAppendC(const char* cmdlinetxt,
                                 const char* outfnametxt)
{
#ifndef WIN32
    pid_t pid;
    pid_t retval;
    ajint status = 0;
    char *pgm;
    char **argptr;
    ajint i;

    AjPStr pname = NULL;

    if(!ajSysArglistBuildC(cmdlinetxt, &pgm, &argptr))
	return -1;

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

	if(!freopen(outfnametxt, "ab", stdout))
	    ajErr("Failed to redirect standard output and error to '%s'",
                  outfnametxt);
        close(STDERR_FILENO);
        dup(fileno(stdout));
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

#else

    PROCESS_INFORMATION pinf;
    STARTUPINFO si;
    HANDLE fp;
    SECURITY_ATTRIBUTES sa;
    ajint status = -1;

    ajDebug ("Launching process '%s'\n", cmdlinetxt);
    
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags |= STARTF_USESTDHANDLES;


    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    fp = CreateFile(TEXT(outfnametxt), GENERIC_WRITE, 0 , &sa,
		    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if(fp == INVALID_HANDLE_VALUE)
        ajFatal("Cannot open file %s\n",outfnametxt);

    SetFilePointer(fp, 0, NULL, FILE_END);

    si.hStdOutput = fp;
    si.hStdInput  = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdError  = fp;

    
    if(!CreateProcess(NULL, (char *) cmdlinetxt, NULL, NULL, TRUE,
                      CREATE_NO_WINDOW, NULL, NULL, &si, &pinf))
    {
	ajFatal("CreateProcess failed");
    }
    
    if(!WaitForSingleObject(pinf.hProcess, INFINITE))
        status = 0;
    else
        status = -1;

    CloseHandle(pinf.hProcess);
    CloseHandle(pinf.hThread);
#endif

    return status;
}




/* @func ajSysExecOutnameErrAppendS *******************************************
**
** Exec a command line as if from the C shell with standard output and
** standard error redirected and appended to a named file
**
** The exec'd program is passed a new argv array in argptr
**
** @param [r] cmdline [const AjPStr] The command line
** @param [r] outfname [const AjPStr] The output file name
** @return [ajint] Exit status
** @@
******************************************************************************/

ajint ajSysExecOutnameErrAppendS(const AjPStr cmdline, const AjPStr outfname)
{
    return ajSysExecOutnameErrAppendC(MAJSTRGETPTR(cmdline),
                                      MAJSTRGETPTR(outfname));
}




/* @func ajSysExecPathC *******************************************************
**
** Exec a command line with a test for the program name in the current path
**
** The exec'd program is passed a new argv array in argptr
**
** @param [r] cmdlinetxt [const char*] The command line
** @return [ajint] Exit status
** @@
******************************************************************************/

ajint ajSysExecPathC(const char* cmdlinetxt)
{
#ifndef WIN32
    pid_t pid;
    pid_t retval;
    ajint status = 0;
    char *pgm;
    char **argptr;
    ajint i;

    AjPStr pname = NULL;

    ajDebug("ajSysExecPathS '%s'\n", cmdlinetxt);

    if(!ajSysArglistBuildC(cmdlinetxt, &pgm, &argptr))
	return -1;

    pname = ajStrNewC(pgm);

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
	execvp(ajStrGetPtr(pname), argptr);
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

#else
    PROCESS_INFORMATION procInfo;
    STARTUPINFO startInfo;
    ajint status = 0;

    ajDebug ("Launching process '%s'\n", cmdlinetxt);
    
    ZeroMemory(&startInfo, sizeof(startInfo));
    startInfo.cb = sizeof(startInfo);
    
    if (!CreateProcess(NULL, (char *)cmdlinetxt, NULL, NULL, FALSE,
		       CREATE_NO_WINDOW, NULL, NULL, &startInfo, &procInfo))
	ajFatal("CreateProcess failed");

    if(!WaitForSingleObject(procInfo.hProcess, INFINITE))
        status = 0;
    else
        status = -1;

#endif

    return status;
}




/* @func ajSysExecPathS *******************************************************
**
** Exec a command line with a test for the program name in the current path
**
** The exec'd program is passed a new argv array in argptr
**
** @param [r] cmdline [const AjPStr] The command line
** @return [ajint] Exit status
** @@
******************************************************************************/

ajint ajSysExecPathS(const AjPStr cmdline)
{
    return ajSysExecPathC(MAJSTRGETPTR(cmdline));
}




/* @obsolete ajSystem
** @rename ajSysSystem
*/

__deprecated void ajSystem(const AjPStr cl)
{
    ajSysExecS(cl);
    return;
}




/* @obsolete ajSysSystem
** @rename ajSysExecS
*/

__deprecated void ajSysSystem(const AjPStr cmdline)
{
    ajSysExecS(cmdline);
    return;
}




/* @func ajSysExecProgArgEnvNowaitC *******************************************
**
** Exec a command line with no parent wait
**
** This routine must be passed a program, an argument list and an environment.
** The wait handling is performed by user-supplied parent process code and
** is therefore used by the Java Native Interface software.
**
** @param [r] prog[const char*] The command line
** @param [r] arg [char* const[]] Argument list
** @param [r] env [char* const[]] An environment
** @return [ajint] Exit status
** @@
******************************************************************************/

ajint  ajSysExecProgArgEnvNowaitC(const char *prog, char * const arg[],
                                char * const env[])
{
#ifndef WIN32
    if(execve(prog,arg,env) == -1)
        ajFatal("ajSysExecProgArgEnvNowaitC: Cannot exec application %s",
                prog);
#endif

    return 0;
}




/* @func ajSysExecProgArgEnvNowaitS *******************************************
**
** Exec a command line with no parent wait
**
** This routine must be passed a program, an argument list and an environment.
** The wait handling is performed by user-supplied parent process code and
** is therefore used by the Java Native Interface software.
**
** @param [r] progstr [const AjPStr] The command line
** @param [r] arg [char* const[]] Argument list
** @param [r] env [char* const[]] An environment
** @return [ajint] Exit status
** @@
******************************************************************************/

ajint ajSysExecProgArgEnvNowaitS(const AjPStr progstr, char * const arg[],
                                 char * const env[])
{
    return ajSysExecProgArgEnvNowaitC(MAJSTRGETPTR(progstr), arg, env);
}




/* @section Miscellaneous system functions ************************************
**
** Miscellaneous system functions
**
** @fdata [none]
** @fcategory misc
**
** @nam3rule  Canon          Sets or unsets TTY canonical mode
** @nam3rule  Socketclose    Closes a UNIX or WIN32 socket
** @nam3rule  Exit           Cleans up system internals memory
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




/* @func ajSysFdFromSocket  ****************************************************
**
** return a file descriptor from a UNIX of Windows socket
**
** @param [r] sock [struct AJSOCKET] AJAX socket structure
** @param [r] mode [const char*] Opening mode ("r" or "a")
** @return [FILE*] File descriptor
** @@
******************************************************************************/

FILE* ajSysFdFromSocket(struct AJSOCKET sock, const char *mode)
{
    FILE *ret;
#ifdef WIN32
    int fd;
    int flags = 0;
    char c;
    
#endif

    if(!mode)
        return NULL;
    
#ifndef WIN32
    ret = ajSysFuncFdopen(sock.sock,mode);
#else
    c = *mode;
    
    switch(c)
    {
        case 'r':
            flags = _O_RDONLY;
            break;
        case 'a':
            flags = _O_APPEND;
            break;
        default:
            ajErr("ajSysFdFromSocket: Illegal mode [%c]",c);
            break;
    }
    
    fd  = _open_osfhandle(sock.sock, _O_RDONLY);
    ret = ajSysFuncFdopen(fd,mode);
#endif

    return ret;
}




/* @func ajSysSocketclose  ****************************************************
**
** Closes a UNIX or WIN32 socket
**
** @param [r] sock [struct AJSOCKET] AJAX socket structure
** @return [void]
** @@
******************************************************************************/

void ajSysSocketclose(struct AJSOCKET sock)
{
#ifndef WIN32
    close(sock.sock);
#else
    closesocket(sock.sock);
#endif

    return;
}




/* @func ajSysTimeoutSet  ****************************************************
**
** Sets an alarm abort timeout for UNIX and Windows
**
** @param [u] ts [struct AJTIMEOUT*] AJAX timeout structure
** @return [int] 0 = success -1 = error
** @@
******************************************************************************/

int ajSysTimeoutSet(struct AJTIMEOUT *ts)
{
    int ret = 0;
    
#ifndef WIN32
    sigemptyset( &ts->sa.sa_mask );
    ts->sa.sa_flags = 0;
    ts->sa.sa_handler = sysTimeoutAbort;
    ret = sigaction( SIGALRM, &ts->sa, NULL );

    alarm(ts->seconds);
#else
    PTIMERAPCROUTINE ptim = NULL;

    ts->wtime.QuadPart = -10000000LL;
    ts->wtime.QuadPart *= ts->seconds;

    ts->thandle = CreateWaitableTimer(NULL, TRUE, NULL);
    if(!ts->thandle)
        return -1;

    ptim = (PTIMERAPCROUTINE) sysTimeoutAbort;

    if (!SetWaitableTimer(ts->thandle, &ts->wtime, 0, ptim, NULL, 0))
        ret = -1;
    
#endif

    return ret;
}




/* @func ajSysTimeoutUnset ***************************************************
**
** Unsets an alarm abort timeout for UNIX and Windows
**
** @param [u] ts [struct AJTIMEOUT*] AJAX timeout structure
** @return [int] 0 = success -1 = error
** @@
******************************************************************************/

int ajSysTimeoutUnset(struct AJTIMEOUT *ts)
{
    int ret = 0;
    
#ifndef WIN32
    ret = sigemptyset(&ts->sa.sa_mask);

    alarm(0);
#else
    if(!CancelWaitableTimer(ts->thandle))
        return -1;

    if(!CloseHandle(ts->thandle))
      ret = -1;
#endif

    return ret;
}




#ifndef WIN32
/* @funcstatic sysTimeoutAbort ********************************************
**
** Fatal error if a socket read hangs
**
** @param [r] sig [int] Signal code - always SIGALRM but required by the
**                      signal call
** @return [void]
** @@
******************************************************************************/
static void sysTimeoutAbort(int sig)
{
    (void) sig;

    ajDie("Alarm timeout");

    return;
}
#else
static void CALLBACK sysTimeoutAbort(LPVOID arg, DWORD low, DWORD high)
{
    (void) arg;
    (void) low;
    (void) high;
    
    ajDie("Timer timeout");

    return;
}
#endif




/* @obsolete ajSysBasename
** @rename ajFilenameTrimPath
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




/* @section piped commands ****************************************************
**
** system command lines piped to a file
**
** @fdata [none]
** @fcategory misc
**
** @nam3rule  Create         Create a new input
** @nam3rule  Inpipe         Create a new input
**
** @suffix C C character string arguments
** @suffix S String object arguments
** 
** @argrule C commandtxt [const char*] Command line
** @argrule S command [const AjPStr] Command line
** @valrule   *Inpipe  [AjPFile]
**
******************************************************************************/




/* @func ajSysCreateInpipeC ***************************************************
**
** Return a new file object from which to read the output from a command.
**
** @param [r] commandtxt [const char*] Command string.
**                    The string may end with a trailing pipe character.
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajSysCreateInpipeC(const char* commandtxt)
{
    AjPFile thys;
    AjPStr cmdstr = NULL;

#ifndef WIN32
    ajint pipefds[2];		     /* file descriptors for a pipe */
    char** arglist        = NULL;
    char* pgm;
#else
    HANDLE cstdinr  = NULL;
    HANDLE cstdinw  = NULL;
    HANDLE cstdoutr = NULL;
    HANDLE cstdoutw = NULL;
    HANDLE svstdout = NULL;
    HANDLE cstdoutrdup = NULL;

    SECURITY_ATTRIBUTES sa;

    PROCESS_INFORMATION pinf;
    STARTUPINFO sinf;

    BOOL ret =  FALSE;
    
    int fd;
#endif

    cmdstr = ajStrNew();
    
    AJNEW0(thys);
    ajStrAssignC(&cmdstr, commandtxt);

    ajDebug("ajSysCreateInpipeC: '%s'\n", commandtxt);

    /* pipe character at end */
    if(ajStrGetCharLast(cmdstr) == '|')
	ajStrCutEnd(&cmdstr, 1);


#ifndef WIN32

    if(pipe(pipefds) < 0)
	ajFatal("ajSysCreateInpipeC: pipe create failed");

    /* negative return indicates failure */
    thys->Pid = fork();

    if(thys->Pid < 0)
	ajFatal("ajSysCreateInpipeC: fork create failed");

    /* pid is zero in the child, but is the child PID in the parent */
    
    if(!thys->Pid)
    {
	/* this is the child process */
	close(pipefds[0]);

	dup2(pipefds[1], 1);
	close(pipefds[1]);
	ajSysArglistBuildS(cmdstr, &pgm, &arglist);
	ajDebug("ajSysCreateInpipeC: execvp ('%S', NULL)\n", cmdstr);
	execvp(pgm, arglist);
	ajErr("ajSysCreateInpipeC: execvp ('%S', NULL) failed: '%s'\n",
		cmdstr, strerror(errno));
	ajExitAbort();
    }
    
    ajDebug("ajSysCreateInpipeC: pid %d, pipe '%d', '%d'\n",
	    thys->Pid, pipefds[0], pipefds[1]);

    /* fp is what we read from the pipe */
    thys->fp = ajSysFuncFdopen(pipefds[0], "r");
    close(pipefds[1]);

#else
    
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle =  TRUE;
    sa.lpSecurityDescriptor = NULL;

    svstdout = GetStdHandle(STD_OUTPUT_HANDLE);

    if(!CreatePipe(&cstdoutr, &cstdoutw, &sa,  0))
        ajFatal("ajSysCreateInpipeC: pipe create failed");

    if(!SetHandleInformation(cstdoutr, HANDLE_FLAG_INHERIT, 0))
        ajFatal("ajSysCreateInpipeC: Can't set no-inherit on child stdout "
		"read handle");


    if(!SetStdHandle(STD_OUTPUT_HANDLE, cstdoutw)) 
        ajFatal("ajSysCreateInpipeC: redirecting of STDOUT failed");

    ret = DuplicateHandle(GetCurrentProcess(),
			  cstdoutr,
			  GetCurrentProcess(),
			  &cstdoutrdup , 0,
			  FALSE,
			  DUPLICATE_SAME_ACCESS);
    if(!ret)
      ajFatal("ajSysCreateInpipeC: Could not duplicate stdout read handle");

    CloseHandle(cstdoutr);

    /* Create Process here */

    ZeroMemory(&pinf, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&sinf, sizeof(STARTUPINFO));
    sinf.cb = sizeof(STARTUPINFO);

    ret = CreateProcess(NULL, (char *)ajStrGetPtr(cmdstr), NULL, NULL, TRUE, 0,
			NULL, NULL, &sinf, &pinf);

    if(!ret)
        ajFatal("ajSysCreateInpipeC: CreateProcess failed");

    thys->Process = pinf.hProcess;
    thys->Thread  = pinf.hThread;

    if(!SetStdHandle(STD_OUTPUT_HANDLE, svstdout))
       ajFatal("restoring  stdout failed\n");
    
    CloseHandle(cstdoutw);

    fd = _open_osfhandle((intptr_t) cstdoutrdup, _O_RDONLY);
    thys->fp = ajSysFuncFdopen(fd, "r");
#endif

    ajStrDel(&cmdstr);

    if(!thys->fp)
    {
	thys->Handle = 0;
	ajFileClose(&thys);

	return NULL;
    }

    return thys;
}




/* @func ajSysCreateInpipeS ***************************************************
**
** Return a new file object from which to read the output from a command.
**
** @param [r] command [const AjPStr] Command string.
**                    The string may end with a trailing pipe character.
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajSysCreateInpipeS(const AjPStr command)
{
    return ajSysCreateInpipeC(MAJSTRGETPTR(command));
}




/* @func ajSysExecRedirectC **************************************************
**
** Execute an application redirecting its stdin/out to pipe fds
**
** @param [r] command [const char*] Command string.
**                    The string may end with a trailing pipe character.
** @param [w] pipeto [int**] pipes to the process
** @param [w] pipefrom [int**] pipes from the process
** @return [AjBool] True on success
** @@
******************************************************************************/

AjBool ajSysExecRedirectC(const char *command, int **pipeto, int **pipefrom)
{
    int *pipeout = NULL;
    int *pipein  = NULL;
#ifndef WIN32
    pid_t pid;
    char *pgm = NULL;
    char **argptr = NULL;
    ajint i;
#else
    HANDLE cstdinr;
    HANDLE cstdinw;
    HANDLE cstdoutr;
    HANDLE cstdoutw;
    HANDLE svstdin;
    HANDLE svstdout;
    BOOL ret;

    HANDLE cstdinwdup;
    HANDLE cstdoutrdup;

    SECURITY_ATTRIBUTES sa;

    PROCESS_INFORMATION pinf;
    STARTUPINFO sinf;
#endif
    
    if(!pipeto || !pipefrom)
        return ajFalse;

    if(!*pipeto || !*pipefrom)
        return ajFalse;
    
    pipeout = *pipeto;
    pipein  = *pipefrom;
    
#ifndef WIN32

    if(!ajSysArglistBuildC(command, &pgm, &argptr))
    {
        ajDebug("ajSysExecWithRedirect: Cannot parse command line");
        return ajFalse;
    }

    
    if(pipe(pipeout))
    {
        ajDebug("ajSysExecWithRedirect: Cannot open pipeout");
        return ajFalse;
    }

    if(pipe(pipein))
    {
        ajDebug("ajSysExecWithRedirect: Cannot open pipein");
        return ajFalse;
    }
    
    pid = fork();
    if(pid < 0)
    {
        ajDebug("ajSysExecWithRedirect: fork failure");
        return ajFalse;
    }
    else if(!pid)
    {
        /*
        ** CHILD PROCESS
        ** dup pipe read/write to stdin/stdout
        */
        dup2(pipeout[0],  fileno(stdin));
        dup2(pipein[1], fileno(stdout));

        /* close unnecessary pipe descriptors */
        close(pipeout[0]);
        close(pipeout[1]);
        close(pipein[0]);
        close(pipein[1]);

	execv(pgm, argptr);
        
        ajDebug("ajSysExecWithRedirect: Problem executing application");
        return ajFalse;
    }

    /*
    ** PARENT PROCESS
    ** Close unused pipe ends. This is especially important for the
    ** pipein[1] write descriptor, otherwise reading will never
    ** give an EOF.
    */

    ajDebug("ajSysExecWithRedirect: Within the PARENT process");
    
    close(pipeout[0]);
    close(pipein[1]);

    i = 0;

    while(argptr[i])
    {
	AJFREE(argptr[i]);
	++i;
    }

    AJFREE(argptr);
    AJFREE(pgm);
    
#else	/* WIN32 */

    
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;
		
    /*
    ** Redirect stdout by
    **    Save stdout for later restoration
    **    Create anonymous pipe as stdout for the child
    **    Set stdout of parent to be write handle to  the pipe thereby
    **       making it inherited by the child
    **    Create non-inheritable duplicate of the read handle and close
    **       the inheritable read handle
    */

    svstdout = GetStdHandle(STD_OUTPUT_HANDLE);

    if (!CreatePipe(&cstdoutr, &cstdoutw, &sa, 0))
    {
        ajDebug("ajSysExecWithRedirect: Couldn't open input pipe" );
        return ajFalse;
    }
    
    SetHandleInformation(cstdoutr,HANDLE_FLAG_INHERIT,0);

    if (!SetStdHandle(STD_OUTPUT_HANDLE, cstdoutw))
    {
        ajDebug("ajSysExecWithRedirect: failure redirecting stdout");
        return ajFalse;
    }
    
    ret = DuplicateHandle(GetCurrentProcess(),
                          cstdoutr,
                          GetCurrentProcess(),
                          &cstdoutrdup , 0,
                          FALSE,
                          DUPLICATE_SAME_ACCESS);
    if(!ret)
    {
        ajDebug("ajSysExecWithRedirect: DuplicateHandle failed");
        return ajFalse;
    }

    CloseHandle(cstdoutr);

    /*
    ** Redirect stdin by
    **    Save stdin for later restoration
    **    Create anonymous pipe as stdin for the child
    **    Set stdin of parent to be read handle of the pipe thereby
    **       making it inherited by the child
    **    Create non-inheritable duplicate of the write handle and close
    **       the inheritable write handle
    */

    svstdin = GetStdHandle(STD_INPUT_HANDLE);

    if(!CreatePipe(&cstdinr, &cstdinw, &sa, 0)) 
    {
        ajDebug("ajSysExecWithRedirect: Cannot open pipeout");
        return ajFalse;
    }

    if (!SetStdHandle(STD_INPUT_HANDLE, cstdinr)) 
    {
        ajDebug("ajSysExecWithRedirect: Cannot redirect stdin");
        return ajFalse;
    }

    ret = DuplicateHandle(GetCurrentProcess(),
                               cstdinw, 
                               GetCurrentProcess(),
                               &cstdinwdup, 0, 
                               FALSE, 
                               DUPLICATE_SAME_ACCESS); 
    if(!ret) 
    {
        ajDebug("ajSysExecWithRedirect: DuplicateHandle failure");
        return ajFalse;
    }

    CloseHandle(cstdinw);

    ZeroMemory(&pinf, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&sinf, sizeof(STARTUPINFO));
    sinf.cb = sizeof(STARTUPINFO);

    ret = CreateProcess(NULL, (char *)command, NULL, NULL, TRUE, 0, NULL, NULL,
                        &sinf, &pinf);

    if(ret == 0)
    {
        ajDebug("ajSysExecWithRedirect: Cannot execute application");
        return ajFalse;
    }

    CloseHandle(pinf.hProcess);
    CloseHandle(pinf.hThread);


    if(!SetStdHandle(STD_INPUT_HANDLE, svstdin))
    {
        ajDebug("ajSysExecWithRedirect: Error restoring stdin");
        return ajFalse;
    }

    if(!SetStdHandle(STD_OUTPUT_HANDLE, svstdout))
    {
        ajDebug("ajSysExecWithRedirect: Error restoring stdout");
        return ajFalse;
    }

    CloseHandle(cstdoutw);

    pipeout[1] = _open_osfhandle((intptr_t) cstdinwdup,
                                 _O_APPEND);

    pipein[0]  = _open_osfhandle((intptr_t) cstdoutrdup,
                                 _O_RDONLY);
#endif

    return ajTrue;
}




/* @func ajSysGetHomedirFromName *********************************************
**
** Get a home directory location from  a username
**
** @param [r] username [const char*] Username
** @return [char*] Home directory or NULL
** @@
******************************************************************************/

char* ajSysGetHomedirFromName(const char *username)
{
    char *hdir = NULL;
#ifndef WIN32
    struct passwd *pass = NULL;
    

    pass = getpwnam(username);
    
    if(!pass)
        return NULL;

    hdir = ajCharNewC(pass->pw_dir);
#else
    LPTSTR domainname = NULL;
    LPTSTR localsvr   = NULL;
    DWORD dnsize  = 0;
    PSID psid     = NULL;
    DWORD sidsize = 0;

    SID_NAME_USE sidtype;

    LPTSTR strsid = NULL;
    AjPStr subkey = NULL;

    DWORD hdbuffsize = MAX_PATH;
    char hdbuff[MAX_PATH];

    LONG ret;
    HKEY hkey = NULL;

    if(!LookupAccountName(localsvr, username, psid, &sidsize, domainname,
			  &dnsize, &sidtype))
    {
        if(GetLastError() == ERROR_NONE_MAPPED)
        {
            ajWarn("No Windows username found for '%s'", username);
            return NULL;
        }
        else if(GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            psid = (LPTSTR) LocalAlloc(LPTR,sidsize * sizeof(TCHAR));
            if(!psid)
            {
                ajWarn("ajSysLookupAccount: SID allocation failure");
                return NULL;
            }

            domainname = (LPTSTR) LocalAlloc(LPTR, dnsize * sizeof(TCHAR));
            if(!domainname)
            {
                ajWarn("ajSysLookupAccount: Domain allocation failure");
                LocalFree(psid);
                return NULL;
            }

            if(!LookupAccountName(localsvr, username, psid, &sidsize,
				  domainname, &dnsize, &sidtype))
            {
                ajWarn("LookupAccountName failed with %d", GetLastError());
                LocalFree(psid);
                LocalFree(domainname);
                return NULL;
            }
        }
        else
        {
            ajWarn("General LookupAccountName failure with %d", GetLastError());
            return NULL;
        }
    }


    if(!ConvertSidToStringSid(psid, &strsid))
    {
        LocalFree(psid);

        return NULL;
    }

    LocalFree(psid);

    subkey = ajStrNew();

    ajFmtPrintS(&subkey,"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\"
                "ProfileList\\%s",strsid);

    LocalFree(strsid);

    ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE,ajStrGetPtr(subkey),(DWORD) 0,
		       KEY_QUERY_VALUE,&hkey);

    ajStrDel(&subkey);

    if(ret != ERROR_SUCCESS)
        return NULL;

    ret = RegQueryValueEx(hkey,"ProfileImagePath",NULL,NULL,(LPBYTE)hdbuff,
			  &hdbuffsize);

    if(ret != ERROR_SUCCESS)
        return NULL;

    ret = RegCloseKey(hkey);

    if(ret != ERROR_SUCCESS)
        return NULL;

    hdir = ajCharNewC(hdbuff);
#endif


    return hdir;
}




/* @func ajSysGetHomedir *************************************************
**
** Get the home directory of the current user
**
** @return [char*] Home directory or NULL
** @@
******************************************************************************/

char* ajSysGetHomedir(void)
{
    char *hdir = NULL;
#ifndef WIN32
    char *p = NULL;

    if(!(p = getenv("HOME")))
        return NULL;

    hdir = ajCharNewC(p);
#else
    TCHAR wpath[MAX_PATH];
    HRESULT ret;

    /* Replace with SHGetKnownFolderPath when XP is no longer supported */
    ret = SHGetFolderPath(NULL,CSIDL_PROFILE,NULL,0,wpath);

    if(ret != S_OK)
        return NULL;
    
    hdir = ajCharNewC(wpath);
#endif

    return hdir;
}




/* @section exit **************************************************************
**
** Functions called on exit from the program by ajExit to do
** any necessary cleanup and to report internal statistics to the debug file
**
** @fdata      [none]
** @fnote     general exit functions, no arguments
**
** @nam3rule Exit Cleanup and report on exit
**
** @valrule * [void]
**
** @fcategory misc
*/




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

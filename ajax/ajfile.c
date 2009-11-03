/******************************************************************************
** @source AJAX file routines
**
** @version 1.0
** @modified May 14 Jon Ison Added ajFileExtnTrim & ajFileDirExtnTrim
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
#ifndef WIN32
#include <dirent.h>
#else
#include "win32.h"
#include "dirent_w32.h"
#include <direct.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>
#ifndef WIN32
#include <sys/wait.h>
#include <pwd.h>
#endif
#include <string.h>
#include <errno.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <limits.h>
#include <fcntl.h>

#define FILERECURSLV 20
static ajint fileBuffSize = 2048;

static ajint fileHandle = 0;
static ajint fileOpenCnt = 0;
static ajint fileOpenMax = 0;
static ajint fileCloseCnt = 0;
static ajint fileOpenTot = 0;

static AjPStr fileNameFix = NULL;
static AjPStr fileBaseTmp = NULL;
static AjPStr fileHomeTmp = NULL;
static AjPStr fileNameTmp = NULL;
static AjPStr filePackTmp = NULL;
static AjPStr fileDirfixTmp = NULL;
static AjPStr fileCwd = NULL;
static AjPStr fileTmpStr = NULL;
static AjPStr fileTempFilename = NULL;

static AjPRegexp fileUserExp = NULL;
static AjPRegexp fileWildExp = NULL;
static AjPRegexp fileEntryExp = NULL;
static AjPRegexp fileFileExp = NULL;
static AjPRegexp fileRestExp = NULL;
static AjPRegexp fileDirExp = NULL;
static AjPRegexp fileFilenameExp = NULL;

static void   fileBuffInit(AjPFileBuff thys);
static void   fileBuffLineAdd(AjPFileBuff thys, const AjPStr line);
static void   fileBuffLineDel(AjPFileBuff thys);
static AjBool fileBuffLineNext(AjPFileBuff thys);
static void   fileClose(AjPFile thys);
static void   fileListRecurs(const AjPStr file, AjPList list, ajint *recurs);
static DIR*   fileOpenDir(AjPStr *dir);

#ifdef __CYGWIN__
#define fopen(a,b) ajSysFuncFopen(a,b)
#endif




/* ==================================================================== */
/* ========================= constructors ============================= */
/* ==================================================================== */

/* @section Outfile Constructors ********************************************
**
** All constructors return a new open file by pointer. It is the responsibility
** of the user to first destroy any previous file pointer. The target pointer
** does not need to be initialised to NULL, but it is good programming practice
** to do so anyway.
**
** To replace or reuse an existing file, see instead
** the {File Assignments} and {File Modifiers} functions.
**
** The range of constructors is provided to allow flexibility in how
** applications can open files to read various kinds of data.
**
******************************************************************************/



/* @func ajOutfileNew *********************************************************
**
** Creates a new formatted output file object with a specified name.
**
** 'stdout' and 'stderr' are special names for standard output and
** standard error respectively.
**
** @param [r] name [const AjPStr] File name.
** @return [AjPOutfile] New output file object.
** @category new [AjPOutfile] Default constructor for an output file
** @@
******************************************************************************/

AjPOutfile ajOutfileNew(const AjPStr name)
{
    AjPOutfile thys;

    AJNEW0(thys);
    thys->File = ajFileNewOut(name);
    if(!thys->File)
	return NULL;

    return thys;
}




/* @section Directory Constructors ********************************************
**
** All constructors return a directory object by pointer.
** It is the responsibility of the user to first destroy any previous
** directory pointer. The target pointer
** does not need to be initialised to NULL, but it is good programming practice
** to do so anyway.
**
** To replace or reuse an existing file, see instead
** the {File Assignments} and {File Modifiers} functions.
**
** The range of constructors is provided to allow flexibility in how
** applications can open files to read various kinds of data.
**
******************************************************************************/






/* @func ajDirNew ************************************************************
**
** Creates a new directory object.
**
** @param [r] name [const AjPStr] Directory name
** @return [AjPDir] New directory object.
** @category new [AjPDir] Default constructor for a directory
** @@
******************************************************************************/

AjPDir ajDirNew(const AjPStr name)
{
    AjPDir thys;

    AJNEW0(thys);
    ajStrAssignS(&thys->Name, name);
    thys->Prefix = NULL;
    thys->Extension = NULL;
    thys->Output = ajFalse;

    return thys;
}



/* @func ajDirNewS ************************************************************
**
** Creates a new directory object.
**
** @param [r] name [const AjPStr] Directory name
** @param [r] ext [const AjPStr] File extension
** @return [AjPDir] New directory object.
** @category new [AjPDir] Directory constructor with extension
** @@
******************************************************************************/

AjPDir ajDirNewS(const AjPStr name, const AjPStr ext)
{
    AjPDir thys;

    AJNEW0(thys);
    ajStrAssignS(&thys->Name, name);
    if (ajStrGetLen(ext))
	ajStrAssignS(&thys->Extension, ext);
    thys->Output = ajFalse;

    return thys;
}



/* @func ajDirNewSS ***********************************************************
**
** Creates a new directory object.
**
** @param [r] name [const AjPStr] Directory name
** @param [r] prefix [const AjPStr] Filename prefix
** @param [r] ext [const AjPStr] Filename extension
** @return [AjPDir] New directory object.
** @category new [AjPDir] Directory constructor with prefix and extension
** @@
******************************************************************************/

AjPDir ajDirNewSS(const AjPStr name, const AjPStr prefix, const AjPStr ext)
{
    AjPDir thys;

    AJNEW0(thys);
    ajStrAssignS(&thys->Name, name);
    if (ajStrGetLen(prefix))
	ajStrAssignS(&thys->Prefix, prefix);
    if (ajStrGetLen(ext))
	ajStrAssignS(&thys->Extension, ext);
    thys->Output = ajFalse;

    return thys;
}



/* @func ajDiroutNew **********************************************************
**
** Creates a new directory outputobject.
**
** @param [r] name [const AjPStr] Directory name
** @return [AjPDir] New directory object.
** @category new [AjPDir] Default constructor for an output directory
** @@
******************************************************************************/

AjPDir ajDiroutNew(const AjPStr name)
{
    AjPDir thys;

    AJNEW0(thys);
    ajStrAssignS(&thys->Name, name);
    thys->Extension = NULL;
    thys->Output = ajTrue;

    return thys;
}



/* @func ajDiroutNewS *********************************************************
**
** Creates a new directory output object.
**
** @param [r] name [const AjPStr] Directory name
** @param [r] ext [const AjPStr] File extension
** @return [AjPDir] New directory object.
** @category new [AjPDir] Output directory constructor with extension
** @@
******************************************************************************/

AjPDir ajDiroutNewS(const AjPStr name, const AjPStr ext)
{
    AjPDir thys;

    AJNEW0(thys);
    ajStrAssignS(&thys->Name, name);
    if (ajStrGetLen(ext))
	ajStrAssignS(&thys->Extension, ext);
    thys->Output = ajTrue;

    return thys;
}



/* @func ajDiroutNewSS ********************************************************
**
** Creates a new directory output object.
**
** @param [r] name [const AjPStr] Directory name
** @param [r] prefix [const AjPStr] Filename prefix
** @param [r] ext [const AjPStr] File extension
** @return [AjPDir] New directory object.
** @category new [AjPDir] Output directory constructor with prefix
**                        and extension
** @@
******************************************************************************/

AjPDir ajDiroutNewSS(const AjPStr name, const AjPStr prefix, const AjPStr ext)
{
    AjPDir thys;

    AJNEW0(thys);
    ajStrAssignS(&thys->Name, name);
    if (ajStrGetLen(prefix))
	ajStrAssignS(&thys->Prefix, prefix);
    if (ajStrGetLen(ext))
	ajStrAssignS(&thys->Extension, ext);
    thys->Output = ajTrue;

    return thys;
}



/* @func ajDirName ************************************************************
**
** Returns the name of a directory object
**
** @param [r] thys [const AjPDir] Directory object.
** @return [AjPStr] Directory name
** @@
******************************************************************************/

AjPStr ajDirName(const AjPDir thys)
{
    if (!thys)
	return NULL;
    return thys->Name;
}



/* @func ajDirExt *************************************************************
**
** Returns the extension of a directory object
**
** @param [r] thys [const AjPDir] Directory object.
** @return [AjPStr] Directory name
** @@
******************************************************************************/

AjPStr ajDirExt(const AjPDir thys)
{
    if (!thys)
	return NULL;
    return thys->Extension;
}



/* @section File Constructors *************************************************
**
** All constructors return a new open file by pointer. It is the responsibility
** of the user to first destroy any previous file pointer. The target pointer
** does not need to be initialised to NULL, but it is good programming practice
** to do so anyway.
**
** To replace or reuse an existing file, see instead
** the {File Assignments} and {File Modifiers} functions.
**
** The range of constructors is provided to allow flexibility in how
** applications can open files to read various kinds of data.
**
******************************************************************************/




/* @func ajFileNew ************************************************************
**
** Creates a new file object.
**
** @return [AjPFile] New file object.
** @category new [AjPFile] Default constructor for an input file
** @@
******************************************************************************/

AjPFile ajFileNew(void)
{
    AjPFile thys;

    AJNEW0(thys);
    thys->fp = NULL;
    thys->Handle = 0;
    thys->Name = ajStrNew();
    thys->Buff = ajStrNewRes(fileBuffSize);
    thys->List = NULL;
    thys->End = ajFalse;

    fileOpenCnt++;
    fileOpenTot++;

    if(fileOpenCnt > fileOpenMax)
	fileOpenMax = fileOpenCnt;

    return thys;
}




/* @func ajFileNewInPipe ******************************************************
**
** Creates a new file object to read the output from a command.
**
** @param [r] name [const AjPStr] Command string.
** @return [AjPFile] New file object.
** @category new [AjPFile] Constructor using output from a forked command
** @@
******************************************************************************/

AjPFile ajFileNewInPipe(const AjPStr name)
{
#ifndef WIN32
    AjPFile thys;
    
    ajint pipefds[2];		     /* file descriptors for a pipe */
    char** arglist        = NULL;
    char* pgm;

    AJNEW0(thys);
    ajStrAssignS(&fileNameTmp, name);

    ajDebug("ajFileNewInPipe '%S'\n", name);

    /* pipe character at end */
    if(ajStrGetCharLast(fileNameTmp) == '|')
	ajStrCutEnd(&fileNameTmp, 1);
    if(pipe(pipefds) < 0)
	ajFatal("pipe create failed");

    /* negative return indicates failure */
    thys->Pid = fork();
    if(thys->Pid < 0)
	ajFatal("fork create failed");

    /* pid is zero in the child, but is the child PID in the parent */
    
    if(!thys->Pid)
    {
	/* this is the child process */
	close(pipefds[0]);

	dup2(pipefds[1], 1);
	close(pipefds[1]);
	ajSysArglistBuild(fileNameTmp, &pgm, &arglist);
	ajDebug("execvp ('%S', NULL)\n", fileNameTmp);
	execvp(pgm, arglist);
	ajErr("execvp ('%S', NULL) failed: '%s'\n",
		fileNameTmp, strerror(errno));
	ajExitAbort();
    }
    
    ajDebug("pid %d, pipe '%d', '%d'\n",
	    thys->Pid, pipefds[0], pipefds[1]);

    /* fp is what we read from the pipe */
    thys->fp = ajSysFuncFdopen(pipefds[0], "r");
    close(pipefds[1]);
    ajStrDelStatic(&fileNameTmp);

    if(!thys->fp)
    {
	thys->Handle = 0;
	ajFileClose(&thys);
	return NULL;
    }
    thys->Handle = ++fileHandle;
    ajStrAssignS(&thys->Name, name);
    thys->End = ajFalse;
    
    fileOpenCnt++;
    fileOpenTot++;
    if(fileOpenCnt > fileOpenMax)
	fileOpenMax = fileOpenCnt;
    
   return thys;
#else /* WIN32 */
    return NULL;
#endif
}




/* @func ajFileNewIn **********************************************************
**
** Creates a new file object to read a named file.
**
** If the filename ends with a pipe character then a pipe is opened
** using ajFileNewInPipe.
**
** @param [r] name [const AjPStr] File name.
** @return [AjPFile] New file object.
** @category new [AjPFile] Constructor using a filename for an input file
** @@
******************************************************************************/

AjPFile ajFileNewIn(const AjPStr name)
{
    AjPFile thys          = NULL;
    AjPStr userstr = NULL;
    AjPStr reststr = NULL;
    struct passwd* pass   = NULL;
    AjPStr dirname        = NULL;
    AjPStr wildname       = NULL;
    AjPFile ptr;
    
    char   *p = NULL;
    
    ajDebug("ajFileNewIn '%S'\n", name);
    
    if(ajStrMatchC(name, "stdin"))
    {
	thys = ajFileNewF(stdin);
	ajStrAssignC(&thys->Name, "stdin");
	return thys;
    }

    ajStrAssignS(&fileNameTmp, name);
    
#ifndef WIN32
    if(ajStrGetCharLast(name) == '|')	/* pipe character at end */
	return ajFileNewInPipe(name);

    if(ajStrGetCharFirst(fileNameTmp) == '~')
    {
	ajDebug("starts with '~'\n");
	if(!fileUserExp) fileUserExp = ajRegCompC("^~([^/]*)");
	ajRegExec(fileUserExp, fileNameTmp);
	ajRegSubI(fileUserExp, 1, &userstr);
	ajRegPost(fileUserExp, &reststr);
	ajDebug("  user: '%S' rest: '%S'\n", userstr, reststr);

	if(ajStrGetLen(userstr))
	{
	    /* username specified */
	    pass = getpwnam(ajStrGetPtr(userstr));
	    if(!pass) {
		ajStrDel(&userstr);
		ajStrDelStatic(&fileNameTmp);
		ajStrDel(&reststr);
		return NULL;
	    }
	    ajFmtPrintS(&fileNameTmp, "%s%S", pass->pw_dir, reststr);
	    ajDebug("use getpwnam: '%S'\n", fileNameTmp);
	}
	else
	{
	    /* just ~/ */
	    if((p = getenv("HOME")))
		ajFmtPrintS(&fileNameTmp, "%s%S", p, reststr);
	    else
		ajFmtPrintS(&fileNameTmp,"%S",reststr);
	    ajDebug("use HOME: '%S'\n", fileNameTmp);
	}
    }
    ajStrDel(&userstr);
    ajStrDel(&reststr);
#endif

    if(!fileWildExp)
	fileWildExp = ajRegCompC("(.*/)?([^/]*[*?][^/]*)$");
    
    if(ajRegExec(fileWildExp, fileNameTmp))
    {
	/* wildcard file names */
	ajRegSubI(fileWildExp, 1, &dirname);
	ajRegSubI(fileWildExp, 2, &wildname);
	ajDebug("wild dir '%S' files '%S'\n", dirname, wildname);
	ptr = ajFileNewDW(dirname, wildname);
	ajStrDelStatic(&fileNameTmp);
	ajStrDel(&dirname);
	ajStrDel(&wildname);
	return ptr;
    }
    
    
    
    AJNEW0(thys);
    ajStrAssignS(&thys->Name, fileNameTmp);
    ajStrDelStatic(&fileNameTmp);

    ajNamResolve(&thys->Name);
    thys->fp = fopen(ajStrGetPtr(thys->Name), "rb");
    if(!thys->fp)
    {
	ajStrDel(&thys->Name);

	AJFREE(thys);
	/*    thys->Handle = 0;*/
	return NULL;
    }
    thys->Handle = ++fileHandle;
    thys->List = NULL;
    thys->End = ajFalse;
    
    fileOpenCnt++;
    fileOpenTot++;
    if(fileOpenCnt > fileOpenMax)
	fileOpenMax = fileOpenCnt;
    
    return thys;
}




/* @func ajFileNewInC *********************************************************
**
** Creates a new file object to read a named file.
**
** If the filename begins with a pipe character then a pipe is opened
** using ajFileNewInPipe.
**
** @param [r] name [const char*] File name.
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajFileNewInC(const char *name)
{
    AjPStr tmp;
    AjPFile fp;

    tmp = ajStrNewC(name);
    fp = ajFileNewIn(tmp);
    ajStrDel(&tmp);

    return fp;
}




/* @func ajFileNewInList ******************************************************
**
** Creates a new file object with a list of input files.
**
** @param [u] list [AjPList] List of input filenames as strings.
** @return [AjPFile] New file object.
** @category new [AjPFile] Constructor using an AjPList of filenames
** @@
******************************************************************************/

AjPFile ajFileNewInList(AjPList list)
{
    AjPFile thys;

    AJNEW0(thys);

    thys->List = list;
    thys->Name = NULL;
    ajListstrTrace(thys->List);
    ajListstrPop(thys->List, &thys->Name);
    ajDebug("ajFileNewInList pop '%S'\n", thys->Name);
    ajListstrTrace(thys->List);
    ajNamResolve(&thys->Name);
    thys->fp = fopen(ajStrGetPtr(thys->Name), "rb");
    if(!thys->fp)
    {
	ajDebug("ajFileNewInList fopen failed\n");
	thys->Handle = 0;
	return NULL;
    }
    thys->Handle = ++fileHandle;
    thys->End = ajFalse;

    fileOpenCnt++;
    fileOpenTot++;
    if(fileOpenCnt > fileOpenMax)
	fileOpenMax = fileOpenCnt;

    return thys;
}




/* @func ajFileNewApp *********************************************************
**
** Creates an output file object with a specified name.
** The file is opened for append so it either appends to an existing file
** or opens a new one.
**
** @param [r] name [const AjPStr] File name.
** @return [AjPFile] New file object.
** @category new [AjPFile] Default constructor using a filename for an
**                         output file to be opened for appending records
**                         to the end of the file
** @@
******************************************************************************/

AjPFile ajFileNewApp(const AjPStr name)
{
    AjPFile thys;

    AJNEW0(thys);
    thys->fp = fopen(ajStrGetPtr(name), "ab");
    if(!thys->fp)
    {
	thys->Handle = 0;
	return NULL;
    }
    thys->Handle = ++fileHandle;
    ajStrAssignS(&thys->Name, name);
    thys->End = ajFalse;

    fileOpenCnt++;
    fileOpenTot++;
    if(fileOpenCnt > fileOpenMax)
	fileOpenMax = fileOpenCnt;
    thys->App = ajTrue;
    
    return thys;
}

/* @func ajFileNewOut *********************************************************
**
** Creates a new output file object with a specified name.
**
** 'stdout' and 'stderr' are special names for standard output and
** standard error respectively.
**
** @param [r] name [const AjPStr] File name.
** @return [AjPFile] New file object.
** @category new [AjPFile] Default constructor using a filename for an
**                         output file
** @@
******************************************************************************/

AjPFile ajFileNewOut(const AjPStr name)
{
    AjPFile thys;

    if(ajStrMatchC(name, "stdout"))
    {
	thys = ajFileNewF(stdout);
	ajStrAssignC(&thys->Name, "stdout");
	return thys;
    }

    if(ajStrMatchC(name, "stderr"))
    {
	thys = ajFileNewF(stderr);
	ajStrAssignC(&thys->Name, "stderr");
	return thys;
    }

    AJNEW0(thys);
    thys->fp = fopen(ajStrGetPtr(name), "wb");
    if(!thys->fp)
    {
	thys->Handle = 0;
	return NULL;
    }
    thys->Handle = ++fileHandle;
    ajStrAssignS(&thys->Name, name);
    thys->End = ajFalse;

    fileOpenCnt++;
    fileOpenTot++;
    if(fileOpenCnt > fileOpenMax)
	fileOpenMax = fileOpenCnt;
    thys->App = ajFalse;

    return thys;
}




/* @func ajFileNewOutC ********************************************************
**
** Creates a new output file object with a specified name.
**
** 'stdout' and 'stderr' are special names for standard output and
** standard error respectively.
**
** @param [r] name [const char*] File name.
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajFileNewOutC(const char* name)
{
    AjPStr tmp;
    AjPFile thys;

    tmp = ajStrNewC(name);
    thys = ajFileNewOut(tmp);
    ajStrDel(&tmp);

    return thys;
}




/* @func ajFileNewOutD ********************************************************
**
** Creates a new output file object with a specified directory and name.
**
** 'stdout' and 'stderr' are special names for standard output and
** standard error respectively.
**
** If the filename already has a directory specified,
** the "dir" argument is ignored.
**
** @param [rN] dir [const AjPStr] Directory (optional, can be empty or NULL).
** @param [r] name [const AjPStr] File name.
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajFileNewOutD(const AjPStr dir, const AjPStr name)
{
    AjPFile thys;

    ajDebug("ajFileNewOutD('%S' '%S')\n", dir, name);

    if(ajStrMatchC(name, "stdout"))
	return ajFileNewF(stdout);

    if(ajStrMatchC(name, "stderr"))
	return ajFileNewF(stderr);

    AJNEW0(thys);

    if(!ajStrGetLen(dir))
    {
	thys->fp = fopen(ajStrGetPtr(name), "wb");
	ajDebug("ajFileNewOutD open name '%S'\n", name);
    }
    else
    {
	if(ajFileHasDir(name))
	    ajStrAssignS(&fileDirfixTmp, name);
	else
	{
	    ajStrAssignS(&fileDirfixTmp, dir);
	    if(ajStrGetCharLast(dir) != SLASH_CHAR)
		ajStrAppendC(&fileDirfixTmp, SLASH_STRING);
	    ajStrAppendS(&fileDirfixTmp, name);
	}
	thys->fp = fopen(ajStrGetPtr(fileDirfixTmp), "wb");
	ajDebug("ajFileNewOutD open dirfix '%S'\n", fileDirfixTmp);
    }

    if(!thys->fp)
    {
	thys->Handle = 0;
	return NULL;
    }

    thys->Handle = ++fileHandle;
    ajStrAssignS(&thys->Name, name);
    thys->End = ajFalse;

    fileOpenCnt++;
    fileOpenTot++;
    if(fileOpenCnt > fileOpenMax)
	fileOpenMax = fileOpenCnt;

    return thys;
}




/* @func ajFileNewOutDir ******************************************************
**
** Creates a new output file object with a specified directory and name.
** Uses the default extension (if any) specified for the directory.
**
** 'stdout' and 'stderr' are special names for standard output and
** standard error respectively.
**
** If the filename already has a directory specified,
** the "dir" argument is ignored.
**
** @param [rN] dir [const AjPDir] Directory (optional, can be empty or NULL).
** @param [r] name [const AjPStr] File name.
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajFileNewOutDir(const AjPDir dir, const AjPStr name)
{
    AjPFile thys;

    ajDebug("ajFileNewOutDir('%S' '%S')\n", dir->Name, name);

    AJNEW0(thys);

    if(!dir)
    {
	thys->fp = fopen(ajStrGetPtr(name), "wb");
	ajDebug("ajFileNewOutDir open name '%S'\n", name);
    }
    else
    {
	if(ajFileHasDir(name))
	    ajStrAssignS(&fileDirfixTmp, name);
	else
	{
	    ajStrAssignS(&fileDirfixTmp, dir->Name);
	    if(ajStrGetCharLast(dir->Name) != SLASH_CHAR)
		ajStrAppendC(&fileDirfixTmp, SLASH_STRING);
	    ajStrAppendS(&fileDirfixTmp, name);
	}
	ajFileNameExt(&fileDirfixTmp, dir->Extension);

	thys->fp = fopen(ajStrGetPtr(fileDirfixTmp), "wb");
	ajDebug("ajFileNewOutDir open dirfix '%S'\n", fileDirfixTmp);
    }

    if(!thys->fp)
    {
	thys->Handle = 0;
	return NULL;
    }

    thys->Handle = ++fileHandle;
    ajStrAssignS(&thys->Name, name);
    thys->End = ajFalse;

    fileOpenCnt++;
    fileOpenTot++;
    if(fileOpenCnt > fileOpenMax)
	fileOpenMax = fileOpenCnt;

    return thys;
}




/* @func ajFileSetDir ********************************************************
**
** Adds a default directory to a filename.
** If the filename already has a directory, then this is left unchanged.
**
** 'stdout' and 'stderr' are special names for standard output and
** standard error respectively which need no directory.
**
** @param [u] pname [AjPStr*] File name, returned with directory prefix
** @param [rN] dir [const AjPStr] Directory (optional, can be empty or NULL).
** @return [AjBool] ajTrue if the filename was changed
** @@
******************************************************************************/

AjBool ajFileSetDir(AjPStr *pname, const AjPStr dir)
{
    AjBool ret;


    ajDebug("ajFileSetDir name '%S' dir '%S'\n", *pname, dir);

    ret = ajFalse;

    if(ajStrMatchC(*pname, "stdout"))
	return ret;

    if(ajStrMatchC(*pname, "stderr"))
	return ret;

    if(!ajStrGetLen(dir))
	return ret;

    if(ajFileHasDir(*pname))
	return ret;

    ajStrAssignS(&fileDirfixTmp, dir);
    if(ajStrGetCharLast(dir) != SLASH_CHAR)
	ajStrAppendC(&fileDirfixTmp, SLASH_STRING);
    ajStrAppendS(&fileDirfixTmp, *pname);
  
    ajStrAssignS(pname, fileDirfixTmp);
    ret = ajTrue;

    ajDebug("ajFileSetDir changed name '%S'\n", *pname);

    return ret;
}




/* @func ajFileHasDir ********************************************************
**
** Tests whether a filename includes a directory specification.
**
** @param [r] name [const AjPStr] File name.
** @return [AjBool] ajTrue if directory filename syntax was found
** @@
******************************************************************************/

AjBool ajFileHasDir(const AjPStr name)
{
    if(ajStrFindC(name, SLASH_STRING) < 0)
	return ajFalse;

    return ajTrue;
}




/* @func ajFileNewF ***********************************************************
**
** Creates a new file object from an open C file.
**
** @param [u] file [FILE*] C file.
** @return [AjPFile] New file object.
** @category new [AjPFile] Constructor using an existing open file,
**                         for example stdin or stdout
** @@
******************************************************************************/

AjPFile ajFileNewF(FILE* file)
{
    AjPFile thys;

    if(!file)
	ajFatal("Trying to create an AJAX file from a bad C RTL FILE*");

    AJNEW0(thys);
    thys->fp     = file;
    thys->Handle = ++fileHandle;
    thys->Name   = ajStrNew();
    thys->End    = ajFalse;

    fileOpenCnt++;
    fileOpenTot++;
    if(fileOpenCnt > fileOpenMax)
	fileOpenMax = fileOpenCnt;

    return thys;
}




/* ==================================================================== */
/* =========================== destructor ============================= */
/* ==================================================================== */

/* @section Outfile Destructors ***********************************************
**
** Destruction is achieved by closing the file.
**
** Unlike ANSI C, there are tests to ensure a file is not closed twice.
**
******************************************************************************/

/* @func ajOutfileClose *******************************************************
**
** Close and free an outfile object.
**
** @param [d] pthis [AjPOutfile*] Output file.
** @return [void]
** @category delete [AjPOutfile] Close file and descruct
** @@
******************************************************************************/

void ajOutfileClose(AjPOutfile* pthis)
{
    AjPOutfile thys;

    thys = pthis ? *pthis : 0;

    if(!pthis)
	return;
    if(!*pthis)
	return;

    fileClose(thys->File);
    AJFREE(thys->File);
    ajStrDel(&thys->Type);
    ajStrDel(&thys->Formatstr);
    AJFREE(*pthis);

    return;
}


/* @func ajOutfileDel *******************************************************
**
** Close and free an outfile object.
**
** @param [d] pthis [AjPOutfile*] Output file.
** @return [void]
** @category delete [AjPOutfile] Default destructor
** @@
******************************************************************************/

void ajOutfileDel(AjPOutfile* pthis)
{
    ajOutfileClose(pthis);
    return;
}




/* @section Directory Destructors *********************************************
**
** Destruction is achieved by deleting the object.
**
******************************************************************************/

/* @func ajDirDel *******************************************************
**
** Close and free a directory object.
**
** @param [d] pthis [AjPDir*] Directory object.
** @return [void]
** @category delete [AjPDir] Default destructor
** @@
******************************************************************************/

void ajDirDel(AjPDir* pthis)
{
    AjPDir thys;

    thys = *pthis;
    if(!thys)
	return;

    ajStrDel(&thys->Name);
    ajStrDel(&thys->Prefix);
    ajStrDel(&thys->Extension);
    AJFREE(*pthis);

    return;
}



/* @func ajDiroutDel *******************************************************
**
** Close and free a directory object.
**
** @param [d] pthis [AjPDir*] Directory object.
** @return [void]
** @category delete [AjPDir] Default destructor for output directory
** @@
******************************************************************************/

void ajDiroutDel(AjPDir* pthis)
{
    ajDirDel(pthis);
    return;
}



/* @section File Destructors **************************************************
**
** Destruction is achieved by closing the file.
**
** Unlike ANSI C, there are tests to ensure a file is not closed twice.
**
******************************************************************************/




/* @func ajFileClose **********************************************************
**
** Close and free a file object.
**
** @param [d] pthis [AjPFile*] File.
** @return [void]
** @category delete [AjPFile] Default destructor
** @@
******************************************************************************/

void ajFileClose(AjPFile* pthis)
{
    AjPFile thys;

    thys = pthis ? *pthis : 0;

    if(!pthis)
	return;
    if(!*pthis)
	return;

    fileClose(thys);
    AJFREE(*pthis);

    return;
}




/* @func ajFileOutClose *******************************************************
**
** Closes and deletes an output file object.
**
** @param [d] pthis [AjPFile*] Output file.
** @return [void]
** @category delete [AjPFile] Destructor and writes file closing message at end
** @@
******************************************************************************/

void ajFileOutClose(AjPFile* pthis)
{
    AjPFile thys;

    thys = pthis ? *pthis : 0;

    ajFmtPrintF(thys, "Standard output close ...\n");
    ajFileClose(pthis);

    return;
}




/* @funcstatic fileClose ******************************************************
**
** Closes a file object. Used as part of the public destructor and
** other public functions.
**
** @param [w] thys [AjPFile] File.
** @return [void]
** @@
******************************************************************************/

static void fileClose(AjPFile thys)
{
    int status = 0;
#ifndef WIN32
    pid_t retval;
#endif

    if(!thys)
	return;

#ifndef WIN32
    if (thys->Pid)
    {
	ajDebug("fileClose waiting for waitpid for pid  %d\n",
		thys->Pid);
	while((retval=waitpid(thys->Pid,&status,WNOHANG))!= thys->Pid)
	{
	    sleep(1);

	    /*ajDebug("fileClose waitpid returns %d status %d\n",
		    retval, status);*/
	    if(retval == -1)
		if(errno != EINTR)
		    break;
	    status = 0;
	}
    }
#endif

    if(thys->Handle)
    {
	ajDebug("closing file '%F'\n", thys);
	if(thys->fp)
	{
	    if(thys->fp != stdout && thys->fp != stderr)
	    {
		if(fclose(thys->fp))
		    ajFatal("File close problem in fileClose");
	    }
	}
	thys->Handle = 0;

	fileCloseCnt++;
	fileOpenCnt--;
    }
    else
	ajDebug("file already closed\n");

    ajStrDel(&thys->Name);
    ajStrDel(&thys->Buff);
    ajListstrFreeData(&thys->List);

    return;
}




/* ==================================================================== */
/* ========================== Assignments ============================= */
/* ==================================================================== */

/* @section File Assignments **************************************************
**
** These functions overwrite the file provided as the first argument
**
******************************************************************************/




/* @func ajFileDataNew ********************************************************
**
** Returns an allocated AjFileInNew pointer (AjPFile) if file exists
** a) in .   b) in ./.embossdata c) ~/ d) ~/.embossdata e) $DATA
**
** @param [r] tfile [const AjPStr] Filename in AjStr.
** @param [w] fnew [AjPFile*] file pointer.
** @return [void]
** @@
******************************************************************************/

void ajFileDataNew(const AjPStr tfile, AjPFile *fnew)
{
    char *p;
    
    if(tfile == NULL)
	return;
    
    ajStrAssignS(&fileBaseTmp, tfile);
    ajDebug("ajFileDataNew trying '%S'\n", fileBaseTmp);
    if(ajFileStat(fileBaseTmp, AJ_FILE_R))
    {
	*fnew = ajFileNewIn(fileBaseTmp);
	ajStrDelStatic(&fileBaseTmp);
	return;
    }
    
    ajStrAssignC(&fileNameTmp, ".embossdata");
    ajStrAppendC(&fileNameTmp, SLASH_STRING);
    ajStrAppendS(&fileNameTmp, fileBaseTmp);
    ajDebug("ajFileDataNew trying '%S'\n", fileNameTmp);
    if(ajFileStat(fileNameTmp, AJ_FILE_R))
    {
	*fnew = ajFileNewIn(fileNameTmp);
	ajStrDelStatic(&fileBaseTmp);
	ajStrDelStatic(&fileNameTmp);
	return;
    }
    
    
    if((p=getenv("HOME")))
    {
	ajStrAssignC(&fileHomeTmp,p);
	ajStrAppendC(&fileHomeTmp,SLASH_STRING);
	ajStrAppendS(&fileHomeTmp,fileBaseTmp);
	ajDebug("ajFileDataNew trying '%S'\n", fileHomeTmp);
	if(ajFileStat(fileHomeTmp, AJ_FILE_R))
	{
	    *fnew = ajFileNewIn(fileHomeTmp);
	    ajStrDelStatic(&fileHomeTmp);
	    ajStrDelStatic(&fileBaseTmp);
	    ajStrDelStatic(&fileNameTmp);
	    return;
	}

	ajStrAssignC(&fileHomeTmp,p);
	ajStrAppendC(&fileHomeTmp, "/.embossdata");
	ajStrAppendC(&fileHomeTmp, SLASH_STRING);
	ajStrAppendS(&fileHomeTmp,fileBaseTmp);
	ajDebug("ajFileDataNew trying '%S'\n", fileHomeTmp);
	if(ajFileStat(fileHomeTmp, AJ_FILE_R))
	{
	    *fnew = ajFileNewIn(fileHomeTmp);
	    ajStrDelStatic(&fileHomeTmp);
	    ajStrDelStatic(&fileBaseTmp);
	    ajStrDelStatic(&fileNameTmp);
	    return;
	}
	ajStrDelStatic(&fileHomeTmp);
    }
    
    if(ajNamGetValueC("DATA", &fileNameTmp))
    {
        ajFileDirFix(&fileNameTmp);
	ajStrAppendS(&fileNameTmp,fileBaseTmp);
	ajDebug("ajFileDataNew trying '%S'\n", fileNameTmp);
	if(ajFileStat(fileNameTmp, AJ_FILE_R))
	{
	    *fnew = ajFileNewIn(fileNameTmp);
	    ajStrDelStatic(&fileBaseTmp);
	    ajStrDelStatic(&fileNameTmp);
	    return;
	}
    }
    
    /* just EMBOSS/data under installation */
    if(ajNamRootInstall(&fileNameTmp))
    {
        ajNamRootPack(&filePackTmp);		/* just EMBOSS */
	ajFileDirFix(&fileNameTmp);
	ajStrAppendC(&fileNameTmp,"share/");
	ajStrAppendS(&fileNameTmp,filePackTmp);
	ajStrAppendC(&fileNameTmp,"/data/");
	ajStrAppendS(&fileNameTmp,fileBaseTmp);
	ajDebug("ajFileDataNew trying '%S'\n", fileNameTmp);
	if(ajFileStat(fileNameTmp, AJ_FILE_R))
	{
	    *fnew = ajFileNewIn(fileNameTmp);
	    ajStrDelStatic(&fileBaseTmp);
	    ajStrDelStatic(&fileNameTmp);
	    return;
	}
    }
    
    /* just emboss/data under source */
    if(ajNamRoot(&fileNameTmp))
    {
	ajStrAppendC(&fileNameTmp,"/data/");
	ajStrAppendS(&fileNameTmp,fileBaseTmp);
	ajDebug("ajFileDataNew trying '%S'\n", fileNameTmp);
	if(ajFileStat(fileNameTmp, AJ_FILE_R))
	{
	    *fnew = ajFileNewIn(fileNameTmp);
	    ajStrDelStatic(&fileBaseTmp);
	    ajStrDelStatic(&fileNameTmp);
	    return;
	}
    }
    
    ajStrDelStatic(&fileBaseTmp);
    ajStrDelStatic(&fileNameTmp);
    
    ajDebug("ajFileDataNew failed to find '%S'\n", fileNameTmp);
    *fnew = NULL;
    
    return;
}




/* @func ajFileDataNewC *******************************************************
**
** Returns an allocated AjFileInNew pointer (AjPFile) if file exists
** a) in .   b) in ./.embossdata c) ~/ d) ~/.embossdata e) $DATA
**
** @param [r] s [const char*] Filename.
** @param [w] f [AjPFile*] file pointer.
** @return [void]
** @@
******************************************************************************/

void ajFileDataNewC(const char *s, AjPFile *f)
{
    AjPStr t;

    t = ajStrNewC(s);
    ajFileDataNew(t,f);
    ajStrDel(&t);

    return;
}




/* @func ajFileDataDirNew *****************************************************
**
** Returns an allocated AjFileInNew pointer (AjPFile) if file exists
** in the EMBOSS/data/(dir) directory, or is found in the usual directories
** by ajFileDataNew
**
** @param [r] tfile [const AjPStr] Filename in AjStr.
** @param [r] dir [const AjPStr] Data directory name in AjStr.
** @param [w] fnew [AjPFile*] file pointer.
** @return [void]
** @@
******************************************************************************/

void ajFileDataDirNew(const AjPStr tfile, const AjPStr dir, AjPFile *fnew)
{
    if(ajNamGetValueC("DATA", &fileNameTmp))
    {
        ajFileDirFix(&fileNameTmp);
	if(ajStrGetLen(dir))
	{
	    ajStrAppendS(&fileNameTmp,dir);
	    ajFileDirFix(&fileNameTmp);
	}
	ajStrAppendS(&fileNameTmp,tfile);
	ajDebug("ajFileDataDirNew trying '%S'\n", fileNameTmp);
	if(ajFileStat(fileNameTmp, AJ_FILE_R))
	{
	    *fnew = ajFileNewIn(fileNameTmp);
	    ajStrDelStatic(&fileNameTmp);
	    return;
	}
    }
    
    /* just EMBOSS/data under installation */
    if(ajNamRootInstall(&fileNameTmp))
    {
	/* just EMBOSS */
        ajNamRootPack(&filePackTmp);
	ajFileDirFix(&fileNameTmp);
	ajStrAppendC(&fileNameTmp,"share/");
	ajStrAppendS(&fileNameTmp,filePackTmp);
	ajStrAppendC(&fileNameTmp,"/data/");
	if(ajStrGetLen(dir))
	{
	    ajStrAppendS(&fileNameTmp,dir);
	    ajFileDirFix(&fileNameTmp);
	}
	ajStrAppendS(&fileNameTmp,tfile);
	ajDebug("ajFileDataDirNew trying '%S'\n", fileNameTmp);
	if(ajFileStat(fileNameTmp, AJ_FILE_R))
	{
	    *fnew = ajFileNewIn(fileNameTmp);
	    ajStrDelStatic(&filePackTmp);
	    ajStrDelStatic(&fileNameTmp);
	    return;
	}
    }

    /* just emboss/data under source */
    if(ajNamRoot(&fileNameTmp))
    {
	ajStrAppendC(&fileNameTmp,"/data/");
	if(ajStrGetLen(dir))
	{
	    ajStrAppendS(&fileNameTmp,dir);
	    ajFileDirFix(&fileNameTmp);
	}
	ajStrAppendS(&fileNameTmp,tfile);
	ajDebug("ajFileDataDirNew trying '%S'\n", fileNameTmp);
	if(ajFileStat(fileNameTmp, AJ_FILE_R))
	{
	    *fnew = ajFileNewIn(fileNameTmp);
	    ajStrDelStatic(&filePackTmp);
	    ajStrDelStatic(&fileNameTmp);
	    return;
	}
    }
    
    ajStrDelStatic(&filePackTmp);
    ajStrDelStatic(&fileNameTmp);
    
    *fnew = NULL;
    
    ajFileDataNew(tfile, fnew);
    
    return;
}




/* @func ajFileDataDirNewC ****************************************************
**
** Returns an allocated AjFileInNew pointer (AjPFile) if file exists
** in the EMBOSS/data/(dir) directory, or is found in the usual directories
** by ajFileDataNew
**
** @param [r] s [const char*] Filename
** @param [r] d [const char*] Data directory name.
** @param [w] f [AjPFile*] file pointer.
** @return [void]
** @@
******************************************************************************/

void ajFileDataDirNewC(const char *s, const char* d, AjPFile *f)
{
    AjPStr t;
    AjPStr u;

    t = ajStrNewC(s);
    u = ajStrNewC(d);
    ajFileDataDirNew(t,u,f);
    ajStrDel(&t);
    ajStrDel(&u);

    return;
}




/* ==================================================================== */
/* =========================== Modifiers ============================== */
/* ==================================================================== */

/* @section File Modifiers ****************************************************
**
** These functions use the contents of a file object and update them.
**
******************************************************************************/




/* @func ajFileSeek ***********************************************************
**
** Sets the current position in an open file.
**
** Resets the end-of-file flag End for cases where end-of-file was
** reached and then we seek back somewhere in the file.
**
** @param [u] thys [AjPFile] File.
** @param [r] offset [ajlong] Offset
** @param [r] wherefrom [ajint] Start of offset, as defined for 'fseek'.
** @return [ajint] Result of 'fseek'
** @@
******************************************************************************/

ajint ajFileSeek(AjPFile thys, ajlong offset, ajint wherefrom)
{
    ajint ret;

    clearerr(thys->fp);
    ret = fseek(thys->fp, offset, wherefrom);

    if(feof(thys->fp))
    {
	thys->End = ajTrue;
	ajDebug("EOF ajFileSeek file %F\n", thys);
    }
    else
	thys->End = ajFalse;

    return ret;
}




/* @func ajFileRead ***********************************************************
**
** Binary read from an input file object using the C 'fread' function.
**
** @param [w] ptr [void*] Buffer for output.
** @param [r] element_size [size_t] Number of bytes per element.
** @param [r] count [size_t] Number of elements to read.
** @param [u] thys [AjPFile] Input file.
** @return [size_t] Return value from 'fread'
** @@
******************************************************************************/

size_t ajFileRead(void* ptr, size_t element_size, size_t count,
		  AjPFile thys)
{
    return fread(ptr, element_size, count, thys->fp);
}




/* @func ajFileReadUint *******************************************************
**
** Binary read of an unsigned integer from an input file object using
** the C 'fread' function. Converts from a specified endianism.
**
** @param [u] thys [AjPFile] Input file.
** @param [r] Bigendian [AjBool] Big endian or not.
** @return [ajuint] Converted integer value
** @@
******************************************************************************/

ajuint ajFileReadUint(AjPFile thys, AjBool Bigendian)
{
    static ajint called  = 0;
    static AjBool bigend = AJFALSE;
    ajuint ret;
    ajint ret2;

    if(!called)
	bigend = ajUtilGetBigendian();

    fread(&ret, 4, 1, thys->fp);
    if(Bigendian && bigend)
	return ret;
    else if(!Bigendian && !bigend)
	return ret;

    /*ajDebug("Reversed: %u", ret);*/
    ret2 = (ajint) ret;
    ajByteRevLen4(&ret2);
    ret = (ajuint) ret2;
    /*ajDebug(" => %u\n", ret);*/

    return ret;
}




/* @func ajFileWrite **********************************************************
**
** Binary write to an output file object using the C 'fwrite' function.
**
** @param [u] thys [AjPFile] Output file.
** @param [r] ptr [const void*] Buffer for output.
** @param [r] element_size [size_t] Number of bytes per element.
** @param [r] count [size_t] Number of elements to write.
** @return [size_t] Return value from 'fwrite'
** @@
******************************************************************************/

size_t ajFileWrite(AjPFile thys, const void* ptr,
		   size_t element_size, size_t count)
{
    return fwrite(ptr, element_size, count, thys->fp);
}




/* @func ajFileNext ***********************************************************
**
** Given a file object that includes a list of input files, closes the
** current input file and opens the next one.
**
** @param [u] thys [AjPFile] File object.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajFileNext(AjPFile thys)
{
    static AjPStr name = NULL;

    if(!thys->List)
    {
	ajDebug("ajFileNext for non-list file %F\n", thys);
	return ajFalse;
    }

    ajDebug("ajFileNext for non-list file %F name '%S'\n", thys, thys->Name);
    ajListTrace(thys->List);
    if(!ajListPop(thys->List, (void*) &name))
    {
	/* end of list */
	ajDebug("ajFileNext failed - list completed\n");
	return ajFalse;
    }

    ajDebug("ajFileNext filename '%S'\n", name);
    if(!ajFileReopen(thys, name))
    {
	/* popped from the list */
	ajStrDel(&name);
	return ajFalse;
    }

    /* popped from the list */
    ajStrDel(&name);
    thys->End = ajFalse;

    ajDebug("ajFileNext success\n");

    return ajTrue;
}




/* @func ajFileReopen *********************************************************
**
** Reopens a file with a new name.
**
** @param [u] thys [AjPFile] Input file.
** @param [r] name [const AjPStr] name of file.
** @return [FILE*] copy of file pointer
** @@
******************************************************************************/

FILE* ajFileReopen(AjPFile thys, const AjPStr name)
{
    ajStrAssignS(&thys->Name, name);
    return freopen(ajStrGetPtr(thys->Name), "rb", thys->fp);
}




/* @func ajFileReadLine *******************************************************
**
** Reads a line from the input file, removing any trailing newline.
**
** @param [u] thys [AjPFile] Input file.
** @param [w] pdest [AjPStr*] Buffer to hold the current line.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajFileReadLine(AjPFile thys, AjPStr* pdest)
{
    return ajFileGetsTrim(thys, pdest);
}




/* @func ajFileGetsTrimL ******************************************************
**
** Reads a line from a file and removes any trailing newline.
**
** @param [u] thys [AjPFile] Input file.
** @param [w] pdest [AjPStr*] Buffer to hold the current line.
** @param [w] fpos [ajlong*] File position before the read.
** @return [AjBool] ajTrue on success.
** @category modify [AjPFile] Reads a record from a file and removes
**                            newline characters
** @@
******************************************************************************/

AjBool ajFileGetsTrimL(AjPFile thys, AjPStr* pdest, ajlong* fpos)
{
    AjBool ok;
    AjPStr dest;

    ok = ajFileGetsL(thys, pdest, fpos);

    if(!ok)
	return ajFalse;

    /* trim any trailing newline */

    dest = *pdest;

    /*ajDebug("Remove carriage-return characters from PC-style files\n");*/
    if(ajStrGetCharLast(dest) == '\n')
	ajStrCutEnd(pdest, 1);

    /* PC files have \r\n Macintosh files have just \r : this fixes both */
    if(ajStrGetCharLast(dest) == '\r')
	ajStrCutEnd(pdest, 1);

    return ajTrue;
}




/* @func ajFileGetsTrim *******************************************************
**
** Reads a line from a file and removes any trailing newline.
**
** @param [u] thys [AjPFile] Input file.
** @param [w] pdest [AjPStr*] Buffer to hold the current line.
** @return [AjBool] ajTrue on success.
** @category modify [AjPFile] Reads a record from a file and removes
**                            newline characters
** @@
******************************************************************************/

AjBool ajFileGetsTrim(AjPFile thys, AjPStr* pdest)
{
    ajlong fpos=0;

    return ajFileGetsTrimL(thys, pdest, &fpos);
}




/* @func ajFileGets ***********************************************************
**
** Reads a line from a file and returns the initial file position.
**
** @param [u] thys [AjPFile] Input file.
** @param [w] pdest [AjPStr*] Buffer to hold the current line.
** @return [AjBool] ajTrue on success.
** @category modify [AjPFile] Reads a record from a file
** @@
******************************************************************************/

AjBool ajFileGets(AjPFile thys, AjPStr* pdest)
{
    ajlong fpos = 0;

    return ajFileGetsL(thys, pdest, &fpos);
}




/* @func ajFileGetsL **********************************************************
**
** Reads a line from a file.
**
** @param [u] thys [AjPFile] Input file.
** @param [w] pdest [AjPStr*] Buffer to hold the current line.
** @param [w] fpos [ajlong*] File position before the read.
** @return [AjBool] ajTrue on success.
** @category modify [AjPFile] Reads a record from a file
** @@
******************************************************************************/

AjBool ajFileGetsL(AjPFile thys, AjPStr* pdest, ajlong* fpos)
{
    const char *cp;
    char *buff;
    ajint isize;
    ajint ilen;
    ajint jlen;
    ajint ipos;
    
    ajStrSetRes(&thys->Buff, fileBuffSize);
    buff  = ajStrGetuniquePtr(&thys->Buff);
    isize = ajStrGetRes(thys->Buff);
    ilen  = 0;
    ipos  = 0;
    
    if(!thys->fp)
	ajWarn("ajFileGets file not found");
    
    while(buff)
    {
	if(thys->End)
	{
	    ajDebug("at EOF: File already read to end %F\n", thys);
	    return ajFalse;
	}
	
	*fpos = ajFileTell(thys);

#ifndef __ppc__
	cp = fgets(&buff[ipos], isize, thys->fp);
#else
	cp = ajSysFuncFgets(&buff[ipos], isize, thys->fp);
#endif

        if(!cp && !ipos)
	{
	    if(feof(thys->fp))
	    {
		thys->End = ajTrue;
		ajStrAssignC(pdest, "");
		ajDebug("EOF ajFileGetsL file %F\n", thys);
		return ajFalse;
	    }
	    else
		ajFatal("Error reading from file '%s'\n", ajFileName(thys));
	}

	jlen = strlen(&buff[ipos]);
	ilen += jlen;

	/*
	 ** We need to read again if:
	 ** We have read the entire buffer
	 ** and we don't have a newline at the end
	 ** (must be careful about that - we may just have read enough)
	 */
	ajStrSetValidLen(&thys->Buff, ilen);
	if((jlen == (isize-1)) &&
	   (ajStrGetCharLast(thys->Buff) != '\n'))
	{
	    ajStrSetRes(&thys->Buff, ajStrGetRes(thys->Buff)+fileBuffSize);
	    ajDebug("more to do: jlen: %d ipos: %d isize: %d ilen: %d "
		    "Size: %d\n",
		    jlen, ipos, isize, ilen, ajStrGetRes(thys->Buff));
	    ipos += jlen;
	    buff = ajStrGetuniquePtr(&thys->Buff);
	    isize = ajStrGetRes(thys->Buff) - ipos;
	    ajDebug("expand to: ipos: %d isize: %d Size: %d\n",
		    ipos, isize, ajStrGetRes(thys->Buff));

	}
	else
	    buff = NULL;
    }
    
    ajStrSetValidLen(&thys->Buff, ilen);
    if (ajStrGetCharLast(thys->Buff) != '\n')
    {
	ajDebug("Appending missing newline to '%S'\n", thys->Buff);
	ajStrAppendK(&thys->Buff, '\n');
    }
    ajStrAssignS(pdest, thys->Buff);
 
    return ajTrue;
}




/* @func ajFileUnbuffer *******************************************************
**
** Turns off system buffering of an output file, for example to allow
** debug output to appear even in the event of a program abort.
**
** @param [u] thys [AjPFile] File object.
** @return [void]
** @@
******************************************************************************/

void ajFileUnbuffer(AjPFile thys)
{
    setbuf(thys->fp, NULL);

    return;
}




/* @func ajFileReadAppend *****************************************************
**
** Reads a record from a file and appends it to the user supplied buffer.
**
** @param [u] thys [AjPFile] Input file.
** @param [w] pbuff [AjPStr*] Buffer to hold results.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajFileReadAppend(AjPFile thys, AjPStr* pbuff)
{
    static AjPStr locbuff = 0;
    AjBool ok;

    if(!locbuff)
	locbuff = ajStrNew();

    ok = ajFileGets(thys, &locbuff);

    if(ok)
	ajStrAppendS(pbuff, locbuff);

    return ok;
}




/* @func ajFileOutHeader ******************************************************
**
** Writes a header record to the output file.
**
** @param [u] thys [AjPFile] Output file.
** @return [void]
** @@
******************************************************************************/

void ajFileOutHeader(AjPFile thys)
{
    ajFmtPrintF(thys, "Standard output header ...\n");

    return;
}




/* @func ajFileNameShorten ****************************************************
**
** Truncates a filename to a basic file name.
**
** @param [uP] fname [AjPStr*] File name
** @return [AjBool] ajTrue on success, and returns a filename.
**                  ajFalse on failure, and returns an empty string.
** @@
******************************************************************************/

AjBool ajFileNameShorten(AjPStr* fname)
{
    /* entryname at end */
    if(!fileEntryExp)
	fileEntryExp = ajRegCompC(":([A-Za-z0-9_-]+)$");

    if(ajRegExec(fileEntryExp, *fname))
    {
	ajRegSubI(fileEntryExp, 1, fname);
	return ajTrue;
    }

    /* name.ext */
    if(!fileFileExp)
	fileFileExp = ajRegCompC("([A-Za-z0-9_-]+)[.][A-Za-z0-9_-]+$");

    if(ajRegExec(fileFileExp, *fname))
    {
	ajRegSubI(fileFileExp, 1, fname);
	return ajTrue;
    }

    /* last valid word */
    if(!fileRestExp)
	fileRestExp = ajRegCompC("([A-Za-z0-9_-]+)[^A-Za-z0-9_-]*$");

    if(ajRegExec(fileRestExp, *fname))
    {
	ajRegSubI(fileRestExp, 1, fname);
	return ajTrue;
    }

    ajStrAssignC(fname, "");

    return ajFalse;
}




/* @func ajFileNameTrim *******************************************************
**
** Truncates a filename to a basic file name.extension
**
** @param [uP] fname [AjPStr*] File name
** @return [AjBool] ajTrue on success, and returns a filename.
**                  ajFalse on failure, and returns an empty string.
** @@
******************************************************************************/

AjBool ajFileNameTrim(AjPStr* fname)
{
    char *p;

    if((p = strrchr(ajStrGetPtr(*fname),(ajint)SLASH_CHAR)))
    {
	ajStrAssignC(&fileTmpStr,p+1);
	ajStrAssignS(fname,fileTmpStr);
    }

    return ajTrue;
}




/* @func ajFileDataNewWrite ***************************************************
**
** Returns an allocated AjFileNewOut pointer (AjPFile) to a file in the
** emboss_DATA area
**
** @param [r] tfile [const AjPStr] Filename in AjStr.
** @param [w] fnew [AjPFile*] file pointer.
** @return [void]
** @@
******************************************************************************/

void ajFileDataNewWrite(const AjPStr tfile, AjPFile *fnew)
{
    if (tfile == NULL) return;
    
    if(ajNamGetValueC("DATA", &fileNameTmp))
    {
	/* also does ajFileDirFix */
	if(!ajFileDir(&fileNameTmp))
	{
	    ajNamRootPack(&filePackTmp);
	    ajFatal("%S_DATA directory not found: %S\n",
		    filePackTmp, fileNameTmp);
	}
	ajStrAppendS(&fileNameTmp,tfile);
	if(!(*fnew = ajFileNewOut(fileNameTmp)))
	    ajFatal("Cannot write to file %S\n",fileNameTmp);
	ajStrDel(&fileNameTmp);

	return;
    }
    
    /* just emboss/data under installation */
    if(ajNamRootInstall(&fileNameTmp))
    {
	/* just EMBOSS */
        ajNamRootPack(&filePackTmp);
	ajFileDirFix(&fileNameTmp);
	ajStrAppendC(&fileNameTmp,"share/");
	ajStrAppendS(&fileNameTmp,filePackTmp);
	ajStrAppendC(&fileNameTmp,"/data/");

	/* if we are installed, else see below */
	if(ajFileDir(&fileNameTmp))
	{
	    ajStrAppendS(&fileNameTmp,tfile);
	    if(!(*fnew = ajFileNewOut(fileNameTmp)))
		ajFatal("Cannot write to file %S\n",fileNameTmp);
	    ajStrDel(&fileNameTmp);
	    return;
	}
    }
    
    /* just emboss/data under source */
    if(ajNamRoot(&fileNameTmp))
    {
	ajStrAppendC(&fileNameTmp,"/data/");
	if(!ajFileDir(&fileNameTmp))
	    ajFatal("Not installed, and source data directory not found: %S\n",
		    fileNameTmp);
	ajStrAppendS(&fileNameTmp,tfile);
	if(!(*fnew = ajFileNewOut(fileNameTmp)))
	    ajFatal("Cannot write to file %S\n",fileNameTmp);
	ajStrDel(&fileNameTmp);
	return;
    }


    ajNamRootPack(&filePackTmp);
    ajFatal("No install or source data directory, and %S_DATA not defined\n",
	    filePackTmp);
    ajStrDelStatic(&fileNameTmp);
    *fnew = NULL;
    
    return;
}




/* ==================================================================== */
/* ======================== Operators ==================================*/
/* ==================================================================== */

/* @section File Operators ****************************************************
**
** These functions use the contents of a file object but do not make
** any changes.
**
******************************************************************************/




/* @func ajFileDir ************************************************************
**
** Checks that a string is a valid existing directory, and appends a
** trailing '/' if it is missing.
**
** @param [u] dir [AjPStr*] Directory path
** @return [AjBool] true if a valid directory.
** @@
******************************************************************************/

AjBool ajFileDir(AjPStr* dir)
{
    DIR* odir;

    odir = fileOpenDir(dir);	/* appends trailing slash if needed */
    if(!odir)
	return ajFalse;

    closedir(odir);

    return ajTrue;
}




/* @func ajFileDirPath ********************************************************
**
** Checks that a string is a valid directory, and makes sure it has the
** full path definition.
**
** @param [u] dir [AjPStr*] Directory path
** @return [AjBool] true if a valid directory.
** @@
******************************************************************************/

AjBool ajFileDirPath(AjPStr* dir)
{
    DIR* odir;
    
    ajDebug("ajFileDirPath '%S'\n", *dir);

    /* appends trailing slash if needed */
    odir = fileOpenDir(dir);
    if(!odir)
	return ajFalse;
    free(odir);

    ajDebug("So far '%S'\n", *dir);

    /* full path already */
    if(*ajStrGetPtr(*dir) == SLASH_CHAR)
	return ajTrue;

    ajFileGetwd(&fileCwd);

    /* current directory */
    if(ajStrMatchC(*dir, CURRENT_DIR))
    {
	ajStrAssignS(dir, fileCwd);
	ajDebug("Current '%S'\n", *dir);
	return ajTrue;
    }

    /*  going up */
    while(ajStrPrefixC(*dir, UP_DIR))
    {
	ajFileDirUp(&fileCwd);
	ajStrKeepRange(dir, 3, -1);
	ajDebug("Going up '%S' '%S'\n", *dir, fileCwd);
    }

    ajStrInsertS(dir, 0, fileCwd);

    ajDebug("Full path '%S'\n", *dir);

    return ajTrue;
}




/* @func ajFileGetwd **********************************************************
**
** Returns the current directory
**
** @param [w] dir [AjPStr*] Directory name.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajFileGetwd(AjPStr* dir)
{
    char cwd[PATH_MAX+1];

    if(!getcwd(cwd,PATH_MAX))
    {
	ajStrDelStatic(dir);
	return ajFalse;
    }

    if(ajCharSuffixC(cwd, SLASH_STRING))
	ajStrAssignC(dir, cwd);
    else
	ajFmtPrintS(dir, "%s%s", cwd, SLASH_STRING);

    return ajTrue;
}




/* @func ajFileDirUp **********************************************************
**
** Changes directory name to one level up
**
** @param [u] dir [AjPStr*] Directory name.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajFileDirUp(AjPStr* dir)
{
    AjPStr tmpdir = NULL;
    AjBool modded = ajFalse;
    
    const char *p;
    const char *q;
    ajint len;
    
    ajStrAssignS(&tmpdir, *dir);
    len = ajStrGetLen(tmpdir);
    p   = ajStrGetPtr(tmpdir);
    q   = p + len -2;
    if(q > p)
    {
	while(q!=p && *q==SLASH_CHAR)
	    --q;
	while(q!=p && *q!=SLASH_CHAR)
	    --q;

	if(q-p > 1)
	{
	    ajStrAssignSubC(dir,p,0,q-p);
	    modded = ajTrue;
	}
   }
    
    ajStrDel(&tmpdir);

    return modded;
}




/* @funcstatic fileOpenDir ****************************************************
**
** Runs 'opendir' on the specified directory. If the directory name
** has no trailing slash (on Unix) then one is added. This is why the
** directory name must be writeable.
**
** @param [u] dir [AjPStr*] Directory name.
** @return [DIR*] result of the opendir call.
** @@
******************************************************************************/

static DIR* fileOpenDir(AjPStr* dir)
{
    AjBool moved = ajFalse;

    if(ajStrGetCharLast(*dir) != SLASH_CHAR)
	ajStrAppendC(dir, SLASH_STRING);

    /*  going up */
    while(ajStrPrefixC(*dir, UP_DIR))
    {
	if(!moved)
	    ajFileGetwd(&fileCwd);
	moved = ajTrue;
	ajFileDirUp(&fileCwd);
	ajStrKeepRange(dir, 3, -1);
	ajDebug("Going up '%S' '%S'\n", *dir, fileCwd);
    }

    if(moved)
	ajStrInsertS(dir, 0, fileCwd);

    ajDebug("fileOpenDir opened '%S'\n", *dir);

    return opendir(ajStrGetPtr(*dir));
}




/* @func ajFileDirFix *********************************************************
**
** If the directory name has no trailing slash (on Unix) then one is
** added. This is why the directory name must be writeable.
**
** @param [u] dir [AjPStr*] Directory name.
** @return [void]
** @@
******************************************************************************/

void ajFileDirFix(AjPStr* dir)
{
    if(ajStrGetCharLast(*dir) != SLASH_CHAR)
	ajStrAppendC(dir, SLASH_STRING);

    return;
}




/* @func ajFileExit ***********************************************************
**
** Prints a summary of file usage with debug calls
**
** @return [void]
** @@
******************************************************************************/

void ajFileExit(void)
{
    ajDebug("File usage : %d opened, %d closed, %d max, %d total\n",
	     fileOpenCnt, fileCloseCnt, fileOpenMax, fileOpenTot);

    ajStrDel(&fileNameFix);
    ajStrDel(&fileBaseTmp);
    ajStrDel(&fileHomeTmp);
    ajStrDel(&fileNameTmp);
    ajStrDel(&filePackTmp);
    ajStrDel(&fileDirfixTmp);
    ajStrDel(&fileCwd);
    ajStrDel(&fileTmpStr);
    ajStrDel(&fileTempFilename);

    ajRegFree(&fileUserExp);
    ajRegFree(&fileWildExp);
    ajRegFree(&fileEntryExp);
    ajRegFree(&fileFileExp);
    ajRegFree(&fileRestExp);
    ajRegFree(&fileDirExp);
    ajRegFree(&fileFilenameExp);

    return;
}




/* @func ajFileTrace **********************************************************
**
** Writes debug messages to trace the contents of a file object.
**
** @param [r] thys [const AjPFile] File.
** @return [void]
** @@
******************************************************************************/

void ajFileTrace(const AjPFile thys)
{
    ajDebug("File: '%S'\n", thys->Name);
    ajDebug("  handle:  %d\n", thys->Handle);

    return;
}




/* ==================================================================== */
/* ============================ Casts ==================================*/
/* ==================================================================== */

/* @section Outfile Casts *****************************************************
**
** These functions examine the contents of an outfile object and return some
** derived information. Some of them provide access to the internal
** components of a file object. They are provided for programming convenience
** but should be used with caution.
**
******************************************************************************/

/* @func ajOutfileFile ********************************************************
**
** Returns the AjPFile for an AjPOutfile object
**
** @param [r] thys [const AjPOutfile] Outfile object
** @return [AjPFile] AjPFile object
******************************************************************************/

AjPFile ajOutfileFile (const AjPOutfile thys)
{
    return thys->File;
}



/* @func ajOutfileFp ********************************************************
**
** Returns the C FILE* for an AjPOutfile object
**
** @param [r] thys [const AjPOutfile] Outfile object
** @return [FILE*] C file pointer
******************************************************************************/

FILE* ajOutfileFp (const AjPOutfile thys)
{
    return thys->File->fp;
}



/* @func ajOutfileFormat ******************************************************
**
** Returns the C FILE* for an AjPOutfile object
**
** @param [r] thys [const AjPOutfile] Outfile object
** @return [AjPStr] Format name
******************************************************************************/

AjPStr ajOutfileFormat (const AjPOutfile thys)
{
    return thys->Formatstr;
}



/* @section File Casts ********************************************************
**
** These functions examine the contents of a file object and return some
** derived information. Some of them provide access to the internal
** components of a file object. They are provided for programming convenience
** but should be used with caution.
**
******************************************************************************/




/* @func ajFileBuffSize *******************************************************
**
** Returns the standard record buffer size for a file
**
** @return [ajint] File record buffer size
** @@
******************************************************************************/

ajint ajFileBuffSize(void)
{
    return fileBuffSize;
}




/* @func ajFileName ***********************************************************
**
** Returns the file name for a file object. The filename returned is a pointer
** to the real string internally, so the user must take care not to change
** it and cannot trust the value if the file object is deleted.
**
** @param [r] thys [const AjPFile] File.
** @return [const char*] Filename as a C character string.
** @category cast [AjPFile] Returns the filename as char
** @@
******************************************************************************/

const char* ajFileName(const AjPFile thys)
{
    return ajStrGetPtr(thys->Name);
}



/* @func ajFileNameS **********************************************************
**
** Returns the file name for a file object. The filename returned is a pointer
** to the real string internally, so the user must take care not to change
** it and cannot trust the value if the file object is deleted.
**
** @param [r] thys [const AjPFile] File.
** @return [const AjPStr] Filename.
** @category cast [AjPFile] Returns the filename as an AjPStr
** @@
******************************************************************************/

const AjPStr ajFileNameS(const AjPFile thys)
{
    return thys->Name;
}




/* @func ajFileGetApp *********************************************************
**
** Returns the App element for a file object. The App element is True if the 
** file was opened for appending to, False otherwise. 
**
** @param [r] thys [const AjPFile] File.
** @return [AjBool] App element, True if if file was opened for appending to, 
** False otherwise. 
** @@
******************************************************************************/

AjBool ajFileGetApp(const AjPFile thys)
{
    return thys->App;
}





/* @func ajFileGetName ********************************************************
**
** Returns the file name for a file object. The filename returned is a pointer
** to the real string internally, so the user must take care not to change
** it and cannot trust the value if the file object is deleted.
**
** @param [r] thys [const AjPFile] File.
** @return [AjPStr] Filename as a C character string.
** @@
******************************************************************************/

AjPStr ajFileGetName(const AjPFile thys)
{
    return thys->Name;
}




/* @func ajFileStat ***********************************************************
**
** Returns true if file exists and is read or write or executable by the user
** as determined by AJ_FILE_R AJ_FILE_W AJ_FILE_X file modes
**
** @param [r] fname [const AjPStr] Filename.
** @param [r] mode [ajint] file mode.
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFileStat(const AjPStr fname, ajint mode)
{
#if defined(AJ_IRIXLF)
    struct stat64 buf;
#else
    struct stat buf;
#endif

#if defined(AJ_IRIXLF)
    if(!stat64(ajStrGetPtr(fname), &buf))
#else
	if(!stat(ajStrGetPtr(fname), &buf))
#endif
	    if((ajuint)buf.st_mode & mode)
		return ajTrue;

    return ajFalse;
}

/* @func ajFileNameValid ******************************************************
**
** Returns true if file exists and is readable by the user
**
** @param [r] fname [const AjPStr] Filename.
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/


AjBool ajFileNameValid (const AjPStr fname)
{
    return ajFileStat(fname, AJ_FILE_R);
}

/* @func ajFileLength *********************************************************
**
** Returns the length of a file
**
** @param [r] fname [const AjPStr] Filename.
** @return [ajlong] length or -1 if file doesn't exist
** @@
******************************************************************************/

ajlong ajFileLength(const AjPStr fname)
{
#if defined(AJ_IRIXLF)
    struct stat64 buf;
#else
    struct stat buf;
#endif

#if defined(AJ_IRIXLF)
    if(!stat64(ajStrGetPtr(fname), &buf))
	return (ajlong)buf.st_size;
#else
    if(!stat(ajStrGetPtr(fname), &buf))
	return (ajlong)buf.st_size;
#endif

    return -1;
}




/* @func ajFileTell ***********************************************************
**
** Returns the current position in an open file.
**
** @param [r] thys [const AjPFile] File.
** @return [ajlong] Result of 'ftell'
** @@
******************************************************************************/

ajlong ajFileTell(const AjPFile thys)
{
    if(!thys->fp)
	return 0;
    return ftell(thys->fp);
}




/* @func ajFileStdout *********************************************************
**
** Tests whether a file object is really stdout.
**
** @param [r] file [const AjPFile] File object.
** @return [AjBool] ajTrue if the file matches stdout.
** @@
******************************************************************************/

AjBool ajFileStdout(const AjPFile file)
{
    if(file->fp == stdout)
	return ajTrue;

    return ajFalse;
}




/* @func ajFileStderr *********************************************************
**
** Tests whether a file object is really stderr.
**
** @param [r] file [const AjPFile] File object.
** @return [AjBool] ajTrue if the file matches stderr.
** @@
******************************************************************************/

AjBool ajFileStderr(const AjPFile file)
{
    if(file->fp == stderr)
	return ajTrue;

    return ajFalse;
}




/* @func ajFileStdin **********************************************************
**
** Tests whether a file object is really stdin.
**
** @param [r] file [const AjPFile] File object.
** @return [AjBool] ajTrue if the file matches stdin.
** @@
******************************************************************************/

AjBool ajFileStdin(const AjPFile file)
{
    if(file->fp == stdin)
	return ajTrue;

    return ajFalse;
}




/* @func ajFileFp *************************************************************
**
** Returns the C file pointer for an open file.
**
** @param [r] thys [const AjPFile] File.
** @return [FILE*] C file pointer for the file.
** @category cast [AjPFile] Returns the equivalent C file pointer
** @@
******************************************************************************/

FILE* ajFileFp(const AjPFile thys)
{
    return thys->fp;
}




/* @func ajFileEof ************************************************************
**
** Tests whether we have reached end of file already
**
** @param [r] thys [const AjPFile] File
** @return [AjBool] ajTrue if we already set end-of-file
** @@
******************************************************************************/

AjBool ajFileEof(const AjPFile thys)
{
    return thys->End;
}




/* ==================================================================== */
/* ========================= constructors ============================= */
/* ==================================================================== */

/* @section Buffered File Constructors ****************************************
**
** All constructors return a new open file by pointer. It is the responsibility
** of the user to first destroy any previous file pointer. The target pointer
** does not need to be initialised to NULL, but it is good programming practice
** to do so anyway.
**
** To replace or reuse an existing file, see instead
** the {File Assignments} and {File Modifiers} functions.
**
** The range of constructors is provided to allow flexibility in how
** applications can open files to read various kinds of data.
**
******************************************************************************/




/* @func ajFileBuffNewIn ******************************************************
**
** Creates a new buffered input file object with an opened named file.
**
** @param [r] name [const AjPStr] File name.
** @return [AjPFileBuff] New buffered file object.
** @category new [AjPFileBuff] Constructor using a filename
** @@
******************************************************************************/

AjPFileBuff ajFileBuffNewIn(const AjPStr name)
{
    AjPFile file;

    file = ajFileNewIn(name);

    return ajFileBuffNewFile(file);
}




/* @func ajFileBuffNew ********************************************************
**
** Creates a new buffered input file object with an undefined file.
**
** @return [AjPFileBuff] New buffered file object.
** @category new [AjPFileBuff] Default constructor for an input file
** @@
******************************************************************************/

AjPFileBuff ajFileBuffNew(void)
{
    AjPFile file;

    file = ajFileNew();

    return ajFileBuffNewFile(file);
}




/* @func ajFileBuffNewFile ****************************************************
**
** Creates a new buffered input file object from an open file.
**
** @param [u] file [AjPFile] File object to be buffered.
** @return [AjPFileBuff] New buffered file object.
** @@
******************************************************************************/

AjPFileBuff ajFileBuffNewFile(AjPFile file)
{
    AjPFileBuff thys;

    if(!file)
	return NULL;

    AJNEW0(thys);
    thys->File = file;

    thys->Last = thys->Curr = thys->Prev = thys->Lines =  NULL;
    thys->Freelines = thys->Freelast = NULL;
    thys->Pos = thys->Size = 0;

    return thys;
}




/* @func ajFileBuffSetFile ****************************************************
**
** Creates a new buffered input file object from an open file.
**
** The AjPFile pointer is a clone, so we should simply overwrite
** whatever was there before, but we do need to clear the buffer
**
** @param [w] pthys [AjPFileBuff*] Buffered file object.
** @param [u] file [AjPFile] File object to be buffered.
** @param [r] samefile [AjBool] true if the buff currently uses this file
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFileBuffSetFile(AjPFileBuff* pthys, AjPFile file, AjBool samefile)
{
    AjPFileBuff thys;

    if(!file)
    {
	ajFileBuffDel(pthys);
	return ajFalse;
    }

    if(!*pthys)
    {
	*pthys = ajFileBuffNewFile(file);
	thys = *pthys;
	return ajTrue;
    }

    thys = *pthys;

    /* same file ??? */
    if(samefile)
    {
	ajFileBuffClear(thys, -1);
	return ajTrue;
    }

    /* No: this is a copy of the true pointer. */
    /* ajFileClose(&thys->File); */

    thys->File = file;

    fileBuffInit(thys);

    return ajTrue;
}




/* @funcstatic fileBuffInit ***************************************************
**
** Initialized the data for a buffered file.
**
** @param [u] thys [AjPFileBuff] Buffered file object.
** @return [void]
******************************************************************************/

static void fileBuffInit(AjPFileBuff thys)
{
    thys->Last = thys->Curr = thys->Prev = thys->Lines = NULL;
    thys->Freelines = thys->Freelast = NULL;
    thys->Pos = thys->Size = 0;

    return;
}




/* @func ajFileBuffNewS *******************************************************
**
** Creates a new buffered input file object with no file but with
** one line of buffered data provided.
**
** @param [r] data [const AjPStr] One line of buffered data.
** @return [AjPFileBuff] New buffered file object.
** @category new [AjPFileBuff] Constructor using a line of buffered data
** @@
******************************************************************************/

AjPFileBuff ajFileBuffNewS(const AjPStr data)
{
    AjPFileBuff thys;
    AjPFile file;

    AJNEW0(file);
    file->End = ajTrue;
    ajDebug("EOF ajFileBuffNewS file <none>\n");

    thys = ajFileBuffNewFile(file);

    thys->Lines = AJNEW0(thys->Last);
    ajStrAssignS(&thys->Last->Line,data);

    thys->Curr = thys->Lines;
    thys->Pos  = 0;
    thys->Size = 1;

    return thys;
}




/* @func ajFileBuffNewF *******************************************************
**
** Creates a new buffered input file from an already open C file.
**
** @param [u] fp [FILE*] Open C file.
** @return [AjPFileBuff] New buffered file object.
** @category new [AjPFileBuff] Constructor using an existing open file,
**                             for example stdin or stdout.
** @@
******************************************************************************/

AjPFileBuff ajFileBuffNewF(FILE* fp)
{
    AjPFile file;

    file = ajFileNewF(fp);
    if(!file)
	return NULL;

    return ajFileBuffNewFile(file);
}




/* @func ajFileBuffNewDW ******************************************************
**
** Opens directory "dir"
** Looks for file(s) matching "file"
** Opens them as a list of files using a buffered file object.
**
** @param [r] dir [const AjPStr] Directory
** @param [r] wildfile [const AjPStr] Wildcard filename.
** @return [AjPFileBuff] New buffered file object.
** @category new [AjPFileBuff] Constructor using a directory and
**                             wildcard filename
** @@
******************************************************************************/

AjPFileBuff ajFileBuffNewDW(const AjPStr dir, const AjPStr wildfile)
{
    DIR* dp;
#if defined(AJ_IRIXLF)
    struct dirent64 *de;
#else
    struct dirent* de;
#endif
    ajint dirsize;
    AjPList list = NULL;
    AjPStr name  = NULL;
#ifdef _POSIX_C_SOURCE
    char buf[sizeof(struct dirent)+MAXNAMLEN];
#endif
    
    if(ajStrGetLen(dir))
	ajStrAssignS(&fileDirfixTmp, dir);
    else
	ajStrAssignC(&fileDirfixTmp, CURRENT_DIR);
    
    if(ajStrGetCharLast(dir) != SLASH_CHAR)
	ajStrAppendC(&fileDirfixTmp, SLASH_STRING);
    
    dp = fileOpenDir(&fileDirfixTmp);
    if(!dp)
	return NULL;
    
    dirsize = 0;
    list = ajListstrNew();

#if defined(AJ_IRIXLF)
#ifdef _POSIX_C_SOURCE
    while(!readdir64_r(dp,(struct dirent64 *)buf,&de))
    {
	if(!de)
	    break;
#else
	while((de=readdir64(dp)))
	{
#endif
#else
#ifdef _POSIX_C_SOURCE
    while(!readdir_r(dp,(struct dirent *)buf,&de))
    {
	if(!de)
	    break;
#else
	while((de=readdir(dp)))
	{
#endif
#endif
	/* skip deleted files with inode zero */
#ifndef __CYGWIN__
	if(!de->d_ino)
	    continue;
#endif
	if(ajCharMatchC(de->d_name, "."))
	    continue;
	if(ajCharMatchC(de->d_name, ".."))
	    continue;
	if(!ajCharMatchWildS(de->d_name, wildfile))
	    continue;
	dirsize++;
	ajDebug("accept '%s'\n", de->d_name);
	name = NULL;
	ajFmtPrintS(&name, "%S%s", fileDirfixTmp, de->d_name);
	ajListstrPushAppend(list, name);
    }
    
    closedir(dp);
    ajDebug("%d files for '%S' '%S'\n", dirsize, dir, wildfile);
    
    return ajFileBuffNewInList(list);
}




/* @func ajFileBuffNewDWE *****************************************************
**
** Opens directory "dir"
** Looks for file(s) matching "file"
** Skip files natching excluded files wildcard
** Opens them as a list of files using a buffered file object.
**
** @param [r] dir [const AjPStr] Directory
** @param [r] wildfile [const AjPStr] Wildcard filename.
** @param [r] exclude [const AjPStr] Wildcard excluded filename.
** @return [AjPFileBuff] New buffered file object.
** @category new [AjPFileBuff] Constructor using a directory and
**                             wildcard filename
** @@
******************************************************************************/

AjPFileBuff ajFileBuffNewDWE(const AjPStr dir, const AjPStr wildfile,
			     const AjPStr exclude)
{
    DIR* dp;
#if defined(AJ_IRIXLF)
    struct dirent64 *de;
#else
    struct dirent* de;
#endif
    ajint dirsize;
    AjPList list = NULL;
    AjPStr name  = NULL;
#ifdef _POSIX_C_SOURCE
    char buf[sizeof(struct dirent)+MAXNAMLEN];
#endif
    
    if(ajStrGetLen(dir))
	ajStrAssignS(&fileDirfixTmp, dir);
    else
	ajStrAssignC(&fileDirfixTmp, CURRENT_DIR);
    
    if(ajStrGetCharLast(dir) != SLASH_CHAR)
	ajStrAppendC(&fileDirfixTmp, SLASH_STRING);
    
    dp = fileOpenDir(&fileDirfixTmp);
    if(!dp)
	return NULL;
    
    dirsize = 0;
    list = ajListstrNew();

#if defined(AJ_IRIXLF)
#ifdef _POSIX_C_SOURCE
    while(!readdir64_r(dp,(struct dirent64 *)buf,&de))
    {
	if(!de)
	    break;
#else
	while((de=readdir64(dp)))
	{
#endif
#else
#ifdef _POSIX_C_SOURCE
    while(!readdir_r(dp,(struct dirent *)buf,&de))
    {
	if(!de)
	    break;
#else
	while((de=readdir(dp)))
	{
#endif
#endif
	/* skip deleted files with inode zero */
#ifndef __CYGWIN__
	if(!de->d_ino)
	    continue;
#endif
	if(ajCharMatchC(de->d_name, "."))
	    continue;
	if(ajCharMatchC(de->d_name, ".."))
	    continue;
	ajStrAssignC(&fileNameTmp, de->d_name);
	ajDebug("testing '%s'\n", de->d_name);
	if(!ajFileTestSkip(fileNameTmp, exclude, wildfile, ajFalse, ajFalse))
	    continue;
	dirsize++;
	ajDebug("accept '%s'\n", de->d_name);
	name = NULL;
	ajFmtPrintS(&name, "%S%s", fileDirfixTmp, de->d_name);
	ajListstrPushAppend(list, name);
    }
    
    closedir(dp);
    ajDebug("%d files for '%S' '%S'\n", dirsize, dir, wildfile);
    ajStrDelStatic(&fileNameTmp);
    
    return ajFileBuffNewInList(list);
}




/* @func ajFileBuffNewDC ******************************************************
**
** Opens directory "dir"
** Looks for file "file"
** Opens the file.
**
** @param [r] dir [const AjPStr] Directory. If empty uses current directory.
** @param [r] filename [const char*] Filename.
** @return [AjPFileBuff] New buffered file object.
** @@
******************************************************************************/

AjPFileBuff ajFileBuffNewDC(const AjPStr dir, const char* filename)
{
    if(ajStrGetLen(dir))
	ajStrAssignS(&fileNameFix, dir);
    else
	ajStrAssignC(&fileNameFix, CURRENT_DIR);

    if(ajStrGetCharLast(fileNameFix) != SLASH_CHAR)
	ajStrAppendC(&fileNameFix, SLASH_STRING);

    ajStrAppendC(&fileNameFix, filename);

    return ajFileBuffNewIn(fileNameFix);
}




/* @func ajFileBuffNewDF ******************************************************
**
** Opens directory "dir"
** Looks for file "file"
** Opens the file.
**
** @param [r] dir [const AjPStr] Directory. If empty uses current directory.
** @param [r] filename [const AjPStr] Filename.
** @return [AjPFileBuff] New buffered file object.
** @@
******************************************************************************/

AjPFileBuff ajFileBuffNewDF(const AjPStr dir, const AjPStr filename)
{
    if(ajStrGetLen(dir))
	ajStrAssignS(&fileNameFix, dir);
    else
	ajStrAssignC(&fileNameFix, CURRENT_DIR);

    if(ajStrGetCharLast(fileNameFix) != SLASH_CHAR)
	ajStrAppendC(&fileNameFix, SLASH_STRING);

    ajStrAppendS(&fileNameFix, filename);

    return ajFileBuffNewIn(fileNameFix);
}




/* @func ajFileNewDW **********************************************************
**
** Opens directory "dir"
** Looks for file(s) matching "file"
** Opens them as a list of files using a simple file object.
**
** @param [r] dir [const AjPStr] Directory
** @param [r] wildfile [const AjPStr] Wildcard filename.
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajFileNewDW(const AjPStr dir, const AjPStr wildfile)
{
    DIR* dp;
#if defined(AJ_IRIXLF)
    struct dirent64 *de;
#else
    struct dirent* de;
#endif
    ajint dirsize;
    AjPList list = NULL;
    AjPStr name  = NULL;
#ifdef _POSIX_C_SOURCE
    char buf[sizeof(struct dirent)+MAXNAMLEN];
#endif
    
    if(ajStrGetLen(dir))
	ajStrAssignS(&fileDirfixTmp, dir);
    else
	ajStrAssignC(&fileDirfixTmp, CURRENT_DIR);
    
    if(ajStrGetCharLast(dir) != SLASH_CHAR)
	ajStrAppendC(&fileDirfixTmp, SLASH_STRING);
    
    dp = fileOpenDir(&fileDirfixTmp);
    if(!dp)
	return NULL;
    
    dirsize = 0;
    list = ajListstrNew();
    
#if defined(AJ_IRIXLF)
#ifdef _POSIX_C_SOURCE
    while(!readdir64_r(dp,(struct dirent64 *)buf,&de))
    {
	if(!de)
	    break;
#else
	while((de=readdir64(dp)))
	{
#endif
#else
#ifdef _POSIX_C_SOURCE
    while(!readdir_r(dp,(struct dirent *)buf,&de))
    {
	if(!de)
	    break;
#else
	while((de=readdir(dp)))
	{
#endif
#endif
	/* skip deleted files with inode zero */
#ifndef __CYGWIN__
	if(!de->d_ino)
	    continue;
#endif
	if(ajCharMatchC(de->d_name, "."))
	    continue;
	if(ajCharMatchC(de->d_name, ".."))
	    continue;
	if(!ajCharMatchWildS(de->d_name, wildfile))
	    continue;
	dirsize++;
	ajDebug("accept '%s'\n", de->d_name);
	name = NULL;
	ajFmtPrintS(&name, "%S%s", fileDirfixTmp, de->d_name);
	ajListstrPushAppend(list, name);
    }
    
    closedir(dp);
    ajDebug("%d files for '%S' '%S'\n", dirsize, dir, wildfile);
    
    return ajFileNewInList(list);
}




/* @func ajFileNewDWE *********************************************************
**
** Opens directory "dir"
** Looks for file(s) matching "file"
** Skip files natching excluded files wildcard
** Opens them as a list of files using a simple file object.
**
** @param [r] dir [const AjPStr] Directory
** @param [r] wildfile [const AjPStr] Wildcard filename.
** @param [r] exclude [const AjPStr] Wildcard excluded filename.
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajFileNewDWE(const AjPStr dir, const AjPStr wildfile,
			     const AjPStr exclude)
{
    DIR* dp;
#if defined(AJ_IRIXLF)
    struct dirent64 *de;
#else
    struct dirent* de;
#endif
    ajint dirsize;
    AjPList list = NULL;
    AjPStr name  = NULL;
#ifdef _POSIX_C_SOURCE
    char buf[sizeof(struct dirent)+MAXNAMLEN];
#endif
    
    if(ajStrGetLen(dir))
	ajStrAssignS(&fileDirfixTmp, dir);
    else
	ajStrAssignC(&fileDirfixTmp, CURRENT_DIR);
    
    if(ajStrGetCharLast(dir) != SLASH_CHAR)
	ajStrAppendC(&fileDirfixTmp, SLASH_STRING);
    
    dp = fileOpenDir(&fileDirfixTmp);
    if(!dp)
	return NULL;
    
    dirsize = 0;
    list = ajListstrNew();
    
#if defined(AJ_IRIXLF)
#ifdef _POSIX_C_SOURCE
    while(!readdir64_r(dp,(struct dirent64 *)buf,&de))
    {
	if(!de)
	    break;
#else
	while((de=readdir64(dp)))
	{
#endif
#else
#ifdef _POSIX_C_SOURCE
    while(!readdir_r(dp,(struct dirent *)buf,&de))
    {
	if(!de)
	    break;
#else
	while((de=readdir(dp)))
	{
#endif
#endif
	/* skip deleted files with inode zero */
#ifndef __CYGWIN__
	if(!de->d_ino)
	    continue;
#endif
	if(ajCharMatchC(de->d_name, "."))
	    continue;
	if(ajCharMatchC(de->d_name, ".."))
	    continue;
	ajStrAssignC(&fileNameTmp, de->d_name);
	if(!ajFileTestSkip(fileNameTmp, exclude, wildfile, ajFalse, ajFalse))
	    continue;
	dirsize++;
	ajDebug("accept '%s'\n", de->d_name);
	name = NULL;
	ajFmtPrintS(&name, "%S%s", fileDirfixTmp, de->d_name);
	ajListstrPushAppend(list, name);
    }
    
    closedir(dp);
    ajDebug("%d files for '%S' '%S'\n", dirsize, dir, wildfile);
    
    return ajFileNewInList(list);
}




/* @func ajFileNewDF **********************************************************
**
** Opens directory "dir"
** Looks for file "file"
**
** @param [r] dir [const AjPStr] Directory
** @param [r] filename [const AjPStr] Wildcard Filename.
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajFileNewDF(const AjPStr dir, const AjPStr filename)
{
    if(ajStrGetLen(dir))
	ajStrAssignS(&fileNameFix, dir);
    else
	ajStrAssignC(&fileNameFix, CURRENT_DIR);
    
    if(ajStrGetCharLast(fileNameFix) != SLASH_CHAR)
	ajStrAppendC(&fileNameFix, SLASH_STRING);
    
    ajStrAppendS(&fileNameFix, filename);
     
    return ajFileNewIn(fileNameFix);
}



/* @func ajFileNewDirF ********************************************************
**
** Opens directory "dir"
** Looks for file "file" with the extension (if any) specified
** for the directory
**
** @param [r] dir [const AjPDir] Directory
** @param [r] filename [const AjPStr] Wildcard Filename.
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajFileNewDirF(const AjPDir dir, const AjPStr filename)
{
    if(ajStrGetLen(dir->Name))
	ajStrAssignS(&fileNameFix, dir->Name);
    else
	ajStrAssignC(&fileNameFix, CURRENT_DIR);
    
    if(ajStrGetCharLast(fileNameFix) != SLASH_CHAR)
	ajStrAppendC(&fileNameFix, SLASH_STRING);
    
    ajStrAppendS(&fileNameFix, filename);
    ajFileNameExt(&fileNameFix, dir->Extension);

    return ajFileNewIn(fileNameFix);
}





/* @func ajFileNewDC **********************************************************
**
** Opens directory "dir"
** Looks for file "file"
**
** @param [r] dir [const AjPStr] Directory
** @param [r] filename [const char*] Filename.
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajFileNewDC(const AjPStr dir, const char* filename)
{
    if(ajStrGetLen(dir))
	ajStrAssignS(&fileNameFix, dir);
    else
	ajStrAssignC(&fileNameFix, CURRENT_DIR);

    if(ajStrGetCharLast(fileNameFix) != SLASH_CHAR)
	ajStrAppendC(&fileNameFix, SLASH_STRING);

    ajStrAppendC(&fileNameFix, filename);

    return ajFileNewIn(fileNameFix);
}




/* @func ajFileBuffNewInList **************************************************
**
** Creates a new buffered file object from a list of filenames.
**
** @param [u] list [AjPList] List of filenames as strings.
** @return [AjPFileBuff] New buffered file object.
** @category new [AjPFileBuff] Constructor using a list of filenames
** @@
******************************************************************************/

AjPFileBuff ajFileBuffNewInList(AjPList list)
{
    AjPFile file;

    file = ajFileNewInList(list);
    if(!file)
	return NULL;

    return ajFileBuffNewFile(file);
}




/* ==================================================================== */
/* =========================== destructor ============================= */
/* ==================================================================== */

/* @section Buffered File Destructors *****************************************
**
** Destruction is achieved by closing the file.
**
** Unlike ANSI C, there are tests to ensure a file is not closed twice.
**
******************************************************************************/




/* @func ajFileBuffDel ********************************************************
**
** Destructor for a buffered file object.
**
** @param [d] pthis [AjPFileBuff*] Buffered file object.
** @return [void]
** @category delete [AjPFileBuff] Default destructor
** @@
******************************************************************************/

void ajFileBuffDel(AjPFileBuff* pthis)
{
    AjPFileBuff thys;

    if(!pthis)
	return;
    
    thys = *pthis;
    
    if(!thys)
	return;

    ajFileBuffClear(thys, -1);
    ajFileBuffFreeClear(thys);
    ajFileClose(&thys->File);
    AJFREE(*pthis);
    
    return;
}




/* ==================================================================== */
/* ========================== Assignments ============================= */
/* ==================================================================== */

/* @section Buffered File Assignments *****************************************
**
** These functions overwrite the file provided as the first argument
**
******************************************************************************/


/* ==================================================================== */
/* =========================== Modifiers ============================== */
/* ==================================================================== */

/* @section BufferedFile Modifiers ********************************************
**
** These functions use the contents of a file object and update them.
**
******************************************************************************/




/* @func ajFileBuffGet ********************************************************
**
** Reads a line from a buffered file. If the buffer has data, reads from the
** buffer. If the buffer is exhausted, reads from the file. If the file is
** exhausted, sets end of file and returns. If end of file was already set,
** looks for another file to open.
**
** @param [u] thys [AjPFileBuff] Buffered input file.
** @param [w] pdest [AjPStr*] Buffer to hold results.
** @return [AjBool] ajTrue if data was read.
** @category modify [AjPFileBuff] Reads a line from a buffered file.
** @@
******************************************************************************/

AjBool ajFileBuffGet(AjPFileBuff thys, AjPStr* pdest)
{
    ajlong fpos = 0;

    return ajFileBuffGetL(thys, pdest, &fpos);
}




/* @func ajFileBuffGetTrim ****************************************************
**
** Reads a line from a buffered file. If the buffer has data, reads from the
** buffer. If the buffer is exhausted, reads from the file. If the file is
** exhausted, sets end of file and returns. If end of file was already set,
** looks for another file to open.
**
** @param [u] thys [AjPFileBuff] Buffered input file.
** @param [w] pdest [AjPStr*] Buffer to hold results.
** @return [AjBool] ajTrue if data was read.
** @category modify [AjPFileBuff] Reads a line from a buffered file.
** @@
******************************************************************************/

AjBool ajFileBuffGetTrim(AjPFileBuff thys, AjPStr* pdest)
{
    AjBool ret;
    ajlong fpos = 0;
    AjPStr dest;

    ret = ajFileBuffGetL(thys, pdest, &fpos);
    
    /* trim any trailing newline */

    dest = *pdest;

    /*ajDebug("Remove carriage-return characters from PC-style files\n");*/
    if(ajStrGetCharLast(dest) == '\n')
	ajStrCutEnd(pdest, 1);

    /* PC files have \r\n Macintosh files have just \r : this fixes both */
    if(ajStrGetCharLast(dest) == '\r')
	ajStrCutEnd(pdest, 1);

    return ret;
}




/* @func ajFileBuffGetStore ***************************************************
**
** Reads a line from a buffered file. Also appends the line to
** a given string if the append flag is true. A double NULL character
** is added afterwards. If the buffer has data, reads from the
** buffer. If the buffer is exhausted, reads from the file. If the file is
** exhausted, sets end of file and returns. If end of file was already set,
** looks for another file to open.
**
** @param [u] thys [AjPFileBuff] Buffered input file.
** @param [w] pdest [AjPStr*] Buffer to hold results.
** @param [r] store [AjBool] append if true
** @param [w] astr [AjPStr*] string to append to
** @return [AjBool] ajTrue if data was read.
** @category modify [AjPFileBuff] Reads a line from a buffered file
**                                with append.
** @@
******************************************************************************/

AjBool ajFileBuffGetStore(AjPFileBuff thys, AjPStr* pdest,
			  AjBool store, AjPStr *astr)
{
    ajlong fpos = 0;
    AjBool ret;

    ret =  ajFileBuffGetL(thys, pdest, &fpos);

    if(store && ret)
    {
	ajDebug("ajFileBuffGetStore: '%S'", *pdest);
	ajStrAppendS(astr,*pdest);
    }

    return ret;
}




/* @func ajFileBuffGetStoreL **************************************************
**
** Reads a line from a buffered file. Also appends the line to
** a given string if the append flag is true. A double NULL character
** is added afterwards. If the buffer has data, reads from the
** buffer. If the buffer is exhausted, reads from the file. If the file is
** exhausted, sets end of file and returns. If end of file was already set,
** looks for another file to open.
**
** @param [u] thys [AjPFileBuff] Buffered input file.
** @param [w] pdest [AjPStr*] Buffer to hold results.
** @param [w] fpos [ajlong*] File position before the read.
** @param [r] store [AjBool] append if true
** @param [w] astr [AjPStr*] string to append to
** @return [AjBool] ajTrue if data was read.
** @category modify [AjPFileBuff] Reads a line from a buffered file
**                                with append.
** @@
******************************************************************************/

AjBool ajFileBuffGetStoreL(AjPFileBuff thys, AjPStr* pdest,
			   ajlong* fpos,
			   AjBool store, AjPStr *astr)
{
    AjBool ret;

    ret =  ajFileBuffGetL(thys, pdest, fpos);

    if(store && ret)
    {
	ajDebug("ajFileBuffGetStoreL:\n%S", *pdest);
	ajStrAppendS(astr,*pdest);
    }

    return ret;
}




/* @func ajFileBuffGetL *******************************************************
**
** Reads a line from a buffered file. If the buffer has data, reads from the
** buffer. If the buffer is exhausted, reads from the file. If the file is
** exhausted, sets end of file and returns. If end of file was already set,
** looks for another file to open.
**
** @param [u] thys [AjPFileBuff] Buffered input file.
** @param [w] pdest [AjPStr*] Buffer to hold results.
** @param [w] fpos [ajlong*] File position before the read.
** @return [AjBool] ajTrue if data was read.
** @@
******************************************************************************/

AjBool ajFileBuffGetL(AjPFileBuff thys, AjPStr* pdest, ajlong* fpos)
{
    AjBool ok;
    
    /* read from the buffer if it is not empty */
    
    *fpos = 0;
    
    if(thys->Pos < thys->Size)
    {
	ajStrAssignS(pdest, thys->Curr->Line);
	*fpos = thys->Curr->Fpos;
	thys->Prev = thys->Curr;
	thys->Curr = thys->Curr->Next;
	thys->Pos++;

	return ajTrue;
    }
    
    /* file has been closed */
    if(!thys->File->Handle)
	return ajFalse;
    
    /* buffer used up - try reading from the file */
    
    ok = ajFileGetsL(thys->File, pdest, &thys->Fpos);
    
    if(!ok)
    {
	if(thys->File->End)
	{
	    if(thys->Size)
	    {
		/* we have data in the buffer - fail */
		ajDebug("End of file - data in buffer - return ajFalse\n");
		return ajFalse;
	    }
	    else
	    {
		/* buffer clear - try another file */
		if(ajFileNext(thys->File))
		{
		    /* OK - read the new file */
		    ok = ajFileBuffGetL(thys, pdest, fpos);
		    ajDebug("End of file - trying next file ok: %B "
			    "fpos: %Ld %Ld\n",
			    ok, *fpos, thys->Fpos);
		    return ok;
		}
		else
		{
		    /* no new file, fail again */
		    ajDebug("End of file - no new file to read - "
			    "return ajFalse\n");
		    return ajFalse;
		}
	    }
	}
	else
	    ajFatal("Error reading from file '%s'\n",
		    ajFileName(thys->File));
    }
    
    if(thys->Nobuff)
    {
	*fpos = thys->Fpos;
	/*ajDebug("ajFileBuffGetL unbuffered fpos: %Ld\n", *fpos);*/
	return ajTrue;
    }
    
    fileBuffLineAdd(thys, *pdest);
    *fpos = thys->Fpos;
    
    return ajTrue;
}




/* @func ajFileBuffStripHtml **************************************************
**
** Processes data in the file buffer, removing HTML tokens between
** angle brackets, plus any TITLE. This seems to be enough to make HTML
** output readable.
**
** @param [u] thys [AjPFileBuff] Buffered file with data loaded
**                                     in the buffer.
** @return [void]
** @category modify [AjPFileBuff] Processes data in the file buffer,
**                                removing HTML tokens
** @@
******************************************************************************/

void ajFileBuffStripHtml(AjPFileBuff thys)
{
    AjPRegexp tagexp   = NULL;
    AjPRegexp fullexp  = NULL;
    AjPRegexp httpexp  = NULL;
    AjPRegexp nullexp  = NULL;
    AjPRegexp chunkexp = NULL;
    AjPRegexp hexexp   = NULL;
    AjPRegexp ncbiexp  = NULL;
    AjPRegexp ncbiexp2 = NULL;
    AjPRegexp srsdbexp = NULL;
    
    AjPStr s1 = NULL;
    AjPStr s2 = NULL;
    AjPStr s3 = NULL;
    ajint i;
    AjBool doChunk = ajFalse;
    ajint ichunk;
    ajint chunkSize;
    ajint iline;
    AjPStr saveLine = NULL;
    AjPStr hexstr   = NULL;
    
    tagexp   = ajRegCompC("^(.*)(<[!/A-Za-z][^>]*>)(.*)$");
    fullexp  = ajRegCompC("^(.*)(<([A-Za-z]+)[^>]*>.*</\\3>)(.*)$");
    httpexp  = ajRegCompC("^HTTP/");
    nullexp  = ajRegCompC("^\r?\n?$");
    chunkexp = ajRegCompC("^Transfer-Encoding: +chunked");
    hexexp   = ajRegCompC("^([0-9a-fA-F]+) *\r?\n?$");
    ncbiexp  = ajRegCompC("^Entrez Reports\r?\n$");
    ncbiexp2 = ajRegCompC("^----------------\r?\n$");
    srsdbexp = ajRegCompC("^([A-Za-z0-9_-]+)(:)([A-Za-z0-9_-]+)");
    
    /* first take out the HTTP header (HTTP 1.0 onwards) */
    if(!thys->Size)
	return;
    
    ajFileBuffPrint(thys, "Before ajFileBuffStripHtml");

    i = 0;
    
    ajDebug("First line [%d] '%S' \n",
	     ajStrGetUse(thys->Curr->Line), thys->Curr->Line);
    
    if(ajRegExec(httpexp, thys->Curr->Line))
    {
	/* ^HTTP  header processing */
	while(thys->Pos < thys->Size &&
	      !ajRegExec(nullexp, thys->Curr->Line))
	{
	    /* to empty line */
	    if(ajRegExec(chunkexp, thys->Curr->Line))
	    {
		ajDebug("Chunk encoding: %S", thys->Curr->Line);
		/* chunked - see later */
		doChunk = ajTrue;
	    }
	    fileBuffLineDel(thys);
	}

	/* blank line after header */
	fileBuffLineDel(thys);
    }

    if(doChunk)
    {
	/*ajFileBuffTraceFull(thys, 999999, 0);*/
	
	if(!ajRegExec(hexexp, thys->Curr->Line))
	{
	    ajFatal("Bad chunk data from HTTP, expect chunk size got '%S'",
		    thys->Curr->Line);
	}
	ajRegSubI(hexexp, 1, &hexstr);
	ajStrToHex(hexstr, &chunkSize);
	
	ajDebug("chunkSize hex:%x %d\n", chunkSize, chunkSize);
	fileBuffLineDel(thys);	/* chunk size */
	
	ichunk = 0;
	iline = 0;
	while(chunkSize && thys->Curr)
	{
	    iline++;
	    /* get the chunk size - zero is the end */
	    /* process the chunk */
	    ichunk += ajStrGetLen(thys->Curr->Line);
	    
	    ajDebug("++input line [%d] ichunk=%d:%d %d:%S",
		    iline, ichunk, chunkSize,
		    ajStrGetLen(thys->Curr->Line), thys->Curr->Line);
	    if(ichunk >= chunkSize)	/* end of chunk */
	    {
		if(ichunk == chunkSize)
		{
		    /* end-of-chunk at end-of-line */
		    fileBuffLineNext(thys);
		    ajStrAssignC(&saveLine, "");
		    ajDebug("end-of-chunk at end-of-line: '%S'\n", saveLine);
		}
		else
		{
		    /* end-of-chunk in mid-line, patch up the input */
		    ajDebug("end-of-chunk in mid-line, %d:%d have input: %d '%S'\n",
			    ichunk, chunkSize,
			    ajStrGetLen(thys->Curr->Line),
			    thys->Curr->Line);
		    ajStrAssignSubS(&saveLine, thys->Curr->Line, 0,
				-(ichunk-chunkSize+1));
		    ajStrKeepRange(&thys->Curr->Line, -(ichunk-chunkSize), -1);
		}
		
		/* skip a blank line */
		
		if(!ajRegExec(nullexp, thys->Curr->Line))
		{
		    ajFileBuffPrint(thys, "Blank line not found");
		    ajFatal("Bad chunk data from HTTP, expect blank line"
			    " got '%S'", thys->Curr->Line);
		}
		fileBuffLineDel(thys);
		
		/** read the next chunk size */
		
		if(!ajRegExec(hexexp, thys->Curr->Line))
		{
		    ajFileBuffPrint(thys, "Chunk size not found");
		    ajFatal("Bad chunk data from HTTP, expect chunk size "
			    "got '%S'",
			    thys->Curr->Line);
		}
		ajRegSubI(hexexp, 1, &hexstr);
		ajStrToHex(hexstr, &chunkSize);
		ichunk = 0;
		fileBuffLineDel(thys);
	    }

	    if(saveLine)
	    {
		if(ajStrGetLen(saveLine))
		{
		    ichunk = ajStrGetLen(thys->Curr->Line);
		    /* preserve the line split by chunksize */
		    ajStrInsertS(&thys->Curr->Line, 0, saveLine);
		    if(ichunk < chunkSize)
		    {
			/* process the next line */
			fileBuffLineNext(thys); /* after restored line */
		    }
		    else
		    {
			/* we alrady have the whole chunk! */
			ichunk -= ajStrGetLen(thys->Curr->Line);
		    }
		}
		else
		{
		    /* just a chunksize, skip */
		    if(thys->Curr && chunkSize)
		    {
			/*fileBuffLineDel(thys);*/
		    }
		    else if (chunkSize)/* final non-zero chunksize */
		    {
			fileBuffLineDel(thys);
		    }
		}
		ajStrDel(&saveLine);
	    }
	    else
	    {
		/* next line */
		fileBuffLineNext(thys);
	    }
	}

	ajFileBuffFix(thys);
	/*ajFileBuffTraceFull(thys, 999999, 0);*/
	ajStrDel(&hexstr);
	/*ajFileBuffPrint(thys, "Chunks resolved");*/
    }
    
    ajFileBuffReset(thys);

    /*
     ** Now we have a clean single file to process
     */
    
    ajFileBuffPrint(thys, "About to preprocess");
    ajFileBuffStripHtmlPre(thys);

    while(thys->Curr)
    {
	if(ajRegExec(ncbiexp, thys->Curr->Line))
	    ajStrAssignC(&thys->Curr->Line, "\n");
	if(ajRegExec(ncbiexp2, thys->Curr->Line))
	    ajStrAssignC(&thys->Curr->Line, "\n");

	while(ajRegExec(fullexp, thys->Curr->Line))
	{
	    ajRegSubI(fullexp, 1, &s1);
	    ajRegSubI(fullexp, 2, &s2);
	    ajRegSubI(fullexp, 4, &s3);
	    ajFmtPrintS(&thys->Curr->Line, "%S%S", s1, s3);
	}

	while(ajRegExec(tagexp, thys->Curr->Line))
	{
	    ajRegSubI(tagexp, 1, &s1);
	    ajRegSubI(tagexp, 2, &s2);
	    ajRegSubI(tagexp, 3, &s3);
	    ajFmtPrintS(&thys->Curr->Line, "%S%S", s1, s3);
	}

	if(ajRegExec(srsdbexp, thys->Curr->Line))
	{
	    ajRegSubI(srsdbexp,1,&s1);
	    ajRegSubI(srsdbexp,2,&s2);
	    ajRegSubI(srsdbexp,3,&s3);
	    fileBuffLineDel(thys);
	    ++i;
	    continue;
	}

	if(ajRegExec(nullexp, thys->Curr->Line))
	{
	    /* allow for newline */
	    fileBuffLineDel(thys);
	}
	else
	{
	    fileBuffLineNext(thys);
	}
	i++;
    }
    
    ajFileBuffReset(thys);
    
    ajStrDel(&s1);
    ajStrDel(&s2);
    ajStrDel(&s3);
    
    /* free the regular expressions - we expect to use them once only */
    
    ajRegFree(&tagexp);
    ajRegFree(&fullexp);
    ajRegFree(&httpexp);
    ajRegFree(&nullexp);
    ajRegFree(&chunkexp);
    ajRegFree(&hexexp);
    ajRegFree(&ncbiexp);
    ajRegFree(&ncbiexp2);
    ajRegFree(&srsdbexp);
    
    ajFileBuffPrint(thys, "After ajFileBuffStripHtml");

    return;
}




/* @func ajFileBuffLoadC ******************************************************
**
** Adds a line to the buffer.
**
** Intended for cases where the file data must be preprocessed before
** being seen by the sequence reading routines. The first case was
** for stripping HTML tags after reading via HTTP.
**
** @param [u] thys [AjPFileBuff] Buffered file.
** @param [r] line [const char*] Line of input.
** @return [void]
** @@
******************************************************************************/

void ajFileBuffLoadC(AjPFileBuff thys, const char* line)
{
    if(!thys->Lines)
	thys->Curr = thys->Lines = AJNEW0(thys->Last);
    else
	thys->Last = AJNEW0(thys->Last->Next);

    ajStrAssignC(&thys->Last->Line,line);
    thys->Last->Next = NULL;
    thys->Size++;

    return;
}




/* @func ajFileBuffLoadS ******************************************************
**
** Adds a copy of a line to the buffer.
**
** Intended for cases where the file data must be preprocessed before
** being seen by the sequence reading routines. The first case was
** for stripping HTML tags after reading via HTTP.
**
** @param [u] thys [AjPFileBuff] Buffered file.
** @param [r] line [const AjPStr] Line of input.
** @return [void]
** @@
******************************************************************************/

void ajFileBuffLoadS(AjPFileBuff thys, const AjPStr line)
{
    if(!thys->Lines)
	thys->Curr = thys->Lines = AJNEW0(thys->Last);
    else
	thys->Last = AJNEW0(thys->Last->Next);

    ajStrAssignS(&thys->Last->Line,line);
    thys->Last->Next = NULL;
    thys->Size++;

    return;
}




/* @func ajFileBuffEof ********************************************************
**
** Tests whether we have reached end of file already
**
** @param [r] thys [const AjPFileBuff] File buffer
** @return [AjBool] ajTrue if we already set end-of-file
** @@
******************************************************************************/

AjBool ajFileBuffEof(const AjPFileBuff thys)
{
    return thys->File->End;
}




/* @func ajFileBuffEnd ********************************************************
**
** Tests whether the file is exhausted. This means end of file is reached
** and the buffer is empty
**
** @param [r] thys [const AjPFileBuff] File buffer
** @return [AjBool] ajTrue if we already set end-of-file
** @@
******************************************************************************/

AjBool ajFileBuffEnd(const AjPFileBuff thys)
{
    ajDebug("ajFileBuffEnd End: %B Size: %d\n", thys->File->End, thys->Size);

    /* not reached EOF yet */
    if(!thys->File->End)
	return ajFalse;

    /* Something in the buffer*/
    if(thys->Size != 0)
	return ajFalse;

    return ajTrue;
}




/* @func ajFileBuffReset ******************************************************
**
** Resets the pointer and current record of a file buffer so the next read
** starts at the first buffered line.
**
** @param [u] thys [AjPFileBuff] File buffer
** @return [void]
** @category modify [AjPFileBuff] Resets so the next read uses the first
**                                buffered line
** @@
******************************************************************************/

void ajFileBuffReset(AjPFileBuff thys)
{
    thys->Pos  = 0;
    thys->Curr = thys->Lines;
    thys->Prev = NULL;

    return;
}




/* @func ajFileBuffResetStore *************************************************
**
** Resets the pointer and current record of a file buffer so the next read
** starts at the first buffered line.
**
** @param [u] thys [AjPFileBuff] File buffer
** @param [r] store [AjBool] append if true
** @param [w] astr [AjPStr*] string to append to
** @return [void]
** @@
******************************************************************************/

void ajFileBuffResetStore(AjPFileBuff thys, AjBool store, AjPStr *astr)
{
    ajFileBuffReset(thys);
    if(store)
	ajStrAssignC(astr, "");

    return;
}




/* @func ajFileBuffResetPos ***************************************************
**
** Resets the pointer and current record of a file buffer so the next read
** starts at the first buffered line.
**
** Also resets the file position to the last known read, to undo the
** damage done by (for example) ajseqabi functions.
**
** @param [u] thys [AjPFileBuff] File buffer
** @return [void]
** @@
******************************************************************************/

void ajFileBuffResetPos(AjPFileBuff thys)
{
    ajFileBuffTraceFull(thys, 10, 10);

    thys->Pos  = 0;
    thys->Curr = thys->Lines;

    if(!thys->File->End && (thys->File->fp != stdin))
	ajFileSeek(thys->File, thys->Fpos, SEEK_SET);

    ajFileBuffTraceFull(thys,10,10);

    return;
}




/* @func ajFileBuffFix ********************************************************
**
** Resets the pointer and current record of a file buffer so the next
** read starts at the first buffered line. Fixes buffer size after the
** buffer has been edited.
**
** @param [u] thys [AjPFileBuff] File buffer
** @return [void]
** @@
******************************************************************************/

void ajFileBuffFix(AjPFileBuff thys)
{
    AjPFileBuffList list;
    ajint i = 1;

    ajFileBuffReset(thys);
    thys->Pos  = 0;
    thys->Curr = thys->Lines;

    list = thys->Lines;
    if(!list)
    {
	thys->Size = 0;
	return;
    }

    while(list->Next)
    {
	i++;
	list = list->Next;
    }

    if(i != thys->Size)
	ajDebug("ajFileBuffFix size was %d now %d\n", thys->Size, i);

    thys->Size=i;
    return;
}




/* @func ajFileBuffFreeClear **************************************************
**
** Deletes freed lines from a file buffer. The free list is used to avoid
** reallocating space for new records and must be deleted as part of
** the destructor.
**
** @param [u] thys [AjPFileBuff] File buffer
** @return [void]
** @@
******************************************************************************/

void ajFileBuffFreeClear(AjPFileBuff thys)
{
    AjPFileBuffList list;

    if(!thys)
	return;

    /*ajDebug("ajFileBuffFreeClear %x\n", thys->Freelines);*/

    while(thys->Freelines)
    {
	list = thys->Freelines;
	thys->Freelines = thys->Freelines->Next;
	ajStrDel(&list->Line);
	AJFREE(list);
    }

    return;
}




/* @func ajFileBuffClear ******************************************************
**
** Deletes processed lines from a file buffer. The buffer has a record
** (Pos) of the next unprocessed line in the buffer.
**
** Unbuffered files need special handling. The buffer can be turned off
** while it still contains data. If so, we have to carefully run it down.
** If this runs it to zero, we may want to save the last line read.
**
** @param [u] thys [AjPFileBuff] File buffer
** @param [r] lines [ajint] Number of lines to retain. -1 deletes everything.
** @return [void]
** @category modify [AjPFileBuff] Deletes unwanted old lines from the buffer
**                              but can keep the most recent line(s) for reuse.
** @@
******************************************************************************/

void ajFileBuffClear(AjPFileBuff thys, ajint lines)
{
    ajint i = 0;
    AjPFileBuffList list;
    AjPFileBuffList next;
    ajint first;
    ajint ifree = 0;
    
    ajDebug("ajFileBuffClear (%d) Nobuff: %B\n", lines, thys->Nobuff);
    /*FileBuffTraceFull(thys, thys->Size, 100);*/
    if(!thys)
	return;
    
    if(!thys->File)
	return;
    
    if(lines < 0)
	first = thys->Size;
    else
	first = thys->Pos - lines;
    
    if(first < 0)
	first = 0;
    
    /* nobuff, and all read */
    if(thys->Nobuff && thys->Pos == thys->Size)
	/* delete any old saved line */
	first = thys->Pos;
    
    list = thys->Lines;
    for(i=0; i < first; i++)
    {
	/* we save one line at a time */
	next = list->Next;
	/* so keep a note of the next one for later */
	/*ajDebug("Try to reuse %x size: %d use: %d\n",
	  list->Line, ajStrGetRes(list->Line), ajStrGetUse(list->Line));*/
	
	if(thys->Nobuff)
	{
	    ajStrDel(&list->Line);
	    AJFREE(list);	     /* deleted, kill the list item */
	}
	else
	{
	    if(ajStrDelStatic(&list->Line))
	    {
		/* move free line to the end */

		/*ajDebug("can save to free list %x %d bytes\n",
		  list->Line, ajStrGetRes(list->Line));*/

		ifree++;
		/* just save the one line */
		list->Next = NULL;
		if(!thys->Freelines)
		{
		    /* start a new free list */
		    thys->Freelines = list;
		    thys->Freelast = list;
		    /*ajDebug("start  list Free %x Freelast %x \n",
		      thys->Freelines, thys->Freelast);*/
		}
		else
		{
		    /* append to free list */
		    thys->Freelast->Next = list;
		    thys->Freelast = thys->Freelast->Next;
		    /*ajDebug("append list Free %x Freelast %x \n",
		      thys->Freelines, thys->Freelast);*/
		}
	    }
	    else
	    {
		ajDebug("ajStrDelReuse was false\n");
	    }
	}
	list = next;
    }
    
    thys->Pos   = 0;
    thys->Size -= i;
    if(!thys->Size)
    {
	ajDebug("size 0: Lines: %x Curr: %x  Prev: %x Last: %x Free: %x Freelast: %x\n",
		thys->Lines, thys->Curr, thys->Prev, thys->Last,
		thys->Freelines, thys->Freelast);
    }

    thys->Lines = thys->Curr = list;

    ajDebug("ajFileBuffClear '%F' (%d lines)\n"
	     "     %b size: %d pos: %d removed %d lines add to free: %d\n",
	     thys->File, lines, thys->Nobuff, thys->Size, thys->Pos, i, ifree);
    
    ajFileBuffTrace(thys);
    
    if(thys->Nobuff && !thys->Size && lines == 1)
    {
	/* unbuffered - can only save last line */
	if(thys->Lines)
	    ajFatal("Buffer error clearing unbuffered file "
		    "in ajFileBuffClear\n");

	thys->Lines = AJNEW0(thys->Last);

	ajStrAssignS(&thys->Last->Line, thys->File->Buff);
	thys->Curr = thys->Last;
	thys->Curr->Fpos = thys->Fpos;
	thys->Last->Next = NULL;
	thys->Pos = 0;
	thys->Size = 1;
    }
    
    return;
}




/* @func ajFileBuffClearStore *************************************************
**
** Deletes processed lines from a file buffer. The buffer has a record
** (Pos) of the next unprocessed line in the buffer.
**
** Unbuffered files need special handling. The buffer can be turned off
** while it still contains data. If so, we have to carefully run it down.
** If this runs it to zero, we may want to save the last line read.
**
** @param [u] thys [AjPFileBuff] File buffer
** @param [r] lines [ajint] Number of lines to retain. -1 deletes everything.
** @param [r] rdline [const AjPStr] Last line of input.
**                            Used to count characters to be saved
** @param [r] store [AjBool] append if true
** @param [w] astr [AjPStr*] string to append to
** @return [void]
** @@
******************************************************************************/

void ajFileBuffClearStore(AjPFileBuff thys, ajint lines,
			   const AjPStr rdline, AjBool store, AjPStr *astr)
{
    ajFileBuffClear(thys, lines);
    if(store && ajStrGetLen(rdline))
	ajStrCutEnd(astr, ajStrGetLen(rdline));

    return;
}




/* @func ajFileBuffNobuff *****************************************************
**
** Sets file to be unbuffered. If it already has buffered data, we have to
** first run down the buffer.
**
** @param [u] thys [AjPFileBuff] Buffered file object.
** @return [AjBool] ajTrue if the file was unbuffered before
** @category modify [AjPFileBuff] Turns off input buffering.
** @@
******************************************************************************/

AjBool ajFileBuffNobuff(AjPFileBuff thys)
{
    AjBool ret;

    if(!thys)
	return ajFalse;

    ret = thys->Nobuff;
    ajDebug("ajFileBuffNobuff %F buffsize: %d\n", thys->File, thys->Size);
    thys->Nobuff = ajTrue;

    return ret;
}


/* @func ajFileBuffBuff *****************************************************
**
** Sets file to be buffered. If it already has buffered data, we have to
** first run down the buffer.
**
** @param [u] thys [AjPFileBuff] Buffered file object.
** @return [AjBool] ajTrue if the file was unbuffered before
** @@
******************************************************************************/

AjBool ajFileBuffBuff(AjPFileBuff thys)
{
    AjBool ret;

    if(!thys)
	return ajFalse;

    ret = thys->Nobuff;
    ajDebug("ajFileBuffBuff %F buffsize: %d\n", thys->File, thys->Size);
    thys->Nobuff = ajFalse;

    return ret;
}


/* ==================================================================== */
/* ======================== Operators ==================================*/
/* ==================================================================== */

/* @section Buffered File Operators *******************************************
**
** These functions use the contents of a file object but do not make
** any changes.
**
******************************************************************************/




/* @func ajFileBuffEmpty ******************************************************
**
** Tests whether a file buffer is empty.
**
** @param [r] thys [const AjPFileBuff] Buffered file.
** @return [AjBool] ajTrue if the buffer is empty.
** @category use [AjPFileBuff] Tests whether a file buffer is empty.
** @@
******************************************************************************/

AjBool ajFileBuffEmpty(const AjPFileBuff thys)
{
    ajDebug("ajFileBuffEmpty Size: %d Pos: %d End: %b Handle: %d "
	     "Fp: %x List; %d\n",
	     thys->Size, thys->Pos, thys->File->End, thys->File->Handle,
	     thys->File->fp, ajListstrGetLength(thys->File->List));

    if(thys->Pos < thys->Size)
	return ajFalse;

    /* not open */
    if(!thys->File->fp || !thys->File->Handle)
	return ajTrue;

    if(thys->File->End && !ajListstrGetLength(thys->File->List))
        /* EOF and done */
	return ajTrue;

    ajDebug("ajFileBuffEmpty false\n");

    return ajFalse;
}




/* @func ajFileBuffTrace ******************************************************
**
** Writes debug messages to indicate the contents of a buffered file.
**
** @param [r] thys [const AjPFileBuff] Buffered file.
** @return [void]
** @category use [AjPFileBuff] Writes debug messages to indicate the
**                             contents of a buffered file.
** @@
******************************************************************************/

void ajFileBuffTrace(const AjPFileBuff thys)
{
    AjPFileBuffList test;
    ajint i = 0;
    ajint j = -1;

    ajDebug("Trace buffer file '%S'\n"
	     "             Pos: %d Size: %d FreeSize: %d Fpos: %Ld End: %b\n",
	    thys->File->Name, thys->Pos, thys->Size,thys->FreeSize,
	    thys->Fpos, thys->File->End);

    if(thys->Size)
    {
	ajDebug(" Lines:\n");
	if(thys->Curr)
	    ajDebug("  Curr: %8Ld %x %x <%S>\n",
		    thys->Curr->Fpos, 
		    thys->Curr->Line, thys->Curr->Next,
		    thys->Curr->Line);
	else
	    ajDebug("  Curr: <null>\n");
	if(thys->Lines)
	    ajDebug("  From: %8Ld %x %x <%S>\n",
		    thys->Lines->Fpos, 
		    thys->Lines->Line, thys->Lines->Next,
		    thys->Lines->Line);
	else
	    ajDebug("  From: <null>\n");
	if(thys->Last)
	    ajDebug("    To: %8Ld %x %x <%S>\n",
		    thys->Last->Fpos, 
		    thys->Last->Line, thys->Last->Next,
		    thys->Last->Line);
	else
	    ajDebug("    To: <null>\n");
    }

    if(thys->Freelines)
	for(test = thys->Freelines; test; test=test->Next)
	{
	    i++;
	    if(test == thys->Freelast) j = i;
	}

    ajDebug(" Free: %d Last: %d\n", i, j);

    return;
}




/* @func ajFileBuffTraceFull **************************************************
**
** Writes debug messages to show the full contents of a buffered file.
**
** @param [r] thys [const AjPFileBuff] Buffered file.
** @param [r] nlines [size_t] Maximum number of lines to trace.
** @param [r] nfree [size_t] Maximum number of free lines to trace.
** @return [void]
** @category use [AjPFileBuff] Writes debug messages to show the full
**                              contents of a buffered file.
** @@
******************************************************************************/

void ajFileBuffTraceFull(const AjPFileBuff thys, size_t nlines,
			  size_t nfree)
{
    ajint i;
    AjPFileBuffList line;
    AjBool last = ajFalse;

    ajDebug("Trace buffer file '%S' End: %B\n"
	     "             Pos: %d Size: %d Nobuff: %B Fpos: %Ld\n",
	     thys->File->Name, thys->File->End,
	     thys->Pos, thys->Size, thys->Nobuff, thys->Fpos);

    line = thys->Lines;
    for(i=1; line && (i <= (ajint)nlines); i++)
    {
	if(line == thys->Curr)
	    ajDebug("*Line %x %d: %5d %5d <%-20S>\n",
		     line->Line, i,
		     ajStrGetLen(line->Line), strlen(ajStrGetPtr(line->Line)),
		     line->Line);
	else
	    ajDebug(" Line %x %d: %5d %5d <%-20S>\n",
		     line->Line, i,
		     ajStrGetLen(line->Line), strlen(ajStrGetPtr(line->Line)),
		     line->Line);
	line = line->Next;
    }

    line = thys->Freelines;
    for(i=1; line && (i <= (ajint)nfree);  i++)
    {
	if(line == thys->Freelast) last = ajTrue;
	ajDebug(" Free %x %d: %d bytes %B\n",
		 line->Line, i, ajStrGetRes(line->Line), last);
	line = line->Next;
    }

    return;
}




/* @func ajFileBuffPrint ******************************************************
**
** Writes the full contents of a buffered file to the debug file
**
** @param [r] thys [const AjPFileBuff] Buffered file.
** @param [r] title [const char*] Report title
** @return [void]
** @@
******************************************************************************/

void ajFileBuffPrint(const AjPFileBuff thys, const char* title)
{
    ajint i;
    AjPFileBuffList line;
    ajint last = 0;

    ajDebug("=== File Buffer: %s ===\n", title);
    line = thys->Lines;
    for(i=1; line; i++)
    {
	ajStrAssignS(&fileTmpStr, line->Line);
	ajStrRemoveLastNewline(&fileTmpStr);
	if(line == thys->Curr)
	    ajDebug("* %x %S\n", line->Line, fileTmpStr);
	else
	    ajDebug("  %x %S\n", line->Line, fileTmpStr);
	line = line->Next;
    }

    line = thys->Freelines;
    for(i=1; line;  i++)
    {
	if(line == thys->Freelast) last = i;
	if(line == thys->Freelast)
	    ajDebug("F %x %S\n", line->Line, fileTmpStr);
	else
	    ajDebug("f %x %S\n", line->Line, fileTmpStr);
	line = line->Next;
    }
    if (!last)
	last = i;
    ajDebug("=== end of file, free list %d lines ===\n", last);

    return;
}




/* ==================================================================== */
/* ============================ Casts ==================================*/
/* ==================================================================== */

/* @section Buffered File Casts ***********************************************
**
** These functions examine the contents of a file object and return some
** derived information. Some of them provide access to the internal
** components of a file object. They are provided for programming convenience
** but should be used with caution.
**
******************************************************************************/




/* @func ajFileBuffIsBuffered *************************************************
**
** Tests whether an input file is buffered.
**
** @param [r] thys [const AjPFileBuff] Buffered file object.
** @return [AjBool] ajTrue if the file was unbuffered before
** @category cast [AjPFileBuff] Tests for input buffering.
** @@
******************************************************************************/

AjBool ajFileBuffIsBuffered(const AjPFileBuff thys)
{
    if(!thys)
	return ajFalse;

    if(thys->Nobuff)
	return ajFalse;

    return ajTrue;
}




/* @func ajFileBuffFp *********************************************************
**
** Returns the C file pointer for an open buffered file.
**
** @param [r] thys [const AjPFileBuff] Buffered file.
** @return [FILE*] C file pointer for the file.
** @@
******************************************************************************/

FILE* ajFileBuffFp(const AjPFileBuff thys)
{
    return thys->File->fp;
}




/* @func ajFileBuffFile *******************************************************
**
** Returns the file object from a buffered file object.
**
** @param [r] thys [const AjPFileBuff] Buffered file.
** @return [AjPFile] File object.
** @@
******************************************************************************/

AjPFile ajFileBuffFile(const AjPFileBuff thys)
{
    return thys->File;
}




/* @section Buffered File Inputs **********************************************
**
** These functions read the contents of a buffered file object
**
******************************************************************************/

/* @func ajFileBuffLoad *******************************************************
**
** Reads all input lines from a file into the buffer.
**
** Intended for cases where the file data must be preprocessed before
** being seen by the sequence reading routines. The first case was
** for stripping HTML tags after reading via HTTP.
**
** @param [u] thys [AjPFileBuff] Buffered file.
** @return [void]
** @@
******************************************************************************/

void ajFileBuffLoad(AjPFileBuff thys)
{
    AjPStr rdline = NULL;
    AjBool status = ajTrue;

    while(status)
    {
	status = ajFileBuffGet(thys, &rdline);
	ajDebug("read: <%S>\n", rdline);
    }

    ajFileBuffReset(thys);
    ajStrDel(&rdline);

    /*ajFileBuffTrace(thys);*/

    return;
}




/* @func ajFileNameDir *****************************************************
**
** Sets the directory part of a filename
**
** @param [u] filename [AjPStr*] Filename.
** @param [r] dir [const AjPDir] Directory
** @param [r] name [const AjPStr] Base filename
** @return [AjBool] ajTrue if the replacement succeeded.
** @@
******************************************************************************/

AjBool ajFileNameDir(AjPStr* filename, const AjPDir dir, const AjPStr name)
{
    AjBool ret = ajFalse;
    if(!dir)
    {
	ajStrAssignS(filename, name);
	return ajFalse;
    }

    ret = ajFileNameDirSet(filename, dir->Name);
    if (!ret)
	return ajFalse;

    ret = ajFileNameExt(filename, dir->Extension);

    return ret;
}




/* @func ajFileNameDirSet *****************************************************
**
** Sets the directory part of a filename
**
** @param [u] filename [AjPStr*] Filename.
** @param [r] dir [const AjPStr] New directory
** @return [AjBool] ajTrue if the replacement succeeded.
** @@
******************************************************************************/

AjBool ajFileNameDirSet(AjPStr* filename, const AjPStr dir)
{
    if(!ajStrGetLen(dir))
	return ajFalse;

    return ajFileNameDirSetC(filename, ajStrGetPtr(dir));
}




/* @func ajFileNameDirSetC ****************************************************
**
** Sets the directory part of a filename
**
** @param [u] filename [AjPStr*] Filename.
** @param [r] dir [const char*] Directory
** @return [AjBool] ajTrue if the replacement succeeded.
** @@
******************************************************************************/

AjBool ajFileNameDirSetC(AjPStr* filename, const char* dir)
{
    AjPStr tmpname = NULL;
    AjPStr tmpdir = NULL;

    if(!dir)
	return ajFalse;

    if(!fileFilenameExp)
#ifndef WIN32
	fileFilenameExp = ajRegCompC("(.*/)?([^/]+)$");
#else
	fileFilenameExp = ajRegCompC("(.*\\\\)?([^\\\\]+)$");
#endif

    if(ajRegExec(fileFilenameExp, *filename))
    {
	ajRegSubI(fileFilenameExp, 1, &tmpdir);
	/* we already have a directory */
	if(ajStrGetLen(tmpdir)) {
	    ajStrDel(&tmpdir);
	    return ajFalse;
	}

	ajRegSubI(fileFilenameExp, 2, &tmpname);

	if(dir[strlen(dir)-1] == SLASH_CHAR)
	    ajFmtPrintS(filename, "%s%S", dir, tmpname);
	else
	    ajFmtPrintS(filename, "%s%s%S", dir, SLASH_STRING,tmpname);
	ajStrDel(&tmpname);
    }

    return ajTrue;
}




/* @func ajFileNameExt ********************************************************
**
** Replaces the extension part of a filename
**
** @param [u] filename [AjPStr*] Filename.
** @param [r] extension [const AjPStr] New file extension
** @return [AjBool] ajTrue if the replacement succeeded.
** @@
******************************************************************************/

AjBool ajFileNameExt(AjPStr* filename, const AjPStr extension)
{
    if(!extension)
	return ajFileNameExtC(filename, NULL);

    return ajFileNameExtC(filename, ajStrGetPtr(extension));
}




/* @func ajFileNameExtC *******************************************************
**
** Replaces the extension part of a filename
**
** @param [u] filename [AjPStr*] Filename.
** @param [r] extension [const char*] New file extension
** @return [AjBool] ajTrue if the replacement succeeded.
** @@
******************************************************************************/

AjBool ajFileNameExtC(AjPStr* filename, const char* extension)
{
    AjBool doext;
    char *p = NULL;

    doext = ajTrue;
    if(!extension || !*extension)
	doext = ajFalse;


    ajDebug("ajFileNameExtC '%S' '%s'\n", *filename, extension);
    ajStrAssignC(&fileTmpStr,ajStrGetPtr(*filename));

    /* Skip any directory path */
    p = strrchr(ajStrGetuniquePtr(&fileTmpStr),SLASH_CHAR);
    if (!p)
	p = ajStrGetuniquePtr(&fileTmpStr);

    p = strrchr(p,'.');
    if(p)
    {
	*p='\0';
	fileTmpStr->Len = p - ajStrGetPtr(fileTmpStr);
    }


    if(doext)
    {
	ajStrAppendC(&fileTmpStr,".");
	ajStrAppendC(&fileTmpStr,extension);
    }

    ajStrAssignS(filename,fileTmpStr);

    ajDebug("result '%S'\n", *filename);

    return ajTrue;
}




/* @func ajFileScan ***********************************************************
**
** Recursively scan through a directory
**
** @param [r] path [const AjPStr] Directory to scan
** @param [r] filename [const AjPStr] Filename to search for (or NULL)
** @param [w] result [AjPList *] List for matching filenames
** @param [r] show [AjBool] Print all files found if set
** @param [r] dolist [AjBool] Store all filenames in a list (if set)
** @param [w] list [AjPList *] List for dolist results
** @param [u] rlist [AjPList] List of directories to ignore
** @param [r] recurs [AjBool] Do recursion
** @param [u] outf [AjPFile] File for "show" results (or NULL)
**
** @return [ajint] number of entries in list
** @@
******************************************************************************/

ajint ajFileScan(const AjPStr path, const AjPStr filename, AjPList *result,
		 AjBool show, AjBool dolist, AjPList *list,
		 AjPList rlist, AjBool recurs, AjPFile outf)
{
    AjPList dirs = NULL;
    AjIList iter = NULL;
    DIR *indir;
#if defined(AJ_IRIXLF)
    struct dirent64 *dp;
#else
    struct dirent *dp;
#endif
    AjPStr s = NULL;
    AjPStr t = NULL;
    AjBool flag;
    AjPStr tpath = NULL;
#ifdef _POSIX_C_SOURCE
    char buf[sizeof(struct dirent)+MAXNAMLEN];
#endif
    
    tpath = ajStrNew();
    ajStrAssignS(&tpath,path);
    
    
    if(dolist)
    {
	t=ajStrNewS(path);
	ajListPushAppend(*list,(void *)t);
    }
    
    if(show)
	ajFmtPrintF(outf,"\n\nDIRECTORY: %S\n\n",path);

    ajDebug("ajFileScan directory: '%S'\n",path);
    
    if(!ajFileDir(&tpath))
    {
	ajStrDel(&tpath);
	return 0;
    }


    if(!(indir=opendir(ajStrGetPtr(tpath))))
    {
	ajStrDel(&tpath);
	return 0;
    }
    
    
    s = ajStrNew();
    dirs = ajListNew();
    
#if defined(AJ_IRIXLF)
#ifdef _POSIX_C_SOURCE
    while(!readdir64_r(indir,(struct dirent64 *)buf,&dp))
    {
	if(!dp)
	    break;
#else
	while((dp=readdir64(indir)))
	{
#endif
#else
#ifdef _POSIX_C_SOURCE
    while(!readdir_r(indir,(struct dirent *)buf,&dp))
    {
	if(!dp)
	    break;
#else
	while((dp=readdir(indir)))
	{
#endif
#endif

#ifndef __CYGWIN__
	if(!dp->d_ino ||
	   !strcmp(dp->d_name,".") ||
	   !strcmp(dp->d_name,".."))
#else
	if(!strcmp(dp->d_name,".") ||
	   !strcmp(dp->d_name,".."))
#endif
	    continue;
	ajStrAssignS(&s,tpath);
	/*	ajStrAppendC(&s,SLASH_STRING);*/
	ajStrAppendC(&s,dp->d_name);

	/* Its a directory */
	if(ajFileStat(s,AJ_FILE_DIR))
	{
	    if(!recurs)
		continue;

	    /* Ignore selected directories */
	    if(rlist)
	    {
		flag = ajFalse;
		iter = ajListIterNewread(rlist);
		while(!ajListIterDone(iter))
		{
		    t = ajListIterGet(iter);
		    if(!strcmp(ajStrGetPtr(t),dp->d_name))
		    {
			flag = ajTrue;
			break;
		    }
		}
		ajListIterDel(&iter);
		if(flag)
		    continue;
	    }

	    if(!ajFileStat(s,AJ_FILE_R) || !ajFileStat(s,AJ_FILE_X))
		continue;
	    t = ajStrNewC(ajStrGetPtr(s));
	    ajListPushAppend(dirs,(void *)t);
	}
	else if(ajFileStat(s,AJ_FILE_R))
	{
	    if(filename)
		if(ajCharMatchWildC(dp->d_name,ajStrGetPtr(filename)))
		{
		    t = ajStrNewS(s);
		    ajListPushAppend(*result,(void *)t);
		}
	    
	    if(dolist)
	    {
		t = ajStrNewS(s);
		ajListPushAppend(*list,(void *)t);
	    }
	    
	    if(show)
		ajFmtPrintF(outf,"  %s\n",dp->d_name);
	    ajDebug("  %s\n",dp->d_name);
	}
    }
    closedir(indir);
    
    if(recurs)
	while(ajListPop(dirs,(void **)&t))
	{
	    ajFileScan(t,filename,result,show,dolist,list,rlist,recurs,outf);
	    ajStrDel(&t);
	}
    
    ajStrDel(&s);
    ajStrDel(&tpath);
    ajListFree(&dirs);


    if(result)
	return ajListGetLength(*result);
    
    return 0;
}




/* @func ajFileTestSkip *******************************************************
**
** Tests a filename against wildcard
** lists of file names to be included and excluded.
**
** By default files are included. The exclusion list is used to trim
** out files, and the inclusion list is then used to add selected
** files again.
**
** @param [r] fullname [const AjPStr] File to test
** @param [r] exc [const AjPStr] List of wildcard names to exclude
** @param [r] inc [const AjPStr] List of wildcard names to include
** @param [r] keep [AjBool] Default to keep if ajTrue, else skip unless
**                          inc is matched.
** @param [r] ignoredirectory [AjBool] Delete directory from name
**                                     before testing.
** @return [AjBool] ajTrue if the filename is accepted.
** @@
******************************************************************************/

AjBool ajFileTestSkip(const AjPStr fullname,
		      const AjPStr exc, const AjPStr inc,
		      AjBool keep, AjBool ignoredirectory)
{
    AjBool ret = keep;

    AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    
    ajStrAssignS(&fileNameTmp, fullname);
    if(ignoredirectory)
	ajFileDirTrim(&fileNameTmp);
    
    if(keep && ajStrGetLen(exc))
    {
	/* keep, so test exclude first */
	ajStrTokenAssignC(&handle, exc, " \t,;\n");

	while(ajStrTokenNextParse(&handle, &token))
	    if(ajStrMatchWildS(fullname, token) ||
		(ignoredirectory && ajStrMatchWildS(fileNameTmp, token)))
		ret = ajFalse;

	ajStrTokenReset(&handle);
    }

    if(ajStrGetLen(inc))
    {
	ajStrTokenAssignC(&handle, inc, " \t,;\n");

	while(ajStrTokenNextParse(&handle, &token))
	    if(ajStrMatchWildS(fullname, token) ||
	       (ignoredirectory && ajStrMatchWildS(fileNameTmp, token)))
		ret = ajTrue;

	ajStrTokenReset(&handle);
    }

    if(!keep && ajStrGetLen(exc))
    {				       /* nokeep, test exclude last */
	ajStrTokenAssignC(&handle, exc, " \t,;\n");

	while(ajStrTokenNextParse(&handle, &token))
	    if(ajStrMatchWildS(fullname, token) ||
		(ignoredirectory && ajStrMatchWildS(fileNameTmp, token)))
		ret = ajFalse;

	ajStrTokenReset(&handle);
    }

    ajStrTokenDel(&handle);
    ajStrDel(&token);

    return ret;
}




/* @func ajFileDirTrim ********************************************************
**
** Trims the directory path (if any) from a filename
**
** @param [u] name [AjPStr*] Filename
** @return [AjBool] ajTrue is there was a directory
******************************************************************************/

AjBool ajFileDirTrim(AjPStr* name)
{
    ajint i;

    if(!ajStrGetLen(*name))
	return ajFalse;

    i = ajStrFindlastC(*name, SLASH_STRING);
    if(i < 0)
	return ajFalse;

    ajStrCutStart(name, i+1);

    return ajTrue;
}



/* @func ajFileExtnTrim *******************************************************
**
** Trims the extension (if any) from a filename
**
** @param [u] name [AjPStr*] Filename
** @return [AjBool] ajTrue is there was an extension
******************************************************************************/

AjBool ajFileExtnTrim(AjPStr* name)
{
    ajint i;
    ajint len;

    if(!ajStrGetLen(*name))
	return ajFalse;

    len = ajStrGetLen(*name);
    i = ajStrFindlastC(*name, ".");

    if(i < 0)
	return ajFalse;

    ajStrCutEnd(name, (len-i));

    return ajTrue;
}




/* @func ajFileDirExtnTrim ****************************************************
**
** Trims the directory path (if any) and extension (if any) from a filename.
**
** @param [u] name [AjPStr*] Filename
** @return [AjBool] ajTrue is there was a directory path or extension.
******************************************************************************/

AjBool ajFileDirExtnTrim(AjPStr* name)
{
    ajint i;
    ajint j;
    ajint len;
    
    if(!ajStrGetLen(*name))
	return ajFalse;

    i = ajStrFindlastC(*name, SLASH_STRING);
    if(i >= 0)
	ajStrCutStart(name, i+1);

    len = ajStrGetLen(*name);
    j = ajStrFindlastC(*name, ".");
    if(j >= 0)
	ajStrCutEnd(name, (len-j));

    if((i < 0) && (j < 0))
	return ajFalse;

    return ajTrue;
}





/* @func ajFileTempName *******************************************************
**
** Returns an available temporary filename that can be opened for writing
** Filename will be of the form progname-time.randomnumber
** Tries 5 times to find a new filename. Returns NULL if not
** successful or the file cannot be opened for writing.
** This function returns only the filename, not a file pointer.
**
** @param [r] dir [const char*] Directory for filename
**                              or NULL for current dir (.)
** @return [const char*] available filename or NULL if error.
** @@
******************************************************************************/

const char* ajFileTempName(const char *dir)
{
#if defined(AJ_IRIXLF)
    struct  stat64 buf;
#else
    struct  stat buf;
#endif
    AjPStr  direct;
    ajint   retry;
    AjBool  ok;
    AjPFile outf;
    
    if(!fileTempFilename)
	fileTempFilename = ajStrNew();
    
    direct = ajStrNew();
    
    if(!dir)
	ajStrAssignC(&direct,".");
    else
	ajStrAssignC(&direct,dir);
    ajStrAppendC(&direct,SLASH_STRING);
    
    
    
    ajFmtPrintS(&fileTempFilename,
		"%S%S-%d.%d",direct,ajAcdGetProgram(),time(0),
		ajRandomNumber());
    
    retry = 5;
    ok    = ajTrue;
    
#if defined(AJ_IRIXLF)
    while(!stat64(ajStrGetPtr(fileTempFilename),&buf) && retry)
#else
	while(!stat(ajStrGetPtr(fileTempFilename),&buf) && retry)
#endif
	{
	    ajFmtPrintS(&fileTempFilename,
			"%S%S-%d.%d",direct,ajAcdGetProgram(),time(0),
			ajRandomNumber());
	    --retry;
	}
    
    if(!retry)
    {
	ajDebug("Cannot find a unique filename [last try %S]\n",
		fileTempFilename);
	ok = ajFalse;
    }
    
    if(!(outf = ajFileNewOut(fileTempFilename)))
    {
	ajDebug("Cannot write to file %S\n",fileTempFilename);
	ok = ajFalse;
    }
    else
    {
	ajFileClose(&outf);
	unlink(ajStrGetPtr(fileTempFilename));
    }
    
    ajStrDel(&direct);
    
    if(!ok)
	return NULL;
    
    return ajStrGetPtr(fileTempFilename);
}




/* @func ajFileWriteByte ******************************************************
**
** Writes a single byte to a binary file
**
** @param [u] thys [AjPFile] Output file
** @param [r] ch [char] Character
** @return [ajint] Return value from fwrite
** @@
******************************************************************************/

ajint ajFileWriteByte(AjPFile thys, char ch)
{
    return fwrite(&ch, 1, 1, ajFileFp(thys));
}




/* @func ajFileWriteChar ******************************************************
**
** Writes a text string to a binary file
**
** @param [u] thys [AjPFile] Output file
** @param [r] str [const char*] Text string
** @param [r] len [ajint] Length (padded) to use in the file
** @return [ajint] Return value from fwrite
** @@
******************************************************************************/

ajint ajFileWriteChar(AjPFile thys, const char* str, ajint len)
{
    static char buf[256];
    ajint i;

    i = strlen(str);

    strcpy(buf, str);
    if(i < len)
	memset(&buf[i], '\0', len-i);

    return fwrite(buf, len, 1, ajFileFp(thys));
}




/* @func ajFileWriteInt2 ******************************************************
**
** Writes a 2 byte integer to a binary file, with the correct byte orientation
**
** @param [u] thys [AjPFile] Output file
** @param [r] i [short] Integer
** @return [ajint] Return value from fwrite
** @@
******************************************************************************/

ajint ajFileWriteInt2(AjPFile thys, short i)
{
    short j;

    j = i;
    if(ajUtilGetBigendian())
	ajByteRevLen2(&j);

    return fwrite(&j, 2, 1, ajFileFp(thys));
}




/* @func ajFileWriteInt4 ******************************************************
**
** Writes a 4 byte integer to a binary file, with the correct byte orientation
**
** @param [u] thys [AjPFile] Output file
** @param [r] i [ajint] Integer
** @return [ajint] Return value from fwrite
** @@
******************************************************************************/

ajint ajFileWriteInt4(AjPFile thys, ajint i)
{
    ajint j;

    j = i;

    if(ajUtilGetBigendian())
	ajByteRevLen4(&j);

    return fwrite(&j, 4, 1, ajFileFp(thys));
}




/* @func ajFileWriteInt8 ******************************************************
**
** Writes an 8 byte long to a binary file, with the correct byte orientation
**
** @param [u] thys [AjPFile] Output file
** @param [r] l [ajlong] Integer
** @return [ajint] Return value from fwrite
** @@
******************************************************************************/

ajint ajFileWriteInt8(AjPFile thys, ajlong l)
{
    ajlong j;

    j = l;

    if(ajUtilGetBigendian())
	ajByteRevLen8(&j);

    return fwrite(&j, 8, 1, ajFileFp(thys));
}




/* @func ajFileWriteStr *******************************************************
**
** Writes a string to a binary file
**
** @param [u] thys [AjPFile] Output file
** @param [r] str [const AjPStr] String
** @param [r] len [ajuint] Length (padded) to use in the file
** @return [ajint] Return value from fwrite
** @@
******************************************************************************/

ajint ajFileWriteStr(AjPFile thys, const AjPStr str, ajuint len)
{
    static char buf[256];
    ajuint i;

    i = ajStrGetLen(str);
    strcpy(buf, ajStrGetPtr(str));
    if(i < len)
	memset(&buf[i], '\0', len-i);

    return fwrite(buf, len, 1, ajFileFp(thys));
}




/* @func ajFileBuffStripHtmlPre ***********************************************
**
** If we only have one pre-formatted section in HTML, that is all we keep.
**
** @param [u] thys [AjPFileBuff] buffer
** @return [AjBool] ajTrue=cleaned ajFalse=unchanged
** @@
******************************************************************************/

AjBool ajFileBuffStripHtmlPre(AjPFileBuff thys)
{
    AjPFileBuffList lptr    = NULL;
    AjPFileBuffList tptr    = NULL;
    AjPRegexp preexp = NULL;
    AjPRegexp endexp = NULL;
    ajint ifound = 0;
    
    lptr = thys->Curr;
    
    preexp = ajRegCompC("<[Pp][Rr][Ee]>");

    lptr=thys->Curr;
    
    ajDebug("ajFileBuffStripHtmlPre testing for <pre> line(s)\n");
    while(lptr)
    {
	if(ajRegExec(preexp, lptr->Line))
	    ifound++;
	lptr = lptr->Next;
    }
    
    if(!ifound)
    {
	ajRegFree(&preexp);
	return ajFalse;
    }

    if (ifound > 1)
    {
	ajRegFree(&preexp);
	return ajFalse;
    }

    lptr=thys->Curr;
    

    while(lptr && !ajRegExec(preexp, lptr->Line))
    {
	tptr = lptr;
	lptr = lptr->Next;
	ajStrDel(&tptr->Line);
	AJFREE(tptr);
	thys->Size--;
    }
    
    thys->Lines = thys->Curr = lptr;
    ajRegPost(preexp, &lptr->Line);
    ajRegFree(&preexp);

    endexp = ajRegCompC("</[Pp][Rr][Ee]>");

    while(lptr && !ajRegExec(endexp,lptr->Line))
    {
	lptr    = lptr->Next;
    }
    
    ajRegPre(endexp, &lptr->Line);
    thys->Last = lptr;
    lptr = lptr->Next;
    ajRegFree(&endexp);

    while(lptr)
    {
	tptr = lptr;
	lptr = lptr->Next;
	ajStrDel(&tptr->Line);
	AJFREE(tptr);
	thys->Size--;
    }

    thys->Last->Next = NULL;
    ajFileBuffReset(thys);
    ajFileBuffPrint(thys, "ajFileBuffStripHtmlPre completed");

    return ajTrue;
}




/* @funcstatic fileListRecurs  ************************************************
**
** Add a filename, expanded wildcard filenames and list file contents to
** a list
**
** @param [r] srcfile [const AjPStr] filename, wildfilename or listfilename
** @param [u] list [AjPList] result filename list
** @param [u] recurs [ajint *] recursion level counter
**
** @return [void]
** @@
******************************************************************************/

static void fileListRecurs(const AjPStr srcfile, AjPList list, ajint *recurs)
{
    char c;
    AjPStr ptr = NULL;
    AjPStr dir = NULL;
    char   *p;
    AjPList dlist;
    AjPFile inf;
    AjPStr  line = NULL;
    AjPStr file  = NULL;
    
    ++(*recurs);
    if(*recurs > FILERECURSLV)
	ajFatal("Filelist maximum recursion level reached");

    ajStrAssignS(&file, srcfile);
    ajStrTrimWhite(&file);
    c = *ajStrGetPtr(file);
    
    dir   = ajStrNew();
    line  = ajStrNew();
    dlist = ajListNew();
    
    
    if(ajStrIsWild(file))
    {
	if(!(p=strrchr(ajStrGetPtr(file),(int)SLASH_CHAR)))
	    ajStrAssignC(&dir,CURRENT_DIR);
	else
	    ajStrAssignSubC(&dir,ajStrGetPtr(file),0,p-ajStrGetPtr(file));
	ajFileScan(dir,file,&dlist,ajFalse,ajFalse,NULL,NULL,ajFalse,NULL);
	while(ajListPop(dlist,(void **)&ptr))
	{
	    if(ajStrPrefixC(ptr,CURRENT_DIR))
		ajStrCutStart(&ptr,2);
	    ajListPushAppend(list,(void *)ptr);
	}
    }
    else if(c=='@')
    {
	if((inf=ajFileNewInC(ajStrGetPtr(file)+1)))
	    while(ajFileReadLine(inf,&line))
		fileListRecurs(line,list,recurs);
	if(inf)
	    ajFileClose(&inf);
    }
    else if(ajStrPrefixC(file,"list::"))
    {
	if((inf=ajFileNewInC(ajStrGetPtr(file)+6)))
	    while(ajFileReadLine(inf,&line))
		fileListRecurs(line,list,recurs);
	if(inf)
	    ajFileClose(&inf);
    }
    else
    {
	ptr = ajStrNewC(ajStrGetPtr(file));
	ajListPushAppend(list,(void *)ptr);
    }
    
    
    ajListFree(&dlist);
    ajStrDel(&dir);
    ajStrDel(&line);
    ajStrDel(&file);
    
    --(*recurs);
    
    return;
}




/* @func ajFileFileList *******************************************************
**
** Return a list of files that match a comma-separated string of
** filenames which can include wildcards or listfiles
**
** @param [r] files [const AjPStr] comma-separated filename list
**
** @return [AjPList] or NULL if no files were specified
** @@
******************************************************************************/

AjPList ajFileFileList(const AjPStr files)
{
    AjPStr *fstr = NULL;
    ajint  ncl;
    ajint  i;
    ajint  rlevel = 0;
    AjPList list;

    list = ajListNew();

    ncl = ajArrCommaList(files,&fstr);
    for(i=0;i<ncl;++i)
    {
	fileListRecurs(fstr[i],list,&rlevel);
	ajStrDel(&fstr[i]);
    }

    AJFREE(fstr);

    if(!ajListGetLength(list))
    {
	ajListFree(&list);
	return NULL;
    }

    return list;
}




/* @funcstatic fileBuffLineAdd ************************************************
**
** Appends a line to a buffer.
**
** @param [u] thys [AjPFileBuff] File buffer
** @param [r] line [const AjPStr] Line
** @return [void]
******************************************************************************/

static void fileBuffLineAdd(AjPFileBuff thys, const AjPStr line)
{
    /* ajDebug("fileBuffLineAdd '%S'\n", line);*/
    if(thys->Freelines)
    {
	if(!thys->Lines)
	{
	    /* Need to set first line in list */
	    thys->Lines = thys->Freelines;
	}
	else
	    thys->Last->Next = thys->Freelines;

	thys->Last = thys->Freelines;
	thys->Freelines = thys->Freelines->Next;
	if(!thys->Freelines)
	{
	    /* Free list now empty */
	    thys->Freelast = NULL;
	}
    }
    else
    {
	/* No Free list, make a new string */
	if(!thys->Lines)
	    thys->Lines = AJNEW0(thys->Last);
	else
	    thys->Last = AJNEW0(thys->Last->Next);
    }
    
    ajStrAssignS(&thys->Last->Line, line);
    thys->Prev = thys->Curr;
    thys->Curr = thys->Last;
    thys->Last->Next = NULL;
    thys->Last->Fpos = thys->Fpos;
    thys->Pos++;
    thys->Size++;
        
    return;
}




/* @funcstatic fileBuffLineDel ************************************************
**
** Delete a line from a buffer.
**
** @param [u] thys [AjPFileBuff] File buffer
** @return [void]
******************************************************************************/

static void fileBuffLineDel(AjPFileBuff thys)
{
    AjPFileBuffList saveprev;

    if(!thys->Curr)
	return;

    ajDebug("fileBuffLineDel removing line [%d%s%d], '%S' len %d\n",
	     thys->Pos, SLASH_STRING, thys->Size, thys->Curr->Line,
	     ajStrGetLen(thys->Curr->Line));

    /* first line */
    if(!thys->Prev)
    {
	thys->Prev = thys->Lines;
	thys->Curr = thys->Lines = thys->Lines->Next;
	ajStrDel(&thys->Prev->Line);
	AJFREE(thys->Prev);
	--thys->Size;
	return;
    }

    /* last line */
    if(!thys->Curr->Next)
    {
	saveprev = thys->Prev;
	thys->Prev = thys->Lines;
	while(thys->Prev && thys->Prev->Next != saveprev)
	    thys->Prev = thys->Prev->Next;
	saveprev->Next = NULL;

	ajStrDel(&thys->Curr->Line);
	AJFREE(thys->Curr);
	thys->Curr = NULL;
	thys->Last = saveprev;
	--thys->Size;
	thys->Pos = thys->Size;
	return;
    }

    thys->Prev->Next = thys->Curr->Next;
    ajStrDel(&thys->Curr->Line);
    AJFREE(thys->Curr);
    thys->Curr = thys->Prev->Next;
    --thys->Size;

    return;
}




/* @funcstatic fileBuffLineNext ***********************************************
**
** Steps the Curr pointer to the next line in a buffer.
**
** Not for use when reading from a file. This steps through the buffer
**
** @param [u] thys [AjPFileBuff] File buffer
** @return [AjBool] ajTrue if there was another line
******************************************************************************/

static AjBool fileBuffLineNext(AjPFileBuff thys)
{
    if(thys->Pos < thys->Size)
    {
	thys->Prev = thys->Curr;
	thys->Curr = thys->Curr->Next;
	thys->Pos++;
	return ajTrue;
    }

    return ajFalse;
}

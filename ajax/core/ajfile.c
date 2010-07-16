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
#endif
#include <string.h>
#include <errno.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <limits.h>
#include <fcntl.h>

#ifdef WIN32
#define PATH_MAX _MAX_PATH
#define getcwd _getcwd
#define fileno _fileno
#endif

#define FILERECURSLV 20

static ajuint fileBuffSize = 2049;
static AjBool fileUsedStdin  = AJFALSE;
static AjBool fileUsedStdout = AJFALSE;
static AjBool fileUsedStderr = AJFALSE;

static ajint fileHandle = 0;
static ajint fileOpenCnt = 0;
static ajint fileOpenMax = 0;
static ajint fileCloseCnt = 0;
static ajint fileOpenTot = 0;

static AjPStr fileNameFix = NULL;
static AjPStr fileNameStrTmp = NULL;
static AjPStr fileNameTmp = NULL;
static AjPStr fileDirfixTmp = NULL;
static AjPStr fileCwd = NULL;
static AjPStr fileTmpStr = NULL;
static AjPStr fileTempFilename = NULL;
static AjPStr fileDirectory = NULL;

static AjPRegexp fileUserExp = NULL;
static AjPRegexp fileWildExp = NULL;
static AjPRegexp fileEntryExp = NULL;
static AjPRegexp fileFileExp = NULL;
static AjPRegexp fileRestExp = NULL;
static AjPRegexp fileDirExp = NULL;
static AjPRegexp fileFilenameExp = NULL;

static void   fileBuffInit(AjPFilebuff thys);
static void   fileBuffLineDel(AjPFilebuff thys);
static AjBool fileBuffLineNext(AjPFilebuff thys);
static void   fileClose(AjPFile thys);
static void   fileListRecurs(const AjPStr file, AjPList list, ajint *recurs);
static DIR*   fileOpenDir(AjPStr *dir);
static void   filebuffFreeClear(AjPFilebuff buff);

#ifdef __CYGWIN__
#define fopen(a,b) ajSysFuncFopen(a,b)
#endif




/* @filesection ajfile ********************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
*/




/* @datasection [AjPDir] Directory *******************************************
**
** Function is for manipulating directories and returns or takes at least one 
** AjPDir argument.
** 
** @nam2rule Dir  
**
*/




/* @section Directory Constructors ********************************************
**
** @fdata [AjPDir]
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
** @nam3rule New Constructor
** @nam4rule Path Input file directory path
** @suffix Pre Filename prefix(es) specified
** @suffix Ext Filename extension(s) specified
**
** @argrule Path path [const AjPStr] Path (full or relative) to directory
** @argrule Pre prefix [const AjPStr] List of prefix wildcards for filename
**                                    prefixes to be used
** @argrule Ext ext [const AjPStr] List of prefix wildcards for filename
**                                    extensions to be used
**
** @valrule * [AjPDir] Directory object
**
** @fcategory new
**
******************************************************************************/




/* @func ajDirNewPath ********************************************************
**
** Creates a new directory object.
**
** @param [r] path [const AjPStr] Directory name
** @return [AjPDir] New directory object.
** @@
******************************************************************************/

AjPDir ajDirNewPath(const AjPStr path)
{
    AjPDir thys;

    AJNEW0(thys);
    ajStrAssignS(&thys->Name, path);
    thys->Prefix = NULL;
    thys->Extension = NULL;

    return thys;
}




/* @obsolete ajDirNew
** @rename ajDirNewPath
*/
__deprecated AjPDir ajDirNew(const AjPStr name)
{
    return ajDirNewPath(name);
}




/* @func ajDirNewPathExt ******************************************************
**
** Creates a new directory object.
**
** @param [r] path [const AjPStr] Directory name
** @param [r] ext [const AjPStr] File extension
** @return [AjPDir] New directory object.
** @@
******************************************************************************/

AjPDir ajDirNewPathExt(const AjPStr path, const AjPStr ext)
{
    AjPDir thys;

    AJNEW0(thys);
    ajStrAssignS(&thys->Name, path);

    if (ajStrMatchC(ext, " "))
	ajStrAssignC(&thys->Extension, "");
    else if (ajStrGetLen(ext))
	ajStrAssignS(&thys->Extension, ext);

    return thys;
}




/* @obsolete ajDirNewS
** @rename ajDirNewPathExt
*/
__deprecated AjPDir ajDirNewS(const AjPStr name, const AjPStr ext)
{
    return ajDirNewPathExt(name, ext);
}




/* @func ajDirNewPathPreExt ***************************************************
**
** Creates a new directory object.
**
** @param [r] path [const AjPStr] Directory name
** @param [r] prefix [const AjPStr] Filename prefix
** @param [r] ext [const AjPStr] Filename extension
** @return [AjPDir] New directory object.
** @@
******************************************************************************/

AjPDir ajDirNewPathPreExt(const AjPStr path,
                          const AjPStr prefix, const AjPStr ext)
{
    AjPDir thys;

    AJNEW0(thys);
    ajStrAssignS(&thys->Name, path);

    if (ajStrMatchC(prefix, " "))
	ajStrAssignC(&thys->Prefix, "");
    else if (ajStrGetLen(prefix))
	ajStrAssignS(&thys->Prefix, prefix);

    if (ajStrMatchC(ext, " "))
	ajStrAssignC(&thys->Extension, "");
    else if (ajStrGetLen(ext))
	ajStrAssignS(&thys->Extension, ext);

    return thys;
}




/* @obsolete ajDirNewSS
** @rename ajDirNewPathPreExt
*/
__deprecated AjPDir ajDirNewSS(const AjPStr name,
                               const AjPStr prefix, const AjPStr ext)
{
    return ajDirNewPathPreExt(name,prefix,ext);
}




/* @section Directory Destructors *********************************************
**
** @fdata [AjPDir]
**
** Destruction is achieved by deleting the object.
**
** @nam3rule Del Destructor
**
** @argrule Del Pdir [AjPDir*] Directory to be deleted
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajDirDel *******************************************************
**
** Close and free a directory object.
**
** @param [d] Pdir [AjPDir*] Directory object.
** @return [void]
** @@
******************************************************************************/

void ajDirDel(AjPDir* Pdir)
{
    AjPDir thys;

    thys = *Pdir;

    if(!thys)
	return;

    ajStrDel(&thys->Name);
    ajStrDel(&thys->Prefix);
    ajStrDel(&thys->Extension);
    AJFREE(*Pdir);

    return;
}




/* @section Directory element retrieval ***************************************
**
** @fdata [AjPDir]
**
** Returns attributes of a directory
**
** @nam3rule Get Return attribute value
** @nam4rule Ext Return file extension
** @nam4rule Prefix Return filename prefix
** @nam4rule Path Return directory path
**
** @argrule Get thys [const AjPDir] Directory
**
** @valrule * [const AjPStr] String value
**
** @fcategory use
**
******************************************************************************/




/* @func ajDirGetExt **********************************************************
**
** Returns the extension(s) of a directory object
**
** @param [r] thys [const AjPDir] Directory object.
** @return [const AjPStr] Directory name
** @@
******************************************************************************/

const AjPStr ajDirGetExt(const AjPDir thys)
{
    if (!thys)
	return NULL;

    return thys->Extension;
}




/* @obsolete ajDirExt
** @rename ajDirGetExt
*/
__deprecated const AjPStr ajDirExt(const AjPDir thys)
{
    return ajDirGetExt(thys);
}




/* @func ajDirGetPath *********************************************************
**
** Returns the full path of a directory object
**
** @param [r] thys [const AjPDir] Directory object.
** @return [const AjPStr] Directory name
** @@
******************************************************************************/

const AjPStr ajDirGetPath(const AjPDir thys)
{
    if (!thys)
	return NULL;

    return thys->Name;
}




/* @obsolete ajDirName
** @rename ajDirGetPath
*/
__deprecated const AjPStr ajDirName(const AjPDir thys)
{
    return ajDirGetPath(thys);
}




/* @func ajDirGetPrefix *******************************************************
**
** Returns the filename prefix(es) of a directory object
**
** @param [r] thys [const AjPDir] Directory object.
** @return [const AjPStr] Directory name
** @@
******************************************************************************/

const AjPStr ajDirGetPrefix(const AjPDir thys)
{
    if (!thys)
	return NULL;
    return thys->Prefix;
}




/* @datasection [AjPDirout] Output directory **********************************
**
** Function is for manipulating output directories and returns or
** takes at least one AjPDirout argument.
** 
** @nam2rule Dirout
**
*/




/* @section Output directory Constructors *************************************
**
** @fdata [AjPDirout]
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
** @nam3rule New Constructor
** @nam4rule Path Output file directory path
** @suffix Pre Filename prefix(es) specified
** @suffix Ext Filename extension(s) specified
**
** @argrule Path path [const AjPStr] Path (full or relative) to directory
** @argrule Pre prefix [const AjPStr] List of prefix wildcards for filename
**                                    prefixes to be used
** @argrule Ext ext [const AjPStr] List of prefix wildcards for filename
**                                    extensions to be used
**
** @valrule * [AjPDirout] Output directory object
**
** @fcategory new
**
******************************************************************************/




/* @func ajDiroutNewPath ******************************************************
**
** Creates a new directory output object.
**
** @param [r] path [const AjPStr] Directory name
** @return [AjPDirout] New directory object.
** @@
******************************************************************************/

AjPDirout ajDiroutNewPath(const AjPStr path)
{
    AjPDirout thys;

    AJNEW0(thys);
    ajStrAssignS(&thys->Name, path);
    thys->Extension = NULL;

    return thys;
}




/* @obsolete ajDirOutNew
** @remove Use ajDiroutNewPath
*/
__deprecated AjPDir ajDirOutNew(const AjPStr name)
{
    (void)name;
    return NULL;
}




/* @func ajDiroutNewPathExt ***************************************************
**
** Creates a new directory output object.
**
** @param [r] path [const AjPStr] Directory name
** @param [r] ext [const AjPStr] File extension
** @return [AjPDirout] New directory object.
** @@
******************************************************************************/

AjPDirout ajDiroutNewPathExt(const AjPStr path, const AjPStr ext)
{
    AjPDirout thys;

    AJNEW0(thys);
    ajStrAssignS(&thys->Name, path);

    if (ajStrGetLen(ext))
	ajStrAssignS(&thys->Extension, ext);

    return thys;
}




/* @obsolete ajDirOutNewS
** @remove Use ajDiroutNewPathExt
*/
__deprecated AjPDir ajDirOutNewS(const AjPStr name, const AjPStr ext)
{
    (void)name;
    (void)ext;

    return NULL;
}




/* @obsolete ajDirOutNewSS
** @remove Use ajDiroutNewPathExt (prefix not relevant)
*/
__deprecated AjPDir ajDirOutNewSS(const AjPStr name,
                                  const AjPStr prefix, const AjPStr ext)
{
    (void)name;
    (void) prefix;
    (void)ext;

    return NULL;
}




/* @section Output directory Destructors **************************************
**
** @fdata [AjPDirout]
**
** Destruction is achieved by deleting the object.
**
** @nam3rule Del Destructor
**
** @argrule Del Pdir [AjPDirout*] Directory to be deleted
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajDiroutDel *******************************************************
**
** Close and free a directory object.
**
** @param [d] Pdir [AjPDirout*] Directory object.
** @return [void]
** @@
******************************************************************************/

void ajDiroutDel(AjPDirout* Pdir)
{
    AjPDirout thys;

    thys = *Pdir;

    if(!thys)
	return;

    ajStrDel(&thys->Name);
    ajStrDel(&thys->Extension);
    AJFREE(*Pdir);

    return;
}




/* @section Output directory element retrieval ********************************
**
** @fdata [AjPDirout]
**
** Returns attributes of a directory
**
** @nam3rule Get Return attribute value
** @nam4rule Ext Return file extension
** @nam4rule Path Return directory path
**
** @argrule Get thys [const AjPDirout] Directory
**
** @valrule * [const AjPStr] String value
**
** @fcategory use
**
******************************************************************************/




/* @func ajDiroutGetExt *******************************************************
**
** Returns the extension of an output directory object
**
** @param [r] thys [const AjPDirout] Directory object.
** @return [const AjPStr] Directory name
** @@
******************************************************************************/

const AjPStr ajDiroutGetExt(const AjPDirout thys)
{
    if (!thys)
	return NULL;

    return thys->Extension;
}




/* @func ajDiroutGetPath *****************************************************
**
** Returns the name of an output directory object
**
** @param [r] thys [const AjPDirout] Directory object.
** @return [const AjPStr] Directory name
** @@
******************************************************************************/

const AjPStr ajDiroutGetPath(const AjPDirout thys)
{
    if (!thys)
	return NULL;

    return thys->Name;
}




/* @section Output directory element modifiers ********************************
**
** @fdata [AjPDirout]
**
** Modifies attributes of a directory
**
** @nam3rule Exists tests whether directory exists already
** @nam3rule Open Open the directory, creating if needed
**
** @argrule * thys [AjPDirout] Directory
**
** @valrule * [AjBool] True on success
**
** @fcategory use
**
******************************************************************************/




/* @func ajDiroutExists ********************************************************
**
** Tests a directory output object is for an existing directory
**
** @param [u] thys [AjPDirout] Directory name
** @return [AjBool] True on success.
** @@
******************************************************************************/

AjBool ajDiroutExists(AjPDirout thys)
{
    if(ajStrGetCharLast(thys->Name) != SLASH_CHAR)
	ajStrAppendC(&thys->Name, SLASH_STRING);

    if(!ajFilenameExistsDir(thys->Name))
        return ajFalse;

    return ajTrue;
}




/* @func ajDiroutOpen *********************************************************
**
** Opens a directory output object, creating it if it does not already exist
**
** @param [u] thys [AjPDirout] Directory name
** @return [AjBool] True on success.
** @@
******************************************************************************/

AjBool ajDiroutOpen(AjPDirout thys)
{
    if(ajStrGetCharLast(thys->Name) != SLASH_CHAR)
	ajStrAppendC(&thys->Name, SLASH_STRING);

    if(!ajFilenameExists(thys->Name))
        ajSysCommandMakedirS(thys->Name);
   
    if(!ajFilenameExistsDir(thys->Name))
        return ajFalse;

    return ajTrue;
}




/* @datasection [AjPFile] File object *****************************************
**
** Function is for manipulating buffered files and returns or takes at least
** one AjSFileBuff argument.
** 
** Function is for manipulating file and file-related objects and usually
** processes an AjSOutfile, AjSDir, AjSFileBuff, AjSFileBuffList or AjSFile 
** object.
** 
** @nam2rule File       
*/




/* @section File Constructors *************************************************
**
** @fdata [AjPFile]
**
** All constructors return a new open file by pointer. It is the responsibility
** of the user to first destroy any previous file pointer. The target pointer
** does not need to be initialised to NULL, but it is good programming practice
** to do so anyway.
**
** The range of constructors is provided to allow flexibility in how
** applications can open files to read and write various kinds of data.
**
** @nam3rule New Constructor
** @nam4rule From Create from an already open file
** @nam4rule In Input file
** @nam4rule Listin List of one or more input files
** @nam4rule Out Output file created or rewritten
** @nam4rule Outappend Output file appended to existing content
** @nam5rule Block File opened for block read with internal system functions
** @nam5rule FromCfile C FILE* structure used to create file object
** @nam5rule Pipe Read from piped output of command
** @nam5rule ListinList List of files specified
** @nam5rule InPath Input directory path specified
** @nam5rule OutPath Output directory path specified
** @nam5rule Dir Input directory specified
** @nam6rule ListinNameDir Input directory specified
** @nam6rule OutNameDir Output directory specified
**
** @suffix Name Existing filename specified
** @suffix C Filename as C character string
** @suffix S Filename as string
** @suffix Pre Filename prefix(es) specified
** @suffix Ext Filename extension(s) specified
** @suffix Dir Input directory specified
** @suffix Path Input directory path specified
** @suffix Wild Wildcard filename
** @suffix Exclude Filename exclusion wildcard(s)
**
**
**
** @argrule C name [const char*] Filename
** @argrule S name [const AjPStr] Filename
**
** @argrule Block blocksize [ajuint] Block size for buffered system reads
** @argrule Cfile file [FILE*] C file pointer
** @argrule Pipe command [const AjPStr] Commandline
** @argrule ListinList list [AjPList] List of filenames as strings
** @argrule ListinNameDir dir [const AjPDir] Input directory
** @argrule ListinDir dir [const AjPDir] Input directory
** @argrule InNameDir dir [const AjPDir] Input directory
** @argrule OutNameDir dir [const AjPDirout] Output directory
** @argrule Path path [const AjPStr] Input directory path
** @argrule OutPath path [const AjPStr] Output directory path
** @argrule Pre prefix [const AjPStr] List of prefix wildcards for filename
**                                    prefixes to be used
** @argrule Wild wildname [const AjPStr] Wildcard filename
** @argrule Exclude exclude [const AjPStr] Filename exclusion wildcard(s)
** @argrule Ext ext [const AjPStr] List of wildcards for filename
**                                    extensions to be used
**
** @valrule * [AjPFile] New file object
** @fcategory new
**
******************************************************************************/




/* @func ajFileNewFromCfile ***************************************************
**
** Creates a new file object from an open C file.
**
** The file is for input, output, or append - depending on how the C FILE
** pointer was opened.
**
** @param [u] file [FILE*] C file.
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajFileNewFromCfile(FILE* file)
{
    AjPFile thys;

    if(!file)
	ajFatal("Trying to create an AJAX file from a bad C RTL FILE*");

    AJNEW0(thys);
    thys->fp     = file;
    thys->Handle = ++fileHandle;

    if(file == stdout)
        thys->Name = ajStrNewC("stdout");
    else if(file == stderr)
        thys->Name = ajStrNewC("stderr");
    else if(file == stdin)
        thys->Name = ajStrNewC("stdin");
    else
        thys->Name = ajStrNew();

    thys->End    = ajFalse;

    fileOpenCnt++;
    fileOpenTot++;

    if(fileOpenCnt > fileOpenMax)
	fileOpenMax = fileOpenCnt;

    if(file == stdin)
	fileUsedStdin = ajTrue;
    else if(file == stdout)
	fileUsedStdout = ajTrue;
    else if(file == stderr)
	fileUsedStderr = ajTrue;

    ajDebug("Created file from C FILE %p\n", file);

    return thys;
}




/* @obsolete ajFileNewF
** @rename ajFileNewFromCfile
*/
__deprecated AjPFile ajFileNewF(FILE* file)
{
    return ajFileNewFromCfile(file);
}




/* @funcstatic fileNew ********************************************************
**
** Creates a new file object.
**
** @return [AjPFile] New file object.
** @@
******************************************************************************/

static AjPFile fileNew(void)
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




/* @obsolete ajFileNew
** @remove Use a constructor that opens a real file
*/

__deprecated AjPFile ajFileNew(void)
{
    return fileNew();
}




/* @func ajFileNewInBlockS ****************************************************
**
** Creates a new file object to read a named file using blocked fread calls
**
** If the filename ends with a pipe character then a pipe is opened
** using ajFileNewInPipe.
**
** @param [r] name [const AjPStr] File name.
** @param [r] blocksize [ajuint] Block size
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajFileNewInBlockS(const AjPStr name, ajuint blocksize)
{
    AjPFile ret;
    ret = ajFileNewInNameS(name);
    ret->Blocksize = blocksize;

    if(blocksize) 
    {
        ret->Workbuffer = ajCharNewRes(blocksize);
        /*setvbuf(ret->fp, ret->Workbuffer, _IOFBF, blocksize);*/
        ret->Readblock = ajCharNewRes(blocksize+1);
        ret->Blockpos = 0;
        ret->Blocklen = 0;
        ret->Blocksize = blocksize;
    }

    ajDebug("ajFileNewInBlock '%S' blocksize:%u\n", name, blocksize);

    return ret;
}




/* @func ajFileNewInNameC ******************************************************
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

AjPFile ajFileNewInNameC(const char *name)
{
    ajStrAssignC(&fileNameStrTmp, name);

    return ajFileNewInNameS(fileNameStrTmp);
}




/* @obsolete ajFileNewInC
** @rename ajFileNewInNameC
*/

__deprecated AjPFile ajFileNewInC(const char *name)
{
    return ajFileNewInNameC(name);
}




/* @func ajFileNewInNameS *****************************************************
**
** Creates a new file object to read a named file.
**
** If the filename ends with a pipe character then a pipe is opened
** using ajFileNewInPipe.
**
** @param [r] name [const AjPStr] File name.
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajFileNewInNameS(const AjPStr name)
{
    AjPFile thys          = NULL;
    AjPStr userstr = NULL;
    AjPStr reststr = NULL;

    AjPStr dirname        = NULL;
    AjPStr wildname       = NULL;
    AjPFile ptr;

    char *hdir = NULL;
    
    ajDebug("ajFileNewInNameS '%S'\n", name);
    
    if(ajStrMatchC(name, "stdin"))
    {
	thys = ajFileNewFromCfile(stdin);
	ajStrAssignC(&thys->Name, "stdin");

	return thys;
    }

    ajStrAssignS(&fileNameTmp, name);
    
    if(ajStrGetCharLast(name) == '|')	/* pipe character at end */
	return ajFileNewInPipe(name);

    if(ajStrGetCharFirst(fileNameTmp) == '~')
    {
	ajDebug("starts with '~'\n");

	if(!fileUserExp)
            fileUserExp = ajRegCompC("^~([^/\\\\]*)");

	ajRegExec(fileUserExp, fileNameTmp);
	ajRegSubI(fileUserExp, 1, &userstr);
	ajRegPost(fileUserExp, &reststr);
	ajDebug("  user: '%S' rest: '%S'\n", userstr, reststr);

	if(ajStrGetLen(userstr))
	{
	    /* username specified */
            hdir = ajSysGetHomedirFromName(ajStrGetPtr(userstr));
            
	    if(!hdir)
            {
		ajStrDel(&userstr);
		ajStrDelStatic(&fileNameTmp);
		ajStrDel(&reststr);

		return NULL;
	    }

	    ajFmtPrintS(&fileNameTmp, "%s%S", hdir, reststr);
            AJFREE(hdir);
            
	    ajDebug("use getpwnam: '%S'\n", fileNameTmp);
	}
	else
	{
	    /* just ~/ */
            hdir = ajSysGetHomedir();
            
            if(hdir)
            {
		ajFmtPrintS(&fileNameTmp, "%s%S", hdir, reststr);
                AJFREE(hdir);
            }
	    else
		ajFmtPrintS(&fileNameTmp,"%S",reststr);

	    ajDebug("use HOME: '%S'\n", fileNameTmp);
	}
    }

    ajStrDel(&userstr);
    ajStrDel(&reststr);

    if(!fileWildExp)
	fileWildExp = ajRegCompC("(.*/)?([^/]*[*?][^/]*)$");
    
    if(ajRegExec(fileWildExp, fileNameTmp))
    {
	/* wildcard file names */
	ajRegSubI(fileWildExp, 1, &dirname);
	ajRegSubI(fileWildExp, 2, &wildname);
	ajDebug("wild dir '%S' files '%S'\n", dirname, wildname);
	ptr = ajFileNewListinPathWild(dirname, wildname);
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




/* @obsolete ajFileNewIn
** @rename ajFileNewInNameS
*/
__deprecated AjPFile ajFileNewIn(const AjPStr name)
{

    return ajFileNewInNameS(name);
}




/* @func ajFileNewInNamePathC *************************************************
**
** Opens directory "dir"
** Looks for file "file"
**
** @param [r] name [const char*] Filename.
** @param [r] path [const AjPStr] Directory
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajFileNewInNamePathC(const char* name, const AjPStr path)
{
    ajStrAssignC(&fileNameStrTmp, name);

    return ajFileNewInNamePathS(fileNameStrTmp, path);
}




/* @func ajFileNewInNamePathS *************************************************
**
** Opens directory "dir"
** Looks for file "file"
**
** @param [r] name [const AjPStr] Filename.
** @param [r] path [const AjPStr] Directory
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajFileNewInNamePathS(const AjPStr name, const AjPStr path)
{
    if(ajStrGetLen(path))
	ajStrAssignS(&fileNameFix, path);
    else
	ajStrAssignC(&fileNameFix, CURRENT_DIR);
    
    if(ajStrGetCharLast(fileNameFix) != SLASH_CHAR)
	ajStrAppendC(&fileNameFix, SLASH_STRING);
    
    ajStrAppendS(&fileNameFix, name);
     
    return ajFileNewInNameS(fileNameFix);
}




/* @obsolete ajFileNewDC
** @remove Use ajFileNewInNamePathS
*/
__deprecated AjPFile ajFileNewDC(const AjPStr dir, const char* filename)
{
    ajStrAssignC(&fileNameStrTmp, filename);
    return ajFileNewInNamePathS(fileNameStrTmp, dir);
}




/* @obsolete ajFileNewDF
** @replace ajFileNewInNamePathS (1,2/2,1)
*/
__deprecated AjPFile ajFileNewDF(const AjPStr dir, const AjPStr filename)
{
    return ajFileNewInNamePathS(filename, dir);
}




/* @func ajFileNewInPipe ******************************************************
**
** Creates a new file object to read the output from a command.
**
** @param [r] command [const AjPStr] Command string.
**                    The string may end with a trailing pipe character.
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajFileNewInPipe(const AjPStr command)
{
    AjPFile thys = NULL;
    
    thys = ajSysCreateInpipeS(command);
    
    if(!thys)
        return NULL;
    
    thys->Handle = ++fileHandle;
    ajStrAssignS(&thys->Name, command);
    thys->End = ajFalse;
    
    fileOpenCnt++;
    fileOpenTot++;
    if(fileOpenCnt > fileOpenMax)
	fileOpenMax = fileOpenCnt;
    
   return thys;
}




/* @func ajFileNewListinDirPre ************************************************
**
** Opens directory "dir"
** Looks for file "file" with the extension (if any) specified
** for the directory
**
** @param [r] dir [const AjPDir] Directory
** @param [r] prefix [const AjPStr] Wildcard Filename.
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajFileNewListinDirPre(const AjPDir dir, const AjPStr prefix)
{
    if(ajStrGetLen(dir->Name))
	ajStrAssignS(&fileNameFix, dir->Name);
    else
	ajStrAssignC(&fileNameFix, CURRENT_DIR);
    
    if(ajStrGetCharLast(fileNameFix) != SLASH_CHAR)
	ajStrAppendC(&fileNameFix, SLASH_STRING);
    
    ajStrAppendS(&fileNameFix, prefix);
    ajFilenameReplaceExtS(&fileNameFix, dir->Extension);

    return ajFileNewInNameS(fileNameFix);
}




/* @func ajFileNewListinList **************************************************
**
** Creates a new file object with a list of input file names.
**
** @param [u] list [AjPList] List of input filenames as strings.
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajFileNewListinList(AjPList list)
{
    AjPFile thys;

    AJNEW0(thys);

    thys->List = list;
    thys->Name = NULL;
    ajListstrTrace(thys->List);
    ajListstrPop(thys->List, &thys->Name);
    ajDebug("ajFileNewListinList pop '%S'\n", thys->Name);
    ajListstrTrace(thys->List);
    ajNamResolve(&thys->Name);
    thys->fp = fopen(ajStrGetPtr(thys->Name), "rb");

    if(!thys->fp)
    {
	ajDebug("ajFileNewListinList fopen failed\n");
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




/* @obsolete ajFileNewInList
** @rename ajFileNewListinList
*/

__deprecated AjPFile ajFileNewInList(AjPList list)
{
    return ajFileNewListinList(list);
}




/* @func ajFileNewListinNameDirS **********************************************
**
** Opens directory "dir" and looks for file "filename"
**
** @param [r] name [const AjPStr] Wildcard Filename.
** @param [r] dir [const AjPDir] Directory
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajFileNewListinNameDirS(const AjPStr name, const AjPDir dir)
{
    if(ajStrGetLen(dir->Name))
	ajStrAssignS(&fileNameFix, dir->Name);
    else
	ajStrAssignC(&fileNameFix, CURRENT_DIR);
    
    if(ajStrGetCharLast(fileNameFix) != SLASH_CHAR)
	ajStrAppendC(&fileNameFix, SLASH_STRING);
    
    ajStrAppendS(&fileNameFix, name);

    return ajFileNewInNameS(fileNameFix);
}




/* @obsolete ajFileNewDirF
** @rename ajFileNewListinDirPre
*/
__deprecated AjPFile ajFileNewDirF(const AjPDir dir, const AjPStr filename)
{
    return ajFileNewListinDirPre(dir,filename);
}




/* @func ajFileNewListinPathWild **********************************************
**
** Opens directory "dir"
** Looks for file(s) matching "file"
** Opens them as a list of files using a simple file object.
**
** @param [r] path [const AjPStr] Directory
** @param [r] wildname [const AjPStr] Wildcard filename.
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajFileNewListinPathWild(const AjPStr path, const AjPStr wildname)
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
    
    if(ajStrGetLen(path))
	ajStrAssignS(&fileDirfixTmp, path);
    else
	ajStrAssignC(&fileDirfixTmp, CURRENT_DIR);
    
    if(ajStrGetCharLast(fileDirfixTmp) != SLASH_CHAR)
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

	if(!ajCharMatchWildS(de->d_name, wildname))
	    continue;

	dirsize++;
	ajDebug("accept '%s'\n", de->d_name);
	name = NULL;
	ajFmtPrintS(&name, "%S%s", fileDirfixTmp, de->d_name);
	ajListstrPushAppend(list, name);
    }
    
    closedir(dp);
    ajDebug("%d files for '%S' '%S'\n", dirsize, path, wildname);
    
    return ajFileNewListinList(list);
}




/* @obsolete ajFileNewDW
** @rename ajFileNewListinPathWild
*/
    
__deprecated AjPFile ajFileNewDW(const AjPStr dir, const AjPStr wildfile)
{
    return ajFileNewListinPathWild(dir, wildfile);
}




/* @func ajFileNewListinPathWildExclude ***************************************
**
** Opens directory "dir"
** Looks for file(s) matching "file"
** Skip files matching excluded files wildcard
** Opens them as a list of files using a simple file object.
**
** @param [r] path [const AjPStr] Directory path
** @param [r] wildname [const AjPStr] Wildcard filename.
** @param [r] exclude [const AjPStr] Wildcard excluded filename.
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajFileNewListinPathWildExclude(const AjPStr path,
                                       const AjPStr wildname,
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
    
    if(ajStrGetLen(path))
	ajStrAssignS(&fileDirfixTmp, path);
    else
	ajStrAssignC(&fileDirfixTmp, CURRENT_DIR);
    
    if(ajStrGetCharLast(fileDirfixTmp) != SLASH_CHAR)
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

	if(!ajFilenameTestExclude(fileNameTmp, exclude, wildname))
	    continue;

	dirsize++;
	ajDebug("accept '%s'\n", de->d_name);
	name = NULL;
	ajFmtPrintS(&name, "%S%s", fileDirfixTmp, de->d_name);
	ajListstrPushAppend(list, name);
    }
    
    closedir(dp);
    ajDebug("%d files for '%S' '%S'\n", dirsize, path, wildname);
    
    return ajFileNewListinList(list);
}




/* @obsolete ajFileNewDWE
** @rename ajFileNewListinPathWildExclude
*/
__deprecated AjPFile ajFileNewDWE(const AjPStr dir, const AjPStr wildfile,
			     const AjPStr exclude)
{
    return ajFileNewListinPathWildExclude(dir, wildfile, exclude);
}




/* @func ajFileNewOutNameC *****************************************************
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

AjPFile ajFileNewOutNameC(const char* name)
{
    ajStrAssignC(&fileNameStrTmp, name);

    return ajFileNewOutNameS(fileNameStrTmp);
}




/* @func ajFileNewOutNameS *****************************************************
**
** Creates a new output file object with a specified name.
**
** 'stdout' and 'stderr' are special names for standard output and
** standard error respectively.
**
** @param [r] name [const AjPStr] File name.
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajFileNewOutNameS(const AjPStr name)
{
    AjPFile thys;

    if(ajStrMatchC(name, "stdout"))
    {
	thys = ajFileNewFromCfile(stdout);
	ajStrAssignC(&thys->Name, "stdout");
	return thys;
    }

    if(ajStrMatchC(name, "stderr"))
    {
	thys = ajFileNewFromCfile(stderr);
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




/* @obsolete ajFileNewOut
** @rename ajFileNewOutNameS
*/
__deprecated AjPFile ajFileNewOut(const AjPStr name)
{
    return ajFileNewOutNameS(name);
}




/* @func ajFileNewOutNameDirS *************************************************
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
** @param [r] name [const AjPStr] File name.
** @param [rN] dir [const AjPDirout] Directory
**                                  (optional, can be empty or NULL).
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajFileNewOutNameDirS(const AjPStr name, const AjPDirout dir)
{
    AjPFile thys;

    ajDebug("ajFileNewOutNameDirS('%S' '%S')\n", dir->Name, name);

    AJNEW0(thys);

    if(!dir)
    {
	thys->fp = fopen(ajStrGetPtr(name), "wb");
	ajDebug("ajFileNewOutNameDirS open name '%S'\n", name);
    }
    else
    {
	if(ajFilenameHasPath(name))
	    ajStrAssignS(&fileDirfixTmp, name);
	else
	{
	    ajStrAssignS(&fileDirfixTmp, dir->Name);

	    if(ajStrGetCharLast(dir->Name) != SLASH_CHAR)
		ajStrAppendC(&fileDirfixTmp, SLASH_STRING);

	    ajStrAppendS(&fileDirfixTmp, name);
	}

	ajFilenameSetExtS(&fileDirfixTmp, dir->Extension);

	thys->fp = fopen(ajStrGetPtr(fileDirfixTmp), "wb");
	ajDebug("ajFileNewOutNameDirS open dirfix '%S'\n", fileDirfixTmp);
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




/* @obsolete ajFileNewOutDir
** @replace ajFileNewOutNameDirS (1,2/2,1)
*/
__deprecated AjPFile ajFileNewOutDir(const AjPDirout dir, const AjPStr name)
{
    return ajFileNewOutNameDirS(name, dir);
}




/* @func ajFileNewOutNamePathS *************************************************
**
** Creates a new output file object with a specified directory and name.
**
** 'stdout' and 'stderr' are special names for standard output and
** standard error respectively.
**
** If the filename already has a directory specified,
** the "dir" argument is ignored.
**
** @param [r] name [const AjPStr] File name.
** @param [rN] path [const AjPStr] Directory (optional, can be empty or NULL).
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajFileNewOutNamePathS(const AjPStr name, const AjPStr path)
{
    AjPFile thys;

    ajDebug("ajFileNewOutNamePathS('%S' '%S')\n", path, name);

    if(ajStrMatchC(name, "stdout"))
	return ajFileNewFromCfile(stdout);

    if(ajStrMatchC(name, "stderr"))
	return ajFileNewFromCfile(stderr);

    AJNEW0(thys);

    if(!ajStrGetLen(path))
    {
	thys->fp = fopen(ajStrGetPtr(name), "wb");
	ajDebug("ajFileNewOutNamePathS open name '%S'\n", name);
    }
    else
    {
	if(ajFilenameHasPath(name))
	    ajStrAssignS(&fileDirfixTmp, name);
	else
	{
	    ajStrAssignS(&fileDirfixTmp, path);

	    if(ajStrGetCharLast(path) != SLASH_CHAR)
		ajStrAppendC(&fileDirfixTmp, SLASH_STRING);

	    ajStrAppendS(&fileDirfixTmp, name);
	}

	thys->fp = fopen(ajStrGetPtr(fileDirfixTmp), "wb");
	ajDebug("ajFileNewOutNamePathS open dirfix '%S'\n", fileDirfixTmp);
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




/* @obsolete ajFileNewOutD
** @replace ajFileNewOutNamePathS (1,2/2,1)
*/
__deprecated AjPFile ajFileNewOutD(const AjPStr dir, const AjPStr name)
{
    return ajFileNewOutNamePathS(name, dir);
}




/* @func ajFileNewOutappendNameS ***********************************************
**
** Creates an output file object with a specified name.
** The file is opened for append so it either appends to an existing file
** or opens a new one.
**
** @param [r] name [const AjPStr] File name.
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajFileNewOutappendNameS(const AjPStr name)
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




/* @obsolete ajFileNewApp
** @rename ajFileNewOutappendNameS
*/
__deprecated AjPFile ajFileNewApp(const AjPStr name)
{

    return ajFileNewOutappendNameS(name);
}




/* @section File Destructors **************************************************
**
** @fdata [AjPFile]
**
** Destruction is achieved by closing the file.
**
** Unlike ANSI C, there are tests to ensure a file is not closed twice.
**
** @nam3rule Close Close file and destroy object
**
** @argrule Close Pfile [AjPFile*] File to be closed and deleted
**
** @valrule Close [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajFileClose **********************************************************
**
** Close and free a file object.
**
** @param [d] Pfile [AjPFile*] File.
** @return [void]
** @@
******************************************************************************/

void ajFileClose(AjPFile* Pfile)
{
    AjPFile thys;

    thys = Pfile ? *Pfile : 0;

    if(!Pfile)
	return;

    if(!*Pfile)
	return;

    fileClose(thys);
    AJFREE(*Pfile);

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
    int sleepcount = 0;
    int status = 0;
    int maxsleep = 60;
#ifndef WIN32
    pid_t retval;
#endif

    if(!thys)
	return;

#ifndef WIN32

    /*
    ** Only wait for the PID to close if we have read everything.
    ** Otherwise ... we will wait for ever as it will not
    ** go away until its output is read or closed
    */

    if (thys->Pid && thys->End)
    {
	while((retval=waitpid(thys->Pid,&status,WNOHANG))!= thys->Pid)
	{
            if(sleepcount > maxsleep)
                break;

            sleepcount++;
	    sleep(1);

	    /*ajDebug("fileClose waitpid returns %d status %d\n",
		    retval, status);*/

	    if(retval == -1)
		if(errno != EINTR)
		    break;

	    status = 0;
	}
    }
#else
    if(thys->Process && thys->End)
    {
        WaitForSingleObject(thys->Process,INFINITE);
        CloseHandle(thys->Process);
        CloseHandle(thys->Thread);
    }
    
#endif

    if(thys->Handle)
    {
	if(thys->fp)
	{
	    if(thys->fp == stdout)
		fileUsedStdout = ajFalse;
	    else if(thys->fp == stderr)
		fileUsedStderr = ajFalse;
	    else if(thys->fp == stdin)
		fileUsedStdin = ajFalse;
	    else
	    {
                if(fclose(thys->fp))
		    ajFatal("File close problem in fileClose error:%d '%s'",
                            errno, strerror(errno));
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
    AJFREE(thys->Workbuffer);
    AJFREE(thys->Readblock);

    return;
}




/* @section file reopen *******************************************************
**
** @fdata [AjPFile]
**
** These functions close a file and open a new file with the same file object
**
** @nam3rule Reopen Reopen an existing input file object
** @nam4rule Name Reopen using a new filename
** @nam4rule Next Reopen using the next filename in the input file list
**
** @argrule Reopen file [AjPFile] File object
** @argrule Name name [const AjPStr] Filename to open
**
** @valrule * [AjBool] True on success
**
** @fcategory modify
**
******************************************************************************/




/* @func ajFileReopenName *****************************************************
**
** Reopens an input file with a new name
**
** @param [u] file [AjPFile] Input file.
** @param [r] name [const AjPStr] name of file.
** @return [AjBool] True on success
** @@
******************************************************************************/

AjBool ajFileReopenName(AjPFile file, const AjPStr name)
{
    ajStrAssignS(&file->Name, name);

    if(!freopen(MAJSTRGETPTR(file->Name), "rb", file->fp))
        return ajFalse;

    return ajTrue;
}




/* @obsolete ajFileReopen
** @rename ajFileReopenName
*/
__deprecated FILE* ajFileReopen(AjPFile thys, const AjPStr name)
{
    ajFileReopenName(thys, name);
    
    return thys->fp;
}




/* @func ajFileReopenNext *****************************************************
**
** Given a file object that includes a list of input files, closes the
** current input file and opens the next one.
**
** @param [u] file [AjPFile] File object.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajFileReopenNext(AjPFile file)
{
    static AjPStr name = NULL;

    if(!file->List)
    {
	ajDebug("ajFileReopenNext for non-list file %F\n", file);
	return ajFalse;
    }

    ajDebug("ajFileReopenNext for non-list file %F name '%S'\n",
            file, file->Name);

    /*ajListTrace(file->List);*/
    if(!ajListPop(file->List, (void*) &name))
    {
	/* end of list */
	ajDebug("ajFileReopenNext failed - list completed\n");

	return ajFalse;
    }

    ajDebug("ajFileReopenNext filename '%S'\n", name);

    if(!ajFileReopenName(file, name))
    {
	/* popped from the list */
	ajStrDel(&name);

	return ajFalse;
    }

    /* popped from the list */
    ajStrDel(&name);
    file->End = ajFalse;

    ajDebug("ajFileNext success\n");

    return ajTrue;
}




/* @obsolete ajFileNext
** @rename ajFileReopenNext
*/
__deprecated AjBool ajFileNext(AjPFile thys)
{
    return ajFileReopenNext(thys);
}




/* @section File modifiers
**
** @fdata [AjPFile]
**
** @nam3rule Fix Reset all file attribute that may have been changed
**                 by non-AJAX calls
** @nam3rule Reset Reset a file attribute that may have been changed
**                 by non-AJAX calls
** @nam3rule Seek Call system file seek function
** @nam3rule Set Set a file attribute
** @nam4rule ResetEof Reset the end of file value using the system
**                    'feof' function
** @nam4rule ResetPos Reset the file position using the system
**                    'ftell' function
** @nam4rule SetUnbuffer Set file unbuffered
**
** @argrule * file [AjPFile] File object
** @argrule Seek offset [ajlong] File offset to pass to system seek call
** @argrule Seek wherefrom [ajint] File wherefrom value to pass to
**                                 system seek call
**
** @valrule Seek [ajint] Return value from seek
** @valrule SetUnbuffer [void]
** @valrule Fix [AjBool] End of file reached
** @valrule Eof [AjBool] End of file reached
** @valrule Pos [ajlong] File position from ftell
**
** @fcategory modify
**
******************************************************************************/




/* @func ajFileFix *********************************************************
**
** Resets internal file attribute after non-AJAX operations.
**
** @param [u] file [AjPFile] File.
** @return [AjBool] True if end of file is reached
** @@
******************************************************************************/

AjBool ajFileFix(AjPFile file)
{
    ajFileResetPos(file);

    return ajFileResetEof(file);
}




/* @func ajFileResetEof ******************************************************
**
** Resets the end of file attribute after non-AJAX operations.
**
** @param [u] file [AjPFile] File.
** @return [AjBool] True if end of file is set
** @@
******************************************************************************/

AjBool ajFileResetEof(AjPFile file)
{
    if(feof(file->fp))
        file->End = ajTrue;
    else
        file->End = ajFalse;

    return file->End;
}




/* @func ajFileResetPos ******************************************************
**
** Resets and returns the current position in an open file.
**
** @param [u] file [AjPFile] File.
** @return [ajlong] Result of 'ftell'
** @@
******************************************************************************/

ajlong ajFileResetPos(AjPFile file)
{
    if(!file->fp)
	return 0;

    file->Filepos = ftell(file->fp);

    return file->Filepos;
}




/* @obsolete ajFileTell
** @rename ajFileResetPos
*/
__deprecated ajlong ajFileTell(AjPFile file)
{
    return ajFileResetPos(file);
}




/* @func ajFileSeek ***********************************************************
**
** Sets the current position in an open file.
**
** Resets the end-of-file flag End for cases where end-of-file was
** reached and then we seek back somewhere in the file.
**
** @param [u] file [AjPFile] File.
** @param [r] offset [ajlong] Offset
** @param [r] wherefrom [ajint] Start of offset, as defined for 'fseek'.
** @return [ajint] Result of 'fseek'
** @@
******************************************************************************/

ajint ajFileSeek(AjPFile file, ajlong offset, ajint wherefrom)
{
    ajint ret;

    clearerr(file->fp);
    ret = fseek(file->fp, offset, wherefrom);

    if(feof(file->fp))
    {
	file->End = ajTrue;
	ajDebug("EOF ajFileSeek file %F\n", file);
    }
    else
	file->End = ajFalse;

    return ret;
}




/* @func ajFileSetUnbuffer ****************************************************
**
** Turns off system buffering of an output file, for example to allow
** debug output to appear even in the event of a program abort.
**
** @param [u] file [AjPFile] File object.
** @return [void]
** @@
******************************************************************************/

void ajFileSetUnbuffer(AjPFile file)
{
    setbuf(file->fp, NULL);

    return;
}




/* @obsolete ajFileUnbuffer
** @rename ajFileSetUnbuffer
*/
__deprecated void ajFileUnbuffer(AjPFile thys)
{
    ajFileSetUnbuffer(thys);

    return;
}




/* @section file casts
**
** @fdata [AjPFile]
**
** Returns attributes of a file object
**
** @nam3rule Get Return attribute of a file object
** @nam4rule GetFileptr Return C file pointer
** @nam4rule GetName Return filename
** @nam3rule Is Return true if attribute is set
** @nam4rule IsAppend Test file is open for appending output
** @nam4rule IsEof Test end of file has been reached
** @nam4rule IsFile Test file is a regular file, not a pipe or terminal
** @nam4rule IsStderr Test file is writing to standard error
** @nam4rule IsStdin test file is reading from standard input
** @nam4rule IsStdout test file is writing to standard output
**
** @suffix C Return a C character string
** @suffix S return a string object
**
** @argrule * file [const AjPFile] File object
**
** @valrule C [const char*] C character string
** @valrule S [const AjPStr] String object
** @valrule Is [AjBool] True on success
** @valrule GetFileptr [FILE*] C file pointer
**
** @fcategory use
**
******************************************************************************/




/* @func ajFileGetFileptr *****************************************************
**
** Returns the C file pointer for an open file.
**
** Warning: Using the C file pointer will make internals of the file
** object invalid. The file position can be reset with ajFileResetPos.
**
** @param [r] file [const AjPFile] File.
** @return [FILE*] C file pointer for the file.
** @@
******************************************************************************/

FILE* ajFileGetFileptr(const AjPFile file)
{
    if(!file)
        return NULL;

    return file->fp;
}




/* @obsolete ajFileFp
** @rename ajFileGetFileptr
*/
__deprecated FILE* ajFileFp(const AjPFile thys)
{
    return ajFileGetFileptr(thys);
}




/* @func ajFileGetNameC ******************************************************
**
** Returns the file name for a file object. The filename returned is a pointer
** to the real string internally, so the user must take care not to change
** it and cannot trust the value if the file object is deleted.
**
** @param [r] file [const AjPFile] File.
** @return [const char*] Filename as a C character string.
** @@
******************************************************************************/

const char* ajFileGetNameC(const AjPFile file)
{
    if(!file)
        return "";

    return ajStrGetPtr(file->Name);
}




/* @obsolete ajFileName
** @rename ajFileGetNameC
*/
__deprecated const char* ajFileName(const AjPFile file)
{
    return ajFileGetNameC(file);
}




/* @func ajFileGetNameS *******************************************************
**
** Returns the file name for a file object. The filename returned is a pointer
** to the real string internally, so the user must take care not to change
** it and cannot trust the value if the file object is deleted.
**
** @param [r] file [const AjPFile] File.
** @return [const AjPStr] Filename as a C character string.
** @@
******************************************************************************/

const AjPStr ajFileGetNameS(const AjPFile file)
{
    if(!file)
        return NULL;

    return file->Name;
}




/* @obsolete ajFileNameS
** @rename ajFileGetNameS
*/
__deprecated const AjPStr ajFileNameS(const AjPFile file)
{
    return ajFileGetNameS(file);
}




/* @obsolete ajFileGetName
** @rename ajFileGetNameS
*/
__deprecated const AjPStr ajFileGetName(const AjPFile file)
{
    return ajFileGetNameS(file);
}




/* @func ajFileIsAppend *******************************************************
**
** Returns the App element for a file object. The App element is True if the 
** file was opened for appending to, False otherwise. 
**
** @param [r] file [const AjPFile] File.
** @return [AjBool] App element, True if if file was opened for appending to, 
** False otherwise. 
** @@
******************************************************************************/

AjBool ajFileIsAppend(const AjPFile file)
{
    if(!file)
        return ajFalse;

    return file->App;
}




/* @obsolete ajFileGetApp
** @rename ajFileIsAppend
*/
__deprecated AjBool ajFileGetApp(const AjPFile thys)
{
    return ajFileIsAppend(thys);
}




/* @func ajFileIsEof **********************************************************
**
** Tests whether we have reached end of file already
**
** @param [r] file [const AjPFile] File
** @return [AjBool] ajTrue if we already set end-of-file
** @@
******************************************************************************/

AjBool ajFileIsEof(const AjPFile file)
{
    if(!file)
        return ajTrue;

    return file->End;
}




/* @obsolete ajFileEof
** @rename ajFileIsEof
*/
__deprecated AjBool ajFileEof(const AjPFile thys)
{
    return ajFileIsEof(thys);
}




/* @func ajFileIsFile *********************************************************
**
** Tests whether a file object is really a regular file.
**
** Used to test for character devices, for example standard input from
** a terminal.
**
** @param [r] file [const AjPFile] File object.
** @return [AjBool] ajTrue if the file matches stderr.
** @@
******************************************************************************/

AjBool ajFileIsFile(const AjPFile file)
{
#if defined(AJ_IRIXLF)
    struct stat64 buf;
#else
    struct stat buf;
#endif

#if defined(AJ_IRIXLF)
    if(!fstat64(fileno(file->fp), &buf))
#else
    if(!fstat(fileno(file->fp), &buf))
#endif

    if((ajuint)buf.st_mode & AJ_FILE_R)
        return ajTrue;

    return ajFalse;
}




/* @func ajFileIsStderr *******************************************************
**
** Tests whether a file object is really stderr.
**
** @param [r] file [const AjPFile] File object.
** @return [AjBool] ajTrue if the file matches stderr.
** @@
******************************************************************************/

AjBool ajFileIsStderr(const AjPFile file)
{
    if(!file)
        return ajFalse;

    if(file->fp == stderr)
	return ajTrue;

    return ajFalse;
}




/* @obsolete ajFileStderr
** @rename ajFileIsStderr
*/
__deprecated AjBool ajFileStderr(const AjPFile file)
{
    return ajFileIsStderr(file);
}




/* @func ajFileIsStdin ********************************************************
**
** Tests whether a file object is really stdin.
**
** @param [r] file [const AjPFile] File object.
** @return [AjBool] ajTrue if the file matches stdin.
** @@
******************************************************************************/

AjBool ajFileIsStdin(const AjPFile file)
{
    if(!file)
        return ajFalse;

    if(file->fp == stdin)
	return ajTrue;

    return ajFalse;
}




/* @obsolete ajFileStdin
** @rename ajFileIsStdin
*/
__deprecated AjBool ajFileStdin(const AjPFile file)
{
    return ajFileIsStdin(file);
}




/* @func ajFileIsStdout *******************************************************
**
** Tests whether a file object is really stdout.
**
** @param [r] file [const AjPFile] File object.
** @return [AjBool] ajTrue if the file matches stdout.
** @@
******************************************************************************/

AjBool ajFileIsStdout(const AjPFile file)
{
    if(!file)
        return ajFalse;

    if(file->fp == stdout)
	return ajTrue;

    return ajFalse;
}




/* @obsolete ajFileStdout
** @rename ajFileIsStdout
*/
__deprecated AjBool ajFileStdout(const AjPFile file)
{
    return ajFileIsStdout(file);
}





/* @section file debug
**
** @fdata [AjPFile]
**
** report file object contents for debugging
**
**
** @nam3rule Trace    Print report to debug file (if any)
**
** @argrule * file [const AjPFile]
**
** @valrule * [void]
**
** @fcategory misc
******************************************************************************/




/* @func ajFileTrace **********************************************************
**
** Writes debug messages to trace the contents of a file object.
**
** @param [r] file [const AjPFile] File.
** @return [void]
** @@
******************************************************************************/

void ajFileTrace(const AjPFile file)
{
    ajuint i;
    ajuint j;
    AjIList iter;

    ajDebug("File: '%S'\n", file->Name);
    ajDebug("  Handle:  %d\n", file->Handle);
    ajDebug("  End:  %B\n", file->End);
    ajDebug("  Append:  %B\n", file->App);
    ajDebug("  Filepos:  %ld\n", file->Filepos);
#ifndef WIN32
    ajDebug("  PID:  %d\n", file->Pid);
#endif
    ajDebug(" feof:  %d\n", feof(file->fp));
    ajDebug("ftell:  %ld\n", ftell(file->fp));
    i =  ajListGetLength(file->List);
    ajDebug("  List:  %u\n", i);

    if(i)
    {
        j = 0;
        iter = ajListIterNewread(file->List);

        while (!ajListIterDone(iter))
            ajDebug("    %3d: '%S'\n", ++j, ajListstrIterGet(iter));
    }

    return;
}




/* @section file exit
**
** @fdata [AjPFile]
**
** Cleanup memory on program exit
**
** @fnote     general exit functions, no arguments
**
** @nam3rule Exit Cleanup and report on exit
**
** @valrule * [void]
**
** @fcategory misc
**
******************************************************************************/




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
    ajStrDel(&fileNameTmp);
    ajStrDel(&fileNameStrTmp);
    ajStrDel(&fileDirfixTmp);
    ajStrDel(&fileCwd);
    ajStrDel(&fileTmpStr);
    ajStrDel(&fileTempFilename);
    ajStrDel(&fileDirectory);

    ajRegFree(&fileUserExp);
    ajRegFree(&fileWildExp);
    ajRegFree(&fileEntryExp);
    ajRegFree(&fileFileExp);
    ajRegFree(&fileRestExp);
    ajRegFree(&fileDirExp);
    ajRegFree(&fileFilenameExp);

    return;
}




/* @datasection [AjPFilebuff] Buffered file object ****************************
**
** Function is for manipulating buffered files and returns or takes at least 
** one AjSFileBuff argument.
** 
** @nam2rule Filebuff   
**
*/




/* @section Buffered File Constructors ****************************************
**
** @fdata [AjPFilebuff]
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
** @nam3rule New Constructor
** @nam4rule From Create from an already open file
** @nam4rule Nofile Create without an input file for use as a text buffer
** @nam5rule FromFile AjPfile object used to create buffered file object
** @nam5rule FromCfile C FILE* structure used to create buffered file object
** @nam4rule Line Create buffer with line of text
** @nam4rule Listin List of one or more input files
** @nam5rule ListinList List of files specified
**
** @suffix Name Existing filename specified
** @suffix C Filename as C character string
** @suffix S Filename as string
** @suffix Path Input directory path specified
** @suffix Wild Wildcard filename
** @suffix Exclude Filename exclusion wildcard(s)
**
** @argrule Cfile file [FILE*] C file pointer
** @argrule ListinList list [AjPList] List of filenames as strings
** @argrule C name [const char*] Filename
** @argrule S name [const AjPStr] Filename
** @argrule Line line [const AjPStr] First line of new buffer
** @argrule FromFile file [AjPFile] Input file object
** @argrule Path path [const AjPStr] Input directory path
** @argrule Wild wildname [const AjPStr] Wildcard filename
** @argrule Exclude exclude [const AjPStr] Filename exclusion wildcard(s)
**
** @valrule * [AjPFilebuff] New buffered file object
**
** @fcategory new
**
******************************************************************************/




/* @func ajFilebuffNewFromCfile ************************************************
**
** Creates a new buffered input file from an already open C file.
**
** @param [u] file [FILE*] Open C file.
** @return [AjPFilebuff] New buffered file object.
** @@
******************************************************************************/

AjPFilebuff ajFilebuffNewFromCfile(FILE* file)
{
    AjPFile tmpcfile;

    tmpcfile = ajFileNewFromCfile(file);

    if(!tmpcfile)
	return NULL;

    return ajFilebuffNewFromFile(tmpcfile);
}




/* @obsolete ajFileBuffNewF
** @rename ajFilebuffNewFromCfile
*/
__deprecated AjPFilebuff ajFileBuffNewF(FILE* fp)
{
    return ajFilebuffNewFromCfile(fp);
}




/* @func ajFilebuffNewFromFile *************************************************
**
** Creates a new buffered input file object from an open file.
**
** @param [u] file [AjPFile] File object to be buffered.
** @return [AjPFilebuff] New buffered file object.
** @@
******************************************************************************/

AjPFilebuff ajFilebuffNewFromFile(AjPFile file)
{
    AjPFilebuff thys;

    if(!file)
	return NULL;

    AJNEW0(thys);
    thys->File = file;

    thys->Last = thys->Curr = thys->Prev = thys->Lines =  NULL;
    thys->Freelines = thys->Freelast = NULL;
    thys->Pos = thys->Size = 0;

    return thys;
}




/* @obsolete ajFileBuffNewFile
** @rename ajFilebuffNewFromFile
*/
__deprecated AjPFilebuff ajFileBuffNewFile(AjPFile file)
{
    return ajFilebuffNewFromFile(file);
}




/* @func ajFilebuffNewLine *****************************************************
**
** Creates a new buffered input file object with no file but with
** one line of buffered data provided.
**
** @param [r] line [const AjPStr] One line of buffered data.
** @return [AjPFilebuff] New buffered file object.
** @@
******************************************************************************/

AjPFilebuff ajFilebuffNewLine(const AjPStr line)
{
    AjPFilebuff thys;
    AjPFile file;

    file = fileNew();
    file->End = ajTrue;
    ajDebug("EOF ajFileBuffNewLine file <none>\n");

    thys = ajFilebuffNewFromFile(file);

    thys->Lines = AJNEW0(thys->Last);
    ajStrAssignS(&thys->Last->Line,line);

    thys->Curr = thys->Lines;
    thys->Pos  = 0;
    thys->Size = 1;

    return thys;
}




/* @obsolete ajFileBuffNewS
** @rename ajFilebuffNewLine
*/
__deprecated AjPFilebuff ajFileBuffNewS(const AjPStr data)
{
    return ajFilebuffNewLine(data);
}




/* @func ajFilebuffNewListinList **********************************************
**
** Creates a new buffered file object from a list of filenames.
**
** @param [u] list [AjPList] List of filenames as strings.
** @return [AjPFilebuff] New buffered file object.
** @@
******************************************************************************/

AjPFilebuff ajFilebuffNewListinList(AjPList list)
{
    AjPFile file;

    file = ajFileNewListinList(list);

    if(!file)
	return NULL;

    return ajFilebuffNewFromFile(file);
}




/* @obsolete ajFileBuffNewList
** @rename ajFilebuffNewListinList
*/

__deprecated AjPFilebuff ajFileBuffNewList(AjPList list)
{
    return ajFilebuffNewListinList(list);
}




/* @func ajFilebuffNewNameS ****************************************************
**
** Creates a new buffered input file object with an opened named file.
**
** @param [r] name [const AjPStr] File name.
** @return [AjPFilebuff] New buffered file object.
** @@
******************************************************************************/

AjPFilebuff ajFilebuffNewNameS(const AjPStr name)
{
    AjPFile file;

    file = ajFileNewInNameS(name);

    return ajFilebuffNewFromFile(file);
}




/* @obsolete ajFileBuffNewIn
** @rename ajFilebuffNewNameS
*/
__deprecated AjPFilebuff ajFileBuffNewIn(const AjPStr name)
{
    return ajFilebuffNewNameS(name);
}




/* @func ajFilebuffNewNamePathC ************************************************
**
** Opens directory "dir", finds and opens file "name"
**
** @param [r] name [const char*] Filename.
** @param [r] path [const AjPStr] Directory. If empty uses current directory.
** @return [AjPFilebuff] New buffered file object.
** @@
******************************************************************************/

AjPFilebuff ajFilebuffNewNamePathC(const char* name, const AjPStr path)
{
    if(ajStrGetLen(path))
	ajStrAssignS(&fileNameFix, path);
    else
	ajStrAssignC(&fileNameFix, CURRENT_DIR);

    if(ajStrGetCharLast(fileNameFix) != SLASH_CHAR)
	ajStrAppendC(&fileNameFix, SLASH_STRING);

    ajStrAppendC(&fileNameFix, name);

    return ajFilebuffNewNameS(fileNameFix);
}




/* @obsolete ajFileBuffNewDC
** @replace ajFileBuffNewNamePathC (1,2/2,1)
*/
__deprecated AjPFilebuff ajFileBuffNewDC(const AjPStr dir,
                                         const char* filename)
{
    return ajFilebuffNewNamePathC(filename, dir);
}




/* @func ajFilebuffNewNamePathS ************************************************
**
** Opens directory "dir", finds and opens file "name"
**
** @param [r] name [const AjPStr] Filename.
** @param [r] path [const AjPStr] Directory. If empty uses current directory.
** @return [AjPFilebuff] New buffered file object.
** @@
******************************************************************************/

AjPFilebuff ajFilebuffNewNamePathS(const AjPStr name, const AjPStr path)
{
    if(ajStrGetLen(path))
	ajStrAssignS(&fileNameFix, path);
    else
	ajStrAssignC(&fileNameFix, CURRENT_DIR);

    if(ajStrGetCharLast(fileNameFix) != SLASH_CHAR)
	ajStrAppendC(&fileNameFix, SLASH_STRING);

    ajStrAppendS(&fileNameFix, name);

    return ajFilebuffNewNameS(fileNameFix);
}




/* @obsolete ajFileBuffNewDF
** @replace ajFilebuffNewNamePathS (1,2/2,1)
*/
__deprecated AjPFilebuff ajFileBuffNewDF(const AjPStr dir,
                                         const AjPStr filename)
{
    return ajFilebuffNewNamePathS(filename, dir);
}




/* @func ajFilebuffNewNofile ************************************************
**
** Creates a new buffered input file object with an undefined file
** to be used as a text buffer.
**
** @return [AjPFilebuff] New buffered file object.
** @@
******************************************************************************/

AjPFilebuff ajFilebuffNewNofile(void)
{
    AjPFile file;

    file = fileNew();           /* dummy file */

    return ajFilebuffNewFromFile(file);
}




/* @obsolete ajFileBuffNew
** @rename ajFilebuffNewNofile
*/
__deprecated AjPFilebuff ajFileBuffNew(void)
{
    return ajFilebuffNewNofile();
}




/* @func ajFilebuffNewPathWild *****************************************
**
** Opens directory "dir"
** Looks for file(s) matching "file"
** Opens them as a list of files using a buffered file object.
**
** @param [r] path [const AjPStr] Directory
** @param [r] wildname [const AjPStr] Wildcard filename.
** @return [AjPFilebuff] New buffered file object.
** @@
******************************************************************************/

AjPFilebuff ajFilebuffNewPathWild(const AjPStr path,
                                  const AjPStr wildname)
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
    
    if(ajStrGetLen(path))
	ajStrAssignS(&fileDirfixTmp, path);
    else
	ajStrAssignC(&fileDirfixTmp, CURRENT_DIR);
    
    if(ajStrGetCharLast(fileDirfixTmp) != SLASH_CHAR)
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

	if(!ajCharMatchWildS(de->d_name, wildname))
	    continue;

	dirsize++;
	ajDebug("accept '%s'\n", de->d_name);
	name = NULL;
	ajFmtPrintS(&name, "%S%s", fileDirfixTmp, de->d_name);
	ajListstrPushAppend(list, name);
    }
    
    closedir(dp);
    ajDebug("%d files for '%S' '%S'\n", dirsize, path, wildname);
    
    return ajFilebuffNewListinList(list);
}




/* @obsolete ajFileBuffNewDW
** @rename ajFilebuffNewPathWild
*/

__deprecated AjPFilebuff ajFileBuffNewDW(const AjPStr dir,
                                         const AjPStr wildfile)
{
    return ajFilebuffNewPathWild(dir, wildfile);
}




/* @func ajFilebuffNewPathWildExclude ***********************************
**
** Opens directory "dir"
** Looks for file(s) matching "file"
** Skip files matching excluded files wildcard
** Opens them as a list of files using a buffered file object.
**
** @param [r] path [const AjPStr] Directory
** @param [r] wildname [const AjPStr] Wildcard filename.
** @param [r] exclude [const AjPStr] Wildcard excluded filename.
** @return [AjPFilebuff] New buffered file object.
** @@
******************************************************************************/

AjPFilebuff ajFilebuffNewPathWildExclude(const AjPStr path,
                                         const AjPStr wildname,
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
    
    if(ajStrGetLen(path))
	ajStrAssignS(&fileDirfixTmp, path);
    else
	ajStrAssignC(&fileDirfixTmp, CURRENT_DIR);
    
    if(ajStrGetCharLast(fileDirfixTmp) != SLASH_CHAR)
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

	if(!ajFilenameTestExclude(fileNameTmp, exclude, wildname))
	    continue;

	dirsize++;
	ajDebug("accept '%s'\n", de->d_name);
	name = NULL;
	ajFmtPrintS(&name, "%S%s", fileDirfixTmp, de->d_name);
	ajListstrPushAppend(list, name);
    }
    
    closedir(dp);
    ajDebug("%d files for '%S' '%S'\n", dirsize, path, wildname);
    ajStrDelStatic(&fileNameTmp);
    
    return ajFilebuffNewListinList(list);
}




/* @obsolete ajFileBuffNewDWE
** @rename ajFilebuffNewPathWildExclude
*/

__deprecated AjPFilebuff ajFileBuffNewDWE(const AjPStr dir,
                                          const AjPStr wildfile,
                                          const AjPStr exclude)
{
    return ajFilebuffNewPathWildExclude(dir, wildfile, exclude);
}




/* @funcstatic fileBuffInit ***************************************************
**
** Initialises the data for a buffered file.
**
** @param [u] thys [AjPFilebuff] Buffered file object.
** @return [void]
******************************************************************************/

static void fileBuffInit(AjPFilebuff thys)
{
    thys->Last = thys->Curr = thys->Prev = thys->Lines = NULL;
    thys->Freelines = thys->Freelast = NULL;
    thys->Pos = thys->Size = 0;

    return;
}




/* @funcstatic fileOpenDir ****************************************************
**
** Runs 'opendir' on the specified directory. If the directory name
** has no trailing slash (on Unix) then one is added. This is why the
** directory name must be writable.
**
** @param [u] dir [AjPStr*] Directory name.
** @return [DIR*] result of the opendir call.
** @@
******************************************************************************/

static DIR* fileOpenDir(AjPStr* dir)
{
    AjBool moved = ajFalse;
    AjPStr cwdpath = NULL;

    if(ajStrGetCharLast(*dir) != SLASH_CHAR)
	ajStrAppendC(dir, SLASH_STRING);

    /*  going up */
    while(ajStrPrefixC(*dir, UP_DIR))
    {
	if(!moved)
	  cwdpath = ajStrNewS(ajFileValueCwd());

	moved = ajTrue;
	ajDirnameUp(&cwdpath);
	ajStrKeepRange(dir, 3, -1);
	ajDebug("Going up '%S' '%S'\n", *dir, fileCwd);
    }

    if(moved)
	ajStrInsertS(dir, 0, cwdpath);

    ajDebug("fileOpenDir opened '%S'\n", *dir);

    if(moved)
      ajStrDel(&cwdpath);

    return opendir(ajStrGetPtr(*dir));
}




/* @section Buffered File Destructors *****************************************
**
** @fdata [AjPFilebuff]
**
** Destruction is achieved by closing the file.
**
** Unlike ANSI C, there are tests to ensure a file is not closed twice.
**
** @nam3rule Del Destructor
**
** @argrule Del Pbuff [AjPFilebuff*] Buffered file to be deleted
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajFilebuffDel ********************************************************
**
** Destructor for a buffered file object.
**
** @param [d] Pbuff [AjPFilebuff*] Buffered file object.
** @return [void]
** @@
******************************************************************************/

void ajFilebuffDel(AjPFilebuff* Pbuff)
{
    AjPFilebuff thys;

    if(!Pbuff)
	return;
    
    thys = *Pbuff;
    
    if(!thys)
	return;

    if(thys->File)
        ajDebug("ajFilebuffDel fp: %p\n", thys->File->fp);

    ajFilebuffClear(thys, -1);
    filebuffFreeClear(thys);
    ajFileClose(&thys->File);
    AJFREE(*Pbuff);
    
    return;
}




/* @obsolete ajFileBuffDel
** @rename ajFilebuffDel
*/
__deprecated void ajFileBuffDel(AjPFilebuff* Pbuff)
{
    ajFilebuffDel(Pbuff);
}




/* @section file reopen
**
** @fdata [AjPFilebuff]
**
** These functions close a file and open a new file with the same file object
**
** @nam3rule Reopen Reopen an existing buffered file object
** @nam4rule File Reopen using a new file object
**
** @argrule Reopen Pbuff [AjPFilebuff*] Buffered file object
** @argrule Reopen file [AjPFile] File object
**
** @valrule * [AjBool] True on success
**
** @fcategory modify
**
******************************************************************************/




/* @func ajFilebuffReopenFile *************************************************
**
** Sets buffered input file to use a new open file.
**
** The AjPFile pointer is a clone, so we should simply overwrite
** whatever was there before, but we do need to clear the previous buffer
** contents.
**
** @param [w] Pbuff [AjPFilebuff*] Buffered file object.
** @param [u] file [AjPFile] File object to be buffered.
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFilebuffReopenFile(AjPFilebuff* Pbuff, AjPFile file)
{
    AjPFilebuff thys;

    if(!file)
    {
        ajFatal("used ajFilebuffSetFile to delete file buffer");
	ajFilebuffDel(Pbuff);
	return ajFalse;
    }

    if(!*Pbuff)
    {
	*Pbuff = ajFilebuffNewFromFile(file);
	thys = *Pbuff;
	return ajTrue;
    }

    thys = *Pbuff;

    thys->File = file;

    fileBuffInit(thys);

    return ajTrue;
}




/* @obsolete ajFileBuffSetFile
** @remove Use ajFilebuffReopenFile or ajFilebuffClear
*/
__deprecated AjBool ajFileBuffSetFile(AjPFilebuff* pthys,
                                      AjPFile file, AjBool samefile)
{
    if(samefile)
        ajFilebuffClear(*pthys, -1);
    else
        ajFilebuffReopenFile(pthys, file);

    return ajTrue;
}




/* @section Buffered File Modifiers *******************************************
**
** @fdata [AjPFilebuff]
**
** These functions use the attributes of a buffered file object and
** update them.
**
** @nam3rule Clear Removes processed lines from the buffer
** @nam3rule Fix Resets the pointer and current record of a file buffer
**           after buffer contents have been edited
** @nam3rule Reset Reset buffered file attributes and contents.
** @nam3rule Set Set file attribute
** @nam4rule Pos Reset file pointer to start of buffer
** @nam5rule Buffered Set file unbuffered
** @nam4rule Buffered Set file buffered
** @nam4rule Unbuffered Set file unbuffered
**
** @suffix Store Reset test store buffer
**
** @argrule * buff [AjPFilebuff] Buffered input file object
** @argrule C line [const char*] Line of text
** @argrule S line [const AjPStr] Line of text
** @argrule Clear lines [ajint] Number of lines to retain
**                              (-1 to clear whole buffer)
** @argrule ClearStore lastline [const AjPStr] Last line of input
** @argrule Store dostore [AjBool] If true, use the text store buffer
** @argrule Store Pstore [AjPStr*] Original text store buffer
**
** @valrule * [void]
** @valrule *Set [AjBool] Previous buffer setting
**
** @fcategory modify
**
******************************************************************************/




/* @func ajFilebuffClear ******************************************************
**
** Deletes processed lines from a file buffer. The buffer has a record
** (Pos) of the next unprocessed line in the buffer.
**
** Unbuffered files need special handling. The buffer can be turned off
** while it still contains data. If so, we have to carefully run it down.
** If this runs it to zero, we may want to save the last line read.
**
** @param [u] buff [AjPFilebuff] File buffer
** @param [r] lines [ajint] Number of lines to retain. -1 deletes everything.
** @return [void]
** @@
******************************************************************************/

void ajFilebuffClear(AjPFilebuff buff, ajint lines)
{
    ajint i = 0;
    AjPFilebufflist list;
    AjPFilebufflist next;
    ajint first;
    ajint ifree = 0;
    
    ajDebug("ajFilebuffClear (%d) Nobuff: %B\n", lines, buff->Nobuff);
    /*FilebuffTraceFull(buff, buff->Size, 100);*/

    if(!buff)
	return;
    
    if(!buff->File)
	return;
    
    if(lines < 0)
	first = buff->Size;
    else
	first = buff->Pos - lines;
    
    if(first < 0)
	first = 0;
    
    /* nobuff, and all read */
    if(buff->Nobuff && buff->Pos == buff->Size)
	/* delete any old saved line */
	first = buff->Pos;
    
    list = buff->Lines;
    for(i=0; i < first; i++)
    {
	/* we save one line at a time */
	next = list->Next;
	/* so keep a note of the next one for later */
	/*ajDebug("Try to reuse %x size: %d use: %d\n",
	  list->Line, ajStrGetRes(list->Line), ajStrGetUse(list->Line));*/
	
	if(buff->Nobuff)
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

		if(!buff->Freelines)
		{
		    /* start a new free list */
		    buff->Freelines = list;
		    buff->Freelast = list;
		    /*ajDebug("start  list Free %x Freelast %x \n",
		      buff->Freelines, buff->Freelast);*/
		}
		else
		{
		    /* append to free list */
		    buff->Freelast->Next = list;
		    buff->Freelast = buff->Freelast->Next;
		    /*ajDebug("append list Free %x Freelast %x \n",
		      buff->Freelines, buff->Freelast);*/
		}
	    }
	    else
	    {
		ajDebug("ajStrDelReuse was false\n");
	    }
	}
	list = next;
    }
    
    buff->Pos   = 0;
    buff->Size -= i;

    if(!buff->Size)
    {
	ajDebug("size 0: Lines: %x Curr: %x  Prev: %x Last: %x Free: "
                "%x Freelast: %x\n",
		buff->Lines, buff->Curr, buff->Prev, buff->Last,
		buff->Freelines, buff->Freelast);
    }

    buff->Lines = buff->Curr = list;

    ajDebug("ajFilebuffClear '%F' (%d lines)\n"
	     "     %b size: %d pos: %d removed %d lines add to free: %d\n",
	     buff->File, lines, buff->Nobuff, buff->Size, buff->Pos, i, ifree);
    
    ajFilebuffTrace(buff);
    
    if(buff->Nobuff && !buff->Size && lines == 1)
    {
	/* unbuffered - can only save last line */
	if(buff->Lines)
	    ajFatal("Buffer error clearing unbuffered file "
		    "in ajFilebuffClear\n");

	buff->Lines = AJNEW0(buff->Last);

	ajStrAssignS(&buff->Last->Line, buff->File->Buff);
	buff->Curr = buff->Last;
	buff->Curr->Fpos = buff->Fpos;
	buff->Last->Next = NULL;
	buff->Pos = 0;
	buff->Size = 1;
    }
    
    return;
}




/* @obsolete ajFileBuffClear
** @rename ajFilebuffClear
*/
__deprecated void ajFileBuffClear(AjPFilebuff buff, ajint lines)
{
    ajFilebuffClear(buff, lines);
}




/* @func ajFilebuffClearStore *************************************************
**
** Deletes processed lines from a file buffer. The buffer has a record
** (Pos) of the next unprocessed line in the buffer.
**
** Unbuffered files need special handling. The buffer can be turned off
** while it still contains data. If so, we have to carefully run it down.
** If this runs it to zero, we may want to save the last line read.
**
** @param [u] buff [AjPFilebuff] File buffer
** @param [r] lines [ajint] Number of lines to retain. -1 deletes everything.
** @param [r] lastline [const AjPStr] Last line of input.
**                            Used to count characters to be saved
** @param [r] dostore [AjBool] append if true
** @param [w] Pstore [AjPStr*] Caller's record of the processed lines
** @return [void]
** @@
******************************************************************************/

void ajFilebuffClearStore(AjPFilebuff buff, ajint lines, const AjPStr lastline,
                          AjBool dostore, AjPStr *Pstore)
{
    ajFilebuffClear(buff, lines);

    if(dostore && ajStrGetLen(lastline))
	ajStrCutEnd(Pstore, ajStrGetLen(lastline));

    return;
}




/* @obsolete ajFileBuffClearStore
** @rename ajFilebuffClearStore
*/
__deprecated void ajFileBuffClearStore(AjPFilebuff buff, ajint lines,
			   const AjPStr rdline, AjBool store, AjPStr *astr)
{
    ajFilebuffClearStore(buff, lines, rdline, store, astr);

    return;
}




/* @func ajFilebuffFix ********************************************************
**
** Resets the pointer and current record of a file buffer so the next
** read starts at the first buffered line. Fixes buffer size after the
** buffer has been edited.
**
** @param [u] buff [AjPFilebuff] File buffer
** @return [void]
** @@
******************************************************************************/

void ajFilebuffFix(AjPFilebuff buff)
{
    AjPFilebufflist list;
    ajint i = 1;

    ajFilebuffReset(buff);
    buff->Pos  = 0;
    buff->Curr = buff->Lines;

    list = buff->Lines;

    if(!list)
    {
	buff->Size = 0;
	return;
    }

    while(list->Next)
    {
	i++;
	list = list->Next;
    }

    if(i != buff->Size)
	ajDebug("ajFilebuffFix size was %d now %d\n", buff->Size, i);

    buff->Size=i;
    return;
}




/* @obsolete ajFileBuffFix
** @rename ajFilebuffFix
*/
__deprecated void ajFileBuffFix(AjPFilebuff buff)
{
    ajFilebuffFix(buff);

    return;
}




/* @func ajFilebuffReset ******************************************************
**
** Resets the pointer and current record of a file buffer so the next read
** starts at the first buffered line.
**
** @param [u] buff [AjPFilebuff] File buffer
** @return [void]
** @@
******************************************************************************/

void ajFilebuffReset(AjPFilebuff buff)
{
    buff->Pos  = 0;
    buff->Curr = buff->Lines;
    buff->Prev = NULL;

    return;
}




/* @obsolete ajFileBuffReset
** @rename ajFilebuffReset
*/
__deprecated void ajFileBuffReset(AjPFilebuff buff)
{
    ajFilebuffReset(buff);

    return;
}




/* @func ajFilebuffResetPos ***************************************************
**
** Resets the pointer and current record of a file buffer so the next read
** starts at the first buffered line.
**
** Also resets the file position to the last known read, to undo the
** damage done by (for example) ajseqabi functions.
**
** @param [u] buff [AjPFilebuff] File buffer
** @return [void]
** @@
******************************************************************************/

void ajFilebuffResetPos(AjPFilebuff buff)
{
    ajDebug("ajFilebuffResetPos   End: %B  Fpos: %ld ftell: %ld\n",
            buff->File->End, buff->Fpos, ftell(buff->File->fp));
    /*ajFilebuffTraceFull(buff, 10, 10);*/

    buff->Pos  = 0;
    buff->Curr = buff->Lines;

    if(!buff->File->End && (buff->File->fp != stdin))
	ajFileSeek(buff->File, buff->File->Filepos, SEEK_SET);

    buff->File->Filepos = buff->Fpos;

    /*ajFilebuffTraceFull(buff,10,10);*/

    return;
}




/* @obsolete ajFileBuffResetPos
** @rename ajFilebuffResetPos
*/
__deprecated void ajFileBuffResetPos(AjPFilebuff buff)
{
    ajFilebuffResetPos(buff);

    return;
}




/* @func ajFilebuffResetStore *************************************************
**
** Resets the pointer and current record of a file buffer so the next read
** starts at the first buffered line.
**
** @param [u] buff [AjPFilebuff] File buffer
** @param [r] dostore [AjBool] True if text is stored
** @param [w] Pstore [AjPStr*] Stored string cleared if store is true
** @return [void]
** @@
******************************************************************************/

void ajFilebuffResetStore(AjPFilebuff buff, AjBool dostore, AjPStr *Pstore)
{
    ajFilebuffReset(buff);

    if(dostore)
	ajStrAssignClear(Pstore);

    return;
}




/* @obsolete ajFileBuffResetStore
** @rename ajFilebuffResetStore
*/
__deprecated void ajFileBuffResetStore(AjPFilebuff buff,
                                       AjBool store, AjPStr *astr)
{
    ajFilebuffResetStore(buff, store, astr);

    return;
}




/* @funcstatic filebuffFreeClear ***********************************************
**
** Deletes freed lines from a file buffer. The free list is used to avoid
** reallocating space for new records and must be deleted as part of
** the destructor.
**
** @param [u] buff [AjPFilebuff] File buffer
** @return [void]
** @@
******************************************************************************/

static void filebuffFreeClear(AjPFilebuff buff)
{
    AjPFilebufflist list;

    if(!buff)
	return;

    /*ajDebug("ajFileBuffFreeClear %x\n", buff->Freelines);*/

    while(buff->Freelines)
    {
	list = buff->Freelines;
	buff->Freelines = buff->Freelines->Next;
	ajStrDel(&list->Line);
	AJFREE(list);
    }

    return;
}




/* @func ajFilebuffSetBuffered *************************************************
**
** Sets file to be buffered. If it already has buffered data, we have to
** first run down the buffer.
**
** @param [u] buff [AjPFilebuff] Buffered file object.
** @return [AjBool] ajTrue if the file was unbuffered before
** @@
******************************************************************************/

AjBool ajFilebuffSetBuffered(AjPFilebuff buff)
{
    AjBool ret;

    if(!buff)
	return ajFalse;

    ret = buff->Nobuff;
    ajDebug("ajFileBuffBuff %F buffsize: %d\n", buff->File, buff->Size);
    buff->Nobuff = ajFalse;

    return ret;
}




/* @obsolete ajFileBuffBuff
** @rename ajFilebuffSetBuffered
*/
__deprecated AjBool ajFileBuffBuff(AjPFilebuff buff)
{
    return ajFilebuffSetBuffered(buff);
}




/* @func ajFilebuffSetUnbuffered ***********************************************
**
** Sets file to be unbuffered. If it already has buffered data, we have to
** first run down the buffer.
**
** @param [u] buff [AjPFilebuff] Buffered file object.
** @return [AjBool] ajTrue if the file was unbuffered before
** @@
******************************************************************************/

AjBool ajFilebuffSetUnbuffered(AjPFilebuff buff)
{
    AjBool ret;

    if(!buff)
	return ajFalse;

    ret = buff->Nobuff;
    ajDebug("ajFileBuffNobuff %F buffsize: %d\n", buff->File, buff->Size);
    buff->Nobuff = ajTrue;

    return ret;
}




/* @obsolete ajFileBuffNobuff
** @rename ajFilebuffSetUnbuffered
*/
__deprecated AjBool ajFileBuffNobuff(AjPFilebuff buff)
{
    return ajFilebuffSetUnbuffered(buff);
}




/* @section Buffered File Content Modifiers ************************************
**
** @fdata [AjPFilebuff]
**
** These functions use the buffer contents of a buffered file object
** and update them.
**
** @nam3rule Html Modify HTML tags in the buffer
** @nam4rule HtmlPre Reduce to a pre-formatted section if found in HTML
** @nam4rule HtmlNoheader Remove HTML header in the buffer
** @nam4rule HtmlStrip Remove all HTML tags in the buffer
** @nam3rule Load Add text to the buffer
** @nam4rule LoadAll Read all file lines into buffer
**
** @suffix C Character string text
** @suffix S String object text
**
** @argrule * buff [AjPFilebuff] Buffered file object
** @argrule C line [const char*] Line of text
** @argrule S line [const AjPStr] Line of text
**
** @valrule * [void]
** @valrule *HtmlPre [AjBool] True if changed
**
** @fcategory modify
**
******************************************************************************/




/* @func ajFilebuffHtmlNoheader **********************************************
**
** Processes data in the file buffer, removing HTML titles and
** decoding possible chunked input
**
** @param [u] buff [AjPFilebuff] Buffered file with data loaded
**                                     in the buffer.
** @return [void]
** @@
******************************************************************************/

void ajFilebuffHtmlNoheader(AjPFilebuff buff)
{
    AjPRegexp httpexp  = NULL;
    AjPRegexp nullexp  = NULL;
    AjPRegexp chunkexp = NULL;
    AjPRegexp hexexp   = NULL;
    
    AjBool doChunk = ajFalse;
    ajint ichunk;
    ajint chunkSize;
    ajint iline;
    AjPStr saveLine = NULL;
    AjPStr hexstr   = NULL;
    
    httpexp  = ajRegCompC("^HTTP/");
    nullexp  = ajRegCompC("^\r?\n?$");
    chunkexp = ajRegCompC("^Transfer-Encoding: +chunked");
    hexexp   = ajRegCompC("^([0-9a-fA-F]+) *\r?\n?$");
    
    /* first take out the HTTP header (HTTP 1.0 onwards) */
    if(!buff->Size)
	return;
    
    /*ajFilebuffTraceTitle(buff, "Before ajFileBuffStripHtml");*/


    ajDebug("First line [%d] '%S' \n",
	     ajStrGetUse(buff->Curr->Line), buff->Curr->Line);
    
    if(ajRegExec(httpexp, buff->Curr->Line))
    {
	/* ^HTTP  header processing */
	while(buff->Pos < buff->Size &&
	      !ajRegExec(nullexp, buff->Curr->Line))
	{
	    /* to empty line */
	    if(ajRegExec(chunkexp, buff->Curr->Line))
	    {
		ajDebug("Chunk encoding: %S", buff->Curr->Line);
		/* chunked - see later */
		doChunk = ajTrue;
	    }
	    fileBuffLineDel(buff);
	}

	/* blank line after header */
	fileBuffLineDel(buff);
    }

    if(doChunk)
    {
	/*ajFilebuffTraceFull(buff, 999999, 0);*/
	
	if(!ajRegExec(hexexp, buff->Curr->Line))
	{
	    ajFatal("Bad chunk data from HTTP, expect chunk size got '%S'",
		    buff->Curr->Line);
	}

	ajRegSubI(hexexp, 1, &hexstr);
	ajStrToHex(hexstr, &chunkSize);
	
	ajDebug("chunkSize hex:%x %d\n", chunkSize, chunkSize);
	fileBuffLineDel(buff);	/* chunk size */
	
	ichunk = 0;
	iline = 0;

	while(chunkSize && buff->Curr)
	{
	    iline++;
	    /* get the chunk size - zero is the end */
	    /* process the chunk */
	    ichunk += ajStrGetLen(buff->Curr->Line);
	    
	    /*ajDebug("++input line [%d] ichunk=%d:%d %d:%S",
		    iline, ichunk, chunkSize,
		    ajStrGetLen(buff->Curr->Line), buff->Curr->Line);*/

	    if(ichunk >= chunkSize)	/* end of chunk */
	    {
		if(ichunk == chunkSize)
		{
		    /* end-of-chunk at end-of-line */
		    fileBuffLineNext(buff);
		    ajStrAssignClear(&saveLine);
		    /*ajDebug("end-of-chunk at end-of-line: '%S'\n",
                              saveLine);*/
		}
		else
		{
		    /* end-of-chunk in mid-line, patch up the input */
		    ajDebug("end-of-chunk in mid-line, %d:%d have input: "
                            "%d '%S'\n",
			    ichunk, chunkSize,
			    ajStrGetLen(buff->Curr->Line),
			    buff->Curr->Line);
		    ajStrAssignSubS(&saveLine, buff->Curr->Line, 0,
				-(ichunk-chunkSize+1));
		    ajStrKeepRange(&buff->Curr->Line, -(ichunk-chunkSize), -1);
		}
		
		/* skip a blank line */
		
		if(!ajRegExec(nullexp, buff->Curr->Line))
		{
		    ajFilebuffTraceTitle(buff, "Blank line not found");
		    ajFatal("Bad chunk data from HTTP, expect blank line"
			    " got '%S'", buff->Curr->Line);
		}

		fileBuffLineDel(buff);
		
		/** read the next chunk size */
		
		if(!ajRegExec(hexexp, buff->Curr->Line))
		{
		    ajFilebuffTraceTitle(buff, "Chunk size not found");
		    ajFatal("Bad chunk data from HTTP, expect chunk size "
			    "got '%S'",
			    buff->Curr->Line);
		}

		ajRegSubI(hexexp, 1, &hexstr);
		ajStrToHex(hexstr, &chunkSize);
		ichunk = 0;
		fileBuffLineDel(buff);
	    }

	    if(saveLine)
	    {
		if(ajStrGetLen(saveLine))
		{
		    ichunk = ajStrGetLen(buff->Curr->Line);
		    /* preserve the line split by chunk size */
		    ajStrInsertS(&buff->Curr->Line, 0, saveLine);

		    if(ichunk < chunkSize)
		    {
			/* process the next line */
			fileBuffLineNext(buff); /* after restored line */
		    }
		    else
		    {
			/* we already have the whole chunk! */
			ichunk -= ajStrGetLen(buff->Curr->Line);
		    }
		}
		else
		{
		    /* just a chunk size, skip */
		    if(buff->Curr && chunkSize)
		    {
			/*fileBuffLineDel(buff);*/
		    }
		    else if (chunkSize)/* final non-zero chunk size */
		    {
			fileBuffLineDel(buff);
		    }
		}

		ajStrDel(&saveLine);
	    }
	    else
	    {
		/* next line */
		fileBuffLineNext(buff);
	    }
	}

	ajFilebuffFix(buff);
	/*ajFilebuffTraceFull(buff, 999999, 0);*/
	ajStrDel(&hexstr);
	/*ajFilebuffTraceTitle(buff, "Chunks resolved");*/
    }
    
    ajFilebuffReset(buff);

    ajRegFree(&httpexp);
    ajRegFree(&nullexp);
    ajRegFree(&chunkexp);
    ajRegFree(&hexexp);
    
    return;
}




/* @func ajFilebuffHtmlPre *****************************************************
**
** If we only have one pre-formatted section in HTML, that is all we keep.
**
** @param [u] buff [AjPFilebuff] buffer
** @return [AjBool] ajTrue=cleaned ajFalse=unchanged
** @@
******************************************************************************/

AjBool ajFilebuffHtmlPre(AjPFilebuff buff)
{
    AjPFilebufflist lptr    = NULL;
    AjPFilebufflist tptr    = NULL;
    AjPRegexp preexp = NULL;
    AjPRegexp endexp = NULL;
    ajint ifound = 0;
    
    lptr = buff->Curr;
    
    preexp = ajRegCompC("<[Pp][Rr][Ee]>");

    lptr=buff->Curr;
    
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

    lptr=buff->Curr;
    

    while(lptr && !ajRegExec(preexp, lptr->Line))
    {
	tptr = lptr;
	lptr = lptr->Next;
	ajStrDel(&tptr->Line);
	AJFREE(tptr);
	buff->Size--;
    }
    
    buff->Lines = buff->Curr = lptr;
    ajRegPost(preexp, &lptr->Line);
    ajRegFree(&preexp);

    endexp = ajRegCompC("</[Pp][Rr][Ee]>");

    while(lptr && !ajRegExec(endexp,lptr->Line))
    {
	lptr    = lptr->Next;
    }
    
    ajRegPre(endexp, &lptr->Line);
    buff->Last = lptr;
    lptr = lptr->Next;
    ajRegFree(&endexp);

    while(lptr)
    {
	tptr = lptr;
	lptr = lptr->Next;
	ajStrDel(&tptr->Line);
	AJFREE(tptr);
	buff->Size--;
    }

    buff->Last->Next = NULL;
    ajFilebuffReset(buff);
    ajFilebuffTraceTitle(buff, "ajFileBuffHtmlPre completed");

    return ajTrue;
}




/* @obsolete ajFileBuffStripHtmlPre
** @rename ajFileBuffHtmlPre
*/
__deprecated AjBool ajFileBuffStripHtmlPre(AjPFilebuff buff)
{
    return ajFilebuffHtmlPre(buff);
}




/* @func ajFilebuffHtmlStrip **************************************************
**
** Processes data in the file buffer, removing HTML tokens between
** angle brackets, plus any TITLE. This seems to be enough to make HTML
** output readable.
**
** @param [u] buff [AjPFilebuff] Buffered file with data loaded
**                                     in the buffer.
** @return [void]
** @@
******************************************************************************/

void ajFilebuffHtmlStrip(AjPFilebuff buff)
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
    
    /* first take out the HTTP header (HTTP 1.0 onwards) */
    if(!buff->Size)
	return;
    
    tagexp   = ajRegCompC("^(.*)(<[!/A-Za-z][^>]*>)(.*)$");
    fullexp  = ajRegCompC("^(.*)(<([A-Za-z]+)[^>]*>.*</\\3>)(.*)$");
    httpexp  = ajRegCompC("^HTTP/");
    nullexp  = ajRegCompC("^\r?\n?$");
    chunkexp = ajRegCompC("^Transfer-Encoding: +chunked");
    hexexp   = ajRegCompC("^([0-9a-fA-F]+) *\r?\n?$");
    ncbiexp  = ajRegCompC("^Entrez Reports\r?\n$");
    ncbiexp2 = ajRegCompC("^----------------\r?\n$");
    srsdbexp = ajRegCompC("^([A-Za-z0-9_-]+)(:)([A-Za-z0-9_-]+)");
    
    ajFilebuffTraceTitle(buff, "Before ajFileBuffStripHtml");

    i = 0;
    
    /* ajDebug("First line [%d] '%S' \n",
       ajStrGetUse(buff->Curr->Line), buff->Curr->Line);*/
    
    if(ajRegExec(httpexp, buff->Curr->Line))
    {
	/* ^HTTP  header processing */
	while(buff->Pos < buff->Size &&
	      !ajRegExec(nullexp, buff->Curr->Line))
	{
	    /* to empty line */
	    if(ajRegExec(chunkexp, buff->Curr->Line))
	    {
		/*ajDebug("Chunk encoding: %S", buff->Curr->Line);*/
		/* chunked - see later */
		doChunk = ajTrue;
	    }
	    fileBuffLineDel(buff);
	}

	/* blank line after header */
	fileBuffLineDel(buff);
    }

    if(doChunk)
    {
	/*ajFilebuffTraceFull(buff, 999999, 0);*/
	
	if(!ajRegExec(hexexp, buff->Curr->Line))
	{
	    ajFatal("Bad chunk data from HTTP, expect chunk size got '%S'",
		    buff->Curr->Line);
	}

	ajRegSubI(hexexp, 1, &hexstr);
	ajStrToHex(hexstr, &chunkSize);
	
	/*ajDebug("chunkSize hex:%x %d\n", chunkSize, chunkSize);*/
	fileBuffLineDel(buff);	/* chunk size */
	
	ichunk = 0;
	iline = 0;

	while(chunkSize && buff->Curr)
	{
	    iline++;
	    /* get the chunk size - zero is the end */
	    /* process the chunk */
	    ichunk += ajStrGetLen(buff->Curr->Line);
	    
	    /*ajDebug("++input line [%d] ichunk=%d:%d %d:%S",
		    iline, ichunk, chunkSize,
		    ajStrGetLen(buff->Curr->Line), buff->Curr->Line);*/

	    if(ichunk >= chunkSize)	/* end of chunk */
	    {
		if(ichunk == chunkSize)
		{
		    /* end-of-chunk at end-of-line */
		    fileBuffLineNext(buff);
		    ajStrAssignClear(&saveLine);
		    /*ajDebug("end-of-chunk at end-of-line: '%S'\n",
                      saveLine); */
		}
		else
		{
		    /* end-of-chunk in mid-line, patch up the input */
		    /*ajDebug("end-of-chunk in mid-line, %d:%d have input: "
                            "%d '%S'\n",
			    ichunk, chunkSize,
			    ajStrGetLen(buff->Curr->Line),
			    buff->Curr->Line);*/
		    ajStrAssignSubS(&saveLine, buff->Curr->Line, 0,
				-(ichunk-chunkSize+1));
		    ajStrKeepRange(&buff->Curr->Line, -(ichunk-chunkSize), -1);
		}
		
		/* skip a blank line */
		
		if(!ajRegExec(nullexp, buff->Curr->Line))
		{
		    ajFilebuffTraceTitle(buff, "Blank line not found");
		    ajFatal("Bad chunk data from HTTP, expect blank line"
			    " got '%S'", buff->Curr->Line);
		}

		fileBuffLineDel(buff);
		
		/** read the next chunk size */
		
		if(!ajRegExec(hexexp, buff->Curr->Line))
		{
		    ajFilebuffTraceTitle(buff, "Chunk size not found");
		    ajFatal("Bad chunk data from HTTP, expect chunk size "
			    "got '%S'",
			    buff->Curr->Line);
		}

		ajRegSubI(hexexp, 1, &hexstr);
		ajStrToHex(hexstr, &chunkSize);
		ichunk = 0;
		fileBuffLineDel(buff);
	    }

	    if(saveLine)
	    {
		if(ajStrGetLen(saveLine))
		{
		    ichunk = ajStrGetLen(buff->Curr->Line);
		    /* preserve the line split by chunk size */
		    ajStrInsertS(&buff->Curr->Line, 0, saveLine);

		    if(ichunk < chunkSize)
		    {
			/* process the next line */
			fileBuffLineNext(buff); /* after restored line */
		    }
		    else
		    {
			/* we already have the whole chunk! */
			ichunk -= ajStrGetLen(buff->Curr->Line);
		    }
		}
		else
		{
		    /* just a chunk size, skip */
		    if(buff->Curr && chunkSize)
		    {
			/*fileBuffLineDel(buff);*/
		    }
		    else if (chunkSize)/* final non-zero chunk size */
		    {
			fileBuffLineDel(buff);
		    }
		}

		ajStrDel(&saveLine);
	    }
	    else
	    {
		/* next line */
		fileBuffLineNext(buff);
	    }
	}

	ajFilebuffFix(buff);
	/*ajFilebuffTraceFull(buff, 999999, 0);*/
	ajStrDel(&hexstr);
	/*ajFilebuffTraceTitle(buff, "Chunks resolved");*/
    }
    
    ajFilebuffReset(buff);

    /*
     ** Now we have a clean single file to process
     */
    
    ajFilebuffTraceTitle(buff, "About to preprocess");
    ajFilebuffHtmlPre(buff);

    while(buff->Curr)
    {
	if(ajRegExec(ncbiexp, buff->Curr->Line))
	    ajStrAssignC(&buff->Curr->Line, "\n");

	if(ajRegExec(ncbiexp2, buff->Curr->Line))
	    ajStrAssignC(&buff->Curr->Line, "\n");

	while(ajRegExec(fullexp, buff->Curr->Line))
	{
	    ajRegSubI(fullexp, 1, &s1);
	    ajRegSubI(fullexp, 2, &s2);
	    ajRegSubI(fullexp, 4, &s3);
	    ajFmtPrintS(&buff->Curr->Line, "%S%S", s1, s3);
	}

	while(ajRegExec(tagexp, buff->Curr->Line))
	{
	    ajRegSubI(tagexp, 1, &s1);
	    ajRegSubI(tagexp, 2, &s2);
	    ajRegSubI(tagexp, 3, &s3);
	    ajFmtPrintS(&buff->Curr->Line, "%S%S", s1, s3);
	}

	if(ajRegExec(srsdbexp, buff->Curr->Line))
	{
	    ajRegSubI(srsdbexp,1,&s1);
	    ajRegSubI(srsdbexp,2,&s2);
	    ajRegSubI(srsdbexp,3,&s3);
	    fileBuffLineDel(buff);
	    ++i;
	    continue;
	}

	if(ajRegExec(nullexp, buff->Curr->Line))
	{
	    /* allow for newline */
	    fileBuffLineDel(buff);
	}
	else
	    fileBuffLineNext(buff);

	i++;
    }
    
    ajFilebuffReset(buff);
    
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
    
    ajFilebuffTraceTitle(buff, "After ajFileBuffStripHtml");

    return;
}




/* @obsolete ajFileBuffStripHtml
** @rename ajFilebuffHtmlStrip
*/
__deprecated void ajFileBuffStripHtml(AjPFilebuff buff)
{
    ajFilebuffHtmlStrip(buff);

    return;
}




/* @func ajFilebuffLoadC ******************************************************
**
** Adds a line to the buffer.
**
** Intended for cases where the file data must be pre-processed before
** being seen by the sequence reading routines. The first case was
** for stripping HTML tags after reading via HTTP.
**
** @param [u] buff [AjPFilebuff] Buffered file.
** @param [r] line [const char*] Line of input.
** @return [void]
** @@
******************************************************************************/

void ajFilebuffLoadC(AjPFilebuff buff, const char* line)
{
    if(!buff->Lines)
	buff->Curr = buff->Lines = AJNEW0(buff->Last);
    else
	buff->Last = AJNEW0(buff->Last->Next);

    ajStrAssignC(&buff->Last->Line,line);
    buff->Last->Next = NULL;
    buff->Size++;

    return;
}




/* @obsolete ajFileBuffLoadC
** @rename ajFilebuffLoadC
*/
__deprecated void ajFileBuffLoadC(AjPFilebuff buff, const char* line)
{
    ajFilebuffLoadC(buff, line);

    return;
}




/* @func ajFilebuffLoadS ******************************************************
**
** Adds a copy of a line to the buffer.
**
** Intended for cases where the file data must be pre processed before
** being seen by the sequence reading routines. The first case was
** for stripping HTML tags after reading via HTTP.
**
** @param [u] buff [AjPFilebuff] Buffered file.
** @param [r] line [const AjPStr] Line of input.
** @return [void]
** @@
******************************************************************************/

void ajFilebuffLoadS(AjPFilebuff buff, const AjPStr line)
{
    if(!buff->Lines)
	buff->Curr = buff->Lines = AJNEW0(buff->Last);
    else
	buff->Last = AJNEW0(buff->Last->Next);

    ajStrAssignS(&buff->Last->Line,line);
    buff->Last->Next = NULL;
    buff->Size++;

    return;
}




/* @obsolete ajFileBuffLoadS
** @rename ajFilebuffLoadS
*/
__deprecated void ajFileBuffLoadS(AjPFilebuff buff, const AjPStr line)
{
    ajFilebuffLoadS(buff, line);

    return;
}




/* @func ajFilebuffLoadAll ****************************************************
**
** Reads all input lines from a file into the buffer.
**
** Intended for cases where the file data must be pre-processed before
** being seen by the sequence reading routines. The first case was
** for stripping HTML tags after reading via HTTP.
**
** @param [u] buff [AjPFilebuff] Buffered file.
** @return [void]
** @@
******************************************************************************/

void ajFilebuffLoadAll(AjPFilebuff buff)
{
    AjPStr rdline = NULL;
    AjBool status = ajTrue;

    while(status)
    {
	status = ajBuffreadLine(buff, &rdline);
     /* ajDebug("read: <%S>\n", rdline);*/
    }

    ajFilebuffReset(buff);
    ajStrDel(&rdline);

    /*ajFilebuffTrace(buff);*/

    return;
}




/* @obsolete ajFileBuffLoad
** @rename ajFilebuffLoadAll
*/
__deprecated void ajFileBuffLoad(AjPFilebuff buff)
{
    ajFilebuffLoadAll(buff);
}




/* @section Buffered File Operators *******************************************
**
** @fdata [AjPFilebuff]
**
** These functions use the contents of a file object but do not make
** any changes.
**
** @nam3rule Get Return an attribute value
** @nam4rule GetFile Return the file object
** @nam4rule GetFileptr Return the C file pointer
** @nam3rule Is Tests a condition
** @nam4rule IsBuffered Tests whether file input is buffered
** @nam4rule IsEmpty Tests whether buffer is exhausted for all input files
** @nam4rule IsEnded Tests whether buffer is at end of current file and empty
** @nam4rule IsEof Tests whether buffer is at end of file
**
** @argrule * buff [const AjPFilebuff] Buffered file object
**
** @valrule GetFileptr [FILE*] C file pointer
** @valrule GetFile [AjPFile] File object
** @valrule IsBuffered [AjBool] True if file is buffered
** @valrule IsEmpty [AjBool] True if buffer is empty
** @valrule IsEnded [AjBool] True if buffer is ended for this file
** @valrule IsEof [AjBool] True if buffer has read to end of this file
**
** @fcategory use
**
******************************************************************************/




/* @func ajFilebuffGetFile ****************************************************
**
** Returns the file object from a buffered file object.
**
** @param [r] buff [const AjPFilebuff] Buffered file.
** @return [AjPFile] File object.
** @@
******************************************************************************/

AjPFile ajFilebuffGetFile(const AjPFilebuff buff)
{
    return buff->File;
}




/* @obsolete ajFileBuffFile
** @rename ajFilebuffGetFile
*/
__deprecated AjPFile ajFileBuffFile(const AjPFilebuff buff)
{
    return ajFilebuffGetFile(buff);
}




/* @obsolete ajFileBuffSize
** @rename ajFileValueBuffsize
*/
__deprecated ajint ajFileBuffSize(void)
{
    return ajFileValueBuffsize();
}




/* @func ajFilebuffGetFileptr **************************************************
**
** Returns the C file pointer for an open buffered file.
**
** @param [r] buff [const AjPFilebuff] Buffered file.
** @return [FILE*] C file pointer for the file.
** @@
******************************************************************************/

FILE* ajFilebuffGetFileptr(const AjPFilebuff buff)
{
    return buff->File->fp;
}




/* @obsolete ajFileBuffFp
** @rename ajFilebuffGetFileptr
*/
__deprecated FILE* ajFileBuffFp(const AjPFilebuff buff)
{
    return ajFilebuffGetFileptr(buff);
}




/* @func ajFilebuffIsBuffered *************************************************
**
** Tests whether an input file is buffered.
**
** @param [r] buff [const AjPFilebuff] Buffered file object.
** @return [AjBool] ajTrue if the file was unbuffered before
** @@
******************************************************************************/

AjBool ajFilebuffIsBuffered(const AjPFilebuff buff)
{
    if(!buff)
	return ajFalse;

    if(buff->Nobuff)
	return ajFalse;

    return ajTrue;
}




/* @obsolete ajFileBuffIsBuffered
** @rename ajFilebuffIsBuffered
*/
__deprecated AjBool ajFileBuffIsBuffered(const AjPFilebuff buff)
{
    return ajFilebuffIsBuffered(buff);
}




/* @func ajFilebuffIsEmpty *****************************************************
**
** Tests whether a file buffer is empty, including testing for a
** possible next file
**
** @param [r] buff [const AjPFilebuff] Buffered file.
** @return [AjBool] ajTrue if the buffer is empty.
** @@
******************************************************************************/

AjBool ajFilebuffIsEmpty(const AjPFilebuff buff)
{
    ajDebug("ajFileBuffEmpty Size: %d Pos: %d End: %b Handle: %d "
	     "Fp: %x List; %d\n",
	     buff->Size, buff->Pos, buff->File->End, buff->File->Handle,
	     buff->File->fp, ajListstrGetLength(buff->File->List));

    if(buff->Pos < buff->Size)
	return ajFalse;

    /* not open */
    if(!buff->File->fp || !buff->File->Handle)
	return ajTrue;

    if(buff->File->End && !ajListstrGetLength(buff->File->List))
        /* EOF and done */
	return ajTrue;

    ajDebug("ajFileBuffEmpty false\n");

    return ajFalse;
}




/* @obsolete ajFileBuffEmpty
** @rename ajFilebuffIsEmpty
*/
__deprecated AjBool ajFileBuffEmpty(const AjPFilebuff buff)
{
    return ajFilebuffIsEmpty(buff);
}




/* @func ajFilebuffIsEnded *****************************************************
**
** Tests whether the current file is exhausted.
** This means end of file is reached and the buffer is empty.
**
** It is possible that further files are defined in the input list
** (see ajFilebuffIsEmpty)
**
** @param [r] buff [const AjPFilebuff] File buffer
** @return [AjBool] ajTrue if we already set end-of-file
** @@
******************************************************************************/

AjBool ajFilebuffIsEnded(const AjPFilebuff buff)
{
    ajDebug("ajFileBuffEnd End: %B Size: %d\n", buff->File->End, buff->Size);

    /* not reached EOF yet */
    if(!buff->File->End)
	return ajFalse;

    /* Something in the buffer*/
    if(buff->Size != 0)
	return ajFalse;

    return ajTrue;
}




/* @obsolete ajFileBuffEnd
** @rename ajFilebuffIsEnded
*/
__deprecated AjBool ajFileBuffEnd(const AjPFilebuff buff)
{
    return ajFilebuffIsEnded(buff);
}




/* @func ajFilebuffIsEof *******************************************************
**
** Tests whether we have reached end of file already
**
** @param [r] buff [const AjPFilebuff] File buffer
** @return [AjBool] ajTrue if we already set end-of-file
** @@
******************************************************************************/

AjBool ajFilebuffIsEof(const AjPFilebuff buff)
{
    return buff->File->End;
}




/* @obsolete ajFileBuffEof
** @rename ajFilebuffIsEof
*/
__deprecated AjBool ajFileBuffEof(const AjPFilebuff buff)
{
    return ajFilebuffIsEof(buff);
}




/* @section file debug
**
** @fdata [AjPFilebuff]
**
** report file object contents for debugging
**
**
** @nam3rule Trace    Print report to debug file (if any)
** @nam4rule TraceFull    Print full contents report to debug file (if any)
** @nam4rule TraceTitle    Print report with a set title
**
** @argrule * buff [const AjPFilebuff]
** @argrule Full nlines [size_t] Maximum number of lines to report
** @argrule Full nfree [size_t] Maximum number of free lines to report
** @argrule Title title [const char*] Report title
**
** @valrule * [void]
**
** @fcategory misc
******************************************************************************/




/* @func ajFilebuffTrace ******************************************************
**
** Writes debug messages to indicate the contents of a buffered file.
**
** @param [r] buff [const AjPFilebuff] Buffered file.
** @return [void]
** @@
******************************************************************************/

void ajFilebuffTrace(const AjPFilebuff buff)
{
    AjPFilebufflist test;
    ajint i = 0;
    ajint j = -1;

    ajDebug("Trace buffer file '%S'\n"
	     "             Pos: %d Size: %d FreeSize: %d Fpos: %Ld End: %b\n",
	    buff->File->Name, buff->Pos, buff->Size,buff->FreeSize,
	    buff->Fpos, buff->File->End);

    if(buff->Size)
    {
	ajDebug(" Lines: %u\n", buff->Size);

	if(buff->Curr)
	    ajDebug("  Curr: %8Ld %x %x <%S>\n",
		    buff->Curr->Fpos, 
		    buff->Curr->Line, buff->Curr->Next,
		    buff->Curr->Line);
	else
	    ajDebug("  Curr: <null>\n");

	if(buff->Lines)
	    ajDebug("  From: %8Ld %x %x <%S>\n",
		    buff->Lines->Fpos, 
		    buff->Lines->Line, buff->Lines->Next,
		    buff->Lines->Line);
	else
	    ajDebug("  From: <null>\n");

	if(buff->Last)
	    ajDebug("    To: %8Ld %x %x <%S>\n",
		    buff->Last->Fpos, 
		    buff->Last->Line, buff->Last->Next,
		    buff->Last->Line);
	else
	    ajDebug("    To: <null>\n");
    }

    if(buff->Freelines)
	for(test = buff->Freelines; test; test=test->Next)
	{
	    i++;

	    if(test == buff->Freelast)
                j = i;
	}

    ajDebug(" Free: %d Last: %d\n", i, j);

    return;
}




/* @obsolete ajFileBuffTrace
** @rename ajFilebuffTrace
*/
__deprecated void ajFileBuffTrace(const AjPFilebuff buff)
{
    ajFilebuffTrace(buff);

    return;
}




/* @func ajFilebuffTraceFull **************************************************
**
** Writes debug messages to show the full contents of a buffered file.
**
** @param [r] buff [const AjPFilebuff] Buffered file.
** @param [r] nlines [size_t] Maximum number of lines to trace.
** @param [r] nfree [size_t] Maximum number of free lines to trace.
** @return [void]
** @@
******************************************************************************/

void ajFilebuffTraceFull(const AjPFilebuff buff, size_t nlines,
			  size_t nfree)
{
    ajint i;
    AjPFilebufflist line;
    AjBool last = ajFalse;

    ajDebug("Trace buffer file '%S' End: %B\n"
	     "             Pos: %d Size: %d Nobuff: %B Fpos: %Ld\n",
	     buff->File->Name, buff->File->End,
	     buff->Pos, buff->Size, buff->Nobuff, buff->Fpos);

    line = buff->Lines;

    for(i=1; line && (i <= (ajint)nlines); i++)
    {
	if(line == buff->Curr)
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

    line = buff->Freelines;

    for(i=1; line && (i <= (ajint)nfree);  i++)
    {
	if(line == buff->Freelast)
            last = ajTrue;

	ajDebug(" Free %x %d: %d bytes %B\n",
		 line->Line, i, ajStrGetRes(line->Line), last);
	line = line->Next;
    }

    return;
}




/* @obsolete ajFileBuffTraceFull
** @rename ajFilebuffTraceFull
*/
__deprecated void ajFileBuffTraceFull(const AjPFilebuff buff, size_t nlines,
			  size_t nfree)
{
    ajFilebuffTraceFull(buff, nlines, nfree);

    return;
}




/* @func ajFilebuffTraceTitle **************************************************
**
** Writes the full contents of a buffered file to the debug file
**
** @param [r] buff [const AjPFilebuff] Buffered file.
** @param [r] title [const char*] Report title
** @return [void]
** @@
******************************************************************************/

void ajFilebuffTraceTitle(const AjPFilebuff buff, const char* title)
{
    ajint i;
    AjPFilebufflist line;
    ajint last = 0;

    ajDebug("=== File Buffer: %s ===\n", title);
    line = buff->Lines;

    for(i=1; line; i++)
    {
	ajStrAssignS(&fileTmpStr, line->Line);
	ajStrRemoveLastNewline(&fileTmpStr);

	if(line == buff->Curr)
	    ajDebug("* %x %S\n", line->Line, fileTmpStr);
	else
	    ajDebug("  %x %S\n", line->Line, fileTmpStr);

	line = line->Next;
    }

    line = buff->Freelines;

    for(i=1; line;  i++)
    {
	if(line == buff->Freelast)
            last = i;

	if(line == buff->Freelast)
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




/* @obsolete ajFileBuffPrint
** @rename ajFilebuffTraceTitle
*/
__deprecated void ajFileBuffPrint(const AjPFilebuff buff, const char* title)
{
    ajFilebuffTraceTitle(buff, title);

    return;
}




/* @funcstatic fileBuffLineDel ************************************************
**
** Delete a line from a buffer.
**
** @param [u] buff [AjPFilebuff] File buffer
** @return [void]
******************************************************************************/

static void fileBuffLineDel(AjPFilebuff buff)
{
    AjPFilebufflist saveprev;

    if(!buff->Curr)
	return;

    ajDebug("fileBuffLineDel removing line [%d%s%d], '%S' len %d\n",
	     buff->Pos, SLASH_STRING, buff->Size, buff->Curr->Line,
	     ajStrGetLen(buff->Curr->Line));

    /* first line */
    if(!buff->Prev)
    {
	buff->Prev = buff->Lines;
	buff->Curr = buff->Lines = buff->Lines->Next;
	ajStrDel(&buff->Prev->Line);
	AJFREE(buff->Prev);
	--buff->Size;

	return;
    }

    /* last line */
    if(!buff->Curr->Next)
    {
	saveprev = buff->Prev;
	buff->Prev = buff->Lines;

	while(buff->Prev && buff->Prev->Next != saveprev)
	    buff->Prev = buff->Prev->Next;

	saveprev->Next = NULL;

	ajStrDel(&buff->Curr->Line);
	AJFREE(buff->Curr);
	buff->Curr = NULL;
	buff->Last = saveprev;
	--buff->Size;
	buff->Pos = buff->Size;

	return;
    }

    buff->Prev->Next = buff->Curr->Next;
    ajStrDel(&buff->Curr->Line);
    AJFREE(buff->Curr);
    buff->Curr = buff->Prev->Next;
    --buff->Size;

    return;
}




/* @funcstatic fileBuffLineNext ***********************************************
**
** Steps the Curr pointer to the next line in a buffer.
**
** Not for use when reading from a file. This steps through the buffer
**
** @param [u] buff [AjPFilebuff] File buffer
** @return [AjBool] ajTrue if there was another line
******************************************************************************/

static AjBool fileBuffLineNext(AjPFilebuff buff)
{
    if(buff->Pos < buff->Size)
    {
	buff->Prev = buff->Curr;
	buff->Curr = buff->Curr->Next;
	buff->Pos++;

	return ajTrue;
    }

    return ajFalse;
}




/* @datasection [AjPOutfile] output file object *******************************
**
** Function is for manipulating output files
** and their attributes.
**
** @nam2rule Outfile
*/




/* @section Outfile Constructors ********************************************
**
** @fdata [AjPOutfile]
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
** @nam3rule New Output file object constructor
** @suffix Name File name
** @suffix S File name as a string
**
** @argrule S name [const AjPStr] Output filename
**
** @valrule * [AjPOutfile] Output file
**
** @fcategory new
**
******************************************************************************/




/* @func ajOutfileNewNameS ****************************************************
**
** Creates a new formatted output file object with a specified name.
**
** 'stdout' and 'stderr' are special names for standard output and
** standard error respectively.
**
** @param [r] name [const AjPStr] File name.
** @return [AjPOutfile] New output file object.
** @@
******************************************************************************/

AjPOutfile ajOutfileNewNameS(const AjPStr name)
{
    AjPOutfile thys;

    AJNEW0(thys);
    thys->File = ajFileNewOutNameS(name);

    if(!thys->File)
	return NULL;

    return thys;
}




/* @obsolete ajOutfileNew
** @rename ajOutfileNewNameS
*/
__deprecated AjPOutfile ajOutfileNew(const AjPStr name)
{
    return ajOutfileNewNameS(name);
}




/* @section Outfile Destructors ***********************************************
**
** @fdata [AjPOutfile]
**
** Destruction is achieved by closing the file.
**
** Unlike ANSI C, there are tests to ensure a file is not closed twice.
**
** @nam3rule Close Close file and delete output file object
**
** @argrule * Pfile [AjPOutfile*] Output file object pointer
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajOutfileClose *******************************************************
**
** Close and free an outfile object.
**
** @param [d] Pfile [AjPOutfile*] Output file.
** @return [void]
** @@
******************************************************************************/

void ajOutfileClose(AjPOutfile* Pfile)
{
    AjPOutfile thys;

    thys = Pfile ? *Pfile : 0;

    if(!Pfile)
	return;

    if(!*Pfile)
	return;

    fileClose(thys->File);
    AJFREE(thys->File);
    ajStrDel(&thys->Type);
    ajStrDel(&thys->Formatstr);
    AJFREE(*Pfile);

    return;
}




/* @obsolete ajOutfileDel
** @rename ajOutfileClose
*/
__deprecated void ajOutfileDel(AjPOutfile* pthis)
{
    ajOutfileClose(pthis);
    return;
}




/* @section Outfile Casts *****************************************************
**
** @fdata [AjPOutfile]
**
** These functions examine the contents of an outfile object and return some
** derived information. Some of them provide access to the internal
** components of a file object. They are provided for programming convenience
** but should be used with caution.
**
** @nam3rule Get Return an output file attribute value
** @nam4rule GetFile Return the file object
** @nam4rule GetFileptr Return the C file pointer
** @nam4rule GetFormat Return the output format name
**
** @argrule * file [const AjPOutfile] Output file object
**
** @valrule GetFile [AjPFile] File object
** @valrule GetFileptr [FILE*] C file pointer
** @valrule GetFormat [const AjPStr] C file pointer
**
** @fcategory cast
**
******************************************************************************/




/* @func ajOutfileGetFile ******************************************************
**
** Returns the AjPFile for an AjPOutfile object
**
** @param [r] file [const AjPOutfile] Outfile object
** @return [AjPFile] AjPFile object
******************************************************************************/

AjPFile ajOutfileGetFile (const AjPOutfile file)
{
    return file->File;
}




/* @obsolete ajOutfileFile
** @rename ajOutfileGetFile
*/
__deprecated AjPFile ajOutfileFile (const AjPOutfile thys)
{
    return thys->File;
}




/* @func ajOutfileGetFileptr **************************************************
**
** Returns the C FILE* for an AjPOutfile object
**
** @param [r] file [const AjPOutfile] Outfile object
** @return [FILE*] C file pointer
******************************************************************************/

FILE* ajOutfileGetFileptr (const AjPOutfile file)
{
    return file->File->fp;
}




/* @obsolete ajOutfileFp
** @rename ajOutfileGetFileptr
*/
__deprecated FILE* ajOutfileFp (const AjPOutfile thys)
{
    return ajOutfileGetFileptr(thys);
}




/* @func ajOutfileGetFormat ***************************************************
**
** Returns the file format name for an AjPOutfile object
**
** @param [r] file [const AjPOutfile] Outfile object
** @return [const AjPStr] Format name
******************************************************************************/

const AjPStr ajOutfileGetFormat (const AjPOutfile file)
{
    return file->Formatstr;
}




/* @obsolete ajOutfileFormat
** @rename ajOutfileGetFormat
*/
__deprecated AjPStr ajOutfileFormat (const AjPOutfile thys)
{
    return thys->Formatstr;
}




/* @datasection [AjPStr] Filename functions
**
** Functions operating on strings containing filenames
**
** @nam2rule Filename Operations on filename strings
**
******************************************************************************/




/* @section Filename tests
**
** @fdata [AjPStr]
**
** Tests on filenames and searching the file system
**
** @nam3rule Exists tests whether file exists in current directory
** @nam4rule Dir Tests whether file exists and is a directory
** @nam4rule Exec Tests whether file exists and is executable by the user
** @nam4rule Read Tests whether file exists and is readable by the user
** @nam4rule Write Tests whether file exists and is writable by the user
** @nam3rule Get Return a filename element
** @nam4rule GetSize Return the file size
** @nam3rule Has Filename contains element
** @nam4rule Ext Filename extension
** @nam4rule Path Directory path
** @nam3rule Test Test a filename
** @nam4rule TestExclude Test a filename against a list of files to include.
**                       Any file not in the list is excluded.
** @nam4rule TestInclude Test a filename against a list of files to exclude
**                       Any file not in the list is included.
** @suffix Path Include the file path in tests
**
** @argrule * filename [const AjPStr] Filename string
** @argrule Stat mode [ajint] Mode
** @argrule Test exclude [const AjPStr] Excluded filenames wildcard
** @argrule Test include [const AjPStr] Included filenames wildcard
**
** @valrule Exists [AjBool] True if file exists with any requested access
** @valrule GetSize [ajlong] File size or -1 if not found
** @valrule Has [AjBool] True if file has attribute
** @valrule Test [AjBool] True if file passed test
** @fcategory use
**
******************************************************************************/




/* @func ajFilenameExists ******************************************************
**
** Returns true if file exists and is read or write or executable by the user
** as determined by AJ_FILE_R AJ_FILE_W AJ_FILE_X file modes
**
** @param [r] filename [const AjPStr] Filename.
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFilenameExists(const AjPStr filename)
{
#if defined(AJ_IRIXLF)
    struct stat64 buf;
#else
    struct stat buf;
#endif

#if defined(AJ_IRIXLF)
    if(!stat64(ajStrGetPtr(filename), &buf))
#else
    if(!stat(ajStrGetPtr(filename), &buf))
#endif
        return ajTrue;

    return ajFalse;
}




/* @func ajFilenameExistsDir ***************************************************
**
** Returns true if file exists and is a directory
**
** @param [r] filename [const AjPStr] Filename.
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFilenameExistsDir(const AjPStr filename)
{
#if defined(AJ_IRIXLF)
    struct stat64 buf;
#else
    struct stat buf;
#endif

#if defined(AJ_IRIXLF)
    if(!stat64(ajStrGetPtr(filename), &buf))
#else
    if(!stat(ajStrGetPtr(filename), &buf))
#endif

    if((ajuint)buf.st_mode & AJ_FILE_DIR)
        return ajTrue;

    return ajFalse;
}




/* @func ajFilenameExistsExec **************************************************
**
** Returns true if file exists and is executable by the user
**
** @param [r] filename [const AjPStr] Filename.
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFilenameExistsExec(const AjPStr filename)
{
#if defined(AJ_IRIXLF)
    struct stat64 buf;
#else
    struct stat buf;
#endif

#if defined(AJ_IRIXLF)
    if(!stat64(ajStrGetPtr(filename), &buf))
#else
    if(!stat(ajStrGetPtr(filename), &buf))
#endif

    if((ajuint)buf.st_mode & AJ_FILE_X)
        return ajTrue;

    return ajFalse;
}




/* @func ajFilenameExistsRead **************************************************
**
** Returns true if file exists and is readable by the user
**
** @param [r] filename [const AjPStr] Filename.
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFilenameExistsRead(const AjPStr filename)
{
#if defined(AJ_IRIXLF)
    struct stat64 buf;
#else
    struct stat buf;
#endif

#if defined(AJ_IRIXLF)
    if(!stat64(ajStrGetPtr(filename), &buf))
#else
    if(!stat(ajStrGetPtr(filename), &buf))
#endif

    if((ajuint)buf.st_mode & AJ_FILE_R)
        return ajTrue;

    return ajFalse;
}




/* @func ajFilenameExistsWrite ************************************************
**
** Returns true if file exists and is writable by the user
**
** @param [r] filename [const AjPStr] Filename.
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFilenameExistsWrite(const AjPStr filename)
{
#if defined(AJ_IRIXLF)
    struct stat64 buf;
#else
    struct stat buf;
#endif

#if defined(AJ_IRIXLF)
    if(!stat64(ajStrGetPtr(filename), &buf))
#else
    if(!stat(ajStrGetPtr(filename), &buf))
#endif

    if((ajuint)buf.st_mode & AJ_FILE_W)
        return ajTrue;

    return ajFalse;
}




/* @obsolete ajFileStat
** @remove Use ajFilenameExists ajFileNameExistsRead ajFilenameExistsWrite
**             or ajFileNameExistsExec
*/
__deprecated AjBool ajFileStat(const AjPStr fname, ajint mode)
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




/* @obsolete ajFileNameValid
** @rename ajFilenameExistsRead
*/
__deprecated AjBool ajFileNameValid (const AjPStr fname)
{
    return ajFilenameExistsRead(fname);
}




/* @func ajFilenameGetSize ****************************************************
**
** Returns the length of a file
**
** @param [r] filename [const AjPStr] Filename.
** @return [ajlong] length or -1 if file doesn't exist
** @@
******************************************************************************/

ajlong ajFilenameGetSize(const AjPStr filename)
{
#if defined(AJ_IRIXLF)
    struct stat64 buf;
#else
    struct stat buf;
#endif

#if defined(AJ_IRIXLF)
    if(!stat64(ajStrGetPtr(filename), &buf))
	return (ajlong)buf.st_size;
#else
    if(!stat(ajStrGetPtr(filename), &buf))
	return (ajlong)buf.st_size;
#endif

    return -1;
}




/* @obsolete ajFileLength
** @rename ajFilenameGetSize
*/
__deprecated ajlong ajFileLength(const AjPStr fname)
{
    return ajFilenameGetSize(fname);
}




/* @func ajFilenameHasPath *****************************************************
**
** Tests whether a filename includes a directory specification.
**
** @param [r] filename [const AjPStr] File name.
** @return [AjBool] ajTrue if directory filename syntax was found
** @@
******************************************************************************/

AjBool ajFilenameHasPath(const AjPStr filename)
{
    if(ajStrFindC(filename, SLASH_STRING) < 0)
	return ajFalse;

    return ajTrue;
}




/* @obsolete ajFileHasDir
** @rename ajFilenameHasPath
*/
__deprecated AjBool ajFileHasDir(const AjPStr name)
{
    return ajFilenameHasPath(name);
}




/* @func ajFilenameTestExclude ************************************************
**
** Tests a filename against wildcard
** lists of file names to be included and excluded.
**
** The path (if any) is removed before checking.
**
** By default files are excluded. The inclusion list is used to select
** files, and the exclusion list is then used to exclude selected
** files again.
**
** @param [r] filename [const AjPStr] File to test
** @param [r] exclude [const AjPStr] List of wildcard names to exclude
** @param [r] include [const AjPStr] List of wildcard names to include
** @return [AjBool] ajTrue if the filename is accepted.
** @@
******************************************************************************/

AjBool ajFilenameTestExclude(const AjPStr filename,
			     const AjPStr exclude,
			     const AjPStr include)
{
    AjBool ret = ajFalse;

    AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    
    ajStrAssignS(&fileNameTmp, filename);
    ajFileDirTrim(&fileNameTmp);
    
    if(ajStrGetLen(include))
    {
	ajStrTokenAssignC(&handle, include, " \t,;\n");

	while(ajStrTokenNextParse(&handle, &token))
	    if(ajStrMatchWildS(filename, token) ||
	       ajStrMatchWildS(fileNameTmp, token))
		ret = ajTrue;

	ajStrTokenReset(&handle);
    }

    if(ajStrGetLen(exclude))
    {	/* nokeep, test exclude last */
	ajStrTokenAssignC(&handle, exclude, " \t,;\n");

	while(ajStrTokenNextParse(&handle, &token))
	    if(ajStrMatchWildS(filename, token) ||
	       ajStrMatchWildS(fileNameTmp, token))
		ret = ajFalse;

	ajStrTokenReset(&handle);
    }

    ajStrTokenDel(&handle);
    ajStrDel(&token);

    return ret;
}




/* @func ajFilenameTestExcludePath ********************************************
**
** Tests a full path filename against wildcard
** lists of file names to be included and excluded.
**
** The full path is retained and included in the tests.
**
** By default files are excluded. The inclusion list is used to select
** files, and the exclusion list is then used to exclude selected
** files again.
**
** @param [r] filename [const AjPStr] File to test
** @param [r] exclude [const AjPStr] List of wildcard names to exclude
** @param [r] include [const AjPStr] List of wildcard names to include
** @return [AjBool] ajTrue if the filename is accepted.
** @@
******************************************************************************/

AjBool ajFilenameTestExcludePath(const AjPStr filename,
				 const AjPStr exclude,
				 const AjPStr include)
{
    AjBool ret = ajFalse;

    AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    
    if(ajStrGetLen(include))
    {
	ajStrTokenAssignC(&handle, include, " \t,;\n");

	while(ajStrTokenNextParse(&handle, &token))
	    if(ajStrMatchWildS(filename, token))
		ret = ajTrue;

	ajStrTokenReset(&handle);
    }

    if(ajStrGetLen(exclude))
    {	/* nokeep, test exclude last */
	ajStrTokenAssignC(&handle, exclude, " \t,;\n");

	while(ajStrTokenNextParse(&handle, &token))
	    if(ajStrMatchWildS(filename, token))
		ret = ajFalse;

	ajStrTokenReset(&handle);
    }

    ajStrTokenDel(&handle);
    ajStrDel(&token);

    return ret;
}




/* @func ajFilenameTestInclude ************************************************
**
** Tests a filename against wildcard
** lists of file names to be included and excluded.
**
** By default files are included. The exclusion list is used to trim
** out files, and the inclusion list is then used to add selected
** files again.
**
** @param [r] filename [const AjPStr] File to test
** @param [r] exclude [const AjPStr] List of wildcard names to exclude
** @param [r] include [const AjPStr] List of wildcard names to include
** @return [AjBool] ajTrue if the filename is accepted.
** @@
******************************************************************************/

AjBool ajFilenameTestInclude(const AjPStr filename,
			     const AjPStr exclude,
			     const AjPStr include)
{
    AjBool ret = ajTrue;

    AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    
    ajStrAssignS(&fileNameTmp, filename);
    ajFileDirTrim(&fileNameTmp);
    
    if(ajStrGetLen(exclude))
    {
	/* keep, so test exclude first */
	ajStrTokenAssignC(&handle, exclude, " \t,;\n");

	while(ajStrTokenNextParse(&handle, &token))
	    if(ajStrMatchWildS(filename, token) ||
	       ajStrMatchWildS(fileNameTmp, token))
		ret = ajFalse;

	ajStrTokenReset(&handle);
    }

    if(ajStrGetLen(include))
    {
	ajStrTokenAssignC(&handle, include, " \t,;\n");

	while(ajStrTokenNextParse(&handle, &token))
	    if(ajStrMatchWildS(filename, token) ||
	       ajStrMatchWildS(fileNameTmp, token))
		ret = ajTrue;

	ajStrTokenReset(&handle);
    }

    ajStrTokenDel(&handle);
    ajStrDel(&token);

    return ret;
}




/* @func ajFilenameTestIncludePath ********************************************
**
** Tests a filename against wildcard
** lists of file names to be included and excluded.
**
** By default files are included. The exclusion list is used to trim
** out files, and the inclusion list is then used to add selected
** files again.
**
** @param [r] filename [const AjPStr] File to test
** @param [r] exclude [const AjPStr] List of wildcard names to exclude
** @param [r] include [const AjPStr] List of wildcard names to include
** @return [AjBool] ajTrue if the filename is accepted.
** @@
******************************************************************************/

AjBool ajFilenameTestIncludePath(const AjPStr filename,
				 const AjPStr exclude,
				 const AjPStr include)
{
    AjBool ret = ajTrue;

    AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    
    if(ajStrGetLen(exclude))
    {
	/* keep, so test exclude first */
	ajStrTokenAssignC(&handle, exclude, " \t,;\n");

	while(ajStrTokenNextParse(&handle, &token))
	    if(ajStrMatchWildS(filename, token))
		ret = ajFalse;

	ajStrTokenReset(&handle);
    }

    if(ajStrGetLen(include))
    {
	ajStrTokenAssignC(&handle, include, " \t,;\n");

	while(ajStrTokenNextParse(&handle, &token))
	    if(ajStrMatchWildS(filename, token))
		ret = ajTrue;

	ajStrTokenReset(&handle);
    }

    ajStrTokenDel(&handle);
    ajStrDel(&token);

    return ret;
}




/* @obsolete ajFileTestSkip
** @remove Use ajFilenameTestExclude ajFilenameTestInclude and *Path
*/
__deprecated AjBool ajFileTestSkip(const AjPStr fullname,
				   const AjPStr exc, const AjPStr inc,
				   AjBool keep, AjBool ignoredirectory)
{
    if(ignoredirectory)
    {
        if(keep)
            return ajFilenameTestInclude(fullname, exc, inc);
        else
            return ajFilenameTestExclude(fullname, exc, inc);
    }

    if(keep)
        return ajFilenameTestIncludePath(fullname, exc, inc);

    return ajFilenameTestExcludePath(fullname, exc, inc);
}




/* @section filename modifiers
**
** @fdata [AjPStr]
**
** These functions overwrite a previous filename
**
** @nam3rule Replace Replace part of a filename
** @nam3rule Set     Set undefined part of a filename
** @nam4rule Tempname Set a new temporary filename
** @nam3rule Trim    Remove part of a filename
** @nam4rule TrimAll  Remove path and extension and trailing :identifier
**                    to leave the file basename
**
** @suffix Ext Filename extension
** @suffix Path Filename path
** @suffix C New character value
** @suffix S New string value
**
** @argrule * Pfilename [AjPStr*] Filename
** @argrule C txt [const char*] New character value
** @argrule S str [const AjPStr] New string value
**
** @valrule * [AjBool] True on success
**
** @fcategory modify
**
******************************************************************************/




/* @func ajFilenameReplaceExtC ************************************************
**
** Replaces the extension part of a filename
**
** @param [u] Pfilename [AjPStr*] Filename.
** @param [r] txt [const char*] New file extension
** @return [AjBool] ajTrue if the replacement succeeded.
** @@
******************************************************************************/

AjBool ajFilenameReplaceExtC(AjPStr* Pfilename, const char* txt)
{
    AjBool doext;
    char *p = NULL;

    doext = ajTrue;
    if(!txt || !*txt)
        doext = ajFalse;


    /*ajDebug("ajFilenameReplaceExtC '%S' '%s'\n", *Pfilename, txt);*/

    ajStrAssignS(&fileTmpStr,*Pfilename);

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
        ajStrAppendC(&fileTmpStr,txt);
    }

    ajStrAssignS(Pfilename,fileTmpStr);

    /*ajDebug("result '%S'\n", *Pfilename);*/

    return ajTrue;
}




/* @obsolete ajFileNameExtC
** @rename ajFilenameReplaceExtC
*/
__deprecated AjBool ajFileNameExtC(AjPStr* filename, const char* extension)
{
    return ajFilenameReplaceExtC(filename, extension);
}




/* @func ajFilenameReplaceExtS ************************************************
**
** Replaces the extension part of a filename
**
** @param [u] Pfilename [AjPStr*] Filename.
** @param [r] str [const AjPStr] New file extension
** @return [AjBool] ajTrue if the replacement succeeded.
** @@
******************************************************************************/

AjBool ajFilenameReplaceExtS(AjPStr* Pfilename, const AjPStr str)
{
    if(!str)
        return ajFilenameReplaceExtC(Pfilename, NULL);

    return ajFilenameReplaceExtC(Pfilename, ajStrGetPtr(str));
}




/* @obsolete ajFileNameExt
** @rename ajFilenameReplaceExtS
*/
__deprecated AjBool ajFileNameExt(AjPStr* filename, const AjPStr extension)
{
    return ajFilenameReplaceExtS(filename, extension);
}




/* @func ajFilenameReplacePathC **********************************************
**
** Sets the directory part of a filename
**
** @param [u] Pfilename [AjPStr*] Filename.
** @param [r] txt [const char*] Directory
** @return [AjBool] ajTrue if the replacement succeeded.
** @@
******************************************************************************/

AjBool ajFilenameReplacePathC(AjPStr* Pfilename, const char* txt)
{
    AjPStr tmpname = NULL;
    AjPStr tmpdir = NULL;

    if(!txt)
        return ajFalse;

    if(!fileFilenameExp)
#ifndef WIN32
        fileFilenameExp = ajRegCompC("(.*/)?([^/]+)$");
#else
        fileFilenameExp = ajRegCompC("(.*\\\\)?([^\\\\]+)$");
#endif

    if(ajRegExec(fileFilenameExp, *Pfilename))
    {
        ajRegSubI(fileFilenameExp, 1, &tmpdir);

        /* we already have a directory */
        if(ajStrGetLen(tmpdir))
        {
            ajStrDel(&tmpdir);
            return ajFalse;
        }

        ajRegSubI(fileFilenameExp, 2, &tmpname);

        if(txt[strlen(txt)-1] == SLASH_CHAR)
            ajFmtPrintS(Pfilename, "%s%S", txt, tmpname);
        else
            ajFmtPrintS(Pfilename, "%s%s%S", txt, SLASH_STRING,tmpname);

        ajStrDel(&tmpname);
    }

    return ajTrue;
}




/* @obsolete ajFileNameDirSetC
** @rename ajFilenameReplacePathC
*/
__deprecated AjBool ajFileNameDirSetC(AjPStr* filename, const char* dir)
{
    return ajFilenameReplacePathC(filename, dir);
}




/* @func ajFilenameReplacePathS ***********************************************
**
** Sets the directory part of a filename
**
** @param [u] Pfilename [AjPStr*] Filename.
** @param [r] str [const AjPStr] New directory
** @return [AjBool] ajTrue if the replacement succeeded.
** @@
******************************************************************************/

AjBool ajFilenameReplacePathS(AjPStr* Pfilename, const AjPStr str)
{
    if(!ajStrGetLen(str))
        return ajFalse;

    return ajFilenameReplacePathC(Pfilename, ajStrGetPtr(str));
}




/* @obsolete ajFileNameDirSet
** @rename ajFilenameReplacePathS
*/
__deprecated AjBool ajFileNameDirSet(AjPStr* filename, const AjPStr dir)
{
    return ajFilenameReplacePathS(filename, dir);
}




/* @obsolete ajFileSetDir
** @rename ajFilenameReplacePathS
*/
__deprecated AjBool ajFileSetDir (AjPStr *pname, const AjPStr dir)

{
    return ajFilenameReplacePathS(pname, dir);
}




/* @func ajFilenameSetExtC ****************************************************
**
** Sets the extension part of a filename
**
** @param [u] Pfilename [AjPStr*] Filename.
** @param [r] txt [const char*] New file extension
** @return [AjBool] ajTrue if the replacement succeeded.
** @@
******************************************************************************/

AjBool ajFilenameSetExtC(AjPStr* Pfilename, const char* txt)
{
    char *p = NULL;

    if(!txt || !*txt)
        return ajFalse;

    /*ajDebug("ajFilenameSetExtC '%S' '%s'\n", *Pfilename, txt);*/

    ajStrAssignS(&fileTmpStr,*Pfilename);

    /* Skip any directory path */
    p = strrchr(ajStrGetuniquePtr(&fileTmpStr),SLASH_CHAR);

    if (!p)
        p = ajStrGetuniquePtr(&fileTmpStr);

    ajStrAppendC(&fileTmpStr,".");
    ajStrAppendC(&fileTmpStr,txt);

    ajStrAssignS(Pfilename,fileTmpStr);

    /*ajDebug("result '%S'\n", *Pfilename);*/

    return ajTrue;
}




/* @func ajFilenameSetExtS ****************************************************
**
** Sets the extension part of a base filename
**
** @param [u] Pfilename [AjPStr*] Filename.
** @param [r] str [const AjPStr] New file extension
** @return [AjBool] ajTrue if the replacement succeeded.
** @@
******************************************************************************/

AjBool ajFilenameSetExtS(AjPStr* Pfilename, const AjPStr str)
{
    if(!str)
        return ajFalse;

    return ajFilenameSetExtC(Pfilename, ajStrGetPtr(str));
}




/* @func ajFilenameSetTempname ************************************************
**
** Returns an available temporary filename that can be opened for writing
** Filename will be of the form progname-time.randomnumber
** Tries 5 times to find a new filename. Returns ajFalse if not
** successful or the file cannot be opened for writing.
** This function returns only the filename, not a file pointer.
**
** @param [w] Pfilename [AjPStr*] Filename of new temporary file
** @return [AjBool] True on success
** @@
******************************************************************************/

AjBool ajFilenameSetTempname(AjPStr* Pfilename)
{
#if defined(AJ_IRIXLF)
    struct  stat64 buf;
#else
    struct  stat buf;
#endif
    ajint   retry;
    AjBool  ok;
    AjPFile outf;
    
    if(!fileTempFilename)
	fileTempFilename = ajStrNew();
    
    ajStrAssignC(&fileDirectory,".");
    ajDirnameFix(&fileDirectory);

    ajFmtPrintS(&fileTempFilename,
		"%S%S-%d.%d",fileDirectory,ajUtilGetProgram(),time(0),
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
			"%S%S-%d.%d",fileDirectory,ajUtilGetProgram(),time(0),
			ajRandomNumber());
	    --retry;
	}
    
    if(!retry)
    {
	ajDebug("Cannot find a unique filename [last try %S]\n",
		fileTempFilename);
	ok = ajFalse;
    }
    
    if(!(outf = ajFileNewOutNameS(fileTempFilename)))
    {
	ajDebug("Cannot write to file %S\n",fileTempFilename);
	ok = ajFalse;
    }
    else
    {
	ajFileClose(&outf);
	unlink(ajStrGetPtr(fileTempFilename));
    }
    
    ajStrAssignS(Pfilename,fileTempFilename);

    return ok;
}




/* @func ajFilenameSetTempnamePathC *******************************************
**
** Returns an available temporary filename that can be opened for writing
** Filename will be of the form progname-time.randomnumber
** Tries 5 times to find a new filename. Returns ajFalse if not
** successful or the file cannot be opened for writing.
** This function returns only the filename, not a file pointer.
**
** @param [w] Pfilename [AjPStr*] Filename of new temporary file
** @param [r] txt [const char*] Directory path
** @return [AjBool] True on success
** @@
******************************************************************************/

AjBool ajFilenameSetTempnamePathC(AjPStr* Pfilename, const char* txt)
{
#if defined(AJ_IRIXLF)
    struct  stat64 buf;
#else
    struct  stat buf;
#endif
    ajint   retry;
    AjBool  ok;
    AjPFile outf;
    
    if(!fileTempFilename)
	fileTempFilename = ajStrNew();
    
    ajStrAssignC(&fileDirectory,txt);
    ajDirnameFix(&fileDirectory);

    ajFmtPrintS(&fileTempFilename,
		"%S%S-%d.%d",fileDirectory,ajUtilGetProgram(),time(0),
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
			"%S%S-%d.%d",fileDirectory,ajUtilGetProgram(),time(0),
			ajRandomNumber());
	    --retry;
	}
    
    if(!retry)
    {
	ajDebug("Cannot find a unique filename [last try %S]\n",
		fileTempFilename);
	ok = ajFalse;
    }
    
    if(!(outf = ajFileNewOutNameS(fileTempFilename)))
    {
	ajDebug("Cannot write to file %S\n",fileTempFilename);
	ok = ajFalse;
    }
    else
    {
	ajFileClose(&outf);
	unlink(ajStrGetPtr(fileTempFilename));
    }
    
    ajStrAssignS(Pfilename,fileTempFilename);

    return ok;
}




/* @func ajFilenameSetTempnamePathS *******************************************
**
** Returns an available temporary filename that can be opened for writing
** Filename will be of the form progname-time.randomnumber
** Tries 5 times to find a new filename. Returns ajFalse if not
** successful or the file cannot be opened for writing.
** This function returns only the filename, not a file pointer.
**
** @param [w] Pfilename [AjPStr*] Filename of new temporary file
** @param [r] str [const AjPStr] Directory path
** @return [AjBool] True on success
** @@
******************************************************************************/

AjBool ajFilenameSetTempnamePathS(AjPStr* Pfilename, const AjPStr str)
{
#if defined(AJ_IRIXLF)
    struct  stat64 buf;
#else
    struct  stat buf;
#endif
    ajint   retry;
    AjBool  ok;
    AjPFile outf;
    
    if(!fileTempFilename)
	fileTempFilename = ajStrNew();
    
    ajStrAssignS(&fileDirectory,str);
    ajDirnameFix(&fileDirectory);

    ajFmtPrintS(&fileTempFilename,
		"%S%S-%d.%d",fileDirectory,ajUtilGetProgram(),time(0),
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
			"%S%S-%d.%d",fileDirectory,ajUtilGetProgram(),time(0),
			ajRandomNumber());
	    --retry;
	}
    
    if(!retry)
    {
	ajDebug("Cannot find a unique filename [last try %S]\n",
		fileTempFilename);
	ok = ajFalse;
    }
    
    if(!(outf = ajFileNewOutNameS(fileTempFilename)))
    {
	ajDebug("Cannot write to file %S\n",fileTempFilename);
	ok = ajFalse;
    }
    else
    {
	ajFileClose(&outf);
	unlink(ajStrGetPtr(fileTempFilename));
    }
    
    ajStrAssignS(Pfilename,fileTempFilename);

    return ok;
}




/* @obsolete ajFileTempName
** @remove Use ajFilenameSetTempname or ajFilenameSetTempnamePath
*/
__deprecated const char* ajFileTempName(const char *dir)
{
  AjPStr tmpstr = NULL;

  (void) dir;
  ajFilenameSetTempname(&tmpstr);

  return ajStrGetPtr(tmpstr);
}




/* @func ajFilenameTrimAll *****************************************************
**
** Truncates a filename to a basic file name.
**
** @param [uP] Pfilename [AjPStr*] File name
** @return [AjBool] ajTrue on success, and returns a filename.
**                  ajFalse on failure, and returns an empty string.
** @@
******************************************************************************/

AjBool ajFilenameTrimAll(AjPStr* Pfilename)
{
    /* entryname at end */
    if(!fileEntryExp)
	fileEntryExp = ajRegCompC(":([A-Za-z0-9_-]+)$");

    if(ajRegExec(fileEntryExp, *Pfilename))
    {
	ajRegSubI(fileEntryExp, 1, Pfilename);
	return ajTrue;
    }

    /* name.ext */
    if(!fileFileExp)
	fileFileExp = ajRegCompC("([A-Za-z0-9_-]+)[.][A-Za-z0-9_-]+$");

    if(ajRegExec(fileFileExp, *Pfilename))
    {
	ajRegSubI(fileFileExp, 1, Pfilename);
	return ajTrue;
    }

    /* last valid word */
    if(!fileRestExp)
	fileRestExp = ajRegCompC("([A-Za-z0-9_-]+)[^A-Za-z0-9_-]*$");

    if(ajRegExec(fileRestExp, *Pfilename))
    {
	ajRegSubI(fileRestExp, 1, Pfilename);
	return ajTrue;
    }

    ajStrAssignClear(Pfilename);

    return ajFalse;
}




/* @obsolete ajFileNameShorten
** @rename ajFilenameTrimAll
*/
__deprecated AjBool ajFileNameShorten(AjPStr* fname)
{
    return ajFilenameTrimAll(fname);
}




/* @func ajFilenameTrimExt *****************************************************
**
** Trims the extension (if any) from a filename
**
** @param [u] Pfilename [AjPStr*] Filename
** @return [AjBool] ajTrue is there was an extension
******************************************************************************/

AjBool ajFilenameTrimExt(AjPStr* Pfilename)
{
    ajint i;
    ajint len;

    if(!ajStrGetLen(*Pfilename))
	return ajFalse;

    len = ajStrGetLen(*Pfilename);
    i = ajStrFindlastC(*Pfilename, ".");

    if(i < 0)
	return ajFalse;

    ajStrCutEnd(Pfilename, (len-i));

    return ajTrue;
}




/* @obsolete ajFileExtnTrim
** @rename ajFilenameTrimExt
*/
__deprecated AjBool ajFileExtnTrim(AjPStr* name)
{
    return ajFilenameTrimExt(name);
}




/* @func ajFilenameTrimPath ***************************************************
**
** Trims the directory path (if any) from a filename
**
** @param [u] Pfilename [AjPStr*] Filename
** @return [AjBool] ajTrue is there was a directory
******************************************************************************/

AjBool ajFilenameTrimPath(AjPStr* Pfilename)
{
    ajint i;

    if(!ajStrGetLen(*Pfilename))
	return ajFalse;

    i = ajStrFindlastC(*Pfilename, SLASH_STRING);

    if(i < 0)
	return ajFalse;

    ajStrCutStart(Pfilename, i+1);

    return ajTrue;
}




/* @obsolete ajFileNameTrim
** @rename ajFilenameTrimPath
*/
__deprecated AjBool ajFileNameTrim(AjPStr* fname)
{
    char *p;

    if((p = strrchr(ajStrGetPtr(*fname),(ajint)SLASH_CHAR)))
    {
	ajStrAssignC(&fileTmpStr,p+1);
	ajStrAssignS(fname,fileTmpStr);
    }

    return ajTrue;
}




/* @obsolete ajFileDirTrim
** @rename ajFilenameTrimPath
*/
__deprecated AjBool ajFileDirTrim(AjPStr* name)
{
    return ajFilenameTrimPath(name);
}




/* @func ajFilenameTrimPathExt *************************************************
**
** Trims the directory path (if any) and extension (if any) from a filename.
**
** @param [u] Pfilename [AjPStr*] Filename
** @return [AjBool] ajTrue is there was a directory path or extension.
******************************************************************************/

AjBool ajFilenameTrimPathExt(AjPStr* Pfilename)
{
    ajint i;
    ajint j;
    ajint len;
    
    if(!ajStrGetLen(*Pfilename))
	return ajFalse;

    i = ajStrFindlastC(*Pfilename, SLASH_STRING);

    if(i >= 0)
	ajStrCutStart(Pfilename, i+1);

    len = ajStrGetLen(*Pfilename);
    j = ajStrFindlastC(*Pfilename, ".");

    if(j >= 0)
	ajStrCutEnd(Pfilename, (len-j));

    if((i < 0) && (j < 0))
	return ajFalse;

    return ajTrue;
}




/* @obsolete ajFileDirExtnTrim
** @rename ajFilenameTrimPathExt
*/
__deprecated AjBool ajFileDirExtnTrim(AjPStr* name)
{
    return ajFilenameTrimPathExt(name);
}




/* @datasection [AjPStr] Directory name functions
**
** Functions operating on strings containing directory names
**
** @nam2rule Dirname Operations on directory name strings
**
******************************************************************************/




/* @section directory name modifiers
**
** @fdata [AjPStr]
**
** These functions overwrite a previous filename
**
** @nam3rule Fill    Fill in all parts of incomplete directory name
** @nam3rule Fix     Fix incomplete directory name
** @nam4rule FixExists Fix incomplete directory name and test it can be opened
** @nam3rule Up      Change to parent directory
**
** @suffix Ext Filename extension
** @suffix Path Filename path
** @suffix C New character value
** @suffix S New string value
**
** @argrule * Pdirname [AjPStr*] Filename
** @argrule C txt [const char*] New character value
** @argrule S str [const AjPStr] New string value
**
** @valrule * [void]
** @valrule *Exists [AjBool] True on success
** @valrule *Fill [AjBool] True on success
** @valrule *Up [AjBool] True on success
**
** @fcategory modify
**
******************************************************************************/




/* @func ajDirnameFillPath ****************************************************
**
** Checks that a string is a valid directory, and makes sure it has the
** full path definition.
**
** @param [u] Pdirname [AjPStr*] Directory path
** @return [AjBool] true if a valid directory.
** @@
******************************************************************************/

AjBool ajDirnameFillPath(AjPStr* Pdirname)
{
    DIR* odir;
    AjPStr cwdpath = NULL;

    ajDebug("ajDirnameFillPath '%S'\n", *Pdirname);

    /* appends trailing slash if needed */
    odir = fileOpenDir(Pdirname);

    if(!odir)
	return ajFalse;

    free(odir);

    ajDebug("So far '%S'\n", *Pdirname);

    /* full path already */
    if(*ajStrGetPtr(*Pdirname) == SLASH_CHAR)
	return ajTrue;

    /* current directory */
    if(ajStrMatchC(*Pdirname, CURRENT_DIR))
    {
        ajStrAssignS(Pdirname, ajFileValueCwd());
	ajDebug("Current '%S'\n", *Pdirname);

	return ajTrue;
    }

    /*  going up */
    ajStrAssignS(&cwdpath, ajFileValueCwd());

    while(ajStrPrefixC(*Pdirname, UP_DIR))
    {
	ajDirnameUp(&cwdpath);
	ajStrKeepRange(Pdirname, 3, -1);
	ajDebug("Going up '%S' '%S'\n", *Pdirname, cwdpath);
    }

    ajStrInsertS(Pdirname, 0, cwdpath);

    ajDebug("Full path '%S'\n", *Pdirname);

    ajStrDel(&cwdpath);

    return ajTrue;
}




/* @obsolete ajFileDirPath
** @rename ajDirnameFillPath
*/
__deprecated AjBool ajFileDirPath(AjPStr* dir)
{
    return ajDirnameFillPath(dir);
}




/* @func ajDirnameFix *********************************************************
**
** If the directory name has no trailing slash (on Unix) then one is
** added. This is why the directory name must be writable.
**
** @param [u] Pdirname [AjPStr*] Directory name.
** @return [void]
** @@
******************************************************************************/

void ajDirnameFix(AjPStr* Pdirname)
{
    if(ajStrGetCharLast(*Pdirname) != SLASH_CHAR)
	ajStrAppendC(Pdirname, SLASH_STRING);

    return;
}




/* @obsolete ajFileDirFix
** @rename ajDirnameFix
*/
__deprecated void ajFileDirFix(AjPStr* dir)
{
    ajDirnameFix(dir);

    return;
}




/* @func ajDirnameFixExists ****************************************************
**
** Checks that a string is a valid existing directory, and appends a
** trailing '/' if it is missing.
**
** @param [u] Pdirname [AjPStr*] Directory path
** @return [AjBool] true if a valid directory.
** @@
******************************************************************************/

AjBool ajDirnameFixExists(AjPStr* Pdirname)
{
    DIR* odir;

    odir = fileOpenDir(Pdirname);	/* appends trailing slash if needed */

    if(!odir)
	return ajFalse;

    closedir(odir);

    return ajTrue;
}




/* @obsolete ajFileDir
** @rename ajDirnameFixExists
*/
__deprecated AjBool ajFileDir(AjPStr* dir)
{
    return ajDirnameFixExists(dir);
}




/* @func ajDirnameUp **********************************************************
**
** Changes directory name to one level up
**
** @param [u] Pdirname [AjPStr*] Directory name.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajDirnameUp(AjPStr* Pdirname)
{
    AjPStr tmpdir = NULL;
    AjBool modded = ajFalse;
    
    const char *p;
    const char *q;
    ajint len;
    
    ajStrAssignS(&tmpdir, *Pdirname);
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
	    ajStrAssignSubC(Pdirname,p,0,q-p);
	    modded = ajTrue;
	}
   }
    
    ajStrDel(&tmpdir);

    return modded;
}




/* @obsolete ajFileDirUp
** @rename ajDirnameUp
*/
__deprecated AjBool ajFileDirUp(AjPStr* dir)
{
    return ajDirnameUp(dir);
}




/* @section directory functions
**
** @fdata [AjPStr]
**
** Functions using a directory name
**
** @nam3rule Print Print directory or filenames
** @nam4rule Recursive Step through subdirectories
**
** @suffix Ignore Process list of sub directory names to ignore
** @argrule * path [const AjPStr] directory name
** @argrule Ignore ignorelist [AjPList] List of names to ignore
** @argrule Print outfile [AjPFile] Output file
**
** @valrule * [void]
**
** @fcategory use
**
******************************************************************************/




/* @func ajDirnamePrintRecursiveIgnore ***************************************
**
** Recursively scan through a directory, printing directory and file names
**
** @param [r] path [const AjPStr] Directory to scan
** @param [u] ignorelist [AjPList] List of directories to ignore
** @param [u] outfile [AjPFile] File for "show" results (or NULL)
**
** @return [void]
** @@
******************************************************************************/

void ajDirnamePrintRecursiveIgnore(const AjPStr path,
				   AjPList ignorelist, AjPFile outfile)
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

    AjPStr tstr = NULL;
    
    AjBool flag;
    AjPStr tpath = NULL;
    AjPList plist = NULL;
    
#ifdef _POSIX_C_SOURCE
    char buf[sizeof(struct dirent)+MAXNAMLEN];
#endif
    
    tpath = ajStrNewS(path);
    
    ajFmtPrintF(outfile,"\n\nDIRECTORY: %S\n\n",path);

    if(!ajDirnameFixExists(&tpath))
    {
	ajStrDel(&tpath);

	return;
    }


    if(!(indir=opendir(ajStrGetPtr(tpath))))
    {
	ajStrDel(&tpath);

	return;
    }
    
    
    s = ajStrNew();
    dirs = ajListNew();
    plist = ajListNew();
    
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
	if(ajFilenameExistsDir(s))
	{
	    /* Ignore selected directories */
	    if(ajListGetLength(ignorelist))
	    {
		flag = ajFalse;
		iter = ajListIterNewread(ignorelist);

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

	    if(!ajFilenameExistsRead(s) || !ajFilenameExistsExec(s))
		continue;

	    t = ajStrNewC(ajStrGetPtr(s));
	    ajListPushAppend(dirs,(void *)t);
	}
	else if(ajFilenameExistsRead(s))
	{
            tstr = ajStrNew();
            ajStrAssignC(&tstr,dp->d_name);
            ajListPush(plist, (void *)tstr);
            

	}
    }

    closedir(indir);

    ajListSort(plist,ajStrVcmp);
    while(ajListPop(plist,(void **)&tstr))
    {
        ajFmtPrintF(outfile,"  %S\n",tstr);
        ajStrDel(&tstr);
    }

    ajListFree(&plist);
    
    while(ajListPop(dirs,(void **)&t))
      {
	ajDirnamePrintRecursiveIgnore(t, ignorelist, outfile);
	ajStrDel(&t);
      }
    
    ajStrDel(&s);
    ajStrDel(&tpath);
    ajListFree(&dirs);


    return;
}




/* @datasection [AjPlist] Lists of filenames *********************************
**
** Functions building and managing lists of filenames
**
** @nam2rule Filelist
**
******************************************************************************/




/* @section File list addition
**
** @fdata [AjPlist]
**
** Functions that add filenames to a list
**
** @nam3rule Add Adds names to a list
** @nam4rule Path Directory path provided
** @nam5rule Wild Wildcard filename provided
** @suffix Recursive Process subdirectories
** @suffix Ignore List of subdirectories to be ignored
** @suffix Directory Directory object provided
** @suffix Dir Include directories in the list
** @nam4rule Listname List of files or lists
**
** @argrule * list [AjPList] Filename list
** @argrule Listname listname [const AjPStr] Commandline list of filenames
** @argrule Directory dir [const AjPDir] Directory object
** @argrule Path path [const AjPStr] Pathname of directory
** @argrule Wild wildname [const AjPStr] Wildcard filename to add
** @argrule Ignore ignorelist [AjPList] Subdirectory name list to ignore
**
** @valrule * [ajint] Number of entries added to the list
**
** @fcategory modify
**
******************************************************************************/




/* @func ajFilelistAddDirectory ************************************************
**
** Scan through a directory object returning all filenames that are
** not directories. Uses the file extension and any other attributes
** of the directory object. An empty string as a file extension accepts
** only files that have no extension.
**
** @param [u] list [AjPList] List for matching entries
** @param [r] dir [const AjPDir] Directory to scan
**
** @return [ajint] number of entries in list
** @@
******************************************************************************/

ajint ajFilelistAddDirectory(AjPList list,
                             const AjPDir dir)
{
    ajuint oldsize;
    DIR *indir;
#if defined(AJ_IRIXLF)
    struct dirent64 *dp;
#else
    struct dirent *dp;
#endif
    AjPStr f = NULL;
    AjPStr s = NULL;
    AjPStr t = NULL;
    AjPStr e = NULL;

    AjPStr tpath = NULL;
#ifdef _POSIX_C_SOURCE
    char buf[sizeof(struct dirent)+MAXNAMLEN];
#endif

    AjBool doprefix = AJFALSE;
    AjBool doextension = AJFALSE;

    if(!dir)
        return 0;

    if(dir->Prefix)
        doprefix = ajTrue;
    if(dir->Extension)
    {
        if(ajStrGetCharFirst(dir->Extension) == '.')
            e = ajStrNewS(dir->Extension);
        else if(ajStrGetLen(dir->Extension))
            ajFmtPrintS(&e, ".%S", dir->Extension);
            
        doextension = ajTrue;
    }
    
    oldsize = ajListGetLength(list);

    tpath = ajStrNewS(dir->Name);

    ajDebug("ajFilelistAddDir '%S' oldsize:%u\n",
            tpath, oldsize);

    if(!ajDirnameFixExists(&tpath))
    {
        ajDebug("... not a directory '%S'\n", tpath);
	ajStrDel(&tpath);

	return 0;
    }


    if(!(indir=opendir(ajStrGetPtr(tpath))))
    {
        ajDebug("... failed to open directory '%S'\n", tpath);
	ajStrDel(&tpath);

	return 0;
    }
    
    
    s = ajStrNew();
    f = ajStrNew();
    
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

        ajStrAssignC(&f,dp->d_name);

        if(doprefix && !ajStrPrefixS(f, dir->Prefix))
            continue;

        if(doextension)
        {
            if(e)
            {
                if(!ajStrSuffixS(f, e))
                    continue;
            }
            else
            {
                if(ajStrFindAnyK(f,'.') >= 0)
                    continue;
            }
        }

        ajStrAssignS(&s,tpath);
	ajStrAppendS(&s,f);

        if(ajFilenameExistsDir(s))
            continue;

        ajDebug("... add to list '%S'\n",
                s);
    
	t = ajStrNewS(s);
	ajListPushAppend(list,(void *)t);
        
    }
    closedir(indir);
    
    
    ajStrDel(&e);
    ajStrDel(&f);
    ajStrDel(&s);
    ajStrDel(&tpath);

    return ajListGetLength(list) - oldsize;
}




/* @obsolete ajFileScan
** @remove Use ajFilelistAddPathWild and others
*/
__deprecated ajint ajFileScan(const AjPStr path,
		 const AjPStr filename, AjPList *result,
		 AjBool show, AjBool dolist, AjPList *list,
		 AjPList rlist, AjBool recurs, AjPFile outf)
{
    (void)dolist;
    (void)list;

    if(show)
    {
        ajDirnamePrintRecursiveIgnore(path, rlist, outf);

        return 0;
    }

    if(recurs)
        return ajFilelistAddPathWildRecursiveIgnore(*result,
                                                    path, filename, rlist);

    return ajFilelistAddPathWild(*result, path, filename);
}




/* @func ajFilelistAddListname ************************************************
**
** Adds to a list of files that match a comma-separated string of
** filenames which can include wildcards or listfiles
**
** @param [u] list [AjPList] List of filenames
** @param [r] listname [const AjPStr] comma-separated filename list
**
** @return [ajint] Number of entries added to list
** @@
******************************************************************************/

ajint ajFilelistAddListname(AjPList list, const AjPStr listname)
{
    ajuint oldsize;
    AjPStr *fstr = NULL;
    ajint  ncl;
    ajint  i;
    ajint  rlevel = 0;

    oldsize = ajListGetLength(list);
    ncl = ajArrCommaList(listname,&fstr);

    for(i=0;i<ncl;++i)
    {
	fileListRecurs(fstr[i],list,&rlevel);
	ajStrDel(&fstr[i]);
    }

    AJFREE(fstr);

    return ajListGetLength(list) - oldsize;
}




/* @func ajFilelistAddPath ****************************************************
**
** Scan through a directory returning all filenames excluding directory names
**
** @param [u] list [AjPList] List for matching entries
** @param [r] path [const AjPStr] Directory to scan
**
** @return [ajint] number of entries in list
** @@
******************************************************************************/

ajint ajFilelistAddPath(AjPList list,
			const AjPStr path)
{
    ajuint oldsize;
    DIR *indir;
#if defined(AJ_IRIXLF)
    struct dirent64 *dp;
#else
    struct dirent *dp;
#endif
    AjPStr s = NULL;
    AjPStr t = NULL;
    AjPStr tpath = NULL;
#ifdef _POSIX_C_SOURCE
    char buf[sizeof(struct dirent)+MAXNAMLEN];
#endif

    oldsize = ajListGetLength(list);

    tpath = ajStrNewS(path);

    ajDebug("ajFilelistAddPath '%S' oldsize:%u\n",
            path, oldsize);

    if(!ajDirnameFixExists(&tpath))
    {
        ajDebug("... not a directory '%S'\n", tpath);
	ajStrDel(&tpath);

	return 0;
    }


    if(!(indir=opendir(ajStrGetPtr(tpath))))
    {
        ajDebug("... failed to open directory '%S'\n", tpath);
	ajStrDel(&tpath);

	return 0;
    }
    
    
    s = ajStrNew();
    
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
	ajStrAppendC(&s,dp->d_name);

        ajDebug("... testing '%S'\n",
                s);

        if(ajFilenameExistsDir(s))
        {
            ajDebug("... rejected: directory '%S'\n",
                s);
            continue;
        }
        
        ajDebug("... add to list '%S'\n",
                s);
    
	t = ajStrNewS(s);
	ajListPushAppend(list,(void *)t);
        
    }

        closedir(indir);
    
    
    ajStrDel(&s);
    ajStrDel(&tpath);

    return ajListGetLength(list) - oldsize;
}




/* @func ajFilelistAddPathDir *************************************************
**
** Scan through a directory returning all filenames and directory names
** except '.' and '..', including directories
**
** @param [u] list [AjPList] List for matching entries
** @param [r] path [const AjPStr] Directory to scan
**
** @return [ajint] number of entries in list
** @@
******************************************************************************/

ajint ajFilelistAddPathDir(AjPList list,
                           const AjPStr path)
{
    ajuint oldsize;
    DIR *indir;
#if defined(AJ_IRIXLF)
    struct dirent64 *dp;
#else
    struct dirent *dp;
#endif
    AjPStr s = NULL;
    AjPStr t = NULL;
    AjPStr tpath = NULL;
#ifdef _POSIX_C_SOURCE
    char buf[sizeof(struct dirent)+MAXNAMLEN];
#endif

    oldsize = ajListGetLength(list);

    tpath = ajStrNewS(path);

    ajDebug("ajFilelistAddPath '%S' oldsize:%u\n",
            path, oldsize);

    if(!ajDirnameFixExists(&tpath))
    {
        ajDebug("... not a directory '%S'\n", tpath);
	ajStrDel(&tpath);

	return 0;
    }


    if(!(indir=opendir(ajStrGetPtr(tpath))))
    {
        ajDebug("... failed to open directory '%S'\n", tpath);
	ajStrDel(&tpath);

	return 0;
    }
    
    
    s = ajStrNew();
    
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
	ajStrAppendC(&s,dp->d_name);

        ajDebug("... add to list '%S'\n",
                s);
    
	t = ajStrNewS(s);
	ajListPushAppend(list,(void *)t);
        
    }
    closedir(indir);
    
    
    ajStrDel(&s);
    ajStrDel(&tpath);

    return ajListGetLength(list) - oldsize;
}




/* @func ajFilelistAddPathWild ************************************************
**
** Scan through a directory returning all filenames matching a
** wildcard filename
**
** @param [w] list [AjPList] List for matching entries
** @param [r] path [const AjPStr] Directory to scan
** @param [r] wildname [const AjPStr] Filename to search for
**
** @return [ajint] number of entries in list
** @@
******************************************************************************/

 ajint ajFilelistAddPathWild(AjPList list,
			     const AjPStr path,
			     const AjPStr wildname)
{
    ajuint oldsize;
    DIR *indir;
#if defined(AJ_IRIXLF)
    struct dirent64 *dp;
#else
    struct dirent *dp;
#endif
    AjPStr s = NULL;
    AjPStr t = NULL;
    AjPStr tpath = NULL;
#ifdef _POSIX_C_SOURCE
    char buf[sizeof(struct dirent)+MAXNAMLEN];
#endif

    oldsize = ajListGetLength(list);

    tpath = ajStrNew();
    ajStrAssignS(&tpath,path);
    
    
    if(!ajDirnameFixExists(&tpath))
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
	ajStrAppendC(&s,dp->d_name);

        if(ajFilenameExistsDir(s))
            continue;

	if(ajCharMatchWildS(dp->d_name,wildname))
        {
	    t = ajStrNewS(s);
	    ajListPushAppend(list,(void *)t);
        }
    }
    closedir(indir);
    
    
    ajStrDel(&s);
    ajStrDel(&tpath);

    return ajListGetLength(list) - oldsize;
}




/* @func ajFilelistAddPathWildDir *********************************************
**
** Scan through a directory returning all filenames matching a
** wildcard filename, including directories
**
** @param [w] list [AjPList] List for matching entries
** @param [r] path [const AjPStr] Directory to scan
** @param [r] wildname [const AjPStr] Filename to search for
**
** @return [ajint] number of entries in list
** @@
******************************************************************************/

 ajint ajFilelistAddPathWildDir(AjPList list,
                                const AjPStr path,
                                const AjPStr wildname)
{
    ajuint oldsize;
    DIR *indir;
#if defined(AJ_IRIXLF)
    struct dirent64 *dp;
#else
    struct dirent *dp;
#endif
    AjPStr s = NULL;
    AjPStr t = NULL;
    AjPStr tpath = NULL;
#ifdef _POSIX_C_SOURCE
    char buf[sizeof(struct dirent)+MAXNAMLEN];
#endif

    oldsize = ajListGetLength(list);

    tpath = ajStrNew();
    ajStrAssignS(&tpath,path);
    
    
    if(!ajDirnameFixExists(&tpath))
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
	ajStrAppendC(&s,dp->d_name);

	if(ajCharMatchWildS(dp->d_name,wildname))
        {
	    t = ajStrNewS(s);
	    ajListPushAppend(list,(void *)t);
        }
    }

    closedir(indir);
    
    
    ajStrDel(&s);
    ajStrDel(&tpath);

    return ajListGetLength(list) - oldsize;
}




/* @obsolete ajDirScan
** @remove Use ajFilelistAddPathWildDir
*/
__deprecated ajint ajDirScan(const AjPStr path,
			     const AjPStr filename, AjPList *result)
{
  return ajFilelistAddPathWildDir(*result, path, filename);
}




/* @funcstatic fileListRecurs  ************************************************
**
** Add a filename, expanded wildcard filenames and list file contents to
** a list
**
** @param [r] srcfile [const AjPStr] filename, wild filename or list filename
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

	ajFilelistAddPathWild(dlist, dir, file);

	while(ajListPop(dlist,(void **)&ptr))
	{
	    if(ajStrPrefixC(ptr,CURRENT_DIR))
		ajStrCutStart(&ptr,2);

	    ajListPushAppend(list,(void *)ptr);
	}
    }
    else if(c=='@')
    {
	if((inf=ajFileNewInNameC(ajStrGetPtr(file)+1)))
	    while(ajReadlineTrim(inf,&line))
		fileListRecurs(line,list,recurs);
	if(inf)
	    ajFileClose(&inf);
    }
    else if(ajStrPrefixC(file,"list::"))
    {
	if((inf=ajFileNewInNameC(ajStrGetPtr(file)+6)))
	    while(ajReadlineTrim(inf,&line))
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




/* @func ajFilelistAddPathWildRecursiveIgnore *********************************
**
** Recursively scan through a directory
**
** @param [u] list [AjPList] List for matching filenames
** @param [r] path [const AjPStr] Directory to scan
** @param [r] wildname [const AjPStr] Filename to search for (or NULL)
** @param [u] ignorelist [AjPList] List of directories to ignore
**
** @return [ajint] number of new entries in list
** @@
******************************************************************************/

ajint ajFilelistAddPathWildRecursiveIgnore(AjPList list,
					   const AjPStr path,
					   const AjPStr wildname,
					   AjPList ignorelist)
{
    ajuint oldsize;
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

    oldsize = ajListGetLength(list);

    tpath = ajStrNew();
    ajStrAssignS(&tpath,path);
    
    if(!ajDirnameFixExists(&tpath))
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
	if(ajFilenameExistsDir(s))
	{
	    /* Ignore selected directories */
	    if(ajListGetLength(ignorelist))
	    {
		flag = ajFalse;
		iter = ajListIterNewread(ignorelist);

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

	    if(!ajFilenameExistsRead(s) || !ajFilenameExistsExec(s))
		continue;

	    t = ajStrNewC(ajStrGetPtr(s));
	    ajListPushAppend(dirs,(void *)t);
	}
	else if(ajFilenameExistsRead(s))
	{
	  if(ajStrGetLen(wildname))
		if(ajCharMatchWildS(dp->d_name,wildname))
		{
		    t = ajStrNewS(s);
		    ajListPushAppend(list,(void *)t);
		}
	    
	    /*ajDebug("  %s\n",dp->d_name);*/
	}
    }

    closedir(indir);
    
    while(ajListPop(dirs,(void **)&t))
    {
	ajFilelistAddPathWildRecursiveIgnore(list, t,wildname, ignorelist);
	ajStrDel(&t);
    }
    
    ajStrDel(&s);
    ajStrDel(&tpath);
    ajListFree(&dirs);

    return ajListGetLength(list) - oldsize;
}




/* @datasection [none] File internal values **********************************
**
** Functions returning internal values and system parameters
**
** @nam2rule File
**
******************************************************************************/




/* @section Values
**
** @fdata [none]
**
** @nam3rule Value Return a value
** @nam4rule Buffsize Internal buffer size default
** @nam4rule Cwd Current working directory
** @nam4rule Redirect Test whether a system file is redirected to
**                    a file object
** @nam5rule RedirectStderr Test redirection of standard error
** @nam5rule RedirectStdin  Test redirection of standard input
** @nam5rule RedirectStdout Test redirection of standard output
**
** @valrule *Cwd [const AjPStr] True on success
** @valrule *Buffsize [ajuint] Buffer size
** @valrule Redirect [AjBool] True if file is redirected.
**
** @fcategory misc
**
******************************************************************************/




/* @func ajFileValueBuffsize **************************************************
**
** Returns the default buffer size for a file
**
** @return [ajuint] Buffer size default value
******************************************************************************/

ajuint ajFileValueBuffsize(void)
{
    return fileBuffSize;
}




/* @func ajFileValueCwd *******************************************************
**
** Returns the current directory
**
** @return [const AjPStr] Directory name.
** @@
******************************************************************************/

const AjPStr ajFileValueCwd(void)
{
    char cwd[PATH_MAX+1];

    if(!getcwd(cwd,PATH_MAX))
    {
        ajStrAssignClear(&fileCwd);

	return fileCwd;
    }


    ajStrAssignC(&fileCwd, cwd);

    if(!ajStrSuffixC(fileCwd, SLASH_STRING))
      ajStrAppendC(&fileCwd, SLASH_STRING);

    return fileCwd;
}




/* @obsolete ajFileGetwd
** @remove Use ajFileValueCwd
*/
__deprecated AjBool ajFileGetwd(AjPStr* dir)
{
  ajStrAssignS(dir, ajFileValueCwd());

  return ajTrue;
}




/* @func ajFileValueRedirectStderr ********************************************
**
** Tests whether stderr is in use by an internal file
**
** @return [AjBool] ajTrue if the file matches stderr.
** @@
******************************************************************************/

AjBool ajFileValueRedirectStderr(void)
{
    return fileUsedStderr;
}




/* @func ajFileValueRedirectStdin *********************************************
**
** Tests whether stdin is in use by an internal file
**
** @return [AjBool] ajTrue if the file matches stdin.
** @@
******************************************************************************/

AjBool ajFileValueRedirectStdin(void)
{
    return fileUsedStdin;
}




/* @func ajFileValueRedirectStdout ********************************************
**
** Tests whether stdout is in use by an internal file
**
** @return [AjBool] ajTrue if the file matches stdout.
** @@
******************************************************************************/

AjBool ajFileValueRedirectStdout(void)
{
    return fileUsedStdout;
}

/* @source ajfiledata *********************************************************
**
** AJAX datafile routines
**
** @author Copyright (C) 1999 Peter Rice
** @version $Revision: 1.9 $
** @modified Peter Rice pmr@ebi.ac.uk Data file functions from ajfile.c
** @modified $Date: 2011/11/08 15:07:45 $ by $Author: rice $
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


#include "ajlib.h"

#include "ajfiledata.h"
#include "ajarr.h"
#include "ajfiledata.h"
#include "ajnam.h"


#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>


#ifndef WIN32
#include <dirent.h>
#else
#include "win32.h"
#include "dirent_w32.h"
#include <direct.h>
#endif

#ifndef WIN32
#include <sys/wait.h>
#include <pwd.h>
#endif

#ifndef WIN32
#include <unistd.h>
#endif


static AjPStr filedataBaseTmp = NULL;
static AjPStr filedataHomeTmp = NULL;
static AjPStr filedataNameDataTmp = NULL;
static AjPStr filedataTmpstr = NULL;

static AjPStr filedataPath = NULL;




/* @filesection ajfile ********************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
*/




/* @datasection [AjPFile] File object *****************************************
**
** Function is for manipulating input files and returns or takes at least
** one AjPFile argument.
** 
** @nam2rule Datafile
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
** @nam4rule In Input file
** @nam4rule Out Output file created or rewritten
** @suffix Name Existing filename specified
** @suffix Path Input directory path specified
** @suffix C Filename as C character string
** @suffix S Filename as string
**
** @argrule C name [const char*] Filename
** @argrule S name [const AjPStr] Filename
** @argrule Path path [const AjPStr] Input directory path
**
** @valrule * [AjPFile] New file object
** @fcategory new
**
******************************************************************************/




/* @func ajDatafileNewInNameC *************************************************
**
** Returns an allocated data file pointer (AjPFile) if file exists
** a) in .   b) in ./.embossdata c) ~/ d) ~/.embossdata e) $DATA
**
** @param [r] name [const char*] Filename.
** @return [AjPFile] file pointer
**
** @release 6.0.0
** @@
******************************************************************************/

AjPFile ajDatafileNewInNameC(const char *name)
{
    AjPFile fnew = NULL;

    ajStrAssignC(&filedataTmpstr, name);
    fnew = ajDatafileNewInNameS(filedataTmpstr);
    ajStrDelStatic(&filedataTmpstr);

    return fnew;
}




/* @func ajDatafileNewInNameS *************************************************
**
** Returns an allocated data file pointer (AjPFile) if file exists
** a) in .   b) in ./.embossdata c) ~/ d) ~/.embossdata e) $DATA
**
** @param [r] name [const AjPStr] Filename
** @return [AjPFile] file pointer
**
** @release 6.0.0
** @@
******************************************************************************/

AjPFile ajDatafileNewInNameS(const AjPStr name)
{
    AjPFile fnew = NULL;
    char *p;
    
    if(!ajStrGetLen(name))
	return NULL;
    
    ajStrAssignS(&filedataBaseTmp, name);
    ajDebug("ajDatafileNewInNameS trying '%S'\n", filedataBaseTmp);

    if(ajFilenameExistsRead(filedataBaseTmp))
    {
	fnew = ajFileNewInNameS(filedataBaseTmp);
	ajStrDelStatic(&filedataBaseTmp);

	return fnew;
    }
    
    ajStrAssignC(&filedataNameDataTmp, ".embossdata");
    ajStrAppendC(&filedataNameDataTmp, SLASH_STRING);
    ajStrAppendS(&filedataNameDataTmp, filedataBaseTmp);
    ajDebug("ajDatafileNewInNameS trying '%S'\n", filedataNameDataTmp);

    if(ajFilenameExistsRead(filedataNameDataTmp))
    {
	fnew = ajFileNewInNameS(filedataNameDataTmp);
	ajStrDelStatic(&filedataBaseTmp);
	ajStrDelStatic(&filedataNameDataTmp);

	return fnew;
    }
    
    
    if((p=getenv("HOME")))
    {
	ajStrAssignC(&filedataHomeTmp,p);
	ajStrAppendC(&filedataHomeTmp,SLASH_STRING);
	ajStrAppendS(&filedataHomeTmp,filedataBaseTmp);
	ajDebug("ajDatafileNewInNameS trying '%S'\n", filedataHomeTmp);

	if(ajFilenameExistsRead(filedataHomeTmp))
	{
	    fnew = ajFileNewInNameS(filedataHomeTmp);
	    ajStrDelStatic(&filedataHomeTmp);
	    ajStrDelStatic(&filedataBaseTmp);
	    ajStrDelStatic(&filedataNameDataTmp);

	    return fnew;
	}

	ajStrAssignC(&filedataHomeTmp,p);
	ajStrAppendC(&filedataHomeTmp, "/.embossdata");
	ajStrAppendC(&filedataHomeTmp, SLASH_STRING);
	ajStrAppendS(&filedataHomeTmp,filedataBaseTmp);
	ajDebug("ajDatafileNewInNameS trying '%S'\n", filedataHomeTmp);

	if(ajFilenameExistsRead(filedataHomeTmp))
	{
	    fnew = ajFileNewInNameS(filedataHomeTmp);
	    ajStrDelStatic(&filedataHomeTmp);
	    ajStrDelStatic(&filedataBaseTmp);
	    ajStrDelStatic(&filedataNameDataTmp);

	    return fnew;
	}

	ajStrDelStatic(&filedataHomeTmp);
    }
    
    if(ajNamGetValueC("DATA", &filedataNameDataTmp))
    {
        ajDirnameFix(&filedataNameDataTmp);
	ajStrAppendS(&filedataNameDataTmp,filedataBaseTmp);
	ajDebug("ajDatafileNewInNameS trying '%S'\n", filedataNameDataTmp);

	if(ajFilenameExistsRead(filedataNameDataTmp))
	{
	    fnew = ajFileNewInNameS(filedataNameDataTmp);
	    ajStrDelStatic(&filedataBaseTmp);
	    ajStrDelStatic(&filedataNameDataTmp);

	    return fnew;
	}
    }
    
    /* just EMBOSS/data under installation */
    ajStrAssignS(&filedataNameDataTmp, ajNamValueInstalldir());

    if(ajStrGetLen(filedataNameDataTmp))
    {
	ajDirnameFix(&filedataNameDataTmp);
	ajStrAppendC(&filedataNameDataTmp,"share/");
	ajStrAppendS(&filedataNameDataTmp,ajNamValuePackage());
	ajStrAppendC(&filedataNameDataTmp,"/data/");
	ajStrAppendS(&filedataNameDataTmp,filedataBaseTmp);
	ajDebug("ajDatafileNewInNameS trying '%S'\n", filedataNameDataTmp);

	if(ajFilenameExistsRead(filedataNameDataTmp))
	{
	    fnew = ajFileNewInNameS(filedataNameDataTmp);
	    ajStrDelStatic(&filedataBaseTmp);
	    ajStrDelStatic(&filedataNameDataTmp);

	    return fnew;
	}
    }
    
    /* just emboss/data under source */
    ajStrAssignS(&filedataNameDataTmp, ajNamValueRootdir());

    if(ajStrGetLen(filedataNameDataTmp))
    {
	ajStrAppendC(&filedataNameDataTmp,"/data/");
	ajStrAppendS(&filedataNameDataTmp,filedataBaseTmp);
	ajDebug("ajDatafileNewInNameS trying '%S'\n", filedataNameDataTmp);

	if(ajFilenameExistsRead(filedataNameDataTmp))
	{
	    fnew = ajFileNewInNameS(filedataNameDataTmp);
	    ajStrDelStatic(&filedataBaseTmp);
	    ajStrDelStatic(&filedataNameDataTmp);

	    return fnew;
	}
    }
    
    ajStrDelStatic(&filedataBaseTmp);
    ajStrDelStatic(&filedataNameDataTmp);
    
    ajDebug("ajDatafileNewInNameS failed to find '%S'\n", filedataNameDataTmp);
    
    return NULL;
}




/* @func ajDatafileNewInNamePathS *********************************************
**
** Returns an allocated AjFileInNew pointer (AjPFile) if file exists
** in the EMBOSS/data/(dir) directory, or is found in the usual directories
** by ajDatafileNewInNameS
**
** @param [r] name [const AjPStr] Filename in AjStr.
** @param [r] path [const AjPStr] Data directory name in AjStr.
** @return [AjPFile] file pointer
**
** @release 6.0.0
** @@
******************************************************************************/

AjPFile ajDatafileNewInNamePathS(const AjPStr name, const AjPStr path)
{
    AjPFile fnew = NULL;
    char *p;

    if(!ajStrGetLen(name))
	return NULL;
    if(ajStrGetLen(path))
    {
        ajStrAppendS(&filedataNameDataTmp,path);
        ajDirnameFix(&filedataNameDataTmp);
        ajStrAppendS(&filedataBaseTmp, name);
    }
    else
    {
        ajStrAssignS(&filedataBaseTmp, name);
    }
    
    ajDebug("ajDatafileNewInNamePathS trying '%S'\n", filedataBaseTmp);

    if(ajFilenameExistsRead(filedataBaseTmp))
    {
	fnew = ajFileNewInNameS(filedataBaseTmp);
	ajStrDelStatic(&filedataBaseTmp);

	return fnew;
    }
    
    ajStrAssignC(&filedataNameDataTmp, ".embossdata");
    ajStrAppendC(&filedataNameDataTmp, SLASH_STRING);
    if(ajStrGetLen(path))
    {
        ajStrAppendS(&filedataNameDataTmp,path);
        ajDirnameFix(&filedataNameDataTmp);
    }
    ajStrAppendS(&filedataNameDataTmp, filedataBaseTmp);
    ajDebug("ajDatafileNewInNamePathS trying '%S'\n", filedataNameDataTmp);

    if(ajFilenameExistsRead(filedataNameDataTmp))
    {
	fnew = ajFileNewInNameS(filedataNameDataTmp);
	ajStrDelStatic(&filedataBaseTmp);
	ajStrDelStatic(&filedataNameDataTmp);

	return fnew;
    }
    
    
    if((p=getenv("HOME")))
    {
	ajStrAssignC(&filedataHomeTmp,p);
	ajStrAppendC(&filedataHomeTmp,SLASH_STRING);
        if(ajStrGetLen(path))
        {
            ajStrAppendS(&filedataNameDataTmp,path);
            ajDirnameFix(&filedataNameDataTmp);
        }
	ajStrAppendS(&filedataHomeTmp,filedataBaseTmp);
	ajDebug("ajDatafileNewInNamePathS trying '%S'\n", filedataHomeTmp);

	if(ajFilenameExistsRead(filedataHomeTmp))
	{
	    fnew = ajFileNewInNameS(filedataHomeTmp);
	    ajStrDelStatic(&filedataHomeTmp);
	    ajStrDelStatic(&filedataBaseTmp);
	    ajStrDelStatic(&filedataNameDataTmp);

	    return fnew;
	}

	ajStrAssignC(&filedataHomeTmp,p);
	ajStrAppendC(&filedataHomeTmp, "/.embossdata");
	ajStrAppendC(&filedataHomeTmp, SLASH_STRING);
        if(ajStrGetLen(path))
        {
            ajStrAppendS(&filedataNameDataTmp,path);
            ajDirnameFix(&filedataNameDataTmp);
        }
	ajStrAppendS(&filedataHomeTmp,filedataBaseTmp);
	ajDebug("ajDatafileNewInNamePathS trying '%S'\n", filedataHomeTmp);

	if(ajFilenameExistsRead(filedataHomeTmp))
	{
	    fnew = ajFileNewInNameS(filedataHomeTmp);
	    ajStrDelStatic(&filedataHomeTmp);
	    ajStrDelStatic(&filedataBaseTmp);
	    ajStrDelStatic(&filedataNameDataTmp);

	    return fnew;
	}

	ajStrDelStatic(&filedataHomeTmp);
    }
    
    if(ajNamGetValueC("DATA", &filedataNameDataTmp))
    {
        ajDirnameFix(&filedataNameDataTmp);

	if(ajStrGetLen(path))
	{
	    ajStrAppendS(&filedataNameDataTmp,path);
	    ajDirnameFix(&filedataNameDataTmp);
	}

	ajStrAppendS(&filedataNameDataTmp,name);
	ajDebug("ajDatafileNewInNamePathS trying '%S'\n", filedataNameDataTmp);

	if(ajFilenameExistsRead(filedataNameDataTmp))
	{
	    fnew = ajFileNewInNameS(filedataNameDataTmp);
	    ajStrDelStatic(&filedataNameDataTmp);

	    return fnew;
	}
    }
    
    /* just EMBOSS/data under installation */
    ajStrAssignS(&filedataNameDataTmp, ajNamValueInstalldir());

    if(ajStrGetLen(filedataNameDataTmp))
    {
	/* just EMBOSS */
	ajDirnameFix(&filedataNameDataTmp);
	ajStrAppendC(&filedataNameDataTmp,"share/");
	ajStrAppendS(&filedataNameDataTmp,ajNamValuePackage());
	ajStrAppendC(&filedataNameDataTmp,"/data/");

	if(ajStrGetLen(path))
	{
	    ajStrAppendS(&filedataNameDataTmp,path);
	    ajDirnameFix(&filedataNameDataTmp);
	}

	ajStrAppendS(&filedataNameDataTmp,name);
	ajDebug("ajDatafileNewInNamePathS trying '%S'\n", filedataNameDataTmp);

	if(ajFilenameExistsRead(filedataNameDataTmp))
	{
	    fnew = ajFileNewInNameS(filedataNameDataTmp);
	    ajStrDelStatic(&filedataNameDataTmp);

	    return fnew;
	}
    }

    /* just emboss/data under source */
    ajStrAssignS(&filedataNameDataTmp, ajNamValueRootdir());

    if(ajStrGetLen(filedataNameDataTmp))
    {
	ajStrAppendC(&filedataNameDataTmp,"/data/");

	if(ajStrGetLen(path))
	{
	    ajStrAppendS(&filedataNameDataTmp,path);
	    ajDirnameFix(&filedataNameDataTmp);
	}

	ajStrAppendS(&filedataNameDataTmp,name);
	ajDebug("ajDatafileNewInNamePathS trying '%S'\n", filedataNameDataTmp);

	if(ajFilenameExistsRead(filedataNameDataTmp))
	{
	    fnew = ajFileNewInNameS(filedataNameDataTmp);
	    ajStrDelStatic(&filedataNameDataTmp);

	    return fnew;
	}
    }
    
    ajStrDelStatic(&filedataNameDataTmp);
    
    return ajDatafileNewInNameS(name);
    
}




/* @func ajDatafileNewOutNameS ************************************************
**
** Returns an allocated output data file pointer (AjPFile) to a file in the
** emboss_DATA area
**
** @param [r] name [const AjPStr] Filename in AjStr.
** @return [AjPFile] file pointer.
**
** @release 6.0.0
** @@
******************************************************************************/

AjPFile ajDatafileNewOutNameS(const AjPStr name)
{
    AjPFile fnew = NULL;

    if(!ajStrGetLen(name))
        return NULL;

    if(ajNamGetValueC("DATA", &filedataNameDataTmp))
    {
	/* also does ajDirnameFix */
	if(!ajDirnameFixExists(&filedataNameDataTmp))
	{
	    ajFatal("%S_DATA directory not found: %S\n",
		    ajNamValuePackage(), filedataNameDataTmp);
	}

	ajStrAppendS(&filedataNameDataTmp,name);

	if(!(fnew = ajFileNewOutNameS(filedataNameDataTmp)))
	    ajFatal("Cannot write to file %S\n",filedataNameDataTmp);

	ajStrDelStatic(&filedataNameDataTmp);

	return fnew;
    }
    
    /* just emboss/data under installation */
    ajStrAssignS(&filedataNameDataTmp, ajNamValueInstalldir());

    if(ajStrGetLen(filedataNameDataTmp))
    {
	/* just EMBOSS */
	ajDirnameFix(&filedataNameDataTmp);
	ajStrAppendC(&filedataNameDataTmp,"share/");
	ajStrAppendS(&filedataNameDataTmp,ajNamValuePackage());
	ajStrAppendC(&filedataNameDataTmp,"/data/");

	/* if we are installed, else see below */
	if(ajDirnameFixExists(&filedataNameDataTmp))
	{
	    ajStrAppendS(&filedataNameDataTmp,name);

	    if(!(fnew = ajFileNewOutNameS(filedataNameDataTmp)))
		ajFatal("Cannot write to file %S\n",filedataNameDataTmp);

	    ajStrDelStatic(&filedataNameDataTmp);

	    return fnew;
	}
    }
    
    /* just emboss/data under source */
    ajStrAssignS(&filedataNameDataTmp, ajNamValueRootdir());

    if(ajStrGetLen(filedataNameDataTmp))
    {
	ajStrAppendC(&filedataNameDataTmp,"/data/");

	if(!ajDirnameFixExists(&filedataNameDataTmp))
	    ajFatal("Not installed, and source data directory not found: %S\n",
		    filedataNameDataTmp);

	ajStrAppendS(&filedataNameDataTmp,name);

	if(!(fnew = ajFileNewOutNameS(filedataNameDataTmp)))
	    ajFatal("Cannot write to file %S\n",filedataNameDataTmp);

	ajStrDelStatic(&filedataNameDataTmp);

	return fnew;
    }


    ajFatal("No install or source data directory, and %S_DATA not defined\n",
	    ajNamValuePackage());
    ajStrDelStatic(&filedataNameDataTmp);
    
    return NULL;
}




/* @section data file values ***************************************************
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
** @nam3rule Value Return an internal value
** @nam4rule Path Return the datafile path
**
** @valrule Path [const AjPStr] Datafile path
**
** @fcategory use
**
******************************************************************************/




/* @func ajDatafileValuePath **************************************************
**
** Get the path to the active data directory
**
** @return [const AjPStr] path.
**
** @release 6.0.0
** @@
******************************************************************************/

const AjPStr ajDatafileValuePath(void)
{
    if(filedataPath)
	return filedataPath;

    if(ajNamGetValueC("DATA", &filedataPath))
    {
	ajDirnameFix(&filedataPath);

	return filedataPath;
    }

#ifndef WIN32
    ajStrAssignS(&filedataPath,ajNamValueInstalldir()); 

    if(ajStrGetLen(filedataPath))
    {
	ajDirnameFix(&filedataPath);
	ajFmtPrintS(&filedataTmpstr,"%Sshare%c%S%cdata%c",
                    filedataPath,SLASH_CHAR,ajNamValuePackage(),
		    SLASH_CHAR,SLASH_CHAR);
	ajStrAssignS(&filedataPath,filedataTmpstr);

	if(ajDirnameFixExists(&filedataPath))
	    return filedataPath;
    }

    ajStrAssignS(&filedataPath, ajNamValueRootdir());

    if(ajStrGetLen(filedataPath))
    {
	ajStrAppendC(&filedataPath,SLASH_STRING);
	ajStrAppendC(&filedataPath,"data");
	ajStrAppendC(&filedataPath,SLASH_STRING);

	return filedataPath;
    }	

#endif

    return NULL;
}




/* @section file exit
**
** @fdata [AjPFile]
**
** Cleanup memory on program exit
**
** @nam3rule Exit Clean up memory and exit
**
** @valrule * [void]
**
** @fcategory misc
**
******************************************************************************/




/* @func ajDatafileExit *******************************************************
**
** Prints a summary of file usage with debug calls
**
** @return [void]
**
** @release 6.0.0
** @@
******************************************************************************/

void ajDatafileExit(void)
{
    ajStrDel(&filedataBaseTmp);
    ajStrDel(&filedataHomeTmp);
    ajStrDel(&filedataNameDataTmp);
    ajStrDel(&filedataTmpstr);
    ajStrDel(&filedataPath);

    return;
}




#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif




#ifdef AJ_COMPILE_DEPRECATED
/* @obsolete ajFileDataNewC
** @remove Use ajDatafileNewNameC
*/
__deprecated void ajFileDataNewC(const char *s, AjPFile *f)
{
    *f = ajDatafileNewInNameC(s);

    return;
}




/* @obsolete ajFileDataNew
** @remove Use ajDatafileNewInNameS
*/
__deprecated void ajFileDataNew(const AjPStr name, AjPFile *fnew)
{
    *fnew = ajDatafileNewInNameS(name);
}




/* @obsolete ajFileDataDirNew
** @remove Use ajDatafileNewInNamePathS
*/
__deprecated void ajFileDataDirNew(const AjPStr name, const AjPStr dir,
                                   AjPFile *fnew)
{
    *fnew = ajDatafileNewInNamePathS(name, dir);

    return;
}




/* @obsolete ajFileDataDirNewC
** @remove Use ajDatafileNewInNamePathS
*/
__deprecated void ajFileDataDirNewC(const char *s, const char* d, AjPFile *f)
{
    AjPStr u;

    u = ajStrNewC(d);
    ajStrAssignC(&filedataTmpstr, s);
    *f = ajDatafileNewInNamePathS(filedataTmpstr,u);
    ajStrDelStatic(&filedataTmpstr);
    ajStrDel(&u);

    return;
}




/* @obsolete ajFileDataNewWrite
** @remove Use ajDatafileNewOutNameS
*/
__deprecated void ajFileDataNewWrite(const AjPStr name, AjPFile *fnew)
{
    *fnew = ajDatafileNewOutNameS(name);

    return;
}




/* @obsolete ajFilePathData
** @remove Use ajDatafileValuePath
*/
__deprecated AjBool ajFilePathData(AjPStr *Ppath)
{

    ajStrAssignS(Ppath, ajDatafileValuePath());
    if(!ajStrGetLen(*Ppath)) return ajFalse;
    return ajTrue;
}
#endif

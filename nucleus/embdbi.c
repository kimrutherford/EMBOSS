/* @source embdbi.c
**
** General routines for alignment.
** Copyright (c) 2000 Peter Rice
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
******************************************************************************/

#include "emboss.h"
#include <math.h>
#ifndef WIN32
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#else
#include "win32.h"
#include "dirent_w32.h"
#endif

#include <errno.h>

static AjPStr dbiCmdStr    = NULL;
static AjPStr dbiCmdStr2    = NULL;
static AjPStr dbiDirFix    = NULL;
static AjPStr dbiWildFname = NULL;
static AjPStr dbiInFname   = NULL;
static AjPStr dbiOutFname  = NULL;
static AjPStr dbiOutRecord = NULL;
static AjPStr dbiSortExt   = NULL;
static AjPStr dbiLastId    = NULL;
static AjPStr dbiFieldId   = NULL;
static AjPStr dbiIdStr     = NULL;
static AjPStr dbiTmpStr     = NULL;
static AjPStr dbiRdLine     = NULL;
static AjPStr dbiIdLine     = NULL;
static AjPStr dbiFieldSort     = NULL;
static AjPStr dbiFieldSort2     = NULL;
static AjPStr dbiFieldStr     = NULL;
static AjPStr dbiFieldName     = NULL;
static AjPStr dbiFieldId2     = NULL;
static AjPStr dbiCurrentId = NULL;

static AjPRegexp dbiRegFieldIdSort    = NULL;
static AjPRegexp dbiRegFieldTokSort   = NULL;
static AjPRegexp dbiRegFieldTokIdSort = NULL;
static AjPRegexp dbiRegEntryIdSort    = NULL;
static AjPRegexp dbiRegDate           = NULL;


/* @datastatic DbiOField ******************************************************
**
** Database index field names and index filenames
**
** @attr name [const char*] Field name as used in USAs
** @attr index [const char*] Index filename for EMBLCD indices
** @attr desc [const char*] Field description
******************************************************************************/

typedef struct DbiSField
{
    const char* name;
    const char* index;
    const char* desc;
} DbiOField;


static DbiOField fieldDef[] =
{
   /* Name  Index      Description */
    {"acc", "acnum",   "accession number"},
    {"sv",  "seqvn",   "seqeunce version and GI number"},
    {"des", "des",     "entry description"},
    {"org", "taxon",   "taxonomy and organism"},
    {"key", "keyword", "keywords"},
    {NULL, NULL, NULL}
};

static const char* dbiFieldFile(const AjPStr fieldname);


/* @func embDbiFieldNew *******************************************************
**
** Constructor for field token structures.
**
** @return [EmbPField] Field token structure.
******************************************************************************/

EmbPField embDbiFieldNew(void)
{
    EmbPField ret;
    AJNEW0(ret);

    return ret;
}




/* @func embDbiFieldDel *******************************************************
**
** Destructor for field token structures.
**
** @param [d] pthys [EmbPField*] Field token structure.
** @return [void]
******************************************************************************/

void embDbiFieldDel(EmbPField* pthys)
{
    EmbPField thys;

    if(!pthys || !*pthys)
	return;

    thys = *pthys;

    AJFREE(thys->field);
    AJFREE(thys->entry);
    AJFREE(*pthys);

    return;
}




/* @func embDbiFieldDelMap ****************************************************
**
** Destructor for field token structures to be mapped to lists or tables.
**
** @param [d] pthys [void**] Field token structure.
** @param [u] cl [void*] Unused
** @return [void]
******************************************************************************/

void embDbiFieldDelMap(void** pthys, void* cl)
{
    EmbPField thys = (*(EmbPField*)pthys);

    if(!thys)
	return;

    (void) cl;				/* make it used */

    thys = *pthys;

    /*AJFREE(thys->field);*/
    /*AJFREE(thys->entry);*/
    AJFREE(*pthys);

    return;
}




/* @func embDbiCmpId **********************************************************
**
** Comparison function for two entries.
**
** @param [r] a [const void*] First id (EmbPEntry*)
** @param [r] b [const void*] Second id (EmbPEntry*)
** @return [ajint] Comparison value, -1, 0 or +1.
** @@
******************************************************************************/

ajint embDbiCmpId(const void* a, const void* b)
{
    const EmbPEntry aa;
    const EmbPEntry bb;

    aa = *(EmbPEntry const *) a;
    bb = *(EmbPEntry const *) b;

    return strcmp(aa->entry, bb->entry);
}




/* @func embDbiCmpFieldId *****************************************************
**
** Comparison function for the entrynames in two field structures.
**
** @param [r] a [const void*] First id (EmbPField*)
** @param [r] b [const void*] Second id (EmbPField*)
** @return [ajint] Comparison value, -1, 0 or +1.
** @@
******************************************************************************/

ajint embDbiCmpFieldId(const void* a, const void* b)
{
    const EmbPField aa;
    const EmbPField bb;

    aa = *(EmbPField const *) a;
    bb = *(EmbPField const *) b;

    return strcmp(aa->entry, bb->entry);
}




/* @func embDbiCmpFieldField **************************************************
**
** Comparison function for two field token values
**
** @param [r] a [const void*] First id (EmbPField*)
** @param [r] b [const void*] Second id (EmbPField*)
** @return [ajint] Comparison value, -1, 0 or +1.
** @@
******************************************************************************/

ajint embDbiCmpFieldField(const void* a, const void* b)
{
    ajint ret;

    const EmbPField aa;
    const EmbPField bb;

    aa = *(EmbPField const *) a;
    bb = *(EmbPField const *) b;

    ret = strcmp(aa->field, bb->field);

    if(ret)
	return ret;

    return strcmp(aa->entry, bb->entry);
}




/* @func embDbiEntryNew *******************************************************
**
** Constructor for entry structures.
**
** @param [r] nfields [ajuint] Number of data fields to be included
** @return [EmbPEntry] Entry structure.
******************************************************************************/

EmbPEntry embDbiEntryNew(ajuint nfields)
{
    EmbPEntry ret;

    AJNEW0(ret);
    ret->nfields = nfields;
    AJCNEW0(ret->nfield, nfields);
    AJCNEW0(ret->field, nfields);

    return ret;
}




/* @func embDbiEntryDel *******************************************************
**
** Destructor for entry structures.
**
** @param [d] Pentry [EmbPEntry*] Entry structure
** @return [void]
******************************************************************************/

void embDbiEntryDel(EmbPEntry* Pentry)
{
    EmbPEntry entry;
    ajuint i;
    ajuint j;

    if(!*Pentry)
        return;

    entry = *Pentry;

    for(i=0;i<entry->nfields;i++)
    {
	for(j=0;j<entry->nfield[i];j++)
	{
	    AJFREE(entry->field[i][j]);
	}

	AJFREE(entry->field[i]);
    }

    AJFREE(entry->nfield);
    AJFREE(entry->field);
    AJFREE(entry->entry);
    AJFREE(*Pentry);

    return;
}




/* @func embDbiEntryDelMap ****************************************************
**
** Destructor for entry structures to be mapped to lists or tables.
**
** @param [d] pthys [void**] Field token structure.
** @param [u] cl [void*] Unused
** @return [void]
******************************************************************************/

void embDbiEntryDelMap(void** pthys, void* cl)
{
    EmbPEntry entry;
    ajuint i;
    ajuint j;

    if(!pthys || !*pthys)
	return;

    (void) cl;				/* make it used */

    entry = (*(EmbPEntry*)pthys);

    for(i=0;i<entry->nfields;i++)
    {
	for(j=0;j<entry->nfield[i];j++)
	{
	    AJFREE(entry->field[i][j]);
	}

	AJFREE(entry->field[i]);
    }

    AJFREE(entry->nfield);
    AJFREE(entry->field);
    AJFREE(entry->entry);
    AJFREE(*pthys);

    return;
}




/* @func embDbiFileList *******************************************************
**
** Makes a list of all files in a directory matching a wildcard file name.
**
** @param [r] dir [const AjPStr] Directory
** @param [r] wildfile [const AjPStr] Wildcard file name
** @param [r] trim [AjBool] Expand to search, trim results
** @return [AjPList] New list of all files with full paths
** @@
******************************************************************************/

AjPList embDbiFileList(const AjPStr dir, const AjPStr wildfile, AjBool trim)
{
    AjPList retlist = NULL;

    DIR* dp;
    struct dirent* de;
    int dirsize;

    AjPStr name = NULL;
    AjPStr tmp;
    AjPStr s;
    AjPStr s2;
    AjPStr t;

    char *p;
    char *q;
    AjPList l;
    int ll;
    int i;
    AjBool d;

    ajDebug("embDbiFileList dir '%S' wildfile '%S' maxsize %Ld\n",
	    dir, wildfile, (ajlong) INT_MAX);

    ajStrAssignS(&dbiWildFname,wildfile);

    tmp = ajStrNewS(dbiWildFname);

    if(ajStrGetLen(dir))
	ajStrAssignS(&dbiDirFix, dir);
    else
	ajStrAssignC(&dbiDirFix, CURRENT_DIR);

    if(ajStrGetCharLast(dbiDirFix) != SLASH_CHAR)
	ajStrAppendC(&dbiDirFix, SLASH_STRING);

    if(trim)
	ajStrAppendC(&dbiWildFname,"*");

    dp = opendir(ajStrGetPtr(dbiDirFix));

    if(!dp)
	ajFatal("opendir failed on '%S'", dbiDirFix);

    s = ajStrNew();
    l = ajListNew();
    dirsize = 0;
    retlist = ajListstrNew();

    while((de = readdir(dp)))
    {
#ifndef __CYGWIN__
	if(!de->d_ino)
	    continue; 		/* skip deleted files with inode zero */
#endif

	if(ajCharMatchC(de->d_name, "."))
	    continue;

	if(ajCharMatchC(de->d_name, ".."))
	    continue;

	if(!ajCharMatchWildS(de->d_name, dbiWildFname))
	    continue;

	ajStrAssignC(&s,de->d_name);
	p = q =ajStrGetuniquePtr(&s);

	if(trim)
	{
	    p=strrchr(p,(int)'.');

	    if(p)
		*p='\0';
	}

	s2 = ajStrNewC(q);
	ll=ajListGetLength(l);
	d=ajFalse;

	for(i=0;i<ll;++i)
	{
	    ajListPop(l,(void *)&t);

	    if(ajStrMatchS(t,s2))
		d=ajTrue;

	    ajListPushAppend(l,(void *)t);
	}

	if(!d)
	    ajListPush(l,(void *)s2);
	else
	{
	    ajStrDel(&s2);
	    continue;
	}

	dirsize++;
	name = NULL;
	ajFmtPrintS(&name, "%S%S", dbiDirFix, s2);

	if(ajFilenameGetSize(name) > (ajlong) INT_MAX)
	  ajDie("File '%S' too large for DBI indexing", name);

	ajDebug("accept '%S' (%Ld)\n", s2, ajFilenameGetSize(name));
	ajListstrPushAppend(retlist, name);
    }

    if(!ajListGetLength(retlist))
	ajFatal("No match for file specification %S",tmp);

    while(ajListPop(l,(void *)&t))
	ajStrDel(&t);

    ajListFree(&l);

    ajStrDel(&s);
    ajStrDel(&tmp);

    closedir(dp);
    ajDebug("%d files for '%S' '%S'\n", dirsize, dir, dbiWildFname);

    return retlist;
}




/* @func embDbiFileListExc ****************************************************
**
** Makes a list of all files in a directory matching a wildcard file name.
**
** @param [r] dir [const AjPStr] Directory
** @param [r] wildfile [const AjPStr] Wildcard file list
** @param [r] exclude [const AjPStr] Wildcard file list
**                                   (NULL if none to exclude)
** @return [AjPList] New list of all files with full paths
** @@
******************************************************************************/

AjPList embDbiFileListExc(const AjPStr dir, const AjPStr wildfile,
			  const AjPStr exclude)
{
    AjPList retlist = NULL;

    DIR* dp;
    struct dirent* de;
    ajuint dirsize;
    AjPStr name = NULL;

    ajDebug("embDbiFileListExc dir '%S' wildfile '%S' exclude '%S' "
            "maxsize %Ld\n",
	    dir, wildfile, exclude, (ajlong) INT_MAX);

    if(ajStrGetLen(dir))
	ajStrAssignS(&dbiDirFix, dir);
    else
	ajStrAssignC(&dbiDirFix, CURRENT_DIR);

    if(ajStrGetCharLast(dbiDirFix) != SLASH_CHAR)
	ajStrAppendC(&dbiDirFix, SLASH_STRING);

    ajDebug("dirfix '%S'\n", dbiDirFix);

    dp = opendir(ajStrGetPtr(dbiDirFix));

    if(!dp)
	ajFatal("opendir failed on '%S'", dbiDirFix);

    dirsize = 0;
    retlist = ajListstrNew();

    while((de = readdir(dp)))
    {
	/* skip deleted files with inode zero */	
#ifndef __CYGWIN__
	if(!de->d_ino)
	    continue;
#endif

	if(ajCharMatchC(de->d_name, "."))
	    continue;

	if(ajCharMatchC(de->d_name, ".."))
	    continue;

	ajStrAssignC(&dbiInFname, de->d_name);

	if(exclude && !ajFilenameTestExclude(dbiInFname, exclude, wildfile))
	    continue;

	dirsize++;
	name = NULL;
	ajFmtPrintS(&name, "%S%S", dbiDirFix, dbiInFname);

	if(ajFilenameGetSize(name) > (ajlong) INT_MAX)
	  ajDie("File '%S' too large for DBI indexing", name);

	ajDebug("accept '%S' (%Ld)\n", dbiInFname, ajFilenameGetSize(name));
	ajListstrPushAppend(retlist, name);
    }

    closedir(dp);
    ajDebug("%d files for '%S' '%S'\n", dirsize, dir, wildfile);

    return retlist;
}




/* @func embDbiFlatOpenlib ****************************************************
**
** Open a flat file library
**
** @param [r] lname [const AjPStr] Source file basename
** @param [u] libr [AjPFile*] Database file
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool embDbiFlatOpenlib(const AjPStr lname, AjPFile* libr)
{
    ajFileClose(libr);

    *libr = ajFileNewInNameS(lname);

    if(!*libr)
	ajFatal("Cannot open %S for reading",lname);

    if(!*libr)
    {
	ajErr(" cannot open library flat file: %S\n",
	      lname);

	return ajFalse;
    }

    return ajTrue;
}




/* @func embDbiRmFile *********************************************************
**
** Remove a file or a set of numbered files
**
** @param [r] dbname [const AjPStr] Database name
** @param [r] ext [const char*] Base file extension
** @param [r] nfiles [ajuint] Number of files, or zero for unnumbered.
** @param [r] cleanup [AjBool] If ajTrue, clean up temporary files after
** @return [void]
** @@
******************************************************************************/

void embDbiRmFile(const AjPStr dbname, const char* ext, ajuint nfiles,
		  AjBool cleanup)
{
#ifndef WIN32
    ajuint i;

    if(!cleanup)
	return;

    if(nfiles)
    {
	ajFmtPrintS(&dbiCmdStr, "rm ");

	for(i=1; i<= nfiles; i++)
	    ajFmtPrintAppS(&dbiCmdStr, "%S%03d.%s ", dbname, i, ext);
    }
    else
	ajFmtPrintS(&dbiCmdStr, "rm %S.%s", dbname, ext);

    embDbiSysCmd(dbiCmdStr);

    return;

#else	/* WIN32 */
    static AjPStr filestr = NULL;
    ajuint i;
    
    if (!cleanup)
	return;
    
    if (nfiles)
    {
	for (i=1; i<= nfiles; i++)
	{
	    ajFmtPrintS (&filestr, "%S%03d.%s", dbname, i, ext);
	    DeleteFile(ajStrGetPtr(filestr));
	    ajDebug("Deleting file %S\n", filestr);
	}
    }
    else
    {
	ajFmtPrintS (&filestr, "%S.%s", dbname, ext);
	DeleteFile(ajStrGetPtr(filestr));
	ajDebug("Deleting file %S\n", filestr);
    }
    
    return;
#endif	/* WIN32 */
}




/* @func embDbiRmFileI ********************************************************
**
** Remove a numbered file
**
** @param [r] dbname [const AjPStr] Database name
** @param [r] ext [const char*] Base file extension
** @param [r] ifile [ajuint] File number.
** @param [r] cleanup [AjBool] If ajTrue, clean up temporary files after
** @return [void]
******************************************************************************/

void embDbiRmFileI(const AjPStr dbname, const char* ext, ajuint ifile,
		   AjBool cleanup)
{
#ifndef WIN32
    if(!cleanup)
	return;

    ajFmtPrintS(&dbiCmdStr, "rm %S%03d.%s ", dbname, ifile, ext);

    embDbiSysCmd(dbiCmdStr);
#else
    static AjPStr filestr = NULL;
    
    if(!cleanup)
	return;
    
    ajFmtPrintS (&filestr, "%S%03d.%s ", dbname, ifile, ext);
    DeleteFile(ajStrGetPtr(filestr));
    ajDebug("Deleting file %S\n", filestr);
#endif	/* WIN32 */

    return;
}




/* @func embDbiRmEntryFile ****************************************************
**
** Remove the sorted entryname file (kept until end of processing
** as it is the sorted list of all entries, used to count entries for
** field indexing.
**
** @param [r] dbname [const AjPStr] Database name
** @param [r] cleanup [AjBool] If ajTrue, clean up temporary files after
** @return [void]
** @@
******************************************************************************/

void embDbiRmEntryFile(const AjPStr dbname,  AjBool cleanup)
{
    embDbiRmFile(dbname, "idsrt", 0, cleanup);

    return;
}




/* @func embDbiSortFile *******************************************************
**
** Sort a file, or a set of numbered files, individually
**
** @param [r] dbname [const AjPStr] Database name
** @param [r] ext1 [const char*] Input file extension
** @param [r] ext2 [const char*] Output file extension
** @param [r] nfiles [ajuint] Number of files to sort (zero if unnumbered)
** @param [r] cleanup [AjBool] If ajTrue, clean up temporary files after
** @param [r] sortopt [const AjPStr] Extra options for the system sort
** @return [void]
** @@
******************************************************************************/

void embDbiSortFile(const AjPStr dbname, const char* ext1, const char* ext2,
		    ajuint nfiles, AjBool cleanup, const AjPStr sortopt)
{
    ajuint i;
    AjPStr dir = NULL;
    ajuint j;
    ajuint isplit;
    ajuint nsplit;
    double td;
    
#ifndef WIN32
    static const char *prog = "env LC_ALL=C sort";

    dir = ajStrNewC("");
#else
    static const char *prog = "sort.exe";
    
    char* sortProgDir = getenv("EMBOSS_ROOT");

    if(sortProgDir == NULL)
    {
	AjPStr msg = ajStrNewC("EMBOSS_ROOT");
	ajStrAppendC(&msg, " environment variable not defined");
	ajFatal(ajStrGetPtr(msg));
    }

    dir = ajStrNewC(sortProgDir);
    ajStrAppendC(&dir,SLASH_STRING);
#endif


    if(nfiles)
    {
	for(i=1; i<=nfiles; i++)
	{
	    ajFmtPrintS(&dbiInFname, "%S%03d.%s ", dbname, i, ext1);
	    ajFmtPrintS(&dbiOutFname, "%S%03d.%s.srt", dbname, i, ext1);

	    if(sortopt)
		ajFmtPrintS(&dbiCmdStr, "%S%s -o %S %S %S",
			    dir,prog,dbiOutFname,sortopt,dbiInFname);
	    else
		ajFmtPrintS(&dbiCmdStr, "%S%s -o %S %S",
			    dir,prog,dbiOutFname,dbiInFname);

	    embDbiSysCmd(dbiCmdStr);
	    embDbiRmFileI(dbname, ext1, i, cleanup);
	}

	td = sqrt(nfiles);
	nsplit = (ajuint) td;
	
	ajDebug("embDbiSortFile nfiles:%d split:%d\n", nfiles, nsplit);

	/* file merge in groups if more than 24 files ... avoids huge merges */

	if(nsplit < 2)		/* up to 3 source files */
	{
            ajFmtPrintS(&dbiCmdStr, "%S%s -m -o %S.%s %S",
                        dir,prog,dbname,ext2,sortopt);

            for(i=1; i<=nfiles; i++)
                ajFmtPrintAppS(&dbiCmdStr, " %S%03d.%s.srt", dbname, i, ext1);

            embDbiSysCmd(dbiCmdStr);
            ajFmtPrintS(&dbiSortExt, "%s.srt ", ext1);

            for(i=1; i<=nfiles; i++)
                embDbiRmFileI(dbname, ajStrGetPtr(dbiSortExt), i, cleanup);

	}
	else
	{
            ajFmtPrintS(&dbiCmdStr2, "%S%s -m -o %S.%s %S",
                        dir,prog,dbname,ext2,sortopt);
            isplit = 0;

            for(i=1; i<=nfiles; i+=nsplit)
            {
                isplit++;
                ajFmtPrintAppS(&dbiCmdStr2, " %S%03d.%s.mrg1",
                               dbname, isplit, ext2);

                /* Now we make that .mrg1 file */

                ajFmtPrintS(&dbiCmdStr, "%S%s -m -o %S%03d.%s.mrg1 %S",
                            dir,prog,dbname,isplit,ext2,sortopt);

                for(j=0; j<nsplit; j++)
                    if((i+j) <= nfiles)
                        ajFmtPrintAppS(&dbiCmdStr, " %S%03d.%s.srt",
                                       dbname, i+j, ext1);

                embDbiSysCmd(dbiCmdStr);
                ajFmtPrintS(&dbiSortExt, "%s.srt ", ext1);

                for(j=0; j<nsplit; j++)
                    if((i+j) <= nfiles)
                        embDbiRmFileI(dbname, ajStrGetPtr(dbiSortExt), (i+j),
                                      cleanup);
            }

            embDbiSysCmd(dbiCmdStr2);
            ajFmtPrintS(&dbiSortExt, "%s.mrg1", ext2);

            for(j=1; j<=isplit; j++)
                embDbiRmFileI(dbname, ajStrGetPtr(dbiSortExt), j, cleanup);
	}
    }
    else
    {
	ajFmtPrintS(&dbiInFname, "%S.%s ", dbname, ext1);
	ajFmtPrintS(&dbiOutFname, "%S.%s", dbname, ext2);
	ajFmtPrintS(&dbiCmdStr, "%S%s -o %S %S %S",
		    dir,prog,dbiOutFname,sortopt,dbiInFname);

	embDbiSysCmd(dbiCmdStr);
	embDbiRmFile(dbname, ext1, 0, cleanup);
    }

    ajStrDel(&dir);

    return;
}




/* @func embDbiSysCmd *********************************************************
**
** Fork a system command
**
** @param [r] cmdstr [const AjPStr] Command line
** @return [void]
** @@
******************************************************************************/

void embDbiSysCmd(const AjPStr cmdstr)
{
#ifndef WIN32
    char** arglist = NULL;
    char* pgm;
    pid_t pid;
    pid_t retval;
    ajint status;

    ajDebug("forking '%S'", cmdstr);
    ajSysArglistBuild(cmdstr, &pgm, &arglist);

    pid = fork();
    if(pid==-1)
	ajFatal("System fork failed");


    if(!pid)
    {
	execvp(pgm, arglist);
	ajExitAbort();			/* just in case */
    }

    while((retval=waitpid(pid,&status,0))!=pid)
    {
	if(retval == -1)
	    if(errno != EINTR)
		break;
    }

    ajSysArglistFree(&arglist);
    ajCharDel(&pgm);

#else
    PROCESS_INFORMATION procInfo;
    STARTUPINFO startInfo;
    
    ajDebug ("Launching process '%S'\n", cmdstr);
    
    ZeroMemory(&startInfo, sizeof(startInfo));
    startInfo.cb = sizeof(startInfo);
    
    if (!CreateProcess(NULL, (char *)ajStrGetPtr(cmdstr), NULL, NULL, FALSE,
		       CREATE_NO_WINDOW, NULL, NULL, &startInfo, &procInfo))
	ajFatal("CreateProcess failed");

    WaitForSingleObject(procInfo.hProcess, INFINITE);

#endif	/* WIN32 */

    return;
}




/* @func embDbiHeaderSize *****************************************************
**
** Updates the file header for an index file to include the correct file size.
**
** @param [u] file [AjPFile] Output file
** @param [r] filesize [ajuint] File size (if known, can be rewritten)
** @param [r] recordcnt [ajuint] Number of records
** @return [void]
******************************************************************************/

void embDbiHeaderSize(AjPFile file, ajuint filesize, ajuint recordcnt)
{
    ajFileSeek(file, 0, 0);

    ajWritebinInt4(file, (ajint) filesize);	/* filesize */
    ajWritebinInt4(file, (ajint) recordcnt);	/* #records */

    return;
}




/* @func embDbiHeader *********************************************************
**
** Writes the header for an index file. Resets the file pointer to beginning
** of file, and leaves the file pointer at the start of the first record.
**
** @param [u] file [AjPFile] Output file
** @param [r] filesize [ajuint] File size (if known, can be rewritten)
** @param [r] recordcnt [ajuint] Number of records
** @param [r] recordlen [short] Record length (bytes)
** @param [r] dbname [const AjPStr] Database name (up to 20 characters used)
** @param [r] release [const AjPStr] Release as a string (up to 10
**                                   characters used)
** @param [r] date [const char[4]] Date dd,mm,yy,00
** @return [void]
******************************************************************************/

void embDbiHeader(AjPFile file, ajuint filesize, ajuint recordcnt,
		  short recordlen, const AjPStr dbname, const AjPStr release,
		  const char date[4])
{
    ajuint i;
    static char padding[256];
    static AjBool firstcall = AJTRUE;

    if(firstcall)
    {
	for(i=0;i<256;i++)
	    padding[i] = ' ';

	firstcall = ajFalse;
    }

    ajFileSeek(file, 0, 0);

    ajWritebinInt4(file, (ajint) filesize);	/* filesize */

    ajWritebinInt4(file, (ajint) recordcnt);	/* #records */

    ajWritebinInt2(file, (ajint) recordlen);	/* recordsize */

    /* rest of the header */
    ajWritebinStr(file, dbname,  20); /* dbname */
    ajWritebinStr(file, release, 10); /* release */
    ajWritebinByte(file, date[0]);	/* release date */
    ajWritebinByte(file, date[1]);	/* release date */
    ajWritebinByte(file, date[2]);	/* release date */
    ajWritebinByte(file, date[3]);	/* release date */
    ajWritebinBinary(file, 1, 256, padding); /* padding 256 bytes */

    return;
}




/* @func embDbiFileSingle *****************************************************
**
** Builds a filename for a single temporary file to save IDs or some other
** index field, for example EMBL01.list
**
** @param [r] dbname [const AjPStr] Database name
** @param [r] extension [const char*] Filename extension.
** @param [r] num [ajuint] Number for this file (start at 1)
** @return [AjPFile] Opened output file
**
******************************************************************************/

AjPFile embDbiFileSingle(const AjPStr dbname, const char* extension, ajuint num)
{
    AjPFile ret;

    ajFmtPrintS(&dbiOutFname, "%S%03d.%s", dbname, num, extension);
    ret = ajFileNewOutNameS(dbiOutFname);

    if(!ret)
	ajFatal("Cannot open %S for writing", dbiOutFname);

    return ret;
}




/* @func embDbiFileIn *********************************************************
**
** Builds a filename for a summary file to read IDs or some other
** index field, for example EMBL.acnum_sort
**
** @param [r] dbname [const AjPStr] Database name
** @param [r] extension [const char*] Filename extension.
** @return [AjPFile] Opened output file
**
******************************************************************************/

AjPFile embDbiFileIn(const AjPStr dbname, const char* extension)
{
    AjPFile ret;

    ajFmtPrintS(&dbiInFname, "%S.%s", dbname, extension);
    ret = ajFileNewInNameS(dbiInFname);

    if(!ret)
	ajFatal("Cannot open %S for reading", dbiInFname);

    return ret;
}




/* @func embDbiFileOut ********************************************************
**
** Builds a filename for a summary file to save IDs or some other
** index field, for example EMBL.acnum_srt2
**
** @param [r] dbname [const AjPStr] Database name
** @param [r] extension [const char*] Filename extension.
** @return [AjPFile] Opened output file
**
******************************************************************************/

AjPFile embDbiFileOut(const AjPStr dbname, const char* extension)
{
    AjPFile ret;

    ajFmtPrintS(&dbiOutFname, "%S.%s", dbname, extension);
    ret = ajFileNewOutNameS(dbiOutFname);

    if(!ret)
	ajFatal("Cannot open %S for writing", dbiOutFname);

    return ret;
}




/* @func embDbiFileIndex ******************************************************
**
** Builds a filename for a summary file to save IDs or some other
** index field, for example EMBL.acsrt2
**
** @param [r] indexdir [const AjPStr] Index directory
** @param [r] field [const AjPStr] Field name
** @param [r] extension [const char*] Filename extension.
** @return [AjPFile] Opened output file
**
******************************************************************************/

AjPFile embDbiFileIndex(const AjPStr indexdir, const AjPStr field,
			const char* extension)
{
    AjPFile ret;

    ajFmtPrintS(&dbiOutFname, "%S.%s", field, extension);
    ret = ajFileNewOutNamePathS(dbiOutFname, indexdir);

    if(!ret)
	ajFatal("Cannot open %S for writing", dbiOutFname);

    return ret;
}




/* @func embDbiWriteDivision **************************************************
**
** Writes the division index file
**
** @param [r] indexdir [const AjPStr] Index directory
** @param [r] dbname [const AjPStr] Database name
** @param [r] release [const AjPStr] Release number as a string
** @param [r] date [const char[4]] Date
** @param [r] maxfilelen [ajuint] Max file name length
** @param [r] nfiles [ajuint] Number of files indexes
** @param [r] divfiles [AjPStr const *] Division filenames
** @param [r] seqfiles [AjPStr const *] Sequence filenames (or NULL if none)
** @return [void]
******************************************************************************/

void embDbiWriteDivision(const AjPStr indexdir,
			 const AjPStr dbname, const AjPStr release,
			 const char date[4],  ajuint maxfilelen, ajuint nfiles,
			 AjPStr const * divfiles, AjPStr const * seqfiles)
{
    AjPFile divFile;
    AjPStr tmpfname = NULL;
    ajuint i;
    ajuint filesize;
 
    short recsize;

    ajStrAssignC(&tmpfname, "division.lkp");
    divFile = ajFileNewOutNamePathS(tmpfname, indexdir);

    filesize = 256 + 44 + (nfiles * (maxfilelen+2));
    recsize = maxfilelen + 2;

    embDbiHeader(divFile, filesize, nfiles, recsize, dbname, release, date);

    for(i=0; i<nfiles; i++)
    {
        if(seqfiles)
	    embDbiWriteDivisionRecord(divFile, maxfilelen, (short)(i+1),
				      divfiles[i], seqfiles[i]);
	else
	    embDbiWriteDivisionRecord(divFile, maxfilelen, (short)(i+1),
				      divfiles[i], NULL);
    }

    ajFileClose(&divFile);
    ajStrDel(&tmpfname);

    return;
}




/* @func embDbiWriteDivisionRecord ********************************************
**
** Writes a record to the division lookup file
**
** @param [u] file [AjPFile] Index file
** @param [r] maxnamlen [ajuint] Maximum file name length
** @param [r] recnum [short] Record number
** @param [r] datfile [const AjPStr] Data file name
** @param [r] seqfile [const AjPStr] Sequence file name (or NULL if none)
** @return [void]
******************************************************************************/

void embDbiWriteDivisionRecord(AjPFile file, ajuint maxnamlen, short recnum,
			       const AjPStr datfile, const AjPStr seqfile)
{
    ajWritebinInt2(file, recnum);

    if(ajStrGetLen(seqfile))
    {
	ajFmtPrintS(&dbiOutRecord, "%S %S", datfile, seqfile);
	ajWritebinStr(file, dbiOutRecord, maxnamlen);
    }
    else
	ajWritebinStr(file, datfile, maxnamlen);

    return;
}




/* @func embDbiWriteEntryRecord ***********************************************
**
** Writes a record to the entryname index file
**
** @param [u] file [AjPFile] hit file
** @param [r] maxidlen [ajuint] Maximum length for an id string
** @param [r] id [const AjPStr] The id string for this entry
** @param [r] rpos [ajint] Data file offset
** @param [r] spos [ajint] sequence file offset
** @param [r] filenum [short] file number in division file
** @return [void]
******************************************************************************/

void embDbiWriteEntryRecord(AjPFile file, ajuint maxidlen, const AjPStr id,
			    ajint rpos, ajint spos, short filenum)
{

    ajWritebinStr(file, id, maxidlen);
    ajWritebinInt4(file, rpos);
    ajWritebinInt4(file, spos);
    ajWritebinInt2(file, filenum);

    return;
}




/* @func embDbiWriteHit *******************************************************
**
** Writes a record to the field hit (.hit) index file
**
** @param [u] file [AjPFile] hit file
** @param [r] idnum [ajuint] Entry number (1 for the first) in the
**                          entryname file
** @return [void]
******************************************************************************/

void embDbiWriteHit(AjPFile file, ajuint idnum)
{
    ajWritebinInt4(file, (ajint) idnum);

    return;
}




/* @func embDbiWriteTrg *******************************************************
**
** Writes a record to the field target (.trg) index file
**
** @param [u] file [AjPFile] hit file
** @param [r] maxfieldlen [ajuint] Maximum field token length
** @param [r] idnum [ajuint] First record number (1 for the first) in the
**                          field hit index file
** @param [r] idcnt [ajuint] Number of entries for this field value
**                          in the field hit index file
** @param [r] hitstr [const AjPStr] Field token string
** @return [void]
******************************************************************************/

void embDbiWriteTrg(AjPFile file, ajuint maxfieldlen, ajuint idnum,
		    ajuint idcnt, const AjPStr hitstr)
{
    ajWritebinInt4(file, (ajint) idnum);
    ajWritebinInt4(file, (ajint) idcnt);
    ajWritebinStr(file, hitstr, maxfieldlen);

    return;
}




/* @func embDbiSortOpen *******************************************************
**
** Open sort files for entries and all fields
**
** @param [w] alistfile [AjPFile*] Sort files for each field.
** @param [r] ifile [ajuint] Input file number (used for temporary file names)
** @param [r] dbname [const AjPStr] Database name
**                                  (used for temporary file names)
** @param [r] fields [AjPStr const *] Field names (used for temporary
**                                   file names)
** @param [r] nfields [ajuint] Number of fields
** @return [AjPFile] Sort file for entries
******************************************************************************/

AjPFile embDbiSortOpen(AjPFile* alistfile,
		       ajuint ifile, const AjPStr dbname,
		       AjPStr const * fields, ajuint nfields)
{
    AjPFile elistfile;
    ajuint ifield;

    elistfile = embDbiFileSingle(dbname, "list", ifile+1);

    for(ifield=0;ifield < nfields; ifield++)
	alistfile[ifield] = embDbiFileSingle(dbname,
					     dbiFieldFile(fields[ifield]),
					     ifile+1);

    return elistfile;
}




/* @funcstatic dbiFieldFile ***************************************************
**
** Returns the index filename that relates to a USA field name
**
** @param [r] fieldname [const AjPStr] Field name
** @return [const char*] Index filename for this field
******************************************************************************/

static const char* dbiFieldFile(const AjPStr fieldname)
{
    ajuint i = 0;

    for(i=0;fieldDef[i].name;i++)
	if(ajStrMatchCaseC(fieldname, fieldDef[i].name))
	    return fieldDef[i].index;

    ajErr("Unknown query field '%S' in index filename lookup", fieldname);
    return NULL;
}




/* @func embDbiSortClose ******************************************************
**
** Close the sort files for entries and all fields
**
** @param [u] elistfile [AjPFile*] Sort file for entries
** @param [u] alistfile [AjPFile*] Sort files for each field.
** @param [r] nfields [ajuint] Number of fields
** @return [void]
******************************************************************************/

void embDbiSortClose(AjPFile* elistfile, AjPFile* alistfile, ajuint nfields)
{
    ajuint ifield;

    ajFileClose(elistfile);

    for(ifield=0; ifield < nfields; ifield++)
	ajFileClose(&alistfile[ifield]);

    return;
}




/* @func embDbiMemEntry *******************************************************
**
** Stores data for current entry in memory by appending to lists
**
** @param [u] idlist [AjPList] List of entry IDs
** @param [u] fieldList [AjPList*] List of field tokens for each field
** @param [r] nfields [ajuint] Number of fields
** @param [u] entry [EmbPEntry] Current entry
** @param [r] ifile [ajuint] Current input file number
** @return [void]
******************************************************************************/

void embDbiMemEntry(AjPList idlist, AjPList* fieldList, ajuint nfields,
		    EmbPEntry entry, ajuint ifile)
{
    ajuint ifield;
    ajuint i;
    EmbPField fieldData = NULL;

    entry->filenum = ifile+1;
    ajListPushAppend(idlist, entry);

    for(ifield=0; ifield < nfields; ifield++)
	for(i=0;i<entry->nfield[ifield]; i++)
	{
	    fieldData = embDbiFieldNew();
	    fieldData->entry = entry->entry;
	    fieldData->field = entry->field[ifield][i];
	    ajListPushAppend(fieldList[ifield], fieldData);
	}

    return;
}




/* @func embDbiSortWriteEntry *************************************************
**
** Write the entryname index file using data from the entry sort file.
**
** @param [u] entFile [AjPFile] Entry file
** @param [r] maxidlen [ajuint] Maximum id length
** @param [r] dbname [const AjPStr] Database name (used in temp file names)
** @param [r] nfiles [ajuint] Number of files
** @param [r] cleanup [AjBool] Cleanup temp files if true
** @param [r] sortopt [const AjPStr] Sort commandline options
** @return [ajuint] Number of entries
******************************************************************************/

ajuint embDbiSortWriteEntry(AjPFile entFile, ajuint maxidlen,
                            const AjPStr dbname, ajuint nfiles,
                            AjBool cleanup, const AjPStr sortopt)
{
    AjPFile esortfile;
    ajint rpos;
    ajint spos;
    ajint filenum;
    ajuint idcnt = 0;

    if(!dbiRegEntryIdSort)
	dbiRegEntryIdSort =
	    ajRegCompC("^([^ ]+) +([0-9]+) +([0-9]+) +([0-9]+)");

    embDbiSortFile(dbname, "list", "idsrt", nfiles, cleanup, sortopt);
    ajStrAssignC(&dbiLastId, " ");
    esortfile = embDbiFileIn(dbname, "idsrt");

    while(ajReadline(esortfile, &dbiRdLine))
    {
	ajRegExec(dbiRegEntryIdSort, dbiRdLine);
	ajRegSubI(dbiRegEntryIdSort, 1, &dbiIdStr);
	ajRegSubI(dbiRegEntryIdSort, 2, &dbiTmpStr);
	ajStrToInt(dbiTmpStr, &rpos);
	ajRegSubI(dbiRegEntryIdSort, 3, &dbiTmpStr);
	ajStrToInt(dbiTmpStr, &spos);
	ajRegSubI(dbiRegEntryIdSort, 4, &dbiTmpStr);
	ajStrToInt(dbiTmpStr, &filenum);

	if(ajStrMatchCaseS(dbiIdStr, dbiLastId))
	{
            ajDebug("Duplicate ID '%S' filenum: %d",
                    dbiIdStr, filenum);
            ajWarn("Duplicate ID skipped: '%S' "
                   "All hits will point to first ID found",
                   dbiIdStr);
            continue;
	}

	embDbiWriteEntryRecord(entFile, maxidlen, dbiIdStr,
			       rpos, spos, filenum);
	ajStrAssignS(&dbiLastId, dbiIdStr);
	idcnt++;
    }
    ajFileClose(&esortfile);

    return idcnt;
}




/* @func embDbiMemWriteEntry **************************************************
**
** Write entryname index for in-memory processing
**
** @param [u] entFile [AjPFile] entryname index file
** @param [r] maxidlen [ajuint] Maximum entry id length
** @param [r] idlist [const AjPList] List of entry IDs to be written
** @param [w] ids [void***] AjPStr* array of IDs from list
** @return [ajuint] Number of entries written (excluding duplicates)
******************************************************************************/

ajuint embDbiMemWriteEntry(AjPFile entFile, ajuint maxidlen,
			  const AjPList idlist,
			  void ***ids)
{
    ajuint idCount;
    ajuint i;
    EmbPEntry entry;
    ajuint idcnt = 0;

    idCount = ajListToarray(idlist, ids);
    qsort(*ids, idCount, sizeof(void*), embDbiCmpId);
    ajDebug("ids sorted\n");

    for(i = 0; i < idCount; i++)
    {
	entry = (EmbPEntry)(*ids)[i];

	if(ajStrMatchCaseC(dbiIdStr, entry->entry))
	{
	    ajErr("Duplicate ID found: '%S'. ", dbiIdStr);
	    continue;
	}

	ajStrAssignC(&dbiIdStr, entry->entry);
	embDbiWriteEntryRecord(entFile, maxidlen, dbiIdStr,
			       entry->rpos, entry->spos, entry->filenum);
	idcnt++;
    }

    return idcnt;
}




/* @func embDbiSortWriteFields ************************************************
**
** Write the indices for a field.
**
** @param [r] dbname [const AjPStr] Database name (used for temp file names)
** @param [r] release [const AjPStr] Release number as a string
** @param [r] date [const char[4]] Date
** @param [r] indexdir [const AjPStr] Index directory
** @param [r] fieldname [const AjPStr] Field name (used for temp file names)
** @param [r] maxFieldLen [ajuint] Maximum field token length
** @param [r] nfiles [ajuint] Number of data files
** @param [r] nentries [ajuint] Number of entries
** @param [r] cleanup [AjBool] Cleanup temp files if true
** @param [r] sortopt [const AjPStr] Sort command line options
** @return [ajuint] Number of unique field targets written
******************************************************************************/

ajuint embDbiSortWriteFields(const AjPStr dbname, const AjPStr release,
			    const char date[4], const AjPStr indexdir,
			    const AjPStr fieldname, ajuint maxFieldLen,
			    ajuint nfiles, ajuint nentries,
			    AjBool cleanup, const AjPStr sortopt)
{
    AjPFile asortfile;
    AjPFile asrt2file;
    AjPFile blistfile;
    AjPFile elistfile;
    ajuint ient;

    ajuint fieldCount=0;
    ajuint idwidth;

    AjPFile trgFile;
    AjPFile hitFile;
    short alen;
    ajuint asize;
    ajuint ahsize;
    ajuint itoken = 0;
    ajuint i;
    ajuint j;
    ajuint k;
    ajint idnum;
    ajint lastidnum;

    ajStrAssignC(&dbiFieldName, dbiFieldFile(fieldname));
    ajFmtPrintS(&dbiTmpStr, "%d", nentries);
    idwidth = ajStrGetLen(dbiTmpStr);

    if(!dbiRegFieldIdSort)
	dbiRegFieldIdSort = ajRegCompC("^([^ ]+) +");

    if(!dbiRegFieldTokSort)
	dbiRegFieldTokSort = ajRegCompC("^([^ ]+) +([^\n]+)");

    if(!dbiRegFieldTokIdSort)
	dbiRegFieldTokIdSort = ajRegCompC("^(.*[^ ]) +([0-9]+)\n$");

    ajFmtPrintS(&dbiFieldId2, "%S_id2", dbiFieldName);
    ajFmtPrintS(&dbiFieldSort, "%S_sort", dbiFieldName);
    ajFmtPrintS(&dbiFieldSort2, "%S_sort2", dbiFieldName);

    trgFile = embDbiFileIndex(indexdir, dbiFieldName, "trg");
    hitFile = embDbiFileIndex(indexdir, dbiFieldName, "hit");

    embDbiSortFile(dbname, ajStrGetPtr(dbiFieldName),
		   ajStrGetPtr(dbiFieldSort),
		   nfiles, cleanup, sortopt);

    /* put in the entry numbers and remove the names */
    /* read dbname.<field>srt, for each entry, increment the count */

    elistfile = embDbiFileIn(dbname, "idsrt");
    asortfile = embDbiFileIn(dbname, ajStrGetPtr(dbiFieldSort));
    blistfile = embDbiFileOut(dbname, ajStrGetPtr(dbiFieldId2));

    fieldCount = 0;

    ient=0;
    ajStrAssignC(&dbiCurrentId, "");

    while(ajReadline(asortfile, &dbiRdLine))
    {
	ajRegExec(dbiRegFieldTokSort, dbiRdLine);
	ajRegSubI(dbiRegFieldTokSort, 1, &dbiIdStr);
	ajRegSubI(dbiRegFieldTokSort, 2, &dbiFieldStr);

	while(!ajStrMatchS(dbiIdStr, dbiCurrentId))
	{
	    ajStrAssignS(&dbiFieldId, dbiCurrentId);

	    if(!ajReadline(elistfile, &dbiIdLine))
		ajFatal("Error in embDbiSortWriteFields, "
			"expected entry %S not found",
			dbiIdStr);
	    ajRegExec(dbiRegFieldIdSort, dbiIdLine);
	    ajRegSubI(dbiRegFieldIdSort, 1, &dbiCurrentId);

	    if(!ajStrMatchS(dbiFieldId, dbiCurrentId))
		ient++;
	}

	ajFmtPrintF(blistfile, "%S %0*d\n", dbiFieldStr, idwidth, ient);
	fieldCount++;
    }

    ajFileClose(&asortfile);
    ajFileClose(&blistfile);
    ajFileClose(&elistfile);

    /* sort again */

    embDbiRmFile(dbname, ajStrGetPtr(dbiFieldSort), 0, cleanup);
    embDbiSortFile(dbname, ajStrGetPtr(dbiFieldId2),
		   ajStrGetPtr(dbiFieldSort2),
		   0, cleanup, sortopt);

    alen = maxFieldLen+8;
    asize = 300 + (fieldCount*(ajuint)alen); /* to be fixed later */
    embDbiHeader(trgFile, asize, fieldCount,
		 alen, dbname, release, date);

    ahsize = 300 + (fieldCount*4);
    embDbiHeader(hitFile, ahsize, fieldCount, 4,
		 dbname, release, date);

    itoken = 0;
    j = 0;
    k = 1;

    i = 0;
    lastidnum = 999999999;
    ajStrAssignC(&dbiFieldId, "");
    asrt2file = embDbiFileIn(dbname, ajStrGetPtr(dbiFieldSort2));

    while(ajReadline(asrt2file, &dbiRdLine))
    {
	ajRegExec(dbiRegFieldTokIdSort, dbiRdLine);
	ajRegSubI(dbiRegFieldTokIdSort, 1, &dbiIdStr);
	ajRegSubI(dbiRegFieldTokIdSort, 2, &dbiTmpStr);
	ajStrToInt(dbiTmpStr, &idnum);

	if(!i)
	    ajStrAssignS(&dbiFieldId, dbiIdStr);

	if(!ajStrMatchS(dbiFieldId, dbiIdStr))
	{
	    embDbiWriteHit(hitFile, idnum);
	    embDbiWriteTrg(trgFile, maxFieldLen,
			   j, k, dbiFieldId);
	    j = 1;			/* number of hits */
	    k = i+1;			/* first hit */
	    ajStrAssignS(&dbiFieldId, dbiIdStr);
	    i++;
	    itoken++;
	    lastidnum=idnum;
	}
	else if(idnum != lastidnum)	/* dbiIdStr is the same */
	{
	    embDbiWriteHit(hitFile, idnum);
	    lastidnum = idnum;
	    j++;
	    i++;
	}
    }

    ajFileClose(&asrt2file);
    embDbiRmFile(dbname, ajStrGetPtr(dbiFieldSort2), 0, cleanup);

    ajDebug("targets i:%d itoken: %d\n", i, itoken);

    if(i)
    {
	/* possibly there were no target tokens */
	embDbiWriteTrg(trgFile, maxFieldLen,
		       j, k, dbiFieldId);
	itoken++;
    }

    ajDebug("wrote %F %d\n", trgFile, itoken);

    embDbiHeaderSize(trgFile, 300+itoken*(ajuint)alen, itoken);

    ajDebug("finished...\n%7d files\n%7d %F\n%7d %F\n",
	    nfiles, itoken, trgFile,
	    fieldCount, hitFile);

    ajFileClose(&trgFile);
    ajFileClose(&hitFile);

    return itoken;
}




/* @func embDbiMemWriteFields *************************************************
**
** Write the fields indices
**
** @param [r] dbname [const AjPStr] Database name (used for temp file names)
** @param [r] release [const AjPStr] Release number as a string
** @param [r] date [const char[4]] Date
** @param [r] indexdir [const AjPStr] Index directory
** @param [r] fieldname [const AjPStr] Field name (used for file names)
** @param [r] maxFieldLen [ajuint] Maximum field token length
** @param [r] fieldList [const AjPList] List of field tokens to be written
** @param [r] ids [void**] AjPStr* array offield token s from list
** @return [ajuint] Number of unique field targets written
******************************************************************************/

ajuint embDbiMemWriteFields(const AjPStr dbname,const  AjPStr release,
			   const char date[4], const AjPStr indexdir,
			   const AjPStr fieldname, ajuint maxFieldLen,
			   const AjPList fieldList, void** ids)
{
    AjPStr field = NULL;

    ajuint fieldCount = 0;
    ajuint ient;
    ajuint fieldent;
    ajuint i;
    ajuint j;
    ajint k;
    void **fieldItems = NULL;
    AjPFile trgFile;
    AjPFile hitFile;
    short alen;
    ajuint asize;
    ajuint ahsize;
    ajuint itoken = 0;
    ajuint idup   = 0;
    EmbPField fieldData    = NULL;
    static const char* lastfd    = "";
    ajuint lastidnum = 0;

    ajStrAssignC(&field, dbiFieldFile(fieldname));
    trgFile = embDbiFileIndex(indexdir, field, "trg");
    hitFile = embDbiFileIndex(indexdir, field, "hit");

    fieldCount = ajListToarray(fieldList, &fieldItems);

    ajDebug("fieldItems: %d %x\n",
	    fieldCount, fieldItems);

    if(fieldCount)
    {
	qsort(fieldItems, fieldCount, sizeof(void*),
	      embDbiCmpFieldId);
	ajDebug("%S sorted by id\n", field);
	ient = 0;
	fieldent = 0;

	while(ids[ient] && fieldItems[fieldent])
	{
	    k = strcmp(((EmbPEntry)ids[ient])->entry,
		       ((EmbPField)fieldItems[fieldent])->entry);
	    if(k < 0)
		ient++;
	    else if(k > 0)
		fieldent++;
	    else
		((EmbPField)fieldItems[fieldent++])->nid = ient+1;
	}
	ajDebug("checked ids: %d fieldItems: %d %d\n",
		ient, fieldent, fieldCount);

	qsort(fieldItems, fieldCount, sizeof(void*),
	      embDbiCmpFieldField);
	ajDebug("%S sorted by %S\n", field, field);
    }

    alen = maxFieldLen+8;
    asize = 300 + (fieldCount*(ajuint)alen); /* to be fixed later */
    embDbiHeader(trgFile, asize, fieldCount,
		 alen, dbname, release, date);

    ahsize = 300 + (fieldCount*4);
    embDbiHeader(hitFile, ahsize, fieldCount, 4,
		 dbname, release, date);

    itoken = 0;
    j      = 0;
    k      = 1;
    idup   = 0;

    for(i = 0; i < fieldCount; i++)
    {
	fieldData = (EmbPField)fieldItems[i];

	if(!i)
	{
	    lastfd = fieldData->field;
	    lastidnum = 999999999;
	}

	if(strcmp(lastfd, fieldData->field))
	{
	    embDbiWriteHit(hitFile, fieldData->nid);
	    ajStrAssignC(&dbiFieldStr, lastfd);
	    embDbiWriteTrg(trgFile, maxFieldLen,
			   j, k,dbiFieldStr);
	    j = 1;
	    k = i+1-idup;
	    itoken++;
	    lastfd = fieldData->field;
	    lastidnum=fieldData->nid;
	}
	else if(fieldData->nid != lastidnum) /* lastfd is the same */
	{
	    embDbiWriteHit(hitFile, fieldData->nid);
	    lastidnum = fieldData->nid;
	    j++;
	}
	else
	    idup++;
    }

    ajStrAssignC(&dbiFieldStr, lastfd);

    if(fieldCount)
    {
	embDbiWriteTrg(trgFile, maxFieldLen, j, k, dbiFieldStr);
	itoken++;
    }

    ajDebug("wrote %F %d\n", trgFile, itoken);

    embDbiHeaderSize(trgFile, 300+itoken*(ajuint)alen, itoken);

    ajDebug("finished...\n%7d %F\n%7d %F\n",
	    itoken, trgFile,
	    fieldCount, hitFile);

    ajFileClose(&trgFile);
    ajFileClose(&hitFile);

    ajStrDel(&field);
    AJFREE(fieldItems);

    return itoken;
}




/* @func embDbiDateSet ********************************************************
**
** Sets the date as an integer array from a formatted string.
** The integer array is the internal format in database index headers
**
** @param [r] datestr [const AjPStr] Date as a string
** @param [w] date [char[4]] Data char (1 byte int) array
** @return [void]
******************************************************************************/

void embDbiDateSet(const AjPStr datestr, char date[4])
{
    ajuint i;
    ajint j;

    if(!dbiRegDate)
	dbiRegDate = ajRegCompC("^([0-9]+).([0-9]+).([0-9]+)");

    date[3] = 0;

    if(ajRegExec(dbiRegDate, datestr))
	for(i=1; i<4; i++)
	{
	    ajRegSubI(dbiRegDate, i, &dbiTmpStr);
	    ajStrToInt(dbiTmpStr, &j);
	    date[3-i] = j;
	}

    return;
}




/* @func embDbiMaxlen *********************************************************
**
** Compares a string to a maximum string length.
**
** A negative maximum length limits the string to that absolute length.
**
** A non-negative length is updated if the string is longer
**
** @param [u] token [AjPStr*] Token string
** @param [u] maxlen [ajint*] Maximum string length
** @return [void]
******************************************************************************/

void embDbiMaxlen(AjPStr* token, ajint* maxlen)
{
    if(*maxlen < 0)
	ajStrKeepRange(token, 1, -(*maxlen));
    else
    {
	if((ajint)ajStrGetLen(*token) > *maxlen)
	    *maxlen = ajStrGetLen(*token);
    }

    return;
}




/* @func embDbiLogHeader ******************************************************
**
** Writes the header to a database indexing logfile
**
** @param [u] logfile [AjPFile] Log file
** @param [r] dbname [const AjPStr] Database name
** @param [r] release [const AjPStr] Release number, name or code
** @param [r] datestr [const AjPStr] Indexing date as a string dd/mm/yy
** @param [r] indexdir [const AjPStr] Index directory relative path
** @param [r] maxindex [ajuint] Maximum index token length (usually zero)
** @return [void]
******************************************************************************/

void embDbiLogHeader(AjPFile logfile, const AjPStr dbname,
		     const AjPStr release, const AjPStr datestr,
		     const AjPStr indexdir,
		     ajuint maxindex)
{
    AjPStr dirname = NULL;
    AjPTime today = NULL;

    today =  ajTimeNewTodayFmt("report");
    ajFmtPrintF(logfile, "########################################\n");
    ajFmtPrintF(logfile, "# Program: %S\n", ajAcdGetProgram());
    ajFmtPrintF(logfile, "# Rundate: %D\n", today);
    ajFmtPrintF(logfile, "# Dbname: %S\n", dbname);
    ajFmtPrintF(logfile, "# Release: %S\n", release);
    ajFmtPrintF(logfile, "# Date: %S\n", datestr);
    ajFmtPrintF(logfile, "# CurrentDirectory: %S\n", ajFileValueCwd());
    ajFmtPrintF(logfile, "# IndexDirectory: %S\n", indexdir);
    ajStrAssignS(&dirname, indexdir);
    ajDirnameFillPath(&dirname);
    ajFmtPrintF(logfile, "# IndexDirectoryPath: %S\n", dirname);
    ajFmtPrintF(logfile, "# Maxindex: %d\n", maxindex);

    ajTimeDel(&today);
    ajStrDel(&dirname);

    return;
}




/* @func embDbiLogFields ******************************************************
**
** Writes database indexing logfile report of fields selected for indexing
**
** @param [u] logfile [AjPFile] Log file
** @param [r] fields [AjPStr const *] Field names
** @param [r] nfields [ajuint] Number of fields
** @return [void]
******************************************************************************/

void embDbiLogFields(AjPFile logfile, AjPStr const * fields, ajuint nfields)
{
    ajuint i;

    ajFmtPrintF(logfile, "# Fields: %d\n", nfields+1);
    ajFmtPrintF(logfile, "#   Field 1: id\n");

    for(i=0;i<nfields;i++)
	ajFmtPrintF(logfile, "#   Field %d: %S\n", i+2, fields[i]);

    return;
}




/* @func embDbiLogSource ******************************************************
**
** Writes database indexing logfile report of source data selected for indexing
**
** @param [u] logfile [AjPFile] Log file
** @param [r] directory [const AjPStr] Data directory relative path
** @param [r] filename [const AjPStr] Selected filenames wildcard
** @param [r] exclude [const AjPStr] Excluded filenames wildcard
** @param [r] inputFiles [AjPStr const *] File names
** @param [r] nfiles [ajuint] Number of files
** @return [void]
******************************************************************************/

void embDbiLogSource(AjPFile logfile, const AjPStr directory,
		     const AjPStr filename, const AjPStr exclude,
		     AjPStr const * inputFiles, ajuint nfiles)
{
    AjPStr dirname = NULL;
    ajuint i;

    ajFmtPrintF(logfile, "# Directory: %S\n", directory);
    ajStrAssignS(&dirname, directory);
    ajDirnameFillPath(&dirname);
    ajFmtPrintF(logfile, "# DirectoryPath: %S\n", dirname);
    ajFmtPrintF(logfile, "# Filenames: %S\n", filename);
    ajFmtPrintF(logfile, "# Exclude: %S\n", exclude);
    ajFmtPrintF(logfile, "# Files: %d\n", nfiles);

    for(i=0;i<nfiles;i++)
	ajFmtPrintF(logfile, "#   File %d: %S\n", i+1, inputFiles[i]);

    ajStrDel(&dirname);

    return;
}




/* @func embDbiLogCmdline *****************************************************
**
** Writes database indexing logfile report of commandline used
**
** @param [u] logfile [AjPFile] Log file
** @return [void]
******************************************************************************/

void embDbiLogCmdline(AjPFile logfile)
{
    AjPStr cmdline = NULL;

    ajFmtPrintF(logfile, "########################################\n");
    ajFmtPrintF(logfile, "# Commandline: %S\n", ajAcdGetProgram());
    ajStrAssignS(&cmdline, ajAcdGetCmdline());

    if(ajStrGetLen(cmdline))
    {
	ajStrExchangeCC(&cmdline, "\n", "\1#    ");
	ajStrExchangeCC(&cmdline, "\1", "\n");
	ajFmtPrintF(logfile, "#    %S\n", cmdline);
    }

    ajStrAssignS(&cmdline, ajAcdGetInputs());

    if(ajStrGetLen(cmdline))
    {
	ajStrExchangeCC(&cmdline, "\n", "\1#    ");
	ajStrExchangeCC(&cmdline, "\1", "\n");
	ajFmtPrintF(logfile, "#    %S\n", cmdline);
    }

    ajFmtPrintF(logfile, "########################################\n\n");
    ajStrDel(&cmdline);

    return;
}




/* @func embDbiLogFile *****************************************************
**
** Writes database indexing logfile report of a single source file
**
** @param [u] logfile [AjPFile] Log file
** @param [r] curfilename [const AjPStr] Source filename
** @param [r] idCountFile [ajuint] Number of IDs in file
** @param [r] fields [AjPStr const *] Field names
** @param [r] countField [const ajuint*] Number of field tokens in this file
** @param [r] nfields [ajuint] Number of fields
** @return [void]
******************************************************************************/

void embDbiLogFile(AjPFile logfile, const AjPStr curfilename,
		   ajuint idCountFile, AjPStr const * fields,
		   const ajuint* countField,
		   ajuint nfields)
{
    ajuint i;

    ajFmtPrintF(logfile, "filename: '%S'\n", curfilename);
    ajFmtPrintF(logfile, "    id: %d\n", idCountFile);

    for(i=0;i<nfields;i++)
	ajFmtPrintF(logfile, "   %3S: %d\n", fields[i], countField[i]);

    return;
}




/* @func embDbiLogFinal *******************************************************
**
** Writes database indexing logfile report of final totals
**
** @param [u] logfile [AjPFile] Log file
** @param [r] maxindex [ajuint] User defined maximum index token length
**                             (usually zero)
** @param [r] maxFieldLen [const ajint*] Maximum index token length 
**                                       for each field. Negative values
**                                       were upper limits. Positive values
**                                       are the maximum in the data
** @param [r] fields [AjPStr const *] Field names
** @param [r] fieldTot [const ajuint*] Number of unique field tokens
** @param [r] nfields [ajuint] Number of fields
** @param [r] nfiles [ajuint] Number of input files
** @param [r] idDone [ajuint] Number of unique IDs indexed
** @param [r] idCount [ajuint] Total number of IDs indexed
** @return [void]
******************************************************************************/

void embDbiLogFinal(AjPFile logfile, ajuint maxindex,
		    const ajint* maxFieldLen,
		    AjPStr const * fields, const ajuint* fieldTot,
		    ajuint nfields, ajuint nfiles, ajuint idDone,
		    ajuint idCount)
{
    ajuint i;
    ajuint maxlen;

    ajFmtPrintF(logfile, "\n");

    for(i=0;i<nfields;i++)
    {
        if(maxindex)
	    maxlen = maxindex;
	else
	    maxlen = maxFieldLen[i];

	ajFmtPrintF(logfile, "Index %S: maxlen %d items %d\n",
		    fields[i], maxlen, fieldTot[i]);
    }

    ajFmtPrintF(logfile, "\nTotal %d files %d entries (%d duplicates)\n",
		nfiles, idCount, (idCount-idDone));
    return;
}




/* @func embDbiExit ***********************************************************
**
** Cleanup database indexing internals on exit
**
** @return [void]
******************************************************************************/

void embDbiExit(void)
{
    ajStrDel(&dbiCmdStr);
    ajStrDel(&dbiCmdStr2);
    ajStrDel(&dbiDirFix);
    ajStrDel(&dbiWildFname);
    ajStrDel(&dbiInFname);
    ajStrDel(&dbiOutFname);
    ajStrDel(&dbiOutRecord);
    ajStrDel(&dbiSortExt);
    ajStrDel(&dbiLastId);
    ajStrDel(&dbiFieldId);
    ajStrDel(&dbiIdStr);
    ajStrDel(&dbiTmpStr);
    ajStrDel(&dbiRdLine);
    ajStrDel(&dbiIdLine);
    ajStrDel(&dbiFieldSort);
    ajStrDel(&dbiFieldSort2);
    ajStrDel(&dbiFieldStr);
    ajStrDel(&dbiFieldName);
    ajStrDel(&dbiFieldId2);
    ajStrDel(&dbiCurrentId);

    ajRegFree(&dbiRegFieldIdSort);
    ajRegFree(&dbiRegFieldTokSort);
    ajRegFree(&dbiRegFieldTokIdSort);
    ajRegFree(&dbiRegEntryIdSort);
    ajRegFree(&dbiRegDate);

    return;
}

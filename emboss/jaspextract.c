/* @source jaspextract application
**
** Copy JASPAR database to correct EMBOSS location
**
** @author Copyright (C) Alan Bleasby (ajb@ebi.ac.uk)
** @@
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

#ifdef WIN32
#include <direct.h>
#endif

#define PFMNUMGUESS 2000
#define MATRIXFILE "matrix_list.txt"




/* @datastatic JaspPrefix **************************************************
**
** JASPAR data directory structure
**
** @alias SJaspPrefix
** @alias OJaspPrefix
**
** @attr Prefix [const char*] Filename prefix of pfm files
** @attr Directory [const char*] Directory name for files of a given prefix
** @@
******************************************************************************/

typedef struct SJaspPrefix
{
    const char* Prefix;
    const char* Directory;
} OJaspPrefix;

static OJaspPrefix Jprefix[] =
{
    {"PB", "JASPAR_PBM"},
    {"PL", "JASPAR_PBM_HLH"},
    {"PH", "JASPAR_PBM_HOMEO"},
    {"MA", "JASPAR_CORE"},
    {"CN", "JASPAR_CNE"},
    {"MF", "JASPAR_FAM"},
    {"PF", "JASPAR_PHYLOFACTS"},
    {"POL", "JASPAR_POLII"},
    {"SA", "JASPAR_SPLICE"},
    {"SD", "JASPAR_SPLICE"},
    {NULL, NULL}
};




static void jaspextract_openoutdirs(void);
static void jaspextract_copyfiles(AjPStr directory);
static void jaspextract_readmatrixlist(AjPTable mtable, const AjPStr directory);
static void jaspextract_getjaspdirs(AjPList jdirlist);
static void jaspextract_writematrixfile(const AjPTable mtable,
                                        const AjPStr directory);




/* @prog jaspextract **********************************************************
**
** Copy JASPAR database to correct EMBOSS location.
**
** Looks for JASPAR_* directories using prefixes in jdirnames[]
** and copies files to EMBOSS_DATA. Avoids usual UNIX
** copy commands.
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPStr directory = NULL;
    AjPTable mtable = NULL;

    AjPList jdirlist = NULL;
    AjPStr  jdirloc  = NULL;
    
    embInit("jaspextract",argc,argv);

    directory = ajAcdGetDirectoryName("directory");

    mtable = ajTablestrNew(PFMNUMGUESS);
    jdirlist = ajListNew();
    
    jaspextract_openoutdirs();
    jaspextract_copyfiles(directory);

    jaspextract_readmatrixlist(mtable, directory);
    
    jaspextract_getjaspdirs(jdirlist);

    while(ajListPop(jdirlist,(void **)&jdirloc))
    {
        jaspextract_writematrixfile(mtable, jdirloc);
        ajStrDel(&jdirloc);
    }
    
    ajTablestrFree(&mtable);
    ajStrDel(&directory);
    ajListFree(&jdirlist);
    
    embExit();

    return 0;
}




/* @funcstatic jaspextract_openoutdirs **************************************
**
** Checks whether JASPAR output subdirectories exist in the current
** EMBOSS data area. Create them if not. Open them.
**
** @return [void]
** @@
******************************************************************************/

static void jaspextract_openoutdirs(void)
{
    const AjPStr datadir = NULL;
    AjPStr dir = NULL;
    const char *p = NULL;
    
    ajuint i = 0;
    
    datadir = ajDatafileValuePath();
    if(!datadir)
        ajFatal("jaspextract: Cannot determine the EMBOSS data directory");

    dir  = ajStrNew();
    
    i = 0;

    while(Jprefix[i].Directory)
    {
        ajFmtPrintS(&dir,"%S%s",datadir,Jprefix[i].Directory);

        if(!ajFilenameExistsDir(dir))
        {
            p = ajStrGetPtr(dir);
#ifndef WIN32
            mkdir(p,0755);
#else
            _mkdir(p);
#endif
        }
        
        if(!ajFilenameExistsDir(dir))
            ajFatal("jaspextract: No such directory %S",
                    dir);

        ++i;
    }
    
    ajStrDel(&dir);
    
    return;
}




/* @funcstatic jaspextract_copyfiles *****************************************
**
** Copy datafiles into the EMBOSS data area
**
** @param [u] directory [AjPStr] jaspar source flatfile directory
**
** @return [void]
** @@
******************************************************************************/

static void jaspextract_copyfiles(AjPStr directory)
{
    AjPStr matrixfile = NULL;
    AjPList flist     = NULL;
    
    AjPStr wild  = NULL;
    AjPStr entry = NULL;
    AjPStr bname = NULL;
    AjPStr line  = NULL;
    AjPStr dest  = NULL;

    const AjPStr datadir = NULL;
    
    ajuint preflen = 0;
    ajuint i       = 0;
    const char *p  = NULL;

    AjPFile inf   = NULL;
    AjPFile outf  = NULL;    
    
    matrixfile = ajStrNew();
    flist      = ajListNew();
    wild       = ajStrNewC("*.pfm");
    bname      = ajStrNew();
    line       = ajStrNew();
    dest       = ajStrNew();


    datadir = ajDatafileValuePath();
    if(!datadir)
        ajFatal("jaspextract: Cannot determine the EMBOSS data directory");
    
    ajFmtPrintS(&matrixfile,"%S%s",directory,MATRIXFILE);

    if(!ajFilenameExistsRead(matrixfile))
        ajFatal("jaspextract: Directory (%S) doesn't appear to be a JASPAR "
                "one\nNo matrix_list.txt file found",directory);
    
    ajFilelistAddPathWild(flist, directory, wild);


    while(ajListPop(flist,(void **)&entry))
    {
        ajStrAssignS(&bname,entry);
        ajFilenameTrimPath(&bname);
        
        i = 0;

        while(Jprefix[i].Prefix)
        {
            if(!ajStrPrefixC(bname,Jprefix[i].Prefix))
            {
                ++i;
                continue;
            }

            preflen = strlen(Jprefix[i].Prefix);
            p = ajStrGetPtr(bname);
            if(p[preflen]>='0' && p[preflen]<='9')
                break;

            ++i;
        }

        if(!Jprefix[i].Prefix)
        {
            ajStrDel(&entry);
            continue;
        }


        ajFmtPrintS(&dest,"%S%s%c%S",datadir,Jprefix[i].Directory,SLASH_CHAR,
                    bname);

        outf = ajFileNewOutNameS(dest);
        if(!outf)
            ajFatal("Cannot open output file %S",dest);

        /* Avoid UNIX copy for portability */
        inf  = ajFileNewInNameS(entry);
        if(!inf)
            ajFatal("Cannot open input file: %S",entry);

        while(ajReadlineTrim(inf,&line))
            ajFmtPrintF(outf,"%S\n",line);

        ajFileClose(&inf);
        ajFileClose(&outf);
        
        ajStrDel(&entry);        

    }
    
    ajListFree(&flist);
    
    ajStrDel(&wild);
    ajStrDel(&dest);
    ajStrDel(&line);
    ajStrDel(&bname);
    ajStrDel(&matrixfile);

    return;
}




/* @funcstatic jaspextract_readmatrixlist *************************************
**
** Read the matrix_list.txt file
**
** @param [u] mtable [AjPTable] Table for pfm entry descriptions
** @param [r] directory [const AjPStr] Directory containing matrix_list.txt
**
** @return [void]
** @@
******************************************************************************/

static void jaspextract_readmatrixlist(AjPTable mtable, const AjPStr directory)
{
    const AjPStr datadir = NULL;

    AjPStr matrixfile = NULL;
    AjPFile inf = NULL;

    AjPStr line  = NULL;
    AjPStr key   = NULL;
    AjPStr value = NULL;
    
    matrixfile = ajStrNew();
    

    datadir = ajDatafileValuePath();
    if(!datadir)
        ajFatal("jaspextract: Cannot determine the EMBOSS data directory");
    
    ajFmtPrintS(&matrixfile,"%S%s",directory,MATRIXFILE);

    if(!ajFilenameExistsRead(matrixfile))
        ajFatal("jaspextract: Directory (%S) doesn't appear to be a JASPAR "
                "one\nNo matrix_list.txt file found",directory);


    inf  = ajFileNewInNameS(matrixfile);
    if(!inf)
        ajFatal("Cannot open input file: %S",matrixfile);

    while(ajReadline(inf,&line))
    {
        key = ajStrNew();
        
        if(ajFmtScanS(line,"%S",&key) != 1)
        {
            ajStrDel(&key);
            continue;
        }

        value = ajStrNew();
        ajStrAssignS(&value,line);

        ajTablePut(mtable,(void *)key, (void *)value);
    }
    

    ajFileClose(&inf);
    
    ajStrDel(&matrixfile);
    ajStrDel(&line);
        
    return;
}




/* @funcstatic jaspextract_getjaspdirs ****************************************
**
** Construct a list of JASPAR_ directory locations in the EMBOSS data area
**
** @param [u] jdirlist [AjPList] List for JASPAR_ directory locations
**
** @return [void]
** @@
******************************************************************************/

static void jaspextract_getjaspdirs(AjPList jdirlist)
{
    const AjPStr datadir = NULL;
    AjPStr line = NULL;
    
    ajuint n = 0;
    ajuint i = 0;
    
    datadir = ajDatafileValuePath();
    if(!datadir)
        ajFatal("jaspextract: Cannot determine the EMBOSS data directory");


    n = ajFilelistAddPathDir(jdirlist, datadir);

    
    for(i=0; i < n; ++i)
    {
        ajListPop(jdirlist,(void **)&line);

        if(ajStrFindC(line,"JASPAR_") != -1)
            ajListPushAppend(jdirlist,(void *)line);
        else
            ajStrDel(&line);
    }

    return;
}




/* @funcstatic jaspextract_writematrixfile **********************************
**
** Writes a matrix_list.txt file in a JASPAR_ directory
**
** @param [r] mtable [const AjPTable] Table of matrix definitions 
** @param [r] directory [const AjPStr] JASPAR_ directory location
**
** @return [void]
** @@
******************************************************************************/

static void jaspextract_writematrixfile(const AjPTable mtable,
                                        const AjPStr directory)
{
    AjPStr wild   = NULL;
    AjPList flist = NULL;
    AjPStr key    = NULL;
    AjPStr fname  = NULL;
    AjPStr dest   = NULL;
    const AjPStr value  = NULL;
    
    AjPFile outf = NULL;
    
    const char *p = NULL;
    char *q = NULL;
    
    wild = ajStrNewC("*.pfm");
    flist = ajListNew();
    key   = ajStrNew();
    dest  = ajStrNew();
    
    ajFmtPrintS(&dest,"%S%c%s",directory,SLASH_CHAR,MATRIXFILE);
    
    outf = ajFileNewOutNameS(dest);
    if(!outf)
        ajFatal("Cannot open output file %S",dest);

    ajFilelistAddPathWild(flist, directory, wild);

    while(ajListPop(flist,(void**)&fname))
    {
        ajFilenameTrimPath(&fname);        

        p = ajStrGetPtr(fname);
        q = strrchr(p,(int)'.');
        ajStrAssignSubC(&key,p,0,q-p-1);
        
        value = ajTableFetchS(mtable, key);

        if(value)
            ajFmtPrintF(outf,"%S",value);

        ajStrDel(&fname);
    }
    
        
    ajFileClose(&outf);
    
    ajStrDel(&wild);
    ajStrDel(&dest);
    ajStrDel(&key);
    ajListFree(&flist);
    
    return;
}

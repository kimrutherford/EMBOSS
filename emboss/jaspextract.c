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




/* @datastatic JaspDir **************************************************
**
** JASPAR data directory structure
**
** @alias JaspSDir
** @alias JaspODir
**
** @attr namactual [AjPStr] name of Jaspar source subdirectory
** @attr namdata [AjPStr] name of Jaspar subdirectory in EMBOSS_DATA area
** @@
******************************************************************************/

typedef struct JaspSDir
{
    AjPStr namactual;
    AjPStr namdata;
} JaspODir;
#define JaspPDir JaspODir*




static void jaspextract_check(const AjPStr directory, AjPList dlist);
static void jaspextract_copy(AjPList dlist);

static void jaspextract_jddel(JaspPDir *thys);
static JaspPDir jaspextract_jdnew(void);




static const char *jdirnames[] = {
    "JASPAR_CORE",
    "JASPAR_FAM",
    "JASPAR_PHYLOFACTS",
    "JASPAR_CNE",
    "JASPAR_POLII",
    "JASPAR_SPLICE",
    NULL
};




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
    AjPList dlist = NULL;
    
    embInit("jaspextract",argc,argv);

    directory = ajAcdGetDirectoryName("directory");

    dlist = ajListNew();
    
    jaspextract_check(directory, dlist);

    jaspextract_copy(dlist);

    embExit();

    return 0;
}




/* @funcstatic jaspextract_check ***********************************************
**
** Checks whether specific JASPAR subdirectories exist
**
** @param [r] directory [const AjPStr] jaspar directory
** @param [w] dlist     [AjPList] jaspar directory list
**
** @return [void]
** @@
******************************************************************************/

static void jaspextract_check(const AjPStr directory, AjPList dlist)
{
    AjPStr entry  = NULL;
    AjPStr nbase  = NULL;
    
    AjPList flist = NULL;
    ajint i = 0;

    JaspPDir jdir = NULL;
    
    

    flist = ajListNew();


    ajFilelistAddPathDir(flist, directory);


    while(ajListPop(flist,(void **)&entry))
    {
        ajDebug("jaspextract_check '%S'\n", entry);
        
        if(!ajFilenameExistsRead(entry))
            continue;

        ajStrAssignS(&nbase,entry);
	ajFilenameTrimPath(&nbase);

        i = 0;
        while(jdirnames[i])
        {
            if(ajStrPrefixC(nbase,jdirnames[i]))
            {
                jdir = jaspextract_jdnew();
                ajStrAssignS(&jdir->namactual,entry);
                ajStrAssignC(&jdir->namdata,jdirnames[i]);
                ajListPush(dlist,(void *)jdir);
                ajDebug("found %d: '%s' => '%S' '%S'\n",
                        i, jdirnames[i], jdir->namactual, jdir->namdata);
            }
            

            ++i;
        }

        ajStrDel(&entry);
    }


    ajStrDel(&nbase);

    ajListFree(&flist);
    
    return;
}




/* @funcstatic jaspextract_copy ***********************************************
**
** Copy datafile into the EMBOSS  data area
**
** @param [u] dlist [AjPList] jaspar directory list
**
** @return [void]
** @@
******************************************************************************/

static void jaspextract_copy(AjPList dlist)
{
    AjPStr wild   = NULL;
    AjPList flist = NULL;
    AjPStr entry  = NULL;
    AjPStr nbase  = NULL;
    AjPStr dfile  = NULL;
    AjPFile inf   = NULL;
    AjPFile outf  = NULL;
    AjPStr line   = NULL;

    JaspPDir jdir = NULL;
    
    
    wild   = ajStrNewC("*");
    flist  = ajListNew();
    nbase  = ajStrNew();
    dfile  = ajStrNew();
    line   = ajStrNew();




    while(ajListPop(dlist,(void **)&jdir))
    {
        ajFilelistAddPathWild(flist, jdir->namactual,wild);
        ajDebug("jaspextract_copy '%S' '%S'\n",
                jdir->namactual, wild);
        while(ajListPop(flist,(void **)&entry))
        {
            ajStrAssignS(&nbase,entry);
            ajFilenameTrimPath(&nbase);
            ajFmtPrintS(&dfile,"%S%c%S",jdir->namdata,SLASH_CHAR,nbase);
            ajDebug("copying '%S' => '%S'\n",
                    entry, dfile);
            /* Avoid UNIX copy for portability */
            inf  = ajFileNewInNameS(entry);
            if(!inf)
                ajFatal("Cannot open input file: %S",entry);

            outf = ajDatafileNewOutNameS(dfile);
            if(!outf)
                ajFatal("Cannot open output file: %S",dfile);

            while(ajReadlineTrim(inf,&line))
                ajFmtPrintF(outf,"%S\n",line);

            ajFileClose(&inf);
            ajFileClose(&outf);

            ajStrDel(&entry);
        }

        jaspextract_jddel(&jdir);
    }
    

    ajListFree(&flist);
    ajListFree(&dlist);
    
    ajStrDel(&wild);
    ajStrDel(&dfile);
    ajStrDel(&nbase);
    ajStrDel(&line);
    
    return;
}




/* @funcstatic jaspextract_jdnew ***********************************************
**
** Create a jaspar directory structure
**
** @return [JaspPDir] Jaspar directory structure
** @@
******************************************************************************/

static JaspPDir jaspextract_jdnew(void)
{
    JaspPDir ret = NULL;

    AJNEW(ret);

    ret->namactual = ajStrNew();
    ret->namdata   = ajStrNew();

    return ret;
}




/* @funcstatic jaspextract_jddel ***********************************************
**
** Delete a Jaspar directory structure
**
** @param [w] thys [JaspPDir*] jaspar directory structure
**
** @return [void]
** @@
******************************************************************************/

static void jaspextract_jddel(JaspPDir *thys)
{
    JaspPDir pthis = NULL;

    if(!thys)
        return;

    if(!*thys)
        return;
    
    pthis = *thys;

    ajStrDel(&pthis->namactual);
    ajStrDel(&pthis->namdata);

    *thys = NULL;

    return;
}


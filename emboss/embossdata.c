/* @source embossdata application
**
** Finds or fetches the data files read in by the EMBOSS programs
**
** @author Copyright (C) Gary Williams (gwilliam@hgmp.mrc.ac.uk)
** @modified Alan Bleasby (ableasby@hgmp.mrc.ac.uk) for recursion
** @modified Alan Bleasby (ableasby@hgmp.mrc.ac.uk) to remove HOME dependency
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




static void embossdata_check_dir(const AjPStr d, AjPFile outf);
static void embossdata_check_file(const AjPStr d, const AjPStr file,
				  AjPFile outf);
static AjPStr embossdata_data_dir(void);




/* @prog embossdata ***********************************************************
**
** Finds or fetches the data files read in by the EMBOSS programs
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPList rlist = NULL;
    AjPList flocs = NULL;
    AjPFile outf;
    AjPStr  t = NULL;

    AjBool  recurs = ajTrue;
    ajint i;


    AjPStr filename = NULL;
    AjBool isname;

    AjBool fetch;
    AjBool showall;

    AjPStr directory = NULL;
    AjPStr hdir = NULL;
    AjPStr ddir = NULL;
    AjPStr path = NULL;
    AjPStr cmd  = NULL;
    AjPStr* rstrs = NULL;

    ajint result;
    char *p = NULL;

    embInit("embossdata", argc, argv);

    filename = ajAcdGetString("filename");
    fetch    = ajAcdGetBool("fetch");
    showall  = ajAcdGetToggle("showall");
    rstrs    = ajAcdGetSelect("reject");
    outf     = ajAcdGetOutfile("outfile");


    if(ajStrGetLen(filename))
	isname = ajTrue;
    else
	isname = ajFalse;


    /* Get directory reject list */
    rlist = ajListNew();
    if(!ajStrMatchCaseC(rstrs[0],"None"))
    {
	i = 0;
	while(rstrs[i])
	{
	    ajListPush(rlist,(void *)rstrs[i]);
	    ++i;
	}
    }


    /* get the full pathname of the  emboss/data installation directory */
    ddir = embossdata_data_dir();

    flocs = ajListNew();

    /*
    ** fetch the data from the 'official' EMBOSS
    ** data directory?
    */
    if(fetch)
    {
	ajStrAssignS(&path,ddir);
	ajFileScan(path,filename,&flocs,ajFalse,ajFalse,NULL,rlist,
		   recurs, outf);
	if(!ajListPop(flocs,(void **)&t))
	    ajFatal("The file '%S' does not exist.", filename);
	/* fetch it */
	ajStrAppendC(&cmd, "cp ");
	ajStrAppendS(&cmd, t);
	ajStrAppendC(&cmd, " ");
	ajStrAppendS(&cmd, filename);
	result = system(ajStrGetPtr(cmd));
	if(result)
	    ajFatal("File not copied.");
	ajFmtPrintF(outf, "File '%S' has been copied successfully.\n", t);
	ajStrDel(&t);
	ajStrDel(&cmd);
    }


    /*
    **  report whether data directories exist (no filename given)
    **  or whether a specific file is in those directories
    */
    if(!fetch && !showall)
    {
	ajFmtPrintF(outf,"# The following directories can contain "
		    "EMBOSS data files.\n");
	ajFmtPrintF(outf,"# They are searched in the following order "
		    "until the file is found.\n");
	ajFmtPrintF(outf, "# If the directory does not exist, then this "
		    "is noted below.\n");
	ajFmtPrintF(outf,"# '.' is the UNIX name for your current "
		    "working directory.\n");
	ajFmtPrintF(outf,"\n");
	
	ajStrAssignC(&directory, ".");
	if(isname)
	    embossdata_check_file(directory,filename,outf);
	else
	    embossdata_check_dir(directory,outf);
	
	/* .embossdata */
	ajStrAssignC(&directory, ".embossdata");
	if(isname)
	    embossdata_check_file(directory,filename,outf);
	else
	    embossdata_check_dir(directory,outf);
	
	/* HOME */
	if((p = getenv("HOME")))
	{
	    ajStrAssignC(&hdir, p);
	    ajStrAssignS(&directory, hdir);
	    if(isname)
		embossdata_check_file(directory,filename,outf);
	    else
		embossdata_check_dir(directory,outf);

	    /* ~/.embossdata */
	    ajStrAssignS(&directory, hdir);
	    ajStrAppendC(&directory, "/.embossdata");
	    if(isname)
		embossdata_check_file(directory,filename,outf);
	    else
		embossdata_check_dir(directory,outf);
	}
	

	/* DATA */
	if(isname)
	{
	    ajStrAssignS(&path,ddir);
	    ajFileScan(path,filename,&flocs,ajFalse,ajFalse,NULL,rlist,
		       recurs,outf);

	    if(!ajListPop(flocs,(void **)&t))
		embossdata_check_file(ddir,filename,outf);
	    else
	    {
		ajFmtPrintF(outf,"File %-60.60S Exists\n",t);
		ajStrDel(&t);
	    }
	}
	else
	    embossdata_check_dir(ddir,outf);
    }


    /* Just show all the files in the EMBOSS Installation data directory */
    if(showall)
    {
	ajStrAssignS(&path,ddir);
	ajFileScan(path,NULL,NULL,ajTrue,ajFalse,NULL,rlist,recurs,outf);
    }

    ajListFree(&flocs);
    while(ajListPop(rlist,(void **)&t))
	ajStrDel(&t);
    ajListFree(&rlist);
    ajStrDel(&path);
    ajStrDel(&hdir);
    ajStrDel(&ddir);
    ajStrDel(&directory);
    ajStrDel(&cmd);
    ajStrDel(&filename);
    ajFileClose(&outf);

    AJFREE(rstrs);			/* individual strings freed on list */

    embExit();

    return 0;
}




/* @funcstatic embossdata_check_dir *******************************************
**
** Undocumented.
**
** @param [r] d [const AjPStr] Undocumented
** @param [u] outf [AjPFile] Undocumented
** @@
******************************************************************************/



static void embossdata_check_dir(const AjPStr d, AjPFile outf)
{
    AjPStr tmpstr;
    tmpstr = ajStrNewS(d);

    if(ajFileDir(&tmpstr))
	ajFmtPrintF(outf,"%-60.60S Exists\n",d);
    else
	ajFmtPrintF(outf,"%-60.60S Does not exist\n",d);

    ajStrDel(&tmpstr);

    return;
}




/* @funcstatic embossdata_check_file ******************************************
**
** Undocumented.
**
** @param [r] d [const AjPStr] Undocumented
** @param [r] file [const AjPStr] Undocumented
** @param [u] outf [AjPFile] Undocumented
** @@
******************************************************************************/

static void embossdata_check_file(const AjPStr d, const AjPStr file,
				  AjPFile outf)
{
    AjPStr s;

    s = ajStrNew();

    ajStrAssignS(&s,d);
    ajStrAppendC(&s,"/");
    ajStrAppendS(&s,file);
    if(ajFileNameValid(s))
	ajFmtPrintF(outf,"File %-60.60S Exists\n",s);
    else
	ajFmtPrintF(outf,"File %-60.60S Does not exist\n",s);

    ajStrDel(&s);

    return;
}




/* @funcstatic embossdata_data_dir ********************************************
**
** Undocumented.
**
** @return [AjPStr] Undocumented
** @@
******************************************************************************/

static AjPStr embossdata_data_dir(void)
{
    static AjPStr where = NULL;
    AjPStr tmp = NULL;

    where = ajStrNew();
    tmp   = ajStrNew();


    if(!ajNamGetValueC("DATA",&tmp))
    {
	ajNamRootInstall(&where);
	ajFileDirFix(&where);
	ajFmtPrintS(&tmp,"%Sshare/EMBOSS/data/",where);

	if(!ajFileDir(&tmp))
	{
	    if(ajNamRoot(&tmp))
		ajStrAppendC(&tmp,"/data");
	    else
		ajFatal("The EMBOSS 'DATA' directory isn't defined.");
	}
    }

    ajStrAssignC(&where,ajStrGetPtr(tmp));

    ajStrDel(&tmp);

    return where;
}

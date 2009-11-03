/* @source embgroup.c
**
** Group Routines.
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

#include <sys/types.h>  /* for opendir etc. */
#ifndef WIN32
#include <dirent.h>     /* for readdir */
#else
#include "win32.h"
#include "dirent_w32.h"
#endif
#include <string.h>
#include <sys/stat.h>   /* for stat */


static void grpGetAcdFiles(AjPList glist, AjPList alpha, char * const env[],
			   const AjPStr acddir, AjBool explode, AjBool colon,
			   AjBool gui, AjBool embassy,
			   const AjPStr embassyname);
static void grpGetAcdDirs(AjPList glist, AjPList alpha, char * const env[],
			  const AjPStr acddir, AjBool explode, AjBool colon,
			  AjBool gui, AjBool embassy,
			  const AjPStr embassyname);
static void grpParse(AjPFile file, AjPStr *appl, AjPStr *doc, AjPStr* keywords,
		     AjPList groups, AjBool explode, AjBool colon,
		     AjBool *gui, AjBool* embassy, AjPStr* hasembassyname);
static void grpParseEmbassy(AjPFile file, AjPStr* embassyname);
static void grpNoComment(AjPStr* text);
static AjPStr grpParseValueRB(AjPStrTok* tokenhandle, const char* delim);
static void grpSplitList(AjPList groups, const AjPStr value, AjBool explode,
			 AjBool colon, AjPStr* keywords);
static void grpSubSplitList(AjPList groups, AjPList sublist);
static void grpAddGroupsToList(const AjPList alpha, AjPList glist,
			       const AjPList groups,
			       const AjPStr appl, const AjPStr doc,
			       const AjPStr keywords, const AjPStr package);
static AjBool grpGetAcdByname(const AjPStr appname, const AjPStr acddir,
			      AjPStr* embassyname);

static AjPStr grpStr1 = NULL;
static AjPStr grpStr2 = NULL;



/* @func embGrpGetProgGroups **************************************************
**
** Optionally constructs a path to the directory of normal EMBOSS or
** embassy ACD files. Calls grpGetAcdFiles to construct lists of the
** group, doc and program name information.
**
** @param [w] glist [AjPList] List of groups of programs
** @param [w] alpha [AjPList] Alphabetic list of programs
** @param [r] env [char* const[]] Environment passed in from C main()
**                                 parameters
** @param [r] emboss [AjBool] Read in EMBOSS ACD data
** @param [r] embassy [AjBool] Read in EMBASSY ACD data
** @param [r] embassyname [const AjPStr] Name of embassy package.
**                                       default is to search for all
** @param [r] explode [AjBool] Expand group names around ':'
** @param [r] colon [AjBool] Retain ':' in group names
** @param [r] gui [AjBool] Only report programs that are OK in GUIs
** @return [void]
** @@
******************************************************************************/

void embGrpGetProgGroups(AjPList glist, AjPList alpha, char * const env[],
			 AjBool emboss, AjBool embassy,
			 const AjPStr embassyname,
			 AjBool explode, AjBool colon, AjBool gui)
{

    AjPStr acdroot     = NULL;
    AjPStr acdrootdir  = NULL;
    AjPStr acdrootinst = NULL;
    AjPStr acdpack     = NULL;
    AjPStr alphaname   = NULL;
    EmbPGroupTop gpnode; /* new member (first & only) of alpha being added */
    AjBool doneinstall = ajFalse;

    /* set up alpha programs group list */
    ajStrAssignC(&alphaname, "Alphabetic list of programs");
    gpnode = embGrpMakeNewGnode(alphaname);
    ajListPushAppend(alpha, gpnode);
    ajStrDel(&alphaname);


    /* look at all EMBOSS ACD files */
    acdpack     = ajStrNew();
    acdroot     = ajStrNew();
    acdrootdir  = ajStrNew();
    acdrootinst = ajStrNew();
    alphaname   = ajStrNew();

    ajNamRootPack(&acdpack);
    ajNamRootInstall(&acdrootinst);
    if(emboss)
    {
	if(ajNamGetValueC("acdroot", &acdroot))
	{
	    ajFileDirFix(&acdroot);
	    /*ajStrAppendC(&acdroot, "acd/");*/
	}
	else
	{
	    ajFileDirFix(&acdrootinst);
	    ajFmtPrintS(&acdroot, "%Sshare/%S/acd/", acdrootinst, acdpack);

	    if(ajFileDir(&acdroot))
		doneinstall = ajTrue;
	    else
	    {
		ajNamRoot(&acdrootdir);
		ajFileDirFix(&acdrootdir);
		ajFmtPrintS(&acdroot, "%Sacd/", acdrootdir);
	    }
	}

	/* normal EMBOSS ACD */
	grpGetAcdFiles(glist, alpha, env, acdroot, explode, colon,
		       gui, embassy, embassyname);
    }

    if(embassy && !doneinstall)
    {
	ajFileDirFix(&acdroot);

	/* EMBOSS install directory */
	ajFmtPrintS(&acdroot, "%Sshare/%S/acd/",
		    acdrootinst, acdpack);

	if(ajFileDir(&acdroot))
	    /* embassadir ACD files */
	    grpGetAcdFiles(glist, alpha, env, acdroot, explode, colon,
			   gui, embassy, embassyname);
	else
	{
	    /* look for all source directories */
	    ajNamRoot(&acdrootdir);
	    ajFileDirUp(&acdrootdir);
	    ajFmtPrintS(&acdroot, "%Sembassy/", acdrootdir);
	    /* embassadir ACD files */
	    grpGetAcdDirs(glist, alpha, env, acdroot, explode, colon,
			  gui, embassy, embassyname);
	}

    }

    /* sort the groups and alpha lists */
    embGrpSortGroupsList(glist);
    embGrpSortGroupsList(alpha);

    ajStrDel(&acdroot);
    ajStrDel(&acdrootdir);
    ajStrDel(&acdrootinst);
    ajStrDel(&alphaname);
    ajStrDel(&acdpack);

    return;
}



/* @func embGrpGetEmbassy *****************************************************
**
** Optionally constructs a path to the directory of normal EMBOSS or
** embassy ACD files. Calls grpGetAcdFiles to construct lists of the
** group, doc and program name information.
**
** @param [r] appname [const AjPStr] Application name
** @param [w] embassyname [AjPStr*] Embassy package attribute value,
**                                  or an empty string if in the main package
** @return [AjBool] ajTrue if an ACD file was found
** @@
******************************************************************************/

AjBool embGrpGetEmbassy(const AjPStr appname, AjPStr* embassyname)
{

    AjPStr acdroot     = NULL;
    AjPStr acdrootdir  = NULL;
    AjPStr acdrootinst = NULL;
    AjPStr acdpack     = NULL;
    AjPFile acdfile     = NULL;
    AjPStr filename    = NULL;
    AjBool ok = ajFalse;

    /* look at all EMBOSS ACD files */
    acdpack     = ajStrNew();
    acdroot     = ajStrNew();
    acdrootdir  = ajStrNew();
    acdrootinst = ajStrNew();

    ajNamRootPack(&acdpack);
    ajNamRootInstall(&acdrootinst);

    ajStrAssignC(embassyname, "");

    if(ajNamGetValueC("acdroot", &acdroot))
    {
	ajFileDirFix(&acdroot);
	/*ajStrAppendC(&acdroot, "acd/");*/
    }
    else
    {
	ajFileDirFix(&acdrootinst);
	ajFmtPrintS(&acdroot, "%Sshare/%S/acd/", acdrootinst, acdpack);
      
	if(!ajFileDir(&acdroot))
	{
	    ajNamRoot(&acdrootdir);
	    ajFileDirFix(&acdrootdir);
	    ajFmtPrintS(&acdroot, "%Sacd/", acdrootdir);
	}
    }
    
    /* normal EMBOSS ACD */
    ajFmtPrintS(&filename, "%S%S.acd", acdroot, appname);
    acdfile = ajFileNewIn(filename);
    if(acdfile) {
	grpParseEmbassy(acdfile, embassyname);
	ajFileClose(&acdfile);
	ok = ajTrue;
    }

    if(!ok)
    {
      /* look for all source directories */
      ajNamRoot(&acdrootdir);
      ajFileDirUp(&acdrootdir);
      ajFmtPrintS(&acdroot, "%Sembassy/", acdrootdir);
      /* embassadir ACD files */
      ok = grpGetAcdByname(appname, acdroot, embassyname);
    }

    ajStrDel(&acdroot);
    ajStrDel(&acdrootdir);
    ajStrDel(&acdrootinst);
    ajStrDel(&acdpack);
    ajStrDel(&filename);

    ajDebug("embGrpGetEmbassy ok:%B embassy '%S'\n",
	    ok, *embassyname);
    return ok;
}




/* @funcstatic grpGetAcdByname ************************************************
**
** Given a directory from main package or EMBASSY sources, it searches
** for directories of ACD files and passes processing on to
** grpGetAcdFiles
**
** @param [r] appname [const AjPStr] Application name
** @param [r] acddir [const AjPStr] Path of directory holding ACD files
**                                  to read in
** @param [w] embassyname [AjPStr*] Embassy package name
**                                  or empty string for main package
** @return [AjBool] ajTrue if an ACD file was found
** @@
******************************************************************************/

static AjBool grpGetAcdByname(const AjPStr appname, const AjPStr acddir,
			      AjPStr* embassyname)
{
    DIR *dirp;
    DIR *dirpa;
    struct dirent *dp;
    AjPStr dirname = NULL;
    AjPStr filename = NULL;
    AjPFile acdfile = NULL;
    AjBool ok = ajFalse;

    /* go through all the directories in this directory */
    dirp = opendir(ajStrGetPtr(acddir));

    if(!dirp)
      return ajFalse;		/* could be no embassy installed */
    ajDebug("grpGetAcdbyName '%S' '%S'\n", acddir, appname);

    for(dp = readdir(dirp); dp != NULL; dp = readdir(dirp))
    {
	if(dp->d_name[0] == '.')
	    continue;			/* don't want hidden files */
	ajFmtPrintS(&dirname, "%S%s/emboss_acd/", acddir, dp->d_name);

	dirpa = opendir(ajStrGetPtr(dirname));
	if(dirpa)
	{
	    closedir(dirpa);
	    ajFmtPrintS(&filename, "%S%S.acd", dirname, appname);
	    acdfile = ajFileNewIn(filename);
	    if(acdfile) {
	      grpParseEmbassy(acdfile, embassyname);
	      ajFileClose(&acdfile);
	      ok = ajTrue;
	    }
	}
        if(ok)
	  break;
    }
    ajStrDel(&dirname);
    ajStrDel(&filename);
    closedir(dirp);

    return ok;
}



/* @funcstatic grpGetAcdDirs **************************************************
**
** Given a directory from EMBASSY sources, it searches for directories
** of ACD files and passes processing on to grpGetAcdFiles
**
** @param [w] glist [AjPList] List of groups of programs
** @param [w] alpha [AjPList] Alphabetic list of programs
** @param [r] env [char* const[]] Environment passed in from C main()
**                                 parameters
** @param [r] acddir [const AjPStr] path of directory holding ACD files
**                                  to read in
** @param [r] explode [AjBool] Expand group names around ':'
** @param [r] colon [AjBool] Retain ':' in group names
** @param [r] gui [AjBool] Report only those applications OK in GUIs
** @param [r] embassy [AjBool] Report only those applications not in
**                             an EMBASSY package (embassy attribute in ACD)
** @param [r] embassyname [const AjPStr] Name of embassy package.
**                                       default is to search for all
** @return [void]
** @@
******************************************************************************/

static void grpGetAcdDirs(AjPList glist, AjPList alpha, char * const env[],
			  const AjPStr acddir, AjBool explode, AjBool colon,
			  AjBool gui, AjBool embassy,
			  const AjPStr embassyname)
{
    DIR *dirp;
    DIR *dirpa;
    struct dirent *dp;
    static AjPStr dirname = NULL;


    /* go through all the directories in this directory */
    if((dirp = opendir(ajStrGetPtr(acddir))) == NULL)
	return;			   /* could be no embassy installed */


    for(dp = readdir(dirp); dp != NULL; dp = readdir(dirp))
    {
	if(dp->d_name[0] == '.')
	    continue;			/* don't want hidden files */
	ajFmtPrintS(&dirname, "%S%s/emboss_acd/", acddir, dp->d_name);

	if((dirpa = opendir(ajStrGetPtr(dirname))))
	{
	    grpGetAcdFiles(glist, alpha, env, dirname, explode, colon,
			   gui, embassy, embassyname);
	    closedir(dirpa);
	}
    }

    closedir(dirp);

    return;
}




/* @funcstatic grpGetAcdFiles *************************************************
**
** Given a directory, it searches for ACD files which describe an
** existing program on the path,
** parses out the documentation and groups from these ACD files,
** returns a list of program names and documentation grouped by group names,
** and returns an alphabetic list of program names and documentation.
**
** @param [w] glist [AjPList] List of groups of programs
** @param [w] alpha [AjPList] Alphabetic list of programs
** @param [r] env [char* const[]] Environment passed in from C main()
**                             parameters
** @param [r] acddir [const AjPStr] path of directory holding ACD files
**                           to read in
** @param [r] explode [AjBool] Expand group names around ':'
** @param [r] colon [AjBool] Retain ':' in group names
** @param [r] gui [AjBool] Report only those applications OK in GUIs
** @param [r] embassy [AjBool] Report only those applications not in
**                             an EMBASSY package (embassy attribute in ACD)
** @param [r] embassyname [const AjPStr] Name of embassy package.
**                                       default is to search for all
** @return [void]
** @@
******************************************************************************/

static void grpGetAcdFiles(AjPList glist, AjPList alpha, char * const env[],
			   const AjPStr acddir, AjBool explode, AjBool colon,
			   AjBool gui, AjBool embassy,
			   const AjPStr embassyname)
{
    DIR *dirp;
    struct dirent *dp;
    AjPStr progpath = NULL;
    AjPFile file    = NULL;
    AjPStr appl     = NULL;
    AjPStr applpath = NULL;		/* path of application */
    AjPStr doc      = NULL;
    AjPList groups  = NULL;
    AjPStr keywords = NULL;
    AjBool guiresult;
    AjBool isembassy;
    AjPStr hasembassyname = NULL;

    /* go through all the files in this directory */
    if((dirp = opendir(ajStrGetPtr(acddir))) == NULL)
	ajFatal("You do not have read permission on the directory '%S'",
		acddir);

    for(dp = readdir(dirp); dp != NULL; dp = readdir(dirp))
    {
	if(dp->d_name[0] != '.')
	{
	    ajStrAssignResS(&progpath,
			    ajStrGetLen(acddir)+strlen(dp->d_name)+3,
			    acddir);
	    ajStrAppendC(&progpath, dp->d_name);

	    /* does it end with ".acd" ? */
	    if(ajStrSuffixC(progpath, ".acd"))
	    {
		/* see if it is a normal file */
		if(ajFileNameValid(progpath))
		{
		    /* open the file and parse it */
		    if((file = ajFileNewIn(progpath)) != NULL)
		    {
			groups = ajListstrNew();
			grpParse(file, &appl, &doc, &keywords, groups,
				 explode, colon, &guiresult,
				 &isembassy, &hasembassyname);

			/* see if the appl is the name of a real program */
			ajStrAssignS(&applpath, appl);
			if(ajSysFileWhichEnv(&applpath, env))
			{
			    /*
			    ** see if the appl is OK in GUIs or we don't
			    ** want just GUI apps
			    */
			    if(gui && !guiresult)
				ajDebug("%S is not a OK in GUIs\n", appl);
			    else if(!embassy && isembassy)
				ajDebug("%S is in EMBASSY\n", appl);
			    else if (ajStrGetLen(embassyname) &&
				     !ajStrMatchCaseS(embassyname,
						      hasembassyname))
				ajDebug("%S is in not in EMBASSY %S\n",
					appl, embassyname);
			    else
				grpAddGroupsToList(alpha, glist, groups,
						   appl, doc, keywords,
						   hasembassyname);
			}

			ajFileClose(&file);
			ajListstrFreeData(&groups);
			ajStrDel(&appl);
			ajStrDel(&doc);
			ajStrDel(&hasembassyname);
		    }
		}
	    }
	    ajStrDel(&progpath);
	}
    }

    closedir(dirp);
    ajStrDel(&applpath);
    ajStrDel(&keywords);

    return;
}

/* @funcstatic grpParse *******************************************************
**
** parse the acd file.
**
** @param [u] file [AjPFile]  ACD file
** @param [w] appl [AjPStr*] Application name
** @param [w] doc  [AjPStr*]  Documentation string
** @param [w] keywords  [AjPStr*]  Keywords string
** @param [w] groups [AjPList] Program groups string
** @param [r] explode [AjBool] Expand group names around ':'
** @param [r] colon [AjBool] Retain ':' in group names
** @param [w] gui [AjBool*] returns ajTrue if application is OK in GUIs
** @param [w] embassy [AjBool*] returns ajTrue if application has
**                              an EMBASSY package definition
**
** @param [w] hasembassyname [AjPStr*] EMBASSY package name from
**                                     embassy attribute
** @return [void]
** @@
******************************************************************************/

static void grpParse(AjPFile file, AjPStr *appl, AjPStr *doc, AjPStr *keywords,
		     AjPList groups,
		     AjBool explode, AjBool colon,
		     AjBool *gui, AjBool* embassy, AjPStr* hasembassyname)
{

    AjPStr line = NULL;
    AjPStr text = NULL;

    AjPStrTok tokenhandle;
    char white[]     = " \t\n\r";
    char whiteplus[] = " \t\n\r:=";
    AjPStr tmpstr = NULL;
    AjPStr token  = NULL;
    AjPStr value  = NULL;
    ajint done = 0;
    AjPStr nullgroup = NULL;
    AjPStr newstr    = NULL;
    AjPStr tmpvalue  = NULL;

    /* initialise a name for programs with no assigned group */
    ajStrAppendC(&nullgroup, "ASSORTED");

    ajStrAssignC(keywords, "");

    /* if 'gui' not defined in ACD, default is 'gui: Y' */
    *gui = ajTrue;
    *embassy = ajFalse;

    /* read file into one line, stripping out comment lines and blanks */
    while(ajFileReadLine(file, &line))
    {
	grpNoComment(&line);
	if(ajStrGetLen(line))
	{
	    ajStrAppendS(&text, line);
	    ajStrAppendC(&text, " ");
	}
    }

    tokenhandle = ajStrTokenNewC(text, white);

    /* find appl token */
    while(ajStrTokenNextParseC(&tokenhandle, whiteplus, &tmpstr))
	if(ajStrPrefixCaseC(tmpstr, "appl"))
	    break;

    /* next token is the application name */
    ajStrTokenNextParseC(&tokenhandle, white, appl);

    /* if next token is '[' */
    ajStrTokenNextParseC(&tokenhandle, white, &tmpstr);
    if(ajStrCmpC(tmpstr, "[") == 0)
    {
	token=ajStrNew();

	/* is the next token 'doc' or 'groups' or 'gui' */
	while(ajStrTokenNextParseC(&tokenhandle, whiteplus, &tmpstr))
	{
	    while(!ajStrMatchC(tmpstr, "]"))
	    {
		ajStrAssignS(&token, tmpstr);
		value = grpParseValueRB(&tokenhandle, white);
		done = ajStrMatchC(value, "]");

		if(!done)
		{
		    ajStrTokenNextParseC(&tokenhandle, whiteplus, &tmpstr);
		    ajStrFmtLower(&tmpstr);
		    done = ajStrMatchC(tmpstr, "]");
		}

		if(ajStrPrefixCaseC(token, "documentation"))
		{
		    ajStrAssignS(doc, value);
		    ajStrTrimWhite(doc);
		    ajStrTrimC(doc, ".,");

		}
		else if(ajStrPrefixCaseC(token, "gui"))
		{
		    ajStrAssignS(&tmpvalue, value);
		    ajStrTrimWhite(&tmpvalue);
		    ajDebug("gui value '%S'\n", tmpvalue);
		    /* test for '[Nn]*' */
		    if(tolower((int)(ajStrGetPtr(tmpvalue))[0]) == 'n')
			*gui = ajFalse;

		    ajStrDel(&tmpvalue);
		}
		else if(ajStrPrefixCaseC(token, "groups"))
		{
		    grpSplitList(groups, value, explode, colon, keywords);
		}
		else if(ajStrPrefixCaseC(token, "keywords"))
		{
		    ajStrExchangeKK(&value, ' ', '_');
		    if(ajStrGetLen(*keywords))
			ajStrAppendK(keywords, ' ');
		    ajStrAppendS(keywords, value);
		}
		else if(ajStrPrefixCaseC(token, "embassy"))
		{
		    *embassy = ajTrue;
		    ajStrAssignS(hasembassyname, value);
		}
	    }
	    if(done)
		break;
	}
    }

    /* check that we got the doc, keywords and groups descriptions */
    if(!ajStrGetLen(*doc))
	ajStrAssignC(doc, "");

    if(!ajStrGetLen(*keywords))
	ajStrAssignC(keywords, "");

    if(!ajListGetLength(groups))
    {
	newstr = ajStrNewRef(nullgroup);
	ajListstrPushAppend(groups, newstr);
    }

    ajStrAssignEmptyC(hasembassyname, "");

    ajStrDel(&nullgroup);
    ajStrDel(&tmpstr);
    ajStrDel(&line);
    ajStrDel(&text);
    ajStrTokenDel(&tokenhandle);
    ajStrDel(&token);
    ajStrDel(&nullgroup);

    return;
}



/* @funcstatic grpParseEmbassy ***********************************************
**
** parse the acd file to get the EMBASSY application attribute
**
** @param [u] file [AjPFile]  ACD file
** @param [w] embassyname [AjPStr*] EMBASSY package name from
**                                     embassy attribute
** @return [void]
** @@
******************************************************************************/

static void grpParseEmbassy(AjPFile file, AjPStr* embassyname)
{

    AjPStr line = NULL;
    AjPStr text = NULL;

    AjPStrTok tokenhandle;
    char white[]     = " \t\n\r";
    char whiteplus[] = " \t\n\r:=";
    AjPStr tmpstr = NULL;
    AjPStr token  = NULL;
    AjPStr value  = NULL;
    ajint done = 0;

    ajStrAssignC(embassyname, "");

    /* read file into one line, stripping out comment lines and blanks */
    while(ajFileReadLine(file, &line))
    {
	grpNoComment(&line);
	if(ajStrGetLen(line))
	{
	    ajStrAppendS(&text, line);
	    ajStrAppendC(&text, " ");
	}
    }

    tokenhandle = ajStrTokenNewC(text, white);

    /* find appl token */
    while(ajStrTokenNextParseC(&tokenhandle, whiteplus, &tmpstr))
	if(ajStrPrefixCaseC(tmpstr, "appl"))
	    break;

    /* next token is the application name */
    ajStrTokenNextParseC(&tokenhandle, white, &tmpstr);

    /* if next token is '[' */
    ajStrTokenNextParseC(&tokenhandle, white, &tmpstr);
    if(ajStrCmpC(tmpstr, "[") == 0)
    {
	token=ajStrNew();

	/* is the next token 'doc' or 'groups' or 'gui' */
	while(ajStrTokenNextParseC(&tokenhandle, whiteplus, &tmpstr))
	{
	    while(!ajStrMatchC(tmpstr, "]"))
	    {
		ajStrAssignS(&token, tmpstr);
		value = grpParseValueRB(&tokenhandle, white);
		done = ajStrMatchC(value, "]");

		if(!done)
		{
		    ajStrTokenNextParseC(&tokenhandle, whiteplus, &tmpstr);
		    ajStrFmtLower(&tmpstr);
		    done = ajStrMatchC(tmpstr, "]");
		}

		if(ajStrPrefixCaseC(token, "embassy"))
		{
		    ajStrAssignS(embassyname, value);
		}
	    }
	    if(done)
		break;
	}
    }

    ajStrDel(&tmpstr);
    ajStrDel(&line);
    ajStrDel(&text);
    ajStrTokenDel(&tokenhandle);

    return;
}




/* @funcstatic grpNoComment ***************************************************
**
** Strips comments from a character string (a line from an trn file).
** Comments are blank lines or any text following a "#" character.
** Whitespace characters can be included in a blank line.
**
** @param [u] text [AjPStr*] Line of text from input file
** @return [void]
** @@
******************************************************************************/

static void grpNoComment(AjPStr* text)
{
    ajint i;
    char *cp;

    ajStrTrimWhite(text);
    i = ajStrGetLen(*text);

    if(!i)				/* empty string */
	return;

    cp = strchr(ajStrGetPtr(*text), '#');
    if(cp)
    {					/* comment found */
	*cp = '\0';
	ajStrSetValid(text);
    }

    return;
}




/* @funcstatic grpParseValueRB ************************************************
**
** Copied from ajacd.c
**
** Uses ajStrTok to complete a (possibly) quoted value.
** Note that ajStrTok has a stored internal copy of the text string
** which is set up at the start of acdParse and is being used here.
**
** Quotes can be single or double, or any kind of parentheses,
** depending on the first character of the next token examined.
**
** @param [u] tokenhandle [AjPStrTok*] Current parsing handle for input text
** @param [r] delim [const char*] Delimiter string
** @return [AjPStr] String containing next value using acdStrTok
** @@
******************************************************************************/

static AjPStr grpParseValueRB(AjPStrTok* tokenhandle, const char* delim)
{
    char  endq[]   = " ";
    char  endqbr[] = " ]";
    ajint iquote;
    char *cq;
    AjBool done   = ajFalse;
    AjBool rightb = ajFalse;

    const char *quotes    = "\"'{(<";
    const char *endquotes = "\"'})>";

    if(!ajStrTokenNextParseC(tokenhandle, delim, &grpStr1))
	return NULL;

    cq = strchr(quotes, ajStrGetCharFirst(grpStr1));
    if(!cq)
	return grpStr1;


    /* quote found: parse up to closing quote then strip white space */

    ajStrDelStatic(&grpStr2);

    iquote = (ajint) (cq - quotes);
    endq[0] = endqbr[0] = endquotes[iquote];
    ajStrCutStart(&grpStr1, 1);

    while(!done)
    {
	if(ajStrSuffixC(grpStr1, endq))
	{
	    ajStrCutEnd(&grpStr1, 1);
	    done = ajTrue;
	}

	if(ajStrSuffixC(grpStr1, endqbr))
	{
	    ajStrCutEnd(&grpStr1, 2);
	    rightb = ajTrue;
	    done = ajTrue;
	}

	if(ajStrGetLen(grpStr1))
	{
	    if(ajStrGetLen(grpStr2))
		ajStrAppendC(&grpStr2, " ");
	    ajStrAppendS(&grpStr2, grpStr1);
	}

	if(!done)
	    if(!ajStrTokenNextParseC(tokenhandle, delim, &grpStr1))
		return NULL;
    }

    if(rightb)
	ajStrAppendC(&grpStr2, "]");

    return grpStr2;
}




/* @funcstatic grpSplitList ***************************************************
**
** Split a string containing group names into a list on the delimiters
** ',' or ';' to form the primary names of the groups.
** Any names containing a colon ':' are optionally expanded in a call to
** grpSubSplitList() to form many combinations of group names.
**
** The group names are returned as a list.
**
** @param [u] groups [AjPList] List of groups
** @param [r]  value  [const AjPStr] Groups string from ACD file
** @param [r]  explode [AjBool] Expand group names around ':'
** @param [r]  colon [AjBool] Retain ':' in group names
** @param [u]  keywords [AjPStr*] List of keywords
**
** @return [void]
** @@
******************************************************************************/

static void grpSplitList(AjPList groups, const AjPStr value, AjBool explode,
			 AjBool colon, AjPStr *keywords)
{
    AjPStrTok colontokenhandle;
    AjPStrTok tokenhandle;
    char delim[]       = ",;";
    char colonstring[] = ":";
    AjPList subnames;
    AjPStr tmpstr  = NULL;
    AjPStr substr  = NULL;
    AjPStr copystr = NULL;
    AjPStr keystr  = NULL;

    tokenhandle = ajStrTokenNewC(value, delim);

    while(ajStrTokenNextParse(&tokenhandle, &tmpstr))
    {
	ajStrTrimWhite(&tmpstr);
	ajStrTrimC(&tmpstr, ".");

	ajStrAssignS(&keystr, tmpstr);
	ajStrExchangeKK(&keystr, ':', '_');
	ajStrExchangeKK(&keystr, ' ', '_');
	if(ajStrGetLen(*keywords))
	    ajStrAppendK(keywords, ' ');
	ajStrAppendS(keywords, keystr);

	/*
	** split the group name on colons and expand the sub-names into several
	** combinations of name
	*/
	if(explode)
	{
	    subnames = ajListstrNew();
	    colontokenhandle  = ajStrTokenNewC(tmpstr, colonstring);
	    while(ajStrTokenNextParse(&colontokenhandle, &substr))
	    {
		copystr = ajStrNewS(substr); /* make new copy of the string
					       for the list to hold */
		ajStrTrimWhite(&copystr);
		ajListstrPushAppend(subnames, copystr);
	    }

	    /*
	    ** make the combinations of sub-names and add them to the list of
	    ** group names
	    */
	    grpSubSplitList(groups, subnames);

	    ajStrTokenDel(&colontokenhandle);
	    ajStrDel(&substr);
	    ajListstrFreeData(&subnames);
	    /*
	     ** don't free up copystr - because ajListstrFreeData()
	     ** then tries to free
	     ** it as well ajStrDel(&copystr);
	     */

	}
	else
	{
	    /*
	    ** don't explode, just remove ':'s and excess spaces and add to
	    ** 'groups' list
	    */
	    copystr = ajStrNewRef(tmpstr);	/* make new copy of the string
					   for the list to hold */
	    /*
	     ** we might want to retain the ':' in the output
	     ** if it is being parsed by
	     ** other programs that create 2-level menus for an interface etc.
	     */
	    if(!colon)
	    {
		ajStrExchangeSetCC(&copystr, ":", " ");
		ajStrRemoveWhiteExcess(&copystr);
	    }
	    else
	    {
		/* tidy up spurious spaces around the colon */
		ajStrRemoveWhiteExcess(&copystr);
		ajStrExchangeCC(&copystr, " :", ":");
		ajStrExchangeCC(&copystr, ": ", ":");
	    }

	    ajListstrPushAppend(groups, copystr);
	}
    }

    ajStrTokenDel(&tokenhandle);
    ajStrDel(&tmpstr);
    ajStrDel(&substr);
    ajStrDel(&keystr);

    return;
}




/* @funcstatic grpSubSplitList ************************************************
**
** Takes a list of words and makes several combinations of them to
** construct the expanded group constructs made from the ':' operator in
** the group names.
**
** For example, the group name 'aaa:bbb:ccc' will be passed over to this
** routine as the list 'aaa', 'bbb', 'ccc' and the group names:
** 'aaa bbb ccc'
** 'ccc bbb aaa'
** 'aaa bbb'
** 'bbb aaa'
** 'bbb ccc'
** 'ccc bbb'
** 'ccc'
** 'bbb'
** 'aaa'
** will be constructed and added to the list of group names in 'groups'
**
** @param [u] groups [AjPList] List of groups
** @param [u] sublist [AjPList] (Sub)-names of groups string from ACD file
**
** @return [void]
** @@
******************************************************************************/

static void grpSubSplitList(AjPList groups, AjPList sublist)
{
    AjPStr *sub;			/* array of sub-names */
    ajint len;			    /* length of array of sub-names */
    ajint i;
    ajint j;
    AjPStr head;			/* constructed group names */
    AjPStr tail;
    AjPStr revhead;
    AjPStr revtail;
    AjPStr dummy;		 /* dummy string for ajListstrPop() */


    len = ajListstrToarray(sublist, &sub);

    for(i=0; i<len; i++)
    {
	/* do head of list */
	head = ajStrNew();
	for(j=0; j<=i; j++)
	{
	    if(ajStrGetLen(head) > 0)
		ajStrAppendC(&head, " ");
	    ajStrAppendS(&head, sub[j]);
	}

	ajListstrPushAppend(groups, head);

	/*
	** do reverse head of list if there is more than
	** one name in the head
	*/
	if(i)
	{
	    revhead = ajStrNew();
	    for(j=i; j>=0; j--)
	    {
		if(ajStrGetLen(revhead) > 0)
		    ajStrAppendC(&revhead, " ");
		ajStrAppendS(&revhead, sub[j]);
	    }

	    ajListstrPushAppend(groups, revhead);
	}

	/* do tail of list, if there is any tail left */
	if(i < len-1)
	{
	    tail = ajStrNew();
	    for(j=i+1; j<len; j++)
	    {
		if(ajStrGetLen(tail) > 0)
		    ajStrAppendC(&tail, " ");
		ajStrAppendS(&tail, sub[j]);
	    }

	    ajListstrPushAppend(groups, tail);
	}

	/*
	** do reverse tail of list if there is more than
	** one name in the tail
	*/
	if(i < len-2)
	{
	    revtail = ajStrNew();
	    for(j=len-1; j>i; j--)
	    {
		if(ajStrGetLen(revtail) > 0) ajStrAppendC(&revtail, " ");
		ajStrAppendS(&revtail, sub[j]);
	    }

	    ajListstrPushAppend(groups, revtail);
	}

    }

    AJFREE(sub);

    /* if list length is greater than 2, pop off head and tail and recurse */
    if(len > 2)
    {
	ajListstrPop(sublist, &dummy);	/* remove first node */
	ajStrDel(&dummy);

	/* remove last node of list. */

	ajListstrReverse(sublist);
	ajListstrPop(sublist, &dummy);	/* remove first node */
	ajStrDel(&dummy);
	ajListstrReverse(sublist);

	/* recurse */
	grpSubSplitList(groups, sublist);
    }

    return;
}




/* @funcstatic grpAddGroupsToList *********************************************
**
** Add application to applications list and its groups to the full groups list
** Results are alpha list and glist.
**
** alpha is a list of application with sub-lists of their groups
** glist is a list of groups with sub-lists of their applications
**
** @param [r] alpha [const AjPList] Alphabetic list of programs
** @param [u] glist [AjPList] List of all known groups
** @param [r] groups [const AjPList] List of groups for this application
** @param [r]  appl  [const AjPStr] Application name
** @param [r]  doc  [const AjPStr] Documentation string
** @param [r] keywords [const AjPStr] List of keywords for this application
** @param [r]  package  [const AjPStr] Name of package
**
** @return [void]
** @@
******************************************************************************/

static void grpAddGroupsToList(const AjPList alpha, AjPList glist,
			       const AjPList groups,
			       const AjPStr appl, const AjPStr doc,
			       const AjPStr keywords, const AjPStr package)
{
    AjPStr g = NULL;	/* temporary value of member of groups list */
    AjIList aiter;	/* 'alpha' iterator */
    AjIList iter;	/* 'groups' iterator */
    AjIList giter;	/* 'glist' iterator */
    AjIList niter;	/* 'nlist' iterator */
    EmbPGroupTop al;	/* next (first) member of alpha */
    EmbPGroupTop gl;	/* next member of glist */
    EmbPGroupTop nl;	/* next member of nlist */
    EmbPGroupTop gpnode;	/* new member of glist being added */
    EmbPGroupProg ppnode;	/* new member of plist being added */
    EmbPGroupProg apnode;	/* new member of alpha list being added */
    AjPList nlist=NULL;	/* list of programs in a group - used to check
			   name is unique */
    AjBool foundit;	/* flag for found the program name */


    /* add this program to the alphabetic list of programs */
    apnode = embGrpMakeNewPnode(appl, doc, keywords, package);
    aiter = ajListIterNewread(alpha);
    al = ajListIterGet(aiter);
    ajListPushAppend(al->progs, apnode);
    ajListIterDel(&aiter);

    /*
    ** Now step through all groups that this program belongs to and add this
    ** program to the groups
    */
    iter = ajListIterNewread(groups);

    while((g = ajListIterGet(iter)) != NULL)
    {
	/* add the group name to the program node in alpha list */
	gpnode = embGrpMakeNewGnode(g);
	ajListPushAppend(apnode->groups, gpnode);

	/* add the application to the appropriate groups list in glist */
	giter = ajListIterNewread(glist);

	while((gl = ajListIterGet(giter)) != NULL)
	{
	    /* is this our group ? */
	    if(!ajStrCmpCaseS(gl->name, g))
	    {
		/*
		** found the group.
		** look through the program names in this group
		** and only add if name is
		** not already there
		*/
		foundit = ajFalse;
		nlist   = gl->progs;
		niter   = ajListIterNewread(nlist);
		while((nl = ajListIterGet(niter)) != NULL)
		{
		    if(!ajStrCmpCaseS(nl->name, appl))
		    {
			/* found the program name */
			foundit = ajTrue;
			break;
		    }
		}
		ajListIterDel(&niter);

		if(!foundit)
		{
		    ppnode = embGrpMakeNewPnode(appl, doc, keywords, package);
		    ajListPushAppend(gl->progs, ppnode);
		}
		break;
	    }
	}

	if(gl == NULL)
	{
	    /* went past the end of the group list */
	    gpnode = embGrpMakeNewGnode(g);
	    ajListPushAppend(glist, gpnode);
	    ppnode = embGrpMakeNewPnode(appl, doc, keywords, package);
	    ajListPushAppend(gpnode->progs, ppnode);
	}
	ajListIterDel(&giter);
    }

    ajListIterDel(&iter);
    ajStrDel(&g);

    /* sort the groups for this application in alpha list */
    embGrpSortGroupsList(apnode->groups);

    return;
}




/* @func embGrpMakeNewGnode ***************************************************
**
** Creates a new pointer to a Gnode struct for holding a group's
** name and pointer to a list of programs (also held in Gnodes).
**
** @param [r] name [const AjPStr] Name of the group
** @return [EmbPGroupTop] pointer to a new GPnode struct
** @@
******************************************************************************/

EmbPGroupTop embGrpMakeNewGnode(const AjPStr name)
{
    EmbPGroupTop gpnode;
    AjPStr newstr = NULL;
    AjPStr dummy  = NULL;


    /*  ajDebug("New groups gnode name=%S\n", name); */
    AJNEW0(gpnode);

    newstr = ajStrNewS(name);
    ajStrFmtUpper(&newstr);

    gpnode->name = newstr;
    ajStrAssignC(&dummy, "");
    gpnode->doc = dummy;
    gpnode->progs = ajListNew();

    return gpnode;
}




/* @func embGrpMakeNewPnode ***************************************************
**
** Creates a new pointer to a Gnode struct for holding a program's
** name and documentation.
**
** @param [r] name [const AjPStr] Name of the program
** @param [r] doc [const AjPStr] Description of the program
** @param [r] keywords [const AjPStr] Keywords for this program
**                                    with underscores for spaces and
**                                    with spaces as separators
** @param [r] package [const AjPStr] Name of the package
** @return [EmbPGroupProg] pointer to a new gnode struct
** @@
******************************************************************************/

EmbPGroupProg embGrpMakeNewPnode(const AjPStr name, const AjPStr doc,
				 const AjPStr keywords, const AjPStr package)
{
    EmbPGroupProg gpnode;

    AJNEW0(gpnode);
    gpnode->name    = ajStrNewS(name);
    gpnode->doc     = ajStrNewS(doc);
    gpnode->keywords= ajStrNewS(keywords);
    gpnode->package = ajStrNewS(package);
    gpnode->groups  = ajListNew();

    return gpnode;
}




/* @func embGrpSortGroupsList *************************************************
**
** Sort a list of GPnodes by their name.
**
** @param [u] groupslist [AjPList] List to sort
** @return [void]
** @@
******************************************************************************/

void embGrpSortGroupsList(AjPList groupslist)
{
    EmbPGroupTop gl;
    AjIList giter;

    /* sort the programs for each group */
    giter = ajListIterNewread(groupslist);

    while((gl = ajListIterGet(giter)) != NULL)
	ajListSort(gl->progs, embGrpCompareTwoPnodes);


    ajListIterDel(&giter);

    /* sort the groups themselves */
    ajListSort(groupslist, embGrpCompareTwoGnodes);

    return;
}




/* @func embGrpSortProgsList *************************************************
**
** Sort a list of Pnodes by their name.
**
** @param [u] progslist [AjPList] List to sort
** @return [void]
** @@
******************************************************************************/

void embGrpSortProgsList(AjPList progslist)
{
    EmbPGroupProg pl;
    AjIList piter;

    /* sort the groups for each program */
    piter = ajListIterNewread(progslist);

    while((pl = ajListIterGet(piter)) != NULL)
	ajListSort(pl->groups, embGrpCompareTwoGnodes);


    ajListIterDel(&piter);

    /* sort the groups themselves */
    ajListSort(progslist, embGrpCompareTwoPnodes);

    return;
}




/* @func embGrpCompareTwoGnodes ***********************************************
**
** Compare two Gnodes as case-insensitive strings.
**
** @param [r] a [const void *] First node
** @param [r] b [const void *] Second node
**
** @return [ajint] Compare value (-1, 0, +1)
** @@
******************************************************************************/

ajint embGrpCompareTwoGnodes(const void * a, const void * b)
{
    return ajStrCmpCaseS((*(EmbPGroupTop const *)a)->name,
			 (*(EmbPGroupTop const *)b)->name);
}




/* @func embGrpCompareTwoPnodes ***********************************************
**
** Compare two Pnodes as case-insensitive strings.
**
** @param [r] a [const void *] First node
** @param [r] b [const void *] Second node
**
** @return [ajint] Compare value (-1, 0, +1)
** @@
******************************************************************************/

ajint embGrpCompareTwoPnodes(const void * a, const void * b)
{
    return ajStrCmpCaseS((*(EmbPGroupProg const *)a)->name,
			 (*(EmbPGroupProg const *)b)->name);
}




/* @func embGrpOutputGroupsList ***********************************************
**
** Displays a list of groups to an output file handle.
**
** @param [u] outfile [AjPFile] Output file handle
** @param [r] groupslist [const AjPList] List of groups to be displayed
** @param [r] showprogs [AjBool] If True, display the programs in each group
** @param [r] html [AjBool] If True, format for HTML, else make a simple list
** @param [r] showkey [AjBool] If True, show keywords
** @param [r] package [const AjPStr] Name of current package
** @return [void]
** @@
******************************************************************************/

void embGrpOutputGroupsList(AjPFile outfile, const AjPList groupslist,
			    AjBool showprogs, AjBool html,
			    AjBool showkey, const AjPStr package)
{
    EmbPGroupTop gl;
    AjIList giter;			/* 'groupslist' iterator */

    /* output the programs for each group */
    giter = ajListIterNewread(groupslist);
    if(!showprogs && html)
	ajFmtPrintF(outfile,"<ul>\n");

    while((gl = ajListIterGet(giter)) != NULL)
    {
	if(html)
	{
	    if(showprogs)
		ajFmtPrintF(outfile,"<h2><a name=\"%S\">%S</a></h2>\n",
			    gl->name, gl->name);
	    else
	    {
		ajFmtPrintF(outfile,"<li><a href=\"%S.html\">%S</a></li>\n",
				gl->name,gl->name);
	    }
	}
	else
	    ajFmtPrintF(outfile,"%S\n", gl->name);

	if(showprogs)
	{
	    if(html) ajFmtPrintF(outfile,"<table border cellpadding=4 "
				 "bgcolor=\"#FFFFF0\">\n");
	    embGrpOutputProgsList(outfile, gl->progs, html, showkey, package);
	    if(html)
		ajFmtPrintF(outfile,"</table>\n");
	    else
		ajFmtPrintF(outfile,"\n");
	}
    }

    if(!showprogs && html)
	ajFmtPrintF(outfile,"</ul>\n");
    ajListIterDel(&giter);

    return;
}




/* @func embGrpOutputProgsList ************************************************
**
** Displays a list of programs and their descriptions to an output file handle.
**
** @param [u] outfile [AjPFile] Output file handle
** @param [r] progslist [const AjPList] List of programs to be displayed
** @param [r] html [AjBool] If True, format for HTML, else make a simple list
** @param [r] showkey [AjBool] Show keywords in output
** @param [r] package [const AjPStr] Name of current package
** @return [void]
** @@
******************************************************************************/

void embGrpOutputProgsList(AjPFile outfile, const AjPList progslist,
			   AjBool html, AjBool showkey, const AjPStr package)
{
    EmbPGroupProg pl;
    AjIList piter;			/* 'progslist' iterator */
    AjPStr keystr = NULL;

    /* output the programs for each group */
    piter = ajListIterNewread(progslist);
    if(html) ajFmtPrintF(outfile,
			 "<tr><th>Program name</th><th>Description"
			 "</th></tr>\n");

    while((pl = ajListIterGet(piter)) != NULL)
    {
	if(showkey && ajStrGetLen(pl->keywords)) {
	    ajFmtPrintS(&keystr, "(%S)", pl->keywords);
	    ajStrExchangeKK(&keystr, ' ', ',');
	    ajStrExchangeKK(&keystr, '_', ' ');
	    ajStrInsertK(&keystr, 0, ' ');
	}
	else
	{
	    ajStrAssignC(&keystr, "");
	}

	if(html)
	{
	    ajFmtPrintF(outfile, "<tr>\n");

	    if(ajStrMatchCaseS(package, pl->package))
		ajFmtPrintF(outfile,
			    "<td><a href=\"%S.html\">%S</a></td>\n",
			    pl->name, pl->name);
	    else if(ajStrGetLen(pl->package))
		ajFmtPrintF(outfile,
			    "<td><a href=\"/embassy/%S/%S.html\">%S</a></td>\n",
			    pl->package, pl->name, pl->name);
	    else
		ajFmtPrintF(outfile,
			    "<td><a href=\"/emboss/apps/%S.html\">%S</a></td>\n",
			    pl->name, pl->name);
	    ajFmtPrintF(outfile,
			"<td>%S%S</td>\n</tr>\n\n",
			pl->doc, keystr);
	}
	else
	    ajFmtPrintF(outfile, "%-16S %S%S\n", pl->name, pl->doc, keystr);
    }

    ajListIterDel(&piter);

    ajStrDel(&keystr);

    return;
}




/* @func embGrpGroupsListDel **************************************************
**
** Destructor for a groups list
**
** @param [d] groupslist [AjPList*] List of groups to be destroyed
** @return [void]
** @@
******************************************************************************/

void embGrpGroupsListDel(AjPList *groupslist)
{
    EmbPGroupTop gl;
    AjIList giter;

    giter = ajListIterNew(*groupslist);
    while((gl = ajListIterGet(giter)) != NULL)
    {
	ajStrDel(&(gl->doc));
	ajStrDel(&(gl->name));
	embGrpProgsListDel(&(gl->progs));
	AJFREE(gl);
    }

    ajListIterDel(&giter);
    ajListFree(groupslist);

    return;
}




/* @func embGrpProgsListDel ***************************************************
**
** Destructor for a groups list
**
** @param [d] progslist [AjPList*] List of programss to be destroyed
** @return [void]
** @@
******************************************************************************/

void embGrpProgsListDel(AjPList *progslist)
{
    EmbPGroupProg gl;
    AjIList piter;

    piter = ajListIterNew(*progslist);
    while((gl = ajListIterGet(piter)) != NULL)
    {
	ajStrDel(&(gl->name));
	ajStrDel(&(gl->doc));
	ajStrDel(&(gl->package));
	ajStrDel(&(gl->keywords));
	embGrpGroupsListDel(&(gl->groups));
	AJFREE(gl);
    }

    ajListIterDel(&piter);
    ajListFree(progslist);

    return;
}




/* @func embGrpKeySearchProgs *************************************************
**
** Searches a list of groups and programs for (partial) matches to a keyword
**
** @param [w] newlist [AjPList] List of matching EmbPGroupProg struct returned
** @param [r] glist [const AjPList] List of EmbPGroupProg struct to
**                                  search through
** @param [r] key [const AjPStr] String to search for
** @param [r] all [AjBool] Match all words in key search string
** @return [void]
** @@
******************************************************************************/

void embGrpKeySearchProgs(AjPList newlist,
			  const AjPList glist, const AjPStr key, AjBool all)
{
    AjIList giter;		/* 'glist' iterator */
    AjIList piter;		/* 'plist' iterator */
    EmbPGroupTop gl;			/* next member of glist */
    EmbPGroupTop gpnode;		/* new member of glist being added */
    EmbPGroupProg pl;			/* next member of plist */
    EmbPGroupProg ppnode;		/* new member of plist being added */
    AjPStr gname = NULL;
    AjPStr name  = NULL;
    AjPStr doc   = NULL;
    AjPStr keywords= NULL;
    AjPStr keystr = NULL;

    /*
    ** compare case independently - so use upper case of both key
    ** and name/doc
    */
    keystr = ajStrNewS(key);
    ajStrFmtUpper(&keystr);

    /* make new group */
    ajStrAssignC(&gname, "Search for '");
    ajStrAppendS(&gname, keystr);
    ajStrAppendC(&gname, "'");
    gpnode = embGrpMakeNewGnode(gname);
    ajListPushAppend(newlist, gpnode);

    giter = ajListIterNewread(glist); /* iterate through existing groups list */

    while((gl = ajListIterGet(giter)) != NULL)
    {
	piter = ajListIterNewread(gl->progs);
	while((pl = ajListIterGet(piter)) != NULL)
	{
	    ajStrAssignS(&name, pl->name);
	    ajStrAssignS(&doc, pl->doc);
	    ajStrAssignS(&keywords, pl->keywords);
	    ajStrFmtUpper(&name);
	    ajStrFmtUpper(&doc);
	    ajStrFmtUpper(&keywords);

	    if (all)
	    {
		if(ajStrMatchWordAllS(doc,keystr) ||
		   ajStrMatchWordAllS(keywords, keystr) ||
		   ajStrMatchWordAllS(name, keystr))
		{
		    ajDebug("Search '%S' in name:'%S' doc:'%S' key:'%S'\n",
			    keystr, pl->name, pl->doc, pl->keywords);
		    ppnode = embGrpMakeNewPnode(pl->name, pl->doc, 
						pl->keywords, pl->package);
		    ajListPushAppend(gpnode->progs, ppnode);
		}
	    }
	    else
	    {
		if(ajStrMatchWordOneS(doc,keystr) ||
		   ajStrMatchWordOneS(keywords, keystr) ||
		   ajStrMatchWordOneS(name, keystr))
		{
		    ajDebug("Search '%S' in name:'%S' doc:'%S' key:'%S'\n",
			    keystr, pl->name, pl->doc, pl->keywords);
		    ppnode = embGrpMakeNewPnode(pl->name, pl->doc, 
						pl->keywords, pl->package);
		    ajListPushAppend(gpnode->progs, ppnode);
		}
	    }

	    ajStrDel(&name);
	    ajStrDel(&doc);
	}
	ajListIterDel(&piter);
    }
    ajListIterDel(&giter);

    /* sort the results */
    embGrpSortGroupsList(newlist);

    ajStrDel(&gname);
    ajStrDel(&name);
    ajStrDel(&doc);
    ajStrDel(&keystr);
    ajStrDel(&keywords);

    return;
}




/* @func embGrpKeySearchSeeAlso ***********************************************
**
** Takes an application name and returns a list of the groups that the
** application belongs to and a list of the applications that are in
** those groups.
**
** If the program we are searching for is not found, it returns *appgroups
** as NULL.
**
** @param [u] newlist [AjPList] List of application groups EmbPGroupTop
**                              returned
** @param [w] appgroups [AjPList *] List of EmbPGroupTop groups of programs
**                                  returned
** @param [w] package [AjPStr *] List of EmbPGroupTop groups of programs
** @param [r] alpha [const AjPList] List of EmbPGroupProg struct to
**                                  search through
** @param [r] glist [const AjPList] List of EmbPGroupTop struct to
**                                  search through
** @param [r] key [const AjPStr] program name to search for
** @return [void]
** @@
******************************************************************************/

void embGrpKeySearchSeeAlso(AjPList newlist, AjPList *appgroups,
			    AjPStr* package,
			    const AjPList alpha, const AjPList glist,
			    const AjPStr key)
{

    AjIList giter;	 /* 'glist' iterator */
    AjIList piter;	 /* 'plist' iterator */
    AjIList griter;      /* iterator through list of groups we have found */
    EmbPGroupTop gl;	 /* next member of glist */
    EmbPGroupTop gpnode; /* new member of newlist being added */
    EmbPGroupProg ppnode; /* new member of glist being added */
    EmbPGroupProg pl;	 /* next member of plist */
    EmbPGroupTop gr;     /* next member of list of groups we have found */
    AjPStr tmp = NULL;
    AjPList base;

    /* make initial group node and push on newlist */
    tmp = ajStrNewC("See also");
    gpnode = embGrpMakeNewGnode(tmp);
    base = gpnode->progs;
    ajListPushAppend(newlist, gpnode);


    /*
    **  set *appgroups to NULL initially - test to see if still NULL after
    **  we have searched for the application name
    **/
    *appgroups = NULL;

    /*
    **  initially look for our application in list 'alpha' to get its list of
    **  groups
    **/

    /* iterate through existing applications list */
    giter = ajListIterNewread(alpha);
    while((gl = ajListIterGet(giter)) != NULL)
    {
	piter = ajListIterNewread(gl->progs);
	while((pl = ajListIterGet(piter)) != NULL)
	    if(ajStrMatchCaseS(pl->name, key))
	    {
		*appgroups = pl->groups;
		ajStrAssignS(package, pl->package);
	    }
	ajListIterDel(&piter);
    }
    ajListIterDel(&giter);

    /* If application not found */
    if(*appgroups == NULL)
	return;

    /*
    ** go through each group in glist finding those that are
    ** used by the application
    */

    /* iterate through existing applications list */
    giter = ajListIterNewread(glist);
    while((gl = ajListIterGet(giter)) != NULL)
    {
	griter = ajListIterNewread(*appgroups); /* iterate through groups found */
	while((gr = ajListIterGet(griter)) != NULL)
	{
	    if(!ajStrCmpCaseS(gr->name, gl->name))
	    {
		/*
		**  found one of the groups - pull out
		**  the applications
		**/
		piter = ajListIterNewread(gl->progs);
		while((pl = ajListIterGet(piter)) != NULL)
		{
		    /* don't want to include our key program */
		    if(!ajStrCmpS(pl->name, key))
			continue;

		    /* make new application node and push on base */
		    ppnode = embGrpMakeNewPnode(pl->name, pl->doc,
						pl->keywords, pl->package);
		    ajListPushAppend(base, ppnode);

		}
		ajListIterDel(&piter);

	    }
	}
	ajListIterDel(&griter);
    }
    ajListIterDel(&giter);

    /* sort the results and remove duplicates */
    embGrpSortProgsList(base);
    embGrpProgsMakeUnique(base);

    ajStrDel(&tmp);

    return;
}




/* @func embGrpProgsMakeUnique ************************************************
**
** Takes a sorted EmbPGroupProg list and ensures that there are no duplicate
** group or application names in that list.
**
** @param [u] list [AjPList] List of application GPnode returned
** @return [void]
** @@
******************************************************************************/

void embGrpProgsMakeUnique(AjPList list)
{
    AjIList iter;
    EmbPGroupProg l;			/* next member of list */
    AjPStr old = NULL;			/* previous name */

    old = ajStrNewC("");

    iter = ajListIterNew(list);
    while((l = ajListIterGet(iter)) != NULL)
    {

	if(!ajStrCmpCaseS(l->name, old))
	{

	    /* delete this GPnode's lists and data */
	    embGrpGroupsListDel(&l->groups);
	    ajStrDel(&(l->name));
	    ajStrDel(&(l->doc));
	    ajStrDel(&(l->package));
	    AJFREE(l);

	    /* delete this element of the list */
	    ajListIterRemove(iter);

	}
	else
	{
	    ajStrDel(&old);
	    old = ajStrNewRef(l->name);
	    embGrpGroupMakeUnique(l->groups);
	}

    }
    ajListIterDel(&iter);

    ajStrDel(&old);

    return;
}


/* @func embGrpGroupMakeUnique ************************************************
**
** Takes a sorted EmbPGroupTop list and ensures that there are no duplicate
** group or application names in that list.
**
** @param [u] list [AjPList] List of application GPnode returned
** @return [void]
** @@
******************************************************************************/

void embGrpGroupMakeUnique(AjPList list)
{
    AjIList iter;
    EmbPGroupTop l;			/* next member of list */
    AjPStr old = NULL;			/* previous name */

    old = ajStrNewC("");

    iter = ajListIterNew(list);
    while((l = ajListIterGet(iter)) != NULL)
    {

	if(!ajStrCmpCaseS(l->name, old))
	{

	    /* delete this GPnode's lists and data */
	    embGrpProgsListDel(&l->progs);
	    ajStrDel(&(l->doc));
	    ajStrDel(&(l->name));
	    AJFREE(l);

	    /* delete this element of the list */
	    ajListIterRemove(iter);

	}
	else
	{
	    ajStrDel(&old);
	    old = ajStrNewRef(l->name);
	    embGrpProgsMakeUnique(l->progs);
	}

    }
    ajListIterDel(&iter);

    ajStrDel(&old);

    return;
}


/* @func embGrpExit ***********************************************************
**
** Cleanup program group internals on exit
**
** @return [void]
******************************************************************************/

void embGrpExit(void)
{
    ajStrDel(&grpStr1);
    ajStrDel(&grpStr2);

    return;
}

/******************************************************************************
** @source AJAX PATTERN (ajax pattern and patternlist) functions
**
** These functions allow handling of patternlists.
**
** @author Copyright (C) 2004 Henrikki Almusa, Medicel Oy
** @version 0.9
** @modified Aug 10 Beta version
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

/* @datastatic PatPRegTypes ***************************************************
**
** Regular expression pattern types
**
** @alias PatSTypes
** @alias PatOTypes
**
** @attr Name [const char*] Type name
** @attr Desc [const char*] Type description
** @@
******************************************************************************/

typedef struct PatSRegTypes
{
    const char *Name;
    const char *Desc;
} PatORegTypes;

#define PatPRegTypes PatORegTypes*

static PatORegTypes patRegTypes[] = {
/* "Name",        "Description" */
  {"string",      "General string pattern"},
  {"protein",     "Protein sequence pattern"},
  {"nucleotide",  "Nucleotide sequence pattern"},
  {NULL, NULL}
};

/* @datastatic PatPRegInformat ************************************************
**
** Regular expression pattern file formats
**
** @alias PatSRegInformat
** @alias PatORegInformat
**
** @attr Name [const char*] Format name
** @attr Desc [const char*] Format description
** @@
******************************************************************************/

typedef struct PatSRegInformat
{
    const char *Name;
    const char *Desc;
} PatORegInformat;

#define PatPRegInformat PatORegInformat*

static PatORegInformat patRegInformat[] = {
    /* "Name",        "Description" */
    {"unknown",    "Unknown"}, 
    {"simple",     "Single sequence per line"},
    {"fasta",      "Fasta sequence style with header"},
    {NULL, NULL}
};

/* @datastatic PatPSeqInformat ************************************************
**
** Sequence pattern file formats
**
** @alias PatSSeqInformat
** @alias PatOSeqInformat
**
** @attr Name [const char*] Format name
** @attr Desc [const char*] Format description
** @@
******************************************************************************/

typedef struct PatSSeqInformat
{
    const char *Name;
    const char *Desc;
} PatOSeqInformat;

#define PatPSeqInformat PatOSeqInformat*

static PatOSeqInformat patSeqInformat[] = {
/* "Name",        "Description" */
  {"fasta",      "Fasta sequence style with header"},
  {"simple",     "Single sequence per line"},
  {NULL, NULL}
};

static ajuint patternRegexFormat(const AjPStr fmt);
static ajuint patternSeqFormat(const AjPStr fmt);

/* @func ajPatternSeqNewList **************************************************
**
** Constructor for a sequence pattern object. Sets all but compiled object.
** That is set with search function. Adds the pattern to a pattern list.
**
** @param [u] plist [AjPPatlistSeq] Pattern list
** @param [r] name [const AjPStr] Name of the pattern
** @param [r] pat [const AjPStr] Pattern as string
** @param [r] mismatch [ajuint] mismatch value
** @return [AjPPatternSeq] New pattern object
** @@
******************************************************************************/
AjPPatternSeq ajPatternSeqNewList (AjPPatlistSeq plist,
				   const AjPStr name, const AjPStr pat,
				   ajuint mismatch)
{
    AjPPatternSeq pthis;

    if (!ajStrGetLen(pat))
	return NULL;

    AJNEW0(pthis);

    if(ajStrGetLen(name))
	ajStrAssignS (&pthis->Name,name);
    else
	ajFmtPrintS(&pthis->Name, "pattern%d",
		    1+ajListGetLength(plist->Patlist));

    ajStrAssignS(&pthis->Pattern,pat);
    pthis->Protein  = plist->Protein;
    pthis->Mismatch = mismatch;

    ajPatlistAddSeq (plist,pthis);

    return pthis;
}

/* @func ajPatternRegexNewList ************************************************
**
** Constructor for a pattern object. Sets all but compiled object. That is set
** with search function. Adds the pattern to a pattern list.
**
** @param [u] plist [AjPPatlistRegex] Regular expression pattern list
** @param [r] name [const AjPStr] Name of the pattern
** @param [r] pat [const AjPStr] Pattern as string
** @return [AjPPatternRegex] New regular expression pattern object
** @@
******************************************************************************/
AjPPatternRegex ajPatternRegexNewList(AjPPatlistRegex plist,
				      const AjPStr name,
				      const AjPStr pat)
{
    AjPPatternRegex pthis;

    if (!ajStrGetLen(pat))
	return NULL;

    AJNEW0(pthis);

    if(ajStrGetLen(name))
	ajStrAssignS (&pthis->Name,name);
    else
	ajFmtPrintS(&pthis->Name, "regex%d",
		    1+ajListGetLength(plist->Patlist));

    ajStrAssignS  (&pthis->Pattern,pat);
    pthis->Type = plist->Type;

    if(plist->Type == AJ_PAT_TYPE_PRO)
    {
    }
    else if(plist->Type == AJ_PAT_TYPE_NUCL)
    {
    }

    pthis->Compiled = ajRegComp(pthis->Pattern);
    ajPatlistAddRegex (plist,pthis);

    return pthis;
}

/* @func ajPatternRegexDel ****************************************************
**
** Destructor for a regular expression pattern object
**
** @param [d] pthys [AjPPatternRegex*] Pattern object reference
** @return [void]
** @@
******************************************************************************/
void ajPatternRegexDel (AjPPatternRegex* pthys)
{
    AjPPatternRegex thys = *pthys;
    ajStrDel(&thys->Name);
    ajStrDel(&thys->Pattern);

    ajRegFree(&thys->Compiled);

    AJFREE (*pthys);

    return;
}

/* @func ajPatternSeqDel ******************************************************
**
** Destructor for a pattern object
**
** @param [d] pthys [AjPPatternSeq*] Pattern object reference
** @return [void]
** @@
******************************************************************************/
void ajPatternSeqDel (AjPPatternSeq* pthys)
{
    AjPPatternSeq thys = *pthys;
    ajStrDel(&thys->Name);
    ajStrDel(&thys->Pattern);

    ajPatCompDel((AjPPatComp*) &thys->Compiled);
    AJFREE (*pthys);

    return;
}

/* @func ajPatternSeqGetName **************************************************
**
** Returns the name of the pattern.
**
** @param [r] thys [const AjPPatternSeq] Pattern
** @return [const AjPStr] Name of the pattern. Real pointer in structure.
** @@
******************************************************************************/
const AjPStr ajPatternSeqGetName (const AjPPatternSeq thys)
{
    return thys->Name;
}

/* @func ajPatternRegexGetName ************************************************
**
** Returns the name of the pattern.
**
** @param [r] thys [const AjPPatternRegex] Pattern
** @return [const AjPStr] Name of the pattern. Real pointer in structure.
** @@
******************************************************************************/
const AjPStr ajPatternRegexGetName (const AjPPatternRegex thys)
{
    return thys->Name;
}

/* @func ajPatternSeqGetPattern ***********************************************
**
** Returns pattern in string format.
**
** @param [r] thys [const AjPPatternSeq] Pattern
** @return [const AjPStr] Pattern. Real pointer in structure.
** @@
******************************************************************************/
const AjPStr ajPatternSeqGetPattern (const AjPPatternSeq thys)
{
    return thys->Pattern;
}

/* @func ajPatternRegexGetPattern *********************************************
**
** Returns pattern in string format.
**
** @param [r] thys [const AjPPatternRegex] Pattern
** @return [const AjPStr] Pattern. Real pointer in structure.
** @@
******************************************************************************/
const AjPStr ajPatternRegexGetPattern (const AjPPatternRegex thys)
{
    return thys->Pattern;
}

/* @func ajPatternSeqGetCompiled **********************************************
**
** Returns void pointer to compiled pattern.
**
** @param [r] thys [const AjPPatternSeq] Pattern
** @return [AjPPatComp] Reference for compiled pattern
** @@
******************************************************************************/
AjPPatComp ajPatternSeqGetCompiled (const AjPPatternSeq thys)
{
    return thys->Compiled;
}

/* @func ajPatternRegexGetCompiled ********************************************
**
** Returns void pointer to compiled pattern. Compiles expression if not
** yet done.
**
** @param [r] thys [const AjPPatternRegex] Pattern
** @return [AjPRegexp] Reference for compiled pattern
** @@
******************************************************************************/
AjPRegexp ajPatternRegexGetCompiled (const AjPPatternRegex thys)
{
    return thys->Compiled;
}

/* @func ajPatternSeqGetProtein ***********************************************
**
** Returns true if the pattern is for a protein sequence.
**
** @param [r] thys [const AjPPatternSeq] Pattern
** @return [AjBool] ajTrue for a protein pattern
** @@
******************************************************************************/
AjBool ajPatternSeqGetProtein (const AjPPatternSeq thys)
{
    return thys->Protein;
}

/* @func ajPatternRegexGetType ************************************************
**
** Returns the type of the pattern.
**
** @param [r] thys [const AjPPatternRegex] Pattern
** @return [ajuint] Type of the pattern.
** @@
******************************************************************************/
ajuint ajPatternRegexGetType (const AjPPatternRegex thys)
{
    return thys->Type;
}

/* @func ajPatternSeqGetMismatch **********************************************
**
** Returns the mismatch of the pattern.
**
** @param [r] thys [const AjPPatternSeq] Pattern
** @return [ajuint] Mismatch value of the pattern.
** @@
******************************************************************************/
ajuint ajPatternSeqGetMismatch (const AjPPatternSeq thys)
{
    return thys->Mismatch;
}


/* @func ajPatternSeqSetCompiled **********************************************
**
** Sets the compiled pattern
**
** @param [u] thys [AjPPatternSeq] Pattern
** @param [u] pat [void *] Compiled pattern
** @return [void]
** @@
******************************************************************************/
void ajPatternSeqSetCompiled (AjPPatternSeq thys, void* pat)
{
    thys->Compiled = pat;
    return;
}

/* @func ajPatternRegexSetCompiled ********************************************
**
** Sets the compiled pattern
**
** @param [u] thys [AjPPatternRegex] Pattern
** @param [u] pat [AjPRegexp] Compiled pattern
** @return [void]
** @@
******************************************************************************/
void ajPatternRegexSetCompiled (AjPPatternRegex thys, AjPRegexp pat)
{
    thys->Compiled = pat;
    return;
}

/* @func ajPatternSeqDebug ****************************************************
**
** Constructor for a pattern list object
**
** @param [r] pat [const AjPPatternSeq] Pattern object
** @return [void]
** @@
******************************************************************************/
void ajPatternSeqDebug (const AjPPatternSeq pat)
{
    ajDebug ("patPatternSeqDebug:\n  name: %S\n  pattern: %S\n  protein: %B,"
             " mismatch: %d\n",
	     pat->Name, pat->Pattern, pat->Protein, pat->Mismatch);
    return;
}



/* @func ajPatternRegexDebug **************************************************
**
** Constructor for a pattern list object
**
** @param [r] pat [const AjPPatternRegex] Pattern object
** @return [void]
** @@
******************************************************************************/
void ajPatternRegexDebug (const AjPPatternRegex pat)
{
    ajDebug ("patPatternRegexDebug:\n  name: %S\n  pattern: %S\n  type: %d",
	     pat->Name, pat->Pattern, pat->Type);
    return;
}




/* @func ajPatlistRegexNew ****************************************************
**
** Constructor for a pattern list object
**
** @return [AjPPatlistRegex] New pattern list object
** @@
******************************************************************************/
AjPPatlistRegex ajPatlistRegexNew (void)
{
    AjPPatlistRegex pthis;

    AJNEW0(pthis);

    pthis->Patlist = ajListNew();
    pthis->Iter    = NULL;

    // ajDebug ("ajPatlistRegexNew size '%d'\n",ajListGetLength(pthis->Patlist));

    return pthis;
}

/* @func ajPatlistRegexNewType ************************************************
**
** Constructor for a pattern list object with a specified type
**
** @param [r] type [ajuint] type value
** @return [AjPPatlistRegex] New pattern list object
** @@
******************************************************************************/
AjPPatlistRegex ajPatlistRegexNewType (ajuint type)
{
    AjPPatlistRegex pthis;

    AJNEW0(pthis);

    pthis->Patlist = ajListNew();
    pthis->Iter    = NULL;
    pthis->Type = type;

    // ajDebug ("ajPatlistRegexNew size '%d'\n",ajListGetLength(pthis->Patlist));

    return pthis;
}

/* @func ajPatlistSeqNew ******************************************************
**
** Constructor for a pattern list object. Defaults to protein.
**
** @return [AjPPatlistSeq] New pattern list object
** @@
******************************************************************************/
AjPPatlistSeq ajPatlistSeqNew (void)
{
    AjPPatlistSeq pthis;

    AJNEW0(pthis);

    pthis->Patlist = ajListNew();
    pthis->Iter    = NULL;
    pthis->Protein = ajTrue;

    ajDebug ("ajPatlistSeqNew\n");

    return pthis;
}

/* @func ajPatlistSeqNewType **************************************************
**
** Constructor for a pattern list object
**
** @param [r] type [AjBool] True for a protein pattern
** @return [AjPPatlistSeq] New pattern list object
** @@
******************************************************************************/
AjPPatlistSeq ajPatlistSeqNewType(AjBool type)
{
    AjPPatlistSeq pthis;

    AJNEW0(pthis);

    pthis->Patlist = ajListNew();
    pthis->Iter    = NULL;
    pthis->Protein = type;

    ajDebug ("ajPatlistSeqNewType type '%d'\n", pthis->Protein);

    return pthis;
}

/* @func ajPatlistRegexDel ****************************************************
**
** Destructor for a pattern list object
**
** @param [d] pthys [AjPPatlistRegex*] Pattern list object reference
** @return [void]
** @@
******************************************************************************/
void ajPatlistRegexDel (AjPPatlistRegex* pthys)
{
    AjPPatlistRegex thys = NULL;
    AjPPatternRegex patternregex = NULL;

    thys = *pthys;

    while (ajListPop(thys->Patlist, (void **)&patternregex))
    {
	ajDebug("ajPatlistRegexDel list size: %d\n",
		ajListGetLength(thys->Patlist));
	ajPatternRegexDel(&patternregex);
    }
    if (thys->Iter)
	ajListIterDel(&thys->Iter);
    ajListFree(&thys->Patlist);

    AJFREE(*pthys);

    return;
}

/* @func ajPatlistSeqDel ******************************************************
**
** Destructor for a pattern list object
**
** @param [d] pthys [AjPPatlistSeq*] Pattern list object reference
** @return [void]
** @@
******************************************************************************/
void ajPatlistSeqDel (AjPPatlistSeq* pthys)
{
    AjPPatlistSeq thys = NULL;
    AjPPatternSeq patternseq = NULL;

    thys = *pthys;

    while (ajListPop(thys->Patlist, (void **)&patternseq))
	ajPatternSeqDel(&patternseq);
    if (thys->Iter)
	ajListIterDel(&thys->Iter);
    ajListFree(&thys->Patlist);

    AJFREE(*pthys);

    return;
}

/* @func ajPatlistSeqRead *****************************************************
**
** Parses a file into pattern list object. If there is not mismatch value on
** pattern in file, it is assumed to be 0.
**
** @param [r] patspec [const AjPStr] Pattern specification
** @param [r] patname [const AjPStr] Default pattern name prefix
** @param [r] fmt [const AjPStr] Pattern file format
** @param [r] protein [AjBool] ajTrue for protein patterns
** @param [r] mismatches [ajuint] default number of mismatches
** @return [AjPPatlistSeq] Pattern list
** @@
******************************************************************************/
AjPPatlistSeq ajPatlistSeqRead (const AjPStr patspec,
				const AjPStr patname,
				const AjPStr fmt,
				AjBool protein, ajuint mismatches)
{
    AjPPatlistSeq patlist = NULL;
    AjPStr line = NULL;
    AjPStr name = NULL;
    AjPFileBuff infile = NULL;
    AjPRegexp mismreg = NULL;
    AjPStr patstr = NULL;
    AjPStr pat = NULL;
    ajuint mismatch = 0;
    ajint ifmt = 0;
    ajuint npat = 0;
    AjPStr namestr = NULL;

    ajStrAssignS(&namestr, patname);
    ajStrAssignEmptyC(&namestr, "pattern");

    ajStrAssignS(&patstr, patspec);

    patlist = ajPatlistSeqNewType(protein);

    ifmt = patternSeqFormat(fmt);

    ajDebug("ajPatlistSeqRead patspec: '%S' patname: '%S' "
	    "protein: %B mismatches: %d\n",
	    patspec, patname, protein, mismatches);
    if(ajStrGetCharFirst(patstr) == '@')
    {
	ajStrCutStart(&patstr, 1);
	infile = ajFileBuffNewIn(patstr);
	if(!infile)
	{
	    ajErr("Unable to open pattern file '%S'", patstr);
	    return NULL;
	}
	line = ajStrNew();
	name = ajStrNew();

	if(!ifmt)
	{
	    ajFileBuffGetTrim(infile,&line);
	    if(ajStrPrefixC(line, ">"))
		ifmt = 2;
	    else
		ifmt = 1;
	    ajFileBuffReset(infile);
	}
	
	switch(ifmt)
	{
	case 1:
	    while (ajFileBuffGetTrim(infile,&line))
	    {
		npat++;
		ajStrAppendS (&pat,line);
		ajFmtPrintS(&name, "%S%u", namestr, npat);
		ajPatternSeqNewList(patlist,name,pat,mismatches);
		ajStrSetClear(&pat);
	    }
	    break;
	default:
	    mismreg = ajRegCompC("<mismatch=(\\d+)>");
	    while (ajFileBuffGetTrim(infile,&line))
	    {
		if (ajStrGetCharFirst(line) == '>')
		{
		    if (ajStrGetLen(name))
		    {
			ajPatternSeqNewList(patlist,name,pat,
					    mismatch);
			ajStrSetClear(&name);
			ajStrSetClear(&pat);
			mismatch=mismatches;
		    }
		    ajStrCutStart(&line,1);
		    if (ajRegExec(mismreg,line))
		    {
			ajRegSubI(mismreg,1,&name);
			ajStrToUint(name,&mismatch);
			ajStrTruncateLen(&line,ajRegOffset(mismreg));
			ajStrTrimWhiteEnd(&line);
		    }
		    ajStrAssignS (&name,line);
		    ajStrAssignEmptyS(&name, patname);
		}
		else
		    ajStrAppendS (&pat,line);
	    }
	    ajStrAssignEmptyS(&name, patname);
	    ajPatternSeqNewList(patlist,name,pat,mismatch);
	    ajRegFree(&mismreg);
	    break;
	}
	ajFileBuffDel(&infile);
    }
    else
    {
	npat++;
	ajFmtPrintS(&name, "%S%u", namestr, npat);
	ajPatternSeqNewList(patlist,name,patstr,mismatches);
    }

    ajStrDel(&name);
    ajStrDel(&line);
    ajStrDel(&pat);
    ajStrDel(&namestr);
    ajStrDel(&patstr);

    return patlist;
}

/* @func ajPatlistRegexRead ***************************************************
**
** Parses a file of regular expressions into a pattern list object.
**
** @param [r] patspec [const AjPStr] Name of the file with patterns
** @param [r] patname [const AjPStr] Default pattern name prefix
** @param [r] fmt [const AjPStr] Pattern file format
** @param [r] type [ajuint] Type of the patterns
** @param [r] upper [AjBool] Convert to upper case
** @param [r] lower [AjBool] Convert to lower case
** @return [AjPPatlistRegex] Pattern list
** @@
******************************************************************************/
AjPPatlistRegex ajPatlistRegexRead (const AjPStr patspec,
				    const AjPStr patname,
				    const AjPStr fmt,
				    ajuint type, AjBool upper, AjBool lower)
{
    AjPPatlistRegex patlist = NULL;
    AjPStr line = NULL;
    AjPStr pat  = NULL;
    AjPStr name = NULL;
    AjPFileBuff infile = NULL;
    AjPStr patstr = NULL;
    ajuint ifmt;
    ajuint npat = 0;
    AjPStr namestr = NULL;

    ajStrAssignS(&namestr, patname);
    ajStrAssignEmptyC(&namestr, "regex");

    ajStrAssignS(&patstr, patspec);

    patlist = ajPatlistRegexNewType(type);

    ifmt = patternRegexFormat(fmt);

    if(ajStrGetCharFirst(patspec) ==  '@')
    {
	ajStrCutStart(&patstr, 1);
	infile = ajFileBuffNewIn(patstr);
	if(!infile) {
	    ajErr("Unable to open regular expression file '%S'", patstr);
	    return NULL;
	}
	line = ajStrNew();
	pat  = ajStrNew();
	name = ajStrNew();

	if(!ifmt)
	{
	    ajFileBuffGetTrim(infile,&line);
	    if(ajStrPrefixC(line, ">"))
		ifmt = 2;
	    else
		ifmt = 1;
	    ajFileBuffReset(infile);
	}
	
	switch(ifmt)
	{
	case 1:
	    while (ajFileBuffGetTrim(infile,&line))
	    {
		npat++;
		ajStrAppendS (&pat,line);
		if(lower)
		    ajStrFmtLower(&pat);
		if(upper)
		    ajStrFmtUpper(&pat);
		ajFmtPrintS(&name, "%S%u", namestr, npat);
		ajPatternRegexNewList(patlist,name,pat);
		ajStrSetClear(&pat);
	    }
	    break;
	default:
	    while (ajFileBuffGetTrim(infile,&line))
	    {
		if (ajStrFindC(line,">")>-1)
		{
		    npat++;
		    if (ajStrGetLen(name))
		    {
			if(lower)
			    ajStrFmtLower(&pat);
			if(upper)
			    ajStrFmtUpper(&pat);
			ajPatternRegexNewList(patlist,name,pat);
			ajStrSetClear(&name);
			ajStrSetClear(&pat);
		    }
		    ajStrCutStart(&line,1);
		    ajStrAssignS (&name,line);
		    if(!ajStrGetLen(name))
			ajFmtPrintS(&name, "%S%u", namestr, npat);
		}
		else
		    ajStrAppendS (&pat,line);
	    }
	    ajStrAssignEmptyS(&name, patname);
	    ajPatternRegexNewList(patlist,name,pat);
	    ajStrSetClear(&pat);
	    break;
	}
	ajFileBuffDel(&infile);
    }
    else
    {
	npat++;
	ajStrAssignS(&pat, patspec);
	if(lower)
	    ajStrFmtLower(&pat);
	if(upper)
	    ajStrFmtUpper(&pat);
	ajFmtPrintS(&name, "%S%u", namestr, npat);
	ajPatternRegexNewList(patlist,name,pat);
    }

    ajStrDel(&name);
    ajStrDel(&namestr);
    ajStrDel(&patstr);
    ajStrDel(&line);
    ajStrDel(&pat);

    return patlist;
}

/* @func ajPatlistSeqGetSize **************************************************
**
** Gets number of patterns from list.
**
** @param [r] thys [const AjPPatlistSeq] Pattern list object
** @return [ajuint] Number of patterns
** @@
******************************************************************************/
ajuint ajPatlistSeqGetSize (const AjPPatlistSeq thys)
{
    return ajListGetLength(thys->Patlist);
}

/* @func ajPatlistRegexGetSize ************************************************
**
** Gets number of patterns from list.
**
** @param [r] thys [const AjPPatlistRegex] Pattern list object
** @return [ajuint] Number of patterns
** @@
******************************************************************************/
ajuint ajPatlistRegexGetSize (const AjPPatlistRegex thys)
{
    return ajListGetLength(thys->Patlist);
}

/* @func ajPatlistSeqGetNext **************************************************
**
** Gets next available pattern from list.
**
** @param [u] thys [AjPPatlistSeq] Pattern list object
** @param [w] pattern [AjPPatternSeq*] Pattern object reference
** @return [AjBool] ajTrue if there was next object
** @@
******************************************************************************/
AjBool ajPatlistSeqGetNext (AjPPatlistSeq thys, AjPPatternSeq* pattern)
{
    if (!thys->Iter)
	thys->Iter = ajListIterNew(thys->Patlist);

    if (!ajListIterDone(thys->Iter))
	*pattern = ajListIterGet(thys->Iter);
    else
    {
	ajPatlistSeqRewind(thys);
	return ajFalse;
    }

    return ajTrue;
}

/* @func ajPatlistRegexGetNext ************************************************
**
** Gets next available pattern from list.
**
** @param [u] thys [AjPPatlistRegex] Pattern list object
** @param [w] pattern [AjPPatternRegex*] Pattern object reference
** @return [AjBool] ajTrue if there was next object
** @@
******************************************************************************/
AjBool ajPatlistRegexGetNext (AjPPatlistRegex thys,
			      AjPPatternRegex* pattern)
{
    if (!thys->Iter)
	thys->Iter = ajListIterNew(thys->Patlist);

    if (!ajListIterDone(thys->Iter))
	*pattern = ajListIterGet(thys->Iter);
    else
    {
	ajPatlistRegexRewind(thys);
	return ajFalse;
    }

    return ajTrue;
}



/* @func ajPatlistRegexRewind *************************************************
**
** Resets the pattern list iteration.
**
** @param [u] thys [AjPPatlistRegex] Pattern list object reference
** @return [void]
** @@
******************************************************************************/
void ajPatlistRegexRewind (AjPPatlistRegex thys)
{
    if (thys->Iter)
	ajListIterDel(&thys->Iter);
    thys->Iter=NULL;

    return;
}

/* @func ajPatlistSeqRewind ***************************************************
**
** Resets the pattern list iteration.
**
** @param [u] thys [AjPPatlistSeq] Pattern list object reference
** @return [void]
** @@
******************************************************************************/
void ajPatlistSeqRewind (AjPPatlistSeq thys)
{
    if (thys->Iter)
	ajListIterDel(&thys->Iter);
    thys->Iter=NULL;

    return;
}

/* @func ajPatlistRegexRemoveCurrent ******************************************
**
** Removes current pattern from pattern list. If looping has not started or
** pattern list has just been rewound then nothing is removed.
**
** @param [u] thys [AjPPatlistRegex] Pattern list from which to remove
** @return [void]
** @@
******************************************************************************/
void ajPatlistRegexRemoveCurrent (AjPPatlistRegex thys)
{
    if (!thys->Iter)
	return;

    ajListIterRemove(thys->Iter);
    ajListIterGetBack(thys->Iter);

    return;
}


/* @func ajPatlistSeqRemoveCurrent ********************************************
**
** Removes current pattern from pattern list. If looping has not started or
** pattern list has just been rewound then nothing is removed.
**
** @param [u] thys [AjPPatlistSeq] Pattern list from which to remove
** @return [void]
** @@
******************************************************************************/
void ajPatlistSeqRemoveCurrent (AjPPatlistSeq thys)
{
    if (!thys->Iter)
	return;

    ajListIterRemove(thys->Iter);
    ajListIterGetBack(thys->Iter);

    return;
}


/* @func ajPatlistAddSeq ******************************************************
**
** Adds pattern into patternlist
**
** @param [u] thys [AjPPatlistSeq] Pattern list object reference
** @param [u] pat [AjPPatternSeq] Pattern to be added
** @return [void]
** @@
******************************************************************************/
void ajPatlistAddSeq (AjPPatlistSeq thys, AjPPatternSeq pat)
{
    ajDebug ("ajPatlistAddSeq list size %d '%S' '%S' '%B' '%d'\n",
             ajListGetLength (thys->Patlist), pat->Name,
             pat->Pattern, pat->Protein, pat->Mismatch);
    ajListPushAppend(thys->Patlist, pat);

    return;
}


/* @func ajPatlistAddRegex ****************************************************
**
** Adds pattern into patternlist
**
** @param [u] thys [AjPPatlistRegex] Pattern list object reference
** @param [u] pat [AjPPatternRegex] Pattern to be added
** @return [void]
** @@
******************************************************************************/
void ajPatlistAddRegex (AjPPatlistRegex thys, AjPPatternRegex pat)
{
    ajDebug ("ajPatlistAddRegex list size %d '%S' '%S' '%d'\n",
             ajListGetLength (thys->Patlist), pat->Name,
             pat->Pattern, pat->Type);
    ajListPushAppend(thys->Patlist, pat);

    return;
}

/* @func ajPatCompNew *******************************************************
**
** Create prosite pattern structure.
**
** @return [AjPPatComp] pattern structure
** @@
******************************************************************************/
AjPPatComp ajPatCompNew (void)
{
    AjPPatComp pthis;

    AJNEW0(pthis);

    pthis->pattern=ajStrNew();
    pthis->regex=ajStrNew();

    return pthis;
}

/* @func ajPatCompDel *******************************************************
**
** Delete prosite pattern structure.
**
** @param [d] pthys [AjPPatComp*] Prosite pattern structure
**
** @return [void]
** @@
******************************************************************************/
void ajPatCompDel (AjPPatComp *pthys)
{
    ajuint i;

    AjPPatComp thys = *pthys;
    ajStrDel(&thys->pattern);
    ajStrDel(&thys->regex);

    if(thys->buf)
      AJFREE(thys->buf);
    if(thys->sotable)
      AJFREE(thys->sotable);

    if(thys->type==6)
	for(i=0;i<thys->m;++i)
	    AJFREE(thys->skipm[i]);

    AJFREE(*pthys);

    return;
}


/* @funcstatic patternRegexFormat *********************************************
**
** Returns format associated with a named format of regular expression
**
** @param [r] fmt [const AjPStr] Regular expression format
** @return [ajuint] Format number, defaults to 0 (string)
******************************************************************************/

static ajuint patternRegexFormat(const AjPStr fmt)
{
    ajuint i = 0;

    if(!ajStrGetLen(fmt))
       return 0;

    while (patRegInformat[i].Name) {
	if(ajStrMatchCaseC(fmt, patRegInformat[i].Name))
	    return i;
	i++;
    }
    ajErr("Unrecognized regular expression file format '%S'",fmt);

    return 0;
}




/* @funcstatic patternSeqFormat ***********************************************
**
** Returns format associated with a named format of sequence pattern
**
** @param [r] fmt [const AjPStr] Regular expression format
** @return [ajuint] Format number, defaults to 0 
******************************************************************************/

static ajuint patternSeqFormat(const AjPStr fmt)
{
    ajuint i = 0;

    if(!ajStrGetLen(fmt))
       return 0;

    while (patSeqInformat[i].Name) {
	if(ajStrMatchCaseC(fmt, patSeqInformat[i].Name))
	    return i;
	i++;
    }
    ajErr("Unrecognized pattern file format '%S'",fmt);

    return 0;
}




/* @func ajPatternRegexType ***************************************************
**
** Returns type associated with a named type of regular expression
**
** @param [r] type [const AjPStr] Regular expression type
** @return [ajuint] Type number, defaults to 0 (string)
******************************************************************************/

ajuint ajPatternRegexType(const AjPStr type)
{
    ajuint i = 0;

    while (patRegTypes[i].Name) {
	if(ajStrMatchCaseC(type, patRegTypes[i].Name))
	    return i;
	i++;
    }
    return 0;
}




/* @func ajPatlistRegexDoc **************************************************
**
** Documents patterns to a formatted string
**
** @param [u] plist [AjPPatlistRegex] Pattern list object
** @param [w] pdoc [AjPStr*] Formatted string
** @return [ajuint] Number of patterns
** @@
******************************************************************************/
ajuint ajPatlistRegexDoc (AjPPatlistRegex plist, AjPStr* pdoc)
{
    AjPPatternRegex pat = NULL;

    ajFmtPrintS(pdoc, "%-12S %S\n", "Pattern_name", "Pattern");
    while (ajPatlistRegexGetNext(plist, &pat))
    {
	ajFmtPrintAppS(pdoc, "%-12S %S\n",
		       ajPatternRegexGetName(pat),
		       ajPatternRegexGetPattern(pat));
    }
    return ajListGetLength(plist->Patlist);
}




/* @func ajPatlistSeqDoc **************************************************
**
** Documents patterns to a formatted string
**
** @param [u] plist [AjPPatlistSeq] Pattern list object
** @param [w] pdoc [AjPStr*] Formatted string
** @return [ajuint] Number of patterns
** @@
******************************************************************************/
ajuint ajPatlistSeqDoc (AjPPatlistSeq plist, AjPStr* pdoc)
{
    AjPPatternSeq pat = NULL;

    ajFmtPrintS(pdoc, "%-12s %8s %s\n",
		   "Pattern_name", "Mismatch", "Pattern");
    while (ajPatlistSeqGetNext(plist, &pat))
    {
	ajFmtPrintAppS(pdoc, "%-12S %8d %S\n",
		       ajPatternSeqGetName(pat),
		       ajPatternSeqGetMismatch(pat),
		       ajPatternSeqGetPattern(pat));
    }
    return ajListGetLength(plist->Patlist);
}

/******************************************************************************
** @source AJAX string functions
**
** AjPStr objects are reference counted strings
** Any change will need a new string object if the use count
** is greater than 1, so the original ajStr provided so that it can
** be reallocated in any routine where string modification is possible.
**
** In many cases
** the text is always a copy, even of a constant original, so
** that it can be simply freed.
**
** @author Copyright (C) 1998 Peter Rice
** @version 1.0
** @modified Jun 25 pmr First version
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

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#ifndef HAVE_MEMMOVE
#include <sys/types.h>
/* @header memmove ************************************************************
**
******************************************************************************/
static void* memmove(void *dst, const void* src, size_t len)
{
    return (void *)bcopy(src, dst, len);
}
#endif


#include <math.h>
#include "ajax.h"
#include <stdarg.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>
#include <float.h>
#include <string.h>




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */




#define STRSIZE  32
#define LONGSTR  512
#define NULL_USE 1
char charNULL[1] = "";

AjOStr strONULL = { 1,0,charNULL,NULL_USE,0}; /* use set to avoid changes */
AjPStr strPNULL = &strONULL;


static char* strParseCp = NULL;


/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static AjPStr strNew(ajuint size);
static AjPStr strClone(AjPStr* Pstr);
static void   strCloneL(AjPStr* pthis, ajuint size);

#ifdef AJ_SAVESTATS
static ajlong strAlloc     = 0;
static ajlong strFree      = 0;
static ajlong strFreeCount = 0;
static ajlong strCount     = 0;
static ajlong strTotal     = 0;
#endif



/* @filesection ajstr ********************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
** @suffix Len Length
** @suffix C C character string
** @suffix S string object
** @suffix K single character
*/




/* @datasection [char*] C character string ***********************************
**
** Function is for manipulating C-type (char*) strings and
** takes at least one char* argument.
**
** @nam2rule Char
*/




/* @section constructors ******************************************************
**
** Functions for constructing C-type (char*) strings,
** possibly with a starting string. 
**
** @fdata [char*]
** @fnote     Same argrule as "String constructor functions"
**
** @nam3rule  New     Construct a new string.
** @nam4rule  NewRes  Minimum reserved size.
** @nam3rule  Null    Return an internal empty string
**
** @argrule   C       txt [const char*] C character string
** @argrule   S       str [const AjPStr] Text string
** @argrule   NewRes  size [ajuint] Reserved size
** @argrule   Len     len [ajuint] Length of string
**
** @valrule   *  [char*] New C-style string
** @fcategory new
**
******************************************************************************/


/* @func ajCharNewC ***********************************************************
**
** A text string constructor which allocates memory for a string
** and initialises it with the text string provided.
**
** @param [r] txt [const char*] Initial text, possibly shorter than the
**        space allocated.
** @return [char*] A new text string.
** @ure The text provided must fit in the specified length
** @@
******************************************************************************/

char* ajCharNewC(const char* txt)
{
    char* cp;
    ajuint len;

    if(txt)
    {
        len = strlen(txt);
        cp = (char*) AJALLOC0(len+1);
        memmove(cp, txt, len+1);
    }
    else
    {
        cp = (char*) AJALLOC0(1);
    }
    
    return cp;
}




/* @func ajCharNewS ***********************************************************
**
** A text string constructor which allocates memory for a string 
** and initialises it with the string provided.
**
** @param [r] str [const AjPStr] String object as initial value and size
**                          for the text.
** @return [char*] A new text string.
** @ure The text provided must fit in the specified length
** @@
******************************************************************************/

char* ajCharNewS(const AjPStr str)
{
    static char* cp;

    cp = (char*) AJALLOC0(str->Len+1);
    memmove(cp, str->Ptr, str->Len+1);

    return cp;
}



/* @obsolete ajCharNew
** @rename ajCharNewS
*/

__deprecated char  *ajCharNew(const AjPStr str)
{
    return ajCharNewS(str);
}

/* @func ajCharNewRes *********************************************************
**
** A text string constructor which allocates memory for a string of the 
** specified length and intialises the contents as an empty string.
**
** @param [r] size [ajuint] Length of the Cstring, excluding the trailing NULL.
** @return [char*] A new text string with no contents.
** @@
******************************************************************************/

char* ajCharNewRes(ajuint size)
{
    static char* cp;

    cp = (char*) AJALLOC0(size+1);
    cp[0] = '\0';

    return cp;
}

/* @obsolete ajCharNewL
** @rename ajCharNewRes
*/

__deprecated char  *ajCharNewL(size_t size) {
    return ajCharNewRes(size);
}

/* @func ajCharNewResC ********************************************************
**
** A text string constructor which allocates memory for a string of the 
** specified length and initialises it with the text string provided.
**
** @param [r] txt [const char*] String object as initial value and size
**                          for the text.
** @param [r] size [ajuint] Maximum string length, as returned by strlen
** @return [char*] A new text string.
** @ure The text provided must fit in the specified length
** @@
******************************************************************************/

char* ajCharNewResC(const char* txt, ajuint size)
{
    static char* cp;
    ajuint isize;
    ajuint ilen;

    isize = size;
    if(txt)
    {
        ilen = strlen(txt);

        if(ilen >= isize)
            isize = ilen + 1;

        cp = (char*) AJALLOC0(isize);
        memmove(cp, txt, ilen+1);
    }
    else
    {
        cp = (char*) AJALLOC0(isize);
    }
   
    return cp;
}

/* @func ajCharNewResS ********************************************************
**
** A text string constructor which allocates memory for a string of the 
** specified length and initialises it with the string provided.
**
** @param [r] str [const AjPStr] String object as initial value and size
**                          for the text.
** @param [r] size [ajuint] Maximum string length, as returned by strlen
** @return [char*] A new text string.
** @ure The text provided must fit in the specified length
** @@
******************************************************************************/

char* ajCharNewResS(const AjPStr str, ajuint size)
{
    static char* cp;
    ajuint isize;

    isize = size;
    if(str->Len >= isize)
	isize = str->Len + 1;

    cp = (char*) AJALLOC0(isize);
    memmove(cp, str->Ptr, str->Len+1);

    return cp;
}


/* @obsolete ajCharNewLS
** @replace ajStrNewResS (1,2/2,1)
*/

__deprecated char  *ajCharNewLS(size_t size, const AjPStr thys) {
    return ajCharNewResS(thys, size);
}



/* @func ajCharNewResLenC *****************************************************
**
** A text string constructor which allocates memory for a string of the 
** specified length and initialises it with the text string provided.
**
** @param [r] txt [const char*] String object as initial value and size
**                          for the text.
** @param [r] size [ajuint] Maximum string length, as returned by strlen
** @param [r] len [ajuint] Length of txt to save calculation time.
** @return [char*] A new text string.
** @ure The text provided must fit in the specified length
** @@
******************************************************************************/

char* ajCharNewResLenC(const char* txt, ajuint size, ajuint len)
{
    static char* cp;
    ajuint isize;

    isize = size;

    if(len >= isize)
	isize = len + 1;

    cp = (char*) AJALLOC0(isize);
    if(len)
        memmove(cp, txt, len);

    return cp;
}

/* @func ajCharNull ***********************************************************
**
** Returns a pointer to an empty string
**
** @return [char*] Empty string
******************************************************************************/

char* ajCharNull (void)
{
  return charNULL;
}

/* @section destructors *******************************************************
**
** Functions for destruction of C-type (char*) strings.
**
** @fdata      [char*]
** @fnote     None.
** @nam3rule  Del    Destroy (free) an existing string.
** @argrule   Del Ptxt [char**] Pointer to character string to be deleted
** @valrule   * [void]
** @fcategory delete
******************************************************************************/


/* @func ajCharDel ***********************************************************
**
** A text string destructor to free memory for a text string.
**
** @param [d] Ptxt [char**] Text string to be deallocated.
** @return [void]
** @ure The string is freed using free in the C RTL, so it
**      must have been allocated by malloc in the C RTL
** @@
******************************************************************************/

void ajCharDel(char** Ptxt)
{
    AJFREE(*Ptxt);

    return;
}

/* @obsolete ajCharFree
** @rename ajCharDel
*/

__deprecated void  ajCharFree(char** txt)
{
    ajCharDel(txt);
}



/* @section formatting
**
** Functions for formatting C-type (char*) strings).
**
** @fdata      [char*]
** @fnote     Same namrule as "String formatting functions"
**
** @nam3rule  Fmt         Change the format of a string.
** @nam4rule  FmtLower    Convert to lower case.
** @nam4rule  FmtTitle    Convert to title case.
** @nam4rule  FmtUpper    Convert to upper case.
**
** @argrule   * txt [char*] Character string
** @valrule   * [AjBool]
** @fcategory modify
*/

/* @func ajCharFmtLower *******************************************************
**
** Converts a text string to lower case.
**
** @param [u] txt [char*] Text string
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajCharFmtLower(char* txt)
{
    char* cp;

    cp = txt;

    while(*cp)
    {
	/*
	 *  AJB: function ajSysItoC was there as some really fussy compilers
	 *  complained about casting ajint to char. However, for conversion of
	 *  large databases it's too much of an overhead. Think about a macro
	 *  later. In the meantime revert to the standard system call
	 *    *cp = ajSysCastItoc(tolower((ajint) *cp));
	 */
	*cp = (char)tolower((ajint) *cp);
	cp++;
    }

    return ajTrue;
}



/* @obsolete ajCharToLower
** @rename ajCharFmtLower
*/

__deprecated void  ajCharToLower(char* txt)
{
    ajCharFmtLower(txt);
}

/* @func ajCharFmtUpper *******************************************************
**
** Converts a text string to upper case.
**
** @param [u] txt [char*] Text string
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajCharFmtUpper(char* txt)
{
    char* cp;

    cp = txt;

    while(*cp)
    {
	/*
	 *  AJB: function ajSysItoC was there as some really fussy compilers
	 *  complained about casting ajint to char. However, for conversion of
	 *  large databases it's too much of an overhead. Think about a macro
	 *  later. In the meantime revert to the standard system call
	 *    *cp = ajSysCastItoc(toupper((ajint) *cp));
	 */
	*cp = (char) toupper((ajint) *cp);
	cp++;
    }

    return ajTrue;
}



/* @obsolete ajCharToUpper
** @rename ajCharFmtUpper
*/

__deprecated void  ajCharToUpper(char* txt)
{
    ajCharFmtUpper(txt);
}




/* @section comparison ********************************************************
**
** Functions for comparing C-type (char*) strings.
**
** @fdata      [char*]
** @fnote     Same namrule as "String comparison functions".
** @nam3rule  Match         Compare two complete strings.
** @nam3rule  Prefix        Compare start of string to given prefix.
** @nam3rule  Suffix        Compare end of string to given suffix.
** @nam4rule  MatchCase   Case-insensitive comparison.
** @nam4rule  MatchWild   Comparison using wildcard characters.
** @nam5rule  MatchWildCase   Case-insensitive comparison using
**                            wildcard characters.
** @nam5rule  MatchWildNext   Comparison using wildcard characters to
**                            next word in a string
** @nam6rule  MatchWildNextCase   Case-insensitive comparison using
**                           wildcard characters to next word in a string
** @nam5rule  MatchWildWord   Comparison using wildcard characters to
**                          whole words within a string.
** @nam6rule  MatchWildWordCase   Case-insensitive comparison using
**                         wildcard characters to whole words within a string.
** @nam4rule  PrefixCase  Case-insensitive comparison.
** @nam4rule  SuffixCase  Case-insensitive comparison.
**
** @argrule   * txt [const char*] Character string
** @argrule   C txt2 [const char*] Character string to compare
** @argrule   S str [const AjPStr] Character string to compare
** @valrule   * [AjBool] True on success
** @fcategory use
*/


/* @func ajCharMatchC *********************************************************
**
** Simple test for matching two text strings.
**
** @param [r] txt [const char*] String
** @param [r] txt2 [const char*] Text
** @return [AjBool] ajTrue if text completely matches the start of String
** @@
******************************************************************************/

AjBool ajCharMatchC(const char* txt, const char* txt2)
{
    if(!txt || !txt2)
	return ajFalse;

    if(!strcmp(txt, txt2))
	return ajTrue;

    return ajFalse;
}

/* @obsolete ajStrMatchCC
** @rename ajCharMatchC
*/

__deprecated AjBool  ajStrMatchCC(const char* thys, const char* text)
{
    return ajCharMatchC(thys, text);
}



/* @func ajCharMatchCaseC *****************************************************
**
** Simple case-insensitive test for matching two text strings.
**
** @param [r] txt [const char*] String
** @param [r] txt2 [const char*] Text
** @return [AjBool] ajTrue if two strings are exactly the same excluding case
** @@
******************************************************************************/

AjBool ajCharMatchCaseC(const char* txt, const char* txt2)
{
    const char* cp;
    const char* cq;

    cp = txt;
    cq = txt2;

    if(!*cp && !*cq)
	return ajTrue;

    if(!*cp || !*cq)
	return ajFalse;

    while(*cp && *cq)
	if(toupper((ajint) *cp++) != toupper((ajint) *cq++))
	    return ajFalse;

    if(*cp || *cq)
	return ajFalse;

    return ajTrue;
}

/* @obsolete ajStrMatchCaseCC
** @rename ajCharMatchCaseC
*/

__deprecated AjBool  ajStrMatchCaseCC(const char* thys, const char* text)
{
    return ajCharMatchCaseC(thys, text);
}

/* @func ajCharMatchWildC *****************************************************
**
** Simple case-sensitive test for matching two text strings using 
** wildcard characters. 
**
** @param [r] txt [const char*] String
** @param [r] txt2 [const char*] Text
** @return [AjBool] ajTrue if the strings match
** @@
******************************************************************************/

AjBool ajCharMatchWildC(const char* txt, const char* txt2)
{
    ajint i;

    i = ajCharCmpWild(txt, txt2);

/*
    ajDebug("ajCharMatchWildC' '%s' cmp: %d\n",
	    str, text, i);
*/

    if(i)
	return ajFalse;

    return ajTrue;
}

/* @obsolete ajStrMatchWildCC
** @rename ajCharMatchWildC
*/

__deprecated AjBool  ajStrMatchWildCC(const char* str, const char* text)
{
    return ajCharMatchWildC(str, text);
}

/* @func ajCharMatchWildS *****************************************************
**
** Simple case-sensitive test for matching a text string and a string using
** wildcard characters.
**
** @param [r] txt [const char*] String
** @param [r] str [const AjPStr] Wildcard text
** @return [AjBool] ajTrue if the strings match
** @@
******************************************************************************/

AjBool ajCharMatchWildS(const char* txt, const AjPStr str)
{
    return ajCharMatchWildC(txt, str->Ptr);
}

/* @func ajCharMatchWildCaseC *************************************************
**
** Simple case-insensitive test for matching two text strings using 
** wildcard characters. 
**
** @param [r] txt [const char*] String
** @param [r] txt2 [const char*] Text
** @return [AjBool] ajTrue if the strings match
** @@
******************************************************************************/

AjBool ajCharMatchWildCaseC(const char* txt, const char* txt2)
{
    ajint i;

    i = ajCharCmpWildCase(txt, txt2);


    ajDebug("ajCharMatchWildCaseC '%s' '%s' cmp: %d\n",
	    txt, txt2, i);


    if(i)
	return ajFalse;

    return ajTrue;
}

/* @func ajCharMatchWildCaseS *************************************************
**
** Simple case-insensitive test for matching a text string and a string using
** wildcard characters.
**
** @param [r] txt [const char*] String
** @param [r] str [const AjPStr] Wildcard text
** @return [AjBool] ajTrue if the strings match
** @@
******************************************************************************/

AjBool ajCharMatchWildCaseS(const char* txt, const AjPStr str)
{
    return ajCharMatchWildCaseC(txt, str->Ptr);
}

/* @obsolete ajStrMatchWildCO
** @rename ajCharMatchWildS
*/

__deprecated AjBool  ajStrMatchWildCO(const char* str, const AjPStr wild)
{
    return ajCharMatchWildS(str, wild);
}


/* @func ajCharMatchWildNextC *************************************************
**
** Test for matching the next 'word' in two text strings using 
** wildcard characters.
**
** @param [r] txt [const char*] String
** @param [r] txt2 [const char*] Text
** @return [AjBool] ajTrue if found
** @@
******************************************************************************/

AjBool ajCharMatchWildNextC(const char* txt, const char* txt2)
{
    const char* cp;
    const char* cq;
    const char* savecp;
    char lastch = '\0';
    
    ajDebug("ajCharMatchWildNextC '%s' '%s'\n", txt, txt2);

    cp = txt2;
    cq = txt;
    
    if(!*cp && !*cq)
	return ajTrue; /* both empty */

    if(!*cp)
	return ajFalse;	/* no query text */
    
    while(*cp && !isspace((int) *cp))
    {
	if(!*cq && *cp != '*')
	    return ajFalse;

	switch(*cp)
	{
	case '?':		/* skip next character and continue */
	    lastch = *cq;
	    cp++;
	    cq++;
	    break;
	case '*':
	    cp++;		 /* recursive call to test the rest */
	    if(!*cp)
	    {
		ajDebug("...matches at end +%d '%s' +%d '%s'\n",
			 (cq - txt), cq, (cp - txt2), cp);
		return ajTrue;	 /* just match the rest */
	    }

	    if(!*cq)		 /* no more string to compare */
	    {
		savecp = cp;
		while(*cp == '*') {
		    savecp = cp++;	/* may be ***... savecp is last '*' */
		}
		if(!*cp) return ajTrue;
		return ajCharMatchWildNextC(cq,savecp);
	    }

	    while(*cq)
	    {		 /* wildcard in mid name, look for the rest */
		if(ajCharMatchWildNextC(cq, cp)) /* recursive + repeats */
		    return ajTrue;
		ajDebug("...'*' at +%d '%s' +%d '%s' continuing\n",
			 (cq - txt), cq, (cp - txt2), cp);
		cq++;
	    }

	    return ajFalse;	  /* if we're still here, it failed */

	    /* always returns once '*' is found */

	default:	 /* for all other characters, keep checking */
	    if(*cp != *cq)
		return ajFalse;

	    cp++;
	    if(*cq)
	    {
		lastch = *cq;
		cq++;
	    }
	}
    }

    ajDebug("...done comparing at +%d '%s' +%d '%s' lastch '%c'\n",
	     (cq - txt), cq, (cp - txt2), cp, lastch);
    
    if(!isalnum((int) lastch))
    {
	ajDebug("not a word boundary at '%c'\n", lastch);
	return ajFalse;
    }
    
    if(*cp)
    {
	ajDebug("...incomplete cp, FAILED\n");
	return ajFalse ;
    }
    
    if(*cq)
    {
	if(isalnum((int) *cq))
	{
	    ajDebug("word continues, failed\n");
	    return ajFalse;
	}
	ajDebug("word end ... success\n");
	return ajTrue;
    }
    
    ajDebug("...all finished and matched\n");
    
    return ajTrue;
}



/* @func ajCharMatchWildNextCaseC *********************************************
**
** Test for matching the next 'word' in two text strings using 
** wildcard characters, case-insensitive.
**
** @param [r] txt [const char*] String
** @param [r] txt2 [const char*] Text
** @return [AjBool] ajTrue if found
** @@
******************************************************************************/

AjBool ajCharMatchWildNextCaseC(const char* txt, const char* txt2)
{
    const char* cp;
    const char* cq;
    const char* savecp;
    char lastch = '\0';
    
    ajDebug("ajCharMatchWildNextCaseC '%s' '%s'\n", txt, txt2);

    cp = txt2;
    cq = txt;
    
    if(!*cp && !*cq)
	return ajTrue; /* both empty */

    if(!*cp)
	return ajFalse;	/* no query text */
    
    while(*cp && !isspace((int) *cp))
    {
	if(!*cq && *cp != '*')
	    return ajFalse;

	switch(*cp)
	{
	case '?':		/* skip next character and continue */
	    lastch = *cq;
	    cp++;
	    cq++;
	    break;
	case '*':
	    cp++;		 /* recursive call to test the rest */
	    if(!*cp)
	    {
		ajDebug("...matches at end +%d '%s' +%d '%s'\n",
			 (cq - txt), cq, (cp - txt2), cp);
		return ajTrue;	 /* just match the rest */
	    }

	    if(!*cq)		 /* no more string to compare */
	    {
		savecp = cp;
		while(*cp == '*') {
		    savecp = cp++;	/* may be ***... savecp is last '*' */
		}
		if(!*cp) return ajTrue;
		return ajCharMatchWildNextCaseC(cq,savecp);
	    }

	    while(*cq)
	    {		 /* wildcard in mid name, look for the rest */
		if(ajCharMatchWildNextCaseC(cq, cp)) /* recursive + repeats */
		    return ajTrue;
		ajDebug("...'*' at +%d '%s' +%d '%s' continuing\n",
			 (cq - txt), cq, (cp - txt2), cp);
		cq++;
	    }

	    return ajFalse;	  /* if we're still here, it failed */

	    /* always returns once '*' is found */

	default:	 /* for all other characters, keep checking */
	    if(toupper((ajint) *cp) != toupper((ajint) *cq))
		return ajFalse;

	    cp++;
	    if(*cq)
	    {
		lastch = *cq;
		cq++;
	    }
	}
    }

    ajDebug("...done comparing at +%d '%s' +%d '%s' lastch '%c'\n",
	     (cq - txt), cq, (cp - txt2), cp, lastch);
    
    if(!isalnum((int) lastch))
    {
	ajDebug("not a word boundary at '%c'\n", lastch);
	return ajFalse;
    }
    
    if(*cp)
    {
	ajDebug("...incomplete cp, FAILED\n");
	return ajFalse ;
    }
    
    if(*cq)
    {
	if(isalnum((int) *cq))
	{
	    ajDebug("word continues, failed\n");
	    return ajFalse;
	}
	ajDebug("word end ... success\n");
	return ajTrue;
    }
    
    ajDebug("...all finished and matched\n");
    
    return ajTrue;
}



/* @func ajCharMatchWildWordC *************************************************
**
** Case-sensitive test for matching a text string 'word' against any 
** word in a test text string using wildcard characters.
**
** 'Word' is defined as starting and ending with an alphanumeric character
** (A-Z a-z 0-9) with no white space.
**
** The query text can use '*' or '?' as a wildcard.
**
** @param [r] txt [const char*] String
** @param [r] txt2 [const char*] Wildcard word
** @return [AjBool] ajTrue if found
** @@
******************************************************************************/

AjBool ajCharMatchWildWordC(const char* txt, const char* txt2)
{
    const char* cp;
    const char* cq;
    AjBool word = ajFalse;

    ajDebug("ajCharMatchWildWordC('%s', '%s')\n", txt, txt2);

    cp = txt2;
    cq = txt;

    if(!*cp && !*cq)
	return ajTrue;

    if(!*cp)
	return ajFalse;

    ajDebug("something to test, continue...\n");

    /* unlike ajCharMatchWild, we step through the string (str) not the
       query (text) */

    while(*cq)
    {
	if(!word)
	{
	    if(isalnum((int) *cq))
	    {				/* start of word */
		word = ajTrue;
		if(ajCharMatchWildNextC(cq, txt2))
		    return ajTrue;
	    }
	}
	else
	{
	    if(!isalnum((int) *cq))
		word = ajFalse;
	}

	cq++;
    }

    return ajFalse;
}


/* @func ajCharMatchWildWordCaseC *********************************************
**
** Case-insensitive test for matching a text string 'word' against any 
** word in a text string using wildcard characters.
**
** 'Word' is defined as starting and ending with an alphanumeric character
** (A-Z a-z 0-9) with no white space.
**
** The query text can use '*' or '?' as a wildcard.
**
** @param [r] txt [const char*] String
** @param [r] txt2 [const char*] Wildcard word
** @return [AjBool] ajTrue if found
** @@
******************************************************************************/

AjBool ajCharMatchWildWordCaseC(const char* txt, const char* txt2)
{
    const char* cp;
    const char* cq;
    AjBool word = ajFalse;

    ajDebug("ajCharMatchWildWordCaseC('%s', '%s')\n", txt, txt2);

    cp = txt2;
    cq = txt;

    if(!*cp && !*cq)
	return ajTrue;

    if(!*cp)
	return ajFalse;

    ajDebug("something to test, continue...\n");

    /* unlike ajCharMatchWild, we step through the string (str) not the
       query (text) */

    while(*cq)
    {
	if(!word)
	{
	    if(isalnum((int) *cq))
	    {				/* start of word */
		word = ajTrue;
		if(ajCharMatchWildNextCaseC(cq, txt2))
		    return ajTrue;
	    }
	}
	else
	{
	    if(!isalnum((int) *cq))
		word = ajFalse;
	}

	cq++;
    }

    return ajFalse;
}


/* @obsolete ajStrMatchWordCC
** @rename ajCharMatchWildWordS
*/

__deprecated AjBool  ajStrMatchWordCC (const char* str, const char* txt)
{
    return ajCharMatchWildWordC(str, txt);
}

/* @func ajCharPrefixC ********************************************************
**
** Test for matching the start of a text string against a given prefix text 
** string.
**
** @param [r] txt [const char*]  Test string as text
** @param [r] txt2 [const char*] Prefix as text
** @return [AjBool] ajTrue if the string begins with the prefix
** @@
******************************************************************************/

AjBool ajCharPrefixC(const char* txt, const char* txt2)
{
    ajuint ilen;

    if(!txt)
	return ajFalse;
    if(!txt2)
	return ajFalse;

    ilen = strlen(txt2);

    if(!ilen)				/* no prefix */
	return ajFalse;

    if(ilen > strlen(txt))		/* pref longer */
	return ajFalse;

    if(strncmp(txt, txt2, ilen))    /* +1 or -1 for a failed match */
	return ajFalse;

    return ajTrue;
}

/* @obsolete ajStrPrefixCC
** @rename ajCharPrefixC
*/

__deprecated AjBool  ajStrPrefixCC(const char* str, const char* pref)
{
    return ajCharPrefixC(str, pref);
}



/* @func ajCharPrefixS ********************************************************
**
** Test for matching the start of a text string against a given prefix string.
**
** @param [r] txt [const char*] Test string as text
** @param [r] str [const AjPStr] Prefix as string
** @return [AjBool] ajTrue if the string begins with the prefix
** @@
******************************************************************************/

AjBool ajCharPrefixS(const char* txt, const AjPStr str)
{
    if(!txt)
	return ajFalse;

    if(str->Len > strlen(txt))	/* pref longer */
	return ajFalse;

    if(strncmp(txt, str->Ptr, str->Len)) /* +1 or -1 for a failed match */
	return ajFalse;

    return ajTrue;
}


/* @obsolete ajStrPrefixCO
** @rename ajCharPrefixS
*/

__deprecated AjBool  ajStrPrefixCO(const char* str, const AjPStr thys)
{
    return ajCharPrefixS(str, thys);
}

/* @func ajCharPrefixCaseC ****************************************************
**
** Case-insensitive test for matching the start of a text string against a 
** given prefix text string.
**
** @param [r] txt [const char*] Text
** @param [r] txt2 [const char*] Prefix
** @return [AjBool] ajTrue if the string begins with the prefix
** @@
******************************************************************************/

AjBool ajCharPrefixCaseC(const char* txt, const char* txt2)
{
    const char* cp;
    const char* cq;

    cp = txt;
    cq = txt2;

    /* ajDebug("ajCharPrefixCaseC '%s' '%s'\n", txt, txt2); */

    if(!*cq)
	return ajFalse;

    while(*cp && *cq)
    {
	if(toupper((ajint) *cp) != toupper((ajint) *cq)) return ajFalse;
	cp++;cq++;
    }

    if(*cq)
	return ajFalse;

    /* ajDebug("ajStrPrefixCaseCC ..TRUE..\n"); */
    return ajTrue;
}

/* @obsolete ajStrPrefixCaseCC
** @rename ajCharPrefixCaseC
*/

__deprecated AjBool  ajStrPrefixCaseCC(const char* thys, const char* pref)
{
    return ajCharPrefixCaseC(thys, pref);
}

/* @func ajCharPrefixCaseS ****************************************************
**
** Case-insensitive test for matching start of a text string against a given 
** prefix string,
**
** @param [r] txt [const char*] Text
** @param [r] str [const AjPStr] Prefix
** @return [AjBool] ajTrue if the string begins with the prefix
** @@
******************************************************************************/

AjBool ajCharPrefixCaseS(const char* txt, const AjPStr str)
{
    return ajCharPrefixCaseC(txt, str->Ptr);
}


/* @obsolete ajStrPrefixCaseCO
** @rename ajCharPrefixCaseS
*/

__deprecated AjBool  ajStrPrefixCaseCO(const char* thys, const AjPStr pref)
{
    return ajCharPrefixCaseS(thys, pref);
}



/* @func ajCharSuffixC ********************************************************
**
** Test for matching the end of a text string against a given suffix text 
** string.
**
** @param [r] txt [const char*] String
** @param [r] txt2 [const char*] Suffix as text
** @return [AjBool] ajTrue if the string ends with the suffix
** @@
******************************************************************************/

AjBool ajCharSuffixC(const char* txt, const char* txt2)
{
    ajuint ilen;
    ajuint jlen;
    ajuint jstart;

    ilen   = strlen(txt2);
    jlen   = strlen(txt);
    jstart = jlen - ilen;

    if(ilen > jlen)			/* suff longer */
	return ajFalse;

    if(strncmp(&txt[jstart], txt2, ilen)) /* +1 or -1 for a
					      failed match */
	return ajFalse;

    return ajTrue;
}

/* @obsolete ajStrSuffixCC
** @rename ajCharSuffixC
*/

__deprecated AjBool  ajStrSuffixCC(const char* str, const char* suff)
{
    return ajCharSuffixC(str, suff);
}

/* @func ajCharSuffixS ********************************************************
**
** Test for matching the end of a text string against a given suffix string.
**
** @param [r] txt [const char*] Test string as text
** @param [r] str [const AjPStr] Suffix as string
** @return [AjBool] ajTrue if the string ends with the suffix
** @@
******************************************************************************/

AjBool ajCharSuffixS(const char* txt, const AjPStr str)
{
    ajuint jlen;
    ajuint jstart;

    jlen   = strlen(txt);
    jstart = jlen - str->Len;

    if(str->Len > jlen)		/* suff longer */
	return ajFalse;

    if(strncmp(&txt[jstart], str->Ptr, str->Len)) /* +1 or -1 for a
							failed match */
	return ajFalse;

    return ajTrue;
}



/* @obsolete ajStrSuffixCO
** @rename ajCharSuffixS
*/

__deprecated AjBool  ajStrSuffixCO(const char* txt, const AjPStr suff)
{
    return ajCharSuffixS(txt, suff);
}

/* @func ajCharSuffixCaseC ****************************************************
**
** Case-insensitive test for matching the end of a text string against a given 
** suffix text string.
**
** @param [r] txt [const char*] String
** @param [r] txt2 [const char*] Suffix as text
** @return [AjBool] ajTrue if the string ends with the suffix
** @@
******************************************************************************/

AjBool ajCharSuffixCaseC(const char* txt, const char* txt2)
{
    ajuint ilen;
    ajuint jlen;
    ajuint jstart;
    const char* cp;
    const char* cq;

    ilen   = strlen(txt2);
    jlen   = strlen(txt);
    jstart = jlen - ilen;

    if(ilen > jlen)			/* suff longer */
	return ajFalse;

    cp = &txt[jstart];
    cq = txt2;
    while (*cp)
    {
	if(toupper((ajint)*cp) != toupper((ajint)*cq)) return ajFalse;
	cp++; cq++;
    }

    return ajTrue;
}

/* @func ajCharSuffixCaseS ****************************************************
**
** Case-insensitive test for matching the end of a text string against a given 
** suffix string.
**
** @param [r] txt [const char*] Test string as text
** @param [r] str [const AjPStr] Suffix as string
** @return [AjBool] ajTrue if the string ends with the suffix
** @@
******************************************************************************/

AjBool ajCharSuffixCaseS(const char* txt, const AjPStr str)
{
    ajuint jlen;
    ajuint jstart;
    const char* cp;
    const char* cq;
    jlen   = strlen(txt);
    jstart = jlen - str->Len;

    if(str->Len > jlen)		/* suff longer */
	return ajFalse;

    cp = &txt[jstart];
    cq = str->Ptr;
    while (cp)
    {
	if(toupper((ajint)*cp) != toupper((ajint)*cq)) return ajFalse;
	cp++; cq++;
    }

    return ajTrue;
}



/* @section comparison (sorting)
**
** Functions for sorting strings (including C-type char* strings).
**
** @fdata      [char*]
** @fnote     namrule same as "String comparison (sorting) functions".
**
** @nam3rule  Cmp           Compare two complete strings & return sort order.
** @nam4rule  CmpCase     Case-insensitive comparison.
** @nam4rule  CmpWild     Comparison using wildcard characters.
** @nam5rule  CmpWildCase Case-insensitive comparison using
**                        wildcard characters.
**
** @argrule   * txt [const char*] Character string
** @argrule   * txt2 [const char*] Comparison string
** @argrule   Len len [ajuint] Number of characters to compare
** @valrule   * [int]  -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
** @fcategory use
*/


/* @func ajCharCmpCase ********************************************************
**
** Finds the sort order of two text strings. 
**
** @param [r] txt [const char*] Text string
** @param [r] txt2 [const char*] Text string
** @return [int] -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
** @@
******************************************************************************/

int ajCharCmpCase(const char* txt, const char* txt2)
{
    const char* cp;
    const char* cq;

    for(cp = txt, cq = txt2; *cp && *cq; cp++, cq++)
    {
	if(toupper((ajint) *cp) != toupper((ajint) *cq))
	{
	    if(toupper((ajint) *cp) > toupper((ajint) *cq))
		return 1;
	    else
		return -1;
	}
    }

    if(*cp)
	return 1;

    if(*cq)
	return -1;

    return 0;
}


/* @obsolete ajStrCmpCaseCC
** @rename ajCharCmpCase
*/

__deprecated int  ajStrCmpCaseCC(const char* str1, const char* str2)
{
    return ajCharCmpCase (str1, str2);
}

/* @func ajCharCmpCaseLen *****************************************************
**
** Finds the sort order of substrings from the start of two text strings.
**
** @param [r] txt [const char*] Text string
** @param [r] txt2 [const char*] Text string
** @param [r] len  [ajuint] length
** @return [int] -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
** @@
******************************************************************************/

int ajCharCmpCaseLen(const char* txt, const char* txt2, ajuint len)
{
    const char* cp;
    const char* cq;
    ajuint i;

    for(cp=txt,cq=txt2,i=0;*cp && *cq && i<len;++i,++cp,++cq)
	if(toupper((ajint) *cp) != toupper((ajint) *cq))
	{
	    if(toupper((ajint) *cp) > toupper((ajint) *cq))
		return 1;
	    else
		return -1;
	}

    if(i==len)
	return 0;

    if(*cp)
	return 1;

    return -1;
}


/* @obsolete ajStrNCmpCaseCC
** @rename ajCharCmpCaseLen
*/

__deprecated int  ajStrNCmpCaseCC(const char* str1, const char* str2, ajint len)
{
    return ajCharCmpCaseLen(str1, str2, len);
}

/* @func ajCharCmpWild ********************************************************
**
** Finds the sort-order (case insensitive) of two text strings using
** wildcard characters. 
**
** @param [r] txt [const char*] String
** @param [r] txt2 [const char*] Text
** @return [int]  -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
** @@
******************************************************************************/

int ajCharCmpWild(const char* txt, const char* txt2)
{
    const char* cp;
    const char* cq;
    
    /*ajDebug("ajCharCmpWild('%s', '%s')\n", str, text);*/

    cp = txt2;
    cq = txt;
    
    if(!*cp && !*cq)
	return 0;

    if(!*cp)
	return -1;
    
    /*ajDebug("something to test, continue...\n");*/
    
    while(*cp)
    {
	if(!*cq && *cp != '*')
	    return 1;

	switch(*cp)
	{
	case '?':		/* skip next character and continue */
	    cp++;
	    cq++;
	    break;
	case '*':
	    cp++;
	    if(!*cp)
	    {
		/* ajDebug("...matches at end +%d '%s' +%d '%s'\n",
		   (cq - str), cq, (cp - text), cp);*/
		return 0;		/* just match the rest */
	    }

	    if(!*cq)
	    {
		/*ajDebug("...test match to null string just in case\n");*/
		return ajCharCmpWild(cq, cp);
	    }

	    while(*cq)
	    {		 /* wildcard in mid name, look for the rest */
		if(ajCharMatchWildC(cq, cp))
		    return 0; /* recursive + repeats */
		/* ajDebug("...'*' at +%d '%s' +%d '%s' continuing\n",
		   (cq - str), cq, (cp - text), cp);*/
		cq++;
	    }

	    return 1;

	    /* always returns once '*' is found */

	default:	 /* for all other characters, keep checking */
	    if(*cp != *cq)
	    {
		if(*cp > *cq)
		    return -1;
		else
		    return 1;
	    }
	    cp++;
	    if(*cq)
		cq++;
	}
    }

    /*ajDebug("...done comparing at +%d '%s' +%d '%s'\n",
      (cq - str), cq, (cp - text), cp);*/
    if(*cp)
    {
	/*ajDebug("...incomplete cp, FAILED\n");*/
	return -1 ;
    }

    if(*cq)
    {
	/*ajDebug("...incomplete cq, FAILED\n");*/
	return 1;
    }
    /*ajDebug("...all finished and matched\n");*/
    
    return 0;
}


/* @func ajCharCmpWildCase ****************************************************
**
** Finds the sort-order (case sensitive) of two text strings using
** wildcard characters. 
**
** @param [r] txt [const char*] String
** @param [r] txt2 [const char*] Text
** @return [int]  -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
** @@
******************************************************************************/

int ajCharCmpWildCase(const char* txt, const char* txt2)
{
    const char* cp;
    const char* cq;
    
    ajDebug("ajCharCmpWildCase('%s', '%s')\n", txt, txt2);

    cp = txt2;
    cq = txt;
    
    if(!*cp && !*cq)
	return 0;

    if(!*cp)
	return -1;
    
    /*ajDebug("something to test, continue...\n");*/
    
    while(*cp)
    {
	if(!*cq && *cp != '*')
	    return 1;

	switch(*cp)
	{
	case '?':		/* skip next character and continue */
	    cp++;
	    cq++;
	    break;
	case '*':
	    cp++;
	    if(!*cp)
	    {
		/* ajDebug("...matches at end +%d '%s' +%d '%s'\n",
		   (cq - str), cq, (cp - text), cp);*/
		return 0;		/* just match the rest */
	    }

	    if(!*cq)
	    {
		/*ajDebug("...test match to null string just in case\n");*/
		return ajCharCmpWildCase(cq, cp);
	    }

	    while(*cq)
	    {		 /* wildcard in mid name, look for the rest */
		if(ajCharMatchWildCaseC(cq, cp))
		    return 0; /* recursive + repeats */
		/* ajDebug("...'*' at +%d '%s' +%d '%s' continuing\n",
		   (cq - str), cq, (cp - text), cp);*/
		cq++;
	    }

	    return 1;

	    /* always returns once '*' is found */

	default:	 /* for all other characters, keep checking */
	    if(toupper((ajint) *cp) != toupper((ajint) *cq))
	    {
		if(toupper((ajint) *cp) > toupper((ajint) *cq))
		    return -1;
		else
		    return 1;
	    }
	    cp++;
	    if(*cq)
		cq++;
	}
    }

    /*ajDebug("...done comparing at +%d '%s' +%d '%s'\n",
      (cq - str), cq, (cp - text), cp);*/
    if(*cp)
    {
	/*ajDebug("...incomplete cp, FAILED\n");*/
	return -1 ;
    }

    if(*cq)
    {
	/*ajDebug("...incomplete cq, FAILED\n");*/
	return 1;
    }
    /*ajDebug("...all finished and matched\n");*/
    
    return 0;
}

/* @obsolete ajStrCmpWildCC
** @rename ajCharCmpWild
*/

__deprecated ajint  ajStrCmpWildCC(const char* str, const char* text)
{
    return ajCharCmpWild (str, text);
}

/* @section parsing functions *************************************************
**
** Simple token parsing of strings
**
** @fdata [char*]
**
** @nam3rule Parse Parsing
** @argrule * txt [const char*] Text to be parsed
** @argrule C txtdelim [const char*] Delimiter set
** @valrule * [AjPStr] String token result, now owned by calling function.
** @fcategory use
*/


/* @func ajCharParseC *********************************************************
**
** Simple token parsing from text string using a specified set of delimiters.
**
** @param [r] txt [const char*] String to be parsed (first call) or
**        NULL for followup calls using the same string, as for the
**        C RTL function strtok which is eventually called.
** @param [r] txtdelim [const char*] Delimiter(s) to be used between tokens.
** @return [AjPStr] Token
** @error NULL if no further token is found.
** @@
******************************************************************************/

AjPStr ajCharParseC (const char* txt, const char* txtdelim)
{
    static AjPStr strp = NULL; /* internal AjPStr - do not try to destroy */
    static char* cp    = NULL;

    if (!strp)
    {
	if (!txt)
	{
	    ajWarn("Error in ajCharParseC: NULL argument and not initialised");
	    ajUtilCatch();
	    return NULL;
	}
	AJNEW0(strp);
	strp->Use = 1;
    }

    if (txt)
    {
	if (cp) ajCharDel(&cp);
	cp = ajCharNewC(txt);
	strp->Ptr = ajSysFuncStrtok(cp, txtdelim);
    }
    else
    {
	strp->Ptr = ajSysFuncStrtok(NULL, txtdelim);
    }

    if (strp->Ptr)
    {
	strp->Len = strlen(strp->Ptr);
	strp->Res = strp->Len + 1;
	return strp;
    }
    else
    {
	strp->Len=0;
	strp->Res=1;
	strp->Use=0;
    }

    return NULL;
}



/* @obsolete ajStrTokCC
** @rename ajCharParseC
*/

__deprecated const AjPStr  ajStrTokCC (const char* txt, const char* delim)
{
    return ajCharParseC(txt, delim);
}

/* @datasection [AjPStr] String ***********************************************
**
** Functions for manipulating AJAX (AjPStr) strings
**
** @nam2rule Str    Function is for manipulating strings
**
*/



/* @section constructors ******************************************************
**
** Functions for constructing string objects, possibly with a starting string. 
**
** @fdata      [AjPStr]
** @fnote     Same namrule as "String constructor functions:
**            C-type (char*) strings"
** @nam3rule  New      Construct a new string.
** @nam4rule  NewClear Construct with an empty string.
** @nam4rule  NewRef   Construct making a reference counted copy of
**                     an existing string.
** @nam4rule  NewRes   Construct with reserved size.
**
** @argrule   C       txt [const char*] Text string
** @argrule   S       str [const AjPStr] Text string
** @argrule   NewRes  size [ajuint] Reserved size
** @argrule   NewRef  refstr [AjPStr] Text string to be duplicated
** @argrule   Len     len [ajuint] Length of string
** @valrule   *     [AjPStr] New string
** @fcategory new
*/


/* @func ajStrNew *************************************************************
**
** Default string constructor which allocates memory for a string.
** 
** The null string usage pointer is incremented.
**
** @return [AjPStr] Pointer to an empty string
** @@
******************************************************************************/

AjPStr ajStrNew(void)
{
    return ajStrNewRef(strPNULL);
}


/* @func ajStrNewC ************************************************************
**
** String constructor which allocates memory for a string and initialises it
** with the text string provided. 
** 
** The string size is set just large enough to hold the supplied text.
**
** @param [r] txt [const char*] Null-terminated character string to initialise
**        the new string.
** @return [AjPStr] Pointer to a string containing the supplied text
** @@
******************************************************************************/

AjPStr ajStrNewC(const char* txt)
{
    ajuint i;
    ajuint j;
    AjPStr thys;

    i = strlen(txt);
    j = ajRound(i + 1, STRSIZE);

    thys = ajStrNewResLenC(txt, j, i);

    return thys;
}




/* @func ajStrNewS ************************************************************
**
** String constructor which allocates memory for a string and initialises it
** with the string provided.
**
** @param [r] str [const AjPStr] String to be cloned
** @return [AjPStr] Pointer to a string of the specified size
**         containing the supplied text.
** @@
******************************************************************************/

AjPStr ajStrNewS(const AjPStr str)
{
  if(str)
    return ajStrNewResLenC(str->Ptr, str->Res, str->Len);
  else
    return ajStrNewResLenC("",1,0);
}




/* @func ajStrNewRef **********************************************************
**
** String referencing function which returns a pointer to the string passed in
** and increases its reference count.
**
** Not a true string copy, but if modified the new string will behave
** as a copy.
**
** @param [u] refstr [AjPStr] AJAX string object
** @return [AjPStr] Pointer to the string passed as an argument,
**         with its use count increased by 1.
** @@
******************************************************************************/

AjPStr ajStrNewRef(AjPStr refstr)
{
    if(!refstr)
	ajFatal("ajStrNewRef source string NULL");

    refstr->Use++;

    return refstr;
}


/* @obsolete ajStrDup
** @rename ajStrNewRef
*/

__deprecated AjPStr  ajStrDup(AjPStr thys)
{
    return ajStrNewRef(thys);
}

/* @func ajStrNewRes **********************************************************
**
** String constructor which allocates memory for a string of an initial 
** reserved size (including a possible null).
**
** @param [r] size [ajuint] Reserved size (including a possible null).
** @return [AjPStr] Pointer to an empty string of specified size.
** @@
******************************************************************************/

AjPStr ajStrNewRes(ajuint size)
{
    AjPStr thys;

    thys = ajStrNewResLenC("", size, 0);

    return thys;
}

/* @obsolete ajStrNewL
** @rename ajStrNewRes
*/

__deprecated AjPStr  ajStrNewL(size_t size)
{
    return ajStrNewRes(size);
}

/* @func ajStrNewResC *********************************************************
**
** String constructor which allocates memory for a string of an initial 
** reserved size (including a possible null) and initialises it with the text 
** string provided.
**
** @param [r] txt [const char*] Null-terminated character string to initialise
**        the new string.
** @param [r] size [ajuint]  Reserved size (including a possible null).
** @return [AjPStr] Pointer to a string of the specified size
**         containing the supplied text.
** @@
******************************************************************************/

AjPStr ajStrNewResC(const char* txt, ajuint size)
{
    ajuint i;
    AjPStr thys;

    i = strlen(txt);

    thys = ajStrNewResLenC(txt, size, i);

    return thys;
}



/* @obsolete ajStrNewCL
** @rename ajStrNewResC
*/

__deprecated AjPStr  ajStrNewCL(const char* txt, size_t size)
{
    return ajStrNewResC(txt, size);
}

/* @func ajStrNewResS *********************************************************
**
** String constructor which allocates memory for a string of an initial 
** reserved size (including a possible null) and initialises it with the string
** provided.
**
** @param [r] str [const AjPStr] String to initialise
**        the new string.
** @param [r] size [ajuint]  Reserved size (including a possible null).
** @return [AjPStr] Pointer to a string of the specified size
**         containing the supplied text.
** @@
******************************************************************************/

AjPStr ajStrNewResS(const AjPStr str, ajuint size)
{
    AjPStr thys;

    thys = ajStrNewResLenC(str->Ptr, size, str->Len);

    return thys;
}


/* @func ajStrNewResLenC ******************************************************
**
** String constructor which allocates memory for a string of a given length 
** and of an initial reserved size (including a possible null) and initialises 
** it with the text string provided.
**
** @param [r] txt [const char*] Null-terminated character string to initialise
**        the new string.
** @param [r] size [ajuint]  Reserved size, including a trailing null and
**                           possible space for expansion
** @param [r] len [ajuint] Length of txt to save calculation time.
** @return [AjPStr] Pointer to a string of the specified size
**         containing the supplied text.
** @@
******************************************************************************/

AjPStr ajStrNewResLenC(const char* txt, ajuint size, ajuint len)
{
    AjPStr thys;
    ajuint minlen;

    minlen = size;

    if(size <= len)
	minlen = len+1;

    thys = strNew(minlen);
    thys->Len = len;
    if(txt)
	memmove(thys->Ptr, txt, len+1);
    thys->Ptr[len] = '\0';

    return thys;
}

/* @obsolete ajStrNewCIL
** @rename ajStrNewResLenC
*/

__deprecated AjPStr  ajStrNewCIL(const char* txt, ajint len, size_t size)
{
    return ajStrNewResLenC (txt, size, len);
}

/* @funcstatic strClone *******************************************************
**
** Makes a new clone of a string with a usage count of one and unchanged
** reserved size.
**
** @param [w] Pstr [AjPStr*] String
** @return [AjPStr] New String
** @@
******************************************************************************/

static AjPStr strClone(AjPStr* Pstr)
{
    AjPStr thys;
    AjPStr ret;

    thys = *Pstr;
    ret = ajStrNewResLenC(thys->Ptr, thys->Res, thys->Len);

    if(thys)
    {
      if(thys->Use <= 1)
	ajStrDel(Pstr);
      else
	thys->Use--;
    }

    *Pstr = ret;

    return ret;
}





/* @funcstatic strCloneL ******************************************************
**
** Makes a new clone of a string with a usage count of one and a minimum
** reserved size.
**
** @param [w] Pstr [AjPStr*] String
** @param [r] size [ajuint] Minimum reserved size.
** @return [void]
** @@
******************************************************************************/

static void strCloneL(AjPStr* Pstr, ajuint size)
{
    AjPStr thys;
    AjPStr ret;

    thys = *Pstr;
    ret = ajStrNewResLenC(thys->Ptr, size, thys->Len);

    if(thys)
    {
      if(thys->Use <= 1)
	ajStrDel(Pstr);
      else
	thys->Use--;
    }

    *Pstr = ret;

    return;
}





/* @funcstatic strNew ******************************************************
**
** Internal constructor for modifiable AJAX strings. Used by all the string
** parameterized contructors to allocate the space for the text string.
** The only exception is ajStrNew which returns a clone of the null string.
**
** @param [rE] size [ajuint] size of the reserved space, including the
**        terminating NULL character. Zero uses a default string size STRSIZE.
** @return [AjPStr] A pointer to an empty string
** @@
******************************************************************************/

static AjPStr strNew(ajuint size)
{
    AjPStr ret;

    if(!size)
	size = STRSIZE;

    AJNEW0(ret);
    ret->Res = size;
    ret->Ptr = AJALLOC0(size);
    ret->Len = 0;
    ret->Use = 1;
    ret->Ptr[0] = '\0';

#ifdef AJ_SAVESTATS
    strAlloc += size;
    strCount++;
    strTotal++;
#endif

    return ret;
}




/* @section destructors *******************************************************
**
** Functions for destruction of string objects.
**
** @fdata     [AjPStr]
** @fnote     Return type could be standardised.
**
** @nam3rule  Del       Destroy (free) an existing string.
** @nam3rule  Delarray  Destroy (free) an array of strings
** @nam4rule  DelStatic Destroy (clear) a string without freeing memory
**                      to save reallocation of static string variables
** 
** @argrule   Del Pstr [AjPStr*]
** @argrule   Delarray PPstr [AjPStr**]
** 
** @valrule   * [void]
** @valrule   *DelStatic [AjBool] True if the string still exists as empty
**                               False if the string was reference counted
**                               and this instance was deleted
** @fcategory delete
******************************************************************************/



/* @macro MAJSTRDEL ***********************************************************
**
** Default string destructor which frees memory for a string.
**
** Decrements the use count. When it reaches zero, the string is removed from
** memory.  If the given string is NULL, or a NULL pointer, simply returns.
**
** A macro version of {ajStrDel} available in case it is needed for speed.
**
** @param  [d] Pstr [AjPStr*] Pointer to the string to be deleted.
**         The pointer is always deleted.
** @return [void]
** @@
******************************************************************************/


/* @func ajStrDel *************************************************************
**
** Default string destructor which frees memory for a string.
** 
** Decrements the use count. When it reaches zero, the string is removed from
** memory.  If the given string is NULL, or a NULL pointer, simply returns.
**
** @param  [d] Pstr [AjPStr*] Pointer to the string to be deleted.
**         The pointer is always deleted.
** @return [void]
** @cre    The default null string must not be deleted. Calling this
**         routine for copied pointers could cause this. An error message
**         is issued and the null string use count is restored.
** @@
******************************************************************************/

void ajStrDel(AjPStr* Pstr)
{
    AjPStr thys;

    if(!Pstr)
	return;

    if(!*Pstr)
	return;

    thys = *Pstr;

    --thys->Use;
    if(!thys->Use)
    {					/* any other references? */
	if(thys == strPNULL)
	{
	    ajErr("Error - trying to delete the null string constant\n");
	    thys->Use = NULL_USE;     /* restore the original value */
	}
	else
	{
	    AJFREE(thys->Ptr);		/* free the string */

#ifdef AJ_SAVESTATS
	    strFree += thys->Res;
	    strFreeCount++;
	    strCount--;
#endif

	    thys->Res = 0;	      /* in case of copied pointers */
	    thys->Len = 0;

	    AJFREE(*Pstr);		/* free the object */
	}
    }

    *Pstr = NULL;

    return;
}



/* @func ajStrDelStatic *******************************************************
**
** String dereferencing function which sets a string pointer to NULL and 
** decrements the use count of the string to which it referred. Strings with a
** use count of 1 are *not* freed to avoid freeing and reallocating memory when
** they are reused.
**
** Memory reserved for the string is never deleted and can always be
** reused by any remaining AjPStr that points to it even if this pointer
** is cleared.
**
** Use for more efficient memory management for static strings, to avoid
** reallocation when used in a subsequent call.
**
** If the given string is NULL, or a NULL pointer, simply returns.
**
** @param  [w] Pstr [AjPStr*] Pointer to the string to be deleted.
** @return [AjBool] True if the string exists and can be reused
**                  False if the string was deleted.
** @@
******************************************************************************/

AjBool ajStrDelStatic(AjPStr* Pstr)
{
    AjPStr thys;

    if(!*Pstr)
	return ajFalse;

    thys = *Pstr;

    if(thys->Use == 1)
    {			       /* last reference - clear the string */
	*thys->Ptr = '\0';
	thys->Len  = 0;
	return ajTrue;
    }
    else
    {
	--thys->Use;
	*Pstr = NULL;
    }

    return ajFalse;
}




/* @obsolete ajStrDelReuse
** @rename ajStrDelStatic
*/

__deprecated AjBool  ajStrDelReuse(AjPStr* pthis)
{
    ajStrDelStatic(pthis);
    if(*pthis) return ajTrue;
    return ajFalse;
}


/* @func ajStrDelarray ********************************************************
**
** Default string destructor which frees memory for an array of strings.
** 
** Decrements the use count. When it reaches zero, the string is removed from
** memory.  If the given string is NULL, or a NULL pointer, simply returns.
**
** @param  [d] PPstr [AjPStr**] Pointer to the string array to be deleted.
**         The last string is a NULL pointer
**         The pointer is always deleted.
** @return [void]
** @cre    The default null string must not be deleted. Calling this
**         routine for copied pointers could cause this. An error message
**         is issued and the null string use count is restored.
** @@
******************************************************************************/

void ajStrDelarray(AjPStr** PPstr)
{
    ajuint i = 0;

    if(!PPstr)
	return;

    if(!*PPstr)
	return;

    for(i=0;(*PPstr)[i];i++)
	ajStrDel(&(*PPstr)[i]);
    AJFREE(*PPstr);

    return;
}


/* @section assignment ********************************************************
**
** Functions for assigning a string.
**
** @fdata      [AjPStr]
** @fnote     Memory for the string is allocated if necessary.
**
** @nam3rule  Assign      Assign a string (or substring) either by reference
**                        or duplication.
** @nam4rule  AssignRef   Copy by setting pointer reference only.
** @nam4rule  AssignSub   Copy a substring.
** @nam4rule  AssignRes   Copy into string of a reserved size (at least).
** @nam4rule  AssignMax   Copy up to a maximum number of characters only.
** @nam4rule  AssignEmpty Copy only if existing string is empty.
** @nam4rule  AssignClear Assign an empty string
**
** @argrule   *       Pstr [AjPStr*] Modifiable string
** @argrule   Ref     refstr [AjPStr] Master string
** @argrule   Res     size [ajuint] Reserved maximum size
** @argrule   C       txt [const char*] Text string
** @argrule   K       chr [char] Single character
** @argrule   S       str [const AjPStr] Text string
** @argrule   Len     len [ajuint] Length of string
** @argrule   Sub     pos1 [ajint] Start position, negative value counts
**                                 from end
** @argrule   Sub     pos2 [ajint] End position, negative value counts from end
** @valrule   * [AjBool]
**
** @fcategory assign
*/


/* @func ajStrAssignC *********************************************************
**
** Copy a text string to a string.
**
** @param [w] Pstr [AjPStr*] Target string.
** @param [r] txt [const char*] Source text.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrAssignC(AjPStr* Pstr, const char* txt)
{
    AjBool ret = ajFalse;
    AjPStr thys;
    ajuint i;
    ajuint ires;

    if(!*Pstr)
    {
	if(txt)
            *Pstr = ajStrNewC(txt);
        else
            *Pstr = ajStrNew();
	return ajTrue;
    }

    if (!txt)
      i = 0;
    else
      i = strlen(txt);

    ires = i+1;

    thys = *Pstr;
    if(thys->Use != 1 || thys->Res <= i)
    {
      ret  = ajStrSetResRound(Pstr, ires);
      thys = *Pstr;
    }

    thys->Len = i;

    if(i)
	memmove(thys->Ptr, txt, ires);
    else
	thys->Ptr[0] = '\0';

    return ret;
}

/* @obsolete ajStrAssC
** @rename ajStrAssignC
*/

__deprecated AjBool  ajStrAssC(AjPStr* pthis, const char* text)
{
    return ajStrAssignC (pthis, text);
}

/* @func ajStrAssignK *********************************************************
**
** Copy a single character to a string.
**
** @param [w] Pstr [AjPStr*] Target string.
** @param [r] chr [char] Source text.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrAssignK(AjPStr* Pstr, char chr)
{
    AjBool ret = ajFalse;
    AjPStr thys;

    if(!*Pstr)
      *Pstr=ajStrNewRes(2);

    thys = *Pstr;
    if(thys->Use != 1 || thys->Res <= 1)
    {
      ret  = ajStrSetRes(Pstr, 2);
      thys = *Pstr;
    }

    thys->Ptr[0] = chr;
    thys->Ptr[1] = '\0';
    thys->Len = 1;

    return ret;
}


/* @obsolete ajStrAssK
** @rename ajStrAssignK
*/

__deprecated AjBool  ajStrAssK(AjPStr* pthis, const char text)
{
    return ajStrAssignK(pthis, text);
}

/* @func ajStrAssignS *********************************************************
**
** Copy a string to a string.
**
** This is a genuine copy (reference count isn't used).  Useful where both 
** strings will be separately overwritten later so that they can both remain
** modifiable.
**
** @param [w] Pstr [AjPStr*] Target string.
** @param [rN] str [const AjPStr] Source string.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrAssignS(AjPStr* Pstr, const AjPStr str)
{
    AjBool ret = ajFalse;
    AjPStr thys;

    if(!*Pstr)
    {
	*Pstr = ajStrNewS(str);
	return ajTrue;
    }

    if(!str)
	return ajStrAssignClear(Pstr);

    thys = *Pstr;
    if(thys->Use != 1 || thys->Res <= str->Len)
    {
      ret = ajStrSetResRound(Pstr, str->Len+1); /* min. reserved size OR more */
      thys = *Pstr;
    }

    thys->Len = str->Len;
    memmove(thys->Ptr, str->Ptr, str->Len+1);

    return ret;
}


/* @obsolete ajStrAssS
** @rename ajStrAssignS
*/

__deprecated AjBool  ajStrAssS(AjPStr* pthis, const AjPStr str)
{
    return ajStrAssignS(pthis, str);
}

/* @obsolete ajStrAssI
** @replace ajStrAssignS (1,2,3/1,2)
*/

__deprecated AjBool  ajStrAssI(AjPStr* Pstr, const AjPStr str, size_t i)
{
    (void) i;
    return ajStrAssignS(Pstr, str);
}

/* @func ajStrAssignClear *****************************************************
**
** Clear the string value
**
** @param [w] Pstr [AjPStr*] Target string.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrAssignClear(AjPStr* Pstr)
{
    AjBool ret = ajFalse;
    AjPStr thys;

    if(!*Pstr)
    {
	*Pstr = strNew(0);
	return ajTrue;
    }

    thys = *Pstr;
    if(thys->Use != 1)
    {
      ajStrGetuniqueStr(Pstr);
      thys = *Pstr;
      ret = ajTrue;
    }

    thys->Len = 0;

    thys->Ptr[0] = '\0';

    return ret;
}

/* @func ajStrAssignEmptyC ****************************************************
**
** Ensures a string is set (has a value). If the string is set it is left 
** alone, otherwise it is initialised with a text string.
**
** @param [w] Pstr [AjPStr*] Target string which is overwritten.
** @param [r] txt [const char*] Source text.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrAssignEmptyC(AjPStr* Pstr, const char* txt)
{
    AjBool ret = ajFalse;

    if(!*Pstr)
	ret = ajStrAssignC(Pstr, txt);
    else if(!(*Pstr)->Len)
	ret = ajStrAssignC(Pstr, txt);

    return ret;
}



/* @obsolete ajStrSetC
** @rename ajStrAssignEmptyC
*/

__deprecated AjBool  ajStrSetC(AjPStr* pthis, const char* str)
{
    return ajStrAssignEmptyC(pthis, str);
}

/* @func ajStrAssignEmptyS ****************************************************
**
** Ensures a string is set (has a value). If the string is set it is left 
** alone, otherwise it is initialised with a string.
**
** @param [w] Pstr [AjPStr*] Target string which is overwritten.
** @param [r] str [const AjPStr] Source string object.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrAssignEmptyS(AjPStr* Pstr, const AjPStr str)
{
    AjBool ret = ajFalse;		/* true if ajStrDup is used */

    if(!*Pstr)
	ret = ajStrAssignS(Pstr, str);
    else if(!(*Pstr)->Len)
	ret = ajStrAssignS(Pstr, str);

    return ret;
}




/* @obsolete ajStrSet
** @rename ajStrAssignEmptyS
*/

__deprecated AjBool  ajStrSet(AjPStr* pthis, const AjPStr str)
{
    AjBool ret = ajFalse;
    ajDebug("ajStrSet pthis:%x '%S' str:%x '%S'\n",
	    *pthis, *pthis, str, str);
    ret = ajStrAssignEmptyS(pthis, str);
    ajDebug("ajStrSet ret:%B pthis:%x '%S' str:%x '%S'\n",
	    ret, *pthis, *pthis, str, str);
    return ret;
}

/* @func ajStrAssignLenC ******************************************************
**
** Copy a text string of a given length to a string.
**
** @param [w] Pstr [AjPStr*] Target string.
** @param [rN] txt [const char*] Source text.
** @param [r] len [ajuint] Length of source text.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrAssignLenC(AjPStr* Pstr, const char* txt, ajuint  len)
{
    AjBool ret = ajFalse;
    AjPStr thys;

    if (!txt)
	ajFatal("ajStrAssignLenC source text NULL");

    thys = *Pstr;
    if(!thys)
    {
	ret = ajStrSetResRound(Pstr, len+1);
	thys = *Pstr;
    }
    else if(thys->Res < len+1)
    {
	ret = ajStrSetResRound(Pstr, len+1);
	thys = *Pstr;
    }

    thys->Len = len;
    if (len)
	memmove(thys->Ptr, txt, len);

    thys->Ptr[len] = '\0';

    return ret;
}


/* @obsolete ajStrAssCI
** @rename ajStrAssignLenC
*/

__deprecated AjBool  ajStrAssCI(AjPStr* pthis, const char* txt, size_t ilen)
{
    return ajStrAssignLenC(pthis, txt, ilen);
}

/* @func ajStrAssignRef *******************************************************
**
** String referencing function which makes a string point to another.  
** Increases the reference count of the string pointed to.
**
** Not a true string copy, but if modified the new string will behave
** as a copy.  
** 
** Sets the destination string to NULL if the source string is NULL.
**
** @param [w] Pstr [AjPStr*] Target string which is overwritten.
** @param [u] refstr [AjPStr] Source string object
**        Copy by reference count so not const.
** @return [AjBool] ajTrue if string was reallocated
** @cre If both arguments point to the same string object, nothing happens.
** @@
******************************************************************************/

AjBool ajStrAssignRef(AjPStr* Pstr, AjPStr refstr)
{
    AjBool ret = ajTrue;		/* true if ajStrDup is used */

    if(*Pstr)
    {
      if((*Pstr)->Use <= 1)
	ajStrDel(Pstr);
      else
	(*Pstr)->Use--;
    }

    if(!refstr)
	ajFatal("ajStrAssignRef source string NULL");

    *Pstr = ajStrNewRef(refstr);

    return ret;
}

/* @obsolete ajStrCopy
** @rename ajStrAssignRef
*/

__deprecated AjBool  ajStrCopy(AjPStr* pthis, AjPStr str)
{
    return ajStrAssignRef(pthis, str);
}

/* @func ajStrAssignResC ******************************************************
**
** Copy a text string to a string with a minimum reserved size.
**
** @param [w] Pstr [AjPStr*] Target string.
** @param [r] size [ajuint] Space to reserve.
** @param [r] txt [const char*] Source text.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrAssignResC(AjPStr* Pstr, ajuint size, const char* txt)
{
    AjBool ret = ajFalse;
    AjPStr thys;
    ajuint ilen;
    ajuint isize;

    if (txt)
	ilen = strlen(txt);
    else
	ilen = 0;

    isize = size;

    if(ilen >= isize)
	isize = ilen + 1;

    ret = ajStrSetResRound(Pstr, isize);
    thys = *Pstr;

    thys->Len = ilen;

    if(ilen)
	memmove(thys->Ptr, txt, ilen);

    thys->Ptr[ilen] = '\0';

    return ret;
}

/* @obsolete ajStrAssCL
** @replace ajStrAssignResC (1,2,3/1,3,2)
*/

__deprecated AjBool  ajStrAssCL(AjPStr* pthis, const char* txt, size_t i)
{
    return ajStrAssignResC(pthis, i, txt);
}




/* @func ajStrAssignResS ******************************************************
**
** Copy a string to a string with a minimum reserved size.
**
** @param [w] Pstr [AjPStr*] Target string.
** @param [r] size [ajuint] Size of new string.
** @param [rN] str [const AjPStr] Source text.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrAssignResS(AjPStr* Pstr, ajuint size, const AjPStr str)
{
    AjBool ret = ajFalse;
    AjPStr thys;
    ajuint isize;

    isize = size;

    if (str)
    {
	if(isize <= str->Len)
	    isize = str->Len+1;
    }

    ret = ajStrSetResRound(Pstr, isize);
    thys = *Pstr;

    if (str)
    {
	thys->Len = str->Len;
	memmove(thys->Ptr, str->Ptr, str->Len);
	thys->Ptr[str->Len] = '\0';
    }
    else
    {
	thys->Len = 0;
	thys->Ptr[0] = '\0';
    }

    return ret;
}

/* @obsolete ajStrAssL
** @replace ajStrAssignResS (1,2,3/1,3,2)
*/

__deprecated AjBool  ajStrAssL(AjPStr* pthis, const AjPStr str, size_t i)
{
    return ajStrAssignResS(pthis, i, str);
}

/* @func ajStrAssignSubC ******************************************************
**
** Copies a substring of a text string to a string.
**
** @param [w] Pstr [AjPStr*] Target string
** @param [r] txt [const char*] Source text
** @param [r] pos1 [ajint] start position for substring
** @param [r] pos2 [ajint] end position for substring
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrAssignSubC(AjPStr* Pstr, const char* txt, ajint pos1, ajint pos2)
{
    AjBool ret = ajFalse;
    ajuint ilen;
    ajuint ibegin;
    ajuint iend;
    AjPStr thys;

    ibegin = pos1;
    iend   = pos2;

    if(pos1 < 0)
	ibegin = strlen(txt) + pos1;
    if(pos2 < 0)
	iend = strlen(txt) + pos2;

    ilen = iend - ibegin + 1;

    if(!*Pstr)
	ret = ajStrSetResRound(Pstr, ilen+1);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);

    if((*Pstr)->Res < ilen+1)
	ret = ajStrSetResRound(Pstr, ilen+1);

    thys = *Pstr;

    thys->Len = ilen;
    if (ilen)
	memmove(thys->Ptr, &txt[ibegin], ilen);

    thys->Ptr[ilen] = '\0';
    return ret;
}


/* @obsolete ajStrAssSubC
** @rename ajStrAssignSubC
*/

__deprecated AjBool  ajStrAssSubC(AjPStr* pthis, const char* txt,
				 ajint begin, ajint end)
{
    return ajStrAssignSubC(pthis, txt, begin, end);
}

/* @func ajStrAssignSubS ******************************************************
**
** Copies a substring of a string to a string.
**
** The substring is defined from character positions beginpos to endpos.
**
** ajTrue is returned if target was
** (re)allocated, ajFalse is returned otherwise. 
**
** @short Copies a substring of a string to a string.
**
** @param [w] Pstr [AjPStr*] Target string
**          {memory will be automatically allocated if required}
** @param [r] str [const AjPStr] Source string
** @param [r] pos1 [ajint] Start position in src of substring
**              {negative values count from the end of the string
**               with -1 as the last position}
** @param [r] pos2 [ajint] End position in src of substring
**              {negative values count from the end of the string
**               with -1 as the last position}
** @return [AjBool] ajTrue if pthis was (re)allocated, ajFalse otherwise
** @release 1.0.0 
** @@
******************************************************************************/

AjBool ajStrAssignSubS(AjPStr* Pstr, const AjPStr str,
		      ajint pos1, ajint pos2)
{
    AjBool ret = ajFalse;
    ajuint ilen;
    ajuint ibegin;
    ajuint iend;
    AjPStr thys;

    ibegin = ajMathPos(str->Len, pos1);
    iend = ajMathPosI(str->Len, ibegin, pos2);
    if(iend == str->Len)
	iend--;

    ilen = iend - ibegin + 1;

    if(!*Pstr)
	ret = ajStrSetResRound(Pstr, ilen+1);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);

    if((*Pstr)->Res < ilen+1)
	ret = ajStrSetResRound(Pstr, ilen+1);

    thys = *Pstr;

    thys->Len = ilen;
    if (ilen)
	memmove(thys->Ptr, &str->Ptr[ibegin], ilen);

    thys->Ptr[ilen] = '\0';
    return ret;
}


/* @obsolete ajStrAssSub
** @rename ajStrAssignSubS
*/
__deprecated AjBool  ajStrAssSub(AjPStr* pthis, const AjPStr src,
		   ajint beginpos, ajint endpos)
{
    return ajStrAssignSubS(pthis, src, beginpos, endpos);
}


/* @section combination functions *********************************************
**
** Functions for appending, inserting or overwriting a string
** or substring to another.
**
** @fdata      [AjPStr]
** @fnote     Memory for the object is allocated if necessary
**
** @nam3rule  Append         Append one string to another.
** @nam4rule  AppendSub    Append substring.
** @nam4rule  AppendCount    Append number of characters.
** @nam3rule  Join        Append one string to another after cutting strings. 
** @nam3rule  Insert      Insert one string into another.
** @nam3rule  Paste       Overwrite one string with another. 
** @nam4rule  PasteCount  Overwrite with a number of single characters
** @nam4rule  PasteMax    Length of overwrite string specified
** @nam3rule  Mask        Mask a range of characters. 
**
** @argrule   *        Pstr [AjPStr*] Modifiable string
** @argrule   Insert  pos [ajint] Position in string to start inserting,
**                                negative values count from the end
** @argrule   Paste   pos [ajint] Position in string to start inserting,
**                                negative values count from the end
** @argrule   Join   pos [ajint] Position in string to start inserting,
**                                negative values count from the end
** @argrule   C       txt [const char*] Text string
** @argrule   K       chr [char] Single character
** @argrule   S       str [const AjPStr] Text string
** @argrule   Count     num [ajuint] Number of single characters to copy
** @argrule   Len     len [ajuint] Number of characters to copy from string
** @argrule   Max     len [ajuint] Length of string
** @argrule   Sub     pos1 [ajint] Start position, negative value counts
**                                 from end
** @argrule   Mask     pos1 [ajint] Start position, negative value counts
**                                 from end
** @argrule   Sub     pos2 [ajint] End position, negative value counts from end
** @argrule   Mask    pos2 [ajint] End position, negative value counts from end
** @argrule   Join    posb [ajint] Position in source string
**                                negative values count from the end
** @argrule   Mask    maskchr [char] Masking character
** 
** @valrule   * [AjBool] 
**
** @fcategory modify
** @@
*/



/* @func ajStrAppendC *********************************************************
**
** Appends a text string to the end of a string.
** 
** Uses {ajStrSetRes} to make sure target string is modifiable.
**
** @param [w] Pstr [AjPStr*] Target string
** @param [r] txt [const char*] Source text
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrAppendC(AjPStr* Pstr, const char* txt)
{
    ajuint i;

    i = strlen(txt);

    return ajStrAppendLenC(Pstr, txt, i);
}

/* @obsolete ajStrAppC
** @rename ajStrAppendC
*/

__deprecated AjBool  ajStrAppC(AjPStr* pthis, const char* txt)
{
    return ajStrAppendC(pthis, txt);
}

/* @func ajStrAppendK *********************************************************
**
** Appends a character to the end of a string.
**
** Uses {ajStrSetRes} to make sure target string is modifiable.
**
** @param [w] Pstr [AjPStr*] Target string
** @param [rN] chr [char] Source character
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrAppendK(AjPStr* Pstr, char chr)
{
    AjBool ret = ajFalse;
    AjPStr thys;
    ajuint j;

    thys = *Pstr;

    if(thys)
	j = AJMAX(thys->Res, thys->Len+2);
    else
	j = 2;

    ret = ajStrSetResRound(Pstr, j);
    thys = *Pstr;			/* possible new location */

    thys->Ptr[thys->Len] = chr;
    thys->Ptr[++thys->Len] = '\0';

    return ret;
}

/* @obsolete ajStrAppK
** @rename ajStrAppendK
*/

__deprecated AjBool  ajStrAppK(AjPStr* pthis, const char chr)
{
    return ajStrAppendK(pthis, chr);
}

/* @func ajStrAppendS *********************************************************
**
** Appends a string to the end of another string.
** 
** Uses {ajStrSetRes} to make sure target string is modifiable.
**
** @param [w] Pstr [AjPStr*] Target string
** @param [r] str [const AjPStr] Source string
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrAppendS(AjPStr* Pstr, const AjPStr str)
{
    AjBool ret = ajFalse;

    AjPStr thys;
    ajuint j;

    if(!str)
        return ajFalse;

    thys = *Pstr;

    if(thys)
    {
	j = AJMAX(thys->Res, thys->Len+str->Len+1);
    }
    else
	j = str->Len+1;

    ret = ajStrSetResRound(Pstr, j);
    thys = *Pstr;			/* possible new location */

    memmove(thys->Ptr+thys->Len, str->Ptr, str->Len+1);	/* include the null */
    thys->Len += str->Len;

    return ret;
}


/* @obsolete ajStrApp
** @rename ajStrAppendS
*/

__deprecated AjBool  ajStrApp(AjPStr* pthis, const AjPStr src)
{
    return ajStrAppendS(pthis, src);
}

/* @func ajStrAppendCountK ****************************************************
**
** Appends any number of a given character to the end of a string.
** 
** Equivalent to a repeat count for ajStrAppK.
**
** Uses {ajStrSetRes} to make sure target string is modifiable.
**
** @param [w] Pstr [AjPStr*] Target string
** @param [r] chr [char] Source character
** @param [r] num [ajuint] Repeat count
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrAppendCountK(AjPStr* Pstr, char chr, ajuint num)
{
    AjBool ret = ajFalse;
    AjPStr thys;
    ajuint i;
    ajuint j;
    char* cp;

    thys = *Pstr;

    if(thys)
	j = AJMAX(thys->Res, (thys->Len+num+1));
    else
	j = num+1;

    ret = ajStrSetResRound(Pstr, j);
    thys = *Pstr;			/* possible new location */

    cp = &thys->Ptr[thys->Len];
    for(i=0; i<num; i++)
    {
	*cp = chr;
	cp++;
    }

    *cp = '\0';
    thys->Len += num;

    return ret;
}

/* @obsolete ajStrAppKI
** @rename ajStrAppendCountK
*/

__deprecated AjBool  ajStrAppKI(AjPStr* pthis, const char chr, ajint number)
{
    return ajStrAppendCountK(pthis, chr, number);
}
/* @func ajStrAppendLenC ******************************************************
**
** Appends a text string of a given length to the end of a string.
** 
** Uses {ajStrSetRes} to make sure target string is modifiable.
**
** @param [w] Pstr [AjPStr*] Target string
** @param [rN] txt [const char*] Source text
** @param [r] len [ajuint] String length
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrAppendLenC(AjPStr* Pstr, const char* txt, ajuint len)
{
    AjBool ret = ajFalse;

    AjPStr thys;
    ajuint j;

    thys = *Pstr;

    if(!txt)
	return ajFalse;

    if(*Pstr)
	j = AJMAX(thys->Res, (thys->Len+len+1));
    else
	j = len+1;

    ret = ajStrSetResRound(Pstr, j);
    thys = *Pstr;			/* possible new location */

    memmove(thys->Ptr+thys->Len, txt, len+1);
    thys->Len += len;

    thys->Ptr[thys->Len] = '\0';

    return ret;
}


/* @obsolete ajStrAppCI
** @rename ajStrAppendLenC
*/

__deprecated AjBool  ajStrAppCI(AjPStr* pthis, const char* txt, size_t i)
{
    return ajStrAppendLenC(pthis, txt, i);
}

/* @func ajStrAppendSubC ******************************************************
**
** Appends a substring of a string to the end of another string.
** 
** @param [w] Pstr [AjPStr*] Target string
** @param [r] txt [const char*] Source string
** @param [r] pos1 [ajint] start position for substring
** @param [r] pos2 [ajint] end position for substring
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrAppendSubC(AjPStr* Pstr, const char* txt, ajint pos1, ajint pos2)
{
    ajuint ilen;
    ajuint jlen;
    ajuint ibegin;
    ajuint iend;
    AjBool ret = ajFalse;

    AjPStr thys;
    ajuint j;

    thys = *Pstr;

    jlen = strlen(txt);
    ibegin = ajMathPos(jlen, pos1);
    iend   = ajMathPosI(jlen, ibegin, pos2);

    ilen = iend - ibegin + 1;

    if(thys)
    {
	j = AJMAX(thys->Res, thys->Len+ilen+1);
    }
    else
	j = ilen+1;

    ret = ajStrSetResRound(Pstr, j);
    thys = *Pstr;			/* possible new location */

    memmove(thys->Ptr+thys->Len, &txt[ibegin], ilen);
    thys->Len += ilen;

    thys->Ptr[thys->Len] = '\0';

    return ret;
}

/* @func ajStrAppendSubS ******************************************************
**
** Appends a substring of a string to the end of another string.
** 
** Uses {ajStrSetRes} to make sure target string is modifiable.
**
** @param [w] Pstr [AjPStr*] Target string
** @param [r] str [const AjPStr] Source string
** @param [r] pos1 [ajint] start position for substring
** @param [r] pos2 [ajint] end position for substring
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrAppendSubS(AjPStr* Pstr, const AjPStr str, ajint pos1, ajint pos2)
{
    ajuint ilen;
    ajuint ibegin;
    ajuint iend;
    AjBool ret = ajFalse;

    AjPStr thys;
    ajuint j;

    thys = *Pstr;

    ibegin = ajMathPos(str->Len, pos1);
    iend   = ajMathPosI(str->Len, ibegin, pos2);

    ilen = iend - ibegin + 1;

    if(thys)
    {
	j = AJMAX(thys->Res, thys->Len+ilen+1);
    }
    else
	j = ilen+1;

    ret = ajStrSetResRound(Pstr, j);
    thys = *Pstr;			/* possible new location */

    memmove(thys->Ptr+thys->Len, &str->Ptr[ibegin], ilen);
    thys->Len += ilen;

    thys->Ptr[thys->Len] = '\0';

    return ret;
}

/* @obsolete ajStrAppSub
** @rename ajStrAppendSubS
*/

__deprecated AjBool  ajStrAppSub(AjPStr* pthis, const AjPStr src,
			       ajint begin, ajint end)
{
    return ajStrAppendSubS(pthis, src, begin, end);
}

/* @func ajStrInsertC *********************************************************
**
** Insert a text string into a string at a specified postion.
**
** @param [u] Pstr [AjPStr*] Target string
** @param [r] pos [ajint] Position where text is to be inserted.
**                        Negative position counts from the end
** @param [r] txt [const char*] Text to be inserted
** @return [AjBool]  ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrInsertC(AjPStr* Pstr, ajint pos, const char* txt )
{
    AjBool ret = ajFalse;
    AjPStr thys;
    ajuint j = 0;
    ajuint y = 0;
    ajuint ibegin;
    char* ptr1;
    const char* ptr2;
    ajuint len;

    len = strlen(txt);

    thys = *Pstr;
    if(!thys)
    {
	ajStrAssignResC(Pstr, len+1, "");
	thys = *Pstr;
    }

    /* can be at start or after end */
    ibegin = ajMathPosI(thys->Len+1, 0, pos);

    j = thys->Len+len+1;

    if(j > thys->Res)
    {
	ret = ajStrSetResRound(Pstr, j);
    }
    else
    {
        if(thys->Use > 1)
	  ajStrGetuniqueStr(Pstr);
	ret = ajTrue;
    }

    thys = *Pstr;			/* possible new location */

    /* move characters "i" places up to leave place for insertion */
    ptr1 =  &thys->Ptr[thys->Len+len];
    ptr2 =  &thys->Ptr[thys->Len];
    for(y=0; y<=thys->Len-ibegin ; y++)
    {
	*ptr1 = *ptr2;
	ptr1--;
	ptr2--;
    }

    thys->Len += len;			/* set the new length */
    thys->Ptr[thys->Len] = '\0';	/* ### was Len+1 ### add the
					   end character */

    /* add the new text */
    ptr1 = & thys->Ptr[ibegin];
    ptr2 = txt;

    for(y=0; y< len; y++)
    {
	*ptr1 = *ptr2;
	ptr1++;
	ptr2++;
    }

    return ret;
}




/* @func ajStrInsertK *********************************************************
**
** Inserts a character into a string at a specified postion.
**
** @param [u] Pstr [AjPStr*] Target string
** @param [r] pos [ajint] Position where text is to be inserted.
**                        Negative position counts from the end
** @param [r] chr [char] Text to be inserted
** @return [AjBool]  ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrInsertK(AjPStr* Pstr, ajint pos, char chr )
{
    char tmpstr[2] = "?";
    *tmpstr = chr;
    return ajStrInsertC(Pstr, pos, tmpstr);
}

/* @func ajStrInsertS *********************************************************
**
** Inserts a text string into a string at a specified postion.
**
** @param [u] Pstr [AjPStr*] Target string
** @param [r] pos [ajint] Position where text is to be inserted.
**                        Negative position counts from the end
** @param [r] str [const AjPStr] String to be inserted
** @return [AjBool] ajTrue on successful completion else ajFalse;
** @error ajFalse if the insert failed. Currently this happens if
**        pos is negative, but this could be reassigned to a position
**        from the end of the string in future.
** @@
******************************************************************************/
AjBool ajStrInsertS(AjPStr* Pstr, ajint pos, const AjPStr str )
{
    return ajStrInsertC(Pstr, pos, str->Ptr);
}

/* @obsolete ajStrInsert
** @rename ajStrInsertS
*/

__deprecated AjBool  ajStrInsert(AjPStr* Pstr, ajint pos, const AjPStr str )
{
    return ajStrInsertS(Pstr, pos, str);
}


/* @func ajStrJoinC ***********************************************************
**
** Cut down string at pos1 and add string2 from position pos2.
**
** @param [u] Pstr [AjPStr*] Target string.
** @param [r] pos [ajint] Number of characters to keep in target string.
** @param [r] txt [const char*] Text to append.
** @param [r] posb [ajint] Position of first character to copy from text.
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajStrJoinC(AjPStr* Pstr, ajint pos, const char* txt,
		   ajint posb)
{
    AjPStr thys;
    ajuint len;
    ajuint ibegin;
    ajuint ibegin2;
    ajuint i = 0;
    ajuint j = 0;
    ajuint newlen = 0;

    len = strlen(txt);

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);
    thys = *Pstr;

    ibegin = ajMathPos(thys->Len, pos);
    ibegin2 = ajMathPos(len, posb);

    if(thys->Len < ibegin || len < ibegin2)
	return ajFalse;

    newlen = ibegin + len - ibegin2 + 1;

    if(newlen > thys->Res)
    {
	ajStrSetResRound(Pstr, newlen);
	thys = *Pstr;
    }

    for(i=ibegin,j=ibegin2; j <= len; i++,j++)
	thys->Ptr[i] = txt[j];

    thys->Len = i-1;

    return ajTrue;
}




/* @func ajStrJoinS ***********************************************************
**
** Appends one string to another after cutting both strings.
** 
** Cuts down string at pos1 and add string2 from position pos2.
**
** @param [u] Pstr [AjPStr*] Target string.
** @param [r] pos [ajint] Start position in target string,
**                         negative numbers count from the end.
** @param [r] str [const AjPStr] String to append.
** @param [r] posb [ajint] Starts position to copy,
**                         negative numbers count from the end.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajStrJoinS(AjPStr* Pstr, ajint pos, const AjPStr str,
		  ajint posb)
{
    ajuint ibegin1;
    ajuint ibegin2;

    ibegin1 = ajMathPos((*Pstr)->Len, pos);
    ibegin2 = ajMathPos(str->Len, posb);

    return ajStrJoinC(Pstr, ibegin1, str->Ptr, ibegin2);
}



/* @func ajStrMask ************************************************************
**
** Masks out characters from a string over a specified range.
**
** @param [w] Pstr [AjPStr*] Target string
** @param [r] pos1 [ajint] start position to be masked
** @param [r] pos2 [ajint] end position to be masked
** @param [r] maskchr [char] masking character
** @return [AjBool] ajTrue on success, ajFalse if begin is out of range
** @@
******************************************************************************/

AjBool ajStrMask(AjPStr* Pstr, ajint pos1, ajint pos2, char maskchr)
{
    AjPStr thys;
    ajuint ibegin;
    ajuint iend;
    ajuint i;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);
    thys = *Pstr;

    ibegin = ajMathPos(thys->Len, pos1);
    iend = ajMathPosI(thys->Len, ibegin, pos2);

/*
    ajDebug("ajStrMask %d %d len: %d ibegin: %d iend: %d char '%c'\n",
	     begin, end, thys->Len, ibegin, iend, maskchar);
*/

    if(iend < ibegin)
	return ajFalse;

    for(i=ibegin; i<=iend; i++)
	thys->Ptr[i] = maskchr;

    return ajTrue;
}




/* @func ajStrPasteS **********************************************************
**
** Overwrite one string with another. 
** 
** Replace string at pos1 with new string.
**
** @param [u] Pstr [AjPStr*] Target string
** @param [r] pos [ajint] Number of characters of target string to keep.
** @param [r] str [const AjPStr] String to replace.
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajStrPasteS( AjPStr* Pstr, ajint pos, const AjPStr str)
{
    return ajStrPasteMaxC(Pstr, pos, str->Ptr, str->Len);
}


/* @obsolete ajStrReplaceS
** @rename ajStrPasteS
*/

__deprecated AjBool  ajStrReplaceS( AjPStr* pthis, ajint begin,
				  const AjPStr overwrite)
{
    return ajStrPasteS(pthis, begin, overwrite);
}


/* @func ajStrPasteCountK *****************************************************
**
** Overwrites a string with a number of single characters.
** 
** Replace string at pos and add num copies of character chr.  Or to the end 
** of the existing string
**
** @param [u] Pstr [AjPStr*] Target string
** @param [r] pos [ajint] Number of characters of target string to keep.
** @param [r] chr [char] Character to replace.
** @param [r] num [ajuint] Number of characters to copy from text.
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajStrPasteCountK( AjPStr* Pstr, ajint pos, char chr,
		      ajuint num)
{
    AjPStr thys;
    ajuint ibegin;
    ajuint iend;
    char* ptr1 = 0;
    ajuint i;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);
    thys = *Pstr;

    ibegin = ajMathPos(thys->Len, pos);
    iend   = ibegin + num;

    if(iend > thys->Len)		/* can't fit */
	iend = thys->Len - 1;

    ptr1 = &thys->Ptr[ibegin];

    for(i=ibegin;i<iend;i++)
	*ptr1++ = chr;

    return ajTrue;
}


/* @obsolete ajStrReplaceK
** @rename ajStrPasteCountK
*/

__deprecated AjBool  ajStrReplaceK( AjPStr* pthis, ajint ibegin,
		      char overwrite, ajint ilen)
{
    return ajStrPasteCountK(pthis, ibegin, overwrite, ilen);
}


/* @func ajStrPasteMaxC *******************************************************
**
** Overwrite one string with a specifed number of characters from a text 
** string.
** 
** Replaces string at pos ands add len characters from text string txt.
** Or to the end of the existing string
**
** @param [u] Pstr [AjPStr*] Target string
** @param [r] pos [ajint] Start position in target string.
** @param [r] txt [const char*] String to replace.
** @param [r] len [ajuint] Number of characters to copy from text.
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajStrPasteMaxC (AjPStr* Pstr, ajint pos, const char* txt,
		       ajuint len)
{
    AjPStr thys;
    ajuint ibegin;
    ajuint iend;
    char* ptr1       = 0;
    const char* ptr2 = 0;
    ajuint slen;
    ajuint ilen;

    slen = strlen(txt);

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);
    thys = *Pstr;

    ibegin = ajMathPos(thys->Len, pos);
    iend = ibegin + len;

    if((iend  > thys->Len) || (len > slen) ) /* can't fit */
	return ajFalse;

    ptr1 = &thys->Ptr[ibegin];
    ptr2 = txt;

    for(ilen=len;ilen>0;ilen--)
	*ptr1++ = *ptr2++;

    return ajTrue;
}

/* @obsolete ajStrReplaceC
** @rename ajStrPasteMaxC
*/

__deprecated AjBool  ajStrReplaceC( AjPStr* pthis, ajint begin,
				   const char* overwrite, ajint ilen)
{
    return ajStrPasteMaxC(pthis, begin, overwrite, ilen);
}

/* @func ajStrPasteMaxS *******************************************************
**
** Overwrite one string with a specifed number of characters from another 
** string. 

** Replaces string at pos and add len characters from string str.
** Or to the end of the existing string
**
** @param [u] Pstr [AjPStr*] Target string
** @param [r] pos [ajint] Start position in tagret string.
** @param [r] str [const AjPStr] Replacement string
** @param [r] len [ajuint] Number of characters to copy from text.
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajStrPasteMaxS(AjPStr* Pstr, ajint pos, const AjPStr str,
		    ajuint len)
{
    return ajStrPasteMaxC(Pstr, pos, str->Ptr, len);
}


/* @obsolete ajStrReplace
** @rename ajStrPasteMaxS
*/
__deprecated AjBool  ajStrReplace( AjPStr* pthis, ajint begin,
				  const AjPStr overwrite, ajint ilen)
{
    return ajStrPasteMaxS(pthis, begin, overwrite, ilen);
}


/* @section cutting ***********************************************************
**
** Functions for removing characters or regions (substrings) from a string.
**
** @fdata      [AjPStr]
** @fnote     None.
** @nam3rule  Cut                Remove region(s) from a string. 
** @nam4rule  CutComments      Remove comment lines.
** @nam5rule  CutCommentsStart Remove comment lines beginning with '#' only.
** @nam5rule  CutCommentsRestpos Remove comment lines and note start position
** @nam4rule  CutEnd           Remove one end or another.
** @nam4rule  CutRange         Remove a substring.
** @nam4rule  CutStart         Remove one end or another.
** @nam4rule  CutWild          Remove end from first wildcard character.
** @nam4rule  CutHtml          Remove HTML (i.e. everything between
**                             '<' and '>').
** @nam3rule  Keep             Keep part of a string.
** @nam4rule  KeepRange        Keep range of character positions.
** @nam4rule  KeepSet          Keep only characters in a set.
** @nam5rule  KeepSetAlpha     Also remove non-alphabetic.
** @nam6rule  KeepSetAlphaRest  Also remove non-alphabetic and report non-space
** @nam3rule  Quote            Editing quotes in qtrings
** @nam4rule  QuoteStrip       Removing quotes
** @nam5rule  QuoteStripAll    Removing internal and external quotes
** @nam3rule  Remove           Remove individual characters from a string.
** @nam4rule  RemoveGap        Remove non-sequence characters.
** @nam4rule  RemoveHtml       Remove HTML tags.
** @nam4rule  RemoveLast       Remove last character.
** @nam5rule  RemoveLastNewline   Remove last character if a newline only.
** @nam4rule  RemoveNonseq     Remove non-sequence characters
**                               (all chars except alphabetic & '*')
** @nam4rule  RemoveSet        Remove a set of characters.
** @nam4rule  RemoveWhite      Remove all whitespace characters.
** @nam5rule  RemoveWhiteExcess  Remove excess whitespace only.
** @nam5rule  RemoveWhiteSpaces  Remove excess space characters only.
** @nam4rule  RemoveWild         Remove characters after a wildcard.
** @nam3rule  Trim               Remove region(s) of a given character
**                               composition only from start and / or end
**                               of a string.
** @nam4rule  TrimEnd          Trim from end only.
** @nam4rule  TrimStart        Trim from start only.
** @nam3rule  Truncate         Intuitively named end-cut function. 
** @nam4rule  TruncateLen      Truncate to a specified length
** @nam4rule  TruncatePos      Truncate after a specified position
** @nam4rule  TrimWhite        Trim whitespace only.
** @nam5rule  TrimWhiteEnd     Trim whitespace from end.
** @nam5rule  TrimWhiteStart   Trim whitespace from start.
**
** @argrule   *        Pstr  [AjPStr*] Modifiable string
** @argrule   Pos      pos   [ajint]   First position to be deleted.
                                       Negative values count from the end
** @argrule   Restpos  Pcomment  [AjPStr*]  Removed (comment) characters
** @argrule   Restpos  Pstartpos [ajuint*]  Position at start of comment
** @argrule   C        txt   [const char*]  Text string
** @argrule   K        chr   [char]         Single character
** @argrule   S        str   [const AjPStr] Text string
** @argrule   Len      len   [ajuint] Number of characters to copy
** @argrule   CutEnd   len   [ajuint] Number of characters to copy
** @argrule   CutStart len   [ajuint] Number of characters to copy
** @argrule   Range    pos1  [ajint]  Start position in string, negative
**                                    numbers count from end
** @argrule   Range    pos2  [ajint]  End position in string, negative
**                                    numbers count from end
** @argrule   Rest     Prest [AjPStr*] Excluded characters
**
** @valrule   * [AjBool]
**
** @fcategory modify
*/



/* @func ajStrCutComments *****************************************************
**
** Removes comments from a string.
** 
** A comment begins with a "#" character and may appear anywhere in the string.
** See ajStrCutCommentsStart for alternative definition of a comment.
**
** @param [u] Pstr [AjPStr*] Line of text from input file
** @return [AjBool] ajTrue if there is some text remaining
** @@
******************************************************************************/

AjBool ajStrCutComments(AjPStr* Pstr)
{
    AjPStr thys;
    char *cp;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);
    thys = *Pstr;

    if(!thys->Len)		/* empty string */
	return ajFalse;

    cp = strchr(thys->Ptr, '#');
    if(cp)
    {				/* comment found and removed */
	*cp = '\0';
	thys->Len = cp - thys->Ptr;
    }

    if(!thys->Len)	      /* no text before the comment */
	return ajFalse;

    return ajTrue;
}

/* @obsolete ajStrUncomment
** @rename ajStrCutComments
*/

__deprecated AjBool  ajStrUncomment(AjPStr* text)
{
    return ajStrCutComments(text);
}

/* @func ajStrCutCommentsRestpos **********************************************
**
** Removes comments from a string.
** 
** A comment begins with a "#" character and may appear anywhere in the string.
** See ajStrCutCommentsStart for alternative definition of a comment.
**
** @param [u] Pstr [AjPStr*] Line of text from input file
** @param [w] Pcomment [AjPStr*] Comment characters deleted
** @param [w] Pstartpos [ajuint*] Comment start position
** @return [AjBool] ajTrue if there is some text remaining
** @@
******************************************************************************/

AjBool ajStrCutCommentsRestpos(AjPStr* Pstr,
			       AjPStr* Pcomment, ajuint* Pstartpos)
{
    AjPStr thys;
    char *cp;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);
    thys = *Pstr;

    *Pstartpos=0;
    ajStrAssignClear(Pcomment);

    if(!thys->Len)		/* empty string */
	return ajFalse;

    cp = strchr(thys->Ptr, '#');
    if(cp)
    {
	/* comment found and removed */
	*Pstartpos = cp -thys->Ptr;
	ajStrAssignC(Pcomment, cp);
	*cp = '\0';
	thys->Len = cp - thys->Ptr;
    }

    if(!thys->Len)	      /* no text before the comment */
	return ajFalse;

    return ajTrue;
}

/* @func ajStrCutCommentsStart ************************************************
**
** Removes comments from a string.
** 
** A comment is a blank line or any text starting with a "#" character.
**
** @param [u] Pstr [AjPStr*] Line of text from input file
** @return [AjBool] ajTrue if there is some text remaining
** @@
******************************************************************************/

AjBool ajStrCutCommentsStart(AjPStr* Pstr)
{
    AjPStr thys;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);
    thys = *Pstr;

    if(!MAJSTRGETLEN(thys))		/* empty string */
	return ajFalse;

    if(thys->Ptr[0] == '#')
    {					/* comment found */
	ajStrAssignClear(Pstr);
	return ajFalse;
    }

    return ajTrue;
}


/* @obsolete ajStrUncommentStart
** @rename ajStrCutCommentsStart
*/

__deprecated AjBool  ajStrUncommentStart(AjPStr* text)
{
    return ajStrCutCommentsStart(text);
}

/* @func ajStrCutEnd **********************************************************
**
** Removes a number of characters from the end of a string
**
** @param [u] Pstr [AjPStr*] string
** @param [r] len [ajuint] Number of characters to delete from the end
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrCutEnd(AjPStr* Pstr, ajuint len)
{
    AjPStr thys;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);
    thys = *Pstr;

    if(!len)
	return ajTrue;

    if(len > thys->Len)
	thys->Len = 0;
    else
	thys->Len -= len;

    thys->Ptr[thys->Len] = '\0';

    return ajTrue;
}


/* @obsolete ajStrTrim
** @replace ajStrCutStart (1,+/1,2)
** @replace ajStrCutEnd   (1,-/1,-2)
*/

__deprecated AjBool  ajStrTrim(AjPStr* pthis, ajint num)
{
    AjBool ret;
    if(num >= 0)
	ret = ajStrCutStart(pthis, num);
    else
	ret = ajStrCutEnd(pthis, -num);
    return ret;
}

/* @func ajStrCutRange ********************************************************
**
** Removes a substring from a string.
**
** @param [w] Pstr [AjPStr*] Target string
** @param [r] pos1 [ajint] start position to be cut
** @param [r] pos2 [ajint] end position to be cut
** @return [AjBool] ajTrue on success, ajFalse if begin is out of range
** @@
******************************************************************************/

AjBool ajStrCutRange(AjPStr* Pstr, ajint pos1, ajint pos2)
{
    AjPStr thys;
    ajuint ilen;
    ajuint ibegin;
    ajuint iend;
    ajuint irest;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);
    thys = *Pstr;

    ibegin = ajMathPos(thys->Len, pos1);
    iend = ajMathPosI(thys->Len, ibegin, pos2) + 1;
    ilen = iend - ibegin;

    /*ajDebug("ajStrCut %d %d len: %d ibegin: %d iend: %d\n",
	     begin, end, thys->Len, ibegin, iend);*/

    irest = thys->Len - iend + 1;
    if(irest > 0)
      memmove(&thys->Ptr[ibegin], &thys->Ptr[iend], irest);
    thys->Len -= ilen;
    thys->Ptr[thys->Len] = '\0';

    return ajTrue;
}



/* @obsolete ajStrCut
** @rename ajStrCutRange
*/

__deprecated AjBool  ajStrCut(AjPStr* pthis, ajint begin, ajint end)
{
    return ajStrCutRange(pthis, begin, end);
}


/* @func ajStrCutStart ********************************************************
**
** Removes a number of characters from the start of a string
**
** @param [u] Pstr [AjPStr*] string
** @param [r] len [ajuint] Number of characters to delete from the start
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrCutStart(AjPStr* Pstr, ajuint len)
{
    AjPStr thys;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);
    thys = *Pstr;

    if(!len)
	return ajTrue;

    if(len > thys->Len)
	thys->Len = 0;
    else
    {
	memmove(thys->Ptr, &thys->Ptr[len], thys->Len - len);
	thys->Len -= len;
	thys->Ptr[thys->Len] = '\0';
    }

    return ajTrue;
}


/* @func ajStrKeepRange *******************************************************
**
** Reduces target string to a substring of itself by deleting all except
** a range of character positions.
**
** The end is allowed to be before begin, in which case the output is an
** empty string.
**
** @param [w] Pstr [AjPStr*] Target string.
** @param [r] pos1 [ajint] Start position for substring.
** @param [r] pos2 [ajint] End position for substring.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrKeepRange(AjPStr* Pstr, ajint pos1, ajint pos2)
{
    AjBool ret = ajFalse;
    AjPStr str;
    ajuint ibegin;
    ajuint ilen;
    ajuint iend;

    str = *Pstr;
    if(str->Use > 1)
      str = ajStrGetuniqueStr(Pstr);

    ibegin = ajMathPos(str->Len, pos1);
    iend = ajMathPos(str->Len, pos2);

    if(iend == str->Len)
	iend--;

    if(iend < ibegin)
	ilen = 0;
    else
	ilen = iend - ibegin + 1;

    if(ilen)
    {
	if(ibegin)
	    ajMemMove(str->Ptr, &str->Ptr[ibegin], ilen);
	str->Len = ilen;
	str->Ptr[ilen] = '\0';
    }
    else
    {
	str->Len = 0;
	str->Ptr[0] = '\0';
    }

    return ret;
}


/* @obsolete ajStrSub
** @rename ajStrKeepRange
*/

__deprecated AjBool  ajStrSub(AjPStr* pthis, ajint begin, ajint end)
{
    return ajStrKeepRange(pthis, begin, end);
}

/* @func ajStrKeepSetC ***************************************************
**
** Removes all characters from a string that are not in a given set.
**
** @param [u] Pstr [AjPStr *] String to clean.
** @param [r] txt [const char*] Character set to keep
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrKeepSetC(AjPStr* Pstr, const char* txt)
{
    AjPStr thys;
    char *p;
    char *q;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);
    thys = *Pstr;

    p = thys->Ptr;
    q = thys->Ptr;

    while(*p)
    {
	if(strchr(txt, *p))
	    *q++=*p;
	p++;
    }

    *q='\0';
    thys->Len = q - thys->Ptr;

    if(!thys->Len) return ajFalse;
    return ajTrue;
}

/* @func ajStrKeepSetS ***************************************************
**
** Removes all characters from a string that are not in a given set.
**
** @param [u] Pstr [AjPStr *] String to clean.
** @param [r] str [const AjPStr] Character set to keep
** @return [AjBool] ajTrue if string is not empty
** @@
******************************************************************************/

AjBool ajStrKeepSetS(AjPStr* Pstr, const AjPStr str)
{
    AjPStr thys;
    char *p;
    char *q;
    const char* txt;

    if(!str)
    {
      if(MAJSTRGETLEN(*Pstr))
	return ajTrue;
      else
	return ajFalse;
    }

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);
    thys = *Pstr;
    
    txt = MAJSTRGETPTR(str);

    p = thys->Ptr;
    q = thys->Ptr;

    while(*p)
    {
	if(strchr(txt, *p))
	    *q++=*p;
	p++;
    }

    *q='\0';
    thys->Len = q - thys->Ptr;

    if(!thys->Len) return ajFalse;
    return ajTrue;
}

/* @obsolete ajStrKeepC
** @rename ajStrKeepSetC
*/

__deprecated AjBool  ajStrKeepC(AjPStr* s, const char* charset)
{
    return ajStrKeepSetC (s, charset);
}

/* @func ajStrKeepSetAlpha ****************************************************
**
** Removes all characters from a string that are not alphabetic.
**
** @param [u] Pstr [AjPStr *] String to clean.
** @return [AjBool] ajTrue if string is not empty
** @@
******************************************************************************/

AjBool ajStrKeepSetAlpha(AjPStr* Pstr)
{
    AjPStr thys;
    char *p;
    char *q;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);
    thys = *Pstr;

    p = thys->Ptr;
    q = thys->Ptr;

    while(*p)
    {
	if(isalpha((ajint)*p))
	    *q++=*p;
	p++;
    }

    *q='\0';
    thys->Len = q - thys->Ptr;

    if(!thys->Len) return ajFalse;
    return ajTrue;
}



/* @func ajStrKeepSetAlphaC ***************************************************
**
** Removes all characters from a string that are not alphabetic and
** are not in a given set.
**
** @param [u] Pstr [AjPStr *] String to clean.
** @param [r] txt [const char*] Non-alphabetic character set to keep
** @return [AjBool] ajTrue if string is not empty
** @@
******************************************************************************/

AjBool ajStrKeepSetAlphaC(AjPStr* Pstr, const char* txt)
{
    AjPStr thys;
    char *p;
    char *q;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);
    thys = *Pstr;

    p = thys->Ptr;
    q = thys->Ptr;

    while(*p)
    {
	if(isalpha((ajint)*p))
	    *q++=*p;
	else if(strchr(txt, *p))
	    *q++=*p;
	p++;
    }

    *q='\0';
    thys->Len = q - thys->Ptr;

    if(!thys->Len) return ajFalse;
    return ajTrue;
}



/* @obsolete ajStrKeepAlphaC
** @rename ajStrKeepSetAlphaC
*/

__deprecated AjBool  ajStrKeepAlphaC(AjPStr* s, const char* charset)
{
    return ajStrKeepSetAlphaC(s, charset);
}

/* @func ajStrKeepSetAlphaS ***************************************************
**
** Removes all characters from a string that are not alphabetic and
** are not in a given set.
**
** @param [u] Pstr [AjPStr *] String to clean.
** @param [r] str [const AjPStr] Non-alphabetic character set to keep
** @return [AjBool] ajTrue if string is not empty
** @@
******************************************************************************/

AjBool ajStrKeepSetAlphaS(AjPStr* Pstr, const AjPStr str)
{
    return ajStrKeepSetAlphaC(Pstr, str->Ptr);
}



/* @func ajStrKeepSetAlphaRest ************************************************
**
** Removes all characters from a string that are not alphabetic.
**
** Also returns any non-whitespace characters in the strings
**
** @param [u] Pstr [AjPStr *] String to clean.
** @param [u] Prest [AjPStr *] Excluded non-whitespace characters.
** @return [AjBool] ajTrue if string is not empty
** @@
******************************************************************************/

AjBool ajStrKeepSetAlphaRest(AjPStr* Pstr, AjPStr* Prest)
{
    AjPStr thys;
    char *p;
    char *q;

    ajStrAssignClear(Prest);

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);
    thys = *Pstr;

    p = thys->Ptr;
    q = thys->Ptr;

    while(*p)
    {
	if(isalpha((ajint)*p))
	    *q++=*p;
	else if(!isspace((ajint)*p))
	    ajStrAppendK(Prest, *p);
	p++;
    }

    *q='\0';
    thys->Len = q - thys->Ptr;

    if(!thys->Len) return ajFalse;
    return ajTrue;
}



/* @func ajStrKeepSetAlphaRestC ***********************************************
**
** Removes all characters from a string that are not alphabetic and
** are not in a given set.
**
** Also returns any non-whitespace characters in the strings
**
** @param [u] Pstr [AjPStr *] String to clean.
** @param [r] txt [const char*] Non-alphabetic character set to keep
** @param [u] Prest [AjPStr *] Excluded non-whitespace characters.
** @return [AjBool] ajTrue if string is not empty
** @@
******************************************************************************/

AjBool ajStrKeepSetAlphaRestC(AjPStr* Pstr, const char* txt, AjPStr* Prest)
{
    AjPStr thys;
    char *p;
    char *q;

    ajStrAssignClear(Prest);

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);
    thys = *Pstr;

    p = thys->Ptr;
    q = thys->Ptr;

    while(*p)
    {
	if(isalpha((ajint)*p))
	    *q++=*p;
	else if(strchr(txt, *p))
	    *q++=*p;
	else if(!isspace((ajint)*p))
	    ajStrAppendK(Prest, *p);
	p++;
    }

    *q='\0';
    thys->Len = q - thys->Ptr;

    if(!thys->Len) return ajFalse;
    return ajTrue;
}



/* @func ajStrKeepSetAlphaRestS ***********************************************
**
** Removes all characters from a string that are not alphabetic and
** are not in a given set.
**
** Also returns any non-whitespace characters in the strings
**
** @param [u] Pstr [AjPStr *] String to clean.
** @param [r] str [const AjPStr] Non-alphabetic character set to keep
** @param [u] Prest [AjPStr *] Excluded non-whitespace characters.
** @return [AjBool] ajTrue if string is not empty
** @@
******************************************************************************/

AjBool ajStrKeepSetAlphaRestS(AjPStr* Pstr, const AjPStr str, AjPStr* Prest)
{
    return ajStrKeepSetAlphaRestC(Pstr, str->Ptr, Prest);
}



/* @func ajStrQuoteStrip ******************************************************
**
** Removes any double quotes from a string.
**
** Internal escaped or double quotes are converted to single quotes.
**
** @param [u] Pstr [AjPStr *] string
**
** @return [AjBool] ajTrue if string is not empty
** @@
******************************************************************************/
AjBool ajStrQuoteStrip(AjPStr* Pstr)
{
    AjPStr thys;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);
    thys = *Pstr;

    if(ajStrGetCharLast(thys) == '"')
	ajStrCutEnd(Pstr, 1);

    if(ajStrGetCharFirst(thys) == '"')
	ajStrCutStart(Pstr, 1);

    ajStrExchangeCC(Pstr, "\"\"", "\""); /* doubled quotes to single */
    ajStrExchangeCC(Pstr, "\\\"", "\""); /* escaped quotes to single */

    if(!thys->Len) return ajFalse;

    return ajTrue;
}




/* @func ajStrQuoteStripAll ***************************************************
**
** Removes any single or double quotes from a string.
** 
** Internal escaped or doubled-up quotes are converted to single quotes.
**
** @param [u] Pstr [AjPStr *] string
**
** @return [AjBool] True on success
** @@
******************************************************************************/
AjBool ajStrQuoteStripAll(AjPStr* Pstr)
{
    AjPStr thys;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);
    thys = *Pstr;

    if(ajStrGetCharLast(thys) == '"')
    {
	ajStrCutEnd(Pstr, 1);

	if(ajStrGetCharFirst(thys) == '"')
	    ajStrCutStart(Pstr, 1);

	ajStrExchangeCC(Pstr, "\"\"", "\""); /* doubled quotes to single */
	ajStrExchangeCC(Pstr, "\\\"", "\""); /* escaped quotes to single */
    }
    else if(ajStrGetCharLast(thys) == '\'')
    {
	ajStrCutEnd(Pstr, 1);

	if(ajStrGetCharFirst(thys) == '\'')
	    ajStrCutStart(Pstr, 1);

	ajStrExchangeCC(Pstr, "''", "'"); /* doubled quotes to single */
	ajStrExchangeCC(Pstr, "\\'", "'"); /* escaped quotes to single */
    }

    if (!thys->Len) return ajFalse;

    return ajTrue;
}




/* @func ajStrRemoveGap *******************************************************
**
** Removes non-sequence characters (all but alphabetic characters and asterisk)
** from a string.
**
** @param [w] Pstr [AjPStr*] String
** @return [AjBool] True if string is not empty
** @@
******************************************************************************/

AjBool ajStrRemoveGap(AjPStr* Pstr)
{
    char *p;
    char *q;
    ajuint  i;
    ajuint  len;
    char c;
    AjPStr thys;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);
    thys = *Pstr;

    p = q = thys->Ptr;
    len = thys->Len;

    for(i=0;i<len;++i)
    {
	c = *(p++);
	/* O is a gap character for Phylip ... but now valid for proteins */
	if((c>='A' && c<='Z') || (c>='a' && c<='z') || (c=='*'))
	    *(q++) = c;
	else
	    --thys->Len;
    }

    thys->Ptr[thys->Len] = '\0';

    if(!thys->Len) return ajFalse;

    return ajTrue;
}



/* @obsolete ajStrDegap
** @rename ajStrRemoveGap
*/

__deprecated void  ajStrDegap(AjPStr* thys)
{
    ajStrRemoveGap(thys);
}

/* @func ajStrRemoveHtml ******************************************************
**
** Removes html from a string.  
**
** html is defined as all substrings between and including angle brackets.
**
** @param [w] Pstr [AjPStr*] String
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajStrRemoveHtml(AjPStr* Pstr)
{
    char *p;
    char *q;
    AjPStr thys;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);
    thys = *Pstr;

    p = q = thys->Ptr;
    while(*p)
    {
	if(*p!='<')
	{
	    *q++=*p++;
	    continue;
	}

	while(*p)
	{
	    --thys->Len;
	    if(*p=='>')
	    {
		++p;
		break;
	    }
	    ++p;
	}
    }
    *q='\0';

    if(!thys->Len) return ajFalse;
    return ajTrue;
}




/* @func ajStrRemoveLastNewline ***********************************************
**
** Removes last character from a string if it is a newline character.
**
** @param [w] Pstr [AjPStr*] String
** @return [AjBool] True is string is not empty
** @@
******************************************************************************/

AjBool ajStrRemoveLastNewline(AjPStr* Pstr)
{
    AjPStr thys;

    thys = *Pstr;

    if(thys->Len)
	if(thys->Ptr[thys->Len-1]=='\n')
	{
	    thys->Ptr[thys->Len-1]='\0';
	    --thys->Len;
	}

    if(!thys->Len) return ajFalse;
    return ajTrue;
}

/* @obsolete ajStrRemoveNewline
** @rename ajStrRemoveLastNewline
*/

__deprecated void  ajStrRemoveNewline(AjPStr* thys)
{
    ajStrRemoveLastNewline(thys);
}

/* @func ajStrRemoveSetC ***************************************************
**
** Removes all of a given set of characters from a string.
**
** @param [w] Pstr [AjPStr*] String
** @param [r] txt [const char*] characters to remove
** @return [AjBool] True on success
** @@
******************************************************************************/

AjBool ajStrRemoveSetC(AjPStr* Pstr, const char *txt)
{
    char *p = NULL;
    char *q = NULL;
    char *r = NULL;
    AjPStr thys = NULL;
    
    if(!(*Pstr))
	return ajFalse;

    thys = *Pstr;
    if(!MAJSTRGETLEN(thys))
	return ajFalse;

    
    p = thys->Ptr;
    q = p;

    while((r=strpbrk(p,txt)))
    {
	while(p!=r)
	    *(q++) = *(p++);
	++p;
	--thys->Len;
	*q = '\0';
    }

    while(*p)
	*(q++) = *(p++);

    *q = '\0';

    if(!thys->Len) return ajFalse;
    return ajTrue;
}

/* @obsolete ajStrRemoveCharsC
** @rename ajStrRemoveSetC
*/

__deprecated void  ajStrRemoveCharsC(AjPStr* pthis, const char *strng)
{
    ajStrRemoveSetC(pthis, strng);
}

/* @func ajStrRemoveWhite *****************************************************
**
** Removes all whitespace characters from a string.
**
** @param [u] Pstr [AjPStr *] String to clean.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrRemoveWhite(AjPStr* Pstr)
{
    AjPStr thys;
    ajuint i = 0;
    ajuint j = 0;
    ajuint len;
    char *p;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);
    thys = *Pstr;

    p   = thys->Ptr;
    len = thys->Len;

    for(i=0;i<len;++i)
	if(p[i]=='\t' || p[i]=='\n' || p[i]=='\r')
	    p[i]=' ';

    for(i=0;i<len;++i)
	if(p[i]!=' ')
	    p[j++]=p[i];
	else
	    --thys->Len;

    p[j]='\0';

    if(!thys->Len) return ajFalse;

    return ajTrue;
}

/* @obsolete ajStrCleanWhite
** @rename ajStrRemoveWhite
*/

__deprecated AjBool  ajStrCleanWhite(AjPStr* s)
{
    return ajStrRemoveWhite(s);
}

/* @func ajStrRemoveWhiteExcess ***********************************************
**
** Removes excess whitespace characters from a string.
**
** Leading/trailing whitespace removed. Multiple spaces replaced by single
** spaces.
**
** @param [u] Pstr [AjPStr *] String to clean.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrRemoveWhiteExcess(AjPStr* Pstr)
{
    AjBool ret      = ajFalse;
    AjPStr thys;

    ajuint i = 0;
    ajuint j = 0;
    ajuint len;
    char *p;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);
    thys = *Pstr;

    p = thys->Ptr;
    
    /* if string was already empty, no need to do anything */
    
    len = thys->Len;
    if(!len)
	return ajFalse;
    
    /* tabs converted to spaces */
    
    for(i=0;i<len;++i)
	if(p[i]=='\t' || p[i]=='\n' || p[i]=='\r')
	    p[i]=' ';
    
    /* remove leading spaces */
    
    i = 0;
    while(p[i])
    {
	if(p[i]!=' ')
	    break;
	++i;
	--len;
    }
    
    if(i)
    {
	len++;
	memmove(p,&p[i], len);
	len=strlen(p);
	if(!len)
	{			    /* if that emptied it, so be it */
	    thys->Len = 0;
	    return ajFalse;
	}
    }
    
    /* remove newline at the end (if any) */
    
    if(p[len-1]=='\n')
    {
	--len;
	p[len]='\0';
    }
    
    if(!len)
    {				    /* if that emptied it, so be it */
	thys->Len = 0;
	return ajFalse;
    }
    
    /* clean up any space at the end */

    for(i=len;i>0;--i)
	if(p[i-1]!=' ')
	    break;

    p[i]='\0';
    
    len=strlen(p);
    
    for(i=j=0;i<len;++i)
    {
	if(p[i]!=' ')
	{
	    p[j++]=p[i];
	    continue;
	}
	p[j++]=' ';

	for(++i;;++i)
	    if(p[i]!=' ')
	    {
		p[j++]=p[i];
		break;
	    }
    }
    
    p[j]='\0';

    thys->Len = j;

    if(!MAJSTRGETLEN(thys))
	return ajFalse;

    return ret;
}


/* @obsolete ajStrClean
** @rename ajStrRemoveWhiteExcess
*/

__deprecated AjBool ajStrClean(AjPStr* s)
{
    return ajStrRemoveWhiteExcess(s);
}

/* @func ajStrRemoveWhiteSpaces ***********************************************
**
** Removes excess space characters from a string.
**
** Leading/trailing whitespace removed. Multiple spaces replaced by single
** spaces. Tabs converted to spaces. Newlines left unchanged
**
** @param [u] Pstr [AjPStr *] String to clean.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrRemoveWhiteSpaces(AjPStr* Pstr)
{
    AjBool ret      = ajFalse;
    AjPStr thys;

    ajuint i = 0;
    ajuint j = 0;
    ajuint len;
    char *p;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);
    thys = *Pstr;

    p = thys->Ptr;
    
    /* if string was already empty, no need to do anything */
    
    len = thys->Len;
    if(!len)
	return ajFalse;
    
    /* tabs converted to spaces */
    
    for(i=0;i<len;++i)
	if(p[i]=='\t')
	    p[i]=' ';
    
    /* remove leading spaces */
    
    i = 0;
    while(p[i])
    {
	if(p[i]!=' ')
	    break;
	++i;
	--len;
    }
    
    if(i)
    {
	len++;
	memmove(p,&p[i], len);
	len=strlen(p);
	if(!len)
	{			    /* if that emptied it, so be it */
	    thys->Len = 0;
	    return ajFalse;
	}
    }
    
    /* remove newline at the end (if any) */
    
    if(p[len-1]=='\n')
    {
	--len;
	p[len]='\0';
    }
    
    if(!len)
    {				    /* if that emptied it, so be it */
	thys->Len = 0;
	return ajFalse;
    }
    
    /* clean up any space at the end */

    for(i=len;i>0;--i)
	if(p[i-1]!=' ')
	    break;

    p[i]='\0';
    
    len=strlen(p);
    
    for(i=j=0;i<len;++i)
    {
	if(p[i]!=' ')
	{
	    p[j++]=p[i];
	    continue;
	}
	p[j++]=' ';

	for(++i;;++i)
	    if(p[i]!=' ')
	    {
		p[j++]=p[i];
		break;
	    }
    }
    
    p[j]='\0';

    thys->Len = j;

    if(!MAJSTRGETLEN(thys))
	return ajFalse;

    return ret;
}


/* @func ajStrRemoveWild ******************************************************
**
** Removes all characters after the first wildcard character (if found). 
**
** @param [u] Pstr [AjPStr*] String
** @return [AjBool] ajTrue if the string contained a wildcard and was
**                  truncated.
** @@
******************************************************************************/

AjBool ajStrRemoveWild(AjPStr* Pstr)
{
    char* cp;
    AjPStr thys;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);
    thys = *Pstr;

    cp = thys->Ptr;

    while(*cp)
    {
	switch(*cp)
	{
	case '?':
	case '*':
	    *cp = '\0';
	    thys->Len = cp - thys->Ptr;
	    return ajTrue;
	default:
	    cp++;
	}
    }

    return ajFalse;
}



/* @obsolete ajStrWildPrefix
** @rename ajStrRemoveWild
*/

__deprecated AjBool  ajStrWildPrefix(AjPStr* str)
{
    return ajStrRemoveWild(str);
}

/* @func ajStrTrimC ***********************************************************
**
** Removes regions with a given character composition from start and end of a 
** string.
**
** @param [u] Pstr [AjPStr*] string
** @param [r] txt [const char*] Characters to delete from each end
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrTrimC(AjPStr* Pstr, const char* txt)
{
    AjPStr thys;
    const char* cp;
    ajuint i;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);
    thys = *Pstr;

    cp = thys->Ptr;
    i = strspn(cp, txt);
    if(i)
    {
	thys->Len -= i;
	if(thys->Len)
	    memmove(thys->Ptr, &thys->Ptr[i], thys->Len);
	else
	    *thys->Ptr = '\0';
    }

    if(i)
	thys->Ptr[thys->Len] = '\0';

    if(!thys->Len)
	return ajFalse;


    cp = &thys->Ptr[thys->Len-1];
    i = 0;
    while(thys->Len && strchr(txt, *cp))
    {
	i++;
	thys->Len--;
	if(thys->Len)
	    cp--;
    }

    if(i)
	thys->Ptr[thys->Len] = '\0';

    return ajTrue;
}




/* @func ajStrTrimEndC ********************************************************
**
** Removes a region with a given character composition from end of a string.
**
** @param [u] Pstr [AjPStr*] string
** @param [r] txt [const char*] Characters to delete from the end
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrTrimEndC(AjPStr* Pstr, const char* txt)
{
    AjPStr thys;
    const char* cp;
    ajuint i;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);
    thys = *Pstr;

    if(!thys->Len)
	return ajFalse;

    cp = &thys->Ptr[thys->Len-1];
    i = 0;
    while(thys->Len && strchr(txt, *cp))
    {
	i++;
	thys->Len--;
	if(thys->Len)
	    cp--;
    }

    if(i)
	thys->Ptr[thys->Len] = '\0';

    return ajTrue;
}




/* @func ajStrTrimStartC ******************************************************
**
** Removes a region with a given character composition from start of a string.
**
** @param [u] Pstr [AjPStr*] string
** @param [r] txt [const char*] Characters to delete from the end
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrTrimStartC(AjPStr* Pstr, const char* txt)
{
    AjPStr thys;
    const char* cp;
    ajuint i;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);
    thys = *Pstr;

    if(!thys->Len)
	return ajFalse;

    cp = thys->Ptr;
    i = 0;
    while(thys->Len && strchr(txt, *cp))
    {
	i++;
	thys->Len--;
	cp++;
    }

    if(i)
	memmove(thys->Ptr, &thys->Ptr[i], thys->Len+1);

    return ajTrue;
}




/* @func ajStrTrimWhite *******************************************************
**
** Removes regions composed of white space characters only from the start and 
** end of a string.
**
** @param [u] Pstr [AjPStr*] String
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrTrimWhite(AjPStr* Pstr)
{
    return ajStrTrimC(Pstr, "\t \n\r");
}


/* @obsolete ajStrChomp
** @rename ajStrTrimWhite
*/

__deprecated AjBool  ajStrChomp(AjPStr* pthis)
{
    return ajStrTrimWhite(pthis);
}


/* @obsolete ajStrChompC
** @rename ajStrTrimC
*/

__deprecated AjBool  ajStrChompC(AjPStr* pthis, const char* delim)
{
    return ajStrTrimC(pthis, delim);
}

/* @obsolete ajStrChop
** @replace ajStrCutEnd (1/1,'1')
*/

__deprecated AjBool  ajStrChop(AjPStr* pthis)
{
    return ajStrCutEnd(pthis, 1);
}



/* @func ajStrTrimWhiteEnd ****************************************************
**
** Removes regions composed of white space characters only from the end of a 
** string.
**
** @param [u] Pstr [AjPStr*] String
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrTrimWhiteEnd(AjPStr* Pstr)
{
    return ajStrTrimEndC(Pstr, "\t \n\r");
}

/* @obsolete ajStrChompEnd
** @rename ajStrTrimWhiteEnd
*/

__deprecated AjBool  ajStrChompEnd(AjPStr* pthis)
{
    return ajStrTrimWhiteEnd(pthis);
}

/* @func ajStrTruncateLen *****************************************************
**
** Removes the end from a string reducing it to a defined length.
**
** @param [u] Pstr [AjPStr*] Target string
** @param [r] len [ajuint] Length of required string.
**
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrTruncateLen(AjPStr* Pstr, ajuint len)
{
    AjPStr thys;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);
    thys = *Pstr;

    if(len > thys->Len) return ajTrue;

    thys->Ptr[len] = '\0';
    thys->Len = len;

    return ajTrue;
}



/* @func ajStrTruncatePos *************************************************
**
** Removes the end from a string by cutting at a defined position.
**
** @param [u] Pstr [AjPStr*] target string
** @param [r] pos [ajint] First position to be deleted. Negative values
**                        count from the end
** @return [AjBool] True is string was feallocated
** @@
******************************************************************************/

AjBool ajStrTruncatePos(AjPStr* Pstr, ajint pos)
{
    AjPStr thys;
    ajuint ibegin;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);
    thys = *Pstr;

    ibegin = 1 + ajMathPos(thys->Len, pos);
    thys->Ptr[ibegin] = '\0';
    thys->Len = ibegin;

    return ajTrue;
}


/* @obsolete ajStrTruncate
@rename ajStrTruncateLen
*/

__deprecated AjBool  ajStrTruncate(AjPStr* Pstr, ajint pos)
{
    return ajStrTruncateLen(Pstr, pos);
}


/* @section substitution ******************************************************
**
** Functions for substitutions of characters or regions (substrings)
** within a string.
**
** @fdata      [AjPStr]
** @fnote     None.
**
** @nam3rule  Exchange           Substitute substrings in a string with other
**                               strings.
** @nam3rule  Random             Randomly rearrange characters.
** @nam3rule  Reverse            Reverse order of characters.
** @nam4rule  ExchangePos        Substitute character(s) at a set position
** @nam4rule  ExchangeSet        Substitute character(s) in a string with
**                               other character(s).
** @nam5rule  ExchangeSetRest    Substitute character(s) in a string with
**                               other character(s).
**
** @argrule * Pstr [AjPStr*] String to be edited
** @argrule Pos ipos [ajint] String position to be edited
** @arg1rule C txt [const char*] Text to be replaced
** @arg1rule K chr [char] Text to be replaced
** @arg1rule S str [const AjPStr] Text to be replaced
** @arg2rule C txtnew [const char*] New text
** @arg2rule K chrnew [char] New text
** @arg2rule S strnew [const AjPStr] New text
**
** @valrule * [AjBool] True if string was reallocated
**
** @fcategory modify
*/

/* @func ajStrExchangeCC ****************************************************
**
** Replace all occurrences in a string of one substring with another.
**
** @param [u] Pstr [AjPStr*]  Target string.
** @param [r] txt [const char*] string to replace.
** @param [r] txtnew [const char*] string to insert.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrExchangeCC(AjPStr* Pstr, const char* txt,
			 const char* txtnew)
{    
    AjBool cycle = ajTrue;
    ajint findpos    = 0;
    ajuint tlen = strlen(txt);
    ajuint newlen = strlen(txtnew);
    ajint lastpos = 0;			/* make sure we don't loop forever */

    if(!tlen && !newlen)
	return ajFalse;

    if(*txt)
    {
	while(cycle)
	{
	    findpos = ajStrFindC(*Pstr, txt);
	    if(findpos >= lastpos)
	    {
		ajStrCutRange(Pstr,findpos,findpos+tlen-1);
		ajStrInsertC(Pstr,findpos,txtnew);
		lastpos = findpos+newlen;
	    }
	    else
		cycle = ajFalse;
	}
    }

    return ajTrue;
}

/* @obsolete ajStrSubstituteCC
** @rename ajStrExchangeCC
*/

__deprecated AjBool  ajStrSubstituteCC(AjPStr* pthis, const char* replace,
			 const char* putin)
{
    return ajStrExchangeCC(pthis, replace, putin);
}


/* @func ajStrExchangeCS ****************************************************
**
** Replace all occurrences in a string of one substring with another.
**
** @param [u] Pstr [AjPStr*]  Target string.
** @param [r] txt [const char*] string to replace.
** @param [r] strnew [const AjPStr] string to insert.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrExchangeCS(AjPStr* Pstr, const char* txt,
		       const AjPStr strnew)
{    
    AjBool cycle = ajTrue;
    ajint findpos    = 0;
    ajuint tlen = strlen(txt);
    ajuint newlen = strnew->Len;
    ajint lastpos = 0;			/* make sure we don't loop forever */

    if(!tlen && !newlen)
	return ajFalse;

    if(*txt)
    {
	while(cycle)
	{
	    findpos = ajStrFindC(*Pstr, txt);
	    if(findpos >= lastpos)
	    {
		ajStrCutRange(Pstr,findpos,findpos+tlen-1);
		ajStrInsertS(Pstr,findpos,strnew);
		lastpos = findpos+newlen;
	    }
	    else
		cycle = ajFalse;
	}
    }

    return ajTrue;
}

/* @func ajStrExchangeKK ******************************************************
**
** Replace all occurrences in a string of one character with another. 
**
** @param [u] Pstr [AjPStr*]  Target string.
** @param [r]  chr [char] Character to replace.
** @param [r]  chrnew [char] Character to insert.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrExchangeKK(AjPStr* Pstr, char chr, char chrnew)
{    
    AjBool ret = ajFalse;
    AjPStr thys;
    char* cp;
    
    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);
    thys = *Pstr;

    cp   = thys->Ptr;
    
    while(*cp)
    {
	if(*cp == chr)
	    *cp = chrnew;
	cp++;
    }
    
    return ret;
}


/* @obsolete ajStrSubstituteKK
** @rename ajStrExchangeKK
*/

__deprecated AjBool  ajStrSubstituteKK(AjPStr* pthis, char replace, char putin)
{    
    return ajStrExchangeKK(pthis, replace, putin);
}

/* @func ajStrExchangeSC ****************************************************
**
** Replace all occurrences in a string of one substring with another.
**
**
** @param [u] Pstr [AjPStr*]  Target string.
** @param [r] str [const AjPStr] string to replace.
** @param [r] txtnew [const char*] string to insert.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrExchangeSC(AjPStr* Pstr, const AjPStr str,
		       const char* txtnew)
{    
    AjBool cycle = ajTrue;
    ajint findpos    = 0;

    if(str->Len)
    {
	while(cycle)
	{
	    findpos = ajStrFindS(*Pstr, str);
	    if(findpos >= 0)
	    {
		ajStrCutRange(Pstr,findpos,findpos+str->Len-1);
		ajStrInsertC(Pstr,findpos,txtnew);
	    }
	    else
		cycle = ajFalse;
	}
    }

    return ajTrue;
}

/* @func ajStrExchangeSS ******************************************************
**
** Replace all occurrences in a string of one substring with another.
**
** @param [u] Pstr [AjPStr*]  Target string.
** @param [r] str [const AjPStr] string to replace.
** @param [r] strnew [const AjPStr]   string to insert.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrExchangeSS(AjPStr* Pstr, const AjPStr str, const AjPStr strnew)
{    
    AjBool cycle = ajTrue;
    ajint findpos    = 0;
    
    if(str->Len !=0)
    {
	while(cycle)
	{
	    findpos = ajStrFindC(*Pstr, str->Ptr);
	    if(findpos >= 0)
	    {
		ajStrCutRange(Pstr,findpos,findpos+str->Len-1);
		ajStrInsertS(Pstr,findpos,strnew);
	    }
	    else
		cycle = ajFalse;
	}
    }

    return ajTrue;
}


/* @obsolete ajStrSubstitute
** @rename ajStrExchangeSS
*/

__deprecated AjBool  ajStrSubstitute(AjPStr* pthis,
				    const AjPStr replace, const AjPStr putin)
{    
    return ajStrExchangeSS(pthis, replace, putin);
}

/* @func ajStrExchangePosCC ***************************************************
**
** Replace one substring with another at a given position in the text.
**
** @param [u] Pstr [AjPStr*]  Target string.
** @param [r] ipos [ajint] Position in the string, negative values are
**        from the end of the string.
** @param [r] txt [const char*] string to replace.
** @param [r] txtnew [const char*] string to insert.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrExchangePosCC(AjPStr* Pstr, ajint ipos, const char* txt,
			 const char* txtnew)
{    
    ajuint tlen = strlen(txt);
    ajint jpos = ajMathPos((*Pstr)->Len, ipos);

    if(ajCharPrefixC(&(*Pstr)->Ptr[jpos], txt))
    {
	ajStrCutRange(Pstr,jpos,jpos+tlen-1);
	ajStrInsertC(Pstr,jpos,txtnew);
    }

    return ajTrue;
}

/* @func ajStrExchangeSetCC ***************************************************
**
** Replace all occurrences in a string of one set of characters with another
** set. 
**
** @param [w] Pstr [AjPStr*] String
** @param [r] txt [const char*] Unwanted characters
** @param [r] txtnew [const char*] Replacement characters
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrExchangeSetCC(AjPStr* Pstr, const char* txt, const char* txtnew)
{
    char filter[256] = {'\0'};		/* should make all zero */
    ajuint i;

    const char *co;
    const char *cn;
    char* cp;
    AjPStr thys;

    co = txt;
    cn = txtnew;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);
    thys = *Pstr;

    i = strlen(txtnew);
    if(strlen(txt) > i)
    {
	ajErr("ajStrExchangeSetCC new char set '%s' shorter than old '%s'",
	       txt, txtnew);
    }

    while(*co)
    {
	if(!*cn)			/* oops, too short, use first */
	    filter[(ajint)*co++] = *txtnew;
	else
	    filter[(ajint)*co++] = *cn++;
    }

    for(cp = thys->Ptr; *cp; cp++)
	if(filter[(ajint)*cp])
	    *cp = filter[(ajint)*cp];

    return ajTrue;
}




/* @obsolete ajStrConvertCC
** @rename ajStrExchangeSetCC
*/

__deprecated AjBool  ajStrConvertCC(AjPStr* pthis, const char* oldc,
				 const char* newc)
{
    return ajStrExchangeSetCC(pthis, oldc, newc);
}






/* @func ajStrExchangeSetSS ***************************************************
**
** Replace all occurrences in a string of one set of characters with another
** set. 
**
** @param [w] Pstr [AjPStr*] String
** @param [r] str [const AjPStr] Unwanted characters
** @param [r] strnew [const AjPStr] Replacement characters
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrExchangeSetSS(AjPStr* Pstr, const AjPStr str, const AjPStr strnew)
{
    return ajStrExchangeSetCC(Pstr, str->Ptr, strnew->Ptr);
}



/* @obsolete ajStrConvert
** @rename ajStrExchangeSetSS
*/

__deprecated AjBool  ajStrConvert(AjPStr* pthis, const AjPStr oldc,
				 const AjPStr newc)
{
    return ajStrExchangeSetSS(pthis, oldc, newc);
}



/* @func ajStrExchangeSetRestCK ***********************************************
**
** Replace all occurrences in a string of one set of characters with
** a substitute character
**
** @param [w] Pstr [AjPStr*] String
** @param [r] txt [const char*] Wanted characters
** @param [r] chrnew [char] Replacement character
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrExchangeSetRestCK(AjPStr* Pstr, const char* txt, char chrnew)
{
    char filter[256] = {'\0'};		/* should make all zero */

    const char *co;
    char* cp;
    AjPStr thys;

    /*ajDebug("ajStrExchangeSetRestCK '%s' => '%c' for '%S\n",
      txt, chrnew, *Pstr);*/
    co = txt;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);
    thys = *Pstr;

    while(*co)
    {
	filter[(ajint)toupper(*co)] = chrnew;
	filter[(ajint)tolower(*co++)] = chrnew;
    }

    for(cp = thys->Ptr; *cp; cp++)
	if(!filter[(ajint)*cp])
	    *cp = chrnew;

    /*ajDebug("ajStrExchangeSetRestCK returns '%S\n",
            *Pstr);*/
    return ajTrue;
}



/* @func ajStrExchangeSetRestSK ***********************************************
**
** Replace all occurrences in a string not in one set of characters with
** a substitute character.
**
** @param [w] Pstr [AjPStr*] String
** @param [r] str [const AjPStr] Wanted characters
** @param [r] chrnew [char] Replacement character
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrExchangeSetRestSK(AjPStr* Pstr, const AjPStr str,
			      char chrnew)
{
    return ajStrExchangeSetRestCK(Pstr, str->Ptr, chrnew);
}


/* @func ajStrRandom **********************************************************
**
** Randomly rearranges the characters in a string. 
**
** The application code must first call ajRandomSeed().
**
** @param [u] Pstr [AjPStr *] string
**
** @return [AjBool] True unless string is empty
** @@
******************************************************************************/
AjBool ajStrRandom(AjPStr* Pstr)
{
    AjPStr copy = NULL;
    AjPStr thys;
    const char *p;
    char *q;

    ajuint *rn = NULL;
    ajuint *na = NULL;

    ajuint len;
    ajuint i;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);
    thys = *Pstr;

    ajStrAssignS(&copy, thys);
    p=copy->Ptr;
    q=thys->Ptr;

    len = thys->Len;
    AJCNEW(na, len);
    AJCNEW(rn, len);

    for(i=0;i<len;++i)
    {
	na[i] = i;
	rn[i] = ajRandomNumber();
    }
    ajSortUintIncI(rn,na,len);

    for(i=0;i<len;++i)
	q[i] = p[na[i]];

    AJFREE(na);
    AJFREE(rn);
    ajStrDel(&copy);


    if(!thys->Len) return ajFalse;
    return ajTrue;
}



/* @func ajStrReverse *********************************************************
**
** Reverses the order of characters in a string
**
** @param [w] Pstr [AjPStr*] Target string
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrReverse(AjPStr* Pstr)
{
    AjBool ret = ajFalse;
    char *cp;
    char *cq;
    char tmp;
    AjPStr thys;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);
    thys = *Pstr;

    cp = thys->Ptr;
    cq = cp + thys->Len - 1;

    while(cp < cq)
    {
	tmp = *cp;
	*cp = *cq;
	*cq = tmp;
	cp++;
	cq--;
    }

    return ret;
}



/* @obsolete ajStrRev
** @rename ajStrReverse
*/

__deprecated AjBool  ajStrRev(AjPStr* pthis)
{
    return ajStrReverse(pthis);
}

/* @section query *************************************************************
**
** Functions for querying the properties of strings.
**
** @fdata       [AjPStr]
** @fnote      None.
** @nam3rule   Calc   Calculate a value (not in categories above) derived from
**                    elements of a string object.
** @nam4rule   CalcCount Counts occurence of character(s) in string.
** @nam4rule   CalcWhole Tests whether range refers to whole string.
** @nam3rule   Has    Check whether string contains certain character(s).
** @nam4rule   HasParentheses Contains parentheses.
** @nam4rule   HasWild        Contains wildcard character(s).
** @nam3rule   Is     Check whether string value represents a datatype or is
**                    of a certain character composition.
** @nam4rule   IsAlnum        Alphanumeric characters only.
** @nam4rule   IsAlpha        Alphabetic characters only.
** @nam4rule   IsBool         Represents Boolean value.
** @nam4rule   IsCharset      Specified characters only.
** @nam5rule   IsCharsetCase  Specified characters only.
** @nam4rule   IsDouble       Represents double value.
** @nam4rule   IsFloat        Represents float value.
** @nam4rule   IsHex          Represents hex value.
** @nam4rule   IsInt          Represents integer value.
** @nam4rule   IsLong         Represents long value.
** @nam4rule   IsLower        No uppercase alphabetic characters.
** @nam4rule   IsNum          Decimal digits only.
** @nam4rule   IsUpper        No lowercase alphabetic characters.
** @nam4rule   IsWhite        Whitespace characters only.
** @nam4rule   IsWild         One or more wildcard characters.
** @nam4rule   IsWord         Non-whitespace characters only. 
** @nam3rule   Whole          Specified range covers whole string. 
**
** @argrule * str [const AjPStr] String
** @arg2rule S str2 [const AjPStr] String
** @argrule Whole pos1 [ajint] Start position, negative values count from end
** @argrule Whole pos2 [ajint] End position, negative values count from end
** @argrule CountC txt [const char*] Text to count
** @argrule CountK chr [char] Character to count
** @argrule IsCharsetC txt [const char*] Characters to test
** @argrule IsCharsetS str2 [const AjPStr] Characters to test
** @argrule IsCharsetCaseC txt [const char*] Characters to test
** @argrule IsCharsetCaseS str2 [const AjPStr] Characters to test
**
** @valrule * [AjBool] Result of query
** @valrule *Count [ajuint] Number of occurrences
**
** @fcategory use
*/


/* @func ajStrCalcCountC ******************************************************
**
** Counts occurrences of set of characters in a string.
**
** @param [r] str [const AjPStr] String
** @param [r] txt [const char*] Characters to count
** @return [ajuint] Number of times characters were found in string
******************************************************************************/

ajuint ajStrCalcCountC(const AjPStr str, const char* txt)
{
    ajuint ret = 0;
    const char* cp;

    cp = txt;

    while(*cp)
    {
	ret += ajStrCalcCountK(str, *cp);
	cp++;
    }

    return ret;
}



/* @obsolete ajStrCountC
** @rename ajStrCalcCountC
*/

__deprecated ajint  ajStrCountC(const AjPStr str, const char* txt)
{
    return ajStrCalcCountC(str, txt);
}

/* @func ajStrCalcCountK ******************************************************
**
** Counts occurrences of a character in a string.
**
** @param [r] str [const AjPStr] String
** @param [r] chr [char] Character to count
** @return [ajuint] Number of times character was found in string
******************************************************************************/

ajuint ajStrCalcCountK(const AjPStr str, char chr)
{
    ajuint ret = 0;
    const char* cp;

    if(!str) return 0;

    cp = str->Ptr;

    while(*cp)
    {
	if(*cp == chr) ret++;
	cp++;
    }

    return ret;
}



/* @obsolete ajStrCountK
** @rename ajStrCalcCountK
*/

__deprecated ajint  ajStrCountK(const AjPStr str, char ch)
{
    return ajStrCalcCountK(str, ch);
}

/* @func ajStrHasParentheses **************************************************
**
** Tests whether a string contains (possibly nested) pairs of parentheses.
** 
** Pairs of parentheses are difficult to test for with regular expressions.
**
** @param [r] str [const AjPStr] String to test
** @return [AjBool] ajTrue if string has zero or more () pairs
**                  with possibly other text
** @@
******************************************************************************/

AjBool ajStrHasParentheses(const AjPStr str)
{
    ajuint left = 0;
    const char *cp;

    if(!str) return ajFalse;
    
    cp = str->Ptr;
    
    /* if string was already empty, no need to do anything */
    
    while (*cp)
    {
	switch (*cp++)
	{
	case '(':
	    left++;
	    break;
	case ')':
	    if (!left) return ajFalse;
	    left--;
	    break;
	default:
	    break;
	}
    }
    if (left)
	return ajFalse;

    return ajTrue;
}


/* @obsolete ajStrParentheses
** @rename ajStrHasParentheses
*/

__deprecated AjBool  ajStrParentheses(const AjPStr s)
{
    return ajStrHasParentheses(s);
}

/* @func ajStrIsAlnum *********************************************************
**
** Test whether a string contains alphanumeric characters only (no white 
** space).
**
** The test is defined by isalnum in the C RTL plus underscores.
**
** @param [r] str [const AjPStr] String
** @return [AjBool] ajTrue if the string is entirely alphanumeric
** @cre an empty string returns ajFalse
** @@
******************************************************************************/

AjBool ajStrIsAlnum(const AjPStr str)
{
    const char* cp;

    if(!str)
	return ajFalse;

    if(!str->Len)
	return ajFalse;

    for(cp = str->Ptr;*cp;cp++)
	if(*cp != '_' && !isalnum((ajint)*cp))
	    return ajFalse;

    return ajTrue;
}



/* @func ajStrIsAlpha *********************************************************
**
** Test whether a string contains alphabetic characters only (no white space).
** 
** Test is defined by isalpha in the C RTL.
**
** @param [r] str [const AjPStr] String
** @return [AjBool] ajTrue if the string is entirely alphabetic
** @cre an empty string returns ajFalse
** @@
******************************************************************************/

AjBool ajStrIsAlpha(const AjPStr str)
{
    const char* cp;

    if(!str)
	return ajFalse;

    if(!str->Len)
	return ajFalse;

    cp = str->Ptr;

    while(*cp)
	if(!isalpha((ajint)*cp++))
	    return ajFalse;

    return ajTrue;
}




/* @func ajStrIsBool **********************************************************
**
** Tests whether a string represents a valid Boolean value.
**
** The string must be a single character boolean value, or one of "yes, "no",
** "true" or "false".
**
** @param [r] str [const AjPStr] String
** @return [AjBool] ajTrue if the string is acceptable as a boolean.
** @see ajStrToBool
** @@
******************************************************************************/

AjBool ajStrIsBool(const AjPStr str)
{
    const char* cp;

    if(!str)
	return ajFalse;

    if(!str->Len)
	return ajFalse;

    cp = str->Ptr;

    if(!strchr("YyTt1NnFf0", *cp))
	return ajFalse;

    if(str->Len == 1)		/* one character only */
	return ajTrue;

/* longer strings - test they really match */

    if(ajStrMatchCaseC(str, "yes"))
	return ajTrue;

    if(ajStrMatchCaseC(str, "no"))
	return ajTrue;

    if(ajStrMatchCaseC(str, "true"))
	return ajTrue;

    if(ajStrMatchCaseC(str, "false"))
	return ajTrue;

    return ajFalse;
}




/* @func ajStrIsCharsetC ******************************************************
**
** Test whether a string contains specified characters only.
** 
** @param [r] str [const AjPStr] String
** @param [r] txt [const char*] Character set to test
** @return [AjBool] ajTrue if the string is entirely composed of characters
**                  in the specified set
** @cre an empty string returns ajFalse
** @@
******************************************************************************/

AjBool ajStrIsCharsetC(const AjPStr str, const char* txt)
{
    char filter[256] = {'\0'};		/* should make all zero */
    const char* cp;
    const char* cq;

    if(!str)
	return ajFalse;

    if(!str->Len)
	return ajFalse;

    cq = txt;
    while (*cq)
      filter[(int)*cq++] = 1;

    cp = str->Ptr;

    while(*cp)
      if(!filter[(int)*cp++])
	    return ajFalse;

    return ajTrue;
}




/* @func ajStrIsCharsetS ******************************************************
**
** Test whether a string contains specified characters only.
** 
** @param [r] str [const AjPStr] String
** @param [r] str2 [const AjPStr] Character set to test
** @return [AjBool] ajTrue if the string is entirely composed of characters
**                  in the specified set
** @cre an empty string returns ajFalse
** @@
******************************************************************************/

AjBool ajStrIsCharsetS(const AjPStr str, const AjPStr str2)
{
    char filter[256] = {'\0'};		/* should make all zero */
    const char* cp;
    const char* cq;

    if(!str)
	return ajFalse;

    if(!str->Len)
	return ajFalse;

    if(!str2)
      return ajFalse;

    cq = str2->Ptr;

    while (*cq)
      filter[(int)*cq++] = 1;

    cp = str->Ptr;

    while(*cp)
      if(!filter[(int)*cp++])
	    return ajFalse;

    return ajTrue;
}


/* @func ajStrIsCharsetCaseC **************************************************
**
** Test whether a string contains specified characters only.
** The test is case-insensitive
** 
** @param [r] str [const AjPStr] String
** @param [r] txt [const char*] Character set to test
** @return [AjBool] ajTrue if the string is entirely composed of characters
**                  in the specified set
** @cre an empty string returns ajFalse
** @@
******************************************************************************/

AjBool ajStrIsCharsetCaseC(const AjPStr str, const char* txt)
{
    char filter[256] = {'\0'};		/* should make all zero */
    const char* cp;
    const char* cq = txt;

    if(!str)
	return ajFalse;

    if(!str->Len)
	return ajFalse;

    while (*cq)
      filter[toupper((int)*cq++)] = 1;

    cp = str->Ptr;

    while(*cp)
      if(!filter[toupper((int)*cp++)])
	    return ajFalse;

    return ajTrue;
}




/* @func ajStrIsCharsetCaseS **************************************************
**
** Test whether a string contains specified characters only.
** The test is case-insensitive
** 
** @param [r] str [const AjPStr] String
** @param [r] str2 [const AjPStr] Character set to test
** @return [AjBool] ajTrue if the string is entirely composed of characters
**                  in the specified set
** @cre an empty string returns ajFalse
** @@
******************************************************************************/

AjBool ajStrIsCharsetCaseS(const AjPStr str, const AjPStr str2)
{
    char filter[256] = {'\0'};		/* should make all zero */
    const char* cp;
    const char* cq;

    if(!str)
	return ajFalse;

    if(!str->Len)
	return ajFalse;

    if(!str2)
	return ajFalse;

    cq = str2->Ptr;

    while (*cq)
      filter[(int)*cq++] = 1;

    cp = str->Ptr;

    while(*cp)
      if(!filter[toupper((int)*cp++)])
	    return ajFalse;

    return ajTrue;
}




/* @func ajStrIsDouble ********************************************************
**
** Tests whether a string represents a valid double precision value,
**
** Test uses the strtod call in the C RTL.
**
** @param [r] str [const AjPStr] String
** @return [AjBool] ajTrue if the string is acceptable as a double
**         precision number.
** @cre an empty string always returns false.
** @see ajStrTokIntBool
** @@
******************************************************************************/

AjBool ajStrIsDouble(const AjPStr str)
{
    const char* cp;
    char* ptr = NULL;

    if(!str)
	return ajFalse;

    if(!str->Len)
	return ajFalse;

    cp = str->Ptr;

    errno = 0;
    strtod(cp, &ptr);

    if(*ptr || errno == ERANGE)
	return ajFalse;

    return ajTrue;
}




/* @func ajStrIsFloat *********************************************************
**
** Tests whether a string represents a valid floating point value.
**
** Test uses the strtod call in the C RTL.
**
** @param [r] str [const AjPStr] String
** @return [AjBool] ajTrue if the string is acceptable as a floating
**         point number.
** @cre an empty string always returns false.
** @see ajStrTokIntBool
** @@
******************************************************************************/

AjBool ajStrIsFloat(const AjPStr str)
{
    const char* cp;
    char* ptr = NULL;
    double d;

    if(!str)
	return ajFalse;

    if(!str->Len)
	return ajFalse;

    cp = str->Ptr;

    errno = 0;
    d = strtod(cp, &ptr);

    if(*ptr || errno == ERANGE)
	return ajFalse;

    if(d > FLT_MAX)
	return ajFalse;

    if(d < -FLT_MAX)
	return ajFalse;

    return ajTrue;
}




/* @func ajStrIsHex ***********************************************************
**
** Tests whether a string represents a valid hexadecimal value.
**
** Test uses the strtol call in the C RTL.
**
** @param [r] str [const AjPStr] String
** @return [AjBool] ajTrue if the string is acceptable as a hexadecimal value.
** @cre an empty string always returns false.
** @@
******************************************************************************/

AjBool ajStrIsHex(const AjPStr str)
{
    const char* cp;
    char* ptr = NULL;

    if(!str)
	return ajFalse;

    if(!str->Len)
	return ajFalse;

    cp = str->Ptr;

    errno = 0;
    strtol(cp, &ptr, 16);
    if(*ptr || errno == ERANGE)
	return ajFalse;

    return ajTrue;
}




/* @func ajStrIsInt ***********************************************************
**
** Tests whether a string represents a valid integer value.
**
** Test uses the strtol call in the C RTL.
**
** @param [r] str [const AjPStr] String
** @return [AjBool] ajTrue if the string is acceptable as an integer.
** @cre an empty string always returns false.
** @see ajStrTokIntBool
** @@
******************************************************************************/

AjBool ajStrIsInt(const AjPStr str)
{
    const char* cp;
    char* ptr = NULL;

    if(!str)
	return ajFalse;

    if(!str->Len)
	return ajFalse;

    cp = str->Ptr;

    errno = 0;
    strtol(cp, &ptr, 10);
    if(*ptr || errno == ERANGE)
	return ajFalse;

    return ajTrue;
}




/* @func ajStrIsLong **********************************************************
**
** Tests whether a string represents a valid ajlong integer value.
**
** Test uses the strtol call in the C RTL.
**
** @param [r] str [const AjPStr] String
** @return [AjBool] ajTrue if the string is acceptable as an integer.
** @cre an empty string always returns false.
** @see ajStrTokIntBool
** @@
******************************************************************************/

AjBool ajStrIsLong(const AjPStr str)
{
    const char* cp;
    char* ptr = NULL;

    if(!str)
	return ajFalse;

    if(!str->Len)
	return ajFalse;

    cp = str->Ptr;

    errno = 0;
    strtol(cp, &ptr, 10);

    if(*ptr || errno == ERANGE)
	return ajFalse;

    return ajTrue;
}




/* @func ajStrIsLower *********************************************************
**
** Tests whether a string contains no upper case alphabetic characters.
**
** Test is defined by isupper in the C RTL..
**
** @param [r] str [const AjPStr] String
** @return [AjBool] ajTrue if the string is entirely alphabetic
** @cre an empty string returns ajTrue
** @@
******************************************************************************/

AjBool ajStrIsLower(const AjPStr str)
{
    const char* cp;

    if(!str)
	return ajTrue;

    if(!str->Len)
	return ajTrue;

    cp =str->Ptr;

    while(*cp)
	if(isupper((ajint)*cp++))
	    return ajFalse;

    return ajTrue;
}




/* @func ajStrIsNum ***********************************************************
**
** Test whether a string contains decimal digits only.
**
** Test is defined by isdigit in the C RTL..
**
** @param [r] str [const AjPStr] String
** @return [AjBool] ajTrue if the string is entirely numeric
** @cre an empty string returns ajFalse
** @@
******************************************************************************/

AjBool ajStrIsNum(const AjPStr str)
{
    const char* cp;

    if(!str)
	return ajFalse;

    if(!str->Len)
	return ajFalse;

    for(cp = str->Ptr;*cp;cp++)
	if(!isdigit((ajint)*cp))
	    return ajFalse;

    return ajTrue;
}




/* @func ajStrIsUpper *********************************************************
**
** Test whether a string contains no lower case alphabetic characters.
**
** Test is defined by islower in the C RTL..
**
** @param [r] str [const AjPStr] String
** @return [AjBool] ajTrue if the string has no lower case characters.
** @cre an empty string returns ajTrue
** @@
******************************************************************************/

AjBool ajStrIsUpper(const AjPStr str)
{
    const char* cp;

    if(!str)
	return ajFalse;

    if(!str->Len)
	return ajFalse;

    cp = str->Ptr;

    while(*cp)
	if(islower((ajint)*cp++))
	    return ajFalse;

    return ajTrue;
}




/* @func ajStrIsWhite ********************************************************
**
** Test whether a string contains whitespace characters only.
**
** The test is made using the strtod call in the C RTL.
**
** @param [r] str [const AjPStr] String
** @return [AjBool] ajTrue if the string is only white space (or empty).
** @@
******************************************************************************/

AjBool ajStrIsWhite(const AjPStr str)
{
    const char* cp;

    if(!str)
	return ajFalse;

    if(!str->Len)
	return ajTrue;

    cp = str->Ptr;

    while (*cp)
    {
	if (!isspace((int)*cp)) return ajFalse;
	cp++;
    }

    return ajTrue;
}




/* @func ajStrIsWild **********************************************************
**
** Tests whether a string contains the standard wildcard characters * or ?.
**
** @param [r] str [const AjPStr] String
** @return [AjBool] ajTrue if string has wildcards.
** @@
******************************************************************************/

AjBool ajStrIsWild(const AjPStr str)
{
    if(ajStrFindAnyC(str, "*?") >= 0)
	return ajTrue;

    return ajFalse;
}





/* @func ajStrIsWord **********************************************************
**
** Test whether a string contains no white space characters. 
**
** Test is defined by isspace in the C RTL.
**
** @param [r] str [const AjPStr] String
** @return [AjBool] ajTrue if the string has no white space
** @cre an empty string returns ajFalse
** @@
******************************************************************************/

AjBool ajStrIsWord(const AjPStr str)
{
    const char* cp;

    if(!str)
	return ajFalse;

    if(!str->Len)
	return ajFalse;

    cp = str->Ptr;

    while(*cp)
	if(isspace((ajint)*cp++))
	    return ajFalse;

    return ajTrue;
}


/* @obsolete ajStrIsSpace
** @rename ajStrIsWhite
*/

__deprecated AjBool  ajStrIsSpace(const AjPStr thys)
{
    return ajStrIsWhite(thys);
}





/* @func ajStrWhole ***********************************************************
**
** Tests whether a range refers to the whole string
**
** @param [r] str [const AjPStr] String
** @param [r] pos1 [ajint] Begin position (0 start, negative from the end)
** @param [r] pos2 [ajint] Begin position (0 start, negative from the end)
** @return [AjBool] ajTrue is range covers the whole string
******************************************************************************/

AjBool ajStrWhole(const AjPStr str, ajint pos1, ajint pos2)
{
    ajuint ibeg;
    ajuint iend;

    ibeg = ajMathPos(str->Len, pos1);
    if(ibeg)
	return ajFalse;

    iend = ajMathPosI(str->Len, ibeg, pos2);
    if(iend != (str->Len - 1))
	return ajFalse;

    return ajTrue;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of a string object.
**
** @fdata       [AjPStr]
** @fnote      The last word of the ajStrGet family of functions is the
**             element variable name.
** @fnote      Each ajXxxGet function should be provided as a macro.
**
** @nam3rule   Get         Retrieve an unmodified element of a string object. 
** @nam4rule   GetChar     Retrieve first single character
** @nam5rule   GetCharFirst     Retrieve first single character
** @nam5rule   GetCharLast     Retrieve last single character
** @nam5rule   GetCharPos    Retrieve a single character
** @nam4rule   GetLen     Retrieve length
** @nam4rule   GetPtr     Retrieve char* pointer
** @nam4rule   GetRes     Retrieve eserved memory size
** @nam4rule   GetPtr         Retrieve additional space available
** @nam4rule   GetRoom         Retrieve additional space available
** @nam4rule   GetUse     Retrieve use (reference) count
** @nam4rule   GetValid     Test string is valid
**
** @argrule * str [const AjPStr] String
** @argrule GetCharPos pos [ajint] Position, negative values count from the end
**
** @valrule GetChar [char] Single character
** @valrule GetLen [ajuint] String length
** @valrule GetRes [ajuint] Reserved size
** @valrule GetPtr [const char*] read-only C string pointer to internal string
** @valrule GetuniquePtr [char*] Modifiable C string pointer to internal string
** @valrule GetUse [ajuint] Reference (use) count
** @valrule GetRoom [ajuint] Remaining space to extend string
**                           without reallocating
** @valrule GetValid [AjBool] True if string is valid
**
** @fcategory use
*/

/* @func ajStrGetCharFirst ****************************************************
**
** Returns the first character from a string.
**
** @param [r] str [const AjPStr] String
** @return [char] First character or null character if empty.
** @@
******************************************************************************/

char ajStrGetCharFirst(const AjPStr str)
{
    if(!str)
	return '\0';
    return str->Ptr[0];
}


/* @func ajStrGetCharLast ****************************************************
**
** Returns the last character from a string.
**
** @param [r] str [const AjPStr] String
** @return [char] Last character or null character if empty.
** @@
******************************************************************************/

char ajStrGetCharLast(const AjPStr str)
{
    if(!str)
	return '\0';
    if(!str->Len)
	return '\0';

    return str->Ptr[str->Len - 1];
}


/* @func ajStrGetCharPos ******************************************************
**
** Returns a single character at a given position from a string.
**
** @param [r] str [const AjPStr] String
** @param [r] pos [ajint] Position in the string, negative values are
**        from the end of the string.
** @return [char] Character at position pos or null character if out of range.
** @@
******************************************************************************/

char ajStrGetCharPos(const AjPStr str, ajint pos)
{
    ajuint ipos;

    if(!str)
	return '\0';

    if(pos < 0)
	ipos = str->Len + pos;
    else
	ipos = pos;

    if(ipos > str->Len)
	return '\0';

    return str->Ptr[ipos];
}


/* @obsolete ajStrChar
** @replace ajStrGetCharFirst (1,'0'/1)
** @replace ajStrGetCharLast (1,'-1'/1)
** @replace ajStrGetCharPos (1,2/1,2)
*/

__deprecated char  ajStrChar(const AjPStr thys, ajint pos)
{
    if(pos == 0)
	ajStrGetCharFirst(thys);
    if(pos == -1)
	ajStrGetCharLast(thys);
    
    return ajStrGetCharPos(thys, pos);
}

/* @func ajStrGetLen **********************************************************
**
** Returns the current length of the C (char *) string.
**
** The length will remain valid unless the string is resized or deleted.
**
** @param [r] str [const AjPStr] Source string
** @return [ajuint] Current string length
** @@
******************************************************************************/

ajuint ajStrGetLen(const AjPStr str)
{
    if(!str)
	return 0;

    else
	return str->Len;
}


/* @macro MAJSTRGETLEN ********************************************************
**
** Returns the current length of the C (char *) string. 
**
** A macro version of {ajStrGetLen} available in case it is needed for speed.
**
** @param [r] str [const AjPStr] Source string
** @return [ajuint] Current string length
** @@
******************************************************************************/




/* @obsolete ajStrLen
** @rename ajStrGetLen
*/

__deprecated ajint  ajStrLen(const AjPStr thys)
{
    return ajStrGetLen(thys);
}

/* @obsolete MAJSTRLEN
** @rename MAJSTRGETLEN
*/

__deprecated ajint  MAJSTRLEN(const AjPStr thys)
{
    return MAJSTRGETLEN(thys);
}

/* @func ajStrGetPtr **********************************************************
**
** Returns the current pointer to C (char *) string.
**
** The pointer will remain valid unless the string is resized or deleted.
**
** @param [r] str [const AjPStr] Source string
** @return [const char*] Current string pointer, or a null string if undefined.
** @@
******************************************************************************/

const char* ajStrGetPtr(const AjPStr str)
{
    if(!str)
	return charNULL;

    return str->Ptr;
}



/* @macro MAJSTRGETPTR ********************************************************
**
** Returns the current pointer to C (char *) string.
**
** A macro version of {ajStrGetPtr} available in case it is needed for speed.
**
** @param [r] str [const AjPStr] Source string
** @return [const char*] Current string pointer, or a null string if undefined.
** @@
******************************************************************************/




/* @obsolete ajStrStr
** @rename ajStrGetPtr
*/

__deprecated const char  *ajStrStr(const AjPStr thys)
{
    return ajStrGetPtr(thys);
}

/* @obsolete MAJSTRSTR
** @rename MAJSTRGETPTR
*/

__deprecated const char  *MAJSTRSTR(const AjPStr thys)
{
    return MAJSTRGETPTR(thys);
}

/* @func ajStrGetRes **********************************************************
**
** Returns the current reserved size of the C (char *) string. 
**
** The reserved size will remain valid unless the string is resized or deleted.
**
** @param [r] str [const AjPStr] Source string
** @return [ajuint] Current string reserved size
** @@
******************************************************************************/

ajuint ajStrGetRes(const AjPStr str)
{
    if(!str)
	return 0;

    return str->Res;
}



/* @macro MAJSTRGETRES ********************************************************
**
** Returns the current reserved size of the C (char *) string. 
** 
** A macro version of {ajStrGetRes} available in case it is needed for speed.
**
** @param [r] str [const AjPStr] Source string
** @return [ajint] Current string reserved size
** @@
******************************************************************************/




/* @obsolete ajStrSize
** @rename ajStrGetRes
*/
__deprecated ajint  ajStrSize(const AjPStr thys)
{
    return ajStrGetRes(thys);
}

/* @obsolete MAJSTRSIZE
** @rename MAJSTRGETRES
*/
__deprecated ajint  MAJSTRSIZE(const AjPStr thys)
{
    return MAJSTRGETRES(thys);
}

/* @func ajStrGetRoom *********************************************************
**
** Returns the additional space available in a string before it would require
** reallocating.
**
** @param [r] str [const AjPStr] String
** @return [ajuint] Space available for additional characters.
** @@
******************************************************************************/

ajuint ajStrGetRoom(const AjPStr str)
{
    return (str->Res - str->Len - 1);
}


/* @obsolete ajStrRoom
** @rename ajStrGetRoom
*/

__deprecated ajint  ajStrRoom(const AjPStr thys)
{
    return ajStrGetRoom(thys);
}

/* @func ajStrGetUse **********************************************************
**
** Returns the current usage count of the C (char *) string. 
**
** The usage count will remain valid unless the string is resized or deleted.
**
** @param [r] str [const AjPStr] Source string
** @return [ajuint] Current string usage count
** @@
******************************************************************************/

ajuint ajStrGetUse(const AjPStr str)
{
    if(!str)
	return 0;

    return str->Use;
}



/* @macro MAJSTRGETUSE ********************************************************
**
** Returns the current usage count of the C (char *) string. 
** 
** A macro version of {ajStrGetUse} available in case it is needed for speed.
**
** @param [r] str [const AjPStr] Source string
** @return [ajuint] Current string usage count
** @@
******************************************************************************/




/* @obsolete ajStrRef
** @rename ajStrGetUse
*/

__deprecated ajint  ajStrRef(const AjPStr thys)
{
    return ajStrGetUse(thys);
}

/* @obsolete MAJSTRREF
** @rename MAJSTRGETUSE
*/

__deprecated ajint  MAJSTRREF(const AjPStr thys)
{
    return MAJSTRGETUSE(thys);
}

/* @func ajStrGetValid ********************************************************
**
** Checks a string object for consistency. 
**
** Intended for debugging and testing of these routines, but made publicly 
** available.
**
** @param [r] str [const AjPStr] String
** @return [AjBool] ajTrue if no errors were found.
** @@
******************************************************************************/

AjBool ajStrGetValid(const AjPStr str)
{
    AjBool ret = ajTrue;

    if(!str)
    {
	ajErr("ajStrGetValid: NULL string pointer");
	ret = ajFalse;
    }

    if(str->Res < 1)
    {
	ajErr("ajStrGetValid: Bad size value %d", str->Res);
	ret = ajFalse;
    }

    if(str->Len >= str->Res)
    {
	ajErr("ajStrGetValid: Size %d too small for length %d\n",
	       str->Res, str->Len);
	ret = ajFalse;
    }

    if(!str->Ptr)
    {
	ajErr("ajStrGetValid: NULL pointer\n");
	ret = ajFalse;
    }
    else
    {
	if(str->Len != strlen(str->Ptr))
	{
	    ajErr("ajStrGetValid: Len %d differs from strlen %d\n",
		   str->Len, strlen(str->Ptr));
	    ret = ajFalse;
	}
    }

    return ret;
}




/* @section modifiable string retrieval ***************************************
**
** Functions for returning elements of a string object.
**
** @fdata       [AjPStr]
** @fnote      The last word of the ajStrGet family of functions is the
**             element variable name.
** @fnote      Each ajXxxGet function should be provided as a macro.
**
** @nam3rule   Getunique   Single copy strings
** @nam4rule   GetuniquePtr   Retrieve char* pointer as single copy
** @nam4rule   GetuniqueStr   Retrieve string pointer as single copy
**
** @argrule * Pstr [AjPStr*] String
**
** @valrule GetuniqueStr [AjPStr] Modifiable string pointer to internal string
** @valrule GetuniquePtr [char*] Modifiable C string pointer to internal string
**
** @fcategory modify
*/

/* @func ajStrGetuniquePtr ****************************************************
**
** Makes the string value unique by copying any string with a reference count
** of more than 1, and returns the current C (char *) pointer.
**
** As the input string is not const, the calling program must be able
** to modify it in order to call this function.
**
** If the length of the string is changed, it is the responsibility of
** the caller to reset it with ajStrSetValid
**
** @param [u] Pstr [AjPStr*] Source string
** @return [char*] Current string pointer, or a null string if undefined.
** @@
******************************************************************************/

char* ajStrGetuniquePtr(AjPStr *Pstr)
{
    AjPStr thys;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);
    thys = *Pstr;

    return thys->Ptr;
}




/* @obsolete ajStrStrMod
** @rename ajStrGetuniquePtr
*/

__deprecated char  *ajStrStrMod(AjPStr *pthis)
{
    return ajStrGetuniquePtr(pthis);
}

/* @macro MAJSTRGETUNIQUESTR **************************************************
**
** Make certain a string is modifiable by checking it has no other references, 
** or by making a new real copy of the string.
**
** Uses strCloneL to copy without copying the reference count.
**
** The target string is guaranteed to have a reference count of exactly 1.
**
** A macro version of {ajStrGetuniqueStr} in case it is needed for speed
**
** @param [u] Pstr [AjPStr*] String
** @return [AjPStr] The new string pointer, or NULL for failure
** @@
******************************************************************************/


/* @func ajStrGetuniqueStr ****************************************************
**
** Make certain a string is modifiable by checking it has no other references, 
** or by making a new real copy of the string.
**
** Uses strCloneL to copy without copying the reference count.
**
** The target string is guaranteed to have a reference count of exactly 1.
**
** @param [u] Pstr [AjPStr*] String
** @return [AjPStr] The new string pointer, or NULL for failure
** @@
******************************************************************************/

AjPStr ajStrGetuniqueStr(AjPStr* Pstr)
{
    AjPStr thys;

    if(!*Pstr)
    {
	*Pstr = ajStrNew();
	return *Pstr;
    }

    thys = *Pstr;
    if(thys->Use > 1)
    {
	strClone(Pstr);
    }

    return *Pstr;
}




/* @obsolete ajStrMod
** @rename ajStrGetUniqueStr
*/

__deprecated AjBool  ajStrMod(AjPStr* pthis)
{
    AjBool ret = ajTrue;
    if ((*pthis)->Use == 1)
	ret = ajFalse;
    ajStrGetuniqueStr(pthis);
    return ret;
}

/* @section element assignment ************************************************
**
** Functions for assigning elements of a string object.
**
** @fdata       [AjPStr]
** @fnote      The last word of the ajStrGet family of functions is the
**             element variable name.
** @fnote      Each ajXXXSet function should be provided as a macro.
** @nam3rule   Set       Set elements (attributes)  
** @nam4rule   SetClear  Clear all elements (empty string, length zero)
** @nam4rule   SetLen  
** @nam4rule   SetRes    Set (increase) reserved memory
** @nam5rule   SetResRound  Maximum rounded up by a standard amount (32)
** @nam4rule   SetUnique    Set reference count to 1, making a copy if needed
** @nam4rule   SetValid   Sets the string elements to be valid,
**                    for example adjusting the string length after editing.
**
** @argrule * Pstr [AjPStr*] String
** @argrule Res size [ajuint] Reserved size
** @argrule Len len [ajuint] Correct string length
**
** @valrule * [AjBool] True on success
**
** @fcategory modify
*/

/* @func ajStrSetClear ********************************************************
**
** Clears all elemements in a string object.
**
** If the string is already empty nothing happens. If the string has data, 
** it makes sure the string is modifiable and sets it to empty.
**
** @param  [w] Pstr [AjPStr*] Pointer to the string to be deleted.
**         The pointer is always deleted.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrSetClear(AjPStr* Pstr)
{
    AjPStr thys;

    if(!*Pstr)
        return ajTrue;
    
    thys = *Pstr;
    if(thys->Use > 1)
      thys  = ajStrGetuniqueStr(Pstr);

    thys->Ptr[0] = '\0';
    thys->Len = 0;

    return ajTrue;
}


/* @obsolete ajStrClear
** @rename ajStrSetClear
*/

__deprecated AjBool  ajStrClear(AjPStr* pthis)
{
    return ajStrSetClear(pthis);
}

/* @func ajStrSetRes **********************************************************
**
** Ensures a string is modifiable and big enough for its intended purpose.
**
** The target string is guaranteed to have a reference count of 1 and a 
** minimum reserved size.
**
** @param [u] Pstr [AjPStr*] String
** @param [r] size [ajuint] Minimum reserved size.
** @return [AjBool] ajTrue if the string was reallocated
** @@
******************************************************************************/

AjBool ajStrSetRes(AjPStr* Pstr, ajuint size)
{
    AjPStr thys;
    ajuint savesize;

    savesize = size; /* often part of *Pstr, about to vanish */

    if(!*Pstr)
    {
	*Pstr = ajStrNewRes(savesize);
	return ajTrue;
    }

    thys = *Pstr;

    if((thys->Use > 1) || (thys->Res < savesize))
    {
	strCloneL(Pstr, savesize);
	return ajTrue;
    }

    return ajFalse;
}



/* @obsolete ajStrModL
** @rename ajStrSetRes
*/
__deprecated AjBool  ajStrModL(AjPStr* pthis, size_t size)
{
    return ajStrSetRes(pthis, size);
}

/* @func ajStrSetResRound *****************************************************
**
** Ensures a string is modifiable and big enough for its intended purpose. 
**
** The target string is guaranteed to have a reference count of 1,
** and a minimum reserved size.  The string size is rounded up.
**
** @param [w] Pstr [AjPStr*] String
** @param [r] size [ajuint] Minimum reserved size.
** @return [AjBool] ajTrue if the string was reallocated
** @@
******************************************************************************/

AjBool ajStrSetResRound(AjPStr* Pstr, ajuint size)
{
    AjPStr thys;
    ajuint trysize;
    ajuint roundsize;

    if(!*Pstr)
    {
	roundsize = ajRound(size, STRSIZE);
	*Pstr = ajStrNewRes(roundsize);
	return ajTrue;
    }

    thys = *Pstr;

    if(thys->Res < size)
    {
	if(size >= LONGSTR)
	{
	    trysize = thys->Res;
	    while(trysize<size)
	      trysize+=trysize;
	  roundsize = ajRound(trysize, LONGSTR);
	}
	else
	  roundsize = ajRound(size, STRSIZE);

	strCloneL(Pstr, roundsize);
	return ajTrue;
    }

    if(thys->Use > 1)
    {
	strCloneL(Pstr, size);
	return ajTrue;
    }

    return ajFalse;
}



/* @obsolete ajStrModMinL
** @rename ajStrSetResRound
*/

__deprecated AjBool  ajStrModMinL(AjPStr* pthis, ajuint size)
{
    return ajStrSetResRound(pthis, size);
}

/* @func ajStrSetValid ********************************************************
**
** Reset string length when some nasty caller may have edited it
**
** @param [u] Pstr [AjPStr*] String.
** @return [AjBool] True on success.
** @@
******************************************************************************/

AjBool ajStrSetValid(AjPStr *Pstr)
{
    AjBool ret = ajTrue;
    AjPStr thys = *Pstr;

    if(thys->Use > 1)
    {
	ajWarn("ajStrSetValid called for string in use %d times\n", thys->Use);
	ret = ajFalse;
    }

    thys->Len = strlen(thys->Ptr);

    if(thys->Len > (thys->Res - 1))	/* ouch someone went off the end */
    {
	ajWarn("ajStrSetValid found string with length %d in size %d",
	       thys->Len, thys->Res);
	ret = ajFalse;
	ajStrSetResRound(Pstr, thys->Len+1);
    }

    return ret;
}



/* @obsolete ajStrFix
** @rename ajStrSetValid
*/

__deprecated void  ajStrFix(AjPStr *pthis)
{
    ajStrSetValid(pthis);
    return;
}

/* @func ajStrSetValidLen *****************************************************
**
** Reset string length when some nasty caller may have edited it
**
** @param [u] Pstr [AjPStr*] String
** @param [r] len [ajuint] Length expected.
** @return [AjBool] True on success
** @@
******************************************************************************/

AjBool ajStrSetValidLen(AjPStr* Pstr, ajuint len)
{
    AjBool ret = ajTrue;
    AjPStr thys = *Pstr;

    /* check 2 error conditions */

    if(thys->Use > 1)
    {
	ajWarn("ajStrSetValidLen called for string in use %d times\n",
	       thys->Use);
	ret = ajFalse;
    }

    if(len >= thys->Res)
    {
	ajWarn("ajStrSetValidLen called with length %d for string size %d\n",
	       len, thys->Res);
	thys->Ptr[thys->Res-1] = '\0';
	len = strlen(thys->Ptr);
	ret = ajFalse;
    }

    thys->Ptr[len] = '\0';

    thys->Len = len;

    return ret;
}



/* @obsolete ajStrFixI
** @rename ajStrSetValidLen
*/

__deprecated void  ajStrFixI(AjPStr* pthis, ajint ilen)
{
    ajStrSetValidLen(pthis, ilen);
    return;
}

/* @section string to datatype conversion functions ***************************
**
** Functions for converting strings to other datatypes.
**
** @fdata      [AjPStr]
** @fnote     None.
**
** @nam3rule  To            Convert string value to a different datatype.
** @nam4rule  ToBool        Convert to boolean
** @nam4rule  ToDouble        Convert to double
** @nam4rule  ToFloat        Convert to float
** @nam4rule  ToHex        Convert to hexadecimal
** @nam4rule  ToInt        Convert to integer
** @nam4rule  ToLong        Convert to long
** @nam4rule  ToUint        Convert to unsigned integer
**
** @argrule * str [const AjPStr] String
** @argrule ToBool Pval [AjBool*] Boolean return value
** @argrule ToDouble Pval [double*] Double return value
** @argrule ToFloat Pval [float*] Float return value
** @argrule ToHex Pval [ajint*] Integer return value
** @argrule ToInt Pval [ajint*] Integer return value
** @argrule ToLong Pval [ajlong*] Long integer return value
** @argrule ToUint Pval [ajuint*] Unsigned integer return value
**
** @valrule * [AjBool] True on success
**
** @fcategory cast
*/

/* @func ajStrToBool **********************************************************
**
** Converts a string into a Boolean value.
**
** @param [r] str [const AjPStr] String
** @param [w] Pval [AjBool*] ajTrue if the string is "true" as a boolean.
** @return [AjBool] ajTrue if the string had a valid boolean value.
** @cre an empty string returns ajFalse.
** @see ajStrIsBool
** @@
******************************************************************************/

AjBool ajStrToBool(const AjPStr str, AjBool* Pval)
{
    const char* cp;
    ajuint i;

    *Pval = ajFalse;

    if(!str)
	return ajFalse;

    if(str->Len < 1)
	return ajFalse;

    cp = str->Ptr;

    if(strchr("YyTt1", *cp))
    {
	*Pval = ajTrue;
	if(str->Len == 1)
	    return ajTrue;

	if(ajStrMatchCaseC(str, "yes"))
	    return ajTrue;

	if(ajStrMatchCaseC(str, "true"))
	    return ajTrue;

	return ajFalse;
    }
    else if(strchr("NnFf", *cp))
    {
	*Pval = ajFalse;
	if(!ajStrGetCharPos(str, 1))
	    return ajTrue;

	if(ajStrMatchCaseC(str, "no"))
	    return ajTrue;

	if(ajStrMatchCaseC(str, "false"))
	    return ajTrue;

	return ajFalse;
    }
    else if(strchr("123456789", *cp))
    {
	*Pval = ajTrue;
	if(ajStrIsFloat(str))
	    return ajTrue;

	return ajFalse;
    }
    else if(strchr("0+-", *cp))
    {
	i = strcspn(cp, "123456789");	/* e.g. 0.1, 0007 */
	if(cp[i])
	    *Pval = ajTrue;
	else
	    *Pval = ajFalse;
	if(ajStrIsFloat(str))
	    return ajTrue;

	return ajFalse;
    }

    return ajFalse;
}




/* @func ajStrToDouble ********************************************************
**
** Converts a string into a double precision value.
**
** Uses the strtod call in the C RTL.
**
** @param [r] str [const AjPStr] String
** @param [w] Pval [double*] String represented as a double precision number.
** @return [AjBool] ajTrue if the string had a valid double precision value.
** @cre an empty string returns ajFalse.
** @see ajStrIsInt
** @@
******************************************************************************/

AjBool ajStrToDouble(const AjPStr str, double* Pval)
{
    AjBool ret = ajFalse;
    const char* cp;
    double d;
    char* ptr = NULL;

    *Pval = 0.0;

    if(!str)
	return ret;

    if(!str->Len)
	return ret;

    cp = str->Ptr;

    errno = 0;
    d = strtod(cp, &ptr);

    if(!*ptr  && errno != ERANGE)
    {
	*Pval = d;
	ret = ajTrue;
    }

    return ret;
}




/* @func ajStrToFloat *********************************************************
**
** Converts a string into a floating point value.
**
** Uses the strtod call in the C RTL.
**
** @param [r] str [const AjPStr] String
** @param [w] Pval [float*] String represented as a floating point number.
** @return [AjBool] ajTrue if the string had a valid floating point value.
** @cre an empty string returns ajFalse.
** @see ajStrIsInt
** @@
******************************************************************************/

AjBool ajStrToFloat(const AjPStr str, float* Pval)
{
    AjBool ret = ajFalse;
    const char* cp;
    double d;
    char* ptr = NULL;

    *Pval = 0.0;

    if(!str)
	return ret;

    if(!str->Len)
	return ret;

    cp = str->Ptr;

    errno = 0;
    d = strtod(cp, &ptr);

    if(!*ptr  && errno != ERANGE)
    {
	if(d > FLT_MAX)
	    return ajFalse;

	if(d < -FLT_MAX)
	    return ajFalse;

	*Pval = (float) d;
	ret = ajTrue;
    }

    return ret;
}




/* @func ajStrToHex ***********************************************************
**
** Converts a string from hexadecimal into an integer value.
**
** Uses the strtol call in the C RTL.
**
** @param [r] str [const AjPStr] String
** @param [w] Pval [ajint*] String represented as an integer.
** @return [AjBool] ajTrue if the string had a valid hexadecimal value.
** @cre an empty string returns ajFalse.
** @see ajStrIsHex
** @@
******************************************************************************/

AjBool ajStrToHex(const AjPStr str, ajint* Pval)
{
    AjBool ret = ajFalse;
    const char* cp;
    ajlong l;
    char* ptr;

    *Pval = 0;

    if(!str)
	return ret;

    if(!str->Len)
	return ret;

    cp = str->Ptr;
    errno = 0;
    l = strtol(cp, &ptr, 16);
    if(!*ptr && errno != ERANGE)
    {
	l = AJMAX(INT_MIN, l);
	l = AJMIN(INT_MAX, l);
	*Pval =(ajint) l;
	ret = ajTrue;
    }

    return ret;
}




/* @func ajStrToInt ***********************************************************
**
** Converts a string into an integer value.
**
** Uses the strtol call in the C RTL.
**
** @param [r] str [const AjPStr] String
** @param [w] Pval [ajint*] String represented as an integer.
** @return [AjBool] ajTrue if the string had a valid integer value.
** @cre an empty string returns ajFalse.
** @see ajStrIsInt
** @@
******************************************************************************/

AjBool ajStrToInt(const AjPStr str, ajint* Pval)
{
    AjBool ret = ajFalse;
    const char* cp;
    ajlong l;
    char* ptr;

    *Pval = 0;

    if(!str)
	return ret;

    if(!str->Len)
	return ret;

    cp = str->Ptr;

    errno = 0;
    l = strtol(cp, &ptr, 10);
    if(!*ptr && errno != ERANGE)
    {
	l = AJMAX(INT_MIN, l);
	l = AJMIN(INT_MAX, l);
	*Pval =(ajint) l;
	ret = ajTrue;
    }

    return ret;
}




/* @func ajStrToLong **********************************************************
**
** Converts a string into an integer value.
**
** Uses the strtol call in the C RTL.
**
** @param [r] str [const AjPStr] String
** @param [w] Pval [ajlong*] String represented as an integer.
** @return [AjBool] ajTrue if the string had a valid integer value.
** @cre an empty string returns ajFalse.
** @see ajStrIsInt
** @@
******************************************************************************/

AjBool ajStrToLong(const AjPStr str, ajlong* Pval)
{
    AjBool ret = ajFalse;
    const char* cp;
    ajlong l;
    char* ptr;

    *Pval = 0;

    if(!str)
	return ret;

    if(!str->Len)
	return ret;

    cp = str->Ptr;

    errno = 0;
    l = strtol(cp, &ptr, 10);

    if(!*ptr && errno != ERANGE)
    {
	*Pval = l;
	ret = ajTrue;
    }

    return ret;
}




/* @func ajStrToUint **********************************************************
**
** Converts a string into an unsigned integer value.
**
** Uses the strtol call in the C RTL. There is a strtoul call in C89 onwards.
** Negative values are rejected.
**
** @param [r] str [const AjPStr] String
** @param [w] Pval [ajuint*] String represented as an unsigned integer.
** @return [AjBool] ajTrue if the string had a valid unsigned integer value.
** @cre an empty string returns ajFalse.
** @see ajStrIsInt
** @@
******************************************************************************/

AjBool ajStrToUint(const AjPStr str, ajuint* Pval)
{
    AjBool ret = ajFalse;
    const char* cp;
    ajlong l;
    char* ptr;

    *Pval = 0;

    if(!str)
	return ret;

    if(!str->Len)
	return ret;

    cp = str->Ptr;

    errno = 0;
    l = strtol(cp, &ptr, 10);
    if(l < 0)
	return ret;

    if(!*ptr && errno != ERANGE)
    {
	l = AJMAX(0, l);
	l = AJMIN(INT_MAX, l);
	*Pval =(ajuint) l;
	ret = ajTrue;
    }

    return ret;
}




/* @section datatype to string conversion functions ***************************
**
** Functions for converting datatypes to strings.
**
** @fdata      [AjPStr]
** @fnote     See also "String to datatype conversion functions: C-type (char*)
**            strings"
** @nam3rule  From          Convert a datatype value to a string.
** @nam4rule  FromBool        Convert from boolean
** @nam4rule  FromDouble        Convert from double
** @nam5rule  FromDoubleExp        Convert from double in exponential form
** @nam4rule  FromFloat        Convert from float
** @nam4rule  FromInt        Convert from integer
** @nam4rule  FromLong        Convert from long
** @nam4rule  FromUint        Convert from unsigned integer
**
** @argrule * Pstr [AjPStr*] String result
** @argrule FromBool val [AjBool] boolean value
** @argrule FromDouble val [double] Double value
** @argrule FromFloat val [float] Float value
** @argrule FromInt val [ajint] Integer value
** @argrule FromLong val [ajlong] Long integer value
** @argrule FromDouble precision [ajint] Floating point precision
** @argrule FromFloat precision [ajint] Floating point precision
** @argrule FromUint val [ajuint] Unsigned integer value
**
** @valrule * [AjBool] True on success
**
** @fcategory derive
*/

/* @func ajStrFromBool ********************************************************
**
** Converts a Boolean value into a 1-letter string. 
**
** Can be used to print boolean values, but the ajFmt library has better ways.
**
** @param [w] Pstr [AjPStr*] String to hold the result.
** @param [r] val [AjBool] Boolean value
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrFromBool(AjPStr* Pstr, AjBool val)
{
    AjBool ret = ajFalse;
    static char bool_y[] = "Y";
    static char bool_n[] = "N";

    if(val)
	ret = ajStrAssignC(Pstr, bool_y);
    else
	ret = ajStrAssignC(Pstr, bool_n);

    return ret;
}


/* @func ajStrFromDouble ******************************************************
**
** Converts a double precision value into a string. 
**
** The string size is set to be just large enough to hold the value.
**
** @param [w] Pstr [AjPStr*] Target string
** @param [r] val [double] Double precision value
** @param [r] precision [ajint] Precision (number of decimal places) to use.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrFromDouble(AjPStr* Pstr, double val, ajint precision)
{
    AjBool ret = ajFalse;
    long long i;
    char fmt[12];

    if(val)
	i = precision + ajNumLengthDouble(val) + 3;
    else
	i = precision + 4;

    ret = ajStrSetResRound(Pstr, (ajuint)i);

    sprintf(fmt, "%%.%df", precision);
    ajFmtPrintS(Pstr, fmt, val);

    return ret ;
}




/* @func ajStrFromDoubleExp ***************************************************
**
** Converts a double precision value into a string. 
**
** The string size is set to be just large enough to hold the value. Uses 
** exponential form.
**
** @param [w] Pstr [AjPStr*] Target string
** @param [r] val [double] Double precision value
** @param [r] precision [ajint] Precision (number of decimal places) to use.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrFromDoubleExp(AjPStr* Pstr, double val, ajint precision)
{
    AjBool ret = ajFalse;
    long long i;
    char fmt[12];

    if(val)
	i = precision + ajNumLengthDouble(val) + 8;
    else
	i = precision + 8;

    ret = ajStrSetResRound(Pstr, (ajuint)i);

    sprintf(fmt, "%%.%de", precision);
    ajFmtPrintS(Pstr, fmt, val);

    return ret ;
}


/* @obsolete ajStrFromDoubleE
** @rename ajStrFromDoubleExp
*/

__deprecated AjBool  ajStrFromDoubleE(AjPStr* Pstr, double val, ajint precision)
{
    return ajStrFromDoubleExp(Pstr, val, precision);
}

/* @func ajStrFromFloat *******************************************************
**
** Converts a floating point value into a string. 
**
** The string size is set to be just large enough to hold the value.
**
** @param [w] Pstr [AjPStr*] Target string
** @param [r] val [float] Floating point value
** @param [r] precision [ajint] Precision (number of decimal places) to use.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrFromFloat(AjPStr* Pstr, float val, ajint precision)
{
    AjBool ret = ajFalse;
    ajuint i;
    char fmt[12];

    if(val)
	i = precision + ajNumLengthFloat(val) + 4;
    else
	i = precision + 4;

    ret = ajStrSetResRound(Pstr, i);

    sprintf(fmt, "%%.%df", precision);
    ajFmtPrintS(Pstr, fmt, val);

    return ret;
}




/* @func ajStrFromInt *********************************************************
**
** Converts an integer value into a string. 
** 
** The string size is set to be just large enough to hold the value.
**
** @param [w] Pstr [AjPStr*] Target string
** @param [r] val [ajint] Integer value
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrFromInt(AjPStr* Pstr, ajint val)
{
    AjBool ret = ajFalse;
    ajuint i;

    if(val)
	i = ajNumLengthInt(val) + 2;
    else
	i = 2;

    if(val < 0)
	i++;

    ret = ajStrSetResRound(Pstr, i);

    ajFmtPrintS(Pstr, "%d", val);

    return ret ;
}




/* @func ajStrFromLong ********************************************************
**
** Converts an ajlong integer value into a string.
**
** The string size is set to be just large enough to hold the value.
**
** @param [w] Pstr [AjPStr*] Target string
** @param [r] val [ajlong] Long integer value
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrFromLong(AjPStr* Pstr, ajlong val)
{
    AjBool ret = ajFalse;
    ajlong i;

    if(val)
	i = ajNumLengthUint(val) + 2;
    else
	i = 2;

    if(val < 0)
	i++;

    ret = ajStrSetResRound(Pstr, i);

    ajFmtPrintS(Pstr, "%ld", (long)val);

    return ret;
}




/* @func ajStrFromUint ********************************************************
**
** Converts an unsigned integer value into a string. 
** 
** The string size is set to be just large enough to hold the value.
**
** @param [w] Pstr [AjPStr*] Target string
** @param [r] val [ajuint] Integer value
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrFromUint(AjPStr* Pstr, ajuint val)
{
    AjBool ret = ajFalse;
    ajuint i;
    AjPStr thys;

    if(val)
	i = ajNumLengthUint(val) + 1;
    else
	i = 2;

    ret = ajStrSetResRound(Pstr, i);
    thys = *Pstr;

    thys->Len = sprintf(thys->Ptr, "%u", val);
    ajFmtPrintS(Pstr, "%u", val);

    return ret ;
}




/* @section formatting ********************************************************
**
** Functions for formatting strings.
**
** @fdata      [AjPStr]
** @fnote     Same namrule as "String formatting functions: C-type (char*)
**            strings".
** @nam3rule  Fmt           Change the format of a string.
** @nam4rule  FmtBlock    Format in blocks
** @nam4rule  FmtLower    Convert to lower case.
** @nam5rule  FmtLowerSub Convert sub-string.
** @nam4rule  FmtQuote    Enclose in double quotes
** @nam4rule  FmtTitle    Convert first character of string to uppercase. 
** @nam4rule  FmtUpper    Convert to upper case.
** @nam5rule  FmtUpperSub Substring only
** @nam4rule  FmtWrap     Wrap with newlines
** @nam5rule  FmtWrapAt   Wrap with newlines at a preferred character
** @nam5rule  FmtWrapLeft Wrap with newlines and left margin of spaces
**
** @argrule * Pstr [AjPStr*] String
** @argrule FmtBlock len [ajuint] Block length
** @argrule Sub pos1 [ajint] Start position, negative value counts from end
** @argrule Sub pos2 [ajint] End position, negative value counts from end
** @argrule Wrap width [ajuint] Line length
** @argrule WrapAt ch [char] Preferred last character on line
** @argrule WrapLeft margin [ajuint] Left margin
** @argrule WrapLeft indent [ajuint] Indentation on later lines
**
** @valrule * [AjBool] True on success
**
** @fcategory modify
*/

/* @func ajStrFmtBlock ********************************************************
**
** Splits a string into words (blocks) of a given size by inserting spaces.
**
** Mainly intended for sequence ouput formats
**
** @param [u] Pstr [AjPStr*] String.
** @param [r] len [ajuint] Block size
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajStrFmtBlock(AjPStr* Pstr, ajuint len)
{
    ajuint i;
    char* cp;
    char* cq;
    AjPStr thys;
    ajuint j;

    i = (*Pstr)->Len + ((*Pstr)->Len-1)/len;
    ajStrSetResRound(Pstr, i+1);
    thys = *Pstr;

/*
    ajDebug("ajStrBlock len: %d blksize: %d i: %d\n", thys->Len, blksize, i);
*/

    cp = &thys->Ptr[thys->Len];
    cq = &thys->Ptr[i];
    for(j=thys->Len-1; j; j--)
    {
	*(--cq) = *(--cp);
	if(!(j%len))
	    *(--cq) = ' ';
    }
    thys->Ptr[i]='\0';
    thys->Len = i;

    ajStrTrace(thys);

/*
    ajDebug("result '%S'\n", thys);
*/

    return ajTrue;
}


/* @obsolete ajStrBlock
** @rename ajStrFmtBlock
*/

__deprecated AjBool  ajStrBlock(AjPStr* pthis, ajint blksize)
{
    return ajStrFmtBlock(pthis, blksize);
}

/* @func ajStrFmtLower ********************************************************
**
** Converts a string to lower case. 
**
** If the string has multiple references, a new string is made first.
**
** @param [u] Pstr [AjPStr*] String
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrFmtLower(AjPStr* Pstr)
{
    AjPStr thys;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);
    thys = *Pstr;

    ajCharFmtLower(thys->Ptr);

    return ajTrue;
}


/* @obsolete ajStrToLower
** @rename ajStrFmtLower
*/
__deprecated AjBool  ajStrToLower(AjPStr* pthis)
{
    return ajStrFmtLower(pthis);
}

/* @func ajStrFmtLowerSub *****************************************************
**
** Converts a substring of a string to lower case.
**
** @param [u] Pstr [AjPStr*] String
** @param [r] pos1 [ajint] start position for conversion
** @param [r] pos2 [ajint] end position for conversion
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrFmtLowerSub(AjPStr* Pstr, ajint pos1, ajint pos2)
{
    AjPStr thys;
    ajuint ibegin;
    ajuint iend;
    ajuint i;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);
    thys = *Pstr;

    ibegin = ajMathPos(thys->Len, pos1);
    iend = ajMathPosI(thys->Len, ibegin, pos2);
    for (i=ibegin; i<=iend;i++)
	thys->Ptr[i] = (char)tolower((ajint) thys->Ptr[i]);

    return ajTrue;
}




/* @obsolete ajStrToLowerII
** @rename ajStrFmtLowerSub
*/
__deprecated AjBool  ajStrToLowerII(AjPStr* pthis, ajint begin, ajint end)
{
    return ajStrFmtLowerSub(pthis, begin, end);
}

/* @func ajStrFmtQuote ********************************************************
**
** Ensures a string is quoted with double quotes.
**
** @param [u] Pstr [AjPStr *] string
**
** @return [AjBool] True if string is not empty.
**                  As it is at least quoted, it will always return true.
** @@
******************************************************************************/

AjBool ajStrFmtQuote(AjPStr* Pstr)
{
    if(ajStrGetCharFirst(*Pstr) != '"')
	ajStrInsertC(Pstr, 0, "\"");

    /* careful: could be an empty string that is now '"' */
    /* this is the only way to have a length of 1 */

    if((*Pstr)->Len == 1 || ajStrGetCharLast(*Pstr) != '"')
	ajStrAppendK(Pstr, '"');

    if(!(*Pstr)->Len) return ajFalse;

    return ajTrue;
}




/* @obsolete ajStrQuote
** @rename ajStrFmtQuote
*/

__deprecated void  ajStrQuote(AjPStr* s)
{
    ajStrFmtQuote(s);
}


/* @func ajStrFmtTitle ********************************************************
**
** Converts the first character of a string to upper case.
**
** @param [u] Pstr [AjPStr*] String
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrFmtTitle(AjPStr* Pstr)
{
    AjPStr thys;
    char* cp;

    ajStrFmtLower(Pstr);

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);
    thys = *Pstr;

    cp = thys->Ptr;

    while(*cp) {
	if(isalpha(*cp))
	{
	    *cp = (char) toupper((int)*cp);
	    return ajTrue;
	}
	cp++;
    }

    return ajFalse;
}




/* @obsolete ajStrToTitle
** @rename ajStrFmtTitle
*/

__deprecated AjBool  ajStrToTitle(AjPStr* pthis)
{
    return ajStrFmtTitle(pthis);
}



/* @func ajStrFmtUpper ********************************************************
**
** Converts a string to upper case. 
**
** If the string has multiple references, a new string is made first.
**
** @param [u] Pstr [AjPStr*] String
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrFmtUpper(AjPStr* Pstr)
{
    AjPStr thys;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);
    thys = *Pstr;

    ajCharFmtUpper(thys->Ptr);

    return ajTrue;
}



/* @obsolete ajStrToUpper
** @rename ajStrFmtUpper
*/

__deprecated AjBool  ajStrToUpper(AjPStr* pthis)
{
    return ajStrFmtUpper(pthis);
}

/* @func ajStrFmtUpperSub *****************************************************
**
** Converts a substring of a string to upper case.
**
** @param [u] Pstr [AjPStr*] String
** @param [r] pos1 [ajint] start position for conversion
** @param [r] pos2 [ajint] end position for conversion
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrFmtUpperSub(AjPStr* Pstr, ajint pos1, ajint pos2)
{
    AjPStr thys;
    ajuint ibegin;
    ajuint iend;
    ajuint i;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);
    thys = *Pstr;

    ibegin = ajMathPos(thys->Len, pos1);
    iend = ajMathPosI(thys->Len, ibegin, pos2);
    for (i=ibegin; i<=iend;i++)
	thys->Ptr[i] = (char)toupper((ajint) thys->Ptr[i]);

    return ajTrue;
}


/* @obsolete ajStrToUpperII
** @rename ajStrFmtUpperII
*/

__deprecated AjBool  ajStrToUpperII(AjPStr* pthis, ajint begin, ajint end)
{
    return ajStrFmtUpperSub(pthis, begin, end);
}

/* @func ajStrFmtWrap *********************************************************
**
** Formats a string so that it wraps when printed.  
**
** Newline characters are inserted, at white space if possible,
** with a break at whitespace following the preferred character
** if found, or at the last whitespace, or just at the line width if there
** is no whitespace found (it does happen with long hyphenated  enzyme names)
**
** @param [u] Pstr [AjPStr*] Target string
** @param [r] width [ajuint] Line width
** @return [AjBool] ajTrue on successful completion else ajFalse;
** @@
******************************************************************************/

AjBool ajStrFmtWrap(AjPStr* Pstr, ajuint width )
{
    AjPStr thys;
    char* cq;
    ajuint i;
    ajuint j;
    ajuint k;
    ajuint imax;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);
    thys = *Pstr;

    if(width > (*Pstr)->Len)		/* already fits on one line */
	return ajTrue;

    cq = thys->Ptr;
    i=0;
    imax = thys->Len - width;

    ajDebug("ajStrFmtWrap imax:%u len:%u '%S'\n",
	   imax, MAJSTRGETLEN(thys), *Pstr);

    while(i < imax)
    {
	j = i+width+1;
	if(j > thys->Len)
	    j = thys->Len;

	k = j;

	while(i < j)
	{
	    if(isspace((ajint)*cq))
	    {
		k = i;
		if(*cq == '\n')
		    break;
	    }
	    cq++;
	    i++;
	}

	if(*cq != '\n')
	{
	    if(k == j)
	    {
		ajStrInsertC(Pstr, k, "\n");
		imax++;
	    }
	    else
		thys->Ptr[k] = '\n';
	}

	i=k+1;
	cq=&thys->Ptr[i];
	ajDebug("k:%u len:%u i:%u imax:%u '%s'\n",
	       k, MAJSTRGETLEN(thys)-k-1, i, imax, &thys->Ptr[k+1]);
    }
    ajDebug("Done i:%u\n", i);
    return ajTrue;
}



/* @obsolete ajStrWrap
** @rename ajStrFmtWrap
*/

__deprecated AjBool  ajStrWrap(AjPStr* Pstr, ajint width )
{
    return ajStrFmtWrap(Pstr, width);
}

/* @func ajStrFmtWrapAt *******************************************************
**
** Formats a string so that it wraps when printed.
** Breaks are at a preferred character (for example ',' for author lists)
**
** Newline characters are inserted, at white space if possible,
** with a break at whitespace following the preferred character
** if found, or at the last whitespace, or just at the line width if there
** is no whitespace found (it does happen with long hyphenated  enzyme names)
**
** @param [u] Pstr [AjPStr*] Target string
** @param [r] width [ajuint] Line width
** @param [r] ch [char] Preferred last character on line
** @return [AjBool] ajTrue on successful completion else ajFalse;
** @@
******************************************************************************/

AjBool ajStrFmtWrapAt(AjPStr* Pstr, ajuint width, char ch)
{
    AjPStr thys;
    char* cq;
    ajuint i;
    ajuint j;
    ajuint k;
    ajuint kk;
    ajuint imax;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);
    thys = *Pstr;

    if(width > (*Pstr)->Len)		/* already fits on one line */
	return ajTrue;

    cq = thys->Ptr;
    i=0;
    imax = thys->Len - width;

    ajDebug("ajStrFmtWrapPref '%c' imax:%u len:%u '%S'\n",
	   ch, imax, MAJSTRGETLEN(thys), *Pstr);

    while(i < imax)
    {
	j = i+width+1;
	if(j > thys->Len)
	    j = thys->Len;

	k = j;
	kk = j;

	while(i < j)
	{
	    if(isspace((ajint)*cq))
	    {
		k = i;
		if(*cq == '\n')
		    break;
		if(i && thys->Ptr[i-1] == ch)
		    kk = i;
	    }
	    cq++;
	    i++;
	}

	if(*cq != '\n')
	{
	    if(kk < j)
	    {
		thys->Ptr[kk] = '\n';
		k = kk;
	    }
	    else if(k == j)
	    {
		ajStrInsertC(Pstr, k, "\n");
		imax++;
	    }
	    else
		thys->Ptr[k] = '\n';
	}

	i=k+1;
	cq=&thys->Ptr[i];
	ajDebug("k:%u len:%u i:%u imax:%u '%s'\n",
	       k, MAJSTRGETLEN(thys)-k-1, i, imax, &thys->Ptr[k+1]);
    }
    ajDebug("Done i:%u\n", i);
    return ajTrue;
}



/* @func ajStrFmtWrapLeft *****************************************************
**
** Formats a string so that it wraps and has a margin of space characters
** and an additional indent when printed.  
**
** Newline characters are inserted, at white space if possible, 
**
** @param [u] Pstr [AjPStr*] Target string
** @param [r] width [ajuint] Line width
** @param [r] margin [ajuint] Left margin
** @param [r] indent [ajuint] Left indentation on later lines
** @return [AjBool] ajTrue on successful completion else ajFalse;
** @@
******************************************************************************/

AjBool ajStrFmtWrapLeft(AjPStr* Pstr, ajuint width,
			ajuint margin, ajuint indent)
{
    AjPStr newstr = NULL;
    char* cp;
    ajuint len;
    ajuint i   = 0;
    ajuint j;
    ajuint isp = 0;
    ajuint leftmargin = margin;
    ajuint maxwidth = width + indent;

   /* ajDebug("ajStrFmtWrapLeft %d %d %d\n'%S'\n",
	   width, margin, indent, *Pstr); */

    len = 1 + (*Pstr)->Len + (indent + margin + 1) * (*Pstr)->Len / width;
    ajStrAssignS(&newstr, *Pstr);
    ajStrAssignResC(Pstr, len, "");
    ajStrAppendCountK(Pstr, ' ', margin);

    for(cp = newstr->Ptr; *cp; cp++)
    {
	switch(*cp)
	{
	case '\n':
	    ajStrAppendK(Pstr, '\n');
	    for(j=0; j<leftmargin; j++)
		ajStrAppendK(Pstr, ' ');
	    i = indent;
	    isp = 0;
	    leftmargin = margin + indent;
	    break;
	case ' ':
	case '\t':
	    isp = (*Pstr)->Len;
	    /* ajDebug("can split at %d\n", isp); */
	default:
	    if(++i >= maxwidth)
	    {	/* too wide, time to split */
		/* ajDebug("split at i: %d isp: %d\n'%S'\n",
		   i, isp, *Pstr); */
		if(isp)
		{
		    if(isp == (*Pstr)->Len)
			ajStrAppendK(Pstr, '\n');
		    else
			(*Pstr)->Ptr[isp] = '\n';
		    leftmargin = margin + indent;
		}
		else
		{
		    ajStrAppendK(Pstr, *cp); /* keep going */
		    break;
		}

		for(j=0; j<leftmargin; j++)
		{	  /* follow newline with left margin spaces */
		    isp++;
		    ajStrInsertK(Pstr, isp, ' ');
		}
		i = (*Pstr)->Len - isp + indent;
		isp = 0;

		if(!isspace((ajint)*cp))
		    ajStrAppendK(Pstr, *cp);
	    }
	    else
		ajStrAppendK(Pstr, *cp);

	    break;
	}
    }
    
    ajStrDel(&newstr);

    return ajTrue;
}


/* @obsolete ajStrWrapLeft
** @rename ajStrFmtWrapLeft
*/

__deprecated AjBool  ajStrWrapLeft(AjPStr* pthis, ajint width, ajint left)
{
    return ajStrFmtWrapLeft(pthis, width, 0, left);
}

/* @section comparison ********************************************************
**
** Functions for comparing strings 
**
** @fdata      [AjPStr]
**
** @nam3rule  Match          Compare two complete strings.
** @nam4rule  MatchCase      Case-insensitive comparison.
** @nam4rule  MatchWild      Comparison using wildcard characters.
** @nam5rule  MatchWildCase  Case-insensitive comparison
**                           using wildcard characters.
** @nam5rule  MatchWildWord  Case-sensitive wildcard comparison of 
**                           first words within two strings.
** @nam6rule  MatchWildWordCase  Case-insensitive wildcard comparison of 
**                           first words within two strings.
** @nam4rule  MatchWord      Comparison using whole words.
** @nam5rule  MatchWordOne   Comparison using whole words matching any one.
** @nam5rule  MatchWordAll   Comparison using whole words matching every one.

** @nam3rule  Prefix         Compare start of string to given prefix.
** @nam4rule  PrefixCase     Case-insensitive comparison.
** @nam3rule  Suffix         Compare end of string to given suffix.
** @nam4rule  SuffixCase     Case-insensitive comparison.
**
** @argrule * str [const AjPStr] String
** @argrule C txt2 [const char*] String
** @argrule S str2 [const AjPStr] String
**
** @valrule * [AjBool] True on success
**
** @fcategory use
*/



/* @func ajStrMatchC **********************************************************
**
** Simple test for matching a string and a text string.
**
** @param [r] str [const AjPStr] String
** @param [r] txt2 [const char*] Text
** @return [AjBool] ajTrue if two complete strings are the same
** @@
******************************************************************************/

AjBool ajStrMatchC(const AjPStr str, const char* txt2)
{
    if(!str || !txt2)
	return ajFalse;

    if(!strcmp(str->Ptr, txt2))
	return ajTrue;

    return ajFalse;
}




/* @func ajStrMatchS **********************************************************
**
** Simple test for matching two strings. 
**
** @param [r] str [const AjPStr] String
** @param [r] str2 [const AjPStr] Second String
** @return [AjBool] ajTrue if two complete strings are the same
** @@
******************************************************************************/

AjBool ajStrMatchS(const AjPStr str, const AjPStr str2)
{
    if(!str || !str2)
	return ajFalse;

    if(!strcmp(str->Ptr, str2->Ptr))
	return ajTrue;

    return ajFalse;
}


/* @obsolete ajStrMatch
** @rename ajStrMatchS
*/
__deprecated AjBool  ajStrMatch(const AjPStr str, const AjPStr str2)
{
    return ajStrMatchS(str, str2);
}

/* @func ajStrMatchCaseC ******************************************************
**
** Simple case-insensitive test for matching a string and a text string.
**
** @param [r] str [const AjPStr] String
** @param [r] txt2 [const char*] Text
** @return [AjBool] ajTrue if two strings are exactly the same excluding case
** @@
******************************************************************************/

AjBool ajStrMatchCaseC(const AjPStr str, const char* txt2)
{
    if(!str || !txt2)
	return ajFalse;

    return ajCharMatchCaseC(str->Ptr, txt2);
}




/* @func ajStrMatchCaseS ******************************************************
**
** Simple case-insensitive test for matching two strings.
**
** @param [r] str [const AjPStr] String
** @param [r] str2 [const AjPStr] Second String
** @return [AjBool] ajTrue if two strings are exactly the same excluding case
** @@
******************************************************************************/

AjBool ajStrMatchCaseS(const AjPStr str, const AjPStr str2)
{
    if(!str || !str2)
	return ajFalse;

    return ajCharMatchCaseC(str->Ptr, str2->Ptr);
}

/* @obsolete ajStrMatchCase
** @rename ajStrMatchCaseS
*/

__deprecated AjBool  ajStrMatchCase(const AjPStr str, const AjPStr str2)
{
    return ajStrMatchCaseS(str, str2);
}


/* @func ajStrMatchWildC ******************************************************
**
** Simple case-sensitive test for matching a string and a text string using
** wildcard characters.
**
** @param [r] str [const AjPStr] String
** @param [r] txt2 [const char*] Wildcard text
** @return [AjBool] ajTrue if the strings match
** @@
******************************************************************************/

AjBool ajStrMatchWildC(const AjPStr str, const char* txt2)
{
    return ajCharMatchWildC(str->Ptr, txt2);
}




/* @func ajStrMatchWildS ******************************************************
**
** Simple case-sensitive test for matching two strings using wildcard 
** characters.
**
** @param [r] str [const AjPStr] String
** @param [r] str2 [const AjPStr] Wildcard string
** @return [AjBool] ajTrue if two strings match
** @@
******************************************************************************/

AjBool ajStrMatchWildS(const AjPStr str, const AjPStr str2)
{
    return ajCharMatchWildC(str->Ptr, str2->Ptr);
}


/* @obsolete ajStrMatchWild
** @rename ajStrMatchWildS
*/

__deprecated AjBool  ajStrMatchWild(const AjPStr str, const AjPStr str2)
{
    return ajStrMatchWildS(str, str2);
}




/* @func ajStrMatchWildCaseC **************************************************
**
** Simple case-insensitive test for matching a string and a text string using
** wildcard characters.
**
** @param [r] str [const AjPStr] String
** @param [r] txt2 [const char*] Wildcard text
** @return [AjBool] ajTrue if the strings match
** @@
******************************************************************************/

AjBool ajStrMatchWildCaseC(const AjPStr str, const char* txt2)
{
    return ajCharMatchWildCaseC(str->Ptr, txt2);
}




/* @func ajStrMatchWildCaseS **************************************************
**
** Simple case-insensitive test for matching two strings using wildcard 
** characters.
**
** @param [r] str [const AjPStr] String
** @param [r] str2 [const AjPStr] Wildcard string
** @return [AjBool] ajTrue if two strings match
** @@
******************************************************************************/

AjBool ajStrMatchWildCaseS(const AjPStr str, const AjPStr str2)
{
    return ajCharMatchWildCaseC(str->Ptr, str2->Ptr);
}


/* @func ajStrMatchWildWordC **************************************************
**
** Case-sensitive test for matching a text string "word" against a string
** using wildcard characters. 
**
** 'Word' is defined as starting and ending with an alphanumeric character
** (A-Z a-z 0-9) with no white space.
**
** The query text can use '*' or '?' as a wildcard.
**
** @param [r] str [const AjPStr] String
** @param [r] txt2 [const char*] Text
** @return [AjBool] ajTrue if found
** @@
******************************************************************************/

AjBool ajStrMatchWildWordC(const AjPStr str, const char* txt2)
{
    return ajCharMatchWildWordC(str->Ptr, txt2);
}




/* @func ajStrMatchWildWordS **************************************************
**
** Case-sensitive test for matching a string "word" against a string
** using wildcard characters. 
**
** 'Word' is defined as starting and ending with an alphanumeric character
** (A-Z a-z 0-9) with no white space.
**
** The query text can use '*' or '?' as a wildcard.
**
** @param [r] str [const AjPStr] String
** @param [r] str2 [const AjPStr] Text
** @return [AjBool]  ajTrue if found
** @@
******************************************************************************/

AjBool ajStrMatchWildWordS(const AjPStr str, const AjPStr str2)
{
    return ajCharMatchWildWordC(str->Ptr, str2->Ptr);
}



/* @func ajStrMatchWildWordCaseC **********************************************
**
** Case-insensitive test for matching a text string "word" against a string
** using wildcard characters. 
**
** 'Word' is defined as starting and ending with an alphanumeric character
** (A-Z a-z 0-9) with no white space.
**
** The query text can use '*' or '?' as a wildcard.
**
** @param [r] str [const AjPStr] String
** @param [r] txt2 [const char*] Text
** @return [AjBool] ajTrue if found
** @@
******************************************************************************/

AjBool ajStrMatchWildWordCaseC(const AjPStr str, const char* txt2)
{
    return ajCharMatchWildWordCaseC(str->Ptr, txt2);
}




/* @func ajStrMatchWildWordCaseS **********************************************
**
** Case-insensitive test for matching a string "word" against a string
** using wildcard characters. 
**
** 'Word' is defined as starting and ending with an alphanumeric character
** (A-Z a-z 0-9) with no white space.
**
** The query text can use '*' or '?' as a wildcard.
**
** @param [r] str [const AjPStr] String
** @param [r] str2 [const AjPStr] Text
** @return [AjBool]  ajTrue if found
** @@
******************************************************************************/

AjBool ajStrMatchWildWordCaseS(const AjPStr str, const AjPStr str2)
{
    return ajCharMatchWildWordCaseC(str->Ptr, str2->Ptr);
}



/* @obsolete ajStrMatchWord
** @rename ajStrMatchWildWordS
*/

__deprecated AjBool  ajStrMatchWord(const AjPStr str, const AjPStr str2)
{
    return ajStrMatchWildWordS(str, str2);
}




/* @func ajStrMatchWordAllS **************************************************
**
** Test for matching all words within a string.
** Matches can be to parts of words in the original string.
**
** 'Word' is defined as no white space.
**
** @param [r] str [const AjPStr] String
** @param [r] str2 [const AjPStr] Text
** @return [AjBool]  ajTrue if found
** @@
******************************************************************************/

AjBool ajStrMatchWordAllS(const AjPStr str, const AjPStr str2)
{
    AjBool ret = ajTrue;
    const AjPStr teststr = NULL;

    teststr = ajStrParseWhite(str2);

    if(!teststr)
	return ajFalse;

    while (teststr)
    {
	if(ajStrFindS(str, teststr) == -1)
	    ret = ajFalse;
	teststr = ajStrParseWhite(NULL);
    }
    return ret;
}



/* @func ajStrMatchWordOneS **************************************************
**
** Test for matching a word within a string.
** Matches can be to parts of words in the original string.
**
** 'Word' is defined as no white space.
**
** @param [r] str [const AjPStr] String
** @param [r] str2 [const AjPStr] Text
** @return [AjBool]  ajTrue if found
** @@
******************************************************************************/

AjBool ajStrMatchWordOneS(const AjPStr str, const AjPStr str2)
{
    AjBool ret = ajFalse;
    const AjPStr teststr = NULL;

    teststr = ajStrParseWhite(str2);

    if(!teststr)
	return ajFalse;

    while (teststr)
    {
	if(ajStrFindS(str, teststr) != -1)
	    ret = ajTrue;
	teststr = ajStrParseWhite(NULL);
    }
    return ret;
}



/* @func ajStrPrefixC *********************************************************
**
** Test for matching the start of a string against a given prefix text string.
**
** @param [r] str [const AjPStr] String
** @param [r] txt2 [const char*] Prefix as text
** @return [AjBool] ajTrue if the string begins with the prefix
** @@
******************************************************************************/

AjBool ajStrPrefixC(const AjPStr str, const char* txt2)
{
    ajuint ilen;

    if(!str)
	return ajFalse;

    ilen = strlen(txt2);

    if(!ilen)				/* no prefix */
	return ajFalse;

    if(ilen > MAJSTRGETLEN(str))		/* pref longer */
	return ajFalse;

    if(strncmp(str->Ptr, txt2, ilen))	/* +1 or -1 for a failed match */
	return ajFalse;

    return ajTrue;
}




/* @func ajStrPrefixS *********************************************************
**
** Test for matching the start of a string against a given prefix string.
**
** @param [r] str [const AjPStr] String
** @param [r] str2 [const AjPStr] Prefix
** @return [AjBool] ajTrue if the string begins with the prefix
** @@
******************************************************************************/

AjBool ajStrPrefixS(const AjPStr str, const AjPStr str2)
{
    if(!str)
	return ajFalse;

    if(!str2)
	return ajFalse;

    if(!str2->Len)			/* no prefix */
	return ajFalse;

    if(str2->Len > MAJSTRGETLEN(str))	/* pref longer */
	return ajFalse;

    if(strncmp(str->Ptr, str2->Ptr, str2->Len)) /* +1 or -1 for a
						     failed match */
	return ajFalse;

    return ajTrue;
}

/* @obsolete ajStrPrefix
** @rename ajStrPrefixS
*/

__deprecated AjBool  ajStrPrefix(const AjPStr str, const AjPStr str2)
{
    return ajStrPrefixS(str, str2);
}



/* @func ajStrPrefixCaseC *****************************************************
**
** Case-insensitive test for matching the start of a string against a 
** given prefix text string.
**
** @param [r] str [const AjPStr] String
** @param [r] txt2 [const char*] Prefix
** @return [AjBool] ajTrue if the string begins with the prefix
** @@
******************************************************************************/

AjBool ajStrPrefixCaseC(const AjPStr str, const char* txt2)
{
    return ajCharPrefixCaseC(str->Ptr, txt2);
}




/* @func ajStrPrefixCaseS *****************************************************
**
** Case-insensitive test for matching the start of a string against a 
** given prefix string.
**
** @param [r] str [const AjPStr] String
** @param [r] str2 [const AjPStr] Prefix
** @return [AjBool] ajTrue if the string begins with the prefix
** @@
******************************************************************************/

AjBool ajStrPrefixCaseS(const AjPStr str, const AjPStr str2)
{
    return ajCharPrefixCaseC(str->Ptr, str2->Ptr);
}


/* @obsolete ajStrPrefixCase
** @rename ajStrPrefixCaseS
*/

__deprecated AjBool  ajStrPrefixCase(const AjPStr str, const AjPStr str2)
{
    return ajStrPrefixCaseS(str,str2);
}




/* @func ajStrSuffixC *********************************************************
**
** Test for matching the end of a string against a given suffix text string.
**
** @param [r] str [const AjPStr] String
** @param [r] txt2 [const char*] Suffix as text
** @return [AjBool] ajTrue if the string ends with the suffix
** @@
******************************************************************************/

AjBool ajStrSuffixC(const AjPStr str, const char* txt2)
{
    ajuint ilen;
    ajuint istart;

    if(!str)
	return ajFalse;

    ilen   = strlen(txt2);
    istart = str->Len - ilen;

    if(ilen > MAJSTRGETLEN(str))		/* suff longer */
	return ajFalse;

    if(strncmp(&str->Ptr[istart], txt2, ilen)) /* +1 or -1 for a
						    failed match */
	return ajFalse;

    return ajTrue;
}




/* @func ajStrSuffixS *********************************************************
**
** Test for matching the end of a string against a given suffix string.
** 
** @param [r] str [const AjPStr] String
** @param [r] str2 [const AjPStr] Suffix
** @return [AjBool] ajTrue if the string ends with the suffix
** @@
******************************************************************************/

AjBool ajStrSuffixS(const AjPStr str, const AjPStr str2)
{
    ajuint ilen;
    ajuint istart;

    if(!str)
	return ajFalse;

    ilen   = MAJSTRGETLEN(str2);
    istart = str->Len - ilen;

    if(ilen > MAJSTRGETLEN(str))		/* suffix longer */
	return ajFalse;

    if(strncmp(&str->Ptr[istart], str2->Ptr, ilen)) /* +1 or -1 for a
						     failed match */
	return ajFalse;

    return ajTrue;
}



/* @obsolete ajStrSuffix
** @rename ajStrSuffixS
*/

__deprecated AjBool  ajStrSuffix(const AjPStr str, const AjPStr str2)
{
    return ajStrSuffixS(str, str2);
}

/* @func ajStrSuffixCaseC *****************************************************
**
** Case-insensitive test for matching the end of a string against a 
** given suffix text string.
**
** @param [r] str [const AjPStr] String
** @param [r] txt2 [const char*] Prefix
** @return [AjBool] ajTrue if the string ends with the suffix
** @@
******************************************************************************/

AjBool ajStrSuffixCaseC(const AjPStr str, const char* txt2)
{
    return ajCharSuffixCaseC(str->Ptr, txt2);
}




/* @func ajStrSuffixCaseS *****************************************************
**
** Case-insensitive test for matching the end of a string against a 
** given suffix string.
**
** @param [r] str [const AjPStr] String
** @param [r] str2 [const AjPStr] Prefix
** @return [AjBool] ajTrue if the string ends with the suffix
** @@
******************************************************************************/

AjBool ajStrSuffixCaseS(const AjPStr str, const AjPStr str2)
{
    return ajCharSuffixCaseC(str->Ptr, str2->Ptr);
}


/* @section comparison (sorting) **********************************************
**
** Functions for sorting strings.
**
** @fdata      [AjPStr]
** @fnote     namrule same as "String comparison (sorting) functions (C-type
**            char* strings)".
** @nam3rule  Cmp           Compare two complete strings & return sort order.
** @nam4rule  CmpCase     Case-insensitive comparison.
** @nam4rule  CmpWild     Comparison using wildcard characters.
** @nam5rule  CmpWildCase Case-insensitive comparison
**                         using wildcard characters.
** @nam3rule  Vcmp     Compare string using void arguments.
**
** @argrule Cmp str [const AjPStr] String to compare
** @argrule C txt2 [const char*] String to compare
** @argrule S str2 [const AjPStr] String to compare
** @argrule Len len [ajuint] Number of characters to compare
** @argrule Vcmp str [const void*] String
** @argrule Vcmp str2 [const void*] String to compare
**
** @valrule * [int] -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
**
** @fcategory use
*/



/* @macro MAJSTRCMPC **********************************************************
**
** Finds the sort order of a string and a text string.
**
** A macro version of {ajStrCmpC} in case it is needed for speed
**
** @param [r] str [const AjPStr] String object
** @param [r] txt2 [const char*] Text string
** @return [int] -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
** @@
******************************************************************************/

/* @func ajStrCmpC ************************************************************
**
** Finds the sort order of a string and a text string.
**
** @param [r] str [const AjPStr] String object
** @param [r] txt2 [const char*] Text string
** @return [int] -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
** @@
******************************************************************************/

int ajStrCmpC(const AjPStr str, const char* txt2)
{
    return strcmp(str->Ptr, txt2);
}



/* @func ajStrCmpLenC *********************************************************
**
** Finds the sort order of a string and a text string comparing the first 
** n characters only.
**
** @param [r] str [const AjPStr] String object
** @param [r] txt2 [const char*] Text string
** @param [r] len [ajuint] Length to compare
** @return [int] -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
** @@
******************************************************************************/

int ajStrCmpLenC(const AjPStr str, const char* txt2, ajuint len)
{
    return strncmp(str->Ptr, txt2, len);
}




/* @obsolete ajStrNCmpC
** @rename ajStrCmpLenC
*/

__deprecated int  ajStrNCmpC(const AjPStr str, const char* txt, ajint len)
{
    return ajStrCmpLenC(str, txt, len);
}


/* @macro MAJSTRCMPS **********************************************************
**
** Finds the sort order of two strings.
**
** A macro version of {ajStrCmpS} in case it is needed for speed
**
** @param [r] str [const AjPStr] First string
** @param [r] str2 [const AjPStr] Second string
** @return [int] -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
** @@
******************************************************************************/

/* @func ajStrCmpS ************************************************************
**
** Finds the sort order of two strings. 
**
** @param [r] str [const AjPStr] First string
** @param [r] str2 [const AjPStr] Second string
** @return [int] -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
** @@
******************************************************************************/

int ajStrCmpS(const AjPStr str, const AjPStr str2)
{
    return strcmp(str->Ptr, str2->Ptr);
}





/* @obsolete ajStrCmpO
** @rename ajStrCmpS
*/

__deprecated int  ajStrCmpO(const AjPStr thys, const AjPStr anoth)
{
    return ajStrCmpS(thys, anoth);
}

/* @func ajStrCmpCaseS ********************************************************
**
** Finds the sort order of two strings using a case-insensitive comparison.
**
** @param [r] str [const AjPStr] text string
** @param [r] str2 [const AjPStr] Text string
** @return [int] -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
** @@
******************************************************************************/

int ajStrCmpCaseS(const AjPStr str, const AjPStr str2)
{
    const char* cp;
    const char* cq;

    for(cp = str->Ptr, cq = str2->Ptr; *cp && *cq; cp++, cq++)
	if(toupper((ajint) *cp) != toupper((ajint) *cq))
	{
	    if(toupper((ajint) *cp) > toupper((ajint) *cq))
		return 1;
	    else
		return -1;
	}

    if(*cp)
	return 1;

    if(*cq)
	return -1;

    return 0;
}

/* @obsolete ajStrCmpCase
** @rename ajStrCmpCaseS
*/

__deprecated int  ajStrCmpCase(const AjPStr str, const AjPStr str2)
{
    return ajStrCmpCaseS(str, str2);
}

/* @func ajStrCmpLenS *********************************************************
**
** Finds the sort order of two strings comparing the first
** n characters only.

** Compares the first n characters of two strings
**
** @param [r] str [const AjPStr] String object
** @param [r] str2 [const AjPStr] Second string object
** @param [r] len [ajuint] Length to compare
** @return [int] -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
** @@
******************************************************************************/

int ajStrCmpLenS(const AjPStr str, const AjPStr str2, ajuint len)
{
    return strncmp(str->Ptr, str2->Ptr, len);
}


/* @obsolete ajStrNCmpO
** @rename ajStrCmpLenS
*/

__deprecated int  ajStrNCmpO(const AjPStr str, const AjPStr str2, ajint len)
{
    return ajStrCmpLenS(str, str2, len);
}


/* @func ajStrCmpWildC ********************************************************
**
** Finds the sort-order (case insensitive) of string and a text string using 
** wildcard characters. 
**
** @param [r] str [const AjPStr] String
** @param [r] txt2 [const char*] Wildcard text
** @return [int]  -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
** @@
******************************************************************************/

int ajStrCmpWildC(const AjPStr str, const char* txt2)
{
    return ajCharCmpWild(str->Ptr, txt2);
}




/* @func ajStrCmpWildS ********************************************************
**
** Finds the sort-order (case insensitive) of two strings using wildcard 
** characters. 
**
** @param [r] str [const AjPStr] String
** @param [r] str2 [const AjPStr] Wildcard string
** @return [int]  -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
** @@
******************************************************************************/

int ajStrCmpWildS(const AjPStr str, const AjPStr str2)
{
    return ajCharCmpWild(str->Ptr, str2->Ptr);
}

/* @func ajStrCmpWildCaseC ****************************************************
**
** Finds the sort-order (case sensitive) of string and a text string using 
** wildcard characters. 
**
** @param [r] str [const AjPStr] String
** @param [r] txt2 [const char*] Wildcard text
** @return [int]  -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
** @@
******************************************************************************/

int ajStrCmpWildCaseC(const AjPStr str, const char* txt2)
{
    return ajCharCmpWildCase(str->Ptr, txt2);
}




/* @func ajStrCmpWildCaseS ****************************************************
**
** Finds the sort-order (case sensitive) of two strings using wildcard 
** characters. 
**
** @param [r] str [const AjPStr] String
** @param [r] str2 [const AjPStr] Wildcard string
** @return [int]  -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
** @@
******************************************************************************/

int ajStrCmpWildCaseS(const AjPStr str, const AjPStr str2)
{
    return ajCharCmpWildCase(str->Ptr, str2->Ptr);
}

/* @obsolete ajStrCmpWild
** @rename ajStrCmpWildS
*/


__deprecated int  ajStrCmpWild(const AjPStr str, const AjPStr str2)
{
    return ajStrCmpWildS(str, str2);
}



/* @func ajStrVcmp *********************************************************
**
** Finds the sort order of two strings cast as void.  
**
** For use with {ajListSort}, {ajListUnique} and other functions which 
** require a general function with void arguments.
**
** @param [r] str [const void*] First string
** @param [r] str2 [const void*] Second string
** @return [int] -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
** @@
******************************************************************************/

int ajStrVcmp(const void* str, const void* str2)
{
    return strcmp((*(AjPStr const *)str)->Ptr, (*(AjPStr const *)str2)->Ptr);
}


/* @obsolete ajStrCmp
** @rename ajStrVcmp
*/

__deprecated int  ajStrCmp(const void* str, const void* str2)
{
    return ajStrVcmp(str, str2);
}



/* @section comparison (search) functions *************************************
**
** Functions for finding substrings or characters in strings.
**
** @fdata      [AjPStr]
** @fnote     None.
**
** @nam3rule  Find          Locate first occurence of a string
**                          within another string. 
** @nam4rule  FindAny       Any in a set of characters from the start
** @nam4rule  FindCase      Case insensitive
** @nam4rule  FindNext      Next in a set of characters from a given position
** @nam4rule  FindRest      Any not in a set of characters
** @nam5rule  FindRestCase  Any not in a set of characters, case insensitive
** @nam3rule  Findlast      Locate last occurence of a string
**
** @argrule * str [const AjPStr] String
** @argrule FindNext pos1 [ajint] String position to search from
** @argrule C txt2 [const char*] Text to find
** @argrule K chr [char] Character
** @argrule S str2 [const AjPStr] Text to find
**
** @valrule * [ajint] Position of the start of text in string if found.
**                Or -1 for text not found.
**
** @fcategory use
*/



/* @func ajStrFindC ***********************************************************
**
** Finds the first occurrence in a string of a second (text) string.
**
** @param [r] str [const AjPStr] String
** @param [r] txt2 [const char*] text to find
** @return [ajint] Position of the start of text in string if found.
**                Or -1 for text not found.
** @@
******************************************************************************/

ajint ajStrFindC(const AjPStr str, const char* txt2)
{
    const char* cp;

    cp = strstr(str->Ptr, txt2);
    if(!cp)
	return -1;

    return(cp - str->Ptr);
}




/* @func ajStrFindS ***********************************************************
**
** Finds the first occurrence in a string of a second string.
**
** @param [r] str [const AjPStr] String
** @param [r] str2 [const AjPStr] text to find
** @return [ajint] Position of the start of text in string if found.
**                Or -1 for text not found.
** @@
******************************************************************************/

ajint ajStrFindS(const AjPStr str, const AjPStr str2)
{
    const char* cp;

    cp = strstr(str->Ptr, str2->Ptr);
    if(!cp)
	return -1;

    return (cp - str->Ptr);
}



/* @obsolete ajStrFind
** @rename ajStrFindS
*/

__deprecated ajint  ajStrFind(const AjPStr str, const AjPStr str2)
{
    return ajStrFindS(str, str2);
}

/* @func ajStrFindAnyC ********************************************************
**
** Finds the first occurrence in a string of any character in a second 
** (text) string.
**
** @param [r] str [const AjPStr] String
** @param [r] txt2 [const char*] text to find
** @return [ajint] Position of the start of text in string if found.
**                Or -1 for text not found.
** @@
******************************************************************************/

ajint ajStrFindAnyC(const AjPStr str, const char* txt2)
{
    ajuint i;

    i = strcspn(str->Ptr, txt2);
    if(i == str->Len)
	return -1;
    return i;
}




/* @func ajStrFindAnyK ********************************************************
**
** Finds the first occurrence in a string of a specified character.
**
** @param [r] str [const AjPStr] String
** @param [r] chr [char] character to find
** @return [ajint] Position of the start of text in string if found.
**                Or -1 for text not found.
** @@
******************************************************************************/

ajint ajStrFindAnyK(const AjPStr str, char chr)
{
    const char* cp;

    cp = strchr(str->Ptr, (ajint) chr);
    if(!cp)
	return -1;

    return(cp - str->Ptr);
}



/* @obsolete ajStrFindK
** @rename ajStrFindAnyK
*/

__deprecated ajint  ajStrFindK(const AjPStr thys, const char chr)
{
    return ajStrFindAnyK(thys, chr);
}

/* @func ajStrFindAnyS ********************************************************
**
** Finds the first occurrence in a string of any character in a second 
** string.
**
** @param [r] str [const AjPStr] String
** @param [r] str2 [const AjPStr] text to find
** @return [ajint] Position of the start of text in string if found.
**                Or -1 for text not found.
** @@
******************************************************************************/

ajint ajStrFindAnyS(const AjPStr str, const AjPStr str2)
{
    ajuint i;

    i = strcspn(str->Ptr, str2->Ptr);
    if(i == str->Len)
	return -1;
    return i;
}



/* @func ajStrFindCaseC *******************************************************
**
** Finds the first occurrence in a string of a second (text) string using a
** case-insensitive search.
**
** @param [r] str [const AjPStr] String
** @param [r] txt2 [const char*] text to find
** @return [ajint] Position of the start of text in string if found.
** @error -1 Text not found.
** @@
******************************************************************************/

ajint ajStrFindCaseC(const AjPStr str, const char *txt2)
{
    AjPStr t1;
    AjPStr t2;
    ajint v;

    t1 = ajStrNewC(str->Ptr);
    t2 = ajStrNewC(txt2);
    ajStrFmtUpper(&t1);
    ajStrFmtUpper(&t2);

    v = ajStrFindS(t1,t2);
    ajStrDel(&t1);
    ajStrDel(&t2);

    return v;
}




/* @func ajStrFindCaseS *******************************************************
**
** Finds the first occurrence in a string of a second string using a 
** case-insensitive search.
**
** @param [r] str [const AjPStr] String
** @param [r] str2 [const AjPStr] text to find
** @return [ajint] Position of the start of text in string if found.
**                -1 if not found.
** @@
******************************************************************************/

ajint ajStrFindCaseS(const AjPStr str, const AjPStr str2)
{
    return ajStrFindCaseC(str,str2->Ptr);
}



/* @obsolete ajStrFindCase
** @rename ajStrFindCaseS
*/

__deprecated ajint  ajStrFindCase(const AjPStr str, const AjPStr str2)
{
    return ajStrFindCaseS(str,str2);
}


/* @func ajStrFindNextC *******************************************************
**
** Finds the next occurrence in a string of any character in a second 
** (text) string.
**
** @param [r] str [const AjPStr] String
** @param [r] pos1 [ajint] Start position in string
** @param [r] txt2 [const char*] text to find
** @return [ajint] Position of the start of text in string if found.
**                Or -1 for text not found.
** @@
******************************************************************************/

ajint ajStrFindNextC(const AjPStr str, ajint pos1, const char* txt2)
{
    ajuint i;
    ajuint jpos;

    jpos = ajMathPos(str->Len, pos1);

    i = jpos + strcspn(&str->Ptr[jpos], txt2);
    if(i == str->Len)
	return -1;
    return i;
}




/* @func ajStrFindNextK *******************************************************
**
** Finds the next occurrence in a string of a specified character.
**
** @param [r] str [const AjPStr] String
** @param [r] pos1 [ajint] Start position in string
** @param [r] chr [char] character to find
** @return [ajint] Position of the start of text in string if found.
**                Or -1 for text not found.
** @@
******************************************************************************/

ajint ajStrFindNextK(const AjPStr str, ajint pos1, char chr)
{
    const char* cp;
    ajuint jpos;

    jpos = ajMathPos(str->Len, pos1);

    cp = strchr(&str->Ptr[jpos], (ajint) chr);
    if(!cp)
	return -1;

    return(cp - str->Ptr);
}



/* @func ajStrFindNextS *******************************************************
**
** Finds the next occurrence in a string of any character in a second 
** string.
**
** @param [r] str [const AjPStr] String
** @param [r] pos1 [ajint] Start position in string
** @param [r] str2 [const AjPStr] text to find
** @return [ajint] Position of the start of text in string if found.
**                Or -1 for text not found.
** @@
******************************************************************************/

ajint ajStrFindNextS(const AjPStr str, ajint pos1, const AjPStr str2)
{
    ajuint i;
    ajuint jpos;

    jpos = ajMathPos(str->Len, pos1);

    i = jpos + strcspn(&str->Ptr[jpos], str2->Ptr);
    if(i == str->Len)
	return -1;
    return i;
}


/* @func ajStrFindRestC *******************************************************
**
** Finds the first occurrence in a string of any character not in a second 
** (text) string.
**
** @param [r] str [const AjPStr] String
** @param [r] txt2 [const char*] text to find
** @return [ajint] Position of the start of text in string if found.
**                Or -1 for text not found.
** @@
******************************************************************************/

ajint ajStrFindRestC(const AjPStr str, const char* txt2)
{
    ajuint i;

    i = strspn(str->Ptr, txt2);
    if(i == str->Len)
	return -1;
    return i;
}




/* @func ajStrFindRestS *******************************************************
**
** Finds the first occurrence in a string of any character not in a second 
** (text) string.
**
** @param [r] str [const AjPStr] String
** @param [r] str2 [const AjPStr] text to find
** @return [ajint] Position of the start of text in string if found.
**                Or -1 for text not found.
** @@
******************************************************************************/

ajint ajStrFindRestS(const AjPStr str, const AjPStr str2)
{
  return ajStrFindRestC(str, str2->Ptr);
}




/* @func ajStrFindRestCaseC ***************************************************
**
** Finds the first occurrence in a string of any character not in a second 
** (text) string (case-insensitive).
**
** @param [r] str [const AjPStr] String
** @param [r] txt2 [const char*] text to find
** @return [ajint] Position of the start of text in string if found.
**                Or -1 for text not found.
** @@
******************************************************************************/

ajint ajStrFindRestCaseC(const AjPStr str, const char* txt2)
{
    ajuint i;
    AjPStr tmpstr = ajStrNewS(str);
    AjPStr tmptxt = ajStrNewC(txt2);

    ajStrFmtUpper(&tmptxt);
    ajStrFmtUpper(&tmpstr);

    i = strspn(tmpstr->Ptr, tmptxt->Ptr);
    ajStrDel(&tmpstr);
    ajStrDel(&tmptxt);
    if(i == str->Len)
	return -1;
    return i;
}




/* @func ajStrFindRestCaseS ***************************************************
**
** Finds the first occurrence in a string of any character not in a second 
** (text) string (case-insensitive).
**
** @param [r] str [const AjPStr] String
** @param [r] str2 [const AjPStr] text to find
** @return [ajint] Position of the start of text in string if found.
**                Or -1 for text not found.
** @@
******************************************************************************/

ajint ajStrFindRestCaseS(const AjPStr str, const AjPStr str2)
{
  return ajStrFindRestCaseC(str, str2->Ptr);
}




/* @func ajStrFindlastC *******************************************************
**
** Finds the last occurrence in a string of a second (text) string.
**
** @param [r] str [const AjPStr] String to search
** @param [r] txt2 [const char*] text to look for
** @return [ajint] Position of the text string if found.
** @error -1 Text not found.
** @@
******************************************************************************/

ajint ajStrFindlastC(const AjPStr str, const char* txt2)
{
    ajint i = 0;
    ajuint j = 0;
    ajuint len = 0;
    const char* ptr1 = 0;
    const char* ptr2 = 0;
    AjBool found = ajTrue;

    len = strlen(txt2);

    for(i=str->Len-len;i>=0;i--)
    {
	ptr1 = &str->Ptr[i];
	ptr2 = txt2;
	found = ajTrue;
	for(j=0;j<len;j++)
	{
	    if(*ptr1 != *ptr2)
	    {
		found = ajFalse;
		break;
	    }
	    ptr2++;
	    ptr1++;
	}

	if(found)
	    return i;
    }

    return -1;
}


/* @obsolete ajStrRFindC
** @rename ajStrFindlastC
*/

__deprecated ajint  ajStrRFindC(const AjPStr thys, const char* text)
{
    return ajStrFindlastC(thys, text);
}

/* @func ajStrFindlastS *******************************************************
**
** Finds the last occurrence in a string of a second (text) string.
**
** @param [r] str [const AjPStr] String to search
** @param [r] str2 [const AjPStr] text to look for
** @return [ajint] Position of the text string if found.
** @error -1 Text not found.
** @@
******************************************************************************/

ajint ajStrFindlastS(const AjPStr str, const AjPStr str2)
{
    ajint i = 0;
    ajuint j = 0;
    const char* ptr1 = 0;
    const char* ptr2 = 0;
    AjBool found = ajTrue;

    for(i=str->Len-str2->Len;i>=0;i--)
    {
	ptr1 = &str->Ptr[i];
	ptr2 = &str2->Ptr[i];
	found = ajTrue;
	for(j=0;j<str2->Len;j++)
	{
	    if(*ptr1 != *ptr2)
	    {
		found = ajFalse;
		break;
	    }
	    ptr2++;
	    ptr1++;
	}

	if(found)
	    return i;
    }

    return -1;
}


/* @section parsing functions *************************************************
**
** Functions for parsing tokens from strings.
**
** @fdata      [AjPStr]
** @fnote     None.
**
** @nam3rule Extract        Return token(s) from a string and return
**                          the remainder
** @nam4rule ExtractFirst   Remove first word from a string, no leading spaces
** @nam4rule ExtractWord    Remove first word from a string, skipping spaces
** @nam3rule Parse          Parse tokens using strtok
** @nam4rule ParseCount     Count tokens using string token functions
** @nam5rule ParseCountMulti Multiple (reentrant) version (obsolete?)
** @nam4rule ParseSplit     Parse into array of strings
** @nam4rule ParseWhite     Count tokens using string token functions
**
** @argrule * str [const AjPStr] String
** @argrule C txtdelim [const char*] Text to find
** @argrule K chr [char] Character
** @argrule S strdelim [const AjPStr] Text to find
** @argrule Split PPstr [AjPStr**] String array of results
** @argrule Extract Prest [AjPStr*] Remainder of string
** @argrule First Pword [AjPStr*] First word of string
** @argrule Word Pword [AjPStr*] First word of string
**
** @valrule * [const AjPStr] Latest string parsed.
** @valrule *Count [ajuint] Number of string matches.
** @valrule *Split [ajuint] Number of string matches.
** @valrule *Extract [AjBool] True if a match was found
**
** @fcategory use
*/



/* @func ajStrExtractFirst ****************************************************
**
** Returns a word from the start of a string, and the remainder of the string
**
** @param [r] str [const AjPStr] String to be parsed
** @param [w] Prest [AjPStr*] Remainder of string
** @param [w] Pword [AjPStr*] First word of string
** @return [AjBool] True if parsing succeeded
** @@
******************************************************************************/

AjBool ajStrExtractFirst(const AjPStr str, AjPStr* Prest, AjPStr* Pword)
{
    ajuint i=0;
    ajuint j=0;
    const char* cp;

    if(!str)
	return ajFalse;

    if(!str->Len)
	return ajFalse;

    cp = str->Ptr;
    if(isspace(*cp++)) 	return ajFalse;

    while(*cp && !isspace(*cp))		/* look for end of first word */
    {
	cp++;
	i++;
    }

    j = i+1;
    while(*cp && isspace(*cp))
    {
	cp++;
	j++;
    }

    ajStrAssignSubS(Pword, str, 0, i);
    if(cp)
	ajStrAssignSubS(Prest, str, j, str->Len);
    else
	ajStrAssignClear(Prest);

    /*ajDebug("ajStrExtractFirst i:%d j:%d len:%d word '%S'\n",
      i, j, str->Len, *Pword);*/
    return ajTrue;
}



/* @func ajStrExtractWord *****************************************************
**
** Returns a word from the start of a string, and the remainder of the string.
** Leading spaces are skipped.
**
** @param [r] str [const AjPStr] String to be parsed
** @param [w] Prest [AjPStr*] Remainder of string
** @param [w] Pword [AjPStr*] First word of string
** @return [AjBool] True if parsing succeeded
** @@
******************************************************************************/

AjBool ajStrExtractWord(const AjPStr str, AjPStr* Prest, AjPStr* Pword)
{
    ajuint i=0;
    ajuint istart=0;
    ajuint j=0;
    const char* cp;

    if(!str)
	return ajFalse;

    if(!str->Len)
	return ajFalse;

    cp = str->Ptr;
    while(isspace(*cp))
    {
	cp++;
	istart++;
    }

    if(!*cp)
	return ajFalse;

    i = istart-1;
    while(*cp && !isspace(*cp))		/* look for end of first word */
    {
	cp++;
	i++;
    }

    j = i+1;
    while(*cp && isspace(*cp))
    {
	cp++;
	j++;
    }

    ajStrAssignSubS(Pword, str, istart, i);
    if(cp)
	ajStrAssignSubS(Prest, str, j, str->Len);
    else
	ajStrAssignClear(Prest);

    /*ajDebug("ajStrExtractWord i:%d j:%d len:%d word '%S'\n",
      i, j, str->Len, *Pword);*/
    return ajTrue;
}



/* @func ajStrParseC **********************************************************
**
** Tokenise a string using a specified set of delimiters and return tokens
** from the string. 
**
** This function uses the C run time library strtok function. This makes it
** necessary to only call it from one string at a time as strtok and this
** function have internal memories of the last position.
**
** @param [r] str [const AjPStr] String to be parsed (first call) or
**        NULL for followup calls using the same string, as for the
**        C RTL function strtok which is eventually called.
** @param [r] txtdelim [const char*] Delimiter(s) to be used betwen tokens.
** @return [const AjPStr] Token returned, when all tokens are parsed
**                  a NULL is returned.
** @@
******************************************************************************/

const AjPStr ajStrParseC(const AjPStr str, const char* txtdelim)
{
    static AjOStr strParseStr = { 1, 0, NULL, 0, 0};
    static AjPStr strp = &strParseStr;

    if(!strp->Ptr)
    {
	if(!str)
	{
	    ajWarn("Error in ajStrParseC: NULL argument and not initialised");
	    ajUtilCatch();
	    return NULL;
	}
    }

    if(str)
    {
	if(strParseCp) ajCharDel(&strParseCp);
	strParseCp = ajCharNewC(str->Ptr);
	strp->Ptr = ajSysFuncStrtok(strParseCp, txtdelim);
    }
    else
	strp->Ptr = ajSysFuncStrtok(NULL, txtdelim);

    if(strp->Ptr)
    {
	strp->Len = strlen(strp->Ptr);
	strp->Res = strp->Len + 1;
	return strp;
    }
    else {
	strp->Len=0;
	strp->Res=1;
	ajCharDel(&strParseCp);
    }

    return NULL;
}



/* @obsolete ajStrTokC
** @rename ajStrParseC
*/

__deprecated const AjPStr  ajStrTokC(const AjPStr thys, const char* delim)
{
    return ajStrParseC(thys, delim);
}

/* @func ajStrParseCount ******************************************************
**
** Returns the number of tokens in a string, delimited by whitespace
**
** @param [r] str [const AjPStr] String to examine.
** @return [ajuint] The number of tokens
** @@
******************************************************************************/

ajuint ajStrParseCount(const AjPStr str)
{
    AjPStrTok t = NULL;
    AjPStr tmp  = NULL;

    ajuint count;

    count = 0;
    ajStrTokenAssignC(&t, str, " \t\n\r");

    while(ajStrTokenNextParse(&t, &tmp))
	++count;

    ajStrTokenDel(&t);
    ajStrDel(&tmp);

    return count;
}


/* @obsolete ajStrTokenCount
** @rename ajStrParseCountC
*/

__deprecated ajint  ajStrTokenCount(const AjPStr line, const char *delim)
{
    return ajStrParseCountC(line, delim);
}

/* @func ajStrParseCountC *****************************************************
**
** Returns the number of tokens in a string using a specified set of 
** delimiters held in a text string.
**
** @param [r] str [const AjPStr] String to examine.
** @param [r] txtdelim [const char *] String of delimiter characters.
** @return [ajuint] The number of tokens
** @@
******************************************************************************/

ajuint ajStrParseCountC(const AjPStr str, const char *txtdelim)
{
    AjPStrTok t = NULL;
    AjPStr tmp  = NULL;

    ajuint count;

    count = 0;
    ajStrTokenAssignC(&t, str, txtdelim);

    while(ajStrTokenNextParse(&t, &tmp))
	++count;

    ajStrTokenDel(&t);
    ajStrDel(&tmp);

    return count;
}


/* @func ajStrParseCountS *****************************************************
**
** Returns the number of tokens in a string using a specified set of 
** delimiters held in a string.
**
** @param [r] str [const AjPStr] String to examine.
** @param [r] strdelim [const AjPStr] String of delimiter characters.
** @return [ajuint] The number of tokens
** @@
******************************************************************************/

ajuint ajStrParseCountS(const AjPStr str, const AjPStr strdelim)
{
    AjPStrTok t = NULL;
    AjPStr tmp  = NULL;

    ajuint count;

    count = 0;
    ajStrTokenAssignS(&t, str, strdelim);

    while(ajStrTokenNextParse(&t, &tmp))
	++count;

    ajStrTokenDel(&t);
    ajStrDel(&tmp);

    return count;
}


/* @func ajStrParseCountMultiC ************************************************
**
** Returns the number of tokens in a string.
**
** Uses reentrant {ajSysStrtokR} in place of the C RTL strtok function.
**
** @param [r] str [const AjPStr] String to examine.
** @param [r] txtdelim [const char *] String of delimiter characters.
** @return [ajuint] The number of tokens
** @@
******************************************************************************/

ajuint ajStrParseCountMultiC(const AjPStr str, const char *txtdelim)
{
    AjPStr buf  = NULL;
    ajuint count;
    char  *p;
    const char  *save = NULL;
    AjPStr mystr  = NULL;

    if(!str)
	return 0;

    buf = ajStrNew();
    mystr = ajStrNewS(str);

    p = ajSysFuncStrtokR(ajStrGetuniquePtr(&mystr),txtdelim,&save,&buf);
    if(!p)
    {
	ajStrDel(&buf);
	return 0;
    }

    count = 1;
    while(ajSysFuncStrtokR(NULL,txtdelim,&save,&buf))
	++count;

    ajStrDel(&buf);
    ajStrDel(&mystr);

    return count;
}


/* @obsolete ajStrTokenCountR
** @rename ajStrParseCountMultiC
*/

__deprecated ajint  ajStrTokenCountR(const AjPStr line, const char *delim)
{
    return ajStrParseCountMultiC(line, delim);
}

/* @func ajStrParseSplit ******************************************************
**
** Splits a newline-separated multi-line string into an array of strings.
**
** @param [r] str [const AjPStr] String
** @param [w] PPstr [AjPStr**] pointer to array of AjPStrs
**
** @return [ajuint] Number of array elements created
** @@
******************************************************************************/


ajuint ajStrParseSplit(const AjPStr str, AjPStr **PPstr)
{
    ajuint c;
    ajuint len;
    ajuint i;
    ajuint n;
    const char *p = NULL;
    const char *q = NULL;

    if(!str)
	return 0;

    if(!str->Len)
	return 0;

    p = q = str->Ptr;

    len = str->Len;
    c=0;
    n=0;
    for(i=0;i<len;++i)
	if(*(p++)=='\n')
	    ++c;
    p=q;
    if(ajStrGetCharLast(str) != '\n')
	++c;

    AJCNEW0(*PPstr,c);


    for(i=0;i<c;++i)
    {
	while(*q!='\n')	/* safe - we already counted c as # of newlines */
	    ++q;
	(*PPstr)[n] = ajStrNew();
	ajStrAssignSubC(&(*PPstr)[n++],p,0,q-p);
	p = ++q;
    }
    if(ajStrGetCharLast(str) != '\n')
	ajStrAssignC(&(*PPstr)[n++],q);

    return c;
}

/* @obsolete ajStrListToArray
** @rename ajStrParseSplit
*/

__deprecated ajint  ajStrListToArray(const AjPStr thys, AjPStr **array)
{
    return ajStrParseSplit(thys, array);
}

/* @func ajStrParseWhite ******************************************************
**
** Tokenise a string using whitespace and return tokens from the string. 
**
** @param [r] str [const AjPStr] String to be parsed (first call) or
**        NULL for followup calls using the same string, as for the
**        C RTL function strtok which is eventually called.
** @return [const AjPStr] Token
** @error NULL if no further token is found.
** @@
******************************************************************************/

const AjPStr ajStrParseWhite(const AjPStr str)
{
    return ajStrParseC(str, " \t\n\r");
}



/* @obsolete ajStrTok
** @rename ajStrParseWhite
*/

__deprecated const AjPStr  ajStrTok(const AjPStr str)
{
    return ajStrParseWhite(str);
}

/* @section debugging *********************************************************
**
** Functions for reporting of a string object.
**
** @fdata       [AjPStr]
** @nam3rule   Probe        Test string memory is valid
** @nam3rule   Stat         Report string statistics to debug file 
** @nam3rule   Trace        Report string elements to debug file 
** @nam4rule   TraceFull        All characters in detail
** @nam4rule   TraceTitle        With title text
**
** @argrule Probe Pstr [AjPStr const*] String
** @argrule Stat title [const char*] Report title
** @argrule Trace str [const AjPStr] String
** @argrule TraceTitle title [const char*] Report title
**
** @valrule * [void]
**
** @fcategory misc
*/

/* @func ajStrProbe ***********************************************************
**
** Default string memory probe which checks the string object and the
** string it contains are valid memory
** 
** If the given string is NULL, or a NULL pointer, simply returns.
**
** @param  [r] Pstr [AjPStr const *] Pointer to the string to be probed.
** @return [void]
** @@
******************************************************************************/

void ajStrProbe(AjPStr const * Pstr)
{
    AjPStr thys;

    if(!Pstr)
	return;

    if(!*Pstr)
	return;

    thys = *Pstr;

    if(thys != strPNULL)	/* ignore the constant null string */
    {
      AJMPROBE(thys->Ptr);
      AJMPROBE(*Pstr);
    }
    return;
}


/* @func ajStrStat ************************************************************
**
** Prints a summary of string usage with debug calls.
**
** @param [r] title [const char*] Title for this summary
** @return [void]
** @@
******************************************************************************/

void ajStrStat(const char* title)
{
#ifdef AJ_SAVESTATS
    static ajlong statAlloc     = 0;
    static ajlong statCount     = 0;
    static ajlong statFree      = 0;
    static ajlong statFreeCount = 0;
    static ajlong statTotal     = 0;

    ajDebug("String usage statistics since last call %s:\n", title);
    ajDebug("String usage (bytes): %Ld allocated, %Ld freed\n",
	    strAlloc - statAlloc, strFree - statFree);
    ajDebug("String usage (number): %Ld allocated, %Ld freed, %Ld in use\n",
	    strTotal - statTotal, strFreeCount - statFreeCount,
	    strCount - statCount);

    statAlloc     = strAlloc;
    statCount     = strCount;
    statFree      = strFree;
    statFreeCount = strFreeCount;
    statTotal     = strTotal;
#else
    (void) title;
#endif

    return;
}




/* @func ajStrTrace ***********************************************************
**
** Checks a string object for consistency and reports its contents.
**
** @param [r] str [const AjPStr] String
** @return [void]
** @@
******************************************************************************/

void ajStrTrace(const AjPStr str)
{
    if(!str)
    {
	ajDebug("String trace NULL\n");
	return;
    }

    if(!ajStrGetValid(str))
	ajDebug("ajStrGetValid problems: see warning messages\n");

    ajDebug("String trace use: %d size: %d len: %d string: ",
	    str->Use, str->Res, str->Len);

    if(str->Ptr)
    {
	if(str->Len <= 20)
	    ajDebug("<%s>\n", str->Ptr);
	else
	    ajDebug("<%10.10s>..<%s>\n",
		    str->Ptr, str->Ptr + str->Len-10);
    }
    else
	ajDebug("<NULL>\n");

    ajDebug("             ptr: %x    charptr: %x\n",
	    str, str->Ptr);

    return;
}




/* @func ajStrTraceFull ******************************************************
**
** Checks a string object for consistency and reports its contents character 
** by character.
**
** @param [r] str [const AjPStr] String
** @return [void]
** @@
******************************************************************************/

void ajStrTraceFull(const AjPStr str)
{
    ajuint i;

    if(!str)
    {
	ajDebug("String tracechars NULL\n");
	return;
    }

    if(!ajStrGetValid(str))
	ajDebug("ajStrGetValid problems: see warning messages\n");

    ajDebug("String tracechars use: %d size: %d len: %d string: ",
	    str->Use, str->Res, str->Len);

    if(str->Len)
    {
        ajDebug("\n");
        for(i=0; i < str->Len; i++)
	    ajDebug(" '%c' %20x [%d]\n",
		     str->Ptr[i], (ajint) str->Ptr[i], i);
    }
    else
	ajDebug("<NULL>\n");

    ajDebug("             ptr: %x    charptr: %x\n",
	    str, str->Ptr);

    return;
}




/* @func ajStrTraceTitle ******************************************************
**
** Checks a string object for consistency and reports its contents using a 
** defined title for the report.
**
** @param [r] str [const AjPStr] String
** @param [r] title [const char*] Report title
** @return [void]
** @@
******************************************************************************/

void ajStrTraceTitle(const AjPStr str, const char* title)
{
    ajDebug("%s\n",title);
    ajStrTrace(str);

    return;
}




/* @section exit **************************************************************
**
** Functions called on exit from the program by ajExit to do
** any necessary cleanup and to report internal statistics to the debug file
**
** @fdata      [AjPStr]
** @fnote     general exit functions, no arguments
**
** @nam3rule Exit Cleanup and report on exit
**
** @valrule * [void]
**
** @fcategory misc
*/

/* @func ajStrExit ************************************************************
**
** Prints a summary of string usage with debug calls.
**
** @return [void]
** @@
******************************************************************************/

void ajStrExit(void)
{
#ifdef AJ_SAVESTATS
    ajDebug("String usage (bytes): %Ld allocated, %Ld freed, %Ld in use\n",
	    strAlloc, strFree,
	    (strAlloc - strFree));
    ajDebug("String usage (number): %Ld allocated, %Ld freed %Ld in use\n",
	    strTotal, strFreeCount, strCount);
#endif

    ajCharDel(&strParseCp);
    return;
}




/* @obsolete ajStrFill
** @replace ajStrAppendCountK (1,2,3/1,3,*)
*/

__deprecated void  ajStrFill(AjPStr* pthys, ajint len, char fill)
{
    ajint icount;
    icount = len - (*pthys)->Len;
    ajStrAppendCountK(pthys, fill, icount);
    return;
}




/* @obsolete ajStrPos
** @replace ajMathPos (1,2/'ajStrGetLen[1]',2)
*/

__deprecated ajint  ajStrPos(const AjPStr thys, ajint ipos)
{
    return ajMathPos(thys->Len, ipos);
}




/* @obsolete ajStrPosI
** @replace ajMathPosI (1,2,3/'ajStrGetLen[1]',2,3)
*/

__deprecated ajint  ajStrPosI(const AjPStr thys, ajint imin, ajint ipos)
{
    return ajMathPosI(thys->Len, imin, ipos);
}




/* @obsolete ajStrPosII
** @rename ajMathPosI
*/

__deprecated ajint  ajStrPosII(ajint ilen, ajint imin, ajint ipos)
{
    return ajMathPosI(ilen, imin, ipos);
}




/* @obsolete ajCharPos
** @replace ajMathPos (1,2/'strlen[1]',2)
*/

__deprecated ajint  ajCharPos(const char* thys, ajint ipos)
{
    return ajMathPos(strlen(thys), ipos);
}



/* @datasection [AjIStr] String iterator *************************************
**
** String iterators point to successive characters in a string
**
** @nam2rule Str String
** @nam3rule Iter Iterator
**
******************************************************************************/

/* @section constructors ******************************************************
**
** @fdata [AjIStr]
**
** @nam4rule New      String iterator constructor.
** @nam5rule NewBack  String iterator reverse direction constructor.
**
** @argrule New str [const AjPStr] Original string
**
** @valrule * [AjIStr] String iterator
**
** @fcategory new
******************************************************************************/



/* @func ajStrIterNew *********************************************************
**
** String iterator constructor which allocates memory for a string iterator, 
** used to iterate over the characters in a string.
**
** @param [r] str [const AjPStr] Original string
**
** @return [AjIStr] String Iterator
** @@
******************************************************************************/

AjIStr ajStrIterNew(const AjPStr str)
{
    AjIStr iter;

    if(!str)
	ajFatal("ajStrIterNew source string NULL");

    AJNEW0(iter);
    iter->Start = iter->Ptr = str->Ptr;
    iter->End = iter->Start + MAJSTRGETLEN(str) - 1;

    return iter;
}


/* @obsolete ajStrIter
** @rename ajStrIterNew
*/

__deprecated AjIStr  ajStrIter(const AjPStr str)
{
    return ajStrIterNew(str);
}

/* @func ajStrIterNewBack *****************************************************
**
** String iterator constructor which allocates memory for a string iterator, 
** used to iterate over the characters in a string, from end to start.
**
** @param [r] str [const AjPStr] Original string
** @return [AjIStr] String Iterator
** @@
******************************************************************************/

AjIStr ajStrIterNewBack(const AjPStr str)
{
    AjIStr iter;

    if(!str)
	ajFatal("ajStrIterNewBack source string NULL");

    AJNEW0(iter);
    iter->Start = str->Ptr;
    iter->End = iter->Ptr = iter->Start + MAJSTRGETLEN(str) - 1;

    return iter;
}



/* @obsolete ajStrIterBack
** @rename ajStrIterNewBack
*/

__deprecated AjIStr  ajStrIterBack(const AjPStr str)
{
    return ajStrIterNewBack(str);
}

/* @section destructors *******************************************************
** @fdata [AjIStr]
**
** @nam4rule Del Destructor
**
** @argrule Del iter [AjIStr*] String iterator
**
** @valrule * [void]
**
** @fcategory delete
*/



/* @func ajStrIterDel ********************************************************
**
** String iterator destructor which frees memory for a string iterator.
**
** @param [d] iter [AjIStr*] String iterator
** @return [void]
** @@
******************************************************************************/

void ajStrIterDel(AjIStr* iter)
{
    AJFREE(*iter);

    return;
}


/* @obsolete ajStrIterFree
** @rename ajStrIterDel
*/

__deprecated void  ajStrIterFree(AjIStr* iter)
{
    ajStrIterDel(iter);
}

/* @section tests *************************************************************
** @fdata [AjIStr]
** @nam4rule   Done      Check whether iteration has ended (no more
**                             characters).
** @nam5rule   DoneBack  Reverse iterator
**
** @argrule Done iter [const AjIStr] Iterator
**
** @valrule * [AjBool] Result of test
**
** @fcategory use
*/

/* @func ajStrIterDone ******************************************************
**
** Tests whether a string iterator has completed yet.
**
** @param [r] iter [const AjIStr] String iterator.
** @return [AjBool] true if complete
** @@
******************************************************************************/

AjBool ajStrIterDone(const AjIStr iter)
{
    return (iter->Ptr > iter->End);
}

/* @func ajStrIterDoneBack ****************************************************
**
** Tests whether a string iterator (from end to start) has completed yet.
**
** @param [r] iter [const AjIStr] String iterator.
** @return [AjBool] true if complete
** @@
******************************************************************************/

AjBool ajStrIterDoneBack(const AjIStr iter)
{
    return (iter->Ptr < iter->Start);
}



/* @obsolete ajStrIterBackDone
** @rename ajStrIterDoneBack
*/

__deprecated AjBool  ajStrIterBackDone(AjIStr iter)
{
    return ajStrIterDoneBack(iter);
}

/* @section resets ************************************************************
** @fdata [AjIStr]
**
** @nam4rule   Begin     Resets iterator to start (first character).
** @nam4rule   End       Resets iterator to end (last character).
** @argrule * iter [AjIStr] String iterator
**
** @valrule * [void]
**
** @fcategory modify
*/

/* @func ajStrIterBegin *******************************************************
**
** Sets a string iterator to its start condition,
**
** @param [u] iter [AjIStr] String iterator.
** @return [void]
** @@
******************************************************************************/

void ajStrIterBegin(AjIStr iter)
{
    iter->Ptr = iter->Start;
    return;
}

/* @func ajStrIterEnd ********************************************************
**
** Sets a string iterator to its stop condition.
**
** @param [u] iter [AjIStr] String iterator.
** @return [void]
** @@
******************************************************************************/

void ajStrIterEnd(AjIStr iter)
{
    iter->Ptr = iter->End;
}

/* @section attributes ********************************************************
**
** @fdata [AjIStr]
**
** @nam4rule Get Return element
**
** @argrule Get iter [const AjIStr] Iterator
** @valrule GetC [const char*]
** @valrule GetK [char]
**
** @fcategory cast
*/

/* @func ajStrIterGetC *******************************************************
**
** Returns the remainder of the string at the current string iterator 
** position.
**
** @param [r] iter [const AjIStr] String iterator.
** @return [const char*] Current text string within iterator
** @@
******************************************************************************/

const char* ajStrIterGetC(const AjIStr iter)
{
    return iter->Ptr;
}


/* @func ajStrIterGetK ********************************************************
**
** Returns the value (character) at the current string iterator position.
**
** @param [r] iter [const AjIStr] String iterator.
** @return [char] Current character within iterator
** @@
******************************************************************************/

char ajStrIterGetK(const AjIStr iter)
{
    return *(iter->Ptr);
}


/* @section modifiers *********************************************************
**
** @fdata [AjIStr]
**
** @nam4rule Put Replace an element
**
** @argrule * iter [AjIStr]
** @argrule K chr [char] Character to put at current position
** @valrule * [void]
**
** @fcategory modify
*/

/* @func ajStrIterPutK ********************************************************
**
** Replaces the character at the current string iterator position.
**
** @param [u] iter [AjIStr] String iterator.
** @param [r] chr [char] Character
** @return [void]
** @@
******************************************************************************/

void ajStrIterPutK(AjIStr iter, char chr)
{
    *iter->Ptr = chr;
    return;
}



/* @section stepping **********************************************************
**
** @fdata [AjIStr]
**
** @nam4rule   Next      Iterate to next character.
** @nam5rule   NextBack  Reverse iterator
**
** @argrule * iter [AjIStr] String iterator
**
** @valrule * [AjIStr] Updated iterator
**
** @fcategory modify
*/


/* @func ajStrIterNext ********************************************************
**
** Step to next character in string iterator.
**
** @param [u] iter [AjIStr] String iterator.
** @return [AjIStr] Updated iterator duplicated as return value.
** @@
******************************************************************************/

AjIStr ajStrIterNext(AjIStr iter)
{
    iter->Ptr++;
    if(iter->Ptr > iter->End)
	return NULL;

    return iter;
}




/* @func ajStrIterNextBack ****************************************************
**
** Step to previous character in string iterator.
**
** @param [u] iter [AjIStr] String iterator.
** @return [AjIStr] Updated iterator duplicated as return value.
** @@
******************************************************************************/

AjIStr ajStrIterNextBack(AjIStr iter)
{
    iter->Ptr--;

    if(iter->Ptr < iter->Start)
	return NULL;

    return iter;
}



/* @obsolete ajStrIterBackNext
** @rename ajStrIterNextBack
*/

__deprecated AjIStr  ajStrIterBackNext(AjIStr iter)
{
    return ajStrIterNextBack(iter);
}


/* @obsolete ajStrIterMore
** @replace ajStrIterDone (1/!1)
*/

__deprecated AjBool  ajStrIterMore(AjIStr iter)
{
    return (!ajStrIterDone(iter));
}




/* @obsolete ajStrIterMoreBack
** @rename ajStrIterDoneBack
*/

__deprecated AjBool  ajStrIterMoreBack(AjIStr iter)
{
    return (!ajStrIterDoneBack(iter));
}


/* @datasection [AjPStrTok] String token parser *******************************
**
** @nam2rule Str String
** @nam3rule Token Token parser
**
******************************************************************************/

/* @section constructors ******************************************************
**
** @fdata [AjPStrTok]
**
** @nam4rule New Constructor
**
** @argrule New str [const AjPStr] string
** @argrule C txtdelim [const char*] Text delimiter
** @argrule S strdelim [const AjPStr] Text delimiter
** @valrule * [AjPStrTok] String token parser
**
** @fcategory new
*/


/* @func ajStrTokenNewC *******************************************************
**
** String token parser constructor which allocates memory for a string token 
** parser object from a string and a set of default delimiters defined in a 
** text string.
**
** @param [r] str [const AjPStr] Source string
** @param [r] txtdelim [const char*] Default delimiter(s)
** @return [AjPStrTok] A new string token parser.
** @@
******************************************************************************/

AjPStrTok ajStrTokenNewC(const AjPStr str, const char* txtdelim)
{
    AjPStrTok ret;

    AJNEW0(ret);

    ajStrAssignS(&ret->String, str);

    ret->Delim = ajStrNewC(txtdelim);
    ret->Pos   = 0;		  /* GFF parsing needed this change */

    return ret;
}


/* @obsolete ajStrTokenInit
** @rename ajStrTokenNewC
*/

__deprecated AjPStrTok  ajStrTokenInit(const AjPStr thys, const char* delim)
{
    return ajStrTokenNewC(thys, delim);
}

/* @func ajStrTokenNewS *******************************************************
**
** String token parser constructor which allocates memory for a string token 
** parser object from a string and an optional set of default delimiters 
** defined in a text string.
**
** @param [r] str [const AjPStr] Source string
** @param [r] strdelim [const AjPStr] Default delimiter(s)
** @return [AjPStrTok] A new string token parser.
** @@
******************************************************************************/

AjPStrTok ajStrTokenNewS(const AjPStr str, const AjPStr strdelim)
{
    AjPStrTok ret;

    AJNEW0(ret);

    ajStrAssignS(&ret->String, str);

    ret->Delim = ajStrNewS(strdelim);
    ret->Pos   = 0;		  /* GFF parsing needed this change */

    return ret;
}


/* @section destructors *******************************************************
**
** @fdata [AjPStrTok]
**
** @nam4rule Del Destructor
**
** @argrule Del Ptoken [AjPStrTok*] String token parser
**
** @valrule * [void]
**
** @fcategory delete
*/

/* @func ajStrTokenDel ******************************************************
**
** String token parser destructor which frees memory for a string token 
** parser.
**
** @param [d] Ptoken [AjPStrTok*] Token parser
** @return [void]
** @@
******************************************************************************/

void ajStrTokenDel(AjPStrTok* Ptoken)
{
    if(!*Ptoken)
	return;

    ajStrDel(&(*Ptoken)->String);
    ajStrDel(&(*Ptoken)->Delim);
    AJFREE(*Ptoken);

    return;
}




/* @obsolete ajStrTokenClear
** @rename ajStrTokenDel
*/

__deprecated void  ajStrTokenClear(AjPStrTok* token)
{
    ajStrTokenDel(token);
    return;
}


/* @section assignment ********************************************************
**
** @fdata [AjPStrTok]
**
** @nam4rule Assign Assignment of string and/or delimiter(s)
**
** @argrule Assign Ptoken [AjPStrTok*] Token parser
** @argrule Assign str [const AjPStr] String to be parsed
** @argrule C txtdelim [const char*] delimiter
** @argrule S strdelim [const AjPStr] delimiter
**
** @valrule * [AjBool] True on success
**
** @fcategory assign
*/

/* @func ajStrTokenAssign ****************************************************
**
** Generates a string token parser object from a string without a specific
** set of delimiters.
**
** @param [w] Ptoken [AjPStrTok*] String token object
** @param [r] str [const AjPStr] Source string
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajStrTokenAssign(AjPStrTok* Ptoken, const AjPStr str)
{
    AjPStrTok tok;
    
    if(*Ptoken)
	tok = *Ptoken;
    else
    {
	AJNEW0(tok);
	*Ptoken = tok;
    }
    
    ajStrAssignS(&tok->String, str);
    ajStrAssignClear(&tok->Delim);
    tok->Pos = 0;

    return ajTrue;
}



/* @obsolete ajStrTokenAss
** @rename ajStrTokenAssignC
*/

__deprecated AjBool  ajStrTokenAss(AjPStrTok* ptok, const AjPStr thys,
				  const char* delim)
{
    return ajStrTokenAssignC(ptok, thys, delim);
}

/* @func ajStrTokenAssignC ****************************************************
**
** Generates a string token parser object from a string and an optional
** set of default delimiters defined in a text string.
**
** The string token can be either an existing token to be overwritten
** or a NULL.
**
** @param [w] Ptoken [AjPStrTok*] String token object
** @param [r] str [const AjPStr] Source string
** @param [r] txtdelim [const char*] Default delimiter(s)
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajStrTokenAssignC(AjPStrTok* Ptoken, const AjPStr str,
			 const char* txtdelim)
{
    AjPStrTok tok;
    
    if(*Ptoken)
	tok = *Ptoken;
    else
    {
	AJNEW0(tok);
	*Ptoken = tok;
    }
    
    ajStrAssignS(&tok->String, str);
    ajStrAssignC(&tok->Delim, txtdelim);
    tok->Pos = strspn(tok->String->Ptr, tok->Delim->Ptr);

    return ajTrue;
}



/* @func ajStrTokenAssignS ****************************************************
**
** Generates a string token parser object from a string and an optional
** set of default delimiters defined in a string.
**
** @param [w] Ptoken [AjPStrTok*] String token object
** @param [r] str [const AjPStr] Source string
** @param [r] strdelim [const AjPStr] Default delimiter(s)
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajStrTokenAssignS(AjPStrTok* Ptoken, const AjPStr str,
			 const AjPStr strdelim)
{
    AjPStrTok tok;
    
    if(*Ptoken)
	tok = *Ptoken;
    else
    {
	AJNEW0(tok);
	*Ptoken = tok;
    }
    
    ajStrAssignS(&tok->String, str);
    ajStrAssignS(&tok->Delim, strdelim);
    tok->Pos = strspn(tok->String->Ptr, tok->Delim->Ptr);

    return ajTrue;
}

/* @section reset *************************************************************
**
** @fdata [AjPStrTok]
**
** @nam4rule Reset Reset the token parser internals
**
** @argrule * Ptoken [AjPStrTok*] String token parser
**
** @valrule * [void]
**
** @fcategory modify
*/

/* @func ajStrTokenReset ******************************************************
**
** Clears the strings from a string token parser object.
**
** @param [w] Ptoken [AjPStrTok*] String token object
** @return [void]
** @@
******************************************************************************/

void ajStrTokenReset(AjPStrTok* Ptoken)
{
    AjPStrTok tok;

    if(!Ptoken)
	return;

    if(!*Ptoken)
	return;

    tok = *Ptoken;

    ajStrDelStatic(&tok->String);
    ajStrDelStatic(&tok->Delim);

    return;
}




/* @section debugging *********************************************************
**
** @fdata [AjPStrTok]
**
** @nam4rule Trace Write internals to debug file
**
** @argrule * token [const AjPStrTok] String token parser
**
** @valrule * [void]
**
** @fcategory misc
*/

/* @func ajStrTokenTrace ******************************************************
**
** Writes a debug trace of a string token parser object.
**
** @param [r] token [const AjPStrTok] String token object
** @return [void]
** @@
******************************************************************************/

void ajStrTokenTrace(const AjPStrTok token)
{
    ajDebug("ajStrTokenTrace %x\n", token);

    if(!token)
	return;

    ajDebug("... String:\n");
    ajStrTrace(token->String);
    ajDebug("... Delim:\n");
    ajStrTrace(token->Delim);

    return;
}




/* @section parsing ***********************************************************
**
** @fdata [AjPStrTok]
**
** @nam4rule Next Return next token
** @nam5rule NextFind Use delimiter as a set of characters
** @nam5rule NextParse Use delimiter as a string
** @nam4rule Rest Return remainder of string
** @nam5rule RestParse Return remainder of string
**
** @argrule * Ptoken [AjPStrTok*] String token parser
** @argrule C txtdelim [const char*] Delimiter
** @argrule S strdelim [const AjPStr] Delimiter
** @argrule * Pstr [AjPStr*] String result
**
** @valrule * [AjBool] True on success
**
** @fcategory modify
*/

/* @func ajStrTokenNextFind *************************************************
**
** Parses tokens from a string using a string token parser.  Treats the 
** delimiter as a set of characters. 
**
** @param [u] Ptoken [AjPStrTok*] Token parser. Updated with the delimiter
**        string (if any) in delim.
** @param [w] Pstr [AjPStr*] Token found
**
** @return [AjBool] ajTrue if another token was found.
** @@
******************************************************************************/

AjBool ajStrTokenNextFind(AjPStrTok* Ptoken, AjPStr* Pstr)
{
    ajuint ilen;
    AjPStrTok token;
    char* cp;
    char* cq;

    token = *Ptoken;

    if(!*Ptoken)
    {					/* token already cleared */
	ajStrAssignClear(Pstr);
	return ajFalse;
    }

    if(token->Pos >= token->String->Len)
    {					/* all done */
	ajStrAssignClear(Pstr);
	ajStrTokenDel(Ptoken);
	return ajFalse;
    }

    cp = &token->String->Ptr[token->Pos];
    cq = strstr(cp, token->Delim->Ptr);
    if(cq)
    {
	ilen = cq - cp;
	ajStrAssignSubS(Pstr, token->String, token->Pos,
			token->Pos + ilen - 1);
	token->Pos += ilen;
	token->Pos += token->Delim->Len;
	return ajTrue;
    }

    /* delimiter not found - return rest of string */

    ilen = token->String->Len - token->Pos;
    ajStrAssignLenC(Pstr, cp, ilen);
    token->Pos += ilen;

    return ajTrue;
}



/* @obsolete ajStrDelim
** @replace ajStrTokenNextFind (1,2,n/2,1)
** @replace ajStrTokenNextFindC (1,2,3/2,3,1)
*/

__deprecated AjBool  ajStrDelim(AjPStr* pthis, AjPStrTok* ptoken,
			       const char* delim)
{
    if(delim)
	return ajStrTokenNextFindC(ptoken, delim, pthis);
    else
	return ajStrTokenNextFind(ptoken, pthis);
}

/* @func ajStrTokenNextFindC *************************************************
**
** Parses tokens from a string using a string token parser.  Uses characters 
** from a defined text string as a delimiter.
**
** @param [u] Ptoken [AjPStrTok*] Token parser. Updated with the delimiter
**        string in delim.
** @param [r] txtdelim [const char*] Delimiter string.
** @param [w] Pstr [AjPStr*] Token found
**
** @return [AjBool] ajTrue if another token was found.
** @@
******************************************************************************/

AjBool ajStrTokenNextFindC(AjPStrTok* Ptoken, const char* txtdelim,
			   AjPStr* Pstr)
{
    AjPStrTok token;

    token = *Ptoken;

    ajStrAssignC(&token->Delim, txtdelim);

    return ajStrTokenNextFind(Ptoken, Pstr);
}

/* @func ajStrTokenNextParse **************************************************
**
** Parses tokens from a string using a string token parser.  Treats the 
** delimiter as a string.
**
** The test uses the C function 'strcspn'.
**
** @param [u] Ptoken [AjPStrTok*] String token parsing object.
** @param [w] Pstr [AjPStr*] Next token returned, may be empty if the
**                           delimiter has changed.
** @return [AjBool] True if successfully parsed.
**                  False (and string set to empty) if there is nothing
**                  more to parse.
** @@
******************************************************************************/

AjBool ajStrTokenNextParse(AjPStrTok* Ptoken, AjPStr* Pstr)
{
    ajuint ilen;
    AjPStrTok token;
    char* cp;

    token = *Ptoken;

    if(!*Ptoken)
    {					/* token already cleared */
	ajStrAssignClear(Pstr);
	return ajFalse;
    }

    if(token->Pos >= token->String->Len)
    {					/* all done */
	ajStrAssignClear(Pstr);
	ajStrTokenDel(Ptoken);
	return ajFalse;
    }

    cp = &token->String->Ptr[token->Pos];
    ilen = strcspn(cp, token->Delim->Ptr);

    if(ilen)
    {
	ajStrAssignSubS(Pstr, token->String,
		       token->Pos, token->Pos + ilen - 1);
    }
    else
    {
	ajStrAssignClear(Pstr);
    }
    token->Pos += ilen;
    token->Pos += strspn(&token->String->Ptr[token->Pos], token->Delim->Ptr);

    return ajTrue;
}



/* @obsolete ajStrToken
** @replace ajStrTokenNextParse (1,2,n/2,1)
** @replace ajStrTokenNextParseC (1,2,3/2,3,1)
*/
__deprecated AjBool  ajStrToken(AjPStr* pthis, AjPStrTok* ptoken,
			       const char* delim)
{
    if(delim)
	return ajStrTokenNextParseC(ptoken, delim, pthis);
    else
	return ajStrTokenNextParse(ptoken, pthis);
}

/* @func ajStrTokenNextParseC *************************************************
**
** Parses tokens from a string using a string token parser.  Uses characters 
** from a defined text string as a delimiter.
**
** Returns the next token parsed from a string token parsing object
**
** Note: This can return "true" but an empty string in cases where the
** delimiter has changed since the previous call.
**
** The test uses the C function 'strcspn'.
**
** @param [u] Ptoken [AjPStrTok*] String token parsing object.
** @param [r] txtdelim [const char*] Delimiter character set.
** @param [w] Pstr [AjPStr*] Next token returned, may be empty if the
**                           delimiter has changed.
** @return [AjBool] True if successfully parsed.
**                  False (and string set to empty) if there is nothing
**                  more to parse.
** @@
******************************************************************************/

AjBool ajStrTokenNextParseC(AjPStrTok* Ptoken, const char* txtdelim,
			    AjPStr* Pstr)
{
    if(!*Ptoken) {
	ajStrAssignClear(Pstr);
	return ajFalse;
    }
    ajStrAssignC(&(*Ptoken)->Delim, txtdelim);
    return ajStrTokenNextParse(Ptoken, Pstr);
}




/* @func ajStrTokenNextParseS *************************************************
**
** Parses tokens from a string using a string token parser.  Uses characters 
** from a defined string as a delimiter.
**
** Note: This can return "true" but an empty string in cases where the
** delimiter has changed since the previous call.
**
** The test uses the C function 'strcspn'.
**
** @param [u] Ptoken [AjPStrTok*] String token parsing object.
** @param [r] strdelim [const AjPStr] Delimiter character set.
** @param [w] Pstr [AjPStr*] Next token returned, may be empty if the
**                           delimiter has changed.
** @return [AjBool] True if successfully parsed.
**                  False (and string set to empty) if there is nothing
**                  more to parse.
** @@
******************************************************************************/

AjBool ajStrTokenNextParseS(AjPStrTok* Ptoken, const AjPStr strdelim,
			    AjPStr* Pstr)
{
    if(!*Ptoken)
    {
	ajStrAssignClear(Pstr);
	return ajFalse;
    }

    ajStrAssignS(&(*Ptoken)->Delim, strdelim);
    return ajStrTokenNextParse(Ptoken, Pstr);
}






/* @func ajStrTokenRestParse **************************************************
**
** Returns the remainder of a string that's been partially parsed using a 
** string token parser.
**
** @param [u] Ptoken [AjPStrTok*] String token parsing object.
** @param [w] Pstr [AjPStr*] Next token returned.
** @return [AjBool] True if successfully parsed.
**                  False (and string set to empty) if there is nothing
**                  more to parse.
** @@
******************************************************************************/

AjBool ajStrTokenRestParse(AjPStrTok* Ptoken, AjPStr* Pstr)
{
    AjPStrTok token;

    token = *Ptoken;

    if(!*Ptoken)
    {					/* token already cleared */
	ajStrAssignClear(Pstr);
	return ajFalse;
    }

    if(token->Pos >= token->String->Len)
    {					/* all done */
	ajStrAssignClear(Pstr);
	ajStrTokenDel(Ptoken);
	return ajFalse;
    }

    if(token->Pos < token->String->Len)
	ajStrAssignSubS(Pstr, token->String,
			    token->Pos, token->String->Len);
    else
	ajStrAssignClear(Pstr);

    token->Pos = token->String->Len;

    return ajTrue;
}


/* @obsolete ajStrTokenRest
** @replace ajStrTokenRestParse (1,2/2,1)
*/

__deprecated AjBool  ajStrTokenRest(AjPStr* pthis, AjPStrTok* ptoken)
{
    return ajStrTokenRestParse(ptoken, pthis);
}



/* @obsolete ajStrNull
** @remove Make a local empty string instead
*/

__deprecated const AjPStr  ajStrNull(void)
{
    return strPNULL;
}





/* @obsolete ajStrArrayDel
** @remove Make a local copy if needed -
**         no need for full set of array functions
*/

__deprecated void  ajStrArrayDel(AjPStr** pthis)
{
    AjPStr* thys;
    ajint i;

    thys = pthis ? *pthis : 0;

    if(!pthis)
	return;

    if(!*pthis)
	return;

    for (i=0; thys[i];i++)
    {
	ajStrDel(&thys[i]);
    }

    AJFREE(*pthis);
    return;
}





/* @obsolete ajStrAss
** @replace ajStrAssignC (1,n/1,'""')
** @replace ajStrAssignRef (1,2/1,2)
*/

__deprecated AjBool  ajStrAss(AjPStr* pthis, AjPStr str)
{
    AjBool ret = ajTrue;		/* always true for now */

    ajStrDel(pthis);		/* we just use the ref count of str */

    if(str)
	ajStrAssignRef(pthis, str);
    else
	ajStrAssignClear(pthis);

    return ret;
}




/* @obsolete ajStrCopyC
** @replace ajStrDel (1,n/1)
** @replace ajStrAssignC (1,2/1,2)
*/

__deprecated AjBool  ajStrCopyC(AjPStr* pthis, const char* str)
{
    AjBool ret = ajTrue;		/* true if ajStrDup is used */

    if(!str)
	ajStrDel(pthis);
    else
	ajStrAssignC(pthis, str);

    return ret;
}




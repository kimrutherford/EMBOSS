/* @source ajstr ***************************************************************
**
** AJAX string functions
**
** AjPStr objects are reference counted strings
** Any change will need a new string object if the use count
** is greater than 1, so the original AjPStr is provided as a pointer
** so that it can be reallocated in any routine where string modification
** may be needed.
**
** In many cases the text is always a copy, even of a constant original, so
** that it can be simply freed.
**
** @author Copyright (C) 1998 Peter Rice
** @version $Revision: 1.205 $
** @modified 1998-2011 Peter Rice
** @modified $Date: 2013/06/30 12:03:51 $ by $Author: rice $
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




/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include <math.h>
#include <stdarg.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>
#include <float.h>
#include <string.h>
#include <sys/types.h>

#include "ajstr.h"
#include "ajfmt.h"
#include "ajmem.h"
#include "ajmess.h"
#include "ajsys.h"
#include "ajmath.h"
#include "ajsort.h"

#ifndef HAVE_MEMMOVE




/* @header memmove ************************************************************
**
******************************************************************************/

static void* memmove(void *dst, const void* src, size_t len)
{
    return (void *)bcopy(src, dst, len);
}
#endif




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

static AjPStr strNew(size_t size);
static AjPStr strClone(AjPStr* Pstr);
static void   strCloneL(AjPStr* pthis, size_t size);

#ifdef AJ_SAVESTATS
static ajlong strAlloc     = 0;
static ajlong strExtra     = 0;
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
** @argrule   NewRes  size [size_t] Reserved size
** @argrule   Len     len [size_t] Length of string
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
**
** @release 1.0.0
** @@
******************************************************************************/

char* ajCharNewC(const char* txt)
{
    char* cp;
    size_t len;

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
**
** @release 4.0.0
** @@
******************************************************************************/

char* ajCharNewS(const AjPStr str)
{
    static char* cp;

    cp = (char*) AJALLOC0(str->Len+1);
    memmove(cp, str->Ptr, str->Len+1);

    return cp;
}




/* @func ajCharNewRes *********************************************************
**
** A text string constructor which allocates memory for a string of the 
** specified length and initialises the contents as an empty string.
**
** @param [r] size [size_t] Length of the Cstring, excluding the trailing NULL.
** @return [char*] A new text string with no contents.
**
** @release 4.0.0
** @@
******************************************************************************/

char* ajCharNewRes(size_t size)
{
    static char* cp;

    cp = (char*) AJALLOC0(size+1);
    cp[0] = '\0';

    return cp;
}




/* @func ajCharNewResC ********************************************************
**
** A text string constructor which allocates memory for a string of the 
** specified length and initialises it with the text string provided.
**
** @param [r] txt [const char*] String object as initial value and size
**                          for the text.
** @param [r] size [size_t] Maximum string length, as returned by strlen
** @return [char*] A new text string.
** @ure The text provided must fit in the specified length
**
** @release 4.0.0
** @@
******************************************************************************/

char* ajCharNewResC(const char* txt, size_t size)
{
    static char* cp;
    size_t isize;
    size_t ilen;

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
** @param [r] size [size_t] Maximum string length, as returned by strlen
** @return [char*] A new text string.
** @ure The text provided must fit in the specified length
**
** @release 4.0.0
** @@
******************************************************************************/

char* ajCharNewResS(const AjPStr str, size_t size)
{
    static char* cp;
    size_t isize;

    isize = size;
    if(str->Len >= isize)
	isize = str->Len + 1;

    cp = (char*) AJALLOC0(isize);
    memmove(cp, str->Ptr, str->Len+1);

    return cp;
}




/* @func ajCharNewResLenC *****************************************************
**
** A text string constructor which allocates memory for a string of the 
** specified length and initialises it with the text string provided.
**
** @param [r] txt [const char*] String object as initial value and size
**                          for the text.
** @param [r] size [size_t] Maximum string length, as returned by strlen
** @param [r] len [size_t] Length of txt to save calculation time.
** @return [char*] A new text string.
** @ure The text provided must fit in the specified length
**
** @release 4.0.0
** @@
******************************************************************************/

char* ajCharNewResLenC(const char* txt, size_t size, size_t len)
{
    static char* cp;
    size_t isize;

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
**
** @release 6.0.0
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




/* @func ajCharDel ************************************************************
**
** A text string destructor to free memory for a text string.
**
** @param [d] Ptxt [char**] Text string to be deallocated.
** @return [void]
** @ure The string is freed using free in the C RTL, so it
**      must have been allocated by malloc in the C RTL
**
** @release 4.0.0
** @@
******************************************************************************/

void ajCharDel(char** Ptxt)
{
    AJFREE(*Ptxt);

    return;
}




/* @section filters
**
** Functions for creating filters from C-type (char*) strings).
**
** @fdata     [char*]
**
** @nam3rule  Getfilter   Get a filter array
** @nam4rule  Case        Case-insensitive filter
** @nam4rule  Lower       Lower case filter.
** @nam4rule  Upper       Upper case filter.
**
** @argrule   * txt [const char*] Character string
** @valrule   * [char*] Filter
** @fcategory use
*/




/* @func ajCharGetfilter ******************************************************
**
** Returns a filter array to test for any character in a string.
** 
** @param [r] txt [const char*] Character set to test
** @return [char*] Filter array set to 1 for each ascii value accepted
**                  zero for any character not in the set.
**
** @release 6.6.0
** @@
******************************************************************************/

char* ajCharGetfilter(const char *txt)
{
    char *ret;
    const char* cp;

    ret = AJALLOC0(256);

    if(!txt)
	return ret;

    if(!*txt)
        return ret;

    cp = txt;

    while (*cp)
    {
        ret[((int)*cp++)] = 1;
    }

    return ret;
}




/* @func ajCharGetfilterCase **************************************************
** Returns a filter array to test for any character in a string.
** The filter is case-insensitive
** 
** @param [r] txt [const char*] Character set to test
** @return [char*] Filter array set to 1 for each ascii value accepted
**                  zero for any character not in the set.
**
** @release 6.6.0
** @@
******************************************************************************/

char* ajCharGetfilterCase(const char *txt)
{
    char *ret;
    const char* cp;

    ret = AJALLOC0(256);

    if(!txt)
	return ret;

    if(!*txt)
        return ret;

    cp = txt;

    while (*cp)
    {
        ret[tolower((int)*cp)] = 1;
        ret[toupper((int)*cp++)] = 1;
    }

    return ret;
}




/* @func ajCharGetfilterLower *************************************************
**
** Returns a filter array to test for any character in a string as lower case.
** 
** @param [r] txt [const char*] Character set to test
** @return [char*] Filter array set to 1 for each ascii value accepted
**                  zero for any character not in the set.
**
** @release 6.6.0
** @@
******************************************************************************/

char* ajCharGetfilterLower(const char *txt)
{
    char *ret;
    const char* cp;

    ret = AJALLOC0(256);

    if(!txt)
	return ret;

    if(!*txt)
        return ret;

    cp = txt;

    while (*cp)
    {
        ret[tolower((int)*cp++)] = 1;
    }

    return ret;
}




/* @func ajCharGetfilterUpper ************************************************
**
** Returns a filter array to test for any character in a string as upper case.
** 
** @param [r] txt [const char*] Character set to test
** @return [char*] Filter array set to 1 for each ascii value accepted
**                  zero for any character not in the set.
**
** @release 6.6.0
** @@
******************************************************************************/

char* ajCharGetfilterUpper(const char *txt)
{
    char *ret;
    const char* cp;

    ret = AJALLOC0(256);

    if(!txt)
	return ret;

    if(!*txt)
        return ret;

    cp = txt;

    while (*cp)
    {
        ret[toupper((int)*cp++)] = 1;
    }

    return ret;
}




/* @section formatting
**
** Functions for formatting C-type (char*) strings).
**
** @fdata      [char*]
** @fnote     Same namrule as "String formatting functions"
**
** @nam3rule  Fmt         Change the format of a string.
** @nam4rule  FmtCapital  Convert each word to title case.
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
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajCharFmtLower(char* txt)
{
    char* cp;
    size_t ispan;

    ispan = strcspn(txt, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    if(txt[ispan])
    {
        cp = txt;

        while(*cp)
        {
	/*
	 *  AJB: function ajSysItoC was there as some really fussy compilers
	 *  complained about casting ajint to char. However, for conversion of
	 *  large databases it's too much of an overhead. Think about a macro
	 *  later. In the meantime revert to the standard system call
	 *    *cp = ajSysCastItoc(tolower((int) *cp));
	 */
            *cp = (char)tolower((int) *cp);
            cp++;
        }
    }

    return ajTrue;
}




/* @func ajCharFmtUpper *******************************************************
**
** Converts a text string to upper case.
**
** @param [u] txt [char*] Text string
** @return [AjBool] ajTrue on success
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajCharFmtUpper(char* txt)
{
    char* cp;
    size_t ispan;

    ispan = strcspn(txt, "abcdefghijklmnopqrstuvwxyz");
    if(txt[ispan])
    {
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
            *cp = (char) toupper((int) *cp);
            cp++;
        }
    }

    return ajTrue;
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
**
** @release 4.0.0
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




/* @func ajCharMatchCaseC *****************************************************
**
** Simple case-insensitive test for matching two text strings.
**
** @param [r] txt [const char*] String
** @param [r] txt2 [const char*] Text
** @return [AjBool] ajTrue if two strings are exactly the same excluding case
**
** @release 4.0.0
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
	if(toupper((int) *cp++) != toupper((int) *cq++))
	    return ajFalse;

    if(*cp || *cq)
	return ajFalse;

    return ajTrue;
}




/* @func ajCharMatchWildC *****************************************************
**
** Simple case-sensitive test for matching two text strings using 
** wildcard characters. 
**
** @param [r] txt [const char*] String
** @param [r] txt2 [const char*] Text
** @return [AjBool] ajTrue if the strings match
**
** @release 4.0.0
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




/* @func ajCharMatchWildS *****************************************************
**
** Simple case-sensitive test for matching a text string and a string using
** wildcard characters.
**
** @param [r] txt [const char*] String
** @param [r] str [const AjPStr] Wildcard text
** @return [AjBool] ajTrue if the strings match
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajCharMatchWildS(const char* txt, const AjPStr str)
{
    return ajCharMatchWildC(txt, MAJSTRGETPTR(str));
}




/* @func ajCharMatchWildCaseC *************************************************
**
** Simple case-insensitive test for matching two text strings using 
** wildcard characters. 
**
** @param [r] txt [const char*] String
** @param [r] txt2 [const char*] Text
** @return [AjBool] ajTrue if the strings match
**
** @release 4.1.0
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
**
** @release 4.1.0
** @@
******************************************************************************/

AjBool ajCharMatchWildCaseS(const char* txt, const AjPStr str)
{
    return ajCharMatchWildCaseC(txt, MAJSTRGETPTR(str));
}




/* @func ajCharMatchWildNextC *************************************************
**
** Test for matching the next 'word' in two text strings using 
** wildcard characters.
**
** @param [r] txt [const char*] String
** @param [r] txt2 [const char*] Text
** @return [AjBool] ajTrue if found
**
** @release 4.0.0
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

		while(*cp == '*')
		    savecp = cp++;	/* may be ***... savecp is last '*' */

		if(!*cp)
                    return ajTrue;

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
**
** @release 4.1.0
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

		while(*cp == '*')
		    savecp = cp++;	/* may be ***... savecp is last '*' */

		if(!*cp)
                    return ajTrue;

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
	    if(toupper((int) *cp) != toupper((int) *cq))
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
**
** @release 4.0.0
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
**
** @release 4.1.0
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




/* @func ajCharPrefixC ********************************************************
**
** Test for matching the start of a text string against a given prefix text 
** string.
**
** @param [r] txt [const char*]  Test string as text
** @param [r] txt2 [const char*] Prefix as text
** @return [AjBool] ajTrue if the string begins with the prefix
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajCharPrefixC(const char* txt, const char* txt2)
{
    size_t ilen;

    if(!txt)
	return ajFalse;

    if(!txt2)
	return ajFalse;

    ilen = strlen(txt2);

    if(!ilen)				/* no prefix */
	return ajFalse;

    if(ilen > strlen(txt))		/* prefix longer */
	return ajFalse;

    if(strncmp(txt, txt2, ilen))    /* +1 or -1 for a failed match */
	return ajFalse;

    return ajTrue;
}




/* @func ajCharPrefixS ********************************************************
**
** Test for matching the start of a text string against a given prefix string.
**
** @param [r] txt [const char*] Test string as text
** @param [r] str [const AjPStr] Prefix as string
** @return [AjBool] ajTrue if the string begins with the prefix
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajCharPrefixS(const char* txt, const AjPStr str)
{
    if(!txt)
	return ajFalse;

    if(!str)
	return ajFalse;

    if(str->Len > strlen(txt))	/* prefix longer */
	return ajFalse;

    if(strncmp(txt, MAJSTRGETPTR(str), str->Len)) /* +1 or -1 for
                                                     a failed match */
	return ajFalse;

    return ajTrue;
}




/* @func ajCharPrefixCaseC ****************************************************
**
** Case-insensitive test for matching the start of a text string against a 
** given prefix text string.
**
** @param [r] txt [const char*] Text
** @param [r] txt2 [const char*] Prefix
** @return [AjBool] ajTrue if the string begins with the prefix
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajCharPrefixCaseC(const char* txt, const char* txt2)
{
    const char* cp;
    const char* cq;

    if(!txt)
	return ajFalse;

    if(!txt2)
	return ajFalse;

    cp = txt;
    cq = txt2;

    /* ajDebug("ajCharPrefixCaseC '%s' '%s'\n", txt, txt2); */

    if(!*cq)
	return ajFalse;

    while(*cp && *cq)
    {
	if(toupper((int) *cp) != toupper((int) *cq))
            return ajFalse;

	cp++;cq++;
    }

    if(*cq)
	return ajFalse;

    /* ajDebug("ajStrPrefixCaseCC ..TRUE..\n"); */
    return ajTrue;
}




/* @func ajCharPrefixCaseS ****************************************************
**
** Case-insensitive test for matching start of a text string against a given 
** prefix string,
**
** @param [r] txt [const char*] Text
** @param [r] str [const AjPStr] Prefix
** @return [AjBool] ajTrue if the string begins with the prefix
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajCharPrefixCaseS(const char* txt, const AjPStr str)
{
    if(!str)
	return ajFalse;

    return ajCharPrefixCaseC(txt, MAJSTRGETPTR(str));
}




/* @func ajCharSuffixC ********************************************************
**
** Test for matching the end of a text string against a given suffix text 
** string.
**
** @param [r] txt [const char*] String
** @param [r] txt2 [const char*] Suffix as text
** @return [AjBool] ajTrue if the string ends with the suffix
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajCharSuffixC(const char* txt, const char* txt2)
{
    size_t ilen;
    size_t jlen;
    size_t jstart;

    if(!txt)
	return ajFalse;

    if(!txt2)
	return ajFalse;

    ilen   = strlen(txt2);
    jlen   = strlen(txt);
    jstart = jlen - ilen;

    if(ilen > jlen)			/* suffix longer */
	return ajFalse;

    if(strncmp(&txt[jstart], txt2, ilen)) /* +1 or -1 for a
					    failed match */
	return ajFalse;

    return ajTrue;
}




/* @func ajCharSuffixS ********************************************************
**
** Test for matching the end of a text string against a given suffix string.
**
** @param [r] txt [const char*] Test string as text
** @param [r] str [const AjPStr] Suffix as string
** @return [AjBool] ajTrue if the string ends with the suffix
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajCharSuffixS(const char* txt, const AjPStr str)
{
    size_t jlen;
    size_t jstart;

    if(!txt)
	return ajFalse;

    if(!str)
	return ajFalse;

    jlen   = strlen(txt);
    jstart = jlen - str->Len;

    if(str->Len > jlen)		/* suffix longer */
	return ajFalse;

    if(strncmp(&txt[jstart], MAJSTRGETPTR(str), str->Len)) /* +1 or -1 for a
                                                              failed match */
	return ajFalse;

    return ajTrue;
}




/* @func ajCharSuffixCaseC ****************************************************
**
** Case-insensitive test for matching the end of a text string against a given 
** suffix text string.
**
** @param [r] txt [const char*] String
** @param [r] txt2 [const char*] Suffix as text
** @return [AjBool] ajTrue if the string ends with the suffix
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajCharSuffixCaseC(const char* txt, const char* txt2)
{
    size_t ilen;
    size_t jlen;
    size_t jstart;
    const char* cp;
    const char* cq;

    if(!txt)
	return ajFalse;

    if(!txt2)
	return ajFalse;

    ilen   = strlen(txt2);
    jlen   = strlen(txt);
    jstart = jlen - ilen;

    if(ilen > jlen)			/* suffix longer */
	return ajFalse;

    cp = &txt[jstart];
    cq = txt2;

    while (*cp)
    {
	if(toupper((int)*cp) != toupper((int)*cq))
            return ajFalse;

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
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajCharSuffixCaseS(const char* txt, const AjPStr str)
{
    size_t jlen;
    size_t jstart;
    const char* cp;
    const char* cq;

    if(!txt)
	return ajFalse;

    if(!str)
	return ajFalse;

    jlen   = strlen(txt);
    jstart = jlen - str->Len;

    if(str->Len > jlen)		/* suffix longer */
	return ajFalse;

    cp = &txt[jstart];
    cq = MAJSTRGETPTR(str);

    while (cp)
    {
	if(toupper((int)*cp) != toupper((int)*cq)) return ajFalse;
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
** @argrule   Len len [size_t] Number of characters to compare
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
**
** @release 4.0.0
** @@
******************************************************************************/

int ajCharCmpCase(const char* txt, const char* txt2)
{
    const char* cp;
    const char* cq;

    for(cp = txt, cq = txt2; *cp && *cq; cp++, cq++)
	if(toupper((int) *cp) != toupper((int) *cq))
	{
	    if(toupper((int) *cp) > toupper((int) *cq))
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




/* @func ajCharCmpCaseLen *****************************************************
**
** Finds the sort order of substrings from the start of two text strings.
**
** @param [r] txt [const char*] Text string
** @param [r] txt2 [const char*] Text string
** @param [r] len  [size_t] length
** @return [int] -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
**
** @release 4.0.0
** @@
******************************************************************************/

int ajCharCmpCaseLen(const char* txt, const char* txt2, size_t len)
{
    const char* cp;
    const char* cq;
    size_t i;

    for(cp=txt,cq=txt2,i=0;*cp && *cq && i<len;++i,++cp,++cq)
	if(toupper((int) *cp) != toupper((int) *cq))
	{
	    if(toupper((int) *cp) > toupper((int) *cq))
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
**
** @release 4.0.0
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
**
** @release 4.1.0
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
	    if(toupper((int) *cp) != toupper((int) *cq))
	    {
		if(toupper((int) *cp) > toupper((int) *cq))
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
**        NULL for follow-up calls using the same string, as for the
**        C RTL function strtok which is eventually called.
** @param [r] txtdelim [const char*] Delimiter(s) to be used between tokens.
** @return [AjPStr] Token
** @error NULL if no further token is found.
**
** @release 4.0.0
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
** @argrule   K       ch [char] Single character
** @argrule   S       str [const AjPStr] Text string
** @argrule   NewRes  size [size_t] Reserved size
** @argrule   NewRef  refstr [AjPStr] Text string to be duplicated
** @argrule   Len     len [size_t] Length of string
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
**
** @release 1.0.0
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
**
** @release 1.0.0
** @@
******************************************************************************/

AjPStr ajStrNewC(const char* txt)
{
    size_t i;
    size_t j;
    AjPStr thys;

    i = strlen(txt);
    j = ajRound(i + 1, STRSIZE);

    thys = ajStrNewResLenC(txt, j, i);

    return thys;
}




/* @func ajStrNewK ************************************************************
**
** String constructor which allocates memory for a string and initialises it
** with the single character provided. 
** 
** The string size is set just large enough to hold the supplied text.
**
** @param [r] ch [char] Null-terminated character string to initialise
**                      the new string.
** @return [AjPStr] Pointer to a string containing the supplied text
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ajStrNewK(char ch)
{
    char txt[2] = " ";
    AjPStr thys;

    txt[0] = ch;
    thys = ajStrNewResLenC(txt, 2, 1);

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
**
** @release 1.0.0
** @@
******************************************************************************/

AjPStr ajStrNewS(const AjPStr str)
{
    if(str)
        return ajStrNewResLenC(str->Ptr, str->Res, str->Len);

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
**
** @release 4.0.0
** @@
******************************************************************************/

AjPStr ajStrNewRef(AjPStr refstr)
{
    if(!refstr)
	ajFatal("ajStrNewRef source string NULL");

    refstr->Use++;

    return refstr;
}




/* @func ajStrNewRes **********************************************************
**
** String constructor which allocates memory for a string of an initial 
** reserved size (including a possible null).
**
** @param [r] size [size_t] Reserved size (including a possible null).
** @return [AjPStr] Pointer to an empty string of specified size.
**
** @release 4.0.0
** @@
******************************************************************************/

AjPStr ajStrNewRes(size_t size)
{
    AjPStr thys;

    thys = ajStrNewResLenC("", size, 0);

    return thys;
}




/* @func ajStrNewResC *********************************************************
**
** String constructor which allocates memory for a string of an initial 
** reserved size (including a possible null) and initialises it with the text 
** string provided.
**
** @param [r] txt [const char*] Null-terminated character string to initialise
**        the new string.
** @param [r] size [size_t]  Reserved size (including a possible null).
** @return [AjPStr] Pointer to a string of the specified size
**         containing the supplied text.
**
** @release 4.0.0
** @@
******************************************************************************/

AjPStr ajStrNewResC(const char* txt, size_t size)
{
    size_t i;
    AjPStr thys;

    i = strlen(txt);

    thys = ajStrNewResLenC(txt, size, i);

    return thys;
}




/* @func ajStrNewResS *********************************************************
**
** String constructor which allocates memory for a string of an initial 
** reserved size (including a possible null) and initialises it with the string
** provided.
**
** @param [r] str [const AjPStr] String to initialise
**        the new string.
** @param [r] size [size_t]  Reserved size (including a possible null).
** @return [AjPStr] Pointer to a string of the specified size
**         containing the supplied text.
**
** @release 4.0.0
** @@
******************************************************************************/

AjPStr ajStrNewResS(const AjPStr str, size_t size)
{
    AjPStr thys;

    if(str)
        thys = ajStrNewResLenC(str->Ptr, size, str->Len);
    else
        thys = ajStrNewResLenC("", size, 0);

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
** @param [r] size [size_t]  Reserved size, including a trailing null and
**                           possible space for expansion
** @param [r] len [size_t] Length of txt to save calculation time.
** @return [AjPStr] Pointer to a string of the specified size
**         containing the supplied text.
**
** @release 4.0.0
** @@
******************************************************************************/

AjPStr ajStrNewResLenC(const char* txt, size_t size, size_t len)
{
    AjPStr thys;
    size_t minlen;

    minlen = size;

    if(size <= len)
	minlen = len+1;

    thys = strNew(minlen);
    thys->Len = len;

    if(txt && len)
	memmove(thys->Ptr, txt, len);

    thys->Ptr[len] = '\0';

    return thys;
}




/* @funcstatic strClone *******************************************************
**
** Makes a new clone of a string with a usage count of one and unchanged
** reserved size.
**
** @param [w] Pstr [AjPStr*] String
** @return [AjPStr] New String
**
** @release 1.0.0
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
** @param [r] size [size_t] Minimum reserved size.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void strCloneL(AjPStr* Pstr, size_t size)
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




/* @funcstatic strNew *********************************************************
**
** Internal constructor for modifiable AJAX strings. Used by all the string
** Parameterised constructors to allocate the space for the text string.
** The only exception is ajStrNew which returns a clone of the null string.
**
** @param [rE] size [size_t] size of the reserved space, including the
**        terminating NULL character. Zero uses a default string size STRSIZE.
** @return [AjPStr] A pointer to an empty string
**
** @release 4.0.0
** @@
******************************************************************************/

static AjPStr strNew(size_t size)
{
    AjPStr ret;

    if(!size)
	size = STRSIZE;
    else
        size = ajRound(size, 16);

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
**
** @release 1.0.0
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

    if(!thys->Use)
        ajErr("trying to delete unused string");

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
**
** @release 4.0.0
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
**
** @release 4.0.0
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
** @argrule   Res     size [size_t] Reserved maximum size
** @argrule   C       txt [const char*] Text string
** @argrule   K       chr [char] Single character
** @argrule   S       str [const AjPStr] Text string
** @argrule   Len     len [size_t] Length of string
** @argrule   Sub     pos1 [ajlong] Start position, negative value counts
**                                 from end
** @argrule   Sub     pos2 [ajlong] End position, negative value counts from end
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
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrAssignC(AjPStr* Pstr, const char* txt)
{
    AjBool ret = ajFalse;
    AjPStr thys;
    size_t i;
    size_t ires;

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




/* @func ajStrAssignK *********************************************************
**
** Copy a single character to a string.
**
** @param [w] Pstr [AjPStr*] Target string.
** @param [r] chr [char] Source text.
** @return [AjBool] ajTrue if string was reallocated
**
** @release 4.0.0
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
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrAssignS(AjPStr* Pstr, const AjPStr str)
{
    AjBool ret = ajFalse;
    AjPStr thys;
    size_t size;
    size_t roundsize = STRSIZE;

    if(!*Pstr)
    {
        if(str)
        {
            size = str->Len + 1;

            if(size >= LONGSTR)
                roundsize = ajRound(size, LONGSTR);
            else
                roundsize = ajRound(size, STRSIZE);
        }

	*Pstr = ajStrNewResS(str,roundsize);

	return ajTrue;
    }

    if(!str)
	return ajStrAssignClear(Pstr);

    thys = *Pstr;

    if(thys->Use != 1 || thys->Res <= str->Len)
    {
        /* min. reserved size OR more */
        ret = ajStrSetResRound(Pstr, str->Len+1);
        thys = *Pstr;
    }

    thys->Len = str->Len;
    memmove(thys->Ptr, str->Ptr, str->Len+1);

    return ret;
}




/* @func ajStrAssignClear *****************************************************
**
** Clear the string value
**
** @param [w] Pstr [AjPStr*] Target string.
** @return [AjBool] ajTrue if string was reallocated
**
** @release 6.0.0
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
**
** @release 4.0.0
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




/* @func ajStrAssignEmptyS ****************************************************
**
** Ensures a string is set (has a value). If the string is set it is left 
** alone, otherwise it is initialised with a string.
**
** @param [w] Pstr [AjPStr*] Target string which is overwritten.
** @param [r] str [const AjPStr] Source string object.
** @return [AjBool] ajTrue if string was reallocated
**
** @release 4.0.0
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




/* @func ajStrAssignLenC ******************************************************
**
** Copy a text string of a given length to a string.
**
** @param [w] Pstr [AjPStr*] Target string.
** @param [rN] txt [const char*] Source text.
** @param [r] len [size_t] Length of source text.
** @return [AjBool] ajTrue if string was reallocated
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrAssignLenC(AjPStr* Pstr, const char* txt, size_t  len)
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
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrAssignRef(AjPStr* Pstr, AjPStr refstr)
{
    AjBool ret = ajTrue;		/* true if ajStrNewRef is used */

    if(*Pstr)
    {
        if((*Pstr)->Use <= 1)
            ajStrDel(Pstr);
        else
            (*Pstr)->Use--;
    }

    if(!refstr)
	*Pstr = ajStrNewRef(strPNULL);
    else
        *Pstr = ajStrNewRef(refstr);

    return ret;
}




/* @func ajStrAssignResC ******************************************************
**
** Copy a text string to a string with a minimum reserved size.
**
** @param [w] Pstr [AjPStr*] Target string.
** @param [r] size [size_t] Space to reserve.
** @param [r] txt [const char*] Source text.
** @return [AjBool] ajTrue if string was reallocated
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrAssignResC(AjPStr* Pstr, size_t size, const char* txt)
{
    AjBool ret = ajFalse;
    AjPStr thys;
    size_t ilen;
    size_t isize;

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




/* @func ajStrAssignResS ******************************************************
**
** Copy a string to a string with a minimum reserved size.
**
** @param [w] Pstr [AjPStr*] Target string.
** @param [r] size [size_t] Size of new string.
** @param [rN] str [const AjPStr] Source text.
** @return [AjBool] ajTrue if string was reallocated
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrAssignResS(AjPStr* Pstr, size_t size, const AjPStr str)
{
    AjBool ret = ajFalse;
    AjPStr thys;
    size_t isize;

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




/* @func ajStrAssignSubC ******************************************************
**
** Copies a substring of a text string to a string.
**
** @param [w] Pstr [AjPStr*] Target string
** @param [r] txt [const char*] Source text
** @param [r] pos1 [ajlong] start position for substring
** @param [r] pos2 [ajlong] end position for substring
** @return [AjBool] ajTrue if string was reallocated
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrAssignSubC(AjPStr* Pstr, const char* txt, ajlong pos1, ajlong pos2)
{
    AjBool ret = ajFalse;
    size_t ilen;
    size_t ibegin;
    size_t iend;
    AjPStr thys;

    ibegin = (size_t) pos1;
    iend   = (size_t) pos2;

    if(pos1 < 0)
      ibegin = strlen(txt) + (size_t) pos1;

    if(pos2 < 0)
      iend = strlen(txt) + (size_t) pos2;

    ilen = iend - ibegin + 1;

    if(!*Pstr)
    {
        ret = ajTrue;
	ret = ajStrSetResRound(Pstr, ilen+1);
    }
    else if((*Pstr)->Use > 1)
    {
        ret = ajTrue;
        ajStrGetuniqueStr(Pstr);
    }

    if((*Pstr)->Res < ilen+1)
	ret = ajStrSetResRound(Pstr, ilen+1);

    thys = *Pstr;

    thys->Len = ilen;

    if (ilen)
	memmove(thys->Ptr, &txt[ibegin], ilen);

    thys->Ptr[ilen] = '\0';

    return ret;
}




/* @func ajStrAssignSubS ******************************************************
**
** Copies a substring of a string to a string.
**
** The substring is defined from character positions pos1 to pos2.
**
** ajTrue is returned if target was
** (re)allocated, ajFalse is returned otherwise. 
**
** @short Copies a substring of a string to a string.
**
** @param [w] Pstr [AjPStr*] Target string
**          {memory will be automatically allocated if required}
** @param [r] str [const AjPStr] Source string
** @param [r] pos1 [ajlong] Start position in src of substring
**              {negative values count from the end of the string
**               with -1 as the last position}
** @param [r] pos2 [ajlong] End position in src of substring
**              {negative values count from the end of the string
**               with -1 as the last position}
** @return [AjBool] ajTrue if Pstr was (re)allocated, ajFalse otherwise
** @release 1.0.0 
** @@
******************************************************************************/

AjBool ajStrAssignSubS(AjPStr* Pstr, const AjPStr str,
                       ajlong pos1, ajlong pos2)
{
    AjBool ret = ajFalse;
    size_t ilen;
    size_t ibegin;
    size_t iend;
    AjPStr thys;

    ibegin = ajCvtSposToPos(str->Len, pos1);
    iend = ajCvtSposToPosStart(str->Len, ibegin, pos2);

    if(iend == str->Len)
	iend--;

    ilen = iend - ibegin + 1;

    if(!*Pstr)
    {
        ret = ajTrue;
	ret = ajStrSetResRound(Pstr, ilen+1);
    }
    else if((*Pstr)->Use > 1)
    {
        ret = ajTrue;
        ajStrGetuniqueStr(Pstr);
    }

    if((*Pstr)->Res < ilen+1)
	ret = ajStrSetResRound(Pstr, ilen+1);

    thys = *Pstr;

    thys->Len = ilen;

    if (ilen)
	memmove(thys->Ptr, &str->Ptr[ibegin], ilen);

    thys->Ptr[ilen] = '\0';

    return ret;
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
** @nam3rule  Mask        Mask characters. 
** @nam4rule  MaskIdent   Mask matching characters. 
** @nam4rule  MaskRange   Mask a range of characters. 
**
** @argrule   *        Pstr [AjPStr*] Modifiable string
** @argrule   Insert  pos [ajlong] Position in string to start inserting,
**                                negative values count from the end
** @argrule   Paste   pos [ajlong] Position in string to start inserting,
**                                negative values count from the end
** @argrule   Join   pos [ajlong] Position in string to start inserting,
**                                negative values count from the end
** @argrule   C       txt [const char*] Text string
** @argrule   K       chr [char] Single character
** @argrule   S       str [const AjPStr] Text string
** @argrule   Count     num [ajulong] Number of single characters to copy
** @argrule   Len     len [size_t] Number of characters to copy from string
** @argrule   Max     len [size_t] Length of string
** @argrule   Sub       pos1 [ajlong] Start position,
**                                   negative value counts from end
** @argrule   MaskRange pos1 [ajlong] Start position,
**                                   negative value counts from end
** @argrule   Sub       pos2 [ajlong] End position,
**                                   negative value counts from end
** @argrule   MaskRange pos2 [ajlong] End position,
**                                   negative value counts from end
** @argrule   Join    posb [ajlong] Position in source string
**                                 negative values count from the end
** @argrule   MaskIdent str [const AjPStr] Comparison master string
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
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrAppendC(AjPStr* Pstr, const char* txt)
{
    size_t i;

    i = strlen(txt);

    return ajStrAppendLenC(Pstr, txt, i);
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
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrAppendK(AjPStr* Pstr, char chr)
{
    AjBool ret = ajFalse;
    AjPStr thys;
    size_t j;

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




/* @func ajStrAppendS *********************************************************
**
** Appends a string to the end of another string.
** 
** Uses {ajStrSetResRound} to make sure target string is modifiable.
**
** @param [w] Pstr [AjPStr*] Target string
** @param [r] str [const AjPStr] Source string
** @return [AjBool] ajTrue if string was reallocated
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrAppendS(AjPStr* Pstr, const AjPStr str)
{
    AjBool ret = ajFalse;

    AjPStr thys;
    size_t j;

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
** @param [r] num [ajulong] Repeat count
** @return [AjBool] ajTrue if string was reallocated
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrAppendCountK(AjPStr* Pstr, char chr, ajulong num)
{
    AjBool ret = ajFalse;
    AjPStr thys;
    size_t i;
    size_t j;
    char* cp;
    size_t snum;

    thys = *Pstr;

    snum = (size_t) num;

    if(thys)
      j = AJMAX(thys->Res, (thys->Len+snum+1));
    else
      j = snum+1;

    ret = ajStrSetResRound(Pstr, j);
    thys = *Pstr;			/* possible new location */

    cp = &thys->Ptr[thys->Len];

    for(i=0; i<num; i++)
    {
	*cp++ = chr;
    }

    *cp = '\0';
    thys->Len += snum;

    return ret;
}




/* @func ajStrAppendLenC ******************************************************
**
** Appends a text string of a given length to the end of a string.
** 
** Uses {ajStrSetRes} to make sure target string is modifiable.
**
** @param [w] Pstr [AjPStr*] Target string
** @param [rN] txt [const char*] Source text
** @param [r] len [size_t] String length
** @return [AjBool] ajTrue if string was reallocated
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrAppendLenC(AjPStr* Pstr, const char* txt, size_t len)
{
    AjBool ret = ajFalse;

    AjPStr thys;
    size_t j;

    thys = *Pstr;

    if(!txt)
	return ajFalse;

    if(*Pstr)
	j = AJMAX(thys->Res, (thys->Len+len+1));
    else
	j = len+1;

    ret = ajStrSetResRound(Pstr, j);
    thys = *Pstr;			/* possible new location */

    if(len)
        memmove(thys->Ptr+thys->Len, txt, len);

    thys->Len += len;

    thys->Ptr[thys->Len] = '\0';

    return ret;
}




/* @func ajStrAppendSubC ******************************************************
**
** Appends a substring of a string to the end of another string.
** 
** @param [w] Pstr [AjPStr*] Target string
** @param [r] txt [const char*] Source string
** @param [r] pos1 [ajlong] start position for substring
** @param [r] pos2 [ajlong] end position for substring
** @return [AjBool] ajTrue if string was reallocated
**
** @release 6.0.0
** @@
******************************************************************************/

AjBool ajStrAppendSubC(AjPStr* Pstr, const char* txt, ajlong pos1, ajlong pos2)
{
    size_t ilen;
    size_t jlen;
    size_t ibegin;
    size_t iend;
    AjBool ret = ajFalse;

    AjPStr thys;
    size_t j;

    thys = *Pstr;

    jlen = strlen(txt);
    ibegin = ajCvtSposToPosStart(jlen, 0, pos1);
    iend   = ajCvtSposToPosStart(jlen, ibegin, pos2);

    ilen = iend - ibegin + 1;

    if(thys)
	j = AJMAX(thys->Res, thys->Len+ilen+1);
    else
	j = ilen+1;

    ret = ajStrSetResRound(Pstr, j);
    thys = *Pstr;			/* possible new location */

    if(ilen)
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
** @param [r] pos1 [ajlong] start position for substring
** @param [r] pos2 [ajlong] end position for substring
** @return [AjBool] ajTrue if string was reallocated
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrAppendSubS(AjPStr* Pstr, const AjPStr str, ajlong pos1, ajlong pos2)
{
    size_t ilen;
    size_t ibegin;
    size_t iend;
    AjBool ret = ajFalse;

    AjPStr thys;
    size_t j;

    thys = *Pstr;

    ibegin = ajCvtSposToPosStart(str->Len, 0, pos1);
    iend   = ajCvtSposToPosStart(str->Len, ibegin, pos2);

    ilen = iend - ibegin + 1;

    if(thys)
	j = AJMAX(thys->Res, thys->Len+ilen+1);
    else
	j = ilen+1;

    ret = ajStrSetResRound(Pstr, j);
    thys = *Pstr;			/* possible new location */

    if(ilen)
        memmove(thys->Ptr+thys->Len, &str->Ptr[ibegin], ilen);

    thys->Len += ilen;

    thys->Ptr[thys->Len] = '\0';

    return ret;
}




/* @func ajStrInsertC *********************************************************
**
** Insert a text string into a string at a specified position.
**
** @param [u] Pstr [AjPStr*] Target string
** @param [r] pos [ajlong] Position where text is to be inserted.
**                        Negative position counts from the end
** @param [r] txt [const char*] Text to be inserted
** @return [AjBool]  ajTrue if string was reallocated
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajStrInsertC(AjPStr* Pstr, ajlong pos, const char* txt )
{
    AjBool ret = ajFalse;
    AjPStr thys;
    size_t j = 0;
    size_t y = 0;
    size_t ibegin;
    char* ptr1;
    const char* ptr2;
    size_t len;

    len = strlen(txt);

    thys = *Pstr;

    if(!thys)
    {
	ajStrAssignResC(Pstr, len+1, "");
	thys = *Pstr;
    }

    /* can be at start or after end */
    ibegin = ajCvtSposToPosStart(thys->Len+1, 0, pos);

    j = thys->Len+len+1;

    if(j > thys->Res)
	ret = ajStrSetResRound(Pstr, j);
    else
    {
        if(thys->Use > 1)
        {
            ajStrGetuniqueStr(Pstr);
            ret = ajTrue;
        }
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
** Inserts a character into a string at a specified position.
**
** @param [u] Pstr [AjPStr*] Target string
** @param [r] pos [ajlong] Position where text is to be inserted.
**                        Negative position counts from the end
** @param [r] chr [char] Text to be inserted
** @return [AjBool]  ajTrue if string was reallocated
**
** @release 2.9.0
** @@
******************************************************************************/

AjBool ajStrInsertK(AjPStr* Pstr, ajlong pos, char chr )
{
    char tmpstr[2] = "?";
    *tmpstr = chr;

    return ajStrInsertC(Pstr, pos, tmpstr);
}




/* @func ajStrInsertS *********************************************************
**
** Inserts a text string into a string at a specified position.
**
** @param [u] Pstr [AjPStr*] Target string
** @param [r] pos [ajlong] Position where text is to be inserted.
**                         Negative position counts from the end
** @param [r] str [const AjPStr] String to be inserted
** @return [AjBool] ajTrue on successful completion else ajFalse;
** @error ajFalse if the insert failed. Currently this happens if
**        pos is negative, but this could be reassigned to a position
**        from the end of the string in future.
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrInsertS(AjPStr* Pstr, ajlong pos, const AjPStr str)
{
    return ajStrInsertC(Pstr, pos, str->Ptr);
}




/* @func ajStrJoinC ***********************************************************
**
** Cut down string at pos1 and add string2 from position pos2.
**
** @param [u] Pstr [AjPStr*] Target string.
** @param [r] pos [ajlong] Number of characters to keep in target string.
** @param [r] txt [const char*] Text to append.
** @param [r] posb [ajlong] Position of first character to copy from text.
** @return [AjBool] ajTrue on success
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajStrJoinC(AjPStr* Pstr, ajlong pos, const char* txt,
		  ajlong posb)
{
    AjPStr thys;
    size_t len;
    size_t ibegin;
    size_t ibegin2;
    size_t i = 0;
    size_t j = 0;
    size_t newlen = 0;

    len = strlen(txt);

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);
    thys = *Pstr;

    ibegin = ajCvtSposToPosStart(thys->Len, 0, pos);
    ibegin2 = ajCvtSposToPosStart(len, 0, posb);

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
** @param [r] pos [ajlong] Start position in target string,
**                         negative numbers count from the end.
** @param [r] str [const AjPStr] String to append.
** @param [r] posb [ajlong] Starts position to copy,
**                         negative numbers count from the end.
** @return [AjBool] ajTrue on success.
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrJoinS(AjPStr* Pstr, ajlong pos, const AjPStr str,
		  ajlong posb)
{
    size_t ibegin1;
    size_t ibegin2;

    ibegin1 = ajCvtSposToPosStart((*Pstr)->Len, 0, pos);
    ibegin2 = ajCvtSposToPosStart(str->Len, 0, posb);

    return ajStrJoinC(Pstr, ibegin1, str->Ptr, ibegin2);
}




/* @func ajStrMaskIdent *******************************************************
**
** Masks out characters from a string that are identical to a second string.
**
** @param [w] Pstr [AjPStr*] Target string
** @param [r] str [const AjPStr] Comparison string
** @param [r] maskchr [char] masking character
** @return [AjBool] ajTrue on success
**
** @release 6.1.0
** @@
******************************************************************************/

AjBool ajStrMaskIdent(AjPStr* Pstr, const AjPStr str, char maskchr)
{
    char *cp;
    const char* cq;

    cp = ajStrGetuniquePtr(Pstr);
    cq = str->Ptr;

    while(*cq)
    {
        if(!*cp)
            return ajFalse;
        
        if(*cp == *cq)
            *cp = maskchr;

        cp++;
        cq++;
    }

    if(*cp)
        return ajFalse;

    return ajTrue;
}




/* @func ajStrMaskRange *******************************************************
**
** Masks out characters from a string over a specified range.
**
** @param [w] Pstr [AjPStr*] Target string
** @param [r] pos1 [ajlong] start position to be masked
** @param [r] pos2 [ajlong] end position to be masked
** @param [r] maskchr [char] masking character
** @return [AjBool] ajTrue on success, ajFalse if begin is out of range
**
** @release 6.1.0
** @@
******************************************************************************/

AjBool ajStrMaskRange(AjPStr* Pstr, ajlong pos1, ajlong pos2, char maskchr)
{
    AjPStr thys;
    size_t ibegin;
    size_t iend;
    size_t i;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);

    thys = *Pstr;

    ibegin = ajCvtSposToPosStart(thys->Len, 0, pos1);
    iend = ajCvtSposToPosStart(thys->Len, ibegin, pos2);

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
** @param [r] pos [ajlong] Position in target string
** @param [r] str [const AjPStr] String to replace.
** @return [AjBool] ajTrue on success
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrPasteS(AjPStr* Pstr, ajlong pos, const AjPStr str)
{
    return ajStrPasteMaxC(Pstr, pos, str->Ptr, str->Len);
}




/* @func ajStrPasteCountK *****************************************************
**
** Overwrites a string with a number of single characters.
** 
** Replace string at pos and add num copies of character chr.  Or to the end 
** of the existing string
**
** @param [u] Pstr [AjPStr*] Target string
** @param [r] pos [ajlong] position in string
** @param [r] chr [char] Character to replace.
** @param [r] num [ajulong] Number of characters to copy from text.
** @return [AjBool] ajTrue on success
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrPasteCountK( AjPStr* Pstr, ajlong pos, char chr,
                         ajulong num)
{
    AjPStr thys;
    size_t ibegin;
    size_t iend;
    char* ptr1 = 0;
    size_t i;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);

    thys = *Pstr;

    ibegin = ajCvtSposToPosStart(thys->Len, 0, pos);
    iend   = ibegin + (size_t) num;

    if(iend > thys->Len)		/* can't fit */
	iend = thys->Len - 1;

    ptr1 = &thys->Ptr[ibegin];

    for(i=ibegin;i<iend;i++)
	*ptr1++ = chr;

    return ajTrue;
}




/* @func ajStrPasteMaxC *******************************************************
**
** Overwrite one string with a specified number of characters from a text
** string.
** 
** Replaces string at pos ands add len characters from text string txt.
** Or to the end of the existing string
**
** @param [u] Pstr [AjPStr*] Target string
** @param [r] pos [ajlong] Start position in target string.
** @param [r] txt [const char*] String to replace.
** @param [r] len [size_t] Number of characters to copy from text.
** @return [AjBool] ajTrue on success
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrPasteMaxC (AjPStr* Pstr, ajlong pos, const char* txt,
		       size_t len)
{
    AjPStr thys;
    size_t ibegin;
    size_t iend;
    char* ptr1       = 0;
    const char* ptr2 = 0;
    size_t slen;
    size_t ilen;

    slen = strlen(txt);

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);

    thys = *Pstr;

    ibegin = ajCvtSposToPosStart(thys->Len, 0, pos);
    iend = ibegin + (size_t) len;

    if((iend  > thys->Len) || (len > slen) ) /* can't fit */
	return ajFalse;

    ptr1 = &thys->Ptr[ibegin];
    ptr2 = txt;

    for(ilen=(size_t )len; ilen > 0; --ilen)
	*ptr1++ = *ptr2++;

    return ajTrue;
}




/* @func ajStrPasteMaxS *******************************************************
**
** Overwrite one string with a specified number of characters from another
** string. 

** Replaces string at pos and add len characters from string str.
** Or to the end of the existing string
**
** @param [u] Pstr [AjPStr*] Target string
** @param [r] pos [ajlong] Start position in target string.
** @param [r] str [const AjPStr] Replacement string
** @param [r] len [size_t] Number of characters to copy from text.
** @return [AjBool] ajTrue on success
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrPasteMaxS(AjPStr* Pstr, ajlong pos, const AjPStr str,
		    size_t len)
{
    return ajStrPasteMaxC(Pstr, pos, str->Ptr, len);
}




/* @section cutting ***********************************************************
**
** Functions for removing characters or regions (substrings) from a string.
**
** @fdata      [AjPStr]
** @fnote     None.
** @nam3rule  Cut              Remove region(s) from a string. 
** @nam4rule  CutBraces        Remove comments in training braces
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
** @nam5rule  KeepSetAscii     Keep a range of ASCII characters.
** @nam5rule  KeepSetAlpha     Also remove non-alphabetic.
** @nam6rule  KeepSetAlphaRest  Also remove non-alphabetic and report non-space
** @nam5rule  KeepSetFilter    Use a saved filter object
** @nam3rule  Quote            Editing quotes in quoted strings
** @nam4rule  QuoteStrip       Removing quotes
** @nam5rule  QuoteStripAll    Removing internal and external quotes
** @nam3rule  Remove           Remove individual characters from a string.
** @nam4rule  RemoveGap        Remove non-sequence characters.
** @nam4rule  RemoveHtml       Remove HTML tags.
** @nam4rule  RemoveLast       Remove last character.
** @nam5rule  RemoveLastNewline   Remove last character if a newline only.
** @nam4rule  RemoveNonseq     Remove non-sequence characters
**                               (all chars except alphabetic & '*')
** @nam4rule  RemoveDupchar    Remove duplicated characters.
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
** @suffix F  Float array to be kept in step with one value per character
**
** @argrule   *        Pstr  [AjPStr*] Modifiable string
** @argrule   Pos      pos   [ajlong]   First position to be deleted.
                                       Negative values count from the end
** @argrule   Restpos  Pcomment  [AjPStr*]  Removed (comment) characters
** @argrule   Restpos  Pstartpos [size_t*]  Position at start of comment
** @argrule   C        txt   [const char*]  Text string
** @argrule   K        chr   [char]         Single character
** @argrule   S        str   [const AjPStr] Text string
** @argrule   F        Pfloat  [float*] Array of floats
** @argrule   Len      len   [size_t] Number of characters to process
** @argrule   CutEnd   len   [size_t] Number of characters to remove
** @argrule   CutStart len   [size_t] Number of characters to remove
** @argrule   Filter   filter [const char*] Filter array non-zero per character
** @argrule   Range    pos1  [ajlong]  Start position in string, negative
**                                    numbers count from end
** @argrule   Range    pos2  [ajlong]  End position in string, negative
**                                    numbers count from end
** @argrule   Rest     Prest [AjPStr*] Excluded characters
** @argrule   Ascii    minchar  [int] Lowest ASCII code to keep
** @argrule   Ascii    maxchar  [int] Highest ASCII code to keep
**
** @valrule   * [AjBool]
**
** @fcategory modify
*/




/* @func ajStrCutBraces *******************************************************
**
** Removes comments enclosed in braces from a string.
** 
** A comment is enclosed in curly braces at the end of a string.
**
** @param [u] Pstr [AjPStr*] Line of text from input file
** @return [AjBool] ajTrue if there is some text remaining
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajStrCutBraces(AjPStr* Pstr)
{
    AjPStr thys;
    char *cp;
    AjBool inbrace = ajFalse;
    AjBool skipbrace = ajFalse;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);

    thys = *Pstr;

    if(!thys->Len)		/* empty string */
	return ajFalse;

    cp = &thys->Ptr[thys->Len-1];

    while(cp >= thys->Ptr)
    {
      if(!isspace((int)*cp))
        {
            if(!inbrace)
            {
                if(*cp != '}')
                    return ajTrue;
                else
                    inbrace = ajTrue;
            }
            else
            {
                if(*cp == '{')
                {
                    skipbrace = ajTrue;
                    inbrace = ajFalse;
                    *cp = '\0';
                    thys->Len = cp - thys->Ptr;
                }
            }
        }
        else if (skipbrace)
        {
            *cp = '\0';
            thys->Len = cp - thys->Ptr;
        }
            
        cp--;
    }

    if(!thys->Len)	      /* no text before the comment */
	return ajFalse;

    return ajTrue;
}




/* @func ajStrCutComments *****************************************************
**
** Removes comments from a string.
** 
** A comment begins with a "#" character and may appear anywhere in the string.
** See ajStrCutCommentsStart for alternative definition of a comment.
**
** @param [u] Pstr [AjPStr*] Line of text from input file
** @return [AjBool] ajTrue if there is some text remaining
**
** @release 4.0.0
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




/* @func ajStrCutCommentsRestpos **********************************************
**
** Removes comments from a string.
** 
** A comment begins with a "#" character and may appear anywhere in the string.
** See ajStrCutCommentsStart for alternative definition of a comment.
**
** @param [u] Pstr [AjPStr*] Line of text from input file
** @param [w] Pcomment [AjPStr*] Comment characters deleted
** @param [w] Pstartpos [size_t*] Comment start position
** @return [AjBool] ajTrue if there is some text remaining
**
** @release 6.0.0
** @@
******************************************************************************/

AjBool ajStrCutCommentsRestpos(AjPStr* Pstr,
			       AjPStr* Pcomment, size_t* Pstartpos)
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
**
** @release 4.0.0
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




/* @func ajStrCutEnd **********************************************************
**
** Removes a number of characters from the end of a string
**
** @param [u] Pstr [AjPStr*] string
** @param [r] len [size_t] Number of characters to delete from the end
** @return [AjBool] ajTrue if string was reallocated
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrCutEnd(AjPStr* Pstr, size_t len)
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




/* @func ajStrCutRange ********************************************************
**
** Removes a substring from a string.
**
** @param [w] Pstr [AjPStr*] Target string
** @param [r] pos1 [ajlong] start position to be cut
** @param [r] pos2 [ajlong] end position to be cut
** @return [AjBool] ajTrue on success, ajFalse if begin is out of range
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrCutRange(AjPStr* Pstr, ajlong pos1, ajlong pos2)
{
    AjPStr thys;
    size_t ilen;
    size_t ibegin;
    size_t iend;
    size_t irest;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);

    thys = *Pstr;

    ibegin = ajCvtSposToPosStart(thys->Len, 0, pos1);
    iend   = ajCvtSposToPosStart(thys->Len, ibegin, pos2) + 1;
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




/* @func ajStrCutStart ********************************************************
**
** Removes a number of characters from the start of a string
**
** @param [u] Pstr [AjPStr*] string
** @param [r] len [size_t] Number of characters to delete from the start
** @return [AjBool] ajTrue if string was reallocated
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrCutStart(AjPStr* Pstr, size_t len)
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
** @param [r] pos1 [ajlong] Start position for substring.
** @param [r] pos2 [ajlong] End position for substring.
** @return [AjBool] ajTrue if string was reallocated
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrKeepRange(AjPStr* Pstr, ajlong pos1, ajlong pos2)
{
    AjBool ret = ajFalse;
    AjPStr str;
    size_t ibegin;
    size_t ilen;
    size_t iend;

    str = *Pstr;

    if(str->Use > 1)
      str = ajStrGetuniqueStr(Pstr);

    ibegin = ajCvtSposToPosStart(str->Len, 0, pos1);
    iend = ajCvtSposToPosStart(str->Len, 0, pos2);

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




/* @func ajStrKeepSetC ********************************************************
**
** Removes all characters from a string that are not in a given set.
**
** @param [u] Pstr [AjPStr *] String to clean.
** @param [r] txt [const char*] Character set to keep
** @return [AjBool] ajTrue if string was reallocated
**
** @release 4.0.0
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

    if(!thys->Len)
        return ajFalse;

    return ajTrue;
}




/* @func ajStrKeepSetFilter ***************************************************
**
** Removes all characters from a string that are not defined by a filter
** 
** @param [u] Pstr [AjPStr*] String
** @param [r] filter [const char*] Filter non-zero for each allowed value
** @return [AjBool] ajTrue if the string is entirely composed of characters
**                  in the specified set
**
** @release 6.6.0
** @@
******************************************************************************/

AjBool ajStrKeepSetFilter(AjPStr *Pstr, const char* filter)
{
    const char* cp;
    char* cq;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);

    cp = cq = (*Pstr)->Ptr;

    while(*cp)
    {
        if(filter[(int)*cp])
            *cq++ = *cp;
        cp++;
    }

    if(cp != cq)
    {
        *cq = '\0';
        (*Pstr)->Len -= (cp - cq);

        return ajFalse;
    }

    return ajTrue;
}




/* @func ajStrKeepSetS ********************************************************
**
** Removes all characters from a string that are not in a given set.
**
** @param [u] Pstr [AjPStr *] String to clean.
** @param [r] str [const AjPStr] Character set to keep
** @return [AjBool] ajTrue if string is not empty
**
** @release 4.1.0
** @@
******************************************************************************/

AjBool ajStrKeepSetS(AjPStr* Pstr, const AjPStr str)
{
    AjPStr thys;
    char *p = NULL;
    char *q = NULL;
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

    if(!thys->Len)
        return ajFalse;

    return ajTrue;
}




/* @func ajStrKeepSetAlpha ****************************************************
**
** Removes all characters from a string that are not alphabetic.
**
** @param [u] Pstr [AjPStr *] String to clean.
** @return [AjBool] ajTrue if string is not empty
**
** @release 4.1.0
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

    if(!thys->Len)
        return ajFalse;

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
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrKeepSetAlphaC(AjPStr* Pstr, const char* txt)
{
    AjPStr thys;
    char *p = NULL;
    char *q = NULL;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);

    thys = *Pstr;

    p = thys->Ptr;

    while(*p && (isalpha((ajint)*p) || strchr(txt, (ajint)*p)))
        p++;
    q = p;

    while(*p)
    {
	if(isalpha((ajint)*p) || strchr(txt, (ajint)*p))
	    *q++=*p;

	p++;
    }

    *q='\0';
    thys->Len = q - thys->Ptr;

    if(!thys->Len)
        return ajFalse;

    return ajTrue;
}




/* @func ajStrKeepSetAlphaS ***************************************************
**
** Removes all characters from a string that are not alphabetic and
** are not in a given set.
**
** @param [u] Pstr [AjPStr *] String to clean.
** @param [r] str [const AjPStr] Non-alphabetic character set to keep
** @return [AjBool] ajTrue if string is not empty
**
** @release 5.0.0
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
**
** @release 5.0.0
** @@
******************************************************************************/

AjBool ajStrKeepSetAlphaRest(AjPStr* Pstr, AjPStr* Prest)
{
    AjPStr thys = NULL;
    char *p = NULL;
    char *q = NULL;

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

    if(!thys->Len)
        return ajFalse;

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
**
** @release 5.0.0
** @@
******************************************************************************/

AjBool ajStrKeepSetAlphaRestC(AjPStr* Pstr, const char* txt, AjPStr* Prest)
{
    AjPStr thys = NULL;
    char *p = NULL;
    char *q = NULL;

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

    if(!thys->Len)
        return ajFalse;

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
**
** @release 5.0.0
** @@
******************************************************************************/

AjBool ajStrKeepSetAlphaRestS(AjPStr* Pstr, const AjPStr str, AjPStr* Prest)
{
    return ajStrKeepSetAlphaRestC(Pstr, str->Ptr, Prest);
}




/* @func ajStrKeepSetAscii ****************************************************
**
** Removes all characters from a string that are not within a range of
** ASCII character codes.
**
** @param [u] Pstr [AjPStr *] String to clean.
** @param [r] minchar [int] Lowest ASCII code to keep
** @param [r] maxchar [int] Highest ASCII code to keep
** @return [AjBool] ajTrue if string is not empty
**
** @release 6.1.0
** @@
******************************************************************************/

AjBool ajStrKeepSetAscii(AjPStr* Pstr, int minchar, int maxchar)
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

    while(*p && (*p >= minchar) && (*p <= maxchar))
    {
        p++;
    }
    q = p;

    while(*p)
    {
	if((*p >= minchar) && (*p <= maxchar))
	    *q++=*p;

	p++;
    }

    *q='\0';
    thys->Len = q - thys->Ptr;

    if(!thys->Len)
        return ajFalse;

    return ajTrue;
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
**
** @release 2.0.1
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

    if(!thys->Len)
        return ajFalse;

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
**
** @release 3.0.0
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

    if(!thys->Len)
        return ajFalse;

    return ajTrue;
}




/* @func ajStrRemoveDupchar ***************************************************
**
** Removes duplicate characters from a string
**
** @param [w] Pstr [AjPStr*] String
** @return [AjBool] True if string is not empty
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ajStrRemoveDupchar(AjPStr* Pstr)
{
    char filter[256] = {'\0'};		/* should make all zero */
    char *cp;
    char *cq;
    AjPStr thys;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);

    thys = *Pstr;

    cp = thys->Ptr;
    while(*cp && !filter[(ajint)*cp])
        filter[(ajint)*cp++] = 1;
    cq = cp;

    while(*cp)
    {
        if(!filter[(ajint)*cp])
        {
            filter[(ajint)*cp] = 1;
            *cq++ = *cp;
        }
        else
        {
            --thys->Len;
        }
        
        cp++;
    }
    *cq = '\0';

    if(!thys->Len)
        return ajFalse;

    return ajTrue;
}




/* @func ajStrRemoveGap *******************************************************
**
** Removes non-sequence characters (all but alphabetic characters and asterisk)
** from a string.
**
** @param [w] Pstr [AjPStr*] String
** @return [AjBool] True if string is not empty
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrRemoveGap(AjPStr* Pstr)
{
    char *p;
    char *q;
    size_t  i;
    size_t  len;
    char c;
    size_t ispan;
    AjPStr thys;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);

    thys = *Pstr;

    p = q = thys->Ptr;
    len = thys->Len;

    ispan = strspn(p, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz*");

    if(p[ispan])
    {
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
    }

    if(!thys->Len)
        return ajFalse;

    return ajTrue;
}




/* @func ajStrRemoveGapF ******************************************************
**
** Removes non-sequence characters (all but alphabetic characters and asterisk)
** from a string. Also removes floats from an array of the same size
**
** @param [w] Pstr [AjPStr*] String
** @param [w] Pfloat [float*] Floating point array (e.g. quality scores) 
** @return [AjBool] True if string is not empty
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ajStrRemoveGapF(AjPStr* Pstr, float* Pfloat)
{
    char *p;
    char *q;
    size_t  i;
    size_t  len;
    char c;
    size_t ispan;
    AjPStr thys;
    float *fp;
    float *fq;
    float f;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);

    thys = *Pstr;

    p = q = thys->Ptr;
    fp = fq = Pfloat;
    len = thys->Len;

    ispan = strspn(p, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz*");

    if(p[ispan])
    {
        for(i=0;i<len;++i)
        {
            c = *(p++);
            f = (*fp++);

            /* O is a gap character for Phylip ... but now valid for proteins */
            if((c>='A' && c<='Z') || (c>='a' && c<='z') || (c=='*'))
            {
                *(q++) = c;
                *(fq++) = f;
            }
            else
                --thys->Len;
        }

        thys->Ptr[thys->Len] = '\0';
    }

    if(!thys->Len)
        return ajFalse;

    return ajTrue;
}




/* @func ajStrRemoveHtml ******************************************************
**
** Removes html from a string.  
**
** html is defined as all substrings between and including angle brackets.
**
** @param [w] Pstr [AjPStr*] String
** @return [AjBool] ajTrue on success
**
** @release 1.9.0
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

    if(!thys->Len)
        return ajFalse;

    return ajTrue;
}




/* @func ajStrRemoveLastNewline ***********************************************
**
** Removes last character from a string if it is a newline character.
**
** @param [w] Pstr [AjPStr*] String
** @return [AjBool] True is string is not empty
**
** @release 4.0.0
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

    if(!thys->Len)
        return ajFalse;

    return ajTrue;
}




/* @func ajStrRemoveSetC ******************************************************
**
** Removes all of a given set of characters from a string.
**
** @param [w] Pstr [AjPStr*] String
** @param [r] txt [const char*] characters to remove
** @return [AjBool] True on success
**
** @release 4.0.0
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

    if(!thys->Len)
        return ajFalse;

    return ajTrue;
}




/* @func ajStrRemoveWhite *****************************************************
**
** Removes all whitespace characters from a string.
**
** @param [u] Pstr [AjPStr *] String to clean.
** @return [AjBool] ajTrue if string was reallocated
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrRemoveWhite(AjPStr* Pstr)
{
    AjPStr thys;
    size_t i = 0;
    size_t j = 0;
    size_t len;
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

    if(!thys->Len)
        return ajFalse;

    return ajTrue;
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
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrRemoveWhiteExcess(AjPStr* Pstr)
{
    AjBool ret  = ajFalse;
    AjPStr thys = NULL;

    size_t i = 0;
    size_t j = 0;
    size_t len;
    char *p = NULL;

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




/* @func ajStrRemoveWhiteSpaces ***********************************************
**
** Removes excess space characters from a string.
**
** Leading/trailing whitespace removed. Multiple spaces replaced by single
** spaces. Tabs converted to spaces. Newlines left unchanged
**
** @param [u] Pstr [AjPStr *] String to clean.
** @return [AjBool] ajTrue if string was reallocated
**
** @release 4.1.0
** @@
******************************************************************************/

AjBool ajStrRemoveWhiteSpaces(AjPStr* Pstr)
{
    AjBool ret      = ajFalse;
    AjPStr thys;

    size_t i = 0;
    size_t j = 0;
    size_t len;
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
**
** @release 4.0.0
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




/* @func ajStrTrimC ***********************************************************
**
** Removes regions with a given character composition from start and end of a 
** string.
**
** @param [u] Pstr [AjPStr*] string
** @param [r] txt [const char*] Characters to delete from each end
** @return [AjBool] ajTrue if string was reallocated
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajStrTrimC(AjPStr* Pstr, const char* txt)
{
    AjPStr thys;
    const char* cp;
    size_t i;

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
**
** @release 2.3.1
** @@
******************************************************************************/

AjBool ajStrTrimEndC(AjPStr* Pstr, const char* txt)
{
    AjPStr thys;
    const char* cp;
    size_t i;

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
**
** @release 3.0.0
** @@
******************************************************************************/

AjBool ajStrTrimStartC(AjPStr* Pstr, const char* txt)
{
    AjPStr thys;
    const char* cp;
    size_t i;

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
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrTrimWhite(AjPStr* Pstr)
{
    return ajStrTrimC(Pstr, "\t \n\r");
}




/* @func ajStrTrimWhiteEnd ****************************************************
**
** Removes regions composed of white space characters only from the end of a 
** string.
**
** @param [u] Pstr [AjPStr*] String
** @return [AjBool] ajTrue if string was reallocated
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrTrimWhiteEnd(AjPStr* Pstr)
{
    return ajStrTrimEndC(Pstr, "\t \n\r");
}




/* @func ajStrTrimWhiteStart **************************************************
**
** Removes regions composed of white space characters only from the start of a 
** string.
**
** @param [u] Pstr [AjPStr*] String
** @return [AjBool] ajTrue if string was reallocated
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ajStrTrimWhiteStart(AjPStr* Pstr)
{
    return ajStrTrimStartC(Pstr, "\t \n\r");
}




/* @func ajStrTruncateLen *****************************************************
**
** Removes the end from a string reducing it to a defined length.
**
** @param [u] Pstr [AjPStr*] Target string
** @param [r] len [size_t] Length of required string.
**
** @return [AjBool] ajTrue if string was reallocated
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrTruncateLen(AjPStr* Pstr, size_t len)
{
    AjPStr thys;
    AjBool ret = ajFalse;

    if(!*Pstr)
    {
        ret = ajTrue;
        *Pstr = ajStrNewResLenC("", 1, 0);
    }
    else if((*Pstr)->Use > 1)
    {
        ret = ajTrue;
        ajStrGetuniqueStr(Pstr);
    }
    
    thys = *Pstr;

    if(len >= thys->Len)
        return ret;

    thys->Ptr[len] = '\0';
    thys->Len = len;

    return ret;
}




/* @func ajStrTruncatePos *****************************************************
**
** Removes the end from a string by cutting at a defined position.
**
** @param [u] Pstr [AjPStr*] target string
** @param [r] pos [ajlong] First position to be deleted. Negative values
**                         count from the end
** @return [AjBool] True is string was reallocated
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrTruncatePos(AjPStr* Pstr, ajlong pos)
{
    AjPStr thys = NULL;
    size_t ibegin;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);

    thys = *Pstr;

    ibegin = 1 + ajCvtSposToPosStart(thys->Len, 0, pos);
    thys->Ptr[ibegin] = '\0';
    thys->Len = ibegin;

    return ajTrue;
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
** @argrule Pos ipos [ajlong] String position to be edited
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




/* @func ajStrExchangeCC ******************************************************
**
** Replace all occurrences in a string of one substring with another.
**
** @param [u] Pstr [AjPStr*]  Target string.
** @param [r] txt [const char*] string to replace.
** @param [r] txtnew [const char*] string to insert.
** @return [AjBool] ajTrue if string was reallocated
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrExchangeCC(AjPStr* Pstr, const char* txt,
                       const char* txtnew)
{    
    ajlong findpos = 0;
    size_t tlen;
    size_t newlen;
    ajlong lastpos = 0;			/* make sure we don't loop forever */
    AjPStr thys;
    const char* cp;

    if(!txt || !txtnew)
        return ajFalse;

    tlen = strlen(txt);
    newlen = strlen(txtnew);
    
    if(!tlen && !newlen)
	return ajFalse;

    thys = *Pstr;

    if(*txt)
    {
        findpos = ajStrFindC(thys, txt);
        if(findpos < 0)
            return ajFalse;

        cp = strstr(&thys->Ptr[lastpos], txt);

        while(cp)
        {
            findpos = cp - &thys->Ptr[lastpos];

	    if(findpos < ((ajlong)thys->Len - lastpos))
	    {
                lastpos += findpos;
		ajStrCutRange(Pstr,lastpos,lastpos+tlen-1);
		ajStrInsertC(Pstr,lastpos,txtnew);
		lastpos += newlen;
                ajDebug("new lastpos %Ld len %Ld\n",
                        lastpos, (ajlong) thys->Len);
	    }

            cp = strstr(&thys->Ptr[lastpos], txt);

	}
    }

    return ajTrue;
}




/* @func ajStrExchangeCS ******************************************************
**
** Replace all occurrences in a string of one substring with another.
**
** @param [u] Pstr [AjPStr*]  Target string.
** @param [r] txt [const char*] string to replace.
** @param [r] strnew [const AjPStr] string to insert.
** @return [AjBool] ajTrue if string was reallocated
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrExchangeCS(AjPStr* Pstr, const char* txt,
		       const AjPStr strnew)
{    
    AjBool cycle = ajTrue;
    ajlong findpos    = 0;
    size_t tlen;
    size_t newlen;
    ajlong lastpos = 0;			/* make sure we don't loop forever */

    if(!txt || !strnew)
        return ajFalse;

    tlen = strlen(txt);
    newlen = strnew->Len;
    
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
** @param [r] chr [char] Character to replace.
** @param [r] chrnew [char] Character to insert.
** @return [AjBool] ajTrue if string was reallocated
**
** @release 4.0.0
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




/* @func ajStrExchangeSC ******************************************************
**
** Replace all occurrences in a string of one substring with another.
**
**
** @param [u] Pstr [AjPStr*]  Target string.
** @param [r] str [const AjPStr] string to replace.
** @param [r] txtnew [const char*] string to insert.
** @return [AjBool] ajTrue if string was reallocated
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrExchangeSC(AjPStr* Pstr, const AjPStr str,
		       const char* txtnew)
{    
    AjBool cycle = ajTrue;
    ajlong findpos    = 0;

    if(!str || !txtnew)
        return ajFalse;

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
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrExchangeSS(AjPStr* Pstr, const AjPStr str, const AjPStr strnew)
{    
    AjBool cycle = ajTrue;
    ajlong findpos    = 0;
    
    if(!str || !strnew)
        return ajFalse;

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




/* @func ajStrExchangePosCC ***************************************************
**
** Replace one substring with another at a given position in the text.
**
** @param [u] Pstr [AjPStr*]  Target string.
** @param [r] ipos [ajlong] Position in the string, negative values are
**        from the end of the string.
** @param [r] txt [const char*] string to replace.
** @param [r] txtnew [const char*] string to insert.
** @return [AjBool] ajTrue if string was reallocated
**
** @release 5.0.0
** @@
******************************************************************************/

AjBool ajStrExchangePosCC(AjPStr* Pstr, ajlong ipos, const char* txt,
			 const char* txtnew)
{    
    size_t tlen;
    ajlong jpos;

    if(!txt || !txtnew)
        return ajFalse;

    tlen = strlen(txt);
    jpos = ajCvtSposToPosStart((*Pstr)->Len, 0, ipos);
    
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
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrExchangeSetCC(AjPStr* Pstr, const char* txt, const char* txtnew)
{
    char filter[256] = {'\0'};		/* should make all zero */
    size_t i;
    size_t ilen;

    const char *co = NULL;
    const char *cn;
    char* cp;
    AjPStr thys;

    if(!txt || !txtnew)
        return ajFalse;

    if(ajStrFindAnyC(*Pstr, txt) < 0)
        return ajFalse;

    i = strlen(txtnew);

    if(strlen(txt) > i)
	ajErr("ajStrExchangeSetCC new char set '%s' shorter than old '%s'",
	       txt, txtnew);
    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);

    thys = *Pstr;

    cp = thys->Ptr;
    ilen = thys->Len;
    i = strcspn(cp, txt);
    while(i < ilen)
    {
        if(!co)
        {
            co = txt;
            cn = txtnew;
            while(*co)
            {
                if(!*cn)			/* oops, too short, use first */
                    filter[(ajint)*co++] = *txtnew;
                else
                    filter[(ajint)*co++] = *cn++;
            }
        }
        cp += i++;
        *cp = filter[(ajint)*cp];
        ilen -= i;
        i = strcspn(++cp, txt);
    }

/*
  for(cp = thys->Ptr; *cp; cp++)
	if(filter[(ajint)*cp])
	    *cp = filter[(ajint)*cp];
            */
    return ajTrue;
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
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrExchangeSetSS(AjPStr* Pstr, const AjPStr str, const AjPStr strnew)
{
    if(!str || !strnew)
        return ajFalse;

    return ajStrExchangeSetCC(Pstr, str->Ptr, strnew->Ptr);
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
**
** @release 4.1.0
** @@
******************************************************************************/

AjBool ajStrExchangeSetRestCK(AjPStr* Pstr, const char* txt, char chrnew)
{
    char filter[256] = {'\0'};		/* should make all zero */

    const char *co;
    char* cp;
    AjPStr thys;

    if(!txt)
        return ajFalse;

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
      filter[(ajint)toupper((int)*co)] = '1';
      filter[(ajint)tolower((int)*co++)] = '1';
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
**
** @release 4.1.0
** @@
******************************************************************************/

AjBool ajStrExchangeSetRestSK(AjPStr* Pstr, const AjPStr str,
			      char chrnew)
{
    if(!str)
        return ajFalse;

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
**
** @release 1.0.0
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

    size_t len;
    size_t i;

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


    if(!thys->Len)
        return ajFalse;

    return ajTrue;
}




/* @func ajStrReverse *********************************************************
**
** Reverses the order of characters in a string
**
** @param [w] Pstr [AjPStr*] Target string
** @return [AjBool] ajTrue if string was reallocated
**
** @release 4.0.0
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




/* @section query *************************************************************
**
** Functions for querying the properties of strings.
**
** @fdata       [AjPStr]
** @fnote      None.
** @nam3rule   Calc   Calculate a value (not in categories above) derived from
**                    elements of a string object.
** @nam4rule   CalcCount Counts occurrence of character(s) in string.
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
** @nam4rule   IsFilter       Characters defined in filter only
** @nam4rule   IsFloat        Represents float value.
** @nam4rule   IsHex          Represents hex value.
** @nam4rule   IsInt          Represents integer value.
** @nam4rule   IsLong         Represents long value.
** @nam4rule   IsLower        No upper case alphabetic characters.
** @nam4rule   IsNum          Decimal digits only.
** @nam4rule   IsUpper        No lower case alphabetic characters.
** @nam4rule   IsWhite        Whitespace characters only.
** @nam4rule   IsWild         One or more wildcard characters.
** @nam4rule   IsWord         Non-whitespace characters only. 
** @nam3rule   Whole          Specified range covers whole string. 
**
** @argrule * str [const AjPStr] String
** @arg2rule S str2 [const AjPStr] String
** @argrule Whole pos1 [ajlong] Start position, negative values count from end
** @argrule Whole pos2 [ajlong] End position, negative values count from end
** @argrule CountC txt [const char*] Text to count
** @argrule CountK chr [char] Character to count
** @argrule IsCharsetC txt [const char*] Characters to test
** @argrule IsCharsetS str2 [const AjPStr] Characters to test
** @argrule IsCharsetCaseC txt [const char*] Characters to test
** @argrule IsCharsetCaseS str2 [const AjPStr] Characters to test
** @argrule IsFilter filter [const char*] Filter non-zero for characters to test
**
** @valrule * [AjBool] Result of query
** @valrule *Count [ajulong] Number of occurrences
**
** @fcategory use
*/




/* @func ajStrCalcCountC ******************************************************
**
** Counts occurrences of set of characters in a string.
**
** @param [r] str [const AjPStr] String
** @param [r] txt [const char*] Characters to count
** @return [ajulong] Number of times characters were found in string
**
** @release 4.0.0
******************************************************************************/

ajulong ajStrCalcCountC(const AjPStr str, const char* txt)
{
    ajulong ret = 0;
    const char* cp;

    cp = txt;

    while(*cp)
    {
	ret += ajStrCalcCountK(str, *cp);
	++cp;
    }

    return ret;
}




/* @func ajStrCalcCountK ******************************************************
**
** Counts occurrences of a character in a string.
**
** @param [r] str [const AjPStr] String
** @param [r] chr [char] Character to count
** @return [ajulong] Number of times character was found in string
**
** @release 4.0.0
******************************************************************************/

ajulong ajStrCalcCountK(const AjPStr str, char chr)
{
    ajulong ret = 0;
    const char* cp;

    cp = str->Ptr;

    while(*cp)
    {
	if(*cp == chr) ret++;
	cp++;
    }

    return ret;
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
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrHasParentheses(const AjPStr str)
{
    ajulong left = 0;
    const char *cp;

    if(!str)
        return ajFalse;
    
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
**
** @release 1.0.0
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
**
** @release 1.0.0
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
**
** @release 1.0.0
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
**
** @release 4.1.0
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
**
** @release 4.1.0
** @@
******************************************************************************/

AjBool ajStrIsCharsetS(const AjPStr str, const AjPStr str2)
{

    /*
    char filter[256] = {'\0'};
    const char* cp;
    const char* cq;
    */
    if(!str)
	return ajFalse;

    if(!str->Len)
	return ajFalse;

    if(!str2)
      return ajFalse;

/*
    cq = str2->Ptr;

    while (*cq)
      filter[(int)*cq++] = 1;

    cp = str->Ptr;

    while(*cp)
      if(!filter[(int)*cp++])
	    return ajFalse;
    */

    if(strspn(str->Ptr, str2->Ptr) != str->Len)
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
**
** @release 4.1.0
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
    {
        filter[tolower((int)*cq)] = 1;
        filter[toupper((int)*cq++)] = 1;
    }

    cp = str->Ptr;

    while(*cp)
      if(!filter[(int)*cp++])
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
**
** @release 4.1.0
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
    {
        filter[tolower((int)*cq)] = 1;
        filter[toupper((int)*cq++)] = 1;
    }

    cp = str->Ptr;

    while(*cp)
      if(!filter[(int)*cp++])
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
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajStrIsDouble(const AjPStr str)
{
    const char* cp;
    char* ptr = NULL;
    double value;

    if(!str)
	return ajFalse;

    if(!str->Len)
	return ajFalse;

    cp = str->Ptr;

    errno = 0;
    value = strtod(cp, &ptr);
    (void) value;

    if(*ptr || errno == ERANGE)
	return ajFalse;

    return ajTrue;
}




/* @func ajStrIsFilter ********************************************************
**
** Test whether a string contains only characters defined by a filter
** 
** @param [r] str [const AjPStr] String
** @param [r] filter [const char*] Filter non-zero for each allowed value
** @return [AjBool] ajTrue if the string is entirely composed of characters
**                  in the specified set
** @cre an empty string returns ajFalse
**
** @release 6.6.0
** @@
******************************************************************************/

AjBool ajStrIsFilter(const AjPStr str, const char* filter)
{
    const char* cp;

    if(!str)
	return ajFalse;

    if(!str->Len)
	return ajFalse;

    cp = str->Ptr;

    while(*cp)
      if(!filter[(int)*cp++])
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
**
** @release 1.0.0
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
**
** @release 1.13.0
** @@
******************************************************************************/

AjBool ajStrIsHex(const AjPStr str)
{
    const char* cp;
    char* ptr = NULL;
    ajlong value;

    if(!str)
	return ajFalse;

    if(!str->Len)
	return ajFalse;

    cp = str->Ptr;

    errno = 0;
    value = strtol(cp, &ptr, 16);
    (void) value;

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
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajStrIsInt(const AjPStr str)
{
    const char* cp;
    char* ptr = NULL;
    ajlong value;

    if(!str)
	return ajFalse;

    if(!str->Len)
	return ajFalse;

    cp = str->Ptr;

    errno = 0;
    value = strtol(cp, &ptr, 10);

    if(*ptr || errno == ERANGE)
	return ajFalse;

    if(value < INT_MIN)
	return ajFalse;

    if(value > INT_MAX)
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
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajStrIsLong(const AjPStr str)
{
    const char* cp;
    char* ptr = NULL;
    ajlong value;

    if(!str)
	return ajFalse;

    if(!str->Len)
	return ajFalse;

    cp = str->Ptr;

    errno = 0;
    value = strtol(cp, &ptr, 10);
    (void) value;

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
**
** @release 2.7.0
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
**
** @release 3.0.0
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
**
** @release 2.7.0
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




/* @func ajStrIsWhite *********************************************************
**
** Test whether a string contains whitespace characters only.
**
** The test is made using the strtod call in the C RTL.
**
** @param [r] str [const AjPStr] String
** @return [AjBool] ajTrue if the string is only white space (or empty).
**
** @release 2.9.0
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
	if (!isspace((int)*cp))
            return ajFalse;

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
**
** @release 1.0.0
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
**
** @release 1.0.0
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




/* @func ajStrWhole ***********************************************************
**
** Tests whether a range refers to the whole string
**
** @param [r] str [const AjPStr] String
** @param [r] pos1 [ajlong] Begin position (0 start, negative from the end)
** @param [r] pos2 [ajlong] Begin position (0 start, negative from the end)
** @return [AjBool] ajTrue is range covers the whole string
**
** @release 2.5.0
******************************************************************************/

AjBool ajStrWhole(const AjPStr str, ajlong pos1, ajlong pos2)
{
    size_t ibeg;
    size_t iend;

    ibeg = ajCvtSposToPosStart(str->Len, 0, pos1);

    if(ibeg)
	return ajFalse;

    iend = ajCvtSposToPosStart(str->Len, ibeg, pos2);

    if(iend != (str->Len - 1))
	return ajFalse;

    return ajTrue;
}




/* @section filters
**
** Functions for creating filters from C-type (char*) strings).
**
** @fdata     [AjPStr]
**
** @nam3rule  Getfilter   Get a filter array
** @nam4rule  Case        Case-insensitive filter
** @nam4rule  Lower       Lower case filter.
** @nam4rule  Upper       Upper case filter.
**
** @argrule   * str [const AjPStr] String
** @valrule   * [char*] Filter
** @fcategory use
*/




/* @func ajStrGetfilter *******************************************************
**
** Returns a filter array to test for any character in a string.
** 
** @param [r] str [const AjPStr] Character set to test
** @return [char*] Filter array set to 1 for each ascii value accepted
**                  zero for any character not in the set.
**
** @release 6.6.0
** @@
******************************************************************************/

char* ajStrGetfilter(const AjPStr str)
{
    char *ret;
    const char* cp;

    ret = AJALLOC0(256);

    if(!str)
	return ret;

    if(!str->Len)
        return ret;

    cp = str->Ptr;

    while (*cp)
    {
        ret[((int)*cp++)] = 1;
    }

    return ret;
}




/* @func ajStrGetfilterCase ***************************************************
**
** Returns a filter array to test for any character in a string.
** The filter is case-insensitive
** 
** @param [r] str [const AjPStr] Character set to test
** @return [char*] Filter array set to 1 for each ascii value accepted
**                  zero for any character not in the set.
**
** @release 6.6.0
** @@
******************************************************************************/

char* ajStrGetfilterCase(const AjPStr str)
{
    char *ret;
    const char* cp;

    ret = AJALLOC0(256);

    if(!str)
	return ret;

    if(!str->Len)
        return ret;

    cp = str->Ptr;

    while (*cp)
    {
        ret[tolower((int)*cp)] = 1;
        ret[toupper((int)*cp++)] = 1;
    }

    return ret;
}




/* @func ajStrGetfilterLower **************************************************
**
** Returns a filter array to test for any character in a string as lower case.
** 
** @param [r] str [const AjPStr] Character set to test
** @return [char*] Filter array set to 1 for each ascii value accepted
**                  zero for any character not in the set.
**
** @release 6.6.0
** @@
******************************************************************************/

char* ajStrGetfilterLower(const AjPStr str)
{
    char *ret;
    const char* cp;

    ret = AJALLOC0(256);

    if(!str)
	return ret;

    if(!str->Len)
        return ret;

    cp = str->Ptr;

    while (*cp)
    {
        ret[tolower((int)*cp++)] = 1;
    }

    return ret;
}




/* @func ajStrGetfilterUpper **************************************************
**
** Returns a filter array to test for any character in a string as upper case.
** 
** @param [r] str [const AjPStr] Character set to test
** @return [char*] Filter array set to 1 for each ascii value accepted
**                  zero for any character not in the set.
**
** @release 6.6.0
** @@
******************************************************************************/

char* ajStrGetfilterUpper(const AjPStr str)
{
    char *ret;
    const char* cp;

    ret = AJALLOC0(256);

    if(!str)
	return ret;

    if(!str->Len)
        return ret;

    cp = str->Ptr;

    while (*cp)
    {
        ret[toupper((int)*cp++)] = 1;
    }

    return ret;
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
** @nam4rule   GetAscii    Retrieve ASCII character code
** @nam5rule   GetAsciiLow Retrieve lowest ASCII character code
** @nam5rule   GetAsciiHigh  Retrieve highest ASCII character code
** @nam5rule   GetAsciiCommon  Retrieve most common ASCII character code
** @nam4rule   GetChar     Retrieve first single character
** @nam5rule   GetCharFirst     Retrieve first single character
** @nam5rule   GetCharLast     Retrieve last single character
** @nam5rule   GetCharPos    Retrieve a single character
** @nam4rule   GetLen     Retrieve length
** @nam4rule   GetPtr     Retrieve char* pointer
** @nam4rule   GetRes     Retrieve reserved memory size
** @nam4rule   GetPtr         Retrieve additional space available
** @nam4rule   GetRoom         Retrieve additional space available
** @nam4rule   GetUse     Retrieve use (reference) count
** @nam4rule   GetValid     Test string is valid
**
** @argrule * str [const AjPStr] String
** @argrule GetCharPos pos [ajlong] Position, negative values count from the end
**
** @valrule GetChar [char] Single character
** @valrule GetLen [size_t] String length
** @valrule GetRes [size_t] Reserved size
** @valrule GetPtr [const char*] read-only C string pointer to internal string
** @valrule GetuniquePtr [char*] Modifiable C string pointer to internal string
** @valrule GetUse [ajuint] Reference (use) count
** @valrule GetRoom [size_t] Remaining space to extend string
**                           without reallocating
** @valrule GetValid [AjBool] True if string is valid
** @valrule GetAscii [char] ASCII character
**
** @fcategory use
*/




/* @func ajStrGetAsciiCommon **************************************************
**
** Returns the most common ASCII character code in a string.
**
** @param [r] str [const AjPStr] String
** @return [char] Most common character or null character if empty.
**
** @release 6.1.0
** @@
******************************************************************************/

char ajStrGetAsciiCommon(const AjPStr str)
{
    const char *cp;

    ajulong totals[256];
    ajulong maxtot = 0;
    char maxcode = 0;
    int icode;
    ajulong i;

    if(!str)
        return '\0';
    if(!str->Len)
        return '\0';

    for(i=0;i<256;i++)
        totals[i] = 0;

    cp = str->Ptr;

    while (*cp)
    {
        icode = (int) *cp;
        totals[icode]++;
        if(totals[icode] > maxtot)
        {
            maxtot = totals[icode];
            maxcode = (char) icode;
        }
        cp++;
    }

    return maxcode;
}




/* @func ajStrGetAsciiHigh ****************************************************
**
** Returns the highest ASCII character code in a string.
**
** @param [r] str [const AjPStr] String
** @return [char] Highest character or null character if empty.
**
** @release 6.1.0
** @@
******************************************************************************/

char ajStrGetAsciiHigh(const AjPStr str)
{
    char ret = CHAR_MIN;
    const char *cp;

    if(!str)
        return '\0';
    if(!str->Len)
        return '\0';

    cp = str->Ptr;

    while (*cp)
    {
        if(*cp > ret)
            ret = *cp;
        cp++;
    }

    return ret;
}




/* @func ajStrGetAsciiLow *****************************************************
**
** Returns the lowest ASCII character code in a string.
**
** @param [r] str [const AjPStr] String
** @return [char] Lowest character or null character if empty.
**
** @release 6.1.0
** @@
******************************************************************************/

char ajStrGetAsciiLow(const AjPStr str)
{
    char ret = CHAR_MAX;
    const char *cp;

    if(!str)
        return '\0';
    if(!str->Len)
        return '\0';

    cp = str->Ptr;

    while (*cp)
    {
        if(*cp < ret)
            ret = *cp;
        cp++;
    }

    return ret;
}




/* @func ajStrGetCharFirst ****************************************************
**
** Returns the first character from a string.
**
** @param [r] str [const AjPStr] String
** @return [char] First character or null character if empty.
**
** @release 4.0.0
** @@
******************************************************************************/

char ajStrGetCharFirst(const AjPStr str)
{
    if(!str)
	return '\0';

    return str->Ptr[0];
}




/* @func ajStrGetCharLast *****************************************************
**
** Returns the last character from a string.
**
** @param [r] str [const AjPStr] String
** @return [char] Last character or null character if empty.
**
** @release 4.0.0
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
** @param [r] pos [ajlong] Position in the string, negative values are
**        from the end of the string.
** @return [char] Character at position pos or null character if out of range.
**
** @release 4.0.0
** @@
******************************************************************************/

char ajStrGetCharPos(const AjPStr str, ajlong pos)
{
    size_t ipos;

    if(!str)
	return '\0';

    if(pos < 0)
      ipos = str->Len + (size_t) pos;
    else
      ipos = (size_t) pos;

    if(ipos > str->Len)
	return '\0';

    return str->Ptr[ipos];
}




/* @func ajStrGetLen **********************************************************
**
** Returns the current length of the C (char *) string.
**
** The length will remain valid unless the string is resized or deleted.
**
** @param [r] str [const AjPStr] Source string
** @return [size_t] Current string length
**
** @release 4.0.0
** @@
******************************************************************************/

size_t ajStrGetLen(const AjPStr str)
{
    if(!str)
	return 0;


    return str->Len;
}




/* @macro MAJSTRGETLEN ********************************************************
**
** Returns the current length of the C (char *) string. 
**
** A macro version of {ajStrGetLen} available in case it is needed for speed.
**
** @param [r] str [const AjPStr] Source string
** @return [size_t] Current string length
** @@
******************************************************************************/




/* @func ajStrGetPtr **********************************************************
**
** Returns the current pointer to C (char *) string.
**
** The pointer will remain valid unless the string is resized or deleted.
**
** @param [r] str [const AjPStr] Source string
** @return [const char*] Current string pointer, or a null string if undefined.
**
** @release 4.0.0
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




/* @func ajStrGetRes **********************************************************
**
** Returns the current reserved size of the C (char *) string. 
**
** The reserved size will remain valid unless the string is resized or deleted.
**
** @param [r] str [const AjPStr] Source string
** @return [size_t] Current string reserved size
**
** @release 4.0.0
** @@
******************************************************************************/

size_t ajStrGetRes(const AjPStr str)
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
** @return [size_t] Current string reserved size
** @@
******************************************************************************/




/* @func ajStrGetRoom *********************************************************
**
** Returns the additional space available in a string before it would require
** reallocating.
**
** @param [r] str [const AjPStr] String
** @return [size_t] Space available for additional characters.
**
** @release 4.0.0
** @@
******************************************************************************/

size_t ajStrGetRoom(const AjPStr str)
{
    return (str->Res - str->Len - 1);
}




/* @func ajStrGetUse **********************************************************
**
** Returns the current usage count of the C (char *) string. 
**
** The usage count will remain valid unless the string is resized or deleted.
**
** @param [r] str [const AjPStr] Source string
** @return [ajuint] Current string usage count
**
** @release 4.0.0
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




/* @func ajStrGetValid ********************************************************
**
** Checks a string object for consistency. 
**
** Intended for debugging and testing of these routines, but made publicly 
** available.
**
** @param [r] str [const AjPStr] String
** @return [AjBool] ajTrue if no errors were found.
**
** @release 4.0.0
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
**
** @release 4.0.0
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
**
** @release 4.0.0
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
	strClone(Pstr);

    return *Pstr;
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
** @argrule Res size [size_t] Reserved size
** @argrule Len len [size_t] Correct string length
**
** @valrule * [AjBool] True on success
**
** @fcategory modify
*/




/* @func ajStrSetClear ********************************************************
**
** Clears all elements in a string object.
**
** If the string is already empty nothing happens. If the string has data, 
** it makes sure the string is modifiable and sets it to empty.
**
** @param  [w] Pstr [AjPStr*] Pointer to the string to be deleted.
**         The pointer is always deleted.
** @return [AjBool] ajTrue if string was reallocated
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrSetClear(AjPStr* Pstr)
{
    AjPStr thys;

    if(!*Pstr)
        return ajTrue;
    
    thys = *Pstr;

    if(thys->Use > 1)
    {
        thys->Use--;
        *Pstr  = ajStrNew();
        return ajTrue;
    }

    thys->Ptr[0] = '\0';
    thys->Len = 0;

    return ajTrue;
}




/* @func ajStrSetRes **********************************************************
**
** Ensures a string is modifiable and big enough for its intended purpose.
**
** The target string is guaranteed to have a reference count of 1 and a 
** minimum reserved size.
**
** @param [u] Pstr [AjPStr*] String
** @param [r] size [size_t] Minimum reserved size.
** @return [AjBool] ajTrue if the string was reallocated
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrSetRes(AjPStr* Pstr, size_t size)
{
    AjPStr thys;
    size_t savesize;

    savesize = size; /* often part of *Pstr, about to vanish */

    if(!*Pstr)
    {
	*Pstr = ajStrNewRes(savesize);

	return ajTrue;
    }

    thys = *Pstr;

    if(thys->Use > 1)
    {
	strCloneL(Pstr, savesize);

	return ajTrue;
    }

    if(thys->Res < savesize)
    {
#ifdef AJ_SAVESTATS
        strExtra += savesize - thys->Res;
#endif
	AJRESIZE0(thys->Ptr, thys->Res, savesize);
	thys->Res = savesize;

	return ajTrue;
    }

    return ajFalse;
}




/* @func ajStrSetResRound *****************************************************
**
** Ensures a string is modifiable and big enough for its intended purpose. 
**
** The target string is guaranteed to have a reference count of 1,
** and a minimum reserved size.  The string size is rounded up.
**
** @param [w] Pstr [AjPStr*] String
** @param [r] size [size_t] Minimum reserved size.
** @return [AjBool] ajTrue if the string was reallocated
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrSetResRound(AjPStr* Pstr, size_t size)
{
    AjPStr thys;
    size_t trysize;
    size_t roundsize;

    if(!*Pstr)
    {
	roundsize = ajRound(size, STRSIZE);
	*Pstr = ajStrNewRes(roundsize);

	return ajTrue;
    }

    thys = *Pstr;

    if(thys->Use > 1)
    {
	strCloneL(Pstr, size);

	return ajTrue;
    }

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

#ifdef AJ_SAVESTATS
        strExtra += roundsize - thys->Res;
#endif
        AJRESIZE0(thys->Ptr, thys->Res, roundsize);


        thys->Res = roundsize;

	return ajTrue;
    }

    return ajFalse;
}




/* @func ajStrSetValid ********************************************************
**
** Reset string length when some nasty caller may have edited it
**
** @param [u] Pstr [AjPStr*] String.
** @return [AjBool] True on success.
**
** @release 4.0.0
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




/* @func ajStrSetValidLen *****************************************************
**
** Reset string length when some nasty caller may have edited it
**
** @param [u] Pstr [AjPStr*] String
** @param [r] len [size_t] Length expected.
** @return [AjBool] True on success
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrSetValidLen(AjPStr* Pstr, size_t len)
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




/* @section string to datatype conversion functions ***************************
**
** Functions for converting strings to other datatypes.
**
** @fdata      [AjPStr]
** @fnote     None.
**
** @nam3rule  To            Convert string value to a different datatype.
** @nam4rule  ToBool        Convert to boolean
** @nam4rule  ToDouble      Convert to double
** @nam4rule  ToFloat       Convert to float
** @nam4rule  ToHex         Convert to hexadecimal
** @nam4rule  ToInt         Convert to integer
** @nam4rule  ToLong        Convert to long
** @nam4rule  ToUint        Convert to unsigned integer
** @nam4rule  ToUlong       Convert to unsigned long integer
**
** @argrule * str [const AjPStr] String
** @argrule ToBool Pval [AjBool*] Boolean return value
** @argrule ToDouble Pval [double*] Double return value
** @argrule ToFloat Pval [float*] Float return value
** @argrule ToHex Pval [ajint*] Integer return value
** @argrule ToInt Pval [ajint*] Integer return value
** @argrule ToLong Pval [ajlong*] Long integer return value
** @argrule ToUint Pval [ajuint*] Unsigned integer return value
** @argrule ToUlong Pval [ajulong*] Unsigned long integer return value
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
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajStrToBool(const AjPStr str, AjBool* Pval)
{
    const char* cp;
    size_t i;

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
**
** @release 1.0.0
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

#ifdef WIN32
    if(ajStrMatchC(str, "NaN") || ajStrMatchC(str, "NAN"))
    {
        *Pval = 0.0;
        return ajTrue;
    }
#endif

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
**
** @release 1.0.0
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
#ifdef WIN32
    if(ajStrMatchC(str, "NaN") || ajStrMatchC(str, "NAN"))
    {
        *Pval = 0.0;
        return ajTrue;
    }
#endif

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
**
** @release 1.13.0
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
**
** @release 1.0.0
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
**
** @release 1.0.0
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
** Uses the strtoul call in the C RTL from  C89 onwards.
**
** @param [r] str [const AjPStr] String
** @param [w] Pval [ajuint*] String represented as an unsigned integer.
** @return [AjBool] ajTrue if the string had a valid unsigned integer value.
** @cre an empty string returns ajFalse.
** @see ajStrIsInt
**
** @release 4.1.0
** @@
******************************************************************************/

AjBool ajStrToUint(const AjPStr str, ajuint* Pval)
{
    AjBool ret = ajFalse;
    const char* cp;
    ajulong l;
    char* ptr;

    *Pval = 0;

    if(!str)
	return ret;

    if(!str->Len)
	return ret;

    cp = str->Ptr;

    errno = 0;
    l = strtoul(cp, &ptr, 10);

    if(!*ptr && errno != ERANGE)
    {
	*Pval =(ajuint) l;
	ret = ajTrue;
    }

    return ret;
}




/* @func ajStrToUlong *********************************************************
**
** Converts a string into an unsigned long integer value.
**
** Uses the strtoul call in the C RTL.
**
** @param [r] str [const AjPStr] String
** @param [w] Pval [ajulong*] String represented as an integer.
** @return [AjBool] ajTrue if the string had a valid integer value.
** @cre an empty string returns ajFalse.
** @see ajStrIsInt
**
** @release 6.3.0
** @@
******************************************************************************/

AjBool ajStrToUlong(const AjPStr str, ajulong* Pval)
{
    AjBool ret = ajFalse;
    const char* cp;
    ajulong l;
    char* ptr;

    *Pval = 0;

    if(!str)
	return ret;

    if(!str->Len)
	return ret;

    cp = str->Ptr;

    errno = 0;
    l = strtoul(cp, &ptr, 10);

    if(!*ptr && errno != ERANGE)
    {
	*Pval = l;
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
** @nam3rule  From           Convert a datatype value to a string.
** @nam4rule  FromBool       Convert from boolean
** @nam4rule  FromDouble     Convert from double
** @nam5rule  FromDoubleExp  Convert from double in exponential form
** @nam4rule  FromFloat      Convert from float
** @nam4rule  FromInt        Convert from integer
** @nam4rule  FromLong       Convert from long
** @nam4rule  FromUint       Convert from unsigned integer
** @nam4rule  FromVoid       Convert from void pointer if it appears to be
**                           a string
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
** @argrule FromVoid vval [const void*] Void pointer value
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
**
** @release 1.0.0
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
**
** @release 1.0.0
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
**
** @release 4.0.0
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
**
** @release 1.0.0
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
**
** @release 1.0.0
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
**
** @release 1.0.0
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

    ret = ajStrSetResRound(Pstr, (size_t) i);

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
**
** @release 4.1.0
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




/* @func ajStrFromVoid ********************************************************
**
** Tests a void pointer and if it is potentially an ASCII string
** saves it as the output string.
** 
** The string size is set to be just large enough to hold the value.
**
** Intended to allow possible strings to be reported in tables and lists
** where they must be held as void* pointers
**
** @param [w] Pstr [AjPStr*] Target string
** @param [r] vval [const void*] Integer value
** @return [AjBool] ajTrue if string was found
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajStrFromVoid(AjPStr* Pstr, const void* vval)
{
    const AjPStr str = (const AjPStr) vval;

    size_t ires;
    size_t ilen;

    ajStrAssignC(Pstr, "<not a string>");

    ires = str->Res;
    ilen = str->Len;

    if((ilen+1) > ires)
        return ajFalse;

    if(ires > 4096)
        return ajFalse;

    if(str->Ptr[ilen])
        return ajFalse;

    ajStrAssignS(Pstr, str);

    return ajTrue;
}




/* @section formatting ********************************************************
**
** Functions for formatting strings.
**
** @fdata      [AjPStr]
** @fnote     Same namrule as "String formatting functions: C-type (char*)
**            strings".
** @nam3rule  Fmt         Change the format of a string.
** @nam4rule  FmtBlock    Format in blocks
** @nam4rule  FmtCapital  Convert to captal for start of each word
** @nam4rule  FmtLower    Convert to lower case.
** @nam5rule  FmtLowerSub Convert sub-string.
** @nam4rule  FmtPercent  URL percent-encoding
** @nam5rule  FmtPercentDecode  Decode URL percent-encoding
** @nam5rule  FmtPercentEncode  Encode using URL percent-encoding
** @nam4rule  FmtQuery    Convert to a standard EMBOSS query in lower case
**                        with white space replaced by an underscore
** @nam4rule  FmtQuote    Enclose in double quotes
** @nam4rule  FmtTitle    Convert first character of string to upper case.
** @nam4rule  FmtUpper    Convert to upper case.
** @nam5rule  FmtUpperSub Substring only
** @nam4rule  FmtWord     Make into a word by replacing whitespace
** @nam4rule  FmtWrap     Wrap with newlines
** @nam5rule  FmtWrapAt   Wrap with newlines at a preferred character
** @nam5rule  FmtWrapLeft Wrap with newlines and left margin of spaces
** @suffix    C           C string
** @suffix    S           String object
**
** @argrule * Pstr [AjPStr*] String
** @argrule FmtBlock len [ajulong] Block length
** @argrule Sub pos1 [ajlong] Start position, negative value counts from end
** @argrule Sub pos2 [ajlong] End position, negative value counts from end
** @argrule Wrap width [ajuint] Line length
** @argrule WrapAt ch [char] Preferred last character on line
** @argrule WrapLeft margin [ajuint] Left margin
** @argrule WrapLeft indent [ajuint] Indentation on later lines
** @argrule EncodeC txt [const char*] Characters to encode
** @argrule EncodeS str [const AjPStr] Characters to encode
**
** @valrule * [AjBool] True on success
**
** @fcategory modify
*/




/* @func ajStrFmtBlock ********************************************************
**
** Splits a string into words (blocks) of a given size by inserting spaces.
**
** Mainly intended for sequence output formats
**
** @param [u] Pstr [AjPStr*] String.
** @param [r] len [ajulong] Block size
** @return [AjBool] ajTrue on success
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrFmtBlock(AjPStr* Pstr, ajulong len)
{
    size_t i;
    char* cp;
    char* cq;
    AjPStr thys;
    ajuint j;

    i = (size_t) ((*Pstr)->Len + ((*Pstr)->Len-1)/len);
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




/* @func ajStrFmtCapital ******************************************************
**
** Converts the first character of each word in a string to upper case.
**
** @param [u] Pstr [AjPStr*] String
** @return [AjBool] ajTrue if string was reallocated
**
** @release 6.6.0
** @@
******************************************************************************/

AjBool ajStrFmtCapital(AjPStr* Pstr)
{
    AjPStr thys;
    char* cp;
    AjBool wasspace = ajTrue;

    ajStrFmtLower(Pstr);

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);

    thys = *Pstr;

    cp = thys->Ptr;

    while(*cp)
    {
        if(wasspace && isalpha((int)*cp))
	{
	    *cp = (char) toupper((int)*cp);
            wasspace = ajFalse;

	}

        if(isspace((int)*cp))
            wasspace = ajTrue;

	cp++;
    }

    return ajFalse;
}




/* @func ajStrFmtLower ********************************************************
**
** Converts a string to lower case. 
**
** If the string has multiple references, a new string is made first.
**
** @param [u] Pstr [AjPStr*] String
** @return [AjBool] ajTrue if string was reallocated
**
** @release 4.0.0
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




/* @func ajStrFmtLowerSub *****************************************************
**
** Converts a substring of a string to lower case.
**
** @param [u] Pstr [AjPStr*] String
** @param [r] pos1 [ajlong] start position for conversion
** @param [r] pos2 [ajlong] end position for conversion
** @return [AjBool] ajTrue if string was reallocated
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrFmtLowerSub(AjPStr* Pstr, ajlong pos1, ajlong pos2)
{
    AjPStr thys;
    size_t ibegin;
    size_t iend;
    size_t i;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);

    thys = *Pstr;

    ibegin = ajCvtSposToPosStart(thys->Len, 0, pos1);
    iend = ajCvtSposToPosStart(thys->Len, ibegin, pos2);

    for (i=ibegin; i<=iend;i++)
	thys->Ptr[i] = (char)tolower((int) thys->Ptr[i]);

    return ajTrue;
}




/* @func ajStrFmtPercentDecode ************************************************
**
** Converts percent-encoded characters
**
** @param [u] Pstr [AjPStr *] string
**
** @return [AjBool] True if string is not empty.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajStrFmtPercentDecode(AjPStr* Pstr)
{
    AjPStr thys;

    char *cp;
    char *cq;
    int ia;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);

    thys = *Pstr;
    cp = thys->Ptr;
    cq = thys->Ptr;

    while(*cp) 
    {
        if(*cp != '%')
        {
            *cq++ = *cp++;
        }
        else 
        {
            ia = 0;
            cp++;
            if(!*cp) break;
            if(*cp >= '0' && *cp <= '9')
                ia += 16 * (*cp - '0');
            else if(*cp >= 'A' && *cp <= 'F')
                ia += 16 * (10 + (*cp - 'A'));
            else if(*cp >= 'a' && *cp <= 'f')
                ia += 16 * (10 + (*cp - 'a'));
            cp++;
            if(!*cp) break;
            if(*cp >= '0' && *cp <= '9')
                ia += (*cp - '0');
            else if(*cp >= 'A' && *cp <= 'F')
                ia += (10 + (*cp - 'A'));
            else if(*cp >= 'a' && *cp <= 'f')
                ia += (10 + (*cp - 'a'));
            *cq++ = (char) ia;
            cp++;
        }
    }

    *cq = '\0';
    thys->Len = cq - thys->Ptr;

    return ajTrue;
}




/* @func ajStrFmtPercentEncodeC ***********************************************
**
** Percent-encoded specified characters
**
** @param [u] Pstr [AjPStr *] string
** @param [r] txt [const char*] Characters to encode
**
** @return [AjBool] True if string is not empty.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajStrFmtPercentEncodeC(AjPStr* Pstr, const char* txt)
{
    AjPStr thys;
    AjPStr copy;

    char *cp;
    char *cq;
    int ia;
    int ip;
    int iq;
    size_t ir;
    ajuint i;
    ajuint npct;

    AjBool changed = ajFalse;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);

    thys = *Pstr;
    npct = (ajuint) ajStrCalcCountC(thys, txt);
    if(!npct)
        return ajTrue;

    copy = ajStrNewS(thys);
    ajStrSetRes(Pstr, thys->Len + 3*npct);
    thys = *Pstr;
    
    cp = copy->Ptr;
    cq = thys->Ptr;

    while(*cp) 
    {
        ir = strcspn(cp, txt);

        for(i=0; i < ir; i++)
            *cq++ = *cp++;

        if(*cp)
        {
            changed = ajTrue;
            ia = (int) *cp;
            ip = ia / 16;
            iq = ia % 16;

            *cq++ = '%';

            if(ip < 10)
                *cq++ = '0' + ip;
            else
                *cq++ = 'A' - 10 + ip;

            if(iq < 10)
                *cq++ = '0' + iq;
            else
                *cq++ = 'A' - 10 + iq;

            cp++;
        }
    }

    *cq = '\0';
    thys->Len = cq - thys->Ptr;

    if(changed)
        ajDebug("ajStrFmtPercentEncodeC changed '%s' '%S' => '%S'\n",
                txt, copy, thys);

    ajStrDel(&copy);

    return ajTrue;
}




/* @func ajStrFmtPercentEncodeS ***********************************************
**
** Percent-encoded specified characters
**
** @param [u] Pstr [AjPStr *] string
** @param [r] str [const AjPStr] Characters to encode
**
** @return [AjBool] True if string is not empty.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajStrFmtPercentEncodeS(AjPStr* Pstr, const AjPStr str)
{
    return ajStrFmtPercentEncodeC(Pstr, MAJSTRGETPTR(str));
}




/* @func ajStrFmtQuery ********************************************************
**
** Ensures a string is a standard EMBOSS query term with all
** alphabetic characters in lower case, and any non alphanumeric characters
** replaced by an underscore
**
** @param [u] Pstr [AjPStr *] string
**
** @return [AjBool] True if string is not empty.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajStrFmtQuery(AjPStr* Pstr)
{
    AjPStr thys;

    static char filter[256];
    static int spaces[256];
    static AjBool called = AJFALSE;

    char *cp;
    char *cq;

    int c;
    int ispace = 0;
    int iextra = 0;

    if(!called)
    {
        for(c=0; c<256; c++)
        {
            if(isalnum(c) || strchr("*?.", c))
            {
                spaces[c] = 0;
                if(isupper(c))
                    filter[c] = tolower(c);
                else
                    filter[c] = c;
            }
            else
            {
                spaces[c] = 1;
                filter[c] = '_';
            }
        }

        spaces[0] = 0;

        called = ajTrue;
    }

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);

    thys = *Pstr;
    cp = thys->Ptr;
    cq = thys->Ptr;

    while(*cp) 
    {
        while(spaces[(int)*cp])
        {
            ispace++;
            cp++;
        }
        if(ispace)
        {
            iextra += ispace-1;
            *cq++ = '_';
            ispace = 0;
        }

        while(*cp && !spaces[(int)*cp])
            *cq++ = filter[(int)*cp++];
    }
    
    *cq = '\0';
    thys->Len -= iextra;

    return ajTrue;
}




/* @func ajStrFmtQuote ********************************************************
**
** Ensures a string is quoted with double quotes.
**
** @param [u] Pstr [AjPStr *] string
**
** @return [AjBool] True if string is not empty.
**                  As it is at least quoted, it will always return true.
**
** @release 4.0.0
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

    if(!(*Pstr)->Len)
        return ajFalse;

    return ajTrue;
}




/* @func ajStrFmtTitle ********************************************************
**
** Converts the first character of a string to upper case.
**
** @param [u] Pstr [AjPStr*] String
** @return [AjBool] ajTrue if string was reallocated
**
** @release 4.0.0
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

    while(*cp)
    {
      if(isalpha((int)*cp))
	{
	    *cp = (char) toupper((int)*cp);

	    return ajTrue;
	}

	cp++;
    }

    return ajFalse;
}




/* @func ajStrFmtUpper ********************************************************
**
** Converts a string to upper case. 
**
** If the string has multiple references, a new string is made first.
**
** @param [u] Pstr [AjPStr*] String
** @return [AjBool] ajTrue if string was reallocated
**
** @release 4.0.0
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




/* @func ajStrFmtUpperSub *****************************************************
**
** Converts a substring of a string to upper case.
**
** @param [u] Pstr [AjPStr*] String
** @param [r] pos1 [ajlong] start position for conversion
** @param [r] pos2 [ajlong] end position for conversion
** @return [AjBool] ajTrue if string was reallocated
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrFmtUpperSub(AjPStr* Pstr, ajlong pos1, ajlong pos2)
{
    AjPStr thys;
    size_t ibegin;
    size_t iend;
    size_t i;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);

    thys = *Pstr;

    ibegin = ajCvtSposToPosStart(thys->Len, 0, pos1);
    iend = ajCvtSposToPosStart(thys->Len, ibegin, pos2);

    for (i=ibegin; i<=iend;i++)
	thys->Ptr[i] = (char)toupper((int) thys->Ptr[i]);

    return ajTrue;
}




/* @func ajStrFmtWord *********************************************************
**
** Converts a string to a single word. Spaces are converted to underscores.
** Leading substrings ending in a colon are removed.
**
** @param [u] Pstr [AjPStr*] String
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajStrFmtWord(AjPStr* Pstr)
{
    ajlong colpos;

    if(!*Pstr)
        *Pstr = ajStrNewResLenC("", 1, 0);
    else if((*Pstr)->Use > 1)
        ajStrGetuniqueStr(Pstr);

    if(!ajStrIsWord(*Pstr))
    {
	/* ajDebug("ajStrFmtWord non-word '%S'\n", *Pstr); */
	ajStrRemoveWhiteExcess(Pstr);
	ajStrExchangeKK(Pstr, ' ', '_');
	/* ajDebug("ajStrFmtWord cleaned '%S'\n", *Pstr); */
    }

    /* ajDebug("ajStrFmtWord word '%S'\n", *Pstr); */
    if(ajStrGetCharLast(*Pstr) == ':')
        ajStrTrimEndC(Pstr, ":");

    colpos = 1 +  ajStrFindlastK(*Pstr, ':');

    if(colpos)
        ajStrKeepRange(Pstr, colpos, -1);

    return ajTrue;
}




/* @func ajStrFmtWrap *********************************************************
**
** Formats a string so that it wraps when printed.  
**
** Newline characters are inserted, at white space if possible,
** with a break at whitespace following the preferred character
** if found, or at the last whitespace, or just at the line width if there
** is no whitespace found (it does happen with long hyphenated enzyme names)
**
** @param [u] Pstr [AjPStr*] Target string
** @param [r] width [ajuint] Line width
** @return [AjBool] ajTrue on successful completion else ajFalse;
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrFmtWrap(AjPStr* Pstr, ajuint width)
{
    AjPStr thys;
    char* cq;
    size_t i;
    size_t j;
    size_t k;
    size_t imax;

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




/* @func ajStrFmtWrapAt *******************************************************
**
** Formats a string so that it wraps when printed.
** Breaks are at a preferred character (for example ',' for author lists)
**
** Newline characters are inserted, at white space if possible,
** with a break at whitespace following the preferred character
** if found, or at the last whitespace, or just at the line width if there
** is no whitespace found (it does happen with long hyphenated enzyme names)
**
** @param [u] Pstr [AjPStr*] Target string
** @param [r] width [ajuint] Line width
** @param [r] ch [char] Preferred last character on line
** @return [AjBool] ajTrue on successful completion else ajFalse;
**
** @release 5.0.0
** @@
******************************************************************************/

AjBool ajStrFmtWrapAt(AjPStr* Pstr, ajuint width, char ch)
{
    AjPStr thys;
    char* cq;
    size_t i;
    size_t j;
    size_t k;
    size_t kk;
    size_t imax;

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
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrFmtWrapLeft(AjPStr* Pstr, ajuint width,
			ajuint margin, ajuint indent)
{
    AjPStr newstr = NULL;
    char* cp;
    size_t len;
    size_t i   = 0;
    size_t j;
    size_t isp = 0;
    size_t leftmargin = margin;
    size_t maxwidth = width + indent;

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
**
** @release 1.0.0
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
**
** @release 4.0.0
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




/* @func ajStrMatchCaseC ******************************************************
**
** Simple case-insensitive test for matching a string and a text string.
**
** @param [r] str [const AjPStr] String
** @param [r] txt2 [const char*] Text
** @return [AjBool] ajTrue if two strings are exactly the same excluding case
**
** @release 1.0.0
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
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrMatchCaseS(const AjPStr str, const AjPStr str2)
{
    if(!str || !str2)
	return ajFalse;

    return ajCharMatchCaseC(str->Ptr, str2->Ptr);
}




/* @func ajStrMatchWildC ******************************************************
**
** Simple case-sensitive test for matching a string and a text string using
** wildcard characters.
**
** @param [r] str [const AjPStr] String
** @param [r] txt2 [const char*] Wildcard text
** @return [AjBool] ajTrue if the strings match
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajStrMatchWildC(const AjPStr str, const char* txt2)
{
    return ajCharMatchWildC(MAJSTRGETPTR(str), txt2);
}




/* @func ajStrMatchWildS ******************************************************
**
** Simple case-sensitive test for matching two strings using wildcard 
** characters.
**
** @param [r] str [const AjPStr] String
** @param [r] str2 [const AjPStr] Wildcard string
** @return [AjBool] ajTrue if two strings match
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrMatchWildS(const AjPStr str, const AjPStr str2)
{
    return ajCharMatchWildC(MAJSTRGETPTR(str), MAJSTRGETPTR(str2));
}




/* @func ajStrMatchWildCaseC **************************************************
**
** Simple case-insensitive test for matching a string and a text string using
** wildcard characters.
**
** @param [r] str [const AjPStr] String
** @param [r] txt2 [const char*] Wildcard text
** @return [AjBool] ajTrue if the strings match
**
** @release 4.1.0
** @@
******************************************************************************/

AjBool ajStrMatchWildCaseC(const AjPStr str, const char* txt2)
{
    return ajCharMatchWildCaseC(MAJSTRGETPTR(str), txt2);
}




/* @func ajStrMatchWildCaseS **************************************************
**
** Simple case-insensitive test for matching two strings using wildcard 
** characters.
**
** @param [r] str [const AjPStr] String
** @param [r] str2 [const AjPStr] Wildcard string
** @return [AjBool] ajTrue if two strings match
**
** @release 4.1.0
** @@
******************************************************************************/

AjBool ajStrMatchWildCaseS(const AjPStr str, const AjPStr str2)
{
    return ajCharMatchWildCaseC(MAJSTRGETPTR(str), MAJSTRGETPTR(str2));
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
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrMatchWildWordC(const AjPStr str, const char* txt2)
{
    return ajCharMatchWildWordC(MAJSTRGETPTR(str), txt2);
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
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrMatchWildWordS(const AjPStr str, const AjPStr str2)
{
    return ajCharMatchWildWordC(MAJSTRGETPTR(str), MAJSTRGETPTR(str2));
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
**
** @release 4.1.0
** @@
******************************************************************************/

AjBool ajStrMatchWildWordCaseC(const AjPStr str, const char* txt2)
{
    return ajCharMatchWildWordCaseC(MAJSTRGETPTR(str), txt2);
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
**
** @release 4.1.0
** @@
******************************************************************************/

AjBool ajStrMatchWildWordCaseS(const AjPStr str, const AjPStr str2)
{
    return ajCharMatchWildWordCaseC(MAJSTRGETPTR(str), MAJSTRGETPTR(str2));
}




/* @func ajStrMatchWordAllS ***************************************************
**
** Test for matching all words within a string.
** Matches can be to parts of words in the original string.
**
** 'Word' is defined as no white space.
**
** @param [r] str [const AjPStr] String
** @param [r] str2 [const AjPStr] Text
** @return [AjBool]  ajTrue if found
**
** @release 4.0.0
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




/* @func ajStrMatchWordOneS ***************************************************
**
** Test for matching a word within a string.
** Matches can be to parts of words in the original string.
**
** 'Word' is defined as no white space.
**
** @param [r] str [const AjPStr] String
** @param [r] str2 [const AjPStr] Text
** @return [AjBool]  ajTrue if found
**
** @release 4.0.0
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
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajStrPrefixC(const AjPStr str, const char* txt2)
{
    size_t ilen;

    if(!str)
	return ajFalse;

    ilen = strlen(txt2);

    if(!ilen)				/* no prefix */
	return ajFalse;

    if(ilen > MAJSTRGETLEN(str))		/* prefix longer */
	return ajFalse;

    if(strncmp(MAJSTRGETPTR(str), txt2, ilen)) /* +1 or -1 for a failed match */
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
**
** @release 4.0.0
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

    if(str2->Len > MAJSTRGETLEN(str))	/* prefix longer */
	return ajFalse;

    if(strncmp(MAJSTRGETPTR(str),
               MAJSTRGETPTR(str2), MAJSTRGETLEN(str2))) /* +1 or -1 for a
                                                           failed match */
	return ajFalse;

    return ajTrue;
}




/* @func ajStrPrefixCaseC *****************************************************
**
** Case-insensitive test for matching the start of a string against a 
** given prefix text string.
**
** @param [r] str [const AjPStr] String
** @param [r] txt2 [const char*] Prefix
** @return [AjBool] ajTrue if the string begins with the prefix
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajStrPrefixCaseC(const AjPStr str, const char* txt2)
{
    return ajCharPrefixCaseC(MAJSTRGETPTR(str), txt2);
}




/* @func ajStrPrefixCaseS *****************************************************
**
** Case-insensitive test for matching the start of a string against a 
** given prefix string.
**
** @param [r] str [const AjPStr] String
** @param [r] str2 [const AjPStr] Prefix
** @return [AjBool] ajTrue if the string begins with the prefix
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrPrefixCaseS(const AjPStr str, const AjPStr str2)
{
    return ajCharPrefixCaseC(MAJSTRGETPTR(str),
                             MAJSTRGETPTR(str2));
}




/* @func ajStrSuffixC *********************************************************
**
** Test for matching the end of a string against a given suffix text string.
**
** @param [r] str [const AjPStr] String
** @param [r] txt2 [const char*] Suffix as text
** @return [AjBool] ajTrue if the string ends with the suffix
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajStrSuffixC(const AjPStr str, const char* txt2)
{
    size_t ilen;
    size_t istart;

    if(!str)
	return ajFalse;

    ilen   = strlen(txt2);
    istart = str->Len - ilen;

    if(ilen > MAJSTRGETLEN(str))		/* suffix longer */
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
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrSuffixS(const AjPStr str, const AjPStr str2)
{
    size_t ilen;
    size_t istart;

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




/* @func ajStrSuffixCaseC *****************************************************
**
** Case-insensitive test for matching the end of a string against a 
** given suffix text string.
**
** @param [r] str [const AjPStr] String
** @param [r] txt2 [const char*] Prefix
** @return [AjBool] ajTrue if the string ends with the suffix
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrSuffixCaseC(const AjPStr str, const char* txt2)
{
    return ajCharSuffixCaseC(MAJSTRGETPTR(str), txt2);
}




/* @func ajStrSuffixCaseS *****************************************************
**
** Case-insensitive test for matching the end of a string against a 
** given suffix string.
**
** @param [r] str [const AjPStr] String
** @param [r] str2 [const AjPStr] Prefix
** @return [AjBool] ajTrue if the string ends with the suffix
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrSuffixCaseS(const AjPStr str, const AjPStr str2)
{
    return ajCharSuffixCaseC(MAJSTRGETPTR(str),
                             MAJSTRGETPTR(str2));
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
** @argrule Len len [size_t] Number of characters to compare
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
**
** @release 1.0.0
** @@
******************************************************************************/

int ajStrCmpC(const AjPStr str, const char* txt2)
{
    return strcmp(MAJSTRGETPTR(str), txt2);
}




/* @func ajStrCmpLenC *********************************************************
**
** Finds the sort order of a string and a text string comparing the first 
** n characters only.
**
** @param [r] str [const AjPStr] String object
** @param [r] txt2 [const char*] Text string
** @param [r] len [size_t] Length to compare
** @return [int] -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
**
** @release 4.0.0
** @@
******************************************************************************/

int ajStrCmpLenC(const AjPStr str, const char* txt2, size_t len)
{
    return strncmp(MAJSTRGETPTR(str), txt2, len);
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
**
** @release 4.0.0
** @@
******************************************************************************/

int ajStrCmpS(const AjPStr str, const AjPStr str2)
{
    return strcmp(MAJSTRGETPTR(str),
                  MAJSTRGETPTR(str2));
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
**
** @release 4.0.0
** @@
******************************************************************************/

int ajStrCmpCaseS(const AjPStr str, const AjPStr str2)
{
    const char* cp;
    const char* cq;

    if(!str)
    {
        if(!str2) return 0;
        else return -1;
    }
    if(!str2) return 1;

    for(cp = MAJSTRGETPTR(str), cq = MAJSTRGETPTR(str2);
        *cp && *cq; cp++, cq++)
	if(toupper((int) *cp) != toupper((int) *cq))
	{
	    if(toupper((int) *cp) > toupper((int) *cq))
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




/* @func ajStrCmpLenS *********************************************************
**
** Finds the sort order of two strings comparing the first
** n characters only.

** Compares the first n characters of two strings
**
** @param [r] str [const AjPStr] String object
** @param [r] str2 [const AjPStr] Second string object
** @param [r] len [size_t] Length to compare
** @return [int] -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
**
** @release 4.0.0
** @@
******************************************************************************/

int ajStrCmpLenS(const AjPStr str, const AjPStr str2, size_t len)
{
    if(!str)
    {
        if(!str2) return 0;
        else return -1;
    }
    if(!str2) return 1;

    return strncmp(MAJSTRGETPTR(str), MAJSTRGETPTR(str2), len);
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
**
** @release 1.0.0
** @@
******************************************************************************/

int ajStrCmpWildC(const AjPStr str, const char* txt2)
{
    if(!str)
    {
        if(!txt2) return 0;
        else return -1;
    }
    if(!txt2) return 1;

    return ajCharCmpWild(MAJSTRGETPTR(str), txt2);
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
**
** @release 4.0.0
** @@
******************************************************************************/

int ajStrCmpWildS(const AjPStr str, const AjPStr str2)
{
    if(!str)
    {
        if(!str2) return 0;
        else return -1;
    }
    if(!str2) return 1;

    return ajCharCmpWild(MAJSTRGETPTR(str), MAJSTRGETPTR(str2));
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
**
** @release 4.1.0
** @@
******************************************************************************/

int ajStrCmpWildCaseC(const AjPStr str, const char* txt2)
{
    if(!str)
    {
        if(!txt2) return 0;
        else return -1;
    }
    if(!txt2) return 1;

    return ajCharCmpWildCase(MAJSTRGETPTR(str), txt2);
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
**
** @release 4.1.0
** @@
******************************************************************************/

int ajStrCmpWildCaseS(const AjPStr str, const AjPStr str2)
{
    if(!str)
    {
        if(!str2) return 0;
        else return -1;
    }
    if(!str2) return 1;

    return ajCharCmpWildCase(MAJSTRGETPTR(str), MAJSTRGETPTR(str2));
}




/* @func ajStrVcmp ************************************************************
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
**
** @release 4.0.0
** @@
******************************************************************************/

int ajStrVcmp(const void* str, const void* str2)
{
    if(!str)
    {
        if(!str2) return 0;
        else return -1;
    }
    if(!str2) return 1;

    return strcmp((*(AjPStr const *)str)->Ptr, (*(AjPStr const *)str2)->Ptr);
}




/* @section comparison (search) functions *************************************
**
** Functions for finding substrings or characters in strings.
**
** @fdata      [AjPStr]
** @fnote     None.
**
** @nam3rule  Find          Locate first occurrence of a string
**                          within another string. 
** @nam4rule  FindAny       Any in a set of characters from the start
** @nam4rule  FindCase      Case insensitive
** @nam4rule  FindNext      Next in a set of characters from a given position
** @nam4rule  FindRest      Any not in a set of characters
** @nam5rule  FindRestCase  Any not in a set of characters, case insensitive
** @nam3rule  Findlast      Locate last occurrence of a string
**
** @argrule * str [const AjPStr] String
** @argrule FindNext pos1 [ajlong] String position to search from
** @argrule C txt2 [const char*] Text to find
** @argrule K chr [char] Character
** @argrule S str2 [const AjPStr] Text to find
**
** @valrule * [ajlong] Position of the start of text in string if found.
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
** @return [ajlong] Position of the start of text in string if found.
**                Or -1 for text not found.
**
** @release 1.0.0
** @@
******************************************************************************/

ajlong ajStrFindC(const AjPStr str, const char* txt2)
{
    const char* cp;

    cp = strstr(MAJSTRGETPTR(str), txt2);

    if(!cp)
	return -1;

    return(cp - str->Ptr);
}




/* @func ajStrFindK ***********************************************************
**
** Finds the first occurrence in a string of a single character.
**
** @param [r] str [const AjPStr] String
** @param [r] chr [char] Character to find
** @return [ajlong] Position of the start of text in string if found.
**                Or -1 for text not found.
**
** @release 1.0.0
** @@
******************************************************************************/

ajlong ajStrFindK(const AjPStr str, char chr)
{
    const char* cp;

    cp = strchr(MAJSTRGETPTR(str), chr);

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
** @return [ajlong] Position of the start of text in string if found.
**                  Or -1 for text not found.
**
** @release 4.0.0
** @@
******************************************************************************/

ajlong ajStrFindS(const AjPStr str, const AjPStr str2)
{
    const char* cp;

    cp = strstr(MAJSTRGETPTR(str), MAJSTRGETPTR(str2));

    if(!cp)
	return -1;

    return (cp - str->Ptr);
}




/* @func ajStrFindAnyC ********************************************************
**
** Finds the first occurrence in a string of any character in a second 
** (text) string.
**
** @param [r] str [const AjPStr] String
** @param [r] txt2 [const char*] text to find
** @return [ajlong] Position of the start of text in string if found.
**                  Or -1 for text not found.
**
** @release 3.0.0
** @@
******************************************************************************/

ajlong ajStrFindAnyC(const AjPStr str, const char* txt2)
{
    size_t i;

    i = strcspn(MAJSTRGETPTR(str), txt2);

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
** @return [ajlong] Position of the start of text in string if found.
**                  Or -1 for text not found.
**
** @release 4.0.0
** @@
******************************************************************************/

ajlong ajStrFindAnyK(const AjPStr str, char chr)
{
    const char* cp;

    cp = strchr(MAJSTRGETPTR(str), (ajint) chr);

    if(!cp)
	return -1;

    return(cp - str->Ptr);
}




/* @func ajStrFindAnyS ********************************************************
**
** Finds the first occurrence in a string of any character in a second 
** string.
**
** @param [r] str [const AjPStr] String
** @param [r] str2 [const AjPStr] text to find
** @return [ajlong] Position of the start of text in string if found.
**                  Or -1 for text not found.
**
** @release 4.0.0
** @@
******************************************************************************/

ajlong ajStrFindAnyS(const AjPStr str, const AjPStr str2)
{
    size_t i;

    i = strcspn(MAJSTRGETPTR(str), MAJSTRGETPTR(str2));

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
** @return [ajlong] Position of the start of text in string if found.
** @error -1 Text not found.
**
** @release 1.7.0
** @@
******************************************************************************/

ajlong ajStrFindCaseC(const AjPStr str, const char *txt2)
{
    AjPStr t1;
    AjPStr t2;
    ajlong v;

    t1 = ajStrNewC(MAJSTRGETPTR(str));
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
** @return [ajlong] Position of the start of text in string if found.
**                  -1 if not found.
**
** @release 4.0.0
** @@
******************************************************************************/

ajlong ajStrFindCaseS(const AjPStr str, const AjPStr str2)
{
    return ajStrFindCaseC(str,MAJSTRGETPTR(str2));
}




/* @func ajStrFindNextC *******************************************************
**
** Finds the next occurrence in a string of a second 
** (text) string.
**
** @param [r] str [const AjPStr] String
** @param [r] pos1 [ajlong] Start position in string
** @param [r] txt2 [const char*] text to find
** @return [ajlong] Position of the start of text in string if found.
**                  Or -1 for text not found.
**
** @release 6.0.0
** @@
******************************************************************************/

ajlong ajStrFindNextC(const AjPStr str, ajlong pos1, const char* txt2)
{
    size_t i;
    size_t jpos;
    char *cp;

    jpos = ajCvtSposToPosStart(str->Len, 0, pos1);
    cp = strstr(&str->Ptr[jpos], txt2);
    if(!cp)
        return -1;

    i = cp - str->Ptr;

    return i;
}




/* @func ajStrFindNextK *******************************************************
**
** Finds the next occurrence in a string of a specified character.
**
** @param [r] str [const AjPStr] String
** @param [r] pos1 [ajlong] Start position in string
** @param [r] chr [char] character to find
** @return [ajlong] Position of the start of text in string if found.
**                  Or -1 for text not found.
**
** @release 6.0.0
** @@
******************************************************************************/

ajlong ajStrFindNextK(const AjPStr str, ajlong pos1, char chr)
{
    const char* cp;
    size_t jpos;

    jpos = ajCvtSposToPosStart(str->Len, 0, pos1);

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
** @param [r] pos1 [ajlong] Start position in string
** @param [r] str2 [const AjPStr] text to find
** @return [ajlong] Position of the start of text in string if found.
**                Or -1 for text not found.
**
** @release 6.0.0
** @@
******************************************************************************/

ajlong ajStrFindNextS(const AjPStr str, ajlong pos1, const AjPStr str2)
{
    size_t i;
    size_t jpos;
    char *cp;

    jpos = ajCvtSposToPosStart(str->Len, 0, pos1);

    cp = strstr(&str->Ptr[jpos], str2->Ptr);

    if(!cp)
        return -1;

    i = cp - str->Ptr;

    return i;
}




/* @func ajStrFindRestC *******************************************************
**
** Finds the first occurrence in a string of any character not in a second 
** (text) string.
**
** @param [r] str [const AjPStr] String
** @param [r] txt2 [const char*] text to find
** @return [ajlong] Position of the start of text in string if found.
**                  Or -1 for text not found.
**
** @release 4.1.0
** @@
******************************************************************************/

ajlong ajStrFindRestC(const AjPStr str, const char* txt2)
{
    size_t i;

    i = strspn(MAJSTRGETPTR(str), txt2);

    if(i == MAJSTRGETLEN(str))
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
** @return [ajlong] Position of the start of text in string if found.
**                  Or -1 for text not found.
**
** @release 4.1.0
** @@
******************************************************************************/

ajlong ajStrFindRestS(const AjPStr str, const AjPStr str2)
{
    return ajStrFindRestC(str, MAJSTRGETPTR(str2));
}




/* @func ajStrFindRestCaseC ***************************************************
**
** Finds the first occurrence in a string of any character not in a second 
** (text) string (case-insensitive).
**
** @param [r] str [const AjPStr] String
** @param [r] txt2 [const char*] text to find
** @return [ajlong] Position of the start of text in string if found.
**                  Or -1 for text not found.
**
** @release 4.1.0
** @@
******************************************************************************/

ajlong ajStrFindRestCaseC(const AjPStr str, const char* txt2)
{
    size_t i;
    AjPStr tmpstr;
    AjPStr tmptxt;

    tmpstr = ajStrNewS(str);
    tmptxt = ajStrNewC(txt2);

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
** @return [ajlong] Position of the start of text in string if found.
**                  Or -1 for text not found.
**
** @release 4.1.0
** @@
******************************************************************************/

ajlong ajStrFindRestCaseS(const AjPStr str, const AjPStr str2)
{
    return ajStrFindRestCaseC(str, MAJSTRGETPTR(str2));
}




/* @func ajStrFindlastC *******************************************************
**
** Finds the last occurrence in a string of a second (text) string.
**
** @param [r] str [const AjPStr] String to search
** @param [r] txt2 [const char*] text to look for
** @return [ajlong] Position of the text string if found.
** @error -1 Text not found.
**
** @release 4.0.0
** @@
******************************************************************************/

ajlong ajStrFindlastC(const AjPStr str, const char* txt2)
{
    ajlong i = 0;
    size_t j = 0;
    size_t len = 0;
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




/* @func ajStrFindlastK *******************************************************
**
** Finds the last occurrence in a string of a character.
**
** @param [r] str [const AjPStr] String to search
** @param [r] chr [char] Character to look for
** @return [ajlong] Position of the character if found.
** @error -1 Text not found.
**
** @release 6.1.0
** @@
******************************************************************************/

ajlong ajStrFindlastK(const AjPStr str, char chr)
{
    const char* cp;

    cp = strrchr(MAJSTRGETPTR(str), (ajint) chr);

    if(!cp)
	return -1;

    return(cp - str->Ptr);
}




/* @func ajStrFindlastS *******************************************************
**
** Finds the last occurrence in a string of a second (text) string.
**
** @param [r] str [const AjPStr] String to search
** @param [r] str2 [const AjPStr] text to look for
** @return [ajlong] Position of the text string if found.
** @error -1 Text not found.
**
** @release 4.0.0
** @@
******************************************************************************/

ajlong ajStrFindlastS(const AjPStr str, const AjPStr str2)
{
    ajlong i = 0;
    size_t j = 0;
    const char* ptr1 = 0;
    const char* ptr2 = 0;
    AjBool found = ajTrue;

    for(i=str->Len-str2->Len; i >= 0; --i)
    {
	ptr1 = &str->Ptr[i];
	ptr2 = &str2->Ptr[i];
	found = ajTrue;

	for(j=0; j < str2->Len; ++j)
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
** @nam4rule ExtractToken   Remove first token from a string, skipping spaces
**                          and punctuation (commas, colons)
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
** @argrule Token Pword [AjPStr*] First word of string
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
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrExtractFirst(const AjPStr str, AjPStr* Prest, AjPStr* Pword)
{
    size_t i=0;
    size_t j=0;
    const char* cp;

    if(!str)
	return ajFalse;

    if(!str->Len)
	return ajFalse;

    cp = str->Ptr;

    if(isspace((int)*cp++))
 	return ajFalse;

    while(*cp && !isspace((int)*cp))		/* look for end of first word */
    {
	cp++;
	i++;
    }

    j = i+1;

    while(*cp && isspace((int)*cp))
    {
	cp++;
	j++;
    }

    ajStrAssignSubS(Pword, str, 0, i);

    if(*cp)
	ajStrAssignSubS(Prest, str, j, str->Len);
    else
	ajStrAssignClear(Prest);

    /*ajDebug("ajStrExtractFirst i:%d j:%d len:%d word '%S'\n",
      i, j, str->Len, *Pword);*/

    return ajTrue;
}




/* @func ajStrExtractToken ****************************************************
**
** Returns a word from the start of a string, and the remainder of the string.
** Leading spaces and punctuation (commas, semicolons) are ignored
**
** @param [r] str [const AjPStr] String to be parsed
** @param [w] Prest [AjPStr*] Remainder of string
** @param [w] Pword [AjPStr*] First token of string
** @return [AjBool] True if parsing succeeded
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajStrExtractToken(const AjPStr str, AjPStr* Prest, AjPStr* Pword)
{
    size_t i=0;
    size_t istart=0;
    size_t j=0;
    const char* cp;

    if(!str)
	return ajFalse;

    if(!str->Len)
	return ajFalse;

    cp = str->Ptr;

    while(isspace((int)*cp))
    {
	cp++;
	istart++;
    }

    if(!*cp)
	return ajFalse;

    i = istart-1;

    while(*cp && !isspace((int)*cp) &&
	  !(strchr(",;", *cp)))                 /* look for token end */
    {
	cp++;
	i++;
    }

    j = i+1;

    while(*cp && isspace((int)*cp) && !(strchr(",;", *cp)))
    {
	cp++;
	j++;
    }

    ajStrAssignSubS(Pword, str, istart, i);

    if(*cp)
	ajStrAssignSubS(Prest, str, j, str->Len);
    else
	ajStrAssignClear(Prest);

    /*ajDebug("ajStrExtractWord i:%d j:%d len:%d word '%S'\n",
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
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrExtractWord(const AjPStr str, AjPStr* Prest, AjPStr* Pword)
{
    size_t i=0;
    size_t istart=0;
    size_t j=0;
    const char* cp;

    if(!str)
	return ajFalse;

    if(!str->Len)
	return ajFalse;

    cp = str->Ptr;

    while(isspace((int)*cp))
    {
	cp++;
	istart++;
    }

    if(!*cp)
	return ajFalse;

    i = istart-1;

    while(*cp && !isspace((int)*cp))	/* look for end of first word */
    {
	cp++;
	i++;
    }

    j = i+1;

    while(*cp && isspace((int)*cp))
    {
	cp++;
	j++;
    }

    ajStrAssignSubS(Pword, str, istart, i);

    if(*cp)
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
**        NULL for follow-up calls using the same string, as for the
**        C RTL function strtok which is eventually called.
** @param [r] txtdelim [const char*] Delimiter(s) to be used between tokens.
** @return [const AjPStr] Token returned, when all tokens are parsed
**                  a NULL is returned.
**
** @release 4.0.0
** @@
******************************************************************************/

const AjPStr ajStrParseC(const AjPStr str, const char* txtdelim)
{
    static AjOStr strParseStr = { 1, 0, NULL, 0, 0};
    static AjPStr strp = &strParseStr;

    if(!strp->Ptr)
	if(!str)
	{
	    ajWarn("Error in ajStrParseC: NULL argument and not initialised");

	    return NULL;
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
    else
    {
	strp->Len=0;
	strp->Res=1;
	ajCharDel(&strParseCp);
    }

    return NULL;
}




/* @func ajStrParseCount ******************************************************
**
** Returns the number of tokens in a string, delimited by whitespace
**
** @param [r] str [const AjPStr] String to examine.
** @return [ajuint] The number of tokens
**
** @release 4.0.0
** @@
******************************************************************************/

ajuint ajStrParseCount(const AjPStr str)
{
    AjPStrTok t = NULL;

    ajuint count;

    if(!str)
        return 0;

    if(!ajStrGetLen(str))
        return 0;

    count = 0;
    ajStrTokenAssignC(&t, str, " \t\n\r");

    while(ajStrTokenStep(t))
	++count;

    ajStrTokenDel(&t);

    return count;
}




/* @func ajStrParseCountC *****************************************************
**
** Returns the number of tokens in a string using a specified set of 
** delimiters held in a text string.
**
** @param [r] str [const AjPStr] String to examine.
** @param [r] txtdelim [const char *] String of delimiter characters.
** @return [ajuint] The number of tokens
**
** @release 4.0.0
** @@
******************************************************************************/

ajuint ajStrParseCountC(const AjPStr str, const char *txtdelim)
{
    AjPStrTok t = NULL;

    ajuint count;

    if(!str)
        return 0;

    if(!ajStrGetLen(str))
        return 0;

    count = 0;
    ajStrTokenAssignC(&t, str, txtdelim);

    while(ajStrTokenStep(t))
	++count;

    ajStrTokenDel(&t);

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
**
** @release 4.0.0
** @@
******************************************************************************/

ajuint ajStrParseCountS(const AjPStr str, const AjPStr strdelim)
{
    AjPStrTok t = NULL;

    ajuint count;

    if(!str)
        return 0;

    if(!ajStrGetLen(str))
        return 0;

    count = 0;
    ajStrTokenAssignS(&t, str, strdelim);

    while(ajStrTokenStep(t))
	++count;

    ajStrTokenDel(&t);

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
**
** @release 4.0.0
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




/* @func ajStrParseSplit ******************************************************
**
** Splits a newline-separated multi-line string into an array of strings.
**
** @param [r] str [const AjPStr] String
** @param [w] PPstr [AjPStr**] pointer to array of AjPStrs
**
** @return [ajuint] Number of array elements created
**
** @release 4.0.0
** @@
******************************************************************************/


ajuint ajStrParseSplit(const AjPStr str, AjPStr **PPstr)
{
    ajuint c;
    size_t len;
    size_t i;
    size_t n;
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




/* @func ajStrParseWhite ******************************************************
**
** Tokenise a string using whitespace and return tokens from the string. 
**
** @param [r] str [const AjPStr] String to be parsed (first call) or
**        NULL for follow-up calls using the same string, as for the
**        C RTL function strtok which is eventually called.
** @return [const AjPStr] Token
** @error NULL if no further token is found.
**
** @release 4.0.0
** @@
******************************************************************************/

const AjPStr ajStrParseWhite(const AjPStr str)
{
    return ajStrParseC(str, " \t\n\r");
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
**
** @release 6.0.0
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
**
** @release 1.0.0
** @@
******************************************************************************/

void ajStrStat(const char* title)
{
#ifdef AJ_SAVESTATS
    static ajlong statAlloc     = 0;
    static ajlong statExtra     = 0;
    static ajlong statCount     = 0;
    static ajlong statFree      = 0;
    static ajlong statFreeCount = 0;
    static ajlong statTotal     = 0;

    ajDebug("String usage statistics since last call %s:\n", title);
    ajDebug("String usage (bytes): %Ld allocated, %Ld resize, %Ld freed\n",
	    strAlloc - statAlloc, strExtra - statExtra,
            strFree - statFree);
    ajDebug("String usage (number): %Ld allocated, %Ld freed, %Ld in use\n",
	    strTotal - statTotal, strFreeCount - statFreeCount,
	    strCount - statCount);

    statAlloc     = strAlloc;
    statExtra     = strExtra;
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
**
** @release 1.0.0
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




/* @func ajStrTraceFull *******************************************************
**
** Checks a string object for consistency and reports its contents character 
** by character.
**
** @param [r] str [const AjPStr] String
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajStrTraceFull(const AjPStr str)
{
    size_t i;

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
**
** @release 4.0.0
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
**
** @release 1.0.0
** @@
******************************************************************************/

void ajStrExit(void)
{
#ifdef AJ_SAVESTATS
    ajDebug("String usage (bytes): %Ld allocated, %Ld resized, "
            "%Ld freed, %Ld in use\n",
	    strAlloc, strExtra, strFree,
	    (strAlloc + strExtra - strFree));
    ajDebug("String usage (number): %Ld allocated, %Ld freed %Ld in use\n",
	    strTotal, strFreeCount, strCount);
#endif

    ajCharDel(&strParseCp);

    return;
}




/* @datasection [const AjPStr] String constant *********************************
**
** Functions for manipulating AJAX (AjPStr) string constants
**
** @nam2rule Str    Function is for manipulating strings
**
*/




/* @section string constant constructors ***************************************
**
** Functions for constructing string constant objects 
**
** @fdata      [const AjPStr]
** @fnote     Same namrule as "String constructor functions:
**            C-type (char*) strings"
** @nam3rule  Const    Construct a constant string.
** @nam4rule  ConstEmpty Construct with an empty string.
**
** @argrule   S   str [const AjPStr] Text string
**
** @valrule   *     [const AjPStr] New string
** @fcategory new
*/




/* @func ajStrConstS **********************************************************
**
** Returns an unmodifiable empty string.
**
** @param [r] str [const AjPStr] String
** @return [const AjPStr] Pointer to an empty string
**
** @release 6.2.0
** @@
******************************************************************************/

const AjPStr ajStrConstS(const AjPStr str)
{
    if(!str)
        return strPNULL;

    return str;
}




/* @func ajStrConstEmpty ******************************************************
**
** Returns an unmodifiable empty string.
** 
** @return [const AjPStr] Pointer to an empty string
**
** @release 6.2.0
** @@
******************************************************************************/

const AjPStr ajStrConstEmpty(void)
{
    return strPNULL;
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
**
** @release 4.0.0
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




/* @func ajStrIterNewBack *****************************************************
**
** String iterator constructor which allocates memory for a string iterator, 
** used to iterate over the characters in a string, from end to start.
**
** @param [r] str [const AjPStr] Original string
** @return [AjIStr] String Iterator
**
** @release 4.0.0
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




/* @func ajStrIterDel *********************************************************
**
** String iterator destructor which frees memory for a string iterator.
**
** @param [d] iter [AjIStr*] String iterator
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajStrIterDel(AjIStr* iter)
{
    AJFREE(*iter);

    return;
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




/* @func ajStrIterDone ********************************************************
**
** Tests whether a string iterator has completed yet.
**
** @param [r] iter [const AjIStr] String iterator.
** @return [AjBool] true if complete
**
** @release 4.0.0
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
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrIterDoneBack(const AjIStr iter)
{
    return (iter->Ptr < iter->Start);
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
**
** @release 4.0.0
** @@
******************************************************************************/

void ajStrIterBegin(AjIStr iter)
{
    iter->Ptr = iter->Start;
    return;
}




/* @func ajStrIterEnd *********************************************************
**
** Sets a string iterator to its stop condition.
**
** @param [u] iter [AjIStr] String iterator.
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajStrIterEnd(AjIStr iter)
{
    iter->Ptr = iter->End;

    return;
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




/* @func ajStrIterGetC ********************************************************
**
** Returns the remainder of the string at the current string iterator 
** position.
**
** @param [r] iter [const AjIStr] String iterator.
** @return [const char*] Current text string within iterator
**
** @release 4.0.0
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
**
** @release 4.0.0
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
**
** @release 4.0.0
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
**
** @release 1.0.0
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
**
** @release 4.0.0
** @@
******************************************************************************/

AjIStr ajStrIterNextBack(AjIStr iter)
{
    iter->Ptr--;

    if(iter->Ptr < iter->Start)
	return NULL;

    return iter;
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
** @nam4rule New Constructor with string object data
** @nam4rule Newchar Constructor with C string data
**
** @argrule New str [const AjPStr] string
** @argrule Newchar txt [const char*] string
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
**
** @release 4.0.0
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




/* @func ajStrTokenNewS *******************************************************
**
** String token parser constructor which allocates memory for a string token 
** parser object from a string and an optional set of default delimiters 
** defined in a text string.
**
** @param [r] str [const AjPStr] Source string
** @param [r] strdelim [const AjPStr] Default delimiter(s)
** @return [AjPStrTok] A new string token parser.
**
** @release 4.0.0
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




/* @func ajStrTokenNewcharC ***************************************************
**
** String token parser constructor which allocates memory for a string token 
** parser object from a string and a set of default delimiters defined in a 
** text string.
**
** @param [r] txt [const char*] Source string
** @param [r] txtdelim [const char*] Default delimiter(s)
** @return [AjPStrTok] A new string token parser.
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStrTok ajStrTokenNewcharC(const char* txt, const char* txtdelim)
{
    AjPStrTok ret;

    AJNEW0(ret);

    ajStrAssignC(&ret->String, txt);

    ret->Delim = ajStrNewC(txtdelim);
    ret->Pos   = 0;		  /* GFF parsing needed this change */

    return ret;
}




/* @func ajStrTokenNewcharS ***************************************************
**
** String token parser constructor which allocates memory for a string token 
** parser object from a string and an optional set of default delimiters 
** defined in a text string.
**
** @param [r] txt [const char*] Source string
** @param [r] strdelim [const AjPStr] Default delimiter(s)
** @return [AjPStrTok] A new string token parser.
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStrTok ajStrTokenNewcharS(const char* txt, const AjPStr strdelim)
{
    AjPStrTok ret;

    AJNEW0(ret);

    ajStrAssignC(&ret->String, txt);

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




/* @func ajStrTokenDel ********************************************************
**
** String token parser destructor which frees memory for a string token 
** parser.
**
** @param [d] Ptoken [AjPStrTok*] Token parser
** @return [void]
**
** @release 4.0.0
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




/* @section assignment ********************************************************
**
** @fdata [AjPStrTok]
**
** @nam4rule Assign Assignment of string and/or delimiter(s)
** @nam4rule Assignchar Assignment of C character string and/or delimiter(s)
**
** @argrule Assign Ptoken [AjPStrTok*] Token parser
** @argrule Assignchar Ptoken [AjPStrTok*] Token parser
** @argrule Assign str [const AjPStr] String to be parsed
** @argrule Assignchar txt [const char*] String to be parsed
** @argrule C txtdelim [const char*] delimiter
** @argrule S strdelim [const AjPStr] delimiter
**
** @valrule * [AjBool] True on success
**
** @fcategory assign
*/




/* @func ajStrTokenAssign *****************************************************
**
** Generates a string token parser object from a string without a specific
** set of delimiters.
**
** @param [w] Ptoken [AjPStrTok*] String token object
** @param [r] str [const AjPStr] Source string
** @return [AjBool] ajTrue on success.
**
** @release 4.0.0
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
**
** @release 4.0.0
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
**
** @release 4.0.0
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




/* @func ajStrTokenAssignchar *************************************************
**
** Generates a string token parser object from a string without a specific
** set of delimiters.
**
** @param [w] Ptoken [AjPStrTok*] String token object
** @param [r] txt [const char*] Source string
** @return [AjBool] ajTrue on success.
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrTokenAssignchar(AjPStrTok* Ptoken, const char *txt)
{
    AjPStrTok tok;
    
    if(*Ptoken)
	tok = *Ptoken;
    else
    {
	AJNEW0(tok);
	*Ptoken = tok;
    }
    
    ajStrAssignC(&tok->String, txt);
    ajStrAssignClear(&tok->Delim);
    tok->Pos = 0;

    return ajTrue;
}




/* @func ajStrTokenAssigncharC ************************************************
**
** Generates a string token parser object from a string and an optional
** set of default delimiters defined in a text string.
**
** The string token can be either an existing token to be overwritten
** or a NULL.
**
** @param [w] Ptoken [AjPStrTok*] String token object
** @param [r] txt [const char*] Source string
** @param [r] txtdelim [const char*] Default delimiter(s)
** @return [AjBool] ajTrue on success.
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrTokenAssigncharC(AjPStrTok* Ptoken, const char *txt,
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
    
    ajStrAssignC(&tok->String, txt);
    ajStrAssignC(&tok->Delim, txtdelim);
    tok->Pos = strspn(tok->String->Ptr, tok->Delim->Ptr);

    return ajTrue;
}




/* @func ajStrTokenAssigncharS ************************************************
**
** Generates a string token parser object from a string and an optional
** set of default delimiters defined in a string.
**
** @param [w] Ptoken [AjPStrTok*] String token object
** @param [r] txt [const char*] Source string
** @param [r] strdelim [const AjPStr] Default delimiter(s)
** @return [AjBool] ajTrue on success.
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrTokenAssigncharS(AjPStrTok* Ptoken, const char *txt,
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
    
    ajStrAssignC(&tok->String, txt);
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
** @argrule * token [AjPStrTok] String token parser
**
** @valrule * [void]
**
** @fcategory modify
*/




/* @func ajStrTokenReset ******************************************************
**
** Clears the strings from a string token parser object.
**
** @param [w] token [AjPStrTok] String token object
** @return [void]
**
** @release 6.6.0
** @@
******************************************************************************/

void ajStrTokenReset(AjPStrTok token)
{
    if(!token)
	return;

    ajStrDelStatic(&token->String);
    ajStrDelStatic(&token->Delim);
    token->Pos = 0;

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
**
** @release 1.0.0
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
** @nam5rule NextParse Use delimiter as a set of characters
** @nam5rule NextFind Use delimiter as a string
** @nam6rule Delimiters Also save the delimiter found in the source string
** @nam6rule NextParseNoskip Use delimiter as a string, stop at first delimiter
** @nam4rule Rest Return remainder of string
** @nam5rule RestParse Return remainder of string
** @nam4rule Step Use delimiter as a set of characters, no string returned
**
** @argrule * token [AjPStrTok] String token parser
** @argrule C txtdelim [const char*] Delimiter
** @argrule S strdelim [const AjPStr] Delimiter
** @argrule Rest Pstr [AjPStr*] String result
** @argrule Next Pstr [AjPStr*] String result
** @argrule Delimiters Pdelim [AjPStr*] Delimiter(s) following the token
**
** @valrule * [AjBool] True on success
**
** @fcategory modify
*/




/* @func ajStrTokenNextFind ***************************************************
**
** Parses tokens from a string using a string token parser. Treats the 
** whole delimiter as a single string between tokens. 
**
** @param [u] token [AjPStrTok] Token parser. Updated with the delimiter
**        string (if any) in delim.
** @param [w] Pstr [AjPStr*] Token found
**
** @return [AjBool] ajTrue if another token was found.
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrTokenNextFind(AjPStrTok token, AjPStr* Pstr)
{
    size_t ilen;
    char* cp;
    char* cq;

    if(!token)
    {					/* token already cleared */
	ajStrAssignClear(Pstr);

	return ajFalse;
    }

    if(token->Pos >= token->String->Len)
    {					/* all done */
	ajStrAssignClear(Pstr);
	ajStrTokenReset(token);

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




/* @func ajStrTokenNextFindC **************************************************
**
** Parses tokens from a string using a string token parser. Treats the 
** whole delimiter as a single string between tokens. 
**
** @param [u] token [AjPStrTok] Token parser. Updated with the delimiter
**        string in delim.
** @param [r] txtdelim [const char*] Delimiter string.
** @param [w] Pstr [AjPStr*] Token found
**
** @return [AjBool] ajTrue if another token was found.
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrTokenNextFindC(AjPStrTok token, const char* txtdelim,
			   AjPStr* Pstr)
{
    ajStrAssignC(&token->Delim, txtdelim);

    return ajStrTokenNextFind(token, Pstr);
}




/* @func ajStrTokenNextParse **************************************************
**
** Parses tokens from a string using a string token parser. Uses any
** character defined in the string token parsing object delimiter
** character set as a delimiter.
**
** The test uses the C function 'strcspn'.
**
** @param [u] token [AjPStrTok] String token parsing object.
** @param [w] Pstr [AjPStr*] Next token returned, may be empty if the
**                           delimiter has changed.
** @return [AjBool] True if successfully parsed.
**                  False (and string set to empty) if there is nothing
**                  more to parse.
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrTokenNextParse(AjPStrTok token, AjPStr* Pstr)
{
    size_t ilen;
    char* cp;

    if(!token)
    {					/* token already cleared */
	ajStrAssignClear(Pstr);

	return ajFalse;
    }

    if(token->Pos >= token->String->Len)
    {					/* all done */
	ajStrAssignClear(Pstr);
	ajStrTokenReset(token);

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




/* @func ajStrTokenNextParseC *************************************************
**
** Parses tokens from a string using a string token parser. Uses any
** character defined in the delimiter character set as a
** delimiter.
**
** Returns the next token parsed from a string token parsing object
**
** Note: This can return "true" but an empty string in cases where the
** delimiter has changed since the previous call.
**
** The test uses the C function 'strcspn'.
**
** @param [u] token [AjPStrTok] String token parsing object.
** @param [r] txtdelim [const char*] Delimiter character set.
** @param [w] Pstr [AjPStr*] Next token returned, may be empty if the
**                           delimiter has changed.
** @return [AjBool] True if successfully parsed.
**                  False (and string set to empty) if there is nothing
**                  more to parse.
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrTokenNextParseC(AjPStrTok token, const char* txtdelim,
			    AjPStr* Pstr)
{
    if(!token)
    {
	ajStrAssignClear(Pstr);

	return ajFalse;
    }

    ajStrAssignC(&(token)->Delim, txtdelim);

    return ajStrTokenNextParse(token, Pstr);
}




/* @func ajStrTokenNextParseS *************************************************
**
** Parses tokens from a string using a string token parser. Uses any
** character defined in the delimiter character set as a
** delimiter.
**
** Note: This can return "true" but an empty string in cases where the
** delimiter has changed since the previous call.
**
** The test uses the C function 'strcspn'.
**
** @param [u] token [AjPStrTok] String token parsing object.
** @param [r] strdelim [const AjPStr] Delimiter character set.
** @param [w] Pstr [AjPStr*] Next token returned, may be empty if the
**                           delimiter has changed.
** @return [AjBool] True if successfully parsed.
**                  False (and string set to empty) if there is nothing
**                  more to parse.
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrTokenNextParseS(AjPStrTok token, const AjPStr strdelim,
			    AjPStr* Pstr)
{
    if(!token)
    {
	ajStrAssignClear(Pstr);

	return ajFalse;
    }

    ajStrAssignS(&(token)->Delim, strdelim);

    return ajStrTokenNextParse(token, Pstr);
}




/* @func ajStrTokenNextParseDelimiters ****************************************
**
** Parses tokens from a string using a string token parser. Uses any
** character defined in the string token parsing object delimiter
** character set as a delimiter.
**
** The test uses the C function 'strcspn'.
**
** @param [u] token [AjPStrTok] String token parsing object.
** @param [w] Pstr [AjPStr*] Next token returned, may be empty if the
**                           delimiter has changed.
** @param [w] Pdelim [AjPStr*] Delimiter(s) following the token.
** @return [AjBool] True if successfully parsed.
**                  False (and string set to empty) if there is nothing
**                  more to parse.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajStrTokenNextParseDelimiters(AjPStrTok token, AjPStr* Pstr,
                                     AjPStr* Pdelim)
{
    size_t ilen;
    char* cp;

    if(!token)
    {					/* token already cleared */
	ajStrAssignClear(Pstr);

	return ajFalse;
    }

    if(token->Pos >= token->String->Len)
    {					/* all done */
	ajStrAssignClear(Pstr);
	ajStrTokenReset(token);

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
    ilen= strspn(&token->String->Ptr[token->Pos], token->Delim->Ptr);
    if(ilen)
    {
	ajStrAssignSubS(Pdelim, token->String,
                        token->Pos, token->Pos + ilen - 1);
    }
    else
    {
	ajStrAssignClear(Pdelim);
    }

    token->Pos +=ilen;

    return ajTrue;
}




/* @func ajStrTokenNextParseNoskip ********************************************
**
** Parses tokens from a string using a string token parser. Uses any
** character defined in the string token parsing object's delimiter
** character set as a delimiter.
**
** The test uses the C function 'strcspn'.
**
** @param [u] token [AjPStrTok] String token parsing object.
** @param [w] Pstr [AjPStr*] Next token returned, may be empty if the
**                           delimiter has changed or if next character
**                           was also a delimiter
** @return [AjBool] True if successfully parsed.
**                  False (and string set to empty) if there is nothing
**                  more to parse.
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ajStrTokenNextParseNoskip(AjPStrTok token, AjPStr* Pstr)
{
    size_t ilen;
    char* cp;

    if(!token)
    {					/* token already cleared */
	ajStrAssignClear(Pstr);

	return ajFalse;
    }

    if(token->Pos >= token->String->Len)
    {					/* all done */
	ajStrAssignClear(Pstr);
	ajStrTokenReset(token);

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

    token->Pos += ilen + 1;

    return ajTrue;
}




/* @func ajStrTokenRestParse **************************************************
**
** Returns the remainder of a string that has been partially parsed using a 
** string token parser.
**
** @param [u] token [AjPStrTok] String token parsing object.
** @param [w] Pstr [AjPStr*] Remainder of string
** @return [AjBool] True if successfully parsed.
**                  False (and string set to empty) if there is nothing
**                  more to parse.
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajStrTokenRestParse(AjPStrTok token, AjPStr* Pstr)
{
    if(!token)
    {					/* token already cleared */
	ajStrAssignClear(Pstr);

	return ajFalse;
    }

    if(token->Pos >= token->String->Len)
    {					/* all done */
	ajStrAssignClear(Pstr);
	ajStrTokenReset(token);

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




/* @func ajStrTokenStep *******************************************************
**
** Steps to next token from a string using a string token parser. Uses any
** character defined in the string token parsing object delimiter
** character set as a delimiter.
**
** The test uses the C function 'strcspn'.
**
** @param [u] token [AjPStrTok] String token parsing object.
** @return [AjBool] True if successfully parsed.
**                  False (and token parser deleted) if there is nothing
**                  more to parse.
**
** @release 6.6.0
** @@
******************************************************************************/

AjBool ajStrTokenStep(AjPStrTok token)
{
    size_t ilen;
    char* cp;

    if(!token)
	return ajFalse;

    if(token->Pos >= token->String->Len)
    {					/* all done */
	ajStrTokenReset(token);

	return ajFalse;
    }

    cp = &token->String->Ptr[token->Pos];
    ilen = strcspn(cp, token->Delim->Ptr);

    token->Pos += ilen;
    token->Pos += strspn(&token->String->Ptr[token->Pos], token->Delim->Ptr);

    return ajTrue;
}




/* @func ajStrTokenStepC ******************************************************
**
**Steps to next token from a string using a string token parser. Uses any
** character defined in the delimiter character set as a
** delimiter.
**
** Note: This can return "true" without moving in cases where the
** delimiter has changed since the previous call.
**
** The test uses the C function 'strcspn'.
**
** @param [u] token [AjPStrTok] String token parsing object.
** @param [r] txtdelim [const char*] Delimiter character set.
** @return [AjBool] True if successfully parsed.
**                  False (and string set to empty) if there is nothing
**                  more to parse.
**
** @release 6.6.0
** @@
******************************************************************************/

AjBool ajStrTokenStepC(AjPStrTok token, const char* txtdelim)
{
    if(!token)
	return ajFalse;

    ajStrAssignC(&token->Delim, txtdelim);

    return ajStrTokenStep(token);
}




/* @func ajStrTokenStepS ******************************************************
**
** Parses tokens from a string using a string token parser. Uses any
** character defined in the delimiter character set as a
** delimiter.
**
** Note: This can return "true" but an empty string in cases where the
** delimiter has changed since the previous call.
**
** The test uses the C function 'strcspn'.
**
** @param [u] token [AjPStrTok] String token parsing object.
** @param [r] strdelim [const AjPStr] Delimiter character set.
** @return [AjBool] True if successfully parsed.
**                  False (and string set to empty) if there is nothing
**                  more to parse.
**
** @release 6.6.0
** @@
******************************************************************************/

AjBool ajStrTokenStepS(AjPStrTok token, const AjPStr strdelim)
{
    if(!token)
	return ajFalse;

    ajStrAssignS(&token->Delim, strdelim);

    return ajStrTokenStep(token);
}




#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif




#ifdef AJ_COMPILE_DEPRECATED
/* @obsolete ajCharNew
** @rename ajCharNewS
*/

__deprecated char  *ajCharNew(const AjPStr str)
{
    return ajCharNewS(str);
}




/* @obsolete ajCharNewL
** @rename ajCharNewRes
*/

__deprecated char  *ajCharNewL(size_t size)
{
    return ajCharNewRes(size);
}




/* @obsolete ajCharNewLS
** @replace ajStrNewResS (1,2/2,1)
*/

__deprecated char  *ajCharNewLS(size_t size, const AjPStr thys)
{
    return ajCharNewResS(thys, size);
}




/* @obsolete ajCharFree
** @rename ajCharDel
*/

__deprecated void  ajCharFree(char** txt)
{
    ajCharDel(txt);
}




/* @obsolete ajCharToLower
** @rename ajCharFmtLower
*/

__deprecated void  ajCharToLower(char* txt)
{
    ajCharFmtLower(txt);
}




/* @obsolete ajCharToUpper
** @rename ajCharFmtUpper
*/

__deprecated void  ajCharToUpper(char* txt)
{
    ajCharFmtUpper(txt);
}




/* @obsolete ajStrMatchCC
** @rename ajCharMatchC
*/

__deprecated AjBool  ajStrMatchCC(const char* thys, const char* text)
{
    return ajCharMatchC(thys, text);
}




/* @obsolete ajStrMatchCaseCC
** @rename ajCharMatchCaseC
*/

__deprecated AjBool  ajStrMatchCaseCC(const char* thys, const char* text)
{
    return ajCharMatchCaseC(thys, text);
}




/* @obsolete ajStrMatchWildCC
** @rename ajCharMatchWildC
*/

__deprecated AjBool  ajStrMatchWildCC(const char* str, const char* text)
{
    return ajCharMatchWildC(str, text);
}




/* @obsolete ajStrMatchWildCO
** @rename ajCharMatchWildS
*/

__deprecated AjBool  ajStrMatchWildCO(const char* str, const AjPStr wild)
{
    return ajCharMatchWildS(str, wild);
}




/* @obsolete ajStrMatchWordCC
** @rename ajCharMatchWildWordS
*/

__deprecated AjBool  ajStrMatchWordCC (const char* str, const char* txt)
{
    return ajCharMatchWildWordC(str, txt);
}




/* @obsolete ajStrPrefixCC
** @rename ajCharPrefixC
*/

__deprecated AjBool  ajStrPrefixCC(const char* str, const char* pref)
{
    return ajCharPrefixC(str, pref);
}




/* @obsolete ajStrPrefixCO
** @rename ajCharPrefixS
*/

__deprecated AjBool  ajStrPrefixCO(const char* str, const AjPStr thys)
{
    return ajCharPrefixS(str, thys);
}




/* @obsolete ajStrPrefixCaseCC
** @rename ajCharPrefixCaseC
*/

__deprecated AjBool  ajStrPrefixCaseCC(const char* thys, const char* pref)
{
    return ajCharPrefixCaseC(thys, pref);
}




/* @obsolete ajStrPrefixCaseCO
** @rename ajCharPrefixCaseS
*/

__deprecated AjBool  ajStrPrefixCaseCO(const char* thys, const AjPStr pref)
{
    return ajCharPrefixCaseS(thys, pref);
}




/* @obsolete ajStrSuffixCC
** @rename ajCharSuffixC
*/

__deprecated AjBool  ajStrSuffixCC(const char* str, const char* suff)
{
    return ajCharSuffixC(str, suff);
}




/* @obsolete ajStrSuffixCO
** @rename ajCharSuffixS
*/

__deprecated AjBool  ajStrSuffixCO(const char* txt, const AjPStr suff)
{
    return ajCharSuffixS(txt, suff);
}




/* @obsolete ajStrCmpCaseCC
** @rename ajCharCmpCase
*/

__deprecated int  ajStrCmpCaseCC(const char* str1, const char* str2)
{
    return ajCharCmpCase (str1, str2);
}




/* @obsolete ajStrNCmpCaseCC
** @rename ajCharCmpCaseLen
*/

__deprecated int  ajStrNCmpCaseCC(const char* str1, const char* str2, ajint len)
{
    return ajCharCmpCaseLen(str1, str2, len);
}




/* @obsolete ajStrCmpWildCC
** @rename ajCharCmpWild
*/

__deprecated ajint  ajStrCmpWildCC(const char* str, const char* text)
{
    return ajCharCmpWild (str, text);
}




/* @obsolete ajStrTokCC
** @rename ajCharParseC
*/

__deprecated const AjPStr  ajStrTokCC (const char* txt, const char* delim)
{
    return ajCharParseC(txt, delim);
}




/* @obsolete ajStrDup
** @rename ajStrNewRef
*/

__deprecated AjPStr  ajStrDup(AjPStr thys)
{
    return ajStrNewRef(thys);
}




/* @obsolete ajStrNewL
** @rename ajStrNewRes
*/

__deprecated AjPStr  ajStrNewL(size_t size)
{
    return ajStrNewRes(size);
}




/* @obsolete ajStrNewCL
** @rename ajStrNewResC
*/

__deprecated AjPStr  ajStrNewCL(const char* txt, size_t size)
{
    return ajStrNewResC(txt, size);
}




/* @obsolete ajStrNewCIL
** @rename ajStrNewResLenC
*/

__deprecated AjPStr  ajStrNewCIL(const char* txt, ajint len, size_t size)
{
    return ajStrNewResLenC (txt, size, len);
}




/* @obsolete ajStrDelReuse
** @rename ajStrDelStatic
*/

__deprecated AjBool  ajStrDelReuse(AjPStr* pthis)
{
    ajStrDelStatic(pthis);

    if(*pthis)
        return ajTrue;

    return ajFalse;
}




/* @obsolete ajStrAssC
** @rename ajStrAssignC
*/

__deprecated AjBool  ajStrAssC(AjPStr* pthis, const char* text)
{
    return ajStrAssignC (pthis, text);
}




/* @obsolete ajStrAssK
** @rename ajStrAssignK
*/

__deprecated AjBool  ajStrAssK(AjPStr* pthis, const char text)
{
    return ajStrAssignK(pthis, text);
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




/* @obsolete ajStrSetC
** @rename ajStrAssignEmptyC
*/

__deprecated AjBool  ajStrSetC(AjPStr* pthis, const char* str)
{
    return ajStrAssignEmptyC(pthis, str);
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




/* @obsolete ajStrAssCI
** @rename ajStrAssignLenC
*/

__deprecated AjBool  ajStrAssCI(AjPStr* pthis, const char* txt, size_t ilen)
{
    return ajStrAssignLenC(pthis, txt, ilen);
}




/* @obsolete ajStrCopy
** @rename ajStrAssignRef
*/

__deprecated AjBool  ajStrCopy(AjPStr* pthis, AjPStr str)
{
    return ajStrAssignRef(pthis, str);
}




/* @obsolete ajStrAssCL
** @replace ajStrAssignResC (1,2,3/1,3,2)
*/

__deprecated AjBool  ajStrAssCL(AjPStr* pthis, const char* txt, size_t i)
{
    return ajStrAssignResC(pthis, i, txt);
}




/* @obsolete ajStrAssL
** @replace ajStrAssignResS (1,2,3/1,3,2)
*/

__deprecated AjBool  ajStrAssL(AjPStr* pthis, const AjPStr str, size_t i)
{
    return ajStrAssignResS(pthis, i, str);
}




/* @obsolete ajStrAssSubC
** @rename ajStrAssignSubC
*/

__deprecated AjBool  ajStrAssSubC(AjPStr* pthis, const char* txt,
				 ajint begin, ajint end)
{
    return ajStrAssignSubC(pthis, txt, begin, end);
}




/* @obsolete ajStrAssSub
** @rename ajStrAssignSubS
*/
__deprecated AjBool  ajStrAssSub(AjPStr* pthis, const AjPStr src,
		   ajint beginpos, ajint endpos)
{
    return ajStrAssignSubS(pthis, src, beginpos, endpos);
}




/* @obsolete ajStrAppC
** @rename ajStrAppendC
*/

__deprecated AjBool  ajStrAppC(AjPStr* pthis, const char* txt)
{
    return ajStrAppendC(pthis, txt);
}




/* @obsolete ajStrAppK
** @rename ajStrAppendK
*/

__deprecated AjBool  ajStrAppK(AjPStr* pthis, const char chr)
{
    return ajStrAppendK(pthis, chr);
}




/* @obsolete ajStrApp
** @rename ajStrAppendS
*/

__deprecated AjBool  ajStrApp(AjPStr* pthis, const AjPStr src)
{
    return ajStrAppendS(pthis, src);
}




/* @obsolete ajStrAppKI
** @rename ajStrAppendCountK
*/

__deprecated AjBool  ajStrAppKI(AjPStr* pthis, const char chr, ajint number)
{
    return ajStrAppendCountK(pthis, chr, number);
}




/* @obsolete ajStrAppCI
** @rename ajStrAppendLenC
*/

__deprecated AjBool  ajStrAppCI(AjPStr* pthis, const char* txt, size_t i)
{
    return ajStrAppendLenC(pthis, txt, i);
}




/* @obsolete ajStrAppSub
** @rename ajStrAppendSubS
*/

__deprecated AjBool  ajStrAppSub(AjPStr* pthis, const AjPStr src,
			       ajint begin, ajint end)
{
    return ajStrAppendSubS(pthis, src, begin, end);
}




/* @obsolete ajStrInsert
** @rename ajStrInsertS
*/

__deprecated AjBool  ajStrInsert(AjPStr* Pstr, ajint pos, const AjPStr str )
{
    return ajStrInsertS(Pstr, pos, str);
}




/* @obsolete ajStrMask
** @rename ajStrMaskRange
*/

__deprecated AjBool ajStrMask(AjPStr* Pstr, ajint pos1, ajint pos2,
                              char maskchr)
{
    return ajStrMaskRange(Pstr, pos1, pos2, maskchr);
}




/* @obsolete ajStrReplaceS
** @rename ajStrPasteS
*/

__deprecated AjBool  ajStrReplaceS( AjPStr* pthis, ajint begin,
				  const AjPStr overwrite)
{
    return ajStrPasteS(pthis, begin, overwrite);
}




/* @obsolete ajStrReplaceK
** @rename ajStrPasteCountK
*/

__deprecated AjBool  ajStrReplaceK( AjPStr* pthis, ajint ibegin,
		      char overwrite, ajint ilen)
{
    return ajStrPasteCountK(pthis, ibegin, overwrite, ilen);
}




/* @obsolete ajStrReplaceC
** @rename ajStrPasteMaxC
*/

__deprecated AjBool  ajStrReplaceC( AjPStr* pthis, ajint begin,
				   const char* overwrite, ajint ilen)
{
    return ajStrPasteMaxC(pthis, begin, overwrite, ilen);
}




/* @obsolete ajStrReplace
** @rename ajStrPasteMaxS
*/
__deprecated AjBool  ajStrReplace( AjPStr* pthis, ajint begin,
				  const AjPStr overwrite, ajint ilen)
{
    return ajStrPasteMaxS(pthis, begin, overwrite, ilen);
}




/* @obsolete ajStrUncomment
** @rename ajStrCutComments
*/

__deprecated AjBool  ajStrUncomment(AjPStr* text)
{
    return ajStrCutComments(text);
}




/* @obsolete ajStrUncommentStart
** @rename ajStrCutCommentsStart
*/

__deprecated AjBool  ajStrUncommentStart(AjPStr* text)
{
    return ajStrCutCommentsStart(text);
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




/* @obsolete ajStrCut
** @rename ajStrCutRange
*/

__deprecated AjBool  ajStrCut(AjPStr* pthis, ajint begin, ajint end)
{
    return ajStrCutRange(pthis, begin, end);
}




/* @obsolete ajStrSub
** @rename ajStrKeepRange
*/

__deprecated AjBool  ajStrSub(AjPStr* pthis, ajint begin, ajint end)
{
    return ajStrKeepRange(pthis, begin, end);
}




/* @obsolete ajStrKeepC
** @rename ajStrKeepSetC
*/

__deprecated AjBool  ajStrKeepC(AjPStr* s, const char* charset)
{
    return ajStrKeepSetC (s, charset);
}




/* @obsolete ajStrKeepAlphaC
** @rename ajStrKeepSetAlphaC
*/

__deprecated AjBool  ajStrKeepAlphaC(AjPStr* s, const char* charset)
{
    return ajStrKeepSetAlphaC(s, charset);
}




/* @obsolete ajStrDegap
** @rename ajStrRemoveGap
*/

__deprecated void  ajStrDegap(AjPStr* thys)
{
    ajStrRemoveGap(thys);
}




/* @obsolete ajStrRemoveNewline
** @rename ajStrRemoveLastNewline
*/

__deprecated void  ajStrRemoveNewline(AjPStr* thys)
{
    ajStrRemoveLastNewline(thys);
}




/* @obsolete ajStrRemoveCharsC
** @rename ajStrRemoveSetC
*/

__deprecated void  ajStrRemoveCharsC(AjPStr* pthis, const char *strng)
{
    ajStrRemoveSetC(pthis, strng);
}




/* @obsolete ajStrCleanWhite
** @rename ajStrRemoveWhite
*/

__deprecated AjBool  ajStrCleanWhite(AjPStr* s)
{
    return ajStrRemoveWhite(s);
}




/* @obsolete ajStrClean
** @rename ajStrRemoveWhiteExcess
*/

__deprecated AjBool ajStrClean(AjPStr* s)
{
    return ajStrRemoveWhiteExcess(s);
}




/* @obsolete ajStrWildPrefix
** @rename ajStrRemoveWild
*/

__deprecated AjBool  ajStrWildPrefix(AjPStr* str)
{
    return ajStrRemoveWild(str);
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




/* @obsolete ajStrChompEnd
** @rename ajStrTrimWhiteEnd
*/

__deprecated AjBool  ajStrChompEnd(AjPStr* pthis)
{
    return ajStrTrimWhiteEnd(pthis);
}




/* @obsolete ajStrTruncate
@rename ajStrTruncateLen
*/

__deprecated AjBool  ajStrTruncate(AjPStr* Pstr, ajint pos)
{
    return ajStrTruncateLen(Pstr, pos);
}




/* @obsolete ajStrSubstituteCC
** @rename ajStrExchangeCC
*/

__deprecated AjBool  ajStrSubstituteCC(AjPStr* pthis, const char* replace,
			 const char* putin)
{
    return ajStrExchangeCC(pthis, replace, putin);
}




/* @obsolete ajStrSubstituteKK
** @rename ajStrExchangeKK
*/

__deprecated AjBool  ajStrSubstituteKK(AjPStr* pthis, char replace, char putin)
{    
    return ajStrExchangeKK(pthis, replace, putin);
}




/* @obsolete ajStrSubstitute
** @rename ajStrExchangeSS
*/

__deprecated AjBool  ajStrSubstitute(AjPStr* pthis,
				    const AjPStr replace, const AjPStr putin)
{    
    return ajStrExchangeSS(pthis, replace, putin);
}




/* @obsolete ajStrConvertCC
** @rename ajStrExchangeSetCC
*/

__deprecated AjBool  ajStrConvertCC(AjPStr* pthis, const char* oldc,
				 const char* newc)
{
    return ajStrExchangeSetCC(pthis, oldc, newc);
}




/* @obsolete ajStrConvert
** @rename ajStrExchangeSetSS
*/

__deprecated AjBool  ajStrConvert(AjPStr* pthis, const AjPStr oldc,
				 const AjPStr newc)
{
    if(!oldc || !newc)
        return ajFalse;

    return ajStrExchangeSetSS(pthis, oldc, newc);
}




/* @obsolete ajStrRev
** @rename ajStrReverse
*/

__deprecated AjBool  ajStrRev(AjPStr* pthis)
{
    return ajStrReverse(pthis);
}




/* @obsolete ajStrCountC
** @rename ajStrCalcCountC
*/

__deprecated ajint  ajStrCountC(const AjPStr str, const char* txt)
{
    return (ajint) ajStrCalcCountC(str, txt);
}




/* @obsolete ajStrCountK
** @rename ajStrCalcCountK
*/

__deprecated ajint  ajStrCountK(const AjPStr str, char ch)
{
    return (ajint) ajStrCalcCountK(str, ch);
}




/* @obsolete ajStrParentheses
** @rename ajStrHasParentheses
*/

__deprecated AjBool  ajStrParentheses(const AjPStr s)
{
    return ajStrHasParentheses(s);
}




/* @obsolete ajStrIsSpace
** @rename ajStrIsWhite
*/

__deprecated AjBool  ajStrIsSpace(const AjPStr thys)
{
    return ajStrIsWhite(thys);
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




/* @obsolete ajStrLen
** @rename ajStrGetLen
*/

__deprecated ajint  ajStrLen(const AjPStr thys)
{
    return (ajint) ajStrGetLen(thys);
}




/* @obsolete MAJSTRLEN
** @rename MAJSTRGETLEN
*/

__deprecated ajint  MAJSTRLEN(const AjPStr thys)
{
    return (ajint) MAJSTRGETLEN(thys);
}




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




/* @obsolete ajStrSize
** @rename ajStrGetRes
*/
__deprecated ajint  ajStrSize(const AjPStr thys)
{
    return (ajint) ajStrGetRes(thys);
}




/* @obsolete MAJSTRSIZE
** @rename MAJSTRGETRES
*/
__deprecated ajint  MAJSTRSIZE(const AjPStr thys)
{
    return (ajint) MAJSTRGETRES(thys);
}




/* @obsolete ajStrRoom
** @rename ajStrGetRoom
*/

__deprecated ajint  ajStrRoom(const AjPStr thys)
{
    return (ajint) ajStrGetRoom(thys);
}




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




/* @obsolete ajStrStrMod
** @rename ajStrGetuniquePtr
*/

__deprecated char  *ajStrStrMod(AjPStr *pthis)
{
    return ajStrGetuniquePtr(pthis);
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




/* @obsolete ajStrClear
** @rename ajStrSetClear
*/

__deprecated AjBool  ajStrClear(AjPStr* pthis)
{
    return ajStrSetClear(pthis);
}




/* @obsolete ajStrModL
** @rename ajStrSetRes
*/
__deprecated AjBool  ajStrModL(AjPStr* pthis, size_t size)
{
    return ajStrSetRes(pthis, size);
}




/* @obsolete ajStrModMinL
** @rename ajStrSetResRound
*/

__deprecated AjBool  ajStrModMinL(AjPStr* pthis, ajuint size)
{
    return ajStrSetResRound(pthis, size);
}




/* @obsolete ajStrFix
** @rename ajStrSetValid
*/

__deprecated void  ajStrFix(AjPStr *pthis)
{
    ajStrSetValid(pthis);

    return;
}




/* @obsolete ajStrFixI
** @rename ajStrSetValidLen
*/

__deprecated void  ajStrFixI(AjPStr* pthis, ajint ilen)
{
    ajStrSetValidLen(pthis, (size_t) ilen);

    return;
}




/* @obsolete ajStrFromDoubleE
** @rename ajStrFromDoubleExp
*/

__deprecated AjBool  ajStrFromDoubleE(AjPStr* Pstr, double val, ajint precision)
{
    return ajStrFromDoubleExp(Pstr, val, precision);
}




/* @obsolete ajStrBlock
** @rename ajStrFmtBlock
*/

__deprecated AjBool  ajStrBlock(AjPStr* pthis, ajint blksize)
{
    return ajStrFmtBlock(pthis, blksize);
}




/* @obsolete ajStrToLower
** @rename ajStrFmtLower
*/
__deprecated AjBool  ajStrToLower(AjPStr* pthis)
{
    return ajStrFmtLower(pthis);
}




/* @obsolete ajStrToLowerII
** @rename ajStrFmtLowerSub
*/
__deprecated AjBool  ajStrToLowerII(AjPStr* pthis, ajint begin, ajint end)
{
    return ajStrFmtLowerSub(pthis, begin, end);
}




/* @obsolete ajStrQuote
** @rename ajStrFmtQuote
*/

__deprecated void  ajStrQuote(AjPStr* s)
{
    ajStrFmtQuote(s);
}




/* @obsolete ajStrToTitle
** @rename ajStrFmtTitle
*/

__deprecated AjBool  ajStrToTitle(AjPStr* pthis)
{
    return ajStrFmtTitle(pthis);
}




/* @obsolete ajStrToUpper
** @rename ajStrFmtUpper
*/

__deprecated AjBool  ajStrToUpper(AjPStr* pthis)
{
    return ajStrFmtUpper(pthis);
}




/* @obsolete ajStrToUpperII
** @rename ajStrFmtUpperII
*/

__deprecated AjBool  ajStrToUpperII(AjPStr* pthis, ajint begin, ajint end)
{
    return ajStrFmtUpperSub(pthis, begin, end);
}




/* @obsolete ajStrWrap
** @rename ajStrFmtWrap
*/

__deprecated AjBool  ajStrWrap(AjPStr* Pstr, ajint width )
{
    return ajStrFmtWrap(Pstr, width);
}




/* @obsolete ajStrWrapLeft
** @rename ajStrFmtWrapLeft
*/

__deprecated AjBool  ajStrWrapLeft(AjPStr* pthis, ajint width, ajint left)
{
    return ajStrFmtWrapLeft(pthis, width, 0, left);
}




/* @obsolete ajStrMatch
** @rename ajStrMatchS
*/
__deprecated AjBool  ajStrMatch(const AjPStr str, const AjPStr str2)
{
    return ajStrMatchS(str, str2);
}





/* @obsolete ajStrMatchCase
** @rename ajStrMatchCaseS
*/

__deprecated AjBool  ajStrMatchCase(const AjPStr str, const AjPStr str2)
{
    return ajStrMatchCaseS(str, str2);
}




/* @obsolete ajStrMatchWild
** @rename ajStrMatchWildS
*/

__deprecated AjBool  ajStrMatchWild(const AjPStr str, const AjPStr str2)
{
    return ajStrMatchWildS(str, str2);
}




/* @obsolete ajStrMatchWord
** @rename ajStrMatchWildWordS
*/

__deprecated AjBool  ajStrMatchWord(const AjPStr str, const AjPStr str2)
{
    return ajStrMatchWildWordS(str, str2);
}




/* @obsolete ajStrPrefix
** @rename ajStrPrefixS
*/

__deprecated AjBool  ajStrPrefix(const AjPStr str, const AjPStr str2)
{
    return ajStrPrefixS(str, str2);
}




/* @obsolete ajStrPrefixCase
** @rename ajStrPrefixCaseS
*/

__deprecated AjBool  ajStrPrefixCase(const AjPStr str, const AjPStr str2)
{
    return ajStrPrefixCaseS(str,str2);
}




/* @obsolete ajStrSuffix
** @rename ajStrSuffixS
*/

__deprecated AjBool  ajStrSuffix(const AjPStr str, const AjPStr str2)
{
    return ajStrSuffixS(str, str2);
}




/* @obsolete ajStrNCmpC
** @rename ajStrCmpLenC
*/

__deprecated int  ajStrNCmpC(const AjPStr str, const char* txt, ajint len)
{
    return ajStrCmpLenC(str, txt, len);
}




/* @obsolete ajStrCmpO
** @rename ajStrCmpS
*/

__deprecated int  ajStrCmpO(const AjPStr thys, const AjPStr anoth)
{
    return ajStrCmpS(thys, anoth);
}




/* @obsolete ajStrCmpCase
** @rename ajStrCmpCaseS
*/

__deprecated int  ajStrCmpCase(const AjPStr str, const AjPStr str2)
{
    return ajStrCmpCaseS(str, str2);
}




/* @obsolete ajStrNCmpO
** @rename ajStrCmpLenS
*/

__deprecated int  ajStrNCmpO(const AjPStr str, const AjPStr str2, ajint len)
{
    return ajStrCmpLenS(str, str2, len);
}




/* @obsolete ajStrCmpWild
** @rename ajStrCmpWildS
*/


__deprecated int  ajStrCmpWild(const AjPStr str, const AjPStr str2)
{
    return ajStrCmpWildS(str, str2);
}




/* @obsolete ajStrCmp
** @rename ajStrVcmp
*/

__deprecated int  ajStrCmp(const void* str, const void* str2)
{
    return ajStrVcmp(str, str2);
}




/* @obsolete ajStrFind
** @rename ajStrFindS
*/

__deprecated ajint  ajStrFind(const AjPStr str, const AjPStr str2)
{
    return (ajint) ajStrFindS(str, str2);
}




/* @obsolete ajStrFindK
** @rename ajStrFindAnyK
*/

__deprecated ajint  ajStrFindK(const AjPStr thys, const char chr)
{
    return (ajint) ajStrFindAnyK(thys, chr);
}




/* @obsolete ajStrFindCase
** @rename ajStrFindCaseS
*/

__deprecated ajint  ajStrFindCase(const AjPStr str, const AjPStr str2)
{
    return (ajint) ajStrFindCaseS(str,str2);
}




/* @obsolete ajStrRFindC
** @rename ajStrFindlastC
*/

__deprecated ajint  ajStrRFindC(const AjPStr thys, const char* text)
{
    return (ajint) ajStrFindlastC(thys, text);
}




/* @obsolete ajStrTokC
** @rename ajStrParseC
*/

__deprecated const AjPStr  ajStrTokC(const AjPStr thys, const char* delim)
{
    return ajStrParseC(thys, delim);
}




/* @obsolete ajStrTokenCount
** @rename ajStrParseCountC
*/

__deprecated ajint  ajStrTokenCount(const AjPStr line, const char *delim)
{
    return ajStrParseCountC(line, delim);
}




/* @obsolete ajStrTokenCountR
** @rename ajStrParseCountMultiC
*/

__deprecated ajint  ajStrTokenCountR(const AjPStr line, const char *delim)
{
    return ajStrParseCountMultiC(line, delim);
}




/* @obsolete ajStrListToArray
** @rename ajStrParseSplit
*/

__deprecated ajint  ajStrListToArray(const AjPStr thys, AjPStr **array)
{
    return ajStrParseSplit(thys, array);
}




/* @obsolete ajStrTok
** @rename ajStrParseWhite
*/

__deprecated const AjPStr  ajStrTok(const AjPStr str)
{
    return ajStrParseWhite(str);
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
** @replace ajCvtSposToPos (1,2/'ajStrGetLen[1]',2)
*/

__deprecated ajint  ajStrPos(const AjPStr thys, ajint ipos)
{
    return (ajint) ajCvtSposToPos(thys->Len, ipos);
}




/* @obsolete ajStrPosI
** @replace ajCvtSposToPosStart (1,2,3/'ajStrGetLen[1]',2,3)
*/

__deprecated ajint  ajStrPosI(const AjPStr thys, ajint imin, ajint ipos)
{
    return (ajint) ajCvtSposToPosStart(thys->Len, imin, ipos);
}




/* @obsolete ajStrPosII
** @rename ajCvtSposToPosStart
*/

__deprecated ajint  ajStrPosII(ajint ilen, ajint imin, ajint ipos)
{
    return (ajint) ajCvtSposToPosStart(ilen, imin, ipos);
}




/* @obsolete ajCharPos
** @replace ajCvtSposToPos (1,2/'strlen[1]',2)
*/

__deprecated ajint  ajCharPos(const char* thys, ajint ipos)
{
    return (ajint) ajCvtSposToPos(strlen(thys), ipos);
}




/* @obsolete ajStrIter
** @rename ajStrIterNew
*/

__deprecated AjIStr  ajStrIter(const AjPStr str)
{
    return ajStrIterNew(str);
}




/* @obsolete ajStrIterBack
** @rename ajStrIterNewBack
*/

__deprecated AjIStr  ajStrIterBack(const AjPStr str)
{
    return ajStrIterNewBack(str);
}




/* @obsolete ajStrIterFree
** @rename ajStrIterDel
*/

__deprecated void  ajStrIterFree(AjIStr* iter)
{
    ajStrIterDel(iter);
}




/* @obsolete ajStrIterBackDone
** @rename ajStrIterDoneBack
*/

__deprecated AjBool  ajStrIterBackDone(AjIStr iter)
{
    return ajStrIterDoneBack(iter);
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




/* @obsolete ajStrTokenInit
** @rename ajStrTokenNewC
*/

__deprecated AjPStrTok  ajStrTokenInit(const AjPStr thys, const char* delim)
{
    return ajStrTokenNewC(thys, delim);
}




/* @obsolete ajStrTokenClear
** @rename ajStrTokenDel
*/

__deprecated void  ajStrTokenClear(AjPStrTok* token)
{
    ajStrTokenDel(token);
    return;
}




/* @obsolete ajStrTokenAss
** @rename ajStrTokenAssignC
*/

__deprecated AjBool  ajStrTokenAss(AjPStrTok* ptok, const AjPStr thys,
				  const char* delim)
{
    return ajStrTokenAssignC(ptok, thys, delim);
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

    return ajStrTokenNextFind(ptoken, pthis);
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

    return ajStrTokenNextParse(ptoken, pthis);
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

    for(i=0; thys[i];i++)
	ajStrDel(&thys[i]);

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
#endif

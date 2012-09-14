/* @source ajreg **************************************************************
**
** AJAX REG (ajax regular expression) functions
**
** Uses the Perl-Comparible Regular Expressions Library (PCRE)
** included as a sepoarate library in the EMBOSS distribution.
**
** @author Copyright (C) 1998 Peter Rice
** @version $Revision: 1.34 $
** @modified Jun 25 pmr First version
** @modified 1999-2011 pmr Replace Henry Spencer library with PCRE
** @modified $Date: 2011/10/18 14:23:40 $ by $Author: rice $
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

#include "ajreg.h"



static ajlong regAlloc     = 0;
static ajlong regFree      = 0;
static ajlong regFreeCount = 0;
static ajlong regCount     = 0;
static ajlong regTotal     = 0;

static int *regDfaWorkspace = NULL;
static int regDfaWsCount = 10000;


/* constructors */




/* @func ajRegComp ************************************************************
**
** Compiles a regular expression.
**
** @param [r] rexp [const AjPStr] Regular expression string.
** @return [AjPRegexp] Compiled regular expression.
**
** @release 1.0.0
** @@
******************************************************************************/

AjPRegexp ajRegComp(const AjPStr rexp)
{
    return ajRegCompC(ajStrGetPtr(rexp));
}




/* @func ajRegCompC ***********************************************************
**
** Compiles a regular expression.
**
** @param [r] rexp [const char*] Regular expression character string.
** @return [AjPRegexp] Compiled regular expression.
**
** @release 1.0.0
** @@
******************************************************************************/

AjPRegexp ajRegCompC(const char* rexp)
{
    AjPRegexp ret;
    int options = 0;
    int errpos  = 0;
    const char *errptr            = NULL;
    const unsigned char *tableptr = NULL;

    AJNEW0(ret);
    AJCNEW0(ret->ovector, AJREG_OVECSIZE);
    ret->ovecsize = AJREG_OVECSIZE/3;
    ret->pcre = pcre_compile(rexp, options, &errptr, &errpos, tableptr);

    if(!ret->pcre)
    {
	ajErr("Failed to compile regular expression '%s' at position %d: %s",
	      rexp, errpos, errptr);
	AJFREE(ret);
	return NULL;
    }

    regAlloc += sizeof(ret);
    regCount ++;
    regTotal ++;
    /*ajDebug("ajRegCompC %x size %d regexp '%s'\n",
      ret, (int) sizeof(ret), rexp);*/

    return ret;
}




/* @func ajRegCompCase ********************************************************
**
** Compiles a case-insensitive regular expression.
**
** @param [r] rexp [const AjPStr] Regular expression string.
** @return [AjPRegexp] Compiled regular expression.
**
** @release 2.8.0
** @@
******************************************************************************/

AjPRegexp ajRegCompCase(const AjPStr rexp)
{
    return ajRegCompCaseC(ajStrGetPtr(rexp));
}




/* @func ajRegCompCaseC *******************************************************
**
** Compiles a case-insensitive regular expression.
**
** @param [r] rexp [const char*] Regular expression character string.
** @return [AjPRegexp] Compiled regular expression.
**
** @release 2.8.0
** @@
******************************************************************************/

AjPRegexp ajRegCompCaseC(const char* rexp)
{
    AjPRegexp ret;
    int options = PCRE_CASELESS;
    int errpos  = 0;
    const char *errptr            = NULL;
    const unsigned char *tableptr = NULL;

    AJNEW0(ret);
    AJCNEW0(ret->ovector, AJREG_OVECSIZE);
    ret->ovecsize = AJREG_OVECSIZE/3;
    ret->pcre = pcre_compile(rexp, options, &errptr, &errpos, tableptr);

    if(!ret->pcre)
    {
	ajErr("Failed to compile regular expression '%s' at position %d: %s",
	      rexp, errpos, errptr);
	AJFREE(ret);

	return NULL;
    }

    regAlloc += sizeof(ret);
    regCount ++;
    regTotal ++;
    /*ajDebug("ajRegCompCaseC %x size %d regexp '%s'\n",
      ret, (int) sizeof(ret), rexp);*/

    return ret;
}




/* execute expression match */




/* @func ajRegExec ************************************************************
**
** Execute a regular expression search.
** The expression must first have been compiled with ajRegComp or ajRegCompC.
**
** Internal data structures in the expression will be set to substrings
** which other functions can retrieve.
**
** @param [u] prog [AjPRegexp] Compiled regular expression.
** @param [r] str [const AjPStr] String to be compared.
** @return [AjBool] ajTrue if a match was found.
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajRegExec(AjPRegexp prog, const AjPStr str)
{
    int startoffset = 0;
    int options     = 0;
    int status      = 0;

    status = pcre_exec(prog->pcre, prog->extra, ajStrGetPtr(str),
                       ajStrGetLen(str), startoffset, options, prog->ovector,
                       3*prog->ovecsize);

    if(status >= 0)
    {
	prog->orig = ajStrGetPtr(str);

	if(status == 0)
	    ajWarn("ajRegExec too many substrings");

	return ajTrue;
    }

    if(status < -1)		    /* -1 is a simple fail to match */
    {				/* others are recursion limits etc. */
	ajDebug("ajRegExec returned unexpected status '%d'\n", status);
	prog->orig = ajStrGetPtr(str);	/* needed for the trace */
	ajRegTrace(prog);
    }

    prog->orig = NULL;

    return ajFalse;
}




/* @func ajRegExecC ***********************************************************
**
** Execute a regular expression search.
** The expression must first have been compiled with ajRegComp or ajRegCompC.
**
** Internal data structures in the expression will be set to substrings
** which other functions can retrieve.
**
** @param [u] prog [AjPRegexp] Compiled regular expression.
** @param [r] str [const char*] String to be compared.
** @return [AjBool] ajTrue if a match was found.
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajRegExecC(AjPRegexp prog, const char* str)
{
    int startoffset = 0;
    int options     = 0;
    int status      = 0;

    status = pcre_exec(prog->pcre, prog->extra, str, strlen(str),
		       startoffset, options, prog->ovector, 3*prog->ovecsize);

    if(status >= 0)
    {
	prog->orig = str;

	if(status == 0)
	    ajWarn("ajRegExecC too many substrings");

	return ajTrue;
    }

    if(status < -1)		    /* -1 is a simple fail to match */
    {				/* others are recursion limits etc. */
	ajDebug("ajRegExecC returned unexpected status '%d'\n", status);
	prog->orig = str;		/* needed for the trace */
	ajRegTrace(prog);
    }

    prog->orig = NULL;

    return ajFalse;
}




/* @func ajRegExecall *********************************************************
**
** Execute a regular expression search using the alternative 'DFA' algorithm
** which generates all matches instead of the perl-compatible maximum match.
**
** The expression must first have been compiled with ajRegComp or ajRegCompC.
**
** Internal data structures in the expression will be set to substrings
** which other functions can retrieve.
**
** @param [u] prog [AjPRegexp] Compiled regular expression.
** @param [r] str [const AjPStr] String to be compared.
** @return [AjBool] ajTrue if a match was found.
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajRegExecall(AjPRegexp prog, const AjPStr str)
{
    int startoffset = 0;
    int options     = 0;

    if(!regDfaWorkspace)
        AJCNEW(regDfaWorkspace, regDfaWsCount);
        
    prog->matches = pcre_dfa_exec(prog->pcre, prog->extra,
                                  ajStrGetPtr(str), ajStrGetLen(str),
                                  startoffset, options,
                                  prog->ovector, 3*prog->ovecsize,
                                  regDfaWorkspace, regDfaWsCount);

    if(prog->matches >= 0)
    {
	prog->orig = ajStrGetPtr(str);

	if(prog->matches == 0)
	    ajWarn("ajRegExecall too many substrings");

	return ajTrue;
    }

    if(prog->matches < -1)		    /* -1 is a simple fail to match */
    {				/* others are recursion limits etc. */
	ajDebug("ajRegExecall returned unexpected status '%d'\n",
                prog->matches);
	prog->orig = ajStrGetPtr(str);	/* needed for the trace */
	ajRegTrace(prog);
    }

    prog->orig = NULL;

    return ajFalse;
}




/* @func ajRegExecallC ********************************************************
**
** Execute a regular expression search using the alternative 'DFA' algorithm
** which generates all matches instead of the perl-compatible maximum match.
**
** The expression must first have been compiled with ajRegComp or ajRegCompC.
**
** Internal data structures in the expression will be set to substrings
** which other functions can retrieve.
**
** @param [u] prog [AjPRegexp] Compiled regular expression.
** @param [r] str [const char*] String to be compared.
** @return [AjBool] ajTrue if a match was found.
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajRegExecallC(AjPRegexp prog, const char* str)
{
    int startoffset = 0;
    int options     = 0;

    if(!regDfaWorkspace)
        AJCNEW(regDfaWorkspace, regDfaWsCount);
        
    prog->matches = pcre_dfa_exec(prog->pcre, prog->extra,
                                  str, strlen(str),
                                  startoffset, options,
                                  prog->ovector, 3*prog->ovecsize,
                                  regDfaWorkspace, regDfaWsCount);

    if(prog->matches >= 0)
    {
	prog->orig = str;

	if(prog->matches == 0)
	    ajWarn("ajRegExecallC too many substrings");

	return ajTrue;
    }

    if(prog->matches < -1)		    /* -1 is a simple fail to match */
    {				/* others are recursion limits etc. */
	ajDebug("ajRegExecallC returned unexpected status '%d'\n",
                prog->matches);
	prog->orig = str;		/* needed for the trace */
	ajRegTrace(prog);
    }

    prog->orig = NULL;

    return ajFalse;
}




/* @func ajRegGetMatches ******************************************************
**
** After a successful regular expression match, uses the regular
** expression and the original string to calculate the offset
** of the match from the start of the string.
**
** @param [r] rp [const AjPRegexp] Compiled regular expression.
** @return [ajint] Number of matches found
**
** @release 1.0.0
** @@
******************************************************************************/

ajint ajRegGetMatches(const AjPRegexp rp)
{
    return (rp->matches);
}




/* @func ajRegOffset **********************************************************
**
** After a successful regular expression match, uses the regular
** expression and the original string to calculate the offset
** of the match from the start of the string.
**
** This information is normally lost during processing.
**
** @param [r] rp [const AjPRegexp] Compiled regular expression.
** @return [ajint] Offset of match from start of string.
**               -1 if the string and the expression do not match.
**
** @release 1.0.0
** @@
******************************************************************************/

ajint ajRegOffset(const AjPRegexp rp)
{
    return (rp->ovector[0]);
}




/* @func ajRegOffsetI *********************************************************
**
** After a successful regular expression match, uses the regular
** expression and the original string to calculate the offset
** of a substring from the start of the string.
**
** This information is normally lost during processing.
**
** @param [r] rp [const AjPRegexp] Compiled regular expression.
** @param [r] isub [ajint] Substring number.
** @return [ajint] Offset of match from start of string.
**               -1 if the string and the expression do not match.
**
** @release 1.0.0
** @@
******************************************************************************/

ajint ajRegOffsetI(const AjPRegexp rp, ajint isub)
{
    if(isub < 0)
	ajErr("Invalid substring number %d", isub);

    if(isub >= (rp->ovecsize))
	ajErr("Invalid substring number %d", isub);

    return (rp->ovector[isub*2]);
}




/* @func ajRegLenI ************************************************************
**
** After a successful comparison, returns the length of a substring.
**
** @param [r] rp [const AjPRegexp] Compiled regular expression.
** @param [r] isub [ajint] Substring number.
** @return [ajint] Substring length, or 0 if not found.
**
** @release 1.0.0
** @@
******************************************************************************/

ajint ajRegLenI(const AjPRegexp rp, ajint isub)
{
    ajint istart;
    ajint iend;

    istart = 2*isub;
    iend   = istart+1;

    if(isub < 0)
	return 0;

    if(isub >= rp->ovecsize)
	return 0;

    if(rp->ovector[istart] < 0)
	return 0;

    return (rp->ovector[iend] - rp->ovector[istart]);
}




/* @func ajRegPost ************************************************************
**
** After a successful match, returns the remainder of the string.
**
** @param [r] rp [const AjPRegexp] Compiled regular expression.
** @param [w] post [AjPStr*] String to hold the result.
** @return [AjBool] ajTrue on success.
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajRegPost(const AjPRegexp rp, AjPStr* post)
{
    if(rp->ovector[1])
    {
	ajStrAssignC(post, &rp->orig[rp->ovector[1]]);

	return ajTrue;
    }

    ajStrDelStatic(post);

    return ajFalse;
}




/* @func ajRegPostC ***********************************************************
**
** After a successful match, returns the remainder of the string.
** Result is a character string, which is set to point to the internal
** string data. This in turn is part of the original string. If this
** changes then the results are undefined.
**
** @param [r] rp [const AjPRegexp] Compiled regular expression.
** @param [w] post [const char**] Character string to hold the result.
** @return [AjBool] ajTrue on success.
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajRegPostC(const AjPRegexp rp, const char** post)
{
    if(rp->ovector[1])
    {
	*post = &rp->orig[rp->ovector[1]];

	return ajTrue;
    }

    *post = 0;

    return ajFalse;
}




/* @func ajRegPre *************************************************************
**
** After a successful match, returns the string before the match.
**
** @param [r] rp [const AjPRegexp] Compiled regular expression.
** @param [w] dest [AjPStr*] String to hold the result.
** @return [AjBool] ajTrue on success.
**
** @release 2.8.0
** @@
******************************************************************************/

AjBool ajRegPre(const AjPRegexp rp, AjPStr* dest)
{
    ajint ilen;

    ilen = rp->ovector[0];
    ajStrSetRes(dest, ilen+1);

    if(ilen)
    {
	memmove((*dest)->Ptr, rp->orig, ilen);
	(*dest)->Len = ilen;
	(*dest)->Ptr[ilen] = '\0';

	return ajTrue;
    }

    ajStrDelStatic(dest);

    return ajFalse;
}




/* @func ajRegSubI ************************************************************
**
** After a successful match, returns a substring.
**
** @param [r] rp [const AjPRegexp] Compiled regular expression.
** @param [r] isub [ajint] Substring number.
** @param [w] dest [AjPStr*] String to hold the result.
** @return [AjBool] ajTrue if a substring was defined
**                  ajFalse if the substring is not matched
**                  ajFalse if isub is out of range
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajRegSubI(const AjPRegexp rp, ajint isub, AjPStr* dest)
{
    ajint ilen;
    ajint istart;
    ajint iend;

    istart = 2*isub;
    iend   = istart+1;

    if(isub < 0)
    {
	ajStrDelStatic(dest);

	return ajFalse;
    }

    if(isub >= rp->ovecsize)
    {
	ajStrDelStatic(dest);

	return ajFalse;
    }

    if(rp->ovector[istart] < 0)
    {
	ajStrDelStatic(dest);

	return ajFalse;
    }

    ilen = rp->ovector[iend] - rp->ovector[istart];
    ajStrSetRes(dest, ilen+1);

    if(ilen)
	memmove((*dest)->Ptr, &rp->orig[rp->ovector[istart]], ilen);
    (*dest)->Len = ilen;
    (*dest)->Ptr[ilen] = '\0';

    return ajTrue;
}




/* destructor */




/* @func ajRegFree ************************************************************
**
** Clears and frees a compiled regular expression.
**
** @param [d] pexp [AjPRegexp*] Compiled regular expression.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajRegFree(AjPRegexp* pexp)
{
    AjPRegexp exp;

    if(!pexp)
	return;

    if(!*pexp)
	return;

    exp = *pexp;

    /*
       ajDebug("ajRegFree %x size regexp %d\n", exp,
       (ajint) sizeof(exp));
    */

    regFreeCount += 1;
    regFree += sizeof(*exp);

    if(exp->pcre)
	regFree += sizeof(exp->pcre);

    if(exp->extra)
	regFree += sizeof(exp->extra);

    regTotal --;

    AJFREE(exp->pcre);
    AJFREE(exp->extra);
    AJFREE(exp->ovector);
    AJFREE(*pexp);

    return;
}




/* @func ajRegTrace ***********************************************************
**
** Traces a compiled regular expression with debug calls.
**
** @param [r] rexp [const AjPRegexp] Compiled regular expression.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajRegTrace(const AjPRegexp rexp)
{
    ajint isub;
    ajint ilen;
    ajint ipos;
    ajint istart;
    ajint iend;
    static AjPStr str = NULL;

    ajDebug("  REGEXP trace\n");

    if (!rexp->orig)
	ajDebug("original string not saved - unable to trace string values\n");

    for(isub=0; isub < rexp->ovecsize; isub++)
    {
	istart = 2*isub;
	iend   = istart+1;

	if (!rexp->orig)
        {
	    if(!isub)
		ajDebug("original string from %d .. %d\n",
			rexp->ovector[istart], rexp->ovector[iend]);
	    else
		ajDebug("substring %2d from %d .. %d\n",
			isub, rexp->ovector[istart], rexp->ovector[iend]);

	    continue;
	}

	if(rexp->ovector[iend] >= rexp->ovector[istart])
	{
	    ilen = rexp->ovector[iend] - rexp->ovector[istart];
	    ajStrSetRes(&str, ilen+1);
	    memmove(str->Ptr, &rexp->orig[rexp->ovector[istart]], ilen);
	    str->Len = ilen;
	    str->Ptr[ilen] = '\0';

	    if(!isub)
	    {
		ajDebug(" original string '%s'\n", rexp->orig);
		ajDebug("    string match '%S'\n", str);
	    }
	    else
	    {
		ipos = rexp->ovector[istart];
		ajDebug("    substring %2d '%S' at %d\n", isub, str, ipos);
	    }
	}
    }

    ajDebug("\n");

    ajStrDel(&str);

    return;
}




/* @func ajRegExit ************************************************************
**
** Prints a summary of regular expression (AjPRegexp) usage with debug calls
**
** @return [void]
**
** @release 2.7.0
** @@
******************************************************************************/

void ajRegExit(void)
{
    if(regDfaWorkspace)
        AJFREE(regDfaWorkspace);

    ajDebug("Regexp usage (bytes): %Ld allocated, %Ld freed, %Ld in use "
            "(sizes change)\n",
	    regAlloc, regFree, (regAlloc - regFree));
    ajDebug("Regexp usage (number): %Ld allocated, %Ld freed %Ld in use\n",
	    regCount, regFreeCount, regTotal);

    return;
}

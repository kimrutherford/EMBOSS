/* @source ajrange ************************************************************
**
** AJAX range functions
**
** @author Copyright (C) 1999 Alan Bleasby
** @version $Revision: 1.43 $
** @modified Aug 21 ajb First version
** @modified 7 Sept 1999 GWW - String range edit functions added
** @modified 5 Nov 1999 GWW - store text after pairs of numbers
** @modified $Date: 2012/07/17 12:45:26 $ by $Author: rice $
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


#include "ajlib.h"

#include "ajrange.h"
#include "ajsys.h"
#include "ajseq.h"
#include "ajfileio.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */



/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */



/* ==================================================================== */
/* ========================= constructors ============================= */
/* ==================================================================== */




/* @section Range Constructors ************************************************
**
** All constructors return a new object by pointer. It is the responsibility
** of the user to first destroy any previous object. The target pointer
** does not need to be initialised to NULL, but it is good programming practice
** to do so anyway.
**
******************************************************************************/




/* @func ajRangeNewI **********************************************************
**
** Default constructor for AJAX range objects.
**
** @param [r] n [ajuint] number of ranges
**
** @return [AjPRange] Pointer to a range object
** @category new [AjPRange] Default constructor for range objects
**
** @release 1.0.0
** @@
******************************************************************************/

AjPRange ajRangeNewI(ajuint n)
{
    AjPRange thys;

    AJNEW0(thys);

    thys->n = n;

    if(n > 0)
    {
        thys->start = AJALLOC0(n * sizeof (ajuint));
        thys->end   = AJALLOC0(n * sizeof (ajuint));
        thys->text  = AJALLOC0(n * sizeof (AjPStr *));
    }

    return thys;
}




/* @func ajRangeNewFilename ***************************************************
**
** Load a range object from a file
**
** The format of the range file is:
** Comment lines start with '#' in the first column.
** Comment lines and blank lines are ignored.
** The line may start with white-space.
** There are two positive numbers per line separated by white-space.
** The second number must be greater or equal to the first number.
** There is optional text after the two numbers.
** White-space before or after the text is removed.
**
** e.g.:
**
** # this is my set of ranges
** 12   23
**  4   5       this is like 12-23, but smaller
** 67   10348   interesting region
**
** @param [r] name [const AjPStr] range file name
**
** @return [AjPRange] range object
** @category new [AjPRange] Create a range object from a file
**
** @release 6.2.0
** @@
******************************************************************************/

AjPRange ajRangeNewFilename(const AjPStr name)
{
    return ajRangeNewFilenameLimits(name, 1, UINT_MAX, 0, 0);
}




/* @func ajRangeNewFilenameLimits *********************************************
**
** Load a range object from a file
**
** The format of the range file is:
** Comment lines start with '#' in the first column.
** Comment lines and blank lines are ignored.
** The line may start with white-space.
** There are two positive numbers per line separated by white-space.
** The second number must be greater or equal to the first number.
** There is optional text after the two numbers.
** White-space before or after the text is removed.
**
** e.g.:
**
** # this is my set of ranges
** 12   23
**  4   5       this is like 12-23, but smaller
** 67   10348   interesting region
**
** @param [r] name [const AjPStr] range file name
** @param [r] imin [ajuint] Minimum value
** @param [r] imax [ajuint] Maximum value
** @param [r] minsize [ajuint] Minimum number of values
** @param [r] size [ajuint] Required number of values, zero for unlimited
**
** @return [AjPRange] range object
** @category new [AjPRange] Create a range object from a file
**
** @release 6.2.0
** @@
******************************************************************************/

AjPRange ajRangeNewFilenameLimits(const AjPStr name, ajuint imin, ajuint imax,
                                  ajuint minsize, ajuint size)
{
    AjPRange ret = NULL;
    AjPFile infile = NULL;
    AjPStr line   = NULL;
    char whiteSpace[] = "- \t\n\r";
    char notSpace[]   = "\n\r";
    AjPStrTok tokens;
    ajuint n = 0U;                       /* ranges found so far */
    ajuint k;
    ajuint numone;
    ajuint numtwo;

    AjPStr one = NULL;
    AjPStr two = NULL;
    AjPStr text = NULL;

    AjPList onelist;
    AjPList twolist;
    AjPList textlist;


    onelist  = ajListstrNew();
    twolist  = ajListstrNew();
    textlist = ajListstrNew();


    if((infile = ajFileNewInNameS(name)) == NULL)
        return NULL;

    while(ajReadlineTrim(infile, &line))
    {
        ajStrTrimWhite(&line);

        if(!ajStrFindC(line, "#")) /* starts with # (comment line) */
            continue;

        if(!ajStrGetLen(line))  /* empty line */
            continue;

        /*
        ** parse the numbers out of the line and store in temporary
        ** list (we may be reading data from stdin, so we can't read
        ** in once to count the number of ajRange elements, close
        ** file, open it again and read the data again to populate
        ** ajRange)
        */

        tokens = ajStrTokenNewC(line, whiteSpace);

        one = ajStrNew();
        ajStrTokenNextParse(&tokens, &one);
        ajListstrPushAppend(onelist, one);
        one = NULL;

        two = ajStrNew();
        ajStrTokenNextParse(&tokens, &two);

        if(ajStrGetLen(two))
        {
            ajListstrPushAppend(twolist, two);
            two = NULL;
        }
        else
        {
            ajWarn("Odd integer(s) in range specification:\n%S\n", line);

            return NULL;
        }

        /* get any remaining text and store in temporary list */
        text = ajStrNew();
        ajStrTokenNextParseC(&tokens, notSpace, &text);
        ajStrTrimWhite(&text);
        ajListstrPushAppend(textlist, text);
        text = NULL;

        ajStrTokenDel( &tokens);
    }

    ajFileClose(&infile);
    ajStrDel(&line);

    /* now we know how many pairs of numbers to store, create ajRange object */
    n  = (ajuint) ajListstrGetLength(onelist);
    if(size)
    {
        if(n != size)
        {
            ajWarn("Range specification requires exactly %d pairs",
                   size);

            return NULL;
        }
    }
    else if (n < minsize)
    {
        ajWarn("Range specification requires at least %d pairs",
               minsize);

        return NULL;
    }

    ret = ajRangeNewI(n);


    /* populate ajRange object from lists and check numbers are valid */
    for(k = 0; k < n; k++)
    {
        ajListstrPop(onelist, &one);
        if(!ajStrToUint(one, &numone))
        {
            ajWarn("Bad range value [%S]",one);
            ajRangeDel(&ret);

            return NULL;
        }

        ajListstrPop(twolist, &two);
        if(!ajStrToUint(two, &numtwo))
        {
            ajWarn("Bad range value [%S]",two);
            ajRangeDel(&ret);

            return NULL;
        }

        ajStrDel(&one);
        ajStrDel(&two);

        if(numone > numtwo)
        {
            ajWarn("From range [%d] greater than To range [%d]",
                   numone, numtwo);
            ajRangeDel(&ret);

            return NULL;
        }

        if (numone < imin)
        {
            ajWarn("From range [%d] less than minimum [%d]",
                   numone,imin);
            ajRangeDel(&ret);

            return NULL;
        }

        if (numtwo > imax)
        {
            ajWarn("To range [%d] greater than maximum [%d]",
                   numtwo,imax);
            ajRangeDel(&ret);

            return NULL;
        }

        ret->start[k] = numone;
        ret->end[k]   = numtwo;

        /* do the text */
        ajListstrPop(textlist, &text);
        ret->text[k] = text;
        text = NULL;
    }


    ajListstrFreeData(&onelist);
    ajListstrFreeData(&twolist);
    ajListstrFreeData(&textlist);

    return ret;
}




/* @func ajRangeNewRange ******************************************************
**
** Copy constructor for AJAX range objects.
**
** @param [r] src [const AjPRange] Source range
**
** @return [AjPRange] Pointer to a range object
** @category new [AjPRange] Copy constructor for range objects
**
** @release 6.2.0
** @@
******************************************************************************/

AjPRange ajRangeNewRange(const AjPRange src)
{
    AjPRange thys;
    ajuint i;
    ajuint n;

    AJNEW0(thys);

    n = src->n;

    thys->n = n;

    if(src->n > 0)
    {
        thys->start = AJALLOC0(n * sizeof (ajuint));
        thys->end   = AJALLOC0(n * sizeof (ajuint));
        thys->text  = AJALLOC0(n * sizeof (AjPStr *));

        for(i = 0; i < n; i++)
        {
            thys->start[i] = src->start[i];
            thys->end[i] = src->end[i];
            ajStrAssignS(&thys->text[i], src->text[i]);
        }
    }

    return thys;
}




/* @func ajRangeNewString *****************************************************
**
** Create a range object from a string
**
** @param [r] str [const AjPStr] range string
**
** @return [AjPRange] range object
** @category new [AjPRange] Create a range object from a string
**
** @release 6.2.0
** @@
******************************************************************************/

AjPRange ajRangeNewString(const AjPStr str)
{
    return ajRangeNewStringLimits(str, 1, UINT_MAX, 0, 0);
}




/* @func ajRangeNewStringLimits ***********************************************
**
** Create a range object from a string
**
** @param [r] str [const AjPStr] range string
** @param [r] imin [ajuint] Minimum value
** @param [r] imax [ajuint] Maximum value
** @param [r] minsize [ajuint] Minimum number of values
** @param [r] size [ajuint] Required number of values, zero for unlimited
**
** @return [AjPRange] range object
** @category new [AjPRange] Create a range object from a string
**
** @release 6.2.0
** @@
******************************************************************************/

AjPRange ajRangeNewStringLimits(const AjPStr str, ajuint imin, ajuint imax,
                                ajuint minsize, ajuint size)
{
    AjPRange ret = NULL;
    AjPStr c1 = NULL;
    AjPStr c2 = NULL;
    AjPStr c3 = NULL;
    static AjPStr s   =NULL;
    const char *cp;
    char *p;
    ajuint n;
    ajuint e;
    ajuint f;
    ajuint t;
    ajuint i;
    AjBool doneone = ajFalse;

    const char *nondigit="abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
        " \t\n\r!@#$%^&*()_-+=|\\~`{[}]:;\"'<,>.?/";
    const char *digit="0123456789";

    ajStrAssignS(&s, str);

    /* clean up the ranges string */
    ajStrTrimWhite(&s);

    /* is this a file of ranges? (does it start with a '@' ?) */
    if(*(ajStrGetPtr(s)) == '@')
    {
        /* knock off the '@' */
        ajStrKeepRange(&s, 1, ajStrGetLen(s));
        ret = ajRangeNewFilenameLimits(s, imin, imax, minsize, size);
    }
    else
    {
        /* get some copies of the string for parsing with strtok */
        ajStrAssignS(&c1, s);
        ajStrAssignS(&c2, s);
        ajStrAssignS(&c3, s);

        cp = ajStrGetPtr(c1);
        p = ajSysFuncStrtok(cp, nondigit);

        n = 0;

        if(p)
        {
            /*
             *  count the pairs of numbers so that we know the size of
             *  arrays to create
             */
            ++n;
            while((p = ajSysFuncStrtok(NULL, nondigit)))
                ++n;

            if(n % 2)
            {
                ajWarn("Odd integer(s) in range specification [%d]",n);

                return NULL;
            }

            if(size)
            {
                if(n != size)
                {
                    ajWarn("Range specification requires exactly %d pairs",
                           size);

                    return NULL;
                }
            }
            else if (n < minsize)
            {
                ajWarn("Range specification requires at least %d pairs",
                       minsize);

                return NULL;
            }
            ret = ajRangeNewI((e = n >> 1));

            /* get the pairs of numbers and put them in the AjPRange object */
            cp = ajStrGetPtr(c2);
            p = ajSysFuncStrtok(cp, nondigit);

            if(!sscanf(p,"%u",&f))
            {
                ajWarn("Bad range value [%s]",p);
                ajRangeDel(&ret);

                return NULL;
            }

            p = ajSysFuncStrtok(NULL, nondigit);

            if(!sscanf(p, "%u", &t))
            {
                ajWarn("Bad range value [%s]",p);
                ajRangeDel(&ret);

                return NULL;
            }

            if(f>t)
            {
                ajWarn("From range [%d] greater than To range [%d]", f, t);
                ajRangeDel(&ret);

                return NULL;
            }

            if (f < imin)
            {
                ajWarn("From range [%d] less than minimum [%d]", f, imin);
                ajRangeDel(&ret);

                return NULL;
            }

            if (t > imax)
            {
                ajWarn("To range [%d] greater than maximum [%d]", t, imax);
                ajRangeDel(&ret);

                return NULL;
            }

            ret->start[0] = f;
            ret->end[0] = t;

            for(i = 1; i < e; ++i)
            {
                p = ajSysFuncStrtok(NULL, nondigit);

                if(!sscanf(p, "%u", &f))
                {
                    ajWarn("Bad range value [%s]", p);
                    ajRangeDel(&ret);

                    return NULL;
                }

                p = ajSysFuncStrtok(NULL, nondigit);

                if(!sscanf(p, "%u", &t))
                {
                    ajWarn("Bad range value [%s]", p);
                    ajRangeDel(&ret);

                    return NULL;
                }

                if(f > t)
                {
                    ajWarn("From range [%d] greater than To range [%d]", f, t);
                    ajRangeDel(&ret);

                    return NULL;
                }

                ret->start[i] = f;
                ret->end[i]   = t;
            }

            /* now get any strings after the pairs of ranges */
            cp = ajStrGetPtr(c3);

            if(!isdigit((ajint) *cp))
            {
                doneone = ajTrue;
                p = ajSysFuncStrtok(cp, digit);
            }

            for(i = 0; i < e; ++i)
            {
                /* ignore anything between the two numbers */
                if(!doneone)
                {
                    p = ajSysFuncStrtok(cp, digit);
                    doneone = ajTrue;
                }
                else
                    p = ajSysFuncStrtok(NULL, digit);

                /* this must be the text after the pair of numbers */
                /* get the string after the two numbers */
                p = ajSysFuncStrtok(NULL, digit);

                if(p)
                {
                    ajStrAssignC(&(ret->text[i]), p);
                    ajStrTrimWhite(&(ret->text[i]));
                }
            }
        }
        else
        {
            if(size)
            {
                ajWarn("Range specification requires exactly %d pairs",
                       size);

                return NULL;
            }
            else if (0 < minsize)
            {
                ajWarn("Range specification requires at least %d pairs",
                       minsize);

                return NULL;
            }
            ret = ajRangeNewI(0);
        }

        ajStrDel(&c1);
        ajStrDel(&c2);
        ajStrDel(&c3);
    }

    ajStrDel(&s);

    return ret;
}




/* @section Range Destructors ************************************************
**
** Default destructor for AJAX range objects
**
******************************************************************************/




/* @func ajRangeDel ***********************************************************
**
** Default destructor for AJAX range objects.
**
** @param [d] thys [AjPRange *] range structure
**
** @return [void]
** @category delete [AjPRange] Default destructor for range objects
**
** @release 1.0.0
** @@
******************************************************************************/

void ajRangeDel(AjPRange *thys)
{
    ajuint i;

    if(!*thys) return;

    if((*thys)->n > 0)
    {
        AJFREE((*thys)->start);
        AJFREE((*thys)->end);

        for(i = 0; i < (*thys)->n; i++)
            ajStrDel(&(*thys)->text[i]);
    }

    AJFREE((*thys)->text);
    AJFREE(*thys);

    return;
}




/* @section Range Functions ************************************************
**
** Other functions for AJAX range objects
**
******************************************************************************/




/* @func ajRangeGetSize *******************************************************
**
** Return the number of ranges in a range object
**
** @param [r] thys [const AjPRange] range object
**
** @return [ajuint] number of ranges
** @category use [AjPRange] Return the number of ranges in a range object
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ajRangeGetSize(const AjPRange thys)
{
    return thys->n;
}




/* @func ajRangeElementGetText ************************************************
**
** Return (as parameters) text value of a range element
**
** The text value of a range is any non-digit after the pair of range numbers
** e.g. in a pair of range '10-20 potential exon 50-60 repeat'
** the text values of the two ranges are: 'potential exon' and 'repeat'
**
** @param [r] thys [const AjPRange] range object
** @param [r] element [ajuint] range element (0 to n-1)
** @param [w] text [AjPStr *] text value
**
** @return [AjBool] true if range exists
** @category use [AjPRange] Return (as parameters) text value of a range
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ajRangeElementGetText(const AjPRange thys, ajuint element, AjPStr * text)
{
    if(element >= thys->n)
        return ajFalse;

    if(thys->text[element])
        ajStrAssignS(text, thys->text[element]);
    else
        ajStrAssignClear(text);

    return ajTrue;
}




/* @func ajRangeElementGetValues **********************************************
**
** Return (as parameters) start and end values in a range
**
** @param [r] thys [const AjPRange] range object
** @param [r] element [ajuint] range element (0 to n-1)
** @param [w] start [ajuint *] start value
** @param [w] end [ajuint *] end value
**
** @return [AjBool] true if range exists
** @category use [AjPRange] Return (as parameters) start and end values
**                          in a range
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ajRangeElementGetValues(const AjPRange thys, ajuint element,
                               ajuint *start, ajuint *end)
{
    if(element >= thys->n)
        return ajFalse;

    if(thys->start[element] < 1)
        return ajFalse;

    if(thys->end[element] < 1)
        return ajFalse;

    if(thys->start[element] > thys->end[element])
        return ajFalse;

    *start = thys->start[element];
    *end   = thys->end[element];

    return ajTrue;
}




/* @func ajRangeElementSet ****************************************************
**
** Set the values of a start and end in a (pre-existing) range element
**
** @param [w] thys [AjPRange] range object
** @param [r] element [ajuint] range element (0 to n-1)
** @param [r] start [ajuint] start value
** @param [r] end [ajuint] end value
**
** @return [AjBool] true if range exists
** @category modify [AjPRange] Set the values of a start and end in a
**                              range element
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ajRangeElementSet(AjPRange thys, ajuint element,
                         ajuint start, ajuint end)
{
    if(element >= thys->n)
        return ajFalse;

    thys->start[element] = start;
    thys->end[element]   = end;

    return ajTrue;
}




/* @func ajRangeSetOffset *****************************************************
**
** Sets the range values offset relative to the Begin value.
**
** Used when a sequence has -sbegin= and -send= parameters set
** and we have extracted the specified subsequence.
** So if -sbeg 11 has been set and the range is 11-12
** the resulting range is changed to 1-2
**
** @param [u] thys [AjPRange] range object
** @param [r] begin [ajuint] begin parameter obtained from ajSeqGetBegin(seq)
**
** @return [AjBool] true if region values modified
** @category modify [AjPRange] Sets the range values relative to the
**                             Begin value
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ajRangeSetOffset(AjPRange thys, ajuint begin)
{
    ajuint nr;
    ajuint i;
    ajuint st;
    ajuint en;
    AjBool result = ajFalse;


    nr = thys->n;

    for(i = 0; i < nr; i++)
    {
        if(begin > 1)
            result = ajTrue;

        ajRangeElementGetValues(thys, i, &st, &en);
        st -= begin - 1;
        en -= begin - 1;
        ajRangeElementSet(thys, i, st, en);
    }

    return result;
}




/* @func ajRangeSeqExtractList ************************************************
**
** Extract the range from a sequence and place the resulting text in a
** list of strings.
**
** N.B. the resulting list will be regions of the input sequence listed
** in the order specified in the set of ranges. If these are not in ascending
** order, the resulting list of strings will not be in ascending order either.
**
** @param [r] thys [const AjPRange] range object
** @param [r] seq [const AjPSeq] sequence to extract from
** @param [w] outliststr [AjPList] resulting list of strings
**
** @return [AjBool] true if result is not the whole sequence
** @category use [AjPRange] PushApp substrings defined by range onto list
**
** @release 3.0.0
** @@
******************************************************************************/

AjBool ajRangeSeqExtractList(const AjPRange thys,
                             const AjPSeq seq, AjPList outliststr)
{

    ajuint nr;
    ajuint i;
    ajuint st;
    ajuint en;
    AjBool result = ajFalse;
    AjPStr str;

    nr = thys->n;

    if(nr)
    {
        for(i = 0; i < nr; i++)
        {
            result = ajTrue;

            if(!ajRangeElementGetValues(thys, i, &st, &en))
                continue;

            str = ajStrNew();
            ajStrAppendSubS(&str, ajSeqGetSeqS(seq), st - 1, en - 1);
            ajListstrPushAppend(outliststr, str);
        }
    }
    else
    {
        str = ajSeqGetSeqCopyS(seq);
        ajListstrPushAppend(outliststr, str);
    }

    return result;
}




/* @func ajRangeSeqExtract ****************************************************
**
** Extract the range from a sequence (Remove regions not in the range(s))
** N.B. the resulting sequence will be regions of the original concatenated
** in the order specified in the set of ranges. If these are not in ascending
** order, the resulting sequence will not be in position order either.
**
** @param [r] thys [const AjPRange] range object
** @param [u] seq [AjPSeq] Sequence
**
** @return [AjBool] true if sequence was modified
** @category use [AjPRange] Extract substrings defined by range
**
** @release 3.0.0
** @@
******************************************************************************/

AjBool ajRangeSeqExtract(const AjPRange thys, AjPSeq seq)
{
    ajuint nr;
    ajuint i;
    ajuint st;
    ajuint en;
    AjBool result = ajFalse;
    AjPStr outstr = NULL;

    nr = thys->n;

    ajDebug("ajRangeSeqExtract Number:%d\n", nr);

    if (nr)
    {
        for(i = 0; i < nr; i++)
        {
            result = ajTrue;

            if(!ajRangeElementGetValues(thys, i, &st, &en))
                continue;

            ajStrAppendSubS(&outstr, ajSeqGetSeqS(seq), st - 1, en - 1);
            ajDebug("Range [%d] %d..%d '%S'\n", i, st, en, outstr);
        }
        ajSeqAssignSeqS(seq, outstr);
        ajStrDel(&outstr);
    }

    return result;
}




/* @func ajRangeSeqExtractPep *************************************************
**
** Extract the range from a sequence (Remove regions not in the range(s))
** and translate to protein.
**
** N.B. the resulting sequence will be regions of the original concatenated
** in the order specified in the set of ranges. If these are not in ascending
** order, the resulting sequence will not be in position order either.
**
** @param [r] thys [const AjPRange] range object
** @param [u] seq [AjPSeq] Sequence
** @param [r] trntable [const AjPTrn] Translation table
** @param [r] frame [ajint] Reading frame 1..3 or -1..-3
**
** @return [AjPSeq] Translated protein sequence
** @category use [AjPRange] Extract substrings defined by range
**
** @release 6.1.0
** @@
******************************************************************************/

AjPSeq ajRangeSeqExtractPep(const AjPRange thys, AjPSeq seq,
                            const AjPTrn trntable, ajint frame)
{
    ajuint nr;
    ajuint i;
    ajuint st;
    ajuint en;
    AjPStr outstr = NULL;
    AjPSeq pepseq = NULL;
    AjPStr pepstr = NULL;
    ajuint pos = 0;
    ajuint lastpos = 0;
    ajuint npos = 0;
    ajuint rlen = 0;
    ajuint shift = 0;

    nr = thys->n;

    if (!nr)
        return NULL;

    if(frame > 0)
    {
        for(i = 0; i < nr; i++)
        {
            if(!ajRangeElementGetValues(thys, i, &st, &en))
                continue;

            ajStrAppendSubS(&outstr, ajSeqGetSeqS(seq), st - 1, en - 1);
        }

        ajSeqAssignSeqS(seq, outstr);
        ajStrAssignClear(&outstr);
        pepseq = ajTrnSeqOrig(trntable, seq, 1);
        ajStrAssignS(&pepstr, ajSeqGetSeqS(pepseq));

        for(i = 0; i < nr; i++)
        {
            if(!ajRangeElementGetValues(thys, i, &st, &en))
                continue;

            shift = npos % 3;

            rlen = (en - st) + 1;
            npos += rlen;
            pos = (npos - 1) / 3;

            if(frame % 3 == (((ajint) st - (ajint) shift) % 3))
            {
                if(pos >= lastpos)
                    ajStrAppendSubS(&outstr, pepstr, lastpos, pos);
            }

            if(pos >= lastpos)
                lastpos = pos + 1;
            else
                lastpos = pos;
        }

        ajSeqAssignSeqS(pepseq, outstr);
        ajStrDel(&pepstr);
        ajStrDel(&outstr);
    }
    else if (frame < 0)
    {
        for(i = nr; i > 0; i--)
        {
            if(!ajRangeElementGetValues(thys, i - 1, &st, &en))
                continue;

            ajStrAppendSubS(&outstr, ajSeqGetSeqS(seq), st - 1, en - 1);
        }

        ajSeqAssignSeqS(seq, outstr);
        ajStrAssignClear(&outstr);
        pepseq = ajTrnSeqOrig(trntable, seq, -1);
        ajStrAssignS(&pepstr, ajSeqGetSeqS(pepseq));

        for(i = 0; i < nr; i++)
        {
            if(!ajRangeElementGetValues(thys, i, &st, &en))
                continue;

            rlen = (en - st) + 1;
            npos += rlen;
            pos = (npos - 1) / 3;

            if((-frame) % 3 == (((ajint) st) % 3))
            {
                if(pos >= lastpos)
                    ajStrAppendSubS(&outstr, pepstr, lastpos, pos);
            }

            if(pos >= lastpos)
                lastpos = pos + 1;
            else
                lastpos = pos;
        }

        ajSeqAssignSeqS(pepseq, outstr);
        ajStrDel(&outstr);
    }

    return pepseq;
}




/* @func ajRangeSeqStuff ******************************************************
**
** The opposite of ajRangeSeqExtract()
** Stuff space characters into a string to pad out to the range.
**
** It takes a string and an ordered, non-overlapping set of ranges and puts
** spaces into the string between the ranges.
** So starting with the string 'abcde' and the ranges 3-5,7-8 it will produce:
** '  abc de'
**
** @param [r] thys [const AjPRange] range object
** @param [u] seq [AjPSeq] Sequence to be modified
**
** @return [AjBool] true if sequence was modified
** @category use [AjPRange] The opposite of ajRangeSeqExtract
**
** @release 3.0.0
** @@
******************************************************************************/

AjBool ajRangeSeqStuff(const AjPRange thys, AjPSeq seq)
{
    ajuint nr;
    ajuint i;
    ajuint j;
    ajuint lasten = 0;
    ajuint lastst = 0;
    ajuint len;
    ajuint st;
    ajuint en;
    AjBool result = ajFalse;
    AjPStr outstr = NULL;

    nr = thys->n;

    if(nr)
    {
        for(i = 0; i < nr; i++)
        {
            result = ajTrue;

            if(!ajRangeElementGetValues(thys, i, &st, &en))
                continue;

            /* change range positions to string positions */
            --st;
            --en;
            len = en - st;

            for(j = lasten; j < st; j++)
                ajStrAppendC(&outstr, " ");

            ajStrAppendSubS(&outstr, ajSeqGetSeqS(seq), lastst, lastst + len);
            lastst = lastst + len + 1;
            lasten = en + 1;
        }

        ajSeqAssignSeqS(seq, outstr);
        ajStrDel(&outstr);
    }

    return result;
}




/* @func ajRangeSeqStuffPep ***************************************************
**
** The opposite of ajRangeSeqExtractPep()
** Stuff space characters into a translated string to pad out to the range.
**
** It takes a string and an ordered, non-overlapping set of ranges and puts
** spaces into the string between the ranges.
** So starting with the string 'abcde' and the ranges 3-5,7-8 it will produce:
** '  abc de'
**
** @param [r] thys [const AjPRange] range object
** @param [u] seq [AjPSeq] Sequence to be modified
** @param [r] frame [ajint] Reading frame 1..3 or -1..-3
**
** @return [AjBool] true if sequence was modified
** @category use [AjPRange] The opposite of ajRangeSeqExtract
**
** @release 6.1.0
** @@
******************************************************************************/

AjBool ajRangeSeqStuffPep(const AjPRange thys, AjPSeq seq, ajint frame)
{
    ajuint nr;
    ajuint i;
    ajuint j;
    ajuint lasten = 0;
    ajuint lastst = 0;
    ajuint len;
    ajuint st;
    ajuint en;
    ajuint nbases = 0;
    ajuint shift = 0;
    AjBool result = ajFalse;
    AjPStr outstr = NULL;
    ajint lastframe = 0;

    nr = thys->n;

    if(!nr)
        return ajFalse;

    if(frame > 0)
    {
        for(i = 0; i < nr; i++)
        {
            result = ajTrue;

            if(!ajRangeElementGetValues(thys, i, &st, &en))
                continue;

            shift = nbases%3;
            nbases += en - st + 1;

            if((frame) % 3 != (((ajint) st - (ajint) shift) % 3))
                continue;

            /* change range positions to string positions */
            --st;
            --en;
            len = en - st;

            ajDebug("lasten:%u st:%u shift:%u frame:%d lastframe:%d\n",
                    lasten, st, shift, frame, lastframe);
            for(j = lasten; j < st; j++)
                ajStrAppendC(&outstr, " ");

            if(shift && (frame != lastframe))
                for(j = 3; j > shift; j--)
                    ajStrAppendC(&outstr, " ");

            ajStrAppendSubS(&outstr, ajSeqGetSeqS(seq), lastst, lastst + len);
            lastst = lastst + len + 1;
            lasten = en + 1;
            lastframe = frame;
        }

        ajSeqAssignSeqS(seq, outstr);
        ajStrDel(&outstr);
    }
    else if (frame < 0)
    {
        ajSeqReverseOnly(seq);

        for(i = nr; i > 0; i--)
        {
            result = ajTrue;
            if(!ajRangeElementGetValues(thys, i - 1, &st, &en))
                continue;

            if((-frame) % 3 != (((ajint) st) % 3))
                continue;

            /* change range positions to string positions */
            --st;
            --en;
            len = en - st;

            for(j = lasten; j < st; j++)
                ajStrAppendC(&outstr, " ");

            ajStrAppendSubS(&outstr, ajSeqGetSeqS(seq), lastst, lastst + len);
            lastst = lastst + len + 1;
            lasten = en + 1;
        }

        ajSeqAssignSeqS(seq, outstr);
        ajStrDel(&outstr);
    }


    return result;
}




/* @func ajRangeSeqMask *******************************************************
**
** Mask the range in a String
**
** @param [r] thys [const AjPRange] range object
** @param [r] maskchar [const AjPStr] character to mask with
** @param [u] seq [AjPSeq] sequence to be masked
**
** @return [AjBool] true if string modified
** @category use [AjPRange] Mask the range in a String
**
** @release 3.0.0
** @@
******************************************************************************/

AjBool ajRangeSeqMask(const AjPRange thys, const AjPStr maskchar, AjPSeq seq)
{
    ajuint nr;
    ajuint i;
    ajuint j;
    ajuint jj;
    ajuint st;
    ajuint en;
    AjBool result = ajFalse;
    AjPStr str = NULL;

    nr = thys->n;

    if (nr)
    {
        for(i = 0; i < nr; ++i)
        {
            result = ajTrue;
            if(!ajRangeElementGetValues(thys, i, &st, &en))
                continue;

            /* change range positions to string positions */
            --st;
            --en;

            /* cut out the region */
            ajStrCutRange(&str, st, en);

            /* replace the region with the mask character */
            for(j = st; j <= en; ++j)
                ajStrInsertS(&str, st, maskchar);
        }
        ajSeqAssignSeqS(seq, str);
        ajStrDel(&str);
    }
    else
    {
        str = ajStrNew();

        for(jj = 0; jj <= ajStrGetLen(str); ++jj)
            ajStrInsertS(&str, jj, maskchar);

        ajSeqAssignSeqS(seq, str);
        ajStrDel(&str);
    }

    return result;
}




/* @func ajRangeSeqToLower ****************************************************
**
** Change the range in a String to lower-case
**
** @param [r] thys [const AjPRange] range object
** @param [u] seq [AjPSeq] sequence to be lower-cased
**
** @return [AjBool] true if sequence was modified
** @category use [AjPRange] Change to lower-case the range in a sequence
**
** @release 3.0.0
** @@
******************************************************************************/

AjBool ajRangeSeqToLower(const AjPRange thys, AjPSeq seq)
{
    ajuint nr;
    ajuint i;
    ajuint st;
    ajuint en;
    AjBool result = ajFalse;
    AjPStr substr = NULL;
    AjPStr str = NULL;
    const AjPStr seqstr;


    nr = thys->n;

    if (nr)
    {
        substr = ajStrNew();
        str = ajStrNew();
        seqstr = ajSeqGetSeqS(seq);

        for(i = 0; i < nr; ++i)
        {
            if(!ajRangeElementGetValues(thys, i, &st, &en))
                continue;

            /* change range positions to string positions */
            --st;
            --en;

            /* extract the region and lowercase */
            ajStrAppendSubS(&substr, seqstr, st, en);
            ajStrFmtLower(&substr);

            /* remove and replace the lowercased region */
            ajStrCutRange(&str, st, en);
            ajStrInsertS(&str, st, substr);
            ajStrSetClear(&substr);
        }

        ajStrDel(&substr);
    }
    else
        ajSeqFmtLower(seq);

    return result;
}




/* @func ajRangeStrExtractList ************************************************
**
** Extract the range from a String and place the resulting text in a
** list of strings.
**
** N.B. the resulting list will be regions of the input string listed
** in the order specified in the set of ranges. If these are not in ascending
** order, the resulting list of strings will not be in ascending order either.
**
** @param [r] thys [const AjPRange] range object
** @param [r] instr [const AjPStr] string to extract from
** @param [w] outliststr [AjPList] resulting list of strings
**
** @return [AjBool] true if string modified
** @category use [AjPRange] PushApp substrings defined by range onto list
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajRangeStrExtractList(const AjPRange thys,
                             const AjPStr instr, AjPList outliststr)
{
    ajuint nr;
    ajuint i;
    ajuint st;
    ajuint en;
    AjBool result = ajFalse;
    AjPStr str;

    nr = thys->n;

    for(i = 0; i < nr; i++)
    {
        result = ajTrue;
        ajRangeElementGetValues(thys, i, &st, &en);
        str = ajStrNew();
        ajStrAppendSubS(&str, instr, st - 1, en - 1);
        ajListstrPushAppend(outliststr, str);
    }

    return result;
}




/* @func ajRangeStrExtract ****************************************************
**
** Extract the range from a String (Remove regions not in the range(s))
**
** N.B. the resulting string will be regions of the input string appended
** in the order specified in the set of ranges. If these are not in ascending
** order, the resulting string will not be in ascending order either.
**
** Intended for use in extracting exon ranges from the sequence of
** a transcript.
**
** @param [r] thys [const AjPRange] range object
** @param [r] instr [const AjPStr] string to extract from
** @param [w] outstr [AjPStr *] resulting extracted string
**
** @return [AjBool] true if string modified
** @category use [AjPRange] Extract substrings defined by range
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajRangeStrExtract(const AjPRange thys, const AjPStr instr,
                         AjPStr *outstr)
{
    ajuint nr;
    ajuint i;
    ajuint st;
    ajuint en;
    AjBool result = ajFalse;

    nr = thys->n;

    ajDebug("ajRangeStrExtract Number:%d\n", nr);

    if (nr)
        for(i = 0; i < nr; i++)
        {
            result = ajTrue;
            ajRangeElementGetValues(thys, i, &st, &en);
            ajStrAppendSubS(outstr, instr, st - 1, en - 1);
            ajDebug("Range [%d] %d..%d '%S'\n", i, st, en, *outstr);
        }
    else
        ajStrAssignS(outstr, instr);

    return result;
}




/* @func ajRangeStrStuff ******************************************************
**
** The opposite of ajRangeStrExtract()
** Stuff space characters into a string to pad out to the range.
**
** It takes a string and an ordered, non-overlapping set of ranges and puts
** spaces into the string between the ranges.
** So starting with the string 'abcde' and the ranges 3-5,7-8 it will produce:
** '  abc de'
**
** @param [r] thys [const AjPRange] range object
** @param [r] instr [const AjPStr] string to stuff
** @param [w] outstr [AjPStr *] resulting stuffed string
**
** @return [AjBool] true if string modified
** @category use [AjPRange] The opposite of ajRangeStrExtract
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajRangeStrStuff(const AjPRange thys, const AjPStr instr, AjPStr *outstr)
{
    ajuint nr;
    ajuint i;
    ajuint j;
    ajuint lasten = 0;
    ajuint lastst = 0;
    ajuint len;
    ajuint st;
    ajuint en;
    AjBool result = ajFalse;

    nr = thys->n;

    for(i = 0; i < nr; i++)
    {
        result = ajTrue;
        ajRangeElementGetValues(thys, i, &st, &en);
        /* change range positions to string positions */
        --st;
        --en;
        len = en - st;

        for(j = lasten; j < st; j++)
            ajStrAppendC(outstr, " ");

        ajStrAppendSubS(outstr, instr, lastst, lastst + len);
        lastst = lastst + len + 1;
        lasten = en + 1;
    }

    return result;
}




/* @func ajRangeStrMask *******************************************************
**
** Mask the positions defined by a range in a string
**
** @param [r] thys [const AjPRange] range object
** @param [r] maskchar [const AjPStr] character to mask with
** @param [w] str [AjPStr *] string to be masked
**
** @return [AjBool] true if string modified
** @category use [AjPRange] Mask the range in a String
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajRangeStrMask(const AjPRange thys, const AjPStr maskchar, AjPStr *str)
{
    ajuint nr;
    ajuint i;
    ajuint j;
    ajuint st;
    ajuint en;
    AjBool result = ajFalse;

    nr = thys->n;

    for(i = 0; i < nr; ++i)
    {
        result = ajTrue;
        ajRangeElementGetValues(thys, i, &st, &en);

        /* change range positions to string positions */
        --st;
        --en;

        /* cut out the region */
        ajStrCutRange(str, st, en);

        /* replace the region with the mask character */
        for(j = st; j <= en; ++j)
            ajStrInsertS(str, st, maskchar);
    }

    return result;
}




/* @func ajRangeStrToLower ****************************************************
**
** Change positions defined by the range in a string to lower-case
**
** @param [r] thys [const AjPRange] range object
** @param [w] str [AjPStr *] string to be lower-cased
**
** @return [AjBool] true if string modified
** @category use [AjPRange] Change to lower-case the range in a String
**
** @release 2.7.0
** @@
******************************************************************************/

AjBool ajRangeStrToLower(const AjPRange thys, AjPStr *str)
{
    ajuint nr;
    ajuint i;
    ajuint st;
    ajuint en;
    AjBool result = ajFalse;
    AjPStr substr;

    substr = ajStrNew();

    nr = thys->n;

    for(i = 0; i < nr; ++i)
    {
        result = ajTrue;
        ajRangeElementGetValues(thys, i, &st, &en);

        /* change range positions to string positions */
        --st;
        --en;

        /* extract the region and lowercase */
        ajStrAppendSubS(&substr, *str, st, en);
        ajStrFmtLower(&substr);

        /* remove and replace the lowercased region */
        ajStrCutRange(str, st, en);
        ajStrInsertS(str, st, substr);
        ajStrSetClear(&substr);
    }

    ajStrDel(&substr);

    return result;
}




/* @func ajRangeElementTypeOverlap ********************************************
**
** Detect an overlap of a single range element to a region of a sequence
**
** @param [r] thys [const AjPRange] range object
** @param [r] element [ajuint] range element (0 to n-1)
** @param [r] pos [ajuint] position in sequence of start of region of sequence
** @param [r] length [ajuint] length of region of sequence
**
** @return [ajuint] 0=no overlap 1=internal 2=complete 3=at left 4=at right
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ajRangeElementTypeOverlap(const AjPRange thys, ajuint element,
                                 ajuint pos, ajuint length)
{
    ajuint start;
    ajuint end;
    ajuint posend;

    if(element >= thys->n)
        return 0;

    if(thys->start[element] < 1)
        return 0;

    if(thys->end[element] < 1)
        return 0;

    if(thys->start[element] > thys->end[element])
        return 0;

    start = thys->start[element];
    end   = thys->end[element];

    /* end position of region in sequence */
    posend = pos + length - 1;

    /* convert range positions to sequence positions */
    start--;
    end--;

    if(end < pos || start > posend)
        return 0;

    /* no overlap       ~~~~ |--------|         */
    if(start >= pos && end <= posend)
        return 1;

    /* internal overlap      |-~~~~~--|         */
    if(start < pos && end > posend)
        return 2;

    /* complete overlap ~~~~~|~~~~~~~~|~~       */
    if(start < pos && end >= pos )
        return 3;

    /* overlap at left  ~~~~~|~~~-----|         */
    if(start >= pos && end > posend )
        return 4;

    /* overlap at right      |----~~~~|~~~      */

    ajFatal("ajRangeElementTypeOverlap error");

    return -1;
}




/* @func ajRangeCountOverlaps *************************************************
**
** Detect overlaps of a set of ranges to a region of a sequence
** @param [r] thys [const AjPRange] range object
** @param [r] pos [ajuint] position in sequence of start of region of sequence
** @param [r] length [ajuint] length of region of sequence
**
** @return [ajuint] Number of ranges in range object with overlaps
**                  to the region
** @category use [AjPRange] Detect overlaps of a set of ranges to a seq region
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ajRangeCountOverlaps(const AjPRange thys, ajuint pos, ajuint length)
{
    ajuint nr;
    ajuint i;
    ajuint result = 0;

    nr = thys->n;

    for(i = 0; i < nr; i++)
    {
        if(ajRangeElementTypeOverlap(thys, i, pos, length))
            result++;
    }

    return result;
}




/* @func ajRangeIsOrdered *****************************************************
**
** Tests to see if the set of ranges are in ascending non-overlapping order
** @param [r] thys [const AjPRange] range object
**
** @return [AjBool] ajTrue if in ascending non-overlapping order
** @category use [AjPRange] Test if ranges are in ascending non-overlapping
**                          order
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ajRangeIsOrdered(const AjPRange thys)
{
    ajuint nr;
    ajuint i;
    ajuint st;
    ajuint en;
    ajuint last = 0;

    nr = thys->n;

    for(i = 0; i < nr; i++)
    {
        ajRangeElementGetValues(thys, i, &st, &en);
        ajDebug("ajRangeOrdered [%u] st:%u en:%u (last:%u)\n",
                i, st, en, last);

        if(st <= last || en <= st)
            return ajFalse;

        last = en;
    }

    return ajTrue;
}




/* @func ajRangeIsWhole *******************************************************
**
** Test whether the default range is used for a sequence
**
** The test is whether the given range is a single range from the start to
** the end of a sequence string.
**
** @param [r] thys [const AjPRange] range object
** @param [r] s [const AjPSeq] sequence
**
** @return [AjBool] true if default range
** @category use [AjPRange] Test if the default range has been set
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ajRangeIsWhole(const AjPRange thys, const AjPSeq s)
{
    /* test the range - 1..end or empty means whole sequence */

    if(thys->n == 0)
    {
        ajDebug("ajRangeDefault n:%d begin:%u end:%u\n",
                thys->n,
                ajSeqGetBegin(s), ajSeqGetEnd(s));

        return ajTrue;
    }

    ajDebug("ajRangeDefault n:%d start:%d end:%d begin:%u end:%u\n",
            thys->n, thys->start[0], thys->end[0],
            ajSeqGetBegin(s), ajSeqGetEnd(s));

    if(thys->n == 1 &&
       thys->start[0] == ajSeqGetBegin(s) &&
       thys->end[0] == ajSeqGetEnd(s))
        return ajTrue;

    return ajFalse;
}




#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif




#ifdef AJ_COMPILE_DEPRECATED
/* @obsolete ajRangeFile
** @rename ajRangeNewFilename
*/

__deprecated AjPRange ajRangeFile(const AjPStr name)
{
    return ajRangeNewFilename(name);
}




/* @obsolete ajRangeFileLimits
** @rename ajRangeNewFilenameLimits
*/

__deprecated AjPRange ajRangeFileLimits(const AjPStr name,
                                        ajuint imin, ajuint imax,
                                        ajuint minsize, ajuint size)
{
    return ajRangeNewFilenameLimits(name, imin, imax, minsize, size);
}




/* @obsolete ajRangeCopy
** @rename ajRangeNewRange
*/

__deprecated AjPRange ajRangeCopy(const AjPRange src)
{
    return ajRangeNewRange(src);
}




/* @obsolete ajRangeGet
** @rename ajRangeNewString
*/

__deprecated AjPRange ajRangeGet(const AjPStr str)
{
    return ajRangeNewString(str);
}




/* @obsolete ajRangeGetLimits
** @rename ajRangeNewStringLimits
*/

__deprecated AjPRange ajRangeGetLimits(const AjPStr str,
                                       ajuint imin, ajuint imax,
                                       ajuint minsize, ajuint size)
{
    return ajRangeNewStringLimits(str, imin, imax, minsize, size);
}




/* @obsolete ajRangeNumber
** @rename ajRangeGetSize
*/

__deprecated ajuint ajRangeNumber(const AjPRange thys)
{
    return thys->n;
}




/* @obsolete ajRangeText
** @rename ajRangeElementGetText
*/

__deprecated AjBool ajRangeText(const AjPRange thys, ajuint element,
                                AjPStr * text)
{
    return ajRangeElementGetText(thys, element, text);
}




/* @obsolete ajRangeValues
** @rename ajRangeElementGetValues
*/

__deprecated AjBool ajRangeValues(const AjPRange thys, ajuint element,
                                  ajuint *start, ajuint *end)
{
    return ajRangeElementGetValues(thys, element, start, end);
}




/* @obsolete ajRangeChange
** @rename ajRangeElementSet
*/
__deprecated AjBool ajRangeChange(AjPRange thys, ajuint element,
                                  ajuint start, ajuint end)
{
    return ajRangeElementSet(thys, element, start, end);
}




/* @obsolete ajRangeBegin
** @rename ajRangeSetOffset
*/

__deprecated AjBool ajRangeBegin(AjPRange thys, ajuint begin)
{
    return ajRangeSetOffset(thys, begin);
}




/* @obsolete ajRangeOverlapSingle
** @remove Use ajRangeElementTypeOverlap
*/

__deprecated ajuint ajRangeOverlapSingle(ajuint start, ajuint end,
                                         ajuint pos, ajuint length)
{
    ajuint posend;

    /* end position of region in sequence */
    posend = pos + length - 1;

    /* convert range positions to sequence positions */
    start--;
    end--;

    if(end < pos || start > posend)
        return 0;

    /* no overlap               ~~~~ |--------| */
    if(start >= pos && end <= posend)
        return 1;

    /* internal overlap      |-~~~~~--|         */
    if(start < pos && end > posend)
        return 2;

    /* complete overlap ~~~~~|~~~~~~~~|~~       */
    if(start < pos && end >= pos )
        return 3;

    /* overlap at left  ~~~~~|~~~-----|         */
    if(start >= pos && end > posend )
        return 4;

    /* overlap at right      |----~~~~|~~~      */

    ajFatal("ajrangeoverlapsingle error");

    return -1;
}




/* @obsolete ajRangeOverlaps
** @rename ajRangeCountOverlaps
*/

__deprecated ajuint ajRangeOverlaps(const AjPRange thys,
                                    ajuint pos, ajuint length)
{
    return ajRangeCountOverlaps(thys, pos, length);
}




/* @obsolete ajRangeOrdered
** @rename ajRangeIsOrdered
*/

__deprecated AjBool ajRangeOrdered(const AjPRange thys)
{
    return ajRangeIsOrdered(thys);
}




/* @obsolete ajRangeDefault
** @rename ajRangeIsWhole
*/

__deprecated AjBool ajRangeDefault(const AjPRange thys, const AjPSeq s)
{
    return ajRangeIsWhole(thys, s);
}
#endif

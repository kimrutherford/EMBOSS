/* @source trimest application
**
** Trim poly-A tails off EST sequences
**
** @author Copyright (C) Gary Williams (gwilliam@hgmp.mrc.ac.uk)
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
#include <ctype.h>		/* for tolower, toupper */




static ajint trimest_get_tail(const AjPSeq seq, ajint direction,
			      ajint minlength, ajint mismatches);

static void trimest_tolower(AjPStr *strng, ajint start, ajint end);



/* @prog trimest **************************************************************
**
** Trim poly-A tails off EST sequences
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall	seqall;
    AjPSeqout	seqout;
    AjPSeq seq = NULL;
    AjPStr str = NULL;
    AjPStr desc = NULL;
    ajint  tail3;
    ajint  tail5 = 0;
    ajint  minlength;
    ajint  mismatches;
    AjBool reverse;
    AjBool fiveprime;
    AjBool cvttolower;

    embInit("trimest", argc, argv);

    seqall 	= ajAcdGetSeqall("sequence");
    seqout 	= ajAcdGetSeqoutall("outseq");
    minlength 	= ajAcdGetInt("minlength");
    mismatches 	= ajAcdGetInt("mismatches");
    reverse	= ajAcdGetBoolean("reverse");
    fiveprime	= ajAcdGetBoolean("fiveprime");
    cvttolower  = ajAcdGetToggle("tolower");
    

    str = ajStrNew();

    while(ajSeqallNext(seqall, &seq))
    {
        /* get sequence description */
        ajStrAssignS(&desc, ajSeqGetDescS(seq));

        /* get positions to cut in 5' poly-T and 3' poly-A tails */
	if(fiveprime)
	    tail5 = trimest_get_tail(seq, 5, minlength, mismatches);
	tail3 = trimest_get_tail(seq, 3, minlength, mismatches);

	/* get a COPY of the sequence string */
	ajStrAssignS(&str, ajSeqGetSeqS(seq));

        /* cut off longest of 3' or 5' tail */
	if(tail5 > tail3)
	{
	    /* if 5' poly-T tail, then reverse the sequence */
	    ajDebug("Tail=%d\n", tail5);
            if(cvttolower)
                trimest_tolower(&str, 0, tail5-1);
            else
	        ajStrKeepRange(&str, tail5, ajSeqGetLen(seq)-1);
	    ajStrAppendC(&desc, " [poly-T tail removed]");

	}
	else if(tail3 > tail5)
	{
	    /* remove 3' poly-A tail */
	    ajDebug("Tail=%d\n", tail3);
            if(cvttolower)
                trimest_tolower(&str, ajSeqGetLen(seq)-tail3,
				ajSeqGetLen(seq));
            else
	        ajStrKeepRange(&str, 0, ajSeqGetLen(seq)-tail3-1);
            ajStrAppendC(&desc, " [poly-A tail removed]");

	}

        /* write sequence out */
	ajSeqAssignSeqS(seq, str);

	/* reverse complement if poly-T found */
	if(tail5 > tail3 && reverse)
	{
	    ajSeqReverseForce(seq);
	    ajStrAppendC(&desc, " [reverse complement]");
	}

        /* set description */
        ajSeqAssignDescS(seq, desc);

	ajSeqoutWriteSeq(seqout, seq);
    }

    ajSeqoutClose(seqout);

    ajStrDel(&str);
    ajStrDel(&desc);

    ajSeqallDel(&seqall);
    ajSeqDel(&seq);
    ajSeqoutDel(&seqout);

    embExit();

    return 0;
}




/* @funcstatic trimest_get_tail ***********************************************
**
** Trim sequence
**
** @param [r] seq [const AjPSeq] sequence
** @param [r] direction [ajint] 5 = 5' end, 3 = 3' end
** @param [r] minlength [ajint] minimum length of tail to cut
** @param [r] mismatches [ajint] max allowed contiguous mismatches in tail
** @return [ajint] length of tail (0 if no tail found)
** @@
******************************************************************************/

static ajint trimest_get_tail(const AjPSeq seq, ajint direction,
			      ajint minlength, ajint mismatches)
{
    char t;
    const char *s;
    char c;
    ajint inc;
    ajint start;
    ajint end;
    ajint i;
    ajint mismatchcount; /* number of contiguous mismatches */
    ajint polycount;	 /* length of poly-A/T since end/last mismatch */
    ajint length;	 /* length of tail looked at so far */
    ajint result;	 /* resulting length of tail */

    if(direction == 5)
    {
	t = 'T';
    	inc = 1;
    	start = 0;
	end = ajSeqGetLen(seq);

    }
    else
    {
	t = 'A';
    	inc = -1;
    	start = ajSeqGetLen(seq)-1;
    	end = -1;
    }

    s = ajSeqGetSeqC(seq);

    mismatchcount = 0;
    polycount = 0;
    length = 0;
    result = 0;

    for(i = start; i != end; i += inc, length++)
    {
        c = toupper((int)s[i]);
	ajDebug("end = %d, c=%c\n", direction, c);
        if(c == t)
	{
            polycount++;
            mismatchcount = 0;
        }
	else if(c != 'N')
	{
	    /*
	    ** There is a mismatch
	    ** N is ignored - it is not a poly-tail or a mismatch
	    */
            polycount = 0;
            mismatchcount++;
        }

        if(polycount >= minlength)
            result = length+1;
	ajDebug("end = %d, polycount = %d, so far tail=%d\n",
		direction, polycount, result);

        if(mismatchcount > mismatches)
            break;
    }

    return result;
}

/* @funcstatic trimest_tolower ***********************************************
**
** Change a part of a string to lowercase
**
** @param [u] strng [AjPStr *] string to change
** @param [r] start [ajint] start of region of string to change
** @param [r] end   [ajint] end of region of string to change
** @return [void] 
** @@
******************************************************************************/

static void trimest_tolower(AjPStr *strng, ajint start, ajint end)
{

    AjPStr substr;
    substr = ajStrNew();
    
    /* extract the region and lowercase */
    ajStrAppendSubS(&substr, *strng, start, end);
    ajStrFmtLower(&substr);
    
    /* remove and replace the lowercased region */
    ajStrCutRange(strng, start, end);
    ajStrInsertS(strng, start, substr);
    ajStrDel(&substr);

    return;
}

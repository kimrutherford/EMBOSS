/* @source palindrome application
**
** Brute force inverted repeat finder. Allows mismatches but not gaps
**
** @author Copyright (C) Mark Faller
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

static AjBool overlap;




/* @datastatic Palindrome *****************************************************
**
** Palindrome internals
**
** @attr forwardStart [ajint] Start forward
** @attr forwardEnd [ajint] End forward
** @attr revStart [ajint] Start reverse
** @attr revEnd [ajint] End reverse
** @attr next [struct palindrome*] Pointer to next in list
******************************************************************************/

typedef struct palindrome
{
   ajint forwardStart;
   ajint forwardEnd;
   ajint revStart;
   ajint revEnd;
   struct palindrome *next;
} *Palindrome;




static Palindrome palindrome_New(ajint fstart, ajint fend, ajint rstart,
				 ajint rend);
static AjBool palindrome_AInB(const Palindrome a, const Palindrome b);
static AjBool palindrome_AOverB(const Palindrome a, const Palindrome b);
static AjBool palindrome_Over(ajint astart, ajint aend, ajint bstart,
			     ajint bend);
static void   palindrome_Print(AjPFile outfile,
			       const AjPStr seq, const Palindrome pal,
			       ajint maxLen);
static AjBool palindrome_Longer(const Palindrome a, const Palindrome b);
static void   palindrome_Swap(const Palindrome a, Palindrome b);




/* @prog palindrome ***********************************************************
**
** Looks for inverted repeats in a nucleotide sequence
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPSeqall seqall;
    AjPSeq sequence;
    AjPFile outfile;
    ajint minLen;
    ajint maxLen;
    ajint maxGap;
    ajint beginPos;
    ajint endPos;
    ajint maxmismatches;

    AjPStr seqstr;
    ajint current;
    ajint rev;
    ajint count;
    ajint gap;

    ajint begin;
    ajint end;
    ajint mismatches;
    ajint mismatchAtEnd;
    ajint istart;
    ajint iend;
    ajint ic;
    ajint ir;

    AjBool alln;		/* TRUE if all of palindrome is N's */

    Palindrome pfirstpal;
    Palindrome plastpal = NULL;
    Palindrome ppal = NULL;
    Palindrome pnext = NULL;

    AjBool found = AJFALSE;

    embInit("palindrome", argc, argv);

    seqall  = ajAcdGetSeqall("sequence");
    minLen  = ajAcdGetInt("minpallen");
    maxLen  = ajAcdGetInt("maxpallen");
    maxGap  = ajAcdGetInt("gaplimit");
    outfile = ajAcdGetOutfile("outfile");

    maxmismatches = ajAcdGetInt("nummismatches");
    overlap       = ajAcdGetBool("overlap");

    while(ajSeqallNext(seqall, &sequence))
    {
	beginPos = ajSeqallGetseqBegin(seqall);
	endPos = ajSeqallGetseqEnd(seqall);

	/* set to NULL to indicate that we have no first palindrome find yet */
	pfirstpal = NULL;

	/* write header to file */

	ajFmtPrintF(outfile, "Palindromes of:  %s \n", ajSeqGetNameC(sequence));
	ajFmtPrintF(outfile, "Sequence length is: %d \n", ajSeqGetLen(sequence));
	ajFmtPrintF(outfile, "Start at position: %d\nEnd at position: %d\n",
		    beginPos, endPos);
	ajFmtPrintF(outfile,"Minimum length of Palindromes is: %d \n", minLen);
	ajFmtPrintF(outfile,"Maximum length of Palindromes is: %d \n", maxLen);
	ajFmtPrintF(outfile,"Maximum gap between elements is: %d \n", maxGap);
	ajFmtPrintF(outfile,"Number of mismatches allowed in Palindrome: %d\n",
		    maxmismatches);
	ajFmtPrintF(outfile, "\n\n\n");
	ajFmtPrintF(outfile, "Palindromes:\n");


	/* set vars in readiness to enter loop */
	seqstr = ajStrNewC(ajSeqGetSeqC(sequence));
	begin  = beginPos - 1;
	end    = endPos - 1;

	ajStrFmtLower(&seqstr); /* make comparisons case independent */

	/* loop to look for inverted repeats */
	for(current = begin; current < end; current++)
	{
	    iend = current + 2*(maxLen) + maxGap;
	    if(iend > end)
		iend = end;
	    istart = current + minLen;

	    for(rev = iend; rev > istart; rev--)
	    {
		count = 0;
		mismatches = 0;
		mismatchAtEnd = 0;
		alln = ajTrue;
		ic = current;
		ir = rev;
		if(ajStrGetCharPos(seqstr, ic) ==
		   ajBaseComp(ajStrGetCharPos(seqstr, ir)))
		    while(mismatches <= maxmismatches && ic < ir)
		    {
			if(ajStrGetCharPos(seqstr, ic++) ==
			   ajBaseComp(ajStrGetCharPos(seqstr, ir--)))
			{
			    mismatchAtEnd = 0;
			    if(ajStrGetCharPos(seqstr, ic-1) != 'n')
				alln = ajFalse;
			}
			else
			{
			    mismatches++;
			    mismatchAtEnd++;
			}
			count++;
		    }

		count -= mismatchAtEnd;
		gap = rev - current - count - count + 1;

		/* Find out if there's a reverse repeat long enough */
		if(count >= minLen && gap <= maxGap && !alln)
		{
		    /* create new struct to hold palindrome data */
		    ppal = palindrome_New(current,(current+count),rev,
					  (rev-count));

		    /*
		    ** if it is the first palindrome find then save it as start
		    **  of palindrome list
		    */
		    if(pfirstpal == NULL)
		    {
			pfirstpal = ppal;
			plastpal = ppal;
		    }
		    else
		    {
			/* Is it  a subset of a palindrome already met */
			pnext = pfirstpal;
			found = AJFALSE;
			while(pnext != NULL)
			{
			    if(overlap && palindrome_AInB(ppal, pnext))
			    {
				found = AJTRUE;
				break;
			    }

			    if(!overlap && palindrome_AOverB(ppal, pnext))
			    {
				if(palindrome_Longer(ppal, pnext))
				{
				    palindrome_Swap(ppal, pnext);
				}

				found = AJTRUE;
				break;
			    }
			    pnext = pnext->next;
			}

			/* if new palindrome add to end of list */
			if(!found)
			{
			    plastpal->next = ppal;
			    plastpal = ppal;
			}
			else
			    AJFREE(ppal);
		    }
		}
	    }
	}
    
    
    
    
	/* Print out palindromes */
	ppal = pfirstpal;
	while(ppal != NULL)
	{
	    palindrome_Print(outfile, seqstr, ppal, maxLen);
	    ppal = ppal->next;
	}
    
    
	/* make a gap beween outputs of different sequences */
	ajFmtPrintF(outfile, "\n\n\n");
    
    
	/* free memory used for palindrome list */
	ppal = pfirstpal;
	while(ppal != NULL)
	{
	    pnext = ppal->next;
	    AJFREE(ppal);
	    ppal = pnext;
	}
    
	ajStrDel(&seqstr);
    
    }

    ajFileClose(&outfile);

    ajSeqallDel(&seqall);
    ajSeqDel(&sequence);
    ajStrDel(&seqstr);

    embExit();

    return 0;
}




/* @funcstatic palindrome_New *************************************************
**
** Create new Palindrome object
**
** @param [r] fstart [ajint] Undocumented
** @param [r] fend [ajint] Undocumented
** @param [r] rstart [ajint] Undocumented
** @param [r] rend [ajint] Undocumented
** @return [Palindrome] Undocumented
** @@
******************************************************************************/

static Palindrome palindrome_New(ajint fstart, ajint fend, ajint rstart,
				 ajint rend)
{

    Palindrome pal;

    AJNEW(pal);
    pal->forwardStart = fstart;
    pal->forwardEnd   = fend;
    pal->revStart     = rstart;
    pal->revEnd       = rend;
    pal->next         = NULL;

    return pal;
}




/* @funcstatic palindrome_AInB ************************************************
**
** Test whether Palindrome A is within Palindrome B (is a subset of B)
** in both halves of the stem
**
** @param [r] a [const Palindrome] Undocumented
** @param [r] b [const Palindrome] Undocumented
** @return [AjBool] Undocumented
** @@
******************************************************************************/

static AjBool palindrome_AInB(const Palindrome a, const Palindrome b)
{
    if((a->forwardStart >= b->forwardStart) &&
       (a->forwardEnd <=b->forwardEnd))
	if((a->revStart <= b->revStart) &&
	   (a->revEnd >= b->revEnd))
	    return AJTRUE;

    return AJFALSE;
}




/* @funcstatic palindrome_AOverB **********************************************
**
** Test whether Palindrome A overlaps Palindrome A on both halves of the stem
**
** @param [r] a [const Palindrome] Undocumented
** @param [r] b [const Palindrome] Undocumented
** @return [AjBool] Undocumented
** @@
******************************************************************************/

static AjBool palindrome_AOverB(const Palindrome a, const Palindrome b)
{
    if(palindrome_Over(a->forwardStart, a->forwardEnd,
		       b->forwardStart, b->forwardEnd) &&
       palindrome_Over(a->revEnd, a->revStart,
		       b->revEnd, b->revStart))
	return AJTRUE;

    return AJFALSE;
}




/* @funcstatic palindrome_Over ************************************************
**
** Test whether two regions overlap each other
**
** @param [r] astart [ajint] Undocumented
** @param [r] aend [ajint] Undocumented
** @param [r] bstart [ajint] Undocumented
** @param [r] bend [ajint] Undocumented
** @return [AjBool] Undocumented
** @@
******************************************************************************/

static AjBool palindrome_Over(ajint astart, ajint aend, ajint bstart,
			      ajint bend)
{
    if(astart >= bstart && astart <= bend)
	return ajTrue;
    if(bstart >= astart && bstart <= aend)
	return ajTrue;

    return ajFalse;
}




/* @funcstatic palindrome_Longer **********************************************
**
** Test whether the stem of Palindrome A is longer than the stem of
** Palindrome B
**
** @param [r] a [const Palindrome] Undocumented
** @param [r] b [const Palindrome] Undocumented
** @return [AjBool] Undocumented
** @@
******************************************************************************/

static AjBool palindrome_Longer(const Palindrome a, const Palindrome b)
{
    if((a->forwardEnd - a->forwardStart) >
       (b->forwardEnd - b->forwardStart))
	return ajTrue;

    return ajFalse;
}




/* @funcstatic palindrome_Swap ************************************************
**
** Set Palindrome B to be the same position as Palindrome A 
**
** @param [r] a [const Palindrome] Undocumented
** @param [w] b [Palindrome] Undocumented
** @@
******************************************************************************/

static void palindrome_Swap(const Palindrome a, Palindrome b)
{
    b->forwardStart =  a->forwardStart;
    b->forwardEnd =  a->forwardEnd;
    b->revStart =  a->revStart;
    b->revEnd =  a->revEnd;

    return;
}




/* @funcstatic palindrome_Print ***********************************************
**
** Output results to file
**
** @param [u] outfile [AjPFile] Undocumented
** @param [r] seq [const AjPStr] Undocumented
** @param [r] pal [const Palindrome] Undocumented
** @param [r] maxLen [ajint] Maximum palindrome length
** @@
******************************************************************************/

static void palindrome_Print(AjPFile outfile,
			     const AjPStr seq, const Palindrome pal,
			     ajint maxLen)
{

    ajint i;
    ajint j;

    if(pal->forwardEnd - pal->forwardStart > maxLen)
	return;

    ajFmtPrintF(outfile, "%-8d ", (pal->forwardStart+1));
    for(i = pal->forwardStart; i < pal->forwardEnd; i++)
	ajFmtPrintF(outfile, "%c", ajStrGetCharPos(seq, i));

    ajFmtPrintF(outfile, " %8d\n         ", pal->forwardEnd);

    for(i = pal->forwardStart,
	j=pal->revStart; i < pal->forwardEnd; i++)
	if(ajStrGetCharPos(seq, i) == ajBaseComp(ajStrGetCharPos(seq, j--)))
	    ajFmtPrintF(outfile, "|");
	else
	    ajFmtPrintF(outfile, " ");

    ajFmtPrintF(outfile, "\n%-8d ", (pal->revStart+1));

    for(i = pal->revStart; i > pal->revEnd; i--)
	ajFmtPrintF(outfile, "%c", ajStrGetCharPos(seq, i));

    ajFmtPrintF(outfile, " %8d\n\n", (pal->revEnd+2));

    return;
}

/* @source trimseq application
**
** Trim ambiguous bits off the ends of sequences
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

#define TRIMCHARSET 256		/* size of character set */




static ajint trimseq_trim(const AjPSeq seq, ajint sense, AjBool isnuc,
			  ajint window, float percent, AjBool strict,
			  AjBool star);
static void trimseq_parole(AjBool *gang, const char *good_guys);




/* @prog trimseq **************************************************************
**
** Trim ambiguous bits off the ends of sequences
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPSeqout seqout;
    AjPSeq seq = NULL;
    ajint window;
    AjBool left;
    AjBool right;
    AjBool strict;
    AjBool star;
    float percent;
    AjBool isnuc;
    AjPStr str = NULL;
    ajint start;
    ajint end;


    embInit("trimseq", argc, argv);

    seqall  = ajAcdGetSeqall("sequence");
    seqout  = ajAcdGetSeqoutall("outseq");
    window  = ajAcdGetInt("window");
    percent = ajAcdGetFloat("percent");
    left    = ajAcdGetBoolean("left");
    right   = ajAcdGetBoolean("right");
    strict  = ajAcdGetBoolean("strict");
    star    = ajAcdGetBoolean("star");

    str = ajStrNew();

    while(ajSeqallNext(seqall, &seq))
    {
	/* is this a protein or nucleic sequence? */
	isnuc = ajSeqIsNuc(seq);

	/* find the left start */
	if(left)
	    start = trimseq_trim(seq, 1, isnuc, window, percent, strict, star)
		+ 1;
	else
	    start = 0;

	/* find the right end */
	if(right)
	    end = trimseq_trim(seq, 0, isnuc, window, percent, strict, star)
		- 1;
	else
	    end = ajSeqGetLen(seq)-1;

	/* get a COPY of the sequence string */
	ajStrAssignS(&str, ajSeqGetSeqS(seq));

	ajStrKeepRange(&str, start, end);
	ajSeqAssignSeqS(seq, str);
	ajSeqoutWriteSeq(seqout, seq);
    }

    ajSeqoutClose(seqout);

    ajStrDel(&str);
    ajSeqallDel(&seqall);
    ajSeqDel(&seq);
    ajSeqoutDel(&seqout);

    embExit();

    return 0;
}




/* @funcstatic trimseq_trim ***************************************************
**
** Trim sequence
**
** @param [r] seq [const AjPSeq] sequence
** @param [r] sense [ajint] 0=right trim    1=left trim
** @param [r] isnuc [AjBool] whether nucleic or protein
** @param [r] window [ajint] window size
** @param [r] percent [float] % ambiguity in window
** @param [r] strict [AjBool] trim off all IUPAC ambiguity codes, not just X, N
** @param [r] star [AjBool] trim off asterisks in proteins
** @return [ajint] position to trim to or -1
**                 or ajSeqGetLen(seq) if no bad characters were found
** @@
******************************************************************************/

static ajint trimseq_trim(const AjPSeq seq,
			  ajint sense, AjBool isnuc, ajint window,
			  float percent, AjBool strict, AjBool star)
{
    ajint leroy_brown;		/* last bad character */
    ajint suspect;		/* possible last bad character */
    AjBool gang[TRIMCHARSET];	/*
				 * all the characters - true if a bad one to
				 * be removed
				 */
    ajint i;
    ajint a;		/* start position of window */
    ajint z;		/* position to end moving window to */
    ajint inc;		/* increment value to move window (+-1) */
    ajint count;	/* count of bad characters in window */
    ajint look;		/* position in wind we are looking at */
    float pc;		/* percentage of bad characters in this window */
    char c;

    /* set the characters to trim */
    for(i=0; i<TRIMCHARSET; i++) 	/* set them all to be bad initially */
	gang[i] = ajTrue;

    if(isnuc)
    {
	/* normal bases and gap characters are good guys */
	trimseq_parole(gang, "acgtu.-~ ");
	if(!strict)
	    /* so are ambiguity codes if we are not strict */
	    trimseq_parole(gang, "mrwsykvhdb");
    }
    else
    {
	/*
	** protein
	** normal residues and gap characters are good guys
	*/
	trimseq_parole(gang, "arndcqeghilkmfpstwyv.-~ ");
	if(!strict)
	    /* so are ambiguity codes if not strict */
	    trimseq_parole(gang, "bz");

	if(star)
	    /* so is an asterisk if needed */
	    trimseq_parole(gang, "*");
    }

    /* start loop - see direction */
    if(sense)
    {
	a = 0;
	z = ajSeqGetLen(seq) - window;
	inc = 1;
	leroy_brown = -1;
	suspect = -1;
    }
    else
    {
	a = ajSeqGetLen(seq)-1;
	z = window;
	inc = -1;
	leroy_brown = ajSeqGetLen(seq);
	suspect = ajSeqGetLen(seq);
    }

    /*
    **   do an initial trim of contiguous runs of bad characters from the ends
    **   Always trim gaps from the end
    **/
    for(; a != z; a += inc)
    {
	c = (ajSeqGetSeqC(seq))[a];
	if(gang[(ajint)c] || c == '.' || c == '-' || c == '~' || c == ' ')
	    /* trim if bad character or a gap character at the end */
	    leroy_brown = a;		/* want to trim down to here */
	else
	    break;
    }

    /* do the window trim of the remainder of the sequence */
    for(; a != z; a += inc)
    {
	/* look in the window */
	for(count = 0, look = 0; look < window && look > -window; look += inc)
	{
	    c = (ajSeqGetSeqC(seq))[a+look];
	    if(gang[(ajint)c])
	    {
		/* count the bad characters */
		count++;
		/* remember the last bad character position in this window */
		suspect = a+look;
	    }
	}

	/* what is the percentage of bad characters in this window */
	pc = (float)100.0 * (float)count/(float)window;
	/* Need to trim this window? */
	if(pc < percent)
	    break;

	if(sense)
	{
	    if(suspect > leroy_brown)
		leroy_brown = suspect;
	}
	else
	    if(suspect < leroy_brown)
		leroy_brown = suspect;
    }

    /*
    **  do a final tidy up of gap characters left at the new end of the
    **  sequence
    **  Always trim gaps from the end
    */
    for(a = leroy_brown+inc; a != z; a += inc)
    {
	c = (ajSeqGetSeqC(seq))[a];
	if(c == '.' || c == '-' || c == '~' || c == ' ')
	    /* trim if we have a gap character at the end */
	    leroy_brown = a;		/* want to trim down to here */
	else
	    break;
    }

    return leroy_brown;
}




/* @funcstatic trimseq_parole *************************************************
**
**  sets the upper and lowercase characters in the array 'gang' to be ajFalse
**
** @param [w] gang [AjBool*] array of flags for whether a character
**                           is required or not
** @param [r] good_guys [const char*] string of chars that are required
** @@
******************************************************************************/

static void trimseq_parole(AjBool *gang, const char *good_guys)
{
    ajint i;

    for(i=0; good_guys[i]; i++)
    {
	gang[tolower((ajint) good_guys[i])] = ajFalse;
	gang[toupper((ajint) good_guys[i])] = ajFalse;
    }

    return;
}

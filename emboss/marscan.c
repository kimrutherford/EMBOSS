/* @source marscan application
**
** Finds MAR/SAR sites in nucleic sequences
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
#include "stdlib.h"




static AjBool marscan_getpos(AjPList l, ajint *thisprev, ajint otherprev,
			     AjBool *stored_match, ajint *stored_dist,
			     ajint *stored_8_pos, ajint *stored_16_pos,
			     ajint *stored_lastpos, AjPFeattable *tab,
			     AjBool this_is_8,
			     ajint *end_of_last_output_match);

static void marscan_stepdown(AjPList l16, AjPList l8, AjPFeattable *tab);

static void marscan_output_stored_match(AjBool stored_match,
					ajint s8, ajint s16,
					AjPFeattable *tab);

/*
** the maximum distance between a length 16 pattern and a length 8
** pattern in a MRS
*/
#define MAXDIST 200




/* @prog marscan **************************************************************
**
** Finds MAR/SAR sites in nucleic sequences
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPSeq seq;
    AjPFeattable tab = NULL;
    AjPReport report = NULL;

    AjPStr pattern16;
    AjPStr opattern16 = NULL;
    AjBool amino16    = ajFalse;
    AjBool carboxyl16 = ajFalse;
    ajuint  type16 = 0;
    ajuint  m16    = 0;
    ajuint  plen16 = 0;
    ajint  *buf16 = NULL;

    EmbOPatBYPNode off16[AJALPHA];

    ajuint *sotable16 = NULL;
    ajuint solimit16  = 0;
    AjPStr regexp16   = NULL;
    ajuint  **skipm16  = NULL;
    ajuint  mismatch16 = 1;		/* allow a single mismatch */
    AjPList l16;
    ajuint  hits16  = 0;
    const void   *tidy16 = NULL;

    AjPStr pattern16rev;
    AjPStr opattern16rev = NULL;
    AjBool amino16rev    = ajFalse;
    AjBool carboxyl16rev = ajFalse;
    ajuint  type16rev = 0;
    ajuint  m16rev    = 0;
    ajuint  plen16rev = 0;
    ajint  *buf16rev = NULL;

    EmbOPatBYPNode off16rev[AJALPHA];

    ajuint   *sotable16rev = NULL;
    ajuint   solimit16rev  = 0;
    AjPStr   regexp16rev   = NULL;
    ajuint    **skipm16rev  = NULL;
    ajuint    mismatch16rev = 1;		/* allow a single mismatch */
    AjPList  l16rev;
    ajuint    hits16rev  = 0;
    const void     *tidy16rev = NULL;

    AjPStr pattern8;
    AjPStr opattern8 = NULL;
    AjBool amino8    = ajFalse;
    AjBool carboxyl8 = ajFalse;
    ajuint  type8 = 0;
    ajuint  m8    = 0;
    ajuint  plen8 = 0;
    ajint  *buf8 = NULL;

    EmbOPatBYPNode off8[AJALPHA];

    ajuint   *sotable8 = NULL;
    ajuint   solimit8  = 0;
    AjPStr   regexp8   = NULL;
    ajuint    **skipm8  = NULL;
    ajuint    mismatch8 = 0;
    AjPList  l8;
    ajuint    hits8  = 0;
    const void     *tidy8 = NULL;

    AjPStr pattern8rev;
    AjPStr opattern8rev = NULL;
    AjBool amino8rev    = ajFalse;
    AjBool carboxyl8rev = ajFalse;
    ajuint  type8rev = 0;
    ajuint  m8rev    = 0;
    ajuint  plen8rev = 0;
    ajint  *buf8rev = NULL;

    EmbOPatBYPNode off8rev[AJALPHA];

    ajuint   *sotable8rev = NULL;
    ajuint   solimit8rev  = 0;
    AjPStr   regexp8rev   = NULL;
    ajuint    **skipm8rev  = NULL;
    ajuint    mismatch8rev = 0;
    AjPList  l8rev;
    ajuint    hits8rev  = 0;
    const void     *tidy8rev = NULL;

    AjPStr seqname = NULL;
    AjPStr text = NULL;

    ajuint    i;
    ajint    begin;
    ajint    end;

    EmbPMatMatch aptr = NULL;


    embInit("marscan", argc, argv);

    seqall = ajAcdGetSeqall("sequence");
    report = ajAcdGetReport("outfile");


    pattern16    = ajStrNewC("awwrtaannwwgnnnc");
    pattern16rev = ajStrNewC("gnnncwwnnttaywwt");
    pattern8     = ajStrNewC("aataayaa");
    pattern8rev  = ajStrNewC("ttrttatt");

    seqname       = ajStrNew();
    opattern16    = ajStrNew();
    opattern16rev = ajStrNew();
    opattern8     = ajStrNew();
    opattern8rev  = ajStrNew();


    /* Copy original patterns for regexps */
    ajStrAssignS(&opattern16, pattern16);
    ajStrAssignS(&opattern16rev, pattern16rev);
    ajStrAssignS(&opattern8, pattern8);
    ajStrAssignS(&opattern8rev, pattern8rev);

    if(!(type16=embPatGetType(opattern16,&pattern16,
			      mismatch16, 0, &m16, &amino16,
			      &carboxyl16)))
	ajFatal("Illegal pattern");

    embPatCompile(type16, pattern16, &plen16,
		  &buf16, off16, &sotable16, &solimit16,
		  &m16, &regexp16, &skipm16, mismatch16);

    if(!(type16rev=embPatGetType(opattern16rev, &pattern16rev,
				 mismatch16rev, 0, &m16rev,
				 &amino16rev, &carboxyl16rev)))
	ajFatal("Illegal pattern");

    embPatCompile(type16rev, pattern16rev, &plen16rev,
		  &buf16rev, off16rev, &sotable16rev, &solimit16rev,
		  &m16rev, &regexp16rev, &skipm16rev, mismatch16rev);

    if(!(type8=embPatGetType(opattern8, &pattern8, mismatch8, 0, &m8, &amino8,
			     &carboxyl8)))
	ajFatal("Illegal pattern");

    embPatCompile(type8, pattern8, &plen8,
		  &buf8, off8, &sotable8, &solimit8,
		  &m8, &regexp8, &skipm8, mismatch8);

    if(!(type8rev=embPatGetType(opattern8rev, &pattern8rev,
				mismatch8rev, 0, &m8rev,
				&amino8rev, &carboxyl8rev)))
	ajFatal("Illegal pattern");

    embPatCompile(type8rev, pattern8rev, &plen8rev,
		  &buf8rev, off8rev, &sotable8rev, &solimit8rev,
		  &m8rev, &regexp8rev, &skipm8rev, mismatch8rev);


    text = ajStrNew();


    while(ajSeqallNext(seqall, &seq))
    {
	l16    = ajListNew();
	l16rev = ajListNew();
	l8     = ajListNew();
	l8rev  = ajListNew();

	ajStrAssignC(&seqname, ajSeqGetNameC(seq));
	begin = ajSeqallGetseqBegin(seqall);
	end   = ajSeqallGetseqEnd(seqall);
	ajStrAssignSubC(&text, ajSeqGetSeqC(seq), begin-1, end-1);
	ajStrFmtUpper(&text);

	embPatFuzzSearch(type16, begin, pattern16, seqname,
			 text, l16, plen16, mismatch16, amino16, carboxyl16,
			 buf16, off16, sotable16, solimit16, regexp16,
			 skipm16, &hits16, m16, &tidy16);

	embPatFuzzSearch(type16rev, begin, pattern16rev,
			 seqname, text, l16rev, plen16rev, mismatch16rev,
			 amino16rev, carboxyl16rev, buf16rev, off16rev,
			 sotable16rev, solimit16rev, regexp16rev, skipm16rev,
			 &hits16rev, m16rev, &tidy16rev);

	embPatFuzzSearch(type8, begin, pattern8, seqname, text,
			 l8, plen8, mismatch8, amino8, carboxyl8, buf8, off8,
			 sotable8, solimit8, regexp8, skipm8, &hits8, m8,
			 &tidy8);

	embPatFuzzSearch(type8rev, begin, pattern8rev,
			 seqname, text, l8rev, plen8rev, mismatch8rev,
			 amino8rev, carboxyl8rev, buf8rev, off8rev,
			 sotable8rev, solimit8rev, regexp8rev, skipm8rev,
			 &hits8rev, m8rev, &tidy8rev);
	ajDebug("Marscan '%S' hits %d:%d %d:%d\n", seqname,
		hits16, hits16rev,
		hits8, hits8rev);

	/* initialise the output feature table */
	tab = ajFeattableNewDna(seqname);

	/*
	**  append reverse lists to forward lists and sort them by match
	**  position
	*/

	if(hits8 || hits8rev)
	{
	    ajListPushlist(l8, &l8rev);
	    ajListSort(l8, embPatRestrictStartCompare);
	}
	if((hits16 || hits16rev))
	{
	    ajListPushlist(l16, &l16rev);
	    ajListSort(l16, embPatRestrictStartCompare);
	}


	/*
	**  find pairs of hits within the required distance and output
	**  the results
	*/
	marscan_stepdown(l16, l8, &tab);

	ajDebug("Marscan reportwrite '%S'\n", seqname);
	/* write features and tidy up */
	ajReportWrite(report, tab, seq);
	ajFeattableDel(&tab);

        while(ajListPop(l16,(void **)&aptr))
            embMatMatchDel(&aptr);

        while(ajListPop(l8,(void **)&aptr))
            embMatMatchDel(&aptr);

	/*
	**  tidy up - (l8rev and l16rev have already been deleted in
	**  ajListPushList)
	**  but not if the routine was never called.
	*/
	ajListFree(&l16);
	ajListFree(&l8);

	ajListFree(&l16rev);
	ajListFree(&l8rev);

    }

    if(skipm16)
    {
	for(i=0;i<m16;++i)
	    AJFREE(skipm16[i]);
	AJFREE(skipm16);
    }

    if(skipm16rev)
    {
	for(i=0;i<m16rev;++i)
	    AJFREE(skipm16rev[i]);
	AJFREE(skipm16rev);
    }

    if(skipm8)
    {
	for(i=0;i<m8;++i)
	    AJFREE(skipm8[i]);
	AJFREE(skipm8);
    }

    if(skipm8rev)
    {
	for(i=0;i<m8rev;++i)
	    AJFREE(skipm8rev[i]);
	AJFREE(skipm8rev);
    }

    AJFREE(sotable16);
    AJFREE(sotable16rev);
    AJFREE(sotable8);
    AJFREE(sotable8rev);
 
/*
    if(tidy16)
	AJFREE(tidy16);

    if(tidy16rev)
	AJFREE(tidy16rev);

    if(tidy8)
	AJFREE(tidy8);

    if(tidy8rev)
	AJFREE(tidy8rev);
*/

    ajStrDel(&pattern16);
    ajStrDel(&pattern16rev);
    ajStrDel(&pattern8);
    ajStrDel(&pattern8rev);

    ajStrDel(&opattern16);
    ajStrDel(&opattern16rev);
    ajStrDel(&opattern8);
    ajStrDel(&opattern8rev);

    ajStrDel(&seqname);
    ajSeqDel(&seq);
    ajSeqallDel(&seqall);

    ajReportClose(report);
    ajReportDel(&report);

    ajStrDel(&text);

    ajStrDel(&seqname);

    embExit();

    return 0;
}




/* @funcstatic marscan_stepdown ***********************************************
**
** steps down the two lists of matches looking for hits within the required
** maximum distance
**
** @param [u] l16 [AjPList] List of length 16 hits (both forward and reverse)
** @param [u] l8 [AjPList] List of length 8 hits (both forward and reverse)
** @param [u] tab [AjPFeattable*] feature table
** @return [void]
** @@
******************************************************************************/

static void marscan_stepdown(AjPList l16, AjPList l8, AjPFeattable *tab)
{
    ajint prev16 = -1;	/* haven't got a stored position for length 16 */
    ajint prev8  = -1;	/* haven'tgot a stored position for length 8  */


    /* flag ajtrue if have stored match */
    AjBool stored_match = ajFalse;

    /* distance between the patterns in the stored match */
    ajint stored_dist = 0;

    /* position of 16 pattern match in stored match */
    ajint stored_16_pos = 0;

    /* position of 8 pattern match in stored match */
    ajint stored_8_pos = 0;

    /* position of end of second pattern in stored match */
    ajint stored_lastpos = 0;

    /* flag for empty list of length 16 pattern matches*/
    AjBool notend16 = ajTrue;

    /* flag for empty list of length 8 pattern matches*/
    AjBool notend8 = ajTrue;

    /* used to prevent clusters of MRS within MAXDIST of each other */
    ajint end_of_last_output_match = -(MAXDIST+1);

    while(notend16 || notend8)
    {
	notend16 = marscan_getpos(l16, &prev16, prev8, &stored_match,
				  &stored_dist, &stored_8_pos, &stored_16_pos,
				  &stored_lastpos, tab, ajFalse,
				  &end_of_last_output_match);

	/*
	**  if the list of 8 pattern matches is empty and the 16's have
	**  gone past the last 8 pattern match, stop searching
	*/
	if(prev16 >= prev8 && !notend8)
	    notend16 = ajFalse;

	notend8 = marscan_getpos(l8, &prev8, prev16, &stored_match,
				 &stored_dist, &stored_8_pos, &stored_16_pos,
				 &stored_lastpos, tab, ajTrue,
				 &end_of_last_output_match);

	/*
	**  if the list of 16 pattern matches is empty and the 8's have
	**  gone past the last 16 pattern match, stop searching
	*/
	if(prev8 >= prev16 && !notend16)
	    notend8 = ajFalse;
    }

    /*
    **  Both lists are empty. Output any remaining stored match
    **  only output if this match is further than MAXDIST from the last hit
    **  output to prevent lots of outputs for a cluster of MRS's
    */

    if(stored_8_pos - end_of_last_output_match > MAXDIST ||
	stored_16_pos - end_of_last_output_match > MAXDIST)
	marscan_output_stored_match(stored_match, stored_8_pos,
				    stored_16_pos, tab);

    return;
}




/* @funcstatic marscan_getpos *************************************************
**
** gets the next position from a list and checks to see if we have a match
** within MAXDIST of the last match in the other list
**
** @param [u] l [AjPList] the list of matching positions
** @param [u] thisprev [ajint *] pointer to last stored position of
**                                this pattern
** @param [r] otherprev [ajint] last stored position of the other pattern
** @param [u] stored_match [AjBool *] flag set to ajtrue if have stored match
** @param [u] stored_dist [ajint *] distance between the patterns in
**                                   the stored match
** @param [u] stored_8_pos [ajint *] position of this pattern match in
**                                    stored match
** @param [u] stored_16_pos [ajint *] position of 8 pattern match in
**                                     stored match
** @param [u] stored_lastpos [ajint *] position of end of second pattern
**                                      in stored match
** @param [u] tab [AjPFeattable*] feature table
** @param [r] this_is_8 [AjBool] ajTrue is 'thisprev' refers to the
**                               length 8 pattern
** @param [u] end_of_last_output_match [ajint *] end of the last output match
** @return [AjBool] False if the list is empty
** @@
******************************************************************************/

static AjBool marscan_getpos(AjPList l, ajint *thisprev, ajint otherprev,
			      AjBool *stored_match, ajint *stored_dist,
			      ajint *stored_8_pos, ajint *stored_16_pos,
			      ajint *stored_lastpos,
			      AjPFeattable *tab, AjBool this_is_8,
			      ajint *end_of_last_output_match)
{

    EmbPMatMatch m;
    ajint dist;		/* distance between the two patterns */
    ajint firstpos;	/* first position of the first of the two patterns */
    ajint lastpos;	/* last position of the second of the two patterns */
    ajint s8;
    ajint s16;
    ajint e8;
    ajint e16;		/* start and end positions */

    while(*thisprev <= otherprev)
    {
	/* if the list is empty, return ajFalse */
	if(!ajListPop(l, (void **)&m))
	    return ajFalse;

	/* get position of next list element and store it */
	*thisprev = m->start;
	embMatMatchDel(&m);

	/*
	**  get the start and end positions of the 8bp and 16bp patterns
	**  and get the end position of the MRS = second pattern + length
	**  of second pattern -1
	*/
	if(this_is_8)
	{
	    s8  = *thisprev;
	    s16 = otherprev;
	}
	else
	{
	    s8  = otherprev;
	    s16 = *thisprev;
	}

	e8  = s8+7;
	e16 = s16+15;

	/* get last position of the two patterns */
	if(e8>e16)
	    lastpos = e8;
	else
	    lastpos = e16;

	/* get distance from end of first pattern to start of second */
	if(e8 < s16)
	    dist = s16-e8;
	else if(e16 < s8)
	    dist = s8-e16;
	else				/* overlap */
	    dist = 0;


	/* the first position of the two patterns */
	if(s8>s16)
	    firstpos = s16;
	else
	    firstpos = s8;


	/* otherprev is -1 if it hasn't got a position stored in it yet */
	if(otherprev == -1)
	{
	    dist = MAXDIST + 1;
	    firstpos = MAXDIST + 1;
	}

	/* if they are overlapping, maybe still have a negative length */
	if(dist < 0)
	    ajFatal("Have a negative distance!\n");


	/* if dist to other stored pos is within range */
	if(dist <= MAXDIST)
	{
	    /* if there's a stored match output it */
	    if(*stored_match)
	    {
		if(dist < *stored_dist)
		{
		    /*
		    **  store new match
		    **  only output if this match is further than MAXDIST
		    **  from the last hit output to prevent lots of outputs
		    **  for a cluster of MRS's
		    */

		    if(firstpos - *end_of_last_output_match > MAXDIST)
		    {
			*stored_match   = ajTrue;
			*stored_dist    = dist;
			*stored_8_pos   = s8;
			*stored_16_pos  = s16;
			*stored_lastpos = lastpos;
		    }
		}
		else
		{
		    marscan_output_stored_match(*stored_match,
						*stored_8_pos, *stored_16_pos,
						tab);
		    *stored_match = ajFalse;
		    *end_of_last_output_match = *stored_lastpos;
		}
	    }
	    else
	    {
		/*
		 *  store new match
		 *  only output if this match is further than MAXDIST from
		 *  the last hit output to prevent lots of outputs for a
		 *  cluster of MRS's
		 */
		if(firstpos - *end_of_last_output_match > MAXDIST)
		{
		    /*
		     *  printf("STORE for OUTPUT dist = %d\n", firstpos -
		     *  *end_of_last_output_match);
		     */
		    *stored_match = ajTrue;
		    *stored_dist = dist;
		    *stored_8_pos = s8;
		    *stored_16_pos = s16;
		    *stored_lastpos = lastpos;
		}
	    }
	}
	else
	{
	    if(*stored_match)
	    {
		marscan_output_stored_match(*stored_match,
					    *stored_8_pos, *stored_16_pos,
					    tab);
		*stored_match = ajFalse;
		*end_of_last_output_match = *stored_lastpos;
	    }
	}
    }

    return ajTrue;
}




/* @funcstatic marscan_output_stored_match ************************************
**
** Outputs the results of finding a match of the two patterns
**
** @param [r] stored_match [AjBool] flag set to ajtrue if have stored match
** @param [r] s8 [ajint]  position of 8bp pattern match in stored match
** @param [r] s16 [ajint] position of 16bp pattern match in stored match
** @param [u] tab [AjPFeattable*] feature table
** @return [void]
** @@
******************************************************************************/

static void marscan_output_stored_match(AjBool stored_match,
					ajint s8, ajint s16, AjPFeattable *tab)
{
    /*
    **  strand is set to forward because the MAR/SAR recognition
    **  signature (MRS) is not dependent on the strand(s) it is on
    */
    char strand = '+';

    ajint frame = 0;
    float score = 0.0;
    AjPStr source = NULL;
    AjPStr type   = NULL;

    AjPFeature feature;
    static AjPStr tmp = NULL;
    ajint start;
    ajint end;
    ajint e8;
    ajint e16;	/* end positions of the 8bp and 16 bp pattern matches */

    if(!stored_match)
	return;

    if(!type)
    {
      ajStrAssignC(&source,"marscan");
      ajStrAssignC(&type,"misc_signal");
    }

    /*
    **  get the start and end positions of the 8bp and 16bp patterns and get
    **  the end position of the MRS = second pattern + length of second
    **  pattern -1
    */
    e8  = s8+7;
    e16 = s16+15;
    if(s8 < s16)
	start = s8;
    else
	start = s16;

    if(e8>e16)
	end = e8;
    else
	end = e16;


    feature = ajFeatNew(*tab, source, type, start, end,
			score, strand, frame) ;


    ajFmtPrintS(&tmp, "*type MAR/SAR recognition site (MRS)");
    ajFeatTagAdd(feature, NULL, tmp);

    ajFmtPrintS(&tmp, "*start8bp %d", s8);
    ajFeatTagAdd(feature, NULL, tmp);

    ajFmtPrintS(&tmp, "*end8bp %d", e8);
    ajFeatTagAdd(feature, NULL, tmp);

    ajFmtPrintS(&tmp, "*start16bp %d", s16);
    ajFeatTagAdd(feature, NULL, tmp);

    ajFmtPrintS(&tmp, "*end16bp %d", e16);
    ajFeatTagAdd(feature, NULL, tmp);

    ajStrDel(&source);
    ajStrDel(&type);
    ajStrDel(&tmp);

    return;
}

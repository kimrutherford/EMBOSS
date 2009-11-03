/* @source oddcomp application
**
** Identifies sequences with a region with a high composition of specific
** words
**
** @author Copyright (C) David Martin (david.martin@biotek.uio.no) based on
** compseq by Gary Williams
** @modified 29 June 2006    Jon Ison   All mods marked up with JISON
** @modified 8 November 1999 David Martin.
** @@
**
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




static ajint oddcomp_readexpfreq(AjPTable *exptable, AjPFile compdata,
				 ajint *size);




/* @prog oddcomp **************************************************************
**
** Finds protein sequence regions with a biased composition
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPSeqall seqall      = NULL;
    AjPSeq    seq         = NULL;
    ajint     word        = 2;
    AjPFile   outfile     = NULL;
    AjPFile   compdata    = NULL;
    ajuint     window      = 0;
    ajint     pos         = 0;
    const     char *s     = NULL;
    ajlong    result      = 0;
    ajlong   *bigarray    = NULL;
    ajlong   *windowbuffer= NULL;	/* ring buffer for sliding window */
    ajulong   no_elements = 0;
    AjBool    first_time_round = ajTrue;
    AjBool    ignorebz    = ajTrue;
    ajulong   count       = 0;
    AjPStr    dispseq     = NULL;
    AjPStr    ajb         = NULL;
    ajulong   total       = 0;
    ajulong   other       = 0;
    AjBool    otherflag; 
    AjBool    seqisprot   = ajTrue;
    ajint     increment   = 1;
    ajint     ringsize    = 0;
    ajint     lastringsize= 0;          /* JISON */
    ajlong    steps       = 0;
    AjPTable  exptable    = NULL;	/* table of expected frequencies */
    ajlong    exp_freq    = 0;
    AjBool    fullwindow  = ajFalse;    /* JISON */
    ajulong   tul;

    embInit("oddcomp", argc, argv);

    seqall     = ajAcdGetSeqall("sequence");
    window     = ajAcdGetInt("window");
    outfile    = ajAcdGetOutfile("outfile");
    compdata   = ajAcdGetInfile("infile");
    ignorebz   = ajAcdGetBoolean("ignorebz");
    fullwindow = ajAcdGetToggle("fullwindow");

    /* Output some documentation to the results file */
    ajFmtPrintF(outfile, "#\n# Output from 'oddcomp'\n#\n");
    ajFmtPrintF(outfile, "# The Expected frequencies are taken from the "
		"file: %s\n", ajFileGetNameC(compdata));

    /* read the required frequencies into a table */
    oddcomp_readexpfreq(&exptable, compdata, &word);

    /* number of overlapping words in a window */
    ringsize = window - word + 1;

    /* more notes */
    ajFmtPrintF(outfile, "#\n#\tWord size: %d\n",word);



    while(ajSeqallNext(seqall, &seq))
    {
	/* JISON entire 'if' block below */
	if(fullwindow)
	{
	    window   = ajSeqGetLen(seq);    
	    ringsize = window - word + 1;
	}	
	
	steps = 0;
	seqisprot = ajSeqIsProt(seq);
	ajDebug("Reading sequence '%S'\n", ajSeqGetNameS(seq));

	/* not interested in nucleotide sequences so ignore any that get in */
	if(!seqisprot)
	{
	    lastringsize = ringsize;   /* JISON */
	    continue;
	}
	
	/* ignore sequences shorter than the window of interest */
	if(ajSeqGetLen(seq)<window)
	{
	    lastringsize = ringsize;   /* JISON */
	    continue;
	}
	

	/*  first of all need to make a store for the results */
	if(first_time_round)
	{

	    if(!embNmerGetNoElements(&no_elements, word, ajFalse, ignorebz))
		ajFatal("The word size is too large for the data "
				"structure available.");

	    AJCNEW(bigarray, no_elements);
	    AJCNEW(windowbuffer, ringsize);  /* create ring buffer */

	    first_time_round = ajFalse;
	}
	/* JISON ... entire 'else' block below */
	else
	{
	    if(ringsize > lastringsize)
	    {
		AJFREE(windowbuffer);
		AJCNEW(windowbuffer, ringsize);
	    }
	}
	

	ajSeqFmtUpper(seq);
	s = ajSeqGetSeqC(seq);

	/*
	** initialise the results buffer for this sequence.
	** each word will require a certain number of counts to get to the
	** necessary frequency. Set the number of counts to negative this so
	** it is only necessary to check for counts >0. Also set the steps
	** variable to go the number of steps needed before a new check needs
	** to be made (count minimum number of words required before the state
	** can change.)
	*/
	for(count=0; count< no_elements;count++)
	{
	    ajStrSetClear(&dispseq);
	    /*
	    **  need to clear the string as embNmerInt2Prot will prepend
	    **  to it
	    */
	    embNmerInt2prot(&dispseq, word, count, ignorebz);
	    ajb=ajTableFetch(exptable, dispseq);

	    if(ajb)
		ajStrToLong( ajb, &exp_freq);
	    else
		exp_freq = 0;

	    if(exp_freq>0)
		/*
		**  set bigarray count to negative the count needed to
		**  exceed the frequency
		*/
		bigarray[count] = - exp_freq;
	    else
		bigarray[count] = 0;
	    ajDebug("loop a count:%Ld no_elements:%Ld bigarray[%Ld]:%Ld dispseq:%S ajb:%S\n",
		    count, no_elements, count, bigarray[count], dispseq, ajb);
	}


	/*
	**  Start at the first position, and fill the ring buffer by
	**  sliding one step at a time.
	*/
	for(pos=1;pos<= ringsize; pos += increment)
	{
	    ajDebug("loop b pos:%d ringsize:%d increment:%d\n",
		    pos, ringsize, increment);

	    tul = embNmerProt2int(s, word, pos-1, &otherflag,ignorebz);
	    result = (ajlong) tul;

	    if(otherflag)
		windowbuffer[pos%ringsize] = -1;
	    else
	    {
		windowbuffer[pos%ringsize] = result;
		bigarray[result]++;
	    }
	}

	/*
	**  ringbuffer now full. calculate the number of steps to get a
	**  change in state by working out the sum of negative values
	*/

	for(count=0; count<no_elements;count++)
	{
	    if(bigarray[count]<0)
		steps -= bigarray[count];
	    ajDebug("loop c count:%Ld no_elements:%Ld bigarray[%Ld]:%Ld steps:%Ld\n",
		    count, no_elements, count,
		    bigarray[count], steps);

	}

	ajDebug("ringsize:%d seqlen:%d word:%d end:%d increment:%d\n",
		ringsize, ajSeqGetLen(seq), word, ajSeqGetLen(seq)-word, increment);
	/*
	 **  need to check to see whether or not we have the
	 **  necessary composition?
	 */
	if(steps==0)
	{
	    for(count=0; count<no_elements;count++)
		if(bigarray[count]<0)
		    steps -= bigarray[count];
	    
	    /* now check to see if the composition is a hit. */
	    if(steps==0)
	    {
		ajFmtPrintF(outfile, "\t%s\n", ajSeqGetNameC(seq));
		total++;
		lastringsize = ringsize;   /* JISON */
		continue;
	    }
	}
	else {
	    steps--;

	    for(pos=ringsize+1;
		pos <= (ajint)ajSeqGetLen(seq)-word+1;
		pos += increment)
	    {
		ajDebug("loop d pos:%d steps:%Ld\n",
			pos, steps);

		for(count=0; count<no_elements;count++)
		    if(bigarray[count]<0)
			ajDebug("bigarray[%Ld]:%Ld\n",
				count, bigarray[count]);

		tul = embNmerProt2int(s, word, pos-1, &otherflag,ignorebz);
		result = (ajlong) tul;

		/* uncount the word just leaving the window if it wasn't 'other'*/
		if(windowbuffer[pos%ringsize] >=0)
		{
		    bigarray[windowbuffer[pos%ringsize]]--;
		    count = windowbuffer[pos%ringsize];
		    ajDebug("pos:%d bigarray[%Ld] %Ld\n",
			    pos, count, bigarray[count]);
		}
		else
		    other--;

		/* count this word */


		if(!otherflag)
		{
		    windowbuffer[pos%ringsize] = result;
		    bigarray[result]++;
		    ajDebug("result:%Ld bigarray[%Ld] %Ld\n",
			    result, result, bigarray[result]);
		}
		else
		{
		    windowbuffer[pos%ringsize] = -1;
		    other++;
		}
		/*
		 **  need to check to see whether or not we have the
		 **  necessary composition?
		 */
		if(steps==0)
		{
		    for(count=0; count<no_elements;count++)
			if(bigarray[count]<0)
			    steps -= bigarray[count];

		    /* now check to see if the composition is a hit. */
		    if(steps==0)
		    {
			ajFmtPrintF(outfile, "\t%s\n", ajSeqGetNameC(seq));
			total++;
			break;
		    }
		}
		else
		    steps--;
	    }
	}
	lastringsize = ringsize;   /* JISON */
    }
    
    ajFmtPrintF(outfile, "\n#\tEND\t#\n");

    ajFileClose(&outfile);

    AJFREE(bigarray);

    ajTablestrFree(&exptable);

    ajSeqallDel(&seqall);
    ajSeqDel(&seq);
    ajFileClose(&compdata);

    ajStrDel(&dispseq);
    AJFREE(windowbuffer);

    embExit();

    return 0;
}





/* @funcstatic oddcomp_readexpfreq ********************************************
**
** Undocumented.
**
** @param [w] exptable [AjPTable*] Table created
** @param [u] compdata [AjPFile] Undocumented
** @param [w] size [ajint*] Undocumented
** @return [ajint] Undocumented
** @@
******************************************************************************/

static ajint oddcomp_readexpfreq(AjPTable *exptable, AjPFile compdata,
				 ajint *size)
{
    AjPStr line = NULL;
    char whiteSpace[] = " \t\n\r";
    AjPStrTok tokens;
    AjPStr sizestr = NULL;
    ajint thissize;
    AjPStr key;
    AjPStr value;

    /* initialise the hash table - use case-insensitive comparison */

    *exptable = ajTablestrNewCaseLen(350);



    /* read the file */
    while(ajReadlineTrim(compdata, &line))
    {
	/* skip comment and blank lines */
	if(!ajStrFindC(line, "#"))
	    continue;

	if(!ajStrGetLen(line))
	    continue;

	/* look for the word size */
	if(!ajStrFindC(line, "Word size"))
	{
	    ajStrAssignSubS(&sizestr, line, 10, ajStrGetLen(line));
	    ajStrTrimWhite(&sizestr);
	    ajStrToInt(sizestr, &thissize);

	    *size = thissize;
	    break;
	}
	else
	    ajFatal("The 'Word size' line was not found, "
			    "instead found:\n%S\n",line);
    }

    /* read the file */
    while(ajReadlineTrim(compdata, &line))
    {
	/* skip comment and blank lines */
	if(!ajStrFindC(line, "#"))
	    continue;

	if(!ajStrGetLen(line))
	    continue;

	/*
	**  look for the total number of counts - anything after this is
	**  the data
	*/
	if(!ajStrFindC(line, "Total"))
	    break;
    }

    /* read in the observed frequencies as a string */
    while(ajReadlineTrim(compdata, &line))
    {
	/* skip comment and blank lines */
	if(!ajStrFindC(line, "#"))
	    continue;

	if(!ajStrGetLen(line))
	    continue;

	tokens = ajStrTokenNewC(line, whiteSpace);

	/* get the word as the key */
	key = ajStrNew();
	ajStrTokenNextParse( &tokens, &key);

	/*
	**  get the observed count as the value - use this as the
	**  expected frequency
	*/
	value = ajStrNew();
	ajStrTokenNextParse( &tokens, &value);
	ajTablePut( *exptable, key, value);
	ajStrTokenDel( &tokens);
    }

    /* tidy up */
    ajStrDel(&line);
    ajStrDel(&sizestr);

    return 0;
}

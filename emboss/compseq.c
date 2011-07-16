/* @source compseq application
**
** Counts the composition of dimer/trimer/etc words in a sequence
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




static void compseq_readexpfreq(AjPTable *exptable, AjPFile infile,
				 ajint size);
static void compseq_makebigarray(ajulong no_elements, ajulong **bigarray);
static double compseq_getexpfreqnuc(const AjPStr dispseq, ajint word,
				    const ajulong *calcfreq_array,
				    ajulong calcfreq_total);
static double compseq_getexpfreqprot(const AjPStr dispseq, ajint word,
				     AjBool ignorebz,
				     const ajulong *calcfreq_array,
				     ajulong calcfreq_total);




/* @prog compseq **************************************************************
**
** Counts the composition of dimer/trimer/etc words in a sequence
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPSeqall seqall;
    AjPSeq seq;
    ajuint word;
    AjBool zerocount;
    AjBool ignorebz;
    ajint frame;
    AjPFile outfile;
    AjPFile infile;
    AjBool reverse;
    AjBool calcfreq;

    ajuint pos;
    ajuint endpos;
    const char *s;
    ajulong result;
    ajulong *bigarray;
    ajulong *calcfreq_array;
    ajulong no_elements;
    ajulong no_calcfreq_elements;
    AjBool first_time_round = ajTrue;
    ajulong count;
    AjPStr dispseq = NULL;
    const AjPStr tmpstr = NULL;		/* from table, no need to delete */

    ajulong total = 0;		/* no. of words counted */
    ajulong calcfreq_total = 0; /* no. of single bases/residues counted */
    ajulong other = 0;		/* no. of words containing 'other' bases/residues */
    ajulong calcfreq_other = 0; /* no. of single 'other' bases/residues observed */

    AjBool otherflag;
    AjBool seqisnuc=ajFalse;
    ajint increment;
    ajint count_of_sequence_names = 0;
    AjPTable exptable = NULL;		/* table of expected frequencies */

    /* holds the string of "Other" for looking in the hash table 'exptable' */
    AjPStr strother = NULL;


    /* Don't have a file of expected frequencies yet */
    AjBool have_exp_freq = ajFalse;

    double default_exp_freq;
    double obs_freq = 0.0;
    double exp_freq;
    double obs_exp;

    embInit("compseq", argc, argv);

    seqall    = ajAcdGetSeqall("sequence");
    word      = ajAcdGetInt("word");
    outfile   = ajAcdGetOutfile("outfile");
    infile    = ajAcdGetInfile("infile");
    zerocount = ajAcdGetBoolean("zerocount");
    ignorebz  = ajAcdGetBoolean("ignorebz");
    frame     = ajAcdGetInt("frame");
    reverse   = ajAcdGetBoolean("reverse");
    calcfreq  = ajAcdGetBoolean("calcfreq");

    /* Output some documentation to the results file */
    ajFmtPrintF(outfile, "#\n# Output from 'compseq'\n#\n");

    if(!zerocount)
	ajFmtPrintF(outfile,"# Words with a frequency of zero are "
		    "not reported.\n");


    if(!ignorebz)
	ajFmtPrintF(outfile,"# The amino acid codes 'B' and 'Z' will "
		    "be counted,\n# rather than treated as 'Other'.\n");
    
    
    if(frame)
	ajFmtPrintF(outfile,"# Only words in frame %d will be "
		    "counted.\n", frame);
    
    if(infile == NULL) {
        if (calcfreq)
 	    ajFmtPrintF(outfile,"# The Expected frequencies are "
		      "calculated from the observed single\n"
                      "# base or residue frequencies in these sequences\n");

	else
 	    ajFmtPrintF(outfile,"# The Expected frequencies are "
		      "calculated on the (false) assumption that every\n"
		      "# word has equal frequency.\n");

    }
    else
    {
	ajFmtPrintF(outfile,"# The Expected frequencies are taken "
		    "from the file: %S\n",ajFileGetPrintnameS(infile));
	compseq_readexpfreq(&exptable, infile, word);
	have_exp_freq = ajTrue;
	ajFileClose(&infile);
    }

    ajFmtPrintF(outfile, "#\n");
    ajFmtPrintF(outfile, "# The input sequences are:\n");
    
    
    /*
    **  see if using a sliding window or only looking at a word-sized
    **  single frame
    */
    if(frame)
	increment = word;
    else
	increment = 1;
    
    while(ajSeqallNext(seqall, &seq))
    {
	ajSeqTrim(seq);

	/* note the name of the sequence */
	if(count_of_sequence_names++ < 10)
	    ajFmtPrintF(outfile, "#\t%s\n", ajSeqGetNameC(seq));
	else if(count_of_sequence_names++ == 11)
	    ajFmtPrintF(outfile, "# ... et al.\n");


	/* first of all need to make a store for the results */
	if(first_time_round)
	{
	    seqisnuc = ajSeqIsNuc(seq);
	    if(!embNmerGetNoElements(&no_elements, word, seqisnuc, ignorebz))
		ajFatal("The word size is too large for the data "
			"structure available.");

	    compseq_makebigarray(no_elements, &bigarray);
	    first_time_round = ajFalse;

            if(calcfreq) 
            {
                if(!embNmerGetNoElements(&no_calcfreq_elements, word,
					 seqisnuc, 
                                         ignorebz))
                    ajFatal("The word size is too large for the data "
                            "structure available.");
                    compseq_makebigarray(no_calcfreq_elements,
					 &calcfreq_array);
            }
          
	}

	ajSeqFmtUpper(seq);
	s = ajSeqGetSeqC(seq);

	/*
	**  Start at the first position, or at the frame, if it has been
	**  specified and then look at each word in a sliding window if
	**  no frame is specified, or at each increment of the word-size
	**  if frame is specified. Stop when less than a word-length from
	**  the end of the sequence.
	*/
	if(ajSeqGetLen(seq) > word)
	    endpos = ajSeqGetLen(seq)-word;
	else
	    endpos = 1;
	for(pos=frame; pos <= endpos; pos += increment)
	{
	    if(seqisnuc)
		result = embNmerNuc2int(s, word, pos, &otherflag);
	    else
		result = embNmerProt2int(s, word, pos, &otherflag, ignorebz);


	    /* count this word */
	    if(!otherflag)
		bigarray[result]++;
	    else
		other++;

	    total++;
	}

        if(calcfreq) 
        {
        /* Count the single bases or residues to get the observed frequences */
            for(pos=0; pos < ajSeqGetLen(seq); pos++) 
            {
                if(seqisnuc)
                    result = embNmerNuc2int(s, 1, pos, &otherflag);
                else
                    result = embNmerProt2int(s, 1, pos, &otherflag, ignorebz);
                
                if(!otherflag)
                    calcfreq_array[result]++;
                else
                    calcfreq_other++;

                calcfreq_total++;
            }
        }



	if(seqisnuc && reverse)
	{
	    /* Do it again on the reverse strand */
	    ajSeqReverseForce(seq);
	    s = ajSeqGetSeqC(seq);

	    for(pos=frame; pos <= ajSeqGetLen(seq)-word; pos += increment)
	    {
		result = embNmerNuc2int(s, word, pos, &otherflag);

		/* count this word */
		if(!otherflag)
		    bigarray[result]++;
		else
		    other++;

		total++;
	    }

            if(calcfreq) 
            {
                /* 
                ** Count the complemented single bases to get their
                ** observed frequences 
                */

                for(pos=0; pos < ajSeqGetLen(seq); pos++) 
                {
                    result = embNmerNuc2int(s, 1, pos, &otherflag);
                                    
                    if(!otherflag)
                        calcfreq_array[result]++;
                    else
                        calcfreq_other++;

                    calcfreq_total++;
                }
            }
	}
    }
    
    /* Give the word size used */
    ajFmtPrintF(outfile,"\n\nWord size\t%d\n", word);
    
    /* Now output the Total count */
    ajFmtPrintF(outfile,"Total count\t%Lu\n\n", total);
    
    /* we have now counted the frequency of the words in the sequences */
    ajFmtPrintF(outfile,
		"#\n# Word\tObs Count\tObs Frequency\tExp Frequency\t"
		"Obs/Exp Frequency\n#\n");
    
    /*
    **  If there's no file of expected word frequencies, 
    **  then use the frequencies of single bases or residues to calculate them.
    **  If we are not calculating the frequencies, then make a default
    **  by giving each word an equal frequency.
    */
    default_exp_freq = 1/(double)no_elements;
    
    for(count=0; count<no_elements; count++)
    {
	if(!zerocount && bigarray[count] == 0)
	    continue;

	ajStrSetClear(&dispseq);

	if(seqisnuc)
	    embNmerInt2nuc(&dispseq, word, count);
	else
	    embNmerInt2prot(&dispseq, word, count, ignorebz);

        /* the default value will cover all contingencies if things fail */
	exp_freq = default_exp_freq;

	if(have_exp_freq)
	{
	    if((tmpstr=ajTableFetchS(exptable,dispseq)))
	    {
		ajStrToDouble(tmpstr, &exp_freq);
	    }
	}
	else
	{
            if (calcfreq) 
            {
                /* 
                ** get the expected frequency of this word based on
                ** observed single frequencies 
                */
		if(seqisnuc)
		{
		    exp_freq = compseq_getexpfreqnuc(dispseq, word,
                                             calcfreq_array, calcfreq_total);
                }
		else
		{
		    exp_freq = compseq_getexpfreqprot(dispseq, word, ignorebz,
                                             calcfreq_array, calcfreq_total);
                }
            }
	}
	
        obs_freq = (double)bigarray[count]/(double)total;

	if(exp_freq == 0.0)
	    /* display a big number rather than a divide by zero error */
	    obs_exp = 10000000000.0;
	else
	    obs_exp = obs_freq/exp_freq;

	ajFmtPrintF(outfile, "%S\t%Lu\t\t%.7f\t%.7f\t%.7f\n", dispseq,
		    bigarray[count], obs_freq, exp_freq, obs_exp);
    }

    /* 
    **
    ** now output the Other count 
    **
    */

    /* get the observed Other frequency */
    obs_freq = (double)other/(double)total;

    /* get the expected Other frequency */
    if(have_exp_freq)
    {
	ajStrAssignC(&strother, "Other");
	ajStrToDouble((const AjPStr)ajTableFetchS(exptable, strother),
                      &exp_freq);
    }
    else 
    {
        if (calcfreq) 
            /*
            ** Is it correct to calculate exp_freq using expected single
            ** base 'other' when the words are of size > 1?
            ** I don't think so, because words may contain more than 1
            ** 'other' base, but there isn't an easy way to do this
            ** without storing the sequence of all words with an 'other'
            ** in them.  Maybe do this in a later version?
            **/
            exp_freq = (double)calcfreq_other/(double)calcfreq_total;
    }


    /* get the Other Observed/Expected ratio */
    if((!have_exp_freq && !calcfreq) || exp_freq==0.0)
    {
	exp_freq = 0.0;
	/* display a big number rather than a divide by zero error */
	obs_exp = 10000000000.0;
    }
    else
	obs_exp = obs_freq/exp_freq;
    
    
    ajFmtPrintF(outfile, "\nOther\t%Lu\t\t%.7f\t%.7f\t%.7f\n", other,
		obs_freq, exp_freq, obs_exp);
    
    ajFileClose(&outfile);
    ajSeqallDel(&seqall);
    ajSeqDel(&seq);

    AJFREE(bigarray);
    ajStrDel(&dispseq);
    ajStrDel(&strother);

    if(have_exp_freq)
	ajTablestrFree(&exptable);
 

    embExit();

    return 0;
}




/* @funcstatic compseq_makebigarray *******************************************
**
** Create an array to hold counts of the observed frequencies of words.
**
** @param [r] no_elements [ajulong] Size of array
** @param [w] bigarray [ajulong**] Returned pointer to array
** @return [void] 
** @@
******************************************************************************/

static void compseq_makebigarray(ajulong no_elements, ajulong **bigarray)
{

    ajDebug("makebigarray for %Ld elements\n", no_elements);
    AJCNEW(*bigarray, no_elements);
    ajDebug("makebigarray - after AJCNEW\n");

    return;
}




/* @funcstatic compseq_readexpfreq ********************************************
**
** Read the expected frequencies of words from a file produced by a
** previous run of this program. 
**
** @param [w] exptable [AjPTable*] Table of words and their
**                                 expected frequencies
** @param [u] infile [AjPFile] Input file holding expected frequencies
** @param [r] size [ajint] Size of word expected (used for validation)
** @return [void] 
** @@
******************************************************************************/

static void compseq_readexpfreq(AjPTable *exptable, AjPFile infile,
				 ajint size)
{

    AjPStr line = NULL;
    char whiteSpace[] = " \t\n\r";
    AjPStrTok tokens;
    AjPStr sizestr = NULL;
    ajint thissize;
    AjPStr key;
    AjPStr value;

    /* initialise the hash table - use case-insensitive comparison */
    *exptable = ajTablestrNewCase(350);


    /* read the file */
    while(ajReadlineTrim(infile, &line))
    {
	/* skip comment and blank lines */
	if(!ajStrFindC(line, "#"))
	    continue;

	if(!ajStrGetLen(line))
	    continue;

	/* look for the word size */
	if(!ajStrFindC(line, "Word size"))
	{
	    ajStrAssignSubS(&sizestr, line, 10, ajStrGetLen(line)-1);
	    ajStrTrimWhite(&sizestr);
	    ajStrToInt(sizestr, &thissize);

	    if(size == thissize)
		break;

	    ajFatal("The word size you are counting (%d) is different "
			    "to the word\nsize in the file of expected "
			    "frequencies (%d).", size, thissize);

	}
	else
	    ajFatal("The 'Word size' line was not found, instead "
			    "found:\n%S\n", line);
    }

    /* read the file */
    while(ajReadlineTrim(infile, &line))
    {
	/* skip comment and blank lines */
	if(!ajStrFindC(line, "#"))
	    continue;

	if(!ajStrGetLen(line))
	    continue;

	/*
	**  look for the total number of counts - anything after this
	**  is the data
	*/
	if(!ajStrFindC(line, "Total"))
	    break;
    }

    /* read in the observed frequencies as a string */
    while(ajReadlineTrim(infile, &line))
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
	**  get the observed frequency as the value - we'll use this as
	**  the expected frequency
	*/
	value = ajStrNew();

	/* skip the observed count column */
	ajStrTokenNextParse(&tokens, &value);
	ajStrTokenNextParse(&tokens, &value);

	ajTablePut(*exptable, key, value);
	ajStrTokenDel(&tokens);
    }


    ajStrDel(&line);
    ajStrDel(&sizestr);
    ajStrTokenDel(&tokens);

    return;
}




/* @funcstatic compseq_getexpfreqnuc ******************************************
**
** Using the observed counts of single bases in the sequence this
** calculates the expected frequency of a word of one or more bases
** as the product of the single base frequencies.
**
** @param [r] dispseq [const AjPStr] sequence of word
** @param [r] word [ajint] word length
** @param [r] calcfreq_array [const ajulong *] array of counts of single bases
** @param [r] calcfreq_total [ajulong] number of bases counted
** @return [double] Calculated expected frequency of this word
** @@
******************************************************************************/

static double compseq_getexpfreqnuc(const AjPStr dispseq, ajint word,
	const ajulong *calcfreq_array, ajulong calcfreq_total)
{
    ajint i;
    ajint offset = 0;
    char c;
    double result;
    
    result = 1.0;

    for(i=0; i<word; i++)
    {
        c = ajStrGetPtr(dispseq)[i];

        if(c == 'A')
            offset = 0;
        else if(c == 'C')
            offset = 1;
        else if(c == 'G')
            offset = 2;
        else if(c == 'T')
            offset = 3; 
        else
            ajFatal("A non-ACGT base was found: %c\n", c);

        result *= (double)calcfreq_array[offset]/(double)calcfreq_total;
    }

    return result;

}




/* @funcstatic compseq_getexpfreqprot *****************************************
**
** Using the observed counts of single residues in the sequence this
** calculates the expected frequency of a word of one or more residues
** by simply summing the single residue frequencies.
**
** @param [r] dispseq [const AjPStr] sequence of word
** @param [r] word [ajint] word length
** @param [r] ignorebz [AjBool] True if ignoring B and Z residues
** @param [r] calcfreq_array [const ajulong *] array of counts of single bases
** @param [r] calcfreq_total [ajulong] number of bases counted
** @return [double] Calculated expected frequency of this word
** @@
******************************************************************************/


static double compseq_getexpfreqprot(const AjPStr dispseq, ajint word,
				     AjBool ignorebz,
				     const ajulong *calcfreq_array,
				     ajulong calcfreq_total)
{
    double result;
    ajint i;
    ajint offset = 0;
    const char *s;
    AjBool otherflag;
    ajulong tul;
    
    result = 1.0;

    s = ajStrGetPtr(dispseq);

    for(i=0; i<word; i++)
    {
        /* get the value of the next residue in dispseq */
        tul = embNmerProt2int(s, 1, i, &otherflag, ignorebz);
	offset = (ajint) tul;
        result *= (double)calcfreq_array[offset]/(double)calcfreq_total;
    }
    return result;

}                                      


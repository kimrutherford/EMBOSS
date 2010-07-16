/* @source sirna application
**
** Finds siRNA duplexes in mRNA
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
#include <math.h>
#include <stdlib.h>

#define NOT_WANTED -100




static void sirna_report(AjPReport report, const AjPSeq seq,
			 AjBool poliii, AjBool aa, AjBool tt, AjBool polybase,
			 AjBool context, AjPSeqout seqout);
static ajint sirna_begin(const AjPSeq seq, AjPReport report, AjBool poliii,
			  AjBool aa, AjBool tt, AjBool polybase,
			  AjBool context);
static void sirna_new_value(AjPList list, ajint pos, ajint score, 
			    ajint GCcount);
static int sirna_compare_score(const void* v1, const void* v2);
static void sirna_output(const AjPList list,
			 AjPFeattable TabRpt, const AjPSeq seq,
			 AjBool context, AjPSeqout seqout);




/* @datastatic PValue *********************************************************
**
** structure for position, score and GC count at the start of a window
**
** @alias SValue
** @alias OValue
**
** @attr pos [ajint] Position
** @attr GCcount [ajint] GC count
** @attr score [ajint] Score
******************************************************************************/

typedef struct SValue
{
    ajint pos;
    ajint GCcount;
    ajint score;
} OValue;
#define PValue OValue*




/* @prog sirna ****************************************************************
**
** Finds siRNA duplexes in mRNA
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPReport report = NULL;
    AjBool poliii;
    AjBool aa;
    AjBool tt;
    AjBool polybase;
    AjBool context;
    AjPSeqout seqout;
    
    AjPSeq seq = NULL;
		

    embInit("sirna",argc,argv);

    seqall    = ajAcdGetSeqall("sequence");
    poliii    = ajAcdGetBoolean("poliii");
    aa        = ajAcdGetBoolean("aa");
    tt        = ajAcdGetBoolean("tt");
    polybase  = ajAcdGetBoolean("polybase");
    context   = ajAcdGetBoolean("context");
    report    = ajAcdGetReport("outfile");
    seqout    = ajAcdGetSeqoutall("outseq");

    while(ajSeqallNext(seqall, &seq))
	sirna_report(report, seq, poliii, aa, tt, polybase, context, seqout);

    ajReportSetSeqstats(report, seqall);

    ajSeqallDel(&seqall);
    ajSeqDel(&seq);
    ajReportClose(report);
    ajSeqoutDel(&seqout);
    ajReportDel(&report);

    embExit();

    return 0;
}




/* @funcstatic sirna_report ***************************************************
**
** finds regions that can be used as siRNAs
**
** @param [u] report [AjPReport] Undocumented
** @param [r] seq [const AjPSeq] Sequence
** @param [r] poliii [AjBool] True if want poliii expressable probes
** @param [r] aa [AjBool] True if want AA at start of region
** @param [r] tt [AjBool] True if want TT at end of region
** @param [r] polybase [AjBool] True if we allow 4-mers of any base
** @param [r] context [AjBool] True if we wish to show the 2 bases before
**                              the probe region
** @param [u] seqout [AjPSeqout] Ouput sequence object
** @return [void] 
** @@
******************************************************************************/


static void sirna_report(AjPReport report, const AjPSeq seq, AjBool poliii,
			 AjBool aa, AjBool tt, AjBool polybase,
			 AjBool context, AjPSeqout seqout)
{
    AjPFeattable TabRpt=NULL;	 /* output feature table for report */
    ajint position;
    ajint i;
    ajint j;
    ajint window_len = 23; /* the length of the required siRNA region */
    ajint shift = 1;	       /* want to evaluate at each position */
    ajint count_gc;	     /* number of G + C bases in the window */
    char c;
    const char *cseq;
    ajint aaaa_count;
    ajint cccc_count;
    ajint gggg_count;
    ajint tttt_count;
    AjPList scorelist;
    ajint score;
    AjPStr newstr = NULL;

    /* scores */
    ajint CDS_50  = NOT_WANTED;
    ajint CDS_100 = -2;
    ajint A_start = +3;  
    ajint TT_end  = +1;
    ajint GGGG    = NOT_WANTED;

    ajint GC_5  = NOT_WANTED;
    ajint GC_6  = 0;
    ajint GC_7  = +2;
    ajint GC_8  = +4;
    ajint GC_9  = +5;
    ajint GC_10 = +6;
    ajint GC_11 = +5;
    ajint GC_12 = +4;
    ajint GC_13 = +2;
    ajint GC_14 = +0;
    ajint GC_15 = NOT_WANTED;

    /* heavy penalty for position 2 not 'A' */
    ajint POS_2_NOT_A = NOT_WANTED;

    /*
    ** if want Pol III expressable probes, then have a heavy
    ** penalty for not NAR(N17)YNN
    */
    ajint NOT_POLIII = NOT_WANTED;
   
    ajint e;
    ajint CDS_begin;

    /* initalise report feature table */
    TabRpt = ajFeattableNewSeq(seq);

    /* get start of CDS region */
    CDS_begin = sirna_begin(seq, report, poliii, aa, tt, polybase, context);

    cseq = ajSeqGetSeqC(seq);

    /*
    ** Using a list holding info on position, %GC, score. Sort by
    ** score and write the top N over the threshold to the report file
    */
    scorelist = ajListNew();

    /*
    ** initialise the GC count for the first window
    ** we count the %GC in the 20 bases from position 2 to 21 of the 23
    */
    count_gc = 0;
    for (i=CDS_begin+1; i < CDS_begin+window_len-2; i++)
    {
	c = cseq[i];
	if(tolower((int)c) == 'g' || tolower((int)c) == 'c')
	    count_gc++;
    }

    e = (ajSeqGetEnd(seq) - window_len); /* last position a window can start */
    for(position=CDS_begin; position < e; position+=shift)
    {
	/* initialise score */
	score = 0;

	/* penalty for being too close to the CDS start */
	if(CDS_begin != 0)
	{
	    /* check for a CDS start site */
	    if(position < CDS_begin + 50)
		score += CDS_50;
	    else if(position < CDS_begin + 100)
		score += CDS_100;
	}

	/* set score for GC bases */
	if(count_gc <= 5)
	    score += GC_5;
	else if(count_gc == 6)
	    score += GC_6;
	else if(count_gc == 7)
	    score += GC_7;
	else if(count_gc == 8)
	    score += GC_8;
	else if(count_gc == 9)
	    score += GC_9;
	else if(count_gc == 10)
	    score += GC_10;
	else if(count_gc == 11)
	    score += GC_11;
	else if(count_gc == 12)
	    score += GC_12;
	else if(count_gc == 13)
	    score += GC_13;
	else if(count_gc == 14)
	    score += GC_14;
	else if(count_gc >= 15)
	    score += GC_15;
      	

	ajDebug("%d) GC score = %d\n", position, score);
      
	/* check for AA at start */
	if(tolower((int)cseq[position]) == 'a')
	{
	    score += A_start;
	    ajDebug("Found A at start\n");
	}
	else if(aa)
	    score += NOT_WANTED;

	/* check for TT at end */
	if(((tolower((int)cseq[position+window_len-1]) == 't') &&
	    (tolower((int)cseq[position+window_len-2]) == 't')) ||
	   ((tolower((int)cseq[position+window_len-1]) == 'u') &&
	    (tolower((int)cseq[position+window_len-2]) == 'u')))
	{
	    score += TT_end;
	    ajDebug("Found TT at end\n");
	}
	else if(tt)
	    score += NOT_WANTED;      	


	/* check for GGGG and other 4-mers */
	aaaa_count = 0;
	cccc_count = 0;
	gggg_count = 0;
	tttt_count = 0;
	for(j=position; j < position+window_len; j++)
	{
	    c = cseq[j];
	    if(tolower((int)c) != 'a')
		aaaa_count = 0;
	    else
		aaaa_count++;

	    if(tolower((int)c) != 'c')
		cccc_count = 0;
	    else
		cccc_count++;

	    if(tolower((int)c) != 'g')
		gggg_count = 0;
	    else
		gggg_count++;

	    if(tolower((int)c) != 't' &&
	       tolower((int)c) != 'u')
		tttt_count = 0;
	    else
		tttt_count++;

	    /* always bomb out if we find GGGG */
	    if(gggg_count >= 4)
	    {
		score += GGGG;
		ajDebug("Found a GGGG\n");
		break;
	    }

	    /* bomb out if we don't want any 4-mer and we find one */        
	    if(!polybase)
	    {
		if(aaaa_count >= 4)
		{
		    score += NOT_WANTED;
		    break;
		}

		if(cccc_count >= 4)
		{
		    score += NOT_WANTED;
		    break;
		}

		if(tttt_count >= 4) {
		    score += NOT_WANTED;
		    break;
		}
	    }
	}

	/* check to see if the second base is an 'A' */
	if(tolower((int)cseq[position+1]) != 'a')
	    score += POS_2_NOT_A;	

	/*
	** if want probes that can be expressed from Pol III
	** expression vectors
	** add a penalty if they do not have the correct purine/pyrimidine
	** pattern
	*/
	if(poliii &&
	   ((tolower((int)cseq[position+2]) != 'a' &&
	     tolower((int)cseq[position+2]) != 'g') ||
	    (tolower((int)cseq[position+window_len-3]) != 'c' &&
	     tolower((int)cseq[position+window_len-3]) != 'u' &&
	     tolower((int)cseq[position+window_len-3]) != 't')))
	{
	    score += NOT_POLIII;
	    ajDebug("Not a PolIII region\n");
	}

	/* save the score and other details */
	if(score > 0)
	    sirna_new_value(scorelist, position, score, count_gc);

	/*
	** update count of GC bases
	** dropping this base off the 5' end of the window
	*/
	c = cseq[position+1];  
	ajDebug("%d) score = %d\n", position, score);
	ajDebug("** base = %c\n", c);

	if(tolower((int)c) == 'g' || tolower((int)c) == 'c')
	    count_gc--;

	/* new base shifted onto 3' end of window -2 */
	c = cseq[position+window_len-2];
	if(tolower((int)c) == 'g' || tolower((int)c) == 'c')
	    count_gc++;

	ajDebug("Next GC count=%d\n", count_gc);
    }

    /* now sort the list by score then position */
    ajListSort(scorelist, sirna_compare_score);



    /* now pop off the positive scores and write them to the report object */
    sirna_output(scorelist, TabRpt, seq, context, seqout);
    ajReportWrite(report, TabRpt, seq);

    ajSeqoutClose(seqout);
    ajListFreeData(&scorelist);
    ajFeattableDel(&TabRpt);
    ajStrDel(&newstr);

    return;
}




/* @funcstatic sirna_begin ***************************************************
**
** Finds the start position of the CDS  or uses -sbegin if no feature table
** Adds some explanation to the report header.
**
** @param [r] seq [const AjPSeq] Sequence
** @param [u] report [AjPReport] Report object
** @param [r] poliii [AjBool] True if want poliii expressable probes
** @param [r] aa [AjBool] True if want AA at start of region
** @param [r] tt [AjBool] True if want TT at end of region
** @param [r] polybase [AjBool] True if we allow 4-mers of any base
** @param [r] context [AjBool] True if we wish to show the 2 bases
**                           before the probe region
** @return [ajint] start of CDS region (using positions starting from 0)
** @@
******************************************************************************/

static ajint sirna_begin(const AjPSeq seq, AjPReport report, AjBool poliii,
			 AjBool aa, AjBool tt, AjBool polybase,
			 AjBool context)
{
    AjPFeattable featab = NULL;	    /* input sequence feature table */
    ajint begin = 0;
    AjPFeature gf = NULL;
    AjIList iter  = NULL;
    AjPStr type   = NULL;
    AjPStr head   = NULL;
    AjPStr head2  = NULL;

    /* get input sequence features */
    featab = ajSeqGetFeatCopy(seq);
    type   = ajStrNew();
    ajStrAssignC(&type, "CDS");
    head  = ajStrNew();
    head2 = ajStrNew();

    ajDebug("sirna_begin()\n");

    /* say something about the options in the report header */
    if(poliii)
	ajStrAppendC(&head, "Selecting only regions suitable for "
		  "PolIII expression vectors\n");

    if(aa)
	ajStrAppendC(&head, "Selecting only siRNA regions starting with 'AA'\n");

    if(tt)
	ajStrAppendC(&head, "Selecting only siRNA regions ending with 'TT'\n");

    if(!polybase)
	ajStrAppendC(&head, "Selecting only siRNA regions with no 4-mers "
		  "of any base\n");

    if(context)
	ajStrAppendC(&head, "The forward sense sequence shows the first 2 "
		  "bases of\nthe 23 base region in brackets, this "
		  "should be ignored\nwhen ordering siRNA probes.\n");
 
    /* are there any features - find the first CDS feature */
    if(ajFeattableGetSize(featab))
    {
	iter = ajListIterNewread(featab->Features);
	while(!ajListIterDone(iter))
	{
	    gf = ajListIterGet(iter);
	    if(ajStrMatchS(type, ajFeatGetType(gf)))
	    {
		/* is the feature 'CDS'? */
		begin = ajFeatGetStart(gf)-1;
		ajFmtPrintS(&head2, "CDS region found in feature table "
			    "starting at %d", 
			    begin+1);        
		/* ajDebug("Found a CDS at %d\n", begin);*/
		break;
	    }
	}
	ajListIterDel(&iter);
    }

    /* if didn't find a CDS, assume -sbegin is the CDS */
    if(begin == 0)
    {
	begin = ajSeqGetBegin(seq)-1;
	if(begin == 0)
	{
	    ajDebug("begin = 0\n");
	    ajFmtPrintS(&head2, "%s%s%s%s",  
			"No CDS region was found in the feature table.\n",
			"No CDS region was indicated by setting -sbegin.\n",
			"There will therefore be no penalty for siRNAs found ",
			"in the first 100 bases.");
	}
	else
	{
	    ajDebug("begin != 0\n");
	    ajFmtPrintS(&head2, "%s%s%s%d",
			"No CDS region was found in the feature table.\n",
			"The CDS region is assumed to start at the position ",
			"given by -sbegin: ",
			begin+1);
	}

	ajDebug("CDS specified at %d\n", begin);
    } 

    ajStrAppendS(&head, head2);
    ajReportSetHeaderS(report, head);

    ajDebug("sirna_begin begin=%d\n", begin);

    ajFeattableDel(&featab);
    ajStrDel(&type);
    ajStrDel(&head);
    ajStrDel(&head2);

    return begin;
}




/* @funcstatic sirna_new_value ***********************************************
**
** creates a PValue object and adds it to the list
**
** @param [u] list [AjPList] list to add value to
** @param [r] pos [ajint] position in sequence
** @param [r] score [ajint] score
** @param [r] GCcount [ajint] number of GC bases
** @return [void] 
** @@
******************************************************************************/

static void sirna_new_value(AjPList list, ajint pos, ajint score, 
			    ajint GCcount) 
{
	
    PValue value;

    AJNEW0(value);
    value->pos     = pos;
    value->GCcount = GCcount;
    value->score   = score;

    ajListPushAppend(list, value);

    return;
}




/* @funcstatic sirna_compare_score *******************************************
**
** Compares the scores of two positions for use in sorting
**
** @param [r] v1 [const void*] First structure
** @param [r] v2 [const void*] Second structure
** @return [int] -1 if first value should sort before second, +1 if the
**         second value should sort first. 0 if they are identical
**         in score
** @@
******************************************************************************/

static int sirna_compare_score(const void* v1, const void* v2)
{
    const PValue s1;
    const PValue s2;

    s1 = *(PValue const *)v1;
    s2 = *(PValue const *)v2;

	
    if(s1->score > s2->score)
    	return -1;
    else if(s1->score < s2->score)
    	return +1;

    /* sort by position if scores are equal */
    if(s1->pos < s2->pos)
	return -1;
    else if(s1->pos > s2->pos)
	return +1;

    return 0;
}




/* @funcstatic sirna_output *********************************************
**
** Output the probes
**
** @param [r] list [const AjPList] list of PValue structures
** @param [u] TabRpt [AjPFeattable] feature table 
** @param [r] seq [const AjPSeq] Sequence
** @param [r] context [AjBool] True if we wish to show the 2 bases before
**                             the probe region
** @param [u] seqout [AjPSeqout] Ouput sequence object
** @return [void]
** @@
******************************************************************************/

static void sirna_output(const AjPList list,
			 AjPFeattable TabRpt, const AjPSeq seq,
			 AjBool context, AjPSeqout seqout)
{
    AjIList iter;
    PValue value;
    AjPFeature gf;
    AjPStr tmpStr;
    AjPStr subseq;
    AjPStr source;
    AjPStr type;
    AjPStr name;
    AjPStr desc;
    AjPSeq seq23;


    tmpStr = ajStrNew();
    subseq = ajStrNew();
    source = ajStrNewC("siRNA");
    type   = ajStrNewC("misc_feature");
    name   = ajStrNew();     /* new name of the 23 base target sequence */
    desc = ajStrNew();       /* new description of 23 base target sequence */
    seq23 = ajSeqNewRes(24);   /* 23-base target sequence */

    /* if no hits then ignore much of this routine */
    if(ajListGetLength(list))
    {
	iter = ajListIterNewread(list);
	while((value = ajListIterGet(iter)) != NULL)
	{
	    /* ajDebug("(%d) %d %d\n", value->pos+1, value->score,
	       value->GCcount); */

	    gf = ajFeatNew(TabRpt, source, type, value->pos+1,
			   value->pos+23, (float)value->score, '+', 0);

	    ajFmtPrintS(&tmpStr, "*gc %4.1f",
			((float)value->GCcount*100.0)/20.0);
	    ajFeatTagAdd(gf,  NULL, tmpStr);

	    /* get the sequences to order */
	    if(context)
	    {
		/*
		** get the first two characters of the sequence before
		** the siRNA probe region
		*/
		ajStrAssignC(&subseq, "(");
		ajStrAppendSubS(&subseq, ajSeqGetSeqS(seq),
				value->pos, value->pos+1);
		ajStrAppendC(&subseq, ")");
	    }

	    ajStrAppendSubS(&subseq, ajSeqGetSeqS(seq),
			    value->pos+2, value->pos+20);
	    ajStrFmtUpper(&subseq);
	    ajStrExchangeKK(&subseq, 'T', 'U');
	    ajStrAppendC(&subseq, "dTdT");
	    ajFmtPrintS(&tmpStr, "*forward %S", subseq);
	    ajFeatTagAdd(gf,  NULL, tmpStr);

	    ajStrAssignSubS(&subseq, ajSeqGetSeqS(seq),
			    value->pos+2, value->pos+20);
	    ajStrFmtUpper(&subseq);
	    ajSeqstrReverse(&subseq);
	    ajStrExchangeKK(&subseq, 'T', 'U');
	    ajStrAppendC(&subseq, "dTdT");
	    ajFmtPrintS(&tmpStr, "*reverse %S", subseq);
	    ajFeatTagAdd(gf,  NULL, tmpStr);

	    /*
	    ** now write out the 23 base bit of sequence to the seqout file
	    ** get sequence
	    */
	    ajDebug("Now write sequence file\n");
	    ajStrAssignSubS(&subseq, ajSeqGetSeqS(seq),
			    value->pos, value->pos+22);
	    ajSeqAssignSeqS(seq23, subseq);
	    ajSeqSetNuc(seq23);

	    /* give it a name */
	    ajDebug("Doing name\n");
	    ajStrAssignS(&name, ajSeqGetNameS(seq));
	    ajStrAppendC(&name, "_");
	    ajStrFromInt(&tmpStr, value->pos+1);
	    ajStrAppendS(&name, tmpStr);
	    ajSeqAssignNameS(seq23, name);

	    /* get description */
	    ajDebug("Doing description\n");
	    ajFmtPrintS(&desc, "%%GC %4.1f Score %d ", 
			((float)value->GCcount*100.0)/20.0,
			value->score);
	    ajStrAppendS(&desc, ajSeqGetDescS(seq));
	    ajSeqAssignDescS(seq23, desc);

	    ajDebug("Write seq23\n");
	    ajSeqoutWriteSeq(seqout, seq23);

	    /* prepare sequence string for re-use */
	    ajStrSetClear(&subseq);
	}
	ajListIterDel(&iter);
    }
  
    ajStrDel(&tmpStr);
    ajStrDel(&subseq);
    ajStrDel(&source);
    ajStrDel(&type);
    ajStrDel(&name);
    ajStrDel(&desc);
    ajSeqDel(&seq23);

    return;  
}




/*
For the definitive guide to siRNA, see:
http://www.mpibpc.gwdg.de/abteilungen/100/105/sirna.html


The siRNA user guide (revised October 11, 2002)

Selection of siRNA duplexes from the target mRNA sequence

Using Drosophila melanogaster lysates (Tuschl et al.  1999), we have
systematically analyzed the silencing efficiency of siRNA duplexes as a
function of the length of the siRNAs, the length of the overhang and the
sequence in the overhang (Elbashir et al.  2001c).  The most efficient
silencing was obtained with siRNA duplexes composed of 21-nt sense and
21-nt antisense strands, paired in a manner to have a 2-nt 3' overhang. 
The sequence of the 2-nt 3' overhang makes a small contribution to the
specificity of target recognition restricted to the unpaired nucleotide
adjacent to the first base pair.  2'-Deoxynucleotides in the 3'
overhangs are as efficient as ribonucleotides, but are often cheaper to
synthesize and probably more nuclease resistant.  We used to select
siRNA sequences with TT in the overhang. 

The targeted region is selected from a given cDNA sequence beginning 50
to 100 nt downstream of the start codon.  Initially, 5' or 3' UTRs and
regions nearby the start codon were avoided assuming that UTR-binding
proteins and/or translation initiation complexes may interfere with
binding of the siRNP or RISC endonuclease complex.  More recently,
however, we have targeted 3'-UTRs and have not experienced any problems
in knocking down the targeted genes.  In order to design a siRNA duplex,
we search for the 23-nt sequence motif AA(N19)TT (N, any nucleotide) and
select hits with approx.  50% G/C-content (30% to 70% has also worked in
our hands).  If no suitable sequences are found, the search is extended
using the motif NA(N21).  The sequence of the sense siRNA corresponds to
(N19)TT or N21 (position 3 to 23 of the 23-nt motif), respectively.  In
the latter case, we convert the 3' end of the sense siRNA to TT.  The
rationale for this sequence conversion is to generate a symmetric duplex
with respect to the sequence composition of the sense and antisense 3'
overhangs.  The antisense siRNA is synthesized as the complement to
position 1 to 21 of the 23-nt motif.  Because position 1 of the 23-nt
motif is not recognized sequence-specifically by the antisense siRNA,
the 3'-most nucleotide residue of the antisense siRNA, can be chosen
deliberately.  However, the penultimate nucleotide of the antisense
siRNA (complementary to position 2 of the 23-nt motif) should always be
complementary to the targeted sequence.  For simplifying chemical
synthesis, we always use TT.  More recently, we preferentially select
siRNAs corresponding to the target motif NAR(N17)YNN, where R is purine
(A, G) and Y is pyrimidine (C, U).  The respective 21-nt sense and
antisense siRNAs therefore begin with a purine nucleotide and can also
be expressed from pol III expression vectors without a change in
targeting site; expression of RNAs from pol III promoters is only
efficient when the first transcribed nucleotide is a purine. 

We always design siRNAs with symmetric 3' TT overhangs, believing that
symmetric 3' overhangs help to ensure that the siRNPs are formed with
approximately equal ratios of sense and antisense target RNA-cleaving
siRNPs (Elbashir et al.  2001b; Elbashir et al.  2001c).  Please note
that the modification of the overhang of the sense sequence of the siRNA
duplex is not expected to affect targeted mRNA recognition, as the
antisense siRNA strand guides target recognition.  In summary, no matter
what you do to your overhangs, siRNAs should still function to a
reasonable extent.  However, using TT in the 3' overhang will always
help your RNA synthesis company to let you know when you accidentally
order a siRNA sequences 3' to 5' rather than in the recommended format
of 5' to 3'.  You may think this is funny, but it has happened quite a
lot. 

Compared to antisense or ribozyme technology, the secondary structure of
the target mRNA does not appear to have a strong effect on silencing. 
We say that, because we have already knocked-down more than 20 genes
using a single, essentially randomly chosen siRNA duplex (Harborth et
al.  2001).  Only 3 siRNA duplexes have been ineffective so far.  In one
or two other cases, we have found siRNAs to be inactive because the
targeting site contained a single-nucleotide polymorphism.  We were also
able to knock-down two genes simultaneously (e.g.  lamin A/C and NuMA)
by using equal concentrations of siRNA duplexes. 

We recommend to blast-search (NCBI database) the selected siRNA sequence
against EST libraries to ensure that only one gene is targeted.  In
addition, we also recommend to knock-down your gene with two independent
siRNA duplexes to control for specificity of the silencing effect.  If
selected siRNA duplexes do not function for silencing, please check for
sequencing errors of the gene, polymorphisms, and whether your cell line
is really from the expected species.  Our initial studies on the
specificity of target recognition by siRNA duplexes indicate that a
single point mutation located in the paired region of an siRNA duplex is
sufficient to abolish target mRNA degradation (Elbashir et al.  2001c). 
Furthermore, it is unknown if targeting of a gene by two different siRNA
duplexes is more effective than using a single siRNA duplex.  We think
that the amount of siRNA-associating proteins is limiting for silencing
rather than the target accessibility. 

Sequences of siRNA duplexes used in our studies

The sequences of siRNA duplexes described in the Nature paper (Elbashir
et al.  2001a):
  
Lamin A/C
targeted region (cDNA): 5' AACTGGACTTCCAGAAGAACATC
           sense siRNA: 5'   CUGGACUUCCAGAAGAACAdTdT
       antisense siRNA: 5' UGUUCUUCUGGAAGUCCAGdTdT
    
   
Vimentin
targeted region (cDNA): 5' AACTACATCGACAAGGTGCGCTT
           sense siRNA: 5'   CUACAUCGACAAGGUGCGCdTdT
       antisense siRNA: 5' GCGCACCUUGUCGAUGUAGdTdT
      
     
NuMA
targeted region (cDNA): 5' AAGGCGTGGCAGGAGAAGTTCTT
           sense siRNA: 5'   GGCGUGGCAGGAGAAGUUCdTdT
       antisense siRNA: 5' GAACUUCUCCUGCCACGCCdTdT
           
          
Lamin B1
targeted region (cDNA): 5' AACGCGCTTGGTAGAGGTGGATT
           sense siRNA: 5'   CGCGCUUGGUAGAGGUGGAdTdT
       antisense siRNA: 5' UCCACCUCUACCAAGCGCGdTdT
             
            
GL2 Luciferase
targeted region (cDNA): 5' AACGTACGCGGAATACTTCGATT
           sense siRNA: 5'   CGUACGCGGAAUACUUCGAdTdT
       antisense siRNA: 5' UCGAAGUAUUCCGCGUACGdTdT

============================================================================

siRNA description from http://www.oligoengine.com/
30 October 2002

siRNA: Background and Applications RNA interference, or RNAi, is a
phenomenon in which double stranded RNA (dsRNA) effects silencing of the
expression of genes that are highly homologous to either of the RNA
strands in the duplex.  Gene silencing in RNAi results from the
degradation of mRNA sequences, and the effect has been used to determine
the function of many genes in Drosophilia, C.  elegans, and many plant
species. 

The duration of knockdown by siRNA can typically last for 7-10 days, and
has been shown to transfer to daughter cells.  Of further note, siRNAs
are effective at quantities much lower than alternative gene silencing
methodologies, including antisense and ribozyme based strategies. 
 
Considerable interest has developed in RNAi lately among life science
professionals.  The ability to quickly and easily generate
loss-of-function phenotypes in mammalian cells has spawned a new wave of
research endeavors; companies are also exploring the potential of siRNA
for use in drug development and gene-specific therapeutics. 
 
Due to various mechanisms of antiviral response to long dsRNA, RNAi at
first proved more difficult to establish in mammalian species.  Then,
Tuschl, Elbashir, and others discovered that RNAi can be elicited very
effectively by well-defined 21-base duplex RNAs.  When these small
interfering RNA, or siRNA, are added in duplex form with a transfection
agent to mammalian cell cultures, the 21-base-pair RNA acts in concert
with cellular components to silence the gene with sequence homology to
one of the siRNA sequences.

Strategies for the design of effective siRNA sequences have been
recently documented, most notably by Sayda Elbashir, Thomas Tuschl, et
al.  Like all of the design applications in the OligoEngine Platform,
our siRNA Design Tool transforms these strategies into an automated
process that improves accuracy, saves time, and provides valuable
feedback to the user. 
        
In brief, the Tool analyzes a user's gene sequence and applies various
algorithms to generate "candidate" siRNA sequences that are 19
nucleotides in length.  Users then select the 2-nt 3' overhang to order
two (sense and antisense) 21-nucleotide RNA oligos. 
 
Use either:

1) Type the accession number of a GenBank mRNA file associated with your
gene of interest.  (Hint: for best results, use a GenBank file with
complete CDS)
        
2) Copy a complete mRNA sequence from another file and use the "Paste
Sequence" button to quickly input your sequence. 
       
Next, select the general parameters for siRNA design.  These include:
 
GC Content: 50% is optimal, and the Tool uses a default range of 45-55%. 
You can set parameters from 30-70%, the max range for potentially
effective siRNA. 

Leader: The tool uses AA as its default, the widely accepted standard
for the dinucleotide leader.  CA leaders can also be effective and may
be provide a broader range of options, especially when searching for a
sequence homologous to two related genes. 
 
UTR Search: While we do not recommend siRNA candidates targeting 5' or
3' UTR regions, the Tool gives users the option to design in UTRs (if
UTR sequences are available). 
 
ORF length: For paste-in sequences, the Tool first predicts the ORF to
ensure siRNA design within the coding region.  Setting a minimum ORF
length helps to make an accurate prediction and account for "false" stop
codons inside the CDS. 


 
Choose the 3' overhang: Click to select dTdT (default setting and
generally recommended choice) or UU.  Contact us for other custom
overhang options if required. 
 
BLAST your siRNA: click the BLAST button to check for sequence homology
against our local NCBI genome databases.  When complete, the button will
turn green- click it to view the BLAST results in a separate screen
(reports on any siRNA you purchase are saved for your future review). 


As mentioned, the siRNA Design Tool is based on guidelines developed by
leading researchers in the study of siRNA and RNAi: chiefly, Sayda
Elbashir, Thomas Tuschl, and their coworkers.  These in turn were
developed based on systematic analyses of the structural and functional
requirements of siRNA duplexes in Drosophila cell free lysates. 
 
They have systematically analyzed the silencing efficiency of siRNA
duplexes as a function of the length of the siRNAs, the length of the
3'-overhangs, and the sequence in the overhangs.  More specifically, the
most effective siRNA were those where:
 
The duplexes are composed of 21-nucleotide sense and 21-nt antisense
strands, paired in a manner to have a 19-nt duplex region and a 2-nt
overhang at each 3'-terminus. 
 
The sense strand of the 19-nt duplex is homologous to the target gene. 
 
The 3'-overhangs contain either UU or dTdT dinucleotides. 
 
The overhangs ensure that the sequence-specific endonuclease complexes
(siRNPs) are formed with approximately equal ratios of sense and
antisense target RNA cleaving siRNPs. 
 
The 3'-overhang in the sense strand provides no contribution to
recognition as it is believed the antisense siRNA strand guides target
recognition.  Therefore, 3'-overhang of the antisense sequences is
complementary to the target mRNA but the symmetrical 3'-overhang of the
sense siRNA oligo does not need to correspond to the mRNA.  The use of
deoxythymidines in both 3'-overhangs may increase nuclease resistance,
although siRNA duplexes with either UU or dTdT overhangs have both
proven effective. 
 
It is recommended that the targeted region of the mRNA is within the
ORF.  Both the 5' and 3' UTRs and regions near the start codon are not
recommended for targeting as these may be richer in regulatory protein
binding sites; UTR-binding proteins and/or translation initiation
complexes may interfere with binding of the siRNP endonuclease complex. 
(However, because these guidelines continue to evolve and be refined,
the Tool gives users the option to search for targets within the UTRs if
so desired.)
 
Taking these above characteristics and phenomena into account, the
general strategy for siRNA sequence design is summarized in the box at
right. 
 
To date, successful silencing has been achieved using this strategy to
select the target sequence.  Based on feedback from customers and
collaborators where transfection conditions have been optimized, siRNA
duplexes designed using this method will work 70-80% of the time.  This
percentage is consistent with reports from Tuschl, Elbashir, and others

    
============================================================================

http://www.xeragon.com/siRNA_support.html
Nov 2002

siRNA Target Sequence Design

Introduction

Initial studies of mammalian RNAi suggest that the most efficient
gene-silencing effect is achieved using double-stranded siRNA having a
19-nucleotide complementary region and a 2-nucleotide 3' overhang at
each end (see examples below).  Current models of the RNAi mechanism
suggest that the antisense siRNA strand recognizes the specific gene
target. 

In gene-specific RNAi, the open reading frame segment of the mRNA is
usually targeted.  The search for an appropriate target sequence should
begin 50-100 nucleotides downstream of the start codon.  To avoid
interference from mRNA regulatory proteins, sequences in the 5' or 3'
untranslated region or near the start codon should not be targeted. 

Shortly after the first reports on the use of RNAi in mammalian cells, a
set of simple rules for siRNA target site selection were developed
(http://www.mpibpc.gwdg.de/abteilungen/100/105/sirna.html).  One key
feature in this early set of rules was the suggestion to look for
specific sequence motifs to find the most appropriate siRNA targets. 
This original set of rules suggested that siRNA target sites with the
sequence AA(N19)TT (Example 3) are the most appropriate target sequence. 
If an appropriate sequence could not be found, an AA(N21) (Example 1) or
CA(N21) sequence motif could be considered.  In these situations,
because the sense strand does not appear to be involved in target
recognition, the sequence of the sense strand can be synthesized with a
dTdT regardless of the target sequence. 

A number of laboratories have used these rules to create siRNA
oligonucleotides that successfully silenced target gene expression. 
However, based on feedback from multiple laboratories, it does not
appear that there is a strict need for the siRNA target sequence to
begin with AA. 

Based on the results of an extensive study of a single gene, in which
approximately 70 different siRNA oligonucleotide target sequences were
used for RNAi, we suggest a modified set of rules (see below).  The
results of this recent work, indicate that choosing a 21 nt region of
the mRNA with a GC content as close as possible to 50% is a more
important consideration than choosing a target sequence that begins with
AA.  Another key consideration in target selection is to avoid having
more than three guanosines in a row, since poly G sequences can
hyperstack and form agglomerates that potentially interfere with the
siRNA silencing mechanism.  It is recommended that these two rules be a
primary consideration when designing and siRNA. 

There are some advantages to using a target sequence that begins with
AA.  The use of a target sequence that begins with an AA allows dT
residues to be used to create the 3'-end of the siRNA, this simplifies
the synthesis and decreases the cost of the siRNA.  Although duplexes
synthesized with UU overhangs appear to work as well as dTdT overhangs,
oligonucleotides with a dTdT overhang cost less and are potentially more
resistant to nucleases.  However, if the GC content is higher than 60%
alternate regions of the coding region of the mRNA should be considered. 
Our suggestions for criteria to consider when designing an siRNA
oligonucleotide are summarized in the "new relaxed set of rules" below. 
In addition, examples of siRNA oligonucleotides that start with an AA
sequence as well as examples of siRNA oligonucleotides that do not are
given. 

Current Considerations for siRNA Design (19 complementary bases and 2
bases overhang)

1.Choose a 21 (Example 1) or a 23 (Example 2 and 3) nt sequence in the
coding region of the mRNA with a GC ratio as close to 50% as possible. 
Ideally the GC ratio will be between 45% and 55%.  An siRNA with 60% GC
content has worked in many cases, however when an siRNA with 70% GC
content is used for RNAi typically a sharp decrease in the level of
silencing is observed.  Avoid regions within 50-100 nt of the AUG start
codon or 50-100 nt of the termination codon. 
  
2.Avoid more than three guanosines in a row.  Poly G sequences can
hyperstack and therefore form agglomerates that potentially interfere in
the siRNA silencing mechanism. 
   
3.Preferentially choose target sequences that start with two adenosines
(Example 1).  This will make synthesis easier, more economical, and
create siRNA that is potentially more resistant to nucleases.  When a
sequence that starts with AA is used, siRNA with dTdT overhangs can be
produced.  If it is not possible to find a sequence that starts with AA
and matches rules 1and 2, choose any 23 nt region of the coding sequence
with a GC content between 45 and 55% that does not have more than three
guanosines in a row (Example 3). 
  
4.Ensure that your target sequence is not homologous to any other genes. 
It is strongly recommended that a BLAST search of the target sequence be
performed to prevent the silencing of unwanted genes with a similar
sequence. 
   
5.Based on feedback from various customers, labelling the 3'-end of the
sense strand gives the best results with respect to not interfering with
the gene silencing mechanism of siRNA.  Consult our custom siRNA page
for available modifications. 

When these rules are used for siRNA target sequence design RNAi
effectively silences genes in more than 80% of cases.  Current data
indicate that there are regions of some mRNAs where gene silencing does
not work.  To help ensure that a given target gene is silenced, it is
advised that at least two target sequences as far apart on the gene as
possible be chosen. 
   
This method does not take into account mRNA secondary structure.  At
present it does not appear that mRNA secondary structure has a
significant impact on gene silencing.
  

*/

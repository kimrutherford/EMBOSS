/* @source vectorstrip application
**
** Looks for user defined linkers in a sequence and outputs
** only the sequence that lies between those linkers.
** @author Copyright (C) Val Curwen (vcurwen@hgmp.mrc.ac.uk)
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




/* @datastatic CPattern *******************************************************
**
** vectorstrip internals
**
** @alias clip_pattern
**
** @attr patstr [AjPStr] Undocumented
** @attr origpat [AjPStr] Undocumented
** @attr len [ajuint] Undocumented
** @attr real_len [ajuint] Undocumented
** @attr amino [AjBool] Undocumented
** @attr carboxyl [AjBool] Undocumented
** @attr buf [ajint*] Undocumented
** @attr sotable [ajuint*] Undocumented
** @attr solimit [ajuint] Undocumented
** @attr type [ajuint] Undocumented
** @attr off [EmbOPatBYPNode[AJALPHA]] Undocumented
** @attr re [AjPStr] Undocumented
** @attr skipm [ajuint**] Undocumented
** @attr tidy [const void*] Undocumented
******************************************************************************/

typedef struct clip_pattern
{
    AjPStr patstr;
    AjPStr origpat;
    ajuint len;
    ajuint real_len;
    AjBool amino;
    AjBool carboxyl;
    ajint* buf;
    ajuint* sotable;
    ajuint solimit;
    ajuint type;
    EmbOPatBYPNode off[AJALPHA];
    AjPStr re;
    ajuint** skipm;
    const void* tidy;
}*CPattern;




/* @datastatic Vector *********************************************************
**
** vectorstrip internals
**
** @alias vector
**
** @attr name [AjPStr] Undocumented
** @attr fiveprime [AjPStr] Undocumented
** @attr threeprime [AjPStr] Undocumented
******************************************************************************/

typedef struct vector
{
    AjPStr name;
    AjPStr fiveprime;
    AjPStr threeprime;
}*Vector;




/* constructors */
static void vectorstrip_initialise_cp(CPattern* pat);
static Vector vectorstrip_initialise_vector(const AjPStr name,
					    const AjPStr five,
					    const AjPStr three);
static void vectorstrip_read_vector_data(AjPFile vectorfile,
					 AjPList vectorlist);

/* destructors */
static void vectorstrip_free_list(AjPList *list);
static void vectorstrip_free_cp(CPattern* pat);
static void vectorstrip_vlistdel(AjPList* vlist);


/* data processing */
static void vectorstrip_process_pattern(const AjPStr pat, AjPList hitlist,
					const AjPStr seqname,
					const AjPStr seqstr,
					ajint threshold, ajint begin,
					AjBool besthits);
static void vectorstrip_process_hits(const AjPList fivelist,
				     const AjPList threelist,
				     const AjPSeq sequence, AjPSeqout seqout,
				     AjPFile outf);
static void vectorstrip_scan_sequence(const Vector vector, AjPSeqout seqout,
				      AjPFile outf, AjPSeq sequence,
				      ajint mis_per,
				      AjBool besthits, AjBool allsequences);
static void vectorstrip_ccs_pattern(const AjPStr pattern, AjPList hitlist,
				    const AjPStr seqname, const AjPStr seqstr,
				    ajint begin, ajuint* hits, ajint mm);
/* result output */
static void vectorstrip_print_sequence(const AjPSeq sequence,
				       ajint start, ajint end, AjPFile outf);
static void vectorstrip_write_sequence(const AjPSeq sequence,
				       AjPSeqout seqout,
				       ajint start, ajint end);
static void vectorstrip_print_hits(AjPList l, AjPFile outf, const AjPStr seq,
				   ajint begin);
static void vectorstrip_reportseq(const AjPStr seqstr, AjPFile outf);




/* @prog vectorstrip **********************************************************
**
** Strips out DNA between a pair of vector sequences
**
******************************************************************************/

int main(int argc, char **argv)
{
    /* sequence related */
    AjPSeqall seqall;
    AjPSeq seq;
    AjPSeqout seqout;
    AjPFile outf;
    ajint threshold;
    AjBool vec = AJFALSE;
    AjPFile vectorfile;
    AjPList vectorlist = NULL;
    AjBool besthits = AJTRUE;
    AjBool allsequences;
    Vector vvec;
    /* pattern related */
    AjPStr fiveprime  = NULL;
    AjPStr threeprime = NULL;

    /* get values for parameters */
    embInit("vectorstrip", argc, argv);

    seqall     = ajAcdGetSeqall("sequence");
    outf       = ajAcdGetOutfile("outfile");
    seqout     = ajAcdGetSeqoutall("outseq");
    vec        = ajAcdGetToggle("readfile");
    besthits   = ajAcdGetBoolean("besthits");
    fiveprime  = ajAcdGetString("alinker");
    threeprime = ajAcdGetString("blinker");
    vectorfile = ajAcdGetInfile("vectorsfile");
    allsequences = ajAcdGetBoolean("allsequences");

    vectorlist = ajListNew();

    /* data from command line or file? */
    if(vec)
    {
        if (!vectorfile)
            ajFatal("vectors file not found");
	vectorstrip_read_vector_data(vectorfile, vectorlist);
    }
    else
    {
	Vector v = NULL;
	AjPStr name = NULL;
	name = ajStrNewC("no_name");

	v = vectorstrip_initialise_vector(name, fiveprime, threeprime);
	ajListPushAppend(vectorlist, v);
	ajStrDel(&name);
    }

    threshold = ajAcdGetInt("mismatch");

    /* check there are vectors to be searched */
    if(!ajListGetLength(vectorlist))
    {
	ajErr("No suitable vectors found - exiting");
	embExitBad();
	return 0;
    }

    /* search each sequence for the vectors */
    while(ajSeqallNext(seqall,&seq))
    {
	AjIList iter=ajListIterNewread(vectorlist);
	while(!ajListIterDone(iter))
	{
	    vvec = ajListIterGet(iter);
	    vectorstrip_scan_sequence(vvec, seqout, outf, seq, threshold,
				      besthits, allsequences);
	}
	ajListIterDel(&iter);
    }


    ajSeqoutClose(seqout);

    ajStrDel(&fiveprime);
    ajStrDel(&threeprime);

    ajFileClose(&outf);
    ajSeqallDel(&seqall);
    ajSeqDel(&seq);
    ajSeqoutDel(&seqout);
    ajFileClose(&vectorfile);

    vectorstrip_vlistdel(&vectorlist);

    embExit();

    return 0;
}




/* @funcstatic vectorstrip_vlistdel *******************************************
**
** Undocumented.
**
** @param [d] vlist [AjPList*] vectors
** @return [void]
******************************************************************************/

static void vectorstrip_vlistdel(AjPList* vlist)
{
    Vector v = NULL;
    
    while(ajListPop(*vlist,(void **)&v))
    {
	ajStrDel(&v->name);
	ajStrDel(&v->fiveprime);
	ajStrDel(&v->threeprime);
	AJFREE(v);
    }
    ajListFree(vlist);
}


/* "constructors" */

/* @funcstatic vectorstrip_initialise_cp **************************************
**
** Initialises data members of a CPattern.
**
** @param [w] pat [CPattern*] CPattern to be initialised
** @return [void]
******************************************************************************/

static void vectorstrip_initialise_cp(CPattern* pat)
{
    AJNEW(*pat);
    (*pat)->patstr   = NULL;
    (*pat)->origpat  = ajStrNew();
    (*pat)->type     = 0;
    (*pat)->len      = 0;
    (*pat)->real_len = 0;
    (*pat)->re       = NULL;
    (*pat)->amino    = 0;
    (*pat)->carboxyl = 0;

    (*pat)->buf     = NULL;
    (*pat)->sotable = NULL;
    (*pat)->solimit = 0;
    (*pat)->skipm   = NULL;
    (*pat)->tidy    = NULL;

    return;
}




/* @funcstatic vectorstrip_initialise_vector **********************************
**
** Initialises data members of a Vector.
**
** @param [r] name [const AjPStr] string representing name of vector
** @param [r] five [const AjPStr] string representing 5' pattern
** @param [r] three [const AjPStr] string representing 3' pattern
** @return [Vector] Vector object
******************************************************************************/
static Vector vectorstrip_initialise_vector(const AjPStr name,
					    const AjPStr five,
					    const  AjPStr three)
{
    Vector vec;
    AJNEW(vec);
    vec->name       = ajStrNewS(name);
    vec->fiveprime  = ajStrNewS(five);
    vec->threeprime = ajStrNewS(three);

    return vec;
}




/* @funcstatic vectorstrip_read_vector_data ***********************************
**
** Reads vector data from a file into a list of Vectors.
**
** @param [u] vectorfile [AjPFile] the file containing vector data
** @param [w] vectorlist [AjPList] list to store vector data
**                                  contains one node for each set of
**                                  vector data in vectorfile
** @return [void]
******************************************************************************/

static void vectorstrip_read_vector_data(AjPFile vectorfile,
					 AjPList vectorlist)
{
    AjPStr rdline = NULL;
    AjPStrTok handle = NULL;

    Vector vector = NULL;

    while(ajReadlineTrim(vectorfile, &rdline))
    {
	AjPStr name  = NULL;
	AjPStr five  = NULL;
	AjPStr three = NULL;
	vector = NULL;

	if(ajStrGetCharFirst(rdline) == '#')
	    continue;

	if(ajStrSuffixC(rdline, ".."))
	    continue;

	handle = ajStrTokenNewC(rdline, " \t");
	ajStrTokenNextParse(&handle, &name);

	ajStrTokenNextParse(&handle, &five);
	ajStrFmtUpper(&five);
	ajStrTokenNextParse(&handle, &three);
	ajStrFmtUpper(&three);
	ajStrTokenDel(&handle);

	if(ajStrGetLen(five) || ajStrGetLen(three))
	{
	    vector = vectorstrip_initialise_vector(name, five, three);
	    ajListPushAppend(vectorlist, vector);
	}
	ajStrDel(&name);
	ajStrDel(&five);
	ajStrDel(&three);
    }

    ajStrDel(&rdline);

    return;
}




/* "destructors" */

/* @funcstatic vectorstrip_free_list ******************************************
**
** Frees a list of EmbPMatMatch.
**
** @param [d] list [AjPList*] the list of EmbPMatMatch to be freed
** @return [void]
******************************************************************************/

static void vectorstrip_free_list(AjPList *list)
{
    AjIList iter;

    iter = ajListIterNewread(*list);
    while(!ajListIterDone(iter))
    {
	EmbPMatMatch fm = ajListIterGet(iter);
	embMatMatchDel(&fm);
    }
    ajListFree(list);
    ajListFree(list);
    ajListIterDel(&iter);

    return;
}




/* @funcstatic vectorstrip_free_cp ********************************************
**
** Frees a CPattern.
**
** @param [d] pat [CPattern*] the pattern to be to be freed
** @return [void]
******************************************************************************/

static void vectorstrip_free_cp(CPattern* pat)
{
    ajuint i = 0;

    ajStrDel(&(*pat)->patstr);
    ajStrDel(&(*pat)->origpat);
    ajStrDel(&(*pat)->re);

    if(((*pat)->type==1 || (*pat)->type==2) && ((*pat)->buf))
	free((*pat)->buf);

    if(((*pat)->type==3 || (*pat)->type==4) && ((*pat)->sotable))
	free((*pat)->sotable);

    if((*pat)->type==6)
	for(i=0;i<(*pat)->real_len;++i) AJFREE((*pat)->skipm[i]);

    AJFREE(*pat);

    return;
}




/* data processing */

/* @funcstatic vectorstrip_process_pattern ************************************
**
** searches pattern against sequence starting with no mismatches
** then increases allowed mismatches until a) hits are found
** or b) the number of mismatches >= threshold% of the pattern length
**
** The pattern may be repeatedly recompiled using embPatCompile in order to
** allow searching with different numbers of mismatches.
**
** @param [r] pattern [const AjPStr] the pattern to be searched
** @param [w] hitlist [AjPList] list to which hits will be written
** @param [r] seqname [const AjPStr] the name of the sequence
** @param [r] seqstr [const AjPStr] string representing the sequence
**                                  to be searched
** @param [r] threshold [ajint] max allowable percent mismatch based on length
**                              of pattern
** @param [r] begin [ajint] start position of sequence
** @param [r] besthits [AjBool] Best hits
** @return [void]
******************************************************************************/
static void vectorstrip_process_pattern(const AjPStr pattern,
					AjPList hitlist,
					const AjPStr seqname,
					const AjPStr seqstr,
					ajint threshold, ajint begin,
					AjBool besthits)
{
    ajint mm = 0;
    ajint max_mm = 1;
    ajuint hits = 0;

    /* calculate max allowed mismatches based on threshold */
    if(threshold)
	max_mm = (ajint) (ajStrGetLen(pattern) * threshold)/100;
    else max_mm = 0;

    if(!besthits)
	/* report all hits */
	vectorstrip_ccs_pattern(pattern, hitlist, seqname, seqstr, begin,
				&hits, max_mm);
    else
	/* start with mm=0, keep going till we get a hit, then STOP */
	while((!hits) && (mm <= max_mm))
	{
	    vectorstrip_ccs_pattern(pattern, hitlist, seqname, seqstr, begin,
				    &hits, mm);
	    mm++;
	}

    return;
}




/* @funcstatic vectorstrip_process_hits ***************************************
**
** Output the hits of the patterns against a sequence; write out the
** relevant trimmed subsequences
**
** Parameters:
**
** AjPList fivelist - list of EmbPMatMatch representing hits of 5'
** pattern against the sequence
** AjPList threelist - list of EmbPMatMatch representing hits of 3'
** pattern against the sequence
** AjPSeq sequence - the sequence itself
** AjPSeqout seqout - place to writesubsequences
** AjPFile outf - file for writing information about the hits
**
** Parameters modified:
** results written to outf and seqout
**
** @param [r] fivelist [const AjPList] list of EmbPMatMatch representing hits
**                              of 5'  pattern against the sequence
** @param [r] threelist [const AjPList] list of EmbPMatMatch representing hits
**                               of 3'  pattern against the sequence
** @param [r] sequence [const AjPSeq] the sequence itself
** @param [u] seqout [AjPSeqout] place to writesubsequences
** @param [u] outf [AjPFile] file for writing information about the hits
** @return [void]
******************************************************************************/

static void vectorstrip_process_hits(const AjPList fivelist,
				     const AjPList threelist,
				     const AjPSeq sequence, AjPSeqout seqout,
				     AjPFile outf)
{
    ajuint i = 0;
    ajuint j = 0;
    ajint type = 0;

    AjPInt five;
    AjPInt three;

    EmbPMatMatch m = NULL;
    AjIList iter;

    five  = ajIntNew();	/* start positions for hits with 5' pattern */
    three = ajIntNew();	/* start positions for hits with 3' pattern */

    iter = ajListIterNewread(fivelist);

    /* populate five and three with start positions */
    while(!ajListIterDone(iter))
    {
	m = ajListIterGet(iter);
	ajIntPut(&five, i, ((m->len) + (m->start)));
	i++;
    }
    ajListIterDel(&iter);

    iter = ajListIterNewread(threelist);
    while(!ajListIterDone(iter))
    {
	m = ajListIterGet(iter);
	ajIntPut(&three, j, (m->start -1));
	j++;
    }
    ajListIterDel(&iter);

    /* classify the hits */
    if((ajListGetLength(fivelist) == 1 &&
	ajListGetLength(threelist) == 1)
       && ((ajIntGet(three, 0) + (ajint)m->len + 1) == ajIntGet(five, 0)))
	/* the patterns are identical and only match once in the sequence */
	type = 1;
    else if(ajIntLen(five) && ajIntLen(three))
	/* both patterns have hit */
	type = 2;
    else if(ajIntLen(five))
	/* five but not three */
	type = 3;
    else if(ajIntLen(three))
	/* three but not five */
	type = 4;

    /* write out subsequences */
    switch(type)
    {
    case 1:
	ajWarn("5' and 3' sequence matches are identical; inconclusive");
	break;

    case 2:
	/*
	** generally, every 5' hit will be matched against every 3' hit to
	** produce subsequences. Special case: 3' pattern matches upstream
	** of 5' pattern - to be consistent, this will cause everything
	** from the start of the sequence to the 3' hit to be written out,
	** and also everything from the 5' hit to the end of the sequence.
	** It's a bit back to front ...
	*/
	for(i=0; i<ajIntLen(five); i++)
	{
	    ajint hit = 0;
	    for(j=0; j<ajIntLen(three); j++)
		if(ajIntGet(five,i) <= ajIntGet(three,j))
		{
		    hit = 1;
		    vectorstrip_print_sequence(sequence,
					       ajIntGet(five,i),
					       ajIntGet(three,j), outf);
		    vectorstrip_write_sequence(sequence, seqout,
					       ajIntGet(five,i),
					       ajIntGet(three,j));
		}

	    if(!hit)
	    {
		vectorstrip_print_sequence(sequence, ajIntGet(five, i),
					   ajSeqGetEnd(sequence),outf);
		vectorstrip_write_sequence(sequence, seqout, ajIntGet(five, i),
					   ajSeqGetEnd(sequence));
	    }
	}

	for(i=0; i<ajIntLen(three); i++)
	{
	    ajint hit = 0;
	    for(j=0; j<ajIntLen(five); j++)
		if(ajIntGet(three,i) >= ajIntGet(five,j))
		    hit=1;
	    if(!hit)
	    {
		vectorstrip_print_sequence(sequence,
					   ajSeqGetBegin(sequence),
					   ajIntGet(three,i),outf);
		vectorstrip_write_sequence(sequence, seqout,
					   ajSeqGetBegin(sequence),
					   ajIntGet(three,i));
	    }
	}
	break;

    case 3:
	for(i=0; i<ajIntLen(five); i++)
	{
	    vectorstrip_print_sequence(sequence, ajIntGet(five, i),
				       ajSeqGetEnd(sequence), outf);
	    vectorstrip_write_sequence(sequence, seqout, ajIntGet(five, i),
				       ajSeqGetEnd(sequence));
	}
	break;
    case 4:
	for(j=0; j<ajIntLen(three); j++)
	{
	    vectorstrip_print_sequence(sequence,
				       ajSeqGetBegin(sequence),
				       ajIntGet(three, j), outf);
	    vectorstrip_write_sequence(sequence, seqout,
				       ajSeqGetBegin(sequence),
				       ajIntGet(three, j));
	}
	break;

    default:
	break;
    }


    ajIntDel(&five);
    ajIntDel(&three);

    return;
}




/* @funcstatic vectorstrip_scan_sequence **************************************
**
** Scans a Vector against a sequence
**
** @param [r] vector [const Vector] the vector data
** @param [u] seqout [AjPSeqout] where to write out subsequences
** @param [w] outf [AjPFile] file for writing results details
** @param [w] sequence [AjPSeq] the sequence to be scanned
** @param [r] mis_per [ajint] max mismatch percentage
** @param [r] besthits [AjBool] stop scanning when we get hits, even if
**                              mis_per is not reached yet
** @param [r] allsequences [AjBool] report all sequences in the output file
** @return [void]
******************************************************************************/

static void vectorstrip_scan_sequence(const Vector vector, AjPSeqout seqout,
				      AjPFile outf, AjPSeq sequence,
				      ajint mis_per,
				      AjBool besthits, AjBool allsequences)
{
    ajint begin = 0;
    ajint end = 0;

    /* set up seq related vars */
    AjPStr seqname;
    AjPStr text = NULL;

    /* need new hitlists for each pattern for each sequence */
    AjPList fivelist;
    AjPList threelist;

    seqname   = ajStrNew();
    fivelist  = ajListNew();
    threelist = ajListNew();


    ajStrAssignC(&seqname,ajSeqGetNameC(sequence));
    begin = ajSeqGetBegin(sequence);
    end   = ajSeqGetEnd(sequence);
    ajStrAssignSubC(&text,ajStrGetPtr(ajSeqGetSeqS(sequence)),begin-1,end-1);
    ajStrFmtUpper(&text);

    if(ajStrGetLen(vector->fiveprime))
	vectorstrip_process_pattern(vector->fiveprime, fivelist, seqname,
				    text, mis_per, begin, besthits);

    if(ajStrGetLen(vector->threeprime))
	vectorstrip_process_pattern(vector->threeprime, threelist, seqname,
				    text, mis_per, begin, besthits);

    if(!(ajListGetLength(fivelist) || ajListGetLength(threelist)))
    {
	ajFmtPrintF(outf, "\nSequence: %s \t Vector: %s\tNo match\n",
		    ajStrGetPtr(seqname), ajStrGetPtr(vector->name));
	if(allsequences)
	    vectorstrip_process_hits(fivelist, threelist, sequence,
				     seqout, outf);
    }
    else
    {
	ajFmtPrintF(outf, "\n\nSequence: %s \t Vector: %s\n",
		    ajStrGetPtr(seqname), ajStrGetPtr(vector->name));
	ajFmtPrintF(outf, "5' sequence matches:\n");
	vectorstrip_print_hits(fivelist,outf,text,begin);
	ajFmtPrintF(outf, "3' sequence matches:\n");
	vectorstrip_print_hits(threelist,outf,text,begin);

	ajFmtPrintF(outf, "Sequences output to file:\n");
	vectorstrip_process_hits(fivelist, threelist, sequence, seqout, outf);
    }

    vectorstrip_free_list(&fivelist);
    vectorstrip_free_list(&threelist);
    ajStrDel(&seqname);
    ajStrDel(&text);

    return;
}




/* @funcstatic vectorstrip_ccs_pattern ****************************************
**
** Classifies, compiles and searches pattern against seqstr with
** mm mismatches
**
** @param [r] pattern [const AjPStr] pattern to be searched for
** @param [w] hitlist [AjPList] list of hits
** @param [r] seqname [const AjPStr] name of sequence to be searched
** @param [r] seqstr [const AjPStr] string representing sequence to be searched
** @param [r] begin [ajint] start position of sequence
** @param [w] hits [ajuint*] number of hits
** @param [r] mm [ajint] number of mismatches
** @return [void]
******************************************************************************/

static void vectorstrip_ccs_pattern(const AjPStr pattern, AjPList hitlist,
				    const AjPStr seqname, const AjPStr seqstr,
				    ajint begin, ajuint* hits, ajint mm)
{
    /* set up CPattern */
    CPattern cpat = NULL;

    vectorstrip_initialise_cp(&cpat);
    ajStrAssignC(&(cpat->patstr), ajStrGetPtr(pattern));

    /* copy the original pattern for Henry Spencer code */
    ajStrAssignC(&(cpat->origpat), ajStrGetPtr(pattern));

    if(!(cpat->type = embPatGetType((cpat->origpat),&(cpat->patstr),mm,0,
				    &(cpat->real_len),
				    &(cpat->amino),
				    &(cpat->carboxyl))))
    {
	ajWarn("Illegal pattern: %s", ajStrGetPtr(cpat->patstr));
	vectorstrip_free_cp(&cpat);
	return;
    }

    embPatCompile(cpat->type, cpat->patstr,
		  &(cpat->len),
		  &(cpat->buf), cpat->off,
		  &(cpat->sotable), &(cpat->solimit),
		  &(cpat->real_len), &(cpat->re),
		  &(cpat->skipm),mm );


    embPatFuzzSearch(cpat->type, begin, cpat->patstr,
		     seqname, seqstr, hitlist,
		     cpat->len, mm, cpat->amino,
		     cpat->carboxyl, cpat->buf,
		     cpat->off, cpat->sotable,
		     cpat->solimit, cpat->re,
		     cpat->skipm, hits, cpat->real_len,
		     &(cpat->tidy));

    vectorstrip_free_cp(&cpat);
    return;
}




/* result output */


/* @funcstatic vectorstrip_print_sequence *************************************
**
** Details of the output
** sequence (hit positions, number of mismatches, sequences trimmed
** from 5' and 3' ends) are written to outf
**
** @param [r] sequence [const AjPSeq] the entire sequence
** @param [r] start [ajint] start position of desired subsequence relative
**                          to sequence
** @param [r] end [ajint] end position of desired subsequence relative
**                        to sequence
** @param [u] outf [AjPFile] file to write details of output subsequence
** @return [void]
******************************************************************************/

static void vectorstrip_print_sequence(const AjPSeq sequence,
				       ajint start, ajint end, AjPFile outf)
{
    AjPStr name = NULL;
    AjPStr num  = NULL;

    /* copy the sequence */
    AjPSeq seqcp     = NULL;
    AjPStr fivetrim  = NULL;
    AjPStr threetrim = NULL;
    AjPStr outs      = NULL;

    seqcp = ajSeqNewSeq(sequence);
    name  = ajStrNewS(ajSeqGetNameS(seqcp));
    num   = ajStrNew();

    if(start <= end)
    {
	ajSeqSetRange(seqcp, start, end);

	ajStrAppendC(&name, "_from_");
	ajStrFromInt(&num, start);
	ajStrAppendS(&name,num);
	ajStrAppendC(&name, "_to_");
	ajStrFromInt(&num, end);
	ajStrAppendS(&name, num);

	ajSeqAssignNameS(seqcp, name);

	/* report the hit to outf */
	ajFmtPrintF(outf, "\tfrom %d to %d\n", start, end);
	ajStrAssignSubS(&outs, ajSeqGetSeqS(seqcp), start-1, end-1);
	vectorstrip_reportseq(outs, outf);
	if(start !=1)
	{
	    ajStrAssignSubS(&fivetrim, ajSeqGetSeqS(seqcp), 0, start-2);
	    ajFmtPrintF(outf, "\tsequence trimmed from 5' end:\n");
	    vectorstrip_reportseq(fivetrim, outf);
	}

	if(end!=(ajint)ajSeqGetLen(seqcp))
	{
	    ajStrAssignSubS(&threetrim, ajSeqGetSeqS(seqcp),
			    end, ajSeqGetLen(seqcp));
	    ajFmtPrintF(outf, "\tsequence trimmed from 3' end:\n");
	    vectorstrip_reportseq(threetrim, outf);
	}
	ajFmtPrintF(outf, "\n");
    }

    ajSeqDel(&seqcp);
    ajStrDel(&fivetrim);
    ajStrDel(&threetrim);
    ajStrDel(&outs);
    ajStrDel(&name);
    ajStrDel(&num);

    return;
}



/* @funcstatic vectorstrip_write_sequence *************************************
**
** Details of the output
** sequence (hit positions, number of mismatches, sequences trimmed
** from 5' and 3' ends) are written to outf
**
** @param [r] sequence [const AjPSeq] the entire sequence
** @param [w] seqout [AjPSeqout] where to write out subsequences
** @param [r] start [ajint] start position of desired subsequence relative
**                          to sequence
** @param [r] end [ajint] end position of desired subsequence relative
**                        to sequence
** @return [void]
******************************************************************************/

static void vectorstrip_write_sequence(const AjPSeq sequence, AjPSeqout seqout,
				       ajint start, ajint end)
{
    AjPStr name = NULL;
    AjPStr num  = NULL;

    /* copy the sequence */
    AjPSeq seqcp     = NULL;
 
    seqcp = ajSeqNewSeq(sequence);
    name  = ajStrNewS(ajSeqGetNameS(seqcp));
    num   = ajStrNew();

    if(start <= end)
    {
	ajSeqSetRange(seqcp, start, end);

	ajStrAppendC(&name, "_from_");
	ajStrFromInt(&num, start);
	ajStrAppendS(&name,num);
	ajStrAppendC(&name, "_to_");
	ajStrFromInt(&num, end);
	ajStrAppendS(&name, num);

	ajSeqAssignNameS(seqcp, name);
	ajSeqoutWriteSeq(seqout, seqcp);

    }

    ajSeqDel(&seqcp);
     ajStrDel(&name);
    ajStrDel(&num);

    return;
}




/* @funcstatic vectorstrip_print_hits *****************************************
**
** Output hit positions and number of mismatches of a pattern against a
** sequence to file
**
** @param [u] hitlist [AjPList] list of hits of pattern against the sequence
**                              reversed during processing (could iterate
**                              backwards instead now)
** @param [u] outf [AjPFile] file for writing information about the hits
** @param [r] seq [const AjPStr] string representation of sequence
** @param [r] begin [ajint] start of sequence
** @return [void]
******************************************************************************/
static void vectorstrip_print_hits(AjPList hitlist, AjPFile outf,
				   const AjPStr seq,
				   ajint begin)
{
    EmbPMatMatch m;
    AjPStr s;
    AjIList iter;

    s=ajStrNew();

    ajListReverse(hitlist);
    iter = ajListIterNewread(hitlist);

    while(!ajListIterDone(iter))
    {
	m = ajListIterGet(iter);
	ajStrAssignSubC(&s,ajStrGetPtr(seq),m->start-begin,m->end-begin);

	ajFmtPrintF(outf,"\tFrom %d to %d with %d mismatches\n",m->start,
		    m->end, m->mm);

    }

    ajListIterDel(&iter);
    ajStrDel(&s);

    return;
}




/* @funcstatic vectorstrip_reportseq ******************************************
**
** Formatted output of sequence data.
** sequence is written to outf in lines of length 50.
**
** @param [r] seqstr [const AjPStr] sequence to be output
** @param [u] outf [AjPFile] file to write to
** @return [void]
******************************************************************************/

static void vectorstrip_reportseq(const AjPStr seqstr, AjPFile outf)
{
    AjPStr tmp = NULL;
    ajuint x = 0;
    ajuint linelen = 50;

    for(x=0; x<ajStrGetLen(seqstr); x+= linelen)
    {
	ajStrAssignSubS(&tmp, seqstr, x, x+linelen-1);
	ajFmtPrintF(outf, "\t\t%S\n", tmp);
    }

    ajStrDel(&tmp);

    return;
}

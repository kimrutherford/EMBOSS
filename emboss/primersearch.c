/* @source primersearch application
**
** Searches a set of primer pairs against a set of DNA sequences in both
** forward and reverse sense.
** Modification of fuzznuc.
** @author Copyright (C) Val Curwen (vac@sanger.ac.uk)
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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define AJA2 50




/* @datastatic PGuts **********************************************************
**
** the internals of a primer; each Primer has two of these,
** one forward and one reverse
**
** @alias primerguts
**
** @attr patstr [AjPStr] Undocumented
** @attr origpat [AjPStr] Undocumented
** @attr type [ajuint] Undocumented
** @attr len [ajuint] Undocumented
** @attr real_len [ajuint] Undocumented
** @attr amino [AjBool] Undocumented
** @attr carboxyl [AjBool] Undocumented
** @attr mm [ajuint] Undocumented
** @attr buf [ajint*] Undocumented
** @attr sotable [ajuint*] Undocumented
** @attr off [EmbOPatBYPNode[AJALPHA]] Undocumented
** @attr re [AjPStr] Undocumented
** @attr skipm [ajuint**] Undocumented
** @attr tidy [const void*] Undocumented
** @attr solimit [ajuint] Undocumented
** @attr Padding [char[4]] Padding to alignment boundary
******************************************************************************/

typedef struct primerguts
{
    AjPStr patstr;
    AjPStr origpat;
    ajuint type;
    ajuint len;
    ajuint real_len;
    AjBool amino;
    AjBool carboxyl;

    ajuint mm;

    ajint* buf;
    ajuint* sotable;

    EmbOPatBYPNode off[AJALPHA];
    AjPStr re;
    ajuint** skipm;
    const void* tidy;
    ajuint solimit;
    char Padding[4];
} *PGuts;




/* @datastatic PHit ***********************************************************
**
** holds details of a hit against a sequence ie this primer will amplify
**
** @alias primerhit
**
** @attr seqname [AjPStr] Undocumented
** @attr desc [AjPStr] Undocumented
** @attr acc [AjPStr] Undocumented
** @attr forward [AjPStr] pattern that hits forward strand 
** @attr reverse [AjPStr] pattern that hits reverse strand 
** @attr forward_pos [ajuint] Undocumented
** @attr reverse_pos [ajuint] Undocumented
** @attr amplen [ajuint] Undocumented
** @attr forward_mismatch [ajuint] Undocumented
** @attr reverse_mismatch [ajuint] Undocumented
** @attr Padding [char[4]] Padding to alignment boundary
******************************************************************************/

typedef struct primerhit
{
    AjPStr seqname;
    AjPStr desc;
    AjPStr acc;
    AjPStr forward;
    AjPStr reverse;
    ajuint forward_pos;
    ajuint reverse_pos;
    ajuint amplen;
    ajuint forward_mismatch;
    ajuint reverse_mismatch;
    char Padding[4];
} *PHit;




/* @datastatic Primer *********************************************************
**
** primer pairs will be read into a list of these structs
**
** @alias primers
**
** @attr Name [AjPStr] Undocumented
** @attr forward [PGuts] Undocumented
** @attr reverse [PGuts] Undocumented
** @attr hitlist [AjPList] Undocumented
******************************************************************************/

typedef struct primers
{
    AjPStr Name;
    PGuts forward;
    PGuts reverse;
    AjPList hitlist;
} *Primer;




/* "constructors" */
static void primersearch_initialise_pguts(PGuts* primer);

/* "destructors" */
static void primersearch_free_pguts(PGuts* primer);
static void primersearch_free_primer(void** x, void* cl);
static void primersearch_clean_hitlist(AjPList* hlist);

/* utilities */
static void primersearch_read_primers(AjPList* primerList, AjPFile primerFile,
				 ajint mmp);
static AjBool primersearch_classify_and_compile(Primer* primdata);
static void primersearch_primer_search(const AjPList primerList,
				       const AjPSeq seq);
static void primersearch_scan_seq(const Primer primdata,
			     const AjPSeq seq, AjBool reverse);
static void primersearch_store_hits(const Primer primdata, AjPList fhits_list,
			       AjPList rhits_list, const AjPSeq seq,
			       AjBool reverse);
static void primersearch_print_hits(const AjPList primerList, AjPFile outf);




/* @prog primersearch *********************************************************
**
** Searches DNA sequences for matches with primer pairs
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPSeq seq = NULL;
    AjPFile primerFile;		  /* read the primer pairs from a file */
    AjPFile outf;
    AjPList primerList;

    ajint mmp = 0;

    embInit("primersearch", argc, argv);

    seqall     = ajAcdGetSeqall("seqall");
    outf       = ajAcdGetOutfile("outfile");
    primerFile = ajAcdGetInfile("infile");
    mmp        = ajAcdGetInt("mismatchpercent");

    /* build list of forward/reverse primer pairs as read from primerfile */
    primerList = ajListNew();

    /* read in primers from primerfile, classify and compile them */
    primersearch_read_primers(&primerList,primerFile, mmp);

    /* check there are primers to be searched */
    if(!ajListGetLength(primerList))
    {
	ajUser("\nNo suitable primers found - exiting\n");
	embExitBad();
	return 0;

    }

    /* query sequences one by one */
    while(ajSeqallNext(seqall,&seq))
	primersearch_primer_search(primerList, seq);

    /* output the results */
    primersearch_print_hits(primerList, outf);

    /* delete all nodes of list, then the list itself */
    ajListMap(primerList, primersearch_free_primer, NULL);
    ajListFree(&primerList);
    ajListFree(&primerList);

    ajFileClose(&outf);

    ajSeqallDel(&seqall);
    ajSeqDel(&seq);

    ajFileClose(&primerFile);

    embExit();

    return 0;
}




/* "constructors" */

/* @funcstatic primersearch_initialise_pguts **********************************
**
** Initialise primer guts
**
** @param [w] primer [PGuts*] primer guts
** @@
******************************************************************************/

static void primersearch_initialise_pguts(PGuts* primer)
{

    AJNEW(*primer);
    (*primer)->patstr  = NULL;
    (*primer)->origpat = ajStrNew();
    (*primer)->type = 0;
    (*primer)->len  = 0;
    (*primer)->real_len = 0;
    (*primer)->re = NULL;
    (*primer)->amino = 0;
    (*primer)->carboxyl = 0;
    (*primer)->tidy = NULL;

    (*primer)->mm = 0;
    (*primer)->buf = NULL;
    (*primer)->sotable = NULL;
    (*primer)->solimit = 0;
    (*primer)->re = NULL;
    (*primer)->skipm = NULL;

    return;
}




/* "destructors" */

/* @funcstatic primersearch_free_pguts ****************************************
**
** Frees up all the internal members of a PGuts struct
**
** @param [d] primer [PGuts*] Undocumented
** @return [void]
** @@
******************************************************************************/

static void primersearch_free_pguts(PGuts* primer)
{
    ajuint i = 0;

    ajStrDel(&(*primer)->patstr);
    ajStrDel(&(*primer)->origpat);
    ajStrDel(&(*primer)->re);


    if(((*primer)->type==1 || (*primer)->type==2) && ((*primer)->buf))
	free((*primer)->buf);

    if(((*primer)->type==3 || (*primer)->type==4) && ((*primer)->sotable))
	free((*primer)->sotable);

    if((*primer)->type==6)
	for(i=0;i<(*primer)->real_len;++i) AJFREE((*primer)->skipm[i]);
    AJFREE(*primer);

    return;
}




/* @funcstatic primersearch_free_primer ***************************************
**
** frees up the internal members of a Primer
**
** @param [r] x [void**] Undocumented
** @param [r] cl [void*] Undocumented
** @@
******************************************************************************/

static void primersearch_free_primer(void **x, void *cl)
{
    Primer* p;
    Primer primdata;
    AjIList lIter;

    (void) cl;				/* make it used */

    p = (Primer*) x;
    primdata = *p;

    primersearch_free_pguts(&primdata->forward);
    primersearch_free_pguts(&primdata->reverse);
    ajStrDel(&primdata->Name);

    /* clean up hitlist */
    lIter = ajListIterNewread(primdata->hitlist);
    while(!ajListIterDone(lIter))
    {
	PHit phit = ajListIterGet(lIter);
	ajStrDel(&phit->forward);
	ajStrDel(&phit->reverse);
	ajStrDel(&phit->seqname);
	ajStrDel(&phit->acc);
	ajStrDel(&phit->desc);

	AJFREE(phit);
    }

    ajListFree(&primdata->hitlist);
    ajListFree(&primdata->hitlist);
    ajListIterDel(&lIter);

    AJFREE(primdata);

    return;
}




/* @funcstatic primersearch_clean_hitlist *************************************
**
** Clean the hitlist
**
** @param [d] hlist [AjPList*] Undocumented
** @@
******************************************************************************/

static void primersearch_clean_hitlist(AjPList* hlist)
{
    AjIList lIter;

    lIter = ajListIterNewread(*hlist);
    while(!ajListIterDone(lIter))
    {
	EmbPMatMatch fm = ajListIterGet(lIter);
	embMatMatchDel(&fm);
    }
    ajListFree(hlist);
    ajListFree(hlist);
    ajListIterDel(&lIter);

    return;
}




/* utilities */

/* @funcstatic primersearch_read_primers **************************************
**
** read primers in from primerfile, classify and compile the patterns
**
** @param [w] primerList [AjPList*] primer list
** @param [u] primerFile [AjPFile] Undocumented
** @param [r] mmp [ajint] Undocumented
** @@
******************************************************************************/

static void primersearch_read_primers(AjPList *primerList, AjPFile primerFile,
				 ajint mmp)
{
    AjPStr rdline = NULL;
    AjPStrTok handle = NULL;

    ajint nprimers = 0;
    Primer primdata = NULL;


    while (ajFileReadLine (primerFile, &rdline))
    {
	primdata = NULL;
	if (ajStrGetCharFirst(rdline) == '#')
	    continue;
	if (ajStrSuffixC(rdline, ".."))
	    continue;

	AJNEW(primdata);
	primdata->Name = NULL;

	primersearch_initialise_pguts(&primdata->forward);
	primersearch_initialise_pguts(&primdata->reverse);

	primdata->hitlist = ajListNew();

	handle = ajStrTokenNewC(rdline, " \t");
	ajStrTokenNextParse(&handle, &primdata->Name);

	ajStrTokenNextParse(&handle, &primdata->forward->patstr);
	ajStrFmtUpper(&primdata->forward->patstr);
	ajStrTokenNextParse(&handle, &primdata->reverse->patstr);
	ajStrFmtUpper(&primdata->reverse->patstr);
	ajStrTokenDel(&handle);

	/* copy patterns for Henry Spencer code */
	ajStrAssignC(&primdata->forward->origpat,
		  ajStrGetPtr(primdata->forward->patstr));
	ajStrAssignC(&primdata->reverse->origpat,
		  ajStrGetPtr(primdata->reverse->patstr));

	/* set the mismatch level */
	primdata->forward->mm = (ajint)
	    (ajStrGetLen(primdata->forward->patstr)*mmp)/100;
	primdata->reverse->mm = (ajint)
	    (ajStrGetLen(primdata->reverse->patstr)*mmp)/100;

	if(primersearch_classify_and_compile(&primdata))
	{
	    ajListPushAppend(*primerList, primdata);
	    nprimers++;
	}
	else	/* there was something funny about the primer sequences */
	{
	    ajUser("Cannot use %s\n", ajStrGetPtr(primdata->Name));
	    primersearch_free_pguts(&primdata->forward);
	    primersearch_free_pguts(&primdata->reverse);
	    ajStrDel(&primdata->Name);
	    ajListFree(&primdata->hitlist);
	    ajListFree(&primdata->hitlist);
	    AJFREE(primdata);
	}
    }

    ajStrDel(&rdline);

    return;
}




/* @funcstatic primersearch_classify_and_compile ******************************
**
** determines pattern type and compiles it
**
** @param [w] primdata [Primer*] primer data
** @return [AjBool] true if useable primer
** @@
******************************************************************************/

static AjBool primersearch_classify_and_compile(Primer* primdata)
{

    /* forward primer */
    if(!((*primdata)->forward->type =
	 embPatGetType(((*primdata)->forward->origpat),
		       &((*primdata)->forward->patstr),
		       (*primdata)->forward->mm,0,
		       &((*primdata)->forward->real_len),
		       &((*primdata)->forward->amino),
		       &((*primdata)->forward->carboxyl))))
	ajFatal("Illegal pattern");

    /* reverse primer */
    if(!((*primdata)->reverse->type =
	 embPatGetType(((*primdata)->reverse->origpat),
		       &((*primdata)->reverse->patstr),
		       (*primdata)->reverse->mm,0,
		       &((*primdata)->reverse->real_len),
		       &((*primdata)->reverse->amino),
		       &((*primdata)->reverse->carboxyl))))
	ajFatal("Illegal pattern");

    embPatCompile((*primdata)->forward->type,
		  (*primdata)->forward->patstr,
		  &((*primdata)->forward->len),
		  &((*primdata)->forward->buf),
		  (*primdata)->forward->off,
		  &((*primdata)->forward->sotable),
		  &((*primdata)->forward->solimit),
		  &((*primdata)->forward->real_len),
		  &((*primdata)->forward->re),
		  &((*primdata)->forward->skipm),
		  (*primdata)->forward->mm );

    embPatCompile((*primdata)->reverse->type,
		  (*primdata)->reverse->patstr,
		  &((*primdata)->reverse->len),
		  &((*primdata)->reverse->buf),
		  (*primdata)->reverse->off,
		  &((*primdata)->reverse->sotable),
		  &((*primdata)->reverse->solimit),
		  &((*primdata)->reverse->real_len),
		  &((*primdata)->reverse->re),
		  &((*primdata)->reverse->skipm),
		  (*primdata)->reverse->mm );

    return AJTRUE;			/* this is a useable primer */
}




/* @funcstatic primersearch_primer_search *************************************
**
** tests the primers in primdata against seq and writes results to outfile
**
** @param [r] primerList [const AjPList] primer list
** @param [r] seq [const AjPSeq] sequence
** @@
******************************************************************************/

static void primersearch_primer_search(const AjPList primerList,
				       const AjPSeq seq)
{
    AjIList listIter;

    /* test each list node against this sequence */
    listIter = ajListIterNewread(primerList);
    while(!ajListIterDone(listIter))
    {
	Primer curr_primer = ajListIterGet(listIter);

	primersearch_scan_seq(curr_primer, seq, AJFALSE);
	primersearch_scan_seq(curr_primer, seq, AJTRUE);
    }

    ajListIterDel(&listIter);

    return;
}




/* @funcstatic primersearch_scan_seq ******************************************
**
** scans the primer pairs against the sequence in either forward
** sense or reverse complemented
** works out amplimer length if the two primers both hit
**
** @param [r] primdata [const Primer] primer data
** @param [r] seq [const AjPSeq] sequence
** @param [r] reverse [AjBool] do reverse
** @@
******************************************************************************/

static void primersearch_scan_seq(const Primer primdata,
			     const AjPSeq seq, AjBool reverse)
{
    AjPStr seqstr = NULL;
    AjPStr revstr = NULL;
    AjPStr seqname = NULL;
    ajuint fhits = 0;
    ajuint rhits = 0;
    AjPList fhits_list = NULL;
    AjPList rhits_list = NULL;

    /* initialise variables for search */
    ajStrAssignC(&seqname,ajSeqGetNameC(seq));
    ajStrAssignS(&seqstr, ajSeqGetSeqS(seq));
    ajStrAssignS(&revstr, ajSeqGetSeqS(seq));
    ajStrFmtUpper(&seqstr);
    ajStrFmtUpper(&revstr);
    ajSeqstrReverse(&revstr);
    fhits_list = ajListNew();
    rhits_list = ajListNew();

    if(!reverse)
    {
	/* test OligoA against forward sequence, and OligoB against reverse */
	embPatFuzzSearch(primdata->forward->type,
			 ajSeqGetBegin(seq),
			 primdata->forward->patstr,
			 seqname,
			 seqstr,
			 fhits_list,
			 primdata->forward->len,
			 primdata->forward->mm,
			 primdata->forward->amino,
			 primdata->forward->carboxyl,
			 primdata->forward->buf,
			 primdata->forward->off,
			 primdata->forward->sotable,
			 primdata->forward->solimit,
			 primdata->forward->re,
			 primdata->forward->skipm,
			 &fhits,
			 primdata->forward->real_len,
			 &(primdata->forward->tidy));

	if(fhits)
	    embPatFuzzSearch(primdata->reverse->type,
			     ajSeqGetBegin(seq),
			     primdata->reverse->patstr,
			     seqname,
			     revstr,
			     rhits_list,
			     primdata->reverse->len,
			     primdata->reverse->mm,
			     primdata->reverse->amino,
			     primdata->reverse->carboxyl,
			     primdata->reverse->buf,
			     primdata->reverse->off,
			     primdata->reverse->sotable,
			     primdata->reverse->solimit,
			     primdata->reverse->re,
			     primdata->reverse->skipm,
			     &rhits,
			     primdata->reverse->real_len,
			     &(primdata->reverse->tidy));
    }
    else
    {
	/*test OligoB against forward sequence, and OligoA against reverse  */
	embPatFuzzSearch(primdata->reverse->type,
			 ajSeqGetBegin(seq),
			 primdata->reverse->patstr,
			 seqname,
			 seqstr,
			 fhits_list,
			 primdata->reverse->len,
			 primdata->reverse->mm,
			 primdata->reverse->amino,
			 primdata->reverse->carboxyl,
			 primdata->reverse->buf,
			 primdata->reverse->off,
			 primdata->reverse->sotable,
			 primdata->reverse->solimit,
			 primdata->reverse->re,
			 primdata->reverse->skipm,
			 &fhits,
			 primdata->reverse->real_len,
			 &(primdata->reverse->tidy));

	if(fhits)
	    embPatFuzzSearch(primdata->forward->type,
			     ajSeqGetBegin(seq),
			     primdata->forward->patstr,
			     seqname,
			     revstr,
			     rhits_list,
			     primdata->forward->len,
			     primdata->forward->mm,
			     primdata->forward->amino,
			     primdata->forward->carboxyl,
			     primdata->forward->buf,
			     primdata->forward->off,
			     primdata->forward->sotable,
			     primdata->forward->solimit,
			     primdata->forward->re,
			     primdata->forward->skipm,
			     &rhits,
			     primdata->forward->real_len,
			     &(primdata->forward->tidy));
    }

    if(fhits && rhits)
	/* get amplimer length(s) and write out the hit */
	primersearch_store_hits(primdata, fhits_list, rhits_list,
				seq, reverse);

    /* tidy up */
    primersearch_clean_hitlist(&fhits_list);
    primersearch_clean_hitlist(&rhits_list);

    ajStrDel(&seqstr);
    ajStrDel(&revstr);
    ajStrDel(&seqname);

    return;
}




/* @funcstatic primersearch_store_hits ****************************************
**
** Store primer hits
**
** @param [r] primdata [const Primer] primer data
** @param [w] fhits [AjPList] forward hits
** @param [w] rhits [AjPList] reverse hits
** @param [r] seq [const AjPSeq] sequence
** @param [r] reverse [AjBool] do reverse
** @@
******************************************************************************/

static void primersearch_store_hits(const Primer primdata,
			       AjPList fhits, AjPList rhits,
			       const AjPSeq seq, AjBool reverse)
{
    ajint amplen = 0;
    AjIList fi;
    AjIList ri;

    PHit primerhit = NULL;

    fi = ajListIterNewread(fhits);
    while(!ajListIterDone(fi))
    {
	EmbPMatMatch fm = NULL;
	EmbPMatMatch rm = NULL;
	amplen = 0;

	fm = ajListIterGet(fi);
	ri = ajListIterNewread(rhits);
	while(!ajListIterDone(ri))
	{
	    ajint seqlen = ajSeqGetLen(seq);
	    ajint s = (fm->start);
	    ajint e;

	    rm = ajListIterGet(ri);
	    e = (rm->start-1);
	    amplen = seqlen-(s-1)-e;

	    if (amplen > 0)	   /* no point making a hit if -ve length! */
	    {
		primerhit = NULL;
		AJNEW(primerhit);
		primerhit->desc=NULL;	 /* must be NULL for ajStrAss */
		primerhit->seqname=NULL; /* must be NULL for ajStrAss */
		primerhit->acc=NULL;
		primerhit->forward=NULL;
		primerhit->reverse=NULL;
		ajStrAssignC(&primerhit->seqname,ajSeqGetNameC(seq));
		ajStrAssignS(&primerhit->desc, ajSeqGetDescS(seq));
		ajStrAssignS(&primerhit->acc, ajSeqGetAccS(seq));
		primerhit->forward_pos = fm->start;
		primerhit->reverse_pos = rm->start;
		primerhit->forward_mismatch = fm->mm;
		primerhit->reverse_mismatch = rm->mm;
		primerhit->amplen = amplen;
		if(!reverse)
		{
		    ajStrAssignS(&primerhit->forward,
				 primdata->forward->patstr);
		    ajStrAssignS(&primerhit->reverse,
				 primdata->reverse->patstr);
		}
		else
		{
		    ajStrAssignS(&primerhit->forward,
				 primdata->reverse->patstr);
		    ajStrAssignS(&primerhit->reverse,
				 primdata->forward->patstr);
		}
		ajListPushAppend(primdata->hitlist, primerhit);


	    }
	}
	/*
	**  clean up rListIter here as it will be new'ed again next
	**  time through
	*/
	ajListIterDel(&ri);
    }

    ajListIterDel(&fi);
    return;
}




/* @funcstatic primersearch_print_hits ****************************************
**
** Print primer hits
**
** @param [r] primerList [const AjPList] primer hits
** @param [w] outf [AjPFile] outfile
** @@
******************************************************************************/

static void primersearch_print_hits(const AjPList primerList, AjPFile outf)
{
    /* iterate through list of hits */
    AjIList lIter;

    ajint count = 1;
    lIter = ajListIterNewread(primerList);
    while(!ajListIterDone(lIter))
    {
	Primer primer = ajListIterGet(lIter);
	AjIList hIter = ajListIterNewread(primer->hitlist);
	count = 1;

	ajFmtPrintF(outf, "\nPrimer name %s\n", ajStrGetPtr(primer->Name));

	while(!ajListIterDone(hIter))
	{
	    PHit hit = ajListIterGet(hIter);
	    ajFmtPrintF(outf, "Amplimer %d\n", count);
	    ajFmtPrintF(outf, "\tSequence: %s %s \n\t%s\n",
			ajStrGetPtr(hit->seqname),
			ajStrGetPtr(hit->acc), ajStrGetPtr(hit->desc));
	    ajFmtPrintF(outf, "\t%s hits forward strand at %d with %d "
			"mismatches\n",
			ajStrGetPtr(hit->forward), hit->forward_pos,
			hit->forward_mismatch);
	    ajFmtPrintF(outf, "\t%s hits reverse strand at [%d] with %d "
			"mismatches\n",
			ajStrGetPtr(hit->reverse), (hit->reverse_pos),
			(hit->reverse_mismatch));
	    ajFmtPrintF(outf, "\tAmplimer length: %d bp\n", hit->amplen);
	    count++;
	}
	ajListIterDel(&hIter);
    }
    ajListIterDel(&lIter);

    return;
}

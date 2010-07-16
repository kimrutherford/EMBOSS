/* @source showalign application
**
** Display a multiple sequence alignment with consensus
**
** @author Copyright (C) Gary Williams (gwilliam@hgmp.mrc.ac.uk)
** 8 May 2001 - GWW - written
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

/*
**
** Outputs a set of sequence as compared to a reference sequence as follows:
**
** With reference: AII
** and sequence:   ALW
** ie. 		identical, similar, not-similar
**
** Show
** ----
**
** All		ALw
** Identical	A..
** Non-id		.lW
** Similar		Al.
** Dissimilar	..W
*/

#include "emboss.h"
#include <ctype.h>
#include <math.h>	/* for log10() */




/* @datastatic POrder *******************************************************
**
** showalign internals
**
** @alias SOrder
** @alias OOrder
**
** @attr seq [AjPSeq] Sequence
** @attr similarity [ajint] total of similarity scores to consensus
**                          for sort order 
** @attr idcount [ajint] count of identical residues for stats
** @attr simcount [ajint] count of similar residues for stats
** @attr Padding [char[4]] Padding to alignment boundary
******************************************************************************/

typedef struct SOrder
{
    AjPSeq seq;
    ajint similarity;
    ajint idcount;
    ajint simcount;
    char Padding[4];
} OOrder;
#define POrder OOrder*




static ajint showalign_Getrefseq(const AjPStr refseq, const AjPSeqset seqset);
static void showalign_NiceMargin(const AjPSeqset seqset,
				 ajint *margin, AjBool docon,
				 ajint nrefseq);
static void showalign_Convert(AjPSeq  *seqs, const AjPStr show,
			      AjBool similarcase, ajint nrefseq,
			      ajint * const *sub,
			      const AjPSeqCvt cvt, const AjPSeq consensus);
static void showalign_MakeAll(const AjPSeq ref, AjPSeq seq,
			      ajint * const *sub,
			      const AjPSeqCvt cvt,AjBool similarcase);
static void showalign_MakeIdentity(const AjPSeq ref, AjPSeq seq);
static void showalign_MakeNonidentity(const AjPSeq ref,
				      AjPSeq seq,
				      ajint * const *sub,
				      const AjPSeqCvt cvt, AjBool similarcase);
static void showalign_MakeSimilar(const AjPSeq ref, AjPSeq seq,
				  ajint * const *sub,
				  const AjPSeqCvt cvt, AjBool similarcase);
static void showalign_MakeDissimilar(const AjPSeq ref, AjPSeq seq,
				     ajint * const *sub,
				     const AjPSeqCvt cvt);
static void showalign_Order(const AjPStr order, AjPSeq const * seqs,
			    const AjPSeq consensus,
			    ajint nrefseq,
			    ajint * const *sub, const AjPSeqCvt cvt,
			    OOrder *aorder);
static ajint showalign_Output(AjPFile outf, AjPSeq const * seqs, ajint nrefseq,
			      ajuint width, ajuint margin,
			      const AjPSeq consensus,
			      AjBool docon, AjBool bottom,
			      const OOrder * aorder,
			      AjBool html, const AjPRange highlight,
			      const AjPRange uppercase, AjBool number,
			      AjBool ruler, ajuint nseqs,
			      ajuint begin, ajuint end);
static AjPStr showalign_OutputNums(ajint nrefseq, const AjPSeq ref,
                                   ajuint firstpos, ajuint lastpos,
                                   ajuint margin);
static AjPStr showalign_OutputTicks(ajint nrefseq, const AjPSeq ref,
                                    ajuint *firstpos, ajuint *lastpos);
static ajint showalign_OutputSeq(AjPFile outf, const AjPSeq seq,
				 ajint pos, ajint end, ajint width,
				 ajint margin, AjBool html,
				 const AjPRange highlight,
				 const AjPRange uppercase);
static ajint showalign_CompareTwoSeqNames(const void * a, const void * b);
static ajint showalign_CompareTwoSeqSimilarities(const void * a,
						 const void * b);




/* @prog showalign ************************************************************
**
** Display a multiple sequence alignment
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPFile outf;
    AjPSeqset seqset = NULL;
    AjPStr refseq;		/* input name/number of reference sequence */
    ajint  nrefseq;		/* numeric reference sequence */
    AjPStr show=NULL;		/* what to show */
    ajint width;		/* width of displayed sequence line */
    ajint margin;		/* width of displayed margin on left side */
    AjPMatrix matrix;		/* scoring matrix structure */
    ajint **sub;		/* integer scoring matrix */
    AjPSeqCvt  cvt = 0;		/* conversion table for scoring matrix */
    /* True if want to change case based on Similarity */
    AjBool similarcase;
    float identity;
    ajint ident;
    float fplural;
    float setcase;
    AjPStr cons;
    AjPSeq consensus;
    AjBool html;		/* format for HTML display */
    AjPRange highlight;		/* ranges to colour in HTML */
    AjPRange uppercase;		/* ranges to uppercase */
    AjBool docon;		/* show the consensus line */
    /*
     *  show the refseq line at the bottom of the alignment as well
     *  as the top
     */
    AjBool bottom;
    AjPStr order;		/* input required order */
    OOrder *aorder;		/* the output order array */
    AjBool number;		/* display number line */
    AjBool ruler;		/* display ruler line */
    AjPSeq* seqs;
    ajint nseqs;
    ajint begin;
    ajint end;
    AjBool gaps;

    ajint i;

    embInit("showalign", argc, argv);

    seqset      = ajAcdGetSeqset("sequence");
    outf        = ajAcdGetOutfile("outfile");
    refseq      = ajAcdGetString("refseq");
    show        = ajAcdGetListSingle("show");
    order       = ajAcdGetListSingle("order");
    width       = ajAcdGetInt("width");
    margin      = ajAcdGetInt("margin");
    matrix      = ajAcdGetMatrix("matrix");
    similarcase = ajAcdGetBoolean("similarcase");
    docon       = ajAcdGetBoolean("consensus");
    bottom      = ajAcdGetBoolean("bottom");
    number      = ajAcdGetBoolean("number");
    ruler       = ajAcdGetBoolean("ruler");

    /* html and range formatting parameters */
    html      = ajAcdGetBoolean("html");
    uppercase = ajAcdGetRange("uppercase");
    highlight = ajAcdGetRange("highlight");

    /* consensus parameters */
    fplural   = ajAcdGetFloat("plurality");
    setcase   = ajAcdGetFloat("setcase");
    identity  = ajAcdGetFloat("identity");
    gaps      = ajAcdGetBoolean("gaps");

    cons      = ajStrNew();
    consensus = ajSeqNew();


   /* get conversion table and scoring matrix */
    cvt = ajMatrixGetCvt(matrix);
    sub = ajMatrixGetMatrix(matrix);

    /* get the number of the reference sequence */
    nrefseq = showalign_Getrefseq(refseq, seqset);

    /* change the % plurality to the fraction of absolute total weight */
    fplural = ajSeqsetGetTotweight(seqset) * fplural / 100;

    /*
    ** change the % identity to the number of identical sequences at a
    ** position required for consensus
    */
    ident = ajSeqsetGetSize(seqset) * (ajint)identity / 100;

    /* get the consensus sequence */
    embConsCalc(seqset, matrix, ajSeqsetGetSize(seqset), ajSeqsetGetLen(seqset),
		fplural, setcase, ident, gaps, &cons);
    ajSeqAssignSeqS(consensus, cons);	/* set the sequence string */

    /* name the sequence */

    ajSeqAssignNameC(consensus,"Consensus");

    /* if margin is given as -1 ensure it is reset to a nice value */
    showalign_NiceMargin(seqset, &margin, docon, nrefseq);

    /* order the output */
    AJCNEW(aorder, ajSeqsetGetSize(seqset));
    seqs = ajSeqsetGetSeqarray(seqset);
    showalign_Order(order, seqs, consensus, nrefseq, sub, cvt, aorder);

    /* convert all sequences except the refseq to the required symbols */
    showalign_Convert(seqs, show, similarcase, nrefseq, sub, cvt, consensus);

    /* output the sequences */
    nseqs = ajSeqsetGetSize(seqset);	/* number of sequences */
    begin = ajSeqsetGetBegin(seqset)-1;
    end   = ajSeqsetGetEnd(seqset)-1;
    showalign_Output(outf, seqs, nrefseq, width, margin, consensus, docon,
		     bottom, aorder, html, highlight, uppercase, number,
		     ruler, nseqs, begin, end);

    for (i=0;seqs[i];i++)
	ajSeqDel(&seqs[i]);
    AJFREE(seqs);

    ajFileClose(&outf);
    ajSeqDel(&consensus);
    AJFREE(aorder);

    ajSeqsetDel(&seqset);
    ajStrDel(&refseq);
    ajStrDel(&show);
    ajMatrixDel(&matrix);
    ajStrDel(&cons);
    ajRangeDel(&highlight);
    ajRangeDel(&uppercase);
    ajStrDel(&order);

    embExit();

    return 0;
}




/* @funcstatic showalign_Getrefseq ********************************************
**
** Determines which sequence should be the reference sequence.
** The first sequence in the set is returned as 0.
** -1 is returned as the consensus sequence.
**
** @param [r] refseq [const AjPStr] input name/number of reference sequence
** @param [r] seqset [const AjPSeqset] the sequences
** @return [ajint] the number of the reference sequence
** @@
******************************************************************************/

static ajint showalign_Getrefseq(const AjPStr refseq, const AjPSeqset seqset)
{
    ajint i;
    const AjPSeq seq;
    ajint ifound = 0;
    ajint icase = 0;

    for(i=0; i<(ajint)ajSeqsetGetSize(seqset); i++)
    {
	seq = ajSeqsetGetseqSeq(seqset, i);

	if(!ajStrCmpS(ajSeqGetNameS(seq), refseq))
	    return i;
	if(!ajStrCmpCaseS(ajSeqGetNameS(seq), refseq))
        {
            icase = i;
            ifound++;
        }
    }

    if(ifound == 1)
        return icase;

    /* not a name of a sequence, so it must be a number */
    if(ajStrToInt(refseq, &i))
    {
	if(i < 0 || i > (ajint)ajSeqsetGetSize(seqset))
	    ajFatal("Reference sequence number < 0 or > number of input "
		    "sequences: %d", i);
	return i-1;
    }
    else
	ajFatal("Reference sequence is not a sequence ID or a number: %S",
		refseq);

    return 0;
}




/* @funcstatic showalign_NiceMargin *******************************************
**
** If margin is input as -1, change it to the margin which allows
** the longest name to be displayed plus one extra space after it
**
** @param [r] seqset [const AjPSeqset] the sequences
** @param [w] margin [ajint*] margin
** @param [r] docon [AjBool] displaying the consensus line
** @param [r] nrefseq [ajint] the sequence being displayed
** @return [void]
** @@
******************************************************************************/

static void showalign_NiceMargin(const AjPSeqset seqset, ajint *margin,
				 AjBool docon, ajint nrefseq)
{
    ajint longest = 0;
    ajint len;
    ajuint i;
    const AjPSeq seq;

    /* if margin has been explicitly set, use that value */
    if(*margin != -1)
	return;

    /*
    **  Displaying the consensus at the end or using it as the ref
    **  seq?
    */
    if(docon || nrefseq == -1)
	longest = 9;			/* the length of "Consensus" */

    /* get length of longest sequence name */
    for(i=0; i<ajSeqsetGetSize(seqset); i++)
    {
	seq = ajSeqsetGetseqSeq(seqset, i);
	len = ajStrGetLen(ajSeqGetNameS(seq));
	if(len > longest)
	    longest = len;
    }

    *margin = longest+1;	/* use longest name plus one extra space */

    return;
}




/* @funcstatic showalign_Convert **********************************************
**
** Convert all sequences except the refseq to the required symbols
**
** @param [u] seqs [AjPSeq*] the sequences
** @param [r] show [const AjPStr] type of thing to show
** @param [r] similarcase [AjBool] change case according to similarity
** @param [r] nrefseq [ajint] number of the refseq
** @param [r] sub [ajint* const *] scoring matrix
** @param [r] cvt [const AjPSeqCvt] conversion table for scoring matrix
** @param [r] consensus [const AjPSeq] consensus sequence
** @return [void]
** @@
******************************************************************************/

static void showalign_Convert(AjPSeq* seqs, const AjPStr show,
			      AjBool similarcase, ajint nrefseq,
			      ajint * const *sub,
			      const AjPSeqCvt cvt, const AjPSeq consensus)
{
    ajint i;
    AjPSeq seq;
    const AjPSeq ref;
    char showchar;


    showchar = ajStrGetCharFirst(show); /* first char of 'show' */

    /* get the reference sequence */
    if(nrefseq == -1)
	ref = consensus;
    else
	ref = seqs[nrefseq];

    /* convert the other sequences */
    for(i=0; seqs[i]; i++)
	if(i != nrefseq)
	{
	    /* don't convert the reference sequence */
	    seq = seqs[i];
	    switch(showchar)
	    {
	    case 'A':			/* All - no change, except for case */
		showalign_MakeAll(ref, seq, sub, cvt, similarcase);
		break;
	    case 'I':			/* Identities */
		showalign_MakeIdentity(ref, seq);
		break;
	    case 'N':			/* Non-identities */
		showalign_MakeNonidentity(ref, seq, sub, cvt, similarcase);
		break;
	    case 'S':			/* Similarities */
		showalign_MakeSimilar(ref, seq, sub, cvt, similarcase);
		break;
	    case 'D':			/* Dissimilarities */
		showalign_MakeDissimilar(ref, seq, sub, cvt);
		break;
	    default:
		ajFatal("Unknown option for '-show': %S", show[0]);
		break;
	    }
	}

    return;
}




/* @funcstatic showalign_MakeAll **********************************************
**
** Leave the sequence unchanged unless we are changing the case depending
** on the similarity to the reference sequence
**
** @param [r] ref [const AjPSeq] the reference sequence
** @param [u] seq [AjPSeq] the sequence to be changed
** @param [r] sub [ajint * const *] scoring matrix
** @param [r] cvt [const AjPSeqCvt] conversion table for scoring matrix
** @param [r] similarcase [AjBool] change case depending on similarity
** @return [void]
** @@
******************************************************************************/

static void showalign_MakeAll(const AjPSeq ref,
			      AjPSeq seq, ajint * const *sub,
			      const AjPSeqCvt cvt, AjBool similarcase)
{
    ajint i;
    ajint lenseq;
    ajint lenref;

    AjPStr sstr = NULL;
    char *s;
    const char *r;

    /* if not changing the case, do nothing */
    if(!similarcase)
	return;

    sstr = ajSeqGetSeqCopyS(seq);
    lenseq = ajSeqGetLen(seq);
    lenref = ajSeqGetLen(ref);
    s = ajStrGetuniquePtr(&sstr);
    r = ajSeqGetSeqC(ref);

    for(i=0; i<lenref; i++)
	if(s[i] != '-')
	{
	    if(sub[ajSeqcvtGetCodeK(cvt, r[i])][ajSeqcvtGetCodeK(cvt, s[i])] <= 0)
		s[i] = tolower((int)s[i]);	/* dissimilar to lowercase */
	    else
		s[i] = toupper((int)s[i]);	/* similar to uppercase */
	}

    /* is seq longer than ref? */
    if(lenseq > lenref)
	for(; i<lenseq; i++)
	    if(s[i] != '-')
		s[i] = tolower((int)s[i]);	/* dissimilar to lowercase */;

    ajSeqAssignSeqS(seq, sstr);
    ajStrDel(&sstr);
    return;
}




/* @funcstatic showalign_MakeIdentity *****************************************
**
** Convert 'seq' to '.'s except where identical to 'ref'
**
** @param [r] ref [const AjPSeq] the reference sequence
** @param [u] seq [AjPSeq] the sequence to be changed
** @return [void]
** @@
******************************************************************************/

static void showalign_MakeIdentity(const AjPSeq ref, AjPSeq seq)
{

    ajint i;
    ajint lenseq;
    ajint lenref;

    AjPStr sstr = NULL;
    char *s;
    const char *r;

    sstr = ajSeqGetSeqCopyS(seq);
    lenseq = ajSeqGetLen(seq);
    lenref = ajSeqGetLen(ref);
    s = ajStrGetuniquePtr(&sstr);
    r = ajSeqGetSeqC(ref);



    for(i=0; i<lenref && s[i] != '\0'; i++)
	if(toupper((int)s[i]) != toupper((int)r[i]) && s[i] != '-')
	    s[i] = '.';

    /* is seq longer than ref? */
    if(lenseq > lenref)
	for(; i<lenseq; i++)
	    if(s[i] != '-')
		s[i] = '.';

    ajSeqAssignSeqS(seq, sstr);
    ajStrDel(&sstr);
    return;
}




/* @funcstatic showalign_MakeNonidentity **************************************
**
** Convert 'seq' to '.'s where identical to 'ref'
** Change case to lowercase where similar as given by scoring matrix.
** Change remaining dissimilar residues to uppercase.
**
** @param [r] ref [const AjPSeq] the reference sequence
** @param [u] seq [AjPSeq] the sequence to be changed
** @param [r] sub [ajint * const *] scoring matrix
** @param [r] cvt [const AjPSeqCvt] conversion table for scoring matrix
** @param [r] similarcase [AjBool] change case depending on similarity
** @return [void]
** @@
******************************************************************************/

static void showalign_MakeNonidentity(const AjPSeq ref,
				      AjPSeq seq, ajint * const *sub,
				      const AjPSeqCvt cvt, AjBool similarcase)
{
    ajint i;
    ajint lenseq;
    ajint lenref;

    AjPStr sstr = NULL;
    char *s;
    const char *r;

    sstr = ajSeqGetSeqCopyS(seq);
    lenseq = ajSeqGetLen(seq);
    lenref = ajSeqGetLen(ref);
    s = ajStrGetuniquePtr(&sstr);
    r = ajSeqGetSeqC(ref);


    for(i=0; i<lenref; i++)
	if(s[i] != '-')
	{
	    if(toupper((int)s[i]) == toupper((int)r[i]))
		s[i] = '.';
	    else
	    {
		/* change case based on similarity? */
		if(similarcase)
		{
		    if(sub[ajSeqcvtGetCodeK(cvt, r[i])][ajSeqcvtGetCodeK(cvt, s[i])] <= 0)
			s[i] = toupper((int)s[i]);
		    else
			s[i] = tolower((int)s[i]);
		}
	    }
	}


    /* Is seq longer than ref? Make it all upppercase */
    if(lenseq > lenref && similarcase)
	for(; i<lenseq; i++)
	    if(s[i] != '-')
		s[i] = toupper((int)s[i]);


    ajSeqAssignSeqS(seq, sstr);
    ajStrDel(&sstr);
    
    return;
}




/* @funcstatic showalign_MakeSimilar ******************************************
**
** Convert 'seq' to '.'s except where similar to 'ref', as defined by
** similarity scoring matrix.
** Change identities to upper-case, all others to lower-case.
**
** @param [r] ref [const AjPSeq] the reference sequence
** @param [u] seq [AjPSeq] the sequence to be changed
** @param [r] sub [ajint * const *] scoring matrix
** @param [r] cvt [const AjPSeqCvt] conversion table for scoring matrix
** @param [r] similarcase [AjBool] change case depending on similarity
** @return [void]
** @@
******************************************************************************/

static void showalign_MakeSimilar(const AjPSeq ref, AjPSeq seq,
				  ajint * const *sub,
				  const AjPSeqCvt cvt, AjBool similarcase)
{
    ajint i;
    ajint lenseq;
    ajint lenref;

    AjPStr sstr = NULL;
    char *s;
    const char *r;

    sstr = ajSeqGetSeqCopyS(seq);
    lenseq = ajSeqGetLen(seq);
    lenref = ajSeqGetLen(ref);
    s = ajStrGetuniquePtr(&sstr);
    r = ajSeqGetSeqC(ref);


    for(i=0; i<lenref; i++)
	if(s[i] != '-')
	{
	    if(sub[ajSeqcvtGetCodeK(cvt, r[i])][ajSeqcvtGetCodeK(cvt, s[i])] <= 0)
		s[i] = '.';
	    else
	    {
		/* change case based on similarity? */
		if(similarcase)
		{
		    if(toupper((int)s[i]) == toupper((int)r[i]))
			s[i] = toupper((int)s[i]);
		    else
			s[i] = tolower((int)s[i]);
		}
	    }
	}


    /* is seq longer than ref? */
    if(lenseq > lenref)
	for(; i<lenseq; i++)
	    if(s[i] != '-')
		s[i] = '.';

    ajSeqAssignSeqS(seq, sstr);
    ajStrDel(&sstr);
    return;
}




/* @funcstatic showalign_MakeDissimilar ***************************************
**
** Convert 'seq' to '.'s where identical or similar to 'ref', as defined by
** similarity scoring matrix.
**
** @param [r] ref [const AjPSeq] the reference sequence
** @param [u] seq [AjPSeq] the sequence to be changed
** @param [r] sub [ajint * const *] scoring matrix
** @param [r] cvt [const AjPSeqCvt] conversion table for scoring matrix
** @return [void]
** @@
******************************************************************************/

static void showalign_MakeDissimilar(const AjPSeq ref, AjPSeq seq,
				     ajint * const *sub,
				     const AjPSeqCvt cvt)
{
    ajint i;
    ajint lenref;

    AjPStr sstr = NULL;
    char *s;
    const char *r;

    sstr = ajSeqGetSeqCopyS(seq);
    lenref = ajSeqGetLen(ref);
    s = ajStrGetuniquePtr(&sstr);
    r = ajSeqGetSeqC(ref);

    for(i=0; i<lenref; i++)
	if(s[i] != '-')
	    if(sub[ajSeqcvtGetCodeK(cvt, r[i])][ajSeqcvtGetCodeK(cvt, s[i])] > 0)
		s[i] = '.';

    ajSeqAssignSeqS(seq, sstr);
    ajStrDel(&sstr);
    return;
}




/* @funcstatic showalign_Order ************************************************
**
** Orders the sequences
**
** @param [r] order [const AjPStr] how to order the sequences
** @param [r] seqs [AjPSeq const *] input sequences
** @param [r] consensus [const AjPSeq] consensus sequence
** @param [r] nrefseq [ajint] number of reference sequence
** @param [r] sub [ajint * const *] substitution matrix
** @param [r] cvt [const AjPSeqCvt] substitution conversion table
** @param [u] aorder [OOrder *] output order to display the sequences
** @return [void]
** @@
******************************************************************************/

static void showalign_Order(const AjPStr order,
			    AjPSeq const * seqs, const AjPSeq consensus,
			    ajint nrefseq, ajint * const *sub,
			    const AjPSeqCvt cvt,
			    OOrder *aorder)
{
    char orderchar;
    const AjPSeq ref;
    ajint i;
    ajint j;
    ajint k;
    const char *s;
    const char *r;
    ajint rlen;
    ajint len;

    orderchar = ajStrGetCharFirst(order); /* first char of 'order' */

    /* get the reference sequence */
    if(nrefseq == -1)
	ref = consensus;
    else
	ref = seqs[nrefseq];

    rlen = ajSeqGetLen(ref);

    /* initialise all order positions as unused */
    for(i=0; seqs[i]; i++)
	aorder[i].seq = NULL;


    /* do the ordering */
    switch(orderchar)
    {
    case 'I':				/* Input order */
	for(i=0, j=0; seqs[i]; i++)
	{
	    if(i != nrefseq)
		aorder[j++].seq = seqs[i];
	}
	break;

    case 'A':				/* Alphabetical name order */
	for(i=0, j=0; seqs[i]; i++)
	{
	    if(i != nrefseq)
		aorder[j++].seq = seqs[i];
	}

	/* sort alphabetically by name */
	qsort(aorder, j, sizeof(OOrder), showalign_CompareTwoSeqNames);
	break;

    case 'S':			/* Similarity to the reference sequence */
	if(nrefseq == -1)
	    r = ajSeqGetSeqC(consensus);
	else
	    r = ajSeqGetSeqC(seqs[nrefseq]);

	for(i=0, j=0; seqs[i]; i++)
	{
	    if(i != nrefseq)
	    {
		aorder[j].seq = seqs[i];
		aorder[j].similarity = 0;
		len = ajSeqGetLen(aorder[j].seq);
		s = ajSeqGetSeqC(aorder[j].seq);
		for(k=0; k<len && k<rlen; k++)
		    aorder[j].similarity += sub[ajSeqcvtGetCodeK(cvt,
			             r[k])][ajSeqcvtGetCodeK(cvt,s[k])];
		j++;
	    }
	}

	/* sort by similarity */
	qsort(aorder, j, sizeof(OOrder),
	      showalign_CompareTwoSeqSimilarities);
	break;

    default:
	ajFatal("Unknown option for '-order': %S", order[0]);
	break;
    }


    return;
}




/* @funcstatic showalign_Output ***********************************************
**
** Writes the sequences to the output file
**
** @param [u] outf [AjPFile] output file handle
** @param [r] seqs [AjPSeq const *] the sequences
** @param [r] nrefseq [ajint] number of the refseq
** @param [r] width [ajuint] width of displayed sequence line
** @param [r] margin [ajuint] width of margin on left side
** @param [r] consensus [const AjPSeq] consensus sequence
** @param [r] docon [AjBool] display consensus sequence at the bottom
** @param [r] bottom [AjBool] display refseq at the bottom of the alignment
**                            as well
** @param [u] aorder [const OOrder *] order to display the sequences
** @param [r] html [AjBool] format for html display
** @param [r] highlight [const AjPRange] ranges to highlight
** @param [r] uppercase [const AjPRange] ranges to uppercase
** @param [r] number [AjBool] display number line
** @param [r] ruler [AjBool] display ruler line
** @param [r] nseqs [ajuint] Number of sequences
** @param [r] begin [ajuint] Begin position
** @param [r] end [ajuint] End position
** @return [ajint] Always 0
** @@
******************************************************************************/

static ajint showalign_Output(AjPFile outf, AjPSeq const * seqs,
			      ajint nrefseq,
			      ajuint width, ajuint margin,
			      const AjPSeq consensus,
			      AjBool docon, AjBool bottom,
			      const OOrder* aorder,
			      AjBool html, const AjPRange highlight,
			      const AjPRange uppercase,
			      AjBool number, AjBool ruler,
			      ajuint nseqs, ajuint begin, ajuint end)
{
    ajuint pos;		/* start position in sequences of next line */
    ajuint i;
    const AjPSeq ref;			/* the reference sequence */

    ajuint firstpos = 0;
    ajuint lastpos = 0;
    AjPStr numline = NULL;
    AjPStr tickline = NULL;
    AjPStr showstr = NULL;
    ajuint istart = 0;
    ajuint iend = 0;
    ajuint imargin;

    /*
    **  if consensus line is the refseq, then aorder holds all the seqset
    **  sequences else it is one less than the seqset size
    */
    if(nrefseq != -1)
    {
	nseqs--;
	ref = seqs[nrefseq];
    }
    else
	ref = consensus;


    /* format for html display */
    if(html)
	ajFmtPrintF(outf, "<pre>\n");

    /* build the tick line for the consensus/reference sequence */
    tickline = showalign_OutputTicks(nrefseq, ref, &firstpos, &lastpos);

    /* build the number line for the consensus/reference sequence */
    numline = showalign_OutputNums(nrefseq, ref, firstpos, lastpos, margin);

    ajDebug("begin:%d end:%d width:%d\n",
           begin, end, width);
    ajDebug("first: %u last: %u len:%u %u %u\n",
           firstpos, lastpos, ajSeqGetLen(ref),
           ajStrGetLen(tickline), ajStrGetLen(numline));
    ajDebug("'%S'\n", numline);
    ajDebug("%*s'%S'\n", margin, "", tickline);

    /* get next set of lines to output */
    for(pos=begin; pos<=end; pos+=width)
    {
	/* numbers line */
	if(number)
        {
            imargin = margin;
            istart = pos+margin;
            while(istart && ajStrGetCharPos(numline, istart-1) == ' ')
            {
                istart--;
                if(imargin)
                    imargin--;
            }
            
            iend = pos+width+margin-1;
            if(iend < (end+margin))
            {
                while(iend < end && ajStrGetCharPos(numline, iend+1) != ' ')
                    iend++;
            }
            else
            {
                iend = end+margin;
            }
            
            ajStrAssignSubS(&showstr, numline, istart, iend);
            ajFmtPrintF(outf, "%*s%S\n", imargin, "", showstr);
        }
        
	/* ruler/ticks line */
        if(ruler)
        {
	    ajStrAssignSubS(&showstr, tickline, pos, pos+width-1);
	    ajFmtPrintF(outf, "%*s%S\n", margin, "", showstr);
        }
        
	/* refseq is always displayed at the top if it is not the consensus */
        if(nrefseq != -1) 
        {
	    showalign_OutputSeq(outf, ref, pos, end, width, margin, html,
			    highlight, uppercase);
	} 
	else if(docon && !bottom) 
	{
	    /* if refseq is consensus and docon is true,
               and bottom is false, display consensus at the top */
	    showalign_OutputSeq(outf, consensus, pos, end, width, margin,
			html, highlight, uppercase);
	}

	/* sequences */
	for(i=0; i<nseqs; i++)
	    showalign_OutputSeq(outf, aorder[i].seq, pos, end, width, margin,
				html, highlight, uppercase);


	/* consensus at the bottom */
        if(docon && bottom)
	    showalign_OutputSeq(outf, consensus, pos, end, width, margin,
			html, highlight, uppercase);



	/* blank line */
	ajFmtPrintF(outf, "\n");
    }

    /* format for html display */
    if(html)
	ajFmtPrintF(outf, "</pre>\n");

    ajStrDel(&showstr);
    ajStrDel(&tickline);
    ajStrDel(&numline);

    return 0;
}




/* @funcstatic showalign_OutputNums *******************************************
**
** Writes the numbers line
**
** @param [r] nrefseq [ajint] the sequence being displayed
**                            skip gaps if it is not -1 (consensus)
** @param [r] ref [const AjPSeq] Reference sequence
** @param [r] firstpos [ajuint] length of left hand margin
** @param [r] lastpos [ajuint] length of left hand margin
** @param [r] margin [ajuint] length of left hand margin
** @return [AjPStr] Numbers line with margin at start
** @@
******************************************************************************/

static AjPStr showalign_OutputNums(ajint nrefseq, const AjPSeq ref,
                                   ajuint firstpos, ajuint lastpos,
                                   ajuint margin)
{
    AjPStr line;
    ajuint i;
    ajuint j;
    const AjPStr refstr = ajSeqGetSeqS(ref);
    ajint iwidth;
    ajuint iend;
    char ch;
    AjPStr numstr = ajStrNewRes(10);

    iend = ajStrGetLen(refstr);
    
    line      = ajStrNewRes(iend+margin+1); /* line of numbers to print */

    if(nrefseq == -1)           /* simple consensus */
    {
        iwidth = margin+10;
        for(i=10; i<iend; i+=10)
        {
            ajFmtPrintAppS(&line, "%*d", iwidth, i);
            iwidth = 10;
        }
        
        iwidth = iend % 10;
    }
    else                        /* reference sequence */
    {
        iwidth = margin;
        j = firstpos;
        for(i=1; i<firstpos; i++)
        {
            j--;
            iwidth++;
            if (!(j % 10))
            {
                ajFmtPrintS(&numstr, "-%d", j);
                ajFmtPrintAppS(&line, "%*S", iwidth, numstr);
                iwidth = 0;
            }
        }

        j = 0;
        for(i=firstpos-1; i<lastpos; i++)
        {
            iwidth++;
            ch = ajStrGetCharPos(refstr, i);
            if(ch != '-')
            {
                j++;
                if (!(j % 10))
                {
                    ajFmtPrintAppS(&line, "%*d", iwidth, j);
                    iwidth = 0;
                }
            }
        }

        j = 0;
        for(i=lastpos; i<=iend; i++)
        {
            iwidth++;
            j++;
            if (!(j % 10))
            {
                ajFmtPrintS(&numstr, "+%d", j);
                ajFmtPrintAppS(&line, "%*S", iwidth, numstr);
                iwidth = 0;
            }
        }
    }

    if(iwidth)
        ajStrAppendCountK(&line, ' ', iwidth);

    ajStrDel(&numstr);

    return line;
}




/* @funcstatic showalign_OutputTicks ******************************************
**
** Writes the ticks line
**
** @param [r] nrefseq [ajint] the sequence being displayed
**                            skip gaps if it is not -1 (consensus)
** @param [r] ref [const AjPSeq] Reference sequence
** @param [w] firstpos [ajuint*] length of left hand margin
** @param [w] lastpos [ajuint*] length of left hand margin
** @return [AjPStr] Tick line matching reference sequence
** @@
******************************************************************************/

static AjPStr showalign_OutputTicks(ajint nrefseq, const AjPSeq ref,
                                    ajuint* firstpos, ajuint* lastpos)
{
    AjPStr line = NULL;
    const AjPStr refstr = ajSeqGetSeqS(ref);
    ajuint i;
    ajuint j;
    ajuint iend;
    ajuint ifirst = 0;
    ajuint ilast = 0;
    ajuint ipos = 0;
    const char* cp;
    char* cq;

    iend = ajStrGetLen(refstr);

    line = ajStrNewRes(1+iend);	/* line of ticks to print */

    /* make the ticks line */
    if(nrefseq == -1)
    {
        for(i=1; i<=iend; i++)
        {
            if(!(i % 10))
                ajStrAppendK(&line, '|');
            else if(!(i % 5))
                ajStrAppendK(&line, ':');
            else
                ajStrAppendK(&line, '-');
        }
        ifirst = 1;
        ilast = iend;
    }
    else
    {
        i = 0;
        ipos = 0;
        cp = ajStrGetPtr(refstr);
        while(*cp)
        {
            ipos++;
            if(*cp == '-')
                ajStrAppendK(&line, '.');
            else
            {
                i++;
                if(!(i % 10))
                    ajStrAppendK(&line, '|');
                else if(!(i % 5))
                    ajStrAppendK(&line, ':');
                else
                    ajStrAppendK(&line, '-');

                if(!ifirst) 
                    ifirst = ajStrGetLen(line);

                ilast = ipos;
            }

            cp++;           
        }
    }

    cq = ajStrGetuniquePtr(&line);
    j=0;
    if (ifirst > 1)
    {
        for(i = ifirst-2; i>0; i--)
        {
            j++;
            if(!(j%10))
                cq[i] = 'V';
            else if(!(j%5))
                cq[i] = 'v';
        }
    }

    if(ilast < iend)
    {
        j = 0;
        for(i=ilast; i<iend; i++)
        {
            j++;
            if(!(j%10))
                cq[i] = 'V';
            else if(!(j%5))
                cq[i] = 'v';
        }
    }

    *firstpos = ifirst;
    *lastpos = ilast;

    return line;
}




/* @funcstatic showalign_OutputSeq ********************************************
**
** Writes the specified sequence line to the output file
**
** @param [u] outf [AjPFile] output file handle
** @param [r] seq [const AjPSeq] the sequence to display
** @param [r] pos [ajint] position in sequence to start line
** @param [r] end [ajint] position to end sequence at
** @param [r] width [ajint] width of line
** @param [r] margin [ajint] length of left hand margin
** @param [r] html [AjBool] display formatted for html
** @param [r] highlight [const AjPRange] ranges to highlight
** @param [r] uppercase [const AjPRange] ranges to uppercase
** @return [ajint] Always 0
** @@
******************************************************************************/

static ajint showalign_OutputSeq(AjPFile outf, const AjPSeq seq, ajint pos,
				 ajint end, ajint width, ajint margin,
				 AjBool html, const AjPRange highlight,
				 const AjPRange uppercase)
{
    AjPStr line;
    AjPStr marginfmt;

    line = ajStrNewRes(margin+width+1); /* next line of sequence to print */
    marginfmt = ajStrNewRes(10);

    /* get end to display up to */
    if(end > pos+width-1)
	end = pos+width-1;

    /* the bit of the sequence to be output */
    ajStrAssignSubS(&line, ajSeqGetSeqS(seq), pos, end);

    /* name of sequence */
    ajFmtPrintS(&marginfmt, "%%-%d.%dS", margin, margin);
    if(margin > 0)
	ajFmtPrintF(outf, ajStrGetPtr(marginfmt), ajSeqGetNameS(seq));

    /* change required ranges to uppercase */
    embShowUpperRange(&line, uppercase, pos);

    /* +++ colour the sequences */

    /* highlight required ranges */
    if(html)
	embShowColourRange(&line, highlight, pos);

    /* print the sequence */
    ajFmtPrintF(outf, "%S\n", line);


    ajStrDel(&line);
    ajStrDel(&marginfmt);

    return 0;
}




/* @funcstatic showalign_CompareTwoSeqNames ***********************************
**
** Compare two Sequences' Names
**
** @param [r] a [const void *] First sequence
** @param [r] b [const void *] Second sequence
**
** @return [ajint] Compare value (-1, 0, +1)
** @@
******************************************************************************/

static ajint showalign_CompareTwoSeqNames(const void * a, const void * b)
{
    return strcmp(ajSeqGetNameC((*(OOrder const *)a).seq),
		  ajSeqGetNameC((*(OOrder const *)b).seq));
}




/* @funcstatic showalign_CompareTwoSeqSimilarities ****************************
**
** Compare two Sequences by their similarity to the reference sequence
**
** @param [r] a [const void *] First sequence similarity
** @param [r] b [const void *] Second sequence similarity
**
** @return [ajint] Compare value (-1, 0, +1)
** @@
******************************************************************************/

static ajint showalign_CompareTwoSeqSimilarities(const void * a,
						 const void * b)
{
    return (*(OOrder const *)b).similarity -
	(*(OOrder const *)a).similarity;
}

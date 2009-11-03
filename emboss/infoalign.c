/* @source infoalign application
**
** Display information on a multiple sequence alignment with consensus
**
** @author Copyright (C) Gary Williams (gwilliam@hgmp.mrc.ac.uk)
** 23 May 2001 - GWW - written
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
#include <ctype.h>




static int infoalign_Getrefseq(const AjPStr refseq, const AjPSeqset seqset);
static void infoalign_OutputFloat(AjPFile outfile, float num, AjBool html,
				   AjBool after);
static void infoalign_OutputInt(AjPFile outfile, ajint num, AjBool html,
				 AjBool after);
static void infoalign_OutputStr(AjPFile outfile, const AjPStr str, AjBool html,
				 AjBool after, ajuint minlength);
static void infoalign_Compare(const AjPSeq ref, const AjPSeq seq,
			      ajint *const *sub,
			      const AjPSeqCvt cvt, ajint *seqlength,
			      ajint *alignlength, ajint *gaps,
			      ajint *gapcount, ajint *idcount,
			      ajint *simcount, ajint *difcount, float *change);


#define NOLIMIT 0


/* @prog infoalign ************************************************************
**
** Information on a multiple sequence alignment
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPSeqset seqset = NULL;
    AjPStr refseq;	/* input name/number of reference sequence */
    ajint  nrefseq;	/* numeric reference sequence */
    AjPMatrix matrix;	/* scoring matrix structure */
    ajint **sub;	/* integer scoring matrix */
    AjPSeqCvt cvt = 0;	/* conversion table for scoring matrix */
    float identity;
    ajint ident;
    float fplural;
    AjPStr cons;
    AjPSeq consensus;

    const AjPSeq ref;
    const AjPSeq seq;
    ajuint i;

    AjBool html;
    AjBool doheader;
    AjBool dousa;
    AjBool doname;
    AjBool doseqlength;
    AjBool doalignlength;
    AjBool dogaps;
    AjBool dogapcount;
    AjBool doidcount;
    AjBool dosimcount;
    AjBool dodifcount;
    AjBool dochange;
    AjBool dodesc;
    AjBool dowt;

    ajint  seqlength;
    ajint  alignlength;
    ajint  gaps;
    ajint  gapcount;
    ajint  idcount;
    ajint  simcount;
    ajint  difcount;
    float  change;

    AjPFile outfile;

    const AjPStr usa;
    const AjPStr name;
    AjPStr altusa;			/* default name when the real name
					   is not known */
    AjPStr altname;

    AjPStr xxx = NULL;

    embInit("infoalign", argc, argv);


    seqset  = ajAcdGetSeqset("sequence");
    refseq  = ajAcdGetString("refseq");
    matrix  = ajAcdGetMatrix("matrix");

    ajSeqsetFill(seqset);

    outfile = ajAcdGetOutfile("outfile");

    html          = ajAcdGetBool("html");
    doheader      = ajAcdGetBool("heading");
    dousa         = ajAcdGetBool("usa");
    doname        = ajAcdGetBool("name");
    doseqlength   = ajAcdGetBool("seqlength");
    doalignlength = ajAcdGetBool("alignlength");
    dogaps        = ajAcdGetBool("gaps");
    dogapcount    = ajAcdGetBool("gapcount");
    doidcount     = ajAcdGetBool("idcount");
    dosimcount    = ajAcdGetBool("simcount");
    dodifcount    = ajAcdGetBool("diffcount");
    dochange      = ajAcdGetBool("change");
    dodesc        = ajAcdGetBool("description");
    dowt          = ajAcdGetBool("weight");

    /* consensus parameters */
    fplural   = ajAcdGetFloat("plurality");
    identity  = ajAcdGetFloat("identity");


    cons      = ajStrNew();
    consensus = ajSeqNew();
    altusa    = ajStrNewC("-");
    altname   = ajStrNewC("-");


    /* get conversion table and scoring matrix */
    cvt = ajMatrixCvt(matrix);
    sub = ajMatrixArray(matrix);

    /* get the number of the reference sequence */
    nrefseq = infoalign_Getrefseq(refseq, seqset);

    /* change the % plurality to the fraction of absolute total weight */
    fplural = ajSeqsetGetTotweight(seqset) * fplural / 100;

    /*
    ** change the % identity to the number of identical sequences at a
    ** position required for consensus
    */
    ident = ajSeqsetGetSize(seqset) * (ajint)identity / 100;

    /* get the consensus sequence */
    embConsCalc(seqset, matrix, ajSeqsetGetSize(seqset), ajSeqsetGetLen(seqset),
		 fplural, 0.0, ident, ajFalse, &cons);
    ajSeqAssignSeqS(consensus, cons);

    ajSeqAssignNameS(consensus,(xxx=ajStrNewC("Consensus")));

    /* get the reference sequence */
    if(nrefseq == -1)
	ref = consensus;
    else
	ref = ajSeqsetGetseqSeq(seqset, nrefseq);


    /* start the HTML table */
    if(html)
	ajFmtPrintF(outfile,"<table border cellpadding=4 bgcolor="
		    "\"#FFFFF0\">\n");

    /* print the header information */
    if(doheader)
    {
	/* start the HTML table title line and output the Name header */
	if(html)			
	    ajFmtPrintF(outfile, "<tr>");
	else
	    ajFmtPrintF(outfile, "%s", "# ");

	if(dousa)
	{
	    if(html)
		ajFmtPrintF(outfile, "<th>USA</th>");
	    else
		ajFmtPrintF(outfile, "%-16s", "USA");
	}

	if(doname)
	{
	    if(html)
		ajFmtPrintF(outfile, "<th>Name</th>");
	    else
		ajFmtPrintF(outfile, "%-12s", "Name");
	}

	if(doseqlength)
	{
	    if(html)
		ajFmtPrintF(outfile, "<th>Sequence Length</th>");
	    else
		ajFmtPrintF(outfile, "SeqLen\t");
	}

	if(doalignlength)
	{
	    if(html)
		ajFmtPrintF(outfile, "<th>Aligned Length</th>");
	    else
		ajFmtPrintF(outfile, "AlignLen\t");
	}

	if(dogaps)
	{
	    if(html)
		ajFmtPrintF(outfile, "<th>Gaps</th>");
	    else
		ajFmtPrintF(outfile, "Gaps\t");
	}

	if(dogapcount)
	{
	    if(html)
		ajFmtPrintF(outfile, "<th>Gap Length</th>");
	    else
		ajFmtPrintF(outfile, "GapLen\t");
	}

	if(doidcount)
	{
	    if(html)
		ajFmtPrintF(outfile, "<th>Identity</th>");
	    else
		ajFmtPrintF(outfile, "Ident\t");
	}

	if(dosimcount)
	{
	    if(html)
		ajFmtPrintF(outfile, "<th>Similarity</th>");
	    else
		ajFmtPrintF(outfile, "Similar\t");
	}

	if(dodifcount)
	{
	    if(html)
		ajFmtPrintF(outfile, "<th>Difference</th>");
	    else
		ajFmtPrintF(outfile, "Differ\t");
	}

	if(dochange)
	{
	    if(html)
		ajFmtPrintF(outfile, "<th>%% Change</th>");
	    else
		ajFmtPrintF(outfile, "%% Change\t");
	}

        if(dowt)
        {
            if(html)
                ajFmtPrintF(outfile, "<th>Weight</th>");
            else
                ajFmtPrintF(outfile, "Weight\t");
        }


	if(dodesc)
	{
	    if(html)
		ajFmtPrintF(outfile, "<th>Description</th>");
	    else
		ajFmtPrintF(outfile, "Description");
	}

	/* end the HTML table title line */
	if(html)
	    ajFmtPrintF(outfile, "</tr>\n");
	else
	    ajFmtPrintF(outfile, "\n");
    }


    for(i=0; i<ajSeqsetGetSize(seqset); i++)
    {
    	seq = ajSeqsetGetseqSeq(seqset, i);

	/* get the usa ('-' if unknown) */
	usa = ajSeqGetUsaS(seq);
	if(ajStrGetLen(usa) == 0)
	    usa = altusa;

	/* get the name ('-' if unknown) */
	name = ajSeqGetNameS(seq);
	if(ajStrGetLen(name) == 0)
	    name = altname;

	/* get the stats from the comparison to the reference sequence */
	infoalign_Compare(ref, seq, sub, cvt, &seqlength, &alignlength,
			  &gaps, &gapcount, &idcount, &simcount,
			  &difcount, &change);

	/* start table line */
	if(html)
	    ajFmtPrintF(outfile, "<tr>");

	if(dousa)
	    infoalign_OutputStr(outfile, usa, html,
				(dodesc || dowt || dochange ||
				 dodifcount || dosimcount ||
				 doidcount || dogapcount || dogaps ||
				 doseqlength || doalignlength || doname), 18);
	
	if(doname)
	    infoalign_OutputStr(outfile, name, html,
				(dodesc || dowt || dochange ||
				 dodifcount || dosimcount ||
				 doidcount || dogapcount || dogaps ||
				 doseqlength || doalignlength), 14);
	
	if(doseqlength)
	    infoalign_OutputInt(outfile, seqlength, html,
				(dodesc || dowt || dochange ||
				 dodifcount || dosimcount ||
				 doidcount || dogapcount ||
				 dogaps || doalignlength));
	
	if(doalignlength)
	    infoalign_OutputInt(outfile, alignlength, html,
				(dodesc || dowt || dochange ||
				 dodifcount || dosimcount ||
				 doidcount || dogapcount || dogaps));
	
	if(dogaps)
	    infoalign_OutputInt(outfile, gaps, html,
				(dodesc || dowt || dochange ||
				 dodifcount || dosimcount ||
				 doidcount || dogapcount));
	
	if(dogapcount)
	    infoalign_OutputInt(outfile, gapcount, html,
				(dodesc || dowt || dochange ||
				 dodifcount || dosimcount ||
				 doidcount));
	
	if(doidcount)
	    infoalign_OutputInt(outfile, idcount, html,
				(dodesc || dowt || dochange ||
                                 dodifcount || dosimcount));
	
	if(dosimcount)
	    infoalign_OutputInt(outfile, simcount, html, 
                                (dodesc || dowt || dochange ||
				 dodifcount));
	
	if(dodifcount)
	    infoalign_OutputInt(outfile, difcount, html, 
           		        (dodesc || dowt || dochange));
	
	if(dochange)
	    infoalign_OutputFloat(outfile, change, html, (dodesc || dowt) );
	
        if(dowt)
            infoalign_OutputFloat(outfile, ajSeqsetGetseqWeight(seqset,i), html, 
            	dodesc);

	if(dodesc)
	    infoalign_OutputStr(outfile, ajSeqGetDescS(seq), html, ajFalse, 
	    	NOLIMIT);
	
	/* end table line */
	if(html)
	    ajFmtPrintF(outfile, "</tr>\n");
	else
	    ajFmtPrintF(outfile, "\n");
    }
    
    
    /* end the HTML table */
    if(html)
	ajFmtPrintF(outfile, "</table>\n");
    
    ajFileClose(&outfile);
    
    /* tidy up */
    ajStrDel(&altusa);
    ajStrDel(&altname);
    ajStrDel(&xxx);
    ajSeqDel(&consensus);

    ajSeqsetDel(&seqset);
    ajStrDel(&refseq);
    ajMatrixDel(&matrix);
    ajStrDel(&cons);

    embExit();
    return 0;
}




/* @funcstatic infoalign_OutputFloat ******************************************
**
** Output a number.
** If this is not formatted for HTML, and is not the last item output
** then add a TAB after it, else ensure that the last data item has no
** characters after it.
**
** @param [u] outfile [AjPFile] output file handle
** @param [r] num [float] number to be printed
** @param [r] html [AjBool] True if want it formatted as an HTML table
**                          data item
** @param [r] after [AjBool] True if we want to output more columns after
**                           this one
** @return [void]
** @@
******************************************************************************/

static void infoalign_OutputFloat(AjPFile outfile, float num, AjBool html,
				  AjBool after)
{
    if(html)
	ajFmtPrintF(outfile, "<td>");

    ajFmtPrintF(outfile, "%f", num);

    if(html)
	ajFmtPrintF(outfile, "</td>\n");
    else
	if(after)
	    ajFmtPrintF(outfile, "\t");

    return;
}




/* @funcstatic infoalign_OutputInt ********************************************
**
** Output a number.
** If this is not formatted for HTML, and is not the last item output
** then add a TAB after it, else ensure that the last data item has no
** characters after it.
**
** @param [u] outfile [AjPFile] output file handle
** @param [r] num [ajint] number to be printed
** @param [r] html [AjBool] True if want it formatted as an HTML table
**                          data item
** @param [r] after [AjBool] True if we want to output more columns
**                           after this one
** @return [void]
** @@
******************************************************************************/

static void infoalign_OutputInt(AjPFile outfile, ajint num, AjBool html,
				AjBool after)
{
    if(html)
	ajFmtPrintF(outfile, "<td>");

    ajFmtPrintF(outfile, "%d", num);
    if(html)
	ajFmtPrintF(outfile, "</td>\n");
    else
	if(after)
	    ajFmtPrintF(outfile, "\t");

    return;
}




/* @funcstatic infoalign_OutputStr ********************************************
**
** Output the string.
** If this is not formatted for HTML, and is not the last item output
** then try to format it within a minimum length field.
** If it is longer than this field, just add a TAB.
**
** @param [u] outfile [AjPFile] output file handle
** @param [r] str [const AjPStr] string to be printed
** @param [r] html [AjBool] True if want it formatted as an HTML table
**                          data item
** @param [r] after [AjBool] True if we want to output more columns
**                           after this one
** @param [r] minlength [ajuint] minimum length of field to print the
**                              string in (0 (NOLIMIT) = no limit)
** @return [void]
** @@
******************************************************************************/

static void infoalign_OutputStr(AjPFile outfile, const AjPStr str, AjBool html,
				 AjBool after, ajuint minlength)
{
    AjPStr marginfmt;

    marginfmt = ajStrNewRes(10);

    /* ajFmtPrintF doesn't seem to deal with formats like "%-*S" correctly */
    ajFmtPrintS(&marginfmt, "%%-%dS", minlength);

    if(html)
	ajFmtPrintF(outfile, "<td>");

    if(html || !after || minlength == NOLIMIT)
	ajFmtPrintF(outfile, "%S", str);
    else
	/*
	**  Format:
	**  If this is the last item, don't put spaces or TABs after it.
	**  Try to fit the name in 'minlength' spaces, else just add a
	**  TAB after it
	*/
	ajFmtPrintF(outfile, ajStrGetPtr(marginfmt), str);

    if(html)
	ajFmtPrintF(outfile, "</td>\n");
    else
	if(after &&  ajStrGetLen(str) >= minlength)
	    ajFmtPrintF(outfile, "\t");

    ajStrDel(&marginfmt);

    return;
}




/* @funcstatic infoalign_Getrefseq ********************************************
**
** Determines which sequence should be the reference sequence.
** The first sequence in the set is returned as 0.
** -1 is returned as the consensus sequence.
**
** @param [r] refseq [const AjPStr] input name/number of reference sequence
** @param [r] seqset [const AjPSeqset] the sequences
** @return [int] the number of the reference sequence
** @@
******************************************************************************/

static int infoalign_Getrefseq(const AjPStr refseq, const AjPSeqset seqset)
{
    ajint i;
    const AjPSeq seq;

    for(i=0; i<(ajint)ajSeqsetGetSize(seqset); i++)
    {
	seq = ajSeqsetGetseqSeq(seqset, i);
	if(!ajStrCmpS(ajSeqGetNameS(seq), refseq))
	    return i;
    }

    /* not a name of a sequence, so it must be a number */
    if(!ajStrToInt(refseq, &i))
	ajFatal("Reference sequence is not a sequence ID or a number: %S",
		refseq);

    if(i < 0 || i > (ajint) ajSeqsetGetSize(seqset))
	ajFatal("Reference sequence number < 0 or > number "
		"of input sequences: %d", i);

    return i-1;
}




/* @funcstatic infoalign_Compare **********************************************
**
** Returns lots of trivial measures of comparison between a sequence and a
** reference sequence
**
** @param [r] ref [const AjPSeq] the reference sequence
** @param [r] seq [const AjPSeq] the sequence to be compared to 'ref'
** @param [r] sub [ajint * const *] scoring matrix
** @param [r] cvt [const AjPSeqCvt] conversion table for scoring matrix
** @param [w] seqlength [ajint *] sequence length
** @param [w] alignlength [ajint *] alignment length
** @param [w] gaps [ajint *] number of gaps
** @param [w] gapcount [ajint *] number of gap characters
** @param [w] idcount [ajint *] number of identical positions
** @param [w] simcount [ajint *] number of similar positions
** @param [w] difcount [ajint *] number of different positions
** @param [w] change [float *] % difference
** @return [void]
** @@
******************************************************************************/

static void infoalign_Compare(const AjPSeq ref, const AjPSeq seq,
			      ajint * const *sub,
			      const AjPSeqCvt cvt, ajint *seqlength,
			      ajint *alignlength, ajint *gaps,
			      ajint *gapcount, ajint *idcount,
			      ajint *simcount, ajint *difcount,
			      float *change)
{
    ajint i;
    ajint lenseq;
    ajint lenref;
    const char *s;
    const char *r;

    AjBool inGap = ajFalse;	/* true if in a gap in 'seq' */
    ajint begin;
    ajint end;

    lenseq = ajSeqGetLen(seq);
    lenref = ajSeqGetLen(ref);
    s = ajSeqGetSeqC(seq);
    r = ajSeqGetSeqC(ref);

    /* initialise counts */
    *seqlength   = 0;
    *alignlength = 0;
    *gaps     = 0;
    *gapcount = 0;
    *idcount  = 0;
    *simcount = 0;
    *difcount = 0;
    *change   = 0.0;

    /* ignore gaps at the ends of the sequence */
    for(begin = 0; s[begin] == '-'; begin++);
    for(end = lenseq-1; s[end] == '-'; end--);

    for(i=begin; i<=end; i++)
    {
	/* count gaps and their length */
	if(s[i] == '-')
	{
	    if(!inGap)
	    {
		inGap = ajTrue;
		(*gaps)++;
	    }
	    (*gapcount)++;
	}
	else
	{
	    inGap = ajFalse;

	    /*
	    ** count identity, similarity, differences
	    ** Past the end of the reference sequence ?
	    */
	    if(i >= lenref)
	        (*difcount)++;
	    else
	    {
	        /* identity */
	        if((toupper((int)r[i]) == toupper((int)s[i])))
		    (*idcount)++;
	        /* similarity */
	        else if(sub[ajSeqcvtGetCodeK(cvt, r[i])][ajSeqcvtGetCodeK(cvt, s[i])] > 0)
		    (*simcount)++;
	        /* difference */
	        else
		    (*difcount)++;
	    }
	}
    }

    *seqlength   = *idcount + *simcount + *difcount;
    *alignlength = end-begin+1;

    *change = (float)(*alignlength - *idcount)*(float)100.0/
	(float)(*alignlength);

    return;
}

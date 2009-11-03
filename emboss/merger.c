/* @source merger application
**
** Merge two sequences after a global alignment
** @author Copyright (C) Gary Williams (gwilliam@hgmp.mrc.ac.uk)
** @@
**
** Closely based on work by Alan Bleasby
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




static void merger_Merge(AjPAlign align, AjPStr *merged,
			 const char *a, const char *b,
			 const AjPStr m, const AjPStr n,
			 ajint start1,
			 ajint start2,
			 const char *namea, const char *nameb);

static float merger_quality(const char * seq, ajuint pos, ajuint window);

static AjBool merger_bestquality(const char * a, const char *b,
				 ajint apos, ajint bpos);




/* @prog merger ***************************************************************
**
** Merge two overlapping nucleic acid sequences
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPAlign align;
    AjPSeq a;
    AjPSeq b;
    AjPSeqout seqout;

    AjPStr m;
    AjPStr n;

    AjPStr merged = NULL;

    ajuint lena;
    ajuint lenb;

    const char   *p;
    const char   *q;

    ajint start1 = 0;
    ajint start2 = 0;

    float *path;
    ajint *compass;

    AjPMatrixf matrix;
    AjPSeqCvt  cvt = 0;
    float **sub;

    float gapopen;
    float gapextend;
    ajulong maxarr = 1000;
    ajulong len;		  /* arbitrary. realloc'd if needed */
    size_t  stlen;

    float score;
    ajint begina;
    ajint beginb;

    embInit("merger", argc, argv);

    a         = ajAcdGetSeq("asequence");
    b         = ajAcdGetSeq("bsequence");
    seqout    = ajAcdGetSeqout("outseq");
    matrix    = ajAcdGetMatrixf("datafile");
    gapopen   = ajAcdGetFloat("gapopen");
    gapextend = ajAcdGetFloat("gapextend");
    align     = ajAcdGetAlign("outfile");

    gapopen = ajRoundF(gapopen, 8);
    gapextend = ajRoundF(gapextend, 8);

    AJCNEW(path, maxarr);
    AJCNEW(compass, maxarr);

    /*
    **  make the two sequences lowercase so we can show which one we are
    **  using in the merge by uppercasing it
    */

    ajSeqFmtLower(a);
    ajSeqFmtLower(b);

    m = ajStrNew();
    n = ajStrNew();

    sub = ajMatrixfArray(matrix);
    cvt = ajMatrixfCvt(matrix);

    begina = ajSeqGetBegin(a);
    beginb = ajSeqGetBegin(b);

    lena = ajSeqGetLen(a);
    lenb = ajSeqGetLen(b);

    if(lenb > (ULONG_MAX/(ajulong)(lena+1)))
	ajFatal("Sequences too big. Try 'supermatcher'");

    len  = lena*lenb;

    if(len>maxarr)
    {

	ajDebug("merger: resize path, len to %d (%d * $d)\n",
		len, lena, lenb);

	stlen = (size_t) len;
        AJCRESIZE(path,stlen);
        AJCRESIZE(compass,stlen);
        maxarr=len;
    }


    p = ajSeqGetSeqC(a);
    q = ajSeqGetSeqC(b);

    ajStrAssignC(&m,"");
    ajStrAssignC(&n,"");

    score = embAlignPathCalc(p,q,lena,lenb,gapopen,gapextend,path,sub,cvt,
		     compass, ajFalse);

    /*score = embAlignScoreNWMatrix(path,compass,gapopen,gapextend,
                                  a,b,lena,lenb,sub,cvt,
				  &start1,&start2);*/

    embAlignWalkNWMatrix(path,a,b,&m,&n,lena,lenb, &start1,&start2,gapopen,
			 gapextend,compass);

    /*
    ** now construct the merged sequence, uppercase the bits of the two
    ** input sequences which are used in the merger
    */
    merger_Merge(align, &merged,p,q,m,n,start1,start2,
		 ajSeqGetNameC(a),ajSeqGetNameC(b));

    embAlignReportGlobal(align, a, b, m, n,
			 start1, start2, gapopen, gapextend,
			 score, matrix, begina, beginb);

    ajAlignWrite(align);
    ajAlignReset(align);

    /* write the merged sequence */
    ajSeqAssignSeqS(a, merged);
    ajSeqoutWriteSeq(seqout, a);
    ajSeqoutClose(seqout);
    ajSeqoutDel(&seqout);

    ajSeqDel(&a);
    ajSeqDel(&b);

    ajAlignClose(align);
    ajAlignDel(&align);
    ajStrDel(&merged);

    AJFREE(compass);
    AJFREE(path);

    ajStrDel(&n);
    ajStrDel(&m);

    embExit();

    return 0;
}




/* @funcstatic merger_Merge ***************************************************
**
** Print a global alignment
** Nucleotides or proteins as needed.
**
** @param [w] align [AjPAlign] Alignment object
** @param [w] ms [AjPStr *] output merged sequence
** @param [r] a [const char *] complete first sequence
** @param [r] b [const char *] complete second sequence
** @param [r] m [const AjPStr] Walk alignment for first sequence
** @param [r] n [const AjPStr] Walk alignment for second sequence
** @param [r] start1 [ajint] start of alignment in first sequence
** @param [r] start2 [ajint] start of alignment in second sequence
** @param [r] namea [const char *] name of first sequence
** @param [r] nameb [const char *] name of second sequence
**
** @return [void]
******************************************************************************/

static void merger_Merge(AjPAlign align, AjPStr *ms,
			 const char *a, const char *b,
			 const AjPStr m, const AjPStr n, ajint start1,
			 ajint start2,
			 const char *namea, const char *nameb)
{
    ajint apos;
    ajint bpos;
    ajint i;
    AjPStr mm = NULL;
    AjPStr nn = NULL;

    char *p;
    char *q;

    ajint olen;				/* length of walk alignment */
    size_t tt;
    
    /* lengths of the sequences after the aligned region */
    ajint alen;
    ajint blen;
    AjPStr tmpstr = NULL;

    mm = ajStrNewS(m);
    nn = ajStrNewS(n);

    p    = ajStrGetuniquePtr(&mm);
    q    = ajStrGetuniquePtr(&nn);
    olen = ajStrGetLen(mm);

    /* output the left hand side */
    if(start1 > start2)
    {
	for(i=0; i<start1; i++)
	    ajStrAppendK(ms, a[i]);

	if(start2)
	{
	    ajFmtPrintAppS(&tmpstr, "WARNING: *************************"
			   "********\n");
	    ajFmtPrintAppS(&tmpstr, "The region of alignment only starts at "
			   "position %d of sequence %s\n", start2+1, nameb);
	    ajFmtPrintAppS(&tmpstr, "Only the sequence of %s is being used "
			   "before this point\n\n", namea);
	    ajAlignSetTailApp(align, tmpstr);
	    ajStrDel(&tmpstr);
	}
    }
    else if(start2 > start1)
    {
	for(i=0; i<start2; i++)
	    ajStrAppendK(ms, b[i]);

	if(start1)
	{
	    ajFmtPrintAppS(&tmpstr, "WARNING: **************************"
			   "*******\n");
	    ajFmtPrintAppS(&tmpstr, "The region of alignment only starts at "
			   "position %d of sequence %s\n", start1+1, namea);
	    ajFmtPrintAppS(&tmpstr, "Only the sequence of %s is being used "
			   "before this point\n\n", nameb);
	    ajAlignSetTailApp(align, tmpstr);
	    ajStrDel(&tmpstr);
	}
    }
    else if(start1 && start2)
    {
	/* both the same length and > 1 before the aligned region */
	ajFmtPrintAppS(&tmpstr,
			      "WARNING: *********************************\n");
	ajFmtPrintAppS(&tmpstr, "There is an equal amount of unaligned "
		       "sequence (%d) at the start of the sequences.\n",
		       start1);
	ajFmtPrintAppS(&tmpstr, "Sequence %s is being arbitrarily chosen "
		       "for the merged sequence\n\n", namea);
	ajAlignSetTailApp(align, tmpstr);
	ajStrDel(&tmpstr);

	for(i=0; i<start1; i++)
	    ajStrAppendK(ms, a[i]);
    }

    /* header */
    ajFmtPrintS(&tmpstr, "Conflicts: %15.15s %15.15s\n", namea, nameb);
    ajFmtPrintAppS(&tmpstr,
		   "             position base   position base Using\n");
    ajAlignSetTailApp(align, tmpstr);

    /* make the merged sequence
    **
    **  point to the start of the alignment in the complete unaligned
    **  sequences
    */
    apos = start1;
    bpos = start2;

    for(i=0; i<olen; i++)
    {
	if(p[i]=='.' || p[i]==' ' || p[i]=='-' ||
	   q[i]=='.' || q[i]==' ' || q[i]=='-')
	{				/* gap! */
	    if(merger_bestquality(a, b, apos, bpos))
	    {
		p[i] = toupper((ajint)p[i]);
		if(p[i] != '.' && p[i] != ' ' && p[i] != '-')
		    ajStrAppendK(ms, p[i]);
		ajFmtPrintS(&tmpstr,
			    "             %8d  '%c'   %8d  '%c'   '%c'\n",
			    apos+1, p[i], bpos+1, q[i], p[i]);
		ajAlignSetTailApp(align, tmpstr);
	    }
	    else
	    {
		q[i] = toupper((ajint)q[i]);
		if(q[i] != '.' && q[i] != ' ' && q[i] != '-')
		    ajStrAppendK(ms, q[i]);
		ajFmtPrintS(&tmpstr,
			    "             %8d  '%c'   %8d  '%c'   '%c'\n",
			    apos+1, p[i], bpos+1, q[i], q[i]);
		ajAlignSetTailApp(align, tmpstr);
	    }

	}
	else if(p[i]=='n' || p[i]=='N')
	{
	    q[i] = toupper((ajint)q[i]);
	    ajStrAppendK(ms, q[i]);
	}
	else if(q[i]=='n' || q[i]=='N')
	{
	    p[i] = toupper((ajint)p[i]);
	    ajStrAppendK(ms, p[i]);
	}
	else if(p[i] != q[i])
	{
	    /*
	    **  get the sequence with the best quality and use the base
	    **  from that one
	    */
	    if(merger_bestquality(a, b, apos, bpos))
	    {
		p[i] = toupper((ajint)p[i]);
		ajStrAppendK(ms, p[i]);
		ajFmtPrintS(&tmpstr,
			    "             %8d  '%c'   %8d  '%c'   '%c'\n",
			    apos+1, p[i], bpos+1, q[i], p[i]);
		ajAlignSetTailApp(align, tmpstr);
	    }
	    else
	    {
		q[i] = toupper((ajint)q[i]);
		ajStrAppendK(ms, q[i]);
		ajFmtPrintS(&tmpstr,
			    "             %8d  '%c'   %8d  '%c'   '%c'\n",
			    apos+1, p[i], bpos+1, q[i], q[i]);
		ajAlignSetTailApp(align, tmpstr);
	    }

	}
	else
	    ajStrAppendK(ms, p[i]);

	/* update the positions in the unaligned complete sequences */
	if(p[i] != '.' &&  p[i] != ' ' &&  p[i] != '-') apos++;
	if(q[i] != '.' &&  q[i] != ' ' &&  q[i] != '-') bpos++;
    }

    /* output the right hand side */
    tt = strlen(&a[apos]);
    alen = (ajint) tt;

    tt = strlen(&b[bpos]);
    blen = (ajint) tt;
    
    if(alen > blen)
    {
	ajStrAppendC(ms, &a[apos]);
	if(blen)
	{
	    ajFmtPrintAppS(&tmpstr, "WARNING: ***************************"
			   "******\n");
	    ajFmtPrintAppS(&tmpstr, "The region of alignment ends at "
			   "position %d of sequence %s\n",
			   bpos+1, nameb);
	    ajFmtPrintAppS(&tmpstr, "Only the sequence of %s is being used "
			   "after this point\n\n", namea);
	    ajAlignSetTailApp(align, tmpstr);
	    ajStrDel(&tmpstr);
	}

    }

    if(blen > alen)
    {
	ajStrAppendC(ms, &b[bpos]);
	if(alen)
	{
	    ajFmtPrintAppS(&tmpstr, "WARNING: ***************************"
			   "******\n");
	    ajFmtPrintAppS(&tmpstr, "The region of alignment ends at "
			   "position %d of sequence %s\n",
			   apos+1, namea);
	    ajFmtPrintAppS(&tmpstr, "Only the sequence of %s is being used "
			   "after this point\n\n", nameb);
	    ajAlignSetTailApp(align, tmpstr);
	    ajStrDel(&tmpstr);
	}
    }
    else if(alen && blen)
    {	/* both the same length and > 1 */
	ajFmtPrintAppS(&tmpstr, "WARNING: ************************"
		       "*********\n");
	ajFmtPrintAppS(&tmpstr, "There is an equal amount of unaligned "
		       "sequence (%d) at the end of the sequences.\n",
		       alen);
	ajFmtPrintAppS(&tmpstr, "Sequence %s is being arbitrarily chosen "
		       "for the merged sequence\n\n", namea);
	ajStrAppendC(ms, &a[apos]);
	ajAlignSetTailApp(align, tmpstr);
	ajStrDel(&tmpstr);
    }

    ajStrDel(&mm);
    ajStrDel(&nn);
    ajStrDel(&tmpstr);

    return;
}




/* @funcstatic merger_bestquality *********************************************
**
** Return ajTrue if the first sequence has the best quality
** If both sequences have the same quality, pick the first
**
** @param [r] a [const char*] First sequence
** @param [r] b [const char*] Second sequence
** @param [r] apos [ajint] Position in first sequence
** @param [r] bpos [ajint] Position in second sequence
** @return [AjBool] ajTrue = first sequence is the best quality at this point
**
******************************************************************************/

static AjBool merger_bestquality(const char * a, const char *b,
				 ajint apos, ajint bpos)
{
    float qa;
    float qb;

    qa = merger_quality(a, apos, 5);
    qb = merger_quality(b, bpos, 5);

    if(qa == qb)
    {
	/* both have the same quality, use a larger window */
	qa = merger_quality(a, apos, 10);
	qb = merger_quality(b, bpos, 10);
    }

    if(qa == qb)
    {
	/* both have the same quality, use a larger window */
	qa = merger_quality(a, apos, 20);
	qb = merger_quality(b, bpos, 20);
    }

    ajDebug("merger_bestquality %d..%d = %.3f %.3f\n", apos, bpos, qa, qb);

    if(qa >= qb)
	/*  both have the same quality, use the first sequence */
	return ajTrue;

    return ajFalse;
}




/* @funcstatic merger_quality *************************************************
**
** Calculate the quality of a window of a sequence
**
** quality = sequence value/length under window either side of a position
**
** sequence value = sum of points in that subsequence
**
** good bases = 2 points
**
** ambiguous bases = 1 point
**
** N's = 0 points
**
** off end of the sequence = 0 points
**
** THIS HEAVILY DISCRIMINATES AGAINST THE IFFY BITS AT THE END OF
** SEQUENCE READS
**
** @param [r] seq [const char*] Sequence
** @param [r] pos [ajuint] Position
** @param [r] window [ajuint] Window size
** @return [float] quality of the window
**
******************************************************************************/

static float merger_quality(const char * seq, ajuint pos, ajuint window)
{
    ajint value = 0;
    ajuint i;
    ajint j;
    ajint jlast;
    float tf;
    
    for(i=pos; i<pos+window && i < strlen(seq); i++)
	if(strchr("aAcCgGtTuU", seq[i]))
	    /* good bases count for two points */
	    value+=2;
	else if(strchr("mrwsykvhdbMRWSYKVHDB", seq[i]))
	    /* ambiguous bases count for only one point */
	    value++;
    jlast = pos-window;
    for(j=pos-1; j>jlast && j>=0; j--)
	if(strchr("aAcCgGtTuU", seq[j]))
	    /* good bases count for two points */
	    value+=2;
	else if(strchr("mrwsykvhdbMRWSYKVHDB", seq[j]))
	    /* ambiguous bases count for only one point */
	    value++;

    tf = (float) ((double)value/((double)window*2.+1.));
    
    return tf;
}


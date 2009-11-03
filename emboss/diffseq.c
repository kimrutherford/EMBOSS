/* @source diffseq application
**
** Find differences (SNPs) between nearly identical sequences
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




/* @datastatic CdsPval ****************************************************
**
** CDS information object
**
** @alias CdsSval
** @alias CdsOval
**
** @attr Start [ajint] Start of CDS (always less than End)
** @attr End [ajint] End of CDS (always greater than Start)
** @attr Phase [ajint] Phase of translation (0,1 or 2)
** @attr Parent [AjBool] ajTrue is this CDS is a parent of a forward sense join
**			or the last CDS of a reverse sense join
** @attr Single [AjBool] ajTrue is this CDS is a not member of a join()
** @attr ReverseParent [AjBool] ajTrue is this CDS is a parent of rev sense join
**			or the last CDS in a forward sense join
** @attr Local [AjBool] ajTrue is this CDS is local
** @attr Sense [char] Sense '+' or '-'
** @attr Padding [char[3]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct CdsSval
{
    ajint  Start;
    ajint  End;
    ajint  Phase;
    AjBool Parent;
    AjBool Single;
    AjBool ReverseParent;
    AjBool Local;
    char   Sense;
    char Padding[3];
} CdsOval;
#define CdsPval CdsOval*

/* @datastatic PosPDiff ****************************************************
**
** Position of a difference between two matching regions
** If there is something inserted in one sequence that does not
** occur in the other, then the other has Start after the gap and
** End before the gap and Len=0.
**
** @alias PosSDiff
** @alias PosODiff
**
** @attr Start1 [ajint] Start of difference in sequence 1 (End1+1 if gap)
** @attr Start2 [ajint] Start of difference in sequence 2 (End1+1 if gap)
** @attr End1 [ajint] End of difference in seq 1 
** @attr End2 [ajint] End of difference in seq 2
** @attr Len1 [ajint] Length of difference in sequence 1 (0 = a gap)
** @attr Len2 [ajint] Length of difference in sequence 2 (0 = a gap)
** @@
******************************************************************************/

typedef struct PosSDiff
{
    ajint  Start1;
    ajint  Start2;
    ajint  End1;
    ajint  End2;
    ajint  Len1;
    ajint  Len2;
} PosODiff;
#define PosPDiff PosODiff*

  
static void diffseq_Diff(const AjPList difflist,
			    const AjPSeq seq1, const AjPSeq seq2,
			    AjPReport report, AjPFeattable ftab,
			    ajint over1start, ajint over1end,
			    ajint over2start, ajint over2end);

static void diffseq_WordMatchListConvDiffToFeat(const AjPList list,
                                                AjPFeattable *tab1,
                                                AjPFeattable *tab2,
                                                const AjPSeq seq1,
                                                const AjPSeq seq2);

static void diffseq_Features(const char* typefeat, AjPFeature rf,
				const AjPFeattable feat,
				ajuint start, ajuint end);

static void diffseq_AddTags(AjPStr* strval, const AjPFeature feat,
			       AjBool values);

static void diffseq_DiffList(const AjPList matchlist, AjPList difflist,
                             AjBool global, const AjPSeq seq1,
                             const AjPSeq seq2, ajint *over1start,
                             ajint *over1end, ajint *over2start,
                             ajint *over2end);

static PosPDiff diffseq_PosPDiffNew(void);

static void diffseq_PosPDiffDel(void **x, void *cl);

static void diffseq_FeatSetCDSFrame(AjPFeattable ftab);


/* @prog diffseq **************************************************************
**
** Find differences (SNPs) between nearly identical sequences
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeq seq1;
    AjPSeq seq2;
    ajint wordlen;
    AjPTable seq1MatchTable = 0;
    AjPList matchlist = NULL;
    AjPList difflist = NULL;
    AjPReport report;
    AjPFeattable Tab1 = NULL;
    AjPFeattable Tab2 = NULL;
    AjPFeattable TabRpt   = NULL;
    AjPFeattabOut seq1out = NULL;
    AjPFeattabOut seq2out = NULL;
    AjPStr tmpstr=NULL;
    AjBool global;    /* true if want to report the differences at the ends */
    ajint over1start;	/* start and end positions of match overlap */
    ajint over1end;	        /* ... for sequence 1 */
    ajint over2start = 0;	/* overlap for sequence 2 */
    ajint over2end;

                         
    embInit("diffseq", argc, argv);

    wordlen = ajAcdGetInt("wordsize");
    seq1    = ajAcdGetSeq("asequence");
    seq2    = ajAcdGetSeq("bsequence");
    report  = ajAcdGetReport("outfile");
    seq1out = ajAcdGetFeatout("aoutfeat");
    seq2out = ajAcdGetFeatout("boutfeat");

    /* advanced qualifiers */
    global        = ajAcdGetBool("globaldifferences");
		    

    ajSeqTrim(seq1);
    ajSeqTrim(seq2);

    TabRpt = ajFeattableNewSeq(seq1);

    embWordLength(wordlen);
    if(embWordGetTable(&seq1MatchTable, seq1))
	/* get table of words */
	matchlist = embWordBuildMatchTable(seq1MatchTable, seq2, ajTrue);


    /* get the minimal set of overlapping matches */
    if(matchlist)
	embWordMatchMin(matchlist);


    if(matchlist)
    {
        /* convert the list of matches to a list of differences */
        difflist = ajListNew();
        diffseq_DiffList(matchlist, difflist, global, seq1, seq2,
                         &over1start, &over1end, &over2start, &over2end);


	/* output the gff files */
	diffseq_WordMatchListConvDiffToFeat(difflist, &Tab1, &Tab2,
						   seq1, seq2);


	diffseq_Diff(difflist, seq1, seq2, report, TabRpt,
	             over1start, over1end, over2start, over2end);


	embWordMatchListDelete(&matchlist); /* free the match structures */

        /* delete difflist */
        ajListMap(difflist, diffseq_PosPDiffDel, NULL);
        ajListFree(&difflist);

    }

    ajFeatWrite(seq1out, Tab1);
    ajFeatWrite(seq2out, Tab2);

    tmpstr = ajStrNew();
    ajFmtPrintS(&tmpstr, "Feature file for first sequence");
    ajReportFileAdd(report, ajFeattabOutFile(seq1out), tmpstr);

    ajFmtPrintS(&tmpstr, "Feature file for second sequence");
    ajReportFileAdd(report, ajFeattabOutFile(seq2out), tmpstr);

    ajReportWrite(report, TabRpt, seq1);
    ajReportClose(report);

    /* tidy up */
    ajSeqDel(&seq1);
    ajSeqDel(&seq2);
    embWordFreeTable(&seq1MatchTable);
    ajReportDel(&report);
    ajListFree(&matchlist);
    ajListFree(&difflist);
    ajFeattableDel(&Tab1);
    ajFeattableDel(&Tab2);
    ajFeattableDel(&TabRpt);
    ajFeattabOutDel(&seq1out);
    ajFeattabOutDel(&seq2out);
    ajStrDel(&tmpstr);


    embExit();
    return 0;
}




/* @funcstatic diffseq_Diff ************************************************
**
** Do a diff and build a report on the diff of the two sequences.
**
** @param [r] difflist [const AjPList] List of differences
** @param [r] seq1 [const AjPSeq] Sequence to be diff'd.
** @param [r] seq2 [const AjPSeq] Sequence to be diff'd.
** @param [u] report [AjPReport] Report object.
** @param [u] ftab [AjPFeattable] Report feature table
** @param [r] over1start [ajint] start of overlap region in sequence1
** @param [r] over1end [ajint] end of overlap region in sequence1
** @param [r] over2start [ajint] start of overlap region in sequence2
** @param [r] over2end [ajint] end of overlap region in sequence2
** @return [void]
** @@
******************************************************************************/

static void diffseq_Diff(const AjPList difflist,
			    const AjPSeq seq1, const AjPSeq seq2,
			    AjPReport report, AjPFeattable ftab,
			    ajint over1start, ajint over1end,
			    ajint over2start, ajint over2end)
{
    AjIList iter = NULL;	/* match list iterator */
    PosPDiff diff = NULL;	/* difference structure */
    const AjPStr s1;		/* string of seq1 */
    const AjPStr s2;		/* string of seq2 */
    AjPStr tmp;			/* temporary string */

    /* stuff for counting SNPs, transitions & transversions */
    ajint snps = 0;
    ajint transitions   = 0;
    ajint transversions = 0;	/* counts of SNP types */
    char base1 = '\0';
    char base2 = '\0';
    AjPStr tmpstr = NULL;
    static AjPStr tmpseq = NULL;

    AjPFeattable feat1 = NULL;
    AjPFeattable feat2 = NULL;
    
    AjPFeature gf = NULL;
    


    tmpstr = ajStrNew();
    tmp    = ajStrNew();


    s1 = ajSeqGetSeqS(seq1);
    s2 = ajSeqGetSeqS(seq2);
    
    feat1 = ajSeqGetFeatCopy(seq1);
    feat2 = ajSeqGetFeatCopy(seq2);

    /* 
    ** Fix the Frame field in the CDS features of the feature table for
    ** seq1. These are often left as 'unsure' by the ajfeat routines.
    */
    diffseq_FeatSetCDSFrame(feat1);

    /* create report header */
    ajFmtPrintS(&tmpstr, "Compare: %S     from: %d   to: %d\n\n",
		ajReportSeqName(report, seq2),
		ajSeqGetBegin(seq2), ajSeqGetEnd(seq2));    
    if (over1start != -1)   
    { 
        ajFmtPrintAppS(&tmpstr, "%S overlap starts at %d\n",
			   ajReportSeqName(report, seq1),
			   over1start);
        ajFmtPrintAppS(&tmpstr, "%S overlap starts at %d\n\n",
			   ajReportSeqName(report, seq2),
			   over2start);
    }
    ajReportSetHeader(report, tmpstr);


    iter = ajListIterNewread(difflist);
    while(!ajListIterDone(iter))
    {
	diff = (PosPDiff) ajListIterGet(iter) ;

        /* seq 1 details */
        if(diff->Len1 > 0)
	{
	    gf = ajFeatNewII(ftab, diff->Start1, diff->End1);
	    ajStrAssignSubS(&tmp, s1, diff->Start1-1, diff->End1-1);
            base1 = * ajStrGetPtr(tmp);
	}
        else
        {
            gf = ajFeatNewII(ftab, diff->End1, diff->End1-1);
            ajStrAssignC(&tmp, "");
        }
        diffseq_Features("first_feature", gf,
                            feat1, diff->Start1, diff->End1);

        /* seq2 details */
        diffseq_Features("second_feature", gf,
                            feat2, diff->Start2, diff->End2);

        ajFmtPrintS(&tmp, "*name %S", ajReportSeqName(report, seq2));
        ajFeatTagAdd(gf, NULL, tmp);

        if(diff->Len2 > 0)
        {
            ajFmtPrintS(&tmp, "*length %d", diff->Len2);
            ajFeatTagAdd(gf, NULL, tmp);
            ajFmtPrintS(&tmp, "*start %d", diff->Start2);
            ajFeatTagAdd(gf, NULL, tmp);
            ajFmtPrintS(&tmp, "*end %d", diff->End2);
            ajFeatTagAdd(gf, NULL, tmp);
            ajStrAssignSubS(&tmpseq, s2, diff->Start2-1, diff->End2-1);
            ajFmtPrintS(&tmp, "*sequence %S", tmpseq);
            ajFeatTagAdd(gf, NULL, tmp);
            base2 = * ajStrGetPtr(tmpseq);
        }
        else
        {
            ajFmtPrintS(&tmp, "*length %d", diff->Len2);
            ajFmtPrintS(&tmp, "*start %d", diff->End2);
            ajFeatTagAdd(gf, NULL, tmp);
            ajFmtPrintS(&tmp, "*end %d", diff->End2);
            ajFeatTagAdd(gf, NULL, tmp);
        }

        /* count SNPs, transitions & transversions */
        if(diff->Len1 == 1 && diff->Len2 == 1)
        {
            snps++;
            transitions += (ajint) embPropTransition(base1, base2);
            transversions += (ajint) embPropTransversion(base1, base2);
        }

    }
    
    ajListIterDel(&iter);
    
    /* create report tail */
    if(over1start != -1)
    {
        ajFmtPrintS(&tmp, "Overlap_end: %d in %S\n",
                    over1end,
                    ajReportSeqName(report, seq1));
        ajFmtPrintAppS(&tmp, "Overlap_end: %d in %S\n",
                       over2end,
                       ajReportSeqName(report, seq2));
        if(ajSeqIsNuc(seq1))
        {
            ajFmtPrintAppS(&tmp, "\n");
            ajFmtPrintAppS(&tmp, "SNP_count: %d\n", snps);
            ajFmtPrintAppS(&tmp, "Transitions: %d\n", transitions);
            ajFmtPrintAppS(&tmp, "Transversions: %d\n", transversions);
        }
    }
    else
        /* no iterations of the match list done - ie no matches */
        ajFmtPrintS(&tmp, "No regions of alignment found.\n");
    
    ajReportSetTail(report, tmp);
    

    ajStrDel(&tmp);
    ajFeattableDel(&feat1);
    ajFeattableDel(&feat2);
    
    ajStrDel(&tmpstr);
    ajStrDel(&tmpseq);
    
    return;
}

/* @funcstatic diffseq_WordMatchListConvDiffToFeat ****************************
**
** Convert the differences list to feature tables for output.
**
** @param [r] list [const AjPList] differences list
** @param [u] tab1 [AjPFeattable*] feature table for sequence 1
** @param [u] tab2 [AjPFeattable*] feature table for sequence 2
** @param [r] seq1 [const AjPSeq] sequence 1
** @param [r] seq2 [const AjPSeq] sequence 2
** @return [void]
** @@
******************************************************************************/

static void diffseq_WordMatchListConvDiffToFeat(const AjPList list,
                                                AjPFeattable *tab1,
                                                AjPFeattable *tab2,
                                                const AjPSeq seq1,
                                                const AjPSeq seq2)
{
    char strand = '+';
    ajint frame = 0;
    AjPStr source  = NULL;
    AjPStr type    = NULL;
    AjPStr note    = NULL;
    AjPStr replace = NULL;
    AjPFeature feature = NULL;
    AjIList iter    = NULL;
    AjPStr notestr     = NULL;
    AjPStr replacestr  = NULL;
    AjPStr sourcestr   = NULL;
    AjPStr conflictstr = NULL;
    float score = 0.0;

    if(!*tab1)
        *tab1 = ajFeattableNewSeq(seq1);

    if(!*tab2)
        *tab2 = ajFeattableNewSeq(seq2);

    source     = ajStrNew();
    type       = ajStrNew();
    note       = ajStrNew();
    replace    = ajStrNew();
    replacestr = ajStrNew();
    notestr    = ajStrNew();

    ajStrAssignC(&source,"diffseq");
    ajStrAssignC(&type,"conflict");
    ajStrAssignC(&note,"note");
    ajStrAssignC(&replace,"replace");
    score = 1.0;

    iter = ajListIterNewread(list);
    while(!ajListIterDone(iter))
    {
        PosPDiff diff = (PosPDiff) ajListIterGet(iter) ;

        if(diff->Len1)
        {        /* is there a gap between the matches? */
            feature = ajFeatNew(*tab1, source, type,
                                diff->Start1, diff->End1,
                                score, strand, frame) ;
            if(diff->Len1 == 1 && diff->Len2 == 1)
                ajFmtPrintS(&notestr, "SNP in %S", ajSeqGetNameS(seq2));
            else if(diff->Len2 == 0)
                ajFmtPrintS(&notestr, "Insertion of %d bases in %S",
                            diff->Len1, ajSeqGetNameS(seq1));
            else
                ajFmtPrintS(&notestr, "%S", ajSeqGetNameS(seq2));

            ajFeatTagSet(feature, note, notestr);

            if(diff->Len2 > 0)
                ajStrAssignSubS(&replacestr, ajSeqGetSeqS(seq2), diff->Start2-1,
                            diff->End2-1);
            else
                ajStrAssignC(&replacestr, "");

            if(ajFeattableIsProt(*tab1))
            {
                if(ajStrGetLen(replacestr))
                {
                    ajStrAssignSubS(&sourcestr, ajSeqGetSeqS(seq1), diff->Start1-1,
                                diff->End1-1);
                    ajFmtPrintS(&conflictstr, "%S -> %S",
                                sourcestr, replacestr);
                }
                else
                    ajFmtPrintS(&conflictstr, "MISSING");

                ajFeatTagSet(feature, note, conflictstr);
            }
            else
                ajFeatTagSet(feature, replace, replacestr);
        }

        if(diff->Len2)
        {        /* is there a gap between the matches? */
            feature = ajFeatNew(*tab2, source, type,
                                diff->Start2, diff->End2,
                                score, strand, frame) ;

            if(diff->Len2 == 1 && diff->Len1 == 1)
                ajFmtPrintS(&notestr, "SNP in %S", ajSeqGetNameS(seq1));
            else if(diff->Len1 == 0)
                ajFmtPrintS(&notestr, "Insertion of %d bases in %S",
                            diff->Len2, ajSeqGetNameS(seq2));
            else
                ajFmtPrintS(&notestr, "%S", ajSeqGetNameS(seq1));

            ajFeatTagSet(feature, note, notestr);

            if(diff->Len1 > 0)
                ajStrAssignSubS(&replacestr, ajSeqGetSeqS(seq1), diff->Start1-1,
                            diff->End1-1);
            else
                ajStrAssignC(&replacestr, "");

            if(ajFeattableIsProt(*tab2))
            {
                if(ajStrGetLen(replacestr))
                {
                    ajStrAssignSubS(&sourcestr, ajSeqGetSeqS(seq2), diff->Start2-1,
                                diff->End2-1);
                    ajFmtPrintS(&conflictstr, "%S -> %S",
                                sourcestr, replacestr);
                }
                else
                    ajFmtPrintS(&conflictstr, "MISSING");
                        
                ajFeatTagSet(feature, note, conflictstr);
            }
            else
                ajFeatTagSet(feature, replace, replacestr);

        }

    }

    ajListIterDel(&iter);
    ajStrDel(&source);
    ajStrDel(&type);
    ajStrDel(&note);
    ajStrDel(&replace);
    ajStrDel(&replacestr);
    ajStrDel(&sourcestr);
    ajStrDel(&conflictstr);
    ajStrDel(&notestr);

    return;
}




/* @funcstatic diffseq_Features ********************************************
**
** Write out any features which overlap this region.
** Don't write out the source feature - far too irritating!
**
** @param [r] typefeat [const char*] Report feature tag type
** @param [u] rf [AjPFeature] Report feature to store results in
** @param [r] feat [const AjPFeattable] Feature table to search
** @param [r] start [ajuint] Start position of region (in human coordinates)
** @param [r] end [ajuint] End position of region (in human coordinates)
** @return [void]
** @@
******************************************************************************/

static void diffseq_Features(const char* typefeat, AjPFeature rf,
                                const AjPFeattable feat,
                                ajuint start, ajuint end)
{
    AjIList iter  = NULL;
    AjPFeature gf = NULL;
    AjPStr tmp    = NULL;

    if(!feat)
        return;

    if(feat->Features)
    {
        iter = ajListIterNewread(feat->Features);
        while(!ajListIterDone(iter))
        {
            gf = ajListIterGet(iter);


            /* check that the feature is within the range for display */
            if(start > ajFeatGetEnd(gf) || end < ajFeatGetStart(gf))
                continue;

            /* don't output the 'source' feature */
            if(!ajStrCmpC(ajFeatGetType(gf), "source"))
                continue;

            /* write out the feature details */
            ajFmtPrintS(&tmp, "*%s %S %d-%d",
                               typefeat, ajFeatGetType(gf),
                               ajFeatGetStart(gf), ajFeatGetEnd(gf));
            diffseq_AddTags(&tmp, gf, ajTrue);
            ajFeatTagAdd(rf, NULL,  tmp);

        }
        ajListIterDel(&iter) ;
    }


    ajStrDel(&tmp);

    return;
}




/* @funcstatic diffseq_AddTags *********************************************
**
** Appends feature tag values to a string in a simple format.
** Don't write out the translation - is it often far too long!
**
** @param [u] strval [AjPStr*] String
** @param [r] feat [const AjPFeature] Feature to be processed
** @param [r] values [AjBool] display values of tags
**
** @return [void]
** @@
******************************************************************************/

static void diffseq_AddTags(AjPStr* strval,
			    const AjPFeature feat, AjBool values)
{
    AjIList titer;                        /* iterator for taglist */
    AjPStr tagnam = NULL;
    AjPStr tagval = NULL;

    /* iterate through the tags and test for match to patterns */

    titer = ajFeatTagIter(feat);
    while(ajFeatTagval(titer, &tagnam, &tagval))
        /* don't display the translation tag - it is far too long :-) */
        if(!ajStrMatchC(tagnam, "translation"))
        {
            if(values == ajTrue)
                ajFmtPrintAppS(strval, " %S='%S'", tagnam, tagval);
            else
                ajFmtPrintAppS(strval, " %S", tagnam);
        }

    ajListIterDel(&titer);
    ajStrDel(&tagnam);
    ajStrDel(&tagval);

    return;
}



/* @funcstatic diffseq_DiffList ************************************************
**
** Converts a list of matching regions into a list of differences
** The positions are held in human coordinates (starting at 1)
** rather than computer coordinates (starting at 0) because
** there will be a lot of comparing/writing to feature tables
** which use human coordinates.
** over1start is returned with value '-1' if no matching regions were found.
**
** @param [r] matchlist [const AjPList] List of minimal non-overlapping matches
** @param [u] difflist [AjPList] Resulting list of differences
** @param [r] global [AjBool] ajTrue if we want the differences at the ends
** @param [r] seq1 [const AjPSeq] sequence 1
** @param [r] seq2 [const AjPSeq] sequence 2
** @param [w] over1start [ajint *] start of overlap region in sequence1
** @param [w] over1end [ajint *] end of overlap region in sequence1
** @param [w] over2start [ajint *] start of overlap region in sequence2
** @param [w] over2end [ajint *] end of overlap region in sequence2
** @return [void]
** @@
******************************************************************************/

static void diffseq_DiffList(const AjPList matchlist, AjPList difflist,
                             AjBool global, const AjPSeq seq1,
                             const AjPSeq seq2, ajint *over1start,
                             ajint *over1end, ajint *over2start,
                             ajint *over2end)
{

    AjIList iter    = NULL;
    ajint misstart1 = -1;                /* start of mismatch region in seq1 */
    ajint misstart2 = -1;                /* start of mismatch region in seq2 */
    ajint misend1;
    ajint misend2;                        /* end of mismatch region */
    PosPDiff diff = NULL;                /* Difference object */
    ajint i;
    ajint j;
    const char *seqc1;
    const char *seqc2;
    
    seqc1 = ajSeqGetSeqC(seq1);
    seqc2 = ajSeqGetSeqC(seq2);

    *over1start = -1;                        /* flag for no matches found */

    iter = ajListIterNewread(matchlist);
    while(!ajListIterDone(iter))
    {
        EmbPWordMatch p =(EmbPWordMatch) ajListIterGet(iter) ;

        misend1 = p->seq1start;
        misend2 = p->seq2start;

        if (misstart1 == -1)        /* this is the first iteration */
        {
            /* note the end of the overlap (start of first match) */
            *over1start = p->seq1start + 1;
            *over2start = p->seq2start + 1;

            /* add difference at the start, if required */
            if(global &&                /* we want the global differences */
              (p->seq1start != 0 || 
               p->seq2start != 0))        /* no match at pos 1,1 */
            {
                diff = diffseq_PosPDiffNew();
                /* 
                ** look for matches of less than the size of a word that
                ** will have been missed by the word-match routines, but which
                ** look weird if they are described as differences in
                ** the output. 
                */
                diff->Start1 = 1;
                diff->Start2 = 1;
                diff->End1 = misend1;
                diff->End2 = misend2;
                diff->Len1 = diff->End1 - diff->Start1 + 1;
                diff->Len2 = diff->End2 - diff->Start2 + 1;
                /* 
                ** If there are mismatches on both sequences, see if we can
                ** make a small match at the starts to tidy things up a bit.
                ** In other words, we increase the start of the mismatch if 
                ** there are any matching bases there.
                */
                for (i=0;
                     diff->Len1 && diff->Len2 &&
                     tolower((ajint)seqc1[i]) == tolower((ajint)seqc2[i]); i++)
                {
                    diff->Len1--;
                    diff->Len2--;
                    diff->Start1++;
                    diff->Start2++;        
                }
               
                /* add node to the end of the list */
                ajListPushAppend(difflist, diff);
            }
        }
        else    /* this is a mismatch between two matches */
        {
            diff = diffseq_PosPDiffNew();
            diff->Start1 = misstart1;
            diff->Start2 = misstart2;
            diff->End1 = misend1;
            diff->End2 = misend2;
            diff->Len1 = diff->End1 - diff->Start1 + 1;
            diff->Len2 = diff->End2 - diff->Start2 + 1;

            /* add node to the end of the list */
            ajListPushAppend(difflist, diff);
        }

        /* note the start position of the next mismatch */
        misstart1 = p->seq1start + p->length + 1;
        misstart2 = p->seq2start + p->length + 1;
    }

    /* note the end of the overlap (end of last match) */
    *over1end = misstart1-1;
    *over2end = misstart2-1;

    /* add difference at the end, if required */
    if(global &&                        /* we want the global differences */
      (misstart1 <= (ajint) ajSeqGetLen(seq1) ||   /* no match at the end */
       misstart2 <= (ajint) ajSeqGetLen(seq2)))
    {
        diff = diffseq_PosPDiffNew();
        diff->Start1 = misstart1;
        diff->Start2 = misstart2;
        diff->End1 = ajSeqGetLen(seq1);
        diff->End2 = ajSeqGetLen(seq2);
        diff->Len1 = diff->End1 - diff->Start1 + 1;
        diff->Len2 = diff->End2 - diff->Start2 + 1;

        /* 
        ** If there are mismatches on both sequences, see if we can
        ** make a small match at the ends to tidy things up a bit.
        ** In other words, we reduce the end of the mismatch if there
        ** are any matching bases there.
        */
        for (i=ajSeqGetLen(seq1), j=ajSeqGetLen(seq2); 
             diff->Len1 && diff->Len2 &&
             tolower((ajint)seqc1[i-1]) == tolower((ajint)seqc2[j-1]);
	     i--, j--)
        {
            diff->Len1--;
            diff->Len2--;
            diff->End1--;
            diff->End2--;        
        }
        /* add node to the end of the list */
        ajListPushAppend(difflist, diff);
    }

    ajListIterDel(&iter);

    return;
}


/* @funcstatic diffseq_PosPDiffNew ********************************************
**
** Constructor for an empty PosPDiff object
**
** @return [PosPDiff] Diference object
** @category new [PosPDiff] Constructor
** @@
******************************************************************************/

static PosPDiff diffseq_PosPDiffNew(void)
{
    PosPDiff pthis;
    AJNEW0(pthis);
                
    ajDebug("diffseq_PosPDiffNew %x\n", pthis);
                   
    return pthis;
}
    
/* @funcstatic diffseq_PosPDiffDel ********************************************
**
** Destructor for a PosPDiff object for use with ajListMap
**
** @param [r] x [void**] Undocumented
** @param [r] cl [void*] Undocumented 
** @return [void]
** @@
******************************************************************************/

static void diffseq_PosPDiffDel(void **x, void *cl)
{
    PosPDiff thys;
    thys = (PosPDiff)*x;

    (void) cl;				/* make it used */

    AJFREE(thys);
                
    ajDebug("diffseq_PosPDiffDel\n");
                   
    return;
}
    



/* @funcstatic diffseq_FeatSetCDSFrame ****************************************
**
** The Feature object in the ajfeat library has a field 'Frame' but it is
** often left as '0' (unknown) in CDS features. This routine fixes it.
** This routine assumes that the CDS features within a join()
** are sorted by start position.
**
** Frame value of 0 is unknown, values 1,2,3 are equal to GFF phases 0,1,2
**
** @param [u] ftab [AjPFeattable] Feature table
** @return [void]
** @@
******************************************************************************/

static void diffseq_FeatSetCDSFrame(AjPFeattable ftab)
{

    AjIList iter      = NULL;        /* feature table iterator */
    AjIList titer     = NULL;        /* feature tags iterator */
    AjPFeature gf     = NULL;
    ajint phase = 0;                /* translation phase, 0,1 or 2 */
    ajint prevstart = 0;        /* start and end of previous CDS in table */
    ajint prevend = 0;
    AjBool prevparent;                /* flag true if prev CDS was a parent */
    AjBool unsure;                /* true if we are unsure of the phase */
    AjPStr tagnam = NULL;/* name and value of tags of the feature */
    AjPStr tagval = NULL;

#define FEATFLAG_START_BEFORE_SEQ 0x0001 /* <start */
#define FEATFLAG_END_AFTER_SEQ    0x0002 /* >end */
#define FEATFLAG_START_TWO        0x0010  /* x.y.. */
#define FEATFLAG_END_TWO          0x0020  /* ..x.y */
#define FEATFLAG_START_UNSURE     0x4000  /* unsure position - SwissProt '?' */
#define FEATFLAG_END_UNSURE       0x8000  /* unsure position - SwissProt '?' */

    unsure = ajFalse;

    if(!ftab)
        return;
	    
    if(!ftab->Features)
        return;
        
    iter = ajListIterNewread(ftab->Features);
    while(!ajListIterDone(iter))
    {
        gf = ajListIterGet(iter);

        /* is this a CDS feature? */
        if (ajStrCmpC(ajFeatGetType(gf), "CDS"))
            continue;

        /* is this a forward sense CDS? */
        if(gf->Strand == '+') 
        {

            /* is this the start of a new group of CDS in a multi-exon gene? */
            /* reset the phase to zero, or continue it accordingly */
            if(ajFeatIsChild(gf))
            {
                phase += (prevend + 1 - prevstart) % 3;
                phase %= 3;
            }
            else
            {
                phase = 0;
                unsure = ajFalse;
            }
            
            /* are we unsure about the start of this CDS? */
            if (gf->Flags & FEATFLAG_START_BEFORE_SEQ ||
                gf->Flags & FEATFLAG_START_UNSURE ||
                gf->Flags & FEATFLAG_START_TWO)
                unsure = ajTrue;

            /* check to see if the /codon_start tag is set on this feature */

            /* check to see if the /codon_start tag is set on this feature */
            titer = ajFeatTagIter(gf);
            while(ajFeatTagval(titer, &tagnam, &tagval))
                if(!ajStrCmpC(tagnam, "codon_start"))
                {
                    ajStrToInt(tagval, &phase);
                    phase--;
                    unsure = ajFalse;
                }
            ajListIterDel(&titer);


            /* If we are sure, set the Frame field in the feature
            ** object.
            */
            if (unsure)
                gf->Frame = 0;
            else
                if (gf->Frame == 0)
                    gf->Frame = phase+1;
        }

        /* are we unsure about the end of this CDS? */
        if (gf->Flags & FEATFLAG_END_AFTER_SEQ ||
            gf->Flags & FEATFLAG_END_UNSURE ||
            gf->Flags & FEATFLAG_END_TWO)
            unsure = ajTrue;

        /* remember the start/end of this feature */
        prevstart = ajFeatGetStart(gf);
        prevend = ajFeatGetEnd(gf);
    }

    ajListIterDel(&iter);

    /* 
    ** Go back up through list filling in the frame of the reverse sense
    ** CDS features.
    */
    unsure = ajFalse;
    prevparent = ajTrue;
    iter = ajListIterNewBack(ftab->Features);
    while(!ajListIterDoneBack(iter))
    {
        gf = ajListIterGetBack(iter);

        /* is this a CDS feature? */
        if (ajStrCmpC(ajFeatGetType(gf), "CDS"))
            continue;

        /* is this a reverse sense CDS? */
        if(gf->Strand == '-') 
        {
            /* is this a new CDS group? - set the phase */
            if(prevparent) 
            {
                phase = 0;
                unsure = ajFalse;
            }
            else
            {
                phase += (prevend + 1 - prevstart) % 3;
                phase %= 3;
            }

            /* are we unsure about the end of this CDS? */
            if (gf->Flags & FEATFLAG_END_AFTER_SEQ ||
                gf->Flags & FEATFLAG_END_UNSURE ||
                gf->Flags & FEATFLAG_END_TWO)
                unsure = ajTrue;

            /* check to see if the /codon_start tag is set on this feature */
            titer = ajFeatTagIter(gf);
            while(ajFeatTagval(titer, &tagnam, &tagval))
                if(!ajStrCmpC(tagnam, "codon_start"))
                {
                    ajStrToInt(tagval, &phase);
                    phase--;
                    unsure = ajFalse;
                }
            ajListIterDel(&titer);

            /* 
            ** If we are sure, set the Frame field in the feature
            ** object.
            */
            if (unsure)
                gf->Frame = 0;
            else
                if (gf->Frame == 0)
                    gf->Frame = phase+1;
        }

        /* remember the parental status of this CDS */
        prevparent = !ajFeatIsChild(gf);

        /* are we unsure about the start of this CDS? */
        if (gf->Flags & FEATFLAG_START_BEFORE_SEQ ||
            gf->Flags & FEATFLAG_START_UNSURE ||
            gf->Flags & FEATFLAG_START_TWO)
            unsure = ajTrue;

        /* remember the start/end of this feature */
        prevstart = ajFeatGetStart(gf);
        prevend = ajFeatGetEnd(gf);
    }

    ajListIterDel(&iter);
    ajListIterDel(&titer);
    ajStrDel(&tagnam);
    ajStrDel(&tagval);

    return;
}



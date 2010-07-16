/* @source wordmatch application
**
** Finds matching words in DNA sequences
**
** @author
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

/* wordmatch
** Create a word table for the sequences in the sequence set.
** Then iterate over the sequences in the seqall set
** to find word matches. When word matches are found then
** it is checked whether the following characters do also match.
**
*/

#include "emboss.h"




/* @prog wordmatch ************************************************************
**
** Finds all exact matches of a given size between 2 sequences
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqset seqset;
    AjPSeqall seqall;
    AjPSeq queryseq;
    const AjPSeq targetseq;
    ajint wordlen;
    AjPTable wordsTable = NULL;
    AjPList* matchlist = NULL;
    AjPFile logfile;
    AjPFeattable* seqsetftables = NULL;
    AjPFeattable seqallseqftable = NULL;
    AjPFeattabOut ftoutforseqsetseq = NULL;
    AjPFeattabOut ftoutforseqallseq = NULL;
    AjPAlign align = NULL;
    AjIList iter = NULL;
    ajint targetstart;
    ajint querystart;
    ajint len;
    ajuint i, j;
    ajulong nAllMatches = 0;
    ajulong sumAllScore = 0;
    AjBool dumpAlign = ajTrue;
    AjBool dumpFeature = ajTrue;
    AjBool checkmode = ajFalse;
    EmbPWordRK* wordsw = NULL;
    ajuint npatterns = 0;
    ajuint seqsetsize;
    ajuint nmatches;
    ajuint* nmatchesseqset;
    ajuint* lastlocation; /* Cursors for Rabin-Karp search. */
                          /* Shows until what point the query sequence was
                           *  scanned for a pattern sequences in the seqset.
                          */
    char* paddedheader = NULL;
    const char* header;
    AjPStr padding;

    header = "Pattern %S  #pat-sequences  #all-matches  avg-match-length\n";
    padding = ajStrNew();

    embInit("wordmatch", argc, argv);

    wordlen = ajAcdGetInt("wordsize");
    seqset  = ajAcdGetSeqset("asequence");
    seqall  = ajAcdGetSeqall("bsequence");
    logfile = ajAcdGetOutfile("logfile");
    dumpAlign = ajAcdGetToggle("dumpalign");
    dumpFeature = ajAcdGetToggle("dumpfeat");

    if(dumpAlign)
    {
        align = ajAcdGetAlign("outfile");
        ajAlignSetExternal(align, ajTrue);
    }

    seqsetsize = ajSeqsetGetSize(seqset);
    ajSeqsetTrim(seqset);
    AJCNEW0(matchlist, seqsetsize);
    AJCNEW0(seqsetftables, seqsetsize);
    AJCNEW0(nmatchesseqset, seqsetsize);

    if (dumpFeature)
    {
        ftoutforseqsetseq =  ajAcdGetFeatout("aoutfeat");
        ftoutforseqallseq =  ajAcdGetFeatout("boutfeat");
    }

    checkmode = !dumpFeature && !dumpAlign;
    embWordLength(wordlen);

    ajFmtPrintF(logfile, "Small sequence/file for constructing"
	    " target patterns: %S\n", ajSeqsetGetFilename(seqset));
    ajFmtPrintF(logfile, "Large sequence/file to be scanned"
	    " for patterns: %S\n", ajSeqallGetFilename(seqall));
    ajFmtPrintF(logfile, "Number of sequences in the patterns file: %u\n",
            seqsetsize);
    ajFmtPrintF(logfile, "Pattern/word length: %u\n", wordlen);

    for(i=0;i<seqsetsize;i++)
    {
        targetseq = ajSeqsetGetseqSeq(seqset, i);
        embWordGetTable(&wordsTable, targetseq);
    }

    AJCNEW0(lastlocation, seqsetsize);

    if(ajTableGetLength(wordsTable)>0)
    {
        npatterns = embWordRabinKarpInit(wordsTable,
                                       &wordsw, wordlen, seqset);
        ajFmtPrintF(logfile, "Number of patterns/words found: %u\n", npatterns);

        while(ajSeqallNext(seqall,&queryseq))
        {
            for(i=0;i<seqsetsize;i++)
            {
                lastlocation[i]=0;

                if (!checkmode)
                    matchlist[i] = ajListstrNew();
            }

            nmatches = embWordRabinKarpSearch(
                    ajSeqGetSeqS(queryseq), seqset,
                    (const EmbPWordRK*)wordsw, wordlen, npatterns,
                    matchlist, lastlocation, checkmode);
            nAllMatches += nmatches;

            if (checkmode)
        	continue;

            for(i=0;i<seqsetsize;i++)
            {
                if(ajListGetLength(matchlist[i])>0)
                {
                    iter = ajListIterNewread(matchlist[i]) ;

                    while(embWordMatchIter(iter, &targetstart, &querystart, &len,
                            &targetseq))
                    {
                        if(dumpAlign)
                        {
                            ajAlignDefineSS(align, targetseq, queryseq);
                            ajAlignSetScoreI(align, len);
                            /* ungapped alignment means same length
                             *  for both sequences
                            */
                            ajAlignSetSubRange(align, targetstart, 1, len,
                                    ajSeqIsReversed(targetseq),
                                    ajSeqGetLen(targetseq),
                                    querystart, 1, len,
                                    ajSeqIsReversed(queryseq),
                                    ajSeqGetLen(queryseq));
                        }
                    }

                    if(dumpAlign)
                    {
                	ajAlignWrite(align);
                	ajAlignReset(align);
                    }

                    if(ajListGetLength(matchlist[i])>0 && dumpFeature)
                    {
                        embWordMatchListConvToFeat(matchlist[i],
                                                   &seqsetftables[i],
                                                   &seqallseqftable,
                                                   targetseq, queryseq);
                        ajFeattableWrite(ftoutforseqallseq, seqallseqftable);
                        ajFeattableDel(&seqallseqftable);
                    }

                    ajListIterDel(&iter);
                }

                embWordMatchListDelete(&matchlist[i]);
            }
        }

        /* search completed, now report statistics */
        for(i=0;i<npatterns;i++)
        {
            sumAllScore += wordsw[i]->lenMatches;

            for(j=0;j<wordsw[i]->nseqs;j++)
        	nmatchesseqset[wordsw[i]->seqindxs[j]] +=
        		wordsw[i]->nSeqMatches[j];
        }

        ajFmtPrintF(logfile, "Number of sequences in the file scanned "
                "for patterns: %u\n", ajSeqallGetCount(seqall));
        ajFmtPrintF(logfile, "Number of all matches: %Lu"
                " (wordmatch finds exact matches only)\n", nAllMatches);

        if(nAllMatches>0)
        {
            ajFmtPrintF(logfile, "Sum of match lengths: %Lu\n", sumAllScore);
            ajFmtPrintF(logfile, "Average match length: %.2f\n",
        	    sumAllScore*1.0/nAllMatches);

            ajFmtPrintF(logfile, "\nDistribution of the matches among pattern"
        	    " sequences:\n");
            ajFmtPrintF(logfile, "-----------------------------------------"
        	    "-----------\n");

            for(i=0;i<ajSeqsetGetSize(seqset);i++)
            {
        	if (nmatchesseqset[i]>0)
        	    ajFmtPrintF(logfile, "%-42s: %8u\n",
        	                ajSeqGetNameC(ajSeqsetGetseqSeq(seqset, i)),
        	                nmatchesseqset[i]);

        	ajFeattableWrite(ftoutforseqsetseq, seqsetftables[i]);
        	ajFeattableDel(&seqsetftables[i]);
            }

            ajFmtPrintF(logfile, "\nPattern statistics:\n");
            ajFmtPrintF(logfile, "-------------------\n");
            if(wordlen>7)
        	ajStrAppendCountK(&padding, ' ', wordlen-7);
            paddedheader = ajFmtString(header,padding);
            ajFmtPrintF(logfile, paddedheader);

            for(i=0;i<npatterns;i++)
        	if (wordsw[i]->nMatches>0)
        	    ajFmtPrintF(logfile, "%-7s: %12u  %12u %17.2f\n",
        	                wordsw[i]->word->fword, wordsw[i]->nseqs,
        	                wordsw[i]->nMatches,
        	                wordsw[i]->lenMatches*1.0/wordsw[i]->nMatches);
        }

    }

    for(i=0;i<npatterns;i++)
    {
        for(j=0;j<wordsw[i]->nseqs;j++)
            AJFREE(wordsw[i]->locs[j]);

        AJFREE(wordsw[i]->locs);
        AJFREE(wordsw[i]->seqindxs);
        AJFREE(wordsw[i]->nnseqlocs);
        AJFREE(wordsw[i]->nSeqMatches);
        AJFREE(wordsw[i]);
    }

    embWordFreeTable(&wordsTable);

    AJFREE(wordsw);
    AJFREE(matchlist);
    AJFREE(lastlocation);
    AJFREE(nmatchesseqset);
    AJFREE(seqsetftables);

    if(dumpAlign)
    {
        ajAlignClose(align);
        ajAlignDel(&align);
    }

    if(dumpFeature)
    {
        ajFeattabOutDel(&ftoutforseqsetseq);
        ajFeattabOutDel(&ftoutforseqallseq);
    }

    ajFileClose(&logfile);

    ajSeqallDel(&seqall);
    ajSeqsetDel(&seqset);
    ajSeqDel(&queryseq);
    ajStrDel(&padding);
    AJFREE(paddedheader);

    embExit();

    return 0;
}



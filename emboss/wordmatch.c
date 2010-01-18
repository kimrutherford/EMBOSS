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
#include <string.h>


/*
**
** Modulus (q, a large prime) and radix are used for calculating hash values.
** We should be able to replace binary search with direct search
** by selecting a small q, but in this case we should always make sure that
** a hit is a correct hit.
** radix has a relation with alphabet size
**
*/

static ajulong q = 1073741789;
static ajulong radix =256;




/* @datastatic EmbPWordWrap ***************************************************
**
** data structure that wraps EmbPWord objects for efficient access
**
** @attr word [const EmbPWord] Original word object
** @attr seqindxs [ajuint*] Positions in the seqset
**                          for each sequence the word has been seen
** @attr nnseqlocs [ajuint*] Number of word start positions for each sequence
** @attr locs [ajuint**] List of word start positions for each sequence
** @attr hash [ajulong] Hash value for the word
** @attr nseqs [ajuint] Number of sequences word has been seen
** @attr minskip [ajuint] We can scan all other words (during preprocessing)
**                        to find out a minimum length that can be skipped
**                        safely when this word is matched (not yet used)
** @attr nAllMatches [ajuint] Total number of all matches in all sequences
** @attr nSeqMatches [ajuint] Number of sequences that at least one match
** @attr lenAllMatches [ajulong] Total score/length of all the matches
**                               in all sequences
** @attr lenLongestMatches [ajulong] Total score/length of the longest matches
**                                   in all sequences
**
** @@
******************************************************************************/

typedef struct EmbSWordWrap {
    const EmbPWord word;
    ajuint* seqindxs;
    ajuint* nnseqlocs;
    ajuint** locs;
    ajulong hash;
    ajuint nseqs;
    ajuint minskip;
    ajuint nAllMatches;
    ajuint nSeqMatches;
    ajulong lenAllMatches;
    ajulong lenLongestMatches;
} EmbOWordWrap;

#define EmbPWordWrap EmbOWordWrap*



static ajuint wordmatch_embPatRabinKarpSearchMultiPattern(const AjPStr sseq,
        AjPSeqset seqset,
        const EmbPWordWrap* patterns,
        ajuint plen, ajuint nwords, AjPList* l,
        ajuint* lastlocation, ajuint** nmatchesseqset, AjBool checkmode);

static ajuint wordmatch_getWords(const AjPTable table,
                                 EmbPWordWrap**, ajuint wordlen,
                                 const AjPSeqset seqset);


static ajint wordmatch_compEmbWordWrap(const void *a, const void *b);
static ajulong wordmatch_precomputeRM(ajuint m);


/* @prog wordmatch ************************************************************
**
** Finds all exact matches of a given size between 2 sequences
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqset seqset;
    AjPSeqall seqall;
    AjPSeq seqofseqall;
    const AjPSeq seqofseqset;
    ajint wordlen;
    AjPTable wordsTable = 0;
    AjPList* matchlist = NULL;
    AjPFile logfile;
    AjPFeattable* seqsetftables = NULL;
    AjPFeattable seqallseqftable = NULL;
    AjPFeattabOut ftoutforseqsetseq = NULL;
    AjPFeattabOut ftoutforseqallseq = NULL;
    AjPAlign align = NULL;
    AjIList iter = NULL;
    ajint start1;
    ajint start2;
    ajint len;
    ajuint i, j;
    ajulong nAllMatches = 0;
    ajulong sumAllScore = 0;
    AjBool dumpAlign = ajTrue;
    AjBool dumpFeature = ajTrue;
    AjBool checkmode = ajFalse;
    EmbPWordWrap* wordsw = NULL;
    ajuint npatterns=0, seqsetsize;
    ajuint* nmatchesseqset;
    const char* header = "Pattern %S  #all-matches   avg-match-length\n";
    char* paddedheader = NULL;
    AjPStr padding = ajStrNew();


    /* Cursors for the current sequence being scanned,
    ** i.e., until which location it was scanned.
    ** We have a cursor/location entry for each sequence in the seqset.
    */
    ajuint* lastlocation;


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

    ajFmtPrintF(logfile, "Sequence file for patterns: %S\n",
            ajSeqsetGetFilename(seqset));
    ajFmtPrintF(logfile, "Sequence file to be scanned for patterns: %S\n",
            ajSeqallGetFilename(seqall));
    ajFmtPrintF(logfile, "Number of sequences in the patterns file: %u\n",
            seqsetsize);
    ajFmtPrintF(logfile, "Pattern/word length: %u\n", wordlen);

    for(i=0;i<seqsetsize;i++)
    {
        const AjPSeq seq;
        seq = ajSeqsetGetseqSeq(seqset, i);
        embWordGetTable(&wordsTable, seq);
    }
    AJCNEW0(lastlocation, i);

    if(ajTableGetLength(wordsTable)>0)
    {
        npatterns = wordmatch_getWords(wordsTable,
                                       &wordsw, wordlen, seqset);
        ajFmtPrintF(logfile, "Number of patterns/words: %u\n", npatterns);

        while(ajSeqallNext(seqall,&seqofseqall))
        {
            ajuint nmatches;
            for(i=0;i<seqsetsize;i++)
            {
                lastlocation[i]=0;
                if (!checkmode)
                matchlist[i] = ajListstrNew();
            }
            nmatches = wordmatch_embPatRabinKarpSearchMultiPattern(
                    ajSeqGetSeqS(seqofseqall), seqset,
                    (const EmbPWordWrap*)wordsw, wordlen, npatterns,
                    matchlist, lastlocation, &nmatchesseqset, checkmode);

            if (!checkmode)
            for(i=0;i<seqsetsize;i++)
            {
                if(nmatches>0)
                {
                    iter = ajListIterNewread(matchlist[i]) ;

                    while(embWordMatchIter(iter, &start1, &start2, &len,
                            &seqofseqset))
                    {
                        if (dumpAlign){
                            /* TODO: check any possible improvements using
                             * if(ajAlignFormatShowsSequences(align)
                             */
                            ajAlignDefineSS(align, seqofseqset, seqofseqall);
                            ajAlignSetScoreI(align, len);
                            /* ungapped alignment
                             * so same length for both sequences */
                            ajAlignSetSubRange(align, start1, 1, len,
                                    ajSeqIsReversed(seqofseqset),
                                    ajSeqGetLen(seqofseqset),
                                    start2, 1, len,
                                    ajSeqIsReversed(seqofseqall),
                                    ajSeqGetLen(seqofseqall));
                            ajAlignWrite(align);
                            ajAlignReset(align);
                        }
                    }

                    if(ajListGetLength(matchlist[i])>0 && dumpFeature)
                    {
                        embWordMatchListConvToFeat(matchlist[i],
                                &seqsetftables[i], &seqallseqftable,
                                seqofseqset, seqofseqall);
                        ajFeattableWrite(ftoutforseqallseq, seqallseqftable);
                        ajFeattableDel(&seqallseqftable);
                    }

                    ajListIterDel(&iter);
                }
                embWordMatchListDelete(&matchlist[i]);
            }
            nAllMatches += nmatches;
        }

        for(i=0;i<npatterns;i++)
        {
            sumAllScore += wordsw[i]->lenAllMatches;
        }

        ajFmtPrintF(logfile, "Number of sequences in the file scanned "
                "for patterns: %u\n", ajSeqallGetCount(seqall));
        ajFmtPrintF(logfile, "Number of all matches: %Lu"
                " (wordmatch finds exact matches only)\n", nAllMatches);
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
        {
            if (wordsw[i]->nAllMatches>0)
                ajFmtPrintF(logfile, "%-7s: %12u  %17.2f\n",
                        wordsw[i]->word->fword,
                        wordsw[i]->nAllMatches,
                        wordsw[i]->lenAllMatches*1.0/wordsw[i]->nAllMatches
                );
        }
    }
    AJFREE(seqsetftables);

    for(i=0;i<npatterns;i++)
    {
        AJFREE(wordsw[i]->seqindxs);
        for(j=0;j<wordsw[i]->nseqs;j++)
            AJFREE(wordsw[i]->locs[j]);
        AJFREE(wordsw[i]->nnseqlocs);
        AJFREE(wordsw[i]->locs);
        AJFREE(wordsw[i]);
    }

    embWordFreeTable(&wordsTable);
    AJFREE(wordsw);
    AJFREE(matchlist);
    AJFREE(lastlocation);
    AJFREE(nmatchesseqset);

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
    ajSeqDel(&seqofseqall);
    ajStrDel(&padding);
    AJFREE(paddedheader);

    embExit();

    return 0;
}




/* @funcstatic wordmatch_compEmbWordWrap **************************************
**
** Comparison function for EmbPWordWrap objects, based on their hash values
**
** @param [r] a [const void *] First EmbPWordWrap object
** @param [r] b [const void *] Second EmbPWordWrap object
**
** @return [ajint] difference of hash values
******************************************************************************/

static ajint wordmatch_compEmbWordWrap(const void *a, const void *b)
{
    const EmbPWordWrap ww1;
    const EmbPWordWrap ww2;

    ww1 = *(const EmbPWordWrap const *) a;
    ww2 = *(const EmbPWordWrap const *) b;

    if (ww1->hash > ww2->hash)
        return 1;

    if (ww1->hash < ww2->hash)
        return -1;

    return 0;
}




/* @funcstatic wordmatch_precomputeRM *****************************************
**
** Precomputes a value that helps recalculating consecutive hash values
** with less computation. Uses q and radix variables defined above.
**
** @param [r] m [ajuint] word length
** @return [ajulong] radix^(m-1) % q
** @@
******************************************************************************/

static ajulong wordmatch_precomputeRM(ajuint m)
{
    ajulong rm;
    ajuint i;
    rm = 1;
    for (i = 1; i <= m-1; i++)
    {
        rm = (radix * rm) % q;
    }
    return rm;
}




/* @funcstatic wordmatch_embPatRabinKarpSearchMultiPattern ********************
**
** Rabin Karp search for multiple patterns.
**
** @param [r] sseq [const AjPStr] Sequence to be scanned for multiple patterns
** @param [r] seqset [AjPSeqset] Sequence set patterns found
** @param [r] patterns [const EmbPWordWrap*] Patterns to be searched
** @param [r] plen [ajuint] Length of patterns
** @param [r] npatterns [ajuint] Number of patterns
** @param [u] matchlist [AjPList*] List of matches for each sequence
**                                in the sequence set
** @param [u] lastlocation [ajuint*] Position of the search for each sequence
**                                in the sequence set
** @param [u] nmatchesseqset [ajuint**] Number of matches to the sequence set
** @param [r] checkmode [AjBool] If true, not writing features or alignments
**                               running in check mode only
** @return [ajuint] number of matches
** @@
******************************************************************************/

static ajuint wordmatch_embPatRabinKarpSearchMultiPattern(const AjPStr sseq,
    AjPSeqset seqset,
    const EmbPWordWrap* patterns,
    ajuint plen, ajuint npatterns, AjPList* matchlist,
    ajuint* lastlocation, ajuint** nmatchesseqset, AjBool checkmode)
{
    const char *text;
    const AjPSeq seq;
    ajuint i, matchlen, tlen, ii, k, seqsetindx, indxloc, maxloc;
    ajuint nMatches=0;
    EmbPWordWrap* bsres; /* match found using binary search */
    EmbPWordWrap cursor;
    ajulong rm;
    ajulong textHash = 0;
    ajuint seq2start;
    char* tmp;

    AJNEW0(cursor);

    rm = wordmatch_precomputeRM(plen);
    text = ajStrGetPtr(sseq);
    tlen  = ajStrGetLen(sseq);

    for (i=0; i<plen; i++)
    {
        textHash    = (ajulong)(radix * textHash   + text[i]) %q;
    }

    /* Scan the sequence */
    for (i=plen; i<=tlen;)
    {
        cursor->hash = textHash;
        bsres = bsearch(&cursor, patterns, npatterns,
                sizeof(EmbPWordWrap), wordmatch_compEmbWordWrap);
        if(bsres!=NULL)
        {
            seq2start = i-plen;

            for (k=0;k<(*bsres)->nseqs;k++)
            {
                seqsetindx = (*bsres)->seqindxs[k];
                seq = ajSeqsetGetseqSeq(seqset, seqsetindx);
                if (lastlocation[seqsetindx] < i)
                {
                    maxloc = 0;
                    for(indxloc=0; indxloc < (*bsres)->nnseqlocs[k]; indxloc++)
                    {
                        ajuint pos = (*bsres)->locs[k][indxloc];
                        const char* seq_ = ajSeqGetSeqC(seq);
                        matchlen=0;
                        /* following loop is to make sure we never have
                         * false positives, after we are confident that
                         * we don't get false hits we can delete/disable it
                         */
                        while(matchlen<plen)
                        {
                            if(seq_[pos+matchlen] != text[i+matchlen-plen])
                            {
                                AJCNEW0(tmp,plen+1);
                                tmp[plen] = '\0';
                                memcpy(tmp, text+i-plen, plen);
                                ajWarn("unexpected match:   pat:%s  pat-pos:%u,"
                                        " txt-pos:%u text:%s hash:%u\n",
                                        (*bsres)->word->fword, pos, i+matchlen-plen,
                                        tmp, textHash);
                                AJFREE(tmp);
                                break;
                            }
                            matchlen++;
                        }

                        if(matchlen<plen)
                            continue;
                        (*bsres)->nAllMatches++;
                        ii = seq2start+plen;
                        while(ii<tlen  && pos+matchlen<ajSeqGetLen(seq))
                        {
                            if(seq_[pos+matchlen] != text[ii++])
                                break;
                            else
                                ++matchlen;
                        }
                        nMatches ++;
                        if (!checkmode)
                        embWordMatchListAppend(matchlist[seqsetindx],
                                seq, pos, seq2start, matchlen);
                        if ( ii > maxloc )
                            maxloc = ii;
                        (*bsres)->lenAllMatches += matchlen;

                        (*nmatchesseqset)[seqsetindx] ++;

                    }
                    if (maxloc>0)
                    {
                        lastlocation[seqsetindx] = maxloc;
                    }
                }
            }
        }
        textHash = ((textHash +text[i-plen] *(q-rm))*radix+ text[i]) % q;
        ++i;
    }
    AJFREE(cursor);
    return nMatches;
}




/* @funcstatic wordmatch_getWords *********************************************
**
** Preprocesses word/pattern table and repackages the words in EmbPWordWrap
** objects to improve access efficiency. Also computes hash values
** for each pattern.
**
** @param [r] table [const AjPTable] table of patterns
** @param [u] newwords [EmbPWordWrap**] Patterns to be searched
** @param [r] wordlen [ajuint] Length of words/patterns
** @param [r] seqset [const AjPSeqset] Sequence set patterns are derived from
** @return [ajuint] number of words
** @@
******************************************************************************/

static ajuint wordmatch_getWords(const AjPTable table, EmbPWordWrap** newwords,
        ajuint wordlen, const AjPSeqset seqset)
{
    ajuint i, j, k, l;
    EmbPWord* embwords = NULL;
    const EmbPWord embword;
    ajulong patternHash;
    EmbPWordWrap newword;
    AjIList iterp;
    EmbPWordSeqLocs* seqlocs;
    ajuint nseqlocs;
    const AjPSeq seq;
    const char* word;
    ajuint nseqs = ajSeqsetGetSize(seqset);
    ajuint nwords = ajTableToarrayValues(table, (void***)&embwords);

    AJCNEW(*newwords, nwords);

    for (i=0; i<nwords; i++)
    {
        seqlocs=NULL;
        embword = embwords[i];
        word = embword->fword;

        AJNEW0(newword);

        patternHash = 0;
        for (j=0; j<wordlen; j++)
        {
            patternHash = (ajulong)(radix * patternHash + word[j]) % q;
        }

        nseqlocs = ajTableToarrayValues(embword->seqlocs, (void***)&seqlocs);
        newword->nseqs = nseqlocs;
        newword->hash  = patternHash;
        newword->word = embword;
        AJCNEW(newword->seqindxs, nseqlocs);
        AJCNEW(newword->locs, nseqlocs);
        AJCNEW(newword->nnseqlocs, nseqlocs);

        for(j=0; j<nseqlocs; j++)
        {
            seq= seqlocs[j]->seq;
            for(l=0;l<nseqs;l++)
            {
                if (seq == ajSeqsetGetseqSeq(seqset,l))
                {
                    newword->seqindxs[j] = l;
                    break;
                }
            }
            if (l==nseqs)
            {
                ajErr("something wrong, sequence not found in seqset");
                ajExitBad();
            }
            iterp = ajListIterNewread(seqlocs[j]->locs);
            k=0;
            newword->nnseqlocs[j]= ajListGetLength(seqlocs[j]->locs);
            AJCNEW(newword->locs[j],newword->nnseqlocs[j]);
            while(!ajListIterDone(iterp))
            {
                ajuint pos = *(ajuint *) ajListIterGet(iterp);
                newword->locs[j][k++] = pos;
            }
            ajListIterDel(&iterp);
        }

        AJFREE(seqlocs);

        (*newwords)[i] = newword;

    }

    AJFREE(embwords);

    qsort(*newwords, nwords, sizeof(EmbPWordWrap), wordmatch_compEmbWordWrap);

    return nwords;
}

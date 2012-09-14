/* @source embword ************************************************************
**
** Wordmatch routines
**
** @author Copyright (c) 1999 Gary Williams
** @version $Revision: 1.65 $
** @modified $Date: 2012/07/14 14:52:41 $ by $Author: rice $
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public
** License along with this library; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA  02110-1301,  USA.
**
******************************************************************************/

#include "ajlib.h"

#include "embword.h"
#include "ajassert.h"
#include "ajseq.h"
#include "ajfeat.h"
#include "ajfile.h"
#include "ajlist.h"
#include "ajtable.h"
#include "ajutil.h"

#include <math.h>




/*
** current wordlength - this is an easily accessible copy of the value
** in the first node of wordLengthList
*/
static ajuint wordLength = 0;

/* list of wordlengths with current one at top of list */
static AjPList wordLengthList = NULL;

static AjPList wordCurList = NULL;

/*
** Rabin-Karp multi-pattern search parameters.
** Modulus (a large prime) and radix are used in calculating hash values
** efficiently.
**
** Note: we should be able to replace binary search in Rabin-Karp search
** algorithm with direct search by selecting a small q, however in this case
** we should always check that a hit is a correct hit (as we currently do).
** Radix has a relation with alphabet size, selecting its value depending
** on input might be awarding in this context.
**
*/

#define RK_MODULUS 1073741789UL
#define RK_RADIX 256UL



static ajint    wordCmpStr(const void *x, const void *y);
static ajint    wordCompare(const void *x, const void *y);
static void     wordCurIterTrace(const AjIList curiter);
static void     wordCurListTrace(const AjPList curlist);
static ajint    wordDeadZone(EmbPWordMatch match,
			     ajint deadx1, ajint deady1,
			     int deadx2, ajint deady2, ajint minlength);
static void     wordListInsertNodeOld(AjPListNode* pnode, void* x);
static void     wordListInsertOld(AjIList iter, void* x);
static ajint    wordMatchCmp(const void* v1, const void* v2);
static ajint    wordMatchCmpPos(const void* v1, const void* v2);
static void     wordNewListTrace(ajint i, const AjPList newlist);
static void     wordOrderPosMatchTable(AjPList unorderedList);

static ajulong  wordStrHash(const void *key, ajulong hashsize);

static void     wordVFreeLocs(void **value);
static void     wordVFreeSeqlocs(void **value);

static ajint    wordRabinKarpCmp(const void *trgseq, const void *qryseq);
static ajulong  wordRabinKarpConstant(ajuint m);




/* @funcstatic wordCmpStr *****************************************************
**
** Compare two words for first n chars. n set by embWordLength.
**
** @param [r] x [const void *] First word
** @param [r] y [const void *] Second word
** @return [ajint] difference
**
** @release 1.0.0
** @@
******************************************************************************/

static ajint wordCmpStr(const void *x, const void *y)
{
    return ajCharCmpCaseLen((const char *)x, (const char *)y, wordLength);
}




/* @funcstatic wordStrHash ****************************************************
**
** Create hash value from key.
**
** @param [r] key [const void *] key.
** @param [r] hashsize [ajulong] Hash size
** @return [ajulong] hash value
**
** @release 1.0.0
** @@
******************************************************************************/

static ajulong wordStrHash(const void *key, ajulong hashsize)
{
    ajulong hashval;
    const char *s;

    ajuint i;

    s = (const char *) key;

    for(i=0, hashval = 0; i < wordLength; i++, s++)
	hashval = toupper((ajint)*s) + 31 *hashval;

    return hashval % hashsize;
}




/* @funcstatic wordCompare ****************************************************
**
** Compare two words in descending order.
**
** @param [r] x [const void *] First word
** @param [r] y [const void *] Second word
** @return [ajint] count difference for words.
**
** @release 2.1.0
** @@
******************************************************************************/

static ajint wordCompare(const void *x, const void *y)
{
/*
    const EmbPWord xw;
    const EmbPWord yw;

    xw = ((const EmbPWord2)x)->fword;
    yw = ((const EmbPWord2)y)->fword;

    return (yw->count - xw->count);
*/

    return ((*(EmbPWord const *)y)->count -
	    (*(EmbPWord const *)x)->count);
}




/* @func embWordLength ********************************************************
**
** Sets the word length for all functions. Must be called first.
** Creates the word length list if not yet done.
** Pushes the latest word length value on the list.
**
** @param [r] wordlen [ajint] Word length
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void embWordLength(ajint wordlen)
{
    ajint *pint;

    if(!wordLengthList)
	wordLengthList = ajListNew();

    /* store the wordlength in case we do recursive word stuff */
    AJNEW0(pint);
    *pint = wordlen;
    ajListPush(wordLengthList, pint);

    /* set the current wordlength as an easily accessible static ajint */
    wordLength = wordlen;

    return;
}




/* @func embWordClear *********************************************************
**
** Clears the word length for all functions. To be called when all is done.
** Pops the last word length from the list and frees it.
** If there is nothing else on the list, it frees the list.
**
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void embWordClear(void)
{
    ajint *pint;

    /*
    ** pop the previous word length from the list in case there's
    ** recursive word stuff
    */
    if(ajListGetLength(wordLengthList))
    {
	ajListPop(wordLengthList, (void **)&pint);
	AJFREE(pint);
    }

    if(!ajListGetLength(wordLengthList))
    {
	ajListFree(&wordLengthList);
	wordLengthList = NULL;		/* no valid word length set */
	wordLength = 0;			/* no valid word length set */
    }
    else
    {
	/* set the current wordlength as an easily accessible static ajint */
	ajListPeekFirst(wordLengthList, (void **)&pint);
	wordLength = *pint;
    }

    return;
}




/* @func embWordPrintTable ****************************************************
**
** Print the words found with their frequencies.
**
** @param [r] table [const AjPTable] table to be printed
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void embWordPrintTable(const AjPTable table)
{
    void **valarray = NULL;
    EmbPWord ajnew;
    ajint i;

    ajTableToarrayValues(table, &valarray);

    qsort(valarray, (size_t) ajTableGetLength(table), sizeof (*valarray), wordCompare);

    for(i = 0; valarray[i]; i++)
    {
	ajnew = (EmbPWord) valarray[i];
	ajUser("%.*s\t%d", wordLength, ajnew->fword,ajnew->count);
    }

    AJFREE(valarray);

    return;
}




/* @func embWordPrintTableFI **************************************************
**
** Print the words found with their frequencies.
**
** @param [r] table [const AjPTable] table to be printed
** @param [r] mincount [ajint] Minimum frequency to report
** @param [u] outf [AjPFile] Output file.
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void embWordPrintTableFI(const AjPTable table, ajint mincount, AjPFile outf)
{
    void **valarray = NULL;
    EmbPWord ajnew;
    ajint i;

    if(!ajTableGetLength(table)) return;

    i = (ajuint) ajTableToarrayValues(table, &valarray);

    ajDebug("embWordPrintTableFI size %d mincount:%d\n", i, mincount);

    for(i = 0; valarray[i]; i++)
    {
	ajnew = (EmbPWord) valarray[i];
	ajDebug("embWordPrintTableFI unsorted [%d] %.*s %d\n",
		i, wordLength, ajnew->fword,ajnew->count);
    }

    qsort(valarray, (size_t) ajTableGetLength(table), sizeof (*valarray), wordCompare);

    for(i = 0; valarray[i]; i++)
    {
	ajnew = (EmbPWord) valarray[i];
	ajDebug("embWordPrintTableFI sorted [%d] %.*s %d\n",
		i, wordLength, ajnew->fword,ajnew->count);

	if(ajnew->count < mincount)
            break;

	ajFmtPrintF(outf, "%.*s\t%d\n",
			   wordLength, ajnew->fword,ajnew->count);
    }

    AJFREE(valarray);

    return;
}




/* @func embWordPrintTableF ***************************************************
**
** Print the words found with their frequencies.
**
** @param [r] table [const AjPTable] table to be printed
** @param [u] outf [AjPFile] Output file.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void embWordPrintTableF(const AjPTable table, AjPFile outf)
{
    embWordPrintTableFI(table, 1, outf);

    return;
}




/* @funcstatic wordVFreeLocs **************************************************
**
** Free the elements in a EmbPWord locations table.
**
** @param [d] value [void**] Data value for a table item
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void wordVFreeLocs(void **value)
{
    AjPTable table = ((EmbPWord)*value)->seqlocs;

    ajTableDelValdel(&table, &wordVFreeSeqlocs);

    /* free the locations structure */
    AJFREE(*value);

    return;
}




/* @funcstatic wordVFreeSeqlocs ***********************************************
**
** Free the elements in a EmbPWord sequence locations list.
**
** @param [d] value [void**] Data values as void**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void wordVFreeSeqlocs(void **value)
{
    AjPList list = ((EmbPWordSeqLocs)*value)->locs;

    /* free the elements in the list of the positions */
    ajListFreeData(&list);

    /* free the locations structure */
    AJFREE(*value);

    return;
}




/* @func embWordFreeTable *****************************************************
**
** delete the word table and free the memory.
**
** @param [d] table [AjPTable*] table to be deleted
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void embWordFreeTable(AjPTable *table)
{
    ajTableDel(table);

    return;
}




/* @funcstatic wordMatchListDelete ********************************************
**
** deletes entries in a list of matches.
**
** @param [d] x [void**] Data values as void**
** @param [r] cl [void*] Ignored user data, usually NULL.
** @return [void]
**
** @release 2.1.0
** @@
******************************************************************************/

static void wordMatchListDelete(void **x,void *cl)
{
    EmbPWordMatch p;

    p = (EmbPWordMatch)*x;

    AJFREE(p);

    if(!cl)
        return;

    return;
}




/* @func embWordMatchListDelete ***********************************************
**
** delete the word table.
**
** @param [u] plist [AjPList*] list to be deleted.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void embWordMatchListDelete(AjPList* plist)
{
    if(!*plist)
	return;

    ajListMap(*plist, &wordMatchListDelete, NULL);
    ajListFree(plist);

    return;
}




/* @funcstatic wordMatchListPrint *********************************************
**
** print the word table.
**
** @param [r] x [void*] List item (EmbPWordMatch*)
** @param [r] cl [void*] Output file AjPFile
** @return [void]
**
** @release 2.1.0
** @@
******************************************************************************/

static void wordMatchListPrint(void *x,void *cl)
{
    EmbPWordMatch p;
    AjPFile file;

    p    = (EmbPWordMatch)x;
    file = (AjPFile) cl;

    ajFmtPrintF(file, "%10d  %10d %10d\n",
		       p->seq1start+1,
		       p->seq2start+1,
		       p->length);

    return;
}




/* @func embWordMatchListPrint ************************************************
**
** print the word table.
**
** @param [u] file [AjPFile] Output file
** @param [r] list [const AjPList] list to be printed.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void embWordMatchListPrint(AjPFile file, const AjPList list)
{
    ajListMapread(list, &wordMatchListPrint, file);

    return;
}




/* @func embWordMatchListConvToFeat *******************************************
**
** convert the word table to feature tables.
**
** @param [r] list [const AjPList] list to be printed.
** @param [u] tab1 [AjPFeattable*] feature table for sequence 1
** @param [u] tab2 [AjPFeattable*] feature table for sequence 2
** @param [r] seq1 [const AjPSeq] sequence
** @param [r] seq2 [const AjPSeq] second sequence
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void embWordMatchListConvToFeat(const AjPList list,
				AjPFeattable *tab1, AjPFeattable *tab2,
				const AjPSeq seq1, const AjPSeq seq2)
{
    char strand = '+';
    ajint frame = 0;
    AjPStr source = NULL;
    AjPStr type   = NULL;
    AjPStr tag    = NULL;
    AjPFeature feature;
    AjIList iter  = NULL;
    float score   = 1.0;
    
    if(!*tab1)
	*tab1 = ajFeattableNewSeq(seq1);

    if(!*tab2)
	*tab2 = ajFeattableNewSeq(seq2);
    
    ajStrAssignC(&source,"wordmatch");
    ajStrAssignC(&type,"misc_feature");
    score = 1.0;
    ajStrAssignC(&tag,"note");
    
    iter = ajListIterNewread(list);

    while(!ajListIterDone(iter))
    {
	EmbPWordMatch p = (EmbPWordMatch) ajListIterGet(iter);
	feature = ajFeatNew(*tab1, source, type,
			    p->seq1start+1,p->seq1start+p->length , score,
			    strand, frame) ;
	
	ajFeatTagSet(feature, tag, ajSeqGetNameS(seq2));
	
	feature = ajFeatNew(*tab2, source, type,
			    p->seq2start+1,p->seq2start+p->length , score,
			    strand, frame) ;
	
	ajFeatTagSet(feature, tag, ajSeqGetNameS(seq1));
    }
    
    ajListIterDel(&iter);
    ajStrDel(&source);
    ajStrDel(&type);
    ajStrDel(&tag);
    
    return;
}




/* @func embWordGetTable ******************************************************
**
** Builds a table of all words in a sequence.
**
** The word length must be defined by a call to embWordLength.
**
** @param [u] table [AjPTable*] table to be created or updated.
** @param [r] seq [const AjPSeq] Sequence to be "worded"
** @return [AjBool] ajTrue if successful
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool embWordGetTable(AjPTable *table, const AjPSeq seq)
{
    const char * startptr;
    ajuint i;
    ajuint j;
    ajuint ilast;
    ajuint *k;
    EmbPWord rec;
    EmbPWordSeqLocs seqlocs;
    const AjPStr seqname;
    char* key;

    ajuint wordsize;
    char skipchar;

    wordsize = wordLength+1;

    skipchar = 'X';

    if(ajSeqIsNuc(seq))
	skipchar = 'N';

    assert(wordLength > 0);

    ajDebug("embWordGetTable seq.len %d wordlength %d skipchar '%c'\n",
	     ajSeqGetLen(seq), wordLength, skipchar);

    if(ajSeqGetLen(seq) < wordLength)
    {
	ajDebug("sequence too short: wordsize = %d, sequence length = %d",
	       wordLength, ajSeqGetLen(seq));

	return ajFalse;
    }

    if(!*table)
    {
	*table = ajTableNewFunctionLen(ajSeqGetLen(seq),
				       &wordCmpStr, &wordStrHash,
                                       &ajMemFree, &wordVFreeLocs);
	ajDebug("make new table\n");
    }

    /* initialise ptr to start of seq string */
    startptr = ajSeqGetSeqC(seq);

    i = 0;
    ilast = ajSeqGetLen(seq) - wordLength;

    j=0;

    while(j<wordLength)
    {
	if((char)toupper((ajint)startptr[j]) == skipchar)
	{
	    ajDebug("Skip '%c' at start from %d",
		    skipchar, i+j+1);

	    while((char)toupper((ajint)startptr[j]) == skipchar)
	    {
		i++;
		startptr++;
	    }

	    ajDebug(" to %d\n",
		    i+j);
	    j = 0;

	    if(i > ilast)
            {
		ajDebug("sequence has no word without ambiguity code '%c'\n",
			skipchar);

		return ajFalse;
	    }
	}
	else
	    j++;
    }

    j = wordLength - 1;

    while(i <= ilast)
    {
	if((char)toupper((ajint)startptr[j]) == skipchar)
	{
	    ajDebug("Skip '%c' from %d", skipchar, j);

	    while((char)toupper((ajint)startptr[j]) == skipchar)
	    {
		i++;
		startptr++;
	    }

	    i += j;
	    startptr += j;
	    ajDebug(" to %d\n", i);
	    continue;
	}

	rec = (EmbPWord) ajTableFetchmodV(*table, startptr);

	/* does it exist already */
	if(rec)
	{
	    /* if yes increment count */
	    rec->count++;
	}
	else
	{
	    /* else create a new word */
	    AJNEW0(rec);
	    rec->count = 1;
	    key = ajCharNewResLenC(startptr, wordsize, wordLength);
	    rec->fword = key;
	    rec->seqlocs = ajTablestrNew(1000);
	    ajTablePut(*table, key, rec);
	}

	AJNEW0(k);
	*k = i;
	seqname = ajSeqGetNameS(seq);
	seqlocs = (EmbPWordSeqLocs) ajTableFetchmodS(rec->seqlocs, seqname);

	if (seqlocs == NULL)
	{
	    AJNEW0(seqlocs);
	    seqlocs->seq = seq;
	    seqlocs->locs = ajListNew();
	    ajTablePut(rec->seqlocs, ajStrNewS(seqname), seqlocs);
	}

	ajListPushAppend(seqlocs->locs, k);

	startptr++;
	i++;

    }

    ajDebug("table done, size %Lu\n", ajTableGetLength(*table));

    return ajTrue;
}




/* @funcstatic wordOrderMatchTable ********************************************
**
** Sort the hits by length then seq1 start then by seq2 start
**
** @param [u] unorderedList [AjPList] Unsorted list
** @return [void]
**
** @release 2.1.0
** @@
******************************************************************************/

static void wordOrderMatchTable(AjPList unorderedList)
{
    ajDebug("wordOrderMatchTable size %d\n", ajListGetLength(unorderedList));
    ajListSort(unorderedList, &wordMatchCmp);

    return;
}




/* @funcstatic wordMatchCmp ***************************************************
**
** Compares two sequence matches so the result can be used in sorting.
** The comparison is done by size and if the size is equal, by seq1
** start position.  If the seq1 start positions are equal they are
** sorted by seq2 start position.
**
** @param [r] v1 [const void*] First word
** @param [r] v2 [const void*] Comparison word
** @return [ajint] Comparison value. 0 if equal, -1 if first is lower,
**               +1 if first is higher.
**
** @release 1.0.0
** @@
******************************************************************************/

static ajint wordMatchCmp(const void* v1, const void* v2)
{
    const EmbPWordMatch m1;
    const EmbPWordMatch m2;

    m1 = *(EmbPWordMatch const *) v1;
    m2 = *(EmbPWordMatch const *) v2;

    /*
       ajDebug("m1 %x %5d %5d %5d\n",
       m1, m1->length, m1->seq2start, m1->seq1start);
       ajDebug("m2 %x %5d %5d %5d\n",
       m2, m2->length, m2->seq2start, m2->seq1start);
       */

    if(m1->length != m2->length)
    {
	if(m1->length < m2->length)
	{
	    /*ajDebug("return 1\n");*/
	    return 1;
	}
	else
	    return -1;
    }

    if(m1->seq1start != m2->seq1start)
    {
	if(m1->seq1start > m2->seq1start)
	    return 1;
	else
	    return -1;
    }

    if(m1->seq2start != m2->seq2start)
    {
	if(m1->seq2start > m2->seq2start)
	    return 1;
	else
	    return -1;
    }

    return 0;
}




/* @funcstatic wordOrderPosMatchTable *****************************************
**
** Sort the hits by seq1 start then by seq2 start
**
** @param [u] unorderedList [AjPList] Unsorted list
** @return [void]
**
** @release 2.1.0
** @@
******************************************************************************/

static void wordOrderPosMatchTable(AjPList unorderedList)
{
    ajListSort(unorderedList, &wordMatchCmpPos);

    return;
}




/* @funcstatic wordMatchCmpPos ************************************************
**
** Compares two sequence matches so the result can be used in sorting.
** The comparison is done by seq1
** start position.  If the seq1 start positions are equal they are
** sorted by seq2 start position.
**
** @param [r] v1 [const void*] First word
** @param [r] v2 [const void*] Comparison word
** @return [ajint] Comparison value. 0 if equal, -1 if first is lower,
**               +1 if first is higher.
**
** @release 1.0.0
** @@
******************************************************************************/

static ajint wordMatchCmpPos(const void* v1, const void* v2)
{
    EmbPWordMatch m1;
    EmbPWordMatch m2;

    m1 = *(EmbPWordMatch const *) v1;
    m2 = *(EmbPWordMatch const *) v2;

    /*
       ajDebug("m1 %x %5d %5d %5d\n",
       m1, m1->length, m1->seq2start, m1->seq1start);
       ajDebug("m2 %x %5d %5d %5d\n",
       m2, m2->length, m2->seq2start, m2->seq1start);
       */

    if(m1->seq1start != m2->seq1start)
    {
	if(m1->seq1start > m2->seq1start)
	{
	    /*ajDebug("return 1\n");*/
	    return 1;
	}
	else return -1;
    }

    if(m1->seq2start != m2->seq2start)
    {
	if(m1->seq2start > m2->seq2start)
	{
	    /*ajDebug("return 1\n");*/
	    return 1;
	}
	else return -1;
    }

    return 0;
}




/* @func embWordBuildMatchTable ***********************************************
**
** Create a linked list of all the matches and order them by the
** second sequence.
**
** We need three lists:
**   (a) all hits, added in positional order
**   (b) ongoing hits, where we have not reached the end yet
**                 which is a list of items in "all hits" being updated
**   (c) new hits, found in the word table from the other sequence.
**
** @param [r] seq1MatchTable [const AjPTable] Match table
** @param [r] seq2 [const AjPSeq] Second sequence
** @param [r] orderit [ajint] 1 to sort results at end, else 0.
** @return [AjPList] List of matches.
** @error NULL table was not built due to an error.
**
** @release 1.0.0
** @@
******************************************************************************/

AjPList embWordBuildMatchTable(const AjPTable seq1MatchTable,
			       const AjPSeq seq2,
				ajint orderit)
{
    ajuint i = 0;
    ajuint ilast;
    AjPList hitlist = NULL;
    const AjPList newlist = NULL;
    const AjPTable seqlocst;
    EmbPWordSeqLocs* seqlocs=NULL;
    const char *startptr;
    EmbPWord wordmatch;
    EmbPWordMatch newmatch;
    EmbPWordMatch curmatch = NULL;
    AjIList newiter;
    AjIList curiter;
    void *ptr = NULL;

    ajint *k = 0;
    ajuint kcur = 0;
    ajuint kcur2 = 0;
    ajuint knew = 0;
    AjBool matched = ajFalse;

    assert(wordLength > 0);

    hitlist = ajListNew();

    if(!wordCurList)
	wordCurList = ajListNew();

    if(ajSeqGetLen(seq2) < wordLength)
    {
	ajWarn("ERROR: Sequence %S length %d less than word length %d",
	       ajSeqGetUsaS(seq2), ajSeqGetLen(seq2), wordLength);

	return hitlist;
    }

    startptr = ajSeqGetSeqC(seq2);
    ilast    = ajSeqGetLen(seq2) - wordLength;

    /*ajDebug("embWordBuildMatchTable ilast: %u\n", ilast);*/

    while(i < (ilast+1))
    {
	if((wordmatch = ajTableFetchmodV(seq1MatchTable, startptr)))
	{
	    /* match found so create EmbSWordMatch structure and fill it
	    ** in. Then set next pos accordingly
	    ** BUT this could match several places so need to do for each
            ** position
            **
	    ** there is a match between the two sequences
	    ** this could extend an existing match or start a new one
	    */

	    seqlocst = wordmatch->seqlocs;
	    ajTableToarrayValues(seqlocst, (void***)&seqlocs);
	    /* TODO: assumes matching against single sequence */
	    newlist = seqlocs[0]->locs;

	    if(!ajListGetLength(newlist))
		ajWarn("ERROR: newlist is empty\n");

            /*ajDebug("\nnewlist %u i:%u\n", ajListGetLength(newlist), i);*/
	    newiter = ajListIterNewread(newlist);

	    /* this is the list of matches for the current word and position */

	    if(ajListGetLength(wordCurList))
	    {
                /*ajDebug("wordCurList size %d\n",
                  ajListGetLength(wordCurList));*/
		curiter = ajListIterNew(wordCurList);

		curmatch = ajListIterGet(curiter);
		kcur = curmatch->seq1start + curmatch->length - wordLength + 1;
		kcur2 = curmatch->seq2start + curmatch->length - wordLength + 1;
	    }
	    else
		curiter = NULL;

	    while(!ajListIterDone(newiter) )
	    {
		k = (ajint*) ajListIterGet(newiter);
		knew = *k;

                /*ajDebug("knew: %u i:%u\n", knew, i);*/
		/* compare to current hits to test for extending */

                ajListIterRewind(curiter);
                matched = ajFalse;

                while(!ajListIterDone(curiter) )
                {
		    curmatch = ajListIterGet(curiter);
		    kcur = curmatch->seq1start + curmatch->length -
                        wordLength + 1;
                    kcur2 = curmatch->seq2start + curmatch->length -
                        wordLength + 1;
                    /*ajDebug(".test kcur/knew %u/%u kcur2/i %u/%u\n",
                      kcur, knew, kcur2, i);*/

                    /* when we test, we may have already incremented
                       one of the matches - so test old and new kcur2 */
                    if(kcur2 != i && kcur2 != i+1)
                    {
                        /*ajDebug("finished kcur: %u kcur2: %u i: %u\n",
                          kcur, kcur2,i);*/
                        ajListIterRemove(curiter);
                        continue;
                    }

                    if(kcur == knew && kcur2 == i)
                    {			/* check continued matches */
                        /* ajDebug("**match knew: %d kcur: %d kcur2: %d "
                           "start1: %d "
                                "start2: %d len: %d i:%d\n",
                                knew, kcur, kcur2,curmatch->seq1start,
                                curmatch->seq2start,curmatch->length, i);*/
			curmatch->length++;
                        matched = ajTrue;
                        continue;
                    }
                }

                if(!matched)
                {			/* new current match */
                    /*ajDebug("save start1: %d start2: %d len: %d\n",
                            match2->seq1start, match2->seq2start,
                            match2->length);*/
                    /* add to hitlist */
                    newmatch = embWordMatchNew(seq2, knew, i, wordLength);
                    ajListPushAppend(hitlist, newmatch);

                    if(curiter)
                    {			/* add to wordCurList */
                        /*ajDebug("...ajListInsert using curiter %u\n",
                          ajListGetLength(wordCurList));*/
                        wordListInsertOld(curiter, newmatch);
                        /*wordCurListTrace(wordCurList);*/
                        /*wordCurIterTrace(curiter);*/
                    }
                    else
                    {
                        /*ajDebug("...ajListPushAppend to wordCurList %u\n",
                          ajListGetLength(wordCurList));*/
                        ajListPushAppend(wordCurList, newmatch);
                        /* wordCurListTrace(wordCurList); */
                    }
                }
 		/* ajDebug("k: %d i: %d\n", *k, i); */
	    }

	    ajListIterDel(&newiter);
            ajListIterDel(&curiter);
            AJFREE(seqlocs);
	}

	/* no match, so all existing matches are completed */

	i++;
	startptr++;
    }

    /* wordCurListTrace(hitlist); */
    if(orderit)
	wordOrderMatchTable(hitlist);

    /* wordCurListTrace(hitlist); */

    while(ajListPop(wordCurList,(void **)&ptr));

    return hitlist;
}




/* @func embWordMatchNew ******************************************************
**
** Creates and initialises a word match object
**
** @param [r] seq[const AjPSeq] Query sequence, match has been found
** @param [r] seq1start [ajuint] Start position in target sequence
** @param [r] seq2start [ajuint] Start position in query sequence
** @param [r] length [ajint] length of the word match
** @return [EmbPWordMatch] New word match object.
**
** @release 6.3.0
** @@
******************************************************************************/

EmbPWordMatch embWordMatchNew(const AjPSeq seq, ajuint seq1start,
	                      ajuint seq2start, ajint length)
{
    EmbPWordMatch match;

    AJNEW0(match);

    match->sequence  = seq;
    match->seq1start = seq1start;
    match->seq2start = seq2start;
    match->length = length;

    ajDebug("new word match start1: %d start2: %d len: %d\n",
            match->seq1start, match->seq2start,
            match->length);

    return match;
}




/* @funcstatic wordNewListTrace ***********************************************
**
** Reports contents of a word list.
**
** @param [r] i [ajint] Offset
** @param [r] newlist [const AjPList] word list.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void wordNewListTrace(ajint i, const AjPList newlist)
{
    ajint *k;
    AjIList iter;

    iter = ajListIterNewread(newlist);

    ajDebug("\n++newlist... %d \n", i);
    ajDebug("++  k+len  i+len    k+1    i+1    len\n");

    while(!ajListIterDone(iter))
    {
	k = (ajint*) ajListIterGet(iter);
	ajDebug("++ %6d %6d %6d %6d %6d\n",
		(*k)+wordLength, i+wordLength, (*k)+1, i+1, wordLength);
    }

    ajListIterDel(&iter);

    return;
}




/* @funcstatic wordCurListTrace ***********************************************
**
** Reports contents of a word list.
**
** @param [r] curlist [const AjPList] word list.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void wordCurListTrace(const AjPList curlist)
{
/*
    EmbPWordMatch match;
    ajint i;
    ajint j;
    ajint ilen;
*/    
    AjIList iter = ajListIterNewread(curlist);

    /*
       ajDebug("\ncurlist...\n");
       while(!ajListIterDone(iter))
       {
       match = ajListIterGet(iter);
       i = match->seq1start + 1;
       j = match->seq2start + 1;
       ilen = match->length;
       ajDebug("%6d %6d %6d %6d %6d\n",
       i+ilen, j+ilen, i, j, ilen);
       }
    */

    ajListIterDel(&iter);

    return;
}




/* @funcstatic wordCurIterTrace ***********************************************
**
** Reports contents of a current word list iterator
**
** @param [r] curiter [const AjIList] List iterator for the current word list
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void wordCurIterTrace(const AjIList curiter)
{
    AjPListNode node;
    EmbPWordMatch match;
    ajint i, j, ilen;

    ajDebug("curiter ...\n");

    if(curiter->Here)
    {
        node = curiter->Here;
        match = node->Item;
        i = match->seq1start + 1;
        j = match->seq2start + 1;
        ilen = match->length;
        ajDebug(" Here: %6d %6d %6d %6d %6d\n",
                i+ilen, j+ilen, i, j, ilen);
    }
    else
        ajDebug(" Here: NULL\n");

    node = curiter->Head->First;
    match = node->Item;
    i = match->seq1start + 1;
    j = match->seq2start + 1;
    ilen = match->length;
    ajDebug(" Orig: %6d %6d %6d %6d %6d\n",
            i+ilen, j+ilen, i, j, ilen);

    return;
}




/* @funcstatic wordDeadZone ***************************************************
**
** Determines if a match is within the region which is not overlapped by the
** match starting at position (deadx1, deady1) or ending at position
** (deadx2, deady2). If it is in this region
** (the 'live zone') then 0 is returned, if it is partially in this region
** then it is truncated to lie inside it and 2 is returned, else 1 is returned.
**
** What is the 'live zone' and 'dead zone'?
** When an initial large match has been found we wish to remove any other
** (smaller) matches that overlap with it. The region in which other matches
** overlap with the first match are called here the 'dead zones'. The regions
** in which they don't overlap are called the 'live zones'. Other matches are
** OK if they are in live zones - they can co-exist with this match.
**
**
**                   deadx2
**       |              .
**       |              .   live
**       |              .   zone 2
**       |     dead     ............deady2
**       |     zone     /
**  seq2 |             /
**       |            /match
**       |           /
**   deady1..........
**       |          .     dead
**       |          .     zone
**       |live      .
**       |zone 1  deadx1
**       -------------------------
**                 seq1
**
** @param [u] match [EmbPWordMatch] match to investigate
** @param [r] deadx1 [ajint] x position of end of live zone 1
** @param [r] deady1 [ajint] y position of end of live zone 1
** @param [r] deadx2 [ajint] x position of end of live zone 2
** @param [r] deady2 [ajint] y position of end of live zone 2
** @param [r] minlength [ajint] minimum length of match
** @return [ajint] 0=in live zone, 1=in dead zone, 2=truncated
**
** @release 2.1.0
** @@
******************************************************************************/

static ajint wordDeadZone(EmbPWordMatch match,
			  ajint deadx1, ajint deady1,
			  ajint deadx2, ajint deady2, ajint minlength)
{
    ajint startx;
    ajint starty;
    ajint endx;
    ajint endy;

    startx = match->seq1start;
    starty = match->seq2start;

    endx = match->seq1start + match->length -1;
    endy = match->seq2start + match->length -1;


    /* is it in the top right live zone ? */
    if(startx > deadx2 && starty > deady2)
	return 0;

    /* is it in the bottom right live zone ? */
    if(endx < deadx1 && endy < deady1)
	return 0;

    /* is it in the top left dead zone? */
    if(starty >=  deady1 && endx <= deadx2)
	return 1;

    /* is it in the bottom right dead zone? */
    if(endy <= deady2 && startx >= deadx1)
	return 1;

    /* it must be partially in a dead zone - truncate it */

    if(endy < deady2)
    {
	if(startx - starty < deadx1 - deady1)	        /* crosses deady1 */
	    match->length = deady1 - starty;
	else if(startx - starty > deadx1 - deady1)	/* crosses deadx1 */
	    match->length = deadx1 - startx;
	else
	    ajFatal("Found a match where match is on the same diagonal \n"
		    "startx=%d, starty=%d, endx=%d, endy=%d \n"
		    "deadx1=%d, deady1=%d, deadx2=%d, deady2=%d\n",
		    startx, starty, endx, endy, deadx1, deady1, deadx2,
		    deady2);
    }
    else if(starty > deady1)
    {
	if(startx - starty < deadx1 - deady1)
	{
	    /* crosses deadx2 */
	    match->length = endx - deadx2;
	    match->seq1start = deadx2 +1;
	    match->seq2start += deadx2 - startx +1;

	}
	else if(startx - starty > deadx1 - deady1)
	{
	    /* crosses deady2 */
	    match->length = endy - deady2;
	    match->seq1start += deady2 - starty +1;
	    match->seq2start = deady2 +1;

	}
	else
	    ajFatal("Found a match where match is on the same diagonal \n"
		    "startx=%d, starty=%d, endx=%d, endy=%d \n"
		    "deadx1=%d, deady1=%d, deadx2=%d, deady2=%d\n",
		    startx, starty, endx, endy, deadx1, deady1, deadx2,
		    deady2);
    }
    else
	ajFatal("Found a match that was not caught by any case \n"
		"startx=%d, starty=%d, endx=%d, endy=%d \n"
		"deadx1=%d, deady1=%d, deadx2=%d, deady2=%d\n",
		startx, starty, endx, endy, deadx1, deady1, deadx2, deady2);

    /*
    **  is the truncated match shorter than our allowed minimum length?
    **  If so it should be dead
    */
    if(match->length < minlength)
	return 1;

    return 2;
}




/* @func embWordMatchMin ******************************************************
**
** Given a list of matches, reduce it to the minimal set of best
** non-overlapping matches.
**
** @param [u] matchlist [AjPList] list of matches to reduce to
**                                non-overlapping set
** @return [void]
**
** @release 2.0.0
** @@
******************************************************************************/

void embWordMatchMin(AjPList matchlist)
{
    AjIList iter = NULL;
    EmbPWordMatch match;
    EmbPWordMatch thismatch;
    AjPList minlist;			/* list of matches in min set */
    ajint deadx1;			/* positions of the dead zones */
    ajint deady1;
    ajint deadx2;
    ajint deady2;
    AjBool truncated;
    ajint result;

    minlist = ajListNew();

    /* order the matches by size - largest first */
    wordOrderMatchTable(matchlist);

    /*
    **  remove all other matches in the overlapping dead-zone, truncating
    **  those that extend into the live-zone
    */

    /* repeat until there are no more matches to process */
    while(ajListGetLength(matchlist))
    {
	/* get next longest match and append to list of minimal matches */
	ajListPop(matchlist, (void **)&thismatch);

	ajListPushAppend(minlist, thismatch);

	/* get the positions of the dead zones */
	deadx1 = thismatch->seq1start;	/* first pos of match */
	deady1 = thismatch->seq2start;	/* first pos of match */

	/* last pos of match */
	deadx2 = thismatch->seq1start + thismatch->length -1;

	/* last pos of match */
	deady2 = thismatch->seq2start + thismatch->length -1;

	/* haven't truncated any matches yet */
	truncated = ajFalse;

	/* look at all remaining matches in matchlist */
	iter = ajListIterNew(matchlist);

	while(!ajListIterDone(iter))
	{
	    match = ajListIterGet(iter);

	    /* want to remove this match if it is in the dead zone */
	    result = wordDeadZone(match, deadx1, deady1, deadx2, deady2,
				  wordLength);
	    if(result == 1)
	    {
		/*
		**  it is in the dead zone - remove it
		**  Need to free up the match structure and remove the
		**  current node of the list
		*/
		wordMatchListDelete((void **)&match, NULL);
		ajListIterRemove(iter);
	    }
	    else if(result == 2)
	    {
		/* it is partially in the dead zone - now truncated */
		truncated = ajTrue;
	    }
	}

	ajListIterDel(&iter);

	/*
        **  if some truncating done then need to sort the matchlist
        **  again by size
        */
	if(truncated)
	    wordOrderMatchTable(matchlist);
    }


    /* sort by x start position */
    wordOrderPosMatchTable(minlist);


    /* matchlist is now reduced to the minimal non-overlapping list */
    ajListPushlist(matchlist, &minlist);

    return;
}




/* @func embWordMatchFirstMax *************************************************
**
** Given list of matches returns the first match with maximum similarity/score.
**
** @param [r] matchlist [const AjPList] list of matches
** @return [EmbPWordMatch] maximum match
**
** @release 6.3.0
** @@
******************************************************************************/

EmbPWordMatch embWordMatchFirstMax(const AjPList matchlist)
{
    ajint maxmatch = 0;
    EmbPWordMatch p;
    EmbPWordMatch max = NULL;

    AjIList iter;
    
    iter = ajListIterNewread(matchlist);

    while(!ajListIterDone(iter))
    {
	p = (EmbPWordMatch) ajListIterGet(iter);

	if(p->length>maxmatch)
	{
	    max = p;
	    maxmatch = p->length;
	}
	else if(p->length==maxmatch)
	{
	    ajDebug("possible max match position start1:%d start2:%d"
		    " length:%d\n",p->seq1start, p->seq2start, p->length);

	    if(p->seq1start<max->seq1start)
		max =p;
	}
    }

    ajDebug("maximum match position start1:%d start2:%d"
	    " length:%d\n",max->seq1start, max->seq2start, max->length);

    ajListIterDel(&iter);

    return max;
}




/* @funcstatic wordRabinKarpConstant ******************************************
**
** Returns a value that helps recalculating consecutive hash values
** with less computation during Rabin-Karp search.
**
** @param [r] m [ajuint] word length
** @return [ajulong] radix^(m-1) % modulus
**
** @release 6.3.0
** @@
******************************************************************************/

static ajulong wordRabinKarpConstant(ajuint m)
{
    ajulong rm;
    ajuint i;

    rm = 1;

    for(i = 1; i <= m-1; i++)
        rm = (RK_RADIX * rm) % RK_MODULUS;

    return rm;
}




/* @funcstatic wordRabinKarpCmp ***********************************************
**
** Comparison function for EmbPWordRK objects, based on their hash values
**
** @param [r] trgseq [const void *] First EmbPWordRK object
** @param [r] qryseq [const void *] Second EmbPWordRK object
**
** @return [ajint] difference of hash values
**
** @release 6.3.0
******************************************************************************/

static ajint wordRabinKarpCmp(const void *trgseq, const void *qryseq)
{
    const EmbPWordRK ww1;
    const EmbPWordRK ww2;

    ww1 = *(const EmbPWordRK const *) trgseq;
    ww2 = *(const EmbPWordRK const *) qryseq;

    if(ww1->hash > ww2->hash)
        return 1;

    if(ww1->hash < ww2->hash)
        return -1;

    return 0;
}




/* @func embWordRabinKarpInit *************************************************
**
** Scans word/pattern table and repackages the words in EmbPWordRK
** objects to improve access efficiency by Rabin-Karp search.
** Computes hash values for each word/pattern.
**
** @param [r] table [const AjPTable] Table of patterns
** @param [u] ewords [EmbPWordRK**] Extended word objects to be used
**                                  in Rabin-Karp search
** @param [r] wordlen [ajuint] Length of words/patterns, kmer size
** @param [r] seqset [const AjPSeqset] Sequence set, input patterns
**                                     were derived from
** @return [ajuint] number of words
**
** @release 6.3.0
** @@
******************************************************************************/

ajuint embWordRabinKarpInit(const AjPTable table, EmbPWordRK** ewords,
                            ajuint wordlen, const AjPSeqset seqset)
{
    ajuint i;
    ajuint j;
    ajuint k;
    ajuint l;
    EmbPWord* words = NULL;
    const EmbPWord embword = NULL;
    ajulong patternHash;
    EmbPWordRK newword = NULL;
    AjIList iterp;
    EmbPWordSeqLocs* seqlocs = NULL;
    ajuint nseqlocs;
    const AjPSeq seq = NULL;
    const char* word;
    ajuint nseqs;
    ajuint nwords;
    ajuint pos;
    
    nseqs = ajSeqsetGetSize(seqset);
    nwords = (ajuint) ajTableToarrayValues(table, (void***)&words);
    AJCNEW(*ewords, nwords);

    for(i=0; i<nwords; i++)
    {
        seqlocs=NULL;
        embword = words[i];
        word = embword->fword;

        AJNEW0(newword);

        patternHash = 0;

        /* TODO: we can continuously calculate the hash value
         *       as we do in the search function */
        for(j=0; j<wordlen; j++)
            patternHash = (RK_RADIX * patternHash +
			   toupper((int)word[j]))% RK_MODULUS;

        nseqlocs = (ajuint) ajTableToarrayValues(embword->seqlocs, (void***)&seqlocs);
        newword->nseqs = nseqlocs;
        newword->hash  = patternHash;
        newword->word = embword;
        AJCNEW(newword->seqindxs, nseqlocs);
        AJCNEW(newword->locs, nseqlocs);
        AJCNEW(newword->nnseqlocs, nseqlocs);
        AJCNEW(newword->nSeqMatches, nseqlocs);

        for(j=0; j<nseqlocs; j++)
        {
            seq= seqlocs[j]->seq;

            for(l=0;l<nseqs;l++)
                if (seq == ajSeqsetGetseqSeq(seqset,l))
                {
                    newword->seqindxs[j] = l;
                    break;
                }

            if(l == nseqs)
            {
                ajErr("something wrong, sequence not found in seqset");
                ajExitBad();
            }

            iterp = ajListIterNewread(seqlocs[j]->locs);
            k = 0;
            newword->nnseqlocs[j] = (ajuint) ajListGetLength(seqlocs[j]->locs);
            AJCNEW(newword->locs[j],newword->nnseqlocs[j]);

            while(!ajListIterDone(iterp))
            {
                pos = *(ajuint *) ajListIterGet(iterp);
                newword->locs[j][k++] = pos;
            }

            ajListIterDel(&iterp);
        }

        AJFREE(seqlocs);

        (*ewords)[i] = newword;

    }

    AJFREE(words);

    qsort(*ewords, nwords, sizeof(EmbPWordRK), wordRabinKarpCmp);

    return nwords;
}




/* @func embWordRabinKarpSearch ***********************************************
**
** Rabin Karp search for multiple patterns.
**
** @param [r] sseq [const AjPStr] Sequence to be scanned for multiple patterns
** @param [r] seqset [const AjPSeqset] Sequence-set,
**                                     where search patterns coming from
** @param [r] patterns [EmbPWordRK const *] Patterns to be searched
** @param [r] plen [ajuint] Length of patterns
** @param [r] npatterns [ajuint] Number of patterns
** @param [u] matchlist [AjPList*] List of matches for each sequence
**                                 in the sequence set
** @param [u] lastlocation [ajuint*] Position of the search for each sequence
**                                   in the sequence set
** @param [r] checkmode [AjBool] If true, not writing features or alignments
**                               but running to produce match statistics only
** @return [ajuint] total number of matches
**
** @release 6.3.0
** @@
******************************************************************************/

ajuint embWordRabinKarpSearch(const AjPStr sseq,
                              const AjPSeqset seqset,
                              EmbPWordRK const * patterns,
                              ajuint plen, ajuint npatterns,
                              AjPList* matchlist,
                              ajuint* lastlocation, AjBool checkmode)
{
    const char *text;
    const AjPSeq seq;
    ajuint i;
    ajuint matchlen;
    ajuint tlen;
    ajuint ii;
    ajuint k;
    ajuint seqsetindx;
    ajuint indxloc;
    ajuint maxloc;
    ajuint nMatches = 0;
    EmbPWordRK* bsres; /* match found using binary search */
    EmbPWordRK cursor;
    ajulong rm;
    ajulong textHash = 0;
    ajuint seq2start;
    char* tmp;

    ajuint pos;
    const char *seq_;
    
    AJNEW0(cursor);

    rm = wordRabinKarpConstant(plen);
    text = ajStrGetPtr(sseq);
    tlen  = ajStrGetLen(sseq);

    for(i=0; i<plen; i++)
        textHash = (ajulong)(RK_RADIX * textHash   +
			     toupper((int)text[i])) % RK_MODULUS;

    /* Scan the input sequence sseq for all patterns */
    for (i=plen; i<=tlen; i++)
    {
        cursor->hash = textHash;
        bsres = bsearch(&cursor, patterns, npatterns,
                sizeof(EmbPWordRK), wordRabinKarpCmp);

        if(bsres!=NULL)
        {
            seq2start = i-plen;

            for(k=0;k<(*bsres)->nseqs;k++)
            {
                seqsetindx = (*bsres)->seqindxs[k];
                seq = ajSeqsetGetseqSeq(seqset, seqsetindx);

                if(lastlocation[seqsetindx] < i)
                {
                    maxloc = 0;

                    for(indxloc=0; indxloc < (*bsres)->nnseqlocs[k]; indxloc++)
                    {
                        pos = (*bsres)->locs[k][indxloc];
                        seq_ = ajSeqGetSeqC(seq);
                        matchlen=0;
                        ii = seq2start;

                        /* following loop is to make sure we never have
                         * false positives, after we are confident that
                         * we don't get false hits we can delete/disable it
                         */
                        while(matchlen<plen)
                        {
			  if(toupper((int)seq_[pos+matchlen]) !=
			     toupper((int)text[ii++]))
                            {
                                AJCNEW0(tmp,plen+1);
                                tmp[plen] = '\0';
                                memcpy(tmp, text+i-plen, plen);
                                ajDebug("unexpected match:   pat:%s  pat-pos:"
                                        "%u, txt-pos:%u text:%s hash:%u\n",
                                        (*bsres)->word->fword, pos,
                                        i+matchlen-plen, tmp, textHash);
                                AJFREE(tmp);
                                break;
                            }

                            matchlen++;
                        }

                        /* if the match was a false positive skip it */
                        if(matchlen<plen)
                            continue;

                        /* this is where we extend matches */
                        while(ii<tlen  && pos+matchlen<ajSeqGetLen(seq))
                        {
			  if(toupper((int)seq_[pos+matchlen]) !=
			     toupper((int)text[ii++]))
                                break;
                            else
                                ++matchlen;
                        }

                        nMatches ++;

                        if(!checkmode)
                            ajListPushAppend(matchlist[seqsetindx],
                                             embWordMatchNew(seq,pos,seq2start,
                                                             matchlen));

                        if(ii > maxloc)
                            maxloc = ii;

                        (*bsres)->lenMatches += matchlen;
                        (*bsres)->nMatches++;
                        (*bsres)->nSeqMatches[k]++;

                    }

                    if(maxloc>0)
                    {
                        lastlocation[seqsetindx] = maxloc;
                    }
                }
            }
        }

        textHash = ((textHash + toupper((int)text[i-plen]) * (RK_MODULUS-rm))
		    * RK_RADIX + toupper((int)text[i])) % RK_MODULUS;
    }

    AJFREE(cursor);

    return nMatches;
}




/* @func embWordMatchIter *****************************************************
**
** Return the start positions and length for the next match.
** The caller iterates over the list, which is a standard AjPList
**
** @param [u] iter [AjIList] List iterator
** @param [w] start1 [ajint*] Start in first sequence
** @param [w] start2 [ajint*] Start in second sequence
** @param [w] len [ajint*] Length of match
** @param [w] seq [const AjPSeq*] Pointer to sequence
** @return [AjBool] ajFalse if the iterator was exhausted
**
**
** @release 2.4.0
******************************************************************************/

AjBool embWordMatchIter(AjIList iter, ajint* start1, ajint* start2,
			ajint* len, const AjPSeq* seq)
{
    EmbPWordMatch p;

    if(ajListIterDone(iter))
	return ajFalse;

    p = (EmbPWordMatch) ajListIterGet(iter);
    *start1 = p->seq1start;
    *start2 = p->seq2start;
    *len = p->length;
    *seq = p->sequence;

    return ajTrue;
}




/* @funcstatic wordListInsertOld **********************************************
**
** Obsolete ajListInsert version emulation
** Insert an item in a list, using an iterator (if not null)
** to show which position to insert. Otherwise, simply push.
**
** @param [u] iter [AjIList] List iterator.
** @param [r] x [void*] Data item to insert.
** @return [void]
**
** @release 2.1.0
** @@
******************************************************************************/

static void wordListInsertOld(AjIList iter, void* x)
{
    AjPList list;
    AjPListNode p;

    list = iter->Head;
    p    = iter->Here;

    if(!p->Prev)
    {
	ajListPush(list,(void *)x);
	return;
    }

    if(p == list->First)
    {
	if(!p->Prev->Prev)
	    wordListInsertNodeOld(&list->First->Next,x);
	else
	    wordListInsertNodeOld(&p->Prev->Next,x);
    }
    else
    {
	if(!p->Prev->Prev)
	    wordListInsertNodeOld(&list->First,x);
	else
	    wordListInsertNodeOld(&p->Prev->Prev->Next,x);
    }

    list->Count++;

    return;
}




/* @funcstatic wordListInsertNodeOld ******************************************
**
** Inserts a new node in a list at the current node position.
**
** @param [u] pnode [AjPListNode*] Current node.
** @param [r] x [void*] Data item to insert.
** @return [void]
**
** @release 2.1.0
** @@
******************************************************************************/

static void wordListInsertNodeOld(AjPListNode* pnode, void* x)
{
    AjPListNode p;

    AJNEW0(p);
    p->Item = x;
    p->Next = (*pnode);
    p->Prev = (*pnode)->Prev;
    p->Next->Prev = p;
    *pnode = p;

    return;
}




/* @func embWordUnused ********************************************************
**
** Unused functions. Here to keep compiler warnings away
**
** @return [void]
**
** @release 2.0.0
******************************************************************************/

void embWordUnused(void)
{

    wordCurListTrace(NULL);	/* comment out in embWordBuildMatchTable */
    wordCurIterTrace(NULL);	/* comment out in embWordBuildMatchTable */
    wordNewListTrace(0, NULL);	/* comment out in embWordBuildMatchTable */

    return;
}




/* @func embWordExit **********************************************************
**
** Cleanup word matching indexing internals on exit
**
** @return [void]
**
** @release 4.0.0
******************************************************************************/

void embWordExit(void)
{
    embWordClear();
    ajListFree(&wordCurList);

    return;
}




#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif




#ifdef AJ_COMPILE_DEPRECATED
/* @obsolete embWordMatchListAppend
** @remove use embWordMatchNew followed by a list append call
*/
__deprecated EmbPWordMatch embWordMatchListAppend(AjPList hitlist,
                                                  const AjPSeq seq,
                                                  const ajuint seq1start,
                                                  ajuint seq2start,
                                                  ajint length)
{
    EmbPWordMatch match;
    AJNEW0(match);
    match->sequence  = seq;
    match->seq1start = seq1start;
    match->seq2start = seq2start;
    match->length = length;
    ajDebug("new word match start1: %d start2: %d len: %d\n",
            match->seq1start, match->seq2start,
            match->length);
    ajListPushAppend(hitlist, match);
    return match;
}
#endif

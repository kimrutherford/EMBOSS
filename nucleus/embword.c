/* @source embword.c
**
** Wordmatch routines
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
#include "math.h"

/*
** current wordlength - this is an easily accessible copy of the value
** in the first node of wordLengthList
*/
static ajuint wordLength = 0;

/* list of wordlengths with current one at top of list */
static AjPList wordLengthList = NULL;

static AjPList wordCurList = NULL;



static ajint    wordCmpStr(const void *x, const void *y);
static ajint    wordCompare(const void *x, const void *y);
static void     wordCurIterTrace(const AjIList curiter);
static void     wordCurListTrace(const AjPList curlist);
static ajint    wordDeadZone(EmbPWordMatch match,
			     ajint deadx1, ajint deady1,
			     int deadx2, ajint deady2, ajint minlength);
static ajint    wordFindWordAtPos(const char *word,
				  const AjPTable seq1MatchTable,
				  ajint nextpos);
static ajint    wordGetWholeMatch(EmbPWordMatch match,
				  const AjPTable seq1MatchTable);
static void     wordListInsertNodeOld(AjPListNode* pnode, void* x);
static void     wordListInsertOld(AjIList iter, void* x);
static ajint    wordMatchCmp(const void* v1, const void* v2);
static ajint    wordMatchCmpPos(const void* v1, const void* v2);
static void     wordNewListTrace(ajint i, const AjPList newlist);
static void     wordOrderPosMatchTable(AjPList unorderedList);

static unsigned wordStrHash(const void *key, unsigned hashsize);

static void     wordVFree(void **key, void **count, void *cl);


/* @funcstatic wordCmpStr *****************************************************
**
** Compare two words for first n chars. n set by embWordLength.
**
** @param [r] x [const void *] First word
** @param [r] y [const void *] Second word
** @return [ajint] difference
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
** @param [r] hashsize [unsigned] Hash size
** @return [unsigned] hash value
** @@
******************************************************************************/

static unsigned wordStrHash(const void *key, unsigned hashsize)
{
    unsigned hashval;
    const char *s;

    ajuint i;

    s = (const char *) key;

    for(i=0, hashval = 0; i < wordLength; i++, s++)
	hashval = toupper((ajint)*s) + 31 *hashval;

    return hashval % hashsize;
}




/* @funcstatic wordCompare ****************************************************
**
** Compare two word in descoending order.
**
** @param [r] x [const void *] First word
** @param [r] y [const void *] Second word
** @return [ajint] count difference for words.
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
** @@
******************************************************************************/

void embWordPrintTable(const AjPTable table)
{
    void **keyarray = NULL;
    void **valarray = NULL;
    EmbPWord ajnew;
    ajint i;

    ajTableToarray(table, &keyarray, &valarray);

    qsort(valarray, ajTableGetLength(table), sizeof (*valarray), wordCompare);
    for(i = 0; valarray[i]; i++)
    {
	ajnew = (EmbPWord) valarray[i];
	ajUser("%.*s\t%d", wordLength, ajnew->fword,ajnew->count);
    }

    AJFREE(keyarray);
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
** @@
******************************************************************************/

void embWordPrintTableFI(const AjPTable table, ajint mincount, AjPFile outf)
{
    void **keyarray = NULL;
    void **valarray = NULL;
    EmbPWord ajnew;
    ajint i;

    i = ajTableToarray(table, &keyarray, &valarray);

    ajDebug("embWordPrintTableFI size %d mincount:%d\n", i, mincount);
    for(i = 0; valarray[i]; i++)
    {
	ajnew = (EmbPWord) valarray[i];
	ajDebug("embWordPrintTableFI unsorted [%d] %.*s %d\n",
		i, wordLength, ajnew->fword,ajnew->count);
    }
    qsort(valarray, ajTableGetLength(table), sizeof (*valarray), wordCompare);
    for(i = 0; valarray[i]; i++)
    {
	ajnew = (EmbPWord) valarray[i];
	ajDebug("embWordPrintTableFI sorted [%d] %.*s %d\n",
		i, wordLength, ajnew->fword,ajnew->count);
	if(ajnew->count < mincount) break;
	ajFmtPrintF(outf, "%.*s\t%d\n",
			   wordLength, ajnew->fword,ajnew->count);
    }

    AJFREE(keyarray);
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
** @@
******************************************************************************/

void embWordPrintTableF(const AjPTable table, AjPFile outf)
{
    embWordPrintTableFI(table, 1, outf);
    return;
}




/* @funcstatic wordPositionListDelete *****************************************
**
** deletes entries in a list of positions.
**
** @param [d] x [void**] Data values as void**
** @param [r] cl [void*] Ignored user data, usually NULL.
** @return [void]
** @@
******************************************************************************/

static void wordPositionListDelete(void **x,void *cl)
{
    ajint *p;

    p = (ajint *)*x;

    AJFREE(p);

    if(!cl) return;

    return;
}




/* @funcstatic wordVFree ******************************************************
**
** free the elements in a list of positons
**
** @param [r] key [void**] key for a table item
** @param [d] count [void**] Data values as void**
** @param [r] cl [void*] Ignored user data, usually NULL.
** @return [void]
** @@
******************************************************************************/

static void wordVFree(void **key, void **count, void *cl)
{
    char* ckey;

    ckey = (char*) *key;
    ajCharDel(&ckey);

    /* free the elements in the list of the positons */
    ajListMap(((EmbPWord)*count)->list,wordPositionListDelete, NULL);

    /* free the list structure for the positions. */
    ajListFree(&((EmbPWord)*count)->list);

    /* free the word structure */
    AJFREE(*count);

    if(!cl)return;
    return;
}




/* @func embWordFreeTable *****************************************************
**
** delete the word table and free the memory.
**
** @param [d] table [AjPTable*] table to be deleted
** @return [void]
** @@
******************************************************************************/

void embWordFreeTable(AjPTable *table)
{
    ajTableMapDel(*table, wordVFree, NULL);
    ajTableFree(table);
    table = 0;

    return;
}




/* @funcstatic wordMatchListDelete ********************************************
**
** deletes entries in a list of matches.
**
** @param [d] x [void**] Data values as void**
** @param [r] cl [void*] Ignored user data, usually NULL.
** @return [void]
** @@
******************************************************************************/

static void wordMatchListDelete(void **x,void *cl)
{
    EmbPWordMatch p;

    p = (EmbPWordMatch)*x;

    AJFREE(p);

    if(!cl)return;
    return;
}




/* @func embWordMatchListDelete ***********************************************
**
** delete the word table.
**
** @param [u] plist [AjPList*] list to be deleted.
** @return [void]
** @@
******************************************************************************/

void embWordMatchListDelete(AjPList* plist)
{
    if(!*plist)
	return;

    ajListMap(*plist,wordMatchListDelete, NULL);
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
** @@
******************************************************************************/

void embWordMatchListPrint(AjPFile file, const AjPList list)
{
    ajListMapread(list,wordMatchListPrint, file);

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
				       wordCmpStr, wordStrHash);
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
	    if(i > ilast) {
		ajDebug("sequence has no word without ambiguity code '%c'\n",
			skipchar);
		return ajFalse;
	    }
	}
	else
	    j++;
    }



    while(i <= ilast)
    {
	j = wordLength - 1;
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

	rec = (EmbPWord) ajTableFetch(*table, startptr);

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
	    rec->list = ajListNew();
	    ajTablePut(*table, key, rec);
	}

	AJNEW0(k);
	*k = i;
	ajListPushAppend(rec->list, k);

	startptr ++;
	i++;

    }

    ajDebug("table done, size %d\n", ajTableGetLength(*table));

    return ajTrue;
}




/* @funcstatic wordFindWordAtPos **********************************************
**
** Looks for a word at a given position
**
** @param [r] word [const char*] Word to find
** @param [r] seq1MatchTable [const AjPTable] Match table
** @param [r] nextpos [ajint] Not used
** @return [ajint] Position found (1 is the start) or 0 if none.
** @@
******************************************************************************/

static ajint wordFindWordAtPos(const char *word, const AjPTable seq1MatchTable,
			       ajint nextpos)
{
    EmbPWord wordmatch;
    ajint *k;
    ajint *pos;
    AjIList iter;

    k = &nextpos;

    wordmatch = ajTableFetch(seq1MatchTable, word);
    if(wordmatch)
    {
	iter = ajListIterNewread(wordmatch->list);

	while((pos = (ajint *) ajListIterGet(iter)))
	    if(*pos == *k)
	    {
		ajListIterDel(&iter);
		return *pos +1;
	    }

	ajListIterDel(&iter);
    }

    return 0;
}




/* @funcstatic wordGetWholeMatch **********************************************
**
** Looks for a word length match.
**
** @param [u] match [EmbPWordMatch] match structure
** @param [r] seq1MatchTable [const AjPTable] match table
** @return [ajint] Match position
** @@
******************************************************************************/

static ajint wordGetWholeMatch(EmbPWordMatch match,
			       const AjPTable seq1MatchTable)
{
    const AjPSeq seq2;
    const char *startptr;
    ajuint i = 0;
    ajuint ilast;
    ajint nextpos = 0;

    assert(wordLength > 0);

    seq2 = match->sequence;

    startptr = &(ajSeqGetSeqC(seq2)[match->seq2start+1]);

    i = match->seq2start;

    nextpos = match->seq1start + 1;

    ilast = ajSeqGetLen(seq2) - wordLength;
    while(i < (ilast+1))
    {
	/* find if it matches */
	if(!wordFindWordAtPos(startptr, seq1MatchTable, nextpos)) break;

	match->length++;
	nextpos++;
	i++;
	startptr++;
    }


    return (nextpos+wordLength) - (match->seq1start +1);
}




/* @funcstatic wordOrderMatchTable ********************************************
**
** Sort the hits by length then seq1 start then by seq2 start
**
** @param [u] unorderedList [AjPList] Unsorted list
** @return [void]
** @@
******************************************************************************/

static void wordOrderMatchTable(AjPList unorderedList)
{
    ajDebug("wordOrderMatchTable size %d\n", ajListGetLength(unorderedList));
    ajListSort(unorderedList, wordMatchCmp);

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
** @@
******************************************************************************/

static void wordOrderPosMatchTable(AjPList unorderedList)
{
    ajListSort(unorderedList, wordMatchCmpPos);

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
    const char *startptr;
    EmbPWord wordmatch;
    EmbPWordMatch match;
    EmbPWordMatch match2;
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
    AJNEW0(match);

    match->sequence = seq2;

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
	if((wordmatch = ajTableFetch(seq1MatchTable, startptr)))
	{
	    /* match found so create EmbSWordMatch structure and fill it
	    ** in. Then set next pos accordingly
	    ** BUT this could match several places so need to do for each
            ** position
            **
	    ** there is a match between the two sequences
	    ** this could extend an existing match or start a new one
	    */

	    newlist = wordmatch->list;

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
                        /* ajDebug("**match knew: %d kcur: %d kcur2: %d start1: %d "
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
                    AJNEW0(match2);
                    match2->sequence  = seq2;
                    match2->seq1start = knew;
                    match2->seq2start = i;
                    match2->length = wordLength;
                    /*ajDebug("save start1: %d start2: %d len: %d\n",
                            match2->seq1start, match2->seq2start,
                            match2->length);*/
                    ajListPushAppend(hitlist, match2); /* add to hitlist */
                    if(curiter)
                    {			/* add to wordCurList */
                        /*ajDebug("...ajListInsert using curiter %u\n",
                          ajListGetLength(wordCurList));*/
                        wordListInsertOld(curiter, match2);
                        /*wordCurListTrace(wordCurList);*/
                        /*wordCurIterTrace(curiter);*/
                    }
                    else
                    {
                        /*ajDebug("...ajListPushAppend to wordCurList %u\n",
                          ajListGetLength(wordCurList));*/
                        ajListPushAppend(wordCurList, match2);
                        /* wordCurListTrace(wordCurList); */
                    }
                }
 		/* ajDebug("k: %d i: %d\n", *k, i); */
	    }
	    ajListIterDel(&newiter);
            ajListIterDel(&curiter);
	}

	/* no match, so all existing matches are completed */

	i++;
	startptr++;
    }

    /* wordCurListTrace(hitlist); */
    if(orderit)
	wordOrderMatchTable(hitlist);

    /* wordCurListTrace(hitlist); */

    AJFREE(match);

    while(ajListPop(wordCurList,(void **)&ptr));

    return hitlist;
}




/* @funcstatic wordNewListTrace ***********************************************
**
** Reports contents of a word list.
**
** @param [r] i [ajint] Offset
** @param [r] newlist [const AjPList] word list.
** @return [void]
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




/* @func embWordMatchIter  ****************************************************
**
** Return the start positions and length for the next match.
** The caller iterates over the list, which is a standard AjPList
**
** @param [u] iter [AjIList] List iterator
** @param [w] start1 [ajint*] Start in first sequence
** @param [w] start2 [ajint*] Start in second sequence
** @param [w] len [ajint*] Length of match
** @return [AjBool] ajFalse if the iterator was exhausted
**
******************************************************************************/

AjBool embWordMatchIter(AjIList iter, ajint* start1, ajint* start2,
			ajint* len)
{
    EmbPWordMatch p;

    if(ajListIterDone(iter))
	return ajFalse;

    p = (EmbPWordMatch) ajListIterGet(iter);
    *start1 = p->seq1start;
    *start2 = p->seq2start;
    *len = p->length;

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
******************************************************************************/

void embWordUnused(void)
{
    EmbPWordMatch match;
    AjPTable ajptable = NULL;

    AJNEW0(match);

    wordGetWholeMatch(match,ajptable);
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
******************************************************************************/

void embWordExit(void)
{
    embWordClear();
    ajListFree(&wordCurList);

    return;
}

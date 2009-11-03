#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embWord_h
#define embWord_h




/* @data EmbPWordMatch ********************************************************
**
** NUCLEUS data structure for word matches
**
** @attr seq1start [ajuint] match start point in original sequence
** @attr seq2start [ajuint] match start point in comparison sequence
** @attr sequence [const AjPSeq] need in case we build multiple matches here
**                         so we know which one the match belongs to
** @attr length [ajint] length of match
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EmbSWordMatch {
  ajuint seq1start;
  ajuint seq2start;
  const AjPSeq sequence;
  ajint length;
  char Padding[4];
} EmbOWordMatch;
#define EmbPWordMatch EmbOWordMatch*




/* @data EmbPWord *************************************************************
**
** NUCLEUS data structure for words
**

** @attr fword [const char*] Original word
** @attr list [AjPList] List of words
** @attr count [ajint] Size of list
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EmbSWord {
  const char *fword;
  AjPList list;
  ajint count;
  char Padding[4];
} EmbOWord;
#define EmbPWord EmbOWord*




/*
** Prototype definitions
*/

AjPList embWordBuildMatchTable (const AjPTable seq1MatchTable,
				const AjPSeq seq2, ajint orderit);
void    embWordClear (void);
void    embWordExit(void);
void    embWordFreeTable(AjPTable *table);
AjBool  embWordGetTable (AjPTable *table, const AjPSeq seq);
void    embWordLength (ajint wordlen);
AjBool  embWordMatchIter (AjIList iter, ajint* start1, ajint* start2,
			  ajint* len);
void    embWordMatchListDelete (AjPList* plist);
void    embWordMatchListPrint (AjPFile file, const AjPList list);
void    embWordPrintTable  (const AjPTable table);
void    embWordPrintTableF (const AjPTable table, AjPFile outf);
void    embWordPrintTableFI (const AjPTable table, ajint mincount,
			     AjPFile outf);
void    embWordMatchListConvToFeat(const AjPList list,
				   AjPFeattable *tab1, AjPFeattable *tab2,
				   const AjPSeq seq1, const AjPSeq seq2);

void    embWordMatchMin(AjPList matchlist);
void    embWordUnused(void);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif



#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajpat_h
#define ajpat_h

/*
 *  Defines for string search algorithms
 */
#define AJALPHA  256			/* Alphabet			*/
#define AJMOD256 0xff
#define AJALPHA2 128			/* ASCII printable		*/
#define AJWORD   32			/* Size of a word		*/
#define AJBPS    1			/* Bits per state		*/




/* @data AjPPatBYPNode *******************************************************
**
** AJAX data structure for nodes in Baeza-Yates & Perleberg algorithm
**
** @attr next [struct AjSPatBYPNode*] Pointer to next node
** @attr offset [ajint] Offset
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSPatBYPNode
{
    struct AjSPatBYPNode *next;
    ajint offset;
    char  Padding[4];
} AjOPatBYPNode;
#define AjPPatBYPNode AjOPatBYPNode*




/* @data AjPPatComp **********************************************************
**
** AJAX data structure that holds all needed data for compiling and
** searching. Not including mismatch number.
**
** @alias AjSPatComp
** @alias AjOPatComp
**
** @attr pattern [AjPStr] Prosite pattern string
** @attr type [ajuint] Prosite pattern compile type
** @attr plen [ajuint] Prosite pattern length
** @attr buf [ajint*] Buffer for BMH search (can be -1)
** @attr off [AjOPatBYPNode[AJALPHA]] Offset buffer for B-Y/P search
** @attr sotable [ajuint*] Buffer for SHIFT-OR
** @attr solimit [ajuint] Limit for BMH search
** @attr m [ajuint] Real length of pattern (from embPatGetType)
** @attr regex [AjPStr] PCRE regexp string
** @attr skipm [ajuint**] Skip buffer for Tarhio-Ukkonen
** @attr amino [AjBool] Must match left begin
** @attr carboxyl [AjBool] Must match right
**
** @@
******************************************************************************/

typedef struct AjSPatComp
{
    AjPStr pattern;
    ajuint type;
    ajuint plen;
    ajint* buf;
    AjOPatBYPNode off[AJALPHA];
    ajuint* sotable;
    ajuint solimit;
    ajuint m;
    AjPStr regex;
    ajuint** skipm;
    AjBool amino;
    AjBool carboxyl;
} AjOPatComp;
#define AjPPatComp AjOPatComp*




/* @data AjPPatternSeq ********************************************************
**
** Ajax sequence pattern object.
**
** Holds definition of feature pattern. Regular expression patterns ignore
** mismatch value.
**
** @alias AjSPatternSeq
** @alias AjOPatternSeq
**
** @attr Name [AjPStr] Name.
** @attr Pattern [AjPStr] Pattern in string format.
** @attr Compiled [void*] Compiled version of the pattern.
** @attr Protein [AjBool] True if protein pattern
** @attr Mismatch [ajuint] Mismatch value.
**
** @new ajPatternSeqNew Default constructor
** @delete ajPatternSeqDel Default destructor
** @use ajPatternSeqGetName Returns name.
** @use ajPatternSeqGetPattern Returns pattern in string format.
** @use ajPatternSeqGetCompiled Returns pointer to compiled pattern.
** @use ajPatternSeqGetType Returns type as integer value.
** @use ajPatternSeqGetMismatch Return mismatch value.
** @@
******************************************************************************/

typedef struct AjSPatternSeq {
  AjPStr Name;
  AjPStr Pattern;
  void* Compiled;
  AjBool Protein;
  ajuint Mismatch;
} AjOPatternSeq;

#define AjPPatternSeq AjOPatternSeq*




/* @data AjPPatternRegex ******************************************************
**
** Ajax regular expression pattern object.
**
** Holds definition of feature pattern. Regular expression patterns ignore
** mismatch value.
**
** @alias AjSPatternRegex
** @alias AjOPatternRegex
**
** @attr Name [AjPStr] Name.
** @attr Pattern [AjPStr] Pattern in string format.
** @attr Compiled [AjPRegexp] Compiled version of the pattern.
** @attr Type [ajuint] Type.
** @attr Padding [char[4]] Padding to alignment boundary
**
** @new ajPatternRegexNew Default constructor
** @delete ajPatternRegexDel Default destructor
** @use ajPatternRegexGetName Returns name.
** @use ajPatternRegexGetPattern Returns pattern in string format.
** @use ajPatternRegexGetCompiled Returns pointer to compiled pattern.
** @use ajPatternRegexGetType Returns type as integer value.
** @@
******************************************************************************/

typedef struct AjSPatternRegex {
  AjPStr    Name;
  AjPStr    Pattern;
  AjPRegexp Compiled;
  ajuint    Type;
  char      Padding[4];
} AjOPatternRegex;

#define AjPPatternRegex AjOPatternRegex*

/*
** type can be 0: string, 1: prosite (protein) 2: prosite like (nucleotide)
*/
#define AJ_PAT_TYPE_STRING 0
#define AJ_PAT_TYPE_PRO 1
#define AJ_PAT_TYPE_NUCL 2




/* @data AjPPatlistSeq ********************************************************
**
** Ajax Pattern List object.
**
** Holds list of feature patterns and general information of them.
**
** @alias AjSPatlistSeq
** @alias AjOPatlistSeq
**
** @attr Patlist [AjPList] List for patterns.
** @attr Iter [AjIList] List iterator.
** @attr Protein [AjBool] True if protein
** @attr Padding [char[4]] Padding to alignment boundary
**
** @new ajPatlistNew Default constructor.
** @delete ajPatlistDel Default destructor.
** @modify ajPatlistRegexRead Reads the pattern file and fills the list.
** @modify ajPatlistSeqRead Reads the pattern file and fills the list.
** @modify ajPatlistRewind Restarts the iteration loop.
** @modify ajPatlistAdd Adds new pattern into list.
** @use ajPatlistGetNext Gets the next pattern from file and returns true if
**      available and false if not.
** @@
******************************************************************************/

typedef struct AjSPatlistSeq {
  AjPList Patlist;
  AjIList Iter;
  AjBool Protein;
  char   Padding[4];
} AjOPatlistSeq;

#define AjPPatlistSeq AjOPatlistSeq*




/* @data AjPPatlistRegex ******************************************************
**
** Ajax Pattern List object.
**
** Holds list of feature patterns and general information of them.
**
** @alias AjSPatlistRegex
** @alias AjOPatlistRegex
**
** @attr Patlist [AjPList] List for patterns.
** @attr Iter [AjIList] List iterator.
** @attr Type [ajuint] Type of expression
** @attr Padding [char[4]] Padding to alignment boundary
**
** @new ajPatlistNew Default constructor.
** @delete ajPatlistDel Default destructor.
** @modify ajPatlistRegexRead Reads the pattern file and fills the list.
** @modify ajPatlistSeqRead Reads the pattern file and fills the list.
** @modify ajPatlistRewind Restarts the iteration loop.
** @modify ajPatlistAdd Adds new pattern into list.
** @use ajPatlistGetNext Gets the next pattern from file and returns true if
**      available and false if not.
** @@
******************************************************************************/

typedef struct AjSPatlistRegex {
  AjPList Patlist;
  AjIList Iter;
  ajuint Type;
  char Padding[4];
} AjOPatlistRegex;

#define AjPPatlistRegex AjOPatlistRegex*

/*
** Prototype definitions
*/

AjPPatternSeq ajPatternSeqNewList (AjPPatlistSeq plist, const AjPStr name,
				   const AjPStr pat, ajuint mismatch);
void ajPatternSeqDel (AjPPatternSeq* pthys);
const AjPStr ajPatternSeqGetName (const AjPPatternSeq thys);
const AjPStr ajPatternSeqGetPattern (const AjPPatternSeq thys);
AjPPatComp ajPatternSeqGetCompiled (const AjPPatternSeq thys);
AjBool ajPatternSeqGetProtein (const AjPPatternSeq thys);
ajuint ajPatternSeqGetMismatch (const AjPPatternSeq thys);
void ajPatternSeqSetCompiled (AjPPatternSeq thys, void* pat);
void ajPatternSeqDebug (const AjPPatternSeq pat);

AjPPatternRegex ajPatternRegexNewList (AjPPatlistRegex plist,
				       const AjPStr name,
				       const AjPStr pat);
void ajPatternRegexDel (AjPPatternRegex* pthys);
const AjPStr ajPatternRegexGetName (const AjPPatternRegex thys);
const AjPStr ajPatternRegexGetPattern (const AjPPatternRegex thys);
AjPRegexp ajPatternRegexGetCompiled (const AjPPatternRegex thys);
ajuint ajPatternRegexGetType (const AjPPatternRegex thys);
void ajPatternRegexSetCompiled (AjPPatternRegex thys, AjPRegexp pat);
void ajPatternRegexDebug (const AjPPatternRegex pat);

/* Patlist handling functions */
AjPPatlistSeq ajPatlistSeqNewType (AjBool Protein);
AjPPatlistRegex ajPatlistRegexNewType (ajuint type);
AjPPatlistRegex ajPatlistRegexNew (void);
AjPPatlistSeq ajPatlistSeqNew (void);
void ajPatlistSeqDel (AjPPatlistSeq* pthys);
void ajPatlistRegexDel (AjPPatlistRegex* pthys);
AjPPatlistRegex ajPatlistRegexRead(const AjPStr patspec,
				   const AjPStr patname,
				   const AjPStr fmt,
				   ajuint type, AjBool upper, AjBool lower);
AjPPatlistSeq ajPatlistSeqRead(const AjPStr patspec,
			       const AjPStr patname,
			       const AjPStr fmt,
			       AjBool protein, ajuint mismatches);
AjBool ajPatlistRegexGetNext (AjPPatlistRegex thys,
			      AjPPatternRegex* pattern);
AjBool ajPatlistSeqGetNext (AjPPatlistSeq thys,
			    AjPPatternSeq* pattern);
void ajPatlistSeqRewind (AjPPatlistSeq thys);
void ajPatlistRegexRewind (AjPPatlistRegex thys);
void ajPatlistSeqRemoveCurrent (AjPPatlistSeq thys);
void ajPatlistRegexRemoveCurrent (AjPPatlistRegex thys);
void ajPatlistAddRegex (AjPPatlistRegex thys, AjPPatternRegex pat);
void ajPatlistAddSeq (AjPPatlistSeq thys, AjPPatternSeq pat);
ajuint ajPatlistSeqGetSize(const AjPPatlistSeq plist);
ajuint ajPatlistRegexGetSize(const AjPPatlistRegex plist);
ajuint ajPatlistRegexDoc(AjPPatlistRegex thys, AjPStr* pdoc);
ajuint ajPatlistSeqDoc(AjPPatlistSeq thys, AjPStr* pdoc);

AjPPatComp	ajPatCompNew (void);
void		ajPatCompDel (AjPPatComp* pthys);
ajuint ajPatternRegexType(const AjPStr type);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif

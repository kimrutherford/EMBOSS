#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajindex_h
#define ajindex_h


#define BT_PAGESIZE BUFSIZ      /* Default cache page size       */
#define BT_MAXRETRIES	100	/* Maximum number of read/write attempts */

#define BT_LOCK  2
#define BT_DIRTY 1
#define BT_CLEAN 0

#define BT_READ  ajTrue
#define BT_WRITE ajFalse


#define BT_ROOT     1
#define BT_INTERNAL 2
#define BT_LEAF	    4
#define BT_BUCKET   8
#define BT_OVERFLOW 16
#define BT_PRIBUCKET 32
#define BT_SECBUCKET 64

#define BTNO_BALANCE 100L
#define BTNO_NODE    100L


#define ID_EXTENSION "xid"
#define AC_EXTENSION "xac"
#define SV_EXTENSION "xsv"
#define DE_EXTENSION "xde"
#define KW_EXTENSION "xkw"
#define TX_EXTENSION "xtx"


/* Useful page filling values */

/*
 *  Length of integers after the ID string in a primary bucket 
 *  i.e. dbno + dups + offset + refoffset
 */
#define BT_DDOFFROFF (sizeof(ajint) + sizeof(ajint) + sizeof(ajlong) +  \
                      sizeof(ajlong))

/*
 *  Preamble before key lengths in an internal node
 * i.e. nodetype+blocknumber+nkeys+totlen+left+right+overflow+prev
 */
#define BT_NODEPREAMBLE (sizeof(ajint) + sizeof(ajlong) + sizeof(ajint) + \
    sizeof(ajint) + sizeof(ajlong) + sizeof(ajlong) + sizeof(ajlong) + \
                         sizeof(ajlong))

/*
 *  This is the length of extra pointer required in primary index
 *  page space calculations i.e. the total space taken by
 *  key/pointer pairs in an internal page is:
 *  BT_PTRLEN + ((keylenmax + sizeof(long))*nkeys)
 *  This is because there is one more pointer than the
 *  value of 'order' in a B+ tree.
 */
#define BT_PTRLEN sizeof(ajlong)


/*
 *  Length of extra space, on top of a key string, taken up by a
 *  key in an internal node. This is an ajint holding the length
 *  of a string plus the length of a block pointer (ajlong)
 */
#define BT_IDKEYEXTRA (sizeof(ajint) + sizeof(ajlong))


/*
 *  Preamble before key lengths in an id bucket
 *  i.e. nodetype+nentries+overflow
 */
#define BT_BUCKPREAMBLE (sizeof(ajint) + sizeof(ajint) + sizeof(ajlong))


/*
 *  Size of a key in a key length table block in an internal node or bucket
 */
#define BT_KEYLENENTRY sizeof(ajint)

/*
 *  Size of an offset key
 */
#define BT_OFFKEYLEN sizeof(ajlong)

/*
 *  Length of integers per entry in a num bucket 
 *  i.e. dbno + offset + refoffset
 */
#define BT_DOFFROFF (sizeof(ajint) + sizeof(ajlong) + sizeof(ajlong))
    



/* @data AjPBtNode ***********************************************************
**
** Btree node
**
** @attr BlockOffset [ajlong] Offset within mainindex
** @attr Nkeys [ajint] Number of keys filled
** @attr TotLen [ajint] Total length of keys
** @attr Left [ajlong] Left Sibling
** @attr Right [ajlong] Right Sibling
** @attr Overflow [ajlong] Offset to overflow block
** @attr PrevNode [ajlong] Previous node
** @attr NodeType [ajint] Root, Internal or Leaf
** @attr Padding [char[4]] Padding to alignment boundary
*****************************************************************************/

typedef struct AjSBtNode
{
    ajlong BlockOffset;
    ajint  Nkeys;
    ajint  TotLen;
    ajlong Left;
    ajlong Right;
    ajlong Overflow;
    ajlong PrevNode;
    ajint  NodeType;
    char Padding[4];
} AjOBtNode;
#define AjPBtNode AjOBtNode*




/* @data AjPBtMem ***********************************************************
**
** Dynamic list for btree memory arrays
**
** @attr next [struct AjSBtMem*] next node
** @attr prev [struct AjSBtMem*] previous node
** @attr karray [AjPStr*] key array (primary trees)
** @attr parray [ajlong*] pointer arrays (primary and secondary trees)
** @attr overflows [ajlong*] overflows (primary) and keys (secondary)
** @attr used [AjBool] node in use
** @attr Padding [char[4]] Padding to alignment boundary
*****************************************************************************/

typedef struct AjSBtMem
{
    struct AjSBtMem *next;
    struct AjSBtMem *prev;
    AjPStr *karray;
    ajlong *parray;
    ajlong *overflows;
    AjBool used;
    char Padding[4];
} AjOBtMem;
#define AjPBtMem AjOBtMem*




/* @data AjPBtId ***************************************************
**
** Btree ID
**
** @attr id [AjPStr] Unique ID
** @attr dbno [ajuint] Database file number
** @attr dups [ajuint] Duplicates
** @attr offset [ajlong] Offset within database file (ftello)
** @attr refoffset [ajlong] Offset within reference database file (ftello)
******************************************************************************/

typedef struct AjSBtId
{
    AjPStr id;
    ajuint  dbno;
    ajuint  dups;
    ajlong offset;
    ajlong refoffset;
} AjOBtId;
#define AjPBtId AjOBtId*




/* @data AjPBtWild ***************************************************
**
** Btree wildcard
**
** @attr id [AjPStr] Wildcard ID
** @attr pageno [ajlong] Page number of leaf
** @attr list [AjPList] list of AjPBtIds
** @attr first [AjBool] true for first search
** @attr Padding [char[4]] Padding to alignment boundary
******************************************************************************/

typedef struct AjSBtWild
{
    AjPStr id;
    ajlong pageno;
    AjPList list;
    AjBool first;
    char Padding[4];
} AjOBtWild;
#define AjPBtWild AjOBtWild*




/* @data AjPBucket ***************************************************
**
** Bucket structure on disc
**
** Key, filenumber, ftell ID, subkey page (char*, ajint, ajlong, ajlong)
**
** @attr NodeType [ajint] Node type
** @attr Nentries [ajint] Number of entries
** @attr Overflow [ajlong] Offset to overflow block
** @attr keylen [ajint*] key lengths
** @attr Ids [AjPBtId*] Ids
******************************************************************************/

typedef struct AjSBucket
{
    ajint    NodeType;
    ajint    Nentries;
    ajlong   Overflow;
    ajint    *keylen;
    AjPBtId *Ids;
} AjOBucket;
#define AjPBucket AjOBucket*




/* @data AjPBtNumId ***************************************************
**
** Btree ID
**
** @attr offset [ajlong] Offset within database file (ftello)
** @attr refoffset [ajlong] Offset within reference database file (ftello)
** @attr dbno [ajint] Database file number
** @attr Padding [char[4]] Padding to alignment boundary
******************************************************************************/

typedef struct AjSBtNumId
{
    ajlong offset;
    ajlong refoffset;
    ajint  dbno;
    char Padding[4];
} AjOBtNumId;
#define AjPBtNumId AjOBtNumId*




/* @data AjPNumBucket ***************************************************
**
** Offset bucket structure on disc
**
** Key, filenumber, ftell ID, subkey page (char*, ajint, ajlong, ajlong)
**
** @attr NodeType [ajint] Node type
** @attr Nentries [ajint] Number of entries
** @attr Overflow [ajlong] Offset to overflow block
** @attr NumId [AjPBtNumId*] secondary tree IDs
******************************************************************************/

typedef struct AjSNumBucket
{
    ajint    NodeType;
    ajint    Nentries;
    ajlong   Overflow;
    AjPBtNumId *NumId;
} AjONumBucket;
#define AjPNumBucket AjONumBucket*




/* Database file name structure
**
** ajint        order			Order of B+tree
** ajint        m			Max entries per bucket
** ajint	NFiles			Number of Indexed files
** ajint        TotalLen                Total length if dir/name entries
** Directory/FileName pairs
*/

#if defined(LENDIAN)
#define BT_GETAJINT(p,v) (memcpy((void*)v,(void*)p,sizeof(ajint)))
#define BT_GETAJUINT(p,v) (memcpy((void*)v,(void*)p,sizeof(ajuint)))
#define BT_GETAJLONG(p,v) (memcpy((void*)v,(void*)p,sizeof(ajlong)))
#define BT_SETAJINT(p,v) (memcpy((void*)p,(void*)&v,sizeof(ajint)))
#define BT_SETAJLONG(p,v) (memcpy((void*)p,(void*)&v,sizeof(ajlong)))
#else
#define BT_GETAJINT(p,v) memcpy((void*)v,(void*)p,sizeof(ajint)); \
                         ajByteRevInt(v)
#define BT_GETAJUINT(p,v) memcpy((void*)v,(void*)p,sizeof(ajuint)); \
                         ajByteRevUint(v)
#define BT_GETAJLONG(p,v) memcpy((void*)v,(void*)p,sizeof(ajlong)); \
                          ajByteRevLong(v)
#define BT_SETAJINT(p,v)  ajByteRevInt(&v); \
                          memcpy((void*)p,(void*)&v,sizeof(ajint))
#define BT_SETAJLONG(p,v) ajByteRevLong(&v); \
                          memcpy((void*)p,(void*)&v,sizeof(ajlong))
#endif


#define BT_BUCKIDLEN(str) (ajStrGetLen(str) + 1 + sizeof(ajint) + \
			   sizeof(ajint) + sizeof(ajlong) + \
			   sizeof(ajlong))


/*
** Macros to determine entry positions within a bucket
*/

#define PBT_BUCKNODETYPE(p) p
#define PBT_BUCKNENTRIES(p) (p + sizeof(ajint))
#define PBT_BUCKOVERFLOW(p) (p + sizeof(ajint) + sizeof(ajint))
#define PBT_BUCKKEYLEN(p) (p + sizeof(ajint) + sizeof(ajint) + sizeof(ajlong))
#define BT_BUCKPRILEN(str) (ajStrGetLen(str) + 1 + sizeof(ajlong))
#define BT_BUCKSECLEN(str) (ajStrGetLen(str) +1)

/*
** Macros to return a page entry value within a bucket
*/

#if defined(LENDIAN)
#define GBT_BUCKNODETYPE(p,v) (memcpy((void*)v,(void*)PBT_BUCKNODETYPE(p), \
				      sizeof(ajint)))
#define GBT_BUCKNENTRIES(p,v) (memcpy((void*)v,(void*)PBT_BUCKNENTRIES(p), \
				      sizeof(ajint)))
#define GBT_BUCKOVERFLOW(p,v) (memcpy((void*)v,(void*)PBT_BUCKOVERFLOW(p), \
				      sizeof(ajlong)))
#else
#define GBT_BUCKNODETYPE(p,v) memcpy((void*)v,(void*)PBT_BUCKNODETYPE(p), \
				      sizeof(ajint)); \
                              ajByteRevInt(v)
#define GBT_BUCKNENTRIES(p,v) memcpy((void*)v,(void*)PBT_BUCKNENTRIES(p), \
                                      sizeof(ajint)); \
                              ajByteRevInt(v)
#define GBT_BUCKOVERFLOW(p,v) memcpy((void*)v,(void*)PBT_BUCKOVERFLOW(p), \
				      sizeof(ajlong)); \
                              ajByteRevLong(v)
#endif


/*
** Macros to set a page entry value within an internal/leaf node
*/

#if defined(LENDIAN)
#define SBT_BUCKNODETYPE(p,v) (memcpy((void*)PBT_BUCKNODETYPE(p), \
				      (const void*)&v,sizeof(ajint)))
#define SBT_BUCKNENTRIES(p,v) (memcpy((void*)PBT_BUCKNENTRIES(p), \
				      (const void*)&v,sizeof(ajint)))
#define SBT_BUCKOVERFLOW(p,v) (memcpy((void*)PBT_BUCKOVERFLOW(p), \
				      (const void*)&v,sizeof(ajlong)))
#else
#define SBT_BUCKNODETYPE(p,v) ajByteRevInt(&v); \
                              memcpy((void*)PBT_BUCKNODETYPE(p), \
				      (const void*)&v,sizeof(ajint))
#define SBT_BUCKNENTRIES(p,v) ajByteRevInt(&v); \
                              memcpy((void*)PBT_BUCKNENTRIES(p), \
				     (const void*)&v,sizeof(ajint))
#define SBT_BUCKOVERFLOW(p,v) ajByteRevLong(&v); \
                              memcpy((void*)PBT_BUCKOVERFLOW(p), \
				     (const void*)&v,sizeof(ajlong))
#endif

/*
** Macros to determine entry positions within an internal/leaf node
*/

#define PBT_NODETYPE(p) p
#define PBT_BLOCKNUMBER(p) (p + sizeof(ajint))
#define PBT_NKEYS(p) (p + sizeof(ajint) + sizeof(ajlong))
#define PBT_TOTLEN(p) (p+sizeof(ajint)+sizeof(ajlong)+sizeof(ajint))
#define PBT_LEFT(p) (p+sizeof(ajint)+sizeof(ajlong)+sizeof(ajint) \
		     +sizeof(ajint))
#define PBT_RIGHT(p) (p+sizeof(ajint)+sizeof(ajlong)+sizeof(ajint) \
		      +sizeof(ajint)+sizeof(ajlong))
#define PBT_OVERFLOW(p) (p+sizeof(ajint)+sizeof(ajlong)+sizeof(ajint) \
		         +sizeof(ajint)+sizeof(ajlong)+sizeof(ajlong))
#define PBT_PREV(p) (p+sizeof(ajint)+sizeof(ajlong)+sizeof(ajint) \
		     +sizeof(ajint)+sizeof(ajlong)+sizeof(ajlong) \
		     +sizeof(ajlong))
#define PBT_KEYLEN(p) (p+sizeof(ajint)+sizeof(ajlong)+sizeof(ajint) \
		       +sizeof(ajint)+sizeof(ajlong)+sizeof(ajlong) \
		       +sizeof(ajlong)+sizeof(ajlong))

/*
** Macros to return a page entry value within an internal/leaf node
*/

#if defined(LENDIAN)
#define GBT_NODETYPE(p,v) (memcpy((void*)v,(void*)PBT_NODETYPE(p), \
				  sizeof(ajint)))
#define GBT_BLOCKNUMBER(p,v) (memcpy((void*)v,(void*)PBT_BLOCKNUMBER(p), \
				     sizeof(ajlong)))
#define GBT_NKEYS(p,v) (memcpy((void*)v,(void*)PBT_NKEYS(p), \
			       sizeof(ajint)))
#define GBT_TOTLEN(p,v) (memcpy((void*)v,(void*)PBT_TOTLEN(p), \
			       sizeof(ajint)))
#define GBT_LEFT(p,v) (memcpy((void*)v,(void*)PBT_LEFT(p), \
			      sizeof(ajlong)))
#define GBT_RIGHT(p,v) (memcpy((void*)v,(void*)PBT_RIGHT(p), \
			       sizeof(ajlong)))
#define GBT_PREV(p,v) (memcpy((void*)v,(void*)PBT_PREV(p), \
			      sizeof(ajlong)))
#define GBT_OVERFLOW(p,v) (memcpy((void*)v,(void*)PBT_OVERFLOW(p), \
				  sizeof(ajlong)))
#else
#define GBT_NODETYPE(p,v) memcpy((void*)v,(void*)PBT_NODETYPE(p), \
				 sizeof(ajint)); \
                          ajByteRevInt(v)
#define GBT_BLOCKNUMBER(p,v) memcpy((void*)v,(void*)PBT_BLOCKNUMBER(p), \
				     sizeof(ajlong)); \
                             ajByteRevLong(v)
#define GBT_NKEYS(p,v) memcpy((void*)v,(void*)PBT_NKEYS(p), \
			       sizeof(ajint)); \
                       ajByteRevInt(v)
#define GBT_TOTLEN(p,v) memcpy((void*)v,(void*)PBT_TOTLEN(p), \
			       sizeof(ajint)); \
                        ajByteRevInt(v)
#define GBT_LEFT(p,v) memcpy((void*)v,(void*)PBT_LEFT(p), \
                             sizeof(ajlong)); \
                      ajByteRevLong(v)

#define GBT_RIGHT(p,v) memcpy((void*)v,(void*)PBT_RIGHT(p), \
			      sizeof(ajlong)); \
                       ajByteRevLong(v)
#define GBT_PREV(p,v) memcpy((void*)v,(void*)PBT_PREV(p), \
			      sizeof(ajlong)); \
                      ajByteRevLong(v)
#define GBT_OVERFLOW(p,v) memcpy((void*)v,(void*)PBT_OVERFLOW(p), \
				  sizeof(ajlong)); \
                          ajByteRevLong(v)
#endif


/*
** Macros to set a page entry value within an internal/leaf node
*/

#if defined(LENDIAN)
#define SBT_NODETYPE(p,v) (memcpy((void*)PBT_NODETYPE(p),(const void*)&v, \
				  sizeof(ajint)))
#define SBT_BLOCKNUMBER(p,v) (memcpy((void*)PBT_BLOCKNUMBER(p), \
				     (const void*)&v,sizeof(ajlong)))
#define SBT_NKEYS(p,v) (memcpy((void*)PBT_NKEYS(p),(const void*)&v, \
			       sizeof(ajint)))
#define SBT_TOTLEN(p,v) (memcpy((void*)PBT_TOTLEN(p),(const void*)&v, \
				sizeof(ajint)))
#define SBT_LEFT(p,v) (memcpy((void*)PBT_LEFT(p), \
			      (const void*)&v,sizeof(ajlong)))
#define SBT_RIGHT(p,v) (memcpy((void*)PBT_RIGHT(p), \
			       (const void*)&v,sizeof(ajlong)))
#define SBT_PREV(p,v) (memcpy((void*)PBT_PREV(p), \
			      (const void*)&v,sizeof(ajlong)))
#define SBT_OVERFLOW(p,v) (memcpy((void*)PBT_OVERFLOW(p), \
				  (const void*)&v,sizeof(ajlong)))
#else
#define SBT_NODETYPE(p,v) ajByteRevInt(&v); \
                          memcpy((void*)PBT_NODETYPE(p),(const void*)&v, \
				  sizeof(ajint))
#define SBT_BLOCKNUMBER(p,v) ajByteRevLong(&v); \
                             memcpy((void*)PBT_BLOCKNUMBER(p), \
				     (const void*)&v,sizeof(ajlong))
#define SBT_NKEYS(p,v) ajByteRevInt(&v); \
                       memcpy((void*)PBT_NKEYS(p),(const void*)&v, \
			       sizeof(ajint))
#define SBT_TOTLEN(p,v) ajByteRevInt(&v); \
                        memcpy((void*)PBT_TOTLEN(p),(const void*)&v, \
				sizeof(ajint))
#define SBT_LEFT(p,v) ajByteRevLong(&v); \
                      memcpy((void*)PBT_LEFT(p), \
			      (const void*)&v,sizeof(ajlong))
#define SBT_RIGHT(p,v) ajByteRevLong(&v); \
                       memcpy((void*)PBT_RIGHT(p), \
			       (const void*)&v,sizeof(ajlong))
#define SBT_PREV(p,v) ajByteRevLong(&v); \
                      memcpy((void*)PBT_PREV(p), \
			      (const void*)&v,sizeof(ajlong))
#define SBT_OVERFLOW(p,v) ajByteRevLong(&v); \
                          memcpy((void*)PBT_OVERFLOW(p), \
				 (const void*)&v,sizeof(ajlong))
#endif




/* @data AjPBtpage ***************************************************
**
** Btree page
**
** @attr pageno [ajlong] Page number
** @attr next [struct AjSBtpage*] Next page
** @attr prev [struct AjSBtpage*] Previous page
** @attr buf [unsigned char*] Buffer
** @attr dirty [ajint] BT_DIRTY if page needs to be written to disc
** @attr Padding [char[4]] Padding to alignment boundary
******************************************************************************/

typedef struct AjSBtpage
{
    ajlong pageno;
    struct AjSBtpage *next;
    struct AjSBtpage *prev;
    unsigned char *buf;
    ajint  dirty;
    char Padding[4];
} AjOBtpage;
#define AjPBtpage AjOBtpage*


/* @data AjPBtcache ***************************************************
**
** B tree cache
**
** @attr fp [FILE*] Tree index file pointer
** @attr totsize [ajlong] Tree index length
** @attr lru [AjPBtpage] Least recently used cache page
** @attr mru [AjPBtpage] Most recently used cache page
** @attr pagesize [ajint] Size of cache pages
** @attr listLength [ajint] Number of pages in cache
** @attr order [ajint] Order of primary tree
** @attr level [ajint] Depth of primary tree
** @attr cachesize [ajint] Maximum number of pages to cache
** @attr nperbucket [ajint] Number of entries in a primary bucket
** @attr replace [AjPStr] Replacement ID
** @attr numreplace [ajlong] Replacement numeric ID
** @attr count [ajlong] Number of entries indexed
** @attr deleted [AjBool] Deletion flag
** @attr slevel [ajint] Depth of secondary tree
** @attr sorder [ajint] Order of secondary tree
** @attr snperbucket [ajint] Number of entries in a secondary bucket
** @attr secrootblock [ajlong] Secondary tree root block
** @attr bmem [AjPBtMem] Primary array allocation MRU bottom
** @attr tmem [AjPBtMem] Primary array allocation MRU top
** @attr bsmem [AjPBtMem] Secondary array allocation MRU bottom
** @attr tsmem [AjPBtMem] Secondary array allocation MRU top
** @attr kwlimit [ajint] Max length of secondary key
** @attr Padding [char[4]] Padding to alignment boundary
******************************************************************************/

typedef struct AjSBtCache
{
    FILE *fp;
    ajlong totsize;
    AjPBtpage lru;
    AjPBtpage mru;
    ajint pagesize;
    ajint listLength;
    ajint order;
    ajint level;
    ajint cachesize;
    ajint nperbucket;
    AjPStr replace;
    ajlong numreplace;
    ajlong count;
    AjBool deleted;
    ajint slevel;
    ajint sorder;
    ajint snperbucket;
    ajlong secrootblock;
    AjPBtMem bmem;
    AjPBtMem tmem;
    AjPBtMem bsmem;
    AjPBtMem tsmem;
    ajint  kwlimit;
    char Padding[4];
} AjOBtcache;
#define AjPBtcache AjOBtcache*




/* @data AjPBtPri ***************************************************
**
** Btree primary keyword
**
** @attr keyword [AjPStr] keyword
** @attr treeblock [ajlong] disc block of secondary tree
** @attr id [AjPStr] Id string
******************************************************************************/

typedef struct AjSBtPri
{
    AjPStr keyword;
    ajlong treeblock;
    AjPStr id;
} AjOBtPri;
#define AjPBtPri AjOBtPri*




/* @data AjPPriBucket ***************************************************
**
** Keyword primary bucket structure on disc
**
** @attr NodeType [ajint] Node type
** @attr Nentries [ajint] Number of entries
** @attr Overflow [ajlong] Offset to overflow block
** @attr keylen [ajint*] key lengths
** @attr codes [AjPBtPri*] Primary keywords
******************************************************************************/

typedef struct AjSPriBucket
{
    ajint    NodeType;
    ajint    Nentries;
    ajlong   Overflow;
    ajint    *keylen;
    AjPBtPri *codes;
} AjOPriBucket;
#define AjPPriBucket AjOPriBucket*




/* @data AjPSecBucket ***************************************************
**
** Keyword secondary bucket structure on disc
**
** @attr NodeType [ajint] Node type
** @attr Nentries [ajint] Number of entries
** @attr Overflow [ajlong] Offset to overflow block
** @attr keylen [ajint*] key lengths
** @attr ids [AjPStr*] Ids
******************************************************************************/

typedef struct AjSSecBucket
{
    ajint    NodeType;
    ajint    Nentries;
    ajlong   Overflow;
    ajint    *keylen;
    AjPStr   *ids;
} AjOSecBucket;
#define AjPSecBucket AjOSecBucket*




/* @data AjPBtKeyWild ***************************************************
**
** Btree keyword wildcard object
**
** @attr keyword [AjPStr] Wildcard keyword
** @attr pageno [ajlong] Page number of primary tree leaf
** @attr list [AjPList] list of AjPBtPris
** @attr cache [AjPBtcache] cache for secondary tree
** @attr idlist [AjPList] list of AjPStr IDs
** @attr secpageno [ajlong] Page number of secondary tree leaf
** @attr first [AjBool] true for first search
** @attr Padding [char[4]] Padding to alignment boundary
******************************************************************************/

typedef struct AjSBtKeyWild
{
    AjPStr keyword;
    ajlong pageno;
    AjPList list;
    AjPBtcache cache;
    AjPList idlist;
    ajlong secpageno;
    AjBool first;
    char Padding[4];
} AjOBtKeyWild;
#define AjPBtKeyWild AjOBtKeyWild*




/* @data AjPBtHybrid ***************************************************
**
** Btree ID
**
** @attr key1 [AjPStr] Unique ID
** @attr dbno [ajint] Database file number
** @attr dups [ajint] Duplicates
** @attr offset [ajlong] Offset within database file (ftello)
** @attr refoffset [ajlong] Offset within reference database file (ftello)
** @attr treeblock [ajlong] Secondary tree root page
******************************************************************************/

typedef struct AjSBtHybrid
{
    AjPStr key1;
    ajint  dbno;
    ajint  dups;
    ajlong offset;
    ajlong refoffset;
    ajlong treeblock;
} AjOBtHybrid;
#define AjPBtHybrid AjOBtHybrid*




/*
** Prototype definitions
*/

AjPBtcache ajBtreeCacheNewC(const char *file, const char *ext,
			    const char *idir, const char *mode,
			    ajint pagesize, ajint order, ajint fill,
			    ajint level, ajint cachesize);
AjPBtpage  ajBtreeCacheRead(AjPBtcache cache, ajlong pageno);
AjPBtpage  ajBtreeCacheWrite(AjPBtcache cache, ajlong pageno);
void       ajBtreeCreateRootNode(AjPBtcache cache, ajlong rootpage);
AjPBtpage  ajBtreeFindInsert(AjPBtcache cache, const char *key);

void     ajBtreeCacheDel(AjPBtcache *thys);
void     ajBtreeInsertId(AjPBtcache cache, const AjPBtId id);
void     ajBtreeIdDel(AjPBtId *thys);
AjPBtId  ajBtreeIdNew(void);
AjPBtId  ajBtreeIdFromKey(AjPBtcache cache, const char *key);
void     ajBtreeWriteParams(const AjPBtcache cache, const char *fn,
			    const char *ext, const char *idir);
void     ajBtreeReadParams(const char *fn, const char *ext,
			   const char *idir, ajint *order,
			   ajint *nperbucket, ajint *pagesize, ajint *level,
			   ajint *cachesize, ajint *sorder,
			   ajint *snperbucket, ajlong *count, ajint *kwlimit);
void     ajBtreeCacheSync(AjPBtcache cache, ajlong rootpage);

AjPBtWild    ajBtreeWildNew(AjPBtcache cache, const AjPStr wild);
AjPBtKeyWild ajBtreeKeyWildNew(AjPBtcache cache, const AjPStr wild);
void         ajBtreeWildDel(AjPBtWild *thys);
void         ajBtreeKeyWildDel(AjPBtKeyWild *thys);

AjPBtpage  ajBtreeFindInsertW(AjPBtcache cache, const char *key);
AjPBtId    ajBtreeIdFromKeyW(AjPBtcache cache, AjPBtWild wild);
void       ajBtreeListFromKeyW(AjPBtcache cache, const char *key,
			       AjPList idlist);
AjPBtId    ajBtreeIdFromKeywordW(AjPBtcache cache, AjPBtKeyWild wild,
				 AjPBtcache idcache);
void       ajBtreeListFromKeywordW(AjPBtcache cache, const char *key,
				   AjPBtcache idcache, AjPList btidlist);

AjBool     ajBtreeReplaceId(AjPBtcache cache, const AjPBtId rid);

ajint      ajBtreeReadEntries(const char *filename, const char *indexdir,
			      const char *directory,
			      AjPStr **seqfiles, AjPStr **reffiles);
void       ajBtreeInsertDupId(AjPBtcache cache, AjPBtId id);
AjPList    ajBtreeDupFromKey(AjPBtcache cache, const char *key);




AjPBtPri   ajBtreePriNew(void);
void       ajBtreePriDel(AjPBtPri *thys);
AjPBtPri   ajBtreePriFromKeyword(AjPBtcache cache, const char *key);

AjPBtcache ajBtreeSecCacheNewC(const char *file, const char *ext,
			       const char *idir, const char *mode,
			       ajint pagesize, ajint order, ajint fill,
			       ajint level, ajint cachesize,
			       ajint sorder, ajint slevel, ajint sfill,
			       ajlong count, ajint kwlimit);
AjPBtpage  ajBtreeSecFindInsert(AjPBtcache cache, const char *key);
void       ajBtreeInsertSecId(AjPBtcache cache, const AjPStr id);
AjBool     ajBtreeSecFromId(AjPBtcache cache, const char *key);

AjPList    ajBtreeSecLeafList(AjPBtcache cache, ajlong rootblock);
AjBool     ajBtreeVerifyId(AjPBtcache cache, ajlong rootblock, const char *id);

void       ajBtreeInsertKeyword(AjPBtcache cache, const AjPBtPri pri);

void       ajBtreeLockTest(AjPBtcache cache);



AjPBtpage   ajBtreeHybFindInsert(AjPBtcache cache, const char *key);
AjPBtpage   ajBtreeNumFindInsert(AjPBtcache cache, const ajlong key);

void        ajBtreeInsertNum(AjPBtcache cache, const AjPBtNumId num,
			     AjPBtpage page);
void        ajBtreeHybInsertId(AjPBtcache cache, const AjPBtHybrid hyb);
AjPBtHybrid ajBtreeHybNew(void);
void        ajBtreeHybDel(AjPBtHybrid *thys);
void        ajBtreeFreePriArray(AjPBtcache cache);
void        ajBtreeFreeSecArray(AjPBtcache cache);
void 	    ajBtreeHybLeafList(AjPBtcache cache, ajlong rootblock,
			       const AjPStr idname, AjPList list);

void        ajBtreeDumpHybKeys(AjPBtcache cache, ajint dmin, ajint dmax,
			       AjPFile outf);

AjBool ajBtreeDeleteHybId(AjPBtcache cache, const AjPBtHybrid hyb);
AjBool ajBtreeDeletePriId(AjPBtcache cache, const AjPBtPri pri);
    

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajindex_h
#define ajindex_h


#define BT_PAGESIZE BUFSIZ      /* Default cache page size
                                **  use C default buffer size */
#define BT_CACHESIZE 100

#define BT_MAXRETRIES	100	/* Maximum number of read/write attempts */
#define BT_KWLIMIT     15

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
#define BT_NUMBUCKET 128
#define BT_FREEPAGE 256

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
#define BT_DDOFFROFF (sizeof(ajuint) + sizeof(ajuint) + sizeof(ajulong) +  \
                      sizeof(ajulong))

/*
 *  Preamble before key lengths in an internal node
 * i.e. nodetype+blocknumber+nkeys+totlen+left+right+overflow+prev
 */
#define BT_NODEPREAMBLE (sizeof(ajuint) + sizeof(ajulong) + sizeof(ajuint) + \
    sizeof(ajuint) + sizeof(ajulong) + sizeof(ajulong) + sizeof(ajulong) + \
                         sizeof(ajulong))

/*
 *  This is the length of extra pointer required in primary index
 *  page space calculations i.e. the total space taken by
 *  key/pointer pairs in an internal page is:
 *  BT_PTRLEN + ((keylenmax + sizeof(long))*nkeys)
 *  This is because there is one more pointer than the
 *  value of 'order' in a B+ tree.
 */
#define BT_PTRLEN sizeof(ajulong)


/*
 *  Length of extra space, on top of a key string, taken up by a
 *  key in an internal node. This is an ajuint holding the length
 *  of a string plus the length of a block pointer (ajulong)
 */
#define BT_IDKEYEXTRA (sizeof(ajuint) + sizeof(ajulong))


/*
 *  Preamble before key lengths in an id bucket
 *  i.e. nodetype+nentries+overflow
 */
#define BT_BUCKPREAMBLE (sizeof(ajuint) + sizeof(ajuint) + sizeof(ajulong))


/*
 *  Size of a key in a key length table block in an internal node or bucket
 */
#define BT_KEYLENENTRY sizeof(ajuint)

/*
 *  Size of an offset key
 */
#define BT_OFFKEYLEN sizeof(ajulong)

/*
 *  Length of integers per entry in a num bucket 
 *  i.e. dbno + offset + refoffset
 */
#define BT_DOFFROFF (sizeof(ajuint) + sizeof(ajulong) + sizeof(ajulong))




/* @data AjPBtNode ***********************************************************
**
** Btree node
**
** @attr BlockOffset [ajulong] Offset within main index
** @attr Nkeys [ajuint] Number of keys filled
** @attr TotLen [ajuint] Total length of keys
** @attr Left [ajulong] Left Sibling
** @attr Right [ajulong] Right Sibling
** @attr Overflow [ajulong] Offset to overflow block
** @attr PrevNode [ajulong] Previous node
** @attr NodeType [ajuint] Root, Internal or Leaf
** @attr Padding [char[4]] Padding to alignment boundary
*****************************************************************************/

typedef struct AjSBtNode
{
    ajulong BlockOffset;
    ajuint  Nkeys;
    ajuint  TotLen;
    ajulong Left;
    ajulong Right;
    ajulong Overflow;
    ajulong PrevNode;
    ajuint  NodeType;
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
** @attr parray [ajulong*] pointer arrays (primary and secondary trees)
** @attr overflows [ajulong*] overflows (primary) and keys (secondary)
** @attr used [AjBool] node in use
** @attr Padding [char[4]] Padding to alignment boundary
*****************************************************************************/

typedef struct AjSBtMem
{
    struct AjSBtMem *next;
    struct AjSBtMem *prev;
    AjPStr *karray;
    ajulong *parray;
    ajulong *overflows;
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
** @attr offset [ajulong] Offset within database file (ftello)
** @attr refoffset [ajulong] Offset within reference database file (ftello)
******************************************************************************/

typedef struct AjSBtId
{
    AjPStr id;
    ajuint  dbno;
    ajuint  dups;
    ajulong offset;
    ajulong refoffset;
} AjOBtId;
#define AjPBtId AjOBtId*




/* @data AjPBtWild ***************************************************
**
** Btree wildcard
**
** @attr id [AjPStr] Wildcard ID
** @attr pagepos [ajulong] Page number of leaf
** @attr list [AjPList] list of AjPBtIds
** @attr first [AjBool] true for first search
** @attr Padding [char[4]] Padding to alignment boundary
******************************************************************************/

typedef struct AjSBtWild
{
    AjPStr id;
    ajulong pagepos;
    AjPList list;
    AjBool first;
    char Padding[4];
} AjOBtWild;
#define AjPBtWild AjOBtWild*




/* @data AjPBucket ***************************************************
**
** Bucket structure on disc
**
** Key, file number, ftell ID, subkey page (char*, ajuint, ajulong, ajulong)
**
** @attr NodeType [ajuint] Node type
** @attr Maxentries [ajuint] Number of entries available
** @attr Nentries [ajuint] Number of entries
** @attr Overflow [ajulong] Offset to overflow block
** @attr keylen [ajuint*] key lengths
** @attr Ids [AjPBtId*] Ids
******************************************************************************/

typedef struct AjSBucket
{
    ajuint   NodeType;
    ajuint   Maxentries;
    ajuint   Nentries;
    ajulong  Overflow;
    ajuint  *keylen;
    AjPBtId *Ids;
} AjOBucket;
#define AjPBucket AjOBucket*




/* @data AjPBtNumId ***************************************************
**
** Btree ID
**
** @attr offset [ajulong] Offset within database file (ftello)
** @attr refoffset [ajulong] Offset within reference database file (ftello)
** @attr dbno [ajuint] Database file number
** @attr Padding [char[4]] Padding to alignment boundary
******************************************************************************/

typedef struct AjSBtNumId
{
    ajulong offset;
    ajulong refoffset;
    ajuint  dbno;
    char Padding[4];
} AjOBtNumId;
#define AjPBtNumId AjOBtNumId*




/* @data AjPNumBucket ***************************************************
**
** Offset bucket structure on disc
**
** Key, file number, ftell ID, subkey page (char*, ajuint, ajulong, ajulong)
**
** @attr NodeType [ajuint] Node type
** @attr Maxentries [ajuint] Number of entries available
** @attr Nentries [ajuint] Number of entries
** @attr Overflow [ajulong] Offset to overflow block
** @attr NumId [AjPBtNumId*] secondary tree IDs
******************************************************************************/

typedef struct AjSNumBucket
{
    ajuint   NodeType;
    ajuint   Maxentries;
    ajuint   Nentries;
    ajulong  Overflow;
    AjPBtNumId *NumId;
} AjONumBucket;
#define AjPNumBucket AjONumBucket*




/* Database file name structure
**
** ajuint        order			Order of B+tree
** ajuint        m			Max entries per bucket
** ajuint	NFiles			Number of Indexed files
** ajuint        TotalLen                Total length if dir/name entries
** Directory/FileName pairs
*/

#if !defined(WORDS_BIGENDIAN)
#define BT_GETAJINT(p,v) (memcpy((void*)v,(const void*)p,sizeof(ajint)))
#define BT_GETAJUINT(p,v) (memcpy((void*)v,(const void*)p,sizeof(ajuint)))
#define BT_GETAJLONG(p,v) (memcpy((void*)v,(const void*)p,sizeof(ajlong)))
#define BT_GETAJULONG(p,v) (memcpy((void*)v,(const void*)p,sizeof(ajulong)))
#define BT_SETAJINT(p,v) (memcpy((void*)p,(const void*)&v,sizeof(ajint)))
#define BT_SETAJUINT(p,v) (memcpy((void*)p,(const void*)&v,sizeof(ajuint)))
#define BT_SETAJLONG(p,v) (memcpy((void*)p,(const void*)&v,sizeof(ajlong)))
#define BT_SETAJULONG(p,v) (memcpy((void*)p,(const void*)&v,sizeof(ajulong)))
#else
#define BT_GETAJINT(p,v) memcpy((void*)v,(const void*)p,sizeof(ajint)); \
                         ajByteRevInt(v)
#define BT_GETAJUINT(p,v) memcpy((void*)v,(const void*)p,sizeof(ajuint)); \
                         ajByteRevUint(v)
#define BT_GETAJLONG(p,v) memcpy((void*)v,(const void*)p,sizeof(ajlong)); \
                          ajByteRevLong(v)
#define BT_GETAJULONG(p,v) memcpy((void*)v,(const void*)p,sizeof(ajulong)); \
                          ajByteRevUlong(v)
#define BT_SETAJINT(p,v)  ajByteRevInt(&v); \
                          memcpy((void*)p,(const void*)&v,sizeof(ajint))
#define BT_SETAJUINT(p,v) ajByteRevUint(&v); \
                          memcpy((void*)p,(const void*)&v,sizeof(ajuint))
#define BT_SETAJLONG(p,v) ajByteRevLong(&v); \
                          memcpy((void*)p,(const void*)&v,sizeof(ajlong))
#define BT_SETAJULONG(p,v) ajByteRevUlong(&v); \
                          memcpy((void*)p,(const void*)&v,sizeof(ajulong))
#endif


#define BT_BUCKIDLEN(str) (ajStrGetLen(str) + 1 + sizeof(ajuint) + \
			   sizeof(ajuint) + sizeof(ajulong) + \
			   sizeof(ajulong))


/*
** Macros to determine entry positions within a bucket
*/

#define PBT_BUCKNODETYPE(p) p
#define PBT_BUCKNENTRIES(p) (p + sizeof(ajuint))
#define PBT_BUCKOVERFLOW(p) (p + sizeof(ajuint) + sizeof(ajuint))
#define PBT_BUCKKEYLEN(p) (p + sizeof(ajuint) + sizeof(ajuint) + sizeof(ajulong))
#define BT_BUCKPRILEN(str) (ajStrGetLen(str) + 1 + sizeof(ajulong))
#define BT_BUCKSECLEN(str) (ajStrGetLen(str) +1)

/*
** Macros to return a page entry value within a bucket
*/

#if !defined(WORDS_BIGENDIAN)
#define GBT_BUCKNODETYPE(p,v) (memcpy((void*)v, \
                                      (const void*)PBT_BUCKNODETYPE(p), \
				      sizeof(ajuint)))
#define GBT_BUCKNENTRIES(p,v) (memcpy((void*)v, \
                                      (const void*)PBT_BUCKNENTRIES(p), \
				      sizeof(ajuint)))
#define GBT_BUCKOVERFLOW(p,v) (memcpy((void*)v, \
                                      (const void*)PBT_BUCKOVERFLOW(p), \
				      sizeof(ajulong)))
#else
#define GBT_BUCKNODETYPE(p,v) memcpy((void*)v, \
                                     (const void*)PBT_BUCKNODETYPE(p),  \
				      sizeof(ajuint)); \
                              ajByteRevUint(v)
#define GBT_BUCKNENTRIES(p,v) memcpy((void*)v, \
                                     (const void*)PBT_BUCKNENTRIES(p),  \
                                     sizeof(ajuint));                    \
                              ajByteRevUint(v)
#define GBT_BUCKOVERFLOW(p,v) memcpy((void*)v, \
                                     (const void*)PBT_BUCKOVERFLOW(p),  \
				      sizeof(ajulong)); \
                              ajByteRevUlong(v)
#endif


/*
** Macros to set a page entry value within an internal/leaf node
*/

#if !defined(WORDS_BIGENDIAN)
#define SBT_BUCKNODETYPE(p,v) (memcpy((void*)PBT_BUCKNODETYPE(p), \
				      (const void*)&v,sizeof(ajuint)))
#define SBT_BUCKNENTRIES(p,v) (memcpy((void*)PBT_BUCKNENTRIES(p), \
				      (const void*)&v,sizeof(ajuint)))
#define SBT_BUCKOVERFLOW(p,v) (memcpy((void*)PBT_BUCKOVERFLOW(p), \
				      (const void*)&v,sizeof(ajulong)))
#else
#define SBT_BUCKNODETYPE(p,v) ajByteRevUint(&v); \
                              memcpy((void*)PBT_BUCKNODETYPE(p), \
				      (const void*)&v,sizeof(ajuint))
#define SBT_BUCKNENTRIES(p,v) ajByteRevUint(&v); \
                              memcpy((void*)PBT_BUCKNENTRIES(p), \
				     (const void*)&v,sizeof(ajuint))
#define SBT_BUCKOVERFLOW(p,v) ajByteRevUlong(&v); \
                              memcpy((void*)PBT_BUCKOVERFLOW(p), \
				     (const void*)&v,sizeof(ajulong))
#endif

/*
** Macros to determine entry positions within an internal/leaf node
*/

#define PBT_NODETYPE(p) p
#define PBT_BLOCKNUMBER(p) (p + sizeof(ajuint))
#define PBT_NKEYS(p) (p + sizeof(ajuint) + sizeof(ajulong))
#define PBT_TOTLEN(p) (p+sizeof(ajuint)+sizeof(ajulong)+sizeof(ajuint))
#define PBT_LEFT(p) (p+sizeof(ajuint)+sizeof(ajulong)+sizeof(ajuint) \
		     +sizeof(ajuint))
#define PBT_RIGHT(p) (p+sizeof(ajuint)+sizeof(ajulong)+sizeof(ajuint) \
		      +sizeof(ajuint)+sizeof(ajulong))
#define PBT_OVERFLOW(p) (p+sizeof(ajuint)+sizeof(ajulong)+sizeof(ajuint) \
		         +sizeof(ajuint)+sizeof(ajulong)+sizeof(ajulong))
#define PBT_PREV(p) (p+sizeof(ajuint)+sizeof(ajulong)+sizeof(ajuint) \
		     +sizeof(ajuint)+sizeof(ajulong)+sizeof(ajulong) \
		     +sizeof(ajulong))
#define PBT_KEYLEN(p) (p+sizeof(ajuint)+sizeof(ajulong)+sizeof(ajuint) \
		       +sizeof(ajuint)+sizeof(ajulong)+sizeof(ajulong) \
		       +sizeof(ajulong)+sizeof(ajulong))

/*
** Macros to return a page entry value within an internal/leaf node
*/

#if !defined(WORDS_BIGENDIAN)
#define GBT_NODETYPE(p,v) (memcpy((void*)v,(const void*)PBT_NODETYPE(p), \
				  sizeof(ajuint)))
#define GBT_BLOCKNUMBER(p,v) (memcpy((void*)v,(const void*)PBT_BLOCKNUMBER(p), \
				     sizeof(ajulong)))
#define GBT_NKEYS(p,v) (memcpy((void*)v,(const void*)PBT_NKEYS(p), \
			       sizeof(ajuint)))
#define GBT_TOTLEN(p,v) (memcpy((void*)v,(const void*)PBT_TOTLEN(p), \
			       sizeof(ajuint)))
#define GBT_LEFT(p,v) (memcpy((void*)v,(const void*)PBT_LEFT(p), \
			      sizeof(ajulong)))
#define GBT_RIGHT(p,v) (memcpy((void*)v,(const void*)PBT_RIGHT(p), \
			       sizeof(ajulong)))
#define GBT_PREV(p,v) (memcpy((void*)v,(const void*)PBT_PREV(p), \
			      sizeof(ajulong)))
#define GBT_OVERFLOW(p,v) (memcpy((void*)v,(const void*)PBT_OVERFLOW(p), \
				  sizeof(ajulong)))
#else
#define GBT_NODETYPE(p,v) memcpy((void*)v,(const void*)PBT_NODETYPE(p), \
				 sizeof(ajuint)); \
                          ajByteRevUint(v)
#define GBT_BLOCKNUMBER(p,v) memcpy((void*)v,(const void*)PBT_BLOCKNUMBER(p), \
				     sizeof(ajulong)); \
                             ajByteRevUlong(v)
#define GBT_NKEYS(p,v) memcpy((void*)v,(const void*)PBT_NKEYS(p), \
			       sizeof(ajuint)); \
                       ajByteRevUint(v)
#define GBT_TOTLEN(p,v) memcpy((void*)v,(const void*)PBT_TOTLEN(p), \
			       sizeof(ajuint)); \
                        ajByteRevUint(v)
#define GBT_LEFT(p,v) memcpy((void*)v,(const void*)PBT_LEFT(p), \
                             sizeof(ajulong)); \
                      ajByteRevUlong(v)

#define GBT_RIGHT(p,v) memcpy((void*)v,(const void*)PBT_RIGHT(p), \
			      sizeof(ajulong)); \
                       ajByteRevUlong(v)
#define GBT_PREV(p,v) memcpy((void*)v,(const void*)PBT_PREV(p), \
			      sizeof(ajulong)); \
                      ajByteRevUlong(v)
#define GBT_OVERFLOW(p,v) memcpy((void*)v,(const void*)PBT_OVERFLOW(p), \
				  sizeof(ajulong)); \
                          ajByteRevUlong(v)
#endif


/*
** Macros to set a page entry value within an internal/leaf node
*/

#if !defined(WORDS_BIGENDIAN)
#define SBT_NODETYPE(p,v) (memcpy((void*)PBT_NODETYPE(p),(const void*)&v, \
				  sizeof(ajuint)))
#define SBT_BLOCKNUMBER(p,v) (memcpy((void*)PBT_BLOCKNUMBER(p), \
				     (const void*)&v,sizeof(ajulong)))
#define SBT_NKEYS(p,v) (memcpy((void*)PBT_NKEYS(p),(const void*)&v, \
			       sizeof(ajuint)))
#define SBT_TOTLEN(p,v) (memcpy((void*)PBT_TOTLEN(p),(const void*)&v, \
				sizeof(ajuint)))
#define SBT_LEFT(p,v) (memcpy((void*)PBT_LEFT(p), \
			      (const void*)&v,sizeof(ajulong)))
#define SBT_RIGHT(p,v) (memcpy((void*)PBT_RIGHT(p), \
			       (const void*)&v,sizeof(ajulong)))
#define SBT_PREV(p,v) (memcpy((void*)PBT_PREV(p), \
			      (const void*)&v,sizeof(ajulong)))
#define SBT_OVERFLOW(p,v) (memcpy((void*)PBT_OVERFLOW(p), \
				  (const void*)&v,sizeof(ajulong)))
#else
#define SBT_NODETYPE(p,v) ajByteRevUint(&v); \
                          memcpy((void*)PBT_NODETYPE(p),(const void*)&v, \
				  sizeof(ajuint))
#define SBT_BLOCKNUMBER(p,v) ajByteRevUlong(&v); \
                             memcpy((void*)PBT_BLOCKNUMBER(p), \
				     (const void*)&v,sizeof(ajulong))
#define SBT_NKEYS(p,v) ajByteRevUint(&v); \
                       memcpy((void*)PBT_NKEYS(p),(const void*)&v, \
			       sizeof(ajuint))
#define SBT_TOTLEN(p,v) ajByteRevUint(&v); \
                        memcpy((void*)PBT_TOTLEN(p),(const void*)&v, \
				sizeof(ajuint))
#define SBT_LEFT(p,v) ajByteRevUlong(&v); \
                      memcpy((void*)PBT_LEFT(p), \
			      (const void*)&v,sizeof(ajulong))
#define SBT_RIGHT(p,v) ajByteRevUlong(&v); \
                       memcpy((void*)PBT_RIGHT(p), \
			       (const void*)&v,sizeof(ajulong))
#define SBT_PREV(p,v) ajByteRevUlong(&v); \
                      memcpy((void*)PBT_PREV(p), \
			      (const void*)&v,sizeof(ajulong))
#define SBT_OVERFLOW(p,v) ajByteRevUlong(&v); \
                          memcpy((void*)PBT_OVERFLOW(p), \
				 (const void*)&v,sizeof(ajulong))
#endif




/* @data AjPBtpage ***************************************************
**
** Btree page
**
** @attr pagepos [ajulong] Page number
** @attr next [struct AjSBtpage*] Next page
** @attr prev [struct AjSBtpage*] Previous page
** @attr buf [unsigned char*] Buffer
** @attr dirty [ajuint] BT_DIRTY if page needs to be written to disc
** @attr lockfor [ajuint] Reason for last setting of dirty as BT_LOCK
******************************************************************************/

typedef struct AjSBtpage
{
    ajulong pagepos;
    struct AjSBtpage *next;
    struct AjSBtpage *prev;
    unsigned char *buf;
    ajuint  dirty;
    ajuint  lockfor;
} AjOBtpage;
#define AjPBtpage AjOBtpage*




/* @data AjPBtcache ***************************************************
**
** B+ tree cache
**
** @attr fp [FILE*] Tree index file pointer
** @attr filename [AjPStr] Filename
** @attr lru [AjPBtpage] Least recently used cache page
** @attr mru [AjPBtpage] Most recently used cache page
** @attr bmem [AjPBtMem] Primary array allocation MRU bottom
** @attr tmem [AjPBtMem] Primary array allocation MRU top
** @attr bsmem [AjPBtMem] Secondary array allocation MRU bottom
** @attr tsmem [AjPBtMem] Secondary array allocation MRU top
** @attr replace [AjPStr] Replacement ID
** @attr pagetable [AjPTable] Table of cached pages
** @attr totsize [ajulong] Tree index total length
** @attr filesize [ajulong] Tree index length after any compression
** @attr extendsize [ajulong] Tree index extension block length
** @attr pagecount [ajulong] Tree index number of pages
** @attr secrootblock [ajulong] Secondary tree root block
** @attr numreplace [ajulong] Replacement numeric ID
** @attr countunique [ajulong] Number of unique tokens indexed
** @attr countall [ajulong] Number of total tokens indexed
** @attr cachehits [ajulong] Number of cached page reads
** @attr reads [ajulong] Number of physical reads from disk
** @attr writes [ajulong] Number of physical writes to disk
** @attr pagesize [ajuint] Size of cache pages
** @attr listLength [ajuint] Number of pages in cache
** @attr order [ajuint] Order of primary tree
** @attr level [ajuint] Depth of primary tree
** @attr cachesize [ajuint] Maximum number of pages to cache
** @attr nperbucket [ajuint] Number of entries in a primary bucket
** @attr slevel [ajuint] Depth of secondary tree
** @attr sorder [ajuint] Order of secondary tree
** @attr snperbucket [ajuint] Number of entries in a secondary bucket
** @attr kwlimit [ajuint] Max length of secondary key
** @attr secondary [AjBool] Secondary index
** @attr readonly [AjBool] Read only flag
** @attr deleted [AjBool] Deletion flag
** @attr compressed [AjBool] Index is compressed
******************************************************************************/

typedef struct AjSBtCache
{
    FILE *fp;
    AjPStr filename;
    AjPBtpage lru;
    AjPBtpage mru;
    AjPBtMem bmem;
    AjPBtMem tmem;
    AjPBtMem bsmem;
    AjPBtMem tsmem;
    AjPStr replace;
    AjPTable pagetable;
    ajulong totsize;
    ajulong filesize;
    ajulong extendsize;
    ajulong pagecount;
    ajulong secrootblock;
    ajulong numreplace;
    ajulong countunique;
    ajulong countall;
    ajulong cachehits;
    ajulong reads;
    ajulong writes;
    ajuint pagesize;
    ajuint listLength;
    ajuint order;
    ajuint level;
    ajuint cachesize;
    ajuint nperbucket;
    ajuint slevel;
    ajuint sorder;
    ajuint snperbucket;
    ajuint kwlimit;
    AjBool secondary;
    AjBool readonly;
    AjBool deleted;
    AjBool compressed;
} AjOBtcache;
#define AjPBtcache AjOBtcache*




/* @data AjPBtPri ***************************************************
**
** Btree primary keyword
**
** @attr keyword [AjPStr] keyword
** @attr treeblock [ajulong] disc block of secondary tree
** @attr id [AjPStr] Id string
******************************************************************************/

typedef struct AjSBtPri
{
    AjPStr keyword;
    ajulong treeblock;
    AjPStr id;
} AjOBtPri;
#define AjPBtPri AjOBtPri*




/* @data AjPPriBucket ***************************************************
**
** Keyword primary bucket structure on disc
**
** @attr NodeType [ajuint] Node type
** @attr Maxentries [ajuint] Number of entries available
** @attr Nentries [ajuint] Number of entries
** @attr Overflow [ajulong] Offset to overflow block
** @attr keylen [ajuint*] key lengths
** @attr codes [AjPBtPri*] Primary keywords
******************************************************************************/

typedef struct AjSPriBucket
{
    ajuint    NodeType;
    ajuint   Maxentries;
    ajuint   Nentries;
    ajulong   Overflow;
    ajuint   *keylen;
    AjPBtPri *codes;
} AjOPriBucket;
#define AjPPriBucket AjOPriBucket*




/* @data AjPSecBucket ***************************************************
**
** Keyword secondary bucket structure on disc
**
** @attr NodeType [ajuint] Node type
** @attr Maxentries [ajuint] Number of entries available
** @attr Nentries [ajuint] Number of entries
** @attr Overflow [ajulong] Offset to overflow block
** @attr keylen [ajuint*] key lengths
** @attr SecIds [AjPStr*] Ids
******************************************************************************/

typedef struct AjSSecBucket
{
    ajuint    NodeType;
    ajuint   Maxentries;
    ajuint   Nentries;
    ajulong   Overflow;
    ajuint   *keylen;
    AjPStr   *SecIds;
} AjOSecBucket;
#define AjPSecBucket AjOSecBucket*




/* @data AjPBtKeyWild ***************************************************
**
** Btree keyword wildcard object
**
** @attr keyword [AjPStr] Wildcard keyword
** @attr pagepos [ajulong] Page number of primary tree leaf
** @attr list [AjPList] list of AjPBtPris
** @attr cache [AjPBtcache] cache for secondary tree
** @attr idlist [AjPList] list of AjPStr IDs
** @attr secpagepos [ajulong] Page number of secondary tree leaf
** @attr first [AjBool] true for first search
** @attr Padding [char[4]] Padding to alignment boundary
******************************************************************************/

typedef struct AjSBtKeyWild
{
    AjPStr keyword;
    ajulong pagepos;
    AjPList list;
    AjPBtcache cache;
    AjPList idlist;
    ajulong secpagepos;
    AjBool first;
    char Padding[4];
} AjOBtKeyWild;
#define AjPBtKeyWild AjOBtKeyWild*




/* @data AjPBtHybrid ***************************************************
**
** Btree ID
**
** @attr key1 [AjPStr] Unique ID
** @attr dbno [ajuint] Database file number
** @attr dups [ajuint] Duplicates
** @attr offset [ajulong] Offset within database file (ftello)
** @attr refoffset [ajulong] Offset within reference database file (ftello)
** @attr treeblock [ajulong] Secondary tree root page
******************************************************************************/

typedef struct AjSBtHybrid
{
    AjPStr key1;
    ajuint  dbno;
    ajuint  dups;
    ajulong offset;
    ajulong refoffset;
    ajulong treeblock;
} AjOBtHybrid;
#define AjPBtHybrid AjOBtHybrid*




/*
** Prototype definitions
*/

ajuint       ajBtreeFieldGetLenC(const char* nametxt);
ajuint       ajBtreeFieldGetLenS(const AjPStr name);
const AjPStr ajBtreeFieldGetExtensionC(const char *nametxt);
const AjPStr ajBtreeFieldGetExtensionS(const AjPStr name);
AjBool       ajBtreeFieldGetSecondaryC(const char *nametxt);
AjBool       ajBtreeFieldGetSecondaryS(const AjPStr name);
AjBool       ajBtreeCacheIsSecondary(const AjPBtcache thys);
AjPBtcache   ajBtreeCacheNewC(const char *file, const char *ext,
                              const char *idir, const char *mode,
                              AjBool compressed, ajuint kwlimit,
                              ajuint pagesize, ajuint cachesize,
                              ajulong pagecount,
                              ajuint order, ajuint fill, ajuint level,
                              ajuint sorder, ajuint sfill,
                              ajulong count, ajulong countall);
AjPBtcache   ajBtreeCacheNewS(const AjPStr file, const AjPStr ext,
                              const AjPStr idir, const char *mode,
                              AjBool compressed, ajuint kwlimit,
                              ajuint pagesize, ajuint cachesize,
                              ajulong pagecount,
                              ajuint order, ajuint fill, ajuint level,
                              ajuint sorder, ajuint sfill,
                              ajulong count, ajulong countall);
AjPBtcache   ajBtreeCacheNewReadC(const char *filetxt, const char *exttxt,
                                  const char *idirtxt);
AjPBtcache   ajBtreeCacheNewReadS(const AjPStr file, const AjPStr ext,
                                  const AjPStr idir);
AjPBtcache   ajBtreeCacheNewUpdateC(const char *filetxt, const char *exttxt,
                                    const char *idirtxt);
AjPBtcache   ajBtreeCacheNewUpdateS(const AjPStr file, const AjPStr ext,
                                    const AjPStr idir);
AjPBtpage    ajBtreeCacheRead(AjPBtcache cache, ajulong pagepos);
AjPBtpage    ajBtreeCacheWrite(AjPBtcache cache, ajulong pagepos);
AjPBtpage    ajBtreeCacheWriteBucket(AjPBtcache cache, ajulong pagepos);
AjPBtpage    ajBtreeCacheWriteNode(AjPBtcache cache, ajulong pagepos);
AjPBtpage    ajBtreeCacheWriteBucketnew(AjPBtcache cache);
AjPBtpage    ajBtreeCacheWriteNodenew(AjPBtcache cache);
AjPBtpage    ajBtreeCacheWriteOverflownew(AjPBtcache cache);
void         ajBtreeCreateRootNode(AjPBtcache cache, ajulong rootpage);
AjPBtpage    ajBtreeFindInsert(AjPBtcache cache, const AjPStr key);

ajulong      ajBtreeCacheDel(AjPBtcache *thys);
void         ajBtreeInsertId(AjPBtcache cache, const AjPBtId id);
void         ajBtreeIdDel(AjPBtId *thys);
void         ajBtreeIdDelVoid(void **voidarg);
AjPBtId      ajBtreeIdNew(void);
AjPBtId      ajBtreeIdFromKey(AjPBtcache cache, const AjPStr key);
void         ajBtreeWriteParamsC(const AjPBtcache cache, const char *fntxt,
                                 const char *exttxt, const char *idirtxt);
void         ajBtreeWriteParamsS(const AjPBtcache cache, const AjPStr fn,
                                 const AjPStr ext, const AjPStr idir);
AjBool       ajBtreeReadParamsC(const char *fn, const char *ext,
                                const char *idir,
                                AjBool *secondary, AjBool *compressed,
                                ajuint *kwlimit,
                                ajuint *pagesize, ajuint *cachesize,
                                ajulong *pagecount,
                                ajuint *order, ajuint *nperbucket,
                                ajuint *level,
                                ajuint *sorder, ajuint *snperbucket,
                                ajulong *count, ajulong *countall);
AjBool       ajBtreeReadParamsS(const AjPStr fn, const AjPStr ext,
                                const AjPStr idir,
                                AjBool *secondary, AjBool *compressed,
                                ajuint *kwlimit,
                                ajuint *pagesize, ajuint *cachesize,
                                ajulong *pagecount,
                                ajuint *order, ajuint *nperbucket,
                                ajuint *level,
                                ajuint *sorder, ajuint *snperbucket,
                                ajulong *count, ajulong *countall);
void         ajBtreeCacheSync(AjPBtcache cache, ajulong rootpage);
void         ajBtreeCacheRootSync(AjPBtcache cache, ajulong rootpage);

AjPBtWild    ajBtreeWildNew(AjPBtcache cache, const AjPStr wild);
AjPBtKeyWild ajBtreeKeyWildNew(AjPBtcache cache, const AjPStr wild);
void         ajBtreeWildDel(AjPBtWild *thys);
void         ajBtreeKeyWildDel(AjPBtKeyWild *thys);

AjPBtpage    ajBtreeFindInsertW(AjPBtcache cache, const AjPStr key);
AjPBtId      ajBtreeIdFromKeyW(AjPBtcache cache, AjPBtWild wild);
void         ajBtreeListFromKeyW(AjPBtcache cache, const AjPStr key,
                                 AjPList idlist);
AjPBtId      ajBtreeIdFromKeywordW(AjPBtcache cache, AjPBtKeyWild wild,
                                   AjPBtcache idcache);
void         ajBtreeListFromKeywordW(AjPBtcache cache, const AjPStr key,
                                     AjPBtcache idcache, AjPList btidlist);

AjBool       ajBtreeReplaceId(AjPBtcache cache, const AjPBtId rid);

ajuint       ajBtreeReadEntries(const char *filename, const char *indexdir,
                                const char *directory,
                                AjPStr **seqfiles, AjPStr **reffiles);
ajuint       ajBtreeReadEntriesS(const AjPStr filename, const AjPStr indexdir,
                                 const AjPStr directory,
                                 AjPStr **seqfiles, AjPStr **reffiles);
void         ajBtreeInsertDupId(AjPBtcache cache, AjPBtId id);
AjPList      ajBtreeDupFromKey(AjPBtcache cache, const AjPStr key);




AjPBtPri     ajBtreePriNew(void);
void         ajBtreePriDel(AjPBtPri *thys);
AjBool       ajBtreePriFindKeyword(AjPBtcache cache, const AjPStr key,
                                   ajulong *treeblock);
AjBool       ajBtreePriFindKeywordLen(AjPBtcache cache, const AjPStr key,
                                      ajulong *treeblock);

AjPBtcache   ajBtreeSecCacheNewC(const char *file, const char *ext,
                                 const char *idir, const char *mode,
                                 AjBool compressed, ajuint kwlimit,
                                 ajuint pagesize, ajuint cachesize,
                                 ajulong pagecount,
                                 ajuint order, ajuint fill, ajuint level,
                                 ajuint sorder, ajuint sfill,
                                 ajulong count, ajulong countall);
AjPBtcache   ajBtreeSecCacheNewS(const AjPStr file, const AjPStr ext,
                                 const AjPStr idir, const char *mode,
                                 AjBool compressed, ajuint kwlimit,
                                 ajuint pagesize, ajuint cachesize,
                                 ajulong pagecount,
                                 ajuint order, ajuint fill, ajuint level,
                                 ajuint sorder, ajuint sfill,
                                 ajulong count, ajulong countall);
AjPBtpage    ajBtreeSecFindInsert(AjPBtcache cache, const AjPStr key);
AjBool       ajBtreeSecInsertId(AjPBtcache cache, const AjPStr id);
AjBool       ajBtreeSecFindId(AjPBtcache cache, const AjPStr key);

AjPList      ajBtreeSecLeafList(AjPBtcache cache, ajulong rootblock);
AjBool       ajBtreeVerifyId(AjPBtcache cache, ajulong rootblock,
                             const AjPStr id);

void         ajBtreeDumpKeywords(AjPBtcache cache,
                               ajuint dmin, ajuint dmax, AjPFile outf);
AjBool       ajBtreeInsertKeyword(AjPBtcache cache, AjPBtPri pri);

void         ajBtreeLockTest(AjPBtcache cache);



AjPBtpage    ajBtreeHybFindInsert(AjPBtcache cache, const AjPStr key);
AjPBtpage    ajBtreeNumFindInsert(AjPBtcache cache, const ajulong key);

void         ajBtreeInsertNum(AjPBtcache cache, const AjPBtNumId num,
                              AjPBtpage page);
void         ajBtreeHybInsertId(AjPBtcache cache, AjPBtHybrid hyb);
AjPBtHybrid  ajBtreeHybNew(void);
void         ajBtreeHybDel(AjPBtHybrid *thys);
void         ajBtreeFreePriArray(AjPBtcache cache);
void         ajBtreeFreeSecArray(AjPBtcache cache);
void 	     ajBtreeHybLeafList(AjPBtcache cache, ajulong rootblock,
                                const AjPStr idname, AjPList list);

void         ajBtreeDumpHybKeys(AjPBtcache cache, ajuint dmin, ajuint dmax,
                                AjPFile outf);

AjBool       ajBtreeDeleteHybId(AjPBtcache cache, const AjPBtHybrid hyb);
AjBool       ajBtreeDeletePriId(AjPBtcache cache, const AjPBtPri pri);
    
void         ajBtreeExit(void);

ajulong      ajBtreeGetPagecount(const AjPBtcache cache);
ajuint       ajBtreeGetPagesize(const AjPBtcache cache);
ajulong      ajBtreeGetTotsize(const AjPBtcache cache);
ajuint       ajBtreePageGetSize(const AjPBtpage page);
const char*  ajBtreePageGetTypename(const AjPBtpage page);
AjBool       ajBtreeStatBucket(AjPBtcache cache,
                               const AjPBtpage page, AjBool full,
                               ajuint* nkeys, ajuint* ndups, ajuint* nextra,
                               ajuint* overflows, ajuint* freespace,
                               ajulong *refs, const AjPTable newpostable);
AjBool       ajBtreeStatNumbucket(AjPBtcache cache,
                                  const AjPBtpage page, AjBool full,
                                  ajuint* nkeys, ajuint* overflows,
                                  ajuint* freespace);
AjBool       ajBtreeStatPribucket(AjPBtcache cache,
                                  const AjPBtpage page, AjBool full,
                                  ajuint* nkeys, ajuint* overflows,
                                  ajuint* freespace, ajulong *refs,
                                  const AjPTable newpostable);
AjBool       ajBtreeStatSecbucket(AjPBtcache cache,
                                  const AjPBtpage page, AjBool full,
                                  ajuint* nkeys, ajuint* overflows,
                                  ajuint* freespace);
AjBool       ajBtreeStatNode(AjPBtcache cache,
                             const AjPBtpage page, AjBool full,
                             ajuint* nkeys, ajuint* overflows,
                             ajuint* freespace, ajulong* refs,
                             const AjPTable newpostable);
AjBool       ajBtreeStatNumnode(AjPBtcache cache,
                                const AjPBtpage page, AjBool full,
                                ajuint* nkeys, ajuint* overflows,
                                ajuint* freespace, ajulong* refs,
                                const AjPTable newpostable);

void         ajBtreeCacheStatsOut(AjPFile outf, const AjPBtcache cache,
                                  ajulong *Pcache, ajulong* Preads,
                                  ajulong* Pwrites, ajulong *Psize);

ajint        ajBtreeIdCmp(const void *x, const void *y);
ajuint       ajBtreeIdHash(const void *x, ajuint hashsize);


/*
** End of prototype definitions
*/
__deprecated void     ajBtreeWriteParams(const AjPBtcache cache, const char *fn,
                                         const char *ext, const char *idir);

#endif

#ifdef __cplusplus
}
#endif

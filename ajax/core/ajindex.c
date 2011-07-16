/* @source ajindex.c
**
** B+ Tree Indexing plus Disc Cache.
** Copyright (c) 2003 Alan Bleasby
**     subsequently heavily modified by Peter Rice
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

#include "ajax.h"
#include <errno.h>

#ifdef WIN32
#define fileno _fileno
#define ftruncate _chsize_s
#endif

#if defined (AJ_IRIXLF)
#define usestat64 1
#endif

#define AJINDEX_DEBUG 1

static AjBool btreeDoRootSync = AJFALSE;

static ajulong   statCallSync = 0L;
static ajulong   statCallRootSync = 0L;
static ajulong   statCallSplitRoot = 0L;
static ajulong   statCallSplitPriRoot = 0L;
static ajulong   statCallSplitSecRoot = 0L;
static ajulong   statCallSplitHybRoot = 0L;
static ajulong   statCallSplitNumRoot = 0L;
static ajulong   statCallSplitLeaf = 0L;
static ajulong   statCallSplitPriLeaf = 0L;
static ajulong   statCallSplitSecLeaf = 0L;
static ajulong   statCallSplitHybLeaf = 0L;
static ajulong   statCallSplitNumLeaf = 0L;
static ajulong   statCallReorderBuckets = 0L;
static ajulong   statCallReorderPriBuckets = 0L;
static ajulong   statCallReorderSecBuckets = 0L;
static ajulong   statCallReorderHybBuckets = 0L;
static ajulong   statCallReorderNumBuckets = 0L;
static ajulong   statSyncLocked = 0L;
static ajulong   statSyncWrite = 0L;
static ajulong   statRootSyncLocked = 0L;
static ajulong   statRootSyncUnlocked = 0L;
static ajuint    statRootSyncMaxUnlocked = 0;
static ajuint    statRootSyncMaxLocked = 0;

static AjPStr*   statSaveSecId = NULL;
static ajuint    statSaveSecIdNext = 0;
static ajuint    statSaveSecIdMax = 0;

static AjPBtId*  statSaveBtreeId = NULL;
static ajuint    statSaveBtreeIdNext = 0;
static ajuint    statSaveBtreeIdMax = 0;

static AjPBtPri* statSaveBtreePri = NULL;
static ajuint    statSaveBtreePriNext = 0;
static ajuint    statSaveBtreePriMax = 0;

/* bucket cache */
static AjPBucket* statSaveBucket = NULL;
static AjPBucket* statSaveBucketEmpty = NULL;
static ajuint     statSaveBucketNext = 0;
static ajuint     statSaveBucketMax = 0;
static ajuint     statSaveBucketEmptyNext = 0;
static ajuint     statSaveBucketEmptyMax = 0;

/* primary bucket cache */
static AjPPriBucket* statSavePriBucket = NULL;
static AjPPriBucket* statSavePriBucketEmpty = NULL;
static ajuint        statSavePriBucketNext = 0;
static ajuint        statSavePriBucketMax = 0;
static ajuint        statSavePriBucketEmptyNext = 0;
static ajuint        statSavePriBucketEmptyMax = 0;

/* secondary bucket cache */
static AjPSecBucket* statSaveSecBucket = NULL;
static AjPSecBucket* statSaveSecBucketEmpty = NULL;
static ajuint        statSaveSecBucketNext = 0;
static ajuint        statSaveSecBucketMax = 0;
static ajuint        statSaveSecBucketEmptyNext = 0;
static ajuint        statSaveSecBucketEmptyMax = 0;

/* number bucket cache */
static AjPNumBucket* statSaveNumBucket = NULL;
static ajuint        statSaveNumBucketNext = 0;
static ajuint        statSaveNumBucketMax = 0;

static ajulong statCountAllocPriArrayNew = 0;
static ajulong statCountAllocPriArrayReuse = 0;
static ajulong statCountAllocPriArrayDel = 0;
static ajulong statCountAllocPriArrayFree = 0;

static ajulong statCountAllocSecArrayNew = 0;
static ajulong statCountAllocSecArrayReuse = 0;
static ajulong statCountAllocSecArrayDel = 0;
static ajulong statCountAllocSecArrayFree = 0;

static char const **btreeNodetypeNames = NULL;

/* @datastatic BtreeSField ****************************************************
**
** Index field data
**
** @alias BtreeOField
** @alias BtreePField
**
** @attr Name [AjPStr] Name
** @attr Extension [AjPStr] Extension
** @attr Secondary [AjBool] True for primary/secondary ID index
**                          False for Identifier/fileposition index
** @attr Len [ajuint] Index field maximum length
**
******************************************************************************/

typedef struct BtreeSField
{
    AjPStr Name;
    AjPStr Extension;
    AjBool Secondary;
    ajuint Len;
} BtreeOField;
#define BtreePField BtreeOField*


/* @datastatic BtreeSFielddef *************************************************
**
** Known index fields
**
** @alias BtreeOFielddef
** @alias BtreePFielddef
**
** @attr Name [const char*] Name
** @attr Extension [const char*] Extension
** @attr Secondary [AjBool] True for primary/secondary ID index
**                          False for Identifier/fileposition index
** @attr Len [ajuint] Index field maximum length
**
******************************************************************************/

typedef struct BtreeSFielddef
{
    const char* Name;
    const char* Extension;
    AjBool Secondary;
    ajuint Len;
} BtreeOFielddef;
#define BtreePFielddef BtreeOFielddef*

BtreeOFielddef btreeFields[] = {
    {"acc",  "xac", AJFALSE, 15},
    {"id",   "xid", AJFALSE, 15},
    {"sv",   "xsv", AJFALSE, 15},
    {"up",   "xup", AJFALSE, 8},
    {"spc",  "xspc", AJFALSE, 15},
    {"nam",  "xnm", AJTRUE,  15},
    {"key",  "xkw", AJTRUE,  15},
    {"des",  "xde", AJTRUE,  15},
    {"org",  "xtx", AJTRUE,  15},
    {"isa",  "xis", AJTRUE,  15},
    {"xref", "xrf", AJTRUE,  15},
    {"ns",   "xns", AJTRUE,  15},
    {NULL,   NULL,  AJFALSE, 0}
    
};

static AjPTable btreeFieldsTable = NULL;
static AjPStr   btreeFieldnameTmp = NULL;

static AjBool btreeDoExtra = AJFALSE;

static AjPBtpage    btreeCacheLocate(AjPBtcache cache, ajulong page);
static AjPBtpage    btreeCacheLruUnlink(AjPBtcache cache);
static void         btreeCacheUnlink(AjPBtcache cache, AjPBtpage cpage);
static void         btreeCacheDestage(AjPBtcache cache, AjPBtpage cpage);
static void         btreeCacheWriteCompress(AjPBtcache cache, AjPBtpage cpage,
                                            ajulong pagepos, ajuint pagesize);
static void         btreeCacheWriteUncompress(AjPBtcache cache,
                                              AjPBtpage cpage,
                                              ajulong pagepos);
static ajulong      btreeCacheCompress(AjPBtcache thys);
static ajulong      btreeCacheUncompress(AjPBtcache thys);
static ajulong      btreePageposCompress(ajulong oldpos,
                                         ajulong const* newpagepos,
                                         ajuint pagesize, const char* desc);
static ajulong      btreePageposUncompress(ajulong oldpos,
                                           const AjPTable newpagetable,
                                           const char* desc);
static AjPBtpage    btreePageNew(AjPBtcache cache);
static AjBool       btreePageUncompress(AjPBtpage page,
                                        const AjPTable newpagetable,
                                        ajuint pagesize);
static void         btreePageUncompressBucket(AjPBtpage page,
                                              const AjPTable newpagetable);
static void         btreePageUncompressNode(AjPBtpage page,
                                            const AjPTable newpagetable,
                                            ajuint pagesize);
static void         btreePageUncompressNumbucket(AjPBtpage page,
                                                 const AjPTable newpagetable);
static void         btreePageUncompressNumnode(AjPBtpage page,
                                               const AjPTable newpagetable,
                                               ajuint pagesize);
static void         btreePageUncompressPribucket(AjPBtpage page,
                                                 const AjPTable newpagetable);
static void         btreePageUncompressSecbucket(AjPBtpage page,
                                                 const AjPTable newpagetable);
static AjBool       btreePageCompress(AjPBtpage page,
                                      ajulong const* newpagepos,
                                      ajuint pagesize);
static void         btreePageCompressBucket(AjPBtpage page,
                                            ajulong const * newpagepos,
                                            ajuint pagesize);
static void         btreePageCompressNode(AjPBtpage page,
                                          ajulong const* newpagepos,
                                          ajuint pagesize);
static void         btreePageCompressNumbucket(AjPBtpage page,
                                               ajulong const* newpagepos,
                                               ajuint pagesize);
static void         btreePageCompressNumnode(AjPBtpage page,
                                             ajulong const* newpagepos,
                                             ajuint pagesize);
static void         btreePageCompressPribucket(AjPBtpage page,
                                               ajulong const* newpagepo,
                                               ajuint pagesizes);
static void         btreePageCompressSecbucket(AjPBtpage page,
                                               ajulong const* newpagepos,
                                               ajuint pagesize);
static ajuint       btreePageGetSizeBucket(const AjPBtpage page);
static ajuint       btreePageGetSizeNode(const AjPBtpage page);
static ajuint       btreePageGetSizeNumbucket(const AjPBtpage page);
static ajuint       btreePageGetSizeNumnode(const AjPBtpage page);
static ajuint       btreePageGetSizePribucket(const AjPBtpage page);
static ajuint       btreePageGetSizeSecbucket(const AjPBtpage page);

static void         btreeCacheFetch(AjPBtcache cache, AjPBtpage cpage,
                                    ajulong pagepos);
static void         btreeCacheMruAdd(AjPBtcache cache, AjPBtpage cpage);
static AjPBtpage    btreeCacheControl(AjPBtcache cache, ajulong pagepos,
                                      AjBool isread);
static AjPBtpage    btreeFindINode(AjPBtcache cache, AjPBtpage page,
                                   const AjPStr item);
static AjPBtpage    btreeSecFindINode(AjPBtcache cache, AjPBtpage page,
                                      const AjPStr item);


static AjPBtpage    btreePageFromKey(AjPBtcache cache, unsigned char *buf,
                                     const AjPStr item);
static AjPBtpage    btreeSecPageFromKey(AjPBtcache cache, unsigned char *buf,
                                        const AjPStr item);
static ajuint       btreeNumInBucket(AjPBtcache cache, ajulong pagepos);
static AjPBtId      btreeBucketFindDupId(AjPBtcache cache, ajulong pagepos,
                                       const AjPStr id, ajuint* ientry);
static ajulong      btreeBucketIdlist(AjPBtcache cache, ajulong pagepos,
                                      AjPList idlist);
static AjPBucket    btreeReadBucket(AjPBtcache cache, ajulong pagepos);
static void         btreeWriteBucket(AjPBtcache cache, const AjPBucket bucket,
                                     ajulong pagepos);
static void         btreeWriteBucketEmpty(AjPBtcache cache, ajulong pagepos);
static void         btreeWriteBucketId(AjPBtcache cache, ajulong pagepos,
                                       const AjPBtId btid, ajuint ientry);
static void         btreeAddToBucket(AjPBtcache cache, ajulong pagepos,
                                     const AjPBtId id);
static void         btreeAddToBucketFull(AjPBtcache cache, ajulong pagepos,
                                         const AjPBtId id);
static void 	    btreeBucketDel(AjPBucket *thys);
static void         btreeBucketFree(AjPBucket *thys);
static AjBool       btreeReorderBuckets(AjPBtcache cache, AjPBtpage page);
static void         btreeGetKeys(AjPBtcache cache, unsigned char *buf,
                                 AjPStr **keys, ajulong **ptrs);
static ajuint       btreeGetPointers(AjPBtcache cache, unsigned char *buf,
                                     ajulong **ptrs);
static ajulong      btreeGetBlockC(AjPBtcache cache, unsigned char *buf,
                                   const char* ckey);
static ajulong      btreeGetBlockN(AjPBtcache cache, unsigned char *buf,
                                   ajulong numkey);
static ajulong      btreeGetBlockS(AjPBtcache cache, unsigned char *buf,
                                   const AjPStr key);
static ajulong      btreeGetBlockFirstC(AjPBtcache cache, unsigned char *buf,
                                        const char* ckey, ajuint clen);
static ajulong      btreeGetBlockFirstN(AjPBtcache cache, unsigned char *buf,
                                        ajulong numkey);
static ajulong      btreeGetBlockFirstS(AjPBtcache cache, unsigned char *buf,
                                        const AjPStr key);
static void         btreeIdFree(AjPBtId *thys);
static ajint        btreeIdCompare(const void *a, const void *b);
static AjPBucket    btreeBucketNew(ajuint n);
static void         btreeWriteNode(AjPBtcache cache, AjPBtpage page,
                                   AjPStr const *keys, const ajulong *ptrs,
                                   ajuint nkeys);
static void         btreeWriteNodeSingle(AjPBtcache cache, AjPBtpage spage,
                                         const AjPStr key, ajulong lptr,
                                         ajulong rptr);
static AjBool       btreeNodeIsFull(const AjPBtcache cache, AjPBtpage page);
static AjBool       btreeNodeIsFullSec(const AjPBtcache cache, AjPBtpage page);
static void         btreeInsertNonFull(AjPBtcache cache, AjPBtpage page,
                                       const AjPStr key, ajulong less,
                                       ajulong greater);
static void         btreeSplitRoot(AjPBtcache cache);
static void         btreeInsertKey(AjPBtcache cache, AjPBtpage page,
                                   const AjPStr key, ajulong less,
                                   ajulong greater);

static ajulong      btreeInsertShiftS(AjPBtcache cache, AjPBtpage *retpage,
                                      const AjPStr key);
static void         btreeKeyShift(AjPBtcache cache, AjPBtpage tpage);


#if 0
static AjPBtpage    btreeTraverseLeaves(AjPBtcache cache, AjPBtpage thys);
static void         btreeJoinLeaves(AjPBtcache cache);
#endif


static AjPBtpage    btreeFindINodeW(AjPBtcache cache, AjPBtpage page,
                                    const AjPStr item);
static AjPBtpage    btreePageFromKeyW(AjPBtcache cache, unsigned char *buf,
                                      const AjPStr key);
static void         btreeReadLeaf(AjPBtcache cache, AjPBtpage page,
                                  AjPList list);
static void         btreeReadPriLeaf(AjPBtcache cache, AjPBtpage page,
                                     AjPList list);
static AjPBtpage    btreeSplitLeaf(AjPBtcache cache, AjPBtpage spage);
static AjPBtpage    btreeSplitPriLeaf(AjPBtcache cache, AjPBtpage spage);




static void         btreePriFree(AjPBtPri *thys);
static AjPPriBucket btreePriBucketNew(ajuint n);
static void         btreePriBucketDel(AjPPriBucket *thys);
static void         btreePriBucketFree(AjPPriBucket *thys);
static ajulong      btreePriBucketIdlist(AjPBtcache cache, ajulong pagepos,
                                         AjPList idlist);
static AjBool       btreePriBucketFindId(AjPBtcache cache, ajulong pagepos,
                                         const AjPStr id, ajulong* treeblock);
static AjPPriBucket btreeReadPriBucket(AjPBtcache cache, ajulong pagepos);
static void         btreeWritePriBucket(AjPBtcache cache,
                                        const AjPPriBucket bucket,
                                        ajulong pagepos);
static void         btreeWritePriBucketEmpty(AjPBtcache cache, ajulong pagepos);
static void         btreeAddToPriBucket(AjPBtcache cache, ajulong pagepos,
                                        const AjPBtPri pri);
static void         btreeAddToPriBucketFull(AjPBtcache cache, ajulong pagepos,
                                            const AjPBtPri pri);
static ajuint       btreeNumInPriBucket(AjPBtcache cache, ajulong pagepos);
static ajint        btreeKeywordCompare(const void *a, const void *b);
static AjBool       btreeReorderPriBuckets(AjPBtcache cache,
                                           AjPBtpage leaf);




static AjBool        btreeSecBucketFindId(AjPBtcache cache, ajulong pagepos,
                                          const AjPStr id);
static ajulong       btreeSecBucketIdlist(AjPBtcache cache, ajulong pagepos,
                                          AjPList idlist);
static AjPSecBucket  btreeReadSecBucket(AjPBtcache cache, ajulong pagepos);
static void          btreeWriteSecBucket(AjPBtcache cache,
					 const AjPSecBucket bucket,
					 ajulong pagepos);
static ajint         btreeKeywordIdCompare(const void *a, const void *b);
static AjPBtpage     btreeSplitSecLeaf(AjPBtcache cache, AjPBtpage spage);

static AjPSecBucket  btreeSecBucketNew(ajuint n, ajuint idlen);
static void          btreeSecBucketDel(AjPSecBucket *thys);
static void          btreeSecBucketFree(AjPSecBucket *thys);
static void          btreeSecLeftLeaf(AjPBtcache cache, AjPBtKeyWild wild);
static AjBool        btreeSecNextLeafList(AjPBtcache cache, AjPBtKeyWild wild);
static void          btreeReadAllSecLeaves(AjPBtcache cache, AjPList list);

static void          btreeAddToSecBucket(AjPBtcache cache, ajulong pagepos,
					 const AjPStr id);
static void          btreeAddToSecBucketFull(AjPBtcache cache, ajulong pagepos,
                                             const AjPStr id);
static AjBool        btreeReorderSecBuckets(AjPBtcache cache, AjPBtpage leaf);
#if 0
static               void btreeInsertIdOnly(AjPBtcache cache,
					    const AjPBtPri pri);
#endif
static void          btreeSplitRootSec(AjPBtcache cache);
static ajuint        btreeNumInSecBucket(AjPBtcache cache, ajulong pagepos);
static void          btreeInsertKeySec(AjPBtcache cache, AjPBtpage page,
			               const AjPStr key, ajulong less,
			               ajulong greater);
static ajulong       btreeInsertShiftSec(AjPBtcache cache, AjPBtpage *retpage,
				         const AjPStr key);
static void          btreeKeyShiftSec(AjPBtcache cache, AjPBtpage tpage);
static void          btreeInsertNonFullSec(AjPBtcache cache, AjPBtpage page,
				           const AjPStr key, ajulong less,
				           ajulong greater);

static void          btreeStrDel(void** pentry, void* cl);
static void          btreeIdDelFromList(void** pentry, void* cl);
static void          btreeKeyFullSearch(AjPBtcache cache, const AjPStr key,
					AjPList idlist);
static void          btreeKeywordFullSearch(AjPBtcache cache, const AjPStr key,
					    AjPBtcache idcache,
					    AjPList idlist);
static ajint         btreeOffsetCompare(const void *a, const void *b);
static ajint         btreeDbnoCompare(const void *a, const void *b);


static AjPBtMem      btreeAllocPriArray(AjPBtcache cache);
static void          btreeDeallocPriArray(AjPBtcache cache, AjPBtMem node);
static AjPBtMem      btreeAllocSecArray(AjPBtcache cache);
static void          btreeDeallocSecArray(AjPBtcache cache, AjPBtMem node);



static void          btreeAddToHybBucket(AjPBtcache cache, ajulong pagepos,
					 const AjPBtHybrid id);
static void          btreeAddToHybBucketFull(AjPBtcache cache, ajulong pagepos,
                                             const AjPBtHybrid id);
static AjPBtpage     btreeHybFindINode(AjPBtcache cache, AjPBtpage page,
				       const AjPStr item);
static AjPBtpage     btreeHybPageFromKey(AjPBtcache cache,
					 unsigned char *buf, const AjPStr key);
static ajulong       btreeHybInsertShift(AjPBtcache cache, AjPBtpage *retpage,
					 const AjPStr key);
static AjBool        btreeHybReorderBuckets(AjPBtcache cache, AjPBtpage leaf);
static AjPBtpage     btreeHybSplitLeaf(AjPBtcache cache, AjPBtpage spage);
static void          btreeHybInsertKey(AjPBtcache cache, AjPBtpage page,
				       const AjPStr key, ajulong less,
				       ajulong greater);
static void          btreeHybSplitRoot(AjPBtcache cache);
static void          btreeHybDupInsert(AjPBtcache cache, const AjPBtHybrid hyb,
				       AjPBtId btid);
static ajulong       btreeHybBucketIdlist(AjPBtcache cache, ajulong pagepos,
                                          AjPList idlist);
static ajulong       btreeHybBucketIdlistAll(AjPBtcache cache, ajulong pagepos,
                                             AjPList idlist);

static void          btreeGetNumKeys(AjPBtcache cache, unsigned char *buf,
				     ajulong **keys, ajulong **ptrs);
static void          btreeGetNumPointers(AjPBtcache cache, unsigned char *buf,
                                         ajulong **ptrs);
static void          btreeWriteNumNode(AjPBtcache cache, AjPBtpage spage,
				       const ajulong *keys, const ajulong *ptrs,
				       ajuint nkeys);
static ajulong       btreeNumBucketIdlist(AjPBtcache cache, ajulong pagepos,
                                          AjPList idlist);
static ajulong       btreeNumBucketBtidlist(AjPBtcache cache, ajulong pagepos,
                                            const AjPStr idname,
                                            AjPList idlist);
static AjPNumBucket  btreeReadNumBucket(AjPBtcache cache, ajulong pagepos);
static void          btreeWriteNumBucket(AjPBtcache cache,
                                         const AjPNumBucket bucket,
                                         ajulong pagepos);
static void          btreeWriteNumBucketEmpty(AjPBtcache cache,
                                              ajulong pagepos);
static void          btreeNumBucketDel(AjPNumBucket *thys);
static void          btreeNumBucketFree(AjPNumBucket *thys);
static void          btreeAddToNumBucket(AjPBtcache cache, ajulong pagepos,
					 const AjPBtNumId num);
static AjPBtpage     btreeNumFindINode(AjPBtcache cache, AjPBtpage page,
				       ajulong item);
static AjPBtpage     btreeNumPageFromKey(AjPBtcache cache, unsigned char *buf,
					 ajulong key);
static ajuint        btreeNumInNumBucket(AjPBtcache cache, ajulong pagepos);
static AjBool        btreeReorderNumBuckets(AjPBtcache cache, AjPBtpage leaf);
static AjPNumBucket  btreeNumBucketNew(ajuint n);
static ajint         btreeNumIdCompare(const void *a, const void *b);
static AjBool        btreeNumNodeIsFull(const AjPBtcache cache,
					AjPBtpage page);
static void          btreeNumInsertNonFull(AjPBtcache cache, AjPBtpage page,
					   ajulong key, ajulong less,
					   ajulong greater);
static void          btreeNumInsertKey(AjPBtcache cache, AjPBtpage page,
				       ajulong key, ajulong less,
				       ajulong greater);
static void          btreeNumSplitRoot(AjPBtcache cache);
static void          btreeNumKeyShift(AjPBtcache cache, AjPBtpage tpage);
static ajulong       btreeNumInsertShift(AjPBtcache cache, AjPBtpage *retpage,
					 ajulong key);
static AjPBtpage     btreeNumSplitLeaf(AjPBtcache cache, AjPBtpage spage);


static ajulong       btreeFindHybBalanceOne(AjPBtcache cache, ajulong thisNode,
                                            ajulong leftNode, ajulong rightNode,
                                            ajulong lAnchor, ajulong rAnchor,
                                            const AjPBtHybrid hyb);
static void          btreeFindHybMinOne(AjPBtcache cache, ajulong pagepos,
                                        const AjPStr key);

static AjBool        btreeRemoveHybEntryOne(AjPBtcache cache,ajulong pagepos,
                                            const AjPBtHybrid hyb);

static void          btreeAdjustHybBucketsOne(AjPBtcache cache,
                                              AjPBtpage leaf);

static ajulong       btreeRebalanceHybOne(AjPBtcache cache, ajulong thisNode,
                                          ajulong leftNode, ajulong rightNode,
                                          ajulong lAnchor, ajulong rAnchor);

static ajulong       btreeShiftHybOne(AjPBtcache cache, ajulong thisNode,
                                      ajulong balanceNode, ajulong anchorNode);

static ajulong       btreeMergeHybOne(AjPBtcache cache, ajulong thisNode,
                                      ajulong mergeNode, ajulong anchorNode);

static ajulong       btreeCollapseRootHybOne(AjPBtcache cache, ajulong pagepos);

static AjBool        btreeDeleteHybIdTwo(AjPBtcache cache,
                                         const AjPBtHybrid hyb,
                                         AjPBtId did);

static ajulong       btreeFindHybBalanceTwo(AjPBtcache cache, ajulong thisNode,
                                            ajulong leftNode, ajulong rightNode,
                                            ajulong lAnchor, ajulong rAnchor,
                                            ajulong key);

static void          btreeFindHybMinTwo(AjPBtcache cache, ajulong pagepos,
                                        ajulong key);

static AjBool        btreeRemoveHybEntryTwo(AjPBtcache cache, ajulong pagepos,
                                            ajulong key);

static void          btreeAdjustHybBucketsTwo(AjPBtcache cache, AjPBtpage leaf);

static ajulong       btreeRebalanceHybTwo(AjPBtcache cache, ajulong thisNode,
                                          ajulong leftNode, ajulong rightNode,
                                          ajulong lAnchor, ajulong rAnchor);

static ajulong       btreeShiftHybTwo(AjPBtcache cache, ajulong thisNode,
                                      ajulong balanceNode, ajulong anchorNode);

static ajulong       btreeMergeHybTwo(AjPBtcache cache, ajulong thisNode,
                                      ajulong mergeNode, ajulong anchorNode);

static ajulong       btreeCollapseRootHybTwo(AjPBtcache cache, ajulong pagepos);

static ajulong       btreeFindPriBalanceTwo(AjPBtcache cache, ajulong thisNode,
                                            ajulong leftNode, ajulong rightNode,
                                            ajulong lAnchor, ajulong rAnchor,
                                            const AjPBtPri pri);

static void          btreeFindPriMinTwo(AjPBtcache cache, ajulong pagepos,
                                        const AjPStr key);

static AjBool        btreeRemovePriEntryTwo(AjPBtcache cache, ajulong pagepos,
                                            const AjPBtPri pri);

static void          btreeAdjustPriBucketsTwo(AjPBtcache cache, AjPBtpage leaf);

static ajulong       btreeRebalancePriTwo(AjPBtcache cache, ajulong thisNode,
                                          ajulong leftNode, ajulong rightNode,
                                          ajulong lAnchor, ajulong rAnchor);

static ajulong       btreeShiftPriTwo(AjPBtcache cache, ajulong thisNode,
                                      ajulong balanceNode, ajulong anchorNode);

static ajulong       btreeMergePriTwo(AjPBtcache cache, ajulong thisNode,
                                      ajulong mergeNode, ajulong anchorNode);

static ajulong       btreeCollapseRootPriTwo(AjPBtcache cache, ajulong pagepos);


static ajulong       btreeFindPriBalanceOne(AjPBtcache cache, ajulong thisNode,
                                            ajulong leftNode, ajulong rightNode,
                                            ajulong lAnchor, ajulong rAnchor,
                                            const AjPBtPri pri);

static void          btreeFindPriMinOne(AjPBtcache cache, ajulong pagepos,
                                        const AjPStr key);

static AjBool        btreeRemovePriEntryOne(AjPBtcache cache, ajulong pagepos,
                                            const AjPBtPri pri);

static void          btreeAdjustPriBucketsOne(AjPBtcache cache, AjPBtpage leaf);

static ajulong       btreeRebalancePriOne(AjPBtcache cache, ajulong thisNode,
                                          ajulong leftNode, ajulong rightNode,
                                          ajulong lAnchor, ajulong rAnchor);

static ajulong       btreeShiftPriOne(AjPBtcache cache, ajulong thisNode,
                                      ajulong balanceNode, ajulong anchorNode);

static ajulong       btreeMergePriOne(AjPBtcache cache, ajulong thisNode,
                                      ajulong mergeNode, ajulong anchorNode);

static ajulong       btreeCollapseRootPriOne(AjPBtcache cache, ajulong pagepos);

static AjBool        btreeIsSecEmpty(AjPBtcache cache);

static AjBool        btreeBucketSplitCalc(ajuint totalkeys, ajuint totalbuckets,
                                          ajuint maxbucketsize,
                                          ajuint *leftbuckets, ajuint *leftmax,
                                          ajuint *leftkeys,
                                          ajuint *rightbuckets,
                                          ajuint *rightmax,
                                          ajuint *rightkeys);
static AjBool        btreeBucketCalc(ajuint totalkeys, ajuint totalbuckets,
                                    ajuint maxbucketsize,
                                    ajuint *newbuckets, ajuint *newmax);
static const char*   btreeNodetype(const unsigned char* buf);
static void          btreeDumpNumnode(AjPBtcache cache, const AjPBtpage page);
static AjBool        btreeCheckNode(AjPBtcache cache, const AjPBtpage page);
static AjBool        btreeCheckNumnode(AjPBtcache cache, const AjPBtpage page);
static AjBool        btreeCheckNodeHeader(AjPBtcache cache,
                                          const AjPBtpage page,
                                          const char* type);
static void          btreeNocacheFetch(const AjPBtcache cache, AjPBtpage cpage,
                                       ajulong pagepos);
static void          btreePageClear(AjPBtcache cache, AjPBtpage page);
static void          btreePageSetfree(AjPBtcache cache, ajulong pagepos);
static void          btreeFieldInit(void);
static BtreePField   btreeFieldNewField(const BtreePFielddef field);
static BtreePField   btreeFieldNewC(const char* nametxt);
static BtreePField   btreeFieldNewS(const AjPStr name);
static void          btreeFieldDel(BtreePField *Pfield);
static void          btreeFieldMapDel(void** key, void** value, void* cl);




/* @funcstatic btreeFieldInit *************************************************
**
** Initialises the table for named fields
**
** @return [void]
******************************************************************************/

static void btreeFieldInit(void)
{
    ajuint i;
    BtreePField field = NULL;

    if(btreeFieldsTable)
        return;

    btreeFieldsTable = ajTablestrNewConst(30);

    for(i=0; btreeFields[i].Name; i++)
    {
        field = btreeFieldNewField(&btreeFields[i]);
        ajTablePut(btreeFieldsTable,
                   field->Name,
                   (void*) field);
    }

    return;
}




/* @funcstatic btreeFieldNewC *************************************************
**
** Construct a new named field with default values
**
** @param [r] nametxt [const char*] Field name
** @return [BtreePField] New field object
******************************************************************************/

static BtreePField btreeFieldNewC(const char* nametxt)
{
    BtreePField ret;
    ajuint ilen;

    ilen = strlen(nametxt);

    AJNEW0(ret);

    ret->Name = ajStrNewC(nametxt);
    ret->Extension = ajStrNewRes(ilen+2);
    ajStrAssignK(&ret->Extension, 'x');
    ajStrAppendC(&ret->Extension, nametxt);
    ret->Secondary = ajTrue;
    ret->Len = 15;

    return ret;
}




/* @funcstatic btreeFieldNewS *************************************************
**
** Construct a new named field with default values
**
** @param [r] name [const AjPStr] Field name
** @return [BtreePField] New field object
******************************************************************************/

static BtreePField btreeFieldNewS(const AjPStr name)
{
    BtreePField ret;
    ajuint ilen;

    ilen = MAJSTRGETLEN(name);

    AJNEW0(ret);

    ret->Name = ajStrNewS(name);
    ret->Extension = ajStrNewRes(ilen+2);
    ajStrAssignK(&ret->Extension, 'x');
    ajStrAppendS(&ret->Extension, name);
    ret->Secondary = ajTrue;
    ret->Len = 15;

    return ret;
}




/* @funcstatic btreeFieldNewField *********************************************
**
** Construct copy of a known field
**
** @param [r] field [const BtreePFielddef] Source field
** @return [BtreePField] New field object
******************************************************************************/

static BtreePField btreeFieldNewField(const BtreePFielddef field)
{
    BtreePField ret = NULL;

    AJNEW0(ret);

    ret->Name = ajStrNewC(field->Name);
    ret->Extension = ajStrNewC(field->Extension);
    ret->Secondary = field->Secondary;
    ret->Len = field->Len;

    return ret;
}




/* @funcstatic btreeFieldDel **************************************************
**
** Destructor for a named field
**
** @param [d] Pfield [BtreePField*] Field object
** @return [void]
******************************************************************************/

static void btreeFieldDel(BtreePField *Pfield)
{
    BtreePField field = *Pfield;

    ajStrDel(&field->Name);
    ajStrDel(&field->Extension);
    AJFREE(field);
    *Pfield = NULL;
    
    return;
}




/* @funcstatic btreeFieldMapDel ************************************************
**
** Deletes entries from the MSF internal table. Called for each entry in turn.
**
** @param [d] key [void**] Standard argument, table key.
** @param [d] value [void**] Standard argument, table data item.
** @param [r] cl [void*] Standard argument, usually NULL
** @return [void]
** @@
******************************************************************************/

static void btreeFieldMapDel(void** key, void** value, void* cl)
{
    BtreePField field;

    (void) cl;

    field = (BtreePField) *value;

    btreeFieldDel(&field);

    *key = NULL;
    *value = NULL;

    return;
}




/* @func ajBtreeFieldGetLenC **************************************************
**
** Returns the default indexed name length for a named field
**
** @param [r] nametxt [const char*] Field name
** @return [ajuint] Field name length
******************************************************************************/

ajuint ajBtreeFieldGetLenC(const char* nametxt)
{
    ajuint ret = 15;
    const BtreePField field;

    btreeFieldInit();

    field = ajTableFetchC(btreeFieldsTable, nametxt);

    if(field)
        return field->Len;

    return ret;
}




/* @func ajBtreeFieldGetLenS **************************************************
**
** Returns the default indexed name length for a named field
**
** @param [r] name [const AjPStr] Field name
** @return [ajuint] Field name length
******************************************************************************/

ajuint ajBtreeFieldGetLenS(const AjPStr name)
{
    ajuint ret = 15;
    const BtreePField field;

    btreeFieldInit();

    field = ajTableFetchS(btreeFieldsTable, name);

    if(field)
        return field->Len;

    return ret;
}




/* @func ajBtreeFieldGetExtensionC ********************************************
**
** Returns the default index file extension for a named field
**
** @param [r] nametxt [const char*] Field name
** @return [const AjPStr] Filename extension
******************************************************************************/

const AjPStr ajBtreeFieldGetExtensionC(const char* nametxt)
{
    const BtreePField field;
    BtreePField newfield;

    btreeFieldInit();

    field = ajTableFetchC(btreeFieldsTable, nametxt);
    if(field)
        return field->Extension;
        
    newfield = btreeFieldNewC(nametxt);
    ajTablePut(btreeFieldsTable,
               newfield->Name,
               (void*) newfield);

    return newfield->Extension;
}




/* @func ajBtreeFieldGetExtensionS ********************************************
**
** Returns the default index file extension for a named field
**
** @param [r] name [const AjPStr] Field name
** @return [const AjPStr] Filename extension
******************************************************************************/

const AjPStr ajBtreeFieldGetExtensionS(const AjPStr name)
{
    const BtreePField field;
    BtreePField newfield;

    btreeFieldInit();

    field = ajTableFetchS(btreeFieldsTable, name);
    if(field)
        return field->Extension;

    newfield = btreeFieldNewS(name);
    ajTablePut(btreeFieldsTable,
               newfield->Name,
               (void*) newfield);

    return newfield->Extension;
}




/* @func ajBtreeFieldGetSecondaryC ********************************************
**
** Returns whether a named field uses a secondary index
**
** @param [r] nametxt [const char*] Field name
** @return [AjBool] True for a secondary index where indexed terms treturn IDs
**                  False for an ID index where terms return file positions.
******************************************************************************/

AjBool ajBtreeFieldGetSecondaryC(const char* nametxt)
{
    AjBool ret = ajTrue;
    const BtreePField field;

    btreeFieldInit();

    ajStrAssignC(&btreeFieldnameTmp, nametxt);
    field = ajTableFetchC(btreeFieldsTable, nametxt);
    if(field)
        return field->Secondary;

    return ret;
}




/* @func ajBtreeFieldGetSecondaryS ********************************************
**
** Returns whether a named field uses a secondary index
**
** @param [r] name [const AjPStr] Field name
** @return [AjBool] True for a secondary index where indexed terms treturn IDs
**                  False for an ID index where terms return file positions.
******************************************************************************/

AjBool ajBtreeFieldGetSecondaryS(const AjPStr name)
{
    AjBool ret = ajTrue;
    const BtreePField field;

    btreeFieldInit();
    
    field = ajTableFetchS(btreeFieldsTable, name);
    if(field)
        return field->Secondary;

    return ret;
}




/* @func ajBtreeCacheNewC **************************************************
**
** Open a b+tree index file and initialise a cache object
**
** @param [r] filetxt [const char *] name of file
** @param [r] exttxt [const char *] extension of file
** @param [r] idirtxt [const char *] index file directory
** @param [r] mode [const char *] opening mode
** @param [r] compressed [AjBool] Compressed index flag
** @param [r] kwlimit [ajuint] Max key size
** @param [r] pagesize [ajuint] pagesize
** @param [r] cachesize [ajuint] size of cache
** @param [r] pagecount [ajulong] page count
** @param [r] order [ajuint] Tree order
** @param [r] fill [ajuint] Number of entries per bucket
** @param [r] level [ajuint] level of tree
** @param [r] sorder [ajuint] order of secondary tree
** @param [r] sfill [ajuint] Number of entries per secondary bucket
** @param [r] count [ajulong] Number of entries in the index
** @param [r] countall [ajulong] Number of total entries in the index
**
** @return [AjPBtcache] initialised disc block cache structure
** @@
******************************************************************************/

AjPBtcache ajBtreeCacheNewC(const char *filetxt, const char *exttxt,
			    const char *idirtxt, const char *mode,
			    AjBool compressed, ajuint kwlimit,
                            ajuint pagesize, ajuint cachesize,
                            ajulong pagecount,
                            ajuint order, ajuint fill, ajuint level,
                            ajuint sorder, ajuint sfill,
                            ajulong count, ajulong countall)
{
    FILE *fp;
    AjPBtcache cache = NULL;
#if defined (usestat64)
    struct stat64 buf;
#else
    struct stat buf;
#endif

    ajulong filelen = 0L;
    AjBool douncompress = ajFalse;

    AJNEW0(cache);

    cache->filename = ajStrNew();

    if(!*idirtxt)
        ajFmtPrintS(&cache->filename,"%s.%s",filetxt,exttxt);
    else if(idirtxt[strlen(idirtxt)-1] == SLASH_CHAR)
        ajFmtPrintS(&cache->filename,"%s%s.%s",idirtxt,filetxt,exttxt);
    else
        ajFmtPrintS(&cache->filename,"%s%s%s.%s",idirtxt,SLASH_STRING,
		    filetxt,exttxt);
    
    fp = fopen(MAJSTRGETPTR(cache->filename),mode);

    if(!fp)
	return NULL;


    if(ajCharMatchC(mode, "rb")) /* read-only */
    {
#if defined (usestat64)
	if(!stat64(MAJSTRGETPTR(cache->filename), &buf))
#else
        if(!stat(MAJSTRGETPTR(cache->filename), &buf))
#endif
            filelen = buf.st_size;

        cache->readonly = ajTrue;
    }
    else if(ajCharMatchC(mode, "rb+")) /* update */
    {
#if defined (usestat64)
	if(!stat64(MAJSTRGETPTR(cache->filename), &buf))
#else
        if(!stat(MAJSTRGETPTR(cache->filename), &buf))
#endif
            filelen = buf.st_size;
        if(compressed)
            douncompress = ajTrue;
    }
    else if(!ajCharMatchC(mode, "wb+")) /* create */
    {
        ajWarn("ajBtreeCacheNewC unknown mode '%s'", mode);
    }
    
    cache->pagetable = ajTableulongNewConst(cachesize);

    cache->listLength = 0;

    cache->lru   = NULL;
    cache->mru   = NULL;
    cache->fp    = fp;
    
    cache->replace    = ajStrNew();
    cache->numreplace = 0L;
    
    if(pagesize>0)
	cache->pagesize = pagesize;
    else
	cache->pagesize = BT_PAGESIZE;

    cache->level     = level;

    cache->order      = order;
    cache->nperbucket = fill;
    cache->totsize    = filelen;
    cache->filesize    = filelen;
    cache->pagecount  = pagecount;
    cache->cachesize  = cachesize;
    cache->extendsize = (ajulong) cache->pagesize * (ajulong) cache->cachesize;
    cache->sorder = sorder;
    cache->slevel = 0;
    cache->snperbucket = sfill;
    cache->countunique = count;
    cache->countall = countall;
    cache->kwlimit = kwlimit;
    cache->compressed = compressed;


    cache->bmem = NULL;
    cache->tmem = NULL;

    cache->bsmem = NULL;
    cache->tsmem = NULL;

    if(douncompress)
        btreeCacheUncompress(cache);

    return cache;
}




/* @func ajBtreeCacheNewS **************************************************
**
** Open a b+tree index file and initialise a cache object
**
** @param [r] file [const AjPStr] name of file
** @param [r] ext [const AjPStr] extension of file
** @param [r] idir [const AjPStr] index file directory
** @param [r] mode [const char *] opening mode
** @param [r] compressed [AjBool] Compressed index flag
** @param [r] kwlimit [ajuint] Max key size
** @param [r] pagesize [ajuint] pagesize
** @param [r] cachesize [ajuint] size of cache
** @param [r] pagecount [ajulong] page count
** @param [r] order [ajuint] Tree order
** @param [r] fill [ajuint] Number of entries per bucket
** @param [r] level [ajuint] level of tree
** @param [r] sorder [ajuint] order of secondary tree
** @param [r] sfill [ajuint] Number of entries per secondary bucket
** @param [r] count [ajulong] Number of entries in the index
** @param [r] countall [ajulong] Number of total entries in the index
**
** @return [AjPBtcache] initialised disc block cache structure
** @@
******************************************************************************/

AjPBtcache ajBtreeCacheNewS(const AjPStr file, const AjPStr ext,
			    const AjPStr idir, const char *mode,
                            AjBool compressed, ajuint kwlimit,
                            ajuint pagesize, ajuint cachesize,
                            ajulong pagecount,
                            ajuint order, ajuint fill, ajuint level,
                            ajuint sorder, ajuint sfill,
                            ajulong count, ajulong countall)
{
    return ajBtreeCacheNewC(MAJSTRGETPTR(file), MAJSTRGETPTR(ext),
                            MAJSTRGETPTR(idir), mode,
                            compressed, kwlimit, pagesize, cachesize,
                            pagecount, order, fill, level,
                            sorder, sfill,
                            count, countall);
}




/* @func ajBtreeCacheNewReadC *************************************************
**
** Open an existing b+tree index file and initialise a cache object
**
** @param [r] filetxt [const char *] name of file
** @param [r] exttxt [const char *] extension of file
** @param [r] idirtxt [const char *] index file directory
**
** @return [AjPBtcache] initialised disc block cache structure
** @@
******************************************************************************/

AjPBtcache ajBtreeCacheNewReadC(const char *filetxt, const char *exttxt,
                                const char *idirtxt)
{
    AjPBtcache cache = NULL;

    ajuint pagesize;
    ajuint cachesize;
    ajuint order;
    ajuint sorder;
    ajuint fill;
    ajuint sfill;
    ajuint level;
    ajulong pagecount;
    ajulong count;
    ajulong countall;
    ajuint kwlimit;
    AjBool secondary = ajTrue;
    AjBool compressed = ajFalse;
    
/* first read the parameter file */
   
    if(!ajBtreeReadParamsC(filetxt, exttxt, idirtxt,
                           &secondary, &compressed,
                           &kwlimit, &pagesize, &cachesize, &pagecount,
                           &order, &fill, &level,
                           &sorder, &sfill,
                           &count, &countall))
    {
         return ajFalse;
    }

    if(secondary)
        cache = ajBtreeSecCacheNewC(filetxt, exttxt, idirtxt, "rb",
                                    compressed, kwlimit,
                                    pagesize, cachesize, pagecount,
                                    order, fill, level,
                                    sorder, sfill,
                                    count, countall);
    else
        cache = ajBtreeCacheNewC(filetxt, exttxt, idirtxt, "rb",
                                 compressed, kwlimit,
                                 pagesize, cachesize, pagecount,
                                 order, fill, level,
                                 sorder, sfill,
                                 count, countall);

    if(cache)
        cache->secondary = secondary;
    
    return cache;
}




/* @func ajBtreeCacheNewReadS *************************************************
**
** Open an existing b+tree index file and initialise a cache object
**
** @param [r] file [const AjPStr] name of file
** @param [r] ext  [const AjPStr] extension of file
** @param [r] idir [const AjPStr] index file directory
**
** @return [AjPBtcache] initialised disc block cache structure
** @@
******************************************************************************/

AjPBtcache ajBtreeCacheNewReadS(const AjPStr file, const AjPStr ext,
                                const AjPStr idir)
{
    return ajBtreeCacheNewReadC(MAJSTRGETPTR(file), MAJSTRGETPTR(ext),
                                MAJSTRGETPTR(idir));
}




/* @func ajBtreeCacheNewUpdateC ***********************************************
**
** Open an existing b+tree index file for update and initialise a cache object
**
** @param [r] filetxt [const char *] name of file
** @param [r] exttxt [const char *] extension of file
** @param [r] idirtxt [const char *] index file directory
**
** @return [AjPBtcache] initialised disc block cache structure
** @@
******************************************************************************/

AjPBtcache ajBtreeCacheNewUpdateC(const char *filetxt, const char *exttxt,
                                  const char *idirtxt)
{
    AjPBtcache cache = NULL;

    ajuint pagesize;
    ajuint cachesize;
    ajuint order;
    ajuint sorder;
    ajuint fill;
    ajuint sfill;
    ajuint level;
    ajulong pagecount;
    ajulong count;
    ajulong countall;
    ajuint kwlimit;
    AjBool secondary = ajTrue;
    AjBool compressed = ajFalse;
    
/* first read the parameter file */
   
    if(!ajBtreeReadParamsC(filetxt, exttxt, idirtxt,
                           &secondary, &compressed,
                           &kwlimit, &pagesize, &cachesize, &pagecount,
                           &order, &fill, &level,
                           &sorder, &sfill,
                           &count, &countall))
    {
         return ajFalse;
    }

    if(secondary)
        cache = ajBtreeSecCacheNewC(filetxt, exttxt, idirtxt, "rb+",
                                    compressed, kwlimit,
                                    pagesize, cachesize, pagecount,
                                    order, fill, level,
                                    sorder, sfill,
                                    count, countall);
    else
        cache = ajBtreeCacheNewC(filetxt, exttxt, idirtxt, "rb+",
                                 compressed, kwlimit,
                                 pagesize, cachesize, pagecount,
                                 order, fill, level,
                                 sorder, sfill,
                                 count, countall);

    if(cache)
        cache->secondary = secondary;
    
    return cache;
}




/* @func ajBtreeCacheNewUpdateS ***********************************************
**
** Open an existing b+tree index file for update and initialise a cache object
**
** @param [r] file [const AjPStr] name of file
** @param [r] ext  [const AjPStr] extension of file
** @param [r] idir [const AjPStr] index file directory
**
** @return [AjPBtcache] initialised disc block cache structure
** @@
******************************************************************************/

AjPBtcache ajBtreeCacheNewUpdateS(const AjPStr file, const AjPStr ext,
                                  const AjPStr idir)
{
    return ajBtreeCacheNewUpdateC(MAJSTRGETPTR(file), MAJSTRGETPTR(ext),
                                  MAJSTRGETPTR(idir));
}




/* @funcstatic btreePageNew ***************************************************
**
** Construct a cache page object
**
** @param [u] cache [AjPBtcache] cache
**
** @return [AjPBtpage] initialised disc block cache structure
** @@
******************************************************************************/

static AjPBtpage btreePageNew(AjPBtcache cache)
{
    AjPBtpage thys = NULL;

    /* ajDebug("In btreePageNew\n"); */

    AJNEW0(thys);
    AJCNEW0(thys->buf,cache->pagesize);
    thys->next = NULL;
    thys->prev = NULL;
    
    ++cache->listLength;

    
    return thys;
}




/* @funcstatic btreeBucketNew *********************************************
**
** Construct a bucket object
**
** @param [r] n [ajuint] Number of IDs
**
** @return [AjPBucket] initialised disc block cache structure
** @@
******************************************************************************/

static AjPBucket btreeBucketNew(ajuint n)
{
    AjPBucket bucket = NULL;
    ajuint i;

    /* ajDebug("In btreeBucketNew\n"); */
    
    if(n)
    {
        if(statSaveBucketNext)
        {
            bucket = statSaveBucket[--statSaveBucketNext];
            for(i=0;i<bucket->Maxentries;++i)
                ajStrAssignClear(&bucket->Ids[i]->id);
            if(n > bucket->Maxentries)
            {
                AJCRESIZE0(bucket->keylen,bucket->Maxentries,n);
                AJCRESIZE0(bucket->Ids,bucket->Maxentries,n);
                for(i=bucket->Maxentries;i<n;++i)
                    bucket->Ids[i] = ajBtreeIdNew();
                bucket->Maxentries = n;
            }
        }
        else
        {
            AJNEW0(bucket);
            AJCNEW0(bucket->Ids,n);
            AJCNEW0(bucket->keylen,n);
            for(i=0;i<n;++i)
                bucket->Ids[i] = ajBtreeIdNew();
            bucket->Maxentries = n;
            
        }
        
    }
    else 
    {
        if(statSaveBucketEmptyNext)
            bucket = statSaveBucketEmpty[--statSaveBucketEmptyNext];
        else
            AJNEW0(bucket);
    }

    bucket->NodeType = BT_BUCKET;
    bucket->Nentries = n;
    bucket->Overflow = 0L;
    
    return bucket;
}




/* @funcstatic btreeCacheLocate *******************************************
**
** Search for a page in the cache
**
** @param [u] cache [AjPBtcache] cache structure
** @param [r] page [ajulong] page number to locate
**
** @return [AjPBtpage]	pointer to page or NULL if not found
** @@
******************************************************************************/

static AjPBtpage btreeCacheLocate(AjPBtcache cache, ajulong page)
{
    AjPBtpage cpage = NULL;

    /*ajDebug("In btreeCacheLocate %Lu\n", page);*/
    

    cpage = ajTableFetchmodV(cache->pagetable, (const void*) &page);

    if(!cpage)
        return NULL;

    cache->cachehits++;

    return cpage;
    
/*
  for(cpage = cache->mru; cpage; cpage = cpage->prev)
	if(cpage->pagepos == page)
        {
            cache->cachehits++;
	    return cpage;
        }

    return NULL;
*/

}




/* @funcstatic btreeCacheUnlink *******************************************
**
** Remove links to a cache page and return the address of the page
**
** @param [w] cache [AjPBtcache] cache structure
** @param [u] cpage [AjPBtpage] cache page
**
** @return [void]
** @@
******************************************************************************/

static void btreeCacheUnlink(AjPBtcache cache, AjPBtpage cpage)
{
    /* ajDebug("In btreeCacheUnlink\n"); */

    ajTableRemove(cache->pagetable, (void*) &cpage->pagepos);

    if(cache->mru == cpage)
    {
	cache->mru = cpage->prev;

	if(cpage->prev)
	    cpage->prev->next = NULL;

	if(cache->lru == cpage)
	    cache->lru = NULL;
    }
    else if(cache->lru == cpage)
    {
	cache->lru = cpage->next;

	if(cpage->next)
	    cpage->next->prev = NULL;
    }
    else
    {
	cpage->prev->next = cpage->next;
	cpage->next->prev = cpage->prev;
    }

    return;
}




/* @funcstatic btreeCacheMruAdd *******************************************
**
** Insert a cache page at the mru position
**
** @param [w] cache [AjPBtcache] cache structure
** @param [u] cpage [AjPBtpage] cache page
**
** @return [void]
** @@
******************************************************************************/

static void btreeCacheMruAdd(AjPBtcache cache, AjPBtpage cpage)
{

    /* ajDebug("In btreeCacheMruAdd\n"); */

    ajTablePut(cache->pagetable,
               (void*) &cpage->pagepos, (void*) cpage);

    cpage->prev = cache->mru;
    cpage->next = NULL;

    if(cache->mru)
	cache->mru->next = cpage;

    if(!cache->lru)
	cache->lru = cpage;

    cache->mru = cpage;

    return;
}




/* @funcstatic btreeCacheLruUnlink ****************************************
**
** Remove links to an LRU cache page
**
** @param [w] cache [AjPBtcache] cache structure
**
** @return [AjPBtpage]	pointer to unlinked cache page
** @@
******************************************************************************/

static AjPBtpage btreeCacheLruUnlink(AjPBtcache cache)
{
    AjPBtpage ret;

    /* ajDebug("In btreeCacheLruUnlink\n"); */

    if(cache->lru->dirty != BT_LOCK)
    {
	if(!cache->lru)
	    ajFatal("btreeCacheLruUnlink: No pages nodes found in cache %S",
                    cache->filename);

	ret = cache->lru;
	ret->next->prev = NULL;
	cache->lru = ret->next;
        ajTableRemove(cache->pagetable, (void*) &ret->pagepos);

        return ret;
    }
    
    for(ret=cache->lru; ret; ret=ret->next)
	if(ret->dirty != BT_LOCK)
	    break;

    if(!ret)
	ajFatal("Too many locked cache page in cache %S. "
                "Try increasing cachesize (%u)",
                cache->filename, cache->cachesize);

    btreeCacheUnlink(cache,ret);
    
    return ret;
}




/* @funcstatic btreeCacheDestage **********************************************
**
** Destage a cache page
**
** @param [u] cache [AjPBtcache] cache
** @param [u] cpage [AjPBtpage] cache page
**
** @return [void]
** @@
******************************************************************************/

static void btreeCacheDestage(AjPBtcache cache, AjPBtpage cpage)
{
    ajuint written = 0;
    ajuint retries = 0;

    /* ajDebug("In btreeCacheDestage\n");*/

/* no speedup from extending the file */
/*
//  while(cache->totsize > cache->filesize)
//    {
//        cache->filesize += cache->extendsize;
//        fseek(cache->fp,(cache->filesize-1L),SEEK_SET);
//        fwrite("", 1, 1, cache->fp);
//    }
*/

    if(fseek(cache->fp, cpage->pagepos,SEEK_SET))
    {
        ajWarn("fseek %Lu failed for cache '%S' %d: '%s'",
               cpage->pagepos, cache->filename, errno, strerror(errno));
	fseek(cache->fp,0L,SEEK_END);
    }

    while(written != cache->pagesize && retries != BT_MAXRETRIES)
    {
	written += fwrite((void *)cpage->buf,1,cache->pagesize-written,
			  cache->fp);
	++retries;
    }

    if(retries == BT_MAXRETRIES)
	ajFatal("Maximum retries (%u) reached in btreeCacheDestage "
                "cache %S error: '%s'",
		BT_MAXRETRIES, cache->filename, strerror(ferror(cache->fp)));

    cpage->dirty = BT_CLEAN;
    
    cache->writes++;

    return;
}




/* @funcstatic btreeCacheFetch ************************************************
**
** Fetch a cache page from disc
**
** @param [u] cache [AjPBtcache] cache
** @param [w] cpage [AjPBtpage] cache page 
** @param [r] pagepos [ajulong] page number
**
** @return [void]
** @@
******************************************************************************/

static void btreeCacheFetch(AjPBtcache cache, AjPBtpage cpage,
			    ajulong pagepos)
{
    ajuint sum = 0;
    ajuint retries = 0;

    /* ajDebug("In btreeCacheFetch\n"); */
    
    if(fseek(cache->fp,pagepos,SEEK_SET))
	ajFatal("Seek error '%s' in ajBtreeCachefetch file %S",
                strerror(ferror(cache->fp)), cache->filename);

    while(sum != cache->pagesize && retries != BT_MAXRETRIES)
    {
	sum += fread((void *)(cpage->buf+sum),1,cache->pagesize-sum,
		     cache->fp);

	++retries;
    }
    
    if(retries == BT_MAXRETRIES)
	ajFatal("Maximum retries (%u) reached in btreeCacheFetch for page %Lu"
                " cache '%S'",
		BT_MAXRETRIES,pagepos, cache->filename);

    cpage->pagepos = pagepos;
    cache->reads++;
    
    return;
}




/* @funcstatic btreeCacheWriteCompress ****************************************
**
** Write a compressed cache page
**
** @param [u] cache [AjPBtcache] cache
** @param [u] cpage [AjPBtpage] cache page
** @param [r] pagepos [ajulong] Cache file position
** @param [r] pagesize [ajuint] Compressed page size
**
** @return [void]
** @@
******************************************************************************/

static void btreeCacheWriteCompress(AjPBtcache cache, AjPBtpage cpage,
                                    ajulong pagepos, ajuint pagesize)
{
    ajuint written = 0;
    ajuint retries = 0;

    /* ajDebug("In btreeCacheWriteCompress\n");*/

    if(fseek(cache->fp, pagepos,SEEK_SET))
    {
        ajWarn("fseek %Lu failed for cache '%S' %d: '%s'",
               pagepos, cache->filename, errno, strerror(errno));
	fseek(cache->fp,0L,SEEK_END);
    }

    while(written != pagesize && retries != BT_MAXRETRIES)
    {
	written += fwrite((void *)cpage->buf,1,pagesize-written,
			  cache->fp);
	++retries;
    }

    if(retries == BT_MAXRETRIES)
	ajFatal("Maximum retries (%u) reached in btreeCacheWriteCompress "
                "cache %S error: '%s'",
		BT_MAXRETRIES, cache->filename, strerror(ferror(cache->fp)));

    return;
}




/* @funcstatic btreeCacheWriteUncompress **************************************
**
** Rewrite an uncompressed cache page
**
** @param [u] cache [AjPBtcache] cache
** @param [u] cpage [AjPBtpage] cache page
** @param [r] pagepos [ajulong] Cache file position
**
** @return [void]
** @@
******************************************************************************/

static void btreeCacheWriteUncompress(AjPBtcache cache, AjPBtpage cpage,
                                      ajulong pagepos)
{
    ajuint written = 0;
    ajuint retries = 0;

    /* ajDebug("In btreeCacheWriteCompress\n");*/

    if(fseek(cache->fp, pagepos,SEEK_SET))
    {
        ajWarn("fseek %Lu failed for cache '%S' %d: '%s'",
               pagepos, cache->filename, errno, strerror(errno));
	fseek(cache->fp,0L,SEEK_END);
    }

    while(written != cache->pagesize && retries != BT_MAXRETRIES)
    {
	written += fwrite((void *)cpage->buf,1,cache->pagesize-written,
			  cache->fp);
	++retries;
    }

    if(retries == BT_MAXRETRIES)
	ajFatal("Maximum retries (%u) reached in btreeCacheWriteUncompress "
                "cache %S error: '%s'",
		BT_MAXRETRIES, cache->filename, strerror(ferror(cache->fp)));

    return;
}




/* @funcstatic btreeCacheCompress *********************************************
**
** Compress a b+tree cache on closing
**
** @param [u] thys [AjPBtcache] Expanded cache to be compressed
**
** @return [ajulong] File size after compression
** @@
******************************************************************************/

static ajulong btreeCacheCompress(AjPBtcache thys)
{
    ajulong* newpagepos  = NULL;
    ajuint*  newpagesize = NULL;
    ajulong pagecount = 0;
    AjPBtpage page = NULL;
    ajulong i;
    /* ajuint j; */
    /* ajuint icnt; */
    ajuint freepages = 0;
    ajulong newsize = 0L;
    ajulong oldpos = 0L;

    page = btreePageNew(thys);
    pagecount = ajBtreeGetPagecount(thys);
    AJCNEW(newpagepos, pagecount);
    AJCNEW(newpagesize, pagecount);

    for(i=0; i < pagecount; i++)
    {
        btreeCacheFetch(thys, page, oldpos);
        oldpos += thys->pagesize;

        newpagepos[i] = newsize;
        newpagesize[i] = ajBtreePageGetSize(page);
        newsize += newpagesize[i];

        /*
        buf = page->buf;

        icnt=0;

        for(j=newpagesize[i]; j < thys->pagesize; j++)
        {
            if(buf[j])
                icnt++;
        }

        if(icnt)
            ajWarn("Page %S %u at %Lu has %u non-zero bytes (%s)",
                   thys->filename, i, page->pagepos,
                   icnt, btreeNodetype(buf));
        */
    }

    newsize += thys->pagesize - newpagesize[pagecount-1];
    newpagesize[pagecount-1] = thys->pagesize;

    oldpos = 0L;

    for(i=0; i < pagecount; i++)
    {
        btreeCacheFetch(thys, page, oldpos);
        oldpos += thys->pagesize;

        if(!btreePageCompress(page, newpagepos, thys->pagesize))
        {
            freepages++;
            continue;
        }

        btreeCacheWriteCompress(thys, page, newpagepos[i], newpagesize[i]);
    }

    thys->filesize = newsize;
    AJFREE(newpagepos);
    AJFREE(newpagesize);
    AJFREE(page->buf);
    AJFREE(page);

    return newsize;
}




/* @funcstatic btreeCacheUncompress *******************************************
**
** Uncompress a b+tree cache on opening
**
** @param [u] thys [AjPBtcache] Compressed cache to be expanded
**
** @return [ajulong] File size compressed
** @@
*******************************************************************************/

static ajulong btreeCacheUncompress(AjPBtcache thys)
{
    ajuint  oldpagesize;
    AjPBtpage page = NULL;
    ajulong i;
    ajuint icnt;
    ajuint freepages = 0;
    ajulong newsize = 0L;
    ajulong oldpos = 0L;
    ajulong newpos = 0L;
    unsigned char* buf = NULL;
    ajulong oldsize;
    AjPTable newpostable;
    ajulong *oldpagepos = NULL;
    ajulong *newpagepos = NULL;

    ajDebug("btreeCacheUncompress %S size: %Lu pages: %Lu",
            thys->filename, thys->totsize, thys->pagecount);

    AJCNEW0(oldpagepos, thys->pagecount);
    AJCNEW0(newpagepos, thys->pagecount);
    oldsize = thys->totsize;
    page = btreePageNew(thys);
    newpostable = ajTableulongNewConst((ajint) thys->pagecount);
    oldpos = 0L;

    for(i=0; i < thys->pagecount; i++)
    {
        btreeCacheFetch(thys, page, oldpos);
        oldpagesize = ajBtreePageGetSize(page);

        oldpagepos[i] = oldpos;
        newpagepos[i] = newpos;
        ajTablePut(newpostable, (void*) &oldpagepos[i],
                   (void*) &newpagepos[i]);
        buf = page->buf;

        oldpos += oldpagesize;
        newpos += thys->pagesize;
   }

    thys->totsize = (ajulong) thys->pagesize * thys->pagecount;

    oldpos = 0L;
    newpos = 0L;
    for(i=thys->pagecount; i > 0; i--)
    {
        btreeCacheFetch(thys, page, oldpagepos[i-1]);

        if(!btreePageUncompress(page, newpostable, thys->pagesize))
        {
            freepages++;
            continue;
        }

        oldpagesize = ajBtreePageGetSize(page);
        icnt=thys->pagesize - oldpagesize;
        if(icnt)
            memset(buf+oldpagesize, 0, icnt);
        
        btreeCacheWriteUncompress(thys, page, (i-1)*thys->pagesize);

    }

    thys->filesize = newsize;

    AJFREE(oldpagepos);
    AJFREE(newpagepos);

    ajTableFree(&newpostable);

    AJFREE(page->buf);
    AJFREE(page);

    return oldsize;
}




/* @func ajBtreeCacheDel ******************************************************
**
** Close a b+tree cache
**
** @param [w] Pthis [AjPBtcache*] Cache object
**
** @return [ajulong] Index file size
** @@
******************************************************************************/

ajulong ajBtreeCacheDel(AjPBtcache *Pthis)
{
    AjPBtcache thys;
    AjPBtpage  page  = NULL;
    AjPBtpage  temp  = NULL;
    ajulong ret = 0L;

    if(!Pthis || !*Pthis) return ret;
    thys = *Pthis;

    /* ajDebug("In ajBtreeCacheDel\n"); */

    /* clear the dirty / locked pages */

    if(!thys->readonly)
        ajBtreeCacheSync(thys, 0L);

    if(thys->bmem)
        ajBtreeFreePriArray(thys);
    if(thys->bsmem)
        ajBtreeFreeSecArray(thys);

    /* tidy up clean (read not written or locked) cached pages */

    for(page=thys->lru;page;page=temp)
    {
	temp = page->next;
        ajTableRemove(thys->pagetable, (void*) &page->pagepos);
	AJFREE(page->buf);
	AJFREE(page);
    }

    if(!thys->readonly)
    {
        /* Ubuntu warns if ftruncate return is not used */
        if(ftruncate(fileno(thys->fp), thys->totsize) == -1)
        {
        }

        if(thys->compressed)
        {
            ret = btreeCacheCompress(thys);
            /* Ubuntu warns if ftruncate return is not used */
            if(ftruncate(fileno(thys->fp), ret) == -1)
            {
            }
        }
    }
    else
        ret = thys->totsize;

    ajStrDel(&thys->filename);
    ajStrDel(&thys->replace);

    fclose(thys->fp);

    ajTableFree(&thys->pagetable);

    AJFREE(*Pthis);
    *Pthis = NULL;
    
    return ret;
}




/* @func ajBtreeCacheIsSecondary *********************************************
**
** Test whether a cache is a secondary index
**
** @param [r] thys [const AjPBtcache] Cache object
**
** @return [AjBool] True if cache is secondary
******************************************************************************/

AjBool ajBtreeCacheIsSecondary(const AjPBtcache thys)
{
    return thys->secondary;
}




/* @func ajBtreeCacheStatsOut *************************************************
**
** Cache statistics for writing a new index
**
** @param [u] outf [AjPFile] output file file
** @param [r] cache [const AjPBtcache] cache object
** @param [u] Pcache [ajulong*] Number of cache reads to date
** @param [u] Preads [ajulong*] Number of disk reads to date
** @param [u] Pwrites [ajulong*] Number of disk writes to date
** @param [u] Psize [ajulong*] Number of cache pages to date
**
** @return [void]
**
******************************************************************************/

void ajBtreeCacheStatsOut(AjPFile outf, const AjPBtcache cache,
                          ajulong* Pcache, ajulong* Preads,
                          ajulong* Pwrites, ajulong* Psize)
{
    ajulong lread;
    ajulong lcache;
    ajulong lwrite;
    ajulong lnew;
    ajulong lpages;

    ajlong lrewrite;

    if(!cache) return;

    lpages = cache->totsize / (ajulong) cache->pagesize;
    lcache = cache->cachehits - *Pcache;
    lread = cache->reads - *Preads;
    lwrite = cache->writes - *Pwrites;
    if(lpages > *Psize)
        lnew = lpages - *Psize;
    else
        lnew = 0L;

    /*
    ** writes counts those committed to disk
    ** newpages counts growth in index size
    ** pending writes may give a negative count for rewrites
    */
    lrewrite = (ajlong)lwrite - (ajlong)lnew;

    ajFmtPrintF(outf,
                "ajBtreeCacheStatsOut '%S' cached: %Lu reads: %8Lu "
                "writes: %8Lu rewrite: %8Ld pages: %8Lu\n",
                cache->filename,  lcache,  lread, lwrite, lrewrite, lnew);

    *Pcache = cache->cachehits;
    *Preads = cache->reads;
    *Pwrites = cache->writes;
    *Psize = lpages;

    return;
}




/* @funcstatic btreeCacheControl **********************************************
**
** Master control function for cache read/write
**
** @param [w] cache [AjPBtcache] name of file
** @param [r] pagepos [ajulong] page number
** @param [r] isread [AjBool] is this a read operation?
**
** @return [AjPBtpage] disc cache page pointer
** @@
******************************************************************************/

static AjPBtpage btreeCacheControl(AjPBtcache cache, ajulong pagepos,
				    AjBool isread)
{
    AjPBtpage ret      = NULL;

    /* ajDebug("In btreeCacheControl\n"); */

    if(pagepos < cache->totsize)
        ret = btreeCacheLocate(cache,pagepos);

    if(ret)
    {
	btreeCacheUnlink(cache,ret);
    }
    else
    {
	if(cache->listLength == cache->cachesize)
	{
	    ret = btreeCacheLruUnlink(cache);

	    if(ret->dirty == BT_DIRTY)
		btreeCacheDestage(cache,ret);

	    if(isread || pagepos!=cache->totsize)
		btreeCacheFetch(cache,ret,pagepos);
            else
                btreePageClear(cache, ret);
	}
	else
	{
	    ret = btreePageNew(cache);

	    if(isread || pagepos!=cache->totsize)
		btreeCacheFetch(cache,ret,pagepos);
	}

	if(!isread)
	    ret->pagepos = pagepos;
	else
	    ret->dirty = BT_CLEAN;
    }

    btreeCacheMruAdd(cache,ret);

    return ret;
}




/* @func ajBtreeCacheRead *****************************************************
**
** Get a pointer to a disc cache page
**
** @param [w] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
**
** @return [AjPBtpage] disc cache page pointer
** @@
******************************************************************************/

AjPBtpage ajBtreeCacheRead(AjPBtcache cache, ajulong pagepos)
{
    AjPBtpage ret = NULL;

    /* ajDebug("In ajBtreeCacheRead\n"); */

    ret = btreeCacheControl(cache,pagepos,BT_READ);

    return ret;
}




/* @func ajBtreeCacheSync *********************************************
**
** Sync all dirty cache pages
**
** @param [u] cache [AjPBtcache] cache
** @param [r] rootpage [ajulong] root page
**
** @return [void]
** @@
******************************************************************************/

void ajBtreeCacheSync(AjPBtcache cache, ajulong rootpage)
{
    AjPBtpage page = NULL;
    ajuint numlocked = 0;
    ajuint numwrite = 0;

    /* ajDebug("In ajBtreeCacheSync\n");*/

    if(!cache->mru) /* cache unused*/
        return;

    statCallSync++;

    for(page=cache->lru;page;page=page->next)
    {
	if(page->dirty)         /* BT_LOCK or BT_DIRTY */
	{
            numwrite++;
            if(page->dirty == BT_LOCK)
                numlocked++;
            btreeCacheDestage(cache,page);
        }
    }

    if(rootpage)
    {
        if(numlocked > 2)
            ajWarn("CacheSync cache '%S' root: %Lu locked: %u",
                   cache->filename, rootpage, numlocked);
    }
    else
    {
        if(numlocked > 1)
            ajWarn("CacheSync cache '%S' root: zero locked: %u",
                   cache->filename, numlocked);
    }
    
                
    page = btreeCacheLocate(cache,rootpage);
    page->dirty = BT_LOCK;
    page->lockfor = 1001;

    if(rootpage)
    {
        page = btreeCacheLocate(cache,0L);
        page->dirty = BT_LOCK;
        page->lockfor = 1002;
    }

    statSyncLocked += numlocked;
    statSyncWrite += numwrite;
    return;
}




/* @func ajBtreeCacheRootSync *********************************************
**
** Reset locked pages to dirty and relock the specified secondary root page.
**
** Pages will be written only when they are removed from the cache.
**
** This function clears all locks.
** The caller must relock the root page (zero) if needed.
**
** @param [u] cache [AjPBtcache] cache
** @param [r] rootpage [ajulong] secondary (locked) root page
**
** @return [void]
** @@
******************************************************************************/

void ajBtreeCacheRootSync(AjPBtcache cache, ajulong rootpage)
{
    AjPBtpage page = NULL;
    ajuint numlocked = 0;
    ajuint numunlocked = 0;

    /*ajDebug("In ajBtreeCacheRootSync '%S' rootpage:%Lu\n",
      cache->filename, rootpage);*/

    statCallRootSync++;

    for(page=cache->lru;page;page=page->next)
    {
        if(page->dirty == BT_LOCK)
        {
            numlocked++;
            if(page->pagepos)
            {
                if(page->pagepos != rootpage)
                {
                    numunlocked++;
                    page->dirty = BT_DIRTY;
                    ajWarn("ajBtreeCacheRootSync locked '%Lu' lockfor: %u",
                            page->pagepos, page->lockfor);
#if AJINDEX_DEBUG                    
                    ajDebug("ajBtreeCacheRootSync locked '%Lu' lockfor: %u\n",
                            page->pagepos, page->lockfor);
#endif
                }
            }
        }
    }
    
    if(numunlocked > statRootSyncMaxUnlocked)
        statRootSyncMaxUnlocked = numunlocked;

    if(numlocked > statRootSyncMaxLocked)
        statRootSyncMaxLocked = numlocked;

    statRootSyncLocked += numlocked;
    statRootSyncUnlocked += numunlocked;

    page = btreeCacheLocate(cache,rootpage);
    page->dirty = BT_LOCK;      /* (re)lock the secondary root page */
    page->lockfor = 1011;

    return;
}




/* @func ajBtreeCacheWrite ******************************************
**
** Get a pointer to a disc cache page for writing
**
** @param [w] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
**
** @return [AjPBtpage] disc cache page pointer
** @@
******************************************************************************/

AjPBtpage ajBtreeCacheWrite(AjPBtcache cache, ajulong pagepos)
{
    AjPBtpage ret = NULL;

#if AJINDEX_DEBUG
    ajDebug("ajBtreeCacheWrite %Lu\n", pagepos);
#endif

    ret = btreeCacheControl(cache,pagepos,BT_WRITE);

    return ret;
}




/* @func ajBtreeCacheWriteBucket ******************************************
**
** Get a pointer to a disc cache page for writing
**
** @param [w] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
**
** @return [AjPBtpage] disc cache page pointer
** @@
******************************************************************************/

AjPBtpage ajBtreeCacheWriteBucket(AjPBtcache cache, ajulong pagepos)
{
    AjPBtpage ret = NULL;

    /* ajDebug("In ajBtreeCacheWriteBucket\n");*/

    ret = btreeCacheControl(cache,pagepos,BT_WRITE);

    return ret;
}




/* @func ajBtreeCacheWriteNode ******************************************
**
** Get a pointer to a disc cache page for writing a node
**
** @param [w] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
**
** @return [AjPBtpage] disc cache page pointer
** @@
******************************************************************************/

AjPBtpage ajBtreeCacheWriteNode(AjPBtcache cache, ajulong pagepos)
{
    AjPBtpage ret = NULL;

    /* ajDebug("In ajBtreeCacheWriteNode\n");*/

    ret = btreeCacheControl(cache,pagepos,BT_WRITE);

    return ret;
}




/* @func ajBtreeCacheWriteBucketnew *******************************************
**
** Get a pointer to a new disc cache page for writing a bucket.
**
** Clears the page.
**
** @param [w] cache [AjPBtcache] cache
**
** @return [AjPBtpage] disc cache page pointer
** @@
******************************************************************************/

AjPBtpage ajBtreeCacheWriteBucketnew(AjPBtcache cache)
{
    AjPBtpage ret = NULL;
    unsigned char *p;

#if AJINDEX_DEBUG
    ajDebug("ajBtreeCacheWriteBucketnew pagepos:%Lu\n", cache->totsize);
#endif

    ret = btreeCacheControl(cache,cache->totsize,BT_WRITE);
    p = ret->buf;
    AJCSET0(p, cache->pagesize); /* clear buffer to zeros */
/*
    ret->next = NULL;
    ret->prev = NULL;
*/
    ret->dirty = BT_CLEAN;

    ret->pagepos = cache->totsize;

    cache->totsize += cache->pagesize;
    cache->pagecount++;

    return ret;
}




/* @func ajBtreeCacheWriteNodenew *********************************************
**
** Get a pointer to a new disc cache page for writing a node
**
** Clears the page and sets the block number.
**
** @param [w] cache [AjPBtcache] cache
**
** @return [AjPBtpage] disc cache page pointer
** @@
******************************************************************************/

AjPBtpage ajBtreeCacheWriteNodenew(AjPBtcache cache)
{
    AjPBtpage ret = NULL;
    unsigned char *p;

#if AJINDEX_DEBUG
    ajDebug("ajBtreeCacheWriteNodenew pagepos:%Lu\n", cache->totsize);
#endif

    ret = btreeCacheControl(cache,cache->totsize,BT_WRITE);
    p = ret->buf;
    AJCSET0(p, cache->pagesize); /* clear buffer to zeros */
/*
    ret->next = NULL;
    ret->prev = NULL;
*/
    ret->dirty = BT_CLEAN;

    ret->pagepos = cache->totsize;

    SBT_BLOCKNUMBER(p,ret->pagepos);

    cache->totsize += cache->pagesize;
    cache->pagecount++;

    return ret;
}




/* @func ajBtreeCacheWriteOverflownew *****************************************
**
** Get a pointer to a new disc cache page for writing an overflow node
**
** Clears the page and sets the block number and nodetype
**
** @param [w] cache [AjPBtcache] cache
**
** @return [AjPBtpage] disc cache page pointer
** @@
******************************************************************************/

AjPBtpage ajBtreeCacheWriteOverflownew(AjPBtcache cache)
{
    AjPBtpage ret = NULL;
    unsigned char *p;
    ajuint v;

#if AJINDEX_DEBUG
    ajDebug("ajBtreeCacheWriteOverflownew pagepos:%Lu\n", cache->totsize);
#endif

    ret = btreeCacheControl(cache,cache->totsize,BT_WRITE);
    p = ret->buf;
    AJCSET0(p, cache->pagesize); /* clear buffer to zeros */
/*
    ret->next = NULL;
    ret->prev = NULL;
*/
    ret->dirty = BT_CLEAN;

    ret->pagepos = cache->totsize;

    SBT_BLOCKNUMBER(p,ret->pagepos);

    v = BT_OVERFLOW;
    SBT_NODETYPE(p,v);

    cache->totsize += cache->pagesize;
    cache->pagecount++;

    return ret;
}




/* @func ajBtreeCreateRootNode ***********************************************
**
** Create and write an empty root node. Set it as root, write it to
** disc and then lock the page in the disc cache.
** The root node is at block 0L
**
** @param [w] cache [AjPBtcache] cache
** @param [r] rootpage [ajulong] root block
**
** @return [void]
** @@
******************************************************************************/

void ajBtreeCreateRootNode(AjPBtcache cache, ajulong rootpage)
{
    AjPBtpage page = NULL;
    unsigned char *p;
    ajuint nodetype;
    ajulong blocknumber;
    ajuint nkeys;
    ajuint totlen;
    ajulong left;
    ajulong right;
    ajulong overflow;
    ajulong prev;

#if AJINDEX_DEBUG
    ajDebug("ajBtreeCreateRootNode %Lu\n", rootpage); 
#endif

    if(rootpage == cache->totsize)
        page = ajBtreeCacheWriteNodenew(cache);
    else
        page = ajBtreeCacheWrite(cache,rootpage);

    page->pagepos    = rootpage;
    
    p = page->buf;

    nodetype    = BT_ROOT;
    blocknumber = rootpage;
    nkeys       = 0;
    totlen      = 0;
    left        = 0L;
    right       = 0L;
    prev        = 0L;
    overflow    = 0L;

    /* Don't reuse the variables. Endian-ness may be changed */
    SBT_NODETYPE(p,nodetype);
    SBT_BLOCKNUMBER(p,blocknumber);
    SBT_NKEYS(p,nkeys);
    SBT_TOTLEN(p,totlen);
    SBT_LEFT(p,left);
    SBT_RIGHT(p,right);
    SBT_PREV(p,prev);
    SBT_OVERFLOW(p,overflow);

    page->dirty = BT_DIRTY;

    if(btreeDoRootSync)
        ajBtreeCacheRootSync(cache,rootpage);
    page->dirty = BT_LOCK;
    page->lockfor = 1021;

    return;
}




/* @func ajBtreeFindInsert ***********************************************
**
** Find the node that should contain a new key for insertion
**
** @param [u] cache [AjPBtcache] cache
** @param [r] key [const AjPStr] key to search for 
**
** @return [AjPBtpage] leaf node where item should be inserted
** @@
******************************************************************************/

AjPBtpage ajBtreeFindInsert(AjPBtcache cache, const AjPStr key)
{
    AjPBtpage root = NULL;
    AjPBtpage ret  = NULL;

    /* ajDebug("In ajBtreeFindInsert\n"); */

    /* The root node should always be in the cache (BT_LOCKed) */
    root = btreeCacheLocate(cache,0L);

    if(!root)
	ajFatal("The PRI root cache page in %S has been unlocked\n",
                cache->filename);
    
    if(!cache->level)
	return root;
    
    ret = btreeFindINode(cache,root,key);
    
    return ret;
}




/* @funcstatic btreeFindINode *************************************************
**
** Recursive search for insert node
**
** @param [u] cache [AjPBtcache] cache
** @param [u] page [AjPBtpage] page
** @param [r] item [const AjPStr] key to search for 
**
** @return [AjPBtpage] leaf node where item should be inserted
** @@
******************************************************************************/

static AjPBtpage btreeFindINode(AjPBtcache cache, AjPBtpage page,
				 const AjPStr item)
{
    AjPBtpage ret = NULL;
    AjPBtpage pg  = NULL;

    unsigned char *buf = NULL;
    ajuint status       = 0;
    ajuint ival         = 0;

    /* ajDebug("In btreeFindINode\n"); */
    
    ret = page;
    buf = page->buf;
    GBT_NODETYPE(buf,&ival);

    if(ival != BT_LEAF)
    {
	status = ret->dirty;
	ret->dirty = BT_LOCK;	/* Lock in case of lots of overflow pages */
        ret->lockfor = 1031;
	pg = btreePageFromKey(cache,buf,item);
	ret->dirty = status;
	ret = btreeFindINode(cache,pg,item);
    }

    return ret;
}




/* @funcstatic btreeSecFindINode *********************************************
**
** Recursive search for insert node in a secondary tree
**
** @param [u] cache [AjPBtcache] cache
** @param [u] page [AjPBtpage] page
** @param [r] item [const AjPStr] key to search for 
**
** @return [AjPBtpage] leaf node where item should be inserted
** @@
******************************************************************************/

static AjPBtpage btreeSecFindINode(AjPBtcache cache, AjPBtpage page,
                                   const AjPStr item)
{
    AjPBtpage ret = NULL;
    AjPBtpage pg  = NULL;

    unsigned char *buf = NULL;
    ajuint status       = 0;
    ajuint ival         = 0;

    /* ajDebug("In btreeSecFindINode\n"); */
    
    ret = page;
    buf = page->buf;
    GBT_NODETYPE(buf,&ival);

    if(ival != BT_LEAF)
    {
	status = ret->dirty;
	ret->dirty = BT_LOCK;	/* Lock in case of lots of overflow pages */
        ret->lockfor = 1041;
	pg = btreeSecPageFromKey(cache,buf,item);
	ret->dirty = status;
	ret = btreeSecFindINode(cache,pg,item);
    }
    
    return ret;
}




/* @funcstatic btreePageFromKey *******************************************
**
** Return next lower index page given a key
**
** @param [u] cache [AjPBtcache] cache
** @param [u] buf [unsigned char *] page buffer
** @param [r] key [const AjPStr] key to search for 
**
** @return [AjPBtpage] pointer to a page
** @@
******************************************************************************/

static AjPBtpage btreePageFromKey(AjPBtcache cache, unsigned char *buf,
				   const AjPStr key)
{
    ajulong blockno = 0L;
    AjPBtpage page = NULL;
    
    /* ajDebug("In btreePageFromKey\n"); */

    blockno = btreeGetBlockS(cache,buf,key);
    page = ajBtreeCacheRead(cache,blockno);

    return page;
}




/* @funcstatic btreeSecPageFromKey *******************************************
**
** Return next lower index page given a key in a secondary tree
**
** @param [u] cache [AjPBtcache] cache
** @param [u] buf [unsigned char *] page buffer
** @param [r] key [const AjPStr] key to search for 
**
** @return [AjPBtpage] pointer to a page
** @@
******************************************************************************/

static AjPBtpage btreeSecPageFromKey(AjPBtcache cache, unsigned char *buf,
				     const AjPStr key)
{
    ajulong blockno = 0L;
    AjPBtpage page = NULL;
    
    /* ajDebug("In btreePageFromKey\n"); */
    
    blockno = btreeGetBlockS(cache, buf, key);

    page =  ajBtreeCacheRead(cache,blockno);

    return page;
}




/* @func ajBtreeIdNew *********************************************
**
** Constructor for index bucket ID information
**
**
** @return [AjPBtId] Index ID object
** @@
******************************************************************************/

AjPBtId ajBtreeIdNew(void)
{
    AjPBtId Id = NULL;

    /* ajDebug("In ajBtreeIdNew\n"); */

    if(statSaveBtreeIdNext)
    {
        Id = statSaveBtreeId[--statSaveBtreeIdNext];
        MAJSTRASSIGNCLEAR(&Id->id);
    }
    else
    {
        AJNEW0(Id);
        Id->id = ajStrNew();
    }

    Id->dbno = 0;
    Id->dups = 0;
    Id->offset = 0L;
    Id->refoffset = 0L;

    return Id;
}




/* @func ajBtreeIdDel *********************************************************
**
** Destructor for index bucket ID information
**
** @param [w] thys [AjPBtId*] index ID object
**
** @return [void]
** @@
******************************************************************************/

void ajBtreeIdDel(AjPBtId *thys)
{
    ajuint newmax;

    /* ajDebug("In ajBtreeIdDel\n"); */

    if(!statSaveBtreeId)
    {
        statSaveBtreeIdMax = 2048;
        AJCNEW0(statSaveBtreeId, statSaveBtreeIdMax);
        statSaveBtreeIdNext = 0;
    }

    if(!thys || !*thys)
	return;

    if(statSaveBtreeIdNext >= statSaveBtreeIdMax) 
    {
        newmax = statSaveBtreeIdMax + statSaveBtreeIdMax;
        AJCRESIZE0(statSaveBtreeId,statSaveBtreeIdMax,newmax);
        statSaveBtreeIdMax = newmax;
    }
    
    statSaveBtreeId[statSaveBtreeIdNext++] = *thys;
    
    *thys = NULL;

    return;
}




/* @func ajBtreeIdDelVoid *****************************************************
**
** Destructor for index bucket ID information
**
** @param [w] voidarg [void**] Index ID object, passed as void for use by
**                             list and table destructors.
** @return [void]
** @@
******************************************************************************/

void ajBtreeIdDelVoid(void **voidarg)
{
    AjPBtId *thys = (AjPBtId *)voidarg;

    ajBtreeIdDel(thys);

    return;
}




/* @funcstatic btreeIdFree *********************************************
**
** Destructor for index bucket ID information
**
** @param [w] thys [AjPBtId*] index ID object
**
** @return [void]
** @@
******************************************************************************/

static void btreeIdFree(AjPBtId *thys)
{
    AjPBtId Id = NULL;

    /* ajDebug("In ajBtreeIdFree\n"); */

    if(!thys || !*thys)
	return;

    Id = *thys;

    ajStrDel(&Id->id);
    AJFREE(Id);
    *thys = NULL;

    return;
}




/* @funcstatic btreeBucketIdlist ***********************************************
**
** Copies all IDs into a list
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
** @param [u] idlist [AjPList] list to hold IDs
**
** @return [ajulong] Overflow
** @@
******************************************************************************/

static ajulong btreeBucketIdlist(AjPBtcache cache, ajulong pagepos,
                                 AjPList idlist)
{
    AjPBtpage page      = NULL;
    AjPBtpage lpage     = NULL;
    unsigned char *buf  = NULL;
    unsigned char *kptr = NULL;
    AjPBtId id          = NULL;
    
    unsigned char *idptr = NULL;
    
    ajuint  nodetype  = 0;
    ajuint  nentries  = 0;
    ajulong overflow  = 0L;
    ajulong pageoverflow = 0L;
    ajuint  dirtysave = 0;
    
    ajuint  i;
    ajuint  len = 0;
    ajuint idlen;

    /* ajDebug("In btreeBucketIdlist\n"); */
    
    if(!pagepos)
	ajFatal("BucketIdlist: cannot read bucket from root page cache %S",
                cache->filename);

    page  = ajBtreeCacheRead(cache,pagepos);
    lpage = page;
    dirtysave = lpage->dirty;
    lpage->dirty = BT_LOCK;     /* reset on return */
    lpage->lockfor = 1051;

    buf = lpage->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);
    if(nodetype != BT_BUCKET)
	ajFatal("BucketIdlist: NodeType mismatch. "
                "Not bucket (%u) cache %S",
                nodetype, cache->filename);
    
    GBT_BUCKNENTRIES(buf,&nentries);
    if(nentries > cache->nperbucket)
	ajFatal("BucketIdlist: Bucket too full page: %Lu "
                "entries: %u max: %u cache %S",
                pagepos, nentries, cache->nperbucket, cache->filename);
    

    GBT_BUCKOVERFLOW(buf,&overflow);
    pageoverflow = overflow;
    
    kptr  = PBT_BUCKKEYLEN(buf);
    idptr = kptr + (nentries * sizeof(ajuint));

    for(i=0;i<nentries;++i)
    {
	BT_GETAJUINT(kptr,&len);
        idlen = len - BT_DDOFFROFF-1;

	if((idptr-buf+1) + len > cache->pagesize)	/* overflow */
	{
	    /* ajDebug("HybBucketIdlist: Overflow\n"); */
	    page  = ajBtreeCacheRead(cache,overflow);
	    buf = page->buf;
	    GBT_BUCKNODETYPE(buf,&nodetype);
	    if(nodetype != BT_BUCKET)
		ajFatal("BucketIdlist: NodeType mismatch. "
                        "Not bucket (%u) cache %S",
			nodetype, cache->filename);
	    GBT_BUCKOVERFLOW(buf,&overflow);
	    /* overflow bucket ids start at the keylen position */
	    idptr = PBT_BUCKKEYLEN(buf);
	}

	id = ajBtreeIdNew();
        
	/* Fill ID objects */
	ajStrAssignLenC(&id->id,(const char *)idptr,idlen);
	idptr += (idlen + 1);
	BT_GETAJUINT(idptr,&id->dbno);
	idptr += sizeof(ajuint);
	BT_GETAJUINT(idptr,&id->dups);
	idptr += sizeof(ajuint);	
	BT_GETAJULONG(idptr,&id->offset);
	idptr += sizeof(ajulong);
	BT_GETAJULONG(idptr,&id->refoffset);
	idptr += sizeof(ajulong);

	kptr += sizeof(ajuint);
        ajListPushAppend(idlist, id);
    }

    lpage->dirty = dirtysave;
    
    return pageoverflow;
}




/* @funcstatic btreeReadBucket *********************************************
**
** Constructor for index bucket given a disc page number
** Creates one empty key slot for possible addition
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
**
** @return [AjPBucket] bucket
** @@
******************************************************************************/

static AjPBucket btreeReadBucket(AjPBtcache cache, ajulong pagepos)
{
    AjPBucket bucket    = NULL;
    AjPBtpage page      = NULL;
    AjPBtpage lpage     = NULL;
    unsigned char *buf  = NULL;
    unsigned char *kptr = NULL;
    AjPBtId id          = NULL;
    
    unsigned char *idptr = NULL;
    
    ajuint nodetype  = 0;
    ajuint  nentries  = 0;
    ajulong overflow  = 0L;
    ajuint  dirtysave = 0;
    
    ajuint  i;
    ajuint  len = 0;
    ajuint idlen;
    
    /* ajDebug("In btreeReadBucket\n"); */
    
    if(!pagepos)
	ajFatal("BucketRead: cannot read bucket from root page");

    page  = ajBtreeCacheRead(cache,pagepos);
    lpage = page;
    dirtysave = lpage->dirty;
    lpage->dirty = BT_LOCK;
    lpage->lockfor = 1061;

    buf = lpage->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);

    if(nodetype != BT_BUCKET)
	ajFatal("ReadBucket: NodeType mismatch. Not bucket (%u)", nodetype);
    
    GBT_BUCKNENTRIES(buf,&nentries);

    if(nentries > cache->nperbucket)
	ajFatal("ReadBucket: Bucket too full page: %Lu "
                "entries: %u max: %u cache %S",
                pagepos, nentries, cache->nperbucket, cache->filename);
    

    GBT_BUCKOVERFLOW(buf,&overflow);

    bucket = btreeBucketNew(cache->nperbucket);
    bucket->Nentries = nentries;
    
    kptr  = PBT_BUCKKEYLEN(buf);
    idptr = kptr + (nentries * sizeof(ajuint));

    for(i=0;i<nentries;++i)
    {
	BT_GETAJUINT(kptr,&len);
        idlen = len - BT_DDOFFROFF-1;

	if((idptr-buf+1) + len > cache->pagesize)	/* overflow */
	{
#if AJINDEX_DEBUG
	    ajDebug("ReadBucket: Overflow\n");
#endif
	    page  = ajBtreeCacheRead(cache,overflow);
	    buf = page->buf;
	    GBT_BUCKNODETYPE(buf,&nodetype);
	    if(nodetype != BT_BUCKET)
		ajFatal("ReadBucket: NodeType mismatch. Not bucket (%u)",
			nodetype);
	    GBT_BUCKOVERFLOW(buf,&overflow);
	    /* overflow bucket ids start at the keylen position */
	    idptr = PBT_BUCKKEYLEN(buf);
	}

	id = bucket->Ids[i];

	/* Fill ID objects */
	ajStrAssignLenC(&id->id,(const char *)idptr, idlen);
	idptr += (idlen + 1);
	BT_GETAJUINT(idptr,&id->dbno);
	idptr += sizeof(ajuint);
	BT_GETAJUINT(idptr,&id->dups);
	idptr += sizeof(ajuint);	
	BT_GETAJULONG(idptr,&id->offset);
	idptr += sizeof(ajulong);
	BT_GETAJULONG(idptr,&id->refoffset);
	idptr += sizeof(ajulong);

	kptr += sizeof(ajuint);
    }

    lpage->dirty = dirtysave;
    
    return bucket;
}




/* @funcstatic btreeBucketFindDupId ********************************************
**
** Tests for an ID in a bucket.
** If found, returns the ID and its count in the bucket.
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
** @param [r] id [const AjPStr] id to search for
** @param [w] ientry [ajuint*] entry number matched
**
** @return [AjPBtId] ID found
** @@
******************************************************************************/

static AjPBtId btreeBucketFindDupId(AjPBtcache cache, ajulong pagepos,
                                   const AjPStr id, ajuint* ientry)
{
    AjPBtId bid = NULL;
    AjPBtpage page      = NULL;
    AjPBtpage lpage     = NULL;
    unsigned char *buf  = NULL;
    unsigned char *lbuf  = NULL;
    unsigned char *kptr = NULL;
    
    unsigned char *idptr = NULL;
    
    ajuint  nodetype  = 0;
    ajuint  nentries  = 0;
    ajulong overflow  = 0L;
    ajuint  dirtysave = 0;

    ajuint  i;
    ajuint  len = 0;
    ajuint  idlen;
    
    /* ajDebug("In btreeBucketFindId\n"); */
    
    if(!pagepos)
	ajFatal("BucketFindDupId: cannot read bucket from root page cache %S",
                cache->filename);

    page  = ajBtreeCacheRead(cache,pagepos);
    lpage = page;
    dirtysave = lpage->dirty;
    lpage->dirty = BT_LOCK;     /* reset on return */
    lpage->lockfor = 1071;

    buf = lpage->buf;
    lbuf = buf;

    GBT_BUCKNODETYPE(lbuf,&nodetype);
    if(nodetype != BT_BUCKET)
	ajFatal("BucketFindDupId: NodeType mismatch. "
                "Not bucket (%u) cache %S",
                nodetype, cache->filename);
    
    GBT_BUCKNENTRIES(lbuf,&nentries);
    if(nentries > cache->nperbucket)
	ajFatal("BucketFindDupId: Bucket too full page: %Lu "
                "entries: %u max: %u cache %S",
                pagepos, nentries, cache->nperbucket, cache->filename);
    

    GBT_BUCKOVERFLOW(lbuf,&overflow);
    
    kptr  = PBT_BUCKKEYLEN(buf);
    idptr = kptr + (nentries * sizeof(ajuint));

    for(i=0;i<nentries;++i)
    {
	BT_GETAJUINT(kptr,&len);
	if((idptr-buf+1) + len > cache->pagesize)	/* overflow */
	{
	    /* ajDebug("BucketFindId: Overflow\n"); */
	    page  = ajBtreeCacheRead(cache,overflow);
	    buf = page->buf;
	    GBT_BUCKNODETYPE(buf,&nodetype);
	    if(nodetype != BT_BUCKET)
		ajFatal("BucketFindId: NodeType mismatch. "
                        "Not bucket (%u) cache %S",
			nodetype, cache->filename);
	    GBT_BUCKOVERFLOW(buf,&overflow);
	    /* overflow bucket ids start at the keylen position */
	    idptr = PBT_BUCKKEYLEN(buf);
	}

	if(!MAJSTRCMPC(id,(const char*)idptr))
        {
            bid = ajBtreeIdNew();
            idlen = len - BT_DDOFFROFF - 1;
            ajStrAssignLenC(&bid->id,(const char *)idptr, idlen);
            idptr += (idlen + 1);
            BT_GETAJUINT(idptr,&bid->dbno);
            idptr += sizeof(ajuint);
            BT_GETAJUINT(idptr,&bid->dups);
            idptr += sizeof(ajuint);	
            BT_GETAJULONG(idptr,&bid->offset);
            idptr += sizeof(ajulong);
            BT_GETAJULONG(idptr,&bid->refoffset);
            idptr += sizeof(ajulong);
            *ientry = i;
            lpage->dirty = dirtysave;
            return bid;
        }
/* Step through ID object data */
	idptr += len;
        kptr += sizeof(ajuint);
    }

    lpage->dirty = dirtysave;

    *ientry = nentries;
    return NULL;
}




/* @funcstatic btreeWriteBucketId *********************************************
**
** Tests for an ID in a bucket.
** If found, returns the ID and its count in the bucket.
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
** @param [r] btid [const AjPBtId] Bucket id to update
** @param [r] ientry [ajuint] entry number for this ID
**
** @return [void]
** @@
******************************************************************************/

static void btreeWriteBucketId(AjPBtcache cache, ajulong pagepos,
                                const AjPBtId btid, ajuint ientry)
{
    AjPBtpage page      = NULL;
    AjPBtpage lpage     = NULL;
    unsigned char *buf  = NULL;
    unsigned char *lbuf = NULL;
    unsigned char *kptr = NULL;
    
    unsigned char *idptr = NULL;
    
    ajuint  nodetype  = 0;
    ajuint  nentries  = 0;
    ajulong overflow  = 0L;

    ajuint  i;
    ajuint  len = 0;
    ajuint  idlen;
    ajuint  v;
    ajulong lv;
    
    /* ajDebug("In btreeWriteBucketId\n"); */
    
    if(!pagepos)
	ajFatal("WriteBucketId: cannot read bucket from root page cache %S",
                cache->filename);

    page  = ajBtreeCacheRead(cache,pagepos);
    lpage = page;
    lpage->dirty = BT_LOCK;     /* reset on return */
    lpage->lockfor = 1081;

    buf = lpage->buf;
    lbuf= buf;

    GBT_BUCKNODETYPE(lbuf,&nodetype);
    if(nodetype != BT_BUCKET)
	ajFatal("WriteBucketId: NodeType mismatch. "
                "Not bucket (%u) cache %S",
                nodetype, cache->filename);
    
    GBT_BUCKNENTRIES(lbuf,&nentries);
    if(nentries > cache->nperbucket)
	ajFatal("WriteBucketId: Bucket too full page: %Lu "
                "entries: %u max: %u cache %S",
                pagepos, nentries, cache->nperbucket, cache->filename);
    

    GBT_BUCKOVERFLOW(lbuf,&overflow);
    
    kptr  = PBT_BUCKKEYLEN(lbuf);
    idptr = kptr + (nentries * sizeof(ajuint));

    for(i=0;i<ientry;++i)
    {
	BT_GETAJUINT(kptr,&len);
	if((idptr-buf+1) + len > cache->pagesize)	/* overflow */
	{
	    /* ajDebug("BucketFindId: Overflow\n"); */
	    page  = ajBtreeCacheRead(cache,overflow);
	    buf = page->buf;
	    GBT_BUCKNODETYPE(buf,&nodetype);
	    if(nodetype != BT_BUCKET)
		ajFatal("WriteBucketId: NodeType mismatch. "
                        "Not bucket (%u) cache %S",
			nodetype, cache->filename);
	    GBT_BUCKOVERFLOW(buf,&overflow);
	    /* overflow bucket ids start at the keylen position */
	    idptr = PBT_BUCKKEYLEN(buf);
	}

/* Step through ID object data */
	idptr += len;
        kptr += sizeof(ajuint);
    }

    BT_GETAJUINT(kptr,&len);
    idlen = len - BT_DDOFFROFF - 1;
    idptr += (idlen + 1);

    v = btid->dbno;
    BT_SETAJUINT(idptr,v);
    idptr += sizeof(ajuint);

    v = btid->dups;
    BT_SETAJUINT(idptr,v);
    idptr += sizeof(ajuint);	

    lv = btid->offset;
    BT_SETAJULONG(idptr,lv);
    idptr += sizeof(ajulong);

    lv = btid->refoffset;
    BT_SETAJULONG(idptr,lv);
    idptr += sizeof(ajulong);

    lpage->dirty = BT_DIRTY;

    return;
}




/* @funcstatic btreeWriteBucket *******************************************
**
** Write index bucket object to the cache given a disc page number
**
** @param [u] cache [AjPBtcache] cache
** @param [r] bucket [const AjPBucket] bucket
** @param [r] pagepos [ajulong] page number
**
** @return [void]
** @@
******************************************************************************/

static void btreeWriteBucket(AjPBtcache cache, const AjPBucket bucket,
			     ajulong pagepos)
{
    AjPBtpage page      = NULL;
    AjPBtpage lpage     = NULL;
    unsigned char *buf  = NULL;
    unsigned char *lbuf = NULL;
    ajuint  uv  = 0;
    ajuint   v   = 0;
    ajuint  i   = 0;
    ajuint   len = 0;
    ajulong lv   = 0L;
    AjPBtId id  = NULL;

    ajuint nentries = 0;
    ajulong overflow = 0L;
    unsigned char *keyptr = NULL;
    unsigned char *lptr   = NULL;

    /* ajuint unused = 0;*/

    if(pagepos == cache->totsize)	/* Create a new page */
    {
	page = ajBtreeCacheWriteBucketnew(cache);
	buf = page->buf;
    }
    else
    {
	page = ajBtreeCacheRead(cache,pagepos);
	buf = page->buf;
	GBT_BUCKOVERFLOW(buf,&overflow);
    }

    v = BT_BUCKET;
    SBT_BUCKNODETYPE(buf,v);

    lbuf = buf;
    page->dirty = BT_LOCK;
    page->lockfor = 1091;
    lpage = page;

    nentries = bucket->Nentries;
    v = nentries;
    SBT_BUCKNENTRIES(buf,v);

    /* Write out key lengths */
    keyptr = PBT_BUCKKEYLEN(lbuf);

    for(i=0;i<nentries;++i)
    {
	if((ajuint)((keyptr-lbuf+1)+sizeof(ajuint)) > cache->pagesize)
	    ajFatal("BucketWrite: Bucket cannot hold more than %u keys",
		    i-1);

	id = bucket->Ids[i];
	/* Need to alter this if bucket ID structure changes */
	len = BT_BUCKIDLEN(id->id);
        v = len;
	BT_SETAJUINT(keyptr,v);
	keyptr += sizeof(ajuint);
    }


    /* Write out IDs using overflow if necessary */
    lptr = keyptr;

    for(i=0;i<nentries;++i)
    {
	id = bucket->Ids[i];
	len = BT_BUCKIDLEN(id->id);

        /* overflow */        
	if((lptr-buf+1) + (len+1+BT_DDOFFROFF) > (ajuint) cache->pagesize)
	{
	    if(!overflow)		/* No overflow buckets yet */
	    {
                page = ajBtreeCacheWriteBucketnew(cache);
		buf = page->buf;
		v = BT_BUCKET;
		SBT_BUCKNODETYPE(buf,v);
	    }
	    else
	    {
		page = ajBtreeCacheRead(cache,overflow);
		buf  = page->buf;
		GBT_BUCKOVERFLOW(buf,&overflow);
	    }

	    page->dirty = BT_DIRTY;

	    lptr = PBT_BUCKKEYLEN(buf);	    
	}
	
	sprintf((char *)lptr,"%s",ajStrGetPtr(id->id));
	lptr += (ajStrGetLen(id->id) + 1);
        uv = id->dbno;
	BT_SETAJUINT(lptr,uv);
	lptr += sizeof(ajuint);
        uv = id->dups;
	BT_SETAJUINT(lptr,uv);
	lptr += sizeof(ajuint);
        lv = id->offset;
	BT_SETAJULONG(lptr,lv);
	lptr += sizeof(ajulong);
        lv = id->refoffset;
	BT_SETAJULONG(lptr,lv);
	lptr += sizeof(ajulong);
    }

    lv = 0L;
    SBT_BUCKOVERFLOW(buf,lv);

    /*
    unused = cache->pagesize - (lptr - buf);
    if(unused > cache->pagesize)
        ajFatal("unused %u for pagesize %u", unused, cache->pagesize);
    if(unused > 0)
        memset(lptr, 0, unused);
    */

    lpage->dirty = BT_DIRTY;

    return;
}




/* @funcstatic btreeWriteBucketEmpty *******************************************
**
** Write empty index bucket object to the cache given a disc page number
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
**
** @return [void]
** @@
******************************************************************************/

static void btreeWriteBucketEmpty(AjPBtcache cache, ajulong pagepos)
{
    AjPBtpage page      = NULL;
    unsigned char *buf  = NULL;
    ajuint   v   = 0;
    ajulong lv   = 0L;

    ajuint  nentries = 0;
    ajulong overflow = 0L;

    /* ajuint unused = 0; */

    /* ajDebug("In btreeWriteBucketEmpty\n"); */

    if(pagepos == cache->totsize)	/* Create a new page */
    {
	page = ajBtreeCacheWriteBucketnew(cache);
	buf = page->buf;
	overflow = 0L;
	lv = overflow;
	SBT_BUCKOVERFLOW(buf,lv);
    }
    else
    {
	page = ajBtreeCacheRead(cache,pagepos);
	buf = page->buf;
	GBT_BUCKOVERFLOW(buf,&overflow);
    }
    
    v = BT_BUCKET;
    SBT_BUCKNODETYPE(buf,v);

    nentries = 0;
    v = nentries;
    SBT_BUCKNENTRIES(buf,v);

    lv = 0L;
    SBT_BUCKOVERFLOW(buf,lv);

    /*
    unused = cache->pagesize - (lptr - buf);
    if(unused > cache->pagesize)
        ajFatal("unused %u for pagesize %u", unused, cache->pagesize);
    if(unused > 0)
        memset(lptr, 0, unused);
    */

    page->dirty = BT_DIRTY;

    return;
}




/* @funcstatic btreeBucketDel *********************************************
**
** Delete a bucket object
**
** @param [w] thys [AjPBucket*] bucket
**
** @return [void] bucket
** @@
******************************************************************************/

static void btreeBucketDel(AjPBucket *thys)
{
    AjPBucket pthis = NULL;
    ajuint newmax;

    /* ajDebug("In btreeBucketDel\n"); */

    if(!thys || !*thys)
	return;

 
    pthis = *thys;

    if(!statSaveBucket)
    {
        statSaveBucketMax=2048;
        statSaveBucketNext=0;
        AJCNEW0(statSaveBucket,statSaveBucketMax);
    }
    
    if(!statSaveBucketEmpty)
    {
        statSaveBucketEmptyMax=2048;
        statSaveBucketEmptyNext=0;
        AJCNEW0(statSaveBucketEmpty,statSaveBucketEmptyMax);
    }
    
    if(pthis->Maxentries)
    {
        if(statSaveBucketNext >= statSaveBucketMax)
        {
            newmax = statSaveBucketMax + statSaveBucketMax;
            AJCRESIZE0(statSaveBucket,statSaveBucketMax,newmax);
            statSaveBucketMax = newmax;
        }
        
        statSaveBucket[statSaveBucketNext++] = pthis;
    }
    else
    {
        if(statSaveBucketEmptyNext >= statSaveBucketEmptyMax)
        {
            newmax = statSaveBucketEmptyMax + statSaveBucketEmptyMax;
            AJCRESIZE0(statSaveBucketEmpty,statSaveBucketEmptyMax,newmax);
            statSaveBucketEmptyMax = newmax;
        }
        statSaveBucketEmpty[statSaveBucketEmptyNext++] = pthis;
    }
    
    *thys = NULL;

    return;
}




/* @funcstatic btreeBucketFree *********************************************
**
** Delete a keyword primary bucket object
**
** @param [w] thys [AjPBucket*] bucket
**
** @return [void]
** @@
******************************************************************************/

static void btreeBucketFree(AjPBucket *thys)
{
    AjPBucket pthis = NULL;
    ajuint n;
    ajuint i;
    
    /*ajDebug("In btreeBucketFree\n"); */

    if(!thys || !*thys)
	return;

    pthis = *thys;

    n = pthis->Maxentries;

    for(i=0;i<n;++i)
	btreeIdFree(&pthis->Ids[i]);
    
    AJFREE(pthis->keylen);
    AJFREE(pthis->Ids);
    
    AJFREE(pthis);

    *thys = NULL;

    return;
}




/* @funcstatic btreeAddToBucket *******************************************
**
** Add an ID to a bucket
** Only called if there is room in the bucket
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number of bucket
** @param [r] id [const AjPBtId] ID info
**
** @return [void]
** @@
******************************************************************************/

static void btreeAddToBucket(AjPBtcache cache, ajulong pagepos,
			     const AjPBtId id)
{
    unsigned char *buf  = NULL;
    unsigned char *kptr = NULL;
    unsigned char *src  = NULL;
    unsigned char *dest = NULL;

    unsigned char *lastptr = NULL;
    unsigned char *endptr  = NULL;
    
    ajuint nentries = 0;
    ajuint nodetype = 0;
    ajuint idlen    = 0;
    
    ajuint sum = 0;
    ajuint len = 0;
    ajuint i;
    ajuint v;
    ajuint uv;
    ajulong lv;
    
    AjPBtpage page = NULL;
    static ajuint calls = 0;
    static ajuint overflowcalls=0;
   
    calls++;

    page = ajBtreeCacheRead(cache,pagepos);
    buf  = page->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);
    if(nodetype != BT_BUCKET)
        ajFatal("Wrong nodetype in AddToBucket cache %S", cache->filename);

    GBT_BUCKNENTRIES(buf,&nentries);
    if(nentries == cache->snperbucket)
        ajFatal("Bucket too full in AddToBucket page: %Lu "
                "entries: %u max: %u cache %S",
                pagepos, nentries, cache->snperbucket, cache->filename);    

    kptr = PBT_BUCKKEYLEN(buf);
    src  = kptr + (nentries * sizeof(ajuint));

    sum = 0;
    for(i=0;i<nentries;++i)
    {
        BT_GETAJUINT(kptr,&len);
        sum += len;
        kptr += sizeof(ajuint);
    }
    /*sum += nentries;*/
    
    endptr  = src + sum - 1;
    idlen   = MAJSTRGETLEN(id->id);
    lastptr = endptr + sizeof(ajuint) + BT_DDOFFROFF + idlen + 1;
    if((ajuint) (lastptr - buf) >= cache->pagesize)
    {
        overflowcalls++;
        ajWarn("\nOverflow in AddToBucket nentries:%u fails %u/%u cache %S",
               nentries, overflowcalls,calls, cache->filename);
        btreeAddToBucketFull(cache,pagepos,id);
        return;
    }
    
    dest = src + sizeof(ajuint);
    memmove((void *)dest, (void *)src, sum);

    v = BT_BUCKIDLEN(id->id);
    BT_SETAJUINT(src,v);

    endptr += sizeof(ajuint) + 1;
    strcpy((char *)endptr,MAJSTRGETPTR(id->id));
    endptr += (MAJSTRGETLEN(id->id) + 1);
    uv = id->dbno;
    BT_SETAJUINT(endptr,uv);
    endptr += sizeof(ajuint);
    uv = id->dups;
    BT_SETAJUINT(endptr,uv);
    endptr += sizeof(ajuint);
    lv = id->offset;
    BT_SETAJULONG(endptr,lv);
    endptr += sizeof(ajulong);
    lv = id->refoffset;
    BT_SETAJULONG(endptr,lv);
    endptr += sizeof(ajulong);

    v = nentries + 1;
    SBT_BUCKNENTRIES(buf,v);

    page->dirty = BT_DIRTY;

    return;
}




/* @funcstatic btreeAddToBucketFull *******************************************
**
** Add an ID to a bucket
** Only called if there is room in the bucket
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number of bucket
** @param [r] id [const AjPBtId] ID info
**
** @return [void]
** @@
******************************************************************************/

static void btreeAddToBucketFull(AjPBtcache cache, ajulong pagepos,
                                 const AjPBtId id)
{
    AjPBucket bucket = NULL;
    AjPBtId   destid = NULL;
    
    ajuint nentries;
    
    /* ajDebug("In btreeAddToBucket\n"); */

    bucket   = btreeReadBucket(cache,pagepos);
    nentries = bucket->Nentries;


    /* Reading a bucket always gives one extra ID position */
    destid = bucket->Ids[nentries];

    ajStrAssignS(&destid->id,id->id);
    destid->dbno      = id->dbno;
    destid->offset    = id->offset;
    destid->refoffset = id->refoffset;
    destid->dups      = id->dups;
    
    ++bucket->Nentries;

    btreeWriteBucket(cache,bucket,pagepos);

    btreeBucketDel(&bucket);
    
    return;
}




/* @funcstatic btreeNumInBucket *******************************************
**
** Return number of entries in a bucket
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
**
** @return [ajuint] Number of entries in bucket
** @@
******************************************************************************/

static ajuint btreeNumInBucket(AjPBtcache cache, ajulong pagepos)
{
    AjPBtpage page     = NULL;
    unsigned char *buf = NULL;
    ajuint nodetype    = 0;
    ajuint nentries    = 0;
    
    /* ajDebug("In btreeNumInBucket\n"); */
    
    if(!pagepos)
	ajFatal("NumInBucket: Attempt to read bucket from root page\n");

    page  = ajBtreeCacheRead(cache,pagepos);

    buf = page->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);

    if(nodetype != BT_BUCKET)
	ajFatal("ReadBucket: NodeType mismatch. Not bucket (%u)", nodetype);
    
    GBT_BUCKNENTRIES(buf,&nentries);

    return nentries;
}




/* @funcstatic btreeReorderBuckets ********************************************
**
** Re-order leaf buckets
** Must only be called if one of the buckets is full
**
** @param [u] cache [AjPBtcache] cache
** @param [u] leaf [AjPBtpage] leaf page
**
** @return [AjBool] true if reorder was successful i.e. leaf not full
** @@
******************************************************************************/

static AjBool btreeReorderBuckets(AjPBtcache cache, AjPBtpage leaf)
{
    ajuint nkeys = 0;
    unsigned char *lbuf = NULL;

    AjPBtMem arrays1 = NULL;
    AjPBtMem arrays2 = NULL;
    ajulong *ptrs        = NULL;
    AjPStr *newkeys     = NULL;
    ajulong *newptrs     = NULL;
    ajulong *overflows   = NULL;
    
    ajuint i = 0;
    
    ajuint order;
    ajuint totalkeys     = 0;
    ajuint maxnperbucket = 0;
    ajuint count         = 0;
    ajuint totkeylen     = 0;
    ajuint keylimit      = 0;
    ajuint bucketlimit   = 0;
    ajuint nodetype      = 0;
    
    AjPList idlist    = NULL;
    ajuint  dirtysave = 0;
    AjPBtId bid       = NULL;
    AjPBucket cbucket = NULL;
    AjPBtId cid       = NULL;
    ajuint iold = 0;
    
#if AJINDEX_DEBUG
    ajDebug("btreeReorderBuckets %Lu\n", leaf->pagepos);
#endif
    ++statCallReorderBuckets;

    /* ajDebug("In btreeReorderBuckets\n"); */

    dirtysave = leaf->dirty;

    leaf->dirty = BT_LOCK;
    leaf->lockfor = 1101;
    lbuf = leaf->buf;

    GBT_NODETYPE(lbuf,&nodetype);

    order = cache->order;    

    /* Read keys/ptrs */
    arrays1 = btreeAllocPriArray(cache);
    ptrs = arrays1->parray;

    ptrs = arrays1->parray;

    btreeGetPointers(cache,lbuf,&ptrs);

    GBT_NKEYS(lbuf,&nkeys);


    if(!nkeys)
	ajFatal("ReorderBuckets: Attempt to reorder empty leaf");

    keylimit = nkeys + 1;
    for(i=0;i<nkeys;++i)
	totalkeys += btreeNumInBucket(cache,ptrs[i]);

    totalkeys += btreeNumInBucket(cache,ptrs[i]);

    btreeBucketCalc(totalkeys, keylimit, cache->nperbucket,
                    &bucketlimit, &maxnperbucket);

    if(bucketlimit >= order)
    {
        btreeDeallocPriArray(cache, arrays1);
	
	leaf->dirty = dirtysave;
	return ajFalse;
    }
    
    arrays2 = btreeAllocPriArray(cache);
    newkeys = arrays2->karray;
    newptrs = arrays2->parray;
    overflows = arrays2->overflows;
    
    /* Read IDs from all buckets and push to list and sort (increasing id) */
    idlist  = ajListNew();
    
    for(i=0;i<keylimit;++i)
	overflows[i] = btreeBucketIdlist(cache,ptrs[i],idlist);

    ajListSort(idlist,btreeIdCompare);


    cbucket = btreeBucketNew(maxnperbucket);
    iold=0;
    for(i=0;i<bucketlimit;++i)
    {
	cbucket->Overflow = overflows[i];
	cbucket->Nentries = 0;

	count = 0;

	while(count!=maxnperbucket)
	{
	    ajListPop(idlist,(void **)&bid);
	    
	    cid = cbucket->Ids[count];
	    ajStrAssignS(&cid->id,bid->id);
	    cid->dbno = bid->dbno;
	    cid->dups = bid->dups;
	    cid->offset = bid->offset;
	    cid->refoffset = bid->refoffset;
	    
	    cbucket->keylen[count] = BT_BUCKIDLEN(bid->id);
	    ++cbucket->Nentries;
	    ++count;
	    ajBtreeIdDel(&bid);
	}


	ajListPeek(idlist,(void **)&bid);
	ajStrAssignS(&newkeys[i],bid->id);

	totkeylen += ajStrGetLen(bid->id);

	if((iold < order) && ptrs[iold])
            newptrs[i] = ptrs[iold++];
        else
	    newptrs[i] = cache->totsize;
	btreeWriteBucket(cache,cbucket,newptrs[i]);
    }


    /* Deal with greater-than bucket */

    cbucket->Overflow = overflows[i];
    cbucket->Nentries = 0;

    count = 0;

    while(ajListPop(idlist,(void **)&bid))
    {
	cid = cbucket->Ids[count];
	ajStrAssignS(&cid->id,bid->id);
	cid->dbno = bid->dbno;
	cid->dups = bid->dups;
	cid->offset = bid->offset;
	cid->refoffset = bid->refoffset;
	
	++cbucket->Nentries;
	++count;
	ajBtreeIdDel(&bid);
    }
    
    if((iold < order) && ptrs[iold])
        newptrs[i] = ptrs[iold++];
    else
        newptrs[i] = cache->totsize;
     btreeWriteBucket(cache,cbucket,newptrs[i]);

    btreeBucketDel(&cbucket);

#if AJINDEX_DEBUG
    if(iold < keylimit)
        ajDebug("btreeReorderBuckets '%S' %u -> %u",
                cache->filename, keylimit, iold);
#endif

    for(i = iold+1; i <= keylimit; i++)
    {
        btreePageSetfree(cache, ptrs[i]);
    }

    /* Now write out a modified leaf with new keys/ptrs */

    nkeys = bucketlimit;
    btreeWriteNode(cache,leaf,newkeys,newptrs,nkeys);

    leaf->dirty = BT_DIRTY;

    if(nodetype == BT_ROOT)
    {
	leaf->dirty = BT_LOCK;
        leaf->lockfor = 1102;
    }

    btreeDeallocPriArray(cache, arrays1);
    btreeDeallocPriArray(cache, arrays2);

    btreeBucketDel(&cbucket);
    ajListFree(&idlist);

    return ajTrue;
}




/* @funcstatic btreeNodeIsFull ************************************************
**
** Tests whether a node is full of keys
**
** @param [r] cache [const AjPBtcache] cache
** @param [u] page [AjPBtpage] original page
**
** @return [AjBool] true if full
** @@
******************************************************************************/

static AjBool btreeNodeIsFull(const AjPBtcache cache, AjPBtpage page)
{
    unsigned char *buf = NULL;
    ajuint nkeys = 0;

    /* ajDebug("In btreeNodeIsFull\n"); */

    buf = page->buf;
    GBT_NKEYS(buf,&nkeys);

    if(nkeys == cache->order - 1)
	return ajTrue;

    return ajFalse;
}





/* @func ajBtreeDumpKeywords **************************************************
**
** Read the leaves of a secondary keywords tree
**
** @param [u] cache [AjPBtcache] cache
** @param [r] dmin [ajuint] minimum number of times the key should appear
** @param [r] dmax [ajuint] maximum number of times the key should appear
** @param [u] outf [AjPFile] output file
**
** @return [void]
** @@
******************************************************************************/

void ajBtreeDumpKeywords(AjPBtcache cache,
                         ajuint dmin, ajuint dmax, AjPFile outf)
{
    ajulong *parray;
    AjPBtpage page;
    unsigned char *buf;
    ajuint nodetype;
    ajuint i;
    ajuint j;
    ajuint kdups;
    
    AjPPriBucket bucket;
    ajuint nkeys;
    ajulong right;
    AjPBtMem array = NULL;
    AjPList tlist  = NULL;
    ajuint dirtysave = 0;
    ajulong totkeys = 0L;
    ajulong totdups = 0L;

    if(!cache->secondary)
    {
        ajBtreeDumpHybKeys(cache, dmin, dmax, outf);
        return;
    }

    array = btreeAllocPriArray(cache);
    parray = array->parray;

    page = ajBtreeCacheRead(cache, 0);
    buf = page->buf;
    dirtysave  = page->dirty;
    page->dirty = BT_LOCK;
    page->lockfor = 1111;

    btreeGetPointers(cache,buf,&parray);
    GBT_NODETYPE(buf,&nodetype);

    while(nodetype != BT_LEAF && cache->level != 0)
    {
        page->dirty = dirtysave;
	page = ajBtreeCacheRead(cache,parray[0]);
        dirtysave  =page->dirty;
        page->dirty = BT_LOCK;
        page->lockfor = 1112;
	buf = page->buf;
	btreeGetPointers(cache,buf,&parray);
	GBT_NODETYPE(buf,&nodetype);
    }

    do
    {
	GBT_NKEYS(buf,&nkeys);
	for(i=0;i<=nkeys;++i)
	{
	    bucket = btreeReadPriBucket(cache,parray[i]);
	    for(j=0;j<bucket->Nentries;++j)
	    {
                totkeys++;

		tlist = ajBtreeSecLeafList(cache, bucket->codes[j]->treeblock);
                kdups = ajListGetLength(tlist);
                totdups += kdups;

		ajListstrFreeData(&tlist);

                if(kdups < dmin)
                    continue;
                if(dmax && kdups > dmax)
                    continue;
                ajFmtPrintF(outf,"%10d %S\n",
                            kdups, bucket->codes[j]->keyword);
	    }
	    btreePriBucketDel(&bucket);
	}

	right = 0L;
	if(cache->level)
	{
	    GBT_RIGHT(buf,&right);
	    if(right)
	    {
                page->dirty = dirtysave;
		page = ajBtreeCacheRead(cache,right);
                dirtysave  =page->dirty;
                page->dirty = BT_LOCK;
                page->lockfor = 1113;
		buf = page->buf;
		btreeGetPointers(cache,buf,&parray);
	    }
	}
    } while(right);

    btreeDeallocPriArray(cache,array);
    
    ajFmtPrintF(outf,"\nTotal keywords: %Lu IDs: %Lu\n", totkeys, totdups);
    return;
}




/* @funcstatic btreeNodeIsFullSec *****************************************
**
** Tests whether a secondary node is full of keys
**
** @param [r] cache [const AjPBtcache] cache
** @param [u] page [AjPBtpage] original page
**
** @return [AjBool] true if full
** @@
******************************************************************************/

static AjBool btreeNodeIsFullSec(const AjPBtcache cache, AjPBtpage page)
{
    unsigned char *buf = NULL;
    ajuint nkeys = 0;

    /* ajDebug("In btreeNodeIsFull\n"); */

    buf = page->buf;
    GBT_NKEYS(buf,&nkeys);

    if(nkeys == cache->sorder - 1)
	return ajTrue;

    return ajFalse;
}




/* @funcstatic btreeInsertNonFull *****************************************
**
** Insert a key into a non-full node
**
** @param [u] cache [AjPBtcache] cache
** @param [u] page [AjPBtpage] original page
** @param [r] key [const AjPStr] key to insert
** @param [r] less [ajulong] less-than pointer
** @param [r] greater [ajulong] greater-than pointer
**
** @return [void]
** @@
******************************************************************************/

static void btreeInsertNonFull(AjPBtcache cache, AjPBtpage page,
			       const AjPStr key,
                               ajulong less, ajulong greater)
{
    unsigned char *buf = NULL;

    AjPStr *karray     = NULL;
    ajulong *parray     = NULL;
    AjPBtMem arrays1 = NULL;

    ajuint nkeys  = 0;
    ajuint ipos   = 0;
    ajuint i;
    ajint  ii;
    ajuint count  = 0;

    ajulong lv = 0L;

    AjPBtpage ppage = NULL;
    ajulong pagepos   = 0L;

    ajuint nodetype = 0;
    
    /* ajDebug("In btreeInsertNonFull\n"); */

    arrays1 = btreeAllocPriArray(cache);
    karray = arrays1->karray;
    parray = arrays1->parray;
 
    buf = page->buf;
    GBT_NKEYS(buf,&nkeys);
    GBT_NODETYPE(buf,&nodetype);
    
    btreeGetKeys(cache,buf,&karray,&parray);

    i = 0;

    while(i!=nkeys && MAJSTRCMPS(key,karray[i]) >= 0)
	++i;

    ipos = i;

    count = nkeys - ipos;
    

    if(ipos == nkeys)
    {
	ajStrAssignS(&karray[ipos],key);
	parray[ipos+1] = greater;
	parray[ipos]   = less;
    }
    else
    {
	parray[nkeys+1] = parray[nkeys];

	for(ii=nkeys-1; count>0; --count, --ii)
	{
	    ajStrAssignS(&karray[ii+1],karray[ii]);
	    parray[ii+1] = parray[ii];
	}

	ajStrAssignS(&karray[ipos],key);
	parray[ipos] = less;
	parray[ipos+1] = greater;
    }

    ++nkeys;
    btreeWriteNode(cache,page,karray,parray,nkeys);

    if(nodetype == BT_ROOT)
    {
	page->dirty = BT_LOCK;
        page->lockfor = 1121;
    }
    
    pagepos = page->pagepos;
    ppage = ajBtreeCacheRead(cache,less);
    lv = pagepos;
    SBT_PREV(ppage->buf,lv);
    ppage->dirty = BT_DIRTY;

    ppage = ajBtreeCacheRead(cache,greater);
    lv = pagepos;
    SBT_PREV(ppage->buf,lv);
    ppage->dirty = BT_DIRTY;
    
    btreeDeallocPriArray(cache,arrays1);

    if(nodetype != BT_ROOT)
	btreeKeyShift(cache,page);

    return;
}




/* @funcstatic btreeInsertKey *****************************************
**
** Insert a key into a potentially full node
**
** @param [u] cache [AjPBtcache] cache
** @param [u] page [AjPBtpage] original page
** @param [r] key [const AjPStr] key to insert
** @param [r] less [ajulong] less-than pointer
** @param [r] greater [ajulong] greater-than pointer
**
** @return [void]
** @@
******************************************************************************/

static void btreeInsertKey(AjPBtcache cache, AjPBtpage page,
			   const AjPStr key, ajulong less, ajulong greater)
{
    unsigned char *lbuf = NULL;
    unsigned char *rbuf = NULL;
    unsigned char *tbuf = NULL;

    AjPStr *karray      = NULL;
    ajulong *parray      = NULL;
    AjPStr *tkarray     = NULL;
    ajulong *tparray     = NULL;
    AjPBtMem savekeyarrays = NULL;
    AjPBtMem arrays2 = NULL;

    ajuint nkeys    = 0;
    ajuint order    = 0;
    ajuint keypos   = 0;
    ajuint rkeyno   = 0;
    
    ajuint i = 0;
    ajuint n = 0;
    
    ajuint nodetype  = 0;
    AjPBtpage ipage = NULL;
    AjPBtpage lpage = NULL;
    AjPBtpage rpage = NULL;
    AjPBtpage tpage = NULL;

    ajulong blockno  = 0L;
    ajulong rblockno = 0L;
    ajulong lblockno = 0L;
    ajulong ibn      = 0L;
    

    AjPStr mediankey  = NULL;
    ajulong medianless = 0L;
    ajulong mediangtr  = 0L;
    ajulong overflow   = 0L;
    ajulong prev       = 0L;
    ajuint totlen     = 0;
    
    ajulong lv = 0L;
    ajuint  v  = 0;
    
    if(!btreeNodeIsFull(cache,page))
    {
	btreeInsertNonFull(cache,page,key,less,greater);
	return;
    }
    
    order = cache->order;
    lbuf = page->buf;
    GBT_NODETYPE(lbuf,&nodetype);
    page->dirty = BT_LOCK;
    page->lockfor = 1131;

    if(nodetype == BT_ROOT)
    {
	btreeSplitRoot(cache);

	if(page->pagepos)
	    page->dirty = BT_DIRTY;
        
        blockno = btreeGetBlockFirstS(cache,lbuf,key);

	ipage = ajBtreeCacheRead(cache,blockno);
	btreeInsertNonFull(cache,ipage,key,less,greater);

	return;
    }


    savekeyarrays = btreeAllocPriArray(cache);
    arrays2 = btreeAllocPriArray(cache);
    karray = savekeyarrays->karray;
    parray = savekeyarrays->parray;
    tkarray = arrays2->karray;
    tparray = arrays2->parray;
    
    lpage = page;
    lbuf = lpage->buf;
    
    btreeGetKeys(cache,lbuf,&karray,&parray);

    GBT_BLOCKNUMBER(lbuf,&lblockno);
    rblockno = cache->totsize;
    rpage = ajBtreeCacheWriteNodenew(cache);
    rpage->dirty = BT_LOCK;
    rpage->lockfor = 1132;
    rbuf = rpage->buf;

    
    GBT_PREV(lbuf,&prev);
    lv = prev;
    SBT_PREV(rbuf,lv);

    nkeys = order - 1;
    keypos = nkeys / 2;

    if(!(nkeys % 2))
	--keypos;

    mediankey = ajStrNewS(karray[keypos]);
    medianless = lblockno;
    mediangtr  = rblockno;


    GBT_NODETYPE(lbuf,&nodetype);
    v = nodetype;
    SBT_NODETYPE(rbuf,v);
    lv = overflow;
    SBT_OVERFLOW(rbuf,lv);


    totlen = 0;

    for(i=0;i<keypos;++i)
    {
	ajStrAssignS(&tkarray[i],karray[i]);
	totlen += ajStrGetLen(karray[i]);
	tparray[i] = parray[i];
    }

    tparray[i] = parray[i];
    v = totlen;
    SBT_TOTLEN(lbuf,v);
    n = i;
    v = n;
    SBT_NKEYS(lbuf,v);
    btreeWriteNode(cache,lpage,tkarray,tparray,i);



    for(i=0;i<n+1;++i)
    {
	tpage = ajBtreeCacheRead(cache,tparray[i]);
	tbuf = tpage->buf;
	lv = lblockno;
	SBT_PREV(tbuf,lv);
	tpage->dirty = BT_DIRTY;
    }


    totlen = 0;

    for(i=keypos+1;i<nkeys;++i)
    {
	ajStrAssignS(&tkarray[i-(keypos+1)],karray[i]);
	totlen += ajStrGetLen(karray[i]);
	tparray[i-(keypos+1)] = parray[i];
    }

    tparray[i-(keypos+1)] = parray[i];
    v = totlen;
    SBT_TOTLEN(rbuf,v);
    rkeyno = (nkeys-keypos) - 1;
    v = rkeyno;
    SBT_NKEYS(rbuf,v);
    rpage->dirty = BT_DIRTY;

    btreeWriteNode(cache,rpage,tkarray,tparray,rkeyno);

    for(i=0;i<rkeyno+1;++i)
    {
	tpage = ajBtreeCacheRead(cache,tparray[i]);
	tbuf = tpage->buf;
	lv = rblockno;
	SBT_PREV(tbuf,lv);
	tpage->dirty = BT_DIRTY;
    }


    ibn = rblockno;

    if(MAJSTRCMPS(key,mediankey)<0)
	ibn = lblockno;

    ipage = ajBtreeCacheRead(cache,ibn);
    
    btreeInsertNonFull(cache,ipage,key,less,greater);

    btreeDeallocPriArray(cache, savekeyarrays);
    btreeDeallocPriArray(cache, arrays2);

    ipage = ajBtreeCacheRead(cache,prev);

    btreeInsertKey(cache,ipage,mediankey,medianless,mediangtr);
    ajStrDel(&mediankey);

    return;
}




/* @funcstatic btreeSplitRoot *****************************************
**
** Split the root node
**
** @param [u] cache [AjPBtcache] cache
**
** @return [void]
** @@
******************************************************************************/

static void btreeSplitRoot(AjPBtcache cache)
{
    AjPBtpage rootpage = NULL;
    AjPBtpage rpage    = NULL;
    AjPBtpage lpage    = NULL;
    AjPBtpage tpage    = NULL;

    AjPBtMem arrays1 = NULL;
    AjPBtMem arrays2 = NULL;
    AjPStr *karray     = NULL;
    AjPStr *tkarray    = NULL;
    ajulong *parray     = NULL;
    ajulong *tparray    = NULL;

    ajuint order     = 0;
    ajuint nkeys     = 0;
    ajuint keypos    = 0;
    
    ajulong rblockno = 0L;
    ajulong lblockno = 0L;
    
    AjPStr key = NULL;
    ajuint  i;
    ajuint  j;

    unsigned char *rootbuf = NULL;
    unsigned char *rbuf    = NULL;
    unsigned char *lbuf    = NULL;
    unsigned char *tbuf    = NULL;
    
    ajuint nodetype  = 0;
    ajulong overflow = 0L;
    ajulong zero     = 0L;
    ajuint rkeyno    = 0;

    ajulong lv = 0L;
    ajuint  v  = 0;
    
#if AJINDEX_DEBUG
    ajDebug("btreeSplitRoot zero\n");
#endif
    ++statCallSplitRoot;

    /* ajDebug("In btreeSplitRoot\n"); */

    order = cache->order;

    arrays1 = btreeAllocPriArray(cache);
    arrays2 = btreeAllocPriArray(cache);
    karray = arrays1->karray;
    parray = arrays1->parray;
    tkarray = arrays2->karray;
    tparray = arrays2->parray;

    key = ajStrNew();

    rootpage = btreeCacheLocate(cache,0L);

    if(!rootpage)
	ajFatal("Root page has been unlocked 1 cache %S", cache->filename);
    
    rootbuf = rootpage->buf;

    nkeys = order - 1;

    keypos = nkeys / 2;

    if(!(nkeys % 2))
	--keypos;


    rblockno = cache->totsize;
    rpage = ajBtreeCacheWriteNodenew(cache);
    rpage->dirty = BT_LOCK;
    rpage->lockfor = 1141;

    lblockno = cache->totsize;
    lpage = ajBtreeCacheWriteNodenew(cache);

    if(!cache->level)
    {
	lv = zero;
	SBT_LEFT(lpage->buf,lv);
	lv = rblockno;
	SBT_RIGHT(lpage->buf,lv);
	lv = lblockno;
	SBT_LEFT(rpage->buf,lv);
	lv = zero;
	SBT_RIGHT(rpage->buf,lv);
    }

    btreeGetKeys(cache,rootbuf,&karray,&parray);
    
    btreeWriteNodeSingle(cache,rootpage,karray[keypos],lblockno,rblockno);

    rootpage->dirty = BT_LOCK;
    rootpage->lockfor = 1142;

    rbuf = rpage->buf;
    lbuf = lpage->buf;
    
    if(cache->level)
	nodetype = BT_INTERNAL;
    else
	nodetype = BT_LEAF;

    v = nodetype;
    SBT_NODETYPE(rbuf,v);
    v = nodetype;
    SBT_NODETYPE(lbuf,v);
    lv = overflow;
    SBT_OVERFLOW(rbuf,lv);
    lv = zero;
    SBT_PREV(rbuf,lv);
    lv = overflow;
    SBT_OVERFLOW(lbuf,lv);
    lv = zero;
    SBT_PREV(lbuf,lv);

    for(i=0;i<keypos;++i)
    {
	ajStrAssignS(&tkarray[i],karray[i]);
	tparray[i] = parray[i];
    }

    tparray[i] = parray[i];
    btreeWriteNode(cache,lpage,tkarray,tparray,keypos);

    for(i=0;i<=keypos;++i)
    {
	tpage = ajBtreeCacheRead(cache,tparray[i]);
	tbuf = tpage->buf;
	lv = lblockno;
	SBT_PREV(tbuf,lv);
	tpage->dirty = BT_DIRTY;
    }

    j = 0;

    for(i=keypos+1;i<nkeys;++i)
    {
	ajStrAssignS(&tkarray[j],karray[i]);
	tparray[j++] = parray[i];
    }

    tparray[j] = parray[i];
    rkeyno = (nkeys-keypos) - 1;
    rpage->dirty = BT_DIRTY;

    btreeWriteNode(cache,rpage,tkarray,tparray,rkeyno);

    for(i=0;i<rkeyno+1;++i)
    {
	tpage = ajBtreeCacheRead(cache,tparray[i]);
	tbuf = tpage->buf;
	lv = rblockno;
	SBT_PREV(tbuf,lv);
	tpage->dirty = BT_DIRTY;
    }


    for(i=0;i<order;++i)
    {
	ajStrDel(&karray[i]);
	ajStrDel(&tkarray[i]);
    }

    btreeDeallocPriArray(cache,arrays1);
    btreeDeallocPriArray(cache,arrays2);
    ++cache->level;

    ajStrDel(&key);
    
    return;
}




/* @funcstatic btreeGetKeys *********************************************
**
** Get Keys and Pointers from an internal node
**
** @param [u] cache [AjPBtcache] cache
** @param [u] buf [unsigned char *] page buffer
** @param [w] keys [AjPStr **] keys
** @param [w] ptrs [ajulong**] ptrs
**
** @return [void]
** @@
******************************************************************************/

static void btreeGetKeys(AjPBtcache cache, unsigned char *buf,
			 AjPStr **keys, ajulong **ptrs)
{
    AjPStr *karray = NULL;
    ajulong *parray = NULL;
    
    ajuint  nkeys;
    unsigned char *lenptr = NULL;
    unsigned char *keyptr = NULL;
    unsigned char *tbuf   = NULL;

    ajuint    i;
    ajuint    ival = 0;
    
    ajuint    len;
    ajuint    pagesize = 0;
    ajulong   overflow = 0L;

    AjPBtpage page = NULL;

    karray = *keys;
    parray = *ptrs;
    
    tbuf    = buf;

    pagesize = cache->pagesize;
    
    GBT_NKEYS(tbuf,&ival);
    nkeys = ival;
    if(!nkeys)
	ajFatal("GetKeys: No keys in node cache %S", cache->filename);

    lenptr =  PBT_KEYLEN(tbuf);
    keyptr = lenptr + nkeys * sizeof(ajuint);


    for(i=0; i<nkeys; ++i)
    {
	BT_GETAJUINT(lenptr,&ival);
	len = ival+1;
	
	if((ajuint)((keyptr-tbuf+1) + len + sizeof(ajulong)) > pagesize)
	{
#if AJINDEX_DEBUG
    	    ajDebug("GetKeys: Overflow\n");
#endif
	    GBT_OVERFLOW(tbuf,&overflow);
	    page = ajBtreeCacheRead(cache,overflow);
	    tbuf = page->buf;
	    GBT_NODETYPE(tbuf,&ival);

	    if(ival != BT_OVERFLOW)
		ajFatal("Overflow node expected but not found for cache %S",
                        cache->filename);
	    /*
	     ** The length pointer is restricted to the initial page.
	     ** The keyptr in overflow pages starts at the Key Lengths
	     ** position!
	     */
	    keyptr = PBT_KEYLEN(tbuf);
	}

	ajStrAssignC(&karray[i],(const char *)keyptr);
	keyptr += len;

	BT_GETAJULONG(keyptr,&parray[i]);
	keyptr += sizeof(ajulong);
	lenptr += sizeof(ajuint);
    }
    

    if((ajuint)((keyptr-tbuf+1) + sizeof(ajulong)) > pagesize)
    {
	GBT_OVERFLOW(tbuf,&overflow);
	page = ajBtreeCacheRead(cache,overflow);
	tbuf = page->buf;
	GBT_NODETYPE(tbuf,&ival);

	if(ival != BT_OVERFLOW)
	    ajFatal("Overflow node expected but not found at end for cache %S",
                    cache->filename);
	/*
	 ** The length pointer is restricted to the initial page.
	 ** The keyptr in overflow pages starts at the Key Lengths
	 ** position!
	 */
	keyptr = PBT_KEYLEN(tbuf);
    }
    
    BT_GETAJULONG(keyptr,&parray[i]);

    return;
}




/* @funcstatic btreeGetPointers *********************************************
**
** Get Pointers from an internal node
**
** @param [u] cache [AjPBtcache] cache
** @param [u] buf [unsigned char *] page buffer
** @param [w] ptrs [ajulong**] ptrs
**
** @return [ajuint] Number of pointers
** @@
******************************************************************************/

static ajuint btreeGetPointers(AjPBtcache cache, unsigned char *buf,
                               ajulong **ptrs)
{
    ajulong *parray = NULL;
    
    ajuint  nkeys;
    unsigned char *lenptr = NULL;
    unsigned char *keyptr = NULL;
    unsigned char *tbuf   = NULL;

    ajuint    i;
    ajuint    ival = 0;
    
    ajuint    len;
    ajuint    pagesize = 0;
    ajulong   overflow = 0L;
    AjPBtpage page = NULL;

    parray = *ptrs;
    
    tbuf    = buf;

    pagesize = cache->pagesize;
    
    GBT_NKEYS(tbuf,&ival);
    nkeys = ival;
    if(!nkeys)
	ajFatal("btreeGetPointers: No keys in node cache %S",
                cache->filename);

    lenptr =  PBT_KEYLEN(tbuf);
    keyptr = lenptr + nkeys * sizeof(ajuint);

    GBT_NODETYPE(tbuf,&ival);

    for(i=0; i<nkeys; ++i)
    {
	BT_GETAJUINT(lenptr,&ival);
	len = ival+1;
	
	if((ajuint)((keyptr-tbuf+1) + len + sizeof(ajulong)) > pagesize)
	{
	    GBT_OVERFLOW(tbuf,&overflow);
	    page = ajBtreeCacheRead(cache,overflow);
	    tbuf = page->buf;
	    GBT_NODETYPE(tbuf,&ival);
	    if(ival != BT_OVERFLOW)
		ajFatal("Overflow node expected but not found cache %S",
                        cache->filename);
	    /*
	     ** The length pointer is restricted to the initial page.
	     ** The keyptr in overflow pages starts at the Key Lengths
	     ** position!
	     */
	    keyptr = PBT_KEYLEN(tbuf);
	}

	keyptr += len;

	BT_GETAJULONG(keyptr,&parray[i]);
	keyptr += sizeof(ajulong);
	lenptr += sizeof(ajuint);
    }
    

    if((ajuint)((keyptr-tbuf+1) + sizeof(ajulong)) > pagesize)
    {
	/* ajDebug("GetPointers: Overflow\n"); */
	GBT_OVERFLOW(tbuf,&overflow);
	page = ajBtreeCacheRead(cache,overflow);
	tbuf = page->buf;
	GBT_NODETYPE(tbuf,&ival);
	if(ival != BT_OVERFLOW)
	    ajFatal("Overflow node expected but not found cache %S",
                    cache->filename);
	/*
	 ** The length pointer is restricted to the initial page.
	 ** The keyptr in overflow pages starts at the Key Lengths
	 ** position!
	 */
	keyptr = PBT_KEYLEN(tbuf);
    }
    
    BT_GETAJULONG(keyptr,&parray[i]);

    return i;
}





/* @funcstatic btreeGetBlockC **********************************************
**
** Get Block number matching a key
**
** @param [u] cache [AjPBtcache] cache
** @param [u] buf [unsigned char *] page buffer
** @param [r] ckey [const char*] key
**
** @return [ajulong] Block number
** @@
******************************************************************************/

static ajulong btreeGetBlockC(AjPBtcache cache, unsigned char *buf,
                              const char* ckey)
{
    ajulong blockno = 0L;

    ajuint m;
    unsigned char *lenptr = NULL;
    unsigned char *keyptr = NULL;
    unsigned char *tbuf   = NULL;

    ajuint    i;
    ajuint    ival = 0;
    ajuint    len;
    ajuint    pagesize = 0;
    ajulong   overflow = 0L;
    
    AjPBtpage page = NULL;

    tbuf    = buf;

    pagesize = cache->pagesize;

    GBT_NKEYS(tbuf,&m);
    if(!m)
	ajFatal("btreeGetBlockC: No keys in node cache %S",
                cache->filename);

    lenptr =  PBT_KEYLEN(tbuf);
    keyptr = lenptr + m * sizeof(ajuint);

    for(i=0; i<m; ++i)
    {
	BT_GETAJUINT(lenptr,&ival);
	len = ival+1;
	
	if((ajuint)((keyptr-tbuf+1) + len + sizeof(ajulong)) > pagesize)
	{
    	    /* ajDebug("btreeGetBlockC: Overflow\n"); */
	    GBT_OVERFLOW(tbuf,&overflow);
	    page = ajBtreeCacheRead(cache,overflow);
	    tbuf = page->buf;
	    GBT_NODETYPE(tbuf,&ival);
	    if(ival != BT_OVERFLOW)
		ajFatal("Overflow node expected but not found cache %S",
                        cache->filename);
	    /*
	     ** The length pointer is restricted to the initial page.
	     ** The keyptr in overflow pages starts at the Key Lengths
	     ** position!
	     */
	    keyptr = PBT_KEYLEN(tbuf);
	}

        if(strcmp((const char*)keyptr, ckey) > 0)
        {
            keyptr += len;

            BT_GETAJULONG(keyptr,&blockno);
            return blockno;
        }

        keyptr += len;
	keyptr += sizeof(ajulong);
	lenptr += sizeof(ajuint);
    }
    

    if((ajuint)((keyptr-tbuf+1) + sizeof(ajulong)) > pagesize)
    {
	/* ajDebug("btreeGetBlockC: Overflow\n"); */
	GBT_OVERFLOW(tbuf,&overflow);
	page = ajBtreeCacheRead(cache,overflow);
	tbuf = page->buf;
	GBT_NODETYPE(tbuf,&ival);
	if(ival != BT_OVERFLOW)
	    ajFatal("Overflow node expected but not found cache %S",
                    cache->filename);
	/*
	 ** The length pointer is restricted to the initial page.
	 ** The keyptr in overflow pages starts at the Key Lengths
	 ** position!
	 */
	keyptr = PBT_KEYLEN(tbuf);
    }
    
    BT_GETAJULONG(keyptr,&blockno);
    
    return blockno;
}




/* @funcstatic btreeGetBlockN **********************************************
**
** Get Block number matching a numeric key
**
** @param [u] cache [AjPBtcache] cache
** @param [u] buf [unsigned char *] page buffer
** @param [r] numkey [ajulong] key
**
** @return [ajulong] Block number
** @@
******************************************************************************/

static ajulong btreeGetBlockN(AjPBtcache cache, unsigned char *buf,
                              ajulong numkey)
{
    ajulong blockno = 0L;

    ajuint m;
    unsigned char *pptr = NULL;
    unsigned char *kptr = NULL;

    ajuint    i;
    ajulong   kval = 0L;

    GBT_NKEYS(buf,&m);
    if(!m)
	ajFatal("btreeGetBlockN: No keys in node cache %S", cache->filename);

    kptr = PBT_KEYLEN(buf);
    pptr = kptr + m * sizeof(ajulong);

    for(i=0; i<m; ++i)
    {
	BT_GETAJULONG(kptr,&kval);
	
        if(kval > numkey)
        {
            BT_GETAJULONG(pptr,&blockno);
            return blockno;
        }

	kptr += sizeof(ajulong);
	pptr += sizeof(ajulong);
    }
    

    BT_GETAJULONG(pptr,&blockno);

    return blockno;
}




/* @funcstatic btreeGetBlockFirstN *****************************************
**
** Get Block number of first block if more than numeric key
** or second block
**
** @param [u] cache [AjPBtcache] cache
** @param [u] buf [unsigned char *] page buffer
** @param [r] numkey [ajulong] key
**
** @return [ajulong] Block number
** @@
******************************************************************************/

static ajulong btreeGetBlockFirstN(AjPBtcache cache, unsigned char *buf,
                                   ajulong numkey)
{
    ajulong blockno = 0L;

    ajuint m;
    unsigned char *pptr = NULL;
    unsigned char *kptr = NULL;

    ajulong   kval = 0L;
    
    GBT_NKEYS(buf,&m);
    if(!m)
	ajFatal("btreeGetBlockFirstN: No keys in node cache %S",
                cache->filename);

    kptr = PBT_KEYLEN(buf);
    pptr = kptr + m * sizeof(ajulong);

    BT_GETAJULONG(kptr,&kval);
	
    if(kval > numkey)
    {
        BT_GETAJULONG(pptr,&blockno);
        return blockno;
    }

    kptr += sizeof(ajulong);
    pptr += sizeof(ajulong);
    

    BT_GETAJULONG(pptr,&blockno);

    return blockno;
}




/* @funcstatic btreeGetBlockS *************************************************
**
** Get Block number matching a key
**
** @param [u] cache [AjPBtcache] cache
** @param [u] buf [unsigned char *] page buffer
** @param [r] key [const AjPStr] key
**
** @return [ajulong] Block number
** @@
******************************************************************************/

static ajulong btreeGetBlockS(AjPBtcache cache, unsigned char *buf,
                             const AjPStr key)
{
    return btreeGetBlockC(cache, buf, MAJSTRGETPTR(key));
}




/* @funcstatic btreeGetBlockFirstC ********************************************
**
** Get Block number of first block if more than key
** or second block
**
** @param [u] cache [AjPBtcache] cache
** @param [u] buf [unsigned char *] page buffer
** @param [r] ckey [const char*] key
** @param [r] clen [ajuint] Maximum length of key to test (e.g. for wildcards)
**                         or zero to test full key
**
** @return [ajulong] Block number
** @@
******************************************************************************/

static ajulong btreeGetBlockFirstC(AjPBtcache cache, unsigned char *buf,
                                  const char* ckey, ajuint clen)
{
    ajulong blockno = 0L;

    ajuint m;
    unsigned char *lenptr = NULL;
    unsigned char *keyptr = NULL;
    unsigned char *tbuf   = NULL;

    ajuint    ival = 0;
    ajuint    len;
    ajuint    klen = 0;
    
    tbuf    = buf;

    GBT_NKEYS(tbuf,&m);
    if(!m)
	ajFatal("btreeGetBlockFirstC: No keys in node cache %S",
                cache->filename);

    lenptr =  PBT_KEYLEN(tbuf);
    keyptr = lenptr + m * sizeof(ajuint);

    BT_GETAJUINT(lenptr,&ival);
    len = ival+1;
    klen = ival;
    if(clen && (klen > clen))
        klen = clen;
	
    if(strncmp((const char*)keyptr, ckey, klen) > 0)
    {
        keyptr += len;
        
        BT_GETAJULONG(keyptr,&blockno);
        return blockno;
    }

    keyptr += len;
    keyptr += sizeof(ajulong);
    lenptr += sizeof(ajuint);
    
    BT_GETAJULONG(keyptr,&blockno);

    return blockno;
}




/* @funcstatic btreeGetBlockFirstS ********************************************
**
** Get Block number matching a key
**
** @param [u] cache [AjPBtcache] cache
** @param [u] buf [unsigned char *] page buffer
** @param [r] key [const AjPStr] key
**
** @return [ajulong] Block number
** @@
******************************************************************************/

static ajulong btreeGetBlockFirstS(AjPBtcache cache, unsigned char *buf,
                                  const AjPStr key)
{
    return btreeGetBlockFirstC(cache, buf, MAJSTRGETPTR(key),0);
}




/* @funcstatic btreeIdCompare *******************************************
**
** Comparison function for ajListSort
**
** @param [r] a [const void*] ID 1
** @param [r] b [const void*] ID 2
**
** @return [ajint] 0 = bases match
** @@
******************************************************************************/

static ajint btreeIdCompare(const void *a, const void *b)
{
    return MAJSTRCMPS((*(AjPBtId const *)a)->id,
                      (*(AjPBtId const *)b)->id);
}




/* @funcstatic btreeNumIdCompare *******************************************
**
** Comparison function for ajListSort
**
** @param [r] a [const void*] ID 1
** @param [r] b [const void*] ID 2
**
** @return [ajint] 0 = bases match
** @@
******************************************************************************/

static ajint btreeNumIdCompare(const void *a, const void *b)
{
    ajlong val;

    val = (ajlong) ((*(AjPBtNumId const *)a)->offset -
                    (*(AjPBtNumId const *)b)->offset);

    if(!val)
      return 0;

    return (val < 0L) ? -1 : 1;
}




/* @funcstatic btreeWriteNode *******************************************
**
** Write an internal node
**
** @param [u] cache [AjPBtcache] cache
** @param [u] spage [AjPBtpage] buffer
** @param [r] keys [AjPStr const *] keys
** @param [r] ptrs [const ajulong*] page pointers
** @param [r] nkeys [ajuint] number of keys

**
** @return [void]
** @@
******************************************************************************/

static void btreeWriteNode(AjPBtcache cache, AjPBtpage spage,
			   AjPStr const *keys, const ajulong *ptrs,
                           ajuint nkeys)
{
    unsigned char *lbuf   = NULL;
    unsigned char *tbuf   = NULL;
    unsigned char *lenptr = NULL;
    unsigned char *keyptr = NULL;
    AjPBtpage page        = NULL;
    
    ajulong   overflow = 0L;
    ajulong   blockno  = 0L;

    ajuint   i;
    ajuint   len;
    ajuint    v  = 0;
    ajulong   lv = 0L;
    ajuint totlen = 0;

    /* ajuint unused = 0; */

    lbuf = spage->buf;
    tbuf = lbuf;
    page = spage;

    v = nkeys;
    SBT_NKEYS(lbuf,v);
    
    lenptr = PBT_KEYLEN(lbuf);
    keyptr = lenptr + nkeys * sizeof(ajuint);

    for(i=0;i<nkeys;++i)
    {
	if((ajuint) (lenptr-lbuf+1) > cache->pagesize)
	    ajFatal("WriteNode: Too many key lengths for available pagesize");

	len = ajStrGetLen(keys[i]);
	v = len;
	BT_SETAJUINT(lenptr,v);
        totlen += len;
	lenptr += sizeof(ajuint);
    }

    v = totlen;
    SBT_TOTLEN(lbuf,v);

    GBT_OVERFLOW(lbuf,&overflow);
    GBT_BLOCKNUMBER(lbuf, &blockno);

    for(i=0;i<nkeys;++i)
    {
	len = ajStrGetLen(keys[i]) + 1;
	if((ajuint)((keyptr-tbuf+1) + len + sizeof(ajulong)) > cache->pagesize)
	{
	    if(!overflow)		/* No overflow buckets yet */
	    {
		page->dirty = BT_DIRTY;
		blockno = cache->totsize;
		lv = blockno;
		SBT_OVERFLOW(tbuf,lv);
		page = ajBtreeCacheWriteNodenew(cache);
		tbuf = page->buf;
		v = BT_OVERFLOW;
		SBT_NODETYPE(tbuf,v);
		lv = blockno;
		SBT_BLOCKNUMBER(tbuf,lv);
	    }
	    else
	    {
		page = ajBtreeCacheRead(cache,overflow);
		tbuf = page->buf;
		GBT_OVERFLOW(tbuf,&overflow);
	    }

	    keyptr = PBT_KEYLEN(tbuf);
	    page->dirty = BT_DIRTY;
	}


	sprintf((char *)keyptr,"%s",ajStrGetPtr(keys[i]));
	keyptr += len;
	lv = ptrs[i];
	BT_SETAJULONG(keyptr,lv);
	keyptr += sizeof(ajulong);
    }
    



    if((ajuint)((keyptr-tbuf+1) + sizeof(ajulong)) > cache->pagesize)
    {
	page->dirty = BT_DIRTY;

	if(!overflow)			/* No overflow buckets yet */
	{
	    blockno = cache->totsize;
	    lv = blockno;
	    SBT_OVERFLOW(tbuf,lv);
	    page = ajBtreeCacheWriteNodenew(cache);
	    tbuf = page->buf;
	    v = BT_OVERFLOW;
	    SBT_NODETYPE(tbuf,v);
	}
	else
	{
	    page = ajBtreeCacheRead(cache,overflow);
	    tbuf = page->buf;
	}

	keyptr = PBT_KEYLEN(tbuf);
    }
    
    page->dirty = BT_DIRTY;

    overflow = 0L;
    SBT_OVERFLOW(tbuf,overflow);

    lv = ptrs[i];
    BT_SETAJULONG(keyptr,lv);
    keyptr += sizeof(ajulong);

    /*
    unused = cache->pagesize - (keyptr - lbuf);
    if(unused > cache->pagesize)
        ajFatal("unused %u for pagesize %u", unused, cache->pagesize);
    if(unused > 0)
        memset(keyptr, 0, unused);
    */

    return;
}




/* @funcstatic btreeWriteNodeSingle *******************************************
**
** Write an internal node with a single key
**
** @param [u] cache [AjPBtcache] cache
** @param [u] spage [AjPBtpage] buffer
** @param [r] key [const AjPStr] key
** @param [r] lptr [ajulong] left page pointer
** @param [r] rptr [ajulong] right page pointer

**
** @return [void]
** @@
******************************************************************************/

static void btreeWriteNodeSingle(AjPBtcache cache, AjPBtpage spage,
                                 const AjPStr key, ajulong lptr,
                                 ajulong rptr)
{
    unsigned char *lbuf   = NULL;
    unsigned char *tbuf   = NULL;
    unsigned char *lenptr = NULL;
    unsigned char *keyptr = NULL;
    AjPBtpage page        = NULL;
    
    ajulong   overflow = 0L;
    ajulong   blockno  = 0L;

    ajuint   len;
    ajuint    v  = 0;
    ajulong   lv = 0L;

    ajuint nkeys = 1;

    /* ajuint unused = 1; */

    /* ajDebug("In btreeWriteNodeSingle\n"); */

    lbuf = spage->buf;
    tbuf = lbuf;
    page = spage;

    v = 1;
    SBT_NKEYS(lbuf,v);
    lenptr = PBT_KEYLEN(lbuf);
    keyptr = lenptr + nkeys * sizeof(ajuint);

    if((ajuint) (lenptr-lbuf+1) > cache->pagesize)
        ajFatal("WriteNode: Too many key lengths for available pagesize "
                "cache %S", cache->pagesize);
    len = MAJSTRGETLEN(key);
    v = len;
    BT_SETAJUINT(lenptr,v);
    lenptr += sizeof(ajuint);
    v = len;
    SBT_TOTLEN(lbuf,v);

    GBT_OVERFLOW(lbuf,&overflow);

    len = MAJSTRGETLEN(key) + 1;
    if((ajuint)((keyptr-tbuf+1) + len + sizeof(ajulong)) > cache->pagesize)
    {
        /* ajDebug("WriteNode: Overflow\n"); */
        ajWarn("WriteNodeSingle: Overflow at start in page %Lu\n",
                   page->pagepos);
        if(!overflow)		/* No overflow buckets yet */
        {
            page->dirty = BT_DIRTY;
	    blockno = cache->totsize;
	    lv = blockno;
            SBT_OVERFLOW(tbuf,lv);
            page = ajBtreeCacheWriteOverflownew(cache);
            tbuf = page->buf;
        }
        else
        {
            page = ajBtreeCacheRead(cache,overflow);
            tbuf = page->buf;
            GBT_OVERFLOW(tbuf,&overflow);
        }
        keyptr = PBT_KEYLEN(tbuf);
        page->dirty = BT_DIRTY;
    }


    sprintf((char *)keyptr,"%s",MAJSTRGETPTR(key));
    keyptr += len;
    lv = lptr;
    BT_SETAJULONG(keyptr,lv);
    keyptr += sizeof(ajulong);
    

    if((ajuint)((keyptr-tbuf+1) + sizeof(ajulong)) > cache->pagesize)
    {
	/* ajDebug("WriteNode: Overflow\n"); */
        page->dirty = BT_DIRTY;
	if(!overflow)			/* No overflow buckets yet */
	{
            ajWarn("WriteNodeSingle: Overflow at end in page %Lu\n",
                   page->pagepos);
	    blockno = cache->totsize;
	    lv = blockno;
	    SBT_OVERFLOW(tbuf,lv);
	    page = ajBtreeCacheWriteOverflownew(cache);
	    tbuf = page->buf;
	}
	else
	{
	    page = ajBtreeCacheRead(cache,overflow);
	    tbuf = page->buf;
	}
	keyptr = PBT_KEYLEN(tbuf);
    }
    
    overflow = 0L;
    SBT_OVERFLOW(tbuf,overflow);

    lv = rptr;
    BT_SETAJULONG(keyptr,lv);
    keyptr += sizeof(ajulong);

    /*
    unused = cache->pagesize - (keyptr - tbuf);
    if(unused > cache->pagesize)
        ajFatal("unused %u for pagesize %u", unused, cache->pagesize);
    if(unused > 0)
        memset(keyptr, 0, unused);
    */

    page->dirty = BT_DIRTY;

    return;
}




/* @func ajBtreeInsertId *********************************************
**
** Insert an ID structure into the tree
**
** @param [u] cache [AjPBtcache] cache
** @param [r] id [const AjPBtId] Id object
**
** @return [void] pointer to a page
** @@
******************************************************************************/

void ajBtreeInsertId(AjPBtcache cache, const AjPBtId id)
{
    AjPBtpage spage   = NULL;
    AjPStr key        = NULL;
    AjPBucket lbucket = NULL;
    AjPBucket rbucket = NULL;
    ajulong lblockno = 0L;
    ajulong rblockno = 0L;
    ajulong blockno  = 0L;
    ajulong shift    = 0L;

    ajuint nkeys = 0;

    ajuint nodetype = 0;
 
    ajuint n;
    
    unsigned char *buf = NULL;

    /* ajDebug("In ajBtreeInsertId\n"); */
    
    key = ajStrNew();
    

    ajStrAssignS(&key,id->id);

    if(!ajStrGetLen(key))
    {
	ajStrDel(&key);

	return;
    }

    spage = ajBtreeFindInsert(cache,key);
    buf = spage->buf;

    GBT_NKEYS(buf,&nkeys);
    GBT_NODETYPE(buf,&nodetype);
    
    if(!nkeys)
    {
	lbucket  = btreeBucketNew(0);
	rbucket  = btreeBucketNew(0);

	lblockno = cache->totsize;
	btreeWriteBucket(cache,lbucket,lblockno);

	rblockno = cache->totsize;
	btreeWriteBucket(cache,rbucket,rblockno);	

	btreeWriteNodeSingle(cache,spage,key,lblockno, rblockno);

	btreeBucketDel(&lbucket);
	btreeBucketDel(&rbucket);

	btreeAddToBucket(cache,rblockno,id);
        ++cache->countunique;
        ++cache->countall;

        ajStrDel(&key);

	return;
    }
    
    blockno = btreeGetBlockS(cache, buf, key);

    if(nodetype != BT_ROOT)
	if((shift = btreeInsertShiftS(cache,&spage,key)))
	    blockno = shift;

    buf = spage->buf;

    n = btreeNumInBucket(cache,blockno);

    if(n == cache->nperbucket)
    {
	if(btreeReorderBuckets(cache,spage))
	{
            blockno = btreeGetBlockS(cache, buf, key);
	}
	else
	{
	    btreeSplitLeaf(cache,spage);
	    spage = ajBtreeFindInsert(cache,key);
	    buf = spage->buf;

            blockno = btreeGetBlockS(cache, buf, key);
	}
    }


    btreeAddToBucket(cache,blockno,id);

    ++cache->countunique;
    ++cache->countall;

    ajStrDel(&key);

    return;
}




/* @func ajBtreeIdFromKey ****************************************************
**
** Get an ID structure from a leaf node given a key
**
** @param [u] cache [AjPBtcache] cache
** @param [r] key [const AjPStr] key
**
** @return [AjPBtId] pointer to an ID structure or NULL if not found
** @@
******************************************************************************/

AjPBtId ajBtreeIdFromKey(AjPBtcache cache, const AjPStr key)
{
    AjPBtpage page   = NULL;
    AjPBucket bucket = NULL;
    AjPBtId   id     = NULL;
    AjPBtId   tid    = NULL;
    
    unsigned char *buf = NULL;

    ajuint nentries = 0;
    
    ajuint i;

    ajulong blockno = 0L;
    AjBool found   = ajFalse;

    AjPStr keystr = NULL;

    /* ajDebug("In ajBtreeIdFromKey\n"); */

    if(!cache->countunique && !cache->countall)
        return NULL;

    keystr = ajStrNewS(key);
    ajStrFmtQuery(&keystr);

    page = ajBtreeFindInsert(cache,keystr);
    buf = page->buf;

    blockno = btreeGetBlockS(cache,buf,keystr);

    bucket = btreeReadBucket(cache,blockno);
    
    nentries = bucket->Nentries;

    found = ajFalse;

    for(i=0;i<nentries;++i)
	if(!MAJSTRCMPS(bucket->Ids[i]->id, keystr))
	{
	    found = ajTrue;
	    break;
	}

    if(found)
    {
	id  = ajBtreeIdNew();
	tid = bucket->Ids[i];
	ajStrAssignS(&id->id,tid->id);
	id->dups = tid->dups;
	id->dbno = tid->dbno;
	id->offset = tid->offset;
	id->refoffset = tid->refoffset;
    }

    btreeBucketDel(&bucket);
    ajStrDel(&keystr);

    if(!found)
	return NULL;

    return id;
}




/* @func ajBtreeWriteParamsC ************************************************
**
** Write B+ tree parameters to file
**
** @param [r] cache [const AjPBtcache] cache
** @param [r] fntxt [const char *] file name
** @param [r] exttxt [const char *] index file extension name
** @param [r] idirtxt [const char *] index file directory
**
** @return [void]
** @@
******************************************************************************/

void ajBtreeWriteParamsC(const AjPBtcache cache, const char *fntxt,
			 const char *exttxt, const char *idirtxt)
{
    AjPStr  fname = NULL;
    AjPFile outf  = NULL;

    fname = ajStrNewRes(128);

    if(!*idirtxt)
      ajFmtPrintS(&fname,"%s.p%s",fntxt,exttxt);
    else
      ajFmtPrintS(&fname,"%s%s%s.p%s",idirtxt,SLASH_STRING,fntxt,exttxt);

    if(!(outf = ajFileNewOutNameS(fname)))
	ajFatal("Cannot open param file %S\n",fname);

    if(cache->secondary)
        ajFmtPrintF(outf,"Type      %s\n","Secondary");
    else
        ajFmtPrintF(outf,"Type      %s\n","Identifier");
    ajFmtPrintF(outf,"Compress  %B\n",cache->compressed);
    ajFmtPrintF(outf,"Pages     %Lu\n",cache->pagecount);
    ajFmtPrintF(outf,"Order     %u\n",cache->order);
    ajFmtPrintF(outf,"Fill      %u\n",cache->nperbucket);
    ajFmtPrintF(outf,"Pagesize  %u\n",cache->pagesize);
    ajFmtPrintF(outf,"Level     %u\n",cache->level);
    ajFmtPrintF(outf,"Cachesize %u\n",cache->cachesize);
    ajFmtPrintF(outf,"Order2    %u\n",cache->sorder);
    ajFmtPrintF(outf,"Fill2     %u\n",cache->snperbucket);
    ajFmtPrintF(outf,"Count     %Lu\n",cache->countunique);
    ajFmtPrintF(outf,"Fullcount %Lu\n",cache->countall);
    ajFmtPrintF(outf,"Kwlimit   %u\n",cache->kwlimit);

    ajFileClose(&outf);
    ajStrDel(&fname);

    return;
}




/* @obsolete ajBtreeWriteParams
** @rename ajBtreeWriteParamsC
*/
__deprecated void ajBtreeWriteParams(const AjPBtcache cache, const char *fn,
			const char *ext, const char *idir)
{
    ajBtreeWriteParamsC(cache, fn, ext, idir);
    return;
}




/* @func ajBtreeWriteParamsS ************************************************
**
** Write B+ tree parameters to file
**
** @param [r] cache [const AjPBtcache] cache
** @param [r] fn [const AjPStr] file name
** @param [r] ext [const AjPStr] index file extension name
** @param [r] idir [const AjPStr] index file directory
**
** @return [void]
** @@
******************************************************************************/

void ajBtreeWriteParamsS(const AjPBtcache cache, const AjPStr fn,
                         const AjPStr ext, const AjPStr idir)
{
    AjPStr  fname = NULL;
    AjPFile outf  = NULL;

    fname = ajStrNewRes(128);

    if(ajStrGetLen(idir))
      ajFmtPrintS(&fname,"%S%s%S.p%S",idir,SLASH_STRING,fn,ext);
    else
        ajFmtPrintS(&fname,"%S.p%S",fn,ext);

    if(!(outf = ajFileNewOutNameS(fname)))
	ajFatal("Cannot open param file %S\n",fname);

    if(cache->secondary)
        ajFmtPrintF(outf,"Type      %s\n","Secondary");
    else
        ajFmtPrintF(outf,"Type      %s\n","Identifier");
    ajFmtPrintF(outf,"Compress  %B\n",cache->compressed);
    ajFmtPrintF(outf,"Pages     %u\n",cache->pagecount);
    ajFmtPrintF(outf,"Order     %u\n",cache->order);
    ajFmtPrintF(outf,"Fill      %u\n",cache->nperbucket);
    ajFmtPrintF(outf,"Pagesize  %u\n",cache->pagesize);
    ajFmtPrintF(outf,"Level     %u\n",cache->level);
    ajFmtPrintF(outf,"Cachesize %u\n",cache->cachesize);
    ajFmtPrintF(outf,"Order2    %u\n",cache->sorder);
    ajFmtPrintF(outf,"Fill2     %u\n",cache->snperbucket);
    ajFmtPrintF(outf,"Count     %Lu\n",cache->countunique);
    ajFmtPrintF(outf,"Fullcount %Lu\n",cache->countall);
    ajFmtPrintF(outf,"Kwlimit   %u\n",cache->kwlimit);

    ajFileClose(&outf);
    ajStrDel(&fname);

    return;
}




/* @func ajBtreeReadParamsC ************************************************
**
** Read B+ tree parameters from file
**
** @param [r] filetxt [const char *] file
** @param [r] exttxt [const char *] file extension
** @param [r] idirtxt [const char *] index directory
** @param [w] secondary [AjBool*] true for a secondary index
** @param [w] compressed [AjBool*] true for a compressed index
** @param [w] kwlimit [ajuint*] maximum length of a keyword
** @param [w] pagesize [ajuint*] size of pages
** @param [w] cachesize [ajuint*] cachesize
** @param [w] pagecount [ajulong*] page count
** @param [w] order [ajuint*] tree order
** @param [w] nperbucket [ajuint*] bucket fill
** @param [w] level [ajuint*] depth of tree (0 = root leaf)
** @param [w] sorder [ajuint*] secondary tree order
** @param [w] snperbucket [ajuint*] secondary bucket fill
** @param [w] count [ajulong*] number of primary keywords in the index
** @param [w] countall [ajulong*] number of total keywords in the index
**
** @return [AjBool] True on success
** @@
******************************************************************************/

AjBool ajBtreeReadParamsC(const char *filetxt, const char *exttxt,
                          const char *idirtxt,
                          AjBool *secondary, AjBool *compressed,
                          ajuint *kwlimit,
                          ajuint *pagesize, ajuint *cachesize,
                          ajulong *pagecount,
                          ajuint *order, ajuint *nperbucket, ajuint *level,
                          ajuint *sorder, ajuint *snperbucket,
                          ajulong *count, ajulong *countall)
{
    AjPStr fname = NULL;
    AjPStr line  = NULL;
    AjPFile inf  = NULL;
    AjPStr type = NULL;
    AjBool setsecond = ajFalse;

    *countall = 0L;
    *compressed = ajFalse;
    *pagecount = 0L;

    line  = ajStrNew();

    fname = ajStrNew();

    if(!*idirtxt)
        ajFmtPrintS(&fname,"%s.p%s",filetxt,exttxt);
    else if(idirtxt[strlen(idirtxt)-1] == SLASH_CHAR)
        ajFmtPrintS(&fname,"%s%s.p%s",idirtxt,filetxt,exttxt);
    else
      ajFmtPrintS(&fname,"%s%s%s.p%s",idirtxt,SLASH_STRING,filetxt,exttxt);
    
    if(!(inf = ajFileNewInNameS(fname)))
    {
	ajErr("Cannot open param file %S %d: '%s'\n",
              fname, errno, strerror(errno));
        return ajFalse;
    }

    while(ajReadlineTrim(inf,&line))
    {
	if(ajStrPrefixC(line,"Order2 "))
	{
	    ajFmtScanS(line,"%*s%u",sorder);
	    continue;
	}

	if(ajStrPrefixC(line,"Fill2 "))
	{
	    ajFmtScanS(line,"%*s%u",snperbucket);
	    continue;
	}

	if(ajStrPrefixC(line,"Type "))
        {
	    ajFmtScanS(line,"%*s%S",&type);
            if(ajStrMatchC(type, "Secondary"))
                *secondary = ajTrue;
            else if(ajStrMatchC(type, "Identifier"))
                *secondary = ajFalse;
            else
                ajErr("param file %S unknown Type value '%S'",
                      fname, type);
            setsecond = ajTrue;
        }

	if(ajStrPrefixC(line,"Compress "))
	    ajFmtScanS(line,"%*s%b",compressed);

	if(ajStrPrefixC(line,"Pages "))
	    ajFmtScanS(line,"%*s%Lu",pagecount);

	if(ajStrPrefixC(line,"Order "))
	    ajFmtScanS(line,"%*s%u",order);

	if(ajStrPrefixC(line,"Fill "))
	    ajFmtScanS(line,"%*s%u",nperbucket);

	if(ajStrPrefixC(line,"Pagesize "))
	    ajFmtScanS(line,"%*s%u",pagesize);

	if(ajStrPrefixC(line,"Level "))
	    ajFmtScanS(line,"%*s%u",level);

	if(ajStrPrefixC(line,"Cachesize "))
	    ajFmtScanS(line,"%*s%u",cachesize);

	if(ajStrPrefixC(line,"Count "))
	    ajFmtScanS(line,"%*s%Lu",count);

	if(ajStrPrefixC(line,"Fullcount ")) /* new index files only */
	    ajFmtScanS(line,"%*s%Lu",countall);

	if(ajStrPrefixC(line,"Kwlimit "))
	    ajFmtScanS(line,"%*s%u",kwlimit);
    }
    if(!setsecond)
    {
        *secondary = ajBtreeFieldGetSecondaryC(exttxt);
    }

    if(!*pagecount && !*compressed)
    {
        if(!*idirtxt)
            ajFmtPrintS(&fname,"%s.%s",filetxt,exttxt);
        else if(idirtxt[strlen(idirtxt)-1] == SLASH_CHAR)
            ajFmtPrintS(&fname,"%s%s.%s",idirtxt,filetxt,exttxt);
        else
	    ajFmtPrintS(&fname,"%s%s%s.%s",
			idirtxt,SLASH_STRING, filetxt,exttxt);
        *pagecount = ajFilenameGetSize(fname) / *pagesize;
    }

    ajFileClose(&inf);
    ajStrDel(&fname);
    ajStrDel(&line);
    ajStrDel(&type);
    
    return ajTrue;
}




/* @func ajBtreeReadParamsS ************************************************
**
** Read B+ tree parameters from file
**
** @param [r] file [const AjPStr] file
** @param [r] ext [const AjPStr] file extension
** @param [r] idir [const AjPStr] index directory
** @param [w] secondary [AjBool*] true for a secondary index
** @param [w] compressed [AjBool*] true for a compressed index
** @param [w] kwlimit [ajuint*] maximum length of a keyword
** @param [w] pagesize [ajuint*] size of pages
** @param [w] cachesize [ajuint*] cachesize
** @param [w] pagecount [ajulong*] page count
** @param [w] order [ajuint*] tree order
** @param [w] nperbucket [ajuint*] bucket fill
** @param [w] level [ajuint*] depth of tree (0 = root leaf)
** @param [w] sorder [ajuint*] secondary tree order
** @param [w] snperbucket [ajuint*] secondary bucket fill
** @param [w] count [ajulong*] number of primary keywords in the index
** @param [w] countall [ajulong*] number of total keywords in the index
**
** @return [AjBool] True on success
** @@
******************************************************************************/

AjBool ajBtreeReadParamsS(const AjPStr file, const AjPStr ext,
                          const AjPStr idir,
                          AjBool *secondary, AjBool *compressed,
                          ajuint *kwlimit,
                          ajuint *pagesize, ajuint *cachesize,
                          ajulong *pagecount,
                          ajuint *order, ajuint *nperbucket, ajuint *level,
                          ajuint *sorder, ajuint *snperbucket,
                          ajulong *count, ajulong *countall)
{
    return ajBtreeReadParamsC(MAJSTRGETPTR(file), MAJSTRGETPTR(ext),
                              MAJSTRGETPTR(idir), secondary, compressed,
                              kwlimit, pagesize, cachesize, pagecount, order,
                              nperbucket, level, sorder, snperbucket,
                              count, countall);
}




/* @funcstatic btreeSplitLeaf *********************************************
**
** Split a leaf and propagate up if necessary
**
** @param [u] cache [AjPBtcache] cache
** @param [u] spage [AjPBtpage] page
**
** @return [AjPBtpage] pointer to a parent page
** @@
******************************************************************************/

static AjPBtpage btreeSplitLeaf(AjPBtcache cache, AjPBtpage spage)
{
    ajuint nkeys     = 0;
    ajuint order     = 0;
    ajuint totalkeys = 0;
    ajuint keylimit  = 0;
    ajuint nodetype  = 0;

    ajuint rootnodetype  = 0;
    
    ajuint i;
    ajuint j;
    
    AjPBtpage lpage = NULL;
    AjPBtpage rpage = NULL;
    AjPBtpage page  = NULL;
    
    AjPStr mediankey  = NULL;
    ajulong mediangtr  = 0L;
    ajulong medianless = 0L;
    

    AjPBtId bid = NULL;
    AjPBtId cid = NULL;

    unsigned char *buf  = NULL;
    unsigned char *lbuf = NULL;
    unsigned char *rbuf = NULL;

    AjPList idlist = NULL;

    AjPBucket cbucket  = NULL;
    
    AjPBtMem arrays1 = NULL;
    AjPBtMem arrays2 = NULL;
    ajulong *parray = NULL;
    AjPStr *newkarray = NULL;
    ajulong *newparray = NULL;
    
    ajuint lno    = 0;
    ajuint rno    = 0;

    ajuint lbucketlimit   = 0;
    ajuint rbucketlimit   = 0;
    ajuint lmaxnperbucket = 0;
    ajuint rmaxnperbucket = 0;
    ajuint nperbucket    = 0;
    ajuint count         = 0;
    
    ajulong lblockno = 0L;
    ajulong rblockno = 0L;
    ajulong prev     = 0L;
    ajulong overflow = 0L;
    ajulong prevsave = 0L;

    ajulong zero = 0L;
    ajulong join = 0L;
    
    ajulong lv = 0L;
    ajuint  v  = 0;
    ajuint iold = 0;

#if AJINDEX_DEBUG
    ajDebug("btreeSplitLeaf %Lu\n", spage->pagepos);
#endif
    ++statCallSplitLeaf;

    /* ajDebug("In btreeSplitLeaf\n"); */

    order = cache->order;
    nperbucket = cache->nperbucket;

    mediankey = ajStrNew();
    
    arrays1 = btreeAllocPriArray(cache);
    parray = arrays1->parray;

    arrays2 = btreeAllocPriArray(cache);
    newkarray = arrays2->karray;
    newparray = arrays2->parray;

    buf = spage->buf;
    lbuf = buf;

    GBT_NKEYS(buf,&nkeys);
    GBT_NODETYPE(buf,&rootnodetype);

    if(rootnodetype == BT_ROOT)
    {
	/* ajDebug("Splitting root node\n"); */
	lblockno = cache->totsize;
	lpage = ajBtreeCacheWriteNodenew(cache);
	lbuf = lpage->buf;
	lv = prev;
	SBT_PREV(lbuf,lv);
    }
    else
    {
	lpage = spage;
    }

    lblockno = lpage->pagepos;
    lpage->dirty = BT_LOCK;
    lpage->lockfor = 1151;


    rblockno = cache->totsize;
    rpage = ajBtreeCacheWriteNodenew(cache);
    rpage->dirty = BT_LOCK;
    rpage->lockfor = 1152;
    rbuf = rpage->buf;

    if(rootnodetype == BT_ROOT)
    {
	lv = zero;
	SBT_RIGHT(rbuf,lv);
	lv = zero;
	SBT_LEFT(lbuf,lv);
    }
    else
    {
	GBT_RIGHT(lbuf,&join);
	lv = join;
	SBT_RIGHT(rbuf,lv);
    }

    lv = lblockno;
    SBT_LEFT(rbuf,lv);
    lv = rblockno;
    SBT_RIGHT(lbuf,lv);


    btreeGetPointers(cache,buf,&parray);


    keylimit = nkeys+1;

    idlist = ajListNew();

    for(i=0;i<keylimit;++i)
	btreeBucketIdlist(cache,parray[i],idlist);

    ajListSort(idlist,btreeIdCompare);

    totalkeys = ajListGetLength(idlist);

    btreeBucketSplitCalc(totalkeys, keylimit, nperbucket,
                         &lbucketlimit,&lmaxnperbucket,&lno,
                         &rbucketlimit,&rmaxnperbucket,&rno);

    cbucket = btreeBucketNew(cache->nperbucket);

    count = 0;
    iold = 0;

    for(i=0;i<lbucketlimit;++i)
    {
	cbucket->Nentries = 0;

	for(j=0;j<lmaxnperbucket;++j)
	{
	    ajListPop(idlist,(void **)&bid);
	    
	    cid = cbucket->Ids[j];
	    ajStrAssignS(&cid->id,bid->id);
	    cid->dbno = bid->dbno;
	    cid->dups = bid->dups;
	    cid->offset = bid->offset;
	    cid->refoffset = bid->refoffset;
	    
	    cbucket->keylen[j] = BT_BUCKIDLEN(bid->id);
	    ++count;
	    ++cbucket->Nentries;
	    ajBtreeIdDel(&bid);
	}

	ajListPeek(idlist,(void **)&bid);
	
	ajStrAssignS(&newkarray[i],bid->id);

	if((iold < order) && parray[iold])
            newparray[i] = parray[iold++];
        else
	    newparray[i] = cache->totsize;

	btreeWriteBucket(cache,cbucket,newparray[i]);
    }

    cbucket->Nentries = 0;

    j = 0;

    while(count != lno)
    {
	ajListPop(idlist,(void **)&bid);
	cid = cbucket->Ids[j];
	++j;
	++count;

	ajStrAssignS(&cid->id,bid->id);
	cid->dbno = bid->dbno;
	cid->dups = bid->dups;
	cid->offset = bid->offset;
	cid->refoffset = bid->refoffset;
	
	++cbucket->Nentries;
	ajBtreeIdDel(&bid);
    }

    if((iold < order) && parray[iold])
        newparray[i] = parray[iold++];
    else
        newparray[i] = cache->totsize;
    btreeWriteBucket(cache,cbucket,newparray[i]);

    nkeys = lbucketlimit;
    nodetype = BT_LEAF;
    v = nodetype;
    SBT_NODETYPE(lbuf,v);
    lpage->dirty = BT_DIRTY;

    GBT_PREV(lbuf,&prevsave);

    btreeWriteNode(cache,lpage,newkarray,newparray,nkeys);

    ajListPeek(idlist,(void **)&bid);
    ajStrAssignS(&mediankey,bid->id);

    for(i=0;i<rbucketlimit;++i)
    {
	cbucket->Nentries = 0;

	for(j=0;j<rmaxnperbucket;++j)
	{
	    ajListPop(idlist,(void **)&bid);
	    
	    cid = cbucket->Ids[j];
	    ajStrAssignS(&cid->id,bid->id);
	    cid->dbno = bid->dbno;
	    cid->dups = bid->dups;
	    cid->offset = bid->offset;
	    cid->refoffset = bid->refoffset;
	    
	    cbucket->keylen[j] = BT_BUCKIDLEN(bid->id);
	    ++cbucket->Nentries;
	    ajBtreeIdDel(&bid);
	}

	ajListPeek(idlist,(void **)&bid);
	ajStrAssignS(&newkarray[i],bid->id);

        if((iold < order) && parray[iold])
            newparray[i] = parray[iold++];
        else
            newparray[i] = cache->totsize;
	btreeWriteBucket(cache,cbucket,newparray[i]);
    }

    cbucket->Nentries = 0;

    j = 0;

    while(ajListPop(idlist,(void**)&bid))
    {
	cid = cbucket->Ids[j];
	++j;

	ajStrAssignS(&cid->id,bid->id);
	cid->dbno = bid->dbno;
	cid->dups = bid->dups;
	cid->offset = bid->offset;
	cid->refoffset = bid->refoffset;
	
	++cbucket->Nentries;
	ajBtreeIdDel(&bid);
    }
    
    if((iold < order) && parray[iold])
        newparray[i] = parray[iold++];
    else
        newparray[i] = cache->totsize;
    btreeWriteBucket(cache,cbucket,newparray[i]);

    nkeys = rbucketlimit;

    nodetype = BT_LEAF;
    v = nodetype;
    SBT_NODETYPE(rbuf,v);
    lv = prevsave;
    SBT_PREV(rbuf,lv);
    lv = overflow;
    SBT_OVERFLOW(rbuf,lv);

    btreeWriteNode(cache,rpage,newkarray,newparray,nkeys);
    rpage->dirty = BT_DIRTY;

    btreeBucketDel(&cbucket);
    ajListFree(&idlist);



    medianless = lblockno;
    mediangtr  = rblockno;

    btreeDeallocPriArray(cache,arrays1);
    btreeDeallocPriArray(cache,arrays2);

    if(rootnodetype == BT_ROOT)
    {
	btreeWriteNodeSingle(cache,spage,mediankey,lblockno,rblockno);	
	spage->dirty = BT_LOCK;
        spage->lockfor = 1153;

	ajStrDel(&mediankey);
	++cache->level;

	return spage;
    }


    page = ajBtreeCacheRead(cache,prevsave);
    btreeInsertKey(cache,page,mediankey,medianless,mediangtr);
    ajStrDel(&mediankey);

    page = ajBtreeCacheRead(cache,prevsave);

    return page;
}




/* @funcstatic btreeInsertShiftS ********************************************
**
** Rebalance buckets on insertion
**
** @param [u] cache [AjPBtcache] cache
** @param [u] retpage [AjPBtpage*] page
** @param [r] key [const AjPStr] key
**
** @return [ajulong] bucket block or 0L if shift not possible
** @@
******************************************************************************/

static ajulong btreeInsertShiftS(AjPBtcache cache, AjPBtpage *retpage,
                                 const AjPStr key)
{
    unsigned char *tbuf = NULL;
    unsigned char *pbuf = NULL;
    unsigned char *sbuf = NULL;

    AjPBtpage ppage = NULL;
    AjPBtpage spage = NULL;
    AjPBtpage tpage = NULL;

    ajuint tkeys = 0;
    ajuint pkeys = 0;
    ajuint skeys = 0;
    ajuint order = 0;
    
    ajint ii;
    ajuint i;
    ajuint n;
    
    ajulong parent  = 0L;
    ajulong blockno = 0L;
    
    AjPBtMem arrays1 = NULL;
    AjPBtMem arrays2 = NULL;
    AjPBtMem arrays3 = NULL;
    AjPStr *kTarray = NULL;
    AjPStr *kParray = NULL;
    AjPStr *kSarray = NULL;
    ajulong *pTarray = NULL;
    ajulong *pParray = NULL;
    ajulong *pSarray = NULL;

    AjPStr *karray = NULL;
    ajulong *parray = NULL;

    ajuint ppos    = 0;
    ajuint pkeypos = 0;
    ajuint minsize = 0;
    
    /* ajDebug("In btreeInsertShift\n"); */


    tpage = *retpage;

    tbuf = tpage->buf;

    GBT_PREV(tbuf,&parent);
    GBT_NKEYS(tbuf,&tkeys);

    order = cache->order;
    minsize = order / 2;

    if(order % 2)
	++minsize;

    if(tkeys <= minsize)
	return 0L;

    ppage = ajBtreeCacheRead(cache,parent);
    
    pbuf = ppage->buf;
    GBT_NKEYS(pbuf,&pkeys);
    
    
    arrays1 = btreeAllocPriArray(cache);
    kParray = arrays1->karray;
    pParray = arrays1->parray;

    arrays2 = btreeAllocPriArray(cache);
    kSarray = arrays2->karray;
    pSarray = arrays2->parray;

    arrays3 = btreeAllocPriArray(cache);
    kTarray = arrays3->karray;
    pTarray = arrays3->parray;
    
    btreeGetKeys(cache,pbuf,&kParray,&pParray);

    i=0;

    while(i!=pkeys && MAJSTRCMPS(kParray[i],key)<=0)
	++i;

    pkeypos = i;
    
    if(i==pkeys)
    {
	if(MAJSTRCMPS(kParray[i-1],key)>0)
	    ppos = i-1;
	else
	    ppos = i;
    }
    else
	ppos = i;

    
    if(ppos) /* There is another leaf to the left */
    {
	spage = ajBtreeCacheRead(cache,pParray[ppos-1]);
	sbuf = spage->buf;
	GBT_NKEYS(sbuf,&skeys);
    }

    if(i && skeys != order-1) /* There is space in the left leaf */
    {
	/* ajDebug("Left shift\n"); */
	btreeGetKeys(cache,tbuf,&kTarray,&pTarray);
	if(skeys)
	    btreeGetKeys(cache,sbuf,&kSarray,&pSarray);

	i = 0;

	while(pParray[i] != tpage->pagepos)
	    ++i;

	--i;

	pkeypos = i;

	ajStrAssignS(&kSarray[skeys],kParray[pkeypos]);
	pSarray[skeys+1] = pTarray[0];
	++skeys;
	--tkeys;
	ajStrAssignS(&kParray[pkeypos],kTarray[0]);

	for(i=0;i<tkeys;++i)
	{
	    ajStrAssignS(&kTarray[i],kTarray[i+1]);
	    pTarray[i] = pTarray[i+1];
	}

	pTarray[i] = pTarray[i+1];
	pTarray[i+1] = 0L;
	
	btreeWriteNode(cache,spage,kSarray,pSarray,skeys);
	btreeWriteNode(cache,tpage,kTarray,pTarray,tkeys);
	btreeWriteNode(cache,ppage,kParray,pParray,pkeys);

	if(!ppage->pagepos)
        {
	    ppage->dirty = BT_LOCK;
            ppage->lockfor = 1161;
        }

	i = 0;

	while(i!=pkeys && MAJSTRCMPS(kParray[i],key)<=0)
	    ++i;

	if(i==pkeys)
	{
	    if(MAJSTRCMPS(kParray[i-1],key)>0)
		blockno = pParray[i-1];
	    else
		blockno = pParray[i];
	}
	else
	    blockno = pParray[i];

	if(blockno == spage->pagepos)
	{
	    *retpage = spage;
	    karray = kSarray;
	    parray = pSarray;
	    n = skeys;
	}
	else
	{
	    karray = kTarray;
	    parray = pTarray;
	    n = tkeys;
	}
	

	i = 0;

	while(i!=n && MAJSTRCMPS(karray[i],key)<=0)
	    ++i;

	if(i==n)
	{
	    if(MAJSTRCMPS(karray[i-1],key)>0)
		blockno = parray[i-1];
	    else
		blockno = parray[i];
	}
	else
	    blockno = parray[i];

        btreeDeallocPriArray(cache,arrays1);
        btreeDeallocPriArray(cache,arrays2);
        btreeDeallocPriArray(cache,arrays3);

	/* ajDebug("... returns blockno (a) %Lu\n",blockno); */

	return blockno;
    }
    

    if(ppos != pkeys)	/* There is a right node */
    {
	spage = ajBtreeCacheRead(cache,pParray[ppos+1]);
	sbuf = spage->buf;
	GBT_NKEYS(sbuf,&skeys);
    }


    /* Space in the right leaf */
    if(ppos != pkeys && skeys != order-1)
    {
	/* ajDebug("Right shift\n"); */
	btreeGetKeys(cache,tbuf,&kTarray,&pTarray);
	btreeGetKeys(cache,sbuf,&kSarray,&pSarray);

	i = 0;

	while(pParray[i] != tpage->pagepos)
	    ++i;

	pkeypos = i;
	
	pSarray[skeys+1] = pSarray[skeys];
	for(ii=skeys-1;ii>-1;--ii)
	{
	    ajStrAssignS(&kSarray[ii+1],kSarray[ii]);
	    pSarray[ii+1] = pSarray[ii];
	}
	ajStrAssignS(&kSarray[0],kParray[pkeypos]);
	pSarray[0] = pTarray[tkeys];
	ajStrAssignS(&kParray[pkeypos],kTarray[tkeys-1]);
	++skeys;
	--tkeys;
	pTarray[tkeys+1] = 0L;
	
	btreeWriteNode(cache,spage,kSarray,pSarray,skeys);
	btreeWriteNode(cache,tpage,kTarray,pTarray,tkeys);
	btreeWriteNode(cache,ppage,kParray,pParray,pkeys);
	if(!ppage->pagepos)
        {
	    ppage->dirty = BT_LOCK;
            ppage->lockfor = 1162;
        }

	i = 0;

	while(i!=pkeys && MAJSTRCMPS(kParray[i],key)<=0)
	    ++i;

	if(i==pkeys)
	{
	    if(MAJSTRCMPS(kParray[i-1],key)>0)
		blockno = pParray[i-1];
	    else
		blockno = pParray[i];
	}
	else
	    blockno = pParray[i];

	if(blockno == spage->pagepos)
	{
	    *retpage = spage;
	    karray = kSarray;
	    parray = pSarray;
	    n = skeys;
	}
	else
	{
	    karray = kTarray;
	    parray = pTarray;
	    n = tkeys;
	}
	
	i = 0;

	while(i!=n && MAJSTRCMPS(karray[i],key)<=0)
	    ++i;

	if(i==n)
	{
	    if(MAJSTRCMPS(karray[i-1],key)>0)
		blockno = parray[i-1];
	    else
		blockno = parray[i];
	}
	else
	    blockno = parray[i];

        btreeDeallocPriArray(cache,arrays1);
        btreeDeallocPriArray(cache,arrays2);
        btreeDeallocPriArray(cache,arrays3);

	/* ajDebug("... returns blockno (b) %Lu\n",blockno); */
	
	return blockno;
    }


    btreeDeallocPriArray(cache,arrays1);
    btreeDeallocPriArray(cache,arrays2);
    btreeDeallocPriArray(cache,arrays3);

    /* ajDebug("... returns 0L\n"); */

    return 0L;
}




/* @funcstatic btreeKeyShift ********************************************
**
** Rebalance Nodes on insertion
**
** @param [u] cache [AjPBtcache] cache
** @param [u] tpage [AjPBtpage] page
**
** @return [void]
** @@
******************************************************************************/

static void btreeKeyShift(AjPBtcache cache, AjPBtpage tpage)
{
    unsigned char *tbuf = NULL;
    unsigned char *pbuf = NULL;
    unsigned char *sbuf = NULL;
    unsigned char *buf  = NULL;

    AjPBtpage ppage = NULL;
    AjPBtpage spage = NULL;
    AjPBtpage page  = NULL;

    ajuint tkeys = 0;
    ajuint pkeys = 0;
    ajuint skeys = 0;
    ajuint order = 0;

    ajuint i;
    ajint  ii;

    ajulong parent  = 0L;
    
    AjPBtMem Parrays = NULL;
    AjPBtMem Sarrays = NULL;
    AjPBtMem Tarrays = NULL;
    AjPStr *kTarray = NULL;
    AjPStr *kParray = NULL;
    AjPStr *kSarray = NULL;
    ajulong *pTarray = NULL;
    ajulong *pParray = NULL;
    ajulong *pSarray = NULL;

    ajuint pkeypos = 0;
    ajuint minsize = 0;

    ajulong lv = 0L;
    
    /* ajDebug("In btreeKeyShift\n"); */
    
    tbuf = tpage->buf;

    GBT_PREV(tbuf,&parent);
    GBT_NKEYS(tbuf,&tkeys);

    order = cache->order;
    minsize = order / 2;

    if(order % 2)
	++minsize;

    if(tkeys <= minsize)
	return;

    
    ppage = ajBtreeCacheRead(cache,parent);
    pbuf = ppage->buf;
    GBT_NKEYS(pbuf,&pkeys);

    Parrays = btreeAllocPriArray(cache);
    Sarrays = btreeAllocPriArray(cache);
    Tarrays = btreeAllocPriArray(cache);
    kParray = Parrays->karray;
    pParray = Parrays->parray;
    kSarray = Sarrays->karray;
    pSarray = Sarrays->parray;
    kTarray = Tarrays->karray;
    pTarray = Tarrays->parray;
    
    btreeGetKeys(cache,tbuf,&kTarray,&pTarray);
    GBT_NKEYS(tbuf,&tkeys);

    btreeGetKeys(cache,pbuf,&kParray,&pParray);

    i=0;

    while(pParray[i] != tpage->pagepos)
	++i;

    if(i) /* There is another leaf to the left */
    {
	pkeypos = i-1;
	spage = ajBtreeCacheRead(cache,pParray[pkeypos]);
	sbuf = spage->buf;
	GBT_NKEYS(sbuf,&skeys);
	
    }

    if(i && skeys != order-1) /* There is space in the left leaf */
    {
	if(skeys)
	    btreeGetKeys(cache,sbuf,&kSarray,&pSarray);

	ajStrAssignS(&kSarray[skeys],kParray[pkeypos]);
	pSarray[skeys+1] = pTarray[0];
	++skeys;
	--tkeys;
	ajStrAssignS(&kParray[pkeypos],kTarray[0]);

	for(i=0;i<tkeys;++i)
	{
	    ajStrAssignS(&kTarray[i],kTarray[i+1]);
	    pTarray[i] = pTarray[i+1];
	}

	pTarray[i] = pTarray[i+1];
	pTarray[i+1] = 0L;
	
	btreeWriteNode(cache,spage,kSarray,pSarray,skeys);
	btreeWriteNode(cache,tpage,kTarray,pTarray,tkeys);
	btreeWriteNode(cache,ppage,kParray,pParray,pkeys);
	if(!ppage->pagepos)
        {
	    ppage->dirty = BT_LOCK;
            ppage->lockfor = 1171;
        }

	page = ajBtreeCacheRead(cache,pSarray[skeys]);
	buf = page->buf;
	lv = spage->pagepos;
	SBT_PREV(buf,lv);
	page->dirty = BT_DIRTY;

        btreeDeallocPriArray(cache,Parrays);
        btreeDeallocPriArray(cache,Sarrays);
        btreeDeallocPriArray(cache,Tarrays);

	return;
    }



    if(i != pkeys)	/* There is a right node */
    {
	pkeypos = i;
	spage = ajBtreeCacheRead(cache,pParray[pkeypos+1]);
	sbuf = spage->buf;
	GBT_NKEYS(sbuf,&skeys);
    }


    if(i != pkeys && skeys != order-1) /* Space in the right node */
    {
	if(skeys)
	    btreeGetKeys(cache,sbuf,&kSarray,&pSarray);

	pSarray[skeys+1] = pSarray[skeys];

	for(ii=skeys-1;ii>-1;--ii)
	{
	    ajStrAssignS(&kSarray[ii+1],kSarray[ii]);
	    pSarray[ii+1] = pSarray[ii];
	}

	ajStrAssignS(&kSarray[0],kParray[pkeypos]);
	pSarray[0] = pTarray[tkeys];
	ajStrAssignS(&kParray[pkeypos],kTarray[tkeys-1]);
	++skeys;
	--tkeys;
	pTarray[tkeys+1] = 0L;
	
	btreeWriteNode(cache,spage,kSarray,pSarray,skeys);
	btreeWriteNode(cache,tpage,kTarray,pTarray,tkeys);
	btreeWriteNode(cache,ppage,kParray,pParray,pkeys);
	if(!ppage->pagepos)
        {
	    ppage->dirty = BT_LOCK;
            ppage->lockfor = 1172;
        }
        
	page = ajBtreeCacheRead(cache,pSarray[0]);
	buf = page->buf;
	lv = spage->pagepos;
	SBT_PREV(buf,lv);
	page->dirty = BT_DIRTY;

        btreeDeallocPriArray(cache,Parrays);
        btreeDeallocPriArray(cache,Sarrays);
        btreeDeallocPriArray(cache,Tarrays);
        
	return;
    }

    btreeDeallocPriArray(cache,Parrays);
    btreeDeallocPriArray(cache,Sarrays);
    btreeDeallocPriArray(cache,Tarrays);

    return;
}




#if 0
/* @funcstatic btreeTraverseLeaves *****************************************
**
** Find the next leaf by traversing the tree
**
** @param [u] cache [AjPBtcache] cache
** @param [u] thys [AjPBtpage] current leaf page
**
** @return [AjPBtpage] next leaf or NULL
** @@
******************************************************************************/

static AjPBtpage btreeTraverseLeaves(AjPBtcache cache, AjPBtpage thys)
{
    AjPBtpage page = NULL;
    
    ajulong pagepos = 0L;
    ajulong prev   = 0L;
    
    AjPBtMem arrays = NULL;
    ajulong *parray = NULL;

    ajuint nodetype = 0;
    ajuint nkeys    = 0;
    ajuint apos     = 0;
    ajuint order    = 0;
    ajuint i;

    unsigned char *buf = NULL;    

    if(!cache->level)
	return NULL;

    order = cache->order;
    arrays = btreeAllocPriArray(cache);
    parray = arrays->Parray;

    pagepos = thys->pagepos;
    buf = thys->buf;
    GBT_PREV(buf,&prev);

    page = ajBtreeCacheRead(cache,prev);
    buf = page->buf;
    GBT_NKEYS(buf,&nkeys);
    GBT_NODETYPE(buf,&nodetype);
    btreeGetPointers(cache,buf,&parray);

    apos = 0;

    while(parray[apos] != pagepos)
	++apos;

    while(apos == nkeys)
    {
	if(nodetype == BT_ROOT)
	{
            btreeDeallocPriArray(cache,arrays);

	    return NULL;
	}

	GBT_PREV(buf,&prev);
	pagepos = page->pagepos;
	page = ajBtreeCacheRead(cache,prev);
	buf = page->buf;
	GBT_NKEYS(buf,&nkeys);
	GBT_NODETYPE(buf,&nodetype);
	btreeGetPointers(cache,buf,&parray);

	apos = 0;

	while(parray[apos] != pagepos)
	    ++apos;
    }

    page = ajBtreeCacheRead(cache,parray[apos+1]);
    buf = page->buf;
    GBT_NODETYPE(buf,&nodetype);
    btreeGetPointers(cache,buf,&parray);
    
    while(nodetype != BT_LEAF)
    {
	page = ajBtreeCacheRead(cache,parray[0]);
	buf = page->buf;
	btreeGetPointers(cache,buf,&parray);
	GBT_NODETYPE(buf,&nodetype);
    }

    btreeDeallocPriArray(cache,arrays);

    return page;
}
#endif




#if 0
/* @funcstatic btreeJoinLeaves ***********************************************
**
** Update all Left/Right Leaf Pointers
**
** @param [u] cache [AjPBtcache] cache
**
** @return [void] next leaf or NULL
** @@
******************************************************************************/

static void btreeJoinLeaves(AjPBtcache cache)
{
    unsigned char *buf = NULL;
    AjPBtpage page     = NULL;
    AjPBtpage newpage  = NULL;
    
    ajuint nodetype = 0;
    ajuint order    = 0;
    ajuint i;

    AjPBtMem arrays = NULL;
    ajulong *parray = NULL;

    ajulong left    = 0L;
    ajulong right   = 0L;

    ajulong lv = 0L;
    
    if(!cache->level)
	return;

    order = cache->order;
    arrays = btreeAllocPriArray(cache);
    parray = arrays->Parray;

    page = btreeCacheLocate(cache,0L);
    buf = page->buf;
    btreeGetPointers(cache,buf,&parray);
    GBT_NODETYPE(buf,&nodetype);

    while(nodetype != BT_LEAF)
    {
	page = ajBtreeCacheRead(cache,parray[0]);
	buf = page->buf;
	btreeGetKeys(cache,buf,&karray,&parray);
	GBT_NODETYPE(buf,&nodetype);
    }
    
    lv = left;
    SBT_LEFT(buf,lv);
    
    while((newpage = btreeTraverseLeaves(cache,page)))
    {
	right = newpage->pagepos;
	lv = right;
	SBT_RIGHT(buf,lv);
	page->dirty = BT_DIRTY;
	left = page->pagepos;
	buf = newpage->buf;
	lv = left;
	SBT_LEFT(buf,lv);
	page = newpage;
    }

    right = 0L;
    SBT_RIGHT(buf,right);
    page->dirty = BT_DIRTY;

    btreeDeallocPriArray(cache,arrays);

    return;
}
#endif




/* @func ajBtreeWildNew *********************************************
**
** Construct a wildcard search object
**
** @param [u] cache [AjPBtcache] cache
** @param [r] wild [const AjPStr] wildcard id prefix (without asterisk)
**
** @return [AjPBtWild] b+ tree wildcard object
** @@
******************************************************************************/

AjPBtWild ajBtreeWildNew(AjPBtcache cache, const AjPStr wild)
{
    AjPBtWild thys = NULL;

    (void) cache;			/* make it used */

    AJNEW0(thys);

    thys->id   = ajStrNewS(wild);
    ajStrTrimC(&thys->id,"*"); /* Need to revisit this */
    thys->list = ajListNew();

    thys->first = ajTrue;

    return thys;
}




/* @func ajBtreeWildDel *********************************************
**
** Destroy a wildcard search object
**
** @param [u] Pthis [AjPBtWild*] b+ tree wildcard structure
**
** @return [void]
** @@
******************************************************************************/

void ajBtreeWildDel(AjPBtWild *Pthis)
{
    AjPBtWild thys = NULL;
    AjPBtId   id    = NULL;
    
    if(!Pthis || !*Pthis)
	return;

    thys = *Pthis;

    ajStrDel(&thys->id);

    while(ajListPop(thys->list,(void **)&id))
	ajBtreeIdDel(&id);

    ajListFree(&thys->list);

    AJFREE(thys);
    *Pthis = NULL;

    return;
}




/* @func ajBtreeKeyWildNew *********************************************
**
** Construct a wildcard keyword search object
**
** @param [u] cache [AjPBtcache] cache
** @param [r] wild [const AjPStr] wildcard keyword prefix (without asterisk)
**
** @return [AjPBtKeyWild] b+ tree wildcard object
** @@
******************************************************************************/

AjPBtKeyWild ajBtreeKeyWildNew(AjPBtcache cache, const AjPStr wild)
{
    AjPBtKeyWild thys = NULL;

    (void) cache;			/* make it used */

    AJNEW0(thys);

    thys->keyword = ajStrNewS(wild);
    ajStrTrimC(&thys->keyword,"*"); /* Need to revisit this */

    thys->list   = ajListNew();
    thys->idlist = ajListNew();

    thys->first = ajTrue;

    return thys;
}




/* @func ajBtreeKeyWildDel *********************************************
**
** Destroy a wildcard keyword search object
**
** @param [u] Pthis [AjPBtKeyWild*] b+ tree wildcard keyword structure
**
** @return [void]
** @@
******************************************************************************/

void ajBtreeKeyWildDel(AjPBtKeyWild *Pthis)
{
    AjPBtKeyWild thys = NULL;
    AjPStr id = NULL;
    AjPBtPri pri = NULL;
    
    if(!Pthis || !*Pthis)
	return;

    thys = *Pthis;

    ajStrDel(&thys->keyword);

    while(ajListPop(thys->idlist,(void **)&id))
	ajStrDel(&id);

    while(ajListPop(thys->list,(void **)&pri))
	ajBtreePriDel(&pri);

    ajListFree(&thys->list);

    AJFREE(thys);
    *Pthis = NULL;

    return;
}




/* @func ajBtreeFindInsertW ***********************************************
**
** Find the node that should contain a key (wild)
**
** @param [u] cache [AjPBtcache] cache
** @param [r] key [const AjPStr] key to search for 
**
** @return [AjPBtpage] leaf node where item should be inserted
** @@
******************************************************************************/

AjPBtpage ajBtreeFindInsertW(AjPBtcache cache, const AjPStr key)
{
    AjPBtpage root = NULL;
    AjPBtpage ret  = NULL;

    /* ajDebug("In ajBtreeFindInsertW\n"); */

    /* The root node should always be in the cache (BT_LOCKed) */
    root = btreeCacheLocate(cache,0L);
    
    if(!cache->level)
	return root;
    
    ret = btreeFindINodeW(cache,root,key);

    return ret;
}




/* @funcstatic btreeFindINodeW ************************************************
**
** Recursive search for node (wild)
**
** @param [u] cache [AjPBtcache] cache
** @param [u] page [AjPBtpage] page
** @param [r] item [const AjPStr] key to search for 
**
** @return [AjPBtpage] leaf node where item should be inserted
** @@
******************************************************************************/

static AjPBtpage btreeFindINodeW(AjPBtcache cache, AjPBtpage page,
				  const AjPStr item)
{
    AjPBtpage ret = NULL;
    AjPBtpage pg  = NULL;

    unsigned char *buf = NULL;

    ajuint status = 0;
    ajuint ival   = 0;

    /* ajDebug("In btreeFindINodeW\n"); */
    
    ret = page;
    buf = page->buf;
    GBT_NODETYPE(buf,&ival);

    if(ival != BT_LEAF)
    {
	status = ret->dirty;
	ret->dirty = BT_LOCK;	/* Lock in case of lots of overflow pages */
        ret->lockfor = 1181;
	pg = btreePageFromKeyW(cache,buf,item);
	ret->dirty = status;
	ret = btreeFindINodeW(cache,pg,item);
    }
    
    return ret;
}




/* @funcstatic btreePageFromKeyW *******************************************
**
** Return next lower index page given a key (wild)
**
** @param [u] cache [AjPBtcache] cache
** @param [u] buf [unsigned char *] page buffer
** @param [r] key [const AjPStr] key to search for 
**
** @return [AjPBtpage] pointer to a page
** @@
******************************************************************************/

static AjPBtpage btreePageFromKeyW(AjPBtcache cache, unsigned char *buf,
				    const AjPStr key)
{
    ajulong blockno = 0L;
    AjPBtpage page = NULL;

    unsigned char *rootbuf = NULL;
    
    /* ajDebug("In btreePageFromKeyW\n"); */
    
    rootbuf = buf;

    blockno = btreeGetBlockS(cache,rootbuf,key);
    page =  ajBtreeCacheRead(cache,blockno);

    return page;
}




/* @funcstatic btreeReadLeaf ***********************************************
**
** Read all leaf Ids into a list
**
** @param [u] cache [AjPBtcache] cache
** @param [u] page [AjPBtpage] page
** @param [w] list [AjPList] list
**
** @return [void]
** @@
******************************************************************************/

static void btreeReadLeaf(AjPBtcache cache, AjPBtpage page, AjPList list)
{
    unsigned char *buf = NULL;
    AjPBtMem arrays    = NULL;
    ajulong *parray     = NULL;

    ajuint keylimit = 0;
    ajuint nkeys    = 0;

    ajuint i;
    
    /* ajDebug("In ReadLeaf\n"); */
    
    buf = page->buf;

    GBT_NKEYS(buf,&nkeys);

    if(!nkeys)
        return;

    arrays = btreeAllocPriArray(cache);
    parray = arrays->parray;

    btreeGetPointers(cache,buf,&parray);

    GBT_NKEYS(buf,&nkeys);
    
    keylimit = nkeys+1;

    for(i=0;i<keylimit;++i)
	btreeHybBucketIdlistAll(cache,parray[i],list);

    ajListSort(list,btreeIdCompare);

    btreeDeallocPriArray(cache,arrays);

    return;
}




/* @func ajBtreeIdFromKeyW ********************************************
**
** Wildcard retrieval of entries
**
** @param [u] cache [AjPBtcache] cache
** @param [u] wild [AjPBtWild] Wildcard
**
** @return [AjPBtId] next matching Id or NULL
** @@
******************************************************************************/

AjPBtId ajBtreeIdFromKeyW(AjPBtcache cache, AjPBtWild wild)
{

    AjPBtId id     = NULL;
    AjPBtpage page = NULL;
    const AjPStr key = NULL;
    AjPList list   = NULL;
    AjBool found   = ajFalse;

    ajulong pagepos = 0L;

    unsigned char *buf = NULL;    

    key  = wild->id;
    list = wild->list;

    found = ajFalse;
    
    if(wild->first)
    {
	page = ajBtreeFindInsertW(cache,key);
	page->dirty = BT_LOCK;
        page->lockfor = 1191;
	wild->pagepos = page->pagepos;
	
	btreeReadLeaf(cache,page,list);

	page->dirty = BT_CLEAN;
	
	if(!ajListGetLength(list))
	    return NULL;

	while(ajListPop(list,(void **)&id))
	{
	    if(ajStrPrefixS(id->id,key))
	    {
		found = ajTrue;
		break;
	    }

	    ajBtreeIdDel(&id);
	}

	wild->first = ajFalse;


	if(!found)	/* Check the next leaf just in case key==internal */
	{
	    buf = page->buf;
	    GBT_RIGHT(buf,&pagepos);

	    if(!pagepos)
		return NULL;

	    page = ajBtreeCacheRead(cache,pagepos);
	    wild->pagepos = pagepos;
	    page->dirty = BT_LOCK;
            page->lockfor = 1192;

	    btreeReadLeaf(cache,page,list);	
	    
	    page->dirty = BT_CLEAN;
	    
	    if(!ajListGetLength(list))
		return NULL;
	    
	    found = ajFalse;

	    while(ajListPop(list,(void **)&id))
	    {
		if(ajStrPrefixS(id->id,key))
		{
		    found = ajTrue;
		    break;
		}

		ajBtreeIdDel(&id);
	    }
	}


	if(!found)
	    return NULL;

	return id;
    }


    if(!ajListGetLength(list))
    {
	page = ajBtreeCacheRead(cache,wild->pagepos); 
	buf = page->buf;
	GBT_RIGHT(buf,&pagepos);

	if(!pagepos)
	    return NULL;

	page = ajBtreeCacheRead(cache,pagepos);
	wild->pagepos = pagepos;
	page->dirty = BT_LOCK;
        page->lockfor = 1193;

	btreeReadLeaf(cache,page,list);	

	page->dirty = BT_CLEAN;
	
	if(!ajListGetLength(list))
	    return NULL;
    }
    


    while(ajListPop(list,(void **)&id))
    {
	if(ajStrPrefixS(id->id,key))
	{
	    found = ajTrue;
	    break;
	}

	ajBtreeIdDel(&id);
    }
    
    
    if(!found)
	return NULL;
    
    return id;
}




/* @func ajBtreeListFromKeyW **************************************************
**
** Wildcard retrieval of entries
**
** @param [u] cache [AjPBtcache] cache
** @param [r] key [const AjPStr] Wildcard key
** @param [u] idlist [AjPList] list of matching AjPBtIds
**
** @return [void]
** @@
******************************************************************************/

void ajBtreeListFromKeyW(AjPBtcache cache, const AjPStr key, AjPList idlist)
{

    AjPBtId id     = NULL;
    AjPBtpage page = NULL;
    AjPList list   = NULL;
    AjBool found   = ajFalse;

    ajulong pripagepos = 0L;
    ajulong right = 0L;
    
    unsigned char *buf = NULL;    
    AjBool finished = ajFalse;

    AjPStr prefix = NULL;
    AjPStr keystr = NULL;
    
    char *p;
    char *cp;

    ajDebug("ajBtreeListFromKeyW '%S' list: %u\n", key, ajListGetLength(list));

    if(!cache->countunique && !cache->countall)
        return;

    keystr = ajStrNewS(key);
    ajStrFmtQuery(&keystr);

    if(MAJSTRGETLEN(keystr) > cache->kwlimit)
        ajStrTruncateLen(&keystr, cache->kwlimit);
    
    cp = MAJSTRGETPTR(keystr);
    p = strpbrk(cp,"*?");

    if(p)
    {
	if(p-cp)
	    ajStrAssignSubS(&prefix,keystr,0,p-cp-1);
	else
	{
	    btreeKeyFullSearch(cache,keystr,idlist);
            ajStrDel(&keystr);
	    ajStrDel(&prefix);
	    return;
	}
    }
    else
	ajStrAssignS(&prefix,keystr);

    ajStrFmtQuery(&prefix);
    if(MAJSTRGETLEN(prefix) > cache->kwlimit)
        ajStrTruncateLen(&prefix, cache->kwlimit);

    list = ajListNew();

    found = ajFalse;

    page = ajBtreeFindInsertW(cache,prefix);
    page->dirty = BT_LOCK;
    page->lockfor = 1201;
    pripagepos = page->pagepos;
    
    btreeReadLeaf(cache,page,list);
    page->dirty = BT_CLEAN;
    

    while(ajListPop(list,(void **)&id))
    {
	if(ajStrPrefixS(id->id,prefix))
	{
	    found = ajTrue;
	    break;
	}
	else
	    ajBtreeIdDel(&id);
    }
    
    
    if(!found)	/* Check the next leaf just in case key==internal */
    {
	buf = page->buf;
	GBT_RIGHT(buf,&right);

	if(!right)
	{
            ajStrDel(&keystr);
	    ajStrDel(&prefix);
	    ajListFree(&list);

	    return;
	}

	page = ajBtreeCacheRead(cache,right);
	pripagepos = right;
	page->dirty = BT_LOCK;
        page->lockfor = 1202;

	btreeReadLeaf(cache,page,list);	
	    
	page->dirty = BT_CLEAN;
	    
	if(!ajListGetLength(list))
	{
            ajStrDel(&keystr);
	    ajStrDel(&prefix);
	    ajListFree(&list);

	    return;
	}
	
	    
	found = ajFalse;

	while(ajListPop(list,(void **)&id))
	{
	    if(ajStrPrefixS(id->id,prefix))
	    {
		found = ajTrue;
		break;
	    }
	    else
		ajBtreeIdDel(&id);
	}
    }
    
    
    if(!found)
    {
        ajStrDel(&keystr);
	ajStrDel(&prefix);
	ajListFree(&list);

	return;
    }
    

    finished = ajFalse;

    
    while(!finished)
    {
	if(ajStrMatchWildS(id->id,keystr))
        {
	    ajListPush(idlist,(void *)id);
            id = NULL;
        }
	else
	    ajBtreeIdDel(&id);
	
	if(!ajListGetLength(list))
	{
	    page = ajBtreeCacheRead(cache,pripagepos);
	    buf = page->buf;
	    GBT_RIGHT(buf,&right);

	    if(!right)
	    {
		finished = ajTrue;
		continue;
	    }

	    page = ajBtreeCacheRead(cache,right);
	    page->dirty = BT_LOCK;
            page->lockfor = 1203;
	    buf = page->buf;
	    pripagepos = right;
	    
	    btreeReadLeaf(cache,page,list);	
	    
	    page->dirty = BT_CLEAN;
	    
	    if(!ajListGetLength(list))
	    {
		finished = ajTrue;
		continue;
	    }
	}

	ajListPop(list,(void **)&id);

	if(!ajStrPrefixS(id->id,prefix))
	{
	    finished = ajTrue;
	    ajBtreeIdDel(&id);
	}
    }
    
    while(ajListPop(list,(void **)&id))
	ajBtreeIdDel(&id);
    ajListFree(&list);

    ajListSortTwoUnique(idlist,btreeDbnoCompare,btreeOffsetCompare,
                        btreeIdDelFromList);

    ajStrDel(&prefix);
    ajStrDel(&keystr);

    return;
}




/* @funcstatic btreeKeyFullSearch ********************************************
**
** Wildcard retrieval of id/acc/sv entries. Whole index scan. Only used for
** wildcard searches with keys beginning with '?' or '*'
**
** @param [u] cache [AjPBtcache] cache
** @param [r] key [const AjPStr] Wildcard key
** @param [u] idlist [AjPList] list of matching AjPBtIds
**
** @return [void]
** @@
******************************************************************************/

static void btreeKeyFullSearch(AjPBtcache cache, const AjPStr key,
			       AjPList idlist)
{
    AjPBtId id     = NULL;
    AjPBtpage root = NULL;
    AjPBtpage page = NULL;
    ajulong right   = 0L;
    ajuint nodetype = 0;
    
    AjPBtMem arrays = NULL;
    ajulong *parray = NULL;

    AjPList list   = NULL;
    
    unsigned char *buf = NULL;    

    list   = ajListNew();

    root = btreeCacheLocate(cache, 0L);
    page = root;
    
    buf = root->buf;
    GBT_NODETYPE(buf,&nodetype);
    
    if(cache->level)
    {
        arrays = btreeAllocPriArray(cache);
        parray = arrays->parray;
	while(nodetype != BT_LEAF)
	{
	    btreeGetPointers(cache,buf,&parray);
	    page = ajBtreeCacheRead(cache, parray[0]);
	    buf = page->buf;
	    GBT_NODETYPE(buf,&nodetype);
	    page->dirty = BT_CLEAN;
	}

        btreeDeallocPriArray(cache,arrays);
    }

    right = 99L;

    while(right)
    {
	btreeReadLeaf(cache,page,list);

	while(ajListPop(list,(void **)&id))
	{
	    if(ajStrMatchWildS(id->id,key))
		ajListPushAppend(idlist,(void *)id);
	    else
		ajBtreeIdDel(&id);
	}

	GBT_RIGHT(buf,&right);

	if(right)
	{
	    page = ajBtreeCacheRead(cache,right);
	    buf = page->buf;
	}
    }
    

    ajListSortTwoUnique(idlist,btreeDbnoCompare,btreeOffsetCompare,
                        btreeIdDelFromList);

    ajListFree(&list);

    return;
}




/* @func ajBtreeReplaceId ************************************************
**
** Replace an ID structure in a leaf node given a key
**
** @param [u] cache [AjPBtcache] cache
** @param [r] rid [const AjPBtId] replacement id object
**
** @return [AjBool] true if success
** @@
******************************************************************************/

AjBool ajBtreeReplaceId(AjPBtcache cache, const AjPBtId rid)
{
    AjPBtpage page   = NULL;
    AjPBucket bucket = NULL;
    AjPBtId   id     = NULL;
    const AjPStr key = NULL;
    
    unsigned char *buf = NULL;

    ajuint nentries = 0;
    
    ajuint i;

    ajulong blockno = 0L;
    AjBool found   = ajFalse;


    key = rid->id;

    page = ajBtreeFindInsert(cache,key);
    buf = page->buf;

    blockno = btreeGetBlockS(cache,buf,key);
    bucket = btreeReadBucket(cache,blockno);
    
    nentries = bucket->Nentries;

    found = ajFalse;

    for(i=0;i<nentries;++i)
    {
	if(ajStrMatchS(key,bucket->Ids[i]->id))
	{
	    found = ajTrue;
	    break;
	}
    }

    if(found)
    {
	id->dbno = rid->dbno;
	id->dups = rid->dups;
	id->offset = rid->offset;
	id->refoffset = rid->refoffset;
	btreeWriteBucket(cache,bucket,blockno);
    }

    btreeBucketDel(&bucket);

    return ajTrue;
}




/* @func ajBtreeReadEntries ************************************************
**
** Read B+ tree entries from file
**
** @param [r] filename [const char*] file name
** @param [r] indexdir [const char*] index file directory
** @param [r] directory [const char*] file directory
** @param [w] seqfiles [AjPStr**] sequence file names
** @param [w] reffiles [AjPStr**] reference file names (if any)
**
** @return [ajuint] number of entries
** @@
******************************************************************************/

ajuint ajBtreeReadEntries(const char *filename, const char *indexdir,
                          const char *directory,
                          AjPStr **seqfiles, AjPStr **reffiles)
{
    AjPStr line = NULL;
    AjPStr fn   = NULL;
    
    AjPList list;
    AjPList reflist;

    AjPStr seqname = NULL;
    AjPStr refname = NULL;
    AjPStr tseqname = NULL;
    AjPStr trefname = NULL;
    
    AjPFile inf   = NULL;
    char p;
    ajuint entries = 0;
    
    AjBool do_ref = ajFalse;


    line    = ajStrNew();
    list    = ajListNew();
    reflist = ajListNew();

    tseqname = ajStrNew();
    trefname = ajStrNew();
    
    fn = ajStrNew();

    if(!*indexdir)
        ajFmtPrintS(&fn, "%s", filename);
    else
        ajFmtPrintS(&fn, "%s%s%s", indexdir, SLASH_STRING, filename);

    ajStrAppendC(&fn,".ent");

    inf = ajFileNewInNameS(fn);

    if(!inf)
	ajFatal("Cannot open database entries file %S",fn);

    while(ajReadlineTrim(inf, &line))
    {
	p = ajStrGetCharFirst(line);

	if(p == '#' || !ajStrGetLen(line))
	    continue;

	if(ajStrPrefixC(line,"Dual"))
	    do_ref = ajTrue;

	break;
    }
    

    if(!do_ref)
    {
	while(ajReadlineTrim(inf, &line))
	{
	    seqname = ajStrNew();
	    ajFmtScanS(line,"%S",&tseqname);
	    ajFmtPrintS(&seqname,"%s%s%S",directory,SLASH_STRING,tseqname);
	    ajListPushAppend(list,(void *)seqname);
	}

	ajListToarray(list,(void ***)&(*seqfiles));
	entries = ajListGetLength(list);
    }
    else
    {
	while(ajReadlineTrim(inf, &line))
	{
	    seqname = ajStrNew();
	    refname = ajStrNew();
	    ajFmtScanS(line,"%S%S",&tseqname,&trefname);
	    ajFmtPrintS(&seqname,"%s%s%S",directory,SLASH_STRING,tseqname);
	    ajFmtPrintS(&refname,"%s%s%S",directory,SLASH_STRING,trefname);
	    ajListPushAppend(list,(void *)seqname);
	    ajListPushAppend(reflist,(void *)refname);
	}

	ajListToarray(list,(void ***)&(*seqfiles));
	ajListToarray(reflist,(void ***)&(*reffiles));
	entries = ajListGetLength(list);
    }
    
    
    ajListFree(&list);
    ajListFree(&reflist);
    ajStrDel(&line);
    ajStrDel(&fn);

    ajStrDel(&tseqname);
    ajStrDel(&trefname);
    

    ajFileClose(&inf);

    return entries;
}




/* @func ajBtreeReadEntriesS **************************************************
**
** Read B+ tree entries from file
**
** @param [r] filename [const AjPStr] file name
** @param [r] indexdir [const AjPStr] index file directory
** @param [r] directory [const AjPStr] file directory
** @param [w] seqfiles [AjPStr**] sequence file names
** @param [w] reffiles [AjPStr**] reference file names (if any)

**
** @return [ajuint] number of entries
** @@
******************************************************************************/

ajuint ajBtreeReadEntriesS(const AjPStr filename, const AjPStr indexdir,
                           const AjPStr directory,
                           AjPStr **seqfiles, AjPStr **reffiles)
{
    AjPStr line = NULL;
    AjPStr fn   = NULL;
    
    AjPList list;
    AjPList reflist;

    AjPStr seqname = NULL;
    AjPStr refname = NULL;
    AjPStr tseqname = NULL;
    AjPStr trefname = NULL;
    
    AjPFile inf   = NULL;
    char p;
    ajuint entries = 0;
    
    AjBool do_ref = ajFalse;


    line    = ajStrNew();
    list    = ajListNew();
    reflist = ajListNew();

    tseqname = ajStrNew();
    trefname = ajStrNew();
    
    fn = ajStrNew();

    if(!ajStrGetLen(indexdir))
        ajFmtPrintS(&fn,"%S.ent",filename);
    else
        ajFmtPrintS(&fn,"%S%s%S.ent",indexdir,SLASH_STRING,filename);
    
    inf = ajFileNewInNameS(fn);

    if(!inf)
	ajFatal("Cannot open database entries file %S", fn);

    while(ajReadlineTrim(inf, &line))
    {
	p = ajStrGetCharFirst(line);

	if(p == '#' || !ajStrGetLen(line))
	    continue;

	if(ajStrPrefixC(line,"Dual"))
	    do_ref = ajTrue;

	break;
    }
    

    if(!do_ref)
    {
	while(ajReadlineTrim(inf, &line))
	{
	    seqname = ajStrNew();
	    ajFmtScanS(line,"%S",&tseqname);
	    ajFmtPrintS(&seqname,"%S%s%S",directory,SLASH_STRING,tseqname);
	    ajListPushAppend(list,(void *)seqname);
	}

	ajListToarray(list,(void ***)&(*seqfiles));
	entries = ajListGetLength(list);
    }
    else
    {
	while(ajReadlineTrim(inf, &line))
	{
	    seqname = ajStrNew();
	    refname = ajStrNew();
	    ajFmtScanS(line,"%S%S",&tseqname,&trefname);
	    ajFmtPrintS(&seqname,"%S%s%S",directory,SLASH_STRING,tseqname);
	    ajFmtPrintS(&refname,"%S%s%S",directory,SLASH_STRING,trefname);
	    ajListPushAppend(list,(void *)seqname);
	    ajListPushAppend(reflist,(void *)refname);
	}

	ajListToarray(list,(void ***)&(*seqfiles));
	ajListToarray(reflist,(void ***)&(*reffiles));
	entries = ajListGetLength(list);
    }
    
    
    ajListFree(&list);
    ajListFree(&reflist);
    ajStrDel(&line);
    ajStrDel(&fn);

    ajStrDel(&tseqname);
    ajStrDel(&trefname);
    

    ajFileClose(&inf);

    return entries;
}




/* @func ajBtreeInsertDupId ************************************************
**
** Get an ID structure from a leaf node given a key
**
** @param [u] cache [AjPBtcache] cache
** @param [u] id [AjPBtId] potentially duplicate id
**
** @return [void]
** @@
******************************************************************************/

void ajBtreeInsertDupId(AjPBtcache cache, AjPBtId id)
{
    AjPBtpage page   = NULL;
    AjPBucket bucket = NULL;
    AjPBtId   tid    = NULL;
    
    unsigned char *buf = NULL;

    ajuint nentries = 0;
    
    ajuint i;
    
    ajulong blockno = 0L;
    AjBool found   = ajFalse;

    AjPStr oldkey = NULL;

    page = ajBtreeFindInsert(cache,id->id);

    buf = page->buf;

    if(cache->countunique)
    {
	blockno = btreeGetBlockS(cache,buf,id->id);
	bucket = btreeReadBucket(cache,blockno);
    
	nentries = bucket->Nentries;

	found = ajFalse;

	for(i=0;i<nentries;++i)
	    if(ajStrMatchS(id->id,bucket->Ids[i]->id))
	    {
		found = ajTrue;
		break;
	    }

	if(found)
	{
	    oldkey = ajStrNewS(id->id);
	    tid = bucket->Ids[i];
	    ++tid->dups;
	    btreeWriteBucket(cache,bucket,blockno);
	    ajWarn("Dealing with a duplicate ID (%S)\n",id->id);
	    ajFmtPrintS(&id->id,"%S%c%u",oldkey,'\1',tid->dups);
	    ajStrDel(&oldkey);
	}

	btreeBucketDel(&bucket);
    }
    

    ajBtreeInsertId(cache,id);  /* increements counts */

    return;
}




/* @func ajBtreeDupFromKey ************************************************
**
** Write B+ tree parameters to file
**
** @param [u] cache [AjPBtcache] cache
** @param [r] key [const AjPStr] key
**
** @return [AjPList] list of matching AjPBtIds or NULL
** @@
******************************************************************************/

AjPList ajBtreeDupFromKey(AjPBtcache cache, const AjPStr key)
{
    AjPList list = NULL;
    AjPBtId id   = NULL;
    ajuint i;
    ajuint dups;
    
    AjPStr dupkey = NULL;
    AjPStr okey   = NULL;
    

    if(!(id = ajBtreeIdFromKey(cache,key)))
	return NULL;

    dupkey = ajStrNew();
    okey   = ajStrNew();
    list   = ajListNew();
    ajListPush(list,(void *)id);


    if(id->dups)
    {
	ajStrAssignS(&okey,id->id);
	dups = id->dups;

	for(i=0;i<dups;++i)
	{
	    ajFmtPrintS(&dupkey,"%S%c%u",okey,'\1',i+1);
	    id = ajBtreeIdFromKey(cache,dupkey);

	    if(!id)
		ajFatal("DupFromKey: Id not found\n");

	    ajListPushAppend(list,(void *)id);
	}
    }

    ajStrDel(&okey);
    ajStrDel(&dupkey);
    

    return list;
}




/* @funcstatic btreePriBucketNew *********************************************
**
** Construct a primary keyword bucket object
**
** @param [r] n [ajuint] Number of IDs
**
** @return [AjPPriBucket] initialised disc block cache structure
** @@
******************************************************************************/

static AjPPriBucket btreePriBucketNew(ajuint n)
{
    AjPPriBucket bucket = NULL;
    ajuint i;

    /*ajDebug("In btreePriBucketNew %u statsave: %u empty: %u\n",
      n, statSavePriBucketNext, statSavePriBucketEmptyNext);*/
    

    if(n)
    {
        if(statSavePriBucketNext)
        {
            bucket = statSavePriBucket[--statSavePriBucketNext];
            for(i=0;i<bucket->Maxentries;++i)
            {
                ajStrAssignClear(&bucket->codes[i]->id);
                ajStrAssignClear(&bucket->codes[i]->keyword);
                bucket->codes[i]->treeblock = 0L;
            }
            if(n > bucket->Maxentries)
            {
                AJCRESIZE0(bucket->keylen,bucket->Maxentries,n);
                AJCRESIZE0(bucket->codes,bucket->Maxentries,n);
                for(i=bucket->Maxentries;i<n;++i)
                    bucket->codes[i] = ajBtreePriNew();
                bucket->Maxentries = n;
            }
        }
        else
        {
            AJNEW0(bucket);
            AJCNEW0(bucket->codes,n);
            AJCNEW0(bucket->keylen,n);
            for(i=0;i<n;++i)
                bucket->codes[i] = ajBtreePriNew();
            bucket->Maxentries = n;
        }
        
    }
    else 
    {
        if(statSavePriBucketEmptyNext)
            bucket = statSavePriBucketEmpty[--statSavePriBucketEmptyNext];
        else
            AJNEW0(bucket);
    }
        
    bucket->NodeType = BT_PRIBUCKET;
    bucket->Nentries = n;
    bucket->Overflow = 0L;
    
    return bucket;
}




/* @funcstatic btreePriBucketDel *********************************************
**
** Delete a keyword primary bucket object
**
** @param [w] thys [AjPPriBucket*] bucket
**
** @return [void]
** @@
******************************************************************************/

static void btreePriBucketDel(AjPPriBucket *thys)
{
    AjPPriBucket pthis = NULL;
    ajuint newmax;
    
    if(!thys || !*thys)
	return;

    pthis = *thys;

    /*ajDebug("In btreePriBucketDel maxentries: %u savepribucket %u empty %u\n",
            pthis->Maxentries, statSavePriBucketNext,
            statSavePriBucketEmptyNext);*/

    if(!statSavePriBucket)
    {
        statSavePriBucketMax=2048;
        statSavePriBucketNext=0;
        AJCNEW0(statSavePriBucket,statSavePriBucketMax);
    }
    
    if(!statSavePriBucketEmpty)
    {
        statSavePriBucketEmptyMax=2048;
        statSavePriBucketEmptyNext=0;
        AJCNEW0(statSavePriBucketEmpty,statSavePriBucketEmptyMax);
    }
    
    if(pthis->Maxentries)
    {
        if(statSavePriBucketNext >= statSavePriBucketMax)
        {
            newmax = statSavePriBucketMax + statSavePriBucketMax;
            AJCRESIZE0(statSavePriBucket,statSavePriBucketMax,newmax);
            statSavePriBucketMax = newmax;
        }
        
        statSavePriBucket[statSavePriBucketNext++] = pthis;
    }
    else
    {
        if(statSavePriBucketEmptyNext >= statSavePriBucketEmptyMax)
        {
            newmax = statSavePriBucketEmptyMax + statSavePriBucketEmptyMax;
            AJCRESIZE0(statSavePriBucketEmpty,statSavePriBucketEmptyMax,newmax);
            statSavePriBucketEmptyMax = newmax;
        }
        statSavePriBucketEmpty[statSavePriBucketEmptyNext++] = pthis;
    }

    *thys = NULL;

    return;
}




/* @funcstatic btreePriBucketFree *********************************************
**
** Delete a keyword primary bucket object
**
** @param [w] thys [AjPPriBucket*] bucket
**
** @return [void]
** @@
******************************************************************************/

static void btreePriBucketFree(AjPPriBucket *thys)
{
    AjPPriBucket pthis = NULL;
    ajuint n;
    ajuint i;

    if(!thys || !*thys)
	return;

    pthis = *thys;
    n = pthis->Maxentries;

    /*ajDebug("In btreePriBucketFree %u\n",pthis->Maxentries);*/

    for(i=0;i<n;++i)
	btreePriFree(&pthis->codes[i]);
    
    AJFREE(pthis->keylen);
    AJFREE(pthis->codes);
    
    AJFREE(pthis);

    *thys = NULL;

    return;
}




/* @func ajBtreePriNew *********************************************
**
** Constructor for index bucket keyword information
**
**
** @return [AjPBtPri] Index ID object
** @@
******************************************************************************/

AjPBtPri ajBtreePriNew(void)
{
    AjPBtPri pri = NULL;

    /* ajDebug("In ajBtreePriNew\n"); */

    if(statSaveBtreePriNext)
    {
        pri = statSaveBtreePri[--statSaveBtreePriNext];
        MAJSTRASSIGNCLEAR(&pri->id);
        MAJSTRASSIGNCLEAR(&pri->keyword);
        pri->treeblock = 0L;
    }
    else
    {
        AJNEW0(pri);
        pri->keyword   = ajStrNew();
        pri->id        = ajStrNew();
        pri->treeblock = 0L;
    }
    
    return pri;
}




/* @func ajBtreePriDel *********************************************
**
** Destructor for keyword index primary bucket information
**
** @param [w] thys [AjPBtPri*] index keyword primary object
**
** @return [void]
** @@
******************************************************************************/

void ajBtreePriDel(AjPBtPri *thys)
{
    ajuint newmax;

    /* ajDebug("In ajBtreePriDel\n"); */

    if(!statSaveBtreePri)
    {
        statSaveBtreePriMax = 2048;
        AJCNEW0(statSaveBtreePri, statSaveBtreePriMax);
        statSaveBtreePriNext = 0;
    }

    if(!thys || !*thys)
	return;
    
    if(statSaveBtreePriNext >= statSaveBtreePriMax) 
    {
        newmax = statSaveBtreePriMax + statSaveBtreePriMax;
        AJCRESIZE0(statSaveBtreePri,statSaveBtreePriMax,newmax);
        statSaveBtreePriMax = newmax;
    }
    
    statSaveBtreePri[statSaveBtreePriNext++] = *thys;
    
    *thys = NULL;

    return;
}




/* @funcstatic btreePriFree *********************************************
**
** Destructor for index primary bucket information
**
** @param [w] thys [AjPBtPri*] index keyword primary object
**
** @return [void]
** @@
******************************************************************************/

static void btreePriFree(AjPBtPri *thys)
{
    AjPBtPri pri = NULL;

    if(!thys || !*thys)
	return;
    pri = *thys;
    
    /*ajDebug("In ajBtreePriFree id '%S' key '%S'\n",
      pri->id, pri->keyword);*/

    ajStrDel(&pri->id);
    ajStrDel(&pri->keyword);
    AJFREE(pri);
    *thys = NULL;

    return;
}




/* @funcstatic btreePriBucketIdlist *******************************************
**
** Copies all primary keys into a list
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
** @param [u] idlist [AjPList] list to hold keys
**
** @return [ajulong] Overflow
** @@
******************************************************************************/

static ajulong btreePriBucketIdlist(AjPBtcache cache, ajulong pagepos,
                                   AjPList idlist)
{
    AjPBtpage page      = NULL;
    AjPBtpage lpage     = NULL;
    unsigned char *buf  = NULL;
    unsigned char *kptr = NULL;
    AjPBtPri pri        = NULL;
    
    unsigned char *codeptr = NULL;
    
    ajuint  nodetype  = 0;
    ajuint  nentries  = 0;
    ajulong overflow  = 0L;
    ajulong pageoverflow = 0L;
    ajuint  dirtysave = 0;
    
    ajuint  i;
    ajuint  len  = 0;
    ajuint idlen = 0;
    
    /* ajDebug("In btreePriBucketIdlist\n"); */
    
    if(!pagepos)
	ajFatal("PriBucketIdlist: cannot read bucket from root page cache %S",
                cache->filename);

    page  = ajBtreeCacheRead(cache,pagepos);
    lpage = page;
    dirtysave = lpage->dirty;
    lpage->dirty = BT_LOCK;
    lpage->lockfor = 1211;

    buf = lpage->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);
    if(nodetype != BT_PRIBUCKET)
	ajFatal("PriBucketIdlist: NodeType mismatch. "
                "Not primary bucket (%u) cache %S",
		nodetype, cache->filename);
    
    GBT_BUCKNENTRIES(buf,&nentries);
    

    GBT_BUCKOVERFLOW(buf,&overflow);
    pageoverflow = overflow;
    
    kptr  = PBT_BUCKKEYLEN(buf);
    codeptr = kptr + (nentries * sizeof(ajuint));

    for(i=0;i<nentries;++i)
    {
	BT_GETAJUINT(kptr,&len);
        idlen = len - sizeof(ajulong) - 1;
	if((codeptr-buf+1) + len > cache->pagesize)	/* overflow */
	{
	    /* ajDebug("ReadPriBucket: Overflow\n"); */
	    page  = ajBtreeCacheRead(cache,overflow);
	    buf = page->buf;
	    GBT_BUCKNODETYPE(buf,&nodetype);
	    if(nodetype != BT_PRIBUCKET)
		ajFatal("PriBucketIdlist: NodeType mismatch. Not primary "
			"bucket (%u) cache %S",
                        nodetype, cache->filename);
	    GBT_BUCKOVERFLOW(buf,&overflow);
	    /* overflow bucket ids start at the keylen position */
	    codeptr = PBT_BUCKKEYLEN(buf);
	}

	pri = ajBtreePriNew();

	/* Fill ID objects */
	ajStrAssignLenC(&pri->keyword,(const char *)codeptr,idlen);
	codeptr += (idlen + 1);
	BT_GETAJULONG(codeptr,&pri->treeblock);
	codeptr += sizeof(ajulong);

	kptr += sizeof(ajuint);
        ajListPushAppend(idlist, pri);
    }

    lpage->dirty = dirtysave;
    
    return pageoverflow;
}




/* @funcstatic btreeReadPriBucket *********************************************
**
** Constructor for keyword index primary bucket given a disc page number
** Creates one empty key slot for possible addition
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
**
** @return [AjPPriBucket] bucket
** @@
******************************************************************************/

static AjPPriBucket btreeReadPriBucket(AjPBtcache cache, ajulong pagepos)
{
    AjPPriBucket bucket = NULL;
    AjPBtpage page      = NULL;
    AjPBtpage lpage     = NULL;
    unsigned char *buf  = NULL;
    unsigned char *kptr = NULL;
    AjPBtPri pri        = NULL;
    
    unsigned char *codeptr = NULL;
    
    ajuint  nodetype  = 0;
    ajuint  nentries  = 0;
    ajulong overflow  = 0L;
    ajuint  dirtysave = 0;
    
    ajuint  i;
    ajuint  len  = 0;
    
    /* ajDebug("In btreeReadPriBucket\n"); */
    
    if(!pagepos)
	ajFatal("PriBucketRead: cannot read bucket from root page cache %S",
                cache->filename);

    page  = ajBtreeCacheRead(cache,pagepos);
    lpage = page;
    dirtysave = lpage->dirty;
    lpage->dirty = BT_LOCK;
    lpage->lockfor = 1221;

    buf = lpage->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);

    if(nodetype != BT_PRIBUCKET)
	ajFatal("PriReadBucket: NodeType mismatch. "
                "Not primary bucket (%u) page %Lu cache %S",
		nodetype, pagepos, cache->filename);
    
    GBT_BUCKNENTRIES(buf,&nentries);

    if(nentries > cache->nperbucket)
	ajFatal("PriReadBucket: Bucket too full page: %Lu "
                "entries: %u max: %u page %Lu cache %S",
                pagepos, nentries, cache->nperbucket,
                pagepos, cache->filename);
    

    GBT_BUCKOVERFLOW(buf,&overflow);

    bucket = btreePriBucketNew(cache->nperbucket);
    bucket->Nentries = nentries;
    
    kptr  = PBT_BUCKKEYLEN(buf);
    codeptr = kptr + (nentries * sizeof(ajuint));

    for(i=0;i<nentries;++i)
    {
	BT_GETAJUINT(kptr,&len);

	if((codeptr-buf+1) + len > cache->pagesize)	/* overflow */
	{
#if AJINDEX_DEBUG
	    ajDebug("PriReadBucket: Overflow\n");
#endif
	    page  = ajBtreeCacheRead(cache,overflow);
	    buf = page->buf;
	    GBT_BUCKNODETYPE(buf,&nodetype);

	    if(nodetype != BT_PRIBUCKET)
		ajFatal("PriReadBucket: NodeType mismatch. Not primary "
			"bucket (%u) page %Lu", nodetype, pagepos);

	    GBT_BUCKOVERFLOW(buf,&overflow);
	    /* overflow bucket ids start at the keylen position */
	    codeptr = PBT_BUCKKEYLEN(buf);
	}

	pri = bucket->codes[i];

	/* Fill ID objects */
	ajStrAssignC(&pri->keyword,(const char *)codeptr);
	codeptr += (strlen((const char *)codeptr) + 1);
	BT_GETAJULONG(codeptr,&pri->treeblock);
	codeptr += sizeof(ajulong);

	kptr += sizeof(ajuint);
    }

    lpage->dirty = dirtysave;
    
    return bucket;
}




/* @funcstatic btreeWritePriBucket *******************************************
**
** Write primary keyword index bucket object to the cache given a disc page
** number
**
** @param [u] cache [AjPBtcache] cache
** @param [r] bucket [const AjPPriBucket] bucket
** @param [r] pagepos [ajulong] page number
**
** @return [void]
** @@
******************************************************************************/

static void btreeWritePriBucket(AjPBtcache cache, const AjPPriBucket bucket,
				ajulong pagepos)
{
    AjPBtpage page  = NULL;
    AjPBtpage lpage = NULL;

    unsigned char *buf  = NULL;
    unsigned char *lbuf = NULL;

    ajuint  v   = 0;
    ajuint i   = 0;
    ajuint len = 0;
    ajulong lv  = 0L;
    ajulong pno = 0L;

    AjPBtPri pri    = NULL;
    ajuint nentries = 0;
    ajulong overflow = 0L;

    unsigned char *keyptr = NULL;
    unsigned char *lptr   = NULL;

    /* ajuint unused = 0; */


    /* ajDebug("In btreeWritePriBucket\n"); */

    if(pagepos == cache->totsize)	/* Create a new page */
    {
	pno = pagepos;
	page = ajBtreeCacheWriteNodenew(cache);
	buf = page->buf;
        overflow = 0L;
    }
    else
    {
	page = ajBtreeCacheRead(cache,pagepos);
	buf = page->buf;
	GBT_BUCKOVERFLOW(buf,&overflow);
    }

    v = BT_PRIBUCKET;
    SBT_BUCKNODETYPE(buf,v);

    lbuf = buf;
    page->dirty = BT_LOCK;
    page->lockfor = 1231;
    lpage = page;

    nentries = bucket->Nentries;
    v = nentries;
    SBT_BUCKNENTRIES(buf,v);

    /* Write out key lengths */
    keyptr = PBT_BUCKKEYLEN(lbuf);

    for(i=0;i<nentries;++i)
    {
	if((ajuint)((keyptr-lbuf+1)+sizeof(ajuint)) > cache->pagesize)
	    ajFatal("PriBucketWrite: Bucket cannot hold more than %u keys",
		    i-1);

	pri = bucket->codes[i];
	/* Need to alter this if bucket primary keyword structure changes */
	len = BT_BUCKPRILEN(pri->keyword);
        v = len;
	BT_SETAJUINT(keyptr,v);
	keyptr += sizeof(ajuint);
    }


    /* Write out IDs using overflow if necessary */
    lptr = keyptr;
    for(i=0;i<nentries;++i)
    {
	pri = bucket->codes[i];
	len = BT_BUCKPRILEN(pri->keyword);

	if((lptr-buf+1)+len > cache->pagesize) /* overflow */
	{
    	    /*ajDebug("WritePriBucket: Overflow\n");*/

	    if(!overflow)		/* No overflow buckets yet */
	    {
		pno = cache->totsize;
                lv = pno;
		SBT_BUCKOVERFLOW(buf,lv);
		page = ajBtreeCacheWriteNodenew(cache);
                buf = page->buf;
		v = BT_PRIBUCKET;
		SBT_BUCKNODETYPE(buf,v);
	    }
	    else
	    {
		page = ajBtreeCacheRead(cache,overflow);
		buf  = page->buf;
		GBT_BUCKOVERFLOW(buf,&overflow);
	    }

	    page->dirty = BT_DIRTY;

	    lptr = PBT_BUCKKEYLEN(buf);	    
	}
	
	sprintf((char *)lptr,"%s",ajStrGetPtr(pri->keyword));
	lptr += (ajStrGetLen(pri->keyword) + 1);
        lv = pri->treeblock;
	BT_SETAJULONG(lptr,lv);
	lptr += sizeof(ajulong);
    }

    lv = 0L;
    SBT_BUCKOVERFLOW(buf,lv);

    /*
    unused = cache->pagesize - (lptr - buf);
    if(unused > cache->pagesize)
        ajFatal("unused %u for pagesize %u", unused, cache->pagesize);
    if(unused > 0)
        memset(lptr, 0, unused);
    */

    lpage->dirty = BT_DIRTY;

    return;
}




/* @funcstatic btreeWritePriBucketEmpty ****************************************
**
** Write empty primary keyword index bucket object to the cache given a
** disc page number
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
**
** @return [void]
** @@
******************************************************************************/

static void btreeWritePriBucketEmpty(AjPBtcache cache, ajulong pagepos)
{
    AjPBtpage page  = NULL;
    AjPBtpage lpage = NULL;

    unsigned char *buf  = NULL;

    ajuint  v   = 0;
    ajulong lv  = 0L;

    ajulong overflow = 0L;

    /* ajuint unused = 0; */

    /* ajDebug("In btreeWritePriBucketEmpty\n"); */

    if(pagepos == cache->totsize)	/* Create a new page */
    {
	page = ajBtreeCacheWriteBucketnew(cache);
	buf = page->buf;
	overflow = 0L;
	lv = overflow;
	SBT_BUCKOVERFLOW(buf,lv);
    }
    else
    {
	page = ajBtreeCacheRead(cache,pagepos);
	buf = page->buf;
	GBT_BUCKOVERFLOW(buf,&overflow);
    }

    v = BT_PRIBUCKET;
    SBT_BUCKNODETYPE(buf,v);

    page->dirty = BT_LOCK;      /* cleared at end */
    page->lockfor = 1241;
    lpage = page;

    v = 0;
    SBT_BUCKNENTRIES(buf,v);

    lv = 0L;
    SBT_BUCKOVERFLOW(buf,lv);

/*
    lptr = PBT_BUCKKEYLEN(buf);  

    unused = cache->pagesize - (lptr - buf);
    if(unused > cache->pagesize)
        ajFatal("unused %u for pagesize %u", unused, cache->pagesize);
    if(unused > 0)
        memset(lptr, 0, unused);
    */

    lpage->dirty = BT_DIRTY;    /* clear the lock */

    return;
}




/* @funcstatic btreeAddToPriBucket *******************************************
**
** Add a keyword ID to a primary bucket
** Only called if there is room in the bucket
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number of bucket
** @param [r] pri [const AjPBtPri] ID info
**
** @return [void]
** @@
******************************************************************************/

static void btreeAddToPriBucket(AjPBtcache cache, ajulong pagepos,
				const AjPBtPri pri)
{
    unsigned char *buf  = NULL;
    unsigned char *kptr = NULL;
    unsigned char *src  = NULL;
    unsigned char *dest = NULL;

    unsigned char *lastptr = NULL;
    unsigned char *endptr  = NULL;
    
    ajuint nentries = 0;
    ajuint nodetype = 0;
    ajuint idlen    = 0;

    ajuint sum = 0;
    ajuint len = 0;
    ajuint i;
    ajuint v;
    ajulong lv;
    
    ajulong secrootpage = 0L;
    ajulong right       = 0L;
    ajulong treeblock  = 0L;

    AjPBtpage page = NULL;
    static ajuint calls = 0;
    static ajuint overflowcalls=0;

    calls++;

    treeblock = pri->treeblock;

    /* See if secondary tree exists, if not then create it */
    if(!pri->treeblock)
    {
	secrootpage = cache->totsize;

	ajBtreeCreateRootNode(cache,secrootpage);
	cache->secrootblock = secrootpage;
	page = ajBtreeCacheWrite(cache,secrootpage);
	page->dirty = BT_LOCK;
        page->lockfor = 1251;

        if(btreeDoRootSync)
            ajBtreeCacheRootSync(cache,secrootpage);

	treeblock = secrootpage;
        buf = page->buf;
	cache->slevel = 0;
    }
    else
    {
	cache->secrootblock = pri->treeblock;
	page = ajBtreeCacheWrite(cache,cache->secrootblock);
	page->dirty = BT_LOCK;
        page->lockfor = 1252;
	buf = page->buf;
	GBT_RIGHT(buf, &right);
	cache->slevel = (ajuint) right;
    }

    ajBtreeSecInsertId(cache, pri->id);

    right = (ajulong) cache->slevel;
    
    SBT_RIGHT(buf, right);
    page->dirty = BT_DIRTY;
    
    /* add to the PriBucket page */


    page = ajBtreeCacheRead(cache,pagepos);
    buf  = page->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);

    if(nodetype != BT_PRIBUCKET)
        ajFatal("Wrong nodetype in AddToPriBucket cache %S",
                cache->filename);

    GBT_BUCKNENTRIES(buf,&nentries);

    if(nentries >= cache->nperbucket)
        ajFatal("Bucket too full in AddToPriBucket page: %Lu "
                "entries: %u max: %u cache %S",
                pagepos, nentries, cache->nperbucket,
                cache->filename);    

    kptr = PBT_BUCKKEYLEN(buf);
    src  = kptr + (nentries * sizeof(ajuint));

    for(i=0;i<nentries;++i)
    {
        BT_GETAJUINT(kptr,&len);
        sum += len;
        kptr += sizeof(ajuint);
    }

    endptr  = src + sum - 1;
    idlen   = MAJSTRGETLEN(pri->keyword);
    lastptr = endptr + sizeof(ajulong) + idlen + 1;

    if((ajuint) (lastptr - buf) >= cache->pagesize)
    {
        overflowcalls++;

        ajWarn("\nOverflow in AddToPriBucket nentries:%u fails %u/%u cache %S",
               nentries, overflowcalls, calls, cache->filename);

        btreeAddToPriBucketFull(cache,pagepos,pri);
        return;
    }
    
    dest = src + sizeof(ajuint);
    memmove((void *)dest, (void *)src, sum);

    v = BT_BUCKPRILEN(pri->keyword);
    BT_SETAJUINT(src,v);

    endptr += sizeof(ajuint) + 1;
    strcpy((char *)endptr,MAJSTRGETPTR(pri->keyword));

    endptr += idlen + 1;
    lv = treeblock;
    BT_SETAJULONG(endptr,lv);
    endptr += sizeof(ajulong);

    v = nentries + 1;
    SBT_BUCKNENTRIES(buf,v);

    page->dirty = BT_DIRTY;

    return;
}




/* @funcstatic btreeAddToPriBucketFull ****************************************
**
** Add a keyword ID to a primary bucket
** Only called if there is room in the bucket
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number of bucket
** @param [r] pri [const AjPBtPri] ID info
**
** @return [void]
** @@
******************************************************************************/

static void btreeAddToPriBucketFull(AjPBtcache cache, ajulong pagepos,
                                    const AjPBtPri pri)
{
    AjPPriBucket bucket  = NULL;
    AjPBtPri     dest    = NULL;
    AjPBtpage    page    = NULL;
    
    ajuint nentries;
    ajulong secrootpage = 0L;
    ajulong right       = 0L;

    unsigned char *buf;
    
    /* ajDebug("In btreeAddToPriBucket\n"); */

    bucket   = btreeReadPriBucket(cache,pagepos);
    nentries = bucket->Nentries;


    /* Reading a bucket always gives one extra ID position */
    bucket->codes[nentries] = ajBtreePriNew();
    dest = bucket->codes[nentries];

    ajStrAssignS(&dest->keyword,pri->keyword);

    /* Need to add code here to access secondary tree */
    dest->treeblock = pri->treeblock;


    /* See if secondary tree exists, if not then create it */
    if(!pri->treeblock)
    {
	secrootpage = cache->totsize;

	ajBtreeCreateRootNode(cache,secrootpage);
	cache->secrootblock = secrootpage;
	page = ajBtreeCacheWrite(cache,secrootpage);
	page->dirty = BT_LOCK;
        page->lockfor = 1261;
        if(btreeDoRootSync)
            ajBtreeCacheRootSync(cache,secrootpage);
	dest->treeblock = secrootpage;
	buf = page->buf;
	cache->slevel = 0;

	/* ajDebug("Created 2ry tree at block %u\n",(ajuint)secrootpage); */
    }
    else
    {
	cache->secrootblock = pri->treeblock;
	page = ajBtreeCacheWrite(cache,cache->secrootblock);
	page->dirty = BT_LOCK;
        page->lockfor = 1262;
	buf = page->buf;
	GBT_RIGHT(buf, &right);
	cache->slevel = (ajuint) right;
    }

    ajBtreeSecInsertId(cache, pri->id);

    right = (ajulong) cache->slevel;
    
    SBT_RIGHT(buf, right);
    page->dirty = BT_DIRTY;
    


    
    ++bucket->Nentries;

    btreeWritePriBucket(cache,bucket,pagepos);

    btreePriBucketDel(&bucket);
    
    return;
}




/* @funcstatic btreeNumInPriBucket *******************************************
**
** Return number of entries in a primary keyword bucket
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
**
** @return [ajuint] Number of entries in bucket
** @@
******************************************************************************/

static ajuint btreeNumInPriBucket(AjPBtcache cache, ajulong pagepos)
{
    AjPBtpage page     = NULL;
    unsigned char *buf = NULL;

    ajuint  nodetype = 0;
    ajuint nentries = 0;
    
    /* ajDebug("In btreeNumInPriBucket\n"); */
    
    if(!pagepos)
	ajFatal("NumInPriBucket: Attempt to read bucket from root page\n");

    page  = ajBtreeCacheRead(cache,pagepos);

    buf = page->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);

    if(nodetype != BT_PRIBUCKET)
	ajFatal("PriReadBucket: NodeType mismatch. Not primary bucket (%u)",
		nodetype);
    
    GBT_BUCKNENTRIES(buf,&nentries);

    return nentries;
}




/* @funcstatic btreeKeywordCompare *******************************************
**
** Comparison function for ajListSort
**
** @param [r] a [const void*] ID 1
** @param [r] b [const void*] ID 2
**
** @return [ajint] 0 = bases match
** @@
******************************************************************************/

static ajint btreeKeywordCompare(const void *a, const void *b)
{
    return MAJSTRCMPS((*(AjPBtPri const *)a)->keyword,
                      (*(AjPBtPri const *)b)->keyword);
}




/* @funcstatic btreeReorderPriBuckets *****************************************
**
** Re-order primary keyword leaf buckets
** Must only be called if one of the buckets is full
**
** @param [u] cache [AjPBtcache] cache
** @param [u] leaf [AjPBtpage] leaf page
**
** @return [AjBool] true if reorder was successful i.e. leaf not full
** @@
******************************************************************************/
static AjBool btreeReorderPriBuckets(AjPBtcache cache, AjPBtpage leaf)
{
    ajuint nkeys = 0;
    unsigned char *lbuf   = NULL;

    ajulong *ptrs      = NULL;
    AjPStr *newkeys      = NULL;
    ajulong *newptrs      = NULL;
    ajulong *overflows = NULL;
    AjPBtMem arrays1 = NULL;
    AjPBtMem arrays2 = NULL;
    
    ajuint i = 0;
    
    ajuint order;
    ajuint totalkeys     = 0;
    ajuint maxnperbucket = 0;
    ajuint count         = 0;
    ajuint keylimit      = 0;
    ajuint bucketlimit   = 0;
    ajuint nodetype      = 0;
    ajuint dirtysave     = 0;    

    AjPList idlist       = NULL;
    AjPBtPri bid         = NULL;
    AjPPriBucket cbucket = NULL;
    AjPBtPri cid         = NULL;

    ajuint iold = 0;
    
#if AJINDEX_DEBUG
    ajDebug("btreeReorderPriBuckets %Lu\n", leaf->pagepos);
#endif
    ++statCallReorderPriBuckets;

    /* ajDebug("In btreeReorderPriBuckets\n"); */

    dirtysave = leaf->dirty;

    leaf->dirty = BT_LOCK;
    leaf->lockfor = 1271;
    lbuf = leaf->buf;

    GBT_NODETYPE(lbuf,&nodetype);

    order = cache->order;
    
    /* Read keys/ptrs */
    arrays1 = btreeAllocPriArray(cache);
    ptrs = arrays1->parray;
    arrays2 = btreeAllocPriArray(cache);
    newkeys = arrays2->karray;
    newptrs = arrays2->parray;
    overflows = arrays2->overflows;
    
    btreeGetPointers(cache,lbuf,&ptrs);

    GBT_NKEYS(lbuf,&nkeys);
    keylimit = nkeys + 1;

    if(!nkeys)
	ajFatal("ReorderPriBuckets: Attempt to reorder empty leaf");

    for(i=0;i<nkeys;++i)
	totalkeys += btreeNumInPriBucket(cache,ptrs[i]);
    totalkeys += btreeNumInPriBucket(cache,ptrs[i]);

    btreeBucketCalc(totalkeys, keylimit, cache->nperbucket,
                    &bucketlimit, &maxnperbucket);

    if(bucketlimit >= order)
    {
        btreeDeallocPriArray(cache,arrays1);
        btreeDeallocPriArray(cache,arrays2);
	leaf->dirty = dirtysave;

	return ajFalse;
    }
    

    /* Read IDs from all buckets and push to list and sort (increasing kw) */
    idlist  = ajListNew();
    
    for(i=0;i<keylimit;++i)
        overflows[i] = btreePriBucketIdlist(cache,ptrs[i],idlist);

    ajListSort(idlist,btreeKeywordCompare);

    cbucket = btreePriBucketNew(cache->nperbucket);
    
    iold = 0;
    for(i=0;i<bucketlimit;++i)
    {
	cbucket->Overflow = overflows[i];
	cbucket->Nentries = 0;

        count = 0;
	while(count!=maxnperbucket)
	{
	    ajListPop(idlist,(void **)&bid);
	    
	    cid = cbucket->codes[count];
	    ajStrAssignS(&cid->keyword,bid->keyword);
	    cid->treeblock = bid->treeblock;

	    cbucket->keylen[count] = BT_BUCKPRILEN(bid->keyword);
	    ++cbucket->Nentries;
	    ++count;
	    ajBtreePriDel(&bid);
	}


	ajListPeek(idlist,(void **)&bid);
	ajStrAssignS(&newkeys[i],bid->keyword);

	if((iold < order) && ptrs[iold])
            newptrs[i] = ptrs[iold++];
        else
	    newptrs[i] = cache->totsize;
	btreeWritePriBucket(cache,cbucket,newptrs[i]);
    }


    /* Deal with greater-than bucket */

    cbucket->Overflow = overflows[i];
    cbucket->Nentries = 0;

    count = 0;

    while(ajListPop(idlist,(void **)&bid))
    {
	cid = cbucket->codes[count];
	ajStrAssignS(&cid->keyword,bid->keyword);
	cid->treeblock = bid->treeblock;

	++cbucket->Nentries;
	++count;
	ajBtreePriDel(&bid);
    }
    
    
    if((iold < order) && ptrs[iold])
        newptrs[i] = ptrs[iold++];
    else
        newptrs[i] = cache->totsize;
    btreeWritePriBucket(cache,cbucket,newptrs[i]);

    btreePriBucketDel(&cbucket);

#if AJINDEX_DEBUG
    if(bucketlimit <= keylimit)
        ajDebug("btreeReorderPriBuckets '%S' %u -> %u",
                cache->filename, keylimit, bucketlimit);
#endif

    for(i = bucketlimit + 1; i <= keylimit; i++)
    {
        btreePageSetfree(cache, ptrs[i]);
    }

    /* Now write out a modified leaf with new keys/ptrs */

    nkeys = bucketlimit;
    btreeWriteNode(cache,leaf,newkeys,newptrs,nkeys);
    leaf->dirty = BT_DIRTY;

    if(nodetype == BT_ROOT)
    {
	leaf->dirty = BT_LOCK;
        leaf->lockfor = 1272;
    }
    
    btreeDeallocPriArray(cache,arrays1);
    btreeDeallocPriArray(cache,arrays2);

    ajListFree(&idlist);

    return ajTrue;
}




/* @func ajBtreeInsertKeyword *********************************************
**
** Insert a keyword structure into the tree
**
** @param [u] cache [AjPBtcache] cache
** @param [u] pri [AjPBtPri] Id object
**
** @return [AjBool] True if keyword and ID combination was inserted
**                  False if keyword exists alraedy for ID
** @@
******************************************************************************/

AjBool ajBtreeInsertKeyword(AjPBtcache cache, AjPBtPri pri)
{
    AjPBtpage spage   = NULL;
    AjPBtpage page    = NULL;
    AjPStr key        = NULL;
    ajulong lblockno = 0L;
    ajulong rblockno = 0L;
    ajulong blockno  = 0L;
    ajulong shift    = 0L;

    ajuint nkeys = 0;

    ajuint nodetype = 0;
    ajulong right   = 0L;
    
    ajuint n;
    ajuint savedirty;
    unsigned char *buf   = NULL;

    ajulong treeblock = 0L;
    AjBool newid = ajTrue;

#if AJINDEX_DEBUG
    /*ajDebug("btreeInsertKeyword '%S' '%S'\n", pri->keyword, pri->id);*/
#endif

    if(!MAJSTRGETLEN(pri->keyword))
	return ajFalse;

    ajStrFmtQuery(&pri->id);
    ajStrFmtQuery(&pri->keyword);

    if(MAJSTRGETLEN(pri->keyword) > cache->kwlimit)
        ajStrTruncateLen(&pri->keyword, cache->kwlimit);

    /* Only insert a primary key if that key doesn't exist */
    if(ajBtreePriFindKeyword(cache,pri->keyword,
                             &treeblock))
    {
	cache->secrootblock = treeblock;
	page = ajBtreeCacheWriteNode(cache,cache->secrootblock);
        savedirty = page->dirty;
	page->dirty = BT_LOCK;
        page->lockfor = 1281;
	buf = page->buf;
	GBT_RIGHT(buf,&right);
	cache->slevel = (ajuint) right;
	if(ajBtreeSecInsertId(cache,pri->id))
        {
            ++cache->countall;
            right = (ajulong) cache->slevel;
            SBT_RIGHT(buf,right);
            page->dirty = BT_DIRTY;

        }
        else
        {
            newid = ajFalse;
            page->dirty = savedirty;
        }
        
	return newid;
    }

    key = ajStrNewS(pri->keyword);

    spage = ajBtreeFindInsert(cache,key);
    buf = spage->buf;

    GBT_NKEYS(buf,&nkeys);
    GBT_NODETYPE(buf,&nodetype);
    
    if(!nkeys)
    {
	lblockno = cache->totsize;
	btreeWritePriBucketEmpty(cache,lblockno);

	rblockno = cache->totsize;
	btreeWritePriBucketEmpty(cache,rblockno);	

	btreeWriteNodeSingle(cache,spage,key,lblockno,rblockno);

	GBT_BLOCKNUMBER(buf,&blockno);

	if(!blockno)
        {
	    spage->dirty = BT_LOCK; /* root page */
            spage->lockfor = 1282;
        }

	btreeAddToPriBucket(cache,rblockno,pri);
	++cache->countunique;
	++cache->countall;

        ajStrDel(&key);
	return ajTrue;
    }
    
    blockno = btreeGetBlockS(cache,buf,key);
    if(nodetype != BT_ROOT)
	if((shift = btreeInsertShiftS(cache,&spage,key)))
	    blockno = shift;

    buf = spage->buf;

    n = btreeNumInPriBucket(cache,blockno);

    if(n == cache->nperbucket)
    {
	if(btreeReorderPriBuckets(cache,spage))
	{
            blockno = btreeGetBlockS(cache,buf,key);
	}
	else
	{
	    btreeSplitPriLeaf(cache,spage);
	    spage = ajBtreeFindInsert(cache,key);
	    buf = spage->buf;

            blockno = btreeGetBlockS(cache,buf,key);
	}
    }

    btreeAddToPriBucket(cache,blockno,pri);
    ++cache->countunique;
    ++cache->countall;

    ajStrDel(&key);

    return ajTrue;
}




/* @func ajBtreePriFindKeyword **********************************************
**
** Get Pri structure matching a keyword
**
** @param [u] cache [AjPBtcache] cache
** @param [r] key [const AjPStr] key
** @param [w] treeblock [ajulong*] Tree block number for id
**
** @return [AjBool] ajTrue if found
** @@
******************************************************************************/

AjBool ajBtreePriFindKeyword(AjPBtcache cache, const AjPStr key,
                             ajulong* treeblock)
{
    AjPBtpage page      = NULL;
    
    unsigned char *buf = NULL;

    ajulong blockno = 0L;

    /*ajDebug("ajBtreePriFindKeyword '%s'\n", ckey);*/
    

    if(!cache->countunique)
	return ajFalse;

    page = ajBtreeFindInsert(cache,key);

    buf = page->buf;

    blockno = btreeGetBlockS(cache, buf, key);

    return btreePriBucketFindId(cache,blockno,key,treeblock);
}




/* @func ajBtreePriFindKeywordLen ********************************************
**
** Get Pri structure matching a keyword truncated to maximum indexed length
**
** @param [u] cache [AjPBtcache] cache
** @param [r] key [const AjPStr] key
** @param [w] treeblock [ajulong*] Tree block number for id
**
** @return [AjBool] ajTrue if found
** @@
******************************************************************************/

AjBool ajBtreePriFindKeywordLen(AjPBtcache cache, const AjPStr key,
                                ajulong* treeblock)
{
    AjBool ret = ajFalse;

    AjPBtpage page      = NULL;
    unsigned char *buf = NULL;
    ajulong blockno = 0L;

    AjPStr keystr = NULL;
    const AjPStr keytest = key;

#if AJINDEX_DEBUG
    ajDebug("ajBtreePriFindKeywordLen '%S'\n", key);
#endif

    keystr = ajStrNewS(key);
    ajStrFmtQuery(&keystr);
    if(MAJSTRGETLEN(keystr) > cache->kwlimit)
        ajStrTruncateLen(&keystr, cache->kwlimit);
    keytest = keystr;

    if(!cache->countunique)
	return ajFalse;

    page = ajBtreeFindInsert(cache,keytest);

    buf = page->buf;

    blockno = btreeGetBlockS(cache, buf, keytest);

    ret = btreePriBucketFindId(cache,blockno,keytest,treeblock);

    ajStrDel(&keystr);

    return ret;
}




/* @funcstatic btreePriBucketFindId *******************************************
**
** Tests for an ID in a primary bucket
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
** @param [r] id [const AjPStr] id to search for
** @param [w] treeblock [ajulong*] Tree block number for id
**
** @return [AjBool] ajTrue if found
** @@
******************************************************************************/

static AjBool btreePriBucketFindId(AjPBtcache cache, ajulong pagepos,
                                   const AjPStr id, ajulong* treeblock)
{
    AjPBtpage page      = NULL;
    AjPBtpage lpage     = NULL;
    unsigned char *buf  = NULL;
    unsigned char *kptr = NULL;
    
    unsigned char *codeptr = NULL;
    
    ajuint  nodetype  = 0;
    ajuint  nentries  = 0;
    ajulong overflow  = 0L;
    ajuint  dirtysave = 0;
    
    ajuint  i;
    ajuint  len  = 0;
    ajuint idlen = 0;
    
    /* ajDebug("In btreePriBucketFindId\n"); */
    
    if(!pagepos)
	ajFatal("PriBucketFindId: cannot read bucket from root page cache %S",
                cache->filename);

    page  = ajBtreeCacheRead(cache,pagepos);
    lpage = page;
    dirtysave = lpage->dirty;
    lpage->dirty = BT_LOCK;
    lpage->lockfor = 1291;

    buf = lpage->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);
    if(nodetype != BT_PRIBUCKET)
	ajFatal("PriBucketFindId: NodeType mismatch. "
                "Not primary bucket (%u) cache %S",
		nodetype, cache->filename);
    
    GBT_BUCKNENTRIES(buf,&nentries);
    if(nentries > cache->nperbucket)
	ajFatal("PriBucketFindId: Bucket too full  page: %Lu "
                "entries: %u max: %u cache %S",
                pagepos, nentries, cache->nperbucket,
                cache->filename);
    

    GBT_BUCKOVERFLOW(buf,&overflow);
    
    kptr  = PBT_BUCKKEYLEN(buf);
    codeptr = kptr + (nentries * sizeof(ajuint));

    for(i=0;i<nentries;++i)
    {
	BT_GETAJUINT(kptr,&len);
        idlen = len - sizeof(ajulong) - 1;
	if((codeptr-buf+1) + len > cache->pagesize)	/* overflow */
	{
	    /* ajDebug("PriBucketFindId: Overflow\n"); */
	    page  = ajBtreeCacheRead(cache,overflow);
	    buf = page->buf;
	    GBT_BUCKNODETYPE(buf,&nodetype);
	    if(nodetype != BT_PRIBUCKET)
		ajFatal("PriBucketFindId: NodeType mismatch. "
                        "Not primary bucket (%u) cache %S",
                        nodetype, cache->filename);
	    GBT_BUCKOVERFLOW(buf,&overflow);
	    /* overflow bucket ids start at the keylen position */
	    codeptr = PBT_BUCKKEYLEN(buf);
	}

	/* Fill ID objects */
	if(ajStrMatchC(id, (const char *)codeptr))
        {
            codeptr += (idlen + 1);
            BT_GETAJULONG(codeptr,treeblock);
            lpage->dirty = dirtysave;
            return ajTrue;
        }
        
	codeptr += (idlen + 1);
	codeptr += sizeof(ajulong);

	kptr += sizeof(ajuint);
    }

    lpage->dirty = dirtysave;
    
    return ajFalse;
}




/* @funcstatic btreeSplitPriLeaf *********************************************
**
** Split a leaf and propagate up if necessary
**
** @param [u] cache [AjPBtcache] cache
** @param [u] spage [AjPBtpage] page
**
** @return [AjPBtpage] pointer to a parent page
** @@
******************************************************************************/

static AjPBtpage btreeSplitPriLeaf(AjPBtcache cache, AjPBtpage spage)
{
    ajuint nkeys     = 0;
    ajuint order     = 0;
    ajuint totalkeys = 0;
    ajuint keylimit  = 0;
    ajuint nodetype  = 0;

    ajuint rootnodetype = 0;
    
    ajuint i;
    ajuint j;
    
    AjPBtpage lpage = NULL;
    AjPBtpage rpage = NULL;
    AjPBtpage page  = NULL;
    
    AjPStr mediankey  = NULL;
    ajulong mediangtr  = 0L;
    ajulong medianless = 0L;
    

    AjPBtPri bid       = NULL;
    AjPBtPri cid       = NULL;
    unsigned char *buf  = NULL;
    unsigned char *lbuf = NULL;
    unsigned char *rbuf = NULL;

    AjPList idlist = NULL;

    AjPPriBucket cbucket  = NULL;
    
    ajulong *parray = NULL;
    AjPStr *newkarray = NULL;
    ajulong *newparray = NULL;
    AjPBtMem arrays = NULL;
    AjPBtMem newarrays = NULL;
    
    ajuint lno    = 0;
    ajuint rno    = 0;

    ajuint lbucketlimit   = 0;
    ajuint rbucketlimit   = 0;
    ajuint lmaxnperbucket = 0;
    ajuint rmaxnperbucket = 0;
    ajuint nperbucket    = 0;
    ajuint count         = 0;
    
    ajulong lblockno = 0L;
    ajulong rblockno = 0L;
    ajulong prev     = 0L;
    ajulong overflow = 0L;
    ajulong prevsave = 0L;
    
    ajulong zero = 0L;
    ajulong join = 0L;
    
    ajulong lv = 0L;
    ajuint  v  = 0;

    ajuint iold = 0;
    
#if AJINDEX_DEBUG
    ajDebug("btreeSplitPriLeaf %Lu\n", spage->pagepos);
#endif
    ++statCallSplitPriLeaf;

    /* ajDebug("In btreeSplitPriLeaf\n"); */

    order = cache->order;
    nperbucket = cache->nperbucket;

    mediankey = ajStrNew();
    arrays = btreeAllocPriArray(cache);
    parray = arrays->parray;

    newarrays = btreeAllocPriArray(cache);
    newkarray = newarrays->karray;
    newparray = newarrays->parray;

    buf = spage->buf;
    lbuf = buf;

    GBT_NKEYS(buf,&nkeys);
    GBT_NODETYPE(buf,&rootnodetype);

    if(rootnodetype == BT_ROOT)
    {
	lblockno = cache->totsize;
	lpage = ajBtreeCacheWriteNodenew(cache);
	lbuf = lpage->buf;
	lv = prev;
	SBT_PREV(lbuf,lv);
    }
    else
    {
	lblockno = spage->pagepos;
	lpage = spage;
    }

    lpage->dirty = BT_LOCK;
    lpage->lockfor = 1301;

    rblockno = cache->totsize;
    rpage = ajBtreeCacheWriteNodenew(cache);
    rbuf = rpage->buf;
    rpage->dirty = BT_LOCK;
    rpage->lockfor = 1302;

    if(rootnodetype == BT_ROOT)
    {
	lv = zero;
	SBT_RIGHT(rbuf,lv);
	lv = zero;
	SBT_LEFT(lbuf,lv);
    }
    else
    {
	GBT_RIGHT(lbuf,&join);
	lv = join;
	SBT_RIGHT(rbuf,lv);
    }

    lv = lblockno;
    SBT_LEFT(rbuf,lv);
    lv = rblockno;
    SBT_RIGHT(lbuf,lv);


    btreeGetPointers(cache,buf,&parray);


    keylimit = nkeys+1;
    idlist = ajListNew();

    for(i=0;i<keylimit;++i)
	btreePriBucketIdlist(cache,parray[i],idlist);

    ajListSort(idlist,btreeKeywordCompare);

    totalkeys = ajListGetLength(idlist);

    btreeBucketSplitCalc(totalkeys, keylimit, nperbucket,
                         &lbucketlimit,&lmaxnperbucket,&lno,
                         &rbucketlimit,&rmaxnperbucket,&rno);

    cbucket = btreePriBucketNew(cache->nperbucket);

    count = 0;
    iold = 0;

    for(i=0;i<lbucketlimit;++i)
    {
	cbucket->Nentries = 0;

	for(j=0;j<lmaxnperbucket;++j)
	{
	    ajListPop(idlist,(void **)&bid);
	    
	    cid = cbucket->codes[j];
	    ajStrAssignS(&cid->keyword,bid->keyword);
	    cid->treeblock = bid->treeblock;

	    cbucket->keylen[j] = BT_BUCKPRILEN(bid->keyword);
	    ++count;
	    ++cbucket->Nentries;
	    ajBtreePriDel(&bid);
	}

	ajListPeek(idlist,(void **)&bid);
	
	ajStrAssignS(&newkarray[i],bid->keyword);

	if((iold < order) && parray[iold])
            newparray[i] = parray[iold++];
        else
	    newparray[i] = cache->totsize;
	btreeWritePriBucket(cache,cbucket,newparray[i]);
    }

    cbucket->Nentries = 0;

    j = 0;

    while(count != lno)
    {
	ajListPop(idlist,(void **)&bid);
	cid = cbucket->codes[j];
	++j;
	++count;

	ajStrAssignS(&cid->keyword,bid->keyword);
	cid->treeblock = bid->treeblock;
	++cbucket->Nentries;
	ajBtreePriDel(&bid);
    }

    if((iold < order) && parray[iold])
        newparray[i] = parray[iold++];
    else
        newparray[i] = cache->totsize;
    btreeWritePriBucket(cache,cbucket,newparray[i]);

    nkeys = lbucketlimit;
    nodetype = BT_LEAF;
    v = nodetype;
    SBT_NODETYPE(lbuf,v);

    GBT_PREV(lbuf,&prevsave);
    lpage->dirty = BT_DIRTY;

    btreeWriteNode(cache,lpage,newkarray,newparray,nkeys);

    ajListPeek(idlist,(void **)&bid);
    ajStrAssignS(&mediankey,bid->keyword);

    for(i=0;i<rbucketlimit;++i)
    {
	cbucket->Nentries = 0;

	for(j=0;j<rmaxnperbucket;++j)
	{
	    ajListPop(idlist,(void **)&bid);
	    
	    cid = cbucket->codes[j];
	    ajStrAssignS(&cid->keyword,bid->keyword);
	    cid->treeblock = bid->treeblock;

	    cbucket->keylen[j] = BT_BUCKPRILEN(bid->keyword);
	    ++cbucket->Nentries;
	    ajBtreePriDel(&bid);
	}

	ajListPeek(idlist,(void **)&bid);
	ajStrAssignS(&newkarray[i],bid->keyword);

	if((iold < order) && parray[iold])
            newparray[i] = parray[iold++];
        else
	    newparray[i] = cache->totsize;
	btreeWritePriBucket(cache,cbucket,newparray[i]);
    }

    cbucket->Nentries = 0;

    j = 0;

    while(ajListPop(idlist,(void**)&bid))
    {
	cid = cbucket->codes[j];
	++j;

	ajStrAssignS(&cid->keyword,bid->keyword);
	cid->treeblock = bid->treeblock;
	++cbucket->Nentries;
	ajBtreePriDel(&bid);
    }
    
    if((iold < order) && parray[iold])
        newparray[i] = parray[iold++];
    else
        newparray[i] = cache->totsize;
    btreeWritePriBucket(cache,cbucket,newparray[i]);

    nkeys = rbucketlimit;

    v = nkeys;
    nodetype = BT_LEAF;
    v = nodetype;
    SBT_NODETYPE(rbuf,v);

    lv = prevsave;
    SBT_PREV(rbuf,lv);
    lv = overflow;
    SBT_OVERFLOW(rbuf,lv);
    
    btreeWriteNode(cache,rpage,newkarray,newparray,nkeys);
    rpage->dirty = BT_DIRTY;

    btreePriBucketDel(&cbucket);
    ajListFree(&idlist);



    medianless = lblockno;
    mediangtr  = rblockno;

    btreeDeallocPriArray(cache,arrays);
    btreeDeallocPriArray(cache,newarrays);

    if(rootnodetype == BT_ROOT)
    {
	btreeWriteNodeSingle(cache,spage,mediankey,lblockno,rblockno);	
	spage->dirty = BT_LOCK;
        spage->lockfor = 1303;
	ajStrDel(&mediankey);
	++cache->level;

	return spage;
    }

    page = ajBtreeCacheRead(cache,prevsave);
    btreeInsertKey(cache,page,mediankey,medianless,mediangtr);
    ajStrDel(&mediankey);

    page = ajBtreeCacheRead(cache,prevsave);

    return page;
}




/* @funcstatic btreeSecBucketFindId *******************************************
**
** Tests for an ID in a secondary bucket
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
** @param [r] id [const AjPStr] id to search for
**
** @return [AjBool] ajTrue if found
** @@
******************************************************************************/

static AjBool btreeSecBucketFindId(AjPBtcache cache, ajulong pagepos,
                                   const AjPStr id)
{
    AjPBtpage page  = NULL;
    AjPBtpage lpage = NULL;

    unsigned char *buf  = NULL;
    unsigned char *kptr = NULL;
    
    unsigned char *codeptr = NULL;
    
    ajuint  nodetype = 0;
    ajuint  nentries = 0;
    ajulong overflow = 0L;
    ajuint  dirtysave = 0;
    
    ajuint  i;
    ajuint  len  = 0;
    
    /* ajDebug("In btreeSecBucketFindId\n"); */

    /* Put in test here for secondary root page read
       instead of !pagepos - done */

    if(pagepos == cache->secrootblock)
	ajFatal("SecBucketFindId: cannot read bucket from root page cache %S",
                cache->filename);

    page  = ajBtreeCacheRead(cache,pagepos);
    lpage = page;
    dirtysave = lpage->dirty;
    lpage->dirty = BT_LOCK;
    lpage->lockfor = 1311;

    buf = lpage->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);
    if(nodetype != BT_SECBUCKET)
	ajFatal("SecBucketFindId: NodeType mismatch. "
                "Not secondary bucket (%u) page %Lu cache %S",
		nodetype, pagepos, cache->filename);

    GBT_BUCKNENTRIES(buf,&nentries);
    if(nentries > cache->snperbucket)
	ajFatal("SecBucketFindId: Bucket too full page: %Lu "
                "entries: %u max: %u cache %S",
                pagepos, nentries, cache->snperbucket,
                cache->filename);
    

    GBT_BUCKOVERFLOW(buf,&overflow);

    kptr  = PBT_BUCKKEYLEN(buf);
    codeptr = kptr + (nentries * sizeof(ajuint));

    for(i=0;i<nentries;++i)
    {
	BT_GETAJUINT(kptr,&len);
	if((codeptr-buf+1) + len > cache->pagesize)	/* overflow */
	{
	    /* ajDebug("SecBucketFindId: Overflow\n"); */
	    page  = ajBtreeCacheRead(cache,overflow);
	    buf = page->buf;
	    GBT_BUCKNODETYPE(buf,&nodetype);
	    if(nodetype != BT_SECBUCKET)
		ajFatal("SecBucketFindId: NodeType mismatch. "
                        "Not secondary bucket (%u) page: %Lu cache %S",
                        nodetype, pagepos, cache->filename);
	    GBT_BUCKOVERFLOW(buf,&overflow);
	    /* overflow bucket ids start at the keylen position */
	    codeptr = PBT_BUCKKEYLEN(buf);
	}

	/* Fill ID objects */
        if(ajStrMatchC(id, (const char *)codeptr))
        {
            lpage->dirty = dirtysave;
            return ajTrue;
        }
        
	codeptr += len;

	kptr += sizeof(ajuint);
    }

    lpage->dirty = dirtysave;
    
    return ajFalse;
}




/* @funcstatic btreeSecBucketIdlist *******************************************
**
** Copies all secondary IDs into a list
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
** @param [u] idlist [AjPList] list to hold secondary ID strings
**
** @return [ajulong] Overflow
** @@
******************************************************************************/

static ajulong btreeSecBucketIdlist(AjPBtcache cache, ajulong pagepos,
                                   AjPList idlist)
{
    AjPStr tmpstr = NULL;
    
    AjPBtpage page  = NULL;
    AjPBtpage lpage = NULL;

    unsigned char *buf  = NULL;
    unsigned char *kptr = NULL;
    
    unsigned char *codeptr = NULL;
    
    ajuint  nodetype = 0;
    ajuint  nentries = 0;
    ajulong overflow = 0L;
    ajulong pageoverflow = 0L;
    ajuint  dirtysave = 0;
    
    ajuint  i;
    ajuint  len  = 0;
    
    /* Put in test here for secondary root page read
       instead of !pagepos - done*/

    if(pagepos == cache->secrootblock)
	ajFatal("SecBucketIdlist: cannot read bucket from root page cache %S",
                cache->filename);

    page  = ajBtreeCacheRead(cache,pagepos);
    lpage = page;
    dirtysave = lpage->dirty;
    lpage->dirty = BT_LOCK;
    lpage->lockfor = 1321;

    buf = lpage->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);
    if(nodetype != BT_SECBUCKET)
	ajFatal("SecBucketIdlist: NodeType mismatch. "
                "Not secondary bucket (%u) cache %S",
		nodetype, cache->filename);
    
    GBT_BUCKNENTRIES(buf,&nentries);
    if(nentries > cache->snperbucket)
	ajFatal("SecBucketIdlist: Bucket too page: %Lu "
                "entries: %u max: %u full cache %S",
                pagepos, nentries, cache->snperbucket, cache->filename);
    

    GBT_BUCKOVERFLOW(buf,&overflow);
    pageoverflow = overflow;
    
    kptr  = PBT_BUCKKEYLEN(buf);
    codeptr = kptr + (nentries * sizeof(ajuint));

    for(i=0;i<nentries;++i)
    {
	BT_GETAJUINT(kptr,&len);
	if((codeptr-buf+1) + len > cache->pagesize)	/* overflow */
	{
	    page  = ajBtreeCacheRead(cache,overflow);
	    buf = page->buf;
	    GBT_BUCKNODETYPE(buf,&nodetype);
	    if(nodetype != BT_SECBUCKET)
		ajFatal("SecBucketIdlist: NodeType mismatch. "
                        "Not secondary bucket (%u) cache %S",
                        nodetype, cache->filename);
	    GBT_BUCKOVERFLOW(buf,&overflow);
	    /* overflow bucket ids start at the keylen position */
	    codeptr = PBT_BUCKKEYLEN(buf);
	}

	/* Fill ID objects */
        if(statSaveSecIdNext)
            tmpstr = statSaveSecId[--statSaveSecIdNext];
	ajStrAssignLenC(&tmpstr,(const char *)codeptr, len-1);
        /*ajStrFmtLower(&tmpstr);*/ /* already lowercase */
        ajListPushAppend(idlist, tmpstr);
        tmpstr = NULL;
	codeptr += len;

	kptr += sizeof(ajuint);
    }

    lpage->dirty = dirtysave;
    
    return pageoverflow;
}




/* @funcstatic btreeReadSecBucket *********************************************
**
** Constructor for keyword index secondary bucket given a disc page number
** Creates one empty key slot for possible addition
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
**
** @return [AjPSecBucket] bucket
** @@
******************************************************************************/

static AjPSecBucket btreeReadSecBucket(AjPBtcache cache, ajulong pagepos)
{
    AjPSecBucket bucket = NULL;

    AjPBtpage page  = NULL;
    AjPBtpage lpage = NULL;

    unsigned char *buf  = NULL;
    unsigned char *kptr = NULL;
    
    unsigned char *codeptr = NULL;
    
    ajuint  nodetype = 0;
    ajuint nentries = 0;
    ajulong overflow = 0L;
    ajuint  dirtysave = 0;
    
    ajuint  i;
    ajuint  len  = 0;
    
    /* ajDebug("In btreeReadSecBucket\n"); */

    if(pagepos == cache->secrootblock)
	ajFatal("btreeReadSecBucket: cannot read bucket from "
                "root page cache %S",
                cache->filename);

    page  = ajBtreeCacheRead(cache,pagepos);
    lpage = page;
    dirtysave = lpage->dirty;
    lpage->dirty = BT_LOCK;
    lpage->lockfor = 1331;

    buf = lpage->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);

    if(nodetype != BT_SECBUCKET)
	ajFatal("SecReadBucket: NodeType mismatch. "
                "Not secondary bucket (%u) cache %S",
		nodetype, cache->filename);
    
    GBT_BUCKNENTRIES(buf,&nentries);

    if(nentries > cache->snperbucket)
	ajFatal("SecReadBucket: Bucket too full page: %Lu "
                "entries: %u max: %u secondary: %B cache %S\n",
                pagepos, nentries, cache->snperbucket,
                cache->secondary, cache->filename);
    

    GBT_BUCKOVERFLOW(buf,&overflow);

    bucket = btreeSecBucketNew(cache->snperbucket,cache->kwlimit);
    bucket->Nentries = nentries;

    kptr  = PBT_BUCKKEYLEN(buf);
    codeptr = kptr + (nentries * sizeof(ajuint));

    for(i=0;i<nentries;++i)
    {
	BT_GETAJUINT(kptr,&len);

	if((codeptr-buf+1) + len > cache->pagesize)	/* overflow */
	{
#if AJINDEX_DEBUG
	    ajDebug("SecReadBucket: Overflow\n");
#endif
	    page  = ajBtreeCacheRead(cache,overflow);
	    buf = page->buf;
	    GBT_BUCKNODETYPE(buf,&nodetype);

	    if(nodetype != BT_SECBUCKET)
		ajFatal("SecReadBucket: NodeType mismatch. Not secondary "
			"bucket (%u)",nodetype);

	    GBT_BUCKOVERFLOW(buf,&overflow);
	    /* overflow bucket ids start at the keylen position */
	    codeptr = PBT_BUCKKEYLEN(buf);
	}

	/* Fill ID objects */
	ajStrAssignLenC(&bucket->SecIds[i],(const char *)codeptr,len-1);
	codeptr += len;

	kptr += sizeof(ajuint);
    }

    lpage->dirty = dirtysave;
    
    return bucket;
}




/* @funcstatic btreeWriteSecBucket *******************************************
**
** Write primary keyword index bucket object to the cache given a disc page
** number
**
** @param [u] cache [AjPBtcache] cache
** @param [r] bucket [const AjPSecBucket] bucket
** @param [r] pagepos [ajulong] page number
**
** @return [void]
** @@
******************************************************************************/

static void btreeWriteSecBucket(AjPBtcache cache, const AjPSecBucket bucket,
				ajulong pagepos)
{
    AjPBtpage page  = NULL;
    AjPBtpage lpage = NULL;

    unsigned char *buf  = NULL;
    unsigned char *lbuf = NULL;

    ajuint  v   = 0;
    ajuint i   = 0;
    ajuint len = 0;
    ajulong lv  = 0L;

    AjPStr sec = NULL;
    ajuint nentries = 0;
    ajulong overflow = 0L;
    unsigned char *keyptr = NULL;
    unsigned char *lptr   = NULL;
    ajulong   pno = 0L;

    /* ajuint unused = 0; */

    /* ajDebug("In btreeWriteSecBucket\n"); */

    if(pagepos == cache->totsize)	/* Create a new page */
    {
	pno = pagepos;
	page = ajBtreeCacheWriteBucketnew(cache);
	buf = page->buf;
	overflow = 0L;
    }
    else
    {
	page = ajBtreeCacheRead(cache,pagepos);
	buf = page->buf;
	GBT_BUCKOVERFLOW(buf,&overflow);
    }

    v = BT_SECBUCKET;
    SBT_BUCKNODETYPE(buf,v);

    lbuf = buf;
    page->dirty = BT_LOCK;
    page->lockfor = 1341;
    lpage = page;

    nentries = bucket->Nentries;
    v = nentries;
    SBT_BUCKNENTRIES(buf,v);

    /* Write out key lengths */
    keyptr = PBT_BUCKKEYLEN(lbuf);

    for(i=0;i<nentries;++i)
    {
	if((ajuint)((keyptr-lbuf+1)+sizeof(ajuint)) > cache->pagesize)
	    ajFatal("btreeWriteSecBucket: Bucket cannot hold more than %u keys",
		    i-1);

	sec = bucket->SecIds[i];
	/* Need to alter this if bucket primary keyword structure changes */
	len = BT_BUCKSECLEN(sec);
        v = len;
	BT_SETAJUINT(keyptr,v);
	keyptr += sizeof(ajuint);
    }


    /* Write out IDs using overflow if necessary */
    lptr = keyptr;

    for(i=0;i<nentries;++i)
    {
	sec = bucket->SecIds[i];
	len = BT_BUCKSECLEN(sec);

	if((lptr-buf+1)+len > cache->pagesize) /* overflow */
	{
#if AJINDEX_DEBUG
    	    ajDebug("btreeWriteSecBucket: Overflow\n");
#endif

	    if(!overflow)		/* No overflow buckets yet */
	    {
		pno = cache->totsize;
                lv = pno;
		SBT_BUCKOVERFLOW(buf,lv);
		page = ajBtreeCacheWriteBucketnew(cache);
		buf = page->buf;
		v = BT_SECBUCKET;
		SBT_BUCKNODETYPE(buf,v);
	    }
	    else
	    {
		page = ajBtreeCacheRead(cache,overflow);
		buf  = page->buf;
		GBT_BUCKOVERFLOW(buf,&overflow);
	    }

	    page->dirty = BT_DIRTY;

	    lptr = PBT_BUCKKEYLEN(buf);	    
	}
	
	sprintf((char *)lptr,"%s",MAJSTRGETPTR(sec));
	lptr += len;
    }

    lv = 0L;
    SBT_BUCKOVERFLOW(buf,lv);

    /*
    unused = cache->pagesize - (lptr - buf);
    if(unused > cache->pagesize)
        ajFatal("unused %u for pagesize %u", unused, cache->pagesize);
    if(unused > 0)
        memset(lptr, 0, unused);
    */

    lpage->dirty = BT_DIRTY;

    return;
}




/* @funcstatic btreeWriteSecBucketEmpty ***************************************
**
** Write empty secondary keyword index bucket object to the cache given a
** disc page number
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
**
** @return [void]
** @@
******************************************************************************/

static void btreeWriteSecBucketEmpty(AjPBtcache cache, ajulong pagepos)
{
    AjPBtpage page  = NULL;
    AjPBtpage lpage = NULL;

    unsigned char *buf  = NULL;

    ajuint  v   = 0;
    ajulong lv  = 0L;

    ajulong overflow = 0L;

    /* ajuint unused = 0; */

    if(pagepos == cache->totsize)	/* Create a new page */
    {
	page = ajBtreeCacheWriteBucketnew(cache);
	buf = page->buf;
	overflow = 0L;
	lv = overflow;
	SBT_BUCKOVERFLOW(buf,lv);
    }
    else
    {
	page = ajBtreeCacheRead(cache,pagepos);
	buf = page->buf;
	GBT_BUCKOVERFLOW(buf,&overflow);
    }

    v = BT_SECBUCKET;
    SBT_BUCKNODETYPE(buf,v);

    page->dirty = BT_LOCK;      /* cleared at end */
    page->lockfor = 1351;
    lpage = page;

    v = 0;
    SBT_BUCKNENTRIES(buf,v);

    lv = 0L;
    SBT_BUCKOVERFLOW(buf,lv);

    /*    
    lptr = PBT_BUCKKEYLEN(buf);  

    unused = cache->pagesize - (lptr - buf);
    if(unused > cache->pagesize)
        ajFatal("unused %u for pagesize %u", unused, cache->pagesize);
    if(unused > 0)
        memset(lptr, 0, unused);
    */

    lpage->dirty = BT_DIRTY;    /* clear the lock */

    return;
}




/* @func ajBtreeSecCacheNewC ************************************************
**
** Open a b+tree index file and initialise a cache object for keyword index
**
** @param [r] filetxt [const char *] name of file
** @param [r] exttxt [const char *] extension of file
** @param [r] idirtxt [const char *] index file directory
** @param [r] mode [const char *] opening mode
** @param [r] compressed [AjBool] Compressed index flag
** @param [r] kwlimit [ajuint] Max key size
** @param [r] pagesize [ajuint] pagesize
** @param [r] cachesize [ajuint] size of cache
** @param [r] pagecount [ajulong] page count
** @param [r] order [ajuint] Tree order
** @param [r] fill [ajuint] Number of entries per bucket
** @param [r] level [ajuint] level of tree
** @param [r] sorder [ajuint] order of secondary tree
** @param [r] sfill [ajuint] Number of entries per secondary bucket
** @param [r] count [ajulong] Number of entries in the index
** @param [r] countall [ajulong] Number of total entries in the index
**
** @return [AjPBtcache] initialised disc block cache structure
** @@
******************************************************************************/

AjPBtcache ajBtreeSecCacheNewC(const char *filetxt, const char *exttxt,
			       const char *idirtxt, const char *mode,
                               AjBool compressed, ajuint kwlimit,
                               ajuint pagesize, ajuint cachesize,
                               ajulong pagecount,
                               ajuint order, ajuint fill, ajuint level,
			       ajuint sorder, ajuint sfill,
			       ajulong count, ajulong countall)
{
    FILE *fp;
    AjPBtcache cache = NULL;
#if defined (usestat64)
    struct stat64 buf;
#else
    struct stat buf;
#endif
    ajulong filelen = 0L;
    AjBool douncompress = ajFalse;

    AJNEW0(cache);

    cache->filename = ajStrNew();
    if(!*idirtxt)
        ajFmtPrintS(&cache->filename,"%s.%s",filetxt,exttxt);
    else if(idirtxt[strlen(idirtxt)-1] == SLASH_CHAR)
        ajFmtPrintS(&cache->filename,"%s%s.%s",idirtxt,filetxt,exttxt);
    else
        ajFmtPrintS(&cache->filename,"%s%s%s.%s",
		    idirtxt,SLASH_STRING,filetxt,exttxt);
    
    fp = fopen(MAJSTRGETPTR(cache->filename),mode);
    if(!fp)
	return NULL;

    /* Commented out pending database updating */
    if(ajCharMatchC(mode, "rb"))
    {
#if defined (usestat64)
	if(!stat64(MAJSTRGETPTR(cache->filename), &buf))
#else
        if(!stat(MAJSTRGETPTR(cache->filename), &buf))
#endif
            filelen = buf.st_size;

        cache->readonly = ajTrue;
    }
    else if(ajCharMatchC(mode, "rb+"))
    {
#if defined (usestat64)
	if(!stat64(MAJSTRGETPTR(cache->filename), &buf))
#else
        if(!stat(MAJSTRGETPTR(cache->filename), &buf))
#endif
            filelen = buf.st_size;
        if(compressed)
            douncompress = ajTrue;
    }
    else if(!ajCharMatchC(mode, "wb+")) /* create */
    {
        ajWarn("ajBtreeSecCacheNewC unknown mode '%s'", mode);
    }
    

    cache->pagetable = ajTablelongNewConst(cachesize);

    cache->listLength = 0;

    cache->lru   = NULL;
    cache->mru   = NULL;
    cache->fp    = fp;
    
    cache->replace    = ajStrNew();
    cache->numreplace = 0L;
    
    if(pagesize>0)
	cache->pagesize = pagesize;
    else
	cache->pagesize = BT_PAGESIZE;

    cache->level     = level;

    cache->order      = order;
    cache->nperbucket = fill;
    cache->totsize    = filelen;
    cache->pagecount = pagecount;
    cache->cachesize  = cachesize;
    cache->filesize  = filelen;
    cache->extendsize = (ajulong) cache->pagesize * (ajulong) cache->cachesize;

    cache->sorder = sorder;
    cache->slevel = 0;
    cache->snperbucket = sfill;
    cache->countunique = count;
    cache->countall = countall;
    cache->kwlimit = kwlimit;
    cache->compressed = compressed;

    cache->bmem = NULL;
    cache->tmem = NULL;

    cache->bsmem = NULL;
    cache->tsmem = NULL;

    cache->secondary = ajTrue;
    
    if(douncompress)
        btreeCacheUncompress(cache);

    return cache;
}




/* @func ajBtreeSecCacheNewS ************************************************
**
** Open a b+tree index file and initialise a cache object for keyword index
**
** @param [r] file [const AjPStr] name of file
** @param [r] ext [const AjPStr] extension of file
** @param [r] idir [const AjPStr] index file directory
** @param [r] mode [const char *] opening mode
** @param [r] compressed [AjBool] Compressed index flag
** @param [r] kwlimit [ajuint] Max key size
** @param [r] pagesize [ajuint] pagesize
** @param [r] cachesize [ajuint] size of cache
** @param [r] pagecount [ajulong] page count
** @param [r] order [ajuint] Tree order
** @param [r] fill [ajuint] Number of entries per bucket
** @param [r] level [ajuint] level of tree
** @param [r] sorder [ajuint] order of secondary tree
** @param [r] sfill [ajuint] Number of entries per secondary bucket
** @param [r] count [ajulong] Number of entries in the index
** @param [r] countall [ajulong] Number of total entries in the index
**
** @return [AjPBtcache] initialised disc block cache structure
** @@
******************************************************************************/

AjPBtcache ajBtreeSecCacheNewS(const AjPStr file, const AjPStr ext,
			       const AjPStr idir, const char *mode,
                               AjBool compressed, ajuint kwlimit,
                               ajuint pagesize, ajuint cachesize,
                               ajulong pagecount,
                               ajuint order, ajuint fill, ajuint level,
			       ajuint sorder, ajuint sfill,
			       ajulong count, ajulong countall)
{
    return ajBtreeSecCacheNewC(MAJSTRGETPTR(file), MAJSTRGETPTR(ext),
                               MAJSTRGETPTR(idir), mode,
                               compressed, kwlimit, pagesize, cachesize,
                               pagecount, order, fill, level,
                               sorder, sfill,
                               count, countall);
}




/* @funcstatic btreeSplitSecLeaf *********************************************
**
** Split a leaf and propagate up if necessary
**
** @param [u] cache [AjPBtcache] cache
** @param [u] spage [AjPBtpage] page
**
** @return [AjPBtpage] pointer to a parent page
** @@
******************************************************************************/

static AjPBtpage btreeSplitSecLeaf(AjPBtcache cache, AjPBtpage spage)
{
    ajuint nkeys     = 0;
    ajuint order     = 0;
    ajuint totalkeys = 0;
    ajuint keylimit  = 0;
    ajuint nodetype  = 0;

    ajuint rootnodetype = 0;
    
    ajuint i;
    ajuint j;
    
    AjPBtpage lpage = NULL;
    AjPBtpage rpage = NULL;
    AjPBtpage page  = NULL;
    
    AjPStr mediankey  = NULL;
    ajulong mediangtr  = 0L;
    ajulong medianless = 0L;

    AjPStr bid = NULL;

    unsigned char *buf  = NULL;
    unsigned char *lbuf = NULL;
    unsigned char *rbuf = NULL;

    AjPList idlist = NULL;

    AjPSecBucket cbucket  = NULL;
    
    ajulong *parray = NULL;
    AjPBtMem arrays = NULL;
    AjPBtMem newarrays = NULL;
    AjPStr *newkarray = NULL;
    ajulong *newparray = NULL;
    
    ajuint lno    = 0;
    ajuint rno    = 0;

    ajuint lbucketlimit   = 0;
    ajuint rbucketlimit   = 0;
    ajuint lmaxnperbucket = 0;
    ajuint rmaxnperbucket = 0;
    ajuint nperbucket    = 0;
    ajuint count         = 0;
    
    ajulong lblockno = 0L;
    ajulong rblockno = 0L;
    ajulong overflow = 0L;
    ajulong prevsave = 0L;
    
    ajulong zero = 0L;
    ajulong join = 0L;
    
    ajulong lv = 0L;
    ajuint  v  = 0;
    ajuint newmax;
    ajuint iold=0;
    
#if AJINDEX_DEBUG
    ajDebug("btreeSplitSecLeaf %Lu\n", spage->pagepos);
#endif
    ++statCallSplitSecLeaf;

    /* ajDebug("In btreeSplitSecLeaf\n"); */

    if(!statSaveSecId)
    {
        statSaveSecIdMax = 2048;
        AJCNEW0(statSaveSecId, statSaveSecIdMax);
        statSaveSecIdNext = 0;
    }

    order = cache->sorder;
    nperbucket = cache->snperbucket;

    mediankey = ajStrNew();

    arrays = btreeAllocSecArray(cache);
    parray = arrays->parray;
    newarrays = btreeAllocSecArray(cache);
    newkarray = newarrays->karray;
    newparray = newarrays->parray;

    buf = spage->buf;
    lbuf = buf;

    GBT_NKEYS(buf,&nkeys);
    GBT_NODETYPE(buf,&rootnodetype);

    if(rootnodetype == BT_ROOT)
    {
	lblockno = cache->totsize;
	lpage = ajBtreeCacheWriteNodenew(cache);
	lbuf = lpage->buf;
	lv = cache->secrootblock;
	SBT_PREV(lbuf,lv);
    }
    else
    {
	lblockno = spage->pagepos;
	lpage = spage;
    }

    lpage->dirty = BT_LOCK;
    lpage->lockfor = 1361;

    rblockno = cache->totsize;
    rpage = ajBtreeCacheWriteNodenew(cache);
    rbuf = rpage->buf;
    rpage->dirty = BT_LOCK;
    rpage->lockfor = 1362;

    if(rootnodetype == BT_ROOT)
    {
	lv = zero;
	SBT_RIGHT(rbuf,lv);
	lv = zero;
	SBT_LEFT(lbuf,lv);
    }
    else
    {
	GBT_RIGHT(lbuf,&join);
	lv = join;
	SBT_RIGHT(rbuf,lv);
    }

    lv = lblockno;
    SBT_LEFT(rbuf,lv);
    lv = rblockno;
    SBT_RIGHT(lbuf,lv);


    btreeGetPointers(cache,buf,&parray);


    keylimit = nkeys+1;
    
    idlist = ajListNew();

    for(i=0;i<keylimit;++i)
        btreeSecBucketIdlist(cache, parray[i], idlist);

    ajListSort(idlist,btreeKeywordIdCompare);


    totalkeys = ajListGetLength(idlist);

    btreeBucketSplitCalc(totalkeys, keylimit, nperbucket,
                         &lbucketlimit,&lmaxnperbucket,&lno,
                         &rbucketlimit,&rmaxnperbucket,&rno);

    cbucket = btreeSecBucketNew(cache->snperbucket,cache->kwlimit);

    count = 0;
    iold=0;

    for(i=0;i<lbucketlimit;++i)
    {
	cbucket->Nentries = 0;

	for(j=0;j<lmaxnperbucket;++j)
	{
	    ajListPop(idlist,(void **)&bid);
	    

	    ajStrAssignS(&cbucket->SecIds[j],bid);

	    cbucket->keylen[j] = BT_BUCKSECLEN(bid);
	    ++count;
	    ++cbucket->Nentries;
            if(statSaveSecIdNext >= statSaveSecIdMax) 
            {
                newmax = statSaveSecIdMax + statSaveSecIdMax;
                AJCRESIZE0(statSaveSecId,statSaveSecIdMax,newmax);
                statSaveSecIdMax = newmax;
            }
            statSaveSecId[statSaveSecIdNext++] = bid;
            bid = NULL;
	}

	ajListPeek(idlist,(void **)&bid);
	
	ajStrAssignS(&newkarray[i],bid);

	if((iold < order) && parray[iold])
            newparray[i] = parray[iold++];
        else
	    newparray[i] = cache->totsize;
	btreeWriteSecBucket(cache,cbucket,newparray[i]);
    }

    cbucket->Nentries = 0;

    j = 0;

    while(count != lno)
    {
	ajListPop(idlist,(void **)&bid);

	ajStrAssignS(&cbucket->SecIds[j],bid);
	++j;
	++count;


	++cbucket->Nentries;
	ajStrDel(&bid);
    }

    if((iold < order) && parray[iold])
        newparray[i] = parray[iold++];
    else
        newparray[i] = cache->totsize;

    btreeWriteSecBucket(cache,cbucket,newparray[i]);

    nkeys = lbucketlimit;

    nodetype = BT_LEAF;
    v = nodetype;
    SBT_NODETYPE(lbuf,v);

    GBT_PREV(lbuf,&prevsave);
    lpage->dirty = BT_DIRTY;

    btreeWriteNode(cache,lpage,newkarray,newparray,nkeys);

    ajListPeek(idlist,(void **)&bid);
    ajStrAssignS(&mediankey,bid);

    for(i=0;i<rbucketlimit;++i)
    {
	cbucket->Nentries = 0;

	for(j=0;j<rmaxnperbucket;++j)
	{
	    ajListPop(idlist,(void **)&bid);
	    

	    ajStrAssignS(&cbucket->SecIds[j],bid);

	    cbucket->keylen[j] = BT_BUCKSECLEN(bid);
	    ++cbucket->Nentries;
	    ajStrDel(&bid);
	}

	ajListPeek(idlist,(void **)&bid);
	ajStrAssignS(&newkarray[i],bid);

	if((iold < order) && parray[iold])
            newparray[i] = parray[iold++];
        else
            newparray[i] = cache->totsize;

	btreeWriteSecBucket(cache,cbucket,newparray[i]);
    }

    cbucket->Nentries = 0;

    j = 0;

    while(ajListPop(idlist,(void**)&bid))
    {
	ajStrAssignS(&cbucket->SecIds[j],bid);
	++j;


	++cbucket->Nentries;
	ajStrDel(&bid);
    }
    
    if((iold < order) && parray[iold])
        newparray[i] = parray[iold++];
    else
        newparray[i] = cache->totsize;
    btreeWriteSecBucket(cache,cbucket,newparray[i]);

    nkeys = rbucketlimit;

    nodetype = BT_LEAF;
    v = nodetype;
    SBT_NODETYPE(rbuf,v);

    lv = prevsave;
    SBT_PREV(rbuf,lv);
    lv = overflow;
    SBT_OVERFLOW(rbuf,lv);
    
    btreeWriteNode(cache,rpage,newkarray,newparray,nkeys);
    rpage->dirty = BT_DIRTY;

    btreeSecBucketDel(&cbucket);
    ajListFree(&idlist);



    medianless = lblockno;
    mediangtr  = rblockno;

    btreeDeallocSecArray(cache,arrays);
    btreeDeallocSecArray(cache,newarrays);

    if(rootnodetype == BT_ROOT)
    {
	spage->dirty = BT_DIRTY;

	btreeWriteNodeSingle(cache,spage,mediankey,lblockno,rblockno);	

	++cache->slevel;
	lv = cache->slevel;
	SBT_RIGHT(buf,lv);
	spage->dirty = BT_LOCK;
        spage->lockfor = 1363;

	ajStrDel(&mediankey);

	return spage;
    }


    page = ajBtreeCacheRead(cache,prevsave);
    btreeInsertKeySec(cache,page,mediankey,medianless,mediangtr);
    ajStrDel(&mediankey);

    page = ajBtreeCacheRead(cache,prevsave);

    return page;
}




/* @funcstatic btreeKeywordIdCompare *****************************************
**
** Comparison function for ajListSort
**
** @param [r] a [const void*] ID 1
** @param [r] b [const void*] ID 2
**
** @return [ajint] 0 = bases match
** @@
******************************************************************************/

static ajint btreeKeywordIdCompare(const void *a, const void *b)
{
    return MAJSTRCMPS((*(AjPStr const *)a),
                      (*(AjPStr const *)b));
}




/* @funcstatic btreeSecBucketNew *********************************************
**
** Construct a primary keyword secondary bucket object
**
** @param [r] n [ajuint] Number of IDs
** @param [r] idlen [ajuint] Maximum size of IDs
**
** @return [AjPSecBucket] initialised disc block cache structure
** @@
******************************************************************************/

static AjPSecBucket btreeSecBucketNew(ajuint n, ajuint idlen)
{
    AjPSecBucket bucket = NULL;
    ajuint i;

    /* ajDebug("In btreeSecBucketNew\n"); */
    
    if(n)
    {
        if(statSaveSecBucketNext)
        {
            bucket = statSaveSecBucket[--statSaveSecBucketNext];
            for(i=0;i<bucket->Maxentries;++i)
                MAJSTRASSIGNCLEAR(&bucket->SecIds[i]);
            if(n > bucket->Maxentries)
            {
                AJCRESIZE0(bucket->keylen,bucket->Maxentries,n);
                AJCRESIZE0(bucket->SecIds,bucket->Maxentries,n);
                for(i=bucket->Maxentries;i<n;++i)
                    bucket->SecIds[i] = ajStrNewRes(idlen+1);
                bucket->Maxentries = n;
            }
            
        }
        else
        {
            AJNEW0(bucket);

            AJCNEW0(bucket->SecIds,n);
            AJCNEW0(bucket->keylen,n);
            for(i=0;i<n;++i)
                bucket->SecIds[i] = ajStrNewRes(idlen+1);
            bucket->Maxentries = n;
        }
    }
    else 
    {
        if(statSaveSecBucketEmptyNext)
            bucket = statSaveSecBucketEmpty[--statSaveSecBucketEmptyNext];
        else            
            AJNEW0(bucket);
    }
    
    bucket->NodeType = BT_SECBUCKET;
    bucket->Nentries = n;
    bucket->Overflow = 0L;
    
    return bucket;
}




/* @funcstatic btreeSecBucketDel *********************************************
**
** Delete a keyword primary bucket object
**
** @param [w] thys [AjPSecBucket*] bucket
**
** @return [void]
** @@
******************************************************************************/

static void btreeSecBucketDel(AjPSecBucket *thys)
{
    AjPSecBucket pthis = NULL;
    ajuint newmax;
    
    /* ajDebug("In btreeSecBucketDel\n"); */

    if(!thys || !*thys)
	return;

    pthis = *thys;

    if(!statSaveSecBucket)
    {
        statSaveSecBucketMax=2048;
        statSaveSecBucketNext=0;
        AJCNEW0(statSaveSecBucket,statSaveSecBucketMax);
    }
    
    if(!statSaveSecBucketEmpty)
    {
        statSaveSecBucketEmptyMax=2048;
        statSaveSecBucketEmptyNext=0;
        AJCNEW0(statSaveSecBucketEmpty,statSaveSecBucketEmptyMax);
    }
    

    /*
    statCountSecBucketDel++;
    statCountSecBucket--;
    if(pthis->Nentries > statMaxSecBucket)
      statMaxSecBucket = pthis->Nentries;
    */

    if(pthis->Maxentries)
    {
        /*statReusedSecBucket++;
          statUsedSecBucket--;*/
        if(statSaveSecBucketNext >= statSaveSecBucketMax)
        {
            newmax = statSaveSecBucketMax + statSaveSecBucketMax;
            AJCRESIZE0(statSaveSecBucket,statSaveSecBucketMax,newmax);
            statSaveSecBucketMax = newmax;
        }
        
        statSaveSecBucket[statSaveSecBucketNext++] = pthis;
        /*if(ajListGetLength(statListSecBucket) > statMaxFreeSecBucket)
          statMaxFreeSecBucket = ajListGetLength(statListSecBucket);*/
    }
    else
    {
        if(statSaveSecBucketEmptyNext >= statSaveSecBucketEmptyMax)
        {
            newmax = statSaveSecBucketEmptyMax + statSaveSecBucketEmptyMax;
            AJCRESIZE0(statSaveSecBucketEmpty,statSaveSecBucketEmptyMax,newmax);
            statSaveSecBucketEmptyMax = newmax;
        }
        statSaveSecBucketEmpty[statSaveSecBucketEmptyNext++] = pthis;
        /*statReusedSecBucketEmpty++;
          statUsedSecBucketEmpty--;*/
        /*if(ajListGetLength(statListSecBucketEmpty) > statMaxFreeSecBucketEmpty)
          statMaxFreeSecBucketEmpty = ajListGetLength(statListSecBucketEmpty);*/
    }
    
    pthis = NULL;

    *thys = NULL;

    return;
}




/* @funcstatic btreeSecBucketFree *********************************************
**
** Delete a keyword primary bucket object
**
** @param [w] thys [AjPSecBucket*] bucket
**
** @return [void]
** @@
******************************************************************************/

static void btreeSecBucketFree(AjPSecBucket *thys)
{
    AjPSecBucket pthis = NULL;
    ajuint n;
    ajuint i;
    
    /* ajDebug("In btreeSecBucketFree\n"); */

    if(!thys || !*thys)
	return;

    pthis = *thys;
    n = pthis->Maxentries;

    for(i=0;i<n;++i)
	ajStrDel(&pthis->SecIds[i]);
    
    AJFREE(pthis->keylen);
    AJFREE(pthis->SecIds);
    
    AJFREE(pthis);

    *thys = NULL;

    return;
}




/* @func ajBtreeSecInsertId *********************************************
**
** Insert a secondary ID into the tree
**
** @param [u] cache [AjPBtcache] cache
** @param [r] id [const AjPStr] Id
**
** @return [AjBool] True if ID was inserted
**                  False if ID already exists
** @@
******************************************************************************/

AjBool ajBtreeSecInsertId(AjPBtcache cache, const AjPStr id)
{
    AjPBtpage spage  = NULL;

    ajulong lblockno = 0L;
    ajulong rblockno = 0L;
    ajulong blockno  = 0L;
    ajulong shift    = 0L;

    ajuint nkeys = 0;

    ajuint nodetype = 0;
    
    unsigned char *buf = NULL;
 
    ajuint n;

    AjBool exists  = ajFalse;
    
    /* ajDebug("In ajBtreeSecInsertId\n"); */


    if(!MAJSTRGETLEN(id))
        return ajFalse;

    /* Only insert an ID if it doesn't exist */
    exists = ajBtreeSecFindId(cache,id);

    if(exists)
	return ajFalse;
    
    spage = ajBtreeSecFindInsert(cache,id);
    buf = spage->buf;

    GBT_NKEYS(buf,&nkeys);
    GBT_NODETYPE(buf,&nodetype);
    
    if(!nkeys)
    {
	lblockno = cache->totsize;
	btreeWriteSecBucketEmpty(cache,lblockno);

	rblockno = cache->totsize;
	btreeWriteSecBucketEmpty(cache,rblockno);	

	btreeWriteNodeSingle(cache,spage,id,lblockno,rblockno);

	btreeAddToSecBucket(cache,rblockno,id);

	return ajTrue;
    }
    
    blockno = btreeGetBlockS(cache,buf,id);
    
    if(nodetype != BT_ROOT)
	if((shift = btreeInsertShiftSec(cache,&spage,id)))
	    blockno = shift;

    buf = spage->buf;

    n = btreeNumInSecBucket(cache,blockno);

    if(n == cache->snperbucket)
    {
	if(btreeReorderSecBuckets(cache,spage))
	{
            blockno = btreeGetBlockS(cache,buf,id);
	}
	else
	{
	    btreeSplitSecLeaf(cache,spage);
	    
	    spage = ajBtreeSecFindInsert(cache,id);
	    buf = spage->buf;

            blockno = btreeGetBlockS(cache,buf,id);
	}
    }


    btreeAddToSecBucket(cache,blockno,id);

    return ajTrue;
}




/* @func ajBtreeSecFindId *****************************************************
**
** See whether ID already exists in the tree
**
** @param [u] cache [AjPBtcache] cache
** @param [r] key [const AjPStr] key
**
** @return [AjBool] true if ID already added
** @@
******************************************************************************/

AjBool ajBtreeSecFindId(AjPBtcache cache, const AjPStr key)
{
    AjPBtpage page      = NULL;
    unsigned char *buf = NULL;

    ajuint nkeys    = 0;
    
    ajulong blockno = 0L;
    AjBool found   = ajFalse;

    /* ajDebug("In ajBtreeSecFindId %u\n",cache->count); */

    if(!cache->countunique)
	return ajFalse;
    
    page = ajBtreeSecFindInsert(cache,key);
    buf = page->buf;
    GBT_NKEYS(buf,&nkeys);

    if(!nkeys)
	return ajFalse;
    
    blockno = btreeGetBlockS(cache,buf,key);
    found = btreeSecBucketFindId(cache, blockno, key);

    return found;
}




/* @func ajBtreeSecFindInsert ***********************************************
**
** Find the node that should contain a new key for insertion
**
** @param [u] cache [AjPBtcache] cache
** @param [r] key [const AjPStr] key to search for 
**
** @return [AjPBtpage] leaf node where item should be inserted
** @@
******************************************************************************/

AjPBtpage ajBtreeSecFindInsert(AjPBtcache cache, const AjPStr key)
{
    AjPBtpage root = NULL;
    AjPBtpage ret  = NULL;

    /* ajDebug("In ajBtreeSecFindInsert\n"); */

    /* The root node should always be in the cache (BT_LOCKed) */
    root = btreeCacheLocate(cache,cache->secrootblock);

    /* ajDebug("cache->slevel = %u root=%u\n",cache->slevel,(ajuint)root); */
    

    if(!cache->slevel)
	return root;
    
    ret = btreeSecFindINode(cache,root,key);

    return ret;
}




/* @funcstatic btreeNumInSecBucket *******************************************
**
** Return number of entries in a secondary id bucket
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
**
** @return [ajuint] Number of entries in bucket
** @@
******************************************************************************/

static ajuint btreeNumInSecBucket(AjPBtcache cache, ajulong pagepos)
{
    unsigned char *buf = NULL;

    AjPBtpage page = NULL;
    ajuint  nodetype = 0;
    ajuint nentries = 0;
    
    /* ajDebug("In btreeNumInPriBucket\n"); */

        
    if(pagepos == cache->secrootblock)
	ajFatal("NumInSecBucket: Attempt to read bucket from root page\n");


    page  = ajBtreeCacheRead(cache,pagepos);

    buf = page->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);

    if(nodetype != BT_SECBUCKET)
	ajFatal("SecReadBucket: NodeType mismatch. Not secondary bucket (%u)",
		nodetype);
    
    GBT_BUCKNENTRIES(buf,&nentries);

    return nentries;
}




/* @funcstatic btreeAddToSecBucket *******************************************
**
** Add a keyword ID to a primary bucket
** Only called if there is room in the bucket
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number of bucket
** @param [r] id [const AjPStr] ID
**
** @return [void]
** @@
******************************************************************************/

static void btreeAddToSecBucket(AjPBtcache cache, ajulong pagepos,
				const AjPStr id)
{
    unsigned char *buf  = NULL;
    unsigned char *kptr = NULL;
    unsigned char *src  = NULL;
    unsigned char *dest = NULL;

    unsigned char *lastptr = NULL;
    unsigned char *endptr  = NULL;
    
    ajuint nentries = 0;
    ajuint nodetype = 0;
    ajuint idlen    = 0;
    
    ajuint sum = 0;
    ajuint len = 0;
    ajuint i;

    ajuint v;
    
    AjPBtpage page = NULL;
    static ajuint calls = 0;
    static ajuint overflowcalls=0;
   
    calls++;
    page = ajBtreeCacheRead(cache,pagepos);
    buf  = page->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);
    if(nodetype != BT_SECBUCKET)
        ajFatal("Wrong nodetype in AddToSecBucket cache %S",
                cache->filename);

    GBT_BUCKNENTRIES(buf,&nentries);
    if(nentries >= cache->snperbucket)
        ajFatal("Bucket too full in AddToSecBucket page %Lu %u %u cache %S",
                pagepos, nentries, cache->snperbucket, cache->filename);    

    kptr = PBT_BUCKKEYLEN(buf);
    src  = kptr + (nentries * sizeof(ajuint));

    sum = 0;
    for(i=0;i<nentries;++i)
    {
        BT_GETAJUINT(kptr,&len);
        sum += len;
        kptr += sizeof(ajuint);
    }
    /*sum += nentries;*/
    
    endptr  = src + sum - 1;
    idlen   = MAJSTRGETLEN(id);
    lastptr = endptr + sizeof(ajuint) + idlen + 1;
    if((ajuint) (lastptr - buf) >= cache->pagesize)
    {
        overflowcalls++;
        ajWarn("\nOverflow in AddToSecBucket nentries:%u fails %u/%u '%S' "
               "cache %S",
               nentries, overflowcalls,calls, id, cache->filename);
        btreeAddToSecBucketFull(cache,pagepos,id);
        return;
    }
    
    dest = src + sizeof(ajuint);
    memmove((void *)dest, (void *)src, sum);

    v = idlen+1;
    BT_SETAJUINT(src,v);

    endptr += sizeof(ajuint) + 1;
    strcpy((char *)endptr,MAJSTRGETPTR(id));

    v = nentries + 1;
    SBT_BUCKNENTRIES(buf,v);

    page->dirty = BT_DIRTY;

    return;
}




/* @funcstatic btreeAddToSecBucketFull ****************************************
**
** Add a keyword ID to a secondary bucket
** Only called if there is room in the bucket
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number of bucket
** @param [r] id [const AjPStr] ID
**
** @return [void]
** @@
******************************************************************************/

static void btreeAddToSecBucketFull(AjPBtcache cache, ajulong pagepos,
                                    const AjPStr id)
{
    AjPSecBucket bucket = NULL;
    ajuint nentries;
    
    /* ajDebug("In btreeAddToSecBucket\n"); */

    bucket   = btreeReadSecBucket(cache,pagepos);
    nentries = bucket->Nentries;


    /* Reading a bucket always gives one extra ID position */

    ajStrAssignS(&bucket->SecIds[nentries],id);

    ++bucket->Nentries;

    btreeWriteSecBucket(cache,bucket,pagepos);

    btreeSecBucketDel(&bucket);
    
    return;
}




/* @funcstatic btreeReorderSecBuckets *****************************************
**
** Re-order secondary id leaf buckets
** Must only be called if one of the buckets is full
**
** @param [u] cache [AjPBtcache] cache
** @param [u] leaf [AjPBtpage] leaf page
**
** @return [AjBool] true if reorder was successful i.e. leaf not full
** @@
******************************************************************************/

static AjBool btreeReorderSecBuckets(AjPBtcache cache, AjPBtpage leaf)
{
    ajuint nkeys = 0;
    AjPSecBucket cbucket  = NULL;
    unsigned char *lbuf   = NULL;

    AjPBtMem arrays1 = NULL;
    AjPBtMem arrays2 = NULL;
    AjPStr *newkeys = NULL;
    ajulong *newptrs = NULL;
    ajulong *ptrs = NULL;
    ajulong *overflows = NULL;

    ajuint i = 0;
    
    ajuint order;
    ajuint totalkeys     = 0;
    ajuint maxnperbucket = 0;
    ajuint count         = 0;
    ajuint keylimit      = 0;
    ajuint bucketlimit   = 0;
    ajuint nodetype      = 0;
    
    AjPList idlist  = NULL;
    ajuint dirtysave = 0;
    AjPStr bid      = NULL;


    ajuint newmax;
    ajuint iold = 0;
    
#if AJINDEX_DEBUG
    ajDebug("btreeReorderSecBuckets %Lu\n", leaf->pagepos);
#endif
    ++statCallReorderSecBuckets;

    /* ajDebug("In btreeReorderSecBuckets\n"); */

    if(!statSaveSecId)
    {
        statSaveSecIdMax = 2048;
        AJCNEW0(statSaveSecId, statSaveSecIdMax);
        statSaveSecIdNext = 0;
    }

    dirtysave = leaf->dirty;

    leaf->dirty = BT_LOCK;
    leaf->lockfor = 1371;
    lbuf = leaf->buf;

    GBT_NODETYPE(lbuf,&nodetype);

    order = cache->sorder;

    /* Read keys/ptrs */
    arrays1 = btreeAllocSecArray(cache);
    ptrs = arrays1->parray;
    arrays2 = btreeAllocSecArray(cache);
    newkeys = arrays2->karray;
    newptrs = arrays2->parray;
    overflows = arrays2->overflows;
    btreeGetPointers(cache,lbuf,&ptrs);


    GBT_NKEYS(lbuf,&nkeys);


    if(!nkeys)
	ajFatal("ReorderSecBuckets: Attempt to reorder empty lea cache %Sf",
                cache->filename);

    if(nkeys > order)
    {
        ajErr("ReorderSecBuckets: nkeys %u > order %u cache %S",
              nkeys, order, cache->filename);
        ajUtilCatch();
    }
    
    for(i=0;i<nkeys;++i)
	totalkeys += btreeNumInSecBucket(cache,ptrs[i]);

    totalkeys += btreeNumInSecBucket(cache,ptrs[i]);

    keylimit = nkeys + 1;

    btreeBucketCalc(totalkeys, keylimit, cache->snperbucket,
                    &bucketlimit, &maxnperbucket);

    if(bucketlimit >= order)
    {
        btreeDeallocSecArray(cache,arrays1);
        btreeDeallocSecArray(cache,arrays2);
	leaf->dirty = dirtysave;

	return ajFalse;
    }
    

    /* Read IDs from all buckets and push to list and sort (increasing kw) */
    idlist  = ajListNew();
    
    for(i=0;i<keylimit;++i)
	overflows[i] = btreeSecBucketIdlist(cache,ptrs[i],idlist);


    ajListSort(idlist,btreeKeywordIdCompare);

    cbucket = btreeSecBucketNew(cache->snperbucket,cache->kwlimit);
    iold = 0;
    
    for(i=0;i<bucketlimit;++i)
    {
	cbucket->Overflow = overflows[i];
	cbucket->Nentries = 0;

	count = 0;
	while(count!=maxnperbucket)
	{
	    ajListPop(idlist,(void **)&bid);
	    
	    ajStrAssignS(&cbucket->SecIds[count],bid);

	    cbucket->keylen[count] = BT_BUCKSECLEN(bid);
	    ++cbucket->Nentries;
	    ++count;
            if(statSaveSecIdNext >= statSaveSecIdMax) 
            {
                newmax = statSaveSecIdMax + statSaveSecIdMax;
                AJCRESIZE0(statSaveSecId,statSaveSecIdMax,newmax);
                statSaveSecIdMax = newmax;
            }
            statSaveSecId[statSaveSecIdNext++] = bid;
            bid = NULL;
	}


	ajListPeek(idlist,(void **)&bid);
	ajStrAssignS(&newkeys[i],bid);

	if((iold < order) && ptrs[iold])
            newptrs[i]=ptrs[iold++];
        else
	    newptrs[i] = cache->totsize;
	btreeWriteSecBucket(cache,cbucket,newptrs[i]);
    }


    /* Deal with greater-than bucket */

    cbucket->Overflow = overflows[i];
    cbucket->Nentries = 0;

    count = 0;

    while(ajListPop(idlist,(void **)&bid))
    {
	ajStrAssignS(&cbucket->SecIds[count],bid);

	++cbucket->Nentries;
	++count;
	ajStrDel(&bid);
    }
    
    if((iold < order) && ptrs[iold])
        newptrs[i]=ptrs[iold++];
    else
        newptrs[i] = cache->totsize;
    btreeWriteSecBucket(cache,cbucket,newptrs[i]);

    btreeSecBucketDel(&cbucket);

#if AJINDEX_DEBUG
    if(bucketlimit <= keylimit)
        ajDebug("btreeReorderSecBuckets '%S' %u -> %u",
                cache->filename, keylimit, bucketlimit);
#endif

    for(i = bucketlimit + 1; i <= keylimit; i++)
    {
        btreePageSetfree(cache, ptrs[i]);
    }

    /* Now write out a modified leaf with new keys/ptrs */

    nkeys = bucketlimit;
    btreeWriteNode(cache,leaf,newkeys,newptrs,nkeys);
    leaf->dirty = BT_DIRTY;

    if(nodetype == BT_ROOT)
    {
	leaf->dirty = BT_LOCK;
        leaf->lockfor = 1372;
    }

    btreeDeallocSecArray(cache,arrays1);
    btreeDeallocSecArray(cache,arrays2);

    ajListFree(&idlist);

    return ajTrue;
}




#if 0
/* @funcstatic btreeInsertIdOnly *****************************************
**
** Add only a secondary ID: the primary keyword already exists
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pri [const AjPBtPri] keyword/id
**
** @return [void]
** @@
******************************************************************************/

static void btreeInsertIdOnly(AjPBtcache cache, const AjPBtPri pri)
{
    unsigned char *buf;
    AjPBtpage page = NULL;

    ajulong right = 0L;

    /* ajDebug("In btreeInsertIdOnly\n"); */
    
    if(!pri->treeblock)
    {
	fprintf(stderr,"btreeInsertIdOnly: root page doesn't exist\n");
	exit(-1);
    }
    
    cache->secrootblock = pri->treeblock;
    page = ajBtreeCacheWriteNode(cache,cache->secrootblock);
    page->dirty = BT_LOCK;
    page->lockfor = 1381;
    buf = page->buf;
    GBT_RIGHT(buf, &right);
    cache->slevel = (ajuint) right;

    ajBtreeSecInsertId(cache, pri->id);

    right = (ajulong) cache->slevel;
    SBT_RIGHT(buf, right);
    page->dirty = BT_DIRTY;

    return;
}
#endif




/* @funcstatic btreeSplitRootSec *****************************************
**
** Split a secondary root node
**
** @param [u] cache [AjPBtcache] cache
**
** @return [void]
** @@
******************************************************************************/

static void btreeSplitRootSec(AjPBtcache cache)
{
    AjPBtpage rootpage = NULL;
    AjPBtpage rpage    = NULL;
    AjPBtpage lpage    = NULL;
    AjPBtpage tpage    = NULL;

    AjPBtMem arrays = NULL;
    AjPBtMem tarrays = NULL;
    AjPStr *karray  = NULL;
    AjPStr *tkarray = NULL;
    ajulong *parray  = NULL;
    ajulong *tparray = NULL;

    ajuint order  = 0;
    ajuint nkeys  = 0;
    ajuint keypos = 0;
    
    ajulong rblockno = 0L;
    ajulong lblockno = 0L;

    ajulong right;
    
    AjPStr key = NULL;
    ajuint i;

    unsigned char *rootbuf = NULL;
    unsigned char *rbuf    = NULL;
    unsigned char *lbuf    = NULL;
    unsigned char *tbuf    = NULL;
    
    ajuint nodetype  = 0;
    ajulong overflow = 0L;
    ajulong zero     = 0L;
    ajuint totlen    = 0;
    ajuint rkeyno    = 0;
    ajuint n         = 0;

    ajulong lv = 0L;
    ajuint  v  = 0;
    
    
#if AJINDEX_DEBUG
    ajDebug("btreeSplitRootSec %Lu\n", cache->secrootblock);
#endif
    ++statCallSplitSecRoot;

    /* ajDebug("In btreeSplitRootSec\n"); */

    order = cache->sorder;

    arrays = btreeAllocSecArray(cache);
    tarrays = btreeAllocSecArray(cache);
    karray = arrays->karray;
    parray = arrays->parray;
    tkarray = tarrays->karray;
    tparray = tarrays->parray;

    rootpage = btreeCacheLocate(cache,cache->secrootblock);
    rootbuf = rootpage->buf;

    nkeys = order - 1;

    keypos = nkeys / 2;

    if(!(nkeys % 2))
	--keypos;


    rblockno = cache->totsize;
    rpage = ajBtreeCacheWriteNodenew(cache);
    rpage->dirty = BT_LOCK;
    rpage->lockfor = 1391;

    lblockno = cache->totsize;
    lpage = ajBtreeCacheWriteNodenew(cache);

    /* Comment this next block out after the beta test */
    if(!cache->slevel)
    {
	fprintf(stderr,"btreeSplitRootSec: Shouldn't get here\n");
	exit(0);
	lv = zero;
	SBT_LEFT(lpage->buf,lv);
	lv = rblockno;
	SBT_RIGHT(lpage->buf,lv);
	lv = lblockno;
	SBT_LEFT(rpage->buf,lv);
	lv = zero;
	SBT_RIGHT(rpage->buf,lv);
    }

    btreeGetKeys(cache,rootbuf,&karray,&parray);

    /* Get key for root node and write new root node */
    ajStrAssignS(&tkarray[0],karray[keypos]);
    tparray[0] = lblockno;
    tparray[1] = rblockno;
    

    n = 1;
    v = n;
    SBT_NKEYS(rootbuf,v);
    btreeWriteNode(cache,rootpage,tkarray,tparray,1);
    right = (ajulong)(cache->slevel + 1);
    SBT_RIGHT(rootbuf,right);

    rootpage->dirty = BT_LOCK;
    rootpage->lockfor = 1392;

    rbuf = rpage->buf;
    lbuf = lpage->buf;
    
    if(cache->slevel)
	nodetype = BT_INTERNAL;
    else
	nodetype = BT_LEAF;

    v = nodetype;
    SBT_NODETYPE(rbuf,v);
    v = nodetype;
    SBT_NODETYPE(lbuf,v);
    lv = overflow;
    SBT_OVERFLOW(rbuf,lv);
    lv = cache->secrootblock;
    SBT_PREV(rbuf,lv);
    lv = overflow;
    SBT_OVERFLOW(lbuf,lv);
    lv = cache->secrootblock;
    SBT_PREV(lbuf,lv);

    totlen = 0;

    for(i=0;i<keypos;++i)
    {
	ajStrAssignS(&tkarray[i],karray[i]);
	totlen += ajStrGetLen(karray[i]);
	tparray[i] = parray[i];
    }

    tparray[i] = parray[i];
    n = i;
    btreeWriteNode(cache,lpage,tkarray,tparray,n);

    for(i=0;i<=n;++i)
    {
	tpage = ajBtreeCacheRead(cache,tparray[i]);
	tbuf = tpage->buf;
	lv = lblockno;
	SBT_PREV(tbuf,lv);
	tpage->dirty = BT_DIRTY;
    }

    totlen = 0;

    for(i=keypos+1;i<nkeys;++i)
    {
	ajStrAssignS(&tkarray[i-(keypos+1)],karray[i]);
	totlen += ajStrGetLen(karray[i]);
	tparray[i-(keypos+1)] = parray[i];
    }

    tparray[i-(keypos+1)] = parray[i];

    rkeyno = (nkeys-keypos) - 1;
    rpage->dirty = BT_DIRTY;

    btreeWriteNode(cache,rpage,tkarray,tparray,rkeyno);

    for(i=0;i<rkeyno+1;++i)
    {
	tpage = ajBtreeCacheRead(cache,tparray[i]);
	tbuf = tpage->buf;
	lv = rblockno;
	SBT_PREV(tbuf,lv);
	tpage->dirty = BT_DIRTY;
    }


    ++cache->slevel;

    btreeDeallocSecArray(cache,arrays);
    btreeDeallocSecArray(cache,tarrays);

    ajStrDel(&key);
    
    return;
}




/* @funcstatic btreeInsertKeySec *****************************************
**
** Insert a secondary key into a potentially full node
**
** @param [u] cache [AjPBtcache] cache
** @param [u] page [AjPBtpage] original page
** @param [r] key [const AjPStr] key to insert
** @param [r] less [ajulong] less-than pointer
** @param [r] greater [ajulong] greater-than pointer
**
** @return [void]
** @@
******************************************************************************/

static void btreeInsertKeySec(AjPBtcache cache, AjPBtpage page,
			      const AjPStr key, ajulong less, ajulong greater)
{
    unsigned char *lbuf = NULL;
    unsigned char *rbuf = NULL;
    unsigned char *tbuf = NULL;

    AjPStr *karray  = NULL;
    ajulong *parray  = NULL;
    AjPStr *tkarray = NULL;
    ajulong *tparray = NULL;
    AjPBtMem savekeyarrays = NULL;
    AjPBtMem tarrays = NULL;

    ajuint nkeys  = 0;
    ajuint order  = 0;
    ajuint keypos = 0;
    ajuint rkeyno = 0;
    
    ajuint i = 0;
    ajuint n = 0;
    
    ajuint nodetype = 0;
    AjPBtpage ipage = NULL;
    AjPBtpage lpage = NULL;
    AjPBtpage rpage = NULL;
    AjPBtpage tpage = NULL;

    ajulong blockno  = 0L;
    ajulong rblockno = 0L;
    ajulong lblockno = 0L;
    ajulong ibn      = 0L;
    
    AjPStr mediankey  = NULL;
    ajulong medianless = 0L;
    ajulong mediangtr  = 0L;
    ajulong overflow   = 0L;
    ajulong prev       = 0L;
    ajuint  totlen     = 0;
    
    ajulong lv = 0L;
    ajuint  v  = 0;
    
    /* ajDebug("In btreeInsertKeySec\n"); */

    if(!btreeNodeIsFullSec(cache,page))
    {
	btreeInsertNonFullSec(cache,page,key,less,greater);

	return;
    }
    
    order = cache->sorder;
    lbuf = page->buf;
    GBT_NODETYPE(lbuf,&nodetype);
    page->dirty = BT_LOCK;
    page->lockfor = 1401;
    
    if(nodetype == BT_ROOT)
    {
	btreeSplitRootSec(cache);
	page->dirty = BT_DIRTY;
	
	blockno = btreeGetBlockFirstS(cache,lbuf,key);

	ipage = ajBtreeCacheRead(cache,blockno);
	btreeInsertNonFullSec(cache,ipage,key,less,greater);

	return;
    }

    savekeyarrays = btreeAllocSecArray(cache);
    tarrays = btreeAllocSecArray(cache);
    karray = savekeyarrays->karray;
    parray = savekeyarrays->parray;
    tkarray = tarrays->karray;
    tparray = tarrays->parray;

    lpage = page;
    lbuf = lpage->buf;
    
    btreeGetKeys(cache,lbuf,&karray,&parray);

    GBT_BLOCKNUMBER(lbuf,&lblockno);

    rblockno = cache->totsize;
    rpage = ajBtreeCacheWriteNodenew(cache);
    rbuf = rpage->buf;
    rpage->dirty = BT_LOCK;
    rpage->lockfor = 1402;
    
    GBT_PREV(lbuf,&prev);
    lv = prev;
    SBT_PREV(rbuf,lv);

    nkeys = order - 1;
    keypos = nkeys / 2;

    if(!(nkeys % 2))
	--keypos;

    mediankey = ajStrNewS(karray[keypos]);
    medianless = lblockno;
    mediangtr  = rblockno;


    GBT_NODETYPE(lbuf,&nodetype);
    v = nodetype;
    SBT_NODETYPE(rbuf,v);
    lv = overflow;
    SBT_OVERFLOW(rbuf,lv);


    totlen = 0;

    for(i=0;i<keypos;++i)
    {
	ajStrAssignS(&tkarray[i],karray[i]);
	totlen += ajStrGetLen(karray[i]);
	tparray[i] = parray[i];
    }

    tparray[i] = parray[i];
    n = i;
    btreeWriteNode(cache,lpage,tkarray,tparray,n);



    for(i=0;i<n+1;++i)
    {
	tpage = ajBtreeCacheRead(cache,tparray[i]);
	tbuf = tpage->buf;
	lv = lblockno;
	SBT_PREV(tbuf,lv);
	tpage->dirty = BT_DIRTY;
    }


    totlen = 0;

    for(i=keypos+1;i<nkeys;++i)
    {
	ajStrAssignS(&tkarray[i-(keypos+1)],karray[i]);
	totlen += ajStrGetLen(karray[i]);
	tparray[i-(keypos+1)] = parray[i];
    }

    tparray[i-(keypos+1)] = parray[i];
    rkeyno = (nkeys-keypos) - 1;
    rpage->dirty = BT_DIRTY;

    btreeWriteNode(cache,rpage,tkarray,tparray,rkeyno);


    for(i=0;i<rkeyno+1;++i)
    {
	tpage = ajBtreeCacheRead(cache,tparray[i]);
	tbuf = tpage->buf;
	lv = rblockno;
	SBT_PREV(tbuf,lv);
	tpage->dirty = BT_DIRTY;
    }


    ibn = rblockno;

    if(MAJSTRCMPS(key,mediankey)<0)
	ibn = lblockno;

    ipage = ajBtreeCacheRead(cache,ibn);
    
    btreeInsertNonFullSec(cache,ipage,key,less,greater);



    ipage = ajBtreeCacheRead(cache,prev);

    btreeInsertKeySec(cache,ipage,mediankey,medianless,mediangtr);
    btreeDeallocSecArray(cache,savekeyarrays); /* mediankey points here */
    btreeDeallocSecArray(cache,tarrays);

    ajStrDel(&mediankey);

    return;
}




/* @funcstatic btreeInsertShiftSec *****************************************
**
** Rebalance buckets on insertion
**
** @param [u] cache [AjPBtcache] cache
** @param [u] retpage [AjPBtpage*] page
** @param [r] key [const AjPStr] key
**
** @return [ajulong] bucket block or 0L if shift not possible
** @@
******************************************************************************/

static ajulong btreeInsertShiftSec(AjPBtcache cache, AjPBtpage *retpage,
                                   const AjPStr key)
{
    unsigned char *tbuf = NULL;
    unsigned char *pbuf = NULL;
    unsigned char *sbuf = NULL;

    AjPBtpage ppage = NULL;
    AjPBtpage spage = NULL;
    AjPBtpage tpage = NULL;

    ajuint tkeys = 0;
    ajuint pkeys = 0;
    ajuint skeys = 0;
    ajuint order = 0;
    
    ajuint i;
    ajuint n;
    ajint ii;
    
    ajulong parent  = 0L;
    ajulong blockno = 0L;
    
    AjPBtMem arrays1 = NULL;
    AjPBtMem arrays2 = NULL;
    AjPBtMem arrays3 = NULL;
    
    AjPStr *kTarray = NULL;
    AjPStr *kParray = NULL;
    AjPStr *kSarray = NULL;
    ajulong *pTarray = NULL;
    ajulong *pParray = NULL;
    ajulong *pSarray = NULL;

    AjPStr *karray = NULL;
    ajulong *parray = NULL;

    ajuint ppos    = 0;
    ajuint pkeypos = 0;
    ajuint minsize = 0;
    
    /* ajDebug("In btreeInsertShiftSec\n"); */


    tpage = *retpage;

    tbuf = tpage->buf;

    GBT_PREV(tbuf,&parent);
    GBT_NKEYS(tbuf,&tkeys);

    order = cache->sorder;
    minsize = order / 2;

    if(order % 2)
	++minsize;

    if(tkeys <= minsize)
	return 0L;

    
    ppage = ajBtreeCacheRead(cache,parent);
    pbuf = ppage->buf;
    GBT_NKEYS(pbuf,&pkeys);
    
    arrays1 = btreeAllocSecArray(cache);
    kParray = arrays1->karray;
    pParray = arrays1->parray;

    arrays2 = btreeAllocSecArray(cache);
    kSarray = arrays2->karray;
    pSarray = arrays2->parray;

    arrays3 = btreeAllocSecArray(cache);
    kTarray = arrays3->karray;
    pTarray = arrays3->parray;
    
    btreeGetKeys(cache,pbuf,&kParray,&pParray);

    i=0;

    while(i!=pkeys && MAJSTRCMPS(key,kParray[i])>=0)
	++i;

    pkeypos = i;
    
    if(i==pkeys)
    {
	if(MAJSTRCMPS(key,kParray[i-1])<0)
	    ppos = i-1;
	else
	    ppos = i;
    }
    else
	ppos = i;

    
    if(ppos) /* There is another leaf to the left */
    {
	spage = ajBtreeCacheRead(cache,pParray[ppos-1]);
	sbuf = spage->buf;
	GBT_NKEYS(sbuf,&skeys);
    }

    if(i && skeys != order-1) /* There is space in the left leaf */
    {
	/* ajDebug("Left shift\n"); */
	btreeGetKeys(cache,tbuf,&kTarray,&pTarray);
	if(skeys)
	    btreeGetKeys(cache,sbuf,&kSarray,&pSarray);

	i = 0;

	while(pParray[i] != tpage->pagepos)
	    ++i;

	--i;

	pkeypos = i;

	ajStrAssignS(&kSarray[skeys],kParray[pkeypos]);
	pSarray[skeys+1] = pTarray[0];
	++skeys;
	--tkeys;
	ajStrAssignS(&kParray[pkeypos],kTarray[0]);

	for(i=0;i<tkeys;++i)
	{
	    ajStrAssignS(&kTarray[i],kTarray[i+1]);
	    pTarray[i] = pTarray[i+1];
	}

	pTarray[i] = pTarray[i+1];
	pTarray[i+1] = 0L;
	
	btreeWriteNode(cache,spage,kSarray,pSarray,skeys);
	btreeWriteNode(cache,tpage,kTarray,pTarray,tkeys);
	btreeWriteNode(cache,ppage,kParray,pParray,pkeys);

	if(ppage->pagepos == cache->secrootblock)
        {
	    ppage->dirty = BT_LOCK;
            ppage->lockfor = 1411;
        }

	i = 0;

	while(i!=pkeys && MAJSTRCMPS(key,kParray[i])>=0)
	    ++i;

	if(i==pkeys)
	{
	    if(MAJSTRCMPS(key,kParray[i-1])<0)
		blockno = pParray[i-1];
	    else
		blockno = pParray[i];
	}
	else
	    blockno = pParray[i];

	if(blockno == spage->pagepos)
	{
	    *retpage = spage;
	    karray = kSarray;
	    parray = pSarray;
	    n = skeys;
	}
	else
	{
	    karray = kTarray;
	    parray = pTarray;
	    n = tkeys;
	}
	

	i = 0;

	while(i!=n && MAJSTRCMPS(key,karray[i])>=0)
	    ++i;

	if(i==n)
	{
	    if(MAJSTRCMPS(key,karray[i-1])<0)
		blockno = parray[i-1];
	    else
		blockno = parray[i];
	}
	else
	    blockno = parray[i];

        btreeDeallocSecArray(cache,arrays1);
        btreeDeallocSecArray(cache,arrays2);
        btreeDeallocSecArray(cache,arrays3);

	return blockno;
    }
    

    if(ppos != pkeys)	/* There is a right node */
    {
	spage = ajBtreeCacheRead(cache,pParray[ppos+1]);
	sbuf = spage->buf;
	GBT_NKEYS(sbuf,&skeys);
    }


    /* Space in the right leaf */
    if(ppos != pkeys && skeys != order-1)
    {
	/* ajDebug("Right shift\n");*/
	btreeGetKeys(cache,tbuf,&kTarray,&pTarray);
	btreeGetKeys(cache,sbuf,&kSarray,&pSarray);

	i = 0;

	while(pParray[i] != tpage->pagepos)
	    ++i;

	pkeypos = i;
	
	pSarray[skeys+1] = pSarray[skeys];

	for(ii=skeys-1;ii>-1;--ii)
	{
	    ajStrAssignS(&kSarray[ii+1],kSarray[ii]);
	    pSarray[ii+1] = pSarray[ii];
	}

	ajStrAssignS(&kSarray[0],kParray[pkeypos]);
	pSarray[0] = pTarray[tkeys];
	ajStrAssignS(&kParray[pkeypos],kTarray[tkeys-1]);
	++skeys;
	--tkeys;
	pTarray[tkeys+1] = 0L;
	
	btreeWriteNode(cache,spage,kSarray,pSarray,skeys);
	btreeWriteNode(cache,tpage,kTarray,pTarray,tkeys);
	btreeWriteNode(cache,ppage,kParray,pParray,pkeys);

	if(ppage->pagepos == cache->secrootblock)
        {
	    ppage->dirty = BT_LOCK;
            ppage->lockfor = 1412;
        }

	i = 0;

	while(i!=pkeys && MAJSTRCMPS(key,kParray[i])>=0)
	    ++i;

	if(i==pkeys)
	{
	    if(MAJSTRCMPS(key,kParray[i-1])<0)
		blockno = pParray[i-1];
	    else
		blockno = pParray[i];
	}
	else
	    blockno = pParray[i];

	if(blockno == spage->pagepos)
	{
	    *retpage = spage;
	    karray = kSarray;
	    parray = pSarray;
	    n = skeys;
	}
	else
	{
	    karray = kTarray;
	    parray = pTarray;
	    n = tkeys;
	}
	
	i = 0;

	while(i!=n && MAJSTRCMPS(key,karray[i])>=0)
	    ++i;

	if(i==n)
	{
	    if(MAJSTRCMPS(key,karray[i-1])<0)
		blockno = parray[i-1];
	    else
		blockno = parray[i];
	}
	else
	    blockno = parray[i];

        btreeDeallocSecArray(cache,arrays1);
        btreeDeallocSecArray(cache,arrays2);
        btreeDeallocSecArray(cache,arrays3);

	return blockno;
    }

    btreeDeallocSecArray(cache,arrays1);
    btreeDeallocSecArray(cache,arrays2);
    btreeDeallocSecArray(cache,arrays3);

    return 0L;
}




/* @funcstatic btreeInsertNonFullSec *****************************************
**
** Insert a key into a non-full node
**
** @param [u] cache [AjPBtcache] cache
** @param [u] page [AjPBtpage] original page
** @param [r] key [const AjPStr] key to insert
** @param [r] less [ajulong] less-than pointer
** @param [r] greater [ajulong] greater-than pointer
**
** @return [void]
** @@
******************************************************************************/

static void btreeInsertNonFullSec(AjPBtcache cache, AjPBtpage page,
				  const AjPStr key, ajulong less,
				  ajulong greater)
{
    unsigned char *buf = NULL;

    AjPBtMem arrays = NULL;
    AjPStr *karray = NULL;
    ajulong *parray = NULL;

    ajuint nkeys = 0;
    ajuint ipos  = 0;
    ajuint i;
    ajuint count = 0;

    ajulong lv = 0L;
    ajuint  v  = 0;
    

    AjPBtpage ppage = NULL;
    ajulong pagepos   = 0L;

    ajuint nodetype = 0;
    
    /* ajDebug("In btreeInsertNonFullSec\n"); */

    arrays = btreeAllocSecArray(cache);
    karray = arrays->karray;
    parray = arrays->parray;

    buf = page->buf;
    GBT_NKEYS(buf,&nkeys);
    GBT_NODETYPE(buf,&nodetype);
    
    btreeGetKeys(cache,buf,&karray,&parray);

    i = 0;

    while(i!=nkeys && MAJSTRCMPS(key,karray[i]) >= 0)
	++i;

    ipos = i;

    count = nkeys - ipos;
    

    if(ipos == nkeys)
    {
	ajStrAssignS(&karray[ipos],key);
	parray[ipos+1] = greater;
	parray[ipos]   = less;
    }
    else
    {
	parray[nkeys+1] = parray[nkeys];

	for(i=nkeys-1; count>0; --count, --i)
	{
	    ajStrAssignS(&karray[i+1],karray[i]);
	    parray[i+1] = parray[i];
	}

	ajStrAssignS(&karray[ipos],key);
	parray[ipos] = less;
	parray[ipos+1] = greater;
    }

    ++nkeys;
    v = nkeys;
    SBT_NKEYS(buf,v);

    btreeWriteNode(cache,page,karray,parray,nkeys);

    if(nodetype == BT_ROOT)
    {
	page->dirty = BT_LOCK;
        page->lockfor = 1421;
    }

    pagepos = page->pagepos;
    ppage = ajBtreeCacheRead(cache,less);
    lv = pagepos;
    SBT_PREV(ppage->buf,lv);
    ppage->dirty = BT_DIRTY;

    ppage = ajBtreeCacheRead(cache,greater);
    lv = pagepos;
    SBT_PREV(ppage->buf,lv);
    ppage->dirty = BT_DIRTY;

    btreeDeallocSecArray(cache,arrays);

    if(nodetype != BT_ROOT)
	btreeKeyShiftSec(cache,page);

    return;
}




/* @funcstatic btreeKeyShiftSec ********************************************
**
** Rebalance secondary Nodes on insertion
**
** @param [u] cache [AjPBtcache] cache
** @param [u] tpage [AjPBtpage] page
**
** @return [void]
** @@
******************************************************************************/

static void btreeKeyShiftSec(AjPBtcache cache, AjPBtpage tpage)
{
    unsigned char *tbuf = NULL;
    unsigned char *pbuf = NULL;
    unsigned char *sbuf = NULL;
    unsigned char *buf  = NULL;

    AjPBtpage ppage = NULL;
    AjPBtpage spage = NULL;
    AjPBtpage page  = NULL;

    ajuint tkeys = 0;
    ajuint pkeys = 0;
    ajuint skeys = 0;
    ajuint order = 0;
    
    ajuint i;
    ajint ii;
    
    ajulong parent = 0L;
    
    AjPBtMem Parrays = NULL;
    AjPBtMem Sarrays = NULL;
    AjPBtMem Tarrays = NULL;
    AjPStr *kTarray = NULL;
    AjPStr *kParray = NULL;
    AjPStr *kSarray = NULL;
    ajulong *pTarray = NULL;
    ajulong *pParray = NULL;
    ajulong *pSarray = NULL;

    ajuint pkeypos = 0;
    ajuint minsize = 0;

    ajulong lv = 0L;
    
    /* ajDebug("In btreeKeyShiftSec\n"); */
    
    tbuf = tpage->buf;

    GBT_PREV(tbuf,&parent);
    GBT_NKEYS(tbuf,&tkeys);

    order = cache->sorder;
    minsize = order / 2;

    if(order % 2)
	++minsize;

    if(tkeys <= minsize)
	return;

    
    ppage = ajBtreeCacheRead(cache,parent);
    pbuf = ppage->buf;
    GBT_NKEYS(pbuf,&pkeys);
    
    Parrays = btreeAllocSecArray(cache);
    Sarrays = btreeAllocSecArray(cache);
    Tarrays = btreeAllocSecArray(cache);
    kParray = Parrays->karray;
    pParray = Parrays->parray;
    kSarray = Sarrays->karray;
    pSarray = Sarrays->parray;
    kTarray = Tarrays->karray;
    pTarray = Tarrays->parray;

    btreeGetKeys(cache,tbuf,&kTarray,&pTarray);
    GBT_NKEYS(tbuf,&tkeys);


    btreeGetKeys(cache,pbuf,&kParray,&pParray);

    i=0;

    while(pParray[i] != tpage->pagepos)
	++i;

    if(i) /* There is another leaf to the left */
    {
	pkeypos = i-1;
	spage = ajBtreeCacheRead(cache,pParray[pkeypos]);
	sbuf = spage->buf;
	GBT_NKEYS(sbuf,&skeys);
	
    }

    if(i && skeys != order-1) /* There is space in the left leaf */
    {
	if(skeys)
	    btreeGetKeys(cache,sbuf,&kSarray,&pSarray);

	ajStrAssignS(&kSarray[skeys],kParray[pkeypos]);
	pSarray[skeys+1] = pTarray[0];
	++skeys;
	--tkeys;
	ajStrAssignS(&kParray[pkeypos],kTarray[0]);

	for(i=0;i<tkeys;++i)
	{
	    ajStrAssignS(&kTarray[i],kTarray[i+1]);
	    pTarray[i] = pTarray[i+1];
	}

	pTarray[i] = pTarray[i+1];
	pTarray[i+1] = 0L;
	
	btreeWriteNode(cache,spage,kSarray,pSarray,skeys);
	btreeWriteNode(cache,tpage,kTarray,pTarray,tkeys);
	btreeWriteNode(cache,ppage,kParray,pParray,pkeys);

	if(ppage->pagepos == cache->secrootblock)
        {
	    ppage->dirty = BT_LOCK;
            ppage->lockfor = 1431;
        }

	page = ajBtreeCacheRead(cache,pSarray[skeys]);
	buf = page->buf;
	lv = spage->pagepos;
	SBT_PREV(buf,lv);
	page->dirty = BT_DIRTY;


        btreeDeallocSecArray(cache,Parrays);
        btreeDeallocSecArray(cache,Sarrays);
        btreeDeallocSecArray(cache,Tarrays);

	return;
    }



    if(i != pkeys)	/* There is a right node */
    {
	pkeypos = i;
	spage = ajBtreeCacheRead(cache,pParray[pkeypos+1]);
	sbuf = spage->buf;
	GBT_NKEYS(sbuf,&skeys);
    }


    if(i != pkeys && skeys != order-1) /* Space in the right node */
    {
	if(skeys)
	    btreeGetKeys(cache,sbuf,&kSarray,&pSarray);

	pSarray[skeys+1] = pSarray[skeys];

	for(ii=skeys-1;ii>-1;--ii)
	{
	    ajStrAssignS(&kSarray[ii+1],kSarray[ii]);
	    pSarray[ii+1] = pSarray[ii];
	}

	ajStrAssignS(&kSarray[0],kParray[pkeypos]);
	pSarray[0] = pTarray[tkeys];
	ajStrAssignS(&kParray[pkeypos],kTarray[tkeys-1]);
	++skeys;
	--tkeys;
	pTarray[tkeys+1] = 0L;
	
	btreeWriteNode(cache,spage,kSarray,pSarray,skeys);
	btreeWriteNode(cache,tpage,kTarray,pTarray,tkeys);
	btreeWriteNode(cache,ppage,kParray,pParray,pkeys);

	if(ppage->pagepos == cache->secrootblock)
        {
	    ppage->dirty = BT_LOCK;
            ppage->lockfor = 1432;
        }

	page = ajBtreeCacheRead(cache,pSarray[0]);
	buf = page->buf;
	lv = spage->pagepos;
	SBT_PREV(buf,lv);
	page->dirty = BT_DIRTY;


        btreeDeallocSecArray(cache,Parrays);
        btreeDeallocSecArray(cache,Sarrays);
        btreeDeallocSecArray(cache,Tarrays);

	return;
    }

    btreeDeallocSecArray(cache,Parrays);
    btreeDeallocSecArray(cache,Sarrays);
    btreeDeallocSecArray(cache,Tarrays);

    return;
}




/* @func ajBtreeLockTest *****************************************************
**
** Test function: show if a primary tree root block is unlocked
**
** @param [u] cache [AjPBtcache] cache
**
** @return [void]
** @@
******************************************************************************/

void ajBtreeLockTest(AjPBtcache cache)
{
    AjPBtpage page = NULL;
    
    page = btreeCacheLocate(cache,0L);

    if(page->dirty != BT_LOCK)
	ajFatal("Root page unlocked\n");

    return;
}




/* @func ajBtreeSecLeafList **************************************************
**
** Read the leaves of a secondary tree
**
** @param [u] cache [AjPBtcache] cache
** @param [r] rootblock [ajulong] root page of secondary tree
**
** @return [AjPList] List of IDs matching a keyword
** @@
******************************************************************************/

AjPList ajBtreeSecLeafList(AjPBtcache cache, ajulong rootblock)
{
    AjPList list;

    AjPBtMem arrays = NULL;
    ajulong *parray;

    AjPBtpage page;
    unsigned char *buf;
    ajuint nodetype;
    ajuint j;
    ajuint k;
    ajulong level = 0L;
    
    AjPSecBucket bucket;
    ajuint nkeys;
    AjPStr id = NULL;
    ajulong right;

    list = ajListNew();

    arrays = btreeAllocSecArray(cache);
    parray = arrays->parray;
    
    page = ajBtreeCacheRead(cache,rootblock);
    buf = page->buf;

    GBT_RIGHT(buf,&level);
    cache->slevel = (ajuint)level;
    
    btreeGetPointers(cache,buf,&parray);
    GBT_NODETYPE(buf,&nodetype);

    while(nodetype != BT_LEAF && cache->slevel!=0)
    {
	page = ajBtreeCacheRead(cache,parray[0]);
	buf = page->buf;
	btreeGetPointers(cache,buf,&parray);
	GBT_NODETYPE(buf,&nodetype);
    }

    do
    {
	GBT_NKEYS(buf,&nkeys);

        page->dirty = BT_LOCK;
        page->lockfor = 9901;

	for(j=0;j<nkeys+1;++j)
	{
	    bucket = btreeReadSecBucket(cache, parray[j]);
	    
	    for(k=0;k<bucket->Nentries;++k)
	    {
		id = ajStrNew();
		ajStrAssignS(&id,bucket->SecIds[k]);
		ajListPush(list, (void *)id);
	    }

	    btreeSecBucketDel(&bucket);
	}

        page->dirty = BT_CLEAN;
	right = 0L;

	if(cache->slevel)
	{
	    GBT_RIGHT(buf,&right);

	    if(right)
	    {
		page = ajBtreeCacheRead(cache,right);	    
		buf = page->buf;
		btreeGetPointers(cache,buf,&parray);
	    }
	}
	
    } while(right);

    btreeDeallocSecArray(cache,arrays);

    return list;
}




/* @func ajBtreeVerifyId ********************************************
**
** Test routine: test for ID within a secondary tree
**
** @param [u] cache [AjPBtcache] cache
** @param [r] rootblock [ajulong] root page of secondary tree
** @param [r] id [const AjPStr] test ID
**
** @return [AjBool] true if ID found
** @@
******************************************************************************/

AjBool ajBtreeVerifyId(AjPBtcache cache, ajulong rootblock, const AjPStr id)
{
    AjPBtpage page;
    AjPBtpage spage;
    unsigned char *buf;
    ajulong blockno;
    AjBool found;
    ajulong right;
    ajuint dirtysave;

    cache->secrootblock = rootblock;
    page = ajBtreeCacheRead(cache,rootblock);
    dirtysave = page->dirty;
    page->dirty = BT_LOCK;
    page->lockfor = 1441;
    buf = page->buf;
    GBT_RIGHT(buf,&right);
    cache->slevel = (ajuint)right;
    
    spage = ajBtreeSecFindInsert(cache,id);

    if(!spage)
	return ajFalse;
    
    buf = spage->buf;
    
    blockno = btreeGetBlockS(cache,buf,id);

    found = btreeSecBucketFindId(cache, blockno, id);

    cache->secrootblock = 0L;
    page->dirty = dirtysave;

    return found;
}




/* @func ajBtreeIdFromKeywordW ********************************************
**
** Wildcard retrieval of keyword index entries
**
** @param [u] cache [AjPBtcache] cache
** @param [u] wild [AjPBtKeyWild] Wildcard
** @param [u] idcache [AjPBtcache] id cache
**
** @return [AjPBtId] next matching Id or NULL
** @@
******************************************************************************/

AjPBtId ajBtreeIdFromKeywordW(AjPBtcache cache, AjPBtKeyWild wild,
			      AjPBtcache idcache)
{

    AjPBtPri pri   = NULL;
    AjPBtpage page = NULL;
    AjPStr key      = NULL;
    AjPList list   = NULL;
    AjBool found   = ajFalse;
    AjPBtId btid   = NULL;
    AjPStr  id     = NULL;
    
    ajulong pagepos = 0L;

    unsigned char *buf = NULL;    

    key  = ajStrNewS(wild->keyword);
    ajStrFmtQuery(&key);
    if(MAJSTRGETLEN(key) > cache->kwlimit)
        ajStrTruncateLen(&key, cache->kwlimit);
    
    list = wild->list;

    found = ajFalse;
    
    if(wild->first)
    {
	page = ajBtreeFindInsertW(cache,key);
	page->dirty = BT_LOCK;
        page->lockfor = 1451;
	wild->pagepos = page->pagepos;
	
	btreeReadPriLeaf(cache,page,list);

	page->dirty = BT_CLEAN;
	
	if(!ajListGetLength(list))
        {
            ajStrDel(&key);
	    return NULL;
        }

	while(ajListPop(list,(void **)&pri))
	{
	    if(ajStrPrefixS(pri->keyword,key))
	    {
		found = ajTrue;
		break;
	    }
	    else
		ajBtreePriDel(&pri);
	}


	wild->first = ajFalse;

	if(found)
	{
	    cache->secrootblock = pri->treeblock;
	    btreeSecLeftLeaf(cache, wild);
	}
	else /* Check the next leaf just in case key==internal */
	{
	    buf = page->buf;
	    GBT_RIGHT(buf,&pagepos);

	    if(!pagepos)
            {
                ajStrDel(&key);
		return NULL;
            }

	    page = ajBtreeCacheRead(cache,pagepos);
	    wild->pagepos = pagepos;
	    page->dirty = BT_LOCK;
            page->lockfor = 1452;
	    
	    btreeReadPriLeaf(cache,page,list);	
	    
	    page->dirty = BT_CLEAN;
	    
	    if(!ajListGetLength(list))
            {
                ajStrDel(&key);
		return NULL;
	    }

	    found = ajFalse;

	    while(ajListPop(list,(void **)&pri))
	    {
		if(ajStrPrefixS(pri->keyword,key))
		{
		    found = ajTrue;
		    break;
		}
		else
		    ajBtreePriDel(&pri);
	    }

	    if(!found)
            {
                ajStrDel(&key);
		return NULL;
            }

	    cache->secrootblock = pri->treeblock;
	    btreeSecLeftLeaf(cache, wild);
	}


    }


    if(ajListGetLength(wild->idlist))
    {
	ajListPop(wild->idlist,(void **)&id);
	btid = ajBtreeIdFromKey(idcache,id);
	ajStrDel(&id);
        ajStrDel(&key);

	return btid;
    }
    else if((btreeSecNextLeafList(cache,wild)))
    {
	if(ajListGetLength(wild->idlist))
	{
	    ajListPop(wild->idlist,(void **)&id);
	    btid = ajBtreeIdFromKey(idcache,id);
	    ajStrDel(&id);
            ajStrDel(&key);

	    return btid;
	}

        ajStrDel(&key);
	return NULL;
    }
    

    /* Done for the current keyword so get the next one */

    if(!ajListGetLength(list))
    {
	page = ajBtreeCacheRead(cache,wild->pagepos); 
	buf = page->buf;
	GBT_RIGHT(buf,&pagepos);

	if(!pagepos)
        {
            ajStrDel(&key);
            return NULL;
        }

	page = ajBtreeCacheRead(cache,pagepos);
	wild->pagepos = pagepos;
	page->dirty = BT_LOCK;
        page->lockfor = 1453;

	btreeReadPriLeaf(cache,page,list);	

	page->dirty = BT_CLEAN;
	
	if(!ajListGetLength(list))
        {
            ajStrDel(&key);
	    return NULL;
        }
    }

    while(ajListPop(list,(void **)&pri))
    {
	if(ajStrPrefixS(pri->keyword,key))
	{
	    found = ajTrue;
	    break;
	}

	ajBtreePriDel(&pri);
    }

    ajStrDel(&key);

    if(!found)
	return NULL;

    cache->secrootblock = pri->treeblock;
    btreeSecLeftLeaf(cache, wild);
    
    if(ajListGetLength(wild->idlist))
    {
	ajListPop(wild->idlist,(void **)&id);
	btid = ajBtreeIdFromKey(idcache,id);
	ajStrDel(&id);
    }
    else
	return NULL;
    
    return btid;
}




/* @func ajBtreeListFromKeywordW ********************************************
**
** Wildcard retrieval of keyword index entries
**
** @param [u] cache [AjPBtcache] cache
** @param [r] key [const AjPStr] key
** @param [u] idcache [AjPBtcache] id cache
** @param [u] btidlist [AjPList] List of matching AjPBtId entries
**
** @return [void]
** @@
******************************************************************************/

void ajBtreeListFromKeywordW(AjPBtcache cache, const AjPStr key,
			     AjPBtcache idcache, AjPList btidlist)
{
    AjPBtPri pri     = NULL;
    AjPBtpage page   = NULL;
    AjPList prilist  = NULL;
    AjPList strlist  = NULL;

    AjPBtId btid = NULL;
    AjPStr id    = NULL;
    
    AjBool found     = ajFalse;
    AjBool finished  = ajFalse;
    
    ajulong pripagepossave = 0L;
    ajulong pagepos        = 0L;
    ajulong right         = 0L;
    
    unsigned char *buf = NULL;    

    AjPStr prefix = NULL;
    
    char *p;
    char *cp;
    
    ajDebug("ajBtreeListFromKeywordW '%S' list: %u\n",
            key, ajListGetLength(btidlist));

    prefix = ajStrNew();
    cp = MAJSTRGETPTR(key);
    p = strpbrk(cp,"*?");

    if(p)
    {
	if(p-cp)
	    ajStrAssignSubS(&prefix,key,0,p-cp-1);
	else
	{
	    ajStrDel(&prefix);
	    btreeKeywordFullSearch(cache,key,idcache,btidlist);

	    return;
	}
    }
    else
	ajStrAssignS(&prefix,key);
    
    ajDebug("ajBtreeListFromKeywordW prefix '%S' list: %u\n",
            prefix, ajListGetLength(btidlist));

    prilist  = ajListNew();
    
    found   = ajFalse;
    
    page = ajBtreeFindInsertW(cache,prefix);
    page->dirty = BT_LOCK;
    page->lockfor = 1461;
    pripagepossave = page->pagepos;
    btreeReadPriLeaf(cache,page,prilist);
    page->dirty = BT_CLEAN;

    if(!ajListGetLength(prilist))
    {
	ajStrDel(&prefix);
	ajListFree(&prilist);

	return;
    }


    while(ajListPop(prilist,(void **)&pri))
    {
	if(ajStrPrefixS(pri->keyword,prefix))
	{
	    found = ajTrue;
	    break;
	}
	else
	    ajBtreePriDel(&pri);
    }

    if(!found)	/* check next leaf in case key == internal */
    {
	buf = page->buf;
	GBT_RIGHT(buf,&pagepos);

	if(!pagepos)
	{
	    ajStrDel(&prefix);
	    ajListFree(&prilist);

	    return;
	}
	page = ajBtreeCacheRead(cache,pagepos);
	page->dirty = BT_LOCK;
        page->lockfor = 1462;
	pripagepossave = pagepos;
	btreeReadPriLeaf(cache,page,prilist);
	page->dirty = BT_CLEAN;

	if(!ajListGetLength(prilist))
	{
	    ajStrDel(&prefix);
	    ajListFree(&prilist);

	    return;
	}

	while(ajListPop(prilist,(void **)&pri))
	{
	    if(ajStrPrefixS(pri->keyword,prefix))
	    {
		found = ajTrue;
		break;
	    }
	    else
		ajBtreePriDel(&pri);
	}
    }
    
    if(!found)
    {
	ajStrDel(&prefix);
	ajListFree(&prilist);
	return;
    }

    finished = ajFalse;

    strlist  = ajListNew();

    while(!finished)
    {
	if(ajStrMatchWildS(pri->keyword,key))
	{
	    cache->secrootblock = pri->treeblock;
	    btreeReadAllSecLeaves(cache,strlist);
	}

	ajBtreePriDel(&pri);

	if(!ajListGetLength(prilist))
	{
	    page = ajBtreeCacheRead(cache,pripagepossave);

	    buf = page->buf;
	    GBT_RIGHT(buf,&right);

	    if(!right)
	    {
		finished = ajTrue;
		continue;
	    }
	    
	    page->dirty = BT_LOCK;
            page->lockfor = 1463;
	    btreeReadPriLeaf(cache,page,prilist);
	    page->dirty = BT_CLEAN;
	    pripagepossave = right;	    

	    if(!ajListGetLength(prilist))
	    {
		finished = ajTrue;
		continue;
	    }
	}
	

	ajListPop(prilist,(void **)&pri);

	if(!ajStrPrefixS(pri->keyword,prefix))
	{
	    ajBtreePriDel(&pri);
	    finished = ajTrue;
	}
    }
    

    while(ajListPop(prilist,(void **)&pri))
	ajBtreePriDel(&pri);

    ajListFree(&prilist);


    if(ajListGetLength(strlist))
    {
	ajListSortUnique(strlist,ajStrVcmp,btreeStrDel);

	while(ajListPop(strlist,(void **)&id))
	{
	    btid = ajBtreeIdFromKey(idcache,id);
            if(btid)
            {
                ajDebug("ajBtreeIdFromKey id: %S btid: '%S'\n",
                        id, btid->id);
                ajListPushAppend(btidlist,(void *)btid);
            }
            else
            {
                ajDebug("ajBtreeIdFromKey id: %S not found\n",
                        id);
            }
	    ajStrDel(&id);
	}
    }

    ajListFree(&strlist);

    ajStrDel(&prefix);

    return;
}




/* @funcstatic btreeKeywordFullSearch ****************************************
**
** Wildcard retrieval of key/des/org entries. Whole index scan. Only used for
** wildcard searches with keys beginning with '?' or '*'
**
** @param [u] cache [AjPBtcache] cache
** @param [r] key [const AjPStr] Wildcard key
** @param [u] idcache [AjPBtcache] id index cache
** @param [u] idlist [AjPList] list of matching AjPBtIds
**
** @return [void]
** @@
******************************************************************************/

static void btreeKeywordFullSearch(AjPBtcache cache, const AjPStr key,
				   AjPBtcache idcache, AjPList idlist)
{
    AjPBtPri pri   = NULL;
    AjPBtpage root = NULL;
    AjPBtpage page = NULL;
    AjPBtId btid   = NULL;
    ajulong right   = 0L;
    ajuint nodetype = 0;
    
    AjPBtMem arrays = NULL;
    ajulong *parray = NULL;

    AjPList list   = NULL;
    AjPList strlist = NULL;
    AjPStr id = NULL;
    
    unsigned char *buf = NULL;    

    list    = ajListNew();
    strlist = ajListNew();
    
    root = btreeCacheLocate(cache, 0L);
    page = root;
    
    buf = root->buf;
    GBT_NODETYPE(buf,&nodetype);
    
    if(cache->level)
    {
        arrays = btreeAllocPriArray(cache);
        parray = arrays->parray;
	while(nodetype != BT_LEAF)
	{
	    btreeGetPointers(cache,buf,&parray);
	    page = ajBtreeCacheRead(cache, parray[0]);
	    buf = page->buf;
	    GBT_NODETYPE(buf,&nodetype);
	    page->dirty = BT_CLEAN;
	}
        btreeDeallocPriArray(cache,arrays);
    }

    right = 99L;

    while(right)
    {
	btreeReadPriLeaf(cache,page,list);

	while(ajListPop(list,(void **)&pri))
	{
	    if(ajStrMatchWildS(pri->keyword,key))
	    {
		cache->secrootblock = pri->treeblock;
		btreeReadAllSecLeaves(cache,strlist);
	    }

	    ajBtreePriDel(&pri);
	}

	GBT_RIGHT(buf,&right);

	if(right)
	{
	    page = ajBtreeCacheRead(cache,right);
	    buf = page->buf;
	}
    }



    if(ajListGetLength(strlist))
    {
	ajListSortUnique(strlist,ajStrVcmp,btreeStrDel);

	while(ajListPop(strlist,(void **)&id))
	{
	    btid = ajBtreeIdFromKey(idcache,id);
	    ajListPushAppend(idlist,(void *)btid);
	    ajStrDel(&id);
	}
    }


    ajListFree(&strlist);

    ajListFree(&list);

    return;
}




/* @funcstatic btreeReadPriLeaf ***********************************************
**
** Read all primary index leaf KWs into a list
**
** @param [u] cache [AjPBtcache] cache
** @param [u] page [AjPBtpage] page
** @param [w] list [AjPList] list of AjPBtPri objects
**
** @return [void]
** @@
******************************************************************************/

static void btreeReadPriLeaf(AjPBtcache cache, AjPBtpage page, AjPList list)
{
    unsigned char *buf = NULL;
    AjPBtMem arrays    = NULL;
    ajulong *parray     = NULL;

    ajuint keylimit = 0;
    ajuint nkeys    = 0;
    
    ajuint i;
    
    /* ajDebug("In ReadPriLeaf\n"); */
    
    buf = page->buf;

    arrays = btreeAllocPriArray(cache);
    parray = arrays->parray;

    btreeGetPointers(cache,buf,&parray);

    GBT_NKEYS(buf,&nkeys);
    
    keylimit = nkeys+1;

    for(i=0;i<keylimit;++i)
	btreePriBucketIdlist(cache,parray[i],list);

    ajListSort(list,btreeKeywordCompare);

    btreeDeallocPriArray(cache,arrays);

    return;
}




/* @funcstatic btreeSecLeftLeaf ***********************************************
**
** Read all secondary index leaf IDs into a list from the lefthand-most
** leaf or the root node if the level is 0.
**
** @param [u] cache [AjPBtcache] cache
** @param [u] wild [AjPBtKeyWild] wildcard keyword object
**
** @return [void]
** @@
******************************************************************************/

static void btreeSecLeftLeaf(AjPBtcache cache, AjPBtKeyWild wild)
{
    AjPBtpage root = NULL;
    AjPBtpage page = NULL;
    ajulong right = 0L;
    ajuint nodetype = 0;
    ajuint nkeys = 0;
    ajuint keylimit = 0;
    
    ajuint i;
    
    AjPBtMem arrays = NULL;
    ajulong *parray = NULL;
    
    unsigned char *buf;
    

    root = ajBtreeCacheRead(cache, cache->secrootblock);
    root->dirty = BT_LOCK;
    root->lockfor = 1471;
   
    buf = root->buf;
    GBT_RIGHT(buf,&right);
    cache->slevel = (ajuint) right;
    GBT_NODETYPE(buf,&nodetype);
    
    arrays = btreeAllocSecArray(cache);
    parray = arrays->parray;

    btreeGetPointers(cache,buf,&parray);

    GBT_NODETYPE(buf,&nodetype);

    if(cache->slevel)
    {
	while(nodetype != BT_LEAF)
	{
	    btreeGetPointers(cache,buf,&parray);
	    page =ajBtreeCacheRead(cache, parray[0]);
	    buf = page->buf;
	    GBT_NODETYPE(buf,&nodetype);
	    page->dirty = BT_CLEAN;
	}

	wild->secpagepos = parray[0];
	btreeGetPointers(cache,buf,&parray);
    }
    else
	wild->secpagepos = cache->secrootblock;
    


    GBT_NKEYS(buf,&nkeys);
	

    keylimit = nkeys+1;
    for(i=0;i<keylimit;++i)
	btreeSecBucketIdlist(cache,parray[i],wild->idlist);

    ajListSort(wild->idlist,ajStrVcmp);
    
    root->dirty = BT_CLEAN;
    
    btreeDeallocSecArray(cache,arrays);

    cache->secrootblock = 0L;

    return;
}




/* @funcstatic btreeSecNextLeafList ******************************************
**
** Get next wadge of secondary index leaf IDs into a list after a successful
** wildcard keyword search
**
** @param [u] cache [AjPBtcache] cache
** @param [u] wild [AjPBtKeyWild] wildcard keyword object
**
** @return [AjBool] true if a wadge was successfully read
** @@
******************************************************************************/

static AjBool btreeSecNextLeafList(AjPBtcache cache, AjPBtKeyWild wild)
{
    AjPBtpage page = NULL;
    unsigned char *buf;
    ajulong right = 0L;
    ajuint nkeys = 0;
    ajuint keylimit = 0;
    ajuint i;
    
    AjPBtMem arrays = NULL;
    ajulong *parray = NULL;

    if(cache->secrootblock == wild->secpagepos)
	return ajFalse;

    
    page = ajBtreeCacheRead(cache,wild->secpagepos);
    buf = page->buf;
    GBT_RIGHT(buf,&right);
    page->dirty = BT_CLEAN;

    if(!right)
	return ajFalse;

    page = ajBtreeCacheRead(cache,right);
    wild->secpagepos = right;

    arrays = btreeAllocSecArray(cache);
    parray = arrays->parray;

    btreeGetPointers(cache,buf,&parray);
    GBT_NKEYS(buf,&nkeys);

    keylimit = nkeys + 1;
    
    for(i=0;i<keylimit;++i)
	btreeSecBucketIdlist(cache,parray[i],wild->idlist);
    
    ajListSort(wild->idlist,ajStrVcmp);
    
    btreeDeallocSecArray(cache,arrays);

    return ajTrue;
}




/* @funcstatic btreeReadAllSecLeaves ******************************************
**
** Read all the IDs from a secondary tree
**
** @param [u] cache [AjPBtcache] cache
** @param [u] list [AjPList] list of IDs to return
**
** @return [void]
** @@
******************************************************************************/

static void btreeReadAllSecLeaves(AjPBtcache cache, AjPList list)
{
    AjPBtpage root = NULL;
    AjPBtpage page = NULL;
    ajulong right = 0L;
    ajuint nodetype = 0;
    ajuint nkeys = 0;
    ajuint keylimit = 0;
    
    ajuint i;
    
    AjPBtMem arrays = NULL;
    ajulong *parray = NULL;
    
    unsigned char *buf;
    
    ajulong secpagepos = 0L;
    

    root = ajBtreeCacheRead(cache,cache->secrootblock);
    root->dirty = BT_LOCK;
    root->lockfor = 1481;
    buf = root->buf;
    GBT_RIGHT(buf,&right);
    cache->slevel = (ajuint) right;
    GBT_NODETYPE(buf,&nodetype);

    arrays = btreeAllocSecArray(cache);
    parray = arrays->parray;

    if(cache->slevel)
    {
	while(nodetype != BT_LEAF)
	{
	    btreeGetPointers(cache,buf,&parray);
	    page = ajBtreeCacheRead(cache,parray[0]);
	    buf = page->buf;
	    GBT_NODETYPE(buf,&nodetype);
	    page->dirty = BT_CLEAN;
	}

	secpagepos = parray[0];
    }
    else
	secpagepos = cache->secrootblock;


    btreeGetPointers(cache,buf,&parray);

    GBT_NKEYS(buf,&nkeys);
    keylimit = nkeys + 1;

    for(i=0;i<keylimit;++i)
	btreeSecBucketIdlist(cache,parray[i],list);

    if(cache->sorder)
    {
	page = ajBtreeCacheRead(cache,secpagepos);
	buf  = page->buf;
	GBT_RIGHT(buf,&right);
	page->dirty = BT_CLEAN;
    }


    while(right && secpagepos != cache->secrootblock)
    {
	secpagepos = right;
	page = ajBtreeCacheRead(cache,secpagepos);
	page->dirty = BT_LOCK;
        page->lockfor = 1482;
	buf = page->buf;

	btreeGetPointers(cache,buf,&parray);
	GBT_NKEYS(buf,&nkeys);
	keylimit = nkeys + 1;
	for(i=0; i < keylimit;++i)
            btreeSecBucketIdlist(cache,parray[i],list);

	GBT_RIGHT(buf,&right);
	page->dirty = BT_CLEAN;
    }


    root->dirty = BT_CLEAN;

    btreeDeallocSecArray(cache,arrays);

    return;
}




/* @funcstatic btreeStrDel ***************************************************
**
** Deletes an AjPStr entry from a list
**
** @param [r] Pstr [void**] Address of an AjPStr object
** @param [r] cl [void*] Standard unused argument, usually NULL.
** @return [void]
** @@
******************************************************************************/

static void btreeStrDel(void** Pstr, void* cl)
{
    AjPStr str = NULL;

    (void) cl;				/* make it used */

    str = *((AjPStr *)Pstr);
    ajStrDel(&str);

    return;
}




/* @funcstatic btreeIdDelFromList ********************************************
**
** Deletes an AjPBtId entry from a list
**
** @param [r] pentry [void**] Address of an AjPBtId object
** @param [r] cl [void*] Standard unused argument, usually NULL.
** @return [void]
** @@
******************************************************************************/

static void btreeIdDelFromList(void** pentry, void* cl)
{
    AjPBtId id = NULL;

    (void) cl;				/* make it used */

    id = *((AjPBtId *)pentry);

    ajDebug("btreeIdDelFromList '%S'\n", id->id);
    ajBtreeIdDel(&id);

    return;
}




/* @funcstatic btreeOffsetCompare *******************************************
**
** Comparison function for ajListUnique2
**
** @param [r] a [const void*] ID 1
** @param [r] b [const void*] ID 2
**
** @return [ajint] 0 = bases match
** @@
******************************************************************************/

static ajint btreeOffsetCompare(const void *a, const void *b)
{
    ajlong val;

    val = (ajlong) ((*(AjPBtId const *)a)->offset -
                    (*(AjPBtId const *)b)->offset);

    if(!val)
      return 0;

    return (val < 0L) ? -1 : 1;
}




/* @funcstatic btreeDbnoCompare *******************************************
**
** Second comparison function for ajListUnique2
**
** @param [r] a [const void*] ID 1
** @param [r] b [const void*] ID 2
**
** @return [ajint] 0 = bases match
** @@
******************************************************************************/

static ajint btreeDbnoCompare(const void *a, const void *b)
{
    return (ajuint) (*(AjPBtId const *)a)->dbno -
           (ajuint) (*(AjPBtId const *)b)->dbno;
}




/* @func ajBtreeHybNew *********************************************
**
** Constructor for index bucket ID information
**
**
** @return [AjPBtHybrid] Index ID object
** @@
******************************************************************************/

AjPBtHybrid ajBtreeHybNew(void)
{
    AjPBtHybrid Id = NULL;

    /* ajDebug("In ajBtreeHybNew\n"); */

    AJNEW0(Id);
    Id->key1 = ajStrNew();
    Id->dbno = 0;
    Id->dups = 0;
    Id->offset = 0L;
    Id->refoffset = 0L;
    Id->treeblock = 0L;    

    return Id;
}




/* @func ajBtreeHybDel *********************************************
**
** Destructor for index bucket ID information
**
** @param [w] Pthis [AjPBtHybrid*] index ID object
**
** @return [void]
** @@
******************************************************************************/

void ajBtreeHybDel(AjPBtHybrid *Pthis)
{
    AjPBtHybrid Id = NULL;

    /* ajDebug("In ajBtreeIdDel\n"); */

    if(!Pthis || !*Pthis)
	return;

    Id = *Pthis;
    
    ajStrDel(&Id->key1);
    AJFREE(Id);
    *Pthis = NULL;

    return;
}




/* @funcstatic btreeAllocPriArray *******************************************
**
** Allocate karray and parray arrays for a primary key
**
** @param [u] cache [AjPBtcache] cache
**
** @return [AjPBtMem] memory node
** @@
******************************************************************************/

static AjPBtMem btreeAllocPriArray(AjPBtcache cache)
{
    AjPBtMem node = NULL;
    ajuint i;
    ajuint limit;
    AjPBtMem p = NULL;
    ajuint klen;

    limit = cache->order;

    if(!cache->bmem)
    {
        statCountAllocPriArrayNew++;

        AJNEW0(node);
        cache->bmem = node;
        cache->tmem = node;
        node->prev = NULL;
        node->next = NULL;
        node->used = ajTrue;
        AJCNEW0(node->karray,limit);
        AJCNEW0(node->parray,limit);
        AJCNEW0(node->overflows,limit);

        klen = cache->kwlimit+1;
        for(i=0;i<limit;++i)
            node->karray[i] = ajStrNewRes(klen);

        return node;
    }

    if(!cache->bmem->used)
    {
        statCountAllocPriArrayReuse++;

        cache->bmem->used = ajTrue;

        if(cache->bmem->next)
        {
            p = cache->bmem->next;

            cache->tmem->next = cache->bmem;
            cache->bmem->next = NULL;

            cache->bmem->prev = cache->tmem;

            cache->tmem = cache->bmem;

            cache->bmem = p;
            cache->bmem->prev = NULL;

	    memset(cache->tmem->parray,0,sizeof(ajulong)*limit);
	    
	    return cache->tmem;
        }

	memset(cache->bmem->parray,0,sizeof(ajulong)*limit);
	
        return cache->bmem;
    }


    statCountAllocPriArrayNew++;

    AJNEW0(node);
    node->used = ajTrue;
    node->next = NULL;
    node->prev = cache->tmem;
    cache->tmem->next = node;
    cache->tmem = node;

    AJCNEW0(node->karray,limit);
    AJCNEW0(node->parray,limit);
    AJCNEW0(node->overflows,limit);

    klen = cache->kwlimit+1;
    for(i=0;i<limit;++i)
        node->karray[i] = ajStrNewRes(klen);

    return node;
}




/* @funcstatic btreeDeallocPriArray *******************************************
**
** Deallocate karray and parray arrays for a primary key
**
** @param [u] cache [AjPBtcache] cache
** @param [u] node [AjPBtMem] node
**
** @return [void]
** @@
******************************************************************************/

static void btreeDeallocPriArray(AjPBtcache cache, AjPBtMem node)
{
    statCountAllocPriArrayDel++;

    node->used = ajFalse;

    if(!node->prev)
        return;

    node->prev->next = node->next;

    if(node->next)
        node->next->prev = node->prev;
    else
        cache->tmem = node->prev;

    node->next = cache->bmem;
    cache->bmem->prev = node;
    cache->bmem = node;
    node->prev = NULL;

    return;
}




/* @funcstatic btreeAllocSecArray *******************************************
**
** Allocate karray and parray arrays for a primary key
**
** @param [u] cache [AjPBtcache] cache
**
** @return [AjPBtMem] memory node
** @@
******************************************************************************/

static AjPBtMem btreeAllocSecArray(AjPBtcache cache)
{
    AjPBtMem node = NULL;
    ajuint i;
    ajuint limit;
    AjPBtMem p = NULL;
    ajuint klen;

    limit = cache->sorder;

    if(!cache->bsmem)
    {
        statCountAllocSecArrayNew++;

        AJNEW0(node);
        cache->bsmem = node;
        cache->tsmem = node;
        node->prev = NULL;
        node->next = NULL;
        node->used = ajTrue;
        AJCNEW0(node->karray,limit);
        AJCNEW0(node->parray,limit);
        AJCNEW0(node->overflows,limit);

        klen = cache->kwlimit + 1;
        for(i=0;i<limit;++i)
            node->karray[i] = ajStrNewRes(klen);

        return node;
    }

    if(!cache->bsmem->used)
    {
        statCountAllocSecArrayReuse++;

        cache->bsmem->used = ajTrue;

        if(cache->bsmem->next)
        {
            p = cache->bsmem->next;

            cache->tsmem->next = cache->bsmem;
            cache->bsmem->next = NULL;

            cache->bsmem->prev = cache->tsmem;

            cache->tsmem = cache->bsmem;

            cache->bsmem = p;
            cache->bsmem->prev = NULL;

	    memset(cache->tsmem->parray,0,sizeof(ajulong)*limit);

	    return cache->tsmem;
        }

	memset(cache->bsmem->parray,0,sizeof(ajulong)*limit);

        return cache->bsmem;
    }

    statCountAllocSecArrayNew++;

    AJNEW0(node);
    node->used = ajTrue;
    node->next = NULL;
    node->prev = cache->tsmem;
    cache->tsmem->next = node;
    cache->tsmem = node;

    AJCNEW0(node->karray,limit);
    AJCNEW0(node->parray,limit);
    AJCNEW0(node->overflows,limit);

    klen = cache->kwlimit + 1;
    for(i=0;i<limit;++i)
        node->karray[i] = ajStrNewRes(klen);

    return node;
}




/* @funcstatic btreeDeallocSecArray *******************************************
**
** Deallocate karray and parray arrays for a primary key
**
** @param [u] cache [AjPBtcache] cache
** @param [u] node [AjPBtMem] node
**
** @return [void]
** @@
******************************************************************************/

static void btreeDeallocSecArray(AjPBtcache cache, AjPBtMem node)
{
    node->used = ajFalse;

    if(!node->prev)
        return;

    node->prev->next = node->next;

    if(node->next)
        node->next->prev = node->prev;
    else
        cache->tsmem = node->prev;

    node->next = cache->bsmem;
    cache->bsmem->prev = node;
    cache->bsmem = node;
    node->prev = NULL;

    return;
}




/* @funcstatic btreeAddToHybBucket *******************************************
**
** Add an ID to a bucket
** Only called if there is room in the bucket
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number of bucket
** @param [r] hyb [const AjPBtHybrid] ID info
**
** @return [void]
** @@
******************************************************************************/

static void btreeAddToHybBucket(AjPBtcache cache, ajulong pagepos,
				const AjPBtHybrid hyb)
{
    unsigned char *buf  = NULL;
    unsigned char *kptr = NULL;
    unsigned char *src  = NULL;
    unsigned char *dest = NULL;

    unsigned char *lastptr = NULL;
    unsigned char *endptr  = NULL;
    
    ajuint nentries = 0;
    ajuint nodetype = 0;
    ajuint idlen    = 0;
    
    ajuint sum = 0;
    ajuint len = 0;
    ajuint i;
    ajuint v;
    ajuint uv;
    ajulong lv;
    
    AjPBtpage page = NULL;
    static ajuint calls = 0;
    static ajuint overflowcalls=0;
   
    calls++;

    page = ajBtreeCacheRead(cache,pagepos);
    buf  = page->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);
    if(nodetype != BT_BUCKET)
        ajFatal("Wrong nodetype in AddToHybBucket cache %S", cache->filename);

    GBT_BUCKNENTRIES(buf,&nentries);
    if(nentries == cache->snperbucket)
        ajFatal("Bucket too full in AddToHybBucket page: %Lu "
                "entries: %u max: %u cache %S",
                pagepos, nentries, cache->snperbucket, cache->filename);

    kptr = PBT_BUCKKEYLEN(buf);
    src  = kptr + (nentries * sizeof(ajuint));

    sum = 0;
    for(i=0;i<nentries;++i)
    {
        BT_GETAJUINT(kptr,&len);
        sum += len;
        kptr += sizeof(ajuint);
    }
    /*sum += nentries;*/
    
    endptr  = src + sum - 1;
    idlen   = MAJSTRGETLEN(hyb->key1);
    lastptr = endptr + sizeof(ajuint) + BT_DDOFFROFF + idlen + 1;
    if((ajuint) (lastptr - buf) >= cache->pagesize)
    {
        overflowcalls++;
        ajWarn("\nOverflow in AddToHybBucket nentries:%u fails %u/%u '%S' "
               "cache %S",
               nentries, overflowcalls,calls, hyb->key1, cache->filename);
        btreeAddToHybBucketFull(cache,pagepos,hyb);
        return;
    }
    
    dest = src + sizeof(ajuint);
    memmove((void *)dest, (void *)src, sum);

    v = BT_BUCKIDLEN(hyb->key1);
    BT_SETAJUINT(src,v);

    endptr += sizeof(ajuint) + 1;
    strcpy((char *)endptr,MAJSTRGETPTR(hyb->key1));
    endptr += (MAJSTRGETLEN(hyb->key1) + 1);
    uv = hyb->dbno;
    BT_SETAJUINT(endptr,uv);
    endptr += sizeof(ajuint);
    uv = hyb->dups;
    BT_SETAJUINT(endptr,uv);
    endptr += sizeof(ajuint);
    lv = hyb->offset;
    BT_SETAJULONG(endptr,lv);
    endptr += sizeof(ajulong);
    lv = hyb->refoffset;
    BT_SETAJULONG(endptr,lv);
    endptr += sizeof(ajulong);

    v = nentries + 1;
    SBT_BUCKNENTRIES(buf,v);

    page->dirty = BT_DIRTY;

    return;
}




/* @funcstatic btreeAddToHybBucketFull ***************************************
**
** Add an ID to a bucket
** Only called if there is room in the bucket
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number of bucket
** @param [r] hyb [const AjPBtHybrid] ID info
**
** @return [void]
** @@
******************************************************************************/

static void btreeAddToHybBucketFull(AjPBtcache cache, ajulong pagepos,
                                    const AjPBtHybrid hyb)
{
    AjPBucket bucket = NULL;
    AjPBtId   destid = NULL;
    
    ajuint nentries;
    
    /* ajDebug("In btreeAddToHybBucket\n"); */

    bucket   = btreeReadBucket(cache,pagepos);
    nentries = bucket->Nentries;


    /* Reading a bucket always gives one extra ID position */
    destid = bucket->Ids[nentries];

    ajStrAssignS(&destid->id,hyb->key1);
    destid->dbno      = hyb->dbno;
    destid->offset    = hyb->offset;
    destid->refoffset = hyb->refoffset;
    destid->dups      = hyb->dups;
    
    ++bucket->Nentries;

    btreeWriteBucket(cache,bucket,pagepos);

    btreeBucketDel(&bucket);
    
    return;
}




/* @func ajBtreeHybFindInsert ***********************************************
**
** Find the node that should contain a new key for insertion
**
** @param [u] cache [AjPBtcache] cache
** @param [r] key [const AjPStr] key to search for 
**
** @return [AjPBtpage] leaf node where item should be inserted
** @@
******************************************************************************/

AjPBtpage ajBtreeHybFindInsert(AjPBtcache cache, const AjPStr key)
{
    AjPBtpage root = NULL;
    AjPBtpage ret  = NULL;

    /* ajDebug("In ajBtreeHybFindInsert\n"); */

    /* The root node should always be in the cache (BT_LOCKed) */
    root = btreeCacheLocate(cache,0L);

    if(!root)
	ajFatal("The PRI root cache page has been unlocked\n");
    
    if(!cache->level)
	return root;
    
    ret = btreeHybFindINode(cache,root,key);
    
    return ret;
}




/* @funcstatic btreeHybFindINode *********************************************
**
** Recursive search for insert node
**
** @param [u] cache [AjPBtcache] cache
** @param [u] page [AjPBtpage] page
** @param [r] item [const AjPStr] key to search for 
**
** @return [AjPBtpage] leaf node where item should be inserted
** @@
******************************************************************************/

static AjPBtpage btreeHybFindINode(AjPBtcache cache, AjPBtpage page,
                                   const AjPStr item)
{
    AjPBtpage ret = NULL;
    AjPBtpage pg  = NULL;

    unsigned char *buf = NULL;
    ajuint status       = 0;
    ajuint ival         = 0;

#if AJINDEX_DEBUG
    ajDebug("In btreeHybFindINode %Lu dirty:%u lockfor:%u\n",
            page->pagepos, page->dirty, page->lockfor);
#endif

    ret = page;
    buf = page->buf;
    GBT_NODETYPE(buf,&ival);

    if(ival != BT_LEAF)
    {
	status = ret->dirty;
	ret->dirty = BT_LOCK;	/* Lock in case of lots of overflow pages */
        ret->lockfor = 1491;
	pg = btreeHybPageFromKey(cache,buf,item);
	ret->dirty = status;
	ret = btreeHybFindINode(cache,pg,item);
#if AJINDEX_DEBUG
        ajDebug("btreeHybFindINode (%Lu) %Lu -> %Lu level:%u\n",
                page->pagepos, pg->pagepos, ret->pagepos, cache->level);
#endif
    }
    
    return ret;
}




/* @funcstatic btreeHybPageFromKey *******************************************
**
** Return next lower index page given a key
**
** @param [u] cache [AjPBtcache] cache
** @param [u] buf [unsigned char *] page buffer
** @param [r] key [const AjPStr] key to search for 
**
** @return [AjPBtpage] pointer to a page
** @@
******************************************************************************/

static AjPBtpage btreeHybPageFromKey(AjPBtcache cache, unsigned char *buf,
				     const AjPStr key)
{
    ajulong blockno = 0L;
    AjPBtpage page = NULL;
    
    /* ajDebug("In btreeHybPageFromKey\n"); */
    
    blockno = btreeGetBlockS(cache,buf,key);

    page =  ajBtreeCacheRead(cache,blockno);

    return page;
}




/* @funcstatic btreeHybInsertShift ********************************************
**
** Rebalance buckets on insertion
**
** @param [u] cache [AjPBtcache] cache
** @param [u] retpage [AjPBtpage*] page
** @param [r] key [const AjPStr] key
**
** @return [ajulong] bucket block or 0L if shift not possible
** @@
******************************************************************************/

static ajulong btreeHybInsertShift(AjPBtcache cache, AjPBtpage *retpage,
                                   const AjPStr key)
{
    unsigned char *tbuf = NULL;
    unsigned char *pbuf = NULL;
    unsigned char *sbuf = NULL;

    AjPBtpage ppage = NULL;
    AjPBtpage spage = NULL;
    AjPBtpage tpage = NULL;

    ajuint tkeys = 0;
    ajuint pkeys = 0;
    ajuint skeys = 0;
    ajuint order = 0;
    
    ajuint i;
    ajuint n;
    ajint ii;

    ajulong parent  = 0L;
    ajulong blockno = 0L;
    
    AjPStr *kTarray = NULL;
    AjPStr *kParray = NULL;
    AjPStr *kSarray = NULL;
    ajulong *pTarray = NULL;
    ajulong *pParray = NULL;
    ajulong *pSarray = NULL;

    AjPStr *karray = NULL;
    ajulong *parray = NULL;

    ajuint ppos    = 0;
    ajuint pkeypos = 0;
    ajuint minsize = 0;

    AjPBtMem arrays1 = NULL;
    AjPBtMem arrays2 = NULL;
    AjPBtMem arrays3 = NULL;
    
    tpage = *retpage;

    tbuf = tpage->buf;

    GBT_PREV(tbuf,&parent);
    GBT_NKEYS(tbuf,&tkeys);

#if AJINDEX_DEBUG
    ajDebug("btreeHybInsertShift %Lu parent: %Lu '%S'\n",
            (*retpage)->pagepos, parent, key);
#endif

    order = cache->order;
    minsize = order / 2;

    if(order % 2)
	++minsize;

    if(tkeys <= minsize)
	return 0L;

    ppage = ajBtreeCacheRead(cache,parent);
    
    pbuf = ppage->buf;
    GBT_NKEYS(pbuf,&pkeys);
    

    arrays1 = btreeAllocPriArray(cache);
    kParray = arrays1->karray;
    pParray = arrays1->parray;

    arrays2 = btreeAllocPriArray(cache);
    kSarray = arrays2->karray;
    pSarray = arrays2->parray;

    arrays3 = btreeAllocPriArray(cache);
    kTarray = arrays3->karray;
    pTarray = arrays3->parray;
    

    btreeGetKeys(cache,pbuf,&kParray,&pParray);

    i=0;

    while(i!=pkeys && MAJSTRCMPS(key,kParray[i])>=0)
	++i;

    pkeypos = i;
    
    if(i==pkeys)
    {
	if(MAJSTRCMPS(key,kParray[i-1])<0)
	    ppos = i-1;
	else
	    ppos = i;
    }
    else
	ppos = i;

    
#if AJINDEX_DEBUG
    ajDebug("    InsertShift tpage: %Lu ppage: %Lu ppos: %u i: %u "
            "skeys: %u order: %u\n",
            tpage->pagepos, ppage->pagepos, ppos, i, skeys, order);
#endif

    if(ppos) /* There is another leaf to the left */
    {
	spage = ajBtreeCacheRead(cache,pParray[ppos-1]);
	sbuf = spage->buf;
	GBT_NKEYS(sbuf,&skeys);
    }

    if(i && skeys != order-1) /* There is space in the left leaf */
    {

#if AJINDEX_DEBUG
    ajDebug("    InsertShift left leaf\n");
#endif

	/* ajDebug("Left shift\n"); */
	btreeGetKeys(cache,tbuf,&kTarray,&pTarray);
	if(skeys)
	    btreeGetKeys(cache,sbuf,&kSarray,&pSarray);

	i = 0;

	while(pParray[i] != tpage->pagepos)
	    ++i;
	--i;

	pkeypos = i;

	ajStrAssignS(&kSarray[skeys],kParray[pkeypos]);
	pSarray[skeys+1] = pTarray[0];
	++skeys;
	--tkeys;
	ajStrAssignS(&kParray[pkeypos],kTarray[0]);
	for(i=0;i<tkeys;++i)
	{
	    ajStrAssignS(&kTarray[i],kTarray[i+1]);
	    pTarray[i] = pTarray[i+1];
	}
	pTarray[i] = pTarray[i+1];
	pTarray[i+1] = 0L;
	
	btreeWriteNode(cache,spage,kSarray,pSarray,skeys);
	btreeWriteNode(cache,tpage,kTarray,pTarray,tkeys);
	btreeWriteNode(cache,ppage,kParray,pParray,pkeys);

	if(!ppage->pagepos)
        {
	    ppage->dirty = BT_LOCK;
            ppage->lockfor = 1501;
        }

	i = 0;

	while(i!=pkeys && MAJSTRCMPS(key,kParray[i])>=0)
	    ++i;

	if(i==pkeys)
	{
	    if(MAJSTRCMPS(key,kParray[i-1])<0)
		blockno = pParray[i-1];
	    else
		blockno = pParray[i];
	}
	else
	    blockno = pParray[i];

	if(blockno == spage->pagepos)
	{
	    *retpage = spage;
	    karray = kSarray;
	    parray = pSarray;
	    n = skeys;
	}
	else
	{
	    karray = kTarray;
	    parray = pTarray;
	    n = tkeys;
	}
	

	i = 0;

	while(i!=n && MAJSTRCMPS(key,karray[i])>=0)
	    ++i;

	if(i==n)
	{
	    if(MAJSTRCMPS(key,karray[i-1])<0)
		blockno = parray[i-1];
	    else
		blockno = parray[i];
	}
	else
	    blockno = parray[i];

	btreeDeallocPriArray(cache,arrays1);
	btreeDeallocPriArray(cache,arrays2);
	btreeDeallocPriArray(cache,arrays3);

	/* ajDebug("... returns blockno (a) %Lu\n",blockno); */

	return blockno;
    }
    

    if(ppos != pkeys)	/* There is a right node */
    {
	spage = ajBtreeCacheRead(cache,pParray[ppos+1]);
	sbuf = spage->buf;
	GBT_NKEYS(sbuf,&skeys);

#if AJINDEX_DEBUG
        ajDebug("    InsertShift right, new spage: %Lu skeys: %u\n",
                spage->pagepos, skeys);
#endif

    }


#if AJINDEX_DEBUG
    ajDebug("    InsertShift test again tpage: %Lu ppage: %Lu spage: %Lu "
            "ppos: %u i: %u skeys: %u order: %u\n",
            tpage->pagepos, ppage->pagepos, spage->pagepos,
            ppos, i, skeys, order);
#endif

    /* Space in the right leaf */
    if(ppos != pkeys && skeys != order-1)
    {

#if AJINDEX_DEBUG
        ajDebug("    InsertShift right leaf tpage: %Lu\n", tpage->pagepos);
#endif

	/* ajDebug("Right shift\n"); */
	btreeGetKeys(cache,tbuf,&kTarray,&pTarray);
	btreeGetKeys(cache,sbuf,&kSarray,&pSarray);

	i = 0;

	while(pParray[i] != tpage->pagepos) /* fails here */
        {

#if AJINDEX_DEBUG
            ajDebug("    InsertShift right pParray[%u] %Lu\n",
                    i, pParray[i]);
#endif
	    ++i;
        }
        
	pkeypos = i;
	
	pSarray[skeys+1] = pSarray[skeys];

	for(ii=skeys-1;ii>-1;--ii)
	{
	    ajStrAssignS(&kSarray[ii+1],kSarray[ii]);
	    pSarray[ii+1] = pSarray[ii];
	}

	ajStrAssignS(&kSarray[0],kParray[pkeypos]);
	pSarray[0] = pTarray[tkeys];
	ajStrAssignS(&kParray[pkeypos],kTarray[tkeys-1]);
	++skeys;
	--tkeys;
	pTarray[tkeys+1] = 0L;
	
	btreeWriteNode(cache,spage,kSarray,pSarray,skeys);
	btreeWriteNode(cache,tpage,kTarray,pTarray,tkeys);
	btreeWriteNode(cache,ppage,kParray,pParray,pkeys);

	if(!ppage->pagepos)
	{
            ppage->dirty = BT_LOCK;
            ppage->lockfor = 1502;
        }

	i = 0;

	while(i!=pkeys && MAJSTRCMPS(key,kParray[i])>=0)
	    ++i;

	if(i==pkeys)
	{
	    if(MAJSTRCMPS(key,kParray[i-1])<0)
		blockno = pParray[i-1];
	    else
		blockno = pParray[i];
	}
	else
	    blockno = pParray[i];

	if(blockno == spage->pagepos)
	{
	    *retpage = spage;
	    karray = kSarray;
	    parray = pSarray;
	    n = skeys;
	}
	else
	{
	    karray = kTarray;
	    parray = pTarray;
	    n = tkeys;
	}
	
	i = 0;

	while(i!=n && MAJSTRCMPS(key,karray[i])>=0)
	    ++i;

	if(i==n)
	{
	    if(MAJSTRCMPS(key,karray[i-1])<0)
		blockno = parray[i-1];
	    else
		blockno = parray[i];
	}
	else
	    blockno = parray[i];

	btreeDeallocPriArray(cache,arrays1);
	btreeDeallocPriArray(cache,arrays2);
	btreeDeallocPriArray(cache,arrays3);

	/* ajDebug("... returns blockno (b) %Lu\n",blockno); */
	
	return blockno;
    }


    btreeDeallocPriArray(cache,arrays1);
    btreeDeallocPriArray(cache,arrays2);
    btreeDeallocPriArray(cache,arrays3);


#if AJINDEX_DEBUG
    ajDebug("    InsertShift returning zero ...\n");
#endif

    /* ajDebug("... returns 0L\n"); */

    return 0L;
}




/* @funcstatic btreeHybReorderBuckets *****************************************
**
** Re-order leaf buckets
** Must only be called if one of the buckets is full
**
** @param [u] cache [AjPBtcache] cache
** @param [u] leaf [AjPBtpage] leaf page
**
** @return [AjBool] true if reorder was successful i.e. leaf not full
** @@
******************************************************************************/

static AjBool btreeHybReorderBuckets(AjPBtcache cache, AjPBtpage leaf)
{
    ajuint nkeys = 0;
    unsigned char *lbuf = NULL;

    ajulong *ptrs        = NULL;
    ajulong *overflows   = NULL;
    AjPStr *newkeys     = NULL;
    ajulong *newptrs     = NULL;
    AjPBtMem arrays     = NULL;
    AjPBtMem newarrays  = NULL;

    ajuint i = 0;
    
    ajuint order;
    ajuint totalkeys     = 0;
    ajuint maxnperbucket = 0;
    ajuint count         = 0;
    ajuint keylimit      = 0;
    ajuint bucketlimit   = 0;
    ajuint nodetype      = 0;
    
    AjPList idlist    = NULL;
    ajuint  dirtysave = 0;
    AjPBtId bid       = NULL;
    AjPBucket cbucket = NULL;
    AjPBtId cid       = NULL;

    ajuint iold = 0;

#if AJINDEX_DEBUG
    ajDebug("btreeHybReorderBuckets %Lu\n", leaf->pagepos);
#endif
    ++statCallReorderHybBuckets;

    /* ajDebug("In btreeHybReorderBuckets\n"); */

    dirtysave = leaf->dirty;

    leaf->dirty = BT_LOCK;
    leaf->lockfor = 1511;
    lbuf = leaf->buf;

    GBT_NODETYPE(lbuf,&nodetype);

    order = cache->order;
    

    /* Read keys/ptrs */
    arrays    = btreeAllocPriArray(cache);
    ptrs      = arrays->parray;
    newarrays    = btreeAllocPriArray(cache);
    newkeys      = newarrays->karray;
    newptrs      = newarrays->parray;
    overflows = newarrays->overflows;

    btreeGetPointers(cache,lbuf,&ptrs);

    GBT_NKEYS(lbuf,&nkeys);


    if(!nkeys)
	ajFatal("HybReorderBuckets: Attempt to reorder empty leaf");

    for(i=0;i<nkeys;++i)
	totalkeys += btreeNumInBucket(cache,ptrs[i]);

    totalkeys += btreeNumInBucket(cache,ptrs[i]);

    keylimit = nkeys + 1;

    btreeBucketCalc(totalkeys, keylimit, cache->nperbucket,
                    &bucketlimit, &maxnperbucket);


    if(bucketlimit >= order)
    {
	btreeDeallocPriArray(cache,arrays);
	btreeDeallocPriArray(cache,newarrays);

	leaf->dirty = dirtysave;
	return ajFalse;
    }

    /* Read buckets */

    idlist  = ajListNew();

    for(i=0;i<keylimit;++i)
	overflows[i] = btreeHybBucketIdlist(cache,ptrs[i],idlist);

    ajListSort(idlist,btreeIdCompare);

    cbucket = btreeBucketNew(cache->nperbucket);

    iold = 0;
    for(i=0;i<bucketlimit;++i)
    {
	cbucket->Overflow = overflows[i];
	cbucket->Nentries = 0;

	count = 0;

	while(count!=maxnperbucket)
	{
	    ajListPop(idlist,(void **)&bid);
	    
	    cid = cbucket->Ids[count];
	    ajStrAssignS(&cid->id,bid->id);
	    cid->dbno = bid->dbno;
	    cid->dups = bid->dups;
	    cid->offset = bid->offset;
	    cid->refoffset = bid->refoffset;
	    
	    cbucket->keylen[count] = BT_BUCKIDLEN(bid->id);
	    ++cbucket->Nentries;
	    ++count;
	    ajBtreeIdDel(&bid);
	}


	ajListPeek(idlist,(void **)&bid);
	ajStrAssignS(&newkeys[i],bid->id);

	if((iold < order) && ptrs[iold])
            newptrs[i] = ptrs[iold++];
        else
	    newptrs[i] = cache->totsize;
	btreeWriteBucket(cache,cbucket,newptrs[i]);
    }


    /* Deal with greater-than bucket */

    cbucket->Overflow = overflows[i];
    cbucket->Nentries = 0;

    count = 0;

    while(ajListPop(idlist,(void **)&bid))
    {
	cid = cbucket->Ids[count];
	ajStrAssignS(&cid->id,bid->id);
	cid->dbno = bid->dbno;
	cid->dups = bid->dups;
	cid->offset = bid->offset;
	cid->refoffset = bid->refoffset;
	
	++cbucket->Nentries;
	++count;
	ajBtreeIdDel(&bid);
    }
    
    
    if((iold < order) && ptrs[iold])
        newptrs[i] = ptrs[iold++];
    else
        newptrs[i] = cache->totsize;
    btreeWriteBucket(cache,cbucket,newptrs[i]);

    btreeBucketDel(&cbucket);

    /* Now write out a modified leaf with new keys/ptrs */

    nkeys = bucketlimit;

    btreeWriteNode(cache,leaf,newkeys,newptrs,nkeys);

    leaf->dirty = BT_DIRTY;

    if(nodetype == BT_ROOT)
    {
	leaf->dirty = BT_LOCK;
        leaf->lockfor = 1512;
    }

    btreeDeallocPriArray(cache,arrays);
    btreeDeallocPriArray(cache,newarrays);
    
    ajListFree(&idlist);

    return ajTrue;
}




/* @funcstatic btreeHybSplitLeaf *********************************************
**
** Split a leaf and propagate up if necessary
**
** @param [u] cache [AjPBtcache] cache
** @param [u] spage [AjPBtpage] page
**
** @return [AjPBtpage] pointer to a parent page
** @@
******************************************************************************/

static AjPBtpage btreeHybSplitLeaf(AjPBtcache cache, AjPBtpage spage)
{
    ajuint nkeys     = 0;
    ajuint order     = 0;
    ajuint totalkeys = 0;
    ajuint keylimit  = 0;
    ajuint nodetype  = 0;

    ajuint rootnodetype  = 0;
    
    ajuint i;
    ajuint j;
    
    AjPBtpage lpage = NULL;
    AjPBtpage rpage = NULL;
    AjPBtpage page  = NULL;
    
    AjPStr mediankey  = NULL;
    ajulong mediangtr  = 0L;
    ajulong medianless = 0L;
    

    AjPBtId bid = NULL;
    AjPBtId cid = NULL;

    unsigned char *buf  = NULL;
    unsigned char *lbuf = NULL;
    unsigned char *rbuf = NULL;

    AjPList idlist = NULL;

    AjPBucket cbucket  = NULL;
    
    ajulong *parray = NULL;
    AjPStr *newkarray  = NULL;
    ajulong *newparray  = NULL;
    AjPBtMem arrays     = NULL;
    AjPBtMem newarrays = NULL;
    
    ajuint lno    = 0;
    ajuint rno    = 0;

    ajuint lbucketlimit   = 0;
    ajuint lmaxnperbucket = 0;
    ajuint rbucketlimit   = 0;
    ajuint rmaxnperbucket = 0;
    ajuint count         = 0;
    
    ajulong lblockno = 0L;
    ajulong rblockno = 0L;
    ajulong prev     = 0L;
    ajulong overflow = 0L;
    ajulong prevsave = 0L;

    ajulong zero = 0L;
    ajulong join = 0L;
    
    ajulong lv = 0L;
    ajuint  v  = 0;
    
    ajuint iold=0;

#if AJINDEX_DEBUG
    ajDebug("btreeHybSplitLeaf %Lu\n", spage->pagepos);
#endif
    ++statCallSplitHybLeaf;

    /*ajDebug("In btreeHybSplitLeaf page:%Lu\n", spage->pagepos);*/

    order = cache->order;

    arrays    = btreeAllocPriArray(cache);
    parray    = arrays->parray;

    newarrays    = btreeAllocPriArray(cache);
    newkarray    = newarrays->karray;
    newparray    = newarrays->parray;

    buf = spage->buf;
    lbuf = buf;

    GBT_NKEYS(buf,&nkeys);
    GBT_NODETYPE(buf,&rootnodetype);

    /*ajDebug("btreeHybSplitLeaf start page %Lu nkeys:%u\n",
      spage->pagepos,  nkeys);*/

    for(i=nkeys+1; i<order; ++i)
	parray[i] = 0L;

    if(rootnodetype == BT_ROOT)
    {
#if AJINDEX_DEBUG
        ajDebug("    splitting root node %Lu new %Lu\n",
                spage->pagepos, cache->totsize);
#endif
	lblockno = cache->totsize;
	lpage = ajBtreeCacheWriteNodenew(cache);
	lbuf = lpage->buf;
	lv = prev;
	SBT_PREV(lbuf,lv);
    }
    else
    {
	lblockno = spage->pagepos;
	lpage = spage;
#if AJINDEX_DEBUG
        ajDebug("    keeping lpage %Lu\n", lpage->pagepos);
#endif
    }

    lpage->dirty = BT_LOCK;
    lpage->lockfor = 1521;


#if AJINDEX_DEBUG
        ajDebug("    new right node %Lu\n",
                cache->totsize);
#endif

    rblockno = cache->totsize;
    rpage = ajBtreeCacheWriteNodenew(cache);
    rpage->dirty = BT_LOCK;
    rpage->lockfor = 1522;
    rbuf = rpage->buf;

    if(rootnodetype == BT_ROOT)
    {
	lv = zero;
	SBT_RIGHT(rbuf,lv);
	lv = zero;
	SBT_LEFT(lbuf,lv);
    }
    else
    {
	GBT_RIGHT(lbuf,&join);
	lv = join;
	SBT_RIGHT(rbuf,lv);
    }

    lv = lblockno;
    SBT_LEFT(rbuf,lv);
    lv = rblockno;
    SBT_RIGHT(lbuf,lv);


    btreeGetPointers(cache,buf,&parray);

    keylimit = nkeys+1;

    idlist = ajListNew();

    for(i=0;i<keylimit;++i)
        btreeBucketIdlist(cache,parray[i],idlist);
    
    ajListSort(idlist,btreeIdCompare);

    totalkeys = ajListGetLength(idlist);

    btreeBucketSplitCalc(totalkeys, keylimit,
                         cache->nperbucket,
                         &lbucketlimit,&lmaxnperbucket,&lno,
                         &rbucketlimit,&rmaxnperbucket,&rno);

    cbucket = btreeBucketNew(cache->nperbucket);

    count = 0;
    iold=0;

    for(i=0;i<lbucketlimit;++i)
    {
	cbucket->Nentries = 0;

	for(j=0;j<lmaxnperbucket;++j)
	{
	    ajListPop(idlist,(void **)&bid);
	    
	    cid = cbucket->Ids[j];
	    ajStrAssignS(&cid->id,bid->id);
	    cid->dbno = bid->dbno;
	    cid->dups = bid->dups;
	    cid->offset = bid->offset;
	    cid->refoffset = bid->refoffset;
	    
	    cbucket->keylen[j] = BT_BUCKIDLEN(bid->id);
	    ++count;
	    ++cbucket->Nentries;
	    ajBtreeIdDel(&bid);
	}

	ajListPeek(idlist,(void **)&bid);
	
	ajStrAssignS(&newkarray[i],bid->id);

	if(iold < keylimit)
            newparray[i] = parray[iold++];
        else
	    newparray[i] = cache->totsize;
	btreeWriteBucket(cache,cbucket,newparray[i]);
    }

    cbucket->Nentries = 0;

    j = 0;

    while(count != lno)
    {
	ajListPop(idlist,(void **)&bid);
	cid = cbucket->Ids[j];
	++j;
	++count;

	ajStrAssignS(&cid->id,bid->id);
	cid->dbno = bid->dbno;
	cid->dups = bid->dups;
	cid->offset = bid->offset;
	cid->refoffset = bid->refoffset;
	
	++cbucket->Nentries;
	ajBtreeIdDel(&bid);
    }

    if(iold < keylimit)
        newparray[i] = parray[iold++];
    else
        newparray[i] = cache->totsize;
    btreeWriteBucket(cache,cbucket,newparray[i]);

    nkeys = lbucketlimit;

    nodetype = BT_LEAF;
    v = nodetype;
    SBT_NODETYPE(lbuf,v);
    lpage->dirty = BT_DIRTY;

    GBT_PREV(lbuf,&prevsave);
#if AJINDEX_DEBUG
    ajDebug("    lpage %Lu prevsave %Lu\n",
            lpage->pagepos, prevsave);
#endif
    btreeWriteNode(cache,lpage,newkarray,newparray,nkeys);

    ajListPeek(idlist,(void **)&bid);
    mediankey = ajStrNewS(bid->id);

    for(i=0;i<rbucketlimit;++i)
    {
	cbucket->Nentries = 0;
	for(j=0;j<rmaxnperbucket;++j)
	{
	    ajListPop(idlist,(void **)&bid);
	    
	    cid = cbucket->Ids[j];
	    ajStrAssignS(&cid->id,bid->id);
	    cid->dbno = bid->dbno;
	    cid->dups = bid->dups;
	    cid->offset = bid->offset;
	    cid->refoffset = bid->refoffset;
	    
	    cbucket->keylen[j] = BT_BUCKIDLEN(bid->id);
	    ++cbucket->Nentries;
	    ajBtreeIdDel(&bid);
	}

	ajListPeek(idlist,(void **)&bid);
	ajStrAssignS(&newkarray[i],bid->id);

	if(iold < keylimit)
            newparray[i] = parray[iold++];
        else
            newparray[i] = cache->totsize;
	btreeWriteBucket(cache,cbucket,newparray[i]);
    }

    cbucket->Nentries = 0;

    j = 0;

    while(ajListPop(idlist,(void**)&bid))
    {
	cid = cbucket->Ids[j];
	++j;

	ajStrAssignS(&cid->id,bid->id);
	cid->dbno = bid->dbno;
	cid->dups = bid->dups;
	cid->offset = bid->offset;
	cid->refoffset = bid->refoffset;
	
	++cbucket->Nentries;
	ajBtreeIdDel(&bid);
    }
    
    if(iold < keylimit)
        newparray[i] = parray[iold++];
    else
        newparray[i] = cache->totsize;
    btreeWriteBucket(cache,cbucket,newparray[i]);

    nkeys = rbucketlimit;

    nodetype = BT_LEAF;
    v = nodetype;
    SBT_NODETYPE(rbuf,v);
    lv = prevsave;
    SBT_PREV(rbuf,lv);
    lv = overflow;
    SBT_OVERFLOW(rbuf,lv);

#if AJINDEX_DEBUG
    ajDebug("    right %Lu prev: %Lu\n",
            rpage->pagepos, prevsave);
#endif
    btreeWriteNode(cache,rpage,newkarray,newparray,nkeys);
    rpage->dirty = BT_DIRTY;

    btreeBucketDel(&cbucket);
    ajListFree(&idlist);



    medianless = lblockno;
    mediangtr  = rblockno;

    btreeDeallocPriArray(cache,arrays);
    btreeDeallocPriArray(cache,newarrays);

    if(rootnodetype == BT_ROOT)
    {
#if AJINDEX_DEBUG
        ajDebug("    rootnode new node for mediankey '%S' %Lu\n",
                mediankey, spage->pagepos);
#endif
	btreeWriteNodeSingle(cache,spage,mediankey,lblockno,rblockno);	
	spage->dirty = BT_LOCK;
        spage->lockfor = 1523;

	ajStrDel(&mediankey);
	++cache->level;

	return spage;
    }


    page = ajBtreeCacheRead(cache,prevsave);
    btreeHybInsertKey(cache,page,mediankey,medianless,mediangtr);
    ajStrDel(&mediankey);

    page = ajBtreeCacheRead(cache,prevsave);

    return page;
}




/* @funcstatic btreeHybInsertKey *****************************************
**
** Insert a key into a potentially full node
**
** @param [u] cache [AjPBtcache] cache
** @param [u] page [AjPBtpage] original page
** @param [r] key [const AjPStr] key to insert
** @param [r] less [ajulong] less-than pointer
** @param [r] greater [ajulong] greater-than pointer
**
** @return [void]
** @@
******************************************************************************/

static void btreeHybInsertKey(AjPBtcache cache, AjPBtpage page,
			      const AjPStr key, ajulong less, ajulong greater)
{
    unsigned char *lbuf = NULL;
    unsigned char *rbuf = NULL;
    unsigned char *tbuf = NULL;
    AjPStr *karray      = NULL;
    ajulong *parray      = NULL;
    AjPStr *tkarray     = NULL;
    ajulong *tparray     = NULL;

    AjPBtMem arrays1    = NULL;
    AjPBtMem arrays2    = NULL;

    ajuint nkeys    = 0;
    ajuint order    = 0;
    ajuint keypos   = 0;
    ajuint rkeyno   = 0;
    
    ajuint i = 0;
    ajuint n = 0;
    
    ajuint nodetype  = 0;
    AjPBtpage ipage = NULL;
    AjPBtpage lpage = NULL;
    AjPBtpage rpage = NULL;
    AjPBtpage tpage = NULL;

    ajulong blockno  = 0L;
    ajulong rblockno = 0L;
    ajulong lblockno = 0L;
    ajulong ibn      = 0L;

    AjPStr mediankey  = NULL;
    ajulong medianless = 0L;
    ajulong mediangtr  = 0L;
    ajulong overflow   = 0L;
    ajulong prev       = 0L;
    ajuint totlen     = 0;
    
    ajulong lv = 0L;
    ajuint  v  = 0;

    if(!btreeNodeIsFull(cache,page))
    {
	btreeInsertNonFull(cache,page,key,less,greater);

	return;
    }
    
    order = cache->order;
    lbuf = page->buf;
    GBT_NODETYPE(lbuf,&nodetype);
    page->dirty = BT_LOCK;
    page->lockfor = 1531;

    if(nodetype == BT_ROOT)
    {
	arrays1   = btreeAllocPriArray(cache);
	karray    = arrays1->karray;
	parray    = arrays1->parray;

	btreeHybSplitRoot(cache);

	if(page->pagepos)
	    page->dirty = BT_DIRTY;

	btreeGetKeys(cache,lbuf,&karray,&parray);

	if(MAJSTRCMPS(key,karray[0])<0)
	    blockno = parray[0];
	else
	    blockno = parray[1];

	ipage = ajBtreeCacheRead(cache,blockno);
	btreeInsertNonFull(cache,ipage,key,less,greater);

	btreeDeallocPriArray(cache,arrays1);

	return;
    }


    arrays1   = btreeAllocPriArray(cache);
    karray    = arrays1->karray;
    parray    = arrays1->parray;

    arrays2   = btreeAllocPriArray(cache);
    tkarray   = arrays2->karray;
    tparray   = arrays2->parray;

    mediankey = ajStrNew();
    
    lpage = page;
    lbuf = lpage->buf;
    
    btreeGetKeys(cache,lbuf,&karray,&parray);

    GBT_BLOCKNUMBER(lbuf,&lblockno);

    rblockno = cache->totsize;
    rpage = ajBtreeCacheWriteNodenew(cache);
    rpage->dirty = BT_LOCK;
    rpage->lockfor = 1532;
    rbuf = rpage->buf;


    GBT_PREV(lbuf,&prev);
    lv = prev;
    SBT_PREV(rbuf,lv);

    nkeys = order - 1;
    keypos = nkeys / 2;

    if(!(nkeys % 2))
	--keypos;

    ajStrAssignS(&mediankey,karray[keypos]);
    medianless = lblockno;
    mediangtr  = rblockno;


    GBT_NODETYPE(lbuf,&nodetype);
    v = nodetype;
    SBT_NODETYPE(rbuf,v);
    lv = overflow;
    SBT_OVERFLOW(rbuf,lv);


    totlen = 0;

    for(i=0;i<keypos;++i)
    {
	ajStrAssignS(&tkarray[i],karray[i]);
	totlen += ajStrGetLen(karray[i]);
	tparray[i] = parray[i];
    }

    tparray[i] = parray[i];
    v = totlen;
    SBT_TOTLEN(lbuf,v);
    n = i;
    v = n;
    SBT_NKEYS(lbuf,v);
    btreeWriteNode(cache,lpage,tkarray,tparray,i);
    lpage->dirty = BT_LOCK;
    lpage->lockfor = 1533;


    for(i=0;i<n+1;++i)
    {
	tpage = ajBtreeCacheRead(cache,tparray[i]);
	tbuf = tpage->buf;
	lv = lblockno;
	SBT_PREV(tbuf,lv);
	tpage->dirty = BT_DIRTY;
    }


    totlen = 0;

    for(i=keypos+1;i<nkeys;++i)
    {
	ajStrAssignS(&tkarray[i-(keypos+1)],karray[i]);
	totlen += ajStrGetLen(karray[i]);
	tparray[i-(keypos+1)] = parray[i];
    }
    tparray[i-(keypos+1)] = parray[i];
    v = totlen;
    SBT_TOTLEN(rbuf,v);
    rkeyno = (nkeys-keypos) - 1;
    v = rkeyno;
    SBT_NKEYS(rbuf,v);
    rpage->dirty = BT_DIRTY;

    btreeWriteNode(cache,rpage,tkarray,tparray,rkeyno);
    rpage->dirty = BT_LOCK;
    rpage->lockfor = 1534;

    for(i=0;i<rkeyno+1;++i)
    {
	tpage = ajBtreeCacheRead(cache,tparray[i]);
	tbuf = tpage->buf;
	lv = rblockno;
	SBT_PREV(tbuf,lv);
	tpage->dirty = BT_DIRTY;
    }


    ibn = rblockno;

    if(MAJSTRCMPS(key,mediankey)<0)
	ibn = lblockno;

    lpage->dirty = BT_DIRTY;
    rpage->dirty = BT_DIRTY;

    ipage = ajBtreeCacheRead(cache,ibn);
    
    btreeInsertNonFull(cache,ipage,key,less,greater);


    btreeDeallocPriArray(cache,arrays1);
    btreeDeallocPriArray(cache,arrays2);

    ipage = ajBtreeCacheRead(cache,prev);

    btreeHybInsertKey(cache,ipage,mediankey,medianless,mediangtr);
    ajStrDel(&mediankey);

    return;
}




/* @funcstatic btreeHybSplitRoot *****************************************
**
** Split the root node
**
** @param [u] cache [AjPBtcache] cache
**
** @return [void]
** @@
******************************************************************************/

static void btreeHybSplitRoot(AjPBtcache cache)
{
    AjPBtpage rootpage = NULL;
    AjPBtpage rpage    = NULL;
    AjPBtpage lpage    = NULL;
    AjPBtpage tpage    = NULL;

    AjPStr *karray     = NULL;
    AjPStr *tkarray    = NULL;
    ajulong *parray     = NULL;
    ajulong *tparray    = NULL;
    AjPBtMem arrays1   = NULL;
    AjPBtMem arrays2   = NULL;

    ajuint order     = 0;
    ajuint nkeys     = 0;
    ajuint keypos    = 0;
    
    ajulong rblockno = 0L;
    ajulong lblockno = 0L;
    
    AjPStr key = NULL;
    ajuint  i;
    ajuint  j;

    unsigned char *rootbuf = NULL;
    unsigned char *rbuf    = NULL;
    unsigned char *lbuf    = NULL;
    unsigned char *tbuf    = NULL;
    
    ajuint nodetype  = 0;
    ajulong overflow = 0L;
    ajulong zero     = 0L;
    ajuint rkeyno    = 0;

    ajulong lv = 0L;
    ajuint  v  = 0;
    
    
#if AJINDEX_DEBUG
    ajDebug("btreeHybSplitRoot zero\n");
#endif
    ++statCallSplitHybRoot;

    /* ajDebug("In btreeHybSplitRoot\n"); */

    order = cache->order;

    arrays1   = btreeAllocPriArray(cache);
    karray    = arrays1->karray;
    parray    = arrays1->parray;

    arrays2   = btreeAllocPriArray(cache);
    tkarray   = arrays2->karray;
    tparray   = arrays2->parray;

    key = ajStrNew();

    rootpage = btreeCacheLocate(cache,0L);

    if(!rootpage)
	ajFatal("Root page has been unlocked 1");
    
    rootbuf = rootpage->buf;
    nkeys   = order - 1;
    keypos  = nkeys / 2;

    if(!(nkeys % 2))
	--keypos;


    rblockno = cache->totsize;
    rpage = ajBtreeCacheWriteNodenew(cache);
    rpage->dirty = BT_LOCK;
    rpage->lockfor = 1541;

    lblockno = cache->totsize;
    lpage = ajBtreeCacheWriteNodenew(cache);

    if(!cache->level)
    {
	lv = zero;
	SBT_LEFT(lpage->buf,lv);
	lv = rblockno;
	SBT_RIGHT(lpage->buf,lv);
	lv = lblockno;
	SBT_LEFT(rpage->buf,lv);
	lv = zero;
	SBT_RIGHT(rpage->buf,lv);
    }

    btreeGetKeys(cache,rootbuf,&karray,&parray);

    /* write new root node */

    btreeWriteNodeSingle(cache,rootpage,karray[keypos],lblockno,rblockno);
    rootpage->dirty = BT_LOCK;
    rootpage->lockfor = 1542;

    rbuf = rpage->buf;
    lbuf = lpage->buf;
    
    if(cache->level)
	nodetype = BT_INTERNAL;
    else
	nodetype = BT_LEAF;

    v = nodetype;
    SBT_NODETYPE(rbuf,v);
    v = nodetype;
    SBT_NODETYPE(lbuf,v);
    lv = overflow;
    SBT_OVERFLOW(rbuf,lv);
    lv = overflow;
    SBT_PREV(rbuf,lv);
    lv = overflow;
    SBT_OVERFLOW(lbuf,lv);
    lv = overflow;
    SBT_PREV(lbuf,lv);

    for(i=0;i<keypos;++i)
    {
	ajStrAssignS(&tkarray[i],karray[i]);
	tparray[i] = parray[i];
    }

    tparray[i] = parray[i];
    btreeWriteNode(cache,lpage,tkarray,tparray,keypos);

    for(i=0;i<=keypos;++i)
    {
	tpage = ajBtreeCacheRead(cache,tparray[i]);
	tbuf = tpage->buf;
	lv = lblockno;
	SBT_PREV(tbuf,lv);
	tpage->dirty = BT_DIRTY;
#if AJINDEX_DEBUG
        ajDebug("    new lpage %Lu prev %Lu\n",
                tpage->pagepos, lblockno);
#endif
    }

    j=0;

    for(i=keypos+1;i<nkeys;++i)
    {
	ajStrAssignS(&tkarray[j],karray[i]);
	tparray[j++] = parray[i];
    }

    tparray[j] = parray[i];
    rkeyno = (nkeys-keypos) - 1;
    rpage->dirty = BT_DIRTY;

    btreeWriteNode(cache,rpage,tkarray,tparray,rkeyno);

    for(i=0;i<=rkeyno;++i)
    {
	tpage = ajBtreeCacheRead(cache,tparray[i]);
	tbuf = tpage->buf;
	lv = rblockno;
	SBT_PREV(tbuf,lv);
	tpage->dirty = BT_DIRTY;
#if AJINDEX_DEBUG
        ajDebug("    new rpage %Lu prev %Lu\n",
                tpage->pagepos, rblockno);
#endif
    }


    btreeDeallocPriArray(cache,arrays1);
    btreeDeallocPriArray(cache,arrays2);

    ++cache->level;

    ajStrDel(&key);
    
    return;
}




/* @func ajBtreeHybInsertId ***************************************************
**
** Insert an ID structure into the tree
**
** @param [u] cache [AjPBtcache] cache
** @param [u] hyb [AjPBtHybrid] Id object
**
** @return [void] pointer to a page
** @@
******************************************************************************/

void ajBtreeHybInsertId(AjPBtcache cache, AjPBtHybrid hyb)
{
    AjPBtpage spage   = NULL;
    ajulong lblockno = 0L;
    ajulong rblockno = 0L;
    ajulong blockno  = 0L;
    ajulong shift    = 0L;

    ajuint nkeys = 0;

    ajuint nodetype = 0;
    
    AjPBtId btid = NULL;
    
    ajuint n;
    ajuint ientry;

    unsigned char *buf = NULL;

#if AJINDEX_DEBUG
    ajDebug("btreeHybInsertId '%S'\n", hyb->key1);
#endif

    /* ajDebug("In ajBtreeHybInsertId\n"); */

    if(!MAJSTRGETLEN(hyb->key1))
	return;

    ajStrFmtQuery(&hyb->key1);

    if(MAJSTRGETLEN(hyb->key1) > cache->kwlimit)
        ajStrTruncateLen(&hyb->key1, cache->kwlimit);

    spage = ajBtreeHybFindInsert(cache,hyb->key1);
    buf = spage->buf;

    GBT_NKEYS(buf,&nkeys);
    GBT_NODETYPE(buf,&nodetype);
    
    if(!nkeys)
    {
	lblockno = cache->totsize;
	btreeWriteBucketEmpty(cache,lblockno);

	rblockno = cache->totsize;
	btreeWriteBucketEmpty(cache,rblockno);	

	btreeWriteNodeSingle(cache,spage,hyb->key1,lblockno,rblockno);

	btreeAddToHybBucket(cache,rblockno,hyb);
        ++cache->countunique;
        ++cache->countall;

	return;
    }


    /* Search to see whether entry exists */
    
    blockno = btreeGetBlockS(cache,buf,hyb->key1);

    btid = btreeBucketFindDupId(cache,blockno,hyb->key1,&ientry);

    if(btid)
    {
        ++cache->countall;

	btreeHybDupInsert(cache,hyb,btid);
        btreeWriteBucketId(cache,blockno,btid,ientry);
        ajBtreeIdDel(&btid);

	return;
    }

    if(nodetype != BT_ROOT)
	if((shift = btreeHybInsertShift(cache,&spage,hyb->key1)))
	    blockno = shift;

    buf = spage->buf;

    n = btreeNumInBucket(cache,blockno);

    if(n == cache->nperbucket)
    {
	if(btreeHybReorderBuckets(cache,spage))
	{
            blockno = btreeGetBlockS(cache,buf,hyb->key1);
	}
	else
	{
	    btreeHybSplitLeaf(cache,spage);
	    spage  = ajBtreeHybFindInsert(cache,hyb->key1);
	    buf = spage->buf;

            blockno = btreeGetBlockS(cache,buf,hyb->key1);
	}
    }


    btreeAddToHybBucket(cache,blockno,hyb);

    ++cache->countunique;
    ++cache->countall;

    return;
}




/* @funcstatic btreeHybDupInsert *********************************************
**
** Insert a known duplicate ID structure into the tree
**
** @param [u] cache [AjPBtcache] cache
** @param [r] hyb [const AjPBtHybrid] Hybrid ID object
** @param [u] btid [AjPBtId] Id object
**
** @return [void]
** @@
******************************************************************************/

static void btreeHybDupInsert(AjPBtcache cache, const AjPBtHybrid hyb,
			      AjPBtId btid)
{
    AjPBtpage page;
    AjPBtpage rpage;
    ajulong secrootpage = 0L;
    unsigned char *buf;
    ajulong right = 0L;
    AjPBtNumId num = NULL;
    AjOBtNumId numobj = {0L, 0L, 0, ""};

    /* ajDebug("In btreeHybDupInsert\n"); */

    if(!btid->dups)
    {
	btid->dups = 1;
	num = &numobj;
	
	num->offset    = btid->offset;
	num->refoffset = btid->refoffset;
	num->dbno      = btid->dbno;

	secrootpage = cache->totsize;

	btid->offset = secrootpage;

	ajBtreeCreateRootNode(cache,secrootpage);
	cache->secrootblock = secrootpage;
	page = ajBtreeCacheWriteNode(cache,secrootpage);
	page->dirty = BT_DIRTY;

        if(btreeDoRootSync)
            ajBtreeCacheRootSync(cache,secrootpage);
	page->dirty = BT_LOCK;
	page->lockfor = 1551;

	rpage = btreeCacheLocate(cache, 0L);
	rpage->dirty = BT_LOCK;
        rpage->lockfor = 1552;

	cache->slevel = 0;

	ajBtreeInsertNum(cache,num,page);

	num->offset    = hyb->offset;
	num->refoffset = hyb->refoffset;
	num->dbno      = hyb->dbno;

	ajBtreeInsertNum(cache,num,page);
	++btid->dups;

	return;
    }
    else
    {
	cache->secrootblock = btid->offset;
	page = ajBtreeCacheWriteNode(cache,cache->secrootblock);
	page->dirty = BT_LOCK;
        page->lockfor = 1553;
	buf = page->buf;
	GBT_RIGHT(buf,&right);
	cache->slevel = (ajuint) right;

	num = &numobj;
	
	num->offset    = hyb->offset;
	num->refoffset = hyb->refoffset;
	num->dbno      = hyb->dbno;
	
	ajBtreeInsertNum(cache,num,page);

	++btid->dups;
    }
    

    page->dirty = BT_DIRTY;

    return;
}




/* @funcstatic btreeHybBucketIdlist ********************************************
**
** Copies all hybrid IDs into a list
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
** @param [u] idlist [AjPList] list to hold hybrid IDs
**
** @return [ajulong] Overflow
** @@
******************************************************************************/

static ajulong btreeHybBucketIdlist(AjPBtcache cache, ajulong pagepos,
                                    AjPList idlist)
{
    AjPBtpage page      = NULL;
    AjPBtpage lpage     = NULL;
    unsigned char *buf  = NULL;
    unsigned char *kptr = NULL;
    AjPBtId id          = NULL;
    
    unsigned char *idptr = NULL;
    
    ajuint  nodetype  = 0;
    ajuint  nentries  = 0;
    ajulong overflow  = 0L;
    ajulong pageoverflow = 0L;
    ajuint  dirtysave = 0;
    
    ajuint  i;
    ajuint  len = 0;
    ajuint idlen;

    /* ajDebug("In btreeHybBucketIdlist\n"); */
    
    if(!pagepos)
	ajFatal("HybBucketIdlist: cannot read bucket from root page cache %S",
                cache->filename);

    page  = ajBtreeCacheRead(cache,pagepos);
    lpage = page;
    dirtysave = lpage->dirty;
    lpage->dirty = BT_LOCK;
    lpage->lockfor = 1561;

    buf = lpage->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);
    if(nodetype != BT_BUCKET)
	ajFatal("HybBucketIdlist: NodeType mismatch. "
                "Not bucket (%u) cache %S",
                nodetype, cache->filename);
    
    GBT_BUCKNENTRIES(buf,&nentries);
    if(nentries > cache->nperbucket)
	ajFatal("HybBucketIdlist: Bucket too full page: %Lu "
                "entries: %u max: %u cache %S",
                pagepos, nentries, cache->nperbucket, cache->filename);
    

    GBT_BUCKOVERFLOW(buf,&overflow);
    pageoverflow = overflow;
    
    kptr  = PBT_BUCKKEYLEN(buf);
    idptr = kptr + (nentries * sizeof(ajuint));

    for(i=0;i<nentries;++i)
    {
	BT_GETAJUINT(kptr,&len);
        idlen = len - BT_DDOFFROFF-1;

	if((idptr-buf+1) + len > cache->pagesize)	/* overflow */
	{
	    /* ajDebug("HybBucketIdlist: Overflow\n"); */
	    page  = ajBtreeCacheRead(cache,overflow);
	    buf = page->buf;
	    GBT_BUCKNODETYPE(buf,&nodetype);
	    if(nodetype != BT_BUCKET)
		ajFatal("HybBucketIdlist: NodeType mismatch. "
                        "Not bucket (%u) cache %S",
			nodetype, cache->filename);
	    GBT_BUCKOVERFLOW(buf,&overflow);
	    /* overflow bucket ids start at the keylen position */
	    idptr = PBT_BUCKKEYLEN(buf);
	}

	id = ajBtreeIdNew();
        
	/* Fill ID objects */
	ajStrAssignLenC(&id->id,(const char *)idptr,idlen);
	idptr += (idlen + 1);
	BT_GETAJUINT(idptr,&id->dbno);
	idptr += sizeof(ajuint);
	BT_GETAJUINT(idptr,&id->dups);
	idptr += sizeof(ajuint);	
	BT_GETAJULONG(idptr,&id->offset);
	idptr += sizeof(ajulong);
	BT_GETAJULONG(idptr,&id->refoffset);
	idptr += sizeof(ajulong);

	kptr += sizeof(ajuint);
        ajListPushAppend(idlist, id);
    }

    lpage->dirty = dirtysave;
    
    return pageoverflow;
}




/* @funcstatic btreeHybBucketIdlistAll *****************************************
**
** Copies all hybrid IDs into a list, following duplicates
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
** @param [u] idlist [AjPList] list to hold hybrid IDs
**
** @return [ajulong] Overflow
** @@
******************************************************************************/

static ajulong btreeHybBucketIdlistAll(AjPBtcache cache, ajulong pagepos,
                                       AjPList idlist)
{
    AjPBtpage page      = NULL;
    AjPBtpage lpage     = NULL;
    unsigned char *buf  = NULL;
    unsigned char *kptr = NULL;
    AjPBtId id          = NULL;
    
    unsigned char *idptr = NULL;
    
    ajuint  nodetype  = 0;
    ajuint  nentries  = 0;
    ajulong overflow  = 0L;
    ajulong pageoverflow = 0L;
    ajuint  dirtysave = 0;
    
    ajuint  i;
    ajuint  len = 0;
    ajuint idlen;

    /* ajDebug("In btreeHybBucketIdlist\n"); */
    
    if(!pagepos)
	ajFatal("HybBucketIdlist: cannot read bucket from root page cache %S",
                cache->filename);

    page  = ajBtreeCacheRead(cache,pagepos);
    lpage = page;
    dirtysave = lpage->dirty;
    lpage->dirty = BT_LOCK;
    lpage->lockfor = 1561;

    buf = lpage->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);
    if(nodetype != BT_BUCKET)
	ajFatal("HybBucketIdlist: NodeType mismatch. "
                "Not bucket (%u) cache %S",
                nodetype, cache->filename);
    
    GBT_BUCKNENTRIES(buf,&nentries);
    if(nentries > cache->nperbucket)
	ajFatal("HybBucketIdlist: Bucket too full page: %Lu "
                "entries: %u max: %u cache %S",
                pagepos, nentries, cache->nperbucket, cache->filename);
    

    GBT_BUCKOVERFLOW(buf,&overflow);
    pageoverflow = overflow;
    
    kptr  = PBT_BUCKKEYLEN(buf);
    idptr = kptr + (nentries * sizeof(ajuint));

    for(i=0;i<nentries;++i)
    {
	BT_GETAJUINT(kptr,&len);
        idlen = len - BT_DDOFFROFF-1;

	if((idptr-buf+1) + len > cache->pagesize)	/* overflow */
	{
	    /* ajDebug("HybBucketIdlist: Overflow\n"); */
	    page  = ajBtreeCacheRead(cache,overflow);
	    buf = page->buf;
	    GBT_BUCKNODETYPE(buf,&nodetype);
	    if(nodetype != BT_BUCKET)
		ajFatal("HybBucketIdlist: NodeType mismatch. "
                        "Not bucket (%u) cache %S",
			nodetype, cache->filename);
	    GBT_BUCKOVERFLOW(buf,&overflow);
	    /* overflow bucket ids start at the keylen position */
	    idptr = PBT_BUCKKEYLEN(buf);
	}

	id = ajBtreeIdNew();
        
	/* Fill ID objects */
	ajStrAssignLenC(&id->id,(const char *)idptr,idlen);
	idptr += (idlen + 1);
	BT_GETAJUINT(idptr,&id->dbno);
	idptr += sizeof(ajuint);
	BT_GETAJUINT(idptr,&id->dups);
	idptr += sizeof(ajuint);	
	BT_GETAJULONG(idptr,&id->offset);
	idptr += sizeof(ajulong);
	BT_GETAJULONG(idptr,&id->refoffset);
	idptr += sizeof(ajulong);

	kptr += sizeof(ajuint);
        if(!id->dups)
            ajListPushAppend(idlist, id);
        else
        {
            ajBtreeHybLeafList(cache,id->offset,id->id,idlist);
            ajBtreeIdDel(&id);
        }
    }

    lpage->dirty = dirtysave;
    
    return pageoverflow;
}




/* @funcstatic btreeGetNumKeys *********************************************
**
** Get Keys and Pointers from an internal node
**
** @param [u] cache [AjPBtcache] cache
** @param [u] buf [unsigned char *] page buffer
** @param [w] keys [ajulong **] keys
** @param [w] ptrs [ajulong**] ptrs
**
** @return [void]
** @@
******************************************************************************/

static void btreeGetNumKeys(AjPBtcache cache, unsigned char *buf,
			    ajulong **keys, ajulong **ptrs)
{
    ajulong *karray = NULL;
    ajulong *parray = NULL;
    
    ajuint nkeys = 0;
    unsigned char *pptr = NULL;
    ajuint    i;

    /* ajDebug("In btreeGetNumKeys\n"); */

    (void) cache;			/* make it used */

    karray = *keys;
    parray = *ptrs;

    pptr = PBT_KEYLEN(buf);
    GBT_NKEYS(buf,&nkeys);

    if(!nkeys)
	ajFatal("GetNumKeys: No keys in node");

    for(i=0;i<nkeys;++i)
    {
	BT_GETAJULONG(pptr,&karray[i]);
	pptr += sizeof(ajulong);
    }

    for(i=0;i<nkeys;++i)
    {
	BT_GETAJULONG(pptr,&parray[i]);
	pptr += sizeof(ajulong);
    }
    
    BT_GETAJULONG(pptr,&parray[i]);

    return;
}




/* @funcstatic btreeGetNumPointers *********************************************
**
** Get Pointers from an internal numeric node
**
** @param [u] cache [AjPBtcache] cache
** @param [u] buf [unsigned char *] page buffer
** @param [w] ptrs [ajulong**] ptrs
**
** @return [void]
** @@
******************************************************************************/

static void btreeGetNumPointers(AjPBtcache cache, unsigned char *buf,
                                ajulong **ptrs)
{
    ajulong *parray = NULL;
    
    ajuint nkeys = 0;
    unsigned char *pptr = NULL;
    ajuint    i;

    (void) cache;			/* make it used */

    parray = *ptrs;

    pptr = PBT_KEYLEN(buf);
    GBT_NKEYS(buf,&nkeys);
    if(!nkeys)
	ajFatal("GetNumKeys: No keys in node cache %S", cache->filename);

    pptr += nkeys*sizeof(ajulong);
    for(i=0;i<nkeys;++i)
    {
	BT_GETAJULONG(pptr,&parray[i]);
	pptr += sizeof(ajulong);
    }
    
    BT_GETAJULONG(pptr,&parray[i]);

    return;
}




/* @funcstatic btreeWriteNumNode *******************************************
**
** Write an internal node
**
** @param [u] cache [AjPBtcache] cache
** @param [u] spage [AjPBtpage] buffer
** @param [r] keys [const ajulong *] keys
** @param [r] ptrs [const ajulong*] page pointers
** @param [r] nkeys [ajuint] number of keys

**
** @return [void]
** @@
******************************************************************************/

static void btreeWriteNumNode(AjPBtcache cache, AjPBtpage spage,
			      const ajulong *keys, const ajulong *ptrs,
			      ajuint nkeys)
{
    unsigned char *pptr   = NULL;
    unsigned char *buf;
    
    ajuint tnkeys = 0;
    ajuint totlen = 0;

    ajulong aspace   = 0L;
    ajulong lv       = 0L;
    ajulong overflow = 0L;
    ajuint i;

    /* ajuint unused = 0; */

#if AJINDEX_DEBUG
    ajDebug("btreeWriteNumNode %Lu nkeys:%u\n", spage->pagepos, nkeys);
#endif

    buf = spage->buf;

    tnkeys = nkeys;
    SBT_NKEYS(buf,tnkeys);
    totlen = 0;
    SBT_TOTLEN(buf, totlen);

    pptr = PBT_KEYLEN(buf);
    aspace = 2 * nkeys * sizeof(ajulong) + sizeof(ajulong);

    if((ajuint) ((pptr+aspace)-buf) > cache->pagesize)
	ajFatal("WriteNumNode: too many keys for available pagesize");

    for(i=0;i<nkeys;++i)
    {
	lv = keys[i];
	BT_SETAJULONG(pptr,lv);
	pptr += sizeof(ajulong);
    }

    for(i=0;i<nkeys;++i)
    {
	lv = ptrs[i];
	BT_SETAJULONG(pptr,lv);
	pptr += sizeof(ajulong);
    }
    
    lv = ptrs[i];
    BT_SETAJULONG(pptr,lv);
    pptr += sizeof(ajulong);

    /*
    unused = cache->pagesize - (pptr - buf);
    if(unused > cache->pagesize)
        ajFatal("unused %u for pagesize %u", unused, cache->pagesize);
    if(unused > 0)
        memset(pptr, 0, unused);
    */

    overflow = 0L;
    SBT_OVERFLOW(buf,overflow);

    spage->dirty = BT_DIRTY;

    return;
}




/* @funcstatic btreeWriteNumNodeSingle ***************************************
**
** Write an internal numeric node with a single key
**
** @param [u] cache [AjPBtcache] cache
** @param [u] spage [AjPBtpage] buffer
** @param [r] numkey [ajulong] key
** @param [r] lptr [ajulong] left page pointer
** @param [r] rptr [ajulong] rightpage pointer
**
** @return [void]
** @@
******************************************************************************/

static void btreeWriteNumNodeSingle(AjPBtcache cache, AjPBtpage spage,
                                    ajulong numkey, ajulong lptr,
                                    ajulong rptr)
{
    unsigned char *pptr   = NULL;
    unsigned char *buf;
    
    ajuint tnkeys = 0;
    ajuint totlen = 0;

    ajulong aspace   = 0L;
    ajulong lv       = 0L;
    ajulong overflow = 0L;

    /* ajuint unused = 0; */

    /* ajDebug("In btreeWriteNumNode\n"); */

    buf = spage->buf;

    tnkeys = 1;
    SBT_NKEYS(buf,tnkeys);
    totlen = 0;
    SBT_TOTLEN(buf,totlen);

    pptr = PBT_KEYLEN(buf);
    aspace = 2 * sizeof(ajulong) + sizeof(ajulong);
    if((ajuint) ((pptr+aspace)-buf) > cache->pagesize)
	ajFatal("WriteNumNodeSingle: "
                "too many keys for available pagesize cache %S",
                cache->filename);

    lv = numkey;
    BT_SETAJULONG(pptr,lv);
    pptr += sizeof(ajulong);

    lv = lptr;
    BT_SETAJULONG(pptr,lv);
    pptr += sizeof(ajulong);
    
    lv = rptr;
    BT_SETAJULONG(pptr,lv);
    pptr += sizeof(ajulong);

    overflow = 0L;
    SBT_OVERFLOW(buf,overflow);

    /*
    unused = cache->pagesize - (pptr - buf);
    if(unused > cache->pagesize)
        ajFatal("unused %u for pagesize %u", unused, cache->pagesize);
    if(unused > 0)
        memset(pptr, 0, unused);
    */

    spage->dirty = BT_DIRTY;    /* test in caller */

    return;
}




/* @funcstatic btreeWriteNumBucket *******************************************
**
** Write index bucket object to the cache given a disc page number
**
** @param [u] cache [AjPBtcache] cache
** @param [r] bucket [const AjPNumBucket] bucket
** @param [r] pagepos [ajulong] page number
**
** @return [void]
** @@
******************************************************************************/

static void btreeWriteNumBucket(AjPBtcache cache, const AjPNumBucket bucket,
                                ajulong pagepos)
{
    AjPBtpage page      = NULL;
    unsigned char *buf  = NULL;
    unsigned char *pptr = NULL;

    ajuint  uv  = 0;
    ajuint   v   = 0;
    ajuint  i   = 0;
    ajulong lv   = 0L;
    ajuint nentries = 0;
    ajulong overflow = 0L;

    /* ajuint unused = 0; */

    /* ajDebug("In btreeWriteNumBucket\n"); */

    if(pagepos == cache->totsize)
    {
	page = ajBtreeCacheWriteBucketnew(cache);
	buf = page->buf;
	overflow = 0L;
    }
    else
    {
	page = ajBtreeCacheRead(cache,pagepos);
	buf = page->buf;
	GBT_BUCKOVERFLOW(buf,&overflow);
    }

    v = BT_NUMBUCKET;
    SBT_BUCKNODETYPE(buf,v);
    page->dirty = BT_LOCK;
    page->lockfor = 1571;

    nentries = bucket->Nentries;
    v = nentries;
    SBT_BUCKNENTRIES(buf,v);
    
    pptr = PBT_BUCKKEYLEN(buf);

    for(i=0;i<nentries;++i)
    {
	lv = bucket->NumId[i]->offset;
	BT_SETAJULONG(pptr,lv);
	pptr += sizeof(ajulong);
	lv = bucket->NumId[i]->refoffset;
	BT_SETAJULONG(pptr,lv);
	pptr += sizeof(ajulong);
	uv = bucket->NumId[i]->dbno;
	BT_SETAJUINT(pptr,uv);
	pptr += sizeof(ajuint);
    }
    
    lv = 0L;
    SBT_BUCKOVERFLOW(buf,lv);

    /*
    unused = cache->pagesize - (pptr - buf);
    if(unused > cache->pagesize)
        ajFatal("unused %u for pagesize %u", unused, cache->pagesize);
    if(unused > 0)
        memset(pptr, 0, unused);
    */

    page->dirty = BT_DIRTY;

    return;
}




/* @funcstatic btreeWriteNumBucketEmpty ***************************************
**
** Write empty index numeric bucket object to the cache given a disc page number
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
**
** @return [void]
** @@
******************************************************************************/

static void btreeWriteNumBucketEmpty(AjPBtcache cache, ajulong pagepos)
{
    AjPBtpage page      = NULL;
    unsigned char *buf  = NULL;

    ajuint   v   = 0;
    ajulong lv   = 0L;
    ajulong overflow = 0L;

    /* ajuint unused = 0; */

    /* ajDebug("In btreeWriteNumBucket\n"); */

    if(pagepos == cache->totsize)
    {
	page = ajBtreeCacheWriteBucketnew(cache);
	buf = page->buf;
	overflow = 0L;
	lv = overflow;
	SBT_BUCKOVERFLOW(buf,lv);
    }
    else
    {
	page = ajBtreeCacheRead(cache,pagepos);
	buf = page->buf;
	GBT_BUCKOVERFLOW(buf,&overflow);
    }

    v = BT_NUMBUCKET;
    SBT_BUCKNODETYPE(buf,v);
    page->dirty = BT_LOCK;      /* clear at end */
    page->lockfor = 1581;

    v = 0;
    SBT_BUCKNENTRIES(buf,v);
    
    lv = 0L;
    SBT_BUCKOVERFLOW(buf,lv);
    /*
    lptr = PBT_BUCKKEYLEN(buf);


    unused = cache->pagesize - (lptr - buf);
    if(unused > cache->pagesize)
        ajFatal("unused %u for pagesize %u", unused, cache->pagesize);
    if(unused > 0)
        memset(lptr, 0, unused);
    */

    page->dirty = BT_DIRTY;     /* clear the lock */

    return;
}




/* @funcstatic btreeNumBucketIdlist *******************************************
**
** Copies all numeric IDs into a list
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
** @param [u] idlist [AjPList] list to hold numeric ID strings
**
** @return [ajulong] Overflow
** @@
******************************************************************************/

static ajulong btreeNumBucketIdlist(AjPBtcache cache, ajulong pagepos,
                                    AjPList idlist)
{
    AjPBtNumId num      = NULL;
    AjPBtpage page      = NULL;
    unsigned char *buf  = NULL;
    unsigned char *pptr = NULL;
    ajuint  nodetype  = 0;
    ajuint  nentries  = 0;
    ajulong overflow  = 0L;
    ajulong pageoverflow = 0L;
    ajuint  dirtysave = 0;
    
    ajuint  i;
    
    /* ajDebug("In btreeNumBucketIdlist\n"); */

    if(pagepos == cache->secrootblock)
	ajFatal("NumBucketIdlist: cannot read bucket from root page cache %S",
                cache->filename);

    page = ajBtreeCacheRead(cache,pagepos);
    dirtysave = page->dirty;
    page->dirty = BT_LOCK;
    page->lockfor = 1591;
    buf = page->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);
    if(nodetype != BT_NUMBUCKET && nodetype != BT_BUCKET)
	ajFatal("NumBucketIdlist: Nodetype mismatch. "
                "Not bucket (%u) cache %S",
                nodetype, cache->filename);

    GBT_BUCKNENTRIES(buf,&nentries);
    if(nentries > cache->snperbucket)
	ajFatal("NumBucketIdlist: Bucket too full page: %Lu "
                "entries: %u max: %u cache %S",
                pagepos, nentries, cache->snperbucket, cache->filename);

    GBT_BUCKOVERFLOW(buf,&overflow);
    pageoverflow = overflow;

    pptr = PBT_BUCKKEYLEN(buf);

    for(i=0;i<nentries;++i)
    {
        AJNEW0(num);
	BT_GETAJULONG(pptr,&num->offset);
	pptr += sizeof(ajulong);
	BT_GETAJULONG(pptr,&num->refoffset);
	pptr += sizeof(ajulong);
	BT_GETAJUINT(pptr,&num->dbno);
	pptr += sizeof(ajuint);
        ajListPushAppend(idlist, num);
        num = NULL;
    }
    
    page->dirty = dirtysave;

    return pageoverflow;
}




/* @funcstatic btreeNumBucketBtidlist ******************************************
**
** Copies all numeric IDs into a list of btree Ids
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
** @param [r] idname [const AjPStr] id name
** @param [u] idlist [AjPList] list to hold numeric ID strings
**
** @return [ajulong] Overflow
** @@
******************************************************************************/

static ajulong btreeNumBucketBtidlist(AjPBtcache cache, ajulong pagepos,
                                      const AjPStr idname, AjPList idlist)
{
    AjPBtId btid      = NULL;
    AjPBtpage page      = NULL;
    unsigned char *buf  = NULL;
    unsigned char *pptr = NULL;
    ajuint  nodetype  = 0;
    ajuint  nentries  = 0;
    ajulong overflow  = 0L;
    ajulong pageoverflow = 0L;
    ajuint  dirtysave = 0;
    
    ajuint  i;
    
    /* ajDebug("In btreeNumBucketIdlist\n"); */

    if(pagepos == cache->secrootblock)
	ajFatal("NumBucketBtidlist: cannot read bucket "
                "from root page cache %S",
                cache->filename);

    page = ajBtreeCacheRead(cache,pagepos);
    dirtysave = page->dirty;
    page->dirty = BT_LOCK;
    page->lockfor = 1601;
    buf = page->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);
    if(nodetype != BT_NUMBUCKET && nodetype != BT_BUCKET)
	ajFatal("NumBucketBtidlist: Nodetype mismatch. "
                "Not bucket (%u) cache %S",
                nodetype, cache->filename);

    GBT_BUCKNENTRIES(buf,&nentries);
    if(nentries > cache->snperbucket)
	ajFatal("NumBucketBtidlist: Bucket too full page: %Lu "
                "entries: %u max: %u cache %S",
                pagepos, nentries, cache->snperbucket,
                cache->filename);

    GBT_BUCKOVERFLOW(buf,&overflow);
    pageoverflow = overflow;

    pptr = PBT_BUCKKEYLEN(buf);

    for(i=0;i<nentries;++i)
    {
        btid = ajBtreeIdNew();
        ajStrAssignS(&btid->id,idname);
	BT_GETAJULONG(pptr,&btid->offset);
	pptr += sizeof(ajulong);
	BT_GETAJULONG(pptr,&btid->refoffset);
	pptr += sizeof(ajulong);
	BT_GETAJUINT(pptr,&btid->dbno);
	pptr += sizeof(ajuint);
        ajListPushAppend(idlist, btid);
        btid = NULL;
    }
    
    page->dirty = dirtysave;

    return pageoverflow;
}




/* @funcstatic btreeReadNumBucket *********************************************
**
** Constructor for index bucket given a disc page number
** Creates one empty key slot for possible addition
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
**
** @return [AjPNumBucket] bucket
** @@
******************************************************************************/

static AjPNumBucket btreeReadNumBucket(AjPBtcache cache, ajulong pagepos)
{
    AjPNumBucket bucket    = NULL;
    AjPBtpage page      = NULL;
    unsigned char *buf  = NULL;
    unsigned char *pptr = NULL;
    ajuint  nodetype  = 0;
    ajuint nentries  = 0;
    ajulong overflow  = 0L;
    ajuint  dirtysave = 0;
    
    ajuint  i;
    
    /* ajDebug("In btreeReadNumBucket\n"); */

    if(pagepos == cache->secrootblock)
	ajFatal("ReadNumBucket: cannot read bucket from a root page");

    page = ajBtreeCacheRead(cache,pagepos);
    dirtysave = page->dirty;
    page->dirty = BT_LOCK;
    page->lockfor = 1611;
    buf = page->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);

    if(nodetype != BT_NUMBUCKET && nodetype !=  BT_BUCKET)
	ajFatal("ReadNumBucket: Nodetype mismatch. Not bucket (%u) cache %S",
                nodetype, cache->filename);

    GBT_BUCKNENTRIES(buf,&nentries);

    if(nentries > cache->snperbucket)
	ajFatal("ReadNumBucket: Bucket too full page: %Lu "
                "entries: %u max: %u cache %S",
                pagepos, nentries, cache->snperbucket,
                cache->filename);

    GBT_BUCKOVERFLOW(buf,&overflow);

    bucket = btreeNumBucketNew(cache->snperbucket);
    bucket->Nentries = nentries;

    pptr = PBT_BUCKKEYLEN(buf);

    for(i=0;i<nentries;++i)
    {
	BT_GETAJULONG(pptr,&bucket->NumId[i]->offset);
	pptr += sizeof(ajulong);
	BT_GETAJULONG(pptr,&bucket->NumId[i]->refoffset);
	pptr += sizeof(ajulong);
	BT_GETAJUINT(pptr,&bucket->NumId[i]->dbno);
	pptr += sizeof(ajuint);
    }
    
    page->dirty = dirtysave;

    return bucket;
}




/* @funcstatic btreeNumBucketDel *********************************************
**
** Delete a bucket object
**
** @param [w] thys [AjPNumBucket*] bucket
**
** @return [void]
** @@
******************************************************************************/

static void btreeNumBucketDel(AjPNumBucket *thys)
{
    AjPNumBucket pthis = NULL;
    ajuint newmax;
    
    /* ajDebug("In btreeNumBucketDel\n"); */

    if(!thys || !*thys)
	return;

    pthis = *thys;

    if(!statSaveNumBucket)
    {
        statSaveNumBucketMax=2048;
        statSaveNumBucketNext=0;
        AJCNEW0(statSaveNumBucket,statSaveNumBucketMax);
    }
    
    if(statSaveNumBucketNext >= statSaveNumBucketMax)
    {
        newmax = statSaveNumBucketMax + statSaveNumBucketMax;
        AJCRESIZE0(statSaveNumBucket,statSaveNumBucketMax,newmax);
        statSaveNumBucketMax = newmax;
    }
        
    statSaveNumBucket[statSaveNumBucketNext++] = pthis;
 
    *thys = NULL;

    return;
}




/* @funcstatic btreeNumBucketFree *********************************************
**
** Delete a bucket object
**
** @param [w] thys [AjPNumBucket*] bucket
**
** @return [void]
** @@
******************************************************************************/

static void btreeNumBucketFree(AjPNumBucket *thys)
{
    AjPNumBucket pthis = NULL;
    ajuint i;
    
    
    /* ajDebug("In btreeNumBucketFree\n"); */

    if(!thys || !*thys)
	return;

    pthis = *thys;

    if(pthis->NumId)
    {
	for(i=0;i<pthis->Maxentries;++i)
	    AJFREE(pthis->NumId[i]);
	AJFREE(pthis->NumId);
    }

    AJFREE(pthis);

    *thys = NULL;

    return;
}




/* @func ajBtreeNumFindInsert ***********************************************
**
** Find the node that should contain a new key for insertion
**
** @param [u] cache [AjPBtcache] cache
** @param [r] key [const ajulong] key to search for 
**
** @return [AjPBtpage] leaf node where item should be inserted
** @@
******************************************************************************/

AjPBtpage ajBtreeNumFindInsert(AjPBtcache cache, const ajulong key)
{
    AjPBtpage root = NULL;
    AjPBtpage ret  = NULL;

    /* ajDebug("In ajBtreeNumFindInsert\n"); */

    /* The root node should always be in the cache (BT_LOCKed) */
    root = btreeCacheLocate(cache,cache->secrootblock);

    /* ajDebug("cache->slevel = %u root=%u\n",cache->slevel,(ajuint)root); */
    

    if(!cache->slevel)
	return root;
    
    ret = btreeNumFindINode(cache,root,key);

    return ret;
}




/* @funcstatic btreeNumFindINode **********************************************
**
** Recursive search for insert node in a secondary tree
**
** @param [u] cache [AjPBtcache] cache
** @param [u] page [AjPBtpage] page
** @param [r] item [ajulong] key to search for 
**
** @return [AjPBtpage] leaf node where item should be inserted
** @@
******************************************************************************/

static AjPBtpage btreeNumFindINode(AjPBtcache cache, AjPBtpage page,
				   ajulong item)
{
    AjPBtpage ret = NULL;
    AjPBtpage pg  = NULL;

    unsigned char *buf = NULL;
    ajuint status       = 0;
    ajuint ival         = 0;

    /* ajDebug("In btreeNumFindINode\n"); */
    
    ret = page;
    buf = page->buf;
    GBT_NODETYPE(buf,&ival);

    if(ival != BT_LEAF)
    {
	status = ret->dirty;
	ret->dirty = BT_LOCK;	/* Lock in case of lots of overflow pages */
        ret->lockfor = 1621;
	pg = btreeNumPageFromKey(cache,buf,item);
	ret->dirty = status;
	ret = btreeNumFindINode(cache,pg,item);
    }
    
    return ret;
}




/* @funcstatic btreeNumPageFromKey *******************************************
**
** Return next lower index page given a key in a secondary tree
**
** @param [u] cache [AjPBtcache] cache
** @param [u] buf [unsigned char *] page buffer
** @param [r] key [ajulong] key to search for 
**
** @return [AjPBtpage] pointer to a page
** @@
******************************************************************************/

static AjPBtpage btreeNumPageFromKey(AjPBtcache cache, unsigned char *buf,
				     ajulong key)
{
    unsigned char *rootbuf = NULL;
    ajuint nkeys = 0;
    ajuint i;
    
    ajulong blockno = 0L;
    ajulong *karray = NULL;
    ajulong *parray = NULL;
    AjPBtpage page = NULL;
    AjPBtMem array = NULL;

    /* ajDebug("In btreeNumPageFromKey\n"); */
    
    rootbuf = buf;


    GBT_NKEYS(rootbuf,&nkeys);

    array = btreeAllocSecArray(cache);
    karray = array->overflows;
    parray = array->parray;
    
    btreeGetNumKeys(cache,rootbuf,&karray,&parray);

    i = 0;

    while(i!=nkeys && key >= karray[i])
	++i;

    if(i==nkeys)
    {
	if(key < karray[i-1])
	    blockno = parray[i-1];
	else
	    blockno = parray[i];
    }
    else
	blockno = parray[i];

    btreeDeallocSecArray(cache,array);

    page =  ajBtreeCacheRead(cache,blockno);

    return page;
}




/* @funcstatic btreeAddToNumBucket *******************************************
**
** Add offset info to a bucket
** Only called if there is room in the bucket
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number of bucket
** @param [r] num [const AjPBtNumId] ID info
**
** @return [void]
** @@
******************************************************************************/

static void btreeAddToNumBucket(AjPBtcache cache, ajulong pagepos,
				const AjPBtNumId num)
{
    AjPBtpage page      = NULL;
    unsigned char *buf  = NULL;
    unsigned char *pptr = NULL;
    ajuint  nodetype  = 0;
    ajuint  nentries  = 0;
    ajulong overflow  = 0L;

    ajulong lv = 0L;
    ajuint uv = 0;

    ajuint  i;

    /* ajDebug("In btreeReadNumBucket\n"); */

    if(pagepos == cache->secrootblock)
	ajFatal("AddToNumBucket: cannot read bucket from root page cache %S",
                cache->filename);

    page = ajBtreeCacheRead(cache,pagepos);

    page->dirty = BT_LOCK;      /* reset at end */
    page->lockfor = 1631;
    buf = page->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);
    if(nodetype != BT_NUMBUCKET)
	ajFatal("AddToNumBucket: Nodetype mismatch. "
                "Not bucket (%u) cache %S",
                nodetype, cache->filename);

    GBT_BUCKNENTRIES(buf,&nentries);
    if(nentries > cache->snperbucket)
	ajFatal("AddToNumBucket: Bucket too full page: %Lu "
                "entries: %u max: %u cache %S",
                pagepos, nentries, cache->snperbucket, cache->filename);

    GBT_BUCKOVERFLOW(buf,&overflow);

    pptr = PBT_BUCKKEYLEN(buf);

    for(i=0;i<nentries;++i)
    {
        pptr += sizeof(ajulong);
        pptr += sizeof(ajulong);
	pptr += sizeof(ajuint);
    }
    
    lv = num->offset;
    BT_SETAJULONG(pptr,lv);
    pptr += sizeof(ajulong);
    lv = num->refoffset;
    BT_SETAJULONG(pptr,lv);
    pptr += sizeof(ajulong);
    uv = num->dbno;
    BT_SETAJUINT(pptr,uv);
    pptr += sizeof(ajuint);

    nentries++;
    SBT_BUCKNENTRIES(buf,nentries);

    page->dirty = BT_DIRTY;

    return;
}





/* @funcstatic btreeNumInNumBucket *******************************************
**
** Return number of entries in a bucket
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
**
** @return [ajuint] Number of entries in bucket
** @@
******************************************************************************/

static ajuint btreeNumInNumBucket(AjPBtcache cache, ajulong pagepos)
{
    AjPBtpage page     = NULL;
    unsigned char *buf = NULL;
    ajuint  nodetype    = 0;
    ajuint nentries    = 0;
    
    /* ajDebug("In btreeNumInNumBucket\n"); */
    
    if(pagepos == cache->secrootblock)
	ajFatal("NumInNumBucket: Attempt to read bucket from root page\n");

    page  = ajBtreeCacheRead(cache,pagepos);

    buf = page->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);

    if(nodetype != BT_NUMBUCKET)
	ajFatal("NumInNumBucket: NodeType mismatch. Not numbucket (%u) "
                "for cache '%S'",
		nodetype, cache->filename);
    
    GBT_BUCKNENTRIES(buf,&nentries);

    return nentries;
}




/* @funcstatic btreeNumBucketNew *********************************************
**
** Construct a bucket object
**
** @param [r] n [ajuint] Number of IDs
**
** @return [AjPNumBucket] initialised disc block cache structure
** @@
******************************************************************************/

static AjPNumBucket btreeNumBucketNew(ajuint n)
{
    AjPNumBucket bucket = NULL;
    ajuint i;

    /* ajDebug("In btreeNumBucketNew\n"); */
    if(statSaveNumBucketNext)
    {
       bucket = statSaveNumBucket[--statSaveNumBucketNext];
       for(i=0;i<bucket->Maxentries;++i)
       {
           bucket->NumId[i]->offset = 0L;
           bucket->NumId[i]->refoffset = 0L;
           bucket->NumId[i]->dbno = 0;
       }
       if(n > bucket->Maxentries)
       {
           if(bucket->Maxentries)
               AJCRESIZE0(bucket->NumId,bucket->Maxentries+1,n+1);
           else
               AJCNEW0(bucket->NumId,n+1);
           for(i=bucket->Maxentries;i<n;++i)
               AJNEW0(bucket->NumId[i]);
           bucket->Maxentries = n;
       }
   }
   else
   {
       AJNEW0(bucket);
       AJCNEW0(bucket->NumId,n+1);
       for(i=0;i<n;++i)
           AJNEW0(bucket->NumId[i]);
       bucket->Maxentries = n;
   }

    bucket->NodeType = BT_NUMBUCKET;
    bucket->Nentries = n;
    bucket->Overflow = 0L;
    
    return bucket;
}




/* @funcstatic btreeReorderNumBuckets *****************************************
**
** Re-order leaf buckets
** Must only be called if one of the buckets is full
**
** @param [u] cache [AjPBtcache] cache
** @param [u] leaf [AjPBtpage] leaf page
**
** @return [AjBool] true if reorder was successful i.e. leaf not full
** @@
******************************************************************************/

static AjBool btreeReorderNumBuckets(AjPBtcache cache, AjPBtpage leaf)
{
    ajuint nkeys = 0;
    unsigned char *lbuf = NULL;

    AjPBtMem array = NULL;
    AjPBtMem newarray = NULL;
    ajulong *ptrs        = NULL;
    ajulong *newkeys        = NULL;
    ajulong *newptrs        = NULL;

    ajuint i = 0;
    
    ajuint order;
    ajuint totalkeys     = 0;
    ajuint nperbucket    = 0;
    ajuint maxnperbucket = 0;
    ajuint count         = 0;
    ajuint keylimit      = 0;
    ajuint bucketlimit   = 0;
    ajuint nodetype      = 0;
    
    AjPList idlist    = NULL;
    ajuint   dirtysave = 0;
    AjPBtNumId bid       = NULL;
    AjPNumBucket cbucket = NULL;
    AjPBtNumId cid       = NULL;

    ajuint   iold = 0;
    
#if AJINDEX_DEBUG
    ajDebug("btreeReorderNumBuckets %Lu\n", leaf->pagepos);
#endif
    ++statCallReorderNumBuckets;

    /* ajDebug("In btreeReorderNumBuckets\n"); */

    dirtysave = leaf->dirty;

    leaf->dirty = BT_LOCK;
    leaf->lockfor = 1641;
    lbuf = leaf->buf;

    GBT_NODETYPE(lbuf,&nodetype);

    order = cache->sorder;
    nperbucket = cache->snperbucket;
    

    array = btreeAllocSecArray(cache);
    ptrs  = array->parray;

    /* Read keys/ptrs */

    btreeGetNumPointers(cache,lbuf,&ptrs);

    GBT_NKEYS(lbuf,&nkeys);
    keylimit = nkeys + 1;


    if(!nkeys)
	ajFatal("ReorderNumBuckets: Attempt to reorder empty leaf");

    for(i=0;i<nkeys;++i)
	totalkeys += btreeNumInNumBucket(cache,ptrs[i]);

    totalkeys += btreeNumInNumBucket(cache,ptrs[i]);

    btreeBucketCalc(totalkeys, keylimit, nperbucket,
                    &bucketlimit, &maxnperbucket);

#if AJINDEX_DEBUG
    ajDebug("   bucketlimit:%u order: %u\n", bucketlimit, order);
#endif
    if(bucketlimit >= order)
    {
	btreeDeallocSecArray(cache,array);
	
	leaf->dirty = dirtysave;

#if AJINDEX_DEBUG
    ajDebug("   return ajFalse\n");
#endif
	return ajFalse;
    }
    

    newarray = btreeAllocSecArray(cache);
    newkeys  = newarray->overflows;
    newptrs  = newarray->parray;

    /* Read buckets */
    
    idlist  = ajListNew();

    /* Read IDs from all buckets and push to list and sort (increasing id) */
    for(i=0;i<keylimit;++i)
	btreeNumBucketIdlist(cache,ptrs[i],idlist);

    ajListSort(idlist,btreeNumIdCompare);

    cbucket = btreeNumBucketNew(maxnperbucket);
    iold = 0;
    for(i=0;i<bucketlimit;++i)
    {
	cbucket->Nentries = 0;

	count = 0;

	while(count!=maxnperbucket)
	{
	    ajListPop(idlist,(void **)&bid);
	    
	    cid = cbucket->NumId[count];
	    cid->dbno = bid->dbno;
	    cid->offset = bid->offset;
	    cid->refoffset = bid->refoffset;
	    
	    ++cbucket->Nentries;
	    ++count;
	    AJFREE(bid);
	}


	ajListPeek(idlist,(void **)&bid);
	newkeys[i] = bid->offset;

	if((iold < order) && ptrs[iold])
            newptrs[i] = ptrs[iold++];
        else
	    newptrs[i] = cache->totsize;
	btreeWriteNumBucket(cache,cbucket,newptrs[i]);
    }


    /* Deal with greater-than bucket */

    cbucket->Nentries = 0;

    count = 0;

    while(ajListPop(idlist,(void **)&bid))
    {
	cid = cbucket->NumId[count];
	cid->dbno = bid->dbno;
	cid->offset = bid->offset;
	cid->refoffset = bid->refoffset;
	
	++cbucket->Nentries;
	++count;
	AJFREE(bid);
    }
    
    if((iold < order) && ptrs[iold])
        newptrs[i] = ptrs[iold++];
    else
        newptrs[i] = cache->totsize;
    btreeWriteNumBucket(cache,cbucket,newptrs[i]);

    btreeNumBucketDel(&cbucket);

#if AJINDEX_DEBUG
    if(iold < keylimit)
        ajDebug("btreeReorderNumBuckets '%S' %u -> %u",
                cache->filename, keylimit, iold);
#endif

    for(i = iold+1; i <= keylimit; i++)
    {
        btreePageSetfree(cache, ptrs[i]);
    }

    /* Now write out a modified leaf with new keys/ptrs */

    nkeys = bucketlimit;

    btreeWriteNumNode(cache,leaf,newkeys,newptrs,nkeys);

    leaf->dirty = BT_DIRTY;

    if(nodetype == BT_ROOT)
    {
	leaf->dirty = BT_LOCK;
        leaf->lockfor = 1642;
    }

    btreeDeallocSecArray(cache,array);
    btreeDeallocSecArray(cache,newarray);

    ajListFree(&idlist);

#if AJINDEX_DEBUG
    ajDebug("   return ajTrue\n");
#endif

    return ajTrue;
}




/* @funcstatic btreeNumNodeIsFull *****************************************
**
** Tests whether a node is full of keys
**
** @param [r] cache [const AjPBtcache] cache
** @param [u] page [AjPBtpage] original page
**
** @return [AjBool] true if full
** @@
******************************************************************************/

static AjBool btreeNumNodeIsFull(const AjPBtcache cache, AjPBtpage page)
{
    unsigned char *buf = NULL;
    ajuint nkeys = 0;

    /* ajDebug("In btreeNumNodeIsFull\n"); */

    buf = page->buf;
    GBT_NKEYS(buf,&nkeys);

    if(nkeys == cache->sorder - 1)
	return ajTrue;

    return ajFalse;
}




/* @funcstatic btreeNumInsertNonFull *****************************************
**
** Insert a key into a non-full node
**
** @param [u] cache [AjPBtcache] cache
** @param [u] page [AjPBtpage] original page
** @param [r] key [ajulong] key to insert
** @param [r] less [ajulong] less-than pointer
** @param [r] greater [ajulong] greater-than pointer
**
** @return [void]
** @@
******************************************************************************/

static void btreeNumInsertNonFull(AjPBtcache cache, AjPBtpage page,
				  ajulong key, ajulong less,
				  ajulong greater)
{
    unsigned char *buf = NULL;
    ajulong *karray     = NULL;
    ajulong *parray     = NULL;
    ajuint nkeys  = 0;
    ajuint ipos   = 0;
    ajuint i;
    ajuint count  = 0;

    ajulong lv = 0L;
    ajuint  v  = 0;
    AjPBtMem array = NULL;    

    AjPBtpage ppage = NULL;
    ajulong pagepos   = 0L;

    ajuint nodetype = 0;
    
    /* ajDebug("In btreeNumInsertNonFull\n"); */

    array = btreeAllocSecArray(cache);
    karray  = array->overflows;
    parray  = array->parray;


    buf = page->buf;
    GBT_NKEYS(buf,&nkeys);
    GBT_NODETYPE(buf,&nodetype);
    
    btreeGetNumKeys(cache,buf,&karray,&parray);

    i = 0;

    while(i!=nkeys && key >= karray[i])
	++i;

    ipos = i;

    count = nkeys - ipos;
    

    if(ipos == nkeys)
    {
	karray[ipos] = key;
	parray[ipos+1] = greater;
	parray[ipos]   = less;
    }
    else
    {
	parray[nkeys+1] = parray[nkeys];

	for(i=nkeys-1; count>0; --count, --i)
	{
	    karray[i+1] = karray[i];
	    parray[i+1] = parray[i];
	}

	karray[ipos] = key;
	parray[ipos] = less;
	parray[ipos+1] = greater;
    }

    ++nkeys;
    v = nkeys;
    SBT_NKEYS(buf,v);

    btreeWriteNumNode(cache,page,karray,parray,nkeys);
    if(nodetype == BT_ROOT)
    {
	page->dirty = BT_LOCK;
        page->lockfor = 1651;
    }

    pagepos = page->pagepos;
    ppage = ajBtreeCacheRead(cache,less);
    lv = pagepos;
    SBT_PREV(ppage->buf,lv);
    ppage->dirty = BT_DIRTY;

    ppage = ajBtreeCacheRead(cache,greater);
    lv = pagepos;
    SBT_PREV(ppage->buf,lv);
    ppage->dirty = BT_DIRTY;

    btreeDeallocSecArray(cache,array);

    if(nodetype != BT_ROOT)
	btreeNumKeyShift(cache,page);

    return;
}




/* @funcstatic btreeNumInsertKey **********************************************
**
** Insert a key into a potentially full node
**
** @param [u] cache [AjPBtcache] cache
** @param [u] page [AjPBtpage] original page
** @param [r] key [ajulong] key to insert
** @param [r] less [ajulong] less-than pointer
** @param [r] greater [ajulong] greater-than pointer
**
** @return [void]
** @@
******************************************************************************/

static void btreeNumInsertKey(AjPBtcache cache, AjPBtpage page,
			      ajulong key, ajulong less, ajulong greater)
{
    unsigned char *lbuf = NULL;
    unsigned char *rbuf = NULL;
    unsigned char *tbuf = NULL;
    ajulong *narray      = NULL;
    ajulong *parray      = NULL;
    ajulong *tnarray     = NULL;
    ajulong *tparray     = NULL;
    ajuint nkeys    = 0;
    ajuint order    = 0;
    ajuint keypos   = 0;
    ajuint rkeyno   = 0;
    
    ajuint i = 0;
    ajuint n = 0;
    
    ajuint nodetype  = 0;
    AjPBtpage ipage = NULL;
    AjPBtpage lpage = NULL;
    AjPBtpage rpage = NULL;
    AjPBtpage tpage = NULL;

    ajulong blockno  = 0L;
    ajulong rblockno = 0L;
    ajulong lblockno = 0L;
    ajulong ibn      = 0L;

    ajulong mediankey  = 0L;
    ajulong medianless = 0L;
    ajulong mediangtr  = 0L;
    ajulong overflow   = 0L;
    ajulong prev       = 0L;
    
    ajulong lv = 0L;
    ajuint  v  = 0;
    AjPBtMem savekeyarray  = NULL;    
    AjPBtMem array2 = NULL;    
    
    /* ajDebug("In btreeNumInsertKey\n"); */

    if(!btreeNumNodeIsFull(cache,page))
    {
	btreeNumInsertNonFull(cache,page,key,less,greater);
	return;
    }
    
    order = cache->sorder;
    lbuf = page->buf;
    GBT_NODETYPE(lbuf,&nodetype);
    page->dirty = BT_LOCK;
    page->lockfor = 1661;

    if(nodetype == BT_ROOT)
    {
	btreeNumSplitRoot(cache);

	if(page->pagepos)
	    page->dirty = BT_DIRTY;

        blockno = btreeGetBlockFirstN(cache,lbuf,key);

	ipage = ajBtreeCacheRead(cache,blockno);
	btreeNumInsertNonFull(cache,ipage,key,less,greater);

	return;
    }


    savekeyarray = btreeAllocSecArray(cache);
    narray  = savekeyarray->overflows;
    parray  = savekeyarray->parray;
    
    array2 = btreeAllocSecArray(cache);
    tnarray  = array2->overflows;
    tparray  = array2->parray;

    
    lpage = page;
    lbuf = lpage->buf;
    
    btreeGetNumKeys(cache,lbuf,&narray,&parray);

    GBT_BLOCKNUMBER(lbuf,&lblockno);

    rblockno = cache->totsize;
    rpage = ajBtreeCacheWriteNodenew(cache);
    rpage->dirty = BT_LOCK;
    rpage->lockfor = 1662;
    rbuf = rpage->buf;

    
    GBT_PREV(lbuf,&prev);
    lv = prev;
    SBT_PREV(rbuf,lv);

    nkeys = order - 1;
    keypos = nkeys / 2;

    if(!(nkeys % 2))
	--keypos;

    mediankey = narray[keypos];
    medianless = lblockno;
    mediangtr  = rblockno;


    GBT_NODETYPE(lbuf,&nodetype);
    v = nodetype;
    SBT_NODETYPE(rbuf,v);
    lv = overflow;
    SBT_OVERFLOW(rbuf,lv);


    for(i=0;i<keypos;++i)
    {
	tnarray[i] = narray[i];
	tparray[i] = parray[i];
    }

    tparray[i] = parray[i];

    btreeWriteNumNode(cache,lpage,tnarray,tparray,i);
    lpage->dirty = BT_LOCK;
    lpage->lockfor = 1663;


    for(i=0;i<n+1;++i)
    {
	tpage = ajBtreeCacheRead(cache,tparray[i]);
	tbuf = tpage->buf;
	lv = lblockno;
	SBT_PREV(tbuf,lv);
	tpage->dirty = BT_DIRTY;
    }


    for(i=keypos+1;i<nkeys;++i)
    {
	tnarray[i-(keypos+1)] = narray[i];
	tparray[i-(keypos+1)] = parray[i];
    }

    tparray[i-(keypos+1)] = parray[i];

    rkeyno = (nkeys-keypos) - 1;
    v = rkeyno;
    SBT_NKEYS(rbuf,v);
    rpage->dirty = BT_DIRTY;

    btreeWriteNumNode(cache,rpage,tnarray,tparray,rkeyno);
    rpage->dirty = BT_LOCK;
    rpage->lockfor = 1664;

    for(i=0;i<rkeyno+1;++i)
    {
	tpage = ajBtreeCacheRead(cache,tparray[i]);
	tbuf = tpage->buf;
	lv = rblockno;
	SBT_PREV(tbuf,lv);
	tpage->dirty = BT_DIRTY;
    }

    btreeDeallocSecArray(cache,array2);

    ibn = rblockno;
    if(key < mediankey)
	ibn = lblockno;

    lpage->dirty = BT_DIRTY;
    rpage->dirty = BT_DIRTY;

    ipage = ajBtreeCacheRead(cache,ibn);
    
    btreeNumInsertNonFull(cache,ipage,key,less,greater);

    ipage = ajBtreeCacheRead(cache,prev);

    btreeNumInsertKey(cache,ipage,mediankey,medianless,mediangtr);

    btreeDeallocSecArray(cache,savekeyarray);

    return;
}




/* @funcstatic btreeNumSplitRoot *****************************************
**
** Split s secondary root node
**
** @param [u] cache [AjPBtcache] cache
**
** @return [void]
** @@
******************************************************************************/

static void btreeNumSplitRoot(AjPBtcache cache)
{
    AjPBtpage rootpage = NULL;
    AjPBtpage rpage    = NULL;
    AjPBtpage lpage    = NULL;
    AjPBtpage tpage    = NULL;

    ajulong *karray  = NULL;
    ajulong *tkarray = NULL;
    ajulong *parray  = NULL;
    ajulong *tparray = NULL;

    ajuint order  = 0;
    ajuint nkeys  = 0;
    ajuint keypos = 0;
    
    ajulong rblockno = 0L;
    ajulong lblockno = 0L;

    ajulong right;
    
    ajuint i;

    unsigned char *rootbuf = NULL;
    unsigned char *rbuf    = NULL;
    unsigned char *lbuf    = NULL;
    unsigned char *tbuf    = NULL;
    
    ajuint nodetype  = 0;
    ajulong overflow = 0L;
    ajulong zero     = 0L;
    ajuint rkeyno   = 0;
    ajuint n        = 0;

    ajulong lv = 0L;
    ajuint  v  = 0;

    AjPBtMem array  = NULL;    
    AjPBtMem array2 = NULL;    
    
    
#if AJINDEX_DEBUG
    ajDebug("btreeNumSplitRoot %Lu\n", cache->secrootblock);
#endif
    ++statCallSplitNumRoot;

    /* ajDebug("In btreeNumSplitRoot\n"); */

    order = cache->sorder;

    array = btreeAllocSecArray(cache);
    karray  = array->overflows;
    parray  = array->parray;
    
    array2 = btreeAllocSecArray(cache);
    tkarray  = array2->overflows;
    tparray  = array2->parray;


    rootpage = btreeCacheLocate(cache,cache->secrootblock);
    rootbuf = rootpage->buf;

    nkeys = order - 1;

    keypos = nkeys / 2;

    if(!(nkeys % 2))
	--keypos;


    rblockno = cache->totsize;
    rpage = ajBtreeCacheWriteNodenew(cache);
    rpage->dirty = BT_LOCK;
    rpage->lockfor = 1671;

    lblockno = cache->totsize;
    lpage = ajBtreeCacheWriteNodenew(cache);


    /* Comment this next block out after the beta test */
    if(!cache->slevel)
    {
	fprintf(stderr,"btreeNumSplitRoot: Shouldn't get here\n");
	exit(0);
	lv = zero;
	SBT_LEFT(lpage->buf,lv);
	lv = rblockno;
	SBT_RIGHT(lpage->buf,lv);
	lv = lblockno;
	SBT_LEFT(rpage->buf,lv);
	lv = zero;
	SBT_RIGHT(rpage->buf,lv);
    }

    btreeGetNumKeys(cache,rootbuf,&karray,&parray);

    /* Get key for root node and write new root node */
    tkarray[0] = karray[keypos];
    tparray[0] = lblockno;
    tparray[1] = rblockno;
    

    n = 1;
    v = n;
    SBT_NKEYS(rootbuf,v);
    btreeWriteNumNode(cache,rootpage,tkarray,tparray,1);
    right = (ajulong)(cache->slevel + 1);
    SBT_RIGHT(rootbuf,right);

    rootpage->dirty = BT_LOCK;
    rootpage->lockfor = 1672;

    rbuf = rpage->buf;
    lbuf = lpage->buf;
    
    if(cache->slevel)
	nodetype = BT_INTERNAL;
    else
	nodetype = BT_LEAF;

    v = nodetype;
    SBT_NODETYPE(rbuf,v);
    v = nodetype;
    SBT_NODETYPE(lbuf,v);
    lv = overflow;
    SBT_OVERFLOW(rbuf,lv);
    lv = cache->secrootblock;
    SBT_PREV(rbuf,lv);
    lv = overflow;
    SBT_OVERFLOW(lbuf,lv);
    lv = cache->secrootblock;
    SBT_PREV(lbuf,lv);

    for(i=0;i<keypos;++i)
    {
	tkarray[i] = karray[i];
	tparray[i] = parray[i];
    }

    tparray[i] = parray[i];

    n = i;
    v = n;
    SBT_NKEYS(lbuf,v);
    btreeWriteNumNode(cache,lpage,tkarray,tparray,i);

    for(i=0;i<n+1;++i)
    {
	tpage = ajBtreeCacheRead(cache,tparray[i]);
	tbuf = tpage->buf;
	lv = lblockno;
	SBT_PREV(tbuf,lv);
	tpage->dirty = BT_DIRTY;
    }

    for(i=keypos+1;i<nkeys;++i)
    {
	tkarray[i-(keypos+1)] = karray[i];
	tparray[i-(keypos+1)] = parray[i];
    }

    tparray[i-(keypos+1)] = parray[i];

    rkeyno = (nkeys-keypos) - 1;
    v = rkeyno;
    SBT_NKEYS(rbuf,v);
    rpage->dirty = BT_DIRTY;

    btreeWriteNumNode(cache,rpage,tkarray,tparray,rkeyno);

    for(i=0;i<rkeyno+1;++i)
    {
	tpage = ajBtreeCacheRead(cache,tparray[i]);
	tbuf = tpage->buf;
	lv = rblockno;
	SBT_PREV(tbuf,lv);
	tpage->dirty = BT_DIRTY;
    }


    ++cache->slevel;

    btreeDeallocSecArray(cache,array);
    btreeDeallocSecArray(cache,array2);
    
    return;
}




/* @funcstatic btreeNumKeyShift ********************************************
**
** Rebalance Nodes on insertion
**
** @param [u] cache [AjPBtcache] cache
** @param [u] tpage [AjPBtpage] page
**
** @return [void]
** @@
******************************************************************************/

static void btreeNumKeyShift(AjPBtcache cache, AjPBtpage tpage)
{
    unsigned char *tbuf = NULL;
    unsigned char *pbuf = NULL;
    unsigned char *sbuf = NULL;
    unsigned char *buf  = NULL;

    AjPBtpage ppage = NULL;
    AjPBtpage spage = NULL;
    AjPBtpage page  = NULL;

    ajuint tkeys = 0;
    ajuint pkeys = 0;
    ajuint skeys = 0;
    ajuint order = 0;
    
    ajuint i;
    ajint ii;

    ajulong parent  = 0L;
    
    AjPBtMem array  = NULL;    
    AjPBtMem array2 = NULL;    
    AjPBtMem array3 = NULL;    
    ajulong *kTarray = NULL;
    ajulong *kParray = NULL;
    ajulong *kSarray = NULL;
    ajulong *pTarray = NULL;
    ajulong *pParray = NULL;
    ajulong *pSarray = NULL;

    ajuint pkeypos = 0;
    ajuint minsize = 0;

    ajulong lv = 0L;
    
    /* ajDebug("In btreeNumKeyShift\n"); */
    
    tbuf = tpage->buf;

    GBT_PREV(tbuf,&parent);
    GBT_NKEYS(tbuf,&tkeys);

    order = cache->sorder;
    minsize = order / 2;

    if(order % 2)
	++minsize;

    if(tkeys <= minsize)
	return;

    
    ppage = ajBtreeCacheRead(cache,parent);
    pbuf = ppage->buf;
    GBT_NKEYS(pbuf,&pkeys);
    
    array = btreeAllocSecArray(cache);
    kParray  = array->overflows;
    pParray  = array->parray;
    
    array2 = btreeAllocSecArray(cache);
    kSarray  = array2->overflows;
    pSarray  = array2->parray;

    array3 = btreeAllocSecArray(cache);
    kTarray  = array3->overflows;
    pTarray  = array3->parray;


    btreeGetNumKeys(cache,tbuf,&kTarray,&pTarray);
    GBT_NKEYS(tbuf,&tkeys);


    btreeGetNumKeys(cache,pbuf,&kParray,&pParray);

    i=0;

    while(pParray[i] != tpage->pagepos)
	++i;

    if(i) /* There is another leaf to the left */
    {
	pkeypos = i-1;
	spage = ajBtreeCacheRead(cache,pParray[pkeypos]);
	sbuf = spage->buf;
	GBT_NKEYS(sbuf,&skeys);
	
    }

    if(i && skeys != order-1) /* There is space in the left leaf */
    {
	if(skeys)
	    btreeGetNumKeys(cache,sbuf,&kSarray,&pSarray);

	kSarray[skeys] = kParray[pkeypos];
	pSarray[skeys+1] = pTarray[0];
	++skeys;
	--tkeys;
	kParray[pkeypos] = kTarray[0];

	for(i=0;i<tkeys;++i)
	{
	    kTarray[i] = kTarray[i+1];
	    pTarray[i] = pTarray[i+1];
	}

	pTarray[i] = pTarray[i+1];
	pTarray[i+1] = 0L;
	
	btreeWriteNumNode(cache,spage,kSarray,pSarray,skeys);
	btreeWriteNumNode(cache,tpage,kTarray,pTarray,tkeys);
	btreeWriteNumNode(cache,ppage,kParray,pParray,pkeys);
	if(ppage->pagepos == cache->secrootblock)
        {
	    ppage->dirty = BT_LOCK;
            ppage->lockfor = 1681;
        }

	page = ajBtreeCacheRead(cache,pSarray[skeys]);
	buf = page->buf;
	lv = spage->pagepos;
	SBT_PREV(buf,lv);
	page->dirty = BT_DIRTY;

	btreeDeallocSecArray(cache,array);
	btreeDeallocSecArray(cache,array2);
	btreeDeallocSecArray(cache,array3);

	return;
    }



    if(i != pkeys)	/* There is a right node */
    {
	pkeypos = i;
	spage = ajBtreeCacheRead(cache,pParray[pkeypos+1]);
	sbuf = spage->buf;
	GBT_NKEYS(sbuf,&skeys);
    }


    if(i != pkeys && skeys != order-1) /* Space in the right node */
    {
	if(skeys)
	    btreeGetNumKeys(cache,sbuf,&kSarray,&pSarray);

	pSarray[skeys+1] = pSarray[skeys];

	for(ii=skeys-1;ii>-1;--ii)
	{
	    kSarray[ii+1] = kSarray[ii];
	    pSarray[ii+1] = pSarray[ii];
	}

	kSarray[0] = kParray[pkeypos];
	pSarray[0] = pTarray[tkeys];
	kParray[pkeypos] = kTarray[tkeys-1];
	++skeys;
	--tkeys;
	pTarray[tkeys+1] = 0L;
	
	btreeWriteNumNode(cache,spage,kSarray,pSarray,skeys);
	btreeWriteNumNode(cache,tpage,kTarray,pTarray,tkeys);
	btreeWriteNumNode(cache,ppage,kParray,pParray,pkeys);

	if(ppage->pagepos == cache->secrootblock)
        {
	    ppage->dirty = BT_LOCK;
            ppage->lockfor = 1682;
        }

	page = ajBtreeCacheRead(cache,pSarray[0]);
	buf = page->buf;
	lv = spage->pagepos;
	SBT_PREV(buf,lv);
	page->dirty = BT_DIRTY;

	btreeDeallocSecArray(cache,array);
	btreeDeallocSecArray(cache,array2);
	btreeDeallocSecArray(cache,array3);

	return;
    }


    btreeDeallocSecArray(cache,array);
    btreeDeallocSecArray(cache,array2);
    btreeDeallocSecArray(cache,array3);

    return;
}




/* @funcstatic btreeNumInsertShift ********************************************
**
** Rebalance buckets on insertion
**
** @param [u] cache [AjPBtcache] cache
** @param [u] retpage [AjPBtpage*] page
** @param [r] key [ajulong] key
**
** @return [ajulong] bucket block or 0L if shift not possible
** @@
******************************************************************************/

static ajulong btreeNumInsertShift(AjPBtcache cache, AjPBtpage *retpage,
                                   ajulong key)
{
    unsigned char *tbuf = NULL;
    unsigned char *pbuf = NULL;
    unsigned char *sbuf = NULL;

    AjPBtpage ppage = NULL;
    AjPBtpage spage = NULL;
    AjPBtpage tpage = NULL;

    ajuint tkeys = 0;
    ajuint pkeys = 0;
    ajuint skeys = 0;
    ajuint order = 0;
    
    ajuint i;
    ajuint n;
    ajint ii;
    
    ajulong parent  = 0L;
    ajulong blockno = 0L;
    
    ajulong *kTarray = NULL;
    ajulong *kParray = NULL;
    ajulong *kSarray = NULL;
    ajulong *pTarray = NULL;
    ajulong *pParray = NULL;
    ajulong *pSarray = NULL;

    ajulong *karray = NULL;
    ajulong *parray = NULL;

    ajuint ppos    = 0;
    ajuint pkeypos = 0;
    ajuint minsize = 0;

    AjPBtMem array  = NULL;    
    AjPBtMem array2 = NULL;    
    AjPBtMem array3 = NULL;    


    
    /* ajDebug("In btreeNumInsertShift\n"); */


    tpage = *retpage;

    tbuf = tpage->buf;

    GBT_PREV(tbuf,&parent);
    GBT_NKEYS(tbuf,&tkeys);


    order = cache->sorder;
    minsize = order / 2;

    if(order % 2)
	++minsize;

    if(tkeys <= minsize)
	return 0L;

    ppage = ajBtreeCacheRead(cache,parent);


    
    pbuf = ppage->buf;
    GBT_NKEYS(pbuf,&pkeys);

    
    array = btreeAllocSecArray(cache);
    kParray  = array->overflows;
    pParray  = array->parray;
    
    array2 = btreeAllocSecArray(cache);
    kSarray  = array2->overflows;
    pSarray  = array2->parray;

    array3 = btreeAllocSecArray(cache);
    kTarray  = array3->overflows;
    pTarray  = array3->parray;


    btreeGetNumKeys(cache,pbuf,&kParray,&pParray);

    i=0;

    while(i!=pkeys && key >= kParray[i])
	++i;

    pkeypos = i;
    
    if(i==pkeys)
    {
	if(key < kParray[i-1])
	    ppos = i-1;
	else
	    ppos = i;
    }
    else
	ppos = i;

    
    if(ppos) /* There is another leaf to the left */
    {
	spage = ajBtreeCacheRead(cache,pParray[ppos-1]);
	sbuf = spage->buf;
	GBT_NKEYS(sbuf,&skeys);
    }

    if(i && skeys != order-1) /* There is space in the left leaf */
    {
	/* ajDebug("Left shift\n"); */
	btreeGetNumKeys(cache,tbuf,&kTarray,&pTarray);
	if(skeys)
	    btreeGetNumKeys(cache,sbuf,&kSarray,&pSarray);

	i = 0;

	while(pParray[i] != tpage->pagepos)
	    ++i;

	--i;

	pkeypos = i;

	kSarray[skeys] = kParray[pkeypos];
	pSarray[skeys+1] = pTarray[0];
	++skeys;
	--tkeys;
	kParray[pkeypos] = kTarray[0];

	for(i=0;i<tkeys;++i)
	{
	    kTarray[i] = kTarray[i+1];
	    pTarray[i] = pTarray[i+1];
	}

	pTarray[i] = pTarray[i+1];
	pTarray[i+1] = 0L;
	
	btreeWriteNumNode(cache,spage,kSarray,pSarray,skeys);
	btreeWriteNumNode(cache,tpage,kTarray,pTarray,tkeys);
	btreeWriteNumNode(cache,ppage,kParray,pParray,pkeys);
	if(ppage->pagepos == cache->secrootblock)
        {
	    ppage->dirty = BT_LOCK;
            ppage->lockfor = 1691;
       }

	i = 0;

	while(i!=pkeys && key >= kParray[i])
	    ++i;

	if(i==pkeys)
	{
	    if(key < kParray[i-1])
		blockno = pParray[i-1];
	    else
		blockno = pParray[i];
	}
	else
	    blockno = pParray[i];

	if(blockno == spage->pagepos)
	{
	    *retpage = spage;
	    karray = kSarray;
	    parray = pSarray;
	    n = skeys;
	}
	else
	{
	    karray = kTarray;
	    parray = pTarray;
	    n = tkeys;
	}
	

	i = 0;

	while(i!=n && key >= karray[i])
	    ++i;

	if(i==n)
	{
	    if(key < karray[i-1])
		blockno = parray[i-1];
	    else
		blockno = parray[i];
	}
	else
	    blockno = parray[i];

	btreeDeallocSecArray(cache,array);
	btreeDeallocSecArray(cache,array2);
	btreeDeallocSecArray(cache,array3);

	/* ajDebug("... returns blockno (a) %Lu\n",blockno); */

	return blockno;
    }
    

    if(ppos != pkeys)	/* There is a right node */
    {
	spage = ajBtreeCacheRead(cache,pParray[ppos+1]);
	sbuf = spage->buf;
	GBT_NKEYS(sbuf,&skeys);
    }


    /* Space in the right leaf */
    if(ppos != pkeys && skeys != order-1)
    {
	/* ajDebug("Right shift\n"); */
	btreeGetNumKeys(cache,tbuf,&kTarray,&pTarray);
	btreeGetNumKeys(cache,sbuf,&kSarray,&pSarray);

	i = 0;

	while(pParray[i] != tpage->pagepos)
	    ++i;

	pkeypos = i;
	
	pSarray[skeys+1] = pSarray[skeys];

	for(ii=skeys-1;ii>-1;--ii)
	{
	    kSarray[ii+1] = kSarray[ii];
	    pSarray[ii+1] = pSarray[ii];
	}

	kSarray[0] = kParray[pkeypos];
	pSarray[0] = pTarray[tkeys];
	kParray[pkeypos] = kTarray[tkeys-1];
	++skeys;
	--tkeys;
	pTarray[tkeys+1] = 0L;
	
	btreeWriteNumNode(cache,spage,kSarray,pSarray,skeys);
	btreeWriteNumNode(cache,tpage,kTarray,pTarray,tkeys);
	btreeWriteNumNode(cache,ppage,kParray,pParray,pkeys);
	if(ppage->pagepos == cache->secrootblock)
        {
            ppage->dirty = BT_LOCK;
            ppage->lockfor = 1692;
        }

	i = 0;

	while(i!=pkeys && key >= kParray[i])
	    ++i;

	if(i==pkeys)
	{
	    if(key < kParray[i-1])
		blockno = pParray[i-1];
	    else
		blockno = pParray[i];
	}
	else
	    blockno = pParray[i];

	if(blockno == spage->pagepos)
	{
	    *retpage = spage;
	    karray = kSarray;
	    parray = pSarray;
	    n = skeys;
	}
	else
	{
	    karray = kTarray;
	    parray = pTarray;
	    n = tkeys;
	}
	
	i = 0;

	while(i!=n && key >= karray[i])
	    ++i;

	if(i==n)
	{
	    if(key < karray[i-1])
		blockno = parray[i-1];
	    else
		blockno = parray[i];
	}
	else
	    blockno = parray[i];

	btreeDeallocSecArray(cache,array);
	btreeDeallocSecArray(cache,array2);
	btreeDeallocSecArray(cache,array3);

	/* ajDebug("... returns blockno (b) %Lu\n",blockno); */
	
	return blockno;
    }


    btreeDeallocSecArray(cache,array);
    btreeDeallocSecArray(cache,array2);
    btreeDeallocSecArray(cache,array3);

    /* ajDebug("... returns 0L\n"); */

    return 0L;
}




/* @func ajBtreeInsertNum *********************************************
**
** Insert a file offset key into a secondary tree
**
** @param [u] cache [AjPBtcache] cache
** @param [r] num [const AjPBtNumId] Id object
** @param [w] page [AjPBtpage] cache page
**
** @return [void] pointer to a page
** @@
******************************************************************************/

void ajBtreeInsertNum(AjPBtcache cache, const AjPBtNumId num, AjPBtpage page)
{
    unsigned char *buf = NULL;
    ajulong numkey;
    AjPBtpage spage  = NULL;
    ajuint nkeys = 0;
    ajuint nodetype = 0;

    ajulong lblockno;
    ajulong rblockno;
    ajulong blockno;
    ajulong shift;
    ajuint n;

    /* ajDebug("In ajBtreeInsertNum\n"); */

    (void) page;			/* make it used */

    numkey = num->offset;

    spage = ajBtreeNumFindInsert(cache,numkey);
    buf = spage->buf;

    GBT_NKEYS(buf,&nkeys);
    GBT_NODETYPE(buf,&nodetype);

    if(!nkeys)
    {
	lblockno = cache->totsize;
	btreeWriteNumBucketEmpty(cache,lblockno);
	rblockno = cache->totsize;
	btreeWriteNumBucketEmpty(cache,rblockno);

	btreeWriteNumNodeSingle(cache,spage,numkey,lblockno,rblockno);

	btreeAddToNumBucket(cache,rblockno,num);

	return;
    }

    blockno = btreeGetBlockN(cache,buf,numkey);

    if(nodetype != BT_ROOT)
	if((shift = btreeNumInsertShift(cache,&spage,numkey)))
	    blockno = shift;


    buf = spage->buf;
    n = btreeNumInNumBucket(cache,blockno);

    if(n == cache->snperbucket)
    {
	if(btreeReorderNumBuckets(cache,spage))
	{
            blockno = btreeGetBlockN(cache,buf,numkey);
	}
	else
	{
	    btreeNumSplitLeaf(cache,spage);
	    spage = ajBtreeNumFindInsert(cache,numkey);
	    buf = spage->buf;

	    blockno = btreeGetBlockN(cache,buf,numkey);
	}
    }

    btreeAddToNumBucket(cache,blockno,num);

    return;
}




/* @funcstatic btreeNumSplitLeaf *********************************************
**
** Split a leaf and propagate up if necessary
**
** @param [u] cache [AjPBtcache] cache
** @param [u] spage [AjPBtpage] page
**
** @return [AjPBtpage] pointer to a parent page
** @@
******************************************************************************/

static AjPBtpage btreeNumSplitLeaf(AjPBtcache cache, AjPBtpage spage)
{
    ajuint nkeys     = 0;
    ajuint totalkeys = 0;
    ajuint keylimit  = 0;
    ajuint order     = 0;
    ajuint nodetype  = 0;

    ajuint rootnodetype = 0;
    
    ajuint i;
    ajuint j;
    
    AjPBtpage lpage = NULL;
    AjPBtpage rpage = NULL;
    AjPBtpage page  = NULL;
    
    ajulong mediankey  = 0L;
    ajulong mediangtr  = 0L;
    ajulong medianless = 0L;

    AjPBtNumId bid = NULL;

    unsigned char *buf  = NULL;
    unsigned char *lbuf = NULL;
    unsigned char *rbuf = NULL;

    AjPList idlist = NULL;

    AjPNumBucket cbucket  = NULL;
    
    AjPBtMem array = NULL;
    AjPBtMem newarray = NULL;
    ajulong *newnarray = NULL;
    ajulong *parray = NULL;
    ajulong *newparray = NULL;
    
    ajuint lno    = 0;
    ajuint rno    = 0;

    ajuint lbucketlimit   = 0;
    ajuint rbucketlimit   = 0;
    ajuint lmaxnperbucket = 0;
    ajuint rmaxnperbucket = 0;
    ajuint nperbucket    = 0;
    ajuint count         = 0;
    
    ajulong lblockno = 0L;
    ajulong rblockno = 0L;
    ajulong overflow = 0L;
    ajulong prevsave = 0L;
    
    ajulong zero = 0L;
    ajulong join = 0L;

    ajuint iold = 0;

    ajulong lv = 0L;
    ajuint  v  = 0;

    
#if AJINDEX_DEBUG
    ajDebug("btreeNumSplitLeaf %Lu dirty:%u lockfor:%u\n",
            spage->pagepos, spage->dirty, spage->lockfor);
#endif
    ++statCallSplitNumLeaf;

    /* ajDebug("In btreeNumSplitLeaf\n"); */

    nperbucket = cache->snperbucket;
    order = cache->sorder;

    array = btreeAllocSecArray(cache);
    parray = array->parray;
    newarray = btreeAllocSecArray(cache);
    newparray = newarray->parray;
    newnarray = newarray->overflows;

    buf = spage->buf;
    lbuf = buf;

    GBT_NKEYS(buf,&nkeys);
    GBT_NODETYPE(buf,&rootnodetype);

#if AJINDEX_DEBUG
    ajDebug("    nkeys:%u nodetype:%u\n", nkeys, rootnodetype);
#endif

    if(rootnodetype == BT_ROOT)
    {
#if AJINDEX_DEBUG
	ajDebug("   Root leaf splitting page:%Lu secroot:%Lu\n",
                cache->totsize, cache->secrootblock);
#endif
	lblockno = cache->totsize;
	lpage = ajBtreeCacheWriteNodenew(cache);
	lbuf = lpage->buf;
	lv = cache->secrootblock;
	SBT_PREV(lbuf,lv);
    }
    else
    {
	lblockno = spage->pagepos;
	lpage = spage;
    }

    lpage->dirty = BT_LOCK;
    lpage->lockfor = 1701;

    rblockno = cache->totsize;
    rpage = ajBtreeCacheWriteNodenew(cache);
    rbuf = rpage->buf;
    rpage->dirty = BT_LOCK;
    rpage->lockfor = 1702;

    if(rootnodetype == BT_ROOT)
    {
	lv = zero;
	SBT_RIGHT(rbuf,lv);
	lv = zero;
	SBT_LEFT(lbuf,lv);
    }
    else
    {
	GBT_RIGHT(lbuf,&join);
	lv = join;
	SBT_RIGHT(rbuf,lv);
    }

    lv = lblockno;
    SBT_LEFT(rbuf,lv);
    lv = rblockno;
    SBT_RIGHT(lbuf,lv);


    btreeGetNumPointers(cache,buf,&parray);


    keylimit = nkeys+1;
    idlist = ajListNew();
    for(i=0;i<keylimit;++i)
	btreeNumBucketIdlist(cache,parray[i], idlist);

    ajListSort(idlist,btreeNumIdCompare);

    totalkeys = ajListGetLength(idlist);

    btreeBucketSplitCalc(totalkeys, keylimit, nperbucket,
                         &lbucketlimit,&lmaxnperbucket,&lno,
                         &rbucketlimit,&rmaxnperbucket,&rno);

    cbucket = btreeNumBucketNew(cache->snperbucket);

    count = 0;
    iold = 0;

    for(i=0;i<lbucketlimit;++i)
    {
	cbucket->Nentries = 0;

	for(j=0;j<lmaxnperbucket;++j)
	{
	    ajListPop(idlist,(void **)&bid);

	    cbucket->NumId[j]->offset    = bid->offset;
	    cbucket->NumId[j]->refoffset = bid->refoffset;
	    cbucket->NumId[j]->dbno      = bid->dbno;
	    
	    ++count;
	    ++cbucket->Nentries;
	    AJFREE(bid);
	}

	ajListPeek(idlist,(void **)&bid);

	newnarray[i] = bid->offset;

        if((iold < order) && parray[iold])
            newparray[i] = parray[iold++];
        else
            newparray[i] = cache->totsize;

	btreeWriteNumBucket(cache,cbucket,newparray[i]);
    }

    cbucket->Nentries = 0;

    j = 0;

    while(count != lno)
    {
	ajListPop(idlist,(void **)&bid);

	cbucket->NumId[j]->offset    = bid->offset;
	cbucket->NumId[j]->refoffset = bid->refoffset;
	cbucket->NumId[j]->dbno      = bid->dbno;

	++j;
	++count;


	++cbucket->Nentries;
	AJFREE(bid);
    }

    if((iold < order) && parray[iold])
        newparray[i] = parray[iold++];
    else
        newparray[i] = cache->totsize;

    btreeWriteNumBucket(cache,cbucket,newparray[i]);

    nkeys = lbucketlimit;
    nodetype = BT_LEAF;
    v = nodetype;
    SBT_NODETYPE(lbuf,v);
    GBT_PREV(lbuf,&prevsave);
    lpage->dirty = BT_DIRTY;

    btreeWriteNumNode(cache,lpage,newnarray,newparray,nkeys);

    ajListPeek(idlist,(void **)&bid);
    mediankey = bid->offset;

    for(i=0;i<rbucketlimit;++i)
    {
	cbucket->Nentries = 0;

	for(j=0;j<rmaxnperbucket;++j)
	{
	    ajListPop(idlist,(void **)&bid);
	    
	    cbucket->NumId[j]->offset    = bid->offset;
	    cbucket->NumId[j]->refoffset = bid->refoffset;
	    cbucket->NumId[j]->dbno      = bid->dbno;
	    
	    ++cbucket->Nentries;
	    AJFREE(bid);
	}

	ajListPeek(idlist,(void **)&bid);
	newnarray[i] = bid->offset;

        if((iold < order) && parray[iold])
            newparray[i] = parray[iold++];
        else
            newparray[i] = cache->totsize;

	btreeWriteNumBucket(cache,cbucket,newparray[i]);
    }

    cbucket->Nentries = 0;

    j = 0;

    while(ajListPop(idlist,(void**)&bid))
    {
	cbucket->NumId[j]->offset    = bid->offset;
	cbucket->NumId[j]->refoffset = bid->refoffset;
	cbucket->NumId[j]->dbno      = bid->dbno;
	++j;


	++cbucket->Nentries;
	AJFREE(bid);
    }
    
    if((iold < order) && parray[iold])
        newparray[i] = parray[iold++];
    else
        newparray[i] = cache->totsize;

    btreeWriteNumBucket(cache,cbucket,newparray[i]);

#if AJINDEX_DEBUG
    ajDebug("end of NumSplitLeaf i:%u iold:%u, order:%u\n", i, iold, order);
#endif
    nkeys = rbucketlimit;

    nodetype = BT_LEAF;
    v = nodetype;
    SBT_NODETYPE(rbuf,v);
    lv = prevsave;
    SBT_PREV(rbuf,lv);
    lv = overflow;
    SBT_OVERFLOW(rbuf,lv);
    
    btreeWriteNumNode(cache,rpage,newnarray,newparray,nkeys);
    rpage->dirty = BT_DIRTY;

    btreeNumBucketDel(&cbucket);
    ajListFree(&idlist);



    medianless = lblockno;
    mediangtr  = rblockno;


    if(rootnodetype == BT_ROOT)
    {
	spage->dirty = BT_DIRTY;
	btreeWriteNumNodeSingle(cache,spage,mediankey,lblockno,rblockno);	
	++cache->slevel;
	lv = cache->slevel;
	SBT_RIGHT(buf,lv);
	spage->dirty = BT_LOCK;
        spage->lockfor = 1703;

	btreeDeallocSecArray(cache,array);
        btreeDeallocSecArray(cache,newarray);

	return spage;
    }

    btreeDeallocSecArray(cache,array);
    btreeDeallocSecArray(cache,newarray);


    page = ajBtreeCacheRead(cache,prevsave);
    btreeNumInsertKey(cache,page,mediankey,medianless,mediangtr);


    page = ajBtreeCacheRead(cache,prevsave);

    return page;
}




/* @func ajBtreeFreePriArray *******************************************
**
** Free karray and parray arrays for a primary key
**
** @param [u] cache [AjPBtcache] cache
**
** @return [void]
** @@
******************************************************************************/

void ajBtreeFreePriArray(AjPBtcache cache)
{
    AjPBtMem p;
    AjPBtMem next;
    ajuint i;

    /* ajDebug("In ajBtreeFreePriArray\n"); */
    
    if(!cache->bmem)
	return;

    p = cache->bmem;
    
    while(p)
    {
        next = p->next;

        AJFREE(p->parray);
	AJFREE(p->overflows);

	for(i=0;i<cache->order;++i)
	    ajStrDel(&p->karray[i]);

	AJFREE(p->karray);
	AJFREE(p);
	p = next;
    }
    

    cache->bmem = NULL;
    cache->tmem = NULL;

    return;
}




/* @func ajBtreeFreeSecArray *******************************************
**
** Free karray and parray arrays for a secondary key
**
** @param [u] cache [AjPBtcache] cache
**
** @return [void]
** @@
******************************************************************************/

void ajBtreeFreeSecArray(AjPBtcache cache)
{
    AjPBtMem p;
    AjPBtMem next;
    ajuint i;

    /* ajDebug("In ajBtreeFreeSecArray\n"); */
    
    if(!cache->bsmem)
	return;

    p = cache->bsmem;
    
    while(p)
    {
        next = p->next;

	AJFREE(p->parray);
	AJFREE(p->overflows);

	for(i=0;i<cache->sorder;++i)
	    ajStrDel(&p->karray[i]);

	AJFREE(p->karray);
	AJFREE(p);
	
	p = next;
    }
    

    cache->bsmem = NULL;
    cache->tsmem = NULL;

    return;
}




/* @func ajBtreeHybLeafList ********************************************
**
** Read the leaves of a secondary hybrid tree
**
** @param [u] cache [AjPBtcache] cache
** @param [r] rootblock [ajulong] root page of secondary tree
** @param [r] idname [const AjPStr] id name
** @param [u] list [AjPList] list to add BtIDs to
**
** @return [void]
** @@
******************************************************************************/

void ajBtreeHybLeafList(AjPBtcache cache, ajulong rootblock,
			const AjPStr idname, AjPList list)
{
    AjPBtMem array = NULL;
    ajulong *parray;

    AjPBtpage page;
    unsigned char *buf;

    ajuint nodetype;
    ajuint i;
    ajulong level = 0L;
    
    ajuint nkeys;
    ajulong right;

    array = btreeAllocSecArray(cache);
    parray = array->parray;

    page = ajBtreeCacheRead(cache, rootblock);
    buf = page->buf;

    GBT_RIGHT(buf,&level);
    cache->slevel = (ajuint) level;

    btreeGetNumPointers(cache,buf,&parray);
    GBT_NODETYPE(buf,&nodetype);

    while(nodetype != BT_LEAF && cache->slevel != 0)
    {
	page = ajBtreeCacheRead(cache,parray[0]);
	buf = page->buf;
	btreeGetNumPointers(cache,buf,&parray);
	GBT_NODETYPE(buf,&nodetype);
    }

    do
    {
	GBT_NKEYS(buf,&nkeys);

	for(i=0;i<nkeys+1;++i)
	    btreeNumBucketBtidlist(cache,parray[i],idname,list);

	right = 0L;
	if(cache->slevel)
	{
	    GBT_RIGHT(buf,&right);

	    if(right)
	    {
		page = ajBtreeCacheRead(cache,right);
		buf = page->buf;
		btreeGetNumPointers(cache,buf,&parray);
	    }
	}
    } while(right);

    btreeDeallocSecArray(cache,array);

    return;
}




/* @func ajBtreeDumpHybKeys ***************************************************
**
** Read the leaves of a primary hybrid tree (requested by EBI services)
**
** @param [u] cache [AjPBtcache] cache
** @param [r] dmin [ajuint] minimum number of times the key should appear
** @param [r] dmax [ajuint] maximum number of times the key should appear
** @param [u] outf [AjPFile] output file
**
** @return [void]
** @@
******************************************************************************/

void ajBtreeDumpHybKeys(AjPBtcache cache, ajuint dmin, ajuint dmax,
                        AjPFile outf)
{
    AjPBtMem array = NULL;
    AjPStr *karray;
    ajulong *parray;

    AjPBtpage page;
    unsigned char *buf;

    ajuint nodetype;
    ajuint i;
    ajuint j;
    ajuint dups;
    
    AjPBucket bucket;
    ajuint nkeys;
    ajulong right;

    if(cache->secondary)
    {
        ajBtreeDumpKeywords(cache, dmin, dmax, outf);
        return;
    }

    array = btreeAllocPriArray(cache);
    karray = array->karray;
    parray = array->parray;

    page = ajBtreeCacheRead(cache, 0);
    buf = page->buf;

    btreeGetKeys(cache,buf,&karray,&parray);
    GBT_NODETYPE(buf,&nodetype);

    while(nodetype != BT_LEAF && cache->level != 0)
    {
	page = ajBtreeCacheRead(cache,parray[0]);
	buf = page->buf;
	btreeGetKeys(cache,buf,&karray,&parray);
	GBT_NODETYPE(buf,&nodetype);
    }

    do
    {
	GBT_NKEYS(buf,&nkeys);
	for(i=0;i<=nkeys;++i)
	{
	    bucket = btreeReadBucket(cache,parray[i]);

	    for(j=0;j<bucket->Nentries;++j)
	    {
		dups = bucket->Ids[j]->dups;

		if(!dups)
		    dups = 1;
		
		if(dups < dmin)
		    continue;

		if(dmax && dups > dmax)
		    continue;

		ajFmtPrintF(outf,"%10d %S\n",
			    dups, bucket->Ids[j]->id);
	    }

	    btreeBucketDel(&bucket);
	}

	right = 0L;

	if(cache->level)
	{
	    GBT_RIGHT(buf,&right);

	    if(right)
	    {
		page = ajBtreeCacheRead(cache,right);
		buf = page->buf;
		btreeGetKeys(cache,buf,&karray,&parray);
	    }
	}
    } while(right);

    btreeDeallocPriArray(cache,array);

    return;
}




/* @func ajBtreeDeleteHybId *********************************************
**
** Entry point for hybrid ID deletion.
**
** Deletion software
**
** @param [u] cache [AjPBtcache] cache
** @param [r] hyb [const AjPBtHybrid] hybrid object
**
** @return [AjBool] True if found and deleted
** @@
******************************************************************************/

AjBool ajBtreeDeleteHybId(AjPBtcache cache, const AjPBtHybrid hyb)
{
    AjPBtpage rootpage = NULL;
    AjPBtpage spage   = NULL;
    AjPStr key        = NULL;
    AjPBucket bucket  = NULL;
    ajulong blockno  = 0L;

    ajuint nkeys = 0;

    ajuint nodetype = 0;
    ajuint nentries = 0;
    
    AjPStr *karray = NULL;
    ajulong *parray = NULL;
    AjPBtMem arrays = NULL;
    AjBool found = ajFalse;
    ajuint dups = 0;
    
    ajuint i;
    
    unsigned char *buf = NULL;
    AjPBtId did = NULL;
    ajulong  secrootpage = 0L;
    AjBool ret = ajFalse;
    
    /* ajDebug("In ajBtreeDeleteHybId\n"); */

    key = ajStrNew();
    

    ajStrAssignS(&key,hyb->key1);

    if(!ajStrGetLen(key))
    {
	ajStrDel(&key);

	return ajFalse;
    }

    spage = ajBtreeHybFindInsert(cache,key);
    buf = spage->buf;

    GBT_NKEYS(buf,&nkeys);
    GBT_NODETYPE(buf,&nodetype);
    
    arrays = btreeAllocPriArray(cache);
    karray = arrays->karray;
    parray = arrays->parray;
    
    if(!nkeys)
    {
	btreeDeallocPriArray(cache,arrays);
	ajStrDel(&key);

	return ajFalse;
    }


    /* Search to see whether entry exists */
    
    btreeGetKeys(cache,buf,&karray,&parray);

    i=0;

    while(i!=nkeys && MAJSTRCMPS(key,karray[i])>=0)
	++i;

    blockno = parray[i];


    bucket = btreeReadBucket(cache,blockno);
    
    nentries = bucket->Nentries;
    
    found = ajFalse;

    for(i=0;i<nentries;++i)
	if(ajStrMatchS(hyb->key1,bucket->Ids[i]->id))
	{
	    found = ajTrue;
	    break;
	}
    

    if(!found)
    {
	btreeDeallocPriArray(cache,arrays);
	ajStrDel(&key);

	return ajFalse;
    }
    

    dups = bucket->Ids[i]->dups;
    

    if(!dups)
    {
        /* ajDebug("No secondary tree\n"); */
        rootpage = btreeCacheLocate(cache,0L);

        if(!rootpage)
            ajFatal("Rootpage has been unlocked (ajBtreeDeleteHybId)");
    
        rootpage->dirty = BT_LOCK;
        rootpage->lockfor = 1711;

        btreeFindHybBalanceOne(cache,0L,BTNO_NODE,BTNO_NODE,BTNO_NODE,
                               BTNO_NODE,hyb);


        btreeDeallocPriArray(cache,arrays);
	ajStrDel(&key);

        if(cache->deleted)
            ret = ajTrue;
        else
            ret = ajFalse;
    }
    else
    {
        did = bucket->Ids[i];
        secrootpage = did->offset;
        cache->secrootblock = secrootpage;

        ret = btreeDeleteHybIdTwo(cache,hyb,did);

        btreeWriteBucket(cache,bucket,blockno);
    }
    

    return ret;
}




/* @funcstatic btreeFindHybBalanceOne ******************************************
**
** Master routine for entry deletion from level 1 hybrid tree.
**
** Deletion software.
**
** @param [u] cache [AjPBtcache] cache
** @param [r] thisNode [ajulong] Current node
** @param [r] leftNode [ajulong] Node to left
** @param [r] rightNode [ajulong] Node to right
** @param [r] lAnchor [ajulong] Left anchor
** @param [r] rAnchor [ajulong] Right anchor
** @param [r] hyb [const AjPBtHybrid] hyb
**
** @return [ajulong] page number or BTNO_NODE
** @@
******************************************************************************/

static ajulong btreeFindHybBalanceOne(AjPBtcache cache, ajulong thisNode,
                                      ajulong leftNode, ajulong rightNode,
                                      ajulong lAnchor, ajulong rAnchor,
                                      const AjPBtHybrid hyb)
{
    unsigned char *buf  = NULL;
    unsigned char *buf1 = NULL;
    
    ajulong nextNode   = BTNO_NODE;
    ajulong nextLeft   = BTNO_NODE;
    ajulong nextRight  = BTNO_NODE;
    ajulong nextAncL   = BTNO_NODE;
    ajulong nextAncR   = BTNO_NODE;
    ajulong done       = 0L;
    
    ajuint  nkeys      = 0;
    ajuint  order      = 0;
    ajuint  minkeys    = 0;
    ajuint  i;
    ajuint   nodetype   = 0;

    ajuint n1keys      = 0;
    
    AjPBtpage page  = NULL;
    AjPBtpage page1 = NULL;

    ajulong balanceNode = 0L;
    ajulong blockno     = 0L;
    ajulong ptrSave     = 0L;

    AjPStr *karray  = NULL;
    ajulong *parray  = NULL;
    AjPStr *k1array = NULL;
    ajulong *p1array = NULL;

    AjPBtMem arrays  = NULL;
    AjPBtMem arrays1 = NULL;
    
    const AjPStr key = NULL;
    AjBool existed = ajFalse;
    
    /* ajDebug("In btreeFindBalance\n"); */

    if(thisNode)
	page = ajBtreeCacheRead(cache,thisNode);
    else
    {   /* It's the root node of the primary hyb tree */
        /* Needs altering for secondary tree          */
	page = btreeCacheLocate(cache,thisNode);
	page->dirty = BT_LOCK;
        page->lockfor = 1721;
    }

    cache->deleted = ajFalse;

    buf = page->buf;
    GBT_NKEYS(buf,&nkeys);

    order = cache->order;
    /* order-1 is the number of keys in the node */
    minkeys = (order-1) / 2;

    if((order-1)%2)
	++minkeys;

    /*
    ** If thisNode contains >= minkeys then it is not a candidate
    ** for balancing
    */
    if(nkeys >= minkeys)
	balanceNode = BTNO_BALANCE;
    else
	balanceNode = page->pagepos;

    arrays  = btreeAllocPriArray(cache);
    arrays1 = btreeAllocPriArray(cache);

    karray = arrays->karray;
    parray = arrays->parray;

    k1array = arrays1->karray;
    p1array = arrays1->parray;

    key = hyb->key1;

    btreeGetKeys(cache,buf,&karray,&parray);

    i=0;

    while(i!=nkeys && MAJSTRCMPS(key,karray[i])>=0)
	++i;

    blockno = parray[i];

    nextNode = blockno;
    ptrSave = i;

    GBT_NODETYPE(buf,&nodetype);

    if(!(nodetype == BT_LEAF) && !(nodetype == BT_ROOT && !cache->level))
    {
	if(nextNode == parray[0])
	{
	    if(leftNode != BTNO_NODE)
	    {
		page1 = ajBtreeCacheRead(cache,leftNode);
		buf1 = page1->buf;
		GBT_NKEYS(buf1,&n1keys);
		btreeGetKeys(cache,buf1,&k1array,&p1array);
		nextLeft = p1array[n1keys];
	    }
	    else
		nextLeft = BTNO_NODE;
	    
	    if(!thisNode)
		nextAncL = thisNode;
	    else
		nextAncL = lAnchor;
	}
	else
	{
	    nextLeft = parray[ptrSave-1];
	    nextAncL = thisNode;
	}

	if(nextNode == parray[nkeys])
	{
	    if(rightNode != BTNO_NODE)
	    {
		page1 = ajBtreeCacheRead(cache,rightNode);
		buf1 = page1->buf;
		GBT_NKEYS(buf1,&n1keys);
		btreeGetKeys(cache,buf1,&k1array,&p1array);
		nextRight = p1array[0];
	    }
	    else
		nextRight = BTNO_NODE;

	    if(!thisNode)
		nextAncR = thisNode;
	    else
		nextAncR = rAnchor;
	}
	else
	{
	    nextRight = parray[ptrSave+1];
	    nextAncR  = thisNode;
	}



	/* Check to see whether key exists in an internal node */
	if(nodetype != BT_LEAF && cache->level)
	{
	    i=0;

	    while(i!=nkeys && MAJSTRCMPS(key,karray[i]))
		++i;

	    if(i!=nkeys)
	    {
		btreeFindHybMinOne(cache,parray[i+1],key);
		ajStrAssignS(&karray[i],cache->replace);
		btreeWriteNode(cache,page,karray,parray,nkeys);
	    }
	
	}
	
	btreeFindHybBalanceOne(cache,nextNode,nextLeft,nextRight,
                               nextAncL,nextAncR,hyb);

	if(thisNode)
	    page = ajBtreeCacheRead(cache,thisNode);
	else
	{
	    page = btreeCacheLocate(cache,thisNode);
	    page->dirty = BT_LOCK;
            page->lockfor = 1722;
	}
	buf = page->buf;

    }
    else
    {
	if(nodetype == BT_LEAF || (nodetype==BT_ROOT && !cache->level))
	{
	    existed = btreeRemoveHybEntryOne(cache,thisNode,hyb);

	    if(existed)
		cache->deleted = ajTrue;

	    GBT_NKEYS(buf,&nkeys);

	    if(nkeys >= minkeys || (nodetype==BT_ROOT && !cache->level))
		balanceNode = BTNO_BALANCE;
	    else
		balanceNode = page->pagepos;
	}
    }


    if(balanceNode == BTNO_BALANCE || thisNode == 0L)
	done = BTNO_NODE;
    else
	done = btreeRebalanceHybOne(cache,thisNode,leftNode,rightNode,
                                    lAnchor,rAnchor);
    

    btreeDeallocPriArray(cache,arrays);
    btreeDeallocPriArray(cache,arrays1);

    return done;
}




/* @funcstatic btreeFindHybMinOne *********************************************
**
** Find minimum key in hybrid level 1 subtree and store in cache.
**
** Deletion software.
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page
** @param [r] key [const AjPStr] key
**
** @return [void]
** @@
******************************************************************************/

static void btreeFindHybMinOne(AjPBtcache cache, ajulong pagepos,
                               const AjPStr key)
{
    AjPBtpage page   = NULL;
    AjPBucket bucket = NULL;
    AjPStr *karray   = NULL;
    ajulong *parray   = NULL;

    ajuint nkeys     = 0;
    ajuint  nodetype = 0;
    ajuint nentries  = 0;
    ajuint i;
    AjPBtMem arrays = NULL;
    
    unsigned char *buf = NULL;

    /* ajDebug("In btreeFindHybMinOne\n"); */

    arrays = btreeAllocPriArray(cache);
    karray = arrays->karray;
    parray = arrays->parray;

    page = ajBtreeCacheRead(cache,pagepos);
    buf  = page->buf;
    GBT_NODETYPE(buf,&nodetype);
    GBT_NKEYS(buf,&nkeys);

    btreeGetKeys(cache,buf,&karray,&parray);

    if(nodetype == BT_LEAF)
    {
	bucket = btreeReadBucket(cache,parray[0]);
	nentries = bucket->Nentries;

        /*
        ** If there's only one entry then it must be the key marked
        ** for deletion
        */
        if(nentries<2)
	{
	    btreeBucketDel(&bucket);
	    bucket = btreeReadBucket(cache,parray[1]);
	    nentries = bucket->Nentries;
	}

        /* Check for empty bucket - shouldn't happen */
        /* Checking solely out of interest */
	if(nentries<1)	
	    ajFatal("FindHybMinOne: Too few entries in bucket Nkeys=%u\n",
                    nkeys);


        /* Find lowest value key in the bucket and store in cache */
        ajStrAssignS(&cache->replace,bucket->Ids[0]->id);
	if(!MAJSTRCMPS(cache->replace,key))
	    ajStrAssignS(&cache->replace,bucket->Ids[1]->id);

	for(i=1;i<nentries;++i)
	    if(MAJSTRCMPS(bucket->Ids[i]->id,cache->replace)<0 &&
	       MAJSTRCMPS(bucket->Ids[i]->id,key))
		ajStrAssignS(&cache->replace,bucket->Ids[i]->id);
	btreeBucketDel(&bucket);
    }
    else
    {
	pagepos = parray[0];
	btreeFindHybMinOne(cache,pagepos,key);
	
    }

    btreeDeallocPriArray(cache,arrays);
    
    return;
}




/* @funcstatic btreeRemoveHybEntryOne *****************************************
**
** Find and delete an ID from a given hybrid tree level 1 leaf node if
** necessary.
**
** Deletion software
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] leaf node page
** @param [r] hyb [const AjPBtHybrid] hyb
**
** @return [AjBool] True if found (and deleted)
** @@
******************************************************************************/

static AjBool btreeRemoveHybEntryOne(AjPBtcache cache, ajulong pagepos,
                                     const AjPBtHybrid hyb)
{
    AjPBtpage page   = NULL;
    AjPBucket bucket = NULL;
    
    AjPStr *karray = NULL;
    ajulong *parray = NULL;
    ajulong blockno = 0L;
    
    ajuint nkeys    = 0;
    ajuint nentries = 0;
    ajuint i;

    ajuint dirtysave = 0;
    
    AjBool found = ajFalse;
    const AjPStr key  = NULL;

    unsigned char *buf = NULL;
    AjPBtMem arrays = NULL;
    
    /* ajDebug("In btreeRemoveHybEntryOne\n"); */

    page = ajBtreeCacheRead(cache,pagepos);
    buf = page->buf;
    dirtysave = page->dirty;
    page->dirty = BT_LOCK;
    page->lockfor = 1731;

    GBT_NKEYS(buf,&nkeys);

    if(!nkeys)
	return ajFalse;


    arrays = btreeAllocPriArray(cache);
    karray = arrays->karray;
    parray = arrays->parray;

    btreeGetKeys(cache,buf,&karray,&parray);
    
    key = hyb->key1;

    i=0;

    while(i!=nkeys && MAJSTRCMPS(key,karray[i])>=0)
	++i;

    blockno = parray[i];
    
    bucket = btreeReadBucket(cache,blockno);

    nentries = bucket->Nentries;
    found = ajFalse;

    for(i=0;i<nentries;++i)
	if(!MAJSTRCMPS(key,bucket->Ids[i]->id))
	{
	    found = ajTrue;
	    break;
	}
    

    if(found)
    {
	/* Perform the deletion */
	if(nentries == 1)
	{
	    bucket->Nentries = 0;
	    ajBtreeIdDel(&bucket->Ids[0]);
	}
	else
	{
	    ajBtreeIdDel(&bucket->Ids[i]);
	    bucket->Ids[i] = bucket->Ids[nentries-1];
	    --bucket->Nentries;
	}

        
	btreeWriteBucket(cache,bucket,blockno);

	btreeAdjustHybBucketsOne(cache,page);

	page->dirty = BT_DIRTY;
    }
    else
	page->dirty = dirtysave;

    btreeBucketDel(&bucket);

    btreeDeallocPriArray(cache,arrays);
    
    if(!found)
	return ajFalse;

    return ajTrue;
}




/* @funcstatic btreeAdjustHybBucketsOne **************************************
**
** Re-order leaf buckets
** Can be called whatever the state of a leaf.
**
** Deletion software
**
** @param [u] cache [AjPBtcache] cache
** @param [u] leaf [AjPBtpage] leaf page
**
** @return [void]
** @@
******************************************************************************/

static void btreeAdjustHybBucketsOne(AjPBtcache cache, AjPBtpage leaf)
{
    ajuint nkeys = 0;
    unsigned char *lbuf = NULL;
    AjPBucket *buckets  = NULL;

    AjPBtMem arrays = NULL;    
    AjPStr *keys        = NULL;
    ajulong *ptrs        = NULL;
    ajulong *overflows   = NULL;
    
    ajuint i = 0;
    ajuint j = 0;
    
    ajuint order;
    ajuint bentries      = 0;
    ajuint totalkeys     = 0;
    ajuint nperbucket    = 0;
    ajuint maxnperbucket = 0;
    ajuint count         = 0;
    ajuint totkeylen     = 0;
    ajuint keylimit      = 0;
    ajuint bucketn       = 0;
    ajuint bucketlimit   = 0;
    ajuint nodetype      = 0;
    ajuint nids          = 0;
    ajuint totnids       = 0;
    
    AjPList idlist    = NULL;
    ajuint  dirtysave = 0;
    AjPBtId bid       = NULL;
    AjPBucket cbucket = NULL;
    AjPBtId cid       = NULL;

    ajuint v = 0;
    
    /* ajDebug("In btreeAdjustHybBucketsOne\n"); */

    dirtysave = leaf->dirty;

    leaf->dirty = BT_LOCK;
    leaf->lockfor = 1741;
    lbuf = leaf->buf;

    GBT_NKEYS(lbuf,&nkeys);

    if(!nkeys)
    {
	leaf->dirty = dirtysave;
	return;
    }


    GBT_NODETYPE(lbuf,&nodetype);

    order = cache->order;
    nperbucket = cache->nperbucket;
    

    /* Read keys/ptrs */

    arrays = btreeAllocPriArray(cache);
    keys = arrays->karray;
    ptrs = arrays->parray;
    overflows = arrays->overflows;

    btreeGetKeys(cache,lbuf,&keys,&ptrs);


    for(i=0;i<nkeys;++i)
	totalkeys += btreeNumInBucket(cache,ptrs[i]);
    totalkeys += btreeNumInBucket(cache,ptrs[i]);


    /* Set the number of entries per bucket to approximately half full */
    maxnperbucket = nperbucket >> 1;

    if(!maxnperbucket)
	++maxnperbucket;

    if(!leaf->pagepos)
	maxnperbucket = nperbucket;

    /* Work out the number of new buckets needed */
    bucketn = (totalkeys / maxnperbucket);
    if(totalkeys % maxnperbucket)
	++bucketn;

    if(bucketn == 1)
	++bucketn;

    while(bucketn > order)
    {
        ++maxnperbucket;
        bucketn = (totalkeys / maxnperbucket);

        if(totalkeys % maxnperbucket)
            ++bucketn;
    }

    /* Read buckets */
    AJCNEW0(buckets,nkeys+1);
    keylimit = nkeys + 1;
    
    for(i=0;i<keylimit;++i)
	buckets[i] = btreeReadBucket(cache,ptrs[i]);


    /* Read IDs from all buckets and push to list and sort (increasing id) */
    idlist  = ajListNew();

    for(i=0;i<keylimit;++i)
    {
	overflows[i] = buckets[i]->Overflow;
	bentries = buckets[i]->Nentries;

	for(j=0;j<bentries;++j)
	    ajListPush(idlist,(void *)buckets[i]->Ids[j]);
	
	AJFREE(buckets[i]->keylen);
	AJFREE(buckets[i]->Ids);
	AJFREE(buckets[i]);
    }

    ajListSort(idlist,btreeIdCompare);
    AJFREE(buckets);

    cbucket = btreeBucketNew(maxnperbucket);
    bucketlimit = bucketn - 1;

    totnids = 0;
    nids = ajListGetLength(idlist);


    if(!totalkeys)
    {
	v = totalkeys;
	SBT_NKEYS(lbuf,v);

        btreeDeallocPriArray(cache,arrays);

	ajListFree(&idlist);
	leaf->dirty = BT_DIRTY;

	return;
    }
    
    if(nids <= maxnperbucket)
    {
	cbucket->Overflow = overflows[1];
	cbucket->Nentries = 0;
	ajListPeek(idlist,(void **)&bid);
	ajStrAssignS(&keys[0],bid->id);

	count = 0;

	while(count!=maxnperbucket && totnids != nids)
	{
	    ajListPop(idlist,(void **)&bid);
	    
	    cid = cbucket->Ids[count];
	    ajStrAssignS(&cid->id,bid->id);
	    cid->dbno = bid->dbno;
	    cid->dups = bid->dups;
	    cid->offset = bid->offset;
	    cid->refoffset = bid->refoffset;
	    
	    cbucket->keylen[count] = BT_BUCKIDLEN(bid->id);
	    ++cbucket->Nentries;
	    ++count;
	    ++totnids;
	    ajBtreeIdDel(&bid);
	}


	totkeylen += ajStrGetLen(keys[0]);

	if(!ptrs[1])
	    ptrs[1] = cache->totsize;

	btreeWriteBucket(cache,cbucket,ptrs[1]);

	cbucket->Overflow = overflows[0];
	cbucket->Nentries = 0;

	if(!ptrs[0])
	    ptrs[0] = cache->totsize;

	btreeWriteBucket(cache,cbucket,ptrs[0]);
    }
    else
    {
	for(i=0;i<bucketlimit;++i)
	{
	    cbucket->Overflow = overflows[i];
	    cbucket->Nentries = 0;

	    count = 0;

	    while(count!=maxnperbucket && totnids != nids)
	    {
		ajListPop(idlist,(void **)&bid);

		cid = cbucket->Ids[count];
		ajStrAssignS(&cid->id,bid->id);
		cid->dbno = bid->dbno;
		cid->dups = bid->dups;
		cid->offset = bid->offset;
		cid->refoffset = bid->refoffset;
		
		cbucket->keylen[count] = BT_BUCKIDLEN(bid->id);
		++cbucket->Nentries;
		++count;
		ajBtreeIdDel(&bid);
	    }


	    ajListPeek(idlist,(void **)&bid);
	    ajStrAssignS(&keys[i],bid->id);


	    totkeylen += ajStrGetLen(bid->id);

	    if(!ptrs[i])
		ptrs[i] = cache->totsize;
	    btreeWriteBucket(cache,cbucket,ptrs[i]);
	}
	
	
	/* Deal with greater-than bucket */
	
	cbucket->Overflow = overflows[i];
	cbucket->Nentries = 0;
	
	
	
	count = 0;

	while(ajListPop(idlist,(void **)&bid))
	{
	    cid = cbucket->Ids[count];
	    ajStrAssignS(&cid->id,bid->id);
	    cid->dbno = bid->dbno;
	    cid->dups = bid->dups;
	    cid->offset = bid->offset;
	    cid->refoffset = bid->refoffset;
	    
	    ++cbucket->Nentries;
	    ++count;
	    ajBtreeIdDel(&bid);
	}
	
	
	if(!ptrs[i])
	    ptrs[i] = cache->totsize;
	
	btreeWriteBucket(cache,cbucket,ptrs[i]);
    }
    

    cbucket->Nentries = maxnperbucket;
    btreeBucketDel(&cbucket);

    /* Now write out a modified leaf with new keys/ptrs */

    nkeys = bucketn - 1;

    btreeWriteNode(cache,leaf,keys,ptrs,nkeys);

    leaf->dirty = dirtysave;
    if(nodetype == BT_ROOT)
    {
	leaf->dirty = BT_LOCK;
        leaf->lockfor = 1742;
    }

    btreeDeallocPriArray(cache,arrays);

    btreeBucketDel(&cbucket);
    ajListFree(&idlist);

    return;
}




/* @funcstatic btreeRebalanceHybOne ******************************************
**
** Rebalance Hybrid level 1 tree after deletion
**
** Deletion software
**
** @param [u] cache [AjPBtcache] cache
** @param [r] thisNode [ajulong] Node to rebalance
** @param [r] leftNode [ajulong] left node
** @param [r] rightNode [ajulong] right node
** @param [r] lAnchor [ajulong] left anchor
** @param [r] rAnchor [ajulong] right anchor
**
** @return [ajulong] page number or BTNO_NODE
** @@
******************************************************************************/

static ajulong btreeRebalanceHybOne(AjPBtcache cache, ajulong thisNode,
                                    ajulong leftNode, ajulong rightNode,
                                    ajulong lAnchor, ajulong rAnchor)
{
    unsigned char *lbuf = NULL;
    unsigned char *rbuf = NULL;
    unsigned char *tbuf = NULL;

    ajulong anchorNode   = 0L;
    ajulong balanceNode  = 0L;
    ajulong mergeNode    = 0L;
    ajulong done         = 0L;
    ajulong parent       = 0L;
    
    AjPBtpage lpage = NULL;
    AjPBtpage rpage = NULL;
    AjPBtpage tpage = NULL;
    
    ajuint lnkeys  = 0;
    ajuint rnkeys  = 0;
    ajuint size    = 0;
    ajuint order   = 0;
    ajuint minsize = 0;

    AjBool leftok  = ajFalse;
    AjBool rightok = ajFalse;
    
    
    /* ajDebug("In btreeRebalanceHybOne\n"); */

    if(leftNode!=BTNO_NODE && lAnchor!=BTNO_NODE)
	leftok = ajTrue;

    if(rightNode!=BTNO_NODE && rAnchor!=BTNO_NODE)
	rightok = ajTrue;

    if(!leftok && !rightok)
	return BTNO_NODE;
    

    if(leftok)
    {
	lpage = ajBtreeCacheRead(cache,leftNode);
	lbuf  = lpage->buf;
	GBT_NKEYS(lbuf,&lnkeys);
    }
    

    if(rightok)
    {
	rpage = ajBtreeCacheRead(cache,rightNode);
	rbuf  = rpage->buf;
	GBT_NKEYS(rbuf,&rnkeys);
    }
    


    if(leftok && rightok)
    {
	size = (lnkeys >= rnkeys) ? lnkeys : rnkeys;
	balanceNode = (lnkeys >= rnkeys) ? leftNode : rightNode;
    }
    else if(leftok)
    {
	size = lnkeys;
	balanceNode = leftNode;
    }
    else
    {
	size = rnkeys;
	balanceNode = rightNode;
    }

    
    order = cache->order;
    minsize = (order-1) / 2;

    if((order-1)%2)
	++minsize;

    if(size >= minsize)
    {
	if(leftok && rightok)
	    anchorNode = (lnkeys >= rnkeys) ? lAnchor : rAnchor;
	else if(leftok)
	    anchorNode = lAnchor;
	else
	    anchorNode = rAnchor;

	done = btreeShiftHybOne(cache,thisNode,balanceNode,anchorNode);
    }
	    
    else
    {
	tpage = ajBtreeCacheRead(cache,thisNode);
	tbuf  = tpage->buf;
	GBT_PREV(tbuf,&parent);

	if(leftok && rightok)
	{
	    anchorNode = (parent == lAnchor) ? lAnchor : rAnchor;
	    mergeNode  = (anchorNode == lAnchor) ? leftNode : rightNode;
	}
	else if(leftok)
	{
	    anchorNode = lAnchor;
	    mergeNode  = leftNode;
	}
	else
	{
	    anchorNode = rAnchor;
	    mergeNode  = rightNode;
	}

	done = btreeMergeHybOne(cache,thisNode,mergeNode,anchorNode);
    }

    return done;
}




/* @funcstatic btreeShiftHybOne ***********************************************
**
** Shift spare entries from one hybrid tree level 1 node to another.
**
** Deletion software.
**
** @param [u] cache [AjPBtcache] cache
** @param [r] thisNode [ajulong] master node
** @param [r] balanceNode [ajulong] balance node
** @param [r] anchorNode [ajulong] anchor node
**
** @return [ajulong] page number or BTNO_NODE
** @@
******************************************************************************/

static ajulong btreeShiftHybOne(AjPBtcache cache, ajulong thisNode,
                                ajulong balanceNode, ajulong anchorNode)
{
    unsigned char *tbuf = NULL;
    unsigned char *abuf = NULL;
    unsigned char *bbuf = NULL;
    unsigned char *buf  = NULL;
    
    AjPStr *kTarray = NULL;
    AjPStr *kAarray = NULL;
    AjPStr *kBarray = NULL;
    ajulong *pTarray = NULL;
    ajulong *pAarray = NULL;
    ajulong *pBarray = NULL;
    
    ajuint  nAkeys = 0;
    ajuint  nBkeys = 0;
    ajuint  nTkeys = 0;
    ajuint  i;
    ajint ii;
    
    AjPBtpage pageA = NULL;
    AjPBtpage pageB = NULL;
    AjPBtpage pageT = NULL;
    AjPBtpage page  = NULL;
    
    AjPBtpage leftpage = NULL;

    ajuint anchorPos   = 0;
    ajulong prev        = 0L;
    ajuint  nodetype    = 0;

    ajulong lv = 0L;
    
    AjPBtMem arraysA = NULL;    
    AjPBtMem arraysB = NULL;    
    AjPBtMem arraysT = NULL;    

    /* ajDebug("In btreeShiftHybOne\n"); */


    arraysA = btreeAllocPriArray(cache);
    kAarray = arraysA->karray;
    pAarray = arraysA->parray;

    arraysB = btreeAllocPriArray(cache);
    kBarray = arraysB->karray;
    pBarray = arraysB->parray;

    arraysT = btreeAllocPriArray(cache);
    kTarray = arraysT->karray;
    pTarray = arraysT->parray;


    pageA = ajBtreeCacheRead(cache,anchorNode);
    pageA->dirty = BT_LOCK;
    pageA->lockfor = 1751;
    abuf = pageA->buf;
    pageB = ajBtreeCacheRead(cache,balanceNode);
    pageB->dirty = BT_LOCK;
    pageB->lockfor = 1752;
    bbuf = pageB->buf;
    pageT = ajBtreeCacheRead(cache,thisNode);
    pageT->dirty = BT_LOCK;
    pageT->lockfor = 1753;
    tbuf = pageT->buf;

    GBT_NKEYS(abuf,&nAkeys);
    GBT_NKEYS(bbuf,&nBkeys);
    GBT_NKEYS(tbuf,&nTkeys);

    btreeGetKeys(cache,abuf,&kAarray,&pAarray);
    btreeGetKeys(cache,bbuf,&kBarray,&pBarray);
    btreeGetKeys(cache,tbuf,&kTarray,&pTarray);
    
    if(MAJSTRCMPS(kTarray[nTkeys-1],kBarray[nBkeys-1])<0)
	leftpage = pageT;
    else
	leftpage = pageB;


    if(leftpage == pageT)
    {
	/* Find anchor key position */
	i=0;

	while(i!=nAkeys && MAJSTRCMPS(kTarray[nTkeys-1],kAarray[i])>=0)
	    ++i;

	anchorPos = i;

	/* Move down anchor key to thisNode */
	ajStrAssignS(&kTarray[nTkeys],kAarray[anchorPos]);
	++nTkeys;

	/* Shift extra */
	while(nTkeys < nBkeys)
	{
	    ajStrAssignS(&kTarray[nTkeys],kBarray[0]);
	    pTarray[nTkeys] = pBarray[0];
	    ++nTkeys;
	    --nBkeys;

	    for(i=0;i<nBkeys;++i)
	    {
		ajStrAssignS(&kBarray[i],kBarray[i+1]);
		pBarray[i] = pBarray[i+1];
	    }
	    pBarray[i] = pBarray[i+1];
	}

	/* Adjust anchor key */
	ajStrAssignS(&kAarray[anchorPos],kTarray[nTkeys-1]);
	--nTkeys;
    }
    else	/* thisNode on the right */
    {
	/* Find anchor key position */
	i=0;

	while(i!=nAkeys && MAJSTRCMPS(kBarray[nBkeys-1],kAarray[i])>=0)
	    ++i;

	anchorPos = i;

	/* Move down anchor key to thisNode */
	pTarray[nTkeys+1] = pTarray[nTkeys];

	for(ii=nTkeys-1;ii>-1;--ii)
	{
	    ajStrAssignS(&kTarray[ii+1],kTarray[ii]);
	    pTarray[ii+1] = pTarray[ii];
	}

	ajStrAssignS(&kTarray[0],kAarray[anchorPos]);
	++nTkeys;

	/* Shift extra */
	while(nTkeys < nBkeys)
	{
	    pTarray[nTkeys+1] = pTarray[nTkeys];

	    for(ii=nTkeys-1;ii>-1;--ii)
	    {
		ajStrAssignS(&kTarray[ii+1],kTarray[ii]);
		pTarray[ii+1] = pTarray[ii];
	    }

	    ajStrAssignS(&kTarray[0],kBarray[nBkeys-1]);
	    pTarray[1] = pBarray[nBkeys];
	    ++nTkeys;
	    --nBkeys;
	}


	/* Adjust anchor key */
	ajStrAssignS(&kAarray[anchorPos],kTarray[0]);
	--nTkeys;

	for(i=0;i<nTkeys;++i)
	{
	    ajStrAssignS(&kTarray[i],kTarray[i+1]);
	    pTarray[i] = pTarray[i+1];
	}

	pTarray[i] = pTarray[i+1];
    }
    

    /* Adjust PREV pointers for thisNode */
    prev = pageT->pagepos;

    for(i=0;i<nTkeys+1;++i)
    {
	page = ajBtreeCacheRead(cache,pTarray[i]);
	buf = page->buf;
	GBT_NODETYPE(buf,&nodetype);

	if(nodetype != BT_BUCKET)
	{
	    lv = prev;
	    SBT_PREV(buf,lv);
	    page->dirty = BT_DIRTY;
	}
    }

    btreeWriteNode(cache,pageA,kAarray,pAarray,nAkeys);
    btreeWriteNode(cache,pageB,kBarray,pBarray,nBkeys);
    btreeWriteNode(cache,pageT,kTarray,pTarray,nTkeys);

    if(!anchorNode)
    {
	pageA->dirty = BT_LOCK;
        pageA->lockfor = 1754;
    }

    btreeDeallocPriArray(cache,arraysA);
    btreeDeallocPriArray(cache,arraysB);
    btreeDeallocPriArray(cache,arraysT);

    return BTNO_NODE;
}




/* @funcstatic btreeMergeHybOne ***********************************************
**
** Merge two nodes.
**
** Deletion software
**
** @param [u] cache [AjPBtcache] cache
** @param [r] thisNode [ajulong] master node
** @param [r] mergeNode [ajulong] merge node
** @param [r] anchorNode [ajulong] anchor node
**
** @return [ajulong] page number or BTNO_NODE
** @@
******************************************************************************/

static ajulong btreeMergeHybOne(AjPBtcache cache, ajulong thisNode,
                                ajulong mergeNode, ajulong anchorNode)
{
    unsigned char *tbuf = NULL;
    unsigned char *abuf = NULL;
    unsigned char *nbuf = NULL;
    unsigned char *buf  = NULL;
    
    AjPStr *kTarray = NULL;
    AjPStr *kAarray = NULL;
    AjPStr *kNarray = NULL;
    ajulong *pTarray = NULL;
    ajulong *pAarray = NULL;
    ajulong *pNarray = NULL;

    ajulong thisprev  = 0L;
    ajulong mergeprev = 0L;
    
    
    ajuint  nAkeys = 0;
    ajuint  nNkeys = 0;
    ajuint  nTkeys = 0;
    ajuint  count  = 0;
    ajuint  i;
    ajint ii;

    ajuint   nodetype = 0;
    
    ajuint saveA = 0;
    ajuint saveN = 0;
    ajuint saveT = 0;
    
    AjPBtpage pageA = NULL;
    AjPBtpage pageN = NULL;
    AjPBtpage pageT = NULL;
    AjPBtpage page  = NULL;
    
    AjPBtpage leftpage = NULL;

    ajuint anchorPos = 0;
    ajulong prev      = 0L;

    ajulong lv = 0L;

    AjPBtMem arraysA = NULL;    
    AjPBtMem arraysN = NULL;    
    AjPBtMem arraysT = NULL;    

    AjBool collapse = ajFalse;
    
    /* ajDebug("In btreeMergeHybOne\n"); */

    pageA = ajBtreeCacheRead(cache,anchorNode);
    saveA = pageA->dirty;
    pageA->dirty = BT_LOCK;
    pageA->lockfor = 1761;
    abuf = pageA->buf;
    pageN = ajBtreeCacheRead(cache,mergeNode);
    saveN = pageN->dirty;
    pageN->dirty = BT_LOCK;
    pageN->lockfor = 1762;
    nbuf = pageN->buf;
    pageT = ajBtreeCacheRead(cache,thisNode);
    saveT = pageT->dirty;
    pageT->dirty = BT_LOCK;
    pageT->lockfor = 1763;
    tbuf = pageT->buf;

    GBT_PREV(tbuf,&thisprev);
    GBT_PREV(nbuf,&mergeprev);

    GBT_NKEYS(abuf,&nAkeys);
    GBT_NKEYS(nbuf,&nNkeys);
    GBT_NKEYS(tbuf,&nTkeys);

    GBT_NODETYPE(nbuf,&nodetype);


    if(nAkeys == 1)
    {
	if(!anchorNode && !thisprev && !mergeprev)
	    collapse = ajTrue;
	else
	{
	    pageA->dirty = saveA;
	    pageN->dirty = saveN;
	    pageT->dirty = saveT;
	    return thisNode;
	}
    }

    arraysA = btreeAllocPriArray(cache);
    kAarray = arraysA->karray;
    pAarray = arraysA->parray;

    arraysN = btreeAllocPriArray(cache);
    kNarray = arraysN->karray;
    pNarray = arraysN->parray;

    arraysT = btreeAllocPriArray(cache);
    kTarray = arraysT->karray;
    pTarray = arraysT->parray;

    btreeGetKeys(cache,abuf,&kAarray,&pAarray);
    btreeGetKeys(cache,nbuf,&kNarray,&pNarray);
    btreeGetKeys(cache,tbuf,&kTarray,&pTarray);

    if(MAJSTRCMPS(kTarray[nTkeys-1],kNarray[nNkeys-1])<0)
	leftpage = pageT;
    else
	leftpage = pageN;


    if(leftpage == pageT)
    {
	/* Find anchor key position */
	i=0;

	while(i!=nAkeys && MAJSTRCMPS(kTarray[nTkeys-1],kAarray[i])>=0)
	    ++i;

	anchorPos = i;

	/* Move down anchor key to neighbour Node */
	pNarray[nNkeys+1] = pNarray[nNkeys];

	for(ii=nNkeys-1;ii>-1;--ii)
	{
	    ajStrAssignS(&kNarray[ii+1],kNarray[ii]);
	    pNarray[ii+1] = pNarray[ii];
	}
	ajStrAssignS(&kNarray[0],kAarray[anchorPos]);
	++nNkeys;


	/* Adjust anchor node keys/ptrs */
	++anchorPos;

	if(anchorPos==nAkeys)
	    pAarray[nAkeys-1] = pAarray[nAkeys];
	else
	{
	    for(i=anchorPos;i<nAkeys;++i)
	    {
		ajStrAssignS(&kAarray[i-1],kAarray[i]);
		pAarray[i-1] = pAarray[i];
	    }

	    pAarray[i-1] = pAarray[i];
	}
	--nAkeys;
	

	/* Merge this to neighbour */

	while(nTkeys)
	{
	    pNarray[nNkeys+1] = pNarray[nNkeys];

	    for(ii=nNkeys-1;ii>-1;--ii)
	    {
		ajStrAssignS(&kNarray[ii+1],kNarray[ii]);
		pNarray[ii+1] = pNarray[ii];
	    }

	    ajStrAssignS(&kNarray[0],kTarray[nTkeys-1]);
	    pNarray[1] = pTarray[nTkeys];
	    pNarray[0] = pTarray[nTkeys-1];
	    --nTkeys;
	    ++nNkeys;
	}

	/* At this point the 'this' node could be added to a free list */
    }
    else
    {
	/* Find anchor key position */
	i=0;

	while(i!=nAkeys && MAJSTRCMPS(kNarray[nNkeys-1],kAarray[i])>=0)
	    ++i;

	anchorPos = i;

	/* Move down anchor key to neighbourNode */
	ajStrAssignS(&kNarray[nNkeys],kAarray[anchorPos]);
	++nNkeys;

	/* Adjust anchor node keys/ptrs */
	++anchorPos;

	if(anchorPos!=nAkeys)
	    for(i=anchorPos;i<nAkeys;++i)
	    {
		ajStrAssignS(&kAarray[i-1],kAarray[i]);
		pAarray[i] = pAarray[i+1];
	    }

	--nAkeys;

	/* merge extra */
	count = 0;

	while(nTkeys)
	{
	    ajStrAssignS(&kNarray[nNkeys],kTarray[count]);
	    pNarray[nNkeys] = pTarray[count];
	    ++nNkeys;
	    ++count;
	    --nTkeys;
	    pNarray[nNkeys] = pTarray[count];
	
	}

	/* At this point the 'this' node could be added to a free list */
    }
    
    
    /* Adjust PREV pointers for neighbour Node */
    prev = pageN->pagepos;

    for(i=0;i<nNkeys+1;++i)
    {
	page = ajBtreeCacheRead(cache,pNarray[i]);
	buf = page->buf;
	GBT_NODETYPE(buf,&nodetype);

	if(nodetype != BT_BUCKET)
	{
	    lv = prev;
	    SBT_PREV(buf,lv);
	    page->dirty = BT_DIRTY;
	}
    }

    pageT->dirty = BT_CLEAN;
    btreeWriteNode(cache,pageA,kAarray,pAarray,nAkeys);
    btreeWriteNode(cache,pageN,kNarray,pNarray,nNkeys);

    if(!anchorNode)
    {
	pageA->dirty = BT_LOCK;
        pageA->lockfor = 1764;
    }

    btreeDeallocPriArray(cache,arraysA);
    btreeDeallocPriArray(cache,arraysN);
    btreeDeallocPriArray(cache,arraysT);
    
    if(collapse)
	btreeCollapseRootHybOne(cache,mergeNode);

    return thisNode;
}




/* @funcstatic btreeCollapseRootHybOne ****************************************
**
** Collapse root page for hybrid level 1 tree.
**
** Deletion software.
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number to make new root
**
** @return [ajulong] page number or BTNO_NODE
** @@
******************************************************************************/

static ajulong btreeCollapseRootHybOne(AjPBtcache cache, ajulong pagepos)
{
    unsigned char *buf  = NULL;
    unsigned char *lbuf = NULL;

    AjPStr *karray = NULL;
    ajulong *parray = NULL;

    AjPBtpage rootpage = NULL;
    AjPBtpage page     = NULL;
    
    ajuint nodetype = 0;
    ajuint nkeys    = 0;
    ajuint i;

    ajulong prev = 0L;
    AjPBtMem arrays = NULL;
    
    /* ajDebug("In btreeCollapseRootHybOne\n"); */
    
    if(!cache->level)
	return BTNO_NODE;

    rootpage = btreeCacheLocate(cache,0L);
    buf = rootpage->buf;
    page = ajBtreeCacheRead(cache,pagepos);


    arrays = btreeAllocPriArray(cache);
    karray = arrays->karray;
    parray = arrays->parray;

    /*
    ** Swap pagepos values to make root the child and child the root
    ** Update node types and mark the original root as a clean page
    */

    /* At this point page->pagepos could be added to a free list */

    rootpage->pagepos = page->pagepos;
    rootpage->dirty = BT_CLEAN;
    nodetype = BT_INTERNAL;
    SBT_NODETYPE(buf,nodetype);

    page->pagepos = 0;
    page->dirty = BT_LOCK;
    page->lockfor = 1771;
    buf = page->buf;
    nodetype = BT_ROOT;
    SBT_NODETYPE(buf,nodetype);
    
    --cache->level;

    if(cache->level)
    {
	/*
	 ** Update the PREV pointers of the new root's children
	 */
	GBT_NKEYS(buf,&nkeys);
	btreeGetKeys(cache,buf,&karray,&parray);
	for(i=0;i<nkeys+1;++i)
	{
	    page = ajBtreeCacheRead(cache,parray[i]);
	    lbuf = page->buf;
	    SBT_PREV(lbuf,prev);
	    page->dirty = BT_DIRTY;
	}
    }

    btreeDeallocPriArray(cache,arrays);
    
    return 0L;
}




/* @funcstatic btreeDeleteHybIdTwo *********************************************
**
** Entry point for secondary hybrid tree ID deletion.
** Assumes cache->secrootblock has been initialised.
**
** Deletion software
**
** @param [u] cache [AjPBtcache] cache
** @param [r] hyb [const AjPBtHybrid] hybrid object
** @param [u] did [AjPBtId] ID object in primary tree
**
** @return [AjBool] True if found and deleted
** @@
******************************************************************************/

static AjBool btreeDeleteHybIdTwo(AjPBtcache cache, const AjPBtHybrid hyb,
                                  AjPBtId did)
{
    AjPBtpage rpage = NULL;
    AjPBtpage page  = NULL;

    ajulong sval = 0L;
    ajulong key  = 0L;
    ajulong *karray = NULL;
    ajulong *parray = NULL;
    ajulong blockno = 0L;
    
    unsigned char *rbuf = NULL;
    unsigned char *buf  = NULL;

    ajuint nkeys    = 0;
    ajuint nentries = 0;
    
    ajuint i;

    AjBool found = ajFalse;
    
    AjPBtNumId num      = NULL;
    AjPNumBucket bucket = NULL;
    
    AjPBtMem array = NULL;
    
    rpage = ajBtreeCacheWriteNode(cache,cache->secrootblock);

    rpage->dirty = BT_LOCK;
    rpage->lockfor = 1781;
    rbuf = rpage->buf;
    GBT_RIGHT(rbuf,&sval);
    cache->slevel = (ajuint) sval;

    key  = hyb->offset;

    page = ajBtreeNumFindInsert(cache,key);
    buf  = page->buf;

    GBT_NKEYS(buf,&nkeys);

    array  = btreeAllocSecArray(cache);

    karray = array->overflows;
    parray = array->parray;
    
    if(!nkeys)
    {
        ajWarn("btreeDeleteHybIdTwo: No keys in findinsert node");
        btreeDeallocSecArray(cache,array);
        rpage->dirty = BT_CLEAN;

        return ajFalse;
    }

    btreeGetNumKeys(cache,buf,&karray,&parray);

    i = 0;

    while(i != nkeys && key >= karray[i])
        ++i;

    blockno = parray[i];

    bucket = btreeReadNumBucket(cache,blockno);
    nentries = bucket->Nentries;

    found = ajFalse;

    for(i=0; i < nentries; ++i)
    {
        num = bucket->NumId[i];
        
        if(num->offset == key)
        {
            found = ajTrue;
            break;
        }
    }

    if(!found)
    {
        ajWarn("btreeDeleteHybIdTwo: Numeric key not in bucket");
        btreeDeallocSecArray(cache,array);
        rpage->dirty = BT_CLEAN;

        return ajFalse;
    }

    btreeFindHybBalanceTwo(cache,cache->secrootblock,BTNO_NODE,BTNO_NODE,
                           BTNO_NODE,BTNO_NODE,key);

    if(!cache->deleted)
    {
        ajWarn("btreeDeleteHybIdTwo: entry %Lu not deleted",key);
        rpage->dirty = BT_CLEAN;
        btreeDeallocSecArray(cache,array);

        return ajFalse;
    }

    --did->dups;

    if(did->dups != 1)
    {
        rpage->dirty = BT_DIRTY;
        btreeDeallocSecArray(cache,array);

        return ajTrue;
    }
    
    /*
    ** Need to find remaining 2ry key here. Should be in root node.
    */
    buf = rpage->buf;
    btreeGetNumKeys(cache,buf,&karray,&parray);
    bucket = btreeReadNumBucket(cache,parray[0]);

    if(!bucket->Nentries)
        bucket = btreeReadNumBucket(cache,parray[1]);

    if(bucket->Nentries != 1)
        ajFatal("Expected only one remaining entry in btreeDeleteHybIdTwo");
    
    num = bucket->NumId[0];

    did->refoffset = num->refoffset;
    did->offset    = num->offset;
    did->dups      = 0;
    rpage->dirty = BT_CLEAN; /* Doesn't matter as page is now defunct */

    /*
    ** At this point pages parray[0], parray[1] and cache->secrootblock
    ** could be reused.
    **/
    
    
    btreeDeallocSecArray(cache,array);

    return ajTrue;
}




/* @funcstatic btreeFindHybBalanceTwo ******************************************
**
** Master routine for entry deletion from level 2 hybrid tree.
**
** Deletion software.
**
** @param [u] cache [AjPBtcache] cache
** @param [r] thisNode [ajulong] Current node
** @param [r] leftNode [ajulong] Node to left
** @param [r] rightNode [ajulong] Node to right
** @param [r] lAnchor [ajulong] Left anchor
** @param [r] rAnchor [ajulong] Right anchor
** @param [r] key [ajulong] key
**
** @return [ajulong] page number or BTNO_NODE
** @@
******************************************************************************/

static ajulong btreeFindHybBalanceTwo(AjPBtcache cache, ajulong thisNode,
                                      ajulong leftNode, ajulong rightNode,
                                      ajulong lAnchor, ajulong rAnchor,
                                      ajulong key)
{
    unsigned char *buf  = NULL;
    unsigned char *buf1 = NULL;
    
    ajulong nextNode   = BTNO_NODE;
    ajulong nextLeft   = BTNO_NODE;
    ajulong nextRight  = BTNO_NODE;
    ajulong nextAncL   = BTNO_NODE;
    ajulong nextAncR   = BTNO_NODE;
    ajulong done       = 0L;
    
    ajuint  nkeys      = 0;
    ajuint  order      = 0;
    ajuint  minkeys    = 0;
    ajuint  i;
    ajuint   nodetype   = 0;

    ajuint n1keys      = 0;
    
    AjPBtpage page  = NULL;
    AjPBtpage page1 = NULL;

    ajulong balanceNode = 0L;
    ajulong blockno     = 0L;
    ajulong ptrSave     = 0L;

    AjPBtMem arrays  = NULL;
    AjPBtMem arrays1 = NULL;
    ajulong *karray  = NULL;
    ajulong *parray  = NULL;
    ajulong *k1array = NULL;
    ajulong *p1array = NULL;

    AjBool existed = ajFalse;
    
    /* ajDebug("In btreeFindHybBalanceTwo\n"); */

    if(thisNode != cache->secrootblock)
	page = ajBtreeCacheRead(cache,thisNode);
    else
    {   /* It's the root node of the primary hyb tree */
	page = btreeCacheLocate(cache,thisNode);
	page->dirty = BT_LOCK;
        page->lockfor = 1791;
    }

    cache->deleted = ajFalse;

    buf = page->buf;
    GBT_NKEYS(buf,&nkeys);
    
    order = cache->sorder;
    /* order-1 is the number of keys in the node */
    minkeys = (order-1) / 2;

    if((order-1)%2)
	++minkeys;

    /*
    ** If thisNode contains >= minkeys then it is not a candidate
    ** for balancing
    */
    if(nkeys >= minkeys)
	balanceNode = BTNO_BALANCE;
    else
	balanceNode = page->pagepos;

    arrays  = btreeAllocSecArray(cache);
    arrays1 = btreeAllocSecArray(cache);

    karray = arrays->overflows;
    parray = arrays->parray;

    k1array = arrays1->overflows;
    p1array = arrays1->parray;

    btreeGetNumKeys(cache,buf,&karray,&parray);

    i=0;

    while(i!=nkeys && key >= karray[i])
	++i;

    blockno = parray[i];

    nextNode = blockno;
    ptrSave = i;

    GBT_NODETYPE(buf,&nodetype);

    if(!(nodetype == BT_LEAF) && !(nodetype == BT_ROOT && !cache->slevel))
    {
	if(nextNode == parray[0])
	{
	    if(leftNode != BTNO_NODE)
	    {
		page1 = ajBtreeCacheRead(cache,leftNode);
		buf1 = page1->buf;
		GBT_NKEYS(buf1,&n1keys);
		btreeGetNumKeys(cache,buf1,&k1array,&p1array);
		nextLeft = p1array[n1keys];
	    }
	    else
		nextLeft = BTNO_NODE;
	    
	    if(thisNode == cache->secrootblock)
		nextAncL = thisNode;
	    else
		nextAncL = lAnchor;
	}
	else
	{
	    nextLeft = parray[ptrSave-1];
	    nextAncL = thisNode;
	}

	if(nextNode == parray[nkeys])
	{
	    if(rightNode != BTNO_NODE)
	    {
		page1 = ajBtreeCacheRead(cache,rightNode);
		buf1 = page1->buf;
		GBT_NKEYS(buf1,&n1keys);
		btreeGetNumKeys(cache,buf1,&k1array,&p1array);
		nextRight = p1array[0];
	    }
	    else
		nextRight = BTNO_NODE;

	    if(thisNode == cache->secrootblock)
		nextAncR = thisNode;
	    else
		nextAncR = rAnchor;
	}
	else
	{
	    nextRight = parray[ptrSave+1];
	    nextAncR  = thisNode;
	}



	/* Check to see whether key exists in an internal node */
	if(nodetype != BT_LEAF && cache->slevel)
	{
	    i=0;

	    while(i!=nkeys && key > karray[i])
		++i;

	    if(i!=nkeys)
	    {
		btreeFindHybMinTwo(cache,parray[i+1],key);
		karray[i] = cache->numreplace;
		btreeWriteNumNode(cache,page,karray,parray,nkeys);
	    }
	
	}
	
	btreeFindHybBalanceTwo(cache,nextNode,nextLeft,nextRight,
                               nextAncL,nextAncR,key);

	if(thisNode != cache->secrootblock)
	    page = ajBtreeCacheRead(cache,thisNode);
	else
	{
	    page = btreeCacheLocate(cache,thisNode);
	    page->dirty = BT_LOCK;
            page->lockfor = 1792;
	}
	buf = page->buf;

    }
    else
    {
	if(nodetype == BT_LEAF || (nodetype==BT_ROOT && !cache->slevel))
	{
	    existed = btreeRemoveHybEntryTwo(cache,thisNode,key);

	    if(existed)
		cache->deleted = ajTrue;

	    GBT_NKEYS(buf,&nkeys);

	    if(nkeys >= minkeys || (nodetype==BT_ROOT && !cache->slevel))
		balanceNode = BTNO_BALANCE;
	    else
		balanceNode = page->pagepos;
	}
    }


    if(balanceNode == BTNO_BALANCE || thisNode == cache->secrootblock)
	done = BTNO_NODE;
    else
	done = btreeRebalanceHybTwo(cache,thisNode,leftNode,rightNode,
                                    lAnchor,rAnchor);
    

    btreeDeallocSecArray(cache,arrays);
    btreeDeallocSecArray(cache,arrays1);

    return done;
}




/* @funcstatic btreeFindHybMinTwo *********************************************
**
** Find minimum key in hybrid level 2 subtree and store in cache.
**
** Deletion software.
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page
** @param [r] key [ajulong] key
**
** @return [void]
** @@
******************************************************************************/

static void btreeFindHybMinTwo(AjPBtcache cache, ajulong pagepos,
                               ajulong key)
{
    AjPBtpage page   = NULL;

    AjPBtMem arrays = NULL;
    ajulong *karray   = NULL;
    ajulong *parray   = NULL;

    ajuint nkeys    = 0;
    ajuint nodetype = 0;
    ajuint nentries = 0;
    ajuint i;

    AjPNumBucket bucket = NULL;
    
    unsigned char *buf = NULL;

    /* ajDebug("In btreeFindHybMinTwo\n"); */

    arrays = btreeAllocSecArray(cache);
    karray = arrays->overflows;
    parray = arrays->parray;

    page = ajBtreeCacheRead(cache,pagepos);
    buf  = page->buf;
    GBT_NODETYPE(buf,&nodetype);
    GBT_NKEYS(buf,&nkeys);

    btreeGetNumKeys(cache,buf,&karray,&parray);

    if(nodetype == BT_LEAF)
    {
	bucket = btreeReadNumBucket(cache,parray[0]);
	nentries = bucket->Nentries;

        /*
        ** If there's only one entry then it must be the key marked
        ** for deletion
        */
        if(nentries<2)
	{
	    btreeNumBucketDel(&bucket);
	    bucket = btreeReadNumBucket(cache,parray[1]);
	    nentries = bucket->Nentries;
	}

        /* Check for empty bucket - shouldn't happen */
        /* Checking solely out of interest */
	if(nentries<1)	
	    ajFatal("FindHybMinTwo: Too few entries in bucket Nkeys=%u\n",
                    nkeys);


        /* Find lowest value key in the bucket and store in cache */
        cache->numreplace = bucket->NumId[0]->offset;

	if(cache->numreplace == key)
	    cache->numreplace = bucket->NumId[1]->offset;

        for(i=1;i<nentries;++i)
            if(bucket->NumId[i]->offset < cache->numreplace  &&
               bucket->NumId[i]->offset != key)
                cache->numreplace = bucket->NumId[i]->offset;

	btreeNumBucketDel(&bucket);
    }
    else
    {
	pagepos = parray[0];
	btreeFindHybMinTwo(cache,pagepos,key);
	
    }

    btreeDeallocSecArray(cache,arrays);
    
    return;
}




/* @funcstatic btreeRemoveHybEntryTwo *****************************************
**
** Find and delete an ID from a given hybrid tree level 2 leaf node if
** necessary.
**
** Deletion software
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] leaf node page
** @param [r] key [ajulong] key
**
** @return [AjBool] True if found (and deleted)
** @@
******************************************************************************/

static AjBool btreeRemoveHybEntryTwo(AjPBtcache cache, ajulong pagepos,
                                     ajulong key)
{
    AjPBtpage page   = NULL;
    AjPNumBucket bucket = NULL;
    
    ajulong *karray = NULL;
    ajulong *parray = NULL;
    ajulong blockno = 0L;
    
    ajuint nkeys    = 0;
    ajuint nentries = 0;
    ajuint i;

    ajuint dirtysave = 0;
    
    AjBool found = ajFalse;

    unsigned char *buf = NULL;
    AjPBtMem arrays = NULL;
    
    /* ajDebug("In btreeRemoveHybEntryTwo\n"); */

    page = ajBtreeCacheRead(cache,pagepos);
    buf = page->buf;
    dirtysave = page->dirty;
    page->dirty = BT_LOCK;
    page->lockfor = 1801;
   
    GBT_NKEYS(buf,&nkeys);

    if(!nkeys)
	return ajFalse;


    arrays = btreeAllocSecArray(cache);
    karray = arrays->overflows;
    parray = arrays->parray;

    btreeGetNumKeys(cache,buf,&karray,&parray);
    
    i=0;

    while(i!=nkeys && key >= karray[i])
	++i;

    blockno = parray[i];
    
    bucket = btreeReadNumBucket(cache,blockno);


    nentries = bucket->Nentries;
    found = ajFalse;

    for(i=0;i<nentries;++i)
	if(key == bucket->NumId[i]->offset)
	{
	    found = ajTrue;
	    break;
	}
    

    if(found)
    {
	/* Perform the deletion */
	if(nentries == 1)
	{
	    bucket->Nentries = 0;
            AJFREE(bucket->NumId[0]);
	}
	else
	{
            AJFREE(bucket->NumId[i]);
	    bucket->NumId[i] = bucket->NumId[nentries-1];
	    --bucket->Nentries;
	}

	btreeWriteNumBucket(cache,bucket,blockno);
	btreeAdjustHybBucketsTwo(cache,page);
	page->dirty = BT_DIRTY;
    }
    else
	page->dirty = dirtysave;

    btreeNumBucketDel(&bucket);

    btreeDeallocSecArray(cache,arrays);
    
    if(!found)
	return ajFalse;

    return ajTrue;
}




/* @funcstatic btreeAdjustHybBucketsTwo **************************************
**
** Re-order leaf buckets in 2ry hybrid tree
** Can be called whatever the state of a leaf.
**
** Deletion software
**
** @param [u] cache [AjPBtcache] cache
** @param [u] leaf [AjPBtpage] leaf page
**
** @return [void]
** @@
******************************************************************************/

static void btreeAdjustHybBucketsTwo(AjPBtcache cache, AjPBtpage leaf)
{
    ajuint nkeys = 0;
    unsigned char *lbuf = NULL;
    AjPNumBucket *buckets  = NULL;

    AjPBtMem arrays = NULL;    
    AjPBtMem overarrays = NULL;    

    ajulong *keys           = NULL;
    ajulong *ptrs           = NULL;
    ajulong *overflows      = NULL;
    
    ajuint i = 0;
    ajuint j = 0;
    
    ajuint order;
    ajuint bentries      = 0;
    ajuint totalkeys     = 0;
    ajuint nperbucket    = 0;
    ajuint maxnperbucket = 0;
    ajuint count         = 0;
    ajuint keylimit      = 0;
    ajuint bucketn       = 0;
    ajuint bucketlimit   = 0;
    ajuint nodetype      = 0;
    ajuint nids          = 0;
    ajuint totnids       = 0;
    
    AjPList idlist    = NULL;
    ajuint   dirtysave = 0;
    AjPBtNumId bid       = NULL;
    AjPNumBucket cbucket = NULL;
    AjPBtNumId cid       = NULL;

    ajuint v = 0;
    
    /* ajDebug("In btreeAdjustHybBucketsTwo\n"); */

    dirtysave = leaf->dirty;

    leaf->dirty = BT_LOCK;
    leaf->lockfor = 1811;
    lbuf = leaf->buf;

    GBT_NKEYS(lbuf,&nkeys);

    if(!nkeys)
    {
	leaf->dirty = dirtysave;
	return;
    }


    GBT_NODETYPE(lbuf,&nodetype);

    order = cache->sorder;
    nperbucket = cache->snperbucket;
    

    /* Read keys/ptrs */

    arrays = btreeAllocSecArray(cache);
    keys = arrays->overflows;
    ptrs = arrays->parray;

    overarrays = btreeAllocSecArray(cache);
    overflows = overarrays->overflows;
    

    btreeGetNumKeys(cache,lbuf,&keys,&ptrs);


    for(i=0;i<nkeys;++i)
	totalkeys += btreeNumInNumBucket(cache,ptrs[i]);

    totalkeys += btreeNumInNumBucket(cache,ptrs[i]);


    /* Set the number of entries per bucket to approximately half full */
    maxnperbucket = nperbucket >> 1;

    if(!maxnperbucket)
	++maxnperbucket;

    if(!leaf->pagepos)
	maxnperbucket = nperbucket;

    /* Work out the number of new buckets needed */
    bucketn = (totalkeys / maxnperbucket);

    if(totalkeys % maxnperbucket)
	++bucketn;

    if(bucketn == 1)
	++bucketn;


    while(bucketn > order)
    {
        ++maxnperbucket;
        bucketn = (totalkeys / maxnperbucket);
        if(totalkeys % maxnperbucket)
            ++bucketn;
    }

    /* Read buckets */
    AJCNEW0(buckets,nkeys+1);
    keylimit = nkeys + 1;
    
    for(i=0;i<keylimit;++i)
	buckets[i] = btreeReadNumBucket(cache,ptrs[i]);


    /* Read IDs from all buckets and push to list and sort (increasing id) */
    idlist  = ajListNew();

    for(i=0;i<keylimit;++i)
    {
	overflows[i] = buckets[i]->Overflow;
	bentries = buckets[i]->Nentries;
	for(j=0;j<bentries;++j)
	    ajListPush(idlist,(void *)buckets[i]->NumId[j]);
	
	AJFREE(buckets[i]->NumId);
	AJFREE(buckets[i]);
    }

    ajListSort(idlist,btreeNumIdCompare);
    AJFREE(buckets);

    cbucket = btreeNumBucketNew(maxnperbucket);
    bucketlimit = bucketn - 1;

    totnids = 0;
    nids = ajListGetLength(idlist);


    if(!totalkeys)
    {
	v = totalkeys;
	SBT_NKEYS(lbuf,v);

        btreeDeallocSecArray(cache,arrays);
        btreeDeallocSecArray(cache,overarrays);

	ajListFree(&idlist);
	leaf->dirty = BT_DIRTY;

	return;
    }
    
    if(nids <= maxnperbucket)
    {
	cbucket->Overflow = overflows[1];
	cbucket->Nentries = 0;
	ajListPeek(idlist,(void **)&bid);
	keys[0] = bid->offset;

	count = 0;

	while(count!=maxnperbucket && totnids != nids)
	{
	    ajListPop(idlist,(void **)&bid);
	    
	    cid = cbucket->NumId[count];
	    cid->dbno = bid->dbno;
	    cid->offset = bid->offset;
	    cid->refoffset = bid->refoffset;
	    
	    ++cbucket->Nentries;
	    ++count;
	    ++totnids;
	    AJFREE(bid);
	}


	if(!ptrs[1])
	    ptrs[1] = cache->totsize;

	btreeWriteNumBucket(cache,cbucket,ptrs[1]);

	cbucket->Overflow = overflows[0];
	cbucket->Nentries = 0;

	if(!ptrs[0])
	    ptrs[0] = cache->totsize;

	btreeWriteNumBucket(cache,cbucket,ptrs[0]);
    }
    else
    {
	for(i=0;i<bucketlimit;++i)
	{
	    cbucket->Overflow = overflows[i];
	    cbucket->Nentries = 0;

	    count = 0;

	    while(count!=maxnperbucket && totnids != nids)
	    {
		ajListPop(idlist,(void **)&bid);

		cid = cbucket->NumId[count];
		cid->dbno = bid->dbno;
		cid->offset = bid->offset;
		cid->refoffset = bid->refoffset;
		
		++cbucket->Nentries;
		++count;
		AJFREE(bid);
	    }


	    ajListPeek(idlist,(void **)&bid);
	    keys[i] = bid->offset;


	    if(!ptrs[i])
		ptrs[i] = cache->totsize;

	    btreeWriteNumBucket(cache,cbucket,ptrs[i]);
	}
	
	
	/* Deal with greater-than bucket */
	
	cbucket->Overflow = overflows[i];
	cbucket->Nentries = 0;
	
	
	
	count = 0;

	while(ajListPop(idlist,(void **)&bid))
	{
	    cid = cbucket->NumId[count];
	    cid->dbno = bid->dbno;
	    cid->offset = bid->offset;
	    cid->refoffset = bid->refoffset;
	    
	    ++cbucket->Nentries;
	    ++count;
	    AJFREE(bid);
	}
	
	
	if(!ptrs[i])
	    ptrs[i] = cache->totsize;
	
	btreeWriteNumBucket(cache,cbucket,ptrs[i]);
    }
    

    cbucket->Nentries = maxnperbucket;
    btreeNumBucketDel(&cbucket);

    /* Now write out a modified leaf with new keys/ptrs */

    nkeys = bucketn - 1;
    v = nkeys;
    SBT_NKEYS(lbuf,v);

    btreeWriteNumNode(cache,leaf,keys,ptrs,nkeys);

    leaf->dirty = dirtysave;
    if(nodetype == BT_ROOT)
    {
	leaf->dirty = BT_LOCK;
        leaf->lockfor = 1812;
    }
    
    btreeDeallocSecArray(cache,arrays);
    btreeDeallocSecArray(cache,overarrays);

    btreeNumBucketDel(&cbucket);
    ajListFree(&idlist);

    return;
}




/* @funcstatic btreeRebalanceHybTwo ******************************************
**
** Rebalance Hybrid level 2 tree after deletion
**
** Deletion software
**
** @param [u] cache [AjPBtcache] cache
** @param [r] thisNode [ajulong] Node to rebalance
** @param [r] leftNode [ajulong] left node
** @param [r] rightNode [ajulong] right node
** @param [r] lAnchor [ajulong] left anchor
** @param [r] rAnchor [ajulong] right anchor
**
** @return [ajulong] page number or BTNO_NODE
** @@
******************************************************************************/

static ajulong btreeRebalanceHybTwo(AjPBtcache cache, ajulong thisNode,
                                   ajulong leftNode, ajulong rightNode,
                                   ajulong lAnchor, ajulong rAnchor)
{
    unsigned char *lbuf = NULL;
    unsigned char *rbuf = NULL;
    unsigned char *tbuf = NULL;

    ajulong anchorNode   = 0L;
    ajulong balanceNode  = 0L;
    ajulong mergeNode    = 0L;
    ajulong done         = 0L;
    ajulong parent       = 0L;
    
    AjPBtpage lpage = NULL;
    AjPBtpage rpage = NULL;
    AjPBtpage tpage = NULL;
    
    ajuint lnkeys  = 0;
    ajuint rnkeys  = 0;
    ajuint size    = 0;
    ajuint order   = 0;
    ajuint minsize = 0;

    AjBool leftok  = ajFalse;
    AjBool rightok = ajFalse;
    
    
    /* ajDebug("In btreeRebalanceHybTwo\n"); */

    if(leftNode!=BTNO_NODE && lAnchor!=BTNO_NODE)
	leftok = ajTrue;

    if(rightNode!=BTNO_NODE && rAnchor!=BTNO_NODE)
	rightok = ajTrue;

    if(!leftok && !rightok)
	return BTNO_NODE;
    

    if(leftok)
    {
	lpage = ajBtreeCacheRead(cache,leftNode);
	lbuf  = lpage->buf;
	GBT_NKEYS(lbuf,&lnkeys);
    }
    

    if(rightok)
    {
	rpage = ajBtreeCacheRead(cache,rightNode);
	rbuf  = rpage->buf;
	GBT_NKEYS(rbuf,&rnkeys);
    }
    


    if(leftok && rightok)
    {
	size = (lnkeys >= rnkeys) ? lnkeys : rnkeys;
	balanceNode = (lnkeys >= rnkeys) ? leftNode : rightNode;
    }
    else if(leftok)
    {
	size = lnkeys;
	balanceNode = leftNode;
    }
    else
    {
	size = rnkeys;
	balanceNode = rightNode;
    }

    
    order = cache->sorder;
    minsize = (order-1) / 2;

    if((order-1)%2)
	++minsize;

    if(size >= minsize)
    {
	if(leftok && rightok)
	    anchorNode = (lnkeys >= rnkeys) ? lAnchor : rAnchor;
	else if(leftok)
	    anchorNode = lAnchor;
	else
	    anchorNode = rAnchor;

	done = btreeShiftHybTwo(cache,thisNode,balanceNode,anchorNode);
    }
	    
    else
    {
	tpage = ajBtreeCacheRead(cache,thisNode);
	tbuf  = tpage->buf;
	GBT_PREV(tbuf,&parent);

	if(leftok && rightok)
	{
	    anchorNode = (parent == lAnchor) ? lAnchor : rAnchor;
	    mergeNode  = (anchorNode == lAnchor) ? leftNode : rightNode;
	}
	else if(leftok)
	{
	    anchorNode = lAnchor;
	    mergeNode  = leftNode;
	}
	else
	{
	    anchorNode = rAnchor;
	    mergeNode  = rightNode;
	}

	done = btreeMergeHybTwo(cache,thisNode,mergeNode,anchorNode);
    }

    return done;
}




/* @funcstatic btreeShiftHybTwo ***********************************************
**
** Shift spare entries from one hybrid tree level 2 node to another.
**
** Deletion software.
**
** @param [u] cache [AjPBtcache] cache
** @param [r] thisNode [ajulong] master node
** @param [r] balanceNode [ajulong] balance node
** @param [r] anchorNode [ajulong] anchor node
**
** @return [ajulong] page number or BTNO_NODE
** @@
******************************************************************************/

static ajulong btreeShiftHybTwo(AjPBtcache cache, ajulong thisNode,
                                ajulong balanceNode, ajulong anchorNode)
{
    unsigned char *tbuf = NULL;
    unsigned char *abuf = NULL;
    unsigned char *bbuf = NULL;
    unsigned char *buf  = NULL;
    
    ajulong *kTarray = NULL;
    ajulong *kAarray = NULL;
    ajulong *kBarray = NULL;
    ajulong *pTarray = NULL;
    ajulong *pAarray = NULL;
    ajulong *pBarray = NULL;
    
    ajuint  nAkeys = 0;
    ajuint  nBkeys = 0;
    ajuint  nTkeys = 0;
    ajuint  i;
    ajint  ii;

    AjPBtpage pageA = NULL;
    AjPBtpage pageB = NULL;
    AjPBtpage pageT = NULL;
    AjPBtpage page  = NULL;
    
    AjPBtpage leftpage = NULL;

    ajuint anchorPos   = 0;
    ajulong prev        = 0L;
    ajuint  nodetype    = 0;

    ajulong lv = 0L;
    
    AjPBtMem arraysA = NULL;    
    AjPBtMem arraysB = NULL;    
    AjPBtMem arraysT = NULL;    

    /* ajDebug("In btreeShiftHybTwo\n"); */


    arraysA = btreeAllocSecArray(cache);
    kAarray = arraysA->overflows;
    pAarray = arraysA->parray;

    arraysB = btreeAllocSecArray(cache);
    kBarray = arraysB->overflows;
    pBarray = arraysB->parray;

    arraysT = btreeAllocSecArray(cache);
    kTarray = arraysT->overflows;
    pTarray = arraysT->parray;


    pageA = ajBtreeCacheRead(cache,anchorNode);
    pageA->dirty = BT_LOCK;
    pageA->lockfor = 1821;
    abuf = pageA->buf;
    pageB = ajBtreeCacheRead(cache,balanceNode);
    pageB->dirty = BT_LOCK;
    pageB->lockfor = 1822;
    bbuf = pageB->buf;
    pageT = ajBtreeCacheRead(cache,thisNode);
    pageT->dirty = BT_LOCK;
    pageT->lockfor = 1823;
    tbuf = pageT->buf;

    GBT_NKEYS(abuf,&nAkeys);
    GBT_NKEYS(bbuf,&nBkeys);
    GBT_NKEYS(tbuf,&nTkeys);

    btreeGetNumKeys(cache,abuf,&kAarray,&pAarray);
    btreeGetNumKeys(cache,bbuf,&kBarray,&pBarray);
    btreeGetNumKeys(cache,tbuf,&kTarray,&pTarray);

    if(kTarray[nTkeys-1] < kBarray[nBkeys-1])
	leftpage = pageT;
    else
	leftpage = pageB;


    if(leftpage == pageT)
    {
	/* Find anchor key position */
	i=0;

	while(i!=nAkeys && kTarray[nTkeys-1] >= kAarray[i])
	    ++i;

	anchorPos = i;

	/* Move down anchor key to thisNode */
        kTarray[nTkeys] = kAarray[anchorPos];
	++nTkeys;

	/* Shift extra */
	while(nTkeys < nBkeys)
	{
            kTarray[nTkeys] = kBarray[0];
	    pTarray[nTkeys] = pBarray[0];
	    ++nTkeys;
	    --nBkeys;

	    for(i=0;i<nBkeys;++i)
	    {
                kBarray[i] = kBarray[i+1];
		pBarray[i] = pBarray[i+1];
	    }

	    pBarray[i] = pBarray[i+1];
	}

	/* Adjust anchor key */
        kAarray[anchorPos] = kTarray[nTkeys-1];
	--nTkeys;
    }
    else	/* thisNode on the right */
    {
	/* Find anchor key position */
	i=0;

	while(i!=nAkeys && kBarray[nBkeys-1] >= kAarray[i])
	    ++i;

	anchorPos = i;

	/* Move down anchor key to thisNode */
	pTarray[nTkeys+1] = pTarray[nTkeys];

	for(ii=nTkeys-1;ii>-1;--ii)
	{
            kTarray[ii+1] = kTarray[ii];
	    pTarray[ii+1] = pTarray[ii];
	}

        kTarray[0] = kAarray[anchorPos];
	++nTkeys;

	/* Shift extra */
	while(nTkeys < nBkeys)
	{
	    pTarray[nTkeys+1] = pTarray[nTkeys];

	    for(ii=nTkeys-1;ii>-1;--ii)
	    {
                kTarray[ii+1] = kTarray[ii];
		pTarray[ii+1] = pTarray[ii];
	    }

            kTarray[0] = kBarray[nBkeys-1];
	    pTarray[1] = pBarray[nBkeys];
	    ++nTkeys;
	    --nBkeys;
	}


	/* Adjust anchor key */
        kAarray[anchorPos] = kTarray[0];
	--nTkeys;

	for(i=0;i<nTkeys;++i)
	{
            kTarray[i] = kTarray[i+1];
	    pTarray[i] = pTarray[i+1];
	}

	pTarray[i] = pTarray[i+1];
    }
    

    /* Adjust PREV pointers for thisNode */
    prev = pageT->pagepos;

    for(i=0;i<nTkeys+1;++i)
    {
	page = ajBtreeCacheRead(cache,pTarray[i]);
	buf = page->buf;
	GBT_NODETYPE(buf,&nodetype);

	if(nodetype != BT_BUCKET)
	{
	    lv = prev;
	    SBT_PREV(buf,lv);
	    page->dirty = BT_DIRTY;
	}
    }

    btreeWriteNumNode(cache,pageA,kAarray,pAarray,nAkeys);
    btreeWriteNumNode(cache,pageB,kBarray,pBarray,nBkeys);
    btreeWriteNumNode(cache,pageT,kTarray,pTarray,nTkeys);

    if(anchorNode == cache->secrootblock)
    {
	pageA->dirty = BT_LOCK;
        pageA->lockfor = 1824;
    }

    btreeDeallocSecArray(cache,arraysA);
    btreeDeallocSecArray(cache,arraysB);
    btreeDeallocSecArray(cache,arraysT);

    return BTNO_NODE;
}




/* @funcstatic btreeMergeHybTwo ***********************************************
**
** Merge two nodes in Hybrid 2ry tree
**
** Deletion software
**
** @param [u] cache [AjPBtcache] cache
** @param [r] thisNode [ajulong] master node
** @param [r] mergeNode [ajulong] merge node
** @param [r] anchorNode [ajulong] anchor node
**
** @return [ajulong] page number or BTNO_NODE
** @@
******************************************************************************/

static ajulong btreeMergeHybTwo(AjPBtcache cache, ajulong thisNode,
                                ajulong mergeNode, ajulong anchorNode)
{
    unsigned char *tbuf = NULL;
    unsigned char *abuf = NULL;
    unsigned char *nbuf = NULL;
    unsigned char *buf  = NULL;
    
    AjPBtMem arraysA = NULL;    
    AjPBtMem arraysN = NULL;    
    AjPBtMem arraysT = NULL;    

    ajulong *kTarray = NULL;
    ajulong *kAarray = NULL;
    ajulong *kNarray = NULL;
    ajulong *pTarray = NULL;
    ajulong *pAarray = NULL;
    ajulong *pNarray = NULL;

    ajulong thisprev  = 0L;
    ajulong mergeprev = 0L;
    
    
    ajuint  nAkeys = 0;
    ajuint  nNkeys = 0;
    ajuint  nTkeys = 0;
    ajuint  count  = 0;
    ajuint  i;
    ajint ii;

    ajuint   nodetype = 0;
    
    ajuint saveA = 0;
    ajuint saveN = 0;
    ajuint saveT = 0;
    
    AjPBtpage pageA = NULL;
    AjPBtpage pageN = NULL;
    AjPBtpage pageT = NULL;
    AjPBtpage page  = NULL;
    
    AjPBtpage leftpage = NULL;

    ajuint anchorPos = 0;
    ajulong prev      = 0L;

    ajulong lv = 0L;

    AjBool collapse = ajFalse;
    ajulong csrb     = 0L;
    
    /* ajDebug("In btreeMergeHybTwo\n"); */

    pageA = ajBtreeCacheRead(cache,anchorNode);
    saveA = pageA->dirty;
    pageA->dirty = BT_LOCK;
    pageA->lockfor = 1831;
    abuf = pageA->buf;
    pageN = ajBtreeCacheRead(cache,mergeNode);
    saveN = pageN->dirty;
    pageN->dirty = BT_LOCK;
    pageN->lockfor = 1832;
    nbuf = pageN->buf;
    pageT = ajBtreeCacheRead(cache,thisNode);
    saveT = pageT->dirty;
    pageT->dirty = BT_LOCK;
    pageT->lockfor = 1833;
    tbuf = pageT->buf;

    GBT_PREV(tbuf,&thisprev);
    GBT_PREV(nbuf,&mergeprev);

    GBT_NKEYS(abuf,&nAkeys);
    GBT_NKEYS(nbuf,&nNkeys);
    GBT_NKEYS(tbuf,&nTkeys);

    GBT_NODETYPE(nbuf,&nodetype);

    csrb = cache->secrootblock;

    if(nAkeys == 1)
    {
	if(anchorNode==csrb && thisprev==csrb && mergeprev==csrb)
	    collapse = ajTrue;
	else
	{
	    pageA->dirty = saveA;
	    pageN->dirty = saveN;
	    pageT->dirty = saveT;

	    return thisNode;
	}
    }

    arraysA = btreeAllocSecArray(cache);
    kAarray = arraysA->overflows;
    pAarray = arraysA->parray;

    arraysN = btreeAllocSecArray(cache);
    kNarray = arraysN->overflows;
    pNarray = arraysN->parray;

    arraysT = btreeAllocSecArray(cache);
    kTarray = arraysT->overflows;
    pTarray = arraysT->parray;

    btreeGetNumKeys(cache,abuf,&kAarray,&pAarray);
    btreeGetNumKeys(cache,nbuf,&kNarray,&pNarray);
    btreeGetNumKeys(cache,tbuf,&kTarray,&pTarray);

    if(kTarray[nTkeys-1] < kNarray[nNkeys-1])
	leftpage = pageT;
    else
	leftpage = pageN;


    if(leftpage == pageT)
    {
	/* Find anchor key position */
	i=0;

	while(i!=nAkeys && kTarray[nTkeys-1] >= kAarray[i])
	    ++i;

	anchorPos = i;

	/* Move down anchor key to neighbour Node */
	pNarray[nNkeys+1] = pNarray[nNkeys];

	for(ii=nNkeys-1;ii>-1;--ii)
	{
	    kNarray[ii+1] = kNarray[ii];
	    pNarray[ii+1] = pNarray[ii];
	}

	kNarray[0] = kAarray[anchorPos];
	++nNkeys;


	/* Adjust anchor node keys/ptrs */
	++anchorPos;

	if(anchorPos==nAkeys)
	    pAarray[nAkeys-1] = pAarray[nAkeys];
	else
	{
	    for(i=anchorPos;i<nAkeys;++i)
	    {
		kAarray[i-1] = kAarray[i];
		pAarray[i-1] = pAarray[i];
	    }
	    pAarray[i-1] = pAarray[i];
	}
	--nAkeys;
	

	/* Merge this to neighbour */

	while(nTkeys)
	{
	    pNarray[nNkeys+1] = pNarray[nNkeys];

	    for(ii=nNkeys-1;ii>-1;--ii)
	    {
		kNarray[ii+1] = kNarray[ii];
		pNarray[ii+1] = pNarray[ii];
	    }

	    kNarray[0] = kTarray[nTkeys-1];
	    pNarray[1] = pTarray[nTkeys];
	    pNarray[0] = pTarray[nTkeys-1];
	    --nTkeys;
	    ++nNkeys;
	}

	/* At this point the 'this' node could be added to a free list */
    }
    else
    {
	/* Find anchor key position */
	i=0;

	while(i!=nAkeys && kNarray[nNkeys-1] >= kAarray[i])
	    ++i;

	anchorPos = i;

	/* Move down anchor key to neighbourNode */
	kNarray[nNkeys] = kAarray[anchorPos];
	++nNkeys;

	/* Adjust anchor node keys/ptrs */
	++anchorPos;

	if(anchorPos!=nAkeys)
	    for(i=anchorPos;i<nAkeys;++i)
	    {
		kAarray[i-1] = kAarray[i];
		pAarray[i]   = pAarray[i+1];
	    }

	--nAkeys;

	/* merge extra */
	count = 0;

	while(nTkeys)
	{
	    kNarray[nNkeys] = kTarray[count];
	    pNarray[nNkeys] = pTarray[count];
	    ++nNkeys;
	    ++count;
	    --nTkeys;
	    pNarray[nNkeys] = pTarray[count];
	
	}

	/* At this point the 'this' node could be added to a free list */
    }
    
    
    /* Adjust PREV pointers for neighbour Node */
    prev = pageN->pagepos;

    for(i=0;i<nNkeys+1;++i)
    {
	page = ajBtreeCacheRead(cache,pNarray[i]);
	buf = page->buf;
	GBT_NODETYPE(buf,&nodetype);

	if(nodetype != BT_BUCKET)
	{
	    lv = prev;
	    SBT_PREV(buf,lv);
	    page->dirty = BT_DIRTY;
	}
    }

    pageT->dirty = BT_CLEAN;
    btreeWriteNumNode(cache,pageA,kAarray,pAarray,nAkeys);
    btreeWriteNumNode(cache,pageN,kNarray,pNarray,nNkeys);

    if(anchorNode == csrb)
    {
	pageA->dirty = BT_LOCK;
        pageA->lockfor = 1834;
    }

    btreeDeallocSecArray(cache,arraysA);
    btreeDeallocSecArray(cache,arraysN);
    btreeDeallocSecArray(cache,arraysT);

    if(collapse)
	btreeCollapseRootHybTwo(cache,mergeNode);

    return thisNode;
}




/* @funcstatic btreeCollapseRootHybTwo ****************************************
**
** Collapse root page for hybrid level 2 tree.
**
** Deletion software.
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number to make new root
**
** @return [ajulong] page number or BTNO_NODE
** @@
******************************************************************************/

static ajulong btreeCollapseRootHybTwo(AjPBtcache cache, ajulong pagepos)
{
    unsigned char *buf  = NULL;
    unsigned char *lbuf = NULL;

    ajulong *karray = NULL;
    ajulong *parray = NULL;

    AjPBtpage rootpage = NULL;
    AjPBtpage page     = NULL;
    
    ajuint nodetype = 0;
    ajuint nkeys    = 0;
    ajuint i;

    ajulong prev = 0L;
    AjPBtMem arrays = NULL;
    
    /* ajDebug("In btreeCollapseRootHybTwo\n"); */
    
    if(!cache->slevel)
	return BTNO_NODE;

    rootpage = btreeCacheLocate(cache,cache->secrootblock);
    buf = rootpage->buf;
    page = ajBtreeCacheRead(cache,pagepos);


    arrays = btreeAllocSecArray(cache);
    karray = arrays->overflows;
    parray = arrays->parray;

    /*
    ** Swap pagepos values to make root the child and child the root
    ** Update node types and mark the original root as a clean page
    */

    /* At this point page->pagepos could be added to a free list */

    rootpage->pagepos = page->pagepos;
    rootpage->dirty = BT_CLEAN;
    nodetype = BT_INTERNAL;
    SBT_NODETYPE(buf,nodetype);

    page->pagepos = cache->secrootblock;
    page->dirty = BT_LOCK;
    page->lockfor = 1841;
    buf = page->buf;
    nodetype = BT_ROOT;
    SBT_NODETYPE(buf,nodetype);
    
    --cache->slevel;

    if(cache->slevel)
    {
	/*
	 ** Update the PREV pointers of the new root's children
	 */
	GBT_NKEYS(buf,&nkeys);
	btreeGetNumKeys(cache,buf,&karray,&parray);

	for(i=0;i<nkeys+1;++i)
	{
	    page = ajBtreeCacheRead(cache,parray[i]);
	    lbuf = page->buf;
	    SBT_PREV(lbuf,prev);
	    page->dirty = BT_DIRTY;
	}
    }

    btreeDeallocSecArray(cache,arrays);

    cache->secrootblock = pagepos;
    
    return 0L;
}




/* @func ajBtreeDeletePriId *********************************************
**
** Entry point for keyword tree ID deletion.
**
** Deletion software
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pri [const AjPBtPri] keyword object
**
** @return [AjBool] True if found and deleted
** @@
******************************************************************************/

AjBool ajBtreeDeletePriId(AjPBtcache cache, const AjPBtPri pri)
{
    AjPBtpage rootpage = NULL;
    AjPBtpage spage    = NULL;
    AjPBtpage page     = NULL;
    AjPStr key         = NULL;
    AjPSecBucket bucket  = NULL;
    ajulong blockno  = 0L;

    ajuint nkeys = 0;
    ajulong slevel = 0L;

    ajuint nentries = 0;
    
    AjPStr *karray = NULL;
    ajulong *parray = NULL;
    AjPBtMem arrays = NULL;
    AjBool found = ajFalse;
    
    
    ajuint i;
    
    unsigned char *buf = NULL;
    ajulong  secrootpage = 0L;

    AjBool empty = ajFalse;
    AjBool ret   = ajFalse;

    AjPBtpage prirootpage = NULL;
    
    ajulong treeblock = 0L;
    
    /* ajDebug("In ajBtreeDeletePriId\n"); */

    key = ajStrNew();
    

    ajStrAssignS(&key,pri->keyword);

    if(!ajStrGetLen(key))
    {
	ajStrDel(&key);

	return ajFalse;
    }

    if(!ajBtreePriFindKeyword(cache,key,&treeblock))
    {
        ajStrDel(&key);
        ajWarn("DeletePriId: Keyword %S not found",pri->keyword);

        return ajFalse;
    }

    secrootpage = treeblock;
    cache->secrootblock = treeblock;
    
    arrays = btreeAllocSecArray(cache);
    karray = arrays->karray;
    parray = arrays->parray;
    
    page = ajBtreeCacheRead(cache,secrootpage);
    page->dirty = BT_LOCK;
    page->lockfor = 1851;
    buf = page->buf;

    GBT_RIGHT(buf,&slevel);
    cache->slevel = (ajuint)slevel;

    spage = ajBtreeSecFindInsert(cache,pri->id);
    buf = spage->buf;

    btreeGetKeys(cache,buf,&karray,&parray);


    
    GBT_NKEYS(buf,&nkeys);

    if(!nkeys)
    {
	btreeDeallocSecArray(cache,arrays);
	ajStrDel(&key);
        page->dirty = BT_CLEAN;
        
	return ajFalse;
    }


    i=0;

    while(i!=nkeys && MAJSTRCMPS(pri->id,karray[i])>=0)
        ++i;

    blockno = parray[i];

    bucket = btreeReadSecBucket(cache,blockno);

    nentries = bucket->Nentries;

    found = ajFalse;

    for(i=0;i<nentries;++i)
	if(ajStrMatchS(pri->id,bucket->SecIds[i]))
	{
	    found = ajTrue;
	    break;
	}

    if(!found)
    {
        ajWarn("DeletePriId: ID %S  not found for Keyword %S",pri->id,
               pri->keyword);
	btreeDeallocPriArray(cache,arrays);
	ajStrDel(&key);
        page->dirty = BT_CLEAN;

	return ajFalse;
    }
    

    
    /*
    ** Have to delete ID from secondary tree. If that empties the
    ** tree then have to delete keyword from primary tree.
    ** Needs a little thought. Maybe use cache->deleted for multiple
    ** purposes i.e. 0=not deleted  1=keyword deleted 2=tree deleted
    */

    rootpage = btreeCacheLocate(cache,secrootpage);
    if(!rootpage)
        ajFatal("DeletePriId: secondary root page became unlocked");
    rootpage->dirty = BT_LOCK;
    rootpage->lockfor = 1852;
    buf = rootpage->buf;

    GBT_RIGHT(buf,&slevel);
    cache->slevel = (ajuint)slevel;

    btreeFindPriBalanceTwo(cache,secrootpage,BTNO_NODE,BTNO_NODE,BTNO_NODE,
                           BTNO_NODE,pri);

    ret = cache->deleted;

    if(!ret)
    {
        btreeDeallocSecArray(cache,arrays);
        ajStrDel(&key);
        page->dirty = BT_CLEAN;

        return ajFalse;
    }
    
    empty = btreeIsSecEmpty(cache);

    
    if(empty)
    {
        prirootpage = btreeCacheLocate(cache,0L);

        if(!prirootpage)
            ajFatal("ajBtreeDeletePriId: prirootpage unlocked");
        
    
        btreeFindPriBalanceOne(cache,0L,BTNO_NODE,BTNO_NODE,BTNO_NODE,
                               BTNO_NODE,pri);

        ret = cache->deleted;
    }
    

    btreeDeallocSecArray(cache,arrays);
    ajStrDel(&key);

    return ret;
}




/* @funcstatic btreeFindPriBalanceTwo ******************************************
**
** Master routine for entry deletion from level 2 keyword tree.
**
** Deletion software.
**
** @param [u] cache [AjPBtcache] cache
** @param [r] thisNode [ajulong] Current node
** @param [r] leftNode [ajulong] Node to left
** @param [r] rightNode [ajulong] Node to right
** @param [r] lAnchor [ajulong] Left anchor
** @param [r] rAnchor [ajulong] Right anchor
** @param [r] pri [const AjPBtPri] pri
**
** @return [ajulong] page number or BTNO_NODE
** @@
******************************************************************************/

static ajulong btreeFindPriBalanceTwo(AjPBtcache cache, ajulong thisNode,
                                      ajulong leftNode, ajulong rightNode,
                                      ajulong lAnchor, ajulong rAnchor,
                                      const AjPBtPri pri)
{
    unsigned char *buf  = NULL;
    unsigned char *buf1 = NULL;
    
    ajulong nextNode   = BTNO_NODE;
    ajulong nextLeft   = BTNO_NODE;
    ajulong nextRight  = BTNO_NODE;
    ajulong nextAncL   = BTNO_NODE;
    ajulong nextAncR   = BTNO_NODE;
    ajulong done       = 0L;
    
    ajuint  nkeys      = 0;
    ajuint  order      = 0;
    ajuint  minkeys    = 0;
    ajuint  i;
    ajuint  nodetype   = 0;

    ajuint n1keys      = 0;
    
    AjPBtpage page  = NULL;
    AjPBtpage page1 = NULL;

    ajulong balanceNode = 0L;
    ajulong blockno     = 0L;
    ajulong ptrSave     = 0L;

    AjPStr *karray  = NULL;
    ajulong *parray  = NULL;
    AjPStr *k1array = NULL;
    ajulong *p1array = NULL;

    AjPBtMem arrays  = NULL;
    AjPBtMem arrays1 = NULL;
    
    const AjPStr key = NULL;
    AjBool existed = ajFalse;
    
    /* ajDebug("In btreeFindPriBalanceTwo\n"); */

    if(thisNode != cache->secrootblock)
	page = ajBtreeCacheRead(cache,thisNode);
    else
    {
	page = btreeCacheLocate(cache,thisNode);
	page->dirty = BT_LOCK;
        page->lockfor = 1861;
    }

    cache->deleted = ajFalse;

    buf = page->buf;
    GBT_NKEYS(buf,&nkeys);
    
    order = cache->sorder;
    /* order-1 is the number of keys in the node */
    minkeys = (order-1) / 2;
    if((order-1)%2)
	++minkeys;

    /*
    ** If thisNode contains >= minkeys then it is not a candidate
    ** for balancing
    */
    if(nkeys >= minkeys)
	balanceNode = BTNO_BALANCE;
    else
	balanceNode = page->pagepos;

    arrays  = btreeAllocSecArray(cache);
    arrays1 = btreeAllocSecArray(cache);

    karray = arrays->karray;
    parray = arrays->parray;

    k1array = arrays1->karray;
    p1array = arrays1->parray;

    key = pri->id;

    btreeGetKeys(cache,buf,&karray,&parray);

    i=0;

    while(i!=nkeys && MAJSTRCMPS(key,karray[i])>=0)
	++i;

    blockno = parray[i];

    nextNode = blockno;
    ptrSave = i;

    GBT_NODETYPE(buf,&nodetype);

    if(!(nodetype == BT_LEAF) && !(nodetype == BT_ROOT && !cache->slevel))
    {
	if(nextNode == parray[0])
	{
	    if(leftNode != BTNO_NODE)
	    {
		page1 = ajBtreeCacheRead(cache,leftNode);
		buf1 = page1->buf;
		GBT_NKEYS(buf1,&n1keys);
		btreeGetKeys(cache,buf1,&k1array,&p1array);
		nextLeft = p1array[n1keys];
	    }
	    else
		nextLeft = BTNO_NODE;
	    
	    if(thisNode == cache->secrootblock)
		nextAncL = thisNode;
	    else
		nextAncL = lAnchor;
	}
	else
	{
	    nextLeft = parray[ptrSave-1];
	    nextAncL = thisNode;
	}

	if(nextNode == parray[nkeys])
	{
	    if(rightNode != BTNO_NODE)
	    {
		page1 = ajBtreeCacheRead(cache,rightNode);
		buf1 = page1->buf;
		GBT_NKEYS(buf1,&n1keys);
		btreeGetKeys(cache,buf1,&k1array,&p1array);
		nextRight = p1array[0];
	    }
	    else
		nextRight = BTNO_NODE;

	    if(thisNode == cache->secrootblock)
		nextAncR = thisNode;
	    else
		nextAncR = rAnchor;
	}
	else
	{
	    nextRight = parray[ptrSave+1];
	    nextAncR  = thisNode;
	}



	/* Check to see whether key exists in an internal node */
	if(nodetype != BT_LEAF && cache->slevel)
	{
	    i=0;

	    while(i!=nkeys && MAJSTRCMPS(key,karray[i]))
		++i;

	    if(i!=nkeys)
	    {
		btreeFindPriMinTwo(cache,parray[i+1],key);
		ajStrAssignS(&karray[i],cache->replace);
		btreeWriteNode(cache,page,karray,parray,nkeys);
	    }
	
	}
	
	btreeFindPriBalanceTwo(cache,nextNode,nextLeft,nextRight,
                               nextAncL,nextAncR,pri);

	if(thisNode != cache->secrootblock)
	    page = ajBtreeCacheRead(cache,thisNode);
	else
	{
	    page = btreeCacheLocate(cache,thisNode);
	    page->dirty = BT_LOCK;
            page->lockfor = 1862;
	}
	buf = page->buf;

    }
    else
    {
	if(nodetype == BT_LEAF || (nodetype==BT_ROOT && !cache->slevel))
	{
	    existed = btreeRemovePriEntryTwo(cache,thisNode,pri);

	    if(existed)
		cache->deleted = ajTrue;
	    GBT_NKEYS(buf,&nkeys);

	    if(nkeys >= minkeys || (nodetype==BT_ROOT && !cache->slevel))
		balanceNode = BTNO_BALANCE;
	    else
		balanceNode = page->pagepos;
	}
    }


    if(balanceNode == BTNO_BALANCE || thisNode == cache->secrootblock)
	done = BTNO_NODE;
    else
	done = btreeRebalancePriTwo(cache,thisNode,leftNode,rightNode,
                                    lAnchor,rAnchor);

    btreeDeallocSecArray(cache,arrays);
    btreeDeallocSecArray(cache,arrays1);

    return done;
}




/* @funcstatic btreeFindPriMinTwo *********************************************
**
** Find minimum key in keyword level 2 subtree and store in cache.
**
** Deletion software.
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page
** @param [r] key [const AjPStr] key
**
** @return [void]
** @@
******************************************************************************/

static void btreeFindPriMinTwo(AjPBtcache cache, ajulong pagepos,
                               const AjPStr key)
{
    AjPBtpage page   = NULL;
    AjPSecBucket bucket = NULL;
    AjPStr *karray   = NULL;
    ajulong *parray   = NULL;

    ajuint nkeys    = 0;
    ajuint nodetype = 0;
    ajuint nentries = 0;
    ajuint i;
    AjPBtMem arrays = NULL;
    
    unsigned char *buf = NULL;

    /* ajDebug("In btreeFindPriMinTwo\n"); */

    arrays = btreeAllocSecArray(cache);
    karray = arrays->karray;
    parray = arrays->parray;

    page = ajBtreeCacheRead(cache,pagepos);
    buf  = page->buf;
    GBT_NODETYPE(buf,&nodetype);
    GBT_NKEYS(buf,&nkeys);

    btreeGetKeys(cache,buf,&karray,&parray);

    if(nodetype == BT_LEAF)
    {
	bucket = btreeReadSecBucket(cache,parray[0]);
	nentries = bucket->Nentries;

        /*
        ** If there's only one entry then it must be the key marked
        ** for deletion
        */
        if(nentries<2)
	{
	    btreeSecBucketDel(&bucket);
	    bucket = btreeReadSecBucket(cache,parray[1]);
	    nentries = bucket->Nentries;
	}

        /* Check for empty bucket - shouldn't happen */
        /* Checking solely out of interest */
	if(nentries<1)	
	    ajFatal("FindPriMinTwo: Too few entries in bucket Nkeys=%u\n",
                    nkeys);


        /* Find lowest value key in the bucket and store in cache */
        ajStrAssignS(&cache->replace,bucket->SecIds[0]);

	if(!MAJSTRCMPS(cache->replace,key))
	    ajStrAssignS(&cache->replace,bucket->SecIds[1]);

	for(i=1;i<nentries;++i)
	    if(MAJSTRCMPS(bucket->SecIds[i],cache->replace)<0 &&
	       MAJSTRCMPS(bucket->SecIds[i],key))
		ajStrAssignS(&cache->replace,bucket->SecIds[i]);

	btreeSecBucketDel(&bucket);
    }
    else
    {
	pagepos = parray[0];
	btreeFindPriMinTwo(cache,pagepos,key);
	
    }

    btreeDeallocSecArray(cache,arrays);
    
    return;
}




/* @funcstatic btreeRemovePriEntryTwo *****************************************
**
** Find and delete an ID from a given keyword tree level 2 leaf node if
** necessary.
**
** Deletion software
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] leaf node page
** @param [r] pri [const AjPBtPri] pri
**
** @return [AjBool] True if found (and deleted)
** @@
******************************************************************************/

static AjBool btreeRemovePriEntryTwo(AjPBtcache cache, ajulong pagepos,
                                     const AjPBtPri pri)
{
    AjPBtpage page   = NULL;
    AjPSecBucket bucket = NULL;
    
    AjPStr *karray = NULL;
    ajulong *parray = NULL;
    ajulong blockno = 0L;
    
    ajuint nkeys    = 0;
    ajuint nentries = 0;
    ajuint i;

    ajuint dirtysave = 0;
    
    AjBool found = ajFalse;
    const AjPStr key  = NULL;

    unsigned char *buf = NULL;
    AjPBtMem arrays = NULL;
    
    /* ajDebug("In btreeRemovePriEntryTwo\n"); */

    page = ajBtreeCacheRead(cache,pagepos);
    buf = page->buf;
    dirtysave = page->dirty;
    page->dirty = BT_LOCK;
    page->lockfor = 1871;
  
    GBT_NKEYS(buf,&nkeys);

    if(!nkeys)
	return ajFalse;


    arrays = btreeAllocSecArray(cache);
    karray = arrays->karray;
    parray = arrays->parray;

    btreeGetKeys(cache,buf,&karray,&parray);
    
    key = pri->id;

    i=0;

    while(i!=nkeys && MAJSTRCMPS(key,karray[i])>=0)
	++i;

    blockno = parray[i];
    
    bucket = btreeReadSecBucket(cache,blockno);


    nentries = bucket->Nentries;
    found = ajFalse;

    for(i=0;i<nentries;++i)
	if(!MAJSTRCMPS(key,bucket->SecIds[i]))
	{
	    found = ajTrue;
	    break;
	}
    

    if(found)
    {
	/* Perform the deletion */
	if(nentries == 1)
	{
	    bucket->Nentries = 0;
	    AJFREE(bucket->SecIds[0]);
	}
	else
	{
	    AJFREE(bucket->SecIds[i]);
	    bucket->SecIds[i] = bucket->SecIds[nentries-1];
	    --bucket->Nentries;
	}

	btreeWriteSecBucket(cache,bucket,blockno);
	btreeAdjustPriBucketsTwo(cache,page);
	page->dirty = BT_DIRTY;
    }
    else
	page->dirty = dirtysave;

    btreeSecBucketDel(&bucket);

    btreeDeallocSecArray(cache,arrays);
    
    if(!found)
	return ajFalse;

    return ajTrue;
}




/* @funcstatic btreeAdjustPriBucketsTwo **************************************
**
** Re-order leaf buckets in keyword level 2 tree
** Can be called whatever the state of a leaf.
**
** Deletion software
**
** @param [u] cache [AjPBtcache] cache
** @param [u] leaf [AjPBtpage] leaf page
**
** @return [void]
** @@
******************************************************************************/

static void btreeAdjustPriBucketsTwo(AjPBtcache cache, AjPBtpage leaf)
{
    ajuint nkeys = 0;
    unsigned char *lbuf = NULL;
    AjPSecBucket *buckets  = NULL;

    AjPBtMem arrays = NULL;    
    AjPBtMem newarrays = NULL;    
    AjPStr *keys        = NULL;
    ajulong *ptrs        = NULL;
    ajulong *overflows   = NULL;
    
    ajuint i = 0;
    ajuint j = 0;
    
    ajuint order;
    ajuint bentries      = 0;
    ajuint totalkeys     = 0;
    ajuint nperbucket    = 0;
    ajuint maxnperbucket = 0;
    ajuint count         = 0;
    ajuint totkeylen     = 0;
    ajuint keylimit      = 0;
    ajuint bucketn       = 0;
    ajuint bucketlimit   = 0;
    ajuint nodetype      = 0;
    ajuint nids          = 0;
    ajuint totnids       = 0;
    
    AjPList idlist    = NULL;
    ajuint   dirtysave = 0;
    AjPStr bid       = NULL;
    AjPSecBucket cbucket = NULL;
    AjPStr cid       = NULL;

    ajuint v = 0;
    
    /* ajDebug("In btreeAdjustPriBucketsTwo\n"); */

    dirtysave = leaf->dirty;

    leaf->dirty = BT_LOCK;
    leaf->lockfor = 1881;
    lbuf = leaf->buf;

    GBT_NKEYS(lbuf,&nkeys);

    if(!nkeys)
    {
	leaf->dirty = dirtysave;

	return;
    }


    GBT_NODETYPE(lbuf,&nodetype);

    order = cache->sorder;
    nperbucket = cache->snperbucket;
    

    /* Read keys/ptrs */

    arrays = btreeAllocSecArray(cache);
    ptrs = arrays->parray;

    btreeGetPointers(cache,lbuf,&ptrs);


    for(i=0;i<nkeys;++i)
	totalkeys += btreeNumInSecBucket(cache,ptrs[i]);

    totalkeys += btreeNumInSecBucket(cache,ptrs[i]);


    /* Set the number of entries per bucket to approximately half full */
    maxnperbucket = nperbucket >> 1;

    if(!maxnperbucket)
	++maxnperbucket;

    if(!leaf->pagepos)
	maxnperbucket = nperbucket;

    /* Work out the number of new buckets needed */
    bucketn = (totalkeys / maxnperbucket);

    if(totalkeys % maxnperbucket)
	++bucketn;

    if(bucketn == 1)
	++bucketn;
    
    while(bucketn > order)
    {
        ++maxnperbucket;
        bucketn = (totalkeys / maxnperbucket);

        if(totalkeys % maxnperbucket)
            ++bucketn;
    }

    /* Read buckets */
    AJCNEW0(buckets,nkeys+1);
    keylimit = nkeys + 1;
    
    for(i=0;i<keylimit;++i)
	buckets[i] = btreeReadSecBucket(cache,ptrs[i]);


    newarrays = btreeAllocSecArray(cache);
    keys = newarrays->karray;
    overflows = newarrays->overflows;
    
    /* Read IDs from all buckets and push to list and sort (increasing id) */
    idlist  = ajListNew();

    for(i=0;i<keylimit;++i)
    {
	overflows[i] = buckets[i]->Overflow;
	bentries = buckets[i]->Nentries;

	for(j=0;j<bentries;++j)
	    ajListPush(idlist,(void *)buckets[i]->SecIds[j]);
	
	AJFREE(buckets[i]->keylen);
	AJFREE(buckets[i]->SecIds);
	AJFREE(buckets[i]);
    }

/* AJB: check this compares ajStr objects OK */
    ajListSort(idlist,ajStrVcmp);
    AJFREE(buckets);

    cbucket = btreeSecBucketNew(maxnperbucket,cache->kwlimit);
    bucketlimit = bucketn - 1;

    totnids = 0;
    nids = ajListGetLength(idlist);


    if(!totalkeys)
    {
	v = totalkeys;
	SBT_NKEYS(lbuf,v);

        btreeDeallocSecArray(cache,arrays);
        btreeDeallocSecArray(cache,newarrays);

	ajListFree(&idlist);
	leaf->dirty = BT_DIRTY;

	return;
    }
    
    if(nids <= maxnperbucket)
    {
	cbucket->Overflow = overflows[1];
	cbucket->Nentries = 0;
	ajListPeek(idlist,(void **)&bid);
	ajStrAssignS(&keys[0],bid);

	count = 0;

	while(count!=maxnperbucket && totnids != nids)
	{
	    ajListPop(idlist,(void **)&bid);
	    
	    cid = cbucket->SecIds[count];
	    ajStrAssignS(&cid,bid);
	    
	    cbucket->keylen[count] = BT_BUCKSECLEN(bid);
	    ++cbucket->Nentries;
	    ++count;
	    ++totnids;
	    ajStrDel(&bid);
	}


	totkeylen += ajStrGetLen(keys[0]);

	if(!ptrs[1])
	    ptrs[1] = cache->totsize;

	btreeWriteSecBucket(cache,cbucket,ptrs[1]);

	cbucket->Overflow = overflows[0];
	cbucket->Nentries = 0;

	if(!ptrs[0])
	    ptrs[0] = cache->totsize;

	btreeWriteSecBucket(cache,cbucket,ptrs[0]);
    }
    else
    {
	for(i=0;i<bucketlimit;++i)
	{
	    cbucket->Overflow = overflows[i];
	    cbucket->Nentries = 0;

	    count = 0;

	    while(count!=maxnperbucket && totnids != nids)
	    {
		ajListPop(idlist,(void **)&bid);

		cid = cbucket->SecIds[count];
		ajStrAssignS(&cid,bid);
		
		cbucket->keylen[count] = BT_BUCKSECLEN(bid);
		++cbucket->Nentries;
		++count;
		ajStrDel(&bid);
	    }


	    ajListPeek(idlist,(void **)&bid);
	    ajStrAssignS(&keys[i],bid);


	    totkeylen += ajStrGetLen(bid);

	    if(!ptrs[i])
		ptrs[i] = cache->totsize;

	    btreeWriteSecBucket(cache,cbucket,ptrs[i]);
	}
	
	
	/* Deal with greater-than bucket */
	
	cbucket->Overflow = overflows[i];
	cbucket->Nentries = 0;

	count = 0;

	while(ajListPop(idlist,(void **)&bid))
	{
	    cid = cbucket->SecIds[count];
	    ajStrAssignS(&cid,bid);
	    
	    ++cbucket->Nentries;
	    ++count;
	    ajStrDel(&bid);
	}
	
	
	if(!ptrs[i])
	    ptrs[i] = cache->totsize;
	
	btreeWriteSecBucket(cache,cbucket,ptrs[i]);
    }
    

    cbucket->Nentries = maxnperbucket;
    btreeSecBucketDel(&cbucket);

    /* Now write out a modified leaf with new keys/ptrs */

    nkeys = bucketn - 1;
    btreeWriteNode(cache,leaf,keys,ptrs,nkeys);

    leaf->dirty = dirtysave;
    if(nodetype == BT_ROOT)
    {
	leaf->dirty = BT_LOCK;
        leaf->lockfor = 1882;
    }

    btreeDeallocSecArray(cache,arrays);
    btreeDeallocSecArray(cache,newarrays);

    btreeSecBucketDel(&cbucket);
    ajListFree(&idlist);

    return;
}




/* @funcstatic btreeRebalancePriTwo ******************************************
**
** Rebalance keyword level 2 tree after deletion
**
** Deletion software
**
** @param [u] cache [AjPBtcache] cache
** @param [r] thisNode [ajulong] Node to rebalance
** @param [r] leftNode [ajulong] left node
** @param [r] rightNode [ajulong] right node
** @param [r] lAnchor [ajulong] left anchor
** @param [r] rAnchor [ajulong] right anchor
**
** @return [ajulong] page number or BTNO_NODE
** @@
******************************************************************************/

static ajulong btreeRebalancePriTwo(AjPBtcache cache, ajulong thisNode,
                                   ajulong leftNode, ajulong rightNode,
                                   ajulong lAnchor, ajulong rAnchor)
{
    unsigned char *lbuf = NULL;
    unsigned char *rbuf = NULL;
    unsigned char *tbuf = NULL;

    ajulong anchorNode   = 0L;
    ajulong balanceNode  = 0L;
    ajulong mergeNode    = 0L;
    ajulong done         = 0L;
    ajulong parent       = 0L;
    
    AjPBtpage lpage = NULL;
    AjPBtpage rpage = NULL;
    AjPBtpage tpage = NULL;
    
    ajuint lnkeys  = 0;
    ajuint rnkeys  = 0;
    ajuint size    = 0;
    ajuint order   = 0;
    ajuint minsize = 0;

    AjBool leftok  = ajFalse;
    AjBool rightok = ajFalse;
    
    
    /* ajDebug("In btreeRebalancePriTwo\n"); */

    if(leftNode!=BTNO_NODE && lAnchor!=BTNO_NODE)
	leftok = ajTrue;

    if(rightNode!=BTNO_NODE && rAnchor!=BTNO_NODE)
	rightok = ajTrue;

    if(!leftok && !rightok)
	return BTNO_NODE;
    

    if(leftok)
    {
	lpage = ajBtreeCacheRead(cache,leftNode);
	lbuf  = lpage->buf;
	GBT_NKEYS(lbuf,&lnkeys);
    }
    

    if(rightok)
    {
	rpage = ajBtreeCacheRead(cache,rightNode);
	rbuf  = rpage->buf;
	GBT_NKEYS(rbuf,&rnkeys);
    }
    


    if(leftok && rightok)
    {
	size = (lnkeys >= rnkeys) ? lnkeys : rnkeys;
	balanceNode = (lnkeys >= rnkeys) ? leftNode : rightNode;
    }
    else if(leftok)
    {
	size = lnkeys;
	balanceNode = leftNode;
    }
    else
    {
	size = rnkeys;
	balanceNode = rightNode;
    }

    
    order = cache->sorder;
    minsize = (order-1) / 2;

    if((order-1)%2)
	++minsize;

    if(size >= minsize)
    {
	if(leftok && rightok)
	    anchorNode = (lnkeys >= rnkeys) ? lAnchor : rAnchor;
	else if(leftok)
	    anchorNode = lAnchor;
	else
	    anchorNode = rAnchor;

	done = btreeShiftPriTwo(cache,thisNode,balanceNode,anchorNode);
    }
	    
    else
    {
	tpage = ajBtreeCacheRead(cache,thisNode);
	tbuf  = tpage->buf;
	GBT_PREV(tbuf,&parent);
	if(leftok && rightok)
	{
	    anchorNode = (parent == lAnchor) ? lAnchor : rAnchor;
	    mergeNode  = (anchorNode == lAnchor) ? leftNode : rightNode;
	}
	else if(leftok)
	{
	    anchorNode = lAnchor;
	    mergeNode  = leftNode;
	}
	else
	{
	    anchorNode = rAnchor;
	    mergeNode  = rightNode;
	}

	done = btreeMergePriTwo(cache,thisNode,mergeNode,anchorNode);
    }

    return done;
}




/* @funcstatic btreeShiftPriTwo ***********************************************
**
** Shift spare entries from one keyword tree level 2 node to another.
**
** Deletion software.
**
** @param [u] cache [AjPBtcache] cache
** @param [r] thisNode [ajulong] master node
** @param [r] balanceNode [ajulong] balance node
** @param [r] anchorNode [ajulong] anchor node
**
** @return [ajulong] page number or BTNO_NODE
** @@
******************************************************************************/

static ajulong btreeShiftPriTwo(AjPBtcache cache, ajulong thisNode,
                               ajulong balanceNode, ajulong anchorNode)
{
    unsigned char *tbuf = NULL;
    unsigned char *abuf = NULL;
    unsigned char *bbuf = NULL;
    unsigned char *buf  = NULL;
    
    AjPStr *kTarray = NULL;
    AjPStr *kAarray = NULL;
    AjPStr *kBarray = NULL;
    ajulong *pTarray = NULL;
    ajulong *pAarray = NULL;
    ajulong *pBarray = NULL;
    
    ajuint  nAkeys = 0;
    ajuint  nBkeys = 0;
    ajuint  nTkeys = 0;
    ajuint  i;
    ajint ii;

    AjPBtpage pageA = NULL;
    AjPBtpage pageB = NULL;
    AjPBtpage pageT = NULL;
    AjPBtpage page  = NULL;
    
    AjPBtpage leftpage = NULL;

    ajuint anchorPos   = 0;
    ajulong prev        = 0L;
    ajuint  nodetype    = 0;

    ajulong lv = 0L;
    
    AjPBtMem arraysA = NULL;    
    AjPBtMem arraysB = NULL;    
    AjPBtMem arraysT = NULL;    

    /* ajDebug("In btreeShiftPriTwo\n"); */


    arraysA = btreeAllocSecArray(cache);
    kAarray = arraysA->karray;
    pAarray = arraysA->parray;

    arraysB = btreeAllocSecArray(cache);
    kBarray = arraysB->karray;
    pBarray = arraysB->parray;

    arraysT = btreeAllocSecArray(cache);
    kTarray = arraysT->karray;
    pTarray = arraysT->parray;


    pageA = ajBtreeCacheRead(cache,anchorNode);
    pageA->dirty = BT_LOCK;
    pageA->lockfor = 1891;
    abuf = pageA->buf;
    pageB = ajBtreeCacheRead(cache,balanceNode);
    pageB->dirty = BT_LOCK;
    pageB->lockfor = 1892;
    bbuf = pageB->buf;
    pageT = ajBtreeCacheRead(cache,thisNode);
    pageT->dirty = BT_LOCK;
    pageT->lockfor = 1893;
    tbuf = pageT->buf;

    GBT_NKEYS(abuf,&nAkeys);
    GBT_NKEYS(bbuf,&nBkeys);
    GBT_NKEYS(tbuf,&nTkeys);

    btreeGetKeys(cache,abuf,&kAarray,&pAarray);
    btreeGetKeys(cache,bbuf,&kBarray,&pBarray);
    btreeGetKeys(cache,tbuf,&kTarray,&pTarray);
    
    if(MAJSTRCMPS(kTarray[nTkeys-1],kBarray[nBkeys-1])<0)
	leftpage = pageT;
    else
	leftpage = pageB;


    if(leftpage == pageT)
    {
	/* Find anchor key position */
	i=0;

	while(i!=nAkeys && MAJSTRCMPS(kTarray[nTkeys-1],kAarray[i])>=0)
	    ++i;

	anchorPos = i;

	/* Move down anchor key to thisNode */
	ajStrAssignS(&kTarray[nTkeys],kAarray[anchorPos]);
	++nTkeys;

	/* Shift extra */

	while(nTkeys < nBkeys)
	{
	    ajStrAssignS(&kTarray[nTkeys],kBarray[0]);
	    pTarray[nTkeys] = pBarray[0];
	    ++nTkeys;
	    --nBkeys;

	    for(i=0;i<nBkeys;++i)
	    {
		ajStrAssignS(&kBarray[i],kBarray[i+1]);
		pBarray[i] = pBarray[i+1];
	    }
	    pBarray[i] = pBarray[i+1];
	}

	/* Adjust anchor key */
	ajStrAssignS(&kAarray[anchorPos],kTarray[nTkeys-1]);
	--nTkeys;
    }
    else	/* thisNode on the right */
    {
	/* Find anchor key position */
	i=0;

	while(i!=nAkeys && MAJSTRCMPS(kBarray[nBkeys-1],kAarray[i])>=0)
	    ++i;

	anchorPos = i;

	/* Move down anchor key to thisNode */
	pTarray[nTkeys+1] = pTarray[nTkeys];

	for(ii=nTkeys-1;ii>-1;--ii)
	{
	    ajStrAssignS(&kTarray[ii+1],kTarray[ii]);
	    pTarray[ii+1] = pTarray[ii];
	}

	ajStrAssignS(&kTarray[0],kAarray[anchorPos]);
	++nTkeys;

	/* Shift extra */
	while(nTkeys < nBkeys)
	{
	    pTarray[nTkeys+1] = pTarray[nTkeys];

	    for(ii=nTkeys-1;ii>-1;--ii)
	    {
		ajStrAssignS(&kTarray[ii+1],kTarray[ii]);
		pTarray[ii+1] = pTarray[ii];
	    }
	    ajStrAssignS(&kTarray[0],kBarray[nBkeys-1]);
	    pTarray[1] = pBarray[nBkeys];
	    ++nTkeys;
	    --nBkeys;
	}


	/* Adjust anchor key */
	ajStrAssignS(&kAarray[anchorPos],kTarray[0]);
	--nTkeys;

	for(i=0;i<nTkeys;++i)
	{
	    ajStrAssignS(&kTarray[i],kTarray[i+1]);
	    pTarray[i] = pTarray[i+1];
	}
	pTarray[i] = pTarray[i+1];
    }
    

    /* Adjust PREV pointers for thisNode */
    prev = pageT->pagepos;

    for(i=0;i<nTkeys+1;++i)
    {
	page = ajBtreeCacheRead(cache,pTarray[i]);
	buf = page->buf;
	GBT_NODETYPE(buf,&nodetype);

	if(nodetype != BT_BUCKET)
	{
	    lv = prev;
	    SBT_PREV(buf,lv);
	    page->dirty = BT_DIRTY;
	}
    }

    btreeWriteNode(cache,pageA,kAarray,pAarray,nAkeys);
    btreeWriteNode(cache,pageB,kBarray,pBarray,nBkeys);
    btreeWriteNode(cache,pageT,kTarray,pTarray,nTkeys);

    if(anchorNode == cache->secrootblock)
    {
	pageA->dirty = BT_LOCK;
        pageA->lockfor = 1894;
    }

    btreeDeallocSecArray(cache,arraysA);
    btreeDeallocSecArray(cache,arraysB);
    btreeDeallocSecArray(cache,arraysT);

    return BTNO_NODE;
}




/* @funcstatic btreeMergePriTwo ***********************************************
**
** Merge two nodes.
**
** Deletion software
**
** @param [u] cache [AjPBtcache] cache
** @param [r] thisNode [ajulong] master node
** @param [r] mergeNode [ajulong] merge node
** @param [r] anchorNode [ajulong] anchor node
**
** @return [ajulong] page number or BTNO_NODE
** @@
******************************************************************************/

static ajulong btreeMergePriTwo(AjPBtcache cache, ajulong thisNode,
                                ajulong mergeNode, ajulong anchorNode)
{
    unsigned char *tbuf = NULL;
    unsigned char *abuf = NULL;
    unsigned char *nbuf = NULL;
    unsigned char *buf  = NULL;
    
    AjPStr *kTarray = NULL;
    AjPStr *kAarray = NULL;
    AjPStr *kNarray = NULL;
    ajulong *pTarray = NULL;
    ajulong *pAarray = NULL;
    ajulong *pNarray = NULL;

    ajulong thisprev  = 0L;
    ajulong mergeprev = 0L;
    
    
    ajuint  nAkeys = 0;
    ajuint  nNkeys = 0;
    ajuint  nTkeys = 0;
    ajuint  count  = 0;
    ajuint  i;
    ajint ii;

    ajuint   nodetype = 0;
    
    ajuint saveA = 0;
    ajuint saveN = 0;
    ajuint saveT = 0;
    
    AjPBtpage pageA = NULL;
    AjPBtpage pageN = NULL;
    AjPBtpage pageT = NULL;
    AjPBtpage page  = NULL;
    
    AjPBtpage leftpage = NULL;

    ajuint anchorPos = 0;
    ajulong prev      = 0L;

    ajulong lv = 0L;

    AjPBtMem arraysA = NULL;    
    AjPBtMem arraysN = NULL;    
    AjPBtMem arraysT = NULL;    

    AjBool collapse = ajFalse;
    ajulong csrb = 0L;
    
    /* ajDebug("In btreeMergePriTwo\n"); */

    pageA = ajBtreeCacheRead(cache,anchorNode);
    saveA = pageA->dirty;
    pageA->dirty = BT_LOCK;
    pageA->lockfor = 1901;
    abuf = pageA->buf;
    pageN = ajBtreeCacheRead(cache,mergeNode);
    saveN = pageN->dirty;
    pageN->dirty = BT_LOCK;
    pageN->lockfor = 1902;
    nbuf = pageN->buf;
    pageT = ajBtreeCacheRead(cache,thisNode);
    saveT = pageT->dirty;
    pageT->dirty = BT_LOCK;
    pageT->lockfor = 1903;
    tbuf = pageT->buf;

    GBT_PREV(tbuf,&thisprev);
    GBT_PREV(nbuf,&mergeprev);

    GBT_NKEYS(abuf,&nAkeys);
    GBT_NKEYS(nbuf,&nNkeys);
    GBT_NKEYS(tbuf,&nTkeys);

    GBT_NODETYPE(nbuf,&nodetype);

    csrb = cache->secrootblock;

    if(nAkeys == 1)
    {
	if(anchorNode==csrb && thisprev==csrb && mergeprev==csrb)
	    collapse = ajTrue;
	else
	{
	    pageA->dirty = saveA;
	    pageN->dirty = saveN;
	    pageT->dirty = saveT;

	    return thisNode;
	}
    }

    arraysA = btreeAllocSecArray(cache);
    kAarray = arraysA->karray;
    pAarray = arraysA->parray;

    arraysN = btreeAllocSecArray(cache);
    kNarray = arraysN->karray;
    pNarray = arraysN->parray;

    arraysT = btreeAllocSecArray(cache);
    kTarray = arraysT->karray;
    pTarray = arraysT->parray;

    btreeGetKeys(cache,abuf,&kAarray,&pAarray);
    btreeGetKeys(cache,nbuf,&kNarray,&pNarray);
    btreeGetKeys(cache,tbuf,&kTarray,&pTarray);

    if(MAJSTRCMPS(kTarray[nTkeys-1],kNarray[nNkeys-1])<0)
	leftpage = pageT;
    else
	leftpage = pageN;


    if(leftpage == pageT)
    {
	/* Find anchor key position */
	i=0;

	while(i!=nAkeys && MAJSTRCMPS(kTarray[nTkeys-1],kAarray[i])>=0)
	    ++i;

	anchorPos = i;

	/* Move down anchor key to neighbour Node */
	pNarray[nNkeys+1] = pNarray[nNkeys];

	for(ii=nNkeys-1;ii>-1;--ii)
	{
	    ajStrAssignS(&kNarray[ii+1],kNarray[ii]);
	    pNarray[ii+1] = pNarray[ii];
	}

	ajStrAssignS(&kNarray[0],kAarray[anchorPos]);
	++nNkeys;


	/* Adjust anchor node keys/ptrs */
	++anchorPos;
	if(anchorPos==nAkeys)
	    pAarray[nAkeys-1] = pAarray[nAkeys];
	else
	{
	    for(i=anchorPos;i<nAkeys;++i)
	    {
		ajStrAssignS(&kAarray[i-1],kAarray[i]);
		pAarray[i-1] = pAarray[i];
	    }
	    pAarray[i-1] = pAarray[i];
	}
	--nAkeys;
	

	/* Merge this to neighbour */

	while(nTkeys)
	{
	    pNarray[nNkeys+1] = pNarray[nNkeys];

	    for(ii=nNkeys-1;ii>-1;--ii)
	    {
		ajStrAssignS(&kNarray[ii+1],kNarray[ii]);
		pNarray[ii+1] = pNarray[ii];
	    }

	    ajStrAssignS(&kNarray[0],kTarray[nTkeys-1]);
	    pNarray[1] = pTarray[nTkeys];
	    pNarray[0] = pTarray[nTkeys-1];
	    --nTkeys;
	    ++nNkeys;
	}

	/* At this point the 'this' node could be added to a free list */
    }
    else
    {
	/* Find anchor key position */
	i=0;

	while(i!=nAkeys && MAJSTRCMPS(kNarray[nNkeys-1],kAarray[i])>=0)
	    ++i;

	anchorPos = i;

	/* Move down anchor key to neighbourNode */
	ajStrAssignS(&kNarray[nNkeys],kAarray[anchorPos]);
	++nNkeys;

	/* Adjust anchor node keys/ptrs */
	++anchorPos;

	if(anchorPos!=nAkeys)
	    for(i=anchorPos;i<nAkeys;++i)
	    {
		ajStrAssignS(&kAarray[i-1],kAarray[i]);
		pAarray[i] = pAarray[i+1];
	    }
	--nAkeys;

	/* merge extra */
	count = 0;

	while(nTkeys)
	{
	    ajStrAssignS(&kNarray[nNkeys],kTarray[count]);
	    pNarray[nNkeys] = pTarray[count];
	    ++nNkeys;
	    ++count;
	    --nTkeys;
	    pNarray[nNkeys] = pTarray[count];
	
	}

	/* At this point the 'this' node could be added to a free list */
    }
    
    
    /* Adjust PREV pointers for neighbour Node */
    prev = pageN->pagepos;

    for(i=0;i<nNkeys+1;++i)
    {
	page = ajBtreeCacheRead(cache,pNarray[i]);
	buf = page->buf;
	GBT_NODETYPE(buf,&nodetype);

	if(nodetype != BT_BUCKET)
	{
	    lv = prev;
	    SBT_PREV(buf,lv);
	    page->dirty = BT_DIRTY;
	}
    }

    pageT->dirty = BT_CLEAN;
    btreeWriteNode(cache,pageA,kAarray,pAarray,nAkeys);
    btreeWriteNode(cache,pageN,kNarray,pNarray,nNkeys);

    if(anchorNode == cache->secrootblock)
    {
	pageA->dirty = BT_LOCK;
        pageA->lockfor = 1904;
    }

    btreeDeallocSecArray(cache,arraysA);
    btreeDeallocSecArray(cache,arraysN);
    btreeDeallocSecArray(cache,arraysT);
    
    if(collapse)
	btreeCollapseRootPriTwo(cache,mergeNode);

    return thisNode;
}




/* @funcstatic btreeCollapseRootPriTwo ****************************************
**
** Collapse root page for keyword level 2 tree.
**
** Deletion software.
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number to make new root
**
** @return [ajulong] page number or BTNO_NODE
** @@
******************************************************************************/

static ajulong btreeCollapseRootPriTwo(AjPBtcache cache, ajulong pagepos)
{
    unsigned char *buf  = NULL;
    unsigned char *lbuf = NULL;

    AjPStr *karray = NULL;
    ajulong *parray = NULL;

    AjPBtpage rootpage = NULL;
    AjPBtpage page     = NULL;
    
    ajuint nodetype = 0;
    ajuint nkeys    = 0;
    ajuint i;

    ajulong prev = 0L;
    AjPBtMem arrays = NULL;
    
    /* ajDebug("In btreeCollapseRootPriTwo\n"); */
    
    if(!cache->slevel)
	return BTNO_NODE;

    rootpage = btreeCacheLocate(cache,cache->secrootblock);
    buf = rootpage->buf;
    page = ajBtreeCacheRead(cache,pagepos);


    arrays = btreeAllocSecArray(cache);
    karray = arrays->karray;
    parray = arrays->parray;

    /*
    ** Swap pagepos values to make root the child and child the root
    ** Update node types and mark the original root as a clean page
    */

    /* At this point page->pagepos could be added to a free list */

    rootpage->pagepos = page->pagepos;
    rootpage->dirty = BT_CLEAN;
    nodetype = BT_INTERNAL;
    SBT_NODETYPE(buf,nodetype);

    page->pagepos = cache->secrootblock;
    page->dirty = BT_LOCK;
    page->lockfor = 1911;
    buf = page->buf;
    nodetype = BT_ROOT;
    SBT_NODETYPE(buf,nodetype);
    
    --cache->slevel;

    if(cache->slevel)
    {
	/*
	 ** Update the PREV pointers of the new root's children
	 */
	GBT_NKEYS(buf,&nkeys);
	btreeGetKeys(cache,buf,&karray,&parray);

	for(i=0;i<nkeys+1;++i)
	{
	    page = ajBtreeCacheRead(cache,parray[i]);
	    lbuf = page->buf;
	    SBT_PREV(lbuf,prev);
	    page->dirty = BT_DIRTY;
	}
    }

    btreeDeallocSecArray(cache,arrays);
    
    return 0L;
}




/* @funcstatic btreeFindPriBalanceOne ******************************************
**
** Master routine for entry deletion from level 1 keyword tree.
**
** Deletion software.
**
** @param [u] cache [AjPBtcache] cache
** @param [r] thisNode [ajulong] Current node
** @param [r] leftNode [ajulong] Node to left
** @param [r] rightNode [ajulong] Node to right
** @param [r] lAnchor [ajulong] Left anchor
** @param [r] rAnchor [ajulong] Right anchor
** @param [r] pri [const AjPBtPri] pri
**
** @return [ajulong] page number or BTNO_NODE
** @@
******************************************************************************/

static ajulong btreeFindPriBalanceOne(AjPBtcache cache, ajulong thisNode,
                                      ajulong leftNode, ajulong rightNode,
                                      ajulong lAnchor, ajulong rAnchor,
                                      const AjPBtPri pri)
{
    unsigned char *buf  = NULL;
    unsigned char *buf1 = NULL;
    
    ajulong nextNode   = BTNO_NODE;
    ajulong nextLeft   = BTNO_NODE;
    ajulong nextRight  = BTNO_NODE;
    ajulong nextAncL   = BTNO_NODE;
    ajulong nextAncR   = BTNO_NODE;
    ajulong done       = 0L;
    
    ajuint  nkeys      = 0;
    ajuint  order      = 0;
    ajuint  minkeys    = 0;
    ajuint  i;
    ajuint  nodetype   = 0;

    ajuint n1keys      = 0;
    
    AjPBtpage page  = NULL;
    AjPBtpage page1 = NULL;

    ajulong balanceNode = 0L;
    ajulong blockno     = 0L;
    ajulong ptrSave     = 0L;

    AjPStr *karray  = NULL;
    ajulong *parray  = NULL;
    AjPStr *k1array = NULL;
    ajulong *p1array = NULL;

    AjPBtMem arrays  = NULL;
    AjPBtMem arrays1 = NULL;
    
    const AjPStr key = NULL;
    AjBool existed = ajFalse;
    
    /* ajDebug("In btreeFindPriBalanceOne\n"); */

    if(thisNode)
	page = ajBtreeCacheRead(cache,thisNode);
    else
    {   /* It's the root node of the primary hyb tree */
        /* Needs altering for secondary tree          */
	page = btreeCacheLocate(cache,thisNode);
	page->dirty = BT_LOCK;
        page->lockfor = 1921;
    }

    cache->deleted = ajFalse;

    buf = page->buf;
    GBT_NKEYS(buf,&nkeys);
    
    order = cache->order;
    /* order-1 is the number of keys in the node */
    minkeys = (order-1) / 2;

    if((order-1)%2)
	++minkeys;

    /*
    ** If thisNode contains >= minkeys then it is not a candidate
    ** for balancing
    */
    if(nkeys >= minkeys)
	balanceNode = BTNO_BALANCE;
    else
	balanceNode = page->pagepos;

    arrays  = btreeAllocPriArray(cache);
    arrays1 = btreeAllocPriArray(cache);

    karray = arrays->karray;
    parray = arrays->parray;

    k1array = arrays1->karray;
    p1array = arrays1->parray;

    key = pri->keyword;

    btreeGetKeys(cache,buf,&karray,&parray);

    i=0;

    while(i!=nkeys && MAJSTRCMPS(key,karray[i])>=0)
	++i;

    blockno = parray[i];

    nextNode = blockno;
    ptrSave = i;

    GBT_NODETYPE(buf,&nodetype);

    if(!(nodetype == BT_LEAF) && !(nodetype == BT_ROOT && !cache->level))
    {
	if(nextNode == parray[0])
	{
	    if(leftNode != BTNO_NODE)
	    {
		page1 = ajBtreeCacheRead(cache,leftNode);
		buf1 = page1->buf;
		GBT_NKEYS(buf1,&n1keys);
		btreeGetKeys(cache,buf1,&k1array,&p1array);
		nextLeft = p1array[n1keys];
	    }
	    else
		nextLeft = BTNO_NODE;
	    
	    if(!thisNode)
		nextAncL = thisNode;
	    else
		nextAncL = lAnchor;
	}
	else
	{
	    nextLeft = parray[ptrSave-1];
	    nextAncL = thisNode;
	}

	if(nextNode == parray[nkeys])
	{
	    if(rightNode != BTNO_NODE)
	    {
		page1 = ajBtreeCacheRead(cache,rightNode);
		buf1 = page1->buf;
		GBT_NKEYS(buf1,&n1keys);
		btreeGetKeys(cache,buf1,&k1array,&p1array);
		nextRight = p1array[0];
	    }
	    else
		nextRight = BTNO_NODE;

	    if(!thisNode)
		nextAncR = thisNode;
	    else
		nextAncR = rAnchor;
	}
	else
	{
	    nextRight = parray[ptrSave+1];
	    nextAncR  = thisNode;
	}



	/* Check to see whether key exists in an internal node */
	if(nodetype != BT_LEAF && cache->level)
	{
	    i=0;

	    while(i!=nkeys && MAJSTRCMPS(key,karray[i]))
		++i;

	    if(i!=nkeys)
	    {
		btreeFindPriMinOne(cache,parray[i+1],key);
		ajStrAssignS(&karray[i],cache->replace);
		btreeWriteNode(cache,page,karray,parray,nkeys);
	    }
	
	}
	
	btreeFindPriBalanceOne(cache,nextNode,nextLeft,nextRight,
                               nextAncL,nextAncR,pri);

	if(thisNode)
	    page = ajBtreeCacheRead(cache,thisNode);
	else
	{
	    page = btreeCacheLocate(cache,thisNode);
	    page->dirty = BT_LOCK;
            page->lockfor = 1922;
	}
	buf = page->buf;

    }
    else
    {
	if(nodetype == BT_LEAF || (nodetype==BT_ROOT && !cache->level))
	{
	    existed = btreeRemovePriEntryOne(cache,thisNode,pri);

	    if(existed)
		cache->deleted = ajTrue;
	    GBT_NKEYS(buf,&nkeys);

	    if(nkeys >= minkeys || (nodetype==BT_ROOT && !cache->level))
		balanceNode = BTNO_BALANCE;
	    else
		balanceNode = page->pagepos;
	}
    }


    if(balanceNode == BTNO_BALANCE || thisNode == 0L)
	done = BTNO_NODE;
    else
	done = btreeRebalancePriOne(cache,thisNode,leftNode,rightNode,
                                    lAnchor,rAnchor);
    

    btreeDeallocPriArray(cache,arrays);
    btreeDeallocPriArray(cache,arrays1);

    return done;
}




/* @funcstatic btreeFindPriMinOne *********************************************
**
** Find minimum key in keyword level 1 subtree and store in cache.
**
** Deletion software.
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page
** @param [r] key [const AjPStr] key
**
** @return [void]
** @@
******************************************************************************/

static void btreeFindPriMinOne(AjPBtcache cache, ajulong pagepos,
                               const AjPStr key)
{
    AjPBtpage page   = NULL;
    AjPPriBucket bucket = NULL;
    AjPStr *karray   = NULL;
    ajulong *parray   = NULL;

    ajuint nkeys    = 0;
    ajuint nodetype = 0;
    ajuint nentries = 0;
    ajuint i;
    AjPBtMem arrays = NULL;
    
    unsigned char *buf = NULL;

    /* ajDebug("In btreeFindPriMinOne\n"); */

    arrays = btreeAllocPriArray(cache);
    karray = arrays->karray;
    parray = arrays->parray;

    page = ajBtreeCacheRead(cache,pagepos);
    buf  = page->buf;
    GBT_NODETYPE(buf,&nodetype);
    GBT_NKEYS(buf,&nkeys);

    btreeGetKeys(cache,buf,&karray,&parray);

    if(nodetype == BT_LEAF)
    {
	bucket = btreeReadPriBucket(cache,parray[0]);
	nentries = bucket->Nentries;

        /*
        ** If there's only one entry then it must be the key marked
        ** for deletion
        */
        if(nentries<2)
	{
	    btreePriBucketDel(&bucket);
	    bucket = btreeReadPriBucket(cache,parray[1]);
	    nentries = bucket->Nentries;
	}

        /* Check for empty bucket - shouldn't happen */
        /* Checking solely out of interest */
	if(nentries<1)	
	    ajFatal("FindPriMinOne: Too few entries in bucket Nkeys=%u\n",
                    nkeys);


        /* Find lowest value key in the bucket and store in cache */
        ajStrAssignS(&cache->replace,bucket->codes[0]->keyword);

	if(!MAJSTRCMPS(cache->replace,key))
	    ajStrAssignS(&cache->replace,bucket->codes[1]->keyword);

	for(i=1;i<nentries;++i)
	    if(MAJSTRCMPS(bucket->codes[i]->keyword,cache->replace)<0 &&
	       MAJSTRCMPS(bucket->codes[i]->keyword,key))
		ajStrAssignS(&cache->replace,bucket->codes[i]->keyword);
	btreePriBucketDel(&bucket);
    }
    else
    {
	pagepos = parray[0];
	btreeFindPriMinOne(cache,pagepos,key);
	
    }

    btreeDeallocPriArray(cache,arrays);
    
    return;
}




/* @funcstatic btreeRemovePriEntryOne *****************************************
**
** Find and delete an ID from a given hybrid tree level 1 leaf node if
** necessary.
**
** Deletion software
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] leaf node page
** @param [r] pri [const AjPBtPri] keyword object
**
** @return [AjBool] True if found (and deleted)
** @@
******************************************************************************/

static AjBool btreeRemovePriEntryOne(AjPBtcache cache, ajulong pagepos,
                                     const AjPBtPri pri)
{
    AjPBtpage page   = NULL;
    AjPPriBucket bucket = NULL;
    
    AjPStr *karray = NULL;
    ajulong *parray = NULL;
    ajulong blockno = 0L;
    
    ajuint nkeys    = 0;
    ajuint nentries = 0;
    ajuint i;

    ajuint dirtysave = 0;
    
    AjBool found = ajFalse;
    const AjPStr key  = NULL;

    unsigned char *buf = NULL;
    AjPBtMem arrays = NULL;
    
    /* ajDebug("In btreeRemovePriEntryOne\n"); */

    page = ajBtreeCacheRead(cache,pagepos);
    buf = page->buf;
    dirtysave = page->dirty;
    page->dirty = BT_LOCK;
    page->lockfor = 1931;
  
    GBT_NKEYS(buf,&nkeys);

    if(!nkeys)
	return ajFalse;


    arrays = btreeAllocPriArray(cache);
    karray = arrays->karray;
    parray = arrays->parray;

    btreeGetKeys(cache,buf,&karray,&parray);
    
    key = pri->keyword;

    i=0;

    while(i!=nkeys && MAJSTRCMPS(key,karray[i])>=0)
	++i;

    blockno = parray[i];
    
    bucket = btreeReadPriBucket(cache,blockno);


    nentries = bucket->Nentries;
    found = ajFalse;

    for(i=0;i<nentries;++i)
	if(!MAJSTRCMPS(key,bucket->codes[i]->keyword))
	{
	    found = ajTrue;
	    break;
	}
    

    if(found)
    {
	/* Perform the deletion */
	if(nentries == 1)
	{
	    bucket->Nentries = 0;
	    ajBtreePriDel(&bucket->codes[0]);
	}
	else
	{
	    ajBtreePriDel(&bucket->codes[i]);
	    bucket->codes[i] = bucket->codes[nentries-1];
	    --bucket->Nentries;
	}

	btreeWritePriBucket(cache,bucket,blockno);
	btreeAdjustPriBucketsOne(cache,page);
	page->dirty = BT_DIRTY;
    }
    else
	page->dirty = dirtysave;

    btreePriBucketDel(&bucket);

    btreeDeallocPriArray(cache,arrays);
    
    if(!found)
	return ajFalse;

    return ajTrue;
}




/* @funcstatic btreeAdjustPriBucketsOne **************************************
**
** Re-order leaf buckets
** Can be called whatever the state of a leaf.
**
** Deletion software
**
** @param [u] cache [AjPBtcache] cache
** @param [u] leaf [AjPBtpage] leaf page
**
** @return [void]
** @@
******************************************************************************/

static void btreeAdjustPriBucketsOne(AjPBtcache cache, AjPBtpage leaf)
{
    ajuint nkeys = 0;
    unsigned char *lbuf = NULL;
    AjPPriBucket *buckets  = NULL;

    AjPBtMem arrays = NULL;    

    AjPStr *keys        = NULL;
    ajulong *ptrs        = NULL;
    ajulong *overflows   = NULL;
    
    ajuint i = 0;
    ajuint j = 0;
    
    ajuint order;
    ajuint bentries      = 0;
    ajuint totalkeys     = 0;
    ajuint nperbucket    = 0;
    ajuint maxnperbucket = 0;
    ajuint count         = 0;
    ajuint totkeylen     = 0;
    ajuint keylimit      = 0;
    ajuint bucketn       = 0;
    ajuint bucketlimit   = 0;
    ajuint nodetype      = 0;
    ajuint nids          = 0;
    ajuint totnids       = 0;
    
    AjPList idlist    = NULL;
    ajuint   dirtysave = 0;
    AjPBtPri bid       = NULL;
    AjPPriBucket cbucket = NULL;
    AjPBtPri cid       = NULL;

    ajuint v = 0;
    
    /* ajDebug("In btreeAdjustPriBucketsOne\n"); */

    dirtysave = leaf->dirty;

    leaf->dirty = BT_LOCK;
    leaf->lockfor = 1941;
    lbuf = leaf->buf;

    GBT_NKEYS(lbuf,&nkeys);

    if(!nkeys)
    {
	leaf->dirty = dirtysave;
	return;
    }

    GBT_NODETYPE(lbuf,&nodetype);

    order = cache->order;
    nperbucket = cache->nperbucket;
    

    /* Read keys/ptrs */

    arrays = btreeAllocPriArray(cache);
    keys = arrays->karray;
    ptrs = arrays->parray;
    overflows = arrays->overflows;

    btreeGetPointers(cache,lbuf,&ptrs);


    for(i=0;i<nkeys;++i)
	totalkeys += btreeNumInPriBucket(cache,ptrs[i]);

    totalkeys += btreeNumInPriBucket(cache,ptrs[i]);


    /* Set the number of entries per bucket to approximately half full */
    maxnperbucket = nperbucket >> 1;

    if(!maxnperbucket)
	++maxnperbucket;

    if(!leaf->pagepos)
	maxnperbucket = nperbucket;

    /* Work out the number of new buckets needed */
    bucketn = (totalkeys / maxnperbucket);

    if(totalkeys % maxnperbucket)
	++bucketn;

    if(bucketn == 1)
	++bucketn;

    while(bucketn > order)
    {
        ++maxnperbucket;
        bucketn = (totalkeys / maxnperbucket);

        if(totalkeys % maxnperbucket)
            ++bucketn;
    }
    

    /* Read buckets */
    AJCNEW0(buckets,nkeys+1);
    keylimit = nkeys + 1;
    
    for(i=0;i<keylimit;++i)
	buckets[i] = btreeReadPriBucket(cache,ptrs[i]);


    /* Read IDs from all buckets and push to list and sort (increasing id) */
    idlist  = ajListNew();

    for(i=0;i<keylimit;++i)
    {
	overflows[i] = buckets[i]->Overflow;
	bentries = buckets[i]->Nentries;

	for(j=0;j<bentries;++j)
	    ajListPush(idlist,(void *)buckets[i]->codes[j]);
	
	AJFREE(buckets[i]->keylen);
	AJFREE(buckets[i]->codes);
	AJFREE(buckets[i]);
    }
    ajListSort(idlist,btreeKeywordCompare);
    AJFREE(buckets);

    cbucket = btreePriBucketNew(maxnperbucket);
    bucketlimit = bucketn - 1;

    totnids = 0;
    nids = ajListGetLength(idlist);


    if(!totalkeys)
    {
	v = totalkeys;
	SBT_NKEYS(lbuf,v);

        btreeDeallocPriArray(cache,arrays);

	ajListFree(&idlist);
	leaf->dirty = BT_DIRTY;

	return;
    }
    
    if(nids <= maxnperbucket)
    {
	cbucket->Overflow = overflows[1];
	cbucket->Nentries = 0;
	ajListPeek(idlist,(void **)&bid);
	ajStrAssignS(&keys[0],bid->keyword);

	count = 0;

	while(count!=maxnperbucket && totnids != nids)
	{
	    ajListPop(idlist,(void **)&bid);
	    
	    cid = cbucket->codes[count];
	    ajStrAssignS(&cid->keyword,bid->keyword);
	    cid->treeblock = bid->treeblock;
	    
	    cbucket->keylen[count] = BT_BUCKPRILEN(bid->keyword);
	    ++cbucket->Nentries;
	    ++count;
	    ++totnids;
	    ajBtreePriDel(&bid);
	}


	totkeylen += ajStrGetLen(keys[0]);

	if(!ptrs[1])
	    ptrs[1] = cache->totsize;
	btreeWritePriBucket(cache,cbucket,ptrs[1]);

	cbucket->Overflow = overflows[0];
	cbucket->Nentries = 0;
	if(!ptrs[0])
	    ptrs[0] = cache->totsize;
	btreeWritePriBucket(cache,cbucket,ptrs[0]);
    }
    else
    {
	for(i=0;i<bucketlimit;++i)
	{
	    cbucket->Overflow = overflows[i];
	    cbucket->Nentries = 0;

	    count = 0;

	    while(count!=maxnperbucket && totnids != nids)
	    {
		ajListPop(idlist,(void **)&bid);

		cid = cbucket->codes[count];
		ajStrAssignS(&cid->keyword,bid->keyword);
		cid->treeblock = bid->treeblock;
		
		cbucket->keylen[count] = BT_BUCKPRILEN(bid->id);
		++cbucket->Nentries;
		++count;
		ajBtreePriDel(&bid);
	    }


	    ajListPeek(idlist,(void **)&bid);
	    ajStrAssignS(&keys[i],bid->keyword);


	    totkeylen += ajStrGetLen(bid->keyword);

	    if(!ptrs[i])
		ptrs[i] = cache->totsize;

	    btreeWritePriBucket(cache,cbucket,ptrs[i]);
	}
	
	
	/* Deal with greater-than bucket */
	
	cbucket->Overflow = overflows[i];
	cbucket->Nentries = 0;
	
	
	
	count = 0;

	while(ajListPop(idlist,(void **)&bid))
	{
	    cid = cbucket->codes[count];
	    ajStrAssignS(&cid->keyword,bid->keyword);
	    cid->treeblock = bid->treeblock;
	    
	    ++cbucket->Nentries;
	    ++count;
	    ajBtreePriDel(&bid);
	}
	
	
	if(!ptrs[i])
	    ptrs[i] = cache->totsize;
	
	btreeWritePriBucket(cache,cbucket,ptrs[i]);
    }
    

    cbucket->Nentries = maxnperbucket;
    btreePriBucketDel(&cbucket);

    /* Now write out a modified leaf with new keys/ptrs */

    nkeys = bucketn - 1;
    v = nkeys;
    SBT_NKEYS(lbuf,v);
    v = totkeylen;
    SBT_TOTLEN(lbuf,v);

    btreeWriteNode(cache,leaf,keys,ptrs,nkeys);

    leaf->dirty = dirtysave;

    if(nodetype == BT_ROOT)
    {
	leaf->dirty = BT_LOCK;
        leaf->lockfor = 1942;
    }

    btreeDeallocPriArray(cache,arrays);

    btreePriBucketDel(&cbucket);
    ajListFree(&idlist);

    return;
}




/* @funcstatic btreeRebalancePriOne ******************************************
**
** Rebalance keyword level 1 tree after deletion
**
** Deletion software
**
** @param [u] cache [AjPBtcache] cache
** @param [r] thisNode [ajulong] Node to rebalance
** @param [r] leftNode [ajulong] left node
** @param [r] rightNode [ajulong] right node
** @param [r] lAnchor [ajulong] left anchor
** @param [r] rAnchor [ajulong] right anchor
**
** @return [ajulong] page number or BTNO_NODE
** @@
******************************************************************************/

static ajulong btreeRebalancePriOne(AjPBtcache cache, ajulong thisNode,
                                    ajulong leftNode, ajulong rightNode,
                                    ajulong lAnchor, ajulong rAnchor)
{
    unsigned char *lbuf = NULL;
    unsigned char *rbuf = NULL;
    unsigned char *tbuf = NULL;

    ajulong anchorNode   = 0L;
    ajulong balanceNode  = 0L;
    ajulong mergeNode    = 0L;
    ajulong done         = 0L;
    ajulong parent       = 0L;
    
    AjPBtpage lpage = NULL;
    AjPBtpage rpage = NULL;
    AjPBtpage tpage = NULL;
    
    ajuint lnkeys  = 0;
    ajuint rnkeys  = 0;
    ajuint size    = 0;
    ajuint order   = 0;
    ajuint minsize = 0;

    AjBool leftok  = ajFalse;
    AjBool rightok = ajFalse;
    
    
    /* ajDebug("In btreeRebalancePriOne\n"); */

    if(leftNode!=BTNO_NODE && lAnchor!=BTNO_NODE)
	leftok = ajTrue;

    if(rightNode!=BTNO_NODE && rAnchor!=BTNO_NODE)
	rightok = ajTrue;

    if(!leftok && !rightok)
	return BTNO_NODE;
    

    if(leftok)
    {
	lpage = ajBtreeCacheRead(cache,leftNode);
	lbuf  = lpage->buf;
	GBT_NKEYS(lbuf,&lnkeys);
    }
    

    if(rightok)
    {
	rpage = ajBtreeCacheRead(cache,rightNode);
	rbuf  = rpage->buf;
	GBT_NKEYS(rbuf,&rnkeys);
    }
    


    if(leftok && rightok)
    {
	size = (lnkeys >= rnkeys) ? lnkeys : rnkeys;
	balanceNode = (lnkeys >= rnkeys) ? leftNode : rightNode;
    }
    else if(leftok)
    {
	size = lnkeys;
	balanceNode = leftNode;
    }
    else
    {
	size = rnkeys;
	balanceNode = rightNode;
    }

    
    order = cache->order;
    minsize = (order-1) / 2;

    if((order-1)%2)
	++minsize;

    if(size >= minsize)
    {
	if(leftok && rightok)
	    anchorNode = (lnkeys >= rnkeys) ? lAnchor : rAnchor;
	else if(leftok)
	    anchorNode = lAnchor;
	else
	    anchorNode = rAnchor;

	done = btreeShiftPriOne(cache,thisNode,balanceNode,anchorNode);
    }
	    
    else
    {
	tpage = ajBtreeCacheRead(cache,thisNode);
	tbuf  = tpage->buf;
	GBT_PREV(tbuf,&parent);

	if(leftok && rightok)
	{
	    anchorNode = (parent == lAnchor) ? lAnchor : rAnchor;
	    mergeNode  = (anchorNode == lAnchor) ? leftNode : rightNode;
	}
	else if(leftok)
	{
	    anchorNode = lAnchor;
	    mergeNode  = leftNode;
	}
	else
	{
	    anchorNode = rAnchor;
	    mergeNode  = rightNode;
	}

	done = btreeMergePriOne(cache,thisNode,mergeNode,anchorNode);
    }

    return done;
}




/* @funcstatic btreeShiftPriOne ***********************************************
**
** Shift spare entries from one keyword tree level 1 node to another.
** Same as btreeShiftHybOne but duplicated for clarity
**
** Deletion software.
**
** @param [u] cache [AjPBtcache] cache
** @param [r] thisNode [ajulong] master node
** @param [r] balanceNode [ajulong] balance node
** @param [r] anchorNode [ajulong] anchor node
**
** @return [ajulong] page number or BTNO_NODE
** @@
******************************************************************************/

static ajulong btreeShiftPriOne(AjPBtcache cache, ajulong thisNode,
                               ajulong balanceNode, ajulong anchorNode)
{
    unsigned char *tbuf = NULL;
    unsigned char *abuf = NULL;
    unsigned char *bbuf = NULL;
    unsigned char *buf  = NULL;
    
    AjPBtMem arraysA = NULL;    
    AjPBtMem arraysB = NULL;    
    AjPBtMem arraysT = NULL;    

    AjPStr *kTarray = NULL;
    AjPStr *kAarray = NULL;
    AjPStr *kBarray = NULL;
    ajulong *pTarray = NULL;
    ajulong *pAarray = NULL;
    ajulong *pBarray = NULL;
    
    ajuint  nAkeys = 0;
    ajuint  nBkeys = 0;
    ajuint  nTkeys = 0;
    ajuint  i;
    ajint   ii;
    
    AjPBtpage pageA = NULL;
    AjPBtpage pageB = NULL;
    AjPBtpage pageT = NULL;
    AjPBtpage page  = NULL;
    
    AjPBtpage leftpage = NULL;

    ajuint anchorPos   = 0;
    ajulong prev        = 0L;
    ajuint  nodetype    = 0;

    ajulong lv = 0L;

    /* ajDebug("In btreeShiftPriOne\n"); */


    arraysA = btreeAllocPriArray(cache);
    kAarray = arraysA->karray;
    pAarray = arraysA->parray;

    arraysB = btreeAllocPriArray(cache);
    kBarray = arraysB->karray;
    pBarray = arraysB->parray;

    arraysT = btreeAllocPriArray(cache);
    kTarray = arraysT->karray;
    pTarray = arraysT->parray;


    pageA = ajBtreeCacheRead(cache,anchorNode);
    pageA->dirty = BT_LOCK;
    pageA->lockfor = 1951;
    abuf = pageA->buf;
    pageB = ajBtreeCacheRead(cache,balanceNode);
    pageB->dirty = BT_LOCK;
    pageB->lockfor = 1952;
    bbuf = pageB->buf;
    pageT = ajBtreeCacheRead(cache,thisNode);
    pageT->dirty = BT_LOCK;
    pageT->lockfor = 1953;
    tbuf = pageT->buf;

    GBT_NKEYS(abuf,&nAkeys);
    GBT_NKEYS(bbuf,&nBkeys);
    GBT_NKEYS(tbuf,&nTkeys);

    btreeGetKeys(cache,abuf,&kAarray,&pAarray);
    btreeGetKeys(cache,bbuf,&kBarray,&pBarray);
    btreeGetKeys(cache,tbuf,&kTarray,&pTarray);
    
    if(MAJSTRCMPS(kTarray[nTkeys-1],kBarray[nBkeys-1])<0)
	leftpage = pageT;
    else
	leftpage = pageB;


    if(leftpage == pageT)
    {
	/* Find anchor key position */
	i=0;

	while(i!=nAkeys && MAJSTRCMPS(kTarray[nTkeys-1],kAarray[i])>=0)
	    ++i;

	anchorPos = i;

	/* Move down anchor key to thisNode */
	ajStrAssignS(&kTarray[nTkeys],kAarray[anchorPos]);
	++nTkeys;

	/* Shift extra */
	while(nTkeys < nBkeys)
	{
	    ajStrAssignS(&kTarray[nTkeys],kBarray[0]);
	    pTarray[nTkeys] = pBarray[0];
	    ++nTkeys;
	    --nBkeys;

	    for(i=0;i<nBkeys;++i)
	    {
		ajStrAssignS(&kBarray[i],kBarray[i+1]);
		pBarray[i] = pBarray[i+1];
	    }

	    pBarray[i] = pBarray[i+1];
	}

	/* Adjust anchor key */
	ajStrAssignS(&kAarray[anchorPos],kTarray[nTkeys-1]);
	--nTkeys;
    }
    else	/* thisNode on the right */
    {
	/* Find anchor key position */
	i=0;

	while(i!=nAkeys && MAJSTRCMPS(kBarray[nBkeys-1],kAarray[i])>=0)
	    ++i;

	anchorPos = i;

	/* Move down anchor key to thisNode */
	pTarray[nTkeys+1] = pTarray[nTkeys];

	for(ii=nTkeys-1;ii>-1;--ii)
	{
	    ajStrAssignS(&kTarray[ii+1],kTarray[ii]);
	    pTarray[ii+1] = pTarray[ii];
	}

	ajStrAssignS(&kTarray[0],kAarray[anchorPos]);
	++nTkeys;

	/* Shift extra */
	while(nTkeys < nBkeys)
	{
	    pTarray[nTkeys+1] = pTarray[nTkeys];

	    for(ii=nTkeys-1;ii>-1;--ii)
	    {
		ajStrAssignS(&kTarray[ii+1],kTarray[ii]);
		pTarray[ii+1] = pTarray[ii];
	    }

	    ajStrAssignS(&kTarray[0],kBarray[nBkeys-1]);
	    pTarray[1] = pBarray[nBkeys];
	    ++nTkeys;
	    --nBkeys;
	}


	/* Adjust anchor key */
	ajStrAssignS(&kAarray[anchorPos],kTarray[0]);
	--nTkeys;

	for(i=0;i<nTkeys;++i)
	{
	    ajStrAssignS(&kTarray[i],kTarray[i+1]);
	    pTarray[i] = pTarray[i+1];
	}

	pTarray[i] = pTarray[i+1];
    }
    

    /* Adjust PREV pointers for thisNode */
    prev = pageT->pagepos;

    for(i=0;i<nTkeys+1;++i)
    {
	page = ajBtreeCacheRead(cache,pTarray[i]);
	buf = page->buf;
	GBT_NODETYPE(buf,&nodetype);
	if(nodetype != BT_BUCKET)
	{
	    lv = prev;
	    SBT_PREV(buf,lv);
	    page->dirty = BT_DIRTY;
	}
    }

    btreeWriteNode(cache,pageA,kAarray,pAarray,nAkeys);
    btreeWriteNode(cache,pageB,kBarray,pBarray,nBkeys);
    btreeWriteNode(cache,pageT,kTarray,pTarray,nTkeys);

    if(!anchorNode)
    {
	pageA->dirty = BT_LOCK;
        pageA->lockfor = 1954;
    }

    btreeDeallocPriArray(cache,arraysA);
    btreeDeallocPriArray(cache,arraysB);
    btreeDeallocPriArray(cache,arraysT);

    return BTNO_NODE;
}




/* @funcstatic btreeMergePriOne ***********************************************
**
** Merge two nodes.
**
** Deletion software
**
** @param [u] cache [AjPBtcache] cache
** @param [r] thisNode [ajulong] master node
** @param [r] mergeNode [ajulong] merge node
** @param [r] anchorNode [ajulong] anchor node
**
** @return [ajulong] page number or BTNO_NODE
** @@
******************************************************************************/

static ajulong btreeMergePriOne(AjPBtcache cache, ajulong thisNode,
                                ajulong mergeNode, ajulong anchorNode)
{
    unsigned char *tbuf = NULL;
    unsigned char *abuf = NULL;
    unsigned char *nbuf = NULL;
    unsigned char *buf  = NULL;
    
    AjPStr *kTarray = NULL;
    AjPStr *kAarray = NULL;
    AjPStr *kNarray = NULL;
    ajulong *pTarray = NULL;
    ajulong *pAarray = NULL;
    ajulong *pNarray = NULL;

    ajulong thisprev  = 0L;
    ajulong mergeprev = 0L;
    
    
    ajuint  nAkeys = 0;
    ajuint  nNkeys = 0;
    ajuint  nTkeys = 0;
    ajuint  count  = 0;
    ajuint  i;
    ajint   ii;

    ajuint   nodetype = 0;
    
    ajuint saveA = 0;
    ajuint saveN = 0;
    ajuint saveT = 0;
    
    AjPBtpage pageA = NULL;
    AjPBtpage pageN = NULL;
    AjPBtpage pageT = NULL;
    AjPBtpage page  = NULL;
    
    AjPBtpage leftpage = NULL;

    ajuint anchorPos = 0;
    ajulong prev      = 0L;

    ajulong lv = 0L;

    AjPBtMem arraysA = NULL;    
    AjPBtMem arraysN = NULL;    
    AjPBtMem arraysT = NULL;    

    AjBool collapse = ajFalse;
    
    /* ajDebug("In btreeMergePriOne\n"); */

    pageA = ajBtreeCacheRead(cache,anchorNode);
    saveA = pageA->dirty;
    pageA->dirty = BT_LOCK;
    pageA->lockfor = 1961;
    abuf = pageA->buf;
    pageN = ajBtreeCacheRead(cache,mergeNode);
    saveN = pageN->dirty;
    pageN->dirty = BT_LOCK;
    pageN->lockfor = 1962;
    nbuf = pageN->buf;
    pageT = ajBtreeCacheRead(cache,thisNode);
    saveT = pageT->dirty;
    pageT->dirty = BT_LOCK;
    pageT->lockfor = 1963;
    tbuf = pageT->buf;

    GBT_PREV(tbuf,&thisprev);
    GBT_PREV(nbuf,&mergeprev);

    GBT_NKEYS(abuf,&nAkeys);
    GBT_NKEYS(nbuf,&nNkeys);
    GBT_NKEYS(tbuf,&nTkeys);

    GBT_NODETYPE(nbuf,&nodetype);


    if(nAkeys == 1)
    {
	if(!anchorNode && !thisprev && !mergeprev)
	    collapse = ajTrue;
	else
	{
	    pageA->dirty = saveA;
	    pageN->dirty = saveN;
	    pageT->dirty = saveT;

	    return thisNode;
	}
    }

    arraysA = btreeAllocPriArray(cache);
    kAarray = arraysA->karray;
    pAarray = arraysA->parray;

    arraysN = btreeAllocPriArray(cache);
    kNarray = arraysN->karray;
    pNarray = arraysN->parray;

    arraysT = btreeAllocPriArray(cache);
    kTarray = arraysT->karray;
    pTarray = arraysT->parray;

    btreeGetKeys(cache,abuf,&kAarray,&pAarray);
    btreeGetKeys(cache,nbuf,&kNarray,&pNarray);
    btreeGetKeys(cache,tbuf,&kTarray,&pTarray);

    if(MAJSTRCMPS(kTarray[nTkeys-1],kNarray[nNkeys-1])<0)
	leftpage = pageT;
    else
	leftpage = pageN;


    if(leftpage == pageT)
    {
	/* Find anchor key position */
	i=0;

	while(i!=nAkeys && MAJSTRCMPS(kTarray[nTkeys-1],kAarray[i])>=0)
	    ++i;

	anchorPos = i;

	/* Move down anchor key to neighbour Node */
	pNarray[nNkeys+1] = pNarray[nNkeys];

	for(ii=nNkeys-1;ii>-1;--ii)
	{
	    ajStrAssignS(&kNarray[ii+1],kNarray[ii]);
	    pNarray[ii+1] = pNarray[ii];
	}

	ajStrAssignS(&kNarray[0],kAarray[anchorPos]);
	++nNkeys;


	/* Adjust anchor node keys/ptrs */
	++anchorPos;

	if(anchorPos==nAkeys)
	    pAarray[nAkeys-1] = pAarray[nAkeys];
	else
	{
	    for(i=anchorPos;i<nAkeys;++i)
	    {
		ajStrAssignS(&kAarray[i-1],kAarray[i]);
		pAarray[i-1] = pAarray[i];
	    }
	    pAarray[i-1] = pAarray[i];
	}

	--nAkeys;
	

	/* Merge this to neighbour */

	while(nTkeys)
	{
	    pNarray[nNkeys+1] = pNarray[nNkeys];

	    for(ii=nNkeys-1;ii>-1;--ii)
	    {
		ajStrAssignS(&kNarray[ii+1],kNarray[ii]);
		pNarray[ii+1] = pNarray[ii];
	    }

	    ajStrAssignS(&kNarray[0],kTarray[nTkeys-1]);
	    pNarray[1] = pTarray[nTkeys];
	    pNarray[0] = pTarray[nTkeys-1];
	    --nTkeys;
	    ++nNkeys;
	}

	/* At this point the 'this' node could be added to a free list */
    }
    else
    {
	/* Find anchor key position */
	i=0;

	while(i!=nAkeys && MAJSTRCMPS(kNarray[nNkeys-1],kAarray[i])>=0)
	    ++i;

	anchorPos = i;

	/* Move down anchor key to neighbourNode */
	ajStrAssignS(&kNarray[nNkeys],kAarray[anchorPos]);
	++nNkeys;

	/* Adjust anchor node keys/ptrs */
	++anchorPos;

	if(anchorPos!=nAkeys)
	    for(i=anchorPos;i<nAkeys;++i)
	    {
		ajStrAssignS(&kAarray[i-1],kAarray[i]);
		pAarray[i] = pAarray[i+1];
	    }

	--nAkeys;

	/* merge extra */
	count = 0;

	while(nTkeys)
	{
	    ajStrAssignS(&kNarray[nNkeys],kTarray[count]);
	    pNarray[nNkeys] = pTarray[count];
	    ++nNkeys;
	    ++count;
	    --nTkeys;
	    pNarray[nNkeys] = pTarray[count];
	
	}

	/* At this point the 'this' node could be added to a free list */
    }
    
    
    /* Adjust PREV pointers for neighbour Node */
    prev = pageN->pagepos;

    for(i=0;i<nNkeys+1;++i)
    {
	page = ajBtreeCacheRead(cache,pNarray[i]);
	buf = page->buf;
	GBT_NODETYPE(buf,&nodetype);

	if(nodetype != BT_BUCKET)
	{
	    lv = prev;
	    SBT_PREV(buf,lv);
	    page->dirty = BT_DIRTY;
	}
    }

    pageT->dirty = BT_CLEAN;
    btreeWriteNode(cache,pageA,kAarray,pAarray,nAkeys);
    btreeWriteNode(cache,pageN,kNarray,pNarray,nNkeys);

    if(!anchorNode)
    {
	pageA->dirty = BT_LOCK;
        pageA->lockfor = 1964;
    }

    btreeDeallocPriArray(cache,arraysA);
    btreeDeallocPriArray(cache,arraysN);
    btreeDeallocPriArray(cache,arraysT);
    
    if(collapse)
	btreeCollapseRootPriOne(cache,mergeNode);

    return thisNode;
}




/* @funcstatic btreeCollapseRootPriOne ****************************************
**
** Collapse root page for keyword level 1 tree.
**
** Deletion software.
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number to make new root
**
** @return [ajulong] page number or BTNO_NODE
** @@
******************************************************************************/

static ajulong btreeCollapseRootPriOne(AjPBtcache cache, ajulong pagepos)
{
    unsigned char *buf  = NULL;
    unsigned char *lbuf = NULL;

    AjPStr *karray = NULL;
    ajulong *parray = NULL;

    AjPBtpage rootpage = NULL;
    AjPBtpage page     = NULL;
    
    ajuint nodetype = 0;
    ajuint nkeys    = 0;
    ajuint i;

    ajulong prev = 0L;
    AjPBtMem arrays = NULL;
    
    /* ajDebug("In btreeCollapseRootPriOne\n"); */
    
    if(!cache->level)
	return BTNO_NODE;

    rootpage = btreeCacheLocate(cache,0L);
    buf = rootpage->buf;
    page = ajBtreeCacheRead(cache,pagepos);


    arrays = btreeAllocPriArray(cache);
    karray = arrays->karray;
    parray = arrays->parray;

    /*
    ** Swap pagepos values to make root the child and child the root
    ** Update node types and mark the original root as a clean page
    */

    /* At this point page->pagepos could be added to a free list */

    rootpage->pagepos = page->pagepos;
    rootpage->dirty = BT_CLEAN;
    nodetype = BT_INTERNAL;
    SBT_NODETYPE(buf,nodetype);

    page->pagepos = 0;
    page->dirty = BT_LOCK;
    page->lockfor = 1971;
    buf = page->buf;
    nodetype = BT_ROOT;
    SBT_NODETYPE(buf,nodetype);
    
    --cache->level;

    if(cache->level)
    {
	/*
	 ** Update the PREV pointers of the new root's children
	 */
	GBT_NKEYS(buf,&nkeys);
	btreeGetKeys(cache,buf,&karray,&parray);

	for(i=0;i<nkeys+1;++i)
	{
	    page = ajBtreeCacheRead(cache,parray[i]);
	    lbuf = page->buf;
	    SBT_PREV(lbuf,prev);
	    page->dirty = BT_DIRTY;
	}
    }

    btreeDeallocPriArray(cache,arrays);
    
    return 0L;
}




/* @funcstatic btreeIsSecEmpty ************************************************
**
** Tests whether a secondary tree is empty
**
** Deletion software.
**
** @param [u] cache [AjPBtcache] cache
**
** @return [AjBool] true if empty, false otherwise
** @@
******************************************************************************/

static AjBool btreeIsSecEmpty(AjPBtcache cache)
{
    AjPBtpage rootpage  = NULL;
    AjPSecBucket bucket = NULL;
    unsigned char *buf  = NULL;
    AjPStr *karray  = NULL;
    ajulong *parray  = NULL;
    AjPBtMem arrays = NULL;
    ajuint nkeys = 0;
    ajuint tkeys = 0;

    if(cache->slevel)
        return ajFalse;

    rootpage = btreeCacheLocate(cache,cache->secrootblock);

    if(!rootpage)
        ajFatal("btreeSecIsEmpty: root page unlocked");

    buf = rootpage->buf;
    GBT_NKEYS(buf,&nkeys);

    if(!nkeys)
        return ajTrue;

    if(nkeys > 1)
        return ajFalse;

    arrays = btreeAllocSecArray(cache);
    karray = arrays->karray;
    parray = arrays->parray;
    btreeGetKeys(cache,buf,&karray,&parray);

    tkeys = 0;

    if(parray[0])
    {
        bucket = btreeReadSecBucket(cache,parray[0]);
        tkeys += bucket->Nentries;
    }

    if(parray[1])
    {
        bucket = btreeReadSecBucket(cache,parray[1]);
        tkeys += bucket->Nentries;
    }

    btreeDeallocSecArray(cache,arrays);

    if(tkeys)
        return ajFalse;

    return ajTrue;
}




/* @funcstatic btreeNodetype *************************************************
**
** Returns a name for the nodetype of a buffer
**
** @param [r] buf [const unsigned char*] buffer
** @return [const char*] Name of nodetype
** @@
******************************************************************************/

static const char* btreeNodetype(const unsigned char* buf)
{
    static AjBool called = AJFALSE;

    ajuint nodetype;

    if(!called)
    {
        AJCNEW0(btreeNodetypeNames, 512);
        btreeNodetypeNames[BT_FREEPAGE]  = "free";
        btreeNodetypeNames[BT_ROOT]      = "root";
        btreeNodetypeNames[BT_INTERNAL]  = "internal";
        btreeNodetypeNames[BT_LEAF]      = "leaf";
        btreeNodetypeNames[BT_BUCKET]    = "bucket";
        btreeNodetypeNames[BT_OVERFLOW]  = "overflow";
        btreeNodetypeNames[BT_PRIBUCKET] = "pribucket";
        btreeNodetypeNames[BT_SECBUCKET] = "secbucket";
        btreeNodetypeNames[BT_NUMBUCKET] = "numbucket";
        called = ajTrue;
    }

    GBT_NODETYPE(buf, &nodetype);

    if(nodetype >= 256)
        return "unknown";
    if(!btreeNodetypeNames[nodetype])
        return "unknown";
    if(nodetype == BT_OVERFLOW)
    {
        ajWarn("Overflow page found");
        ajUtilCatch();
    }
    

    return btreeNodetypeNames[nodetype];
}




/* @func ajBtreeGetPagecount **************************************************
**
** Returns the number of pages in a tree index
**
** @param [r] cache [const AjPBtcache] cache
**
** @return [ajulong] number of pages in a tree index
** @@
******************************************************************************/

ajulong ajBtreeGetPagecount(const AjPBtcache cache)
{
    if(!cache->pagesize)
        return cache->totsize/(ajulong) BT_PAGESIZE;

    return cache->totsize/(ajulong) cache->pagesize;
}




/* @func ajBtreeGetPagesize **************************************************
**
** Returns the page size a tree index
**
** @param [r] cache [const AjPBtcache] cache
**
** @return [ajuint] Page size of a tree index
** @@
******************************************************************************/

ajuint ajBtreeGetPagesize(const AjPBtcache cache)
{
    return cache->pagesize;
}




/* @func ajBtreeGetTotsize **************************************************
**
** Returns the number of bytes in a tree index
**
** @param [r] cache [const AjPBtcache] cache
**
** @return [ajulong] number of bytes in a tree index
** @@
******************************************************************************/

ajulong ajBtreeGetTotsize(const AjPBtcache cache)
{
    return cache->totsize;
}




/* @func ajBtreePageGetTypename ********************************************
**
** Returns the node type name of a tree index page
**
** @param [r] page [const AjPBtpage] Page
**
** @return [const char*] Name of page type
** @@
******************************************************************************/

const char* ajBtreePageGetTypename(const AjPBtpage page)
{
    return btreeNodetype(page->buf);
}




/* @func ajBtreeStatNode *******************************************************
**
** Returns number of keys, overflow pages visited and free space unused
** for a b+ tree node page
**
** @param [w] cache [AjPBtcache] cache
** @param [r] page [const AjPBtpage] Page
** @param [r] full [AjBool] If true, write detailed debug report
** @param [w] nkeys [ajuint*] Number of keys
** @param [w] overflows [ajuint*] Number of overflow pages
** @param [w] freespace [ajuint*] Unused space in this page
** @param [u] refs [ajulong*] References to pages
** @param [r] newpostable [const AjPTable] Page numbers for page offsets
** @return [AjBool] True on success
** @@
******************************************************************************/

AjBool ajBtreeStatNode(AjPBtcache cache, const AjPBtpage page, AjBool full,
                       ajuint* nkeys, ajuint* overflows,
                       ajuint* freespace, ajulong* refs,
                       const AjPTable newpostable)
{
    ajuint i;
    ajuint m;

    ajuint nodetype;
    ajuint klen;
    const AjPBtpage bpage;
    unsigned char *tbuf;
    ajuint freebytes;

    ajuint    pagesize = 0;
    ajulong   overflow = 0L;
    ajulong blockno;
    ajuint totlen;
    ajulong left;
    ajulong right;
    ajulong prev;
    unsigned char *lenptr = NULL;
    unsigned char *keyptr = NULL;
    ajulong lv;
    unsigned char* kp;

    pagesize = cache->pagesize;

    bpage = page;
    tbuf = bpage->buf;
    GBT_TOTLEN(tbuf,&totlen);

    if(!totlen)
      return ajBtreeStatNumnode(cache, page, full,
                                nkeys, overflows, freespace, refs,
                                newpostable);
    
    btreeCheckNode(cache, page);

    GBT_NKEYS(tbuf,&m);
    *nkeys = m;
    *overflows = 0;
    *freespace = 0;

    GBT_OVERFLOW(tbuf,&overflow);

    GBT_LEFT(tbuf,&left);
    GBT_RIGHT(tbuf,&right);
    GBT_PREV(tbuf,&prev);
    GBT_BLOCKNUMBER(tbuf,&blockno);

    if(full)
        ajDebug("\n"
                "StatNode      %12Lu totlen:%u '%s'\n"
                "         left:%12Lu  right:%12Lu  prev:%12Lu\n"
                "        block:%12Lu   over:%12Lu\n",
                page->pagepos, totlen, btreeNodetype(tbuf),
                left, right, prev, blockno, overflow);
    if(full)
        ajDebug("\nStatNode      %12Lu      pointer (%u)\n",
                page->pagepos, m);
    if(full)
        ajDebug("--------      ------------      -------\n");

    lenptr =  PBT_KEYLEN(tbuf);
    keyptr = lenptr + m * sizeof(ajuint);

    for(i=0;i<m;i++)
    {
	BT_GETAJUINT(lenptr,&klen);

	if((ajuint)((keyptr-tbuf+2) + klen + sizeof(ajulong)) > pagesize)
	{
            freebytes = pagesize - (keyptr-tbuf);
            *freespace += freebytes;
            (*overflows)++;
    	    /* ajDebug("btreeStatNode: Overflow\n"); */
	    bpage = ajBtreeCacheRead(cache,overflow);
	    tbuf = bpage->buf;
	    GBT_NODETYPE(tbuf,&nodetype);

	    if(nodetype != BT_OVERFLOW)
		ajFatal("StatNode Overflow node %Lu expected but not found "
                        "cache %S page %Lu "
                        "pagepos:%Lu key:%u/%u free:%u keyptr:%x lenptr:%x "
                        "tbuf:%x klen:%u old nodetype '%s' newnodetype '%s' "
                        "blockno:%Lu totlen:%u left:%Lu right:%Lu prev:%Lu",
                        overflow, cache->filename,
                        page->pagepos, page->pagepos/pagesize,
                        i, m, freebytes,
                        keyptr, lenptr, tbuf, klen,
                        btreeNodetype(page->buf), btreeNodetype(tbuf),
                        blockno,totlen, left, right, prev);

            GBT_BLOCKNUMBER(tbuf,&blockno);
            GBT_TOTLEN(tbuf,&totlen);
            GBT_LEFT(tbuf,&left);
            GBT_RIGHT(tbuf,&right);
            GBT_PREV(tbuf,&prev);

	    /*
	     ** The length pointer is restricted to the initial page.
	     ** The keyptr in overflow pages starts at the Key Lengths
	     ** position!
	     */
            keyptr = PBT_KEYLEN(tbuf);
	}

        kp = keyptr;
        keyptr += klen+1;
        BT_GETAJULONG(keyptr,&lv);
        ++refs[*ajTableulongFetch(newpostable, &lv)];
	keyptr += sizeof(ajulong);

        if(full)
            ajDebug("StatNode      %12Lu %12Lu '%*s'\n",
                    page->pagepos, lv, klen, kp);

	lenptr += sizeof(ajuint);
    }

    BT_GETAJULONG(keyptr,&lv);
    keyptr += sizeof(ajulong);
    ++refs[*ajTableulongFetch(newpostable, &lv)];

    if(full)
        ajDebug("StatNode      %12Lu %12Lu '<end>'\n",
                page->pagepos, lv);

    *freespace += pagesize - (keyptr-tbuf);

    if(full)
        ajDebug("StatNode      %12Lu keys:%6u over:%6u free:%6u\n",
                page->pagepos, *nkeys, *overflows, *freespace);

    return ajTrue;
}




/* @func ajBtreeStatNumnode ****************************************************
**
** Returns number of keys, overflow pages visited and free space unused
** for a b+ tree numeric node page
**
** @param [w] cache [AjPBtcache] cache
** @param [r] page [const AjPBtpage] Page
** @param [r] full [AjBool] If true, write detailed debug report
** @param [w] nkeys [ajuint*] Number of keys
** @param [w] overflows [ajuint*] Number of overflow pages
** @param [w] freespace [ajuint*] Unused space in this page
** @param [u] refs [ajulong*] References to pages
** @param [r] newpostable [const AjPTable] Page numbers for page offsets
** @return [AjBool] True on success
** @@
******************************************************************************/

AjBool ajBtreeStatNumnode(AjPBtcache cache, const AjPBtpage page, AjBool full,
                          ajuint* nkeys, ajuint* overflows,
                          ajuint* freespace, ajulong *refs,
                          const AjPTable newpostable)
{
    ajuint i;
    ajuint m;
    const AjPBtpage bpage;
    unsigned char *tbuf;

    ajuint   pagesize = 0;
    ajulong   overflow = 0L;
    ajulong blockno;
    ajuint totlen;
    ajulong left;
    ajulong right;
    ajulong prev;
    unsigned char *keyptr = NULL;
    unsigned char *valptr = NULL;
    ajulong lv;
    ajulong lk;

    btreeCheckNumnode(cache, page);

    pagesize = cache->pagesize;

    bpage = page;
    tbuf = bpage->buf;
    GBT_TOTLEN(tbuf,&totlen);
    
    GBT_NKEYS(tbuf,&m);
    *nkeys = m;
    *overflows = 0;
    *freespace = 0;

    GBT_LEFT(tbuf,&left);
    GBT_RIGHT(tbuf,&right);
    GBT_BLOCKNUMBER(tbuf,&blockno);
    GBT_PREV(tbuf,&prev);
    GBT_OVERFLOW(tbuf,&overflow);

    if(full)
        btreeDumpNumnode(cache, page);

    keyptr =  PBT_KEYLEN(tbuf);
    valptr = keyptr + m * sizeof(ajulong);
    for(i=0;i<m;i++)
    {
        BT_GETAJULONG(valptr,&lv);
        ++refs[*ajTableulongFetch(newpostable, &lv)];
        BT_GETAJULONG(keyptr,&lk);
        keyptr += sizeof(ajulong);
	valptr += sizeof(ajulong);
    }
    BT_GETAJULONG(valptr,&lv);
    valptr += sizeof(ajulong);
    ++refs[*ajTableulongFetch(newpostable, &lv)];
    *freespace += pagesize - (valptr-tbuf);

    return ajTrue;
}




/* @funcstatic btreeDumpNumnode ************************************************
**
** Writes a numeric node page to the debug file
**
** @param [w] cache [AjPBtcache] cache
** @param [r] page [const AjPBtpage] Page
** @return [void]
** @@
******************************************************************************/

static void btreeDumpNumnode(AjPBtcache cache, const AjPBtpage page)
{
    ajuint i;
    ajuint m;
    const AjPBtpage bpage;
    unsigned char *tbuf;

    ajulong   overflow = 0L;
    ajulong blockno;
    ajuint totlen;
    ajulong left;
    ajulong right;
    ajulong prev;
    unsigned char *keyptr = NULL;
    unsigned char *valptr = NULL;
    ajulong lv;
    ajulong lk;

    (void) cache;
    
    bpage = page;
    tbuf = bpage->buf;
    GBT_TOTLEN(tbuf,&totlen);
    
    GBT_NKEYS(tbuf,&m);

    GBT_LEFT(tbuf,&left);
    GBT_RIGHT(tbuf,&right);
    GBT_BLOCKNUMBER(tbuf,&blockno);
    GBT_PREV(tbuf,&prev);
    GBT_OVERFLOW(tbuf,&overflow);

    ajDebug("\n"
                "DumpNumNode   %12Lu totlen:%u\n"
                "         left:%12Lu  right:%12Lu  prev:%12Lu\n"
                "        block:%12Lu   over:%12Lu\n",
                page->pagepos, totlen, left, right, prev, blockno, overflow);
    ajDebug("\nDumpNumnode   %12Lu   pageoffset (%u)\n",
                page->pagepos, m);
    ajDebug("-----------   ------------ ------------ -------\n");


    keyptr =  PBT_KEYLEN(tbuf);
    valptr = keyptr + m * sizeof(ajulong);

    for(i=0;i<m;i++)
    {
        BT_GETAJULONG(valptr,&lv);
        BT_GETAJULONG(keyptr,&lk);
        ajDebug("DumpNumnode   %12Lu %12Lu %Lu\n", page->pagepos, lv, lk);
	keyptr += sizeof(ajulong);
	valptr += sizeof(ajulong);
    }

    BT_GETAJULONG(valptr,&lv);
    ajDebug("DumpNumnode   %12Lu %12Lu '<end>'\n", page->pagepos, lv);
    ajDebug("DumpNumnode   %12Lu keys:%6u\n",
                page->pagepos, m);
    return;
}




/* @func ajBtreeStatBucket *****************************************************
**
** Returns number of keys, overflow pages visited and free space unused
** for a b+ tree bucket page
**
** @param [w] cache [AjPBtcache] cache
** @param [r] page [const AjPBtpage] Page
** @param [r] full [AjBool] If true, write detailed debug report
** @param [w] nentries [ajuint*] Number of entries
** @param [w] ndups [ajuint*] Number of entries with duplicates
** @param [w] nextra [ajuint*] Number of extra entries as duplicates
** @param [w] overflows [ajuint*] Number of overflow pages
** @param [w] freespace [ajuint*] Unused space in this page
** @param [u] refs [ajulong*] References to pages
** @param [r] newpostable [const AjPTable] Page numbers for page offsets
** @return [AjBool] True on success
** @@
******************************************************************************/

AjBool ajBtreeStatBucket(AjPBtcache cache, const AjPBtpage page, AjBool full,
                         ajuint* nentries, ajuint *ndups, ajuint *nextra,
                         ajuint* overflows, ajuint* freespace, ajulong* refs,
                         const AjPTable newpostable)
{
    ajuint i;
    ajuint m;
    ajuint nodetype;
    ajuint klen;
    const AjPBtpage bpage;
    unsigned char *tbuf;
    ajuint freebytes;

    ajuint   pagesize = 0;
    ajulong   overflow = 0L;
    unsigned char *idptr = NULL;
    unsigned char *keyptr = NULL;
    unsigned char *kp = NULL;

    ajuint uv;
    ajuint uv2;
    ajulong lv;
    ajulong lv2;
    ajuint idlen;

    pagesize = cache->pagesize;

    bpage = page;
    tbuf = bpage->buf;

    GBT_BUCKNENTRIES(tbuf,&m);
    *nentries = m;
    *overflows = 0;
    *freespace = 0;
    *ndups = 0;
    *nextra = 0;

    GBT_BUCKOVERFLOW(tbuf,&overflow);

    keyptr = PBT_BUCKKEYLEN(tbuf);
    idptr = keyptr + (m * sizeof(ajuint));

    if(full)
        ajDebug("\n"
                "StatBucket     %12Lu over:%12Lu\n",
                page->pagepos, overflow);
   if(full)
       ajDebug("\nStatBucket     %12Lu   dbno   dups       offset    "
               "refoffset (%u)\n",
               page->pagepos, m);
    if(full)
        ajDebug("----------                    ----   ----       ------    "
                "---------\n");
    for(i=0;i<m;i++)
    {
	BT_GETAJUINT(keyptr,&klen);
        idlen = klen - BT_DDOFFROFF;

	if((idptr-tbuf+2) + klen > cache->pagesize)	/* overflow */
	{
            freebytes = pagesize - (idptr-tbuf);
            *freespace += freebytes;
            (*overflows)++;
    	    /* ajDebug("btreeStatNode: Overflow\n"); */
	    GBT_BUCKOVERFLOW(tbuf,&overflow);
	    bpage = ajBtreeCacheRead(cache,overflow);
	    tbuf = bpage->buf;
	    GBT_BUCKNODETYPE(tbuf,&nodetype);
	    if(nodetype != BT_OVERFLOW)
		ajFatal("StatBucket Overflow node %Lu expected but not found "
                        "for cache '%S'",
                        overflow, cache->filename);
	    /* overflow bucket ids start at the keylen position */
	    idptr = PBT_BUCKKEYLEN(tbuf);
	}
        kp = idptr;
	idptr += idlen;
        BT_GETAJUINT(idptr, &uv);
        idptr += sizeof(ajuint);
	BT_GETAJUINT(idptr,&uv2);
        if(uv2)
        {
            (*ndups)++;
            *nextra += uv2;
        }
	idptr += sizeof(ajuint);	
	BT_GETAJULONG(idptr,&lv);
        if(uv2)
            ++refs[*ajTableulongFetch(newpostable, &lv)];
	idptr += sizeof(ajulong);
	BT_GETAJULONG(idptr,&lv2);
	idptr += sizeof(ajulong);
        
        if(full)
            ajDebug("#StatBucket    %12Lu %6u %6u %12Lu %12Lu '%*s'\n",
                    page->pagepos, uv, uv2, lv, lv2, idlen-1, kp);

	keyptr += sizeof(ajuint);
    }
    *freespace += pagesize - (idptr-tbuf);
    *nextra -= *ndups;
    if(full)
        ajDebug("StatBucket     %12Lu keys:%6u over:%6u free:%6u\n",
                page->pagepos, *nentries, *overflows, *freespace);
    return ajTrue;
}




/* @func ajBtreeStatNumbucket **************************************************
**
** Returns number of keys, overflow pages visited and free space unused
** for a b+ tree secondary bucket page
**
** @param [w] cache [AjPBtcache] cache
** @param [r] page [const AjPBtpage] Page
** @param [r] full [AjBool] If true, write detailed debug report
** @param [w] nentries [ajuint*] Number of entries
** @param [w] overflows [ajuint*] Number of overflow pages
** @param [w] freespace [ajuint*] Unused space in this page
** @return [AjBool] True on success
** @@
******************************************************************************/

AjBool ajBtreeStatNumbucket(AjPBtcache cache, const AjPBtpage page, AjBool full,
                            ajuint* nentries, ajuint* overflows,
                            ajuint* freespace)
{
    ajuint i;
    ajuint m;
    const AjPBtpage bpage;
    unsigned char *tbuf;

    ajulong lv;
    ajulong lv2;
    ajuint v;
    ajuint   pagesize = 0;
    ajulong   overflow = 0L;
    unsigned char *keyptr = NULL;

    pagesize = cache->pagesize;

    bpage = page;
    tbuf = bpage->buf;
    GBT_BUCKNENTRIES(tbuf,&m);
    *nentries = m;
    *overflows = 0;
    *freespace = 0;

    GBT_BUCKOVERFLOW(tbuf,&overflow);

    keyptr = PBT_BUCKKEYLEN(tbuf);

    if(full)
        ajDebug("\n"
                "StatNumBucket  %12Lu over:%12Lu\n",
                page->pagepos, overflow);
    if(full)
        ajDebug("\nStatNumbucket  %12Lu       offset    "
                "refoffset         dbno (%u)\n",
                page->pagepos, m);
    if(full)
        ajDebug("-------------  ------------      -------   "
                "----------         ----\n");
    for(i=0;i<m;i++)
    {
        BT_GETAJULONG(keyptr,&lv);
	keyptr += sizeof(ajulong);
        BT_GETAJULONG(keyptr,&lv2);
	keyptr += sizeof(ajulong);
        BT_GETAJUINT(keyptr,&v);
	keyptr += sizeof(ajuint);
        if(full)
            ajDebug("#StatNumbucket              %12Lu %12Lu %12u\n",
                    lv, lv2, v);
    }
    *freespace += pagesize - (keyptr-tbuf);
    if(full)
        ajDebug("StatNumbucket  %12Lu keys:%6u over:%6u free:%6u\n",
                page->pagepos, *nentries, *overflows, *freespace);
    
    return ajTrue;
}




/* @func ajBtreeStatPribucket **************************************************
**
** Returns number of keys, overflow pages visited and free space unused
** for a b+ tree primary bucket page
**
** @param [w] cache [AjPBtcache] cache
** @param [r] page [const AjPBtpage] Page
** @param [r] full [AjBool] If true, write detailed debug report
** @param [w] nentries [ajuint*] Number of entries
** @param [w] overflows [ajuint*] Number of overflow pages
** @param [w] freespace [ajuint*] Unused space in this page
** @param [u] refs [ajulong*] References to pages
** @param [r] newpostable [const AjPTable] Page numbers for page offsets
** @return [AjBool] True on success
** @@
******************************************************************************/

AjBool ajBtreeStatPribucket(AjPBtcache cache, const AjPBtpage page, AjBool full,
                            ajuint* nentries, ajuint* overflows,
                            ajuint* freespace, ajulong *refs,
                            const AjPTable newpostable)
{
    ajuint i;
    ajuint m;
    ajuint nodetype;
    ajuint klen;
    const AjPBtpage bpage;
    unsigned char *tbuf;
    ajuint freebytes;

    ajuint   pagesize = 0;
    ajulong   overflow = 0L;
    unsigned char *idptr = NULL;
    unsigned char *keyptr = NULL;

    unsigned char* lp;
    ajulong lv;

    pagesize = cache->pagesize;

    bpage = page;
    tbuf = bpage->buf;
    GBT_BUCKNENTRIES(tbuf,&m);
    *nentries = m;
    *overflows = 0;
    *freespace = 0;

    GBT_BUCKOVERFLOW(tbuf,&overflow);

    keyptr = PBT_BUCKKEYLEN(tbuf);
    idptr = keyptr + (m * sizeof(ajuint));

    if(full)
        ajDebug("\n"
                "StatPribucket  %12Lu over:%12Lu\n",
                page->pagepos, overflow);
    if(full)
        ajDebug("\nStatPribucket  %12Lu (%u)\n",
                page->pagepos, m);
    if(full)
        ajDebug("-------------  ------------      -------\n");
    for(i=0;i<m;i++)
    {
	BT_GETAJUINT(keyptr,&klen);

	if((idptr-tbuf+2) + klen > cache->pagesize)	/* overflow */
	{
            freebytes = pagesize - (idptr-tbuf);
            *freespace += freebytes;
            (*overflows)++;
    	    /* ajDebug("btreeStatNode: Overflow\n"); */
	    GBT_BUCKOVERFLOW(tbuf,&overflow);
	    bpage = ajBtreeCacheRead(cache,overflow);
	    tbuf = bpage->buf;
	    GBT_BUCKNODETYPE(tbuf,&nodetype);
	    if(nodetype != BT_OVERFLOW)
		ajFatal("StatPribucket Overflow node %Lu expected but not found",
                        overflow, cache->filename);
	    /* overflow bucket ids start at the keylen position */
	    idptr = PBT_BUCKKEYLEN(tbuf);
	}

        lp = idptr+ klen-8;
        BT_GETAJULONG(lp,&lv);
        ++refs[*ajTableulongFetch(newpostable, &lv)];
        if(full)
            ajDebug("#StatPribucket %12Lu %12Lu '%*s'\n",
                    page->pagepos, lv, klen-9, idptr);
	idptr += klen;    /* string */
	keyptr += sizeof(ajuint);
    }
    *freespace += pagesize - (idptr-tbuf);
    if(full)
        ajDebug("StatPribucket  %12Lu keys:%6u over:%6u free:%6u\n",
                page->pagepos, *nentries, *overflows, *freespace);    
    return ajTrue;
}




/* @func ajBtreeStatSecbucket **************************************************
**
** Returns number of keys, overflow pages visited and free space unused
** for a b+ tree numeric bucket page
**
** @param [w] cache [AjPBtcache] cache
** @param [r] page [const AjPBtpage] Page
** @param [r] full [AjBool] If true, write detailed debug report
** @param [w] nentries [ajuint*] Number of entries
** @param [w] overflows [ajuint*] Number of overflow pages
** @param [w] freespace [ajuint*] Unused space in this page
** @return [AjBool] True on success
** @@
******************************************************************************/

AjBool ajBtreeStatSecbucket(AjPBtcache cache, const AjPBtpage page, AjBool full,
                            ajuint* nentries, ajuint* overflows,
                            ajuint* freespace)
{
    ajuint i;
    ajuint m;
    ajuint nodetype;
    ajuint klen;
    const AjPBtpage bpage;
    unsigned char *tbuf;
    ajuint freebytes;

    ajuint   pagesize = 0;
    ajulong   overflow = 0L;
    unsigned char *idptr = NULL;
    unsigned char *keyptr = NULL;

    pagesize = cache->pagesize;

    bpage = page;
    tbuf = bpage->buf;
    GBT_BUCKNENTRIES(tbuf,&m);
    *nentries = m;
    *overflows = 0;
    *freespace = 0;

    GBT_BUCKOVERFLOW(tbuf,&overflow);

    keyptr = PBT_BUCKKEYLEN(tbuf);
    idptr = keyptr + (m * sizeof(ajuint));

    if(full)
        ajDebug("\n"
                "StatSecbucket  %12Lu over:%12Lu\n",
                page->pagepos, overflow);
    ajDebug("\nStatSecbucket  %12Lu (%u)\n",
            page->pagepos, m);
    if(full)
        ajDebug("-------------  ------------      -------\n");
    for(i=0;i<m;i++)
    {
	BT_GETAJUINT(keyptr,&klen);

	if((idptr-tbuf+2) + klen > cache->pagesize)	/* overflow */
	{
            freebytes = pagesize - (idptr-tbuf);
            *freespace += freebytes;
            (*overflows)++;
    	    /* ajDebug("btreeStatNode: Overflow\n"); */
	    GBT_BUCKOVERFLOW(tbuf,&overflow);
	    bpage = ajBtreeCacheRead(cache,overflow);
	    tbuf = bpage->buf;
            GBT_BUCKNODETYPE(tbuf,&nodetype);
	    if(nodetype != BT_OVERFLOW)
		ajFatal("StatSecbucket Overflow node %Lu expected "
                        "but not found",
                        overflow, cache->filename);
	    /* overflow bucket ids start at the keylen position */
	    idptr = PBT_BUCKKEYLEN(tbuf);
	}

        ajDebug("#StatSecbucket %12Lu '%*s'\n",
                page->pagepos, klen-1, idptr);
	idptr += klen;    /* string */

	keyptr += sizeof(ajuint);
    }
    *freespace += pagesize - (idptr-tbuf);
    if(full)
        ajDebug("StatSecbucket  %12Lu keys:%6u over:%6u free:%6u\n",
                page->pagepos, *nentries, *overflows, *freespace);
    
    return ajTrue;
}




/* @funcstatic btreeBucketSplitCalc ********************************************
**
** Calculate new bucket sizes to split a node.
** Make sure all existing buckets are reused.
**
** @param [r] totalkeys [ajuint] Total number of keys in all buckets
** @param [r] totalbuckets [ajuint] Total number of current buckets
** @param [r] maxbucketsize [ajuint] Maximum number of keys in a buckets
** @param [w] leftbuckets [ajuint*] Number of left node buckets
** @param [w] leftmax [ajuint*] Maximum number of keys per left node bucket
** @param [w] leftkeys [ajuint*] Maximum number of keys under left node
** @param [w] rightbuckets [ajuint*] Number of right node buckets
** @param [w] rightmax [ajuint*] Maximum number of keys per right node bucket
** @param [w] rightkeys [ajuint*] Maximum number of keys under right node
** @return [AjBool] True if calculated value reused all buckets
**                  False if increased to use all buckets
** @@
******************************************************************************/

static AjBool btreeBucketSplitCalc(ajuint totalkeys, ajuint totalbuckets,
                                   ajuint maxbucketsize,
                                   ajuint *leftbuckets, ajuint *leftmax,
                                   ajuint *leftkeys,
                                   ajuint *rightbuckets, ajuint *rightmax,
                                   ajuint *rightkeys)
{
    ajuint lbuckets;
    ajuint rbuckets;
    ajuint lmax;
    ajuint rmax;
    ajuint lkeys;
    ajuint rkeys;

    ajuint lmax2;
    ajuint rmax2;
    ajuint lbuckets1;
    ajuint rbuckets1;
    ajuint lbuckets2;
    ajuint rbuckets2;

    ajuint bucketn;
    ajuint lrest;
    ajuint rrest;

    AjBool ret = ajTrue;

    /* from the original keys and buckets */

    lkeys = totalkeys/2;
    rkeys = totalkeys - lkeys;

    lbuckets = totalbuckets/2;
    rbuckets = totalbuckets - lbuckets;

    lmax = lkeys/lbuckets;
    if(!(lkeys % lbuckets))
        lmax--;
    if(!lmax) lmax = 1;

    bucketn = lkeys/lmax;
    if(lkeys % lmax)
        bucketn++;
    lbuckets1 = bucketn - 1;
    
    rmax = rkeys/rbuckets;
    if(!(rkeys % rbuckets))
        rmax--;
    if(!rmax) rmax = 1;
    
    bucketn = rkeys/rmax;
    if(rkeys % rmax)
        bucketn++;
    rbuckets1 = bucketn - 1;

    /* minimum space required */

    lmax2 = maxbucketsize/2;
    lmax2++;

    bucketn = lkeys/lmax2;
    if(lkeys % lmax2)
        bucketn++;
    lbuckets2 = bucketn - 1;

    rmax2 = maxbucketsize/2;
    rmax2++;

    bucketn = rkeys/rmax2;
    if(rkeys % rmax2)
        bucketn++;
    rbuckets2 = bucketn - 1;

    *leftkeys = lkeys;
    *rightkeys = rkeys;

    if((rbuckets2 + lbuckets2) < totalbuckets)
    {
        *leftmax = lmax;
        *leftbuckets = lbuckets1;
        *rightmax = rmax;
        *rightbuckets = rbuckets1;
        ret = ajFalse;
    }
    else
    {
        *leftmax = lmax2;
        *leftbuckets = lbuckets2;
        *rightmax = rmax2;
        *rightbuckets = rbuckets2;
    }
    lrest = (*leftkeys) - (*leftbuckets)*(*leftmax);
    rrest = (*rightkeys) - (*rightbuckets)*(*rightmax);

#if AJINDEX_DEBUG
    ajDebug("btreeBucketSplitCalc totalkeys:%u totalbuckets:%u "
            "maxbucketsize:%u => %u*%u + %u (%u) "
            "%u*%u + %u (%u)\n",
            totalkeys, totalbuckets, maxbucketsize,
            *leftbuckets, *leftmax, lrest, *leftkeys,
            *rightbuckets, *rightmax, rrest, *rightkeys);
#endif

    if(lrest < 1 || lrest > (*leftmax))
        ajFatal("Bad btreeBucketSplitCalc lrest:%u", lrest);
    if(rrest < 1 || rrest > (*rightmax))
        ajFatal("Bad btreeBucketSplitCalc rrest:%u", rrest);

    return ret;
}




/* @funcstatic btreeBucketCalc ************************************************
**
** Calculate new bucket sizes to split a node.
** Make sure all existing buckets are reused.
**
** @param [r] totalkeys [ajuint] Total number of keys in all buckets
** @param [r] totalbuckets [ajuint] Total number of current buckets
** @param [r] maxbucketsize [ajuint] Maximum number of keys in a buckets
** @param [w] newbuckets [ajuint*] Number of new buckets
** @param [w] newmax [ajuint*] Maximum number of keys per new bucket
** @return [AjBool] True if calculated value reused all buckets
**                  False if increased to use all buckets
** @@
******************************************************************************/

static AjBool btreeBucketCalc(ajuint totalkeys, ajuint totalbuckets,
                              ajuint maxbucketsize,
                              ajuint *newbuckets, ajuint *newmax)
{
    ajuint bmax;
    ajuint bmax2;

    ajuint buckets1;
    ajuint buckets2;

    ajuint bucketn;
    ajuint rest;

    AjBool ret = ajTrue;

    /* from the original keys and buckets */

    if(!totalbuckets)
        ajFatal("btreeBucketCalc zero buckets keys:%u", totalkeys);
    bmax = totalkeys/totalbuckets;
    if(!(totalkeys % totalbuckets))
        bmax--;
    if(!bmax) bmax = 1;

    bucketn = totalkeys/bmax;
    if(totalkeys % bmax)
        bucketn++;
    buckets1 = bucketn - 1;

    /* minimum space required */

    bmax2 = maxbucketsize/2;
    if(!bmax2)
        ++bmax2;

    bucketn = totalkeys/bmax2;
    if(totalkeys % bmax2)
        bucketn++;
    buckets2 = bucketn - 1;


    if(buckets2 < totalbuckets)
    {
        *newmax = bmax;
        *newbuckets = buckets1;
        ret = ajFalse;
    }
    else
    {
        *newmax = bmax2;
        *newbuckets = buckets2;
    }
    rest = (totalkeys) - (*newbuckets)*(*newmax);

#if AJINDEX_DEBUG
    ajDebug("btreeBucketCalc totalkeys:%u totalbuckets:%u "
            "maxbucketsize:%u => %u*%u + %u ret:%B\n",
            totalkeys, totalbuckets, maxbucketsize,
            *newbuckets, *newmax, rest, ret);
#endif

    if(rest < 1 || rest > (*newmax))
        ajFatal("Bad ajBtreeBucketCalc rest:%u", rest);

    return ret;
}




/* @funcstatic btreePageposCompress *******************************************
**
** Return the new start position of a compressed 
**
** @param [r] oldpos [ajulong] Page position
** @param [r] newpagepos [ajulong const*] Array of new page positions
** @param [r] pagesize [ajuint] Full page size
** @param [r] where [const char*] Location in index for reporting in
**                                warning message.
** @return [ajulong] New page start position
** @@
******************************************************************************/

static ajulong btreePageposCompress(ajulong oldpos,
                                    ajulong const* newpagepos,
                                    ajuint pagesize, const char* where)
{
    if(oldpos % pagesize)
        ajWarn("oldpos %Lu pagesize %u (%s)", oldpos, pagesize, where);
    return newpagepos[oldpos/pagesize];
}




/* @funcstatic btreePageposUncompress *****************************************
**
** Return the new start position of a compressed 
**
** @param [r] oldpos [ajulong] Page position
** @param [r] newpostable [const AjPTable] Array of new page positions
** @param [r] where [const char*] Location in index for reporting in
**                                warning message.
** @return [ajulong] new start position
** @@
******************************************************************************/

static ajulong btreePageposUncompress(ajulong oldpos,
                                      const AjPTable newpostable,
                                      const char* where)
{
    const ajulong *newpos;

    ajDebug("btreePageposUncompress oldpos %Lu table size:%u\n",
            oldpos, ajTableGetLength(newpostable));

    newpos =  ajTableulongFetch(newpostable, &oldpos);

    if(!newpos)
        ajWarn("oldpos %Lu not found (%s)", oldpos, where);
    else
        ajDebug("btreePageposUncompress oldpos %Lu newpos %Lu\n",
                oldpos, *newpos);

    return *newpos;
}




/* @funcstatic btreePageUncompress ********************************************
**
** Uncompress a cache page using a table of new positions for pages
**
** @param [u] page [AjPBtpage] Page
** @param [r] newpostable [const AjPTable] Table of new page positions
** @param [r] pagesize [ajuint] Full page size
** @return [AjBool] True on success
** @@
******************************************************************************/

static AjBool btreePageUncompress(AjPBtpage page,
                                  const AjPTable newpostable,
                                  ajuint pagesize)
{
    const unsigned char* buf = page->buf;
    ajuint nodetype;

    GBT_NODETYPE(buf, &nodetype);

    ajDebug("btreePageUncompress %Lu (%s)\n",
            page->pagepos, btreeNodetype(buf));

    switch (nodetype) 
    {
        case BT_FREEPAGE:       /* no need to save the page */
            return ajFalse;
        case BT_ROOT:
        case BT_INTERNAL:
        case BT_LEAF:
            btreePageUncompressNode(page, newpostable, pagesize);
            return ajTrue;
        case BT_BUCKET:
            btreePageUncompressBucket(page, newpostable);
            return ajTrue;
        case BT_OVERFLOW:
            ajErr("Overflow page type %u in btreePageCompress", nodetype);
            return ajTrue;
        case BT_PRIBUCKET:
            btreePageUncompressPribucket(page, newpostable);
            return ajTrue;
        case BT_SECBUCKET:
            btreePageUncompressSecbucket(page, newpostable);
            return ajTrue;
        case BT_NUMBUCKET:
            btreePageUncompressNumbucket(page, newpostable);
            return ajTrue;
        default:
            ajErr("Unknown page type %u in btreePageUncompress", nodetype);
            break;
    }
    return ajTrue;
}




/* @funcstatic btreePageUncompressBucket **************************************
**
** Uncompress a bucket cache page using an array of new positions for
** uncompressed pages
**
** @param [u] page [AjPBtpage] Page
** @param [r] newpostable [const AjPTable] Table of new page positions
** @return [void]
** @@
******************************************************************************/

static void btreePageUncompressBucket(AjPBtpage page,
                                      const AjPTable newpostable)
{
    unsigned char* buf = page->buf;
    ajulong   overflow = 0L;
    unsigned char *idptr = NULL;
    unsigned char *keyptr = NULL;
    ajulong lv;
    ajuint klen;
    ajuint idlen;
    ajuint nentries;
    ajuint dups;
    ajuint i;

    buf = page->buf;

    GBT_BUCKNENTRIES(buf,&nentries);
    GBT_BUCKOVERFLOW(buf,&overflow);
    if(overflow)
    {
        overflow = btreePageposUncompress(overflow, newpostable,
                                          "Bucket overflow");
        SBT_BUCKOVERFLOW(buf, overflow);
    }

    keyptr = PBT_BUCKKEYLEN(buf);
    idptr = keyptr + (nentries* sizeof(ajuint));

    /*
    ** ajulong values in bucket are offset and refoffset in files for dups=1
    ** but for dups > 1 offset is the secondary root page for the duplicates
    */

    for(i=0;i<nentries;i++)
    {
	BT_GETAJUINT(keyptr,&klen);
        idlen = klen - BT_DDOFFROFF;

	idptr += idlen + sizeof(ajuint);
	BT_GETAJUINT(idptr,&dups);
        idptr += sizeof(ajuint);

        if(dups)
        {
            BT_GETAJULONG(idptr,&lv);
            lv = btreePageposUncompress(lv, newpostable,
                                        "Bucket dup offset (secrootblock)");
            BT_SETAJULONG(idptr, lv);
        }

        idptr += sizeof(ajulong) + sizeof(ajulong);
	keyptr += sizeof(ajuint);
    }

    return;
}




/* @funcstatic btreePageUncompressNode ****************************************
**
** Uncompress a node cache page using a table of new positions
**
** @param [u] page [AjPBtpage] Page
** @param [r] newpostable [const AjPTable] Table of new page positions
** @param [r] pagesize [ajuint] Full page size
** @return [void]
** @@
******************************************************************************/

static void btreePageUncompressNode(AjPBtpage page,
                                    const AjPTable newpostable,
                                    ajuint pagesize)
{
    unsigned char* buf = page->buf;
    ajuint nkeys = 0;
    ajulong overflow = 0L;
    unsigned char *lenptr = NULL;
    unsigned char *keyptr = NULL;

    ajuint klen;
    ajuint totlen;
    ajuint i;
    ajulong blockno;
    ajulong left;
    ajulong right;
    ajulong prev;
    ajulong lv;

    buf = page->buf;

    GBT_TOTLEN(buf,&totlen);
    if(!totlen)
    {
        btreePageUncompressNumnode(page, newpostable, pagesize);
        return;
    }

    GBT_NKEYS(buf,&nkeys);
    GBT_OVERFLOW(buf,&overflow);

    if(overflow)
    {
        overflow = btreePageposUncompress(overflow, newpostable,
                                          "Node overflow");
        SBT_OVERFLOW(buf, overflow);
    }
    
    GBT_LEFT(buf,&left);

    if(left)
    {
        left = btreePageposUncompress(left, newpostable,
                                      "Node left");
        SBT_LEFT(buf, left);
    }
    
    GBT_RIGHT(buf,&right);

    if(right >= pagesize)
    {
        right = btreePageposUncompress(right, newpostable,
                                       "Node right");
        SBT_RIGHT(buf, right);
    }
    
    GBT_PREV(buf,&prev);

    if(prev)
    {
        prev = btreePageposUncompress(prev, newpostable,
                                      "Node prev");
        SBT_PREV(buf, prev);
    }
    
    GBT_BLOCKNUMBER(buf,&blockno);
    if(blockno)
    {
        blockno = btreePageposUncompress(blockno, newpostable,
                                         "Node blockno");
        SBT_BLOCKNUMBER(buf, blockno);
    }
    
    lenptr = PBT_KEYLEN(buf);
    keyptr = lenptr + nkeys * sizeof(ajuint);

    for(i=0;i<nkeys;i++)
    {
	BT_GETAJUINT(lenptr,&klen);
        keyptr += klen + 1;
        BT_GETAJULONG(keyptr,&lv); /* pagepos for key */

        if(lv)
        {
            lv = btreePageposUncompress(lv, newpostable,
                                        "Node keypos");
            BT_SETAJULONG(keyptr, lv);
        }

        keyptr += sizeof(ajulong);
	lenptr += sizeof(ajuint);
    }

    BT_GETAJULONG(keyptr,&lv); /* pagepos after last key */

    if(lv)
    {
        lv = btreePageposUncompress(lv, newpostable,
                                    "Node last keypos");
        BT_SETAJULONG(keyptr, lv);
    }

    return;
}




/* @funcstatic btreePageUncompressNumbucket ***********************************
**
** Uncompress a numeric bucket cache page using a table of new positions
**
** @param [u] page [AjPBtpage] Page
** @param [r] newpostable [const AjPTable] Table of new page positions
** @return [void]
** @@
******************************************************************************/

static void btreePageUncompressNumbucket(AjPBtpage page,
                                         const AjPTable newpostable)
{
    unsigned char* buf = page->buf;
    ajulong   overflow = 0L;

    buf = page->buf;

    GBT_BUCKOVERFLOW(buf,&overflow);
    if(overflow)
    {
        overflow = btreePageposUncompress(overflow, newpostable,
                                          "Numbucket overflow");
        SBT_BUCKOVERFLOW(buf, overflow);
    }

    return;
}




/* @funcstatic btreePageUncompressNumnode ************************************
**
** uncompress a numeric node cache page using a table of new positions
**
** @param [u] page [AjPBtpage] Page
** @param [r] newpostable [const AjPTable] Table of new page positions
** @param [r] pagesize [ajuint] Full page size
** @return [void]
** @@
******************************************************************************/

static void btreePageUncompressNumnode(AjPBtpage page,
                                       const AjPTable newpostable,
                                       ajuint pagesize)
{
    unsigned char* buf = page->buf;
    ajuint nkeys;
    ajulong   overflow = 0L;
    unsigned char *keyptr = NULL;
    unsigned char *valptr = NULL;
    ajuint i;
    ajulong blockno;
    ajulong left;
    ajulong right;
    ajulong prev;
    ajulong lv;

    buf = page->buf;

    GBT_NKEYS(buf,&nkeys);
    GBT_OVERFLOW(buf,&overflow);
    if(overflow)
    {
        overflow = btreePageposUncompress(overflow, newpostable,
                                          "Numnode overflow");
        SBT_OVERFLOW(buf, overflow);
    }

    GBT_LEFT(buf,&left);

    if(left)
    {
        left = btreePageposUncompress(left, newpostable,
                                      "Numnode left");
        SBT_LEFT(buf, left);
    }
    
    GBT_RIGHT(buf,&right);

    if(right >= pagesize)
    {
        right = btreePageposUncompress(right, newpostable,
                                       "Numnode right");
        SBT_RIGHT(buf, right);
    }
    
    GBT_PREV(buf,&prev);

    if(prev)
    {
        prev = btreePageposUncompress(prev, newpostable,
                                      "Numnode prev");
        SBT_PREV(buf, prev);
    }
    
    GBT_BLOCKNUMBER(buf,&blockno);
    if(blockno)
    {
        prev = btreePageposUncompress(blockno, newpostable,
                                      "Numnode blockno");
        SBT_BLOCKNUMBER(buf, blockno);
    }
    
    keyptr =  PBT_KEYLEN(buf);
    valptr = keyptr + nkeys * sizeof(ajulong);

    for(i = 0; i < nkeys; i++)
    {
        BT_GETAJULONG(valptr,&lv); /* pagepos for key */

        if(lv)
        {
            lv = btreePageposUncompress(lv, newpostable,
                                        "Numnode keypos");
            BT_SETAJULONG(valptr, lv);
        }

        valptr += sizeof(ajulong);
    }

    BT_GETAJULONG(valptr,&lv); /* pagepos after last key */

    if(lv)
    {
        lv = btreePageposUncompress(lv, newpostable,
                                    "Numnode last keypos");
        BT_SETAJULONG(valptr, lv);
    }

    return;
}




/* @funcstatic btreePageUncompressPribucket ***********************************
**
** Uncompress a primary bucket cache page using a table of new positions
**
** @param [u] page [AjPBtpage] Page
** @param [r] newpostable [const AjPTable] Table of new page positions
** @return [void]
** @@
******************************************************************************/

static void btreePageUncompressPribucket(AjPBtpage page,
                                         const AjPTable newpostable)
{
    unsigned char* buf = page->buf;
    ajuint nentries;
    ajulong overflow = 0L;
    unsigned char *idptr = NULL;
    unsigned char *keyptr = NULL;
    ajuint klen;
    ajuint i;
    ajulong lv;
    unsigned char * lp = NULL;

    buf = page->buf;

    GBT_BUCKNENTRIES(buf,&nentries);
    GBT_BUCKOVERFLOW(buf,&overflow);
    if(overflow)
    {
        overflow = btreePageposUncompress(overflow, newpostable,
                                          "PriBucket overflow");
        SBT_BUCKOVERFLOW(buf, overflow);
    }

    keyptr = PBT_BUCKKEYLEN(buf);
    idptr = keyptr + (nentries * sizeof(ajuint));

    for(i=0;i<nentries;i++)
    {
	BT_GETAJUINT(keyptr,&klen);
	idptr += klen;
        lp = idptr - 8;
        BT_GETAJULONG(lp,&lv);

        if(lv)
        {
            lv = btreePageposUncompress(lv, newpostable,
                                        "PriBucket keypos");
            BT_SETAJULONG(lp, lv);
        }

	keyptr += sizeof(ajuint);
    }

    return;
}




/* @funcstatic btreePageUncompressSecbucket ***********************************
**
** Uncompress a secondary bucket cache page using a table of new positions
**
** @param [u] page [AjPBtpage] Page
** @param [r] newpostable [const AjPTable] Table of new page positions
** @return [void]
** @@
******************************************************************************/

static void btreePageUncompressSecbucket(AjPBtpage page,
                                         const AjPTable newpostable)
{
    unsigned char* buf = page->buf;
    ajulong   overflow = 0L;
 
    buf = page->buf;

    GBT_BUCKOVERFLOW(buf,&overflow);
    if(overflow)
    {
        overflow = btreePageposUncompress(overflow, newpostable,
                                          "SecBucket overflow");
        SBT_BUCKOVERFLOW(buf, overflow);
    }

    /* bucket contains only ID strings */

    return;
}




/* @funcstatic btreePageCompress ********************************************
**
** Compress a cache page using an array of new positions for uncompressed pages
**
** @param [u] page [AjPBtpage] Page
** @param [r] newpagepos [ajulong const*] Array of new page positions
** @param [r] pagesize [ajuint] Full page size
** @return [AjBool] True on success
** @@
******************************************************************************/

static AjBool btreePageCompress(AjPBtpage page,
                                ajulong const* newpagepos,
                                ajuint pagesize)
{
    const unsigned char* buf = page->buf;
    ajuint nodetype;

    GBT_NODETYPE(buf, &nodetype);
    switch (nodetype) 
    {
        case BT_FREEPAGE:       /* no need to save the page */
            return ajFalse;
        case BT_ROOT:
        case BT_INTERNAL:
        case BT_LEAF:
            btreePageCompressNode(page, newpagepos, pagesize);
            return ajTrue;
        case BT_BUCKET:
            btreePageCompressBucket(page, newpagepos, pagesize);
            return ajTrue;
        case BT_OVERFLOW:
            ajErr("Overflow page type %u in btreePageCompress", nodetype);
            return ajTrue;
        case BT_PRIBUCKET:
            btreePageCompressPribucket(page, newpagepos, pagesize);
            return ajTrue;
        case BT_SECBUCKET:
            btreePageCompressSecbucket(page, newpagepos, pagesize);
            return ajTrue;
        case BT_NUMBUCKET:
            btreePageCompressNumbucket(page, newpagepos, pagesize);
            return ajTrue;
        default:
            ajErr("Unknown page type %u in btreePageCompress", nodetype);
            break;
    }
    return ajTrue;
}




/* @funcstatic btreePageCompressBucket ****************************************
**
** Compress a bucket cache page using an array of new positions for
** uncompressed pages
**
** @param [u] page [AjPBtpage] Page
** @param [r] newpagepos [ajulong const*] Array of new page positions
** @param [r] pagesize [ajuint] Full page size
** @return [void]
** @@
******************************************************************************/

static void btreePageCompressBucket(AjPBtpage page,
                                    ajulong const* newpagepos,
                                    ajuint pagesize)
{
    unsigned char* buf = page->buf;
    ajulong   overflow = 0L;
    unsigned char *idptr = NULL;
    unsigned char *keyptr = NULL;
    ajulong lv;
    ajuint klen;
    ajuint idlen;
    ajuint nentries;
    ajuint dups;
    ajuint i;

    buf = page->buf;

    GBT_BUCKNENTRIES(buf,&nentries);
    GBT_BUCKOVERFLOW(buf,&overflow);
    if(overflow)
    {
        overflow = btreePageposCompress(overflow, newpagepos, pagesize,
                                        "Bucket overflow");
        SBT_BUCKOVERFLOW(buf, overflow);
    }

    keyptr = PBT_BUCKKEYLEN(buf);
    idptr = keyptr + (nentries* sizeof(ajuint));

    /*
    ** ajulong values in bucket are offset and refoffset in files for dups=1
    ** but for dups > 1 offset is the secondary root page for the duplicates
    */

    for(i=0;i<nentries;i++)
    {
	BT_GETAJUINT(keyptr,&klen);
        idlen = klen - BT_DDOFFROFF;

	idptr += idlen + sizeof(ajuint);
	BT_GETAJUINT(idptr,&dups);
        idptr += sizeof(ajuint);

        if(dups)
        {
            BT_GETAJULONG(idptr,&lv);
            lv = btreePageposCompress(lv, newpagepos, pagesize,
                                      "Bucket dup offset (secrootblock)");
            BT_SETAJULONG(idptr, lv);
        }

        idptr += sizeof(ajulong) + sizeof(ajulong);
	keyptr += sizeof(ajuint);
    }

    return;
}




/* @funcstatic btreePageCompressNode ******************************************
**
** Compress a node cache page using an array of new positions for
** uncompressed pages
**
** @param [u] page [AjPBtpage] Page
** @param [r] newpagepos [ajulong const*] Array of new page positions
** @param [r] pagesize [ajuint] Full page size
** @return [void]
** @@
******************************************************************************/

static void btreePageCompressNode(AjPBtpage page,
                                  ajulong const* newpagepos,
                                  ajuint pagesize)
{
    unsigned char* buf = page->buf;
    ajuint nkeys = 0;
    ajulong overflow = 0L;
    unsigned char *lenptr = NULL;
    unsigned char *keyptr = NULL;

    ajuint klen;
    ajuint totlen;
    ajuint i;
    ajulong blockno;
    ajulong left;
    ajulong right;
    ajulong prev;
    ajulong lv;

    buf = page->buf;

    GBT_TOTLEN(buf,&totlen);

    if(!totlen)
    {
        btreePageCompressNumnode(page, newpagepos, pagesize);
        return;
    }

    GBT_NKEYS(buf,&nkeys);
    GBT_OVERFLOW(buf,&overflow);

    if(overflow)
    {
        overflow = btreePageposCompress(overflow, newpagepos, pagesize,
                                        "Node overflow");
        SBT_OVERFLOW(buf, overflow);
    }
    
    GBT_LEFT(buf,&left);

    if(left)
    {
        left = btreePageposCompress(left, newpagepos, pagesize,
                                    "Node left");
        SBT_LEFT(buf, left);
    }
    
    GBT_RIGHT(buf,&right);

    if(right >= pagesize)
    {
        right = btreePageposCompress(right, newpagepos, pagesize,
                                     "Node right");
        SBT_RIGHT(buf, right);
    }
    
    GBT_PREV(buf,&prev);

    if(prev)
    {
        prev = btreePageposCompress(prev, newpagepos, pagesize,
                                    "Node prev");
        SBT_PREV(buf, prev);
    }
    
    GBT_BLOCKNUMBER(buf,&blockno);
    if(blockno)
    {
        blockno = btreePageposCompress(blockno, newpagepos, pagesize,
                                       "Node blockno");
        SBT_BLOCKNUMBER(buf, blockno);
    }
    
    lenptr = PBT_KEYLEN(buf);
    keyptr = lenptr + nkeys * sizeof(ajuint);

    for(i=0;i<nkeys;i++)
    {
	BT_GETAJUINT(lenptr,&klen);
        keyptr += klen + 1;
        BT_GETAJULONG(keyptr,&lv); /* pagepos for key */

        if(lv)
        {
            lv = btreePageposCompress(lv, newpagepos, pagesize,
                                      "Node keypos");
            BT_SETAJULONG(keyptr, lv);
        }

        keyptr += sizeof(ajulong);
	lenptr += sizeof(ajuint);
    }

    BT_GETAJULONG(keyptr,&lv); /* pagepos after last key */

    if(lv)
    {
        lv = btreePageposCompress(lv, newpagepos, pagesize,
                                  "Node last keypos");
        BT_SETAJULONG(keyptr, lv);
    }

    return;
}




/* @funcstatic btreePageCompressNumbucket *************************************
**
** Compress a numeric bucket cache page using an array of new
** positions for uncompressed pages
**
** @param [u] page [AjPBtpage] Page
** @param [r] newpagepos [ajulong const*] Array of new page positions
** @param [r] pagesize [ajuint] Full page size
** @return [void]
** @@
******************************************************************************/

static void btreePageCompressNumbucket(AjPBtpage page,
                                       ajulong const* newpagepos,
                                       ajuint pagesize)
{
    unsigned char* buf = page->buf;
    ajulong   overflow = 0L;

    buf = page->buf;

    GBT_BUCKOVERFLOW(buf,&overflow);
    if(overflow)
    {
        overflow = btreePageposCompress(overflow, newpagepos, pagesize,
                                        "Numbucket overflow");
        SBT_BUCKOVERFLOW(buf, overflow);
    }

    return;
}




/* @funcstatic btreePageCompressNumnode **************************************
**
** Compress a numeric node cache page using an array of new
** positions for uncompressed pages
**
** @param [u] page [AjPBtpage] Page
** @param [r] newpagepos [ajulong const*] Array of new page positions
** @param [r] pagesize [ajuint] Full page size
** @return [void]
** @@
******************************************************************************/

static void btreePageCompressNumnode(AjPBtpage page,
                                     ajulong const* newpagepos,
                                     ajuint pagesize)
{
    unsigned char* buf = page->buf;
    ajuint nkeys;
    ajulong   overflow = 0L;
    unsigned char *keyptr = NULL;
    unsigned char *valptr = NULL;
    ajuint i;
    ajulong blockno;
    ajulong left;
    ajulong right;
    ajulong prev;
    ajulong lv;

    buf = page->buf;

    GBT_NKEYS(buf,&nkeys);
    GBT_OVERFLOW(buf,&overflow);
    if(overflow)
    {
        overflow = btreePageposCompress(overflow, newpagepos, pagesize,
                                        "Numnode overflow");
        SBT_OVERFLOW(buf, overflow);
    }

    GBT_LEFT(buf,&left);

    if(left)
    {
        left = btreePageposCompress(left, newpagepos, pagesize,
                                    "Numnode left");
        SBT_LEFT(buf, left);
    }
    
    GBT_RIGHT(buf,&right);

    if(right >= pagesize)
    {
        right = btreePageposCompress(right, newpagepos, pagesize,
                                     "Numnode right");
        SBT_RIGHT(buf, right);
    }
    
    GBT_PREV(buf,&prev);

    if(prev)
    {
        prev = btreePageposCompress(prev, newpagepos, pagesize,
                                    "Numnode prev");
        SBT_PREV(buf, prev);
    }
    
    GBT_BLOCKNUMBER(buf,&blockno);
    if(blockno)
    {
        blockno = btreePageposCompress(blockno, newpagepos, pagesize,
                                       "Numnode blockno");
        SBT_BLOCKNUMBER(buf, blockno);
    }
    
    keyptr =  PBT_KEYLEN(buf);
    valptr = keyptr + nkeys * sizeof(ajulong);

    for(i = 0; i < nkeys; i++)
    {
        BT_GETAJULONG(valptr,&lv); /* pagepos for key */

        if(lv)
        {
            lv = btreePageposCompress(lv, newpagepos, pagesize,
                                      "Numnode keypos");
            BT_SETAJULONG(valptr, lv);
        }

        valptr += sizeof(ajulong);
    }

    BT_GETAJULONG(valptr,&lv); /* pagepos after last key */

    if(lv)
    {
        lv = btreePageposCompress(lv, newpagepos, pagesize,
                                  "Numnode last keypos");
        BT_SETAJULONG(valptr, lv);
    }

    return;
}




/* @funcstatic btreePageCompressPribucket *************************************
**
** Compress a primary bucket cache page using an array of new
** positions for uncompressed pages
**
** @param [u] page [AjPBtpage] Page
** @param [r] newpagepos [ajulong const*] Array of new page positions
** @param [r] pagesize [ajuint] Full page size
** @return [void]
** @@
******************************************************************************/

static void btreePageCompressPribucket(AjPBtpage page,
                                       ajulong const* newpagepos,
                                       ajuint pagesize)
{
    unsigned char* buf = page->buf;
    ajuint nentries;
    ajulong overflow = 0L;
    unsigned char *idptr = NULL;
    unsigned char *keyptr = NULL;
    ajuint klen;
    ajuint i;
    ajulong lv;
    unsigned char * lp = NULL;

    buf = page->buf;

    GBT_BUCKNENTRIES(buf,&nentries);
    GBT_BUCKOVERFLOW(buf,&overflow);
    if(overflow)
    {
        overflow = btreePageposCompress(overflow, newpagepos, pagesize,
                                        "PriBucket overflow");
        SBT_BUCKOVERFLOW(buf, overflow);
    }

    keyptr = PBT_BUCKKEYLEN(buf);
    idptr = keyptr + (nentries * sizeof(ajuint));

    for(i=0;i<nentries;i++)
    {
	BT_GETAJUINT(keyptr,&klen);
	idptr += klen;
        lp = idptr - 8;
        BT_GETAJULONG(lp,&lv);

        if(lv)
        {
            lv = btreePageposCompress(lv, newpagepos, pagesize,
                                        "PriBucket keypos");
            BT_SETAJULONG(lp, lv);
        }

	keyptr += sizeof(ajuint);
    }

    return;
}




/* @funcstatic btreePageCompressSecbucket *************************************
**
** Compress a secondary bucket cache page using an array of new
** positions for uncompressed pages
**
** @param [u] page [AjPBtpage] Page
** @param [r] newpagepos [ajulong const*] Array of new page positions
** @param [r] pagesize [ajuint] Full page size
** @return [void]
** @@
******************************************************************************/

static void btreePageCompressSecbucket(AjPBtpage page,
                                       ajulong const* newpagepos,
                                       ajuint pagesize)
{
    unsigned char* buf = page->buf;
    ajulong   overflow = 0L;
 
    buf = page->buf;

    GBT_BUCKOVERFLOW(buf,&overflow);
    if(overflow)
    {
        overflow = btreePageposCompress(overflow, newpagepos, pagesize,
                                        "SecBucket overflow");
        SBT_BUCKOVERFLOW(buf, overflow);
    }

    /* bucket contains only ID strings */

    return;
}




/* @func ajBtreePageGetSize ***************************************************
**
** Return the used size of a cache page
**
** @param [r] page [const AjPBtpage] Page
** @return [ajuint] Used page size
** @@
******************************************************************************/

ajuint ajBtreePageGetSize(const AjPBtpage page)
{
    const unsigned char* buf = page->buf;
    ajuint nodetype;

    GBT_NODETYPE(buf, &nodetype);
    switch (nodetype) 
    {
        case BT_FREEPAGE:       /* no need to save the page */
            return 0;
        case BT_ROOT:
        case BT_INTERNAL:
        case BT_LEAF:
            return btreePageGetSizeNode(page);
        case BT_BUCKET:
            return btreePageGetSizeBucket(page);
        case BT_OVERFLOW:
            ajErr("Overflow page type %u in btreePageGetSize", nodetype);
            return 0;
        case BT_PRIBUCKET:
            return btreePageGetSizePribucket(page);
        case BT_SECBUCKET:
            return btreePageGetSizeSecbucket(page);
        case BT_NUMBUCKET:
            return btreePageGetSizeNumbucket(page);
        default:
            ajErr("Unknown page type %u in btreePageGetSize", nodetype);
            break;
    }
    return 0;
}




/* @funcstatic btreePageGetSizeBucket *****************************************
**
** Return the used size of a bucket cache page
**
** @param [r] page [const AjPBtpage] Page
** @return [ajuint] Used page size
** @@
******************************************************************************/

static ajuint btreePageGetSizeBucket(const AjPBtpage page)
{
    const unsigned char* buf = page->buf;
    ajuint nentries;
    ajulong   overflow = 0L;
    const unsigned char *idptr = NULL;
    const unsigned char *keyptr = NULL;
    ajuint klen;
    ajuint idlen;
    ajuint i;

    buf = page->buf;

    GBT_BUCKNENTRIES(buf,&nentries);
    GBT_BUCKOVERFLOW(buf,&overflow);

    keyptr = PBT_BUCKKEYLEN(buf);
    idptr = keyptr + (nentries * sizeof(ajuint));

    for(i=0;i<nentries;i++)
    {
	BT_GETAJUINT(keyptr,&klen);
        idlen = klen - BT_DDOFFROFF;
	idptr += (idlen + sizeof(ajuint) + sizeof(ajuint) +
                  sizeof(ajulong) + sizeof(ajulong));
	keyptr += sizeof(ajuint);
    }

    return (idptr-buf);
}




/* @funcstatic btreePageGetSizeNode *******************************************
**
** Return the used size of a node cache page
**
** @param [r] page [const AjPBtpage] Page
** @return [ajuint] Used page size
** @@
******************************************************************************/

static ajuint btreePageGetSizeNode(const AjPBtpage page)
{
    const unsigned char* buf = page->buf;
    ajuint nkeys = 0;
    ajulong overflow = 0L;
    const unsigned char *lenptr = NULL;
    const unsigned char *keyptr = NULL;

    ajuint klen;
    ajuint totlen;
    ajuint i;

    buf = page->buf;

    GBT_TOTLEN(buf,&totlen);
    if(!totlen)
      return btreePageGetSizeNumnode(page);

    GBT_NKEYS(buf,&nkeys);
    GBT_OVERFLOW(buf,&overflow);

    lenptr = PBT_KEYLEN(buf);
    keyptr = lenptr + nkeys * sizeof(ajuint);

    for(i=0;i<nkeys;i++)
    {
	BT_GETAJUINT(lenptr,&klen);
        keyptr += klen + 1 + sizeof(ajulong);
	lenptr += sizeof(ajuint);
    }

    keyptr += sizeof(ajulong);

    return (keyptr-buf);
}




/* @funcstatic btreePageGetSizeNumbucket **************************************
**
** Return the used size of a numeric bucket cache page
**
** @param [r] page [const AjPBtpage] Page
** @return [ajuint] Used page size
** @@
******************************************************************************/

static ajuint btreePageGetSizeNumbucket(const AjPBtpage page)
{
    const unsigned char* buf = page->buf;
    ajuint nentries;
    ajulong   overflow = 0L;
    const unsigned char *keyptr = NULL;

    buf = page->buf;

    GBT_BUCKNENTRIES(buf,&nentries);
    GBT_BUCKOVERFLOW(buf,&overflow);

    keyptr = PBT_BUCKKEYLEN(buf);

    keyptr += nentries * (sizeof(ajlong) + sizeof(ajlong) + sizeof(ajuint));

    return (keyptr-buf);
}




/* @funcstatic btreePageGetSizeNumnode ****************************************
**
** Return the used size of a numeric node cache page
**
** @param [r] page [const AjPBtpage] Page
** @return [ajuint] Used page size
** @@
******************************************************************************/

static ajuint btreePageGetSizeNumnode(const AjPBtpage page)
{
    const unsigned char* buf = page->buf;
    ajuint nkeys;
    ajulong   overflow = 0L;
    const unsigned char *keyptr = NULL;
    const unsigned char *valptr = NULL;

    buf = page->buf;

    GBT_NKEYS(buf,&nkeys);
    GBT_OVERFLOW(buf,&overflow);

    keyptr =  PBT_KEYLEN(buf);
    valptr = keyptr + nkeys * sizeof(ajulong) +
        nkeys * sizeof(ajulong) + sizeof(ajulong);

    return (valptr-buf);
}




/* @funcstatic btreePageGetSizePribucket **************************************
**
** Return the used size of a primary bucket cache page
**
** @param [r] page [const AjPBtpage] Page
** @return [ajuint] Used page size
** @@
******************************************************************************/

static ajuint btreePageGetSizePribucket(const AjPBtpage page)
{
    const unsigned char* buf = page->buf;
    ajuint nentries;
    ajulong overflow = 0L;
    const unsigned char *idptr = NULL;
    const unsigned char *keyptr = NULL;
    ajuint klen;
    ajuint i;

    buf = page->buf;

    GBT_BUCKNENTRIES(buf,&nentries);
    GBT_BUCKOVERFLOW(buf,&overflow);

    keyptr = PBT_BUCKKEYLEN(buf);
    idptr = keyptr + (nentries * sizeof(ajuint));

    for(i=0;i<nentries;i++)
    {
	BT_GETAJUINT(keyptr,&klen);
	idptr += klen;
	keyptr += sizeof(ajuint);
    }

    return (idptr-buf);
}




/* @funcstatic btreePageGetSizeSecbucket **************************************
**
** Return the used size of a secondary bucket cache page
**
** @param [r] page [const AjPBtpage] Page
** @return [ajuint] Used page size
** @@
******************************************************************************/

static ajuint btreePageGetSizeSecbucket(const AjPBtpage page)
{
    const unsigned char* buf = page->buf;
    ajuint nentries;
    ajulong   overflow = 0L;
    const unsigned char *idptr = NULL;
    const unsigned char *keyptr = NULL;
    ajuint klen;
    ajuint i;

    buf = page->buf;

    GBT_BUCKNENTRIES(buf,&nentries);
    GBT_BUCKOVERFLOW(buf,&overflow);

    keyptr = PBT_BUCKKEYLEN(buf);
    idptr = keyptr + (nentries * sizeof(ajuint));

    for(i=0;i<nentries;i++)
    {
	BT_GETAJUINT(keyptr,&klen);
	idptr += klen;
	keyptr += sizeof(ajuint);
    }

    return (idptr-buf);
}




/* @funcstatic btreeCheckNode *************************************************
**
** Checks consistency of a node page
**
** @param [w] cache [AjPBtcache] cache
** @param [r] page [const AjPBtpage] Page
** @return [AjBool] True on success
** @@
******************************************************************************/

static AjBool btreeCheckNode(AjPBtcache cache, const AjPBtpage page)
{
    ajuint i;
    ajuint m;
    ajuint nodetype;
    ajuint klen;
    const AjPBtpage bpage;
    unsigned char *tbuf;
    ajuint freebytes;

    ajuint   pagesize = 0;
    ajulong   overflow = 0L;
    ajulong blockno;
    ajuint totlen;
    ajulong left;
    ajulong right;
    ajulong prev;
    unsigned char *lenptr = NULL;
    unsigned char *keyptr = NULL;
    ajulong lv;
    AjBool ret = ajTrue;
    AjPBtpage ncpage = NULL;

    if(!btreeCheckNodeHeader(cache, page, "node"))
        ret = ajFalse;

    pagesize = cache->pagesize;

    bpage = page;
    tbuf = bpage->buf;
    GBT_TOTLEN(tbuf,&totlen);
    
    GBT_NKEYS(tbuf,&m);

    GBT_LEFT(tbuf,&left);
    GBT_RIGHT(tbuf,&right);
    GBT_BLOCKNUMBER(tbuf,&blockno);
    GBT_PREV(tbuf,&prev);
    GBT_OVERFLOW(tbuf,&overflow);


    lenptr =  PBT_KEYLEN(tbuf);
    keyptr = lenptr + m * sizeof(ajuint);

    AJNEW0(ncpage);

    for(i=0;i<m;i++)
    {
	BT_GETAJUINT(lenptr,&klen);

	if((ajuint)((keyptr-tbuf+2) + klen + sizeof(ajulong)) > pagesize)
	{
            freebytes = pagesize - (keyptr-tbuf);
    	    /* ajDebug("btreeStatNode: Overflow\n"); */
	    btreeNocacheFetch(cache, ncpage, overflow);
	    tbuf = bpage->buf;
	    GBT_NODETYPE(tbuf,&nodetype);
	    if(nodetype != BT_OVERFLOW)
		ajFatal("btreeCheckNode "
                        "Overflow node %Lu expected but not found "
                        "cache %S page %Lu "
                        "pagepos:%Lu key:%u/%u free:%u keyptr:%x lenptr:%x "
                        "tbuf:%x klen:%u old nodetype '%s' newnodetype '%s' "
                        "blockno:%Lu totlen:%u left:%Lu right:%Lu prev:%Lu",
                        overflow, cache->filename,
                        page->pagepos, page->pagepos/pagesize,
                        i, m, freebytes,
                        keyptr, lenptr, tbuf, klen,
                        btreeNodetype(page->buf), btreeNodetype(tbuf),
                        blockno,totlen, left, right, prev);

            GBT_BLOCKNUMBER(tbuf,&blockno);
            GBT_TOTLEN(tbuf,&totlen);
            GBT_LEFT(tbuf,&left);
            GBT_RIGHT(tbuf,&right);
            GBT_PREV(tbuf,&prev);

	    /*
	     ** The length pointer is restricted to the initial page.
	     ** The keyptr in overflow pages starts at the Key Lengths
	     ** position!
	     */
            keyptr = PBT_KEYLEN(tbuf);
	}

        keyptr += klen+1;
        BT_GETAJULONG(keyptr,&lv);
	keyptr += sizeof(ajulong);
        lenptr += sizeof(ajuint);
    }

    BT_GETAJULONG(keyptr,&lv);
    AJFREE(ncpage);

    return ret;
}




/* @funcstatic btreeCheckNumnode **********************************************
**
** Checks consistency of a numeric node page
**
** @param [w] cache [AjPBtcache] cache
** @param [r] page [const AjPBtpage] Page
** @return [AjBool] True on success
** @@
******************************************************************************/

static AjBool btreeCheckNumnode(AjPBtcache cache, const AjPBtpage page)
{
    ajuint i;
    ajuint m;
    const AjPBtpage bpage;
    unsigned char *tbuf;

    ajulong   overflow = 0L;
    ajulong blockno;
    ajuint totlen;
    ajulong left;
    ajulong right;
    ajulong prev;
    unsigned char *keyptr = NULL;
    unsigned char *valptr = NULL;
    ajulong lv;
    ajulong lk;
    ajulong lastk = 0L;

    AjBool ok = ajTrue;
    AjBool ret = ajTrue;

    if(!btreeDoExtra)
        return ret;

    bpage = page;
    tbuf = bpage->buf;
    GBT_TOTLEN(tbuf,&totlen);
    
    GBT_NKEYS(tbuf,&m);

    GBT_LEFT(tbuf,&left);
    GBT_RIGHT(tbuf,&right);
    GBT_BLOCKNUMBER(tbuf,&blockno);
    GBT_PREV(tbuf,&prev);
    GBT_OVERFLOW(tbuf,&overflow);

    if(!btreeCheckNodeHeader(cache, page, "numnode"))
        ret = ajFalse;

    keyptr =  PBT_KEYLEN(tbuf);
    valptr = keyptr + m*sizeof(ajulong);
    for(i=0;i<m;i++)
    {
        BT_GETAJULONG(valptr,&lv);
        BT_GETAJULONG(keyptr,&lk);

        if(lk <= lastk)
        {
            ok = ajFalse;
            ajWarn("numnode:%Lu lk[%u] %12Lu <= %12Lu in %S",
                   page->pagepos, i, lk, lastk,
                   cache->filename);
        }
        lastk = lk;
        
	keyptr += sizeof(ajulong);
	valptr += sizeof(ajulong);
    }

    BT_GETAJULONG(valptr,&lv);

    if(!ok)
    {
        ret = ajFalse;
        btreeDumpNumnode(cache, page);
    }
    
    return ret;
}




/* @funcstatic btreeCheckNodeHeader *******************************************
**
** Checks consistency of any node header
**
** @param [w] cache [AjPBtcache] cache
** @param [r] page [const AjPBtpage] Page
** @param [r] type [const char*] Node type
** @return [AjBool] True on success
** @@
******************************************************************************/

static AjBool btreeCheckNodeHeader(AjPBtcache cache, const AjPBtpage page,
                                   const char* type)
{
    ajuint nkeys;
    unsigned char *tbuf;

    ajulong   overflow = 0L;
    ajulong blockno;
    ajuint totlen;
    ajulong left;
    ajulong right;
    ajulong prev;
    const char* nodetype;

    AjBool ret = ajTrue;
    
    tbuf = page->buf;
    nodetype = btreeNodetype(tbuf);

    GBT_TOTLEN(tbuf,&totlen);
    
    GBT_NKEYS(tbuf,&nkeys);

    GBT_LEFT(tbuf,&left);
    GBT_RIGHT(tbuf,&right);
    GBT_BLOCKNUMBER(tbuf,&blockno);
    GBT_PREV(tbuf,&prev);
    GBT_OVERFLOW(tbuf,&overflow);

    if(!strcmp(type, "node"))
    {
        if(!strcmp(nodetype, "numbucket"))
        {
            ret = ajFalse;
            ajWarn("%s:%Lu expected non-numeric node in %S",
                   nodetype,
                   cache->filename);
        }
    }
    else if(!strcmp(type, "numnode"))
    {
        if(strcmp(nodetype, "numbucket"))
        {
            ret = ajFalse;
            ajWarn("%s:%Lu expected numeric node in %S",
                   nodetype,
                   cache->filename);
        }
    }
    else
    {
        ret = ajFalse;
        ajWarn("%s:%Lu expected '%s' in %S",
               nodetype, type,
               cache->filename);
    }

    if(nkeys >= cache->sorder)
    {
        ret = ajFalse;
        ajWarn("%s:%Lu nkeys: %u order:%u sorder:%u in %S",
               nodetype, page->pagepos, nkeys,
               cache->order, cache->sorder,
               cache->filename);

    }
/*    
    if(nkeys > cache->order)
    {
        ret = ajFalse;
        ajWarn("%s:%Lu nkeys: %u order:%u",
               nodetype, page->pagepos, nkeys,
               cache->order, cache->sorder);

    }
*/
    if((blockno != page->pagepos))
    {
        ret = ajFalse;
        ajWarn("%s:%Lu blockno: %Lu in %S",
               nodetype, page->pagepos, blockno,
               cache->filename);
    }

    return ret;
}




/* @funcstatic btreeNocacheFetch *****************************************
**
** Fetch a cache page from disc
**
** @param [r] cache [const AjPBtcache] cache
** @param [w] cpage [AjPBtpage] cache page 
** @param [r] pagepos [ajulong] page number
**
** @return [void]
** @@
******************************************************************************/

static void btreeNocacheFetch(const AjPBtcache cache, AjPBtpage cpage,
                              ajulong pagepos)
{
    ajuint sum = 0;
    ajuint retries = 0;

    /* ajDebug("In btreeNoCacheFetch\n"); */
    
    if(fseek(cache->fp,pagepos,SEEK_SET))
	ajFatal("Seek error '%s' in btreeNocacheFetch file %S",
                strerror(ferror(cache->fp)), cache->filename);
    
    while(sum != cache->pagesize && retries != BT_MAXRETRIES)
    {
	sum += fread((void *)cpage->buf,1,cache->pagesize-sum,
		     cache->fp);
	++retries;
    }
    
    if(retries == BT_MAXRETRIES)
	ajFatal("Maximum retries (%u) reached in btreeCacheFetch for page %Lu",
		BT_MAXRETRIES,pagepos);

    cpage->pagepos = pagepos;
    
    return;
}




/* @funcstatic btreePageSetfree ***********************************************
**
** Clear a cache page and set as free
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] Pahge number
**
** @return [void]
** @@
******************************************************************************/

static void btreePageSetfree(AjPBtcache cache, ajulong pagepos)
{
    AjPBtpage page = NULL;
    unsigned char *p;
    ajuint nodetype;

    page = ajBtreeCacheRead(cache,pagepos);
    p = page->buf;
    AJCSET0(p, cache->pagesize); /* clear buffer to zeros */

    page->next = NULL;
    page->prev = NULL;
    nodetype = BT_FREEPAGE;
    SBT_NODETYPE(p,nodetype);
    page->dirty = BT_DIRTY;
    
    return;
}




/* @funcstatic btreePageClear *************************************************
**
** Clear a cache page
**
** @param [u] cache [AjPBtcache] cache
** @param [u] page [AjPBtpage] Page
**
** @return [void]
** @@
******************************************************************************/

static void btreePageClear(AjPBtcache cache, AjPBtpage page)
{
    unsigned char *p;

    p = page->buf;
    AJCSET0(p, cache->pagesize); /* clear buffer to zeros */

    page->next = NULL;
    page->prev = NULL;
    page->dirty = BT_CLEAN;

    return;
}




/* @func ajBtreeExit **********************************************************
**
** Cleans up B+tree indexing internal memory
**
** @return [void]
** @@
******************************************************************************/

void ajBtreeExit(void)
{
    void *tmpfree = NULL;
    
    ajDebug("ajBtreeExit\n");

    ajDebug("sync calls: %Lu Lock:%Lu Write: %Lu\n",
            statCallSync, statSyncLocked, statSyncWrite);

    ajDebug("Rootsync calls: %Lu Lock:%Lu (max %u) Unlock: %Lu (max %u)\n",
            statCallRootSync, statRootSyncLocked, statRootSyncMaxLocked,
            statRootSyncUnlocked, statRootSyncMaxUnlocked);

    ajDebug("   statSaveBucketNext: %u\n", statSaveBucketNext);
    while(statSaveBucketNext)
        btreeBucketFree(&statSaveBucket[--statSaveBucketNext]);
    AJFREE(statSaveBucket);
    statSaveBucketMax=0;
    statSaveBucketNext=0;

    ajDebug("   statSaveBucketEmptyNext: %u\n", statSaveBucketEmptyNext);
    while(statSaveBucketEmptyNext)
        btreeBucketFree(&statSaveBucketEmpty[--statSaveBucketEmptyNext]);
    AJFREE(statSaveBucketEmpty);
    statSaveBucketEmptyMax=0;
    statSaveBucketEmptyNext=0;

    ajDebug("   statSavePriBucketNext: %u\n", statSavePriBucketNext);
    while(statSavePriBucketNext)
        btreePriBucketFree(&statSavePriBucket[--statSavePriBucketNext]);
    AJFREE(statSavePriBucket);
    statSavePriBucketMax=0;
    statSavePriBucketNext=0;

    ajDebug("   statSavePriBucketEmptyNext: %u\n", statSavePriBucketEmptyNext);
    while(statSavePriBucketEmptyNext)
        btreePriBucketFree(&statSavePriBucketEmpty[--statSavePriBucketEmptyNext]);
    AJFREE(statSavePriBucketEmpty);
    statSavePriBucketEmptyMax=0;
    statSavePriBucketEmptyNext=0;

    ajDebug("   statSaveSecBucketNext: %u\n", statSaveSecBucketNext);
    while(statSaveSecBucketNext)
        btreeSecBucketFree(&statSaveSecBucket[--statSaveSecBucketNext]);
    AJFREE(statSaveSecBucket);
    statSaveSecBucketMax=0;
    statSaveSecBucketNext=0;

    ajDebug("   statSaveSecBucketEmptyNext: %u\n", statSaveSecBucketEmptyNext);
    while(statSaveSecBucketEmptyNext)
        btreeSecBucketFree(&statSaveSecBucketEmpty[--statSaveSecBucketEmptyNext]);
    AJFREE(statSaveSecBucketEmpty);
    statSaveSecBucketEmptyMax=0;
    statSaveSecBucketEmptyNext=0;

    ajDebug("   statSaveNumBucketNext: %u\n", statSaveNumBucketNext);
    while(statSaveNumBucketNext)
        btreeNumBucketFree(&statSaveNumBucket[--statSaveNumBucketNext]);
    AJFREE(statSaveNumBucket);
    statSaveNumBucketMax=0;
    statSaveNumBucketNext=0;

    ajDebug("   statSaveSecIdNext: %u\n", statSaveSecIdNext);
    while(statSaveSecIdNext)
        ajStrDel(&statSaveSecId[--statSaveSecIdNext]);
    AJFREE(statSaveSecId);
    statSaveSecIdMax=0;
    statSaveSecIdNext=0;

    ajDebug("   statSaveBtreeIdNext: %u\n", statSaveBtreeIdNext);
    while(statSaveBtreeIdNext)
        btreeIdFree(&statSaveBtreeId[--statSaveBtreeIdNext]);
    AJFREE(statSaveBtreeId);
    statSaveBtreeIdMax=0;
    statSaveBtreeIdNext=0;

    ajDebug("   statSaveBtreePriNext: %u\n", statSaveBtreePriNext);
    while(statSaveBtreePriNext)
        btreePriFree(&statSaveBtreePri[--statSaveBtreePriNext]);
    AJFREE(statSaveBtreePri);
    statSaveBtreePriMax=0;
    statSaveBtreePriNext=0;

    ajDebug("Primary Arrays: New:%Lu Reuse:%Lu Delete:%Lu Free:%Lu\n",
                statCountAllocPriArrayNew, statCountAllocPriArrayReuse,
                statCountAllocPriArrayDel, statCountAllocPriArrayFree);

    ajDebug("Secondary Arrays: New:%Lu Reuse:%Lu Delete:%Lu Free:%Lu\n",
                statCountAllocSecArrayNew, statCountAllocSecArrayReuse,
                statCountAllocSecArrayDel, statCountAllocSecArrayFree);

    ajDebug("Split root:%Lu Pri:%Lu Sec:%Lu Hyb:%Lu Num:%Lu\n",
            statCallSplitRoot,statCallSplitPriRoot,statCallSplitSecRoot,
            statCallSplitHybRoot,statCallSplitNumRoot);

    ajDebug("Split leaf:%Lu Pri:%Lu Sec:%Lu Hyb:%Lu Num:%Lu\n",
            statCallSplitLeaf,statCallSplitPriLeaf,statCallSplitSecLeaf,
            statCallSplitHybLeaf,statCallSplitNumLeaf);

    ajDebug("Split reorder:%Lu Pri:%Lu Sec:%Lu Hyb:%Lu Num:%Lu\n",
            statCallReorderBuckets,
            statCallReorderPriBuckets,statCallReorderSecBuckets,
            statCallReorderHybBuckets,statCallReorderNumBuckets);

    ajStrDel(&btreeFieldnameTmp);
    ajTableMapDel(btreeFieldsTable, btreeFieldMapDel, NULL);
    ajTableFree(&btreeFieldsTable);

    tmpfree = (void *) btreeNodetypeNames;
    AJFREE(tmpfree);
    btreeNodetypeNames = NULL;
    
    return;
}




/* @func ajBtreeIdCmp *********************************************************
**
** Comparison function for sorting B+tree ID objects by database file number
** and offset.
**
** Using the file and position allows unique identifiers to be
** compared with other primary keys such bas accession number.
**
** @param [r] x [const void*] Standard argument. Item value.
** @param [r] y [const void*] Standard argument. Comparison item value.
** @return [ajint] Comparison result. Zero if equal, non-zero if different.
** @@
******************************************************************************/

ajint ajBtreeIdCmp(const void* x, const void* y)
{
    const AjPBtId idx;
    const AjPBtId idy;

    idx = (const AjPBtId) x;
    idy = (const AjPBtId) y;

    if(idx->dbno > idy->dbno)
        return 1;

    if(idx->dbno < idy->dbno)
        return -1;

    if(idx->offset > idy->offset)
        return 1;

    if(idx->offset < idy->offset)
        return -1;

    return 0;
}




/* @func ajBtreeIdHash *******************************************************
**
** Hash function for a table with a B+tree primary key
**
** @param [r] key [const void*] Standard argument. Table key.
** @param [r] hashsize [ajuint] Standard argument. Estimated Hash size.
** @return [ajuint] Hash value in range 0 to hashsize-1
** @@
******************************************************************************/

ajuint ajBtreeIdHash(const void* key, ajuint hashsize)
{
    const AjPBtId id;
    ajuint hash;
    ajulong ia;

    id = (const AjPBtId) key;

     if(!key)
        return 0;

    if(!hashsize)
        return 0;

    ia = id->offset + (ajulong) id->dbno;

    hash = (ia >> 2) % hashsize;

    return hash;
}


    

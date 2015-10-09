/* @source ajindex ************************************************************
**
** B+ Tree Indexing plus Disc Cache.
**
** @author Copyright (c) 2003 Alan Bleasby
** @version $Revision: 1.107 $
** @modified  subsequently heavily modified by Peter Rice
** @modified $Date: 2012/07/05 12:33:12 $ by $Author: rice $
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

#include "ajindex.h"
#include "ajfileio.h"
#include "ajutil.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#ifndef WIN32
#include <unistd.h>
#endif

#include <sys/types.h>


#ifdef WIN32
#include <io.h>
#define fileno _fileno
#define ftruncate _chsize_s
#endif

#if defined (AJ_IRIXLF)
#define usestat64 1
#endif

#define AJINDEX_DEBUG 0
#define AJINDEX_STATIC 0

static AjPBtpage btreeTestpage = NULL;

static AjBool btreeDoRootSync = AJFALSE;

static ajulong   statCallSync = 0L;
static ajulong   statCallRootSync = 0L;
static ajulong   statCallPriSplitroot = 0L;
static ajulong   statCallSecSplitroot = 0L;
static ajulong   statCallIdSplitroot = 0L;
static ajulong   statCallNumSplitroot = 0L;
static ajulong   statCallKeySplitleaf = 0L;
static ajulong   statCallSecSplitleaf = 0L;
static ajulong   statCallIdSplitleaf = 0L;
static ajulong   statCallNumSplitleaf = 0L;
static ajulong   statCallIdbucketsReorder = 0L;
static ajulong   statCallPribucketsReorder = 0L;
static ajulong   statCallSecbucketsReorder = 0L;
/*static ajulong   statCallHybbucketsReorder = 0L;*/
static ajulong   statCallNumbucketsReorder = 0L;
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

static AjPBtHit* statSaveBtreeHit = NULL;
static ajuint    statSaveBtreeHitNext = 0;
static ajuint    statSaveBtreeHitMax = 0;

static AjPBtHitref* statSaveBtreeHitref = NULL;
static ajuint       statSaveBtreeHitrefNext = 0;
static ajuint       statSaveBtreeHitrefMax = 0;

static AjPBtPri* statSaveBtreePri = NULL;
static ajuint    statSaveBtreePriNext = 0;
static ajuint    statSaveBtreePriMax = 0;

/* bucket cache */
static AjPIdbucket* statSaveIdbucket = NULL;
static AjPIdbucket* statSaveIdbucketEmpty = NULL;
static ajuint     statSaveIdbucketNext = 0;
static ajuint     statSaveIdbucketMax = 0;
static ajuint     statSaveIdbucketEmptyNext = 0;
static ajuint     statSaveIdbucketEmptyMax = 0;

/* primary bucket cache */
static AjPPribucket* statSavePribucket = NULL;
static AjPPribucket* statSavePribucketEmpty = NULL;
static ajuint        statSavePribucketNext = 0;
static ajuint        statSavePribucketMax = 0;
static ajuint        statSavePribucketEmptyNext = 0;
static ajuint        statSavePribucketEmptyMax = 0;

/* secondary bucket cache */
static AjPSecbucket* statSaveSecbucket = NULL;
static AjPSecbucket* statSaveSecbucketEmpty = NULL;
static ajuint        statSaveSecbucketNext = 0;
static ajuint        statSaveSecbucketMax = 0;
static ajuint        statSaveSecbucketEmptyNext = 0;
static ajuint        statSaveSecbucketEmptyMax = 0;

/* number bucket cache */
static AjPNumbucket* statSaveNumbucket = NULL;
static ajuint        statSaveNumbucketNext = 0;
static ajuint        statSaveNumbucketMax = 0;

static ajulong statCountAllocPriArrayNew = 0;
static ajulong statCountAllocPriArrayReuse = 0;
static ajulong statCountAllocPriArrayDel = 0;
static ajulong statCountAllocPriArrayFree = 0;

static ajulong statCountAllocSecArrayNew = 0;
static ajulong statCountAllocSecArrayReuse = 0;
static ajulong statCountAllocSecArrayDel = 0;
static ajulong statCountAllocSecArrayFree = 0;

static char const **btreeNodetypeNames = NULL;

static AjPStr indexKeyword = NULL;
static AjPStr indexId = NULL;


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



BtreeOFielddef btreeFields[] =
{
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


static void          btreePrirootCreate(AjPBtcache cache);
static void          btreeSecrootCreate(AjPBtcache cache, ajulong rootpage);

static AjPBtpage     btreeIdentFind(AjPBtcache cache, const AjPStr key);
static AjBool        btreeKeyFind(AjPBtcache cache, const AjPStr key,
                                  ajulong *treeblock);
static AjPBtId       btreeIdentQueryId(AjPBtcache cache, const AjPStr key);
static AjPBtHit      btreeIdentQueryHit(AjPBtcache cache, const AjPStr key);
static AjPBtHitref   btreeIdentQueryHitref(AjPBtcache cache, const AjPStr key);
static void          btreeIdentFetchMulti(AjPBtcache cache, const AjPStr idname,
                                          ajulong rootblock,
                                          AjPList list);
static void          btreeIdentFetchMultiHit(AjPBtcache cache,
                                             ajulong rootblock,
                                             AjPList list);
static void          btreeIdentFetchMultiHitref(AjPBtcache cache,
                                                ajulong rootblock,
                                                AjPList list);

static AjBool        btreeKeyidMakeroot(AjPBtcache cache, AjPBtpage bucket);

static AjBool        btreeKeyidExists(AjPBtcache cache, const AjPStr key);
static AjPBtpage     btreeKeyidFind(AjPBtcache cache, const AjPStr key);
static AjBool        btreeKeyidInsert(AjPBtcache cache, const AjPStr id);

static AjPBtpage     btreePrimaryFetchFindleafWild(AjPBtcache cache,
                                                   const AjPStr key);

static void          btreeCacheSync(AjPBtcache cache, ajulong rootpage);
static void          btreeCacheRootSync(AjPBtcache cache, ajulong rootpage);

static void          btreeCacheWriteCompress(AjPBtcache cache,
                                             AjPBtpage cpage,
                                             ajulong pagepos,
                                             ajuint pagesize);
static void          btreeCacheWriteUncompress(AjPBtcache cache,
                                               AjPBtpage cpage,
                                               ajulong pagepos,
                                               ajuint pagesize);

static ajulong       btreeCacheCompress(AjPBtcache thys);

static ajulong       btreeCacheUncompress(AjPBtcache thys);

static ajulong       btreePageposCompress(ajulong oldpos,
                                          const AjPTable newpagetable,
                                          const char* desc);

static ajulong       btreePageposUncompress(ajulong oldpos,
                                            const AjPTable newpagetable,
                                            const char* desc);

static AjPBtpage     btreePripageNew(AjPBtcache cache);
static AjPBtpage     btreeSecpageNew(AjPBtcache cache);

static ajuint        btreePageGetSizeIdbucket(const AjPBtpage page,
                                            ajuint refcount);
static ajuint        btreePageGetSizeNode(const AjPBtpage page);
static ajuint        btreePageGetSizeNumbucket(const AjPBtpage page,
                                               ajuint refcount);
static ajuint        btreePageGetSizeNumnode(const AjPBtpage page);
static ajuint        btreePageGetSizePribucket(const AjPBtpage page);
static ajuint        btreePageGetSizeSecbucket(const AjPBtpage page);

static AjBool        btreePageCompress(AjPBtpage page,
                                       const AjPTable newpagetable,
                                       ajuint refcount);
static void          btreePageCompressIdbucket(AjPBtpage page,
                                               const AjPTable newpagetable,
                                               ajuint refcount);
static void          btreePageCompressNode(AjPBtpage page,
                                           const AjPTable newpagetable);
static void          btreePageCompressNumnode(AjPBtpage page,
                                              const AjPTable newpagetable);
static void          btreePageCompressPribucket(AjPBtpage page,
                                                const AjPTable newpagetable);
static AjBool        btreePageUncompress(AjPBtpage page,
                                         const AjPTable newpagetable,
                                         ajuint refcount);
static void          btreePageUncompressIdbucket(AjPBtpage page,
                                                 const AjPTable newpagetable,
                                                 ajuint refcount);
static void          btreePageUncompressNode(AjPBtpage page,
                                             const AjPTable newpagetable);
static void          btreePageUncompressNumnode(AjPBtpage page,
                                                const AjPTable newpagetable);
static void          btreePageUncompressPribucket(AjPBtpage page,
                                                  const AjPTable newpagetable);

static void          btreeNocacheFetch(const AjPBtcache cache, AjPBtpage cpage,
                                       ajulong pagepos);
static void          btreeCacheFetchSize(AjPBtcache cache, AjPBtpage cpage,
                                         ajulong pagepos, ajuint pagesize);

static AjPBtpage     btreePricacheBucketnew(AjPBtcache cache);
static AjPBtpage     btreePricacheControl(AjPBtcache cache, ajulong pagepos,
                                          AjBool isread);
static void          btreePricacheDestage(AjPBtcache cache, AjPBtpage cpage);
static void          btreePricacheFetch(AjPBtcache cache, AjPBtpage cpage,
                                        ajulong pagepos);
static AjPBtpage     btreePricacheLocate(AjPBtcache cache, ajulong page);
static AjPBtpage     btreePricacheLruUnlink(AjPBtcache cache);
static void          btreePricacheMruAdd(AjPBtcache cache, AjPBtpage cpage);
static AjPBtpage     btreePricacheNodenew(AjPBtcache cache);
static AjPBtpage     btreePricacheRead(AjPBtcache cache, ajulong pagepos);
static void          btreePricacheUnlink(AjPBtcache cache, AjPBtpage cpage);
static AjPBtpage     btreePricacheWrite(AjPBtcache cache, ajulong pagepos);

static AjPBtpage     btreeSeccacheBucketnew(AjPBtcache cache);
static AjPBtpage     btreeSeccacheControl(AjPBtcache cache, ajulong pagepos,
                                          AjBool isread);
static void          btreeSeccacheDestage(AjPBtcache cache, AjPBtpage cpage);
static void          btreeSeccacheFetch(AjPBtcache cache, AjPBtpage cpage,
                                        ajulong pagepos);
static AjPBtpage     btreeSeccacheLocate(AjPBtcache cache, ajulong page);
static AjPBtpage     btreeSeccacheLruUnlink(AjPBtcache cache);
static void          btreeSeccacheMruAdd(AjPBtcache cache, AjPBtpage cpage);
static AjPBtpage     btreeSeccacheNodenew(AjPBtcache cache);
static AjPBtpage     btreeSeccacheRead(AjPBtcache cache, ajulong pagepos);
static void          btreeSeccacheUnlink(AjPBtcache cache, AjPBtpage cpage);
static AjPBtpage     btreeSeccacheWrite(AjPBtcache cache, ajulong pagepos);

static AjPBtpage     btreePrimaryFindInode(AjPBtcache cache, AjPBtpage page,
                                           const AjPStr item);
static AjPBtpage     btreePrimaryFindInodeWild(AjPBtcache cache, AjPBtpage page,
                                               const AjPStr item);
static AjPBtpage     btreeKeyidFindINode(AjPBtcache cache, AjPBtpage page,
                                         const AjPStr item);
static ajulong       btreeSecTreeCount(AjPBtcache cache, ajulong rootblock);
static AjPList       btreeSecTreeList(AjPBtcache cache, ajulong rootblock);


static AjPBtpage     btreePrimaryPageDown(AjPBtcache cache,
                                          unsigned char *buf,
                                          const AjPStr item);
static AjPBtpage     btreeSecondaryPageDown(AjPBtcache cache,
                                            unsigned char *buf,
                                            const AjPStr item);
static ajuint        btreeIdbucketCount(AjPBtcache cache, ajulong pagepos);
static AjPBtId       btreeIdbucketFindDupId(AjPBtcache cache, ajulong pagepos,
                                            const AjPStr id, ajuint* ientry);
static ajulong       btreeIdbucketIdlist(AjPBtcache cache, ajulong pagepos,
                                         AjPList idlist);
static void          btreeIdbucketSort(AjPIdbucket thys);
static AjPIdbucket   btreeReadIdbucket(AjPBtcache cache, ajulong pagepos);
static void          btreeWriteIdbucket(AjPBtcache cache,
                                        const AjPIdbucket bucket,
                                        ajulong pagepos);
static void          btreeWriteIdbucketEmpty(AjPBtcache cache, ajulong pagepos);
static void          btreeWriteIdbucketId(AjPBtcache cache, ajulong pagepos,
                                          const AjPBtId btid, ajuint ientry);
static void          btreeIdbucketAdd(AjPBtcache cache, ajulong pagepos,
                                      const AjPBtId id);
static void 	     btreeIdbucketDel(AjPIdbucket *thys);
static void          btreeIdbucketFree(AjPIdbucket *thys);
static AjBool        btreeIdbucketsReorder(AjPBtcache cache, AjPBtpage page);
static void          btreeGetKeys(AjPBtcache cache, unsigned char *buf,
                                  AjPStr **keys, ajulong **ptrs);
static ajuint        btreeGetPointers(AjPBtcache cache, unsigned char *buf,
                                      ajulong **ptrs);
static ajulong       btreeGetBlockC(AjPBtcache cache, unsigned char *buf,
                                    const char* ckey);
static ajulong       btreeGetBlockN(AjPBtcache cache, unsigned char *buf,
                                    ajulong numkey);
static ajulong       btreeGetBlockS(AjPBtcache cache, unsigned char *buf,
                                    const AjPStr key);
static ajulong       btreeGetBlockFirstC(AjPBtcache cache, unsigned char *buf,
                                         const char* ckey, ajuint clen);
static ajulong       btreeGetBlockFirstN(AjPBtcache cache, unsigned char *buf,
                                         ajulong numkey);
static ajulong       btreeGetBlockFirstS(AjPBtcache cache, unsigned char *buf,
                                         const AjPStr key);
static void          btreeIdFree(AjPBtId *thys);
static ajint         btreeIdCompare(const void *a, const void *b);

static AjPIdbucket   btreeIdbucketNew(ajuint n, ajuint refcount);
static void          btreeWriteNode(AjPBtcache cache, AjPBtpage page,
                                    AjPStr const *keys, const ajulong *ptrs,
                                    ajuint nkeys);
static void          btreeWriteNodeSingle(AjPBtcache cache, AjPBtpage spage,
                                          const AjPStr key, ajulong lptr,
                                          ajulong rptr);
static AjBool        btreeNodeIsFull(const AjPBtcache cache, AjPBtpage page);
static AjBool        btreeNodeIsFullSec(const AjPBtcache cache,
                                        AjPBtpage page);
static void          btreePrimaryInsertNonfull(AjPBtcache cache, AjPBtpage page,
                                               const AjPStr key, ajulong less,
                                               ajulong greater);
static void          btreePriSplitroot(AjPBtcache cache);
static void          btreePriInsertKey(AjPBtcache cache, AjPBtpage page,
                                       const AjPStr key, ajulong less,
                                       ajulong greater);

static ajulong       btreeKeyInsertShift(AjPBtcache cache,
                                         AjPBtpage *retpage,
                                         const AjPStr key);
static void          btreePrimaryShift(AjPBtcache cache, AjPBtpage tpage);


#if 0
static AjPBtpage     btreeTraverseLeaves(AjPBtcache cache, AjPBtpage thys);
static void          btreeJoinLeaves(AjPBtcache cache);
#endif

static AjPBtpage     btreePrimaryPageDownWild(AjPBtcache cache,
                                              unsigned char *buf,
                                              const AjPStr key);
static void          btreeIdleafFetch(AjPBtcache cache, AjPBtpage page,
                                        AjPList list);
static void          btreePrileafFetch(AjPBtcache cache, AjPBtpage page,
                                       AjPList list);
static AjPBtpage     btreeKeySplitleaf(AjPBtcache cache, AjPBtpage spage);




static void          btreePriFree(AjPBtPri *thys);
static AjPPribucket  btreePribucketNew(ajuint n);
static void          btreePribucketDel(AjPPribucket *thys);
static void          btreePribucketFree(AjPPribucket *thys);
static ajulong       btreePribucketIdlist(AjPBtcache cache, ajulong pagepos,
                                          AjPList idlist);
static AjBool        btreePribucketFindId(AjPBtcache cache, ajulong pagepos,
                                          const AjPStr id, ajulong* treeblock);
static void          btreePribucketSort(AjPPribucket thys);
static AjPPribucket  btreePribucketRead(AjPBtcache cache, ajulong pagepos);
static void          btreeWritePribucket(AjPBtcache cache,
                                         const AjPPribucket bucket,
                                         ajulong pagepos);
static void          btreeWritePribucketEmpty(AjPBtcache cache,
                                              ajulong pagepos);
static void          btreePribucketAdd(AjPBtcache cache, ajulong pagepos,
                                       const AjPStr keyword, const AjPStr id);
static ajuint        btreeNumInPribucket(AjPBtcache cache, ajulong pagepos);
static ajint         btreeKeywordCompare(const void *a, const void *b);
static AjBool        btreePribucketsReorder(AjPBtcache cache,
                                            AjPBtpage leaf);




static AjBool        btreeSecbucketFindId(AjPBtcache cache, ajulong pagepos,
                                          const AjPStr id);
static ajulong       btreeSecbucketIdcount(AjPBtcache cache, ajulong pagepos);
static ajulong       btreeSecbucketIdlist(AjPBtcache cache, ajulong pagepos,
                                          AjPList idlist);
static AjPSecbucket  btreeReadSecbucket(AjPBtcache cache, ajulong pagepos);
static void          btreeWriteSecbucket(AjPBtcache cache,
					 const AjPSecbucket bucket,
					 ajulong pagepos);
static void          btreeWriteSecbucketEmpty(AjPBtcache cache,
                                              ajulong pagepos);
static ajint         btreeKeywordIdCompare(const void *a, const void *b);
static AjPBtpage     btreeSecSplitleaf(AjPBtcache cache, AjPBtpage spage);

static AjPSecbucket  btreeSecbucketNew(ajuint n, ajuint idlen);
static void          btreeSecbucketDel(AjPSecbucket *thys);
static void          btreeSecbucketFree(AjPSecbucket *thys);
static void          btreeSecLeftLeaf(AjPBtcache cache, AjPBtKeywild wild);
static AjBool        btreeKeywildNextList(AjPBtcache cache, AjPBtKeywild wild);
static void          btreeReadAllSecLeaves(AjPBtcache cache, AjPList list);

static void          btreeSecbucketAdd(AjPBtcache cache, ajulong pagepos,
                                       const AjPStr id);
static AjBool        btreeSecbucketsReorder(AjPBtcache cache, AjPBtpage leaf);
static void          btreeSecSplitroot(AjPBtcache cache);
static ajuint        btreeNumInSecbucket(AjPBtcache cache, ajulong pagepos);
static void          btreeInsertKeySec(AjPBtcache cache, AjPBtpage page,
			               const AjPStr key, ajulong less,
			               ajulong greater);
static ajulong       btreeKeyidInsertShift(AjPBtcache cache, AjPBtpage *retpage,
                                           const AjPStr key);
static void          btreeKeyShiftSec(AjPBtcache cache, AjPBtpage tpage);
static void          btreeInsertNonfullSec(AjPBtcache cache, AjPBtpage page,
				           const AjPStr key, ajulong less,
				           ajulong greater);

static void          btreeStrDel(void** pentry, void* cl);

static void          btreeIdDelFromList(void** pentry, void* cl);
static void          btreeHitDelFromList(void** pentry, void* cl);
static void          btreeHitrefDelFromList(void** pentry, void* cl);

static void          btreeKeyFullSearchId(AjPBtcache cache,
                                          const AjPStr key,
                                          AjPList idlist);
static void          btreeKeyFullSearchHit(AjPBtcache cache,
                                           const AjPStr key,
                                           AjPList idlist);
static void          btreeKeyFullSearchHitref(AjPBtcache cache,
                                              const AjPStr key,
                                              AjPList idlist);
static void          btreeKeywordFullSearchId(AjPBtcache cache,
                                              const AjPStr key,
                                              AjPBtcache idcache,
                                              AjPList idlist);
static void          btreeKeywordFullSearchHit(AjPBtcache cache,
                                               const AjPStr key,
                                               AjPBtcache idcache,
                                               AjPList idlist);
static void          btreeKeywordFullSearchHitref(AjPBtcache cache,
                                                  const AjPStr key,
                                                  AjPBtcache idcache,
                                                  AjPList idlist);
static ajint         btreeIdOffsetCompare(const void *a, const void *b);
static ajint         btreeHitOffsetCompare(const void *a, const void *b);
static ajint         btreeHitrefOffsetCompare(const void *a, const void *b);
static ajint         btreeIdDbnoCompare(const void *a, const void *b);
static ajint         btreeHitDbnoCompare(const void *a, const void *b);
static ajint         btreeHitrefDbnoCompare(const void *a, const void *b);


static AjPBtMem      btreeAllocPriArray(AjPBtcache cache);
static void          btreeDeallocPriArray(AjPBtcache cache, AjPBtMem node);
static AjPBtMem      btreeAllocSecArray(AjPBtcache cache);
static void          btreeDeallocSecArray(AjPBtcache cache, AjPBtMem node);
static void          btreeFreePriArray(AjPBtcache cache);
static void          btreeFreeSecArray(AjPBtcache cache);



static ajulong       btreeIdentInsertShift(AjPBtcache cache, AjPBtpage *retpage,
                                           const AjPStr key);
static AjPBtpage     btreeIdSplitleaf(AjPBtcache cache, AjPBtpage spage);
static void          btreeIdInsertKey(AjPBtcache cache, AjPBtpage page,
				       const AjPStr key, ajulong less,
				       ajulong greater);
static void          btreeIdSplitroot(AjPBtcache cache);
static void          btreeIdentDupInsert(AjPBtcache cache, const AjPBtId newid,
                                         AjPBtId curid);
static ajulong       btreeIdbucketIdlistAll(AjPBtcache cache, ajulong pagepos,
                                            AjPList idlist);

static void          btreeGetNumKeys(AjPBtcache cache, unsigned char *buf,
				     ajulong **keys, ajulong **ptrs);
static void          btreeGetNumPointers(AjPBtcache cache, unsigned char *buf,
                                         ajulong **ptrs);
static void          btreeWriteNumNode(AjPBtcache cache, AjPBtpage spage,
				       const ajulong *keys, const ajulong *ptrs,
				       ajuint nkeys);
static ajulong       btreeNumbucketIdlist(AjPBtcache cache, ajulong pagepos,
                                          AjPList idlist);
static ajulong       btreeNumbucketBtidlist(AjPBtcache cache,
                                            ajulong pagepos,
                                            const AjPStr idname,
                                            AjPList idlist);
static ajulong       btreeNumbucketBthitlist(AjPBtcache cache,
                                             ajulong pagepos,
                                             AjPList idlist);
static ajulong       btreeNumbucketBthitreflist(AjPBtcache cache,
                                                ajulong pagepos,
                                                AjPList idlist);
static AjPNumbucket  btreeReadNumbucket(AjPBtcache cache, ajulong pagepos);
static void          btreeWriteNumbucket(AjPBtcache cache,
                                         const AjPNumbucket bucket,
                                         ajulong pagepos);
static void          btreeWriteNumbucketEmpty(AjPBtcache cache,
                                              ajulong pagepos);
static void          btreeNumbucketDel(AjPNumbucket *thys);
static void          btreeNumbucketFree(AjPNumbucket *thys);
static void          btreeNumbucketAdd(AjPBtcache cache, ajulong pagepos,
                                       const AjPBtNumId num);
static AjPBtpage     btreeNumFind(AjPBtcache cache, const ajulong key);
static AjPBtpage     btreeNumFindINode(AjPBtcache cache, AjPBtpage page,
				       ajulong item);
static AjPBtpage     btreeNumPageFromKey(AjPBtcache cache, unsigned char *buf,
					 ajulong key);
static ajuint        btreeNumInNumbucket(AjPBtcache cache, ajulong pagepos);
static AjBool        btreeReorderNumbuckets(AjPBtcache cache, AjPBtpage leaf);
static AjPNumbucket  btreeNumbucketNew(ajuint n, ajuint nref);
static ajint         btreeNumIdCompare(const void *a, const void *b);
static AjBool        btreeNumNodeIsFull(const AjPBtcache cache,
					AjPBtpage page);
static void          btreeNumInsert(AjPBtcache cache, const AjPBtNumId num,
                                    AjPBtpage page);
static void          btreeNumInsertNonfull(AjPBtcache cache, AjPBtpage page,
					   ajulong key, ajulong less,
					   ajulong greater);
static void          btreeNumInsertKey(AjPBtcache cache, AjPBtpage page,
				       ajulong key, ajulong less,
				       ajulong greater);
static void          btreeNumSplitroot(AjPBtcache cache);
static void          btreeNumKeyShift(AjPBtcache cache, AjPBtpage tpage);
static ajulong       btreeNumInsertShift(AjPBtcache cache, AjPBtpage *retpage,
					 ajulong key);
static AjPBtpage     btreeNumSplitleaf(AjPBtcache cache, AjPBtpage spage);


static ajulong       btreeFindIdentBalanceOne(AjPBtcache cache,
                                              ajulong thisNode,
                                              ajulong leftNode,
                                              ajulong rightNode,
                                              ajulong lAnchor, ajulong rAnchor,
                                              const AjPBtId btid);
static void          btreeFindHybMinOne(AjPBtcache cache, ajulong pagepos,
                                        const AjPStr key);

static AjBool        btreeRemoveIdentEntryOne(AjPBtcache cache,ajulong pagepos,
                                              const AjPBtId btid);

static void          btreeAdjustHybbucketsOne(AjPBtcache cache,
                                              AjPBtpage leaf);

static ajulong       btreeRebalanceHybOne(AjPBtcache cache, ajulong thisNode,
                                          ajulong leftNode, ajulong rightNode,
                                          ajulong lAnchor, ajulong rAnchor);

static ajulong       btreeShiftHybOne(AjPBtcache cache, ajulong thisNode,
                                      ajulong balanceNode, ajulong anchorNode);

static ajulong       btreeMergeHybOne(AjPBtcache cache, ajulong thisNode,
                                      ajulong mergeNode, ajulong anchorNode);

static ajulong       btreeCollapseRootHybOne(AjPBtcache cache, ajulong pagepos);

static AjBool        btreeDeleteIdentIdTwo(AjPBtcache cache,
                                           const AjPBtId btid,
                                           AjPBtId did);

static ajulong       btreeFindHybBalanceTwo(AjPBtcache cache, ajulong thisNode,
                                            ajulong leftNode, ajulong rightNode,
                                            ajulong lAnchor, ajulong rAnchor,
                                            ajulong key);

static void          btreeFindHybMinTwo(AjPBtcache cache, ajulong pagepos,
                                        ajulong key);

static AjBool        btreeRemoveHybEntryTwo(AjPBtcache cache, ajulong pagepos,
                                            ajulong key);

static void          btreeAdjustHybbucketsTwo(AjPBtcache cache, AjPBtpage leaf);

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

static void          btreeAdjustPribucketsTwo(AjPBtcache cache, AjPBtpage leaf);

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

static void          btreeAdjustPribucketsOne(AjPBtcache cache, AjPBtpage leaf);

static ajulong       btreeRebalancePriOne(AjPBtcache cache, ajulong thisNode,
                                          ajulong leftNode, ajulong rightNode,
                                          ajulong lAnchor, ajulong rAnchor);

static ajulong       btreeShiftPriOne(AjPBtcache cache, ajulong thisNode,
                                      ajulong balanceNode, ajulong anchorNode);

static ajulong       btreeMergePriOne(AjPBtcache cache, ajulong thisNode,
                                      ajulong mergeNode, ajulong anchorNode);

static ajulong       btreeCollapseRootPriOne(AjPBtcache cache, ajulong pagepos);

static AjBool        btreeIsSecEmpty(AjPBtcache cache);

static AjBool        btreeBucketSplitCalc(ajuint totalkeys,
                                          ajuint totalbickets,
                                          ajuint maxbucketsize,
                                          ajuint *leftbuckets,
                                          ajuint *leftmax,
                                          ajuint *leftkeys,
                                          ajuint *rightbuckets,
                                          ajuint *rightmax,
                                          ajuint *rightkeys);
static AjBool        btreeBucketCalc(ajuint totalkeys, ajuint totalbuckets,
                                     ajuint maxbucketsize,
                                     ajuint *newbuckets, ajuint *newmax);
static const char*   btreeNodetype(const unsigned char* buf);
static void          btreeStatNumnode(AjPBtcache cache, const AjPBtpage page);
static AjBool        btreeCheckNode(AjPBtcache cache, const AjPBtpage page);
static AjBool        btreeCheckNumnode(AjPBtcache cache, const AjPBtpage page);
static AjBool        btreeCheckNodeHeader(AjPBtcache cache,
                                          const AjPBtpage page,
                                          const char* type);
static void          btreePripageClear(AjPBtcache cache, AjPBtpage page);
static void          btreeSecpageClear(AjPBtcache cache, AjPBtpage page);
static void          btreePripageSetfree(AjPBtcache cache, ajulong pagepos);
static void          btreeSecpageSetfree(AjPBtcache cache, ajulong pagepos);
static void          btreeFieldInit(void);
static BtreePField   btreeFieldNewField(const BtreePFielddef field);
static BtreePField   btreeFieldNewC(const char* nametxt);
static BtreePField   btreeFieldNewS(const AjPStr name);
static void          btreeFieldDel(BtreePField *Pfield);
static void          btreeFieldMapDel(void** key, void** value, void* cl);

#if AJINDEX_STATIC
/*
** debugging functions - define only if used in compiled code
*/
static AjBool        btreeKeyidVerify(AjPBtcache cache, ajulong rootblock,
                                      const AjPStr id);
static AjPList       btreeKeyidListDuplicates(AjPBtcache cache,
                                              const AjPStr key);
static void          btreeLockTest(AjPBtcache cache);
static void          btreePageDump(const AjPBtcache cache,
                                   const AjPBtpage page);
#endif

#if 0
/*
** functions only used in commented-out code
** e.g. overflow pages
*/

static void          btreeInsertIdOnly(AjPBtcache cache,
                                       const AjPBtPri pri);
static AjPBtpage     btreeCacheWriteIdbucket(AjPBtcache cache,
                                             ajulong pagepos);
static AjBool        btreeHybbucketsReorder(AjPBtcache cache, AjPBtpage leaf);
static void          btreeHybbucketAdd(AjPBtcache cache, ajulong pagepos,
                                       const AjPBtHybrid id);
static ajulong       btreeHybbucketIdlist(AjPBtcache cache, ajulong pagepos,
                                          AjPList idlist);
static AjBool        btreeIdbucketSplitCalc(ajuint totalkeys,
                                            ajuint totalbickets,
                                            ajuint maxbucketsize,
                                            ajuint *leftbuckets,
                                            ajuint *leftmax,
                                            ajuint *leftkeys,
                                            ajuint *rightbuckets,
                                            ajuint *rightmax,
                                            ajuint *rightkeys);
static AjBool        btreeIdbucketCalc(ajuint totalkeys, ajuint totalbuckets,
                                       ajuint maxbucketsize,
                                       ajuint *newbuckets, ajuint *newmax);
static void          btreeHybbucketAddFull(AjPBtcache cache, ajulong pagepos,
                                           const AjPBtHybrid id);
static void          btreeIdbucketAddFull(AjPBtcache cache, ajulong pagepos,
                                          const AjPBtId id);
static void          btreeSecbucketAddFull(AjPBtcache cache, ajulong pagepos,
                                           const AjPStr id);
static void          btreePribucketAddFull(AjPBtcache cache, ajulong pagepos,
                                           const AjPStr keyword,
                                           const AjPStr id);
static AjPBtpage     btreeCacheWriteOverflownew(AjPBtcache cache);
#endif




/* @funcstatic btreeFieldInit *************************************************
**
** Initialises the table for named fields
**
** @return [void]
**
** @release 6.4.0
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
        field = btreeFieldNewField(&btreeFields[i]);
        ajTablePut(btreeFieldsTable,
                   field->Extension,
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
**
** @release 6.4.0
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
**
** @release 6.4.0
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
**
** @release 6.4.0
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
**
** @release 6.4.0
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




/* @funcstatic btreeFieldMapDel ***********************************************
**
** Deletes entries from the MSF internal table. Called for each entry in turn.
**
** @param [d] key [void**] Standard argument, table key.
** @param [d] value [void**] Standard argument, table data item.
** @param [r] cl [void*] Standard argument, usually NULL
** @return [void]
**
** @release 6.4.0
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
**
** @release 6.4.0
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
**
** @release 6.4.0
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
**
** @release 6.4.0
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
**
** @release 6.4.0
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
**
** @release 6.4.0
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
**
** @release 6.4.0
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




/* @func ajBtreeIdcacheNewC ***************************************************
**
s** Open a b+tree index file and initialise a cache object
**
** @param [r] filetxt [const char *] name of file
** @param [r] exttxt [const char *] extension of file
** @param [r] idirtxt [const char *] index file directory
** @param [r] mode [const char *] opening mode
** @param [r] compressed [AjBool] Compressed index flag
** @param [r] kwlimit [ajuint] Max key size
** @param [r] refcount [ajuint] Extra files for each entry
** @param [r] pripagesize [ajuint] Primary pagesize
** @param [r] secpagesize [ajuint] Secondary pagesize
** @param [r] pricachesize [ajuint] size of primary cache
** @param [r] seccachesize [ajuint] size of secondary cache
** @param [r] pripagecount [ajulong] Primary page count
** @param [r] secpagecount [ajulong] Secondary page count
** @param [r] order [ajuint] Tree order
** @param [r] fill [ajuint] Number of entries per bucket
** @param [r] level [ajuint] level of tree
** @param [r] sorder [ajuint] order of secondary tree
** @param [r] sfill [ajuint] Number of entries per secondary bucket
** @param [r] count [ajulong] Number of entries in the index
** @param [r] countall [ajulong] Number of total entries in the index
**
** @return [AjPBtcache] initialised disc block cache structure
**
** @release 3.0.0
** @@
******************************************************************************/

AjPBtcache ajBtreeIdcacheNewC(const char *filetxt, const char *exttxt,
                              const char *idirtxt, const char *mode,
                              AjBool compressed,
                              ajuint kwlimit, ajuint refcount,
                              ajuint pripagesize, ajuint secpagesize,
                              ajuint pricachesize, ajuint seccachesize,
                              ajulong pripagecount, ajulong secpagecount,
                              ajuint order, ajuint fill, ajuint level,
                              ajuint sorder, ajuint sfill,
                              ajulong count, ajulong countall)
{
    AjPBtcache cache = NULL;
    AjPBtpage  page = NULL;
#if defined (usestat64)
    struct stat64 buf;
#else
    struct stat buf;
#endif

    ajulong filelen = 0L;
    AjBool douncompress = ajFalse;
    AjBool writemode = ajFalse;
    AjBool okcache = ajTrue;

    AJNEW0(cache);

    cache->prilistLength = 0;
    cache->seclistLength = 0;

    cache->plru   = NULL;
    cache->pmru   = NULL;
    cache->slru   = NULL;
    cache->smru   = NULL;
    
    cache->replace    = ajStrNew();
    cache->numreplace = 0L;
    
    if(pripagesize>0)
	cache->pripagesize = pripagesize;
    else
	cache->pripagesize = BT_PAGESIZE;

    cache->plevel       = level;
    cache->porder       = order;
    cache->pnperbucket  = fill;
    cache->pricachesize = pricachesize;
    cache->refcount = refcount;

    cache->pripagecount = pripagecount;
    cache->secpagecount = secpagecount;

    cache->sorder = sorder;
    cache->slevel = 0;
    cache->snperbucket = sfill;

    if(secpagesize > 0)
	cache->secpagesize = secpagesize;
    else
	cache->secpagesize = cache->pripagesize;

    if(seccachesize > 0)
        cache->seccachesize = seccachesize;
    else
        cache->seccachesize = cache->pricachesize;

    cache->countunique = count;
    cache->countall = countall;
    cache->keylimit = kwlimit;

    cache->compressed = compressed;


    cache->bmem = NULL;
    cache->tmem = NULL;

    cache->bsmem = NULL;
    cache->tsmem = NULL;

    cache->pripagetable = ajTableulongNewConst(cache->pricachesize);
    cache->secpagetable = ajTableulongNewConst(cache->seccachesize);

    cache->filename = ajStrNew();

    if(!*idirtxt)
        ajFmtPrintS(&cache->filename,"%s.%s",filetxt,exttxt);
    else if(idirtxt[strlen(idirtxt)-1] == SLASH_CHAR)
        ajFmtPrintS(&cache->filename,"%s%s.%s",idirtxt,filetxt,exttxt);
    else
        ajFmtPrintS(&cache->filename,"%s%s%s.%s",idirtxt,SLASH_STRING,
		    filetxt,exttxt);

    if(cache->porder < 4)
    {
        ajErr("cache '%S' pagesize %u order %u too small, increase pagesize",
              cache->filename, cache->pripagesize, cache->porder);
        okcache = ajFalse;
    }

    if(cache->pnperbucket < 4)
    {
        ajErr("cache '%S' pagesize %u fill %u too small, increase pagesize",
              cache->filename, cache->pripagesize, cache->pnperbucket);
        okcache = ajFalse;
    }

    if(cache->sorder < 4)
    {
        ajErr("cache '%S' pagesize %u srder %u too small, increase pagesize",
              cache->filename, cache->pripagesize, cache->sorder);
        okcache = ajFalse;
    }

    if(cache->snperbucket < 4)
    {
        ajErr("cache '%S' pagesize %u sfill %u too small, increase pagesize",
              cache->filename, cache->secpagesize, cache->snperbucket);
        okcache = ajFalse;
    }

    if(!okcache)
        return NULL;
        

    cache->fp = fopen(MAJSTRGETPTR(cache->filename),mode);

    if(!cache->fp)
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
    else if(ajCharMatchC(mode, "wb+")) /* create */
    {
        writemode = ajTrue;
    }
    else
    {
        ajWarn("ajBtreeIdcacheNewC unknown mode '%s'", mode);
    }
    
    cache->totsize     = filelen;
    cache->filesize    = filelen;

    if(douncompress)
        btreeCacheUncompress(cache);

    /* create or lock the root page */

    if(writemode)
        btreePrirootCreate(cache);
    else
    {
        page = btreePricacheRead(cache,0L);
        page->dirty = BT_LOCK;
    }

    return cache;
}




/* @func ajBtreeIdcacheNewS ***************************************************
**
** Open a b+tree index file and initialise a cache object
**
** @param [r] file [const AjPStr] name of file
** @param [r] ext [const AjPStr] extension of file
** @param [r] idir [const AjPStr] index file directory
** @param [r] mode [const char *] opening mode
** @param [r] compressed [AjBool] Compressed index flag
** @param [r] refcount [ajuint] Extra files for each entry
** @param [r] kwlimit [ajuint] Max key size
** @param [r] pripagesize [ajuint] Primary pagesize
** @param [r] secpagesize [ajuint] Secondary pagesize
** @param [r] pricachesize [ajuint] size of primary cache
** @param [r] seccachesize [ajuint] size of secondary cache
** @param [r] pripagecount [ajulong] Primary page count
** @param [r] secpagecount [ajulong] Secondary page count
** @param [r] order [ajuint] Tree order
** @param [r] fill [ajuint] Number of entries per bucket
** @param [r] level [ajuint] level of tree
** @param [r] sorder [ajuint] order of secondary tree
** @param [r] sfill [ajuint] Number of entries per secondary bucket
** @param [r] count [ajulong] Number of entries in the index
** @param [r] countall [ajulong] Number of total entries in the index
**
** @return [AjPBtcache] initialised disc block cache structure
**
** @release 6.4.0
** @@
******************************************************************************/

AjPBtcache ajBtreeIdcacheNewS(const AjPStr file, const AjPStr ext,
                              const AjPStr idir, const char *mode,
                              AjBool compressed,
                              ajuint refcount,
                              ajuint kwlimit,
                              ajuint pripagesize, ajuint secpagesize,
                              ajuint pricachesize, ajuint seccachesize,
                              ajulong pripagecount, ajulong secpagecount,
                              ajuint order, ajuint fill, ajuint level,
                              ajuint sorder, ajuint sfill,
                              ajulong count, ajulong countall)
{
    return ajBtreeIdcacheNewC(MAJSTRGETPTR(file), MAJSTRGETPTR(ext),
                              MAJSTRGETPTR(idir), mode,
                              compressed,  refcount, kwlimit,
                              pripagesize, secpagesize,
                              pricachesize, seccachesize,
                              pripagecount, secpagecount,
                              order, fill, level,
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
**
** @release 6.4.0
** @@
******************************************************************************/

AjPBtcache ajBtreeCacheNewReadC(const char *filetxt, const char *exttxt,
                                const char *idirtxt)
{
    AjPBtcache cache = NULL;

    ajuint pripagesize;
    ajuint pricachesize;
    ajuint secpagesize;
    ajuint seccachesize;
    ajuint order;
    ajuint sorder;
    ajuint fill;
    ajuint sfill;
    ajuint level;
    ajulong pripagecount;
    ajulong secpagecount;
    ajulong count;
    ajulong countall;
    ajuint kwlimit;
    ajuint idlimit;
    ajuint refcount;
    AjBool secondary = ajTrue;
    AjBool compressed = ajFalse;
    
/* first read the parameter file */
   
    if(!ajBtreeReadParamsC(filetxt, exttxt, idirtxt,
                           &secondary, &compressed,
                           &kwlimit, &idlimit, &refcount,
                           &pripagesize, &secpagesize,
                           &pricachesize, &seccachesize,
                           &pripagecount, &secpagecount,
                           &order, &fill, &level,
                           &sorder, &sfill,
                           &count, &countall))
    {
         return ajFalse;
    }

    if(secondary)
        cache = ajBtreeSeccacheNewC(filetxt, exttxt, idirtxt, "rb",
                                    compressed,
                                    kwlimit, idlimit,
                                    pripagesize, secpagesize,
                                    pricachesize, seccachesize,
                                    pripagecount, secpagecount,
                                    order, fill, level,
                                    sorder, sfill,
                                    count, countall);
    else
        cache = ajBtreeIdcacheNewC(filetxt, exttxt, idirtxt, "rb",
                                   compressed,
                                   kwlimit, refcount,
                                   pripagesize, secpagesize,
                                   pricachesize, seccachesize,
                                   pripagecount, secpagecount,
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
**
** @release 6.4.0
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
**
** @release 6.4.0
** @@
******************************************************************************/

AjPBtcache ajBtreeCacheNewUpdateC(const char *filetxt, const char *exttxt,
                                  const char *idirtxt)
{
    AjPBtcache cache = NULL;

    ajuint pripagesize;
    ajuint secpagesize;
    ajuint pricachesize;
    ajuint seccachesize;
    ajuint order;
    ajuint sorder;
    ajuint fill;
    ajuint sfill;
    ajuint level;
    ajulong pripagecount;
    ajulong secpagecount;
    ajulong count;
    ajulong countall;
    ajuint kwlimit;
    ajuint idlimit;
    ajuint refcount;
    AjBool secondary = ajTrue;
    AjBool compressed = ajFalse;
    
/* first read the parameter file */
   
    if(!ajBtreeReadParamsC(filetxt, exttxt, idirtxt,
                           &secondary, &compressed,
                           &kwlimit, &idlimit, &refcount,
                           &pripagesize, &secpagesize,
                           &pricachesize, &seccachesize,
                           &pripagecount, &secpagecount,
                           &order, &fill, &level,
                           &sorder, &sfill,
                           &count, &countall))
    {
         return ajFalse;
    }

    if(secondary)
        cache = ajBtreeSeccacheNewC(filetxt, exttxt, idirtxt, "rb+",
                                    compressed, kwlimit, idlimit,
                                    pripagesize, secpagesize,
                                    pricachesize, seccachesize,
                                    pripagecount, secpagecount,
                                    order, fill, level,
                                    sorder, sfill,
                                    count, countall);
    else
        cache = ajBtreeIdcacheNewC(filetxt, exttxt, idirtxt, "rb+",
                                   compressed, kwlimit, refcount,
                                   pripagesize, secpagesize,
                                   pricachesize, seccachesize,
                                   pripagecount, secpagecount,
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
**
** @release 6.4.0
** @@
******************************************************************************/

AjPBtcache ajBtreeCacheNewUpdateS(const AjPStr file, const AjPStr ext,
                                  const AjPStr idir)
{
    return ajBtreeCacheNewUpdateC(MAJSTRGETPTR(file), MAJSTRGETPTR(ext),
                                  MAJSTRGETPTR(idir));
}




/* @funcstatic btreePripageNew ************************************************
**
** Construct a cache page object
**
** @param [u] cache [AjPBtcache] cache
**
** @return [AjPBtpage] initialised disc block cache structure
**
** @release 2.8.0
** @@
******************************************************************************/

static AjPBtpage btreePripageNew(AjPBtcache cache)
{
    AjPBtpage thys = NULL;
    ajuint nodetype;

    /* ajDebug("In btreePripageNew\n"); */

    AJNEW0(thys);
    AJCNEW0(thys->buf,cache->pripagesize);
    nodetype     = BT_FREEPAGE;
    SBT_NODETYPE(thys->buf,nodetype);

    thys->next = NULL;
    thys->prev = NULL;
    
    ++cache->prilistLength;

    
    return thys;
}




/* @funcstatic btreeSecpageNew ************************************************
**
** Construct a cache secondary page object
**
** @param [u] cache [AjPBtcache] cache
**
** @return [AjPBtpage] initialised disc block cache structure
**
** @release 2.8.0
** @@
******************************************************************************/

static AjPBtpage btreeSecpageNew(AjPBtcache cache)
{
    AjPBtpage thys = NULL;
    ajuint nodetype;

    /* ajDebug("In btreeSecpageNew\n"); */

    AJNEW0(thys);
    AJCNEW0(thys->buf,cache->secpagesize);

    nodetype     = BT_SECFREEPAGE;
    SBT_NODETYPE(thys->buf,nodetype);

    thys->next = NULL;
    thys->prev = NULL;
    
    ++cache->seclistLength;

    
    return thys;
}




/* @funcstatic btreeIdbucketNew ***********************************************
**
** Construct an id bucket object
**
** @param [r] n [ajuint] Number of IDs
** @param [r] refcount [ajuint] Number pf reference offsets
** @return [AjPIdbucket] initialised id bucket
**
** @release 2.8.0
** @@
******************************************************************************/

static AjPIdbucket btreeIdbucketNew(ajuint n, ajuint refcount)
{
    AjPIdbucket bucket = NULL;
    ajuint i;

    /* ajDebug("In btreeIdbucketNew\n"); */
    
    if(n)
    {
        if(statSaveIdbucketNext)
        {
            bucket = statSaveIdbucket[--statSaveIdbucketNext];
            for(i=0;i<bucket->Maxentries;++i)
                ajStrAssignClear(&bucket->Ids[i]->id);
            if(n > bucket->Maxentries)
            {
                AJCRESIZE0(bucket->keylen,bucket->Maxentries,n);
                AJCRESIZE0(bucket->Ids,bucket->Maxentries,n);
                for(i=bucket->Maxentries;i<n;++i)
                    bucket->Ids[i] = ajBtreeIdNew(refcount);
                bucket->Maxentries = n;
            }
        }
        else
        {
            AJNEW0(bucket);
            AJCNEW0(bucket->Ids,n);
            AJCNEW0(bucket->keylen,n);
            for(i=0;i<n;++i)
                bucket->Ids[i] = ajBtreeIdNew(refcount);
            bucket->Maxentries = n;
            
        }
        
    }
    else 
    {
        if(statSaveIdbucketEmptyNext)
            bucket = statSaveIdbucketEmpty[--statSaveIdbucketEmptyNext];
        else
            AJNEW0(bucket);
    }

    bucket->NodeType = BT_IDBUCKET;
    bucket->Nentries = n;
    bucket->Overflow = 0L;
    
    return bucket;
}




/* @funcstatic btreePricacheLocate ********************************************
**
** Search for a page in the primary cache
**
** @param [u] cache [AjPBtcache] cache structure
** @param [r] page [ajulong] page number to locate
**
** @return [AjPBtpage]	pointer to page or NULL if not found
**
** @release 6.5.0
** @@
******************************************************************************/

static AjPBtpage btreePricacheLocate(AjPBtcache cache, ajulong page)
{
    AjPBtpage cpage = NULL;

    /*ajDebug("In btreePricacheLocate %Lu\n", page);*/
    

    cpage = ajTableFetchmodV(cache->pripagetable, (const void*) &page);

    if(!cpage)
        return NULL;

    cache->pricachehits++;

    if(!ajBtreePageIsPrimary(cpage))
        ajWarn("btreePricacheLocate secondary page %Lu '%s'",
               cpage->pagepos, btreeNodetype(cpage->buf));

    return cpage;
}




/* @funcstatic btreeSeccacheLocate ********************************************
**
** Search for a page in the secondary cache
**
** @param [u] cache [AjPBtcache] cache structure
** @param [r] page [ajulong] page number to locate
**
** @return [AjPBtpage]	pointer to page or NULL if not found
**
** @release 6.5.0
** @@
******************************************************************************/

static AjPBtpage btreeSeccacheLocate(AjPBtcache cache, ajulong page)
{
    AjPBtpage cpage = NULL;

    /*ajDebug("In btreeSeccacheLocate %Lu\n", page);*/
    

    cpage = ajTableFetchmodV(cache->secpagetable, (const void*) &page);

    if(!cpage)
        return NULL;

    cache->seccachehits++;

    if(ajBtreePageIsPrimary(cpage))
        ajWarn("btreeSeccacheLocate primary page %Lu '%s'",
               cpage->pagepos, btreeNodetype(cpage->buf));

    return cpage;
}




/* @funcstatic btreePricacheUnlink ********************************************
**
** Remove links to a primary cache page and return the address of the page
**
** @param [w] cache [AjPBtcache] cache structure
** @param [u] cpage [AjPBtpage] cache page
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void btreePricacheUnlink(AjPBtcache cache, AjPBtpage cpage)
{
    /* ajDebug("In btreePricacheUnlink\n"); */

    if(!ajBtreePageIsPrimary(cpage))
        ajWarn("btreePricacheUnlink secondary page %Lu '%s'",
               cpage->pagepos, btreeNodetype(cpage->buf));

    ajTableRemove(cache->pripagetable, (void*) &cpage->pagepos);

    if(cache->pmru == cpage)
    {
	cache->pmru = cpage->prev;

	if(cpage->prev)
	    cpage->prev->next = NULL;

	if(cache->plru == cpage)
	    cache->plru = NULL;
    }
    else if(cache->plru == cpage)
    {
	cache->plru = cpage->next;

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




/* @funcstatic btreeSeccacheUnlink ********************************************
**
** Remove links to a secondary cache page and return the address of the page
**
** @param [w] cache [AjPBtcache] cache structure
** @param [u] cpage [AjPBtpage] cache page
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void btreeSeccacheUnlink(AjPBtcache cache, AjPBtpage cpage)
{
    /* ajDebug("In btreeSeccacheUnlink\n"); */

    if(ajBtreePageIsPrimary(cpage))
        ajWarn("btreeSeccacheUnlink primary page %Lu '%s'",
               cpage->pagepos, btreeNodetype(cpage->buf));

    ajTableRemove(cache->secpagetable, (void*) &cpage->pagepos);

    if(cache->smru == cpage)
    {
	cache->smru = cpage->prev;

	if(cpage->prev)
	    cpage->prev->next = NULL;

	if(cache->slru == cpage)
	    cache->slru = NULL;
    }
    else if(cache->slru == cpage)
    {
	cache->slru = cpage->next;

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




/* @funcstatic btreePricacheMruAdd ********************************************
**
** Insert a primary cache page at the mru position
**
** @param [w] cache [AjPBtcache] cache structure
** @param [u] cpage [AjPBtpage] cache page
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void btreePricacheMruAdd(AjPBtcache cache, AjPBtpage cpage)
{

    /* ajDebug("In btreeCacheMruAdd\n"); */

    if(!ajBtreePageIsPrimary(cpage))
        ajWarn("btreePricacheMruAdd secondary page %Lu '%s'",
               cpage->pagepos, btreeNodetype(cpage->buf));

    ajTablePut(cache->pripagetable,
               (void*) &cpage->pagepos, (void*) cpage);

    cpage->prev = cache->pmru;
    cpage->next = NULL;

    if(cache->pmru)
	cache->pmru->next = cpage;

    if(!cache->plru)
	cache->plru = cpage;

    cache->pmru = cpage;

    return;
}




/* @funcstatic btreeSeccacheMruAdd ********************************************
**
** Insert a secondary cache page at the mru position
**
** @param [w] cache [AjPBtcache] cache structure
** @param [u] cpage [AjPBtpage] cache page
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void btreeSeccacheMruAdd(AjPBtcache cache, AjPBtpage cpage)
{

    /* ajDebug("In btreeSeccacheMruAdd\n"); */

    if(ajBtreePageIsPrimary(cpage))
        ajWarn("btreeSeccacheMruAdd primary page %Lu '%s'",
               cpage->pagepos, btreeNodetype(cpage->buf));

    ajTablePut(cache->secpagetable,
               (void*) &cpage->pagepos, (void*) cpage);

    cpage->prev = cache->smru;
    cpage->next = NULL;

    if(cache->smru)
	cache->smru->next = cpage;

    if(!cache->slru)
	cache->slru = cpage;

    cache->smru = cpage;

    return;
}




/* @funcstatic btreePricacheLruUnlink *****************************************
**
** Remove links to an LRU primary cache page
**
** @param [w] cache [AjPBtcache] cache structure
**
** @return [AjPBtpage]	pointer to unlinked cache page
**
** @release 6.5.0
** @@
******************************************************************************/

static AjPBtpage btreePricacheLruUnlink(AjPBtcache cache)
{
    AjPBtpage ret;

    /* ajDebug("In btreePricacheLruUnlink\n"); */

    if(cache->plru->dirty != BT_LOCK)
    {
	if(!cache->plru)
	    ajFatal("btreePricacheLruUnlink: No pages nodes found in cache %S",
                    cache->filename);

	ret = cache->plru;
	ret->next->prev = NULL;
	cache->plru = ret->next;
        ajTableRemove(cache->pripagetable, (void*) &ret->pagepos);

        if(!ajBtreePageIsPrimary(ret))
            ajWarn("btreePricacheLruUnlink secondary page %Lu '%s'",
                   ret->pagepos, btreeNodetype(ret->buf));

        return ret;
    }
    
    for(ret=cache->plru; ret; ret=ret->next)
	if(ret->dirty != BT_LOCK)
	    break;

    if(!ret)
	ajFatal("Too many locked cache page in primary cache %S. "
                "Try increasing cachesize (%u)",
                cache->filename, cache->pricachesize);

    btreePricacheUnlink(cache,ret);
    
    return ret;
}




/* @funcstatic btreeSeccacheLruUnlink *****************************************
**
** Remove links to an LRU secondary cache page
**
** @param [w] cache [AjPBtcache] cache structure
**
** @return [AjPBtpage]	pointer to unlinked cache page
**
** @release 6.5.0
** @@
******************************************************************************/

static AjPBtpage btreeSeccacheLruUnlink(AjPBtcache cache)
{
    AjPBtpage ret;

    /* ajDebug("In btreeSeccacheLruUnlink\n"); */

    if(cache->slru->dirty != BT_LOCK)
    {
	if(!cache->slru)
	    ajFatal("btreeSeccacheLruUnlink: No pages nodes found in cache %S",
                    cache->filename);

	ret = cache->slru;
	ret->next->prev = NULL;
	cache->slru = ret->next;

        ajTableRemove(cache->secpagetable, (void*) &ret->pagepos);

        if(ajBtreePageIsPrimary(ret))
            ajWarn("btreeSeccacheLruUnlink primary page %Lu '%s'",
                   ret->pagepos, btreeNodetype(ret->buf));

        return ret;
    }
    
    for(ret=cache->slru; ret; ret=ret->next)
	if(ret->dirty != BT_LOCK)
	    break;

    if(!ret)
	ajFatal("Too many locked cache page in secondary cache %S. "
                "Try increasing cachesize (%u)",
                cache->filename, cache->seccachesize);

    btreeSeccacheUnlink(cache,ret);
    
    return ret;
}




/* @funcstatic btreePricacheDestage *******************************************
**
** Destage a primary cache page
**
** @param [u] cache [AjPBtcache] cache
** @param [u] cpage [AjPBtpage] cache page
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void btreePricacheDestage(AjPBtcache cache, AjPBtpage cpage)
{
    ajuint written = 0;
    ajuint retries = 0;

    /* ajDebug("In btreePricacheDestage\n");*/

    if(!ajBtreePageIsPrimary(cpage))
        ajWarn("btreePricacheDestage secondary page %Lu '%s'",
               cpage->pagepos, btreeNodetype(cpage->buf));

    if(fseek(cache->fp, cpage->pagepos,SEEK_SET))
    {
        ajWarn("fseek %Lu failed for cache '%S' %d: '%s'",
               cpage->pagepos, cache->filename,
               ferror(cache->fp), strerror(ferror(cache->fp)));
	fseek(cache->fp,0L,SEEK_END);
    }

    while(written != cache->pripagesize && retries != BT_MAXRETRIES)
    {
	written += fwrite((void *)(cpage->buf+written),1,
                          cache->pripagesize-written,
			  cache->fp);
	++retries;
    }

    if(retries == BT_MAXRETRIES)
	ajFatal("Maximum retries (%u) reached in btreePricacheDestage "
                "cache %S error: '%s'",
		BT_MAXRETRIES, cache->filename, strerror(ferror(cache->fp)));

    cpage->dirty = BT_CLEAN;
    
    cache->priwrites++;

    return;
}




/* @funcstatic btreeSeccacheDestage *******************************************
**
** Destage a secondary cache page
**
** @param [u] cache [AjPBtcache] cache
** @param [u] cpage [AjPBtpage] cache page
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void btreeSeccacheDestage(AjPBtcache cache, AjPBtpage cpage)
{
    ajuint written = 0;
    ajuint retries = 0;

    /* ajDebug("In btreeSeccacheDestage\n");*/

    if(ajBtreePageIsPrimary(cpage))
        ajWarn("btreeSeccacheDestage primary page %Lu '%s'",
               cpage->pagepos, btreeNodetype(cpage->buf));

    if(fseek(cache->fp, cpage->pagepos,SEEK_SET))
    {
        ajWarn("fseek %Lu failed for cache '%S' %d: '%s'",
               cpage->pagepos, cache->filename,
               ferror(cache->fp), strerror(ferror(cache->fp)));
	fseek(cache->fp,0L,SEEK_END);
    }

    while(written != cache->secpagesize && retries != BT_MAXRETRIES)
    {
	written += fwrite((void *)(cpage->buf+written),1,
                          cache->secpagesize-written,
			  cache->fp);
	++retries;
    }

    if(retries == BT_MAXRETRIES)
	ajFatal("Maximum retries (%u) reached in btreeSeccacheDestage "
                "cache %S error: '%s'",
		BT_MAXRETRIES, cache->filename, strerror(ferror(cache->fp)));

    cpage->dirty = BT_CLEAN;
    
    cache->secwrites++;

    return;
}




/* @funcstatic btreeCacheFetchSize ********************************************
**
** Fetch a cache page from disc
**
** @param [u] cache [AjPBtcache] cache
** @param [w] cpage [AjPBtpage] cache page 
** @param [r] pagepos [ajulong] page number
** @param [r] pagesize [ajuint] page size
**
** @return [void]
**
** @release 2.8.0
** @@
******************************************************************************/

static void btreeCacheFetchSize(AjPBtcache cache, AjPBtpage cpage,
                                ajulong pagepos, ajuint pagesize)
{
    ajuint sum = 0;
    ajuint retries = 0;

    /* ajDebug("In btreeCacheFetchSize\n"); */

    if(fseek(cache->fp,pagepos,SEEK_SET))
	ajFatal("Seek error %d: '%s' in ajBtreeCacheFetchSize file %S",
                ferror(cache->fp),
                strerror(ferror(cache->fp)),
                cache->filename);

    while(sum != pagesize && retries != BT_MAXRETRIES)
    {
	sum += fread((void *)(cpage->buf+sum),1,pagesize-sum,
		     cache->fp);

	++retries;
    }
    
    if(retries == BT_MAXRETRIES)
	ajFatal("Maximum retries (%u) reached in btreeCacheFetchSize "
                "for page %Lu cache '%S'",
		BT_MAXRETRIES,pagepos, cache->filename);

    cpage->pagepos = pagepos;
    
    return;
}




/* @funcstatic btreePricacheFetch *********************************************
**
** Fetch a cache primary page from disc
**
** @param [u] cache [AjPBtcache] cache
** @param [w] cpage [AjPBtpage] cache page 
** @param [r] pagepos [ajulong] page number
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void btreePricacheFetch(AjPBtcache cache, AjPBtpage cpage,
                               ajulong pagepos)
{
    ajuint sum = 0;
    ajuint retries = 0;

    /* ajDebug("In btreePricacheFetch\n"); */

    if(fseek(cache->fp,pagepos,SEEK_SET))
	ajFatal("Seek error %d: '%s' in ajBtreePricacheFetch file %S",
                ferror(cache->fp),
                strerror(ferror(cache->fp)),
                cache->filename);

    while(sum != cache->pripagesize && retries != BT_MAXRETRIES)
    {
	sum += fread((void *)(cpage->buf+sum),1,cache->pripagesize-sum,
		     cache->fp);

	++retries;
    }
    
    if(retries == BT_MAXRETRIES)
	ajFatal("Maximum retries (%u) reached in btreePricacheFetch "
                "for page %Lu cache '%S'",
		BT_MAXRETRIES,pagepos, cache->filename);

    cpage->pagepos = pagepos;
    cache->prireads++;
    
    return;
}




/* @funcstatic btreeSeccacheFetch *********************************************
**
** Fetch a cache secondary page from disc
**
** @param [u] cache [AjPBtcache] cache
** @param [w] cpage [AjPBtpage] cache page 
** @param [r] pagepos [ajulong] page number
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void btreeSeccacheFetch(AjPBtcache cache, AjPBtpage cpage,
                               ajulong pagepos)
{
    ajuint sum = 0;
    ajuint retries = 0;

    /* ajDebug("In btreeSeccacheFetch\n"); */

    if(fseek(cache->fp,pagepos,SEEK_SET))
	ajFatal("Seek error %d: '%s' in ajBtreeSeccacheFetch file %S",
                ferror(cache->fp),
                strerror(ferror(cache->fp)),
                cache->filename);

    while(sum != cache->secpagesize && retries != BT_MAXRETRIES)
    {
	sum += fread((void *)(cpage->buf+sum),1,cache->secpagesize-sum,
		     cache->fp);

	++retries;
    }
    
    if(retries == BT_MAXRETRIES)
	ajFatal("Maximum retries (%u) reached in btreeSeccacheFetch "
                "for page %Lu cache '%S'",
		BT_MAXRETRIES,pagepos, cache->filename);

    cpage->pagepos = pagepos;
    cache->secreads++;
    
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
**
** @release 6.4.0
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
	written += fwrite((void *)(cpage->buf+written),1,pagesize-written,
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
** @param [r] pagesize [ajuint] Page size
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void btreeCacheWriteUncompress(AjPBtcache cache, AjPBtpage cpage,
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
	written += fwrite((void *)(cpage->buf+written),1,pagesize-written,
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
**
** @release 6.4.0
** @@
******************************************************************************/

static ajulong btreeCacheCompress(AjPBtcache thys)
{
    AjPTable  newpostable;
    ajulong  *oldpagepos = NULL;
    ajulong  *newpagepos = NULL;
    ajuint    oldpagesize;
    ajuint    newpagesize;
    AjPBtpage page = NULL;
    ajuint    freepages = 0;
    ajulong   oldpos = 0L;
    ajulong   newpos = 0L;
    ajulong   newsize = 0L;

    ajuint   *newpagesizes = NULL;
    ajulong   i;
    ajuint    maxpagesize = 0;
    ajuint    minpagesize = 0;

    ajulong   pagecount = thys->pripagecount + thys->secpagecount;

    if(thys->secpagesize > thys->pripagesize)
    {
        page = btreeSecpageNew(thys);
        maxpagesize = thys->secpagesize;
        minpagesize = thys->pripagesize;
    }
    else
    {
        page = btreePripageNew(thys);
        maxpagesize = thys->pripagesize;
        minpagesize = thys->secpagesize;
    }

    AJCNEW(oldpagepos, pagecount);
    AJCNEW(newpagepos, pagecount);
    AJCNEW(newpagesizes, pagecount);

    newpostable = ajTableulongNewConst((ajint) pagecount);

    oldpos = 0L;
    newpos = 0L;
    newsize = 0L;

    for(i=0; i < pagecount; i++)
    {
        btreeCacheFetchSize(thys, page, oldpos, minpagesize);

        if(ajBtreePageIsPrimary(page))
            oldpagesize = thys->pripagesize;
        else
            oldpagesize = thys->secpagesize;

        btreeCacheFetchSize(thys, page, oldpos, oldpagesize);

        newpagesize = ajBtreePageGetSize(page, thys->refcount);

        oldpagepos[i] = oldpos;
        newpagepos[i] = newpos;
        newpagesizes[i] = newpagesize;

        ajTablePut(newpostable, (void*) &oldpagepos[i],
                   (void*) &newpagepos[i]);

        oldpos += oldpagesize;
        newpos += newpagesize;
    }

    newsize = newpos;
    newsize += maxpagesize - newpagesizes[pagecount-1];
    newpagesizes[pagecount-1] = maxpagesize;

    oldpos = 0L;

    for(i=0; i < pagecount; i++)
    {
        btreeCacheFetchSize(thys, page, oldpos, minpagesize);
        if(ajBtreePageIsPrimary(page))
            oldpagesize = thys->pripagesize;
        else
            oldpagesize = thys->secpagesize;

        btreeCacheFetchSize(thys, page, oldpos, oldpagesize);

        oldpos += oldpagesize;

        if(!btreePageCompress(page, newpostable, thys->refcount))
        {
            freepages++;
            continue;
        }

        btreeCacheWriteCompress(thys, page, newpagepos[i], newpagesizes[i]);
    }

    thys->filesize = newsize;

    AJFREE(oldpagepos);
    AJFREE(newpagepos);
    AJFREE(newpagesizes);

    ajTableFree(&newpostable);

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
**
** @release 6.4.0
** @@
*******************************************************************************/

static ajulong btreeCacheUncompress(AjPBtcache thys)
{
    AjPTable  newpostable;
    ajulong  *oldpagepos = NULL;
    ajulong  *newpagepos = NULL;
    ajuint    oldpagesize;
    ajuint    newpagesize;
    AjPBtpage page = NULL;
    ajuint    freepages = 0;
    ajulong   newsize = 0L;
    ajulong   oldpos = 0L;
    ajulong   newpos = 0L;

    ajulong   i;
    ajuint    icnt;
    ajulong   writepos;
    ajulong   oldsize;
    ajuint    pagesize;
    ajuint    writepagesize;
    AjBool    isprimary;

    ajulong   pagecount = thys->pripagecount + thys->secpagecount;

    ajDebug("btreeCacheUncompress %S size: %Lu pages: %Lu + %Lu\n",
            thys->filename, thys->totsize,
            thys->pripagecount, thys->secpagecount);

    if(thys->secpagesize > thys->pripagesize)
    {
        page = btreeSecpageNew(thys);
        pagesize = thys->secpagesize;
    }
    else
    {
        page = btreePripageNew(thys);
        pagesize = thys->pripagesize;
    }

    oldsize = thys->totsize;

    AJCNEW0(oldpagepos, pagecount);
    AJCNEW0(newpagepos, pagecount);
    newpostable = ajTableulongNewConst((ajint) pagecount);

    oldpos = 0L;
    newpos = 0L;

    for(i=0; i < pagecount; i++)
    {
        btreeCacheFetchSize(thys, page, oldpos, pagesize);
        oldpagesize = ajBtreePageGetSize(page, thys->refcount);
        if(ajBtreePageIsPrimary(page))
            newpagesize = thys->pripagesize;
        else
            newpagesize = thys->secpagesize;

        oldpagepos[i] = oldpos;
        newpagepos[i] = newpos;

        ajTablePut(newpostable, (void*) &oldpagepos[i],
                   (void*) &newpagepos[i]);

        oldpos += oldpagesize;
        newpos += newpagesize;
    }

    thys->totsize = (ajulong) (thys->pripagesize * thys->pripagecount) +
        (ajulong) (thys->secpagesize * thys->secpagecount);

    writepos = thys->totsize;

    oldpos = 0L;
    newpos = 0L;

    for(i=pagecount; i > 0; i--)
    {
        btreeCacheFetchSize(thys, page, oldpagepos[i-1], pagesize);

        if(!btreePageUncompress(page, newpostable,
                                thys->refcount))
        {
            freepages++;
            continue;
        }

        oldpagesize = ajBtreePageGetSize(page, thys->refcount);

        isprimary = ajBtreePageIsPrimary(page);
        if(isprimary)
            writepagesize = thys->pripagesize;
        else
            writepagesize = thys->secpagesize;

        icnt=writepagesize - oldpagesize;
        if(icnt)
            memset(page->buf+oldpagesize, 0, icnt);
        
        if(writepos < writepagesize)
            ajErr("in btreeCacheWriteUncompress writepos %Lu writepagesize %u",
                  writepos, writepagesize);

        writepos -= writepagesize;

        btreeCacheWriteUncompress(thys, page,
                                  writepos, writepagesize);

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
**
** @release 3.0.0
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
        btreeCacheSync(thys, 0L);

    if(thys->bmem)
        btreeFreePriArray(thys);
    if(thys->bsmem)
        btreeFreeSecArray(thys);

    /* tidy up clean (read not written or locked) cached pages */

    for(page=thys->plru;page;page=temp)
    {
	temp = page->next;
        ajTableRemove(thys->pripagetable, (void*) &page->pagepos);
	AJFREE(page->buf);
	AJFREE(page);
    }

    for(page=thys->slru;page;page=temp)
    {
	temp = page->next;
        ajTableRemove(thys->secpagetable, (void*) &page->pagepos);
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

    ajTableFree(&thys->pripagetable);
    ajTableFree(&thys->secpagetable);

    AJFREE(*Pthis);
    *Pthis = NULL;
    
    return ret;
}




/* @func ajBtreeCacheIsCompressed *********************************************
**
** Returns true if the index is compressed
**
** @param [r] cache [const AjPBtcache] cache
**
** @return [AjBool] True if index is compressed
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ajBtreeCacheIsCompressed(const AjPBtcache cache)
{
    return cache->compressed;
}




/* @func ajBtreeCacheIsSecondary **********************************************
**
** Test whether a cache is a secondary index
**
** @param [r] thys [const AjPBtcache] Cache object
**
** @return [AjBool] True if cache is secondary
**
** @release 6.4.0
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
** @param [u] Ppricache [ajulong*] Number of cache reads to date
** @param [u] Pseccache [ajulong*] Number of cache reads to date
** @param [u] Pprireads [ajulong*] Number of disk reads to date
** @param [u] Psecreads [ajulong*] Number of disk reads to date
** @param [u] Ppriwrites [ajulong*] Number of disk writes to date
** @param [u] Psecwrites [ajulong*] Number of disk writes to date
** @param [u] Pprisize [ajulong*] Number of primary cache pages to date
** @param [u] Psecsize [ajulong*] Number of secondary cache pages to date
**
** @return [void]
**
**
** @release 6.4.0
******************************************************************************/

void ajBtreeCacheStatsOut(AjPFile outf, const AjPBtcache cache,
                          ajulong* Ppricache, ajulong* Pseccache,
                          ajulong* Pprireads, ajulong* Psecreads,
                          ajulong* Ppriwrites, ajulong* Psecwrites,
                          ajulong* Pprisize, ajulong* Psecsize)
{
    ajulong lpriread;
    ajulong lsecread;
    ajulong lpricache;
    ajulong lseccache;
    ajulong lpriwrite;
    ajulong lsecwrite;
    ajulong lprinew;
    ajulong lsecnew;
    ajulong lpripages;
    ajulong lsecpages;

    ajulong lprirewrite = 0;
    ajulong lsecrewrite = 0;

    if(!cache) return;

    lpripages = cache->pripagecount;
    lsecpages = cache->secpagecount;
    lpricache = cache->pricachehits - *Ppricache;
    lseccache = cache->seccachehits - *Pseccache;
    lpriread = cache->prireads - *Pprireads;
    lsecread = cache->secreads - *Psecreads;
    lpriwrite = cache->priwrites - *Ppriwrites;
    lsecwrite = cache->secwrites - *Psecwrites;

    if(lpripages > *Pprisize)
        lprinew = lpripages - *Pprisize;
    else
        lprinew = 0L;

    if(lsecpages > *Psecsize)
        lsecnew = lsecpages - *Psecsize;
    else
        lsecnew = 0L;

    /*
    ** writes counts those committed to disk
    ** newpages counts growth in index size
    ** pending writes may give a negative count for rewrites
    */
    if(lpriwrite >= lprinew)
        lprirewrite = (ajlong)lpriwrite - (ajlong)lprinew;
    if(lsecwrite >= lsecnew)
        lsecrewrite = (ajlong)lsecwrite - (ajlong)lsecnew;

    ajFmtPrintF(outf,
                "ajBtreeCacheStatsOut '%S' cached: %8Lu/%-8Lu "
                "reads: %8Lu/%-8Lu writes: %8Lu/%-8Lu "
                "rewrite: %8Lu/%-8Lu pages: %8Lu/%-8Lu\n",
                cache->filename,  lpricache,  lseccache,
                lpriread, lsecread, lpriwrite, lsecwrite,
                lprirewrite, lsecrewrite, lprinew, lsecnew);

    *Ppricache = cache->pricachehits;
    *Pseccache = cache->seccachehits;
    *Pprireads = cache->prireads;
    *Psecreads = cache->secreads;
    *Ppriwrites = cache->priwrites;
    *Psecwrites = cache->secwrites;
    *Pprisize = lpripages;
    *Psecsize = lsecpages;

    return;
}




/* @funcstatic btreePricacheControl *******************************************
**
** Master control function for primary cache read/write
**
** @param [w] cache [AjPBtcache] name of file
** @param [r] pagepos [ajulong] page number
** @param [r] isread [AjBool] is this a read operation?
**
** @return [AjPBtpage] disc cache page pointer
**
** @release 6.5.0
** @@
******************************************************************************/

static AjPBtpage btreePricacheControl(AjPBtcache cache, ajulong pagepos,
                                      AjBool isread)
{
    AjPBtpage ret      = NULL;

    /* ajDebug("In btreePricacheControl\n"); */

    if(pagepos < cache->totsize)
        ret = btreePricacheLocate(cache,pagepos);

    if(ret)
    {
	btreePricacheUnlink(cache,ret);
    }
    else
    {
	if(cache->prilistLength == cache->pricachesize)
	{
	    ret = btreePricacheLruUnlink(cache);

	    if(ret->dirty == BT_DIRTY)
		btreePricacheDestage(cache,ret);

	    if(isread || pagepos!=cache->totsize)
		btreePricacheFetch(cache,ret,pagepos);
            else
                btreePripageClear(cache, ret);
	}
	else
	{
	    ret = btreePripageNew(cache);

	    if(isread || pagepos!=cache->totsize)
		btreePricacheFetch(cache,ret,pagepos);
	}

	if(!isread)
	    ret->pagepos = pagepos;
	else
	    ret->dirty = BT_CLEAN;
    }

    btreePricacheMruAdd(cache,ret);

    if(!ajBtreePageIsPrimary(ret))
        ajWarn("btreePricacheControl secondary page %Lu '%s'",
               ret->pagepos, btreeNodetype(ret->buf));

    return ret;
}




/* @funcstatic btreeSeccacheControl *******************************************
**
** Master control function for secondary cache read/write
**
** @param [w] cache [AjPBtcache] name of file
** @param [r] pagepos [ajulong] page number
** @param [r] isread [AjBool] is this a read operation?
**
** @return [AjPBtpage] disc cache page pointer
**
** @release 6.5.0
** @@
******************************************************************************/

static AjPBtpage btreeSeccacheControl(AjPBtcache cache, ajulong pagepos,
                                      AjBool isread)
{
    AjPBtpage ret      = NULL;

    /* ajDebug("In btreeSeccacheControl\n"); */

    if(pagepos < cache->totsize)
        ret = btreeSeccacheLocate(cache,pagepos);

    if(ret)
    {
	btreeSeccacheUnlink(cache,ret);
    }
    else
    {
	if(cache->seclistLength == cache->seccachesize)
	{
	    ret = btreeSeccacheLruUnlink(cache);

	    if(ret->dirty == BT_DIRTY)
		btreeSeccacheDestage(cache,ret);

	    if(isread || pagepos!=cache->totsize)
		btreeSeccacheFetch(cache,ret,pagepos);
            else
                btreeSecpageClear(cache, ret);
	}
	else
	{
	    ret = btreeSecpageNew(cache);

	    if(isread || pagepos!=cache->totsize)
		btreeSeccacheFetch(cache,ret,pagepos);
	}

	if(!isread)
	    ret->pagepos = pagepos;
	else
	    ret->dirty = BT_CLEAN;
    }

    btreeSeccacheMruAdd(cache,ret);

    if(ajBtreePageIsPrimary(ret))
        ajWarn("btreeSeccacheControl primary page %Lu '%s'",
               ret->pagepos, btreeNodetype(ret->buf));

    return ret;
}




/* @func ajBtreeCacheRead **************************************************
**
** Get a pointer to a disc cache page
**
** @param [w] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
**
** @return [AjPBtpage] disc cache page pointer
**
** @release 6.5.0
** @@
******************************************************************************/

AjPBtpage ajBtreeCacheRead(AjPBtcache cache, ajulong pagepos)
{
    AjPBtpage ret = NULL;

    if(!btreeTestpage)
    {
        if(cache->secpagesize > cache->pripagesize)
            btreeTestpage = btreeSecpageNew(cache);
        else        
            btreeTestpage = btreePripageNew(cache);
    }
    
    /* ajDebug("In ajBtreeCacheRead\n"); */

    btreeNocacheFetch(cache, btreeTestpage, pagepos);

    if(ajBtreePageIsPrimary(btreeTestpage))
        ret = btreePricacheControl(cache,pagepos,BT_READ);
    else
        ret = btreeSeccacheControl(cache,pagepos,BT_READ);

    return ret;
}




/* @funcstatic btreePricacheRead **********************************************
**
** Get a pointer to a disc primary cache page
**
** @param [w] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
**
** @return [AjPBtpage] disc cache page pointer
**
** @release 6.5.0
** @@
******************************************************************************/

static AjPBtpage btreePricacheRead(AjPBtcache cache, ajulong pagepos)
{
    AjPBtpage ret = NULL;

    /* ajDebug("In btreePricacheRead\n"); */

    ret = btreePricacheControl(cache,pagepos,BT_READ);

    return ret;
}




/* @funcstatic btreeSeccacheRead **********************************************
**
** Get a pointer to a disc secondary cache page
**
** @param [w] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
**
** @return [AjPBtpage] disc cache page pointer
**
** @release 6.5.0
** @@
******************************************************************************/

static AjPBtpage btreeSeccacheRead(AjPBtcache cache, ajulong pagepos)
{
    AjPBtpage ret = NULL;

    /* ajDebug("In btreeSeccacheRead\n"); */

    ret = btreeSeccacheControl(cache,pagepos,BT_READ);

    return ret;
}




/* @funcstatic btreeCacheSync *************************************************
**
** Sync all dirty cache pages
**
** @param [u] cache [AjPBtcache] cache
** @param [r] rootpage [ajulong] root page
**
** @return [void]
**
** @release 3.0.0
** @@
******************************************************************************/

static void btreeCacheSync(AjPBtcache cache, ajulong rootpage)
{
    AjPBtpage page = NULL;
    ajuint numlocked = 0;
    ajuint numwrite = 0;

    /* ajDebug("In btreeCacheSync\n");*/

    if(!cache->pmru && !cache->smru) /* cache unused*/
        return;

    statCallSync++;

    if(cache->pmru)
    {
        for(page=cache->plru;page;page=page->next)
        {
            if(page->dirty)         /* BT_LOCK or BT_DIRTY */
            {
                numwrite++;
                if(page->dirty == BT_LOCK)
                    numlocked++;
                btreePricacheDestage(cache,page);
            }
        }
    }

    if(cache->smru)
    {
        for(page=cache->slru;page;page=page->next)
        {
            if(page->dirty)         /* BT_LOCK or BT_DIRTY */
            {
                numwrite++;
                if(page->dirty == BT_LOCK)
                    numlocked++;
                btreeSeccacheDestage(cache,page);
            }
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
    
                
    page = btreePricacheLocate(cache,rootpage);
    page->dirty = BT_LOCK;
    page->lockfor = 1001;

    if(rootpage)
    {
        page = btreePricacheLocate(cache,0L);
        page->dirty = BT_LOCK;
        page->lockfor = 1002;
    }

    statSyncLocked += numlocked;
    statSyncWrite += numwrite;
    return;
}




/* @funcstatic btreeCacheRootSync *********************************************
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
**
** @release 6.4.0
** @@
******************************************************************************/

static void btreeCacheRootSync(AjPBtcache cache, ajulong rootpage)
{
    AjPBtpage page = NULL;
    ajuint numlocked = 0;
    ajuint numunlocked = 0;

    /*ajDebug("In btreeCacheRootSync '%S' rootpage:%Lu\n",
      cache->filename, rootpage);*/

    statCallRootSync++;

    for(page=cache->plru;page;page=page->next)
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
                    ajWarn("btreeCacheRootSync locked '%Lu' lockfor: %u",
                            page->pagepos, page->lockfor);
#if AJINDEX_DEBUG                    
                    ajDebug("btreeCacheRootSync locked '%Lu' lockfor: %u\n",
                            page->pagepos, page->lockfor);
#endif
                }
            }
        }
    }
    
    for(page=cache->slru;page;page=page->next)
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
                    ajWarn("btreeCacheRootSync locked '%Lu' lockfor: %u",
                            page->pagepos, page->lockfor);
#if AJINDEX_DEBUG                    
                    ajDebug("btreeCacheRootSync locked '%Lu' lockfor: %u\n",
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

    page = btreeSeccacheLocate(cache,rootpage);
    page->dirty = BT_LOCK;      /* (re)lock the secondary root page */
    page->lockfor = 1011;

    return;
}




/* @funcstatic btreePricacheWrite *********************************************
**
** Get a pointer to a disc primary cache page for writing
**
** @param [w] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
**
** @return [AjPBtpage] disc cache page pointer
**
** @release 6.5.0
** @@
******************************************************************************/

static AjPBtpage btreePricacheWrite(AjPBtcache cache, ajulong pagepos)
{
    AjPBtpage ret = NULL;

#if AJINDEX_DEBUG
    ajDebug("btreePricacheWrite %Lu\n", pagepos);
#endif

    ret = btreePricacheControl(cache,pagepos,BT_WRITE);

    return ret;
}




/* @funcstatic btreeSeccacheWrite *********************************************
**
** Get a pointer to a disc secondary cache page for writing
**
** @param [w] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
**
** @return [AjPBtpage] disc cache page pointer
**
** @release 6.5.0
** @@
******************************************************************************/

static AjPBtpage btreeSeccacheWrite(AjPBtcache cache, ajulong pagepos)
{
    AjPBtpage ret = NULL;

#if AJINDEX_DEBUG
    ajDebug("btreeSeccacheWrite %Lu\n", pagepos);
#endif

    ret = btreeSeccacheControl(cache,pagepos,BT_WRITE);

    return ret;
}




#if 0
/* #funcstatic btreeCacheWriteIdbucket ****************************************
**
** Get a pointer to a disc cache page for writing
**
** #param [w] cache [AjPBtcache] cache
** #param [r] pagepos [ajulong] page number
**
** #return [AjPBtpage] disc cache page pointer
**
** #release 6.4.0
** ##
******************************************************************************/
/*
static AjPBtpage btreeCacheWriteIdbucket(AjPBtcache cache, ajulong pagepos)
{
    AjPBtpage ret = NULL;

    /# ajDebug("In btreeCacheWriteIdbucket\n");#/

    ret = btreePricacheControl(cache,pagepos,BT_WRITE);

    return ret;
}
*/
#endif




/* @funcstatic btreePricacheNodenew *******************************************
**
** Get a pointer to a new disc primary cache page for writing a bucket.
**
** Clears the page and sets the block number.
**
** @param [w] cache [AjPBtcache] cache
**
** @return [AjPBtpage] disc cache page pointer
**
** @release 6.5.0
** @@
******************************************************************************/

static AjPBtpage btreePricacheNodenew(AjPBtcache cache)
{
    AjPBtpage ret = NULL;
    unsigned char *p;
    ajuint nodetype;

#if AJINDEX_DEBUG
    ajDebug("btreePricacheNodeenew pagepos:%Lu\n", cache->totsize);
#endif

    ret = btreePricacheControl(cache,cache->totsize,BT_WRITE);
    p = ret->buf;
    AJCSET0(p, cache->pripagesize); /* clear buffer to zeros */

    /* keep pointers to other cached pages */
/*
    ret->next = NULL;
    ret->prev = NULL;
*/

    ret->dirty = BT_CLEAN;

    ret->pagepos = cache->totsize;

    nodetype     = BT_FREEPAGE;
    SBT_NODETYPE(p,nodetype);

    SBT_BLOCKNUMBER(p,ret->pagepos);

    cache->totsize += cache->pripagesize;
    cache->pripagecount++;

    return ret;
}




/* @funcstatic btreeSeccacheNodenew *******************************************
**
** Get a pointer to a new disc secondary cache page for writing
**
** Clears the page and sets the block number.
**
** @param [w] cache [AjPBtcache] cache
**
** @return [AjPBtpage] disc cache page pointer
**
** @release 6.5.0
** @@
******************************************************************************/

static AjPBtpage btreeSeccacheNodenew(AjPBtcache cache)
{
    AjPBtpage ret = NULL;
    unsigned char *p;
    ajuint nodetype;

#if AJINDEX_DEBUG
    ajDebug("btreeSeccacheNodenew pagepos:%Lu\n", cache->totsize);
#endif

    ret = btreeSeccacheControl(cache,cache->totsize,BT_WRITE);
    p = ret->buf;
    AJCSET0(p, cache->secpagesize); /* clear buffer to zeros */

    /* keep pointers to other cached pages */
/*
    ret->next = NULL;
    ret->prev = NULL;
*/

    ret->dirty = BT_CLEAN;

    ret->pagepos = cache->totsize;

    nodetype     = BT_SECFREEPAGE;
    SBT_NODETYPE(p,nodetype);

    SBT_BLOCKNUMBER(p,ret->pagepos);

    cache->totsize += cache->secpagesize;
    cache->secpagecount++;

    return ret;
}




/* @funcstatic btreePricacheBucketnew *****************************************
**
** Get a pointer to a new disc primary cache page for writing a bucket.
**
** Clears the page.
**
** @param [w] cache [AjPBtcache] cache
**
** @return [AjPBtpage] disc cache page pointer
**
** @release 6.5.0
** @@
******************************************************************************/

static AjPBtpage btreePricacheBucketnew(AjPBtcache cache)
{
    AjPBtpage ret = NULL;
    unsigned char *p;
    ajuint nodetype;

#if AJINDEX_DEBUG
    ajDebug("btreePricacheBucketnew pagepos:%Lu\n", cache->totsize);
#endif

    ret = btreePricacheControl(cache,cache->totsize,BT_WRITE);
    p = ret->buf;
    AJCSET0(p, cache->pripagesize); /* clear buffer to zeros */

    /* keep pointers to other cached pages */
/*
    ret->next = NULL;
    ret->prev = NULL;
*/

    ret->dirty = BT_CLEAN;

    ret->pagepos = cache->totsize;

    nodetype     = BT_FREEPAGE;
    SBT_NODETYPE(p,nodetype);

    cache->totsize += cache->pripagesize;
    cache->pripagecount++;

    return ret;
}




/* @funcstatic btreeSeccacheBucketnew ******************************************
**
** Get a pointer to a new disc secondary cache page for writing a bucket
**
** Clears the page.
**
** @param [w] cache [AjPBtcache] cache
**
** @return [AjPBtpage] disc cache page pointer
**
** @release 6.5.0
** @@
******************************************************************************/

static AjPBtpage btreeSeccacheBucketnew(AjPBtcache cache)
{
    AjPBtpage ret = NULL;
    unsigned char *p;
    ajuint nodetype;

#if AJINDEX_DEBUG
    ajDebug("btreeSeccacheBucketnew pagepos:%Lu\n", cache->totsize);
#endif

    ret = btreeSeccacheControl(cache,cache->totsize,BT_WRITE);
    p = ret->buf;
    AJCSET0(p, cache->secpagesize); /* clear buffer to zeros */

    /* keep pointers to other cached pages */
/*
    ret->next = NULL;
    ret->prev = NULL;
*/

    ret->dirty = BT_CLEAN;

    ret->pagepos = cache->totsize;

    nodetype     = BT_SECFREEPAGE;
    SBT_NODETYPE(p,nodetype);

    cache->totsize += cache->secpagesize;
    cache->secpagecount++;

    return ret;
}



#if 0
/* #funcstatic btreePricacheWriteOverflownew **********************************
**
** Get a pointer to a new disc cache page for writing an overflow node
**
** Clears the page and sets the block number and nodetype
**
** #param [w] cache [AjPBtcache] cache
**
** #return [AjPBtpage] disc cache page pointer
**
** #release 6.5.0
** ##
******************************************************************************/

/*
//static AjPBtpage btreePricacheWriteOverflownew(AjPBtcache cache)
//{
//    AjPBtpage ret = NULL;
//    unsigned char *p;
//    ajuint v;
//
//#if AJINDEX_DEBUG
//    ajDebug("btreePricacheWriteOverflownew pagepos:%Lu\n", cache->totsize);
//#endif
//
//    ret = btreePricacheControl(cache,cache->totsize,BT_WRITE);
//    p = ret->buf;
//    AJCSET0(p, cache->pripagesize); /# clear buffer to zeros #/
//
//    ret->next = NULL;
//    ret->prev = NULL;
//
//    ret->dirty = BT_CLEAN;
//
//    ret->pagepos = cache->totsize;
//
//    SBT_BLOCKNUMBER(p,ret->pagepos);
//
//    v = BT_OVERFLOW;
//    SBT_NODETYPE(p,v);
//
//    cache->totsize += cache->pripagesize;
//    cache->pripagecount++;
//
//    return ret;
//}
*/
#endif




/* @funcstatic btreePrirootCreate *********************************************
**
** Create and write an empty primary root node. Set it as root, write it to
** disc and then lock the page in the disc cache.
**
** The root node is at block 0L
**
** @param [w] cache [AjPBtcache] cache
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void btreePrirootCreate(AjPBtcache cache)
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
    ajulong rootpage = 0L;

#if AJINDEX_DEBUG
    ajDebug("btreePrirootCreate\n"); 
#endif


    if(!cache->totsize)
        page = btreePricacheNodenew(cache); /* new index */
    else
        page = btreePricacheWrite(cache, rootpage); /* existing index? */

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
        btreeCacheRootSync(cache,rootpage);
    page->dirty = BT_LOCK;
    page->lockfor = 1021;

    return;
}




/* @funcstatic btreeSecrootCreate *********************************************
**
** Create and write an empty secondary root node. Set it as root, write it to
** disc and then lock the page in the disc cache.
**
** The root node is at block 0L, secondary roots are on other pages
**
** @param [w] cache [AjPBtcache] cache
** @param [r] rootpage [ajulong] root block
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void btreeSecrootCreate(AjPBtcache cache, ajulong rootpage)
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
    ajDebug("btreeSecrootCreate %Lu\n", rootpage); 
#endif


    if(!rootpage)
    {
        if(rootpage == cache->totsize)
            page = btreeSeccacheNodenew(cache);
        else
            page = btreeSeccacheWrite(cache, rootpage);
    }
    else 
    {
        if(rootpage == cache->totsize)
            page = btreeSeccacheNodenew(cache);
        else
            page = btreeSeccacheWrite(cache, rootpage);
    }
           
    page->pagepos    = rootpage;
    
    p = page->buf;

    nodetype    = BT_SECROOT;
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
        btreeCacheRootSync(cache,rootpage);
    page->dirty = BT_LOCK;
    page->lockfor = 1021;

    return;
}




/* @funcstatic btreePrimaryFindInode ******************************************
**
** Recursive search for insert node
**
** @param [u] cache [AjPBtcache] cache
** @param [u] page [AjPBtpage] page
** @param [r] item [const AjPStr] key to search for 
**
** @return [AjPBtpage] leaf node where item should be inserted
**
** @release 6.5.0
** @@
******************************************************************************/

static AjPBtpage btreePrimaryFindInode(AjPBtcache cache, AjPBtpage page,
                                       const AjPStr item)
{
    AjPBtpage ret = NULL;
    AjPBtpage pg  = NULL;

    unsigned char *buf = NULL;
    ajuint status       = 0;
    ajuint ival         = 0;

    /* ajDebug("In btreePrimaryFindInode\n"); */
    
    ret = page;
    buf = page->buf;
    GBT_NODETYPE(buf,&ival);

    if(ival != BT_LEAF)
    {
	status = ret->dirty;
	ret->dirty = BT_LOCK;	/* Lock in case of lots of overflow pages */
        ret->lockfor = 1031;
	pg = btreePrimaryPageDown(cache,buf,item);
	ret->dirty = status;
	ret = btreePrimaryFindInode(cache,pg,item);
    }

    return ret;
}




/* @funcstatic btreeKeyidFindINode ********************************************
**
** Recursive search for insert node in a secondary tree
**
** @param [u] cache [AjPBtcache] cache
** @param [u] page [AjPBtpage] page
** @param [r] item [const AjPStr] key to search for 
**
** @return [AjPBtpage] leaf node where item should be inserted
**
** @release 4.0.0
** @@
******************************************************************************/

static AjPBtpage btreeKeyidFindINode(AjPBtcache cache, AjPBtpage page,
                                   const AjPStr item)
{
    AjPBtpage ret = NULL;
    AjPBtpage pg  = NULL;

    unsigned char *buf = NULL;
    ajuint status       = 0;
    ajuint ival         = 0;

    /* ajDebug("In btreeKeyidFindINode\n"); */
    
    ret = page;
    buf = page->buf;
    GBT_NODETYPE(buf,&ival);

    if(ival != BT_SECLEAF)
    {
	status = ret->dirty;
	ret->dirty = BT_LOCK;	/* Lock in case of lots of overflow pages */
        ret->lockfor = 1041;
	pg = btreeSecondaryPageDown(cache,buf,item);
	ret->dirty = status;
	ret = btreeKeyidFindINode(cache,pg,item);
    }
    
    return ret;
}




/* @funcstatic btreePrimaryPageDown *******************************************
**
** Return next lower index page given a key
**
** @param [u] cache [AjPBtcache] cache
** @param [u] buf [unsigned char *] page buffer
** @param [r] key [const AjPStr] key to search for 
**
** @return [AjPBtpage] pointer to a page
**
** @release 6.5.0
** @@
******************************************************************************/

static AjPBtpage btreePrimaryPageDown(AjPBtcache cache, unsigned char *buf,
                                         const AjPStr key)
{
    ajulong blockno = 0L;
    AjPBtpage page = NULL;
    
    /* ajDebug("In btreePrimaryPageDown\n"); */

    blockno = btreeGetBlockS(cache,buf,key);
    page = btreePricacheRead(cache,blockno);

    return page;
}




/* @funcstatic btreeSecondaryPageDown *****************************************
**
** Return next lower index page given a key in a secondary tree
**
** @param [u] cache [AjPBtcache] cache
** @param [u] buf [unsigned char *] page buffer
** @param [r] key [const AjPStr] key to search for 
**
** @return [AjPBtpage] pointer to a page
**
** @release 6.5.0
** @@
******************************************************************************/

static AjPBtpage btreeSecondaryPageDown(AjPBtcache cache, unsigned char *buf,
                                        const AjPStr key)
{
    ajulong blockno = 0L;
    AjPBtpage page = NULL;
    
    /* ajDebug("In btreeSecondaryPageDown\n"); */
    
    blockno = btreeGetBlockS(cache, buf, key);

    page =  btreeSeccacheRead(cache,blockno);

    return page;
}




/* @func ajBtreeIdNew *********************************************************
**
** Constructor for index bucket ID information
**
** @param [r] refcount [ajuint] Number of reference files for each entry
** @return [AjPBtId] Index ID object
**
** @release 3.0.0
** @@
******************************************************************************/

AjPBtId ajBtreeIdNew(ajuint refcount)
{
    AjPBtId Id = NULL;

    /* ajDebug("In ajBtreeIdNew\n"); */

    if(statSaveBtreeIdNext)
    {
        Id = statSaveBtreeId[--statSaveBtreeIdNext];
        MAJSTRASSIGNCLEAR(&Id->id);
        if(Id->refcount != refcount)
        {
            if(refcount)
                AJCRESIZE0(Id->refoffsets, Id->refcount, refcount);
            else
                AJFREE(Id->refoffsets);

            Id->refcount = refcount;
        }
    }
    else
    {
        AJNEW0(Id);
        Id->id = ajStrNew();
        Id->refcount = refcount;

        if(refcount)
            AJCNEW(Id->refoffsets, refcount);
    }

    Id->dbno = 0;
    Id->dups = 0;
    Id->offset = 0L;

    return Id;
}




/* @func ajBtreeIdDel *********************************************************
**
** Destructor for index bucket ID information
**
** @param [w] thys [AjPBtId*] index ID object
**
** @return [void]
**
** @release 3.0.0
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
**
** @release 6.4.0
** @@
******************************************************************************/

void ajBtreeIdDelVoid(void **voidarg)
{
    AjPBtId *thys = (AjPBtId *)voidarg;

    ajBtreeIdDel(thys);

    return;
}




/* @func ajBtreeIdIsMulti *****************************************************
**
** Tests whether an ID object links to multiple entries
**
** @param [r] thys [const AjPBtId] index ID object
**
** @return [AjBool] True if ID object points to duplicated identifiers
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ajBtreeIdIsMulti(const AjPBtId thys)
{
    if(!thys)
	return ajFalse;

    if(thys->dups)
        return ajTrue;

    return ajFalse;
}




/* @funcstatic btreeIdFree ****************************************************
**
** Destructor for index bucket ID information
**
** @param [w] thys [AjPBtId*] index ID object
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void btreeIdFree(AjPBtId *thys)
{
    AjPBtId Id = NULL;

    /* ajDebug("In btreeIdFree\n"); */

    if(!thys || !*thys)
	return;

    Id = *thys;

    ajStrDel(&Id->id);

    if(Id->refoffsets)
        AJFREE(Id->refoffsets);

    AJFREE(Id);
    *thys = NULL;

    return;
}




/* @func ajBtreeHitNew ********************************************************
**
** Constructor for index hit information
**
** @return [AjPBtHit] Index hit object
**
** @release 6.5.0
** @@
******************************************************************************/

AjPBtHit ajBtreeHitNew(void)
{
    AjPBtHit Hit = NULL;

    /* ajDebug("In ajBtreeHitNew\n"); */

    if(statSaveBtreeHitNext)
    {
        Hit = statSaveBtreeHit[--statSaveBtreeHitNext];
        Hit->dbno = 0;
        Hit->refcount = 0;
        Hit->offset = 0L;
    }
    else
    {
        AJNEW0(Hit);
    }

    return Hit;
}




/* @func ajBtreeHitNewId ******************************************************
**
** Constructor for index hit information from an existing Btid object
**
** @param [r] btid [const AjPBtId] Index id object
** @return [AjPBtHit] Index hit object
**
** @release 6.5.0
** @@
******************************************************************************/

AjPBtHit ajBtreeHitNewId(const AjPBtId btid)
{
    AjPBtHit Hit = NULL;

    /* ajDebug("In ajBtreeHitNewId\n"); */

    if(statSaveBtreeHitNext)
    {
        Hit = statSaveBtreeHit[--statSaveBtreeHitNext];
    }
    else
    {
        AJNEW0(Hit);
    }

    Hit->dbno = btid->dbno;
    Hit->refcount = 0;
    Hit->offset = btid->offset;

    return Hit;
}




/* @func ajBtreeHitDel ********************************************************
**
** Destructor for index hit information
**
** @param [w] thys [AjPBtHit*] index hit object
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ajBtreeHitDel(AjPBtHit *thys)
{
    ajuint newmax;

    /* ajDebug("In ajBtreeHitDel\n"); */

    if(!statSaveBtreeHit)
    {
        statSaveBtreeHitMax = 2048;
        AJCNEW0(statSaveBtreeHit, statSaveBtreeHitMax);
        statSaveBtreeHitNext = 0;
    }

    if(!thys || !*thys)
	return;

    if(statSaveBtreeHitNext >= statSaveBtreeHitMax) 
    {
        newmax = statSaveBtreeHitMax + statSaveBtreeHitMax;
        AJCRESIZE0(statSaveBtreeHit,statSaveBtreeHitMax,newmax);
        statSaveBtreeHitMax = newmax;
    }
    
    statSaveBtreeHit[statSaveBtreeHitNext++] = *thys;
    
    *thys = NULL;

    return;
}




/* @func ajBtreeHitDelVoid ****************************************************
**
** Destructor for index hit information
**
** @param [w] voidarg [void**] Index hit object, passed as void for use by
**                             list and table destructors.
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ajBtreeHitDelVoid(void **voidarg)
{
    AjPBtHit *thys = (AjPBtHit *)voidarg;

    ajBtreeHitDel(thys);

    return;
}




/* @funcstatic btreeHitFree ***************************************************
**
** Destructor for index hit information
**
** @param [w] thys [AjPBtHit*] index hit object
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void btreeHitFree(AjPBtHit *thys)
{
    AjPBtHit Hit = NULL;

    /* ajDebug("In btreeHitFree\n"); */

    if(!thys || !*thys)
	return;

    Hit = *thys;

    AJFREE(Hit);
    *thys = NULL;

    return;
}




/* @func ajBtreeHitrefNew *****************************************************
**
** Constructor for index reference hit information
**
** @return [AjPBtHitref] Index reference hit object
**
** @release 6.5.0
** @@
******************************************************************************/

AjPBtHitref ajBtreeHitrefNew(void)
{
    AjPBtHitref Hitref = NULL;

    /* ajDebug("In ajBtreeHitrefNew\n"); */

    if(statSaveBtreeHitrefNext)
    {
        Hitref = statSaveBtreeHitref[--statSaveBtreeHitrefNext];
        Hitref->dbno = 0;
        Hitref->refcount = 0;
        Hitref->offset = 0L;
    }
    else
    {
        AJNEW0(Hitref);
    }

    return Hitref;
}




/* @func ajBtreeHitrefNewId ****************************************************
**
** Constructor for index reference hit information from an existing
** Btid object
**
** @param [r] btid [const AjPBtId] Index id object
** @return [AjPBtHitref] Index hit object
**
** @release 6.5.0
** @@
******************************************************************************/

AjPBtHitref ajBtreeHitrefNewId(const AjPBtId btid)
{
    AjPBtHitref Hitref = NULL;

    /* ajDebug("In ajBtreeHitrefNewId\n"); */

    if(statSaveBtreeHitrefNext)
    {
        Hitref = statSaveBtreeHitref[--statSaveBtreeHitrefNext];
    }
    else
    {
        AJNEW0(Hitref);
    }

    Hitref->dbno = btid->dbno;
    Hitref->refcount = 0;
    Hitref->offset = btid->offset;
    if(btid->refcount)
        Hitref->refoffset = btid->refoffsets[0];
    else
        Hitref->refoffset = 0L;

    return Hitref;
}




/* @func ajBtreeHitrefDel *****************************************************
**
** Destructor for index reference hit information
**
** @param [w] thys [AjPBtHitref*] index reference hit object
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ajBtreeHitrefDel(AjPBtHitref *thys)
{
    ajuint newmax;

    /* ajDebug("In ajBtreeHitrefDel\n"); */

    if(!statSaveBtreeHitref)
    {
        statSaveBtreeHitrefMax = 2048;
        AJCNEW0(statSaveBtreeHitref, statSaveBtreeHitrefMax);
        statSaveBtreeHitrefNext = 0;
    }

    if(!thys || !*thys)
	return;

    if(statSaveBtreeHitrefNext >= statSaveBtreeHitrefMax) 
    {
        newmax = statSaveBtreeHitrefMax + statSaveBtreeHitrefMax;
        AJCRESIZE0(statSaveBtreeHitref,statSaveBtreeHitrefMax,newmax);
        statSaveBtreeHitrefMax = newmax;
    }
    
    statSaveBtreeHitref[statSaveBtreeHitrefNext++] = *thys;
    
    *thys = NULL;

    return;
}




/* @func ajBtreeHitrefDelVoid *************************************************
**
** Destructor for index reference hit information
**
** @param [w] voidarg [void**] Index reference hit object, passed as
**                             void for use by list and table
**                             destructors.  @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ajBtreeHitrefDelVoid(void **voidarg)
{
    AjPBtHitref *thys = (AjPBtHitref *)voidarg;

    ajBtreeHitrefDel(thys);

    return;
}




/* @funcstatic btreeHitrefFree ************************************************
**
** Destructor for index reference hit information
**
** @param [w] thys [AjPBtHitref*] index reference hit object
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void btreeHitrefFree(AjPBtHitref *thys)
{
    AjPBtHitref Hitref = NULL;

    /* ajDebug("In btreeHitrefFree\n"); */

    if(!thys || !*thys)
	return;

    Hitref = *thys;

    AJFREE(Hitref);
    *thys = NULL;

    return;
}




/* @funcstatic btreeIdbucketIdlist ********************************************
**
** Copies all IDs into a list
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
** @param [u] idlist [AjPList] list to hold IDs
**
** @return [ajulong] Overflow
**
** @release 6.4.0
** @@
******************************************************************************/

static ajulong btreeIdbucketIdlist(AjPBtcache cache, ajulong pagepos,
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
    ajuint  iref;
    ajuint  len = 0;
    ajuint idlen;
    ajuint keyskip = BT_DDOFF + cache->refcount*BT_EXTRA + 1;

    /* ajDebug("In btreeIdbucketIdlist\n"); */
    
    if(!pagepos)
	ajFatal("IdbucketIdlist: cannot read bucket from root page cache %S",
                cache->filename);

    page  = btreePricacheRead(cache,pagepos);
    lpage = page;
    dirtysave = lpage->dirty;
    lpage->dirty = BT_LOCK;     /* reset on return */
    lpage->lockfor = 1051;

    buf = lpage->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);
    if(nodetype != BT_IDBUCKET)
	ajFatal("BucketIdlist: NodeType mismatch. "
                "Not bucket (%u) cache %S",
                nodetype, cache->filename);
    
    GBT_BUCKNENTRIES(buf,&nentries);
    if(nentries > cache->pnperbucket)
	ajFatal("BucketIdlist: Bucket too full page: %Lu "
                "entries: %u max: %u cache %S",
                pagepos, nentries, cache->pnperbucket, cache->filename);
    

    GBT_BUCKOVERFLOW(buf,&overflow);
    pageoverflow = overflow;
    
    kptr  = PBT_BUCKKEYLEN(buf);
    idptr = kptr + (nentries * sizeof(ajuint));

    for(i=0;i<nentries;++i)
    {
	BT_GETAJUINT(kptr,&len);
        idlen = len - keyskip;

/*
//	if((idptr-buf+1) + len > cache->pagesize)	/# overflow #/
//	{
//	    /# ajDebug("IdbucketIdlist: Overflow\n"); #/
//	    page  = btreePricacheRead(cache,overflow);
//	    buf = page->buf;
//	    GBT_BUCKNODETYPE(buf,&nodetype);
//	    if(nodetype != BT_IDBUCKET)
//		ajFatal("BucketIdlist: NodeType mismatch. "
//                        "Not bucket (%u) cache %S",
//			nodetype, cache->filename);
//	    GBT_BUCKOVERFLOW(buf,&overflow);
//	    /# overflow bucket ids start at the keylen position #/
//	    idptr = PBT_BUCKKEYLEN(buf);
//	}
*/

	id = ajBtreeIdNew(cache->refcount);
        
	/* Fill ID objects */
	ajStrAssignLenC(&id->id,(const char *)idptr,idlen);
	idptr += (idlen + 1);
	BT_GETAJUINT(idptr,&id->dbno);
	idptr += sizeof(ajuint);
	BT_GETAJUINT(idptr,&id->dups);
	idptr += sizeof(ajuint);	
	BT_GETAJULONG(idptr,&id->offset);
	idptr += sizeof(ajulong);
        if(cache->refcount)
        {
            for(iref=0; iref< cache->refcount; iref++)
            {
                BT_GETAJULONG(idptr,&id->refoffsets[iref]);
                idptr += sizeof(ajulong);
            }
        }

	kptr += sizeof(ajuint);
        ajListPushAppend(idlist, id);
    }

    lpage->dirty = dirtysave;
    
    return pageoverflow;
}




/* @funcstatic btreeReadIdbucket **********************************************
**
** Constructor for id bucket given a disc page number
** Creates one empty key slot for possible addition
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
**
** @return [AjPIdbucket] Id bucket
**
** @release 2.8.0
** @@
******************************************************************************/

static AjPIdbucket btreeReadIdbucket(AjPBtcache cache, ajulong pagepos)
{
    AjPIdbucket bucket    = NULL;
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
    ajuint  iref;
    ajuint  len = 0;
    ajuint idlen;
    ajuint keyskip = BT_DDOFF + cache->refcount*BT_EXTRA + 1;

    /* ajDebug("In btreeReadIdbucket\n"); */
    
    if(!pagepos)
	ajFatal("BucketRead: cannot read bucket from root page");

    page  = btreePricacheRead(cache,pagepos);
    lpage = page;
    dirtysave = lpage->dirty;
    lpage->dirty = BT_LOCK;
    lpage->lockfor = 1061;

    buf = lpage->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);

    if(nodetype != BT_IDBUCKET)
	ajFatal("ReadBucket: NodeType mismatch. Not bucket (%u)", nodetype);
    
    GBT_BUCKNENTRIES(buf,&nentries);

    if(nentries > cache->pnperbucket)
	ajFatal("ReadBucket: Bucket too full page: %Lu "
                "entries: %u max: %u cache %S",
                pagepos, nentries, cache->pnperbucket, cache->filename);
    

    GBT_BUCKOVERFLOW(buf,&overflow);

    bucket = btreeIdbucketNew(cache->pnperbucket, cache->refcount);
    bucket->Nentries = nentries;
    
    kptr  = PBT_BUCKKEYLEN(buf);
    idptr = kptr + (nentries * sizeof(ajuint));

    for(i=0;i<nentries;++i)
    {
	BT_GETAJUINT(kptr,&len);
        idlen = len - keyskip;

/*
//	if((idptr-buf+1) + len > cache->pagesize)	/# overflow #/
//	{
//#if AJINDEX_DEBUG
//	    ajDebug("ReadIdbucket: Overflow\n");
//#endif
//	    page  = btreePricacheRead(cache,overflow);
//	    buf = page->buf;
//	    GBT_BUCKNODETYPE(buf,&nodetype);
//	    if(nodetype != BT_IDBUCKET)
//		ajFatal("ReadBucket: NodeType mismatch. Not bucket (%u)",
//			nodetype);
//	    GBT_BUCKOVERFLOW(buf,&overflow);
//	    /# overflow bucket ids start at the keylen position #/
//	    idptr = PBT_BUCKKEYLEN(buf);
//	}
*/

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
        if(cache->refcount)
        {
            for(iref=0; iref < cache->refcount; iref++)
            {
                BT_GETAJULONG(idptr,&id->refoffsets[iref]);
                idptr += sizeof(ajulong);
            }
        }

	kptr += sizeof(ajuint);
    }

    lpage->dirty = dirtysave;
    
    return bucket;
}




/* @funcstatic btreeIdbucketFindDupId *****************************************
**
** Tests for an ID in an id bucket.
** If found, returns the ID and its count in the bucket.
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
** @param [r] id [const AjPStr] id to search for
** @param [w] ientry [ajuint*] entry number matched
**
** @return [AjPBtId] ID found
**
** @release 6.4.0
** @@
******************************************************************************/

static AjPBtId btreeIdbucketFindDupId(AjPBtcache cache, ajulong pagepos,
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
    ajuint  iref;
    ajuint  len = 0;
    ajuint  idlen;
    ajuint keyskip = BT_DDOFF + cache->refcount*BT_EXTRA + 1;

    /* ajDebug("In btreeIdbucketFindDupId\n"); */
    
    if(!pagepos)
	ajFatal("btreeIdbucketFindDupId: "
                "cannot read bucket from root page cache %S",
                cache->filename);

    page  = btreePricacheRead(cache,pagepos);
    lpage = page;
    dirtysave = lpage->dirty;
    lpage->dirty = BT_LOCK;     /* reset on return */
    lpage->lockfor = 1071;

    buf = lpage->buf;
    lbuf = buf;

    GBT_BUCKNODETYPE(lbuf,&nodetype);
    if(nodetype != BT_IDBUCKET)
	ajFatal("BucketIdFindDupId: NodeType mismatch. "
                "Not bucket (%u) cache %S",
                nodetype, cache->filename);
    
    GBT_BUCKNENTRIES(lbuf,&nentries);
    if(nentries > cache->pnperbucket)
	ajFatal("BucketIdFindDupId: Bucket too full page: %Lu "
                "entries: %u max: %u cache %S",
                pagepos, nentries, cache->pnperbucket, cache->filename);
    

    GBT_BUCKOVERFLOW(lbuf,&overflow);
    
    kptr  = PBT_BUCKKEYLEN(buf);
    idptr = kptr + (nentries * sizeof(ajuint));

    for(i=0;i<nentries;++i)
    {
	BT_GETAJUINT(kptr,&len);

/*
//	if((idptr-buf+1) + len > cache->pagesize)	/# overflow #/
//	{
//	    /# ajDebug("BucketFindId: Overflow\n"); #/
//	    page  = btreePricacheRead(cache,overflow);
//	    buf = page->buf;
//	    GBT_BUCKNODETYPE(buf,&nodetype);
//	    if(nodetype != BT_IDBUCKET)
//		ajFatal("BucketFindId: NodeType mismatch. "
//                        "Not bucket (%u) cache %S",
//			nodetype, cache->filename);
//	    GBT_BUCKOVERFLOW(buf,&overflow);
//	    /# overflow bucket ids start at the keylen position #/
//	    idptr = PBT_BUCKKEYLEN(buf);
//	}
*/

	if(!MAJSTRCMPC(id,(const char*)idptr))
        {
            bid = ajBtreeIdNew(cache->refcount);
            idlen = len - keyskip;
            ajStrAssignLenC(&bid->id,(const char *)idptr, idlen);
            idptr += (idlen + 1);
            BT_GETAJUINT(idptr,&bid->dbno);
            idptr += sizeof(ajuint);
            BT_GETAJUINT(idptr,&bid->dups);
            idptr += sizeof(ajuint);	
            BT_GETAJULONG(idptr,&bid->offset);
            idptr += sizeof(ajulong);

            if(cache->refcount)
            {
                for(iref=0; iref < cache->refcount; iref++)
                {
                    BT_GETAJULONG(idptr,&bid->refoffsets[iref]);
                    idptr += sizeof(ajulong);
                }
            }

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




/* @funcstatic btreeIdbucketSort **********************************************
**
** Sorts IDs in an id bucket
**
** @param [u] thys [AjPIdbucket] Id bucket
**
** @return [void]
** @@
******************************************************************************/

static void btreeIdbucketSort(AjPIdbucket thys)
{
    qsort(thys->Ids, thys->Nentries, sizeof(AjPBtId), btreeIdCompare);

    return;
}




/* @funcstatic btreeWriteIdbucketId *******************************************
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
**
** @release 6.4.0
** @@
******************************************************************************/

static void btreeWriteIdbucketId(AjPBtcache cache, ajulong pagepos,
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
    ajuint iref;
    ajuint keyskip = BT_DDOFF + cache->refcount*BT_EXTRA + 1;

    /* ajDebug("In btreeWriteIdbucketId\n"); */
    
    if(!pagepos)
	ajFatal("WriteIdbucketId: cannot read bucket from root page cache %S",
                cache->filename);

    page  = btreePricacheRead(cache,pagepos);
    lpage = page;
    lpage->dirty = BT_LOCK;     /* reset on return */
    lpage->lockfor = 1081;

    buf = lpage->buf;
    lbuf= buf;

    GBT_BUCKNODETYPE(lbuf,&nodetype);
    if(nodetype != BT_IDBUCKET)
	ajFatal("WriteIdbucketId: NodeType mismatch. "
                "Not bucket (%u) cache %S",
                nodetype, cache->filename);
    
    GBT_BUCKNENTRIES(lbuf,&nentries);
    if(nentries > cache->pnperbucket)
	ajFatal("WriteIdbucketId: Bucket too full page: %Lu "
                "entries: %u max: %u cache %S",
                pagepos, nentries, cache->pnperbucket, cache->filename);
    

    GBT_BUCKOVERFLOW(lbuf,&overflow);
    
    kptr  = PBT_BUCKKEYLEN(lbuf);
    idptr = kptr + (nentries * sizeof(ajuint));

    for(i=0;i<ientry;++i)
    {
	BT_GETAJUINT(kptr,&len);

/*
//	if((idptr-buf+1) + len > cache->pagesize)	/# overflow #/
//	{
//	    /# ajDebug("BucketFindId: Overflow\n"); #/
//	    page  = btreePricacheRead(cache,overflow);
//	    buf = page->buf;
//	    GBT_BUCKNODETYPE(buf,&nodetype);
//	    if(nodetype != BT_IDBUCKET)
//		ajFatal("WriteIdbucketId: NodeType mismatch. "
//                        "Not bucket (%u) cache %S",
//			nodetype, cache->filename);
//	    GBT_BUCKOVERFLOW(buf,&overflow);
//	    /# overflow bucket ids start at the keylen position #/
//	    idptr = PBT_BUCKKEYLEN(buf);
//	}
*/

/* Step through ID object data */
	idptr += len;
        kptr += sizeof(ajuint);
    }

    BT_GETAJUINT(kptr,&len);
    idlen = len - keyskip;
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

    if(cache->refcount)
    {
        for(iref=0; iref < cache->refcount; iref++)
        {
            lv = btid->refoffsets[iref];
            BT_SETAJULONG(idptr,lv);
            idptr += sizeof(ajulong);
        }

    }

    lpage->dirty = BT_DIRTY;

    return;
}




/* @funcstatic btreeWriteIdbucket *********************************************
**
** Write index bucket object to the cache given a disc page number
**
** @param [u] cache [AjPBtcache] cache
** @param [r] bucket [const AjPIdbucket] Id bucket
** @param [r] pagepos [ajulong] page number
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void btreeWriteIdbucket(AjPBtcache cache, const AjPIdbucket bucket,
                               ajulong pagepos)
{
    AjPBtpage page      = NULL;
    AjPBtpage lpage     = NULL;
    unsigned char *buf  = NULL;
    unsigned char *lbuf = NULL;
    ajuint  uv  = 0;
    ajuint   v   = 0;
    ajuint  i    = 0;
    ajuint  iref = 0;
    ajuint   len = 0;
    ajulong lv   = 0L;
    AjPBtId id  = NULL;

    ajuint nentries = 0;
    ajulong overflow = 0L;
    unsigned char *keyptr = NULL;
    unsigned char *lptr   = NULL;
    ajuint refskip = cache->refcount*BT_EXTRA;

/*    ajuint keyskip = BT_DDOFF + cache->refcount*BT_EXTRA + 1;*/

    if(pagepos == cache->totsize)	/* Create a new page */
    {
	page = btreePricacheBucketnew(cache);
	buf = page->buf;
    }
    else
    {
	page = btreePricacheRead(cache,pagepos);
	buf = page->buf;
	GBT_BUCKOVERFLOW(buf,&overflow);
    }

    v = BT_IDBUCKET;
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
        /* drop check - tested before call - pagesize dependency */
/*
//	if((ajuint)((keyptr-lbuf+1)+sizeof(ajuint)) > cache->pagesize)
//	    ajFatal("WriteIdbucket: Bucket cannot hold more than %u keys",
//		    i-1);
*/

	id = bucket->Ids[i];
	/* Need to alter this if bucket ID structure changes */
	len = BT_BUCKIDLEN(id->id) + refskip;
        v = len;
	BT_SETAJUINT(keyptr,v);
	keyptr += sizeof(ajuint);
    }


    /* Write out IDs using overflow if necessary */
    lptr = keyptr;

    for(i=0;i<nentries;++i)
    {
	id = bucket->Ids[i];
	len = BT_BUCKIDLEN(id->id) + refskip;

/*
//        /# overflow #/        
//	if((lptr-buf+1) + (len+keyskip)) > (ajuint) cache->pagesize)
//	{
//	    if(!overflow)		/# No overflow buckets yet #/
//	    {
//                page = btreePricacheBucketnew(cache);
//		buf = page->buf;
//		v = BT_IDBUCKET;
//		SBT_BUCKNODETYPE(buf,v);
//	    }
//	    else
//	    {
//		page = btreePricacheRead(cache,overflow);
//		buf  = page->buf;
//		GBT_BUCKOVERFLOW(buf,&overflow);
//	    }
//
//	    page->dirty = BT_DIRTY;
//
//	    lptr = PBT_BUCKKEYLEN(buf);	    
//	}
*/	

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

        if(cache->refcount)
        {
            for(iref=0; iref < cache->refcount; iref++)
            {
                lv = id->refoffsets[iref];
                BT_SETAJULONG(lptr,lv);
                lptr += sizeof(ajulong);
            }
        }
    }

    lv = 0L;
    SBT_BUCKOVERFLOW(buf,lv);

    lpage->dirty = BT_DIRTY;

    return;
}




/* @funcstatic btreeWriteIdbucketEmpty ****************************************
**
** Write empty index id bucket object to the cache given a disc page number
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void btreeWriteIdbucketEmpty(AjPBtcache cache, ajulong pagepos)
{
    AjPBtpage page      = NULL;
    unsigned char *buf  = NULL;
    ajuint   v   = 0;
    ajulong lv   = 0L;

    ajuint  nentries = 0;
    ajulong overflow = 0L;

    /* ajDebug("In btreeWriteIdbucketEmpty\n"); */

    if(pagepos == cache->totsize)	/* Create a new page */
    {
	page = btreePricacheBucketnew(cache);
	buf = page->buf;
	overflow = 0L;
	lv = overflow;
	SBT_BUCKOVERFLOW(buf,lv);
    }
    else
    {
	page = btreePricacheRead(cache,pagepos);
	buf = page->buf;
	GBT_BUCKOVERFLOW(buf,&overflow);
    }
    
    v = BT_IDBUCKET;
    SBT_BUCKNODETYPE(buf,v);

    nentries = 0;
    v = nentries;
    SBT_BUCKNENTRIES(buf,v);

    lv = 0L;
    SBT_BUCKOVERFLOW(buf,lv);

    page->dirty = BT_DIRTY;

    return;
}




/* @funcstatic btreeIdbucketDel ***********************************************
**
** Delete an id bucket object
**
** @param [w] thys [AjPIdbucket*] bucket
**
** @return [void] bucket
**
** @release 2.8.0
** @@
******************************************************************************/

static void btreeIdbucketDel(AjPIdbucket *thys)
{
    AjPIdbucket pthis = NULL;
    ajuint newmax;

    /* ajDebug("In btreeIdbucketDel\n"); */

    if(!thys || !*thys)
	return;

 
    pthis = *thys;

    if(!statSaveIdbucket)
    {
        statSaveIdbucketMax=2048;
        statSaveIdbucketNext=0;
        AJCNEW0(statSaveIdbucket,statSaveIdbucketMax);
    }
    
    if(!statSaveIdbucketEmpty)
    {
        statSaveIdbucketEmptyMax=2048;
        statSaveIdbucketEmptyNext=0;
        AJCNEW0(statSaveIdbucketEmpty,statSaveIdbucketEmptyMax);
    }
    
    if(pthis->Maxentries)
    {
        if(statSaveIdbucketNext >= statSaveIdbucketMax)
        {
            newmax = statSaveIdbucketMax + statSaveIdbucketMax;
            AJCRESIZE0(statSaveIdbucket,statSaveIdbucketMax,newmax);
            statSaveIdbucketMax = newmax;
        }
        
        statSaveIdbucket[statSaveIdbucketNext++] = pthis;
    }
    else
    {
        if(statSaveIdbucketEmptyNext >= statSaveIdbucketEmptyMax)
        {
            newmax = statSaveIdbucketEmptyMax + statSaveIdbucketEmptyMax;
            AJCRESIZE0(statSaveIdbucketEmpty,statSaveIdbucketEmptyMax,newmax);
            statSaveIdbucketEmptyMax = newmax;
        }
        statSaveIdbucketEmpty[statSaveIdbucketEmptyNext++] = pthis;
    }
    
    *thys = NULL;

    return;
}




/* @funcstatic btreeIdbucketFree **********************************************
**
** Delete a keyword id bucket object
**
** @param [w] thys [AjPIdbucket*] bucket
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void btreeIdbucketFree(AjPIdbucket *thys)
{
    AjPIdbucket pthis = NULL;
    ajuint n;
    ajuint i;
    
    /*ajDebug("In btreeIdbucketFree\n"); */

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




/* @funcstatic btreeIdbucketAdd ***********************************************
**
** Add an ID to an id bucket
** Only called if there is room in the bucket
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number of bucket
** @param [r] id [const AjPBtId] ID info
**
** @return [void]
**
** @release 2.8.0
** @@
******************************************************************************/

static void btreeIdbucketAdd(AjPBtcache cache, ajulong pagepos,
                             const AjPBtId id)
{
    unsigned char *buf  = NULL;
    unsigned char *kptr = NULL;
    unsigned char *src  = NULL;
    unsigned char *dest = NULL;

    unsigned char *endptr  = NULL;
    
    ajuint nentries = 0;
    ajuint nodetype = 0;
    
    ajuint sum = 0;
    ajuint len = 0;
    ajuint i;
    ajuint v;
    ajuint uv;
    ajulong lv;

    ajuint iref;

    AjPBtpage page = NULL;
    static ajuint calls = 0;
    ajuint refskip = cache->refcount*BT_EXTRA;
    
/*  ajuint keyskip = BT_DDOFF + cache->refcount*BT_EXTRA + 1;*/

/*    unsigned char *lastptr = NULL;*/
/*    ajuint idlen    = 0;*/
/*    static ajuint overflowcalls=0;*/
   
    calls++;

    page = btreePricacheRead(cache,pagepos);
    buf  = page->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);
    if(nodetype != BT_IDBUCKET)
        ajFatal("Wrong nodetype in IdbucketAdd cache %S", cache->filename);

    GBT_BUCKNENTRIES(buf,&nentries);
    if(nentries == cache->pnperbucket)
        ajFatal("Bucket too full in IdbucketAdd page: %Lu "
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

/*
//    idlen   = MAJSTRGETLEN(id->id);
//    lastptr = endptr + sizeof(ajuint) + idlen + keyskip;
//
//    if((ajuint) (lastptr - buf) >= cache->pripagesize)
//    {
//        overflowcalls++;
//        ajWarn("\nOverflow in IdbucketAdd nentries:%u fails %u/%u cache %S",
//               nentries, overflowcalls,calls, cache->filename);
//        btreeIdbucketAddFull(cache,pagepos,id);
//        return;
//    }
*/    

    dest = src + sizeof(ajuint);
    memmove((void *)dest, (void *)src, sum);

    v = BT_BUCKIDLEN(id->id) + refskip;
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

    if(cache->refcount)
    {
        for(iref=0; iref < cache->refcount; iref++)
        {
            lv = id->refoffsets[iref];
            BT_SETAJULONG(endptr,lv);
            endptr += sizeof(ajulong);
        }
    }

    v = nentries + 1;
    SBT_BUCKNENTRIES(buf,v);

    page->dirty = BT_DIRTY;

    return;
}




#if 0
/* #funcstatic btreeIdbucketAddFull *******************************************
**
** Add an ID to an id bucket
** Only called if there is no room in the bucket
**
** #param [u] cache [AjPBtcache] cache
** #param [r] pagepos [ajulong] page number of bucket
** #param [r] id [const AjPBtId] ID info
**
** #return [void]
**
** #release 6.5.0
** ##
******************************************************************************/
/*
//static void btreeIdbucketAddFull(AjPBtcache cache, ajulong pagepos,
//                                 const AjPBtId id)
//{
//    AjPIdbucket bucket = NULL;
//    AjPBtId   destid = NULL;
//    
//    ajuint nentries;
//    ajuint iref;
//    
//    /# ajDebug("In btreeIdbucketAddFull\n"); #/
//
//    bucket   = btreeReadIdbucket(cache,pagepos);
//    nentries = bucket->Nentries;
//
//
//    /# Reading a bucket always gives one extra ID position #/
//    destid = bucket->Ids[nentries];
//
//    ajStrAssignS(&destid->id,id->id);
//    destid->dbno      = id->dbno;
//    destid->dups      = id->dups;
//    destid->offset    = id->offset;
//
//    if(cache->refcount)
//    {
//        for(iref=0; iref < cache->refcount; iref++)
//            destid->refoffsets[iref] = id->refoffsets[iref];
//    }
//
//    ++bucket->Nentries;
//
//    btreeWriteIdbucket(cache,bucket,pagepos);
//
//    btreeIdbucketDel(&bucket);
//    
//    return;
//}
*/
#endif




/* @funcstatic btreeIdbucketCount *********************************************
**
** Return number of entries in an id bucket
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
**
** @return [ajuint] Number of entries in bucket
**
** @release 2.8.0
** @@
******************************************************************************/

static ajuint btreeIdbucketCount(AjPBtcache cache, ajulong pagepos)
{
    AjPBtpage page     = NULL;
    unsigned char *buf = NULL;
    ajuint nodetype    = 0;
    ajuint nentries    = 0;
    
    /* ajDebug("In btreeIdbucketCount\n"); */
    
    if(!pagepos)
	ajFatal("btreeIdbucketCount: Attempt to read bucket from root page\n");

    page  = btreePricacheRead(cache,pagepos);

    buf = page->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);

    if(nodetype != BT_IDBUCKET)
	ajFatal("btreeIdbucketCount: NodeType mismatch. Not id bucket (%u)",
                nodetype);
    
    GBT_BUCKNENTRIES(buf,&nentries);

    return nentries;
}




/* @funcstatic btreeIdbucketsReorder ******************************************
**
** Re-order leaf id buckets
** Must only be called if one of the buckets is full
**
** @param [u] cache [AjPBtcache] cache
** @param [u] leaf [AjPBtpage] leaf page
**
** @return [AjBool] true if reorder was successful i.e. leaf not full
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool btreeIdbucketsReorder(AjPBtcache cache, AjPBtpage leaf)
{
    ajuint nkeys = 0;
    unsigned char *lbuf = NULL;

    ajulong *ptrs        = NULL;
    ajulong *overflows   = NULL;
    AjPStr *newkeys     = NULL;
    ajulong *newptrs     = NULL;
    AjPBtMem arrays = NULL;
    AjPBtMem newarrays = NULL;
    
    ajuint i = 0;
    ajuint iref = 0;

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
    AjPIdbucket cbucket = NULL;
    AjPBtId cid       = NULL;

    ajuint iold = 0;
    ajuint refskip = cache->refcount*BT_EXTRA;
    
#if AJINDEX_DEBUG
    ajDebug("btreeIdbucketsReorder %Lu\n", leaf->pagepos);
#endif

    /* ajDebug("In btreeIdbucketsReorder\n"); */

    dirtysave = leaf->dirty;

    leaf->dirty = BT_LOCK;
    leaf->lockfor = 1101;
    lbuf = leaf->buf;

    GBT_NODETYPE(lbuf,&nodetype);

    order = cache->porder;    

    /* Read keys/ptrs */
    arrays = btreeAllocPriArray(cache);
    ptrs = arrays->parray;

    btreeGetPointers(cache,lbuf,&ptrs);

    GBT_NKEYS(lbuf,&nkeys);


    if(!nkeys)
	ajFatal("IdbucketsReorder: Attempt to reorder empty leaf");

    keylimit = nkeys + 1;
    for(i=0;i<=nkeys;++i)
	totalkeys += btreeIdbucketCount(cache,ptrs[i]);

    btreeBucketCalc(totalkeys, keylimit, cache->pnperbucket,
                    &bucketlimit, &maxnperbucket);

    if(bucketlimit >= order)
    {
        btreeDeallocPriArray(cache, arrays);
	
	leaf->dirty = dirtysave;
	return ajFalse;
    }
    
    ++statCallIdbucketsReorder;

    newarrays = btreeAllocPriArray(cache);
    newkeys = newarrays->karray;
    newptrs = newarrays->parray;
    overflows = newarrays->overflows;
    
    /* Read IDs from all buckets and push to list and sort (increasing id) */
    idlist  = ajListNew();
    
    for(i=0;i<keylimit;++i)
	overflows[i] = btreeIdbucketIdlist(cache,ptrs[i],idlist);

    ajListSort(idlist, &btreeIdCompare);


    cbucket = btreeIdbucketNew(maxnperbucket, cache->refcount);
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

            if(cache->refcount)
            {
                for(iref=0; iref < cache->refcount; iref++)
                    cid->refoffsets[iref] = bid->refoffsets[iref];
	    }

	    cbucket->keylen[count] = BT_BUCKIDLEN(bid->id) + refskip;
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
	btreeWriteIdbucket(cache,cbucket,newptrs[i]);
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

        if(cache->refcount)
        {
            for(iref=0; iref < cache->refcount; iref++)
                cid->refoffsets[iref] = bid->refoffsets[iref];
	}

	++cbucket->Nentries;
	++count;
	ajBtreeIdDel(&bid);
    }
    
    if((iold < order) && ptrs[iold])
        newptrs[i] = ptrs[iold++];
    else
        newptrs[i] = cache->totsize;
     btreeWriteIdbucket(cache,cbucket,newptrs[i]);

    btreeIdbucketDel(&cbucket);

#if AJINDEX_DEBUG
    if(iold < keylimit)
        ajDebug("btreeIdbucketsReorder '%S' %u -> %u",
                cache->filename, keylimit, iold);
#endif

    for(i = iold+1; i <= keylimit; i++)
    {
        btreePripageSetfree(cache, ptrs[i]);
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

    btreeDeallocPriArray(cache, arrays);
    btreeDeallocPriArray(cache, newarrays);

    btreeIdbucketDel(&cbucket);
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
**
** @release 2.8.0
** @@
******************************************************************************/

static AjBool btreeNodeIsFull(const AjPBtcache cache, AjPBtpage page)
{
    unsigned char *buf = NULL;
    ajuint nkeys = 0;

    /* ajDebug("In btreeNodeIsFull\n"); */

    buf = page->buf;
    GBT_NKEYS(buf,&nkeys);

    if(nkeys == cache->porder - 1)
	return ajTrue;

    return ajFalse;
}





/* @func ajBtreeDumpKeywords **************************************************
**
** Read the leaves of a complete keywords tree, returning all identifiers
** found within a range of total occurrences
**
** @param [u] cache [AjPBtcache] cache
** @param [r] dmin [ajuint] minimum number of times the key should appear
** @param [r] dmax [ajuint] maximum number of times the key should appear
** @param [u] outf [AjPFile] output file
**
** @return [void]
**
** @release 6.5.0
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
    ajulong kdups;
    
    AjPPribucket bucket;
    ajuint nkeys;
    ajulong right;
    AjPBtMem array = NULL;
    AjPList tlist  = NULL;
    ajuint dirtysave = 0;
    ajulong totkeys = 0L;
    ajulong totdups = 0L;

    if(!cache->secondary)
    {
        ajBtreeDumpIdentifiers(cache, dmin, dmax, outf);
        return;
    }

    array = btreeAllocPriArray(cache);
    parray = array->parray;

    page = btreePricacheRead(cache, 0);
    buf = page->buf;
    dirtysave  = page->dirty;
    page->dirty = BT_LOCK;
    page->lockfor = 1111;

    btreeGetPointers(cache,buf,&parray);
    GBT_NODETYPE(buf,&nodetype);

    while(nodetype != BT_LEAF && cache->plevel != 0)
    {
        page->dirty = dirtysave;
	page = btreePricacheRead(cache,parray[0]);
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
	    bucket = btreePribucketRead(cache,parray[i]);
            btreePribucketSort(bucket);

	    for(j=0;j<bucket->Nentries;++j)
	    {
                totkeys++;

		kdups = btreeSecTreeCount(cache, bucket->codes[j]->treeblock);
                totdups += kdups;

		ajListstrFreeData(&tlist);

                if(kdups < dmin)
                    continue;
                if(dmax && kdups > dmax)
                    continue;
                ajFmtPrintF(outf,"%10Ld %S\n",
                            kdups, bucket->codes[j]->keyword);
	    }
	    btreePribucketDel(&bucket);
	}

	right = 0L;
	if(cache->plevel)
	{
	    GBT_RIGHT(buf,&right);
	    if(right)
	    {
                page->dirty = dirtysave;
		page = btreePricacheRead(cache,right);
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




/* @funcstatic btreeNodeIsFullSec *********************************************
**
** Tests whether a secondary node is full of keys
**
** @param [r] cache [const AjPBtcache] cache
** @param [u] page [AjPBtpage] original page
**
** @return [AjBool] true if full
**
** @release 4.0.0
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




/* @funcstatic btreePrimaryInsertNonfull **************************************
**
** Insert a key into a non-full primary node (identifiers or keywords)
**
** @param [u] cache [AjPBtcache] cache
** @param [u] page [AjPBtpage] original page
** @param [r] key [const AjPStr] key to insert
** @param [r] less [ajulong] less-than pointer
** @param [r] greater [ajulong] greater-than pointer
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void btreePrimaryInsertNonfull(AjPBtcache cache, AjPBtpage page,
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
    
    /* ajDebug("In btreeInsertNonfull\n"); */

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
    ppage = btreePricacheRead(cache,less);
    lv = pagepos;
    SBT_PREV(ppage->buf,lv);
    ppage->dirty = BT_DIRTY;

    ppage = btreePricacheRead(cache,greater);
    lv = pagepos;
    SBT_PREV(ppage->buf,lv);
    ppage->dirty = BT_DIRTY;
    
    btreeDeallocPriArray(cache,arrays1);

    if(nodetype != BT_ROOT)
	btreePrimaryShift(cache,page);

    return;
}




/* @funcstatic btreePriInsertKey **********************************************
**
** Insert a key into a potentially full primary node
**
** @param [u] cache [AjPBtcache] cache
** @param [u] page [AjPBtpage] original page
** @param [r] key [const AjPStr] key to insert
** @param [r] less [ajulong] less-than pointer
** @param [r] greater [ajulong] greater-than pointer
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void btreePriInsertKey(AjPBtcache cache, AjPBtpage page,
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
	btreePrimaryInsertNonfull(cache,page,key,less,greater);
	return;
    }
    
    order = cache->porder;
    lbuf = page->buf;
    GBT_NODETYPE(lbuf,&nodetype);
    page->dirty = BT_LOCK;
    page->lockfor = 1131;

    if(nodetype == BT_ROOT)
    {
	btreePriSplitroot(cache);

	if(page->pagepos)
	    page->dirty = BT_DIRTY;
        
        blockno = btreeGetBlockFirstS(cache,lbuf,key);

	ipage = btreePricacheRead(cache,blockno);
	btreePrimaryInsertNonfull(cache,ipage,key,less,greater);

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
    rpage = btreePricacheNodenew(cache);
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
	tpage = btreePricacheRead(cache,tparray[i]);
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
	tpage = btreePricacheRead(cache,tparray[i]);
	tbuf = tpage->buf;
	lv = rblockno;
	SBT_PREV(tbuf,lv);
	tpage->dirty = BT_DIRTY;
    }


    ibn = rblockno;

    if(MAJSTRCMPS(key,mediankey)<0)
	ibn = lblockno;

    ipage = btreePricacheRead(cache,ibn);
    
    btreePrimaryInsertNonfull(cache,ipage,key,less,greater);

    btreeDeallocPriArray(cache, savekeyarrays);
    btreeDeallocPriArray(cache, arrays2);

    ipage = btreePricacheRead(cache,prev);

    btreePriInsertKey(cache,ipage,mediankey,medianless,mediangtr);
    ajStrDel(&mediankey);

    return;
}




/* @funcstatic btreePriSplitroot **********************************************
**
** Split the root node
**
** @param [u] cache [AjPBtcache] cache
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void btreePriSplitroot(AjPBtcache cache)
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
    ajDebug("btreePriSplitroot zero\n");
#endif
    ++statCallPriSplitroot;

    /* ajDebug("In btreePriSplitroot\n"); */

    order = cache->porder;

    arrays1 = btreeAllocPriArray(cache);
    arrays2 = btreeAllocPriArray(cache);
    karray = arrays1->karray;
    parray = arrays1->parray;
    tkarray = arrays2->karray;
    tparray = arrays2->parray;

    key = ajStrNew();

    rootpage = btreePricacheLocate(cache,0L);

    if(!rootpage)
	ajFatal("Root page has been unlocked 1 cache %S", cache->filename);
    
    rootbuf = rootpage->buf;

    nkeys = order - 1;

    keypos = nkeys / 2;

    if(!(nkeys % 2))
	--keypos;

    rblockno = cache->totsize;
    rpage = btreePricacheNodenew(cache);
    rpage->dirty = BT_LOCK;
    rpage->lockfor = 1141;

    lblockno = cache->totsize;
    lpage = btreePricacheNodenew(cache);

    if(!cache->plevel)
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
    
    if(cache->plevel)
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
	tpage = btreePricacheRead(cache,tparray[i]);
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
	tpage = btreePricacheRead(cache,tparray[i]);
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
    ++cache->plevel;

    ajStrDel(&key);
    
    return;
}




/* @funcstatic btreeGetKeys ***************************************************
**
** Get Keys and Pointers from an internal node
**
** @param [u] cache [AjPBtcache] cache
** @param [u] buf [unsigned char *] page buffer
** @param [w] keys [AjPStr **] keys
** @param [w] ptrs [ajulong**] ptrs
**
** @return [void]
**
** @release 2.8.0
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

/*    ajulong   overflow = 0L;*/
/*    ajuint    pagesize = 0;*/
/*    AjPBtpage page = NULL;*/

    karray = *keys;
    parray = *ptrs;
    
    tbuf    = buf;

/*    pagesize = cache->pagesize;*/
    
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

/*
//	if((ajuint)((keyptr-tbuf+1) + len + sizeof(ajulong)) > pagesize)
//	{
//#if AJINDEX_DEBUG
//    	    ajDebug("GetKeys: Overflow\n");
//#endif
//	    GBT_OVERFLOW(tbuf,&overflow);
//	    page = btreePricacheRead(cache,overflow);
//	    tbuf = page->buf;
//	    GBT_NODETYPE(tbuf,&ival);
//
//	    if(ival != BT_OVERFLOW)
//		ajFatal("Overflow node expected but not found for cache %S",
//                        cache->filename);
//	    /#
//	     ** The length pointer is restricted to the initial page.
//	     ** The keyptr in overflow pages starts at the Key Lengths
//	     ** position!
//	     #/
//	    keyptr = PBT_KEYLEN(tbuf);
//	}
*/

	ajStrAssignC(&karray[i],(const char *)keyptr);
	keyptr += len;

	BT_GETAJULONG(keyptr,&parray[i]);
	keyptr += sizeof(ajulong);
	lenptr += sizeof(ajuint);
    }
    
/*
//    if((ajuint)((keyptr-tbuf+1) + sizeof(ajulong)) > pagesize)
//    {
//	GBT_OVERFLOW(tbuf,&overflow);
//	page = btreePricacheRead(cache,overflow);
//	tbuf = page->buf;
//	GBT_NODETYPE(tbuf,&ival);
//
//	if(ival != BT_OVERFLOW)
//	    ajFatal("Overflow node expected but not found at end for cache %S",
//                    cache->filename);
//	/#
//	 ** The length pointer is restricted to the initial page.
//	 ** The keyptr in overflow pages starts at the Key Lengths
//	 ** position!
//	 #/
//	keyptr = PBT_KEYLEN(tbuf);
//    }
*/    

    BT_GETAJULONG(keyptr,&parray[i]);

    return;
}




/* @funcstatic btreeGetPointers ***********************************************
**
** Get Pointers from an internal node
**
** @param [u] cache [AjPBtcache] cache
** @param [u] buf [unsigned char *] page buffer
** @param [w] ptrs [ajulong**] ptrs
**
** @return [ajuint] Number of pointers
**
** @release 6.4.0
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
/*    ajuint    pagesize = 0;*/
/*    ajulong   overflow = 0L;*/
/*    AjPBtpage page = NULL;*/

    parray = *ptrs;
    
    tbuf    = buf;

/*    pagesize = cache->pagesize;*/
    
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
	
/*
//        if((ajuint)((keyptr-tbuf+1) + len + sizeof(ajulong)) > pagesize)
//	{
//	    GBT_OVERFLOW(tbuf,&overflow);
//	    page = ajBtreeCacheRead(cache,overflow); /# pri or sec possible #/
//	    tbuf = page->buf;
//	    GBT_NODETYPE(tbuf,&ival);
//	    if(ival != BT_OVERFLOW)
//		ajFatal("Overflow node expected but not found cache %S",
//                        cache->filename);
//	    /#
//	     ** The length pointer is restricted to the initial page.
//	     ** The keyptr in overflow pages starts at the Key Lengths
//	     ** position!
//	     #/
//	    keyptr = PBT_KEYLEN(tbuf);
//	}
*/

	keyptr += len;

	BT_GETAJULONG(keyptr,&parray[i]);
	keyptr += sizeof(ajulong);
	lenptr += sizeof(ajuint);
    }
    
/*
//    if((ajuint)((keyptr-tbuf+1) + sizeof(ajulong)) > pagesize)
//    {
//	/# ajDebug("GetPointers: Overflow\n"); #/
//	GBT_OVERFLOW(tbuf,&overflow);
//	page = ajBtreeCacheRead(cache,overflow); /# pri or sec possible #/
//	tbuf = page->buf;
//	GBT_NODETYPE(tbuf,&ival);
//	if(ival != BT_OVERFLOW)
//	    ajFatal("Overflow node expected but not found cache %S",
//                    cache->filename);
//	/#
//	 ** The length pointer is restricted to the initial page.
//	 ** The keyptr in overflow pages starts at the Key Lengths
//	 ** position!
//	 #/
//	keyptr = PBT_KEYLEN(tbuf);
//    }
*/    

    BT_GETAJULONG(keyptr,&parray[i]);

    return i;
}





/* @funcstatic btreeGetBlockC *************************************************
**
** Get Block number matching a key
**
** @param [u] cache [AjPBtcache] cache
** @param [u] buf [unsigned char *] page buffer
** @param [r] ckey [const char*] key
**
** @return [ajulong] Block number
**
** @release 6.4.0
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
    
/*    AjPBtpage page = NULL;*/
/*    ajuint    pagesize = 0;*/
/*    ajulong   overflow = 0L;*/

/*    pagesize = cache->pagesize;*/

    tbuf = buf;

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
	
/*
//	if((ajuint)((keyptr-tbuf+1) + len + sizeof(ajulong)) > pagesize)
//	{
//    	    /# ajDebug("btreeGetBlockC: Overflow\n"); #/
//	    GBT_OVERFLOW(tbuf,&overflow);
//	    page = ajBtreeCacheRead(cache,overflow); /# pri or sec possible #/
//	    tbuf = page->buf;
//	    GBT_NODETYPE(tbuf,&ival);
//	    if(ival != BT_OVERFLOW)
//		ajFatal("Overflow node expected but not found cache %S",
//                        cache->filename);
//	    /#
//	     ** The length pointer is restricted to the initial page.
//	     ** The keyptr in overflow pages starts at the Key Lengths
//	     ** position!
//	     #/
//	    keyptr = PBT_KEYLEN(tbuf);
//	}
*/

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
    
/*
//    if((ajuint)((keyptr-tbuf+1) + sizeof(ajulong)) > pagesize)
//    {
//	/# ajDebug("btreeGetBlockC: Overflow\n"); #/
//	GBT_OVERFLOW(tbuf,&overflow);
//	page = ajBtreeCacheRead(cache,overflow); /# pri or sec possible #/
//	tbuf = page->buf;
//	GBT_NODETYPE(tbuf,&ival);
//	if(ival != BT_OVERFLOW)
//	    ajFatal("Overflow node expected but not found cache %S",
//                    cache->filename);
//	/#
//	 ** The length pointer is restricted to the initial page.
//	 ** The keyptr in overflow pages starts at the Key Lengths
//	 ** position!
//	 #/
//	keyptr = PBT_KEYLEN(tbuf);
//    }
*/    

    BT_GETAJULONG(keyptr,&blockno);
    
    return blockno;
}




/* @funcstatic btreeGetBlockN *************************************************
**
** Get Block number matching a numeric key
**
** @param [u] cache [AjPBtcache] cache
** @param [u] buf [unsigned char *] page buffer
** @param [r] numkey [ajulong] key
**
** @return [ajulong] Block number
**
** @release 6.4.0
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




/* @funcstatic btreeGetBlockFirstN ********************************************
**
** Get Block number of first block if more than numeric key
** or second block
**
** @param [u] cache [AjPBtcache] cache
** @param [u] buf [unsigned char *] page buffer
** @param [r] numkey [ajulong] key
**
** @return [ajulong] Block number
**
** @release 6.4.0
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
**
** @release 6.4.0
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
**
** @release 6.4.0
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
**
** @release 6.4.0
** @@
******************************************************************************/

static ajulong btreeGetBlockFirstS(AjPBtcache cache, unsigned char *buf,
                                  const AjPStr key)
{
    return btreeGetBlockFirstC(cache, buf, MAJSTRGETPTR(key),0);
}




/* @funcstatic btreeIdCompare *************************************************
**
** Comparison function for ajListSort
**
** @param [r] a [const void*] ID 1
** @param [r] b [const void*] ID 2
**
** @return [ajint] 0 = bases match
**
** @release 2.8.0
** @@
******************************************************************************/

static ajint btreeIdCompare(const void *a, const void *b)
{
    return MAJSTRCMPS((*(AjPBtId const *)a)->id,
                      (*(AjPBtId const *)b)->id);
}




/* @funcstatic btreeNumIdCompare **********************************************
**
** Comparison function for ajListSort
**
** @param [r] a [const void*] ID 1
** @param [r] b [const void*] ID 2
**
** @return [ajint] 0 = bases match
**
** @release 4.0.0
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




/* @funcstatic btreeWriteNode *************************************************
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
**
** @release 2.8.0
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

    (void) cache;               /* make it used */

    lbuf = spage->buf;
    tbuf = lbuf;
    page = spage;

    v = nkeys;
    SBT_NKEYS(lbuf,v);
    
    lenptr = PBT_KEYLEN(lbuf);
    keyptr = lenptr + nkeys * sizeof(ajuint);

    for(i=0;i<nkeys;++i)
    {

/* not the right check - lenptr gets longer than this anyway */
/*
//	if((ajuint) (lenptr-lbuf+1) > cache->pagesize)
//	    ajFatal("WriteNode: Too many key lengths for available pagesize");
*/

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

/*
//	if((ajuint)((keyptr-tbuf+1) + len + sizeof(ajulong)) > cache->pagesize)
//	{
//	    if(!overflow)		/# No overflow buckets yet #/
//	    {
//		page->dirty = BT_DIRTY;
//		blockno = cache->totsize;
//		lv = blockno;
//		SBT_OVERFLOW(tbuf,lv);
//		page = ajBtreeCacheWriteNodenew(cache); /# pri/sec possible #/
//		tbuf = page->buf;
//		v = BT_OVERFLOW;
//		SBT_NODETYPE(tbuf,v);
//		lv = blockno;
//		SBT_BLOCKNUMBER(tbuf,lv);
//	    }
//	    else
//	    {
//		page = ajBtreeCacheRead(cache,overflow); /# pri/sec possible #/
//		tbuf = page->buf;
//		GBT_OVERFLOW(tbuf,&overflow);
//	    }
//
//	    keyptr = PBT_KEYLEN(tbuf);
//	    page->dirty = BT_DIRTY;
//	}
*/

	sprintf((char *)keyptr,"%s",ajStrGetPtr(keys[i]));
	keyptr += len;
	lv = ptrs[i];
	BT_SETAJULONG(keyptr,lv);
	keyptr += sizeof(ajulong);
    }
    


/*
//    if((ajuint)((keyptr-tbuf+1) + sizeof(ajulong)) > cache->pagesize)
//    {
//	page->dirty = BT_DIRTY;
//
//	if(!overflow)			/# No overflow buckets yet #/
//	{
//	    blockno = cache->totsize;
//	    lv = blockno;
//	    SBT_OVERFLOW(tbuf,lv);
//	    page = ajBtreeCacheWriteNodenew(cache); /# pri or sec possible #/
//	    tbuf = page->buf;
//	    v = BT_OVERFLOW;
//	    SBT_NODETYPE(tbuf,v);
//	}
//	else
//	{
//	    page = ajBtreeCacheRead(cache,overflow); /# pri or sec possible #/
//	    tbuf = page->buf;
//	}
//
//	keyptr = PBT_KEYLEN(tbuf);
//    }
*/    

    page->dirty = BT_DIRTY;

    overflow = 0L;
    SBT_OVERFLOW(tbuf,overflow);

    lv = ptrs[i];
    BT_SETAJULONG(keyptr,lv);
    keyptr += sizeof(ajulong);

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
**
** @release 6.4.0
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

    ajuint   len;
    ajuint    v  = 0;
    ajulong   lv = 0L;

    ajuint nkeys = 1;

    /* ajDebug("In btreeWriteNodeSingle\n"); */

    (void) cache;               /* make it used */

    lbuf = spage->buf;
    tbuf = lbuf;
    page = spage;

    v = 1;
    SBT_NKEYS(lbuf,v);
    lenptr = PBT_KEYLEN(lbuf);
    keyptr = lenptr + nkeys * sizeof(ajuint);

    len = MAJSTRGETLEN(key);
    v = len;
    BT_SETAJUINT(lenptr,v);
    lenptr += sizeof(ajuint);
    v = len;
    SBT_TOTLEN(lbuf,v);

    GBT_OVERFLOW(lbuf,&overflow);

    len = MAJSTRGETLEN(key) + 1;

    sprintf((char *)keyptr,"%s",MAJSTRGETPTR(key));
    keyptr += len;
    lv = lptr;
    BT_SETAJULONG(keyptr,lv);
    keyptr += sizeof(ajulong);
    
    overflow = 0L;
    SBT_OVERFLOW(tbuf,overflow);

    lv = rptr;
    BT_SETAJULONG(keyptr,lv);
    keyptr += sizeof(ajulong);

    page->dirty = BT_DIRTY;

    return;
}




/* @funcstatic btreeIdentQueryId **********************************************
**
** Get an ID structure from a leaf node given an identifier
**
** @param [u] cache [AjPBtcache] cache
** @param [r] key [const AjPStr] key
**
** @return [AjPBtId] Btree ID object
**
** @release 6.5.0
** @@
******************************************************************************/

static AjPBtId btreeIdentQueryId(AjPBtcache cache, const AjPStr key)
{
    AjPBtpage page   = NULL;
    AjPIdbucket bucket = NULL;
    AjPBtId   id     = NULL;
    AjPBtId   tid    = NULL;
    
    unsigned char *buf = NULL;

    ajuint nentries = 0;
    
    ajuint i;
    ajuint iref;

    ajulong blockno = 0L;
    AjBool found   = ajFalse;

    AjPStr keystr = NULL;

    /* ajDebug("In ajBtreeIdentQueryId\n"); */

    if(!cache->countunique && !cache->countall)
        return NULL;

    keystr = ajStrNewS(key);
    ajStrFmtQuery(&keystr);

    page = btreeIdentFind(cache,keystr);
    buf = page->buf;

    blockno = btreeGetBlockS(cache,buf,keystr);

    bucket = btreeReadIdbucket(cache,blockno);
    
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
	id  = ajBtreeIdNew(cache->refcount);
	tid = bucket->Ids[i];
	ajStrAssignS(&id->id,tid->id);
	id->dups = tid->dups;
	id->dbno = tid->dbno;
	id->offset = tid->offset;

        if(cache->refcount)
	{
            for(iref=0; iref < cache->refcount; iref++)
                id->refoffsets[iref] = tid->refoffsets[iref];
        }
    }

    btreeIdbucketDel(&bucket);
    ajStrDel(&keystr);

    if(!found)
	return NULL;

    return id;
}




/* @funcstatic btreeIdentQueryHit *********************************************
**
** Get a hit structure from a leaf node given an identifier
**
** @param [u] cache [AjPBtcache] cache
** @param [r] key [const AjPStr] key
**
** @return [AjPBtHit] Btree hit object
**
** @release 6.5.0
** @@
******************************************************************************/

static AjPBtHit btreeIdentQueryHit(AjPBtcache cache, const AjPStr key)
{
    AjPBtpage page   = NULL;
    AjPIdbucket bucket = NULL;
    AjPBtHit  hit    = NULL;
    AjPBtId   tid    = NULL;
    
    unsigned char *buf = NULL;

    ajuint nentries = 0;
    
    ajuint i;

    ajulong blockno = 0L;
    AjBool found   = ajFalse;

    AjPStr keystr = NULL;

    /* ajDebug("In ajBtreeIdentQueryHit\n"); */

    if(!cache->countunique && !cache->countall)
        return NULL;

    keystr = ajStrNewS(key);
    ajStrFmtQuery(&keystr);

    page = btreeIdentFind(cache,keystr);
    buf = page->buf;

    blockno = btreeGetBlockS(cache,buf,keystr);

    bucket = btreeReadIdbucket(cache,blockno);
    
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
	tid = bucket->Ids[i];
	hit = ajBtreeHitNewId(tid);
    }

    btreeIdbucketDel(&bucket);
    ajStrDel(&keystr);

    if(!found)
	return NULL;

    return hit;
}




/* @funcstatic btreeIdentQueryHitref ******************************************
**
** Get a reference hit structure from a leaf node given an identifier
**
** @param [u] cache [AjPBtcache] cache
** @param [r] key [const AjPStr] key
**
** @return [AjPBtHitref] Btree hit object
**
** @release 6.5.0
** @@
******************************************************************************/

static AjPBtHitref btreeIdentQueryHitref(AjPBtcache cache, const AjPStr key)
{
    AjPBtpage page   = NULL;
    AjPIdbucket bucket = NULL;
    AjPBtHitref hitref = NULL;
    AjPBtId   tid    = NULL;
    
    unsigned char *buf = NULL;

    ajuint nentries = 0;
    
    ajuint i;

    ajulong blockno = 0L;
    AjBool found   = ajFalse;

    AjPStr keystr = NULL;

    /* ajDebug("In ajBtreeIdentQueryHit\n"); */

    if(!cache->countunique && !cache->countall)
        return NULL;

    keystr = ajStrNewS(key);
    ajStrFmtQuery(&keystr);

    page = btreeIdentFind(cache,keystr);
    buf = page->buf;

    blockno = btreeGetBlockS(cache,buf,keystr);

    bucket = btreeReadIdbucket(cache,blockno);
    
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
	tid = bucket->Ids[i];
	hitref = ajBtreeHitrefNewId(tid);
    }

    btreeIdbucketDel(&bucket);
    ajStrDel(&keystr);

    if(!found)
	return NULL;

    return hitref;
}




/* @func ajBtreeIdentFetchId **************************************************
**
** Get an ID structure from a leaf node given an identifier
**
** @param [u] cache [AjPBtcache] cache
** @param [r] key [const AjPStr] key
** @param [w] btidlist [AjPList] List of ID objects
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ajBtreeIdentFetchId(AjPBtcache cache, const AjPStr key,
                         AjPList btidlist)
{
    AjPBtpage page   = NULL;
    AjPIdbucket bucket = NULL;
    AjPBtId   id     = NULL;
    AjPBtId   tid    = NULL;
    
    unsigned char *buf = NULL;

    ajuint nentries = 0;
    
    ajuint i;
    ajuint iref;

    ajulong blockno = 0L;
    AjBool found   = ajFalse;

    AjPStr keystr = NULL;

    /* ajDebug("In ajBtreeIdentFetch\n"); */

    if(!cache->countunique && !cache->countall)
        return;

    keystr = ajStrNewS(key);
    ajStrFmtQuery(&keystr);

    page = btreeIdentFind(cache,keystr);
    buf = page->buf;

    blockno = btreeGetBlockS(cache,buf,keystr);

    bucket = btreeReadIdbucket(cache,blockno);
    
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
	id  = ajBtreeIdNew(cache->refcount);
	tid = bucket->Ids[i];
	ajStrAssignS(&id->id,tid->id);
	id->dups = tid->dups;
	id->dbno = tid->dbno;
	id->offset = tid->offset;

        if(cache->refcount)
	{
            for(iref=0; iref < cache->refcount; iref++)
                id->refoffsets[iref] = tid->refoffsets[iref];
        }
    }

    btreeIdbucketDel(&bucket);
    ajStrDel(&keystr);

    if(!found)
	return;

    if(!id->dups)
    {
        ajListPushAppend(btidlist,(void *)id);
        id = NULL;
    }
    else
    {
        btreeIdentFetchMulti(cache, id->id, id->offset,
                             btidlist);
        ajBtreeIdDel(&id);
    }

    return;
}




/* @func ajBtreeIdentFetchHit *************************************************
**
** Get an ID hit structure from a leaf node given an identifier
**
** @param [u] cache [AjPBtcache] cache
** @param [r] key [const AjPStr] key
** @param [w] hitlist [AjPList] List of hit objects
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ajBtreeIdentFetchHit(AjPBtcache cache, const AjPStr key,
                          AjPList hitlist)
{
    AjPBtpage page   = NULL;
    AjPIdbucket bucket = NULL;
    AjPBtHit  hit    = NULL;
    AjPBtId   tid    = NULL;
    
    unsigned char *buf = NULL;

    ajuint nentries = 0;
    
    ajuint i;

    ajulong blockno = 0L;
    AjBool found   = ajFalse;

    AjPStr keystr = NULL;

    /* ajDebug("In ajBtreeIdentFetch\n"); */

    if(!cache->countunique && !cache->countall)
        return;

    if(cache->refcount)
        ajWarn("ajBtreeIdentFetchHit called for cache '%S' "
               "with %u reference files",
               cache->filename, cache->refcount);

    keystr = ajStrNewS(key);
    ajStrFmtQuery(&keystr);

    page = btreeIdentFind(cache,keystr);
    buf = page->buf;

    blockno = btreeGetBlockS(cache,buf,keystr);

    bucket = btreeReadIdbucket(cache,blockno);
    
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
	tid = bucket->Ids[i];
	hit = ajBtreeHitNewId(tid);
    }

    ajStrDel(&keystr);

    if(!found)
    {
        btreeIdbucketDel(&bucket);
	return;
    }
    
    if(!tid->dups)
    {
        ajListPushAppend(hitlist,(void *)hit);
        hit = NULL;
    }
    else
    {
        btreeIdentFetchMultiHit(cache, tid->offset,
                                hitlist);
        ajBtreeHitDel(&hit);
    }

    btreeIdbucketDel(&bucket);

    return;
}




/* @func ajBtreeIdentFetchHitref **********************************************
**
** Get an ID reference hit structure from a leaf node given an identifier
**
** @param [u] cache [AjPBtcache] cache
** @param [r] key [const AjPStr] key
** @param [w] hitlist [AjPList] List of reference hit objects
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ajBtreeIdentFetchHitref(AjPBtcache cache, const AjPStr key,
                          AjPList hitlist)
{
    AjPBtpage page   = NULL;
    AjPIdbucket bucket = NULL;
    AjPBtHitref hitref = NULL;
    AjPBtId   tid    = NULL;
    
    unsigned char *buf = NULL;

    ajuint nentries = 0;
    
    ajuint i;

    ajulong blockno = 0L;
    AjBool found   = ajFalse;

    AjPStr keystr = NULL;

    /* ajDebug("In ajBtreeIdentFetch\n"); */

    if(!cache->countunique && !cache->countall)
        return;

    if(cache->refcount != 1)
        ajWarn("ajBtreeIdentFetchHitref called for cache '%S' "
               "with %u reference files",
               cache->filename, cache->refcount);

    keystr = ajStrNewS(key);
    ajStrFmtQuery(&keystr);

    page = btreeIdentFind(cache,keystr);
    buf = page->buf;

    blockno = btreeGetBlockS(cache,buf,keystr);

    bucket = btreeReadIdbucket(cache,blockno);
    
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
	tid = bucket->Ids[i];
	hitref = ajBtreeHitrefNewId(tid);
    }

    ajStrDel(&keystr);

    if(!found)
    {
        btreeIdbucketDel(&bucket);
	return;
    }
    
    if(!tid->dups)
    {
        ajListPushAppend(hitlist,(void *)hitref);
        hitref = NULL;
    }
    else
    {
        btreeIdentFetchMultiHitref(cache, tid->offset,
                                hitlist);
        ajBtreeHitrefDel(&hitref);
    }

    btreeIdbucketDel(&bucket);

    return;
}




/* @func ajBtreeWriteParamsC **************************************************
**
** Write B+ tree parameters to file
**
** @param [r] cache [const AjPBtcache] cache
** @param [r] fntxt [const char *] file name
** @param [r] exttxt [const char *] index file extension name
** @param [r] idirtxt [const char *] index file directory
**
** @return [void]
**
** @release 6.4.0
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
        ajFmtPrintF(outf,"Type         %s\n","Identifier");
    ajFmtPrintF(outf,"Compress     %B\n",cache->compressed);
    ajFmtPrintF(outf,"Pages        %Lu\n",cache->pripagecount);
    ajFmtPrintF(outf,"Secpages     %Lu\n",cache->secpagecount);
    ajFmtPrintF(outf,"Order        %u\n",cache->porder);
    ajFmtPrintF(outf,"Fill         %u\n",cache->pnperbucket);
    ajFmtPrintF(outf,"Level        %u\n",cache->plevel);
    ajFmtPrintF(outf,"Pagesize     %u\n",cache->pripagesize);
    ajFmtPrintF(outf,"Cachesize    %u\n",cache->pricachesize);
    ajFmtPrintF(outf,"Order2       %u\n",cache->sorder);
    ajFmtPrintF(outf,"Fill2        %u\n",cache->snperbucket);
    ajFmtPrintF(outf,"Secpagesize  %u\n",cache->secpagesize);
    ajFmtPrintF(outf,"Seccachesize %u\n",cache->seccachesize);
    ajFmtPrintF(outf,"Count        %Lu\n",cache->countunique);
    ajFmtPrintF(outf,"Fullcount    %Lu\n",cache->countall);
    ajFmtPrintF(outf,"Kwlimit      %u\n",cache->keylimit);
    if(cache->secondary)
        ajFmtPrintF(outf,"Idlimit      %u\n",cache->idlimit);
    else
        ajFmtPrintF(outf,"Reffiles     %u\n",cache->refcount);

    ajFileClose(&outf);
    ajStrDel(&fname);

    return;
}




/* @func ajBtreeWriteParamsS **************************************************
**
** Write B+ tree parameters to file
**
** @param [r] cache [const AjPBtcache] cache
** @param [r] fn [const AjPStr] file name
** @param [r] ext [const AjPStr] index file extension name
** @param [r] idir [const AjPStr] index file directory
**
** @return [void]
**
** @release 6.4.0
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
        ajFmtPrintF(outf,"Type         %s\n","Secondary");
    else
        ajFmtPrintF(outf,"Type         %s\n","Identifier");
    ajFmtPrintF(outf,"Compress     %B\n",cache->compressed);
    ajFmtPrintF(outf,"Pages        %Lu\n",cache->pripagecount);
    ajFmtPrintF(outf,"Secpages     %Lu\n",cache->secpagecount);
    ajFmtPrintF(outf,"Order        %u\n",cache->porder);
    ajFmtPrintF(outf,"Fill         %u\n",cache->pnperbucket);
    ajFmtPrintF(outf,"Level        %u\n",cache->plevel);
    ajFmtPrintF(outf,"Pagesize     %u\n",cache->pripagesize);
    ajFmtPrintF(outf,"Cachesize    %u\n",cache->pricachesize);
    ajFmtPrintF(outf,"Order2       %u\n",cache->sorder);
    ajFmtPrintF(outf,"Fill2        %u\n",cache->snperbucket);
    ajFmtPrintF(outf,"Secpagesize  %u\n",cache->secpagesize);
    ajFmtPrintF(outf,"Seccachesize %u\n",cache->seccachesize);
    ajFmtPrintF(outf,"Count        %Lu\n",cache->countunique);
    ajFmtPrintF(outf,"Fullcount    %Lu\n",cache->countall);
    ajFmtPrintF(outf,"Kwlimit      %u\n",cache->keylimit);
    if(cache->secondary)
        ajFmtPrintF(outf,"Idlimit      %u\n",cache->idlimit);
    else
        ajFmtPrintF(outf,"Reffiles     %u\n",cache->refcount);

    ajFileClose(&outf);
    ajStrDel(&fname);

    return;
}




/* @func ajBtreeReadParamsC ***************************************************
**
** Read B+ tree parameters from file
**
** @param [r] filetxt [const char *] file
** @param [r] exttxt [const char *] file extension
** @param [r] idirtxt [const char *] index directory
** @param [w] secondary [AjBool*] true for a secondary index
** @param [w] compressed [AjBool*] true for a compressed index
** @param [w] kwlimit [ajuint*] maximum length of a keyword
** @param [w] idlimit [ajuint*] maximum length of a secondary id
** @param [w] refcount [ajuint*] reference file(s) per entry
** @param [w] pripagesize [ajuint*] size of primary pages
** @param [w] secpagesize [ajuint*] size of secondary pages
** @param [w] pricachesize [ajuint*] Primary cachesize
** @param [w] seccachesize [ajuint*] Secondary cachesize
** @param [w] pripagecount [ajulong*] Primary page count
** @param [w] secpagecount [ajulong*] Secondary page count
** @param [w] order [ajuint*] tree order
** @param [w] nperbucket [ajuint*] bucket fill
** @param [w] level [ajuint*] depth of tree (0 = root leaf)
** @param [w] sorder [ajuint*] secondary tree order
** @param [w] snperbucket [ajuint*] secondary bucket fill
** @param [w] count [ajulong*] number of primary keywords in the index
** @param [w] countall [ajulong*] number of total keywords in the index
**
** @return [AjBool] True on success
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajBtreeReadParamsC(const char *filetxt, const char *exttxt,
                          const char *idirtxt,
                          AjBool *secondary, AjBool *compressed,
                          ajuint *kwlimit, ajuint *idlimit,
                          ajuint *refcount,
                          ajuint *pripagesize, ajuint *secpagesize,
                          ajuint *pricachesize, ajuint *seccachesize,
                          ajulong *pripagecount, ajulong *secpagecount,
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
    *pripagecount = 0L;
    *secpagecount = 0L;
    *refcount = 1;
    *secpagesize = 0;
    *seccachesize = 0;
    *idlimit = 0;

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
	    ajFmtScanS(line,"%*s%Lu",pripagecount);

	if(ajStrPrefixC(line,"Secpages "))
	    ajFmtScanS(line,"%*s%Lu",secpagecount);

	if(ajStrPrefixC(line,"Order "))
	    ajFmtScanS(line,"%*s%u",order);

	if(ajStrPrefixC(line,"Fill "))
	    ajFmtScanS(line,"%*s%u",nperbucket);

	if(ajStrPrefixC(line,"Pagesize "))
	    ajFmtScanS(line,"%*s%u",pripagesize);

	if(ajStrPrefixC(line,"Secpagesize "))
	    ajFmtScanS(line,"%*s%u",secpagesize);

	if(ajStrPrefixC(line,"Level "))
	    ajFmtScanS(line,"%*s%u",level);

	if(ajStrPrefixC(line,"Cachesize "))
	    ajFmtScanS(line,"%*s%u",pricachesize);

	if(ajStrPrefixC(line,"Seccachesize "))
	    ajFmtScanS(line,"%*s%u",seccachesize);

	if(ajStrPrefixC(line,"Count "))
	    ajFmtScanS(line,"%*s%Lu",count);

	if(ajStrPrefixC(line,"Fullcount "))
	    ajFmtScanS(line,"%*s%Lu",countall);

	if(ajStrPrefixC(line,"Kwlimit "))
	    ajFmtScanS(line,"%*s%u",kwlimit);

	if(ajStrPrefixC(line,"Idlimit "))
            ajFmtScanS(line,"%*s%u",idlimit);
        
	if(ajStrPrefixC(line,"Reffiles "))
	    ajFmtScanS(line,"%*s%u",refcount);
    }

    if(!setsecond)
    {
        *secondary = ajBtreeFieldGetSecondaryC(exttxt);
    }

    if(!*pripagecount && !*compressed)
    {
        if(!*idirtxt)
            ajFmtPrintS(&fname,"%s.%s",filetxt,exttxt);
        else if(idirtxt[strlen(idirtxt)-1] == SLASH_CHAR)
            ajFmtPrintS(&fname,"%s%s.%s",idirtxt,filetxt,exttxt);
        else
	    ajFmtPrintS(&fname,"%s%s%s.%s",
			idirtxt,SLASH_STRING, filetxt,exttxt);
        *pripagecount = ajFilenameGetSize(fname) / *pripagesize;
        *secpagecount = 0;
    }

    if(!*secpagesize)
        *secpagesize = *pripagesize;

    if(!*seccachesize)
        *seccachesize = *pricachesize;

    if(*secondary && !*idlimit)
        *idlimit = *kwlimit;

    ajFileClose(&inf);
    ajStrDel(&fname);
    ajStrDel(&line);
    ajStrDel(&type);
    
    return ajTrue;
}




/* @func ajBtreeReadParamsS ***************************************************
**
** Read B+ tree parameters from file
**
** @param [r] file [const AjPStr] file
** @param [r] ext [const AjPStr] file extension
** @param [r] idir [const AjPStr] index directory
** @param [w] secondary [AjBool*] true for a secondary index
** @param [w] compressed [AjBool*] true for a compressed index
** @param [w] kwlimit [ajuint*] maximum length of a keyword
** @param [w] idlimit [ajuint*] maximum length of a secondary id
** @param [w] refcount [ajuint*] reference file(s) per entry
** @param [w] pripagesize [ajuint*] size of primary pages
** @param [w] secpagesize [ajuint*] size of secondary pages
** @param [w] pricachesize [ajuint*] Primary cachesize
** @param [w] seccachesize [ajuint*] Secondary cachesize
** @param [w] pripagecount [ajulong*] Primary page count
** @param [w] secpagecount [ajulong*] Secondary page count
** @param [w] order [ajuint*] tree order
** @param [w] nperbucket [ajuint*] bucket fill
** @param [w] level [ajuint*] depth of tree (0 = root leaf)
** @param [w] sorder [ajuint*] secondary tree order
** @param [w] snperbucket [ajuint*] secondary bucket fill
** @param [w] count [ajulong*] number of primary keywords in the index
** @param [w] countall [ajulong*] number of total keywords in the index
**
** @return [AjBool] True on success
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajBtreeReadParamsS(const AjPStr file, const AjPStr ext,
                          const AjPStr idir,
                          AjBool *secondary, AjBool *compressed,
                          ajuint *kwlimit, ajuint *idlimit,
                          ajuint *refcount,
                          ajuint *pripagesize, ajuint *secpagesize,
                          ajuint *pricachesize, ajuint *seccachesize,
                          ajulong *pripagecount, ajulong *secpagecount,
                          ajuint *order, ajuint *nperbucket, ajuint *level,
                          ajuint *sorder, ajuint *snperbucket,
                          ajulong *count, ajulong *countall)
{
    return ajBtreeReadParamsC(MAJSTRGETPTR(file), MAJSTRGETPTR(ext),
                              MAJSTRGETPTR(idir), secondary, compressed,
                              kwlimit, idlimit, refcount,
                              pripagesize, secpagesize,
                              pricachesize, seccachesize,
                              pripagecount, secpagecount,
                              order, nperbucket, level,
                              sorder, snperbucket,
                              count, countall);
}




#if 0
/* #funcstatic btreeIdentSplitleaf ********************************************
**
** Split an identifier primary leaf and propagate up if necessary
**
** #param [u] cache [AjPBtcache] cache
** #param [u] spage [AjPBtpage] page
**
** #return [AjPBtpage] pointer to a parent page
**
** #release 6.5.0
** ##
******************************************************************************/
/*
static AjPBtpage btreeIdentSplitleaf(AjPBtcache cache, AjPBtpage spage)
{
    ajuint nkeys     = 0;
    ajuint order     = 0;
    ajuint totalkeys = 0;
    ajuint keylimit  = 0;
    ajuint nodetype  = 0;

    ajuint rootnodetype  = 0;
    
    ajuint i;
    ajuint j;
    ajuint iref;

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

    AjPIdbucket cbucket  = NULL;
    
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
    ajuint refskip = cache->refcount*BT_EXTRA;

#if AJINDEX_DEBUG
    ajDebug("btreeIdentSplitleaf %Lu\n", spage->pagepos);
#endif
    ++statCallIdentSplitleaf;

    /# ajDebug("In btreeIdentSplitleaf\n"); #/

    order = cache->porder;

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
	/# ajDebug("Splitting root node\n"); #/
	lblockno = cache->totsize;
	lpage = btreePricacheNodenew(cache);
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
    rpage = btreePricacheNodenew(cache);
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
	btreeIdbucketIdlist(cache,parray[i],idlist);

    ajListSort(idlist, &btreeIdCompare);

    totalkeys = ajListGetLength(idlist);

    btreeBucketSplitCalc(totalkeys, keylimit, cache->pnperbucket,
                         &lbucketlimit,&lmaxnperbucket,&lno,
                         &rbucketlimit,&rmaxnperbucket,&rno);

    cbucket = btreeIdbucketNew(cache->pnperbucket, cache->refcount);

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

            if(cache->refcount)
            {
                for(iref=0; iref < cache->refcount; iref++)
                    cid->refoffsets[iref] = bid->refoffsets[iref];
	    }

	    cbucket->keylen[j] =
                BT_BUCKIDLEN(bid->id) + refskip;
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

	btreeWriteIdbucket(cache,cbucket,newparray[i]);
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

        if(cache->refcount)
        {
            for(iref=0; iref < cache->refcount; iref++)
                cid->refoffsets[iref] = bid->refoffsets[iref];
        }

	++cbucket->Nentries;
	ajBtreeIdDel(&bid);
    }

    if((iold < order) && parray[iold])
        newparray[i] = parray[iold++];
    else
        newparray[i] = cache->totsize;
    btreeWriteIdbucket(cache,cbucket,newparray[i]);

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

            if(cache->refcount)
            {
                for(iref=0; iref < cache->refcount; iref++)
                    cid->refoffsets[iref] = bid->refoffsets[iref];
            }

	    cbucket->keylen[j] =
                BT_BUCKIDLEN(bid->id)+ refskip;
	    ++cbucket->Nentries;
	    ajBtreeIdDel(&bid);
	}

	ajListPeek(idlist,(void **)&bid);
	ajStrAssignS(&newkarray[i],bid->id);

        if((iold < order) && parray[iold])
            newparray[i] = parray[iold++];
        else
            newparray[i] = cache->totsize;
	btreeWriteIdbucket(cache,cbucket,newparray[i]);
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

        if(cache->refcount)
        {
            for(iref=0; iref < cache->refcount; iref++)
                cid->refoffsets[iref] = bid->refoffsets[iref];
	}

	++cbucket->Nentries;
	ajBtreeIdDel(&bid);
    }
    
    if((iold < order) && parray[iold])
        newparray[i] = parray[iold++];
    else
        newparray[i] = cache->totsize;
    btreeWriteIdbucket(cache,cbucket,newparray[i]);

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

    btreeIdbucketDel(&cbucket);
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
	++cache->plevel;

	return spage;
    }


    page = btreePricacheRead(cache,prevsave);
    btreePriInsertKey(cache,page,mediankey,medianless,mediangtr);
    ajStrDel(&mediankey);

    page = btreePricacheRead(cache,prevsave);

    return page;
}
*/
#endif




/* @funcstatic btreeKeyInsertShift ********************************************
**
** Rebalance buckets on insertion of a key
**
** @param [u] cache [AjPBtcache] cache
** @param [u] retpage [AjPBtpage*] page
** @param [r] key [const AjPStr] key
**
** @return [ajulong] bucket block or 0L if shift not possible
**
** @release 6.5.0
** @@
******************************************************************************/

static ajulong btreeKeyInsertShift(AjPBtcache cache, AjPBtpage *retpage,
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
    
    /* ajDebug("In btreeKeyInsertShift\n"); */


    tpage = *retpage;

    tbuf = tpage->buf;

    GBT_PREV(tbuf,&parent);
    GBT_NKEYS(tbuf,&tkeys);

    order = cache->porder;
    minsize = order / 2;

    if(order % 2)
	++minsize;

    if(tkeys <= minsize)
	return 0L;

    ppage = btreePricacheRead(cache,parent);
    
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
	spage = btreePricacheRead(cache,pParray[ppos-1]);
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
	spage = btreePricacheRead(cache,pParray[ppos+1]);
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




/* @funcstatic btreePrimaryShift **********************************************
**
** Rebalance primary nodes (identifiers or keywords) on insertion
**
** @param [u] cache [AjPBtcache] cache
** @param [u] tpage [AjPBtpage] page
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void btreePrimaryShift(AjPBtcache cache, AjPBtpage tpage)
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
    
    /* ajDebug("In btreePrimaryShift\n"); */
    
    tbuf = tpage->buf;

    GBT_PREV(tbuf,&parent);
    GBT_NKEYS(tbuf,&tkeys);

    order = cache->porder;
    minsize = order / 2;

    if(order % 2)
	++minsize;

    if(tkeys <= minsize)
	return;

    
    ppage = btreePricacheRead(cache,parent);
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
	spage = btreePricacheRead(cache,pParray[pkeypos]);
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

	page = btreePricacheRead(cache,pSarray[skeys]);
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
	spage = btreePricacheRead(cache,pParray[pkeypos+1]);
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
        
	page = btreePricacheRead(cache,pSarray[0]);
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
/* @funcstatic btreeTraverseLeaves ********************************************
**
** Find the next leaf by traversing the tree
**
** @param [u] cache [AjPBtcache] cache
** @param [u] thys [AjPBtpage] current leaf page
**
** @return [AjPBtpage] next leaf or NULL
**
** @release 3.0.0
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

    if(!cache->plevel)
	return NULL;

    order = cache->porder;
    arrays = btreeAllocPriArray(cache);
    parray = arrays->Parray;

    pagepos = thys->pagepos;
    buf = thys->buf;
    GBT_PREV(buf,&prev);

    page = btreePricacheRead(cache,prev);
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
	page = btreePricacheRead(cache,prev);
	buf = page->buf;
	GBT_NKEYS(buf,&nkeys);
	GBT_NODETYPE(buf,&nodetype);
	btreeGetPointers(cache,buf,&parray);

	apos = 0;

	while(parray[apos] != pagepos)
	    ++apos;
    }

    page = btreePricacheRead(cache,parray[apos+1]);
    buf = page->buf;
    GBT_NODETYPE(buf,&nodetype);
    btreeGetPointers(cache,buf,&parray);
    
    while(nodetype != BT_LEAF)
    {
	page = btreePricacheRead(cache,parray[0]);
	buf = page->buf;
	btreeGetPointers(cache,buf,&parray);
	GBT_NODETYPE(buf,&nodetype);
    }

    btreeDeallocPriArray(cache,arrays);

    return page;
}
#endif




#if 0
/* @funcstatic btreeJoinLeaves ************************************************
**
** Update all Left/Right Leaf Pointers
**
** @param [u] cache [AjPBtcache] cache
**
** @return [void] next leaf or NULL
**
** @release 3.0.0
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
    
    if(!cache->plevel)
	return;

    order = cache->porder;
    arrays = btreeAllocPriArray(cache);
    parray = arrays->Parray;

    page = btreePricacheLocate(cache,0L);
    buf = page->buf;
    btreeGetPointers(cache,buf,&parray);
    GBT_NODETYPE(buf,&nodetype);

    while(nodetype != BT_LEAF)
    {
	page = btreePricacheRead(cache,parray[0]);
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




/* @func ajBtreeIdwildNew *****************************************************
**
** Construct a wildcard id search object
**
** @param [u] cache [AjPBtcache] cache
** @param [r] wild [const AjPStr] wildcard id prefix (without asterisk)
**
** @return [AjPBtIdwild] b+ tree wildcard object
**
** @release 3.0.0
** @@
******************************************************************************/

AjPBtIdwild ajBtreeIdwildNew(AjPBtcache cache, const AjPStr wild)
{
    AjPBtIdwild thys = NULL;

    (void) cache;			/* make it used */

    AJNEW0(thys);

    thys->id   = ajStrNewS(wild);
    ajStrTrimC(&thys->id,"*"); /* Need to revisit this */
    thys->list = ajListNew();

    thys->first = ajTrue;

    return thys;
}




/* @func ajBtreeIdwildDel *****************************************************
**
** Destroy a wildcard search object
**
** @param [u] Pthis [AjPBtIdwild*] b+ tree identifier wildcard structure
**
** @return [void]
**
** @release 3.0.0
** @@
******************************************************************************/

void ajBtreeIdwildDel(AjPBtIdwild *Pthis)
{
    AjPBtIdwild thys = NULL;
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




/* @func ajBtreeKeywildNew ****************************************************
**
** Construct a wildcard keyword search object
**
** @param [u] cache [AjPBtcache] cache
** @param [r] wild [const AjPStr] wildcard keyword prefix (without asterisk)
**
** @return [AjPBtKeywild] b+ tree keyword wildcard object
**
** @release 3.0.0
** @@
******************************************************************************/

AjPBtKeywild ajBtreeKeywildNew(AjPBtcache cache, const AjPStr wild)
{
    AjPBtKeywild thys = NULL;

    char *p;
    char *cp;

    AJNEW0(thys);

    thys->keyword = ajStrNewS(wild);
    ajStrFmtQuery(&thys->keyword);
    if(MAJSTRGETLEN(thys->keyword) > cache->keylimit)
        ajStrTruncateLen(&thys->keyword, cache->keylimit);

    cp = MAJSTRGETPTR(thys->keyword);
    p = strpbrk(cp,"*?");

    if(p)
    {
	if(p-cp)
	    ajStrAssignSubS(&thys->prefix,thys->keyword,0,p-cp-1);
	else
            ajStrAssignC(&thys->prefix, "");
    }
    else
	ajStrAssignS(&thys->prefix,thys->keyword);

    thys->list   = ajListNew();
    thys->idlist = ajListNew();

    thys->first = ajTrue;

    return thys;
}




/* @func ajBtreeKeywildDel ****************************************************
**
** Destroy a wildcard keyword search object
**
** @param [u] Pthis [AjPBtKeywild*] b+ tree wildcard keyword structure
**
** @return [void]
**
** @release 3.0.0
** @@
******************************************************************************/

void ajBtreeKeywildDel(AjPBtKeywild *Pthis)
{
    AjPBtKeywild thys = NULL;
    AjPStr id = NULL;
    AjPBtPri pri = NULL;
    
    if(!Pthis || !*Pthis)
	return;

    thys = *Pthis;

    ajStrDel(&thys->keyword);
    ajStrDel(&thys->prefix);

    while(ajListPop(thys->idlist,(void **)&id))
	ajStrDel(&id);

    while(ajListPop(thys->list,(void **)&pri))
	ajBtreePriDel(&pri);

    ajListFree(&thys->list);

    AJFREE(thys);
    *Pthis = NULL;

    return;
}




/* @funcstatic btreePrimaryFetchFindleafWild **********************************
**
** Find the node that should contain a wildcard identifier or keyword
**
** @param [u] cache [AjPBtcache] cache
** @param [r] key [const AjPStr] key to search for 
**
** @return [AjPBtpage] leaf node where item should be inserted
**
** @release 6.5.0
** @@
******************************************************************************/

static AjPBtpage btreePrimaryFetchFindleafWild(AjPBtcache cache,
                                               const AjPStr key)
{
    AjPBtpage root = NULL;
    AjPBtpage ret  = NULL;

    /* ajDebug("In btreePrimaryFetchFindleafWild\n"); */

    /* The root node should always be in the cache (BT_LOCKed) */
    root = btreePricacheLocate(cache,0L);
    
    if(!cache->plevel)
	return root;
    
    ret = btreePrimaryFindInodeWild(cache,root,key);

    return ret;
}




/* @funcstatic btreePrimaryFindInodeWild **************************************
**
** Recursive search for node (wild)
**
** @param [u] cache [AjPBtcache] cache
** @param [u] page [AjPBtpage] page
** @param [r] item [const AjPStr] key to search for 
**
** @return [AjPBtpage] leaf node where item should be inserted
**
** @release 6.5.0
** @@
******************************************************************************/

static AjPBtpage btreePrimaryFindInodeWild(AjPBtcache cache, AjPBtpage page,
                                           const AjPStr item)
{
    AjPBtpage ret = NULL;
    AjPBtpage pg  = NULL;

    unsigned char *buf = NULL;

    ajuint status = 0;
    ajuint ival   = 0;

    /* ajDebug("In btreePrimaryFindInodeWild\n"); */
    
    ret = page;
    buf = page->buf;
    GBT_NODETYPE(buf,&ival);

    if(ival != BT_LEAF)
    {
	status = ret->dirty;
	ret->dirty = BT_LOCK;	/* Lock in case of lots of overflow pages */
        ret->lockfor = 1181;
	pg = btreePrimaryPageDownWild(cache,buf,item);
	ret->dirty = status;
	ret = btreePrimaryFindInodeWild(cache,pg,item);
    }
    
    return ret;
}




/* @funcstatic btreePrimaryPageDownWild ***************************************
**
** Return next lower index page given a wildcard key
**
** @param [u] cache [AjPBtcache] cache
** @param [u] buf [unsigned char *] page buffer
** @param [r] key [const AjPStr] key to search for 
**
** @return [AjPBtpage] pointer to a page
**
** @release 2.8.0
** @@
******************************************************************************/

static AjPBtpage btreePrimaryPageDownWild(AjPBtcache cache, unsigned char *buf,
                                          const AjPStr key)
{
    ajulong blockno = 0L;
    AjPBtpage page = NULL;

    unsigned char *rootbuf = NULL;
    
    /* ajDebug("In btreePrimaryPageDownWild\n"); */
    
    rootbuf = buf;

    blockno = btreeGetBlockS(cache,rootbuf,key);
    page =  btreePricacheRead(cache,blockno);

    return page;
}




/* @funcstatic btreeIdleafFetch ***********************************************
**
** Read all leaf identifiers into a list
**
** @param [u] cache [AjPBtcache] cache
** @param [u] page [AjPBtpage] page
** @param [w] list [AjPList] list of AjPBtIds
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void btreeIdleafFetch(AjPBtcache cache, AjPBtpage page,
                               AjPList list)
{
    unsigned char *buf = NULL;
    AjPBtMem arrays    = NULL;
    ajulong *parray     = NULL;

    ajuint keylimit = 0;
    ajuint nkeys    = 0;

    ajuint i;
    
    /* ajDebug("In btreeIdleafFetch\n"); */
    
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
	btreeIdbucketIdlistAll(cache,parray[i],list);

    /*
    ** sort the list so we can compare prefix until no more matches
    */

    ajListSort(list, &btreeIdCompare);

    btreeDeallocPriArray(cache,arrays);

    return;
}




/* @func ajBtreeIdwildQuery ***************************************************
**
** Wildcard retrieval of entries by identifier
**
** @param [u] cache [AjPBtcache] cache
** @param [u] wild [AjPBtIdwild] Wildcard
**
** @return [AjPBtId] next matching Id or NULL
**
** @release 6.5.0
** @@
******************************************************************************/

AjPBtId ajBtreeIdwildQuery(AjPBtcache cache, AjPBtIdwild wild)
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
	page = btreePrimaryFetchFindleafWild(cache,key);
	page->dirty = BT_LOCK;
        page->lockfor = 1191;
	wild->pagepos = page->pagepos;
	
	btreeIdleafFetch(cache,page,list);

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

	    page = btreePricacheRead(cache,pagepos);
	    wild->pagepos = pagepos;
	    page->dirty = BT_LOCK;
            page->lockfor = 1192;

	    btreeIdleafFetch(cache,page,list);	
	    
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
	page = btreePricacheRead(cache,wild->pagepos); 
	buf = page->buf;
	GBT_RIGHT(buf,&pagepos);

	if(!pagepos)
	    return NULL;

	page = btreePricacheRead(cache,pagepos);
	wild->pagepos = pagepos;
	page->dirty = BT_LOCK;
        page->lockfor = 1193;

	btreeIdleafFetch(cache,page,list);	

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




/* @func ajBtreeIdentFetchwildId **********************************************
**
** Wildcard retrieval of entries
**
** @param [u] cache [AjPBtcache] cache
** @param [r] key [const AjPStr] Wildcard key
** @param [u] idlist [AjPList] list of matching AjPBtIds
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ajBtreeIdentFetchwildId(AjPBtcache cache, const AjPStr key,
                             AjPList idlist)
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

    ajDebug("ajBtreeIdentFetchwildId '%S' list: %Lu\n",
            key, ajListGetLength(list));

    if(!cache->countunique && !cache->countall)
        return;

    keystr = ajStrNewS(key);
    ajStrFmtQuery(&keystr);

    if(MAJSTRGETLEN(keystr) > cache->keylimit)
        ajStrTruncateLen(&keystr, cache->keylimit);
    
    cp = MAJSTRGETPTR(keystr);
    p = strpbrk(cp,"*?");

    if(p)
    {
	if(p-cp)
	    ajStrAssignSubS(&prefix,keystr,0,p-cp-1);
	else
	{
	    btreeKeyFullSearchId(cache,keystr,idlist);
            ajStrDel(&keystr);
	    ajStrDel(&prefix);
	    return;
	}
    }
    else
	ajStrAssignS(&prefix,keystr);

    ajStrFmtQuery(&prefix);

    if(MAJSTRGETLEN(prefix) > cache->keylimit)
        ajStrTruncateLen(&prefix, cache->keylimit);

    ajDebug("ajBtreeIdentFetchwild '%S' prefix: '%S'\n",
            keystr, prefix);

    list = ajListNew();

    found = ajFalse;

    page = btreePrimaryFetchFindleafWild(cache,prefix);
    page->dirty = BT_LOCK;
    page->lockfor = 1201;
    pripagepos = page->pagepos;
    
    btreeIdleafFetch(cache,page,list);
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
    
    ajDebug("ajBtreeIdentFetchwild first leaf prefix found: %B\n", found);
    
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

	page = btreePricacheRead(cache,right);
	pripagepos = right;
	page->dirty = BT_LOCK;
        page->lockfor = 1202;

	btreeIdleafFetch(cache,page,list);	
	    
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
        ajDebug("ajBtreeIdentFetchwild check next prefix found: %B\n", found);
    }
    
    
    if(!found)
    {
        ajStrDel(&keystr);
	ajStrDel(&prefix);
	ajListFree(&list);

	return;
    }
    

    finished = ajFalse;

    /*
    ** check current ID against full wildcard query
    ** read next ID (new list when list is empty)
    ** stop when no more to read or prefix no longer matches
    */

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
	    page = btreePricacheRead(cache,pripagepos);
	    buf = page->buf;
	    GBT_RIGHT(buf,&right);

	    if(!right)
	    {
		finished = ajTrue;
		continue;
	    }

	    page = btreePricacheRead(cache,right);
	    page->dirty = BT_LOCK;
            page->lockfor = 1203;
	    buf = page->buf;
	    pripagepos = right;
	    
	    btreeIdleafFetch(cache,page,list);	
	    
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

    /*
    ** clear remaining (non-matching) ids in the latest list
    */

    while(ajListPop(list,(void **)&id))
	ajBtreeIdDel(&id);
    ajListFree(&list);

    ajListSortTwoUnique(idlist, &btreeIdDbnoCompare, &btreeIdOffsetCompare,
                        &btreeIdDelFromList);

    ajStrDel(&prefix);
    ajStrDel(&keystr);

    return;
}




/* @func ajBtreeIdentFetchwildHit *********************************************
**
** Wildcard retrieval of entries
**
** @param [u] cache [AjPBtcache] cache
** @param [r] key [const AjPStr] Wildcard key
** @param [u] hitlist [AjPList] list of matching AjPBtHits
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ajBtreeIdentFetchwildHit(AjPBtcache cache, const AjPStr key,
                              AjPList hitlist)
{

    AjPBtId id     = NULL;
    AjPBtHit hit   = NULL;
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

    ajDebug("ajBtreeIdentFetchwildHit '%S' list: %Lu\n",
            key, ajListGetLength(list));

    if(!cache->countunique && !cache->countall)
        return;

    keystr = ajStrNewS(key);
    ajStrFmtQuery(&keystr);

    if(MAJSTRGETLEN(keystr) > cache->keylimit)
        ajStrTruncateLen(&keystr, cache->keylimit);
    
    cp = MAJSTRGETPTR(keystr);
    p = strpbrk(cp,"*?");

    if(p)
    {
	if(p-cp)
	    ajStrAssignSubS(&prefix,keystr,0,p-cp-1);
	else
	{
	    btreeKeyFullSearchHit(cache,keystr,hitlist);
            ajStrDel(&keystr);
	    ajStrDel(&prefix);
	    return;
	}
    }
    else
	ajStrAssignS(&prefix,keystr);

    ajStrFmtQuery(&prefix);

    if(MAJSTRGETLEN(prefix) > cache->keylimit)
        ajStrTruncateLen(&prefix, cache->keylimit);

    ajDebug("ajBtreeIdentFetchwildHit '%S' prefix: '%S'\n",
            keystr, prefix);

    list = ajListNew();

    found = ajFalse;

    page = btreePrimaryFetchFindleafWild(cache,prefix);
    page->dirty = BT_LOCK;
    page->lockfor = 1201;
    pripagepos = page->pagepos;
    
    btreeIdleafFetch(cache,page,list);
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
    
    ajDebug("ajBtreeIdentFetchwildHit first leaf prefix found: %B\n", found);
    
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

	page = btreePricacheRead(cache,right);
	pripagepos = right;
	page->dirty = BT_LOCK;
        page->lockfor = 1202;

	btreeIdleafFetch(cache,page,list);	
	    
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
        ajDebug("ajBtreeIdentFetchwildHit check next prefix found: %B\n",
                found);
    }
    
    
    if(!found)
    {
        ajStrDel(&keystr);
	ajStrDel(&prefix);
	ajListFree(&list);

	return;
    }
    

    finished = ajFalse;

    /*
    ** check current ID against full wildcard query
    ** read next ID (new list when list is empty)
    ** stop when no more to read or prefix no longer matches
    */

    while(!finished)
    {
	if(ajStrMatchWildS(id->id,keystr))
        {
            hit = ajBtreeHitNewId(id);
	    ajListPush(hitlist,(void *)hit);
            hit = NULL;
        }
        ajBtreeIdDel(&id);
	
	if(!ajListGetLength(list))
	{
	    page = btreePricacheRead(cache,pripagepos);
	    buf = page->buf;
	    GBT_RIGHT(buf,&right);

	    if(!right)
	    {
		finished = ajTrue;
		continue;
	    }

	    page = btreePricacheRead(cache,right);
	    page->dirty = BT_LOCK;
            page->lockfor = 1203;
	    buf = page->buf;
	    pripagepos = right;
	    
	    btreeIdleafFetch(cache,page,list);	
	    
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

    /*
    ** clear remaining (non-matching) ids in the latest list
    */

    while(ajListPop(list,(void **)&id))
	ajBtreeIdDel(&id);
    ajListFree(&list);

    ajListSortTwoUnique(hitlist,
                        &btreeHitDbnoCompare, &btreeHitOffsetCompare,
                        &btreeHitDelFromList);

    ajStrDel(&prefix);
    ajStrDel(&keystr);

    return;
}




/* @func ajBtreeIdentFetchwildHitref ******************************************
**
** Wildcard retrieval of entries
**
** @param [u] cache [AjPBtcache] cache
** @param [r] key [const AjPStr] Wildcard key
** @param [u] hitlist [AjPList] list of matching AjPBtHitrefs
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ajBtreeIdentFetchwildHitref(AjPBtcache cache, const AjPStr key,
                                 AjPList hitlist)
{

    AjPBtId id     = NULL;
    AjPBtHitref hitref = NULL;
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

    ajDebug("ajBtreeIdentFetchwildHitref '%S' list: %Lu\n",
            key, ajListGetLength(list));

    if(!cache->countunique && !cache->countall)
        return;

    keystr = ajStrNewS(key);
    ajStrFmtQuery(&keystr);

    if(MAJSTRGETLEN(keystr) > cache->keylimit)
        ajStrTruncateLen(&keystr, cache->keylimit);
    
    cp = MAJSTRGETPTR(keystr);
    p = strpbrk(cp,"*?");

    if(p)
    {
	if(p-cp)
	    ajStrAssignSubS(&prefix,keystr,0,p-cp-1);
	else
	{
	    btreeKeyFullSearchHitref(cache,keystr,hitlist);
            ajStrDel(&keystr);
	    ajStrDel(&prefix);
	    return;
	}
    }
    else
	ajStrAssignS(&prefix,keystr);

    ajStrFmtQuery(&prefix);

    if(MAJSTRGETLEN(prefix) > cache->keylimit)
        ajStrTruncateLen(&prefix, cache->keylimit);

    ajDebug("ajBtreeIdentFetchwildHitref '%S' prefix: '%S'\n",
            keystr, prefix);

    list = ajListNew();

    found = ajFalse;

    page = btreePrimaryFetchFindleafWild(cache,prefix);
    page->dirty = BT_LOCK;
    page->lockfor = 1201;
    pripagepos = page->pagepos;
    
    btreeIdleafFetch(cache,page,list);
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
    
    ajDebug("ajBtreeIdentFetchwildHitref first leaf prefix found: %B\n", found);
    
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

	page = btreePricacheRead(cache,right);
	pripagepos = right;
	page->dirty = BT_LOCK;
        page->lockfor = 1202;

	btreeIdleafFetch(cache,page,list);	
	    
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
        ajDebug("ajBtreeIdentFetchwildHitref check next prefix found: %B\n",
                found);
    }
    
    
    if(!found)
    {
        ajStrDel(&keystr);
	ajStrDel(&prefix);
	ajListFree(&list);

	return;
    }
    

    finished = ajFalse;

    /*
    ** check current ID against full wildcard query
    ** read next ID (new list when list is empty)
    ** stop when no more to read or prefix no longer matches
    */

    while(!finished)
    {
	if(ajStrMatchWildS(id->id,keystr))
        {
            hitref = ajBtreeHitrefNewId(id);
	    ajListPush(hitlist,(void *)hitref);
            hitref = NULL;
        }
        ajBtreeIdDel(&id);
	
	if(!ajListGetLength(list))
	{
	    page = btreePricacheRead(cache,pripagepos);
	    buf = page->buf;
	    GBT_RIGHT(buf,&right);

	    if(!right)
	    {
		finished = ajTrue;
		continue;
	    }

	    page = btreePricacheRead(cache,right);
	    page->dirty = BT_LOCK;
            page->lockfor = 1203;
	    buf = page->buf;
	    pripagepos = right;
	    
	    btreeIdleafFetch(cache,page,list);	
	    
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

    /*
    ** clear remaining (non-matching) ids in the latest list
    */

    while(ajListPop(list,(void **)&id))
	ajBtreeIdDel(&id);
    ajListFree(&list);

    ajListSortTwoUnique(hitlist,
                        &btreeHitrefDbnoCompare, &btreeHitrefOffsetCompare,
                        &btreeHitrefDelFromList);

    ajStrDel(&prefix);
    ajStrDel(&keystr);

    return;
}




/* @funcstatic btreeKeyFullSearchId *******************************************
**
** Wildcard retrieval of id/acc/sv entries. Whole index scan. Only used for
** wildcard searches with keys beginning with '?' or '*'
**
** @param [u] cache [AjPBtcache] cache
** @param [r] key [const AjPStr] Wildcard key
** @param [u] idlist [AjPList] list of matching AjPBtIds
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void btreeKeyFullSearchId(AjPBtcache cache, const AjPStr key,
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

    ajulong idlen = 0UL;

    ajDebug("btreeKeyFullSearchId key '%S'\n", key);

    list   = ajListNew();

    root = btreePricacheLocate(cache, 0L);
    page = root;
    
    buf = root->buf;
    GBT_NODETYPE(buf,&nodetype);
    
    if(cache->plevel)
    {
        arrays = btreeAllocPriArray(cache);
        parray = arrays->parray;

        /* down to the leftmost leaf node */

	while(nodetype != BT_LEAF)
	{
	    btreeGetPointers(cache,buf,&parray);
	    page = btreePricacheRead(cache, parray[0]);
	    buf = page->buf;
	    GBT_NODETYPE(buf,&nodetype);
	    page->dirty = BT_CLEAN;
	}

        btreeDeallocPriArray(cache,arrays);
    }

    right = 99L;

    /* read left-to-right through all leaf nodes */

    while(right)
    {
	btreeIdleafFetch(cache,page,list);

	while(ajListPop(list,(void **)&id))
	{
	    if(ajStrMatchWildS(id->id,key))
		ajListPushAppend(idlist,(void *)id);
	    else
		ajBtreeIdDel(&id);
	}

        if(idlen != ajListGetLength(idlist))
        {
            ajDebug("Page %Lu found %Lu\n", page->pagepos,
                    ajListGetLength(idlist) - idlen);
            idlen = ajListGetLength(idlist);
        }

	GBT_RIGHT(buf,&right);

	if(right)
	{
	    page = btreePricacheRead(cache,right);
	    buf = page->buf;
	}
    }
    

    ajListSortTwoUnique(idlist, &btreeIdDbnoCompare, &btreeIdOffsetCompare,
                        &btreeIdDelFromList);

    ajListFree(&list);

    return;
}




/* @funcstatic btreeKeyFullSearchHit ******************************************
**
** Wildcard retrieval of id/acc/sv entries. Whole index scan. Only used for
** wildcard searches with keys beginning with '?' or '*'
**
** @param [u] cache [AjPBtcache] cache
** @param [r] key [const AjPStr] Wildcard key
** @param [u] hitlist [AjPList] list of matching AjPBtHits
**
** @return [void]
**
** @release 3.0.0
** @@
******************************************************************************/

static void btreeKeyFullSearchHit(AjPBtcache cache, const AjPStr key,
                                  AjPList hitlist)
{
    AjPBtId  id    = NULL;
    AjPBtHit hit   = NULL;
    AjPBtpage root = NULL;
    AjPBtpage page = NULL;
    ajulong right   = 0L;
    ajuint nodetype = 0;
    
    AjPBtMem arrays = NULL;
    ajulong *parray = NULL;

    AjPList list   = NULL;
    
    unsigned char *buf = NULL;    

    ajulong hitlen = 0UL;

    ajDebug("btreeKeyFullSearchId key '%S'\n", key);

    list   = ajListNew();

    root = btreePricacheLocate(cache, 0L);
    page = root;
    
    buf = root->buf;
    GBT_NODETYPE(buf,&nodetype);
    
    if(cache->plevel)
    {
        arrays = btreeAllocPriArray(cache);
        parray = arrays->parray;

        /* down to the leftmost leaf node */

	while(nodetype != BT_LEAF)
	{
	    btreeGetPointers(cache,buf,&parray);
	    page = btreePricacheRead(cache, parray[0]);
	    buf = page->buf;
	    GBT_NODETYPE(buf,&nodetype);
	    page->dirty = BT_CLEAN;
	}

        btreeDeallocPriArray(cache,arrays);
    }

    right = 99L;

    /* read left-to-right through all leaf nodes */

    while(right)
    {
	btreeIdleafFetch(cache,page,list);

	while(ajListPop(list,(void **)&id))
	{
	    if(ajStrMatchWildS(id->id,key))
            {
                hit = ajBtreeHitNewId(id);
		ajListPushAppend(hitlist,(void *)hit);
                hit = NULL;
            }
            ajBtreeIdDel(&id);
	}

        if(hitlen != ajListGetLength(hitlist))
        {
            ajDebug("Page %Lu found %Lu\n", page->pagepos,
                    ajListGetLength(hitlist) - hitlen);
            hitlen = ajListGetLength(hitlist);
        }

	GBT_RIGHT(buf,&right);

	if(right)
	{
	    page = btreePricacheRead(cache,right);
	    buf = page->buf;
	}
    }
    

    ajListSortTwoUnique(hitlist, &btreeHitDbnoCompare, &btreeHitOffsetCompare,
                        &btreeHitDelFromList);

    ajListFree(&list);

    return;
}




/* @funcstatic btreeKeyFullSearchHitref ***************************************
**
** Wildcard retrieval of id/acc/sv entries. Whole index scan. Only used for
** wildcard searches with keys beginning with '?' or '*'
**
** @param [u] cache [AjPBtcache] cache
** @param [r] key [const AjPStr] Wildcard key
** @param [u] hitlist [AjPList] list of matching AjPBtHitrefs
**
** @return [void]
**
** @release 3.0.0
** @@
******************************************************************************/

static void btreeKeyFullSearchHitref(AjPBtcache cache, const AjPStr key,
                                     AjPList hitlist)
{
    AjPBtId  id    = NULL;
    AjPBtHitref hitref = NULL;
    AjPBtpage root = NULL;
    AjPBtpage page = NULL;
    ajulong right   = 0L;
    ajuint nodetype = 0;
    
    AjPBtMem arrays = NULL;
    ajulong *parray = NULL;

    AjPList list   = NULL;
    
    unsigned char *buf = NULL;    

    ajulong hitlen = 0UL;

    ajDebug("btreeKeyFullSearchId key '%S'\n", key);

    list   = ajListNew();

    root = btreePricacheLocate(cache, 0L);
    page = root;
    
    buf = root->buf;
    GBT_NODETYPE(buf,&nodetype);
    
    if(cache->plevel)
    {
        arrays = btreeAllocPriArray(cache);
        parray = arrays->parray;

        /* down to the leftmost leaf node */

	while(nodetype != BT_LEAF)
	{
	    btreeGetPointers(cache,buf,&parray);
	    page = btreePricacheRead(cache, parray[0]);
	    buf = page->buf;
	    GBT_NODETYPE(buf,&nodetype);
	    page->dirty = BT_CLEAN;
	}

        btreeDeallocPriArray(cache,arrays);
    }

    right = 99L;

    /* read left-to-right through all leaf nodes */

    while(right)
    {
	btreeIdleafFetch(cache,page,list);

	while(ajListPop(list,(void **)&id))
	{
	    if(ajStrMatchWildS(id->id,key))
            {
                hitref = ajBtreeHitrefNewId(id);
		ajListPushAppend(hitlist,(void *)hitref);
                hitref = NULL;
            }
            ajBtreeIdDel(&id);
	}

        if(hitlen != ajListGetLength(hitlist))
        {
            ajDebug("Page %Lu found %Lu\n", page->pagepos,
                    ajListGetLength(hitlist) - hitlen);
            hitlen = ajListGetLength(hitlist);
        }

	GBT_RIGHT(buf,&right);

	if(right)
	{
	    page = btreePricacheRead(cache,right);
	    buf = page->buf;
	}
    }
    

    ajListSortTwoUnique(hitlist,
                        &btreeHitrefDbnoCompare, &btreeHitrefOffsetCompare,
                        &btreeHitrefDelFromList);

    ajListFree(&list);

    return;
}




/* @func ajBtreeReplaceId *****************************************************
**
** Replace an ID structure in a leaf node given a key
**
** @param [u] cache [AjPBtcache] cache
** @param [r] rid [const AjPBtId] replacement id object
**
** @return [AjBool] true if success
**
** @release 3.0.0
** @@
******************************************************************************/

AjBool ajBtreeReplaceId(AjPBtcache cache, const AjPBtId rid)
{
    AjPBtpage page   = NULL;
    AjPIdbucket bucket = NULL;
    AjPBtId   id     = NULL;
    const AjPStr key = NULL;
    
    unsigned char *buf = NULL;

    ajuint nentries = 0;
    
    ajuint i;
    ajuint iref;

    ajulong blockno = 0L;
    AjBool found   = ajFalse;


    key = rid->id;

    page = btreeIdentFind(cache,key);
    buf = page->buf;

    blockno = btreeGetBlockS(cache,buf,key);
    bucket = btreeReadIdbucket(cache,blockno);
    
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

        if(cache->refcount)
        {
            for(iref=0; iref < cache->refcount; iref++)
                id->refoffsets[iref] = rid->refoffsets[iref];
        }
        
	btreeWriteIdbucket(cache,bucket,blockno);
    }

    btreeIdbucketDel(&bucket);

    return ajTrue;
}




/* @func ajBtreeReadEntriesC **************************************************
**
** Read B+ tree entries from file
**
** @param [r] filename [const char*] file name
** @param [r] indexdir [const char*] index file directory
** @param [r] directory [const char*] file directory
** @param [w] seqfiles [AjPStr**] sequence file names
** @param [w] reffiles [AjPStr***] reference file names (if any)
** @param [w] refcount [ajuint*] Number of reference file(s) per entry
**
** @return [ajuint] number of entries
**
** @release 6.5.0
** @@
******************************************************************************/

ajuint ajBtreeReadEntriesC(const char *filename, const char *indexdir,
                           const char *directory,
                           AjPStr **seqfiles, AjPStr ***reffiles,
                           ajuint *refcount)
{
    AjPStr line = NULL;
    AjPStr fn   = NULL;
    
    AjPList list;
    AjPList *reflist;

    AjPStr seqname = NULL;
    AjPStr refname = NULL;
    AjPStr tseqname = NULL;
    AjPStr trefname = NULL;
    
    AjPFile inf   = NULL;
    char p;
    ajulong entries = 0UL;
    ajuint iref;

    AjPStrTok handle = NULL;

    *refcount = 0;

    line    = ajStrNew();
    list    = ajListNew();

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
	    *refcount = 1;
        
	if(ajStrPrefixC(line,"Reference "))
        {
	    ajFmtScanS(line,"%S%d",&trefname, refcount);
            --*refcount;
        }
        
	break;
    }
    

    if(!*refcount)
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
        AJCNEW(reflist, *refcount);
        for(iref=0; iref < *refcount; iref++)
            reflist[iref] = ajListNew();

	while(ajReadlineTrim(inf, &line))
	{
	    seqname = ajStrNew();
	    refname = ajStrNew();
            handle = ajStrTokenNewC(line, " \t");
            ajStrTokenNextParse(&handle, &tseqname);
	    ajFmtPrintS(&seqname,"%s%s%S",directory,SLASH_STRING,tseqname);
	    ajListPushAppend(list,(void *)seqname);

            for(iref=0; iref < *refcount; iref++)
            {
                ajStrTokenNextParse(&handle, &trefname);
                ajFmtPrintS(&refname,"%s%s%S",directory,SLASH_STRING,trefname);
                ajListPushAppend(reflist[iref],(void *)refname);
            }
        }

	ajListToarray(list,(void ***)&(*seqfiles));

        for(iref=0; iref < *refcount; iref++)
            ajListToarray(reflist[iref],(void ***)&(*reffiles[iref]));

	entries = ajListGetLength(list);
    }
    
    
    ajStrTokenDel(&handle);
    ajListFree(&list);

    if(*refcount)
    {
        for(iref=0; iref < *refcount; iref++)
            ajListFree(&reflist[iref]);

        AJFREE(reflist);
    }
    
    ajStrDel(&line);
    ajStrDel(&fn);

    ajStrDel(&tseqname);
    ajStrDel(&trefname);
    

    ajFileClose(&inf);

    return (ajuint) entries;
}




/* @func ajBtreeReadEntriesS **************************************************
**
** Read B+ tree entries from file
**
** @param [r] filename [const AjPStr] file name
** @param [r] indexdir [const AjPStr] index file directory
** @param [r] directory [const AjPStr] file directory
** @param [w] seqfiles [AjPStr**] sequence file names
** @param [w] reffiles [AjPStr***] reference file names (if any)
** @param [w] refcount [ajuint*] Number of reference file(s) per entry
**
** @return [ajuint] number of entries
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ajBtreeReadEntriesS(const AjPStr filename, const AjPStr indexdir,
                           const AjPStr directory,
                           AjPStr **seqfiles, AjPStr ***reffiles,
                           ajuint *refcount)
{
    AjPStr line = NULL;
    AjPStr fn   = NULL;
    
    AjPList list;
    AjPList *reflist;

    AjPStr seqname = NULL;
    AjPStr refname = NULL;
    AjPStr tseqname = NULL;
    AjPStr trefname = NULL;
    
    AjPFile inf   = NULL;
    char p;
    ajulong entries = 0UL;
    ajuint iref;

    AjPStrTok handle = NULL;

    *refcount = 0;

    line    = ajStrNew();
    list    = ajListNew();

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
	    *refcount = 1;
        
	if(ajStrPrefixC(line,"Reference "))
        {
	    ajFmtScanS(line,"%S%d",&trefname, refcount);
            --*refcount;
        }
    
	break;
    }
    

    if(!*refcount)
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
        AJCNEW(reflist, *refcount);
        for(iref=0; iref < *refcount; iref++)
            reflist[iref] = ajListNew();

	while(ajReadlineTrim(inf, &line))
	{
	    seqname = ajStrNew();
	    refname = ajStrNew();
            ajStrTokenAssignC(&handle, line, " \t");
            ajStrTokenNextParse(&handle, &tseqname);
	    ajFmtPrintS(&seqname,"%S%s%S",directory,SLASH_STRING,tseqname);
	    ajListPushAppend(list,(void *)seqname);

            for(iref=0; iref < *refcount; iref++)
            {
                ajStrTokenNextParse(&handle, &trefname);
                ajFmtPrintS(&refname,"%S%s%S",directory,SLASH_STRING,trefname);
                ajListPushAppend(reflist[iref],(void *)refname);
            }
        }

	ajListToarray(list,(void ***)&(*seqfiles));

        if(reffiles)
        {
            AJCNEW0(*reffiles, *refcount);
            for(iref=0; iref < *refcount; iref++)
                ajListToarray(reflist[iref],(void ***)&(*reffiles[iref]));
        }

	entries = ajListGetLength(list);
    }
    
    ajStrTokenDel(&handle);
    ajListFree(&list);

    if(*refcount)
    {
        for(iref=0; iref < *refcount; iref++)
            ajListFree(&reflist[iref]);

        AJFREE(reflist);
    }
    
    ajStrDel(&line);
    ajStrDel(&fn);

    ajStrDel(&tseqname);
    ajStrDel(&trefname);
    

    ajFileClose(&inf);

    return (ajuint) entries;
}




#if AJINDEX_STATIC
/* #funcstatic btreeKeyListDuplicates *****************************************
**
** Write B+ tree duplicate entries matching key to a list
**
** #param [u] cache [AjPBtcache] cache
** #param [r] key [const AjPStr] key
**
** #return [AjPList] list of matching AjPBtIds or NULL
**
** #release 6.5.0
** ##
******************************************************************************/
/*
static AjPList btreeKeyListDuplicates(AjPBtcache cache, const AjPStr key)
{
    AjPList list = NULL;
    AjPBtId id   = NULL;
    ajuint i;
    ajuint dups;
    
    AjPStr dupkey = NULL;
    AjPStr okey   = NULL;

    if(!(id = btreeIdentQueryId(cache,key)))
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
	    id = btreeIdentQueryId(cache,dupkey);

	    if(!id)
		ajFatal("DupFromKey: Id not found\n");

	    ajListPushAppend(list,(void *)id);
	}
    }

    ajStrDel(&okey);
    ajStrDel(&dupkey);
    

    return list;
}
*/
#endif




/* @funcstatic btreePribucketNew **********************************************
**
** Construct a primary keyword bucket object
**
** @param [r] n [ajuint] Number of IDs
**
** @return [AjPPribucket] initialised disc block cache structure
**
** @release 3.0.0
** @@
******************************************************************************/

static AjPPribucket btreePribucketNew(ajuint n)
{
    AjPPribucket bucket = NULL;
    ajuint i;

    /*ajDebug("In btreePribucketNew %u statsave: %u empty: %u\n",
      n, statSavePribucketNext, statSavePribucketEmptyNext);*/
    

    if(n)
    {
        if(statSavePribucketNext)
        {
            bucket = statSavePribucket[--statSavePribucketNext];
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
        if(statSavePribucketEmptyNext)
            bucket = statSavePribucketEmpty[--statSavePribucketEmptyNext];
        else
            AJNEW0(bucket);
    }
        
    bucket->NodeType = BT_PRIBUCKET;
    bucket->Nentries = n;
    bucket->Overflow = 0L;
    
    return bucket;
}




/* @funcstatic btreePribucketDel **********************************************
**
** Delete a keyword primary bucket object
**
** @param [w] thys [AjPPribucket*] bucket
**
** @return [void]
**
** @release 3.0.0
** @@
******************************************************************************/

static void btreePribucketDel(AjPPribucket *thys)
{
    AjPPribucket pthis = NULL;
    ajuint newmax;
    
    if(!thys || !*thys)
	return;

    pthis = *thys;

    /*ajDebug("In btreePribucketDel maxentries: %u savepribucket %u empty %u\n",
            pthis->Maxentries, statSavePribucketNext,
            statSavePribucketEmptyNext);*/

    if(!statSavePribucket)
    {
        statSavePribucketMax=2048;
        statSavePribucketNext=0;
        AJCNEW0(statSavePribucket,statSavePribucketMax);
    }
    
    if(!statSavePribucketEmpty)
    {
        statSavePribucketEmptyMax=2048;
        statSavePribucketEmptyNext=0;
        AJCNEW0(statSavePribucketEmpty,statSavePribucketEmptyMax);
    }
    
    if(pthis->Maxentries)
    {
        if(statSavePribucketNext >= statSavePribucketMax)
        {
            newmax = statSavePribucketMax + statSavePribucketMax;
            AJCRESIZE0(statSavePribucket,statSavePribucketMax,newmax);
            statSavePribucketMax = newmax;
        }
        
        statSavePribucket[statSavePribucketNext++] = pthis;
    }
    else
    {
        if(statSavePribucketEmptyNext >= statSavePribucketEmptyMax)
        {
            newmax = statSavePribucketEmptyMax + statSavePribucketEmptyMax;
            AJCRESIZE0(statSavePribucketEmpty,statSavePribucketEmptyMax,newmax);
            statSavePribucketEmptyMax = newmax;
        }
        statSavePribucketEmpty[statSavePribucketEmptyNext++] = pthis;
    }

    *thys = NULL;

    return;
}




/* @funcstatic btreePribucketFree *********************************************
**
** Delete a keyword primary bucket object
**
** @param [w] thys [AjPPribucket*] bucket
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void btreePribucketFree(AjPPribucket *thys)
{
    AjPPribucket pthis = NULL;
    ajuint n;
    ajuint i;

    if(!thys || !*thys)
	return;

    pthis = *thys;
    n = pthis->Maxentries;

    /*ajDebug("In btreePribucketFree %u\n",pthis->Maxentries);*/

    for(i=0;i<n;++i)
	btreePriFree(&pthis->codes[i]);
    
    AJFREE(pthis->keylen);
    AJFREE(pthis->codes);
    
    AJFREE(pthis);

    *thys = NULL;

    return;
}




/* @func ajBtreePriNew ********************************************************
**
** Constructor for index bucket keyword information
**
**
** @return [AjPBtPri] Index ID object
**
** @release 3.0.0
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




/* @func ajBtreePriDel ********************************************************
**
** Destructor for keyword index primary bucket information
**
** @param [w] thys [AjPBtPri*] index keyword primary object
**
** @return [void]
**
** @release 3.0.0
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




/* @funcstatic btreePriFree ***************************************************
**
** Destructor for index primary bucket information
**
** @param [w] thys [AjPBtPri*] index keyword primary object
**
** @return [void]
**
** @release 6.4.0
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




/* @funcstatic btreePribucketIdlist *******************************************
**
** Copies all primary keys into a list
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
** @param [u] idlist [AjPList] list to hold keys
**
** @return [ajulong] Overflow
**
** @release 6.4.0
** @@
******************************************************************************/

static ajulong btreePribucketIdlist(AjPBtcache cache, ajulong pagepos,
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
    
    /* ajDebug("In btreePribucketIdlist\n"); */
    
    if(!pagepos)
	ajFatal("PribucketIdlist: cannot read bucket from root page cache %S",
                cache->filename);

    page  = btreePricacheRead(cache,pagepos);
    lpage = page;
    dirtysave = lpage->dirty;
    lpage->dirty = BT_LOCK;
    lpage->lockfor = 1211;

    buf = lpage->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);
    if(nodetype != BT_PRIBUCKET)
	ajFatal("PribucketIdlist: NodeType mismatch. "
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

/*
//        if((codeptr-buf+1) + len > cache->pagesize)	/# overflow #/
//	{
//	    /# ajDebug("PribucketRead: Overflow\n"); #/
//	    page  = btreePricacheRead(cache,overflow);
//	    buf = page->buf;
//	    GBT_BUCKNODETYPE(buf,&nodetype);
//	    if(nodetype != BT_PRIBUCKET)
//		ajFatal("PribucketIdlist: NodeType mismatch. Not primary "
//			"bucket (%u) cache %S",
//                        nodetype, cache->filename);
//	    GBT_BUCKOVERFLOW(buf,&overflow);
//	    /# overflow bucket ids start at the keylen position #/
//	    codeptr = PBT_BUCKKEYLEN(buf);
//	}
*/

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




/* @funcstatic btreePribucketRead *********************************************
**
** Constructor for keyword index primary bucket given a disc page number
** Creates one empty key slot for possible addition
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
**
** @return [AjPPribucket] bucket
**
** @release 6.5.0
** @@
******************************************************************************/

static AjPPribucket btreePribucketRead(AjPBtcache cache, ajulong pagepos)
{
    AjPPribucket bucket = NULL;
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
    
    /* ajDebug("In btreePribucketRead\n"); */
    
    if(!pagepos)
	ajFatal("PribucketRead: cannot read bucket from root page cache %S",
                cache->filename);

    page  = btreePricacheRead(cache,pagepos);
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

    if(nentries > cache->pnperbucket)
	ajFatal("PriReadBucket: Bucket too full page: %Lu "
                "entries: %u max: %u page %Lu cache %S",
                pagepos, nentries, cache->pnperbucket,
                pagepos, cache->filename);
    

    GBT_BUCKOVERFLOW(buf,&overflow);

    bucket = btreePribucketNew(cache->pnperbucket);
    bucket->Nentries = nentries;
    
    kptr  = PBT_BUCKKEYLEN(buf);
    codeptr = kptr + (nentries * sizeof(ajuint));

    for(i=0;i<nentries;++i)
    {
	BT_GETAJUINT(kptr,&len);

/*
//      if((codeptr-buf+1) + len > cache->pagesize)	/# overflow #/
//	{
//#if AJINDEX_DEBUG
//	    ajDebug("PriReadBucket: Overflow\n");
//#endif
//	    page  = btreePricacheRead(cache,overflow);
//	    buf = page->buf;
//	    GBT_BUCKNODETYPE(buf,&nodetype);
//
//	    if(nodetype != BT_PRIBUCKET)
//		ajFatal("PriReadBucket: NodeType mismatch. Not primary "
//			"bucket (%u) page %Lu", nodetype, pagepos);
//
//	    GBT_BUCKOVERFLOW(buf,&overflow);
//	    /# overflow bucket ids start at the keylen position #/
//	    codeptr = PBT_BUCKKEYLEN(buf);
//	}
*/

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




/* @funcstatic btreePribucketSort *********************************************
**
** Sorts IDs in a primary keyword bucket
**
** @param [u] thys [AjPPribucket] cache
**
** @return [void]
** @@
******************************************************************************/

static void btreePribucketSort(AjPPribucket thys)
{
    qsort(thys->codes, thys->Nentries, sizeof(AjPBtPri), btreeKeywordCompare);

    return;
}




/* @funcstatic btreeWritePribucket ********************************************
**
** Write primary keyword index bucket object to the cache given a disc page
** number
**
** @param [u] cache [AjPBtcache] cache
** @param [r] bucket [const AjPPribucket] bucket
** @param [r] pagepos [ajulong] page number
**
** @return [void]
**
** @release 3.0.0
** @@
******************************************************************************/

static void btreeWritePribucket(AjPBtcache cache, const AjPPribucket bucket,
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

    AjPBtPri pri    = NULL;
    ajuint nentries = 0;
    ajulong overflow = 0L;

    unsigned char *keyptr = NULL;
    unsigned char *lptr   = NULL;

/*    ajulong pno = 0L;*/


    /* ajDebug("In btreeWritePribucket\n"); */

    if(pagepos == cache->totsize)	/* Create a new page */
    {
	/* pno = pagepos; */
	page = btreePricacheBucketnew(cache);
	buf = page->buf;
        overflow = 0L;
    }
    else
    {
	page = btreePricacheRead(cache,pagepos);
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
        /* drop check - tested before write - pagesize dependency */
/*
//	if((ajuint)((keyptr-lbuf+1)+sizeof(ajuint)) > cache->pagesize)
//	    ajFatal("WritePribucket: Bucket cannot hold more than %u keys",
//		    i-1);
*/

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

/*
//	if((lptr-buf+1)+len > cache->pagesize) /# overflow #/
//	{
//    	    /#ajDebug("WritePribucket: Overflow\n");#/
//
//	    if(!overflow)		/# No overflow buckets yet #/
//	    {
//		pno = cache->totsize;
//                lv = pno;
//		SBT_BUCKOVERFLOW(buf,lv);
//		page = btreePricacheBucketnew(cache);
//                buf = page->buf;
//		v = BT_PRIBUCKET;
//		SBT_BUCKNODETYPE(buf,v);
//	    }
//	    else
//	    {
//		page = btreePricacheRead(cache,overflow);
//		buf  = page->buf;
//		GBT_BUCKOVERFLOW(buf,&overflow);
//	    }
//
//	    page->dirty = BT_DIRTY;
//
//	    lptr = PBT_BUCKKEYLEN(buf);	    
//	}
*/	

	sprintf((char *)lptr,"%s",ajStrGetPtr(pri->keyword));
	lptr += (ajStrGetLen(pri->keyword) + 1);
        lv = pri->treeblock;
	BT_SETAJULONG(lptr,lv);
	lptr += sizeof(ajulong);
    }

    lv = 0L;
    SBT_BUCKOVERFLOW(buf,lv);

    lpage->dirty = BT_DIRTY;

    return;
}




/* @funcstatic btreeWritePribucketEmpty ***************************************
**
** Write empty primary keyword index bucket object to the cache given a
** disc page number
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void btreeWritePribucketEmpty(AjPBtcache cache, ajulong pagepos)
{
    AjPBtpage page  = NULL;
    AjPBtpage lpage = NULL;

    unsigned char *buf  = NULL;

    ajuint  v   = 0;
    ajulong lv  = 0L;

    ajulong overflow = 0L;

    /* ajDebug("In btreeWritePribucketEmpty\n"); */

    if(pagepos == cache->totsize)	/* Create a new page */
    {
	page = btreePricacheBucketnew(cache);
	buf = page->buf;
	overflow = 0L;
	lv = overflow;
	SBT_BUCKOVERFLOW(buf,lv);
    }
    else
    {
	page = btreePricacheRead(cache,pagepos);
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

    lpage->dirty = BT_DIRTY;    /* clear the lock */

    return;
}




/* @funcstatic btreePribucketAdd **********************************************
**
** Add a keyword ID to a primary bucket
** Only called if there is room in the bucket
**
** The primary bucket entry for the keyword stores the root page of
** the secondary tree of ids
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number of bucket
** @param [r] keyword [const AjPStr] Keyword
** @param [r] id      [const AjPStr] Identifier
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void btreePribucketAdd(AjPBtcache cache, ajulong pagepos,
                              const AjPStr keyword, const AjPStr id)
{
    unsigned char *buf  = NULL;
    unsigned char *kptr = NULL;
    unsigned char *src  = NULL;
    unsigned char *dest = NULL;

/*    unsigned char *lastptr = NULL;*/
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
    ajulong treeblock  = 0L;

    AjPBtpage page = NULL;
    static ajuint calls = 0;
/*    static ajuint overflowcalls=0;*/

    calls++;

    treeblock = 0;

    /* secondary tree does not exist, can use secbucket */

    secrootpage = cache->totsize;

    btreeWriteSecbucketEmpty(cache,secrootpage);
    btreeSecbucketAdd(cache,secrootpage,id);

    treeblock = secrootpage;
    cache->secrootblock = secrootpage;
    cache->slevel = 0;

    /* add to the Pribucket page */

    page = btreePricacheRead(cache,pagepos);
    buf  = page->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);

    if(nodetype != BT_PRIBUCKET)
        ajFatal("Wrong nodetype in PribucketAdd cache %S",
                cache->filename);

    GBT_BUCKNENTRIES(buf,&nentries);

    if(nentries >= cache->pnperbucket)
        ajFatal("Bucket too full in PribucketAdd page: %Lu "
                "entries: %u max: %u cache %S",
                pagepos, nentries, cache->pnperbucket,
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
    idlen   = MAJSTRGETLEN(keyword);

/*
//    lastptr = endptr + sizeof(ajulong) + idlen + 1;
//
//    if((ajuint) (lastptr - buf) >= cache->pripagesize)
//    {
//        overflowcalls++;
//
//        ajWarn("\nOverflow in PribucketAdd nentries:%u fails %u/%u cache %S",
//               nentries, overflowcalls, calls, cache->filename);
//
//        btreePribucketAddFull(cache,pagepos,keyword, id);
//        return;
//    }
*/    

    dest = src + sizeof(ajuint);
    memmove((void *)dest, (void *)src, sum);

    v = BT_BUCKPRILEN(keyword);
    BT_SETAJUINT(src,v);

    endptr += sizeof(ajuint) + 1;
    strcpy((char *)endptr,MAJSTRGETPTR(keyword));

    endptr += idlen + 1;
    lv = treeblock;
    BT_SETAJULONG(endptr,lv);
    endptr += sizeof(ajulong);

    v = nentries + 1;
    SBT_BUCKNENTRIES(buf,v);

    page->dirty = BT_DIRTY;

    return;
}




#if 0
/* #funcstatic btreePribucketAddFull ******************************************
**
** Add a keyword ID to a full primary bucket
** by making space in a new bucket
**
** #param [u] cache [AjPBtcache] cache
** #param [r] pagepos [ajulong] page number of bucket
** #param [r] keyword [const AjPStr] Keyword
** #param [r] id      [const AjPStr] Identifier
**
** #return [void]
**
** #release 6.5.0
** ##
******************************************************************************/
/*
//static void btreePribucketAddFull(AjPBtcache cache, ajulong pagepos,
//                                  const AjPStr keyword, const AjPStr id)
//{
//    AjPPribucket bucket  = NULL;
//    AjPBtPri     dest    = NULL;
//    AjPBtpage    page    = NULL;
//    
//    ajuint nentries;
//    ajulong secrootpage = 0L;
//    ajulong right       = 0L;
//
//    unsigned char *buf;
//    
//    /# ajDebug("In btreePribucketAddFull\n"); #/
//
//    bucket   = btreePribucketRead(cache,pagepos);
//    nentries = bucket->Nentries;
//
//
//    /# Reading a bucket always gives one extra ID position #/
//    bucket->codes[nentries] = ajBtreePriNew();
//    dest = bucket->codes[nentries];
//
//    ajStrAssignS(&dest->keyword,keyword);
//
//    /# Need to add code here to access secondary tree #/
//    dest->treeblock = 0L;
//
//
//    /# See if secondary tree exists, if not then create it #/
//    if(!dest->treeblock)
//    {
//	secrootpage = cache->totsize;
//
//	btreeSecrootCreate(cache,secrootpage);
//	cache->secrootblock = secrootpage;
//	page = btreeSeccacheWrite(cache,secrootpage);
//	page->dirty = BT_LOCK;
//        page->lockfor = 1261;
//        if(btreeDoRootSync)
//            btreeCacheRootSync(cache,secrootpage);
//	dest->treeblock = secrootpage;
//	buf = page->buf;
//	cache->slevel = 0;
//
//	/# ajDebug("Created 2ry tree at block %u\n",(ajuint)secrootpage); #/
//    }
//    else
//    {
//	cache->secrootblock = dest->treeblock;
//	page = btreeSeccacheWrite(cache,cache->secrootblock);
//	page->dirty = BT_LOCK;
//        page->lockfor = 1262;
//	buf = page->buf;
//	GBT_RIGHT(buf, &right);
//	cache->slevel = (ajuint) right;
//    }
//
//    btreeKeyidInsert(cache, id);
//
//    right = (ajulong) cache->slevel;
//    
//    SBT_RIGHT(buf, right);
//    page->dirty = BT_DIRTY;
//    
//    ++bucket->Nentries;
//
//    btreeWritePribucket(cache,bucket,pagepos);
//
//    btreePribucketDel(&bucket);
//    
//    return;
//}
*/
#endif




/* @funcstatic btreeNumInPribucket ********************************************
**
** Return number of entries in a primary keyword bucket
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
**
** @return [ajuint] Number of entries in bucket
**
** @release 3.0.0
** @@
******************************************************************************/

static ajuint btreeNumInPribucket(AjPBtcache cache, ajulong pagepos)
{
    AjPBtpage page     = NULL;
    unsigned char *buf = NULL;

    ajuint  nodetype = 0;
    ajuint nentries = 0;
    
    /* ajDebug("In btreeNumInPribucket\n"); */
    
    if(!pagepos)
	ajFatal("NumInPribucket: Attempt to read bucket from root page\n");

    page  = btreePricacheRead(cache,pagepos);

    buf = page->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);

    if(nodetype != BT_PRIBUCKET)
	ajFatal("PriReadBucket: NodeType mismatch. Not primary bucket (%u)",
		nodetype);
    
    GBT_BUCKNENTRIES(buf,&nentries);

    return nentries;
}




/* @funcstatic btreeKeywordCompare ********************************************
**
** Comparison function for ajListSort
**
** @param [r] a [const void*] ID 1
** @param [r] b [const void*] ID 2
**
** @return [ajint] 0 = bases match
**
** @release 3.0.0
** @@
******************************************************************************/

static ajint btreeKeywordCompare(const void *a, const void *b)
{
    return MAJSTRCMPS((*(AjPBtPri const *)a)->keyword,
                      (*(AjPBtPri const *)b)->keyword);
}




/* @funcstatic btreePribucketsReorder *****************************************
**
** Re-order primary keyword leaf buckets
** Must only be called if one of the buckets is full
**
** @param [u] cache [AjPBtcache] cache
** @param [u] leaf [AjPBtpage] leaf page
**
** @return [AjBool] true if reorder was successful i.e. leaf not full
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool btreePribucketsReorder(AjPBtcache cache, AjPBtpage leaf)
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
    AjPPribucket cbucket = NULL;
    AjPBtPri cid         = NULL;

    ajuint iold = 0;
    
#if AJINDEX_DEBUG
    ajDebug("btreePribucketsReorder %Lu\n", leaf->pagepos);
#endif

    /* ajDebug("In btreePribucketsReorder\n"); */

    dirtysave = leaf->dirty;

    leaf->dirty = BT_LOCK;
    leaf->lockfor = 1271;
    lbuf = leaf->buf;

    GBT_NODETYPE(lbuf,&nodetype);

    order = cache->porder;
    
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
	ajFatal("PribucketsReorder: Attempt to reorder empty leaf");

    for(i=0;i<nkeys;++i)
	totalkeys += btreeNumInPribucket(cache,ptrs[i]);
    totalkeys += btreeNumInPribucket(cache,ptrs[i]);

    btreeBucketCalc(totalkeys, keylimit, cache->pnperbucket,
                    &bucketlimit, &maxnperbucket);

    if(bucketlimit >= order)
    {
        btreeDeallocPriArray(cache,arrays1);
        btreeDeallocPriArray(cache,arrays2);
	leaf->dirty = dirtysave;

	return ajFalse;
    }
    
    ++statCallPribucketsReorder;

    /* Read IDs from all buckets and push to list and sort (increasing kw) */
    idlist  = ajListNew();
    
    for(i=0;i<keylimit;++i)
        overflows[i] = btreePribucketIdlist(cache,ptrs[i],idlist);

    ajListSort(idlist, &btreeKeywordCompare);

    cbucket = btreePribucketNew(cache->pnperbucket);
    
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
	btreeWritePribucket(cache,cbucket,newptrs[i]);
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
    btreeWritePribucket(cache,cbucket,newptrs[i]);

    btreePribucketDel(&cbucket);

#if AJINDEX_DEBUG
    if(bucketlimit <= keylimit)
        ajDebug("btreePribucketsReorder '%S' %u -> %u",
                cache->filename, keylimit, bucketlimit);
#endif

    for(i = bucketlimit + 1; i <= keylimit; i++)
    {
        btreePripageSetfree(cache, ptrs[i]);
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




/* @func ajBtreeKeyIndex ******************************************************
**
** Insert a keyword structure into the tree.
**
** This is the primary function for adding a keyword and identifier to
** a secondary keyword index
**
** @param [u] cache   [AjPBtcache] cache
** @param [r] keyword [const AjPStr] keyword
** @param [r] id      [const AjPStr] entry identifier
**
** @return [AjBool] True if keyword and ID combination was inserted
**                  False if keyword exists already for ID
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ajBtreeKeyIndex(AjPBtcache cache, const AjPStr keyword, const AjPStr id)
{
    AjPBtpage spage   = NULL;
    AjPBtpage page    = NULL;
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
    /*ajDebug("ajBtreeKeyIndex '%S' '%S'\n", pri->keyword, pri->id);*/
#endif

    if(!MAJSTRGETLEN(keyword))
	return ajFalse;

    ajStrAssignS(&indexKeyword, keyword);
    ajStrAssignS(&indexId, id);
    ajStrFmtQuery(&indexId);
    ajStrFmtQuery(&indexKeyword);

    if(MAJSTRGETLEN(indexKeyword) > cache->keylimit)
        ajStrTruncateLen(&indexKeyword, cache->keylimit);

    if(MAJSTRGETLEN(indexId) > cache->idlimit)
        ajStrTruncateLen(&indexId, cache->idlimit);

    /*
    ** Only insert a primary key if that key doesn't exist
    **
    ** We always have a secondary identifier if there is a match
    ** (for Identifiers there may be no secondary data so their code differs)
    */

    if(btreeKeyFind(cache, indexKeyword, &treeblock))
    {
        /* we have the keyword in the index */

	cache->secrootblock = treeblock;
	page = btreeSeccacheWrite(cache,cache->secrootblock);
        savedirty = page->dirty;
	page->dirty = BT_LOCK;
        page->lockfor = 1281;
	buf = page->buf;
	GBT_RIGHT(buf,&right);
	cache->slevel = (ajuint) right;

	if(btreeKeyidInsert(cache,indexId))
        {
            ++cache->countall;
            GBT_NODETYPE(buf,&nodetype);
            if(nodetype != BT_SECBUCKET)
            {
                right = (ajulong) cache->slevel;
                SBT_RIGHT(buf,right);
                page->dirty = BT_DIRTY;
            }
        }
        else
        {
            newid = ajFalse;
            page->dirty = savedirty;
        }
        
	return newid;
    }

    /* this is a new key */

    spage = btreeIdentFind(cache,indexKeyword);
    buf = spage->buf;

    GBT_NKEYS(buf,&nkeys);
    GBT_NODETYPE(buf,&nodetype);
    
    if(!nkeys)
    {
        /* new primary leaf node with no keys yet */

        /* create two buckets, add to the right-hand one */

	lblockno = cache->totsize;
	btreeWritePribucketEmpty(cache,lblockno);

	rblockno = cache->totsize;
	btreeWritePribucketEmpty(cache,rblockno);	

	btreeWriteNodeSingle(cache,spage,indexKeyword,lblockno,rblockno);

	GBT_BLOCKNUMBER(buf,&blockno);

	if(!blockno)
        {
	    spage->dirty = BT_LOCK; /* root page */
            spage->lockfor = 1282;
        }

	btreePribucketAdd(cache,rblockno,indexKeyword, indexId);
	++cache->countunique;
	++cache->countall;

	return ajTrue;
    }
    
    blockno = btreeGetBlockS(cache,buf,indexKeyword);
    if(nodetype != BT_ROOT)
	if((shift = btreeKeyInsertShift(cache,&spage,indexKeyword)))
	    blockno = shift;

    buf = spage->buf;

    n = btreeNumInPribucket(cache,blockno);

    if(n == cache->pnperbucket)
    {
	if(btreePribucketsReorder(cache,spage))
	{
            blockno = btreeGetBlockS(cache,buf,indexKeyword);
	}
	else
	{
	    btreeKeySplitleaf(cache,spage);
	    spage = btreeIdentFind(cache,indexKeyword);
	    buf = spage->buf;

            blockno = btreeGetBlockS(cache,buf,indexKeyword);
	}
    }

    btreePribucketAdd(cache, blockno, indexKeyword, indexId);
    ++cache->countunique;
    ++cache->countall;

    return ajTrue;
}




/* @funcstatic btreeKeyFind ***************************************************
**
** Get secondary root block matching a keyword
**
** @param [u] cache [AjPBtcache] cache
** @param [r] key [const AjPStr] key
** @param [w] treeblock [ajulong*] Tree block number for id
**
** @return [AjBool] ajTrue if found
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool btreeKeyFind(AjPBtcache cache, const AjPStr key,
                           ajulong* treeblock)
{
    AjPBtpage page      = NULL;
    
    unsigned char *buf = NULL;

    ajulong blockno = 0L;

    /*ajDebug("btreeKeyFind '%s'\n", ckey);*/

    if(!cache->countunique)
	return ajFalse;

    page = btreeIdentFind(cache,key);

    buf = page->buf;

    blockno = btreeGetBlockS(cache, buf, key);

    return btreePribucketFindId(cache,blockno,key,treeblock);
}




/* @func ajBtreeKeyFindLen ****************************************************
**
** Get keyword structure matching a keyword truncated to maximum indexed length
**
** @param [u] cache [AjPBtcache] cache
** @param [r] key [const AjPStr] key
** @param [w] treeblock [ajulong*] Tree block number for id
**
** @return [AjBool] ajTrue if found
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajBtreeKeyFindLen(AjPBtcache cache, const AjPStr key,
                         ajulong* treeblock)
{
    AjBool ret = ajFalse;

    AjPBtpage page      = NULL;
    unsigned char *buf = NULL;
    ajulong blockno = 0L;

    AjPStr keystr = NULL;
    const AjPStr keytest = key;

#if AJINDEX_DEBUG
    ajDebug("ajBtreeKeyFindLen '%S'\n", key);
#endif

    if(!cache->countunique)
	return ajFalse;

    keystr = ajStrNewS(key);
    ajStrFmtQuery(&keystr);
    if(MAJSTRGETLEN(keystr) > cache->keylimit)
        ajStrTruncateLen(&keystr, cache->keylimit);
    keytest = keystr;

    ajDebug("ajBtreeKeyFindLen '%S'\n", keystr);

    page = btreeIdentFind(cache,keytest);

    buf = page->buf;

    blockno = btreeGetBlockS(cache, buf, keytest);

    ret = btreePribucketFindId(cache,blockno,keytest,treeblock);

    ajStrDel(&keystr);

    return ret;
}




/* @funcstatic btreePribucketFindId *******************************************
**
** Tests for an ID in a primary bucket
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
** @param [r] id [const AjPStr] id to search for
** @param [w] treeblock [ajulong*] Tree block number for id
**
** @return [AjBool] ajTrue if found
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool btreePribucketFindId(AjPBtcache cache, ajulong pagepos,
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
    
    /* ajDebug("In btreePribucketFindId\n"); */
    
    if(!pagepos)
	ajFatal("PribucketFindId: cannot read bucket from root page cache %S",
                cache->filename);

    page  = btreePricacheRead(cache,pagepos);
    lpage = page;
    dirtysave = lpage->dirty;
    lpage->dirty = BT_LOCK;
    lpage->lockfor = 1291;

    buf = lpage->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);
    if(nodetype != BT_PRIBUCKET)
	ajFatal("PribucketFindId: NodeType mismatch. "
                "Not primary bucket (%u) cache %S",
		nodetype, cache->filename);
    
    GBT_BUCKNENTRIES(buf,&nentries);
    if(nentries > cache->pnperbucket)
	ajFatal("PribucketFindId: Bucket too full  page: %Lu "
                "entries: %u max: %u cache %S",
                pagepos, nentries, cache->pnperbucket,
                cache->filename);
    

    GBT_BUCKOVERFLOW(buf,&overflow);
    
    kptr  = PBT_BUCKKEYLEN(buf);
    codeptr = kptr + (nentries * sizeof(ajuint));

    for(i=0;i<nentries;++i)
    {
	BT_GETAJUINT(kptr,&len);
        idlen = len - sizeof(ajulong) - 1;

/*
//        if((codeptr-buf+1) + len > cache->pagesize)	/# overflow #/
//	{
//	    /# ajDebug("PribucketFindId: Overflow\n"); #/
//	    page  = btreePricacheRead(cache,overflow);
//	    buf = page->buf;
//	    GBT_BUCKNODETYPE(buf,&nodetype);
//	    if(nodetype != BT_PRIBUCKET)
//		ajFatal("PribucketFindId: NodeType mismatch. "
//                        "Not primary bucket (%u) cache %S",
//                        nodetype, cache->filename);
//	    GBT_BUCKOVERFLOW(buf,&overflow);
//	    /# overflow bucket ids start at the keylen position #/
//	    codeptr = PBT_BUCKKEYLEN(buf);
//	}
*/

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




/* @funcstatic btreeKeySplitleaf **********************************************
**
** Split a keyword leaf and propagate up if necessary
**
** @param [u] cache [AjPBtcache] cache
** @param [u] spage [AjPBtpage] page
**
** @return [AjPBtpage] pointer to a parent page
**
** @release 6.5.0
** @@
******************************************************************************/

static AjPBtpage btreeKeySplitleaf(AjPBtcache cache, AjPBtpage spage)
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

    AjPPribucket cbucket  = NULL;
    
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
    ajDebug("btreeKeySplitleaf %p\n", spage->pagepos);
#endif
    ++statCallKeySplitleaf;

    /* ajDebug("In btreeKeySplitleaf\n"); */

    order = cache->porder;

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
	lpage = btreePricacheNodenew(cache);
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
    rpage = btreePricacheNodenew(cache);
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
	btreePribucketIdlist(cache,parray[i],idlist);

    ajListSort(idlist, &btreeKeywordCompare);

    totalkeys = (ajuint) ajListGetLength(idlist);

    btreeBucketSplitCalc(totalkeys, keylimit, cache->pnperbucket,
                         &lbucketlimit,&lmaxnperbucket,&lno,
                         &rbucketlimit,&rmaxnperbucket,&rno);

    cbucket = btreePribucketNew(cache->pnperbucket);

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
	btreeWritePribucket(cache,cbucket,newparray[i]);
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
    btreeWritePribucket(cache,cbucket,newparray[i]);

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
	btreeWritePribucket(cache,cbucket,newparray[i]);
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
    btreeWritePribucket(cache,cbucket,newparray[i]);

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

    btreePribucketDel(&cbucket);
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
	++cache->plevel;

	return spage;
    }

    page = btreePricacheRead(cache,prevsave);
    btreePriInsertKey(cache,page,mediankey,medianless,mediangtr);
    ajStrDel(&mediankey);

    page = btreePricacheRead(cache,prevsave);

    return page;
}




/* @funcstatic btreeSecbucketFindId *******************************************
**
** Tests for an ID in a secondary bucket
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
** @param [r] id [const AjPStr] id to search for
**
** @return [AjBool] ajTrue if found
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool btreeSecbucketFindId(AjPBtcache cache, ajulong pagepos,
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
    
    /* ajDebug("In btreeSecbucketFindId\n"); */

    /* Put in test here for secondary root page read
       instead of !pagepos - done */

/*
  if(pagepos == cache->secrootblock)
	ajFatal("SecbucketFindId: cannot read bucket from root page cache %S",
                cache->filename);
*/

    page  = btreeSeccacheRead(cache,pagepos);
    lpage = page;
    dirtysave = lpage->dirty;
    lpage->dirty = BT_LOCK;
    lpage->lockfor = 1311;

    buf = lpage->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);
    if(nodetype != BT_SECBUCKET)
	ajFatal("SecbucketFindId: NodeType mismatch. "
                "Not secondary bucket (%u) page %Lu cache %S",
		nodetype, pagepos, cache->filename);

    GBT_BUCKNENTRIES(buf,&nentries);
    if(nentries > cache->snperbucket)
	ajFatal("SecbucketFindId: Bucket too full page: %Lu "
                "entries: %u max: %u cache %S",
                pagepos, nentries, cache->snperbucket,
                cache->filename);
    

    GBT_BUCKOVERFLOW(buf,&overflow);

    kptr  = PBT_BUCKKEYLEN(buf);
    codeptr = kptr + (nentries * sizeof(ajuint));

    for(i=0;i<nentries;++i)
    {
	BT_GETAJUINT(kptr,&len);

/*
//	if((codeptr-buf+1) + len > cache->pagesize)	/# overflow #/
//	{
//	    /# ajDebug("SecbucketFindId: Overflow\n"); #/
//	    page  = btreeSeccacheRead(cache,overflow);
//	    buf = page->buf;
//	    GBT_BUCKNODETYPE(buf,&nodetype);
//	    if(nodetype != BT_SECBUCKET)
//		ajFatal("SecbucketFindId: NodeType mismatch. "
//                        "Not secondary bucket (%u) page: %Lu cache %S",
//                        nodetype, pagepos, cache->filename);
//	    GBT_BUCKOVERFLOW(buf,&overflow);
//	    /# overflow bucket ids start at the keylen position #/
//	    codeptr = PBT_BUCKKEYLEN(buf);
//	}
*/

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




/* @funcstatic btreeSecbucketIdcount ******************************************
**
** Counts secondary IDs in a bucket
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
**
** @return [ajulong] Number of IDs
**
** @release 6.5.0
** @@
******************************************************************************/

static ajulong btreeSecbucketIdcount(AjPBtcache cache, ajulong pagepos)
{
    AjPBtpage page  = NULL;

    unsigned char *buf  = NULL;
    
    ajuint  nodetype = 0;
    ajuint  nentries = 0;
    
    ajulong lcount = 0L;
    
/*
    if(pagepos == cache->secrootblock)
	ajFatal("SecbucketIdcount: cannot read bucket from root page cache %S",
               cache->filename);
*/

    page  = btreeSeccacheRead(cache,pagepos);

    buf = page->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);
    if(nodetype != BT_SECBUCKET)
	ajFatal("SecbucketIdcount: NodeType mismatch. "
                "Not secondary bucket (%u) cache %S",
		nodetype, cache->filename);
    
    GBT_BUCKNENTRIES(buf,&nentries);
    if(nentries > cache->snperbucket)
	ajFatal("SecbucketIdcount: Bucket too full page: %Lu "
                "entries: %u max: %u full cache %S",
                pagepos, nentries, cache->snperbucket, cache->filename);
    
    lcount = nentries;

    return lcount;
}




/* @funcstatic btreeSecbucketIdlist *******************************************
**
** Copies all secondary IDs into a list
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
** @param [u] idlist [AjPList] list to hold secondary ID strings
**
** @return [ajulong] Overflow
**
** @release 6.4.0
** @@
******************************************************************************/

static ajulong btreeSecbucketIdlist(AjPBtcache cache, ajulong pagepos,
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

/*
    if(pagepos == cache->secrootblock)
	ajFatal("SecbucketIdlist: cannot read bucket from root page cache %S",
               cache->filename);
*/

    page  = btreeSeccacheRead(cache,pagepos);
    lpage = page;
    dirtysave = lpage->dirty;
    lpage->dirty = BT_LOCK;
    lpage->lockfor = 1321;

    buf = lpage->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);
    if(nodetype != BT_SECBUCKET)
	ajFatal("SecbucketIdlist: NodeType mismatch. "
                "Not secondary bucket (%u) cache %S",
		nodetype, cache->filename);
    
    GBT_BUCKNENTRIES(buf,&nentries);
    if(nentries > cache->snperbucket)
	ajFatal("SecbucketIdlist: Bucket too full page: %Lu "
                "entries: %u max: %u full cache %S",
                pagepos, nentries, cache->snperbucket, cache->filename);
    

    GBT_BUCKOVERFLOW(buf,&overflow);
    pageoverflow = overflow;
    
    kptr  = PBT_BUCKKEYLEN(buf);
    codeptr = kptr + (nentries * sizeof(ajuint));

    for(i=0;i<nentries;++i)
    {
	BT_GETAJUINT(kptr,&len);

/*
//        if((codeptr-buf+1) + len > cache->pagesize)	/# overflow #/
//	{
//	    page  = btreeSeccacheRead(cache,overflow);
//	    buf = page->buf;
//	    GBT_BUCKNODETYPE(buf,&nodetype);
//	    if(nodetype != BT_SECBUCKET)
//		ajFatal("SecbucketIdlist: NodeType mismatch. "
//                        "Not secondary bucket (%u) cache %S",
//                        nodetype, cache->filename);
//	    GBT_BUCKOVERFLOW(buf,&overflow);
//	    /# overflow bucket ids start at the keylen position #/
//	    codeptr = PBT_BUCKKEYLEN(buf);
//	}
*/

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




/* @funcstatic btreeReadSecbucket *********************************************
**
** Constructor for keyword index secondary bucket given a disc page number
** Creates one empty key slot for possible addition
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
**
** @return [AjPSecbucket] bucket
**
** @release 3.0.0
** @@
******************************************************************************/

static AjPSecbucket btreeReadSecbucket(AjPBtcache cache, ajulong pagepos)
{
    AjPSecbucket bucket = NULL;

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
    
    /* ajDebug("In btreeReadSecbucket\n"); */

    if(pagepos == cache->secrootblock)
	ajFatal("btreeReadSecbucket: cannot read bucket from "
                "root page cache %S",
                cache->filename);

    page  = btreeSeccacheRead(cache,pagepos);
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

    bucket = btreeSecbucketNew(cache->snperbucket,cache->idlimit);
    bucket->Nentries = nentries;

    kptr  = PBT_BUCKKEYLEN(buf);
    codeptr = kptr + (nentries * sizeof(ajuint));

    for(i=0;i<nentries;++i)
    {
	BT_GETAJUINT(kptr,&len);

/*
//	if((codeptr-buf+1) + len > cache->pagesize)	/# overflow #/
//	{
//#if AJINDEX_DEBUG
//	    ajDebug("SecReadBucket: Overflow\n");
//#endif
//	    page  = btreeSeccacheRead(cache,overflow);
//	    buf = page->buf;
//	    GBT_BUCKNODETYPE(buf,&nodetype);
//
//	    if(nodetype != BT_SECBUCKET)
//		ajFatal("SecReadBucket: NodeType mismatch. Not secondary "
//			"bucket (%u)",nodetype);
//
//	    GBT_BUCKOVERFLOW(buf,&overflow);
//	    /# overflow bucket ids start at the keylen position #/
//	    codeptr = PBT_BUCKKEYLEN(buf);
//	}
*/

	/* Fill ID objects */
	ajStrAssignLenC(&bucket->SecIds[i],(const char *)codeptr,len-1);
	codeptr += len;

	kptr += sizeof(ajuint);
    }

    lpage->dirty = dirtysave;
    
    return bucket;
}




/* @funcstatic btreeWriteSecbucket ********************************************
**
** Write primary keyword index bucket object to the cache given a disc page
** number
**
** @param [u] cache [AjPBtcache] cache
** @param [r] bucket [const AjPSecbucket] bucket
** @param [r] pagepos [ajulong] page number
**
** @return [void]
**
** @release 3.0.0
** @@
******************************************************************************/

static void btreeWriteSecbucket(AjPBtcache cache, const AjPSecbucket bucket,
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

/*  ajulong   pno = 0L;*/

    /* ajDebug("In btreeWriteSecbucket\n"); */

    if(pagepos == cache->totsize)	/* Create a new page */
    {
	/* pno = pagepos; */
	page = btreeSeccacheBucketnew(cache);
	buf = page->buf;
	overflow = 0L;
    }
    else
    {
	page = btreeSeccacheRead(cache,pagepos);
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
/* these checks removed - pagesize dependency and already checked before call */
/*
//	if((ajuint)((keyptr-lbuf+1)+sizeof(ajuint)) > cache->pagesize)
//	    ajFatal("btreeWriteSecbucket: Bucket cannot hold more than %u keys",
//		    i-1);
*/

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

/*
//	if((lptr-buf+1)+len > cache->pagesize) /# overflow #/
//	{
//#if AJINDEX_DEBUG
//    	    ajDebug("btreeWriteSecbucket: Overflow\n");
//#endif
//
//	    if(!overflow)		/# No overflow buckets yet #/
//	    {
//		pno = cache->totsize;
//                lv = pno;
//		SBT_BUCKOVERFLOW(buf,lv);
//		page = btreeSeccacheBucketnew(cache);
//		buf = page->buf;
//		v = BT_SECBUCKET;
//		SBT_BUCKNODETYPE(buf,v);
//	    }
//	    else
//	    {
//		page = btreeSeccacheRead(cache,overflow);
//		buf  = page->buf;
//		GBT_BUCKOVERFLOW(buf,&overflow);
//	    }
//
//	    page->dirty = BT_DIRTY;
//
//	    lptr = PBT_BUCKKEYLEN(buf);	    
//	}
*/	

	sprintf((char *)lptr,"%s",MAJSTRGETPTR(sec));
	lptr += len;
    }

    lv = 0L;
    SBT_BUCKOVERFLOW(buf,lv);

    lpage->dirty = BT_DIRTY;

    return;
}




/* @funcstatic btreeWriteSecbucketEmpty ***************************************
**
** Write empty secondary keyword index bucket object to the cache given a
** disc page number
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void btreeWriteSecbucketEmpty(AjPBtcache cache, ajulong pagepos)
{
    AjPBtpage page  = NULL;
    AjPBtpage lpage = NULL;

    unsigned char *buf  = NULL;

    ajuint  v   = 0;
    ajulong lv  = 0L;

    ajulong overflow = 0L;

    if(pagepos == cache->totsize)	/* Create a new page */
    {
	page = btreeSeccacheBucketnew(cache);
	buf = page->buf;
	overflow = 0L;
	lv = overflow;
	SBT_BUCKOVERFLOW(buf,lv);
    }
    else
    {
	page = btreeSeccacheRead(cache,pagepos);
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

    lpage->dirty = BT_DIRTY;    /* clear the lock */

    return;
}




/* @func ajBtreeSeccacheNewC **************************************************
**
** Open a b+tree index file and initialise a cache object for keyword index
**
** @param [r] filetxt [const char *] name of file
** @param [r] exttxt [const char *] extension of file
** @param [r] idirtxt [const char *] index file directory
** @param [r] mode [const char *] opening mode
** @param [r] compressed [AjBool] Compressed index flag
** @param [r] kwlimit [ajuint] Max key size
** @param [r] idlimit [ajuint] Max secondary id size
** @param [r] pripagesize [ajuint] pagesize
** @param [r] secpagesize [ajuint] secondary pagesize
** @param [r] pricachesize [ajuint] size of cache
** @param [r] seccachesize [ajuint] size of secondary cache
** @param [r] pripagecount [ajulong] page count
** @param [r] secpagecount [ajulong] page count
** @param [r] order [ajuint] Tree order
** @param [r] fill [ajuint] Number of entries per bucket
** @param [r] level [ajuint] level of tree
** @param [r] sorder [ajuint] order of secondary tree
** @param [r] sfill [ajuint] Number of entries per secondary bucket
** @param [r] count [ajulong] Number of entries in the index
** @param [r] countall [ajulong] Number of total entries in the index
**
** @return [AjPBtcache] initialised disc block cache structure
**
** @release 3.0.0
** @@
******************************************************************************/

AjPBtcache ajBtreeSeccacheNewC(const char *filetxt, const char *exttxt,
			       const char *idirtxt, const char *mode,
                               AjBool compressed,
                               ajuint kwlimit, ajuint idlimit,
                               ajuint pripagesize, ajuint secpagesize,
                               ajuint pricachesize, ajuint seccachesize,
                               ajulong pripagecount, ajulong secpagecount,
                               ajuint order, ajuint fill, ajuint level,
			       ajuint sorder, ajuint sfill,
			       ajulong count, ajulong countall)
{
    AjPBtcache cache = NULL;
    AjPBtpage  page = NULL;
#if defined (usestat64)
    struct stat64 buf;
#else
    struct stat buf;
#endif
    ajulong filelen = 0L;
    AjBool douncompress = ajFalse;
    AjBool writemode = ajFalse;
    AjBool okcache = ajTrue;

    AJNEW0(cache);

    cache->prilistLength = 0;
    cache->seclistLength = 0;

    cache->plru   = NULL;
    cache->pmru   = NULL;
    cache->slru   = NULL;
    cache->smru   = NULL;
    
    cache->replace    = ajStrNew();
    cache->numreplace = 0L;
    
    if(pripagesize>0)
	cache->pripagesize = pripagesize;
    else
	cache->pripagesize = BT_PAGESIZE;

    if(secpagesize>0)
	cache->secpagesize = secpagesize;
    else
	cache->secpagesize = cache->pripagesize;

    cache->plevel       = level;
    cache->porder       = order;
    cache->pnperbucket  = fill;

    if(pricachesize > 0)
        cache->pricachesize = pricachesize;
    else
        cache->pricachesize = BT_CACHESIZE;

    cache->pripagecount = pripagecount;
    cache->secpagecount = secpagecount;

    cache->slevel = 0;
    cache->sorder = sorder;
    cache->snperbucket = sfill;
    if(seccachesize > 0)
        cache->seccachesize = seccachesize;
    else
        cache->seccachesize = cache->pricachesize;

    cache->countunique = count;
    cache->countall = countall;
    cache->keylimit = kwlimit;
    cache->idlimit = idlimit;
    cache->compressed = compressed;

    cache->bmem = NULL;
    cache->tmem = NULL;

    cache->bsmem = NULL;
    cache->tsmem = NULL;

    cache->secondary = ajTrue;
    
    cache->pripagetable = ajTablelongNewConst(cache->pricachesize);
    cache->secpagetable = ajTablelongNewConst(cache->seccachesize);

    cache->filename = ajStrNew();
    if(!*idirtxt)
        ajFmtPrintS(&cache->filename,"%s.%s",filetxt,exttxt);
    else if(idirtxt[strlen(idirtxt)-1] == SLASH_CHAR)
        ajFmtPrintS(&cache->filename,"%s%s.%s",idirtxt,filetxt,exttxt);
    else
        ajFmtPrintS(&cache->filename,"%s%s%s.%s",
		    idirtxt,SLASH_STRING,filetxt,exttxt);
    
    if(cache->porder < 4)
    {
        ajErr("cache '%S' pagesize %u order %u too small, increase pagesize",
              cache->filename, cache->pripagesize, cache->porder);
        okcache = ajFalse;
    }

    if(cache->pnperbucket < 4)
    {
        ajErr("cache '%S' pagesize %u fill %u too small, increase pagesize",
              cache->filename, cache->pripagesize, cache->pnperbucket);
        okcache = ajFalse;
    }

    if(cache->sorder < 4)
    {
        ajErr("cache '%S' pagesize %u sorder %u too small, increase pagesize",
              cache->filename, cache->pripagesize, cache->sorder);
        okcache = ajFalse;
    }

    if(cache->snperbucket < 4)
    {
        ajErr("cache '%S' pagesize %u sfill %u too small, increase pagesize",
              cache->filename, cache->secpagesize, cache->snperbucket);
        okcache = ajFalse;
    }

    if(!okcache)
        return NULL;

    cache->fp = fopen(MAJSTRGETPTR(cache->filename),mode);
    if(!cache->fp)
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
    else if(ajCharMatchC(mode, "wb+")) /* create */
    {
        writemode = ajTrue;
    }
    else 
    {
        ajWarn("ajBtreeSecCacheNewC unknown mode '%s'", mode);
    }
    
    cache->totsize    = filelen;
    cache->filesize  = filelen;

    if(douncompress)
        btreeCacheUncompress(cache);

    /* create or lock the root page */

    if(writemode)
        btreePrirootCreate(cache);
    else
    {
        page = btreePricacheRead(cache,0L);
        page->dirty = BT_LOCK;
    }

    return cache;
}




/* @func ajBtreeSeccacheNewS **************************************************
**
** Open a b+tree index file and initialise a cache object for keyword index
**
** @param [r] file [const AjPStr] name of file
** @param [r] ext [const AjPStr] extension of file
** @param [r] idir [const AjPStr] index file directory
** @param [r] mode [const char *] opening mode
** @param [r] compressed [AjBool] Compressed index flag
** @param [r] kwlimit [ajuint] Max key size
** @param [r] idlimit [ajuint] Max secondary id size
** @param [r] pripagesize [ajuint] Primary pagesize
** @param [r] secpagesize [ajuint] Secondary pagesize
** @param [r] pricachesize [ajuint] size of primary cache
** @param [r] seccachesize [ajuint] size of secondary cache
** @param [r] pripagecount [ajulong] Primary page count
** @param [r] secpagecount [ajulong] Secondary page count
** @param [r] order [ajuint] Tree order
** @param [r] fill [ajuint] Number of entries per bucket
** @param [r] level [ajuint] level of tree
** @param [r] sorder [ajuint] order of secondary tree
** @param [r] sfill [ajuint] Number of entries per secondary bucket
** @param [r] count [ajulong] Number of entries in the index
** @param [r] countall [ajulong] Number of total entries in the index
**
** @return [AjPBtcache] initialised disc block cache structure
**
** @release 6.4.0
** @@
******************************************************************************/

AjPBtcache ajBtreeSeccacheNewS(const AjPStr file, const AjPStr ext,
			       const AjPStr idir, const char *mode,
                               AjBool compressed,
                               ajuint kwlimit, ajuint idlimit,
                               ajuint pripagesize, ajuint secpagesize,
                               ajuint pricachesize, ajuint seccachesize,
                               ajulong pripagecount, ajulong secpagecount,
                               ajuint order, ajuint fill, ajuint level,
			       ajuint sorder, ajuint sfill,
			       ajulong count, ajulong countall)
{
    return ajBtreeSeccacheNewC(MAJSTRGETPTR(file), MAJSTRGETPTR(ext),
                               MAJSTRGETPTR(idir), mode,
                               compressed, kwlimit, idlimit,
                               pripagesize, secpagesize,
                               pricachesize, seccachesize,
                               pripagecount, secpagecount,
                               order, fill, level,
                               sorder, sfill,
                               count, countall);
}




/* @funcstatic btreeSecSplitleaf **********************************************
**
** Split a leaf and propagate up if necessary
**
** @param [u] cache [AjPBtcache] cache
** @param [u] spage [AjPBtpage] page
**
** @return [AjPBtpage] pointer to a parent page
**
** @release 3.0.0
** @@
******************************************************************************/

static AjPBtpage btreeSecSplitleaf(AjPBtcache cache, AjPBtpage spage)
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

    AjPSecbucket cbucket  = NULL;
    
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
    ajDebug("btreeSecSplitleaf %Lu\n", spage->pagepos);
#endif
    ++statCallSecSplitleaf;

    if(!statSaveSecId)
    {
        statSaveSecIdMax = 2048;
        AJCNEW0(statSaveSecId, statSaveSecIdMax);
        statSaveSecIdNext = 0;
    }

    order = cache->sorder;

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

    if(rootnodetype == BT_SECROOT)
    {
	lblockno = cache->totsize;
	lpage = btreeSeccacheNodenew(cache);
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
    rpage = btreeSeccacheNodenew(cache);
    rbuf = rpage->buf;
    rpage->dirty = BT_LOCK;
    rpage->lockfor = 1362;

    if(rootnodetype == BT_SECROOT)
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
        btreeSecbucketIdlist(cache, parray[i], idlist);

    ajListSort(idlist, &btreeKeywordIdCompare);


    totalkeys = (ajuint) ajListGetLength(idlist);

    btreeBucketSplitCalc(totalkeys, keylimit, cache->snperbucket,
                         &lbucketlimit,&lmaxnperbucket,&lno,
                         &rbucketlimit,&rmaxnperbucket,&rno);

    cbucket = btreeSecbucketNew(cache->snperbucket,cache->idlimit);

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
	btreeWriteSecbucket(cache,cbucket,newparray[i]);
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

    btreeWriteSecbucket(cache,cbucket,newparray[i]);

    nkeys = lbucketlimit;

    nodetype = BT_SECLEAF;
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

	btreeWriteSecbucket(cache,cbucket,newparray[i]);
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
    btreeWriteSecbucket(cache,cbucket,newparray[i]);

    nkeys = rbucketlimit;

    nodetype = BT_SECLEAF;
    v = nodetype;
    SBT_NODETYPE(rbuf,v);

    lv = prevsave;
    SBT_PREV(rbuf,lv);
    lv = overflow;
    SBT_OVERFLOW(rbuf,lv);
    
    btreeWriteNode(cache,rpage,newkarray,newparray,nkeys);
    rpage->dirty = BT_DIRTY;

    btreeSecbucketDel(&cbucket);
    ajListFree(&idlist);



    medianless = lblockno;
    mediangtr  = rblockno;

    btreeDeallocSecArray(cache,arrays);
    btreeDeallocSecArray(cache,newarrays);

    if(rootnodetype == BT_SECROOT)
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


    page = btreeSeccacheRead(cache,prevsave);
    btreeInsertKeySec(cache,page,mediankey,medianless,mediangtr);
    ajStrDel(&mediankey);

    page = btreeSeccacheRead(cache,prevsave);

    return page;
}




/* @funcstatic btreeKeywordIdCompare ******************************************
**
** Comparison function for ajListSort
**
** @param [r] a [const void*] ID 1
** @param [r] b [const void*] ID 2
**
** @return [ajint] 0 = bases match
**
** @release 3.0.0
** @@
******************************************************************************/

static ajint btreeKeywordIdCompare(const void *a, const void *b)
{
    return MAJSTRCMPS((*(AjPStr const *)a),
                      (*(AjPStr const *)b));
}




/* @funcstatic btreeSecbucketNew **********************************************
**
** Construct a primary keyword secondary bucket object
**
** @param [r] n [ajuint] Number of IDs
** @param [r] idlen [ajuint] Maximum size of IDs
**
** @return [AjPSecbucket] initialised disc block cache structure
**
** @release 3.0.0
** @@
******************************************************************************/

static AjPSecbucket btreeSecbucketNew(ajuint n, ajuint idlen)
{
    AjPSecbucket bucket = NULL;
    ajuint i;

    /* ajDebug("In btreeSecbucketNew\n"); */
    
    if(n)
    {
        if(statSaveSecbucketNext)
        {
            bucket = statSaveSecbucket[--statSaveSecbucketNext];
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
        if(statSaveSecbucketEmptyNext)
            bucket = statSaveSecbucketEmpty[--statSaveSecbucketEmptyNext];
        else            
            AJNEW0(bucket);
    }
    
    bucket->NodeType = BT_SECBUCKET;
    bucket->Nentries = n;
    bucket->Overflow = 0L;
    
    return bucket;
}




/* @funcstatic btreeSecbucketDel **********************************************
**
** Delete a keyword primary bucket object
**
** @param [w] thys [AjPSecbucket*] bucket
**
** @return [void]
**
** @release 3.0.0
** @@
******************************************************************************/

static void btreeSecbucketDel(AjPSecbucket *thys)
{
    AjPSecbucket pthis = NULL;
    ajuint newmax;
    
    /* ajDebug("In btreeSecbucketDel\n"); */

    if(!thys || !*thys)
	return;

    pthis = *thys;

    if(!statSaveSecbucket)
    {
        statSaveSecbucketMax=2048;
        statSaveSecbucketNext=0;
        AJCNEW0(statSaveSecbucket,statSaveSecbucketMax);
    }
    
    if(!statSaveSecbucketEmpty)
    {
        statSaveSecbucketEmptyMax=2048;
        statSaveSecbucketEmptyNext=0;
        AJCNEW0(statSaveSecbucketEmpty,statSaveSecbucketEmptyMax);
    }
    

    /*
    statCountSecbucketDel++;
    statCountSecbucket--;
    if(pthis->Nentries > statMaxSecbucket)
      statMaxSecbucket = pthis->Nentries;
    */

    if(pthis->Maxentries)
    {
        /*statReusedSecbucket++;
          statUsedSecbucket--;*/
        if(statSaveSecbucketNext >= statSaveSecbucketMax)
        {
            newmax = statSaveSecbucketMax + statSaveSecbucketMax;
            AJCRESIZE0(statSaveSecbucket,statSaveSecbucketMax,newmax);
            statSaveSecbucketMax = newmax;
        }
        
        statSaveSecbucket[statSaveSecbucketNext++] = pthis;
        /*if(ajListGetLength(statListSecbucket) > statMaxFreeSecbucket)
          statMaxFreeSecbucket = ajListGetLength(statListSecbucket);*/
    }
    else
    {
        if(statSaveSecbucketEmptyNext >= statSaveSecbucketEmptyMax)
        {
            newmax = statSaveSecbucketEmptyMax + statSaveSecbucketEmptyMax;
            AJCRESIZE0(statSaveSecbucketEmpty,statSaveSecbucketEmptyMax,newmax);
            statSaveSecbucketEmptyMax = newmax;
        }
        statSaveSecbucketEmpty[statSaveSecbucketEmptyNext++] = pthis;
        /*statReusedSecbucketEmpty++;
          statUsedSecbucketEmpty--;*/
        /*if(ajListGetLength(statListSecbucketEmpty) > statMaxFreeSecbucketEmpty)
          statMaxFreeSecbucketEmpty = ajListGetLength(statListSecbucketEmpty);*/
    }
    
    pthis = NULL;

    *thys = NULL;

    return;
}




/* @funcstatic btreeSecbucketFree *********************************************
**
** Delete a keyword primary bucket object
**
** @param [w] thys [AjPSecbucket*] bucket
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void btreeSecbucketFree(AjPSecbucket *thys)
{
    AjPSecbucket pthis = NULL;
    ajuint n;
    ajuint i;
    
    /* ajDebug("In btreeSecbucketFree\n"); */

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




/* @funcstatic btreeKeyidInsert ***********************************************
**
** Insert an ID into the secondary keyword tree
**
** Depends on secrootblock being set before the function is called
**
** @param [u] cache [AjPBtcache] cache
** @param [r] id [const AjPStr] Id
**
** @return [AjBool] True if ID was inserted
**                  False if ID already exists
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool btreeKeyidInsert(AjPBtcache cache, const AjPStr id)
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
    
    /* ajDebug("In btreeKeyidInsert\n"); */


    if(!MAJSTRGETLEN(id))
        return ajFalse;

    /* Only insert an ID if it doesn't exist */
    exists = btreeKeyidExists(cache,id);

    if(exists)
	return ajFalse;
    
    spage = btreeKeyidFind(cache,id);
    buf = spage->buf;

    GBT_NODETYPE(buf,&nodetype);
    if(nodetype == BT_SECBUCKET)
    {
        blockno = spage->pagepos;
        n = btreeNumInSecbucket(cache,blockno);
        if(n < cache->snperbucket)
        {
            btreeSecbucketAdd(cache,blockno,id);
            return ajTrue;
        }

        /*
        ** make a list of the IDs in the bucket
        ** make a new root
        ** split the IDs into the two buckets
        */

        btreeKeyidMakeroot(cache, spage);
    }

    GBT_NKEYS(buf,&nkeys);
    if(!nkeys)
    {
        lblockno = cache->totsize;
        btreeWriteSecbucketEmpty(cache,lblockno);

        rblockno = cache->totsize;
        btreeWriteSecbucketEmpty(cache,rblockno);	

        btreeWriteNodeSingle(cache,spage,id,lblockno,rblockno);

        btreeSecbucketAdd(cache,rblockno,id);

        return ajTrue;
    }
    
    blockno = btreeGetBlockS(cache,buf,id);
    
    if(nodetype != BT_SECROOT)
        if((shift = btreeKeyidInsertShift(cache,&spage,id)))
            blockno = shift;

    buf = spage->buf;

    n = btreeNumInSecbucket(cache,blockno);

    if(n == cache->snperbucket)
    {
	if(btreeSecbucketsReorder(cache,spage))
	{
            blockno = btreeGetBlockS(cache,buf,id);
	}
	else
	{
	    btreeSecSplitleaf(cache,spage);
	    
	    spage = btreeKeyidFind(cache,id);
	    buf = spage->buf;

            blockno = btreeGetBlockS(cache,buf,id);
	}
    }


    btreeSecbucketAdd(cache,blockno,id);

    return ajTrue;
}




/* @funcstatic btreeKeyidExists ***********************************************
**
** See whether ID already exists in the tree
**
** @param [u] cache [AjPBtcache] cache
** @param [r] key [const AjPStr] key
**
** @return [AjBool] true if ID already added
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool btreeKeyidExists(AjPBtcache cache, const AjPStr key)
{
    AjPBtpage page      = NULL;
    unsigned char *buf = NULL;

    ajuint nkeys    = 0;
    ajuint nodetype;

    ajulong blockno = 0L;
    AjBool found   = ajFalse;

    /* ajDebug("In btreeKeyidExists %u\n",cache->count); */

    if(!cache->countunique)
	return ajFalse;
    
    page = btreeKeyidFind(cache,key);
    buf = page->buf;
    GBT_NODETYPE(buf, &nodetype);

    if(nodetype == BT_SECBUCKET)
    {
        blockno = page->pagepos;
    }
    else
    {
        GBT_NKEYS(buf,&nkeys);

        if(!nkeys)
            return ajFalse;
    
        blockno = btreeGetBlockS(cache,buf,key);
    }

    found = btreeSecbucketFindId(cache, blockno, key);

    return found;
}




/* @func btreeKeyidFind *************************************************
**
** Find the node that should contain a new key for insertion
**
** @param [u] cache [AjPBtcache] cache
** @param [r] key [const AjPStr] key to search for 
**
** @return [AjPBtpage] leaf node where item should be inserted
**
** @release 3.0.0
** @@
******************************************************************************/

AjPBtpage btreeKeyidFind(AjPBtcache cache, const AjPStr key)
{
    AjPBtpage root = NULL;
    AjPBtpage ret  = NULL;
    ajuint nodetype;

    /* ajDebug("In btreeKeyidFind\n"); */

    /* The root node should always be in the cache (BT_LOCKed) */
    root = btreeSeccacheLocate(cache,cache->secrootblock);

    /* ajDebug("cache->slevel = %u root=%u\n",cache->slevel,(ajuint)root); */
    
    GBT_NODETYPE(root->buf, &nodetype);

    if(nodetype == BT_SECBUCKET)
        return root;

    if(!cache->slevel)
	return root;
    
    ret = btreeKeyidFindINode(cache,root,key);

    return ret;
}




/* @funcstatic btreeNumInSecbucket ********************************************
**
** Return number of entries in a secondary id bucket
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
**
** @return [ajuint] Number of entries in bucket
**
** @release 3.0.0
** @@
******************************************************************************/

static ajuint btreeNumInSecbucket(AjPBtcache cache, ajulong pagepos)
{
    unsigned char *buf = NULL;

    AjPBtpage page = NULL;
    ajuint  nodetype = 0;
    ajuint nentries = 0;
    
    /* ajDebug("In btreeNumInPribucket\n"); */

/*        
    if(pagepos == cache->secrootblock)
	ajFatal("NumInSecbucket: Attempt to read bucket from root page\n");
*/

    page  = btreeSeccacheRead(cache,pagepos);

    buf = page->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);

    if(nodetype != BT_SECBUCKET)
	ajFatal("SecReadBucket: NodeType mismatch. Not secondary bucket (%u)",
		nodetype);
    
    GBT_BUCKNENTRIES(buf,&nentries);

    return nentries;
}




/* @funcstatic btreeSecbucketAdd **********************************************
**
** Add a keyword ID to a secondary bucket
** Only called if there is room in the bucket
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number of bucket
** @param [r] id [const AjPStr] ID
**
** @return [void]
**
** @release 3.0.0
** @@
******************************************************************************/

static void btreeSecbucketAdd(AjPBtcache cache, ajulong pagepos,
                              const AjPStr id)
{
    unsigned char *buf  = NULL;
    unsigned char *kptr = NULL;
    unsigned char *src  = NULL;
    unsigned char *dest = NULL;

/*    unsigned char *lastptr = NULL;*/
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
/*    static ajuint overflowcalls=0;*/
   
    calls++;
    page = btreeSeccacheRead(cache,pagepos);
    buf  = page->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);
    if(nodetype != BT_SECBUCKET)
        ajFatal("Wrong nodetype in SecbucketAdd cache %S",
                cache->filename);

    GBT_BUCKNENTRIES(buf,&nentries);
    if(nentries >= cache->snperbucket)
        ajFatal("Bucket too full in SecbucketAdd page %Lu %u %u cache %S",
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
    
    endptr  = src + sum;
    idlen   = MAJSTRGETLEN(id);

/*
//    lastptr = endptr + sizeof(ajuint) + idlen;
//    if((ajuint) (lastptr - buf) >= cache->pagesize)
//    {
//        overflowcalls++;
//        ajWarn("\nOverflow in SecbucketAdd nentries:%u fails %u/%u '%S' "
//               "cache %S",
//               nentries, overflowcalls,calls, id, cache->filename);
//        btreeSecbucketAddFull(cache,pagepos,id);
//        return;
//    }
*/    

    dest = src + sizeof(ajuint);
    memmove((void *)dest, (void *)src, sum);

    v = idlen+1;
    BT_SETAJUINT(src,v);

    endptr += sizeof(ajuint);
    strcpy((char *)endptr,MAJSTRGETPTR(id));

    v = nentries + 1;
    SBT_BUCKNENTRIES(buf,v);

    page->dirty = BT_DIRTY;

    return;
}




#if 0
/* #funcstatic btreeSecbucketAddFull ******************************************
**
** Add a keyword ID to a secondary bucket
** Only called if there is room in the bucket
**
** #param [u] cache [AjPBtcache] cache
** #param [r] pagepos [ajulong] page number of bucket
** #param [r] id [const AjPStr] ID
**
** #return [void]
**
** #release 6.4.0
** ##
******************************************************************************/
/*
//static void btreeSecbucketAddFull(AjPBtcache cache, ajulong pagepos,
//                                  const AjPStr id)
//{
//    AjPSecbucket bucket = NULL;
//    ajuint nentries;
//    
//    /# ajDebug("In btreeSecbucketAddFull\n"); #/
//
//    bucket   = btreeReadSecbucket(cache,pagepos);
//    nentries = bucket->Nentries;
//
//
//    /# Reading a bucket always gives one extra ID position #/
//
//    ajStrAssignS(&bucket->SecIds[nentries],id);
//
//    ++bucket->Nentries;
//
//    btreeWriteSecbucket(cache,bucket,pagepos);
//
//    btreeSecbucketDel(&bucket);
//    
//    return;
//}
*/
#endif




/* @funcstatic btreeKeyidMakeroot *********************************************
**
** Re-write root secondary id buckets as a root node with two buckets
** Must only be called if the buckets is full
**
** @param [u] cache [AjPBtcache] cache
** @param [u] bucket [AjPBtpage] secbucket page
**
** @return [AjBool] true if reorder was successful i.e. leaf not full
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool btreeKeyidMakeroot(AjPBtcache cache, AjPBtpage bucket)
{
    AjPList idlist;
    ajulong pagepos;
    ajulong lblockno;
    ajulong rblockno;
    AjPBtpage page;
    AjPSecbucket cbucket = NULL;
    AjPStr bid      = NULL;
    ajuint count;
    ajuint maxleft;
    ajuint newmax;

    pagepos = bucket->pagepos;

    if(!statSaveSecId)
    {
        statSaveSecIdMax = 2048;
        AJCNEW0(statSaveSecId, statSaveSecIdMax);
        statSaveSecIdNext = 0;
    }

    /* save the ids in the bucket ... a simple list of strings */

    idlist = ajListNew();
    btreeSecbucketIdlist(cache, bucket->pagepos, idlist);
    ajListSort(idlist, &btreeKeywordIdCompare);

    /* create a pair of buckets linked to the root */

    lblockno = cache->totsize;
    btreeWriteSecbucketEmpty(cache,lblockno);

    rblockno = cache->totsize;
    btreeWriteSecbucketEmpty(cache,rblockno);	
  
    /* save half the ids to the left, half to the right */

    cbucket = btreeSecbucketNew(cache->snperbucket,cache->idlimit);
    cbucket->Overflow = 0L;
    cbucket->Nentries = 0;
    count = 0;
    maxleft = (ajuint) ajListGetLength(idlist)/2;

    while(count < maxleft)
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

    btreeWriteSecbucket(cache,cbucket,lblockno);

    /* make the bucket into the new secondary root */

    cache->secrootblock = pagepos;
    btreeSecrootCreate(cache, pagepos);

    page = btreeSeccacheWrite(cache,pagepos);
    page->dirty = BT_LOCK;
    page->lockfor = 1251;

    if(btreeDoRootSync)
        btreeCacheRootSync(cache,pagepos);

    cache->slevel = 0;

    ajListPeek(idlist,(void **)&bid); /* mid-range key to save in root node */
    btreeWriteNodeSingle(cache,page,bid,lblockno,rblockno);

    cbucket->Overflow = 0L;
    cbucket->Nentries = 0;

    count = 0;

    while(ajListGetLength(idlist))
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

    btreeWriteSecbucket(cache,cbucket,rblockno);

    btreeSecbucketDel(&cbucket);
    ajListFree(&idlist);

    return ajTrue;
}




/* @funcstatic btreeSecbucketsReorder *****************************************
**
** Re-order secondary id leaf buckets
** Must only be called if one of the buckets is full
**
** @param [u] cache [AjPBtcache] cache
** @param [u] leaf [AjPBtpage] leaf page
**
** @return [AjBool] true if reorder was successful i.e. leaf not full
**
** @release 3.0.0
** @@
******************************************************************************/

static AjBool btreeSecbucketsReorder(AjPBtcache cache, AjPBtpage leaf)
{
    ajuint nkeys = 0;
    AjPSecbucket cbucket  = NULL;
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
    ajDebug("btreeSecbucketsReorder %Lu\n", leaf->pagepos);
#endif

    /* ajDebug("In btreeSecbucketsReorder\n"); */

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
	ajFatal("SecbucketsReorder: Attempt to reorder empty lea cache %Sf",
                cache->filename);

    if(nkeys > order)
    {
        ajErr("SecbucketsReorder: nkeys %u > order %u cache %S",
              nkeys, order, cache->filename);
    }
    
    for(i=0;i<nkeys;++i)
	totalkeys += btreeNumInSecbucket(cache,ptrs[i]);

    totalkeys += btreeNumInSecbucket(cache,ptrs[i]);

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
    
    ++statCallSecbucketsReorder;

    /* Read IDs from all buckets and push to list and sort (increasing kw) */
    idlist  = ajListNew();
    
    for(i=0;i<keylimit;++i)
	overflows[i] = btreeSecbucketIdlist(cache,ptrs[i],idlist);


    ajListSort(idlist, &btreeKeywordIdCompare);

    cbucket = btreeSecbucketNew(cache->snperbucket,cache->idlimit);
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
	btreeWriteSecbucket(cache,cbucket,newptrs[i]);
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
    btreeWriteSecbucket(cache,cbucket,newptrs[i]);

    btreeSecbucketDel(&cbucket);

#if AJINDEX_DEBUG
    if(bucketlimit <= keylimit)
        ajDebug("btreeSecbucketsReorder '%S' %u -> %u",
                cache->filename, keylimit, bucketlimit);
#endif

    for(i = bucketlimit + 1; i <= keylimit; i++)
    {
        btreeSecpageSetfree(cache, ptrs[i]);
    }

    /* Now write out a modified leaf with new keys/ptrs */

    nkeys = bucketlimit;
    btreeWriteNode(cache,leaf,newkeys,newptrs,nkeys);
    leaf->dirty = BT_DIRTY;

    if(nodetype == BT_SECROOT)
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
/* @funcstatic btreeInsertIdOnly **********************************************
**
** Add only a secondary ID: the primary keyword already exists
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pri [const AjPBtPri] keyword/id
**
** @return [void]
**
** @release 3.0.0
** @@
******************************************************************************/

static void btreeInsertIdOnly(AjPBtcache cache, const AjPBtPri pri)
{
    unsigned char *buf;
    AjPBtpage page = NULL;
    ajuint nodetype;

    ajulong right = 0L;

    /* ajDebug("In btreeInsertIdOnly\n"); */
    
    if(!pri->treeblock)
    {
	fprintf(stderr,"btreeInsertIdOnly: root page doesn't exist\n");
	exit(-1);
    }
    
    cache->secrootblock = pri->treeblock;
    page = btreeSeccacheWrite(cache,cache->secrootblock);
    page->dirty = BT_LOCK;
    page->lockfor = 1381;
    buf = page->buf;
    GBT_RIGHT(buf, &right);
    cache->slevel = (ajuint) right;

    btreeKeyidInsert(cache, pri->id);

    GBT_NODETYPE(buf, &nodetype);
    if(nodetype != BT_SECBUCKET)
    {
        right = (ajulong) cache->slevel;
        SBT_RIGHT(buf, right);
        page->dirty = BT_DIRTY;
    }

    return;
}
#endif




/* @funcstatic btreeSecSplitroot **********************************************
**
** Split a secondary root node
**
** @param [u] cache [AjPBtcache] cache
**
** @return [void]
**
** @release 3.0.0
** @@
******************************************************************************/

static void btreeSecSplitroot(AjPBtcache cache)
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
    ajDebug("btreeSecSplitroot %Lu\n", cache->secrootblock);
#endif
    ++statCallSecSplitroot;

    /* ajDebug("In btreeSecSplitroot\n"); */

    order = cache->sorder;

    arrays = btreeAllocSecArray(cache);
    tarrays = btreeAllocSecArray(cache);
    karray = arrays->karray;
    parray = arrays->parray;
    tkarray = tarrays->karray;
    tparray = tarrays->parray;

    rootpage = btreeSeccacheLocate(cache,cache->secrootblock);
    rootbuf = rootpage->buf;

    nkeys = order - 1;

    keypos = nkeys / 2;

    if(!(nkeys % 2))
	--keypos;


    rblockno = cache->totsize;
    rpage = btreeSeccacheNodenew(cache);
    rpage->dirty = BT_LOCK;
    rpage->lockfor = 1391;

    lblockno = cache->totsize;
    lpage = btreeSeccacheNodenew(cache);

    /* Comment this next block out after the beta test */
    if(!cache->slevel)
    {
	fprintf(stderr,"btreeSecSplitroot Shouldn't get here\n");
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
	nodetype = BT_SECINTERNAL;
    else
	nodetype = BT_SECLEAF;

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
	tpage = btreeSeccacheRead(cache,tparray[i]);
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
	tpage = btreeSeccacheRead(cache,tparray[i]);
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




/* @funcstatic btreeInsertKeySec **********************************************
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
**
** @release 3.0.0
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
	btreeInsertNonfullSec(cache,page,key,less,greater);

	return;
    }
    
    order = cache->sorder;
    lbuf = page->buf;
    GBT_NODETYPE(lbuf,&nodetype);
    page->dirty = BT_LOCK;
    page->lockfor = 1401;
    
    if(nodetype == BT_SECROOT)
    {
	btreeSecSplitroot(cache);
	page->dirty = BT_DIRTY;
	
	blockno = btreeGetBlockFirstS(cache,lbuf,key);

	ipage = btreeSeccacheRead(cache,blockno);
	btreeInsertNonfullSec(cache,ipage,key,less,greater);

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
    rpage = btreeSeccacheNodenew(cache);
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
	tpage = btreeSeccacheRead(cache,tparray[i]);
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
	tpage = btreeSeccacheRead(cache,tparray[i]);
	tbuf = tpage->buf;
	lv = rblockno;
	SBT_PREV(tbuf,lv);
	tpage->dirty = BT_DIRTY;
    }


    ibn = rblockno;

    if(MAJSTRCMPS(key,mediankey)<0)
	ibn = lblockno;

    ipage = btreeSeccacheRead(cache,ibn);
    
    btreeInsertNonfullSec(cache,ipage,key,less,greater);



    ipage = btreeSeccacheRead(cache,prev);

    btreeInsertKeySec(cache,ipage,mediankey,medianless,mediangtr);
    btreeDeallocSecArray(cache,savekeyarrays); /* mediankey points here */
    btreeDeallocSecArray(cache,tarrays);

    ajStrDel(&mediankey);

    return;
}




/* @funcstatic btreeKeyidInsertShift ******************************************
**
** Rebalance buckets on insertion of a secondary identifier
**
** @param [u] cache [AjPBtcache] cache
** @param [u] retpage [AjPBtpage*] page
** @param [r] key [const AjPStr] key
**
** @return [ajulong] bucket block or 0L if shift not possible
**
** @release 6.5.0
** @@
******************************************************************************/

static ajulong btreeKeyidInsertShift(AjPBtcache cache, AjPBtpage *retpage,
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
    
    /* ajDebug("In btreeKeyidInsertShift\n"); */


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

    
    ppage = btreeSeccacheRead(cache,parent);
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
	spage = btreeSeccacheRead(cache,pParray[ppos-1]);
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
	spage = btreeSeccacheRead(cache,pParray[ppos+1]);
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




/* @funcstatic btreeInsertNonfullSec ******************************************
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
**
** @release 3.0.0
** @@
******************************************************************************/

static void btreeInsertNonfullSec(AjPBtcache cache, AjPBtpage page,
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
    
    /* ajDebug("In btreeInsertNonfullSec\n"); */

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

    if(nodetype == BT_SECROOT)
    {
	page->dirty = BT_LOCK;
        page->lockfor = 1421;
    }

    pagepos = page->pagepos;
    ppage = btreeSeccacheRead(cache,less);
    lv = pagepos;
    SBT_PREV(ppage->buf,lv);
    ppage->dirty = BT_DIRTY;

    ppage = btreeSeccacheRead(cache,greater);
    lv = pagepos;
    SBT_PREV(ppage->buf,lv);
    ppage->dirty = BT_DIRTY;

    btreeDeallocSecArray(cache,arrays);

    if(nodetype != BT_SECROOT)
	btreeKeyShiftSec(cache,page);

    return;
}




/* @funcstatic btreeKeyShiftSec ***********************************************
**
** Rebalance secondary Nodes on insertion
**
** @param [u] cache [AjPBtcache] cache
** @param [u] tpage [AjPBtpage] page
**
** @return [void]
**
** @release 3.0.0
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

    
    ppage = btreeSeccacheRead(cache,parent);
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
	spage = btreeSeccacheRead(cache,pParray[pkeypos]);
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

	page = btreeSeccacheRead(cache,pSarray[skeys]);
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
	spage = btreeSeccacheRead(cache,pParray[pkeypos+1]);
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

	page = btreeSeccacheRead(cache,pSarray[0]);
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




#if AJINDEX_STATIC
/* #func ajBtreeLockTest ******************************************************
**
** Test function: show if a primary tree root block is unlocked
**
** #param [u] cache [AjPBtcache] cache
**
** #return [void]
**
** #release 3.0.0
** ##
******************************************************************************/
/*
static void btreeLockTest(AjPBtcache cache)
{
    AjPBtpage page = NULL;
    
    page = btreePricacheLocate(cache,0L);

    if(page->dirty != BT_LOCK)
	ajFatal("Root page unlocked\n");

    return;
}
*/
#endif




/* @funcstatic btreeSecTreeCount **********************************************
**
** Count the IDs in a secondary tree of identifiers for primary keywords
**
** @param [u] cache [AjPBtcache] cache
** @param [r] rootblock [ajulong] root page of secondary tree
**
** @return [ajulong] Number of IDs in secondary tree
**
** @release 6.5.0
** @@
******************************************************************************/

static ajulong btreeSecTreeCount(AjPBtcache cache, ajulong rootblock)
{
    AjPBtMem arrays = NULL;
    ajulong *parray;

    AjPBtpage page;
    unsigned char *buf;
    ajuint nodetype;
    ajuint j;
    ajulong level = 0L;
    
    ajuint nkeys;
    ajulong right;

    ajulong lcount = 0L;

    page = btreeSeccacheRead(cache,rootblock);
    buf = page->buf;
    GBT_NODETYPE(buf,&nodetype);

    if(nodetype == BT_SECBUCKET)
    {
        return btreeSecbucketIdcount(cache, page->pagepos);
    }

    GBT_RIGHT(buf,&level);
    cache->slevel = (ajuint)level;
    
    arrays = btreeAllocSecArray(cache);
    parray = arrays->parray;
    
    btreeGetPointers(cache,buf,&parray);

    while(nodetype != BT_SECLEAF && cache->slevel!=0)
    {
	page = btreeSeccacheRead(cache,parray[0]);
	buf = page->buf;
	btreeGetPointers(cache,buf,&parray);
	GBT_NODETYPE(buf,&nodetype);
    }

    do
    {
	GBT_NKEYS(buf,&nkeys);

        page->dirty = BT_LOCK;
        page->lockfor = 9901;

	for(j=0;j<=nkeys;++j)
	    lcount += btreeNumInSecbucket(cache, parray[j]);

        page->dirty = BT_CLEAN;
	right = 0L;

	if(cache->slevel)
	{
	    GBT_RIGHT(buf,&right);

	    if(right)
	    {
		page = btreeSeccacheRead(cache,right);	    
		buf = page->buf;
		btreeGetPointers(cache,buf,&parray);
	    }
	}
	
    } while(right);

    btreeDeallocSecArray(cache,arrays);

    return lcount;
}




/* @funcstatic btreeSecTreeList ***********************************************
**
** Read the leaves of a secondary tree, storing the entry secondary
** identifiers in a list
**
** @param [u] cache [AjPBtcache] cache
** @param [r] rootblock [ajulong] root page of secondary tree
**
** @return [AjPList] List of IDs matching a keyword
**
** @release 6.5.0
** @@
******************************************************************************/

static AjPList btreeSecTreeList(AjPBtcache cache, ajulong rootblock)
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
    
    AjPSecbucket bucket;
    ajuint nkeys;
    AjPStr id = NULL;
    ajulong right;

    list = ajListNew();

    page = btreeSeccacheRead(cache,rootblock);
    buf = page->buf;
    GBT_NODETYPE(buf,&nodetype);

    if(nodetype == BT_SECBUCKET)
    {
        btreeSecbucketIdlist(cache, page->pagepos, list);
        return list;
    }

    GBT_RIGHT(buf,&level);
    cache->slevel = (ajuint)level;
    
    arrays = btreeAllocSecArray(cache);
    parray = arrays->parray;
    
    btreeGetPointers(cache,buf,&parray);

    while(nodetype != BT_SECLEAF && cache->slevel!=0)
    {
	page = btreeSeccacheRead(cache,parray[0]);
	buf = page->buf;
	btreeGetPointers(cache,buf,&parray);
	GBT_NODETYPE(buf,&nodetype);
    }

    do
    {
	GBT_NKEYS(buf,&nkeys);

        page->dirty = BT_LOCK;
        page->lockfor = 9901;

	for(j=0;j<=nkeys;++j)
	{
	    bucket = btreeReadSecbucket(cache, parray[j]);
	    
	    for(k=0;k<bucket->Nentries;++k)
	    {
		id = ajStrNewS(bucket->SecIds[k]);
		ajListPushAppend(list, (void *)id);
	    }

	    btreeSecbucketDel(&bucket);
	}

        page->dirty = BT_CLEAN;
	right = 0L;

	if(cache->slevel)
	{
	    GBT_RIGHT(buf,&right);

	    if(right)
	    {
		page = btreeSeccacheRead(cache,right);	    
		buf = page->buf;
		btreeGetPointers(cache,buf,&parray);
	    }
	}
	
    } while(right);

    btreeDeallocSecArray(cache,arrays);

    return list;
}




#if AJINDEX_STATIC
/* #funcstatic btreeKeyidVerify ***********************************************
**
** Test routine: test for ID within a secondary tree
**
** #param [u] cache [AjPBtcache] cache
** #param [r] rootblock [ajulong] root page of secondary tree
** #param [r] id [const AjPStr] test ID
**
** #return [AjBool] true if ID found
**
** #release 6.5.0
** ##
******************************************************************************/
/*
static AjBool btreeKeyidVerify(AjPBtcache cache, ajulong rootblock,
                               const AjPStr id)
{
    AjPBtpage page;
    AjPBtpage spage;
    unsigned char *buf;
    ajulong blockno;
    AjBool found;
    ajulong right;
    ajuint dirtysave;

    cache->secrootblock = rootblock;
    page = btreeSeccacheRead(cache,rootblock);
    dirtysave = page->dirty;
    page->dirty = BT_LOCK;
    page->lockfor = 1441;
    buf = page->buf;
    GBT_RIGHT(buf,&right);
    cache->slevel = (ajuint)right;
    
    spage = btreeKeyidFind(cache,id);

    if(!spage)
	return ajFalse;
    
    buf = spage->buf;
    
    blockno = btreeGetBlockS(cache,buf,id);

    found = btreeSecbucketFindId(cache, blockno, id);

    cache->secrootblock = 0L;
    page->dirty = dirtysave;

    return found;
}
*/
#endif




/* @func ajBtreeKeywildQuery **************************************************
**
** Wildcard retrieval of keyword index entries
**
** @param [u] cache [AjPBtcache] cache
** @param [u] wild [AjPBtKeywild] Wildcard
** @param [u] idcache [AjPBtcache] id cache
**
** @return [AjPBtId] next matching Id or NULL
**
** @release 3.0.0
** @@
******************************************************************************/

AjPBtId ajBtreeKeywildQuery(AjPBtcache cache, AjPBtKeywild wild,
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

    key  = ajStrNewS(wild->prefix);
    
    list = wild->list;

    found = ajFalse;

    /*
    ** note prefix may be empty if query started with a wildcard
    */

    if(wild->first)
    {
	page = btreePrimaryFetchFindleafWild(cache,key);
	page->dirty = BT_LOCK;
        page->lockfor = 1451;
	wild->pagepos = page->pagepos;
	
	btreePrileafFetch(cache,page,list);

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

	    page = btreePricacheRead(cache,pagepos);
	    wild->pagepos = pagepos;
	    page->dirty = BT_LOCK;
            page->lockfor = 1452;
	    
	    btreePrileafFetch(cache,page,list);	
	    
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
	btid = btreeIdentQueryId(idcache,id);
	ajStrDel(&id);
        ajStrDel(&key);

	return btid;
    }
    else if((btreeKeywildNextList(cache,wild)))
    {
	if(ajListGetLength(wild->idlist))
	{
	    ajListPop(wild->idlist,(void **)&id);
	    btid = btreeIdentQueryId(idcache,id);
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
	page = btreePricacheRead(cache,wild->pagepos); 
	buf = page->buf;
	GBT_RIGHT(buf,&pagepos);

	if(!pagepos)
        {
            ajStrDel(&key);
            return NULL;
        }

	page = btreePricacheRead(cache,pagepos);
	wild->pagepos = pagepos;
	page->dirty = BT_LOCK;
        page->lockfor = 1453;

	btreePrileafFetch(cache,page,list);	

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
	btid = btreeIdentQueryId(idcache,id);
	ajStrDel(&id);
    }
    else
	return NULL;
    
    return btid;
}




/* @func ajBtreeKeyFetchId ****************************************************
**
** Retrieval of keyword index entries with a perfect match to the keyword
**
** @param [u] cache [AjPBtcache] cache
** @param [u] idcache [AjPBtcache] id cache
** @param [r] key [const AjPStr] key
** @param [u] btidlist [AjPList] List of matching AjPBtId entries
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ajBtreeKeyFetchId(AjPBtcache cache, AjPBtcache idcache,
                       const AjPStr key, AjPList btidlist)
{
    AjPBtId   id   = NULL;
    AjPList tlist = NULL;
    AjPStr   kwid  = NULL;
    ajulong treeblock = 0L;

    if(ajBtreeKeyFindLen(cache, key,
                         &treeblock))
    {
        tlist = btreeSecTreeList(cache, treeblock);

        while(ajListPop(tlist,(void **)&kwid))
        {
            id = btreeIdentQueryId(idcache, kwid);
            ajDebug("id '%S' entry: %p\n", kwid, id);

            if(id)
            {
                ajDebug("entry id: '%S' dups: %u offset: %Ld\n",
                        id->id, id->dups, id->offset);
                if(!id->dups)
                {
                    ajListPushAppend(btidlist,(void *)id);
                    id = NULL;
                }
                else
                {
                    btreeIdentFetchMulti(idcache, id->id, id->offset,
                                           btidlist);
                    ajBtreeIdDel(&id);
                }
            }

            ajStrDel(&kwid);
        }

        ajListFree(&tlist);
    }
}




/* @func ajBtreeKeyFetchHit ***************************************************
**
** Retrieval of keyword hits with a perfect match to the keyword
**
** @param [u] cache [AjPBtcache] cache
** @param [u] idcache [AjPBtcache] id cache
** @param [r] key [const AjPStr] key
** @param [u] hitlist [AjPList] List of matching AjPBtHit entries
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ajBtreeKeyFetchHit(AjPBtcache cache, AjPBtcache idcache,
                        const AjPStr key, AjPList hitlist)
{
    AjPBtId id  = NULL;
    AjPBtHit hit  = NULL;
    AjPList tlist = NULL;
    AjPStr   kwid  = NULL;
    ajulong treeblock = 0L;

    ajDebug("ajBtreeKeyFetchHit '%S'\n", key);

    if(ajBtreeKeyFindLen(cache, key,
                         &treeblock))
    {
        tlist = btreeSecTreeList(cache, treeblock);

        while(ajListPop(tlist,(void **)&kwid))
        {
            id = btreeIdentQueryId(idcache, kwid);
            ajDebug("id '%S' entry: %p\n", kwid, id);

            if(id)
            {
                ajDebug("entry id: '%S' dups: %u offset: %Ld\n",
                        id->id, id->dups, id->offset);
                if(!id->dups)
                {
                    hit = ajBtreeHitNewId(id);
                    ajListPushAppend(hitlist,(void *)hit);
                    hit = NULL;
                    ajBtreeIdDel(&id);
                }
                else
                {
                    btreeIdentFetchMultiHit(idcache, id->offset,
                                            hitlist);
                    ajBtreeIdDel(&id);
                }
            }

            ajStrDel(&kwid);
        }

        ajListFree(&tlist);
    }
}




/* @func ajBtreeKeyFetchHitref ************************************************
**
** Retrieval of keyword hits with a perfect match to the keyword
**
** @param [u] cache [AjPBtcache] cache
** @param [u] idcache [AjPBtcache] id cache
** @param [r] key [const AjPStr] key
** @param [u] hitlist [AjPList] List of matching AjPBtHitref entries
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ajBtreeKeyFetchHitref(AjPBtcache cache, AjPBtcache idcache,
                           const AjPStr key, AjPList hitlist)
{
    AjPBtId id  = NULL;
    AjPBtHitref hitref = NULL;
    AjPList tlist = NULL;
    AjPStr   kwid  = NULL;
    ajulong treeblock = 0L;

    ajDebug("ajBtreeKeyFetchHitref '%S'\n", key);

    if(ajBtreeKeyFindLen(cache, key,
                         &treeblock))
    {
        tlist = btreeSecTreeList(cache, treeblock);

        while(ajListPop(tlist,(void **)&kwid))
        {
            id = btreeIdentQueryId(idcache, kwid);
            ajDebug("id '%S' entry: %p\n", kwid, id);

            if(id)
            {
                ajDebug("entry id: '%S' dups: %u offset: %Ld\n",
                        id->id, id->dups, id->offset);
                if(!id->dups)
                {
                    hitref = ajBtreeHitrefNewId(id);
                    ajListPushAppend(hitlist,(void *)hitref);
                    hitref = NULL;
                    ajBtreeIdDel(&id);
                }
                else
                {
                    btreeIdentFetchMultiHitref(idcache, id->offset,
                                            hitlist);
                    ajBtreeIdDel(&id);
                }
            }

            ajStrDel(&kwid);
        }

        ajListFree(&tlist);
    }
}




/* @func ajBtreeKeyFetchwildId ************************************************
**
** Wildcard retrieval of keyword index entries
**
** @param [u] cache [AjPBtcache] cache
** @param [u] idcache [AjPBtcache] id cache
** @param [r] key [const AjPStr] key
** @param [u] btidlist [AjPList] List of matching AjPBtId entries
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ajBtreeKeyFetchwildId(AjPBtcache cache, AjPBtcache idcache,
                           const AjPStr key, AjPList btidlist)
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
    
    ajDebug("ajBtreeKeyFetchwildId '%S' list: %Lu\n",
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
	    btreeKeywordFullSearchId(cache,key,idcache,btidlist);

	    return;
	}
    }
    else
	ajStrAssignS(&prefix,key);
    
    ajDebug("ajBtreeKeyFetchwildId prefix '%S' list: %Lu\n",
            prefix, ajListGetLength(btidlist));

    prilist  = ajListNew();
    
    found   = ajFalse;
    
    page = btreePrimaryFetchFindleafWild(cache,prefix);
    page->dirty = BT_LOCK;
    page->lockfor = 1461;
    pripagepossave = page->pagepos;
    btreePrileafFetch(cache,page,prilist);
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
	page = btreePricacheRead(cache,pagepos);
	page->dirty = BT_LOCK;
        page->lockfor = 1462;
	pripagepossave = pagepos;
	btreePrileafFetch(cache,page,prilist);
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
	    page = btreePricacheRead(cache,pripagepossave);

	    buf = page->buf;
	    GBT_RIGHT(buf,&right);

	    if(!right)
	    {
		finished = ajTrue;
		continue;
	    }
	    
	    page->dirty = BT_LOCK;
            page->lockfor = 1463;
	    btreePrileafFetch(cache,page,prilist);
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
	ajListSortUnique(strlist, &ajStrVcmp, &btreeStrDel);

	while(ajListPop(strlist,(void **)&id))
	{
	    btid = btreeIdentQueryId(idcache,id);
            if(btid)
            {
                ajDebug("ajBtreeKeyFetchwildId id: %S btid: '%S'\n",
                        id, btid->id);
                ajListPushAppend(btidlist,(void *)btid);
            }
            else
            {
                ajDebug("ajBtreeKeyFetchwildId id: %S not found\n",
                        id);
            }
	    ajStrDel(&id);
	}
    }

    ajListFree(&strlist);

    ajStrDel(&prefix);

    return;
}




/* @func ajBtreeKeyFetchwildHit ***********************************************
**
** Wildcard retrieval of keyword hit entries
**
** @param [u] cache [AjPBtcache] cache
** @param [u] idcache [AjPBtcache] id cache
** @param [r] key [const AjPStr] key
** @param [u] hitlist [AjPList] List of matching AjPBtHit entries
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ajBtreeKeyFetchwildHit(AjPBtcache cache, AjPBtcache idcache,
                            const AjPStr key, AjPList hitlist)
{
    AjPBtPri pri     = NULL;
    AjPBtpage page   = NULL;
    AjPList prilist  = NULL;
    AjPList strlist  = NULL;

    AjPBtId btid = NULL;
    AjPBtHit hit = NULL;
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

    ajDebug("ajBtreeKeyFetchwildHit '%S' list: %Lu\n",
            key, ajListGetLength(hitlist));

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
	    btreeKeywordFullSearchHit(cache,key,idcache,hitlist);

	    return;
	}
    }
    else
	ajStrAssignS(&prefix,key);
    
    ajDebug("ajBtreeKeyFetchwildHit prefix '%S' list: %Lu\n",
            prefix, ajListGetLength(hitlist));

    prilist  = ajListNew();
    
    found   = ajFalse;
    
    page = btreePrimaryFetchFindleafWild(cache,prefix);
    page->dirty = BT_LOCK;
    page->lockfor = 1461;
    pripagepossave = page->pagepos;
    btreePrileafFetch(cache,page,prilist);
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
	page = btreePricacheRead(cache,pagepos);
	page->dirty = BT_LOCK;
        page->lockfor = 1462;
	pripagepossave = pagepos;
	btreePrileafFetch(cache,page,prilist);
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
	    page = btreePricacheRead(cache,pripagepossave);

	    buf = page->buf;
	    GBT_RIGHT(buf,&right);

	    if(!right)
	    {
		finished = ajTrue;
		continue;
	    }
	    
	    page->dirty = BT_LOCK;
            page->lockfor = 1463;
	    btreePrileafFetch(cache,page,prilist);
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
	ajListSortUnique(strlist, &ajStrVcmp, &btreeStrDel);

	while(ajListPop(strlist,(void **)&id))
	{
	    btid = btreeIdentQueryId(idcache,id);
            if(btid)
            {
                ajDebug("ajBtreeKeyFetchwildHit id: %S btid: '%S'\n",
                        id, btid->id);
                hit = ajBtreeHitNewId(btid);
                ajListPushAppend(hitlist,(void *)hit);
                hit = NULL;
                ajBtreeIdDel(&btid);
            }
            else
            {
                ajDebug("ajBtreeKeyFetchwildHit id: %S not found\n",
                        id);
            }
	    ajStrDel(&id);
	}
    }

    ajListFree(&strlist);

    ajStrDel(&prefix);

    return;
}




/* @func ajBtreeKeyFetchwildHitref ********************************************
**
** Wildcard retrieval of keyword reference hit entries
**
** @param [u] cache [AjPBtcache] cache
** @param [u] idcache [AjPBtcache] id cache
** @param [r] key [const AjPStr] key
** @param [u] hitlist [AjPList] List of matching AjPBtHitref entries
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ajBtreeKeyFetchwildHitref(AjPBtcache cache, AjPBtcache idcache,
                               const AjPStr key, AjPList hitlist)
{
    AjPBtPri pri     = NULL;
    AjPBtpage page   = NULL;
    AjPList prilist  = NULL;
    AjPList strlist  = NULL;

    AjPBtId btid = NULL;
    AjPBtHitref hitref = NULL;
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
    
    ajDebug("ajBtreeKeyFetchwildHitref '%S' list: %Lu\n",
            key, ajListGetLength(hitlist));

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
	    btreeKeywordFullSearchHitref(cache,key,idcache,hitlist);

	    return;
	}
    }
    else
	ajStrAssignS(&prefix,key);
    
    ajDebug("ajBtreeKeyFetchwildHitref prefix '%S' list: %Lu\n",
            prefix, ajListGetLength(hitlist));

    prilist  = ajListNew();
    
    found   = ajFalse;
    
    page = btreePrimaryFetchFindleafWild(cache,prefix);
    page->dirty = BT_LOCK;
    page->lockfor = 1461;
    pripagepossave = page->pagepos;
    btreePrileafFetch(cache,page,prilist);
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
	page = btreePricacheRead(cache,pagepos);
	page->dirty = BT_LOCK;
        page->lockfor = 1462;
	pripagepossave = pagepos;
	btreePrileafFetch(cache,page,prilist);
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
	    page = btreePricacheRead(cache,pripagepossave);

	    buf = page->buf;
	    GBT_RIGHT(buf,&right);

	    if(!right)
	    {
		finished = ajTrue;
		continue;
	    }
	    
	    page->dirty = BT_LOCK;
            page->lockfor = 1463;
	    btreePrileafFetch(cache,page,prilist);
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
	ajListSortUnique(strlist, &ajStrVcmp, &btreeStrDel);

	while(ajListPop(strlist,(void **)&id))
	{
	    btid = btreeIdentQueryId(idcache,id);
            if(btid)
            {
                ajDebug("ajBtreeKeyFetchwildHitref id: %S btid: '%S'\n",
                        id, btid->id);
                hitref = ajBtreeHitrefNewId(btid);
                ajListPushAppend(hitlist,(void *)hitref);
                hitref = NULL;
                ajBtreeIdDel(&btid);
            }
            else
            {
                ajDebug("ajBtreeKeyFetchwildHitref id: %S not found\n",
                        id);
            }
	    ajStrDel(&id);
	}
    }

    ajListFree(&strlist);

    ajStrDel(&prefix);

    return;
}




/* @funcstatic btreeKeywordFullSearchId ***************************************
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
**
** @release 3.0.0
** @@
******************************************************************************/

static void btreeKeywordFullSearchId(AjPBtcache cache, const AjPStr key,
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

    ajulong nids = 0;
    ajulong i;

    AjPTable strtable = NULL;
    AjPStr *idarray = NULL;

    list    = ajListNew();
    strlist = ajListNew();
    strtable = ajTablestrNew(idcache->countunique);
    
    root = btreePricacheLocate(cache, 0L);
    page = root;
    
    buf = root->buf;
    GBT_NODETYPE(buf,&nodetype);
    
    if(cache->plevel)
    {
        arrays = btreeAllocPriArray(cache);
        parray = arrays->parray;
	while(nodetype != BT_LEAF)
	{
	    btreeGetPointers(cache,buf,&parray);
	    page = btreePricacheRead(cache, parray[0]);
	    buf = page->buf;
	    GBT_NODETYPE(buf,&nodetype);
	    page->dirty = BT_CLEAN;
	}
        btreeDeallocPriArray(cache,arrays);
    }

    right = 99L;

    while(right)
    {
	btreePrileafFetch(cache,page,list);

	while(ajListPop(list,(void **)&pri))
	{
	    if(ajStrMatchWildS(pri->keyword,key))
	    {
		cache->secrootblock = pri->treeblock;
		btreeReadAllSecLeaves(cache,strlist);

                while(ajListPop(strlist,(void **)&id))
                    ajTablePut(strtable, id, NULL);
	    }

	    ajBtreePriDel(&pri);
	}

	GBT_RIGHT(buf,&right);

	if(right)
	{
	    page = btreePricacheRead(cache,right);
	    buf = page->buf;
	}
    }

    if(ajTableGetLength(strtable))
    {
        nids = ajTableToarrayKeys(strtable, (void***)&idarray);

        for(i=0; i < nids; i++)
	{
	    btid = btreeIdentQueryId(idcache,idarray[i]);
            if(btid)
                ajListPushAppend(idlist,(void *)btid);
	}
    }

    AJFREE(idarray);
    ajListFree(&list);
    ajListFree(&strlist);
    ajTableDel(&strtable);

    return;
}




/* @funcstatic btreeKeywordFullSearchHit **************************************
**
** Wildcard retrieval of key/des/org entries. Whole index scan. Only used for
** wildcard searches with keys beginning with '?' or '*'
**
** @param [u] cache [AjPBtcache] cache
** @param [r] key [const AjPStr] Wildcard key
** @param [u] idcache [AjPBtcache] id index cache
** @param [u] hitlist [AjPList] list of matching AjPBtHits
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void btreeKeywordFullSearchHit(AjPBtcache cache, const AjPStr key,
                                      AjPBtcache idcache, AjPList hitlist)
{
    AjPBtPri pri   = NULL;
    AjPBtpage root = NULL;
    AjPBtpage page = NULL;
    AjPBtHit  hit  = NULL;
    ajulong right   = 0L;
    ajuint nodetype = 0;
    
    AjPBtMem arrays = NULL;
    ajulong *parray = NULL;

    AjPList list   = NULL;
    AjPList strlist = NULL;
    AjPStr id = NULL;

    unsigned char *buf = NULL;

    ajulong nids = 0;
    ajulong i;

    AjPTable strtable = NULL;
    AjPStr *idarray = NULL;

    list    = ajListNew();
    strlist = ajListNew();
    strtable = ajTablestrNew(idcache->countunique);

    root = btreePricacheLocate(cache, 0L);
    page = root;
    
    buf = root->buf;
    GBT_NODETYPE(buf,&nodetype);
    
    if(cache->plevel)
    {
        arrays = btreeAllocPriArray(cache);
        parray = arrays->parray;
	while(nodetype != BT_LEAF)
	{
	    btreeGetPointers(cache,buf,&parray);
	    page = btreePricacheRead(cache, parray[0]);
	    buf = page->buf;
	    GBT_NODETYPE(buf,&nodetype);
	    page->dirty = BT_CLEAN;
	}
        btreeDeallocPriArray(cache,arrays);
    }

    right = 99L;

    while(right)
    {
	btreePrileafFetch(cache,page,list);

	while(ajListPop(list,(void **)&pri))
	{
	    if(ajStrMatchWildS(pri->keyword,key))
	    {
		cache->secrootblock = pri->treeblock;
		btreeReadAllSecLeaves(cache,strlist);

                while(ajListPop(strlist,(void **)&id))
                    ajTablePut(strtable, id, NULL);
            }

	    ajBtreePriDel(&pri);
	}

	GBT_RIGHT(buf,&right);

	if(right)
	{
	    page = btreePricacheRead(cache,right);
	    buf = page->buf;
	}
    }

    if(ajTableGetLength(strtable))
    {
        nids = ajTableToarrayKeys(strtable, (void***)&idarray);
        for(i=0; i < nids; i++)
	{
	    hit = btreeIdentQueryHit(idcache,idarray[i]);
	    if(hit)
                ajListPushAppend(hitlist,(void *)hit);
	}
    }

    AJFREE(idarray);
    ajListFree(&list);
    ajListFree(&strlist);
    ajTableDel(&strtable);

    return;
}




/* @funcstatic btreeKeywordFullSearchHitref ***********************************
**
** Wildcard retrieval of key/des/org entries. Whole index scan. Only used for
** wildcard searches with keys beginning with '?' or '*'
**
** @param [u] cache [AjPBtcache] cache
** @param [r] key [const AjPStr] Wildcard key
** @param [u] idcache [AjPBtcache] id index cache
** @param [u] hitlist [AjPList] list of matching AjPBtHitrefs
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void btreeKeywordFullSearchHitref(AjPBtcache cache, const AjPStr key,
                                         AjPBtcache idcache, AjPList hitlist)
{
    AjPBtPri pri   = NULL;
    AjPBtpage root = NULL;
    AjPBtpage page = NULL;
    AjPBtHitref hitref = NULL;
    ajulong right   = 0L;
    ajuint nodetype = 0;
    
    AjPBtMem arrays = NULL;
    ajulong *parray = NULL;

    AjPList list   = NULL;
    AjPList strlist = NULL;
    AjPStr id = NULL;
    
    unsigned char *buf = NULL;    

    ajulong nids = 0;
    ajulong i;

    AjPTable strtable = NULL;
    AjPStr *idarray = NULL;

    list    = ajListNew();
    strlist = ajListNew();
    strtable = ajTablestrNew(idcache->countunique);
    
    root = btreePricacheLocate(cache, 0L);
    page = root;
    
    buf = root->buf;
    GBT_NODETYPE(buf,&nodetype);
    
    if(cache->plevel)
    {
        arrays = btreeAllocPriArray(cache);
        parray = arrays->parray;
	while(nodetype != BT_LEAF)
	{
	    btreeGetPointers(cache,buf,&parray);
	    page = btreePricacheRead(cache, parray[0]);
	    buf = page->buf;
	    GBT_NODETYPE(buf,&nodetype);
	    page->dirty = BT_CLEAN;
	}
        btreeDeallocPriArray(cache,arrays);
    }

    right = 99L;

    while(right)
    {
	btreePrileafFetch(cache,page,list);

	while(ajListPop(list,(void **)&pri))
	{
	    if(ajStrMatchWildS(pri->keyword,key))
	    {
		cache->secrootblock = pri->treeblock;
		btreeReadAllSecLeaves(cache,strlist);

                while(ajListPop(strlist,(void **)&id))
                    ajTablePut(strtable, id, NULL);
	    }

	    ajBtreePriDel(&pri);
	}

	GBT_RIGHT(buf,&right);

	if(right)
	{
	    page = btreePricacheRead(cache,right);
	    buf = page->buf;
	}
    }


    if(ajTableGetLength(strtable))
    {
        nids = ajTableToarrayKeys(strtable, (void***)&idarray);

        for(i=0; i < nids; i++)
	{
	    hitref = btreeIdentQueryHitref(idcache,idarray[i]);
            if(hitref)
                ajListPushAppend(hitlist,(void *)hitref);
	}
    }

    AJFREE(idarray);
    ajListFree(&list);
    ajListFree(&strlist);
    ajTableDel(&strtable);

    return;
}




/* @funcstatic btreePrileafFetch **********************************************
**
** Read all primary index leaf keywords into a list
**
** @param [u] cache [AjPBtcache] cache
** @param [u] page [AjPBtpage] page
** @param [w] list [AjPList] list of AjPBtPri objects
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void btreePrileafFetch(AjPBtcache cache, AjPBtpage page, AjPList list)
{
    unsigned char *buf = NULL;
    AjPBtMem arrays    = NULL;
    ajulong *parray     = NULL;

    ajuint keylimit = 0;
    ajuint nkeys    = 0;
    
    ajuint i;
    
    /* ajDebug("In PrileafFetch\n"); */
    
    buf = page->buf;

    arrays = btreeAllocPriArray(cache);
    parray = arrays->parray;

    btreeGetPointers(cache,buf,&parray);

    GBT_NKEYS(buf,&nkeys);
    
    keylimit = nkeys+1;

    for(i=0;i<keylimit;++i)
	btreePribucketIdlist(cache,parray[i],list);

    ajListSort(list, &btreeKeywordCompare);

    btreeDeallocPriArray(cache,arrays);

    return;
}




/* @funcstatic btreeSecLeftLeaf ***********************************************
**
** Read all secondary index leaf IDs into a list from the lefthand-most
** leaf or the root node if the level is 0.
**
** @param [u] cache [AjPBtcache] cache
** @param [u] wild [AjPBtKeywild] wildcard keyword object
**
** @return [void]
**
** @release 3.0.0
** @@
******************************************************************************/

static void btreeSecLeftLeaf(AjPBtcache cache, AjPBtKeywild wild)
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
    

    root = btreeSeccacheRead(cache, cache->secrootblock);
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
	while(nodetype != BT_SECLEAF)
	{
	    btreeGetPointers(cache,buf,&parray);
	    page = btreeSeccacheRead(cache, parray[0]);
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
	btreeSecbucketIdlist(cache,parray[i],wild->idlist);

    ajListSort(wild->idlist, &ajStrVcmp);
    
    root->dirty = BT_CLEAN;
    
    btreeDeallocSecArray(cache,arrays);

    cache->secrootblock = 0L;

    return;
}




/* @funcstatic btreeKeywildNextList *******************************************
**
** Get next wadge of secondary index leaf IDs into a list after a successful
** wildcard keyword search
**
** @param [u] cache [AjPBtcache] cache
** @param [u] wild [AjPBtKeywild] wildcard keyword object
**
** @return [AjBool] true if a wadge was successfully read
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool btreeKeywildNextList(AjPBtcache cache, AjPBtKeywild wild)
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

    
    page = btreeSeccacheRead(cache,wild->secpagepos);
    buf = page->buf;
    GBT_RIGHT(buf,&right);
    page->dirty = BT_CLEAN;

    if(!right)
	return ajFalse;

    page = btreeSeccacheRead(cache,right);
    wild->secpagepos = right;

    arrays = btreeAllocSecArray(cache);
    parray = arrays->parray;

    btreeGetPointers(cache,buf,&parray);
    GBT_NKEYS(buf,&nkeys);

    keylimit = nkeys + 1;
    
    for(i=0;i<keylimit;++i)
	btreeSecbucketIdlist(cache,parray[i],wild->idlist);
    
    ajListSort(wild->idlist, &ajStrVcmp);
    
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
**
** @release 3.0.0
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
    

    root = btreeSeccacheRead(cache,cache->secrootblock);
    buf = root->buf;
    GBT_NODETYPE(buf,&nodetype);

    if(nodetype == BT_SECBUCKET)
    {
        btreeSecbucketIdlist(cache, root->pagepos, list);
        return;
    }

    root->dirty = BT_LOCK;
    root->lockfor = 1481;
    GBT_RIGHT(buf,&right);
    cache->slevel = (ajuint) right;

    arrays = btreeAllocSecArray(cache);
    parray = arrays->parray;

    if(cache->slevel)
    {
	while(nodetype != BT_SECLEAF)
	{
	    btreeGetPointers(cache,buf,&parray);
	    page = btreeSeccacheRead(cache,parray[0]);
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
	btreeSecbucketIdlist(cache,parray[i],list);

    if(cache->sorder)
    {
	page = btreeSeccacheRead(cache,secpagepos);
	buf  = page->buf;
	GBT_RIGHT(buf,&right);
	page->dirty = BT_CLEAN;
    }


    while(right && secpagepos != cache->secrootblock)
    {
	secpagepos = right;
	page = btreeSeccacheRead(cache,secpagepos);
	page->dirty = BT_LOCK;
        page->lockfor = 1482;
	buf = page->buf;

	btreeGetPointers(cache,buf,&parray);
	GBT_NKEYS(buf,&nkeys);
	keylimit = nkeys + 1;
	for(i=0; i < keylimit;++i)
            btreeSecbucketIdlist(cache,parray[i],list);

	GBT_RIGHT(buf,&right);
	page->dirty = BT_CLEAN;
    }


    root->dirty = BT_CLEAN;

    btreeDeallocSecArray(cache,arrays);

    return;
}




/* @funcstatic btreeStrDel ****************************************************
**
** Deletes an AjPStr entry from a list
**
** @param [r] Pstr [void**] Address of an AjPStr object
** @param [r] cl [void*] Standard unused argument, usually NULL.
** @return [void]
**
** @release 3.0.0
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




/* @funcstatic btreeIdDelFromList *********************************************
**
** Deletes an AjPBtId entry from a list
**
** @param [r] pentry [void**] Address of an AjPBtId object
** @param [r] cl [void*] Standard unused argument, usually NULL.
** @return [void]
**
** @release 3.0.0
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




/* @funcstatic btreeHitDelFromList ********************************************
**
** Deletes an AjPBtHit entry from a list
**
** @param [r] pentry [void**] Address of an AjPBtHit object
** @param [r] cl [void*] Standard unused argument, usually NULL.
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void btreeHitDelFromList(void** pentry, void* cl)
{
    AjPBtHit hit = NULL;

    (void) cl;				/* make it used */

    hit = *((AjPBtHit *)pentry);

    ajBtreeHitDel(&hit);

    return;
}




/* @funcstatic btreeHitrefDelFromList *****************************************
**
** Deletes an AjPBtHitref entry from a list
**
** @param [r] pentry [void**] Address of an AjPBtHitref object
** @param [r] cl [void*] Standard unused argument, usually NULL.
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void btreeHitrefDelFromList(void** pentry, void* cl)
{
    AjPBtHitref hitref = NULL;

    (void) cl;				/* make it used */

    hitref = *((AjPBtHitref *)pentry);

    ajBtreeHitrefDel(&hitref);

    return;
}




/* @funcstatic btreeIdOffsetCompare *******************************************
**
** Comparison function for ajListUnique2
**
** @param [r] a [const void*] ID 1
** @param [r] b [const void*] ID 2
**
** @return [ajint] 0 = bases match
**
** @release 6.5.0
** @@
******************************************************************************/

static ajint btreeIdOffsetCompare(const void *a, const void *b)
{
    ajlong val;

    val = (ajlong) ((*(AjPBtId const *)a)->offset -
                    (*(AjPBtId const *)b)->offset);

    if(!val)
      return 0;

    return (val < 0L) ? -1 : 1;
}




/* @funcstatic btreeHitOffsetCompare ******************************************
**
** Comparison function for ajListUnique2
**
** @param [r] a [const void*] ID 1
** @param [r] b [const void*] ID 2
**
** @return [ajint] 0 = offsets match
**
** @release 6.5.0
** @@
******************************************************************************/

static ajint btreeHitOffsetCompare(const void *a, const void *b)
{
    ajlong val;

    val = (ajlong) ((*(AjPBtHit const *)a)->offset -
                    (*(AjPBtHit const *)b)->offset);

    if(!val)
      return 0;

    return (val < 0L) ? -1 : 1;
}




/* @funcstatic btreeHitrefOffsetCompare ***************************************
**
** Comparison function for ajListUnique2
**
** @param [r] a [const void*] ID 1
** @param [r] b [const void*] ID 2
**
** @return [ajint] 0 = offsets match
**
** @release 6.5.0
** @@
******************************************************************************/

static ajint btreeHitrefOffsetCompare(const void *a, const void *b)
{
    ajlong val;

    val = (ajlong) ((*(AjPBtHitref const *)a)->offset -
                    (*(AjPBtHitref const *)b)->offset);

    if(!val)
      return 0;

    return (val < 0L) ? -1 : 1;
}




/* @funcstatic btreeHitDbnoCompare ********************************************
**
** Second comparison function for ajListUnique2
**
** @param [r] a [const void*] ID 1
** @param [r] b [const void*] ID 2
**
** @return [ajint] 0 = values match
**
** @release 6.5.0
** @@
******************************************************************************/

static ajint btreeHitDbnoCompare(const void *a, const void *b)
{
    return (ajuint) (*(AjPBtHit const *)a)->dbno -
           (ajuint) (*(AjPBtHit const *)b)->dbno;
}




/* @funcstatic btreeHitrefDbnoCompare *****************************************
**
** Second comparison function for ajListUnique2
**
** @param [r] a [const void*] ID 1
** @param [r] b [const void*] ID 2
**
** @return [ajint] 0 = values match
**
** @release 6.5.0
** @@
******************************************************************************/

static ajint btreeHitrefDbnoCompare(const void *a, const void *b)
{
    return (ajuint) (*(AjPBtHitref const *)a)->dbno -
           (ajuint) (*(AjPBtHitref const *)b)->dbno;
}




/* @funcstatic btreeIdDbnoCompare *********************************************
**
** Second comparison function for ajListUnique2
**
** @param [r] a [const void*] ID 1
** @param [r] b [const void*] ID 2
**
** @return [ajint] 0 = values match
**
** @release 6.5.0
** @@
******************************************************************************/

static ajint btreeIdDbnoCompare(const void *a, const void *b)
{
    return (ajuint) (*(AjPBtId const *)a)->dbno -
           (ajuint) (*(AjPBtId const *)b)->dbno;
}




#if 0
/* #func ajBtreeHybNew ********************************************************
**
** Constructor for index bucket ID information
**
** #param [r] refcount [ajuint] Number of reference file(s) per entry
** #return [AjPBtHybrid] Index ID object
**
** #release 4.0.0
** ##
******************************************************************************/
/*
AjPBtHybrid ajBtreeHybNew(ajuint refcount)
{
    AjPBtHybrid Id = NULL;

    /# ajDebug("In ajBtreeHybNew\n"); #/

    AJNEW0(Id);
    Id->key1 = ajStrNew();
    Id->dbno = 0;
    Id->dups = 0;
    Id->offset = 0L;
    Id->refcount = refcount;

    if(refcount)
        AJCNEW(Id->refoffsets, refcount);

    Id->treeblock = 0L;

    return Id;
}
*/
#endif




#if 0
/* #func ajBtreeHybDel ********************************************************
**
** Destructor for index bucket ID information
**
** #param [w] Pthis [AjPBtHybrid*] index ID object
**
** #return [void]
**
** #release 4.0.0
** ##
******************************************************************************/
/*
void ajBtreeHybDel(AjPBtHybrid *Pthis)
{
    AjPBtHybrid Id = NULL;

    /# ajDebug("In ajBtreeIdDel\n"); #/

    if(!Pthis || !*Pthis)
	return;

    Id = *Pthis;
    
    ajStrDel(&Id->key1);

    if(Id->refcount)
        AJFREE(Id->refoffsets);

    AJFREE(Id);

    *Pthis = NULL;

    return;
}
*/
#endif




/* @funcstatic btreeAllocPriArray *********************************************
**
** Allocate karray and parray arrays for a primary key
**
** @param [u] cache [AjPBtcache] cache
**
** @return [AjPBtMem] memory node
**
** @release 4.0.0
** @@
******************************************************************************/

static AjPBtMem btreeAllocPriArray(AjPBtcache cache)
{
    AjPBtMem node = NULL;
    ajuint i;
    ajuint limit;
    AjPBtMem p = NULL;
    ajuint klen;

    limit = cache->porder;

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

        klen = cache->keylimit+1;
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

    klen = cache->keylimit+1;
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
**
** @release 4.0.0
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




/* @funcstatic btreeAllocSecArray *********************************************
**
** Allocate karray and parray arrays for a primary key
**
** @param [u] cache [AjPBtcache] cache
**
** @return [AjPBtMem] memory node
**
** @release 4.0.0
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

        klen = cache->idlimit + 1;
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

    klen = cache->idlimit + 1;
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
**
** @release 4.0.0
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




#if 0
/* #funcstatic btreeHybbucketAdd **********************************************
**
** Add a hybrid ID to a bucket
** Only called if there is room in the bucket
**
** Use btreeIdbucketAdd using an ID object which is the same with no
** (unused) treeblock.
**
** #param [u] cache [AjPBtcache] cache
** #param [r] pagepos [ajulong] page number of bucket
** #param [r] hyb [const AjPBtHybrid] ID info
**
** #return [void]
**
** #release 4.0.0
** ##
******************************************************************************/
/*
static void btreeHybbucketAdd(AjPBtcache cache, ajulong pagepos,
                              const AjPBtHybrid hyb)
{
    unsigned char *buf  = NULL;
    unsigned char *kptr = NULL;
    unsigned char *src  = NULL;
    unsigned char *dest = NULL;

/#    unsigned char *lastptr = NULL;#/
    unsigned char *endptr  = NULL;
    
    ajuint nentries = 0;
    ajuint nodetype = 0;
    
    ajuint sum = 0;
    ajuint len = 0;
    ajuint i;
    ajuint v;
    ajuint uv;
    ajulong lv;
    ajuint iref = 0;

    AjPBtpage page = NULL;
    static ajuint calls = 0;
    ajuint refskip = cache->refcount*BT_EXTRA;

/#    ajuint keyskip = BT_DDOFF + cache->refcount*BT_EXTRA + 1;#/
/#    ajuint idlen    = 0;#/
/#    static ajuint overflowcalls=0;#/
   
    calls++;

    page = btreePricacheRead(cache,pagepos);
    buf  = page->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);
    if(nodetype != BT_IDBUCKET)
        ajFatal("Wrong nodetype in HybbucketAdd cache %S", cache->filename);

    GBT_BUCKNENTRIES(buf,&nentries);
    if(nentries == cache->pnperbucket)
        ajFatal("Bucket too full in HybbucketAdd page: %Lu "
                "entries: %u max: %u cache %S",
                pagepos, nentries, cache->pnperbucket, cache->filename);

    kptr = PBT_BUCKKEYLEN(buf);
    src  = kptr + (nentries * sizeof(ajuint));

    sum = 0;
    for(i=0;i<nentries;++i)
    {
        BT_GETAJUINT(kptr,&len);
        sum += len;
        kptr += sizeof(ajuint);
    }
    /#sum += nentries;#/
    
    endptr  = src + sum - 1;

/#
//    idlen   = MAJSTRGETLEN(hyb->key1);
//    lastptr = endptr + sizeof(ajuint) + idlen + keyskip;
//
//    if((ajuint) (lastptr - buf) >= cache->pagesize)
//    {
//        overflowcalls++;
//        ajWarn("\nOverflow in HybbucketAdd nentries:%u fails %u/%u '%S' "
//               "cache %S",
//               nentries, overflowcalls,calls, hyb->key1, cache->filename);
//        btreeHybbucketAddFull(cache,pagepos,hyb);
//        return;
//    }
#/    

    dest = src + sizeof(ajuint);
    memmove((void *)dest, (void *)src, sum);

    v = BT_BUCKIDLEN(hyb->key1) + refskip;
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

    if(cache->refcount)
    {
        for(iref=0; iref < cache->refcount; iref++)
        {
            lv = hyb->refoffsets[iref];
            BT_SETAJULONG(endptr,lv);
            endptr += sizeof(ajulong);
        }
    }

    v = nentries + 1;
    SBT_BUCKNENTRIES(buf,v);

    page->dirty = BT_DIRTY;

    return;
}
*/
#endif




#if 0
/* #funcstatic btreeHybbucketAddFull ******************************************
**
** Add an ID to a bucket
** Only called if there is room in the bucket
**
** #param [u] cache [AjPBtcache] cache
** #param [r] pagepos [ajulong] page number of bucket
** #param [r] hyb [const AjPBtHybrid] ID info
**
** #return [void]
**
** #release 6.4.0
** ##
******************************************************************************/
/*
//static void btreeHybbucketAddFull(AjPBtcache cache, ajulong pagepos,
//                                  const AjPBtHybrid hyb)
//{
//    AjPIdbucket bucket = NULL;
//    AjPBtId   destid = NULL;
//    
//    ajuint nentries;
//    ajuint iref;
//    
//    /# ajDebug("In btreeHybbucketAddFull\n"); #/
//
//    bucket   = btreeReadIdbucket(cache,pagepos);
//    nentries = bucket->Nentries;
//
//
//    /# Reading a bucket always gives one extra ID position #/
//    destid = bucket->Ids[nentries];
//
//    ajStrAssignS(&destid->id,hyb->key1);
//    destid->dbno      = hyb->dbno;
//    destid->dups      = hyb->dups;
//    destid->offset    = hyb->offset;
//
//    if(cache->refcount)
//    {
//        for(iref=0; iref < cache->refcount; iref++)
//            destid->refoffsets[iref] = hyb->refoffsets[iref];
//    }
//
//    ++bucket->Nentries;
//
//    btreeWriteIdbucket(cache,bucket,pagepos);
//
//    btreeIdbucketDel(&bucket);
//    
//    return;
//}
*/
#endif




/* @funcstatic btreeIdentFind *************************************************
**
** Find the node that should contain a new key for insertion
**
** @param [u] cache [AjPBtcache] cache
** @param [r] key [const AjPStr] key to search for 
**
** @return [AjPBtpage] leaf node where item should be inserted
**
** @release 4.0.0
** @@
******************************************************************************/

static AjPBtpage btreeIdentFind(AjPBtcache cache, const AjPStr key)
{
    AjPBtpage root = NULL;
    AjPBtpage ret  = NULL;

    /* ajDebug("In btreeIdentFind\n"); */

    /* The root node should always be in the cache (BT_LOCKed) */
    root = btreePricacheLocate(cache,0L);

    if(!root)
	ajFatal("The master root cache page has been unlocked\n");
    
    if(!cache->plevel)
	return root;
    
    ret = btreePrimaryFindInode(cache,root,key);
    
    return ret;
}




/* @funcstatic btreeIdentInsertShift ******************************************
**
** Rebalance buckets on insertion of an identifier
**
** @param [u] cache [AjPBtcache] cache
** @param [u] retpage [AjPBtpage*] page
** @param [r] key [const AjPStr] key
**
** @return [ajulong] bucket block or 0L if shift not possible
**
** @release 6.5.0
** @@
******************************************************************************/

static ajulong btreeIdentInsertShift(AjPBtcache cache, AjPBtpage *retpage,
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
    ajDebug("btreeIdentInsertShift %Lu parent: %Lu '%S'\n",
            (*retpage)->pagepos, parent, key);
#endif

    order = cache->porder;
    minsize = order / 2;

    if(order % 2)
	++minsize;

    if(tkeys <= minsize)
	return 0L;

    ppage = btreePricacheRead(cache,parent);
    
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
	spage = btreePricacheRead(cache,pParray[ppos-1]);
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
	spage = btreePricacheRead(cache,pParray[ppos+1]);
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




#if 0
/* #funcstatic btreeHybbucketsReorder *****************************************
**
** Re-order leaf buckets
** Must only be called if one of the buckets is full
**
** #param [u] cache [AjPBtcache] cache
** #param [u] leaf [AjPBtpage] leaf page
**
** #return [AjBool] true if reorder was successful i.e. leaf not full
**
** #release 4.0.0
** ##
******************************************************************************/
/*
static AjBool btreeHybbucketsReorder(AjPBtcache cache, AjPBtpage leaf)
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
    ajuint iref = 0;

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
    AjPIdbucket cbucket = NULL;
    AjPBtId cid       = NULL;

    ajuint iold = 0;
    ajuint refskip = cache->refcount*BT_EXTRA;

#if AJINDEX_DEBUG
    ajDebug("btreeHybucketsReorder %Lu\n", leaf->pagepos);
#endif

    /# ajDebug("In btreeHybbucketsReorder\n"); #/

    dirtysave = leaf->dirty;

    leaf->dirty = BT_LOCK;
    leaf->lockfor = 1511;
    lbuf = leaf->buf;

    GBT_NODETYPE(lbuf,&nodetype);

    order = cache->porder;
    

    /# Read keys/ptrs #/
    arrays    = btreeAllocPriArray(cache);
    ptrs      = arrays->parray;
    newarrays    = btreeAllocPriArray(cache);
    newkeys      = newarrays->karray;
    newptrs      = newarrays->parray;
    overflows = newarrays->overflows;

    btreeGetPointers(cache,lbuf,&ptrs);

    GBT_NKEYS(lbuf,&nkeys);


    if(!nkeys)
	ajFatal("HybbucketsReorder: Attempt to reorder empty leaf");

    for(i=0;i<=nkeys;++i)
	totalkeys += btreeIdbucketCount(cache,ptrs[i]);

    keylimit = nkeys + 1;

    btreeBucketCalc(totalkeys, keylimit, cache->pnperbucket,
                    &bucketlimit, &maxnperbucket);


    if(bucketlimit >= order)
    {
	btreeDeallocPriArray(cache,arrays);
	btreeDeallocPriArray(cache,newarrays);

	leaf->dirty = dirtysave;
	return ajFalse;
    }

    ++statCallHybbucketsReorder;

    /# Read buckets #/

    idlist  = ajListNew();

    for(i=0;i<keylimit;++i)
	overflows[i] = btreeHybbucketIdlist(cache,ptrs[i],idlist);

    ajListSort(idlist, &btreeIdCompare);

    cbucket = btreeIdbucketNew(cache->pnperbucket, cache->refcount);

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

            if(cache->refcount)
            {
                for(iref=0; iref< cache->refcount; iref++)
                    cid->refoffsets[iref] = bid->refoffsets[iref];
	    }

	    cbucket->keylen[count] = BT_BUCKIDLEN(bid->id) + refskip;
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
	btreeWriteIdbucket(cache,cbucket,newptrs[i]);
    }


    /# Deal with greater-than bucket #/

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

        if(cache->refcount)
        {
            for(iref=0; iref < cache->refcount; iref++)
                cid->refoffsets[iref] = bid->refoffsets[iref];
	}

	++cbucket->Nentries;
	++count;
	ajBtreeIdDel(&bid);
    }
    
    
    if((iold < order) && ptrs[iold])
        newptrs[i] = ptrs[iold++];
    else
        newptrs[i] = cache->totsize;
    btreeWriteIdbucket(cache,cbucket,newptrs[i]);

    btreeIdbucketDel(&cbucket);

    /# Now write out a modified leaf with new keys/ptrs #/

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
*/
#endif




/* @funcstatic btreeIdSplitleaf **********************************************
**
** Split a leaf and propagate up if necessary
**
** @param [u] cache [AjPBtcache] cache
** @param [u] spage [AjPBtpage] page
**
** @return [AjPBtpage] pointer to a parent page
**
** @release 4.0.0
** @@
******************************************************************************/

static AjPBtpage btreeIdSplitleaf(AjPBtcache cache, AjPBtpage spage)
{
    ajuint nkeys     = 0;
    ajuint order     = 0;
    ajuint totalkeys = 0;
    ajuint keylimit  = 0;
    ajuint nodetype  = 0;

    ajuint rootnodetype  = 0;
    
    ajuint i;
    ajuint j;
    ajuint iref;

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

    AjPIdbucket cbucket  = NULL;
    
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
    ajuint refskip = cache->refcount*BT_EXTRA;

#if AJINDEX_DEBUG
    ajDebug("btreeIdSplitleaf %Lu\n", spage->pagepos);
#endif
    ++statCallIdSplitleaf;

    /*ajDebug("In btreeIdSplitleaf page:%Lu\n", spage->pagepos);*/

    order = cache->porder;

    arrays    = btreeAllocPriArray(cache);
    parray    = arrays->parray;

    newarrays    = btreeAllocPriArray(cache);
    newkarray    = newarrays->karray;
    newparray    = newarrays->parray;

    buf = spage->buf;
    lbuf = buf;

    GBT_NKEYS(buf,&nkeys);
    GBT_NODETYPE(buf,&rootnodetype);

    /*ajDebug("btreeIdSplitleaf start page %Lu nkeys:%u\n",
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
	lpage = btreePricacheNodenew(cache);
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
    rpage = btreePricacheNodenew(cache);
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
        btreeIdbucketIdlist(cache,parray[i],idlist);
    
    ajListSort(idlist, &btreeIdCompare);

    totalkeys = (ajuint) ajListGetLength(idlist);

    btreeBucketSplitCalc(totalkeys, keylimit, cache->pnperbucket,
                         &lbucketlimit,&lmaxnperbucket,&lno,
                         &rbucketlimit,&rmaxnperbucket,&rno);

    cbucket = btreeIdbucketNew(cache->pnperbucket, cache->refcount);

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

            if(cache->refcount)
            {
                for(iref=0; iref < cache->refcount; iref++)
                    cid->refoffsets[iref] = bid->refoffsets[iref];
	    }

	    cbucket->keylen[j] = BT_BUCKIDLEN(bid->id) + refskip;
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
	btreeWriteIdbucket(cache,cbucket,newparray[i]);
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

        if(cache->refcount)
        {
            for(iref=0; iref < cache->refcount; iref++)
                cid->refoffsets[iref] = bid->refoffsets[iref];
	}

	++cbucket->Nentries;
	ajBtreeIdDel(&bid);
    }

    if(iold < keylimit)
        newparray[i] = parray[iold++];
    else
        newparray[i] = cache->totsize;
    btreeWriteIdbucket(cache,cbucket,newparray[i]);

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

            if(cache->refcount)
            {
                for(iref=0; iref < cache->refcount; iref++)
                    cid->refoffsets[iref] = bid->refoffsets[iref];
	    }

	    cbucket->keylen[j] = BT_BUCKIDLEN(bid->id) + refskip;
	    ++cbucket->Nentries;
	    ajBtreeIdDel(&bid);
	}

	ajListPeek(idlist,(void **)&bid);
	ajStrAssignS(&newkarray[i],bid->id);

	if(iold < keylimit)
            newparray[i] = parray[iold++];
        else
            newparray[i] = cache->totsize;
	btreeWriteIdbucket(cache,cbucket,newparray[i]);
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

        if(cache->refcount)
        {
            for(iref=0; iref < cache->refcount; iref++)
                cid->refoffsets[iref] = bid->refoffsets[iref];
	}

	++cbucket->Nentries;
	ajBtreeIdDel(&bid);
    }
    
    if(iold < keylimit)
        newparray[i] = parray[iold++];
    else
        newparray[i] = cache->totsize;
    btreeWriteIdbucket(cache,cbucket,newparray[i]);

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

    btreeIdbucketDel(&cbucket);
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
	++cache->plevel;

	return spage;
    }


    page = btreePricacheRead(cache,prevsave);
    btreeIdInsertKey(cache,page,mediankey,medianless,mediangtr);
    ajStrDel(&mediankey);

    page = btreePricacheRead(cache,prevsave);

    return page;
}




/* @funcstatic btreeIdInsertKey ***********************************************
**
** Insert an identifier into a potentially full node
**
** @param [u] cache [AjPBtcache] cache
** @param [u] page [AjPBtpage] original page
** @param [r] key [const AjPStr] key to insert
** @param [r] less [ajulong] less-than pointer
** @param [r] greater [ajulong] greater-than pointer
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void btreeIdInsertKey(AjPBtcache cache, AjPBtpage page,
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
	btreePrimaryInsertNonfull(cache,page,key,less,greater);

	return;
    }
    
    order = cache->porder;
    lbuf = page->buf;
    GBT_NODETYPE(lbuf,&nodetype);
    page->dirty = BT_LOCK;
    page->lockfor = 1531;

    if(nodetype == BT_ROOT)
    {
	arrays1   = btreeAllocPriArray(cache);
	karray    = arrays1->karray;
	parray    = arrays1->parray;

	btreeIdSplitroot(cache);

	if(page->pagepos)
	    page->dirty = BT_DIRTY;

	btreeGetKeys(cache,lbuf,&karray,&parray);

	if(MAJSTRCMPS(key,karray[0])<0)
	    blockno = parray[0];
	else
	    blockno = parray[1];

	ipage = btreePricacheRead(cache,blockno);
	btreePrimaryInsertNonfull(cache,ipage,key,less,greater);

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
    rpage = btreePricacheNodenew(cache);
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
	tpage = btreePricacheRead(cache,tparray[i]);
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
	tpage = btreePricacheRead(cache,tparray[i]);
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

    ipage = btreePricacheRead(cache,ibn);
    
    btreePrimaryInsertNonfull(cache,ipage,key,less,greater);


    btreeDeallocPriArray(cache,arrays1);
    btreeDeallocPriArray(cache,arrays2);

    ipage = btreePricacheRead(cache,prev);

    btreeIdInsertKey(cache,ipage,mediankey,medianless,mediangtr);
    ajStrDel(&mediankey);

    return;
}




/* @funcstatic btreeIdSplitroot ***********************************************
**
** Split the root node
**
** @param [u] cache [AjPBtcache] cache
**
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

static void btreeIdSplitroot(AjPBtcache cache)
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
    ajDebug("btreeIdSplitroot zero\n");
#endif
    ++statCallIdSplitroot;

    /* ajDebug("In btreeIdSplitroot\n"); */

    order = cache->porder;

    arrays1   = btreeAllocPriArray(cache);
    karray    = arrays1->karray;
    parray    = arrays1->parray;

    arrays2   = btreeAllocPriArray(cache);
    tkarray   = arrays2->karray;
    tparray   = arrays2->parray;

    key = ajStrNew();

    rootpage = btreePricacheLocate(cache,0L);

    if(!rootpage)
	ajFatal("Root page has been unlocked 1");
    
    rootbuf = rootpage->buf;
    nkeys   = order - 1;
    keypos  = nkeys / 2;

    if(!(nkeys % 2))
	--keypos;


    rblockno = cache->totsize;
    rpage = btreePricacheNodenew(cache);
    rpage->dirty = BT_LOCK;
    rpage->lockfor = 1541;

    lblockno = cache->totsize;
    lpage = btreePricacheNodenew(cache);

    if(!cache->plevel)
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
    
    if(cache->plevel)
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
	tpage = btreePricacheRead(cache,tparray[i]);
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
	tpage = btreePricacheRead(cache,tparray[i]);
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

    ++cache->plevel;

    ajStrDel(&key);
    
    return;
}




/* @func ajBtreeIdentIndex ****************************************************
**
** Insert an ID structure into the tree
**
** @param [u] cache [AjPBtcache] cache
** @param [u] id [AjPBtId] Id object
**
** @return [void] pointer to a page
**
** @release 6.5.0
** @@
******************************************************************************/

void ajBtreeIdentIndex(AjPBtcache cache, AjPBtId id)
{
    AjPBtpage spage   = NULL;
    ajulong lblockno = 0L;
    ajulong rblockno = 0L;
    ajulong blockno  = 0L;
    ajulong shift    = 0L;

    ajuint nkeys = 0;

    ajuint nodetype = 0;
    
    AjPBtId curid = NULL;
    
    ajuint n;
    ajuint ientry;

    unsigned char *buf = NULL;

#if AJINDEX_DEBUG
    ajDebug("btreeIdentIndex '%S'\n", id->id);
#endif

    /* ajDebug("In ajBtreeIdentIndex\n"); */

    ajStrFmtQuery(&id->id);

    if(!MAJSTRGETLEN(id->id))
	return;

    if(MAJSTRGETLEN(id->id) > cache->keylimit)
        ajStrTruncateLen(&id->id, cache->keylimit);

    spage = btreeIdentFind(cache,id->id);
    buf = spage->buf;

    GBT_NKEYS(buf,&nkeys);
    GBT_NODETYPE(buf,&nodetype);
    
    if(!nkeys)
    {
	lblockno = cache->totsize;
	btreeWriteIdbucketEmpty(cache,lblockno);

	rblockno = cache->totsize;
	btreeWriteIdbucketEmpty(cache,rblockno);	

	btreeWriteNodeSingle(cache,spage,id->id,lblockno,rblockno);

	btreeIdbucketAdd(cache,rblockno,id);
        ++cache->countunique;
        ++cache->countall;

	return;
    }


    /* Search to see whether entry exists */
    
    blockno = btreeGetBlockS(cache,buf,id->id);

    curid = btreeIdbucketFindDupId(cache,blockno,id->id,&ientry);

    if(curid)
    {
        ++cache->countall;

	btreeIdentDupInsert(cache,id,curid);
        btreeWriteIdbucketId(cache,blockno,curid,ientry);
        ajBtreeIdDel(&curid);

	return;
    }

    if(nodetype != BT_ROOT)
	if((shift = btreeIdentInsertShift(cache,&spage,id->id)))
	    blockno = shift;

    buf = spage->buf;

    n = btreeIdbucketCount(cache,blockno);

    if(n == cache->pnperbucket)
    {
	if(btreeIdbucketsReorder(cache,spage))
	{
            blockno = btreeGetBlockS(cache,buf,id->id);
	}
	else
	{
	    btreeIdSplitleaf(cache,spage);
	    spage  = btreeIdentFind(cache,id->id);
	    buf = spage->buf;

            blockno = btreeGetBlockS(cache,buf,id->id);
	}
    }


    btreeIdbucketAdd(cache,blockno,id);

    ++cache->countunique;
    ++cache->countall;

    return;
}




/* @funcstatic btreeIdentDupInsert ********************************************
**
** Insert a known duplicate ID structure into the tree
**
** @param [u] cache [AjPBtcache] cache
** @param [r] newid [const AjPBtId] New ID object
** @param [u] curid [AjPBtId] Id object from current index
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void btreeIdentDupInsert(AjPBtcache cache, const AjPBtId newid,
                                AjPBtId curid)
{
    AjPBtpage page;
    AjPBtpage rpage;
    ajulong secrootpage = 0L;
    unsigned char *buf;
    ajulong right = 0L;
    ajulong refoffsets[1000];
    AjPBtNumId num = NULL;
    AjOBtNumId numobj = {0L, refoffsets, 0, 0};

    ajuint iref;

    /* ajDebug("In btreeIdentDupInsert\n"); */

    if(!curid->dups)
    {
        /* first duplicate: could use SIMPLEBUCKET */

	curid->dups = 1;
	num = &numobj;
	
	num->dbno      = curid->dbno;
	num->offset    = curid->offset;

        if(cache->refcount)
        {
            if(cache->refcount > 1000)
                ajFatal("btreeIdentDupInsert refcount %u", cache->refcount);
            num->refcount = cache->refcount;
            for(iref=0; iref < cache->refcount; iref++)
                num->refoffsets[iref] = curid->refoffsets[iref];
        }

	secrootpage = cache->totsize;

	curid->offset = secrootpage;

	btreeSecrootCreate(cache,secrootpage);
	cache->secrootblock = secrootpage;
	page = btreeSeccacheWrite(cache,secrootpage);
	page->dirty = BT_DIRTY;

        if(btreeDoRootSync)
            btreeCacheRootSync(cache,secrootpage);
	page->dirty = BT_LOCK;
	page->lockfor = 1551;

	rpage = btreePricacheLocate(cache, 0L);
	rpage->dirty = BT_LOCK;
        rpage->lockfor = 1552;

	cache->slevel = 0;

        btreeNumInsert(cache,num,page);

	num->dbno      = newid->dbno;
	num->offset    = newid->offset;

        if(cache->refcount)
        {
            num->refcount = cache->refcount;
            for(iref=0; iref < cache->refcount; iref++)
                num->refoffsets[iref] = newid->refoffsets[iref];
        }

	btreeNumInsert(cache,num,page);
	++curid->dups;

	return;
    }
    else
    {
	cache->secrootblock = curid->offset;
	page = btreeSeccacheWrite(cache,cache->secrootblock);
	page->dirty = BT_LOCK;
        page->lockfor = 1553;
	buf = page->buf;
	GBT_RIGHT(buf,&right);
	cache->slevel = (ajuint) right;

	num = &numobj;
	
	num->dbno      = newid->dbno;
	num->offset    = newid->offset;

        if(cache->refcount)
        {
            for(iref=0; iref < cache->refcount; iref++)
                num->refoffsets[iref] = newid->refoffsets[iref];
	}

	btreeNumInsert(cache,num,page);

	++curid->dups;
    }
    

    page->dirty = BT_DIRTY;

    return;
}




#if 0
/* #funcstatic btreeHybbucketIdlist *******************************************
**
** Copies all hybrid IDs into a list
**
** #param [u] cache [AjPBtcache] cache
** #param [r] pagepos [ajulong] page number
** #param [u] idlist [AjPList] list to hold hybrid IDs
**
** #return [ajulong] Overflow
**
** #release 6.4.0
** ##
******************************************************************************/
/*
static ajulong btreeHybbucketIdlist(AjPBtcache cache, ajulong pagepos,
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
    ajuint  iref;
    ajuint  len = 0;
    ajuint idlen;
    ajuint keyskip = BT_DDOFF + cache->refcount*BT_EXTRA + 1;

    /# ajDebug("In btreeHybbucketIdlist\n"); #/
    
    if(!pagepos)
	ajFatal("HybbucketIdlist: cannot read bucket from root page cache %S",
                cache->filename);

    page  = btreePricacheRead(cache,pagepos);
    lpage = page;
    dirtysave = lpage->dirty;
    lpage->dirty = BT_LOCK;
    lpage->lockfor = 1561;

    buf = lpage->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);
    if(nodetype != BT_IDBUCKET)
	ajFatal("HybbucketIdlist: NodeType mismatch. "
                "Not bucket (%u) cache %S",
                nodetype, cache->filename);
    
    GBT_BUCKNENTRIES(buf,&nentries);
    if(nentries > cache->pnperbucket)
	ajFatal("HybbucketIdlist: Bucket too full page: %Lu "
                "entries: %u max: %u cache %S",
                pagepos, nentries, cache->pnperbucket, cache->filename);
    

    GBT_BUCKOVERFLOW(buf,&overflow);
    pageoverflow = overflow;
    
    kptr  = PBT_BUCKKEYLEN(buf);
    idptr = kptr + (nentries * sizeof(ajuint));

    for(i=0;i<nentries;++i)
    {
	BT_GETAJUINT(kptr,&len);
        idlen = len - keyskip;

/#
//	if((idptr-buf+1) + len > cache->pagesize)	/# overflow #/
//	{
//	    /# ajDebug("HybbucketIdlist: Overflow\n"); #/
//	    page  = btreePricacheRead(cache,overflow);
//	    buf = page->buf;
//	    GBT_BUCKNODETYPE(buf,&nodetype);
//	    if(nodetype != BT_IDBUCKET)
//		ajFatal("HybbucketIdlist: NodeType mismatch. "
//                        "Not bucket (%u) cache %S",
//			nodetype, cache->filename);
//	    GBT_BUCKOVERFLOW(buf,&overflow);
//	    /# overflow bucket ids start at the keylen position #/
//	    idptr = PBT_BUCKKEYLEN(buf);
//	}
#/

	id = ajBtreeIdNew(cache->refcount);
        
	/# Fill ID objects #/
	ajStrAssignLenC(&id->id,(const char *)idptr,idlen);
	idptr += (idlen + 1);
	BT_GETAJUINT(idptr,&id->dbno);
	idptr += sizeof(ajuint);
	BT_GETAJUINT(idptr,&id->dups);
	idptr += sizeof(ajuint);	
	BT_GETAJULONG(idptr,&id->offset);
	idptr += sizeof(ajulong);

        if(cache->refcount)
        {
            for(iref=0; iref < cache->refcount; iref++)
            {
                BT_GETAJULONG(idptr,&id->refoffsets[iref]);
                idptr += sizeof(ajulong);
            }
        }

	kptr += sizeof(ajuint);
        ajListPushAppend(idlist, id);
    }

    lpage->dirty = dirtysave;
    
    return pageoverflow;
}
*/
#endif




/* @funcstatic btreeIdbucketIdlistAll *****************************************
**
** Copies all ID objects into a list, following duplicates
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
** @param [u] idlist [AjPList] list to hold hybrid IDs
**
** @return [ajulong] Overflow
**
** @release 6.5.0
** @@
******************************************************************************/

static ajulong btreeIdbucketIdlistAll(AjPBtcache cache, ajulong pagepos,
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
    ajuint  iref;
    ajuint  len = 0;
    ajuint idlen;
    ajuint keyskip = BT_DDOFF + cache->refcount*BT_EXTRA + 1;

    /* ajDebug("In btreeIdbucketIdlistAll\n"); */
    
    if(!pagepos)
	ajFatal("IdbucketIdlistAll: cannot read bucket from root page cache %S",
                cache->filename);

    page  = btreePricacheRead(cache,pagepos);
    lpage = page;
    dirtysave = lpage->dirty;
    lpage->dirty = BT_LOCK;
    lpage->lockfor = 1561;

    buf = lpage->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);
    if(nodetype != BT_IDBUCKET)
	ajFatal("IdbucketIdlistAll: NodeType mismatch. "
                "Not bucket (%u) cache %S",
                nodetype, cache->filename);
    
    GBT_BUCKNENTRIES(buf,&nentries);
    if(nentries > cache->pnperbucket)
	ajFatal("IdbucketIdlistAll: Bucket too full page: %Lu "
                "entries: %u max: %u cache %S",
                pagepos, nentries, cache->pnperbucket, cache->filename);
    

    GBT_BUCKOVERFLOW(buf,&overflow);
    pageoverflow = overflow;
    
    kptr  = PBT_BUCKKEYLEN(buf);
    idptr = kptr + (nentries * sizeof(ajuint));

    for(i=0;i<nentries;++i)
    {
	BT_GETAJUINT(kptr,&len);
        idlen = len - keyskip;

/*
//	if((idptr-buf+1) + len > cache->pagesize)	/# overflow #/
//	{
//	    /# ajDebug("IdbucketIdlistAll: Overflow\n"); #/
//	    page  = btreePricacheRead(cache,overflow);
//	    buf = page->buf;
//	    GBT_BUCKNODETYPE(buf,&nodetype);
//	    if(nodetype != BT_IDBUCKET)
//		ajFatal("IdbucketIdlistAll: NodeType mismatch. "
//                        "Not bucket (%u) cache %S",
//			nodetype, cache->filename);
//	    GBT_BUCKOVERFLOW(buf,&overflow);
//	    /# overflow bucket ids start at the keylen position #/
//	    idptr = PBT_BUCKKEYLEN(buf);
//	}
*/

	id = ajBtreeIdNew(cache->refcount);
        
	/* Fill ID objects */
	ajStrAssignLenC(&id->id,(const char *)idptr,idlen);
	idptr += (idlen + 1);
	BT_GETAJUINT(idptr,&id->dbno);
	idptr += sizeof(ajuint);
	BT_GETAJUINT(idptr,&id->dups);
	idptr += sizeof(ajuint);	
	BT_GETAJULONG(idptr,&id->offset);
	idptr += sizeof(ajulong);

        if(cache->refcount)
        {
            for(iref=0; iref < cache->refcount; iref++)
            {
                BT_GETAJULONG(idptr,&id->refoffsets[iref]);
                idptr += sizeof(ajulong);
            }
        }

	kptr += sizeof(ajuint);
        if(!id->dups)
            ajListPushAppend(idlist, id);
        else
        {
            btreeIdentFetchMulti(cache, id->id, id->offset, idlist);
            ajBtreeIdDel(&id);
        }
    }

    lpage->dirty = dirtysave;
    
    return pageoverflow;
}




/* @funcstatic btreeGetNumKeys ************************************************
**
** Get Keys and Pointers from an internal node
**
** @param [u] cache [AjPBtcache] cache
** @param [u] buf [unsigned char *] page buffer
** @param [w] keys [ajulong **] keys
** @param [w] ptrs [ajulong**] ptrs
**
** @return [void]
**
** @release 4.0.0
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




/* @funcstatic btreeGetNumPointers ********************************************
**
** Get Pointers from an internal numeric node
**
** @param [u] cache [AjPBtcache] cache
** @param [u] buf [unsigned char *] page buffer
** @param [w] ptrs [ajulong**] ptrs
**
** @return [void]
**
** @release 6.4.0
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




/* @funcstatic btreeWriteNumNode **********************************************
**
** Write an internal numeric node
**
** @param [u] cache [AjPBtcache] cache
** @param [u] spage [AjPBtpage] buffer
** @param [r] keys [const ajulong *] keys
** @param [r] ptrs [const ajulong*] page pointers
** @param [r] nkeys [ajuint] number of keys

**
** @return [void]
**
** @release 4.0.0
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

    ajulong lv       = 0L;
    ajulong overflow = 0L;
    ajuint i;

/*  ajulong aspace   = 0L;*/

    (void) cache;               /* make it used */

#if AJINDEX_DEBUG
    ajDebug("btreeWriteNumNode %Lu nkeys:%u\n", spage->pagepos, nkeys);
#endif

    buf = spage->buf;

    tnkeys = nkeys;
    SBT_NKEYS(buf,tnkeys);
    totlen = 0;
    SBT_TOTLEN(buf, totlen);

    pptr = PBT_KEYLEN(buf);

    /* drop check - tested before node write - page size dependency */
/*
//    aspace = 2 * nkeys * sizeof(ajulong) + sizeof(ajulong);
//
//    if((ajuint) ((pptr+aspace)-buf) > cache->pagesize)
//	ajFatal("WriteNumNode: too many keys for available pagesize");
*/

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

    overflow = 0L;
    SBT_OVERFLOW(buf,overflow);

    spage->dirty = BT_DIRTY;

    return;
}




/* @funcstatic btreeWriteNumNodeSingle ****************************************
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
**
** @release 6.4.0
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

    ajulong lv       = 0L;
    ajulong overflow = 0L;

/*  ajulong aspace   = 0L;*/

    (void) cache;             /* make it used */

    /* ajDebug("In btreeWriteNumNode\n"); */

    buf = spage->buf;

    tnkeys = 1;
    SBT_NKEYS(buf,tnkeys);
    totlen = 0;
    SBT_TOTLEN(buf,totlen);

    pptr = PBT_KEYLEN(buf);

/* drop check - tested before call - pagesize dependency */
/*
//    aspace = 2 * sizeof(ajulong) + sizeof(ajulong);
//
//    if((ajuint) ((pptr+aspace)-buf) > cache->pagesize)
//	ajFatal("WriteNumNodeSingle: "
//                "too many keys for available pagesize cache %S",
//                cache->filename);
*/

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

    spage->dirty = BT_DIRTY;    /* test in caller */

    return;
}




/* @funcstatic btreeWriteNumbucket ********************************************
**
** Write index bucket object to the cache given a disc page number
**
** @param [u] cache [AjPBtcache] cache
** @param [r] bucket [const AjPNumbucket] bucket
** @param [r] pagepos [ajulong] page number
**
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

static void btreeWriteNumbucket(AjPBtcache cache, const AjPNumbucket bucket,
                                ajulong pagepos)
{
    AjPBtpage page      = NULL;
    unsigned char *buf  = NULL;
    unsigned char *pptr = NULL;

    ajuint  uv  = 0;
    ajuint   v   = 0;
    ajuint  i    = 0;
    ajuint  iref = 0;
    ajulong lv   = 0L;
    ajuint nentries = 0;
    ajulong overflow = 0L;

    /* ajDebug("In btreeWriteNumbucket\n"); */

    if(pagepos == cache->totsize)
    {
	page = btreeSeccacheBucketnew(cache);
	buf = page->buf;
	overflow = 0L;
    }
    else
    {
	page = btreeSeccacheRead(cache,pagepos);
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

        if(cache->refcount)
        {
            for(iref=0; iref < cache->refcount; iref++)
            {
                lv = bucket->NumId[i]->refoffsets[iref];
                BT_SETAJULONG(pptr,lv);
                pptr += sizeof(ajulong);
            }
        }

	uv = bucket->NumId[i]->dbno;
	BT_SETAJUINT(pptr,uv);
	pptr += sizeof(ajuint);
    }
    
    lv = 0L;
    SBT_BUCKOVERFLOW(buf,lv);

    page->dirty = BT_DIRTY;

    return;
}




/* @funcstatic btreeWriteNumbucketEmpty ***************************************
**
** Write empty index numeric bucket object to the cache given a disc page number
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void btreeWriteNumbucketEmpty(AjPBtcache cache, ajulong pagepos)
{
    AjPBtpage page      = NULL;
    unsigned char *buf  = NULL;

    ajuint   v   = 0;
    ajulong lv   = 0L;
    ajulong overflow = 0L;

    /* ajDebug("In btreeWriteNumbucketEmpty\n"); */

    if(pagepos == cache->totsize)
    {
	page = btreeSeccacheBucketnew(cache);
	buf = page->buf;
	overflow = 0L;
	lv = overflow;
	SBT_BUCKOVERFLOW(buf,lv);
    }
    else
    {
	page = btreeSeccacheRead(cache,pagepos);
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
 
    page->dirty = BT_DIRTY;     /* clear the lock */

    return;
}




/* @funcstatic btreeNumbucketIdlist *******************************************
**
** Copies all numeric IDs into a list
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
** @param [u] idlist [AjPList] list to hold numeric ID strings
**
** @return [ajulong] Overflow
**
** @release 6.4.0
** @@
******************************************************************************/

static ajulong btreeNumbucketIdlist(AjPBtcache cache, ajulong pagepos,
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
    ajuint iref;

    /* ajDebug("In btreeNumbucketIdlist\n"); */

    if(pagepos == cache->secrootblock)
	ajFatal("NumbucketIdlist: cannot read bucket from root page cache %S",
                cache->filename);

    page = btreeSeccacheRead(cache,pagepos);
    dirtysave = page->dirty;
    page->dirty = BT_LOCK;
    page->lockfor = 1591;
    buf = page->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);
    if(nodetype != BT_NUMBUCKET && nodetype != BT_IDBUCKET)
	ajFatal("NumbucketIdlist: Nodetype mismatch. "
                "Not bucket (%u) cache %S",
                nodetype, cache->filename);

    GBT_BUCKNENTRIES(buf,&nentries);
    if(nentries > cache->snperbucket)
	ajFatal("NumbucketIdlist: Bucket too full page: %Lu "
                "entries: %u max: %u cache %S",
                pagepos, nentries, cache->snperbucket, cache->filename);

    GBT_BUCKOVERFLOW(buf,&overflow);
    pageoverflow = overflow;

    pptr = PBT_BUCKKEYLEN(buf);

    for(i=0;i<nentries;++i)
    {
        AJNEW0(num);
        if(cache->refcount)
        {
            AJCNEW0(num->refoffsets, cache->refcount);
            num->refcount = cache->refcount;
        }

	BT_GETAJULONG(pptr,&num->offset);
	pptr += sizeof(ajulong);

        if(cache->refcount)
        {
            for(iref=0; iref < cache->refcount; iref++)
            {
                BT_GETAJULONG(pptr,&num->refoffsets[iref]);
                pptr += sizeof(ajulong);
            }
        }

	BT_GETAJUINT(pptr,&num->dbno);
	pptr += sizeof(ajuint);
        ajListPushAppend(idlist, num);
        num = NULL;
    }
    
    page->dirty = dirtysave;

    return pageoverflow;
}




/* @funcstatic btreeNumbucketBtidlist *****************************************
**
** Copies all numeric IDs into a list of btree Ids
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
** @param [r] idname [const AjPStr] id name
** @param [u] idlist [AjPList] list to hold numeric ID strings
**
** @return [ajulong] Overflow
**
** @release 6.4.0
** @@
******************************************************************************/

static ajulong btreeNumbucketBtidlist(AjPBtcache cache, ajulong pagepos,
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
    ajuint  iref;
    
    /* ajDebug("In btreeNumbucketBtidlist\n"); */

    if(pagepos == cache->secrootblock)
	ajFatal("NumbucketBtidlist: cannot read bucket "
                "from root page cache %S",
                cache->filename);

    page = btreeSeccacheRead(cache,pagepos);
    dirtysave = page->dirty;
    page->dirty = BT_LOCK;
    page->lockfor = 1601;
    buf = page->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);
    if(nodetype != BT_NUMBUCKET && nodetype != BT_IDBUCKET)
	ajFatal("NumbucketBtidlist: Nodetype mismatch. "
                "Not bucket (%u) cache %S",
                nodetype, cache->filename);

    GBT_BUCKNENTRIES(buf,&nentries);
    if(nentries > cache->snperbucket)
	ajFatal("NumbucketBtidlist: Bucket too full page: %Lu "
                "entries: %u max: %u cache %S",
                pagepos, nentries, cache->snperbucket,
                cache->filename);

    GBT_BUCKOVERFLOW(buf,&overflow);
    pageoverflow = overflow;

    pptr = PBT_BUCKKEYLEN(buf);

    for(i=0;i<nentries;++i)
    {
        btid = ajBtreeIdNew(cache->refcount);
        ajStrAssignS(&btid->id,idname);
	BT_GETAJULONG(pptr,&btid->offset);
	pptr += sizeof(ajulong);

        if(cache->refcount)
        {
            for(iref=0; iref < cache->refcount; iref++)
            {
                BT_GETAJULONG(pptr,&btid->refoffsets[iref]);
                pptr += sizeof(ajulong);
            }
        }

	BT_GETAJUINT(pptr,&btid->dbno);
	pptr += sizeof(ajuint);
        ajListPushAppend(idlist, btid);
        btid = NULL;
    }
    
    page->dirty = dirtysave;

    return pageoverflow;
}




/* @funcstatic btreeNumbucketBthitlist ****************************************
**
** Copies all numeric IDs into a list of btree Hits
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
** @param [u] hitlist [AjPList] list to hold numeric hits
**
** @return [ajulong] Overflow
**
** @release 6.5.0
** @@
******************************************************************************/

static ajulong btreeNumbucketBthitlist(AjPBtcache cache, ajulong pagepos,
                                       AjPList hitlist)
{
    AjPBtHit bthit      = NULL;
    AjPBtpage page      = NULL;
    unsigned char *buf  = NULL;
    unsigned char *pptr = NULL;
    ajuint  nodetype  = 0;
    ajuint  nentries  = 0;
    ajulong overflow  = 0L;
    ajulong pageoverflow = 0L;
    ajuint  dirtysave = 0;
    
    ajuint  i;
    
    /* ajDebug("In btreeNumbucketBthitlist\n"); */

    if(cache->refcount)
        ajWarn("btreeNumbucketbthitlist called for cache '%S' "
               "with %u reference files",
               cache->filename, cache->refcount);

    if(pagepos == cache->secrootblock)
	ajFatal("NumbucketBthitlist: cannot read bucket "
                "from root page cache %S",
                cache->filename);

    page = btreeSeccacheRead(cache,pagepos);
    dirtysave = page->dirty;
    page->dirty = BT_LOCK;
    page->lockfor = 1601;
    buf = page->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);
    if(nodetype != BT_NUMBUCKET && nodetype != BT_IDBUCKET)
	ajFatal("NumbucketBthitlist: Nodetype mismatch. "
                "Not bucket (%u) cache %S",
                nodetype, cache->filename);

    GBT_BUCKNENTRIES(buf,&nentries);
    if(nentries > cache->snperbucket)
	ajFatal("NumbucketBthitlist: Bucket too full page: %Lu "
                "entries: %u max: %u cache %S",
                pagepos, nentries, cache->snperbucket,
                cache->filename);

    GBT_BUCKOVERFLOW(buf,&overflow);
    pageoverflow = overflow;

    pptr = PBT_BUCKKEYLEN(buf);

    for(i=0;i<nentries;++i)
    {
        bthit = ajBtreeHitNew();
	BT_GETAJULONG(pptr,&bthit->offset);
	pptr += sizeof(ajulong);

        if(cache->refcount)
            pptr += cache->refcount*sizeof(ajulong);

	BT_GETAJUINT(pptr,&bthit->dbno);
	pptr += sizeof(ajuint);
        ajListPushAppend(hitlist, bthit);
        bthit = NULL;
    }
    
    page->dirty = dirtysave;

    return pageoverflow;
}




/* @funcstatic btreeNumbucketBthitreflist *************************************
**
** Copies all numeric IDs into a list of btree Hitrefs
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
** @param [u] hitlist [AjPList] list to hold numeric hitrefs
**
** @return [ajulong] Overflow
**
** @release 6.5.0
** @@
******************************************************************************/

static ajulong btreeNumbucketBthitreflist(AjPBtcache cache, ajulong pagepos,
                                          AjPList hitlist)
{
    AjPBtHitref bthitref = NULL;
    AjPBtpage page      = NULL;
    unsigned char *buf  = NULL;
    unsigned char *pptr = NULL;
    ajuint  nodetype  = 0;
    ajuint  nentries  = 0;
    ajulong overflow  = 0L;
    ajulong pageoverflow = 0L;
    ajuint  dirtysave = 0;
    
    ajuint  i;
    
    /* ajDebug("In btreeNumbucketBthitreflist\n"); */

    if(cache->refcount != 1)
        ajWarn("btreeNumbucketbthitreflist called for cache '%S' "
               "with %u reference files",
               cache->filename, cache->refcount);

    if(pagepos == cache->secrootblock)
	ajFatal("NumbucketBthitreflist: cannot read bucket "
                "from root page cache %S",
                cache->filename);

    page = btreeSeccacheRead(cache,pagepos);
    dirtysave = page->dirty;
    page->dirty = BT_LOCK;
    page->lockfor = 1601;
    buf = page->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);
    if(nodetype != BT_NUMBUCKET && nodetype != BT_IDBUCKET)
	ajFatal("NumbucketBthitreflist: Nodetype mismatch. "
                "Not bucket (%u) cache %S",
                nodetype, cache->filename);

    GBT_BUCKNENTRIES(buf,&nentries);
    if(nentries > cache->snperbucket)
	ajFatal("NumbucketBthitreflist: Bucket too full page: %Lu "
                "entries: %u max: %u cache %S",
                pagepos, nentries, cache->snperbucket,
                cache->filename);

    GBT_BUCKOVERFLOW(buf,&overflow);
    pageoverflow = overflow;

    pptr = PBT_BUCKKEYLEN(buf);

    for(i=0;i<nentries;++i)
    {
        bthitref = ajBtreeHitrefNew();
	BT_GETAJULONG(pptr,&bthitref->offset);
	pptr += sizeof(ajulong);

        if(cache->refcount)
        {
            BT_GETAJULONG(pptr,&bthitref->refoffset);
            pptr += cache->refcount*sizeof(ajulong);
        }
        
	BT_GETAJUINT(pptr,&bthitref->dbno);
	pptr += sizeof(ajuint);
        ajListPushAppend(hitlist, bthitref);
        bthitref = NULL;
    }
    
    page->dirty = dirtysave;

    return pageoverflow;
}




/* @funcstatic btreeReadNumbucket *********************************************
**
** Constructor for index bucket given a disc page number
** Creates one empty key slot for possible addition
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
**
** @return [AjPNumbucket] bucket
**
** @release 4.0.0
** @@
******************************************************************************/

static AjPNumbucket btreeReadNumbucket(AjPBtcache cache, ajulong pagepos)
{
    AjPNumbucket bucket    = NULL;
    AjPBtpage page      = NULL;
    unsigned char *buf  = NULL;
    unsigned char *pptr = NULL;
    ajuint  nodetype  = 0;
    ajuint nentries  = 0;
    ajulong overflow  = 0L;
    ajuint  dirtysave = 0;
    
    ajuint  i;
    ajuint  iref;
    
    /* ajDebug("In btreeReadNumbucket\n"); */

    if(pagepos == cache->secrootblock)
	ajFatal("ReadNumbucket: cannot read bucket from a root page");

    page = btreeSeccacheRead(cache,pagepos);
    dirtysave = page->dirty;
    page->dirty = BT_LOCK;
    page->lockfor = 1611;
    buf = page->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);

    if(nodetype != BT_NUMBUCKET && nodetype !=  BT_IDBUCKET)
	ajFatal("ReadNumbucket: Nodetype mismatch. Not bucket (%u) cache %S",
                nodetype, cache->filename);

    GBT_BUCKNENTRIES(buf,&nentries);

    if(nentries > cache->snperbucket)
	ajFatal("ReadNumbucket: Bucket too full page: %Lu "
                "entries: %u max: %u cache %S",
                pagepos, nentries, cache->snperbucket,
                cache->filename);

    GBT_BUCKOVERFLOW(buf,&overflow);

    bucket = btreeNumbucketNew(cache->snperbucket, cache->refcount);
    bucket->Nentries = nentries;

    pptr = PBT_BUCKKEYLEN(buf);

    for(i=0;i<nentries;++i)
    {
	BT_GETAJULONG(pptr,&bucket->NumId[i]->offset);
	pptr += sizeof(ajulong);

        if(cache->refcount)
        {
            for(iref=0; iref < cache->refcount; iref++)
            {
                BT_GETAJULONG(pptr,&bucket->NumId[i]->refoffsets[iref]);
                pptr += sizeof(ajulong);
            }
        }

	BT_GETAJUINT(pptr,&bucket->NumId[i]->dbno);
	pptr += sizeof(ajuint);
    }
    
    page->dirty = dirtysave;

    return bucket;
}




/* @funcstatic btreeNumbucketDel **********************************************
**
** Delete a bucket object
**
** @param [w] thys [AjPNumbucket*] bucket
**
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

static void btreeNumbucketDel(AjPNumbucket *thys)
{
    AjPNumbucket pthis = NULL;
    ajuint newmax;
    
    /* ajDebug("In btreeNumbucketDel\n"); */

    if(!thys || !*thys)
	return;

    pthis = *thys;

    if(!statSaveNumbucket)
    {
        statSaveNumbucketMax=2048;
        statSaveNumbucketNext=0;
        AJCNEW0(statSaveNumbucket,statSaveNumbucketMax);
    }
    
    if(statSaveNumbucketNext >= statSaveNumbucketMax)
    {
        newmax = statSaveNumbucketMax + statSaveNumbucketMax;
        AJCRESIZE0(statSaveNumbucket,statSaveNumbucketMax,newmax);
        statSaveNumbucketMax = newmax;
    }
        
    statSaveNumbucket[statSaveNumbucketNext++] = pthis;
 
    *thys = NULL;

    return;
}




/* @funcstatic btreeNumbucketFree *********************************************
**
** Delete a bucket object
**
** @param [w] thys [AjPNumbucket*] bucket
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void btreeNumbucketFree(AjPNumbucket *thys)
{
    AjPNumbucket pthis = NULL;
    ajuint i;
    
    
    /* ajDebug("In btreeNumbucketFree\n"); */

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




/* @funcstatic btreeNumFind ***************************************************
**
** Find the node that should contain a new key for insertion
**
** @param [u] cache [AjPBtcache] cache
** @param [r] key [const ajulong] key to search for 
**
** @return [AjPBtpage] leaf node where item should be inserted
**
** @release 6.5.0
** @@
******************************************************************************/

static AjPBtpage btreeNumFind(AjPBtcache cache, const ajulong key)
{
    AjPBtpage root = NULL;
    AjPBtpage ret  = NULL;

    /* ajDebug("In btreeNumFind\n"); */

    /* The root node should always be in the cache (BT_LOCKed) */
    root = btreeSeccacheLocate(cache,cache->secrootblock);

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
**
** @release 4.0.0
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

    if(ival != BT_SECLEAF)
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




/* @funcstatic btreeNumPageFromKey ********************************************
**
** Return next lower index page given a key in a numeric secondary tree
**
** @param [u] cache [AjPBtcache] cache
** @param [u] buf [unsigned char *] page buffer
** @param [r] key [ajulong] key to search for 
**
** @return [AjPBtpage] pointer to a page
**
** @release 4.0.0
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

    page =  btreeSeccacheRead(cache,blockno);

    return page;
}




/* @funcstatic btreeNumbucketAdd **********************************************
**
** Add offset info to a numeric bucket
** Only called if there is room in the bucket
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number of bucket
** @param [r] num [const AjPBtNumId] ID info
**
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

static void btreeNumbucketAdd(AjPBtcache cache, ajulong pagepos,
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
    ajuint  iref;

    /* ajDebug("In btreeReadNumbucket\n"); */

    if(pagepos == cache->secrootblock)
	ajFatal("NumbucketAdd: cannot read bucket from root page cache %S",
                cache->filename);

    page = btreeSeccacheRead(cache,pagepos);

    page->dirty = BT_LOCK;      /* reset at end */
    page->lockfor = 1631;
    buf = page->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);
    if(nodetype != BT_NUMBUCKET)
	ajFatal("NumbucketAdd: Nodetype mismatch. "
                "Not bucket (%u) cache %S",
                nodetype, cache->filename);

    GBT_BUCKNENTRIES(buf,&nentries);
    if(nentries > cache->snperbucket)
	ajFatal("NumbucketAdd: Bucket too full page: %Lu "
                "entries: %u max: %u cache %S",
                pagepos, nentries, cache->snperbucket, cache->filename);

    GBT_BUCKOVERFLOW(buf,&overflow);

    pptr = PBT_BUCKKEYLEN(buf);

    for(i=0;i<nentries;++i)
    {
        pptr += sizeof(ajulong);
        if(cache->refcount)
        {
            for(iref=0; iref < cache->refcount; iref++)
                pptr += sizeof(ajulong);
        }
	pptr += sizeof(ajuint);
    }
    
    lv = num->offset;
    BT_SETAJULONG(pptr,lv);
    pptr += sizeof(ajulong);

    if(cache->refcount)
    {
        for(iref=0; iref < cache->refcount; iref++)
        {
            lv = num->refoffsets[iref];
            BT_SETAJULONG(pptr,lv);
            pptr += sizeof(ajulong);
        }
    }

    uv = num->dbno;
    BT_SETAJUINT(pptr,uv);
    pptr += sizeof(ajuint);

    nentries++;
    SBT_BUCKNENTRIES(buf,nentries);

    page->dirty = BT_DIRTY;

    return;
}





/* @funcstatic btreeNumInNumbucket ********************************************
**
** Return number of entries in a bucket
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] page number
**
** @return [ajuint] Number of entries in bucket
**
** @release 4.0.0
** @@
******************************************************************************/

static ajuint btreeNumInNumbucket(AjPBtcache cache, ajulong pagepos)
{
    AjPBtpage page     = NULL;
    unsigned char *buf = NULL;
    ajuint  nodetype    = 0;
    ajuint nentries    = 0;
    
    /* ajDebug("In btreeNumInNumbucket\n"); */
    
    if(pagepos == cache->secrootblock)
	ajFatal("NumInNumbucket: Attempt to read bucket from root page\n");

    page  = btreeSeccacheRead(cache,pagepos);

    buf = page->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);

    if(nodetype != BT_NUMBUCKET)
	ajFatal("NumInNumbucket: NodeType mismatch. Not numbucket (%u) "
                "for cache '%S'",
		nodetype, cache->filename);
    
    GBT_BUCKNENTRIES(buf,&nentries);

    return nentries;
}




/* @funcstatic btreeNumbucketNew **********************************************
**
** Construct a bucket object
**
** @param [r] n [ajuint] Number of IDs
** @param [r] refcount [ajuint] Number of reference files per entry
**
** @return [AjPNumbucket] initialised disc block cache structure
**
** @release 4.0.0
** @@
******************************************************************************/

static AjPNumbucket btreeNumbucketNew(ajuint n, ajuint refcount)
{
    AjPNumbucket bucket = NULL;
    ajuint i;
    ajuint iref;

    /* ajDebug("In btreeNumbucketNew\n"); */
    if(statSaveNumbucketNext)
    {
       bucket = statSaveNumbucket[--statSaveNumbucketNext];
       for(i=0;i<bucket->Maxentries;++i)
       {
           bucket->NumId[i]->offset = 0L;
           bucket->NumId[i]->dbno = 0;
           if(refcount != bucket->NumId[i]->refcount)
           {
               if(refcount)
                   AJCRESIZE0(bucket->NumId[i]->refoffsets,
                             bucket->NumId[i]->refcount, refcount);
               else
                   AJFREE(bucket->NumId[i]->refoffsets);

               bucket->NumId[i]->refcount = refcount;
           }
           else if(refcount)
           {
               for(iref=0; iref < refcount; iref++)
                   bucket->NumId[i]->refoffsets[iref] = 0L;
           }
       }
       if(n > bucket->Maxentries)
       {
           if(bucket->Maxentries)
               AJCRESIZE0(bucket->NumId,bucket->Maxentries+1,n+1);
           else
               AJCNEW0(bucket->NumId,n+1);

           for(i=bucket->Maxentries;i<n;++i)
           {
               AJNEW0(bucket->NumId[i]);
               if(refcount)
                   AJCNEW0(bucket->NumId[i]->refoffsets, refcount);
           }

           bucket->Maxentries = n;
       }
   }
   else
   {
       AJNEW0(bucket);
       AJCNEW0(bucket->NumId,n+1);

       for(i=0;i<n;++i)
       {
           AJNEW0(bucket->NumId[i]);
           if(refcount)
               AJCNEW0(bucket->NumId[i]->refoffsets, refcount);
       }

       bucket->Maxentries = n;
   }

    bucket->NodeType = BT_NUMBUCKET;
    bucket->Nentries = n;
    bucket->Overflow = 0L;
    
    return bucket;
}




/* @funcstatic btreeReorderNumbuckets *****************************************
**
** Re-order leaf buckets
** Must only be called if one of the buckets is full
**
** @param [u] cache [AjPBtcache] cache
** @param [u] leaf [AjPBtpage] leaf page
**
** @return [AjBool] true if reorder was successful i.e. leaf not full
**
** @release 4.0.0
** @@
******************************************************************************/

static AjBool btreeReorderNumbuckets(AjPBtcache cache, AjPBtpage leaf)
{
    ajuint nkeys = 0;
    unsigned char *lbuf = NULL;

    AjPBtMem array = NULL;
    AjPBtMem newarray = NULL;
    ajulong *ptrs        = NULL;
    ajulong *newkeys        = NULL;
    ajulong *newptrs        = NULL;

    ajuint i = 0;
    ajuint iref = 0;

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
    AjPNumbucket cbucket = NULL;
    AjPBtNumId cid       = NULL;

    ajuint   iold = 0;
    
#if AJINDEX_DEBUG
    ajDebug("btreeReorderNumbuckets %Lu\n", leaf->pagepos);
#endif
    ++statCallNumbucketsReorder;

    /* ajDebug("In btreeReorderNumbuckets\n"); */

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
	ajFatal("ReorderNumbuckets: Attempt to reorder empty leaf");

    for(i=0;i<nkeys;++i)
	totalkeys += btreeNumInNumbucket(cache,ptrs[i]);

    totalkeys += btreeNumInNumbucket(cache,ptrs[i]);

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
	btreeNumbucketIdlist(cache,ptrs[i],idlist);

    ajListSort(idlist, &btreeNumIdCompare);

    cbucket = btreeNumbucketNew(maxnperbucket, cache->refcount);
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

            if(cache->refcount)
            {
                for(iref=0; iref < cache->refcount; iref++)
                    cid->refoffsets[iref] = bid->refoffsets[iref];
	    }

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
	btreeWriteNumbucket(cache,cbucket,newptrs[i]);
    }


    /* Deal with greater-than bucket */

    cbucket->Nentries = 0;

    count = 0;

    while(ajListPop(idlist,(void **)&bid))
    {
	cid = cbucket->NumId[count];
	cid->dbno = bid->dbno;
	cid->offset = bid->offset;

        if(cache->refcount)
        {
            for(iref=0; iref < cache->refcount; iref++)
                cid->refoffsets[iref] = bid->refoffsets[iref];
	}

	++cbucket->Nentries;
	++count;
	AJFREE(bid);
    }
    
    if((iold < order) && ptrs[iold])
        newptrs[i] = ptrs[iold++];
    else
        newptrs[i] = cache->totsize;
    btreeWriteNumbucket(cache,cbucket,newptrs[i]);

    btreeNumbucketDel(&cbucket);

#if AJINDEX_DEBUG
    if(iold < keylimit)
        ajDebug("btreeReorderNumbuckets '%S' %u -> %u",
                cache->filename, keylimit, iold);
#endif

    for(i = iold+1; i <= keylimit; i++)
    {
        btreeSecpageSetfree(cache, ptrs[i]);
    }

    /* Now write out a modified leaf with new keys/ptrs */

    nkeys = bucketlimit;

    btreeWriteNumNode(cache,leaf,newkeys,newptrs,nkeys);

    leaf->dirty = BT_DIRTY;

    if(nodetype == BT_SECROOT)
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




/* @funcstatic btreeNumNodeIsFull *********************************************
**
** Tests whether a node is full of keys
**
** @param [r] cache [const AjPBtcache] cache
** @param [u] page [AjPBtpage] original page
**
** @return [AjBool] true if full
**
** @release 4.0.0
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




/* @funcstatic btreeNumInsertNonfull ******************************************
**
** Insert a key into a non-full numeric secondary node
**
** @param [u] cache [AjPBtcache] cache
** @param [u] page [AjPBtpage] original page
** @param [r] key [ajulong] key to insert
** @param [r] less [ajulong] less-than pointer
** @param [r] greater [ajulong] greater-than pointer
**
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

static void btreeNumInsertNonfull(AjPBtcache cache, AjPBtpage page,
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
    
    /* ajDebug("In btreeNumInsertNonfull\n"); */

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
    if(nodetype == BT_SECROOT)
    {
	page->dirty = BT_LOCK;
        page->lockfor = 1651;
    }

    pagepos = page->pagepos;
    ppage = btreeSeccacheRead(cache,less);
    lv = pagepos;
    SBT_PREV(ppage->buf,lv);
    ppage->dirty = BT_DIRTY;

    ppage = btreeSeccacheRead(cache,greater);
    lv = pagepos;
    SBT_PREV(ppage->buf,lv);
    ppage->dirty = BT_DIRTY;

    btreeDeallocSecArray(cache,array);

    if(nodetype != BT_SECROOT)
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
**
** @release 4.0.0
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
	btreeNumInsertNonfull(cache,page,key,less,greater);
	return;
    }
    
    order = cache->sorder;
    lbuf = page->buf;
    GBT_NODETYPE(lbuf,&nodetype);
    page->dirty = BT_LOCK;
    page->lockfor = 1661;

    if(nodetype == BT_SECROOT)
    {
	btreeNumSplitroot(cache);

	if(page->pagepos)
	    page->dirty = BT_DIRTY;

        blockno = btreeGetBlockFirstN(cache,lbuf,key);

	ipage = btreeSeccacheRead(cache,blockno);
	btreeNumInsertNonfull(cache,ipage,key,less,greater);

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
    rpage = btreeSeccacheNodenew(cache);
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
	tpage = btreeSeccacheRead(cache,tparray[i]);
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
	tpage = btreeSeccacheRead(cache,tparray[i]);
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

    ipage = btreeSeccacheRead(cache,ibn);
    
    btreeNumInsertNonfull(cache,ipage,key,less,greater);

    ipage = btreeSeccacheRead(cache,prev);

    btreeNumInsertKey(cache,ipage,mediankey,medianless,mediangtr);

    btreeDeallocSecArray(cache,savekeyarray);

    return;
}




/* @funcstatic btreeNumSplitroot **********************************************
**
** Split a numeric root node
**
** @param [u] cache [AjPBtcache] cache
**
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

static void btreeNumSplitroot(AjPBtcache cache)
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
    ajDebug("btreeNumSplitroot %Lu\n", cache->secrootblock);
#endif
    ++statCallNumSplitroot;

    /* ajDebug("In btreeNumSplitroot\n"); */

    order = cache->sorder;

    array = btreeAllocSecArray(cache);
    karray  = array->overflows;
    parray  = array->parray;
    
    array2 = btreeAllocSecArray(cache);
    tkarray  = array2->overflows;
    tparray  = array2->parray;


    rootpage = btreeSeccacheLocate(cache,cache->secrootblock);
    rootbuf = rootpage->buf;

    nkeys = order - 1;

    keypos = nkeys / 2;

    if(!(nkeys % 2))
	--keypos;


    rblockno = cache->totsize;
    rpage = btreeSeccacheNodenew(cache);
    rpage->dirty = BT_LOCK;
    rpage->lockfor = 1671;

    lblockno = cache->totsize;
    lpage = btreeSeccacheNodenew(cache);


    /* Comment this next block out after the beta test */
    if(!cache->slevel)
    {
	fprintf(stderr,"btreeNumSplitroot: Shouldn't get here\n");
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
	nodetype = BT_SECINTERNAL;
    else
	nodetype = BT_SECLEAF;

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
	tpage = btreeSeccacheRead(cache,tparray[i]);
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
	tpage = btreeSeccacheRead(cache,tparray[i]);
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




/* @funcstatic btreeNumKeyShift ***********************************************
**
** Rebalance Nodes on insertion
**
** @param [u] cache [AjPBtcache] cache
** @param [u] tpage [AjPBtpage] page
**
** @return [void]
**
** @release 4.0.0
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

    
    ppage = btreeSeccacheRead(cache,parent);
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
	spage = btreeSeccacheRead(cache,pParray[pkeypos]);
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

	page = btreeSeccacheRead(cache,pSarray[skeys]);
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
	spage = btreeSeccacheRead(cache,pParray[pkeypos+1]);
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

	page = btreeSeccacheRead(cache,pSarray[0]);
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
**
** @release 4.0.0
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

    ppage = btreeSeccacheRead(cache,parent);


    
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
	spage = btreeSeccacheRead(cache,pParray[ppos-1]);
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
	spage = btreeSeccacheRead(cache,pParray[ppos+1]);
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




/* @funcstatic btreeNumInsert *************************************************
**
** Insert a file offset key into a secondary tree
**
** @param [u] cache [AjPBtcache] cache
** @param [r] num [const AjPBtNumId] Id object
** @param [w] page [AjPBtpage] cache page
**
** @return [void] pointer to a page
**
** @release 6.5.0
** @@
******************************************************************************/

static void btreeNumInsert(AjPBtcache cache, const AjPBtNumId num,
                           AjPBtpage page)
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

    /* ajDebug("In btreeNumInsert\n"); */

    (void) page;			/* make it used */

    numkey = num->offset;

    spage = btreeNumFind(cache,numkey);
    buf = spage->buf;

    GBT_NKEYS(buf,&nkeys);
    GBT_NODETYPE(buf,&nodetype);

    if(!nkeys)
    {
	lblockno = cache->totsize;
	btreeWriteNumbucketEmpty(cache,lblockno);
	rblockno = cache->totsize;
	btreeWriteNumbucketEmpty(cache,rblockno);

	btreeWriteNumNodeSingle(cache,spage,numkey,lblockno,rblockno);

	btreeNumbucketAdd(cache,rblockno,num);

	return;
    }

    blockno = btreeGetBlockN(cache,buf,numkey);

    if(nodetype != BT_SECROOT)
	if((shift = btreeNumInsertShift(cache,&spage,numkey)))
	    blockno = shift;


    buf = spage->buf;
    n = btreeNumInNumbucket(cache,blockno);

    if(n == cache->snperbucket)
    {
	if(btreeReorderNumbuckets(cache,spage))
	{
            blockno = btreeGetBlockN(cache,buf,numkey);
	}
	else
	{
	    btreeNumSplitleaf(cache,spage);
	    spage = btreeNumFind(cache,numkey);
	    buf = spage->buf;

	    blockno = btreeGetBlockN(cache,buf,numkey);
	}
    }

    btreeNumbucketAdd(cache,blockno,num);

    return;
}




/* @funcstatic btreeNumSplitleaf **********************************************
**
** Split a leaf and propagate up if necessary
**
** @param [u] cache [AjPBtcache] cache
** @param [u] spage [AjPBtpage] page
**
** @return [AjPBtpage] pointer to a parent page
**
** @release 4.0.0
** @@
******************************************************************************/

static AjPBtpage btreeNumSplitleaf(AjPBtcache cache, AjPBtpage spage)
{
    ajuint nkeys     = 0;
    ajuint totalkeys = 0;
    ajuint keylimit  = 0;
    ajuint order     = 0;
    ajuint nodetype  = 0;

    ajuint rootnodetype = 0;
    
    ajuint i;
    ajuint j;
    ajuint iref;

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

    AjPNumbucket cbucket  = NULL;
    
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
    ajDebug("btreeNumSplitleaf %Lu dirty:%u lockfor:%u\n",
            spage->pagepos, spage->dirty, spage->lockfor);
#endif
    ++statCallNumSplitleaf;

    /* ajDebug("In btreeNumSplitleaf\n"); */

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

    if(rootnodetype == BT_SECROOT)
    {
#if AJINDEX_DEBUG
	ajDebug("   Root leaf splitting page:%Lu secroot:%Lu\n",
                cache->totsize, cache->secrootblock);
#endif
	lblockno = cache->totsize;
	lpage = btreeSeccacheNodenew(cache);
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
    rpage = btreeSeccacheNodenew(cache);
    rbuf = rpage->buf;
    rpage->dirty = BT_LOCK;
    rpage->lockfor = 1702;

    if(rootnodetype == BT_SECROOT)
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
	btreeNumbucketIdlist(cache,parray[i], idlist);

    ajListSort(idlist, &btreeNumIdCompare);

    totalkeys = (ajuint) ajListGetLength(idlist);

    btreeBucketSplitCalc(totalkeys, keylimit, cache->snperbucket,
                         &lbucketlimit,&lmaxnperbucket,&lno,
                         &rbucketlimit,&rmaxnperbucket,&rno);

    cbucket = btreeNumbucketNew(cache->snperbucket, cache->refcount);

    count = 0;
    iold = 0;

    for(i=0;i<lbucketlimit;++i)
    {
	cbucket->Nentries = 0;

	for(j=0;j<lmaxnperbucket;++j)
	{
	    ajListPop(idlist,(void **)&bid);

	    cbucket->NumId[j]->dbno      = bid->dbno;
	    cbucket->NumId[j]->offset    = bid->offset;

            if(cache->refcount)
            {
                for(iref=0; iref < cache->refcount; iref++)
                    cbucket->NumId[j]->refoffsets[iref] = bid->refoffsets[iref];
	    }

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

	btreeWriteNumbucket(cache,cbucket,newparray[i]);
    }

    cbucket->Nentries = 0;

    j = 0;

    while(count != lno)
    {
	ajListPop(idlist,(void **)&bid);

	cbucket->NumId[j]->dbno      = bid->dbno;
	cbucket->NumId[j]->offset    = bid->offset;

        if(cache->refcount)
        {
            for(iref=0; iref < cache->refcount; iref++)
                cbucket->NumId[j]->refoffsets[iref] = bid->refoffsets[iref];
        }

	++j;
	++count;


	++cbucket->Nentries;
	AJFREE(bid);
    }

    if((iold < order) && parray[iold])
        newparray[i] = parray[iold++];
    else
        newparray[i] = cache->totsize;

    btreeWriteNumbucket(cache,cbucket,newparray[i]);

    nkeys = lbucketlimit;
    nodetype = BT_SECLEAF;
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
	    
	    cbucket->NumId[j]->dbno      = bid->dbno;
	    cbucket->NumId[j]->offset    = bid->offset;
	    
            if(cache->refcount)
            {
                for(iref=0; iref < cache->refcount; iref++)
                    cbucket->NumId[j]->refoffsets[iref] = bid->refoffsets[iref];
            }

	    ++cbucket->Nentries;
	    AJFREE(bid);
	}

	ajListPeek(idlist,(void **)&bid);
	newnarray[i] = bid->offset;

        if((iold < order) && parray[iold])
            newparray[i] = parray[iold++];
        else
            newparray[i] = cache->totsize;

	btreeWriteNumbucket(cache,cbucket,newparray[i]);
    }

    cbucket->Nentries = 0;

    j = 0;

    while(ajListPop(idlist,(void**)&bid))
    {
	cbucket->NumId[j]->dbno      = bid->dbno;
	cbucket->NumId[j]->offset    = bid->offset;
	
        if(cache->refcount)
        {
            for(iref=0; iref < cache->refcount; iref++)
                cbucket->NumId[j]->refoffsets[iref] = bid->refoffsets[iref];
        }

	++j;


	++cbucket->Nentries;
	AJFREE(bid);
    }
    
    if((iold < order) && parray[iold])
        newparray[i] = parray[iold++];
    else
        newparray[i] = cache->totsize;

    btreeWriteNumbucket(cache,cbucket,newparray[i]);

#if AJINDEX_DEBUG
    ajDebug("end of NumSplitleaf i:%u iold:%u, order:%u\n", i, iold, order);
#endif
    nkeys = rbucketlimit;

    nodetype = BT_SECLEAF;
    v = nodetype;
    SBT_NODETYPE(rbuf,v);
    lv = prevsave;
    SBT_PREV(rbuf,lv);
    lv = overflow;
    SBT_OVERFLOW(rbuf,lv);
    
    btreeWriteNumNode(cache,rpage,newnarray,newparray,nkeys);
    rpage->dirty = BT_DIRTY;

    btreeNumbucketDel(&cbucket);
    ajListFree(&idlist);



    medianless = lblockno;
    mediangtr  = rblockno;


    if(rootnodetype == BT_SECROOT)
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


    page = btreeSeccacheRead(cache,prevsave);
    btreeNumInsertKey(cache,page,mediankey,medianless,mediangtr);


    page = btreeSeccacheRead(cache,prevsave);

    return page;
}




/* @funcstatic btreeFreePriArray **********************************************
**
** Free karray and parray arrays for a primary key
**
** @param [u] cache [AjPBtcache] cache
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void btreeFreePriArray(AjPBtcache cache)
{
    AjPBtMem p;
    AjPBtMem next;
    ajuint i;

    /* ajDebug("In btreeFreePriArray\n"); */
    
    if(!cache->bmem)
	return;

    p = cache->bmem;
    
    while(p)
    {
        next = p->next;

        AJFREE(p->parray);
	AJFREE(p->overflows);

	for(i=0;i<cache->porder;++i)
	    ajStrDel(&p->karray[i]);

	AJFREE(p->karray);
	AJFREE(p);
	p = next;
    }
    

    cache->bmem = NULL;
    cache->tmem = NULL;

    return;
}




/* @funcstatic  btreeFreeSecArray *********************************************
**
** Free karray and parray arrays for a secondary key
**
** @param [u] cache [AjPBtcache] cache
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void btreeFreeSecArray(AjPBtcache cache)
{
    AjPBtMem p;
    AjPBtMem next;
    ajuint i;

    /* ajDebug("In btreeFreeSecArray\n"); */
    
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




/* @funcstatic btreeIdentFetchMulti *******************************************
**
** Read the leaves of a secondary hybrid tree
**
** @param [u] cache [AjPBtcache] cache
** @param [r] idname [const AjPStr] id name
** @param [r] rootblock [ajulong] root page of secondary tree
** @param [u] list [AjPList] list to add BtIDs to
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void btreeIdentFetchMulti(AjPBtcache cache, const AjPStr idname,
                                 ajulong rootblock,
                                 AjPList list)
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

    page = btreeSeccacheRead(cache, rootblock);
    buf = page->buf;

    GBT_RIGHT(buf,&level);
    cache->slevel = (ajuint) level;

    btreeGetNumPointers(cache,buf,&parray);
    GBT_NODETYPE(buf,&nodetype);

    while(nodetype != BT_SECLEAF && cache->slevel != 0)
    {
	page = btreeSeccacheRead(cache,parray[0]);
	buf = page->buf;
	btreeGetNumPointers(cache,buf,&parray);
	GBT_NODETYPE(buf,&nodetype);
    }

    do
    {
	GBT_NKEYS(buf,&nkeys);

	for(i=0;i<=nkeys;++i)
	    btreeNumbucketBtidlist(cache,parray[i],idname,list);

	right = 0L;
	if(cache->slevel)
	{
	    GBT_RIGHT(buf,&right);

	    if(right)
	    {
		page = btreeSeccacheRead(cache,right);
		buf = page->buf;
		btreeGetNumPointers(cache,buf,&parray);
	    }
	}
    } while(right);

    btreeDeallocSecArray(cache,array);

    return;
}




/* @funcstatic btreeIdentFetchMultiHit ****************************************
**
** Read the leaves of a secondary hybrid tree
**
** @param [u] cache [AjPBtcache] cache
** @param [r] rootblock [ajulong] root page of secondary tree
** @param [u] list [AjPList] list to add BtHits to
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void btreeIdentFetchMultiHit(AjPBtcache cache,
                                    ajulong rootblock,
                                    AjPList list)
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

    page = btreeSeccacheRead(cache, rootblock);
    buf = page->buf;

    GBT_RIGHT(buf,&level);
    cache->slevel = (ajuint) level;

    btreeGetNumPointers(cache,buf,&parray);
    GBT_NODETYPE(buf,&nodetype);

    while(nodetype != BT_SECLEAF && cache->slevel != 0)
    {
	page = btreeSeccacheRead(cache,parray[0]);
	buf = page->buf;
	btreeGetNumPointers(cache,buf,&parray);
	GBT_NODETYPE(buf,&nodetype);
    }

    do
    {
	GBT_NKEYS(buf,&nkeys);

	for(i=0;i<=nkeys;++i)
	    btreeNumbucketBthitlist(cache,parray[i],list);

	right = 0L;
	if(cache->slevel)
	{
	    GBT_RIGHT(buf,&right);

	    if(right)
	    {
		page = btreeSeccacheRead(cache,right);
		buf = page->buf;
		btreeGetNumPointers(cache,buf,&parray);
	    }
	}
    } while(right);

    btreeDeallocSecArray(cache,array);

    return;
}




/* @funcstatic btreeIdentFetchMultiHitref *************************************
**
** Read the leaves of a secondary hybrid tree
**
** @param [u] cache [AjPBtcache] cache
** @param [r] rootblock [ajulong] root page of secondary tree
** @param [u] list [AjPList] list to add BtHitrefs to
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void btreeIdentFetchMultiHitref(AjPBtcache cache,
                                       ajulong rootblock,
                                       AjPList list)
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

    page = btreeSeccacheRead(cache, rootblock);
    buf = page->buf;

    GBT_RIGHT(buf,&level);
    cache->slevel = (ajuint) level;

    btreeGetNumPointers(cache,buf,&parray);
    GBT_NODETYPE(buf,&nodetype);

    while(nodetype != BT_SECLEAF && cache->slevel != 0)
    {
	page = btreeSeccacheRead(cache,parray[0]);
	buf = page->buf;
	btreeGetNumPointers(cache,buf,&parray);
	GBT_NODETYPE(buf,&nodetype);
    }

    do
    {
	GBT_NKEYS(buf,&nkeys);

	for(i=0;i<=nkeys;++i)
	    btreeNumbucketBthitreflist(cache,parray[i],list);

	right = 0L;
	if(cache->slevel)
	{
	    GBT_RIGHT(buf,&right);

	    if(right)
	    {
		page = btreeSeccacheRead(cache,right);
		buf = page->buf;
		btreeGetNumPointers(cache,buf,&parray);
	    }
	}
    } while(right);

    btreeDeallocSecArray(cache,array);

    return;
}




/* @func ajBtreeDumpIdentifiers ***********************************************
**
** Read the leaves of an identifier tree
**
** @param [u] cache [AjPBtcache] cache
** @param [r] dmin [ajuint] minimum number of times the key should appear
** @param [r] dmax [ajuint] maximum number of times the key should appear
** @param [u] outf [AjPFile] output file
**
** @return [void]
**
** @release 5.0.0
** @@
******************************************************************************/

void ajBtreeDumpIdentifiers(AjPBtcache cache, ajuint dmin, ajuint dmax,
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
    
    AjPIdbucket bucket;
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

    page = btreePricacheRead(cache, 0L);
    buf = page->buf;

    btreeGetKeys(cache,buf,&karray,&parray);
    GBT_NODETYPE(buf,&nodetype);

    while(nodetype != BT_LEAF && cache->plevel != 0)
    {
	page = btreePricacheRead(cache,parray[0]);
	buf = page->buf;
	btreeGetKeys(cache,buf,&karray,&parray);
	GBT_NODETYPE(buf,&nodetype);
    }

    do
    {
	GBT_NKEYS(buf,&nkeys);
	for(i=0;i<=nkeys;++i)
	{
	    bucket = btreeReadIdbucket(cache,parray[i]);
            btreeIdbucketSort(bucket);

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

	    btreeIdbucketDel(&bucket);
	}

	right = 0L;

	if(cache->plevel)
	{
	    GBT_RIGHT(buf,&right);

	    if(right)
	    {
		page = btreePricacheRead(cache,right);
		buf = page->buf;
		btreeGetKeys(cache,buf,&karray,&parray);
	    }
	}
    } while(right);

    btreeDeallocPriArray(cache,array);

    return;
}




/* @func ajBtreeDeleteIdent ***************************************************
**
** Entry point for hybrid ID deletion.
**
** Deletion software
**
** @param [u] cache [AjPBtcache] cache
** @param [r] btid [const AjPBtId] hybrid object
**
** @return [AjBool] True if found and deleted
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ajBtreeDeleteIdent(AjPBtcache cache, const AjPBtId btid)
{
    AjPBtpage rootpage = NULL;
    AjPBtpage spage   = NULL;
    AjPStr key        = NULL;
    AjPIdbucket bucket  = NULL;
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
    
    /* ajDebug("In ajBtreeDeleteIdent\n"); */


    if(!ajStrGetLen(btid->id))
	return ajFalse;

    spage = btreeIdentFind(cache,btid->id);
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

    while(i!=nkeys && MAJSTRCMPS(btid->id,karray[i])>=0)
	++i;

    blockno = parray[i];


    bucket = btreeReadIdbucket(cache,blockno);
    
    nentries = bucket->Nentries;
    
    found = ajFalse;

    for(i=0;i<nentries;++i)
	if(ajStrMatchS(btid->id,bucket->Ids[i]->id))
	{
	    found = ajTrue;
	    break;
	}
    

    if(!found)
    {
	btreeDeallocPriArray(cache,arrays);

	return ajFalse;
    }
    

    dups = bucket->Ids[i]->dups;
    

    if(!dups)
    {
        /* ajDebug("No secondary tree\n"); */
        rootpage = btreePricacheLocate(cache,0L);

        if(!rootpage)
            ajFatal("Rootpage has been unlocked (ajBtreeDeleteHybId)");
    
        rootpage->dirty = BT_LOCK;
        rootpage->lockfor = 1711;

        btreeFindIdentBalanceOne(cache,0L,BTNO_NODE,BTNO_NODE,BTNO_NODE,
                                 BTNO_NODE,btid);


        btreeDeallocPriArray(cache,arrays);

        if(cache->dodelete)
            ret = ajTrue;
        else
            ret = ajFalse;
    }
    else
    {
        did = bucket->Ids[i];
        secrootpage = did->offset;
        cache->secrootblock = secrootpage;

        ret = btreeDeleteIdentIdTwo(cache,btid,did);

        btreeWriteIdbucket(cache,bucket,blockno);
    }
    

    return ret;
}




/* @funcstatic btreeFindIdentBalanceOne ***************************************
**
** Master routine for entry deletion from level 1 identifier tree.
**
** Deletion software.
**
** @param [u] cache [AjPBtcache] cache
** @param [r] thisNode [ajulong] Current node
** @param [r] leftNode [ajulong] Node to left
** @param [r] rightNode [ajulong] Node to right
** @param [r] lAnchor [ajulong] Left anchor
** @param [r] rAnchor [ajulong] Right anchor
** @param [r] btid [const AjPBtId] Id object
**
** @return [ajulong] page number or BTNO_NODE
**
** @release 6.1.0
** @@
******************************************************************************/

static ajulong btreeFindIdentBalanceOne(AjPBtcache cache, ajulong thisNode,
                                        ajulong leftNode, ajulong rightNode,
                                        ajulong lAnchor, ajulong rAnchor,
                                        const AjPBtId btid)
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
    
    AjBool existed = ajFalse;
    
    /* ajDebug("In btreeFindBalance\n"); */

    if(thisNode)
	page = btreePricacheRead(cache,thisNode);
    else
    {   /* It's the root node of the primary hyb tree */
        /* Needs altering for secondary tree          */
	page = btreePricacheLocate(cache,thisNode);
	page->dirty = BT_LOCK;
        page->lockfor = 1721;
    }

    cache->dodelete = ajFalse;

    buf = page->buf;
    GBT_NKEYS(buf,&nkeys);

    order = cache->porder;
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

    btreeGetKeys(cache,buf,&karray,&parray);

    i=0;

    while(i!=nkeys && MAJSTRCMPS(btid->id,karray[i])>=0)
	++i;

    blockno = parray[i];

    nextNode = blockno;
    ptrSave = i;

    GBT_NODETYPE(buf,&nodetype);

    if(!(nodetype == BT_LEAF) && !(nodetype == BT_ROOT && !cache->plevel))
    {
	if(nextNode == parray[0])
	{
	    if(leftNode != BTNO_NODE)
	    {
		page1 = btreePricacheRead(cache,leftNode);
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
		page1 = btreePricacheRead(cache,rightNode);
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
	if(nodetype != BT_LEAF && cache->plevel)
	{
	    i=0;

	    while(i!=nkeys && MAJSTRCMPS(btid->id,karray[i]))
		++i;

	    if(i!=nkeys)
	    {
		btreeFindHybMinOne(cache,parray[i+1],btid->id);
		ajStrAssignS(&karray[i],cache->replace);
		btreeWriteNode(cache,page,karray,parray,nkeys);
	    }
	
	}
	
	btreeFindIdentBalanceOne(cache,nextNode,nextLeft,nextRight,
                                 nextAncL,nextAncR,btid);

	if(thisNode)
	    page = btreePricacheRead(cache,thisNode);
	else
	{
	    page = btreePricacheLocate(cache,thisNode);
	    page->dirty = BT_LOCK;
            page->lockfor = 1722;
	}
	buf = page->buf;

    }
    else
    {
	if(nodetype == BT_LEAF || (nodetype==BT_ROOT && !cache->plevel))
	{
	    existed = btreeRemoveIdentEntryOne(cache,thisNode,btid);

	    if(existed)
		cache->dodelete = ajTrue;

	    GBT_NKEYS(buf,&nkeys);

	    if(nkeys >= minkeys || (nodetype==BT_ROOT && !cache->plevel))
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
**
** @release 6.1.0
** @@
******************************************************************************/

static void btreeFindHybMinOne(AjPBtcache cache, ajulong pagepos,
                               const AjPStr key)
{
    AjPBtpage page   = NULL;
    AjPIdbucket bucket = NULL;
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

    page = btreePricacheRead(cache,pagepos);
    buf  = page->buf;
    GBT_NODETYPE(buf,&nodetype);
    GBT_NKEYS(buf,&nkeys);

    btreeGetKeys(cache,buf,&karray,&parray);

    if(nodetype == BT_LEAF)
    {
	bucket = btreeReadIdbucket(cache,parray[0]);
	nentries = bucket->Nentries;

        /*
        ** If there's only one entry then it must be the key marked
        ** for deletion
        */
        if(nentries<2)
	{
	    btreeIdbucketDel(&bucket);
	    bucket = btreeReadIdbucket(cache,parray[1]);
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
	btreeIdbucketDel(&bucket);
    }
    else
    {
	pagepos = parray[0];
	btreeFindHybMinOne(cache,pagepos,key);
	
    }

    btreeDeallocPriArray(cache,arrays);
    
    return;
}




/* @funcstatic btreeRemoveIdentEntryOne ***************************************
**
** Find and delete an ID from a given identifier tree level 1 leaf node if
** necessary.
**
** Deletion software
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] leaf node page
** @param [r] btid [const AjPBtId] id object
**
** @return [AjBool] True if found (and deleted)
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool btreeRemoveIdentEntryOne(AjPBtcache cache, ajulong pagepos,
                                       const AjPBtId btid)
{
    AjPBtpage page   = NULL;
    AjPIdbucket bucket = NULL;
    
    AjPStr *karray = NULL;
    ajulong *parray = NULL;
    ajulong blockno = 0L;
    
    ajuint nkeys    = 0;
    ajuint nentries = 0;
    ajuint i;

    ajuint dirtysave = 0;
    
    AjBool found = ajFalse;

    unsigned char *buf = NULL;
    AjPBtMem arrays = NULL;
    
    /* ajDebug("In btreeRemoveIdentEntryOne\n"); */

    page = btreePricacheRead(cache,pagepos);
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
    
    i=0;

    while(i!=nkeys && MAJSTRCMPS(btid->id,karray[i])>=0)
	++i;

    blockno = parray[i];
    
    bucket = btreeReadIdbucket(cache,blockno);

    nentries = bucket->Nentries;
    found = ajFalse;

    for(i=0;i<nentries;++i)
	if(!MAJSTRCMPS(btid->id,bucket->Ids[i]->id))
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

        
	btreeWriteIdbucket(cache,bucket,blockno);

	btreeAdjustHybbucketsOne(cache,page);

	page->dirty = BT_DIRTY;
    }
    else
	page->dirty = dirtysave;

    btreeIdbucketDel(&bucket);

    btreeDeallocPriArray(cache,arrays);
    
    if(!found)
	return ajFalse;

    return ajTrue;
}




/* @funcstatic btreeAdjustHybbucketsOne ***************************************
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
**
** @release 6.1.0
** @@
******************************************************************************/

static void btreeAdjustHybbucketsOne(AjPBtcache cache, AjPBtpage leaf)
{
    ajuint nkeys = 0;
    unsigned char *lbuf = NULL;
    AjPIdbucket *buckets  = NULL;

    AjPBtMem arrays = NULL;    
    AjPStr *keys        = NULL;
    ajulong *ptrs        = NULL;
    ajulong *overflows   = NULL;
    
    ajuint i = 0;
    ajuint j = 0;
    ajuint iref = 0;

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
    AjPIdbucket cbucket = NULL;
    AjPBtId cid       = NULL;

    ajuint v = 0;
    ajuint refskip = cache->refcount*BT_EXTRA;
    
    /* ajDebug("In btreeAdjustHybbucketsOne\n"); */

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

    order = cache->porder;
    nperbucket = cache->pnperbucket;
    

    /* Read keys/ptrs */

    arrays = btreeAllocPriArray(cache);
    keys = arrays->karray;
    ptrs = arrays->parray;
    overflows = arrays->overflows;

    btreeGetKeys(cache,lbuf,&keys,&ptrs);

    for(i=0;i<=nkeys;++i)
	totalkeys += btreeIdbucketCount(cache,ptrs[i]);

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
	buckets[i] = btreeReadIdbucket(cache,ptrs[i]);


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

    ajListSort(idlist, &btreeIdCompare);
    AJFREE(buckets);

    cbucket = btreeIdbucketNew(maxnperbucket, cache->refcount);
    bucketlimit = bucketn - 1;

    totnids = 0;
    nids = (ajuint) ajListGetLength(idlist);


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

	    if(cache->refcount)
            {
                for(iref=0; iref < cache->refcount; iref++)
                    cid->refoffsets[iref] = bid->refoffsets[iref];
            }

	    cbucket->keylen[count] = BT_BUCKIDLEN(bid->id) + refskip;
	    ++cbucket->Nentries;
	    ++count;
	    ++totnids;
	    ajBtreeIdDel(&bid);
	}


	totkeylen += ajStrGetLen(keys[0]);

	if(!ptrs[1])
	    ptrs[1] = cache->totsize;

	btreeWriteIdbucket(cache,cbucket,ptrs[1]);

	cbucket->Overflow = overflows[0];
	cbucket->Nentries = 0;

	if(!ptrs[0])
	    ptrs[0] = cache->totsize;

	btreeWriteIdbucket(cache,cbucket,ptrs[0]);
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
		
                if(cache->refcount)
                {
                    for(iref=0; iref < cache->refcount; iref++)
                        cid->refoffsets[iref] = bid->refoffsets[iref];
                }

		cbucket->keylen[count] = BT_BUCKIDLEN(bid->id) + refskip;
		++cbucket->Nentries;
		++count;
		ajBtreeIdDel(&bid);
	    }


	    ajListPeek(idlist,(void **)&bid);
	    ajStrAssignS(&keys[i],bid->id);


	    totkeylen += ajStrGetLen(bid->id);

	    if(!ptrs[i])
		ptrs[i] = cache->totsize;
	    btreeWriteIdbucket(cache,cbucket,ptrs[i]);
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

            if(cache->refcount)
            {
                for(iref=0; iref < cache->refcount; iref++)
                    cid->refoffsets[iref] = bid->refoffsets[iref];
            }

	    ++cbucket->Nentries;
	    ++count;
	    ajBtreeIdDel(&bid);
	}
	
	
	if(!ptrs[i])
	    ptrs[i] = cache->totsize;
	
	btreeWriteIdbucket(cache,cbucket,ptrs[i]);
    }
    

    cbucket->Nentries = maxnperbucket;
    btreeIdbucketDel(&cbucket);

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

    btreeIdbucketDel(&cbucket);
    ajListFree(&idlist);

    return;
}




/* @funcstatic btreeRebalanceHybOne *******************************************
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
**
** @release 6.1.0
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
	lpage = btreePricacheRead(cache,leftNode);
	lbuf  = lpage->buf;
	GBT_NKEYS(lbuf,&lnkeys);
    }
    

    if(rightok)
    {
	rpage = btreePricacheRead(cache,rightNode);
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

    
    order = cache->porder;
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
	tpage = btreePricacheRead(cache,thisNode);
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
**
** @release 6.1.0
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


    pageA = btreePricacheRead(cache,anchorNode);
    pageA->dirty = BT_LOCK;
    pageA->lockfor = 1751;
    abuf = pageA->buf;
    pageB = btreePricacheRead(cache,balanceNode);
    pageB->dirty = BT_LOCK;
    pageB->lockfor = 1752;
    bbuf = pageB->buf;
    pageT = btreePricacheRead(cache,thisNode);
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
	page = btreePricacheRead(cache,pTarray[i]);
	buf = page->buf;
	GBT_NODETYPE(buf,&nodetype);

	if(nodetype != BT_IDBUCKET)
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
**
** @release 6.1.0
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

    pageA = btreePricacheRead(cache,anchorNode);
    saveA = pageA->dirty;
    pageA->dirty = BT_LOCK;
    pageA->lockfor = 1761;
    abuf = pageA->buf;
    pageN = btreePricacheRead(cache,mergeNode);
    saveN = pageN->dirty;
    pageN->dirty = BT_LOCK;
    pageN->lockfor = 1762;
    nbuf = pageN->buf;
    pageT = btreePricacheRead(cache,thisNode);
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

    for(i=0;i<=nNkeys;++i)
    {
	page = btreePricacheRead(cache,pNarray[i]);
	buf = page->buf;
	GBT_NODETYPE(buf,&nodetype);

	if(nodetype != BT_IDBUCKET)
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
**
** @release 6.1.0
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
    
    if(!cache->plevel)
	return BTNO_NODE;

    rootpage = btreePricacheLocate(cache,0L);
    buf = rootpage->buf;
    page = btreePricacheRead(cache,pagepos);


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
    
    --cache->plevel;

    if(cache->plevel)
    {
	/*
	 ** Update the PREV pointers of the new root's children
	 */
	GBT_NKEYS(buf,&nkeys);
	btreeGetKeys(cache,buf,&karray,&parray);
	for(i=0;i<=nkeys;++i)
	{
	    page = btreePricacheRead(cache,parray[i]);
	    lbuf = page->buf;
	    SBT_PREV(lbuf,prev);
	    page->dirty = BT_DIRTY;
	}
    }

    btreeDeallocPriArray(cache,arrays);
    
    return 0L;
}




/* @funcstatic btreeDeleteIdentIdTwo ******************************************
**
** Entry point for secondary hybrid tree ID deletion.
** Assumes cache->secrootblock has been initialised.
**
** Deletion software
**
** @param [u] cache [AjPBtcache] cache
** @param [r] btid [const AjPBtId] ID object
** @param [u] did [AjPBtId] ID object in primary tree
**
** @return [AjBool] True if found and deleted
**
** @release 6.1.0
** @@
******************************************************************************/

static AjBool btreeDeleteIdentIdTwo(AjPBtcache cache, const AjPBtId btid,
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
    ajuint iref;

    AjBool found = ajFalse;
    
    AjPBtNumId num      = NULL;
    AjPNumbucket bucket = NULL;
    
    AjPBtMem array = NULL;
    
    rpage = btreeSeccacheWrite(cache,cache->secrootblock);

    rpage->dirty = BT_LOCK;
    rpage->lockfor = 1781;
    rbuf = rpage->buf;
    GBT_RIGHT(rbuf,&sval);
    cache->slevel = (ajuint) sval;

    key  = btid->offset;

    page = btreeNumFind(cache,key);
    buf  = page->buf;

    GBT_NKEYS(buf,&nkeys);

    array  = btreeAllocSecArray(cache);

    karray = array->overflows;
    parray = array->parray;
    
    if(!nkeys)
    {
        ajWarn("btreeDeleteIdentIdTwo: No keys in findinsert node");
        btreeDeallocSecArray(cache,array);
        rpage->dirty = BT_CLEAN;

        return ajFalse;
    }

    btreeGetNumKeys(cache,buf,&karray,&parray);

    i = 0;

    while(i != nkeys && key >= karray[i])
        ++i;

    blockno = parray[i];

    bucket = btreeReadNumbucket(cache,blockno);
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
        ajWarn("btreeDeleteIdentIdTwo: Numeric key not in bucket");
        btreeDeallocSecArray(cache,array);
        rpage->dirty = BT_CLEAN;

        return ajFalse;
    }

    btreeFindHybBalanceTwo(cache,cache->secrootblock,BTNO_NODE,BTNO_NODE,
                           BTNO_NODE,BTNO_NODE,key);

    if(!cache->dodelete)
    {
        ajWarn("btreeDeleteIdentIdTwo: entry %Lu not deleted",key);
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
    bucket = btreeReadNumbucket(cache,parray[0]);

    if(!bucket->Nentries)
        bucket = btreeReadNumbucket(cache,parray[1]);

    if(bucket->Nentries != 1)
        ajFatal("Expected only one remaining entry in btreeDeleteIdentIdTwo");
    
    num = bucket->NumId[0];

    if(cache->refcount)
    {
        for(iref=0; iref < cache->refcount; iref++)
            did->refoffsets[iref] = num->refoffsets[iref];
    }
    
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




/* @funcstatic btreeFindHybBalanceTwo *****************************************
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
**
** @release 6.1.0
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
	page = btreeSeccacheRead(cache,thisNode);
    else
    {   /* It's the root node of the primary hyb tree */
	page = btreeSeccacheLocate(cache,thisNode);
	page->dirty = BT_LOCK;
        page->lockfor = 1791;
    }

    cache->dodelete = ajFalse;

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

    if(!(nodetype == BT_SECLEAF) && !(nodetype == BT_SECROOT && !cache->slevel))
    {
	if(nextNode == parray[0])
	{
	    if(leftNode != BTNO_NODE)
	    {
		page1 = btreeSeccacheRead(cache,leftNode);
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
		page1 = btreeSeccacheRead(cache,rightNode);
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
	if(nodetype != BT_SECLEAF && cache->slevel)
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
	    page = btreeSeccacheRead(cache,thisNode);
	else
	{
	    page = btreeSeccacheLocate(cache,thisNode);
	    page->dirty = BT_LOCK;
            page->lockfor = 1792;
	}
	buf = page->buf;

    }
    else
    {
	if(nodetype == BT_SECLEAF || (nodetype==BT_SECROOT && !cache->slevel))
	{
	    existed = btreeRemoveHybEntryTwo(cache,thisNode,key);

	    if(existed)
		cache->dodelete = ajTrue;

	    GBT_NKEYS(buf,&nkeys);

	    if(nkeys >= minkeys || (nodetype==BT_SECROOT && !cache->slevel))
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
**
** @release 6.1.0
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

    AjPNumbucket bucket = NULL;
    
    unsigned char *buf = NULL;

    /* ajDebug("In btreeFindHybMinTwo\n"); */

    arrays = btreeAllocSecArray(cache);
    karray = arrays->overflows;
    parray = arrays->parray;

    page = btreeSeccacheRead(cache,pagepos);
    buf  = page->buf;
    GBT_NODETYPE(buf,&nodetype);
    GBT_NKEYS(buf,&nkeys);

    btreeGetNumKeys(cache,buf,&karray,&parray);

    if(nodetype == BT_SECLEAF)
    {
	bucket = btreeReadNumbucket(cache,parray[0]);
	nentries = bucket->Nentries;

        /*
        ** If there's only one entry then it must be the key marked
        ** for deletion
        */
        if(nentries<2)
	{
	    btreeNumbucketDel(&bucket);
	    bucket = btreeReadNumbucket(cache,parray[1]);
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

	btreeNumbucketDel(&bucket);
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
**
** @release 6.1.0
** @@
******************************************************************************/

static AjBool btreeRemoveHybEntryTwo(AjPBtcache cache, ajulong pagepos,
                                     ajulong key)
{
    AjPBtpage page   = NULL;
    AjPNumbucket bucket = NULL;
    
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

    page = btreeSeccacheRead(cache,pagepos);
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
    
    bucket = btreeReadNumbucket(cache,blockno);


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

	btreeWriteNumbucket(cache,bucket,blockno);
	btreeAdjustHybbucketsTwo(cache,page);
	page->dirty = BT_DIRTY;
    }
    else
	page->dirty = dirtysave;

    btreeNumbucketDel(&bucket);

    btreeDeallocSecArray(cache,arrays);
    
    if(!found)
	return ajFalse;

    return ajTrue;
}




/* @funcstatic btreeAdjustHybbucketsTwo ***************************************
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
**
** @release 6.1.0
** @@
******************************************************************************/

static void btreeAdjustHybbucketsTwo(AjPBtcache cache, AjPBtpage leaf)
{
    ajuint nkeys = 0;
    unsigned char *lbuf = NULL;
    AjPNumbucket *buckets  = NULL;

    AjPBtMem arrays = NULL;    
    AjPBtMem overarrays = NULL;    

    ajulong *keys           = NULL;
    ajulong *ptrs           = NULL;
    ajulong *overflows      = NULL;
    
    ajuint i = 0;
    ajuint j = 0;
    ajuint iref = 0;
    
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
    AjPNumbucket cbucket = NULL;
    AjPBtNumId cid       = NULL;

    ajuint v = 0;
    
    /* ajDebug("In btreeAdjustHybbucketsTwo\n"); */

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
	totalkeys += btreeNumInNumbucket(cache,ptrs[i]);

    totalkeys += btreeNumInNumbucket(cache,ptrs[i]);


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
	buckets[i] = btreeReadNumbucket(cache,ptrs[i]);


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

    ajListSort(idlist, &btreeNumIdCompare);
    AJFREE(buckets);

    cbucket = btreeNumbucketNew(maxnperbucket, cache->refcount);
    bucketlimit = bucketn - 1;

    totnids = 0;
    nids = (ajuint) ajListGetLength(idlist);


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
            if(cache->refcount)
            {
                for(iref=0; iref < cache->refcount; iref++)
                    cid->refoffsets[iref] = bid->refoffsets[iref];
            }
	    
	    ++cbucket->Nentries;
	    ++count;
	    ++totnids;
	    AJFREE(bid);
	}


	if(!ptrs[1])
	    ptrs[1] = cache->totsize;

	btreeWriteNumbucket(cache,cbucket,ptrs[1]);

	cbucket->Overflow = overflows[0];
	cbucket->Nentries = 0;

	if(!ptrs[0])
	    ptrs[0] = cache->totsize;

	btreeWriteNumbucket(cache,cbucket,ptrs[0]);
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

                if(cache->refcount)
                {
                    for(iref=0; iref < cache->refcount; iref++)
                        cid->refoffsets[iref] = bid->refoffsets[iref];
		}

		++cbucket->Nentries;
		++count;
		AJFREE(bid);
	    }


	    ajListPeek(idlist,(void **)&bid);
	    keys[i] = bid->offset;


	    if(!ptrs[i])
		ptrs[i] = cache->totsize;

	    btreeWriteNumbucket(cache,cbucket,ptrs[i]);
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

            if(cache->refcount)
            {
                for(iref=0; iref < cache->refcount; iref++)
                    cid->refoffsets[iref] = bid->refoffsets[iref];
	    }

	    ++cbucket->Nentries;
	    ++count;
	    AJFREE(bid);
	}
	
	
	if(!ptrs[i])
	    ptrs[i] = cache->totsize;
	
	btreeWriteNumbucket(cache,cbucket,ptrs[i]);
    }
    

    cbucket->Nentries = maxnperbucket;
    btreeNumbucketDel(&cbucket);

    /* Now write out a modified leaf with new keys/ptrs */

    nkeys = bucketn - 1;
    v = nkeys;
    SBT_NKEYS(lbuf,v);

    btreeWriteNumNode(cache,leaf,keys,ptrs,nkeys);

    leaf->dirty = dirtysave;
    if(nodetype == BT_SECROOT)
    {
	leaf->dirty = BT_LOCK;
        leaf->lockfor = 1812;
    }
    
    btreeDeallocSecArray(cache,arrays);
    btreeDeallocSecArray(cache,overarrays);

    btreeNumbucketDel(&cbucket);
    ajListFree(&idlist);

    return;
}




/* @funcstatic btreeRebalanceHybTwo *******************************************
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
**
** @release 6.1.0
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
	lpage = btreeSeccacheRead(cache,leftNode);
	lbuf  = lpage->buf;
	GBT_NKEYS(lbuf,&lnkeys);
    }
    

    if(rightok)
    {
	rpage = btreeSeccacheRead(cache,rightNode);
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
	tpage = btreeSeccacheRead(cache,thisNode);
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
**
** @release 6.1.0
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


    pageA = btreeSeccacheRead(cache,anchorNode);
    pageA->dirty = BT_LOCK;
    pageA->lockfor = 1821;
    abuf = pageA->buf;
    pageB = btreeSeccacheRead(cache,balanceNode);
    pageB->dirty = BT_LOCK;
    pageB->lockfor = 1822;
    bbuf = pageB->buf;
    pageT = btreeSeccacheRead(cache,thisNode);
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
	page = btreeSeccacheRead(cache,pTarray[i]);
	buf = page->buf;
	GBT_NODETYPE(buf,&nodetype);

	if(nodetype != BT_IDBUCKET)
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
**
** @release 6.1.0
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

    pageA = btreeSeccacheRead(cache,anchorNode);
    saveA = pageA->dirty;
    pageA->dirty = BT_LOCK;
    pageA->lockfor = 1831;
    abuf = pageA->buf;
    pageN = btreeSeccacheRead(cache,mergeNode);
    saveN = pageN->dirty;
    pageN->dirty = BT_LOCK;
    pageN->lockfor = 1832;
    nbuf = pageN->buf;
    pageT = btreeSeccacheRead(cache,thisNode);
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

    for(i=0;i<=nNkeys;++i)
    {
	page = btreeSeccacheRead(cache,pNarray[i]);
	buf = page->buf;
	GBT_NODETYPE(buf,&nodetype);

	if(nodetype != BT_IDBUCKET)
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
**
** @release 6.1.0
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

    rootpage = btreeSeccacheLocate(cache,cache->secrootblock);
    buf = rootpage->buf;
    page = btreeSeccacheRead(cache,pagepos);


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
    nodetype = BT_SECINTERNAL;
    SBT_NODETYPE(buf,nodetype);

    page->pagepos = cache->secrootblock;
    page->dirty = BT_LOCK;
    page->lockfor = 1841;
    buf = page->buf;
    nodetype = BT_SECROOT;
    SBT_NODETYPE(buf,nodetype);
    
    --cache->slevel;

    if(cache->slevel)
    {
	/*
	 ** Update the PREV pointers of the new root's children
	 */
	GBT_NKEYS(buf,&nkeys);
	btreeGetNumKeys(cache,buf,&karray,&parray);

	for(i=0;i<=nkeys;++i)
	{
	    page = btreeSeccacheRead(cache,parray[i]);
	    lbuf = page->buf;
	    SBT_PREV(lbuf,prev);
	    page->dirty = BT_DIRTY;
	}
    }

    btreeDeallocSecArray(cache,arrays);

    cache->secrootblock = pagepos;
    
    return 0L;
}




/* @func ajBtreeDeletePriId ***************************************************
**
** Entry point for keyword tree ID deletion.
**
** Deletion software
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pri [const AjPBtPri] keyword object
**
** @return [AjBool] True if found and deleted
**
** @release 6.1.0
** @@
******************************************************************************/

AjBool ajBtreeDeletePriId(AjPBtcache cache, const AjPBtPri pri)
{
    AjPBtpage rootpage = NULL;
    AjPBtpage spage    = NULL;
    AjPBtpage page     = NULL;
    AjPStr key         = NULL;
    AjPSecbucket bucket  = NULL;
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

    if(!btreeKeyFind(cache,key,&treeblock))
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
    
    page = btreePricacheRead(cache,secrootpage);
    page->dirty = BT_LOCK;
    page->lockfor = 1851;
    buf = page->buf;

    GBT_RIGHT(buf,&slevel);
    cache->slevel = (ajuint)slevel;

    spage = btreeKeyidFind(cache,pri->id);
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

    bucket = btreeReadSecbucket(cache,blockno);

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
    ** Needs a little thought. Maybe use cache->dodelete for multiple
    ** purposes i.e. 0=not deleted  1=keyword deleted 2=tree deleted
    */

    rootpage = btreeSeccacheLocate(cache,secrootpage);
    if(!rootpage)
        ajFatal("DeletePriId: secondary root page became unlocked");
    rootpage->dirty = BT_LOCK;
    rootpage->lockfor = 1852;
    buf = rootpage->buf;

    GBT_RIGHT(buf,&slevel);
    cache->slevel = (ajuint)slevel;

    btreeFindPriBalanceTwo(cache,secrootpage,BTNO_NODE,BTNO_NODE,BTNO_NODE,
                           BTNO_NODE,pri);

    ret = cache->dodelete;

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
        prirootpage = btreePricacheLocate(cache,0L);

        if(!prirootpage)
            ajFatal("ajBtreeDeletePriId: prirootpage unlocked");
        
    
        btreeFindPriBalanceOne(cache,0L,BTNO_NODE,BTNO_NODE,BTNO_NODE,
                               BTNO_NODE,pri);

        ret = cache->dodelete;
    }
    

    btreeDeallocSecArray(cache,arrays);
    ajStrDel(&key);

    return ret;
}




/* @funcstatic btreeFindPriBalanceTwo *****************************************
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
**
** @release 6.1.0
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
	page = btreeSeccacheRead(cache,thisNode);
    else
    {
	page = btreeSeccacheLocate(cache,thisNode);
	page->dirty = BT_LOCK;
        page->lockfor = 1861;
    }

    cache->dodelete = ajFalse;

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

    if(!(nodetype == BT_SECLEAF) && !(nodetype == BT_SECROOT && !cache->slevel))
    {
	if(nextNode == parray[0])
	{
	    if(leftNode != BTNO_NODE)
	    {
		page1 = btreeSeccacheRead(cache,leftNode);
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
		page1 = btreeSeccacheRead(cache,rightNode);
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
	if(nodetype != BT_SECLEAF && cache->slevel)
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
	    page = btreeSeccacheRead(cache,thisNode);
	else
	{
	    page = btreeSeccacheLocate(cache,thisNode);
	    page->dirty = BT_LOCK;
            page->lockfor = 1862;
	}
	buf = page->buf;

    }
    else
    {
	if(nodetype == BT_SECLEAF || (nodetype==BT_SECROOT && !cache->slevel))
	{
	    existed = btreeRemovePriEntryTwo(cache,thisNode,pri);

	    if(existed)
		cache->dodelete = ajTrue;
	    GBT_NKEYS(buf,&nkeys);

	    if(nkeys >= minkeys || (nodetype==BT_SECROOT && !cache->slevel))
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
**
** @release 6.1.0
** @@
******************************************************************************/

static void btreeFindPriMinTwo(AjPBtcache cache, ajulong pagepos,
                               const AjPStr key)
{
    AjPBtpage page   = NULL;
    AjPSecbucket bucket = NULL;
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

    page = btreeSeccacheRead(cache,pagepos);
    buf  = page->buf;
    GBT_NODETYPE(buf,&nodetype);
    GBT_NKEYS(buf,&nkeys);

    btreeGetKeys(cache,buf,&karray,&parray);

    if(nodetype == BT_SECLEAF)
    {
	bucket = btreeReadSecbucket(cache,parray[0]);
	nentries = bucket->Nentries;

        /*
        ** If there's only one entry then it must be the key marked
        ** for deletion
        */
        if(nentries<2)
	{
	    btreeSecbucketDel(&bucket);
	    bucket = btreeReadSecbucket(cache,parray[1]);
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

	btreeSecbucketDel(&bucket);
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
**
** @release 6.1.0
** @@
******************************************************************************/

static AjBool btreeRemovePriEntryTwo(AjPBtcache cache, ajulong pagepos,
                                     const AjPBtPri pri)
{
    AjPBtpage page   = NULL;
    AjPSecbucket bucket = NULL;
    
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

    page = btreeSeccacheRead(cache,pagepos);
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
    
    bucket = btreeReadSecbucket(cache,blockno);


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

	btreeWriteSecbucket(cache,bucket,blockno);
	btreeAdjustPribucketsTwo(cache,page);
	page->dirty = BT_DIRTY;
    }
    else
	page->dirty = dirtysave;

    btreeSecbucketDel(&bucket);

    btreeDeallocSecArray(cache,arrays);
    
    if(!found)
	return ajFalse;

    return ajTrue;
}




/* @funcstatic btreeAdjustPribucketsTwo ***************************************
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
**
** @release 6.1.0
** @@
******************************************************************************/

static void btreeAdjustPribucketsTwo(AjPBtcache cache, AjPBtpage leaf)
{
    ajuint nkeys = 0;
    unsigned char *lbuf = NULL;
    AjPSecbucket *buckets  = NULL;

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
    AjPSecbucket cbucket = NULL;
    AjPStr cid       = NULL;

    ajuint v = 0;
    
    /* ajDebug("In btreeAdjustPribucketsTwo\n"); */

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
	totalkeys += btreeNumInSecbucket(cache,ptrs[i]);

    totalkeys += btreeNumInSecbucket(cache,ptrs[i]);


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
	buckets[i] = btreeReadSecbucket(cache,ptrs[i]);


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
    ajListSort(idlist, &ajStrVcmp);
    AJFREE(buckets);

    cbucket = btreeSecbucketNew(maxnperbucket,cache->idlimit);
    bucketlimit = bucketn - 1;

    totnids = 0;
    nids = (ajuint) ajListGetLength(idlist);


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

	btreeWriteSecbucket(cache,cbucket,ptrs[1]);

	cbucket->Overflow = overflows[0];
	cbucket->Nentries = 0;

	if(!ptrs[0])
	    ptrs[0] = cache->totsize;

	btreeWriteSecbucket(cache,cbucket,ptrs[0]);
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

	    btreeWriteSecbucket(cache,cbucket,ptrs[i]);
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
	
	btreeWriteSecbucket(cache,cbucket,ptrs[i]);
    }
    

    cbucket->Nentries = maxnperbucket;
    btreeSecbucketDel(&cbucket);

    /* Now write out a modified leaf with new keys/ptrs */

    nkeys = bucketn - 1;
    btreeWriteNode(cache,leaf,keys,ptrs,nkeys);

    leaf->dirty = dirtysave;
    if(nodetype == BT_SECROOT)
    {
	leaf->dirty = BT_LOCK;
        leaf->lockfor = 1882;
    }

    btreeDeallocSecArray(cache,arrays);
    btreeDeallocSecArray(cache,newarrays);

    btreeSecbucketDel(&cbucket);
    ajListFree(&idlist);

    return;
}




/* @funcstatic btreeRebalancePriTwo *******************************************
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
**
** @release 6.1.0
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
	lpage = btreeSeccacheRead(cache,leftNode);
	lbuf  = lpage->buf;
	GBT_NKEYS(lbuf,&lnkeys);
    }
    

    if(rightok)
    {
	rpage = btreeSeccacheRead(cache,rightNode);
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
	tpage = btreeSeccacheRead(cache,thisNode);
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
**
** @release 6.1.0
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


    pageA = btreeSeccacheRead(cache,anchorNode);
    pageA->dirty = BT_LOCK;
    pageA->lockfor = 1891;
    abuf = pageA->buf;
    pageB = btreeSeccacheRead(cache,balanceNode);
    pageB->dirty = BT_LOCK;
    pageB->lockfor = 1892;
    bbuf = pageB->buf;
    pageT = btreeSeccacheRead(cache,thisNode);
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
	page = btreeSeccacheRead(cache,pTarray[i]);
	buf = page->buf;
	GBT_NODETYPE(buf,&nodetype);

	if(nodetype != BT_IDBUCKET)
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
**
** @release 6.1.0
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

    pageA = btreeSeccacheRead(cache,anchorNode);
    saveA = pageA->dirty;
    pageA->dirty = BT_LOCK;
    pageA->lockfor = 1901;
    abuf = pageA->buf;
    pageN = btreeSeccacheRead(cache,mergeNode);
    saveN = pageN->dirty;
    pageN->dirty = BT_LOCK;
    pageN->lockfor = 1902;
    nbuf = pageN->buf;
    pageT = btreeSeccacheRead(cache,thisNode);
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

    for(i=0;i<=nNkeys;++i)
    {
	page = btreeSeccacheRead(cache,pNarray[i]);
	buf = page->buf;
	GBT_NODETYPE(buf,&nodetype);

	if(nodetype != BT_IDBUCKET)
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
**
** @release 6.1.0
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

    rootpage = btreeSeccacheLocate(cache,cache->secrootblock);
    buf = rootpage->buf;
    page = btreeSeccacheRead(cache,pagepos);


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
    nodetype = BT_SECINTERNAL;
    SBT_NODETYPE(buf,nodetype);

    page->pagepos = cache->secrootblock;
    page->dirty = BT_LOCK;
    page->lockfor = 1911;
    buf = page->buf;
    nodetype = BT_SECROOT;
    SBT_NODETYPE(buf,nodetype);
    
    --cache->slevel;

    if(cache->slevel)
    {
	/*
	 ** Update the PREV pointers of the new root's children
	 */
	GBT_NKEYS(buf,&nkeys);
	btreeGetKeys(cache,buf,&karray,&parray);

	for(i=0;i<=nkeys;++i)
	{
	    page = btreeSeccacheRead(cache,parray[i]);
	    lbuf = page->buf;
	    SBT_PREV(lbuf,prev);
	    page->dirty = BT_DIRTY;
	}
    }

    btreeDeallocSecArray(cache,arrays);
    
    return 0L;
}




/* @funcstatic btreeFindPriBalanceOne *****************************************
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
**
** @release 6.1.0
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
	page = btreePricacheRead(cache,thisNode);
    else
    {   /* It's the root node of the primary hyb tree */
        /* Needs altering for secondary tree          */
	page = btreePricacheLocate(cache,thisNode);
	page->dirty = BT_LOCK;
        page->lockfor = 1921;
    }

    cache->dodelete = ajFalse;

    buf = page->buf;
    GBT_NKEYS(buf,&nkeys);
    
    order = cache->porder;
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

    if(!(nodetype == BT_LEAF) && !(nodetype == BT_ROOT && !cache->plevel))
    {
	if(nextNode == parray[0])
	{
	    if(leftNode != BTNO_NODE)
	    {
		page1 = btreePricacheRead(cache,leftNode);
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
		page1 = btreePricacheRead(cache,rightNode);
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
	if(nodetype != BT_LEAF && cache->plevel)
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
	    page = btreePricacheRead(cache,thisNode);
	else
	{
	    page = btreePricacheLocate(cache,thisNode);
	    page->dirty = BT_LOCK;
            page->lockfor = 1922;
	}
	buf = page->buf;

    }
    else
    {
	if(nodetype == BT_LEAF || (nodetype==BT_ROOT && !cache->plevel))
	{
	    existed = btreeRemovePriEntryOne(cache,thisNode,pri);

	    if(existed)
		cache->dodelete = ajTrue;
	    GBT_NKEYS(buf,&nkeys);

	    if(nkeys >= minkeys || (nodetype==BT_ROOT && !cache->plevel))
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
**
** @release 6.1.0
** @@
******************************************************************************/

static void btreeFindPriMinOne(AjPBtcache cache, ajulong pagepos,
                               const AjPStr key)
{
    AjPBtpage page   = NULL;
    AjPPribucket bucket = NULL;
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

    page = btreePricacheRead(cache,pagepos);
    buf  = page->buf;
    GBT_NODETYPE(buf,&nodetype);
    GBT_NKEYS(buf,&nkeys);

    btreeGetKeys(cache,buf,&karray,&parray);

    if(nodetype == BT_LEAF)
    {
	bucket = btreePribucketRead(cache,parray[0]);
	nentries = bucket->Nentries;

        /*
        ** If there's only one entry then it must be the key marked
        ** for deletion
        */
        if(nentries<2)
	{
	    btreePribucketDel(&bucket);
	    bucket = btreePribucketRead(cache,parray[1]);
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
	btreePribucketDel(&bucket);
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
**
** @release 6.1.0
** @@
******************************************************************************/

static AjBool btreeRemovePriEntryOne(AjPBtcache cache, ajulong pagepos,
                                     const AjPBtPri pri)
{
    AjPBtpage page   = NULL;
    AjPPribucket bucket = NULL;
    
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

    page = btreePricacheRead(cache,pagepos);
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
    
    bucket = btreePribucketRead(cache,blockno);


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

	btreeWritePribucket(cache,bucket,blockno);
	btreeAdjustPribucketsOne(cache,page);
	page->dirty = BT_DIRTY;
    }
    else
	page->dirty = dirtysave;

    btreePribucketDel(&bucket);

    btreeDeallocPriArray(cache,arrays);
    
    if(!found)
	return ajFalse;

    return ajTrue;
}




/* @funcstatic btreeAdjustPribucketsOne ***************************************
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
**
** @release 6.1.0
** @@
******************************************************************************/

static void btreeAdjustPribucketsOne(AjPBtcache cache, AjPBtpage leaf)
{
    ajuint nkeys = 0;
    unsigned char *lbuf = NULL;
    AjPPribucket *buckets  = NULL;

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
    AjPPribucket cbucket = NULL;
    AjPBtPri cid       = NULL;

    ajuint v = 0;
    
    /* ajDebug("In btreeAdjustPribucketsOne\n"); */

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

    order = cache->porder;
    nperbucket = cache->pnperbucket;
    

    /* Read keys/ptrs */

    arrays = btreeAllocPriArray(cache);
    keys = arrays->karray;
    ptrs = arrays->parray;
    overflows = arrays->overflows;

    btreeGetPointers(cache,lbuf,&ptrs);


    for(i=0;i<nkeys;++i)
	totalkeys += btreeNumInPribucket(cache,ptrs[i]);

    totalkeys += btreeNumInPribucket(cache,ptrs[i]);


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
	buckets[i] = btreePribucketRead(cache,ptrs[i]);


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
    ajListSort(idlist, &btreeKeywordCompare);
    AJFREE(buckets);

    cbucket = btreePribucketNew(maxnperbucket);
    bucketlimit = bucketn - 1;

    totnids = 0;
    nids = (ajuint) ajListGetLength(idlist);


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
	btreeWritePribucket(cache,cbucket,ptrs[1]);

	cbucket->Overflow = overflows[0];
	cbucket->Nentries = 0;
	if(!ptrs[0])
	    ptrs[0] = cache->totsize;
	btreeWritePribucket(cache,cbucket,ptrs[0]);
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

	    btreeWritePribucket(cache,cbucket,ptrs[i]);
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
	
	btreeWritePribucket(cache,cbucket,ptrs[i]);
    }
    

    cbucket->Nentries = maxnperbucket;
    btreePribucketDel(&cbucket);

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

    btreePribucketDel(&cbucket);
    ajListFree(&idlist);

    return;
}




/* @funcstatic btreeRebalancePriOne *******************************************
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
**
** @release 6.1.0
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
	lpage = btreePricacheRead(cache,leftNode);
	lbuf  = lpage->buf;
	GBT_NKEYS(lbuf,&lnkeys);
    }
    

    if(rightok)
    {
	rpage = btreePricacheRead(cache,rightNode);
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

    
    order = cache->porder;
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
	tpage = btreePricacheRead(cache,thisNode);
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
**
** @release 6.1.0
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


    pageA = btreePricacheRead(cache,anchorNode);
    pageA->dirty = BT_LOCK;
    pageA->lockfor = 1951;
    abuf = pageA->buf;
    pageB = btreePricacheRead(cache,balanceNode);
    pageB->dirty = BT_LOCK;
    pageB->lockfor = 1952;
    bbuf = pageB->buf;
    pageT = btreePricacheRead(cache,thisNode);
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
	page = btreePricacheRead(cache,pTarray[i]);
	buf = page->buf;
	GBT_NODETYPE(buf,&nodetype);
	if(nodetype != BT_IDBUCKET)
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
**
** @release 6.1.0
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

    pageA = btreePricacheRead(cache,anchorNode);
    saveA = pageA->dirty;
    pageA->dirty = BT_LOCK;
    pageA->lockfor = 1961;
    abuf = pageA->buf;
    pageN = btreePricacheRead(cache,mergeNode);
    saveN = pageN->dirty;
    pageN->dirty = BT_LOCK;
    pageN->lockfor = 1962;
    nbuf = pageN->buf;
    pageT = btreePricacheRead(cache,thisNode);
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

    for(i=0;i<=nNkeys;++i)
    {
	page = btreePricacheRead(cache,pNarray[i]);
	buf = page->buf;
	GBT_NODETYPE(buf,&nodetype);

	if(nodetype != BT_IDBUCKET)
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
**
** @release 6.1.0
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
    
    if(!cache->plevel)
	return BTNO_NODE;

    rootpage = btreePricacheLocate(cache,0L);
    buf = rootpage->buf;
    page = btreePricacheRead(cache,pagepos);


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
    
    --cache->plevel;

    if(cache->plevel)
    {
	/*
	 ** Update the PREV pointers of the new root's children
	 */
	GBT_NKEYS(buf,&nkeys);
	btreeGetKeys(cache,buf,&karray,&parray);

	for(i=0;i<=nkeys;++i)
	{
	    page = btreePricacheRead(cache,parray[i]);
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
**
** @release 6.1.0
** @@
******************************************************************************/

static AjBool btreeIsSecEmpty(AjPBtcache cache)
{
    AjPBtpage rootpage  = NULL;
    AjPSecbucket bucket = NULL;
    unsigned char *buf  = NULL;
    AjPStr *karray  = NULL;
    ajulong *parray  = NULL;
    AjPBtMem arrays = NULL;
    ajuint nkeys = 0;
    ajuint tkeys = 0;

    if(cache->slevel)
        return ajFalse;

    rootpage = btreeSeccacheLocate(cache,cache->secrootblock);

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
        bucket = btreeReadSecbucket(cache,parray[0]);
        tkeys += bucket->Nentries;
    }

    if(parray[1])
    {
        bucket = btreeReadSecbucket(cache,parray[1]);
        tkeys += bucket->Nentries;
    }

    btreeDeallocSecArray(cache,arrays);

    if(tkeys)
        return ajFalse;

    return ajTrue;
}




/* @funcstatic btreeNodetype **************************************************
**
** Returns a name for the nodetype of a buffer
**
** @param [r] buf [const unsigned char*] buffer
** @return [const char*] Name of nodetype
**
** @release 6.4.0
** @@
******************************************************************************/

static const char* btreeNodetype(const unsigned char* buf)
{
    static AjBool called = AJFALSE;
    static char nodetypebuf[512];
    ajuint nodetype;

    if(!called)
    {
        AJCNEW0(btreeNodetypeNames, BT_MAXTYPE+1);
        btreeNodetypeNames[BT_SECFREEPAGE] = "secfree";
        btreeNodetypeNames[BT_FREEPAGE]    = "free";
        btreeNodetypeNames[BT_ROOT]        = "root";
        btreeNodetypeNames[BT_INTERNAL]    = "internal";
        btreeNodetypeNames[BT_LEAF]        = "leaf";
        btreeNodetypeNames[BT_IDBUCKET]    = "bucket";
        btreeNodetypeNames[BT_OVERFLOW]    = "overflow";
        btreeNodetypeNames[BT_PRIBUCKET]   = "pribucket";
        btreeNodetypeNames[BT_SECBUCKET]   = "secbucket";
        btreeNodetypeNames[BT_NUMBUCKET]   = "numbucket";
        btreeNodetypeNames[BT_SECROOT]     = "secroot";
        btreeNodetypeNames[BT_SECINTERNAL] = "secinternal";
        btreeNodetypeNames[BT_SECLEAF]     = "secleaf";
        called = ajTrue;
    }

    GBT_NODETYPE(buf, &nodetype);

    if(nodetype >= BT_MAXTYPE)
    {
        sprintf(nodetypebuf, "unknown (%u)", nodetype);
        return nodetypebuf;
    }

    if(!btreeNodetypeNames[nodetype])
    {
        sprintf(nodetypebuf, "unknown (%u)", nodetype);
        return nodetypebuf;
    }

    if(nodetype == BT_OVERFLOW || nodetype == BT_SECOVERFLOW)
    {
        ajWarn("Overflow page found");
    }
    

    return btreeNodetypeNames[nodetype];
}




/* @func ajBtreePageIsPrimary *************************************************
**
** Returns a true is a page is primary
**
** @param [r] page [const AjPBtpage] Page
** @return [AjBool] True if page is primary index or bucket
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ajBtreePageIsPrimary(const AjPBtpage page)
{
    unsigned char* buf = NULL;
    ajuint nodetype;

    buf = page->buf;

    GBT_NODETYPE(buf, &nodetype);

    switch (nodetype) 
    {
        case BT_FREEPAGE:       /* no need to save the page */
        case BT_ROOT:
        case BT_INTERNAL:
        case BT_LEAF:
        case BT_IDBUCKET:
        case BT_PRIBUCKET:
        case BT_OVERFLOW:
            return ajTrue;
            break;
        case BT_SECFREEPAGE:
        case BT_SECROOT:
        case BT_SECINTERNAL:
        case BT_SECLEAF:
        case BT_SECOVERFLOW:
        case BT_SECBUCKET:
        case BT_NUMBUCKET:
            return ajFalse;
            break;
        default:
            return ajTrue;
    }

    return ajTrue;
}





/* @func ajBtreeCacheGetPagecount *********************************************
**
** Returns the total number of pages in a tree index
**
** @param [r] cache [const AjPBtcache] cache
**
** @return [ajulong] number of pages in a tree index
**
** @release 6.4.0
** @@
******************************************************************************/

ajulong ajBtreeCacheGetPagecount(const AjPBtcache cache)
{
    return cache->pripagecount + cache->secpagecount;
}




/* @func ajBtreeCacheGetPripagesize *******************************************
**
** Returns the primary page size of a tree index
**
** @param [r] cache [const AjPBtcache] cache
**
** @return [ajuint] Page size of a tree index
**
** @release 6.5.0
** @@
******************************************************************************/

ajuint ajBtreeCacheGetPripagesize(const AjPBtcache cache)
{
    return cache->pripagesize;
}




/* @func ajBtreeCacheGetSecpagesize *******************************************
**
** Returns the secondary page size of a tree index
**
** @param [r] cache [const AjPBtcache] cache
**
** @return [ajuint] Page size of a tree index
**
** @release 6.5.0
** @@
******************************************************************************/

ajuint ajBtreeCacheGetSecpagesize(const AjPBtcache cache)
{
    return cache->secpagesize;
}




/* @func ajBtreeCacheGetTotsize ***********************************************
**
** Returns the number of bytes in an uncompressed tree index
**
** @param [r] cache [const AjPBtcache] cache
**
** @return [ajulong] number of bytes in a tree index
**
** @release 6.4.0
** @@
******************************************************************************/

ajulong ajBtreeCacheGetTotsize(const AjPBtcache cache)
{
    return (cache->pripagesize * cache->pripagecount) +
        (cache->secpagesize * cache->secpagecount);
}




/* @func ajBtreePageGetTypename ***********************************************
**
** Returns the node type name of a tree index page
**
** @param [r] page [const AjPBtpage] Page
**
** @return [const char*] Name of page type
**
** @release 6.4.0
** @@
******************************************************************************/

const char* ajBtreePageGetTypename(const AjPBtpage page)
{
    return btreeNodetype(page->buf);
}




/* @func ajBtreeStatNode ******************************************************
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
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajBtreeStatNode(AjPBtcache cache, const AjPBtpage page, AjBool full,
                       ajuint* nkeys, ajuint* overflows,
                       ajuint* freespace, ajulong* refs,
                       const AjPTable newpostable)
{
    ajuint i;
    ajuint m;

    ajuint klen;
    const AjPBtpage bpage;
    unsigned char *tbuf;

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

/*    ajuint freebytes;*/
/*    ajuint nodetype;*/

    if(ajBtreePageIsPrimary(page))
        pagesize = cache->pripagesize;
    else
        pagesize = cache->secpagesize;

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

/*
//        if((ajuint)((keyptr-tbuf+2) + klen + sizeof(ajulong)) > pagesize)
//	{
//            freebytes = pagesize - (keyptr-tbuf);
//            *freespace += freebytes;
//            (*overflows)++;
//    	    /# ajDebug("btreeStatNode: Overflow\n"); #/
//	    bpage = ajBtreeCacheRead(cache,overflow);
//	    tbuf = bpage->buf;
//	    GBT_NODETYPE(tbuf,&nodetype);
//
//	    if(nodetype != BT_OVERFLOW)
//		ajFatal("StatNode Overflow node %Lu expected but not found "
//                        "cache %S page %Lu "
//                        "pagepos:%Lu key:%u/%u free:%u keyptr:%x lenptr:%x "
//                        "tbuf:%x klen:%u old nodetype '%s' newnodetype '%s' "
//                        "blockno:%Lu totlen:%u left:%Lu right:%Lu prev:%Lu",
//                        overflow, cache->filename,
//                        page->pagepos, page->pagepos/pagesize,
//                        i, m, freebytes,
//                        keyptr, lenptr, tbuf, klen,
//                        btreeNodetype(page->buf), btreeNodetype(tbuf),
//                        blockno,totlen, left, right, prev);
//
//            GBT_BLOCKNUMBER(tbuf,&blockno);
//            GBT_TOTLEN(tbuf,&totlen);
//            GBT_LEFT(tbuf,&left);
//            GBT_RIGHT(tbuf,&right);
//            GBT_PREV(tbuf,&prev);
//
//	    /#
//	     ** The length pointer is restricted to the initial page.
//	     ** The keyptr in overflow pages starts at the Key Lengths
//	     ** position!
//	     #/
//            keyptr = PBT_KEYLEN(tbuf);
//	}
*/

        kp = keyptr;
        keyptr += klen+1;
        BT_GETAJULONG(keyptr,&lv);
        ++refs[*ajTableulongFetch(newpostable, &lv)];
	keyptr += sizeof(ajulong);

        if(full)
            ajDebug("#StatNode     %12Lu %12Lu '%*s'\n",
                    page->pagepos, lv, klen, kp);

	lenptr += sizeof(ajuint);
    }

    BT_GETAJULONG(keyptr,&lv);
    keyptr += sizeof(ajulong);
    ++refs[*ajTableulongFetch(newpostable, &lv)];

    if(full)
        ajDebug("#StatNode     %12Lu %12Lu '<end>'\n",
                page->pagepos, lv);

    *freespace += pagesize - (keyptr-tbuf);

    if(full)
        ajDebug("StatNode      %12Lu keys:%6u over:%6u free:%6u\n",
                page->pagepos, *nkeys, *overflows, *freespace);

    return ajTrue;
}




/* @func ajBtreeStatNumnode ***************************************************
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
**
** @release 6.4.0
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

    if(ajBtreePageIsPrimary(page))
        pagesize = cache->pripagesize;
    else
        pagesize = cache->secpagesize;

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
        btreeStatNumnode(cache, page);

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




/* @funcstatic btreeStatNumnode ***********************************************
**
** Writes a numeric node page to the debug file
**
** @param [w] cache [AjPBtcache] cache
** @param [r] page [const AjPBtpage] Page
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void btreeStatNumnode(AjPBtcache cache, const AjPBtpage page)
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
                "StatNumNode   %12Lu totlen:%u\n"
                "         left:%12Lu  right:%12Lu  prev:%12Lu\n"
                "        block:%12Lu   over:%12Lu\n",
                page->pagepos, totlen, left, right, prev, blockno, overflow);
    ajDebug("\nStatNumnode   %12Lu   pageoffset (%u)\n",
                page->pagepos, m);
    ajDebug("-----------   ------------ ------------ -------\n");


    keyptr =  PBT_KEYLEN(tbuf);
    valptr = keyptr + m * sizeof(ajulong);

    for(i=0;i<m;i++)
    {
        BT_GETAJULONG(valptr,&lv);
        BT_GETAJULONG(keyptr,&lk);
        ajDebug("#StatNumnode  %12Lu %12Lu %Lu\n", page->pagepos, lv, lk);
	keyptr += sizeof(ajulong);
	valptr += sizeof(ajulong);
    }

    BT_GETAJULONG(valptr,&lv);
    ajDebug("#StatNumnode  %12Lu %12Lu '<end>'\n", page->pagepos, lv);
    ajDebug("StatNumnode   %12Lu keys:%6u\n",
                page->pagepos, m);
    return;
}




/* @func ajBtreeStatIdbucket **************************************************
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
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajBtreeStatIdbucket(AjPBtcache cache, const AjPBtpage page, AjBool full,
                           ajuint* nentries, ajuint *ndups, ajuint *nextra,
                           ajuint* overflows, ajuint* freespace, ajulong* refs,
                           const AjPTable newpostable)
{
    ajuint i;
    ajuint iref;
    ajuint m;
    ajuint klen;
    const AjPBtpage bpage;
    unsigned char *tbuf;

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
    ajuint keyskip = BT_DDOFF + cache->refcount*BT_EXTRA;

/*    ajuint freebytes;*/
/*    ajuint nodetype;*/

    if(ajBtreePageIsPrimary(page))
        pagesize = cache->pripagesize;
    else
        pagesize = cache->secpagesize;

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
        ajDebug("\nStatBucket     %12Lu   dbno   dups       offset",
                page->pagepos);

    if(full && cache->refcount)
    {
        for(iref=0; iref < cache->refcount; iref++)
            ajDebug("    refoffset%d", iref+1);
    }

    if(full)
        ajDebug(" (%u)\n", m);

    if(full)
        ajDebug("----------                    ----   ----");

    if(full && cache->refcount)
    {
        for(iref=0; iref < cache->refcount; iref++)
            ajDebug("       ------");
    }

    if(full)
        ajDebug("    ---------\n");

    for(i=0;i<m;i++)
    {
	BT_GETAJUINT(keyptr,&klen);
        idlen = klen - keyskip;

/*
//	if((idptr-tbuf+2) + klen > cache->pagesize)	/# overflow #/
//	{
//            freebytes = pagesize - (idptr-tbuf);
//            *freespace += freebytes;
//            (*overflows)++;
//    	    /# ajDebug("btreeStatNode: Overflow\n"); #/
//	    GBT_BUCKOVERFLOW(tbuf,&overflow);
//	    bpage = ajBtreeCacheRead(cache,overflow);
//	    tbuf = bpage->buf;
//	    GBT_BUCKNODETYPE(tbuf,&nodetype);
//	    if(nodetype != BT_OVERFLOW)
//		ajFatal("StatBucket Overflow node %Lu expected but not found "
//                        "for cache '%S'",
//                        overflow, cache->filename);
//	    /# overflow bucket ids start at the keylen position #/
//	    idptr = PBT_BUCKKEYLEN(tbuf);
//	}
*/

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

        if(full)
            ajDebug("#StatBucket    %12Lu %6u %6u %12Lu ",
                    page->pagepos, uv, uv2, lv);

        if(cache->refcount)
        {
            for(iref=0; iref < cache->refcount; iref++)
            {
                BT_GETAJULONG(idptr,&lv2);
                idptr += sizeof(ajulong);
                if(full)
                    ajDebug("%12Lu ", lv2);
            }
        }

        if(full)
            ajDebug("'%*s'\n", idlen-1, kp);

	keyptr += sizeof(ajuint);
    }
    *freespace += pagesize - (idptr-tbuf);
    *nextra -= *ndups;
    if(full)
        ajDebug("StatBucket     %12Lu keys:%6u over:%6u free:%6u\n",
                page->pagepos, *nentries, *overflows, *freespace);

    return ajTrue;
}




/* @func ajBtreeStatNumbucket *************************************************
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
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajBtreeStatNumbucket(AjPBtcache cache, const AjPBtpage page, AjBool full,
                            ajuint* nentries, ajuint* overflows,
                            ajuint* freespace)
{
    ajuint iref;
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

    pagesize = cache->secpagesize;

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
                "StatNumbucket  %12Lu over:%12Lu\n",
                page->pagepos, overflow);
    if(full)
        ajDebug("\nStatNumbucket  %12Lu       offset    ",
                page->pagepos);

    if(full && cache->refcount)
    {
        for(iref=0; iref < cache->refcount; iref++)
            ajDebug("refoffset%d", iref+1);
    }

    if (full)
        ajDebug("dbno (%u)\n",
                page->pagepos, m);

    if(full)
        ajDebug("-------------  ------------      -------");
    
    if(full && cache->refcount)
    {
        for(iref=0; iref < cache->refcount; iref++)
            ajDebug("   ----------");
    }
    
    if(full)
        ajDebug("         ----\n");

    for(i=0;i<m;i++)
    {
        BT_GETAJULONG(keyptr,&lv);
	keyptr += sizeof(ajulong);

        if(full)
            ajDebug("#StatNumbucket              %12Lu", lv);

        if(cache->refcount)
        {
            for(iref=0; iref < cache->refcount; iref++)
            {
                BT_GETAJULONG(keyptr,&lv2);
                keyptr += sizeof(ajulong);
                if(full)
                    ajDebug(" %12Lu", lv2);
            }

        }

        BT_GETAJUINT(keyptr,&v);
	keyptr += sizeof(ajuint);
        if(full)
            ajDebug(" %12u\n", v);
    }
    *freespace += pagesize - (keyptr-tbuf);
    if(full)
        ajDebug("StatNumbucket  %12Lu keys:%6u over:%6u free:%6u\n",
                page->pagepos, *nentries, *overflows, *freespace);
    
    return ajTrue;
}




/* @func ajBtreeStatPribucket *************************************************
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
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajBtreeStatPribucket(AjPBtcache cache, const AjPBtpage page, AjBool full,
                            ajuint* nentries, ajuint* overflows,
                            ajuint* freespace, ajulong *refs,
                            const AjPTable newpostable)
{
    ajuint i;
    ajuint m;
    ajuint klen;
    const AjPBtpage bpage;
    unsigned char *tbuf;

    ajuint   pagesize = 0;
    ajulong   overflow = 0L;
    unsigned char *idptr = NULL;
    unsigned char *keyptr = NULL;

    unsigned char* lp;
    ajulong lv;

/*    ajuint nodetype;*/
/*    ajuint freebytes;*/

    pagesize = cache->pripagesize;

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

/*
//	if((idptr-tbuf+2) + klen > pagesize)	/# overflow #/
//	{
//            freebytes = pagesize - (idptr-tbuf);
//            *freespace += freebytes;
//            (*overflows)++;
//    	    /# ajDebug("btreeStatNode: Overflow\n"); #/
//	    GBT_BUCKOVERFLOW(tbuf,&overflow);
//	    bpage = btreePricacheRead(cache,overflow);
//	    tbuf = bpage->buf;
//	    GBT_BUCKNODETYPE(tbuf,&nodetype);
//	    if(nodetype != BT_OVERFLOW)
//		ajFatal("StatPribucket Overflow node %Lu expected but not found",
//                        overflow, cache->filename);
//	    /# overflow bucket ids start at the keylen position #/
//	    idptr = PBT_BUCKKEYLEN(tbuf);
//	}
*/

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




/* @func ajBtreeStatSecbucket *************************************************
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
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajBtreeStatSecbucket(AjPBtcache cache, const AjPBtpage page, AjBool full,
                            ajuint* nentries, ajuint* overflows,
                            ajuint* freespace)
{
    ajuint i;
    ajuint m;
    ajuint klen;
    const AjPBtpage bpage;
    unsigned char *tbuf;

    ajuint   pagesize = 0;
    ajulong   overflow = 0L;
    unsigned char *idptr = NULL;
    unsigned char *keyptr = NULL;

/*    ajuint nodetype;*/
/*    ajuint freebytes;*/

    pagesize = cache->secpagesize;

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

/*
//	if((idptr-tbuf+2) + klen > pagesize)	/# overflow #/
//	{
//            freebytes = pagesize - (idptr-tbuf);
//            *freespace += freebytes;
//            (*overflows)++;
//    	    /# ajDebug("btreeStatNode: Overflow\n"); #/
//	    GBT_BUCKOVERFLOW(tbuf,&overflow);
//	    bpage = btreeSeccacheRead(cache,overflow);
//	    tbuf = bpage->buf;
//            GBT_BUCKNODETYPE(tbuf,&nodetype);
//	    if(nodetype != BT_OVERFLOW)
//		ajFatal("StatSecbucket Overflow node %Lu expected "
//                        "but not found",
//                        overflow, cache->filename);
//	    /# overflow bucket ids start at the keylen position #/
//	    idptr = PBT_BUCKKEYLEN(tbuf);
//	}
*/

        if(full)
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




#if 0
/* #funcstatic btreeIdbucketSplitCalc *****************************************
**
** Calculate new bucket sizes to split a node.
** Make sure all existing buckets are reused.
**
** #param [r] totalkeys [ajuint] Total number of keys in all buckets
** #param [r] totalbuckets [ajuint] Total number of current buckets
** #param [r] maxbucketsize [ajuint] Maximum number of keys in a buckets
** #param [w] leftbuckets [ajuint*] Number of left node buckets
** #param [w] leftmax [ajuint*] Maximum number of keys per left node bucket
** #param [w] leftkeys [ajuint*] Maximum number of keys under left node
** #param [w] rightbuckets [ajuint*] Number of right node buckets
** #param [w] rightmax [ajuint*] Maximum number of keys per right node bucket
** #param [w] rightkeys [ajuint*] Maximum number of keys under right node
** #return [AjBool] True if calculated value reused all buckets
**                  False if increased to use all buckets
**
** #release 6.4.0
** ##
******************************************************************************/
/*
static AjBool btreeIdbucketSplitCalc(ajuint totalkeys,
                                     ajuint totalbuckets,
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

/# we always split the keys to left and right #/

    lkeys = totalkeys/2;
    rkeys = totalkeys - lkeys;

/# values to reuse all the current buckets #/

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

/# minimum needed to fit what we need with half-full buckets #/

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

/# we always use the same keys split #/

    *leftkeys = lkeys;
    *rightkeys = rkeys;

    if((rbuckets2 + lbuckets2) < totalbuckets)
    {
        /# use all the current buckets #/

        *leftmax = lmax;
        *leftbuckets = lbuckets1;
        *rightmax = rmax;
        *rightbuckets = rbuckets1;
        ret = ajFalse;
    }
    else
    {
        /# split to same or more half-full buckets #/

        *leftmax = lmax2;
        *leftbuckets = lbuckets2;
        *rightmax = rmax2;
        *rightbuckets = rbuckets2;
    }

    lrest = (*leftkeys) - (*leftbuckets)*(*leftmax);
    rrest = (*rightkeys) - (*rightbuckets)*(*rightmax);

#if AJINDEX_DEBUG
    ajDebug("btreeIdbucketSplitCalc totalkeys:%u  totalbuckets:%u "
            "maxbucketsize:%u => %u*%u + %u (%u) "
            "%u*%u + %u (%u)\n",
            totalkeys, totalbuckets, maxbucketsize,
            *leftbuckets, *leftmax, lrest, *leftkeys,
            *rightbuckets, *rightmax, rrest, *rightkeys);
#endif

    if(lrest < 1 || lrest > (*leftmax))
        ajFatal("Bad btreeIdbucketSplitCalc lrest:%u", lrest);
    if(rrest < 1 || rrest > (*rightmax))
        ajFatal("Bad btreeIdbucketSplitCalc rrest:%u", rrest);

    return ret;
}
*/
#endif




/* @funcstatic btreeBucketSplitCalc *******************************************
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
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool btreeBucketSplitCalc(ajuint totalkeys,
                                   ajuint totalbuckets,
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

/* we always split the keys to left and right */

    lkeys = totalkeys/2;
    rkeys = totalkeys - lkeys;

/* values to reuse all the current buckets */

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

/* minimum needed to fit what we need with half-full buckets */

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

/* we always use the same keys split */

    *leftkeys = lkeys;
    *rightkeys = rkeys;

    if((rbuckets2 + lbuckets2) < totalbuckets)
    {
        /* use all the current buckets */

        *leftmax = lmax;
        *leftbuckets = lbuckets1;
        *rightmax = rmax;
        *rightbuckets = rbuckets1;
        ret = ajFalse;
    }
    else
    {
        /* split to same or more half-full buckets */

        *leftmax = lmax2;
        *leftbuckets = lbuckets2;
        *rightmax = rmax2;
        *rightbuckets = rbuckets2;
    }

    lrest = (*leftkeys) - (*leftbuckets)*(*leftmax);
    rrest = (*rightkeys) - (*rightbuckets)*(*rightmax);

#if AJINDEX_DEBUG
    ajDebug("btreeBucketSplitCalc totalkeys:%u  totalbuckets:%u "
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




#if 0
/* #funcstatic btreeIdbucketCalc **********************************************
**
** Calculate new bucket sizes to split a node.
** Make sure all existing buckets are reused.
**
** #param [r] totalkeys [ajuint] Total number of keys in all buckets
** #param [r] totalbuckets [ajuint] Total number of current buckets
** #param [r] maxbucketsize [ajuint] Maximum number of keys in a buckets
** #param [w] newbuckets [ajuint*] Number of new buckets
** #param [w] newmax [ajuint*] Maximum number of keys per new bucket
** #return [AjBool] True if calculated value reused all buckets
**                  False if increased to use all buckets
**
** #release 6.4.0
** ##
******************************************************************************/

/*
static AjBool btreeIdbucketCalc(ajuint totalkeys, ajuint totalbuckets,
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

    /# from the original keys and buckets #/

    if(!totalbuckets)
        ajFatal("btreeIdbucketCalc zero buckets keys:%u", totalkeys);
    bmax = totalkeys/totalbuckets;
    if(!(totalkeys % totalbuckets))
        bmax--;
    if(!bmax) bmax = 1;

    bucketn = totalkeys/bmax;
    if(totalkeys % bmax)
        bucketn++;
    buckets1 = bucketn - 1;

    /# minimum space required #/

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
*/
#endif




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
**
** @release 6.4.0
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
** Return the new start position of a compressed page position
**
** @param [r] oldpos [ajulong] Page position
** @param [r] newpostable [const AjPTable] Table of new page positions
** @param [r] where [const char*] Location in index for reporting in
**                                warning message.
** @return [ajulong] New page start position
**
** @release 6.4.0
** @@
******************************************************************************/

static ajulong btreePageposCompress(ajulong oldpos,
                                    const AjPTable newpostable,
                                    const char* where)
{
    const ajulong *newpos;

    ajDebug("btreePageposCompress oldpos %Lu table size:%Lu\n",
            oldpos, ajTableGetLength(newpostable));

    newpos =  ajTableulongFetch(newpostable, &oldpos);

    if(!newpos)
        ajWarn("oldpos %Lu not found (%s)",
               oldpos, where);
    else
        ajDebug("btreePageposCompress oldpos %Lu newpos %Lu\n",
                oldpos, *newpos);

    return *newpos;
}




/* @funcstatic btreePageposUncompress *****************************************
**
** Return the new start position of a compressed page
**
** @param [r] oldpos [ajulong] Page position
** @param [r] newpostable [const AjPTable] Array of new page positions
** @param [r] where [const char*] Location in index for reporting in
**                                warning message.
** @return [ajulong] new start position
**
** @release 6.4.0
** @@
******************************************************************************/

static ajulong btreePageposUncompress(ajulong oldpos,
                                      const AjPTable newpostable,
                                      const char* where)
{
    const ajulong *newpos;

    ajDebug("btreePageposUncompress oldpos %Lu table size:%Lu\n",
            oldpos, ajTableGetLength(newpostable));

    newpos =  ajTableulongFetch(newpostable, &oldpos);

    if(!newpos)
        ajWarn("oldpos %Lu not found (%s)",
               oldpos, where);
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
** @param [r] refcount [ajuint] Reference offset count
** @return [AjBool] True on success
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool btreePageUncompress(AjPBtpage page,
                                  const AjPTable newpostable,
                                  ajuint refcount)
{
    const unsigned char* buf = page->buf;
    ajuint nodetype;

    GBT_NODETYPE(buf, &nodetype);

    ajDebug("btreePageUncompress %Lu (%s)\n",
            page->pagepos, btreeNodetype(buf));

    switch (nodetype) 
    {
        case BT_SECFREEPAGE:    /* no need to save the page */
        case BT_FREEPAGE:       /* no need to save the page */
            return ajFalse;
        case BT_ROOT:
        case BT_INTERNAL:
        case BT_LEAF:
            btreePageUncompressNode(page, newpostable);
            return ajTrue;
        case BT_SECROOT:
        case BT_SECINTERNAL:
        case BT_SECLEAF:
            btreePageUncompressNode(page, newpostable);
            return ajTrue;
        case BT_IDBUCKET:
            btreePageUncompressIdbucket(page, newpostable,
                                        refcount);
            return ajTrue;
        case BT_OVERFLOW:
        case BT_SECOVERFLOW:
            ajErr("Overflow page type %u in btreePageUncompress", nodetype);
            return ajTrue;
        case BT_PRIBUCKET:
            btreePageUncompressPribucket(page, newpostable);
            return ajTrue;
        case BT_SECBUCKET:
            /* no index page offsets to adjust */
            return ajTrue;
        case BT_NUMBUCKET:
            /* no index page offsets to adjust */
            return ajTrue;
        default:
            ajErr("Unknown page type %u in btreePageUncompress", nodetype);
            break;
    }
    return ajTrue;
}




/* @funcstatic btreePageUncompressIdbucket ************************************
**
** Uncompress a bucket cache page using an array of new positions for
** uncompressed pages
**
** @param [u] page [AjPBtpage] Page
** @param [r] newpostable [const AjPTable] Table of new page positions
** @param [r] refcount [ajuint] Number of reference offsets
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void btreePageUncompressIdbucket(AjPBtpage page,
                                        const AjPTable newpostable,
                                        ajuint refcount)
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
    ajuint keyskip = BT_DDOFF + refcount*BT_EXTRA;

    buf = page->buf;

    GBT_BUCKNENTRIES(buf,&nentries);
    GBT_BUCKOVERFLOW(buf,&overflow);
/*
//    if(overflow)
//    {
//        overflow = btreePageposUncompress(overflow, newpostable,
//                                          "Bucket overflow");
//        SBT_BUCKOVERFLOW(buf, overflow);
//    }
*/
    keyptr = PBT_BUCKKEYLEN(buf);
    idptr = keyptr + (nentries* sizeof(ajuint));

    /*
    ** ajulong values in bucket are offset and refoffset in files for dups=1
    ** but for dups > 1 offset is the secondary root page for the duplicates
    */

    for(i=0;i<nentries;i++)
    {
	BT_GETAJUINT(keyptr,&klen);
        idlen = klen - keyskip;

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

        idptr += sizeof(ajulong) + refcount*sizeof(ajulong);
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
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void btreePageUncompressNode(AjPBtpage page,
                                    const AjPTable newpostable)
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
        btreePageUncompressNumnode(page, newpostable);
        return;
    }

    GBT_NKEYS(buf,&nkeys);
    GBT_OVERFLOW(buf,&overflow);

/*
//    if(overflow)
//    {
//        overflow = btreePageposUncompress(overflow, newpostable,
//                                          "Node overflow");
//        SBT_OVERFLOW(buf, overflow);
//    }
*/    

    GBT_LEFT(buf,&left);

    if(left)
    {
        left = btreePageposUncompress(left, newpostable,
                                      "Node left");
        SBT_LEFT(buf, left);
    }
    
    GBT_RIGHT(buf,&right);

    if(right > BT_NODEPREAMBLE)      /* not a level, must be a page position */
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




/* @funcstatic btreePageUncompressNumnode *************************************
**
** uncompress a numeric node cache page using a table of new positions
**
** @param [u] page [AjPBtpage] Page
** @param [r] newpostable [const AjPTable] Table of new page positions
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void btreePageUncompressNumnode(AjPBtpage page,
                                       const AjPTable newpostable)
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

/*
//    if(overflow)
//    {
//        overflow = btreePageposUncompress(overflow, newpostable,
//                                          "Numnode overflow");
//        SBT_OVERFLOW(buf, overflow);
//    }
*/

    GBT_LEFT(buf,&left);

    if(left)
    {
        left = btreePageposUncompress(left, newpostable,
                                      "Numnode left");
        SBT_LEFT(buf, left);
    }
    
    GBT_RIGHT(buf,&right);

    if(right > BT_NODEPREAMBLE)      /* not a level, must be a page position */
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
**
** @release 6.4.0
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

/*
//    if(overflow)
//    {
//        overflow = btreePageposUncompress(overflow, newpostable,
//                                          "Pribucket overflow");
//        SBT_BUCKOVERFLOW(buf, overflow);
//    }
*/

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
                                        "Pribucket keypos");
            BT_SETAJULONG(lp, lv);
        }

	keyptr += sizeof(ajuint);
    }

    return;
}




/* @funcstatic btreePageCompress **********************************************
**
** Compress a cache page using an array of new positions for uncompressed pages
**
** @param [u] page [AjPBtpage] Page
** @param [r] newpostable [const AjPTable] Table of new page positions
** @param [r] refcount [ajuint] Number of reference offsets
** @return [AjBool] True on success
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool btreePageCompress(AjPBtpage page,
                                const AjPTable newpostable,
                                ajuint refcount)
{
    const unsigned char* buf = page->buf;
    ajuint nodetype;

    GBT_NODETYPE(buf, &nodetype);

    ajDebug("btreePageCompress %Lu (%s)\n",
            page->pagepos, btreeNodetype(buf));

    switch (nodetype) 
    {
        case BT_SECFREEPAGE:    /* no need to save the page */
        case BT_FREEPAGE:       /* no need to save the page */
            return ajFalse;
        case BT_ROOT:
        case BT_INTERNAL:
        case BT_LEAF:
            btreePageCompressNode(page, newpostable);
            return ajTrue;
        case BT_SECROOT:
        case BT_SECINTERNAL:
        case BT_SECLEAF:
            btreePageCompressNode(page, newpostable);
            return ajTrue;
        case BT_IDBUCKET:
            btreePageCompressIdbucket(page, newpostable, refcount);
            return ajTrue;
        case BT_OVERFLOW:
        case BT_SECOVERFLOW:
            ajErr("Overflow page type %u in btreePageCompress", nodetype);
            return ajTrue;
        case BT_PRIBUCKET:
            btreePageCompressPribucket(page, newpostable);
            return ajTrue;
        case BT_SECBUCKET:
            /* no index page offsets to adjust */
            return ajTrue;
        case BT_NUMBUCKET:
            /* no index page offsets to adjust */
            return ajTrue;
        default:
            ajErr("Unknown page type %u in btreePageCompress", nodetype);
            break;
    }
    return ajTrue;
}




/* @funcstatic btreePageCompressIdbucket **************************************
**
** Compress a bucket cache page using an array of new positions for
** uncompressed pages
**
** @param [u] page [AjPBtpage] Page
** @param [r] newpostable [const AjPTable] Table of new page positions
** @param [r] refcount [ajuint] Number of reference offsets
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void btreePageCompressIdbucket(AjPBtpage page,
                                      const AjPTable newpostable,
                                      ajuint refcount)
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
    ajuint keyskip = BT_DDOFF + refcount*BT_EXTRA;

    buf = page->buf;

    GBT_BUCKNENTRIES(buf,&nentries);
    GBT_BUCKOVERFLOW(buf,&overflow);

/*
//    if(overflow)
//    {
//        overflow = btreePageposCompress(overflow, newpostable,
//                                        "Bucket overflow");
//        SBT_BUCKOVERFLOW(buf, overflow);
//    }
*/

    keyptr = PBT_BUCKKEYLEN(buf);
    idptr = keyptr + (nentries* sizeof(ajuint));

    /*
    ** ajulong values in bucket are offset and refoffset in files for dups=1
    ** but for dups > 1 offset is the secondary root page for the duplicates
    */

    for(i=0;i<nentries;i++)
    {
	BT_GETAJUINT(keyptr,&klen);
        idlen = klen - keyskip;

	idptr += idlen + sizeof(ajuint);
	BT_GETAJUINT(idptr,&dups);
        idptr += sizeof(ajuint);

        if(dups)
        {
            BT_GETAJULONG(idptr,&lv);
            lv = btreePageposCompress(lv, newpostable,
                                      "Bucket dup offset (secrootblock)");
            BT_SETAJULONG(idptr, lv);
        }

        idptr += sizeof(ajulong) + refcount*sizeof(ajulong);
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
** @param [r] newpostable [const AjPTable] Table of new page positions
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void btreePageCompressNode(AjPBtpage page,
                                 const AjPTable newpostable)
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
        btreePageCompressNumnode(page, newpostable);
        return;
    }

    GBT_NKEYS(buf,&nkeys);
    GBT_OVERFLOW(buf,&overflow);

/*
//    if(overflow)
//    {
//        overflow = btreePageposCompress(overflow, newpostable,
//                                        "Node overflow");
//        SBT_OVERFLOW(buf, overflow);
//    }
*/    

    GBT_LEFT(buf,&left);

    if(left)
    {
        left = btreePageposCompress(left, newpostable,
                                    "Node left");
        SBT_LEFT(buf, left);
    }
    
    GBT_RIGHT(buf,&right);

    if(right > BT_NODEPREAMBLE)      /* not a level, must be a page position */
    {
        right = btreePageposCompress(right, newpostable,
                                     "Node right");
        SBT_RIGHT(buf, right);
    }
    
    GBT_PREV(buf,&prev);

    if(prev)
    {
        prev = btreePageposCompress(prev, newpostable,
                                    "Node prev");
        SBT_PREV(buf, prev);
    }
    
    GBT_BLOCKNUMBER(buf,&blockno);
    if(blockno)
    {
        blockno = btreePageposCompress(blockno, newpostable,
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
            lv = btreePageposCompress(lv, newpostable,
                                      "Node keypos");
            BT_SETAJULONG(keyptr, lv);
        }

        keyptr += sizeof(ajulong);
	lenptr += sizeof(ajuint);
    }

    BT_GETAJULONG(keyptr,&lv); /* pagepos after last key */

    if(lv)
    {
        lv = btreePageposCompress(lv, newpostable,
                                  "Node last keypos");
        BT_SETAJULONG(keyptr, lv);
    }

    return;
}




/* @funcstatic btreePageCompressNumnode ***************************************
**
** Compress a numeric node cache page using an array of new
** positions for uncompressed pages
**
** @param [u] page [AjPBtpage] Page
** @param [r] newpostable [const AjPTable] Table of new page positions
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void btreePageCompressNumnode(AjPBtpage page,
                                     const AjPTable newpostable)
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

/*
//    if(overflow)
//    {
//        overflow = btreePageposCompress(overflow, newpostable,
//                                        "Numnode overflow");
//        SBT_OVERFLOW(buf, overflow);
//    }
*/

    GBT_LEFT(buf,&left);

    if(left)
    {
        left = btreePageposCompress(left, newpostable,
                                    "Numnode left");
        SBT_LEFT(buf, left);
    }
    
    GBT_RIGHT(buf,&right);

    if(right > BT_NODEPREAMBLE)      /* not a level, must be a page position */
    {
        right = btreePageposCompress(right, newpostable,
                                     "Numnode right");
        SBT_RIGHT(buf, right);
    }
    
    GBT_PREV(buf,&prev);

    if(prev)
    {
        prev = btreePageposCompress(prev, newpostable,
                                    "Numnode prev");
        SBT_PREV(buf, prev);
    }
    
    GBT_BLOCKNUMBER(buf,&blockno);
    if(blockno)
    {
        blockno = btreePageposCompress(blockno, newpostable,
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
            lv = btreePageposCompress(lv, newpostable,
                                      "Numnode keypos");
            BT_SETAJULONG(valptr, lv);
        }

        valptr += sizeof(ajulong);
    }

    BT_GETAJULONG(valptr,&lv); /* pagepos after last key */

    if(lv)
    {
        lv = btreePageposCompress(lv, newpostable,
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
** @param [r] newpostable [const AjPTable] Table of new page positions
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void btreePageCompressPribucket(AjPBtpage page,
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

/*
//    if(overflow)
//    {
//        overflow = btreePageposCompress(overflow, newpostable,
//                                        "Pribucket overflow");
//        SBT_BUCKOVERFLOW(buf, overflow);
//    }
*/

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
            lv = btreePageposCompress(lv, newpostable,
                                      "Pribucket keypos");
            BT_SETAJULONG(lp, lv);
        }

	keyptr += sizeof(ajuint);
    }

    return;
}




/* @func ajBtreePageGetSize ***************************************************
**
** Return the used size of a cache page
**
** @param [r] page [const AjPBtpage] Page
*8 @param [r] refcount [ajuint] Number of reference offsets
** @return [ajuint] Used page size
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ajBtreePageGetSize(const AjPBtpage page, ajuint refcount)
{
    const unsigned char* buf = page->buf;
    ajuint nodetype;

    GBT_NODETYPE(buf, &nodetype);
    switch (nodetype) 
    {
        case BT_SECFREEPAGE:    /* no need to save the page */
        case BT_FREEPAGE:       /* no need to save the page */
            return 0;
        case BT_ROOT:
        case BT_INTERNAL:
        case BT_LEAF:
        case BT_SECROOT:
        case BT_SECINTERNAL:
        case BT_SECLEAF:
            return btreePageGetSizeNode(page);
        case BT_IDBUCKET:
            return btreePageGetSizeIdbucket(page, refcount);
        case BT_OVERFLOW:
        case BT_SECOVERFLOW:
            ajErr("Overflow page type %u in btreePageGetSize at %Lu",
                  nodetype, page->pagepos);
            return 0;
        case BT_PRIBUCKET:
            return btreePageGetSizePribucket(page);
        case BT_SECBUCKET:
            return btreePageGetSizeSecbucket(page);
        case BT_NUMBUCKET:
            return btreePageGetSizeNumbucket(page, refcount);
        default:
            ajErr("Unknown page type %u in btreePageGetSize at %Lu",
                  nodetype, page->pagepos);
            break;
    }
    return 0;
}




/* @funcstatic btreePageGetSizeIdbucket ***************************************
**
** Return the used size of a bucket cache page
**
** @param [r] page [const AjPBtpage] Page
** @param [r] refcount [ajuint] Number of reference offsets
** @return [ajuint] Used page size
**
** @release 6.4.0
** @@
******************************************************************************/

static ajuint btreePageGetSizeIdbucket(const AjPBtpage page, ajuint refcount)
{
    const unsigned char* buf = page->buf;
    ajuint nentries;
    ajulong   overflow = 0L;
    const unsigned char *idptr = NULL;
    const unsigned char *keyptr = NULL;
    ajuint klen;
    ajuint idlen;
    ajuint i;
    ajuint keyskip = BT_DDOFF + refcount*BT_EXTRA;

    buf = page->buf;

    GBT_BUCKNENTRIES(buf,&nentries);
    GBT_BUCKOVERFLOW(buf,&overflow);

    keyptr = PBT_BUCKKEYLEN(buf);
    idptr = keyptr + (nentries * sizeof(ajuint));

    for(i=0;i<nentries;i++)
    {
	BT_GETAJUINT(keyptr,&klen);
        idlen = klen - keyskip;
	idptr += (idlen + sizeof(ajuint) + sizeof(ajuint) +
                  sizeof(ajulong) + refcount*sizeof(ajulong));
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
**
** @release 6.4.0
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
** @param [r] refcount [ajuint] Number of reference offsets
** @return [ajuint] Used page size
**
** @release 6.4.0
** @@
******************************************************************************/

static ajuint btreePageGetSizeNumbucket(const AjPBtpage page,
                                        ajuint refcount)
{
    const unsigned char* buf = page->buf;
    ajuint nentries;
    ajulong   overflow = 0L;
    const unsigned char *keyptr = NULL;

    buf = page->buf;

    GBT_BUCKNENTRIES(buf,&nentries);
    GBT_BUCKOVERFLOW(buf,&overflow);

    keyptr = PBT_BUCKKEYLEN(buf);

    keyptr += nentries * (sizeof(ajlong) + refcount*sizeof(ajlong) +
                          sizeof(ajuint));

    return (keyptr-buf);
}




/* @funcstatic btreePageGetSizeNumnode ****************************************
**
** Return the used size of a numeric node cache page
**
** @param [r] page [const AjPBtpage] Page
** @return [ajuint] Used page size
**
** @release 6.4.0
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
**
** @release 6.4.0
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
**
** @release 6.4.0
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
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool btreeCheckNode(AjPBtcache cache, const AjPBtpage page)
{
    ajuint i;
    ajuint m;
    ajuint klen;
    const AjPBtpage bpage;
    unsigned char *tbuf;

/*    ajuint   pagesize = 0;*/
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

/*    ajuint nodetype;*/
/*    ajuint freebytes;*/

    if(!btreeCheckNodeHeader(cache, page, "node"))
        ret = ajFalse;

/*    pagesize = cache->pagesize;*/

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

/*
//	if((ajuint)((keyptr-tbuf+2) + klen + sizeof(ajulong)) > pagesize)
//	{
//            freebytes = pagesize - (keyptr-tbuf);
//    	    /# ajDebug("btreeStatNode: Overflow\n"); #/
//	    btreeNocacheFetch(cache, ncpage, overflow);
//	    tbuf = bpage->buf;
//	    GBT_NODETYPE(tbuf,&nodetype);
//	    if(nodetype != BT_OVERFLOW)
//		ajFatal("btreeCheckNode "
//                        "Overflow node %Lu expected but not found "
//                        "cache %S page %Lu "
//                        "pagepos:%Lu key:%u/%u free:%u keyptr:%x lenptr:%x "
//                        "tbuf:%x klen:%u old nodetype '%s' newnodetype '%s' "
//                        "blockno:%Lu totlen:%u left:%Lu right:%Lu prev:%Lu",
//                        overflow, cache->filename,
//                        page->pagepos, page->pagepos/pagesize,
//                        i, m, freebytes,
//                        keyptr, lenptr, tbuf, klen,
//                        btreeNodetype(page->buf), btreeNodetype(tbuf),
//                        blockno,totlen, left, right, prev);
//
//            GBT_BLOCKNUMBER(tbuf,&blockno);
//            GBT_TOTLEN(tbuf,&totlen);
//            GBT_LEFT(tbuf,&left);
//            GBT_RIGHT(tbuf,&right);
//            GBT_PREV(tbuf,&prev);
//
//	    /#
//	     ** The length pointer is restricted to the initial page.
//	     ** The keyptr in overflow pages starts at the Key Lengths
//	     ** position!
//	     #/
//            keyptr = PBT_KEYLEN(tbuf);
//	}
*/

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
**
** @release 6.4.0
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
        btreeStatNumnode(cache, page);
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
**
** @release 6.4.0
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

    if(ajBtreePageIsPrimary(page))
    {
        if(nkeys >= cache->porder)
        {
            ret = ajFalse;
            ajWarn("%s:%Lu nkeys: %u porder:%u in %S",
                   nodetype, page->pagepos, nkeys,
                   cache->porder,
                   cache->filename);
        }
    }
    else
    {
        if(nkeys > cache->sorder)
        {
            ret = ajFalse;
            ajWarn("%s:%Lu nkeys: %u sorder:%u in %S",
                   nodetype, page->pagepos, nkeys,
                   cache->sorder,
                   cache->filename);
        }
    }
    if((blockno != page->pagepos))
    {
        ret = ajFalse;
        ajWarn("%s:%Lu blockno: %Lu in %S",
               nodetype, page->pagepos, blockno,
               cache->filename);
    }

    return ret;
}




/* @funcstatic btreeNocacheFetch **********************************************
**
** Fetch a cache page from disc
**
** @param [r] cache [const AjPBtcache] cache
** @param [w] cpage [AjPBtpage] cache page 
** @param [r] pagepos [ajulong] page number
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void btreeNocacheFetch(const AjPBtcache cache, AjPBtpage cpage,
                              ajulong pagepos)
{
    ajuint sum = 0;
    ajuint retries = 0;
    ajuint pagesize;

    /* ajDebug("In btreeNoCacheFetch\n"); */
    
    if(cache->secpagesize > cache->pripagesize)
        pagesize = cache->pripagesize;
    else
        pagesize = cache->secpagesize;

    if(fseek(cache->fp,pagepos,SEEK_SET))
	ajFatal("Seek error '%s' in btreeNocacheFetch file %S",
                strerror(ferror(cache->fp)), cache->filename);
    
    while(sum != pagesize && retries != BT_MAXRETRIES)
    {
	sum += fread((void *)(cpage->buf+sum),1,pagesize-sum,
		     cache->fp);
	++retries;
    }
    
    if(retries == BT_MAXRETRIES)
	ajFatal("Maximum retries (%u) reached in btreeNocacheFetch "
                "for page %Lu",
		BT_MAXRETRIES,pagepos);

    if(ajBtreePageIsPrimary(cpage))
    {
        retries = 0;
        while(sum < cache->pripagesize && retries != BT_MAXRETRIES)
        {
            sum += fread((void *)(cpage->buf+sum),1,cache->pripagesize-sum,
                         cache->fp);
            ++retries;
        }
        if(retries == BT_MAXRETRIES)
            ajFatal("Maximum retries (%u) reached in btreeNocacheFetch "
                    "for page %Lu",
                    BT_MAXRETRIES,pagepos);
    }
    else
    {
        retries = 0;
        while(sum < cache->secpagesize && retries != BT_MAXRETRIES)
        {
            sum += fread((void *)(cpage->buf+sum),1,cache->secpagesize-sum,
                         cache->fp);
            ++retries;
        }
        if(retries == BT_MAXRETRIES)
            ajFatal("Maximum retries (%u) reached in btreeNocacheFetch "
                    "for page %Lu",
                    BT_MAXRETRIES,pagepos);
    }

    cpage->pagepos = pagepos;
    
    return;
}




/* @funcstatic btreePripageSetfree ********************************************
**
** Clear a primary cache page and set as free
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] Pahge number
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void btreePripageSetfree(AjPBtcache cache, ajulong pagepos)
{
    AjPBtpage page = NULL;
    unsigned char *p;
    ajuint nodetype;

    page = btreePricacheRead(cache,pagepos);

    if(!ajBtreePageIsPrimary(page))
        ajWarn("btreePripageSetfree secondary page %Lu '%s'",
               pagepos, btreeNodetype(page->buf));

    p = page->buf;
    AJCSET0(p, cache->pripagesize); /* clear buffer to zeros */

    page->next = NULL;
    page->prev = NULL;

    nodetype = BT_FREEPAGE;
    SBT_NODETYPE(p,nodetype);

    page->dirty = BT_DIRTY;
    
    return;
}




/* @funcstatic btreeSecpageSetfree *********************************************
**
** Clear a secondary cache page and set as free
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pagepos [ajulong] Pahge number
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void btreeSecpageSetfree(AjPBtcache cache, ajulong pagepos)
{
    AjPBtpage page = NULL;
    unsigned char *p;
    ajuint nodetype;

    page = btreeSeccacheRead(cache,pagepos);

    if(ajBtreePageIsPrimary(page))
        ajWarn("btreeSecpageSetfree primary page %Lu '%s'",
               pagepos, btreeNodetype(page->buf));

    p = page->buf;
    AJCSET0(p, cache->secpagesize); /* clear buffer to zeros */

    page->next = NULL;
    page->prev = NULL;

    nodetype = BT_SECFREEPAGE;
    SBT_NODETYPE(p,nodetype);

    page->dirty = BT_DIRTY;
    
    return;
}




/* @funcstatic btreePripageClear **********************************************
**
** Clear a primary cache page
**
** @param [u] cache [AjPBtcache] cache
** @param [u] page [AjPBtpage] Page
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void btreePripageClear(AjPBtcache cache, AjPBtpage page)
{
    unsigned char *p;
    ajuint nodetype;

    p = page->buf;

    if(!ajBtreePageIsPrimary(page))
        ajWarn("btreePripageClear secondary page %Lu '%s'",
               page->pagepos, btreeNodetype(page->buf));

    AJCSET0(p, cache->pripagesize); /* clear buffer to zeros */

    nodetype     = BT_FREEPAGE;
    SBT_NODETYPE(p,nodetype);

    page->next = NULL;
    page->prev = NULL;

    page->dirty = BT_CLEAN;

    return;
}




/* @funcstatic btreeSecpageClear **********************************************
**
** Clear a secondary cache page
**
** @param [u] cache [AjPBtcache] cache
** @param [u] page [AjPBtpage] Page
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void btreeSecpageClear(AjPBtcache cache, AjPBtpage page)
{
    unsigned char *p;
    ajuint nodetype;

    p = page->buf;

    if(ajBtreePageIsPrimary(page))
        ajWarn("btreeSecpageClear primary page %Lu '%s'",
               page->pagepos, btreeNodetype(page->buf));

    AJCSET0(p, cache->secpagesize); /* clear buffer to zeros */

    nodetype     = BT_SECFREEPAGE;
    SBT_NODETYPE(p,nodetype);

    page->next = NULL;
    page->prev = NULL;

    page->dirty = BT_CLEAN;

    return;
}




/* @func ajBtreeStatsOut ******************************************************
**
** Reports overall statistics from B+tree indexing since the last call
**
** @param [u] outf [AjPFile] output file file
** @param [u] Psplitrootid [ajulong*] Number of hyb splitroot calls to date
** @param [u] Psplitrootnum [ajulong*] Number of num splitroot calls to date
** @param [u] Psplitrootkey [ajulong*] Number of pri splitroot calls to date
** @param [u] Psplitrootsec [ajulong*] Number of sec splitroot calls to date
** @param [u] Psplitleafid  [ajulong*] Number of id splitleaf calls to date
** @param [u] Psplitleafnum [ajulong*] Number of num splitleaf calls to date
** @param [u] Psplitleafkey [ajulong*] Number of pri splitleaf calls to date
** @param [u] Psplitleafsec [ajulong*] Number of sec splitleaf calls to date
** @param [u] Preorderid  [ajulong*] Number of id reorderings to date
** @param [u] Preordernum [ajulong*] Number of num reorderings to date
** @param [u] Preorderkey [ajulong*] Number of pri reorderings to date
** @param [u] Preordersec [ajulong*] Number of sec reorderings to date
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ajBtreeStatsOut(AjPFile outf,
                     ajulong* Psplitrootid,  ajulong* Psplitrootnum,
                     ajulong* Psplitrootkey, ajulong* Psplitrootsec,
                     ajulong* Psplitleafid,  ajulong* Psplitleafnum,
                     ajulong* Psplitleafkey, ajulong* Psplitleafsec,
                     ajulong* Preorderid,    ajulong* Preordernum,
                     ajulong* Preorderkey,   ajulong* Preordersec)
{
    ajulong splitrootid;
    ajulong splitrootnum;
    ajulong splitrootkey;
    ajulong splitrootsec;
    ajulong splitleafid;
    ajulong splitleafnum;
    ajulong splitleafkey;
    ajulong splitleafsec;
    ajulong reorderid;
    ajulong reordernum;
    ajulong reorderkey;
    ajulong reordersec;

    splitrootid  = statCallIdSplitroot  - *Psplitrootid;
    splitrootnum = statCallNumSplitroot - *Psplitrootnum;
    splitrootkey = statCallPriSplitroot - *Psplitrootkey;
    splitrootsec = statCallSecSplitroot - *Psplitrootsec;
    splitleafid  = statCallIdSplitleaf  - *Psplitleafid;
    splitleafnum = statCallNumSplitleaf - *Psplitleafnum;
    splitleafkey = statCallKeySplitleaf - *Psplitleafkey;
    splitleafsec = statCallSecSplitleaf - *Psplitleafsec;
    reorderid  = statCallIdbucketsReorder - *Preorderid;
    reordernum = statCallNumbucketsReorder - *Preordernum;
    reorderkey = statCallPribucketsReorder - *Preorderkey;
    reordersec = statCallSecbucketsReorder - *Preordersec;

    ajFmtPrintF(outf,
                "ajBtreeStatsOut splitroot              id: %8Lu "
                "num: %8Lu key: %8Lu sec: %8Lu\n",
                splitrootid, splitrootnum,
                splitrootkey, splitrootsec);

    ajFmtPrintF(outf,
                "ajBtreeStatsOut splitleaf id: %8Lu "
                "num: %8Lu key: %8Lu sec: %8Lu\n",
                splitleafid, splitleafnum,
                splitleafkey, splitleafsec);

    ajFmtPrintF(outf,
                "ajBtreeStatsOut reorder   id: %8Lu "
                "num: %8Lu key: %8Lu sec: %8Lu\n",
                reorderid, reordernum,
                reorderkey, reordersec);

    *Psplitrootid  = statCallIdSplitroot;
    *Psplitrootnum = statCallNumSplitroot;
    *Psplitrootkey = statCallPriSplitroot;
    *Psplitrootsec = statCallSecSplitroot;

    *Psplitleafid  = statCallIdSplitleaf;
    *Psplitleafnum = statCallNumSplitleaf;
    *Psplitleafkey = statCallKeySplitleaf;
    *Psplitleafsec = statCallSecSplitleaf;

    *Preorderid    = statCallIdbucketsReorder;
    *Preordernum   = statCallNumbucketsReorder;
    *Preorderkey   = statCallPribucketsReorder;
    *Preordersec   = statCallSecbucketsReorder;

    return;
}




/* @func ajBtreeExit **********************************************************
**
** Cleans up B+tree indexing internal memory
**
** @return [void]
**
** @release 6.4.0
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

    ajDebug("   statSaveIdbucketNext: %u\n", statSaveIdbucketNext);
    while(statSaveIdbucketNext)
        btreeIdbucketFree(&statSaveIdbucket[--statSaveIdbucketNext]);
    AJFREE(statSaveIdbucket);
    statSaveIdbucketMax=0;
    statSaveIdbucketNext=0;

    ajDebug("   statSaveIdbucketEmptyNext: %u\n", statSaveIdbucketEmptyNext);
    while(statSaveIdbucketEmptyNext)
        btreeIdbucketFree(&statSaveIdbucketEmpty[--statSaveIdbucketEmptyNext]);
    AJFREE(statSaveIdbucketEmpty);
    statSaveIdbucketEmptyMax=0;
    statSaveIdbucketEmptyNext=0;

    ajDebug("   statSavePribucketNext: %u\n", statSavePribucketNext);
    while(statSavePribucketNext)
        btreePribucketFree(&statSavePribucket[--statSavePribucketNext]);
    AJFREE(statSavePribucket);
    statSavePribucketMax=0;
    statSavePribucketNext=0;

    ajDebug("   statSavePribucketEmptyNext: %u\n", statSavePribucketEmptyNext);
    while(statSavePribucketEmptyNext)
        btreePribucketFree(&statSavePribucketEmpty[--statSavePribucketEmptyNext]);
    AJFREE(statSavePribucketEmpty);
    statSavePribucketEmptyMax=0;
    statSavePribucketEmptyNext=0;

    ajDebug("   statSaveSecbucketNext: %u\n", statSaveSecbucketNext);
    while(statSaveSecbucketNext)
        btreeSecbucketFree(&statSaveSecbucket[--statSaveSecbucketNext]);
    AJFREE(statSaveSecbucket);
    statSaveSecbucketMax=0;
    statSaveSecbucketNext=0;

    ajDebug("   statSaveSecbucketEmptyNext: %u\n", statSaveSecbucketEmptyNext);
    while(statSaveSecbucketEmptyNext)
        btreeSecbucketFree(&statSaveSecbucketEmpty[--statSaveSecbucketEmptyNext]);
    AJFREE(statSaveSecbucketEmpty);
    statSaveSecbucketEmptyMax=0;
    statSaveSecbucketEmptyNext=0;

    ajDebug("   statSaveNumbucketNext: %u\n", statSaveNumbucketNext);
    while(statSaveNumbucketNext)
        btreeNumbucketFree(&statSaveNumbucket[--statSaveNumbucketNext]);
    AJFREE(statSaveNumbucket);
    statSaveNumbucketMax=0;
    statSaveNumbucketNext=0;

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

    ajDebug("   statSaveBtreeHitrefNext: %u\n", statSaveBtreeHitrefNext);
    while(statSaveBtreeHitrefNext)
        btreeHitrefFree(&statSaveBtreeHitref[--statSaveBtreeHitrefNext]);
    AJFREE(statSaveBtreeHitref);
    statSaveBtreeHitrefMax=0;
    statSaveBtreeHitrefNext=0;

     ajDebug("   statSaveBtreeHitNext: %u\n", statSaveBtreeHitNext);
    while(statSaveBtreeHitNext)
        btreeHitFree(&statSaveBtreeHit[--statSaveBtreeHitNext]);
    AJFREE(statSaveBtreeHit);
    statSaveBtreeHitMax=0;
    statSaveBtreeHitNext=0;

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

    ajDebug("Split root:%Lu Sec:%Lu Hyb:%Lu Num:%Lu\n",
            statCallPriSplitroot,statCallSecSplitroot,
            statCallIdSplitroot,statCallNumSplitroot);

    ajDebug("Split leaf:%Lu Pri:%Lu Sec:%Lu Id:%Lu Num:%Lu\n",
            statCallKeySplitleaf,statCallSecSplitleaf,
            statCallIdSplitleaf,statCallNumSplitleaf);

    ajDebug("Split reorder:%Lu Pri:%Lu Sec:%Lu  Id:%Lu Num:%Lu\n",
            statCallIdbucketsReorder,
            statCallPribucketsReorder,statCallSecbucketsReorder,
            statCallIdbucketsReorder,statCallNumbucketsReorder);

    ajStrDel(&btreeFieldnameTmp);
    ajTableMapDel(btreeFieldsTable, &btreeFieldMapDel, NULL);
    ajTableFree(&btreeFieldsTable);

    tmpfree = (void *) btreeNodetypeNames;
    AJFREE(tmpfree);
    btreeNodetypeNames = NULL;

    if(btreeTestpage)
    {
        AJFREE(btreeTestpage->buf);
        AJFREE(btreeTestpage);
    }

    ajStrDel(&indexKeyword);
    ajStrDel(&indexId);

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
**
** @release 6.4.0
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




/* @func ajBtreeIdHash ********************************************************
**
** Hash function for a table with a B+tree primary key
**
** @param [r] key [const void*] Standard argument. Table key.
** @param [r] hashsize [ajulong] Standard argument. Estimated Hash size.
** @return [ajulong] Hash value in range 0 to hashsize-1
**
** @release 6.4.0
** @@
******************************************************************************/

ajulong ajBtreeIdHash(const void* key, ajulong hashsize)
{
    const AjPBtId id;
    ajulong hash;
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




/* @func ajBtreeHitCmp ********************************************************
**
** Comparison function for sorting B+tree hit objects by database file number
** and offset.
**
** Using the file and position allows unique identifiers to be
** compared with other primary keys such bas accession number.
**
** @param [r] x [const void*] Standard argument. Item value.
** @param [r] y [const void*] Standard argument. Comparison item value.
** @return [ajint] Comparison result. Zero if equal, non-zero if different.
**
** @release 6.5.0
** @@
******************************************************************************/

ajint ajBtreeHitCmp(const void* x, const void* y)
{
    const AjPBtHit hitx;
    const AjPBtHit hity;

    hitx = (const AjPBtHit) x;
    hity = (const AjPBtHit) y;

    if(hitx->dbno > hity->dbno)
        return 1;

    if(hitx->dbno < hity->dbno)
        return -1;

    if(hitx->offset > hity->offset)
        return 1;

    if(hitx->offset < hity->offset)
        return -1;

    return 0;
}




/* @func ajBtreeHitHash *******************************************************
**
** Hash function for a table with a B+tree hit primary key
**
** @param [r] key [const void*] Standard argument. Table key.
** @param [r] hashsize [ajulong] Standard argument. Estimated Hash size.
** @return [ajulong] Hash value in range 0 to hashsize-1
**
** @release 6.5.0
** @@
******************************************************************************/

ajulong ajBtreeHitHash(const void* key, ajulong hashsize)
{
    const AjPBtHit hit;
    ajulong hash;
    ajulong ia;

    hit = (const AjPBtHit) key;

     if(!key)
        return 0;

    if(!hashsize)
        return 0;

    ia = hit->offset + (ajulong) hit->dbno;

    hash = (ia >> 2) % hashsize;

    return hash;
}




/* @func ajBtreeHitrefCmp *****************************************************
**
** Comparison function for sorting B+tree reference hit objects by
** database file number and offset.
**
** Using the file and position allows unique identifiers to be
** compared with other primary keys such bas accession number.
**
** @param [r] x [const void*] Standard argument. Item value.
** @param [r] y [const void*] Standard argument. Comparison item value.
** @return [ajint] Comparison result. Zero if equal, non-zero if different.
**
** @release 6.5.0
** @@
******************************************************************************/

ajint ajBtreeHitrefCmp(const void* x, const void* y)
{
    const AjPBtHitref hitrefx;
    const AjPBtHitref hitrefy;

    hitrefx = (const AjPBtHitref) x;
    hitrefy = (const AjPBtHitref) y;

    if(hitrefx->dbno > hitrefy->dbno)
        return 1;

    if(hitrefx->dbno < hitrefy->dbno)
        return -1;

    if(hitrefx->offset > hitrefy->offset)
        return 1;

    if(hitrefx->offset < hitrefy->offset)
        return -1;

    return 0;
}




/* @func ajBtreeHitrefHash ****************************************************
**
** Hash function for a table with a B+tree reference hit primary key
**
** @param [r] key [const void*] Standard argument. Table key.
** @param [r] hashsize [ajulong] Standard argument. Estimated Hash size.
** @return [ajulong] Hash value in range 0 to hashsize-1
**
** @release 6.5.0
** @@
******************************************************************************/

ajulong ajBtreeHitrefHash(const void* key, ajulong hashsize)
{
    const AjPBtHitref hitref;
    ajulong hash;
    ajulong ia;

    hitref = (const AjPBtHitref) key;

     if(!key)
        return 0;

    if(!hashsize)
        return 0;

    ia = hitref->offset + (ajulong) hitref->dbno;

    hash = (ia >> 2) % hashsize;

    return hash;
}




#if AJINDEX_STATIC
/* @funcstatic btreePageDump **************************************************
**
** Dump a page
**
** @param [r] cache [const AjPBtcache] Page
** @param [r] page [const AjPBtpage] Page
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void btreePageDump(const AjPBtcache cache, const AjPBtpage page)
{
    unsigned char *buf = page->buf;
    AjBool isprimary;
    ajuint i=0;
    ajuint pagesize;

    isprimary =  ajBtreePageIsPrimary(page);
    if(isprimary)
        pagesize = cache->pripagesize;
    else
        pagesize = cache->secpagesize;

    ajUser("PageDump page %Lu '%s' primary: %B",
           page->pagepos, btreeNodetype(buf), isprimary);

    i = 0;
    while(i < pagesize)
    {
        ajUser("%4x %2x%2x%2x%2x %2x%2x%2x%2x %2x%2x%2x%2x %2x%2x%2x%2x",
               i,
               buf[i],buf[i+1],buf[i+2],buf[i+3],
               buf[i+4],buf[i+5],buf[i+6],buf[i+7],
               buf[i+8],buf[i+9],buf[i+10],buf[i+11],
               buf[i+12],buf[i+13],buf[i+14],buf[i+15] );

        i += 16;
    }

    ajUser("pagesize: %u", pagesize);

    return;
}
#endif


    
#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif




#ifdef AJ_COMPILE_DEPRECATED
/* @obsolete ajBtreeWriteParams
** @rename ajBtreeWriteParamsC
*/
__deprecated void ajBtreeWriteParams(const AjPBtcache cache, const char *fn,
			const char *ext, const char *idir)
{
    ajBtreeWriteParamsC(cache, fn, ext, idir);
    return;
}
#endif

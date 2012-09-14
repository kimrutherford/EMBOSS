/* @include ajindex ***********************************************************
**
** B+ Tree Indexing plus Disc Cache.
**
** @author Copyright (c) 2003 Alan Bleasby
** @version $Revision: 1.43 $
** @modified  subsequently heavily modified by Peter Rice
** @modified $Date: 2012/07/02 17:17:16 $ by $Author: rice $
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

#ifndef AJINDEX_H
#define AJINDEX_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajstr.h"
#include "ajtable.h"
#include "ajlist.h"
#include "ajfile.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




#define BT_PAGESIZE BUFSIZ      /* Default cache page size
                                **  use C default buffer size */
#define BT_CACHESIZE 100

#define BT_MAXRETRIES   100     /* Maximum number of read/write attempts */
#define BT_KWLIMIT     15

#define BT_LOCK  2
#define BT_DIRTY 1
#define BT_CLEAN 0

#define BT_READ  ajTrue
#define BT_WRITE ajFalse


#define BT_ROOT           1
#define BT_INTERNAL       2
#define BT_LEAF           4
#define BT_IDBUCKET       8
#define BT_OVERFLOW      16
#define BT_PRIBUCKET     32
#define BT_SECBUCKET     64
#define BT_NUMBUCKET    128
#define BT_FREEPAGE     256
#define BT_SECFREEPAGE  512
#define BT_SECROOT     1024
#define BT_SECINTERNAL 2048
#define BT_SECLEAF     4096
#define BT_SECOVERFLOW 8192
#define BT_MAXTYPE     8192

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
 *  i.e. dbno + dups + offset
 * then later add refoffset
 */
#define BT_DDOFF (sizeof(ajuint) + sizeof(ajuint) + sizeof(ajulong))

/*
 *  Preamble before key lengths in an internal node
 * i.e. nodetype+blocknumber+nkeys+totlen+left+right+overflow+prev
 */
#define BT_NODEPREAMBLE (sizeof(ajuint) + sizeof(ajulong) + sizeof(ajuint) + \
                         sizeof(ajuint) + sizeof(ajulong) + sizeof(ajulong) + \
                         sizeof(ajulong) + sizeof(ajulong))

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
 *  i.e. dbno + offset
 * then later add refoffsets
 */
#define BT_DOFF (sizeof(ajuint) + sizeof(ajulong))

#define BT_EXTRA (sizeof(ajulong))


/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* @data AjPBtNode ************************************************************
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
******************************************************************************/

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




/* @data AjPBtMem *************************************************************
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
******************************************************************************/

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




/* @data AjPBtId **************************************************************
**
** Btree ID
**
** @attr id [AjPStr] Unique ID
** @attr offset [ajulong] Offset within database file (ftello)
** @attr refoffsets [ajulong*] Offset within reference database file(s) (ftello)
** @attr dbno [ajuint] Database file number
** @attr dups [ajuint] Duplicates
** @attr refcount [ajuint] Number of reference files
** @attr Padding [char[4]] Padding to alignment boundary
******************************************************************************/

typedef struct AjSBtId
{
    AjPStr id;
    ajulong offset;
    ajulong *refoffsets;
    ajuint  dbno;
    ajuint  dups;
    ajuint  refcount;
    char Padding[4];
} AjOBtId;

#define AjPBtId AjOBtId*




/* @data AjPBHit *************************************************************
**
** Btree index hit minimal information for refcount zero
**
** @attr dbno [ajuint] Database file number
** @attr refcount [ajuint] Number of reference files (zero, for padding)
** @attr offset [ajulong] Offset within database file (ftello)
******************************************************************************/

typedef struct AjSBtHit
{
    ajuint  dbno;
    ajuint  refcount;
    ajulong offset;
} AjOBtHit;

#define AjPBtHit AjOBtHit*




/* @data AjPBHitref **********************************************************
**
** Btree index hit minimal information for refcount non-zero
**
** @attr dbno [ajuint] Database file number
** @attr refcount [ajuint] Number of reference files (one, for padding)
** @attr offset [ajulong] Offset within database file (ftello)
** @attr refoffset [ajulong] Offset within reference file (ftello)
******************************************************************************/

typedef struct AjSBtHitref
{
    ajuint  dbno;
    ajuint  refcount;
    ajulong offset;
    ajulong refoffset;
} AjOBtHitref;

#define AjPBtHitref AjOBtHitref*




/* @data AjPBtIdwild **********************************************************
**
** Btree identifier search wildcard
**
** @attr id [AjPStr] Wildcard ID
** @attr pagepos [ajulong] Page number of leaf
** @attr list [AjPList] list of AjPBtIds
** @attr first [AjBool] true for first search
** @attr Padding [char[4]] Padding to alignment boundary
******************************************************************************/

typedef struct AjSBIdild
{
    AjPStr id;
    ajulong pagepos;
    AjPList list;
    AjBool first;
    char Padding[4];
} AjOBtIdwild;

#define AjPBtIdwild AjOBtIdwild*




/* @data AjPIdbucket **********************************************************
**
** Id bucket structure on disc
**
** Key, file number, ftell ID, subkey page (char*, ajuint, ajulong, ajulong)
**
** @attr NodeType [ajuint] Node type
** @attr Maxentries [ajuint] Number of entries available
** @attr Nentries [ajuint] Number of entries
** @attr Padding [ajuint] Padding to alignment boundary
** @attr Overflow [ajulong] Offset to overflow block
** @attr keylen [ajuint*] Key lengths
** @attr Ids [AjPBtId*] Ids
******************************************************************************/

typedef struct AjSIdbucket
{
    ajuint   NodeType;
    ajuint   Maxentries;
    ajuint   Nentries;
    ajuint   Padding;
    ajulong  Overflow;
    ajuint  *keylen;
    AjPBtId *Ids;
} AjOIdbucket;

#define AjPIdbucket AjOIdbucket*




/* @data AjPBtNumId ***********************************************************
**
** Btree ID
**
** @attr offset [ajulong] Offset within database file (ftello)
** @attr refoffsets [ajulong*] Offset within reference database file(s) (ftello)
** @attr dbno [ajuint] Database file number
** @attr refcount [ajuint] Number of reference files
******************************************************************************/

typedef struct AjSBtNumId
{
    ajulong offset;
    ajulong *refoffsets;
    ajuint  dbno;
    ajuint  refcount;
} AjOBtNumId;

#define AjPBtNumId AjOBtNumId*




/* @data AjPNumbucket *********************************************************
**
** Offset bucket structure on disc
**
** Key, file number, ftell ID, subkey page (char*, ajuint, ajulong, ajulong)
**
** @attr NodeType [ajuint] Node type
** @attr Maxentries [ajuint] Number of entries available
** @attr Nentries [ajuint] Number of entries
** @attr Padding [ajuint] Padding to alignment boundary
** @attr Overflow [ajulong] Offset to overflow block
** @attr NumId [AjPBtNumId*] secondary tree IDs
******************************************************************************/

typedef struct AjSNumbucket
{
    ajuint   NodeType;
    ajuint   Maxentries;
    ajuint   Nentries;
    ajuint   Padding;
    ajulong  Overflow;
    AjPBtNumId *NumId;
} AjONumbucket;

#define AjPNumbucket AjONumbucket*




/* @data AjPBtpage ************************************************************
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




/* @data AjPBtcache ***********************************************************
**
** B+ tree cache
**
** @attr fp [FILE*] Tree index file pointer
** @attr filename [AjPStr] Filename
** @attr plru [AjPBtpage] Least recently used primary cache page
** @attr pmru [AjPBtpage] Most recently used primary cache page
** @attr slru [AjPBtpage] Least recently used secondary cache page
** @attr smru [AjPBtpage] Most recently used secondary cache page
** @attr bmem [AjPBtMem] Primary array allocation MRU bottom
** @attr tmem [AjPBtMem] Primary array allocation MRU top
** @attr bsmem [AjPBtMem] Secondary array allocation MRU bottom
** @attr tsmem [AjPBtMem] Secondary array allocation MRU top
** @attr replace [AjPStr] Replacement ID
** @attr pripagetable [AjPTable] Table of cached primary pages
** @attr secpagetable [AjPTable] Table of cached secondary pages
** @attr totsize [ajulong] Tree index total length
** @attr filesize [ajulong] Tree index length after any compression
** @attr pripagecount [ajulong] Tree index number of primary pages
** @attr secpagecount [ajulong] Tree index number of secondary pages
** @attr secrootblock [ajulong] Secondary tree root block
** @attr numreplace [ajulong] Replacement numeric ID
** @attr countunique [ajulong] Number of unique tokens indexed
** @attr countall [ajulong] Number of total tokens indexed
** @attr pricachehits [ajulong] Number of primnary cached page reads
** @attr seccachehits [ajulong] Number of secondary cached page reads
** @attr prireads [ajulong] Number of physical primary page reads from disk
** @attr secreads [ajulong] Number of physical secondary page reads from disk
** @attr priwrites [ajulong] Number of physical primary page writes to disk
** @attr secwrites [ajulong] Number of physical secondary page writes to disk
** @attr pripagesize [ajuint] Size of primary cache pages
** @attr secpagesize [ajuint] Size of secondary cache pages
** @attr prilistLength [ajuint] Number of pages in primary cache
** @attr seclistLength [ajuint] Number of pages in secondary cache
** @attr porder [ajuint] Order of primary tree
** @attr plevel [ajuint] Depth of primary tree
** @attr pnperbucket [ajuint] Number of entries in a primary bucket
** @attr pricachesize [ajuint] Maximum number of pages to cache
** @attr sorder [ajuint] Order of secondary tree
** @attr slevel [ajuint] Depth of secondary tree
** @attr snperbucket [ajuint] Number of entries in a secondary bucket
** @attr seccachesize [ajuint] Maximum number of pages to cache
** @attr keylimit [ajuint] Max length of primary key
** @attr idlimit [ajuint] Max length of secondary key
** @attr refcount [ajuint] Number of extra files for each entry
** @attr secondary [AjBool] Secondary index
** @attr readonly [AjBool] Read only flag
** @attr dodelete [AjBool] Deletion flag
** @attr compressed [AjBool] Index is compressed
******************************************************************************/

typedef struct AjSBtCache
{
    FILE *fp;
    AjPStr filename;
    AjPBtpage plru;
    AjPBtpage pmru;
    AjPBtpage slru;
    AjPBtpage smru;
    AjPBtMem bmem;
    AjPBtMem tmem;
    AjPBtMem bsmem;
    AjPBtMem tsmem;
    AjPStr replace;
    AjPTable pripagetable;
    AjPTable secpagetable;
    ajulong totsize;
    ajulong filesize;
    ajulong pripagecount;
    ajulong secpagecount;
    ajulong secrootblock;
    ajulong numreplace;
    ajulong countunique;
    ajulong countall;
    ajulong pricachehits;
    ajulong seccachehits;
    ajulong prireads;
    ajulong secreads;
    ajulong priwrites;
    ajulong secwrites;
    ajuint pripagesize;
    ajuint secpagesize;
    ajuint prilistLength;
    ajuint seclistLength;
    ajuint porder;
    ajuint plevel;
    ajuint pnperbucket;
    ajuint pricachesize;
    ajuint sorder;
    ajuint slevel;
    ajuint snperbucket;
    ajuint seccachesize;
    ajuint keylimit;
    ajuint idlimit;
    ajuint refcount;
    AjBool secondary;
    AjBool readonly;
    AjBool dodelete;
    AjBool compressed;
} AjOBtcache;

#define AjPBtcache AjOBtcache*




/* @data AjPBtPri *************************************************************
**
** Btree primary link from a primary bucket for secondary identifier tree
**
** @attr keyword [AjPStr] keyword
** @attr treeblock [ajulong] root block of secondary tree
** @attr id [AjPStr] Id string
******************************************************************************/

typedef struct AjSBtPri
{
    AjPStr keyword;
    ajulong treeblock;
    AjPStr id;
} AjOBtPri;

#define AjPBtPri AjOBtPri*




/* @data AjPPribucket *********************************************************
**
** Keyword primary bucket structure on disc
**
** @attr NodeType [ajuint] Node type
** @attr Maxentries [ajuint] Number of entries available
** @attr Nentries [ajuint] Number of entries
** @attr Padding [ajuint] Padding to alignment boundary
** @attr Overflow [ajulong] Offset to overflow block
** @attr keylen [ajuint*] key lengths
** @attr codes [AjPBtPri*] Primary keywords
******************************************************************************/

typedef struct AjSPribucket
{
    ajuint    NodeType;
    ajuint    Maxentries;
    ajuint    Nentries;
    ajuint    Padding;
    ajulong   Overflow;
    ajuint   *keylen;
    AjPBtPri *codes;
} AjOPribucket;

#define AjPPribucket AjOPribucket*




/* @data AjPSecbucket *********************************************************
**
** Keyword secondary bucket structure on disc
**
** @attr NodeType [ajuint] Node type
** @attr Maxentries [ajuint] Number of entries available
** @attr Nentries [ajuint] Number of entries
** @attr Padding [ajuint] Padding to alignment boundary
** @attr Overflow [ajulong] Offset to overflow block
** @attr keylen [ajuint*] key lengths
** @attr SecIds [AjPStr*] Ids
******************************************************************************/

typedef struct AjSSecbucket
{
    ajuint    NodeType;
    ajuint    Maxentries;
    ajuint    Nentries;
    ajuint    Padding;
    ajulong   Overflow;
    ajuint   *keylen;
    AjPStr   *SecIds;
} AjOSecbucket;

#define AjPSecbucket AjOSecbucket*




/* @data AjPBtKeywild *********************************************************
**
** Btree keyword wildcard object
**
** @attr keyword [AjPStr] Wildcard keyword
** @attr prefix [AjPStr] Wildcard keyword prefix
** @attr pagepos [ajulong] Page number of primary tree leaf
** @attr list [AjPList] list of AjPBtPris
** @attr cache [AjPBtcache] cache for secondary tree
** @attr idlist [AjPList] list of AjPStr IDs
** @attr secpagepos [ajulong] Page number of secondary tree leaf
** @attr first [AjBool] true for first search
** @attr Padding [char[4]] Padding to alignment boundary
******************************************************************************/

typedef struct AjSBtKeywild
{
    AjPStr keyword;
    AjPStr prefix;
    ajulong pagepos;
    AjPList list;
    AjPBtcache cache;
    AjPList idlist;
    ajulong secpagepos;
    AjBool first;
    char Padding[4];
} AjOBtKeywild;

#define AjPBtKeywild AjOBtKeywild*




#if 0
/* #data AjPBtHybrid **********************************************************
**
** Btree ID
**
** #attr key1 [AjPStr] Unique ID
** #attr offset [ajulong] Offset within database file (ftello)
** #attr refoffsets [ajulong*] Offset within reference database file(s) (ftello)
** #attr treeblock [ajulong] Secondary tree root page
** #attr dbno [ajuint] Database file number
** #attr dups [ajuint] Duplicates
** #attr refcount [ajuint] Number of reference files
** #attr Padding [char[4]] Padding to alignment boundary
******************************************************************************/
/*
typedef struct AjSBtHybrid
{
    AjPStr key1;
    ajulong offset;
    ajulong *refoffsets;
/#    ajulong treeblock;#/
    ajuint  dbno;
    ajuint  dups;
    ajuint  refcount;
    char Padding[4];
} AjOBtHybrid;

#define AjPBtHybrid AjOBtHybrid*
*/
#endif


/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */




/* Database file name structure
**
** ajuint        order                  Order of B+tree
** ajuint        m                      Max entries per bucket
** ajuint       NFiles                  Number of Indexed files
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
#define BT_SETAJINT(p,v)  ajByteRevInt(&v);             \
    memcpy((void*)p,(const void*)&v,sizeof(ajint))
#define BT_SETAJUINT(p,v) ajByteRevUint(&v);            \
    memcpy((void*)p,(const void*)&v,sizeof(ajuint))
#define BT_SETAJLONG(p,v) ajByteRevLong(&v);            \
    memcpy((void*)p,(const void*)&v,sizeof(ajlong))
#define BT_SETAJULONG(p,v) ajByteRevUlong(&v);          \
    memcpy((void*)p,(const void*)&v,sizeof(ajulong))
#endif


#define BT_BUCKIDLEN(str) (MAJSTRGETLEN(str) + 1 + sizeof(ajuint) +      \
                           sizeof(ajuint) + sizeof(ajulong))


/*
** Macros to determine entry positions within a bucket
*/

#define PBT_BUCKNODETYPE(p) p
#define PBT_BUCKNENTRIES(p) (p + sizeof(ajuint))
#define PBT_BUCKOVERFLOW(p) (p + sizeof(ajuint) + sizeof(ajuint))
#define PBT_BUCKKEYLEN(p) (p + sizeof(ajuint) + sizeof(ajuint) + sizeof(ajulong))
#define BT_BUCKPRILEN(str) (MAJSTRGETLEN(str) + 1 + sizeof(ajulong))
#define BT_BUCKSECLEN(str) (MAJSTRGETLEN(str) +1)

/*
** Macros to return a page entry value within a bucket
*/

#if !defined(WORDS_BIGENDIAN)
#define GBT_BUCKNODETYPE(p,v) (memcpy((void*)v,                         \
                                      (const void*)PBT_BUCKNODETYPE(p), \
                                      sizeof(ajuint)))
#define GBT_BUCKNENTRIES(p,v) (memcpy((void*)v,                         \
                                      (const void*)PBT_BUCKNENTRIES(p), \
                                      sizeof(ajuint)))
#define GBT_BUCKOVERFLOW(p,v) (memcpy((void*)v,                         \
                                      (const void*)PBT_BUCKOVERFLOW(p), \
                                      sizeof(ajulong)))
#else
#define GBT_BUCKNODETYPE(p,v) memcpy((void*)v,                          \
                                     (const void*)PBT_BUCKNODETYPE(p),  \
                                     sizeof(ajuint));                   \
    ajByteRevUint(v)
#define GBT_BUCKNENTRIES(p,v) memcpy((void*)v,                          \
                                     (const void*)PBT_BUCKNENTRIES(p),  \
                                     sizeof(ajuint));                   \
    ajByteRevUint(v)
#define GBT_BUCKOVERFLOW(p,v) memcpy((void*)v,                          \
                                     (const void*)PBT_BUCKOVERFLOW(p),  \
                                     sizeof(ajulong));                  \
    ajByteRevUlong(v)
#endif


/*
** Macros to set a page entry value within an internal/leaf node
*/

#if !defined(WORDS_BIGENDIAN)
#define SBT_BUCKNODETYPE(p,v) (memcpy((void*)PBT_BUCKNODETYPE(p),       \
                                      (const void*)&v,sizeof(ajuint)))
#define SBT_BUCKNENTRIES(p,v) (memcpy((void*)PBT_BUCKNENTRIES(p),       \
                                      (const void*)&v,sizeof(ajuint)))
#define SBT_BUCKOVERFLOW(p,v) (memcpy((void*)PBT_BUCKOVERFLOW(p),       \
                                      (const void*)&v,sizeof(ajulong)))
#else
#define SBT_BUCKNODETYPE(p,v) ajByteRevUint(&v);        \
    memcpy((void*)PBT_BUCKNODETYPE(p),                  \
           (const void*)&v,sizeof(ajuint))
#define SBT_BUCKNENTRIES(p,v) ajByteRevUint(&v);        \
    memcpy((void*)PBT_BUCKNENTRIES(p),                  \
           (const void*)&v,sizeof(ajuint))
#define SBT_BUCKOVERFLOW(p,v) ajByteRevUlong(&v);       \
    memcpy((void*)PBT_BUCKOVERFLOW(p),                  \
           (const void*)&v,sizeof(ajulong))
#endif

/*
** Macros to determine entry positions within an internal/leaf node
*/

#define PBT_NODETYPE(p) p
#define PBT_BLOCKNUMBER(p) (p + sizeof(ajuint))
#define PBT_NKEYS(p) (p + sizeof(ajuint) + sizeof(ajulong))
#define PBT_TOTLEN(p) (p+sizeof(ajuint)+sizeof(ajulong)+sizeof(ajuint))
#define PBT_LEFT(p) (p+sizeof(ajuint)+sizeof(ajulong)+sizeof(ajuint)    \
                     +sizeof(ajuint))
#define PBT_RIGHT(p) (p+sizeof(ajuint)+sizeof(ajulong)+sizeof(ajuint)   \
                      +sizeof(ajuint)+sizeof(ajulong))
#define PBT_OVERFLOW(p) (p+sizeof(ajuint)+sizeof(ajulong)+sizeof(ajuint) \
                         +sizeof(ajuint)+sizeof(ajulong)+sizeof(ajulong))
#define PBT_PREV(p) (p+sizeof(ajuint)+sizeof(ajulong)+sizeof(ajuint)    \
                     +sizeof(ajuint)+sizeof(ajulong)+sizeof(ajulong)    \
                     +sizeof(ajulong))
#define PBT_KEYLEN(p) (p+sizeof(ajuint)+sizeof(ajulong)+sizeof(ajuint)  \
                       +sizeof(ajuint)+sizeof(ajulong)+sizeof(ajulong)  \
                       +sizeof(ajulong)+sizeof(ajulong))

/*
** Macros to return a page entry value within an internal/leaf node
*/

#if !defined(WORDS_BIGENDIAN)
#define GBT_NODETYPE(p,v) (memcpy((void*)v,(const void*)PBT_NODETYPE(p), \
                                  sizeof(ajuint)))
#define GBT_BLOCKNUMBER(p,v) (memcpy((void*)v,(const void*)PBT_BLOCKNUMBER(p), \
                                     sizeof(ajulong)))
#define GBT_NKEYS(p,v) (memcpy((void*)v,(const void*)PBT_NKEYS(p),      \
                               sizeof(ajuint)))
#define GBT_TOTLEN(p,v) (memcpy((void*)v,(const void*)PBT_TOTLEN(p),    \
                                sizeof(ajuint)))
#define GBT_LEFT(p,v) (memcpy((void*)v,(const void*)PBT_LEFT(p),        \
                              sizeof(ajulong)))
#define GBT_RIGHT(p,v) (memcpy((void*)v,(const void*)PBT_RIGHT(p),      \
                               sizeof(ajulong)))
#define GBT_PREV(p,v) (memcpy((void*)v,(const void*)PBT_PREV(p),        \
                              sizeof(ajulong)))
#define GBT_OVERFLOW(p,v) (memcpy((void*)v,(const void*)PBT_OVERFLOW(p), \
                                  sizeof(ajulong)))
#else
#define GBT_NODETYPE(p,v) memcpy((void*)v,(const void*)PBT_NODETYPE(p), \
                                 sizeof(ajuint));                       \
    ajByteRevUint(v)
#define GBT_BLOCKNUMBER(p,v) memcpy((void*)v,(const void*)PBT_BLOCKNUMBER(p), \
                                    sizeof(ajulong));                   \
    ajByteRevUlong(v)
#define GBT_NKEYS(p,v) memcpy((void*)v,(const void*)PBT_NKEYS(p),       \
                              sizeof(ajuint));                          \
    ajByteRevUint(v)
#define GBT_TOTLEN(p,v) memcpy((void*)v,(const void*)PBT_TOTLEN(p),     \
                               sizeof(ajuint));                         \
    ajByteRevUint(v)
#define GBT_LEFT(p,v) memcpy((void*)v,(const void*)PBT_LEFT(p), \
                             sizeof(ajulong));                  \
    ajByteRevUlong(v)

#define GBT_RIGHT(p,v) memcpy((void*)v,(const void*)PBT_RIGHT(p),       \
                              sizeof(ajulong));                         \
    ajByteRevUlong(v)
#define GBT_PREV(p,v) memcpy((void*)v,(const void*)PBT_PREV(p), \
                             sizeof(ajulong));                  \
    ajByteRevUlong(v)
#define GBT_OVERFLOW(p,v) memcpy((void*)v,(const void*)PBT_OVERFLOW(p), \
                                 sizeof(ajulong));                      \
    ajByteRevUlong(v)
#endif


/*
** Macros to set a page entry value within an internal/leaf node
*/

#if !defined(WORDS_BIGENDIAN)
#define SBT_NODETYPE(p,v) (memcpy((void*)PBT_NODETYPE(p),(const void*)&v, \
                                  sizeof(ajuint)))
#define SBT_BLOCKNUMBER(p,v) (memcpy((void*)PBT_BLOCKNUMBER(p),         \
                                     (const void*)&v,sizeof(ajulong)))
#define SBT_NKEYS(p,v) (memcpy((void*)PBT_NKEYS(p),(const void*)&v,     \
                               sizeof(ajuint)))
#define SBT_TOTLEN(p,v) (memcpy((void*)PBT_TOTLEN(p),(const void*)&v,   \
                                sizeof(ajuint)))
#define SBT_LEFT(p,v) (memcpy((void*)PBT_LEFT(p),               \
                              (const void*)&v,sizeof(ajulong)))
#define SBT_RIGHT(p,v) (memcpy((void*)PBT_RIGHT(p),                     \
                               (const void*)&v,sizeof(ajulong)))
#define SBT_PREV(p,v) (memcpy((void*)PBT_PREV(p),               \
                              (const void*)&v,sizeof(ajulong)))
#define SBT_OVERFLOW(p,v) (memcpy((void*)PBT_OVERFLOW(p),               \
                                  (const void*)&v,sizeof(ajulong)))
#else
#define SBT_NODETYPE(p,v) ajByteRevUint(&v);            \
    memcpy((void*)PBT_NODETYPE(p),(const void*)&v,      \
           sizeof(ajuint))
#define SBT_BLOCKNUMBER(p,v) ajByteRevUlong(&v);        \
    memcpy((void*)PBT_BLOCKNUMBER(p),                   \
           (const void*)&v,sizeof(ajulong))
#define SBT_NKEYS(p,v) ajByteRevUint(&v);       \
    memcpy((void*)PBT_NKEYS(p),(const void*)&v, \
           sizeof(ajuint))
#define SBT_TOTLEN(p,v) ajByteRevUint(&v);              \
    memcpy((void*)PBT_TOTLEN(p),(const void*)&v,        \
           sizeof(ajuint))
#define SBT_LEFT(p,v) ajByteRevUlong(&v);       \
    memcpy((void*)PBT_LEFT(p),                  \
           (const void*)&v,sizeof(ajulong))
#define SBT_RIGHT(p,v) ajByteRevUlong(&v);      \
    memcpy((void*)PBT_RIGHT(p),                 \
           (const void*)&v,sizeof(ajulong))
#define SBT_PREV(p,v) ajByteRevUlong(&v);       \
    memcpy((void*)PBT_PREV(p),                  \
           (const void*)&v,sizeof(ajulong))
#define SBT_OVERFLOW(p,v) ajByteRevUlong(&v);   \
    memcpy((void*)PBT_OVERFLOW(p),              \
           (const void*)&v,sizeof(ajulong))
#endif




/*
** Prototype definitions
*/

AjBool       ajBtreeKeyFindLen(AjPBtcache cache, const AjPStr key,
                               ajulong* treeblock);

ajuint       ajBtreeFieldGetLenC(const char* nametxt);
ajuint       ajBtreeFieldGetLenS(const AjPStr name);
const AjPStr ajBtreeFieldGetExtensionC(const char *nametxt);
const AjPStr ajBtreeFieldGetExtensionS(const AjPStr name);
AjBool       ajBtreeFieldGetSecondaryC(const char *nametxt);
AjBool       ajBtreeFieldGetSecondaryS(const AjPStr name);

ajulong      ajBtreeCacheGetPagecount(const AjPBtcache cache);
ajuint       ajBtreeCacheGetPripagesize(const AjPBtcache cache);
ajuint       ajBtreeCacheGetSecpagesize(const AjPBtcache cache);
ajulong      ajBtreeCacheGetTotsize(const AjPBtcache cache);

AjBool       ajBtreeCacheIsCompressed(const AjPBtcache cache);
AjBool       ajBtreeCacheIsSecondary(const AjPBtcache thys);

ajuint       ajBtreePageGetSize(const AjPBtpage page, ajuint refcount);
const char*  ajBtreePageGetTypename(const AjPBtpage page);
AjBool       ajBtreePageIsPrimary(const AjPBtpage page);

AjPBtcache   ajBtreeIdcacheNewC(const char *file, const char *ext,
                                const char *idir, const char *mode,
                                AjBool compressed,
                                ajuint kwlimit, ajuint refcount,
                                ajuint pripagesize, ajuint secpagesize,
                                ajuint pricachesize, ajuint seccachesize,
                                ajulong pripagecount, ajulong secpagecount,
                                ajuint order, ajuint fill, ajuint level,
                                ajuint sorder, ajuint sfill,
                                ajulong count, ajulong countall);
AjPBtcache   ajBtreeIdcacheNewS(const AjPStr file, const AjPStr ext,
                                const AjPStr idir, const char *mode,
                                AjBool compressed,
                                ajuint kwlimit, ajuint refcount,
                                ajuint pripagesize, ajuint secpagesize,
                                ajuint pricachesize, ajuint seccachesize,
                                ajulong pripagecount, ajulong secpagecount,
                                ajuint order, ajuint fill, ajuint level,
                                ajuint sorder, ajuint sfill,
                                ajulong count, ajulong countall);

AjPBtcache   ajBtreeSeccacheNewC(const char *file, const char *ext,
                                 const char *idir, const char *mode,
                                 AjBool compressed,
                                 ajuint kwlimit, ajuint idlimit,
                                 ajuint pagesize, ajuint secpagesize,
                                 ajuint cachesize, ajuint seccachesize,
                                 ajulong pagecount, ajulong secpagecount,
                                 ajuint order, ajuint fill, ajuint level,
                                 ajuint sorder, ajuint sfill,
                                 ajulong count, ajulong countall);
AjPBtcache   ajBtreeSeccacheNewS(const AjPStr file, const AjPStr ext,
                                 const AjPStr idir, const char *mode,
                                 AjBool compressed,
                                 ajuint kwlimit, ajuint idlimit,
                                 ajuint pagesize, ajuint secpagesize,
                                 ajuint cachesize, ajuint seccachesize,
                                 ajulong pagecount, ajulong secpagecount,
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

ajulong      ajBtreeCacheDel(AjPBtcache *thys);

void         ajBtreeIdDel(AjPBtId *thys);
void         ajBtreeIdDelVoid(void **voidarg);
AjPBtId      ajBtreeIdNew(ajuint refcount);
AjBool       ajBtreeIdIsMulti(const AjPBtId thys);

void         ajBtreeHitDel(AjPBtHit *thys);
void         ajBtreeHitDelVoid(void **voidarg);
AjPBtHit     ajBtreeHitNew(void);
AjPBtHit     ajBtreeHitNewId(const AjPBtId btid);

void         ajBtreeHitrefDel(AjPBtHitref *thys);
void         ajBtreeHitrefDelVoid(void **voidarg);
AjPBtHitref  ajBtreeHitrefNew(void);
AjPBtHitref  ajBtreeHitrefNewId(const AjPBtId btid);

AjBool       ajBtreeReadParamsC(const char *fn, const char *ext,
                                const char *idir,
                                AjBool *secondary, AjBool *compressed,
                                ajuint *kwlimit, ajuint *idlimit,
                                ajuint *refcount,
                                ajuint *pripagesize, ajuint *secpagesize,
                                ajuint *pricachesize, ajuint *seccachesize,
                                ajulong *pripagecount, ajulong *secpagecount,
                                ajuint *order, ajuint *nperbucket,
                                ajuint *level,
                                ajuint *sorder, ajuint *snperbucket,
                                ajulong *count, ajulong *countall);
AjBool       ajBtreeReadParamsS(const AjPStr fn, const AjPStr ext,
                                const AjPStr idir,
                                AjBool *secondary, AjBool *compressed,
                                ajuint *kwlimit, ajuint *idlimit,
                                ajuint *refcount,
                                ajuint *pripagesize, ajuint *secpagesize,
                                ajuint *pricachesize, ajuint *seccachesize,
                                ajulong *pripagecount, ajulong *secpagecount,
                                ajuint *order, ajuint *nperbucket,
                                ajuint *level,
                                ajuint *sorder, ajuint *snperbucket,
                                ajulong *count, ajulong *countall);

void         ajBtreeWriteParamsC(const AjPBtcache cache, const char *fntxt,
                                 const char *exttxt, const char *idirtxt);
void         ajBtreeWriteParamsS(const AjPBtcache cache, const AjPStr fn,
                                 const AjPStr ext, const AjPStr idir);

ajuint       ajBtreeReadEntriesC(const char *filename, const char *indexdir,
                                 const char *directory,
                                 AjPStr **seqfiles, AjPStr ***reffiles,
                                 ajuint *refcount);
ajuint       ajBtreeReadEntriesS(const AjPStr filename, const AjPStr indexdir,
                                 const AjPStr directory,
                                 AjPStr **seqfiles, AjPStr ***reffiles,
                                 ajuint *refcount);

void         ajBtreeIdentFetchId(AjPBtcache cache, const AjPStr key,
                                 AjPList idlist);
void         ajBtreeIdentFetchwildId(AjPBtcache cache,
                                     const AjPStr key, AjPList idlist);
void         ajBtreeIdentFetchHit(AjPBtcache cache, const AjPStr key,
                                  AjPList idlist);
void         ajBtreeIdentFetchwildHit(AjPBtcache cache,
                                   const AjPStr key, AjPList idlist);
void         ajBtreeIdentFetchHitref(AjPBtcache cache, const AjPStr key,
                                     AjPList idlist);
void         ajBtreeIdentFetchwildHitref(AjPBtcache cache,
                                   const AjPStr key, AjPList idlist);

void         ajBtreeKeyFetchId(AjPBtcache cache, AjPBtcache idcache,
                             const AjPStr key, AjPList btidlist);
void         ajBtreeKeyFetchwildId(AjPBtcache cache, AjPBtcache idcache,
                                 const AjPStr key, AjPList btidlist);
void         ajBtreeKeyFetchHit(AjPBtcache cache, AjPBtcache idcache,
                             const AjPStr key, AjPList btidlist);
void         ajBtreeKeyFetchwildHit(AjPBtcache cache, AjPBtcache idcache,
                                 const AjPStr key, AjPList btidlist);
void         ajBtreeKeyFetchHitref(AjPBtcache cache, AjPBtcache idcache,
                             const AjPStr key, AjPList btidlist);
void         ajBtreeKeyFetchwildHitref(AjPBtcache cache, AjPBtcache idcache,
                                 const AjPStr key, AjPList btidlist);

AjPBtIdwild  ajBtreeIdwildNew(AjPBtcache cache, const AjPStr wild);
void         ajBtreeIdwildDel(AjPBtIdwild *thys);

AjPBtId      ajBtreeIdwildQuery(AjPBtcache cache, AjPBtIdwild wild);

AjPBtKeywild ajBtreeKeywildNew(AjPBtcache cache, const AjPStr wild);
void         ajBtreeKeywildDel(AjPBtKeywild *thys);

AjPBtId      ajBtreeKeywildQuery(AjPBtcache cache, AjPBtKeywild wild,
                                 AjPBtcache idcache);

AjPBtPri     ajBtreePriNew(void);
void         ajBtreePriDel(AjPBtPri *thys);

void         ajBtreeIdentIndex(AjPBtcache cache, AjPBtId id);

AjBool       ajBtreeKeyIndex(AjPBtcache cache, const AjPStr keyword,
                             const AjPStr id);

void         ajBtreeDumpIdentifiers(AjPBtcache cache, ajuint dmin, ajuint dmax,
                                    AjPFile outf);

void         ajBtreeDumpKeywords(AjPBtcache cache,
                                 ajuint dmin, ajuint dmax, AjPFile outf);

ajint        ajBtreeIdCmp(const void *x, const void *y);
ajulong      ajBtreeIdHash(const void *x, ajulong hashsize);

ajint        ajBtreeHitCmp(const void *x, const void *y);
ajulong      ajBtreeHitHash(const void *x, ajulong hashsize);

ajint        ajBtreeHitrefCmp(const void *x, const void *y);
ajulong      ajBtreeHitrefHash(const void *x, ajulong hashsize);

void         ajBtreeCacheStatsOut(AjPFile outf, const AjPBtcache cache,
                                  ajulong *Ppricache, ajulong *Pseccache,
                                  ajulong* Pprireads, ajulong* Psecreads,
                                  ajulong* Ppriwrites, ajulong* Psecwrites,
                                  ajulong *Pprisize, ajulong *Psecsize);
void         ajBtreeStatsOut(AjPFile outf,
                             ajulong* Psplitrootid,  ajulong* Psplitrootnum,
                             ajulong* Psplitrootkey, ajulong* Psplitrootsec,
                             ajulong* Psplitleafid,  ajulong* Psplitleafnum,
                             ajulong* Psplitleafkey, ajulong* Psplitleafsec,
                             ajulong* Preorderid,    ajulong* Preordernum,
                             ajulong* Preorderkey,   ajulong* Preordersec);

AjBool       ajBtreeStatIdbucket(AjPBtcache cache,
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
void         ajBtreeExit(void);

/*
** future functions for index updating
*/

AjBool       ajBtreeReplaceId(AjPBtcache cache, const AjPBtId rid);
AjBool       ajBtreeDeleteIdent(AjPBtcache cache, const AjPBtId btid);
AjBool       ajBtreeDeletePriId(AjPBtcache cache, const AjPBtPri pri);




/*
** End of prototype definitions
*/


#if 0
AjPBtHybrid  ajBtreeHybNew(ajuint refcount);
void         ajBtreeHybDel(AjPBtHybrid *thys);
#endif



#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif /* AJ_COMPILE_DEPRECATED_BOOK */

#ifdef AJ_COMPILE_DEPRECATED

__deprecated void     ajBtreeWriteParams(const AjPBtcache cache, const char *fn,
                                         const char *ext, const char *idir);

#endif /* AJ_COMPILE_DEPRECATED */




AJ_END_DECLS

#endif /* !AJINDEX_H */

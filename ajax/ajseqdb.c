/******************************************************************************
** @source AJAX SEQDB (database) functions
**
** These functions control all aspects of AJAX sequence database access
**
** @author Copyright (C) 1998 Peter Rice
** @author Copyright (C) 2005 Alan Bleasby
** @version 2.0
** @modified Jun 25 pmr First version
** @modified Apr 2005 ajb B+tree code addition
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA  02111-1307, USA.
******************************************************************************/

#include "ajax.h"

#include "limits.h"
#include <stdarg.h>
#include <sys/types.h>
#ifndef WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <dirent.h>
#include <unistd.h>
#else
#include <winsock2.h>
#include <io.h>
#include <fcntl.h>
extern int _open_osfhandle();
#endif
#include <errno.h>
#include <signal.h>




static AjBool seqCdReverse = AJFALSE;

static AjPRegexp seqBlastDivExp = NULL;
static AjPRegexp seqCdDivExp = NULL;
static AjPRegexp seqRegHttpProxy = NULL;
static AjPRegexp seqRegHttpUrl = NULL;

static AjPRegexp seqRegGcgId = NULL;
static AjPRegexp seqRegGcgCont = NULL;
static AjPRegexp seqRegGcgId2 = NULL;
static AjPRegexp seqRegGcgSplit = NULL;

static AjPRegexp seqRegEntrezCount = NULL;
static AjPRegexp seqRegEntrezId = NULL;

static AjPRegexp seqRegGcgRefId = NULL;
static AjPRegexp seqRegGi = NULL;

static char* seqCdName = NULL;
static ajuint seqCdMaxNameSize = 0;


/* @datastatic SeqPCdDiv ******************************************************
**
** EMBLCD division file record structure
**
** @alias SeqSCdDiv
** @alias SeqOCdDiv
**
** @attr FileName [AjPStr] Filename(s)
** @attr DivCode [ajuint] Division code
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct SeqSCdDiv
{
    AjPStr FileName;
    ajuint DivCode;
    char Padding[4];
} SeqOCdDiv;

#define SeqPCdDiv SeqOCdDiv*




/* @datastatic SeqPCdEntry ****************************************************
**
** EMBLCD entrynam.idx file record structure
**
** @alias SeqSCdEntry
** @alias SeqOCdEntry
**
** @attr div [ajuint] division file record
** @attr annoff [ajuint] data file offset
** @attr seqoff [ajuint] sequence file offset (if any)
** @@
******************************************************************************/

typedef struct SeqSCdEntry
{
    ajuint div;
    ajuint annoff;
    ajuint seqoff;
} SeqOCdEntry;

#define SeqPCdEntry SeqOCdEntry*




/* @datastatic SeqPCdFHeader **************************************************
**
** EMBLCD index file header structure, same for all index files.
**
** @alias SeqSCdFHeader
** @alias SeqOCdFHeader
**
** @attr FileSize [ajuint] Index file size
** @attr NRecords [ajuint] Index record count
** @attr IdSize [ajuint] Index string length
** @attr RelDay [ajuint] Release date - day
** @attr RelMonth [ajuint] Release date - month
** @attr RelYear [ajuint] Release date - year
** @attr RecSize [short] Record size
** @attr SPadding [short] Padding to alignment boundary
** @attr DbName [char[24]] Database name
** @attr Release [char[12]] Release name/number
** @attr Date [char[4]] Date as three integers.
** @@
******************************************************************************/

typedef struct SeqSCdFHeader
{
    ajuint FileSize;
    ajuint NRecords;
    ajuint IdSize;
    ajuint RelDay;
    ajuint RelMonth;
    ajuint RelYear;
    short RecSize;
    short SPadding;
    char DbName[24];
    char Release[12];
    char Date[4];
} SeqOCdFHeader;

#define SeqPCdFHeader SeqOCdFHeader*




/* @datastatic SeqPCdFile *****************************************************
**
** EMBLCD file data structure
**
** @alias SeqSCdFile
** @alias SeqOCdFile
**
** @attr Header [SeqPCdFHeader] Header data
** @attr File [AjPFile] File
** @attr NRecords [ajuint] Number of records
** @attr RecSize [ajuint] Record length (for calculating record offsets)
** @@
******************************************************************************/

typedef struct SeqSCdFile
{
    SeqPCdFHeader Header;
    AjPFile File;
    ajuint NRecords;
    ajuint RecSize;
} SeqOCdFile;

#define SeqPCdFile SeqOCdFile*




/* @datastatic SeqPCdHit ******************************************************
**
** EMBLCD hit file record structure
**
** @alias SeqSCdHit
** @alias SeqOCdHit
**
** @attr HitList [ajuint*] Array of hits, as record numbers in the
**                         entrynam file
** @attr NHits [ajuint] Number of hits in HitList array
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct SeqSCdHit
{
    ajuint* HitList;
    ajuint NHits;
    char Padding[4];
} SeqOCdHit;

#define SeqPCdHit SeqOCdHit*




/* @datastatic SeqPCdIdx ******************************************************
**
** EMBLCD entryname index file record structure
**
** @alias SeqSCdIdx
** @alias SeqOCdIdx
**
** @attr AnnOffset [ajuint] Data file offset (see DivCode)
** @attr SeqOffset [ajuint] Sequence file offset (if any) (see DivCode)
** @attr EntryName [AjPStr] Entry ID - the file is sorted by these
** @attr DivCode [short] Division file record
** @attr Padding [char[6]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct SeqSCdIdx
{
    ajuint AnnOffset;
    ajuint SeqOffset;
    AjPStr EntryName;
    short DivCode;
    char Padding[6];
} SeqOCdIdx;

#define SeqPCdIdx SeqOCdIdx*




/* @datastatic SeqPCdTrg ******************************************************
**
** EMBLCD target (,trg) file record structure
**
** @alias SeqSCdTrg
** @alias SeqOCdTrg
**
** @attr FirstHit [ajuint] First hit record in .hit file
** @attr NHits [ajuint] Number of hit records in .hit file
** @attr Target [AjPStr] Indexed target string (the file is sorted by these)
** @@
******************************************************************************/

typedef struct SeqSCdTrg
{
    ajuint FirstHit;
    ajuint NHits;
    AjPStr Target;
} SeqOCdTrg;

#define SeqPCdTrg SeqOCdTrg*




/* @datastatic SeqPCdQry ******************************************************
**
** EMBLCD query structure
**
** @alias SeqSCdQry
** @alias SeqOCdQry
**
** @attr divfile [AjPStr] division.lkp
** @attr idxfile [AjPStr] entryname.idx
** @attr datfile [AjPStr] main data reference
** @attr seqfile [AjPStr] sequence
** @attr tblfile [AjPStr] BLAST table
** @attr srcfile [AjPStr] BLAST FASTA source data
** @attr dfp [SeqPCdFile] division.lkp
** @attr ifp [SeqPCdFile] entryname.idx
** @attr trgfp [SeqPCdFile] acnum.trg
** @attr hitfp [SeqPCdFile] acnum.hit
** @attr trgLine [SeqPCdTrg]acnum input line
** @attr name [char*] filename from division.lkp
** @attr nameSize [ajuint] division.lkp filename length
** @attr div [ajuint] current division number
** @attr maxdiv [ajuint] max division number
** @attr type [ajuint] BLAST type
** @attr libr [AjPFile] main data reference or BLAST header
** @attr libs [AjPFile] sequence or BLAST compressed sequence
** @attr libt [AjPFile] blast table
** @attr libf [AjPFile] blast FASTA source data
** @attr idnum [ajuint] current BLAST entry offset
** @attr TopHdr [ajuint] BLAST table headers offset
** @attr TopCmp [ajuint] BLAST table sequence offset
** @attr TopAmb [ajuint] BLAST table ambiguities offset
** @attr TopSrc [ajuint] BLAST table FASTA source offset
** @attr Size [ajuint] BLAST database size
** @attr List [AjPList] list of entries
** @attr Skip [AjBool*] skip file(s) in division.lkp
** @attr idxLine [SeqPCdIdx] entryname.idx input line
** @attr Samefile [AjBool] true if the same file is passed to ajFileBuffSetFile
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct SeqSCdQry
{
    AjPStr divfile;
    AjPStr idxfile;
    AjPStr datfile;
    AjPStr seqfile;
    AjPStr tblfile;
    AjPStr srcfile;

    SeqPCdFile dfp;
    SeqPCdFile ifp;
    SeqPCdFile trgfp;
    SeqPCdFile hitfp;
    SeqPCdTrg trgLine;

    char* name;
    ajuint nameSize;
    ajuint div;
    ajuint maxdiv;

    ajuint type;

    AjPFile libr;
    AjPFile libs;
    AjPFile libt;
    AjPFile libf;

    ajuint idnum;
    ajuint TopHdr;
    ajuint TopCmp;
    ajuint TopAmb;
    ajuint TopSrc;
    ajuint Size;

    AjPList List;
    AjBool* Skip;
    SeqPCdIdx idxLine;
    AjBool Samefile;
    char Padding[4];
} SeqOCdQry;

#define SeqPCdQry SeqOCdQry*




/* @datastatic SeqPEmbossQry *************************************************
**
** Btree 'emboss' query structure
**
** @alias SeqSEmbossQry
** @alias SeqOEmbossQry
**
** @attr do_id  [AjBool] If true, test ID
** @attr do_ac  [AjBool] If true, test accession
** @attr do_kw  [AjBool] If true, test keyword
** @attr do_de  [AjBool] If true, test description
** @attr do_sv  [AjBool] If true, test seqversion
** @attr do_tx  [AjBool] If true, test text
** @attr idcache [AjPBtcache] ID cache
** @attr accache [AjPBtcache] accession cache
** @attr svcache [AjPBtcache] seqversion cache
** @attr kwcache [AjPBtcache] keyword cache
** @attr txcache [AjPBtcache] text cache
** @attr decache [AjPBtcache] description cache
** @attr files [AjPStr*] database filenames
** @attr reffiles [AjPStr*] database reference filenames
** @attr div [ajuint] division number of currently open database file
** @attr nentries [ajint] number of entries in the filename array(s)
**                        -1 when done
** @attr libs [AjPFile] Primary (database source) file
** @attr libr [AjPFile] Secondary (database bibliographic source) file
** @attr List [AjPList] List of files
** @attr Skip [AjBool*] files numbers to exclude
** @attr Samefile [AjBool] true if the same file is passed to ajFileBuffSetFile
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct SeqSEmbossQry
{
    AjBool do_id;
    AjBool do_ac;
    AjBool do_kw;
    AjBool do_de;
    AjBool do_sv;
    AjBool do_tx;

    AjPBtcache idcache;
    AjPBtcache accache;
    AjPBtcache svcache;
    AjPBtcache kwcache;
    AjPBtcache txcache;
    AjPBtcache decache;

    AjPStr *files;
    AjPStr *reffiles;

    ajuint div;
    ajint nentries;
    
    AjPFile libs;
    AjPFile libr;
    
    AjPList List;
    AjBool *Skip;
    AjBool Samefile;
    char Padding[4];
} SeqOEmbossQry;

#define SeqPEmbossQry SeqOEmbossQry*





static AjBool     seqAccessApp(AjPSeqin seqin);
static AjBool     seqAccessBlast(AjPSeqin seqin);
/* static AjBool     seqAccessCmd(AjPSeqin seqin);*/ /* not implemented */
static AjBool     seqAccessDbfetch(AjPSeqin seqin);
static AjBool     seqAccessDirect(AjPSeqin seqin);
static AjBool     seqAccessEmblcd(AjPSeqin seqin);
static AjBool     seqAccessEmboss(AjPSeqin seqin);
static AjBool     seqAccessEmbossGcg(AjPSeqin seqin);
static AjBool     seqAccessEntrez(AjPSeqin seqin);
static AjBool     seqAccessFreeEmblcd(void* qry);
static AjBool     seqAccessFreeEmboss(void* qry);
static AjBool     seqAccessGcg(AjPSeqin seqin);
static AjBool     seqAccessMrs(AjPSeqin seqin);
static AjBool     seqAccessMrs3(AjPSeqin seqin);
/* static AjBool     seqAccessNbrf(AjPSeqin seqin); */ /* obsolete */
static AjBool     seqAccessSeqhound(AjPSeqin seqin);
static AjBool     seqAccessSrs(AjPSeqin seqin);
static AjBool     seqAccessSrsfasta(AjPSeqin seqin);
static AjBool     seqAccessSrswww(AjPSeqin seqin);
static AjBool     seqAccessUrl(AjPSeqin seqin);

static AjBool     seqBlastOpen(AjPSeqQuery qry, AjBool next);
static ajuint     seqCdDivNext(AjPSeqQuery qry);
static void       seqCdIdxDel(SeqPCdIdx* pthys);
static void       seqCdTrgDel(SeqPCdTrg* pthys);
static AjBool     seqBlastAll(AjPSeqin seqin);
static AjPFile    seqBlastFileOpen(const AjPStr dir, const AjPStr name);
static AjBool     seqBlastLoadBuff(AjPSeqin seqin);
static AjBool     seqBlastQryClose(AjPSeqQuery qry);
static AjBool     seqBlastQryNext(AjPSeqQuery qry);
static AjBool     seqBlastReadTable(AjPSeqin seqin,
				    AjPStr* hline, AjPStr* seq);
static void       seqBlastStripNcbi(AjPStr* line);

static AjBool     seqCdAll(AjPSeqin seqin);
static int        seqCdEntryCmp(const void* a, const void* b);
static void       seqCdEntryDel(void** pentry, void* cl);
static void       seqCdFileClose(SeqPCdFile *thys);
static SeqPCdFile seqCdFileOpen(const AjPStr dir, const char* name,
				AjPStr* fullname);
static size_t     seqCdFileRead(void* ptr, size_t element_size,
				SeqPCdFile thys);
static size_t     seqCdFileReadInt(ajint* i, SeqPCdFile thys);
static size_t     seqCdFileReadName(char* name, size_t namesize,
				    SeqPCdFile thys);
static size_t     seqCdFileReadShort(short* i, SeqPCdFile thys);
static size_t     seqCdFileReadUInt(ajuint* i, SeqPCdFile thys);
static ajint      seqCdFileSeek(SeqPCdFile fil, ajuint ipos);
static void       seqCdIdxLine(SeqPCdIdx idxLine,  ajuint ipos,
			       SeqPCdFile fp);
static char*      seqCdIdxName(ajuint ipos, SeqPCdFile fp);
static AjBool     seqCdIdxQuery(AjPSeqQuery qry);
static ajuint     seqCdIdxSearch(SeqPCdIdx idxLine, const AjPStr entry,
				 SeqPCdFile fp);
static AjBool     seqCdQryClose(AjPSeqQuery qry);
static AjBool     seqCdQryEntry(AjPSeqQuery qry);
static AjBool     seqCdQryFile(AjPSeqQuery qry);
static AjBool     seqCdQryOpen(AjPSeqQuery qry);
static AjBool     seqCdQryNext(AjPSeqQuery qry);
static AjBool     seqCdQryQuery(AjPSeqQuery qry);
static AjBool     seqCdQryReuse(AjPSeqQuery qry);
static AjBool     seqCdReadHeader(SeqPCdFile fp);
static AjBool     seqCdTrgClose(SeqPCdFile *trgfil, SeqPCdFile *hitfil);
static ajuint     seqCdTrgFind(AjPSeqQuery qry, const char* indexname,
			       const AjPStr qrystring);
static void       seqCdTrgLine(SeqPCdTrg trgLine, ajuint ipos,
			       SeqPCdFile fp);
static char*      seqCdTrgName(ajuint ipos, SeqPCdFile fp);
static AjBool     seqCdTrgOpen(const AjPStr dir, const char* name,
			       SeqPCdFile *trgfil, SeqPCdFile *hitfil);
static AjBool     seqCdTrgQuery(AjPSeqQuery qry);
static ajuint     seqCdTrgSearch(SeqPCdTrg trgLine, const AjPStr name,
				SeqPCdFile fp);

static AjBool     seqEmbossAll(AjPSeqin seqin);
static AjBool 	  seqEmbossGcgAll(AjPSeqin seqin);
static void       seqEmbossGcgLoadBuff(AjPSeqin seqin);
static AjBool     seqEmbossGcgReadRef(AjPSeqin seqin);
static AjBool     seqEmbossGcgReadSeq(AjPSeqin seqin);

static void	  seqEmbossOpenCache(AjPSeqQuery qry, const char *ext,
				     AjPBtcache *cache);
static void       seqEmbossOpenSecCache(AjPSeqQuery qry, const char *ext,
					AjPBtcache *cache);
static AjBool     seqEmbossQryOpen(AjPSeqQuery qry);
static AjBool     seqEmbossQryClose(AjPSeqQuery qry);
static AjBool     seqEmbossQryEntry(AjPSeqQuery qry);
static AjBool     seqEmbossQryNext(AjPSeqQuery qry);
static AjBool     seqEmbossQryQuery(AjPSeqQuery qry);
static AjBool     seqEmbossQryReuse(AjPSeqQuery qry);

static AjBool     seqEntrezQryNext(AjPSeqQuery qry, AjPSeqin seqin);
static AjBool     seqGcgAll(AjPSeqin seqin);
static void       seqGcgBinDecode(AjPStr *pthis, ajuint rdlen);
static void       seqGcgLoadBuff(AjPSeqin seqin);
static AjBool     seqGcgReadRef(AjPSeqin seqin);
static AjBool     seqGcgReadSeq(AjPSeqin seqin);
static FILE*      seqHttpGet(const AjPSeqQuery qry,
			     const AjPStr host, ajint iport, const AjPStr get);
static FILE*      seqHttpGetProxy(const AjPSeqQuery qry,
				  const AjPStr proxyname, ajint proxyport,
				  const AjPStr host, ajint iport,
				  const AjPStr get);
static AjBool     seqHttpProxy(const AjPSeqQuery qry,
			       ajint* iport, AjPStr* proxyname);
static AjBool     seqHttpUrl(const AjPSeqQuery qry,
			     ajint* iport, AjPStr* host, AjPStr* urlget);
static FILE*      seqHttpSocket(const AjPSeqQuery qry,
				const struct hostent *hp, ajint hostport,
				const AjPStr host, ajint iport,
				const AjPStr get);
static AjBool     seqHttpVersion(const AjPSeqQuery qry, AjPStr* httpver);
static AjBool     seqSeqhoundQryNext(AjPSeqQuery qry, AjPSeqin seqin);
static void       seqSocketTimeout(int sig);




/* @funclist seqAccess ********************************************************
**
** Functions to access each database or sequence access method
**
******************************************************************************/

static SeqOAccess seqAccess[] =
{
    /*Name        Alias    Entry    Query    All
         AccessFunction   FreeFunction
	 Description*/
    {"dbfetch",    AJFALSE, AJTRUE,  AJFALSE,  AJFALSE,  
	 seqAccessDbfetch, NULL,
	 "retrieve in text format from EBI dbfetch REST services"},
    {"emboss",    AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  
	 seqAccessEmboss, seqAccessFreeEmboss,
	 "dbx program indexed"},
    {"emblcd",    AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  
	 seqAccessEmblcd, seqAccessFreeEmblcd,
	 "use EMBL-CD index from dbi programs or Staden"},
    {"entrez",    AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  
	 seqAccessEntrez, NULL,
	 "use NCBI Entrez services"},
    {"seqhound",  AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  
	 seqAccessSeqhound, NULL,
	 "use BluePrint seqhound services"},
    {"mrs",       AJFALSE, AJTRUE,  AJFALSE,  AJFALSE,  
	 seqAccessMrs, NULL,
	 "retrieve in text format from CMBI MRS server (MRS2 syntax)"},
    {"mrs3",       AJFALSE, AJTRUE,  AJFALSE,  AJFALSE,  
	 seqAccessMrs3, NULL,
	 "retrieve in text format from CMBI MRS3 server"},
    {"srs",       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  
	 seqAccessSrs, NULL,
	 "retrieve in text format from a local SRS installation"},
    {"srsfasta",  AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  
	 seqAccessSrsfasta, NULL,
	 "ertrieve in FASTA format from a local SRS installation"},
    {"srswww",    AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  
	 seqAccessSrswww, NULL,
	 "retrieve in text format from an SRS webserver"},
    {"url",       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  
	 seqAccessUrl, NULL,
	 "retrieve a URL from a remote webserver"},
    {"app",       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  
	 seqAccessApp, NULL,
	 "call an external application"},
    {"external",  AJTRUE,  AJTRUE,  AJTRUE,  AJTRUE,  
	 seqAccessApp, NULL,
	 "call an external application"},
    /* {"asis",      AJFALSE, AJTRUE, AJFALSE, AJFALSE,  
	 ajSeqAccessAsis, NULL,
	 ""}, */        /* called by seqUsaProcess */
    /* {"file",      AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  
	 ajSeqAccessFile, NULL,
	 ""}, */        /* called by seqUsaProcess */
    /* {"offset",    AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  
	 ajSeqAccessOffset, NULL,
	 ""}, */    /* called by seqUsaProcess */
    {"direct",    AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  
	 seqAccessDirect, NULL,
	 "reading the original files unindexed"},
    {"gcg",       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  
	 seqAccessGcg, NULL,
	 "emboss dbigcg indexed"},
    {"embossgcg", AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  
	 seqAccessEmbossGcg, NULL,
	 "emboss dbxgcg indexed"},
    {"blast",     AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  
	 seqAccessBlast, NULL,
	 "blast database format version 2 or 3"},
    {NULL, AJFALSE, AJFALSE, AJFALSE, AJFALSE, NULL, NULL, NULL},
/* after the NULL access method, and so unreachable.
** seqhound requires a username and password which it prompts for
** interactively
*/
};

static char aa_btoa[27] = {"-ARNDCQEGHILKMFPSTWYVBZX*"};
static char aa_btoa2[27]= {"-ABCDEFGHIKLMNPQRSTVWXYZ*"};




/* @func ajSeqMethodTest ******************************************************
**
** Tests for a named method for sequence reading.
**
** @param [r] method [const AjPStr] Method required.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajSeqMethodTest(const AjPStr method)
{
    ajint i;

    for(i=0; seqAccess[i].Name; i++)
	if(ajStrMatchCaseC(method, seqAccess[i].Name))
	    return ajTrue;

    return ajFalse;
}




/* @func ajSeqMethod **********************************************************
**
** Sets the access function for a named method for sequence reading.
**
** @param [r] method [const AjPStr] Method required.
** @return [SeqPAccess] Access function to use
** @category new [SeqPAccess] returns a copy of a known access
**                method definition.
** @@
******************************************************************************/

SeqPAccess ajSeqMethod(const AjPStr method)
{
    ajint i = 0;

    while(seqAccess[i].Name)
    {
	if(ajStrMatchCaseC(method, seqAccess[i].Name))
	{
	    ajDebug("Matched seqAccess[%d] '%s'\n", i, seqAccess[i].Name);
	    return &seqAccess[i];
	}
	i++;
    }

    return NULL;
}




/* @section EMBL CD Database Indexing *****************************************
**
** These functions manage the EMBL CD-ROM index access methods.
** These include the "efetch" indexing used at the Sanger Centre
** based on Erik Sonnhammer's indexseqlibs code
** and a dirct copy of the database and index files from the
** EMBL CD-RM distribution.
**
** The index files start with a file "division.lkp" which contains
** the list of database filenames and an index number for each.
**
** "entrynam.idx" is a sorted index by entry name for each entry
** which points to a file number and a byte offset within the file.
**
** "acnum.trg" and "acnum.hit" index accession numbers and link them
** to record numbers in "entrynam.idx"
**
** Other index files are not used yet by EMBOSS but could be added
** using the "des" field in queries to search descriptions, and so on.
**
******************************************************************************/




/* @funcstatic seqAccessEmblcd ************************************************
**
** Reads a sequence using EMBL CD-ROM index files.
**
** @param [u] seqin [AjPSeqin] Sequence input, including query data.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqAccessEmblcd(AjPSeqin seqin)
{
    AjBool retval = ajFalse;

    AjPSeqQuery qry;
    SeqPCdQry qryd;

    static ajint qrycalled = 0;		/*  check bigendian once */

    qry = seqin->Query;
    qryd = qry->QryData;

    ajDebug("seqAccessEmblcd type %d\n", qry->Type);

    if(qry->Type == QRY_ALL)
	return seqCdAll(seqin);

    /* we need to search the index files and return a query */

    if(!qrycalled)
    {
	if(ajUtilGetBigendian())
	    seqCdReverse = ajTrue;
	qrycalled = 1;
    }

    if(qry->QryData)
    {				        /* reuse unfinished query data */
	if(!seqCdQryReuse(qry))		/* oops, we're finished        */
	    return ajFalse;
    }
    else
    {				        /* starting out, set up query */
	seqin->Single = ajTrue;

	if(!seqCdQryOpen(qry))
	{
	    ajWarn("seqCdQry failed");
	    return ajFalse;
	}

	qryd = qry->QryData;

	/* binary search for the entryname we need */

	if(qry->Type == QRY_ENTRY)
	{
	    ajDebug("entry id: '%S' acc: '%S' gi: '%S' hasacc:%B\n",
		    qry->Id, qry->Acc, qry->Gi, qry->HasAcc);
	    if(!seqCdQryEntry(qry))
	    {
		ajDebug("EMBLCD Entry failed\n");
		if(ajStrGetLen(qry->Id))
		    ajDebug("Database Entry id-'%S' not found\n", qry->Id);
		else if(ajStrGetLen(qry->Gi))
		    ajDebug("Database Entry gi-'%S' not found\n", qry->Gi);
		else if (qry->HasAcc && ajStrGetLen(qry->Acc))
		    ajDebug("Database Entry acc-'%S' not found\n", qry->Acc);
		else
		    ajDebug("Database Entry '%S' not found\n", qry->Id);
	    }
	}

	if(qry->Type == QRY_QUERY)
	{
	    ajDebug("query id: '%S' acc: '%S' gi: '%S' hasacc:%B\n",
		    qry->Id, qry->Acc, qry->Gi, qry->HasAcc);
	    if(!seqCdQryQuery(qry))
	    {
		ajDebug("EMBLCD Query failed\n");
		if(ajStrGetLen(qry->Id))
		    ajDebug("Database Query '%S' not found\n", qry->Id);
		else if(ajStrGetLen(qry->Acc))
		    ajDebug("Database Query '%S' not found\n", qry->Acc);
		else if(ajStrGetLen(qry->Sv))
		    ajDebug("Database Query 'sv:%S' not found\n", qry->Sv);
		else if(ajStrGetLen(qry->Gi))
		    ajDebug("Database Query 'gi:%S' not found\n", qry->Gi);
		else if(ajStrGetLen(qry->Des))
		    ajDebug("Database Query 'des:%S' not found\n", qry->Des);
		else if(ajStrGetLen(qry->Key))
		    ajDebug("Database Query 'key:%S' not found\n", qry->Key);
		else if(ajStrGetLen(qry->Org))
		    ajDebug("Database Query 'org:%S' not found\n", qry->Org);
		else
		    ajDebug("Database Query '%S' not found\n", qry->Id);
	    }
	}
    }

    if(ajListGetLength(qryd->List))
    {
	retval = seqCdQryNext(qry);
	if(retval)
	    ajFileBuffSetFile(&seqin->Filebuff, qryd->libr, qryd->Samefile);
    }

    if(!ajListGetLength(qryd->List)) /* could have been emptied by code above */
    {
	seqCdQryClose(qry);
	/* AJB addition */
        /*
	 * This was for the old code where seqsets had different
         * memory handling ... and the reason for the multi
         * flag in the first place. So far this seems
         * unnecessary for the revised code but is left here
         * for a while as a reminder and 'just in case'
	 */
	if((qry->Type == QRY_ENTRY) && !seqin->multi)
	{
	    AJFREE(qry->QryData);
	    qryd = NULL;
	}
    }

    ajStrAssignS(&seqin->Db, qry->DbName);

    return retval;
}




/* @funcstatic seqAccessFreeEmblcd ********************************************
**
** Frees data specific to reading EMBL CD-ROM index files.
**
** @param [r] qry [void*] query data specific to EMBLCD
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqAccessFreeEmblcd(void* qry)
{
    SeqPCdQry qryd;
    AjPSeqQuery query;
    AjBool retval = ajTrue;

    ajDebug("seqAccessFreeEmblcd\n");

    query = (AjPSeqQuery) qry;
    qryd = query->QryData;
    qryd->libr=0;
    qryd->libs=0;

    ajListFreeData(&qryd->List);

    seqCdQryClose(query);

    return retval;
}




/* @funcstatic seqCdAll *******************************************************
**
** Reads the EMBLCD division lookup file and opens a list of all the
** database files for plain reading.
**
** @param [u] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqCdAll(AjPSeqin seqin)
{
    AjPStr divfile = NULL;
    SeqPCdFile dfp;
    AjPList list;
    AjPSeqQuery qry;

    ajint i;
    short j;
    ajint nameSize;
    char *name;
    AjPStr fullName = NULL;

    static ajint called = 0;	     /*  we test once for bigendian */

    qry = seqin->Query;

    if(!called)
    {
	if(ajUtilGetBigendian())
	    seqCdReverse = ajTrue;
	called = 1;
    }

    if(!ajStrGetLen(qry->IndexDir))
    {
	ajDebug("no indexdir defined for database %S\n", qry->DbName);
	ajErr("no indexdir defined for database %S", qry->DbName);
	return ajFalse;
    }

    ajDebug("EMBLCD All index directory '%S'\n", qry->IndexDir);

    dfp = seqCdFileOpen(qry->IndexDir, "division.lkp", &divfile);
    ajStrDel(&divfile);
    if(!dfp)
    {
	ajWarn("Cannot open division file for database '%S'",
	       qry->DbName);
	return ajFalse;
    }

    nameSize = dfp->RecSize - 2;
    name = ajCharNewRes(nameSize+1);

    list = ajListstrNew();

    seqCdFileSeek(dfp, 0);
    for(i=0; i < (ajint) dfp->Header->NRecords; i++)
    {
	seqCdFileReadShort(&j, dfp);
	seqCdFileReadName(name, nameSize, dfp);
	fullName = ajStrNewC(name);
	ajFileNameDirSet(&fullName, qry->Directory);

	/* test exclusion list and add file if OK */

	if(ajFileTestSkip(fullName, qry->Exclude, qry->Filename,
			  ajTrue, ajTrue))
	{
	    ajDebug("qrybufflist add '%S'\n", fullName);
	    ajListstrPushAppend(list, fullName);
	    fullName = NULL;
	}
	else
	{
	    ajDebug("qrybufflist *delete* '%S'\n", fullName);
	    ajStrDel(&fullName);
	}
    }
    ajFileBuffDel(&seqin->Filebuff);
    seqin->Filebuff = ajFileBuffNewInList(list);
    fullName = NULL;

    ajStrAssignS(&seqin->Db, qry->DbName);

    seqCdFileClose(&dfp);
    ajStrDelStatic(&divfile);
    ajCharDel(&name);

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    return ajTrue;
}




/* @funcstatic seqBlastFileOpen ***********************************************
**
** Opens a named BLAST index file.
**
** @param [r] dir [const AjPStr] Directory
** @param [r] name [const AjPStr] File name.
** @return [AjPFile] file object.
** @@
******************************************************************************/

static AjPFile seqBlastFileOpen(const AjPStr dir, const AjPStr name)
{
    AjPFile thys;

    thys = ajFileNewDF(dir, name);
    if(!thys)
	return NULL;

    ajDebug("seqBlastFileOpen '%F'\n", thys);

    return thys;
}




/* @funcstatic seqCdFileOpen **************************************************
**
** Opens a named EMBL CD-ROM index file.
**
** @param [r] dir [const AjPStr] Directory
** @param [r] name [const char*] File name.
** @param [w] fullname [AjPStr*] Full file name with directory path
** @return [SeqPCdFile] EMBL CD-ROM index file object.
** @@
******************************************************************************/

static SeqPCdFile seqCdFileOpen(const AjPStr dir, const char* name,
				AjPStr* fullname)
{
    SeqPCdFile thys = NULL;


    AJNEW0(thys);

    thys->File = ajFileNewDC(dir, name);

    if(!thys->File)
    {
	AJFREE(thys);
	return NULL;
    }


    AJNEW0(thys->Header);

    seqCdReadHeader(thys);
    thys->NRecords = thys->Header->NRecords;
    thys->RecSize = thys->Header->RecSize;

    ajStrAssignS(fullname, ajFileGetName(thys->File));

    ajDebug("seqCdFileOpen '%F' NRecords: %d RecSize: %d\n",
	    thys->File, thys->NRecords, thys->RecSize);


    return thys;
}




/* @funcstatic seqCdFileSeek **************************************************
**
** Sets the file position in an EMBL CD-ROM index file.
**
** @param [u] fil [SeqPCdFile] EMBL CD-ROM index file object.
** @param [r] ipos [ajuint] Offset.
** @return [ajint] Return value from the seek operation.
** @@
******************************************************************************/


static ajint seqCdFileSeek(SeqPCdFile fil, ajuint ipos)
{
    ajint ret;
    ajuint jpos;

    jpos = 300 + ipos*fil->RecSize;
    ret = ajFileSeek(fil->File, jpos, 0);

    /*
       ajDebug("seqCdFileSeek rec %u pos %u tell %Ld returns %d\n",
       ipos, jpos, ajFileTell(fil->File), ret);
    */

    return ret;
}




/* @funcstatic seqCdFileRead **************************************************
**
** Reads a specified number of bytes from an EMBL CD-ROM index file.
**
** @param [w] ptr [void*] Buffer to read into.
** @param [r] element_size [size_t] Number of bytes to read.
** @param [u] thys [SeqPCdFile] EMBL CD-ROM index file.
** @return [size_t] Number of bytes read.
** @@
******************************************************************************/

static size_t seqCdFileRead(void* ptr, size_t element_size,
			    SeqPCdFile thys)
{
    return ajFileRead(ptr, element_size, 1, thys->File);
}




/* @funcstatic seqCdFileReadName **********************************************
**
** Reads a character string from an EMBL CD-ROM index file. Trailing spaces
** (if any) are truncated. EMBLCD indices normally have a trailing NULL
** character.
**
** @param [w] name [char*] Buffer to read into. Must be at least namesize
**                         bytes in size.
** @param [r] namesize [size_t] Number of bytes to read from index file.
** @param [u] thys [SeqPCdFile] EMBL CD-ROM index file.
** @return [size_t] Number of bytes read.
** @@
******************************************************************************/

static size_t seqCdFileReadName(char* name, size_t namesize,
				SeqPCdFile thys)
{
    size_t ret;
    char* sp;

    /* ajDebug("seqCdFileReadName pos %Ld\n", ajFileTell(thys->File)); */
    ret =  ajFileRead(name, namesize, 1, thys->File);

    /* ajDebug("seqCdFileReadName was '%s' ret %d\n", name, ret); */

    name[namesize] = '\0';
    sp = &name[strlen(name)];
    while(sp > name)
    {
        sp--;
	if(*sp != ' ')
	    break;
	*sp = '\0';
    }

    /* ajDebug("seqCdFileReadName now '%s'\n", name); */
    return ret;
}




/* @funcstatic seqCdFileReadInt ***********************************************
**
** Reads a 4 byte integer from an EMBL CD-ROM index file. If the byte
** order in the index file does not match the current system the bytes
** are reversed automatically.
**
** @param [w] i [ajint*] Integer read from file.
** @param [u] thys [SeqPCdFile] EMBL CR-ROM index file.
** @return [size_t] Number of bytes read.
** @@
******************************************************************************/

static size_t seqCdFileReadInt(ajint* i, SeqPCdFile thys)
{
    size_t ret;

    ret = ajFileRead(i, 4, 1, thys->File);

    if(seqCdReverse)
	ajByteRevLen4(i);

    return ret;
}




/* @funcstatic seqCdFileReadUInt **********************************************
**
** Reads a 4 byte unsigned integer from an EMBL CD-ROM index file. If the byte
** order in the index file does not match the current system the bytes
** are reversed automatically.
**
** @param [w] i [ajuint*] Integer read from file.
** @param [u] thys [SeqPCdFile] EMBL CR-ROM index file.
** @return [size_t] Number of bytes read.
** @@
******************************************************************************/

static size_t seqCdFileReadUInt(ajuint* i, SeqPCdFile thys)
{
    size_t ret;

    ret = ajFileRead(i, 4, 1, thys->File);

    if(seqCdReverse)
	ajByteRevLen4((ajint*)i);

    return ret;
}




/* @funcstatic seqCdFileReadShort *********************************************
**
** Reads a 2 byte integer from an EMBL CD-ROM index file. If the byte
** order in the index file does not match the current system the bytes
** are reversed automatically.
**
** @param [w] i [short*] Integer read from file.
** @param [u] thys [SeqPCdFile] EMBL CR-ROM index file.
** @return [size_t] Number of bytes read.
** @@
******************************************************************************/

static size_t seqCdFileReadShort(short* i, SeqPCdFile thys)
{
    size_t ret;

    ret = ajFileRead(i, 2, 1, thys->File);

    if(seqCdReverse)
	ajByteRevLen2(i);

    return ret;
}




/* @funcstatic seqCdFileClose *************************************************
**
** Closes an EMBL CD-ROM index file.
**
** @param [d] pthis [SeqPCdFile*] EMBL CD-ROM index file.
** @return [void]
** @@
******************************************************************************/

static void seqCdFileClose(SeqPCdFile* pthis)
{
    SeqPCdFile thys;

    thys = *pthis;
    if(!thys) return;

    ajDebug("seqCdFileClose of %F\n", (*pthis)->File);

    ajFileClose(&thys->File);
    AJFREE(thys->Header);
    AJFREE(*pthis);

    return;
}




/* @funcstatic seqCdIdxSearch *************************************************
**
** Binary search through an EMBL CD-ROM index file for an exact match.
**
** @param [u] idxLine [SeqPCdIdx] Index file record.
** @param [r] entry [const AjPStr] Entry name to search for.
** @param [u] fil [SeqPCdFile] EMBL CD-ROM index file.
** @return [ajuint] Record number on success, -1 on failure.
** @@
******************************************************************************/

static ajuint seqCdIdxSearch(SeqPCdIdx idxLine, const AjPStr entry,
			    SeqPCdFile fil)
{
    AjPStr entrystr = NULL;
    ajint ihi;
    ajint ilo;
    ajint ipos = 0;
    ajint icmp = 0;
    char *name;

    ajStrAssignS(&entrystr, entry);
    ajStrFmtUpper(&entrystr);

    ajDebug("seqCdIdxSearch (entry '%S') records: %d\n",
	    entrystr, fil->NRecords);

    if(fil->NRecords < 1)
	return -1;

    ilo = 0;
    ihi = fil->NRecords - 1;
    while(ilo <= ihi)
    {
	ipos = (ilo + ihi)/2;
	name = seqCdIdxName(ipos, fil);
	icmp = ajStrCmpC(entrystr, name);
	ajDebug("idx test %u '%s' %2d (+/- %u)\n", ipos, name, icmp, ihi-ilo);
	if(!icmp) break;
	if(icmp < 0)
	    ihi = ipos-1;
	else
	    ilo = ipos+1;
    }

    ajStrDel(&entrystr);

    if(icmp)
	return -1;

    seqCdIdxLine(idxLine, ipos, fil);

    return ipos;
}




/* @funcstatic seqCdIdxQuery **************************************************
**
** Binary search of an EMBL CD-ROM index file for entries matching a
** wildcard entry name.
**
** @param [u] qry [AjPSeqQuery] Sequence query object.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqCdIdxQuery(AjPSeqQuery qry)
{
    SeqPCdQry qryd;

    AjPList list;
    SeqPCdIdx idxLine;
    AjPStr idname;
    SeqPCdFile fil;

    AjPStr idstr  = NULL;
    AjPStr idpref = NULL;
    ajint ihi;
    ajint ilo;
    ajint ipos = 0;
    ajint icmp;
    char *name;
    ajint i;
    ajint ilen;
    ajint jlo;
    ajint jhi;
    ajint khi;
    AjBool first;
    ajint ifail = 0;
    ajint iskip = 0;

    SeqPCdEntry entry;

    qryd    = qry->QryData;
    list    = qryd->List;
    idxLine = qryd->idxLine;
    idname  = qry->Id;
    fil     = qryd->ifp;

    ajStrAssignS(&idstr,idname);
    ajStrFmtUpper(&idstr);
    ajStrAssignS(&idpref, idstr);

    ajStrRemoveWild(&idpref);

    ajDebug("seqCdIdxQuery (wild '%S' prefix '%S')\n",
	    idstr, idpref);

    jlo = ilo = 0;
    khi = jhi = ihi = fil->NRecords-1;

    ilen = ajStrGetLen(idpref);
    first = ajTrue;
    if(ilen)
    {			       /* find first entry with this prefix */
	while(ilo <= ihi)
	{
	    ipos = (ilo + ihi)/2;
	    name = seqCdIdxName(ipos, fil);
	    name[ilen] = '\0';
	    icmp = ajStrCmpC(idpref, name); /* test prefix */
	    ajDebug("idx test %d '%s' %2d (+/- %d)\n",
		    ipos, name, icmp, ihi-ilo);
	    if(!icmp)
	    {			     /* hit prefix - test for first */
		ajDebug("idx hit %d\n", ipos);
		if(first)
		{
		    jhi = ihi;
		    first = ajFalse;
		    khi = ipos;
		}
		jlo = ipos;
	    }

	    if(icmp > 0)
		ilo = ipos+1;
	    else
		ihi = ipos-1;
	}

	if(first)
	{			  /* failed to find any with prefix */
	    ajStrDel(&idstr);
	    ajStrDel(&idpref);
	    return ajFalse;
	}

	ajDebug("first pass: ipos %d jlo %d jhi %d\n", ipos, jlo, jhi);

	/* now search below for last */

	ilo = jlo+1;
	ihi = jhi;
	while(ilo <= ihi)
	{
	    ipos = (ilo + ihi)/2;
	    name = seqCdIdxName(ipos, fil);
	    name[ilen] = '\0';
	    icmp = ajStrCmpC(idpref, name);
	    ajDebug("idx test %d '%s' %2d (+/- %d)\n",
		    ipos, name, icmp, ihi-ilo);
	    if(!icmp)
	    {				/* hit prefix */
		ajDebug("idx hit %d\n", ipos);
		khi = ipos;
	    }

	    if(icmp < 0)
		ihi = ipos-1;
	    else
		ilo = ipos+1;
	}
	ajDebug("second pass: ipos %d jlo %d khi %d\n",
		ipos, jlo, khi);

	name = seqCdIdxName(jlo, fil);
	ajDebug("first  %d '%s'\n", jlo, name);
	name = seqCdIdxName(khi, fil);
	ajDebug(" last  %d '%s'\n", khi, name);
    }

    for(i=jlo; i < (khi+1); i++)
    {
	seqCdIdxLine(idxLine, i, fil);
	if(ajStrMatchWildS(idxLine->EntryName, idstr))
	{
	    if(!qryd->Skip[idxLine->DivCode-1])
	    {
		if(ifail)
		{
		    ajDebug("FAIL: %d entries\n", ifail);
		    ifail=0;
		}
		if(iskip)
		{
		    ajDebug("SKIP: %d entries\n", iskip);
		    iskip=0;
		}
		ajDebug("  OK: '%S'\n", idxLine->EntryName);
		AJNEW0(entry);
		entry->div = idxLine->DivCode;
		entry->annoff = idxLine->AnnOffset;
		entry->seqoff = idxLine->SeqOffset;
		ajListPushAppend(list, (void*)entry);
	    }
	    else
	    {
		ajDebug("SKIP: '%S' [file %d]\n",
			idxLine->EntryName, idxLine->DivCode);
		iskip++;
	    }
	}
	else
	{
	    ++ifail;
	    /* ajDebug("FAIL: '%S' '%S'\n", idxLine->EntryName, idstr);*/
	}
    }

    if(ifail)
    {
	ajDebug("FAIL: %d entries\n", ifail);
	ifail=0;
    }
    if(iskip)
    {
	ajDebug("SKIP: %d entries\n", iskip);
	ifail=0;
    }

    ajStrDel(&idstr);
    ajStrDel(&idpref);

    if(ajListGetLength(list))
	return ajTrue;

    return ajFalse;
}




/* @funcstatic seqCdTrgSearch *************************************************
**
** Binary search of EMBL CD-ROM target file, for example an accession number
** search.
**
** @param [u] trgLine [SeqPCdTrg] Target file record.
** @param [r] entry [const AjPStr] Entry name or accession number.
** @param [u] fp [SeqPCdFile] EMBL CD-ROM target file
** @return [ajuint] Record number, or -1 on failure.
** @@
******************************************************************************/

static ajuint seqCdTrgSearch(SeqPCdTrg trgLine, const AjPStr entry,
			     SeqPCdFile fp)
{
    AjPStr entrystr = NULL;
    ajint ihi;
    ajint ilo;
    ajint ipos;
    ajint icmp;
    ajint itry;
    char *name;

    ajStrAssignS(&entrystr, entry);
    ajStrFmtUpper(&entrystr);

    if(fp->NRecords < 1)
      return -1;

    ilo  = 0;
    ihi  = fp->NRecords;
    ipos = (ilo + ihi)/2;
    icmp = -1;
    ajDebug("seqCdTrgSearch '%S' recSize: %d\n", entry, fp->RecSize);
    name = seqCdTrgName(ipos, fp);
    icmp = ajStrCmpC(entrystr, name);

    ajDebug("trg testa %d '%s' %2d (+/- %d)\n", ipos, name, icmp, ihi-ilo);

    while(icmp)
    {
	if(icmp < 0)
	    ihi = ipos;
	else
	    ilo = ipos;
	itry = (ilo + ihi)/2;
	if(itry == ipos)
	{
	    ajDebug("'%S' not found in .trg\n", entrystr);
	    ajStrDel(&entrystr);
	    return -1;
	}
	ipos = itry;
	name = seqCdTrgName(ipos, fp);
	icmp = ajStrCmpC(entrystr, name);
	ajDebug("trg testb %d '%s' %2d (+/- %d)\n",
		 ipos, name, icmp, ihi-ilo);
    }

    seqCdTrgLine(trgLine, ipos, fp);

    ajStrDel(&entrystr);

    if(!trgLine->NHits)
	return -1;

    ajDebug("found in .trg at record %d\n", ipos);


    return ipos;
}




/* @funcstatic seqCdIdxName ***************************************************
**
** Reads the name from record ipos of an EMBL CD-ROM index file.
** The name length is known from the index file object.
**
** @param [r] ipos [ajuint] Record number.
** @param [u] fil [SeqPCdFile] EMBL CD-ROM index file.
** @return [char*] Name read from file.
** @@
******************************************************************************/

static char* seqCdIdxName(ajuint ipos, SeqPCdFile fil)
{
    ajuint nameSize;

    nameSize = fil->RecSize-10;

    if(seqCdMaxNameSize < nameSize)
    {
	seqCdMaxNameSize = nameSize;
	if(seqCdName)
	    ajCharDel(&seqCdName);
	seqCdName = ajCharNewRes(seqCdMaxNameSize+1);
    }

    seqCdFileSeek(fil, ipos);
    seqCdFileReadName(seqCdName, nameSize, fil);

    return seqCdName;
}




/* @funcstatic seqCdIdxLine ***************************************************
**
** Reads a numbered record from an EMBL CD-ROM index file.
**
** @param [u] idxLine [SeqPCdIdx] Index file record.
** @param [r] ipos [ajuint] Record number.
** @param [u] fil [SeqPCdFile] EMBL CD-ROM index file.
** @return [void]
** @@
******************************************************************************/

static void seqCdIdxLine(SeqPCdIdx idxLine, ajuint ipos, SeqPCdFile fil)
{
    ajuint nameSize;

    nameSize = fil->RecSize-10;

    if(seqCdMaxNameSize < nameSize)
    {
	seqCdMaxNameSize = nameSize;
	if(seqCdName)
	    ajCharDel(&seqCdName);
	seqCdName = ajCharNewRes(seqCdMaxNameSize+1);
    }

    seqCdFileSeek(fil, ipos);
    seqCdFileReadName(seqCdName, nameSize, fil);

    ajStrAssignC(&idxLine->EntryName,seqCdName);

    seqCdFileReadUInt(&idxLine->AnnOffset, fil);
    seqCdFileReadUInt(&idxLine->SeqOffset, fil);
    seqCdFileReadShort(&idxLine->DivCode, fil);

    return;
}




/* @funcstatic seqCdTrgName ***************************************************
**
** Reads the target name from an EMBL CD-ROM index target file.
**
** @param [r] ipos [ajuint] Record number.
** @param [u] fil [SeqPCdFile] EMBL CD-ROM index target file.
** @return [char*] Name.
** @@
******************************************************************************/

static char* seqCdTrgName(ajuint ipos, SeqPCdFile fil)
{
    ajuint nameSize;
    ajint i;

    nameSize = fil->RecSize-8;

    if(seqCdMaxNameSize < nameSize)
    {
	seqCdMaxNameSize = nameSize;
	if(seqCdName)
	    ajCharDel(&seqCdName);
	seqCdName = ajCharNewRes(seqCdMaxNameSize+1);
    }

    seqCdFileSeek(fil, ipos);
    seqCdFileReadInt(&i, fil);
    seqCdFileReadInt(&i, fil);
    seqCdFileReadName(seqCdName, nameSize, fil);

    ajDebug("seqCdTrgName maxNameSize:%d nameSize:%d name '%s'\n",
	    seqCdMaxNameSize, nameSize, seqCdName);

    return seqCdName;
}




/* @funcstatic seqCdTrgLine ***************************************************
**
** Reads a line from an EMBL CD-ROM index target file.
**
** @param [w] trgLine [SeqPCdTrg] Target file record.
** @param [r] ipos [ajuint] Record number.
** @param [u] fil [SeqPCdFile] EMBL CD-ROM index target file.
** @return [void].
** @@
******************************************************************************/

static void seqCdTrgLine(SeqPCdTrg trgLine, ajuint ipos, SeqPCdFile fil)
{
    ajuint nameSize;

    nameSize = fil->RecSize-8;

    if(seqCdMaxNameSize < nameSize)
    {
	seqCdMaxNameSize = nameSize;
	if(seqCdName)
	    ajCharDel(&seqCdName);
	seqCdName = ajCharNewRes(seqCdMaxNameSize+1);
    }

    seqCdFileSeek(fil, ipos);

    seqCdFileReadUInt(&trgLine->NHits, fil);
    seqCdFileReadUInt(&trgLine->FirstHit, fil);
    seqCdFileReadName(seqCdName, nameSize, fil);

    ajStrAssignC(&trgLine->Target,seqCdName);

    ajDebug("seqCdTrgLine %d nHits %d firstHit %d target '%S'\n",
	    ipos, trgLine->NHits, trgLine->FirstHit, trgLine->Target);

    return;
}




/* @funcstatic seqCdReadHeader ************************************************
**
** Reads the header of an EMBL CD-ROM index file.
**
** @param [u] fil [SeqPCdFile] EMBL CD-ROM index file.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqCdReadHeader(SeqPCdFile fil)
{
    ajint i;

    SeqPCdFHeader header;
    char date[8]; /* seqCdFileReadName needs space for trailing null */

    header = fil->Header;

    seqCdFileReadUInt(&header->FileSize, fil);
    seqCdFileReadUInt(&header->NRecords, fil);
    seqCdFileReadShort(&header->RecSize, fil);

    header->IdSize = header->RecSize - 10;

    seqCdFileReadName(header->DbName, 20, fil);
    seqCdFileReadName(header->Release, 10, fil);

    seqCdFileReadName(date, 4, fil);
    for(i=1;i<4;i++)
	header->Date[i] = date[i];

    header->RelYear  = header->Date[1];
    header->RelMonth = header->Date[2];
    header->RelDay   = header->Date[3];

    ajDebug("seqCdReadHeader file %F\n", fil->File);
    ajDebug("  FileSize: %d NRecords: %hd recsize: %d idsize: %d\n",
	    header->FileSize, header->NRecords,
	    header->RecSize, header->IdSize);

    return ajTrue;
}




/* @funcstatic seqCdTrgOpen ***************************************************
**
** Opens an EMBL CD-ROM target file pair.
**
** @param [r] dir [const AjPStr] Directory.
** @param [r] name [const char*] File name.
** @param [w] trgfil [SeqPCdFile*] Target file.
** @param [w] hitfil [SeqPCdFile*] Hit file.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqCdTrgOpen(const AjPStr dir, const char* name,
			   SeqPCdFile* trgfil, SeqPCdFile* hitfil)
{
    AjPStr tmpname  = NULL;
    AjPStr fullname = NULL;

    ajFmtPrintS(&tmpname, "%s.trg",name);
    *trgfil = seqCdFileOpen(dir, ajStrGetPtr(tmpname), &fullname);
    ajStrDel(&tmpname);
    if(!*trgfil)
	return ajFalse;

    ajFmtPrintS(&tmpname, "%s.hit",name);
    *hitfil = seqCdFileOpen(dir, ajStrGetPtr(tmpname), &fullname);
    ajStrDel(&tmpname);
    ajStrDel(&fullname);

    if(!*hitfil)
	return ajFalse;

    return ajTrue;
}




/* @funcstatic seqCdTrgClose **************************************************
**
** Close an EMBL CD-ROM target file pair.
**
** @param [w] ptrgfil [SeqPCdFile*] Target file.
** @param [w] phitfil [SeqPCdFile*] Hit file.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqCdTrgClose(SeqPCdFile* ptrgfil, SeqPCdFile* phitfil)
{
    seqCdFileClose(ptrgfil);
    seqCdFileClose(phitfil);

    return ajTrue;
}




/*=============================================================================
** Entrez indexed database access
**===========================================================================*/

/* @section Entrez Database Indexing ****************************************
**
** These functions manage the SeqHound remote web API access methods.
**
******************************************************************************/


/* @funcstatic seqAccessEntrez ************************************************
**
** Reads sequence(s) using Entrez. Sends a query to a remote Entrez
** web server. Opens a file using the results and returns to the caller to
** read the data.
**
** @param [u] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqAccessEntrez(AjPSeqin seqin)
{
    AjPStr host      = NULL;
    AjPStr urlsearch = NULL;
    AjPStr get       = NULL;
    AjPStr proxyName = NULL;		/* host for proxy access.*/
    AjPStr httpver   = NULL;		/* HTTP version for GET */
    ajint iport;
    ajint proxyPort;
    AjPStr searchdb = NULL;
    FILE *fp;
    AjPSeqQuery qry;
    AjPStr gilist=NULL;
    AjPStr giline=NULL;
    AjPFile gifile=NULL;
    AjPStr tmpstr=NULL;
    const AjPStr cpystr=NULL;
    ajint numgi=0;
    ajint icount=0;
    AjBool ret = AJFALSE;

    if (!seqRegEntrezCount)
      seqRegEntrezCount = ajRegCompC("<Count>(\\d+)</Count>");
    if (!seqRegEntrezId)
	seqRegEntrezId = ajRegCompC("<Id>(\\d+)</Id>");
    if (!seqRegGi)
	seqRegGi = ajRegCompC("(\\d+)");

    qry = seqin->Query;

    if (!qry->QryData)
    {
	seqin->Single = ajTrue;

	iport     = 80;
	proxyPort = 0;			/* port for proxy axxess */

	if(!ajNamDbGetDbalias(qry->DbName, &searchdb))
	    ajStrAssignS(&searchdb, qry->DbName);
	ajDebug("seqAccessEntrez %S:%S\n", searchdb, qry->Id);

	/* eutils.ncbi.nlm.nih.gov official address
	   gives an error with valgrind */
	ajStrAssignC(&host, "www.ncbi.nlm.nih.gov");
	iport = 80;
	ajStrAssignC(&urlsearch, "/entrez/eutils/esearch.fcgi");

	seqHttpVersion(qry, &httpver);
	if(seqHttpProxy(qry, &proxyPort, &proxyName))
	    ajFmtPrintS(&get, "GET http://%S:%d%S?",
			host, iport, urlsearch);
	else
	    ajFmtPrintS(&get, "GET %S?", urlsearch);
	ajStrDel(&urlsearch);

	ajStrAppendC(&get, "tool=emboss&email=pmr@ebi.ac.uk&retmax=1000");
	if(ajStrPrefixCaseC(qry->DbType, "N"))
	    ajStrAppendC(&get, "&db=nucleotide");
	else
	    ajStrAppendC(&get, "&db=protein");

	if(ajStrGetLen(qry->Id))
	{
	    ajFmtPrintAppS(&get, "&term=%S",
			   qry->Id);
	}
	else if(qry->HasAcc && ajStrGetLen(qry->Acc))
	    ajFmtPrintAppS(&get, "&term=%S[accn]",
			   qry->Acc);
	else if(ajStrGetLen(qry->Gi))
	    ajStrAssignS(&gilist,qry->Gi);
	else if(ajStrGetLen(qry->Sv))
	{
	    cpystr = ajSeqtestIsSeqversion(qry->Sv);
	    if (cpystr)
		ajFmtPrintAppS(&get,"&term=%S[accn]",
			       cpystr);
	    else
		ajWarn("Entrez invalid Seqversion '%S'", qry->Sv);
	    cpystr = NULL;
	}
	else if(ajStrGetLen(qry->Des))
	{
	    ajStrAssignS(&tmpstr, qry->Des);
	    ajStrExchangeKK(&tmpstr, ' ', '+');
	    ajFmtPrintAppS(&get, "&term=%S[titl]",
			   tmpstr);
	    ajStrDel(&tmpstr);
	}
	else if(ajStrGetLen(qry->Org))
	{
	    ajStrAssignS(&tmpstr, qry->Org);
	    ajStrExchangeKK(&tmpstr, ' ', '+');
	    ajFmtPrintAppS(&get, "&term=%S[orgn]",
			   tmpstr);
	    ajStrDel(&tmpstr);
	}
	else if(ajStrGetLen(qry->Key))
	{
	    ajStrAssignS(&tmpstr, qry->Key);
	    ajStrExchangeKK(&tmpstr, ' ', '+');
	    ajFmtPrintAppS(&get, "&term=%S[kywd]",
			   tmpstr);
	    ajStrDel(&tmpstr);
	}
	else
	    ajFmtPrintAppS(&get, "+%S",
			   searchdb);
	ajStrDel(&searchdb);

	if (!ajStrGetLen(gilist))
	{
	    /*
	     ** search to find list of GIs
	     ** use GI list to retrieve genbank entries
	     */

	    ajDebug("searching with Entrez url '%S'\n", get);

	    ajFmtPrintAppS(&get, " HTTP/%S\n", httpver);

	    ajStrAssignS(&seqin->Db, qry->DbName);

	    /* finally we have set the GET command */
	    ajDebug("host '%S' port %d get '%S'\n", host, iport, get);

	    if(ajStrGetLen(proxyName))
		fp = seqHttpGetProxy(qry, proxyName, proxyPort,
				     host, iport, get);
	    else
		fp = seqHttpGet(qry, host, iport, get);

	    ajStrDel(&get);
	    ajStrDel(&host);
	    ajStrDel(&proxyName);
	    ajStrDel(&httpver);

	    if(!fp)
		return ajFalse;

#ifndef WIN32
	    signal(SIGALRM, seqSocketTimeout);
	    alarm(180);	    /* allow 180 seconds to read from the socket */
#endif

	    /*
	     ** read out list of GI numbers
	     ** use to build new query
	     ** return genbank format results
	     */

	    ajDebug("GI list returned\n");
	    gifile = ajFileNewF(fp);
	    while (ajFileGetsTrim(gifile, &giline)) {
		ajDebug("+%S\n", giline);
		if(!ajStrGetLen(giline)) break;
	    }

	    numgi=0;
	    while (ajFileGetsTrim(gifile, &giline)) {
		ajStrTrimWhite(&giline);
		ajDebug("-%S\n", giline);
		if (!icount && ajRegExec(seqRegEntrezCount,giline))
		{
		    ajRegSubI(seqRegEntrezCount, 1, &tmpstr);
		    ajStrToInt(tmpstr, &icount);
		    if(!icount)
		    {
			ajStrDel(&giline);
			ajStrDel(&gilist);
			ajStrDel(&tmpstr);
			ajFileClose(&gifile);
			return ajFalse;
		    }
		}
		if (ajRegExec(seqRegEntrezId, giline))
		{
		    ajRegSubI(seqRegEntrezId, 1, &tmpstr);
		    if (numgi)
			ajStrAppendK(&gilist, '+');
		    ajStrAppendS(&gilist, tmpstr);
		    numgi++;
		}
	    }
	    ajFileClose(&gifile);
	    ajStrDel(&giline);
	}
	if (!ajStrGetLen(gilist))
	    return ajFalse;
	ajStrAssignS((AjPStr*) &qry->QryData, gilist);
    }

    ajDebug("seqAccessEntrez ready '%S' '%S'\n",
	    gilist, (AjPStr) qry->QryData);
    if(seqEntrezQryNext(qry, seqin))
    {
	ret =  ajTrue;
	ajDebug("seqAccessEntrez after QryNext '%S' '%S'\n",
		gilist, (AjPStr) qry->QryData);
    }
    else
    {
	ajStrDel((AjPStr*) &qry->QryData);
    }

    ajStrDel(&gilist);
    ajStrDel(&giline);
    ajStrDel(&get);
    ajStrDel(&host);
    ajStrDel(&tmpstr);
    ajStrDel(&httpver);

    return ret;
}





/* @funcstatic seqEntrezQryNext *********************************************
**
** Processes next GI in list and sets up file buffer with a genbank entry
**
** @param [u] qry [AjPSeqQuery] Query data
** @param [u] seqin [AjPSeqin] Sequence input, including query data.
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
** @@
******************************************************************************/

static AjBool seqEntrezQryNext(AjPSeqQuery qry, AjPSeqin seqin)
{
    AjPStr host      = NULL;
    AjPStr urlfetch  = NULL;
    AjPStr get       = NULL;
    AjPStr proxyName = NULL;		/* host for proxy access.*/
    AjPStr httpver   = NULL;		/* HTTP version for GET */
    ajint iport;
    ajint proxyPort;
    AjPStr gilist = NULL;
    FILE *sfp;
    AjPStr gistr = NULL;
    AjPStr tmpstr=NULL;
    AjPFileBuff seqfile = NULL;
    ajint ihead=0;
    AjPStr seqline=NULL;

    if (!seqRegGi)
	seqRegGi = ajRegCompC("(\\d+)");

    iport     = 80;
    proxyPort = 0;			/* port for proxy access */

    gilist = qry->QryData;
    ajStrAssignC(&urlfetch, "/entrez/eutils/efetch.fcgi");

    /* eutils.ncbi.nlm.nih.gov official address
     gives an error with valgrind */
    ajStrAssignC(&host, "www.ncbi.nlm.nih.gov");
    iport = 80;

    if (!gilist)
    {
	ajDebug("seqEntrezQryNext null gilist\n");
	return ajFalse;
    }
    if(!ajRegExec(seqRegGi, gilist))
    {
	ajDebug("seqEntrezQryNext no match gilist '%S'\n", gilist);
	ajStrDel((AjPStr*)&qry->QryData);
	return ajFalse;
    }

    ajRegSubI(seqRegGi, 1, &gistr);
    ajRegPost(seqRegGi, &tmpstr);
    ajStrAssignS((AjPStr*)&qry->QryData, tmpstr);
    ajDebug("seqEntrezQryNext next gi '%S'\n", gistr);

    seqHttpVersion(qry, &httpver);
    if(seqHttpProxy(qry, &proxyPort, &proxyName))
	ajFmtPrintS(&get,
		    "GET http://%S:%d%S?", host, iport, urlfetch);
    else
	ajFmtPrintS(&get, "GET %S?", urlfetch);
    ajStrDel(&urlfetch);

    ajStrAppendC(&get, "tool=emboss&email=pmr@ebi.ac.uk&retmax=1000");
    if(ajStrPrefixCaseC(qry->DbType, "N"))
	ajStrAppendC(&get, "&db=nucleotide&rettype=gb");
    else
	ajStrAppendC(&get, "&db=protein&rettype=gp");

    ajFmtPrintAppS(&get, "&id=%S", gistr);
    ajFmtPrintAppS(&get, " HTTP/%S\n", httpver);

    ajStrDel(&gistr);
    ajStrDel(&httpver);

    if(ajStrGetLen(proxyName))
	sfp = seqHttpGetProxy(qry, proxyName, proxyPort, host, iport, get);
    else
	sfp = seqHttpGet(qry, host, iport, get);
    if(!sfp)
	return ajFalse;

    ajFileBuffDel(&seqin->Filebuff);
    seqin->Filebuff = ajFileBuffNew();
    seqfile = ajFileBuffNewF(sfp);
    if(!seqfile)
    {
	ajDebug("socket buffer attach failed\n");
	ajErr("socket buffer attach failed for database '%S'",
	      qry->DbName);
	return ajFalse;
    }

    ihead=1;
    while(ajFileBuffGet(seqfile, &seqline))
    {
	if(ihead)
	{
	    ajStrTrimWhite(&seqline);
	    if(!ajStrGetLen(seqline))
		ihead=0;
	}
	else
	{
	    ajDebug("Processing %S\n", seqline);
	    ajFileBuffLoadS(seqin->Filebuff, seqline);
	    if(ajStrPrefixC(seqline, "//"))
		break;
	}
    }
    ajFileBuffDel(&seqfile);

#ifndef WIN32
    alarm(0);
#endif

    ajStrAssignS(&seqin->Db, qry->DbName);

    ajStrDel(&host);
    ajStrDel(&get);
    ajStrDel(&proxyName);
    ajStrDel(&tmpstr);
    ajStrDel(&seqline);

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    return ajTrue;
}




/*=============================================================================
** SeqHound indexed database access
**===========================================================================*/

/* @section SeqHound Database Indexing ****************************************
**
** These functions manage the SeqHound remote web API access methods.
**
******************************************************************************/


/* @funcstatic seqAccessSeqhound **********************************************
**
** Reads sequence(s) using Seqhound. Sends a query to a remote Seqhound
** web server. Uses the query to construct a list of GenInfo Identifiers
** (GI numbers). Uses these to return full entries in Genbank format.
** Opens a file using the results and returns to the caller to
** read the data.
**
** Seqhound access currently does not allow query by description or keyword
**  - but this may be included in SeqHound in the near future.
**
** Taxonomy queries require a numeric taxon ID. In future, a way to use
** taxon names may be added.
**
** @param [u] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqAccessSeqhound(AjPSeqin seqin)
{
    AjPStr host      = NULL;
    AjPStr urlget    = NULL;
    AjPStr get       = NULL;
    AjPStr proxyName = NULL;		/* host for proxy access.*/
    AjPStr httpver   = NULL;		/* HTTP version for GET */
    ajint iport;
    ajint proxyPort;
    AjPStr gilist = NULL;
    FILE *fp;
    AjPFile gifile = NULL;
    AjPStr giline = NULL;
    AjPSeqQuery qry;
    ajint numgi = 0;
    const AjPStr cpystr = NULL;
    AjBool ret = AJFALSE;

    qry = seqin->Query;

    if (!qry->QryData)
    {
	seqin->Single = ajTrue;

	/* new query - build a GI list */
	iport     = 80;
	proxyPort = 0;			/* port for proxy access */

	if(!ajNamDbGetDbalias(qry->DbName, &qry->DbAlias))
	    ajStrAssignS(&qry->DbAlias, qry->DbName);
	ajDebug("seqAccessSeqhound %S:%S\n", qry->DbAlias, qry->Id);

	if(!seqHttpUrl(qry, &iport, &host, &urlget))
	{
	    ajStrDel(&host);
	    ajStrDel(&urlget);
	    return ajFalse;
	}

	seqHttpVersion(qry, &httpver);
	if(seqHttpProxy(qry, &proxyPort, &proxyName))
	    ajFmtPrintS(&get, "GET http://%S:%d%S?",
			host, iport, urlget);
	else
	    ajFmtPrintS(&get, "GET %S?", urlget);
	ajStrDel(&urlget);

	/* Id FindNameList&pname= */
	if(ajStrGetLen(qry->Id))
	    ajFmtPrintAppS(&get, "fnct=SeqHoundFindNameList&pname=%S",
			   qry->Id);
	/* Acc FindAccList&pnacc= */
	else if(qry->HasAcc && ajStrGetLen(qry->Acc))
	    ajFmtPrintAppS(&get, "fnct=SeqHoundFindAccList&pacc=%S",
			   qry->Acc);
	/* Gi Use directly! */
	else if(ajStrGetLen(qry->Gi))
	    ajStrAssignS(&gilist,qry->Gi);
	/* Sv trim to Acc */
	else if(ajStrGetLen(qry->Sv))
	{
	    cpystr = ajSeqtestIsSeqversion(qry->Sv);
	    if (cpystr)
		ajFmtPrintAppS(&get,"fnct=SeqHoundFindAccList&pacc=%S",
			       cpystr);
	    else
		ajWarn("SeqHound invalid Seqversion '%S'", qry->Sv);
	}
	/* Des not yet available - will have Lucene soon */
	else if(ajStrGetLen(qry->Des))
	{
	    ajWarn("SeqHound search by description not yet available\n");
	    /*ajFmtPrintAppS(&get, "fnct=SeqHoundFindDesList&pdes=%S",
	      qry->Des);*/
	}
	/* Tax need to find taxid [Protein|DNA]FromTaxIDList&taxid= */
	else if(ajStrGetLen(qry->Org))
	{
	    if(ajStrPrefixCaseC(qry->DbType, "N"))
		ajFmtPrintAppS(&get, "SeqHoundDNAFromTaxIDList&taxid=%S",
			       qry->Org);
	    else
		ajFmtPrintAppS(&get, "SeqHoundProteinFromTaxIDList&taxid=%S",
			       qry->Org);
	}
	/* Key not yet available - will have Lucene soon */
	else if(ajStrGetLen(qry->Key))
	{
	    ajWarn("SeqHound search by keyword not yet available\n");
	    /*ajFmtPrintAppS(&get, "fnct=SeqHoundFindKeyList&pkey=%S",
	      qry->Key);*/
	}
	/* whole database = maybe taxid=1 or 0 (root)? */
	else
	    ajFmtPrintAppS(&get, "+%S",
			   qry->DbAlias);

	if (!ajStrGetLen(gilist))  /*  could be set directly for GI search */
	{
	    /*
	     ** search to find list of GIs
	     ** use GI list to retrieve genbank entries
	     */

	    ajDebug("searching with SeqHound url '%S'\n", get);

	    ajFmtPrintAppS(&get, " HTTP/%S\n", httpver);
	    ajStrDel(&httpver);

	    ajStrAssignS(&seqin->Db, qry->DbName);

	    /* finally we have set the GET command */
	    ajDebug("host '%S' port %d get '%S'\n", host, iport, get);

	    if(ajStrGetLen(proxyName))
		fp = seqHttpGetProxy(qry, proxyName, proxyPort,
				     host, iport, get);
	    else
		fp = seqHttpGet(qry, host, iport, get);
	    if(!fp)
	    {
		ajStrDel(&host);
		ajStrDel(&get);
		return ajFalse;
	    }

#ifndef WIN32
	    signal(SIGALRM, seqSocketTimeout);
	    alarm(180);	    /* allow 180 seconds to read from the socket */
#endif

	    /*
	     ** read out list of GI numbers
	     ** use to build new query
	     ** return genbank format results
	     */

	    ajDebug("GI list returned\n");
	    gifile = ajFileNewF(fp);
	    while (ajFileGetsTrim(gifile, &giline)) {
		ajDebug("+%S\n", giline);
		if(!ajStrGetLen(giline)) break;
	    }

	    ajFileGetsTrim(gifile, &giline);
	    ajDebug("=%S\n", giline);
	    ajStrTrimWhite(&giline);
	    if (!ajStrMatchC(giline, "SEQHOUND_OK"))
	    {
		ajDebug("SeqHound returned code '%S'", giline);
		ajStrDel(&get);
		ajStrDel(&host);
		ajStrDel(&giline);
		ajFileClose(&gifile);
		return ajFalse;
	    }

	    numgi=0;
	    while (ajFileGetsTrim(gifile, &giline)) {
		ajStrTrimWhite(&giline);
		ajDebug("-%S\n", giline);
		if (numgi)
		    ajStrAppendK(&gilist, '+');
		else
		    if (ajStrMatchC(giline, "(null)"))
		    {
			ajDebug("SeqHound found no entries");
			ajStrDel(&get);
			ajStrDel(&host);
			ajStrDel(&giline);
			ajFileClose(&gifile);
			return ajFalse;
		    }
		ajStrAppendS(&gilist, giline);
		numgi++;
	    }
	    ajFileClose(&gifile);
	    ajStrDel(&giline);
	}
	if (!ajStrGetLen(gilist))
	{
	    ajStrDel(&get);
	    ajStrDel(&host);
	    return ajFalse;
	}
	ajDebug("seqAccessSeqhound QryData '%S' <= '%S'\n",
	       (AjPStr) qry->QryData, gilist);
	ajStrAssignS((AjPStr*) &qry->QryData, gilist);
    }

    ajDebug("seqAccessSeqhound ready '%S' '%S'\n",
	    gilist, (AjPStr) qry->QryData);

    if(seqSeqhoundQryNext(qry, seqin))
    {
	ret = ajTrue;
	ajDebug("seqAccessSeqhound after QryNext '%S' '%S'\n",
		gilist, (AjPStr) qry->QryData);
    }

    ajStrDel(&gilist);
    ajStrDel(&host);
    ajStrDel(&urlget);
    ajStrDel(&get);
    ajStrDel(&httpver);
    ajStrDel(&proxyName);

    return ret;

}

/* @funcstatic seqSeqhoundQryNext *********************************************
**
** Processes next GI in list and sets up file buffer with a genbank entry
**
** @param [u] qry [AjPSeqQuery] Query data
** @param [u] seqin [AjPSeqin] Sequence input, including query data.
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
** @@
******************************************************************************/

static AjBool seqSeqhoundQryNext(AjPSeqQuery qry, AjPSeqin seqin)
{
    AjPStr host      = NULL;
    AjPStr urlget    = NULL;
    AjPStr get       = NULL;
    AjPStr proxyName = NULL;		/* host for proxy access.*/
    AjPStr httpver   = NULL;		/* HTTP version for GET */
    ajint iport;
    ajint proxyPort;
    AjPStr gilist = NULL;
    FILE *fp;
    AjPStr giline = NULL;
    AjPStr gistr = NULL;
    AjPStr tmpstr=NULL;

    if (!seqRegGi)
	seqRegGi = ajRegCompC("(\\d+)");

    iport     = 80;
    proxyPort = 0;			/* port for proxy access */

    gilist = qry->QryData;

    if (!gilist)
    {
	ajDebug("seqSeqhoundQryNext null gilist\n");
	ajDebug("seqSeqhoundQryNext failed: null gilist '%S' QryData '%S'\n",
		gilist, (AjPStr) qry->QryData);
	return ajFalse;
    }
    if(!ajRegExec(seqRegGi, gilist))
    {
	ajDebug("seqSeqhoundQryNext no match gilist '%S'\n", gilist);
	ajStrDel((AjPStr*)&qry->QryData);
	ajDebug("seqSeqhoundQryNext failed: no match "
		"gilist '%S' QryData '%S'\n",
		gilist, (AjPStr) qry->QryData);
	return ajFalse;
    }

    ajRegSubI(seqRegGi, 1, &gistr);
    ajRegPost(seqRegGi, &tmpstr);
    ajStrAssignS((AjPStr*)&qry->QryData, tmpstr);
    ajStrDel(&tmpstr);

    ajDebug("seqSeqhoundQryNext next gi '%S'\n", gistr);

    if(!seqHttpUrl(qry, &iport, &host, &urlget))
    {
	ajStrDel(&gistr);
	ajStrDel(&host);
	ajStrDel(&urlget);
	ajDebug("seqSeqhoundQryNext failed: seqHttpUrl failed "
		"gilist '%S' QryData '%S'\n",
		gilist, (AjPStr) qry->QryData);
	return ajFalse;
    }
    seqHttpVersion(qry, &httpver);
    if(seqHttpProxy(qry, &proxyPort, &proxyName))
	ajFmtPrintS(&get, "GET http://%S:%d%S?",
		    host, iport, urlget);
    else
	ajFmtPrintS(&get, "GET %S?", urlget);
    if(ajStrGetLen(proxyName))
	ajFmtPrintS(&get, "GET http://%S:%d%S?",
		    host, iport, urlget);
    else
	ajFmtPrintS(&get, "GET %S?", urlget);
    ajStrDel(&urlget);

    ajFmtPrintAppS(&get, "fnct=SeqHoundGetGenBankff&gi=%S", gistr);
    ajFmtPrintAppS(&get, " HTTP/%S\n", httpver);
    ajStrDel(&httpver);
    ajStrDel(&gistr);

    if(ajStrGetLen(proxyName))
	fp = seqHttpGetProxy(qry, proxyName, proxyPort, host, iport, get);
    else
	fp = seqHttpGet(qry, host, iport, get);
    if(!fp)
    {
	ajDebug("seqSeqhoundQryNext failed: seqHttpGet* failed "
		"gilist '%S' QryData '%S'",
		gilist, (AjPStr) qry->QryData);
	return ajFalse;
    }
    ajStrDel(&host);
    ajStrDel(&get);
    ajStrDel(&proxyName);

    ajFileBuffDel(&seqin->Filebuff);
    seqin->Filebuff = ajFileBuffNewF(fp);
    if(!seqin->Filebuff)
    {
	ajDebug("socket buffer attach failed\n");
	ajErr("socket buffer attach failed for database '%S'", qry->DbName);
	return ajFalse;
    }

    ajFileBuffLoad(seqin->Filebuff);

#ifndef WIN32
    alarm(0);
#endif

    ajFileBuffStripHtml(seqin->Filebuff);
    ajFileBuffGet(seqin->Filebuff, &giline);
    ajStrTrimWhite(&giline);
    if (!ajStrMatchC(giline, "SEQHOUND_OK"))
    {
	ajStrDel(&giline);
	ajFileBuffReset(seqin->Filebuff);
	ajDebug("seqSeqhoundQryNext failed: SEQHOUND_OK not found "
		"gilist '%S' QryData '%S'\n",
		gilist, (AjPStr) qry->QryData);
	return ajFalse;
    }
    ajFileBuffClear(seqin->Filebuff, 0);
    ajFileBuffPrint(seqin->Filebuff, "Genbank data");

    ajStrAssignS(&seqin->Db, qry->DbName);

    ajStrDel(&giline);

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    ajDebug("seqSeqhoundQryNext success: null gilist '%S' QryData '%S'\n",
	    gilist, (AjPStr) qry->QryData);
    if (!ajStrGetLen((AjPStr)qry->QryData))
	ajStrDel((AjPStr*)&qry->QryData);
	
    return ajTrue;
}





/*=============================================================================
** SRS indexed database access
**===========================================================================*/

/* @section SRS Database Indexing *********************************************
**
** These functions manage the SRS (getz) index access methods.
**
******************************************************************************/

/* @funcstatic seqAccessSrs ***************************************************
**
** Reads sequence(s) using SRS. Opens a file using the results of an SRS
** query and returns to the caller to read the data.
**
** @param [u] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqAccessSrs(AjPSeqin seqin)
{
    AjPSeqQuery qry;

    qry = seqin->Query;

    if(!ajNamDbGetDbalias(qry->DbName, &qry->DbAlias))
	ajStrAssignS(&qry->DbAlias, qry->DbName);

    if(!ajStrGetLen(qry->Application))
	ajStrAssignC(&qry->Application, "getz");

    ajDebug("seqAccessSrs %S:%S\n", qry->DbAlias, qry->Id);
    if(ajStrGetLen(qry->Id))
    {
	ajFmtPrintS(&seqin->Filename, "%S -e '[%S-id:%S]",
		    qry->Application, qry->DbAlias, qry->Id);
	if(qry->HasAcc && ajStrMatchS(qry->Id, qry->Acc)) /* or accnumber */
	    ajFmtPrintAppS(&seqin->Filename, "|[%S-acc:%S]'|",
			   qry->DbAlias, qry->Id);
	else				/* just the ID query */
	    ajFmtPrintAppS(&seqin->Filename, "'|",
			   qry->DbAlias, qry->Id);
    }
    else if(qry->HasAcc && ajStrGetLen(qry->Acc))
	ajFmtPrintS(&seqin->Filename, "%S -e '[%S-acc:%S]'|",
		    qry->Application, qry->DbAlias, qry->Acc);
    else if(ajStrGetLen(qry->Gi))   /* a few SRS servers support GI as 'gid'*/
	ajFmtPrintS(&seqin->Filename,"%S -e '[%S-gid:%S]'|",
		    qry->Application, qry->DbAlias, qry->Gi);
    else if(ajStrGetLen(qry->Sv))
	ajFmtPrintS(&seqin->Filename,"%S -e '[%S-sv:%S]'|",
		    qry->Application, qry->DbAlias, qry->Sv);
    else if(ajStrGetLen(qry->Des))
	ajFmtPrintS(&seqin->Filename, "%S -e '[%S-des:%S]'|",
		    qry->Application, qry->DbAlias, qry->Des);
    else if(ajStrGetLen(qry->Org))
	ajFmtPrintS(&seqin->Filename, "%S -e '[%S-org:%S]'|",
		    qry->Application, qry->DbAlias, qry->Org);
    else if(ajStrGetLen(qry->Key))
	ajFmtPrintS(&seqin->Filename, "%S -e '[%S-key:%S]'|",
		    qry->Application, qry->DbAlias, qry->Key);
    else
	ajFmtPrintS(&seqin->Filename, "%S -e '%S'|",
		    qry->Application, qry->DbAlias);

    ajFileBuffDel(&seqin->Filebuff);
    seqin->Filebuff = ajFileBuffNewIn(seqin->Filename);
    if(!seqin->Filebuff)
    {
	ajDebug("unable to open file '%S'\n", seqin->Filename);
	return ajFalse;
    }
    ajStrAssignS(&seqin->Db, qry->DbName);

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    return ajTrue;
}




/* @funcstatic seqAccessSrsfasta **********************************************
**
** Reads sequence(s) using SRS. Opens a file using the results of an SRS
** query with FASTA format sequence output and returns to the caller to
** read the data.
**
** @param [u] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqAccessSrsfasta(AjPSeqin seqin)
{
    AjPSeqQuery qry;

    qry = seqin->Query;

    if(!ajNamDbGetDbalias(qry->DbName, &qry->DbAlias))
	ajStrAssignS(&qry->DbAlias, qry->DbName);

    if(!ajStrGetLen(qry->Application))
	ajStrAssignC(&qry->Application, "getz");

    ajDebug("seqAccessSrsfasta %S:%S\n", qry->DbAlias, qry->Id);
    if(ajStrGetLen(qry->Id))
    {
	ajFmtPrintS(&seqin->Filename, "%S -d -sf fasta '[%S-id:%S]",
		    qry->Application, qry->DbAlias, qry->Id);
	if(qry->HasAcc && ajStrMatchS(qry->Id, qry->Acc))
	    ajFmtPrintAppS(&seqin->Filename, "|[%S-acc:%S]'|",
			   qry->DbAlias, qry->Id);
	else				/* just the ID query */
	    ajFmtPrintAppS(&seqin->Filename, "'|",
			   qry->DbAlias, qry->Id);
    }
    else if(qry->HasAcc && ajStrGetLen(qry->Acc))
	ajFmtPrintS(&seqin->Filename, "%S -d -sf fasta '[%S-acc:%S]'|",
		    qry->Application, qry->DbAlias, qry->Acc);
    else if(ajStrGetLen(qry->Gi))
    {
        ajFmtPrintS(&seqin->Filename, "%S -d -sf fasta '[%S-gi:%S]'|",
		    qry->Application, qry->DbAlias, qry->Gi);
    }
    else if(ajStrGetLen(qry->Sv))
    {
        ajFmtPrintS(&seqin->Filename, "%S -d -sf fasta '[%S-sv:%S]'|",
		    qry->Application, qry->DbAlias, qry->Sv);
    }
    else if(ajStrGetLen(qry->Des))
	ajFmtPrintS(&seqin->Filename, "%S -d -sf fasta '[%S-des:%S]'|",
		    qry->Application, qry->DbAlias, qry->Des);
    else if(ajStrGetLen(qry->Org))
	ajFmtPrintS(&seqin->Filename, "%S -d -sf fasta '[%S-org:%S]'|",
		    qry->Application, qry->DbAlias, qry->Org);
    else if(ajStrGetLen(qry->Key))
	ajFmtPrintS(&seqin->Filename, "%S -d -sf fasta '[%S-key:%S]'|",
		    qry->Application, qry->DbAlias, qry->Key);
    else
	ajFmtPrintS(&seqin->Filename, "%S -d -sf fasta '%S'|",
		    qry->Application, qry->DbAlias);

    ajDebug("searching with SRS command '%S'\n", seqin->Filename);

    ajFileBuffDel(&seqin->Filebuff);
    seqin->Filebuff = ajFileBuffNewIn(seqin->Filename);
    if(!seqin->Filebuff)
    {
	ajDebug("unable to open file '%S'\n", seqin->Filename);
	return ajFalse;
    }

    ajStrAssignS(&seqin->Db, qry->DbName);

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    return ajTrue;
}




/* @funcstatic seqAccessSrswww ************************************************
**
** Reads sequence(s) using SRS. Sends a query to a remote SRS web server.
** Opens a file using the results and returns to the caller to
** read the data.
**
** @param [u] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqAccessSrswww(AjPSeqin seqin)
{
    AjPStr host      = NULL;
    AjPStr urlget    = NULL;
    AjPStr get       = NULL;
    AjPStr proxyName = NULL;		/* host for proxy access.*/
    AjPStr httpver   = NULL;		/* HTTP version for GET */
    ajint iport;
    ajint proxyPort;
    FILE *fp;
    AjPSeqQuery qry;

    qry = seqin->Query;

    iport     = 80;
    proxyPort = 0;			/* port for proxy axxess */

    if(!ajNamDbGetDbalias(qry->DbName, &qry->DbAlias))
	ajStrAssignS(&qry->DbAlias, qry->DbName);
    ajDebug("seqAccessSrswww %S:%S\n", qry->DbAlias, qry->Id);

    if(!seqHttpUrl(qry, &iport, &host, &urlget))
    {
	ajStrDel(&host);
	ajStrDel(&urlget);
	return ajFalse;
    }

    if(seqHttpProxy(qry, &proxyPort, &proxyName))
	ajFmtPrintS(&get, "GET http://%S:%d%S?-e+-ascii",
		    host, iport, urlget);
    else
	ajFmtPrintS(&get, "GET %S?-e+-ascii", urlget);

    ajStrDel(&urlget);

    if(ajStrGetLen(qry->Id))
    {
	ajFmtPrintAppS(&get, "+[%S-id:%S]",
		       qry->DbAlias, qry->Id);
	if(qry->HasAcc && ajStrMatchS(qry->Id, qry->Acc))
	    ajFmtPrintAppS(&get, "|[%S-acc:%S]",
			   qry->DbAlias, qry->Id);
    }
    else if(qry->HasAcc && ajStrGetLen(qry->Acc))
	ajFmtPrintAppS(&get, "+[%S-acc:%S]",
		       qry->DbAlias, qry->Acc);
    else if(ajStrGetLen(qry->Gi))
	ajFmtPrintAppS(&get,"+[%S-gid:%S]",
		       qry->DbAlias, qry->Gi);
    else if(ajStrGetLen(qry->Sv))
	ajFmtPrintAppS(&get,"+[%S-sv:%S]",
		       qry->DbAlias, qry->Sv);
    else if(ajStrGetLen(qry->Des))
	ajFmtPrintAppS(&get, "+[%S-des:%S]",
		       qry->DbAlias, qry->Des);
    else if(ajStrGetLen(qry->Org))
	ajFmtPrintAppS(&get, "+[%S-org:%S]",
		       qry->DbAlias, qry->Org);
    else if(ajStrGetLen(qry->Key))
	ajFmtPrintAppS(&get, "+[%S-key:%S]",
		       qry->DbAlias, qry->Key);
    else
	ajFmtPrintAppS(&get, "+%S",
		       qry->DbAlias);

    ajDebug("searching with SRS url '%S'\n", get);

    seqHttpVersion(qry, &httpver);
    ajFmtPrintAppS(&get, " HTTP/%S\n", httpver);
    ajStrDel(&httpver);

    ajStrAssignS(&seqin->Db, qry->DbName);

    /* finally we have set the GET command */
    ajDebug("host '%S' port %d get '%S'\n", host, iport, get);

    if(ajStrGetLen(proxyName))
	fp = seqHttpGetProxy(qry, proxyName, proxyPort, host, iport, get);
    else
	fp = seqHttpGet(qry, host, iport, get);
    ajStrDel(&proxyName);
    ajStrDel(&host);
    ajStrDel(&get);

    if(!fp)
	return ajFalse;

    ajFileBuffDel(&seqin->Filebuff);
    seqin->Filebuff = ajFileBuffNewF(fp);
    if(!seqin->Filebuff)
    {
	ajDebug("socket buffer attach failed\n");
	ajErr("socket buffer attach failed for database '%S'", qry->DbName);
	return ajFalse;
    }

#ifndef WIN32
    signal(SIGALRM, seqSocketTimeout);
    alarm(180);	    /* allow 180 seconds to read from the socket */
#endif

    ajFileBuffLoad(seqin->Filebuff);

#ifndef WIN32
    alarm(0);
#endif

    ajFileBuffStripHtml(seqin->Filebuff);

    ajStrAssignS(&seqin->Db, qry->DbName);

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    return ajTrue;
}




/* ==========================================================================
** EMBOSS B+tree database access
** ==========================================================================
*/

/* @section B+tree Database Indexing *****************************************
**
** These functions manage the B+tree index access methods.
**
******************************************************************************/

/* @funcstatic seqAccessEmboss ************************************************
**
** Reads a sequence using EMBOSS B+tree index files.
**
** @param [u] seqin [AjPSeqin] Sequence input, including query data.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqAccessEmboss(AjPSeqin seqin)
{
    AjBool retval = ajFalse;

    AjPSeqQuery qry;
    SeqPEmbossQry qryd = NULL;

    
    qry = seqin->Query;
    qryd = qry->QryData;
    ajDebug("seqAccessEmboss type %d\n", qry->Type);

    if(!ajNamDbGetDbalias(qry->DbName, &qry->DbAlias))
	ajStrAssignS(&qry->DbAlias, qry->DbName);
    
    if(qry->Type == QRY_ALL)
	return seqEmbossAll(seqin);


    if(!qry->QryData)
    {
	seqEmbossQryOpen(qry);

	qryd = qry->QryData;
	seqin->Single = ajTrue;
	
	if(qry->Type == QRY_ENTRY)
	{
	    if(!seqEmbossQryEntry(qry))
	    {
		ajDebug("B+tree Entry failed\n");
		if(qryd->do_id)
		    ajDebug("Database entry id:'%S' not found\n",qry->Id);
		else if(qryd->do_sv)
		    ajDebug("Database entry gi:'%S' not found\n",qry->Gi);
		else if(qryd->do_ac)
		    ajDebug("Database entry acc:'%S' not found\n",qry->Acc);
		else
		    ajDebug("Database entry '%S' not found\n",qry->Id);
	    }
	}

	if(qry->Type == QRY_QUERY)
	{
	    if(!seqEmbossQryQuery(qry))
	    {
		ajDebug("EMBOSS B+tree Query failed\n");
		if(ajStrGetLen(qry->Id))
		    ajDebug("Database Query '%S' not found\n", qry->Id);
		else if(qry->HasAcc && ajStrGetLen(qry->Acc))
		    ajDebug("Database Query acc:'%S' not found\n", qry->Acc);
		else if(ajStrGetLen(qry->Sv))
		    ajDebug("Database Query 'sv:%S' not found\n", qry->Sv);
		else if(ajStrGetLen(qry->Gi))
		    ajDebug("Database Query 'gi:%S' not found\n", qry->Gi);
		else if(ajStrGetLen(qry->Des))
		    ajDebug("Database Query 'des:%S' not found\n", qry->Des);
		else if(ajStrGetLen(qry->Key))
		    ajDebug("Database Query 'key:%S' not found\n", qry->Key);
		else if(ajStrGetLen(qry->Org))
		    ajDebug("Database Query 'org:%S' not found\n", qry->Org);
		else
		    ajDebug("Database Query '%S' not found\n", qry->Id);
	    }
	}
    }
    else if(!seqEmbossQryReuse(qry))
	return ajFalse;
 

    if(ajListGetLength(qryd->List))
    {
	retval = seqEmbossQryNext(qry);
	if(retval)
	    ajFileBuffSetFile(&seqin->Filebuff, qryd->libs, qryd->Samefile);
    }

    if(!ajListGetLength(qryd->List)) /* could have been emptied by code above */
    {
	seqEmbossQryClose(qry);
	/* AJB addition */
        /*
	 * This was for the old code where seqsets had different
         * memory handling ... and the reason for the multi
         * flag in the first place. So far this seems
         * unnecessary for the revised code but is left here
         * for a while as a reminder and 'just in case'
	 */
	if((qry->Type == QRY_ENTRY) && !seqin->multi)
	{
	    AJFREE(qry->QryData);
	    qryd = NULL;
	}
    }


    ajStrAssignS(&seqin->Db, qry->DbName);

    return retval;
}




/* @funcstatic seqAccessFreeEmboss ********************************************
**
** Frees data specific to reading EMBOSS B+tree index files.
**
** @param [r] qry [void*] query data specific to EMBLCD
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqAccessFreeEmboss(void* qry)
{
    AjPSeqQuery query;
    SeqPEmbossQry qryd;
    AjBool retval = ajTrue;

    ajDebug("seqAccessFreeEmboss\n");

    query = (AjPSeqQuery) qry;
    qryd = query->QryData;
    qryd->nentries = -1;

    seqEmbossQryClose(query);

    return retval;
}




/* @funcstatic seqEmbossQryReuse *********************************************
**
** Tests whether the B+tree index query data can be reused or it's finished.
**
** Clears qryData structure when finished.
**
** @param [u] qry [AjPSeqQuery] Query data
** @return [AjBool] ajTrue if reusable,
**                  ajFalse if finished.
** @@
******************************************************************************/

static AjBool seqEmbossQryReuse(AjPSeqQuery qry)
{
    SeqPEmbossQry qryd;

    qryd = qry->QryData;

    if(!qry || !qryd)
	return ajFalse;


    if(!qryd->List)
    {
	ajDebug("seqEmbossQryReuse: query data all finished\n");
	AJFREE(qry->QryData);
	qryd = NULL;
	return ajFalse;
    }
    else
    {
	ajDebug("seqEmbossQryReuse: reusing data from previous call %x\n",
		qry->QryData);
	/*ajListTrace(qryd->List);*/
    }


    qryd->nentries = -1;


    return ajTrue;
}




/* @funcstatic seqEmbossQryOpen **********************************************
**
** Open caches (etc) for B+tree search
**
** @param [u] qry [AjPSeqQuery] Query data
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
** @@
******************************************************************************/

static AjBool seqEmbossQryOpen(AjPSeqQuery qry)
{
    SeqPEmbossQry qryd;
    AjPStr baseindex = NULL;
    ajint i;
    AjPStr name     = NULL;
    
    baseindex = ajStrNew();

    qry->QryData = AJNEW0(qryd);
    qryd = qry->QryData;
    qryd->div = -1;
    qryd->nentries = -1;
    
    qryd->List = ajListNew();


    if(ajStrGetLen(qry->Id))
    {
	ajStrFmtLower(&qry->Id);
	qryd->do_id = ajTrue;
    }
    
    if(qry->HasAcc && ajStrGetLen(qry->Acc))
    {
	ajStrFmtLower(&qry->Acc);
	qryd->do_ac = ajTrue;
    }
    
    if(ajStrGetLen(qry->Des))
    {
	ajStrFmtLower(&qry->Des);
	qryd->do_id = ajTrue;
	qryd->do_de = ajTrue;
    }

    if(ajStrGetLen(qry->Key))
    {
	ajStrFmtLower(&qry->Key);
	qryd->do_id = ajTrue;
	qryd->do_kw = ajTrue;
    }

    if(ajStrGetLen(qry->Org))
    {
	ajStrFmtLower(&qry->Org);
	qryd->do_id = ajTrue;
	qryd->do_tx = ajTrue;
    }

    if(ajStrGetLen(qry->Sv) || ajStrGetLen(qry->Gi))
    {
	if(ajStrGetLen(qry->Sv))
	   ajStrFmtLower(&qry->Sv);
	if(ajStrGetLen(qry->Gi))
	    ajStrFmtLower(&qry->Gi);
	qryd->do_sv = ajTrue;
    }
    


    if(!ajStrGetLen(qry->IndexDir))
    {
	ajDebug("no indexdir defined for database '%S'\n", qry->DbName);
	ajErr("no indexdir defined for database '%S'", qry->DbName);
	return ajFalse;
    }

    ajDebug("directory '%S' entry '%S' acc '%S' hasacc:%B\n",
	    qry->IndexDir, qry->Id, qry->Acc, qry->HasAcc);


    if(qryd->do_id)
        seqEmbossOpenSecCache(qry,ID_EXTENSION,&qryd->idcache);

    if(qryd->do_ac)
	seqEmbossOpenSecCache(qry,AC_EXTENSION,&qryd->accache);

    if(qryd->do_sv)
	seqEmbossOpenSecCache(qry,SV_EXTENSION,&qryd->svcache);

    if(qryd->do_kw)
    {
	if(!qryd->idcache)
	    seqEmbossOpenCache(qry,ID_EXTENSION,&qryd->idcache);
	seqEmbossOpenSecCache(qry,KW_EXTENSION,&qryd->kwcache);
    }

    if(qryd->do_de)
    {
	if(!qryd->idcache)
	    seqEmbossOpenSecCache(qry,ID_EXTENSION,&qryd->idcache);	
	seqEmbossOpenSecCache(qry,DE_EXTENSION,&qryd->decache);
    }

    if(qryd->do_tx)
    {
	if(!qryd->idcache)
	    seqEmbossOpenSecCache(qry,ID_EXTENSION,&qryd->idcache);	
	seqEmbossOpenSecCache(qry,TX_EXTENSION,&qryd->txcache);
    }




    if(ajStrGetLen(qry->Exclude) && qryd->nentries >= 0)
    {
	AJCNEW0(qryd->Skip,qryd->nentries);
	name     = ajStrNew();
	
	for(i=0; i < qryd->nentries; ++i)
	{
	    ajStrAssignS(&name,qryd->files[i]);
	    if(!ajFileTestSkip(name, qry->Exclude, qry->Filename,
			       ajTrue, ajTrue))
		qryd->Skip[i] = ajTrue;
	}

	ajStrDel(&name);
    }

    ajStrDel(&baseindex);
    
    return ajTrue;
}




/* @funcstatic seqEmbossOpenCache ********************************************
**
** Create primary B+tree index cache
**
** @param [u] qry [AjPSeqQuery] Query data
** @param [r] ext [const char *] Index file extension
** @param [w] cache [AjPBtcache *] cache
** @return [void]
** @@
******************************************************************************/

static void seqEmbossOpenCache(AjPSeqQuery qry, const char *ext,
			       AjPBtcache *cache)
{
    SeqPEmbossQry qryd;
    ajint order     = 0;
    ajint fill      = 0;
    ajint pagesize  = 0;
    ajint cachesize = 0;
    ajint level     = 0;
    ajint sorder    = 0;
    ajint sfill     = 0;
    ajlong count    = 0L;
    ajint kwlimit   = 0;
    
    AjPBtpage page   = NULL;

    qryd = qry->QryData;

    ajBtreeReadParams(ajStrGetPtr(qry->DbAlias),ext,ajStrGetPtr(qry->IndexDir),
		      &order, &fill,
		      &pagesize, &level,
		      &cachesize, &sorder,
		      &sfill, &count, &kwlimit);
    
    if(qryd->nentries == -1)
	qryd->nentries = ajBtreeReadEntries(ajStrGetPtr(qry->DbAlias),
					    ajStrGetPtr(qry->IndexDir),
					    ajStrGetPtr(qry->Directory),
					    &qryd->files,&qryd->reffiles);

    *cache = ajBtreeCacheNewC(ajStrGetPtr(qry->DbAlias),ext,
			      ajStrGetPtr(qry->IndexDir),"r",
			      pagesize,order,fill,level,
			      cachesize);

    if(!*cache)
    {
	qryd->nentries = -1;
	return;
    }
    
    page = ajBtreeCacheRead(*cache,0L);
    page->dirty = BT_LOCK;
    
    return;
}




/* @funcstatic seqEmbossOpenSecCache *****************************************
**
** Create double (secondary) B+tree index cache
**
** @param [u] qry [AjPSeqQuery] Query data
** @param [r] ext [const char *] Index file extension
** @param [w] cache [AjPBtcache *] cache
** @return [void]
** @@
******************************************************************************/

static void seqEmbossOpenSecCache(AjPSeqQuery qry, const char *ext,
				  AjPBtcache *cache)
{
    SeqPEmbossQry qryd;
    ajint order     = 0;
    ajint fill      = 0;
    ajint pagesize  = 0;
    ajint cachesize = 0;
    ajint level     = 0;
    ajint sorder    = 0;
    ajint sfill     = 0;
    ajlong count    = 0L;
    ajint kwlimit   = 0;
    
    AjPBtpage page   = NULL;

    qryd = qry->QryData;

    ajBtreeReadParams(ajStrGetPtr(qry->DbAlias),ext,ajStrGetPtr(qry->IndexDir),
		      &order, &fill,
		      &pagesize, &level,
		      &cachesize, &sorder,
		      &sfill, &count, &kwlimit);
    
    if(qryd->nentries == -1)
	qryd->nentries = ajBtreeReadEntries(ajStrGetPtr(qry->DbAlias),
					    ajStrGetPtr(qry->IndexDir),
					    ajStrGetPtr(qry->Directory),
					    &qryd->files,&qryd->reffiles);
    
    
    *cache = ajBtreeSecCacheNewC(ajStrGetPtr(qry->DbAlias),ext,
				 ajStrGetPtr(qry->IndexDir),"r",
				 pagesize,order,fill,level,
				 cachesize,sorder,0,sfill,count,kwlimit);

    if(!*cache)
    {
	qryd->nentries = -1;
	return;
    }
    
    page = ajBtreeCacheRead(*cache,0L);
    page->dirty = BT_LOCK;
    
    return;
}




/* @funcstatic seqEmbossAll **************************************************
**
** Reads the B+tree entry file and opens a list of all the
** database files for plain reading.
**
** @param [u] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqEmbossAll(AjPSeqin seqin)
{
    AjPList list;
    AjPSeqQuery qry;
/*    AjPStrTok handle = NULL;
    AjPStr wildname = NULL;*/
    AjPStr name = NULL;
/*    AjBool del = ajFalse;*/
    
    ajint i;
    
    AjPStr *filestrings = NULL;
    AjPStr *reffilestrings = NULL;
    
    qry = seqin->Query;

    if(!ajStrGetLen(qry->IndexDir))
    {
	ajDebug("no indexdir defined for database %S\n", qry->DbName);
	ajErr("no indexdir defined for database %S", qry->DbName);
	return ajFalse;
    }

    ajDebug("B+tree All index directory '%S'\n", qry->IndexDir);


    ajBtreeReadEntries(qry->DbAlias->Ptr,qry->IndexDir->Ptr,
		       qry->Directory->Ptr,
		       &filestrings,&reffilestrings);


    list = ajListstrNew();
/*    wildname = ajStrNew();*/
    name     = ajStrNew();
    
    i = 0;
    while(filestrings[i])
    {

	ajStrAssignS(&name,filestrings[i]);
	if(ajFileTestSkip(name, qry->Exclude, qry->Filename,
			  ajTrue, ajTrue))
	{
	    ajDebug("qrybufflist add '%S'\n", name);
	    ajListstrPushAppend(list, name);
	    name = NULL;
	}
	else
	{
	    ajDebug("qrybufflist *delete* '%S'\n", name);
	    ajStrDel(&name);
	}
/*
	if(qry->Exclude)
	{
	    ajStrAssignS(&name,filestrings[i]);
	    ajFileNameTrim(&name);
	    del = ajFalse;
	    
	    handle = ajStrTokenNewC(qry->Exclude," \n");
	    while(ajStrTokenNextParseC(&handle," \n",&wildname))
		if(ajStrMatchWildS(name,wildname))
		{
		    del = ajTrue;
		    break;
		}
	    ajStrTokenDel(&handle);

	    if(del)
		ajStrDel(&filestrings[i]);
	    else
		ajListstrPushAppend(list, filestrings[i]);		
	}
	else
	    ajListstrPushAppend(list, filestrings[i]);
*/
	ajStrDel(&filestrings[i]);
	++i;
    }


    ajFileBuffDel(&seqin->Filebuff);
    seqin->Filebuff = ajFileBuffNewInList(list);

    ajStrAssignS(&seqin->Db, qry->DbName);

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    ajStrDel(&name);
/*    ajStrDel(&wildname);*/
    AJFREE(filestrings);
    AJFREE(reffilestrings);
    
    return ajTrue;
}




/* @funcstatic seqEmbossQryEntry *********************************************
**
** Queries for a single entry in a B+tree index
**
** @param [u] qry [AjPSeqQuery] Query data
** @return [AjBool] ajTrue if can continue,
**                  ajFalse if all is done.
** @@
******************************************************************************/

static AjBool seqEmbossQryEntry(AjPSeqQuery qry)
{
    AjPBtId entry  = NULL;
    SeqPEmbossQry qryd;
    
    ajDebug("entry id: '%S' acc: '%S' hasacc:%B\n",
	    qry->Id, qry->Acc, qry->HasAcc);

    qryd = qry->QryData;


    if(qryd->do_id && qryd->idcache)
    {
	    entry = ajBtreeIdFromKey(qryd->idcache,ajStrGetPtr(qry->Id));
	    if(entry)
	    {
		if(!entry->dups)
		    ajListPushAppend(qryd->List,(void *)entry);
		else
		    ajBtreeHybLeafList(qryd->idcache,entry->offset,
				       entry->id,qryd->List);
	    }
    }
    

    if((qryd->do_ac && !entry) && (qryd->do_ac && qryd->accache))
    {
	    entry = ajBtreeIdFromKey(qryd->accache,ajStrGetPtr(qry->Acc));
	    if(entry)
	    {
		if(!entry->dups)
		    ajListPushAppend(qryd->List,(void *)entry);
		else
		    ajBtreeHybLeafList(qryd->accache,entry->offset,
				       entry->id,qryd->List);
	    }
    }

    if((qryd->do_sv && !entry) && (qryd->do_sv && qryd->svcache))
    {
	    entry = ajBtreeIdFromKey(qryd->svcache,ajStrGetPtr(qry->Gi));
	    if(entry)
	    {
		if(!entry->dups)
		    ajListPushAppend(qryd->List,(void *)entry);
		else
		    ajBtreeHybLeafList(qryd->svcache,entry->offset,
				       entry->id,qryd->List);
	    }
    }

    if(!ajListGetLength(qryd->List))
	return ajFalse;

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    return ajTrue;
}




/* @funcstatic seqEmbossQryNext **********************************************
**
** Processes the next query for a B+tree index
**
** @param [u] qry [AjPSeqQuery] Query data
** @return [AjBool] ajTrue if successful
** @@
******************************************************************************/

static AjBool seqEmbossQryNext(AjPSeqQuery qry)
{
    AjPBtId entry;
    SeqPEmbossQry qryd;
    void* item;
    AjBool ok = ajFalse;


    qryd = qry->QryData;

    if(!ajListGetLength(qryd->List))
	return ajFalse;

    /*ajListTrace(qryd->List);*/


    if(!qryd->Skip)
    {
	ajListPop(qryd->List, &item);
	entry = (AjPBtId) item;
    }
    else
    {
	ok = ajFalse;
	while(!ok)
	{
	    ajListPop(qryd->List, &item);
	    entry = (AjPBtId) item;
	    if(!qryd->Skip[entry->dbno])
		ok = ajTrue;
	    else
	    {
		ajBtreeIdDel(&entry);
		if(!ajListGetLength(qryd->List))
		    return ajFalse;
	    }
	}
    }

    qryd->Samefile = ajTrue;
    if(entry->dbno != qryd->div)
    {
	qryd->Samefile = ajFalse;
	qryd->div = entry->dbno;
	ajFileClose(&qryd->libs);
	if(qryd->reffiles)
	    ajFileClose(&qryd->libr);
    }

    if(!qryd->libs)
    {
	qryd->libs = ajFileNewIn(qryd->files[entry->dbno]);
	if(!qryd->libs)
	{
	    ajBtreeIdDel(&entry);
	    return ajFalse;
	}
    }
	
    if(qryd->reffiles && !qryd->libr)
    {
	ajFileClose(&qryd->libr);
	qryd->libr = ajFileNewIn(qryd->reffiles[entry->dbno]);
	if(!qryd->libr)
	{
	    ajBtreeIdDel(&entry);
	    return ajFalse;
	}
    }
    
    
    ajFileSeek(qryd->libs, entry->offset, 0);
    if(qryd->reffiles)
	ajFileSeek(qryd->libr, entry->refoffset, 0);


    ajBtreeIdDel(&entry);

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    return ajTrue;
}




/* @funcstatic seqEmbossQryClose *********************************************
**
** Closes query data for a B+tree index
**
** @param [u] qry [AjPSeqQuery] Query data
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
** @@
******************************************************************************/

static AjBool seqEmbossQryClose(AjPSeqQuery qry)
{
    SeqPEmbossQry qryd;
    ajint i;

    if(!qry)
	return ajFalse;

    ajDebug("seqEmbossQryClose clean up qryd\n");

    qryd = qry->QryData;

    if(qryd->do_id && qryd->idcache)
    {
	ajBtreeFreeSecArray(qryd->idcache);
	ajBtreeCacheDel(&qryd->idcache);
    }
    

    if(qryd->do_ac && qryd->accache)
    {
	ajBtreeFreeSecArray(qryd->accache);	
	ajBtreeCacheDel(&qryd->accache);
    }
    

    if(qryd->do_sv && qryd->svcache)
    {
	ajBtreeFreeSecArray(qryd->svcache);	
	ajBtreeCacheDel(&qryd->svcache);
    }


    if(qryd->do_kw && qryd->kwcache)
	ajBtreeCacheDel(&qryd->kwcache);

    if(qryd->do_de && qryd->decache)
	ajBtreeCacheDel(&qryd->decache);

    if(qryd->do_tx && qryd->txcache)
	ajBtreeCacheDel(&qryd->txcache);
	
    qryd->do_id = ajFalse;
    qryd->do_ac = ajFalse;
    qryd->do_sv = ajFalse;
    qryd->do_kw = ajFalse;
    qryd->do_de = ajFalse;
    qryd->do_tx = ajFalse;

    ajListFree(&qryd->List);

    if(qryd->Skip)
    {
	AJFREE(qryd->Skip);
	qryd->Skip = NULL;
    }

    if(qryd->files)
    {
	i = 0;
	while(qryd->files[i])
	{
	    ajStrDel(&qryd->files[i]);
	    if(qryd->reffiles)
		ajStrDel(&qryd->reffiles[i]);
	    ++i;
	}
	AJFREE(qryd->files);
    }
    if(qryd->reffiles)
	AJFREE(qryd->reffiles);
    qryd->files = NULL;
    qryd->reffiles = NULL;


    /* keep QryData for use at top of loop */

    return ajTrue;
}




/* @funcstatic seqEmbossQryQuery *********************************************
**
** Queries for one or more entries in an EMBOSS B+tree index
**
** @param [u] qry [AjPSeqQuery] Query data
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
** @@
******************************************************************************/

static AjBool seqEmbossQryQuery(AjPSeqQuery qry)
{
    SeqPEmbossQry qryd;
    AjPBtId   id   = NULL;

    AjPBtPri pri   = NULL;
    AjPList  tlist = NULL;
    AjPStr   kwid  = NULL;

    
    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    qryd = qry->QryData;


    if(qryd->do_kw && qryd->kwcache)
    {
	if(!qry->Wild)
	{
	    pri = ajBtreePriFromKeyword(qryd->kwcache, ajStrGetPtr(qry->Key));
	    if(pri)
	    {
		tlist = ajBtreeSecLeafList(qryd->kwcache, pri->treeblock);
		while(ajListPop(tlist,(void **)&kwid))
		{
		    ajStrFmtLower(&kwid);
		    id = ajBtreeIdFromKey(qryd->idcache,ajStrGetPtr(kwid));
		    if(id)
		    {
			if(!id->dups)
			    ajListPushAppend(qryd->List,(void *)id);
			else
			    ajBtreeHybLeafList(qryd->idcache,id->offset,
					       id->id,qryd->List);
		    }
		    ajStrDel(&kwid);
		}
		ajListFree(&tlist);
		ajBtreePriDel(&pri);
	    }
	    return ajTrue;
	}
	else
	{
	    ajBtreeListFromKeywordW(qryd->kwcache,qry->Key->Ptr,
				    qryd->idcache, qryd->List);
	    return ajTrue;
	}
    }


    if(qryd->do_de && qryd->decache)
    {
	if(!qry->Wild)
	{
	    pri = ajBtreePriFromKeyword(qryd->decache, ajStrGetPtr(qry->Des));
	    if(pri)
	    {
		tlist = ajBtreeSecLeafList(qryd->decache, pri->treeblock);
		while(ajListPop(tlist,(void **)&kwid))
		{
		    ajStrFmtLower(&kwid);
		    id = ajBtreeIdFromKey(qryd->idcache,ajStrGetPtr(kwid));
		    if(id)
		    {
			if(!id->dups)
			    ajListPushAppend(qryd->List,(void *)id);
			else
			    ajBtreeHybLeafList(qryd->idcache,id->offset,
					       id->id,qryd->List);
		    }
		    ajStrDel(&kwid);
		}
		ajListFree(&tlist);
		ajBtreePriDel(&pri);
	    }
	    return ajTrue;
	}
	else
	{
	    ajBtreeListFromKeywordW(qryd->decache,qry->Des->Ptr,
				    qryd->idcache, qryd->List);
	    return ajTrue;
	}
    }
    

    if(qryd->do_tx && qryd->txcache)
    {
	if(!qry->Wild)
	{
	    pri = ajBtreePriFromKeyword(qryd->txcache, ajStrGetPtr(qry->Org));
	    if(pri)
	    {
		tlist = ajBtreeSecLeafList(qryd->txcache, pri->treeblock);
		while(ajListPop(tlist,(void **)&kwid))
		{
		    ajStrFmtLower(&kwid);
		    id = ajBtreeIdFromKey(qryd->idcache,ajStrGetPtr(kwid));
		    if(id)
		    {
			if(!id->dups)
			    ajListPushAppend(qryd->List,(void *)id);
			else
			    ajBtreeHybLeafList(qryd->idcache,id->offset,
					       id->id,qryd->List);
		    }
		    ajStrDel(&kwid);
		}
		ajListFree(&tlist);
		ajBtreePriDel(&pri);
	    }
	    return ajTrue;
	}
	else
	{
	    ajBtreeListFromKeywordW(qryd->txcache,qry->Org->Ptr,
				    qryd->idcache, qryd->List);
	    return ajTrue;
	}
    }


    if(qryd->do_id && qryd->idcache)
    {
	ajBtreeListFromKeyW(qryd->idcache,qry->Id->Ptr,qryd->List);
	if(ajListGetLength(qryd->List))
	    return ajTrue;
    }

    if(qryd->do_ac && qryd->accache)
    {
	ajBtreeListFromKeyW(qryd->accache,qry->Acc->Ptr,qryd->List);
	if(ajListGetLength(qryd->List))
	    return ajTrue;
    }

    if(qryd->do_sv && qryd->svcache)
    {
	ajBtreeListFromKeyW(qryd->svcache,qry->Sv->Ptr,qryd->List);
	return ajTrue;
    }

    return ajFalse;
}




/* @section B+tree GCG Database Indexing *************************************
**
** These functions manage the EMBOSS B+tree GCG index access methods.
**
******************************************************************************/




/* @funcstatic seqAccessEmbossGcg ********************************************
**
** Reads sequence(s) from a GCG formatted database, using EMBLCD index
** files. Returns with the file pointer set to the position in the
** sequence file.
**
** @param [u] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqAccessEmbossGcg(AjPSeqin seqin)
{
    AjBool retval = ajFalse;

    AjPSeqQuery qry;
    SeqPEmbossQry qryd = NULL;

    
    qry = seqin->Query;
    qryd = qry->QryData;
    ajDebug("seqAccessEmbossGcg type %d\n", qry->Type);

    if(!ajNamDbGetDbalias(qry->DbName, &qry->DbAlias))
	ajStrAssignS(&qry->DbAlias, qry->DbName);

    if(qry->Type == QRY_ALL)
	return seqEmbossGcgAll(seqin);


    if(!qry->QryData)
    {
	seqEmbossQryOpen(qry);

	qryd = qry->QryData;
	seqin->Single = ajTrue;
	ajFileBuffDel(&seqin->Filebuff);
	seqin->Filebuff = ajFileBuffNew();
	
	if(qry->Type == QRY_ENTRY)
	{
	    if(!seqEmbossQryEntry(qry))
		ajDebug("embossgcg B+tree Entry failed\n");
	}

	if(qry->Type == QRY_QUERY)
	{
	    if(!seqEmbossQryQuery(qry))
		ajDebug("embossgcg B+tree Query failed\n");
	}
    }
    else if(!seqEmbossQryReuse(qry))
	return ajFalse;
    

    if(ajListGetLength(qryd->List))
    {
	retval = seqEmbossQryNext(qry);
	if(retval)
	    seqEmbossGcgLoadBuff(seqin);
    }

    if(!ajListGetLength(qryd->List)) /* could have been emptied by code above */
    {
	seqEmbossQryClose(qry);
	ajFileClose(&qryd->libs);
	ajFileClose(&qryd->libr);
    }

    if(retval)
	ajStrAssignS(&seqin->Db, qry->DbName);

    return retval;
}




/* @funcstatic seqEmbossGcgAll ***********************************************
**
** Opens the first or next GCG file for further reading
**
** @param [u] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqEmbossGcgAll(AjPSeqin seqin)
{
    AjPSeqQuery qry;
    SeqPEmbossQry qryd;
    static ajint i   = 0;
    AjPStr name      = NULL;
    AjBool ok        = ajFalse;
/*
    AjPStrTok handle = NULL;
    AjPStr wildname  = NULL;
    AjBool found     = ajFalse;
*/
    
    qry = seqin->Query;
    qryd = qry->QryData;

    ajDebug("seqEmbossGcgAll\n");


    if(!qry->QryData)
    {
	ajDebug("seqEmbossGcgAll initialising\n");

	qry->QryData = AJNEW0(qryd);
	qryd = qry->QryData;
	i = -1;
	ajBtreeReadEntries(qry->DbAlias->Ptr,qry->IndexDir->Ptr,
			   qry->Directory->Ptr,
			   &qryd->files,&qryd->reffiles);

	seqin->Single = ajTrue;
    }

    qryd = qry->QryData;
    ajFileBuffDel(&seqin->Filebuff);
    seqin->Filebuff = ajFileBuffNew();

    if(!qryd->libs)
    {
	while(!ok && qryd->files[++i])
	{
	    ajStrAssignS(&name,qryd->files[i]);
	    if(ajFileTestSkip(name, qry->Exclude, qry->Filename,
			      ajTrue, ajTrue))
		ok = ajTrue;
	}
	ajStrDel(&name);

/*	if(qry->Exclude)
	{
	    ok = ajFalse;
	    wildname = ajStrNew();
	    name     = ajStrNew();
	    while(!ok)
	    {
		ajStrAssignS(&name,qryd->files[i]);
		ajFileNameTrim(&name);
		handle = ajStrTokenNewC(qry->Exclude," \n");
		found = ajFalse;
		while(ajStrTokenNextParseC(&handle," \n",&wildname))
		    if(ajStrMatchWildS(name,wildname))
		    {
			found = ajTrue;
			break;
		    }
		ajStrTokenDel(&handle);
		if(!found)
		    ok = ajTrue;
		else
		{
		    ++i;
		    if(!qryd->files[i])
			ok = ajTrue;
		}
	    }

	    ajStrDel(&wildname);
	    ajStrDel(&name);
	}
*/

	if(!qryd->files[i])
	{
	    ajDebug("seqEmbossGcgAll finished\n");
	    i=0;
	    while(qryd->files[i])
	    {
		ajStrDel(&qryd->files[i]);
		ajStrDel(&qryd->reffiles[i]);
		++i;
	    }
	    AJFREE(qryd->files);
	    AJFREE(qryd->reffiles);
	    
	    AJFREE(qry->QryData);
	    qry->QryData = NULL;
	    return ajFalse;
	}


	qryd->libs = ajFileNewIn(qryd->files[i]);

	if(!qryd->libs)
	{
	    ajDebug("seqEmbossGcgAll: cannot open sequence file\n");
	    return ajFalse;
	}


	qryd->libr = ajFileNewIn(qryd->reffiles[i]);

	if(!qryd->libr)
	{
	    ajDebug("seqEmbossGcgAll: cannot open reference file\n");
	    return ajFalse;
	}
    }

    seqEmbossGcgLoadBuff(seqin);

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    return ajTrue;
}




/* @funcstatic seqEmbossGcgLoadBuff ******************************************
**
** Copies text data to a buffered file, and sequence data for an
** AjPSeqin internal data structure for reading later
**
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [void]
** @@
******************************************************************************/

static void seqEmbossGcgLoadBuff(AjPSeqin seqin)
{
    AjPSeqQuery qry;
    SeqPEmbossQry qryd;

    qry  = seqin->Query;
    qryd = qry->QryData;

    ajDebug("seqEmbossGcgLoadBuff\n");

    if(!qry->QryData)
	ajFatal("seqEmbossGcgLoadBuff Query Data not initialised");

    /* copy all the ref data */

    seqEmbossGcgReadRef(seqin);

    /* write the sequence (do we care about the format?) */
    seqEmbossGcgReadSeq(seqin);

    /* ajFileBuffTraceFull(seqin->Filebuff, 9999, 100); */

    if(!qryd->libr)
	ajFileClose(&qryd->libs);

    return;
}




/* @funcstatic seqEmbossGcgReadRef *******************************************
**
** Copies text data to a buffered file for reading later
**
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqEmbossGcgReadRef(AjPSeqin seqin)
{
    AjPStr line = NULL;
    AjPSeqQuery qry;
    SeqPEmbossQry qryd;
    ajlong rpos;
    AjPStr id       = NULL;
    AjPStr idc      = NULL;
    AjBool ispir           = ajFalse;
    AjBool continued       = ajFalse;
    AjBool testcontinue    = ajFalse;
    char *p = NULL;

    qry  = seqin->Query;
    qryd = qry->QryData;

    if(!seqRegGcgRefId)
	seqRegGcgRefId =ajRegCompC("^>...([^ \n]+)");

    if(!seqRegGcgSplit)
	seqRegGcgSplit =ajRegCompC("_0+$");

    if(!ajFileGets(qryd->libr, &line))	/* end of file */
	return ajFalse;

    if(ajStrGetCharFirst(line) != '>')	/* not start of entry */
	ajFatal("seqGcgReadRef bad entry start:\n'%S'", line);

    if(ajStrGetCharPos(line, 3) == ';')	/* PIR entry */
	ispir = ajTrue;

    if(ispir)
	ajFileBuffLoadS(seqin->Filebuff, line);


    if(ajRegExec(seqRegGcgRefId, line))
    {
	continued = ajFalse;
	ajRegSubI(seqRegGcgRefId, 1, &id);
	if(ajRegExec(seqRegGcgSplit,id))
	{
	    continued = ajTrue;
	    p = ajStrGetuniquePtr(&id);
	    p = strrchr(p,(ajint)'_');
	    *(++p)='\0';
	    ajStrSetValid(&id);
	}
    }
    else
    {
	ajDebug("seqEmbossGcgReadRef bad ID line\n'%S'\n", line);
	ajFatal("seqEmbossGcgReadRef bad ID line\n'%S'\n", line);
    }



    if(!ajFileGets(qryd->libr, &line))	/* blank desc line */
	return ajFalse;

    if(ispir)
	ajFileBuffLoadS(seqin->Filebuff, line);


    rpos = ajFileTell(qryd->libr);
    while(ajFileGets(qryd->libr, &line))
    {
					/* end of file */
	if(ajStrGetCharFirst(line) == '>')
	{				/* start of next entry */
	    /* skip over split entries so it can be used for "all" */

	    if(continued)
	    {
		testcontinue=ajTrue;
		ajRegExec(seqRegGcgRefId, line);
		ajRegSubI(seqRegGcgRefId, 1, &idc);

		if(!ajStrPrefixS(idc,id))
		{
		    ajFileSeek(qryd->libr, rpos, 0);
		    ajStrDel(&line);
		    ajStrDel(&id);
		    ajStrDel(&idc);
		    return ajTrue;
		}
	    }
	    else
	    {
		ajFileSeek(qryd->libr, rpos, 0);
		ajStrDel(&line);
		ajStrDel(&id);
		ajStrDel(&idc);
		return ajTrue;
	    }
	}
	rpos = ajFileTell(qryd->libr);


	if(!testcontinue)
	{
	    ajStrExchangeCC(&line, ". .", "..");
	    ajFileBuffLoadS(seqin->Filebuff, line);
	}
    }


    /* at end of file */

    ajFileClose(&qryd->libr);
    ajStrDel(&line);
    ajStrDel(&id);
    ajStrDel(&idc);

    return ajTrue;
}




/* @funcstatic seqEmbossGcgReadSeq *******************************************
**
** Copies sequence data with a reformatted sequence to the "Inseq"
** data structure of the AjPSeqin object for later reuse.
**
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqEmbossGcgReadSeq(AjPSeqin seqin)
{
    AjPStr line = NULL;
    AjPSeqQuery qry;
    SeqPEmbossQry qryd;
    AjPStr gcgtype    = NULL;
    AjPStr tmpstr     = NULL;
    AjPStr dstr       = NULL;
    AjPStr id         = NULL;
    AjPStr idc        = NULL;
    AjPStr contseq    = NULL;

    ajint gcglen;
    ajint pos;
    ajint rblock;
    ajlong spos;
    AjBool ispir     = ajFalse;
    char *p = NULL;
    AjBool continued = ajFalse;


    qry  = seqin->Query;
    qryd = qry->QryData;

    if(!seqRegGcgId)
    {
	seqRegGcgId =ajRegCompC("^>...([^ ]+) +([^ ]+) +([^ ]+) +([^ ]+) +([0-9]+)");
	seqRegGcgId2=ajRegCompC("^>[PF]1;([^ ]+)");
    }

    if(!seqRegGcgSplit)
	seqRegGcgSplit =ajRegCompC("_0+$");

    ajDebug("seqEmbossGcgReadSeq pos: %Ld\n", ajFileTell(qryd->libs));

    if(!ajFileGets(qryd->libs, &line))	/* end of file */
	return ajFalse;

    ajDebug("test ID line\n'%S'\n", line);

    if(ajRegExec(seqRegGcgId, line))
    {
	continued = ajFalse;
	ajRegSubI(seqRegGcgId, 3, &gcgtype);
	ajRegSubI(seqRegGcgId, 5, &tmpstr);
	ajRegSubI(seqRegGcgId, 1, &id);
	if(ajRegExec(seqRegGcgSplit, id))
	{
	    continued = ajTrue;
	    p = ajStrGetuniquePtr(&id);
	    p = strrchr(p,(ajint)'_');
	    *(++p)='\0';
	    ajStrSetValid(&id);
	    if(!contseq)
		contseq = ajStrNew();
	    if(!dstr)
		dstr = ajStrNew();
	}

	ajStrToInt(tmpstr, &gcglen);
    }
    else if(ajRegExec(seqRegGcgId2, line))
    {
	ajStrAssignC(&gcgtype, "ASCII");
	ajRegSubI(seqRegGcgId, 1, &tmpstr);
	ispir = ajTrue;
    }
    else
    {
	ajDebug("seqEmbossGcgReadSeq bad ID line\n'%S'\n", line);
	ajFatal("seqEmbossGcgReadSeq bad ID line\n'%S'\n", line);
	return ajFalse;
    }

    if(!ajFileGets(qryd->libs, &line))	/* desc line */
	return ajFalse;

    /*
    ** need to pick up the length and type, and read to the end of sequence
    ** see fasta code to get a real sequence for this
    ** Also need to handle split entries and go find the rest
    */

    if(ispir)
    {
	spos = ajFileTell(qryd->libs);
	while(ajFileGets(qryd->libs, &line))
	{				/* end of file */
	    if(ajStrGetCharFirst(line) == '>')
	    {				/* start of next entry */
		ajFileSeek(qryd->libs, spos, 0);
		break;
	    }
	    spos = ajFileTell(qryd->libs);
	    ajFileBuffLoadS(seqin->Filebuff, line);
	}
    }
    else
    {
	ajStrSetRes(&seqin->Inseq, gcglen+3);
	rblock = gcglen;
	if(ajStrGetCharFirst(gcgtype) == '2')
	    rblock = (rblock+3)/4;

	if(!ajFileRead(ajStrGetuniquePtr(&seqin->Inseq), 1, rblock, qryd->libs))
	    ajFatal("error reading file %F", qryd->libs);

	/* convert 2bit to ascii */
	if(ajStrGetCharFirst(gcgtype) == '2')
	    seqGcgBinDecode(&seqin->Inseq, gcglen);
	else if(ajStrGetCharFirst(gcgtype) == 'A')
	{
	    /* are seq chars OK? */
	    ajStrSetValidLen(&seqin->Inseq, gcglen);
	}
	else
	{
	    ajRegSubI(seqRegGcgId, 1, &tmpstr);
	    ajFatal("Unknown GCG entry type '%S', entry name '%S'",
		    gcgtype, tmpstr);
	}

	if(!ajFileGets(qryd->libs, &line)) /* newline at end */
	    ajFatal("error reading file %F", qryd->libs);

	if(continued)
	{
	    spos = ajFileTell(qryd->libs);
	    while(ajFileGets(qryd->libs,&line))
	    {
		ajRegExec(seqRegGcgId, line);
		ajRegSubI(seqRegGcgId, 5, &tmpstr);
		ajRegSubI(seqRegGcgId, 1, &idc);

		if(!ajStrPrefixS(idc,id))
		{
		    ajFileSeek(qryd->libs, spos, 0);
		    break;
		}

		ajStrToInt(tmpstr, &gcglen);
		if(!ajFileGets(qryd->libs, &dstr)) /* desc line */
		    return ajFalse;

		ajStrSetRes(&contseq, gcglen+3);

		rblock = gcglen;
		if(ajStrGetCharFirst(gcgtype) == '2')
		    rblock = (rblock+3)/4;

		if(!ajFileRead(ajStrGetuniquePtr(&contseq), 1, rblock, qryd->libs))
		    ajFatal("error reading file %F", qryd->libs);

		/* convert 2bit to ascii */
		if(ajStrGetCharFirst(gcgtype) == '2')
		    seqGcgBinDecode(&contseq, gcglen);
		else if(ajStrGetCharFirst(gcgtype) == 'A')
		{
		    /* are seq chars OK? */
		    ajStrSetValidLen(&contseq, gcglen);
		}
		else
		{
		    ajRegSubI(seqRegGcgId, 1, &tmpstr);
		    ajFatal("Unknown GCG entry: name '%S'",
			    tmpstr);
		}

		if(!ajFileGets(qryd->libs, &line)) /* newline at end */
		    ajFatal("error reading file %F", qryd->libs);

		if(!seqRegGcgCont)
		    seqRegGcgCont = ajRegCompC("^([^ ]+) +([^ ]+) +([^ ]+) +"
					       "([^ ]+) +([^ ]+) +([^ ]+) +([^ ]+) +"
					       "([^ ]+) +([0-9]+)");
		
		ajRegExec(seqRegGcgCont, dstr);
		ajRegSubI(seqRegGcgCont, 9, &tmpstr);
		ajStrToInt(tmpstr, &pos);
		seqin->Inseq->Len = pos-1;

		ajStrAppendS(&seqin->Inseq,contseq);
		spos = ajFileTell(qryd->libs);
	    }
	}
    }
    ajStrDel(&gcgtype);
    ajStrDel(&line);
    ajStrDel(&tmpstr);
    ajStrDel(&dstr);
    ajStrDel(&id);
    ajStrDel(&idc);
    ajStrDel(&contseq);

    return ajTrue;
}




/* @funcstatic seqCdQryReuse **************************************************
**
** Tests whether Cd index query data can be reused or whether we are finished.
**
** Clears qryData structure when finished.
**
** @param [u] qry [AjPSeqQuery] Query data
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
** @@
******************************************************************************/

static AjBool seqCdQryReuse(AjPSeqQuery qry)
{
    SeqPCdQry qryd;

    qryd = qry->QryData;

    if(!qry || !qryd)
	return ajFalse;


    /*    ajDebug("qryd->list  %x\n",qryd->List);*/
    if(!qryd->List)
    {
	ajDebug("query data all finished\n");
	AJFREE(qry->QryData);
	qryd = NULL;
	return ajFalse;
    }
    else
    {
	ajDebug("reusing data from previous call %x\n", qry->QryData);
	ajDebug("listlen  %d\n", ajListGetLength(qryd->List));
	ajDebug("divfile '%S'\n", qryd->divfile);
	ajDebug("idxfile '%S'\n", qryd->idxfile);
	ajDebug("datfile '%S'\n", qryd->datfile);
	ajDebug("seqfile '%S'\n", qryd->seqfile);
	ajDebug("name    '%s'\n", qryd->name);
	ajDebug("nameSize %d\n",  qryd->nameSize);
	ajDebug("div      %d\n",  qryd->div);
	ajDebug("maxdiv   %d\n",  qryd->maxdiv);
	ajDebug("qryd->List length %d\n", ajListGetLength(qryd->List));
	/*ajListTrace(qryd->List);*/
    }

    return ajTrue;
}




/* @funcstatic seqCdQryOpen ***************************************************
**
** Opens everything for a new CD query
**
** @param [u] qry [AjPSeqQuery] Query data
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
** @@
******************************************************************************/

static AjBool seqCdQryOpen(AjPSeqQuery qry)
{
    SeqPCdQry qryd;

    ajuint i;
    short j;
    static char *name;
    AjPStr fullName = NULL;

    if(!ajStrGetLen(qry->IndexDir))
    {
	ajDebug("no indexdir defined for database '%S'\n", qry->DbName);
	ajErr("no indexdir defined for database '%S'", qry->DbName);
	return ajFalse;
    }

    ajDebug("directory '%S' entry '%S' acc '%S' hasacc:%B\n",
	    qry->IndexDir, qry->Id, qry->Acc, qry->HasAcc);

    qry->QryData = AJNEW0(qryd);
    qryd->List = ajListNew();
    AJNEW0(qryd->idxLine);
    AJNEW0(qryd->trgLine);
    qryd->dfp = seqCdFileOpen(qry->IndexDir, "division.lkp", &qryd->divfile);
    if(!qryd->dfp)
    {
	ajWarn("Cannot open division file '%S' for database '%S'",
	       qryd->divfile, qry->DbName);
	return ajFalse;
    }


    qryd->nameSize = qryd->dfp->RecSize - 2;
    qryd->maxdiv   = qryd->dfp->NRecords;
    ajDebug("nameSize: %d maxdiv: %d\n",
	    qryd->nameSize, qryd->maxdiv);
    qryd->name = ajCharNewRes(qryd->nameSize+1);
    name = ajCharNewRes(qryd->nameSize+1);
    AJCNEW0(qryd->Skip, qryd->maxdiv);
    seqCdFileSeek(qryd->dfp, 0);
    for(i=0; i < qryd->maxdiv; i++)
    {
	seqCdFileReadShort(&j, qryd->dfp);
	seqCdFileReadName(name, qryd->nameSize, qryd->dfp);

	ajStrAssignC(&fullName, name);
	ajFileNameDirSet(&fullName, qry->Directory);

	if(!ajFileTestSkip(fullName, qry->Exclude, qry->Filename,
			   ajTrue, ajTrue))
	    qryd->Skip[i] = ajTrue;
    }

    qryd->ifp = seqCdFileOpen(qry->IndexDir, "entrynam.idx", &qryd->idxfile);
    if(!qryd->ifp)
    {
	ajErr("Cannot open index file '%S'", qryd->idxfile);
	return ajFalse;
    }

    ajStrDel(&fullName);
    ajCharDel(&name);

    return ajTrue;
}




/* @funcstatic seqCdQryEntry **************************************************
**
** Queries for a single entry in an EMBLCD index
**
** @param [u] qry [AjPSeqQuery] Query data
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
** @@
******************************************************************************/

static AjBool seqCdQryEntry(AjPSeqQuery qry)
{
    SeqPCdEntry entry = NULL;
    ajint ipos = -1;
    ajint trghit;
    SeqPCdQry qryd;

    ajDebug("entry id: '%S' acc: '%S' hasacc:%B\n",
	    qry->Id, qry->Acc, qry->HasAcc);

    qryd = qry->QryData;

    if(ajStrGetLen(qry->Id))
    {					/* search by ID */
	ipos = seqCdIdxSearch(qryd->idxLine, qry->Id, qryd->ifp);
	if(ipos >= 0)
	{
	    if(!qryd->Skip[qryd->idxLine->DivCode-1])
	    {
		AJNEW0(entry);
		entry->div = qryd->idxLine->DivCode;
		entry->annoff = qryd->idxLine->AnnOffset;
		entry->seqoff = qryd->idxLine->SeqOffset;
		ajListPushAppend(qryd->List, (void*)entry);
	    }
	    else
		ajDebug("SKIP: '%S' [file %d]\n",
			qry->Id, qryd->idxLine->DivCode);
	}
    }

    if(ipos < 0 &&		     /* if needed, search by accnum */
       qry->HasAcc &&
       ajStrGetLen(qry->Acc) &&
       seqCdTrgOpen(qry->IndexDir, "acnum",
		    &qryd->trgfp, &qryd->hitfp))
    {
	trghit = seqCdTrgSearch(qryd->trgLine, qry->Acc, qryd->trgfp);
	if(trghit >= 0)
	{
	    ajint i;
	    ajint j;
	    seqCdFileSeek(qryd->hitfp, qryd->trgLine->FirstHit-1);
	    ajDebug("acnum First: %d Count: %d\n",
		    qryd->trgLine->FirstHit, qryd->trgLine->NHits);
	    ipos = qryd->trgLine->FirstHit;
	    for(i = 0; i < (ajint) qryd->trgLine->NHits; i++)
	    {
		seqCdFileReadInt(&j, qryd->hitfp);
		j--;
		ajDebug("hitlist[%d] entry = %d\n", i, j);
		seqCdIdxLine(qryd->idxLine, j, qryd->ifp);

		if(!qryd->Skip[qryd->idxLine->DivCode-1])
		{
		    AJNEW0(entry);
		    entry->div = qryd->idxLine->DivCode;
		    entry->annoff = qryd->idxLine->AnnOffset;
		    entry->seqoff = qryd->idxLine->SeqOffset;
		    ajListPushAppend(qryd->List, (void*)entry);
		}
		else
		    ajDebug("SKIP: accnum '%S' [file %d]\n",
			    qry->Acc, qryd->idxLine->DivCode);
	    }
	}
	seqCdTrgClose(&qryd->trgfp, &qryd->hitfp);
	ajStrDel(&qryd->trgLine->Target);
    }

    /*
     ** if needed, search by SeqVersion
     */

    if(ipos < 0 &&
       ajStrGetLen(qry->Sv) &&
       seqCdTrgOpen(qry->IndexDir, "seqvn",
		    &qryd->trgfp, &qryd->hitfp))
    {
	trghit = seqCdTrgSearch(qryd->trgLine, qry->Sv, qryd->trgfp);
	if(trghit >= 0)
	{
	    ajint i;
	    ajint j;
	    seqCdFileSeek(qryd->hitfp, qryd->trgLine->FirstHit-1);
	    ajDebug("seqvn First: %d Count: %d\n",
		    qryd->trgLine->FirstHit, qryd->trgLine->NHits);
	    ipos = qryd->trgLine->FirstHit;
	    for(i = 0; i < (ajint) qryd->trgLine->NHits; i++)
	    {
		seqCdFileReadInt(&j, qryd->hitfp);
		j--;
		ajDebug("hitlist[%d] entry = %d\n", i, j);
		seqCdIdxLine(qryd->idxLine, j, qryd->ifp);

		if(!qryd->Skip[qryd->idxLine->DivCode-1])
		{
		    AJNEW0(entry);
		    entry->div = qryd->idxLine->DivCode;
		    entry->annoff = qryd->idxLine->AnnOffset;
		    entry->seqoff = qryd->idxLine->SeqOffset;
		    ajListPushAppend(qryd->List, (void*)entry);
		}
		else
		    ajDebug("SKIP: seqvn '%S' [file %d]\n",
			    qry->Sv, qryd->idxLine->DivCode);
	    }
	}
	seqCdTrgClose(&qryd->trgfp, &qryd->hitfp);
	ajStrDel(&qryd->trgLine->Target);
    }

    /*
     ** if needed, search by SeqVersion
     */

    if(ipos < 0 &&
       ajStrGetLen(qry->Gi) &&
       seqCdTrgOpen(qry->IndexDir, "gi",
		    &qryd->trgfp, &qryd->hitfp))
    {
	trghit = seqCdTrgSearch(qryd->trgLine, qry->Gi, qryd->trgfp);
	if(trghit >= 0)
	{
	    ajint i;
	    ajint j;
	    seqCdFileSeek(qryd->hitfp, qryd->trgLine->FirstHit-1);
	    ajDebug("seqvn First: %d Count: %d\n",
		    qryd->trgLine->FirstHit, qryd->trgLine->NHits);
	    ipos = qryd->trgLine->FirstHit;
	    for(i = 0; i < (ajint) qryd->trgLine->NHits; i++)
	    {
		seqCdFileReadInt(&j, qryd->hitfp);
		j--;
		ajDebug("hitlist[%d] entry = %d\n", i, j);
		seqCdIdxLine(qryd->idxLine, j, qryd->ifp);

		if(!qryd->Skip[qryd->idxLine->DivCode-1])
		{
		    AJNEW0(entry);
		    entry->div = qryd->idxLine->DivCode;
		    entry->annoff = qryd->idxLine->AnnOffset;
		    entry->seqoff = qryd->idxLine->SeqOffset;
		    ajListPushAppend(qryd->List, (void*)entry);
		}
		else
		    ajDebug("SKIP: seqvn '%S' [file %d]\n",
			    qry->Gi, qryd->idxLine->DivCode);
	    }
	}
	seqCdTrgClose(&qryd->trgfp, &qryd->hitfp);
	ajStrDel(&qryd->trgLine->Target);
    }

    /*
     ** if needed, search by Description
     */

    if(ipos < 0 &&
       ajStrGetLen(qry->Des) &&
       seqCdTrgOpen(qry->IndexDir, "des",
		    &qryd->trgfp, &qryd->hitfp))
    {
	trghit = seqCdTrgSearch(qryd->trgLine, qry->Des, qryd->trgfp);
	if(trghit >= 0)
	{
	    ajint i;
	    ajint j;
	    seqCdFileSeek(qryd->hitfp, qryd->trgLine->FirstHit-1);
	    ajDebug("des First: %d Count: %d\n",
		    qryd->trgLine->FirstHit, qryd->trgLine->NHits);
	    ipos = qryd->trgLine->FirstHit;
	    for(i = 0; i < (ajint) qryd->trgLine->NHits; i++)
	    {
		seqCdFileReadInt(&j, qryd->hitfp);
		j--;
		ajDebug("hitlist[%d] entry = %d\n", i, j);
		seqCdIdxLine(qryd->idxLine, j, qryd->ifp);

		if(!qryd->Skip[qryd->idxLine->DivCode-1])
		{
		    AJNEW0(entry);
		    entry->div = qryd->idxLine->DivCode;
		    entry->annoff = qryd->idxLine->AnnOffset;
		    entry->seqoff = qryd->idxLine->SeqOffset;
		    ajListPushAppend(qryd->List, (void*)entry);
		}
		else
		    ajDebug("SKIP: des '%S' [file %d]\n",
			    qry->Des, qryd->idxLine->DivCode);
	    }
	}
	seqCdTrgClose(&qryd->trgfp, &qryd->hitfp);
	ajStrDel(&qryd->trgLine->Target);
    }

    /*
     ** if needed, search by Keyword
     */

    if(ipos < 0 &&
       ajStrGetLen(qry->Key) &&
       seqCdTrgOpen(qry->IndexDir, "keyword",
		    &qryd->trgfp, &qryd->hitfp))
    {
	trghit = seqCdTrgSearch(qryd->trgLine, qry->Key, qryd->trgfp);
	if(trghit >= 0)
	{
	    ajint i;
	    ajint j;
	    seqCdFileSeek(qryd->hitfp, qryd->trgLine->FirstHit-1);
	    ajDebug("key First: %d Count: %d\n",
		    qryd->trgLine->FirstHit, qryd->trgLine->NHits);
	    ipos = qryd->trgLine->FirstHit;
	    for(i = 0; i < (ajint) qryd->trgLine->NHits; i++)
	    {
		seqCdFileReadInt(&j, qryd->hitfp);
		j--;
		ajDebug("hitlist[%d] entry = %d\n", i, j);
		seqCdIdxLine(qryd->idxLine, j, qryd->ifp);

		if(!qryd->Skip[qryd->idxLine->DivCode-1])
		{
		    AJNEW0(entry);
		    entry->div = qryd->idxLine->DivCode;
		    entry->annoff = qryd->idxLine->AnnOffset;
		    entry->seqoff = qryd->idxLine->SeqOffset;
		    ajListPushAppend(qryd->List, (void*)entry);
		}
		else
		    ajDebug("SKIP: key '%S' [file %d]\n",
			    qry->Key, qryd->idxLine->DivCode);
	    }
	}
	seqCdTrgClose(&qryd->trgfp, &qryd->hitfp);
	ajStrDel(&qryd->trgLine->Target);
    }

    /*
     ** if needed, search by Taxonomy
     */

    if(ipos < 0 &&
       ajStrGetLen(qry->Org) &&
       seqCdTrgOpen(qry->IndexDir, "taxon",
		    &qryd->trgfp, &qryd->hitfp))
    {
	trghit = seqCdTrgSearch(qryd->trgLine, qry->Org, qryd->trgfp);
	if(trghit >= 0)
	{
	    ajint i;
	    ajint j;
	    seqCdFileSeek(qryd->hitfp, qryd->trgLine->FirstHit-1);
	    ajDebug("tax First: %d Count: %d\n",
		    qryd->trgLine->FirstHit, qryd->trgLine->NHits);
	    ipos = qryd->trgLine->FirstHit;
	    for(i = 0; i < (ajint) qryd->trgLine->NHits; i++)
	    {
		seqCdFileReadInt(&j, qryd->hitfp);
		j--;
		ajDebug("hitlist[%d] entry = %d\n", i, j);
		seqCdIdxLine(qryd->idxLine, j, qryd->ifp);

		if(!qryd->Skip[qryd->idxLine->DivCode-1])
		{
		    AJNEW0(entry);
		    entry->div = qryd->idxLine->DivCode;
		    entry->annoff = qryd->idxLine->AnnOffset;
		    entry->seqoff = qryd->idxLine->SeqOffset;
		    ajListPushAppend(qryd->List, (void*)entry);
		}
		else
		    ajDebug("SKIP: tax '%S' [file %d]\n",
			    qry->Org, qryd->idxLine->DivCode);
	    }
	}
	seqCdTrgClose(&qryd->trgfp, &qryd->hitfp);
	ajStrDel(&qryd->trgLine->Target);
    }

    if(ipos < 0)
	return ajFalse;
    if(!ajListGetLength(qryd->List))
	return ajFalse;

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    return ajTrue;
}




/* @funcstatic seqCdQryQuery **************************************************
**
** Queries for one or more entries in an EMBLCD index
**
** @param [u] qry [AjPSeqQuery] Query data
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
** @@
******************************************************************************/

static AjBool seqCdQryQuery(AjPSeqQuery qry)
{
    SeqPCdQry qryd;

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    if(ajStrGetLen(qry->Id))
	if(seqCdIdxQuery(qry))
	{
	    qryd = qry->QryData;
	    ajListSortUnique(qryd->List, seqCdEntryCmp, seqCdEntryDel);
	    return ajTrue;
	}

    if(ajStrGetLen(qry->Acc) ||
       ajStrGetLen(qry->Sv) ||
       ajStrGetLen(qry->Gi) ||
       ajStrGetLen(qry->Des) ||
       ajStrGetLen(qry->Key) ||
       ajStrGetLen(qry->Org))
    {
	if(!seqCdTrgQuery(qry))
	    return ajFalse;
	qryd = qry->QryData;
	ajListSortUnique(qryd->List, seqCdEntryCmp, seqCdEntryDel);
	return ajTrue;
    }


    return ajFalse;
}




/* @funcstatic seqCdEntryCmp **************************************************
**
** Compares two SeqPEntry objects
**
** @param [r] pa [const void*] SeqPEntry object
** @param [r] pb [const void*] SeqPEntry object
** @return [int] -1 if first entry should sort before second, +1 if the
**         second entry should sort first. 0 if they are identical
** @@
******************************************************************************/
static int seqCdEntryCmp(const void* pa, const void* pb)
{
    const SeqPCdEntry a;
    const SeqPCdEntry b;

    a = *(SeqPCdEntry const *) pa;
    b = *(SeqPCdEntry const *) pb;

    ajDebug("seqCdEntryCmp %x %d %d : %x %d %d\n",
	     a, a->div, a->annoff,
	     b, b->div, b->annoff);

    if(a->div != b->div)
	return (a->div - b->div);

    return (a->annoff - b->annoff);
}




/* @funcstatic seqCdEntryDel***************************************************
**
** Deletes a SeqPCdEntry object
**
** @param [r] pentry [void**] Address of a SeqPCdEntry object
** @param [r] cl [void*] Standard unused argument, usually NULL.
** @return [void]
** @@
******************************************************************************/
static void seqCdEntryDel(void** pentry, void* cl)
{
    (void) cl;

    AJFREE(*pentry);

    return;
}




/* @funcstatic seqCdQryNext ***************************************************
**
** Processes the next query for an EMBLCD index
**
** @param [u] qry [AjPSeqQuery] Query data
** @return [AjBool] ajTrue if successful
** @@
******************************************************************************/

static AjBool seqCdQryNext(AjPSeqQuery qry)
{
    SeqPCdEntry entry;
    SeqPCdQry qryd;
    void* item;

    qryd = qry->QryData;

    if(!ajListGetLength(qryd->List))
	return ajFalse;

    ajDebug("qryd->List (b) length %d\n", ajListGetLength(qryd->List));
    /*ajListTrace(qryd->List);*/
    ajListPop(qryd->List, &item);
    entry = (SeqPCdEntry) item;

    ajDebug("entry: %x div: %d (%d) ann: %d seq: %d\n",
	    entry, entry->div, qryd->div, entry->annoff, entry->seqoff);

    qryd->idnum = entry->annoff - 1;

    ajDebug("idnum: %d\n", qryd->idnum);

    qryd->Samefile = ajTrue;
    if(entry->div != qryd->div)
    {
	qryd->Samefile = ajFalse;
	qryd->div = entry->div;
	ajDebug("div: %d\n", qryd->div);
	if(!seqCdQryFile(qry))
	    return ajFalse;
    }

    ajDebug("Offsets(cd) %d %d\n", entry->annoff, entry->seqoff);
    ajDebug("libr %x\n", qryd->libr);
    ajDebug("libr %F\n", qryd->libr);

    ajFileSeek(qryd->libr, entry->annoff,0);
    if(qryd->libs)
	ajFileSeek(qryd->libs, entry->seqoff,0);

    AJFREE(entry);

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    return ajTrue;
}




/* @funcstatic seqBlastQryNext ************************************************
**
** Processes the next query for an EMBLCD index for a Blast index
**
** @param [u] qry [AjPSeqQuery] Query data
** @return [AjBool] ajTrue if successful
** @@
******************************************************************************/

static AjBool seqBlastQryNext(AjPSeqQuery qry)
{
    SeqPCdEntry entry;
    SeqPCdQry qryd;
    void* item;

    qryd = qry->QryData;

    if(!ajListGetLength(qryd->List))
	return ajFalse;

    ajDebug("seqBlastQryNext qryd %x qryd->List (c) length: %d\n",
	    qryd, ajListGetLength(qryd->List));

    /* ajListTrace(qryd->List);*/

    ajListPop(qryd->List, &item);
    entry = (SeqPCdEntry) item;

    ajDebug("entry: %X div: %d (%d) ann: %d seq: %d\n",
	    entry, entry->div, qryd->div, entry->annoff, entry->seqoff);

    if(entry->div != qryd->div)
    {
	qryd->div = entry->div;
	seqBlastOpen(qry, ajFalse);	/* replaces qry->QryData */
	qryd = qry->QryData;
    }
    qryd->idnum = entry->annoff - 1;

    ajDebug("Offsets (blast) %d %d [%d] qryd: %x\n",
	    entry->annoff, entry->seqoff, qryd->idnum, qryd);

    /* entry->annoff as qryd->idnum sets table position */

    AJFREE(entry);

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    return ajTrue;
}




/* @funcstatic seqCdQryClose **************************************************
**
** Closes query data for an EMBLCD index
**
** @param [u] qry [AjPSeqQuery] Query data
** @return [AjBool] ajTrue if all is done
** @@
******************************************************************************/

static AjBool seqCdQryClose(AjPSeqQuery qry)
{
    SeqPCdQry qryd = NULL;

    ajDebug("seqCdQryClose clean up qryd\n");

    qryd = qry->QryData;
    if(!qryd)
	return ajTrue;

    ajCharDel(&qryd->name);
    ajStrDel(&qryd->divfile);
    ajStrDel(&qryd->idxfile);
    ajStrDel(&qryd->datfile);
    ajStrDel(&qryd->seqfile);
    ajStrDel(&qryd->srcfile);
    ajStrDel(&qryd->tblfile);

    seqCdIdxDel(&qryd->idxLine);
    seqCdTrgDel(&qryd->trgLine);

    seqCdFileClose(&qryd->ifp);
    seqCdFileClose(&qryd->dfp);
    /* defined in a buffer, cleared there */
    /*
    ajFileClose(&qryd->libr);
    ajFileClose(&qryd->libs);
    */
    qryd->libr=0;
    qryd->libs=0;
    ajListFree(&qryd->List);
    AJFREE(qryd->trgLine);
    AJFREE(qryd->idxLine);
    AJFREE(qryd->Skip);

    /* keep QryData for use at top of loop */

    return ajTrue;
}




/* @funcstatic seqBlastQryClose ***********************************************
**
** Closes query data for an EMBLCD blastdb index
**
** @param [u] qry [AjPSeqQuery] Query data
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
** @@
******************************************************************************/

static AjBool seqBlastQryClose(AjPSeqQuery qry)
{
    SeqPCdQry qryd;

    ajDebug("seqBlastQryClose clean up qryd\n");

    qryd = qry->QryData;

    ajCharDel(&qryd->name);
    ajStrDel(&qryd->divfile);
    ajStrDel(&qryd->idxfile);
    ajStrDel(&qryd->datfile);
    ajStrDel(&qryd->seqfile);
    ajStrDel(&qryd->srcfile);
    ajStrDel(&qryd->tblfile);
    ajStrDel(&qryd->idxLine->EntryName);

    seqCdFileClose(&qryd->ifp);
    seqCdFileClose(&qryd->dfp);

    ajFileClose(&qryd->libr);
    ajFileClose(&qryd->libs);

    ajFileClose(&qryd->libt);
    ajFileClose(&qryd->libf);

    ajListFree(&qryd->List);
    AJFREE(qryd->trgLine);
    AJFREE(qryd->idxLine);
    AJFREE(qryd->Skip);

    /* keep QryData for use at top of loop */

    return ajTrue;
}




/* @section GCG Database Indexing *********************************************
**
** These functions manage the GCG index access methods.
**
******************************************************************************/




/* @funcstatic seqAccessGcg ***************************************************
**
** Reads sequence(s) from a GCG formatted database, using EMBLCD index
** files. Returns with the file pointer set to the position in the
** sequence file.
**
** @param [u] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqAccessGcg(AjPSeqin seqin)
{
    AjBool retval = ajFalse;
    AjPSeqQuery qry;
    SeqPCdQry qryd;
    static ajint qrycalled = 0;

    ajDebug("seqAccessGcg type %d\n", seqin->Query->Type);

    qry  = seqin->Query;
    qryd = qry->QryData;

    if(qry->Type == QRY_ALL)
    {
      retval = seqGcgAll(seqin);
      return retval;
    }

    /* we need to search the index files and return a query */

    if(!qrycalled)
    {
	if(ajUtilGetBigendian())
	    seqCdReverse = ajTrue;
	qrycalled = 1;
    }

    if(qry->QryData)
    {				     /* reuse unfinished query data */
	if(!seqCdQryReuse(qry))
	    return ajFalse;
    }
    else
    {
	seqin->Single = ajTrue;

	if(!seqCdQryOpen(qry))
	    ajFatal("seqCdQryOpen failed");

	qryd = qry->QryData;
	ajFileBuffDel(&seqin->Filebuff);
	seqin->Filebuff = ajFileBuffNew();

	/* binary search for the entryname we need */

	if(qry->Type == QRY_ENTRY)
	{
	    ajDebug("entry id: '%S' acc: '%S' hasacc:%B\n",
		    qry->Id, qry->Acc, qry->HasAcc);
	    if(!seqCdQryEntry(qry))
		ajDebug("GCG Entry failed\n");
	}

	if(qry->Type == QRY_QUERY)
	{
	    ajDebug("query id: '%S' acc: '%S' hasacc:%B\n",
		    qry->Id, qry->Acc, qry->HasAcc);
	    if(!seqCdQryQuery(qry))
		ajDebug("GCG Query failed\n");
	}
	AJFREE(qryd->trgLine);
    }

    if(ajListGetLength(qryd->List))
    {
	retval = seqCdQryNext(qry);
	if(retval)
	    seqGcgLoadBuff(seqin);
    }

    if(!ajListGetLength(qryd->List))
    {
	ajFileClose(&qryd->libr);
	ajFileClose(&qryd->libs);
	seqCdQryClose(qry);
    }

    if(retval)
	ajStrAssignS(&seqin->Db, qry->DbName);

    return retval;
}




/* @funcstatic seqGcgLoadBuff *************************************************
**
** Copies text data to a buffered file, and sequence data for an
** AjPSeqin internal data structure for reading later
**
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [void]
** @@
******************************************************************************/

static void seqGcgLoadBuff(AjPSeqin seqin)
{
    AjPSeqQuery qry;
    SeqPCdQry qryd;

    qry  = seqin->Query;
    qryd = qry->QryData;

    if(!qry->QryData)
	ajFatal("seqGcgLoadBuff Query Data not initialised");

    /* copy all the ref data */

    seqGcgReadRef(seqin);

    /* write the sequence (do we care about the format?) */
    seqGcgReadSeq(seqin);

    /* ajFileBuffTraceFull(seqin->Filebuff, 9999, 100); */

    if(!qryd->libr)
    {
      ajFileClose(&qryd->libs);
      ajDebug("seqGcgLoadBuff: closed files\n");
    }

    return;
}




/* @funcstatic seqGcgReadRef **************************************************
**
** Copies text data to a buffered file for reading later
**
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqGcgReadRef(AjPSeqin seqin)
{
    AjPStr line = NULL;
    AjPSeqQuery qry;
    SeqPCdQry qryd;
    ajlong rpos;
    AjPStr id       = NULL;
    AjPStr idc      = NULL;
    AjBool ispir           = ajFalse;
    AjBool continued       = ajFalse;
    AjBool testcontinue    = ajFalse;
    char *p = NULL;

    qry  = seqin->Query;
    qryd = qry->QryData;

    if(!seqRegGcgRefId)
	seqRegGcgRefId =ajRegCompC("^>...([^ \n]+)");

    if(!seqRegGcgSplit)
	seqRegGcgSplit =ajRegCompC("_0+$");

    if(!ajFileGets(qryd->libr, &line))	/* end of file */
	return ajFalse;

    if(ajStrGetCharFirst(line) != '>')	/* not start of entry */
	ajFatal("seqGcgReadRef bad entry start:\n'%S'", line);

    if(ajStrGetCharPos(line, 3) == ';')	/* PIR entry */
	ispir = ajTrue;

    if(ispir)
	ajFileBuffLoadS(seqin->Filebuff, line);

    if(ajRegExec(seqRegGcgRefId, line))
    {
	continued = ajFalse;
	ajRegSubI(seqRegGcgRefId, 1, &id);
	if(ajRegExec(seqRegGcgSplit, id))
	{
	    continued = ajTrue;
	    p = ajStrGetuniquePtr(&id);
	    p = strrchr(p,(ajint)'_');
	    *(++p)='\0';
	    ajStrSetValid(&id);
	}
    }
    else
    {
	ajDebug("seqGcgReadRef bad ID line\n'%S'\n", line);
	ajFatal("seqGcgReadRef bad ID line\n'%S'\n", line);
    }

    if(!ajFileGets(qryd->libr, &line))	/* blank desc line */
    {
      ajStrDel(&id);
      return ajFalse;
    }

    if(ispir)
	ajFileBuffLoadS(seqin->Filebuff, line);

    rpos = ajFileTell(qryd->libr);
    while(ajFileGets(qryd->libr, &line))
    {					/* end of file */
	if(ajStrGetCharFirst(line) == '>')
	{				/* start of next entry */
	    /* skip over split entries so it can be used for "all" */

	    if(continued)
	    {
		testcontinue=ajTrue;
		ajRegExec(seqRegGcgRefId, line);
		ajRegSubI(seqRegGcgRefId, 1, &idc);

		if(!ajStrPrefixS(idc,id))
		{
		    ajFileSeek(qryd->libr, rpos, 0);
		    ajStrDel(&line);
		    ajStrDel(&id);
		    ajStrDel(&idc);
		    return ajTrue;
		}
	    }
	    else
	    {
		ajFileSeek(qryd->libr, rpos, 0);
		ajStrDel(&line);
		ajStrDel(&id);
		ajStrDel(&idc);
		return ajTrue;
	    }
	}
	rpos = ajFileTell(qryd->libr);
	if(!testcontinue)
	{
	    ajStrExchangeCC(&line, ". .", "..");
	    ajFileBuffLoadS(seqin->Filebuff, line);
	}
    }


    /* at end of file */

    ajFileClose(&qryd->libr);

    ajStrDel(&line);
    ajStrDel(&id);
    ajStrDel(&idc);
    return ajTrue;
}




/* @funcstatic seqGcgReadSeq **************************************************
**
** Copies sequence data with a reformatted sequence to the "Inseq"
** data structure of the AjPSeqin object for later reuse.
**
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqGcgReadSeq(AjPSeqin seqin)
{
    AjPStr line = NULL;
    AjPSeqQuery qry;
    SeqPCdQry qryd;
    AjPStr gcgtype    = NULL;
    AjPStr tmpstr     = NULL;
    AjPStr dstr       = NULL;
    AjPStr id         = NULL;
    AjPStr idc        = NULL;
    AjPStr contseq    = NULL;

    ajint gcglen;
    ajint pos;
    ajint rblock;
    ajlong spos;
    AjBool ispir     = ajFalse;
    char *p = NULL;
    AjBool continued = ajFalse;


    qry  = seqin->Query;
    qryd = qry->QryData;

    if(!seqRegGcgId)
    {
	seqRegGcgId =ajRegCompC("^>...([^ ]+) +([^ ]+) +([^ ]+) +([^ ]+) +([0-9]+)");
	seqRegGcgId2=ajRegCompC("^>[PF]1;([^ ]+)");
    }

    if(!seqRegGcgSplit)
	seqRegGcgSplit =ajRegCompC("_0+$");

    ajDebug("seqGcgReadSeq pos: %Ld\n", ajFileTell(qryd->libs));

    if(!ajFileGets(qryd->libs, &line))	/* end of file */
	return ajFalse;

    ajDebug("test ID line\n'%S'\n", line);

    if(ajRegExec(seqRegGcgId, line))
    {
	continued = ajFalse;
	ajRegSubI(seqRegGcgId, 3, &gcgtype);
	ajRegSubI(seqRegGcgId, 5, &tmpstr);
	ajRegSubI(seqRegGcgId, 1, &id);
	if(ajRegExec(seqRegGcgSplit, id))
	{
	    continued = ajTrue;
	    p = ajStrGetuniquePtr(&id);
	    p = strrchr(p,(ajint)'_');
	    *(++p)='\0';
	    ajStrSetValid(&id);
	    if(!contseq)
		contseq = ajStrNew();
	    if(!dstr)
		dstr = ajStrNew();
	}

	ajStrToInt(tmpstr, &gcglen);
    }
    else if(ajRegExec(seqRegGcgId2, line))
    {
	ajStrAssignC(&gcgtype, "ASCII");
	ajRegSubI(seqRegGcgId, 1, &tmpstr);
	ispir = ajTrue;
    }
    else
    {
	ajDebug("seqGcgReadSeq bad ID line\n'%S'\n", line);
	ajFatal("seqGcgReadSeq bad ID line\n'%S'\n", line);
	return ajFalse;
    }

    if(!ajFileGets(qryd->libs, &line))	/* desc line */
	return ajFalse;

    /*
    ** need to pick up the length and type, and read to the end of sequence
    ** see fasta code to get a real sequence for this
    ** Also need to handle split entries and go find the rest
    */

    if(ispir)
    {
	spos = ajFileTell(qryd->libs);
	while(ajFileGets(qryd->libs, &line))
	{				/* end of file */
	    if(ajStrGetCharFirst(line) == '>')
	    {				/* start of next entry */
		ajFileSeek(qryd->libs, spos, 0);
		break;
	    }
	    spos = ajFileTell(qryd->libs);
	    ajFileBuffLoadS(seqin->Filebuff, line);
	}
    }
    else
    {
	ajStrSetRes(&seqin->Inseq, gcglen+3);
	rblock = gcglen;
	if(ajStrGetCharFirst(gcgtype) == '2')
	    rblock = (rblock+3)/4;

	if(!ajFileRead(ajStrGetuniquePtr(&seqin->Inseq), 1, rblock, qryd->libs))
	    ajFatal("error reading file %F", qryd->libs);

	/* convert 2bit to ascii */
	if(ajStrGetCharFirst(gcgtype) == '2')
	    seqGcgBinDecode(&seqin->Inseq, gcglen);
	else if(ajStrGetCharFirst(gcgtype) == 'A')
	{
	    /* are seq chars OK? */
	    ajStrSetValidLen(&seqin->Inseq, gcglen);
	}
	else
	{
	    ajRegSubI(seqRegGcgId, 1, &tmpstr);
	    ajFatal("Unknown GCG entry type '%S', entry name '%S'",
		    gcgtype, tmpstr);
	}

	if(!ajFileGets(qryd->libs, &line)) /* newline at end */
	    ajFatal("error reading file %F", qryd->libs);

	if(continued)
	{
	    spos = ajFileTell(qryd->libs);
	    while(ajFileGets(qryd->libs,&line))
	    {
		ajRegExec(seqRegGcgId, line);
		ajRegSubI(seqRegGcgId, 5, &tmpstr);
		ajRegSubI(seqRegGcgId, 1, &idc);

		if(!ajStrPrefixS(idc,id))
		{
		    ajFileSeek(qryd->libs, spos, 0);
		    break;
		}

		ajStrToInt(tmpstr, &gcglen);
		if(!ajFileGets(qryd->libs, &dstr)) /* desc line */
		    return ajFalse;

		ajStrSetRes(&contseq, gcglen+3);

		rblock = gcglen;
		if(ajStrGetCharFirst(gcgtype) == '2')
		    rblock = (rblock+3)/4;

		if(!ajFileRead(ajStrGetuniquePtr(&contseq), 1, rblock, qryd->libs))
		    ajFatal("error reading file %F", qryd->libs);

		/* convert 2bit to ascii */
		if(ajStrGetCharFirst(gcgtype) == '2')
		    seqGcgBinDecode(&contseq, gcglen);
		else if(ajStrGetCharFirst(gcgtype) == 'A')
		{
		    /* are seq chars OK? */
		    ajStrSetValidLen(&contseq, gcglen);
		}
		else
		{
		    ajRegSubI(seqRegGcgId, 1, &tmpstr);
		    ajFatal("Unknown GCG entry: name '%S'",
			    tmpstr);
		}

		if(!ajFileGets(qryd->libs, &line)) /* newline at end */
		    ajFatal("error reading file %F", qryd->libs);

		if(!seqRegGcgCont)
		    seqRegGcgCont = ajRegCompC("^([^ ]+) +([^ ]+) +([^ ]+) +"
					       "([^ ]+) +([^ ]+) +([^ ]+) +([^ ]+) +"
					       "([^ ]+) +([0-9]+)");

		ajRegExec(seqRegGcgCont, dstr);
		ajRegSubI(seqRegGcgCont, 9, &tmpstr);
		ajStrToInt(tmpstr, &pos);
		seqin->Inseq->Len = pos-1;

		ajStrAppendS(&seqin->Inseq,contseq);
		spos = ajFileTell(qryd->libs);
	    }
	}
    }

    ajStrDel(&line);
    ajStrDel(&gcgtype);
    ajStrDel(&tmpstr);
    ajStrDel(&dstr);
    ajStrDel(&id);
    ajStrDel(&idc);
    ajStrDel(&contseq);

    return ajTrue;
}




/* @funcstatic seqGcgBinDecode ************************************************
**
** Convert GCG binary to ASCII sequence.
**
** @param [u] pthis [AjPStr*] Binary string
** @param [r] sqlen [ajuint] Expected sequence length
** @return [void]
** @@
******************************************************************************/

static void seqGcgBinDecode(AjPStr *pthis, ajuint sqlen)
{
    char* seqp;
    char* cp;
    char* start;
    const char* gcgbton="CTAG";
    char stmp;
    ajint rdlen;

    start = ajStrGetuniquePtr(pthis);
    rdlen = (sqlen+3)/4;

    seqp = start + rdlen;
    cp = start + 4*rdlen;

    ajDebug("seqp:%x start:%x cp:%x sqlen:%d len:%d size:%d (seqp-start):%d\n",
	    seqp, start, cp, sqlen,
	    ajStrGetLen(*pthis), ajStrGetRes(*pthis),
	    (seqp - start));

    while(seqp > start)
    {
	stmp = *--seqp;
	*--cp = gcgbton[stmp&3];
	*--cp = gcgbton[(stmp >>= 2)&3];
	*--cp = gcgbton[(stmp >>= 2)&3];
	*--cp = gcgbton[(stmp >>= 2)&3];
    }

    start[sqlen] = '\0';
    ajStrSetValidLen(pthis, sqlen);

    return;
}




/* @funcstatic seqGcgAll ******************************************************
**
** Opens the first or next GCG file for further reading
**
** @param [u] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqGcgAll(AjPSeqin seqin)
{
    AjPSeqQuery qry;
    SeqPCdQry qryd;
    static ajint called = 0;

    qry = seqin->Query;
    qryd = qry->QryData;

    if(!called)
    {
	if(ajUtilGetBigendian())
	    seqCdReverse = ajTrue;
	called = 1;
    }

    ajDebug("seqGcgAll\n");

    if(!qry->QryData)
    {
	ajDebug("seqGcgAll initialising\n");
	seqin->Single = ajTrue;
	if(!seqCdQryOpen(qry))
	{
	    ajErr("seqGcgAll failed");
	    return ajFalse;
	}
    }

    qryd = qry->QryData;
    ajFileBuffDel(&seqin->Filebuff);
    seqin->Filebuff = ajFileBuffNew();

    if(!qryd->libr)
    {
	if(!seqCdDivNext(qry))
	{
	    seqCdQryClose(qry);
	    ajDebug("seqGcgAll finished\n");
	    return ajFalse;
	}
	if(!seqCdQryFile(qry))
	{
	    ajErr("seqGcgAll out of data");
	    return ajFalse;
	}
	ajDebug("seqCdQryOpen processing file %2d '%F'\n", qryd->div,
		qryd->libr);
	if(qryd->libs)
	    ajDebug("               sequence file    '%F'\n", qryd->libs);
    }
    seqGcgLoadBuff(seqin);

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    return ajTrue;
}




/* @section BLAST Database Indexing *******************************************
**
** These functions manage the BLAST index access methods.
**
******************************************************************************/




/* @funcstatic seqAccessBlast *************************************************
**
** Reads sequence(s) using BLAST index files. Returns with the file pointer
** set to the position in the sequence file.
**
** @param [u] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqAccessBlast(AjPSeqin seqin)
{
    AjBool retval = ajFalse;
    AjPSeqQuery qry;
    SeqPCdQry qryd;
    static ajint qrycalled = 0;


    ajDebug("seqAccessBlast type %d\n", seqin->Query->Type);

    qry  = seqin->Query;
    qryd = qry->QryData;

    if(qry->Type == QRY_ALL)
	return seqBlastAll(seqin);

    /* we need to search the index files and return a query */

    if(!qrycalled)
    {
	if(ajUtilGetBigendian())
	    seqCdReverse = ajTrue;
	qrycalled = 1;
    }

    if(qry->QryData)
    {				     /* reuse unfinished query data */
	if(!seqCdQryReuse(qry))
	    return ajFalse;
    }
    else
    {
	seqin->Single = ajTrue;

	if(!seqCdQryOpen(qry))		/* open the table file */
	{
	    ajErr("seqCdQryOpen failed");
	    return ajFalse;
	}

	qryd = qry->QryData;
	ajFileBuffDel(&seqin->Filebuff);
	seqin->Filebuff = ajFileBuffNew();

	/* binary search for the entryname we need */

	if(qry->Type == QRY_ENTRY)
	{
	    ajDebug("entry id: '%S' acc: '%S' hasacc:%B\n",
		    qry->Id, qry->Acc, qry->HasAcc);
	    if(!seqCdQryEntry(qry))
	    {
		ajDebug("BLAST Entry failed\n");
	    }
	}

	if(qry->Type == QRY_QUERY)
	{
	    ajDebug("query id: '%S' acc: '%S' hasacc:%B\n",
		    qry->Id, qry->Acc, qry->HasAcc);
	    if(!seqCdQryQuery(qry))
	    {
		ajDebug("BLAST Query failed\n");
	    }
	}
    }

    if(ajListGetLength(qryd->List))
    {
	retval = seqBlastQryNext(qry);	/* changes qry->QryData */
	qryd = qry->QryData;
	if(retval)
	    seqBlastLoadBuff(seqin);
    }

    if(!ajListGetLength(qryd->List))
    {
	ajFileClose(&qryd->libr);
	ajFileClose(&qryd->libs);
	ajFileClose(&qryd->libt);
	ajFileClose(&qryd->libf);
	seqCdQryClose(qry);
	if((qry->Type == QRY_ENTRY) && !seqin->multi)
	{
	    AJFREE(qry->QryData);
	    qryd = NULL;
	}
    }

    ajStrAssignS(&seqin->Db, qry->DbName);

    return retval;
}




/* @funcstatic seqBlastOpen ***************************************************
**
** Opens a blast database. The query object can specify protein or DNA type.
** The blast version (1 or 2) is derived from the table file name.
**
** @param [u] qry [AjPSeqQuery] Sequence query object
** @param [r] next [AjBool] Skip to next file (when reading all entries)
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqBlastOpen(AjPSeqQuery qry, AjBool next)
{
    static const char* seqext[] = {"bsq", "csq", "psq", "nsq"};
    static const char* hdrext[] = {"ahd", "nhd", "phr", "nhr"};
    static const char* tblext[] = {"atb", "ntb", "pin", "nin"};

    short j;
    AjBool isblast2 = ajFalse;
    AjBool isdna    = ajFalse;
    ajint rdtmp  = 0;
    ajint rdtmp2 = 0;

    ajint DbType;			/* database type indicator */
    ajint DbFormat;	                /* (version) indicator */
    ajint TitleLen;			/* length of database title */
    ajint DateLen;		        /* length of database date string */
    ajint LineLen;			/* length of database lines */
    ajint HeaderLen;		        /* bytes before tables start */
    ajint Size;			        /* number of database entries */
    ajint CompLen;		        /* length of compressed seq file */
    ajint MaxSeqLen;			/* max. entry length */
    ajint TotLen;	                /* bases or residues in database */
    ajint CleanCount;			/* count of cleaned 8mers */
    AjPStr Title=NULL;			/* database title */
    AjPStr Date=NULL;			/* database date */

    SeqPCdQry qryd;
    AjBool bigend = ajTrue;	        /* Blast indices are bigendian */

    if(!seqBlastDivExp)
	seqBlastDivExp = ajRegCompC("^([^ ]+)( +([^ ]+))?");

    if(!qry->QryData)
	if(!seqCdQryOpen(qry))
	{
	    ajErr("Blast database open failed");
	    return ajFalse;
	}


    if(next)
	if(!seqCdDivNext(qry)) /* set qryd->div to next (included) file */
	    return ajFalse;

    qryd = qry->QryData;

    qryd->type = 0;

    HeaderLen = 0;

    seqCdFileSeek(qryd->dfp, (qryd->div - 1)); /* first (only) file */

    seqCdFileReadShort(&j, qryd->dfp);
    seqCdFileRead(qryd->name, qryd->nameSize, qryd->dfp);

    ajDebug("div: %d namesize: %d name '%s'\n",
	    qryd->div, qryd->nameSize, qryd->name);

    if(!ajRegExecC(seqBlastDivExp, qryd->name))
    {
	ajWarn("index division file error '%s'", qryd->name);
	return ajFalse;
    }


    ajRegSubI(seqBlastDivExp, 1, &qryd->datfile);
    ajRegSubI(seqBlastDivExp, 3, &qryd->seqfile);
    ajDebug("File(s) '%S' '%S'\n", qryd->datfile, qryd->seqfile);

    ajDebug("seqBlastOpen next: %B '%S' '%s'\n",
	    next, qryd->datfile, qryd->name);

    if(ajStrGetCharLast(qryd->datfile) == 'b')
	qryd->type = 0;			/* *tb : blast1 */
    else
    {
	qryd->type = 2;			/* *in : blast2 */
	isblast2 = ajTrue;
    }

    if(ajStrMatchCaseC(qry->DbType, "N"))
    {
	qryd->type += 1;
	isdna = ajTrue;
    }

    ajStrAssignS(&qryd->srcfile, qryd->datfile);
    ajFileNameExtC(&qryd->srcfile, NULL);
    ajFmtPrintS(&qryd->seqfile, "%S.%s", qryd->srcfile, seqext[qryd->type]);
    ajFmtPrintS(&qryd->datfile, "%S.%s", qryd->srcfile, hdrext[qryd->type]);
    ajFmtPrintS(&qryd->tblfile, "%S.%s", qryd->srcfile, tblext[qryd->type]);

    ajFileClose(&qryd->libs);
    ajFileClose(&qryd->libr);
    ajFileClose(&qryd->libt);
    ajFileClose(&qryd->libf);

    qryd->libs = seqBlastFileOpen(qry->Directory, qryd->seqfile);
    qryd->libr = seqBlastFileOpen(qry->Directory, qryd->datfile);
    qryd->libt = seqBlastFileOpen(qry->Directory, qryd->tblfile);
    qryd->libf = seqBlastFileOpen(qry->Directory, qryd->srcfile);

    ajDebug("seqfile '%F'\n", qryd->libs);
    ajDebug("datfile '%F'\n", qryd->libr);
    ajDebug("tblfile '%F'\n", qryd->libt);
    ajDebug("srcfile '%F'\n", qryd->libf);

    /* read the first part of the table file and set up the offsets */

    DbType = ajFileReadUint(qryd->libt, bigend);
    DbFormat = ajFileReadUint(qryd->libt, bigend);
    HeaderLen += 8;
    ajDebug("dbtype: %x dbformat: %x\n", DbType, DbFormat);

    TitleLen = ajFileReadUint(qryd->libt, bigend);
    if(isblast2)
	rdtmp = TitleLen;
    else
	rdtmp = TitleLen + ((TitleLen%4 !=0 ) ? 4-(TitleLen%4) : 0);

    ajStrAssignResC(&Title, rdtmp+1, "");
    ajDebug("IsBlast2: %B title_len: %d rdtmp: %d title_str: '%S'\n",
	    isblast2, TitleLen, rdtmp, Title);
    ajStrTrace(Title);

    if(rdtmp)
    {
	if(!ajFileRead(ajStrGetuniquePtr(&Title),
		       (size_t)1, (size_t)rdtmp, qryd->libt))
	    ajFatal("error reading file %F", qryd->libt);
    }
    else
	ajStrAssignC(&Title, "");

    if(isblast2)
	ajStrSetValidLen(&Title, TitleLen);
    else
	ajStrSetValidLen(&Title, TitleLen-1);

    ajDebug("title_len: %d rdtmp: %d title_str: '%S'\n",
	    TitleLen, rdtmp, Title);

    HeaderLen += 4 + rdtmp;

    /* read the date - blast2 */

    if(isblast2)
    {
	DateLen = ajFileReadUint(qryd->libt, bigend);
	rdtmp2 = DateLen;
	ajStrAssignResC(&Date, rdtmp2+1, "");
	if(!ajFileRead(ajStrGetuniquePtr(&Date),
		       (size_t)1,(size_t)rdtmp2,qryd->libt))
	    ajFatal("error reading file %F", qryd->libt);
	ajStrSetValidLen(&Date, DateLen);
	ajStrSetValid(&Date);
	DateLen = ajStrGetLen(Date);
	ajDebug("datelen: %d rdtmp: %d date_str: '%S'\n",
		DateLen, rdtmp2, Date);
	HeaderLen += 4 + rdtmp2;
    }

    /* read the rest of the header (different for protein and DNA) */

    if(isdna && !isblast2)
    {
	/* length of source lines */
	LineLen = ajFileReadUint(qryd->libt, bigend);
	HeaderLen += 4;
    }
    else
	LineLen = 0;

    /* all formats have the next 3 */

    Size = ajFileReadUint(qryd->libt, bigend);

    qryd->Size = Size;

    if(!isdna)
    {
	/* mad, but they are the other way for DNA */
	TotLen = ajFileReadUint(qryd->libt, bigend);
	MaxSeqLen = ajFileReadUint(qryd->libt, bigend);
    }
    else
    {
	MaxSeqLen = ajFileReadUint(qryd->libt, bigend);
	TotLen = ajFileReadUint(qryd->libt, bigend);
    }

    HeaderLen += 12;

    if(isdna && !isblast2)
    {
	/* Blast 1.4 DNA only */
	/* compressed db length */
	CompLen = ajFileReadUint(qryd->libt, bigend);
	/* count of nt's cleaned */
	CleanCount = ajFileReadUint(qryd->libt, bigend);
	HeaderLen += 8;
    }
    else
    {
	CompLen = 0;
	CleanCount = 0;
    }

    ajDebug(" size: %u, totlen: %d maxseqlen: %u\n",
	    Size, TotLen, MaxSeqLen);
    ajDebug(" linelen: %u, complen: %d cleancount: %d\n",
	    LineLen, CompLen, CleanCount);

    /* Now for the tables of offsets. Again maddeningly different in each */

    if(isblast2)
    {					/* NCBI BLAST 2.x */
	qryd->TopHdr = HeaderLen;	/* header first */
	qryd->TopCmp = qryd->TopHdr + (Size+1) * 4;
	if(isdna)
	    qryd->TopAmb = qryd->TopCmp + (Size+1) * 4; /* DNA only */
	else
	    qryd->TopAmb = 0;
    }
    else
    {				     /* NCBI BLAST 1.x and WU-BLAST */
	qryd->TopCmp = HeaderLen + CleanCount*4;
	if(isdna)
	{
	    /* DNA, if FASTA file used */
	    qryd->TopSrc = qryd->TopCmp + (Size+1) * 4;
	    qryd->TopHdr = qryd->TopSrc + (Size+1) * 4;
	    qryd->TopAmb = qryd->TopHdr + (Size+1) * 4; /* DNA */
	}
	else
	{
	    qryd->TopSrc = 0;
	    qryd->TopHdr = qryd->TopCmp + (Size+1) * 4;
	    qryd->TopAmb = 0;
	}
    }

    ajDebug("table file csq    starts at %d\n", qryd->TopCmp);
    ajDebug("table file src    starts at %d\n", qryd->TopSrc);
    ajDebug("table file hdr    starts at %d\n", qryd->TopHdr);
    ajDebug("table file amb    starts at %d\n", qryd->TopAmb);

    ajStrDel(&Date);
    ajStrDel(&Title);

    return ajTrue;
}




/* @funcstatic seqCdDivNext ***************************************************
**
** Sets the division count to the next included file. We need the division
** file to be already open.
**
** @param [u] qry [AjPSeqQuery] sequence query object.
** @return [ajuint] File number (starting at 1) or zero if all files are done.
** @@
******************************************************************************/

static ajuint seqCdDivNext(AjPSeqQuery qry)
{
    SeqPCdQry qryd;
    AjPStr fullName = NULL;
    ajuint i;

    qryd = qry->QryData;

    ajDebug("seqCdDivNext div: %d dfp: %x nameSize: %d name '%s'\n",
	    qryd->div, qryd->maxdiv, qryd->nameSize, qryd->name);

    for(i=qryd->div; i < qryd->maxdiv; i++)
        if(!qryd->Skip[i])
	{
	    qryd->div = i+1;
	    ajDebug("next file is %d '%S'\n", qryd->div, fullName);
	    return qryd->div;
	}
	else
	    ajDebug("skip %d  '%S'\n", (i+1), fullName);

    return 0;
}




/* @funcstatic seqBlastLoadBuff ***********************************************
**
** Fill a buffered file with text data and preloads the sequence in
** an AjPSeqin data structure for reuse.
**
** @param [u] seqin [AjPSeqin] AjPSeqin sequence input object.
** @return [AjBool] true if text data loaded.
** @@
******************************************************************************/

static AjBool seqBlastLoadBuff(AjPSeqin seqin)
{
    AjBool ret;
    AjPStr hdrstr = NULL;
    AjPStr seqstr = NULL;
    AjPSeqQuery qry;
    SeqPCdQry qryd;

    qry  = seqin->Query;
    qryd = qry->QryData;

    if(!qry->QryData)
	ajFatal("seqBlastLoadBuff Query Data not initialised");

    ajDebug("seqBlastLoadBuff libt: %F %d\n", qryd->libt, qryd->idnum);

    ret = seqBlastReadTable(seqin, &hdrstr, &seqstr);

    ajStrDel(&hdrstr);
    ajStrDel(&seqstr);

    return ret;
}




/* @funcstatic seqBlastAll ****************************************************
**
** Reads the EMBLCD division lookup file when first called for a new query,
** and returns each entry until done.
**
** @param [u] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqBlastAll(AjPSeqin seqin)
{
    AjPSeqQuery qry;
    SeqPCdQry qryd;
    static ajint called = 0;

    qry  = seqin->Query;
    qryd = qry->QryData;

    if(!called)
    {
	if(ajUtilGetBigendian())
	    seqCdReverse = ajTrue;
	called = 1;
    }

    ajDebug("seqBlastAll\n");

    if(!qry->QryData)
    {
	ajDebug("seqBlastAll initialising\n");
	seqin->Single = ajTrue;
	if(!seqBlastOpen(qry, ajTrue))	/* replaces qry->QryData */
	    ajFatal("seqBlastAll failed");

	qryd = qry->QryData;
	ajFileBuffDel(&seqin->Filebuff);
	seqin->Filebuff = ajFileBuffNew();
	qryd->idnum = 0;
    }
    else
	qryd = qry->QryData;

    if(!qryd->libr)
    {
	ajDebug("seqBlastAll finished\n");
	seqBlastQryClose(qry);
	return ajFalse;
    }

    if(!seqBlastLoadBuff(seqin))
    {
	if(!seqBlastOpen(qry, ajTrue))	/* try the next file */
	{
	    seqBlastQryClose(qry);
	    return ajFalse;
	}
	qryd = qry->QryData;
	qryd->idnum = 0;
	if(!qryd->libr)
	{
	    seqBlastQryClose(qry);
	    ajDebug("seqBlastAll finished\n");
	    return ajFalse;
        }

	if(!seqBlastLoadBuff(seqin))
	{
	    seqBlastQryClose(qry);
	    return ajFalse;
	}
    }

    qryd->idnum++;
    ajStrAssignS(&seqin->Db, qry->DbName);

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    return ajTrue;
}




/* @funcstatic seqCdQryFile ***************************************************
**
** Opens a specific file number for an EMBLCD index
**
** @param [u] qry [AjPSeqQuery] Query data
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqCdQryFile(AjPSeqQuery qry)
{
    SeqPCdQry qryd;
    short j;

    if(!seqCdDivExp)
	seqCdDivExp = ajRegCompC("^([^ ]+)( +([^ ]+))?");

    ajDebug("seqCdQryFile qry %x\n",qry);
    qryd = qry->QryData;
    ajDebug("seqCdQryFile qryd %x\n",qryd);
    ajDebug("seqCdQryFile %F\n",qryd->dfp->File);

    seqCdFileSeek(qryd->dfp, (qryd->div - 1));

    /* note - we must not use seqCdFileReadName - we need spaces for GCG */

    seqCdFileReadShort(&j, qryd->dfp);

    seqCdFileRead(qryd->name, qryd->nameSize, qryd->dfp);
    ajDebug("DivCode: %d, code: %2hd '%s'\n",
	    qryd->div, j, qryd->name);

    /**ajCharFmtLower(qryd->name);**/
    if(!ajRegExecC(seqCdDivExp, qryd->name))
    {
	ajErr("index division file error '%S'", qryd->name);
	return ajFalse;
    }
    ajRegSubI(seqCdDivExp, 1, &qryd->datfile);
    ajRegSubI(seqCdDivExp, 3, &qryd->seqfile);
    ajDebug("File(s) '%S' '%S'\n", qryd->datfile, qryd->seqfile);

    ajFileClose(&qryd->libr);
    qryd->libr = ajFileNewDF(qry->Directory, qryd->datfile);
    if(!qryd->libr)
    {
	ajDebug("Cannot open database file '%S'\n", qryd->datfile);
	ajErr("Cannot open database file '%S'", qryd->datfile);
	return ajFalse;
    }

    if(ajStrGetLen(qryd->seqfile))
    {
	ajFileClose(&qryd->libs);
	qryd->libs = ajFileNewDF(qry->Directory, qryd->seqfile);
	if(!qryd->libs)
	{
	    ajDebug("Cannot open sequence file '%S'\n", qryd->seqfile);
	    ajErr("Cannot open sequence file '%S'", qryd->seqfile);
	    return ajFalse;
	}
    }
    else
	qryd->libs = NULL;

    return ajTrue;
}




/* @section Remote URL Database Access ****************************************
**
** These functions manage the remote URL database access methods.
**
******************************************************************************/




/* @funcstatic seqAccessDbfetch ***********************************************
**
** Reads sequence(s) using EBI's dbfetch REST services.
**
** Dbfetch is accessed as a simple URL.
**
** @param [u] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqAccessDbfetch(AjPSeqin seqin)
{
    AjPStr host      = NULL;
    AjPStr urlget    = NULL;
    AjPStr get       = NULL;
    AjPStr proxyName = NULL;		/* host for proxy access.*/
    AjPStr httpver   = NULL;	      /* HTTP version 1.0, 1.1, ... */
    ajint iport;
    ajint proxyPort;
    FILE *fp;
    AjPSeqQuery qry;
    AjPStr searchdb = NULL;
    AjPStr qryid = NULL;

    iport = 80;
    proxyPort = 0;			/* port for proxy axxess */
    qry = seqin->Query;

    if(!ajNamDbGetDbalias(qry->DbName, &searchdb))
	ajStrAssignS(&searchdb, qry->DbName);
    ajDebug("seqAccessDbfetch %S:%S\n", qry->DbAlias, qry->Id);

    if(!seqHttpUrl(qry, &iport, &host, &urlget))
    {
	ajStrDel(&host);
	ajStrDel(&urlget);
	return ajFalse;
    }

    seqHttpVersion(qry, &httpver);
    if(ajStrGetLen(qry->Id))
	ajStrAssignS(&qryid, qry->Id);
    else if(qry->HasAcc && ajStrGetLen(qry->Acc))
	ajStrAssignS(&qryid, qry->Acc);
    else {
	return ajFalse;
    }
    if(seqHttpProxy(qry, &proxyPort, &proxyName))
	ajFmtPrintS(&get, "GET http://%S:%d%S?db=%S&id=%S&style=raw\n",
		    host, iport, urlget, searchdb, qryid);
    else
	ajFmtPrintS(&get, "GET %S?db=%S&id=%S&style=raw\n",
		    urlget, searchdb, qryid);

    /* finally we have set the GET command */
    ajDebug("host '%S' port %d get '%S'\n", host, iport, get);

    if(ajStrGetLen(proxyName))
	fp = seqHttpGetProxy(qry, proxyName, proxyPort, host, iport, get);
    else
	fp = seqHttpGet(qry, host, iport, get);

    if(!fp)
	return ajFalse;

    ajFileBuffDel(&seqin->Filebuff);
    seqin->Filebuff = ajFileBuffNewF(fp);
    if(!seqin->Filebuff)
    {
	ajDebug("socket buffer attach failed\n");
	ajErr("socket buffer attach failed for database '%S'", qry->DbName);
	return ajFalse;
    }
    ajDebug("Ready to read errno %d msg '%s'\n",
	    errno, ajMessSysErrorText());

#ifndef WIN32
    signal(SIGALRM, seqSocketTimeout);
    alarm(180);	    /* we allow 180 seconds to read from the socket */
#endif

    ajFileBuffLoad(seqin->Filebuff);

#ifndef WIN32
    alarm(0);
#endif

    ajFileBuffStripHtml(seqin->Filebuff);

    ajStrAssignS(&seqin->Db, qry->DbName);

    ajStrDel(&host);
    ajStrDel(&urlget);
    ajStrDel(&get);
    ajStrDel(&proxyName);
    ajStrDel(&httpver);

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    return ajTrue;
}




/* @funcstatic seqAccessMrs ***********************************************
**
** Reads sequence(s) using CMBI Nijmegen's Maarten's Retrieval System.
**
** MRS is accessed as a simple URL.
**
** @param [u] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqAccessMrs(AjPSeqin seqin)
{
    AjPStr host      = NULL;
    AjPStr urlget    = NULL;
    AjPStr get       = NULL;
    AjPStr proxyName = NULL;		/* host for proxy access.*/
    AjPStr httpver   = NULL;	      /* HTTP version 1.0, 1.1, ... */
    ajint iport;
    ajint proxyPort;
    FILE *fp;
    AjPSeqQuery qry;
    AjPStr searchdb = NULL;
    AjPStr qryid = NULL;

    iport = 80;
    proxyPort = 0;			/* port for proxy axxess */
    qry = seqin->Query;

    if(!ajNamDbGetDbalias(qry->DbName, &searchdb))
	ajStrAssignS(&searchdb, qry->DbName);
    ajDebug("seqAccessMrs %S:%S\n", qry->DbAlias, qry->Id);

    if(!seqHttpUrl(qry, &iport, &host, &urlget))
    {
	ajStrDel(&host);
	ajStrDel(&urlget);
	return ajFalse;
    }

    seqHttpVersion(qry, &httpver);
    if(seqHttpProxy(qry, &proxyPort, &proxyName))
	ajFmtPrintS(&get, "GET http://%S:%d%S",
		    host, iport, urlget);
    else
	ajFmtPrintS(&get, "GET %S",
		    urlget);

    ajFmtPrintAppS(&get,
		   "?db=%S",
		   searchdb, qryid);

    if(ajStrGetLen(qry->Id))
	ajFmtPrintAppS(&get,
		       "&query=id:%S",
		       qry->Id);
    else if(qry->HasAcc && ajStrGetLen(qry->Acc))
    {
    	ajFmtPrintAppS(&get,
		       "&query=ac:%S",
		       qry->Acc);
	return ajFalse;
    }

    ajFmtPrintAppS(&get,
		   "&format=entry&exp=1&save_to=text/plain",
		   searchdb, qryid);

    ajFmtPrintAppS(&get, " HTTP/%S\n", httpver);

    /* finally we have set the GET command */
    ajDebug("host '%S' port %d get '%S'\n", host, iport, get);

    if(ajStrGetLen(proxyName))
	fp = seqHttpGetProxy(qry, proxyName, proxyPort, host, iport, get);
    else
	fp = seqHttpGet(qry, host, iport, get);

    if(!fp)
	return ajFalse;

    ajFileBuffDel(&seqin->Filebuff);
    seqin->Filebuff = ajFileBuffNewF(fp);
    if(!seqin->Filebuff)
    {
	ajDebug("socket buffer attach failed\n");
	ajErr("socket buffer attach failed for database '%S'", qry->DbName);
	return ajFalse;
    }
    ajDebug("Ready to read errno %d msg '%s'\n",
	    errno, ajMessSysErrorText());

#ifndef WIN32
    signal(SIGALRM, seqSocketTimeout);
    alarm(180);	    /* we allow 180 seconds to read from the socket */
#endif

    ajFileBuffLoad(seqin->Filebuff);

#ifndef WIN32
    alarm(0);
#endif

    ajFileBuffStripHtml(seqin->Filebuff);

    ajStrAssignS(&seqin->Db, qry->DbName);

    ajStrDel(&host);
    ajStrDel(&urlget);
    ajStrDel(&get);
    ajStrDel(&proxyName);
    ajStrDel(&httpver);

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    return ajTrue;
}




/* @funcstatic seqAccessMrs3 ***********************************************
**
** Reads sequence(s) using CMBI Nijmegen's Maarten's Retrieval System.
**
** MRS is accessed as a simple URL.
**
** @param [u] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqAccessMrs3(AjPSeqin seqin)
{
    AjPStr host      = NULL;
    AjPStr urlget    = NULL;
    AjPStr get       = NULL;
    AjPStr proxyName = NULL;		/* host for proxy access.*/
    AjPStr httpver   = NULL;	      /* HTTP version 1.0, 1.1, ... */
    ajint iport;
    ajint proxyPort;
    FILE *fp;
    AjPSeqQuery qry;
    AjPStr searchdb = NULL;
    AjPStr qryid = NULL;

    iport = 80;
    proxyPort = 0;			/* port for proxy axxess */
    qry = seqin->Query;

    if(!ajNamDbGetDbalias(qry->DbName, &searchdb))
	ajStrAssignS(&searchdb, qry->DbName);
    ajDebug("seqAccessMrs %S:%S\n", qry->DbAlias, qry->Id);

    if(!seqHttpUrl(qry, &iport, &host, &urlget))
    {
	ajStrDel(&host);
	ajStrDel(&urlget);
	return ajFalse;
    }

    seqHttpVersion(qry, &httpver);
    if(seqHttpProxy(qry, &proxyPort, &proxyName))
	ajFmtPrintS(&get, "GET http://%S:%d%S",
		    host, iport, urlget);
    else
	ajFmtPrintS(&get, "GET %S",
		    urlget);

    ajFmtPrintAppS(&get,
		   "?db=%S",
		   searchdb, qryid);

    if(ajStrGetLen(qry->Id))
	ajFmtPrintAppS(&get,
		       "&id=%S",
		       qry->Id);
    else if(qry->HasAcc && ajStrGetLen(qry->Acc))
    {
    	ajFmtPrintAppS(&get,
		       "&ac=%S",
		       qry->Acc);
	return ajFalse;
    }

    ajFmtPrintAppS(&get, " HTTP/%S\n", httpver);

    /* finally we have set the GET command */
    ajDebug("host '%S' port %d get '%S'\n", host, iport, get);

    if(ajStrGetLen(proxyName))
	fp = seqHttpGetProxy(qry, proxyName, proxyPort, host, iport, get);
    else
	fp = seqHttpGet(qry, host, iport, get);

    if(!fp)
	return ajFalse;

    ajFileBuffDel(&seqin->Filebuff);
    seqin->Filebuff = ajFileBuffNewF(fp);
    if(!seqin->Filebuff)
    {
	ajDebug("socket buffer attach failed\n");
	ajErr("socket buffer attach failed for database '%S'", qry->DbName);
	return ajFalse;
    }
    ajDebug("Ready to read errno %d msg '%s'\n",
	    errno, ajMessSysErrorText());

#ifndef WIN32
    signal(SIGALRM, seqSocketTimeout);
    alarm(180);	    /* we allow 180 seconds to read from the socket */
#endif

    ajFileBuffLoad(seqin->Filebuff);

#ifndef WIN32
    alarm(0);
#endif

    ajFileBuffStripHtml(seqin->Filebuff);

    ajStrAssignS(&seqin->Db, qry->DbName);

    ajStrDel(&host);
    ajStrDel(&urlget);
    ajStrDel(&get);
    ajStrDel(&proxyName);
    ajStrDel(&httpver);

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    return ajTrue;
}




/* @funcstatic seqAccessUrl ***************************************************
**
** Reads sequence(s) using a remote URL. Reads the results into a buffer
** and strips out HTML before returning to the caller.
**
** @param [u] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqAccessUrl(AjPSeqin seqin)
{
    AjPStr host      = NULL;
    AjPStr urlget    = NULL;
    AjPStr get       = NULL;
    AjPStr proxyName = NULL;		/* host for proxy access.*/
    AjPStr httpver   = NULL;	      /* HTTP version 1.0, 1.1, ... */
    ajint iport;
    ajint proxyPort;
    FILE *fp;
    AjPSeqQuery qry;

    iport = 80;
    proxyPort = 0;			/* port for proxy axxess */
    qry = seqin->Query;

    if(!seqHttpUrl(qry, &iport, &host, &urlget))
    {
	ajStrDel(&host);
	ajStrDel(&urlget);
	return ajFalse;
    }

    seqHttpVersion(qry, &httpver);
    if(seqHttpProxy(qry, &proxyPort, &proxyName))
	ajFmtPrintS(&get, "GET http://%S:%d%S HTTP/%S\n",
		    host, iport, urlget, httpver);
    else
	ajFmtPrintS(&get, "GET %S HTTP/%S\n", urlget, httpver);

    /* replace %s in the "GET" command  with the ID */
    ajStrExchangeCS(&get, "%s", qry->Id);

    /* finally we have set the GET command */
    ajDebug("host '%S' port %d get '%S'\n", host, iport, get);

    if(ajStrGetLen(proxyName))
	fp = seqHttpGetProxy(qry, proxyName, proxyPort, host, iport, get);
    else
	fp = seqHttpGet(qry, host, iport, get);

    if(!fp)
	return ajFalse;

    ajFileBuffDel(&seqin->Filebuff);
    seqin->Filebuff = ajFileBuffNewF(fp);
    if(!seqin->Filebuff)
    {
	ajDebug("socket buffer attach failed\n");
	ajErr("socket buffer attach failed for database '%S'", qry->DbName);
	return ajFalse;
    }
    ajDebug("Ready to read errno %d msg '%s'\n",
	    errno, ajMessSysErrorText());

#ifndef WIN32
    signal(SIGALRM, seqSocketTimeout);
    alarm(180);	    /* we allow 180 seconds to read from the socket */
#endif

    ajFileBuffLoad(seqin->Filebuff);

#ifndef WIN32
    alarm(0);
#endif

    ajFileBuffStripHtml(seqin->Filebuff);

    ajStrAssignS(&seqin->Db, qry->DbName);

    ajStrDel(&host);
    ajStrDel(&urlget);
    ajStrDel(&get);
    ajStrDel(&proxyName);
    ajStrDel(&httpver);

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    return ajTrue;
}




/* @funcstatic seqSocketTimeout ***********************************************
**
** Fatal error if a socket read hangs
**
** @param [r] sig [int] Signal code - always SIGALRM but required by the
**                      signal call
** @return [void]
** @@
******************************************************************************/

static void seqSocketTimeout(int sig)
{
    (void) sig;

    ajDie("Socket read timeout");
    return;
}




/* @funcstatic seqHttpUrl *****************************************************
**
** Returns the components of a URL
**
** @param [r] qry [const AjPSeqQuery] Query object
** @param [w] iport [ajint*] Port
** @param [w] host [AjPStr*] Host name
** @param [w] urlget [AjPStr*] URL for the HTTP header GET
** @return [AjBool] ajTrue if the URL was parsed
** @@
******************************************************************************/

static AjBool seqHttpUrl(const AjPSeqQuery qry, ajint* iport, AjPStr* host,
			 AjPStr* urlget)
{
    AjPStr url              = NULL;
    AjPStr portstr          = NULL;

    if(!seqRegHttpUrl)
	seqRegHttpUrl = ajRegCompC("^http://([a-z0-9.-]+)(:[0-9]+)?(.*)");

    if(!ajNamDbGetUrl(qry->DbName, &url))
    {
	ajErr("no URL defined for database %S", qry->DbName);
	return ajFalse;
    }

    if(!ajRegExec(seqRegHttpUrl, url))
    {
	ajErr("invalid URL '%S' for database '%S'", url, qry->DbName);
	return ajFalse;
    }
    
    ajDebug("seqHttpUrl db: '%S' url: '%S'\n", qry->DbName, url);
    ajRegSubI(seqRegHttpUrl, 1, host);
    ajRegSubI(seqRegHttpUrl, 2, &portstr);
    if(ajStrGetLen(portstr))
    {
	ajStrCutStart(&portstr, 1);
	ajStrToInt(portstr, iport);
    }
    ajRegSubI(seqRegHttpUrl, 3, urlget);
    ajStrDel(&portstr);
    ajStrDel(&url);

    return ajTrue;
}




/* @funcstatic seqHttpProxy ***************************************************
**
** Returns a proxy definition (if any)
**
** @param [r] qry [const AjPSeqQuery] Query object
** @param [w] proxyport [ajint*] Proxy port
** @param [w] proxyname [AjPStr*] Proxy name
** @return [AjBool] ajTrue is a proxy was defined
** @@
******************************************************************************/

static AjBool seqHttpProxy(const AjPSeqQuery qry, ajint* proxyport,
			   AjPStr* proxyname)
{
    AjPStr proxyStr          = NULL;
    AjPStr proxy             = NULL;

    if(!seqRegHttpProxy)
	seqRegHttpProxy = ajRegCompC("^([a-z0-9.-]+):([0-9]+)$");

    ajNamGetValueC("proxy", &proxy);
    if(ajStrGetLen(qry->DbProxy))
	ajStrAssignS(&proxy, qry->DbProxy);

    if(ajStrMatchC(proxy, ":"))
	ajStrAssignC(&proxy, "");

    if(ajRegExec(seqRegHttpProxy, proxy))
    {
	ajRegSubI(seqRegHttpProxy, 1, proxyname);
	ajRegSubI(seqRegHttpProxy, 2, &proxyStr);
	ajStrToInt(proxyStr, proxyport);
	ajStrDel(&proxyStr);
	ajStrDel(&proxy);
	return ajTrue;
    }

    ajStrDel(proxyname);
    *proxyport = 0;
    ajStrDel(&proxy);

    return ajFalse;
}




/* @funcstatic seqHttpVersion *************************************************
**
** Returns the HTTP version
**
** @param [r] qry [const AjPSeqQuery] Query object
** @param [w] httpver [AjPStr*] HTTP version
** @return [AjBool] ajTrue if a version was defined
** @@
******************************************************************************/

static AjBool seqHttpVersion(const AjPSeqQuery qry, AjPStr* httpver)
{
    ajNamGetValueC("httpversion", httpver);
    ajDebug("httpver getValueC '%S'\n", *httpver);

    if(ajStrGetLen(qry->DbHttpVer))
	ajStrAssignS(httpver, qry->DbHttpVer);

    ajDebug("httpver after qry '%S'\n", *httpver);

    if(!ajStrGetLen(*httpver))
    {
	ajStrAssignC(httpver, "1.1");
	return ajFalse;
    }

    if(!ajStrIsFloat(*httpver))
    {
	ajWarn("Invalid HTTPVERSION '%S', reset to 1.1", *httpver);
	ajStrAssignC(httpver, "1.1");
	return ajFalse;
    }

    ajDebug("httpver final '%S'\n", *httpver);

    return ajTrue;
}




/* @funcstatic seqHttpGetProxy ************************************************
**
** Opens an HTTP connection via a proxy
**
** @param [r] qry [const AjPSeqQuery] Query object
** @param [r] proxyname [const AjPStr] Proxy name
** @param [r] proxyport [ajint] Proxy port
** @param [r] host [const AjPStr] Host name
** @param [r] iport [ajint] Port
** @param [r] get [const AjPStr] GET string
** @return [FILE*] Open file on success, NULL on failure
** @@
******************************************************************************/

static FILE* seqHttpGetProxy(const AjPSeqQuery qry,
			     const AjPStr proxyname, ajint proxyport,
			     const AjPStr host, ajint iport, const AjPStr get)
{
    FILE* fp;
    struct hostent* hp;
    ajint i;

#ifndef WIN32
    h_errno = 0;
#endif

    /* herror("proxy error"); */
    ajDebug("seqHttpGetProxy db: '%S' proxy '%S' host; %S get; '%S'\n",
	    qry->DbName, proxyname, host, get);
    hp = gethostbyname(ajStrGetPtr(proxyname));
    if(!hp)
    {
	ajErr("Failed to find proxy host '%S'", proxyname);
	return NULL;
    }

#ifndef WIN32
    ajDebug("gethostbyname proxyName '%S' returns '%s' errno %d hp_addr ",
	    proxyname, hp->h_name, h_errno);
#endif

    for(i=0; i< hp->h_length; i++)
    {
	if(i)
	    ajDebug(".");
	ajDebug("%d", (unsigned char) hp->h_addr[i]);
    }
    ajDebug("\n");
    fp = seqHttpSocket(qry, hp, proxyport, host, iport, get);

    return fp;
}




/* @funcstatic seqHttpGet *****************************************************
**
** Opens an HTTP connection
**
** @param [r] qry [const AjPSeqQuery] Query object
** @param [r] host [const AjPStr] Host name
** @param [r] iport [ajint] Port
** @param [r] get [const AjPStr] GET string
** @return [FILE*] Open file on success, NULL on failure
** @@
******************************************************************************/

static FILE* seqHttpGet(const AjPSeqQuery qry, const AjPStr host, ajint iport,
			const AjPStr get)
{
    FILE* fp;
    struct hostent* hp;
    ajint i;

#ifndef WIN32
    h_errno = 0;
#endif

    ajDebug("seqHttpGet db: '%S' host '%S' get: '%S'\n",
	    qry->DbName, host, get);
    hp = gethostbyname(ajStrGetPtr(host));
    /* herror("host error"); */
    if(!hp)
    {
	ajErr("Failed to find host '%S' for database '%S'",
	      host, qry->DbName);
	return NULL;
    }

#ifndef WIN32
    ajDebug("gethostbyname host '%S' returns '%s' errno %d hp_addr ",
	    host, hp->h_name, h_errno);
#else
    ajDebug("gethostbyname host '%S' returns '%s' errno %d hp_addr ",
	    host, hp->h_name, WSAGetLastError());
#endif


    for(i=0; i< hp->h_length; i++)
    {
	if(i)
	    ajDebug(".");
	ajDebug("%d", (unsigned char) hp->h_addr[i]);
    }
    ajDebug("\n");

    fp = seqHttpSocket(qry, hp, iport, host, iport, get);

    return fp;
}




/* @funcstatic seqHttpSocket **************************************************
**
** Opens an HTTP socket
**
** @param [r] qry [const AjPSeqQuery] Query object
** @param [r] hp [const struct hostent*] Host entry struct
** @param [r] hostport [ajint] Host port
** @param [r] host [const AjPStr] Host name for Host header line
** @param [r] iport [ajint] Port for Host header line
** @param [r] get [const AjPStr] GET string
** @return [FILE*] Open file on success, NULL on failure
** @@
******************************************************************************/

static FILE* seqHttpSocket(const AjPSeqQuery qry,
			   const struct hostent *hp, ajint hostport,
			   const AjPStr host, ajint iport, const AjPStr get)
{
    FILE* fp       = NULL;
    AjPStr gethead = NULL;
#ifndef WIN32
    ajint sock;
#else
    SOCKET sock;
#endif
    ajint istatus;
    ajuint isendlen;
    struct sockaddr_in sin;
    AjPStr errstr  = NULL;
 
    ajDebug("creating socket\n");

#ifndef WIN32
    sock = socket(AF_INET, SOCK_STREAM, 0);
#else
    /* Windows' socket() creates sockets in overlapped mode. */
    /* Only WSASocket() can override this. */
    sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, 0);
    if(sock == INVALID_SOCKET)
    {
	ajErr ("Socket create failed for database '%S'", qry->DbName);
	return NULL;
    }
#endif

    if(sock < 0)
    {
	ajDebug("Socket create failed, sock: %d\n", sock);
	ajErr("Socket create failed for database '%S'", qry->DbName);
	return NULL;
    }

    ajDebug("setup socket data \n");
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(hostport);
#ifndef __VMS
    memcpy(&sin.sin_addr, hp->h_addr, hp->h_length);
#endif

    ajDebug("connecting to socket %d\n", sock);
    ajDebug("sin sizeof %d\n", sizeof(sin));
    istatus = connect(sock, (struct sockaddr*) &sin, sizeof(sin));
    if(istatus < 0)
    {
	ajDebug("socket connect failed, status: %d\n", istatus);
	ajFmtPrintS(&errstr, "socket connect failed for database '%S'",
		    qry->DbName);
	ajErr("%S", errstr);
	perror(ajStrGetPtr(errstr));
	ajStrDel(&errstr);
	return NULL;
    }

    ajDebug("connect status %d errno %d msg '%s'\n",
	    istatus, errno, ajMessSysErrorText());

    ajDebug("inet_ntoa '%s'\n", inet_ntoa(sin.sin_addr));
    isendlen = send(sock, ajStrGetPtr(get), ajStrGetLen(get), 0);
    if(isendlen != ajStrGetLen(get))
	ajErr("send failure, expected %d bytes returned %d : %s\n",
	      ajStrGetLen(get), istatus, ajMessSysErrorText());
    ajDebug("sending: '%S' status: %d\n", get, istatus);
    ajDebug("send for GET errno %d msg '%s'\n",
	    errno, ajMessSysErrorText());

    /*
       ajFmtPrintS(&gethead, "Accept: \n");
       ajDebug("sending: '%S'\n", gethead);
       send(sock, ajStrGetPtr(gethead), ajStrGetLen(gethead), 0);
       
       ajFmtPrintS(&gethead, "User-Agent: EMBOSS\n");
       ajDebug("sending: '%S'\n", gethead);
       send(sock, ajStrGetPtr(gethead), ajStrGetLen(gethead), 0);
       */

    ajFmtPrintS(&gethead, "Host: %S:%d\n", host, iport);
    isendlen =  send(sock, ajStrGetPtr(gethead), ajStrGetLen(gethead), 0);
    if(isendlen != ajStrGetLen(gethead))
	ajErr("send failure, expected %d bytes returned %d : %s\n",
	      ajStrGetLen(gethead), istatus, ajMessSysErrorText());
    ajDebug("sending: '%S' status: %d\n", gethead, istatus);
    ajDebug("send for host errno %d msg '%s'\n",
	    errno, ajMessSysErrorText());

    ajFmtPrintS(&gethead, "\n");
    isendlen =  send(sock, ajStrGetPtr(gethead), ajStrGetLen(gethead), 0);
    if(isendlen != ajStrGetLen(gethead))
	ajErr("send failure, expected %d bytes returned %d : %s\n",
	      ajStrGetLen(gethead), istatus, ajMessSysErrorText());
    ajDebug("sending: '%S' status: %d\n", gethead, istatus);
    ajDebug("send for blankline errno %d msg '%s'\n",
	    errno, ajMessSysErrorText());

    ajStrDel(&gethead);

#ifndef WIN32
    fp = ajSysFuncFdopen(sock, "r");
#else
    {
        int fd = _open_osfhandle(sock, _O_RDONLY);
	fp = ajSysFuncFdopen(fd, "r");
    }
#endif


    ajDebug("fdopen errno %d msg '%s'\n",
	    errno, ajMessSysErrorText());
    if(!fp)
    {
	ajDebug("socket open failed sock: %d\n", sock);
	ajErr("socket open failed for database '%S'", qry->DbName);
	return NULL;
    }

    return fp;
}




/* @section Application Database Access ***************************************
**
** These functions manage the application database access methods.
**
******************************************************************************/

/* @funcstatic seqAccessApp ***************************************************
**
** Reads sequence data using an application which can accept a specification
** in the form "database:entry" such as Erik Sonnhammer's 'efetch'.
**
** @param [u] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqAccessApp(AjPSeqin seqin)
{
    static AjPStr pipename = NULL;
    AjPSeqQuery qry;

    qry = seqin->Query;

    if(!ajStrGetLen(qry->Application))
    {
	ajErr("APP access: application not defined for %S", qry->DbName);
	return ajFalse;
    }

    ajDebug("seqAccessApp '%S' dbname '%S'\n", qry->Application, qry->DbName);

    if(ajStrMatchWildC(qry->Application, "*%s*"))
    {
	if(ajStrGetLen(qry->Id))
	    ajFmtPrintS(&pipename, ajStrGetPtr(qry->Application),
			ajStrGetPtr(qry->Id));
	else if(qry->HasAcc && ajStrGetLen(qry->Acc))
	    ajFmtPrintS(&pipename, ajStrGetPtr(qry->Application),
			ajStrGetPtr(qry->Acc));
	else
	    ajFmtPrintS(&pipename, ajStrGetPtr(qry->Application),
			"*");
	ajStrAppendC(&pipename, " |");
    }
    else
    {
	if(ajStrGetLen(qry->Id))
	    ajFmtPrintS(&pipename, "%S %S:%S|",
			qry->Application, qry->DbName, qry->Id);
	else if(qry->HasAcc && ajStrGetLen(qry->Acc))
	    ajFmtPrintS(&pipename, "%S %S:%S|",
			qry->Application, qry->DbName, qry->Acc);
 	else
	    ajFmtPrintS(&pipename, "%S %S:*|",
			qry->Application, qry->DbName);
    }

    if(!ajStrGetLen(pipename))
    {
	ajErr("APP access: bad query format");
	return ajFalse;
    }


    ajFileBuffDel(&seqin->Filebuff);
    seqin->Filebuff = ajFileBuffNewIn(pipename);
    if(!seqin->Filebuff)
    {
	ajErr("unable to open file '%S'", pipename);
	ajStrDel(&pipename);
	return ajFalse;
    }

    ajStrAssignS(&seqin->Db, qry->DbName);

    ajStrDel(&pipename);

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    return ajTrue;
}




/* @section ASIS Sequence Access **********************************************
**
** These functions manage the ASIS sequence access methods.
**
******************************************************************************/




/* @func ajSeqAccessAsis ******************************************************
**
** Reads a sequence using the 'filename' as the sequence data.
**
** @param [u] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajSeqAccessAsis(AjPSeqin seqin)
{
    AjPSeqQuery qry;

    qry = seqin->Query;

    if(!ajStrGetLen(qry->Filename))
    {
	ajErr("ASIS access: no sequence");
	return ajFalse;
    }

    ajDebug("ajSeqAccessAsis %S\n", qry->Filename);

    ajFileBuffDel(&seqin->Filebuff);
    seqin->Filebuff = ajFileBuffNewS(qry->Filename);
    if(!seqin->Filebuff)
    {
	ajDebug("Asis access: unable to use sequence '%S'\n", qry->Filename);
	return ajFalse;
    }
    ajStrAssignC(&seqin->Filename, "asis");
    /*ajFileBuffTrace(seqin->Filebuff);*/

    return ajTrue;
}




/* @section File Access *******************************************************
**
** These functions manage the sequence file access methods.
**
******************************************************************************/

/* @func ajSeqAccessFile ******************************************************
**
** Reads a sequence from a named file.
**
** @param [u] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajSeqAccessFile(AjPSeqin seqin)
{
    AjPSeqQuery qry;

    qry = seqin->Query;

    if(!ajStrGetLen(qry->Filename))
    {
	ajErr("FILE access: no filename");
	return ajFalse;
    }

    ajDebug("ajSeqAccessFile %S\n", qry->Filename);

    /* ajStrTraceT(qry->Filename, "qry->Filename (before):"); */

    ajFileBuffDel(&seqin->Filebuff);
    seqin->Filebuff = ajFileBuffNewIn(qry->Filename);
    if(!seqin->Filebuff)
    {
	ajDebug("FILE access: unable to open file '%S'\n", qry->Filename);
	return ajFalse;
    }

    /* ajStrTraceT(seqin->Filename, "seqin->Filename:"); */
    /* ajStrTraceT(qry->Filename, "qry->Filename (after):"); */

    ajStrAssignS(&seqin->Filename, qry->Filename);

    return ajTrue;
}




/* @func ajSeqAccessOffset ****************************************************
**
** Reads a sequence from a named file, at a given offset within the file.
**
** @param [u] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajSeqAccessOffset(AjPSeqin seqin)
{
    AjPSeqQuery qry;

    qry = seqin->Query;

    if(!ajStrGetLen(qry->Filename))
    {
	ajErr("FILE access: no filename");
	return ajFalse;
    }

    ajDebug("ajSeqAccessOffset %S %Ld\n", qry->Filename, qry->Fpos);

    /* ajStrTraceT(qry->Filename, "qry->Filename (before):"); */

    ajFileBuffDel(&seqin->Filebuff);
    seqin->Filebuff = ajFileBuffNewIn(qry->Filename);
    if(!seqin->Filebuff)
    {
	ajDebug("OFFSET access: unable to open file '%S'\n", qry->Filename);
	return ajFalse;
    }
    ajFileSeek(ajFileBuffFile(seqin->Filebuff), qry->Fpos, 0);
    /* ajStrTraceT(seqin->Filename, "seqin->Filename:"); */
    /* ajStrTraceT(qry->Filename, "qry->Filename (after):"); */
    ajStrAssignS(&seqin->Filename, qry->Filename);

    return ajTrue;
}




/* @section File Direct Access ************************************************
**
** These functions manage the sequence file direct access methods.
**
******************************************************************************/




/* @funcstatic seqAccessDirect ************************************************
**
** Reads a sequence from a database which may have multiple files.
** The sequence input object holds a directory name and a (wildcard)
** file specification.
**
** Can also use exclude definitions for files in the directory.
**
** @param [u] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqAccessDirect(AjPSeqin seqin)
{
    AjPSeqQuery qry;

    ajDebug("seqAccessDirect %S\n", seqin->Query->DbName);

    qry = seqin->Query;

    if(!ajStrGetLen(qry->Filename))
    {
	ajErr("DIRECT access: filename not specified");
	return ajFalse;
    }

    ajDebug("Try to open %S%S\n", qry->Directory, qry->Filename);

    ajFileBuffDel(&seqin->Filebuff);
    seqin->Filebuff = ajFileBuffNewDWE(qry->Directory, qry->Filename,
				       qry->Exclude);
    if(!seqin->Filebuff)
    {
	ajDebug("DIRECT access: unable to open file '%S/%S'\n",
		qry->Directory, qry->Filename);
	return ajFalse;
    }

    ajStrAssignS(&seqin->Db, qry->DbName);
    ajStrAssignS(&seqin->Filename, qry->Filename);

    return ajTrue;
}




/* @funcstatic seqBlastReadTable **********************************************
**
** Read one entry in the BLAST binary table into memory, and
** load the header and sequence for it. All that is needed is
** a set of open blast files (in qryd) and an idnum set.
**
** @param [u] seqin [AjPSeqin] Sequence input.
** @param [w] hline [AjPStr*] header line.
** @param [w] sline [AjPStr*] sequence line.
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqBlastReadTable(AjPSeqin seqin, AjPStr* hline,
				AjPStr* sline)
{
    ajint ipos;
    ajint start;
    ajint end;
    ajint hsize;
    ajint seq_len = -1;
    ajint seqcnt  = -1;
    char* sptr;
    ajint c_len;
    ajint a_len;
    ajint astart  = 0;
    ajint fstart  = 0;
    ajint fend    = 0;
    ajint i;
    ajint j;
    size_t tmp;
    char* btoa;
    static ajint c_pad;
    char* tptr;
    ajint s_chunk;
    char stmp;
    unsigned char tmpbyte;
    static char bases[] = "NACGT";
    static char abases[] = "NACMGRSVTWYHKDBN";
    ajint spos;
    ajint apos;
    ajuint bc;
    ajuint bn;
    ajuint ip;
    ajuint iamb;
    ajuint ui;
    static AjPStr rdline = NULL;
    ajint nbpn;
    ajint char_bit;
    ajint nsentinels;
    char* seq = NULL;
    unsigned char nt_magic_byte;
    AjPSeqQuery qry;
    SeqPCdQry qryd;
    AjBool bigend = ajTrue;

    nbpn       = 2;
    char_bit   = 8;
    nsentinels = 2;

    nt_magic_byte = 0xfc;

    qry  = seqin->Query;
    qryd = qry->QryData;
    ipos = qryd->idnum;

    ajDebug("seqBlastReadTable %d\n", ipos);


    if(qryd->idnum >= qryd->Size)
    {
	ajDebug("beyond end of database\n");
	return ajFalse;
    }

    /* find the table record and read the positions we want */

    /* find the header record and read it */

    /* find the sequence in the binary or FASTA file and read it */

    ajFileSeek(qryd->libt, qryd->TopHdr + 4*(qryd->idnum), 0);
    ajDebug("hdr reading at %d\n", ajFileTell(qryd->libt));
    start = ajFileReadUint(qryd->libt, bigend);
    ajDebug("hdr read i: %d value: %d\n", qryd->idnum, start);
    end = ajFileReadUint(qryd->libt, bigend);
    ajDebug("hdr read i: %d value: %d\n", qryd->idnum, end);

    if(end)
	hsize = end - start;
    else
	hsize = qryd->Size - start;

    ajStrAssignResC(hline, hsize+1, "");

    ajDebug("type: %d hsize: %d start: %d end: %d dbSize: %d\n",
	    qryd->type, hsize, start, end, qryd->Size);

    ajFileSeek(qryd->libr, start, 0);
    if(!ajFileRead(ajStrGetuniquePtr(hline), 1, hsize, qryd->libr))
	ajFatal("error reading file %F", qryd->libr);
    ajStrSetValidLen(hline, hsize);


    if(qryd->type >= 2)
	seqBlastStripNcbi(hline);	/* trim the gnl| prefix */
    /* The above now just adds a > */

    ajFileBuffClear(seqin->Filebuff, -1); /* delete all lines */

    ajDebug("Load FASTA file with '%S'\n", *hline);
    ajFileBuffLoadS(seqin->Filebuff, *hline);

    ajDebug("\n** Blast Sequence Reading **\n");

    ajFileSeek(qryd->libt, qryd->TopCmp + 4*(qryd->idnum), 0);
    ajDebug("seq reading at %d\n", ajFileTell(qryd->libt));
    start = ajFileReadUint(qryd->libt, bigend);
    ajDebug("seq read i: %d start: %d\n", qryd->idnum, start);
    end = ajFileReadUint(qryd->libt, bigend);
    ajDebug("seq read i: %d   end: %d\n", qryd->idnum, end);

    if(qryd->type == 1 && qryd->libf)
    {					/* BLAST 1 FASTA file */
	ajFileSeek(qryd->libt, qryd->TopSrc + 4*(qryd->idnum), 0);
	ajDebug("src reading at %d\n", ajFileTell(qryd->libt));
	fstart = ajFileReadUint(qryd->libt, bigend);
	ajDebug("src read i: %d fstart: %d\n", qryd->idnum, fstart);
	fend = ajFileReadUint(qryd->libt, bigend);
	ajDebug("src read i: %d   fend: %d\n", qryd->idnum, fend);
    }

    if(qryd->type == 3)
    {					/* BLAST 2 DNA ambuguities */
	ajFileSeek(qryd->libt, qryd->TopAmb + 4*(qryd->idnum), 0);
	ajDebug("amb reading at %d\n", ajFileTell(qryd->libt));
	astart = ajFileReadUint(qryd->libt, bigend);
	ajDebug("amb read i: %d astart: %d\n", qryd->idnum, astart);
    }

    switch(qryd->type)
    {
    case 0:				/* protein 1 */
    case 2:				/* protein 2 */
	ajDebug("reading protein sequence file\n");
	if(qryd->type == 2)
	    btoa = aa_btoa2;
	else
	    btoa = aa_btoa;

	spos = start;
	ajFileSeek(qryd->libs,spos-1,0);

	seq_len = end - start - 1;
	ajDebug("seq_len: %d spos: %d %x\n", seq_len, spos, spos);

	ajStrAssignResC(sline, seq_len+1, "");

	if(!ajFileRead(&tmpbyte, 1, 1, qryd->libs)) /* skip the null byte */
	    ajFatal("error reading file %F", qryd->libs);
	if(tmpbyte)
	    ajErr(" phase error: %d:%d found\n",qryd->idnum,(ajint)tmpbyte);

	if((tmp=ajFileRead(ajStrGetuniquePtr(sline),(size_t)1,(size_t)seq_len,
			   qryd->libs)) != (size_t)seq_len)
	{
	    ajErr(" could not read sequence record (a): %d %d != %d\n",
		  start,tmp,seq_len);
	    ajErr(" error reading seq at %d\n",start);
	    return ajFalse;
	}

	if(btoa[(ajint)ajStrGetCharLast(*sline)] =='*')
	{				/* skip * at end */
	    seqcnt = seq_len-1;
	    ajStrCutEnd(sline, 1);
	}
	else seqcnt=seq_len;

	seq = ajStrGetuniquePtr(sline);
	sptr = seq+seqcnt;

	while(--sptr >= seq)
	    *sptr = btoa[(ajint)*sptr];

	ajStrSetValidLen(sline, seqcnt);
	ajDebug("Read sequence %d %d\n'%S'\n", seqcnt, ajStrGetLen(*sline),
		*sline);
	ajStrAssignS(&seqin->Inseq, *sline);
	return ajTrue;


    case 3:				/* DNA 2 */
	ajDebug("reading blast2 DNA file\n");
	ajFileSeek(qryd->libs,start,0);
	spos = (start)/(char_bit/nbpn);
	c_len = astart - start;	/* we have ambiguities in the nsq file */
	seq_len = c_len*4;

	ajDebug("c_len %d spos %d seq_len %d\n",
		c_len, spos, seq_len);

	ajStrAssignResC(sline, seq_len+1, "");

	seq = ajStrGetuniquePtr(sline);

	/* read the sequence here */

	seqcnt = c_len;
	if((tmp=ajFileRead(ajStrGetuniquePtr(sline),(size_t)1,(size_t)seqcnt,
			   qryd->libs)) != (size_t)seqcnt)
	{
	    ajErr(" could not read sequence record (c): %d %d != %d: %d\n",
		  qryd->idnum,tmp,seqcnt,*seq);
	    exit(0);
	}
	sptr = seq + seqcnt;

	/* the last byte is either '0' (no remainder) or the last 1-3
	   chars and the remainder */

	c_pad = *(sptr-1);
	c_pad &= 0x3;		       /* get the last (low) 2 bits */
	seq_len -= (4 - c_pad);	/* if the last 2 bits are 0, its a NULL byte */
	ajDebug("(a) c_pad %d seq_len %d seqcnt %d\n",
		c_pad, seq_len, seqcnt);

	/*
	 ** point to the last packed byte and to the end of the array
	 ** seqcnt is the exact number of bytes read tptr points to the
	 ** destination, use multiple of 4 to simplify math sptr points
	 ** to the source, note that the last byte will be read 4 cycles
	 ** before it is written
	 */

	tptr = seq + 4*seqcnt;
	s_chunk = seqcnt/8;

	ajDebug("sptr +%d tptr +%d s_chunk %d\n",
		sptr-seq, tptr-seq, s_chunk);

	/* do we need this first section or is it just for parallel code? */

	while(s_chunk-- > 0)
	{
	    stmp = *--sptr;
	    *--tptr = bases[(stmp&3) +1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    stmp = *--sptr;
	    *--tptr = bases[(stmp&3) +1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    stmp = *--sptr;
	    *--tptr = bases[(stmp&3) +1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    stmp = *--sptr;
	    *--tptr = bases[(stmp&3) +1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    stmp = *--sptr;
	    *--tptr = bases[(stmp&3) +1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    stmp = *--sptr;
	    *--tptr = bases[(stmp&3) +1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    stmp = *--sptr;
	    *--tptr = bases[(stmp&3) +1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    stmp = *--sptr;
	    *--tptr = bases[(stmp&3) +1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	}

	ajDebug("after: sptr +%d tptr +%d\n",
		sptr-seq, tptr-seq);

	while(tptr>seq)
	{
	    stmp = *--sptr;
	    *--tptr = bases[(stmp&3) +1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	}

	if(astart < end)
	{				/* read ambiguities */
	    a_len = end - astart;
	    apos = astart;
	    ajDebug("Read ambiguities: a_len %d apos: %d\n", a_len, apos);
	    ajFileSeek(qryd->libs,apos,0);
	    iamb = ajFileReadUint(qryd->libs, bigend);
	    ajDebug("iamb %d\n", iamb);
	    for(i=0;i<(ajint)iamb;i++)
	    {
		ui = ajFileReadUint(qryd->libs, bigend);
		bc = (ui & 0xF0000000);
		bc >>=28;
		bn = (ui & 0x0F000000);
		bn >>=24;
		ip = (ui & 0x00FFFFFF);

		ajDebug("amb[%3d] %x %5d %x %2d %8x %10d\n",
			i, ip, ip, bc, bc, bn, bn, ui, ui);
		for(j=0; j<= (ajint)bn; j++)
		    seq[ip+j] = abases[bc];
	    }
	}
	else
	    a_len = 0;

	ajStrSetValidLen(sline, seq_len);
	ajStrAssignS(&seqin->Inseq, *sline);
	return ajTrue;


    case 1:
	if(qryd->libf)
	{			   /* we have the FASTA source file */
	    seq_len = fend - fstart;
	    ajStrAssignResC(sline, seq_len+1, "");
	    ajDebug("reading FASTA file\n");
	    ajFileSeek(qryd->libf,fstart,0);
	    while(ajFileGetsTrim(qryd->libf, &rdline))
	    {				/* line + newline + 1 */
		ajDebug("Read: '%S'\n", rdline);
		if(ajStrGetCharFirst(rdline) == '>') /* the FASTA line */
		    break;
		ajStrAppendS(sline, rdline);
	    }
	    ajStrAssignS(&seqin->Inseq, *sline);
	    return ajTrue;
	}
	else
	{			 /* DNA Blast 1.4 from the csq file */

	    /*
	     ** Start and End offsets are in bases which are compressed
	     ** so we need to convert them to bytes for the file offsets
	     */

	    ajDebug("reading blast 1.4 compressed DNA file\n");
	    spos = (start)/(char_bit/nbpn);
	    ajFileSeek(qryd->libs,spos-1,0);

	    c_len = end/(char_bit/nbpn) - start/(char_bit/nbpn);
	    c_len -= nsentinels;      /* trim first 2 (magic) bytes */

	    seq_len = c_len*(char_bit/nbpn);
	    c_pad = start & ((char_bit/nbpn)-1);
	    if(c_pad != 0)
		seq_len -= ((char_bit/nbpn) - c_pad);

	    ajDebug("c_len %d c_pad %d spos %d seq_len %d\n",
		    c_len, c_pad, spos, seq_len);

	    ajStrAssignResC(sline, seq_len+1, "");

	    if(!ajFileRead(&tmpbyte, (size_t)1, (size_t)1,
			   qryd->libs))	/* skip the null byte */
		ajFatal("error reading file %F", qryd->libs);
	    if(tmpbyte != nt_magic_byte)
	    {
		ajDebug(" phase error: %d:%d (%d/%d) found\n",
			qryd->idnum,seq_len,(ajint)tmpbyte,
			(ajint)nt_magic_byte);
		ajDebug(" error reading seq at %d\n",start);
		ajErr(" phase error: %d:%d (%d/%d) found\n",
		      qryd->idnum,seq_len,(ajint)tmpbyte,(ajint)nt_magic_byte);
		ajErr(" error reading seq at %d\n",start);
		return ajFalse;
	    }

	    seqcnt=(seq_len+3)/4;
	    if(seqcnt==0)
		seqcnt++;
	    if((tmp=ajFileRead(ajStrGetuniquePtr(sline),(size_t)1,(size_t)seqcnt,
			       qryd->libs)) != (size_t)seqcnt)
	    {
		ajDebug(
			" could not read sequence record (e): %S %d %d"
			" != %d: %d\n",
			*sline,start,tmp,seqcnt,*seq);
		ajDebug(" error reading seq at %d\n",start);
		ajErr(
		      " could not read sequence record (f): %S %d %d"
		      " != %d: %d\n",
		      *sline,start,tmp,seqcnt,*seq);
		ajErr(" error reading seq at %d\n",start);
		return ajFalse;
	    }
	    /* skip the null byte */
	    if(!ajFileRead(&tmpbyte, (size_t)1, (size_t)1, qryd->libs))
		ajFatal("error reading file %F", qryd->libs);

	    if(tmpbyte != nt_magic_byte)
	    {
		ajDebug(" phase2 error: %d:%d (%d/%d) next \n",
			qryd->idnum,seqcnt,(ajint)tmpbyte,
			(ajint)nt_magic_byte);
		ajDebug(" error reading seq at %d\n",start);
		ajErr(" phase2 error: %d:%d (%d/%d) next ",
		      qryd->idnum,seqcnt,(ajint)tmpbyte,(ajint)nt_magic_byte);
		ajErr(" error reading seq at %d\n",start);
		return ajFalse;
	    }

	    /*
	     ** point to the last packed byte and to the end of the array
	     ** seqcnt is the exact number of bytes read
	     ** tptr points to the destination, use multiple of 4 to simplify
	     ** math
	     ** sptr points to the source, note that the last byte will be
	     ** read 4 cycles before it is written
	     */

	    seq = ajStrGetuniquePtr(sline);

	    sptr = seq + seqcnt;
	    tptr = seq + 4*seqcnt;
	    while(sptr>seq)
	    {
		stmp = *--sptr;
		*--tptr = bases[(stmp&3) +1];
		*--tptr = bases[((stmp >>= 2)&3)+1];
		*--tptr = bases[((stmp >>= 2)&3)+1];
		*--tptr = bases[((stmp >>= 2)&3)+1];
	    }

	    if(seqcnt*4 >= seq_len)
	    {				/* there was enough room */
		seq[seq_len]= '\0';
		ajDebug("enough room: seqlen %d\n",seq_len);
	    }
	    else
	    {				/* not enough room */
		seq[seqcnt*4]='\0';
		seq_len -= 4*seqcnt;
		ajDebug("not enough room: seqcnt: %d partial seqlen %d\n",
			seqcnt, seq_len);
	    }

	    ajStrSetValidLen(sline, seq_len);
	    ajStrAssignS(&seqin->Inseq, *sline);
	    return ajTrue;
	}

    default:
	ajErr("Unknown blast database type %d", qryd->type);
    }

    return ajFalse;
}




/* @funcstatic seqBlastStripNcbi **********************************************
**
** Removes the extra part of an NCBI style header from the BLAST header table.
** Function name no longer appropriate as it is now done by prefixing
** with '>' where this is missing.
**
** @param [u] line [AjPStr*] Input line
** @return [void]
** @@
******************************************************************************/

static void seqBlastStripNcbi(AjPStr* line)
{
    AjPStr tmpline = NULL;

    ajStrAssignS(&tmpline, *line);

    ajFmtPrintS(line, ">%S", tmpline);
    ajDebug("trim to   '%S'\n", tmpline);

    ajStrDel(&tmpline);

    return;
}




/* @funcstatic seqCdTrgQuery **************************************************
**
** Binary search of an EMBL CD-ROM index file for entries matching a
** wildcard query.
**
** Where more than one query field is defined (usually acc and sv) it
** can test all and append to a single list.
**
** @param [u] qry [AjPSeqQuery] Sequence query object.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqCdTrgQuery(AjPSeqQuery qry)
{
    ajint ret=0;

    if(ajStrGetLen(qry->Org))
	ret += seqCdTrgFind(qry, "taxon", qry->Org);

    if(ajStrGetLen(qry->Key))
	ret += seqCdTrgFind(qry, "keyword", qry->Key);

    if(ajStrGetLen(qry->Des))
	ret += seqCdTrgFind(qry, "des", qry->Des);

    if(ajStrGetLen(qry->Sv))
	ret += seqCdTrgFind(qry, "seqvn", qry->Sv);

    if(ajStrGetLen(qry->Gi))
	ret += seqCdTrgFind(qry, "gi", qry->Gi);

    if(qry->HasAcc && ajStrGetLen(qry->Acc))
	ret += seqCdTrgFind(qry, "acnum", qry->Acc);


    if(ret)
	return ajTrue;

    return ajFalse;
}




/* @funcstatic seqCdTrgFind **************************************************
**
** Binary search of an EMBL CD-ROM index file for entries matching a
** wildcard query.
**
** Where more than one query field is defined (usually acc and sv) it
** can test all and append to a single list.
**
** @param [u] qry [AjPSeqQuery] Sequence query object.
** @param [r] indexname [const char*] Index name.
** @param [r] queryName [const AjPStr] Query string.
** @return [ajuint] Number of matches found
** @@
******************************************************************************/

static ajuint seqCdTrgFind(AjPSeqQuery qry, const char* indexname,
			   const AjPStr queryName)
{
    SeqPCdQry wild;
    AjPList   l;
    SeqPCdTrg trgline;
    SeqPCdIdx idxline;
    SeqPCdFile idxfp;
    SeqPCdFile trgfp;
    SeqPCdFile hitfp;
    AjBool *skip;

    AjPStr fdstr    = NULL;
    AjPStr fdprefix = NULL;

    ajint t;
    ajint b;
    ajint t2;
    ajint b2;
    ajint t3;
    ajint pos = 0;
    ajint prefixlen;
    ajint start;
    ajint end;
    ajint i;
    ajint j;
    ajint k;
    ajint cmp;
    AjBool match;

    AjBool first;
    char   *name;

    SeqPCdEntry entry;


    wild    = qry->QryData;
    l       = wild->List;
    trgline = wild->trgLine;
    idxline = wild->idxLine;
    idxfp   = wild->ifp;
    trgfp   = wild->trgfp;
    hitfp   = wild->hitfp;
    skip    = wild->Skip;


    if(!seqCdTrgOpen(qry->IndexDir, indexname, &trgfp, &hitfp))
	return 0;

    /* fdstr is the original query string, in uppercase */

    /* fdprefix is the fixed (no wildcard) prefix of fdstr */

    ajStrAssignS(&fdstr,queryName);
    ajStrFmtUpper(&fdstr);
    ajStrAssignS(&fdprefix,fdstr);

    ajStrRemoveWild(&fdprefix);

    ajDebug("queryName '%S' fdstr '%S' fdprefix '%S'\n",
	    queryName, fdstr, fdprefix);
    b = b2 = 0;
    t = t2 = t3 = trgfp->NRecords - 1;

    prefixlen = ajStrGetLen(fdprefix);
    first = ajTrue;

    if(prefixlen)
    {
	/*
	 ** (1a) we have a prefix (no wildcard at the start)
	 ** look for the prefix fdprefix
	 ** Set range of records that match (will be consecutive of course)
	 ** from first match
	 */

	while(b<=t)
	{
	    pos = (t+b)/2;
	    name = seqCdTrgName(pos,trgfp);
	    name[prefixlen]='\0';      /* truncate to prefix length */
	    cmp = ajStrCmpC(fdprefix,name);
	    /*	    match = ajStrMatchWildC(fdstr,name);*/
	    ajDebug(" trg testc %d '%s' '%S' %B (+/- %d)\n",
		    pos,name,fdprefix,cmp, t-b);
	    if(!cmp)
	    {
		ajDebug(" trg hit %d\n",pos);
		if(first)
		{
		    first = ajFalse;
		    t2 = t;
		    t3 = pos;
		}
		b2 = pos;
	    }
	    if(cmp>0)
		b = pos+1;
	    else
		t = pos-1;
	}

	if(first)
	{
	    ajStrDel(&fdprefix);
	    ajStrDel(&fdstr);
	    seqCdTrgClose(&trgfp,&hitfp);
	    return ajFalse;
	}
	ajDebug("first pass: pos:%d b2:%d t2:%d\n",pos,b2,t2);

	/*
	 ** (1b) Process below
	 */

	b = b2-1;
	t = t2;
	while(b<=t)
	{
	    pos = (t+b)/2;
	    name = seqCdTrgName(pos,trgfp);
	    name[prefixlen]='\0';
	    cmp = ajStrCmpC(fdprefix,name);
	    /* match = ajStrMatchWildC(fdstr,name); */
	    ajDebug(" trg testd %d '%s' '%S' %B (+/- %d)\n",
		    pos,name,fdprefix,cmp,t-b);
	    if(!cmp)
	    {
		ajDebug(" trg hit %d\n",pos);
		t3 = pos;
	    }
	    if(cmp<0)
		t = pos-1;
	    else
		b = pos+1;
	}

	ajDebug("second pass: pos:%d b2:%d t3:%d\n",pos,b2,t3);
	name = seqCdTrgName(b2,trgfp);
	ajDebug("first %d '%s'\n",b2,name);
	name = seqCdTrgName(t3,trgfp);
	ajDebug("last %d '%s'\n",t3,name);
    }


    start = b2;
    end   = t3;
    for(i=start;i<(end+1);++i)
    {
	name = seqCdTrgName(i,trgfp);
	match = ajCharMatchWildC(name, ajStrGetPtr(fdstr));

	ajDebug("third pass: match:%B i:%d name '%s' queryName '%S'\n",
		match, i, name, fdstr);
	if(!match) continue;

	seqCdTrgLine(trgline, i, trgfp);
	seqCdFileSeek(hitfp,trgline->FirstHit-1);
	ajDebug("Query First: %d Count: %d\n",
		trgline->FirstHit, trgline->NHits);
	pos = trgline->FirstHit;

	for(j=0;j<(ajint)trgline->NHits;++j)
	{
	    seqCdFileReadInt(&k,hitfp);
	    --k;
	    ajDebug("hitlist[%d] entry = %d\n",j,k);
	    seqCdIdxLine(idxline,k,idxfp);

	    if(!skip[idxline->DivCode-1])
	    {
		AJNEW0(entry);
		entry->div = idxline->DivCode;
		entry->annoff = idxline->AnnOffset;
		entry->seqoff = idxline->SeqOffset;
		ajListPushAppend(l,(void*)entry);
	    }
	    else
		ajDebug("SKIP: token '%S' [file %d]\n",
			queryName,idxline->DivCode);
	}
    }

    seqCdTrgClose(&trgfp, &hitfp);


    ajStrDel(&trgline->Target);
    ajStrDel(&fdstr);
    ajStrDel(&fdprefix);

    return ajListGetLength(l);
}




/* @func ajSeqPrintAccess *****************************************************
**
** Reports the internal data structures
**
** @param [u] outf [AjPFile] Output file
** @param [r] full [AjBool] Full report (usually ajFalse)
** @return [void]
** @@
******************************************************************************/

void ajSeqPrintAccess(AjPFile outf, AjBool full)
{
    ajint i = 0;

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# sequence access methods\n");
    ajFmtPrintF(outf, "# Name       Alias Entry Query   All Description\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "method {\n");
    for(i=0; seqAccess[i].Name; i++) {
	if(full || !seqAccess[i].Alias)
	    ajFmtPrintF(outf, "  %-10s %5B %5B %5B %5B \"%s\"\n",
			seqAccess[i].Name, seqAccess[i].Alias,
			seqAccess[i].Entry, seqAccess[i].Query,
			seqAccess[i].All, seqAccess[i].Desc);
    }

    ajFmtPrintF(outf, "}\n\n");

    return;
}



/* @funcstatic seqCdIdxDel ****************************************************
**
** Destructor for SeqPCdIdx
**
** @param [d] pthys [SeqPCdIdx*] Cd index object
** @return [void]
******************************************************************************/

static void seqCdIdxDel(SeqPCdIdx* pthys)
{
    SeqPCdIdx thys = *pthys;
    if(!thys) return;

    ajStrDel(&thys->EntryName);
    AJFREE(*pthys);
}


/* @funcstatic seqCdTrgDel ****************************************************
**
** Destructor for SeqPCdTrg
**
** @param [d] pthys [SeqPCdTrg*] Cd index target object
** @return [void]
******************************************************************************/

static void seqCdTrgDel(SeqPCdTrg* pthys)
{
    SeqPCdTrg thys = *pthys;
    if(!thys) return;

    ajStrDel(&thys->Target);
    AJFREE(*pthys);
}





/* @func ajSeqDbExit **********************************************************
**
** Cleans up sequence database processing internal memory
**
** @return [void]
** @@
******************************************************************************/

void ajSeqDbExit(void)
{
    ajRegFree(&seqCdDivExp);
    ajRegFree(&seqBlastDivExp);
    ajRegFree(&seqRegHttpProxy);
    ajRegFree(&seqRegHttpUrl);
    ajCharDel(&seqCdName);
    ajRegFree(&seqRegGcgId);
    ajRegFree(&seqRegGcgCont);
    ajRegFree(&seqRegGcgId2);
    ajRegFree(&seqRegGcgSplit);
    ajRegFree(&seqRegEntrezCount);
    ajRegFree(&seqRegEntrezId);
    ajRegFree(&seqRegGcgRefId);

    ajRegFree(&seqRegGi);

    return;
}

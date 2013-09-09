/* @source ajtextdb ***********************************************************
**
** AJAX TEXTDB (database) functions
**
** These functions control all aspects of AJAX text-based database access
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.80 $
** @modified Sep 27 pmr First version using code originally in ajseqdb
** @modified $Date: 2013/07/15 21:22:07 $ by $Author: rice $
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

#include "ajtextdb.h"
#include "ajtextread.h"

#include "ajtable.h"
#include "ajfileio.h"
#include "ajindex.h"
#include "ajsoap.h"
#include "ajcall.h"
#include "ajreg.h"
#include "ajhttp.h"
#include "ajquery.h"
#include "ajnam.h"
#include "ajseq.h"

#include <limits.h>
#include <stdarg.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>


#ifndef WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <netdb.h>

#include <dirent.h>
#include <unistd.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#endif


static AjPRegexp textCdDivExp = NULL;
static AjPRegexp textRegHttpUrl = NULL;

static AjPRegexp textRegEntrezCount = NULL;
static AjPRegexp textRegEntrezId = NULL;

static AjPRegexp textRegGi = NULL;

static AjPRegexp textRegDbfetchErr = NULL;

static char* textCdName = NULL;
static ajuint textCdMaxNameSize = 0;

static AjPStr textObdaName = NULL;
static AjPStr textObdaTmpName = NULL;




/* @datastatic TextPObdaFile **************************************************
**
** OBDA division file record structure
**
** @alias TextSObdaFile
** @alias TextOObdaFile
**
** @attr File [AjPFile] File
** @attr NRecords [ajulong] Index record count
** @attr RecSize [ajulong] Record length (for calculating record offsets)
** @@
******************************************************************************/

typedef struct TextSObdaFile
{
    AjPFile File;
    ajulong NRecords;
    ajulong RecSize;
} TextOObdaFile;

#define TextPObdaFile TextOObdaFile*




/* @datastatic TextPObdaEntry *************************************************
**
** OBDA ID index file record structure
**
** @alias TextSObdaEntry
** @alias TextOObdaEntry
**
** @attr div [ajuint] config file record
** @attr annoff [ajulong] data file offset
** @@
******************************************************************************/

typedef struct TextSObdaEntry
{
    ajuint div;
    ajulong annoff;
} TextOObdaEntry;

#define TextPObdaEntry TextOObdaEntry*




/* @datastatic TextPCdDiv ******************************************************
**
** EMBLCD division file record structure
**
** @alias TextSCdDiv
** @alias TextOCdDiv
**
** @attr FileName [AjPStr] Filename(s)
** @attr DivCode [ajuint] Division code
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct TextSCdDiv
{
    AjPStr FileName;
    ajuint DivCode;
    char Padding[4];
} TextOCdDiv;

#define TextPCdDiv TextOCdDiv*




/* @datastatic TextPCdEntry ***************************************************
**
** EMBLCD entrynam.idx file record structure
**
** @alias TextSCdEntry
** @alias TextOCdEntry
**
** @attr div [ajuint] division file record
** @attr annoff [ajuint] data file offset
** @@
******************************************************************************/

typedef struct TextSCdEntry
{
    ajuint div;
    ajuint annoff;
} TextOCdEntry;

#define TextPCdEntry TextOCdEntry*




/* @datastatic TextPCdFHeader *************************************************
**
** EMBLCD index file header structure, same for all index files.
**
** @alias TextSCdFHeader
** @alias TextOCdFHeader
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

typedef struct TextSCdFHeader
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
} TextOCdFHeader;

#define TextPCdFHeader TextOCdFHeader*




/* @datastatic TextPCdFile ****************************************************
**
** EMBLCD file data structure
**
** @alias TextSCdFile
** @alias TextOCdFile
**
** @attr Header [TextPCdFHeader] Header data
** @attr File [AjPFile] File
** @attr NRecords [ajuint] Number of records
** @attr RecSize [ajuint] Record length (for calculating record offsets)
** @@
******************************************************************************/

typedef struct TextSCdFile
{
    TextPCdFHeader Header;
    AjPFile File;
    ajuint NRecords;
    ajuint RecSize;
} TextOCdFile;

#define TextPCdFile TextOCdFile*




/* @datastatic TextPCdHit *****************************************************
**
** EMBLCD hit file record structure
**
** @alias TextSCdHit
** @alias TextOCdHit
**
** @attr HitList [ajuint*] Array of hits, as record numbers in the
**                         entrynam.idx file
** @attr NHits [ajuint] Number of hits in HitList array
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct TextSCdHit
{
    ajuint* HitList;
    ajuint NHits;
    char Padding[4];
} TextOCdHit;

#define TextPCdHit TextOCdHit*




/* @datastatic TextPObdaIdx ***************************************************
**
** EMBLCD entryname index file record structure
**
** @alias TextSObdaIdx
** @alias TextOObdaIdx
**
** @attr AnnOffset [ajulong] Data file offset (see DivCode)
** @attr EntryName [AjPStr] Entry ID - the file is sorted by these
** @attr DivCode [ajuint] Division file record
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct TextObdaIdx
{
    ajulong AnnOffset;
    AjPStr EntryName;
    ajuint DivCode;
    char Padding[4];
} TextOObdaIdx;

#define TextPObdaIdx TextOObdaIdx*




/* @datastatic TextPObdaSecidx ************************************************
**
** OBDA secondary index file record structure
**
** @alias TextSObdaSecidx
** @alias TextOObdaSecidx
**
** @attr Target [AjPStr] Indexed target string (the file is sorted by these)
** @@
******************************************************************************/

typedef struct TextSObdaSecidx
{
    AjPStr Target;
} TextOObdaSecidx;

#define TextPObdaSecidx TextOObdaSecidx*




/* @datastatic TextPCdIdx *****************************************************
**
** EMBLCD entryname index file record structure
**
** @alias TextSCdIdx
** @alias TextOCdIdx
**
** @attr AnnOffset [ajuint] Data file offset (see DivCode)
** @attr EntryName [AjPStr] Entry ID - the file is sorted by these
** @attr DivCode [short] Division file record
** @attr Padding [char[6]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct TextCdIdx
{
    ajuint AnnOffset;
    AjPStr EntryName;
    short DivCode;
    char Padding[6];
} TextOCdIdx;

#define TextPCdIdx TextOCdIdx*




/* @datastatic TextPCdTrg *****************************************************
**
** EMBLCD target (.trg) file record structure
**
** @alias TextSCdTrg
** @alias TextOCdTrg
**
** @attr FirstHit [ajuint] First hit record in .hit file
** @attr NHits [ajuint] Number of hit records in .hit file
** @attr Target [AjPStr] Indexed target string (the file is sorted by these)
** @@
******************************************************************************/

typedef struct TextSCdTrg
{
    ajuint FirstHit;
    ajuint NHits;
    AjPStr Target;
} TextOCdTrg;

#define TextPCdTrg TextOCdTrg*




/* @datastatic TextPCdQry ******************************************************
**
** EMBLCD query structure
**
** @alias TextSCdQry
** @alias TextOCdQry
**
** @attr divfile [AjPStr] division.lkp
** @attr idxfile [AjPStr] entryname.idx
** @attr datfile [AjPStr] main data reference
** @attr dfp [TextPCdFile] division.lkp
** @attr ifp [TextPCdFile] entryname.idx
** @attr trgfp [TextPCdFile] acnum.trg
** @attr hitfp [TextPCdFile] acnum.hit
** @attr trgLine [TextPCdTrg]acnum input line
** @attr name [char*] filename from division.lkp
** @attr nameSize [ajuint] division.lkp filename length
** @attr div [ajuint] current division number
** @attr maxdiv [ajuint] max division number
** @attr Samefile [AjBool] true if the same file is used
** @attr libt [AjPFile] main data file
** @attr Skip [AjBool*] skip file(s) in division.lkp
** @attr idxLine [TextPCdIdx] entryname.idx input line
** @@
******************************************************************************/

typedef struct TextSCdQry
{
    AjPStr divfile;
    AjPStr idxfile;
    AjPStr datfile;

    TextPCdFile dfp;
    TextPCdFile ifp;
    TextPCdFile trgfp;
    TextPCdFile hitfp;
    TextPCdTrg trgLine;

    char* name;
    ajuint nameSize;
    ajuint div;
    ajuint maxdiv;

    AjBool Samefile;

    AjPFile libt;

    AjBool* Skip;
    TextPCdIdx idxLine;
} TextOCdQry;

#define TextPCdQry TextOCdQry*




/* @datastatic TextPObdaQry ***************************************************
**
** OBDA query structure
**
** @alias TextSEmbossQry
** @alias TextOEmbossQry
**
** @attr divfile  [AjPStr] division.lkp
** @attr idxname  [AjPStr] primary index file name
** @attr files    [AjPStr*]    Database filenames
** @attr idxfiles [AjPStr*]    Index filenames
** @attr Skip     [AjBool*]    Files numbers to exclude
** @attr dfp [TextPObdaFile] config.dat
** @attr ifp [TextPObdaFile] key_ID.key
** @attr trgfp [TextPObdaFile] id_ACC.index
** @attr idxLine [TextPObdaIdx]  key_ID.key input line
** @attr trgLine [TextPObdaSecidx] secondary index input line
** @attr libt     [AjPFile]    Primary (database source) file
** @attr nfiles   [ajuint]     Number of data files
** @attr nindex   [ajuint]     Number of index files
** @attr ifile    [ajuint]     Division number of currently open database file
** @attr nentries [ajint]      Number of entries in the filename array(s)
**                               -1 when done
** @attr Samefile [AjBool]     True if the same file is passed to
**                               ajFilebuffReopenFile
** @attr Padding  [char[4]]    Padding to alignment boundary
** @@
******************************************************************************/

typedef struct TextSObdaQry
{
    AjPStr divfile;
    AjPStr idxname;
    AjPStr *files;
    AjPStr *idxfiles;
    AjBool *Skip;

    TextPObdaFile dfp;
    TextPObdaFile ifp;
    TextPObdaFile trgfp;

    TextPObdaIdx idxLine;
    TextPObdaSecidx trgLine;
    AjPFile libt;
    
    ajuint nfiles;
    ajuint nindex;
    ajuint ifile;
    ajint nentries;
    AjBool Samefile;
    char Padding[4];
} TextOObdaQry;

#define TextPObdaQry TextOObdaQry*




/* @datastatic TextPEmbossQry *************************************************
**
** B+tree 'emboss' query structure
**
** @alias TextSEmbossQry
** @alias TextOEmbossQry
**
** @attr idcache  [AjPBtcache] ID cache
** @attr Caches   [AjPList]    Caches for each query field
** @attr files    [AjPStr*]    Database filenames
** @attr Skip     [AjBool*]    Files numbers to exclude
** @attr libt     [AjPFile]    Primary (database source) file
** @attr div      [ajuint]     Division number of currently open database file
** @attr nentries [ajint]      Number of entries in the filename array(s)
**                               -1 when done
** @attr Samefile [AjBool]     True if the same file is passed to
**                               ajFilebuffReopenFile
** @attr Padding  [char[4]]    Padding to alignment boundary
** @@
******************************************************************************/

typedef struct TextSEmbossQry
{
    AjPBtcache idcache;
    AjPList Caches;

    AjPStr *files;
    AjBool *Skip;

    AjPFile libt;
    
    ajuint div;
    ajint nentries;
    AjBool Samefile;
    char Padding[4];
} TextOEmbossQry;

#define TextPEmbossQry TextOEmbossQry*





#ifdef HAVE_AXIS2C


static AjPList textEbeyeGetdomainsreferencedinentry(
					axis2_svc_client_t* client,
					const axutil_env_t * env,
					const AjPStr domain,
					const AjPStr entry);
static AjPList textEbeyeGetdomainsreferencedinentryParse(
					axiom_node_t *wsResult,
					const axutil_env_t *env);
static axiom_node_t* textEbeyeGetdomainsreferencedinentryPayload(
					const axutil_env_t * env,
					const AjPStr domain,
					const AjPStr entry);


static void textEbeyeGetreferencedentries(
					axis2_svc_client_t* client,
					const axutil_env_t* env,
					const AjPStr domain,
					AjPFilebuff buff,
					const AjPStr entry);
static AjPStr textEbeyeGetreferencedentriesParse(
					axiom_node_t *wsResult,
					const axutil_env_t *env);
static axiom_node_t* textEbeyeGetreferencedentriesPayload(
					const axutil_env_t * env,
                                        const AjPStr domain,
                                        const AjPStr entry,
                                        const AjPStr refdomain);


static ajint textEbeyeGetnumberofresultsParse(
					axiom_node_t *wsResult,
				        const axutil_env_t *env);
static axiom_node_t* textEbeyeGetnumberofresultsPayload(
					const axutil_env_t * env,
					const AjPStr domain,
					const AjPStr query);


static void textEbeyeGetresultsParse(axiom_node_t *wsResult,
				        const axutil_env_t *env,
				        const AjPStr domain,
				        const AjPStr retfields,
				        AjPFilebuff buff);
static axiom_node_t* textEbeyeGetresultsPayload(
					const axutil_env_t * env,
                                        const AjPStr domain,
                                        const AjPStr query,
                                        const AjPStr fields,
                                        ajint start);


static AjPStr        textWsdbfetchFetchData(const AjPStr db,
					    const AjPStr id,
	                                    const AjPStr url,
	                                    const AjPStr format);
static axiom_node_t* textWsdbfetchFetchdataPayload(
					    const axutil_env_t * env,
                                            const AjPStr db,
                                            const AjPStr id,
                                            const AjPStr format);

static char*         textWsdbfetchGetEntryPtr(axiom_node_t *wsResult,
                                              const axutil_env_t *env);


#endif

static AjBool      textObdaConfigOpen(AjPQuery qry);
static AjBool      textAccessApp(AjPTextin textin);
/* static AjBool     textAccessCmd(AjPTextin textin);*/ /* not implemented */
static AjBool      textAccessDbfetch(AjPTextin textin);
static AjBool      textAccessDirect(AjPTextin textin);
static AjBool      textAccessEBeye(AjPTextin textin);
static AjBool      textAccessEmblcd(AjPTextin textin);
static AjBool      textAccessEmboss(AjPTextin textin);
static AjBool      textAccessEntrez(AjPTextin textin);
static AjBool      textAccessFreeEmblcd(void* qry);
static AjBool      textAccessFreeEmboss(void* qry);
static AjBool      textAccessFreeObda(void* qry);
static AjBool      textAccessMrs5(AjPTextin textin);
static AjBool      textAccessMrs4(AjPTextin textin);
static AjBool      textAccessMrs3(AjPTextin textin);
/* static AjBool      textAccessNbrf(AjPTextin textin); */ /* obsolete */
static AjBool      textAccessObda(AjPTextin textin);
static AjBool      textAccessSeqhound(AjPTextin textin);
static AjBool      textAccessSrs(AjPTextin textin);
static AjBool      textAccessSrsfasta(AjPTextin textin);
static AjBool      textAccessSrswww(AjPTextin textin);
static AjBool      textAccessUrl(AjPTextin textin);
static AjBool      textAccessWsdbfetch(AjPTextin textin);

static void        textCdIdxDel(TextPCdIdx* pthys);
static void        textCdTrgDel(TextPCdTrg* pthys);

static void        textObdaIdxDel(TextPObdaIdx* pthys);
static void        textObdaSecDel(TextPObdaSecidx* pthys);

static AjBool      textCdAll(AjPTextin textin);
static int         textObdaEntryCmp(const void* a, const void* b);
static int         textCdEntryCmp(const void* a, const void* b);
static void        textCdEntryDel(void** pentry, void* cl);
static void        textCdFileClose(TextPCdFile *thys);
static TextPObdaFile textObdaFileOpen(const AjPStr dir, const AjPStr name,
                                      AjPStr* fullname);
static TextPCdFile textCdFileOpen(const AjPStr dir, const char* name,
                                  AjPStr* fullname);
static ajint       textObdaFileSeek(TextPObdaFile fil, ajulong ipos);
static ajint       textCdFileSeek(TextPCdFile fil, ajuint ipos);
static void        textCdIdxLine(TextPCdIdx idxLine,  ajuint ipos,
                            TextPCdFile fp);
static void        textObdaIdxLine(TextPObdaIdx idxLine,  ajulong ipos,
                                   TextPObdaFile fp);
static const AjPStr textObdaIdxName(ajulong ipos, TextPObdaFile fp);
static char*       textCdIdxName(ajuint ipos, TextPCdFile fp);
static AjBool      textCdIdxQuery(AjPQuery qry, const AjPStr idqry);
static ajuint      textCdIdxSearch(TextPCdIdx idxLine, const AjPStr entry,
				 TextPCdFile fp);
static AjBool      textObdaIdxQuery(AjPQuery qry, const AjPStr idqry);
static ajlong      textObdaIdxSearch(TextPObdaIdx idxLine, const AjPStr entry,
                                     TextPObdaFile fp);
static AjBool      textCdQryClose(AjPQuery qry);
static AjBool      textCdQryEntry(AjPQuery qry);
static AjBool      textObdaQryFile(AjPQuery qry);
static AjBool      textCdQryFile(AjPQuery qry);
static AjBool      textCdQryOpen(AjPQuery qry);
static AjBool      textCdQryNext(AjPQuery qry);
static AjBool      textCdQryQuery(AjPQuery qry);
static AjBool      textCdQryReuse(AjPQuery qry);
static AjBool      textCdReadHeader(TextPCdFile fp);
static AjBool      textObdaSecClose(TextPObdaFile *trgfil);
static AjBool      textCdTrgClose(TextPCdFile *trgfil, TextPCdFile *hitfil);
static ajuint      textObdaSecFind(AjPQuery qry, const char* indexname,
                                  const AjPStr qrystring);
static ajuint      textCdTrgFind(AjPQuery qry, const char* indexname,
			       const AjPStr qrystring);
static void        textCdTrgLine(TextPCdTrg trgLine, ajuint ipos,
			       TextPCdFile fp);
static void        textObdaSecLine(TextPObdaSecidx trgLine, ajulong ipos,
                                   TextPObdaFile fp);
static const AjPStr textObdaSecName(ajulong ipos, TextPObdaFile fp);
static char*       textCdTrgName(ajuint ipos, TextPCdFile fp);
static AjBool      textCdTrgOpen(const AjPStr dir, const char* name,
			       TextPCdFile *trgfil, TextPCdFile *hitfil);
static AjBool      textObdaSecOpen(const AjPStr dir, const char* name,
                                   TextPObdaFile *trgfil);
static AjBool      textObdaSecQuery(AjPQuery qry, const AjPStr field,
                                    const AjPStr wildqry);
static AjBool      textCdTrgQuery(AjPQuery qry, const AjPStr field,
                                const AjPStr wildqry);
static ajint      textCdTrgSearch(TextPCdTrg trgLine, const AjPStr name,
				TextPCdFile fp);
static ajlong      textObdaSecSearch(TextPObdaSecidx trgLine, const AjPStr name,
                                     TextPObdaFile fp);

static AjBool      textEmbossAll(AjPTextin textin);

static AjBool	   textEmbossOpenCache(AjPQuery qry, const char* ext,
				     AjPBtcache *cache);
static AjBool      textEmbossQryOpen(AjPQuery qry);
static AjBool      textEmbossQryClose(AjPQuery qry);
static AjBool      textEmbossQryEntry(AjPQuery qry);
static AjBool      textEmbossQryNamespace(AjPQuery qry);
static AjBool      textEmbossQryNext(AjPQuery qry);
static AjBool      textEmbossQryOrganisms(AjPQuery qry);
static AjBool      textEmbossQryQuery(AjPQuery qry);
static AjBool      textEmbossQryReuse(AjPQuery qry);

static AjBool      textObdaAll(AjPTextin textin);

static AjBool      textObdaQryOpen(AjPQuery qry);
static AjBool      textObdaQryClose(AjPQuery qry);
static AjBool      textObdaQryEntry(AjPQuery qry);
static AjBool      textObdaQryNext(AjPQuery qry);
static AjBool      textObdaQryQuery(AjPQuery qry);
static AjBool      textObdaQryReuse(AjPQuery qry);

static AjBool      textEntrezQryNext(AjPQuery qry, AjPTextin textin);
static AjBool      textSeqhoundQryNext(AjPQuery qry, AjPTextin textin);
static void        textSocketTimeout(int sig);





/* @funclist textAccess *******************************************************
**
** Functions to access each database or text access method
**
******************************************************************************/

static AjOTextAccess textAccess[] =
{
  /* Name      AccessFunction   FreeFunction
     Qlink,   Description
     Alias    Entry    Query    All      Chunk   Padding*/

    {
      "dbfetch", &textAccessDbfetch, NULL,
      "|",      "retrieve in text format from EBI dbfetch REST services",
      AJFALSE, AJTRUE,  AJFALSE, AJFALSE, AJFALSE, AJFALSE
    },
    {
      "wsdbfetch", &textAccessWsdbfetch, NULL,
      "|",      "retrieve in range of formats from EBI WSDbfetch SOAP services",
      AJFALSE, AJTRUE,  AJTRUE, AJFALSE, AJTRUE, AJFALSE
    },
    {
      "ebeye", &textAccessEBeye, NULL,
      "|&!",      "retrieve selected information from EBIeye SOAP services",
      AJFALSE, AJTRUE,  AJTRUE, AJFALSE, AJTRUE, AJFALSE
    },
    {
      "emboss",  &textAccessEmboss, &textAccessFreeEmboss,
      "|&!^=", "dbx program indexed",
      AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  AJFALSE, AJFALSE
    },
    {
      "emblcd", 	 &textAccessEmblcd, &textAccessFreeEmblcd,
      "|&!^=", "use EMBL-CD index from dbi programs or Staden",
      AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  AJFALSE, AJFALSE
    },
    {
      "entrez",	 &textAccessEntrez, NULL,
      "",      "use NCBI Entrez services",
      AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  AJFALSE, AJFALSE
    },
    {
      "seqhound",  &textAccessSeqhound, NULL,
      "",      "use BluePrint seqhound services",
      AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  AJFALSE, AJFALSE
    },
    {
      "mrs",	 &textAccessMrs5, NULL,
      "",      "retrieve in text format from CMBI MRS5 server",
      AJFALSE, AJTRUE,  AJTRUE,  AJFALSE, AJFALSE, AJFALSE
    },
    {
      "mrs4",	 &textAccessMrs4, NULL,
      "",      "retrieve in text format from CMBI MRS4 server",
      AJFALSE, AJTRUE,  AJTRUE,  AJFALSE, AJFALSE, AJFALSE
    },
    {
      "mrs3",	 &textAccessMrs3, NULL,
      "",      "retrieve in text format from CMBI MRS3 server",
      AJFALSE, AJTRUE,  AJTRUE,  AJFALSE, AJFALSE, AJFALSE
    },
    {
      "obda",     &textAccessObda,   &textAccessFreeObda,
      "|&!^=", "use Open Biological Data Access index",
      AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  AJFALSE, AJFALSE
    },
    {
      "srs",	 &textAccessSrs, NULL,
      "|&!=",  "retrieve in text format from a local SRS installation",
      AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  AJFALSE, AJFALSE
    },
    {
      "srsfasta", &textAccessSrsfasta, NULL,
      "|&!=",  "retrieve in FASTA format from a local SRS installation",
      AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  AJFALSE, AJFALSE
    },
    {
      "srswww",	 &textAccessSrswww, NULL,
      "|&!=",      "retrieve in text format from an SRS webserver",
      AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  AJFALSE, AJFALSE
    },
    {
      "url",	 &textAccessUrl, NULL,
      "",      "retrieve content from a remote webserver URL",
      AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  AJFALSE, AJFALSE
    },
    {
      "app",	 &textAccessApp, NULL,
      "",      "call an external application",
      AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  AJFALSE, AJFALSE
    },
    {
      "external", &textAccessApp, NULL,
      "",      "call an external application",
      AJTRUE,  AJTRUE,  AJTRUE,  AJTRUE,  AJFALSE, AJFALSE
    },
    /* {
       "asis",	 &ajTextAccessAsis, NULL,
       "",      "",
       AJFALSE, AJTRUE, AJFALSE, AJFALSE, AJFALSE, AJFALSE
       }, */        /* called by seqUsaProcess */
    /* {
       "file",	 &ajTextAccessFile, NULL,
       "",      "",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE, AJFALSE, AJFALSE
       }, */        /* called by seqUsaProcess */
    /* {
       "offset", &ajTextAccessOffset, NULL,
       "",      "",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE, AJFALSE, AJFALSE
       }, */    /* called by seqUsaProcess */
    {
      "direct",	 &textAccessDirect, NULL,
      "DATA",  "reading the original files unindexed",
      AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  AJFALSE, AJFALSE
    },
    {
      NULL, NULL, NULL,
      NULL, NULL,
      AJFALSE, AJFALSE, AJFALSE, AJFALSE, AJFALSE, AJFALSE
    },

/* after the NULL access method, and so unreachable.
** seqhound requires a username and password which it prompts for
** interactively
*/
};




/* @func ajTextdbInit *********************************************************
**
** Initialise text database internals
**
** @return [void]
**
** @release 6.4.0
******************************************************************************/

void ajTextdbInit(void)
{
    AjPTable table;
    ajuint i = 0;

    table = ajTextaccessGetDb();

    while(textAccess[i].Name)
    {
        ajCallTableRegister(table, textAccess[i].Name, (void*) &textAccess[i]);
	i++;
    }

    return;
}




/* @section EMBL CD Database Indexing *****************************************
**
** These functions manage the EMBL CD-ROM index access methods.
** These include the "efetch" indexing used at the Sanger Centre
** based on Erik Sonnhammer's indexseqlibs code
** and a direct copy of the database and index files from the
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




/* @funcstatic textAccessEmblcd ***********************************************
**
** Reads a text entry using EMBL CD-ROM index files.
**
** @param [u] textin [AjPTextin] Text input, including query data.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textAccessEmblcd(AjPTextin textin)
{
    AjBool retval = ajFalse;

    AjPQuery qry;
    TextPCdQry qryd;

    AjIList iter = NULL;
    AjPQueryField field = NULL;

    qry = textin->Query;
    qryd = qry->QryData;

    ajDebug("textAccessEmblcd type %d\n", qry->QueryType);

    if(qry->QueryType == AJQUERY_ALL)
	return textCdAll(textin);

    /* we need to search the index files and return a query */

    if(qry->QryData)
    {				        /* reuse unfinished query data */
	if(!textCdQryReuse(qry))        /* oops, we're finished        */
	    return ajFalse;
    }
    else
    {				        /* starting out, set up query */
	textin->Single = ajTrue;

	if(!textCdQryOpen(qry))
	{
	    ajWarn("Failed to open index for database '%S'",
                    qry->DbName);

	    return ajFalse;
	}

	qryd = qry->QryData;

	/* binary search for the entryname we need */

	if(qry->QueryType == AJQUERY_ENTRY)
	{
	    ajDebug("entry fields: %Lu hasacc:%B\n",
		    ajListGetLength(qry->QueryFields), qry->HasAcc);
	    if(!textCdQryEntry(qry))
	    {
		ajDebug("EMBLCD Entry failed\n");

                iter = ajListIterNewread(qry->QueryFields);
                while(!ajListIterDone(iter))
                {
                    field = ajListIterGet(iter);
		    ajDebug("Database Entry '%S:%S' not found\n",
                            field->Field, field->Wildquery);
                }
                ajListIterDel(&iter);
	    }
	}

	if(qry->QueryType == AJQUERY_QUERY)
	{
	    ajDebug("query fields: %Lu hasacc:%B\n",
		    ajListGetLength(qry->QueryFields), qry->HasAcc);

	    if(!textCdQryQuery(qry))
	    {
		ajDebug("EMBLCD Query failed\n");

                iter = ajListIterNewread(qry->QueryFields);
                while(!ajListIterDone(iter))
                {
                    field = ajListIterGet(iter);
		    ajDebug("Database Query '%S:%S' not found\n",
                            field->Field, field->Wildquery);
                }
                ajListIterDel(&iter);
	    }
	}
    }

    if(ajListGetLength(qry->ResultsList))
    {
	retval = textCdQryNext(qry);

	if(retval)
        {
            if(qryd->Samefile)
                ajFilebuffClear(textin->Filebuff, -1);
            else
                ajFilebuffReopenFile(&textin->Filebuff, qryd->libt);
        }
    }

    if(!ajListGetLength(qry->ResultsList)) /* could be emptied by code above */
    {
	textCdQryClose(qry);

	/* AJB addition */
        /*
	 * This was for the old code where seqsets had different
         * memory handling ... and the reason for the multi
         * flag in the first place. So far this seems
         * unnecessary for the revised code but is left here
         * for a while as a reminder and 'just in case'
	 */
	if((qry->QueryType == AJQUERY_ENTRY) && !textin->Multi)
	{
	    AJFREE(qry->QryData);
	    qryd = NULL;
	}
    }

    ajStrAssignS(&textin->Db, qry->DbName);

    return retval;
}




/* @funcstatic textAccessFreeEmblcd *******************************************
**
** Frees data specific to reading EMBL CD-ROM index files.
**
** @param [r] qry [void*] query data specific to EMBLCD
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textAccessFreeEmblcd(void* qry)
{
    TextPCdQry qryd;
    AjPQuery query;
    AjBool retval = ajTrue;

    ajDebug("textAccessFreeEmblcd\n");

    query = (AjPQuery) qry;
    qryd = query->QryData;
    qryd->libt=0;

    textCdQryClose(query);

    return retval;
}




/* @funcstatic textCdAll ******************************************************
**
** Reads the EMBLCD division lookup file and opens a list of all the
** database files for plain reading.
**
** @param [u] textin [AjPTextin] Text input.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textCdAll(AjPTextin textin)
{
    AjPStr divfile = NULL;
    TextPCdFile dfp;
    AjPList list;
    AjPQuery qry;

    ajint i;
    short j;
    ajint nameSize;
    char *name;
    AjPStr fullName = NULL;

    qry = textin->Query;

    if(!ajStrGetLen(qry->IndexDir))
    {
	ajDebug("no indexdir defined for database %S\n", qry->DbName);
	ajErr("no indexdir defined for database %S", qry->DbName);

	return ajFalse;
    }

    ajDebug("EMBLCD All index directory '%S'\n", qry->IndexDir);

    dfp = textCdFileOpen(qry->IndexDir, "division.lkp", &divfile);
    ajStrDel(&divfile);

    if(!dfp)
    {
	ajWarn("Cannot open division file in '%S' for database '%S'",
	       qry->IndexDir, qry->DbName);

	return ajFalse;
    }

    nameSize = dfp->RecSize - 2;
    name = ajCharNewRes(nameSize+1);

    list = ajListstrNew();

    textCdFileSeek(dfp, 0);

    for(i=0; i < (ajint) dfp->Header->NRecords; i++)
    {
	ajReadbinInt2(dfp->File, &j);
	ajReadbinCharTrim(dfp->File, nameSize, name);
	fullName = ajStrNewC(name);
	ajFilenameReplacePathS(&fullName, qry->Directory);

	/* test exclusion list and add file if OK */

	if(ajFilenameTestInclude(fullName, qry->Exclude, qry->Filename))
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

    ajFilebuffDel(&textin->Filebuff);
    textin->Filebuff = ajFilebuffNewListinList(list);
    fullName = NULL;

    ajStrAssignS(&textin->Db, qry->DbName);

    textCdFileClose(&dfp);
    ajStrDelStatic(&divfile);
    ajCharDel(&name);

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    return ajTrue;
}




/* @funcstatic textCdFileOpen *************************************************
**
** Opens a named EMBL CD-ROM index file.
**
** @param [r] dir [const AjPStr] Directory
** @param [r] name [const char*] File name.
** @param [w] fullname [AjPStr*] Full file name with directory path
** @return [TextPCdFile] EMBL CD-ROM index file object.
**
** @release 6.4.0
** @@
******************************************************************************/

static TextPCdFile textCdFileOpen(const AjPStr dir, const char* name,
                                  AjPStr* fullname)
{
    TextPCdFile thys = NULL;


    AJNEW0(thys);

    thys->File = ajFileNewInNamePathC(name, dir);

    if(!thys->File)
    {
	AJFREE(thys);

	return NULL;
    }


    AJNEW0(thys->Header);

    textCdReadHeader(thys);
    thys->NRecords = thys->Header->NRecords;
    thys->RecSize = thys->Header->RecSize;

    ajStrAssignS(fullname, ajFileGetPrintnameS(thys->File));

    ajDebug("textCdFileOpen '%F' NRecords: %d RecSize: %d\n",
	    thys->File, thys->NRecords, thys->RecSize);


    return thys;
}




/* @funcstatic textObdaFileSeek ***********************************************
**
** Sets the file position in an OBDA index file.
**
** @param [u] fil [TextPObdaFile] EMBL CD-ROM index file object.
** @param [r] ipos [ajulong] Offset.
** @return [ajint] Return value from the seek operation.
**
** @release 6.5.0
** @@
******************************************************************************/


static ajint textObdaFileSeek(TextPObdaFile fil, ajulong ipos)
{
    ajint ret;
    ajulong jpos;

    jpos = 4 + ipos*(fil->RecSize);
    ret = ajFileSeek(fil->File, jpos, 0);

    return ret;
}




/* @funcstatic textCdFileSeek *************************************************
**
** Sets the file position in an EMBL CD-ROM index file.
**
** @param [u] fil [TextPCdFile] EMBL CD-ROM index file object.
** @param [r] ipos [ajuint] Offset.
** @return [ajint] Return value from the seek operation.
**
** @release 6.4.0
** @@
******************************************************************************/


static ajint textCdFileSeek(TextPCdFile fil, ajuint ipos)
{
    ajint ret;
    ajuint jpos;

    jpos = 300 + ipos*fil->RecSize;
    ret = ajFileSeek(fil->File, jpos, 0);

    /*
       ajDebug("textCdFileSeek rec %u pos %u tell %Ld returns %d\n",
       ipos, jpos, ajFileResetPos(fil->File), ret);
    */

    return ret;
}




/* @funcstatic textObdaFileClose **********************************************
**
** Closes an OBDA index file.
**
** @param [d] pthis [TextPObdaFile*] EMBL OBDA index file.
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void textObdaFileClose(TextPObdaFile* pthis)
{
    TextPObdaFile thys;

    thys = *pthis;

    if(!thys)
        return;

    ajDebug("textObdaFileClose of %F\n", (*pthis)->File);

    ajFileClose(&thys->File);
    AJFREE(*pthis);

    return;
}




/* @funcstatic textCdFileClose ************************************************
**
** Closes an EMBL CD-ROM index file.
**
** @param [d] pthis [TextPCdFile*] EMBL CD-ROM index file.
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void textCdFileClose(TextPCdFile* pthis)
{
    TextPCdFile thys;

    thys = *pthis;

    if(!thys)
        return;

    ajDebug("textCdFileClose of %F\n", (*pthis)->File);

    ajFileClose(&thys->File);
    AJFREE(thys->Header);
    AJFREE(*pthis);

    return;
}




/* @funcstatic textCdIdxSearch ************************************************
**
** Binary search through an EMBL CD-ROM index file for an exact match.
**
** @param [u] idxLine [TextPCdIdx] Index file record.
** @param [r] entry [const AjPStr] Entry name to search for.
** @param [u] fil [TextPCdFile] EMBL CD-ROM index file.
** @return [ajuint] Record number on success, -1 on failure.
**
** @release 6.4.0
** @@
******************************************************************************/

static ajuint textCdIdxSearch(TextPCdIdx idxLine, const AjPStr entry,
                              TextPCdFile fil)
{
    AjPStr entrystr = NULL;
    ajint ihi;
    ajint ilo;
    ajint ipos = 0;
    ajint icmp = 0;
    char *name;

    ajStrAssignS(&entrystr, entry);
    ajStrFmtUpper(&entrystr);

    ajDebug("textCdIdxSearch (entry '%S') records: %d\n",
	    entrystr, fil->NRecords);

    if(fil->NRecords < 1)
	return -1;

    ilo = 0;
    ihi = fil->NRecords - 1;

    while(ilo <= ihi)
    {
	ipos = (ilo + ihi)/2;
	name = textCdIdxName(ipos, fil);
	icmp = ajStrCmpC(entrystr, name);
	ajDebug("idx test %u '%s' %2d (+/- %u)\n", ipos, name, icmp, ihi-ilo);

	if(!icmp)
            break;

	if(icmp < 0)
	    ihi = ipos-1;
	else
	    ilo = ipos+1;
    }

    ajStrDel(&entrystr);

    if(icmp)
	return -1;

    textCdIdxLine(idxLine, ipos, fil);

    return ipos;
}




/* @funcstatic textObdaIdxSearch **********************************************
**
** Binary search through an OBDA index file for an exact match.
**
** @param [u] idxLine [TextPObdaIdx] Index file record.
** @param [r] entry [const AjPStr] Entry name to search for.
** @param [u] fil [TextPObdaFile] OBDA index file.
** @return [ajlong] Record number on success or -1 on failure.
**
** @release 6.4.0
** @@
******************************************************************************/

static ajlong textObdaIdxSearch(TextPObdaIdx idxLine, const AjPStr entry,
                                TextPObdaFile fil)
{
    AjPStr entrystr = NULL;
    ajlong ihi;
    ajlong ilo;
    ajlong ipos = 0;
    ajint icmp = 0;
    const AjPStr name;

    ajStrAssignS(&entrystr, entry);
    ajStrFmtUpper(&entrystr);

    ajDebug("textObdaIdxSearch (entry '%S') records: %u recsize: %u\n",
	    entrystr, fil->NRecords, fil->RecSize);

    ilo = 0;
    ihi = fil->NRecords - 1;

    while(ilo <= ihi)
    {
	ipos = (ilo + ihi)/2;
	name = textObdaIdxName(ipos, fil);
	icmp = ajStrCmpS(entrystr, name);
	ajDebug("idx test %u '%S' %2d (+/- %u)\n", ipos, name, icmp, ihi-ilo);

	if(!icmp)
            break;

	if(icmp < 0)
	    ihi = ipos-1;
	else
	    ilo = ipos+1;
    }

    ajStrDel(&entrystr);

    if(icmp)
	return -1;

    textObdaIdxLine(idxLine, ipos, fil);

    return ipos;
}




/* @funcstatic textCdIdxQuery *************************************************
**
** Binary search of an EMBL CD-ROM index file for entries matching a
** wildcard entry name.
**
** @param [u] qry [AjPQuery] Text query object.
** @param [r] idqry [const AjPStr] ID Query
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textCdIdxQuery(AjPQuery qry, const AjPStr idqry)
{
    TextPCdQry qryd;

    AjPList list;
    TextPCdIdx idxLine;
    TextPCdFile fil;

    AjPStr idstr = NULL;
    AjPStr idpref = NULL;
    ajint i;
    ajint ihi;
    ajint ilo;
    ajint ipos = 0;
    ajint icmp;
    char *name;
    ajuint ilen;
    ajuint idlen;
    ajint jlo;
    ajint jhi;
    ajint khi;
    AjBool first;
    ajuint ifail = 0;
    ajuint iskip = 0;

    TextPCdEntry entry;

    ajDebug("textCdIdxQuery\n");

    qryd    = qry->QryData;
    list    = qry->ResultsList;
    idxLine = qryd->idxLine;
    fil     = qryd->ifp;

    ajStrAssignS(&idstr,idqry);
    ajStrFmtUpper(&idstr);
    ajStrAssignS(&idpref, idstr);

    ajStrRemoveWild(&idpref);

    ajDebug("textCdIdxQuery (wild '%S' prefix '%S')\n",
	    idstr, idpref);

    jlo = ilo = 0;
    khi = jhi = ihi = fil->NRecords-1;

    ilen = ajStrGetLen(idpref); /* query prefix length to truncate name */

    idlen = fil->RecSize-10;    /* maximum id length in index */
    if(idlen < ilen)
        ilen = idlen;

    first = ajTrue;

    if(ilen)
    {			       /* find first entry with this prefix */
	while(ilo <= ihi)
	{
	    ipos = (ilo + ihi)/2;
	    name = textCdIdxName(ipos, fil);
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
	    name = textCdIdxName(ipos, fil);
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

	name = textCdIdxName(jlo, fil);
	ajDebug("first  %d '%s'\n", jlo, name);
	name = textCdIdxName(khi, fil);
	ajDebug(" last  %d '%s'\n", khi, name);
    }

    for(i=jlo; i < (khi+1); i++)
    {
	textCdIdxLine(idxLine, i, fil);

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

		ajDebug("  OK: '%S' divcode %u AnnOffset %Lu\n",
                        idxLine->EntryName, idxLine->DivCode,
                        idxLine->AnnOffset);
		AJNEW0(entry);
		entry->div = idxLine->DivCode;
		entry->annoff = idxLine->AnnOffset;
		ajListPushAppend(list, (void*)entry);
                entry = NULL;
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




/* @funcstatic textObdaIdxQuery ***********************************************
**
** Binary search of an OBDA index file for entries matching a
** wildcard entry name.
**
** @param [u] qry [AjPQuery] Text query object.
** @param [r] idqry [const AjPStr] ID Query
** @return [AjBool] ajTrue on success.
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool textObdaIdxQuery(AjPQuery qry, const AjPStr idqry)
{
    TextPObdaQry qryd;

    AjPList list;
    TextPObdaIdx idxLine;
    TextPObdaFile fil;

    AjPStr idstr = NULL;
    AjPStr idpref = NULL;
    ajlong i;
    ajlong ihi;
    ajlong ilo;
    ajlong ipos = 0L;
    ajint icmp;
    const AjPStr name;
    ajuint ilen;
    ajlong jlo;
    ajlong jhi;
    ajlong khi;
    AjBool first;
    ajuint ifail = 0;
    ajuint iskip = 0;

    TextPObdaEntry entry;

    ajDebug("textObdaIdxQuery\n");

    qryd    = qry->QryData;
    list    = qry->ResultsList;
    idxLine = qryd->idxLine;
    fil     = qryd->ifp;

    ajStrAssignS(&idstr,idqry);
    ajStrFmtUpper(&idstr);
    ajStrAssignS(&idpref, idstr);

    ajStrRemoveWild(&idpref);

    ajDebug("textObdaIdxQuery (wild '%S' prefix '%S')\n",
	    idstr, idpref);

    jlo = ilo = 0L;
    khi = jhi = ihi = fil->NRecords-1;

    ilen = ajStrGetLen(idpref);
    first = ajTrue;

    if(ilen)
    {			       /* find first entry with this prefix */
	while(ilo <= ihi)
	{
	    ipos = (ilo + ihi)/2;
	    name = textObdaIdxName(ipos, fil);
	    icmp = ajStrCmpS(idpref, name); /* test prefix */
	    ajDebug("idx test %d '%S' %2d (+/- %d)\n",
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
	    name = textObdaIdxName(ipos, fil);
	    icmp = ajStrCmpS(idpref, name);
	    ajDebug("idx test %d '%S' %2d (+/- %d)\n",
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

	name = textObdaIdxName(jlo, fil);
	ajDebug("first  %d '%S'\n", jlo, name);
	name = textObdaIdxName(khi, fil);
	ajDebug(" last  %d '%S'\n", khi, name);
    }

    for(i=jlo; i < (khi+1); i++)
    {
	textObdaIdxLine(idxLine, i, fil);

	if(ajStrMatchWildS(idxLine->EntryName, idstr))
	{
	    if(!qryd->Skip[idxLine->DivCode])
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

		ajDebug("  OK: '%S' divcode %u AnnOffset %Lu\n",
                        idxLine->EntryName, idxLine->DivCode,
                        idxLine->AnnOffset);
		AJNEW0(entry);
		entry->div = idxLine->DivCode;
		entry->annoff = idxLine->AnnOffset;
		ajListPushAppend(list, (void*)entry);
                entry = NULL;
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




/* @funcstatic textCdTrgSearch ************************************************
**
** Binary search of EMBL CD-ROM target file, for example an accession number
** search.
**
** @param [u] trgLine [TextPCdTrg] Target file record.
** @param [r] entry [const AjPStr] Entry name or accession number.
** @param [u] fp [TextPCdFile] EMBL CD-ROM target file
** @return [ajint] Record number, or -1 on failure.
**
** @release 6.4.0
** @@
******************************************************************************/

static ajint textCdTrgSearch(TextPCdTrg trgLine, const AjPStr entry,
                              TextPCdFile fp)
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
    ajDebug("textCdTrgSearch '%S' recSize: %d\n", entry, fp->RecSize);
    name = textCdTrgName(ipos, fp);
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
	name = textCdTrgName(ipos, fp);
	icmp = ajStrCmpC(entrystr, name);
	ajDebug("trg testb %d '%s' %2d (+/- %d)\n",
		 ipos, name, icmp, ihi-ilo);
    }

    textCdTrgLine(trgLine, ipos, fp);

    ajStrDel(&entrystr);

    if(!trgLine->NHits)
	return -1;

    ajDebug("found in .trg at record %d\n", ipos);


    return ipos;
}




/* @funcstatic textObdaSecSearch **********************************************
**
** Binary search of an OBDA secondary index, for example an accession number
** search.
**
** @param [u] trgLine [TextPObdaSecidx] Target file record.
** @param [r] entry [const AjPStr] Entry name or accession number.
** @param [u] fp [TextPObdaFile] EMBL CD-ROM target file
** @return [ajlong] Record number, or -1 on failure.
**
** @release 6.5.0
** @@
******************************************************************************/

static ajlong textObdaSecSearch(TextPObdaSecidx trgLine, const AjPStr entry,
                                TextPObdaFile fp)
{
    AjPStr entrystr = NULL;
    ajlong ihi;
    ajlong ilo;
    ajlong ipos;
    ajint icmp;
    ajlong itry;
    const AjPStr name;

    ajStrAssignS(&entrystr, entry);
    ajStrFmtUpper(&entrystr);

    if(fp->NRecords < 1)
      return -1;

    ilo  = 0;
    ihi  = fp->NRecords;
    ipos = (ilo + ihi)/2;
    icmp = -1;
    ajDebug("textObdaSecSearch '%S' recSize: %d\n", entry, fp->RecSize);
    name = textObdaSecName(ipos, fp);
    icmp = ajStrCmpS(entrystr, name);

    ajDebug("trg testa %d '%S' %2d (+/- %d)\n", ipos, name, icmp, ihi-ilo);

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
	name = textObdaSecName(ipos, fp);
	icmp = ajStrCmpS(entrystr, name);
	ajDebug("trg testb %d '%S' %2d (+/- %d)\n",
		 ipos, name, icmp, ihi-ilo);
    }

    textObdaSecLine(trgLine, ipos, fp);

    ajStrDel(&entrystr);

    ajDebug("found in .trg at record %d\n", ipos);

    return ipos;
}




/* @funcstatic textCdIdxName **************************************************
**
** Reads the name from record ipos of an EMBL CD-ROM index file.
** The name length is known from the index file object.
**
** @param [r] ipos [ajuint] Record number.
** @param [u] fil [TextPCdFile] EMBL CD-ROM index file.
** @return [char*] Name read from file.
**
** @release 6.4.0
** @@
******************************************************************************/

static char* textCdIdxName(ajuint ipos, TextPCdFile fil)
{
    ajuint nameSize;

    nameSize = fil->RecSize-10;

    if(textCdMaxNameSize < nameSize)
    {
	textCdMaxNameSize = nameSize;
	if(textCdName)
	    ajCharDel(&textCdName);
	textCdName = ajCharNewRes(textCdMaxNameSize+1);
    }

    textCdFileSeek(fil, ipos);
    ajReadbinCharTrim(fil->File, nameSize, textCdName);

    return textCdName;
}




/* @funcstatic textObdaIdxName ************************************************
**
** Reads the name from record ipos of an OBDA index file.
** The name length is known from the index file object.
**
** @param [r] ipos [ajulong] Record number.
** @param [u] fil [TextPObdaFile] OBDA index file.
** @return [const AjPStr] Name read from file.
**
** @release 6.5.0
** @@
******************************************************************************/

static const AjPStr textObdaIdxName(ajulong ipos, TextPObdaFile fil)
{
    AjPStr token = NULL;
    AjPStrTok handle = NULL;

    textObdaFileSeek(fil, ipos);
    ajReadbinStrTrim(fil->File, (size_t) fil->RecSize, &textObdaName);

    if(ajStrParseCountC(textObdaName, "\t;") != 4)
    {
        ajErr("bad OBDA index record at %u in '%F'",
              ipos, fil->File);
        return NULL;
    }
    
    handle = ajStrTokenNewC(textObdaName, "\t;");

    ajStrTokenNextParse(handle, &textObdaTmpName);

    ajStrTokenDel(&handle);
    ajStrDel(&token);
   
    return textObdaTmpName;
}




/* @funcstatic textCdIdxLine **************************************************
**
** Reads a numbered record from an EMBL CD-ROM index file.
**
** @param [u] idxLine [TextPCdIdx] Index file record.
** @param [r] ipos [ajuint] Record number.
** @param [u] fil [TextPCdFile] EMBL CD-ROM index file.
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void textCdIdxLine(TextPCdIdx idxLine, ajuint ipos, TextPCdFile fil)
{
    ajuint nameSize;
    ajuint secoffset;

    nameSize = fil->RecSize-10;

    if(textCdMaxNameSize < nameSize)
    {
	textCdMaxNameSize = nameSize;

	if(textCdName)
	    ajCharDel(&textCdName);

	textCdName = ajCharNewRes(textCdMaxNameSize+1);
    }

    textCdFileSeek(fil, ipos);
    ajReadbinCharTrim(fil->File, nameSize, textCdName);

    ajStrAssignC(&idxLine->EntryName,textCdName);

    ajReadbinUint(fil->File, &idxLine->AnnOffset);
    ajReadbinUint(fil->File, &secoffset);
    ajReadbinInt2(fil->File, &idxLine->DivCode);

    return;
}




/* @funcstatic textObdaIdxLine ************************************************
**
** Reads a numbered record from an OBDA index file.
**
** @param [u] idxLine [TextPObdaIdx] Index file record.
** @param [r] ipos [ajulong] Record number.
** @param [u] fil [TextPObdaFile] OBDA index file.
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void textObdaIdxLine(TextPObdaIdx idxLine, ajulong ipos,
                            TextPObdaFile fil)
{
    AjPStr token = NULL;
    AjPStrTok handle = NULL;

    textObdaFileSeek(fil, ipos);

    ajReadbinStrTrim(fil->File, (size_t) fil->RecSize, &textObdaName);

    if(ajStrParseCountC(textObdaName, "\t") != 4)
    {
        ajErr("bad OBDA index record at %u in '%F'",
              ipos, fil->File);
        return;
    }
    
    handle = ajStrTokenNewC(textObdaName, "\t");

    ajStrTokenNextParse(handle, &idxLine->EntryName);
    ajStrTokenNextParse(handle, &token);
    ajStrToUint(token, &idxLine->DivCode);
    ajStrTokenNextParse(handle, &token);
    ajStrToUlong(token, &idxLine->AnnOffset);

    ajStrTokenDel(&handle);
    ajStrDel(&token);

    return;
}




/* @funcstatic textObdaSecName ************************************************
**
** Reads the target name from an OBDA secondary index
**
** @param [r] ipos [ajulong] Record number.
** @param [u] fil [TextPObdaFile] OBDA secondary index file.
** @return [const AjPStr] Name.
**
** @release 6.5.0
** @@
******************************************************************************/

static const AjPStr textObdaSecName(ajulong ipos, TextPObdaFile fil)
{
    AjPStrTok handle = NULL;

    textObdaFileSeek(fil, ipos);

    ajReadbinStrTrim(fil->File, (size_t)fil->RecSize, &textObdaName);

    handle = ajStrTokenNewC(textObdaName, "\t");

    ajStrTokenNextParse(handle, &textObdaTmpName);

    ajDebug("textObdaSecName  ipos %u name '%S'\n",
	    ipos, textObdaTmpName);

    ajStrTokenDel(&handle);
    return textObdaTmpName;
}




/* @funcstatic textCdTrgName **************************************************
**
** Reads the target name from an EMBL CD-ROM index target file.
**
** @param [r] ipos [ajuint] Record number.
** @param [u] fil [TextPCdFile] EMBL CD-ROM index target file.
** @return [char*] Name.
**
** @release 6.4.0
** @@
******************************************************************************/

static char* textCdTrgName(ajuint ipos, TextPCdFile fil)
{
    ajuint nameSize;
    ajint i;

    nameSize = fil->RecSize-8;

    if(textCdMaxNameSize < nameSize)
    {
	textCdMaxNameSize = nameSize;

	if(textCdName)
	    ajCharDel(&textCdName);

	textCdName = ajCharNewRes(textCdMaxNameSize+1);
    }

    textCdFileSeek(fil, ipos);
    ajReadbinInt(fil->File, &i);
    ajReadbinInt(fil->File, &i);
    ajReadbinCharTrim(fil->File, nameSize, textCdName);

    ajDebug("textCdTrgName maxNameSize:%d nameSize:%d name '%s'\n",
	    textCdMaxNameSize, nameSize, textCdName);

    return textCdName;
}




/* @funcstatic textObdaSecLine ************************************************
**
** Reads a line from an OBDA secondary index
**
** @param [w] trgLine [TextPObdaSecidx] Target file record.
** @param [r] ipos [ajulong] Record number.
** @param [u] fil [TextPObdaFile] OBDA index target file.
** @return [void].
**
** @release 6.5.0
** @@
******************************************************************************/

static void textObdaSecLine(TextPObdaSecidx trgLine, ajulong ipos,
                            TextPObdaFile fil)
{
    AjPStrTok handle = NULL;

    textObdaFileSeek(fil, ipos);

    ajReadbinStrTrim(fil->File, (size_t)fil->RecSize, &textObdaName);

    handle = ajStrTokenNewC(textObdaName, "\t");

    ajStrTokenNextParse(handle, &trgLine->Target);
    ajStrTokenNextParse(handle, &trgLine->Target);

    ajStrTokenDel(&handle);

    ajDebug("textObdaSecLine %d target '%S'\n",
	    ipos, trgLine->Target);

    return;
}




/* @funcstatic textCdTrgLine **************************************************
**
** Reads a line from an EMBL CD-ROM index target file.
**
** @param [w] trgLine [TextPCdTrg] Target file record.
** @param [r] ipos [ajuint] Record number.
** @param [u] fil [TextPCdFile] EMBL CD-ROM index target file.
** @return [void].
**
** @release 6.4.0
** @@
******************************************************************************/

static void textCdTrgLine(TextPCdTrg trgLine, ajuint ipos, TextPCdFile fil)
{
    ajuint nameSize;

    nameSize = fil->RecSize-8;

    if(textCdMaxNameSize < nameSize)
    {
	textCdMaxNameSize = nameSize;

	if(textCdName)
	    ajCharDel(&textCdName);

	textCdName = ajCharNewRes(textCdMaxNameSize+1);
    }

    textCdFileSeek(fil, ipos);

    ajReadbinUint(fil->File, &trgLine->NHits);
    ajReadbinUint(fil->File, &trgLine->FirstHit);
    ajReadbinCharTrim(fil->File, nameSize, textCdName);

    ajStrAssignC(&trgLine->Target,textCdName);

    ajDebug("textCdTrgLine %d nHits %d firstHit %d target '%S'\n",
	    ipos, trgLine->NHits, trgLine->FirstHit, trgLine->Target);

    return;
}




/* @funcstatic textCdReadHeader ***********************************************
**
** Reads the header of an EMBL CD-ROM index file.
**
** @param [u] fil [TextPCdFile] EMBL CD-ROM index file.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textCdReadHeader(TextPCdFile fil)
{
    ajint i;

    TextPCdFHeader header;
    char date[8]; /* ajReadbinCharTrim needs space for trailing null */

    header = fil->Header;

    ajReadbinUint(fil->File, &header->FileSize);
    ajReadbinUint(fil->File, &header->NRecords);
    ajReadbinInt2(fil->File, &header->RecSize);

    header->IdSize = header->RecSize - 10;

    ajReadbinCharTrim(fil->File, 20, header->DbName);
    ajReadbinCharTrim(fil->File, 10, header->Release);

    ajReadbinCharTrim(fil->File, 4, date);

    for(i=1;i<4;i++)
	header->Date[i] = date[i];

    header->RelYear  = header->Date[1];
    header->RelMonth = header->Date[2];
    header->RelDay   = header->Date[3];

    ajDebug("textCdReadHeader file %F\n", fil->File);
    ajDebug("  FileSize: %d NRecords: %hd recsize: %d idsize: %d\n",
	    header->FileSize, header->NRecords,
	    header->RecSize, header->IdSize);

    return ajTrue;
}




/* @funcstatic textCdTrgOpen **************************************************
**
** Opens an EMBL CD-ROM target file pair.
**
** @param [r] dir [const AjPStr] Directory.
** @param [r] name [const char*] File name.
** @param [w] trgfil [TextPCdFile*] Target file.
** @param [w] hitfil [TextPCdFile*] Hit file.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textCdTrgOpen(const AjPStr dir, const char* name,
			   TextPCdFile* trgfil, TextPCdFile* hitfil)
{
    AjPStr tmpname  = NULL;
    AjPStr fullname = NULL;

    ajFmtPrintS(&tmpname, "%s.trg",name);
    *trgfil = textCdFileOpen(dir, ajStrGetPtr(tmpname), &fullname);
    ajStrDel(&tmpname);

    if(!*trgfil)
	return ajFalse;

    ajFmtPrintS(&tmpname, "%s.hit",name);
    *hitfil = textCdFileOpen(dir, ajStrGetPtr(tmpname), &fullname);
    ajStrDel(&tmpname);
    ajStrDel(&fullname);

    if(!*hitfil)
	return ajFalse;

    return ajTrue;
}




/* @funcstatic textObdaSecOpen ************************************************
**
** Opens an OBDA secondary index
**
** @param [r] dir [const AjPStr] Directory.
** @param [r] name [const char*] File name.
** @param [w] trgfil [TextPObdaFile*] OBDA secondary index file.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textObdaSecOpen(const AjPStr dir, const char* name,
                              TextPObdaFile* trgfil)
{
    AjPStr tmpname  = NULL;
    AjPStr fullname = NULL;

    ajFmtPrintS(&tmpname, "id_%s.index",name);
    *trgfil = textObdaFileOpen(dir, tmpname, &fullname);
    ajStrDel(&tmpname);

    ajStrDel(&fullname);

    if(!*trgfil)
	return ajFalse;

    return ajTrue;
}




/* @funcstatic textObdaSecClose ***********************************************
**
** Close an OBDA secondary index
**
** @param [w] ptrgfil [TextPObdaFile*] OBDA secondary index file
** @return [AjBool] ajTrue on success.
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool textObdaSecClose(TextPObdaFile* ptrgfil)
{
    textObdaFileClose(ptrgfil);

    return ajTrue;
}




/* @funcstatic textCdTrgClose *************************************************
**
** Close an EMBL CD-ROM target file pair.
**
** @param [w] ptrgfil [TextPCdFile*] Target file.
** @param [w] phitfil [TextPCdFile*] Hit file.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textCdTrgClose(TextPCdFile* ptrgfil, TextPCdFile* phitfil)
{
    textCdFileClose(ptrgfil);
    textCdFileClose(phitfil);

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




/* @funcstatic textAccessEntrez ***********************************************
**
** Reads text entry(s) using Entrez. Sends a query to a remote Entrez
** web server. Opens a file using the results and returns to the caller to
** read the data.
**
** @param [u] textin [AjPTextin] Text input.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textAccessEntrez(AjPTextin textin)
{
    AjPStr host      = NULL;
    AjPStr urlsearch = NULL;
    AjPStr get       = NULL;
    AjPStr proxyName = NULL;		/* host for proxy access.*/
    AjPStr proxyAuth = NULL;
    AjPStr proxyCred = NULL;
    AjPStr httpver   = NULL;		/* HTTP version for GET */
    ajint iport;
    ajint proxyPort;
    AjPStr searchdb = NULL;
    FILE *fp;
    AjPQuery qry;
    AjPStr gilist=NULL;
    AjPStr giline=NULL;
    AjPFilebuff gifilebuff=NULL;
    AjPStr tmpstr=NULL;
    ajint numgi=0;
    ajint icount=0;
    AjBool ret = AJFALSE;
    const AjPList fdlist;
    AjPQueryField fd;
    AjIList iter;
    ajuint i;
    ajuint ifield=0;
    AjOSysSocket sock;

    const char* embossfields[] = {"id",     "acc",    "sv",
                                  "des",    "org",    "key",
                                  NULL};
    const char* entrezfields[] = {"[UID]",  "[ACCN]", "[ACCN]",
                                  "[TITL]", "[ORGN]", "[KYWD]",
                                  NULL};

    /*
    ** check handling of queries
    ** should use esummary for summary list
    ** and use efetch with a list of entries (POST method) to retrieve
    **
    ** einfo returns a list of valid databases
    ** einfo with a dbname returns fields for one database
    */

    if (!textRegEntrezCount)
      textRegEntrezCount = ajRegCompC("<Count>(\\d+)</Count>");

    if (!textRegEntrezId)
	textRegEntrezId = ajRegCompC("<Id>(\\d+)</Id>");

    if (!textRegGi)
	textRegGi = ajRegCompC("(\\d+)");

    qry = textin->Query;

    if (!qry->QryData)
    {
        fdlist = ajQueryGetallFields(qry);
	textin->Single = ajTrue;

	iport     = 80;
	proxyPort = 0;			/* port for proxy access */

        if(ajStrGetLen(qry->SvrName))
        {
            if(!ajNamSvrGetdbAttrC(qry->SvrName, qry->DbName,
                                   "dbalias", &searchdb))
                ajStrAssignS(&searchdb, qry->DbName);
        }
        else if(!ajNamDbGetDbalias(qry->DbName, &searchdb))
        {
	    ajStrAssignS(&searchdb, qry->DbName);
        }

	ajDebug("textAccessEntrez %S\n",
                searchdb);

	/* eutils.ncbi.nlm.nih.gov official address
	   gives an error with valgrind */
	ajStrAssignC(&host, "eutils.ncbi.nlm.nih.gov");
	iport = 80;

	ajStrAssignC(&urlsearch, "/entrez/eutils/esearch.fcgi");

	ajHttpGetVersion(qry->DbHttpVer, &httpver);

	if(ajHttpGetProxyinfo(qry->DbProxy, &proxyPort, &proxyName,
                              &proxyAuth, &proxyCred))
	    ajFmtPrintS(&get, "GET http://%S:%d%S?",
			host, iport, urlsearch);
	else
	    ajFmtPrintS(&get, "GET %S?", urlsearch);
	ajStrDel(&urlsearch);

	ajStrAppendC(&get, "tool=emboss&email=emboss-bug@emboss.open-bio.org"
                     "&retmax=1000");

	ajFmtPrintAppS(&get, "&db=%S", searchdb);

        ifield=0;
        iter = ajListIterNewread(fdlist);

        while(!ajListIterDone(iter))
        {
            fd = ajListIterGet(iter);

            for(i=0; embossfields[i]; i++)
            {
                if(ajStrMatchC(fd->Field, embossfields[i]))
                {
                    if(ifield++)
                        ajFmtPrintAppS(&get, "|%S%s",
                                       fd->Wildquery, entrezfields[i]);
                    else
                        ajFmtPrintAppS(&get, "&term=%S%s",
                                       fd->Wildquery, entrezfields[i]);
                    break;
                }
            }

            if(!embossfields[i])
            {
                if(ifield++)
                    ajFmtPrintAppS(&get, "|%S[%S]",
                                   fd->Wildquery, fd->Field);
                else
                    ajFmtPrintAppS(&get, "&term=%S[%S]",
                                   fd->Wildquery, fd->Field);
            }
         }

        ajListIterDel(&iter);
	ajStrDel(&searchdb);

	if (!ajStrGetLen(gilist))
	{
	    /*
	     ** search to find list of GIs
	     ** use GI list to retrieve genbank entries
	     */

	    ajDebug("searching with Entrez url '%S'\n", get);

	    ajFmtPrintAppS(&get, " HTTP/%S\n", httpver);

	    ajStrAssignS(&textin->Db, qry->DbName);

	    /* finally we have set the GET command */
	    ajDebug("host '%S' port %d get '%S'\n", host, iport, get);

	    if(ajStrGetLen(proxyName))
		fp = ajHttpOpenProxy(qry->DbName, proxyName, proxyPort,
                                     proxyAuth, proxyCred,
                                     host, iport, get, &sock);
	    else
		fp = ajHttpOpen(qry->DbName, host, iport, get, &sock);

	    ajStrDel(&get);
	    ajStrDel(&host);
	    ajStrDel(&proxyName);
	    ajStrDel(&proxyAuth);
	    ajStrDel(&proxyCred);
	    ajStrDel(&httpver);

	    if(!fp)
		return ajFalse;

#ifndef WIN32
	    signal(SIGALRM, textSocketTimeout);
	    alarm(180);	    /* allow 180 seconds to read from the socket */
#endif

	    /*
	     ** read out list of GI numbers
	     ** use to build new query
	     ** return genbank format results
	     */

	    ajDebug("GI list returned\n");
	    gifilebuff = ajFilebuffNewFromCfile(fp);

	    while (ajBuffreadLineTrim(gifilebuff, &giline))
            {
		ajDebug("+%d (%d) %S\n",
                        ajStrGetLen(giline),
                        (ajint) ajStrGetCharFirst(giline),
                        giline);

		if(!ajStrGetLen(giline))
                    break;
	    }

	    numgi=0;

	    while (ajBuffreadLineTrim(gifilebuff, &giline))
            {
		ajStrTrimWhite(&giline);
		ajDebug("-%S\n", giline);

		if (!icount && ajRegExec(textRegEntrezCount,giline))
		{
		    ajRegSubI(textRegEntrezCount, 1, &tmpstr);
		    ajStrToInt(tmpstr, &icount);

		    if(!icount)
		    {
			ajStrDel(&giline);
			ajStrDel(&gilist);
			ajStrDel(&tmpstr);
			ajFilebuffDel(&gifilebuff);

			return ajFalse;
		    }
		}

		if (ajRegExec(textRegEntrezId, giline))
		{
		    ajRegSubI(textRegEntrezId, 1, &tmpstr);

		    if (numgi)
			ajStrAppendK(&gilist, '+');

		    ajStrAppendS(&gilist, tmpstr);
		    numgi++;

		}
	    }

	    ajFilebuffDel(&gifilebuff);
	    ajStrDel(&giline);
	}

	if (!ajStrGetLen(gilist))
	    return ajFalse;

	ajStrAssignS((AjPStr*) &qry->QryData, gilist);
    }

    ajDebug("textAccessEntrez ready '%S' '%S'\n",
	    gilist, (AjPStr) qry->QryData);

    if(textEntrezQryNext(qry, textin))
    {
	ret =  ajTrue;
	ajDebug("textAccessEntrez after QryNext '%S' '%S'\n",
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




/* @funcstatic textEntrezQryNext **********************************************
**
** Processes next GI in list and sets up file buffer with a genbank entry
**
** @param [u] qry [AjPQuery] Query data
** @param [u] textin [AjPTextin] Text input, including query data.
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textEntrezQryNext(AjPQuery qry, AjPTextin textin)
{
    AjPStr host      = NULL;
    AjPStr urlfetch  = NULL;
    AjPStr get       = NULL;
    AjPStr proxyName = NULL;		/* host for proxy access.*/
    AjPStr proxyAuth = NULL;
    AjPStr proxyCred = NULL;
    AjPStr httpver   = NULL;		/* HTTP version for GET */
    ajint iport;
    ajint proxyPort;
    AjPStr gilist = NULL;
    FILE *sfp;
    AjPStr gistr = NULL;
    AjPStr tmpstr=NULL;
    AjPStr textline=NULL;
    AjOSysSocket sock;

    if (!textRegGi)
	textRegGi = ajRegCompC("(\\d+)");

    iport     = 80;
    proxyPort = 0;			/* port for proxy access */

    gilist = qry->QryData;

    if (!gilist)
    {
	ajDebug("textEntrezQryNext null gilist\n");

	return ajFalse;
    }

    if(!ajRegExec(textRegGi, gilist))
    {
	ajDebug("textEntrezQryNext no match gilist '%S'\n", gilist);
	ajStrDel((AjPStr*)&qry->QryData);

	return ajFalse;
    }

    ajRegSubI(textRegGi, 1, &gistr);
    ajRegPost(textRegGi, &tmpstr);
    ajStrAssignS((AjPStr*)&qry->QryData, tmpstr);
    ajDebug("textEntrezQryNext next gi '%S'\n", gistr);

    ajStrAssignC(&urlfetch, "/entrez/eutils/efetch.fcgi");

    /* eutils.ncbi.nlm.nih.gov official address
     gives an error with valgrind */
    ajStrAssignC(&host, "www.ncbi.nlm.nih.gov");
    iport = 80;

    ajHttpGetVersion(qry->DbHttpVer, &httpver);

    if(ajHttpGetProxyinfo(qry->DbProxy, &proxyPort, &proxyName,
                          &proxyAuth, &proxyCred))
	ajFmtPrintS(&get,
		    "GET http://%S:%d%S?", host, iport, urlfetch);
    else
	ajFmtPrintS(&get, "GET %S?", urlfetch);

    ajStrDel(&urlfetch);

    ajStrAppendC(&get, "tool=emboss&email=emboss-bug@emboss.open-bio.org"
                 "&retmax=1000");

    /*
    ** Needs updating
    ** retmode=docsum returns summary information for any database
    ** retmode has a default (from Feb 2012) for all databases
    ** retmode=html obsolete, uses default retmode
    */

    if(ajStrMatchC(qry->DbType, "Nucleotide"))
	ajStrAppendC(&get, "&db=nucleotide&retmode=text&rettype=gb");
    else if(ajStrMatchC(qry->DbType, "Protein"))
	ajStrAppendC(&get, "&db=nucleotide&retmode=text&rettype=gp");
    else
        ajFmtPrintAppS(&get, "&db=%S", qry->DbName);

    ajFmtPrintAppS(&get, "&id=%S", gistr);
    ajFmtPrintAppS(&get, " HTTP/%S\n", httpver);

    ajStrDel(&gistr);
    ajStrDel(&httpver);

    if(ajStrGetLen(proxyName))
	sfp = ajHttpOpenProxy(qry->DbName, proxyName, proxyPort,
                              proxyAuth, proxyCred,
                              host, iport,
                              get, &sock);
    else
	sfp = ajHttpOpen(qry->DbName, host, iport, get, &sock);

    if(!sfp)
	return ajFalse;

    ajFilebuffDel(&textin->Filebuff);
    textin->Filebuff = ajFilebuffNewFromCfile(sfp);

    if(!textin->Filebuff)
    {
	ajDebug("socket buffer attach failed\n");
	ajErr("socket buffer attach failed for database '%S'",
	      qry->DbName);
	return ajFalse;
    }

    ajFilebuffLoadAll(textin->Filebuff);
    ajFilebuffHtmlNoheader(textin->Filebuff);

#ifndef WIN32
    alarm(0);
#endif

    ajStrAssignS(&textin->Db, qry->DbName);

    ajStrDel(&host);
    ajStrDel(&get);
    ajStrDel(&proxyName);
    ajStrDel(&proxyAuth);
    ajStrDel(&proxyCred);
    ajStrDel(&tmpstr);
    ajStrDel(&textline);

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




/* @funcstatic textAccessSeqhound *********************************************
**
** Reads text entry(s) using Seqhound. Sends a query to a remote Seqhound
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
** @param [u] textin [AjPTextin] Text input.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textAccessSeqhound(AjPTextin textin)
{
    AjPStr host      = NULL;
    AjPStr urlget    = NULL;
    AjPStr get       = NULL;
    AjPStr proxyName = NULL;		/* host for proxy access.*/
    AjPStr proxyAuth = NULL;
    AjPStr proxyCred = NULL;
    AjPStr httpver   = NULL;		/* HTTP version for GET */
    ajint iport;
    ajint proxyPort;
    AjPStr gilist = NULL;
    FILE *fp;
    AjPFilebuff gifilebuff = NULL;
    AjPStr giline = NULL;
    AjPQuery qry;
    ajint numgi = 0;
    const AjPStr cpystr = NULL;
    AjBool ret = AJFALSE;

    AjIList iter = NULL;
    AjPQueryField field = NULL;
    ajuint nfields = 0;
    AjOSysSocket sock;

    qry = textin->Query;

    if (!qry->QryData)
    {
	textin->Single = ajTrue;

	/* new query - build a GI list */
	iport     = 80;
	proxyPort = 0;			/* port for proxy access */

	if(!ajNamDbGetDbalias(qry->DbName, &qry->DbAlias))
	    ajStrAssignS(&qry->DbAlias, qry->DbName);
	ajDebug("textAccessSeqhound %S fields: %Lu\n",
                qry->DbAlias, ajListGetLength(qry->QueryFields));

	if(!ajHttpQueryUrl(qry, &iport, &host, &urlget))
	{
	    ajStrDel(&host);
	    ajStrDel(&urlget);

	    return ajFalse;
	}

	ajHttpGetVersion(qry->DbHttpVer, &httpver);

	if(ajHttpGetProxyinfo(qry->DbProxy, &proxyPort, &proxyName,
                              &proxyAuth, &proxyCred))
	    ajFmtPrintS(&get, "GET http://%S:%d%S?",
			host, iport, urlget);
	else
	    ajFmtPrintS(&get, "GET %S?", urlget);

	ajStrDel(&urlget);

        iter = ajListIterNewread(qry->QueryFields);
        while(!ajListIterDone(iter) && !nfields)
        {
            nfields++;
            field = ajListIterGet(iter);

            /* Id FindNameList&pname= */
            if(ajStrMatchC(field->Field, "id"))
                ajFmtPrintAppS(&get, "fnct=SeqHoundFindNameList&pname=%S",
                               field->Wildquery);
            /* Acc FindAccList&pnacc= */
            else if(qry->HasAcc && ajStrMatchC(field->Field, "acc"))
                ajFmtPrintAppS(&get, "fnct=SeqHoundFindAccList&pacc=%S",
                               field->Wildquery);
	/* Gi Use directly! */
            else if(ajStrMatchC(field->Field, "gi"))
                ajStrAssignS(&gilist,field->Wildquery);
            /* Sv trim to Acc */
            else if(ajStrMatchC(field->Field, "sv"))
            {
                cpystr = ajSeqtestIsSeqversion(field->Wildquery);

                if (cpystr)
                    ajFmtPrintAppS(&get,"fnct=SeqHoundFindAccList&pacc=%S",
                                   cpystr);
                else
                    ajWarn("SeqHound invalid Seqversion '%S'",
                           field->Wildquery);
            }
            /* Des not yet available - will have Lucene soon */
            else if(ajStrMatchC(field->Field, "des"))
            {
                ajWarn("SeqHound search by description not yet available\n");
                /*ajFmtPrintAppS(&get, "fnct=SeqHoundFindDesList&pdes=%S",
                                 field->Wildquery);*/
            }
            /* Tax need to find taxid [Protein|DNA]FromTaxIDList&taxid= */
            else if(ajStrMatchC(field->Field, "org"))
            {
                if(ajStrPrefixCaseC(qry->DbType, "N"))
                    ajFmtPrintAppS(&get,
                                   "SeqHoundDNAFromTaxIDList&taxid=%S",
                                   field->Wildquery);
                else
                    ajFmtPrintAppS(&get,
                                   "SeqHoundProteinFromTaxIDList&taxid=%S",
                                   field->Wildquery);
            }
            /* Key not yet available - will have Lucene soon */
            else if(ajStrMatchCaseC(field->Field, "key"))
            {
                ajWarn("SeqHound search by keyword not yet available\n");
                /*ajFmtPrintAppS(&get, "fnct=SeqHoundFindKeyList&pkey=%S",
                                 field->Wildquery);*/
            }
        }

        ajListIterDel(&iter);

        if(!nfields)/* whole database = maybe taxid=1 or 0 (root)? */
        {
            ajWarn("SeqHound search for whole database not yet available\n");
        }
        
        if (!ajStrGetLen(gilist))  /*  could be set directly for GI search */
	{
	    /*
	     ** search to find list of GIs
	     ** use GI list to retrieve genbank entries
	     */

	    ajDebug("searching with SeqHound url '%S'\n", get);

	    ajFmtPrintAppS(&get, " HTTP/%S\n", httpver);
	    ajStrDel(&httpver);

	    ajStrAssignS(&textin->Db, qry->DbName);

	    /* finally we have set the GET command */
	    ajDebug("host '%S' port %d get '%S'\n", host, iport, get);

	    if(ajStrGetLen(proxyName))
		fp = ajHttpOpenProxy(qry->DbName, proxyName, proxyPort,
                                     proxyAuth, proxyCred,
                                     host, iport, get, &sock);
	    else
		fp = ajHttpOpen(qry->DbName, host, iport, get, &sock);

	    if(!fp)
	    {
		ajStrDel(&host);
		ajStrDel(&get);
                ajStrDel(&proxyName);
                ajStrDel(&proxyAuth);
                ajStrDel(&proxyCred);
                
		return ajFalse;
	    }

#ifndef WIN32
	    signal(SIGALRM, textSocketTimeout);
	    alarm(180);	    /* allow 180 seconds to read from the socket */
#endif

	    /*
	     ** read out list of GI numbers
	     ** use to build new query
	     ** return genbank format results
	     */

	    ajDebug("GI list returned\n");
	    gifilebuff = ajFilebuffNewFromCfile(fp);

	    while (ajBuffreadLineTrim(gifilebuff, &giline))
            {
		ajDebug("+%S\n", giline);

		if(!ajStrGetLen(giline))
                    break;
	    }

	    ajBuffreadLineTrim(gifilebuff, &giline);
	    ajDebug("=%S\n", giline);
	    ajStrTrimWhite(&giline);

	    if (!ajStrMatchC(giline, "SEQHOUND_OK"))
	    {
		ajDebug("SeqHound returned code '%S'", giline);
		ajStrDel(&get);
		ajStrDel(&host);
		ajStrDel(&giline);
                ajStrDel(&proxyName);
                ajStrDel(&proxyAuth);
                ajStrDel(&proxyCred);

		ajFilebuffDel(&gifilebuff);

		return ajFalse;
	    }

	    numgi=0;

	    while (ajBuffreadLineTrim(gifilebuff, &giline))
            {
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
                        ajStrDel(&proxyName);
                        ajStrDel(&proxyAuth);
                        ajStrDel(&proxyCred);
			ajFilebuffDel(&gifilebuff);

			return ajFalse;
		    }

		ajStrAppendS(&gilist, giline);
		numgi++;
	    }

	    ajFilebuffDel(&gifilebuff);
	    ajStrDel(&giline);
	}

	if (!ajStrGetLen(gilist))
	{
	    ajStrDel(&get);
	    ajStrDel(&host);
            ajStrDel(&proxyName);
            ajStrDel(&proxyAuth);
            ajStrDel(&proxyCred);

	    return ajFalse;
	}

	ajDebug("textAccessSeqhound QryData '%S' <= '%S'\n",
	       (AjPStr) qry->QryData, gilist);
	ajStrAssignS((AjPStr*) &qry->QryData, gilist);
    }

    ajDebug("textAccessSeqhound ready '%S' '%S'\n",
	    gilist, (AjPStr) qry->QryData);

    if(textSeqhoundQryNext(qry, textin))
    {
	ret = ajTrue;
	ajDebug("textAccessSeqhound after QryNext '%S' '%S'\n",
		gilist, (AjPStr) qry->QryData);
    }

    ajStrDel(&gilist);
    ajStrDel(&host);
    ajStrDel(&urlget);
    ajStrDel(&get);
    ajStrDel(&httpver);
    ajStrDel(&proxyName);
    ajStrDel(&proxyAuth);
    ajStrDel(&proxyCred);

    return ret;
}




/* @funcstatic textSeqhoundQryNext ********************************************
**
** Processes next GI in list and sets up file buffer with a genbank entry
**
** @param [u] qry [AjPQuery] Query data
** @param [u] textin [AjPTextin] Text input, including query data.
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textSeqhoundQryNext(AjPQuery qry, AjPTextin textin)
{
    AjPStr host      = NULL;
    AjPStr urlget    = NULL;
    AjPStr get       = NULL;
    AjPStr proxyName = NULL;		/* host for proxy access.*/
    AjPStr proxyAuth = NULL;
    AjPStr proxyCred = NULL;
    AjPStr httpver   = NULL;		/* HTTP version for GET */
    ajint iport;
    ajint proxyPort;
    AjPStr gilist = NULL;
    FILE *fp;
    AjPStr giline = NULL;
    AjPStr gistr = NULL;
    AjPStr tmpstr=NULL;
    AjOSysSocket sock;

    if (!textRegGi)
	textRegGi = ajRegCompC("(\\d+)");

    iport     = 80;
    proxyPort = 0;			/* port for proxy access */

    gilist = qry->QryData;

    if (!gilist)
    {
	ajDebug("textSeqhoundQryNext null gilist\n");
	ajDebug("textSeqhoundQryNext failed: null gilist '%S' QryData '%S'\n",
		gilist, (AjPStr) qry->QryData);

	return ajFalse;
    }

    if(!ajRegExec(textRegGi, gilist))
    {
	ajDebug("textSeqhoundQryNext no match gilist '%S'\n", gilist);
	ajStrDel((AjPStr*)&qry->QryData);
	ajDebug("textSeqhoundQryNext failed: no match "
		"gilist '%S' QryData '%S'\n",
		gilist, (AjPStr) qry->QryData);

	return ajFalse;
    }

    ajRegSubI(textRegGi, 1, &gistr);
    ajRegPost(textRegGi, &tmpstr);
    ajStrAssignS((AjPStr*)&qry->QryData, tmpstr);
    ajStrDel(&tmpstr);

    ajDebug("textSeqhoundQryNext next gi '%S'\n", gistr);

    if(!ajHttpQueryUrl(qry, &iport, &host, &urlget))
    {
	ajStrDel(&gistr);
	ajStrDel(&host);
	ajStrDel(&urlget);
	ajDebug("textSeqhoundQryNext failed: ajHttpQueryUrl failed "
		"gilist '%S' QryData '%S'\n",
		gilist, (AjPStr) qry->QryData);

	return ajFalse;
    }

    ajHttpGetVersion(qry->DbHttpVer, &httpver);

    if(ajHttpGetProxyinfo(qry->DbProxy, &proxyPort, &proxyName,
                          &proxyAuth, &proxyCred))
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
	fp = ajHttpOpenProxy(qry->DbName, proxyName, proxyPort,
                             proxyAuth, proxyCred,
                             host, iport,
                             get, &sock);
    else
	fp = ajHttpOpen(qry->DbName, host, iport, get, &sock);

    if(!fp)
    {
	ajDebug("textSeqhoundQryNext failed: ajHttpOpen* failed "
		"gilist '%S' QryData '%S'",
		gilist, (AjPStr) qry->QryData);

	return ajFalse;
    }

    ajStrDel(&host);
    ajStrDel(&get);
    ajStrDel(&proxyName);
    ajStrDel(&proxyAuth);
    ajStrDel(&proxyCred);

    ajFilebuffDel(&textin->Filebuff);
    textin->Filebuff = ajFilebuffNewFromCfile(fp);

    if(!textin->Filebuff)
    {
	ajDebug("socket buffer attach failed\n");
	ajErr("socket buffer attach failed for database '%S'", qry->DbName);

	return ajFalse;
    }

    ajFilebuffLoadAll(textin->Filebuff);

#ifndef WIN32
    alarm(0);
#endif

    ajFilebuffHtmlStrip(textin->Filebuff);
    ajBuffreadLine(textin->Filebuff, &giline);
    ajStrTrimWhite(&giline);

    if (!ajStrMatchC(giline, "SEQHOUND_OK"))
    {
	ajStrDel(&giline);
	ajFilebuffReset(textin->Filebuff);
	ajDebug("textSeqhoundQryNext failed: SEQHOUND_OK not found "
		"gilist '%S' QryData '%S'\n",
		gilist, (AjPStr) qry->QryData);

	return ajFalse;
    }

    ajFilebuffClear(textin->Filebuff, 0);
    ajFilebuffTraceTitle(textin->Filebuff, "Genbank data");

    ajStrAssignS(&textin->Db, qry->DbName);

    ajStrDel(&giline);

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    ajDebug("textSeqhoundQryNext success: null gilist '%S' QryData '%S'\n",
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




/* @funcstatic textAccessSrs **************************************************
**
** Reads text entry(s) using SRS. Opens a file using the results of an SRS
** query and returns to the caller to read the data.
**
** @param [u] textin [AjPTextin] Text input.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textAccessSrs(AjPTextin textin)
{
    AjPQuery qry;

    AjIList iter = NULL;
    AjPQueryField field = NULL;
    ajuint nfields = 0;

    qry = textin->Query;

    if(!ajNamDbGetDbalias(qry->DbName, &qry->DbAlias))
	ajStrAssignS(&qry->DbAlias, qry->DbName);

    if(!ajStrGetLen(qry->Application))
	ajStrAssignC(&qry->Application, "getz");

    ajDebug("textAccessSrs %S fields: %Lu\n",
            qry->DbAlias, ajListGetLength(qry->QueryFields));

    ajFmtPrintS(&textin->Filename, "%S -e '",
                qry->Application);

    if(ajStrGetLen(qry->Organisms))
        ajFmtPrintAppS(&textin->Filename, "[%S-org:%S]&(",
                       qry->DbName, qry->Organisms);
    
    if(ajStrGetLen(qry->Namespace))
        ajFmtPrintAppS(&textin->Filename, "[%S-ns:%S]&(",
                       qry->DbName, qry->Namespace);
    
    iter = ajListIterNew(qry->QueryFields);
    while(!ajListIterDone(iter))
    {
        field = ajListIterGet(iter);

        if(nfields)
        {
            switch(field->Link)
            {
                case AJQLINK_EOR:
                    ajErr("Query link operator '^' (EOR) not supported "
                          "for access method SRS");
                    break;
                case AJQLINK_AND:
                    ajStrAppendK(&textin->Filename, '&');
                    break;
                case AJQLINK_NOT:
                    ajStrAppendK(&textin->Filename, '!');
                    break;
                case AJQLINK_OR:
                case AJQLINK_ELSE:
                default:
                    ajStrAppendK(&textin->Filename, '|');
                    break;
            }
        }

        nfields++;

        if(ajStrMatchC(field->Field,"gi"))
            ajFmtPrintS(&textin->Filename, "[%S-gid:%S]",
                    qry->DbAlias, field->Wildquery);

        else if(qry->HasAcc && ajStrMatchC(field->Field, "acc"))
            ajFmtPrintAppS(&textin->Filename, "[%S-acc:%S]",
                           qry->DbAlias, field->Wildquery);
        else
            ajFmtPrintAppS(&textin->Filename, "[%S-%S:%S]",
                           qry->DbAlias, field->Field, field->Wildquery);
    }
    ajListIterDel(&iter);

    if(!nfields)
       ajFmtPrintAppS(&textin->Filename, "%S",
                      qry->DbAlias);

    if(ajStrGetLen(qry->Namespace))
        ajStrAppendK(&textin->Filename, ')');

    if(ajStrGetLen(qry->Organisms))
        ajStrAppendK(&textin->Filename, ')');

    ajFmtPrintAppS(&textin->Filename, "'|");

    ajFilebuffDel(&textin->Filebuff);
    textin->Filebuff = ajFilebuffNewNameS(textin->Filename);

    if(!textin->Filebuff)
    {
	ajDebug("unable to open file '%S'\n", textin->Filename);

	return ajFalse;
    }

    ajStrAssignS(&textin->Db, qry->DbName);

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    return ajTrue;
}




/* @funcstatic textAccessSrsfasta *********************************************
**
** Reads text entry(s) using SRS. Opens a file using the results of an SRS
** query with FASTA format text output and returns to the caller to
** read the data.
**
** @param [u] textin [AjPTextin] Text input.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textAccessSrsfasta(AjPTextin textin)
{
    AjPQuery qry;

    AjIList iter = NULL;
    AjPQueryField field = NULL;
    ajuint nfields = 0;

    qry = textin->Query;

    if(!ajNamDbGetDbalias(qry->DbName, &qry->DbAlias))
	ajStrAssignS(&qry->DbAlias, qry->DbName);

    if(!ajStrGetLen(qry->Application))
	ajStrAssignC(&qry->Application, "getz");

    ajDebug("textAccessSrsfasta %S fields: %Lu\n",
            qry->DbAlias, ajListGetLength(qry->QueryFields));

    ajFmtPrintS(&textin->Filename, "%S -d -sf fasta '",
                qry->Application);

    iter = ajListIterNew(qry->QueryFields);
    while(!ajListIterDone(iter))
    {
        field = ajListIterGet(iter);

        if(nfields)
        {
            switch(field->Link)
            {
                case AJQLINK_EOR:
                    ajErr("Query link operator '^' (EOR) not supported "
                          "for access method SRSFASTA");
                    break;
                case AJQLINK_AND:
                    ajStrAppendK(&textin->Filename, '&');
                    break;
                case AJQLINK_NOT:
                    ajStrAppendK(&textin->Filename, '!');
                    break;
                case AJQLINK_OR:
                case AJQLINK_ELSE:
                default:
                    ajStrAppendK(&textin->Filename, '|');
                    break;
            }
        }

        nfields++;

        if(ajStrMatchC(field->Field,"gi"))
            ajFmtPrintS(&textin->Filename, "[%S-gid:%S]",
                    qry->DbAlias, field->Wildquery);

        else if(qry->HasAcc && ajStrMatchC(field->Field, "acc"))
            ajFmtPrintAppS(&textin->Filename, "[%S-acc:%S]",
                           qry->DbAlias, field->Wildquery);
        else
            ajFmtPrintAppS(&textin->Filename, "[%S-%S:%S]",
                           qry->DbAlias, field->Field, field->Wildquery);
    }
    ajListIterDel(&iter);

    if(!nfields)
       ajFmtPrintAppS(&textin->Filename, "%S",
                      qry->DbAlias);

    ajFmtPrintAppS(&textin->Filename, "'|");
    

    ajDebug("searching with SRS command '%S'\n", textin->Filename);

    ajFilebuffDel(&textin->Filebuff);
    textin->Filebuff = ajFilebuffNewNameS(textin->Filename);

    if(!textin->Filebuff)
    {
	ajDebug("unable to open file '%S'\n", textin->Filename);

	return ajFalse;
    }

    ajStrAssignS(&textin->Db, qry->DbName);

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    return ajTrue;
}




/* @funcstatic textAccessSrswww ***********************************************
**
** Reads text entry(s) using SRS. Sends a query to a remote SRS web server.
** Opens a file using the results and returns to the caller to
** read the data.
**
** @param [u] textin [AjPTextin] Text input.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textAccessSrswww(AjPTextin textin)
{
    AjPStr host      = NULL;
    AjPStr urlget    = NULL;
    AjPStr get       = NULL;
    ajint iport;
    AjPQuery qry;
    AjPStr qrycount = NULL;
    AjPStr qryfield = NULL;
    AjPStr tmpqry = NULL;
    ajuint nfields = 0;

    const char* qrycount5 = "-newId+-fun+PageEntries+-bv+1+-lv+1+-bl+1+-ll+1"
        "+-ascii";
    const char* qrycount6 = "-page+qResult+-bv+1+-lv+1+"
        "-view+%2a%20Names%20only%20%2a+-ascii";

    AjPFilebuff countfilebuff = NULL;
    AjPStr countline = NULL;
    AjPStr countstr = NULL;
    ajlong foundpos;
    ajlong istart;
    ajlong entrypos;
    const char* cp;
    ajuint chunkentries = 30;
    ajuint firstentry;
    ajuint lastentry;

    AjIList iter = NULL;
    AjPQueryField field = NULL;

    const AjPStr textfmt = NULL;
    AjBool ret = ajTrue;

    textin->ChunkEntries = ajTrue;
    qry = textin->Query;

    iport     = 80;

    ajNamQuerySetDbalias(qry);
/*    if(!ajNamDbGetDbalias(qry->DbName, &qry->DbAlias))
      ajStrAssignS(&qry->DbAlias, qry->DbName);*/

    ajDebug("textAccessSrswww %S fields: %Lu %u/%u\n",
            qry->DbAlias, ajListGetLength(qry->QueryFields),
            qry->CountEntries, qry->TotalEntries);

    if(!ajHttpQueryUrl(qry, &iport, &host, &urlget))
    {
	ajStrDel(&host);
	ajStrDel(&urlget);
	return ajFalse;
    }

    iter = ajListIterNew(qry->QueryFields);

    while(!ajListIterDone(iter))
    {
        field = ajListIterGet(iter);

        if(!ajStrGetLen(qryfield))
            ajStrAppendK(&qryfield, '+');
        else
        {
            switch(field->Link)
            {
                case AJQLINK_EOR:
                    ajErr("Query link operator '^' (EOR) not supported "
                          "for access method SRSWWW");
                    return ajFalse;
                case AJQLINK_AND:
                    ajStrAppendK(&qryfield, '&');
                    break;
                case AJQLINK_NOT:
                    ajStrAppendK(&qryfield, '!');
                    break;
                case AJQLINK_OR:
                case AJQLINK_ELSE:
                default:
                    ajStrAppendK(&qryfield, '|');
                    break;
            }
        }

        if(ajStrMatchC(field->Field,"gi"))
            ajFmtPrintS(&qryfield, "[%S-gid:%S]",
                        qry->DbAlias, field->Wildquery);

        else if(qry->HasAcc && ajStrMatchC(field->Field, "acc"))
            ajFmtPrintAppS(&qryfield, "[%S-acc:%S]",
                           qry->DbAlias, field->Wildquery);
        else
            ajFmtPrintAppS(&qryfield, "[%S-%S:%S]",
                           qry->DbAlias, field->Field, field->Wildquery);
        nfields++;
    }
    ajListIterDel(&iter);

    if(!ajStrGetLen(qryfield))
       ajFmtPrintAppS(&qryfield, "+%S",
                      qry->DbAlias);

    if(ajStrGetLen(qry->Organisms))
    {
        ajStrAssignSubS(&tmpqry, qryfield, 1, -1);
        ajFmtPrintS(&qryfield, "+[%S-org:%S]&(%S)",
                    qry->DbAlias, qry->Organisms, tmpqry);
    }

    if(ajStrGetLen(qry->Namespace))
    {
        ajStrAssignSubS(&tmpqry, qryfield, 1, -1);
        ajFmtPrintS(&qryfield, "+[%S-ns:%S]&(%S)",
                    qry->DbAlias, qry->Namespace, tmpqry);
    }

    ajDebug("searching with SRS url '%S'\n", qryfield);

    if(!qry->TotalEntries)
    {
    /*
    ** Can count the entries first with:
    **
    ** query string above for 5.x or any higher version
    ** and looking for [Ff]ound...[0-9]+...entries
    */

        if(ajStrGetCharFirst(qry->ServerVer) == '5')
            ajStrAssignC(&qrycount, qrycount5);
        else
            ajStrAssignC(&qrycount, qrycount6);

	ajFmtPrintS(&get, "%S?%S", urlget, qrycount);
	ajStrDel(&qrycount);
	ajStrAppendS(&get, qryfield);
	countfilebuff = ajHttpRead(qry->DbHttpVer, qry->DbName,
	                           qry->DbProxy,
	                           host, iport, get);
        ajStrDel(&get);

        while(countfilebuff && ajBuffreadLine(countfilebuff, &countline))
        {
            foundpos = ajStrFindC(countline, "ound");

            if(foundpos < 1)
                continue;

            ajDebug("countline:%S", countline);

            istart = foundpos + 4;
            entrypos = ajStrFindC(countline, "entries");

            if(entrypos < istart)
                continue;

            ajStrAssignSubS(&countstr, countline, istart, entrypos);
            ajDebug("countstr: '%S'\n", countstr);
            cp = ajStrGetPtr(countstr);
            qry->TotalEntries = 0;

            while (*cp)
            {
                if(isdigit((int)*cp))
                    qry->TotalEntries = 10*qry->TotalEntries +
                        (int) (*cp - '0');
                cp++;
            }
            ajDebug("nentries: %d\n", qry->TotalEntries);
            break;
        }

        ajFilebuffDel(&countfilebuff);
        ajStrDel(&countline);
        ajStrDel(&countstr);

        if(!qry->TotalEntries)
        {
            ajStrDel(&host);
            ajStrDel(&urlget);
            ajStrDel(&qryfield);
            
            ajDebug("No entries found\n");
            return ajFalse;
        }
    }
    
    /* now fetch the entries in chunks */

    if((qry->DataType == AJDATATYPE_TEXT) &&
        ajStrMatchC(ajQueryGetFormat(qry), "text"))
        chunkentries = 1;

    firstentry = qry->CountEntries+1;
    lastentry = qry->CountEntries+chunkentries;

    if(firstentry > qry->TotalEntries) /* all done */
    {
	ajStrDel(&host);
	ajStrDel(&urlget);
        ajStrDel(&qryfield);
        return ajFalse;
    }

    if(lastentry > qry->TotalEntries) /* last set of entries less than chunk */
        chunkentries = qry->TotalEntries - qry->CountEntries;

    qry->CountEntries += chunkentries;

    ajFmtPrintS(&get, "%S?-e+-ascii+-bv+%u+-lv+%u",
	        urlget, firstentry, chunkentries);
    ajStrAppendS(&get, qryfield);

    ajFilebuffDel(&textin->Filebuff);
    textin->Filebuff = ajHttpRead(qry->DbHttpVer, qry->DbName,
                                  qry->DbProxy,
                                  host, iport, get);
    ajStrDel(&urlget);
    ajStrDel(&qryfield);

    ajStrAssignS(&textin->Db, qry->DbName);

    if(!textin->Filebuff)
	ret = ajFalse;
    else if(qry->DataType == AJDATATYPE_TEXT)
    {
        textfmt = ajQueryGetFormat(qry);
        if(ajStrMatchC(textfmt, "text"))
            ajFilebuffHtmlStrip(textin->Filebuff);
        else
            ajFilebuffHtmlNoheader(textin->Filebuff);
    }
    else
    {
        ajFilebuffHtmlStrip(textin->Filebuff);
    }

    ajStrAssignS(&textin->Db, qry->DbName);

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    ajStrDel(&host);
    ajStrDel(&get);

    return ret;
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




/* @funcstatic textAccessEmboss ***********************************************
**
** Reads a text entry using EMBOSS B+tree index files.
**
** @param [u] textin [AjPTextin] Text input, including query data.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textAccessEmboss(AjPTextin textin)
{
    AjBool retval = ajFalse;

    AjPQuery qry;
    TextPEmbossQry qryd = NULL;
    AjPStr qrystr = NULL;

    qry = textin->Query;
    qryd = qry->QryData;
    ajDebug("textAccessEmboss type %d Single:%B Count:%u Multi:%B QryData:%p\n",
            qry->QueryType, textin->Single, textin->Count, textin->Multi,
            qry->QryData);

    if(!ajNamDbGetDbalias(qry->DbName, &qry->DbAlias))
	ajStrAssignS(&qry->DbAlias, qry->DbName);
    
    if(qry->QueryType == AJQUERY_ALL)
	return textEmbossAll(textin);

    if(!qry->QryData)
    {
	if(!textEmbossQryOpen(qry))
        {
            ajDebug("textEmbossQryOpen failed\n");
            return ajFalse;
        }

	qryd = qry->QryData;
	textin->Single = ajTrue;
	
	if(qry->QueryType == AJQUERY_ENTRY)
	{
	    if(!textEmbossQryEntry(qry))
	    {
                ajQueryGetQuery(qry, &qrystr);
		ajDebug("B+tree Entry failed '%S'\n",
                        qrystr);
                ajStrDel(&qrystr);
	    }

            ajDebug("B+tree Entry success list:%Lu\n",
                    ajListGetLength(qry->ResultsList));
	}

	if(qry->QueryType == AJQUERY_QUERY)
	{
	    if(!textEmbossQryQuery(qry))
	    {
                ajQueryGetQuery(qry, &qrystr);
		ajDebug("EMBOSS B+tree Query failed '%S'\n",
                        qrystr);
                ajStrDel(&qrystr);
	    }

            ajDebug("B+tree Query success\n");
	}
    }
    else
    {
        if(!textEmbossQryReuse(qry))
        {
            textEmbossQryClose(qry);
            return ajFalse;
        }
    }
   

    if(ajListGetLength(qry->ResultsList))
    {
	retval = textEmbossQryNext(qry);
        ajDebug("textEmbossQryNext returned %B list %Lu samefile %B\n",
                retval, ajListGetLength(qry->ResultsList),
                qryd->Samefile);
	if(retval)
        {
            if(qryd->Samefile)
                ajFilebuffClear(textin->Filebuff, -1);
            else
                ajFilebuffReopenFile(&textin->Filebuff, qryd->libt);
        }
    }

    if(!ajListGetLength(qry->ResultsList)) /* maybe emptied by code above */
    {
	textEmbossQryClose(qry);
    }


    ajStrAssignS(&textin->Db, qry->DbName);

    ajDebug("textAccessEmboss db: '%S' returns: %B\n", textin->Db, retval);
    return retval;
}




/* @funcstatic textAccessFreeEmboss *******************************************
**
** Frees data specific to reading EMBOSS B+tree index files.
**
** @param [r] qry [void*] query data specific to EMBLCD
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textAccessFreeEmboss(void* qry)
{
    AjPQuery query;
    TextPEmbossQry qryd;
    AjBool retval = ajTrue;

    ajDebug("textAccessFreeEmboss\n");

    query = (AjPQuery) qry;
    qryd = query->QryData;
    qryd->nentries = -1;

    textEmbossQryClose(query);

    return retval;
}




/* @funcstatic textEmbossQryReuse *********************************************
**
** Tests whether the B+tree index query data can be reused or it's finished.
**
** Clears qryData structure when finished.
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if reusable,
**                  ajFalse if finished.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textEmbossQryReuse(AjPQuery qry)
{
    TextPEmbossQry qryd;

    qryd = qry->QryData;

    if(!qry || !qryd)
	return ajFalse;


    if(!qry->ResultsList)
    {
	ajDebug("textEmbossQryReuse: query data all finished\n");
	AJFREE(qry->QryData);
	qryd = NULL;

	return ajFalse;
    }

    /* ajDebug("textEmbossQryReuse: reusing data from previous call %x\n",
      qry->QryData); */

    /*ajListTrace(qry->ResultsList);*/

    qryd->nentries = -1;


    return ajTrue;
}




/* @funcstatic textEmbossQryOpen **********************************************
**
** Open caches (etc) for B+tree search
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textEmbossQryOpen(AjPQuery qry)
{
    TextPEmbossQry qryd;
    ajint i;
    AjPStr name     = NULL;
    AjIList iter = NULL;
    AjPQueryField field = NULL;
    AjPBtcache cache = NULL;
    AjPStr qrystr = NULL;

    qry->QryData = AJNEW0(qryd);
    qryd = qry->QryData;
    qryd->div = -1;
    qryd->nentries = -1;
    
    qryd->Caches = ajListNew();

    if(!ajStrGetLen(qry->IndexDir))
    {
	ajDebug("no indexdir defined for database '%S'\n", qry->DbName);
	ajErr("no indexdir defined for database '%S'", qry->DbName);
	return ajFalse;
    }

    if(!textEmbossOpenCache(qry,"id",&qryd->idcache))
        return ajFalse;

    iter = ajListIterNewread(qry->QueryFields);
    while(!ajListIterDone(iter))
    {
        field = ajListIterGet(iter);
        ajStrFmtLower(&field->Wildquery);
        if(!textEmbossOpenCache(qry, MAJSTRGETPTR(field->Field), &cache))
            return ajFalse;
        ajListPushAppend(qryd->Caches, cache);
        cache = NULL;
    }
    ajListIterDel(&iter);


    ajQueryGetQuery(qry, &qrystr);

    ajDebug("directory '%S' query: '%S'\n",
	    qry->IndexDir, qrystr);

    ajStrDel(&qrystr);


    if(ajStrGetLen(qry->Exclude) && qryd->nentries >= 0)
    {
	AJCNEW0(qryd->Skip,qryd->nentries);
	name     = ajStrNew();
	
	for(i=0; i < qryd->nentries; ++i)
	{
	    ajStrAssignS(&name,qryd->files[i]);

	    if(!ajFilenameTestInclude(name, qry->Exclude, qry->Filename))
		qryd->Skip[i] = ajTrue;
	}

	ajStrDel(&name);
    }

    return ajTrue;
}




/* @funcstatic textEmbossOpenCache ********************************************
**
** Create primary B+tree index cache
**
** @param [u] qry [AjPQuery] Query data
** @param [r] ext [const char *] Index file extension
** @param [w] cache [AjPBtcache *] cache
** @return [AjBool] True on success
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textEmbossOpenCache(AjPQuery qry, const char *ext,
                                  AjPBtcache *cache)
{
    TextPEmbossQry qryd;
    AjPStr indexextname = NULL;

    ajuint refcount = 0;

    qryd = qry->QryData;

    indexextname = ajStrNewS(ajBtreeFieldGetExtensionC(ext));
    
    *cache = ajBtreeCacheNewReadS(qry->DbAlias,indexextname,
                                  qry->IndexDir);
    ajStrDel(&indexextname);

    if(!*cache)
    {
	qryd->nentries = -1;

	return ajFalse;
    }
    
    if(qryd->nentries == -1)
	qryd->nentries = ajBtreeReadEntriesS(qry->DbAlias,
                                             qry->IndexDir,
                                             qry->Directory,
                                             &qryd->files,
                                             NULL, &refcount);

    return ajTrue;
}





/* @funcstatic textEmbossAll **************************************************
**
** Reads the B+tree entry file and opens a list of all the
** database files for plain reading.
**
** @param [u] textin [AjPTextin] Text input.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textEmbossAll(AjPTextin textin)
{
    AjPList list;
    AjPQuery qry;
/*    AjPStrTok handle = NULL;
    AjPStr wildname = NULL;*/
    AjPStr name = NULL;
/*    AjBool del = ajFalse;*/
    
    ajint i;
    
    AjPStr *filestrings = NULL;
    ajuint refcount = 0;

    qry = textin->Query;

    if(!ajStrGetLen(qry->IndexDir))
    {
	ajDebug("no indexdir defined for database %S\n", qry->DbName);
	ajErr("no indexdir defined for database %S", qry->DbName);

	return ajFalse;
    }

    ajDebug("B+tree All index directory '%S'\n", qry->IndexDir);


    ajBtreeReadEntriesS(qry->DbAlias,qry->IndexDir,
                        qry->Directory,
                        &filestrings, NULL, &refcount);


    list = ajListstrNew();
/*    wildname = ajStrNew();*/
    name     = ajStrNew();
    
    i = 0;
    while(filestrings[i])
    {

	ajStrAssignS(&name,filestrings[i]);

	if(ajFilenameTestInclude(name, qry->Exclude, qry->Filename))
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
	    ajFilenameTrimPath(&name);
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


    ajFilebuffDel(&textin->Filebuff);
    textin->Filebuff = ajFilebuffNewListinList(list);

    ajStrAssignS(&textin->Db, qry->DbName);

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    ajStrDel(&name);
/*    ajStrDel(&wildname);*/
    AJFREE(filestrings);
    
    return ajTrue;
}




/* @funcstatic textEmbossQryEntry *********************************************
**
** Queries for a single entry in a B+tree index
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if can continue,
**                  ajFalse if all is done.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textEmbossQryEntry(AjPQuery qry)
{
    AjPBtHit newhit  = NULL;
    TextPEmbossQry qryd;
    const AjPList fdlist;
    const AjPList cachelist;
    AjIList iter;
    AjIList icache;
    AjPBtcache cache;
    AjPQueryField fd;
    AjPBtHit *allhits = NULL;
    ajuint i;

    ajDebug("textEmbossQryEntry fields: %Lu hasacc:%B\n",
	    ajListGetLength(qry->QueryFields), qry->HasAcc);

    qryd = qry->QryData;

    fdlist = ajQueryGetallFields(qry);
    cachelist = qryd->Caches;

    iter= ajListIterNewread(fdlist);
    icache = ajListIterNewread(cachelist);
    while(!ajListIterDone(iter))
    {
        fd = ajListIterGet(iter);
        cache = ajListIterGet(icache);

        ajDebug("link: %d list: %Lu\n",
                fd->Link, ajListGetLength(qry->ResultsList));
        if((fd->Link == AJQLINK_ELSE) && ajListGetLength(qry->ResultsList))
                continue;

        if(!ajBtreeCacheIsSecondary(cache))
        {
	     ajBtreeIdentFetchHit(cache,fd->Wildquery,
                                 qry->ResultsList);
        }
    }

    ajListIterDel(&iter);
    ajListIterDel(&icache);

    if(ajStrGetLen(qry->Organisms))
    {
        ajTableSetDestroy(qry->ResultsTable, NULL, &ajBtreeHitDelVoid);
        ajTableSettypeUser(qry->ResultsTable, &ajBtreeHitCmp, &ajBtreeHitHash);

        while(ajListPop(qry->ResultsList, (void**)&newhit))
            ajTablePutClean(qry->ResultsTable, newhit, newhit,
                            NULL, &ajBtreeHitDelVoid);

         textEmbossQryOrganisms(qry);

         ajTableToarrayValues(qry->ResultsTable, (void***)&allhits);

         for(i=0; allhits[i]; i++)
             ajListPushAppend(qry->ResultsList, (void*) allhits[i]);

         AJFREE(allhits);
    }

    if(ajStrGetLen(qry->Namespace))
    {
        ajTableSetDestroy(qry->ResultsTable, NULL, &ajBtreeHitDelVoid);
        ajTableSettypeUser(qry->ResultsTable, &ajBtreeHitCmp, &ajBtreeHitHash);

        while(ajListPop(qry->ResultsList, (void**)&newhit))
            ajTablePutClean(qry->ResultsTable, newhit, newhit,
                            NULL, &ajBtreeHitDelVoid);

         textEmbossQryNamespace(qry);

         ajTableToarrayValues(qry->ResultsTable, (void***)&allhits);

         for(i=0; allhits[i]; i++)
             ajListPushAppend(qry->ResultsList, (void*) allhits[i]);

         AJFREE(allhits);
    }

    if(!ajListGetLength(qry->ResultsList))
	return ajFalse;

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    return ajTrue;
}




/* @funcstatic textEmbossQryNext **********************************************
**
** Processes the next query for a B+tree index
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if successful
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textEmbossQryNext(AjPQuery qry)
{
    AjPBtHit entry;
    TextPEmbossQry qryd;
    void* item;
    AjBool ok = ajFalse;

    qryd = qry->QryData;

    ajDebug("textEmbossQryNext list %Lu skip:%B\n",
            ajListGetLength(qry->ResultsList), qryd->Skip);

    if(!ajListGetLength(qry->ResultsList))
	return ajFalse;

    /*ajListTrace(qry->ResultsList);*/


    if(!qryd->Skip)
    {
	ajListPop(qry->ResultsList, &item);
	entry = (AjPBtHit) item;
        /* ajDebug("no skip, popped item '%S' address '%x'\n",
           entry->id, entry); */
    }
    else
    {
	ok = ajFalse;

	while(!ok)
	{
	    ajListPop(qry->ResultsList, &item);
	    entry = (AjPBtHit) item;
            ajDebug("test skip, popped item dbno %u offset: %Ld skip:%B\n",
                    entry->dbno, entry->offset,
                    qryd->Skip[entry->dbno]);

	    if(!qryd->Skip[entry->dbno])
		ok = ajTrue;
	    else
	    {
		ajBtreeHitDel(&entry);

		if(!ajListGetLength(qry->ResultsList))
		    return ajFalse;
	    }
	}
    }

    qryd->Samefile = ajTrue;

    if(entry->dbno != qryd->div)
    {
	qryd->Samefile = ajFalse;
	qryd->div = entry->dbno;
	ajFileClose(&qryd->libt);
    }

    if(!qryd->libt)
    {
	qryd->libt = ajFileNewInNameS(qryd->files[entry->dbno]);

	if(!qryd->libt)
	{
            
	    ajBtreeHitDel(&entry);

	    return ajFalse;
	}
    }
	
    
    ajFileSeek(qryd->libt, (ajlong) entry->offset, 0);

    ajBtreeHitDel(&entry);

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    return ajTrue;
}




/* @funcstatic textEmbossQryClose *********************************************
**
** Closes query data for a B+tree index
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textEmbossQryClose(AjPQuery qry)
{
    TextPEmbossQry qryd;
    void* item;
    ajint i;

    if(!qry)
	return ajFalse;

    if(!qry->QryData)
	return ajFalse;

    ajDebug("textEmbossQryClose clean up qryd\n");

    qryd = qry->QryData;
    while(ajListGetLength(qryd->Caches))
    {
        ajListPop(qryd->Caches, &item);
        ajBtreeCacheDel((AjPBtcache*) &item);
    }
    ajListFree(&qryd->Caches);
    ajBtreeCacheDel(&qryd->idcache);

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

	    ++i;
	}

	AJFREE(qryd->files);
    }

    qryd->files = NULL;

    /* keep QryData for use at top of loop */

    return ajTrue;
}




/* @funcstatic textEmbossQryNamespace *****************************************
**
** Restricts results to matches to namespace attribute of database
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue on success.
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool textEmbossQryNamespace(AjPQuery qry)
{
    TextPEmbossQry qryd;
    AjPBtcache nscache;
    AjPStr nsqry = NULL;
    AjPStrTok nshandle = NULL;
    AjPTable nstable = NULL;
    AjPList nslist = NULL;
    AjPBtHit newhit;
    ajulong fdhits = 0UL;

    if(!ajStrGetLen(qry->Namespace))
        return ajTrue;

    qryd = qry->QryData;

    textEmbossOpenCache(qry, "ns", &nscache);
    nslist = ajListNew();
    nshandle = ajStrTokenNewC(qry->Namespace, " \t,;|");
    while(ajStrTokenNextParse(nshandle, &nsqry))
    {
        if(ajBtreeCacheIsSecondary(nscache))
        {
            if(!qry->Wild)
            {
                ajBtreeKeyFetchHit(nscache, qryd->idcache,
                                   nsqry, nslist);
            }
            else
            {
                ajBtreeKeyFetchwildHit(nscache,qryd->idcache,
                                       nsqry, nslist);
            }
        }
        else
        {
            ajBtreeIdentFetchwildHit(nscache,
                                     nsqry, nslist);
        }

        fdhits += ajListGetLength(nslist);
        ajDebug("Namespace list nsqry '%S' list '%Lu'", nsqry, fdhits);

    }

    nstable = ajTableNewFunctionLen(fdhits,
                                    &ajBtreeHitCmp, &ajBtreeHitHash,
                                    NULL, &ajBtreeHitDelVoid);
    while(ajListPop(nslist, (void**)&newhit))
        ajTablePutClean(nstable, newhit, newhit,
                        NULL, &ajBtreeHitDelVoid);

    ajStrTokenDel(&nshandle);

    ajTableMergeAnd(qry->ResultsTable, nstable);
    ajListFree(&nslist);
    ajBtreeCacheDel(&nscache);
    ajTableFree(&nstable);
    ajStrDel(&nsqry);

    return ajTrue;
}




/* @funcstatic textEmbossQryOrganisms *****************************************
**
** Restricts results to matches to organism(s) in database
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue on success.
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool textEmbossQryOrganisms(AjPQuery qry)
{
    TextPEmbossQry qryd;
    AjPBtcache orgcache;
    AjPStr orgqry = NULL;
    AjPStrTok orghandle = NULL;
    AjPTable orgtable = NULL;
    AjPList orglist = NULL;
    AjPBtHit newhit;
    ajulong fdhits = 0UL;

    if(!ajStrGetLen(qry->Organisms))
        return ajTrue;

    qryd = qry->QryData;

    textEmbossOpenCache(qry, "org", &orgcache);
    orglist = ajListNew();
    orghandle = ajStrTokenNewC(qry->Organisms, "\t,;|");
    while(ajStrTokenNextParse(orghandle, &orgqry))
    {
        if(ajBtreeCacheIsSecondary(orgcache))
        {
            if(!qry->Wild)
            {
                ajBtreeKeyFetchHit(orgcache,qryd->idcache,
                                   orgqry, orglist);

            }
            else
            {
               ajBtreeKeyFetchwildHit(orgcache, qryd->idcache,
                                      orgqry, orglist);
            }
        }
        else
        {
            ajBtreeIdentFetchwildHit(orgcache,
                                     orgqry, orglist);
        }

        fdhits += ajListGetLength(orglist);
        ajDebug("Organisms list orgqry '%S' list '%Lu'", orgqry, fdhits);

    }

    orgtable = ajTableNewFunctionLen(fdhits,
				     &ajBtreeHitCmp, &ajBtreeHitHash,
				     NULL, &ajBtreeHitDelVoid);
    while(ajListPop(orglist, (void**)&newhit))
        ajTablePutClean(orgtable, newhit, newhit,
                        NULL, &ajBtreeHitDelVoid);

    ajStrTokenDel(&orghandle);

    ajTableMergeAnd(qry->ResultsTable, orgtable);
    ajListFree(&orglist);
    ajBtreeCacheDel(&orgcache);
    ajTableFree(&orgtable);
    ajStrDel(&orgqry);

    return ajTrue;
}




/* @funcstatic textEmbossQryQuery *********************************************
**
** Queries for one or more entries in an EMBOSS B+tree index
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textEmbossQryQuery(AjPQuery qry)
{
    TextPEmbossQry qryd;

    const AjPList fdlist;
    const AjPList cachelist;
    AjIList iter;
    AjIList icache;
    AjPBtcache cache;
    AjPQueryField fd;
    AjPBtHit newhit;
    AjPBtHit *allhits = NULL;
    AjPTable newtable = NULL;

    ajuint i;
    ajulong lasthits = 0UL;
    ajulong fdhits = 0UL;

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    qryd = qry->QryData;

    cachelist = qryd->Caches;
    
    ajTableSetDestroy(qry->ResultsTable, NULL, &ajBtreeHitDelVoid);
    ajTableSettypeUser(qry->ResultsTable, &ajBtreeHitCmp, &ajBtreeHitHash);

    fdlist = ajQueryGetallFields(qry);

    ajDebug("textEmbossQryQuery wild: %B list:%Lu fields:%Lu\n",
            qry->Wild, ajListGetLength(qry->ResultsList),
            ajListGetLength(fdlist));

    iter = ajListIterNewread(fdlist);
    icache = ajListIterNewread(cachelist);

    while(!ajListIterDone(iter))
    {
        fd = ajListIterGet(iter);
        cache = ajListIterGet(icache);

        ajDebug("field '%S' query: '%S'\n", fd->Field, fd->Wildquery);

        if((fd->Link == AJQLINK_ELSE) && (lasthits > 0UL))
        {
            continue;
        }

        /* is this a primary or secondary key (check the cache)? */

        if(ajBtreeCacheIsSecondary(cache))
        {
            if(!qry->Wild)
            {
                ajBtreeKeyFetchHit(cache,qryd->idcache,
                                   fd->Wildquery, qry->ResultsList);

            }
            else
            {
                ajBtreeKeyFetchwildHit(cache,qryd->idcache,
                                       fd->Wildquery, qry->ResultsList);
            }
        }
        else
        {
            ajBtreeIdentFetchwildHit(cache,fd->Wildquery,qry->ResultsList);
            ajDebug("ajBtreeIdentFetchwild results:%Lu\n",
                    ajListGetLength(qry->ResultsList));
        }

        fdhits = ajListGetLength(qry->ResultsList);

        switch(fd->Link)
        {
            case AJQLINK_INIT:
                while(ajListPop(qry->ResultsList, (void**)&newhit))
                    ajTablePutClean(qry->ResultsTable, newhit, newhit,
                                    NULL, &ajBtreeHitDelVoid);
                break;

            case AJQLINK_OR:
                newtable = ajTableNewFunctionLen(fdhits,
                                                 &ajBtreeHitCmp,
                                                 &ajBtreeHitHash,
                                                 NULL, &ajBtreeHitDelVoid);
                while(ajListPop(qry->ResultsList, (void**)&newhit))
                    ajTablePutClean(newtable, newhit, newhit,
                                    NULL, &ajBtreeHitDelVoid);

                ajTableMergeOr(qry->ResultsTable, newtable);
                ajTableDel(&newtable);
                break;

            case AJQLINK_AND:
                newtable = ajTableNewFunctionLen(fdhits,
                                                 &ajBtreeHitCmp,
                                                 &ajBtreeHitHash,
                                                 NULL, &ajBtreeHitDelVoid);
                while(ajListPop(qry->ResultsList, (void**)&newhit))
                    ajTablePutClean(newtable, newhit, newhit,
                                    NULL, &ajBtreeHitDelVoid);

                ajTableMergeAnd(qry->ResultsTable, newtable);
                ajTableDel(&newtable);
                break;

            case AJQLINK_EOR:
            case AJQLINK_ELSE:
                newtable = ajTableNewFunctionLen(fdhits,
                                                 &ajBtreeHitCmp,
                                                 &ajBtreeHitHash,
                                                 NULL, &ajBtreeHitDelVoid);
                while(ajListPop(qry->ResultsList, (void**)&newhit))
                    ajTablePutClean(newtable, newhit, newhit,
                                    NULL, &ajBtreeHitDelVoid);

                ajTableMergeEor(qry->ResultsTable, newtable);
                ajTableDel(&newtable);
                break;

            case AJQLINK_NOT:
                newtable = ajTableNewFunctionLen(fdhits,
                                                 &ajBtreeHitCmp,
                                                 &ajBtreeHitHash,
                                                 NULL, &ajBtreeHitDelVoid);
                while(ajListPop(qry->ResultsList, (void**)&newhit))
                    ajTablePutClean(newtable, newhit, newhit,
                                    NULL, &ajBtreeHitDelVoid);

                ajTableMergeNot(qry->ResultsTable, newtable);
                ajTableDel(&newtable);
                break;

            default:
                ajErr("Unexpected query link operator number '%u'",
                      fd->Link);
                break;
        }

        lasthits = fdhits;
    }

    ajListIterDel(&iter);
    ajListIterDel(&icache);

    if(ajStrGetLen(qry->Organisms))
        textEmbossQryOrganisms(qry);

    if(ajStrGetLen(qry->Namespace))
        textEmbossQryNamespace(qry);

    ajTableToarrayValues(qry->ResultsTable, (void***)&allhits);
    for(i=0; allhits[i]; i++)
        ajListPushAppend(qry->ResultsList, (void*) allhits[i]);

    AJFREE(allhits);

    ajTableClear(qry->ResultsTable);

    if(ajListGetLength(qry->ResultsList))
        return ajTrue;

    return ajFalse;
}




/* @funcstatic textCdQryReuse *************************************************
**
** Tests whether Cd index query data can be reused or whether we are finished.
**
** Clears qryData structure when finished.
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textCdQryReuse(AjPQuery qry)
{
    TextPCdQry qryd;

    qryd = qry->QryData;

    if(!qry || !qryd)
	return ajFalse;


    /*    ajDebug("qryd->list  %x\n",qry->ResultsList);*/
    if(!qry->ResultsList)
    {
	ajDebug("query data all finished\n");
	AJFREE(qry->QryData);
	qryd = NULL;

	return ajFalse;
    }
    else
    {
	ajDebug("reusing data from previous call %x\n", qry->QryData);
	ajDebug("listlen  %Lu\n", ajListGetLength(qry->ResultsList));
	ajDebug("divfile '%S'\n", qryd->divfile);
	ajDebug("idxfile '%S'\n", qryd->idxfile);
	ajDebug("datfile '%S'\n", qryd->datfile);
	ajDebug("name    '%s'\n", qryd->name);
	ajDebug("nameSize %d\n",  qryd->nameSize);
	ajDebug("div      %d\n",  qryd->div);
	ajDebug("maxdiv   %d\n",  qryd->maxdiv);
	ajDebug("qry->ResultsList length %Lu\n",
                ajListGetLength(qry->ResultsList));
	/*ajListTrace(qry->ResultsList);*/
    }

    return ajTrue;
}




/* @funcstatic textCdQryOpen **************************************************
**
** Opens everything for a new CD query
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textCdQryOpen(AjPQuery qry)
{
    TextPCdQry qryd;

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

    ajDebug("directory '%S' fields: %Lu hasacc:%B\n",
	    qry->IndexDir, ajListGetLength(qry->QueryFields), qry->HasAcc);

    qry->QryData = AJNEW0(qryd);
    AJNEW0(qryd->idxLine);
    AJNEW0(qryd->trgLine);
    qryd->dfp = textCdFileOpen(qry->IndexDir, "division.lkp", &qryd->divfile);

    if(!qryd->dfp)
    {
	ajWarn("Cannot open division file in '%S' for database '%S'",
	       qry->IndexDir, qry->DbName);

	return ajFalse;
    }


    qryd->nameSize = qryd->dfp->RecSize - 2;
    qryd->maxdiv   = qryd->dfp->NRecords;
    ajDebug("nameSize: %d maxdiv: %d\n",
	    qryd->nameSize, qryd->maxdiv);
    qryd->name = ajCharNewRes(qryd->nameSize+1);
    name = ajCharNewRes(qryd->nameSize+1);
    AJCNEW0(qryd->Skip, qryd->maxdiv);
    textCdFileSeek(qryd->dfp, 0);

    for(i=0; i < qryd->maxdiv; i++)
    {
	ajReadbinInt2(qryd->dfp->File, &j);
	ajReadbinCharTrim(qryd->dfp->File, qryd->nameSize, name);

	ajStrAssignC(&fullName, name);
	ajFilenameReplacePathS(&fullName, qry->Directory);

	if(!ajFilenameTestInclude(fullName, qry->Exclude, qry->Filename))
	    qryd->Skip[i] = ajTrue;
    }

    qryd->ifp = textCdFileOpen(qry->IndexDir, "entrynam.idx", &qryd->idxfile);

    if(!qryd->ifp)
    {
	ajErr("Cannot open index file '%S' for database '%S'",
	      qryd->idxfile, qry->DbName);

	return ajFalse;
    }

    ajStrDel(&fullName);
    ajCharDel(&name);

    return ajTrue;
}




/* @funcstatic textCdQryEntry *************************************************
**
** Queries for a single entry in an EMBLCD index
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textCdQryEntry(AjPQuery qry)
{
    TextPCdEntry entry = NULL;
    ajint ipos = -1;
    ajint trghit;
    TextPCdQry qryd;
    const AjPList fdlist;
    AjIList iter;
    AjPQueryField fd;
    ajuint i;
    ajuint ii;
    ajint j;
    AjPStr qrystr = NULL;

    const char* embossfields[] = {
        "id", "acc",   "sv",    "org",   "key",     "des", "gi", NULL
    };
    const char* emblcdfields[] = {
        NULL, "acnum", "seqvn", "taxon", "keyword", "des", "gi", NULL
    };

    fdlist = ajQueryGetallFields(qry);

    ajQueryGetQuery(qry, &qrystr);

    ajDebug("entry '%S'\n",
	    qrystr);

    ajStrDel(&qrystr);

    qryd = qry->QryData;
    iter= ajListIterNewread(fdlist);

    while(!ajListIterDone(iter))
    {
        fd = ajListIterGet(iter);
        if((fd->Link == AJQLINK_ELSE) && ajListGetLength(qry->ResultsList))
                continue;

        for(i=0; embossfields[i]; i++)
        {
            if(ajStrMatchC(fd->Field, embossfields[i]))
            {
                if(!emblcdfields[i]) /* ID index */
                {
                    ipos = textCdIdxSearch(qryd->idxLine,
                                           fd->Wildquery, qryd->ifp);

                    if(ipos >= 0)
                    {
                        if(!qryd->Skip[qryd->idxLine->DivCode-1])
                        {
                            AJNEW0(entry);
                            entry->div = qryd->idxLine->DivCode;
                            entry->annoff = qryd->idxLine->AnnOffset;
                            ajListPushAppend(qry->ResultsList, (void*)entry);
                            entry = NULL;
                        }
                        else
                            ajDebug("SKIP: '%S' [file %d]\n",
                                    fd->Wildquery, qryd->idxLine->DivCode);
                    }
                }
                else            /* target/hit index */
                {
                    if(textCdTrgOpen(qry->IndexDir, emblcdfields[i],
                                     &qryd->trgfp, &qryd->hitfp))
                    {
                        trghit = textCdTrgSearch(qryd->trgLine,
                                                 fd->Wildquery, qryd->trgfp);

                        if(trghit >= 0)
                        {
                            textCdFileSeek(qryd->hitfp,
                                           qryd->trgLine->FirstHit-1);
                            ajDebug("%s First: %d Count: %d\n",
                                    emblcdfields[i],
                                    qryd->trgLine->FirstHit,
                                    qryd->trgLine->NHits);
                            ipos = qryd->trgLine->FirstHit;

                            for(ii = 0; ii < qryd->trgLine->NHits; ii++)
                            {
                                ajReadbinInt(qryd->hitfp->File, &j);
                                j--;
                                ajDebug("hitlist[%u] entry = %d\n", ii, j);
                                textCdIdxLine(qryd->idxLine, j, qryd->ifp);

                                if(!qryd->Skip[qryd->idxLine->DivCode-1])
                                {
                                    AJNEW0(entry);
                                    entry->div = qryd->idxLine->DivCode;
                                    entry->annoff = qryd->idxLine->AnnOffset;
                                    ajListPushAppend(qry->ResultsList,
                                                     (void*)entry);
                                    entry = NULL;
                                }
                                else
                                    ajDebug("SKIP: %s '%S' [file %d]\n",
                                            emblcdfields[i],
                                            fd->Wildquery,
                                            qryd->idxLine->DivCode);
                            }
                        }

                        textCdTrgClose(&qryd->trgfp, &qryd->hitfp);
                        ajStrDel(&qryd->trgLine->Target);
                    }
                }
                break;
            }
        }
        if(!embossfields[i])
        {
            if(textCdTrgOpen(qry->IndexDir, MAJSTRGETPTR(fd->Field),
                             &qryd->trgfp, &qryd->hitfp))
            {
                trghit = textCdTrgSearch(qryd->trgLine,
                                         fd->Wildquery, qryd->trgfp);

                if(trghit >= 0)
                {
                    textCdFileSeek(qryd->hitfp,
                                   qryd->trgLine->FirstHit-1);
                    ajDebug("%s First: %d Count: %d\n",
                            fd->Field, qryd->trgLine->FirstHit,
                            qryd->trgLine->NHits);
                    ipos = qryd->trgLine->FirstHit;

                    for(ii = 0; ii < qryd->trgLine->NHits; ii++)
                    {
                        ajReadbinInt(qryd->hitfp->File, &j);
                        j--;
                        ajDebug("hitlist[%u] entry = %d\n", ii, j);
                        textCdIdxLine(qryd->idxLine, j, qryd->ifp);

                        if(!qryd->Skip[qryd->idxLine->DivCode-1])
                        {
                            AJNEW0(entry);
                            entry->div = qryd->idxLine->DivCode;
                            entry->annoff = qryd->idxLine->AnnOffset;
                            ajListPushAppend(qry->ResultsList, (void*)entry);
                            entry = NULL;
                        }
                        else
                            ajDebug("SKIP: %S '%S' [file %d]\n",
                                    fd->Field,
                                    fd->Wildquery,
                                    qryd->idxLine->DivCode);
                    }
                }

                textCdTrgClose(&qryd->trgfp, &qryd->hitfp);
                ajStrDel(&qryd->trgLine->Target);
            }
        }
    }

    ajListIterDel(&iter);

    if(ipos < 0)
	return ajFalse;

    if(!ajListGetLength(qry->ResultsList))
	return ajFalse;

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    return ajTrue;
}




/* @funcstatic textCdQryQuery *************************************************
**
** Queries for one or more entries in an EMBLCD index
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textCdQryQuery(AjPQuery qry)
{
    AjIList iter = NULL;
    AjPQueryField field = NULL;
    const AjPList fdlist;

    TextPCdEntry newhit;
    TextPCdEntry *allhits = NULL;
    ajulong** keys = NULL;

    AjPTable newtable = NULL;

    ajuint i;
    ajuint lasthits = 0;
    ajuint fdhits = 0;

    ajulong *ikey = NULL;

    ajuint ishift = sizeof(ajulong)/2;

    ajDebug("textCdQryQuery\n");

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    ajTableSettypeUlong(qry->ResultsTable);
    ajTableSetDestroyboth(qry->ResultsTable);

    fdlist = ajQueryGetallFields(qry);
    iter = ajListIterNewread(fdlist);

    while(!ajListIterDone(iter))
    {
        field = ajListIterGet(iter);

        if((field->Link == AJQLINK_ELSE) && (lasthits > 0))
        {
            ajDebug("ELSE: lasthits:%u skip\n", lasthits);
            continue;
        }

        if(ajStrMatchC(field->Field, "id"))
            textCdIdxQuery(qry, field->Wildquery);
        else
            textCdTrgQuery(qry, field->Field, field->Wildquery);

        fdhits = (ajuint) ajListGetLength(qry->ResultsList);

        ajDebug("textCdQryQuery hits: %u link: %u\n",
                fdhits, field->Link);
        switch(field->Link)
        {
            case AJQLINK_INIT:
                while(ajListPop(qry->ResultsList, (void**)&newhit))
                {
                    AJNEW(ikey);
                    *ikey = (((ajulong)newhit->div) << ishift) +
                        (ajulong)newhit->annoff;
                    ajTablePutClean(qry->ResultsTable, ikey, newhit,
                                    &ajMemFree, &ajMemFree);
                    ikey = NULL;
                }

                break;

            case AJQLINK_OR:
                while(ajListPop(qry->ResultsList, (void**)&newhit))
                {
                    AJNEW(ikey);
                    *ikey = (((ajulong)newhit->div) << ishift) +
                        (ajulong)newhit->annoff;
                    ajTablePutClean(qry->ResultsTable, ikey, newhit,
                                    &ajMemFree, &ajMemFree);
                    ikey = NULL;
                }
                break;

            case AJQLINK_AND:
                newtable = ajTableulongNew(fdhits);
                ajTableSetDestroyboth(newtable);
                while(ajListPop(qry->ResultsList, (void**)&newhit))
                {
                    AJNEW(ikey);
                    *ikey = (((ajulong)newhit->div) << ishift) +
                        (ajulong)newhit->annoff;
                    ajTablePutClean(newtable, ikey, newhit,
                                    &ajMemFree, &ajMemFree);
                    ikey = NULL;
                }
                ajTableMergeAnd(qry->ResultsTable, newtable);
                ajTableDelValdel(&newtable, &ajMemFree);
                break;

            case AJQLINK_EOR:
            case AJQLINK_ELSE:
                newtable = ajTableulongNew(fdhits);
                ajTableSetDestroyboth(newtable);
                while(ajListPop(qry->ResultsList, (void**)&newhit))
                {
                    AJNEW(ikey);
                    *ikey = (((ajulong)newhit->div) << ishift) + newhit->annoff;
                    ajTablePutClean(newtable, ikey, newhit,
                                    &ajMemFree, &ajMemFree);
                    ikey = NULL;
                }
                ajTableMergeEor(qry->ResultsTable, newtable);
                ajTableDelValdel(&newtable, &ajMemFree);
                break;

            case AJQLINK_NOT:
                newtable = ajTableulongNew(fdhits);
                ajTableSetDestroyboth(newtable);
                while(ajListPop(qry->ResultsList, (void**)&newhit))
                {
                    AJNEW(ikey);
                    *ikey = (((ajulong)newhit->div) << ishift) + newhit->annoff;
                    ajTablePutClean(newtable, ikey, newhit,
                                    &ajMemFree, &ajMemFree);
                    ikey = NULL;
                }
                ajTableMergeNot(qry->ResultsTable, newtable);
                ajTableDelValdel(&newtable, &ajMemFree);
                break;

            default:
                ajErr("Unexpected query link operator number '%u'",
                      field->Link);
                break;
        }
        lasthits = fdhits;
    }

    ajListIterDel(&iter);

    ajTableToarrayKeysValues(qry->ResultsTable, (void***) &keys,
                             (void***)&allhits);
    for(i=0; allhits[i]; i++)
    {
        AJFREE(keys[i]);
        ajListPushAppend(qry->ResultsList, (void*) allhits[i]);
    }
    AJFREE(keys);

    ajDebug("ajListSortUnique len:%Lu\n",
            ajListGetLength(qry->ResultsList));
    ajListSortUnique(qry->ResultsList,
                     &textCdEntryCmp, &textCdEntryDel);

    AJFREE(allhits);

    ajDebug("textCdQryQuery clear results table\n");
    ajTableClear(qry->ResultsTable);

    if(ajListGetLength(qry->ResultsList))
        return ajTrue;

    return ajFalse;
}




/* @funcstatic textCdEntryCmp *************************************************
**
** Compares two TextPCdEntry objects
**
** @param [r] pa [const void*] TextPEntry object
** @param [r] pb [const void*] TextPEntry object
** @return [int] -1 if first entry should sort before second, +1 if the
**         second entry should sort first. 0 if they are identical
**
** @release 6.4.0
** @@
******************************************************************************/
static int textCdEntryCmp(const void* pa, const void* pb)
{
    const TextPCdEntry a;
    const TextPCdEntry b;

    a = *(TextPCdEntry const *) pa;
    b = *(TextPCdEntry const *) pb;

    ajDebug("textCdEntryCmp %x %d %d : %x %d %d\n",
	     a, a->div, a->annoff,
	     b, b->div, b->annoff);

    if(a->div != b->div)
	return (a->div - b->div);

    return (a->annoff - b->annoff);
}




/* @funcstatic textObdaEntryCmp ***********************************************
**
** Compares two TextPObdaEntry objects
**
** @param [r] pa [const void*] TextPEntry object
** @param [r] pb [const void*] TextPEntry object
** @return [int] Negative if first entry should sort before second,
**         positive if the second entry should sort first,
**         0 if they are identical
**
** @release 6.4.0
** @@
******************************************************************************/
static int textObdaEntryCmp(const void* pa, const void* pb)
{
    const TextPObdaEntry a;
    const TextPObdaEntry b;

    a = *(TextPObdaEntry const *) pa;
    b = *(TextPObdaEntry const *) pb;

    ajDebug("textObdaEntryCmp %x %d %d : %x %d %d\n",
	     a, a->div, a->annoff,
	     b, b->div, b->annoff);

    if(a->div != b->div)
	return (a->div - b->div);

    if(a->annoff > b->annoff)
        return 1;

    if(a->annoff < b->annoff)
        return -1;

    return 0;
}




/* @funcstatic textCdEntryDel *************************************************
**
** Deletes a TextPCdEntry object
**
** @param [r] pentry [void**] Address of a TextPCdEntry object
** @param [r] cl [void*] Standard unused argument, usually NULL.
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/
static void textCdEntryDel(void** pentry, void* cl)
{
    (void) cl;

    AJFREE(*pentry);

    return;
}




/* @funcstatic textObdaEntryDel ***********************************************
**
** Deletes a TextPObdaEntry object
**
** @param [r] pentry [void**] Address of a TextPObdaEntry object
** @param [r] cl [void*] Standard unused argument, usually NULL.
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/
static void textObdaEntryDel(void** pentry, void* cl)
{
    (void) cl;

    AJFREE(*pentry);

    return;
}




/* @funcstatic textCdQryNext **************************************************
**
** Processes the next query for an EMBLCD index
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if successful
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textCdQryNext(AjPQuery qry)
{
    TextPCdEntry entry;
    TextPCdQry qryd;
    void* item;

    qryd = qry->QryData;

    if(!ajListGetLength(qry->ResultsList))
	return ajFalse;

    ajDebug("qry->ResultsList (b) length %Lu\n",
            ajListGetLength(qry->ResultsList));
    /*ajListTrace(qry->ResultsList);*/
    ajListPop(qry->ResultsList, &item);
    entry = (TextPCdEntry) item;

    /*
    ajDebug("entry: %x div: %d (%d) ann: %d\n",
	    entry, entry->div, qryd->div, entry->annoff);
    */

    qryd->Samefile = ajTrue;

    if(entry->div != qryd->div)
    {
	qryd->Samefile = ajFalse;
	qryd->div = entry->div;
	/*ajDebug("div: %d\n", qryd->div);*/

	if(!textCdQryFile(qry))
	    return ajFalse;
    }

    /*
    ajDebug("Offsets(cd) %d\n", entry->annoff);
    */
    ajFileSeek(qryd->libt, entry->annoff,0);

    AJFREE(entry);

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    return ajTrue;
}




/* @funcstatic textCdQryClose *************************************************
**
** Closes query data for an EMBLCD index
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if all is done
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textCdQryClose(AjPQuery qry)
{
    TextPCdQry qryd = NULL;

    ajDebug("textCdQryClose clean up qryd\n");

    qryd = qry->QryData;

    if(!qryd)
	return ajTrue;

    ajCharDel(&qryd->name);
    ajStrDel(&qryd->divfile);
    ajStrDel(&qryd->idxfile);
    ajStrDel(&qryd->datfile);

    textCdIdxDel(&qryd->idxLine);
    textCdTrgDel(&qryd->trgLine);

    textCdFileClose(&qryd->ifp);
    textCdFileClose(&qryd->dfp);
    /* defined in a buffer, cleared there */
    /*
    ajFileClose(&qryd->libr);
    ajFileClose(&qryd->libs);
    */
    qryd->libt=0;
    AJFREE(qryd->trgLine);
    AJFREE(qryd->idxLine);
    AJFREE(qryd->Skip);

    /* keep QryData for use at top of loop */

    return ajTrue;
}




/* @funcstatic textCdQryFile **************************************************
**
** Opens a specific file number for an EMBLCD index
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textCdQryFile(AjPQuery qry)
{
    TextPCdQry qryd;
    short j;

    if(!textCdDivExp)
	textCdDivExp = ajRegCompC("^([^ ]+)( +([^ ]+))?");

    ajDebug("textCdQryFile qry %x\n",qry);
    qryd = qry->QryData;
    ajDebug("textCdQryFile qryd %x\n",qryd);
    ajDebug("textCdQryFile %F\n",qryd->dfp->File);

    textCdFileSeek(qryd->dfp, (qryd->div - 1));

    /* note - we must not use textCdFileReadName - we need spaces for GCG */

    ajReadbinInt2(qryd->dfp->File, &j);

    ajReadbinChar(qryd->dfp->File, qryd->nameSize, qryd->name);
    ajDebug("DivCode: %d, code: %2hd '%s'\n",
	    qryd->div, j, qryd->name);

    /**ajCharFmtLower(qryd->name);**/
    if(!ajRegExecC(textCdDivExp, qryd->name))
    {
	ajErr("index division file error '%S'", qryd->name);

	return ajFalse;
    }

    ajRegSubI(textCdDivExp, 1, &qryd->datfile);
    ajDebug("File '%S'\n", qryd->datfile);

    ajFileClose(&qryd->libt);
    qryd->libt = ajFileNewInNamePathS(qryd->datfile, qry->Directory);

    if(!qryd->libt)
    {
	ajErr("Cannot open file '%S' for database '%S'",
	      qryd->datfile, qry->DbName);

	return ajFalse;
    }


    return ajTrue;
}




/* @funcstatic textObdaQryFile ************************************************
**
** Opens a specific file number for an OBDA index
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue on success
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool textObdaQryFile(AjPQuery qry)
{
    TextPObdaQry qryd;

    ajDebug("textObdaQryFile qry %x\n",qry);
    qryd = qry->QryData;
    ajDebug("textObdaQryFile qryd %x\n",qryd);
    ajDebug("textObdaQryFile %F\n",qryd->dfp->File);

    ajFileClose(&qryd->libt);
    qryd->libt = ajFileNewInNameS(qryd->files[qryd->ifile]);

    if(!qryd->libt)
    {
	ajErr("Cannot open file '%S' for database '%S'",
	      qryd->files[qryd->ifile], qry->DbName);

	return ajFalse;
    }


    return ajTrue;
}




/* ==========================================================================
** EMBOSS OBDA database access
** ==========================================================================
*/




/* @section OBDA Database Indexing ********************************************
**
** These functions manage the OBDA index access methods.
**
******************************************************************************/




/* @funcstatic textAccessObda *************************************************
**
** Reads a text entry using OBDA index files.
**
** @param [u] textin [AjPTextin] Text input, including query data.
** @return [AjBool] ajTrue on success.
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool textAccessObda(AjPTextin textin)
{
    AjBool retval = ajFalse;

    AjPQuery qry;
    TextPObdaQry qryd = NULL;
    AjPStr qrystr = NULL;

    qry = textin->Query;
    qryd = qry->QryData;
    ajDebug("textAccessObda type %d Single:%B Count:%u Multi:%B QryData:%p\n",
            qry->QueryType, textin->Single, textin->Count, textin->Multi,
            qry->QryData);

    if(!ajNamDbGetDbalias(qry->DbName, &qry->DbAlias))
	ajStrAssignS(&qry->DbAlias, qry->DbName);
    
    if(qry->QueryType == AJQUERY_ALL)
	return textObdaAll(textin);

    if(!qry->QryData)
    {
	if(!textObdaQryOpen(qry))
        {
            ajDebug("textObdaQryOpen failed\n");
            return ajFalse;
        }

	qryd = qry->QryData;
	textin->Single = ajTrue;
	
	if(qry->QueryType == AJQUERY_ENTRY)
	{
	    if(!textObdaQryEntry(qry))
	    {
                ajQueryGetQuery(qry, &qrystr);
		ajDebug("OBDA Entry failed '%S'\n",
                        qrystr);
                ajStrDel(&qrystr);
	    }

            ajDebug("OBDA Entry success list:%Lu\n",
                    ajListGetLength(qry->ResultsList));
	}

	if(qry->QueryType == AJQUERY_QUERY)
	{
	    if(!textObdaQryQuery(qry))
	    {
                ajQueryGetQuery(qry, &qrystr);
		ajDebug("OBDA Query failed '%S'\n",
                        qrystr);
                ajStrDel(&qrystr);
	    }

            ajDebug("OBDA Query success\n");
	}
    }
    else
    {
        if(!textObdaQryReuse(qry))
        {
            textObdaQryClose(qry);
            return ajFalse;
        }
    }
   

    if(ajListGetLength(qry->ResultsList))
    {
	retval = textObdaQryNext(qry);
        ajDebug("textObdaQryNext returned %B list %Lu samefile %B\n",
                retval, ajListGetLength(qry->ResultsList),
                qryd->Samefile);
	if(retval)
        {
            if(qryd->Samefile)
                ajFilebuffClear(textin->Filebuff, -1);
            else
                ajFilebuffReopenFile(&textin->Filebuff, qryd->libt);
        }
    }

    if(!ajListGetLength(qry->ResultsList)) /* maybe emptied by code above */
    {
	textObdaQryClose(qry);
    }


    ajStrAssignS(&textin->Db, qry->DbName);

    ajDebug("textAccessObda db: '%S' returns: %B\n", textin->Db, retval);
    return retval;
}




/* @funcstatic textAccessFreeObda *********************************************
**
** Frees data specific to reading OBDA index files.
**
** @param [r] qry [void*] query data specific to OBDA
** @return [AjBool] ajTrue on success.
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool textAccessFreeObda(void* qry)
{
    AjPQuery query;
    TextPObdaQry qryd;
    AjBool retval = ajTrue;

    ajDebug("textAccessFreeObda\n");

    query = (AjPQuery) qry;
    qryd = query->QryData;
    qryd->nentries = -1;

    textObdaQryClose(query);

    return retval;
}




/* @funcstatic textObdaQryReuse ***********************************************
**
** Tests whether the OBDA index query data can be reused or it's finished.
**
** Clears qryData structure when finished.
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if reusable,
**                  ajFalse if finished.
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool textObdaQryReuse(AjPQuery qry)
{
    TextPObdaQry qryd;

    qryd = qry->QryData;

    if(!qry || !qryd)
	return ajFalse;


    if(!qry->ResultsList)
    {
	ajDebug("textObdaQryReuse: query data all finished\n");
	AJFREE(qry->QryData);
	qryd = NULL;

	return ajFalse;
    }

    /* ajDebug("textObdaQryReuse: reusing data from previous call %x\n",
      qry->QryData); */

    /*ajListTrace(qry->ResultsList);*/

    qryd->nentries = -1;


    return ajTrue;
}




/* @funcstatic textObdaQryOpen ************************************************
**
** Open config file and data file(s) for OBDA search
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool textObdaQryOpen(AjPQuery qry)
{
    TextPObdaQry qryd;

    ajuint i;
    AjPStr fullName = NULL;

    if(!ajStrGetLen(qry->IndexDir))
    {
	ajDebug("no indexdir defined for database '%S'\n", qry->DbName);
	ajErr("no indexdir defined for database '%S'", qry->DbName);

	return ajFalse;
    }

    ajDebug("directory '%S' fields: %Lu hasacc:%B\n",
	    qry->IndexDir, ajListGetLength(qry->QueryFields), qry->HasAcc);

    qry->QryData = AJNEW0(qryd);
    AJNEW0(qryd->idxLine);
    AJNEW0(qryd->trgLine);

    if(!textObdaConfigOpen(qry))
    {
	ajWarn("Cannot open config file in '%S' for database '%S'",
	       qry->IndexDir, qry->DbName);

	return ajFalse;
    }

    AJCNEW0(qryd->Skip, qryd->nfiles);

    for(i=0; i < qryd->nfiles; i++)
    {
	ajStrAssignS(&fullName, qryd->files[i]);
	ajFilenameReplacePathS(&fullName, qry->Directory);

	if(!ajFilenameTestInclude(fullName, qry->Exclude, qry->Filename))
	    qryd->Skip[i] = ajTrue;
    }

    qryd->ifp = textObdaFileOpen(qry->IndexDir, qryd->idxname,
                                 &fullName);

    if(!qryd->ifp)
    {
	ajErr("Cannot open OBDA index file '%S' for database '%S'",
	      qryd->idxname, qry->DbName);

	return ajFalse;
    }

    ajStrDel(&fullName);

    return ajTrue;
}




/* @funcstatic textObdaFileOpen ***********************************************
**
** Opens a named OBDA index file.
**
** @param [r] dir [const AjPStr] Directory
** @param [r] name [const AjPStr] File name.
** @param [w] fullname [AjPStr*] Full file name with directory path
** @return [TextPObdaFile] OBDA index file object.
**
** @release 6.5.0
** @@
******************************************************************************/

static TextPObdaFile textObdaFileOpen(const AjPStr dir, const AjPStr name,
                                      AjPStr* fullname)
{
    TextPObdaFile thys = NULL;
    char lenbuff[8] = "0000";
    ajulong ul;
    char* ptr;
    ajlong isize;

    AJNEW0(thys);

    thys->File = ajFileNewInNamePathS(name, dir);

    if(!thys->File)
    {
	AJFREE(thys);

	return NULL;
    }

    ajReadbinChar(thys->File, 4, lenbuff);
    ul = strtoul(lenbuff, &ptr, 10);
    thys->RecSize = ul;

    isize = ajFilenameGetSize(ajFileGetNameS(thys->File));
    thys->NRecords = (isize-4) / thys->RecSize;

    ajStrAssignS(fullname, ajFileGetPrintnameS(thys->File));

    ajDebug("textObdaFileOpen '%F' RecSize: %u NRecords: %u\n",
	    thys->File, thys->RecSize, thys->NRecords);


    return thys;
}





/* @funcstatic textObdaAll ****************************************************
**
** Reads the OBDA config file and opens a list of all the
** database files for plain reading.
**
** @param [u] textin [AjPTextin] Text input.
** @return [AjBool] ajTrue on success.
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool textObdaAll(AjPTextin textin)
{
    AjPList list;
    AjPQuery qry;
    AjPStr name = NULL;
    TextPObdaQry qryd;
    
    ajint i;
    
    qry = textin->Query;

    if(!ajStrGetLen(qry->IndexDir))
    {
	ajDebug("no indexdir defined for database %S\n", qry->DbName);
	ajErr("no indexdir defined for database %S", qry->DbName);

	return ajFalse;
    }

    ajDebug("OBDA All index directory '%S'\n", qry->IndexDir);


    if(!textObdaQryOpen(qry))
        return ajFalse;

    qryd = qry->QryData;

    list = ajListstrNew();
    name     = ajStrNew();
    
    i = 0;
    while(qryd->files[i])
    {
	ajStrAssignS(&name,qryd->files[i]);

	if(ajFilenameTestInclude(name, qry->Exclude, qry->Filename))
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

	++i;
    }


    ajFilebuffDel(&textin->Filebuff);
    textin->Filebuff = ajFilebuffNewListinList(list);

    ajStrAssignS(&textin->Db, qry->DbName);

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    ajStrDel(&name);
    
    return ajTrue;
}




/* @funcstatic textObdaConfigOpen *********************************************
**
** Reads an OBDA config file.
**
** @param [u] qry [AjPQuery] Query
** @return [AjBool] True on success
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool textObdaConfigOpen(AjPQuery qry)
{
    TextPObdaQry qryd;
    AjPStr rdline = NULL;
    const AjPStr fname = NULL;
    AjPStr idxname = NULL;
    AjPList idxlist = NULL;
    AjPList filelist = NULL;

    qryd = qry->QryData;

    if(!qryd->dfp)
        AJNEW0(qryd->dfp);

    qryd->dfp->File = ajFileNewInNamePathC("config.dat", qry->IndexDir);
    if(!qryd->dfp->File) 
    {
        ajFmtPrintS(&qryd->divfile, "%S/%S", qry->Directory, qry->DbAlias);
        qryd->dfp->File = ajFileNewInNamePathC("config.dat", qryd->divfile);
        if(!qryd->dfp->File) 
        {
            ajFatal("Failed to open OBDA config file 'config.dat' in %S or %S",
                    qry->IndexDir, qryd->divfile);
        }
    }
    else
        ajStrAssignS(&qryd->divfile, ajFileGetNameS(qryd->dfp->File));

/*
** populate obda query with:
** index type
** list of filenames
** list of fields
** format
*/
    ajReadlineTrim(qryd->dfp->File, &rdline);
    if(!ajStrMatchC(rdline, "index\tflat/1"))
        ajErr("Obda index '%F' in format '%S'", qryd->dfp, rdline);

    filelist = ajListNew();
    idxlist = ajListNew();
    while(ajReadlineTrim(qryd->dfp->File, &rdline))
    {
        fname = ajStrParseWhite(rdline);
        if(ajStrPrefixC(fname, "fileid_"))
        {
            fname = ajStrParseWhite(NULL);
            ajListstrPushAppend(filelist, ajStrNewS(fname));
        }
        else if(ajStrMatchC(fname, "primary_namespace"))
        {
            fname = ajStrParseWhite(NULL);
            ajFmtPrintS(&qryd->idxname, "key_%S.key", fname);
        }
        else if(ajStrMatchC(fname, "secondary_namespaces"))
        {
            fname = ajStrParseWhite(NULL);
            while(fname)
            {
                ajFmtPrintS(&idxname, "id_%S.index", fname);
                ajListstrPushAppend(idxlist, ajStrNewS(idxname));
                fname = ajStrParseWhite(NULL);
            }
        }
    }
    ajStrDel(&idxname);

    qryd->nfiles = (ajuint) ajListstrToarray(filelist, &qryd->files);
    ajListFree(&filelist);

    qryd->nindex = (ajuint) ajListstrToarray(idxlist, &qryd->idxfiles);
    ajListFree(&idxlist);

    ajStrDel(&rdline);

    return ajTrue;
}




/* @funcstatic textObdaQryEntry ***********************************************
**
** Queries for a single entry in an OBDA index
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if can continue,
**                  ajFalse if all is done.
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool textObdaQryEntry(AjPQuery qry)
{
    TextPObdaEntry entry = NULL;
    ajlong ipos = -1;
    ajlong trghit;
    TextPObdaQry qryd;
    const AjPList fdlist;
    AjIList iter;
    AjPQueryField fd;
    ajuint i;
    AjPStr qrystr = NULL;

    const char* embossfields[] = {
        "id", "acc", "sv",    "org",   "key",     "des", "gi", NULL
    };
    const char* obdafields[] = {
        NULL, "ACC", "seqvn", "taxon", "keyword", "des", "gi", NULL
    };

    fdlist = ajQueryGetallFields(qry);

    ajQueryGetQuery(qry, &qrystr);

    ajDebug("entry '%S'\n",
	    qrystr);

    ajStrDel(&qrystr);

    qryd = qry->QryData;
    iter= ajListIterNewread(fdlist);

    while(!ajListIterDone(iter))
    {
        fd = ajListIterGet(iter);
        if((fd->Link == AJQLINK_ELSE) && ajListGetLength(qry->ResultsList))
                continue;

        for(i=0; embossfields[i]; i++)
        {
            if(ajStrMatchC(fd->Field, embossfields[i]))
            {
                if(!obdafields[i]) /* ID index */
                {
                    ipos = textObdaIdxSearch(qryd->idxLine,
                                             fd->Wildquery, qryd->ifp);

                    if(ipos >= 0)
                    {
                        if(!qryd->Skip[qryd->idxLine->DivCode])
                        {
                            AJNEW0(entry);
                            entry->div = qryd->idxLine->DivCode;
                            entry->annoff = qryd->idxLine->AnnOffset;
                            ajListPushAppend(qry->ResultsList, (void*)entry);
                            entry = NULL;
                        }
                        else
                            ajDebug("SKIP: '%S' [file %d]\n",
                                    fd->Wildquery, qryd->idxLine->DivCode);
                    }
                }
                else            /* target/hit index */
                {
                    if(textObdaSecOpen(qry->IndexDir, obdafields[i],
                                       &qryd->trgfp))
                    {
                        trghit = textObdaSecSearch(qryd->trgLine,
                                                   fd->Wildquery, qryd->trgfp);

                        if(trghit >= 0)
                        {
                            textObdaIdxSearch(qryd->idxLine,
                                              qryd->trgLine->Target, qryd->ifp);

                            if(!qryd->Skip[qryd->idxLine->DivCode])
                            {
                                AJNEW0(entry);
                                entry->div = qryd->idxLine->DivCode;
                                entry->annoff = qryd->idxLine->AnnOffset;
                                ajListPushAppend(qry->ResultsList,
                                                 (void*)entry);
                                entry = NULL;
                            }
                            else
                                ajDebug("SKIP: %s '%S' [file %d]\n",
                                        obdafields[i],
                                        fd->Wildquery,
                                        qryd->idxLine->DivCode);
                        }

                        textObdaSecClose(&qryd->trgfp);
                        ajStrDel(&qryd->trgLine->Target);
                    }
                }
                break;
            }
        }
        if(!embossfields[i])
        {
/* repeat with native field name in upper case */
        }
    }

    ajListIterDel(&iter);

    if(ipos < 0)
	return ajFalse;

    if(!ajListGetLength(qry->ResultsList))
	return ajFalse;

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    return ajTrue;
}




/* @funcstatic textObdaQryNext ************************************************
**
** Processes the next query for an OBDA index
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if successful
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool textObdaQryNext(AjPQuery qry)
{
    TextPObdaEntry entry;
    TextPObdaQry qryd;
    void* item;

    qryd = qry->QryData;

    if(!ajListGetLength(qry->ResultsList))
	return ajFalse;

    ajDebug("qry->ResultsList (b) length %Lu\n",
            ajListGetLength(qry->ResultsList));
    /*ajListTrace(qry->ResultsList);*/
    ajListPop(qry->ResultsList, &item);
    entry = (TextPObdaEntry) item;

    /*
    ajDebug("entry: %x div: %d (%d) ann: %d\n",
	    entry, entry->div, qryd->div, entry->annoff);
    */

    qryd->Samefile = ajTrue;

    if(!qryd->libt || entry->div != qryd->ifile)
    {
	qryd->Samefile = ajFalse;
	qryd->ifile = entry->div;
	/*ajDebug("div: %d\n", qryd->div);*/

	if(!textObdaQryFile(qry))
	    return ajFalse;
    }

    /*
    ajDebug("Offsets(obda) %d\n", entry->annoff);
    */
    ajFileSeek(qryd->libt, entry->annoff,0);

    AJFREE(entry);

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    return ajTrue;
}




/* @funcstatic textObdaQryClose ***********************************************
**
** Closes query data for an OBDA index
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool textObdaQryClose(AjPQuery qry)
{
    TextPObdaQry qryd = NULL;

    ajDebug("textObdaQryClose clean up qryd\n");

    qryd = qry->QryData;

    if(!qryd)
	return ajTrue;

    ajStrDel(&qryd->divfile);

    textObdaIdxDel(&qryd->idxLine);
    textObdaSecDel(&qryd->trgLine);

    textObdaFileClose(&qryd->ifp);
    textObdaFileClose(&qryd->dfp);
    ajStrDel(&qryd->idxname);
    ajStrDelarray(&qryd->files);
    ajStrDelarray(&qryd->idxfiles);

    qryd->libt=0;
    AJFREE(qryd->trgLine);
    AJFREE(qryd->idxLine);
    AJFREE(qryd->Skip);

    /* keep QryData for use at top of loop */

    return ajTrue;
}




/* @funcstatic textObdaQryQuery *********************************************
**
** Queries for one or more entries in an OBDA index
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool textObdaQryQuery(AjPQuery qry)
{
    AjIList iter = NULL;
    AjPQueryField field = NULL;
    const AjPList fdlist;

    TextPObdaEntry newhit;
    TextPObdaEntry *allhits = NULL;
    ajulong** keys = NULL;

    AjPTable newtable = NULL;

    ajuint i;
    ajulong lasthits = 0UL;
    ajulong fdhits = 0UL;

    ajulong *ikey = NULL;

    ajuint ishift = sizeof(ajulong)/2;

    ajDebug("textObdaQryQuery\n");

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    ajTableSettypeUlong(qry->ResultsTable);
    ajTableSetDestroyboth(qry->ResultsTable);

    fdlist = ajQueryGetallFields(qry);
    iter = ajListIterNewread(fdlist);

    while(!ajListIterDone(iter))
    {
        field = ajListIterGet(iter);

        if((field->Link == AJQLINK_ELSE) && (lasthits > 0))
        {
            ajDebug("ELSE: lasthits:%Lu skip\n", lasthits);
            continue;
        }

        if(ajStrMatchC(field->Field, "id"))
            textObdaIdxQuery(qry, field->Wildquery);
        else
            textObdaSecQuery(qry, field->Field, field->Wildquery);

        fdhits = ajListGetLength(qry->ResultsList);

        ajDebug("textObdaQryQuery hits: %Lu link: %u\n",
                fdhits, field->Link);
        switch(field->Link)
        {
            case AJQLINK_INIT:
                while(ajListPop(qry->ResultsList, (void**)&newhit))
                {
                    AJNEW(ikey);
                    *ikey = (((ajulong)newhit->div) << ishift) +
                        (ajulong)newhit->annoff;
                    ajTablePutClean(qry->ResultsTable, ikey, newhit,
                                    &ajMemFree, &ajMemFree);
                    ikey = NULL;
                }

                break;

            case AJQLINK_OR:
                while(ajListPop(qry->ResultsList, (void**)&newhit))
                {
                    AJNEW(ikey);
                    *ikey = (((ajulong)newhit->div) << ishift) +
                        (ajulong)newhit->annoff;
                    ajTablePutClean(qry->ResultsTable, ikey, newhit,
                                    &ajMemFree, &ajMemFree);
                    ikey = NULL;
                }
                break;

            case AJQLINK_AND:
                newtable = ajTableulongNew(fdhits);
                ajTableSetDestroyboth(newtable);
                while(ajListPop(qry->ResultsList, (void**)&newhit))
                {
                    AJNEW(ikey);
                    *ikey = (((ajulong)newhit->div) << ishift) +
                        (ajulong)newhit->annoff;
                    ajTablePutClean(newtable, ikey, newhit,
                                    &ajMemFree, &ajMemFree);
                    ikey = NULL;
                }
                ajTableMergeAnd(qry->ResultsTable, newtable);
                ajTableDelValdel(&newtable, &ajMemFree);
                break;

            case AJQLINK_EOR:
            case AJQLINK_ELSE:
                newtable = ajTableulongNew(fdhits);
                ajTableSetDestroyboth(newtable);
                while(ajListPop(qry->ResultsList, (void**)&newhit))
                {
                    AJNEW(ikey);
                    *ikey = (((ajulong)newhit->div) << ishift) + newhit->annoff;
                    ajTablePutClean(newtable, ikey, newhit,
                                    &ajMemFree, &ajMemFree);
                    ikey = NULL;
                }
                ajTableMergeEor(qry->ResultsTable, newtable);
                ajTableDelValdel(&newtable, &ajMemFree);
                break;

            case AJQLINK_NOT:
                newtable = ajTableulongNew(fdhits);
                ajTableSetDestroyboth(newtable);
                while(ajListPop(qry->ResultsList, (void**)&newhit))
                {
                    AJNEW(ikey);
                    *ikey = (((ajulong)newhit->div) << ishift) + newhit->annoff;
                    ajTablePutClean(newtable, ikey, newhit,
                                    &ajMemFree, &ajMemFree);
                    ikey = NULL;
                }
                ajTableMergeNot(qry->ResultsTable, newtable);
                ajTableDelValdel(&newtable, &ajMemFree);
                break;

            default:
                ajErr("Unexpected query link operator number '%u'",
                      field->Link);
                break;
        }
        lasthits = fdhits;
    }

    ajListIterDel(&iter);

    ajTableToarrayKeysValues(qry->ResultsTable, (void***) &keys,
                             (void***)&allhits);
    for(i=0; allhits[i]; i++)
    {
        AJFREE(keys[i]);
        ajListPushAppend(qry->ResultsList, (void*) allhits[i]);
    }
    AJFREE(keys);

    ajDebug("ajListSortUnique len:%Lu\n",
            ajListGetLength(qry->ResultsList));
    ajListSortUnique(qry->ResultsList,
                     &textObdaEntryCmp, &textObdaEntryDel);

    AJFREE(allhits);

    ajDebug("textObdaQryQuery clear results table\n");
    ajTableClear(qry->ResultsTable);

    if(ajListGetLength(qry->ResultsList))
        return ajTrue;

    return ajFalse;
}




/* @section Remote URL Database Access ****************************************
**
** These functions manage the remote URL database access methods.
**
******************************************************************************/




/* @funcstatic textAccessDbfetch **********************************************
**
** Reads text entry(s) using EBI's dbfetch REST services.
**
** Dbfetch is accessed as a simple URL.
**
** @param [u] textin [AjPTextin] Text input.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textAccessDbfetch(AjPTextin textin)
{
    AjPStr host      = NULL;
    AjPStr urlget    = NULL;
    AjPStr get       = NULL;
    AjPQuery qry     = NULL;
    AjPStr searchdb  = NULL;
    AjPStr qryid     = NULL;
    AjPStr firstline = NULL;
    AjIList iter     = NULL;
    AjPQueryField field = NULL;
    AjBool ret  = ajTrue;
    ajint iport = 80;

    qry = textin->Query;

    if(qry->SetServer)
    {
	ajNamQuerySetDbalias(qry);
	ajStrAssignS(&searchdb, qry->DbAlias);
    }
    else if(!ajNamDbGetDbalias(qry->DbName, &searchdb))
	ajStrAssignS(&searchdb, qry->DbName);

    ajDebug("textAccessDbfetch %S fields: %Lu\n",
            qry->DbAlias,
            ajListGetLength(qry->QueryFields));

    if(!ajHttpQueryUrl(qry, &iport, &host, &urlget))
    {
	ajStrDel(&host);
	ajStrDel(&urlget);

	return ajFalse;
    }

    if(!ajListGetLength(qry->QueryFields))
        return ajFalse;

    iter = ajListIterNewread(qry->QueryFields);
    while(!ajListIterDone(iter))
    {
        field = ajListIterGet(iter);
        if(ajStrMatchC(field->Field, "id"))
        {
            if(ajStrGetLen(qryid))
		ajStrAppendC(&qryid, ",");
	    ajStrAppendS(&qryid, field->Wildquery);
	}
	else
	    ajWarn("Unexpected field name '%S' in dbfetch query",
	           field->Field);
    }
    ajListIterDel(&iter);

    ajFmtPrintS(&get, "%S?db=%S&id=%S&style=raw",
		    urlget, searchdb, qryid);

    if(ajStrGetLen(qry->Formatstr)  && !ajStrMatchC(qry->Formatstr, "unknown"))
	ajFmtPrintAppS(&get,"&format=%S", qry->Formatstr);

    ajFilebuffDel(&textin->Filebuff);
    textin->Filebuff = ajHttpRead(qry->DbHttpVer, qry->DbName,
                                  qry->DbProxy,
                                  host, iport, get);

    if(textin->Filebuff)
    {
	ajFilebuffLoadAll(textin->Filebuff);
	ajFilebuffHtmlNoheader(textin->Filebuff);
	ajBuffreadLine(textin->Filebuff, &firstline);

	if (!textRegDbfetchErr)
	    textRegDbfetchErr = ajRegCompC("ERROR (\\d+) .*\\.");

	if (ajRegExec(textRegDbfetchErr, firstline))
	{
	    ajDebug("dbfetch call returned error: %S\n", firstline);
	    ajStrDel(&firstline);
	    ret = ajFalse;
	}

	ajFilebuffReset(textin->Filebuff);
    }
    else
	ret = ajFalse;

    ajStrAssignS(&textin->Db, qry->DbName);

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    ajStrDel(&firstline);
    ajStrDel(&host);
    ajStrDel(&urlget);
    ajStrDel(&get);
    ajStrDel(&searchdb);
    ajStrDel(&qryid);

    return ret;
}




/* @funcstatic textAccessWsdbfetch ********************************************
**
** Reads entry(s) using WSDBfetch webservices. Reads the results into a buffer
** and strips out HTML before returning to the caller.
**
** @param [u] textin [AjPTextin] Text input.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textAccessWsdbfetch(AjPTextin textin)
{
#ifdef HAVE_AXIS2C
    AjPQuery qry    = NULL;
    AjPStr data     = NULL;
    AjPStr* lines   = NULL;
    AjPStr qryid    = NULL;
    AjPStr url      = NULL;
    AjIList iter    = NULL;
    ajint i;
    ajint n;
    AjPQueryField field = NULL;

    if(textin->Count > 0)
	return ajFalse;

    qry = textin->Query;

    /*
    ** Get the URL associated with the EMBOSS Sequence Database definition
    */

    url = ajStrNew();

    if(!ajNamSvrGetUrl(qry->SvrName, &url))
    {
	ajDebug("seqAccessWSDbfetch: could not get a URL for "
		"WSDbfetch database '%S', using default URL", qry->DbName);

        ajStrAssignC(&url, WSDBFETCH_EP);
    }

    if(!ajListGetLength(qry->QueryFields))
            return ajFalse;

    iter = ajListIterNewread(qry->QueryFields);
    while(!ajListIterDone(iter))
    {
	field = ajListIterGet(iter);

	if(ajStrMatchC(field->Field, "id"))
	{
	    if(ajStrGetLen(qryid))
		ajStrAppendC(&qryid, ",");
	    ajStrAppendS(&qryid, field->Wildquery);
	}
	else
	    ajWarn("Unexpected field name '%S' in wsdbfetch query",
	           field->Field);
    }
    ajListIterDel(&iter);

    data = textWsdbfetchFetchData(qry->DbName, qryid, url, qry->Formatstr);
    if(!data)
    {
        ajStrDel(&url);
        ajStrDel(&qryid);

        return ajFalse;
    }

    ajFilebuffDel(&textin->Filebuff);
    textin->Filebuff = ajFilebuffNewNofile();

    n = ajStrParseSplit(data, &lines);

    for (i=0; i<n; i++)
    {
	ajFilebuffLoadS(textin->Filebuff, lines[i]);
	ajStrDel(&lines[i]);
    }
    ajFilebuffSetUnbuffered(textin->Filebuff);

    textin->Query->QryDone = ajTrue;
    ajStrDel(&data);
    AJFREE(lines);

    ajStrDel(&url);
    ajStrDel(&qryid);
    return ajTrue;
#else
    (void)textin;
    ajWarn("Cannot use access method wsdbfetch without Axis2C enabled");
    return ajFalse;
#endif
}




/* @funcstatic textAccessMrs5 *************************************************
**
** Reads sequence(s) using CMBI Nijmegen's Maarten's Retrieval System.
** This version of the interface assumes servers running MRS-5.
**
** MRS is accessed as a simple URL.
**
** @param [u] textin [AjPTextin] Text input.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textAccessMrs5(AjPTextin textin)
{
    AjPStr host      = NULL;
    AjPStr urlget    = NULL;
    AjPStr get       = NULL;
    AjPStr proxyName = NULL;		/* host for proxy access.*/
    AjPStr proxyAuth = NULL;
    AjPStr proxyCred = NULL;
    AjPStr httpver   = NULL;	      /* HTTP version 1.0, 1.1, ... */
    ajint iport;
    ajint proxyPort;
    FILE *fp;
    AjPQuery qry;
    AjPStr searchdb = NULL;
    AjOSysTimeout timo;
    const AjPList fdlist;
    ajuint i;
    ajuint iqry=0;
    AjIList iter;
    AjPQueryField fd;
    AjOSysSocket sock;

    const char* embossfields[] = {
        "id", "acc", NULL
    };
    const char* mrsfields[] = {
        "id", "ac", NULL
    };

    iport = 80;
    proxyPort = 0;			/* port for proxy access */
    qry = textin->Query;

    if(!ajNamDbGetDbalias(qry->DbName, &searchdb))
	ajStrAssignS(&searchdb, qry->DbName);

    ajDebug("textAccessMrs5 %S fields: %Lu\n",
            qry->DbAlias, ajListGetLength(qry->QueryFields));

    if(!ajHttpQueryUrl(qry, &iport, &host, &urlget))
    {
	ajStrDel(&host);
	ajStrDel(&urlget);

	return ajFalse;
    }

    ajHttpGetVersion(qry->DbHttpVer, &httpver);

    if(ajHttpGetProxyinfo(qry->DbProxy, &proxyPort, &proxyName,
                          &proxyAuth, &proxyCred))
	ajFmtPrintS(&get, "GET http://%S:%d%S",
		    host, iport, urlget);
    else
	ajFmtPrintS(&get, "GET %S",
		    urlget);

    ajFmtPrintAppS(&get,
		   "?db=%S",
		   searchdb);

    fdlist = ajQueryGetallFields(qry);

    iter = ajListIterNewread(fdlist);
    iqry = 0;
    while(!ajListIterDone(iter))
    {
        fd = ajListIterGet(iter);

        for(i=0; embossfields[i]; i++)
        {
            if(ajStrMatchC(fd->Field, embossfields[i]))
            {
                if(iqry++)
                    ajFmtPrintAppS(&get,
                                   "+or+");
                ajFmtPrintAppS(&get,
                               "&%s=%S",
                               mrsfields[i], fd->Wildquery);
            }

            break;
        }

        if(!embossfields[i])
        {
            if(iqry++)
                ajFmtPrintAppS(&get,
                               "+or+");
            ajFmtPrintAppS(&get,
                           "&%S=%S",
                           fd->Field, fd->Wildquery);
        }
    }

    ajListIterDel(&iter);

    ajFmtPrintAppS(&get,
		   "&format=text");

    ajFmtPrintAppS(&get, " HTTP/%S\n", httpver);

    /* finally we have set the GET command */
    ajDebug("host '%S' port %d get '%S'\n", host, iport, get);

    if(ajStrGetLen(proxyName))
	fp = ajHttpOpenProxy(qry->DbName, proxyName, proxyPort,
                             proxyAuth, proxyCred,
                             host, iport,
                             get, &sock);
    else
	fp = ajHttpOpen(qry->DbName, host, iport, get, &sock);

    if(!fp)
	return ajFalse;

    ajFilebuffDel(&textin->Filebuff);
    textin->Filebuff = ajFilebuffNewFromCfile(fp);

    if(!textin->Filebuff)
    {
	ajDebug("socket buffer attach failed\n");
	ajErr("socket buffer attach failed for database '%S'", qry->DbName);

	return ajFalse;
    }

    ajDebug("Ready to read errno %d msg '%s'\n",
	    errno, ajMessGetSysmessageC());

    timo.seconds = 180;
    ajSysTimeoutSet(&timo);

    ajFilebuffLoadAll(textin->Filebuff);

    ajSysTimeoutUnset(&timo);

    ajFilebuffHtmlStrip(textin->Filebuff);

    ajStrAssignS(&textin->Db, qry->DbName);

    ajStrDel(&host);
    ajStrDel(&urlget);
    ajStrDel(&get);
    ajStrDel(&proxyName);
    ajStrDel(&proxyAuth);
    ajStrDel(&proxyCred);
    ajStrDel(&httpver);

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    return ajTrue;
}




/* @funcstatic textAccessMrs4 *************************************************
**
** Reads sequence(s) using CMBI Nijmegen's Maarten's Retrieval System.
** This version of the interface assumes servers running MRS-4.
**
** MRS is accessed as a simple URL.
**
** @param [u] textin [AjPTextin] Text input.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textAccessMrs4(AjPTextin textin)
{
    AjPStr host      = NULL;
    AjPStr urlget    = NULL;
    AjPStr get       = NULL;
    AjPStr proxyName = NULL;		/* host for proxy access.*/
    AjPStr proxyAuth = NULL;
    AjPStr proxyCred = NULL;
    AjPStr httpver   = NULL;	      /* HTTP version 1.0, 1.1, ... */
    ajint iport;
    ajint proxyPort;
    FILE *fp;
    AjPQuery qry;
    AjPStr searchdb = NULL;
    AjOSysTimeout timo;
    const AjPList fdlist;
    ajuint i;
    ajuint iqry=0;
    AjIList iter;
    AjPQueryField fd;
    AjOSysSocket sock;

    const char* embossfields[] = {
        "id", "acc", NULL
    };
    const char* mrsfields[] = {
        "id", "ac", NULL
    };

    iport = 80;
    proxyPort = 0;			/* port for proxy access */
    qry = textin->Query;

    if(!ajNamDbGetDbalias(qry->DbName, &searchdb))
	ajStrAssignS(&searchdb, qry->DbName);

    ajDebug("textAccessMrs4 %S fields: %Lu\n",
            qry->DbAlias, ajListGetLength(qry->QueryFields));

    if(!ajHttpQueryUrl(qry, &iport, &host, &urlget))
    {
	ajStrDel(&host);
	ajStrDel(&urlget);

	return ajFalse;
    }

    ajHttpGetVersion(qry->DbHttpVer, &httpver);

    if(ajHttpGetProxyinfo(qry->DbProxy, &proxyPort, &proxyName,
                          &proxyAuth, &proxyCred))
	ajFmtPrintS(&get, "GET http://%S:%d%S",
		    host, iport, urlget);
    else
	ajFmtPrintS(&get, "GET %S",
		    urlget);

    ajFmtPrintAppS(&get,
		   "?db=%S",
		   searchdb);

    fdlist = ajQueryGetallFields(qry);

    iter = ajListIterNewread(fdlist);
    iqry = 0;
    while(!ajListIterDone(iter))
    {
        fd = ajListIterGet(iter);

        for(i=0; embossfields[i]; i++)
        {
            if(ajStrMatchC(fd->Field, embossfields[i]))
            {
                if(iqry++)
                    ajFmtPrintAppS(&get,
                                   "+or+");
                ajFmtPrintAppS(&get,
                               "&%s=%S",
                               mrsfields[i], fd->Wildquery);
            }

            break;
        }

        if(!embossfields[i])
        {
            if(iqry++)
                ajFmtPrintAppS(&get,
                               "+or+");
            ajFmtPrintAppS(&get,
                           "&%S=%S",
                           fd->Field, fd->Wildquery);
        }
    }

    ajListIterDel(&iter);

    ajFmtPrintAppS(&get,
		   "&format=text");

    ajFmtPrintAppS(&get, " HTTP/%S\n", httpver);

    /* finally we have set the GET command */
    ajDebug("host '%S' port %d get '%S'\n", host, iport, get);

    if(ajStrGetLen(proxyName))
	fp = ajHttpOpenProxy(qry->DbName, proxyName, proxyPort,
                             proxyAuth, proxyCred,
                             host, iport,
                             get, &sock);
    else
	fp = ajHttpOpen(qry->DbName, host, iport, get, &sock);

    if(!fp)
	return ajFalse;

    ajFilebuffDel(&textin->Filebuff);
    textin->Filebuff = ajFilebuffNewFromCfile(fp);

    if(!textin->Filebuff)
    {
	ajDebug("socket buffer attach failed\n");
	ajErr("socket buffer attach failed for database '%S'", qry->DbName);

	return ajFalse;
    }

    ajDebug("Ready to read errno %d msg '%s'\n",
	    errno, ajMessGetSysmessageC());

    timo.seconds = 180;
    ajSysTimeoutSet(&timo);

    ajFilebuffLoadAll(textin->Filebuff);

    ajSysTimeoutUnset(&timo);

    ajFilebuffHtmlStrip(textin->Filebuff);

    ajStrAssignS(&textin->Db, qry->DbName);

    ajStrDel(&host);
    ajStrDel(&urlget);
    ajStrDel(&get);
    ajStrDel(&proxyName);
    ajStrDel(&proxyAuth);
    ajStrDel(&proxyCred);
    ajStrDel(&httpver);

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    return ajTrue;
}




/* @funcstatic textAccessMrs3 *************************************************
**
** Reads sequence(s) using CMBI Nijmegen's Maarten's Retrieval System.
** This version of the interface assumes servers running MRS-3.
**
** MRS is accessed as a simple URL.
**
** @param [u] textin [AjPTextin] Text input.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textAccessMrs3(AjPTextin textin)
{
    AjPStr host      = NULL;
    AjPStr urlget    = NULL;
    AjPStr get       = NULL;
    AjPStr proxyName = NULL;		/* host for proxy access.*/
    AjPStr proxyAuth = NULL;
    AjPStr proxyCred = NULL;
    AjPStr httpver   = NULL;	      /* HTTP version 1.0, 1.1, ... */
    ajint iport;
    ajint proxyPort;
    FILE *fp;
    AjPQuery qry;
    AjPStr searchdb = NULL;
    AjOSysTimeout timo;
    AjOSysSocket sock;
    const AjPList fdlist;
    ajuint i;
    AjIList iter = NULL;
    AjPQueryField fd = NULL;
    ajuint iqry = 0;

    const char* embossfields[] = {
        "id", "acc", NULL
    };
    const char* mrsfields[] = {
        "id", "ac", NULL
    };

    iport = 80;
    proxyPort = 0;			/* port for proxy access */
    qry = textin->Query;

    if(!ajNamDbGetDbalias(qry->DbName, &searchdb))
	ajStrAssignS(&searchdb, qry->DbName);

    ajDebug("textAccessMrs3 %S fields: %Lu\n",
            qry->DbAlias, ajListGetLength(qry->QueryFields));

    if(!ajHttpQueryUrl(qry, &iport, &host, &urlget))
    {
	ajStrDel(&host);
	ajStrDel(&urlget);

	return ajFalse;
    }

    ajHttpGetVersion(qry->DbHttpVer, &httpver);

    if(ajHttpGetProxyinfo(qry->DbProxy, &proxyPort, &proxyName,
                          &proxyAuth, &proxyCred))
	ajFmtPrintS(&get, "GET http://%S:%d%S",
		    host, iport, urlget);
    else
	ajFmtPrintS(&get, "GET %S",
		    urlget);

    ajFmtPrintAppS(&get,
		   "?db=%S",
		   searchdb);

    fdlist = ajQueryGetallFields(qry);

    iter = ajListIterNewread(fdlist);
    ajFmtPrintAppS(&get, "&query=");

    iqry = 0;
    while(!ajListIterDone(iter))
    {
        fd = ajListIterGet(iter);

        for(i=0; embossfields[i]; i++)
        {
            if(ajStrMatchC(fd->Field, embossfields[i]))
            {
                iqry++;
                ajFmtPrintAppS(&get,
                               "&%s=%S",
                               mrsfields[i], fd->Wildquery);
            }

            break;
        }

        if(!embossfields[i])
        {
            if(iqry++)
                ajFmtPrintAppS(&get,
                               "+or+");
            ajFmtPrintAppS(&get,
                           "&%S=%S",
                           fd->Field, fd->Wildquery);
        }
    }


    ajFmtPrintAppS(&get, " HTTP/%S\n", httpver);

    /* finally we have set the GET command */
    ajDebug("host '%S' port %d get '%S'\n", host, iport, get);

    if(ajStrGetLen(proxyName))
	fp = ajHttpOpenProxy(qry->DbName, proxyName, proxyPort,
                             proxyAuth, proxyCred,
                             host, iport,
                             get, &sock);
    else
	fp = ajHttpOpen(qry->DbName, host, iport, get, &sock);

    if(!fp)
	return ajFalse;

    ajFilebuffDel(&textin->Filebuff);
    textin->Filebuff = ajFilebuffNewFromCfile(fp);

    if(!textin->Filebuff)
    {
	ajDebug("socket buffer attach failed\n");
	ajErr("socket buffer attach failed for database '%S'", qry->DbName);
        ajStrDel(&proxyName);
        ajStrDel(&proxyAuth);
        ajStrDel(&proxyCred);

	return ajFalse;
    }

    ajDebug("Ready to read errno %d msg '%s'\n",
	    errno, ajMessGetSysmessageC());

    timo.seconds = 180;
    ajSysTimeoutSet(&timo);
    
    ajFilebuffLoadAll(textin->Filebuff);

    ajSysTimeoutUnset(&timo);

    ajFilebuffHtmlStrip(textin->Filebuff);

    ajStrAssignS(&textin->Db, qry->DbName);

    ajStrDel(&host);
    ajStrDel(&urlget);
    ajStrDel(&get);
    ajStrDel(&proxyName);
    ajStrDel(&proxyAuth);
    ajStrDel(&proxyCred);
    ajStrDel(&httpver);

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    return ajTrue;
}




/* @funcstatic textAccessUrl **************************************************
**
** Reads sequence(s) using a remote URL. Reads the results into a buffer
** and strips out HTML before returning to the caller.
**
** @param [u] textin [AjPTextin] Text input.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textAccessUrl(AjPTextin textin)
{
    AjPStr host      = NULL;
    AjPStr urlget    = NULL;
    AjPStr get       = NULL;
    ajint iport;
    AjPQuery qry;

    AjIList iter = NULL;
    AjPQueryField field = NULL;

    iport = 80;
    qry = textin->Query;

    if(!ajListGetLength(qry->QueryFields))
        return ajFalse;

    if(ajStrMatchWildC(qry->DbUrl, "*%s*"))
    {
        if(ajListGetLength(qry->QueryFields))
        {
            iter = ajListIterNewread(qry->QueryFields);
            while(!ajListIterDone(iter))
            {
                field = ajListIterGet(iter);
                if(ajStrMatchC(field->Field, "id"))
                    ajStrExchangeCS(&qry->DbUrl, "%s", field->Wildquery);
                else if(qry->HasAcc && ajStrMatchC(field->Field, "acc"))
                    ajStrExchangeCS(&qry->DbUrl, "%s", field->Wildquery);
                else
                    ajStrExchangeCS(&qry->DbUrl, "%s", field->Wildquery);
            }
            ajListIterDel(&iter);
        }
    }

    if(!ajHttpQueryUrl(qry, &iport, &host, &urlget))
    {
	ajStrDel(&host);
	ajStrDel(&urlget);

	return ajFalse;
    }

    ajStrAssignS(&get, urlget);

    iter = ajListIterNewread(qry->QueryFields);
    while(!ajListIterDone(iter))
    {
        field = ajListIterGet(iter);
        if(ajStrMatchC(field->Field, "id"))
        {
            /* replace %s in the "GET" command  with the ID */
            ajStrExchangeCS(&get, "%s", field->Wildquery);
        }
        else
        {
            ajDebug("Query '%S' unexpected field '%S' for URL access\n",
                  textin->Qry, field->Field);
        }
    }
    ajListIterDel(&iter);

    ajFilebuffDel(&textin->Filebuff);
    textin->Filebuff = ajHttpRead(qry->DbHttpVer, qry->DbName,
	                          qry->DbProxy, host, iport, get);

    ajStrDel(&host);
    ajStrDel(&urlget);
    ajStrDel(&get);

    if(!textin->Filebuff)
	return ajFalse;

    ajFilebuffHtmlNoheader(textin->Filebuff);

    ajFilebuffHtmlStrip(textin->Filebuff);

    ajStrAssignS(&textin->Db, qry->DbName);

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    return ajTrue;
}




/* @funcstatic textSocketTimeout **********************************************
**
** Fatal error if a socket read hangs
**
** @param [r] sig [int] Signal code - always SIGALRM but required by the
**                      signal call
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void textSocketTimeout(int sig)
{
    (void) sig;

    ajDie("Socket read timeout");

    return;
}




/* @section Application Database Access ***************************************
**
** These functions manage the application database access methods.
**
******************************************************************************/




/* @funcstatic textAccessApp **************************************************
**
** Reads sequence data using an application which can accept a specification
** in the form "database:entry" such as Erik Sonnhammer's 'efetch'.
**
** @param [u] textin [AjPTextin] Text input.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textAccessApp(AjPTextin textin)
{
    static AjPStr pipename = NULL;
    AjPQuery qry;
    AjIList iter = NULL;
    AjPQueryField field = NULL;

    qry = textin->Query;

    if(!ajStrGetLen(qry->Application))
    {
	ajErr("APP access: application not defined for %S", qry->DbName);

	return ajFalse;
    }

    ajDebug("textAccessApp '%S' dbname '%S'\n", qry->Application, qry->DbName);

    if(ajStrMatchWildC(qry->Application, "*%s*"))
    {
        if(ajListGetLength(qry->QueryFields))
        {
            iter = ajListIterNewread(qry->QueryFields);
            while(!ajListIterDone(iter))
            {
                field = ajListIterGet(iter);
                if(ajStrMatchC(field->Field, "id"))
                    ajFmtPrintS(&pipename, ajStrGetPtr(qry->Application),
                                ajStrGetPtr(field->Wildquery));
                else if(qry->HasAcc && ajStrMatchC(field->Field, "acc"))
                    ajFmtPrintS(&pipename, ajStrGetPtr(qry->Application),
                                ajStrGetPtr(field->Wildquery));
                else
                    ajFmtPrintS(&pipename, ajStrGetPtr(qry->Application),
                                ajStrGetPtr(field->Wildquery));
            }
            ajListIterDel(&iter);
        }
        else
            ajFmtPrintS(&pipename, ajStrGetPtr(qry->Application),
                        "*");

	ajStrAppendC(&pipename, " |");
    }
    else
    {
        if(ajListGetLength(qry->QueryFields))
        {
            iter = ajListIterNewread(qry->QueryFields);
            while(!ajListIterDone(iter))
            {
                field = ajListIterGet(iter);
                if(ajStrMatchC(field->Field, "id"))
                    ajFmtPrintS(&pipename, "%S %S:%S|",
                                qry->Application, qry->DbName,
                                field->Wildquery);
                else if(qry->HasAcc && ajStrMatchC(field->Field, "acc"))
                    ajFmtPrintS(&pipename, "%S %S:%S|",
                                qry->Application, qry->DbName,
                                field->Wildquery);
                else
                    ajFmtPrintS(&pipename, "%S %S:%S|",
                                qry->Application, qry->DbName,
                                field->Wildquery);
            }
            ajListIterDel(&iter);
        }
        else
	    ajFmtPrintS(&pipename, "%S %S:*|",
			qry->Application, qry->DbName);
    }

    if(!ajStrGetLen(pipename))
    {
	ajErr("APP access: bad query format");

	return ajFalse;
    }


    ajFilebuffDel(&textin->Filebuff);
    textin->Filebuff = ajFilebuffNewNameS(pipename);

    if(!textin->Filebuff)
    {
	ajErr("unable to open file '%S'", pipename);
	ajStrDel(&pipename);

	return ajFalse;
    }

    ajStrAssignS(&textin->Db, qry->DbName);

    ajStrDel(&pipename);

    if(!qry->CaseId)
	qry->QryDone = ajTrue;

    return ajTrue;
}




/* @section File Direct Access ************************************************
**
** These functions manage the sequence file direct access methods.
**
******************************************************************************/




/* @funcstatic textAccessDirect ***********************************************
**
** Reads a sequence from a database which may have multiple files.
** The sequence input object holds a directory name and a (wildcard)
** file specification.
**
** Can also use exclude definitions for files in the directory.
**
** @param [u] textin [AjPTextin] Text input.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textAccessDirect(AjPTextin textin)
{
    AjPQuery qry;

    ajDebug("textAccessDirect %S\n", textin->Query->DbName);

    qry = textin->Query;

    if(!ajStrGetLen(qry->Filename))
    {
	ajErr("DIRECT access: filename not specified");

	return ajFalse;
    }

    ajDebug("Try to open %S%S\n", qry->Directory, qry->Filename);

    ajFilebuffDel(&textin->Filebuff);
    textin->Filebuff = ajFilebuffNewPathWildExclude(qry->Directory,
                                                   qry->Filename,
                                                   qry->Exclude);
    if(!textin->Filebuff)
    {
	ajDebug("DIRECT access: unable to open file '%S/%S'\n",
		qry->Directory, qry->Filename);

	return ajFalse;
    }

    ajStrAssignS(&textin->Db, qry->DbName);
    ajStrAssignS(&textin->Filename, qry->Filename);

    return ajTrue;
}




/* @funcstatic textCdTrgQuery *************************************************
**
** Binary search of an EMBL CD-ROM index file for entries matching a
** wildcard query.
**
** Where more than one query field is defined (usually acc and sv) it
** can test all and append to a single list.
**
** @param [u] qry [AjPQuery] Sequence query object.
** @param [r] field [const AjPStr] Query field
** @param [r] wildqry [const AjPStr] Query string
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textCdTrgQuery(AjPQuery qry, const AjPStr field,
                            const AjPStr wildqry)
{
    ajint ret=0;

    if(ajStrMatchC(field, "org"))
	ret += textCdTrgFind(qry, "taxon", wildqry);

    if(ajStrMatchC(field, "key"))
	ret += textCdTrgFind(qry, "keyword", wildqry);

    if(ajStrMatchC(field, "des"))
	ret += textCdTrgFind(qry, "des", wildqry);

    if(ajStrMatchC(field, "sv"))
	ret += textCdTrgFind(qry, "seqvn", wildqry);

    if(ajStrMatchC(field, "gi"))
	ret += textCdTrgFind(qry, "gi", wildqry);

    if(qry->HasAcc && ajStrMatchC(field, "acc"))
	ret += textCdTrgFind(qry, "acnum", wildqry);


    if(ret)
	return ajTrue;

    return ajFalse;
}




/* @funcstatic textObdaSecQuery ***********************************************
**
** Binary search of an OBDA secondary index file for entries matching a
** wildcard query.
**
** Where more than one query field is defined (usually acc and sv) it
** can test all and append to a single list.
**
** @param [u] qry [AjPQuery] Sequence query object.
** @param [r] field [const AjPStr] Query field
** @param [r] wildqry [const AjPStr] Query string
** @return [AjBool] ajTrue on success.
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool textObdaSecQuery(AjPQuery qry, const AjPStr field,
                               const AjPStr wildqry)
{
    ajint ret=0;

    if(ajStrMatchC(field, "org"))
	ret += textObdaSecFind(qry, "taxon", wildqry);

    if(ajStrMatchC(field, "key"))
	ret += textObdaSecFind(qry, "keyword", wildqry);

    if(ajStrMatchC(field, "des"))
	ret += textObdaSecFind(qry, "des", wildqry);

    if(ajStrMatchC(field, "sv"))
	ret += textObdaSecFind(qry, "seqvn", wildqry);

    if(ajStrMatchC(field, "gi"))
	ret += textObdaSecFind(qry, "gi", wildqry);

    if(qry->HasAcc && ajStrMatchC(field, "acc"))
	ret += textObdaSecFind(qry, "ACC", wildqry);


    if(ret)
	return ajTrue;

    return ajFalse;
}




/* @funcstatic textCdTrgFind **************************************************
**
** Binary search of an EMBL CD-ROM index file for entries matching a
** wildcard query.
**
** Where more than one query field is defined (usually acc and sv) it
** can test all and append to a single list.
**
** @param [u] qry [AjPQuery] Sequence query object.
** @param [r] indexname [const char*] Index name.
** @param [r] queryName [const AjPStr] Query string.
** @return [ajuint] Number of matches found
**
** @release 6.4.0
** @@
******************************************************************************/

static ajuint textCdTrgFind(AjPQuery qry, const char* indexname,
                            const AjPStr queryName)
{
    TextPCdQry wild;
    AjPList   l;
    TextPCdTrg trgline;
    TextPCdIdx idxline;
    TextPCdFile idxfp;
    TextPCdFile trgfp;
    TextPCdFile hitfp;
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
    ajint trglen;
    ajint start;
    ajint end;
    ajint i;
    ajint j;
    ajint k;
    ajint cmp;
    AjBool match;

    AjBool first;
    char   *name;

    TextPCdEntry entry;


    wild    = qry->QryData;
    l       = qry->ResultsList;
    trgline = wild->trgLine;
    idxline = wild->idxLine;
    idxfp   = wild->ifp;
    trgfp   = wild->trgfp;
    hitfp   = wild->hitfp;
    skip    = wild->Skip;


    if(!textCdTrgOpen(qry->IndexDir, indexname, &trgfp, &hitfp))
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

    trglen = trgfp->RecSize-8;
    if(trglen < prefixlen)
        prefixlen = trglen;

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
	    name = textCdTrgName(pos,trgfp);
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
	    textCdTrgClose(&trgfp,&hitfp);

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
	    name = textCdTrgName(pos,trgfp);
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
	name = textCdTrgName(b2,trgfp);
	ajDebug("first %d '%s'\n",b2,name);
	name = textCdTrgName(t3,trgfp);
	ajDebug("last %d '%s'\n",t3,name);
    }


    start = b2;
    end   = t3;

    for(i=start;i<(end+1);++i)
    {
	name = textCdTrgName(i,trgfp);
	match = ajCharMatchWildC(name, ajStrGetPtr(fdstr));

	ajDebug("third pass: match:%B i:%d name '%s' queryName '%S'\n",
		match, i, name, fdstr);

	if(!match)
            continue;

	textCdTrgLine(trgline, i, trgfp);
	textCdFileSeek(hitfp,trgline->FirstHit-1);
	ajDebug("Query First: %d Count: %d\n",
		trgline->FirstHit, trgline->NHits);
	pos = trgline->FirstHit;

	for(j=0;j<(ajint)trgline->NHits;++j)
	{
	    ajReadbinInt(hitfp->File, &k);
	    --k;
	    ajDebug("hitlist[%d] entry = %d\n",j,k);
	    textCdIdxLine(idxline,k,idxfp);

	    if(!skip[idxline->DivCode-1])
	    {
		AJNEW0(entry);
		entry->div = idxline->DivCode;
		entry->annoff = idxline->AnnOffset;
		ajListPushAppend(l,(void*)entry);
                entry = NULL;
	    }
	    else
		ajDebug("SKIP: token '%S' [file %d]\n",
			queryName,idxline->DivCode);
	}
    }

    textCdTrgClose(&trgfp, &hitfp);

    ajDebug("result list length: %Lu\n", ajListGetLength(l));

    ajStrDel(&trgline->Target);
    ajStrDel(&fdstr);
    ajStrDel(&fdprefix);

    return (ajuint) ajListGetLength(l);
}




/* @funcstatic textObdaSecFind ************************************************
**
** Binary search of an OBDA secondary index file for entries matching a
** wildcard query.
**
** Where more than one query field is defined (usually acc and sv) it
** can test all and append to a single list.
**
** @param [u] qry [AjPQuery] Sequence query object.
** @param [r] indexname [const char*] Index name.
** @param [r] queryName [const AjPStr] Query string.
** @return [ajuint] Number of matches found
**
** @release 6.5.0
** @@
******************************************************************************/

static ajuint textObdaSecFind(AjPQuery qry, const char* indexname,
                              const AjPStr queryName)
{
    TextPObdaQry wild;
    AjPList   l;
    TextPObdaSecidx trgline;
    TextPObdaIdx idxline;
    TextPObdaFile idxfp;
    TextPObdaFile trgfp;
    AjBool *skip;

    AjPStr fdstr    = NULL;
    AjPStr fdprefix = NULL;

    ajlong t;
    ajlong b;
    ajlong t2;
    ajlong b2;
    ajlong t3;
    ajlong pos = 0L;
    ajint prefixlen;
    ajlong start;
    ajlong end;
    ajlong i;
    ajint cmp;
    AjBool match;

    AjBool first;
    const AjPStr name;

    TextPObdaEntry entry;


    wild    = qry->QryData;
    l       = qry->ResultsList;
    trgline = wild->trgLine;
    idxline = wild->idxLine;
    idxfp   = wild->ifp;
    trgfp   = wild->trgfp;
    skip    = wild->Skip;


    if(!textObdaSecOpen(qry->IndexDir, indexname, &trgfp))
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
	    name = textObdaSecName(pos,trgfp);
	    cmp = ajStrCmpS(fdprefix,name);
	    /*	    match = ajStrMatchWildC(fdstr,name);*/
	    ajDebug(" trg testc %d '%S' '%S' %B (+/- %d)\n",
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
	    textObdaSecClose(&trgfp);

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
	    name = textObdaSecName(pos,trgfp);
	    cmp = ajStrCmpS(fdprefix,name);
	    /* match = ajStrMatchWildC(fdstr,name); */
	    ajDebug(" trg testd %d '%S' '%S' %B (+/- %d)\n",
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
	name = textObdaSecName(b2,trgfp);
	ajDebug("first %d '%S'\n",b2,name);
	name = textObdaSecName(t3,trgfp);
	ajDebug("last %d '%S'\n",t3,name);
    }


    start = b2;
    end   = t3;

    for(i=start;i<=end;++i)
    {
	name = textObdaSecName(i,trgfp);
	match = ajStrMatchWildS(name, fdstr);

	ajDebug("third pass: match:%B i:%d name '%S' queryName '%S'\n",
		match, i, name, fdstr);

	if(!match)
            continue;

	textObdaSecLine(trgline, i, trgfp);

        /* process the first hit */

        textObdaIdxSearch(idxline,
                          trgline->Target, idxfp);
        if(!skip[idxline->DivCode])
        {
            AJNEW0(entry);
            entry->div = idxline->DivCode;
            entry->annoff = idxline->AnnOffset;
            ajListPushAppend(l,(void*)entry);
            entry = NULL;
        }
        else
            ajDebug("SKIP: token '%S' [file %d]\n",
                    queryName,idxline->DivCode);
    }

    textObdaSecClose(&trgfp);

    ajDebug("result list length: %Lu\n", ajListGetLength(l));

    ajStrDel(&trgline->Target);
    ajStrDel(&fdstr);
    ajStrDel(&fdprefix);

    return (ajuint) ajListGetLength(l);
}




/* @func ajTextdbPrintAccess **************************************************
**
** Reports the internal data structures
**
** @param [u] outf [AjPFile] Output file
** @param [r] full [AjBool] Full report (usually ajFalse)
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTextdbPrintAccess(AjPFile outf, AjBool full)
{
    ajint i = 0;

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# Text access methods\n");
    ajFmtPrintF(outf, "# Name       Alias Entry Query   All Description\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "method {\n");

    for(i=0; textAccess[i].Name; i++)
	if(full || !textAccess[i].Alias)
	    ajFmtPrintF(outf, "  %-10s %5B %5B %5B %5B \"%s\"\n",
			textAccess[i].Name,  textAccess[i].Alias,
			textAccess[i].Entry, textAccess[i].Query,
			textAccess[i].All,   textAccess[i].Desc);

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @funcstatic textCdIdxDel ***************************************************
**
** Destructor for TextPCdIdx
**
** @param [d] pthys [TextPCdIdx*] Cd index object
** @return [void]
**
** @release 6.4.0
******************************************************************************/

static void textCdIdxDel(TextPCdIdx* pthys)
{
    TextPCdIdx thys = *pthys;

    if(!thys)
        return;

    ajStrDel(&thys->EntryName);
    AJFREE(*pthys);

    return;
}




/* @funcstatic textCdTrgDel ***************************************************
**
** Destructor for TextPCdTrg
**
** @param [d] pthys [TextPCdTrg*] Cd index target object
** @return [void]
**
** @release 6.4.0
******************************************************************************/

static void textCdTrgDel(TextPCdTrg* pthys)
{
    TextPCdTrg thys = *pthys;

    if(!thys)
        return;

    ajStrDel(&thys->Target);
    AJFREE(*pthys);

    return;
}




/* @funcstatic textObdaIdxDel *************************************************
**
** Destructor for TextPObdaIdx
**
** @param [d] pthys [TextPObdaIdx*] OBDA index object
** @return [void]
**
** @release 6.5.0
******************************************************************************/

static void textObdaIdxDel(TextPObdaIdx* pthys)
{
    TextPObdaIdx thys = *pthys;

    if(!thys)
        return;

    ajStrDel(&thys->EntryName);
    AJFREE(*pthys);

    return;
}




/* @funcstatic textObdaSecDel *************************************************
**
** Destructor for TextPObdaSecidx
**
** @param [d] pthys [TextPObdaSecidx*] OBDA secondary index object
** @return [void]
**
** @release 6.5.0
******************************************************************************/

static void textObdaSecDel(TextPObdaSecidx* pthys)
{
    TextPObdaSecidx thys = *pthys;

    if(!thys)
        return;

    ajStrDel(&thys->Target);
    AJFREE(*pthys);

    return;
}




/* @func ajTextdbExit *********************************************************
**
** Cleans up text entry database processing internal memory
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajTextdbExit(void)
{
    ajRegFree(&textCdDivExp);
    ajRegFree(&textRegHttpUrl);
    ajCharDel(&textCdName);
    ajRegFree(&textRegEntrezCount);
    ajRegFree(&textRegEntrezId);
    ajRegFree(&textRegDbfetchErr);
    ajRegFree(&textRegGi);
    ajStrDel(&textObdaName);
    ajStrDel(&textObdaTmpName);

    return;
}




/* @funcstatic textEBeyeGetresults ********************************************
**
** Makes EB-eye search getResults call for the given domain, query term
** and return fields.
**
** @param [r] domain [const AjPStr] Domain
** @param [r] queryterm [const AjPStr] Query term
** @param [r] retfields [const AjPStr] Return fields in DB definition
** @return [AjPFilebuff] Buffered file
**
** @release 6.4.0
******************************************************************************/

static AjPFilebuff textEBeyeGetresults(const AjPStr domain,
                                       const AjPStr queryterm,
                                       const AjPStr retfields)
{
    AjPFilebuff buff = NULL;

#ifdef HAVE_AXIS2C
    axutil_env_t* env           = NULL;
    const axis2_char_t* address = NULL;
    axis2_svc_client_t* client  = NULL;
    axiom_node_t* payload       = NULL;
    axiom_node_t* result        = NULL;
    ajint i = 0;
    ajint n = 0;

    env = ajSoapAxis2GetEnv();

    address = EBEYE_EP;

    client = ajSoapAxis2GetClient(env, address);

    payload = textEbeyeGetnumberofresultsPayload(env, domain, queryterm);
    result = ajSoapAxis2Call(client, env, payload);

    if(result != NULL)
    {
        buff = ajFilebuffNewNofile();

        n = textEbeyeGetnumberofresultsParse(result, env);

        for(; i<n; i+=100)
        {
            payload = textEbeyeGetresultsPayload(env, domain, queryterm,
                                                 retfields, i);
            result = ajSoapAxis2Call(client, env, payload);
            textEbeyeGetresultsParse(result, env, domain, retfields, buff);
        }
    }

    axis2_svc_client_free(client, env);
    axutil_env_free(env);

#else
    (void) domain;
    (void) queryterm;
    (void) retfields;
    ajWarn("Cannot use ebeye access method without Axis2C enabled");
#endif

    return buff;
}




/* @funcstatic textAccessEBeye ************************************************
**
** Search most EBI databases using EBI's EB-eye SOAP webservice.
**
** @param [u] textin [AjPTextin] Text input.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool textAccessEBeye(AjPTextin textin)
{
    AjPQuery qry        = NULL;
    AjPFilebuff results = NULL;
    AjPStr ebeyeqry     = NULL;
    AjIList iter        = NULL;
    AjPQueryField qfd   = NULL;

    if(textin->Count > 0)
	return ajFalse;

    qry = textin->Query;

    ebeyeqry = ajStrNew();

    ajNamSvrGetDbalias(qry->SvrName, &qry->DbAlias);
    ajStrAssignS(&qry->DbAlias, qry->DbName);


    iter = ajListIterNewread(qry->QueryFields);

    while(!ajListIterDone(iter))
    {
	qfd = ajListIterGet(iter);

	switch(qfd->Link)
	{
	    case AJQLINK_EOR:
		ajErr("Query link operator '^' (EOR) not supported "
			"for access method ebeye");
		break;
	    case AJQLINK_AND:
		ajStrAppendC(&ebeyeqry, " AND ");
		break;
	    case AJQLINK_NOT:
		ajStrAppendC(&ebeyeqry, " NOT ");
		break;
	    case AJQLINK_OR:
		ajStrAppendC(&ebeyeqry, " OR ");
	    default:
		break;
	}

	ajFmtPrintAppS(&ebeyeqry, "%S:%S", qfd->Field, qfd->Wildquery);

    }

    results = textEBeyeGetresults(qry->DbAlias, ebeyeqry, qry->DbReturn);

    ajStrDel(&ebeyeqry);

    if(results==NULL)
	return ajFalse;

    ajFilebuffDel(&textin->Filebuff);
    textin->Filebuff = results;
    textin->Query->QryDone = ajTrue;

    ajListIterDel(&iter);

    return ajTrue;
}


#ifdef HAVE_AXIS2C




/* @funcstatic textWsdbfetchFetchData *****************************************
**
** Reads sequence(s) using EBI's WSdbfetch webservice.
**
** @param [r] db [const AjPStr] wsdbfetch database name.
** @param [r] id [const AjPStr] entry identifier for the wsdbfetch database
** @param [r] url [const AjPStr] URL for the wsdbfetch database
** @param [r] format [const AjPStr] format name for the wsdbfetch database
** @return [AjPStr] Result string
**
** @release 6.4.0
** @@
******************************************************************************/

static AjPStr textWsdbfetchFetchData(const AjPStr db, const AjPStr id,
	                             const AjPStr url, const AjPStr format)
{
    axutil_env_t* env           = NULL;
    const axis2_char_t* address = NULL;
    const axis2_char_t* seqtext = NULL;
    axis2_svc_client_t* client  = NULL;
    axiom_node_t* payload       = NULL;
    axiom_node_t* result        = NULL;
    AjPStr ret                  = NULL;

    address = ajStrGetPtr(url);

    ajDebug("wsdbfetch URL: %s\n", address);

    env = ajSoapAxis2GetEnv();
    client = ajSoapAxis2GetClient(env, address);

    payload = textWsdbfetchFetchdataPayload(env, db, id, format);
    result = ajSoapAxis2Call(client, env, payload);

    if(result != NULL)
    {
	seqtext = textWsdbfetchGetEntryPtr(result, env);

	if(seqtext!=NULL)
	    ret = ajStrNewC(seqtext);
    }

    axis2_svc_client_free(client, env);
    axutil_env_free(env);

    return ret;
}




/* @funcstatic textEbeyeGetresultsParse ***************************************
**
** Parses results of ebeye webservices getResults calls
**
** @param [u] wsResult [axiom_node_t*] axis2 environment
** @param [r] env [const axutil_env_t*] axis2 environment
** @param [r] domain  [const AjPStr] domain
** @param [r] retfields  [const AjPStr] return fields
** @param [u] buff  [AjPFilebuff] Input file buffer
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void textEbeyeGetresultsParse(axiom_node_t *wsResult,
				     const axutil_env_t *env,
				     const AjPStr domain,
				     const AjPStr retfields,
				     AjPFilebuff buff)
{
    axis2_char_t* text   = NULL;
    axis2_char_t* name   = NULL;
    axiom_element_t* elm = NULL;
    axiom_node_t* node   = NULL;
    axiom_children_iterator_t* results = NULL;
    axiom_children_iterator_t* fields = NULL;
    axis2_svc_client_t* client; // client for the refentries call
    axutil_env_t* refenv;

    AjPStr val = NULL;
    AjPStr entryid   = NULL;
    AjPStrTok handle = NULL;
    AjPStr token     = NULL;

    ajuint i, n;
    AjPStr* fieldnames;


    if (axiom_node_get_node_type(wsResult, env) != AXIOM_ELEMENT)
    {
	ajErr(UNRECOGNISEDXML);
	return;
    }

    elm = axiom_node_get_data_element(wsResult, env);
    name = axiom_element_get_localname(elm, env);

    if (!ajCharMatchCaseC(name, "getResultsResponse"))
    {
	if(ajCharMatchCaseC(name, "Fault"))
	{
	    ajSoapAxis2Error(wsResult, env);
	}
	else
	    ajErr(UNRECOGNISEDXML);

	return;
    }

    node = axiom_node_get_first_child(wsResult, env);

    if (node == NULL)
    {
	ajErr(UNRECOGNISEDXML);
	return;
    }

    ajStrTokenAssignC(&handle, retfields, " ,");


    n = ajStrParseCountC(retfields, " ,");

    AJCNEW(fieldnames, n);
    i=0;

    while(ajStrTokenNextParse(handle, &token))
	fieldnames[i++] = ajStrNewS(token);

    results = axiom_element_get_children(elm, env, node);

    val = ajStrNew();
    client = ajSoapAxis2GetClient(env, EBEYE_EP);
    refenv = ajSoapAxis2GetEnv();


    while(results && axiom_children_iterator_has_next(results, env))
    {
	node = axiom_children_iterator_next(results, env);
	elm = axiom_node_get_data_element(node, env);
	fields = axiom_element_get_children(elm, env, node);

	i=0;

	while(fields && axiom_children_iterator_has_next(fields, env))
	{
	    node = axiom_children_iterator_next(fields, env);

	    if(axiom_node_get_node_type(node, env) == AXIOM_ELEMENT)
	    {
		elm = axiom_node_get_data_element(node, env);
		text = axiom_element_get_text(elm, env, node);

		if (text != NULL)
		{
		    ajFmtPrintS(&val, "%S: %s", fieldnames[i], text);
		    ajFilebuffLoadS(buff, val);

		    if(ajStrMatchC(fieldnames[i], "id"))
			ajStrAssignSubS(&entryid, val, 4, -1);
		}

		i++;
	    }
	}


	if(entryid)
	{
	    textEbeyeGetreferencedentries(client, refenv,
	                                  domain, buff, entryid);
	    ajStrDel(&entryid);
	}

	/* TODO: check why the empty line needs space character as well */
	ajFilebuffLoadC(buff, " \n");
    }

    for (i=0;i<n;i++)
	ajStrDel(&fieldnames[i]);

    axis2_svc_client_free(client, env);
    axutil_env_free(refenv);

    AJFREE(fieldnames);

    ajStrTokenDel(&handle);
    ajStrDel(&token);
    ajStrDel(&val);

    return;
}




/* @funcstatic textEbeyeGetdomainsreferencedinentry ***************************
**
** Returns the list of domains referenced in 'entry'
**
** @param [u] client [axis2_svc_client_t*] axis2 client
** @param [r] env [const axutil_env_t*] axis2 environment
** @param [r] domain [const AjPStr] ebeye domain name
** @param [r] entry [const AjPStr] entry id
** @return [AjPList] list of referenced domains
**
** @release 6.4.0
** @@
******************************************************************************/
static AjPList textEbeyeGetdomainsreferencedinentry(
					axis2_svc_client_t* client,
					const axutil_env_t * env,
					const AjPStr domain,
					const AjPStr entry)
{
    axiom_node_t* payload  = NULL;
    axiom_node_t* wsresult = NULL;
    AjPList refdomains = NULL;

    payload = textEbeyeGetdomainsreferencedinentryPayload(env, domain, entry);

    wsresult = ajSoapAxis2Call(client, env, payload);

    refdomains = textEbeyeGetdomainsreferencedinentryParse(wsresult, env);
    
    return refdomains;
}




/* @funcstatic textEbeyeGetreferencedentries **********************************
**
** Returns the list of domains referenced in 'entry'
**
** @param [u] client [axis2_svc_client_t*] axis2 client
** @param [r] env [const axutil_env_t*] axis2c environment
** @param [r] domain [const AjPStr] ebeye domain name for the entry
** @param [u] buff  [AjPFilebuff] buffer for saving ws results
** @param [r] entry [const AjPStr] entry (id)
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void textEbeyeGetreferencedentries(axis2_svc_client_t* client,
					  const axutil_env_t* env,
					  const AjPStr domain,
					  AjPFilebuff buff,
					  const AjPStr entry)
{
    AjIList i = NULL;
    AjPStr refdomain = NULL;
    axiom_node_t* payload;
    axiom_node_t* wsresult;
    AjPStr refentries;
    AjPStr refline = NULL;
    AjPList refdomains = NULL;
    ajint n;

    refdomains = textEbeyeGetdomainsreferencedinentry(client, env,
                                                      domain, entry);

    i = ajListIterNew(refdomains);

    refline = ajStrNewC("db_xref:");

    ajDebug("iterating over reference domains for entry %S\n", entry);

    while(!ajListIterDone(i))
    {
	refdomain = ajListIterGet(i);

	ajDebug("refdomain:%S\n", refdomain);

	payload = textEbeyeGetreferencedentriesPayload(env, domain,
	                                               entry, refdomain);

        wsresult = ajSoapAxis2Call(client, env, payload);

	refentries = textEbeyeGetreferencedentriesParse(wsresult, env);

	if(refentries && ajStrGetLen(refentries))
	{
	    ajStrAppendK(&refline, ' ');
	    ajStrAppendS(&refline, refdomain);

	    ajFmtPrintS(&refdomain,"%S: %S", refdomain, refentries);
	}
	else
	    ajStrSetClear(&refdomain);

	ajStrDel(&refentries);
    }

    ajListIterDel(&i);

    ajFilebuffLoadS(buff, refline);
    ajStrDel(&refline);

    n = (ajuint) ajListGetLength(refdomains);

    while(n-- > 0)
    {
	AjPStr tmp = NULL;
	ajListstrPop(refdomains, &tmp);

	if(ajStrGetLen(tmp))
	    ajFilebuffLoadS(buff, tmp);

	ajStrDel(&tmp);
    }

    ajListstrFree(&refdomains);

    return;
}




/* @funcstatic textEbeyeGetreferencedentriesParse *****************************
**
** Parses results of ebeye webservices getReferencedEntries calls
**
** @param [u] wsResult [axiom_node_t*] axis2 environment
** @param [r] env [const axutil_env_t*] axis2 environment
** @return [AjPStr] Referenced entry ids
**
** @release 6.4.0
** @@
******************************************************************************/

static AjPStr textEbeyeGetreferencedentriesParse(axiom_node_t *wsResult,
						 const axutil_env_t *env)
{
    axis2_char_t* text   = NULL;
    axis2_char_t* name   = NULL;
    axiom_element_t* elm = NULL;
    axiom_node_t* node   = NULL;
    axiom_children_iterator_t* domains = NULL;

    AjPStr refentries = NULL;

    if (axiom_node_get_node_type(wsResult, env) != AXIOM_ELEMENT)
    {
	ajErr(UNRECOGNISEDXML);
	return NULL;
    }

    elm = axiom_node_get_data_element(wsResult, env);
    name = axiom_element_get_localname(elm, env);

    if (!ajCharMatchCaseC(name, "getReferencedEntriesResponse"))
    {
	if(ajCharMatchCaseC(name, "Fault"))
	{
	    ajSoapAxis2Error(wsResult, env);
	}
	else
	    ajErr(UNRECOGNISEDXML);

	return NULL;
    }

    node = axiom_node_get_first_child(wsResult, env);

    if (node == NULL)
    {
	ajErr(UNRECOGNISEDXML);
	return NULL;
    }

    refentries = ajStrNew();

    domains = axiom_element_get_children(elm, env, node);

    while(domains && axiom_children_iterator_has_next(domains, env))
    {

	node = axiom_children_iterator_next(domains, env);

	elm = axiom_node_get_data_element(node, env);
	text = axiom_element_get_text(elm, env, node);

	ajDebug("ref entry: %s\n", text);

	if(ajStrGetLen(refentries))
	    ajStrAppendK(&refentries, ' ');

	ajStrAppendC(&refentries, text);
    }

    return refentries;
}




/* @funcstatic textEbeyeGetdomainsreferencedinentryParse **********************
**
** Parses results of ebeye webservices GetDomainsReferencedInEntry calls
**
** @param [u] wsResult [axiom_node_t*] webservices result obj
** @param [r] env [const axutil_env_t*] axis2 environment
** @return [AjPList] list of domains
**
** @release 6.4.0
** @@
******************************************************************************/

static AjPList textEbeyeGetdomainsreferencedinentryParse(
			axiom_node_t *wsResult, const axutil_env_t *env)
{
    axis2_char_t* text   = NULL;
    axis2_char_t* name   = NULL;
    axiom_element_t* elm = NULL;
    axiom_node_t* node   = NULL;

    AjPStr tmp = NULL;
    axiom_children_iterator_t* domains = NULL;
    AjPList refdomains;

    if (axiom_node_get_node_type(wsResult, env) != AXIOM_ELEMENT)
    {
	ajErr(UNRECOGNISEDXML);
	return NULL;
    }

    elm = axiom_node_get_data_element(wsResult, env);
    name = axiom_element_get_localname(elm, env);

    if (!ajCharMatchCaseC(name, "getDomainsReferencedInEntryResponse"))
    {
	if(ajCharMatchCaseC(name, "Fault"))
	{
	    ajSoapAxis2Error(wsResult, env);
	}
	else
	    ajErr(UNRECOGNISEDXML);

	return NULL;
    }

    node = axiom_node_get_first_child(wsResult, env);

    if (node == NULL)
    {
	ajErr(UNRECOGNISEDXML);
	return NULL;
    }

    refdomains = ajListstrNew();

    domains = axiom_element_get_children(elm, env, node);

    while(domains && axiom_children_iterator_has_next(domains, env))
    {

	node = axiom_children_iterator_next(domains, env);

	elm = axiom_node_get_data_element(node, env);
	text = axiom_element_get_text(elm, env, node);

	tmp = ajStrNewC(text);
	ajDebug("domain: %S\n", tmp);
	ajListstrPushAppend(refdomains, tmp);
	tmp = NULL;
    }

    return refdomains;
}




/* @funcstatic textEbeyeGetnumberofresultsParse *******************************
**
** Parses results of ebeye webservices getNumberOfResults calls
**
** @param [u] wsResult [axiom_node_t*] webservices result obj
** @param [r] env [const axutil_env_t*] axis2 environment
** @return [ajint] Number of results
**
** @release 6.4.0
** @@
******************************************************************************/

static ajint textEbeyeGetnumberofresultsParse(axiom_node_t *wsResult,
                                              const axutil_env_t *env)
{
    axis2_char_t* text     = NULL;
    axis2_char_t* name   = NULL;
    axiom_element_t* elm = NULL;
    axiom_node_t* node   = NULL;
    AjPStr tmp = NULL;
    ajint ret;

    if (axiom_node_get_node_type(wsResult, env) != AXIOM_ELEMENT)
    {
	ajErr(UNRECOGNISEDXML);
	return 0;
    }

    elm = axiom_node_get_data_element(wsResult, env);
    name = axiom_element_get_localname(elm, env);

    if (!ajCharMatchCaseC(name, "getNumberOfResultsResponse"))
    {
	if(ajCharMatchCaseC(name, "Fault"))
	{
	    ajSoapAxis2Error(wsResult, env);
	}
	else
	    ajErr(UNRECOGNISEDXML);

	return 0;
    }

    node = axiom_node_get_first_child(wsResult, env);

    if (node == NULL)
    {
	ajErr(UNRECOGNISEDXML);
	return 0;
    }



    elm = axiom_node_get_data_element(node, env);
    text = axiom_element_get_text(elm, env, node);

    tmp = ajStrNewC(text);

    ajStrToInt(tmp, &ret);

    ajStrDel(&tmp);

    return ret;
}




/* @funcstatic textEbeyeGetreferencedentriesPayload ***************************
**
** Prepares input for ebeye webservices getReferencedEntries calls
**
** @param [r] env [const axutil_env_t*] axis2 environment
** @param [r] domain [const AjPStr] ebeye domain name
** @param [r] entry  [const AjPStr] entry id
** @param [r] refdomain  [const AjPStr] reference domain
** @return [axiom_node_t*] axis2 OM object, for service input
**
** @release 6.4.0
** @@
******************************************************************************/

static axiom_node_t* textEbeyeGetreferencedentriesPayload(
						const axutil_env_t * env,
                                                const AjPStr domain,
                                                const AjPStr entry,
                                                const AjPStr refdomain)
{
    AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    AjPStr tmp       = NULL;

    axiom_node_t *payload = NULL;
    axiom_element_t *elm  = NULL;
    axiom_node_t *child   = NULL;
    axiom_namespace_t *ns = NULL;

    ajDebug("textEbeyeGetreferencedentriesPayload domain:%S refdomain:%S\n",
	    domain, refdomain);

    ns = axiom_namespace_create(env, EBEYE_NS, "ebeye");
    elm = axiom_element_create(env, NULL, "getReferencedEntries",
                               ns, &payload);

    elm = axiom_element_create(env, payload, "domain", ns, &child);
    axiom_element_set_text(elm, env, ajStrGetPtr(domain), child);

    elm = axiom_element_create(env, payload, "entry", ns, &child);
    axiom_element_set_text(elm, env, ajStrGetPtr(entry), child);

    elm = axiom_element_create(env, payload, "referencedDomain", ns, &child);
    axiom_element_set_text(elm, env, ajStrGetPtr(refdomain), child);

    ajStrTokenDel(&handle);
    ajStrDel(&token);
    ajStrDel(&tmp);

    return payload;
}




/* @funcstatic textEbeyeGetresultsPayload *************************************
**
** Prepares input for ebeye webservices getResults calls
**
** @param [r] env [const axutil_env_t*] axis2 environment
** @param [r] domain [const AjPStr] ebeye domain name
** @param [r] query  [const AjPStr] ebeye query
** @param [r] retfields  [const AjPStr] return fields
** @param [r] start  [ajint] Start number
** @return [axiom_node_t*] axis2 OM object, for service input
**
** @release 6.4.0
** @@
******************************************************************************/

static axiom_node_t* textEbeyeGetresultsPayload(const axutil_env_t * env,
                                                const AjPStr domain,
                                                const AjPStr query,
                                                const AjPStr retfields,
                                                ajint start)
{
    AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    AjPStr tmp       = NULL;

    axiom_node_t *payload = NULL;
    axiom_element_t *elm  = NULL;
    axiom_node_t *child   = NULL;
    axiom_node_t *field   = NULL;
    axiom_namespace_t *ns = NULL;

    ns = axiom_namespace_create(env, EBEYE_NS, "ebeye");
    elm = axiom_element_create(env, NULL, "getResults", ns, &payload);

    elm = axiom_element_create(env, payload, "domain", ns, &child);
    axiom_element_set_text(elm, env, ajStrGetPtr(domain), child);

    elm = axiom_element_create(env, payload, "query", ns, &child);
    axiom_element_set_text(elm, env, ajStrGetPtr(query), child);


    /* processing return fields */

    axiom_element_create(env, payload, "fields", ns, &child);
    ajDebug("textEbeyeGetresultsPayload qry '%s' retfields '%S'\n",
            query, retfields);
    ajStrTokenAssignC(&handle, retfields, "\t ,;\n\r");

    while(ajStrTokenNextParse(handle, &token))
    {
	elm = axiom_element_create(env, child, "string", ns, &field);
	axiom_element_set_text(elm, env, ajStrGetuniquePtr(&token), field);
    }

    elm = axiom_element_create(env, payload, "start", ns, &child);

    ajStrFromInt(&tmp, start);
    axiom_element_set_text(elm, env, ajStrGetPtr(tmp), child);
    elm = axiom_element_create(env, payload, "size", ns, &child);
    axiom_element_set_text(elm, env, "100", child);

    ajStrTokenDel(&handle);
    ajStrDel(&token);
    ajStrDel(&tmp);

    return payload;
}




/* @funcstatic textEbeyeGetdomainsreferencedinentryPayload ********************
**
** Prepares input for ebeye webservices GetDomainsReferencedInEntry calls
**
** @param [r] env [const axutil_env_t*] axis2 environment
** @param [r] domain [const AjPStr] ebeye domain name
** @param [r] entry [const AjPStr] entry id
** @return [axiom_node_t*] axis2 OM object, for service input
**
** @release 6.4.0
** @@
******************************************************************************/

static axiom_node_t* textEbeyeGetdomainsreferencedinentryPayload(
					    const axutil_env_t * env,
					    const AjPStr domain,
					    const AjPStr entry)
{
    AjPStrTok handle = NULL;
    AjPStr token     = NULL;

    axiom_node_t *payload = NULL;
    axiom_element_t *elm  = NULL;
    axiom_node_t *child   = NULL;
    axiom_namespace_t *ns = NULL;

    ns = axiom_namespace_create(env, EBEYE_NS, "ebeye");
    elm = axiom_element_create(env, NULL, "getDomainsReferencedInEntry",
                               ns, &payload);

    elm = axiom_element_create(env, payload, "domain", ns, &child);
    axiom_element_set_text(elm, env, ajStrGetPtr(domain), child);

    elm = axiom_element_create(env, payload, "entry", ns, &child);
    axiom_element_set_text(elm, env, ajStrGetPtr(entry), child);

    ajStrTokenDel(&handle);
    ajStrDel(&token);

    return payload;
}




/* @funcstatic textEbeyeGetnumberofresultsPayload *****************************
**
** Prepares input for ebeye webservices getNumberOfResults calls
**
** @param [r] env [const axutil_env_t*] axis2 environment
** @param [r] domain [const AjPStr] ebeye domain name
** @param [r] query  [const AjPStr] ebeye query
** @return [axiom_node_t*] axis2 OM object, for service input
**
** @release 6.4.0
** @@
******************************************************************************/

static axiom_node_t* textEbeyeGetnumberofresultsPayload(
    const axutil_env_t * env,
    const AjPStr domain,
    const AjPStr query)
{
    AjPStrTok handle = NULL;
    AjPStr token     = NULL;

    axiom_node_t *payload = NULL;
    axiom_element_t *elm  = NULL;
    axiom_node_t *child   = NULL;
    axiom_namespace_t *ns = NULL;

    ns = axiom_namespace_create(env, EBEYE_NS, "ebeye");
    elm = axiom_element_create(env, NULL, "getNumberOfResults", ns, &payload);

    elm = axiom_element_create(env, payload, "domain", ns, &child);
    axiom_element_set_text(elm, env, ajStrGetPtr(domain), child);

    elm = axiom_element_create(env, payload, "query", ns, &child);
    axiom_element_set_text(elm, env, ajStrGetPtr(query), child);

    ajStrTokenDel(&handle);
    ajStrDel(&token);

    return payload;
}




/* @funcstatic textWsdbfetchFetchdataPayload **********************************
**
** Prepares input for wsdbfetch fetchData webservice calls
**
** @param [r] env [const axutil_env_t*] axis2 environment
** @param [r] db [const AjPStr] wsdbfetch database name
** @param [r] id [const AjPStr] wsdbfetch entry identifier
** @param [r] format [const AjPStr] wsdbfetch entry format name
** @return [axiom_node_t*] axis2 OM object, for service input
**
** @release 6.4.0
** @@
******************************************************************************/

static axiom_node_t* textWsdbfetchFetchdataPayload(const axutil_env_t * env,
                                                   const AjPStr db,
                                                   const AjPStr id,
                                                   const AjPStr format)
{
    AjPStr usa            = NULL;
    axiom_node_t *payload = NULL;
    axiom_element_t *elm  = NULL;
    axiom_node_t *child   = NULL;
    axiom_namespace_t *ns = NULL;
    const char* formatc   = "default";

    if(!ajStrMatchC(format, "unknown"))
	formatc = ajStrGetPtr(format);

    ns = axiom_namespace_create(env, WSDBFETCH_NS, "wsdbfetch");
    elm = axiom_element_create(env, NULL, "fetchData", ns, &payload);

    ajFmtPrintS(&usa,"%S:%S",db,id);
    elm = axiom_element_create(env, payload, "query", ns, &child);
    axiom_element_set_text(elm, env, ajStrGetPtr(usa), child);

    elm = axiom_element_create(env, payload, "format", ns, &child);
    axiom_element_set_text(elm, env, formatc, child);

    elm = axiom_element_create(env, payload, "style", ns, &child);
    axiom_element_set_text(elm, env, "raw", child);

    ajStrDel(&usa);

    return payload;
}




/* @funcstatic textWsdbfetchGetEntryPtr ***************************************
**
** Returns the entry string from result objects obtained by wsdbfetch call
**
** @param [u] wsResult [axiom_node_t*] axis2 OM object
**				       obtained by wsdbfetch call
** @param [r] env [const axutil_env_t*] axis2 environment
** @return [char*] sequence string
**
** @release 6.4.0
** @@
******************************************************************************/

static char* textWsdbfetchGetEntryPtr(axiom_node_t *wsResult,
                                      const axutil_env_t *env)
{
    axis2_char_t* result = NULL;
    axis2_char_t* name   = NULL;
    axiom_element_t* elm = NULL;
    axiom_node_t* node   = NULL;

    if (axiom_node_get_node_type(wsResult, env) != AXIOM_ELEMENT)
    {
	ajErr(UNRECOGNISEDXML);
	return NULL;
    }

    elm = axiom_node_get_data_element(wsResult, env);
    name = axiom_element_get_localname(elm, env);

    if (!ajCharMatchCaseC(name, "fetchDataResponse"))
    {
	if(ajCharMatchCaseC(name, "Fault"))
	{
	    ajSoapAxis2Error(wsResult, env);
	}
	else
	    ajErr(UNRECOGNISEDXML);

	return NULL;
    }

    node = axiom_node_get_first_element(wsResult, env);

    if (!node)
    {
	ajErr(UNRECOGNISEDXML);
	return NULL;
    }

    elm = axiom_node_get_data_element(node, env);
    result = axiom_element_get_text(elm, env, node);

    return (char*) result;
}


#endif

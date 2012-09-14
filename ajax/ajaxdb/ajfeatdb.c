/* @source ajfeatdb ***********************************************************
**
** AJAX feature database access functions
**
** These functions control all aspects of AJAX feature database access
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.44 $
** @modified Sep 2010 pmr first version
** @modified $Date: 2012/07/14 14:52:39 $ by $Author: rice $
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

#include "ajfeatdb.h"
#include "ajfeat.h"
#include "ajfeatread.h"
#include "ajtextdata.h"

#include "ajtagval.h"
#include "ajsql.h"
#include "ajindex.h"
#include "ajhttp.h"
#include "ajutil.h"
#include "ajnam.h"
#include "ajcall.h"
#include "ajfileio.h"


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



static AjPRegexp featCdDivExp = NULL;

static AjPRegexp featRegGcgId = NULL;
static AjPRegexp featRegGcgCont = NULL;
static AjPRegexp featRegGcgId2 = NULL;
static AjPRegexp featRegGcgSplit = NULL;

static AjPRegexp featRegGcgRefId = NULL;

static char* featCdName = NULL;
static ajuint featCdMaxNameSize = 0;


/* @datastatic FeatPCdDiv *****************************************************
**
** EMBLCD division file record structure
**
** @alias FeatSCdDiv
** @alias FeatOCdDiv
**
** @attr FileName [AjPStr] Filename(s)
** @attr DivCode [ajuint] Division code
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct FeatSCdDiv
{
    AjPStr FileName;
    ajuint DivCode;
    char Padding[4];
} FeatOCdDiv;

#define FeatPCdDiv FeatOCdDiv*




/* @datastatic FeatPCdEntry ***************************************************
**
** EMBLCD entrynam.idx file record structure
**
** @alias FeatSCdEntry
** @alias FeatOCdEntry
**
** @attr div [ajuint] division file record
** @attr annoff [ajuint] data file offset
** @attr seqoff [ajuint] sequence file offset (if any)
** @@
******************************************************************************/

typedef struct FeatSCdEntry
{
    ajuint div;
    ajuint annoff;
    ajuint seqoff;
} FeatOCdEntry;

#define FeatPCdEntry FeatOCdEntry*




/* @datastatic FeatPCdFHeader **************************************************
**
** EMBLCD index file header structure, same for all index files.
**
** @alias FeatSCdFHeader
** @alias FeatOCdFHeader
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

typedef struct FeatSCdFHeader
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
} FeatOCdFHeader;

#define FeatPCdFHeader FeatOCdFHeader*




/* @datastatic FeatPCdFile ****************************************************
**
** EMBLCD file data structure
**
** @alias FeatSCdFile
** @alias FeatOCdFile
**
** @attr Header [FeatPCdFHeader] Header data
** @attr File [AjPFile] File
** @attr NRecords [ajuint] Number of records
** @attr RecSize [ajuint] Record length (for calculating record offsets)
** @@
******************************************************************************/

typedef struct FeatSCdFile
{
    FeatPCdFHeader Header;
    AjPFile File;
    ajuint NRecords;
    ajuint RecSize;
} FeatOCdFile;

#define FeatPCdFile FeatOCdFile*




/* @datastatic FeatPCdHit *****************************************************
**
** EMBLCD hit file record structure
**
** @alias FeatSCdHit
** @alias FeatOCdHit
**
** @attr HitList [ajuint*] Array of hits, as record numbers in the
**                         entrynam.idx file
** @attr NHits [ajuint] Number of hits in HitList array
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct FeatSCdHit
{
    ajuint* HitList;
    ajuint NHits;
    char Padding[4];
} FeatOCdHit;

#define FeatPCdHit FeatOCdHit*




/* @datastatic FeatPCdIdx *****************************************************
**
** EMBLCD entryname index file record structure
**
** @alias FeatSCdIdx
** @alias FeatOCdIdx
**
** @attr AnnOffset [ajuint] Data file offset (see DivCode)
** @attr SeqOffset [ajuint] Sequence file offset (if any) (see DivCode)
** @attr EntryName [AjPStr] Entry ID - the file is sorted by these
** @attr DivCode [ajushort] Division file record
** @attr Padding [char[6]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct FeatSCdIdx
{
    ajuint AnnOffset;
    ajuint SeqOffset;
    AjPStr EntryName;
    ajushort DivCode;
    char Padding[6];
} FeatOCdIdx;

#define FeatPCdIdx FeatOCdIdx*




/* @datastatic FeatPCdTrg *****************************************************
**
** EMBLCD target (.trg) file record structure
**
** @alias FeatSCdTrg
** @alias FeatOCdTrg
**
** @attr FirstHit [ajuint] First hit record in .hit file
** @attr NHits [ajuint] Number of hit records in .hit file
** @attr Target [AjPStr] Indexed target string (the file is sorted by these)
** @@
******************************************************************************/

typedef struct FeatSCdTrg
{
    ajuint FirstHit;
    ajuint NHits;
    AjPStr Target;
} FeatOCdTrg;

#define FeatPCdTrg FeatOCdTrg*




/* @datastatic FeatPCdQry *****************************************************
**
** EMBLCD query structure
**
** @alias FeatSCdQry
** @alias FeatOCdQry
**
** @attr divfile [AjPStr] division.lkp
** @attr idxfile [AjPStr] entryname.idx
** @attr datfile [AjPStr] main data reference
** @attr seqfile [AjPStr] sequence
** @attr tblfile [AjPStr] BLAST table
** @attr srcfile [AjPStr] BLAST FASTA source data
** @attr dfp [FeatPCdFile] division.lkp
** @attr ifp [FeatPCdFile] entryname.idx
** @attr trgfp [FeatPCdFile] acnum.trg
** @attr hitfp [FeatPCdFile] acnum.hit
** @attr trgLine [FeatPCdTrg]acnum input line
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
** @attr Skip [AjBool*] skip file(s) in division.lkp
** @attr idxLine [FeatPCdIdx] entryname.idx input line
** @attr Samefile [AjBool] true if the same file is passed to
**                         ajFilebuffReopenFile
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct FeatSCdQry
{
    AjPStr divfile;
    AjPStr idxfile;
    AjPStr datfile;
    AjPStr seqfile;
    AjPStr tblfile;
    AjPStr srcfile;

    FeatPCdFile dfp;
    FeatPCdFile ifp;
    FeatPCdFile trgfp;
    FeatPCdFile hitfp;
    FeatPCdTrg trgLine;

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

    AjBool* Skip;
    FeatPCdIdx idxLine;
    AjBool Samefile;
    char Padding[4];
} FeatOCdQry;

#define FeatPCdQry FeatOCdQry*




/* @datastatic FeatPEmbossQry *************************************************
**
** Btree 'emboss' query structure
**
** @alias FeatSEmbossQry
** @alias FeatOEmbossQry
**
** @attr idcache [AjPBtcache] ID cache
** @attr Caches [AjPList] Caches for each query field
** @attr files [AjPStr*] database filenames
** @attr reffiles [AjPStr**] database reference filenames
** @attr Skip [AjBool*] files numbers to exclude
** @attr List [AjPList] List of files
** @attr libs [AjPFile] Primary (database source) file
** @attr libr [AjPFile] Secondary (database bibliographic source) file
** @attr div [ajuint] division number of currently open database file
** @attr refcount [ajuint] number of reference file(s) per entry
** @attr nentries [ajint] number of entries in the filename array(s)
**                        -1 when done
** @attr Samefile [AjBool] true if the same file is passed to
**                         ajFilebuffReopenFile
** @@
******************************************************************************/

typedef struct FeatSEmbossQry
{
    AjPBtcache idcache;
    AjPList Caches;

    AjPStr *files;
    AjPStr **reffiles;
    AjBool *Skip;

    AjPList List;

    AjPFile libs;
    AjPFile libr;

    ajuint div;
    ajuint refcount;
    ajint nentries;

    AjBool Samefile;
} FeatOEmbossQry;

#define FeatPEmbossQry FeatOEmbossQry*



static AjBool featAccessDas(AjPFeattabin ftabin);
static AjBool featAccessChado(AjPFeattabin fttabin);
static AjBool featAccessEmbossGcg(AjPFeattabin fttabin);
static AjBool featAccessGcg(AjPFeattabin fttabin);

static AjPSqlconnection featChadoConnect(const AjPQuery qry);
static void featChadoChildfeatureQuery(AjPSqlconnection connection,
			               AjPFeattable  feattab,
                                       const AjPStr srcfeature);

static AjBool featChadoQryfeatureQuery(AjPSqlconnection connection, AjPStr sql,
			               AjPFeattable feattab,
			               ajint qrystart, ajint qryend);

static AjPFeature featChadoChildfeatureRow(AjPFeattable fttab, AjPSqlrow line);
static AjPStr featChadoQryfeatureRow(AjPFeattable fttab, AjPSqlrow row,
				     ajint qrystart, ajint qryend);

static ajuint      featCdDivNext(AjPQuery qry);
static void        featCdIdxDel(FeatPCdIdx* pthys);
static void        featCdTrgDel(FeatPCdTrg* pthys);

static int         featCdEntryCmp(const void* a, const void* b);
static void        featCdEntryDel(void** pentry, void* cl);
static void        featCdFileClose(FeatPCdFile *thys);
static FeatPCdFile featCdFileOpen(const AjPStr dir, const char* name,
                                  AjPStr* fullname);
static ajint       featCdFileSeek(FeatPCdFile fil, ajuint ipos);
static void        featCdIdxLine(FeatPCdIdx idxLine,  ajuint ipos,
                                 FeatPCdFile fp);
static char*       featCdIdxName(ajuint ipos, FeatPCdFile fp);
static AjBool      featCdIdxQuery(AjPQuery qry, const AjPStr idqry);
static ajuint      featCdIdxSearch(FeatPCdIdx idxLine, const AjPStr entry,
                                   FeatPCdFile fp);
static AjBool      featCdQryClose(AjPQuery qry);
static AjBool      featCdQryEntry(AjPQuery qry);
static AjBool      featCdQryFile(AjPQuery qry);
static AjBool      featCdQryOpen(AjPQuery qry);
static AjBool      featCdQryNext(AjPQuery qry);
static AjBool      featCdQryQuery(AjPQuery qry);
static AjBool      featCdQryReuse(AjPQuery qry);
static AjBool      featCdReadHeader(FeatPCdFile fp);
static AjBool      featCdTrgClose(FeatPCdFile *trgfil, FeatPCdFile *hitfil);
static ajuint      featCdTrgFind(AjPQuery qry, const char* indexname,
                                 const AjPStr qrystring);
static void        featCdTrgLine(FeatPCdTrg trgLine, ajuint ipos,
                                 FeatPCdFile fp);
static char*       featCdTrgName(ajuint ipos, FeatPCdFile fp);
static AjBool      featCdTrgOpen(const AjPStr dir, const char* name,
                                 FeatPCdFile *trgfil, FeatPCdFile *hitfil);
static AjBool      featCdTrgQuery(AjPQuery qry, const AjPStr field,
                                const AjPStr wildqry);
static ajuint      featCdTrgSearch(FeatPCdTrg trgLine, const AjPStr name,
                                 FeatPCdFile fp);
static AjBool      featEmbossGcgAll(AjPFeattabin fttabin);
static void        featEmbossGcgLoadBuff(AjPFeattabin fttabin);
static AjBool      featEmbossGcgReadRef(AjPFeattabin fttabin);
static AjBool      featEmbossGcgReadSeq(AjPFeattabin fttabin);

static AjBool      featEmbossOpenCache(AjPQuery qry, const char *ext,
                                       AjPBtcache *cache);
static AjBool      featEmbossQryClose(AjPQuery qry);
static AjBool      featEmbossQryEntry(AjPQuery qry);
static AjBool      featEmbossQryNext(AjPQuery qry);
static AjBool      featEmbossQryOpen(AjPQuery qry);
static AjBool      featEmbossQryOrganisms(AjPQuery qry);
static AjBool      featEmbossQryQuery(AjPQuery qry);
static AjBool      featEmbossQryReuse(AjPQuery qry);

static AjBool      featGcgAll(AjPFeattabin fttabin);
static void        featGcgBinDecode(AjPStr *pthis, ajuint rdlen);
static void        featGcgLoadBuff(AjPFeattabin fttabin);
static AjBool      featGcgReadRef(AjPFeattabin fttabin);
static AjBool      featGcgReadSeq(AjPFeattabin fttabin);




/* @funclist feattabAccess ****************************************************
**
** Functions to access each database or feature access method
**
******************************************************************************/

static AjOFeattabAccess feattabAccess[] =
{
  /*  Name     AccessFunction  FreeFunction
      Qlink    Description
      Alias    Entry    Query    All      Chunk     Padding */

    {
      "das",   &featAccessDas, NULL,
      "&",     "retrieve features from a DAS server",
      AJFALSE, AJTRUE,  AJFALSE,  AJFALSE, AJFALSE, AJFALSE
    },
    {
      "chado", &featAccessChado, NULL,
      "",      "retrieve features from a CHADO server",
      AJFALSE, AJTRUE,  AJFALSE,  AJFALSE, AJFALSE, AJFALSE
    },
    {
      "gcg",   &featAccessGcg, NULL,
      "|&!^=", "emboss dbigcg indexed",
      AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  AJFALSE, AJFALSE
    },
    {
      "embossgcg", &featAccessEmbossGcg, NULL,
      "|&!^=", "emboss dbxgcg indexed",
      AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  AJFALSE, AJFALSE
    },
    {
      NULL, NULL, NULL,
      NULL, NULL,
      AJFALSE, AJFALSE, AJFALSE, AJFALSE, AJFALSE, AJFALSE
    }
};




/* @func ajFeatdbInit *********************************************************
**
** Initialise feature database internals
**
** @return [void]
**
** @release 6.4.0
******************************************************************************/

void ajFeatdbInit(void)
{
    AjPTable table;
    ajuint i = 0;

    table = ajFeattabaccessGetDb();

    while(feattabAccess[i].Name)
    {
        ajCallTableRegister(table, feattabAccess[i].Name,
                            (void*) &feattabAccess[i]);
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




/* @funcstatic featCdFileOpen *************************************************
**
** Opens a named EMBL CD-ROM index file.
**
** @param [r] dir [const AjPStr] Directory
** @param [r] name [const char*] File name.
** @param [w] fullname [AjPStr*] Full file name with directory path
** @return [FeatPCdFile] EMBL CD-ROM index file object.
**
** @release 6.5.0
** @@
******************************************************************************/

static FeatPCdFile featCdFileOpen(const AjPStr dir, const char* name,
                                  AjPStr* fullname)
{
    FeatPCdFile thys = NULL;


    AJNEW0(thys);

    thys->File = ajFileNewInNamePathC(name, dir);

    if(!thys->File)
    {
        AJFREE(thys);

        return NULL;
    }


    AJNEW0(thys->Header);

    featCdReadHeader(thys);
    thys->NRecords = thys->Header->NRecords;
    thys->RecSize = thys->Header->RecSize;

    ajStrAssignS(fullname, ajFileGetPrintnameS(thys->File));

    ajDebug("featCdFileOpen '%F' NRecords: %d RecSize: %d\n",
            thys->File, thys->NRecords, thys->RecSize);


    return thys;
}




/* @funcstatic featCdFileSeek *************************************************
**
** Sets the file position in an EMBL CD-ROM index file.
**
** @param [u] fil [FeatPCdFile] EMBL CD-ROM index file object.
** @param [r] ipos [ajuint] Offset.
** @return [ajint] Return value from the seek operation.
**
** @release 6.5.0
** @@
******************************************************************************/


static ajint featCdFileSeek(FeatPCdFile fil, ajuint ipos)
{
    ajint ret;
    ajuint jpos;

    jpos = 300 + ipos*fil->RecSize;
    ret = ajFileSeek(fil->File, jpos, 0);

    /*
      ajDebug("featCdFileSeek rec %u pos %u tell %Ld returns %d\n",
      ipos, jpos, ajFileResetPos(fil->File), ret);
    */

    return ret;
}




/* @funcstatic featCdFileClose ************************************************
**
** Closes an EMBL CD-ROM index file.
**
** @param [d] pthis [FeatPCdFile*] EMBL CD-ROM index file.
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void featCdFileClose(FeatPCdFile* pthis)
{
    FeatPCdFile thys;

    thys = *pthis;

    if(!thys)
        return;

    ajDebug("featCdFileClose of %F\n", (*pthis)->File);

    ajFileClose(&thys->File);
    AJFREE(thys->Header);
    AJFREE(*pthis);

    return;
}




/* @funcstatic featCdIdxSearch ************************************************
**
** Binary search through an EMBL CD-ROM index file for an exact match.
**
** @param [u] idxLine [FeatPCdIdx] Index file record.
** @param [r] entry [const AjPStr] Entry name to search for.
** @param [u] fil [FeatPCdFile] EMBL CD-ROM index file.
** @return [ajuint] Record number on success, -1 on failure.
**
** @release 6.5.0
** @@
******************************************************************************/

static ajuint featCdIdxSearch(FeatPCdIdx idxLine, const AjPStr entry,
                              FeatPCdFile fil)
{
    AjPStr entrystr = NULL;
    ajint ihi;
    ajint ilo;
    ajint ipos = 0;
    ajint icmp = 0;
    char *name;

    ajStrAssignS(&entrystr, entry);
    ajStrFmtUpper(&entrystr);

    ajDebug("featCdIdxSearch (entry '%S') records: %d\n",
            entrystr, fil->NRecords);

    if(fil->NRecords < 1)
        return -1;

    ilo = 0;
    ihi = fil->NRecords - 1;

    while(ilo <= ihi)
    {
        ipos = (ilo + ihi)/2;
        name = featCdIdxName(ipos, fil);
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

    featCdIdxLine(idxLine, ipos, fil);

    return ipos;
}




/* @funcstatic featCdIdxQuery *************************************************
**
** Binary search of an EMBL CD-ROM index file for entries matching a
** wildcard entry name.
**
** @param [u] qry [AjPQuery] Query object.
** @param [r] idqry [const AjPStr] ID Query
** @return [AjBool] ajTrue on success.
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool featCdIdxQuery(AjPQuery qry, const AjPStr idqry)
{
    FeatPCdQry qryd;

    AjPList list;
    FeatPCdIdx idxLine;
    FeatPCdFile fil;

    AjPStr idstr = NULL;
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

    FeatPCdEntry entry;

    qryd    = qry->QryData;
    list    = qry->ResultsList;
    idxLine = qryd->idxLine;
    fil     = qryd->ifp;

    ajStrAssignS(&idstr,idqry);
    ajStrFmtUpper(&idstr);
    ajStrAssignS(&idpref, idstr);

    ajStrRemoveWild(&idpref);

    ajDebug("featCdIdxQuery (wild '%S' prefix '%S')\n",
            idstr, idpref);

    jlo = ilo = 0;
    khi = jhi = ihi = fil->NRecords-1;

    ilen = ajStrGetLen(idpref);
    first = ajTrue;

    if(ilen)
    {                          /* find first entry with this prefix */
        while(ilo <= ihi)
        {
            ipos = (ilo + ihi)/2;
            name = featCdIdxName(ipos, fil);
            name[ilen] = '\0';
            icmp = ajStrCmpC(idpref, name); /* test prefix */
            ajDebug("idx test %d '%s' %2d (+/- %d)\n",
                    ipos, name, icmp, ihi-ilo);

            if(!icmp)
            {                        /* hit prefix - test for first */
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
        {                         /* failed to find any with prefix */
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
            name = featCdIdxName(ipos, fil);
            name[ilen] = '\0';
            icmp = ajStrCmpC(idpref, name);
            ajDebug("idx test %d '%s' %2d (+/- %d)\n",
                    ipos, name, icmp, ihi-ilo);

            if(!icmp)
            {                           /* hit prefix */
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

        name = featCdIdxName(jlo, fil);
        ajDebug("first  %d '%s'\n", jlo, name);
        name = featCdIdxName(khi, fil);
        ajDebug(" last  %d '%s'\n", khi, name);
    }

    for(i=jlo; i < (khi+1); i++)
    {
        featCdIdxLine(idxLine, i, fil);

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




/* @funcstatic featCdTrgSearch ************************************************
**
** Binary search of EMBL CD-ROM target file, for example an accession number
** search.
**
** @param [u] trgLine [FeatPCdTrg] Target file record.
** @param [r] entry [const AjPStr] Entry name or accession number.
** @param [u] fp [FeatPCdFile] EMBL CD-ROM target file
** @return [ajuint] Record number, or -1 on failure.
**
** @release 6.5.0
** @@
******************************************************************************/

static ajuint featCdTrgSearch(FeatPCdTrg trgLine, const AjPStr entry,
                              FeatPCdFile fp)
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
    ajDebug("featCdTrgSearch '%S' recSize: %d\n", entry, fp->RecSize);
    name = featCdTrgName(ipos, fp);
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
        name = featCdTrgName(ipos, fp);
        icmp = ajStrCmpC(entrystr, name);
        ajDebug("trg testb %d '%s' %2d (+/- %d)\n",
                ipos, name, icmp, ihi-ilo);
    }

    featCdTrgLine(trgLine, ipos, fp);

    ajStrDel(&entrystr);

    if(!trgLine->NHits)
        return -1;

    ajDebug("found in .trg at record %d\n", ipos);


    return ipos;
}




/* @funcstatic featCdIdxName **************************************************
**
** Reads the name from record ipos of an EMBL CD-ROM index file.
** The name length is known from the index file object.
**
** @param [r] ipos [ajuint] Record number.
** @param [u] fil [FeatPCdFile] EMBL CD-ROM index file.
** @return [char*] Name read from file.
**
** @release 6.5.0
** @@
******************************************************************************/

static char* featCdIdxName(ajuint ipos, FeatPCdFile fil)
{
    ajuint nameSize;

    nameSize = fil->RecSize-10;

    if(featCdMaxNameSize < nameSize)
    {
        featCdMaxNameSize = nameSize;
        if(featCdName)
            ajCharDel(&featCdName);
        featCdName = ajCharNewRes(featCdMaxNameSize+1);
    }

    featCdFileSeek(fil, ipos);
    ajReadbinCharTrim(fil->File, nameSize, featCdName);

    return featCdName;
}




/* @funcstatic featCdIdxLine **************************************************
**
** Reads a numbered record from an EMBL CD-ROM index file.
**
** @param [u] idxLine [FeatPCdIdx] Index file record.
** @param [r] ipos [ajuint] Record number.
** @param [u] fil [FeatPCdFile] EMBL CD-ROM index file.
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void featCdIdxLine(FeatPCdIdx idxLine, ajuint ipos, FeatPCdFile fil)
{
    ajuint nameSize;

    nameSize = fil->RecSize-10;

    ajDebug("featCdIdxLine nameSize: %u max: %u ipos: %u '%F'\n",
            nameSize, featCdMaxNameSize, ipos, fil->File);

    if(featCdMaxNameSize < nameSize)
    {
        featCdMaxNameSize = nameSize;

        if(featCdName)
            ajCharDel(&featCdName);

        featCdName = ajCharNewRes(featCdMaxNameSize+1);
    }

    featCdFileSeek(fil, ipos);
    ajReadbinCharTrim(fil->File, nameSize, featCdName);

    ajStrAssignC(&idxLine->EntryName,featCdName);

    ajReadbinUint(fil->File, &idxLine->AnnOffset);
    ajReadbinUint(fil->File, &idxLine->SeqOffset);
    ajReadbinUint2(fil->File, &idxLine->DivCode);

    ajDebug("read ann: %u seq: %u div: %u\n",
            idxLine->AnnOffset, idxLine->SeqOffset,
            (ajuint) idxLine->DivCode);
    return;
}




/* @funcstatic featCdTrgName **************************************************
**
** Reads the target name from an EMBL CD-ROM index target file.
**
** @param [r] ipos [ajuint] Record number.
** @param [u] fil [FeatPCdFile] EMBL CD-ROM index target file.
** @return [char*] Name.
**
** @release 6.5.0
** @@
******************************************************************************/

static char* featCdTrgName(ajuint ipos, FeatPCdFile fil)
{
    ajuint nameSize;
    ajint i;

    nameSize = fil->RecSize-8;

    if(featCdMaxNameSize < nameSize)
    {
        featCdMaxNameSize = nameSize;

        if(featCdName)
            ajCharDel(&featCdName);

        featCdName = ajCharNewRes(featCdMaxNameSize+1);
    }

    featCdFileSeek(fil, ipos);
    ajReadbinInt(fil->File, &i);
    ajReadbinInt(fil->File, &i);
    ajReadbinCharTrim(fil->File, nameSize, featCdName);

    ajDebug("featCdTrgName maxNameSize:%d nameSize:%d name '%s'\n",
            featCdMaxNameSize, nameSize, featCdName);

    return featCdName;
}




/* @funcstatic featCdTrgLine **************************************************
**
** Reads a line from an EMBL CD-ROM index target file.
**
** @param [w] trgLine [FeatPCdTrg] Target file record.
** @param [r] ipos [ajuint] Record number.
** @param [u] fil [FeatPCdFile] EMBL CD-ROM index target file.
** @return [void].
**
** @release 6.5.0
** @@
******************************************************************************/

static void featCdTrgLine(FeatPCdTrg trgLine, ajuint ipos, FeatPCdFile fil)
{
    ajuint nameSize;

    nameSize = fil->RecSize-8;

    if(featCdMaxNameSize < nameSize)
    {
        featCdMaxNameSize = nameSize;

        if(featCdName)
            ajCharDel(&featCdName);

        featCdName = ajCharNewRes(featCdMaxNameSize+1);
    }

    featCdFileSeek(fil, ipos);

    ajReadbinUint(fil->File, &trgLine->NHits);
    ajReadbinUint(fil->File, &trgLine->FirstHit);
    ajReadbinCharTrim(fil->File, nameSize, featCdName);

    ajStrAssignC(&trgLine->Target,featCdName);

    ajDebug("featCdTrgLine %d nHits %d firstHit %d target '%S'\n",
            ipos, trgLine->NHits, trgLine->FirstHit, trgLine->Target);

    return;
}




/* @funcstatic featCdReadHeader ***********************************************
**
** Reads the header of an EMBL CD-ROM index file.
**
** @param [u] fil [FeatPCdFile] EMBL CD-ROM index file.
** @return [AjBool] ajTrue on success.
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool featCdReadHeader(FeatPCdFile fil)
{
    ajint i;

    FeatPCdFHeader header;
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

    ajDebug("featCdReadHeader file %F\n", fil->File);
    ajDebug("  FileSize: %d NRecords: %hd recsize: %d idsize: %d\n",
            header->FileSize, header->NRecords,
            header->RecSize, header->IdSize);

    return ajTrue;
}




/* @funcstatic featCdTrgOpen **************************************************
**
** Opens an EMBL CD-ROM target file pair.
**
** @param [r] dir [const AjPStr] Directory.
** @param [r] name [const char*] File name.
** @param [w] trgfil [FeatPCdFile*] Target file.
** @param [w] hitfil [FeatPCdFile*] Hit file.
** @return [AjBool] ajTrue on success.
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool featCdTrgOpen(const AjPStr dir, const char* name,
                            FeatPCdFile* trgfil, FeatPCdFile* hitfil)
{
    AjPStr tmpname  = NULL;
    AjPStr fullname = NULL;

    ajDebug("featCdTrgOpen dir '%S' name '%s'\n",
            dir, name);

    ajFmtPrintS(&tmpname, "%s.trg",name);
    *trgfil = featCdFileOpen(dir, ajStrGetPtr(tmpname), &fullname);
    ajStrDel(&tmpname);

    if(!*trgfil)
        return ajFalse;

    ajFmtPrintS(&tmpname, "%s.hit",name);
    *hitfil = featCdFileOpen(dir, ajStrGetPtr(tmpname), &fullname);
    ajStrDel(&tmpname);
    ajStrDel(&fullname);

    if(!*hitfil)
        return ajFalse;

    return ajTrue;
}




/* @funcstatic featCdTrgClose *************************************************
**
** Close an EMBL CD-ROM target file pair.
**
** @param [w] ptrgfil [FeatPCdFile*] Target file.
** @param [w] phitfil [FeatPCdFile*] Hit file.
** @return [AjBool] ajTrue on success.
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool featCdTrgClose(FeatPCdFile* ptrgfil, FeatPCdFile* phitfil)
{
    featCdFileClose(ptrgfil);
    featCdFileClose(phitfil);

    return ajTrue;
}




/* @section GCG Database Indexing *********************************************
**
** These functions manage the GCG index access methods.
**
******************************************************************************/




/* @funcstatic featAccessGcg ***************************************************
**
** Reads feature(s) from a GCG formatted database, using EMBLCD index
** files. Returns with the file pointer set to the position in the
** sequence file and reference files.
**
** @param [u] fttabin [AjPFeattabin] Feature table input.
** @return [AjBool] ajTrue on success.
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool featAccessGcg(AjPFeattabin fttabin)
{
    AjBool retval = ajFalse;
    AjPQuery qry;
    FeatPCdQry qryd;

    ajDebug("featAccessGcg type %d\n", fttabin->Input->Query->QueryType);

    qry  = fttabin->Input->Query;
    qryd = qry->QryData;

    if(qry->QueryType == AJQUERY_ALL)
    {
        retval = featGcgAll(fttabin);

        return retval;
    }

    /* we need to search the index files and return a query */

    if(qry->QryData)
    {                                /* reuse unfinished query data */
        if(!featCdQryReuse(qry))
            return ajFalse;
    }
    else
    {
        fttabin->Input->Single = ajTrue;

        if(!featCdQryOpen(qry))
        {
            ajWarn("Failed to open index for database '%S'",
                   qry->DbName);

            return ajFalse;
        }

        qryd = qry->QryData;
        ajFilebuffDel(&fttabin->Input->Filebuff);
        fttabin->Input->Filebuff = ajFilebuffNewNofile();

        /* binary search for the entryname we need */

        if(qry->QueryType == AJQUERY_ENTRY)
        {
            ajDebug("entry fields: %Lu hasacc:%B\n",
                    ajListGetLength(qry->QueryFields), qry->HasAcc);

            if(!featCdQryEntry(qry))
                ajDebug("GCG Entry failed\n");
        }

        if(qry->QueryType == AJQUERY_QUERY)
        {
            ajDebug("query fields: %Lu hasacc:%B\n",
                    ajListGetLength(qry->QueryFields), qry->HasAcc);
            if(!featCdQryQuery(qry))
                ajDebug("GCG Query failed\n");
        }

        AJFREE(qryd->trgLine);
    }

    if(ajListGetLength(qry->ResultsList))
    {
        retval = featCdQryNext(qry);

        if(retval)
            featGcgLoadBuff(fttabin);
    }

    if(!ajListGetLength(qry->ResultsList))
    {
        ajFileClose(&qryd->libr);
        ajFileClose(&qryd->libs);
        featCdQryClose(qry);
    }

    if(retval)
        ajStrAssignS(&fttabin->Input->Db, qry->DbName);

    return retval;
}




/* @funcstatic featGcgLoadBuff ************************************************
**
** Copies text data to a buffered file, and feature data for an
** AjPFeattabin internal data structure for reading later
**
** @param [u] fttabin [AjPFeattabin] Feature table input object
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void featGcgLoadBuff(AjPFeattabin fttabin)
{
    AjPQuery qry;
    FeatPCdQry qryd;

    qry  = fttabin->Input->Query;
    qryd = qry->QryData;

    if(!qry->QryData)
        ajFatal("featGcgLoadBuff Query Data not initialised");

    /* copy all the ref data */

    featGcgReadRef(fttabin);

    /* skip the sequence (do we care about the format?) */
    featGcgReadSeq(fttabin);

    /* ajFilebuffTraceFull(fttabin->Input->Filebuff, 9999, 100); */

    if(!qryd->libr)
    {
        ajFileClose(&qryd->libs);
        ajDebug("featGcgLoadBuff: closed files\n");
    }

    return;
}




/* @funcstatic featGcgReadRef *************************************************
**
** Copies text data to a buffered file for reading later
**
** @param [u] fttabin [AjPFeattabin] Feature table input object
** @return [AjBool] ajTrue on success
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool featGcgReadRef(AjPFeattabin fttabin)
{
    AjPStr line = NULL;
    AjPQuery qry;
    FeatPCdQry qryd;
    ajlong rpos;
    AjPStr id       = NULL;
    AjPStr idc      = NULL;
    AjBool ispir           = ajFalse;
    AjBool continued       = ajFalse;
    AjBool testcontinue    = ajFalse;
    char *p = NULL;

    qry  = fttabin->Input->Query;
    qryd = qry->QryData;

    if(!featRegGcgRefId)
        featRegGcgRefId =ajRegCompC("^>...([^ \n]+)");

    if(!featRegGcgSplit)
        featRegGcgSplit =ajRegCompC("_0+$");

    if(!ajReadline(qryd->libr, &line))  /* end of file */
        return ajFalse;

    if(ajStrGetCharFirst(line) != '>')  /* not start of entry */
        ajFatal("featGcgReadRef bad entry start:\n'%S'", line);

    if(ajStrGetCharPos(line, 3) == ';') /* PIR entry */
        ispir = ajTrue;

    if(ispir)
        ajFilebuffLoadS(fttabin->Input->Filebuff, line);

    if(ajRegExec(featRegGcgRefId, line))
    {
        continued = ajFalse;
        ajRegSubI(featRegGcgRefId, 1, &id);

        if(ajRegExec(featRegGcgSplit, id))
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
        ajDebug("featGcgReadRef bad ID line\n'%S'\n", line);
        ajFatal("featGcgReadRef bad ID line\n'%S'\n", line);
    }

    if(!ajReadline(qryd->libr, &line))  /* blank desc line */
    {
        ajStrDel(&id);

        return ajFalse;
    }

    if(ispir)
        ajFilebuffLoadS(fttabin->Input->Filebuff, line);

    rpos = ajFileResetPos(qryd->libr);

    while(ajReadline(qryd->libr, &line))
    {                                   /* end of file */
        if(ajStrGetCharFirst(line) == '>')
        {                               /* start of next entry */
            /* skip over split entries so it can be used for "all" */

            if(continued)
            {
                testcontinue=ajTrue;
                ajRegExec(featRegGcgRefId, line);
                ajRegSubI(featRegGcgRefId, 1, &idc);

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

        rpos = ajFileResetPos(qryd->libr);

        if(!testcontinue)
        {
            ajStrExchangeCC(&line, ". .", "..");
            ajFilebuffLoadS(fttabin->Input->Filebuff, line);
        }
    }


    /* at end of file */

    ajFileClose(&qryd->libr);

    ajStrDel(&line);
    ajStrDel(&id);
    ajStrDel(&idc);

    return ajTrue;
}




/* @funcstatic featGcgReadSeq *************************************************
**
** Skips unwanted sequence data so file is at start of next entry.
**
** @param [u] fttabin [AjPFeattabin] Feature table input object
** @return [AjBool] ajTrue on success
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool featGcgReadSeq(AjPFeattabin fttabin)
{
    AjPStr line = NULL;
    AjPQuery qry;
    FeatPCdQry qryd;
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

    qry  = fttabin->Input->Query;
    qryd = qry->QryData;

    if(!featRegGcgId)
    {
        featRegGcgId =ajRegCompC("^>...([^ ]+) +([^ ]+) +(Dummy Header|[^ ]+)"
                                " +([^ ]+) +([0-9]+)");
        featRegGcgId2=ajRegCompC("^>[PF]1;([^ ]+)");
    }

    if(!featRegGcgSplit)
        featRegGcgSplit =ajRegCompC("_0+$");

    ajDebug("featGcgReadSeq pos: %Ld\n", ajFileResetPos(qryd->libs));

    if(!ajReadline(qryd->libs, &line))  /* end of file */
        return ajFalse;

    ajDebug("test ID line\n'%S'\n", line);

    if(ajRegExec(featRegGcgId, line))
    {
        continued = ajFalse;
        ajRegSubI(featRegGcgId, 3, &gcgtype);
        ajRegSubI(featRegGcgId, 5, &tmpstr);
        ajRegSubI(featRegGcgId, 1, &id);

        if(ajRegExec(featRegGcgSplit, id))
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
    else if(ajRegExec(featRegGcgId2, line))
    {
        ajStrAssignC(&gcgtype, "ASCII");
        ajRegSubI(featRegGcgId, 1, &tmpstr);
        ispir = ajTrue;
    }
    else
    {
        ajDebug("featGcgReadSeq bad ID line\n'%S'\n", line);
        ajFatal("featGcgReadSeq bad ID line\n'%S'\n", line);

        return ajFalse;
    }

    if(!ajReadline(qryd->libs, &line))  /* desc line */
        return ajFalse;

    /*
    ** need to pick up the length and type, and read to the end of sequence
    ** see fasta code to get a real sequence for this
    ** Also need to handle split entries and go find the rest
    */

    if(ispir)
    {
        spos = ajFileResetPos(qryd->libs);

        while(ajReadline(qryd->libs, &line))
        {                               /* end of file */
            if(ajStrGetCharFirst(line) == '>')
            {                           /* start of next entry */
                ajFileSeek(qryd->libs, spos, 0);
                break;
            }

            spos = ajFileResetPos(qryd->libs);
            ajFilebuffLoadS(fttabin->Input->Filebuff, line);
        }
    }
    else
    {
        ajStrSetRes(&contseq, gcglen+3);
        rblock = gcglen;

        if(ajStrGetCharFirst(gcgtype) == '2')
            rblock = (rblock+3)/4;

        if(!ajReadbinBinary(qryd->libs, rblock, 1,
                            ajStrGetuniquePtr(&contseq)))
            ajFatal("error reading file %F", qryd->libs);
        
        /* convert 2bit to ascii */
        if(ajStrGetCharFirst(gcgtype) == '2')
            featGcgBinDecode(&contseq, gcglen);
        else if(ajStrGetCharFirst(gcgtype) == 'A')
        {
            /* are seq chars OK? */
            ajStrSetValidLen(&contseq, gcglen);
        }
        else
        {
            ajRegSubI(featRegGcgId, 1, &tmpstr);
            ajFatal("Unknown GCG entry type '%S', entry name '%S'",
                    gcgtype, tmpstr);
        }

        if(!ajReadline(qryd->libs, &line)) /* newline at end */
            ajFatal("error reading file %F", qryd->libs);
        ajStrDel(&contseq);

        if(continued)
        {
            spos = ajFileResetPos(qryd->libs);

            while(ajReadline(qryd->libs,&line))
            {
                ajRegExec(featRegGcgId, line);
                ajRegSubI(featRegGcgId, 5, &tmpstr);
                ajRegSubI(featRegGcgId, 1, &idc);

                if(!ajStrPrefixS(idc,id))
                {
                    ajFileSeek(qryd->libs, spos, 0);
                    break;
                }

                ajStrToInt(tmpstr, &gcglen);

                if(!ajReadline(qryd->libs, &dstr)) /* desc line */
                    return ajFalse;

                ajStrSetRes(&contseq, gcglen+3);

                rblock = gcglen;
                if(ajStrGetCharFirst(gcgtype) == '2')
                    rblock = (rblock+3)/4;

                if(!ajReadbinBinary(qryd->libs, rblock, 1,
                                    ajStrGetuniquePtr(&contseq)))
                    ajFatal("error reading file %F", qryd->libs);

                /* convert 2bit to ascii */
                if(ajStrGetCharFirst(gcgtype) == '2')
                    featGcgBinDecode(&contseq, gcglen);
                else if(ajStrGetCharFirst(gcgtype) == 'A')
                {
                    /* are seq chars OK? */
                    ajStrSetValidLen(&contseq, gcglen);
                }
                else
                {
                    ajRegSubI(featRegGcgId, 1, &tmpstr);
                    ajFatal("Unknown GCG entry: name '%S'",
                            tmpstr);
                }

                if(!ajReadline(qryd->libs, &line)) /* newline at end */
                    ajFatal("error reading file %F", qryd->libs);

                if(!featRegGcgCont)
                    featRegGcgCont = ajRegCompC("^([^ ]+) +([^ ]+) +([^ ]+) +"
                                               "([^ ]+) +([^ ]+) +([^ ]+) "
                                               "+([^ ]+) +"
                                               "([^ ]+) +([0-9]+)");

                ajRegExec(featRegGcgCont, dstr);
                ajRegSubI(featRegGcgCont, 9, &tmpstr);
                ajStrToInt(tmpstr, &pos);
                /*seqin->Inseq->Len = pos-1;*/

                /*ajStrAppendS(&seqin->Inseq,contseq);*/
                spos = ajFileResetPos(qryd->libs);
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




/* @funcstatic featGcgBinDecode ***********************************************
**
** Convert GCG binary to ASCII sequence.
**
** @param [u] pthis [AjPStr*] Binary string
** @param [r] sqlen [ajuint] Expected sequence length
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void featGcgBinDecode(AjPStr *pthis, ajuint sqlen)
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




/* @funcstatic featGcgAll *****************************************************
**
** Opens the first or next GCG file for further reading
**
** @param [u] fttabin [AjPFeattabin] Feature table input.
** @return [AjBool] ajTrue on success.
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool featGcgAll(AjPFeattabin fttabin)
{
    AjPQuery qry;
    FeatPCdQry qryd;

    qry = fttabin->Input->Query;
    qryd = qry->QryData;

    ajDebug("featGcgAll\n");

    if(!qry->QryData)
    {
        ajDebug("featGcgAll initialising\n");
        fttabin->Input->Single = ajTrue;

        if(!featCdQryOpen(qry))
        {
            ajErr("featGcgAll failed");

            return ajFalse;
        }
    }

    qryd = qry->QryData;
    ajFilebuffDel(&fttabin->Input->Filebuff);
    fttabin->Input->Filebuff = ajFilebuffNewNofile();

    if(!qryd->libr)
    {
        if(!featCdDivNext(qry))
        {
            featCdQryClose(qry);
            ajDebug("featGcgAll finished\n");

            return ajFalse;
        }

        if(!featCdQryFile(qry))
        {
            ajErr("featGcgAll out of data");

            return ajFalse;
        }

        ajDebug("featCdQryOpen processing file %2d '%F'\n", qryd->div,
                qryd->libr);
        if(qryd->libs)
            ajDebug("               sequence file    '%F'\n", qryd->libs);
    }

    featGcgLoadBuff(fttabin);

    if(!qry->CaseId)
        qry->QryDone = ajTrue;

    return ajTrue;
}




/* @funcstatic featCdDivNext **************************************************
**
** Sets the division count to the next included file. We need the division
** file to be already open.
**
** @param [u] qry [AjPQuery] query object.
** @return [ajuint] File number (starting at 1) or zero if all files are done.
**
** @release 6.5.0
** @@
******************************************************************************/

static ajuint featCdDivNext(AjPQuery qry)
{
    FeatPCdQry qryd;
    AjPStr fullName = NULL;
    ajuint i;

    qryd = qry->QryData;

    ajDebug("featCdDivNext div: %d dfp: %x nameSize: %d name '%s'\n",
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




/* @funcstatic featCdQryFile **************************************************
**
** Opens a specific file number for an EMBLCD index
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue on success
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool featCdQryFile(AjPQuery qry)
{
    FeatPCdQry qryd;
    short j;

    if(!featCdDivExp)
        featCdDivExp = ajRegCompC("^([^ ]+)( +([^ ]+))?");

    ajDebug("featCdQryFile qry %x\n",qry);
    qryd = qry->QryData;
    ajDebug("featCdQryFile qryd %x\n",qryd);
    ajDebug("featCdQryFile %F\n",qryd->dfp->File);

    featCdFileSeek(qryd->dfp, (qryd->div - 1));

    /* note - we must not use featCdFileReadName - we need spaces for GCG */

    ajReadbinInt2(qryd->dfp->File, &j);

    ajReadbinChar(qryd->dfp->File, qryd->nameSize, qryd->name);
    ajDebug("DivCode: %d, code: %2hd '%s'\n",
            qryd->div, j, qryd->name);

    /**ajCharFmtLower(qryd->name);**/
    if(!ajRegExecC(featCdDivExp, qryd->name))
    {
        ajErr("index division file error '%S'", qryd->name);

        return ajFalse;
    }

    ajRegSubI(featCdDivExp, 1, &qryd->datfile);
    ajRegSubI(featCdDivExp, 3, &qryd->seqfile);
    ajDebug("File(s) '%S' '%S'\n", qryd->datfile, qryd->seqfile);

    ajFileClose(&qryd->libr);
    qryd->libr = ajFileNewInNamePathS(qryd->datfile, qry->Directory);

    if(!qryd->libr)
    {
        ajErr("Cannot open database file '%S' for database '%S'",
	      qryd->datfile, qry->DbName);

        return ajFalse;
    }

    if(ajStrGetLen(qryd->seqfile))
    {
        ajFileClose(&qryd->libs);
        qryd->libs = ajFileNewInNamePathS(qryd->seqfile, qry->Directory);

        if(!qryd->libs)
        {
            ajErr("Cannot open sequence file '%S' for database '%S'",
		  qryd->seqfile, qry->DbName);

            return ajFalse;
        }
    }
    else
        qryd->libs = NULL;

    return ajTrue;
}




/* @funcstatic featCdTrgQuery *************************************************
**
** Binary search of an EMBL CD-ROM index file for entries matching a
** wildcard query.
**
** Where more than one query field is defined (usually acc and sv) it
** can test all and append to a single list.
**
** @param [u] qry [AjPQuery] Query object.
** @param [r] field [const AjPStr] Query field
** @param [r] wildqry [const AjPStr] Query string
** @return [AjBool] ajTrue on success.
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool featCdTrgQuery(AjPQuery qry, const AjPStr field,
                             const AjPStr wildqry)
{
    ajint ret=0;

    if(ajStrMatchC(field, "org"))
        ret += featCdTrgFind(qry, "taxon", wildqry);

    if(ajStrMatchC(field, "key"))
        ret += featCdTrgFind(qry, "keyword", wildqry);

    if(ajStrMatchC(field, "des"))
        ret += featCdTrgFind(qry, "des", wildqry);

    if(ajStrMatchC(field, "sv"))
        ret += featCdTrgFind(qry, "seqvn", wildqry);

    if(ajStrMatchC(field, "gi"))
        ret += featCdTrgFind(qry, "gi", wildqry);

    if(qry->HasAcc && ajStrMatchC(field, "acc"))
        ret += featCdTrgFind(qry, "acnum", wildqry);


    if(ret)
        return ajTrue;

    return ajFalse;
}




/* @funcstatic featCdTrgFind **************************************************
**
** Binary search of an EMBL CD-ROM index file for entries matching a
** wildcard query.
**
** Where more than one query field is defined (usually acc and sv) it
** can test all and append to a single list.
**
** @param [u] qry [AjPQuery] Query object.
** @param [r] indexname [const char*] Index name.
** @param [r] queryName [const AjPStr] Query string.
** @return [ajuint] Number of matches found
**
** @release 6.5.0
** @@
******************************************************************************/

static ajuint featCdTrgFind(AjPQuery qry, const char* indexname,
                            const AjPStr queryName)
{
    FeatPCdQry wild;
    AjPList   l;
    FeatPCdTrg trgline;
    FeatPCdIdx idxline;
    FeatPCdFile idxfp;
    FeatPCdFile trgfp;
    FeatPCdFile hitfp;
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

    FeatPCdEntry entry;


    wild    = qry->QryData;
    l       = qry->ResultsList;
    trgline = wild->trgLine;
    idxline = wild->idxLine;
    idxfp   = wild->ifp;
    trgfp   = wild->trgfp;
    hitfp   = wild->hitfp;
    skip    = wild->Skip;


    if(!featCdTrgOpen(qry->IndexDir, indexname, &trgfp, &hitfp))
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
            name = featCdTrgName(pos,trgfp);
            name[prefixlen]='\0';      /* truncate to prefix length */
            cmp = ajStrCmpC(fdprefix,name);
            /*      match = ajStrMatchWildC(fdstr,name);*/
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
            featCdTrgClose(&trgfp,&hitfp);

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
            name = featCdTrgName(pos,trgfp);
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
        name = featCdTrgName(b2,trgfp);
        ajDebug("first %d '%s'\n",b2,name);
        name = featCdTrgName(t3,trgfp);
        ajDebug("last %d '%s'\n",t3,name);
    }


    start = b2;
    end   = t3;

    for(i=start;i<(end+1);++i)
    {
        name = featCdTrgName(i,trgfp);
        match = ajCharMatchWildC(name, ajStrGetPtr(fdstr));

        ajDebug("third pass: match:%B i:%d name '%s' queryName '%S'\n",
                match, i, name, fdstr);

        if(!match)
            continue;

        featCdTrgLine(trgline, i, trgfp);
        featCdFileSeek(hitfp,trgline->FirstHit-1);
        ajDebug("Query First: %d Count: %d\n",
                trgline->FirstHit, trgline->NHits);
        pos = trgline->FirstHit;

        for(j=0;j<(ajint)trgline->NHits;++j)
        {
            ajReadbinInt(hitfp->File, &k);
            --k;
            ajDebug("hitlist[%d] entry = %d\n",j,k);
            featCdIdxLine(idxline,k,idxfp);

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

    featCdTrgClose(&trgfp, &hitfp);


    ajStrDel(&trgline->Target);
    ajStrDel(&fdstr);
    ajStrDel(&fdprefix);

    return (ajuint) ajListGetLength(l);
}




/* @funcstatic featCdIdxDel ***************************************************
**
** Destructor for FeatPCdIdx
**
** @param [d] pthys [FeatPCdIdx*] Cd index object
** @return [void]
**
** @release 6.5.0
******************************************************************************/

static void featCdIdxDel(FeatPCdIdx* pthys)
{
    FeatPCdIdx thys = *pthys;

    if(!thys)
        return;

    ajStrDel(&thys->EntryName);
    AJFREE(*pthys);

    return;
}




/* @funcstatic featCdTrgDel ***************************************************
**
** Destructor for FeatPCdTrg
**
** @param [d] pthys [FeatPCdTrg*] Cd index target object
** @return [void]
**
** @release 6.5.0
**
******************************************************************************/

static void featCdTrgDel(FeatPCdTrg* pthys)
{
    FeatPCdTrg thys = *pthys;

    if(!thys)
        return;

    ajStrDel(&thys->Target);
    AJFREE(*pthys);

    return;
}




/* @section B+tree GCG Database Indexing *************************************
**
** These functions manage the EMBOSS B+tree GCG index access methods.
**
******************************************************************************/




/* @funcstatic featAccessEmbossGcg ********************************************
**
** Reads feature(s) from a GCG formatted database, using B+tree index
** files. Returns with the file pointer set to the position in the
** reference and sequence file.
**
** @param [u] fttabin [AjPFeattabin] Feature table input.
** @return [AjBool] ajTrue on success.
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool featAccessEmbossGcg(AjPFeattabin fttabin)
{
    AjBool retval = ajFalse;

    AjPQuery qry;
    FeatPEmbossQry qryd = NULL;


    qry = fttabin->Input->Query;
    qryd = qry->QryData;
    ajDebug("featAccessEmbossGcg type %d\n", qry->QueryType);

    if(!ajNamDbGetDbalias(qry->DbName, &qry->DbAlias))
        ajStrAssignS(&qry->DbAlias, qry->DbName);

    if(qry->QueryType == AJQUERY_ALL)
        return featEmbossGcgAll(fttabin);


    if(!qry->QryData)
    {
        if(!featEmbossQryOpen(qry))
            return ajFalse;

        qryd = qry->QryData;
        fttabin->Input->Single = ajTrue;
        ajFilebuffDel(&fttabin->Input->Filebuff);
        fttabin->Input->Filebuff = ajFilebuffNewNofile();

        if(qry->QueryType == AJQUERY_ENTRY)
        {
            if(!featEmbossQryEntry(qry))
                ajDebug("embossgcg B+tree Entry failed\n");
        }

        if(qry->QueryType == AJQUERY_QUERY)
        {
            if(!featEmbossQryQuery(qry))
                ajDebug("embossgcg B+tree Query failed\n");
        }
    }
    else
    {
        if(!featEmbossQryReuse(qry))
        {
            featEmbossQryClose(qry);
            return ajFalse;
        }
        ajFilebuffClear(fttabin->Input->Filebuff, -1);
    }

    if(ajListGetLength(qry->ResultsList))
    {
        retval = featEmbossQryNext(qry);

        if(retval)
        {
            featEmbossGcgLoadBuff(fttabin);
            ajStrAssignS(&fttabin->Input->Db, qry->DbName);
        }
    }

    if(!ajListGetLength(qry->ResultsList)) /* could be emptied by code above */
    {
        featEmbossQryClose(qry);
        ajFileClose(&qryd->libs);
        ajFileClose(&qryd->libr);
    }

    return retval;
}




/* @funcstatic featEmbossGcgAll ***********************************************
**
** Opens the first or next GCG file for further reading
**
** @param [u] fttabin [AjPFeattabin] Feature table input.
** @return [AjBool] ajTrue on success.
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool featEmbossGcgAll(AjPFeattabin fttabin)
{
    AjPQuery qry;
    FeatPEmbossQry qryd;
    static ajint i   = 0;
    ajuint iref;
    AjPStr name      = NULL;
    AjBool ok        = ajFalse;
/*
  AjPStrTok handle = NULL;
  AjPStr wildname  = NULL;
  AjBool found     = ajFalse;
*/

    qry = fttabin->Input->Query;
    qryd = qry->QryData;

    ajDebug("featEmbossGcgAll\n");


    if(!qry->QryData)
    {
        ajDebug("featEmbossGcgAll initialising\n");

        qry->QryData = AJNEW0(qryd);
        qryd = qry->QryData;
        i = -1;
        ajBtreeReadEntriesS(qry->DbAlias,qry->IndexDir,
                            qry->Directory,
                            &qryd->files,
                            &qryd->reffiles,
                            &qryd->refcount);

        fttabin->Input->Single = ajTrue;
    }

    qryd = qry->QryData;
    ajFilebuffDel(&fttabin->Input->Filebuff);
    fttabin->Input->Filebuff = ajFilebuffNewNofile();

    if(!qryd->libs)
    {
        while(!ok && qryd->files[++i])
        {
            ajStrAssignS(&name,qryd->files[i]);
            if(ajFilenameTestInclude(name, qry->Exclude, qry->Filename))
                ok = ajTrue;
        }

        ajStrDel(&name);

/*      if(qry->Exclude)
        {
        ok = ajFalse;
        wildname = ajStrNew();
        name     = ajStrNew();
        while(!ok)
        {
        ajStrAssignS(&name,qryd->files[i]);
        ajFilenameTrimPath(&name);
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
            ajDebug("featEmbossGcgAll finished\n");
            i=0;

            while(qryd->files[i])
            {
                ajStrDel(&qryd->files[i]);

                if(qryd->reffiles)
                {
                    for(iref=0; iref < qryd->refcount; iref++)
                        ajStrDel(&qryd->reffiles[iref][i]);
                }

                ++i;
            }

            AJFREE(qryd->files);
            AJFREE(qryd->reffiles);

            AJFREE(qry->QryData);
            qry->QryData = NULL;

            return ajFalse;
        }


        qryd->libs = ajFileNewInNameS(qryd->files[i]);

        if(!qryd->libs)
        {
            ajDebug("featEmbossGcgAll: cannot open sequence file\n");

            return ajFalse;
        }


        if(qryd->reffiles)
            qryd->libr = ajFileNewInNameS(qryd->reffiles[0][i]);

        if(!qryd->libr)
        {
            ajDebug("featEmbossGcgAll: cannot open reference file\n");

            return ajFalse;
        }
    }

    featEmbossGcgLoadBuff(fttabin);

    if(!qry->CaseId)
        qry->QryDone = ajTrue;

    return ajTrue;
}




/* @funcstatic featEmbossGcgLoadBuff ******************************************
**
** Copies text data to a buffered file, and skips sequence data.
**
** @param [u] fttabin [AjPFeattabin] Feature table input object
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void featEmbossGcgLoadBuff(AjPFeattabin fttabin)
{
    AjPQuery qry;
    FeatPEmbossQry qryd;

    qry  = fttabin->Input->Query;
    qryd = qry->QryData;

    ajDebug("featEmbossGcgLoadBuff\n");

    if(!qry->QryData)
        ajFatal("featEmbossGcgLoadBuff Query Data not initialised");

    /* copy all the ref data */

    featEmbossGcgReadRef(fttabin);

    /* skip the sequence (do we care about the format?) */
    featEmbossGcgReadSeq(fttabin);

    /* ajFilebuffTraceFull(fttabin->Input->Filebuff, 9999, 100); */

    if(!qryd->libr)
        ajFileClose(&qryd->libs);

    return;
}




/* @funcstatic featEmbossGcgReadRef *******************************************
**
** Copies text data to a buffered file for reading later
**
** @param [u] fttabin [AjPFeattabin] Feature table input object
** @return [AjBool] ajTrue on success
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool featEmbossGcgReadRef(AjPFeattabin fttabin)
{
    AjPStr line = NULL;
    AjPQuery qry;
    FeatPEmbossQry qryd;
    ajlong rpos;
    AjPStr id       = NULL;
    AjPStr idc      = NULL;
    AjBool ispir           = ajFalse;
    AjBool continued       = ajFalse;
    AjBool testcontinue    = ajFalse;
    char *p = NULL;

    qry  = fttabin->Input->Query;
    qryd = qry->QryData;

    if(!featRegGcgRefId)
        featRegGcgRefId =ajRegCompC("^>...([^ \n]+)");

    if(!featRegGcgSplit)
        featRegGcgSplit =ajRegCompC("_0+$");

    if(!ajReadline(qryd->libr, &line))  /* end of file */
        return ajFalse;

    if(ajStrGetCharFirst(line) != '>')  /* not start of entry */
        ajFatal("featGcgReadRef bad entry start:\n'%S'", line);

    if(ajStrGetCharPos(line, 3) == ';') /* PIR entry */
        ispir = ajTrue;

    if(ispir)
        ajFilebuffLoadS(fttabin->Input->Filebuff, line);


    if(ajRegExec(featRegGcgRefId, line))
    {
        continued = ajFalse;
        ajRegSubI(featRegGcgRefId, 1, &id);

        if(ajRegExec(featRegGcgSplit,id))
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
        ajDebug("featEmbossGcgReadRef bad ID line\n'%S'\n", line);
        ajFatal("featEmbossGcgReadRef bad ID line\n'%S'\n", line);
    }



    if(!ajReadline(qryd->libr, &line))  /* blank desc line */

        return ajFalse;

    if(ispir)
        ajFilebuffLoadS(fttabin->Input->Filebuff, line);


    rpos = ajFileResetPos(qryd->libr);

    while(ajReadline(qryd->libr, &line))
    {
        /* end of file */
        if(ajStrGetCharFirst(line) == '>')
        {                               /* start of next entry */
            /* skip over split entries so it can be used for "all" */

            if(continued)
            {
                testcontinue=ajTrue;
                ajRegExec(featRegGcgRefId, line);
                ajRegSubI(featRegGcgRefId, 1, &idc);

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
        rpos = ajFileResetPos(qryd->libr);


        if(!testcontinue)
        {
            ajStrExchangeCC(&line, ". .", "..");
            ajFilebuffLoadS(fttabin->Input->Filebuff, line);
        }
    }


    /* at end of file */

    ajFileClose(&qryd->libr);
    ajStrDel(&line);
    ajStrDel(&id);
    ajStrDel(&idc);

    return ajTrue;
}




/* @funcstatic featEmbossGcgReadSeq *******************************************
**
** Skips sequence data.
**
** @param [u] fttabin [AjPFeattabin] feature table input object
** @return [AjBool] ajTrue on success
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool featEmbossGcgReadSeq(AjPFeattabin fttabin)
{
    AjPStr line = NULL;
    AjPQuery qry;
    FeatPEmbossQry qryd;
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


    qry  = fttabin->Input->Query;
    qryd = qry->QryData;

    if(!featRegGcgId)
    {
        featRegGcgId =ajRegCompC("^>...([^ ]+) +(Dummy Header|[^ ]+) +"
                                 "([^ ]+) +([^ ]+) +([0-9]+)");
        featRegGcgId2=ajRegCompC("^>[PF]1;([^ ]+)");
    }

    if(!featRegGcgSplit)
        featRegGcgSplit =ajRegCompC("_0+$");

    ajDebug("featEmbossGcgReadSeq pos: %Ld\n", ajFileResetPos(qryd->libs));

    if(!ajReadline(qryd->libs, &line))  /* end of file */
        return ajFalse;

    ajDebug("test ID line\n'%S'\n", line);

    if(ajRegExec(featRegGcgId, line))
    {
        continued = ajFalse;
        ajRegSubI(featRegGcgId, 3, &gcgtype);
        ajRegSubI(featRegGcgId, 5, &tmpstr);
        ajRegSubI(featRegGcgId, 1, &id);

        if(ajRegExec(featRegGcgSplit, id))
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
    else if(ajRegExec(featRegGcgId2, line))
    {
        ajStrAssignC(&gcgtype, "ASCII");
        ajRegSubI(featRegGcgId, 1, &tmpstr);
        ispir = ajTrue;
    }
    else
    {
        ajDebug("featEmbossGcgReadSeq bad ID line\n'%S'\n", line);
        ajFatal("featEmbossGcgReadSeq bad ID line\n'%S'\n", line);

        return ajFalse;
    }

    if(!ajReadline(qryd->libs, &line))  /* desc line */
        return ajFalse;

    /*
    ** need to pick up the length and type, and read to the end of sequence
    ** see fasta code to get a real sequence for this
    ** Also need to handle split entries and go find the rest
    */

    if(ispir)
    {
        spos = ajFileResetPos(qryd->libs);

        while(ajReadline(qryd->libs, &line))
        {                               /* end of file */
            if(ajStrGetCharFirst(line) == '>')
            {                           /* start of next entry */
                ajFileSeek(qryd->libs, spos, 0);
                break;
            }

            spos = ajFileResetPos(qryd->libs);
            ajFilebuffLoadS(fttabin->Input->Filebuff, line);
        }
    }
    else
    {
        ajStrSetRes(&contseq, gcglen+3);
        rblock = gcglen;

        if(ajStrGetCharFirst(gcgtype) == '2')
            rblock = (rblock+3)/4;

        if(!ajReadbinBinary(qryd->libs, rblock, 1,
                            ajStrGetuniquePtr(&contseq)))
            ajFatal("error reading file %F", qryd->libs);

        /* convert 2bit to ascii */
        if(ajStrGetCharFirst(gcgtype) == '2')
            featGcgBinDecode(&contseq, gcglen);
        else if(ajStrGetCharFirst(gcgtype) == 'A')
        {
            /* are seq chars OK? */
            ajStrSetValidLen(&contseq, gcglen);
        }
        else
        {
            ajRegSubI(featRegGcgId, 1, &tmpstr);
            ajFatal("Unknown GCG entry type '%S', entry name '%S'",
                    gcgtype, tmpstr);
        }

        if(!ajReadline(qryd->libs, &line)) /* newline at end */
            ajFatal("error reading file %F", qryd->libs);

        if(continued)
        {
            spos = ajFileResetPos(qryd->libs);

            while(ajReadline(qryd->libs,&line))
            {
                ajRegExec(featRegGcgId, line);
                ajRegSubI(featRegGcgId, 5, &tmpstr);
                ajRegSubI(featRegGcgId, 1, &idc);

                if(!ajStrPrefixS(idc,id))
                {
                    ajFileSeek(qryd->libs, spos, 0);
                    break;
                }

                ajStrToInt(tmpstr, &gcglen);

                if(!ajReadline(qryd->libs, &dstr)) /* desc line */
                    return ajFalse;

                ajStrSetRes(&contseq, gcglen+3);

                rblock = gcglen;
                if(ajStrGetCharFirst(gcgtype) == '2')
                    rblock = (rblock+3)/4;

                if(!ajReadbinBinary(qryd->libs, rblock, 1,
                                    ajStrGetuniquePtr(&contseq)))
                    ajFatal("error reading file %F", qryd->libs);

                /* convert 2bit to ascii */
                if(ajStrGetCharFirst(gcgtype) == '2')
                    featGcgBinDecode(&contseq, gcglen);
                else if(ajStrGetCharFirst(gcgtype) == 'A')
                {
                    /* are seq chars OK? */
                    ajStrSetValidLen(&contseq, gcglen);
                }
                else
                {
                    ajRegSubI(featRegGcgId, 1, &tmpstr);
                    ajFatal("Unknown GCG entry: name '%S'",
                            tmpstr);
                }

                if(!ajReadline(qryd->libs, &line)) /* newline at end */
                    ajFatal("error reading file %F", qryd->libs);

                if(!featRegGcgCont)
                    featRegGcgCont = ajRegCompC("^([^ ]+) +([^ ]+) +([^ ]+) +"
                                                "([^ ]+) +([^ ]+) +([^ ]+) "
                                                "+([^ ]+) +"
                                                "([^ ]+) +([0-9]+)");

                ajRegExec(featRegGcgCont, dstr);
                ajRegSubI(featRegGcgCont, 9, &tmpstr);
                ajStrToInt(tmpstr, &pos);
                /*seqin->Inseq->Len = pos-1;*/

                /*ajStrAppendS(&seqin->Inseq,contseq);*/
                spos = ajFileResetPos(qryd->libs);
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




/* @funcstatic featCdQryReuse *************************************************
**
** Tests whether Cd index query data can be reused or whether we are finished.
**
** Clears qryData structure when finished.
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool featCdQryReuse(AjPQuery qry)
{
    FeatPCdQry qryd;

    qryd = qry->QryData;

    if(!qry || !qryd)
        return ajFalse;


    /*    ajDebug("qry->ResultsList  %x\n",qry->ResultsList);*/
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
        ajDebug("seqfile '%S'\n", qryd->seqfile);
        ajDebug("name    '%s'\n", qryd->name);
        ajDebug("nameSize %d\n",  qryd->nameSize);
        ajDebug("div      %d\n",  qryd->div);
        ajDebug("maxdiv   %d\n",  qryd->maxdiv);
        /*ajListTrace(qry->ResultsList);*/
    }

    return ajTrue;
}




/* @funcstatic featCdQryOpen **************************************************
**
** Opens everything for a new CD query
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool featCdQryOpen(AjPQuery qry)
{
    FeatPCdQry qryd;

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
    qryd->dfp = featCdFileOpen(qry->IndexDir, "division.lkp", &qryd->divfile);

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
    featCdFileSeek(qryd->dfp, 0);

    for(i=0; i < qryd->maxdiv; i++)
    {
        ajReadbinInt2(qryd->dfp->File, &j);
        ajReadbinCharTrim(qryd->dfp->File, qryd->nameSize, name);

        ajStrAssignC(&fullName, name);
        ajFilenameReplacePathS(&fullName, qry->Directory);

        if(!ajFilenameTestInclude(fullName, qry->Exclude, qry->Filename))
            qryd->Skip[i] = ajTrue;
    }

    qryd->ifp = featCdFileOpen(qry->IndexDir, "entrynam.idx", &qryd->idxfile);

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




/* @funcstatic featCdQryEntry *************************************************
**
** Queries for a single entry in an EMBLCD index
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool featCdQryEntry(AjPQuery qry)
{
    FeatPCdEntry entry = NULL;
    ajint ipos = -1;
    ajint trghit;
    FeatPCdQry qryd;
    const AjPList fdlist;
    AjIList iter;
    AjPQueryField fd;
    AjPStr qrystr = NULL;
    ajuint i;
    ajuint ii;
    ajint j;

    const char* embossfields[] = {
        "id", "acc",   "sv",    "org",   "key",     "des", "gi", NULL
    };
    const char* emblcdfields[] = {
        NULL, "acnum", "seqvn", "taxon", "keyword", "des", "gi", NULL
    };

    fdlist = ajQueryGetallFields(qry);

    ajQueryGetQuery(qry, &qrystr);

    ajDebug("featCdQryEntry %S hasacc:%B\n",
            qrystr , qry->HasAcc);
    ajStrDel(&qrystr);

    qryd = qry->QryData;
    iter= ajListIterNewread(fdlist);

    while(!ajListIterDone(iter))
    {
        fd = ajListIterGet(iter);
        ajDebug("query link: %u field '%S' wild '%S'\n",
                fd->Link, fd->Field, fd->Wildquery);

        if((fd->Link == AJQLINK_ELSE) && ajListGetLength(qry->ResultsList))
            continue;

        for(i=0; embossfields[i]; i++)
        {
            ajDebug("test field[%u] '%S' = '%s'\n",
                    i, fd->Field, emblcdfields[i]);
            if(ajStrMatchC(fd->Field, embossfields[i]))
            {
                ajDebug("match field[%u] '%S' = '%s'\n",
                        i, fd->Field, emblcdfields[i]);
                if(!emblcdfields[i]) /* ID index */
                {
                    ipos = featCdIdxSearch(qryd->idxLine,
                                           fd->Wildquery, qryd->ifp);

                    if(ipos >= 0)
                    {
                        if(!qryd->Skip[qryd->idxLine->DivCode-1])
                        {
                            AJNEW0(entry);
                            entry->div = qryd->idxLine->DivCode;
                            entry->annoff = qryd->idxLine->AnnOffset;
                            entry->seqoff = qryd->idxLine->SeqOffset;
                            ajListPushAppend(qry->ResultsList, (void*)entry);
                        }
                        else
                            ajDebug("SKIP: '%S' [file %d]\n",
                                    fd->Wildquery, qryd->idxLine->DivCode);
                    }
                }
                else            /* target/hit index */
                {
                    if(featCdTrgOpen(qry->IndexDir, emblcdfields[i],
                                     &qryd->trgfp, &qryd->hitfp))
                    {
                        trghit = featCdTrgSearch(qryd->trgLine,
                                                 fd->Wildquery, qryd->trgfp);

                        if(trghit >= 0)
                        {
                            featCdFileSeek(qryd->hitfp,
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
                                featCdIdxLine(qryd->idxLine, j, qryd->ifp);

                                if(!qryd->Skip[qryd->idxLine->DivCode-1])
                                {
                                    AJNEW0(entry);
                                    entry->div = qryd->idxLine->DivCode;
                                    entry->annoff = qryd->idxLine->AnnOffset;
                                    entry->seqoff = qryd->idxLine->SeqOffset;
                                    ajListPushAppend(qry->ResultsList,
                                                     (void*)entry);
                                }
                                else
                                    ajDebug("SKIP: %s '%S' [file %d]\n",
                                            emblcdfields[i],
                                            fd->Wildquery,
                                            qryd->idxLine->DivCode);
                            }
                        }

                        featCdTrgClose(&qryd->trgfp, &qryd->hitfp);
                        ajStrDel(&qryd->trgLine->Target);
                    }
                }
                break;
            }
        }
        if(!embossfields[i])
        {
            if(featCdTrgOpen(qry->IndexDir, MAJSTRGETPTR(fd->Field),
                             &qryd->trgfp, &qryd->hitfp))
            {
                trghit = featCdTrgSearch(qryd->trgLine,
                                         fd->Wildquery, qryd->trgfp);

                if(trghit >= 0)
                {
                    featCdFileSeek(qryd->hitfp,
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
                        featCdIdxLine(qryd->idxLine, j, qryd->ifp);

                        if(!qryd->Skip[qryd->idxLine->DivCode-1])
                        {
                            AJNEW0(entry);
                            entry->div = qryd->idxLine->DivCode;
                            entry->annoff = qryd->idxLine->AnnOffset;
                            entry->seqoff = qryd->idxLine->SeqOffset;
                            ajListPushAppend(qry->ResultsList, (void*)entry);
                        }
                        else
                            ajDebug("SKIP: %S '%S' [file %d]\n",
                                    fd->Field,
                                    fd->Wildquery,
                                    qryd->idxLine->DivCode);
                    }
                }

                featCdTrgClose(&qryd->trgfp, &qryd->hitfp);
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




/* @funcstatic featCdQryQuery *************************************************
**
** Queries for one or more entries in an EMBLCD index
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool featCdQryQuery(AjPQuery qry)
{
    AjIList iter = NULL;
    AjPQueryField field = NULL;

    FeatPCdEntry newhit;
    FeatPCdEntry *allhits = NULL;
    ajulong** keys = NULL;

    AjPTable newtable = NULL;

    ajuint i;
    ajuint lasthits = 0;
    ajuint fdhits = 0;

    ajulong *ikey = NULL;

    ajuint ishift = sizeof(ajulong)/2;

    if(!qry->CaseId)
        qry->QryDone = ajTrue;

    ajTableSettypeUlong(qry->ResultsTable);
    ajTableSetDestroyboth(qry->ResultsTable);

    iter = ajListIterNewread(qry->QueryFields);

    while(!ajListIterDone(iter))
    {
        field = ajListIterGet(iter);

        if((field->Link == AJQLINK_ELSE) && (lasthits > 0))
        {
            ajDebug("ELSE: lasthits:%u skip\n", lasthits);
            continue;
        }

        if(ajStrMatchC(field->Field, "id"))
            featCdIdxQuery(qry, field->Wildquery);
        else
            featCdTrgQuery(qry, field->Field, field->Wildquery);

        fdhits = (ajuint) ajListGetLength(qry->ResultsList);

        ajDebug("featCdQryQuery hits: %u link: %u\n",
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
                     &featCdEntryCmp, &featCdEntryDel);

    AJFREE(allhits);

    ajDebug("featCdQryQuery clear results table\n");
    ajTableClear(qry->ResultsTable);

    if(ajListGetLength(qry->ResultsList))
        return ajTrue;

    return ajFalse;
}




/* @funcstatic featCdEntryCmp *************************************************
**
** Compares two FeatPEntry objects
**
** @param [r] pa [const void*] FeatPEntry object
** @param [r] pb [const void*] FeatPEntry object
** @return [int] -1 if first entry should sort before second, +1 if the
**         second entry should sort first. 0 if they are identical
**
** @release 6.5.0
** @@
******************************************************************************/
static int featCdEntryCmp(const void* pa, const void* pb)
{
    const FeatPCdEntry a;
    const FeatPCdEntry b;

    a = *(FeatPCdEntry const *) pa;
    b = *(FeatPCdEntry const *) pb;

    /*
      ajDebug("featCdEntryCmp %x %d %d : %x %d %d\n",
      a, a->div, a->annoff,
      b, b->div, b->annoff);
    */
    if(a->div != b->div)
        return (a->div - b->div);

    return (a->annoff - b->annoff);
}




/* @funcstatic featCdEntryDel***************************************************
**
** Deletes a FeatPCdEntry object
**
** @param [r] pentry [void**] Address of a FeatPCdEntry object
** @param [r] cl [void*] Standard unused argument, usually NULL.
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/
static void featCdEntryDel(void** pentry, void* cl)
{
    (void) cl;

    AJFREE(*pentry);

    return;
}




/* @funcstatic featCdQryNext **************************************************
**
** Processes the next query for an EMBLCD index
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if successful
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool featCdQryNext(AjPQuery qry)
{
    FeatPCdEntry entry;
    FeatPCdQry qryd;
    void* item;

    qryd = qry->QryData;

    if(!ajListGetLength(qry->ResultsList))
        return ajFalse;

    ajDebug("qry->ResultsList (b) length %Lu\n",
            ajListGetLength(qry->ResultsList));
    /*ajListTrace(qry->ResultsList);*/
    ajListPop(qry->ResultsList, &item);
    entry = (FeatPCdEntry) item;

    /*
      ajDebug("entry: %x div: %d (%d) ann: %d seq: %d\n",
      entry, entry->div, qryd->div, entry->annoff, entry->seqoff);
    */
    qryd->idnum = entry->annoff - 1;

    /*
      ajDebug("idnum: %d\n", qryd->idnum);
    */
    qryd->Samefile = ajTrue;

    if(entry->div != qryd->div)
    {
        qryd->Samefile = ajFalse;
        qryd->div = entry->div;
        /*ajDebug("div: %d\n", qryd->div);*/

        if(!featCdQryFile(qry))
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




/* @funcstatic featCdQryClose *************************************************
**
** Closes query data for an EMBLCD index
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if all is done
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool featCdQryClose(AjPQuery qry)
{
    FeatPCdQry qryd = NULL;

    ajDebug("featCdQryClose clean up qryd\n");

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

    featCdIdxDel(&qryd->idxLine);
    featCdTrgDel(&qryd->trgLine);

    featCdFileClose(&qryd->ifp);
    featCdFileClose(&qryd->dfp);
    /* defined in a buffer, cleared there */
    /*
      ajFileClose(&qryd->libr);
      ajFileClose(&qryd->libs);
    */
    qryd->libr=0;
    qryd->libs=0;
    AJFREE(qryd->trgLine);
    AJFREE(qryd->idxLine);
    AJFREE(qryd->Skip);

    /* keep QryData for use at top of loop */

    return ajTrue;
}




/* @section B+tree Database Indexing *****************************************
**
** These functions manage the B+tree index access methods.
**
******************************************************************************/




/* @funcstatic featEmbossQryReuse *********************************************
**
** Tests whether the B+tree index query data can be reused or it's finished.
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

static AjBool featEmbossQryReuse(AjPQuery qry)
{
    FeatPEmbossQry qryd;

    qryd = qry->QryData;

    if(!qry || !qryd)
        return ajFalse;


    if(!qry->ResultsList)
    {
        ajDebug("featEmbossQryReuse: query data all finished\n");

        return ajFalse;
    }
    else
    {
        ajDebug("featEmbossQryReuse: reusing data from previous call %x\n",
                qry->QryData);
        /*ajListTrace(qry->ResultsList);*/
    }


    qryd->nentries = -1;


    return ajTrue;
}




/* @funcstatic featEmbossQryOpen **********************************************
**
** Open caches (etc) for B+tree search
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool featEmbossQryOpen(AjPQuery qry)
{
    FeatPEmbossQry qryd;
    ajint i;
    AjPStr name     = NULL;
    AjIList iter = NULL;
    AjPQueryField field = NULL;
    AjPBtcache cache = NULL;

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

    if(!featEmbossOpenCache(qry,"id",&qryd->idcache))
        return ajFalse;

    iter = ajListIterNewread(qry->QueryFields);
    while(!ajListIterDone(iter))
    {
        field = ajListIterGet(iter);

        ajStrFmtLower(&field->Wildquery);
        if(!featEmbossOpenCache(qry, MAJSTRGETPTR(field->Field), &cache))
            return ajFalse;
        ajListPushAppend(qryd->Caches, cache);
        cache = NULL;
    }
    ajListIterDel(&iter);


    ajDebug("directory '%S'fields: %Lu hasacc:%B\n",
            qry->IndexDir, ajListGetLength(qry->QueryFields), qry->HasAcc);


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




/* @funcstatic featEmbossOpenCache ********************************************
**
** Create primary B+tree index cache
**
** @param [u] qry [AjPQuery] Query data
** @param [r] ext [const char*] Index file extension
** @param [w] cache [AjPBtcache*] cache
** @return [AjBool] True on success
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool featEmbossOpenCache(AjPQuery qry, const char *ext,
                                  AjPBtcache *cache)
{
    FeatPEmbossQry qryd;
    AjPStr indexextname = NULL;

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
                                             &qryd->reffiles,
                                             &qryd->refcount);

    return ajTrue;
}





/* @funcstatic featEmbossQryEntry *********************************************
**
** Queries for a single entry in a B+tree index
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if can continue,
**                  ajFalse if all is done.
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool featEmbossQryEntry(AjPQuery qry)
{
    AjPBtHitref newhit  = NULL;
    FeatPEmbossQry qryd;
    const AjPList fdlist;
    const AjPList cachelist;
    AjIList iter;
    AjIList icache;
    AjPBtcache cache;
    AjPQueryField fd;
    AjPBtHitref *allhits = NULL;
    ajuint i;

    ajDebug("featEmbossQryEntry fields: %Lu hasacc:%B\n",
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

        ajDebug("qry type:%d field '%S' wild '%S'\n",
                fd->Link, fd->Field, fd->Wildquery);

        if((fd->Link == AJQLINK_ELSE) && ajListGetLength(qry->ResultsList))
            continue;

        if(!ajBtreeCacheIsSecondary(cache))
        {
            ajBtreeIdentFetchHitref(cache,fd->Wildquery,
                                    qry->ResultsList);
        }
    }

    ajListIterDel(&iter);
    ajListIterDel(&icache);

    if(ajStrGetLen(qry->Organisms))
    {
        ajTableSetDestroy(qry->ResultsTable, NULL, &ajBtreeHitrefDelVoid);
        ajTableSettypeUser(qry->ResultsTable,
                           &ajBtreeHitrefCmp, &ajBtreeHitrefHash);

        while(ajListPop(qry->ResultsList, (void**)&newhit))
            ajTablePutClean(qry->ResultsTable, newhit, newhit,
                            NULL, &ajBtreeHitrefDelVoid);

         featEmbossQryOrganisms(qry);

         ajTableToarrayValues(qry->ResultsTable, (void***)&allhits);

         for(i=0; allhits[i]; i++)
             ajListPushAppend(qry->ResultsList, (void*) allhits[i]);
    }

    if(!ajListGetLength(qry->ResultsList))
        return ajFalse;

    if(!qry->CaseId)
        qry->QryDone = ajTrue;

    return ajTrue;
}




/* @funcstatic featEmbossQryNext **********************************************
**
** Processes the next query for a B+tree index
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if successful
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool featEmbossQryNext(AjPQuery qry)
{
    AjPBtHitref entry;
    FeatPEmbossQry qryd;
    void* item;
    AjBool ok = ajFalse;


    qryd = qry->QryData;

    if(!ajListGetLength(qry->ResultsList))
        return ajFalse;

    /*ajListTrace(qry->ResultsList);*/


    if(!qryd->Skip)
    {
        ajListPop(qry->ResultsList, &item);
        entry = (AjPBtHitref) item;
    }
    else
    {
        ok = ajFalse;

        while(!ok)
        {
            ajListPop(qry->ResultsList, &item);
            entry = (AjPBtHitref) item;

            if(!qryd->Skip[entry->dbno])
                ok = ajTrue;
            else
            {
                ajBtreeHitrefDel(&entry);

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
        ajFileClose(&qryd->libs);

        if(qryd->reffiles)
            ajFileClose(&qryd->libr);
    }

    if(!qryd->libs)
    {
        qryd->libs = ajFileNewInNameS(qryd->files[entry->dbno]);

        if(!qryd->libs)
        {
            ajBtreeHitrefDel(&entry);

            return ajFalse;
        }
    }

    if(qryd->reffiles && !qryd->libr)
    {
        ajFileClose(&qryd->libr);
        qryd->libr = ajFileNewInNameS(qryd->reffiles[0][entry->dbno]);

        if(!qryd->libr)
        {
            ajBtreeHitrefDel(&entry);

            return ajFalse;
        }
    }


    ajFileSeek(qryd->libs, (ajlong) entry->offset, 0);
    if(qryd->reffiles)
        ajFileSeek(qryd->libr, (ajlong) entry->refoffset, 0);

    ajBtreeHitrefDel(&entry);

    if(!qry->CaseId)
        qry->QryDone = ajTrue;

    return ajTrue;
}




/* @funcstatic featEmbossQryClose *********************************************
**
** Closes query data for a B+tree index
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool featEmbossQryClose(AjPQuery qry)
{
    FeatPEmbossQry qryd;
    void* item;
    ajint i;
    ajuint iref;

    if(!qry)
        return ajFalse;

    if(!qry->QryData)
        return ajFalse;

    ajDebug("featEmbossQryClose clean up qryd\n");

    qryd = qry->QryData;

    while(ajListGetLength(qryd->Caches))
    {
        ajListPop(qryd->Caches, &item);
        ajBtreeCacheDel((AjPBtcache*) &item);
    }
    ajListFree(&qryd->Caches);
    ajBtreeCacheDel(&qryd->idcache);

    ajListFree(&qry->ResultsList);

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
            {
                for(iref=0; iref < qryd->refcount; iref++)
                    ajStrDel(&qryd->reffiles[iref][i]);
            }

            ++i;
        }

        AJFREE(qryd->files);
    }

    if(qryd->reffiles)
    {
        for(iref=0; iref < qryd->refcount; iref++)
                    AJFREE(qryd->reffiles[iref]);
        
        AJFREE(qryd->reffiles);
    }

    qryd->files = NULL;
    qryd->reffiles = NULL;


    /* keep QryData for use at top of loop */

    return ajTrue;
}




/* @funcstatic featEmbossQryOrganisms *****************************************
**
** Restricts results to matches to organism(s) in database
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue on success.
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool featEmbossQryOrganisms(AjPQuery qry)
{
    FeatPEmbossQry qryd;
    AjPBtcache orgcache;
    AjPStr orgqry = NULL;
    AjPStrTok orghandle = NULL;
    AjPTable orgtable = NULL;
    AjPList orglist = NULL;
    AjPBtHitref newhit;
    ajulong fdhits = 0UL;

    if(!ajStrGetLen(qry->Organisms))
        return ajTrue;

    qryd = qry->QryData;

    featEmbossOpenCache(qry, "org", &orgcache);
    orglist = ajListNew();
    orghandle = ajStrTokenNewC(qry->Organisms, "\t,;|");
    while(ajStrTokenNextParse(&orghandle, &orgqry))
    {
        if(ajBtreeCacheIsSecondary(orgcache))
        {
            if(!qry->Wild)
            {
                ajBtreeKeyFetchHitref(orgcache,qryd->idcache,
                                      orgqry, orglist);

            }
            else
            {
               ajBtreeKeyFetchwildHitref(orgcache, qryd->idcache,
                                         orgqry, orglist);
            }
        }
        else
        {
            ajBtreeIdentFetchwildHitref(orgcache,
                                        orgqry, orglist);
        }

        fdhits += ajListGetLength(orglist);
        ajDebug("Organisms list orgqry '%S' list '%Lu'", orgqry, fdhits);

    }

    orgtable = ajTableNewFunctionLen(fdhits,
				     &ajBtreeHitrefCmp,
                                     &ajBtreeHitrefHash,
				     NULL, &ajBtreeHitrefDelVoid);
    while(ajListPop(orglist, (void**)&newhit))
        ajTablePutClean(orgtable, newhit, newhit,
                        NULL, &ajBtreeHitrefDelVoid);

    ajStrTokenDel(&orghandle);

    ajTableMergeAnd(qry->ResultsTable, orgtable);
    ajListFree(&orglist);
    ajBtreeCacheDel(&orgcache);
    ajTableFree(&orgtable);
    ajStrDel(&orgqry);

    return ajTrue;
}




/* @funcstatic featEmbossQryQuery *********************************************
**
** Queries for one or more entries in an EMBOSS B+tree index
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool featEmbossQryQuery(AjPQuery qry)
{
    FeatPEmbossQry qryd;

    const AjPList fdlist;
    const AjPList cachelist;
    AjIList iter;
    AjIList icache;
    AjPBtcache cache;
    AjPQueryField fd;

    AjPBtHitref newhit;
    AjPBtHitref *allhits = NULL;
    AjPTable newtable = NULL;

    ajuint i;
    ajulong lasthits = 0UL;
    ajulong fdhits = 0UL;

    if(!qry->CaseId)
        qry->QryDone = ajTrue;

    qryd = qry->QryData;

    cachelist = qryd->Caches;

    ajTableSetDestroy(qry->ResultsTable, NULL, &ajBtreeHitrefDelVoid);
    ajTableSettypeUser(qry->ResultsTable,
                       &ajBtreeHitrefCmp, &ajBtreeHitrefHash);

    fdlist = ajQueryGetallFields(qry);

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
                ajBtreeKeyFetchHitref(cache, qryd->idcache,
                                      fd->Wildquery, qry->ResultsList);
            }
            else
            {
                ajBtreeKeyFetchwildHitref(cache, qryd->idcache,
                                          fd->Wildquery, qry->ResultsList);
            }
        }
        else
        {
            ajBtreeIdentFetchwildHitref(cache,fd->Wildquery,qry->ResultsList);
            ajDebug("ajBtreeIdentFetchwild results:%Lu\n",
                    ajListGetLength(qry->ResultsList));
        }

        fdhits = ajListGetLength(qry->ResultsList);

        switch(fd->Link)
        {
            case AJQLINK_INIT:
                while(ajListPop(qry->ResultsList, (void**)&newhit))
                    ajTablePutClean(qry->ResultsTable, newhit, newhit,
                                    NULL, &ajBtreeHitrefDelVoid);
                break;

            case AJQLINK_OR:
                newtable = ajTableNewFunctionLen(fdhits,
                                                 &ajBtreeHitrefCmp,
                                                 &ajBtreeHitrefHash,
                                                 NULL, &ajBtreeHitrefDelVoid);
                while(ajListPop(qry->ResultsList, (void**)&newhit))
                    ajTablePutClean(newtable, newhit, newhit,
                                    NULL, &ajBtreeHitrefDelVoid);

                ajTableMergeOr(qry->ResultsTable, newtable);
                ajTableDel(&newtable);
                break;

            case AJQLINK_AND:
                newtable = ajTableNewFunctionLen(fdhits,
                                                 &ajBtreeHitrefCmp,
                                                 &ajBtreeHitrefHash,
                                                 NULL, &ajBtreeHitrefDelVoid);
                while(ajListPop(qry->ResultsList, (void**)&newhit))
                    ajTablePutClean(newtable, newhit, newhit,
                                    NULL, &ajBtreeHitrefDelVoid);

                ajTableMergeAnd(qry->ResultsTable, newtable);
                ajTableDel(&newtable);
                break;

            case AJQLINK_EOR:
            case AJQLINK_ELSE:
                newtable = ajTableNewFunctionLen(fdhits,
                                                 &ajBtreeHitrefCmp,
                                                 &ajBtreeHitrefHash,
                                                 NULL, ajBtreeHitrefDelVoid);
                while(ajListPop(qry->ResultsList, (void**)&newhit))
                    ajTablePutClean(newtable, newhit, newhit,
                                    NULL, &ajBtreeHitrefDelVoid);

                ajTableMergeEor(qry->ResultsTable, newtable);
                ajTableDel(&newtable);
                break;

            case AJQLINK_NOT:
                newtable = ajTableNewFunctionLen(fdhits,
                                                 &ajBtreeHitrefCmp,
                                                 &ajBtreeHitrefHash,
                                                 NULL, &ajBtreeHitrefDelVoid);
                while(ajListPop(qry->ResultsList, (void**)&newhit))
                    ajTablePutClean(newtable, newhit, newhit,
                                    NULL, &ajBtreeHitrefDelVoid);

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
        featEmbossQryOrganisms(qry);

    ajTableToarrayValues(qry->ResultsTable, (void***)&allhits);
    for(i=0; allhits[i]; i++)
        ajListPushAppend(qry->ResultsList, (void*) allhits[i]);

    AJFREE(allhits);

    ajTableClear(qry->ResultsTable);

    if(ajListGetLength(qry->ResultsList))
        return ajTrue;

    return ajFalse;
}




/* @section CHADO Indexing ****************************************************
**
** These functions manage the CHADO database access methods.
**
******************************************************************************/




/* @funcstatic featAccessChado ************************************************
**
** Reads features from CHADO databases
**
** @param [u] fttabin [AjPFeattabin] Feature input.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool featAccessChado(AjPFeattabin fttabin)
{
    AjPQuery qry = NULL;
    AjPStr seqid = NULL;

    AjPStr filterseqregions = NULL;

    AjIList fielditer     = NULL;
    AjPQueryField field   = NULL;
    AjPFeattable  feattab = NULL;

    AjPStr fieldname = NULL;
    AjPStr condition = NULL;

    AjPSqlconnection connection = NULL;

    AjBool ret = ajTrue;

#if !defined(HAVE_MYSQL) && !defined(HAVE_POSTGRESQL)
    ajWarn("Cannot use access method chado without mysql or postgresql");
    return ajFalse;
#endif

    if(fttabin->Input->Records)
	return ajFalse;

    qry = fttabin->Input->Query;

    ajDebug("featAccesschado: %S fields: %Lu\n",
            qry->DbAlias, ajListGetLength(qry->QueryFields));

    filterseqregions = ajStrNew();

    fielditer = ajListIterNewread(qry->QueryFields);

    while(!ajListIterDone(fielditer))
    {
	field = ajListIterGet(fielditer);

	ajStrAssignS(&fieldname, field->Field);

	ajDebug("field:%S - val:%S\n", field->Field, field->Wildquery);

	if(ajStrMatchCaseC(field->Field, "id"))
	{
	    ajStrAssignS(&seqid, field->Wildquery);
	    ajStrAssignC(&fieldname, "uniquename");
	}

	if(ajStrFindAnyC(field->Wildquery,"*?") != -1)
	{
	    ajStrExchangeKK(&field->Wildquery,'*','%');
	    ajStrExchangeKK(&field->Wildquery,'?','_');
	    ajFmtPrintS(&condition," %S LIKE '%S'", fieldname,
	                   field->Wildquery);
	}
	else
	    ajFmtPrintS(&condition,"%S = '%S'", fieldname,
	                   field->Wildquery);

	if (ajStrGetLen(filterseqregions))
	    ajStrAppendC(&filterseqregions," AND ");

	ajStrAppendS(&filterseqregions, condition);

	ajStrDel(&fieldname);
	ajStrDel(&condition);
    }

    ajListIterDel(&fielditer);

    ajDebug("dbfilter: %S\n", qry->DbFilter);

    if(ajStrGetLen(qry->DbFilter))
        ajFmtPrintAppS(&filterseqregions, " AND %S", qry->DbFilter);

    ajDebug("filter: %S\n", filterseqregions);

    feattab = ajFeattableNew(seqid);
    connection = featChadoConnect(qry);

    if(!ajStrGetLen(feattab->Db))
	ajStrAssignS(&feattab->Db, qry->DbName);

    ret = featChadoQryfeatureQuery(connection, filterseqregions, feattab,
                                   fttabin->Start, fttabin->End);

    ajSqlconnectionDel(&connection);

    fttabin->Input->TextData = feattab;

    qry->QryDone = ajTrue;

    ajStrDel(&seqid);
    ajStrDel(&filterseqregions);

    return ret;
}




/* @funcstatic featChadoQryfeatureQuery ***************************************
**
** Queries the connected chado database for the emboss feature query.
** More queries are made for the child features of the queried feature(s)
** using function featChadoChildfeatureQuery.
**
** @param [u] connection [AjPSqlconnection] SQL Database connection
** @param [u] filter [AjPStr] Filter conditions for the SQL query
** @param [u] feattab [AjPFeattable] Feature table
** @param [r] qrystart [ajint] start pos specified as part of the feature query
** @param [r] qryend [ajint] end pos specified as part of the feature query
** @return [AjBool] returns ajFalse if no features found
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool featChadoQryfeatureQuery(AjPSqlconnection connection,
				       AjPStr filter,
				       AjPFeattable  feattab,
				       ajint qrystart, ajint qryend)
{
    AjPSqlstatement statement = NULL;
    AjBool debugOn = ajFalse;
    ajint i = 0;
    AjPStr sql = NULL;

    AjISqlrow iter = NULL;
    AjPSqlrow row  = NULL;
    AjPStr colstr = NULL;
    AjPStr rowstr = NULL;
    AjPStr regionuniquename = NULL;

    AjBool ret = ajTrue;

    debugOn = ajDebugOn();
    sql = ajStrNew();

    ajFmtPrintS(&sql,
	    "SELECT feature.name, feature.uniquename,\n"
	    " (SELECT COUNT(*) FROM featureloc"
	    " WHERE feature.feature_id = featureloc.srcfeature_id),\n"
	    " srcfeatureloc.srcfeature_id,"
	    " srcfeature.uniquename,"
	    " srcfeatureloc.fmin,"
	    " srcfeatureloc.fmax,"
	    " srcfeatureloc.strand, srcfeatureloc.phase,\n"
	    " feature.seqlen,"
	    " cvterm.name, db.name,"
	    " feature.timelastmodified, feature.is_obsolete\n"
	    " FROM feature\n"
	    " LEFT JOIN cvterm ON cvterm.cvterm_id = feature.type_id\n"
	    " LEFT OUTER JOIN dbxref ON dbxref.dbxref_id = feature.dbxref_id\n"
	    " LEFT OUTER JOIN db USING (db_id)\n"
	    " LEFT OUTER JOIN featureloc srcfeatureloc"
	    " ON feature.feature_id = srcfeatureloc.feature_id\n"
	    " LEFT OUTER JOIN feature srcfeature"
	    " ON srcfeature.feature_id = srcfeatureloc.srcfeature_id\n"
	    " WHERE feature.%S\n",
	    filter
	    );

    ajDebug("SQL, query-feature query:\n%S\n",sql);

    statement = ajSqlstatementNewRun(connection,sql);

    if(!statement)
    {
	ajErr("Could not execute SQL statement [%S]", sql);
	ajExitBad();
	return ajFalse;
    }

    iter = ajSqlrowiterNew(statement);

    if(!iter)
	ret = ajFalse;

    while(!ajSqlrowiterDone(iter))
    {
	row = ajSqlrowiterGet(iter);

	if(debugOn)
	{
	    while(row->Current < row->Columns)
	    {
		ajSqlcolumnToStr(row,&colstr);
		ajFmtPrintAppS(&rowstr, "\t%S",colstr);
	    }

	    ajDebug("qryfeature row %d: %S\n", ++i, rowstr);
	    ajStrSetClear(&rowstr);
	}

	ajSqlcolumnRewind(row);

	regionuniquename = featChadoQryfeatureRow(feattab, row,
	                                          qrystart, qryend);
	featChadoChildfeatureQuery(connection, feattab, regionuniquename);
	ajStrDel(&regionuniquename);
    }

    ajSqlrowiterDel(&iter);
    ajSqlstatementDel(&statement);
    ajStrDel(&colstr);
    ajStrDel(&rowstr);
    ajStrDel(&sql);

    return ret;
}




/* @funcstatic featChadoChildfeatureQuery *************************************
**
** Query to retrieve features located on the feature specified
** using the srcfeature parameter
**
** @param [u] connection [AjPSqlconnection] SQL Database connection
** @param [u] feattab [AjPFeattable] Feature table
** @param [r] srcfeature [const AjPStr] Unique name of the sequence region
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void featChadoChildfeatureQuery(AjPSqlconnection connection,
			               AjPFeattable  feattab,
                                       const AjPStr srcfeature)
{
    AjPSqlstatement statement = NULL;
    AjBool debugOn = ajFalse;
    ajint i = 0;

    AjISqlrow iter = NULL;
    AjPSqlrow row  = NULL;
    AjPStr colstr  = NULL;
    AjPStr rowstr  = NULL;
    AjPStr sql     = NULL;
    AjPFeature seqregionf = NULL;

    debugOn = ajDebugOn();
    sql   = ajStrNew();

    ajListPeekLast(feattab->Features, (void**)&seqregionf);

    /*
     * in SQL query below, feature refers to the sequence region being queried
     * sfeature refers to the features located within the sequence region,
     *
     * it is assumed that results are ordered such that features with the same
     * uniquename follows each other, we should check whether we need to use
     * explicit ORDER BY for any reason...
     */

    ajFmtPrintS(&sql,
	    "SELECT sfeature.name, sfeature.uniquename, relation.name,"
	    " object.uniquename, object_type.name, featureloc.fmin,"
	    " featureloc.fmax, featureloc.strand, featureloc.phase,\n"
	    " cvterm.name, db.name, analysis.program, analysisfeature.rawscore,"
	    " sfeature.timelastmodified, sfeature.is_obsolete\n"
	    " FROM feature\n"
	    " JOIN featureloc"
	    " ON feature.feature_id = featureloc.srcfeature_id\n"
	    " JOIN feature sfeature"
	    " ON sfeature.feature_id = featureloc.feature_id\n"
	    " LEFT OUTER JOIN feature_relationship"
	    " ON sfeature.feature_id = feature_relationship.subject_id\n"
	    " LEFT OUTER JOIN cvterm relation"
	    " ON relation.cvterm_id = feature_relationship.type_id\n"
	    " LEFT OUTER JOIN feature object"
	    " ON object.feature_id = feature_relationship.object_id\n"
	    " LEFT OUTER JOIN cvterm object_type"
	    " ON object.type_id = object_type.cvterm_id\n"
	    " JOIN cvterm ON cvterm.cvterm_id = sfeature.type_id\n"
	    " LEFT OUTER JOIN dbxref ON dbxref.dbxref_id = sfeature.dbxref_id\n"
	    " LEFT OUTER JOIN db USING (db_id)\n"
	    " LEFT OUTER JOIN analysisfeature"
	    " ON sfeature.feature_id=analysisfeature.feature_id\n"
	    " LEFT OUTER JOIN analysis USING(analysis_id)\n"
	    " WHERE \n"
	    " feature.uniquename='%S'"
	    " AND"
	    " ((featureloc.fmin>%d AND featureloc.fmin<%d)"
	    " OR"
	    " (featureloc.fmax>=%d AND featureloc.fmax<=%d))\n",
	    srcfeature,
	    seqregionf->Start, seqregionf->End,
	    seqregionf->Start, seqregionf->End);

    ajDebug("SQL features:\n%S\n",sql);

    statement = ajSqlstatementNewRun(connection,sql);

    if(!statement)
    {
	ajErr("Could not execute SQL statement [%S]", sql);
	ajExitAbort();
    }

    iter = ajSqlrowiterNew(statement);

    while(!ajSqlrowiterDone(iter))
    {
	row = ajSqlrowiterGet(iter);

	if(debugOn)
	{
	    while(row->Current < row->Columns)
	    {
		ajSqlcolumnToStr(row,&colstr);
		ajFmtPrintAppS(&rowstr, "\t%S",colstr);
	    }

	    ajDebug("row %d: %S\n", ++i, rowstr);
	    ajStrSetClear(&rowstr);
	}

	ajSqlcolumnRewind(row);

	featChadoChildfeatureRow(feattab, row);
    }

    ajSqlrowiterDel(&iter);
    ajSqlstatementDel(&statement);
    ajStrDel(&colstr);
    ajStrDel(&rowstr);
    ajStrDel(&sql);

    return;
}




/* @funcstatic featChadoChildfeatureRow ***************************************
**
** Generates an emboss feature obj from a row of chado query results, and
** appends it to the feature table fttab.
**
** @param [u] fttab [AjPFeattable] Feature table
** @param [u] row [AjPSqlrow] Input row
** @return [AjPFeature] New feature
**
** @release 6.4.0
** @@
******************************************************************************/

static AjPFeature featChadoChildfeatureRow(AjPFeattable fttab, AjPSqlrow row)
{
    AjPFeature feature = NULL;
    AjPFeature prevft  = NULL;

    AjPStr name = NULL;
    AjPStr type = NULL;
    AjPStr source = NULL;
    AjPStr sourcedb = NULL;
    AjPStr sourceprogram = NULL;
    AjPTagval idtag = NULL;

    ajint start  = 0;
    ajint end    = 0;
    float score  = 0;
    char  strand = '+';
    ajint i = 0;
    ajint frame = 0;

    AjPStr entryid = NULL;
    AjBool updateprevft = ajFalse;
    AjPStr alias = NULL;
    AjPStr relation = NULL;

    /* The object can also be thought of as parent (containing feature),
     * and subject as child (contained feature or subfeature). ref:gmod.org */

    AjPStr object = NULL;
    AjPStr objecttype = NULL;
    AjPTime timelm = NULL;
    AjPStr timelmS = NULL;
    AjBool isObsolete = ajFalse;
    AjPStr isObsoleteS = NULL;

    if(!ajSqlrowGetColumns(row))
	return NULL;

    timelm = ajTimeNew();

    ajSqlcolumnToStr(row, &name);
    ajSqlcolumnToStr(row, &entryid);
    ajSqlcolumnToStr(row, &relation);
    ajSqlcolumnToStr(row, &object);
    ajSqlcolumnToStr(row, &objecttype);
    ajSqlcolumnToInt(row, &start);
    ajSqlcolumnToInt(row, &end);

    ajSqlcolumnToInt(row, &i);
    if( i == 1 )
	strand = '+';
    else if( i == -1 )
	strand = '-';
    else
	strand = '\0';		/* change to \0 later */

    ajSqlcolumnToInt(row, &frame);

    ajSqlcolumnToStr(row, &type);
    ajSqlcolumnToStr(row, &sourcedb);
    ajSqlcolumnToStr(row, &sourceprogram);
    ajSqlcolumnToFloat(row, &score);
    /*ajSqlcolumnToStr(row, &alias);*/
    ajSqlcolumnToTime(row, &timelm);
    ajSqlcolumnToBool(row, &isObsolete);

    if(ajStrGetLen(sourcedb))
	ajStrAssignS(&source, sourcedb);
    else if(ajStrGetLen(sourceprogram))
	ajStrAssignS(&source, sourceprogram);
    else
	source = ajStrNewC(".");

    ajListPeekLast(fttab->Features, (void**)&prevft);

    if(++start==(ajint)prevft->Start && end==(ajint)prevft->End)
    {
	ajListPeekFirst(prevft->GffTags, (void**)&idtag);
	if(ajStrMatchS(entryid, MAJTAGVALGETVALUE(idtag)))
	{
	    updateprevft = ajTrue;
	    feature = prevft;
	}
    }

    if(!updateprevft)
    {
	feature = ajFeatNewNuc(fttab,
	                       source,
	                       type,
	                       start,
	                       end,
	                       score,
	                       strand,
	                       frame,
	                       0,0,0, NULL, NULL);

	ajFeatGfftagAddCS(feature, "ID", entryid);
	ajFeatGfftagAddCS(feature, "Name", name);
	/*ajFeatGfftagAddC(feature, "Alias", alias);*/

	/* FIXME: it looks we replace all type 'synonyms' with SO:0000110
	 * following tag is a workaround until it is fixed */
	if(ajStrMatchC(feature->Type, "SO:0000110"))
	    ajFeatGfftagAddCS(feature, "type", type);
    }

    if(ajStrMatchC(relation, "part_of") ||
	    ajStrMatchC(relation, "proper_part_of") ||
	    ajStrMatchC(relation, "partof") ||
	    ajStrMatchC(relation, "producedby"))
    {
	ajFeatGfftagAddCS(feature, "Parent", object);
	ajFeatGfftagAddCS(feature, "parent_type", objecttype);
    }
    else if(ajStrMatchC(relation, "derives_from"))
    {
	ajFeatGfftagAddCS(feature, "Derives_from", object);
	ajFeatGfftagAddCS(feature, "parent_type", objecttype);
    }
    else if(ajStrGetLen(relation))
    {
	ajFeatGfftagAddSS(feature, relation, object);
	ajFeatGfftagAddCS(feature, "object_type", objecttype);
    }


    if(!updateprevft)
    {
	if(timelm)
	{
	    ajFmtPrintS(&timelmS,"%D",timelm);
	    ajFeatGfftagAddCS(feature,"timelastmodified", timelmS);
	}

	if(isObsolete)
	{
	    isObsoleteS = ajStrNewC("true");
	    ajFeatGfftagAddCS(feature,"isObsolete", isObsoleteS);
	    ajStrDel(&isObsoleteS);
	}
    }

    ajFeatTrace(feature);

    ajStrDel(&name);
    ajStrDel(&entryid);
    ajStrDel(&relation);
    ajStrDel(&object);
    ajStrDel(&objecttype);
    ajStrDel(&type);
    ajStrDel(&source);
    ajStrDel(&sourcedb);
    ajStrDel(&sourceprogram);
    ajStrDel(&alias);
    ajTimeDel(&timelm);
    ajStrDel(&timelmS);

    return feature;
}




/* @funcstatic featChadoQryfeatureRow *****************************************
**
** Generates a new feature from chado query-feature query result row.
** Checks whether the query feature is located on another feature (seq region)
**
** @param [u] fttab    [AjPFeattable] Feature table
** @param [u] row      [AjPSqlrow]    Input row
** @param [r] qrystart [ajint]        Query start position
** @param [r] qryend   [ajint]        Query end position
** @return [AjPStr] New feature ID
**
** @release 6.4.0
** @@
******************************************************************************/

static AjPStr featChadoQryfeatureRow(AjPFeattable fttab, AjPSqlrow row,
	                             ajint qrystart, ajint qryend)
{
    AjPFeature gf  = NULL;
    AjPStr name   = NULL;

    AjPStr source = NULL;
    AjPStr type   = NULL;
    ajint start  = 1;
    ajint seqlen = 0;
    float score  = 0;
    char  strand = '+';
    ajint loccount  = 0; /* this is to be removed; initially thought
                            we can use location count to decide whether
                            the query feature is a sequence region,
                            however current code decides it by checking whether
                            a parent location is defined */
    ajint i = 0;
    ajint frame = 0;
    AjPStr uniquename = NULL;
    AjPStr alias = NULL;
    AjPStr regionid = NULL;
    AjPStr regionuniquename = NULL;
    ajint regionstart  = 1;
    ajint regionend    = 0;
    AjPTime timelm = NULL;
    AjPStr timelmS = NULL;
    AjBool isObsolete = ajFalse;
    AjPStr isObsoleteS = NULL;

    if(!ajSqlrowGetColumns(row))
	return NULL;

    timelm = ajTimeNew();

    ajSqlcolumnToStr(row, &name);
    ajSqlcolumnToStr(row, &uniquename);
    ajSqlcolumnToInt(row, &loccount);
    ajSqlcolumnToStr(row, &regionid);
    ajSqlcolumnToStr(row, &regionuniquename);
    ajSqlcolumnToInt(row, &regionstart);
    ajSqlcolumnToInt(row, &regionend);
    ajDebug("location count:%d  parent:%S parent-id:%S"
	    " region start:%d  region end:%d\n",
	    loccount,
	    regionuniquename, regionid, regionstart, regionend);

    ajSqlcolumnToInt(row, &i);
    if( i == 1 )
	strand = '+';
    else if( i == -1 )
	strand = '-';
    else
	strand = '\0';

    ajSqlcolumnToInt(row, &frame);

    ajSqlcolumnToInt(row, &seqlen);

    ajSqlcolumnToStr(row, &type);
    ajSqlcolumnToStr(row, &source);
    /*ajSqlcolumnToStr(row, &alias);*/
    ajSqlcolumnToTime(row, &timelm);
    ajSqlcolumnToBool(row, &isObsolete);

    if(!ajStrGetLen(source))
	ajStrAssignS(&source, fttab->Db);


    if(qryend)
    {
	start = qrystart;
	seqlen = qryend;
    }

    regionstart++;

    gf = ajFeatNewNuc(fttab, source, type,
                      (ajStrGetLen(regionuniquename) ? regionstart : start),
                      (ajStrGetLen(regionuniquename) ? regionend   : seqlen),
                      score,
                      strand,
                      frame,
                      0,0,0, NULL, NULL);

    ajFeatGfftagAddCS(gf, "ID", uniquename);
    ajFeatGfftagAddCS(gf, "Name", name);
    /*ajFeatGfftagAddC(gf, "Alias", alias);*/

    if(timelm)
    {
	ajFmtPrintS(&timelmS,"%D",timelm);
	ajFeatGfftagAddCS(gf,"timelastmodified", timelmS);
    }

    if(isObsolete)
    {
	isObsoleteS = ajStrNewC("true");
	ajFeatGfftagAddCS(gf,"isObsolete", isObsoleteS);
	ajStrDel(&isObsoleteS);
    }

    ajFeatTrace(gf);

    ajStrDel(&name);
    ajStrDel(&type);
    ajStrDel(&source);
    ajStrDel(&alias);
    ajTimeDel(&timelm);
    ajStrDel(&timelmS);
    ajStrDel(&regionid);

    if(ajStrGetLen(regionuniquename))
    {
	if(loccount)
	    ajDebug("Feature '%S' is used as a source feature for %d other"
		    " feature(s) but it is itself located on feature '%S';"
		    " making queries on the parent feature '%S'\n",
		    uniquename, loccount, regionuniquename, regionuniquename);
	ajStrAssignS(&fttab->Seqid, regionuniquename);

	ajStrDel(&uniquename);
	return regionuniquename;
    }

    ajStrDel(&regionuniquename);

    return uniquename;
}




/* @funcstatic featChadoConnect ***********************************************
**
** Connects to the chado database required by the query
**
** @param [r] qry [const AjPQuery] Query object
** @return [AjPSqlconnection] SQL Database connection
**
** @release 6.4.0
** @@
******************************************************************************/

static AjPSqlconnection featChadoConnect(const AjPQuery qry)
{
    AjESqlconnectionClient client;

    ajint iport = 3306;

    AjPStr url = NULL;
    AjPUrlref uo  = NULL;

    AjPStr password   = NULL;
    AjPStr socketfile = NULL;

    AjPSqlconnection connection = NULL;

    url = ajStrNew();

    if(!ajNamDbGetUrl(qry->DbName, &url))
    {
	ajErr("no URL defined for database %S", qry->DbName);

	return ajFalse;
    }

    uo = ajHttpUrlrefNew();

    ajHttpUrlrefParseC(&uo, ajStrGetPtr(url));
    ajHttpUrlrefSplitPort(uo);
    ajHttpUrlrefSplitUsername(uo);

    if(ajStrMatchCaseC(uo->Method,"mysql"))
        client = ajESqlconnectionClientMySQL;
    else if(ajStrMatchCaseC(uo->Method,"postgresql"))
    {
        client = ajESqlconnectionClientPostgreSQL;
        iport = 5432;
    }
    else
        client = ajESqlconnectionClientNULL;

    if(!ajStrGetLen(uo->Port))
        ajFmtPrintS(&uo->Port,"%d",iport);

    if(ajStrGetLen(uo->Password))
    {
        password = ajStrNew();
        ajStrAssignS(&password,uo->Password);
    }

    connection = ajSqlconnectionNewData(client,uo->Username,password,
                                        uo->Host,uo->Port,socketfile,
                                        uo->Absolute);

    ajStrDel(&password);

    if(!connection)
        ajErr("Could not connect to database server");

    ajStrDel(&url);
    ajHttpUrlrefDel(&uo);

    return connection;
}




/* @section DAS ***************************************************************
**
** These functions manage the DAS database access methods.
**
******************************************************************************/




/* @funcstatic featAccessDas **************************************************
**
** Feature access method for DAS feature sources
**
** @param [u] fttabin [AjPFeattabin] Feature input.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool featAccessDas(AjPFeattabin fttabin)
{
    AjPStr host         = NULL;
    AjPStr path         = NULL;
    AjIList iter        = NULL;
    AjPQueryField field = NULL;
    AjPQuery qry        = NULL;
    AjPTextin textin    = NULL;
    AjPStr dasqueryurl  = NULL;

    AjBool ret = ajTrue;
    ajint port = 80;

    textin = fttabin->Input;

    qry = textin->Query;

    if(qry->QryDone)
	return ajFalse;

    if(!ajHttpQueryUrl(qry, &port, &host, &path))
    {
	ajStrDel(&host);
	ajStrDel(&path);

	return ajFalse;
    }

    if(ajStrGetCharLast(path)!='/')
	ajStrAppendK(&path,'/');

    dasqueryurl = ajStrNew();

    iter = ajListIterNewread(qry->QueryFields);

    while(!ajListIterDone(iter))
    {
	field = ajListIterGet(iter);

	if(ajStrMatchCaseC(field->Field, "id"))
	{
	    if(!ajStrGetLen(dasqueryurl))
		ajFmtPrintS(&dasqueryurl,"segment=%S",
		            field->Wildquery);
	    else
		ajFmtPrintS(&dasqueryurl,"%S;segment=%S",
		            dasqueryurl,
		            field->Wildquery);

	    /* TODO: segment specific start,end positions */
	    if(fttabin->End > 0)
		ajFmtPrintS(&dasqueryurl,"%S:%u,%u",
		            dasqueryurl,fttabin->Start,fttabin->End);
	}
	else {
	    if(!ajStrGetLen(dasqueryurl))
		ajFmtPrintS(&dasqueryurl,"%S=%S",
		            field->Field,
		            field->Wildquery);
	    else
		ajFmtPrintS(&dasqueryurl,"%S;%S=%S",
		            dasqueryurl,
		            field->Field,
		            field->Wildquery);

	    /* TODO: segment specific start,end positions */
	    if(fttabin->End > 0)
		ajFmtPrintS(&dasqueryurl,"%S:%u,%u",
		            dasqueryurl,fttabin->Start,fttabin->End);
	}

    }

    ajFmtPrintS(&path,"%Sfeatures?%S",path, dasqueryurl);

    ajFilebuffDel(&textin->Filebuff);
    textin->Filebuff = ajHttpRead(qry->DbHttpVer, qry->DbName, qry->DbProxy,
                                  host, port, path);

    if (textin->Filebuff)
	ajFilebuffHtmlNoheader(textin->Filebuff);
    else
	ret = ajFalse;

    qry->QryDone = ajTrue;

    ajStrDel(&host);
    ajStrDel(&path);
    ajStrDel(&dasqueryurl);

    ajListIterDel(&iter);

    return ret;
}




/* @func ajFeatdbPrintAccess **************************************************
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

void ajFeatdbPrintAccess(AjPFile outf, AjBool full)
{
    ajint i = 0;

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# Feature access methods\n");
    ajFmtPrintF(outf, "# Name       Alias Entry Query   All Description\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "method {\n");

    for(i=0; feattabAccess[i].Name; i++)
	if(full || !feattabAccess[i].Alias)
	    ajFmtPrintF(outf, "  %-10s %5B %5B %5B %5B \"%s\"\n",
			feattabAccess[i].Name,  feattabAccess[i].Alias,
			feattabAccess[i].Entry, feattabAccess[i].Query,
			feattabAccess[i].All,   feattabAccess[i].Desc);

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajFeatdbExit *********************************************************
**
** Cleans up feature database processing internal memory
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajFeatdbExit(void)
{
    ajRegFree(&featCdDivExp);
    ajCharDel(&featCdName);
    ajRegFree(&featRegGcgId);
    ajRegFree(&featRegGcgCont);
    ajRegFree(&featRegGcgId2);
    ajRegFree(&featRegGcgSplit);
    ajRegFree(&featRegGcgRefId);

    return;
}

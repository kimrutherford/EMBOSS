/* @source ajseqdb ************************************************************
**
** AJAX SEQDB (database) functions
**
** These functions control all aspects of AJAX sequence database access
**
** @author Copyright (C) 1998 Peter Rice
** @author Copyright (C) 2005 Alan Bleasby
** @version $Revision: 1.87 $
** @modified Jun 25 pmr First version
** @modified Apr 2005 ajb B+tree code addition
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
******************************************************************************/

#include <limits.h>
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
#include <ws2tcpip.h>
#endif
#include <errno.h>
#include <signal.h>


#include "ajdas.h"
#include "ajseqdb.h"
#include "ajmart.h"
#include "ensembl.h"





static AjPRegexp seqBlastDivExp = NULL;
static AjPRegexp seqCdDivExp = NULL;
static AjPRegexp seqRegHttpUrl = NULL;

static AjPRegexp seqRegGcgId = NULL;
static AjPRegexp seqRegGcgCont = NULL;
static AjPRegexp seqRegGcgId2 = NULL;
static AjPRegexp seqRegGcgSplit = NULL;

static AjPRegexp seqRegEntrezCount = NULL;
static AjPRegexp seqRegEntrezId = NULL;

static AjPRegexp seqRegGcgRefId = NULL;

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
**                         entrynam.idx file
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
** @attr DivCode [ajushort] Division file record
** @attr Padding [char[6]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct SeqSCdIdx
{
    ajuint AnnOffset;
    ajuint SeqOffset;
    AjPStr EntryName;
    ajushort DivCode;
    char Padding[6];
} SeqOCdIdx;

#define SeqPCdIdx SeqOCdIdx*




/* @datastatic SeqPCdTrg ******************************************************
**
** EMBLCD target (.trg) file record structure
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
** @attr Skip [AjBool*] skip file(s) in division.lkp
** @attr idxLine [SeqPCdIdx] entryname.idx input line
** @attr Samefile [AjBool] true if the same file is passed to
**                         ajFilebuffReopenFile
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

typedef struct SeqSEmbossQry
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
} SeqOEmbossQry;

#define SeqPEmbossQry SeqOEmbossQry*




/* @enumstatic SeqEEnsemblType ************************************************
**
** Ensembl Object Type enumeration
**
** @value seqEEnsemblTypeNULL        Null
** @value seqEEnsemblTypeExon        Exon
** @value seqEEnsemblTypeGene        Gene
** @value seqEEnsemblTypeTranscript  Transcript
** @value seqEEnsemblTypeTranslation Translation
******************************************************************************/

typedef enum SeqOEnsemblType
{
    seqEEnsemblTypeNULL,
    seqEEnsemblTypeExon,
    seqEEnsemblTypeGene,
    seqEEnsemblTypeTranscript,
    seqEEnsemblTypeTranslation
} SeqEEnsemblType;




/* @datastatic SeqPEnsembl ****************************************************
**
** Ensembl query structure
**
** @alias SeqSEnsembl
** @alias SeqOEnsembl
**
** @attr Stableidentifiers [AjPList] AJAX List of AJAX String
**                                   (Ensembl stable identifier) objects
** @attr Alias [AjPStr] Species alias parsed from the USA
** @attr Database [AjPStr] The database name parsed from the USA
** @attr Wildquery [AjPStr] AJAX Query Field Wild Query representing
** @attr Species [AjPStr] Species resolved from an alias
** Ensembl stable identifier
** @attr Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @attr EnsemblType [SeqEEnsemblType] Ensembl Object Type enumeration
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct SeqSEnsembl
{
    AjPList Stableidentifiers;
    AjPStr Alias;
    AjPStr Database;
    AjPStr Wildquery;
    AjPStr Species;
    EnsPDatabaseadaptor Databaseadaptor;
    SeqEEnsemblType EnsemblType;
    ajuint Padding;
} SeqOEnsembl;

#define SeqPEnsembl SeqOEnsembl*




static AjBool     seqAccessBlast(AjPSeqin seqin);
/* static AjBool     seqAccessCmd(AjPSeqin seqin);*/ /* not implemented */
static AjBool     seqAccessDAS(AjPSeqin seqin);
static AjBool     seqAccessEmbossGcg(AjPSeqin seqin);
static AjBool     seqAccessEnsembl(AjPSeqin seqin);
static AjBool     seqAccessGcg(AjPSeqin seqin);
static AjBool     seqAccessMart(AjPSeqin seqin);
/* static AjBool     seqAccessNbrf(AjPSeqin seqin); */ /* obsolete */
static AjBool     seqAccessSql(AjPSeqin seqin);

static AjBool     seqBlastOpen(AjPQuery qry, AjBool next);
static ajuint     seqCdDivNext(AjPQuery qry);
static void       seqCdIdxDel(SeqPCdIdx* pthys);
static void       seqCdTrgDel(SeqPCdTrg* pthys);
static AjBool     seqBlastAll(AjPSeqin seqin);
static AjPFile    seqBlastFileOpen(const AjPStr dir, const AjPStr name);
static AjBool     seqBlastLoadBuff(AjPSeqin seqin);
static AjBool     seqBlastQryClose(AjPQuery qry);
static AjBool     seqBlastQryNext(AjPQuery qry);
static AjBool     seqBlastReadTable(AjPSeqin seqin,
                                    AjPStr* hline, AjPStr* seq);
static void       seqBlastStripNcbi(AjPStr* line);

static int        seqCdEntryCmp(const void* a, const void* b);
static void       seqCdEntryDel(void** pentry, void* cl);
static void       seqCdFileClose(SeqPCdFile *thys);
static SeqPCdFile seqCdFileOpen(const AjPStr dir, const char* name,
                                AjPStr* fullname);
static ajint      seqCdFileSeek(SeqPCdFile fil, ajuint ipos);
static void       seqCdIdxLine(SeqPCdIdx idxLine,  ajuint ipos,
                               SeqPCdFile fp);
static char*      seqCdIdxName(ajuint ipos, SeqPCdFile fp);
static AjBool     seqCdIdxQuery(AjPQuery qry, const AjPStr idqry);
static ajuint     seqCdIdxSearch(SeqPCdIdx idxLine, const AjPStr entry,
                                 SeqPCdFile fp);
static AjBool     seqCdQryClose(AjPQuery qry);
static AjBool     seqCdQryEntry(AjPQuery qry);
static AjBool     seqCdQryFile(AjPQuery qry);
static AjBool     seqCdQryOpen(AjPQuery qry);
static AjBool     seqCdQryNext(AjPQuery qry);
static AjBool     seqCdQryQuery(AjPQuery qry);
static AjBool     seqCdQryReuse(AjPQuery qry);
static AjBool     seqCdReadHeader(SeqPCdFile fp);
static AjBool     seqCdTrgClose(SeqPCdFile *trgfil, SeqPCdFile *hitfil);
static ajuint     seqCdTrgFind(AjPQuery qry, const char* indexname,
                               const AjPStr qrystring);
static void       seqCdTrgLine(SeqPCdTrg trgLine, ajuint ipos,
                               SeqPCdFile fp);
static char*      seqCdTrgName(ajuint ipos, SeqPCdFile fp);
static AjBool     seqCdTrgOpen(const AjPStr dir, const char* name,
                               SeqPCdFile *trgfil, SeqPCdFile *hitfil);
static AjBool     seqCdTrgQuery(AjPQuery qry, const AjPStr field,
                                const AjPStr wildqry);
static ajuint     seqCdTrgSearch(SeqPCdTrg trgLine, const AjPStr name,
                                 SeqPCdFile fp);

static AjBool     seqEmbossGcgAll(AjPSeqin seqin);
static void       seqEmbossGcgLoadBuff(AjPSeqin seqin);
static AjBool     seqEmbossGcgReadRef(AjPSeqin seqin);
static AjBool     seqEmbossGcgReadSeq(AjPSeqin seqin);

static AjBool     seqEmbossOpenCache(AjPQuery qry, const char *ext,
                                     AjPBtcache *cache);
static AjBool     seqEmbossQryClose(AjPQuery qry);
static AjBool     seqEmbossQryEntry(AjPQuery qry);
static AjBool     seqEmbossQryNext(AjPQuery qry);
static AjBool     seqEmbossQryOpen(AjPQuery qry);
static AjBool     seqEmbossQryOrganisms(AjPQuery qry);
static AjBool     seqEmbossQryQuery(AjPQuery qry);
static AjBool     seqEmbossQryReuse(AjPQuery qry);

static void       seqEnsemblDel(SeqPEnsembl* Pse);
static AjBool     seqEnsemblQryFirst(AjPQuery qry);
static AjBool     seqEnsemblQryFirstExon(AjPQuery qry);
static AjBool     seqEnsemblQryFirstGeneric(AjPQuery qry);
static AjBool     seqEnsemblQryFirstTranscript(AjPQuery qry);
static AjBool     seqEnsemblQryFirstTranslation(AjPQuery qry);

static AjBool     seqGcgAll(AjPSeqin seqin);
static void       seqGcgBinDecode(AjPStr *pthis, ajuint rdlen);
static void       seqGcgLoadBuff(AjPSeqin seqin);
static AjBool     seqGcgReadRef(AjPSeqin seqin);
static AjBool     seqGcgReadSeq(AjPSeqin seqin);




/* @funclist seqAccess ********************************************************
**
** Functions to access each database or sequence access method
**
******************************************************************************/

static AjOSeqAccess seqAccess[] =
{
    /* Name     AccessFunction   FreeFunction
       Qlink    Description
       Alias    Entry    Query    All      Chunk  Padding */
    /* {
        "dbfetch", seqAccessDbfetch, NULL,
        "",      "retrieve in text format from EBI dbfetch REST services",
	AJFALSE, AJTRUE,  AJFALSE, AJFALSE, AJFALSE, AJFALSE
	}, */
    /* {
       "asis",  &ajTextAccessAsis, NULL,
       "",      "",
       AJFALSE, AJTRUE, AJFALSE, AJFALSE, AJFALSE, AJFALSE
       }, */ /* called by seqUsaProcess */
    /* {
       "file",  &ajTextAccessFile, NULL,
       "",      "",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE, AJFALSE, AJFALSE
       }, */ /* called by seqUsaProcess */
    /* {
       "offset", &ajTextAccessOffset, NULL,
       "",      "",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE, AJFALSE, AJFALSE
       }, */ /* called by seqUsaProcess */
    {
      "gcg",   &seqAccessGcg, NULL,
      "|&!^=", "emboss dbigcg indexed",
      AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  AJFALSE, AJFALSE
    },
    {
      "embossgcg", &seqAccessEmbossGcg, NULL,
      "|&!^=", "emboss dbxgcg indexed",
      AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  AJFALSE, AJFALSE
    },
    {
      "blast",    &seqAccessBlast, NULL,
      "",      "blast database format version 2 or 3",
      AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  AJFALSE, AJFALSE
    },
    {
      "biomart",  &seqAccessMart, NULL,
      "",      "retrieve a single entry from a BioMart server",
      AJFALSE, AJTRUE,  AJFALSE,  AJFALSE, AJFALSE, AJFALSE
    },
    {
      "sql",      &seqAccessSql, NULL,
      "",      "retrieve from a SQL server",
      AJFALSE, AJTRUE,  AJTRUE,  AJTRUE, AJFALSE, AJFALSE
    },
    {
      "ensembl",  &seqAccessEnsembl, NULL,
      "",      "retrieve a single entry from an Ensembl Database server",
      AJFALSE, AJTRUE, AJFALSE, AJFALSE, AJFALSE, AJFALSE
    },
    {
      "ensemblgenomes", &seqAccessEnsembl, NULL,
      "",      "retrieve a single entry from an Ensemblgenomes Database server",
      AJFALSE, AJTRUE, AJFALSE, AJFALSE, AJFALSE, AJFALSE
    },
    {
      "das",      &seqAccessDAS, NULL,
      "|",     "retrieve sequence entries from a DAS source",
      AJFALSE, AJTRUE,  AJFALSE,  AJFALSE, AJFALSE, AJFALSE
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

static char aa_btoa[27] = {"-ARNDCQEGHILKMFPSTWYVBZX*"};
static char aa_btoa2[27]= {"-ABCDEFGHIKLMNPQRSTVWXYZ*"};




/* @func ajSeqdbInit **********************************************************
**
** Initialise sequence database internals
**
** @return [void]
**
** @release 6.2.0
******************************************************************************/

void ajSeqdbInit(void)
{
    AjPTable table;
    ajuint i = 0;

    table = ajSeqaccessGetDb();

    while(seqAccess[i].Name)
    {
        ajCallTableRegister(table, seqAccess[i].Name, (void*) &seqAccess[i]);
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




/* @funcstatic seqBlastFileOpen ***********************************************
**
** Opens a named BLAST index file.
**
** @param [r] dir [const AjPStr] Directory
** @param [r] name [const AjPStr] File name.
** @return [AjPFile] file object.
**
** @release 1.0.0
** @@
******************************************************************************/

static AjPFile seqBlastFileOpen(const AjPStr dir, const AjPStr name)
{
    AjPFile thys;

    thys = ajFileNewInNamePathS(name, dir);

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
**
** @release 1.0.0
** @@
******************************************************************************/

static SeqPCdFile seqCdFileOpen(const AjPStr dir, const char* name,
                                AjPStr* fullname)
{
    SeqPCdFile thys = NULL;


    AJNEW0(thys);

    thys->File = ajFileNewInNamePathC(name, dir);

    if(!thys->File)
    {
        AJFREE(thys);

        return NULL;
    }


    AJNEW0(thys->Header);

    seqCdReadHeader(thys);
    thys->NRecords = thys->Header->NRecords;
    thys->RecSize = thys->Header->RecSize;

    ajStrAssignS(fullname, ajFileGetPrintnameS(thys->File));

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
**
** @release 1.0.0
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
      ipos, jpos, ajFileResetPos(fil->File), ret);
    */

    return ret;
}




/* @funcstatic seqCdFileClose *************************************************
**
** Closes an EMBL CD-ROM index file.
**
** @param [d] pthis [SeqPCdFile*] EMBL CD-ROM index file.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void seqCdFileClose(SeqPCdFile* pthis)
{
    SeqPCdFile thys;

    thys = *pthis;

    if(!thys)
        return;

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
**
** @release 1.0.0
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

    seqCdIdxLine(idxLine, ipos, fil);

    return ipos;
}




/* @funcstatic seqCdIdxQuery **************************************************
**
** Binary search of an EMBL CD-ROM index file for entries matching a
** wildcard entry name.
**
** @param [u] qry [AjPQuery] Sequence query object.
** @param [r] idqry [const AjPStr] ID Query
** @return [AjBool] ajTrue on success.
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool seqCdIdxQuery(AjPQuery qry, const AjPStr idqry)
{
    SeqPCdQry qryd;

    AjPList list;
    SeqPCdIdx idxLine;
    SeqPCdFile fil;

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

    SeqPCdEntry entry;

    qryd    = qry->QryData;
    list    = qry->ResultsList;
    idxLine = qryd->idxLine;
    fil     = qryd->ifp;

    ajStrAssignS(&idstr,idqry);
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
    {                          /* find first entry with this prefix */
        while(ilo <= ihi)
        {
            ipos = (ilo + ihi)/2;
            name = seqCdIdxName(ipos, fil);
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
            name = seqCdIdxName(ipos, fil);
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
**
** @release 1.0.0
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
**
** @release 1.0.0
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
    ajReadbinCharTrim(fil->File, nameSize, seqCdName);

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
**
** @release 1.0.0
** @@
******************************************************************************/

static void seqCdIdxLine(SeqPCdIdx idxLine, ajuint ipos, SeqPCdFile fil)
{
    ajuint nameSize;

    nameSize = fil->RecSize-10;

    ajDebug("seqCdIdxLine nameSize: %u max: %u ipos: %u '%F'\n",
            nameSize, seqCdMaxNameSize, ipos, fil->File);

    if(seqCdMaxNameSize < nameSize)
    {
        seqCdMaxNameSize = nameSize;

        if(seqCdName)
            ajCharDel(&seqCdName);

        seqCdName = ajCharNewRes(seqCdMaxNameSize+1);
    }

    seqCdFileSeek(fil, ipos);
    ajReadbinCharTrim(fil->File, nameSize, seqCdName);

    ajStrAssignC(&idxLine->EntryName,seqCdName);

    ajReadbinUint(fil->File, &idxLine->AnnOffset);
    ajReadbinUint(fil->File, &idxLine->SeqOffset);
    ajReadbinUint2(fil->File, &idxLine->DivCode);

    ajDebug("read ann: %u seq: %u div: %u\n",
            idxLine->AnnOffset, idxLine->SeqOffset,
            (ajuint) idxLine->DivCode);
    return;
}




/* @funcstatic seqCdTrgName ***************************************************
**
** Reads the target name from an EMBL CD-ROM index target file.
**
** @param [r] ipos [ajuint] Record number.
** @param [u] fil [SeqPCdFile] EMBL CD-ROM index target file.
** @return [char*] Name.
**
** @release 1.0.0
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
    ajReadbinInt(fil->File, &i);
    ajReadbinInt(fil->File, &i);
    ajReadbinCharTrim(fil->File, nameSize, seqCdName);

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
**
** @release 1.0.0
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

    ajReadbinUint(fil->File, &trgLine->NHits);
    ajReadbinUint(fil->File, &trgLine->FirstHit);
    ajReadbinCharTrim(fil->File, nameSize, seqCdName);

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
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool seqCdReadHeader(SeqPCdFile fil)
{
    ajint i;

    SeqPCdFHeader header;
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
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool seqCdTrgOpen(const AjPStr dir, const char* name,
                           SeqPCdFile* trgfil, SeqPCdFile* hitfil)
{
    AjPStr tmpname  = NULL;
    AjPStr fullname = NULL;

    ajDebug("seqCdTrgOpen dir '%S' name '%s'\n",
            dir, name);

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
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool seqCdTrgClose(SeqPCdFile* ptrgfil, SeqPCdFile* phitfil)
{
    seqCdFileClose(ptrgfil);
    seqCdFileClose(phitfil);

    return ajTrue;
}




/* @section B+tree Database Indexing *****************************************
**
** These functions manage the B+tree index access methods.
**
******************************************************************************/




/* @funcstatic seqEmbossQryReuse **********************************************
**
** Tests whether the B+tree index query data can be reused or it's finished.
**
** Clears qryData structure when finished.
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if reusable,
**                  ajFalse if finished.
**
** @release 3.0.0
** @@
******************************************************************************/

static AjBool seqEmbossQryReuse(AjPQuery qry)
{
    SeqPEmbossQry qryd;

    qryd = qry->QryData;

    if(!qry || !qryd)
        return ajFalse;


    if(!qry->ResultsList)
    {
        ajDebug("seqEmbossQryReuse: query data all finished\n");

        return ajFalse;
    }
    else
    {
        ajDebug("seqEmbossQryReuse: reusing data from previous call %x\n",
                qry->QryData);
        /*ajListTrace(qry->ResultsList);*/
    }


    qryd->nentries = -1;


    return ajTrue;
}




/* @funcstatic seqEmbossQryOpen ***********************************************
**
** Open caches (etc) for B+tree search
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
**
** @release 3.0.0
** @@
******************************************************************************/

static AjBool seqEmbossQryOpen(AjPQuery qry)
{
    SeqPEmbossQry qryd;
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

    if(!seqEmbossOpenCache(qry,"id",&qryd->idcache))
        return ajFalse;

    iter = ajListIterNewread(qry->QueryFields);
    while(!ajListIterDone(iter))
    {
        field = ajListIterGet(iter);

        ajStrFmtLower(&field->Wildquery);
        if(!seqEmbossOpenCache(qry, MAJSTRGETPTR(field->Field), &cache))
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




/* @funcstatic seqEmbossOpenCache *********************************************
**
** Create primary B+tree index cache
**
** @param [u] qry [AjPQuery] Query data
** @param [r] ext [const char*] Index file extension
** @param [w] cache [AjPBtcache*] cache
** @return [AjBool] True on success
**
** @release 3.0.0
** @@
******************************************************************************/

static AjBool seqEmbossOpenCache(AjPQuery qry, const char *ext,
                                 AjPBtcache *cache)
{
    SeqPEmbossQry qryd;
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





/* @funcstatic seqEmbossQryEntry **********************************************
**
** Queries for a single entry in a B+tree index
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if can continue,
**                  ajFalse if all is done.
**
** @release 3.0.0
** @@
******************************************************************************/

static AjBool seqEmbossQryEntry(AjPQuery qry)
{
    AjPBtHitref newhit  = NULL;
    SeqPEmbossQry qryd;
    const AjPList fdlist;
    const AjPList cachelist;
    AjIList iter;
    AjIList icache;
    AjPBtcache cache;
    AjPQueryField fd;
    AjPBtHitref *allhits = NULL;
    ajuint i;

    ajDebug("seqEmbossQryEntry fields: %Lu hasacc:%B\n",
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

         seqEmbossQryOrganisms(qry);

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




/* @funcstatic seqEmbossQryNext ***********************************************
**
** Processes the next query for a B+tree index
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if successful
**
** @release 3.0.0
** @@
******************************************************************************/

static AjBool seqEmbossQryNext(AjPQuery qry)
{
    AjPBtHitref entry;
    SeqPEmbossQry qryd;
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




/* @funcstatic seqEmbossQryClose **********************************************
**
** Closes query data for a B+tree index
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
**
** @release 3.0.0
** @@
******************************************************************************/

static AjBool seqEmbossQryClose(AjPQuery qry)
{
    SeqPEmbossQry qryd;
    void* item;
    ajint i;
    ajuint iref;

    if(!qry)
        return ajFalse;

    if(!qry->QryData)
        return ajFalse;

    ajDebug("seqEmbossQryClose clean up qryd\n");

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




/* @funcstatic seqEmbossQryOrganisms ******************************************
**
** Restricts results to matches to organism(s) in database
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue on success.
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool seqEmbossQryOrganisms(AjPQuery qry)
{
    SeqPEmbossQry qryd;
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

    seqEmbossOpenCache(qry, "org", &orgcache);
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




/* @funcstatic seqEmbossQryQuery **********************************************
**
** Queries for one or more entries in an EMBOSS B+tree index
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
**
** @release 3.0.0
** @@
******************************************************************************/

static AjBool seqEmbossQryQuery(AjPQuery qry)
{
    SeqPEmbossQry qryd;

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

        if((fd->Link == AJQLINK_ELSE) && (lasthits > 0))
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
        seqEmbossQryOrganisms(qry);

    ajTableToarrayValues(qry->ResultsTable, (void***)&allhits);
    for(i=0; allhits[i]; i++)
        ajListPushAppend(qry->ResultsList, (void*) allhits[i]);

    AJFREE(allhits);

    ajTableClear(qry->ResultsTable);

    if(ajListGetLength(qry->ResultsList))
        return ajTrue;

    return ajFalse;
}




/* @section B+tree GCG Database Indexing *************************************
**
** These functions manage the EMBOSS B+tree GCG index access methods.
**
******************************************************************************/




/* @funcstatic seqAccessEmbossGcg *********************************************
**
** Reads sequence(s) from a GCG formatted database, using B+tree index
** files. Returns with the file pointer set to the position in the
** sequence file.
**
** @param [u] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
**
** @release 3.0.0
** @@
******************************************************************************/

static AjBool seqAccessEmbossGcg(AjPSeqin seqin)
{
    AjBool retval = ajFalse;

    AjPQuery qry;
    SeqPEmbossQry qryd = NULL;


    qry = seqin->Input->Query;
    qryd = qry->QryData;
    ajDebug("seqAccessEmbossGcg type %d\n", qry->QueryType);

    if(!ajNamDbGetDbalias(qry->DbName, &qry->DbAlias))
        ajStrAssignS(&qry->DbAlias, qry->DbName);

    if(qry->QueryType == AJQUERY_ALL)
        return seqEmbossGcgAll(seqin);


    if(!qry->QryData)
    {
        if(!seqEmbossQryOpen(qry))
            return ajFalse;

        qryd = qry->QryData;
        seqin->Input->Single = ajTrue;
        ajFilebuffDel(&seqin->Input->Filebuff);
        seqin->Input->Filebuff = ajFilebuffNewNofile();

        if(qry->QueryType == AJQUERY_ENTRY)
        {
            if(!seqEmbossQryEntry(qry))
                ajDebug("embossgcg B+tree Entry failed\n");
        }

        if(qry->QueryType == AJQUERY_QUERY)
        {
            if(!seqEmbossQryQuery(qry))
                ajDebug("embossgcg B+tree Query failed\n");
        }
    }
    else
    {
        if(!seqEmbossQryReuse(qry))
        {
            seqEmbossQryClose(qry);
            return ajFalse;
        }
        ajFilebuffClear(seqin->Input->Filebuff, -1);
    }

    if(ajListGetLength(qry->ResultsList))
    {
        retval = seqEmbossQryNext(qry);

        if(retval)
        {
            seqEmbossGcgLoadBuff(seqin);
            ajStrAssignS(&seqin->Input->Db, qry->DbName);
        }
    }

    if(!ajListGetLength(qry->ResultsList)) /* could be emptied by code above */
    {
        seqEmbossQryClose(qry);
        ajFileClose(&qryd->libs);
        ajFileClose(&qryd->libr);
    }

    return retval;
}




/* @funcstatic seqEmbossGcgAll ************************************************
**
** Opens the first or next GCG file for further reading
**
** @param [u] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
**
** @release 3.0.0
** @@
******************************************************************************/

static AjBool seqEmbossGcgAll(AjPSeqin seqin)
{
    AjPQuery qry;
    SeqPEmbossQry qryd;
    static ajint i   = 0;
    ajuint iref;
    AjPStr name      = NULL;
    AjBool ok        = ajFalse;
/*
  AjPStrTok handle = NULL;
  AjPStr wildname  = NULL;
  AjBool found     = ajFalse;
*/

    qry = seqin->Input->Query;
    qryd = qry->QryData;

    ajDebug("seqEmbossGcgAll\n");


    if(!qry->QryData)
    {
        ajDebug("seqEmbossGcgAll initialising\n");

        qry->QryData = AJNEW0(qryd);
        qryd = qry->QryData;
        i = -1;
        ajBtreeReadEntriesS(qry->DbAlias,qry->IndexDir,
                            qry->Directory,
                            &qryd->files,
                            &qryd->reffiles,
                            &qryd->refcount);

        seqin->Input->Single = ajTrue;
    }

    qryd = qry->QryData;
    ajFilebuffDel(&seqin->Input->Filebuff);
    seqin->Input->Filebuff = ajFilebuffNewNofile();

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
            ajDebug("seqEmbossGcgAll finished\n");
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
            ajDebug("seqEmbossGcgAll: cannot open sequence file\n");

            return ajFalse;
        }


        if(qryd->reffiles)
            qryd->libr = ajFileNewInNameS(qryd->reffiles[0][i]);

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




/* @funcstatic seqEmbossGcgLoadBuff *******************************************
**
** Copies text data to a buffered file, and sequence data for an
** AjPSeqin internal data structure for reading later
**
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [void]
**
** @release 3.0.0
** @@
******************************************************************************/

static void seqEmbossGcgLoadBuff(AjPSeqin seqin)
{
    AjPQuery qry;
    SeqPEmbossQry qryd;

    qry  = seqin->Input->Query;
    qryd = qry->QryData;

    ajDebug("seqEmbossGcgLoadBuff\n");

    if(!qry->QryData)
        ajFatal("seqEmbossGcgLoadBuff Query Data not initialised");

    /* copy all the ref data */

    seqEmbossGcgReadRef(seqin);

    /* write the sequence (do we care about the format?) */
    seqEmbossGcgReadSeq(seqin);

    /* ajFilebuffTraceFull(seqin->Input->Filebuff, 9999, 100); */

    if(!qryd->libr)
        ajFileClose(&qryd->libs);

    return;
}




/* @funcstatic seqEmbossGcgReadRef ********************************************
**
** Copies text data to a buffered file for reading later
**
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
**
** @release 3.0.0
** @@
******************************************************************************/

static AjBool seqEmbossGcgReadRef(AjPSeqin seqin)
{
    AjPStr line = NULL;
    AjPQuery qry;
    SeqPEmbossQry qryd;
    ajlong rpos;
    AjPStr id       = NULL;
    AjPStr idc      = NULL;
    AjBool ispir           = ajFalse;
    AjBool continued       = ajFalse;
    AjBool testcontinue    = ajFalse;
    char *p = NULL;

    qry  = seqin->Input->Query;
    qryd = qry->QryData;

    if(!seqRegGcgRefId)
        seqRegGcgRefId =ajRegCompC("^>...([^ \n]+)");

    if(!seqRegGcgSplit)
        seqRegGcgSplit =ajRegCompC("_0+$");

    if(!ajReadline(qryd->libr, &line))  /* end of file */
        return ajFalse;

    if(ajStrGetCharFirst(line) != '>')  /* not start of entry */
        ajFatal("seqGcgReadRef bad entry start:\n'%S'", line);

    if(ajStrGetCharPos(line, 3) == ';') /* PIR entry */
        ispir = ajTrue;

    if(ispir)
        ajFilebuffLoadS(seqin->Input->Filebuff, line);


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



    if(!ajReadline(qryd->libr, &line))  /* blank desc line */

        return ajFalse;

    if(ispir)
        ajFilebuffLoadS(seqin->Input->Filebuff, line);


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
        rpos = ajFileResetPos(qryd->libr);


        if(!testcontinue)
        {
            ajStrExchangeCC(&line, ". .", "..");
            ajFilebuffLoadS(seqin->Input->Filebuff, line);
        }
    }


    /* at end of file */

    ajFileClose(&qryd->libr);
    ajStrDel(&line);
    ajStrDel(&id);
    ajStrDel(&idc);

    return ajTrue;
}




/* @funcstatic seqEmbossGcgReadSeq ********************************************
**
** Copies sequence data with a reformatted sequence to the "Inseq"
** data structure of the AjPSeqin object for later reuse.
**
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
**
** @release 3.0.0
** @@
******************************************************************************/

static AjBool seqEmbossGcgReadSeq(AjPSeqin seqin)
{
    AjPStr line = NULL;
    AjPQuery qry;
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


    qry  = seqin->Input->Query;
    qryd = qry->QryData;

    if(!seqRegGcgId)
    {
        seqRegGcgId =ajRegCompC("^>...([^ ]+) +(Dummy Header|[^ ]+) +"
                                "([^ ]+) +([^ ]+) +([0-9]+)");
        seqRegGcgId2=ajRegCompC("^>[PF]1;([^ ]+)");
    }

    if(!seqRegGcgSplit)
        seqRegGcgSplit =ajRegCompC("_0+$");

    ajDebug("seqEmbossGcgReadSeq pos: %Ld\n", ajFileResetPos(qryd->libs));

    if(!ajReadline(qryd->libs, &line))  /* end of file */
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
            ajFilebuffLoadS(seqin->Input->Filebuff, line);
        }
    }
    else
    {
        ajStrSetRes(&seqin->Inseq, gcglen+3);
        rblock = gcglen;

        if(ajStrGetCharFirst(gcgtype) == '2')
            rblock = (rblock+3)/4;

        if(!ajReadbinBinary(qryd->libs, rblock, 1,
                            ajStrGetuniquePtr(&seqin->Inseq)))
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

        if(!ajReadline(qryd->libs, &line)) /* newline at end */
            ajFatal("error reading file %F", qryd->libs);

        if(continued)
        {
            spos = ajFileResetPos(qryd->libs);

            while(ajReadline(qryd->libs,&line))
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

                if(!ajReadline(qryd->libs, &line)) /* newline at end */
                    ajFatal("error reading file %F", qryd->libs);

                if(!seqRegGcgCont)
                    seqRegGcgCont = ajRegCompC("^([^ ]+) +([^ ]+) +([^ ]+) +"
                                               "([^ ]+) +([^ ]+) +([^ ]+) "
                                               "+([^ ]+) +"
                                               "([^ ]+) +([0-9]+)");

                ajRegExec(seqRegGcgCont, dstr);
                ajRegSubI(seqRegGcgCont, 9, &tmpstr);
                ajStrToInt(tmpstr, &pos);
                seqin->Inseq->Len = pos-1;

                ajStrAppendS(&seqin->Inseq,contseq);
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




/* @funcstatic seqCdQryReuse **************************************************
**
** Tests whether Cd index query data can be reused or whether we are finished.
**
** Clears qryData structure when finished.
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool seqCdQryReuse(AjPQuery qry)
{
    SeqPCdQry qryd;

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




/* @funcstatic seqCdQryOpen ***************************************************
**
** Opens everything for a new CD query
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool seqCdQryOpen(AjPQuery qry)
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

    ajDebug("directory '%S' fields: %Lu hasacc:%B\n",
            qry->IndexDir, ajListGetLength(qry->QueryFields), qry->HasAcc);

    qry->QryData = AJNEW0(qryd);
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
        ajReadbinInt2(qryd->dfp->File, &j);
        ajReadbinCharTrim(qryd->dfp->File, qryd->nameSize, name);

        ajStrAssignC(&fullName, name);
        ajFilenameReplacePathS(&fullName, qry->Directory);

        if(!ajFilenameTestInclude(fullName, qry->Exclude, qry->Filename))
            qryd->Skip[i] = ajTrue;
    }

    qryd->ifp = seqCdFileOpen(qry->IndexDir, "entrynam.idx", &qryd->idxfile);

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




/* @funcstatic seqCdQryEntry **************************************************
**
** Queries for a single entry in an EMBLCD index
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool seqCdQryEntry(AjPQuery qry)
{
    SeqPCdEntry entry = NULL;
    ajint ipos = -1;
    ajint trghit;
    SeqPCdQry qryd;
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

    ajDebug("seqCdQryEntry %S hasacc:%B\n",
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
                    ipos = seqCdIdxSearch(qryd->idxLine,
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
                    if(seqCdTrgOpen(qry->IndexDir, emblcdfields[i],
                                    &qryd->trgfp, &qryd->hitfp))
                    {
                        trghit = seqCdTrgSearch(qryd->trgLine,
                                                fd->Wildquery, qryd->trgfp);

                        if(trghit >= 0)
                        {
                            seqCdFileSeek(qryd->hitfp,
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
                                seqCdIdxLine(qryd->idxLine, j, qryd->ifp);

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

                        seqCdTrgClose(&qryd->trgfp, &qryd->hitfp);
                        ajStrDel(&qryd->trgLine->Target);
                    }
                }
                break;
            }
        }
        if(!embossfields[i])
        {
            if(seqCdTrgOpen(qry->IndexDir, MAJSTRGETPTR(fd->Field),
                            &qryd->trgfp, &qryd->hitfp))
            {
                trghit = seqCdTrgSearch(qryd->trgLine,
                                        fd->Wildquery, qryd->trgfp);

                if(trghit >= 0)
                {
                    seqCdFileSeek(qryd->hitfp,
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
                        seqCdIdxLine(qryd->idxLine, j, qryd->ifp);

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

                seqCdTrgClose(&qryd->trgfp, &qryd->hitfp);
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




/* @funcstatic seqCdQryQuery **************************************************
**
** Queries for one or more entries in an EMBLCD index
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool seqCdQryQuery(AjPQuery qry)
{
    AjIList iter = NULL;
    AjPQueryField field = NULL;

    SeqPCdEntry newhit;
    SeqPCdEntry *allhits = NULL;
    ajulong** keys = NULL;

    AjPTable newtable = NULL;

    ajuint i;
    ajulong lasthits = 0UL;
    ajulong fdhits = 0UL;

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
            seqCdIdxQuery(qry, field->Wildquery);
        else
            seqCdTrgQuery(qry, field->Field, field->Wildquery);

        fdhits = (ajuint) ajListGetLength(qry->ResultsList);

        ajDebug("seqCdQryQuery hits: %u link: %u\n",
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
                     &seqCdEntryCmp, &seqCdEntryDel);

    AJFREE(allhits);

    ajDebug("seqCdQryQuery clear results table\n");
    ajTableClear(qry->ResultsTable);

    if(ajListGetLength(qry->ResultsList))
        return ajTrue;

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
**
** @release 2.4.0
** @@
******************************************************************************/
static int seqCdEntryCmp(const void* pa, const void* pb)
{
    const SeqPCdEntry a;
    const SeqPCdEntry b;

    a = *(SeqPCdEntry const *) pa;
    b = *(SeqPCdEntry const *) pb;

    /*
      ajDebug("seqCdEntryCmp %x %d %d : %x %d %d\n",
      a, a->div, a->annoff,
      b, b->div, b->annoff);
    */
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
**
** @release 2.4.0
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
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if successful
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool seqCdQryNext(AjPQuery qry)
{
    SeqPCdEntry entry;
    SeqPCdQry qryd;
    void* item;

    qryd = qry->QryData;

    if(!ajListGetLength(qry->ResultsList))
        return ajFalse;

    ajDebug("qry->ResultsList (b) length %Lu\n",
            ajListGetLength(qry->ResultsList));
    /*ajListTrace(qry->ResultsList);*/
    ajListPop(qry->ResultsList, &item);
    entry = (SeqPCdEntry) item;

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
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if successful
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool seqBlastQryNext(AjPQuery qry)
{
    SeqPCdEntry entry;
    SeqPCdQry qryd;
    void* item;

    qryd = qry->QryData;

    if(!ajListGetLength(qry->ResultsList))
        return ajFalse;

    ajDebug("seqBlastQryNext qryd %x qry->ResultsList (c) length: %Lu\n",
            qryd, ajListGetLength(qry->ResultsList));

    /* ajListTrace(qry->ResultsList);*/

    ajListPop(qry->ResultsList, &item);
    entry = (SeqPCdEntry) item;

    ajDebug("entry: %X div: %d (%d) ann: %d seq: %d\n",
            entry, entry->div, qryd->div, entry->annoff, entry->seqoff);

    if(entry->div != qryd->div)
    {
        qryd->div = entry->div;
        seqBlastOpen(qry, ajFalse);     /* replaces qry->QryData */
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
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if all is done
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool seqCdQryClose(AjPQuery qry)
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
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
**
** @release 3.0.0
** @@
******************************************************************************/

static AjBool seqBlastQryClose(AjPQuery qry)
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
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool seqAccessGcg(AjPSeqin seqin)
{
    AjBool retval = ajFalse;
    AjPQuery qry;
    SeqPCdQry qryd;

    ajDebug("seqAccessGcg type %d\n", seqin->Input->Query->QueryType);

    qry  = seqin->Input->Query;
    qryd = qry->QryData;

    if(qry->QueryType == AJQUERY_ALL)
    {
        retval = seqGcgAll(seqin);

        return retval;
    }

    /* we need to search the index files and return a query */

    if(qry->QryData)
    {                                /* reuse unfinished query data */
        if(!seqCdQryReuse(qry))
            return ajFalse;
    }
    else
    {
        seqin->Input->Single = ajTrue;

        if(!seqCdQryOpen(qry))
        {
            ajWarn("Failed to open index for database '%S'",
                   qry->DbName);

            return ajFalse;
        }

        qryd = qry->QryData;
        ajFilebuffDel(&seqin->Input->Filebuff);
        seqin->Input->Filebuff = ajFilebuffNewNofile();

        /* binary search for the entryname we need */

        if(qry->QueryType == AJQUERY_ENTRY)
        {
            ajDebug("entry fields: %Lu hasacc:%B\n",
                    ajListGetLength(qry->QueryFields), qry->HasAcc);

            if(!seqCdQryEntry(qry))
                ajDebug("GCG Entry failed\n");
        }

        if(qry->QueryType == AJQUERY_QUERY)
        {
            ajDebug("query fields: %Lu hasacc:%B\n",
                    ajListGetLength(qry->QueryFields), qry->HasAcc);
            if(!seqCdQryQuery(qry))
                ajDebug("GCG Query failed\n");
        }

        AJFREE(qryd->trgLine);
    }

    if(ajListGetLength(qry->ResultsList))
    {
        retval = seqCdQryNext(qry);

        if(retval)
            seqGcgLoadBuff(seqin);
    }

    if(!ajListGetLength(qry->ResultsList))
    {
        ajFileClose(&qryd->libr);
        ajFileClose(&qryd->libs);
        seqCdQryClose(qry);
    }

    if(retval)
        ajStrAssignS(&seqin->Input->Db, qry->DbName);

    return retval;
}




/* @funcstatic seqGcgLoadBuff *************************************************
**
** Copies text data to a buffered file, and sequence data for an
** AjPSeqin internal data structure for reading later
**
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void seqGcgLoadBuff(AjPSeqin seqin)
{
    AjPQuery qry;
    SeqPCdQry qryd;

    qry  = seqin->Input->Query;
    qryd = qry->QryData;

    if(!qry->QryData)
        ajFatal("seqGcgLoadBuff Query Data not initialised");

    /* copy all the ref data */

    seqGcgReadRef(seqin);

    /* write the sequence (do we care about the format?) */
    seqGcgReadSeq(seqin);

    /* ajFilebuffTraceFull(seqin->Input->Filebuff, 9999, 100); */

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
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool seqGcgReadRef(AjPSeqin seqin)
{
    AjPStr line = NULL;
    AjPQuery qry;
    SeqPCdQry qryd;
    ajlong rpos;
    AjPStr id       = NULL;
    AjPStr idc      = NULL;
    AjBool ispir           = ajFalse;
    AjBool continued       = ajFalse;
    AjBool testcontinue    = ajFalse;
    char *p = NULL;

    qry  = seqin->Input->Query;
    qryd = qry->QryData;

    if(!seqRegGcgRefId)
        seqRegGcgRefId =ajRegCompC("^>...([^ \n]+)");

    if(!seqRegGcgSplit)
        seqRegGcgSplit =ajRegCompC("_0+$");

    if(!ajReadline(qryd->libr, &line))  /* end of file */
        return ajFalse;

    if(ajStrGetCharFirst(line) != '>')  /* not start of entry */
        ajFatal("seqGcgReadRef bad entry start:\n'%S'", line);

    if(ajStrGetCharPos(line, 3) == ';') /* PIR entry */
        ispir = ajTrue;

    if(ispir)
        ajFilebuffLoadS(seqin->Input->Filebuff, line);

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

    if(!ajReadline(qryd->libr, &line))  /* blank desc line */
    {
        ajStrDel(&id);

        return ajFalse;
    }

    if(ispir)
        ajFilebuffLoadS(seqin->Input->Filebuff, line);

    rpos = ajFileResetPos(qryd->libr);

    while(ajReadline(qryd->libr, &line))
    {                                   /* end of file */
        if(ajStrGetCharFirst(line) == '>')
        {                               /* start of next entry */
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

        rpos = ajFileResetPos(qryd->libr);

        if(!testcontinue)
        {
            ajStrExchangeCC(&line, ". .", "..");
            ajFilebuffLoadS(seqin->Input->Filebuff, line);
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
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool seqGcgReadSeq(AjPSeqin seqin)
{
    AjPStr line = NULL;
    AjPQuery qry;
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


    qry  = seqin->Input->Query;
    qryd = qry->QryData;

    if(!seqRegGcgId)
    {
        seqRegGcgId =ajRegCompC("^>...([^ ]+) +([^ ]+) +(Dummy Header|[^ ]+)"
                                " +([^ ]+) +([0-9]+)");
        seqRegGcgId2=ajRegCompC("^>[PF]1;([^ ]+)");
    }

    if(!seqRegGcgSplit)
        seqRegGcgSplit =ajRegCompC("_0+$");

    ajDebug("seqGcgReadSeq pos: %Ld\n", ajFileResetPos(qryd->libs));

    if(!ajReadline(qryd->libs, &line))  /* end of file */
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
            ajFilebuffLoadS(seqin->Input->Filebuff, line);
        }
    }
    else
    {
        ajStrSetRes(&seqin->Inseq, gcglen+3);
        rblock = gcglen;

        if(ajStrGetCharFirst(gcgtype) == '2')
            rblock = (rblock+3)/4;

        if(!ajReadbinBinary(qryd->libs, rblock, 1,
                            ajStrGetuniquePtr(&seqin->Inseq)))
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

        if(!ajReadline(qryd->libs, &line)) /* newline at end */
            ajFatal("error reading file %F", qryd->libs);

        if(continued)
        {
            spos = ajFileResetPos(qryd->libs);

            while(ajReadline(qryd->libs,&line))
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

                if(!ajReadline(qryd->libs, &line)) /* newline at end */
                    ajFatal("error reading file %F", qryd->libs);

                if(!seqRegGcgCont)
                    seqRegGcgCont = ajRegCompC("^([^ ]+) +([^ ]+) +([^ ]+) +"
                                               "([^ ]+) +([^ ]+) +([^ ]+) "
                                               "+([^ ]+) +"
                                               "([^ ]+) +([0-9]+)");

                ajRegExec(seqRegGcgCont, dstr);
                ajRegSubI(seqRegGcgCont, 9, &tmpstr);
                ajStrToInt(tmpstr, &pos);
                seqin->Inseq->Len = pos-1;

                ajStrAppendS(&seqin->Inseq,contseq);
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




/* @funcstatic seqGcgBinDecode ************************************************
**
** Convert GCG binary to ASCII sequence.
**
** @param [u] pthis [AjPStr*] Binary string
** @param [r] sqlen [ajuint] Expected sequence length
** @return [void]
**
** @release 1.0.0
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
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool seqGcgAll(AjPSeqin seqin)
{
    AjPQuery qry;
    SeqPCdQry qryd;

    qry = seqin->Input->Query;
    qryd = qry->QryData;

    ajDebug("seqGcgAll\n");

    if(!qry->QryData)
    {
        ajDebug("seqGcgAll initialising\n");
        seqin->Input->Single = ajTrue;

        if(!seqCdQryOpen(qry))
        {
            ajErr("seqGcgAll failed");

            return ajFalse;
        }
    }

    qryd = qry->QryData;
    ajFilebuffDel(&seqin->Input->Filebuff);
    seqin->Input->Filebuff = ajFilebuffNewNofile();

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
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool seqAccessBlast(AjPSeqin seqin)
{
    AjBool retval = ajFalse;
    AjPQuery qry;
    SeqPCdQry qryd;

    ajDebug("seqAccessBlast type %d\n", seqin->Input->Query->QueryType);

    qry  = seqin->Input->Query;
    qryd = qry->QryData;

    if(qry->QueryType == AJQUERY_ALL)
        return seqBlastAll(seqin);

    /* we need to search the index files and return a query */

    if(qry->QryData)
    {                                /* reuse unfinished query data */
        if(!seqCdQryReuse(qry))
            return ajFalse;
    }
    else
    {
        seqin->Input->Single = ajTrue;

        if(!seqCdQryOpen(qry))          /* open the table file */
        {
            ajErr("seqCdQryOpen failed");

            return ajFalse;
        }

        qryd = qry->QryData;
        ajFilebuffDel(&seqin->Input->Filebuff);
        seqin->Input->Filebuff = ajFilebuffNewNofile();

        /* binary search for the entryname we need */

        if(qry->QueryType == AJQUERY_ENTRY)
        {
            ajDebug("entry fields: %Lu hasacc:%B\n",
                    ajListGetLength(qry->QueryFields), qry->HasAcc);

            if(!seqCdQryEntry(qry))
            {
                ajDebug("BLAST Entry failed\n");
            }
        }

        if(qry->QueryType == AJQUERY_QUERY)
        {
            ajDebug("query fields: %Lu hasacc:%B\n",
                    ajListGetLength(qry->QueryFields), qry->HasAcc);

            if(!seqCdQryQuery(qry))
            {
                ajDebug("BLAST Query failed\n");
            }
        }
    }

    if(ajListGetLength(qry->ResultsList))
    {
        retval = seqBlastQryNext(qry);  /* changes qry->QryData */
        qryd = qry->QryData;

        if(retval)
            seqBlastLoadBuff(seqin);
    }

    if(!ajListGetLength(qry->ResultsList))
    {
        ajFileClose(&qryd->libr);
        ajFileClose(&qryd->libs);
        ajFileClose(&qryd->libt);
        ajFileClose(&qryd->libf);
        seqCdQryClose(qry);

        if((qry->QueryType == AJQUERY_ENTRY) && !seqin->Input->Multi)
        {
            AJFREE(qry->QryData);
            qryd = NULL;
        }
    }

    ajStrAssignS(&seqin->Input->Db, qry->DbName);

    return retval;
}




/* @funcstatic seqBlastOpen ***************************************************
**
** Opens a blast database. The query object can specify protein or DNA type.
** The blast version (1 or 2) is derived from the table file name.
**
** @param [u] qry [AjPQuery] Sequence query object
** @param [r] next [AjBool] Skip to next file (when reading all entries)
** @return [AjBool] ajTrue on success
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool seqBlastOpen(AjPQuery qry, AjBool next)
{
    static const char* seqext[] = {"bsq", "csq", "psq", "nsq"};
    static const char* hdrext[] = {"ahd", "nhd", "phr", "nhr"};
    static const char* tblext[] = {"atb", "ntb", "pin", "nin"};

    short j;
    AjBool isblast2 = ajFalse;
    AjBool isdna    = ajFalse;
    ajint rdtmp  = 0;
    ajint rdtmp2 = 0;

    ajint DbType;                       /* database type indicator */
    ajint DbFormat;                     /* (version) indicator */
    ajint TitleLen;                     /* length of database title */
    ajint DateLen;                      /* length of database date string */
    ajint LineLen;                      /* length of database lines */
    ajint HeaderLen;                    /* bytes before tables start */
    ajint Size;                         /* number of database entries */
    ajint CompLen;                      /* length of compressed seq file */
    ajint MaxSeqLen;                    /* max. entry length */
    ajint TotLen;                       /* bases or residues in database */
    ajint CleanCount;                   /* count of cleaned 8mers */
    AjPStr Title=NULL;                  /* database title */
    AjPStr Date=NULL;                   /* database date */

    SeqPCdQry qryd;

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

    ajReadbinInt2(qryd->dfp->File, &j);
    ajReadbinCharTrim(qryd->dfp->File, qryd->nameSize, qryd->name);

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
        qryd->type = 0;                 /* *tb : blast1 */
    else
    {
        qryd->type = 2;                 /* *in : blast2 */
        isblast2 = ajTrue;
    }

    if(ajStrMatchCaseC(qry->DbType, "N"))
    {
        qryd->type += 1;
        isdna = ajTrue;
    }

    ajStrAssignS(&qryd->srcfile, qryd->datfile);
    ajFilenameReplaceExtC(&qryd->srcfile, NULL);
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

    ajReadbinIntEndian(qryd->libt, &DbType);
    ajReadbinIntEndian(qryd->libt, &DbFormat);
    HeaderLen += 8;
    ajDebug("dbtype: %x dbformat: %x\n", DbType, DbFormat);

    ajReadbinIntEndian(qryd->libt, &TitleLen);

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
        if(!ajReadbinBinary(qryd->libt,
                            (size_t)rdtmp, (size_t)1,
                            ajStrGetuniquePtr(&Title)))
            ajFatal("error reading file %F", qryd->libt);
    }
    else
        ajStrAssignClear(&Title);

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
        ajReadbinIntEndian(qryd->libt, &DateLen);
        rdtmp2 = DateLen;
        ajStrAssignResC(&Date, rdtmp2+1, "");

        if(!ajReadbinBinary(qryd->libt,
                            (size_t)rdtmp2,(size_t)1,ajStrGetuniquePtr(&Date)))
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
        ajReadbinIntEndian(qryd->libt, &LineLen);
        HeaderLen += 4;
    }
    else
        LineLen = 0;

    /* all formats have the next 3 */

    ajReadbinIntEndian(qryd->libt, &Size);

    qryd->Size = Size;

    if(!isdna)
    {
        /* mad, but they are the other way for DNA */
        ajReadbinIntEndian(qryd->libt, &TotLen);
        ajReadbinIntEndian(qryd->libt, &MaxSeqLen);
    }
    else
    {
        ajReadbinIntEndian(qryd->libt, &MaxSeqLen);
        ajReadbinIntEndian(qryd->libt, &TotLen);
    }

    HeaderLen += 12;

    if(isdna && !isblast2)
    {
        /* Blast 1.4 DNA only */
        /* compressed db length */
        ajReadbinIntEndian(qryd->libt, &CompLen);
        /* count of nt's cleaned */
        ajReadbinIntEndian(qryd->libt, &CleanCount);
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
    {                                   /* NCBI BLAST 2.x */
        qryd->TopHdr = HeaderLen;       /* header first */
        qryd->TopCmp = qryd->TopHdr + (Size+1) * 4;

        if(isdna)
            qryd->TopAmb = qryd->TopCmp + (Size+1) * 4; /* DNA only */
        else
            qryd->TopAmb = 0;
    }
    else
    {                                /* NCBI BLAST 1.x and WU-BLAST */
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
** @param [u] qry [AjPQuery] sequence query object.
** @return [ajuint] File number (starting at 1) or zero if all files are done.
**
** @release 2.4.0
** @@
******************************************************************************/

static ajuint seqCdDivNext(AjPQuery qry)
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
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool seqBlastLoadBuff(AjPSeqin seqin)
{
    AjBool ret;
    AjPStr hdrstr = NULL;
    AjPStr seqstr = NULL;
    AjPQuery qry;
    SeqPCdQry qryd;

    qry  = seqin->Input->Query;
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
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool seqBlastAll(AjPSeqin seqin)
{
    AjPQuery qry;
    SeqPCdQry qryd;

    qry  = seqin->Input->Query;
    qryd = qry->QryData;

    ajDebug("seqBlastAll\n");

    if(!qry->QryData)
    {
        ajDebug("seqBlastAll initialising\n");
        seqin->Input->Single = ajTrue;

        if(!seqBlastOpen(qry, ajTrue))  /* replaces qry->QryData */
            ajFatal("seqBlastAll failed");

        qryd = qry->QryData;
        ajFilebuffDel(&seqin->Input->Filebuff);
        seqin->Input->Filebuff = ajFilebuffNewNofile();
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
        if(!seqBlastOpen(qry, ajTrue))  /* try the next file */
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
    ajStrAssignS(&seqin->Input->Db, qry->DbName);

    if(!qry->CaseId)
        qry->QryDone = ajTrue;

    return ajTrue;
}




/* @funcstatic seqCdQryFile ***************************************************
**
** Opens a specific file number for an EMBLCD index
**
** @param [u] qry [AjPQuery] Query data
** @return [AjBool] ajTrue on success
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool seqCdQryFile(AjPQuery qry)
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

    ajReadbinInt2(qryd->dfp->File, &j);

    ajReadbinChar(qryd->dfp->File, qryd->nameSize, qryd->name);
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




/* @funcstatic seqAccessMart **************************************************
**
** Reads sequence(s) using BioMart
**
** BioMart is accessed as a URL containing the server and post
**
** @param [u] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
**
** @release 6.3.0
** @@
******************************************************************************/

static AjBool seqAccessMart(AjPSeqin seqin)
{
    AjPStr host      = NULL;
    AjPStr path      = NULL;
    ajint iport;
    AjPQuery qry;
    AjPStr searchdb = NULL;

    AjPMartquery mq = NULL;
    AjPMartqinfo qinfo = NULL;
    AjPMartquery pmq = NULL;
    AjPMartFilter filt = NULL;
    AjPMartAttribute att = NULL;

    AjPStr atts    = NULL;
    AjPStr filts   = NULL;
    const AjPStr value1 = NULL;
    const AjPStr value2 = NULL;
    AjPStr dbid = NULL;

    AjBool dodebug = AJFALSE;

    ajuint i;

    AjIList iter = NULL;
    AjPQueryField field = NULL;

    /*
    ** need to separate the server host, port and address
    **
    ** identifier to be used for entry level
    **
    ** filter(s) to be used for query level
    **
    ** method (if any) to return all entries
    **
    ** attribute(s) to return as text or sequence data
    **
    ** format to return results (maybe some new way to store withn AjPSeqin)
    */

    iport = 80;
    qry = seqin->Input->Query;

    if(!ajListGetLength(qry->QueryFields))
        return ajFalse;

    if(ajStrGetLen(qry->SvrName))
    {
        ajNamSvrGetdbAttrC(qry->SvrName, qry->DbName, "sequence",
                           &seqin->DbSequence);
        if(!ajNamSvrGetdbAttrC(qry->SvrName, qry->DbName,
                               "dbalias", &searchdb))
            ajStrAssignS(&searchdb, qry->DbName);
    }
    else if(!ajNamDbGetDbalias(qry->DbName, &searchdb))
    {
        ajNamDbGetAttrC(qry->DbName, "sequence",
                        &seqin->DbSequence);
        ajStrAssignS(&searchdb, qry->DbName);
    }

    ajDebug("seqAccessMart %S fields: %Lu field: '%S' returns: '%S' "
            "identifier: '%S' accession: '%S' sequence: '%S'\n",
            qry->DbAlias,
            ajListGetLength(qry->QueryFields),
            qry->DbFields, qry->DbReturn,
            qry->DbIdentifier, qry->DbAccession, seqin->DbSequence);

    if(!ajHttpQueryUrl(qry, &iport, &host, &path))
    {
        ajStrDel(&host);
        ajStrDel(&path);
        ajStrDel(&searchdb);

        return ajFalse;
    }

    mq    = ajMartqueryNew();
    ajMartAttachMartquery(seqin,mq);
    ajMartSetMarthostS(seqin,host);
    ajMartSetMartpathS(seqin,path);
    ajMartSetMartport(seqin,iport);


    if(dodebug)
    {
        /* get list of filters for the mart (martfilters)*/

        ajMartGetFilters(seqin, searchdb);
        ajMartfiltersParse(seqin);

        pmq = ajMartGetMartqueryPtr(seqin);
        filt = pmq->Filters;

        /* match to the database definition, try to find standard terms */

        ajDebug("Filters\n=======\n");
        for(i=0; i < filt->Nfilters; ++i)
        {
            value1 = ajTableFetchC(filt->Filters[i], "name");
            value2 = ajTableFetchC(filt->Filters[i], "value");
            ajDebug("%-40S %S\n",value1,value2);
            if(ajStrMatchCaseS(value1, qry->DbIdentifier))
                ajDebug("matched identifier '%S'\n", qry->DbIdentifier);
            if(ajStrMatchCaseS(value1, qry->DbAccession))
                ajDebug("matched accession '%S'\n", qry->DbAccession);
        }

        /* compare filters to USA query and build query string  AjPStr filts */

        /* get list of attributes for the mart (martattributes) */

        ajMartGetAttributes(seqin, searchdb);
        ajMartattributesParse(seqin);

        pmq = ajMartGetMartqueryPtr(seqin);
        att = pmq->Atts;

        /* match to the database definition  AjPStr atts */

        ajDebug("Attributes\n=========\n");
        for(i=0; i < att->Natts; ++i)
        {
            value1 = ajTableFetchC(att->Attributes[i], "name");
            value2 = ajTableFetchC(att->Attributes[i], "value");
            ajDebug("%-40S %S\n",value1,value2);
            if(ajStrMatchCaseS(value1, seqin->DbSequence))
                ajDebug("matched sequence '%S'\n", seqin->DbSequence);
        }

    }

    /* do the query, retrieve the attributes */

    /* build the output - identifier, attributes, sequence */

    /*
    ** Need to get filters from USA
    ** standardize if needed to usual set
    ** attributes should be ignored for now
    */

    /* mart queries are case-sensitive using uppercase
    ** and have '%' as the RDBMS wildcard for '*'
    */

    iter = ajListIterNewread(qry->QueryFields);
    while(!ajListIterDone(iter))
    {
        field = ajListIterGet(iter);
        if(ajStrMatchC(field->Field, "id"))
            dbid = ajStrNewS(field->Wildquery);
    }
    ajListIterDel(&iter);
    if(!ajStrGetLen(dbid))
        return ajFalse;

    ajStrFmtUpper(&dbid);

    ajFmtPrintS(&filts, "%S=\"%S\"", qry->DbIdentifier, dbid);

    if(ajStrGetLen(qry->DbFilter))
        ajFmtPrintAppS(&filts, ",%S", qry->DbFilter);

    ajFmtPrintS(&atts, "%S", qry->DbIdentifier);

    if(ajStrGetLen(qry->DbReturn))
        ajFmtPrintAppS(&atts, ",%S", qry->DbReturn);

    ajFmtPrintAppS(&atts, ",%S", seqin->DbSequence);

    qinfo = ajMartQinfoNew(1);

    if(!qinfo)
        ajErr("Unable to open BioMart '%S'", qry->DbName);

    ajDebug("Parameters:\n");
    ajDebug("atts: '%S'\n", atts);
    ajDebug("filters: '%S'\n", filts);

    ajMartParseParameters(qinfo,atts,filts,0);

    ajMartSetQuerySchemaC(qinfo,"default");
    ajMartSetQueryVersionC(qinfo,"");
    ajMartSetQueryFormatC(qinfo,"TSV");
    ajMartSetQueryCount(qinfo,ajFalse);
    ajMartSetQueryUnique(qinfo,ajFalse);
    ajMartSetQueryStamp(qinfo,ajTrue);
    ajMartSetQueryVerify(qinfo,ajTrue);

    /*
    ** If the header is set to ajTrue then you must make a
    ** call to ajMartCheckHeader after the search has completed.
    ** Doing so will return an AjPStr* array (NULL terminated)
    ** containing the attribute names in the order returned
    ** by the server
    */
    ajMartSetQueryHeader(qinfo,ajFalse);


    ajMartSetQueryDatasetName(qinfo,searchdb,0);

    ajMartSetQueryDatasetInterfaceC(qinfo,"default",0);

    ajMartCheckQinfo(seqin,qinfo);

    ajMartMakeQueryXml(qinfo,seqin);

    ajDebug("Results\n=======\n");

    if(!ajMartSendQuery(seqin))
        ajWarn("Query Failed");

    ajStrDel(&host);
    ajStrDel(&path);
    ajStrDel(&searchdb);
    ajStrDel(&dbid);
    ajStrDel(&filts);
    ajStrDel(&atts);

    if(!qry->CaseId)
        qry->QryDone = ajTrue;

    ajMartQinfoDel(&qinfo);
    ajMartquerySeqinFree(seqin);

    return ajTrue;
}




/* @funcstatic seqAccessDAS ***************************************************
**
** Sequence access method for DAS sources
**
** @param [u] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool seqAccessDAS(AjPSeqin seqin)
{
    AjPStr host         = NULL;
    AjPStr path         = NULL;
    AjIList iter        = NULL;
    AjPQueryField field = NULL;
    AjPStr segments     = NULL;
    AjPQuery qry;
    AjBool ret = ajTrue;
    ajint port = 80;


    qry = seqin->Input->Query;

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

    segments = ajStrNew();

    iter = ajListIterNewread(qry->QueryFields);

    while(!ajListIterDone(iter))
    {
        field = ajListIterGet(iter);

        if(ajStrMatchCaseC(field->Field, "segment") ||
           ajStrMatchCaseC(field->Field, "id"))
        {
            if(!ajStrGetLen(segments))
                ajFmtPrintS(&segments,"segment=%S",
                            field->Wildquery);
            else
                ajFmtPrintS(&segments,"%S;segment=%S",
                            segments,field->Wildquery);

            /* TODO: segment specific start,end positions */
            if(seqin->End > 0)
                ajFmtPrintS(&segments,"%S:%u,%u",
                            segments,seqin->Begin,seqin->End);
        }
    }

    ajFmtPrintS(&path,"%Ssequence?%S",path, segments);

    ajFilebuffDel(&seqin->Input->Filebuff);
    seqin->Input->Filebuff = ajHttpRead(qry->DbHttpVer, qry->DbName,
                                        qry->DbProxy,
                                        host, port, path);

    if(seqin->Input->Filebuff)
        ajFilebuffHtmlNoheader(seqin->Input->Filebuff);
    else
        ret = ajFalse;

    qry->QryDone = ajTrue;

    ajStrDel(&host);
    ajStrDel(&path);
    ajStrDel(&segments);

    ajListIterDel(&iter);

    return ret;
}




/* @funcstatic seqAccessSql ***************************************************
**
** Reads sequence(s) from user-designed SQL databases
**
** SQL is accessed as a URL containing the username/password/host/port
** /databasename
**
** @param [u] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
**
** @release 6.3.0
** @@
******************************************************************************/

static AjBool seqAccessSql(AjPSeqin seqin)
{
    AjPStr password   = NULL;
    AjPStr socketfile = NULL;

    ajint iport;
    AjPQuery qry;
    AjPStr searchtab = NULL;

    AjPStr url = NULL;
    AjPUrlref uo  = NULL;
    AjPStr sql = NULL;

    AjESqlconnectionClient client;
    AjPSqlconnection connection = NULL;
    AjPSqlstatement statement   = NULL;

    AjISqlrow iter = NULL;
    AjPSqlrow row  = NULL;

    AjPStr tabstr = NULL;
    AjPStr str    = NULL;
    AjPStr comp   = NULL;

    const AjPStr qryId = NULL;

#if !defined(HAVE_MYSQL) && !defined(HAVE_POSTGRESQL)
    ajWarn("Cannot use access method SQL without mysql or postgresql");
    return ajFalse;
#endif

    iport = 3306;

    qry = seqin->Input->Query;

    if(ajStrGetLen(qry->SvrName))
    {
        ajNamSvrGetdbAttrC(qry->SvrName, qry->DbName, "sequence",
                           &seqin->DbSequence);
        if(!ajNamSvrGetdbAttrC(qry->SvrName, qry->DbName,
                               "dbalias", &searchtab))
            ajStrAssignS(&searchtab, qry->DbName);
    }
    else if(!ajNamDbGetDbalias(qry->DbName, &searchtab))
    {
        ajNamDbGetAttrC(qry->DbName, "sequence",
                        &seqin->DbSequence);
        ajStrAssignS(&searchtab, qry->DbName);
    }

    ajDebug("seqAccessSql %S fields: %Lu\n",
            qry->DbAlias, ajListGetLength(qry->QueryFields));

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
        client = ajESqlconnectionClientPostgreSQL;
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
        ajErr("Could not connect to SQL database server");


    sql  = ajStrNew();
    comp = ajStrNew();

    qryId = ajQueryGetId(qry);

    if(qry->QueryType == AJQUERY_ENTRY)
        ajFmtPrintS(&comp,"='%S'",qryId);
    else if(qry->QueryType == AJQUERY_QUERY)
    {
        ajFmtPrintS(&comp," LIKE '%S'",qryId);
        ajStrExchangeKK(&comp,'*','%');
        ajStrExchangeKK(&comp,'?','_');
    }
    else if(qry->QueryType == AJQUERY_ALL)
        comp = ajStrNewC(" LIKE '%'");

    ajFmtPrintS(&sql,"SELECT %S,%S",seqin->DbSequence, qry->DbIdentifier);

    if(ajStrGetLen(qry->DbReturn))
        ajFmtPrintAppS(&sql, ",%S", qry->DbReturn);

    ajFmtPrintAppS(&sql, " FROM %S", searchtab);
    ajFmtPrintAppS(&sql, " WHERE %S%S", qry->DbIdentifier,comp);

    if(ajStrGetLen(qry->DbFilter))
        ajFmtPrintAppS(&sql, " %S", qry->DbFilter);

    ajFmtPrintAppS(&sql, ";");

    ajDebug("SQL = %S\n",sql);
    ajStrDel(&comp);

    statement = ajSqlstatementNewRun(connection,sql);
    if(!statement)
        ajErr("Could not execute SQL statement [%S]", sql);

    iter = ajSqlrowiterNew(statement);

    tabstr = ajStrNew();

    ajFilebuffDel(&seqin->Input->Filebuff);
    seqin->Input->Filebuff = ajFilebuffNewNofile();


    while(!ajSqlrowiterDone(iter))
    {
        row = ajSqlrowiterGet(iter);

        /* Sequence */
        ajSqlcolumnToStr(row,&tabstr);

        /* Identifier */
        ajSqlcolumnToStr(row,&str);
        ajFmtPrintAppS(&tabstr, "\t%S",str);

        /* Remaining fields */
        while(ajSqlcolumnToStr(row,&str))
            ajFmtPrintAppS(&tabstr, "\t%S",str);

        ajFilebuffLoadS(seqin->Input->Filebuff,tabstr);
    }

    if(!qry->CaseId)
        qry->QryDone = ajTrue;

    ajSqlrowiterDel(&iter);
    ajSqlstatementDel(&statement);
    ajSqlconnectionDel(&connection);

    ajStrDel(&url);
    ajHttpUrlrefDel(&uo);
    ajStrDel(&searchtab);
    ajStrDel(&tabstr);
    ajStrDel(&str);
    ajStrDel(&sql);

    return ajTrue;
}





/* @section SQL Database Access ***********************************************
**
** These functions manage the SQL database access methods.
**
******************************************************************************/




/* @funcstatic seqAccessEnsembl ***********************************************
**
** Reads sequence(s) using an Ensembl SQL database.
**
** Ensembl is accessed via SQL client server code.
**
** @param [u] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
**
** @release 6.3.0
** @@
******************************************************************************/

static AjBool seqAccessEnsembl(AjPSeqin seqin)
{
    AjBool debug = AJFALSE;

    AjPQuery qry = NULL;

    AjPStr stableidentifier = NULL;

    EnsPExon        exon        = NULL;
    EnsPExonadaptor exonadaptor = NULL;

    EnsPTranscript        transcript        = NULL;
    EnsPTranscriptadaptor transcriptadaptor = NULL;

    EnsPTranslation        translation        = NULL;
    EnsPTranslationadaptor translationadaptor = NULL;

    SeqPEnsembl se = NULL;

    debug = ajDebugTest("seqAccessEnsembl");

    if(debug)
    {
        ajDebug("seqAccessEnsembl\n"
                "  seqin %p\n",
                seqin);

        ensTraceSeqin(seqin, 1);
    }

    if(!seqin)
        return ajFalse;

    /* The seqRead function insists on an AJAX File Buffer. */

    if(seqin->Input->Filebuff == NULL)
        seqin->Input->Filebuff = ajFilebuffNewNofile();

    qry = seqin->Input->Query;

    /* Run the inital query and then return. */

    if(qry->QryDone == ajFalse)
        return seqEnsemblQryFirst(qry);

    se = (SeqPEnsembl) qry->QryData;

    /* Finish if no more identifiers are on the AJAX List. */

    if(ajListGetLength(se->Stableidentifiers) == 0)
    {
        seqEnsemblDel(&se);

        qry->QryData = NULL;

        return ajFalse;
    }

    /*
    ** Retrieve individual sequences from the AJAX List of
    ** stable Ensembl identifiers.
    **
    ** Use the SeqData member of the AJAX Sequence Input structure
    ** to pass the AJAX Sequence object between the AJAX Sequence Reading
    ** (seqReadEnsembl) and AJAX Sequence Database (seqAccessEnsembl) modules.
    */

    switch(se->EnsemblType)
    {
        case seqEEnsemblTypeExon:

            exonadaptor = ensRegistryGetExonadaptor(
                se->Databaseadaptor);

            ajListstrPop(se->Stableidentifiers, &stableidentifier);

            if(stableidentifier)
            {
                ensExonadaptorFetchByStableidentifier(
                    exonadaptor,
                    stableidentifier,
                    0,
                    &exon);

                ajStrDel(&stableidentifier);
            }

            if(exon)
            {
                ensExonFetchSequenceSliceSeq(
                    exon,
                    (AjPSeq*) &seqin->SeqData);

                ensExonDel(&exon);
            }

            break;

        case seqEEnsemblTypeTranslation:

            translationadaptor = ensRegistryGetTranslationadaptor(
                se->Databaseadaptor);

            ajListstrPop(se->Stableidentifiers, &stableidentifier);

            if(stableidentifier)
            {
                ensTranslationadaptorFetchByStableidentifier(
                    translationadaptor,
                    stableidentifier,
                    0,
                    &translation);

                ajStrDel(&stableidentifier);
            }

            if(translation)
            {
                ensTranslationFetchSequenceSeq(
                    translation,
                    (AjPSeq*) &seqin->SeqData);

                ensTranslationDel(&translation);
            }

            break;

        case seqEEnsemblTypeTranscript:

            transcriptadaptor = ensRegistryGetTranscriptadaptor(
                se->Databaseadaptor);

            ajListstrPop(se->Stableidentifiers, &stableidentifier);

            if(stableidentifier)
            {
                ensTranscriptadaptorFetchByStableidentifier(
                    transcriptadaptor,
                    stableidentifier,
                    0,
                    &transcript);

                ajStrDel(&stableidentifier);
            }

            if(transcript)
            {
                ensTranscriptFetchSequenceTranscriptSeq(
                    transcript,
                    (AjPSeq*) &seqin->SeqData);

                ensTranscriptDel(&transcript);
            }

            break;

        default:

            ajDebug("seqAccessEnsembl got unexpected "
                    "Ensembl Object Type %u.\n", se->EnsemblType);
    }

    seqin->Input->Count++;

    return ajTrue;
}




/* @funcstatic seqEnsemblDel **************************************************
**
** Default destructor for an Ensembl Attribute.
**
** @param [u] Pse [SeqPEnsembl*] Ensembl attribute
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void seqEnsemblDel(SeqPEnsembl* Pse)
{
    SeqPEnsembl pthis = NULL;

    if(!Pse)
        return;

    if(!*Pse)
        return;

    pthis = *Pse;

    ajListstrFreeData(&pthis->Stableidentifiers);

    ajStrDel(&pthis->Alias);
    ajStrDel(&pthis->Database);
    ajStrDel(&pthis->Species);
    ajStrDel(&pthis->Wildquery);

    AJFREE(pthis);

    *Pse = NULL;

    return;
}




/* @funcstatic seqEnsemblQryFirst *********************************************
**
** Runs the first AJAX Query for an Ensembl Object.
**
** @param [u] qry [AjPQuery] AJAX Query
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool seqEnsemblQryFirst(AjPQuery qry)
{
    ajuint dbas = 0U;

    AjBool debug  = AJFALSE;
    AjBool result = AJFALSE;

    AjIList iter = NULL;

    AjPQueryField queryfield = NULL;

    AjPStr urlstr = NULL;

    EnsEDatabaseadaptorGroup dbag = ensEDatabaseadaptorGroupCore;

    EnsPDatabaseconnection dbc = NULL;

    SeqPEnsembl se = NULL;

    debug = ajDebugTest("seqEnsemblQryFirst");

    if(debug)
        ajDebug("seqEnsemblQryFirst\n"
                "  qry %p\n",
                qry);

    if(!qry)
        return ajFalse;

    ensInit();

    if(qry->SvrName && ajStrGetLen(qry->SvrName))
    {
        /*
        ** Since the AJAX Query object supplied a server name, use it to
        ** load the Ensembl Registry from AJAX data structures.
        ** If this fails return failure also up-stream.
        */

        ajDebug("seqEnsemblQryFirst got AJAX server name '%S'.\n",
                qry->SvrName);

        dbas = ensRegistryLoadServername(qry->SvrName);
        
        if(!dbas)
            return ajFalse;
    }
    else if(qry->DbName && ajStrGetLen(qry->DbName))
    {
        /*
        ** Since the AJAX Query object supplied a database name, use it to
        ** get the AJAX Sequence Database "url" attribute to establish an
        ** Ensembl Database Connection and subsequntly load the
        ** Ensembl Registry.
        */

        if(debug)
            ajDebug("seqEnsemblQryFirst got AJAX database name '%S'.\n",
                    qry->DbName);

        urlstr = ajStrNew();

        if(!ajNamDbGetUrl(qry->DbName, &urlstr))
        {
            ajErr("seqEnsemblQryFirst could not get a URL for "
                  "Ensembl database '%S'.", qry->DbName);

            ajStrDel(&urlstr);

            return ajFalse;
        }

        if(debug)
            ajDebug("seqEnsemblQryFirst got URL '%S'.\n", urlstr);

        dbc = ensDatabaseconnectionNewUrl(urlstr);

        ajStrDel(&urlstr);

        dbas = ensRegistryLoadDatabaseconnection(dbc);

        ensDatabaseconnectionDel(&dbc);

        if(!dbas)
            return ajFalse;
    }
    else
    {
        ajDebug("seqEnsemblQryFirst got neither an "
                "AJAX server nor AJAX database name.\n");

        return ajFalse;
    }

    AJNEW0(se);

    se->Stableidentifiers = ajListstrNew();
    se->Alias             = ajStrNewS(qry->DbName);
    se->Database          = ajStrNew();
    se->Species           = ajStrNewS(qry->DbName);
    se->Wildquery         = ajStrNew();
    se->Databaseadaptor   = (EnsPDatabaseadaptor) NULL;
    se->EnsemblType       = seqEEnsemblTypeNULL;

    qry->QryData = (void *) se;

    /*
    ** The identifier is supplied via an AJAX Query Field object of
    ** field code "id".
    */

    iter = ajListIterNewread(qry->QueryFields);

    while(!ajListIterDone(iter))
    {
        queryfield = (AjPQueryField) ajListIterGet(iter);

        if(ajStrMatchC(queryfield->Field, "id"))
        {
            /* Ensembl stable identifier */

            ajStrAssignS(&se->Wildquery, queryfield->Wildquery);

            seqEnsemblQryFirstGeneric(qry);
        }

        if(ajStrMatchC(queryfield->Field, "eid"))
        {
            /* Ensembl Exon identifier */

            ajStrAssignS(&se->Wildquery, queryfield->Wildquery);

            se->EnsemblType = seqEEnsemblTypeExon;
        }

        if(ajStrMatchC(queryfield->Field, "pid"))
        {
            /* Ensembl Translation identifier */

            ajStrAssignS(&se->Wildquery, queryfield->Wildquery);

            se->EnsemblType = seqEEnsemblTypeTranslation;
        }

        if(ajStrMatchC(queryfield->Field, "tid"))
        {
            /* Ensembl Transcript identifier */

            ajStrAssignS(&se->Wildquery, queryfield->Wildquery);

            se->EnsemblType = seqEEnsemblTypeTranscript;
        }

        if(ajStrMatchC(queryfield->Field, "grp"))
        {
            /* Ensembl Database Adaptor Group */

            dbag = ensDatabaseadaptorGroupFromStr(queryfield->Wildquery);

            if(dbag == ensEDatabaseadaptorGroupNULL)
            {
                ajWarn("seqEnsemblQryFirst found an invalid "
                       "Ensembl Database Adaptor Group value '%S' "
                       "in the AJAX Query Field '%S'.\n",
                       queryfield->Wildquery, queryfield->Field);

                ajListIterDel(&iter);

                seqEnsemblDel(&se);

                qry->QryData = NULL;

                return ajFalse;
            }
        }
    }

    ajListIterDel(&iter);

    se->Databaseadaptor = ensRegistryGetDatabaseadaptor(dbag, se->Species);

    if(se->Databaseadaptor == NULL)
    {
        ajWarn("seqEnsemblQryFirst could not get an "
               "Ensembl Database Adaptor for species name '%S'.\n",
               se->Species);

        seqEnsemblDel(&se);

        qry->QryData = NULL;

        return ajFalse;
    }

    /* Query the Ensembl database based on the Ensembl Object type. */

    switch(se->EnsemblType)
    {
        case seqEEnsemblTypeExon:

            result = seqEnsemblQryFirstExon(qry);

            break;

        case seqEEnsemblTypeTranscript:

            result = seqEnsemblQryFirstTranscript(qry);

            break;

        case seqEEnsemblTypeTranslation:

            result = seqEnsemblQryFirstTranslation(qry);

            break;

        default:

            ajDebug("seqEnsemblQryFirst got an unexpected "
                    "Ensembl Object Type %d.\n", se->EnsemblType);

            result = ajFalse;
    }

    if(result == ajFalse)
    {
        seqEnsemblDel(&se);

        qry->QryData = NULL;

        return ajFalse;
    }

    /* The query stage has finished at this point. */

    qry->TotalEntries = (ajuint) ajListGetLength(se->Stableidentifiers);

    qry->QryDone = ajTrue;

    if(debug)
        ajDebug("seqEnsemblQryFirst finished the query stage with %u %s.\n",
                qry->TotalEntries,
                (qry->TotalEntries == 1) ? "entry" : "entries");

    return ajTrue;
}




/* @funcstatic seqEnsemblQryFirstExon *****************************************
**
** Runs the first AJAX Query for an Ensembl Exon.
**
** @param [u] qry [AjPQuery] AJAX Query
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool seqEnsemblQryFirstExon(AjPQuery qry)
{
    AjBool debug = AJFALSE;

    AjPList stableidentifiers = NULL;

    AjPStr stableidentifier = NULL;

    EnsPExonadaptor exonadaptor = NULL;

    SeqPEnsembl se = NULL;

    debug = ajDebugTest("seqEnsemblQryFirstExon");

    ajDebug("seqEnsemblQryFirstExon\n"
            "  qry %p\n",
            qry);

    if(!qry)
        return ajFalse;

    se = (SeqPEnsembl) qry->QryData;

    exonadaptor = ensRegistryGetExonadaptor(se->Databaseadaptor);

    switch(qry->QueryType)
    {
        case AJQUERY_ENTRY:

            if(debug)
                ajDebug("seqEnsemblQryFirstExon got Sequence Query type "
                        "'entry'.\n");

            ajListstrPushAppend(se->Stableidentifiers,
                                ajStrNewS(se->Wildquery));

            break;

        case AJQUERY_QUERY:

            if(debug)
                ajDebug("seqEnsemblQryFirstExon got Sequence Query type "
                        "'query'.\n");

            stableidentifiers = ajListstrNew();

            ensExonadaptorRetrieveAllStableidentifiers(exonadaptor,
                                                       stableidentifiers);

            while(ajListstrPop(stableidentifiers, &stableidentifier))
            {
                if(ajStrMatchWildS(stableidentifier, se->Wildquery))
                    ajListstrPushAppend(se->Stableidentifiers,
                                        ajStrNewS(stableidentifier));

                ajStrDel(&stableidentifier);
            }

            ajListstrFreeData(&stableidentifiers);

            break;

        case AJQUERY_ALL:

            if(debug)
                ajDebug("seqEnsemblQryFirstExon got Sequence Query type "
                        "'all'.\n");

            ensExonadaptorRetrieveAllStableidentifiers(
                exonadaptor,
                se->Stableidentifiers);

            break;

        default:

            ajDebug("seqEnsemblQryFirstExon got an unexpected "
                    "AJAX Sequence Query type %d.\n",
                    qry->QueryType);
    }

    return ajTrue;
}




/* @funcstatic seqEnsemblQryFirstGeneric **************************************
**
** Runs the first AJAX Query for a generic Ensembl Object.
**
** @param [u] qry [AjPQuery] AJAX Query
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool seqEnsemblQryFirstGeneric(AjPQuery qry)
{
    AjBool debug = AJFALSE;

    AjPList dbas = NULL;

    AjPRegexp rp = NULL;

    AjPStr expression = NULL;
    AjPStr prefix     = NULL;
    AjPStr type       = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPExon        exon        = NULL;
    EnsPExonadaptor exonadaptor = NULL;

    EnsPTranscript        transcript        = NULL;
    EnsPTranscriptadaptor transcriptadaptor = NULL;

    EnsPTranslation        translation        = NULL;
    EnsPTranslationadaptor translationadaptor = NULL;

    SeqPEnsembl se = NULL;

    debug = ajDebugTest("seqEnsemblQryFirstGeneric");

    ajDebug("seqEnsemblQryFirstGeneric\n"
            "  qry %p\n",
            qry);

    if(!qry)
        return ajFalse;

    se = (SeqPEnsembl) qry->QryData;

    /* Find out which Ensembl Database Adaptor group to use. */

    dbas = ajListNew();

    ensRegistryGetAllDatabaseadaptors(ensEDatabaseadaptorGroupNULL,
                                      se->Species,
                                      dbas);

    while(ajListPop(dbas, (void **) &dba))
    {
        switch(ensDatabaseadaptorGetGroup(dba))
        {
            case ensEDatabaseadaptorGroupCore:

                prefix = ensRegistryGetStableidentifierprefix(dba);

                /* "^%S([EGTP])[0-9]{11,11}" */

                if(prefix)
                    expression = ajFmtStr("^%S([EGTP])", prefix);

                break;

            case ensEDatabaseadaptorGroupVega:

                prefix = ensRegistryGetStableidentifierprefix(dba);

                /* "^OTT...([EGTP])[0-9]{11,11}" */

                if(prefix)
                    expression = ajStrNewC("^OTT...([EGTP])");

                break;

            case ensEDatabaseadaptorGroupOtherFeatures:

                prefix = ensRegistryGetStableidentifierprefix(dba);

                /* "^%SEST([EGTP])[0-9]{11,11}" */

                if(prefix)
                    expression = ajFmtStr("^%SEST([EGTP])", prefix);

                break;

            case ensEDatabaseadaptorGroupCopyDNA:

                break;

            default:

                continue;
        }

        if(!expression)
            continue;

        if(debug)
            ajDebug("seqEnsemblQryFirstGeneric will test expression '%S' "
                    "against identifier '%S'.\n", expression, se->Wildquery);

        rp = ajRegCompC(ajStrGetPtr(expression));

        if(ajRegExec(rp, se->Wildquery))
        {
            type = ajStrNew();

            ajRegSubI(rp, 1, &type);

            if(ajStrMatchC(type, "E"))
                se->EnsemblType = seqEEnsemblTypeExon;
            else if(ajStrMatchC(type, "P"))
                se->EnsemblType = seqEEnsemblTypeTranslation;
            else if(ajStrMatchC(type, "T"))
                se->EnsemblType = seqEEnsemblTypeTranscript;

            ajStrDel(&type);
        }

        ajRegFree(&rp);

        ajStrDel(&expression);
    }

    ajListFree(&dbas);

    if(se->EnsemblType == seqEEnsemblTypeNULL)
    {
        /*
        ** An Ensembl Object type could not be established.
        ** For AJAX Sequence Queries of type ENTRY it is possible to
        ** test whether this is a stable identifier of any Ensembl
        ** *Core* Object type.
        */

        if(qry->QueryType == AJQUERY_ENTRY)
        {
            /* Only use the Ensembl Database Adaptor Group Core. */

            se->Databaseadaptor = ensRegistryGetDatabaseadaptor(
                ensEDatabaseadaptorGroupCore,
                se->Species);

            /* Check if this is an Ensembl Exon. */

            if(se->EnsemblType == seqEEnsemblTypeNULL)
            {
                exonadaptor = ensRegistryGetExonadaptor(
                    se->Databaseadaptor);

                ensExonadaptorFetchByStableidentifier(
                    exonadaptor,
                    se->Wildquery,
                    0,
                    &exon);

                if(exon != NULL)
                {
                    /*
                      ajListstrPushAppend(se->Stableidentifiers,
                      ajStrNewS(se->Wildquery));
                    */
                    se->EnsemblType = seqEEnsemblTypeExon;
                }

                ensExonDel(&exon);
            }

            /* Check if this is an Ensembl Transcript. */

            if(se->EnsemblType == seqEEnsemblTypeNULL)
            {
                transcriptadaptor = ensRegistryGetTranscriptadaptor(
                    se->Databaseadaptor);

                ensTranscriptadaptorFetchByStableidentifier(
                    transcriptadaptor,
                    se->Wildquery,
                    0,
                    &transcript);

                if(transcript != NULL)
                {
                    /*
                      ajListstrPushAppend(se->Stableidentifiers,
                      ajStrNewS(se->Wildquery));
                    */
                    se->EnsemblType = seqEEnsemblTypeTranscript;
                }

                ensTranscriptDel(&transcript);
            }

            /* Check if this is an Ensembl Translation. */

            if(se->EnsemblType == seqEEnsemblTypeNULL)
            {
                translationadaptor = ensRegistryGetTranslationadaptor(
                    se->Databaseadaptor);

                ensTranslationadaptorFetchByStableidentifier(
                    translationadaptor,
                    se->Wildquery,
                    0,
                    &translation);

                if(translation != NULL)
                {
                    /*
                      ajListstrPushAppend(se->Stableidentifiers,
                      ajStrNewS(se->Wildquery));
                    */
                    se->EnsemblType = seqEEnsemblTypeTranslation;
                }

                ensTranslationDel(&translation);
            }
        }
        else
            ajWarn("Since this species does not seem to have a "
                   "stable identifier schema defined in the 'meta' "
                   "table or the identifier type could not be "
                   "established, only queries for single entries can "
                   "be matched against Exons, Transcripts or "
                   "Translations.");
    }

    return ajTrue;
}




/* @funcstatic seqEnsemblQryFirstTranscript ***********************************
**
** Runs the first AJAX Query for an Ensembl Transcript.
**
** @param [u] qry [AjPQuery] AJAX Query
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool seqEnsemblQryFirstTranscript(AjPQuery qry)
{
    AjBool debug = AJFALSE;

    AjPList stableidentifiers = NULL;

    AjPStr stableidentifier = NULL;

    EnsPTranscriptadaptor transcriptadaptor = NULL;

    SeqPEnsembl se = NULL;

    debug = ajDebugTest("seqEnsemblQryFirstTranscript");

    if(debug)
        ajDebug("seqEnsemblQryFirstTranscript\n"
                "  qry %p\n",
                qry);

    if(!qry)
        return ajFalse;

    se = (SeqPEnsembl) qry->QryData;

    transcriptadaptor = ensRegistryGetTranscriptadaptor(se->Databaseadaptor);

    switch(qry->QueryType)
    {
        case AJQUERY_ENTRY:

            if(debug)
                ajDebug("seqEnsemblQryFirstTranscript got Sequence Query "
                        "type 'entry'.\n");

            ajListstrPushAppend(se->Stableidentifiers,
                                ajStrNewS(se->Wildquery));

            break;

        case AJQUERY_QUERY:

            if(debug)
                ajDebug("seqEnsemblQryFirstTranscript got Sequence Query "
                        "type 'query'.\n");

            stableidentifiers = ajListstrNew();

            ensTranscriptadaptorRetrieveAllStableidentifiers(
                transcriptadaptor,
                stableidentifiers);

            while(ajListstrPop(stableidentifiers, &stableidentifier))
            {
                if(ajStrMatchWildS(stableidentifier, se->Wildquery))
                    ajListstrPushAppend(se->Stableidentifiers,
                                        ajStrNewS(stableidentifier));

                ajStrDel(&stableidentifier);
            }

            ajListstrFreeData(&stableidentifiers);

            break;

        case AJQUERY_ALL:

            if(debug)
                ajDebug("seqEnsemblQryFirstTranscript got Sequence Query "
                        "type 'all'.\n");

            ensTranscriptadaptorRetrieveAllStableidentifiers(
                transcriptadaptor,
                se->Stableidentifiers);

            break;

        default:

            ajDebug("seqEnsemblQryFirstTranscript got an unexpected "
                    "AJAX Sequence Query type %d.\n",
                    qry->QueryType);
    }

    return ajTrue;
}




/* @funcstatic seqEnsemblQryFirstTranslation **********************************
**
** Runs the first AJAX Query for an Ensembl Translation.
**
** @param [u] qry [AjPQuery] AJAX Query
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool seqEnsemblQryFirstTranslation(AjPQuery qry)
{
    AjBool debug = AJFALSE;

    AjPList stableidentifiers = NULL;

    AjPStr stableidentifier = NULL;

    EnsPTranslationadaptor translationadaptor = NULL;

    SeqPEnsembl se = NULL;

    debug = ajDebugTest("seqEnsemblQryFirstTranslation");

    if(debug)
        ajDebug("seqEnsemblQryFirstTranslation\n"
                "  qry %p\n",
                qry);

    if(!qry)
        return ajFalse;

    se = (SeqPEnsembl) qry->QryData;

    translationadaptor = ensRegistryGetTranslationadaptor(se->Databaseadaptor);

    switch(qry->QueryType)
    {
        case AJQUERY_ENTRY:

            if(debug)
                ajDebug("seqEnsemblQryFirstTranslation got Sequence Query "
                        "type 'entry'.\n");

            ajListstrPushAppend(se->Stableidentifiers,
                                ajStrNewS(se->Wildquery));

            break;

        case AJQUERY_QUERY:

            if(debug)
                ajDebug("seqEnsemblQryFirstTranslation got Sequence Query "
                        "type 'query'.\n");

            stableidentifiers = ajListstrNew();

            ensTranslationadaptorRetrieveAllStableidentifiers(
                translationadaptor,
                stableidentifiers);

            while(ajListstrPop(stableidentifiers, &stableidentifier))
            {
                if(ajStrMatchWildS(stableidentifier, se->Wildquery))
                    ajListstrPushAppend(se->Stableidentifiers,
                                        ajStrNewS(stableidentifier));

                ajStrDel(&stableidentifier);
            }

            ajListstrFreeData(&stableidentifiers);

            break;

        case AJQUERY_ALL:

            if(debug)
                ajDebug("seqEnsemblQryFirstTranslation got Sequence Query "
                        "type 'all'.\n");

            ensTranslationadaptorRetrieveAllStableidentifiers(
                translationadaptor,
                se->Stableidentifiers);

            break;

        default:

            ajDebug("seqEnsemblQryFirstTranslation got an unexpected "
                    "AJAX Sequence Query type %d.\n",
                    qry->QueryType);
    }

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
**
** @release 1.0.0
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
    AjPQuery qry;
    SeqPCdQry qryd;

    nbpn       = 2;
    char_bit   = 8;
    nsentinels = 2;

    nt_magic_byte = 0xfc;

    qry  = seqin->Input->Query;
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
    ajDebug("hdr reading at %d\n", ajFileResetPos(qryd->libt));
    ajReadbinIntEndian(qryd->libt, &start);
    ajDebug("hdr read i: %d value: %d\n", qryd->idnum, start);
    ajReadbinIntEndian(qryd->libt, &end);
    ajDebug("hdr read i: %d value: %d\n", qryd->idnum, end);

    if(end)
        hsize = end - start;
    else
        hsize = qryd->Size - start;

    ajStrAssignResC(hline, hsize+1, "");

    ajDebug("type: %d hsize: %d start: %d end: %d dbSize: %d\n",
            qryd->type, hsize, start, end, qryd->Size);

    ajFileSeek(qryd->libr, start, 0);

    if(!ajReadbinBinary(qryd->libr, hsize, 1, ajStrGetuniquePtr(hline)))
        ajFatal("error reading file %F", qryd->libr);

    ajStrSetValidLen(hline, hsize);


    if(qryd->type >= 2)
        seqBlastStripNcbi(hline);       /* trim the gnl| prefix */
    /* The above now just adds a > */

    ajFilebuffClear(seqin->Input->Filebuff, -1); /* delete all lines */

    ajDebug("Load FASTA file with '%S'\n", *hline);
    ajFilebuffLoadS(seqin->Input->Filebuff, *hline);

    ajDebug("\n** Blast Sequence Reading **\n");

    ajFileSeek(qryd->libt, qryd->TopCmp + 4*(qryd->idnum), 0);
    ajDebug("seq reading at %d\n", ajFileResetPos(qryd->libt));
    ajReadbinIntEndian(qryd->libt, &start);
    ajDebug("seq read i: %d start: %d\n", qryd->idnum, start);
    ajReadbinIntEndian(qryd->libt, &end);
    ajDebug("seq read i: %d   end: %d\n", qryd->idnum, end);

    if(qryd->type == 1 && qryd->libf)
    {                                   /* BLAST 1 FASTA file */
        ajFileSeek(qryd->libt, qryd->TopSrc + 4*(qryd->idnum), 0);
        ajDebug("src reading at %d\n", ajFileResetPos(qryd->libt));
        ajReadbinIntEndian(qryd->libt, &fstart);
        ajDebug("src read i: %d fstart: %d\n", qryd->idnum, fstart);
        ajReadbinIntEndian(qryd->libt, &fend);
        ajDebug("src read i: %d   fend: %d\n", qryd->idnum, fend);
    }

    if(qryd->type == 3)
    {                                   /* BLAST 2 DNA ambiguities */
        ajFileSeek(qryd->libt, qryd->TopAmb + 4*(qryd->idnum), 0);
        ajDebug("amb reading at %d\n", ajFileResetPos(qryd->libt));
        ajReadbinIntEndian(qryd->libt, &astart);
        ajDebug("amb read i: %d astart: %d\n", qryd->idnum, astart);
    }

    switch(qryd->type)
    {
        case 0:                             /* protein 1 */
        case 2:                             /* protein 2 */
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

            /* skip the null byte */
            if(!ajReadbinBinary(qryd->libs, 1, 1, &tmpbyte))
                ajFatal("error reading file %F", qryd->libs);

            if(tmpbyte)
                ajErr(" phase error: %d:%d found\n",qryd->idnum,(ajint)tmpbyte);

            if((size_t)seq_len !=
               (tmp=ajReadbinBinary(qryd->libs,
                                    (size_t)seq_len,
                                    (size_t)1,
                                    ajStrGetuniquePtr(sline))))
            {
                ajErr(" could not read sequence record (a): %d %d != %d\n",
                      start,tmp,seq_len);
                ajErr(" error reading seq at %d\n",start);

                return ajFalse;
            }

            if(btoa[(ajint)ajStrGetCharLast(*sline)] =='*')
            {                               /* skip * at end */
                seqcnt = seq_len-1;
                ajStrCutEnd(sline, 1);
            }
            else
                seqcnt=seq_len;

            seq = ajStrGetuniquePtr(sline);
            sptr = seq+seqcnt;

            while(--sptr >= seq)
                *sptr = btoa[(ajint)*sptr];

            ajStrSetValidLen(sline, seqcnt);
            ajDebug("Read sequence %d %d\n'%S'\n", seqcnt, ajStrGetLen(*sline),
                    *sline);
            ajStrAssignS(&seqin->Inseq, *sline);

            return ajTrue;


        case 3:                             /* DNA 2 */
            ajDebug("reading blast2 DNA file\n");
            ajFileSeek(qryd->libs,start,0);
            spos = (start)/(char_bit/nbpn);
            c_len = astart - start; /* we have ambiguities in the nsq file */
            seq_len = c_len*4;

            ajDebug("c_len %d spos %d seq_len %d\n",
                    c_len, spos, seq_len);

            ajStrAssignResC(sline, seq_len+1, "");

            seq = ajStrGetuniquePtr(sline);

            /* read the sequence here */

            seqcnt = c_len;

            if((size_t)seqcnt !=
               (tmp=ajReadbinBinary(qryd->libs,(size_t)seqcnt,(size_t)1,
                                    ajStrGetuniquePtr(sline))))
            {
                ajErr(" could not read sequence record (c): %d %d != %d: %d\n",
                      qryd->idnum,tmp,seqcnt,*seq);

                exit(0);
            }

            sptr = seq + seqcnt;

            /* the last byte is either '0' (no remainder) or the last 1-3
               chars and the remainder */

            c_pad = *(sptr-1);
            c_pad &= 0x3;                  /* get the last (low) 2 bits */
            seq_len -= (4 - c_pad); /* if the last 2 bits are 0,
                                       its a NULL byte */
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
            {                               /* read ambiguities */
                a_len = end - astart;
                apos = astart;
                ajDebug("Read ambiguities: a_len %d apos: %d\n", a_len, apos);
                ajFileSeek(qryd->libs,apos,0);
                ajReadbinUintEndian(qryd->libs, &iamb);
                ajDebug("iamb %d\n", iamb);

                for(i=0;i<(ajint)iamb;i++)
                {
                    ajReadbinUintEndian(qryd->libs, &ui);
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
            {                          /* we have the FASTA source file */
                seq_len = fend - fstart;
                ajStrAssignResC(sline, seq_len+1, "");
                ajDebug("reading FASTA file\n");
                ajFileSeek(qryd->libf,fstart,0);

                while(ajReadlineTrim(qryd->libf, &rdline))
                {                           /* line + newline + 1 */
                    ajDebug("Read: '%S'\n", rdline);

                    if(ajStrGetCharFirst(rdline) == '>') /* the FASTA line */
                        break;

                    ajStrAppendS(sline, rdline);
                }

                ajStrAssignS(&seqin->Inseq, *sline);

                return ajTrue;
            }
            else
            {                        /* DNA Blast 1.4 from the csq file */

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

                if(!ajReadbinBinary(qryd->libs, (size_t)1, (size_t)1,
                                    &tmpbyte))   /* skip the null byte */
                    ajFatal("error reading file %F", qryd->libs);

                if(tmpbyte != nt_magic_byte)
                {
                    ajDebug(" phase error: %d:%d (%d/%d) found\n",
                            qryd->idnum,seq_len,(ajint)tmpbyte,
                            (ajint)nt_magic_byte);
                    ajDebug(" error reading seq at %d\n",start);
                    ajErr(" phase error: %d:%d (%d/%d) found\n",
                          qryd->idnum,seq_len,(ajint)tmpbyte,
                          (ajint)nt_magic_byte);
                    ajErr(" error reading seq at %d\n",start);

                    return ajFalse;
                }

                seqcnt=(seq_len+3)/4;

                if(seqcnt==0)
                    seqcnt++;

                if((size_t)seqcnt !=
                   (tmp=ajReadbinBinary(qryd->libs,(size_t)seqcnt,(size_t)1,
                                        ajStrGetuniquePtr(sline))))
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
                if(!ajReadbinBinary(qryd->libs, (size_t)1, (size_t)1, &tmpbyte))
                    ajFatal("error reading file %F", qryd->libs);

                if(tmpbyte != nt_magic_byte)
                {
                    ajDebug(" phase2 error: %d:%d (%d/%d) next \n",
                            qryd->idnum,seqcnt,(ajint)tmpbyte,
                            (ajint)nt_magic_byte);
                    ajDebug(" error reading seq at %d\n",start);
                    ajErr(" phase2 error: %d:%d (%d/%d) next ",
                          qryd->idnum,seqcnt,(ajint)tmpbyte,
                          (ajint)nt_magic_byte);
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
                {                           /* there was enough room */
                    seq[seq_len]= '\0';
                    ajDebug("enough room: seqlen %d\n",seq_len);
                }
                else
                {                           /* not enough room */
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
**
** @release 1.0.0
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
** @param [u] qry [AjPQuery] Sequence query object.
** @param [r] field [const AjPStr] Query field
** @param [r] wildqry [const AjPStr] Query string
** @return [AjBool] ajTrue on success.
**
** @release 1.13.0
** @@
******************************************************************************/

static AjBool seqCdTrgQuery(AjPQuery qry, const AjPStr field,
                            const AjPStr wildqry)
{
    ajint ret=0;

    if(ajStrMatchC(field, "org"))
        ret += seqCdTrgFind(qry, "taxon", wildqry);

    if(ajStrMatchC(field, "key"))
        ret += seqCdTrgFind(qry, "keyword", wildqry);

    if(ajStrMatchC(field, "des"))
        ret += seqCdTrgFind(qry, "des", wildqry);

    if(ajStrMatchC(field, "sv"))
        ret += seqCdTrgFind(qry, "seqvn", wildqry);

    if(ajStrMatchC(field, "gi"))
        ret += seqCdTrgFind(qry, "gi", wildqry);

    if(qry->HasAcc && ajStrMatchC(field, "acc"))
        ret += seqCdTrgFind(qry, "acnum", wildqry);


    if(ret)
        return ajTrue;

    return ajFalse;
}




/* @funcstatic seqCdTrgFind ***************************************************
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
** @release 2.6.0
** @@
******************************************************************************/

static ajuint seqCdTrgFind(AjPQuery qry, const char* indexname,
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
    l       = qry->ResultsList;
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

        if(!match)
            continue;

        seqCdTrgLine(trgline, i, trgfp);
        seqCdFileSeek(hitfp,trgline->FirstHit-1);
        ajDebug("Query First: %d Count: %d\n",
                trgline->FirstHit, trgline->NHits);
        pos = trgline->FirstHit;

        for(j=0;j<(ajint)trgline->NHits;++j)
        {
            ajReadbinInt(hitfp->File, &k);
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

    return (ajuint) ajListGetLength(l);
}




/* @func ajSeqdbPrintAccess ***************************************************
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

void ajSeqdbPrintAccess(AjPFile outf, AjBool full)
{
    ajint i = 0;

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# Sequence access methods\n");
    ajFmtPrintF(outf, "# Name       Alias Entry Query   All Description\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "method {\n");

    for(i=0; seqAccess[i].Name; i++)
        if(full || !seqAccess[i].Alias)
            ajFmtPrintF(outf, "  %-10s %5B %5B %5B %5B \"%s\"\n",
                        seqAccess[i].Name, seqAccess[i].Alias,
                        seqAccess[i].Entry, seqAccess[i].Query,
                        seqAccess[i].All, seqAccess[i].Desc);

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @funcstatic seqCdIdxDel ****************************************************
**
** Destructor for SeqPCdIdx
**
** @param [d] pthys [SeqPCdIdx*] Cd index object
** @return [void]
**
** @release 4.0.0
******************************************************************************/

static void seqCdIdxDel(SeqPCdIdx* pthys)
{
    SeqPCdIdx thys = *pthys;

    if(!thys)
        return;

    ajStrDel(&thys->EntryName);
    AJFREE(*pthys);

    return;
}




/* @funcstatic seqCdTrgDel ****************************************************
**
** Destructor for SeqPCdTrg
**
** @param [d] pthys [SeqPCdTrg*] Cd index target object
** @return [void]
**
** @release 4.0.0
******************************************************************************/

static void seqCdTrgDel(SeqPCdTrg* pthys)
{
    SeqPCdTrg thys = *pthys;

    if(!thys)
        return;

    ajStrDel(&thys->Target);
    AJFREE(*pthys);

    return;
}




/* @func ajSeqDbExit **********************************************************
**
** Cleans up sequence database processing internal memory
**
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajSeqDbExit(void)
{
    ajRegFree(&seqCdDivExp);
    ajRegFree(&seqBlastDivExp);
    ajRegFree(&seqRegHttpUrl);
    ajCharDel(&seqCdName);
    ajRegFree(&seqRegGcgId);
    ajRegFree(&seqRegGcgCont);
    ajRegFree(&seqRegGcgId2);
    ajRegFree(&seqRegGcgSplit);
    ajRegFree(&seqRegEntrezCount);
    ajRegFree(&seqRegEntrezId);
    ajRegFree(&seqRegGcgRefId);

   ensExit();

    return;
}

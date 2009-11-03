/*
** This is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU Library General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
******************************************************************************/

#include "ajax.h"
#include "limits.h"




static ajint seqMaxGcglines = 5000;
static AjPRegexp seqRegQryWild = NULL;

static AjPRegexp seqRegTreeconTop  = NULL;
static AjPRegexp seqRegMegaFeat  = NULL;
static AjPRegexp seqRegMegaSeq  = NULL;
static AjPRegexp seqRegJackTop  = NULL;
static AjPRegexp seqRegJackSeq  = NULL;
static AjPRegexp seqRegGffTyp = NULL;
static AjPRegexp seqRegGff3Typ = NULL;
static AjPRegexp seqRegRawNonseq = NULL;
static AjPRegexp seqRegNbrfId  = NULL;
static AjPRegexp seqRegStadenId = NULL;
static AjPRegexp seqRegHennigBlank = NULL;
static AjPRegexp seqRegHennigSeq   = NULL;
static AjPRegexp seqRegHennigTop   = NULL;
static AjPRegexp seqRegHennigHead  = NULL;
static AjPRegexp seqRegFitchHead = NULL;
static AjPRegexp seqRegStockholmSeq  = NULL;
static AjPRegexp seqRegAbiDots = NULL;
static AjPRegexp seqRegMaseHead = NULL;
static AjPRegexp seqRegPhylipTop  = NULL;
static AjPRegexp seqRegPhylipHead = NULL;
static AjPRegexp seqRegPhylipSeq  = NULL;
static AjPRegexp seqRegPhylipSeq2 = NULL;

static AjPRegexp seqRegGcgDot = NULL;
static AjPRegexp seqRegGcgChk = NULL;
static AjPRegexp seqRegGcgLen = NULL;
static AjPRegexp seqRegGcgTyp = NULL;
static AjPRegexp seqRegGcgNam = NULL;
static AjPRegexp seqRegGcgMsf = NULL;
static AjPRegexp seqRegGcgMsflen = NULL;
static AjPRegexp seqRegGcgMsfnam = NULL;
static AjPRegexp seqRegGcgWgt = NULL;

static AjBool seqInFormatSet = AJFALSE;

static AjPStr seqFtFmtEmbl    = NULL;
static AjPStr seqFtFmtGenbank = NULL;
static AjPStr seqFtFmtGff     = NULL;
static AjPStr seqFtFmtPir     = NULL;
static AjPStr seqFtFmtSwiss   = NULL;
static AjPStr seqUsaTest      = NULL;
static AjPStr seqQryChr       = NULL;
static AjPStr seqQryDb        = NULL;
static AjPStr seqQryList      = NULL;
static AjPStr seqReadLine     = NULL;
static AjPStr seqAppendRestStr = NULL;

static AjPRegexp seqRegUsaAsis  = NULL;
static AjPRegexp seqRegUsaDb    = NULL;
static AjPRegexp seqRegUsaFmt   = NULL;
static AjPRegexp seqRegUsaId    = NULL;
static AjPRegexp seqRegUsaList  = NULL;
static AjPRegexp seqRegUsaRange = NULL;
static AjPRegexp seqRegUsaWild  = NULL;
static AjBool seqDoWarnAppend = AJFALSE;


/* @datastatic SeqPInFormat ***************************************************
**
** Sequence input formats data structure
**
** @alias SeqSInFormat
** @alias SeqOInFormat
**
** @attr Name [const char*] Format name
** @attr Desc [const char*] Format description
** @attr Alias [AjBool] Name is an alias for an identical definition
** @attr Try [AjBool] If true, try for an unknown input. Duplicate names
**                    and read-anything formats are set false
** @attr Nucleotide [AjBool] True if suitable for nucleotide
** @attr Protein [AjBool] True if suitable for protein
** @attr Feature [AjBool] True if includes parsable feature data
** @attr Gap [AjBool] True if allows gap characters
** @attr Read [(AjBool*)] Input function, returns ajTrue on success
** @attr Multiset [AjBool] If true, supports multiple sequence sets
**                         If false, multiple sets must be in separate files
** @attr Padding [ajint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct SeqSInFormat
{
    const char *Name;
    const char *Desc;
    AjBool Alias;
    AjBool Try;
    AjBool Nucleotide;
    AjBool Protein;
    AjBool Feature;
    AjBool Gap;
    AjBool (*Read) (AjPSeq thys, AjPSeqin seqin);
    AjBool Multiset;
    ajint Padding;
} SeqOInFormat;

#define SeqPInFormat SeqOInFormat*




/* @datastatic SeqPMsfData ****************************************************
**
** Sequence alignment data, stored until written when output file is closed
**
** @alias SeqSMsfData
** @alias SeqOMsfData
**
** @attr Table [AjPTable] Ajax table of AjPMsfItem objects
** @attr Names [AjPStr*] Sequence names
** @attr Count [ajuint] Undocumented
** @attr Nseq [ajuint] Number of sequences
** @attr Nexus [AjPNexus] Nexus alignment data
** @attr Bufflines [ajuint] Undocumented
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct SeqSMsfData
{
    AjPTable Table;
    AjPStr* Names;
    ajuint Count;
    ajuint Nseq;
    AjPNexus Nexus;
    ajuint Bufflines;
    char Padding[4];
} SeqOMsfData;

#define SeqPMsfData SeqOMsfData*




/* @datastatic SeqPMsfItem ****************************************************
 **
 ** MSF alignment output individual sequence data
 **
 ** @alias SeqSMsfItem
 ** @alias SeqOMsfItem
 **
 ** @attr Name [AjPStr] Sequence name
 ** @attr Len [ajuint] Sequence length
 ** @attr Check [ajuint] Sequence GCG checksum
 ** @attr Seq [AjPStr] Sequence
 ** @attr Weight [float] Weight (default 1.0)
 ** @attr Padding [char[4]] Padding to alignment boundary
 ** @@
*****************************************************************************/

typedef struct SeqSMsfItem
{
    AjPStr Name;
    ajuint Len;
    ajuint Check;
    AjPStr Seq;
    float Weight;
    char Padding[4];
} SeqOMsfItem;

#define SeqPMsfItem SeqOMsfItem*


/* @datastatic SeqPStockholm **************************************************
**
** Ajax Stockholm object.
**
** @new stockholmNew Default constructor
** @delete stockholmDel Default destructor
**
** @attr id [AjPStr] identifier
** @attr ac [AjPStr] accession
** @attr de [AjPStr] description
** @attr au [AjPStr] author
** @attr al [AjPStr] Undocumented
** @attr tp [AjPStr] Undocumented
** @attr se [AjPStr] Undocumented
** @attr ga [ajuint[2]] Undocumented
** @attr tc [float[2]] Undocumented
** @attr nc [float[2]] Undocumented
** @attr bm [AjPStr] Undocumented
** @attr ref [AjPStr] Undocumented
** @attr dc [AjPStr] Undocumented
** @attr dr [AjPStr] Undocumented
** @attr cc [AjPStr] Undocumented
** @attr sacons [AjPStr] Undocumented
** @attr sscons [AjPStr] Undocumented
** @attr gs [AjPStr] Undocumented
** @attr name [AjPStr*] Undocumented
** @attr str [AjPStr*] Undocumented
** @attr n [ajuint] Undocumented
** @attr Count [ajuint] Count
** @@
******************************************************************************/

typedef struct SeqSStockholm
{
    AjPStr id;
    AjPStr ac;
    AjPStr de;
    AjPStr au;
    AjPStr al;
    AjPStr tp;
    AjPStr se;
    ajuint  ga[2];
    float  tc[2];
    float  nc[2];
    AjPStr bm;
    AjPStr ref;
    AjPStr dc;
    AjPStr dr;
    AjPStr cc;
    AjPStr sacons;
    AjPStr sscons;
    AjPStr gs;
    AjPStr *name;
    AjPStr *str;
    ajuint  n;
    ajuint  Count;
} SeqOStockholm;

#define SeqPStockholm SeqOStockholm*




/* @datastatic SeqPStockholmdata **********************************************
**
** Ajax Stockholm data object (individual sequences)
**
** @new stockholmdataNew Default constructor
** @delete stockholmdataDel Default destructor
**
** @attr id [AjPStr] identifier
** @attr ac [AjPStr] accession
** @attr de [AjPStr] description
** @attr au [AjPStr] author
** @attr al [AjPStr] Undocumented
** @attr tp [AjPStr] Undocumented
** @attr se [AjPStr] Undocumented
** @attr bm [AjPStr] Undocumented
** @attr sscons [AjPStr] Undocumented
** @attr sacons [AjPStr] Undocumented
** @attr ref [AjPStr] Undocumented
** @attr dc [AjPStr] Undocumented
** @attr dr [AjPStr] Undocumented
** @attr cc [AjPStr] Undocumented
** @attr gs [AjPStr] Undocumented
** @attr ga [float[2]] Undocumented
** @attr tc [float[2]] Undocumented
** @attr nc [float[2]] Undocumented
** @@
******************************************************************************/

typedef struct SeqSStockholmdata
{
    AjPStr id;
    AjPStr ac;
    AjPStr de;
    AjPStr au;
    AjPStr al;
    AjPStr tp;
    AjPStr se;
    AjPStr bm;
    AjPStr sscons;
    AjPStr sacons;
    AjPStr ref;
    AjPStr dc;
    AjPStr dr;
    AjPStr cc;
    AjPStr gs;
    float  ga[2];
    float  tc[2];
    float  nc[2];
} SeqOStockholmdata;

#define SeqPStockholmdata SeqOStockholmdata*




/* @datastatic SeqPSelexseq ***************************************************
**
** Ajax Selex object for #=SQ information.
**
** @new selexSQNew Default constructor
** @delete selexSQDel Default destructor
**
** @attr name [AjPStr] Object name
** @attr source [AjPStr] Source file
** @attr ac [AjPStr] accession
** @attr de [AjPStr] description
** @attr wt [float] weight (default 1.0)
** @attr start [ajuint] start position
** @attr stop [ajuint] end position
** @attr len [ajuint] length
** @@
******************************************************************************/

typedef struct SeqSSelexseq
{
    AjPStr name;
    AjPStr source;
    AjPStr ac;
    AjPStr de;
    float  wt;
    ajuint  start;
    ajuint  stop;
    ajuint  len;
}SeqOSelexseq;

#define SeqPSelexseq SeqOSelexseq*




/* @datastatic SeqPSelex ******************************************************
**
** Ajax Selex object.
**
** @new selexNew Default constructor
** @delete selexDel Default destructor
**
** @attr id [AjPStr] identifier
** @attr ac [AjPStr] accession
** @attr de [AjPStr] description
** @attr au [AjPStr] author
** @attr cs [AjPStr] Undocumented
** @attr rf [AjPStr] Undocumented
** @attr name [AjPStr*] Undocumented
** @attr str [AjPStr*] Undocumented
** @attr ss [AjPStr*] Undocumented
** @attr ga [float[2]] Undocumented
** @attr tc [float[2]] Undocumented
** @attr nc [float[2]] Undocumented
** @attr sq [SeqPSelexseq*] Selex sequence objects
** @attr n [ajuint] Number of SeqPSelexseq sequence objects
** @attr Count [ajuint] Count
** @@
******************************************************************************/

typedef struct SeqSSelex
{
    AjPStr id;
    AjPStr ac;
    AjPStr de;
    AjPStr au;
    AjPStr cs;
    AjPStr rf;
    AjPStr *name;
    AjPStr *str;
    AjPStr *ss;
    float  ga[2];
    float  tc[2];
    float  nc[2];
    SeqPSelexseq *sq;
    ajuint  n;
    ajuint  Count;
} SeqOSelex;

#define SeqPSelex SeqOSelex*




/* @datastatic SeqPSelexdata **************************************************
**
** Ajax Selex data object (individual sequences)
**
** @new selexdataNew Default constructor
** @delete selexdataDel Default destructor
**
** @attr id [AjPStr] identifier
** @attr ac [AjPStr] accession
** @attr de [AjPStr] description
** @attr au [AjPStr] author
** @attr cs [AjPStr] Undocumented
** @attr rf [AjPStr] Undocumented
** @attr name [AjPStr] Undocumented
** @attr str [AjPStr] Undocumented
** @attr ss [AjPStr] Undocumented
** @attr ga [float[2]] Undocumented
** @attr tc [float[2]] Undocumented
** @attr nc [float[2]] Undocumented
** @attr sq [SeqPSelexseq] Selex sequence object
** @@
******************************************************************************/

typedef struct SeqSSelexdata
{
    AjPStr id;
    AjPStr ac;
    AjPStr de;
    AjPStr au;
    AjPStr cs;
    AjPStr rf;
    AjPStr name;
    AjPStr str;
    AjPStr ss;
    float  ga[2];
    float  tc[2];
    float  nc[2];
    SeqPSelexseq sq;
} SeqOSelexdata;

#define SeqPSelexdata SeqOSelexdata*






/* @datastatic SeqPListUsa ****************************************************
**
** Usa processing list of USAs from a list file.
**
** Includes data from the original USA (@listfile)
**
** @alias SeqSListUsa
** @alias SeqOListUsa
**
** @attr Begin [ajint] Begin if defined in original USA
** @attr End [ajint] End if defined in original USA
** @attr Rev [AjBool] Reverse if defined in original USA
** @attr Format [ajuint] Format number from original USA
** @attr Formatstr [AjPStr] Format name from original USA
** @attr Usa [AjPStr] Current USA
** @attr Features [AjBool] if true, process features
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct SeqSListUsa
{
    ajint Begin;
    ajint End;
    AjBool Rev;
    ajuint Format;
    AjPStr Formatstr;
    AjPStr Usa;
    AjBool Features;
    char Padding[4];
} SeqOListUsa;

#define SeqPListUsa SeqOListUsa*




enum fmtcode {FMT_OK, FMT_NOMATCH, FMT_BADTYPE, FMT_FAIL, FMT_EOF, FMT_EMPTY};




static AjBool     seqReadAbi(AjPSeq thys, AjPSeqin seqin);

static void       seqAccSave(AjPSeq thys, const AjPStr acc);
static ajuint     seqAppend(AjPStr* seq, const AjPStr line);
static ajuint     seqAppendK(AjPStr* seq, char ch);
static const AjPStr seqAppendWarn(AjPStr* seq, const AjPStr line);
static ajuint     seqAppendCommented(AjPStr* seq, AjBool* incomment,
				     const AjPStr line);
static AjBool     seqClustalReadseq(const AjPStr seqReadLine,
				    const AjPTable msftable);
static AjBool     seqDefine(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqFindInFormat(const AjPStr format, ajint *iformat);
static AjBool     seqFormatSet(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqGcgDots(AjPSeq thys, const AjPSeqin seqin,
			     AjPStr* pline, ajuint maxlines, ajuint *len);
static void       seqGcgRegInit(void);
static AjBool     seqGcgMsfDots(AjPSeq thys, const AjPSeqin seqin,
				AjPStr* pline,
				ajuint maxlines, ajuint *len);
static AjBool     seqGcgMsfHeader(const AjPStr line, SeqPMsfItem* msfitem);
static AjBool     seqGcgMsfReadseq(const AjPStr seqReadLine,
				   const AjPTable msftable);
static AjBool     seqHennig86Readseq(const AjPStr seqReadLine,
				     const AjPTable msftable);
static AjBool     seqinUfoLocal(const AjPSeqin thys);
static void       seqListNoComment(AjPStr* text);
static AjBool     seqListProcess(AjPSeq thys, AjPSeqin seqin,
				 const AjPStr usa);
static void       seqMsfDataDel(SeqPMsfData* pthys);
static void       seqMsfDataTrace(const SeqPMsfData thys);
static void       seqMsfItemDel(SeqPMsfItem* pthys);
static void       seqMsfTabDel(void **key, void **value, void *cl);
static void       seqMsfTabList(const void *key, void **value, void *cl);
static AjBool     seqPhylipReadseq(const AjPStr seqReadLine,
				   const AjPTable phytable,
				   const AjPStr token,
				   ajuint len, ajuint* ilen, AjBool* done);
static AjBool     seqQueryField(const AjPSeqQuery qry, const AjPStr field);
static AjBool     seqQueryFieldC(const AjPSeqQuery qry, const char* field);
static AjBool     seqQueryMatch(const AjPSeqQuery query, const AjPSeq thys);
static void       seqQryWildComp(void);
static AjBool     seqRead(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadAcedb(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadClustal(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadCodata(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadDbId(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadEmbl(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadExperiment(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadFasta(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadFitch(AjPSeq thys, AjPSeqin seqin);
static ajuint     seqReadFmt(AjPSeq thys, AjPSeqin seqin,
			     ajuint format);
static AjBool     seqReadGcg(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadGenbank(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadGifasta(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadGff(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadGff3(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadHennig86(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadIg(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadJackknifer(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadMase(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadMega(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadMsf(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadNbrf(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadNcbi(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadNexus(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadPdb(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadPdbseq(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadPhylip(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadPhylipnon(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadRaw(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadSelex(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadStockholm(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadStaden(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadStrider(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadSwiss(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadText(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadTreecon(AjPSeq thys, AjPSeqin seqin);
static void       seqSelexAppend(const AjPStr src, AjPStr *dest, ajuint beg,
				 ajuint end);
static void       seqSelexCopy(AjPSeq *thys, SeqPSelex selex, ajuint n);
static AjBool     seqSelexHeader(SeqPSelex *thys, const AjPStr line,
				 AjBool *named, ajuint *sqcnt);
static void       seqSelexPos(const AjPStr line, ajuint *begin, ajuint *end);
static AjBool     seqSelexReadBlock(SeqPSelex *thys, AjBool *named, ajuint n,
				    AjPStr *line, AjPFilebuff buff,
				    AjBool store, AjPStr *astr);
static AjBool     seqSetInFormat(const AjPStr format);
static void       seqSetName(AjPStr* name, const AjPStr str);
static void       seqSetNameFile(AjPStr* name, const AjPSeqin seqin);
static void       seqSetNameNospace(AjPStr* name, const AjPStr str);
static void       seqStockholmCopy(AjPSeq *thys, SeqPStockholm stock, ajint n);
static void       seqSvSave(AjPSeq thys, const AjPStr sv);
static void       seqTaxSave(AjPSeq thys, const AjPStr tax, ajuint level);
static void       seqTextSeq(AjPStr* textptr, const AjPStr seq);
static void       seqUsaListTrace(const AjPList list);
static AjBool     seqUsaProcess(AjPSeq thys, AjPSeqin seqin);
static void       seqUsaRestore(AjPSeqin seqin, const SeqPListUsa node);
static void       seqUsaSave(SeqPListUsa node, const AjPSeqin seqin);


static SeqPStockholm stockholmNew(ajuint i);
static void         stockholmDel(SeqPStockholm *thys);

static void         selexDel(SeqPSelex *thys);
static void         selexseqDel(SeqPSelexseq *thys);

/*
static SeqPStockholmdata stockholmdataNew(void);
static void         stockholmdataDel(SeqPStockholmdata *thys);
static SeqPSelexdata seqSelexClone(const SeqPSelexdata thys);
static SeqPSelexdata selexdataNew(void);
static void         selexdataDel(SeqPSelexdata *thys);
*/

static SeqPSelex     selexNew(ajuint n);
static SeqPSelexseq  selexseqNew(void);


/* static data that needs the function definitions and so must come later */

/* @funclist seqInFormatDef ***************************************************
**
** Functions to read each sequence format
**
** New documentation on sequence formats:
** http://www.megasoftware.net/mega4.pdf pages 55 onwards (sections 4.1, 4.2)
** describe MEGA, some other formats, simple XML (name and seq)
**
** The SeqIO program supports some non-EMBOSS formats:
** http://biowulf.nih.gov/apps/seqio_docs/seqio_user.html
** notably FASTA-output, BLAST-output
** and has its own rules for database definitions (BioSeq)
** and database references
**
** For XML formats see Paul Gordon's list at
** http://www.visualgenomics.ca/gordonp/xml/
**
******************************************************************************/

static SeqOInFormat seqInFormatDef[] = {
/* "Name",        "Description" */
/*     Alias,   Try,     Nucleotide, Protein   */
/*     Feature  Gap,     ReadFunction, Multiset, Padding */
  {"unknown",     "Unknown format",
       AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
       AJTRUE,  AJTRUE,  seqReadText, AJFALSE, 0},	/* alias for text */
  {"gcg",         "GCG sequence format",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadGcg, AJFALSE, 0}, /* do 1st, headers mislead */
  {"gcg8",        "GCG old (version 8) sequence format",
       AJTRUE,  AJFALSE, AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadGcg, AJFALSE, 0}, /* alias for gcg (8.x too) */
  {"embl",        "EMBL entry format",
       AJFALSE, AJTRUE,  AJTRUE,  AJFALSE,
       AJFALSE, AJTRUE,  seqReadEmbl, AJFALSE, 0},
  {"em",          "EMBL entry format (alias)",
       AJTRUE,  AJFALSE, AJTRUE,  AJFALSE,
       AJFALSE, AJTRUE,  seqReadEmbl, AJFALSE,0},	/* alias for embl */
  {"swiss",       "Swissprot entry format",
       AJFALSE, AJTRUE,  AJFALSE, AJTRUE,
       AJTRUE,  AJTRUE,  seqReadSwiss, AJFALSE, 0},
  {"sw",          "Swissprot entry format (alias)",
       AJTRUE,  AJFALSE, AJFALSE, AJTRUE,
       AJTRUE,  AJTRUE,  seqReadSwiss, AJFALSE, 0}, /* alias for swiss */
  {"swissprot",   "Swissprot entry format(alias)",
       AJTRUE,  AJTRUE,  AJFALSE, AJTRUE,
       AJTRUE,  AJTRUE,  seqReadSwiss, AJFALSE, 0},
  {"nbrf",        "NBRF/PIR entry format",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadNbrf, AJFALSE, 0},	/* test before NCBI */
  {"pir",         "NBRF/PIR entry format (alias)",
       AJTRUE,  AJFALSE, AJTRUE,  AJTRUE,
       AJTRUE,  AJTRUE,  seqReadNbrf, AJFALSE, 0},	/* alias for nbrf */
  {"pdb",         "PDB protein databank format ATOM lines",
       AJFALSE, AJTRUE,  AJTRUE, AJTRUE,
       AJFALSE, AJFALSE, seqReadPdb, AJFALSE, 0},
  {"pdbseq",         "PDB protein databank format SEQRES lines",
       AJFALSE, AJFALSE, AJTRUE, AJTRUE,
       AJFALSE, AJFALSE, seqReadPdbseq, AJFALSE, 0},
  {"fasta",       "FASTA format including NCBI-style IDs",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadNcbi, AJFALSE, 0}, /* alias for ncbi,
						    preferred name */
  {"ncbi",        "FASTA format including NCBI-style IDs (alias)",
       AJTRUE,  AJFALSE, AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadNcbi, AJFALSE, 0}, /* test before pearson */
  {"gifasta",     "FASTA format including NCBI-style IDs (alias)",
       AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadGifasta, AJFALSE, 0}, /* NCBI with GI as ID*/
  {"pearson",     "Plain old fasta format with IDs not parsed further",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadFasta, AJFALSE, 0}, /* plain fasta - off by
						 default, can read bad files */
  {"genbank",     "Genbank entry format",
       AJFALSE, AJTRUE,  AJTRUE,  AJFALSE,
       AJTRUE,  AJTRUE,  seqReadGenbank, AJFALSE, 0},
  {"gb",          "Genbank entry format (alias)",
       AJTRUE,  AJFALSE, AJTRUE,  AJFALSE,
       AJTRUE,  AJTRUE,  seqReadGenbank, AJFALSE, 0}, /* alias for genbank */
  {"ddbj",        "Genbank/DDBJ entry format (alias)",
       AJTRUE,  AJFALSE, AJTRUE,  AJFALSE,
       AJTRUE,  AJTRUE,  seqReadGenbank, AJFALSE, 0}, /* alias for genbank */
  {"refseq",      "Genbank entry format (alias)",
       AJTRUE,  AJFALSE, AJTRUE,  AJFALSE,
       AJTRUE,  AJTRUE,  seqReadGenbank, AJFALSE, 0}, /* alias for genbank */
  {"codata",      "Codata entry format",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJTRUE,  AJTRUE,  seqReadCodata, AJFALSE, 0},
  {"strider",     "DNA strider output format",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadStrider, AJFALSE, 0},
  {"clustal",     "Clustalw output format",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadClustal, AJFALSE, 0},
  {"aln",         "Clustalw output format (alias)",
       AJTRUE,  AJFALSE, AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadClustal, AJFALSE, 0}, /* alias for clustal */
  {"phylip",      "Phylip interleaved and non-interleaved formats",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadPhylip, AJTRUE, 0},
  {"phylipnon",   "Phylip non-interleaved format",
       AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadPhylipnon, AJTRUE, 0}, /* tried by phylip */
  {"acedb",       "ACEDB sequence format",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadAcedb, AJFALSE, 0},
  {"dbid",        "Fasta format variant with database name before ID",
       AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadDbId, AJFALSE, 0},    /* odd fasta with id as
						       second token */
  {"msf",         "GCG MSF (mutiple sequence file) file format",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadMsf, AJFALSE, 0},
  {"hennig86",    "Hennig86 output format",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadHennig86, AJFALSE, 0},
  {"jackknifer",  "Jackknifer output interleaved format",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadJackknifer, AJFALSE, 0},
  {"nexus",       "Nexus/paup interleaved format",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadNexus, AJFALSE, 0},
  {"paup",        "Nexus/paup interleaved format (alias)",
       AJTRUE,  AJFALSE, AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadNexus, AJFALSE, 0}, /* alias for nexus */
  {"treecon",     "Treecon output format",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadTreecon, AJFALSE, 0},
  {"mega",        "Mega interleaved output format",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadMega, AJFALSE, 0},
  {"ig",          "Intelligenetics sequence format",
       AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadIg, AJFALSE, 0}, /* can read almost anything */
  {"staden",      "Old staden package sequence format",
       AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadStaden, AJFALSE, 0},/* original staden format */
  {"text",        "Plain text",
       AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadText, AJFALSE, 0},/* can read almost anything */
  {"plain",       "Plain text (alias)",
       AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadText, AJFALSE, 0},	/* alias for text */
  {"gff2",         "GFF feature file with sequence in the header",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJTRUE,  AJTRUE,  seqReadGff, AJFALSE, 0},
  {"gff3",         "GFF3 feature file with sequence",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJTRUE,  AJTRUE,  seqReadGff3, AJFALSE, 0},
  {"gff",         "GFF3 feature file with sequence",
       AJTRUE,  AJTRUE,  AJTRUE,  AJTRUE,
       AJTRUE,  AJTRUE,  seqReadGff3, AJFALSE, 0},
  {"stockholm",   "Stockholm (pfam) format",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadStockholm, AJFALSE, 0},
  {"selex",       "Selex format",                /* can read almost anything */
       AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadSelex, AJFALSE, 0},
  {"pfam",        "Stockholm (pfam) format (alias)",
       AJTRUE,  AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadStockholm, AJFALSE, 0},
  {"fitch",       "Fitch program format",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadFitch, AJFALSE, 0},
  {"mase",        "Mase program format",
       AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadMase, AJFALSE, 0},/* like ig - off by default*/
  {"raw",         "Raw sequence with no non-sequence characters",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJFALSE, seqReadRaw, AJFALSE, 0}, /* OK - only sequence chars
						allowed - but off by default*/
  {"experiment",  "Staden experiment file",
       AJFALSE, AJTRUE, AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadExperiment, AJFALSE, 0},
  {"abi",         "ABI trace file",
       AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
       AJFALSE, AJTRUE,  seqReadAbi, AJFALSE, 0},
  {NULL, NULL, 0, 0, 0, 0, 0, 0, NULL, 0, 0}
};



/* ==================================================================== */
/* ========================= constructors ============================= */
/* ==================================================================== */

/* @section Sequence Input Constructors ***************************************
**
** All constructors return a new sequence input object by pointer. It
** is the responsibility of the user to first destroy any previous
** sequence input object. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so
** anyway.
**
******************************************************************************/

/* @func ajSeqinNew ***********************************************************
**
** Creates a new sequence input object.
**
** @return [AjPSeqin] New sequence input object.
** @category new [AjPSeqin] Default constructor
** @@
******************************************************************************/

AjPSeqin ajSeqinNew(void)
{
    AjPSeqin pthis;

    AJNEW0(pthis);

    pthis->Name  = ajStrNew();
    pthis->Acc   = ajStrNew();
    pthis->Db    = ajStrNew();
    pthis->Full  = ajStrNew();
    pthis->Date  = ajStrNew();
    pthis->Desc  = ajStrNew();
    pthis->Doc   = ajStrNew();
    pthis->Rev   = ajFalse;
    pthis->Begin = 0;
    pthis->End   = 0;
    pthis->Usa   = ajStrNew();
    pthis->Ufo   = ajStrNew();
    pthis->List  = NULL;

    pthis->Inputtype = ajStrNew();
    pthis->Formatstr = ajStrNew();
    pthis->Filename  = ajStrNew();
    pthis->Entryname = ajStrNew();


    pthis->Format    = 0;
    pthis->Filebuff  = NULL;
    pthis->Search    = ajTrue;
    pthis->Single    = ajFalse;
    pthis->Features  = ajFalse;
    pthis->Upper     = ajFalse;
    pthis->Lower     = ajFalse;
    pthis->Text      = ajFalse;
    pthis->Count     = 0;
    pthis->Filecount = 0;
    pthis->Fileseqs  = 0;
    pthis->Query     = ajSeqQueryNew();
    pthis->Data      = NULL;
    pthis->Ftquery   = ajFeattabInNew(); /* empty object */
    pthis->multi     = ajFalse;
    pthis->multiset  = ajFalse;

    return pthis;
}





/* ==================================================================== */
/* ========================== destructors ============================= */
/* ==================================================================== */

/* @section Sequence Input Destructors ****************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the sequence input object.
**
******************************************************************************/




/* @func ajSeqinDel ***********************************************************
**
** Deletes a sequence input object.
**
** @param [d] pthis [AjPSeqin*] Sequence input
** @return [void]
** @category delete [AjPSeqin] Default destructor
** @@
******************************************************************************/

void ajSeqinDel(AjPSeqin* pthis)
{
    AjPSeqin thys;
    SeqPListUsa node = NULL;

    if(!pthis) return;

    thys = *pthis;
    if(!thys) return;

    ajDebug("ajSeqinDel called usa:'%S'\n", thys->Usa);


    ajStrDel(&thys->Name);
    ajStrDel(&thys->Acc);

    ajStrDel(&thys->Inputtype);

    ajStrDel(&thys->Db);
    ajStrDel(&thys->Full);
    ajStrDel(&thys->Date);
    ajStrDel(&thys->Desc);
    ajStrDel(&thys->Doc);

    while(ajListGetLength(thys->List))
    {
	ajListPop(thys->List, (void**) &node);
	ajStrDel(&node->Usa);
	ajStrDel(&node->Formatstr);
	AJFREE(node);
    }
    ajListFree(&thys->List);

    ajStrDel(&thys->Usa);
    ajStrDel(&thys->Ufo);
    ajStrDel(&thys->Formatstr);
    ajStrDel(&thys->Filename);
    ajStrDel(&thys->Entryname);
    ajStrDel(&thys->Inseq);
    ajSeqQueryDel(&thys->Query);

    if(thys->Filebuff)
	ajFilebuffDel(&thys->Filebuff);

    if(thys->Fttable)
	ajFeattableDel(&thys->Fttable);

/*
    if(thys->Ftquery && ! thys->multi)
    {
	if(thys->Ftquery->Handle)
	    ajStrDel(&thys->Ftquery->Handle->File->Name);
	if(thys->Ftquery->Handle)
	    ajStrDel(&thys->Ftquery->Handle->File->Buff);
    }
*/

    if(thys->Ftquery)		/* this deletes filebuff stuff above anyway */
        ajFeattabInDel(&thys->Ftquery);

    AJFREE(*pthis);

    return;
}




/* ==================================================================== */
/* =========================== Modifiers ============================== */
/* ==================================================================== */

/* @section Sequence Input Modifiers ******************************************
**
** These functions use the contents of a sequence input object and
** update them.
**
******************************************************************************/




/* @func ajSeqinUsa ***********************************************************
**
** Creates or resets a sequence input object using a new Universal
** Sequence Address
**
** @param [u] pthis [AjPSeqin*] Sequence input object.
** @param [r] Usa [const AjPStr] USA
** @return [void]
** @category modify [AjPSeqin] Resets using a new USA
** @@
******************************************************************************/

void ajSeqinUsa(AjPSeqin* pthis, const AjPStr Usa)
{
    AjPSeqin thys;

    if(!*pthis)
	thys = *pthis = ajSeqinNew();
    else
    {
	thys = *pthis;
	ajSeqinClear(thys);
    }

    ajStrAssignS(&thys->Usa, Usa);

    return;
}




/* @func ajSeqinSetNuc ********************************************************
**
** Sets the type to be forced as nucleic for a sequence input object
**
** @param [u] seqin [AjPSeqin] Sequence input object to be set.
** @return [void]
** @@
******************************************************************************/

void ajSeqinSetNuc(AjPSeqin seqin)
{
    seqin->IsNuc = ajTrue;

    return;
}




/* @func ajSeqinSetProt *******************************************************
**
** Sets the type to be forced as protein for a sequence input object
**
** @param [u] seqin [AjPSeqin] Sequence input object to be set.
** @return [void]
** @@
******************************************************************************/

void ajSeqinSetProt(AjPSeqin seqin)
{
    seqin->IsProt = ajTrue;

    return;
}




/* @func ajSeqinSetRange ******************************************************
**
** Sets the start and end positions for a sequence input object
**
** @param [u] seqin [AjPSeqin] Sequence input object to be set.
** @param [r] ibegin [ajint] Start position. Negative values are from the end.
** @param [r] iend [ajint] End position. Negative values are from the end.
** @return [void]
** @category modify [AjPSeqin] Sets a sequence range for all input sequences
** @@
******************************************************************************/

void ajSeqinSetRange(AjPSeqin seqin, ajint ibegin, ajint iend)
{

    if(ibegin)
	seqin->Begin = ibegin;

    if(iend)
	seqin->End = iend;

    return;
}




/* ==================================================================== */
/* ========================== Assignments ============================= */
/* ==================================================================== */

/* @section Sequence Input Assignments ****************************************
**
** These functions overwrite the sequence input object provided as the
** first argument.
**
******************************************************************************/




/* ==================================================================== */
/* ======================== Operators ==================================*/
/* ==================================================================== */

/* @section Sequence Input Operators ******************************************
**
** These functions use the contents of a sequence input object but do
** not make any changes.
**
******************************************************************************/




/* @func ajSeqAllRead *********************************************************
**
** Parse a USA Uniform Sequence Address into format, access, file and entry
**
** Split at delimiters. Check for the first part as a valid format
** Check for the remaining first part as a database name or as a file
** that can be opened.
** Anything left is an entryname spec.
**
** Return the results in the AjPSeq object but leave the file open for
** future calls.
**
** @param [w] thys [AjPSeq] Sequence returned.
** @param [u] seqin [AjPSeqin] Sequence input definitions
** @return [AjBool] ajTrue on success.
** @category input [AjPSeq] Master sequence stream input, reads first
**           sequence from an open input stream.
** @@
******************************************************************************/

AjBool ajSeqAllRead(AjPSeq thys, AjPSeqin seqin)
{
    AjBool ret       = ajFalse;
    AjPStr tmpformat = NULL;
    SeqPListUsa node = NULL;
    AjBool listdata  = ajFalse;

    if(!seqInFormatSet)
    {
	/* we need a copy of the formatlist */
	if(ajNamGetValueC("format", &tmpformat))
	{
	    seqSetInFormat(tmpformat);
	    ajDebug("seqSetInFormat '%S' from EMBOSS_FORMAT\n", tmpformat);
	}
	ajStrDel(&tmpformat);
	seqInFormatSet = ajTrue;
    }

    if(!seqin->Filebuff)
    {
	/* First call. No file open yet ... */
	if(!seqUsaProcess(thys, seqin)	       /* ... so process the USA */
	   && !ajListGetLength(seqin->List))      /* not list with bad 1st item */
	    return ajFalse; /* if this fails, we read no sequence at all */
	if(ajListGetLength(seqin->List))
	    listdata = ajTrue;
    }


    ret = seqRead(thys, seqin); /* read the sequence */
    if(ret)			/* clone any specified DB or entryname */
    {
	if (ajStrGetLen(seqin->Db))
	{
	    ajDebug("++ajSeqallread set db: '%S' => '%S'\n",
		    seqin->Db, thys->Db);
	    ajStrAssignS(&thys->Db, seqin->Db);
	}
	if (ajStrGetLen(seqin->Entryname))
	{
	    ajDebug("++ajSeqallread set entryname: '%S' => '%S'\n",
		    seqin->Entryname, thys->Entryname);
	    ajStrAssignS(&thys->Entryname, seqin->Entryname);
	}

	if(!ajStrGetLen(thys->Type)) /* make sure the type is set */
	    ajSeqType(thys);
    }

    while(!ret && ajListGetLength(seqin->List))
    {
	/* Failed, but we have a list still - keep trying it */

        ajErr("Failed to read sequence '%S'", seqin->Usa);

	ajListPop(seqin->List, (void**) &node);
	ajDebug("++try again: pop from list '%S'\n", node->Usa);
	ajSeqinUsa(&seqin, node->Usa);
	ajDebug("++SAVE (AGAIN) SEQIN '%S' %d..%d(%b) '%S' %d\n",
		seqin->Usa, seqin->Begin, seqin->End, seqin->Rev,
		seqin->Formatstr, seqin->Format);
	seqUsaRestore(seqin, node);

	ajStrDel(&node->Usa);
	ajStrDel(&node->Formatstr);
	AJFREE(node);

	/* must exit if this fails ... for bad list USAs */

	if(!seqUsaProcess(thys, seqin))
	    continue;

 	ret = seqRead(thys, seqin);
    }

    if(!ret)
    {
      if(listdata)
	ajErr("Failed to read sequence '%S'", seqin->Usa);
      return ajFalse;
    }

    if (seqin->List) {
	ajSeqinClearPos(seqin);
    }

    return ret;
}




/* @func ajSeqallFile *********************************************************
**
** Parse a USA Uniform Sequence Address
**
** Return the results in the AjPSeqall object but leave the file open for
** future calls.
**
** @param [r] usa [const AjPStr] sequence usa.
** @return [AjPSeqall] seqall object
** @@
******************************************************************************/

AjPSeqall ajSeqallFile(const AjPStr usa)
{
    AjPSeqall seqall = NULL;
    AjPSeqin  seqin  = NULL;
    AjPSeq    seq    = NULL;

    seqall = ajSeqallNew();

    seqin = seqall->Seqin;
    seqin->multi  = ajTrue;
    seqin->Single = ajFalse;
    seq = seqall->Seq;

    ajSeqinUsa(&seqin,usa);

    if(!ajSeqAllRead(seq,seqin))
    {
	ajSeqallDel(&seqall);
	return NULL;
    }

    return seqall;
}




/* @func ajSeqallNext *********************************************************
**
** Reads the next sequence into a sequence stream. For the first call this
** simply returns the sequence already loaded. For later calls a new
** sequence is read.
**
** @param [u] seqall [AjPSeqall] Sequence stream
** @param [w] retseq [AjPSeq*] Sequence
** @return [AjBool] ajTrue if a sequence was refound. ajFalse when all is done.
** @category input [AjPSeq] Master sequence stream input, reads next
**                         sequence from an open input stream.
** @category modify [AjPSeqall] Master sequence stream input,
**                 reads next sequence from an open input stream.
** @@
******************************************************************************/

AjBool ajSeqallNext(AjPSeqall seqall, AjPSeq* retseq)
{
    if(!seqall->Count)
    {
	seqall->Count = 1;
	if(seqall->Rev)
	    ajSeqSetRangeRev(seqall->Seq, seqall->Begin, seqall->End);
	else
	    ajSeqSetRange(seqall->Seq, seqall->Begin, seqall->End);

	/*
	seqall->Seq->Begin = seqall->Begin;
	seqall->Seq->End   = seqall->End;
	*/

	*retseq = seqall->Seq;
	seqall->Returned = ajTrue;
	return ajTrue;
    }


    if(ajSeqRead(seqall->Seq, seqall->Seqin))
    {
	seqall->Count++;
	if(seqall->Rev)
	    ajSeqSetRangeRev(seqall->Seq, seqall->Begin, seqall->End);
	else
	    ajSeqSetRange(seqall->Seq, seqall->Begin, seqall->End);

	*retseq = seqall->Seq;
	seqall->Returned = ajTrue;

	ajDebug("ajSeqallNext success\n");
	return ajTrue;
    }

    *retseq = NULL;
    ajDebug("ajSeqallNext failed\n");
    ajSeqallClear(seqall);

    return ajFalse;
}




/* @func ajSeqinClearPos ******************************************************
**
** Clears a Sequence input object position information as possibly read from
** a USA that included the begni, end and direction
**
** @param [u] thys [AjPSeqin] Sequence input
** @return [void]
** @@
******************************************************************************/

void ajSeqinClearPos(AjPSeqin thys)
{
    thys->Rev    = ajFalse;
    thys->Begin = 0;
    thys->End = 0;
    return;
}

/* @func ajSeqinClear *********************************************************
**
** Clears a Sequence input object back to "as new" condition, except
** for the USA list and the features setting which must be preserved.
**
** @param [w] thys [AjPSeqin] Sequence input
** @return [void]
** @category modify [AjPSeqin] Resets ready for reuse.
** @@
******************************************************************************/

void ajSeqinClear(AjPSeqin thys)
{

    ajDebug("ajSeqinClear called\n");

    ajStrSetClear(&thys->Name);
    ajStrSetClear(&thys->Acc);
    /* preserve thys->Inputtype */
    ajStrSetClear(&thys->Db);
    ajStrSetClear(&thys->Full);
    ajStrSetClear(&thys->Date);
    ajStrSetClear(&thys->Desc);
    ajStrSetClear(&thys->Doc);
    /* preserve thys->List */
    ajStrSetClear(&thys->Usa);
    ajStrSetClear(&thys->Ufo);
    ajStrSetClear(&thys->Formatstr);
    ajStrSetClear(&thys->Filename);
    ajStrSetClear(&thys->Entryname);
    ajStrSetClear(&thys->Inseq);

    /* preserve thys->Query */

    if(thys->Filebuff)
	ajFilebuffDel(&thys->Filebuff);

    if(thys->Filebuff)
	ajFatal("ajSeqinClear did not delete Filebuff");

    if(thys->Fttable)
    {
	ajFeattableDel(&thys->Fttable);
    }


/*
    if(thys->Ftquery && ! thys->multi)
    {
	if(thys->Ftquery->Handle)
	    ajStrDel(&thys->Ftquery->Handle->File->Name);
	if(thys->Ftquery->Handle)
	    ajStrDel(&thys->Ftquery->Handle->File->Buff);
   }
*/


    if(thys->Ftquery)  		/* this clears filebuff stuff above anyway */
        ajFeattabInClear(thys->Ftquery);

    ajSeqQueryClear(thys->Query);
    thys->Data = NULL;

    thys->Rev    = ajFalse;
    thys->Format = 0;

    thys->Search = ajTrue;
    thys->Single = ajFalse;

    /* keep thys->Features */
    /* thys->Features = ajFalse;*/

    thys->Count     = 0;
    thys->Filecount = 0;

    thys->Begin = 0;
    thys->End = 0;
    return;
}




/* ==================================================================== */
/* ============================ Casts ==================================*/
/* ==================================================================== */

/* @section Sequence Input Casts **********************************************
**
** These functions examine the contents of a sequence input object and
** return some derived information. Some of them provide access to the
** internal components of a sequence input object. They are provided
** for programming convenience but should be used with caution.
**
******************************************************************************/




/* ==================================================================== */
/* ========================== Assignments ============================= */
/* ==================================================================== */

/* @section Sequence inputs **********************************************
**
** These functions read the sequence provdied by the first argument
**
******************************************************************************/




/* @func ajSeqRead ************************************************************
**
** If the file is not yet open, calls seqUsaProcess to convert the USA into
** an open file stream.
**
** Uses seqRead for the actual file reading.
**
** Returns the results in the AjPSeq object.
**
** @param [w] thys [AjPSeq] Sequence returned.
** @param [u] seqin [AjPSeqin] Sequence input definitions
** @return [AjBool] ajTrue on success.
** @category input [AjPSeq] Master sequence input, calls specific functions
**                  for file access type and sequence format.
** @@
******************************************************************************/

AjBool ajSeqRead(AjPSeq thys, AjPSeqin seqin)
{
    AjPStr tmpformat = NULL;
    AjBool ret       = ajFalse;
    SeqPListUsa node = NULL;
    AjBool listdata  = ajFalse;

    if(!seqInFormatSet)
    {
	/* we need a copy of the formatlist */
	if(ajNamGetValueC("format", &tmpformat))
	{
	    seqSetInFormat(tmpformat);
	    ajDebug("seqSetInFormat '%S' from EMBOSS_FORMAT\n", tmpformat);
	}
	ajStrDel(&tmpformat);
	seqInFormatSet = ajTrue;
    }

    if(seqin->Filebuff)
    {
	/* (a) if file still open, keep reading */
	ajDebug("ajSeqRead: input file '%F' still there, try again\n",
		seqin->Filebuff->File);
	ret = seqRead(thys, seqin);
	ajDebug("ajSeqRead: open buffer  usa: '%S' returns: %B\n",
		seqin->Usa, ret);
    }
    else
    {
	/* (b) if we have a list, try the next USA in the list */
	if(ajListGetLength(seqin->List))
	{
	    listdata = ajTrue;
	    ajListPop(seqin->List, (void**) &node);

	    ajDebug("++pop from list '%S'\n", node->Usa);
	    ajSeqinUsa(&seqin, node->Usa);
	    ajDebug("++SAVE SEQIN '%S' %d..%d(%b) '%S' %d\n",
		    seqin->Usa, seqin->Begin, seqin->End, seqin->Rev,
		    seqin->Formatstr, seqin->Format);
	    seqUsaRestore(seqin, node);

	    ajStrDel(&node->Usa);
	    ajStrDel(&node->Formatstr);
	    AJFREE(node);

	    ajDebug("ajSeqRead: open list, try '%S'\n", seqin->Usa);
	    if(!seqUsaProcess(thys, seqin) && !ajListGetLength(seqin->List))
		return ajFalse;
	    ret = seqRead(thys, seqin);
	    ajDebug("ajSeqRead: list usa: '%S' returns: %B\n",
		    seqin->Usa, ret);
	}
	else
	{
	    ajDebug("ajSeqRead: no file yet - test USA '%S'\n", seqin->Usa);
	    /* (c) Must be a USA - decode it */
	    if(!seqUsaProcess(thys, seqin) && !ajListGetLength(seqin->List))
		return ajFalse;
	    if(ajListGetLength(seqin->List)) /* could be a new list */
		listdata = ajTrue;
	    ret = seqRead(thys, seqin);
	    ajDebug("ajSeqRead: new usa: '%S' returns: %B\n",
		    seqin->Usa, ret);
	}
    }

    /* Now read whatever we got */

    while(!ret && ajListGetLength(seqin->List))
    {
	/* Failed, but we have a list still - keep trying it */
        if(listdata)
	    ajErr("Failed to read sequence '%S'", seqin->Usa);

	listdata = ajTrue;
	ajListPop(seqin->List,(void**) &node);
	ajDebug("++try again: pop from list '%S'\n", node->Usa);
	ajSeqinUsa(&seqin, node->Usa);
	ajDebug("++SAVE (AGAIN) SEQIN '%S' %d..%d(%b) '%S' %d\n",
		seqin->Usa, seqin->Begin, seqin->End, seqin->Rev,
		seqin->Formatstr, seqin->Format);
	seqUsaRestore(seqin, node);

	ajStrDel(&node->Usa);
	ajStrDel(&node->Formatstr);
	AJFREE(node);

	if(!seqUsaProcess(thys, seqin))
	    continue;

	ret = seqRead(thys, seqin);
	ajDebug("ajSeqRead: list retry usa: '%S' returns: %B\n",
		seqin->Usa, ret);
    }

    if(!ret)
    {
	if(listdata)
	    ajErr("Failed to read sequence '%S'", seqin->Usa);

	return ajFalse;
    }


    seqDefine(thys, seqin);

    return ajTrue;
}




/* ==================================================================== */
/* ========================== Assignments ============================= */
/* ==================================================================== */

/* @section Sequence Set Inputs ******************************************
**
** These functions read the sequence set object provided as the
** first argument.
**
******************************************************************************/




/* @func ajSeqsetRead *********************************************************
**
** Parse a USA Uniform Sequence Address into format, access, file and entry
**
** Split at delimiters. Check for the first part as a valid format
** Check for the remaining first part as a database name or as a file
** that can be opened.
** Anything left is an entryname spec.
**
** Read all the sequences until done
**
** Return the results in the AjPSeqset object.
**
** @param [w] thys [AjPSeqset] Sequence set returned.
** @param [u] seqin [AjPSeqin] Sequence input definitions
** @return [AjBool] ajTrue on success.
** @category input [AjPSeqset] Master input routine for a sequence
**                set
** @@
******************************************************************************/

AjBool ajSeqsetRead(AjPSeqset thys, AjPSeqin seqin)
{
    AjPSeq seq;
    AjPList setlist;

    ajuint iseq = 0;

    seq = ajSeqNew();

    ajDebug("ajSeqsetRead\n");

    if(!seqUsaProcess(seq, seqin))
	return ajFalse;

    ajStrAssignS(&thys->Usa, seqin->Usa);
    ajStrAssignS(&thys->Ufo, seqin->Ufo);
    thys->Begin = seqin->Begin;
    thys->End = seqin->End;

    setlist = ajListNew();

    ajDebug("ready to start reading format '%S' '%S' %d..%d\n",
	    seqin->Formatstr, seq->Formatstr, seqin->Begin, seqin->End);

    while(ajSeqRead(seq, seqin))
    {
	if (seqin->List)
	    ajSeqinClearPos(seqin);
	/*ajDebug("read name '%S' length %d format '%S' '%S' seqindata: %x\n",
	  seq->Entryname, ajSeqGetLen(seq),
	  seqin->Formatstr, seq->Formatstr, seqin->Data);*/
	ajStrAssignEmptyS(&seq->Db, seqin->Db);
	if(!ajStrGetLen(seq->Type))
	    ajSeqType(seq);

	if(thys->Rev)
	    ajSeqSetRangeRev(seq, thys->Begin, thys->End);
	else
	    ajSeqSetRange(seq, thys->Begin, thys->End);

	ajDebug ("ajSeqsetRead read sequence %d %x '%s' %d..%d (%d) Rev:%B Reversed:%B\n",
		 iseq, seq, ajSeqGetNameS(seq),
		 seq->Begin, seq->End, ajSeqGetLen(seq),
		 seq->Rev, seq->Reversed);

	/*ajSeqTrace(seq);*/
	iseq++;

	ajListPushAppend(setlist, seq);

	/*ajDebug("appended to list\n");*/

	/* add to a list of sequences */

	seq = ajSeqNew();
	seqFormatSet(seq, seqin);
    }
    ajSeqDel(&seq);

    if(!iseq)
	return ajFalse;

    /* convert the list of sequences into a seqset structure */

    ajSeqsetFromList(thys, setlist);

    ajListFree(&setlist);

    ajDebug("ajSeqsetRead total %d sequences\n", iseq);

    return ajTrue;
}




/* @func ajSeqsetallRead ******************************************************
**
** Parse a USA Uniform Sequence Address into format, access, file and entry
**
** Split at delimiters. Check for the first part as a valid format
** Check for the remaining first part as a database name or as a file
** that can be opened.
** Anything left is an entryname spec.
**
** Read all the sequences into sequence sets until done
**
** Start a new set for each multiple sequence input
**
** Return the results in the AjPList object with AjPSeqset nodes
**
** @param [w] thys [AjPList] List of sequence sets returned.
** @param [u] seqin [AjPSeqin] Sequence input definitions
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajSeqsetallRead(AjPList thys, AjPSeqin seqin)
{
    AjPSeq seq = NULL;
    AjPList setlist = NULL;
    AjPSeqset seqset = NULL;

    ajuint iseq = 0;

    seq = ajSeqNew();

    ajDebug("ajSeqsetallRead\n");

    if(!seqUsaProcess(seq, seqin))
	return ajFalse;



    ajDebug("ready to start reading format '%S' '%S' %d..%d\n",
	    seqin->Formatstr, seq->Formatstr, seqin->Begin, seqin->End);

    while(ajSeqRead(seq, seqin))
    {
	ajDebug("read name '%S' length %d format '%S' '%S' "
		"seqindata: %x multidone: %B\n",
		seq->Entryname, ajSeqGetLen(seq),
		seqin->Formatstr, seq->Formatstr,
		seqin->Data, seqin->multidone);
	ajStrAssignEmptyS(&seq->Db, seqin->Db);
	if(!ajStrGetLen(seq->Type))
	    ajSeqType(seq);

	/*ajDebug ("ajSeqsetallRead read sequence %d '%s' %d..%d\n",
	  iseq, ajSeqGetNameC(seq), seq->Begin, seq->End);*/
	/*ajSeqTrace(seq);*/
	iseq++;

	if(!setlist)
	    setlist = ajListNew();

	ajListPushAppend(setlist, seq);

	/*ajDebug("appended to list\n");*/

	/* add to a list of sequences */

	seq = ajSeqNew();
	seqFormatSet(seq, seqin);
	if(seqin->multidone)
	{
	    seqset = ajSeqsetNew();
	    ajStrAssignS(&seqset->Usa, seqin->Usa);
	    ajStrAssignS(&seqset->Ufo, seqin->Ufo);
	    seqset->Begin = seqin->Begin;
	    seqset->End = seqin->End;

	    ajSeqsetFromList(seqset, setlist);
	    ajListFree(&setlist);
	    ajListPushAppend(thys, seqset);
	    ajDebug("ajSeqsetallRead multidone save set %u of %u sequences\n",
		    ajListGetLength(thys), ajSeqsetGetSize(seqset));
	    seqset = NULL;
	}
    }
    ajSeqDel(&seq);

    if(!iseq)
	return ajFalse;

    /* convert the list of sequences into a seqset structure */

    if(ajListGetLength(setlist))
    {
	seqset = ajSeqsetNew();
	ajStrAssignS(&seqset->Usa, seqin->Usa);
	ajStrAssignS(&seqset->Ufo, seqin->Ufo);
	seqset->Begin = seqin->Begin;
	seqset->End = seqin->End;

	ajSeqsetFromList(seqset, setlist);
	ajListFree(&setlist);
	ajListPushAppend(thys, seqset);
	seqset = NULL;
    }

    ajDebug("ajSeqsetallRead total %d sets of %d sequences\n",
	    ajListGetLength(thys), iseq);

    return ajTrue;
}




/* @func ajSeqsetFromList *****************************************************
**
** Builds a sequence set from a list of sequences
**
** @param [w] thys [AjPSeqset] Sequence set
** @param [r] list [const AjPList] List of sequence objects
** @return [ajint] Number of sequences in the set.
******************************************************************************/

ajint ajSeqsetFromList(AjPSeqset thys, const AjPList list)
{

    ajuint i;
    AjIList iter;
    AjPSeq seq;

    ajDebug("ajSeqsetFromList length: %d\n", ajListGetLength(list));

    /*ajListTrace(list);*/

    thys->Size      = ajListGetLength(list);
    thys->Seq       = AJCALLOC0(thys->Size, sizeof(AjPSeq));
    thys->Seqweight = AJCALLOC0(thys->Size, sizeof(float));

    i = 0;
    iter = ajListIterNewread(list);
    ajListIterTrace(iter);
    while((seq = (AjPSeq) ajListIterGet(iter)))
    {
	if(!i)
	{
	    thys->EType = seq->EType;
	    ajStrAssignS(&thys->Type, seq->Type);
	    thys->Format = seq->Format;
	    ajStrAssignS(&thys->Formatstr, seq->Formatstr);
	    ajStrAssignS(&thys->Filename, seq->Filename);
	    ajStrAssignS(&thys->Full, seq->Full);
	}
	thys->Seqweight[i] = seq->Weight;
	thys->Seq[i] = seq;
	thys->Totweight += seq->Weight;
	if(ajSeqGetLen(seq) > thys->Len)
	    thys->Len = ajSeqGetLen(seq);
	ajDebug("seq %d '%x'\n", i, seq);
	ajDebug("seq '%x' len: %d weight: %.3f\n",
		seq->Name, ajSeqGetLen(seq), thys->Seq[i]->Weight);
	i++;
    }
    ajListIterDel(&iter);

    return thys->Size;
}




/* @func ajSeqsetFromPair *****************************************************
**
** Builds a sequence set from a pair of sequences
**
** @param [w] thys [AjPSeqset] Sequence set
** @param [r] seqa [const AjPSeq] Sequence 1
** @param [r] seqb [const AjPSeq] Sequence 2
** @return [ajint] Number of sequences in the set.
******************************************************************************/

ajint ajSeqsetFromPair(AjPSeqset thys, const AjPSeq seqa, const AjPSeq seqb)
{

    ajSeqsetApp(thys, seqa);
    ajSeqsetApp(thys, seqb);

    return thys->Size;
}




/* @func ajSeqsetApp **********************************************************
**
** Adds a sequence to a sequence set
**
** @param [w] thys [AjPSeqset] Sequence set
** @param [r] seq [const AjPSeq] Sequence
** @return [ajint] Number of sequences in the set.
******************************************************************************/

ajint ajSeqsetApp(AjPSeqset thys, const AjPSeq seq)
{
    ajuint iseq;

    iseq = thys->Size;

    ajDebug("ajSeqsetApp '%S' size %d len %d add '%S' len %d\n",
	    thys->Full, thys->Size, thys->Len,
	    seq->Full, ajSeqGetLen(seq));

    thys->Size ++;
    AJCRESIZE(thys->Seq, thys->Size);
    AJCRESIZE(thys->Seqweight, thys->Size);

    if(!iseq)
    {
	thys->EType = seq->EType;
	ajStrAssignEmptyS(&thys->Type, seq->Type);
	thys->Format = seq->Format;
	ajStrAssignEmptyS(&thys->Formatstr, seq->Formatstr);
	ajStrAssignEmptyS(&thys->Filename, seq->Filename);
	ajStrAssignEmptyS(&thys->Full, seq->Full);
    }

    thys->Seqweight[iseq] = seq->Weight;
    thys->Seq[iseq] = ajSeqNewSeq(seq);
    thys->Totweight += seq->Weight;
    if(ajSeqGetLen(seq) > thys->Len)
	thys->Len = ajSeqGetLen(seq);

    ajDebug("result '%S' size %d len\n",
	    thys->Full, thys->Size, thys->Len);

    return thys->Size;
}




/* @funcstatic seqReadFmt *****************************************************
**
** Tests whether a sequence can be read using the specified format.
** Then tests whether the sequence matches sequence query criteria
** and checks any specified type. Applies upper and lower case.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @param [r] format [ajuint] input format code
** @return [ajuint] 0 if successful.
**                  1 if the query match failed.
**                  2 if the sequence type failed
**                  3 if it failed to read a sequence
** @@
** This is the only function that calls the appropriate Read function
** seqReadXxxxxx where Xxxxxxx is the supported sequence format.
**
** Some of the seqReadXxxxxx functions fail to reset the buffer correctly,
** which is a very serious problem when cycling through all of them to
** identify an unknown format. The extra ajFileBuffReset call at the end is
** intended to address this problem. The individual functions should still
** reset the buffer in case they are called from elsewhere.
**
******************************************************************************/

static ajuint seqReadFmt(AjPSeq thys, AjPSeqin seqin,
			ajuint format)
{
    ajDebug("++seqReadFmt format %d (%s) '%S' feat %B\n",
	    format, seqInFormatDef[format].Name,
	    seqin->Usa, seqin->Features);


    /* Calling funclist seqInFormatDef() */
    if(seqInFormatDef[format].Read(thys, seqin))
    {
	ajDebug("seqReadFmt success with format %d (%s)\n",
		format, seqInFormatDef[format].Name);
        ajDebug("id: '%S' len: %d\n",
                thys->Name, ajStrGetLen(thys->Seq));
	seqin->Format = format;
	ajStrAssignC(&seqin->Formatstr, seqInFormatDef[format].Name);
	ajStrAssignC(&thys->Formatstr, seqInFormatDef[format].Name);
	ajStrAssignEmptyS(&thys->Db, seqin->Db);
	ajStrAssignS(&thys->Entryname, seqin->Entryname);
	ajStrAssignS(&thys->Filename, seqin->Filename);

	if(seqQueryMatch(seqin->Query, thys))
	{
	    ajStrAssignEmptyS(&thys->Entryname, thys->Name);

	    if(seqin->Features && !thys->Fttable)
	    {
		ajStrAssignEmptyS(&seqin->Ftquery->Seqname, thys->Entryname);
		seqin->Fttable = ajFeatUfoRead(seqin->Ftquery,
				  seqin->Ufo);
		if (!seqin->Fttable)
		{
		    ajDebug("seqReadFmt features input failed UFO: '%S'\n",
			    seqin->Ufo);
		    /*
		     **  GWW 21 Aug 2000 - don't warn about missing feature
		     **  tables
		     **/
		}
		else
		{
		    ajFeattableDel(&thys->Fttable);
		    /* ajFeattableTrace(seqin->Fttable); */
		    thys->Fttable = seqin->Fttable;
		    seqin->Fttable = NULL;
		}
	    }

	    if (!ajStrGetLen(thys->Seq))	/* empty sequence string! */
		return FMT_EMPTY;

	    if(ajSeqTypeCheckIn(thys, seqin))
	    {
		/* ajSeqinTrace(seqin); */
		if(seqin->Upper)
		    ajSeqFmtUpper(thys);
		if(seqin->Lower)
		    ajSeqFmtLower(thys);
		if(seqin->Begin)
		    thys->Begin = seqin->Begin;
		if(seqin->End)
		    thys->End = seqin->End;
		if(seqin->Rev)
		    thys->Rev = seqin->Rev;
		return FMT_OK;
	    }
	    else
		return FMT_BADTYPE;
	}
	ajDebug("query match failed, continuing ...\n");
	ajSeqClear(thys);
	return FMT_NOMATCH;
    }
    else
    {
	ajDebug("Testing input buffer: IsBuff: %B Eof: %B\n",
		ajFilebuffIsBuffered(seqin->Filebuff),
		ajFilebuffIsEof(seqin->Filebuff));
	if (!ajFilebuffIsBuffered(seqin->Filebuff) &&
	    ajFilebuffIsEof(seqin->Filebuff))
	    return FMT_EOF;
	ajFilebuffResetStore(seqin->Filebuff, seqin->Text, &thys->TextPtr);
	ajDebug("Format %d (%s) failed, file buffer reset by seqReadFmt\n",
		format, seqInFormatDef[format].Name);
	/* ajFilebuffTraceFull(seqin->Filebuff, 10, 10);*/
    }
    ajDebug("++seqReadFmt failed - nothing read\n");

    return FMT_FAIL;
}




/* @funcstatic seqRead ********************************************************
**
** Given data in a seqin structure, tries to read everything needed
** using the specified format or by trial and error.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqRead(AjPSeq thys, AjPSeqin seqin)
{
    ajuint i;
    ajuint istat;

    AjPFilebuff buff = seqin->Filebuff;

    ajSeqClear(thys);
    ajDebug("seqRead: cleared\n");

    if(seqin->Single && seqin->Count)
    {
	/*
	 ** we read one sequence at a time.
	 ** the first sequence was read by ACD
	 ** for the following ones we need to reset the AjPSeqin
	 **
	 ** Single is set by the access method
	 */

	ajDebug("seqRead: single access - count %d - call access"
		" routine again\n",
		seqin->Count);
	/* Calling funclist seqAccess() */
	if(!seqin->Query->Access->Access(seqin))
	{
	    ajDebug("seqRead: seqin->Query->Access->Access(seqin) "
		    "*failed*\n");
	    return ajFalse;
	}
    }

    ajDebug("seqRead: seqin format %d '%S'\n", seqin->Format,
	    seqin->Formatstr);

    seqin->Count++;

    if(!seqin->Filebuff)
	return ajFalse;

    if(!seqin->Format)
    {			   /* no format specified, try all defaults */

	for(i = 1; seqInFormatDef[i].Name; i++)
	{
	    if(!seqInFormatDef[i].Try)	/* skip if Try is ajFalse */
		continue;

	    ajDebug("seqRead:try format %d (%s)\n",
		    i, seqInFormatDef[i].Name);

	    istat = seqReadFmt(thys, seqin, i);
	    switch(istat)
	    {
	    case FMT_OK:
		ajDebug("++seqRead OK, set format %d\n", seqin->Format);
		seqDefine(thys, seqin);
		return ajTrue;
	    case FMT_BADTYPE:
		ajDebug("seqRead: (a1) seqReadFmt stat == BADTYPE *failed*\n");
		return ajFalse;
	    case FMT_FAIL:
		ajDebug("seqRead: (b1) seqReadFmt stat == FAIL *failed*\n");
		break;			/* we can try next format */
	    case FMT_NOMATCH:
		ajDebug("seqRead: (c1) seqReadFmt stat==NOMATCH try again\n");
		break;
	    case FMT_EOF:
		ajDebug("seqRead: (d1) seqReadFmt stat == EOF *failed*\n");
		return ajFalse;			/* EOF and unbuffered */
	    case FMT_EMPTY:
		ajWarn("Sequence '%S' has zero length, ignored",
		       ajSeqGetUsaS(thys));
		ajDebug("seqRead: (e1) seqReadFmt stat==EMPTY try again\n");
		break;
	    default:
		ajDebug("unknown code %d from seqReadFmt\n", stat);
	    }
	    ajSeqClear(thys);

	    if(seqin->Format)
		break;			/* we read something */
	    ajFilebuffTrace(seqin->Filebuff);
	}

	if(!seqin->Format)
	{		     /* all default formats failed, give up */
	    ajDebug("seqRead:all default formats failed, give up\n");
	    return ajFalse;
	}
	ajDebug("++seqRead set format %d\n", seqin->Format);
    }
    else
    {					/* one format specified */
	ajDebug("seqRead: one format specified\n");
	ajFilebuffSetUnbuffered(seqin->Filebuff);

	ajDebug("++seqRead known format %d\n", seqin->Format);
	istat = seqReadFmt(thys, seqin, seqin->Format);
	switch(istat)
	{
	case FMT_OK:
	    seqDefine(thys, seqin);
	    return ajTrue;
	case FMT_BADTYPE:
	    ajDebug("seqRead: (a2) seqReadFmt stat == BADTYPE *failed*\n");
	    return ajFalse;
	case FMT_FAIL:
	    ajDebug("seqRead: (b2) seqReadFmt stat == FAIL *failed*\n");
	    return ajFalse;
	case FMT_NOMATCH:
	    ajDebug("seqRead: (c2) seqReadFmt stat == NOMATCH *try again*\n");
	    break;
	case FMT_EOF:
	    ajDebug("seqRead: (d2) seqReadFmt stat == EOF *try again*\n");
	    break;		     /* simply end-of-file */
	case FMT_EMPTY:
	    ajWarn("Sequence '%S' has zero length, ignored",
		   ajSeqGetUsaS(thys));
	    ajDebug("seqRead: (e2) seqReadFmt stat == EMPTY *try again*\n");
	    break;
	default:
	    ajDebug("unknown code %d from seqReadFmt\n", stat);
	}

	ajSeqClear(thys); /* 1 : read, failed to match id/acc/query */
    }

    /* failed - probably entry/accession query failed. Can we try again? */

    ajDebug("seqRead failed - try again with format %d '%s'\n",
	    seqin->Format, seqInFormatDef[seqin->Format].Name);

    ajDebug("Search:%B Data:%x ajFileBuffEmpty:%B\n",
	    seqin->Search, seqin->Data, ajFilebuffIsEmpty(buff));
    /* while(seqin->Search) */ /* need to check end-of-file to avoid repeats */
    while(seqin->Search && (seqin->Data ||!ajFilebuffIsEmpty(buff)))
    {
	istat = seqReadFmt(thys, seqin, seqin->Format);
	switch(istat)
	{
	case FMT_OK:
	    seqDefine(thys, seqin);
	    return ajTrue;
	case FMT_BADTYPE:
	    ajDebug("seqRead: (a3) seqReadFmt stat == BADTYPE *failed*\n");
	    return ajFalse;
	case FMT_FAIL:
	    ajDebug("seqRead: (b3) seqReadFmt stat == FAIL *failed*\n");
	    return ajFalse;
	case FMT_NOMATCH:
	    ajDebug("seqRead: (c3) seqReadFmt stat == NOMATCH *try again*\n");
	    break;
	case FMT_EOF:
	    ajDebug("seqRead: (d3) seqReadFmt stat == EOF *failed*\n");
	    return ajFalse;			/* we already tried again */
	case FMT_EMPTY:
	    ajWarn("Sequence '%S' has zero length, ignored",
		   ajSeqGetUsaS(thys));
	    ajDebug("seqRead: (e3) seqReadFmt stat == EMPTY *try again*\n");
	    break;
	default:
	    ajDebug("unknown code %d from seqReadFmt\n", stat);
	}
	ajSeqClear(thys); /* 1 : read, failed to match id/acc/query */
    }

    if(seqin->Format)
	ajDebug("seqRead: *failed* to read sequence %S using format %s\n",
		seqin->Usa, seqInFormatDef[seqin->Format].Name);
    else
	ajDebug("seqRead: *failed* to read sequence %S using any format\n",
		seqin->Usa);

    return ajFalse;
}




/* @funcstatic seqReadFasta ***************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using the FASTA format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadFasta(AjPSeq thys, AjPSeqin seqin)
{
    AjPFilebuff buff;
    AjPStr id   = NULL;
    AjPStr acc  = NULL;
    AjPStr sv   = NULL;
    AjPStr desc = NULL;

    const char *cp;
    ajuint bufflines = 0;
    ajlong fpos     = 0;
    ajlong fposb    = 0;
    AjBool ok       = ajTrue;
    AjPStr tmpline = NULL;
    const AjPStr badstr = NULL;

    ajDebug("seqReadFasta\n");

    buff = seqin->Filebuff;

    /* ajFilebuffTrace(buff); */

    ok = ajBuffreadLinePosStore(buff, &seqReadLine, &fpos,
			     seqin->Text, &thys->TextPtr);
    if(!ok)
	return ajFalse;

    bufflines++;

    ajDebug("First line: %S\n", seqReadLine);
    if(ajStrGetCharPos(seqReadLine, 3) == ';') /* then it is really PIR format */
    {
	ajStrAssignSubS(&tmpline,seqReadLine, 4, -1);
	ajFmtPrintS(&seqReadLine, ">%S",tmpline);
	ajDebug("PIR format changed line to %S\n", seqReadLine);
	ajStrDel(&tmpline);
    }

    cp = ajStrGetPtr(seqReadLine);
    if(*cp != '>')
    {
	ajDebug("first line is not FASTA\n");
	ajFilebuffReset(buff);
	return ajFalse;
    }

    if(!ajSeqParseFasta(seqReadLine, &id, &acc, &sv, &desc))
    {
	ajFilebuffReset(buff);
	return ajFalse;
    }

    seqSetNameNospace(&thys->Name, id);

    if(ajStrGetLen(sv))
	seqSvSave(thys, sv);

    if(ajStrGetLen(acc))
	seqAccSave(thys, acc);

    ajStrAssignS(&thys->Desc, desc);
    ajStrDel(&id);
    ajStrDel(&acc);
    ajStrDel(&sv);
    ajStrDel(&desc);

    if(ajStrGetLen(seqin->Inseq))
    {				       /* we have a sequence to use */
        ajDebug("++fasta use Inseq '%S'\n", seqin->Inseq);
	ajStrAssignS(&thys->Seq, seqin->Inseq);
	if(seqin->Text)
	    seqTextSeq(&thys->TextPtr, seqin->Inseq);

	ajFilebuffClear(buff, 0);
    }
    else
    {
	ok = ajBuffreadLinePosStore(buff, &seqReadLine, &fposb,
				 seqin->Text, &thys->TextPtr);
	while(ok && !ajStrPrefixC(seqReadLine, ">"))
	{
	    badstr = seqAppendWarn(&thys->Seq, seqReadLine);
	    if(badstr)
		ajWarn("Sequence '%S' has bad character(s) '%S'",
			   thys->Name, badstr);
	    bufflines++;
	    ajDebug("++fasta append line '%S'\n", seqReadLine);
	    ok = ajBuffreadLinePosStore(buff, &seqReadLine, &fposb,
				     seqin->Text, &thys->TextPtr);
	}

	if(ok)
	    ajFilebuffClearStore(buff, 1,
				 seqReadLine, seqin->Text, &thys->TextPtr);
	else
	    ajFilebuffClear(buff, 0);
    }

    thys->Fpos = fpos;

    ajDebug("started at fpos %Ld ok: %B fposb: %Ld\n", fpos, ok, fposb);

    return ajTrue;
}




/* @funcstatic seqReadDbId ****************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using the FASTA >db id format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadDbId(AjPSeq thys, AjPSeqin seqin)
{
    AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    AjPFilebuff buff;

    const char *cp;
    const AjPStr vacc = NULL;
    ajuint bufflines = 0;
    ajlong fpos     = 0;
    ajlong fposb    = 0;
    AjBool ok       = ajTrue;

    ajDebug("seqReadDbId\n");

    buff = seqin->Filebuff;
    /* ajFilebuffTrace(buff); */

    ok = ajBuffreadLinePosStore(buff, &seqReadLine, &fpos,
			     seqin->Text, &thys->TextPtr);
    if(!ok)
	return ajFalse;

    bufflines++;

    if(ajStrGetCharPos(seqReadLine, 3) == ';') /* then it is really PIR format */
	return ajFalse;

    cp = ajStrGetPtr(seqReadLine);
    if(*cp != '>')
    {
	ajDebug("first line is not FASTA\n");
	ajFilebuffReset(buff);
	return ajFalse;
    }

    ajStrTokenAssignC(&handle, seqReadLine, "> ");
    ajStrTokenNextParseC(&handle, " \t\n\r", &token);
    ajStrTokenNextParseC(&handle, " \t\n\r", &token);
    seqSetName(&thys->Name, token);

    ajStrTokenNextParse(&handle, &token);

    vacc = ajSeqtestIsSeqversion(token);
    if(vacc)
    {
	seqSvSave(thys, token);
	seqAccSave(thys, vacc);
	ajStrTokenNextParseC(&handle, "\n\r", &thys->Desc);
    }
    else if(ajSeqtestIsAccession(token))
    {
	seqAccSave(thys, token);
	ajStrTokenNextParseC(&handle, "\n\r", &thys->Desc);
    }
    else
    {
	ajStrAssignS(&thys->Desc, token);
	if(ajStrTokenNextParseC(&handle, "\n\r", &token))
	{
	    ajStrAppendC(&thys->Desc, " ");
	    ajStrAppendS(&thys->Desc, token);
	}
    }

    ajStrDel(&token);
    ajStrTokenDel(&handle);

    if(ajStrGetLen(seqin->Inseq))
    {				       /* we have a sequence to use */
	ajStrAssignS(&thys->Seq, seqin->Inseq);
	if(seqin->Text)
	    seqTextSeq(&thys->TextPtr, seqin->Inseq);

	ajFilebuffClear(buff, 0);
    }
    else
    {
	ok = ajBuffreadLinePosStore(buff, &seqReadLine, &fposb,
				 seqin->Text, &thys->TextPtr);
	while(ok && !ajStrPrefixC(seqReadLine, ">"))
	{
	    seqAppend(&thys->Seq, seqReadLine);
	    bufflines++;
	    ok = ajBuffreadLinePosStore(buff, &seqReadLine, &fposb,
				     seqin->Text, &thys->TextPtr);
	}
	if(ok)
	    ajFilebuffClearStore(buff, 1,
				 seqReadLine, seqin->Text, &thys->TextPtr);
	else
	    ajFilebuffClear(buff, 0);
    }

    thys->Fpos = fpos;

    ajDebug("started at fpos %Ld ok: %B fposb: %Ld\n", fpos, ok, fposb);

    return ajTrue;
}




/* @funcstatic seqReadNbrf ****************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using NBRF format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadNbrf(AjPSeq thys, AjPSeqin seqin)
{
    AjPStr token  = NULL;
    AjPStr idline = NULL;
    AjPStr tmpline = NULL;

    AjBool dofeat       = ajFalse;

    AjPStrTok handle2 = NULL;
    AjPStr    token2  = NULL;
    AjPStr    seqReadLine2 = NULL;

    AjBool ok;
    AjPFilebuff buff;

    ajDebug("seqReadNbrf\n");

    buff = seqin->Filebuff;

    if(!token2)
    {
	token2 = ajStrNew();
	seqReadLine2 = ajStrNew();
    }

    if(!seqFtFmtPir)
	ajStrAssignC(&seqFtFmtPir, "pir");

    if(!seqRegNbrfId)
	seqRegNbrfId = ajRegCompC("^>(..)[>;]([^ \t\n]+)");

    if(!ajBuffreadLineStore(buff, &seqReadLine,
			   seqin->Text, &thys->TextPtr))
	return ajFalse;

    ajDebug("nbrf first line:\n%S", seqReadLine);

    if(!ajRegExec(seqRegNbrfId, seqReadLine))
    {
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);
	return ajFalse;
    }
    ajRegSubI(seqRegNbrfId, 1, &token);
    ajRegSubI(seqRegNbrfId, 2, &thys->Name);
    ajDebug("parsed line name '%S' token '%S' token(1) '%c'\n",
	    thys->Name, token, ajStrGetCharFirst(token));
    ajStrAssignSubS(&idline, seqReadLine, 4, -1);

    /*
     ** token has the NBRF 2-char type. First char is the type
     ** and second char is Linear, Circular, or 1
     ** or, for GCG databases, this is just '>>'
     */

    switch(toupper((ajint) ajStrGetCharFirst(token)))
    {
    case 'P':
    case 'F':
	ajSeqSetProt(thys);
	break;
    case 'B':				/* used by DIANA */
    case 'D':				/* DNA */
    case 'R':				/* RNA */
	ajSeqSetNuc(thys);
	break;
    default:
	ajWarn("Unknown NBRF sequence type '%S'", token);
    }

    /* next line is the description, with no prefix */

    if(!ajBuffreadLineStore(buff, &seqReadLine, seqin->Text, &thys->TextPtr))
    {
	ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);
	return ajFalse;
    }

    ajStrAssignS(&thys->Desc, seqReadLine);
    if(ajStrGetCharLast(thys->Desc) == '\n')
	ajStrCutEnd(&thys->Desc, 1);

    /* read on, looking for feature and sequence lines */

    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text, &thys->TextPtr);
    while(ok && !ajStrPrefixC(seqReadLine, ">"))
    {
	if(ajStrGetCharPos(seqReadLine, 1) != ';')
	    seqAppend(&thys->Seq, seqReadLine);
	else
	{
	    if(ajStrPrefixC(seqReadLine, "C;Accession:"))
	    {
		ajStrAssignC(&seqReadLine2,ajStrGetPtr(seqReadLine)+13);
		ajStrTokenAssignC(&handle2,seqReadLine2, " ;\n\r");
		while(ajStrTokenNextParse(&handle2, &token2))
		    seqAccSave(thys, token2);
	    }

	    if(ajStrPrefixC(seqReadLine, "C;Species:"))
	    {
		ajStrAssignC(&seqReadLine2,ajStrGetPtr(seqReadLine)+11);
		ajStrTokenAssignC(&handle2,seqReadLine2, ";.\n\r");
		while(ajStrTokenNextParse(&handle2, &token2))
		{
		    seqTaxSave(thys, token2, 1);
		}
	    }

	    if(ajStrGetCharFirst(seqReadLine) == 'R')
	    {		     /* skip reference lines with no prefix */
		while((ok=ajBuffreadLineStore(buff,&seqReadLine,
					     seqin->Text, &thys->TextPtr)))
		    if(ajStrGetCharPos(seqReadLine,1)==';' ||
		       ajStrGetCharFirst(seqReadLine)=='>')
			break;		/* X; line or next sequence */

		if(ok)
		    continue;
	    }
	    else if(ajStrGetCharFirst(seqReadLine) == 'F')
	    {				/* feature lines */
		if(seqinUfoLocal(seqin))
		{
		    if(!dofeat)
		    {
			dofeat = ajTrue;
			ajFeattabInDel(&seqin->Ftquery);
			seqin->Ftquery = ajFeattabInNewSS(seqFtFmtPir,
							  thys->Name,
							  "N");
			ajDebug("seqin->Ftquery Handle %x\n",
				seqin->Ftquery->Handle);
		    }
		    ajFilebuffLoadS(seqin->Ftquery->Handle, seqReadLine);
		    /* ajDebug("NBRF FEAT saved line:\n%S", seqReadLine); */
		}
	    }
	}
	if(ok)
	    ok = ajBuffreadLineStore(buff, &seqReadLine,
				    seqin->Text, &thys->TextPtr);

	/* SRS 7 and SRS 8.0 put an extra ID line in here */

	/* SRS 8.1 is even worse - it has a peculiar bug that repeats
	   the ID line but with a few digits in front, and then repeats the
	   description */

	/* just for another oddity ... the entra ID line always starts >P1;
	   even if the protein is a fragment */

	if(ok && !ajStrGetLen(thys->Seq) &&
	   (ajStrFindAnyK(seqReadLine, '>') != -1))
	{
	    ajStrAssignS(&tmpline, seqReadLine);
	    ajStrTrimStartC(&tmpline,"0123456789");
	    ajStrCutStart(&tmpline, 4);
	    if(ajStrMatchS(tmpline, idline))
	    {
		ok = ajBuffreadLineStore(buff, &seqReadLine,
					seqin->Text, &thys->TextPtr);
		if(!ajStrIsWhite(seqReadLine)) /* SRS 8.1 description line */
		    ok = ajBuffreadLineStore(buff, &seqReadLine,
					    seqin->Text, &thys->TextPtr);
	    }
	}

    }

    if(ajStrGetCharLast(thys->Seq) == '*')
	ajStrCutEnd(&thys->Seq, 1);

    if(ok)
	ajFilebuffClearStore(buff, 1,
			     seqReadLine, seqin->Text, &thys->TextPtr);
    else
	ajFilebuffClear(buff, 0);

    if(dofeat)
    {
	ajDebug("seqin->Ftquery Handle %x\n",
		seqin->Ftquery->Handle);
	ajFeattableDel(&seqin->Fttable);
	thys->Fttable = ajFeatRead(seqin->Ftquery);
	/* ajFeattableTrace(thys->Fttable); */
	ajFeattabInClear(seqin->Ftquery);
    }

    ajStrDel(&idline);
    ajStrDel(&tmpline);
    ajStrDel(&seqReadLine2);
    ajStrTokenDel(&handle2);
    ajStrDel(&token);
    ajStrDel(&token2);
    return ajTrue;
}




/* @funcstatic seqReadGcg *****************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using GCG format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadGcg(AjPSeq thys, AjPSeqin seqin)
{
    ajuint bufflines      = 0;
    AjBool ok;

    ajuint len     = 0;
    AjBool incomment = ajFalse;

    AjPFilebuff buff;

    buff = seqin->Filebuff;

    ok = ajBuffreadLineStore(buff, &seqReadLine,
			    seqin->Text, &thys->TextPtr);
    if(!ok)
	return ajFalse;
    bufflines++;

    ajDebug("seqReadGcg first line ok: %B\n", ok);

    /* test GCG 9.x file types if available */
    /* any type on the .. line will override this */

    if(ajStrPrefixC(seqReadLine, "!!NA_SEQUENCE"))
	ajSeqSetNuc(thys);
    else if(ajStrPrefixC(seqReadLine, "!!AA_SEQUENCE"))
	ajSeqSetProt(thys);

    if(!seqGcgDots(thys, seqin, &seqReadLine, seqMaxGcglines, &len))
    {
	ajFilebuffReset(buff);
	return ajFalse;
    }
    ajDebug("   Gcg dots read ok len: %d\n", len);


    while(ok &&  (ajSeqGetLen(thys) < len))
    {
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	if(ok)
	{
	    bufflines++;
	    seqAppendCommented(&thys->Seq, &incomment, seqReadLine);
	    ajDebug("line %d seqlen: %d ok: %B\n",
		    bufflines, ajSeqGetLen(thys), ok);
	}
    }
    ajDebug("lines: %d ajSeqGetLen : %d len: %d ok: %B\n",
	    bufflines, ajSeqGetLen(thys), len, ok);

    ajFilebuffClear(buff, 0);


    return ok;
}




/* @funcstatic seqReadNcbi ****************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using NCBI format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadNcbi(AjPSeq thys, AjPSeqin seqin)
{
    AjPStrTok handle = NULL;
    AjPStr id        = NULL;
    AjPStr acc       = NULL;
    AjPStr sv        = NULL;
    AjPStr gi        = NULL;
    AjPStr db        = NULL;
    AjPStr desc      = NULL;

    AjPFilebuff buff;

    ajuint bufflines = 0;
    AjBool ok;
    const AjPStr badstr = NULL;

    buff = seqin->Filebuff;

    ok = ajBuffreadLineStore(buff, &seqReadLine,
			    seqin->Text, &thys->TextPtr);
    if(!ok)
	return ajFalse;

    ajStrAssignClear(&id);
    ajStrAssignClear(&acc);
    ajStrAssignClear(&sv);
    ajStrAssignClear(&gi);
    ajStrAssignClear(&desc);


    if(!ajSeqParseNcbi(seqReadLine,&id,&acc,&sv,&gi,&db,&desc))
    {
	ajFilebuffReset(buff);
	ajStrDel(&id);
	ajStrDel(&acc);
	ajStrDel(&sv);
	ajStrDel(&gi);
	ajStrDel(&db);
	ajStrDel(&desc);
	return ajFalse;
    }

    ajDebug("parsed id '%S' acc '%S' sv '%S' gi '%S' db '%S' (%S) desc '%S'\n",
	    id, acc, sv, gi, db, thys->Setdb, desc);

    ajStrAssignS(&thys->Setdb, db);
    ajDebug("set setdb '%S' db '%S'\n", thys->Setdb, thys->Db);

    if(ajStrGetLen(gi))
	ajStrAssignS(&thys->Gi, gi);

    if(ajStrGetLen(sv))
	seqSvSave(thys, sv);

    if(ajStrGetLen(acc))
	seqAccSave(thys, acc);

    seqSetName(&thys->Name, id);
    ajStrAssignS(&thys->Desc, desc);


    if(ajStrGetLen(seqin->Inseq))
    {				       /* we have a sequence to use */
	ajStrAssignS(&thys->Seq, seqin->Inseq);
	if(seqin->Text)
	    seqTextSeq(&thys->TextPtr, seqin->Inseq);

	ajFilebuffClearStore(buff, 1,
			     seqReadLine, seqin->Text, &thys->TextPtr);
    }
    else
    {
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	while(ok && !ajStrPrefixC(seqReadLine, ">"))
	{
	    badstr = seqAppendWarn(&thys->Seq, seqReadLine);
	    if(badstr)
		ajWarn("Sequence '%S' has bad character(s) '%S'",
			   thys->Name, badstr);
	    bufflines++;
	    ok = ajBuffreadLineStore(buff, &seqReadLine,
				    seqin->Text, &thys->TextPtr);
	}

	if(ok)
	    ajFilebuffClearStore(buff, 1,
				 seqReadLine, seqin->Text, &thys->TextPtr);
	else
	    ajFilebuffClear(buff, 0);
    }

    ajDebug("seqReadNcbi Setdb '%S' Db '%S'\n", thys->Setdb, thys->Db);
    ajStrTokenDel(&handle);
    ajStrDel(&id);
    ajStrDel(&acc);
    ajStrDel(&sv);
    ajStrDel(&gi);
    ajStrDel(&db);
    ajStrDel(&desc);

    return ajTrue;
}




/* @funcstatic seqReadGifasta *************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using NCBI format. However, unlike NCBI format it uses the GI number
** as the sequence ID
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadGifasta(AjPSeq thys, AjPSeqin seqin)
{
    AjPStrTok handle = NULL;
    AjPStr id        = NULL;
    AjPStr acc       = NULL;
    AjPStr sv        = NULL;
    AjPStr gi        = NULL;
    AjPStr db        = NULL;
    AjPStr desc      = NULL;

    AjPFilebuff buff;

    ajuint bufflines = 0;
    AjBool ok;


    buff = seqin->Filebuff;

    ok = ajBuffreadLineStore(buff, &seqReadLine,
			    seqin->Text, &thys->TextPtr);
    if(!ok)
	return ajFalse;

    ajStrAssignClear(&id);
    ajStrAssignClear(&acc);
    ajStrAssignClear(&sv);
    ajStrAssignClear(&gi);
    ajStrAssignClear(&desc);


    if(!ajSeqParseNcbi(seqReadLine,&id,&acc,&sv,&gi,&db,&desc) ||
       !ajStrGetLen(gi))
    {
	ajFilebuffReset(buff);
	ajStrDel(&id);
	ajStrDel(&acc);
	ajStrDel(&sv);
	ajStrDel(&gi);
	ajStrDel(&desc);
	return ajFalse;
    }

    ajDebug("parsed id '%S' acc '%S' sv '%S' gi '%S' db '%S' (%S) desc '%S'\n",
	    id, acc, sv, gi, db, thys->Setdb, desc);

    ajStrAssignS(&thys->Gi, gi);
    
    ajStrAssignS(&thys->Setdb, db);
    ajDebug("set setdb '%S' db '%S'\n", thys->Setdb, thys->Db);

    if(ajStrGetLen(sv))
	seqSvSave(thys, sv);

    if(ajStrGetLen(acc))
	seqAccSave(thys, acc);

    seqSetName(&thys->Name, gi);
    ajStrAssignS(&thys->Desc, desc);


    if(ajStrGetLen(seqin->Inseq))
    {				       /* we have a sequence to use */
	ajStrAssignS(&thys->Seq, seqin->Inseq);
	if(seqin->Text)
	    seqTextSeq(&thys->TextPtr, seqin->Inseq);

	ajFilebuffClearStore(buff, 1,
			     seqReadLine, seqin->Text, &thys->TextPtr);
    }
    else
    {
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);



	while(ok && !ajStrPrefixC(seqReadLine, ">"))
	{
	    seqAppend(&thys->Seq, seqReadLine);
	    bufflines++;
	    ok = ajBuffreadLineStore(buff, &seqReadLine,
				    seqin->Text, &thys->TextPtr);
	}

	if(ok)
	    ajFilebuffClearStore(buff, 1,
				 seqReadLine, seqin->Text, &thys->TextPtr);
	else
	    ajFilebuffClear(buff, 0);
    }

    ajStrTokenDel(&handle);
    ajStrDel(&id);
    ajStrDel(&acc);
    ajStrDel(&sv);
    ajStrDel(&gi);
    ajStrDel(&desc);

    return ajTrue;
}

/* @funcstatic seqReadSelex ***************************************************
**
** Read a Selex file. Assumed a comment on the first line but this may
** not be true.
**
** This format can read anything that looks like a block of "name sequence"
** data. The names are even allowed to change in later blocks.
**
** The format was used by HMMER, but that package now prefers the better
** annotated "Stockholm" format used by Pfam and Rfam.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadSelex(AjPSeq thys, AjPSeqin seqin)
{
    AjPFilebuff buff  = seqin->Filebuff;
    AjPStr      line  = NULL;
    SeqPSelex    selex;
    ajuint       n      = 0;
    const char  *p     = NULL;
    AjBool      ok     = ajFalse;
    AjBool      isseq  = ajFalse;
    AjBool      named  = ajFalse;
    AjBool      head   = ajTrue;
    ajuint       sqcnt  = 0;
    ajuint       i;
    char        c      = '\0';
    AjBool      first  = ajTrue;

    line = ajStrNew();


    if(seqin->Data)
	selex = seqin->Data;
    else
    {
	ajFilebuffSetBuffered(buff);    /* must buffer to test sequences */

	/* First count the sequences, and get any header information */
	while(!isseq && (ok=ajBuffreadLine(buff,&line)))
	{
	    if(first)
	    {
		first=ajFalse;
		if(!ajStrPrefixC(line,"#"))
		{
		    ajStrDel(&line);
		    ajFilebuffReset(buff);
		    return ajFalse;
		}
	    }
	    ajStrRemoveWhiteExcess(&line);
	    p = ajStrGetPtr(line);
	    if(!*p || *p=='#')
		continue;
	    else
		isseq = ajTrue;
	}

	if(!ok && !isseq)
	    return ajFalse;
	++n;

	ok = ajTrue;
	while(ok && ajBuffreadLine(buff,&line))
	{
	    ajStrRemoveWhiteExcess(&line);
	    p = ajStrGetPtr(line);
	    if(*p=='#')
		continue;
	    if(!*p)
		ok = ajFalse;
	    else
		++n;
	}

	ajFilebuffClear(buff,-1);
	ajFilebuffReset(buff);
	buff->Fpos = 0;
	ajFileSeek(buff->File, 0L, 0);
	selex = selexNew(n);

	/* now read it for real */

	while(head && ajBuffreadLineStore(buff,&line,
					 seqin->Text, &thys->TextPtr))
	{
	    if(ajStrPrefixC(line,"#=RF") ||ajStrPrefixC(line,"#=CS"))
		break;

	    if(ajStrPrefixC(line,"#="))
	    {
		head=seqSelexHeader(&selex,line,&named,&sqcnt);
		continue;
	    }
	    c = *ajStrGetPtr(line);
	    if(c>='0')
		head = ajFalse;
	}

	/* Should now be at start of first block, whether RF or sequence */
	ajDebug("First Block Line: %S",line);

	ok = ajTrue;
	while(ok && !ajStrPrefixC(line, "# ID"))
	{
	    seqSelexReadBlock(&selex,&named,n,&line,buff,
			      seqin->Text, &thys->TextPtr);
	    ok = ajBuffreadLineStore(buff,&line,
				     seqin->Text, &thys->TextPtr);
	    ajDebug("... in loop ok: %B\n", ok);
	}
	ajDebug(" Block done. More data (ok): %B\n", ok);
	if(ok)
	    ajFilebuffClearStore(buff, 1,
				 line, seqin->Text, &thys->TextPtr);
	else
	    ajFilebuffClear(buff, 0);

	seqin->Data = selex;
    }


    /* At this point the Selex structure is fully loaded */
    if(selex->Count >= selex->n)
    {
	selexDel(&selex);
	seqin->Data = NULL;
	ajStrDel(&line);
	return ajFalse;
    }

    i = selex->Count;

    seqSelexCopy(&thys,selex,i);

    ++selex->Count;

    ajFilebuffClear(buff,0);

    ajStrDel(&line);

    return ajTrue;
}




/* @funcstatic seqReadStockholm ***********************************************
**
** Read a Stockholm file.
**
** @param [w] thys [AjPSeq] Stockholm input file
** @param [u] seqin [AjPSeqin] seqin object
** @return [AjBool] ajTrue if success
** @@
******************************************************************************/

static AjBool seqReadStockholm(AjPSeq thys, AjPSeqin seqin)
{
    AjPFilebuff buff  = seqin->Filebuff;
    AjPStr      line  = NULL;
    AjPStr      word  = NULL;
    AjPStr      token = NULL;
    AjPStr      post  = NULL;
    AjPStr      namstr = NULL;
    AjPStr      seqstr = NULL;
    AjBool      ok    = ajFalse;
    AjBool      bmf   = ajTrue;
    AjBool      dcf   = ajTrue;
    AjBool      drf   = ajTrue;
    AjBool      ccf   = ajTrue;
    AjBool      gsf   = ajTrue;
    AjBool      reff  = ajTrue;

    SeqPStockholm stock = NULL;

    ajuint i     = 0;
    ajuint n     = 0;
    ajlong lpos = 0L;
    ajuint  scnt = 0;

    line = ajStrNew();

    ajDebug("seqReadStockholm EOF:%B Data:%x\n",
	    ajFilebuffIsEof(buff), seqin->Data);
    if(seqin->Data)
	stock = seqin->Data;
    else
    {
	ajFilebuffSetBuffered(buff); 		/* must buffer to test sequences */
	ajFilebuffTraceFull(buff, 20, 0);
	lpos = ajFileResetPos(buff->File);
	ok=ajBuffreadLineStore(buff,&line, seqin->Text, &thys->TextPtr);

	if(!ok || !ajStrPrefixC(line,"# STOCKHOLM 1."))
	{
	    if (ok)
		ajDebug("Stockholm: bad first line: %S", line);
	    else
		ajDebug("Stockholm: no first line\n");
	    ajFilebuffReset(buff);
	    ajStrDel(&line);
	    return ajFalse;
	}

	ajDebug("Stockholm: good first line: %S", line);

	while(ok && (ajStrPrefixC(line, "#") || ajStrMatchC(line, "\n")))
	{
	    if(ajStrPrefixC(line,"#=GF SQ"))
	    {
		ajFmtScanS(line,"%*s%*s%d",&n);
		ajDebug("Stockholm: parsed SQ line of %d sequences\n", n);
	    }
	    ok=ajBuffreadLineStore(buff,&line, seqin->Text, &thys->TextPtr);
	    ajDebug("Stockholm: SQ search: %S", line);
	}

	if (!n)				/* no SQ line, count first block */
	{
	    while(ok && !ajStrMatchC(line, "\n"))
	    {
		n++;
		ok=ajBuffreadLineStore(buff,&line, seqin->Text, &thys->TextPtr);
		ajDebug("Stockholm: block %d read: %S", n, line);
	    }
	    ajDebug("Stockholm: read block of %d sequences\n", n);
	}
	ajFileSeek(buff->File,lpos,0);
	ajFilebuffClear(buff,-1);
	ajFilebuffReset(buff);

	/* Commented out by jison ... was causing incorrect parsing for input file
	   from HMMER tutorial. */
        /* ok=ajBuffreadLineStore(buff,&line,
	                       seqin->Text, &thys->TextPtr); */
	ok=ajBuffreadLineStore(buff,&line,
			       seqin->Text, &thys->TextPtr);
	stock = stockholmNew(n);

	ajDebug("Created stockholm data object size: %d\n", n);

	word  = ajStrNew();
	token = ajStrNew();
	post  = ajStrNew();

	if(!seqRegStockholmSeq)
	    seqRegStockholmSeq = ajRegCompC("^([^ \t\n]+)[ \t]+([^ \t\n]+)[ \t]+");
	while(ok && !ajStrPrefixC(line,"//"))
	{
	    if(ajRegExec(seqRegStockholmSeq,line))
	    {
		ajRegSubI(seqRegStockholmSeq,1,&word);
		ajRegSubI(seqRegStockholmSeq,2,&token);
		ajRegPost(seqRegStockholmSeq,&post);
		ajStrRemoveLastNewline(&post);

		if(!ajStrCmpC(word,"#=GF"))
		{
		    if(!ajStrCmpC(token,"ID"))
			ajStrAssignS(&stock->id,post);
		    else if(!ajStrCmpC(token,"AC"))
			ajStrAssignS(&stock->ac,post);
		    else if(!ajStrCmpC(token,"DE"))
			ajStrAssignS(&stock->de,post);
		    else if(!ajStrCmpC(token,"AU"))
			ajStrAssignS(&stock->au,post);
		    else if(!ajStrCmpC(token,"AL"))
			ajStrAssignS(&stock->al,post);
		    else if(!ajStrCmpC(token,"SE"))
			ajStrAssignS(&stock->se,post);
		    else if(!ajStrCmpC(token,"TP"))
			ajStrAssignS(&stock->se,post);
		    else if(!ajStrCmpC(token,"GA"))
			ajFmtScanS(post,"%d%d",&stock->ga[0],
				   &stock->ga[1]);
		    else if(!ajStrCmpC(token,"TC"))
			ajFmtScanS(post,"%f%f",&stock->tc[0],
				   &stock->tc[1]);
		    else if(!ajStrCmpC(token,"NC"))
			ajFmtScanS(post,"%f%f",&stock->nc[0],
				   &stock->nc[1]);
		    else if(!ajStrCmpC(token,"BM"))
		    {
			if(bmf)
			{
			    bmf = ajFalse;
			    ajStrAssignS(&stock->bm,line);
			}
			else
			    ajStrAppendS(&stock->bm,line);
		    }
		    else if(!ajStrCmpC(token,"DC"))
		    {
			if(dcf)
			{
			    dcf = ajFalse;
			    ajStrAssignS(&stock->dc,line);
			}
			else
			    ajStrAppendS(&stock->dc,line);
		    }
		    else if(!ajStrCmpC(token,"DR"))
		    {
			if(drf)
			{
			    drf = ajFalse;
			    ajStrAssignS(&stock->dr,line);
			}
			else
			    ajStrAppendS(&stock->dr,line);
		    }
		    else if(!ajStrCmpC(token,"CC"))
		    {
			if(ccf)
			{
			    ccf = ajFalse;
			    ajStrAssignS(&stock->cc,line);
			}
			else
			    ajStrAppendS(&stock->cc,line);
		    }
		    else if(*ajStrGetPtr(token)=='R')
		    {
			if(reff)
			{
			    reff = ajFalse;
			    ajStrAssignS(&stock->ref,line);
			}
			else
			    ajStrAppendS(&stock->ref,line);
		    }
		}

		if(!ajStrCmpC(word,"#=GS"))
		{
		    if(gsf)
		    {
			gsf = ajFalse;
			ajStrAssignS(&stock->gs,line);
		    }
		    else
			ajStrAppendS(&stock->gs,line);
		}

		if(!ajStrCmpC(word,"#=GC"))
		{
		    if(!ajStrCmpC(token,"SS_cons"))
			ajStrAssignS(&stock->sscons,post);
		    else if(!ajStrCmpC(token,"SA_cons"))
			ajStrAssignS(&stock->sacons,post);
		}

	    }
	    else if (!ajStrMatchC(line, "\n"))
	    {
		ajFmtScanS(line,"%S%S", &namstr,&seqstr);
		if(!ajStrGetLen(stock->name[scnt]))
		    ajStrAppendS(&stock->name[scnt], namstr);
		else
		{
		    if(!ajStrMatchS(namstr, stock->name[scnt]))
			ajWarn("Bad stockholm format found id %d '%S' expect '%S'",
			       scnt, namstr, stock->name[scnt]);
		}
		ajStrRemoveLastNewline(&seqstr);
		ajStrAppendS(&stock->str[scnt], seqstr);
		++scnt;
		if(scnt >= n)
		    scnt = 0;
	    }

	    ok = ajBuffreadLineStore(buff,&line,
				     seqin->Text, &thys->TextPtr);
	}
	while(ok && !ajStrPrefixC(line, "# STOCKHOLM 1."))
	{
	    ok = ajBuffreadLineStore(buff,&line,
				     seqin->Text, &thys->TextPtr);
	}
	if(ok)
	    ajFilebuffClearStore(buff, 1,
				 line, seqin->Text, &thys->TextPtr);
	else
	    ajFilebuffClear(buff, 0);

	ajStrDel(&word);
	ajStrDel(&token);
	ajStrDel(&post);
	ajStrDel(&namstr);
	ajStrDel(&seqstr);
	seqin->Data = stock;
    }


    /* At this point the Stockholm structure is fully loaded */
    if(stock->Count >= stock->n)
    {
	ajDebug("Stockholm count %d: All done\n", stock->Count);
	stockholmDel(&stock);
	seqin->Data = NULL;
	ajStrDel(&line);
	return ajFalse;
    }

    i = stock->Count;

    seqStockholmCopy(&thys,stock,i);


    ++stock->Count;



    


    ajFilebuffClear(buff,0);

    ajStrDel(&line);

    return ajTrue;
}




/* @funcstatic seqSelexCopy ***************************************************
**
** Copy Selex data to sequence object.
** Pad with gaps to make lengths equal.
**
** @param [w] thys [AjPSeq*] sequence object
** @param [u] selex [SeqPSelex] seqin containing selex info
** @param [r] n [ajuint] index into selex object
** @return [void]
** @@
******************************************************************************/

static void seqSelexCopy(AjPSeq *thys, SeqPSelex selex, ajuint n)
{
    AjPSeq pthis   = *thys;
    /*SeqPSelexdata sdata;*/

    ajStrAssignS(&pthis->Seq, selex->str[n]);
    ajStrAssignS(&pthis->Name, selex->name[n]);
    pthis->Weight = selex->sq[n]->wt;

/*
    if(!(*thys)->Selexdata)
	(*thys)->Selexdata = selexdataNew();

    sdata = (*thys)->Selexdata;

    ajStrAssignS(&sdata->id,selex->id);
    ajStrAssignS(&sdata->ac,selex->ac);
    ajStrAssignS(&sdata->de,selex->de);
    ajStrAssignS(&sdata->au,selex->au);
    ajStrAssignS(&sdata->cs,selex->cs);
    ajStrAssignS(&sdata->rf,selex->rf);
    ajStrAssignS(&sdata->name,selex->name[n]);
    ajStrAssignS(&sdata->str,selex->str[n]);
    ajStrAssignS(&sdata->ss,selex->ss[n]);

    sdata->ga[0] = selex->ga[0];
    sdata->ga[1] = selex->ga[1];
    sdata->tc[0] = selex->tc[0];
    sdata->tc[1] = selex->tc[1];
    sdata->nc[0] = selex->nc[0];
    sdata->nc[1] = selex->nc[1];

    ajStrAssignS(&sdata->sq->name,selex->sq[n]->name);

    ajStrAssignS(&sdata->sq->ac,selex->sq[n]->ac);
    ajStrAssignS(&sdata->sq->source,selex->sq[n]->source);
    ajStrAssignS(&sdata->sq->de,selex->sq[n]->de);

    sdata->sq->wt    = selex->sq[n]->wt;
    sdata->sq->start = selex->sq[n]->start;
    sdata->sq->stop  = selex->sq[n]->stop;
    sdata->sq->len   = selex->sq[n]->len;
*/
    return;
}




/* @funcstatic seqStockholmCopy ***********************************************
**
** Copy Stockholm data to sequence object.
** Pad with gaps to make lengths equal.
**
** @param [w] thys [AjPSeq*] sequence object
** @param [u] stock [SeqPStockholm] seqin containing selex info
** @param [r] n [ajint] index into stockholm object
** @return [void]
** @@
******************************************************************************/

static void seqStockholmCopy(AjPSeq *thys, SeqPStockholm stock, ajint n)
{
    AjPSeq pthis;
    /*SeqPStockholmdata sdata;*/

    pthis = *thys;

    ajStrAssignS(&pthis->Seq, stock->str[n]);
    ajStrAssignS(&pthis->Name, stock->name[n]);

/*
    if(!(*thys)->Stock)
	(*thys)->Stock = stockholmdataNew();

    sdata = (*thys)->Stock;

    ajStrAssignS(&sdata->id,stock->id);
    ajStrAssignS(&sdata->ac,stock->ac);
    ajStrAssignS(&sdata->de,stock->de);
    ajStrAssignS(&sdata->au,stock->au);
    ajStrAssignS(&sdata->al,stock->al);
    ajStrAssignS(&sdata->tp,stock->tp);
    ajStrAssignS(&sdata->se,stock->se);
    ajStrAssignS(&sdata->gs,stock->gs);
    ajStrAssignS(&sdata->dc,stock->dc);
    ajStrAssignS(&sdata->dr,stock->dr);
    ajStrAssignS(&sdata->cc,stock->cc);
    ajStrAssignS(&sdata->ref,stock->ref);
    ajStrAssignS(&sdata->sacons,stock->sacons);
    ajStrAssignS(&sdata->sscons,stock->sscons);
    sdata->ga[0] = stock->ga[0];
    sdata->ga[1] = stock->ga[1];
    sdata->tc[0] = stock->tc[0];
    sdata->tc[1] = stock->tc[1];
    sdata->nc[0] = stock->nc[0];
    sdata->nc[1] = stock->nc[1];
*/
    return;
}




/* @funcstatic seqSelexAppend *************************************************
**
** Append sequence and related Selex info to selex object.
** Pad with gaps to make lengths equal.
**
** @param [r] src [const AjPStr] source line from Selex file
** @param [w] dest [AjPStr*] Destination in Selex object
** @param [r] beg  [ajuint] start of info in src
** @param [r] end  [ajuint] end of info in src
** @return [void]
** @@
******************************************************************************/

static void seqSelexAppend(const AjPStr src, AjPStr *dest,
			   ajuint beg, ajuint end)
{
    const char *p = NULL;
    char c;
    ajuint len;
    ajuint i;
    ajuint pad = 0;

    len = end-beg+1;
    p   = ajStrGetPtr(src);

    ajDebug("seqSelexAppend srclen: %u beg: %u end: %u src '%S'\n",
	    ajStrGetLen(src), beg, end, src);

    if(beg>=ajStrGetLen(src))
    {
	for(i=0;i<len;++i)
	    ajStrAppendK(dest,'-');
	return;
    }

    p += beg;
    pad = end - ajStrGetLen(src) + 2;

    while((c=*p) && *p!='\n')
    {
	if(c=='.' || c=='_' || c==' ')
	    c='-';
	ajStrAppendK(dest,c);
	++p;
    }

    for(i=0;i<pad;++i)
	ajStrAppendK(dest,'-');

    return;
}




/* @funcstatic seqSelexHeader *************************************************
**
** Load a Selex object with header information for a single line
**
** @param [w] thys [SeqPSelex*] Selex object
** @param [r] line [const AjPStr] Selex header line
** @param [w] named  [AjBool*] Whether names of sequences have been read
** @param [w] sqcnt  [ajuint*] Number of SQ names read
** @return [AjBool] ajTrue if the line contained header information
** @@
******************************************************************************/

static AjBool seqSelexHeader(SeqPSelex *thys, const AjPStr line,
			     AjBool *named, ajuint *sqcnt)
{
    SeqPSelex pthis;
    AjPStrTok token = NULL;
    AjPStr handle   = NULL;


    pthis = *thys;


    if(ajStrPrefixC(line,"#=ID"))
    {
	ajFmtScanS(line,"#=ID %S",&pthis->id);
	return ajTrue;
    }
    else if(ajStrPrefixC(line,"#=AC"))
    {
	ajFmtScanS(line,"#=AC %S",&pthis->ac);
	return ajTrue;
    }
    else if(ajStrPrefixC(line,"#=DE"))
    {
	ajStrAssignC(&pthis->de,ajStrGetPtr(line)+5);
	ajStrRemoveWhiteExcess(&pthis->de);
	return ajTrue;
    }
    else if(ajStrPrefixC(line,"#=AU"))
    {
	ajStrAssignC(&pthis->au,ajStrGetPtr(line)+5);
	ajStrRemoveWhiteExcess(&pthis->au);
	return ajTrue;
    }
    else if(ajStrPrefixC(line,"#=GA"))
    {
	ajFmtScanS(line,"%*s %f %f",&pthis->ga[0],&pthis->ga[1]);
	return ajTrue;
    }
    else if(ajStrPrefixC(line,"#=TC"))
    {
	ajFmtScanS(line,"%*s %f %f",&pthis->tc[0],&pthis->tc[1]);
	return ajTrue;
    }
    else if(ajStrPrefixC(line,"#=NC"))
    {
	ajFmtScanS(line,"%*s %f %f",&pthis->nc[0],&pthis->nc[1]);
	return ajTrue;
    }
    else if(ajStrPrefixC(line,"#=SQ"))
    {
	handle = ajStrNew();
	token = ajStrTokenNewC(line," \t\n");
	ajStrTokenNextParse(&token,&handle);

	ajStrTokenNextParse(&token,&pthis->sq[*sqcnt]->name);
	ajStrAssignS(&pthis->name[*sqcnt],pthis->sq[*sqcnt]->name);

	ajStrTokenNextParse(&token,&handle);
	ajStrToFloat(handle,&pthis->sq[*sqcnt]->wt);

	ajStrTokenNextParse(&token,&handle);
	ajStrAssignS(&pthis->sq[*sqcnt]->source,handle);

	ajStrTokenNextParse(&token,&handle);
	ajStrAssignS(&pthis->sq[*sqcnt]->ac,handle);

	ajStrTokenNextParse(&token,&handle);
	ajFmtScanS(handle,"%d..%d:%d",&pthis->sq[*sqcnt]->start,
		   &pthis->sq[*sqcnt]->stop,&pthis->sq[*sqcnt]->len);

	ajStrTokenNextParseC(&token,"\n",&handle);
	ajStrAssignS(&pthis->sq[*sqcnt]->de,handle);

	ajStrTokenDel(&token);
	ajStrDel(&handle);
	*named = ajTrue;
	++(*sqcnt);
	return ajTrue;
    }


    return ajFalse;
}




/* @funcstatic seqSelexPos ****************************************************
**
** Find start and end positions of sequence & related Selex information
**
** @param [r] line [const AjPStr] Selex sequence or related line
** @param [w] begin  [ajuint*] start pos
** @param [w] end  [ajuint*] end pos
** @return [void]
** @@
******************************************************************************/

static void seqSelexPos(const AjPStr line, ajuint *begin, ajuint *end)
{
    ajuint pos = 0;
    ajuint len = 0;

    const char  *p;

    /*
    **  Selex sequence info can start any number of spaces
    **  after the names so we need to find out where to
    **  start counting chars from and where to end
     */

    len  = ajStrGetLen(line) - 1;
    if(!len) 
    {
        *begin=0;
        *end=0;
        return;
    }
    
    pos  = len -1;
    *end = (pos > *end) ? pos : *end;
    p = ajStrGetPtr(line);

    while(*p && *p!=' ')
	++p;
    while(*p && *p==' ')
	++p;
    if(p)
	pos = p - ajStrGetPtr(line);
    *begin = (pos < *begin) ? pos : *begin;

    ajDebug("seqSelexPos len:%u pos:%u begin:%u end:%u\n",
            len, pos, *begin, *end);

    return;
}




/* @funcstatic seqSelexReadBlock **********************************************
**
** Read a block of sequence information from a selex file
**
** @param [w] thys [SeqPSelex*] Selex object
** @param [w] named  [AjBool*] Whether names of sequences have been read
** @param [r] n  [ajuint] Number of sequences in Selex file
** @param [u] line [AjPStr*] Line from Selex file
** @param [u] buff  [AjPFilebuff] Selex file buffer
** @param [r] store [AjBool] store if ajTrue
** @param [w] astr [AjPStr*] string to append to
** @return [AjBool] ajTrue if data was read.
** @@
******************************************************************************/

static AjBool seqSelexReadBlock(SeqPSelex *thys, AjBool *named, ajuint n,
				AjPStr *line, AjPFilebuff buff,
				AjBool store, AjPStr *astr)
{
    SeqPSelex pthis;
    AjPStr *seqs = NULL;
    AjPStr *ss   = NULL;

    AjPStr rf = NULL;
    AjPStr cs = NULL;
    ajuint  i;
    ajuint  begin=0;
    ajuint  end=0;
    AjBool ok;
    ajuint  cnt;
    AjPStr tmp    = NULL;
    AjBool haverf = ajFalse;
    AjBool havecs = ajFalse;
    AjBool havess = ajFalse;

    pthis = *thys;

    begin = INT_MAX;
    end   = 0;

    tmp = ajStrNew();
    rf = ajStrNew();
    cs = ajStrNew();
    AJCNEW(seqs,n);
    AJCNEW(ss,n);
    for(i=0;i<n;++i)
    {
	seqs[i] = ajStrNew();
	ss[i]  = ajStrNew();
    }

    ok = ajTrue;
    cnt = 0;


    while(ajStrPrefixC(*line,"\n"))
        ok = ajBuffreadLineStore(buff,line, store, astr);
   
    while(ok)
    {
	seqSelexPos(*line,&begin,&end);
	if(ajStrPrefixC(*line,"#=RF"))
	{
	    haverf=ajTrue;
	    ajStrAssignS(&rf,*line);
	}

	if(ajStrPrefixC(*line,"#=CS"))
	{
	    havecs=ajTrue;
	    ajStrAssignS(&cs,*line);
	}

	if(ajStrPrefixC(*line,"#=SS"))
	{
	    havess=ajTrue;
	    ajStrAssignS(&ss[--cnt],*line);
	    ++cnt;
	}

	if(!ajStrPrefixC(*line,"#"))
	{
	    if(!*named)
	    {
		ajFmtScanS(*line,"%S",&pthis->name[cnt]);
		ajStrAssignS(&pthis->sq[cnt]->name,pthis->name[cnt]);
	    }
	    else
	    {
		ajFmtScanS(*line,"%S",&tmp);
		if(!ajStrPrefixS(pthis->name[cnt],tmp))
		    ajWarn("Selex format sequence names do not match ['%S' '%S']",
			   pthis->name[cnt],tmp);
	    }

	    ajStrAssignS(&seqs[cnt],*line);
	    ++cnt;
	}

	ok = ajBuffreadLineStore(buff,line, store, astr);
	if(ajStrPrefixC(*line,"\n"))
	    ok = ajFalse;
    }

    ajDebug("selexReadBlock block done line '%S' n: %u rf:%B cs:%B ss:%B\n",
	    *line, n, haverf, havecs, havess);

    if(cnt != n)
        ajWarn("Selex format expected %u sequences in block, found %u",
               n, cnt);
    if(cnt > n)
        cnt = n;

    if(haverf)
	seqSelexAppend(rf,&pthis->rf,begin,end);

    if(havecs)
	seqSelexAppend(cs,&pthis->cs,begin,end);

    for(i=0;i<cnt;++i)
    {
	seqSelexAppend(seqs[i],&pthis->str[i],begin,end);
	if(havess)
	    seqSelexAppend(ss[i],&pthis->ss[i],begin,end);
    }


    for(i=0;i<n;++i)
    {
	ajStrDel(&seqs[i]);
	ajStrDel(&ss[i]);
    }
    AJFREE(seqs);
    AJFREE(ss);

    ajStrDel(&rf);
    ajStrDel(&cs);
    ajStrDel(&tmp);

    *named = ajTrue;

    return ajTrue;
}




/* @funcstatic seqReadStaden **************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using the old Staden package file format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadStaden(AjPSeq thys, AjPSeqin seqin)
{
    AjPStr token  = NULL;
    ajuint bufflines      = 0;
    AjPFilebuff buff;
    AjBool incomment = ajFalse;

    buff = seqin->Filebuff;

    if(!seqRegStadenId)
	seqRegStadenId = ajRegCompC("^[<]([^>-]+)[-]*[>]");

    if(!ajBuffreadLineStore(buff, &seqReadLine,
			   seqin->Text, &thys->TextPtr))
	return ajFalse;
    bufflines++;

    if(ajRegExec(seqRegStadenId, seqReadLine))
    {
	ajRegSubI(seqRegStadenId, 1, &token);
	seqSetName(&thys->Name, token);
	ajDebug("seqReadStaden name '%S' token '%S'\n",
		thys->Name, token);
	ajRegPost(seqRegStadenId, &token);
	seqAppendCommented(&thys->Seq, &incomment, token);
	ajStrDel(&token);
    }
    else
    {
	seqSetName(&thys->Name, seqin->Filename);
	seqAppendCommented(&thys->Seq, &incomment, seqReadLine);
    }

    while(ajBuffreadLineStore(buff, &seqReadLine,
			     seqin->Text, &thys->TextPtr))
    {
	seqAppendCommented(&thys->Seq, &incomment, seqReadLine);
	bufflines++;
    }

    ajFilebuffClear(buff, 0);

    if(!bufflines) return ajFalse;

    return ajTrue;
}




/* @funcstatic seqReadText ****************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using plain text format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadText(AjPSeq thys, AjPSeqin seqin)
{
    ajuint bufflines      = 0;
    AjPFilebuff buff;

    ajDebug("seqReadText\n");

    buff = seqin->Filebuff;

    while(ajBuffreadLineStore(buff, &seqReadLine,
			     seqin->Text, &thys->TextPtr))
    {
	ajDebug("read '%S'\n", seqReadLine);
	seqAppend(&thys->Seq, seqReadLine);
	bufflines++;
    }

    ajDebug("read %d lines\n", bufflines);
    ajFilebuffClear(buff, 0);

    if(!bufflines)
	return ajFalse;

    seqSetNameFile(&thys->Name, seqin);

    return ajTrue;
}




/* @funcstatic seqReadRaw *****************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using raw format, which accepts only alphanumeric and whitespace
** characters or '-' for gap or '*' for a protein stop
** and rejects anything else.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadRaw(AjPSeq thys, AjPSeqin seqin)
{
    ajuint bufflines      = 0;
    AjPFilebuff buff;
    ajDebug("seqReadRaw\n");

    buff = seqin->Filebuff;

    if(!seqRegRawNonseq)
	seqRegRawNonseq = ajRegCompC("[^A-Za-z0-9 \t\n\r*-]");

    while(ajBuffreadLineStore(buff, &seqReadLine,
			     seqin->Text, &thys->TextPtr))
    {
	ajDebug("read '%S'\n", seqReadLine);
	if(ajRegExec(seqRegRawNonseq, seqReadLine))
	{
	    ajDebug("seqReadRaw: Bad character found in line: %S\n",
		    seqReadLine);
	    ajFilebuffReset(buff);
	    ajStrAssignClear(&thys->Seq);
	    return ajFalse;
	}
	seqAppend(&thys->Seq, seqReadLine);
	bufflines++;
    }

    ajDebug("read %d lines\n", bufflines);
    ajFilebuffClear(buff, 0);

    if(!bufflines)
	return ajFalse;

    return ajTrue;
}




/* @funcstatic seqReadIg ******************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using IntelliGenetics format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadIg(AjPSeq thys, AjPSeqin seqin)
{
    ajuint bufflines      = 0;
    AjPFilebuff buff;
    AjBool ok = ajTrue;

    buff = seqin->Filebuff;

    do
    {
	/* skip comments with ';' prefix */
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	bufflines++;
    } while(ok && ajStrPrefixC(seqReadLine, ";"));

    if(!ok)
	return ajFalse;

    ajStrAssignS(&thys->Name, seqReadLine);
    ajStrCutEnd(&thys->Name, 1);
    bufflines++;

    while(ajBuffreadLineStore(buff, &seqReadLine,
			     seqin->Text, &thys->TextPtr) &&
	  !ajStrPrefixC(seqReadLine, "\014"))
    {
	seqAppend(&thys->Seq, seqReadLine);
	bufflines++;
    }

    ajFilebuffClear(buff, 0);

    return ajTrue;
}




/* @funcstatic seqReadPdbseq **************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using PDB protein databank format using the SEQRES records.
**
** This is the original sequence, see seqReadPdb for parsing the ATOM records
** which give the sequence observed in the structure.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadPdbseq(AjPSeq thys, AjPSeqin seqin)
{
    ajuint bufflines      = 0;
    AjPFilebuff buff;
    AjPStrTok handle = NULL;
    AjPStr name  = NULL;
    AjPStr alnname  = NULL;
    AjPStr token = NULL;
    AjPStr chain = NULL;
    AjPTable alntable    = NULL;
    SeqPMsfItem alnitem  = NULL;
    AjPList alnlist      = NULL;
    SeqPMsfData alndata  = NULL;
    char aa;
    ajuint iseq = 0;
    ajuint nseq = 0;
    ajuint i;
    AjBool ok = ajTrue;

    buff = seqin->Filebuff;

    ajDebug("seqReadPeb seqin->Data %x\n", seqin->Data);

    if(!seqin->Data)
    {					/* start of file */
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	bufflines++;

	ajDebug("first line:\n'%S'\n", seqReadLine);

	if(!ajStrPrefixC(seqReadLine, "HEADER    "))
	{
	    ajFilebuffReset(buff);
	    return ajFalse;
	}

	ajStrAssignSubS(&name,seqReadLine, 62, 71);
	ajStrTrimWhite(&name);

	ajDebug("first line OK name '%S'\n", name);

	seqin->Data = AJNEW0(alndata);
	alndata->Table = alntable = ajTablestrNew();
	alnlist = ajListstrNew();
	seqin->Filecount = 0;

	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);

	while(ok && !ajStrMatchC(seqReadLine, "END"))
	{
	    bufflines++;
	    if(ajStrPrefixC(seqReadLine, "SEQRES"))
	    {
		ajStrKeepRange(&seqReadLine, 0,71);
		ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
		ajStrTokenNextParse(&handle, &token);	/* 'SEQRES' */

		ajStrTokenNextParse(&handle, &token);	/* number */
		ajStrToUint(token, &iseq);

		ajStrTokenNextParse(&handle, &chain);	/* chain letter */
		if(iseq == 1)
		{
		    nseq++;
		    ajFmtPrintS(&token, "%S-%S", name, chain);
		    AJNEW0(alnitem);
		    seqSetName(&alnitem->Name, token);
		    ajStrAssignS(&alnname, alnitem->Name);
		    alnitem->Weight = 1.0;
		    ajTablePut(alntable, alnname, alnitem);
		    alnname = NULL;
		    ajListstrPushAppend(alnlist, ajStrNewS(alnitem->Name));
		}
		while(ajStrTokenNextParse(&handle, &token))
		{
		    if(ajResidueFromTriplet(token,&aa))
		    {
			seqAppendK(&alnitem->Seq, aa);
		    }
		}

	    }

	    ok = ajBuffreadLineStore(buff, &seqReadLine,
				    seqin->Text, &thys->TextPtr);
	}

	ajDebug("PDB Entry has %d sequences\n", nseq);
	ajListstrTrace(alnlist);
	ajTableTrace(alntable);
	ajTableMap(alntable, seqMsfTabList, NULL);

	alndata->Names = AJCALLOC(nseq, sizeof(*alndata->Names));
	for(i=0; i < nseq; i++)
	{
	    ajListstrPop(alnlist, &alndata->Names[i]);
	    ajDebug("list [%d] '%S'\n", i, alndata->Names[i]);
	}
	ajListstrFreeData(&alnlist);

	ajTableMap(alntable, seqMsfTabList, NULL);
	alndata->Nseq = nseq;
	alndata->Count = 0;
	alndata->Bufflines = bufflines;
	ajDebug("PDB format read %d lines\n", bufflines);
    }

    alndata = seqin->Data;
    alntable = alndata->Table;
    if(alndata->Count >= alndata->Nseq)
    {					/* all done */
	ajFilebuffClear(seqin->Filebuff, 0);
	seqMsfDataDel((SeqPMsfData*) &seqin->Data);
	return ajFalse;
    }

    i = alndata->Count;
    ajDebug("returning [%d] '%S'\n", i, alndata->Names[i]);
    alnitem = ajTableFetch(alntable, alndata->Names[i]);
    ajStrAssignS(&thys->Name, alndata->Names[i]);

    thys->Weight = alnitem->Weight;
    ajStrAssignS(&thys->Seq, alnitem->Seq);

    alndata->Count++;

    ajStrDel(&token);
    ajStrDel(&name);

    return ajTrue;
}




/* @funcstatic seqReadPdb **************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using PDB protein databank format using ATOM records.
**
** See seqReadPdbseq for parsing the SEQRES records
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadPdb(AjPSeq thys, AjPSeqin seqin)
{
    ajuint bufflines      = 0;
    AjPFilebuff buff;
    AjPStr name  = NULL;
    AjPStr alnname  = NULL;
    AjPStr token = NULL;
    AjPStr chain = NULL;
    AjPTable alntable    = NULL;
    SeqPMsfItem alnitem  = NULL;
    AjPList alnlist      = NULL;
    SeqPMsfData alndata  = NULL;
    char aa;
    ajuint nseq = 0;
    ajuint i;
    AjBool ok = ajTrue;
    AjPStr aa3 = NULL;
    ajuint iaa = 0;
    ajuint lastaa = 0;

    buff = seqin->Filebuff;

    ajDebug("seqReadPeb seqin->Data %x\n", seqin->Data);

    if(!seqin->Data)
    {					/* start of file */
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	bufflines++;

	ajDebug("first line:\n'%S'\n", seqReadLine);

	if(!ajStrPrefixC(seqReadLine, "HEADER    "))
	{
	    ajFilebuffReset(buff);
	    return ajFalse;
	}

	ajStrAssignSubS(&name,seqReadLine, 62, 71);
	ajStrTrimWhite(&name);

	ajDebug("first line OK name '%S'\n", name);

	seqin->Data = AJNEW0(alndata);
	alndata->Table = alntable = ajTablestrNew();
	alnlist = ajListstrNew();
	seqin->Filecount = 0;

	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);

	while(ok && !ajStrMatchC(seqReadLine, "END"))
	{
	    bufflines++;
	    if(ajStrPrefixC(seqReadLine, "ATOM"))
	    {
		if(!alnitem)
		    AJNEW0(alnitem);

		ajStrKeepRange(&seqReadLine, 0,71);

		ajStrAssignSubS(&aa3, seqReadLine, 17, 19);
		ajStrAssignSubS(&chain, seqReadLine, 21, 21);
		ajStrAssignSubS(&token, seqReadLine, 22, 25);
		ajStrToUint(token, &iaa);

		if(iaa > lastaa)
		{
		    if(ajResidueFromTriplet(aa3,&aa))
			seqAppendK(&alnitem->Seq, aa);
		    else
			seqAppendK(&alnitem->Seq, 'X');
		    lastaa = iaa;
		}

	    }

	    else if(ajStrPrefixC(seqReadLine, "TER"))
	    {
		nseq++;
		ajFmtPrintS(&token, "%S-%S", name, chain);
		seqSetName(&alnitem->Name, token);
		ajStrAssignS(&alnname, alnitem->Name);
		alnitem->Weight = 1.0;
		ajTablePut(alntable, alnname, alnitem);
		alnname = NULL;
		ajListstrPushAppend(alnlist, ajStrNewS(alnitem->Name));
		alnitem = NULL;
		lastaa = 0;
	    }
	    ok = ajBuffreadLineStore(buff, &seqReadLine,
				    seqin->Text, &thys->TextPtr);
	}

	ajDebug("PDB Entry has %d sequences\n", nseq);
	ajListstrTrace(alnlist);
	ajTableTrace(alntable);
	ajTableMap(alntable, seqMsfTabList, NULL);

	alndata->Names = AJCALLOC(nseq, sizeof(*alndata->Names));
	for(i=0; i < nseq; i++)
	{
	    ajListstrPop(alnlist, &alndata->Names[i]);
	    ajDebug("list [%d] '%S'\n", i, alndata->Names[i]);
	}
	ajListstrFreeData(&alnlist);

	ajTableMap(alntable, seqMsfTabList, NULL);
	alndata->Nseq = nseq;
	alndata->Count = 0;
	alndata->Bufflines = bufflines;
	ajDebug("PDB format read %d lines\n", bufflines);
    }

    alndata = seqin->Data;
    alntable = alndata->Table;
    if(alndata->Count >= alndata->Nseq)
    {					/* all done */
	ajFilebuffClear(seqin->Filebuff, 0);
	seqMsfDataDel((SeqPMsfData*) &seqin->Data);
	return ajFalse;
    }

    i = alndata->Count;
    ajDebug("returning [%d] '%S'\n", i, alndata->Names[i]);
    alnitem = ajTableFetch(alntable, alndata->Names[i]);
    ajStrAssignS(&thys->Name, alndata->Names[i]);

    thys->Weight = alnitem->Weight;
    ajStrAssignS(&thys->Seq, alnitem->Seq);

    alndata->Count++;

    ajStrDel(&token);
    ajStrDel(&name);

    return ajTrue;
}




/* @funcstatic seqReadClustal *************************************************
**
** Tries to read input in Clustal ALN format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadClustal(AjPSeq thys, AjPSeqin seqin)
{
    AjPStr seqstr        = NULL;
    AjPStr name          = NULL;
    ajuint bufflines     = 0;
    AjBool ok            = ajFalse;
    ajuint iseq          = 0;
    AjPFilebuff buff     = seqin->Filebuff;
    AjPTable alntable    = NULL;
    SeqPMsfItem alnitem  = NULL;
    AjPList alnlist      = NULL;
    SeqPMsfData alndata  = NULL;

    ajuint i;

    ajDebug("seqReadClustal seqin->Data %x\n", seqin->Data);

    if(!seqin->Data)
    {					/* start of file */
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	bufflines++;
	if(!ok)
	    return ajFalse;

	ajDebug("first line:\n'%S'\n", seqReadLine);

	if(!ajStrPrefixC(seqReadLine, "CLUSTAL"))
	{
	    /* first line test */
	    ajFilebuffReset(buff);
	    return ajFalse;
	}

	ajDebug("first line OK: '%S'\n", seqReadLine);

	while(ok)
	{				/* skip blank lines */
	    ok = ajBuffreadLineStore(buff, &seqReadLine,
				    seqin->Text, &thys->TextPtr);
	    bufflines++;
	    if(!ajStrIsWhite(seqReadLine))
		break;
	}

	if(!ok)
	{
	    ajDebug("FAIL (blank lines only)\n");
	    ajFilebuffReset(buff);
	    return ajFalse;
	}

	seqin->Data = AJNEW0(alndata);
	alndata->Table = alntable = ajTablestrNew();
	alnlist = ajListstrNew();
	seqin->Filecount = 0;

	/* first set - create table */
	ok = ajTrue;
	while(ok && ajStrExtractFirst(seqReadLine, &seqstr, &name))
	{
	    AJNEW0(alnitem);
	    ajStrAssignS(&alnitem->Name, name);
	    alnitem->Weight = 1.0;
	    seqAppend(&alnitem->Seq, seqstr);

	    iseq++;
	    ajDebug("first set %d: '%S'\n line: '%S'\n",
		    iseq, name, seqReadLine);

	    ajTablePut(alntable, name, alnitem);
	    name = NULL;
	    ajListstrPushAppend(alnlist, ajStrNewS(alnitem->Name));

	    ok = ajBuffreadLineStore(buff, &seqReadLine,
				    seqin->Text, &thys->TextPtr);
	    bufflines++;
	}
	ajStrDel(&seqstr);

	ajDebug("Header has %d sequences\n", iseq);
	ajListstrTrace(alnlist);
	ajTableTrace(alntable);
	ajTableMap(alntable, seqMsfTabList, NULL);

	alndata->Names = AJCALLOC(iseq, sizeof(*alndata->Names));
	for(i=0; i < iseq; i++)
	{
	    ajListstrPop(alnlist, &alndata->Names[i]);
	    ajDebug("list [%d] '%S'\n", i, alndata->Names[i]);
	}
	ajListstrFreeData(&alnlist);

	while(ajBuffreadLineStore(buff, &seqReadLine,
				 seqin->Text, &thys->TextPtr))
	{				/* now read the rest */
	    bufflines++;
	    seqClustalReadseq(seqReadLine, alntable);
	}

	ajTableMap(alntable, seqMsfTabList, NULL);
	alndata->Nseq = iseq;
	alndata->Count = 0;
	alndata->Bufflines = bufflines;
	ajDebug("ALN format read %d lines\n", bufflines);
    }

    alndata = seqin->Data;
    alntable = alndata->Table;
    if(alndata->Count >= alndata->Nseq)
    {					/* all done */
	ajFilebuffClear(seqin->Filebuff, 0);
	seqMsfDataDel((SeqPMsfData*) &seqin->Data);
	return ajFalse;
    }
    i = alndata->Count;
    ajDebug("returning [%d] '%S'\n", i, alndata->Names[i]);
    alnitem = ajTableFetch(alntable, alndata->Names[i]);
    ajStrAssignS(&thys->Name, alndata->Names[i]);

    thys->Weight = alnitem->Weight;
    ajStrAssignS(&thys->Seq, alnitem->Seq);

    alndata->Count++;

    return ajTrue;
}




/* @funcstatic seqClustalReadseq **********************************************
**
** Reads sequence name from first token on the input line, and appends
** the sequence data to that sequence in the alntable structure.
**
** @param [r] rdline [const AjPStr] Line from input file.
** @param [r] msftable [const AjPTable] MSF format sequence table.
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqClustalReadseq(const AjPStr rdline, const AjPTable msftable)
{
    SeqPMsfItem msfitem;
    AjPStr token     = NULL;
    AjPStr seqstr    = NULL;

    if(!ajStrExtractFirst(rdline, &seqstr, &token))
	return ajFalse;

    msfitem = ajTableFetch(msftable, token);
    ajStrDel(&token);
    if(!msfitem)
    {
	ajStrDel(&seqstr);
	return ajFalse;
    }

    seqAppend(&msfitem->Seq, seqstr);
    ajStrDel(&seqstr);

    return ajTrue;
}




/* @funcstatic seqReadPhylipnon ***********************************************
**
** Tries to read input in Phylip non-interleaved format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadPhylipnon(AjPSeq thys, AjPSeqin seqin)
{
    AjPStr seqstr = NULL;
    AjPStr tmpstr = NULL;
    ajuint bufflines = 0;
    AjBool ok       = ajFalse;
    ajuint iseq      = 0;
    ajuint jseq      = 0;
    ajuint len       = 0;
    ajuint ilen      = 0;
    AjPFilebuff buff;

    AjPTable phytable        = NULL;
    SeqPMsfItem phyitem      = NULL;
    SeqPMsfData phydata      = NULL;
    ajuint i;
    AjBool done = ajFalse;

    ajDebug("seqReadPhylipnon seqin->Data %x\n", seqin->Data);

    buff = seqin->Filebuff;

    if(!seqRegPhylipTop)
	seqRegPhylipTop = ajRegCompC("^ *([0-9]+) +([0-9]+)");

    if(!seqRegPhylipHead)
	seqRegPhylipHead = ajRegCompC("^(..........) ?"); /* 10 chars */

    if(!seqin->Data)
    {					/* start of file */
	seqin->multidone = ajFalse;
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	if(!ok)
	    return ajFalse;
	bufflines++;

	ajDebug("first line:\n'%-20.20S'\n", seqReadLine);

	if(!ajRegExec(seqRegPhylipTop, seqReadLine))
	{				/* first line test */
	    ajFilebuffReset(buff);
	    return ajFalse;
	}

	ajRegSubI(seqRegPhylipTop, 1, &tmpstr);
	ajStrToUint(tmpstr, &iseq);
	ajDebug("seqRegPhylipTop1 '%S' %d\n", tmpstr, iseq);
	ajRegSubI(seqRegPhylipTop, 2, &tmpstr);
	ajStrToUint(tmpstr, &len);
	ajDebug("seqRegPhylipTop2 '%S' %d\n", tmpstr,len);
	ajDebug("first line OK: '%S' iseq: %d len: %d\n",
		seqReadLine, iseq, len);
	ajStrDel(&tmpstr);

	seqin->Data = AJNEW0(phydata);
	phydata->Table = phytable = ajTablestrNew();
	phydata->Names = AJCALLOC(iseq, sizeof(*phydata->Names));
	seqin->Filecount = 0;

	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	bufflines++;
	ilen = 0;
	while(ok && (jseq < iseq))
	{
	    /* first set - create table */
	    if(!ajRegExec(seqRegPhylipHead, seqReadLine))
	    {
		ajDebug("FAIL (not seqRegPhylipHead): '%S'\n", seqReadLine);
		ajFilebuffReset(buff);
		seqMsfDataDel((SeqPMsfData*) &seqin->Data);
		return ajFalse;
	    }
	    ajDebug("line: '%S'\n", seqReadLine);
	    ajRegSubI(seqRegPhylipHead, 1, &tmpstr);
	    if(!ajStrIsWhite(tmpstr)) {
		/* check previous sequence */
		if(jseq)
		{
		    if(ilen != len)
		    {
			ajDebug("phylipnon format length mismatch at %d "
				"(length %d)\n",
				len, ilen);
			seqMsfDataDel((SeqPMsfData*) &seqin->Data);
			ajStrDel(&tmpstr);
			return ajFalse;
		    }
		}
		/* new sequence */
		AJNEW0(phyitem);
		seqSetName(&phyitem->Name, tmpstr);
		ajStrAssignS(&phydata->Names[jseq], phyitem->Name);
		ajDebug("name: '%S' => '%S'\n", tmpstr, phyitem->Name);
		phyitem->Weight = 1.0;
		ajRegPost(seqRegPhylipHead, &seqstr);
		seqAppend(&phyitem->Seq, seqstr);
		ajStrDel(&seqstr);
		ilen = ajStrGetLen(phyitem->Seq);
		if(ilen == len)
		    done = ajTrue;
		else if(ilen > len)
		{
		    ajDebug("Phylipnon format: sequence %S "
			    "header size %d exceeded\n",
			    phyitem->Name, len);
		    seqMsfDataDel((SeqPMsfData*) &seqin->Data);
		    seqMsfItemDel(&phyitem);
		    ajStrDel(&tmpstr);
		    return ajFalse;
		}
		ajTablePut(phytable, ajStrNewS(phyitem->Name), phyitem);
		ajDebug("seq %d: (%d) '%-20.20S'\n", jseq, ilen, seqReadLine);
	    }
	    else {
		/* more sequence to append */
		if(seqPhylipReadseq(seqReadLine, phytable, phyitem->Name,
				    len, &ilen, &done))
		{
		    ajDebug("read to len %d\n", ilen);
		    if (done)
		    {
			jseq++;
		    }
		}

	    }
	    ajStrDel(&tmpstr);

	    if(jseq < iseq)
	    {
		ok = ajBuffreadLineStore(buff, &seqReadLine,
					seqin->Text, &thys->TextPtr);
		bufflines++;
	    }
	}
	if(ilen != len)
	{
	    ajDebug("phylipnon format final length mismatch at %d "
		    "(length %d)\n",
		    len, ilen);
	    seqMsfDataDel((SeqPMsfData*) &seqin->Data);
	    return ajFalse;
	}

	ajDebug("Header has %d sequences\n", jseq);
	ajTableTrace(phytable);
	ajTableMap(phytable, seqMsfTabList, NULL);

	phydata->Nseq = iseq;
	phydata->Count = 0;
	phydata->Bufflines = bufflines;
	ajDebug("PHYLIP format read %d lines\n", bufflines);
    }

    phydata = seqin->Data;
    phytable = phydata->Table;

    i = phydata->Count;
    ajDebug("returning [%d] '%S'\n", i, phydata->Names[i]);
    phyitem = ajTableFetch(phytable, phydata->Names[i]);
    ajStrAssignS(&thys->Name, phydata->Names[i]);
    ajStrDel(&phydata->Names[i]);

    thys->Weight = phyitem->Weight;
    ajStrAssignS(&thys->Seq, phyitem->Seq);
    ajStrDel(&phyitem->Seq);

    phydata->Count++;
    if(phydata->Count >= phydata->Nseq)
    {
	seqin->multidone = ajTrue;
	ajFilebuffClear(seqin->Filebuff, 0);
	ajDebug("seqReadPhylip multidone\n");
	seqMsfDataDel((SeqPMsfData*) &seqin->Data);
    }

    ajStrDel(&seqstr);
    ajStrDel(&tmpstr);
    seqMsfDataTrace(seqin->Data);

    return ajTrue;
}




/* @funcstatic seqReadPhylip **************************************************
**
** Tries to read input in Phylip interleaved format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadPhylip(AjPSeq thys, AjPSeqin seqin)
{
    AjPStr seqstr = NULL;
    AjPStr tmpstr = NULL;
    ajuint bufflines = 0;
    AjBool ok       = ajFalse;
    ajuint iseq      = 0;
    ajuint jseq      = 0;
    ajuint len       = 0;
    ajuint ilen      = 0;
    ajuint maxlen    = 0;
    AjPFilebuff buff;

    AjPTable phytable        = NULL;
    SeqPMsfItem phyitem      = NULL;
    AjPList phylist          = NULL;
    SeqPMsfData phydata      = NULL;
    ajuint i;
    AjBool done = ajFalse;

    ajDebug("seqReadPhylip seqin->Data %x\n", seqin->Data);

    buff = seqin->Filebuff;
    ajFilebuffSetBuffered(buff);    /* must buffer to test non-interleaved */

    if(!seqRegPhylipTop)
	seqRegPhylipTop = ajRegCompC("^ *([0-9]+) +([0-9]+)");

    if(!seqRegPhylipHead)
	seqRegPhylipHead = ajRegCompC("^(..........) ?"); /* 10 chars */

    if(!seqRegPhylipSeq)
	seqRegPhylipSeq = ajRegCompC("^[ \t\n\r]*$");

    if(!seqin->Data)
    {					/* start of file */
	seqin->multidone = ajFalse;
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	while (ok && ajStrIsWhite(seqReadLine))
	    ok = ajBuffreadLineStore(buff, &seqReadLine,
				    seqin->Text, &thys->TextPtr);

	if(!ok)
	    return ajFalse;
	bufflines++;

	/* ajDebug("first line:\n'%-20.20S'\n", seqReadLine);*/

	if(!ajRegExec(seqRegPhylipTop, seqReadLine))
	{				/* first line test */
	    ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);
	    return ajFalse;
	}

	ajRegSubI(seqRegPhylipTop, 1, &tmpstr);
	ajStrToUint(tmpstr, &iseq);
	ajRegSubI(seqRegPhylipTop, 2, &tmpstr);
	ajStrToUint(tmpstr, &len);
	ajStrDel(&tmpstr);
	/*ajDebug("first line OK: '%S' iseq: %d len: %d\n",
		seqReadLine, iseq, len);*/

	seqin->Data = AJNEW0(phydata);
	phydata->Table = phytable = ajTablestrNew();
	phylist = ajListstrNew();
	seqin->Filecount = 0;

	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	bufflines++;
	ilen = 0;
	while(ok && (jseq < iseq))
	{
	    /* first set - create table */
	    if(!ajRegExec(seqRegPhylipHead, seqReadLine))
	    {
		ajDebug("FAIL (not seqRegPhylipHead): '%S'\n", seqReadLine);
		ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);
		seqMsfDataDel((SeqPMsfData*) &seqin->Data);
		return ajFalse;
	    }
	    /* ajDebug("line: '%S'\n", seqReadLine); */
	    AJNEW0(phyitem);
	    ajRegSubI(seqRegPhylipHead, 1, &tmpstr);
	    seqSetName(&phyitem->Name, tmpstr);
	    ajStrDel(&tmpstr);
	    /* ajDebug("name: '%S' => '%S'\n", tmpstr, phyitem->Name); */
	    phyitem->Weight = 1.0;
	    ajRegPost(seqRegPhylipHead, &seqstr);
	    seqAppend(&phyitem->Seq, seqstr);
	    ajStrDel(&seqstr);
	    ilen = ajStrGetLen(phyitem->Seq);
	    if(ilen == len)
		done = ajTrue;
	    else if(ilen > len)
	    {
		ajDebug("Phylip format: sequence %S header size %d exceeded\n",
			phyitem->Name, len);
		ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);

		seqMsfDataDel((SeqPMsfData*) &seqin->Data);
		seqMsfItemDel(&phyitem);

		ajListstrFreeData(&phylist);

		return ajFalse;
	    }
	    if(ajStrIsWhite(phyitem->Name) ||
	       ajTableFetch(phytable, phyitem->Name))
	    {
		ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);
		ajDebug("phytable repeated name '%S'\n",
			phyitem->Name);

		seqMsfDataDel((SeqPMsfData*) &seqin->Data);
		seqMsfItemDel(&phyitem);

		ajListstrFreeData(&phylist);

		return seqReadPhylipnon(thys, seqin);
	    }
	    ajTablePut(phytable, ajStrNewS(phyitem->Name), phyitem);
	    ajListstrPushAppend(phylist, ajStrNewS(phyitem->Name));
	    ajDebug("added '%S' list:%u table:%u\n",
		    phyitem->Name, ajListGetLength(phylist),
		    ajTableGetLength(phytable));
	    if(!jseq)
		maxlen = ilen;
	    else
	    {
		if(ilen != maxlen)
		{
		    ajDebug("phylip format length mismatch in header "
			    "iseq: %d jseq: %d ilen: %d maxlen: %d\n",
			    iseq, jseq, ilen, maxlen);
		    ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);
		    ajDebug("phytable deleted size:%u\n",
			    ajTableGetLength(phytable));
		    seqMsfDataDel((SeqPMsfData*) &seqin->Data);
		    ajListstrFreeData(&phylist);
		    if(seqReadPhylipnon(thys, seqin))
			return ajTrue;
		    else {
			ajWarn("phylip format length mismatch in header");
			return ajFalse;
		    }
		}
	    }
	    jseq++;
	    /* ajDebug("first set %d: (%d) '%-20.20S'\n",
	       jseq, ilen, seqReadLine); */

	    if(jseq < iseq)
	    {
		ok = ajBuffreadLineStore(buff, &seqReadLine,
					seqin->Text, &thys->TextPtr);
		bufflines++;
	    }
	}

	/* ajDebug("Header has %d sequences\n", jseq);*/
	ajListstrTrace(phylist);
	ajTableTrace(phytable);
	ajTableMap(phytable, seqMsfTabList, NULL);

	phydata->Names = AJCALLOC(iseq, sizeof(*phydata->Names));
	for(i=0; i < iseq; i++)
	{
	    ajListstrPop(phylist, &phydata->Names[i]);
	    /* ajDebug("list [%d] '%S'\n", i, phydata->Names[i]); */
	}
	ajListstrFreeData(&phylist);

	if(ilen < len)
	{
	    jseq=0;
	    while(ajBuffreadLineStore(buff, &seqReadLine,
				     seqin->Text, &thys->TextPtr))
	    {				/* now read the rest */
		/* ajDebug("seqReadPhylip line '%S\n", seqReadLine); */
		bufflines++;
		if(seqPhylipReadseq(seqReadLine, phytable,
				    phydata->Names[jseq],
				    len, &ilen, &done))
		{
		    if(!jseq)
			maxlen = ilen;
		    else
		    {
			if(ilen != maxlen)
			{
			    ajDebug("phylip format length mismatch at %d "
				    "(length %d)\n",
				    maxlen, ilen);
			    ajFilebuffResetStore(buff,
						 seqin->Text, &thys->TextPtr);
			    seqMsfDataDel((SeqPMsfData*) &seqin->Data);
			    ajDebug("File reset, try seqReadPhylipnon\n");
			    return seqReadPhylipnon(thys, seqin);
			}
		    }

		    jseq++;
		    if(jseq == iseq) jseq = 0;
		    if(!jseq && done)
		    {
			/* ajDebug("seqReadPhylip set done\n"); */
			break;
		    }
		    done = ajTrue;	/* for end-of-file */
		}
	    }
	    if(!done)
	    {
		ajDebug("seqReadPhylip read failed, try seqReadPhylipnon\n");
		ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);
		seqMsfDataDel((SeqPMsfData*) &seqin->Data);
		return seqReadPhylipnon(thys, seqin);
	    }

	    if(jseq)
	    {
		ajDebug("Phylip format %d sequences partly read at end\n",
			iseq-jseq);
		ajFilebuffResetStore(buff, seqin->Text, &thys->TextPtr);
		seqMsfDataDel((SeqPMsfData*) &seqin->Data);
		return seqReadPhylipnon(thys, seqin);
	    }
	}

	ajTableMap(phytable, seqMsfTabList, NULL);
	phydata->Nseq = iseq;
	phydata->Count = 0;
	phydata->Bufflines = bufflines;
	/* ajDebug("PHYLIP format read %d lines\n", bufflines);*/
    }

    phydata = seqin->Data;
    phytable = phydata->Table;

    i = phydata->Count;
    /* ajDebug("returning [%d] '%S'\n", i, phydata->Names[i]); */
    phyitem = ajTableFetch(phytable, phydata->Names[i]);
    ajStrAssignS(&thys->Name, phydata->Names[i]);

    thys->Weight = phyitem->Weight;
    ajStrAssignS(&thys->Seq, phyitem->Seq);

    phydata->Count++;
    if(phydata->Count >= phydata->Nseq)
    {
	seqin->multidone = ajTrue;
	ajDebug("seqReadPhylip multidone\n");
	ajFilebuffClear(seqin->Filebuff, 0);
	seqMsfDataDel((SeqPMsfData*) &seqin->Data);
    }

    seqMsfDataTrace(seqin->Data);
    return ajTrue;
}




/* @funcstatic seqPhylipReadseq ***********************************************
**
** Reads sequence from the input line, and appends the sequence data
** to the named sequence in the phytable structure.
**
** @param [r] rdline [const AjPStr] Line from input file.
** @param [r] phytable [const AjPTable] MSF format sequence table.
** @param [r] token [const AjPStr] Name of sequence so it can append
** @param [r] len [ajuint] Final length of each sequence (from file header)
** @param [w] ilen [ajuint*] Length of each sequence so far
** @param [w] done [AjBool*] ajTrue if sequence was completed
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqPhylipReadseq(const AjPStr rdline, const AjPTable phytable,
			       const AjPStr token,
			       ajuint len, ajuint* ilen, AjBool* done)
{
    SeqPMsfItem phyitem;

    *done = ajFalse;
    if(!seqRegPhylipSeq2)
	seqRegPhylipSeq2 = ajRegCompC("[^ \t\n\r]");

    if(!ajRegExec(seqRegPhylipSeq2, rdline))
	return ajFalse;

    phyitem = ajTableFetch(phytable, token);
    if(!phyitem)
    {
	ajDebug("seqPhylipReadseq failed to find '%S' in phytable\n",
		token);
	return ajFalse;
    }

    seqAppend(&phyitem->Seq, rdline);
    *ilen = ajStrGetLen(phyitem->Seq);

    if(*ilen == len)
	*done = ajTrue;
    else if(*ilen > len)
    {
	ajDebug("Phylip format error, sequence %S length %d exceeded\n",
		token, len);
	return ajFalse;
    }

    ajDebug("seqPhylipReadSeq '%S' len: %d ilen: %d done: %B\n",
	    token, len, *ilen, *done);

    return ajTrue;
}




/* @funcstatic seqReadHennig86 ************************************************
**
** Tries to read input in Hennig86 format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadHennig86(AjPSeq thys, AjPSeqin seqin)
{
    AjPStr seqstr = NULL;
    AjPStr tmpstr = NULL;
    ajuint bufflines = 0;
    AjBool ok       = ajFalse;
    ajuint iseq      = 0;
    ajuint len       = 0;
    AjPFilebuff buff;
    AjPTable fmttable   = NULL;
    SeqPMsfItem fmtitem = NULL;
    AjPList fmtlist     = NULL;
    SeqPMsfData fmtdata = NULL;
    char *cp;

    ajuint i;
    ajuint jseq = 0;

    ajDebug("seqReadHennig86 seqin->Data %x\n", seqin->Data);

    buff = seqin->Filebuff;

    if(!seqRegHennigHead)
	seqRegHennigHead = ajRegCompC("[^1-4? \t]");

    if(!seqRegHennigTop)
	seqRegHennigTop = ajRegCompC("^ *([0-9]+) +([0-9]+)");

    if(!seqRegHennigBlank)
	seqRegHennigBlank = ajRegCompC("^[ \t\n\r]*$");

    if(!seqRegHennigSeq)
	seqRegHennigSeq = ajRegCompC("^([^ \t\n\r]+)");

    if(!seqin->Data)
    {
	/* start: load in file */
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	if(!ok)
	    return ajFalse;
	bufflines++;

	ajDebug("first line:\n'%S'\n", seqReadLine);

	if(!ajStrPrefixC(seqReadLine, "xread"))
	{
	    /* first line test */
	    ajFilebuffReset(buff);
	    return ajFalse;
	}

	ajDebug("first line OK: '%S'\n", seqReadLine);

	/* skip title line */
	for(i=0; i<2; i++)
	{
	    ok = ajBuffreadLineStore(buff, &seqReadLine,
				    seqin->Text, &thys->TextPtr);
	    bufflines++;

	    if(!ok)
	    {
		ajDebug("FAIL (bad header)\n");
		ajFilebuffReset(buff);
		return ajFalse;
	    }
	}

	if(!ajRegExec(seqRegHennigTop, seqReadLine))	/* first line test */
	    return ajFalse;

	ajRegSubI(seqRegHennigTop, 1, &tmpstr);
	ajStrToUint(tmpstr, &iseq);
	ajRegSubI(seqRegHennigTop, 2, &tmpstr);
	ajStrToUint(tmpstr, &len);
	ajDebug("first line OK: '%S' iseq: %d len: %d\n",
		seqReadLine, iseq, len);
	ajStrDel(&tmpstr);

	seqin->Data = AJNEW0(fmtdata);
	fmtdata->Table = fmttable = ajTablestrNew();
	fmtlist = ajListstrNew();
	seqin->Filecount = 0;

	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	bufflines++;
	while(ok && (jseq < iseq))
	{				/* first set - create table */
	    if(!ajRegExec(seqRegHennigHead, seqReadLine))
	    {
		ajDebug("FAIL (not seqRegHennigHead): '%S'\n", seqReadLine);
		return ajFalse;
	    }
	    AJNEW0(fmtitem);
	    ajStrAssignS(&fmtitem->Name, seqReadLine);
	    fmtitem->Weight = 1.0;
	    ok = ajBuffreadLineStore(buff, &seqReadLine,
				    seqin->Text, &thys->TextPtr);
	    bufflines++;
	    while(ok && ajRegExec(seqRegHennigSeq, seqReadLine))
	    {
		ajRegPost(seqRegHennigSeq, &seqstr);
		for(cp = ajStrGetuniquePtr(&seqstr); cp; cp++)
		    switch(*cp)
		    {
		    case 0: *cp = 'A';break;
		    case 1: *cp = 'T';break;
		    case 2: *cp = 'G';break;
		    case 3: *cp = 'C';break;
		    default: *cp = '.';break;
		    }

		seqAppend(&fmtitem->Seq, seqstr);
	    }
	    ajStrDel(&seqstr);

	    ajTablePut(fmttable, ajStrNewS(fmtitem->Name), fmtitem);
	    ajListstrPushAppend(fmtlist, ajStrNewS(fmtitem->Name));
	    jseq++;
	    ajDebug("first set %d: '%S'\n", jseq, seqReadLine);

	    ok = ajBuffreadLineStore(buff, &seqReadLine,
				    seqin->Text, &thys->TextPtr);
	    bufflines++;
	}

	ajDebug("Header has %d sequences\n", iseq);
	ajListstrTrace(fmtlist);
	ajTableTrace(fmttable);
	ajTableMap(fmttable, seqMsfTabList, NULL);

	fmtdata->Names = AJCALLOC(iseq, sizeof(*fmtdata->Names));
	for(i=0; i < iseq; i++)
	{
	    ajListstrPop(fmtlist, &fmtdata->Names[i]);
	    ajDebug("list [%d] '%S'\n", i, fmtdata->Names[i]);
	}
	ajListstrFreeData(&fmtlist);

	while(ajBuffreadLineStore(buff, &seqReadLine,
				 seqin->Text, &thys->TextPtr))
	{				/* now read the rest */
	    bufflines++;
	    seqHennig86Readseq(seqReadLine, fmttable);
	}

	ajTableMap(fmttable, seqMsfTabList, NULL);
	fmtdata->Nseq = iseq;
	fmtdata->Count = 0;
	fmtdata->Bufflines = bufflines;
	ajDebug("... format read %d lines\n", bufflines);
    }

    /* processing entries */

    fmtdata = seqin->Data;
    fmttable = fmtdata->Table;
    if(fmtdata->Count >=fmtdata->Nseq)
    {					/* all done */
	ajFilebuffClear(seqin->Filebuff, 0);
	ajTableMapDel(fmttable, seqMsfTabDel, NULL);
	ajTableFree(&fmttable);
	AJFREE(fmtdata->Names);
	AJFREE(fmtdata);
	seqin->Data = NULL;
	return ajFalse;
    }
    i = fmtdata->Count;
    ajDebug("returning [%d] '%S'\n", i, fmtdata->Names[i]);
    fmtitem = ajTableFetch(fmttable, fmtdata->Names[i]);
    ajStrAssignS(&thys->Name, fmtdata->Names[i]);
    ajStrDel(&fmtdata->Names[i]);

    thys->Weight = fmtitem->Weight;
    ajStrAssignS(&thys->Seq, fmtitem->Seq);
    ajStrDel(&fmtitem->Seq);

    fmtdata->Count++;

    return ajTrue;
}




/* @funcstatic seqHennig86Readseq *********************************************
**
** Reads sequence name from first token on the input line, and appends
** the sequence data to that sequence in the fmttable structure.
**
** @param [r] rdline [const AjPStr] Line from input file.
** @param [r] msftable [const AjPTable] MSF format sequence table.
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqHennig86Readseq(const AjPStr rdline, const AjPTable msftable)
{
    SeqPMsfItem msfitem;
    AjPStr token  = NULL;
    AjPStr seqstr = NULL;

    if(!seqRegHennigSeq)
	seqRegHennigSeq = ajRegCompC("^[^ \t\n\r]+"); /* must be line start */

    if(!ajRegExec(seqRegHennigSeq, rdline))
	return ajFalse;

    ajRegSubI(seqRegHennigSeq, 0, &token);
    msfitem = ajTableFetch(msftable, token);
    ajStrDel(&token);
    if(!msfitem)
	return ajFalse;

    ajRegPost(seqRegHennigSeq, &seqstr);
    seqAppend(&msfitem->Seq, seqstr);

    ajStrDel(&seqstr);

    return ajTrue;
}




/* @funcstatic seqReadTreecon *************************************************
**
** Tries to read input in Treecon format.
**
** Treecon is a windows program for tree drawing.
**
** Van de Peer, Y., De Wachter, R. (1994)
** TREECON for Windows: a software package for the construction and
** drawing of evolutionary trees for the Microsoft Windows environment.
** Comput. Applic. Biosci. 10, 569-570.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadTreecon(AjPSeq thys, AjPSeqin seqin)
{
    AjPStr tmpstr = NULL;
    ajuint bufflines = 0;
    AjBool ok       = ajFalse;
    ajint len       = 0;
    ajint ilen      = 0;
    ajuint iseq;
    ajuint i;
    AjPFilebuff buff;

    AjPTable phytable        = NULL;
    SeqPMsfItem phyitem      = NULL;
    AjPList phylist          = NULL;
    SeqPMsfData phydata      = NULL;

    buff = seqin->Filebuff;

    if(!seqRegTreeconTop)
	seqRegTreeconTop = ajRegCompC("^ *([0-9]+)");

    if(!seqin->Data)			/* first time - read the data */
    {
	iseq = 0;
	seqin->multidone = ajFalse;
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	if(!ok)
	    return ajFalse;
	bufflines++;

	if(!ajRegExec(seqRegTreeconTop, seqReadLine))
	{				/* first line test */
	    ajFilebuffReset(buff);
	    return ajFalse;
	}

	ajRegSubI(seqRegTreeconTop, 1, &tmpstr);
	ajStrToInt(tmpstr, &len);
	ajStrDel(&tmpstr);
	ajDebug("first line OK: '%S' len: %d\n",
		seqReadLine, len);

	seqin->Data = AJNEW0(phydata);
	phydata->Table = phytable = ajTablestrNew();
	phylist = ajListstrNew();
	seqin->Filecount = 0;

	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	bufflines++;
	ilen = UINT_MAX;
	while (ok)
	{
	   if (ilen < 0)
	   {
	       ajStrRemoveWhiteExcess(&seqReadLine);
	       if (!ajStrGetLen(seqReadLine))	/* empty line after sequence */
	       {
		   ok = ajBuffreadLineStore(buff, &seqReadLine,
					   seqin->Text, &thys->TextPtr);
		   continue;
	       }
	       AJNEW0(phyitem);
	       phyitem->Weight = 1.0;
	       seqSetName(&phyitem->Name, seqReadLine);
	       ajTablePut(phytable, ajStrNewS(phyitem->Name), phyitem);
	       ajListstrPushAppend(phylist, ajStrNewS(phyitem->Name));
	       iseq++;
	       ilen = 0;
	   }
	   else
	   {
	       ajStrRemoveWhite(&seqReadLine);
	       ilen += ajStrGetLen(seqReadLine);
	       seqAppend(&phyitem->Seq, seqReadLine);
	       
	       if (ilen > len)
	       {
		   ajDebug("Treecon format: '%S' too long, read %d/%d\n",
		    phyitem->Name, ilen, len);
		ajFilebuffReset(buff);
		seqMsfDataDel((SeqPMsfData*)&seqin->Data);
		return ajFalse;
	       }
	       if (ilen == len)
	       {
		   ilen = -1;
	       }
	   }

	   ok = ajBuffreadLineStore(buff, &seqReadLine,
				   seqin->Text, &thys->TextPtr);
	}
	if (ilen >= 0)
	{
	    ajDebug("Treecon format: unfinished sequence '%S' read %d/%d\n",
		    phyitem->Name, ilen, len);
	    seqMsfDataDel((SeqPMsfData*)&seqin->Data);
	    return ajFalse;
	}

	phydata->Names = AJCALLOC(iseq, sizeof(*phydata->Names));
	for(i=0; i < iseq; i++)
	{
	    ajListstrPop(phylist, &phydata->Names[i]);
	    ajDebug("list [%d] '%S'\n", i, phydata->Names[i]);
	}
	ajListstrFreeData(&phylist);
	phydata->Nseq = iseq;
	phydata->Count = 0;
	phydata->Bufflines = bufflines;
	ajDebug("Treecon format read %d lines\n", bufflines);


    }

    phydata = seqin->Data;
    phytable = phydata->Table;

    i = phydata->Count;
    ajDebug("returning [%d] '%S'\n", i, phydata->Names[i]);
    phyitem = ajTableFetch(phytable, phydata->Names[i]);
    ajStrAssignS(&thys->Name, phydata->Names[i]);

    thys->Weight = phyitem->Weight;
    ajStrAssignS(&thys->Seq, phyitem->Seq);

    phydata->Count++;
    if(phydata->Count >=phydata->Nseq)
    {
	seqin->multidone = ajTrue;
	ajDebug("seqReadTreecon multidone\n");
	ajFilebuffClear(seqin->Filebuff, 0);
	seqMsfDataDel((SeqPMsfData*)&seqin->Data);
    }

    return ajTrue;
}




/* @funcstatic seqReadJackknifer **********************************************
**
** Tries to read input in Jackknifer format.
**
** The Jackknifer program by Farris is a parsimony program that also
** implements the jackknife method to test the reliability of branches. 
** The format is similar to the MEGA format.
**
** On the first line a title/description is placed in between single quotes.
** The alignment can be written in sequential or interleaved format,
** but the sequence names have to be placed between brackets.
** Also no blanks are allowed in the names.
** They should be replaced by underscores ( _ ).
** The file is ended by a semicolon.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadJackknifer(AjPSeq thys, AjPSeqin seqin)
{
    AjPStr tmpstr = NULL;
    AjPStr tmpname = NULL;
    ajuint bufflines = 0;
    AjBool ok       = ajFalse;
    ajuint iseq;
    ajuint i;
    AjPFilebuff buff;

    AjPTable phytable        = NULL;
    SeqPMsfItem phyitem      = NULL;
    AjPList phylist          = NULL;
    SeqPMsfData phydata      = NULL;

    buff = seqin->Filebuff;

    if(!seqRegJackTop)
	seqRegJackTop = ajRegCompC("^'(.*)'\\s*$");

    if(!seqRegJackSeq)
	seqRegJackSeq = ajRegCompC("^[(]([^)]+)(.*)$");

    if(!seqin->Data)			/* first time - read the data */
    {
	iseq = 0;
	seqin->multidone = ajFalse;
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	if(!ok)
	    return ajFalse;
	bufflines++;

	if(!ajRegExec(seqRegJackTop, seqReadLine))
	{				/* first line test */
	    ajFilebuffReset(buff);
	    return ajFalse;
	}
	ajDebug("JackKnifer format: First line ok '%S'\n", seqReadLine);

	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);

	seqin->Data = AJNEW0(phydata);
	phydata->Table = phytable = ajTablestrNew();
	phylist = ajListstrNew();
	seqin->Filecount = 0;

	while (ok)
	{
	    if (!ajStrGetLen(seqReadLine))	/* empty line after sequence */
	    {
		ok = ajBuffreadLineStore(buff, &seqReadLine,
					seqin->Text, &thys->TextPtr);
		continue;
	    }
	    if (ajStrPrefixC(seqReadLine, ";"))
		break;			/* done */
	    if (ajStrPrefixC(seqReadLine, "("))
	    {
		if (!ajRegExec(seqRegJackSeq, seqReadLine))
		{
		    ajDebug("JackKnifer format: bad (id) line\n");
		    seqMsfDataDel((SeqPMsfData*)&seqin->Data);
		    return ajFalse;
		}

		ajRegSubI(seqRegJackSeq, 1, &tmpstr);
		seqSetName(&tmpname, tmpstr);
		phyitem = ajTableFetch(phytable, tmpname);
		if (!phyitem)
		{
		    ajDebug("JackKnifer format: new (id) '%S'\n", tmpname);
		    AJNEW0(phyitem);
		    phyitem->Weight = 1.0;
		    ajStrAssignS(&phyitem->Name,tmpname);
		    ajTablePut(phytable, ajStrNewS(phyitem->Name), phyitem);
		    ajListstrPushAppend(phylist, ajStrNewS(phyitem->Name));
		    iseq++;
		}
		else
		{
		    ajDebug("JackKnifer format: More for (id) '%S'\n",
			    tmpname);

		}
		ajRegSubI(seqRegJackSeq, 2, &tmpstr);
		ajStrAssignS(&seqReadLine, tmpstr);
	    }
	    seqAppend(&phyitem->Seq, seqReadLine);

	    ok = ajBuffreadLineStore(buff, &seqReadLine,
				   seqin->Text, &thys->TextPtr);
	}

	phydata->Names = AJCALLOC(iseq, sizeof(*phydata->Names));
	for(i=0; i < iseq; i++)
	{
	    ajListstrPop(phylist, &phydata->Names[i]);
	    ajDebug("list [%d] '%S'\n", i, phydata->Names[i]);
	}
	ajListstrFreeData(&phylist);
	phydata->Nseq = iseq;
	phydata->Count = 0;
	phydata->Bufflines = bufflines;
	ajDebug("JackKnifer format read %d lines\n", bufflines);
    }
    ajStrDel(&tmpstr);
    ajStrDel(&tmpname);

    phydata = seqin->Data;
    phytable = phydata->Table;

    i = phydata->Count;
    ajDebug("returning [%d] '%S'\n", i, phydata->Names[i]);
    phyitem = ajTableFetch(phytable, phydata->Names[i]);
    ajStrAssignS(&thys->Name, phydata->Names[i]);
    ajStrDel(&phydata->Names[i]);

    thys->Weight = phyitem->Weight;
    ajStrAssignS(&thys->Seq, phyitem->Seq);
    ajStrDel(&phyitem->Seq);

    phydata->Count++;
    if(phydata->Count >=phydata->Nseq)
    {
	seqin->multidone = ajTrue;
	ajDebug("seqReadJackKnifer multidone\n");
	ajFilebuffClear(seqin->Filebuff, 0);
	seqMsfDataDel((SeqPMsfData*)&seqin->Data);
    }

    return ajTrue;
}




/* @funcstatic seqReadNexus ***************************************************
**
** Tries to read input in Nexus format.
**
** Nexus files contain many things.
** All Nexus files begin with a #NEXUS line
** Data is in begin ... end blocks
** Sequence data is in a "begin character" block
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadNexus(AjPSeq thys, AjPSeqin seqin)
{
    ajuint bufflines = 0;
    AjBool ok       = ajFalse;
    ajuint i;
    ajuint j;
    AjPFilebuff buff;
    AjPStr* seqs = NULL;
    AjPNexus nexus = NULL;

    SeqPMsfData phydata      = NULL;

    buff = seqin->Filebuff;

    if(!seqin->Data)			/* first time - read the data */
    {
	seqin->multidone = ajFalse;

	ajFilebuffSetBuffered(buff);

	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	ajDebug("Nexus format: Testing first line '%S'\n", seqReadLine);
	if(!ok)
	    return ajFalse;
	bufflines++;

	if(!ajStrPrefixCaseC(seqReadLine, "#NEXUS"))
	{				/* first line test */
	    ajFilebuffReset(buff);
	    return ajFalse;
	}
	ajDebug("Nexus format: First line ok '%S'\n", seqReadLine);

	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	while(ok && !ajStrPrefixCaseC(seqReadLine, "#NEXUS"))
	{
	    ok = ajBuffreadLineStore(buff, &seqReadLine,
				    seqin->Text, &thys->TextPtr);
	}

	ajFilebuffReset(buff);

	AJNEW0(phydata);
	phydata->Nexus = ajNexusParse(buff);
	if (!phydata->Nexus)
	{
	    ajFilebuffReset(buff);
	    ajDebug("Failed to parse in nexus format\n");
	    return ajFalse;
	}
	phydata->Count = 0;
	phydata->Nseq = ajNexusGetNtaxa(phydata->Nexus);
	/* GetTaxa may fail if names are only defined in the sequences */
	seqs = ajNexusGetTaxa(phydata->Nexus);
	phydata->Names = AJCALLOC(phydata->Nseq, sizeof(*phydata->Names));
	for(j=0;j<phydata->Nseq;j++)
	    ajStrAssignS(&phydata->Names[j], seqs[j]);
	seqin->Data = phydata;
	ajDebug("Nexus parsed %d sequences\n", phydata->Nseq);
    }

    phydata = seqin->Data;
    nexus = phydata->Nexus;

    i = phydata->Count;

    seqs = ajNexusGetSequences(nexus);
    if (!seqs)
    {
	seqMsfDataDel((SeqPMsfData*)&seqin->Data);
	return ajFalse;
    }

    if (!phydata->Names)		/* finally set from the sequences */
    {
	seqs = ajNexusGetTaxa(phydata->Nexus);
	phydata->Names = AJCALLOC(phydata->Nseq, sizeof(*phydata->Names));
	for(j=0;j<phydata->Nseq;j++)
	    ajStrAssignS(&phydata->Names[j], seqs[j]);
    }
    ajDebug("returning [%d] '%S'\n", i, phydata->Names[i]);

    ajStrAssignS(&thys->Name, phydata->Names[i]);

    thys->Weight = 1.0;
    ajStrAssignS(&thys->Seq, seqs[i]);

    phydata->Count++;
    if(phydata->Count >= phydata->Nseq)
    {
	seqin->multidone = ajTrue;
	ajDebug("seqReadNexus multidone\n");
	ajFilebuffClear(seqin->Filebuff, 0);
	seqMsfDataDel((SeqPMsfData*)&seqin->Data);
    }

    return ajTrue;
}


/* @funcstatic seqReadMega ****************************************************
**
** Tries to read input in Mega non-interleaved format.
**
** The Molecular Evolutionary Genetic Analysis program by
** Kumar, Tamura & Nei is a tree construction program
** based on distance- and parsimony methods.
**
** http://evolgen.biol.metro-u.ac.jp/MEGA/manual/DataFormat.html
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadMega(AjPSeq thys, AjPSeqin seqin)
{
    AjPStr tmpstr = NULL;
    AjPStr tmpname = NULL;
    AjPStr prestr = NULL;
    AjPStr poststr = NULL;
    ajuint bufflines = 0;
    AjBool ok       = ajFalse;
    ajuint iseq;
    ajuint i;
    AjPFilebuff buff;

    AjPTable phytable        = NULL;
    SeqPMsfItem phyitem      = NULL;
    AjPList phylist          = NULL;
    SeqPMsfData phydata      = NULL;

    buff = seqin->Filebuff;

    if(!seqRegMegaFeat)
	seqRegMegaFeat = ajRegCompC("^(.*)\"[^\"]*\"(.*)$");

    if(!seqRegMegaSeq)
	seqRegMegaSeq = ajRegCompC("^#([^ \t\n\r]+)(.*)$");

    if(!seqin->Data)			/* first time - read the data */
    {
	iseq = 0;
	seqin->multidone = ajFalse;
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	ajDebug("Mega format: Testing first line '%S'\n", seqReadLine);
	if(!ok)
	    return ajFalse;
	bufflines++;

	if(!ajStrMatchCaseC(seqReadLine, "#MEGA\n"))
	{				/* first line test */
	    ajFilebuffReset(buff);
	    return ajFalse;
	}
	ajDebug("Mega format: First line ok '%S'\n", seqReadLine);

	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	if(!ok)
	    return ajFalse;
	bufflines++;

	if(!ajStrPrefixCaseC(seqReadLine, "TITLE"))
	{				/* first line test */
	    ajFilebuffReset(buff);
	    return ajFalse;
	}
	ajDebug("Mega format: Second line ok '%S'\n", seqReadLine);

	while(ok && !ajStrPrefixC(seqReadLine, "#"))
	{				/* skip comments in header */
	    ok = ajBuffreadLineStore(buff, &seqReadLine,
				    seqin->Text, &thys->TextPtr);
	}


	/*
        ** read through looking for #id
	** Some day we could stop at #mega and read multiple files
	*/

	
	seqin->Data = AJNEW0(phydata);
	phydata->Table = phytable = ajTablestrNew();
	phylist = ajListstrNew();
	seqin->Filecount = 0;

	while (ok)
	{
	    if (!ajStrGetLen(seqReadLine))	/* empty line after a sequence */
	    {
		ok = ajBuffreadLineStore(buff, &seqReadLine,
					seqin->Text, &thys->TextPtr);
		continue;
	    }
	    if (ajStrPrefixC(seqReadLine, "#"))
	    {
		if (!ajRegExec(seqRegMegaSeq, seqReadLine))
		{
		    ajDebug("Mega format: bad #id line\n");
		    seqMsfDataDel((SeqPMsfData*)&seqin->Data);
		    return ajFalse;
		}

		ajRegSubI(seqRegMegaSeq, 1, &tmpstr);
		seqSetName(&tmpname, tmpstr);
		phyitem = ajTableFetch(phytable, tmpname);
		if (!phyitem)
		{
		    ajDebug("Mega format: new #id '%S'\n", tmpname);
		    AJNEW0(phyitem);
		    phyitem->Weight = 1.0;
		    ajStrAssignS(&phyitem->Name,tmpname);
		    ajTablePut(phytable, ajStrNewS(phyitem->Name), phyitem);
		    ajListstrPushAppend(phylist, ajStrNewS(phyitem->Name));
		    iseq++;
		}
		else
		{
		    ajDebug("Mega format: More for #id '%S'\n", tmpname);

		}
		ajRegSubI(seqRegMegaSeq, 2, &tmpstr);
		ajStrAssignS(&seqReadLine, tmpstr);
	    }
	    while (ajRegExec(seqRegMegaFeat, seqReadLine))
	    {
		ajDebug("Quotes found: '%S'\n", seqReadLine);
		ajRegSubI(seqRegMegaFeat, 1, &prestr);
		ajRegSubI(seqRegMegaFeat, 2, &poststr);
		ajStrAssignS(&seqReadLine, prestr);
		ajStrAppendS(&seqReadLine, poststr);
		ajDebug("Quotes removed: '%S'\n", seqReadLine);
	    }
	    seqAppend(&phyitem->Seq, seqReadLine);

	    ok = ajBuffreadLineStore(buff, &seqReadLine,
				   seqin->Text, &thys->TextPtr);
	}

	phydata->Names = AJCALLOC(iseq, sizeof(*phydata->Names));
	for(i=0; i < iseq; i++)
	{
	    ajListstrPop(phylist, &phydata->Names[i]);
	    ajDebug("list [%d] '%S'\n", i, phydata->Names[i]);
	}
	ajListstrFreeData(&phylist);
	phydata->Nseq = iseq;
	phydata->Count = 0;
	phydata->Bufflines = bufflines;
	ajDebug("Mega format read %d lines\n", bufflines);
    }
    ajStrDel(&tmpstr);
    ajStrDel(&tmpname);
    ajStrDel(&prestr);
    ajStrDel(&poststr);

    phydata = seqin->Data;
    phytable = phydata->Table;

    i = phydata->Count;
    ajDebug("returning [%d] '%S'\n", i, phydata->Names[i]);
    phyitem = ajTableFetch(phytable, phydata->Names[i]);
    ajStrAssignS(&thys->Name, phydata->Names[i]);
    ajStrDel(&phydata->Names[i]);

    thys->Weight = phyitem->Weight;
    ajStrAssignS(&thys->Seq, phyitem->Seq);
    ajStrDel(&phyitem->Seq);

    phydata->Count++;
    if(phydata->Count >=phydata->Nseq)
    {
	seqin->multidone = ajTrue;
	ajDebug("seqReadMega multidone\n");
	ajFilebuffClear(seqin->Filebuff, 0);
	seqMsfDataDel((SeqPMsfData*)&seqin->Data);
    }

    return ajTrue;
}




/* @funcstatic seqReadCodata **************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using CODATA format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadCodata(AjPSeq thys, AjPSeqin seqin)
{
    AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    ajuint bufflines         = 0;
    AjPFilebuff buff;
    AjBool ok = ajTrue;


    buff = seqin->Filebuff;

    if(!ajBuffreadLineStore(buff, &seqReadLine,
			   seqin->Text, &thys->TextPtr))
	return ajFalse;

    bufflines++;

    ajDebug("first line '%S'\n", seqReadLine);

    if(!ajStrPrefixC(seqReadLine, "ENTRY "))
    {
	ajFilebuffReset(buff);
	return ajFalse;
    }
    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
    ajStrTokenNextParse(&handle, &token);	/* 'ENTRY' */
    ajStrTokenNextParse(&handle, &token);	/* entry name */

    seqSetName(&thys->Name, token);

    ok = ajBuffreadLineStore(buff, &seqReadLine,
			    seqin->Text, &thys->TextPtr);

    while(ok && !ajStrPrefixC(seqReadLine, "SEQUENCE"))
    {
	bufflines++;
	if(ajStrPrefixC(seqReadLine, "ACCESSION "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " ;\n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'AC' */
	    ajStrTokenNextParse(&handle, &token); /* accnum */
	    seqAccSave(thys, token);
	}

	if(ajStrPrefixC(seqReadLine, "TITLE "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'DE' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* desc */
	    while(ok && ajStrPrefixC(seqReadLine, " "))
	    {
		bufflines++;
		ajStrTokenAssignC(&handle, seqReadLine, " ");
		ajStrTokenNextParseC(&handle, "\n\r", &token);
		ajStrAppendC(&thys->Desc, " ");
		ajStrAppendS(&thys->Desc, token);
		ok = ajBuffreadLineStore(buff, &seqReadLine,
					seqin->Text, &thys->TextPtr);
	    }
	}
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
    }

    ok = ajBuffreadLineStore(buff, &seqReadLine,
			    seqin->Text, &thys->TextPtr);
    while(ok && !ajStrPrefixC(seqReadLine, "///"))
    {
	seqAppend(&thys->Seq, seqReadLine);
	bufflines++;
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
    }
    ajFilebuffClear(buff, 0);

    ajStrTokenDel(&handle);
    ajStrDel(&token);

    return ajTrue;
}




/* @funcstatic seqReadAcedb ***************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using ACEDB format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadAcedb(AjPSeq thys, AjPSeqin seqin)
{
    AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    ajuint bufflines         = 0;
    AjPFilebuff buff;
    AjBool ok = ajTrue;

    ajDebug("seqReadAcedb\n");

    buff = seqin->Filebuff;

    do
    {
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	bufflines++;
    } while(ok &&
	    (ajStrPrefixC(seqReadLine, "//") || ajStrPrefixC(seqReadLine, "\n")));

    if(!ok)
    {
	ajFilebuffReset(buff);
	return ajFalse;
    }
    ajDebug("first line:\n'%S'\n", seqReadLine);


    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
    ajStrTokenNextParseC(&handle, " \t", &token); /* 'DNA' or 'Peptide'*/
    ajDebug("Token 1 '%S'\n", token);

    if(ajStrMatchCaseC(token, "Peptide"))
    {
	ajDebug("Protein\n");
	ajSeqSetProt(thys);
    }
    else if(ajStrMatchCaseC(token, "DNA"))
    {
	ajDebug("DNA\n");
	ajSeqSetNuc(thys);
    }
    else
    {
	ajDebug("unknown - failed\n");
	ajFilebuffReset(buff);
	ajStrTokenDel(&handle);
	ajStrDel(&token);
	return ajFalse;
    }

    ajStrTokenNextParseC(&handle, " \t\"", &token); /* : */
    if(!ajStrMatchC(token, ":"))
    {
	ajFilebuffReset(buff);
	ajStrTokenDel(&handle);
	ajStrDel(&token);
	return ajFalse;
    }

    ajStrTokenNextParseC(&handle, "\"", &token);	/* name */
    if(!ajStrGetLen(token))
    {
	ajFilebuffReset(buff);
	ajStrTokenDel(&handle);
	ajStrDel(&token);
	return ajFalse;
    }

    seqSetName(&thys->Name, token);

    /* OK, we have the name. Now look for the sequence */

    ok = ajBuffreadLineStore(buff, &seqReadLine,
			    seqin->Text, &thys->TextPtr);
    while(ok && !ajStrPrefixC(seqReadLine,"\n"))
    {
	seqAppend(&thys->Seq, seqReadLine);
	bufflines++;
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
    }

    ajFilebuffClear(buff, 0);

    ajStrTokenDel(&handle);
    ajStrDel(&token);

    return ajTrue;
}


/* @funcstatic seqReadFitch *************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using fitch format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadFitch(AjPSeq thys, AjPSeqin seqin)
{
    AjPStr token     = NULL;
    AjPFilebuff buff;
    AjBool ok = ajTrue;
    ajuint ilen = 0;

   if (!seqRegFitchHead)
	seqRegFitchHead = ajRegCompC("^(\\S+),\\s+(\\d+)\\s+bases\n");

    buff = seqin->Filebuff;

    ok = ajBuffreadLineStore(buff, &seqReadLine,
			    seqin->Text, &thys->TextPtr);
    if (!ajRegExec(seqRegFitchHead, seqReadLine))
    {
	ajFilebuffReset(buff);
	return ajFalse;
    }

    ajRegSubI(seqRegFitchHead, 1, &token);
    seqSetName(&thys->Name, token);

    ajRegSubI(seqRegFitchHead, 2, &token);
    ajStrToUint(token, &ilen);

    ok = ajBuffreadLineStore(buff, &seqReadLine,
			    seqin->Text, &thys->TextPtr);
    while (ok && (ajStrGetLen(thys->Seq) < ilen))
    {
	seqAppend(&thys->Seq, seqReadLine);
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
    }

    ajStrDel(&token);
    ajFilebuffClear(buff, 0);
    return ajTrue;
}

/* @funcstatic seqReadMase ****************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using mase format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadMase(AjPSeq thys, AjPSeqin seqin)
{
    AjPStr token     = NULL;
    AjPStr des     = NULL;
    AjPFilebuff buff;
    AjBool ok = ajTrue;

    if (!seqRegMaseHead)
	seqRegMaseHead = ajRegCompC("^(;+)");

    buff = seqin->Filebuff;

    ok = ajBuffreadLineStore(buff, &seqReadLine,
			    seqin->Text, &thys->TextPtr);
    if (!ajRegExec(seqRegMaseHead, seqReadLine))
    {
	ajFilebuffReset(buff);
	return ajFalse;
    }

    while (ok && ajRegExec(seqRegMaseHead, seqReadLine))
    {
	if (ajRegLenI(seqRegMaseHead, 1) == 1)
	{
	    ajRegPost(seqRegMaseHead, &token);
	    if (des)
		ajStrAppendK(&des, ' ');
	    ajStrAppendS(&des, token);
	}
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
    }

    ajStrRemoveWhiteExcess(&seqReadLine);
    seqSetName(&thys->Name, seqReadLine);
    ajStrRemoveWhiteExcess(&des);
    ajSeqAssignDescS(thys, des);

    ok = ajBuffreadLineStore(buff, &seqReadLine,
			    seqin->Text, &thys->TextPtr);
    while (ok && !ajRegExec(seqRegMaseHead, seqReadLine))
    {
	seqAppend(&thys->Seq, seqReadLine);
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
    }

    ajStrDel(&token);
    ajStrDel(&des);
    if (ok)
	ajFilebuffClearStore(buff, 1,
			     seqReadLine, seqin->Text, &thys->TextPtr);
    else
	ajFilebuffClear(buff, 0);

    return ajTrue;
}

/* @funcstatic seqReadStrider *************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using DNA strider format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadStrider(AjPSeq thys, AjPSeqin seqin)
{
    AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    ajuint bufflines         = 0;
    AjPFilebuff buff;
    AjBool ok = ajTrue;

    buff = seqin->Filebuff;

    do
    {
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	if(ajStrPrefixC(seqReadLine, "; DNA sequence"))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " ;\t,\n");
	    ajStrTokenNextParse(&handle, &token); /* 'DNA' */
	    ajStrTokenNextParse(&handle, &token); /* sequence */
	    ajStrTokenNextParse(&handle, &token); /* entry name */
	}
	bufflines++;
    } while(ok && ajStrPrefixC(seqReadLine, ";"));

    ajStrTokenDel(&handle);

    if(!ok || !ajStrGetLen(token))
    {
	ajFilebuffReset(buff);
	ajStrDel(&token);
	return ajFalse;
    }

    seqSetName(&thys->Name, token);

    /* OK, we have the name. Now look for the sequence */

    while(ok && !ajStrPrefixC(seqReadLine, "//"))
    {
	seqAppend(&thys->Seq, seqReadLine);
	bufflines++;
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
    }

    ajFilebuffClear(buff, 0);
    ajStrDel(&token);

    return ajTrue;
}




/* @funcstatic seqReadMsf *****************************************************
**
** Tries to read input in MSF format. If successful, can repeat for the
** next call to return the second, third, ... sequence from the same file.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadMsf(AjPSeq thys, AjPSeqin seqin)
{
    ajuint bufflines      = 0;
    ajuint len;
    AjBool ok  = ajFalse;
    ajuint iseq = 0;

    AjPFilebuff buff;
    AjPTable msftable   = NULL;
    SeqPMsfItem msfitem = NULL;
    AjPList msflist     = NULL;
    SeqPMsfData msfdata = NULL;

    ajuint i;

    ajDebug("seqReadMsf seqin->Data %x\n", seqin->Data);

    buff = seqin->Filebuff;

    if(!seqin->Data)
    {
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	if(!ok)
	    return ajFalse;
	bufflines++;

	if(ajStrPrefixC(seqReadLine, "!!"))
	{
	    if(ajStrPrefixC(seqReadLine, "!!AA_MULTIPLE_ALIGNMENT"))
		ajSeqSetProt(thys);

	    if(ajStrPrefixC(seqReadLine, "!!NA_MULTIPLE_ALIGNMENT"))
		ajSeqSetNuc(thys);
	}

	if(!seqGcgMsfDots(thys, seqin, &seqReadLine, seqMaxGcglines, &len))
	{
	    ajDebug("seqGcgMsfDots failed\n");
	    ajFilebuffReset(buff);
	    return ajFalse;
	}

	seqin->Data = AJNEW0(msfdata);
	msfdata->Table = msftable = ajTablestrNew();
	msflist = ajListstrNew();
	seqin->Filecount = 0;
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	bufflines++;
	while(ok && !ajStrPrefixC(seqReadLine, "//"))
	{
	    ok = ajBuffreadLineStore(buff, &seqReadLine,
				    seqin->Text, &thys->TextPtr);
	    bufflines++;

	    if(seqGcgMsfHeader(seqReadLine, &msfitem))
	    {
		ajTablePut(msftable, ajStrNewS(msfitem->Name), msfitem);
		ajListstrPushAppend(msflist, ajStrNewS(msfitem->Name));
		iseq++;
	    }
	}

	ajDebug("Header has %d sequences\n", iseq);
	ajListstrTrace(msflist);
	ajTableTrace(msftable);
	ajTableMap(msftable, seqMsfTabList, NULL);

	msfdata->Names = AJCALLOC(iseq, sizeof(*msfdata->Names));
	for(i=0; i < iseq; i++)
	{
	    ajListstrPop(msflist, &msfdata->Names[i]);
	    ajDebug("list [%d] '%S'\n", i, msfdata->Names[i]);
	}
	ajListstrFreeData(&msflist);
	while(ajBuffreadLineStore(buff, &seqReadLine,
				 seqin->Text, &thys->TextPtr))
	{
	    bufflines++;
	    seqGcgMsfReadseq(seqReadLine, msftable);
	}

	ajTableMap(msftable, seqMsfTabList, NULL);
	msfdata->Nseq = iseq;
	msfdata->Count = 0;
	msfdata->Bufflines = bufflines;
	ajDebug("MSF format read %d lines\n", bufflines);
    }

    msfdata = seqin->Data;
    msftable = msfdata->Table;
    if(msfdata->Count >= msfdata->Nseq)
    {
	ajFilebuffClear(seqin->Filebuff, 0);
	seqMsfDataDel((SeqPMsfData*)&seqin->Data);
	return ajFalse;
    }
    i = msfdata->Count;
    ajDebug("returning [%d] '%S'\n", i, msfdata->Names[i]);
    msfitem = ajTableFetch(msftable, msfdata->Names[i]);
    ajStrAssignS(&thys->Name, msfdata->Names[i]);

    thys->Weight = msfitem->Weight;
    ajStrAssignS(&thys->Seq, msfitem->Seq);

    msfdata->Count++;

    return ajTrue;
}




/* @funcstatic seqGcgMsfReadseq ***********************************************
**
** Reads sequence name from first token on the input line, and appends
** the sequence data to that sequence in the msftable structure.
**
** @param [r] rdline [const AjPStr] Line from input file.
** @param [r] msftable [const AjPTable] MSF format sequence table.
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqGcgMsfReadseq(const AjPStr rdline, const AjPTable msftable)
{
    SeqPMsfItem msfitem;
    AjPStr token     = NULL;
    AjPStr seqstr    = NULL;
    AjBool status;

    status = ajStrExtractWord(rdline, &seqstr, &token);
    if(!status)
    {
	ajStrDel(&token);
	ajStrDel(&seqstr);
	return ajFalse;
    }

    ajDebug("seqGcgMsfReadseq '%S' '%S'\n", token, seqstr);

    msfitem = ajTableFetch(msftable, token);
    if(!msfitem)
    {
	ajStrDel(&token);
	ajStrDel(&seqstr);
	return ajFalse;
    }

    seqAppend(&msfitem->Seq, seqstr);

    ajStrDel(&token);
    ajStrDel(&seqstr);

    return ajTrue;
}




/* @funcstatic seqMsfDataDel **************************************************
**
** Destructor for SeqPMsfData objects
**
** @param [d] pthys [SeqPMsfData*] MSF data object
** @return [void]
** @@
******************************************************************************/

static void seqMsfDataDel(SeqPMsfData* pthys)
{
    SeqPMsfData thys;
    ajuint i;

    if(!pthys) return;
    if(!*pthys) return;

    thys = *pthys;

    ajDebug("seqMsfDataDel Nseq:%u Count:%u Table:%u Nexus:%u\n",
	    thys->Nseq, thys->Count, ajTableGetLength(thys->Table),
	    ajNexusGetNtaxa(thys->Nexus));

    for(i=0; i < thys->Nseq; i++)
	ajStrDel(&thys->Names[i]);
    AJFREE(thys->Names);

    ajNexusDel(&thys->Nexus);
    ajTableMapDel(thys->Table, seqMsfTabDel, NULL);
    ajTableFree(&thys->Table);

    AJFREE(*pthys);

    return;
}




/* @funcstatic seqMsfItemDel **************************************************
**
** Destructor for SeqPMsfItem objects
**
** @param [d] pthys [SeqPMsfItem*] MSF item object
** @return [void]
** @@
******************************************************************************/

static void seqMsfItemDel(SeqPMsfItem* pthys)
{
    SeqPMsfItem thys;

    if(!pthys) return;
    if(!*pthys) return;

    thys = *pthys;

    ajStrDel(&thys->Name);
    ajStrDel(&thys->Seq);

    AJFREE(*pthys);

    return;
}




/* @funcstatic seqMsfTabList **************************************************
**
** Writes a debug report of the contents of an MSF table.
**
** @param [r] key [const void*] Standard argument, key from current table item
**                              which is a string for MSF internal tables.
** @param [r] value [void**] Standard argument, data from current table item,
**                           converted to an MSF internal table item.
** @param [r] cl [void*] Standard argument, usually NULL.
** @return [void]
** @@
******************************************************************************/

static void seqMsfTabList(const void* key, void** value, void* cl)
{
    SeqPMsfItem msfitem;

    (void) cl;

    msfitem = (SeqPMsfItem) *value;

    ajDebug("key '%S' Name '%S' Seqlen %d\n",
	    key, msfitem->Name, ajStrGetLen(msfitem->Seq));

    return;
}




/* @funcstatic seqMsfDataTrace ************************************************
**
** Debug trace report for SeqPMsfData objects
**
** @param [r] thys [const SeqPMsfData] MSF data object
** @return [void]
** @@
******************************************************************************/

static void seqMsfDataTrace(const SeqPMsfData thys)
{
    ajuint i;

    if(!thys)
    {
	ajDebug("seqMsfDataTrace <null>\n");
	return;
    }

    ajDebug("seqMsfDataTrace Nseq:%u Count:%u Table:%u Nexus:%u\n",
	    thys->Nseq, thys->Count, ajTableGetLength(thys->Table),
	    ajNexusGetNtaxa(thys->Nexus));

    for(i=0; i < thys->Nseq; i++)
    {
	if(i < thys->Count)
	    ajDebug("* [%u] '%S'\n", i, thys->Names[i]);
	else
	    ajDebug("  [%u] '%S'\n", i, thys->Names[i]);
    }

    ajTableMap(thys->Table, seqMsfTabList, NULL);

    return;
}




/* @funcstatic seqMsfTabDel ***************************************************
**
** Deletes entries from the MSF internal table. Called for each entry in turn.
**
** @param [d] key [void**] Standard argument, table key.
** @param [d] value [void**] Standard argument, table data item.
** @param [r] cl [void*] Standard argument, usually NULL
** @return [void]
** @@
******************************************************************************/

static void seqMsfTabDel(void** key, void** value, void* cl)
{
    SeqPMsfItem msfitem;
    AjPStr keystr;

    (void) cl;

    keystr = (AjPStr) *key;
    msfitem = (SeqPMsfItem) *value;

    ajDebug("seqMsfTabDel key: '%S' item: '%S'\n", keystr, msfitem->Name);

    ajStrDel(&keystr);

    seqMsfItemDel(&msfitem);

    *key = NULL;
    *value = NULL;

    return;
}




/* @funcstatic seqReadSwiss ***************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using SWISS format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadSwiss(AjPSeq thys, AjPSeqin seqin)
{
    AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    ajuint bufflines         = 0;
    AjBool ok;
    AjPFilebuff buff;
    AjPStr tmpstr = NULL;
    AjBool dofeat        = ajFalse;
    AjPStr liststr;			/* for lists, do not delete */


    buff = seqin->Filebuff;

    if(!seqFtFmtSwiss)
	ajStrAssignC(&seqFtFmtSwiss, "swissprot");

    if(!ajBuffreadLineStore(buff, &seqReadLine, seqin->Text, &thys->TextPtr))
	return ajFalse;

    bufflines++;

    /* for GCG formatted databases */

    while(ajStrPrefixC(seqReadLine, "WP "))
    {
	if(!ajBuffreadLineStore(buff, &seqReadLine,
			       seqin->Text, &thys->TextPtr))
	    return ajFalse;
	bufflines++;
    }

    /* extra blank lines */

    while(ajStrIsWhite(seqReadLine))
    {
	if(!ajBuffreadLineStore(buff, &seqReadLine,
			       seqin->Text, &thys->TextPtr))
	    return ajFalse;
	bufflines++;
    }

    ajDebug("seqReadSwiss first line '%S'\n", seqReadLine);

    if(!ajStrPrefixC(seqReadLine, "ID   "))
    {
	ajFilebuffReset(buff);
	return ajFalse;
    }
    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
    ajStrTokenNextParse(&handle, &token);	/* 'ID' */
    ajStrTokenNextParse(&handle, &token);	/* entry name */

    seqSetName(&thys->Name, token);

    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text, &thys->TextPtr);
    while(ok && !ajStrPrefixC(seqReadLine, "SQ   "))
    {
	bufflines++;

	/* check for Staden Experiment format instead */
	if(ajStrPrefixC(seqReadLine, "EN   ") ||
	   ajStrPrefixC(seqReadLine, "TN   ") ||
	   ajStrPrefixC(seqReadLine, "EX   ") )
	{
	    ajFilebuffReset(buff);
	    ajStrTokenDel(&handle);
	    ajStrDel(&token);
	    return ajFalse;;
	}

	if(ajStrPrefixC(seqReadLine, "AC   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " ;\n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'AC' */
	    while(ajStrTokenNextParse(&handle, &token))
		seqAccSave(thys, token);
	}

	if(ajStrPrefixC(seqReadLine, "DE   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'DE' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* desc */
	    if(ajStrGetLen(thys->Desc))
	    {
		ajStrAppendC(&thys->Desc, " ");
		ajStrAppendS(&thys->Desc, token);
	    }
	    else
		ajStrAssignS(&thys->Desc, token);
	}

	if(ajStrPrefixC(seqReadLine, "KW   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'KW' */
	    while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
	    {
		liststr = ajStrNewS(token);
		ajStrTrimWhite(&liststr);
		ajListstrPushAppend(thys->Keylist, liststr);
	    }
	}

	if(ajStrPrefixC(seqReadLine, "OS   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'OS' */
	    while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
	    {
		ajStrAssignS(&tmpstr, token);
		ajStrTrimWhite(&tmpstr);
		seqTaxSave(thys, tmpstr, 1);
		ajStrDel(&tmpstr);
	    }
	}

	if(ajStrPrefixC(seqReadLine, "OC   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'OC' */
	    while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
	    {
		ajStrAssignS(&tmpstr, token);
		ajStrTrimWhite(&tmpstr);
		seqTaxSave(thys, tmpstr, 0);
		ajStrDel(&tmpstr);
	    }
	}

	if(ajStrPrefixC(seqReadLine, "FT   "))
	    if(seqinUfoLocal(seqin))
	    {
		if(!dofeat)
		{
		    dofeat = ajTrue;
		    ajFeattabInDel(&seqin->Ftquery);
		    seqin->Ftquery = ajFeattabInNewSS(seqFtFmtSwiss,
						      thys->Name, "N");
		    ajDebug("seqin->Ftquery ftfile %x\n",
			    seqin->Ftquery->Handle);
		}
		ajFilebuffLoadS(seqin->Ftquery->Handle, seqReadLine);
		/* ajDebug("SWISS FEAT saved line:\n%S", seqReadLine); */
	    }

	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
    }

    if(dofeat)
    {
 	ajDebug("EMBL FEAT TabIn %x\n", seqin->Ftquery);
	ajFeattableDel(&thys->Fttable);
	thys->Fttable = ajFeatRead(seqin->Ftquery);
	/* ajFeattableTrace(thys->Fttable); */
	ajFeattabInClear(seqin->Ftquery);
    }

    if(ajStrGetLen(seqin->Inseq))
    {
	/* we have a sequence to use */
	ajStrAssignS(&thys->Seq, seqin->Inseq);
	if(seqin->Text)
	{
	    seqTextSeq(&thys->TextPtr, seqin->Inseq);
	    ajFmtPrintAppS(&thys->TextPtr, "//\n");
	}
    }
    else
    {
	/* read the sequence and terminator */
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	while(ok && !ajStrPrefixC(seqReadLine, "//"))
	{
	    seqAppend(&thys->Seq, seqReadLine);
	    bufflines++;
	    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
				    &thys->TextPtr);
	}
    }

    ajSeqSetProt(thys);

    ajFilebuffClear(buff, 0);
    ajStrDel(&token);
    ajStrTokenDel(&handle);

    return ajTrue;
}




/* @funcstatic seqReadEmbl ****************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using EMBL format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadEmbl(AjPSeq thys, AjPSeqin seqin)
{

    AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    ajuint bufflines         = 0;
    AjBool ok;
/*    AjBool okdate; */
    AjPFilebuff buff;
    AjPStr tmpstr = NULL;
    AjBool dofeat        = ajFalse;
    AjPStr liststr;			/* for lists, do not delete */
    AjPStr datestr = NULL;
    AjPStr relstr = NULL;
    AjPStr cmtstr = NULL;		/* stored in AjPSeq - do not delete */
    AjPStr xrefstr = NULL;		/* stored in AjPSeq - do not delete */
    ajuint icount;
    AjPSeqRef seqref = NULL;
    ajuint refnum;
    ajuint seqlen=1024;
    ajuint tmplen;
    ajuint itmp;
    ajuint i;

    buff = seqin->Filebuff;

    if(!seqFtFmtEmbl)
	ajStrAssignC(&seqFtFmtEmbl, "embl");

    if(!ajBuffreadLineStore(buff, &seqReadLine,
			   seqin->Text, &thys->TextPtr))
	return ajFalse;

    bufflines++;

    /* for GCG formatted databases */

    while(ajStrPrefixC(seqReadLine, "WP "))
    {
	if(!ajBuffreadLineStore(buff, &seqReadLine,
			       seqin->Text, &thys->TextPtr))
	    return ajFalse;
	bufflines++;
    }

    /* extra blank lines */

    while(ajStrIsWhite(seqReadLine))
    {
	if(!ajBuffreadLineStore(buff, &seqReadLine,
			       seqin->Text, &thys->TextPtr))
	    return ajFalse;
	bufflines++;
    }

    ajDebug("seqReadEmbl first line '%S'\n", seqReadLine);

    if(!ajStrPrefixC(seqReadLine, "ID   "))
    {
	ajFilebuffReset(buff);
	return ajFalse;
    }

    if(seqin->Text)
	ajStrAssignC(&thys->TextPtr,ajStrGetPtr(seqReadLine));

    ajDebug("seqReadEmbl ID line found\n");
    ajStrTokenAssignC(&handle, seqReadLine, " ;\t\n\r");
    ajStrTokenNextParse(&handle, &token);	/* 'ID' */
    ajStrTokenNextParse(&handle, &token);	/* entry name */

    seqSetName(&thys->Name, token);

    ajStrTokenNextParse(&handle, &token);	/* SV for new syntax */

    if(ajStrMatchC(token, "SV"))	/* new post-2006 EMBL line */
    {
	ajStrTokenNextParse(&handle, &token);	/* SV */
	ajStrInsertK(&token, 0, '.');
	ajStrInsertS(&token, 0, thys->Name);
	seqSvSave(thys, token);

	ajStrTokenNextParse(&handle, &token); /* linear or circular */
	if(ajStrMatchC(token, "circular"))
	    thys->Circular = ajTrue;

	ajStrTokenNextParseC(&handle, ";\t\n\r", &token);
	ajStrTrimWhite(&token);
	ajSeqmolSetEmbl(&thys->Molecule, token);

	ajStrTokenNextParse(&handle, &token);
	ajStrTrimWhite(&token);
	ajStrAssignS(&thys->Class, token);

	ajStrTokenNextParse(&handle, &token);
	ajStrTrimWhite(&token);
	ajStrAssignS(&thys->Division, token);

	ajStrTokenNextParse(&handle, &token);
	ajStrTrimEndC(&token, "BP.");
	ajStrTrimWhite(&token);
	ajStrToUint(token, &seqlen);
    }
    else		     /* test for a SwissProt/SpTrEMBL entry */
    {
	if(ajStrFindC(seqReadLine, " PRT; ")>= 0  ||
	   ajStrFindC(seqReadLine, " Unreviewed; ") >= 0 ||
	   ajStrFindC(seqReadLine, " Reviewed; ") >= 0 ||
	   ajStrFindC(seqReadLine, " Preliminary; ") >= 0 
	   )
	{
	    ajFilebuffReset(buff);
	    ajStrTokenDel(&handle);
	    ajStrDel(&token);
	    return ajFalse;
	}
    }

    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text, &thys->TextPtr);

    while(ok && !ajStrPrefixC(seqReadLine, "SQ"))
    {
	bufflines++;

	/* check for Staden Experiment format instead */
	if(ajStrPrefixC(seqReadLine, "EN   ") ||
	   ajStrPrefixC(seqReadLine, "TN   ") ||
	   ajStrPrefixC(seqReadLine, "EX   ") )
	{
	    ajFilebuffReset(buff);
	    ajStrDel(&token);
	    return ajFalse;;
	}

	else if(ajStrPrefixC(seqReadLine, "FH   ") ||
		ajStrPrefixC(seqReadLine, "AH   "))
	    ok = ajTrue;		/* ignore these lines */

	else if(ajStrPrefixC(seqReadLine, "AC   ") ||
	   ajStrPrefixC(seqReadLine, "PA   ") ) /* emblcds database format */
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " ;\n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'AC' */
	    while(ajStrTokenNextParse(&handle, &token))
		seqAccSave(thys, token);
	}

	else if(ajStrPrefixC(seqReadLine, "SV   ") ||
	   ajStrPrefixC(seqReadLine, "IV   ") ) /* emblcds database format */
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'SV' */
	    ajStrTokenNextParse(&handle, &token); /* version */
	    seqSvSave(thys, token);
	}

	else if(ajStrPrefixC(seqReadLine, "DE   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'DE' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* desc */
	    if(ajStrGetLen(thys->Desc))
	    {
		ajStrAppendC(&thys->Desc, " ");
		ajStrAppendS(&thys->Desc, token);
	    }
	    else
		ajStrAssignS(&thys->Desc, token);
	}

	if(ajStrPrefixC(seqReadLine, "KW   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'KW' */
	    while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
	    {
		liststr = ajStrNewS(token);
		ajStrTrimWhite(&liststr);
		ajListstrPushAppend(thys->Keylist, liststr);
	    }
	}

	else if(ajStrPrefixC(seqReadLine, "OS   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'OS' */

	    /* maybe better remove . from this, and trim from end */
	    while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
	    {
		ajStrAssignS(&tmpstr, token);
		ajStrTrimWhite(&tmpstr);
		seqTaxSave(thys, tmpstr, 1);
		ajStrDel(&tmpstr);
	    }
	}

	else if(ajStrPrefixC(seqReadLine, "OC   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'OC' */

	    /* maybe better remove . from this, and trim from end */
	    while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
	    {
		ajStrAssignS(&tmpstr, token);
		ajStrTrimWhite(&tmpstr);
		seqTaxSave(thys, tmpstr, 0);
		ajStrDel(&tmpstr);
	    }
	}

	else if(ajStrPrefixC(seqReadLine, "OG   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'OG' */

	    /* maybe better remove . from this, and trim from end */
	    while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
	    {
		ajStrAssignS(&tmpstr, token);
		ajStrTrimWhite(&tmpstr);
		seqTaxSave(thys, tmpstr, 2);
		ajStrDel(&tmpstr);
	    }
	}

	else if(ajStrPrefixC(seqReadLine, "CC   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'CC' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* comment */
	    if(ajStrGetLen(cmtstr))
		ajStrAppendC(&cmtstr, "\n");
	    ajStrAppendS(&cmtstr, token);

/* trying to keep commments in one long string with embedded returns
** probably fails for long comments - and also fails for contact details
** which have very short comment lines
** switch to just keeping original lines */

/*
	    if(ajStrGetLen(cmtstr))
	    {
		if(ajStrGetLen(token))
		{
		    if(ajStrGetCharLast(cmtstr) != '\n')
			ajStrAppendK(&cmtstr, ' ');
		    ajStrAppendS(&cmtstr, token);
		}
		else
		{
		    if(ajStrGetCharLast(cmtstr) != '\n')
			ajStrAppendK(&cmtstr, '\n');
		    ajStrAppendC(&cmtstr, " \n");
		}
	    }
	    else
		ajStrAssignS(&cmtstr, token);
	    if(ajStrGetCharLast(token) == '.')
		ajStrAppendK(&cmtstr, '\n');
*/
	}

	else if(ajStrPrefixC(seqReadLine, "DR   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'DR' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* xref */
	    ajStrAssignS(&xrefstr, token);
	    ajListPushAppend(thys->Xreflist, xrefstr);
	    xrefstr = NULL;
	}

	else if(ajStrPrefixC(seqReadLine, "RN   "))
	{
	    seqref = ajSeqrefNew();
	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'RN' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* [num] */
	    ajStrAssignSubS(&tmpstr, token, 1, -2);
	    ajStrToUint(tmpstr, &refnum);
	    ajSeqrefSetnumNumber(seqref, refnum);
	}

	else if(ajStrPrefixC(seqReadLine, "RG   "))
	{
	    if(!seqref)
		seqref = ajSeqrefNew();
	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'RG' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* groupname */
	    ajSeqrefAppendGroupname(seqref, token);
	}

	else if(ajStrPrefixC(seqReadLine, "RX   "))
	{
	    if(!seqref)
		seqref = ajSeqrefNew();
	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'RX' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* xref */
	    ajSeqrefAppendXref(seqref, token);
	}

	else if(ajStrPrefixC(seqReadLine, "RP   "))
	{
	    if(!seqref)
		seqref = ajSeqrefNew();
	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'RP' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* position */
	    ajSeqrefAppendPosition(seqref, token);
	}

	else if(ajStrPrefixC(seqReadLine, "RA   "))
	{
	    if(!seqref)
		seqref = ajSeqrefNew();
	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'RA' */
	    ajStrTokenNextParseC(&handle, "\n\r;", &token); /* authors */
	    ajSeqrefAppendAuthors(seqref, token);
	}

	else if(ajStrPrefixC(seqReadLine, "RT   "))
	{
	    if(!seqref)
		seqref = ajSeqrefNew();
	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'RT' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* title */
	    if(!ajStrMatchC(token, ";"))
		ajSeqrefAppendTitle(seqref, token);
	}

	else if(ajStrPrefixC(seqReadLine, "RL   "))
	{
	    if(!seqref)
		seqref = ajSeqrefNew();
	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'RL' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* authors */
	    ajSeqrefAppendLocation(seqref, token);
	}

	else if(ajStrPrefixC(seqReadLine, "RC   "))
	{
	    if(!seqref)
		seqref = ajSeqrefNew();
	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'RC' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* comment */
	    ajSeqrefAppendComment(seqref, token);
	}

	else if(ajStrPrefixC(seqReadLine, "FT   "))
	{
	    if(seqinUfoLocal(seqin))
	    {
		if(!dofeat)
		{
		    dofeat = ajTrue;
		    ajFeattabInDel(&seqin->Ftquery);
		    seqin->Ftquery = ajFeattabInNewSS(seqFtFmtEmbl,
						      thys->Name, "N");
		    /* ajDebug("seqin->Ftquery Handle %x\n",
		       seqin->Ftquery->Handle); */
		}
		ajFilebuffLoadS(seqin->Ftquery->Handle, seqReadLine);
		/* ajDebug("EMBL FEAT saved line:\n%S", seqReadLine); */
	    }
	}

	else if(ajStrPrefixC(seqReadLine, "DT   "))
	{
	    if(!thys->Date)
		thys->Date = ajSeqdateNew();
	    ajStrTokenAssignC(&handle, seqReadLine, " (),");
	    icount = 0;
/*	    okdate = ajTrue; */
	    while(ajStrTokenNextParse(&handle, &token))
	    {
		icount++;
		if(icount==2)
		    ajStrAssignS(&datestr, token);
		else if(icount==4)
		    ajStrAssignS(&relstr, token);
		else if(icount==5)
		{
		    if(ajStrMatchC(token, "Created"))
		    {
			ajSeqdateSetCreateS(thys->Date, datestr);
			ajStrAssignS(&thys->Date->CreRel, relstr);
		    }
		}
		else if(icount==8)
		{
			ajSeqdateSetModifyS(thys->Date, datestr);
			ajStrAssignS(&thys->Date->ModRel, relstr);
			ajStrAssignS(&thys->Date->ModVer, token);
		}
	    }
	}


	else if(ajStrPrefixC(seqReadLine, "XX"))
	{
	    if(seqref)
	    {
		ajSeqrefStandard(seqref);
		ajListPushAppend(thys->Reflist, seqref);
		seqref = NULL;
	    }
	    if(ajStrGetLen(cmtstr))
	    {
		ajListPushAppend(thys->Cmtlist, cmtstr);
		cmtstr = NULL;
	    }

	}

/* ignored line types */

/* other line types */
/*
	if(ajStrPrefixC(seqReadLine, "RN   "))
	if(ajStrPrefixC(seqReadLine, "RC   "))
	if(ajStrPrefixC(seqReadLine, "RP   "))
	if(ajStrPrefixC(seqReadLine, "RX   "))
	if(ajStrPrefixC(seqReadLine, "RG   "))
	if(ajStrPrefixC(seqReadLine, "RA   "))
	if(ajStrPrefixC(seqReadLine, "RT   "))
	if(ajStrPrefixC(seqReadLine, "RL   "))
	if(ajStrPrefixC(seqReadLine, "AS   "))
	if(ajStrPrefixC(seqReadLine, "CO   "))
	if(ajStrPrefixC(seqReadLine, "CC   "))
*/

	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
    }

    /* now we are on the SQ line - or there was nothing */

    ajStrTokenAssignC(&handle, seqReadLine, " ");
    ajStrTokenNextParse(&handle, &token); /* 'SQ' */
    ajStrTokenNextParse(&handle, &token); /* 'Sequence' */
    ajStrTokenNextParse(&handle, &token); /* len */
    ajStrToUint(token, &tmplen);
    if(tmplen > seqlen)
	seqlen = tmplen;
    ajStrTokenNextParse(&handle, &token); /* BP; */
    tmplen = 0;
    for(i=0;i<4;i++)
    {
	ajStrTokenNextParse(&handle, &token); /* count */
	ajStrToUint(token, &itmp);
	ajStrTokenNextParse(&handle, &token); /* 'A' 'C' 'G' 'T' 'other' */
	tmplen += itmp;
    }
    if(tmplen > seqlen)
	seqlen = tmplen;

    if(dofeat)
    {
	/* ajDebug("EMBL FEAT TabIn %x\n", seqin->Ftquery); */
	ajFeattableDel(&thys->Fttable);
	thys->Fttable = ajFeatRead(seqin->Ftquery);
	/* ajFeattableTrace(thys->Fttable); */
	ajFeattabInClear(seqin->Ftquery);
    }

    if(ajStrGetLen(seqin->Inseq))
    {
	/* we have a sequence to use */
	ajStrAssignS(&thys->Seq, seqin->Inseq);
	if(seqin->Text)
	{
	    seqTextSeq(&thys->TextPtr, seqin->Inseq);
	    ajFmtPrintAppS(&thys->TextPtr, "//\n");
	}
    }
    else
    {
	/* read the sequence and terminator */
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	ajStrSetRes(&thys->Seq, seqlen+1);
	while(ok && !ajStrPrefixC(seqReadLine, "//"))
	{
	    seqAppend(&thys->Seq, seqReadLine);
	    bufflines++;
	    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
				    &thys->TextPtr);
	}
    }

    if(!ajSeqIsNuc(thys))
    {
        ajFilebuffReset(buff);
        ajStrDel(&tmpstr);
        ajStrDel(&token);
        ajStrDel(&datestr);
        ajStrDel(&relstr);
        ajStrTokenDel(&handle);
        return ajFalse;
    }
    
    ajSeqSetNuc(thys);

    ajFilebuffClear(buff, 0);

    ajStrDel(&tmpstr);
    ajStrDel(&token);
    ajStrDel(&datestr);
    ajStrDel(&relstr);

    ajStrTokenDel(&handle);

    /* ajSeqTrace(thys); */

    return ajTrue;
}




/* @funcstatic seqReadExperiment **********************************************
**
** Given data in a sequence structure, tries to read everything needed
** using Staden experiment format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadExperiment(AjPSeq thys, AjPSeqin seqin)
{

    AjPStrTok handle  = NULL;
    AjPStrTok handle2 = NULL;
    AjPStr token      = NULL;
    AjPStr token2     = NULL;
    ajuint bufflines          = 0;
    AjBool ok;
    AjPFilebuff buff;
    AjPStr tmpstr = NULL;
    AjBool dofeat        = ajFalse;
    AjPStr liststr;			/* for lists, do not delete */
    AjPStr accvalstr = NULL;
    ajuint i;
    ajuint j;
    ajuint ilen;
    AjBool avok;

    buff = seqin->Filebuff;

    if(!seqFtFmtEmbl)
	ajStrAssignC(&seqFtFmtEmbl, "embl");

    if(!ajBuffreadLineStore(buff, &seqReadLine,
			   seqin->Text, &thys->TextPtr))
	return ajFalse;

    bufflines++;

    ajDebug("seqReadExperiment first line '%S'\n", seqReadLine);

    if(!ajStrPrefixC(seqReadLine, "ID   "))
    {
	ajFilebuffReset(buff);
	return ajFalse;
    }

    if(seqin->Text)
	ajStrAssignC(&thys->TextPtr,ajStrGetPtr(seqReadLine));

    ajDebug("seqReadExperiment ID line found\n");
    ajStrTokenAssignC(&handle, seqReadLine, " \n\r\t");
    ajStrTokenNextParse(&handle, &token);	/* 'ID' */
    ajStrTokenNextParse(&handle, &token);	/* entry name */

    seqSetName(&thys->Name, token);

    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text, &thys->TextPtr);

    while(ok && !ajStrPrefixC(seqReadLine, "SQ"))
    {
	bufflines++;

	if(ajStrPrefixC(seqReadLine, "EX   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'EX'*/
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /*  expt. desc. */
	    if(ajStrGetLen(thys->Desc))
	    {
		ajStrAppendC(&thys->Desc, " ");
		ajStrAppendS(&thys->Desc, token);
	    }
	    else
		ajStrAssignS(&thys->Desc, token);
	}

	if(ajStrPrefixC(seqReadLine, "AV   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'AV' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* desc */
	    if(ajStrGetLen(accvalstr))
	    {
		ajStrAppendC(&accvalstr, " ");
		ajStrAppendS(&accvalstr, token);
	    }
	    else
		ajStrAssignS(&accvalstr, token);
	}

	/* standard EMBL records are allowed */

	if(ajStrPrefixC(seqReadLine, "AC   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " ;\n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'AC' */
	    while(ajStrTokenNextParse(&handle, &token))
		seqAccSave(thys, token);
	}

	if(ajStrPrefixC(seqReadLine, "SV   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'SV' */
	    ajStrTokenNextParse(&handle, &token); /* version */
	    seqSvSave(thys, token);
	}

	if(ajStrPrefixC(seqReadLine, "DE   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'DE' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* desc */
	    if(ajStrGetLen(thys->Desc))
	    {
		ajStrAppendC(&thys->Desc, " ");
		ajStrAppendS(&thys->Desc, token);
	    }
	    else
		ajStrAssignS(&thys->Desc, token);
	}

	if(ajStrPrefixC(seqReadLine, "KW   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'KW' */
	    while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
	    {
		liststr = ajStrNewS(token);
		ajStrTrimWhite(&liststr);
		ajListstrPushAppend(thys->Keylist, liststr);
	    }
	}

	if(ajStrPrefixC(seqReadLine, "OS   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'OS' */
	    while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
	    {
		ajStrAssignS(&tmpstr, token);
		ajStrTrimWhite(&tmpstr);
		seqTaxSave(thys, tmpstr, 1);
	    }
	}

	if(ajStrPrefixC(seqReadLine, "OC   "))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'OC' */
	    while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
	    {
		ajStrAssignS(&tmpstr, token);
		ajStrTrimWhite(&tmpstr);
		seqTaxSave(thys, tmpstr, 0);
	    }
	}
	ajStrDel(&tmpstr);

	if(ajStrPrefixC(seqReadLine, "FT   "))
	    if(seqinUfoLocal(seqin))
	    {
		if(!dofeat)
		{
		    dofeat = ajTrue;
		    ajFeattabInDel(&seqin->Ftquery);
		    seqin->Ftquery = ajFeattabInNewSS(seqFtFmtEmbl,
						      thys->Name, "N");
		    /* ajDebug("seqin->Ftquery Handle %x\n",
		       seqin->Ftquery->Handle); */
		}
		ajFilebuffLoadS(seqin->Ftquery->Handle, seqReadLine);
		/* ajDebug("EMBL FEAT saved line:\n%S", seqReadLine); */
	    }

	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
    }

    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text, &thys->TextPtr);
    while(ok && !ajStrPrefixC(seqReadLine, "//"))
    {
	seqAppend(&thys->Seq, seqReadLine);
	bufflines++;
	ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
				&thys->TextPtr);
    }
    ajDebug("Sequence read %d bases\n", ajStrGetLen(thys->Seq));

    while(ok && !ajStrPrefixC(seqReadLine, "ID   "))
    {
	bufflines++;
	ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
				&thys->TextPtr);
    }

    if(ok)
	ajFilebuffClearStore(buff, 1,
			     seqReadLine, seqin->Text, &thys->TextPtr);
    else
	ajFilebuffClear(buff, 0);

    if(dofeat)
    {
	/* ajDebug("EMBL FEAT TabIn %x\n", seqin->Ftquery); */
	ajFeattableDel(&thys->Fttable);
	thys->Fttable = ajFeatRead(seqin->Ftquery);
	/* ajFeattableTrace(thys->Fttable); */
	ajFeattabInClear(seqin->Ftquery);
    }

    if(ajStrGetLen(accvalstr))
    {
	ilen = ajStrGetLen(thys->Seq);
	AJCNEW0(thys->Accuracy,ilen);
	ajStrTokenAssignC(&handle, accvalstr, " ");
	avok = ajTrue;
	for(i=0;i<ilen;i++)
	{
	    if(!ajStrTokenNextParse(&handle, &token))
	    {
		ajWarn("Missing accuracy for base %d in experiment format\n",
		       i+1);
		avok = ajFalse;
		break;
	    }
	    ajStrTokenAssignC(&handle2, token, ",");
	    while(ajStrTokenNextParse(&handle2, &token2))
	    {
		if(ajStrToUint(token2, &j))
		{
		    if(j > thys->Accuracy[i])
			thys->Accuracy[i] = j;
		}
		else
		{
		    ajWarn("Bad accuracy '%S' for base %d "
			   "in experiment format\n",
			   token, i+1);
		    avok = ajFalse;
		    break;
		}
	    }
	    ajDebug("Accval[%d] %3d '%S'\n", i+1, thys->Accuracy[i], token);
	}
	if(!avok)
	    AJFREE(thys->Accuracy);
    }

    ajStrDel(&token);
    ajStrDel(&token2);
    ajStrDel(&accvalstr);

    ajStrTokenDel(&handle);
    ajStrTokenDel(&handle2);


    /* ajSeqTrace(thys); */

    return ajTrue;
}




/* @funcstatic seqReadGenbank *************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using Genbank format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadGenbank(AjPSeq thys, AjPSeqin seqin)
{
    AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    ajuint bufflines         = 0;
    AjBool ok;
    AjBool done = ajFalse;
    AjPFilebuff buff;
    AjPStr tmpstr = NULL;
    AjPStr tmpstr2 = NULL;
    AjBool dofeat        = ajFalse;
    AjPSeqQuery qry;
    AjPStr liststr;			/* for lists, do not delete */
    AjPSeqRef seqref = NULL;
    ajuint refnum;
    ajuint seqlen = 1024;
    ajint i;

    ajDebug("seqReadGenbank\n");

    buff = seqin->Filebuff;
    qry  = seqin->Query;

    if(!seqFtFmtGenbank)
	ajStrAssignC(&seqFtFmtGenbank, "genbank");

    if(!ajBuffreadLine(buff, &seqReadLine))
	return ajFalse;
    bufflines++;

    ok = ajTrue;

    /* extra blank lines */

    while(ajStrIsWhite(seqReadLine))
    {
	if(!ajBuffreadLineStore(buff, &seqReadLine,
			       seqin->Text, &thys->TextPtr))
	    return ajFalse;
	bufflines++;
    }

    /* for GCG formatted databases */

    if(ajStrPrefixC(seqReadLine, "WPCOMMENT"))
    {
	ok = ajBuffreadLine(buff, &seqReadLine);
	bufflines++;
	while(ok && ajStrPrefixC(seqReadLine, " "))
	{
	    ok = ajBuffreadLine(buff, &seqReadLine);
	    bufflines++;
	}
    }

    /* This loop necessary owing to headers on GB distro files */
    if(ajStrFindC(seqReadLine,"Genetic Sequence Data Bank") >= 0)
    {
	while(ok && !ajStrPrefixC(seqReadLine, "LOCUS"))
	{
	    ok = ajBuffreadLine(buff, &seqReadLine);
	    bufflines++;
	}
    }

    if(!ok)
    {
	ajFilebuffReset(buff);
	return ajFalse;
    }

    if(!ajStrPrefixC(seqReadLine, "LOCUS"))
    {
	ajDebug("failed - LOCUS not found - first line was\n%S\n",
		seqReadLine);
	ajFilebuffReset(buff);
	return ajFalse;
    }

    if(seqin->Text)
	ajStrAssignC(&thys->TextPtr,ajStrGetPtr(seqReadLine));

    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
    i=0;
    while(ajStrTokenNextParse(&handle, &token))
    {
	switch(++i)
	{
	case 1:
	    break;
	case 2:
	    seqSetName(&thys->Name, token);
	    break;
	case 3:
	    ajStrToUint(token, &seqlen);
	    break;
	case 4:
	    if(!ajStrMatchC(token, "bp"))
		ajWarn("bad Genbank LOCUS line '%S'", seqReadLine);
	    break;
	case 5:
	    ajSeqmolSetGb(&thys->Molecule, token);
	    break;
	case 6:
	    if(ajStrMatchC(token, "circular"))
		thys->Circular = ajTrue;
	    break;
	case 7:
	    ajSeqdivSetGb(&thys->Division, token);
	    ajSeqclsSetGb(&thys->Class, token);
	    break;
	case 8:
	    if(!thys->Date)
		thys->Date = ajSeqdateNew();
	    ajSeqdateSetModifyS(thys->Date, token);
	    break;
	default:
	    break;
	}
    }

    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text, &thys->TextPtr);
    while(ok &&
	  !ajStrPrefixC(seqReadLine, "ORIGIN") &&
	  !ajStrPrefixC(seqReadLine, "BASE COUNT"))
    {
	done = ajFalse;
	bufflines++;

	if(ajStrPrefixC(seqReadLine, "DEFINITION"))
	{
	    ajDebug("definition found\n");
	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'DEFINITION' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* desc */
	    ajStrAssignS(&thys->Desc, token);
	    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
				    &thys->TextPtr);
	    done = ajTrue;
	    while(ok && ajStrPrefixC(seqReadLine, " "))
	    {
		bufflines++;
		ajStrTokenAssignC(&handle, seqReadLine, " ");
		ajStrTokenNextParseC(&handle, "\n\r", &token);
		ajStrAppendC(&thys->Desc, " ");
		ajStrAppendS(&thys->Desc, token);
		ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
					&thys->TextPtr);
	    }
	}

	else if(ajStrPrefixC(seqReadLine, "ACCESSION"))
	{
	    ajDebug("accession found\n");

	    ajStrTokenAssignC(&handle, seqReadLine, " ;\n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'ACCESSION' */
	    while(ajStrTokenNextParse(&handle, &token))
		seqAccSave(thys, token);
	}

	else if(ajStrPrefixC(seqReadLine, "VERSION"))
	{
	    ajDebug("seqversion found\n");

	    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'VERSION' */
	    ajStrTokenNextParse(&handle, &token);
	    seqSvSave(thys, token);
	    if(ajStrTokenNextParseC(&handle, ": \n\r", &token)) /* GI: */
	    {
		ajStrTokenNextParse(&handle, &token);
		ajStrAssignS(&thys->Gi, token);
	    }
	}

	else if(ajStrPrefixC(seqReadLine, "SOURCE"))
	{
	    ajDebug("source found\n");
	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'SOURCE' */
	    ajStrTokenNextParseC(&handle, "\n\r", &token); /* source */
	    ajStrAssignS(&thys->Tax, token);
	    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
				    &thys->TextPtr);
	    done = ajTrue;
	    while(ok && ajStrPrefixC(seqReadLine, " "))
	    {
		bufflines++;
/* process organism lines */
		ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
					&thys->TextPtr);
	    }
	}

	else if(ajStrPrefixC(seqReadLine, "FEATURES"))
	{
	    if(seqinUfoLocal(seqin))
	    {
		ajDebug("features found\n");
		if(!dofeat)
		{
		    dofeat = ajTrue;
		    ajFeattabInDel(&seqin->Ftquery);
		    seqin->Ftquery = ajFeattabInNewSS(seqFtFmtGenbank,
						      thys->Name, "N");
		    ajDebug("seqin->Ftquery Handle %x\n",
			    seqin->Ftquery->Handle);
		    /* ajDebug("GENBANK FEAT first line:\n%S", seqReadLine); */
		}
		ajFilebuffLoadS(seqin->Ftquery->Handle, seqReadLine);
		ok = ajBuffreadLineStore(buff, &seqReadLine,
					seqin->Text, &thys->TextPtr);
		done = ajTrue;
		while(ok && ajStrPrefixC(seqReadLine, " "))
		{
		    bufflines++;
		    ajFilebuffLoadS(seqin->Ftquery->Handle, seqReadLine);
		    /* ajDebug("GENBANK FEAT saved line:\n%S", seqReadLine); */
		    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
					    &thys->TextPtr);
		}
	    }
	}

	else if(ajStrPrefixC(seqReadLine, "REFERENCE"))
	{
	    ajDebug("reference found\n");
	    seqref = ajSeqrefNew();
	    ajStrTokenAssignC(&handle, seqReadLine, " \n\r");
	    ajStrTokenNextParse(&handle, &token); /* 'REFERENCE' */
	    ajStrTokenNextParse(&handle, &token); /* number */
	    ajStrToUint(token, &refnum);
	    ajSeqrefSetnumNumber(seqref, refnum);
	    ajStrAssignClear(&tmpstr2);
	    while (ajStrTokenNextParse(&handle, &token))
	    {
		if(ajStrMatchC(token, "(bases")) continue;
		if(ajStrMatchC(token, "to")) continue;
		if(!ajStrGetLen(tmpstr2))
		    ajStrAssignS(&tmpstr2, token);
		if(ajStrSuffixC(token, ")"))
		{
		    ajStrTrimEndC(&token, ")");
		    ajStrAppendK(&tmpstr2, '-');
		    ajStrAppendS(&tmpstr2, token);
		}
	    }
	    ajSeqrefSetPosition(seqref, tmpstr2);

	    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
				    &thys->TextPtr);
	    done = ajTrue;
	    while(ok && ajStrPrefixC(seqReadLine, " "))
	    {
		ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
					&thys->TextPtr);
		bufflines++;
	    }
	    ajSeqrefStandard(seqref);
	    ajListPushAppend(thys->Reflist, seqref);
	    seqref = NULL;
	}

	else if(ajStrPrefixC(seqReadLine, "KEYWORDS"))
	{
	    ajDebug("keywords found\n");
	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'KEYWORDS' */
	    while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
	    {
		liststr = ajStrNewS(token);
		ajStrTrimWhite(&liststr);
		ajListstrPushAppend(thys->Keylist, liststr);
	    }

	    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
				    &thys->TextPtr);
	    done = ajTrue;
	    while(ok && ajStrPrefixC(seqReadLine, " "))
	    {
		bufflines++;
		ajStrTokenAssignC(&handle, seqReadLine, " ");
		while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
		{
		    liststr = ajStrNewS(token);
		    ajStrTrimWhite(&liststr);
		    ajListstrPushAppend(thys->Keylist, liststr);
		}
		ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
					&thys->TextPtr);
	    }
	}

	else if(ajStrPrefixC(seqReadLine, "  ORGANISM"))
	{
	    ajDebug("organism found\n");
	    ajStrTokenAssignC(&handle, seqReadLine, " ");
	    ajStrTokenNextParse(&handle, &token); /* 'ORGANISM' */
	    while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
	    {
		ajStrAssignS(&tmpstr, token);
		ajStrTrimWhite(&tmpstr);
		seqTaxSave(thys, tmpstr, 0);
	    }

	    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
				    &thys->TextPtr);
	    done = ajTrue;
	    while(ok && ajStrPrefixC(seqReadLine, "    "))
	    {
		bufflines++;
		ajStrTokenAssignC(&handle, seqReadLine, " ");
		while(ajStrTokenNextParseC(&handle, ".;\n\r", &token))
		{
		    ajStrAssignS(&tmpstr, token);
		    ajStrTrimWhite(&tmpstr);
		    seqTaxSave(thys, tmpstr, 0);
		}
		ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
					&thys->TextPtr);
	    }
	}

	if(!done)
	    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
				    &thys->TextPtr);
    }

    if(dofeat)
    {
	ajDebug("GENBANK FEAT TabIn %x\n", seqin->Ftquery);
	ajFeattableDel(&thys->Fttable);
	thys->Fttable = ajFeatRead(seqin->Ftquery);
	/* ajFeattableTrace(thys->Fttable); */
	ajFeattabInClear(seqin->Ftquery);
    }

    if(ajStrGetLen(seqin->Inseq))
    {
	/* we have a sequence to use */
	ajDebug("Got an Inseq sequence\n");
	if(ajStrMatchC(qry->Method,"gcg"))
	    while(ok && !ajStrPrefixC(seqReadLine,"ORIGIN"))
		ok = ajBuffreadLineStore(buff,&seqReadLine, seqin->Text,
					&thys->TextPtr);

	ajStrAssignS(&thys->Seq, seqin->Inseq);
	if(seqin->Text)
	{
	    seqTextSeq(&thys->TextPtr, seqin->Inseq);
	    ajFmtPrintAppS(&thys->TextPtr, "//\n");
	}
    }
    else
    {
	/* read the sequence and terminator */
	ajDebug("sequence start at '%S'\n", seqReadLine);
	while(!ajStrPrefixC(seqReadLine,"ORIGIN") &&
	      !ajStrPrefixC(seqReadLine,"BASE COUNT"))
	    if(!ajBuffreadLineStore(buff,&seqReadLine,
				   seqin->Text, &thys->TextPtr))
		break;
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
	ajStrSetRes(&thys->Seq, seqlen+1);
	while(ok && !ajStrPrefixC(seqReadLine, "//"))
	{
	    if(!ajStrPrefixC(seqReadLine, "ORIGIN") &&
	       !ajStrPrefixC(seqReadLine,"BASE COUNT"))
		seqAppend(&thys->Seq, seqReadLine);
	    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text,
				    &thys->TextPtr);
	    bufflines++;
	}
    }

    if(!ajStrMatchC(qry->Method,"gcg"))
	while(ok && !ajStrPrefixC(seqReadLine,"//"))
	    ok = ajBuffreadLineStore(buff,&seqReadLine,
				    seqin->Text, &thys->TextPtr);


    ajFilebuffClear(buff, 0);

    ajStrTokenDel(&handle);
    ajStrDel(&token);
    ajStrDel(&tmpstr);
    ajStrDel(&tmpstr2);

    return ajTrue;
}




/* @funcstatic seqReadGff *****************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using GFF format.
**
** GFF only offers the sequence, and the type, with the DNA, RNA and
** Protein and End-xxx headers. GFF allows other header lines to be defined,
** so EMBOSS can add more lines for accession number and description
**
** GFF also defines Type and sequence-region headers, but they only
** provide information that is also in the DNA, RNA or Protein header
** and these are required for sequence storage so we ignore the alternatives.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadGff(AjPSeq thys, AjPSeqin seqin)
{

    ajuint bufflines         = 0;
    AjBool ok;
    AjBool isseq            = ajFalse;
    AjPFilebuff buff;
    AjPFilebuff ftfile   = NULL;
    AjBool dofeat        = ajFalse;
    AjPStr typstr = NULL;
    AjPStr verstr = NULL;	/* copy of version line */
    AjPStr outstr = NULL;	/* generated Type line */

    buff = seqin->Filebuff;

    if(!seqRegGffTyp)
	seqRegGffTyp = ajRegCompC("^##([DR]NA|Protein) +([^ \t\r\n]+)");

    if(!seqFtFmtGff)
	ajStrAssignC(&seqFtFmtGff, "gff");

    ok = ajBuffreadLineStore(buff, &seqReadLine,
			    seqin->Text, &thys->TextPtr);
    if(!ok)
	return ajFalse;

    bufflines++;

    ajDebug("seqReadGff first line '%S'\n", seqReadLine);

    if(!ajStrPrefixC(seqReadLine, "##gff-version "))
    {
	ajFilebuffReset(buff);
	return ajFalse;
    }
    ajStrAssignS(&verstr, seqReadLine);

    if(seqin->Text)
	ajStrAssignS(&thys->TextPtr,seqReadLine);

    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text, &thys->TextPtr);
    while(ok && ajStrPrefixC(seqReadLine, "##"))
    {
	if(ajRegExec(seqRegGffTyp, seqReadLine))
	{
	    isseq = ajTrue;
	    ajRegSubI(seqRegGffTyp, 1, &typstr);
	    ajRegSubI(seqRegGffTyp, 2, &thys->Name);
	    ajFmtPrintS(&outstr, "##Type %S %S", typstr, thys->Name);
	}
	else if(ajStrPrefixC(seqReadLine, "##end-"))
	    isseq = ajFalse;
	else if(isseq)
	    seqAppend(&thys->Seq, seqReadLine);

	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
    }

    if(!ajSeqGetLen(thys))
    {
	ajFilebuffReset(buff);
	return ajFalse;
    }

    /* do we want the features now? */

    if(ok & seqinUfoLocal(seqin))
    {
	dofeat = ajTrue;
	ftfile = ajFilebuffNewNofile();
	ajFilebuffLoadS(ftfile, verstr);
	ajFilebuffLoadS(ftfile, outstr);
	while(ok && !ajStrPrefixC(seqReadLine, "##"))
	{
	    ajFilebuffLoadS(ftfile, seqReadLine);
	    /* ajDebug("GFF FEAT saved line:\n%S", seqReadLine); */
	    ok = ajBuffreadLineStore(buff,&seqReadLine,seqin->Text,
				    &thys->TextPtr);
	}
    }

    if(dofeat)
    {
	ajFeattabInDel(&seqin->Ftquery);
	seqin->Ftquery = ajFeattabInNewSSF(seqFtFmtGff, thys->Name,
					   ajStrGetPtr(seqin->Type), ftfile);
	ajDebug("GFF FEAT TabIn %x type: '%S'\n",
                seqin->Ftquery, seqin->Type);
	ftfile = NULL;		  /* now copied to seqin->FeattabIn */
	ajFeattableDel(&seqin->Fttable);
	seqin->Fttable = ajFeatRead(seqin->Ftquery);
	/* ajFeattableTrace(seqin->Fttable); */
	ajFeattableDel(&thys->Fttable);
	thys->Fttable = seqin->Fttable;
	seqin->Fttable = NULL;
    }

    if(ajStrMatchC(typstr, "Protein"))
      ajSeqSetProt(thys);
    else if(ajSeqIsNuc(thys))
      ajSeqSetNuc(thys);
    else
      ajSeqSetProt(thys);


    ajFilebuffClear(buff, 0);

    ajStrDel(&typstr);
    ajStrDel(&verstr);
    ajStrDel(&outstr);

    return ajTrue;
}



/* @funcstatic seqReadGff3 ****************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using GFF3 format.
**
** GFF3 is far stricter than GFF2 but does include a sequence in FASTA format
**
** GFF also defines Type and sequence-region headers, but they only
** provide information that is also in the DNA, RNA or Protein header
** and these are required for sequence storage so we ignore the alternatives.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadGff3(AjPSeq thys, AjPSeqin seqin)
{

    ajuint bufflines         = 0;
    AjBool ok;
    AjPFilebuff buff;
    AjPFilebuff ftfile   = NULL;
    AjBool dofeat        = ajFalse;
    AjPStr verstr = NULL;	/* copy of version line */
    AjPStr outstr = NULL;	/* generated Type line */
    AjPStr typstr = NULL;

    buff = seqin->Filebuff;

    if(!seqFtFmtGff)
	ajStrAssignC(&seqFtFmtGff, "gff3");

    if(!seqRegGff3Typ)
	seqRegGff3Typ = ajRegCompC("^#([DR]NA|Protein) +([^ \t\r\n]+)");

    ok = ajBuffreadLineStore(buff, &seqReadLine,
			    seqin->Text, &thys->TextPtr);
    if(!ok)
	return ajFalse;

    bufflines++;

    ajDebug("seqReadGff3 first line '%S'\n", seqReadLine);

    ajStrRemoveWhiteExcess(&seqReadLine);
    if(!ajStrMatchC(seqReadLine, "##gff-version 3"))
    {
	ajDebug("bad gff3 version line '%S'\n", seqReadLine);
	ajFilebuffReset(buff);
	return ajFalse;
    }
    ajStrAssignS(&verstr, seqReadLine);

    if(seqin->Text)
	ajStrAssignS(&thys->TextPtr,seqReadLine);

    ok = ajBuffreadLineStore(buff, &seqReadLine, seqin->Text, &thys->TextPtr);
    while(ok && ajStrPrefixC(seqReadLine, "#"))
    {
	if(ajStrPrefixC(seqReadLine, "##sequence-region"))
	{
	}
	else if(ajStrPrefixC(seqReadLine, "##feature-ontology"))
	{
	}
	else if(ajStrPrefixC(seqReadLine, "##attribute-ontology"))
	{
	}
	else if(ajStrPrefixC(seqReadLine, "##source-ontology"))
	{
	}
	else if(ajStrPrefixC(seqReadLine, "###"))
	{
	}
	else if(ajStrPrefixC(seqReadLine, "##FASTA"))
	{
	    break;
	}
	else if(ajStrPrefixC(seqReadLine, "##"))
	{
	    ajWarn("GFF3: Unrecognized header directive '%S'",
		   seqReadLine);
	}
	if(ajRegExec(seqRegGff3Typ, seqReadLine))
	{
	    ajRegSubI(seqRegGff3Typ, 1, &typstr);
	    ajRegSubI(seqRegGff3Typ, 2, &thys->Name);
	    ajFmtPrintS(&outstr, "#!Type %S %S", typstr, thys->Name);
	}
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
    }

    /* do we want the features now? */

    if(ok & seqinUfoLocal(seqin))
    {
	dofeat = ajTrue;

	ftfile = ajFilebuffNewNofile();
	ajFilebuffLoadS(ftfile, verstr);
	ajFilebuffLoadS(ftfile, outstr);
    }

    while(ok && !ajStrPrefixC(seqReadLine, "##"))
    {
	if(dofeat)
	    ajFilebuffLoadS(ftfile, seqReadLine);
	/* ajDebug("GFF FEAT saved line:\n%S", seqReadLine); */
	ok = ajBuffreadLineStore(buff,&seqReadLine,seqin->Text,
				&thys->TextPtr);
    }

    if(!ajStrPrefixC(seqReadLine, "##FASTA")) /* no sequence at end */
    {
	ajFilebuffReset(buff);
	return ajFalse;
    }
	
    ok = ajBuffreadLineStore(buff, &seqReadLine,
			    seqin->Text, &thys->TextPtr);
    if(ok)
    {
	if(ajStrPrefixC(seqReadLine, ">"))
	{
	    ajStrCutStart(&seqReadLine, 1);
	    ajStrExtractFirst(seqReadLine, &thys->Desc, &thys->Name);
	}
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
    }
    while(ok && !ajStrPrefixC(seqReadLine, "##"))
    {
	seqAppend(&thys->Seq, seqReadLine);
	ok = ajBuffreadLineStore(buff, &seqReadLine,
				seqin->Text, &thys->TextPtr);
    }
    if(!ajSeqGetLen(thys))
    {
	ajFilebuffReset(buff);
	return ajFalse;
    }

    if(dofeat)
    {
	ajFeattabInDel(&seqin->Ftquery);
	seqin->Ftquery = ajFeattabInNewSSF(seqFtFmtGff, thys->Name,
					   ajStrGetPtr(seqin->Type), ftfile);
	ajDebug("GFF FEAT TabIn %x\n", seqin->Ftquery);
	ftfile = NULL;		  /* now copied to seqin->FeattabIn */
	ajFeattableDel(&seqin->Fttable);
	seqin->Fttable = ajFeatRead(seqin->Ftquery);
	/* ajFeattableTrace(seqin->Fttable); */
	ajFeattableDel(&thys->Fttable);
	thys->Fttable = seqin->Fttable;
	seqin->Fttable = NULL;
    }


    if(ajStrMatchC(typstr, "Protein"))
      ajSeqSetProt(thys);
    else if(ajSeqIsNuc(thys))
      ajSeqSetNuc(thys);
    else
      ajSeqSetProt(thys);

    ajFilebuffClear(buff, 0);

    ajStrDel(&typstr);
    ajStrDel(&verstr);
    ajStrDel(&outstr);

    return ajTrue;
}




/* @funcstatic seqReadAbi *****************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using ABI format.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [u] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadAbi(AjPSeq thys, AjPSeqin seqin)
{
    AjPFilebuff buff;
    AjBool  ok      = ajFalse;
    ajlong baseO    = 0L;
    ajlong numBases = 0L;
    AjPStr sample   = NULL;
    AjPStr smpl     = NULL;
    AjPFile fp;
    ajint filestat;

    buff = seqin->Filebuff;
    fp = ajFilebuffGetFile(buff);

    ajDebug("seqReadAbi file %F\n", fp);

    /* ajFilebuffTraceFull(buff, 10, 10); */

    if(ajFilebuffIsEnded(buff))
	return ajFalse;

    if(!ajSeqABITest(fp))
    {
        ajDebug("seqReadAbi ajSeqABITest failed on %F\n", fp);
	ajFilebuffResetPos(buff);
	return ajFalse;
    }

    if(seqin->Text)
	ajWarn("Failed to read text from binary ABI file %F", fp);

    filestat = ajFileSeek(fp,0L,0);
    ajDebug("filestat %d\n", filestat);

    numBases = ajSeqABIGetNBase(fp);
    /* Find BASE tag & get offset                    */
    baseO = ajSeqABIGetBaseOffset(fp);
    /* Read in sequence         */
    ok = ajSeqABIReadSeq(fp,baseO,numBases,&thys->Seq);
    if(!ok) {
	ajFileSeek(fp,filestat,0);
	ajFilebuffResetPos(buff);
	return ajFalse;
    }

    sample = ajStrNew();
    ajSeqABISampleName(fp, &sample);

    /* replace dots in the sample name with underscore */
    if(!seqRegAbiDots)
	seqRegAbiDots = ajRegCompC("^(.*)[.](.*)$");
    smpl = ajStrNew();

    while(ajRegExec(seqRegAbiDots,sample))
    {
	ajStrSetClear(&sample);
	ajRegSubI(seqRegAbiDots,1,&smpl);
	ajStrAppendC(&smpl,"_");
	ajStrAppendS(&sample,smpl);
	ajRegSubI(seqRegAbiDots,2,&smpl);
	ajStrAppendS(&sample,smpl);
    }

    ajStrAssignC(&thys->Name,ajStrGetPtr(sample));

    ajSeqSetNuc(thys);

    ajFilebuffClear(buff, -1);
    buff->File->End=ajTrue;

    ajStrDel(&smpl);
    ajStrDel(&sample);

    return ajTrue;
}




/* @func ajSeqPrintInFormat ***************************************************
**
** Reports the internal data structures
**
** @param [u] outf [AjPFile] Output file
** @param [r] full [AjBool] Full report (usually ajFalse)
** @return [void]
** @@
******************************************************************************/

void ajSeqPrintInFormat(AjPFile outf, AjBool full)
{
    ajuint i = 0;

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# sequence input formats\n");
    ajFmtPrintF(outf, "# Name  Format name (or alias)\n");
    ajFmtPrintF(outf, "# Alias Alias name\n");
    ajFmtPrintF(outf, "# Try   Test for unknown input files\n");
    ajFmtPrintF(outf, "# Nuc   Can read nucleotide input\n");
    ajFmtPrintF(outf, "# Pro   Can read protein input\n");
    ajFmtPrintF(outf, "# Feat  Can read feature annotation\n");
    ajFmtPrintF(outf, "# Gap   Can read gap characters\n");
    ajFmtPrintF(outf, "# Mset  Can read seqsetall (multiple seqsets)\n");
    ajFmtPrintF(outf, "# Name         Alias Try  Nuc  Pro Feat  Gap MSet "
		"Description");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "InFormat {\n");
    for(i=0; seqInFormatDef[i].Name; i++)
    {
	if(full || !seqInFormatDef[i].Alias)
	    ajFmtPrintF(outf,
			"  %-12s %5B %3B  %3B  %3B  %3B  %3B  %3B \"%s\"\n",
			seqInFormatDef[i].Name,
			seqInFormatDef[i].Alias,
			seqInFormatDef[i].Try,
			seqInFormatDef[i].Nucleotide,
			seqInFormatDef[i].Protein,
			seqInFormatDef[i].Feature,
			seqInFormatDef[i].Gap,
			seqInFormatDef[i].Multiset,
			seqInFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @funcstatic seqFindInFormat ************************************************
**
** Looks for the specified format(s) in the internal definitions and
** returns the index.
**
** Sets iformat as the recognized format, and returns ajTrue.
**
** @param [r] format [const AjPStr] Format required.
** @param [w] iformat [ajint*] Index
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqFindInFormat(const AjPStr format, ajint* iformat)
{
    AjPStr tmpformat = NULL;
    ajuint i = 0;

    /* ajDebug("seqFindInFormat '%S'\n", format); */
    if(!ajStrGetLen(format))
	return ajFalse;

    ajStrAssignS(&tmpformat, format);
    ajStrFmtLower(&tmpformat);

    for(i=0; seqInFormatDef[i].Name; i++)
    {
	/* ajDebug("test %d '%s' \n", i, seqInFormatDef[i].Name); */
	if(ajStrMatchCaseC(tmpformat, seqInFormatDef[i].Name))
	{
	    *iformat = i;
	    ajStrDel(&tmpformat);
	    /* ajDebug("found '%s' at %d\n", seqInFormatDef[i].Name, i); */
	    return ajTrue;
	}
    }

    ajErr("Unknown input format '%S'", format);

    ajStrDel(&tmpformat);

    return ajFalse;
}




/* @func ajSeqFormatTest ******************************************************
**
** tests whether a named format is known
**
** @param [r] format [const AjPStr] Format
** @return [AjBool] ajTrue if formats was accepted
** @@
******************************************************************************/

AjBool ajSeqFormatTest(const AjPStr format)
{
    ajuint i;

    for(i=0; seqInFormatDef[i].Name; i++)
	if(ajStrMatchCaseC(format, seqInFormatDef[i].Name))
	    return ajTrue;

    return ajFalse;
}




/* @funcstatic seqSetInFormat *************************************************
**
** Steps through a list of default formats, setting the Try value for
** each known format to ajTrue if it is in the list, and ajFalse
** if not.
**
** @param [r] format [const AjPStr] Format list, punctuated by whitespace
**                                  or commas
** @return [AjBool] ajTrue if all formats were accepted
** @@
******************************************************************************/

static AjBool seqSetInFormat(const AjPStr format)
{
    ajuint i;
    AjPStr fmtstr     = NULL;
    AjPStrTok handle  = NULL;
    ajuint ifound;
    AjBool ret        = ajTrue;

    for(i=0; seqInFormatDef[i].Name; i++)
	seqInFormatDef[i].Try = ajFalse;

    ajDebug("seqSetInformat '%S'\n", format);

    ajStrTokenAssignC(&handle, format, " \t\n\r,;:");
    while(ajStrTokenNextParseC(&handle, " \t\n\r,;:", &fmtstr))
    {
	ifound = 0;
	for(i=0; seqInFormatDef[i].Name; i++)
	    if(ajStrMatchCaseC(fmtstr, seqInFormatDef[i].Name))
	    {
		/* ajDebug("found '%S' %d\n", fmtstr, i); */
		seqInFormatDef[i].Try = ajTrue;
		ifound = 1;
		break;
	    }

	if(!ifound)
	{
	    /* ajDebug("not found '%S'\n", fmtstr); */

	    ajErr("Input format '%S' not known", fmtstr);
	    ret = ajFalse;
	}
    }

    ajStrTokenDel(&handle);

    return ret;
}




/* @funcstatic seqAppend ******************************************************
**
** Appends sequence characters in the input line to a growing sequence.
** Non sequence characters are simply ignored.
**
** @param [u] pseq [AjPStr*] Sequence as a string
** @param [r] line [const AjPStr] Input line.
** @return [ajuint] Sequence length to date.
** @@
******************************************************************************/

static ajuint seqAppend(AjPStr* pseq, const AjPStr line)
{
    AjPStr tmpstr = NULL;
    ajuint ret = 0;

    ajStrAssignS(&tmpstr, line);
    ajStrKeepSetAlphaC(&tmpstr, "*.~?#+-");
    ajStrAppendS(pseq, tmpstr);

    ret = ajStrGetLen(*pseq);
    ajStrDel(&tmpstr);

    return ret;
}




/* @funcstatic seqAppendK *****************************************************
**
** Appends single sequence character in the input line to a growing sequence.
** Non sequence characters are simply ignored.
**
** @param [u] pseq [AjPStr*] Sequence as a string
** @param [r] ch [char] Input character.
** @return [ajuint] Sequence length to date.
** @@
******************************************************************************/

static ajuint seqAppendK(AjPStr* pseq, char ch)
{
    AjPStr tmpstr = NULL;
    ajuint ret = 0;

    ajStrAssignK(&tmpstr, ch);
    ajStrKeepSetAlphaC(&tmpstr, "*.~?#+-");
    ajStrAppendS(pseq, tmpstr);

    ret = ajStrGetLen(*pseq);
    ajStrDel(&tmpstr);

    return ret;
}




/* @funcstatic seqAppendCommented *********************************************
**
** Appends sequence characters in the input line to a growing sequence.
** Non sequence characters are simply ignored.
**
** This version of seqAppend removes comments in the angle brackets style
** used first by Staden and then later by GCG.
**
** @param [u] pseq [AjPStr*] Sequence as a string
** @param [u] incomment [AjBool*] Currently processing a comment
** @param [r] line [const AjPStr] Input line.
** @return [ajuint] Sequence length to date.
** @@
******************************************************************************/

static ajuint seqAppendCommented(AjPStr* pseq, AjBool* incomment,
				const AjPStr line)
{
    AjPStr tmpstr = NULL;
    ajint i;
    ajuint ret = 0;

    ajStrAssignS(&tmpstr, line);
    ajStrKeepSetAlphaC(&tmpstr, "*.~?#+-<>");

    ajDebug("seqAppendCommented %B '%S'\n", *incomment, tmpstr);
    while(ajStrGetLen(tmpstr))
    {
	/* if we are in a comment, look for the end of it */
	/* Staden comments are <comment> */
	/* GCG comments are <comment< or >comment> */

	/* there should be no case of >comment< 
	   but in a broken file we can't tell */

	/* so we test for both kinds of angle brackets at both ends */

	if(*incomment)
	{
	    i = ajStrFindAnyC(tmpstr, "<>");
	    if(i >= 0)			/* comment ends in this line */
	    {
		ajStrCutStart(&tmpstr, i+1);
		*incomment = ajFalse;
	    }
	    else
	    {
		ajStrAssignClear(&tmpstr);	/* all comment */
	    }
	}
	else
	{
	    i = ajStrFindAnyC(tmpstr, "<>");
	    if(i >= 0)			/* comment starts in this line */
	    {
		if(i)
		    ajStrAppendSubS(pseq, tmpstr, 0, i-1);
		ajDebug("before comment saved '%S'\n", *pseq);
		ajStrCutStart(&tmpstr, i+1);
		*incomment = ajTrue;
	    }
	    else
	    {
		ajStrAppendS(pseq, tmpstr);
		ajDebug("all saved '%S'\n", *pseq);
		ajStrAssignClear(&tmpstr);
	    }
	}
	if(ajStrGetLen(tmpstr))
	    ajDebug("continuing %B '%S'\n", *incomment, tmpstr);
	else
	    ajDebug("done %B '%S'\n", *incomment, tmpstr);
    }

    ret = ajStrGetLen(*pseq);
    ajStrDel(&tmpstr);

    return ret;
}

/* @funcstatic seqAppendWarn ***************************************************
**
** Appends sequence characters in the input line to a growing sequence.
**
** Non sequence characters are reported in the return value
** if EMBOSS_SEQWARN is set
**
** @param [u] pseq [AjPStr*] Sequence as a string
** @param [r] line [const AjPStr] Input line.
** @return [const AjPStr] Any rejected non-space characters
** @@
******************************************************************************/

static const AjPStr seqAppendWarn(AjPStr* pseq, const AjPStr line)
{
    AjPStr tmpstr = NULL;

    if(!seqAppendRestStr) {
	if(ajNamGetValueC("seqwarn", &tmpstr))
	    ajStrToBool(tmpstr, &seqDoWarnAppend);
    }

    ajStrAssignS(&tmpstr, line);

    if(seqDoWarnAppend)
    {
	ajStrKeepSetAlphaRestC(&tmpstr, "*.~?#+-", &seqAppendRestStr);
	ajStrAppendS(pseq, tmpstr);

	ajStrDel(&tmpstr);

	if(!ajStrGetLen(seqAppendRestStr))
	    return NULL;

	return seqAppendRestStr;
    }

    ajStrKeepSetAlphaC(&tmpstr, "*.~?#+-");
    ajStrAppendS(pseq, tmpstr);

    ajStrDel(&tmpstr);

    return NULL;
}




/* @funcstatic seqGcgRegInit **************************************************
**
** Initialises regular expressions for GCG and MSF format parsing
**
**
** @return [void]
******************************************************************************/

static void seqGcgRegInit(void)
{
    if(!seqRegGcgDot)
	seqRegGcgDot = ajRegCompC("[.][.]");

    if(!seqRegGcgChk)
	seqRegGcgChk = ajRegCompC("[Cc][Hh][Ee][Cc][Kk]:[ \t]*([0-9]+)");

    if(!seqRegGcgLen)
	seqRegGcgLen = ajRegCompC("[Ll][Ee][Nn][Gg][Tt][Hh]:[ \t]*([0-9]+)");

    if(!seqRegGcgTyp)
	seqRegGcgTyp = ajRegCompC("[Tt][Yy][Pp][Ee]:[ \t]*([NP])");

    if(!seqRegGcgNam)
	seqRegGcgNam = ajRegCompC("[^ \t>]+");

    if(!seqRegGcgMsf)
	seqRegGcgMsf = ajRegCompC("[Mm][Ss][Ff]:[ \t]*([0-9]+)");

    if(!seqRegGcgMsflen)
	seqRegGcgMsflen = ajRegCompC("[Ll][Ee][Nn]:[ \t]*([0-9]+)");

    if(!seqRegGcgWgt)
	seqRegGcgWgt = ajRegCompC("[Ww][Ee][Ii][Gg][Hh][Tt]:[ \t]*([0-9.]+)");

    if(!seqRegGcgMsfnam)
	seqRegGcgMsfnam = ajRegCompC("[Nn][Aa][Mm][Ee]:[ \t]*([^ \t]+)");

    return;
}

/* @funcstatic seqGcgDots *****************************************************
**
** Looks for the ".." line in the header of a GCG format sequence.
** Care is needed to make sure this is not an MSF header which
** has a very similar format.
**
** Data found on the header line is extracted and returned.
**
** The number of lines searched is limited to avoid parsing large data
** files that are not in GCG format. The user should set this limit to
** be large enough to handle large EMBL/Genbank annotations
**
** @param [u] thys [AjPSeq] Sequence.
** @param [r] seqin [const AjPSeqin] Sequence input.
** @param [u] pline [AjPStr*] Input buffer.
** @param [r] maxlines [ajuint] Maximum number of lines to read
**                              before giving up
** @param [w] len [ajuint*] Length of sequence read.
** @return [AjBool] ajTrue on success. ajFalse on failure or aborting.
** @@
******************************************************************************/

static AjBool seqGcgDots(AjPSeq thys, const  AjPSeqin seqin,
			 AjPStr* pline,
			 ajuint maxlines, ajuint* len)
{
    AjPStr token  = NULL;
    ajuint check  = 0;
    ajuint nlines = 0;

    AjPFilebuff buff;

    buff = seqin->Filebuff;

    seqGcgRegInit();

    while(nlines < maxlines)
    {
	if(nlines++)
	    if(!ajBuffreadLineStore(buff, pline,
				   seqin->Text, &thys->TextPtr))
		return ajFalse;

	if(nlines > maxlines)
	    return ajFalse;

	if(!ajRegExec(seqRegGcgDot, *pline))
	    continue;

	ajDebug("seqGcgDots   .. found\n'%S'\n", *pline);
	if(!ajRegExec(seqRegGcgChk, *pline))	/* checksum required */
	    return ajFalse;

	if(ajRegExec(seqRegGcgMsf, *pline))	/* oops - it's an MSF file */
	    return ajFalse;

	ajRegSubI(seqRegGcgChk, 1, &token);
	ajStrToUint(token, &check);

	ajDebug("   checksum %d\n", check);

	if(ajRegExec(seqRegGcgLen, *pline))
	{
	    ajRegSubI(seqRegGcgLen, 1, &token);
	    ajStrToUint(token, len);
	    ajDebug("   length %d\n", *len);
	}

	if(ajRegExec(seqRegGcgNam, *pline))
	{
	    ajRegSubI(seqRegGcgNam, 0, &thys->Name);
	    ajDebug("   name '%S'\n", thys->Name);
	}

	if(ajRegExec(seqRegGcgTyp, *pline))
	{
	    ajRegSubI(seqRegGcgTyp, 1, &thys->Type);
	    ajDebug("   type '%S'\n", thys->Type);
	}

	ajStrDel(&token);
	return ajTrue;
    }

    return ajFalse;
}




/* @funcstatic seqGcgMsfDots **************************************************
**
** Looks for the ".." line in the header of an MSF format sequence.
** Care is needed to make sure this is not a simple GCG header which
** has a very similar format.
**
** Data found on the header line is extracted and returned.
**
** The number of lines searched is limited to avoid parsing large data
** files that are not in GCG format. The user should set this limit to
** be large enough to handle large EMBL/Genbank annotations
**
** @param [u] thys [AjPSeq] Sequence.
** @param [r] seqin [const AjPSeqin] Sequence input.
** @param [u] pline [AjPStr*] Input buffer.
** @param [r] maxlines [ajuint] Maximum number of lines to read
**                              before giving up
** @param [w] len [ajuint*] Length of sequence read.
** @return [AjBool] ajTrue on success. ajFalse on failure or aborting.
** @@
******************************************************************************/

static AjBool seqGcgMsfDots(AjPSeq thys, const AjPSeqin seqin, AjPStr* pline,
			    ajuint maxlines, ajuint* len)
{
    AjPStr token = NULL;
    ajuint check  = 0;
    ajuint nlines = 0;

    AjPFilebuff buff;

    buff = seqin->Filebuff;

    ajDebug("seqGcgMsfDots maxlines: %d\nline: '%S'\n", maxlines,*pline);

    seqGcgRegInit();

    while(nlines < maxlines)
    {
	if(nlines++)
	    if(!ajBuffreadLineStore(buff, pline,
				   seqin->Text, &thys->TextPtr))
		return ajFalse;

	ajDebug("testing line %d\n'%S'\n", nlines,*pline);
	if(nlines > maxlines)
	    return ajFalse;

	if(!ajRegExec(seqRegGcgDot, *pline))
	    continue;

	/* dots found. This must be the line if this is MSF format */

	if(!ajRegExec(seqRegGcgChk, *pline))	/* check: is required */
	    return ajFalse;

	if(!ajRegExec(seqRegGcgMsf, *pline)) /* MSF: len required for GCG*/
	    return ajFalse;


	ajRegSubI(seqRegGcgMsf, 1, &token);
	ajStrToUint(token, len);

	ajRegSubI(seqRegGcgChk, 1, &token);
	ajStrToUint(token, &check);

	if(ajRegExec(seqRegGcgNam, *pline))
	    ajRegSubI(seqRegGcgNam, 0, &thys->Name);

	if(ajRegExec(seqRegGcgTyp, *pline))
	    ajRegSubI(seqRegGcgTyp, 1, &thys->Type);

	ajStrDel(&token);
	ajDebug("seqGcgMsfDots '%S' '%S' len: %d check: %d\n",
		thys->Name, thys->Type, *len, check);

	return ajTrue;
    }

    return ajFalse;
}




/* @funcstatic seqGcgMsfHeader ************************************************
**
** Parses data from a line of an MSF file header. The header stores
** names and other data for all sequences in the file. Each file
** is definied on a separate line. The results are stored
** in the MSF internal table. The sequence data is read later in the
** input file and added to the table.
**
** @param [r] line [const AjPStr] Input line.
** @param [u] pmsfitem [SeqPMsfItem*] MSF internal table item.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqGcgMsfHeader(const AjPStr line, SeqPMsfItem* pmsfitem)
{
    AjPStr name         = NULL;	/* NOTE: not static. New each time for list */
    AjPStr token = NULL;
    SeqPMsfItem msfitem = NULL;

    ajDebug("seqGcgMsfHeader '%S'\n", line);

    if(!ajRegExec(seqRegGcgMsfnam, line))
	return ajFalse;

    ajRegSubI(seqRegGcgMsfnam, 1, &name);
    /*ajDebug("Name found\n");*/

    if(!ajRegExec(seqRegGcgChk, line))
	return ajFalse;

    /*ajDebug("Check found\n");*/

    *pmsfitem = AJNEW0(msfitem);
    msfitem->Name = name;

    ajRegSubI(seqRegGcgChk, 1, &token);
    ajStrToUint(token, &msfitem->Check);

    if(ajRegExec(seqRegGcgMsflen, line))
    {
	ajRegSubI(seqRegGcgMsflen, 1, &token);
	ajStrToUint(token, &msfitem->Len);
    }
    else
	msfitem->Len = 0;

    msfitem->Seq = ajStrNewRes(msfitem->Len+1);

    if(ajRegExec(seqRegGcgWgt, line))
    {
	ajRegSubI(seqRegGcgWgt, 1, &token);
	ajStrToFloat(token, &msfitem->Weight);
    }
    else
	msfitem->Weight = 1.0;

    ajDebug("MSF header name '%S' check %d len %d weight %.3f\n",
	    msfitem->Name, msfitem->Check, msfitem->Len, msfitem->Weight);

    ajStrDel(&token);

    return ajTrue;
}




/* @funcstatic seqUsaProcess **************************************************
**
** Converts a USA Universal Sequence Address into an open file.
**
** First tests for "[n:n:r]" range and sets this if it is found
**
** Then tests for asis:: in which the "filename" is really the sequence
** and no format is needed.
**
** Then tests for "format::" and sets this if it is found
**
** Then tests for "list:" or "@" and processes as a list file
** using seqListProcess which in turn invokes seqUsaProcess
** until a valid USA is found.
**
** Then tests for dbname:query and opens the file (at the correct position
** if the database definition defines it)
**
** If there is no database, looks for file:query and opens the file.
** In this case the file position is not known and sequence reading
** will have to scan for the entry/entries we need.
**
** @param [u] thys [AjPSeq] Sequence to be read.
** @param [u] seqin [AjPSeqin] Sequence input structure.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqUsaProcess(AjPSeq thys, AjPSeqin seqin)
{
    AjPStr tmpstr  = NULL;

    AjPSeqQuery qry;

    AjBool fmtstat   = ajFalse;	 /* status returns from regex tests */
    AjBool regstat   = ajFalse;
    AjBool dbstat    = ajFalse;
    AjBool accstat   = ajFalse;	/* return status from reading something */
    AjBool liststat  = ajFalse;
    AjBool asisstat  = ajFalse;
    AjBool rangestat = ajFalse;
#ifdef __CYGWIN__
    AjPStr usatmp    = NULL;
#endif

    qry = seqin->Query;

    ajStrDel(&qry->Field);    /* clear it. we test this for regstat */

    ajDebug("++seqUsaProcess '%S' %d..%d(%b) '%S' %d \n",
	    seqin->Usa, seqin->Begin, seqin->End, seqin->Rev,
	    seqin->Formatstr, seqin->Format);

    if(!seqRegUsaFmt)
	seqRegUsaFmt = ajRegCompC("^([A-Za-z0-9]*)::(.*)$");
    /* \1 format letters and numbers only */
    /* \2 remainder (filename, etc.)*/

    if(!seqRegUsaDb)
	seqRegUsaDb = ajRegCompC("^([A-Za-z][A-Za-z0-9_]+)([-]([A-Za-z]+))?"
			   "([:{]([^}]*)}?)?$");

    /* \1 dbname (start with a letter, then alphanumeric) */
    /* \2 -id or -acc etc. */
    /* \3 qry->Field (id or acc etc.) */
    /* \4 :qry->QryString */
    /* \5 qry->QryString */

    if(!seqRegUsaId)		 /* \1 is filename \4 is the qry->QryString */
#ifndef WIN32
	seqRegUsaId = ajRegCompC("^([^|]+[|]|[^:{%]+)"
			   "(([:{%])(([^:}]+):)?([^:}]*)}?)?$");
#else
	/* Windows file names can start with e.g.: 'C:\' */
	/* But allow e.g. 'C:/...', for Staden spin */
	seqRegUsaId = ajRegCompC ("^(([a-zA-Z]:[\\\\/])?[^:{%]+)"
				  "(([:{%])(([^:}]+):)?([^:}]*)}?)?$");
#endif


    if(!seqRegUsaList)	 /* \1 is filename \3 is the qry->QryString */
	seqRegUsaList = ajRegCompC("^(@|[Ll][Ii][Ss][Tt]:+)(.+)$");

    if(!seqRegUsaAsis)	 /* \1 is filename \3 is the qry->QryString */
	seqRegUsaAsis = ajRegCompC("^[Aa][Ss][Ii][Ss]:+(.+)$");

    if(!seqRegUsaWild)
	seqRegUsaWild = ajRegCompC("(.*[*].*)");
    /* \1 wildcard query */

    if(!seqRegUsaRange)    /* \1 is rest of USA \2 start \3 end \5 reverse*/
	seqRegUsaRange = ajRegCompC("(.*)[[](-?[0-9]*):(-?[0-9]*)(:([Rr])?)?[]]$");

    ajStrAssignS(&seqUsaTest, seqin->Usa);
    /* Strip any leading spaces */
    ajStrTrimC(&seqUsaTest," \t\n");

#ifdef __CYGWIN__
    if(*(ajStrGetPtr(seqUsaTest)+1)==':')
    {
	usatmp = ajStrNew();
        ajFmtPrintS(&usatmp,"/cygdrive/%c/%s",*ajStrGetPtr(seqUsaTest),
		    ajStrGetPtr(seqUsaTest)+2);
        ajStrAssignRef(&seqUsaTest,usatmp);
        ajStrDel(&usatmp);
    }
#endif

    ajDebug("USA to test: '%S'\n\n", seqUsaTest);

    rangestat = ajRegExec(seqRegUsaRange, seqUsaTest);
    if(rangestat)
    {
	ajRegSubI(seqRegUsaRange, 2, &tmpstr);
	if(ajStrGetLen(tmpstr))
	    ajStrToInt(tmpstr, &seqin->Begin);
	ajRegSubI(seqRegUsaRange, 3, &tmpstr);
	if(ajStrGetLen(tmpstr))
	    ajStrToInt(tmpstr, &seqin->End);
	ajRegSubI(seqRegUsaRange, 5, &tmpstr);
	if(ajStrGetLen(tmpstr))
	    seqin->Rev = ajTrue;
	ajStrDel(&tmpstr);
	ajRegSubI(seqRegUsaRange, 1, &seqUsaTest);
	ajDebug("range found [%d:%d:%b]\n",
		seqin->Begin, seqin->End, seqin->Rev);
    }

    asisstat = ajRegExec(seqRegUsaAsis, seqUsaTest);
    if(asisstat)
    {
	ajRegSubI(seqRegUsaAsis, 1, &qry->Filename);
	ajStrAssignC(&qry->Formatstr, "text");
	ajStrAssignS(&seqin->Formatstr, qry->Formatstr);
	seqFormatSet(thys, seqin);
	ajDebug("asis sequence '%S'\n", qry->Filename);
	return ajSeqAccessAsis(seqin);
    }

    liststat = ajRegExec(seqRegUsaList, seqUsaTest);
    fmtstat = ajRegExec(seqRegUsaFmt, seqUsaTest);
    ajDebug("format regexp: %B list:%B\n", fmtstat, liststat);

    if(fmtstat && !liststat)
    {
	ajRegSubI(seqRegUsaFmt, 1, &qry->Formatstr);
	/* default unknown */
	ajStrAssignEmptyC(&qry->Formatstr, seqInFormatDef[0].Name);
	ajRegSubI(seqRegUsaFmt, 2, &seqUsaTest);
	ajDebug("found format %S\n", qry->Formatstr);
	if(seqFindInFormat(qry->Formatstr, &seqin->Format))
	    ajStrAssignS(&seqin->Formatstr, qry->Formatstr);
	else
	    ajDebug("unknown format '%S'\n", qry->Formatstr);
    }
    else
	ajDebug("no format specified in USA\n");

    ajDebug("\n");

    seqFormatSet(thys, seqin);

    liststat = ajRegExec(seqRegUsaList, seqUsaTest);
    if(liststat)
    {
	ajRegSubI(seqRegUsaList, 2, &seqQryList);
	ajDebug("list found @%S fmt:%B range:%B\n",
		seqQryList, fmtstat, rangestat);
	if(seqin->Count && fmtstat)
	    ajWarn("List includes another list and format. "
		   "Results undefined\n");
	if(seqin->Count && rangestat)
	    ajWarn("List includes another list with range. "
		   "Results undefined\n");
	return seqListProcess(thys, seqin, seqQryList);
    }

    regstat = ajRegExec(seqRegUsaDb, seqUsaTest);
    ajDebug("dbname dbexp: %B\n", regstat);

    if(regstat)
    {
	/* clear it if this was really a file */	
	ajRegSubI(seqRegUsaDb, 3, &qry->Field);
	ajRegSubI(seqRegUsaDb, 1, &seqQryDb);
	if(!ajNamDatabase(seqQryDb))
	{
	    ajDebug("unknown dbname %S, try filename\n", seqQryDb);
	    regstat = ajFalse;
	}
    }

    if(regstat)
    {
	ajRegSubI(seqRegUsaDb, 5, &qry->QryString);
	ajStrAssignS(&qry->DbName, seqQryDb);
	ajDebug("found dbname '%S' level: '%S' qry->QryString: '%S'\n",
		qry->DbName, qry->Field, qry->QryString);
	dbstat = ajNamDbData(qry);

	if(dbstat && ajStrGetLen(qry->QryString))
	{
	    /* ajDebug("  qry->QryString %S\n", qry->QryString); */
	    if(ajStrGetLen(qry->Field))
	    {
		ajDebug("  db QryString '%S' Field '%S'\n",
			qry->QryString, qry->Field);
		if(ajStrMatchCaseC(qry->Field, "id"))
		    ajStrAssignS(&qry->Id, qry->QryString);
		else if(qry->HasAcc && ajStrMatchCaseC(qry->Field, "acc"))
		    ajStrAssignS(&qry->Acc, qry->QryString);
		else
		{
		    if(!seqQueryField(qry, qry->Field))
		    {
			ajErr("USA '%S' query field '%S' not defined"
			      " for database '%S'",
			      seqUsaTest, qry->Field, qry->DbName);
			return ajFalse;
		    }

		    if(ajStrMatchCaseC(qry->Field, "sv"))
			ajStrAssignS(&qry->Sv, qry->QryString);
		    else if(ajStrMatchCaseC(qry->Field, "gi"))
			ajStrAssignS(&qry->Gi, qry->QryString);
		    else if(ajStrMatchCaseC(qry->Field, "des"))
			ajStrAssignS(&qry->Des, qry->QryString);
		    else if(ajStrMatchCaseC(qry->Field, "org"))
			ajStrAssignS(&qry->Org, qry->QryString);
		    else if(ajStrMatchCaseC(qry->Field, "key"))
			ajStrAssignS(&qry->Key, qry->QryString);
		    else
		    {
			ajErr("USA '%S' query level '%S' not supported",
			      seqUsaTest, qry->Field);
			return ajFalse;
		    }
		}
	    }
	    else
	    {
		ajStrAssignS(&qry->Id, qry->QryString);
		if(qry->HasAcc)
		    ajStrAssignS(&qry->Acc, qry->QryString);
		if(seqQueryFieldC(qry, "sv"))
		    ajStrAssignS(&qry->Sv, qry->QryString);
	    }
	}
	ajSeqQueryStarclear(qry);
	dbstat = ajNamDbQuery(qry);
	if(dbstat)
	{
	    ajDebug("database type: '%S' format '%S'\n",
		    qry->DbType, qry->Formatstr);
	    if(seqFindInFormat(qry->Formatstr, &seqin->Format))
		ajStrAssignS(&seqin->Formatstr, qry->Formatstr);
	    else
		ajDebug("unknown format '%S'\n", qry->Formatstr);

	    ajDebug("use access method '%S'\n", qry->Method);
	    qry->Access = ajSeqMethod(qry->Method);
	    if(!qry->Access)
	    {
		ajErr("Access method '%S' unknown", qry->Method);
		return ajFalse;
	    }
	    else
	    {
		/* ajDebug("trying access method '%S'\n", qry->Method); */

		/* Calling funclist seqAccess() */
		accstat = qry->Access->Access(seqin);
		if(accstat)
		    return ajTrue;

		ajDebug("Database '%S' : access method '%s' failed\n",
			qry->DbName, qry->Access->Name);
		return ajFalse;
	    }
	}
	else
	{
	    ajErr("no access method available for '%S'", seqUsaTest);
	    return ajFalse;
	}
    }
    else
	ajDebug("no dbname specified\n");

    ajDebug("\n");

    /* no database name, try filename */

    if(!dbstat)
    {
	regstat = ajRegExec(seqRegUsaId, seqUsaTest);
	ajDebug("entry-id regexp: %B\n", regstat);

	if(regstat)
	{
#ifndef WIN32
	    ajRegSubI(seqRegUsaId, 1, &qry->Filename);
	    ajRegSubI(seqRegUsaId, 3, &seqQryChr);
	    ajRegSubI(seqRegUsaId, 5, &qry->Field);
	    ajRegSubI(seqRegUsaId, 6, &qry->QryString);
#else
	    ajRegSubI (seqRegUsaId, 1, &qry->Filename);
	    ajRegSubI (seqRegUsaId, 4, &seqQryChr);
	    ajRegSubI (seqRegUsaId, 6, &qry->Field);
	    ajRegSubI (seqRegUsaId, 7, &qry->QryString);
#endif
	    ajDebug("found filename %S\n", qry->Filename);
	    if(ajStrMatchC(seqQryChr, "%")) {
		ajStrToLong(qry->QryString, &qry->Fpos);
		accstat = ajSeqAccessOffset(seqin);
		if(accstat)
		    return ajTrue;
	    }
	    else
	    {
		if(ajStrGetLen(qry->QryString))
		{
		    ajDebug("file QryString '%S' Field '%S' hasAcc:%B seqQryChr '%S'\n",
			    qry->QryString, qry->Field, qry->HasAcc, seqQryChr);
		    if(ajStrGetLen(qry->Field)) /* set by dbname above */
		    {
			/* ajDebug("    qry->Field %S\n", qry->Field); */
			if(ajStrMatchCaseC(qry->Field, "id"))
			    ajStrAssignS(&qry->Id, qry->QryString);
			else if(qry->HasAcc &&
				ajStrMatchCaseC(qry->Field, "acc"))
			    ajStrAssignS(&qry->Acc, qry->QryString);
			else if(ajStrMatchCaseC(qry->Field, "sv"))
			    ajStrAssignS(&qry->Sv, qry->QryString);
			else if(ajStrMatchCaseC(qry->Field, "gi"))
			    ajStrAssignS(&qry->Gi, qry->QryString);
			else if(ajStrMatchCaseC(qry->Field, "des"))
			    ajStrAssignS(&qry->Des, qry->QryString);
			else if(ajStrMatchCaseC(qry->Field, "org"))
			    ajStrAssignS(&qry->Org, qry->QryString);
			else if(ajStrMatchCaseC(qry->Field, "key"))
			    ajStrAssignS(&qry->Key, qry->QryString);
			else  /* assume it was part of the filename */
			{
			    ajErr("Unknown query field '%S' in USA '%S'",
				  qry->Field, seqUsaTest);
			    return ajFalse;
			}
		    }
		    else
		    {
			ajStrAssignS(&qry->Id, qry->QryString);
			if(qry->HasAcc)
			    ajStrAssignS(&qry->Acc, qry->QryString);
		    }
		}
		accstat = ajSeqAccessFile(seqin);
		if(accstat)
		    return ajTrue;
	    }
	    ajErr("Failed to open filename '%S'", qry->Filename);
	    return ajFalse;
	}
	else			  /* dbstat and regstat both failed */
	    ajDebug("no filename specified\n");

	ajDebug("\n");
    }

    return accstat;
}




/* @funcstatic seqQueryField **************************************************
**
** Checks whether a query field is defined for a database as a "fields:"
** string in the database definition.
**
** @param [r] qry [const AjPSeqQuery] Sequence query object
** @param [r] field [const AjPStr] field name
** @return [AjBool] ajTrue if the field is defined
******************************************************************************/

static AjBool seqQueryField(const AjPSeqQuery qry, const AjPStr field)
{

    return seqQueryFieldC(qry, ajStrGetPtr(field));
}




/* @funcstatic seqQueryFieldC *************************************************
**
** Checks whether a query field is defined for a database as a "fields:"
** string in the database definition.
**
** @param [r] qry [const AjPSeqQuery] Sequence query object
** @param [r] field [const char*] field name
** @return [AjBool] ajTrue if the field is defined
******************************************************************************/

static AjBool seqQueryFieldC(const AjPSeqQuery qry, const char* field)
{

    AjPStrTok handle = NULL;
    AjPStr token     = NULL;

    ajDebug("seqQueryFieldC usa '%s' fields '%S'\n", field, qry->DbFields);
    ajStrTokenAssignC(&handle, qry->DbFields, "\t ,;\n\r");
    while(ajStrTokenNextParse(&handle, &token))
    {
	ajDebug("seqQueryField test '%S'\n", token);
	if(ajStrMatchCaseC(token, field))
	{
	    ajDebug("seqQueryField match '%S'\n", token);
	    ajStrTokenDel(&handle);
	    ajStrDel(&token);
	    return ajTrue;
	}
    }

    ajStrTokenDel(&handle);
    ajStrDel(&token);
    return ajFalse;
}




/* @funcstatic seqUsaRestore **************************************************
**
** Restores a sequence input specification from a SeqPListUsa node
**
** @param [w] seqin [AjPSeqin] Sequence input object
** @param [r] node [const SeqPListUsa] Usa list node
** @return [void]
******************************************************************************/

static void seqUsaRestore(AjPSeqin seqin, const SeqPListUsa node)
{

    seqin->Begin    = node->Begin;
    seqin->End      = node->End;
    seqin->Rev      = node->Rev;
    seqin->Format   = node->Format;
    seqin->Features = node->Features;
    ajStrAssignS(&seqin->Formatstr, node->Formatstr);

    return;
}




/* @funcstatic seqUsaSave *****************************************************
**
** Saves a sequence input specification in a SeqPListUsa node
**
** @param [w] node [SeqPListUsa] Usa list node
** @param [r] seqin [const AjPSeqin] Sequence input object
** @return [void]
******************************************************************************/

static void seqUsaSave(SeqPListUsa node, const AjPSeqin seqin)
{
    node->Begin    = seqin->Begin;
    node->End      = seqin->End;
    node->Rev      = seqin->Rev;
    node->Format   = seqin->Format;
    node->Features = seqin->Features;
    ajStrAssignS(&node->Formatstr, seqin->Formatstr);

    return;
}




/* @funcstatic seqUsaListTrace ************************************************
**
** Traces the nodes in a USA list
**
** @param [r] list [const AjPList] The USA list
** @return [void]
******************************************************************************/

static void seqUsaListTrace(const AjPList list)
{
    AjIList iter;
    SeqPListUsa node;
    ajuint i = 0;

    iter = ajListIterNewread(list);

    ajDebug("SeqUsaListTrace %d nodes\n", ajListGetLength(list));
    while(!ajListIterDone(iter))
    {
	node = (SeqPListUsa) ajListIterGet(iter);
	ajDebug("%3d: '%S' %4d..%d (%b) '%S' %d\n",
		++i, node->Usa, node->Begin, node->End, node->Rev,
		node->Formatstr, node->Format);
    }

    ajListIterDel(&iter);
    ajDebug("...Done...\n");

    return;
}




/* @funcstatic seqListProcess *************************************************
**
** Processes a file of USAs.
** This function is called by, and calls, seqUsaProcess. There is
** a depth check to avoid infinite loops, for example where a list file
** refers to itself.
**
** This function produces a list (AjPList) of USAs with all list references
** expanded into lists of USAs.
**
** Because USAs in a list can have their own begin, end and reverse settings
** the prior setting are stored with each USA in the list node so that they
** can be restored after.
**
** @param [u] seq [AjPSeq] Sequence
** @param [u] seqin [AjPSeqin] Sequence input
** @param [r] listfile [const AjPStr] Name of list file.,
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqListProcess(AjPSeq seq, AjPSeqin seqin, const AjPStr listfile)
{
    AjPList list  = NULL;
    AjPFile file  = NULL;
    AjPStr token  = NULL;
    AjPStrTok handle = NULL;
    AjBool ret       = ajFalse;
    SeqPListUsa node = NULL;

    static ajint depth    = 0;
    static ajint MAXDEPTH = 16;

    depth++;
    ajDebug("++seqListProcess %S depth %d Rev: %B\n",
	    listfile, depth, seqin->Rev);
    if(depth > MAXDEPTH)
	ajFatal("USA List too deep");

    if(!ajListGetLength(seqin->List))
	seqin->List = ajListNew();

    list = ajListNew();

    file = ajFileNewInNameS(listfile);
    if(!file)
    {
	ajErr("Failed to open list file '%S'", listfile);
	depth--;
	return ret;
    }
    while(ajReadlineTrim(file, &seqReadLine))
    {
	seqListNoComment(&seqReadLine);
	if(ajStrGetLen(seqReadLine))
	{
	    ajStrTokenAssignC(&handle, seqReadLine, " \t\n\r");
	    ajStrTokenNextParse(&handle, &token);
	    /* ajDebug("Line  '%S'\ntoken '%S'\n", seqReadLine, token); */
	    if(ajStrGetLen(token))
	    {
	        ajDebug("++Add to list: '%S'\n", token);
	        AJNEW0(node);
	        ajStrAssignS(&node->Usa, token);
	        seqUsaSave(node, seqin);
	        ajListPushAppend(list, node);
	    }
	    ajStrDel(&token);
	    token = NULL;
	}
    }
    ajFileClose(&file);
    ajStrDel(&token);

    ajDebug("Trace seqin->List\n");
    seqUsaListTrace(seqin->List);
    ajDebug("Trace new list\n");
    seqUsaListTrace(list);
    ajListPushlist(seqin->List, &list);

    ajDebug("Trace combined seqin->List\n");
    seqUsaListTrace(seqin->List);

    /*
     ** now try the first item on the list
     ** this can descend recursively if it is also a list
     ** which is why we check the depth above
     */

    if(ajListPop(seqin->List, (void**) &node))
    {
        ajDebug("++pop first item '%S'\n", node->Usa);
	ajSeqinUsa(&seqin, node->Usa);
	seqUsaRestore(seqin, node);
	ajStrDel(&node->Usa);
	ajStrDel(&node->Formatstr);
	AJFREE(node);
	ajDebug("descending with usa '%S'\n", seqin->Usa);
	ret = seqUsaProcess(seq, seqin);
    }

    ajStrTokenDel(&handle);
    depth--;
    ajDebug("++seqListProcess depth: %d returns: %B\n", depth, ret);

    return ret;
}




/* @funcstatic seqListNoComment ***********************************************
**
** Strips comments from a character string (a line from an ACD file).
** Comments are blank lines or any text following a "#" character.
**
** @param [u] text [AjPStr*] Line of text from input file.
** @return [void]
** @@
******************************************************************************/

static void seqListNoComment(AjPStr* text)
{
    ajuint i;
    char *cp;

    i = ajStrGetLen(*text);

    if(!i)				/* empty string */
	return;

    MAJSTRGETUNIQUESTR(text);
    
    cp = strchr(ajStrGetPtr(*text), '#');
    if(cp)
    {					/* comment found */
	*cp = '\0';
	ajStrSetValid(text);
    }

    return;
}




/* @funcstatic seqFormatSet ***************************************************
**
** Sets the input format for a sequence using the sequence input object's
** defined format, or a default from variable 'EMBOSS_FORMAT'.
**
** @param [u] thys [AjPSeq] Sequence.
** @param [u] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqFormatSet(AjPSeq thys, AjPSeqin seqin)
{

    if(ajStrGetLen(seqin->Formatstr))
    {
	ajDebug("... input format value '%S'\n", seqin->Formatstr);
	if(seqFindInFormat(seqin->Formatstr, &seqin->Format))
	{
	    ajStrAssignS(&thys->Formatstr, seqin->Formatstr);
	    thys->Format = seqin->Format;
	    ajDebug("...format OK '%S' = %d\n", seqin->Formatstr,
		    seqin->Format);
	}
	else
	    ajDebug("...format unknown '%S'\n", seqin->Formatstr);

	return ajTrue;
    }
    else
	ajDebug("...input format not set\n");


    return ajFalse;
}




/* @funcstatic seqinUfoLocal **************************************************
**
** Tests whether a sequence input object will read features from the
** sequence input file. The alternative is to use a separate UFO.
**
** @param [r] thys [const AjPSeqin] Sequence input object.
** @return [AjBool] ajTrue if the features will be read from the sequence
** @@
******************************************************************************/

static AjBool seqinUfoLocal(const AjPSeqin thys)
{
    if(thys->Features && ! ajStrGetLen(thys->Ufo))
	return ajTrue;

    return ajFalse;
}




/* @funcstatic seqSetName *****************************************************
**
** Sets the name for a sequence object by applying simple conversion
** rules to the input which could be, for example, the name from a
** FASTA format file.
**
** @param [u] name [AjPStr*] Sequence name derived.
** @param [r] str [const AjPStr] User supplied name.
** @return [void]
** @@
******************************************************************************/

static void seqSetName(AjPStr* name, const AjPStr str)
{
    AjPStrTok split = NULL;
    AjPStr token = NULL;

    if(ajStrIsWord(str))
    {
	ajDebug("seqSetName word '%S'\n", str);
	split = ajStrTokenNewC(str, ":");
	while(ajStrTokenNextParse(&split, &token))
	{
	    if(ajStrGetLen(token))
	       ajStrAssignS(name, token);
	}
	ajStrTokenDel(&split);
    }
    else
    {
	ajDebug("seqSetName non-word '%S'\n", str);
	ajStrAssignS(name, str);
	ajStrRemoveWhiteExcess(name);
	ajStrExchangeKK(name, ' ', '_');
	ajDebug("seqSetName cleaned '%S'\n", *name);
    }

    ajDebug("seqSetName '%S' result: '%S'\n", str, *name);
    ajStrDel(&token);
    return;
}




/* @funcstatic seqSetNameNospace **********************************************
**
** Sets the name for a sequence object by applying simple conversion
** rules to the input which could be, for example, the name from a
** FASTA format file.
**
** @param [u] name [AjPStr*] Sequence name derived.
** @param [r] str [const AjPStr] User supplied name.
** @return [void]
** @@
******************************************************************************/

static void seqSetNameNospace(AjPStr* name, const AjPStr str)
{
    ajStrAssignS(name, str);
    if(!ajStrIsWord(str))
    {
	ajDebug("seqSetNameNospace non-word '%S'\n", str);
	ajStrRemoveWhiteExcess(name);
	ajStrExchangeKK(name, ' ', '_');
	ajDebug("seqSetNameNospace cleaned '%S'\n", *name);
    }

    ajDebug("seqSetNameNospace '%S' result: '%S'\n", str, *name);
    return;
}




/* @funcstatic seqSetNameFile *************************************************
**
** Sets the name for a sequence object by applying simple conversion
** rules to the input source file..
**
** @param [u] name [AjPStr*] Sequence name derived.
** @param [r] seqin [const AjPSeqin] Sequence input object
** @return [void]
** @@
******************************************************************************/

static void seqSetNameFile(AjPStr* name, const AjPSeqin seqin)
{
    AjPStr tmpname = NULL;

    ajStrAssignS(&tmpname, seqin->Filename);

    seqSetName(name, tmpname);
    if(seqin->Count > 1)
	ajFmtPrintAppS(name, "_%3d", seqin->Count);

    ajDebug("seqSetNameFile '%S' result: '%S'\n", tmpname, *name);
    ajStrDel(&tmpname);

    return;
}




/* @funcstatic seqAccSave *****************************************************
**
** Adds an accession number to the stored list for a sequence.
** The first accession number is also saved as the primary number.
**
** @param [u] thys [AjPSeq] Sequence object
** @param [r] acc [const AjPStr] Accession number
** @return [void]
** @@
******************************************************************************/

static void seqAccSave(AjPSeq thys, const AjPStr acc)
{
    ajListstrPushAppend(thys->Acclist, ajStrNewS(acc));

    if(!ajStrGetLen(thys->Acc))
	ajStrAssignS(&thys->Acc, acc);

    return;
}




/* @funcstatic seqTaxSave *****************************************************
**
** Adds an organism taxonomy level to the stored list for a sequence.
** The first is also saved as the primary 'Tax' (should be the species).
**
** @param [u] thys [AjPSeq] Sequence object
** @param [r] tax [const AjPStr] Organism taxonomy
** @param [r] level [ajuint] 0: taxon level 1: species 2: organelle
** @return [void]
** @@
******************************************************************************/

static void seqTaxSave(AjPSeq thys, const AjPStr tax, ajuint level)
{
    AjPStr newstr = NULL;

    switch(level)
    {
    case 1:
	if(!ajStrGetLen(thys->Tax))
	    ajStrAssignS(&thys->Tax, tax);
	break;
    case 2:
	if(!ajStrGetLen(thys->Organelle))
	    ajStrAssignS(&thys->Organelle, tax);
	break;
    default:
	newstr = ajStrNewS(tax);
	ajListstrPushAppend(thys->Taxlist, newstr);
	break;
    }

    return;
}




/* @funcstatic seqSvSave ******************************************************
**
** Adds a sequence version number to the stored data for a sequence.
**
** @param [u] thys [AjPSeq] Sequence object
** @param [r] sv [const AjPStr] SeqVersion number
** @return [void]
** @@
******************************************************************************/

static void seqSvSave(AjPSeq thys, const AjPStr sv)
{
    if(!ajStrGetLen(thys->Sv))
	ajStrAssignS(&thys->Sv, sv);

    return;
}




/* ==================================================================== */
/* ========================= constructors ============================= */
/* ==================================================================== */

/* @section Sequence Query Constructors ***************************************
**
** All constructors return a new sequence query object by pointer. It
** is the responsibility of the user to first destroy any previous
** sequenceoutput object. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so
** anyway.
**
******************************************************************************/




/* @func ajSeqQueryNew ********************************************************
**
** Creates a new sequence query object
**
** @return [AjPSeqQuery] New sequence query object.
** @category new [AjPSeqQuery] Default constructor
** @@
******************************************************************************/

AjPSeqQuery ajSeqQueryNew(void)
{
    AjPSeqQuery pthis;

    AJNEW0(pthis);

    pthis->DbName = ajStrNew();
    pthis->DbAlias= ajStrNew();
    pthis->Id     = ajStrNew();
    pthis->Acc    = ajStrNew();
    pthis->Sv     = ajStrNew();
    pthis->Gi     = ajStrNew();
    pthis->Des    = ajStrNew();
    pthis->Org    = ajStrNew();
    pthis->Key    = ajStrNew();

    pthis->Wild   = ajFalse;

    pthis->Method      = ajStrNew();
    pthis->Formatstr   = ajStrNew();
    pthis->IndexDir    = ajStrNew();
    pthis->Directory   = ajStrNew();
    pthis->Filename    = ajStrNew();
    pthis->Application = ajStrNew();
    pthis->Field       = ajStrNew();

    pthis->Type    = QRY_UNKNOWN;
    pthis->Access  = NULL;
    pthis->QryData = NULL;
    pthis->Fpos    = NULLFPOS;
    pthis->QryDone = ajFalse;
    pthis->HasAcc  = ajTrue;

    return pthis;
}




/* ==================================================================== */
/* ========================== destructors ============================= */
/* ==================================================================== */

/* @section Sequence Query Destructors ****************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the sequence query object.
**
******************************************************************************/




/* @func ajSeqQueryDel ********************************************************
**
** Deletes a sequence query object
**
** @param [d] pthis [AjPSeqQuery*] Address of sequence query object
** @return [void]
** @category delete [AjPSeqQuery] Default destructor
** @@
******************************************************************************/

void ajSeqQueryDel(AjPSeqQuery* pthis)
{
    AjPSeqQuery thys;

    ajDebug("ajSeqQueryDel db:'%S' id:'%S'\n", (*pthis)->DbName, (*pthis)->Id);

    thys = *pthis;

    ajStrDel(&thys->DbName);
    ajStrDel(&thys->DbAlias);
    ajStrDel(&thys->DbType);
    ajStrDel(&thys->Id);
    ajStrDel(&thys->Acc);
    ajStrDel(&thys->Des);
    ajStrDel(&thys->Key);
    ajStrDel(&thys->Org);
    ajStrDel(&thys->Sv);
    ajStrDel(&thys->Gi);
    ajStrDel(&thys->Method);
    ajStrDel(&thys->Formatstr);
    ajStrDel(&thys->IndexDir);
    ajStrDel(&thys->Directory);
    ajStrDel(&thys->Filename);
    ajStrDel(&thys->Exclude);
    ajStrDel(&thys->DbFields);
    ajStrDel(&thys->DbProxy);
    ajStrDel(&thys->DbHttpVer);
    ajStrDel(&thys->Field);
    ajStrDel(&thys->QryString);
    ajStrDel(&thys->Application);

    if(thys->QryData)
    {
	if(thys->Access->AccessFree)
	    thys->Access->AccessFree(thys);

	AJFREE(thys->QryData);
    }

    AJFREE(*pthis);

    return;
}




/* ==================================================================== */
/* ========================== Assignments ============================= */
/* ==================================================================== */

/* @section Sequence Query Assignments ****************************************
**
** These functions overwrite the sequence query object provided as
** the first argument.
**
******************************************************************************/




/* ==================================================================== */
/* =========================== Modifiers ============================== */
/* ==================================================================== */

/* @section Sequence Query Modifiers ******************************************
**
** These functions use the contents of a sequence query object and
** update them.
**
******************************************************************************/




/* @func ajSeqQueryClear ******************************************************
**
** Resets a Sequence query object to a clean state for reuse
**
** @param [u] thys [AjPSeqQuery] Sequence query object
** @return [void]
** @category modify [AjPSeqQuery] Clears all contents
** @@
******************************************************************************/

void ajSeqQueryClear(AjPSeqQuery thys)
{

    ajStrSetClear(&thys->DbName);
    ajStrSetClear(&thys->DbAlias);
    ajStrSetClear(&thys->Id);
    ajStrSetClear(&thys->Acc);
    ajStrSetClear(&thys->Sv);
    ajStrSetClear(&thys->Gi);
    ajStrSetClear(&thys->Des);
    ajStrSetClear(&thys->Org);
    ajStrSetClear(&thys->Key);
    ajStrSetClear(&thys->Method);
    ajStrSetClear(&thys->Formatstr);
    ajStrSetClear(&thys->IndexDir);
    ajStrSetClear(&thys->Directory);
    ajStrSetClear(&thys->Filename);
    ajStrSetClear(&thys->Application);

    thys->Type   = QRY_UNKNOWN;
    thys->Access = NULL;
    if(thys->QryData)
	AJFREE(thys->QryData);

    thys->QryDone = ajFalse;

    return;
}




/* ==================================================================== */
/* ======================== Operators ==================================*/
/* ==================================================================== */

/* @section Sequence Query Operators ******************************************
**
** These functions use the contents of a sequence query object but do
** not make any changes.
**
******************************************************************************/




/* @funcstatic seqQueryMatch **************************************************
**
** Compares a sequence to a query and returns true if they match.
**
** @param [r] thys [const AjPSeqQuery] Sequence query.
** @param [r] seq [const AjPSeq] Sequence.
** @return [AjBool] ajTrue if the sequence matches the query.
** @category use [AjPSeqQuery] Compares an AjPSeq to a query.
** @@
******************************************************************************/

static AjBool seqQueryMatch(const AjPSeqQuery thys, const AjPSeq seq)
{
    AjBool tested = ajFalse;
    AjIList iter  = NULL;
    AjPStr accstr;			/* from list, do not delete */
    AjPStr keystr;			/* from list, do not delete */
    AjPStr taxstr;			/* from list, do not delete */
    AjBool ok = ajFalse;

    ajDebug("seqQueryMatch '%S' id '%S' acc '%S' Sv '%S' Gi '%S' Des '%S'"
	    " Key '%S' Org '%S' Case %B Done %B\n",
	    seq->Name, thys->Id, thys->Acc, thys->Sv, thys->Gi,
	    thys->Des, thys->Key, thys->Org, thys->CaseId, thys->QryDone);

    if(!thys)			   /* no query to test, that's fine */
	return ajTrue;

    if(thys->QryDone)			/* do we need to test here? */
	return ajTrue;

    /* test the query field(s) */

    if(ajStrGetLen(thys->Id))
    {
	if(thys->CaseId)
	{
	    if(ajStrMatchWildS(seq->Name, thys->Id))
		return ajTrue;
	}
	else
	{
	    if(ajStrMatchWildCaseS(seq->Name, thys->Id))
		return ajTrue;
	}

	ajDebug("id test failed\n");
	tested = ajTrue;
	ok = ajFalse;
    }

    if(ajStrGetLen(thys->Sv)) /* test Sv and Gi */
    {
	if(ajStrMatchWildCaseS(seq->Sv, thys->Sv))
	    return ajTrue;

	ajDebug("sv test failed\n");
	tested = ajTrue;
	ok = ajFalse;
    }

    if(ajStrGetLen(thys->Gi)) /* test Sv and Gi */
    {
	if(ajStrMatchWildCaseS(seq->Gi, thys->Gi))
	    return ajTrue;

	ajDebug("gi test failed\n");
	tested = ajTrue;
	ok = ajFalse;
    }

    if(!ajStrGetLen(thys->Acc))
    {
	/*ajDebug("No accession number to test\n");*/
    }
    else if(ajListGetLength(seq->Acclist))
    {		   /* accession number test - check the entire list */
	iter = ajListIterNewread(seq->Acclist);
	while(!ajListIterDone(iter))
	{
	    accstr = ajListIterGet(iter);
	    ajDebug("... try accession '%S' '%S'\n", accstr,
		    thys->Acc);

	    if(ajStrMatchWildCaseS(accstr, thys->Acc))
	    {
		ajListIterDel(&iter);
		return ajTrue;
	    }
	}
	tested = ajTrue;
	ajDebug("acc test failed\n");
	ajListIterDel(&iter);
    }

    if(!ajStrGetLen(thys->Org))
    {
	/*ajDebug("No taxonomy to test\n"); */
    }
    else if(ajListGetLength(seq->Taxlist))
    {			   /* taxonomy test - check the entire list */
	iter = ajListIterNewread(seq->Taxlist);
	while(!ajListIterDone(iter))
	{
	    taxstr = ajListIterGet(iter);
	    ajDebug("... try organism '%S' '%S'\n", taxstr,
		    thys->Org);

	    if(ajStrMatchWildCaseS(taxstr, thys->Org))
	    {
		ajListIterDel(&iter);
		return ajTrue;
	    }
	}
	tested = ajTrue;
	ajDebug("org test failed\n");
	ajListIterDel(&iter);
    }
    else
    {
	ajDebug("org test failed - nothing to test\n");
	return ajFalse;
    }

    if(!ajStrGetLen(thys->Key))
    {
	/*ajDebug("No keyword to test\n");*/
    }
    else if(ajListGetLength(seq->Keylist))
    {			    /* keyword test - check the entire list */
	iter = ajListIterNewread(seq->Keylist);
	while(!ajListIterDone(iter))
	{
	    keystr = ajListIterGet(iter);
	    ajDebug("... try keyword '%S' '%S'\n", keystr,
		    thys->Key);

	    if(ajStrMatchWildCaseS(keystr, thys->Key))
	    {
		ajListIterDel(&iter);
		return ajTrue;
	    }
	}
	tested = ajTrue;
	ajDebug("key test failed\n");
	ajListIterDel(&iter);
    }
    else
    {
	ajDebug("key test failed - nothing to test\n");
	return ajFalse;
    }

    if(!ajStrGetLen(thys->Des))
    {
	/*ajDebug("No description to test\n");*/
	ok = ajFalse;
    }
    else if(ajStrGetLen(seq->Desc))
    {			     /* description test - check the string */
	ajDebug("... try description '%S' '%S'\n", seq->Desc,
		thys->Des);

	if(ajStrMatchWildWordCaseS(seq->Desc, thys->Des))
	    return ajTrue;

	tested = ajTrue;
	ajDebug("des test failed\n");
	ajListIterDel(&iter);
    }
    else
    {
	ajDebug("des test failed - nothing to test\n");
	return ajFalse;
    }

    if(!tested)		    /* nothing to test, so accept it anyway */
	return ajTrue;

    ajDebug("result: %B\n", ok);

    return ok;
}




/* @func ajSeqQueryWild *******************************************************
**
** Tests whether a query includes wild cards in any element,
** or can return more than one entry (keyword and some other search terms
** will find multiple entries)
**
** @param [u] qry [AjPSeqQuery] Query object.
** @return [AjBool] ajTrue if query had wild cards.
** @category modify [AjPSeqQuery] Tests whether a query includes wildcards
** @@
******************************************************************************/

AjBool ajSeqQueryWild(AjPSeqQuery qry)
{

    if(!seqRegQryWild)
	seqQryWildComp();

    ajDebug("ajSeqQueryWild id '%S' acc '%S' sv '%S' gi '%S' des '%S'"
	    " org '%S' key '%S'\n",
	    qry->Id, qry->Acc, qry->Sv, qry->Gi, qry->Des, qry->Org, qry->Key);

    if(ajRegExec(seqRegQryWild, qry->Id))
    {
	ajDebug("wild query Id '%S'\n", qry->Id);
	qry->Wild = ajTrue;
	return ajTrue;
    }

    if(ajStrGetLen(qry->Acc))
    {
	if(strpbrk(qry->Acc->Ptr,"*?"))
	    qry->Wild = ajTrue;
	
        if(!ajStrGetLen(qry->Id))
	{
	    ajDebug("wild (has, but no Id) query Acc '%S'\n", qry->Acc);
	    return ajTrue;
	}
	else if(ajRegExec(seqRegQryWild, qry->Id))
        {
	    ajDebug("wild query Acc '%S'\n", qry->Acc);
	    return ajTrue;
	}
    }

    if(ajStrGetLen(qry->Sv))
    {
	if(strpbrk(qry->Sv->Ptr,"*?"))
	    qry->Wild = ajTrue;

	ajDebug("wild (has) query Sv '%S'\n", qry->Sv);
	return ajTrue;
    }

    if(ajStrGetLen(qry->Gi))
    {
	if(strpbrk(qry->Gi->Ptr,"*?"))
	    qry->Wild = ajTrue;

	if(!ajStrIsNum(qry->Gi))
	{
	    ajDebug("wild (has) query Gi '%S'\n", qry->Gi);
	    return ajTrue;
	}
    }

    if(ajStrGetLen(qry->Des))
    {
	if(strpbrk(qry->Des->Ptr,"*?"))
	    qry->Wild = ajTrue;

	ajDebug("wild (has) query Des '%S'\n", qry->Des);
	return ajTrue;
    }

    if(ajStrGetLen(qry->Org))
    {
	if(strpbrk(qry->Org->Ptr,"*?"))
	    qry->Wild = ajTrue;

	ajDebug("wild (has) query Org '%S'\n", qry->Org);
	return ajTrue;
    }

    if(ajStrGetLen(qry->Key))
    {
	if(strpbrk(qry->Key->Ptr,"*?"))
	    qry->Wild = ajTrue;

	ajDebug("wild (has) query Key '%S'\n", qry->Key);
	return ajTrue;
    }

    ajDebug("no wildcard in stored qry\n");

    return ajFalse;
}




/* @func ajSeqQueryStarclear **************************************************
**
** Clears elements of a query object if they are simply "*" because this
** is equivalent to a null string.
**
** @param [u] qry [AjPSeqQuery] Query object.
** @return [void]
** @category modify [AjPSeqQuery] Clears fully wild elements of a query
**                 because empty elements are the same.
** @@
******************************************************************************/

void ajSeqQueryStarclear(AjPSeqQuery qry)
{
    if(ajStrMatchC(qry->Id, "*"))
    {
	ajDebug("ajSeqQueryStarclear clear Id '%S'\n", qry->Id);
	ajStrSetClear(&qry->Id);
    }

    if(ajStrMatchC(qry->Acc, "*"))
    {
	ajDebug("ajSeqQueryStarclear clear Acc '%S'\n", qry->Acc);
	ajStrSetClear(&qry->Acc);
    }

    if(ajStrMatchC(qry->Sv, "*"))
    {
	ajDebug("ajSeqQueryStarclear clear Sv '%S'\n", qry->Sv);
	ajStrSetClear(&qry->Sv);
    }

    if(ajStrMatchC(qry->Gi, "*"))
    {
	ajDebug("ajSeqQueryStarclear clear Gi '%S'\n", qry->Gi);
	ajStrSetClear(&qry->Gi);
    }

    if(ajStrMatchC(qry->Des, "*"))
    {
	ajDebug("ajSeqQueryStarclear clear Des '%S'\n", qry->Des);
	ajStrSetClear(&qry->Des);
    }

    if(ajStrMatchC(qry->Org, "*"))
    {
	ajDebug("ajSeqQueryStarclear clear Org '%S'\n", qry->Org);
	ajStrSetClear(&qry->Org);
    }

    if(ajStrMatchC(qry->Key, "*"))
    {
	ajDebug("ajSeqQueryStarclear clear Key '%S'\n", qry->Key);
	ajStrSetClear(&qry->Key);
    }

    return;
}




/* @func ajSeqQueryIs *********************************************************
**
** Tests whether any element of a query has been set. Elements which
** are simply '*' are cleared as this has the same meaning.
**
** @param [r] qry [const AjPSeqQuery] Query object.
** @return [AjBool] ajTrue if query should be made. ajFalse if the query
**                  includes all entries.
** @category use [AjPSeqQuery] Tests whether a query has been defined
** @@
******************************************************************************/

AjBool ajSeqQueryIs(const AjPSeqQuery qry)
{

    if(ajStrGetLen(qry->Id))
	return ajTrue;
    if(ajStrGetLen(qry->Acc))
	return ajTrue;
    if(ajStrGetLen(qry->Sv))
	return ajTrue;
    if(ajStrGetLen(qry->Gi))
	return ajTrue;
    if(ajStrGetLen(qry->Des))
	return ajTrue;
    if(ajStrGetLen(qry->Org))
	return ajTrue;
    if(ajStrGetLen(qry->Key))
	return ajTrue;

    return ajFalse;
}




/* @funcstatic seqQryWildComp *************************************************
**
** Compiles the reqular expressions for testing wild cards in queries.
** These are held in static storage and built once only if needed.
**
** @return [void]
** @@
******************************************************************************/

static void seqQryWildComp(void)
{
    if(!seqRegQryWild)
	seqRegQryWild = ajRegCompC("[*?]");

    return;
}




/* @func ajSeqQueryTrace ******************************************************
**
** Debug calls to trace the data in a sequence query object.
**
** @param [r] thys [const AjPSeqQuery] Sequence query object.
** @return [void]
** @@
******************************************************************************/

void ajSeqQueryTrace(const AjPSeqQuery thys)
{
    ajDebug( "  Query Trace\n");

    if(ajStrGetLen(thys->DbName))
	ajDebug( "    DbName: '%S'\n", thys->DbName);

    if(ajStrGetLen(thys->DbAlias))
	ajDebug( "    DbAlias: '%S'\n", thys->DbAlias);

    if(ajStrGetLen(thys->DbType))
	ajDebug( "    DbType: '%S' (%d)\n", thys->DbType, thys->Type);

    if(ajStrGetLen(thys->Id))
	ajDebug( "    Id: '%S'\n", thys->Id);

    if(ajStrGetLen(thys->Acc))
	ajDebug( "    Acc: '%S'\n", thys->Acc);

    if(ajStrGetLen(thys->Des))
	ajDebug( "    Des: '%S'\n", thys->Des);

    if(ajStrGetLen(thys->Key))
	ajDebug( "    Key: '%S'\n", thys->Key);

    if(ajStrGetLen(thys->Org))
	ajDebug( "    Org: '%S'\n", thys->Org);

    if(ajStrGetLen(thys->Sv))
	ajDebug( "    Sv: '%S'\n", thys->Sv);

    if(ajStrGetLen(thys->Gi))
	ajDebug( "    Gi: '%S'\n", thys->Gi);

    ajDebug( "    Case-sensitive Id: '%B'\n", thys->CaseId);
    ajDebug( "   Has accession: %B\n", thys->HasAcc);

    if(ajStrGetLen(thys->Method))
	ajDebug( "    Method: '%S'\n", thys->Method);

    if(ajStrGetLen(thys->Formatstr))
	ajDebug( "    Formatstr: '%S'\n", thys->Formatstr);

    if(ajStrGetLen(thys->IndexDir))
	ajDebug( "    IndexDir: '%S'\n", thys->IndexDir);

    if(ajStrGetLen(thys->Directory))
	ajDebug( "    Directory: '%S'\n", thys->Directory);

    if(ajStrGetLen(thys->Filename))
	ajDebug( "    Filename: '%S'\n", thys->Filename);

    if(ajStrGetLen(thys->Exclude))
	ajDebug( "    Exclude: '%S'\n", thys->Exclude);

    if(ajStrGetLen(thys->DbFields))
	ajDebug( "    DbFields: '%S'\n", thys->DbFields);

    if(ajStrGetLen(thys->DbProxy))
	ajDebug( "    DbProxy: '%S'\n", thys->DbProxy);

    if(ajStrGetLen(thys->DbHttpVer))
	ajDebug( "    DbHttpVer: '%S'\n", thys->DbHttpVer);

    if(ajStrGetLen(thys->Field))
	ajDebug( "    Field: '%S'\n", thys->Field);

    if(ajStrGetLen(thys->QryString))
	ajDebug( "    QryString: '%S'\n", thys->QryString);

    if(ajStrGetLen(thys->Application))
	ajDebug( "    Application: '%S'\n", thys->Application);

    if(ajStrGetLen(thys->Application))
	ajDebug( "    Application: '%S'\n", thys->Application);

    ajDebug( "   Fpos: %ld\n", thys->Fpos);
    ajDebug( "   QryDone: %B\n", thys->QryDone);
    ajDebug( "   Wildcard in query: %B\n", thys->Wild);

    if(thys->Access)
	ajDebug( "    Access: exists\n");

    if(thys->QryData)
	ajDebug( "    QryData: exists\n");

    return;
}




/* ==================================================================== */
/* ============================ Casts ================================= */
/* ==================================================================== */

/* @section Sequence Query Casts **********************************************
**
** These functions examine the contents of a sequence query object
** and return some derived information. Some of them provide access to
** the internal components of a sequence query object. They are
** provided for programming convenience but should be used with
** caution.
**
******************************************************************************/




/* @func ajSeqParseFasta ******************************************************
**
** Parse an NCBI format fasta line. Return id acc sv and description
**
** @param [r] instr [const AjPStr]   fasta line.
** @param [w] id [AjPStr*]   id.
** @param [w] acc [AjPStr*]  accession number.
** @param [w] sv [AjPStr*]  sequence version number.
** @param [w] desc [AjPStr*] description.
** @return [AjBool] ajTrue if fasta format
** @@
******************************************************************************/

AjBool ajSeqParseFasta(const AjPStr instr, AjPStr* id, AjPStr* acc,
		       AjPStr* sv, AjPStr* desc)
{
    AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    AjPStr token2    = NULL;
    AjPStr str       = NULL;
    AjBool ok = ajFalse;

    ajDebug("ajSeqParseFasta '%S'\n", instr);

    if(!ajStrPrefixC(instr, ">"))
	return ajFalse;

    ajStrAssignS(&str, instr);

    ajStrTokenAssignC(&handle, str, "> ");
    ajStrTokenNextParseC(&handle, " \t\n\r", id);

    ok = ajStrTokenNextParse(&handle, &token);
    ajStrAssignS(&token2, token);
    ajStrRemoveSetC(&token2, "()");

    if(ok && ajSeqtestIsSeqversion(token2))
    {
        ajStrAssignS(acc, ajSeqtestIsSeqversion(token2));
	ajStrAssignS(sv, token2);
	ajStrTokenNextParseC(&handle, "\n\r", desc);
    }
    else if(ok && ajSeqtestIsAccession(token2))
    {
	ajStrAssignS(acc, token2);
        ajStrAssignClear(sv);
	ajStrTokenNextParseC(&handle, "\n\r", desc);
    }
    else if(ok)
    {
        ajStrAssignClear(acc);
        ajStrAssignClear(sv);
	ajStrAssignS(desc, token);
	if(ajStrTokenNextParseC(&handle, "\n\r", &token))
	{
	    ajStrAppendC(desc, " ");
	    ajStrAppendS(desc, token);
	}
    }

    ajStrDel(&token); /* duplicate of accession or description */
    ajStrDel(&token2);
    ajStrTokenDel(&handle);
    ajStrDel(&str);
    ajDebug("result id: '%S' acc: '%S' desc: '%S'\n", *id, *acc, *desc);

    return ajTrue;
}




/* @func ajSeqParseNcbi *******************************************************
**
** Parse an NCBI format fasta line. Return id acc and description.
**
** Tries to cope with the amazing variety of identifiers NCBI inflicts
** on us all - see the BLAST document README.formatdb from NCBI for
** some of the gory detail, and look at some real files for clues
** to what can really happen. Sadly,'real files' also includes
** internal IDs in blast databases reformatted by formatdb.
**
** @param [r] instr [const AjPStr]   fasta line.
** @param [w] id [AjPStr*]   id.
** @param [w] acc [AjPStr*]  accession number.
** @param [w] sv [AjPStr*]  sequence version number.
** @param [w] gi [AjPStr*]  GI version number.
** @param [w] db [AjPStr*]  NCBI database name
** @param [w] desc [AjPStr*] description.
** @return [AjBool] ajTrue if ncbi format
** @@
******************************************************************************/

AjBool ajSeqParseNcbi(const AjPStr instr, AjPStr* id, AjPStr* acc,
		      AjPStr* sv, AjPStr* gi, AjPStr* db, AjPStr* desc)
{
    AjPStrTok idhandle = NULL;
    AjPStrTok handle   = NULL;
    AjPStr idstr       = NULL;
    AjPStr reststr     = NULL;
    AjPStr prefix      = NULL;
    AjPStr token       = NULL;
    AjPStr numtoken    = NULL;
    AjPStr str         = NULL;
    const AjPStr vacc  = NULL;
    const char *q;
    ajuint  i;
    ajuint  nt;
    AjBool ret = ajFalse;

    ajStrAssignClear(db);

    /* NCBI's list of standard identifiers June 2001
     ** ftp://ncbi.nlm.nih.gov/blast/db/README.formatdb
     **
     ** Database Name                         Identifier Syntax
     **
     ** GenBank                               gb|accession|locus
     ** EMBL Data Library                     emb|accession|locus
     ** DDBJ, DNA Database of Japan           dbj|accession|locus
     ** SWISS-PROT                            sp|accession|entry name
     ** NCBI Reference Sequence               ref|accession|locus
     **
     ** General database identifier           gnl|database|identifier
     ** BLAST formatdb                        gnl|BL_ORD_ID|number
     **   (prefix for normal FASTA header - remove)
     **
     ** NBRF PIR                              pir||entry
     ** Protein Research Foundation           prf||name
     **   (Japanese SEQDB protein DB)
     **
     ** Brookhaven Protein Data Bank          pdb|entry|chain
     **
     ** Patents                               pat|country|number
     **
     ** GenInfo Backbone Id                   bbs|number
     ** Local Sequence identifier             lcl|identifier
     **
     ** GenInfo identifier prefix             gi|gi_identifier
     **   (prefix - remove)
     */

    ajDebug("ajSeqParseNcbi '%S'\n", instr);

    if(ajStrGetCharPos(instr, 3) == ';')  /* then it is really PIR format */
    {
	ajDebug("ajSeqParseNcbi failed: this is PIR format\n");
	return ajFalse;
    }

    ajStrAssignS(&str, instr);

    /* ajDebug("id test %B %B\n",
       !strchr(MAJSTRGETPTR(str), (ajint)'|'),
       (*MAJSTRGETPTR(str)!='>')); */

    /* Line must start with '>', and include '|' bar, hopefully in the ID */

    if(*MAJSTRGETPTR(str)!='>')
    {
	ajDebug("ajSeqParseNcbi failed: no '>' at start\n");
	ajStrDel(&str);
	return ajFalse;
    }

    /* pick out the ID */

    ajStrTokenAssignC(&idhandle,str,"> \t\r\n");
    ajStrTokenNextParse(&idhandle, &idstr);
    ajStrTokenNextParseC(&idhandle, "\r\n", &reststr);
    ajStrTokenDel(&idhandle);

    /* check we have an ID */

    if(!ajStrGetLen(idstr))
    {
	ajDebug("No ID string found - but try FASTA\n");
	ret = ajSeqParseFasta(str, id, acc, sv, desc);
	ajStrDel(&str);
	ajStrDel(&idstr);
	ajStrDel(&reststr);
	return ret;
    }

    /* NCBI ids always have | somewhere. Else we try a simple FASTA format */

    if(!strchr(MAJSTRGETPTR(idstr),(ajint)'|'))
    {
	ajDebug("trying ajSeqParseFasta\n");
	ret = ajSeqParseFasta(str, id, acc, sv, desc);
	ajStrDel(&str);
	ajStrDel(&idstr);
	ajStrDel(&reststr);
	return ret;
    }

    ajStrAssignClear(id);
    ajStrTokenAssignC(&handle,idstr,"|");

    ajStrTokenNextParse(&handle, &prefix);
    q = MAJSTRGETPTR(prefix);

    ajDebug(" idstr: '%S'\n", idstr);
    ajDebug("prefix: '%S'\n", prefix);

    if(!strncmp(q,"gi",2))
    {
        /* ajDebug("gi prefix\n"); */
	ajStrTokenNextParse(&handle, &token);
	ajStrAssignS(gi, token);
	if(! ajStrTokenNextParse(&handle, &prefix))
	{
	    /* we only have a gi prefix */
	    ajDebug("*only* gi prefix\n");
	    ajStrAssignS(id, token);
	    ajStrAssignClear(acc);
	    ajStrAssignS(desc, reststr);
	    ajDebug("found pref: '%S' id: '%S', acc: '%S' "
	       "desc: '%S'\n",
	       prefix, *id, *acc, *desc);
	    ajStrDel(&str);
	    ajStrDel(&idstr);
	    ajStrDel(&reststr);
	    ajStrDel(&prefix);
	    ajStrDel(&token);
	    ajStrTokenDel(&handle);
	    return ajTrue;
	}

	/* otherwise we continue to parse the rest */
	q = MAJSTRGETPTR(prefix);
	ajDebug("continue with '%S'\n", prefix);
    }


    /*
     * This next routine and associated function could be used if
     * whatever is appended to gnl lines is consistent
     */

    if(!strncmp(MAJSTRGETPTR(idstr),"gnl|BL_ORD_ID|",14))
    {
        /* ajDebug("gnl|BL_ORD_ID stripping\n"); */
	ajStrTokenNextParse(&handle, &token); /* BL_ORD_ID */
	ajStrTokenNextParse(&handle, &numtoken); /* number */
	ajStrInsertC(&reststr, 0, ">");

	if(ajSeqParseNcbi(reststr,id,acc,sv,gi,db,desc))
	{
	    ajStrAssignEmptyS(db, token);
	    /* recursive ... */
	    ajDebug("ajSeqParseNcbi recursive success '%S'\n", reststr);
	    /* ajDebug("found pref: '%S' id: '%S', acc: '%S' "
	       "sv: '%S' desc: '%S'\n",
	       prefix, *id, *acc, *sv, *desc); */
	    ajStrDel(&str);
	    ajStrDel(&idstr);
	    ajStrDel(&reststr);
	    ajStrDel(&prefix);
	    ajStrDel(&numtoken);
	    ajStrDel(&token);
	    ajStrTokenDel(&handle);
	    return ajTrue;
	}
        ajDebug("ajSeqParseNcbi recursive failed '%S' - use gnl id\n",
		reststr);
	ajStrAssignS(id,numtoken);
	ajStrAssignClear(acc);
	/* ajDebug("found pref: '%S' id: '%S', acc: '%S' "
	   "sv: '%S' desc: '%S'\n",
	   prefix, *id, *acc, *sv, *desc); */
	ajStrDel(&str);
	ajStrDel(&idstr);
	ajStrDel(&reststr);
	ajStrDel(&prefix);
	ajStrDel(&numtoken);
	ajStrDel(&token);
	ajStrTokenDel(&handle);
	return ajTrue;
    }

    /* works for NCBI formatdb reformatted blast databases
     ** still checking for any misformatted databases elsewhere */

    if(!strcmp(q,"bbs") || !strcmp(q,"lcl"))
    {
	if(!strcmp(q, "lcl"))
	    ajStrAssignS(db, prefix);

        /* ajDebug("bbs or lcl prefix\n"); */
	ajStrTokenNextParse(&handle, id);
	ajStrAssignClear(acc);
	ajStrAssignS(desc, reststr);
	/* ajDebug("found pref: '%S' id: '%S', acc: '%S' desc: '%S'\n",
	   prefix, *id, *acc, *desc); */
	ajStrDel(&str);
	ajStrDel(&idstr);
	ajStrDel(&reststr);
	ajStrDel(&prefix);
	ajStrDel(&numtoken);
	ajStrDel(&token);
	ajStrTokenDel(&handle);
	return ajTrue;
    }

    if(!strcmp(q,"gnl") || !strcmp(q,"pat"))
    {
	/* ajDebug("gnl or pat prefix\n"); */
	ajStrTokenNextParse(&handle, &token);
        if(!strcmp(q,"gnl"))
	   ajStrAssignS(db, token);
	ajStrTokenNextParse(&handle, id);
	ajStrAssignClear(acc);		/* no accession number */
	ajStrAssignS(desc, reststr);
	/* ajDebug("found pref: '%S' id: '%S', acc: '%S' desc: '%S'\n",
	   prefix, *id, *acc, *desc); */
	ajStrDel(&str);
	ajStrDel(&idstr);
	ajStrDel(&reststr);
	ajStrDel(&prefix);
	ajStrDel(&numtoken);
	ajStrDel(&token);
	ajStrTokenDel(&handle);
	return ajTrue;
    }


    if(!strcmp(q,"pdb"))
    {
	ajStrAssignS(db, prefix);
        /* ajDebug("gnl or pat or pdb prefix\n"); */
	ajStrTokenNextParse(&handle, id);
	if(ajStrTokenNextParse(&handle, &token))
	{
	    /* chain identifier to append */
	    ajStrAppendS(id, token);
	}
	ajStrAssignClear(acc);		/* no accession number */
	ajStrAssignS(desc, reststr);
	/* ajDebug("found pref: '%S' id: '%S', acc: '%S' desc: '%S'\n",
	   prefix, *id, *acc, *desc); */
	ajStrDel(&str);
	ajStrDel(&idstr);
	ajStrDel(&reststr);
	ajStrDel(&prefix);
	ajStrDel(&numtoken);
	ajStrDel(&token);
	ajStrTokenDel(&handle);
	return ajTrue;
    }


    if(!strcmp(q,"gb") || !strcmp(q,"emb") || !strcmp(q,"dbj")
       || !strcmp(q,"tpd") || !strcmp(q,"tpd") || !strcmp(q,"tpg")
       || !strcmp(q,"sp") || !strcmp(q,"ref"))
    {
        /* ajDebug("gb,emb,dbj,sp,ref prefix\n"); */
	ajStrAssignS(db, prefix);
	ajStrTokenNextParse(&handle, &token);
	vacc = ajSeqtestIsSeqversion(token);
	if(vacc)
	{
	    ajStrAssignS(sv,token);
	    ajStrAssignS(acc,vacc);
	}
	else if(ajSeqtestIsAccession(token))
	    ajStrAssignS(acc,token);

	if(!ajStrTokenNextParse(&handle, id))
	{
	    /* no ID, reuse accession token */
	    ajStrAssignS(id, token);
	}
	ajStrAssignS(desc, reststr);
	/* ajDebug("found pref: '%S' id: '%S', acc: '%S' desc: '%S'\n",
	   prefix, *id, *acc, *desc); */
	ajStrDel(&str);
	ajStrDel(&idstr);
	ajStrDel(&reststr);
	ajStrDel(&prefix);
	ajStrDel(&numtoken);
	ajStrDel(&token);
	ajStrTokenDel(&handle);
	return ajTrue;
    }


    if(!strcmp(q,"pir") || !strcmp(q,"prf"))
    {
	ajStrAssignS(db, prefix);
        /* ajDebug("pir,prf prefix\n"); */
	ajStrTokenNextParse(&handle, id);
	ajStrAssignS(desc, reststr);
	ajStrAssignClear(acc);
	/* ajDebug("found pref: '%S' id: '%S', acc: '%S' desc: '%S'\n",
	   prefix, *id, *acc, *desc); */
	ajStrDel(&str);
	ajStrDel(&idstr);
	ajStrDel(&reststr);
	ajStrDel(&prefix);
	ajStrDel(&numtoken);
	ajStrDel(&token);
	ajStrTokenDel(&handle);
	return ajTrue;
    }


    /* else assume that the last two barred tokens contain [acc]|id */

    ajDebug("No prefix accepted - try the last 2 fields\n");

    nt = ajStrParseCountC(idstr,"|");
    if(ajStrGetCharLast(idstr) == '|')
      nt++;

    ajDebug("Barred tokens - %d found\n", nt);

    if(nt < 2)
    {
	ajStrDel(&str);
	ajStrDel(&idstr);
	ajStrDel(&reststr);
	ajStrDel(&prefix);
	ajStrDel(&numtoken);
	ajStrDel(&token);
	ajStrTokenDel(&handle);
	return ajFalse;
    }

    /* restart parsing with only bars */

    ajStrTokenAssignC(&handle,idstr,"|");
    for(i=0;i<nt-2;++i)
      ajStrTokenNextParse(&handle, &token);

    ajStrAssignS(db, token);
    ajStrTokenNextParse(&handle, &token);
    ajDebug("token acc: '%S'\n", token);
    vacc = ajSeqtestIsSeqversion(token);
    if(vacc)
    {
	ajStrAssignS(sv,token);
	ajStrAssignS(acc,vacc);
	ajStrAssignS(id,vacc);
    }
    else if(ajSeqtestIsAccession(token))
    {
        ajStrAssignS(acc,token);
        ajStrAssignS(id,vacc);
    }

    if(ajStrTokenNextParseC(&handle, " \n\t\r", &token))
    {
       ajDebug("token id: '%S'\n", token);
       if(ajStrGetLen(token))
	 ajStrAssignS(id,token);
    }

    ajStrTokenNextParseC(&handle, "\n\r", &token);
    ajStrAssignS(desc, reststr);
    ajStrTokenDel(&handle);
    ajStrDel(&token);
    /* ajDebug("found pref: '%S' id: '%S', acc: '%S' desc: '%S'\n",
       prefix, *id, *acc, *desc); */

    ajStrDel(&str);
    ajStrDel(&idstr);
    ajStrDel(&reststr);
    ajStrDel(&prefix);
    ajStrDel(&numtoken);
    ajStrDel(&token);

    return ajTrue;
}




/* @func ajSeqGetFromUsa ******************************************************
**
** Returns a sequence given a USA
**
** @param [r] thys [const AjPStr] USA
** @param [r] protein [AjBool] True if protein
** @param [w] seq [AjPSeq*] sequence
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajSeqGetFromUsa(const AjPStr thys, AjBool protein, AjPSeq *seq)
{
    AjPSeqin seqin;
    AjBool ok;

    seqin        = ajSeqinNew();
    seqin->multi = ajFalse;
    seqin->Text  = ajFalse;

    if(!protein)
	ajSeqinSetNuc(seqin);
    else
	ajSeqinSetProt(seqin);

    ajSeqinUsa(&seqin, thys);
    ok = ajSeqRead(*seq, seqin);
    ajSeqinDel(&seqin);

    if(!ok)
	return ajFalse;

    return ajTrue;
}




/* @func ajSeqsetGetFromUsa *****************************************
**
** Return a seqset given a usa
**
** @param [r] thys [const AjPStr] usa
** @param [w] seq [AjPSeqset*] seqset
** @return [AjBool] ajTrue on success
******************************************************************************/

AjBool ajSeqsetGetFromUsa(const AjPStr thys, AjPSeqset *seq)
{
    AjPSeqin seqin;
    AjBool ok;

    seqin        = ajSeqinNew();
    seqin->multi = ajTrue;
    seqin->Text  = ajFalse;

    ajSeqinUsa(&seqin, thys);
    ok = ajSeqsetRead(*seq, seqin);
    ajSeqinDel(&seqin);


    if(!ok)
	return ajFalse;

    return ajTrue;
}




/* @funcstatic seqTextSeq *****************************************************
**
** Saves a sequence from a string into the text output pointer
**
** Could do some extra formatting here (left margin, numbering)
** but as the EMBOSS formats are not too fussy that can wait.
**
** @param [w] textptr [AjPStr*] Text output
** @param [r] seq [const AjPStr] sequence as a string
** @return [void]
******************************************************************************/

static void seqTextSeq(AjPStr* textptr, const AjPStr seq)
{
    ajuint i;
    ajuint istart;
    ajuint iend;
    ajuint ilen;
    ajuint iwidth;
    AjPStr tmpstr = NULL;

    ilen = ajStrGetLen(seq);
    iwidth = 60;

    for(i=0; i < ilen; i += iwidth)
    {
	istart = i;
	iend = AJMIN(ilen-1, istart+iwidth-1);
	ajStrAssignSubS(&tmpstr, seq, istart, iend);
	ajFmtPrintAppS(textptr, "%S\n", tmpstr);
    }
    ajStrDel(&tmpstr);

    return;
}




/* @func ajSeqReadExit ********************************************************
**
** Cleans up sequence reading internal memory
**
** @return [void]
** @@
******************************************************************************/

void ajSeqReadExit(void)
{
    ajRegFree(&seqRegQryWild);

    /* USA processing regular expressions */

    ajRegFree(&seqRegUsaAsis);
    ajRegFree(&seqRegUsaDb);
    ajRegFree(&seqRegUsaFmt);
    ajRegFree(&seqRegUsaId);
    ajRegFree(&seqRegUsaList);
    ajRegFree(&seqRegUsaRange);
    ajRegFree(&seqRegUsaWild);

    /* sequence reading regular expressions */

    ajRegFree(&seqRegTreeconTop);
    ajRegFree(&seqRegMegaFeat);
    ajRegFree(&seqRegMegaSeq);
    ajRegFree(&seqRegJackTop);
    ajRegFree(&seqRegJackSeq);
    ajRegFree(&seqRegGffTyp);
    ajRegFree(&seqRegGcgDot);
    ajRegFree(&seqRegGcgChk);
    ajRegFree(&seqRegGcgLen);
    ajRegFree(&seqRegGcgNam);
    ajRegFree(&seqRegGcgTyp);
    ajRegFree(&seqRegGcgMsf);
    ajRegFree(&seqRegGcgMsflen);
    ajRegFree(&seqRegGcgMsfnam);
    ajRegFree(&seqRegGcgWgt);
    ajRegFree(&seqRegNbrfId);
    ajRegFree(&seqRegStadenId);
    ajRegFree(&seqRegHennigBlank);
    ajRegFree(&seqRegHennigSeq);
    ajRegFree(&seqRegHennigTop);
    ajRegFree(&seqRegHennigHead);
    ajRegFree(&seqRegFitchHead);
    ajRegFree(&seqRegStockholmSeq);
    ajRegFree(&seqRegAbiDots);
    ajRegFree(&seqRegRawNonseq);
    ajRegFree(&seqRegPhylipTop);
    ajRegFree(&seqRegPhylipHead);
    ajRegFree(&seqRegPhylipSeq);
    ajRegFree(&seqRegPhylipSeq2);

    /* sequence reading strings */
    ajStrDel(&seqFtFmtEmbl);
    ajStrDel(&seqFtFmtGenbank);
    ajStrDel(&seqFtFmtGff);
    ajStrDel(&seqFtFmtPir);
    ajStrDel(&seqFtFmtSwiss);
    ajStrDel(&seqUsaTest);
    ajStrDel(&seqQryChr);
    ajStrDel(&seqQryDb);
    ajStrDel(&seqQryList);
    ajStrDel(&seqAppendRestStr);

    ajStrDel(&seqReadLine);

    return;
}
/* @func ajSeqinTrace *********************************************************
**
** Debug calls to trace the data in a sequence input object.
**
** @param [r] thys [const AjPSeqin] Sequence input object.
** @return [void]
** @@
******************************************************************************/

void ajSeqinTrace(const AjPSeqin thys)
{
    ajDebug("Sequence input trace\n");
    ajDebug( "====================\n\n");
    ajDebug( "  Name: '%S'\n", thys->Name);

    if(ajStrGetLen(thys->Acc))
	ajDebug( "  Accession: '%S'\n", thys->Acc);

    if(ajStrGetLen(thys->Inputtype))
	ajDebug( "  Inputtype: '%S'\n", thys->Inputtype);

    if(ajStrGetLen(thys->Desc))
	ajDebug( "  Description: '%S'\n", thys->Desc);

    if(ajStrGetLen(thys->Inseq))
	ajDebug( "  Inseq len: %d\n", ajStrGetLen(thys->Inseq));

    if(thys->Rev)
	ajDebug( "     Rev: %B\n", thys->Rev);

    if(thys->Begin)
	ajDebug( "   Begin: %d\n", thys->Begin);

    if(thys->End)
	ajDebug( "     End: %d\n", thys->End);

    if(ajStrGetLen(thys->Db))
	ajDebug( "  Database: '%S'\n", thys->Db);

    if(ajStrGetLen(thys->Full))
	ajDebug( "  Full name: '%S'\n", thys->Full);

    if(ajStrGetLen(thys->Date))
	ajDebug( "  Date: '%S'\n", thys->Date);

    if(ajListGetLength(thys->List))
	ajDebug( "  List: (%d)\n", ajListGetLength(thys->List));

    if(thys->Filebuff)
	ajDebug( "  Filebuff: %F (%Ld)\n",
		ajFilebuffGetFile(thys->Filebuff),
		ajFileResetPos(ajFilebuffGetFile(thys->Filebuff)));

    if(ajStrGetLen(thys->Usa))
	ajDebug( "  Usa: '%S'\n", thys->Usa);

    if(ajStrGetLen(thys->Ufo))
	ajDebug( "  Ufo: '%S'\n", thys->Ufo);

    if(thys->Fttable)
	ajDebug( "  Fttable: exists\n");

    if(thys->Ftquery)
	ajDebug( "  Ftquery: exists\n");

    if(ajStrGetLen(thys->Formatstr))
	ajDebug( "  Input format: '%S' (%d)\n", thys->Formatstr,
		thys->Format);

    if(ajStrGetLen(thys->Filename))
	ajDebug( "  Filename: '%S'\n", thys->Filename);

    if(ajStrGetLen(thys->Entryname))
	ajDebug( "  Entryname: '%S'\n", thys->Entryname);

    if(thys->Search)
	ajDebug( "  Search: %B\n", thys->Search);

    if(thys->Single)
	ajDebug( "  Single: %B\n", thys->Single);

    if(thys->Features)
	ajDebug( "  Features: %B\n", thys->Features);

    if(thys->IsNuc)
	ajDebug( "  IsNuc: %B\n", thys->IsNuc);

    if(thys->IsProt)
	ajDebug( "  IsProt: %B\n", thys->IsProt);

    if(thys->Count)
	ajDebug( "  Count: %d\n", thys->Count);

    if(thys->Filecount)
	ajDebug( "  Filecount: %d\n", thys->Filecount);

    if(thys->Fpos)
	ajDebug( "  Fpos: %l\n", thys->Fpos);

    if(thys->Query)
	ajSeqQueryTrace(thys->Query);

    if(thys->Data)
	ajDebug( "  Data: exists\n");

    if(ajStrGetLen(thys->Doc))
	ajDebug( "  Documentation:...\n%S\n", thys->Doc);

    return;
}




/* @funcstatic stockholmNew ***************************************************
**
** Creates and initialises a Stockholm object.
**
** @param [r] i [ajuint] Number of sequences
** @return [SeqPStockholm] New sequence object.
** @@
******************************************************************************/

static SeqPStockholm stockholmNew(ajuint i)
{
    SeqPStockholm thys = NULL;

    AJNEW0(thys);

    thys->id  = ajStrNew();
    thys->ac  = ajStrNew();
    thys->de  = ajStrNew();
    thys->au  = ajStrNew();
    thys->al  = ajStrNew();
    thys->tp  = ajStrNew();
    thys->se  = ajStrNew();
    thys->bm  = ajStrNew();
    thys->dc  = ajStrNew();
    thys->dr  = ajStrNew();
    thys->cc  = ajStrNew();
    thys->gs  = ajStrNew();
    thys->ref = ajStrNew();
    thys->sacons  = ajStrNew();
    thys->sscons  = ajStrNew();

    thys->n = i;

    AJCNEW0(thys->name,i);
    AJCNEW0(thys->str,i);

    for(i=0;i<thys->n;++i)
    {
	thys->name[i] = ajStrNew();
	thys->str[i]  = ajStrNew();
    }

    return thys;
}




/* #funcstatic stockholmdataNew ***********************************************
**
** Creates and initialises a Stockholm data object.
**
** #return [SeqPStockholmdata] New sequence object.
** ##
******************************************************************************/

/*static SeqPStockholmdata stockholmdataNew(void)
{
    SeqPStockholmdata thys = NULL;

    AJNEW0(thys);

    thys->id  = ajStrNew();
    thys->ac  = ajStrNew();
    thys->de  = ajStrNew();
    thys->au  = ajStrNew();
    thys->al  = ajStrNew();
    thys->tp  = ajStrNew();
    thys->se  = ajStrNew();
    thys->bm  = ajStrNew();
    thys->dc  = ajStrNew();
    thys->dr  = ajStrNew();
    thys->cc  = ajStrNew();
    thys->gs  = ajStrNew();
    thys->ref = ajStrNew();
    thys->sacons  = ajStrNew();
    thys->sscons  = ajStrNew();

    return thys;
}*/


/* @funcstatic stockholmDel ***************************************************
**
** Deletes a Stockholm object.
**
** @param [d] Pseq [SeqPStockholm*] Stockholm object
** @return [void]
** @@
******************************************************************************/

static void stockholmDel(SeqPStockholm *Pseq)
{
    SeqPStockholm pthis = NULL;
    ajuint i;

    if(!Pseq)
	return;
    pthis = *Pseq;
    if(!pthis)
	return;

    ajStrDel(&pthis->id);
    ajStrDel(&pthis->ac);
    ajStrDel(&pthis->de);
    ajStrDel(&pthis->au);
    ajStrDel(&pthis->al);
    ajStrDel(&pthis->tp);
    ajStrDel(&pthis->se);
    ajStrDel(&pthis->bm);
    ajStrDel(&pthis->dc);
    ajStrDel(&pthis->dr);
    ajStrDel(&pthis->cc);
    ajStrDel(&pthis->gs);
    ajStrDel(&pthis->ref);
    ajStrDel(&pthis->sacons);
    ajStrDel(&pthis->sscons);

    for(i=0;i<pthis->n;++i)
    {
	ajStrDel(&pthis->name[i]);
	ajStrDel(&pthis->str[i]);
    }

    AJFREE(pthis->name);
    AJFREE(pthis->str);
    AJFREE(*Pseq);

    return;
}




/* #funcstatic stockholmdataDel ***********************************************
**
** Deletes a Stockholm data object.
**
** #param [d] Pseq [SeqPStockholmdata*] Stockholm object
** #return [void]
** ##
******************************************************************************/

/*static void stockholmdataDel(SeqPStockholmdata *Pseq)
{
    SeqPStockholmdata pthis = NULL;

    if(!Pseq)
	return;
    pthis = *Pseq;
    if(!pthis)
	return;

    ajStrDel(&pthis->id);
    ajStrDel(&pthis->ac);
    ajStrDel(&pthis->de);
    ajStrDel(&pthis->au);
    ajStrDel(&pthis->al);
    ajStrDel(&pthis->tp);
    ajStrDel(&pthis->se);
    ajStrDel(&pthis->bm);
    ajStrDel(&pthis->dc);
    ajStrDel(&pthis->dr);
    ajStrDel(&pthis->cc);
    ajStrDel(&pthis->gs);
    ajStrDel(&pthis->ref);
    ajStrDel(&pthis->sacons);
    ajStrDel(&pthis->sscons);

    AJFREE(*Pseq);

    return;
}*/


/* @funcstatic selexNew *******************************************************
**
** Creates and initialises a selex #=SQ line object.
**
** @param [r] n [ajuint] Number of sequences
** @return [SeqPSelex] New sequence object.
** @@
******************************************************************************/

static SeqPSelex selexNew(ajuint n)
{
    SeqPSelex thys = NULL;
    ajuint    i;

    AJNEW0(thys);
    thys->id = ajStrNew();
    thys->ac = ajStrNew();
    thys->de = ajStrNew();
    thys->au = ajStrNew();
    thys->cs = ajStrNew();
    thys->rf = ajStrNew();
    thys->n  = n;

    AJCNEW(thys->name,n);
    AJCNEW(thys->str,n);
    AJCNEW(thys->ss,n);
    AJCNEW(thys->sq,n);

    for(i=0;i<n;++i)
    {
	thys->name[i] = ajStrNew();
	thys->str[i]  = ajStrNew();
	thys->ss[i]   = ajStrNew();
	thys->sq[i]   = selexseqNew();
    }

    return thys;
}




/* @funcstatic selexseqNew ****************************************************
**
** Creates and initialises a selex #=SQ line object.
**
** @return [SeqPSelexseq] New sequence object.
** @@
******************************************************************************/

static SeqPSelexseq selexseqNew(void)
{
    SeqPSelexseq thys = NULL;

    AJNEW0(thys);

    thys->name   = ajStrNew();
    thys->source = ajStrNew();
    thys->ac     = ajStrNew();
    thys->de     = ajStrNew();

    return thys;
}




/* #funcstatic selexdataNew ***************************************************
**
** Creates and initialises a selex #=SQ line object.
**
** #return [SeqPSelexdata] New sequence object.
** ##
******************************************************************************/

/*static SeqPSelexdata selexdataNew(void)
{
    SeqPSelexdata thys = NULL;

    AJNEW0(thys);
    thys->id = ajStrNew();
    thys->ac = ajStrNew();
    thys->de = ajStrNew();
    thys->au = ajStrNew();
    thys->cs = ajStrNew();
    thys->rf = ajStrNew();

    thys->name = ajStrNew();
    thys->str  = ajStrNew();
    thys->ss   = ajStrNew();
    thys->sq   = selexseqNew();

    return thys;
}*/


/* @funcstatic selexseqDel ****************************************************
**
** Deletes a Selex object.
**
** @param [d] Pseq [SeqPSelexseq*] Selex #=SQ object
** @return [void]
** **
******************************************************************************/

static void selexseqDel(SeqPSelexseq *Pseq)
{
    SeqPSelexseq pthis;

    pthis = *Pseq;

    if(!Pseq || !pthis)
	return;

    ajStrDel(&pthis->name);
    ajStrDel(&pthis->source);
    ajStrDel(&pthis->ac);
    ajStrDel(&pthis->de);

    AJFREE(pthis);
    *Pseq = NULL;

    return;
}




/* @funcstatic selexDel *******************************************************
**
** Deletes a Selex object.
**
** @param [d] Pseq [SeqPSelex*] Selex object
** @return [void]
** @@
******************************************************************************/

static void selexDel(SeqPSelex *Pseq)
{
    SeqPSelex pthis;
    ajuint    i;
    ajuint    n;

    pthis = *Pseq;

    if(!Pseq || !pthis)
	return;

    n = pthis->n;
    for(i=0;i<n;++i)
    {
	ajStrDel(&pthis->name[i]);
	ajStrDel(&pthis->str[i]);
	ajStrDel(&pthis->ss[i]);
	selexseqDel(&pthis->sq[i]);
    }

    if(n)
    {
	AJFREE(pthis->name);
	AJFREE(pthis->str);
	AJFREE(pthis->ss);
	AJFREE(pthis->sq);
    }

    ajStrDel(&pthis->id);
    ajStrDel(&pthis->ac);
    ajStrDel(&pthis->de);
    ajStrDel(&pthis->au);
    ajStrDel(&pthis->cs);
    ajStrDel(&pthis->rf);

    AJFREE(pthis);
    *Pseq = NULL;

    return;
}




/* #funcstatic selexdataDel ***************************************************
**
** Deletes a Selex data object.
**
** #param [d] Pseq [SeqPSelexdata*] Selex data object
** #return [void]
** ##
******************************************************************************/

/*static void selexdataDel(SeqPSelexdata *Pseq)
{
    SeqPSelexdata pthis;

    pthis = *Pseq;

    if(!Pseq || !pthis)
	return;


    ajStrDel(&pthis->name);
    ajStrDel(&pthis->str);
    ajStrDel(&pthis->ss);
    selexseqDel(&pthis->sq);

    ajStrDel(&pthis->id);
    ajStrDel(&pthis->ac);
    ajStrDel(&pthis->de);
    ajStrDel(&pthis->au);
    ajStrDel(&pthis->cs);
    ajStrDel(&pthis->rf);

    AJFREE(pthis);
    *Pseq = NULL;

    return;
}*/




/* #funcstatic seqSelexClone *************************************************
**
** Clone a Selexdata object
**
** #param [r] thys [const SeqPSelexdata] selex data object
**
** #return [SeqPSelexdata] New selex data object.
** ##
******************************************************************************/

/*static SeqPSelexdata seqSelexClone(const SeqPSelexdata thys)
{
    SeqPSelexdata pthis;

    pthis = selexdataNew();

    ajStrAssignS(&pthis->id, thys->id);
    ajStrAssignS(&pthis->ac, thys->ac);
    ajStrAssignS(&pthis->de, thys->de);
    ajStrAssignS(&pthis->au, thys->au);
    ajStrAssignS(&pthis->cs, thys->cs);
    ajStrAssignS(&pthis->rf, thys->rf);
    ajStrAssignS(&pthis->name, thys->name);
    ajStrAssignS(&pthis->str, thys->str);
    ajStrAssignS(&pthis->ss, thys->ss);

    pthis->ga[0] = thys->ga[0];
    pthis->ga[1] = thys->ga[1];
    pthis->tc[0] = thys->tc[0];
    pthis->tc[1] = thys->tc[1];
    pthis->nc[0] = thys->nc[0];
    pthis->nc[1] = thys->nc[1];

    ajStrAssignS(&pthis->sq->name, thys->sq->name);
    ajStrAssignS(&pthis->sq->source, thys->sq->source);
    ajStrAssignS(&pthis->sq->ac, thys->sq->ac);
    ajStrAssignS(&pthis->sq->de, thys->sq->de);

    pthis->sq->wt    = thys->sq->wt;
    pthis->sq->start = thys->sq->start;
    pthis->sq->stop  = thys->sq->stop;
    pthis->sq->len   = thys->sq->len;


    return pthis;
}*/




/* @funcstatic seqDefine ******************************************************
**
** Make sure all sequence object attributes are defined
** using valued from the sequence input object if needed
**
** @param [w] thys [AjPSeq] Sequence returned.
** @param [u] seqin [AjPSeqin] Sequence input definitions
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqDefine(AjPSeq thys, AjPSeqin seqin)
{

    /* if values are missing in the sequence object, we can use defaults
       from seqin or calculate where possible */

    ajDebug("seqDefine: thys->Db '%S', seqin->Db '%S'\n",
	  thys->Db, seqin->Db);
    ajDebug("seqDefine: thys->Name '%S' type: %S\n",
	    thys->Name, thys->Type);
    ajDebug("seqDefine: thys->Entryname '%S', seqin->Entryname '%S'\n",
	    thys->Entryname, seqin->Entryname);

    /* assign the dbname and entryname if defined in the seqin object */
    if(ajStrGetLen(seqin->Db))
      ajStrAssignS(&thys->Db, seqin->Db);
    if(ajStrGetLen(seqin->Entryname))
    ajStrAssignEmptyS(&thys->Entryname, seqin->Entryname);
    if(ajStrGetLen(thys->Entryname))
      ajStrAssignS(&thys->Name, thys->Entryname);

    ajDebug("seqDefine: returns thys->Name '%S' type: %S\n",
	    thys->Name, thys->Type);

    if(!ajStrGetLen(thys->Type))
	ajSeqType(thys);
    
    return ajTrue;
}


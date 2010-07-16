/* @source ********************************************************************
**
** a genome feature (in AJAX program context) is a description of a
** genomic entity which was determined by some 'source' analysis
** (which may be of 'wet lab' experimental or 'in silico'
** computational nature), has a 'primary' descriptor ('Primary_Tag'),
** may have some 'score' asserting the level of analysis confidence in
** its identity (e.g. log likelihood relative to a null hypothesis or
** other similar entity), has a 'Position' in the genome, and may have
** any arbitrary number of descriptor tags associated with it.
**
** @author Copyright (C) 1999 Richard Bruskiewich
** modified by Ian Longden.
** modified by Peter Rice.
** @version 3.0
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
**
** Implementation Notes:
**
** see also the header file, ajfeat.h, for details on class structure.
**
** FT6.2 todo alternate mandatory qualifiers /citation /compare
**            labelled with m/ in Efeatures.*
** FT6.2 todo /compare special value accession.version
** FT6.2 todo check spaces in /anticodon /codon and other special values
**            when used in practice
** FT6.2 todo warning for /cons_splice with YES YES
** FT6.2 todo list of db_xref databases from
**            http://www.ncbi.nlm.nih.gov/projects/collab/db_xref.html
** FT6.2 todo list of /country country names from
**            http://www.ncbi.nlm.nih.gov/projects/collab/country.html
** FT6.2 todo EC_number value check
** FT6.2 todo /frequency value check 0.0 to 1.0
** FT6.2 todo QWORD values with no spaces /translation /locus_tag
**            /old_locus_tag
** FT6.2 todo vocab for mod_base ... rel_std*.dat Aug-09 has
**     794 OTHER
**    1475 p
**     971 d
**     331 m5c
**     300 m1a
**     260 t
**     242 m7g
**     241 m2g
**     215 gm
**     202 m1g
**     186 m22g
**     153 cm
**     102 um
**      93 t6a
**      55 s4u
**      52 i
**      52 ac4c
**      49 m1f
**      48 m1i
**      41 x
**      41 i6a
**      33 m3c
**      31 ms2i6a
**      31 m6a
**      27 q
**      22 m2a
**      17 tm
**      11 mt6a
**      11 mcm5s2u
**       8 yw
**       6 s2t
**       6 osyw
**       6 fm
**       6 cmnm5u
**       5 s2c
**       5 mam5s2u
**       4 mo5u
**       4 mcm5u
**       3 o5u
**       3 ms2t6a
**
** FT6.2 todo *qualifier for non-standard qualifiers (for Artemis)
******************************************************************************/

#include "ajax.h"




/* @datastatic FeatPTagval ****************************************************
**
** Feature tag values data structure
**
** @alias FeatSTagval
** @alias FeatOTagval
**
** @attr Tag [AjPStr] Tag name usually from a controlled internal vocabulary
** @attr Value [AjPStr] Tag value
** @@
******************************************************************************/

typedef struct FeatSTagval
{
    AjPStr Tag;
    AjPStr Value;
} FeatOTagval;
#define FeatPTagval FeatOTagval*




#define FEATFLAG_START_BEFORE_SEQ 0x0001 /* <start */
#define FEATFLAG_END_AFTER_SEQ    0x0002 /* >end */
#define FEATFLAG_CHILD            0x0004 /* exon for join() */
#define FEATFLAG_BETWEEN_SEQ      0x0008  /* x^y */
#define FEATFLAG_START_TWO        0x0010  /* x.y.. */
#define FEATFLAG_END_TWO          0x0020  /* ..x.y */
#define FEATFLAG_POINT            0x0040  /* x */
#define FEATFLAG_COMPLEMENT_MAIN  0x0080  /* complement around the join */
#define FEATFLAG_MULTIPLE         0x0100  /* part of a multiple i.e. join*/
#define FEATFLAG_GROUP            0x0200  /* group */
#define FEATFLAG_ORDER            0x0400  /* order */
#define FEATFLAG_ONEOF            0x0800  /* one_of */
#define FEATFLAG_REMOTEID         0x1000  /* AB012345.6: */
#define FEATFLAG_LABEL            0x2000  /* label as location */
#define FEATFLAG_START_UNSURE     0x4000  /* unsure position - SwissProt '?' */
#define FEATFLAG_END_UNSURE       0x8000  /* unsure position - SwissProt '?' */

static AjBool   FeatInitDone         = AJFALSE;
static AjPTable FeatTypeTableDna     = NULL;
static AjPTable FeatTagsTableDna     = NULL;
static AjPTable FeatTypeTableProtein = NULL;
static AjPTable FeatTagsTableProtein = NULL;

static AjBool   FeatInitGff      = AJFALSE;
static AjPTable FeatTypeTableGff = NULL;
static AjPTable FeatTagsTableGff = NULL;
static AjPTable FeatTypeTableGffprotein = NULL;
static AjPTable FeatTagsTableGffprotein = NULL;

static AjBool   FeatInitGff3      = AJFALSE;
static AjPTable FeatTypeTableGff3 = NULL;
static AjPTable FeatTagsTableGff3 = NULL;
static AjPTable FeatTypeTableGff3protein = NULL;
static AjPTable FeatTagsTableGff3protein = NULL;

static AjBool   FeatInitEmbl      = AJFALSE;
static AjPTable FeatTypeTableEmbl = NULL;
static AjPTable FeatTagsTableEmbl = NULL;

static AjBool   FeatInitPir      = AJFALSE;
static AjPTable FeatTypeTablePir = NULL;
static AjPTable FeatTagsTablePir = NULL;

static AjBool   FeatInitSwiss      = AJFALSE;
static AjPTable FeatTypeTableSwiss = NULL;
static AjPTable FeatTagsTableSwiss = NULL;

static AjBool   FeatInitRefseqp    = AJFALSE;
static AjPTable FeatTypeTableRefseqp = NULL;
static AjPTable FeatTagsTableRefseqp = NULL;

static AjPTable FeatCategoryTable     = NULL;
static ajint featWarnCount = 0;

static AjPStr featTypeMiscfeat   = NULL;
static AjPStr featTypeEmpty      = NULL;
static AjPStr featDefSource = NULL;
static AjPStr featFmtTmp = NULL;
static AjPStr featLimTmp = NULL;
static AjPStr featTagTmp = NULL;
static AjPStr featTagTmp2 = NULL;
static AjPStr featValTmp = NULL;
static AjPStr featValTmp2 = NULL;
static AjPStr featUfoTest = NULL;
static AjPStr featFormatTmp = NULL;
static AjPStr featOutStr = NULL;
static AjPStr featReadLine   = NULL;
static AjPStr featProcessLine   = NULL;
static AjPStr featTmpStr  = NULL;
static AjPStr featValCopy     = NULL;
static AjPStr featSubStr      = NULL;
static AjPStr featSourcePir      = NULL;
static AjPStr featSourceSwiss    = NULL;
static AjPStr featSourceRefseqp  = NULL;
static AjPStr featSourceEmbl     = NULL;
static AjPStr featGroup = NULL ;
static AjPStr featId = NULL;
static AjPStr featLabel   = NULL;

static AjPStr featSource    = NULL;
static AjPStr featFeature   = NULL;
static AjPStrTok featGffSplit  = NULL;
static AjPStrTok featEmblSplit  = NULL;
static AjPStrTok featVocabSplit  = NULL;

static AjPStr featLocStr   = NULL;
static AjPStr featLocToken = NULL;
static AjPStr featLocDb = NULL;
static AjPStr featSaveGroupStr   = NULL;
static AjPStr featTagNote = NULL;
static AjPStr featTagComm = NULL;
static AjPStr featTagFtid = NULL;

static AjPStr featTransBegStr = NULL;
static AjPStr featTransEndStr = NULL;
static AjPStr featTransAaStr  = NULL;

static ajint featSaveExon  = 0;
static ajint featSaveGroup = 0;
 
static void         featCleanDasgff(AjPFile file);
static void         featClear(AjPFeature thys );
static ajint        featCompByEnd(const void *a, const void *b);
static ajint        featCompByGroup(const void *a, const void *b);
static ajint        featCompByStart(const void *a, const void *b);
static ajint        featCompByType(const void *a, const void *b);
static AjBool       featDelRegEmbl(void);
static AjBool       featDelRegGff(void);
static AjBool       featDelRegGff3(void);
static AjBool       featDelRegPir(void);
static AjBool       featDelRegRefseqp(void);
static AjBool       featDelRegSwiss(void);
static void         featDumpEmbl(const AjPFeature thys, const AjPStr location,
				 AjPFile file, const AjPStr Seqid,
				 AjBool IsEmbl);
static void         featDumpRefseq(const AjPFeature thys,
                                   const AjPStr location,
                                   AjPFile file, const AjPStr Seqid);
static void         featDumpRefseqp(const AjPFeature thys,
                                    const AjPStr location,
                                    AjPFile file, const AjPStr Seqid);
static void         featDumpGff2(const AjPFeature thys,
				 const AjPFeattable owner,
				 AjPFile file);
static void         featDumpGff3(const AjPFeature thys,
				 const AjPFeattable owner,
				 AjPFile file);
static void         featDumpPir(const AjPFeature thys, const AjPStr location,
				AjPFile file);
static void         featDumpSwiss(const AjPFeature thys, AjPFile file,
				  const AjPFeature gftop);
static AjBool       featEmblLoc(const AjPStr loc,
				AjPStr* begstr, AjBool* between,
				AjBool* simple, AjPStr* endstr);
static AjBool       featEmblLocNum(const AjPStr loc,
				   AjBool* bound, ajint* num);
static AjBool       featEmblLocRange(const AjPStr loc,
				     ajint* num1, ajint* num2);
static AjBool       featEmblOperIn(const AjPStr loc, AjPStr* opnam,
				   AjPStr* value, AjPStr* rest);
static AjBool       featEmblOperNone(const AjPStr loc, AjPStr* entryid,
				     AjPStr* value, AjPStr* rest);
static AjBool       featEmblOperOut(const AjPStr loc,
				    AjPStr* opnam, AjPStr* value);
static AjBool       featEmblTvRest(AjPStr* tags, AjPStr* skip);
static AjBool       featEmblTvTagVal(AjPStr* tags, AjPStr* tag, AjPStr* value);
static AjPFeature   featFeatNew(AjPFeattable thys,
				const AjPStr source,
				const AjPStr type,
				ajint Start, ajint End,
				float score,
				char  strand,
				ajint frame,
				ajint exon,
				ajint Start2, ajint End2,
				const AjPStr entryid,
				const AjPStr label,
				ajint flags );
static AjPFeature   featFeatNewProt(AjPFeattable thys,
				    const AjPStr source,
				    const AjPStr type,
				    ajint Start,
				    ajint  End,
				    float score,
				    ajint flags );
static AjBool       featFeatType(const AjPStr line, AjPStr* type,
				 AjPStr* sofaid, AjPStr* tag, AjPStr* req);
static AjPFeature   featFeatureNew(void);
static AjBool       featFindInFormatC(const char* format, ajint* iformat);
static AjBool       featFindInFormatS(const AjPStr format, ajint* iformat);
static AjBool       featFindOutFormatC(const char* format, ajint* iformat);
static AjBool       featFindOutFormatS(const AjPStr format, ajint* iformat);
static void         featFlagSet(AjPFeature gf, const AjPStr flags);
static AjBool       featFormatSet(AjPFeattabIn featin);
static char         featFrame(ajint frame);
static char         featFrameNuc(ajint frame);
static void         featGroupSet(AjPFeature gf, AjPFeattable table,
				 const AjPStr grouptag);
static void         featInit(void);
static void         featLocEmblWrapC(AjPStr* pval, ajuint margin,
				     const char* prefix, const char* preftyp,
				     AjPStr* retstr);
static AjBool       featoutUfoProcess(AjPFeattabOut thys, const AjPStr ufo);
static AjPFeature   featPirFromLine(AjPFeattable thys,
				    const AjPStr origline);
static AjBool       featReadEmbl(AjPFeattable thys, AjPFilebuff file);
static AjBool       featReadGenpept(AjPFeattable thys, AjPFilebuff file);
static AjBool       featReadRefseq(AjPFeattable thys, AjPFilebuff file);
static AjBool       featReadRefseqp(AjPFeattable thys, AjPFilebuff file);
static AjBool       featReadGff(AjPFeattable thys, AjPFilebuff file);
static AjBool       featReadGff3(AjPFeattable thys, AjPFilebuff file);
static AjBool       featReadPir(AjPFeattable thys, AjPFilebuff file);
static AjBool       featReadSwiss(AjPFeattable thys, AjPFilebuff file);
static AjBool       featRegInitEmbl(void);
static AjBool       featRegInitGff(void);
static AjBool       featRegInitGff3(void);
static AjBool       featRegInitPir(void);
static AjBool       featRegInitRefseqp(void);
static AjBool       featRegInitSwiss(void);
static AjBool       featVocabInitEmbl(void);
static AjBool       featVocabInitGff(void);
static AjBool       featVocabInitGffprotein(void);
static AjBool       featVocabInitGff3(void);
static AjBool       featVocabInitGff3protein(void);
static AjBool       featVocabInitPir(void);
static AjBool       featVocabInitRefseqp(void);
static AjBool       featVocabInitSwiss(void);
static char         featStrand(ajint strand);
static AjPFeature   featSwissFromLine(AjPFeattable thys, const AjPStr line,
				      AjPStr* savefeat, AjPStr* savefrom,
				      AjPStr* saveto, AjPStr* saveline);
static AjPFeature   featSwissProcess(AjPFeattable thys, const AjPStr feature,
				     const AjPStr fromstr, const AjPStr tostr,
				     const AjPStr source,
				     const AjPStr tags);
static void         featTableInit(AjPFeattable thys,
				  const AjPStr name);
static AjPFeattable featTableNew(void);
static AjPFeattable featTableNewS(const AjPStr name);
static const AjPStr featTableTag(const AjPStr tag, const AjPTable table,
				 AjBool* known);
static const AjPStr featTableTagC(const char *tag, const AjPTable table,
				 AjBool* known);
static const AjPStr featTableTypeExternal(const AjPStr type,
					  const AjPTable table);
static const AjPStr featTableTypeInternal(const AjPStr type,
					  const AjPTable table);
static const AjPStr featTableTypeInternalLimit(const AjPStr type,
                                               const AjPTable table);
static AjBool       feattableWriteEmbl(const AjPFeattable Feattab,
				       AjPFile file,
				       AjBool IsEmbl);
static AjBool       feattableWriteRefseq(const AjPFeattable Feattab,
                                         AjPFile file);
static AjBool       feattableWriteRefseqp(const AjPFeattable Feattab,
                                          AjPFile file);
static AjBool       featTagAllLimit(AjPStr* pval, const AjPStr values);
static void         featTagDasgffDefault(AjPStr* pout, const AjPStr tag,
                                         AjPStr* pval);
static const AjPStr featTagDna(const AjPStr type, AjBool* known);
static void         featTagEmblDefault(AjPStr* pout,
				       const AjPStr tag, AjPStr* pval);
static void         featTagEmblWrapC(AjPStr* pval, ajuint margin,
				     const char* prefix,
				     AjPStr* retstr);
static void         featTagFmt(const AjPStr name, const AjPTable table,
			       AjPStr* retstr);
static void         featTagGff2Default(AjPStr* pout, const AjPStr tag,
				       AjPStr* pval);
static void         featTagGff3Default(AjPStr* pout, const AjPStr tag,
				       AjPStr* pval);
static AjBool       featTagGffSpecial(AjPStr* pval, const AjPStr tag);
static AjBool       featTagGff3Special(AjPStr* pval, const AjPStr tag);
static void         featTagLimit(const AjPStr name, const AjPTable table,
				 AjPStr* retstr);
static AjBool       featTagName(const AjPStr line, AjPStr* name, AjPStr* type,
				AjPStr* rest);
static const AjPStr featTagProt(const AjPStr type, AjBool* known);
static void         featTagQuoteEmbl(AjPStr* pval);
static void         featTagQuoteGff2(AjPStr* pval);
static void         featTagQuoteGff3(AjPStr* pval);
static void         featTagSetDefault(AjPFeature thys,
				      const AjPStr tag, const AjPStr value,
				      AjPStr* pdeftag, AjPStr* pdefval);
static void         featTagSetDefaultDna(const AjPStr tag, const AjPStr value,
					 AjPStr* pdeftag, AjPStr* pdefval);
static void         featTagSetDefaultProt(const AjPStr tag,
					  const AjPStr value,
					  AjPStr* pdeftag, AjPStr* pdefval);
static AjBool       featTagSpecial(AjPStr* pval, const AjPStr tag);
static AjBool       featTagSpecialAllAnticodon(const AjPStr val);
static AjBool       featTagSpecialAllBiomaterial(const AjPStr val);
static AjBool       featTagSpecialAllCitation(const AjPStr val);
static AjBool       featTagSpecialAllCodon(AjPStr* pval);
static AjBool       featTagSpecialAllCollectiondate(const AjPStr pval);
static AjBool       featTagSpecialAllConssplice(AjPStr* pval);
static AjBool       featTagSpecialAllInference(const AjPStr pval);
static AjBool       featTagSpecialAllLatlon(const AjPStr pval);
static AjBool       featTagSpecialAllMobile(const AjPStr pval);
static AjBool       featTagSpecialAllPcrprimers(const AjPStr pval);
static AjBool       featTagSpecialAllRptunit(const AjPStr val);
static AjBool       featTagSpecialAllRange(const AjPStr val);
static AjBool       featTagSpecialAllRptunitseq(AjPStr *Pval);
static AjBool       featTagSpecialAllTranslexcept(const AjPStr val);
static AjBool       featTagSpecialAllDbxref(const AjPStr val);
static AjBool       featTagSpecialAllProteinid(const AjPStr val);
static AjBool       featTagSpecialAllReplace(AjPStr* pval);
static AjBool       featTagSpecialAllTranslation(AjPStr* pval);
static AjBool       featTagSpecialAllEstimatedlength(AjPStr* pval);
static AjBool       featTagSpecialAllCompare(const AjPStr val);
static AjBool       featTagSpecialAllNcrnaclass(const AjPStr val);
static void         featTagSwissWrapC(AjPStr* pval, ajuint margin,
				      const char* prefix,
				      AjPStr* retstr);
static FeatPTagval  featTagval(const AjPFeature thys, const AjPStr tag);
static FeatPTagval  featTagvalNew(const AjPFeature thys,
				  const AjPStr tag, const AjPStr value);
static FeatPTagval  featTagvalNewDna(const AjPStr tag, const AjPStr value);
static FeatPTagval  featTagvalNewProt(const AjPStr tag, const AjPStr value);
static const AjPStr featTypeDna(const AjPStr type);
static const AjPStr featTypeDnaLimit(const AjPStr type);
static AjBool       featTypePirIn(AjPStr* type);
static AjBool       featTypePirOut(AjPStr* type);
static const AjPStr featTypeProt(const AjPStr type);
static const AjPStr featTypeProtLimit(const AjPStr type);
static AjBool       featVocabRead(const char *name,
				  AjPTable pTypeTable, AjPTable pTagsTable);
static AjBool       featVocabReadTypes(const AjPStr fname,
				       AjPTable pTypeTable,
				       const AjPTable pTagsTable,
				       AjBool recursion);
static AjBool       featVocabReadTags(const AjPStr fname,
				      AjPTable pTagsTable,
				       AjBool recursion);
static void         featWarn(const char* fmt, ...);
static AjBool featTypeTestDnaWild(const AjPStr type, const AjPStr str);
static AjBool featTypeTestProtWild(const AjPStr type, const AjPStr str);
static AjBool featTableTypeTestWild(const AjPStr type,
                                    const AjPTable table,
                                    const AjPStr str);




/* @datastatic FeatPInFormat **************************************************
**
** Feature input format definition
**
** @alias FeatSInFormat
** @alias FeatOInFormat
**
** @attr Name [const char*] Input format name
** @attr Alias [AjBool] True if name is an alias for an identical definition
** @attr Nucleotide [AjBool] True if suitable for nucleotide data
** @attr Protein [AjBool] True if suitable for protein data
** @attr Used [AjBool] True if already used (initialised)
** @attr Read [(AjBool*)] Function to read feature data
** @attr InitReg [(AjBool*)] Function to initialise regular expressions
** @attr DelReg [(AjBool*)] Function to clean up regular expressions
** @attr Desc [const char*] Description
** @@
******************************************************************************/

typedef struct FeatSInFormat
{
    const char* Name;
    AjBool Alias;
    AjBool Nucleotide;
    AjBool Protein;
    AjBool Used;
    AjBool (*Read)  (AjPFeattable thys, AjPFilebuff file);
    AjBool (*InitReg)(void);
    AjBool (*DelReg)(void);
    const char* Desc;
} FeatOInFormat;
#define FeatPInFormat FeatOInFormat*

/* name             Dna   Protein
   input-function   init-regex-function del-regex-function */




/* @funclist featInFormatDef **************************************************
**
** Input feature formats
**
** Includes the read function (featRead), and initialising (featInitReg)
** and deletion (featDelReg) of parsing regular expression.
**
******************************************************************************/

static FeatOInFormat featInFormatDef[] =
{
    /*Name            Alias    Dna      Prot     Used (initially false)
         ReadFunction      RegInitFunction    RegDelFunction
         Description*/
    {"unknown",       AJFALSE, AJFALSE, AJFALSE, AJFALSE,
	 featReadGff,     NULL,               NULL,
	 "unknown format"},
    {"embl",          AJFALSE, AJTRUE,  AJFALSE, AJFALSE,
	 featReadEmbl,    featRegInitEmbl,    featDelRegEmbl,
	 "embl/genbank/ddbj format"},
    {"em",            AJTRUE,  AJTRUE,  AJFALSE, AJFALSE,
	 featReadEmbl,    featRegInitEmbl,    featDelRegEmbl,
	 "embl/genbank/ddbj format"},
    {"genbank",       AJTRUE,  AJTRUE,  AJFALSE, AJFALSE,
	 featReadEmbl,    featRegInitEmbl,    featDelRegEmbl,
	 "embl/genbank/ddbj format"},
    {"gb",            AJTRUE,  AJTRUE,  AJFALSE, AJFALSE,
	 featReadEmbl,    featRegInitEmbl,    featDelRegEmbl,
	 "embl/genbank/ddbj format"},
    {"ddbj",          AJTRUE,  AJTRUE,  AJFALSE, AJFALSE,
	 featReadEmbl,    featRegInitEmbl,    featDelRegEmbl,
	 "embl/genbank/ddbj format"},
    {"refseq",        AJFALSE, AJTRUE,  AJFALSE, AJFALSE,
	 featReadRefseq,  featRegInitEmbl,    featDelRegEmbl,
	 "embl/genbank/ddbj format"},
    {"refseqp",       AJFALSE, AJFALSE, AJTRUE, AJFALSE,
	 featReadRefseqp, featRegInitRefseqp, featDelRegRefseqp,
	 "RefSeq protein format"},
    {"genpept",       AJFALSE, AJFALSE, AJTRUE,  AJFALSE,
	 featReadGenpept, featRegInitSwiss,    featDelRegSwiss,
	 "genpept format"},
    {"gff3",          AJFALSE, AJTRUE,  AJTRUE,  AJFALSE,
	 featReadGff3,    featRegInitGff3,    featDelRegGff3,
	 "GFF version 3"},
    {"gff2",          AJFALSE, AJTRUE,  AJTRUE,  AJFALSE,
	 featReadGff,     featRegInitGff,     featDelRegGff,
	 "GFF version 1 or 2"},
    {"gff",           AJTRUE,  AJTRUE,  AJTRUE,  AJFALSE,
	 featReadGff3,    featRegInitGff3,    featDelRegGff3,
	 "GFF version3"},
    {"swiss",         AJFALSE, AJFALSE, AJTRUE,  AJFALSE,
	 featReadSwiss,   featRegInitSwiss,   featDelRegSwiss,
	 "SwissProt format"},
    {"sw",            AJTRUE,  AJFALSE, AJTRUE,  AJFALSE,
	 featReadSwiss,   featRegInitSwiss,   featDelRegSwiss,
	 "SwissProt format"},
    {"swissprot",     AJTRUE,  AJFALSE, AJTRUE,  AJFALSE,
	 featReadSwiss,   featRegInitSwiss,   featDelRegSwiss,
	 "SwissProt format"},
    {"pir",           AJFALSE, AJFALSE, AJTRUE,  AJFALSE,
	 featReadPir,     featRegInitPir,     featDelRegPir,
	 "PIR format"},
    {"nbrf",          AJTRUE,  AJFALSE, AJTRUE,  AJFALSE,
	 featReadPir,     featRegInitPir,     featDelRegPir,
	 "PIR format"},
    {NULL, AJFALSE, AJFALSE, AJFALSE, AJFALSE, NULL, NULL, NULL, NULL}
};

static FeatPInFormat featInFormat = featInFormatDef;


static AjPFeature  featEmblFromLine(AjPFeattable thys, const AjPStr line,
				    AjPStr* savefeat,
				    AjPStr* saveloc, AjPStr* saveline);
static AjPFeature  featEmblProcess(AjPFeattable thys, const AjPStr feature,
				   const AjPStr source,
				   AjPStr* loc, AjPStr* tags);
static AjPFeature  featRefseqpFromLine(AjPFeattable thys, const AjPStr line,
                                       AjPStr* savefeat,
                                       AjPStr* saveloc, AjPStr* saveline);
static AjPFeature  featRefseqpProcess(AjPFeattable thys, const AjPStr feature,
                                      const AjPStr source,
                                      AjPStr* loc, AjPStr* tags);
static AjPFeature  featGffFromLine(AjPFeattable thys, const AjPStr line,
				   float version);

static AjPFeature  featGff3FromLine(AjPFeattable thys, const AjPStr line);

static void        featGffProcessTagval(AjPFeature gf,
					AjPFeattable table,
					const AjPStr groupfield,
					float version);

static void        featGff3ProcessTagval(AjPFeature gf,
					AjPFeattable table,
					const AjPStr groupfield);




/* @datastatic FeatPOutFormat *************************************************
**
** Feature output formats
**
** @alias FeatSOutFormat
** @alias FeatOOutFormat
**
** @attr Name [const char*] Format name
** @attr Nucleotide [AjBool] True if suitable for nucleotide data
** @attr Protein [AjBool] True if suitable for protein data
** @attr VocInit [(AjBool*)] Function to initialise vocabulary
** @attr Write [(AjBool*)] Function to write data
** @attr Desc [const char*] Description
** @attr Alias [AjBool] True if name is an alias for an identical definition
** @attr Padding [ajint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct FeatSOutFormat
{
    const char* Name;
    AjBool Nucleotide;
    AjBool Protein;
    AjBool (*VocInit) (void);
    AjBool (*Write) (AjPFeattabOut ftout, const AjPFeattable thys);
    const char* Desc;
    AjBool Alias;
    ajint Padding;
} FeatOOutFormat;
#define FeatPOutFormat FeatOOutFormat*




/* @funclist featOutFormatDef *************************************************
**
** Feature output formats
**
** Includes functions to initialise the internal type/tag tables
** (featVocabInit) - done automatically for input formats by the featInitReg
** functions) and to write the output file (ajFeattableWrite)
**
******************************************************************************/

static FeatOOutFormat featOutFormatDef[] =
{
    /* Name     Nucleotide Prot
         VocInit             WriteFunction
	 Description       Alias   Padding */
    {"unknown", AJFALSE,   AJFALSE,
	 NULL,               ajFeattableWriteGff3,
	 "unknown format", AJFALSE, 0},
    {"gff",       AJTRUE,    AJTRUE,
	 featVocabInitGff3,  ajFeattableWriteGff3,
	 "GFF version 3", AJFALSE, 0},
    {"gff2",       AJTRUE,    AJTRUE,
	 featVocabInitGff,   ajFeattableWriteGff2,
	 "GFF version 2", AJFALSE, 0},
    {"gff3",       AJTRUE,    AJTRUE,
	 featVocabInitGff3,   ajFeattableWriteGff3,
	 "GFF version 3", AJTRUE,  0},
    {"embl",      AJTRUE,    AJFALSE,
	 featVocabInitEmbl,  ajFeattableWriteEmbl,
	 "embl format", AJFALSE, 0},
    {"em",      AJTRUE,    AJFALSE,
	 featVocabInitEmbl,  ajFeattableWriteEmbl,
	 "embl format", AJTRUE,  0},
    {"genbank",   AJTRUE,    AJFALSE,
	 featVocabInitEmbl,  ajFeattableWriteGenbank,
	 "genbank format", AJFALSE, 0},
    {"gb",        AJTRUE,    AJFALSE,
	 featVocabInitEmbl,  ajFeattableWriteGenbank,
	 "genbank format", AJTRUE, 0},
    {"ddbj",      AJTRUE,    AJFALSE,
	 featVocabInitEmbl,  ajFeattableWriteDdbj,
	 "ddbj format", AJFALSE, 0},
    {"refseq",    AJTRUE,    AJFALSE,
	 featVocabInitEmbl,  ajFeattableWriteGenbank,
	 "genbank format", AJTRUE, 0},
    {"refseqp",   AJFALSE,   AJTRUE,
	 featVocabInitRefseqp, ajFeattableWriteRefseqp,
	 "genbank format", AJTRUE, 0},
    {"pir",       AJFALSE,   AJTRUE,
	 featVocabInitPir,   ajFeattableWritePir,
	 "PIR format", AJFALSE, 0},
    {"nbrf",       AJFALSE,   AJTRUE,
	 featVocabInitPir,   ajFeattableWritePir,
	 "PIR format", AJTRUE,  0},
    {"swiss",     AJFALSE,   AJTRUE,
	 featVocabInitSwiss, ajFeattableWriteSwiss,
	 "SwissProt format", AJFALSE, 0},
    {"sw",        AJFALSE,   AJTRUE,
	 featVocabInitSwiss, ajFeattableWriteSwiss,
	 "SwissProt format", AJTRUE, 0},
    {"swissprot", AJFALSE,   AJTRUE,
	 featVocabInitSwiss, ajFeattableWriteSwiss,
	 "SwissProt format", AJTRUE, 0},
    {"dasgff",        AJTRUE,   AJTRUE,
	 featVocabInitGff3, ajFeattableWriteDasgff,
	 "DAS GFF format", AJFALSE, 0},
    {"debug",     AJTRUE,    AJTRUE,
	 featVocabInitEmbl, ajFeattableWriteDebug,
	 "Debugging trace of full internal data content", AJFALSE, 0},
    {NULL, AJFALSE, AJFALSE, NULL, NULL, NULL, AJFALSE, 0}
};

static FeatPOutFormat featOutFormat = featOutFormatDef;





/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */

/* Set each of the regular expressions below, depending on feature format */

static AjPRegexp GffRegexNumeric   = NULL;
static AjPRegexp GffRegexblankline = NULL;
static AjPRegexp GffRegexversion   = NULL;
static AjPRegexp GffRegexdate      = NULL;
static AjPRegexp GffRegexregion    = NULL;
static AjPRegexp GffRegexcomment   = NULL;
static AjPRegexp GffRegextype      = NULL;

static AjPRegexp GffRegexTvTagval  = NULL;

static AjPRegexp Gff3RegexNumeric   = NULL;
static AjPRegexp Gff3Regexblankline = NULL;
static AjPRegexp Gff3Regexversion   = NULL;
static AjPRegexp Gff3Regexdate      = NULL;
static AjPRegexp Gff3Regexregion    = NULL;
static AjPRegexp Gff3Regexcomment   = NULL;
static AjPRegexp Gff3Regextype      = NULL;

static AjPRegexp Gff3RegexTvTagval  = NULL;

static AjPRegexp PirRegexAll          = NULL;
static AjPRegexp PirRegexCom          = NULL;
static AjPRegexp PirRegexLoc          = NULL;
static AjPRegexp PirRegexPos          = NULL;

static AjPRegexp SwRegexComment       = NULL;
static AjPRegexp SwRegexFtid          = NULL;
static AjPRegexp SwRegexNew           = NULL;
static AjPRegexp SwRegexNext          = NULL;

static AjPRegexp DummyRegExec         = NULL;

static AjPRegexp featRegUfoFmt = NULL;
static AjPRegexp featRegUfoFile = NULL;

static AjPRegexp featRegTagReplace = NULL;
static AjPRegexp featRegFlag = NULL;
static AjPRegexp featRegMore = NULL;
static AjPRegexp featRegQuote = NULL;

static AjPRegexp featTagTrans = NULL;    
static AjPRegexp featRegGroup = NULL;

static AjPRegexp featRegSpecialAnticodon = NULL;
static AjPRegexp featRegSpecialBiomaterial = NULL;
static AjPRegexp featRegSpecialCodon = NULL;
static AjPRegexp featRegSpecialCodonBad = NULL;
static AjPRegexp featRegSpecialColdate = NULL;
static AjPRegexp featRegSpecialCompare = NULL;
static AjPRegexp featRegSpecialConssplice = NULL;
static AjPRegexp featRegSpecialEstlen = NULL;
static AjPRegexp featRegSpecialInference = NULL;
static AjPRegexp featRegSpecialLatlon = NULL;
static AjPRegexp featRegSpecialMobile = NULL;
static AjPRegexp featRegSpecialPrimer = NULL;
static AjPRegexp featRegSpecialRptRange = NULL;
static AjPRegexp featRegSpecialRptRangeLab = NULL;
static AjPRegexp featRegSpecialRptRangeComp = NULL;
static AjPRegexp featRegSpecialRptunitSeq = NULL;
static AjPRegexp featRegSpecialRptunitSeqPos = NULL;
static AjPRegexp featRegSpecialTrans = NULL;
static AjPRegexp featRegSpecialTransBad = NULL;
static AjPRegexp featRegSpecialTransComp = NULL;
static AjPRegexp featRegSpecialTransBadComp = NULL;




/* @datastatic FeatPTypeIn ****************************************************
**
** feature input types
**
** @alias FeatSTypeIn
** @alias FeatOTypeIn
**
** @attr Name [const char*] Specified name
** @attr Value [const char*] Internal type "P" or "N"
** @@
******************************************************************************/

typedef struct FeatSTypeIn
{
    const char* Name;
    const char* Value;
} FeatOTypeIn;
#define FeatPTypeIn FeatOTypeIn*




static FeatOTypeIn featInTypes[] =
{
    {"P", "P"},
    {"protein", "P"},
    {"N", "N"},
    {"nucleotide", "N"},
    {"any", ""},
    {NULL, NULL}
};




/* @datastatic FeatPTypeOut ***************************************************
**
** Feature output types
**
** @alias FeatSTypeOut
** @alias FeatOTypeOut
**
** @attr Name [const char*] Specified name
** @attr Value [const char*] Internal type "P" or "N"
** @@
******************************************************************************/

typedef struct FeatSTypeOut
{
    const char* Name;
    const char* Value;
} FeatOTypeOut;
#define FeatPTypeOut FeatOTypeOut*




static FeatOTypeOut featOutTypes[] =
{
    {"P", "P"},
    {"protein", "P"},
    {"N", "N"},
    {"nucleotide", "N"},
    {"any", ""},
    {NULL, NULL}
};




/* @datastatic FeatPCategory ***************************************************
**
** Feature output types
**
** @alias FeatSTypeOut
** @alias FeatOTypeOut
**
** @attr Name [const char*] Specified name
** @attr Desc [const char*] Description
** @attr Types [const char*] Internal type names in this category
** @@
******************************************************************************/

typedef struct FeatSCategory
{
    const char* Name;
    const char* Desc;
    const char* Types;
} FeatOCategory;
#define FeatPCategory FeatOCategory*




static FeatOCategory featCategory[] =
{
    {"other", "all other features",
     "SO:2000061,SO:0000110,SO:0000723,SO:0000298,SO:0000724,SO:0000704,"
     "SO:0000410,SO:0000409,SO:0000297,SO:0000313,SO:0005836_rep_origin"},
    {"component", "component of a sequence assembly",
     ""},
    {"supercomponent", "reference sequence assembly",
     ""},
    {"translation", "translation to protein",
     "SO:0000316,SO:0000419,SO:0000418,SO:0000725,"
     "SO:0000204,SO:0000205,SO:0000139"},
    {"transcription", "transcription of RNA",
     "SO:0000147,SO:0000188,SO:0000673,SO:0005836,SO:0000234,SO:0000551,"
     "SO:0000553,SO:0000185_precursor_RNA,SO:0000185,SO:0000252,"
     "SO:0000013,SO:0000275,SO:0000274,SO:0000253,SO:0000557,SO:0000555"},
    {"promoter", "promoter regions and regulation of transcription",
     "SO:0000167,SO:0000172,SO:0000174,SO:0000178,SO:0000140,SO:0000173"
     "SO:0000175,SO:0000176,SO:0000165,SO:0000141"},
    {"variation", "polymorphic regions",
     ""},
    {"structural", "mapping, sequence and assembly - with no biological information",
     "SO:0005850,SO:0000002,SO:0000331,SO:0000730"},
    {"similarity", "areas similar to other sequences",
     ""},
    {"repeat", "areas of repetitive sequence",
     "SO:0000657,SO:0000726,SO:0000005,SO:0000286"},
    {"experimental", "experimental results",
     ""},
    {"immunoglobulin", "immunoglobulin processing regions",
     "SO:0000001_C_region,SO:0000001_J_segment,SO:0000001_N_region,"
     "SO:0000001_S_region,SO:0000001_V_region,SO:0000001_V_segment,SO:0000458"},
    {NULL, NULL,
     NULL}
};




/* ==================================================================== */
/* ======================== private methods ========================= */
/* ==================================================================== */

/* ==================================================================== */
/* ========================= constructors ============================= */
/* ==================================================================== */




/* @section Feature Object Constructors ***************************************
**
** All constructors return a new feature or feature table set by pointer.
** It is the responsibility of the user to first destroy any previous feature.
** The target pointer does not need to be initialised to NULL, but it is good
** programming practice to do so anyway.
**
** Generally, however, the AjPFeattable feature table object will first
** be created, prior to creating any 'AjPFeature' objects and adding them.
**
** To replace or reuse an existing feature object see instead
** the {Feature Assignments} and {Feature Modifiers} methods.
**
******************************************************************************/




/* @func ajFeattabOutOpen *****************************************************
**
** Processes the specified UFO, and opens the resulting output file.
**
** If the AjPFeattabOut has a filename, this is used in preference.
** If not, the ufo is processed.
**
** @param [u] thys [AjPFeattabOut] Features table output object
** @param [r] ufo [const AjPStr] UFO feature output specifier
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeattabOutOpen(AjPFeattabOut thys, const AjPStr ufo)
{
    /*ajDebug("ajFeattabOutOpen ufo:'%S' dir:'%S' file:'%S'\n",
	    ufo, thys->Directory, thys->Filename);*/
    if(thys->Handle)
	return ajTrue;

    if(ajStrGetLen(thys->Filename))
    {
	if(!featFindOutFormatS(thys->Formatstr, &thys->Format))
	    ajErr("unknown output feature format '%S' "
		  "will write as %s instead\n",
		  thys->Formatstr, featOutFormatDef[thys->Format].Name);
    }
    else
    {
	if(!featoutUfoProcess(thys, ufo))
	    return ajFalse;
    }

    /*ajDebug("trying to open dir:'%S' file:'%S' fmt:'%S'\n",
	    thys->Directory, thys->Filename, thys->Formatstr);*/
    thys->Handle = ajFileNewOutNamePathS(thys->Filename, thys->Directory);

    if(!thys->Handle)
	return ajFalse;
   /* ajDebug("after opening '%S'\n", thys->Filename);*/

    return ajTrue;
}




/* @func ajFeattabOutFile *****************************************************
**
** Returns the feature table output file object
**
** @param [r] thys [const AjPFeattabOut] Features table output object
** @return [AjPFile] File object
** @@
******************************************************************************/

AjPFile ajFeattabOutFile(const AjPFeattabOut thys)
{
   /* ajDebug("ajFeattabOutFile\n");*/
    return thys->Handle;
}




/* @func ajFeattabOutFilename *************************************************
**
** Returns the name of a feature table output file
**
** @param [r] thys [const AjPFeattabOut] Features table output object
** @return [AjPStr] Filename
** @@
******************************************************************************/

AjPStr ajFeattabOutFilename(const AjPFeattabOut thys)
{
   /* ajDebug("ajFeattabOutFilename\n");*/
    if(ajStrGetLen(thys->Filename))
	return thys->Filename;

    return NULL;
}




/* @func ajFeattabOutIsOpen ***************************************************
**
** Checks whether feature output file has already been opened
**
** @param [r] thys [const AjPFeattabOut] Features table output object
** @return [AjBool] ajTrue if file is open
** @@
******************************************************************************/

AjBool ajFeattabOutIsOpen(const AjPFeattabOut thys)
{
   /* ajDebug("ajFeattabOutIsOpen\n");*/
    if(thys->Handle)
	return ajTrue;

    return ajFalse;
}




/* @func ajFeattabOutIsLocal **************************************************
**
** Checks whether feature output file has already been opened
**
** @param [r] thys [const AjPFeattabOut] Features table output object
** @return [AjBool] ajTrue if file is open
** @@
******************************************************************************/

AjBool ajFeattabOutIsLocal(const AjPFeattabOut thys)
{
   /* ajDebug("ajFeattabOutIsLocal Handle %x Local %B\n",
	    thys->Handle, thys->Local);*/

    if(thys->Handle && thys->Local)
	return ajTrue;

    return ajFalse;
}




/* @func ajFeattabOutSet ******************************************************
**
** Processes the specified UFO, and specifies the resulting output file.
**
** @param [u] thys [AjPFeattabOut] Features table output object
** @param [r] ufo [const AjPStr] UFO feature output specifier
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeattabOutSet(AjPFeattabOut thys, const AjPStr ufo)
{
   /* ajDebug("ajFeattabOutSet ufo:'%S' dir:'%S' file:'%S'\n",
	    ufo, thys->Directory, thys->Filename);*/
    if(thys->Handle)
	return ajTrue;

    if(!featoutUfoProcess(thys, ufo))
	return ajFalse;

    return ajTrue;
}




/* @func ajFeattabInNew *******************************************************
**
** Constructor for an empty feature table input object
**
** @return [AjPFeattabIn] Feature table input object
** @category new [AjPFeattabIn] Constructor
** @@
******************************************************************************/

AjPFeattabIn ajFeattabInNew(void)
{
    AjPFeattabIn pthis;
    AJNEW0(pthis);

   /* ajDebug("ajFeatTabInNew %x\n", pthis);*/

    return pthis;
}




/* @func ajFeattabInNewSS *****************************************************
**
** Constructor for an empty feature table input object. The format and
** name are read.
**
** @param [r] fmt [const AjPStr] feature format
** @param [r] name [const AjPStr] sequence name
** @param [r] type [const char*] feature type
** @return [AjPFeattabIn] Feature table input object
** @category new [AjPFeattabIn] Constructor with format, name and type
** @@
******************************************************************************/

AjPFeattabIn ajFeattabInNewSS(const AjPStr fmt, const AjPStr name,
			      const char* type)
{
    AjPFeattabIn pthis;
    ajint iformat = 0;

    if(!featFindInFormatS(fmt, &iformat))
	return NULL;

    pthis = ajFeattabInNew();
    ajStrAssignC(&pthis->Formatstr, featInFormat[pthis->Format].Name);
    pthis->Format = iformat;
    ajStrAssignC(&pthis->Type, type);
    ajStrAssignS(&pthis->Seqname, name);
    pthis->Handle = ajFilebuffNewNofile();

    /*ajDebug("ajFeatTabInNewSS %x Handle %x\n", pthis, pthis->Handle);*/

    return pthis;
}




/* @func ajFeattabInNewCSF ****************************************************
**
** Constructor for an empty feature table input object. The format and
** name are read. The file buffer is moved to the feature table input
** object and should not be deleted by the calling program.
**
** @param [r] fmt [const char*] feature format
** @param [r] name [const AjPStr] sequence name
** @param [r] type [const char*] feature type
** @param [u] buff [AjPFilebuff] Buffer containing feature data
** @return [AjPFeattabIn] Feature table input object
** @category new [AjPFeattabIn] Constructor with format, name, type
**                              and input file
** @@
******************************************************************************/

AjPFeattabIn ajFeattabInNewCSF(const char* fmt, const AjPStr name,
				const char* type, AjPFilebuff buff)
{
    AjPFeattabIn pthis;
    ajint iformat = 0;

    if(!featFindInFormatC(fmt, &iformat))
	return NULL;

    pthis = ajFeattabInNew();
    ajStrAssignC(&pthis->Formatstr, featInFormat[iformat].Name);
    pthis->Format = iformat;
    ajStrAssignC(&pthis->Type, type);
    ajStrAssignS(&pthis->Seqname, name);
    pthis->Local = ajTrue;
    pthis->Handle = buff;

    /*ajDebug("ajFeatTabInNewCCF %x Handle %x\n", pthis, buff);*/

    return pthis;
}




/* @func ajFeattabInNewSSF ****************************************************
**
** Constructor for an empty feature table input object. The format and
** name are read. The file buffer is moved to the feature table input
** object and should not be deleted by the calling program.
**
** @param [r] fmt [const AjPStr] feature format
** @param [r] name [const AjPStr] sequence name
** @param [r] type [const char*] feature type
** @param [u] buff [AjPFilebuff] Buffer containing feature data
** @return [AjPFeattabIn] Feature table input object
** @category new [AjPFeattabIn] Constructor with format, name, type
**                              and input file
** @@
******************************************************************************/

AjPFeattabIn ajFeattabInNewSSF(const AjPStr fmt, const AjPStr name,
				const char* type, AjPFilebuff buff)
{
    AjPFeattabIn pthis;
    ajint iformat = 0;

    if(!featFindInFormatS(fmt, &iformat))
	return NULL;

    pthis = ajFeattabInNew();
    ajStrAssignC(&pthis->Formatstr, featInFormat[iformat].Name);
    pthis->Format = iformat;
    ajStrAssignC(&pthis->Type, type);
    ajStrAssignS(&pthis->Seqname, name);
    pthis->Local = ajTrue;
    pthis->Handle = buff;

    /*ajDebug("ajFeatTabInNewSSF %x Handle %x\n", pthis, buff);*/

    return pthis;
}




/* @func ajFeattabOutNew ******************************************************
**
** Constructor for an empty feature table output object
**
** @return [AjPFeattabOut] Feature table input object
** @category new [AjPFeattabOut] Constructor
** @@
******************************************************************************/

AjPFeattabOut ajFeattabOutNew(void)
{
    AjPFeattabOut pthis;
    AJNEW0(pthis);

    /*ajDebug("ajFeattabOutNew %x\n", pthis);*/

    return pthis;
}




/* @func ajFeattabOutSetBasename **********************************************
**
** Sets the base output filename for feature table output
**
** @param [u] thys [AjPFeattabOut] feature table output
** @param [r] basename [const AjPStr] Output base filename
** @return [void]
** @@
******************************************************************************/

void ajFeattabOutSetBasename(AjPFeattabOut thys, const AjPStr basename)
{
    AjPStr tmpname = NULL;

    tmpname = ajStrNewS(basename);    
    ajFilenameTrimAll(&tmpname);    
    ajStrAssignEmptyS(&thys->Basename, tmpname);
    ajStrDel(&tmpname);
    /*ajDebug("ajFeattabOutSetBasename '%S' result '%S'\n",	
	    basename, thys->Basename);*/
    
    return;  
}




/* @func ajFeattabOutSetSeqname ************************************************
**
** Processes the specified UFO, and specifies the resulting output file.
**
** @param [u] thys [AjPFeattabOut] Features table output object
** @param [r] name [const AjPStr] UFO feature output specifier
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeattabOutSetSeqname(AjPFeattabOut thys, const AjPStr name)
{
   /* ajDebug("ajFeattabOutSet ufo:'%S' dir:'%S' file:'%S'\n",
	    ufo, thys->Directory, thys->Filename);*/
    ajStrAssignS(&thys->Seqname, name);

    return ajTrue;
}




/* @func ajFeattabOutNewCSF ***************************************************
**
** Constructor for an empty feature table output object, using an
** existing open output file (the file sequence data is already written to)
**
** @param [r] fmt [const char*] feature format
** @param [r] name [const AjPStr] sequence name
** @param [r] type [const char*] feature type
** @param [u] file [AjPFile] Output file
** @return [AjPFeattabOut] Feature table output object
** @category new [AjPFeattabOut] Constructor with format, name, type
**                               and output file
** @@
******************************************************************************/

AjPFeattabOut ajFeattabOutNewCSF(const char* fmt, const AjPStr name,
				 const char* type, AjPFile file)
{
    AjPFeattabOut pthis;
    ajint iformat = 0;

    /*ajDebug("ajFeattabOutNewCSF '%S' '%S' '%s' '%F'\n",
	    fmt, name, type, file);*/

    if(!featFindOutFormatC(fmt, &iformat))
	return NULL;

    pthis = ajFeattabOutNew();
    ajStrAssignC(&pthis->Formatstr, featOutFormat[iformat].Name);
    pthis->Format = iformat;
    ajFeattabOutSetTypeC(pthis, type);
    ajStrAssignS(&pthis->Seqname, name);
    pthis->Handle = file;
    pthis->Local = ajTrue;

    /*ajDebug("ajFeatTabOutNewSSF %x\n", pthis);*/

    return pthis;
}




/* @func ajFeattabOutNewSSF ***************************************************
**
** Constructor for an empty feature table output object, using an
** existing open output file (the file sequence data is already written to)
**
** @param [r] fmt [const AjPStr] feature format
** @param [r] name [const AjPStr] sequence name
** @param [r] type [const char*] feature type
** @param [u] file [AjPFile] Output file
** @return [AjPFeattabOut] Feature table output object
** @category new [AjPFeattabOut] Constructor with format, name, type
**                               and output file
** @@
******************************************************************************/

AjPFeattabOut ajFeattabOutNewSSF(const AjPStr fmt, const AjPStr name,
				 const char* type, AjPFile file)
{
    AjPFeattabOut pthis;
    ajint iformat = 0;

    /*ajDebug("ajFeattabOutNewSSF '%S' '%S' '%s' '%F'\n",
	    fmt, name, type, file);*/

    if(!featFindOutFormatS(fmt, &iformat))
	return NULL;

    pthis = ajFeattabOutNew();
    ajStrAssignC(&pthis->Formatstr, featOutFormat[iformat].Name);
    pthis->Format = iformat;
    ajFeattabOutSetTypeC(pthis, type);
    ajStrAssignS(&pthis->Seqname, name);
    pthis->Handle = file;
    pthis->Local = ajTrue;

    /*ajDebug("ajFeatTabOutNewSSF %x\n", pthis);*/

    return pthis;
}




/* @func ajFeatNew ************************************************************
**
** Constructor - must specify associated 'ajFeattable'
**               to which the new feature is automatically added!
**
** @param  [u]   thys    [AjPFeattable] Pointer to the ajFeattable which
**                         will own the feature
** @param  [r] source   [const AjPStr]      Analysis basis for feature
** @param  [r] type     [const AjPStr]      Type of feature (e.g. exon)
** @param  [r]  Start    [ajint]  Start position of the feature
** @param  [r]  End      [ajint]  End position of the feature
** @param  [r] score    [float]      Analysis score for the feature
** @param  [r]  strand   [char]  Strand of the feature
** @param  [r]  frame    [ajint]   Frame of the feature
** @return [AjPFeature] newly allocated feature object
** @category new [AjPFeature] Constructor - must specify the associated
**                           (non-null) AjPFeattable
** @@
******************************************************************************/

AjPFeature ajFeatNew(AjPFeattable thys,
		     const AjPStr source,
		     const AjPStr type,
		     ajint Start, ajint End,
		     float score,
		     char  strand,
		     ajint frame)
{
    ajint flags    = 0;
    AjPFeature ret = NULL;

    if(ajStrMatchC(thys->Type, "P"))
        ret = featFeatNewProt(thys,source,type,Start,End,score,flags);
    else
        ret = featFeatNew(thys,source,type,Start,End,score,strand,frame,
                          0,0,0,NULL, NULL,flags);

    return ret;
}




/* @func ajFeatNewII **********************************************************
**
** Simple constructor with only start and end positions
**
** User must specify associated 'ajFeattable' to which the new feature
** is automatically added!
**
** @param  [u]   thys    [AjPFeattable] Pointer to the ajFeattable which
**                         will own the feature
** @param  [r]  Start    [ajint]  Start position of the feature
** @param  [r]  End      [ajint]  End position of the feature
** @return [AjPFeature] newly allocated feature object
** @category new [AjPFeature] Simple constructor with only start and end
**                            positions
** @@
******************************************************************************/

AjPFeature ajFeatNewII(AjPFeattable thys,
		       ajint Start, ajint End)
{
    AjPStr source = NULL;
    float score   = 0.0;
    char strand   = '.';
    ajint frame   = 0;
    ajint flags   = 0;

    AjPFeature ret = NULL ;

    /*ajDebug("ajFeatNewII %d %d\n", Start, End);*/

    if(!featTypeMiscfeat)
	featTypeMiscfeat = ajStrNewC("misc_feature");

    if(Start > End)
	ret = featFeatNew(thys,source,featTypeMiscfeat,
			  End,Start,score,'-',frame,
			  0,0,0,NULL, NULL,flags);
    else
	ret = featFeatNew(thys,source,featTypeMiscfeat,
			  Start,End,score,strand,frame,
			  0,0,0,NULL, NULL,flags);

    return ret;
}




/* @func ajFeatNewIIRev *******************************************************
**
** Simple constructor with only start and end positions, sets feature to be
** on the reverse strand
**
** User must specify associated 'ajFeattable' to which the new feature
** is automatically added!
**
** @param  [u]   thys    [AjPFeattable] Pointer to the ajFeattable which
**                         will own the feature
** @param  [r]  Start    [ajint]  Start position of the feature
** @param  [r]  End      [ajint]  End position of the feature
** @return [AjPFeature] newly allocated feature object
** @category new [AjPFeature] Simple constructor with only start and end
**                            positions, sets feature to be
**                            on the reverse strand
** @@
******************************************************************************/

AjPFeature ajFeatNewIIRev(AjPFeattable thys,
			  ajint Start, ajint End)
{
    AjPFeature ret = NULL ;

    /*ajDebug("ajFeatNewIIRev %d %d\n", Start, End);*/

    if(Start > End)
	ret = ajFeatNewII(thys,End,Start);
    else
	ret = ajFeatNewII(thys,Start,End);

    ret->Strand = '-';

    return ret;
}




/* @func ajFeatNewProt ********************************************************
**
** Constructor - must specify associated 'ajFeattable'
**               to which the new feature is automatically added!
**
** @param  [u]   thys    [AjPFeattable] Pointer to the ajFeattable which
**                         will own the feature
** @param  [r] source   [const AjPStr]      Analysis basis for feature
** @param  [r] type     [const AjPStr]      Type of feature (e.g. exon)
** @param  [r]  Start    [ajint]  Start position of the feature
** @param  [r]  End      [ajint]  End position of the feature
** @param  [r] score    [float]      Analysis score for the feature
** @return [AjPFeature] newly allocated feature object
** @category new [AjPFeature] Protein-specific constructor - 
**                            must specify the associated
**                            (non-null) AjPFeattable
** @@
**
******************************************************************************/

AjPFeature ajFeatNewProt(AjPFeattable thys,
			 const AjPStr source,
			 const AjPStr type,
			 ajint Start, ajint End,
			 float score)
{
    ajint flags    = 0;
    AjPFeature ret = NULL ;

    ret = featFeatNewProt(thys,source,type,Start,End,score,flags);

    return ret;
}




/* @funcstatic featCompByStart ************************************************
**
** Compare two features by their start.
**
** @param [r] a [const void *] feature
** @param [r] b [const void *] another feature
**
** @return [ajint] -1 if a is less than b, 0 if a is equal to b else +1.
** @@
******************************************************************************/

static ajint featCompByStart(const void *a, const void *b)
{
    const AjPFeature gfa;
    const AjPFeature gfb;
    ajint val = 0;

    gfa = *(AjPFeature const *) a;
    gfb = *(AjPFeature const *) b;

    val = gfa->Start - gfb->Start;

    if(val)
	return val;
    else
    {
	val = gfb->End - gfa->End;
	if(val)
	    return val;
    }

    return 0;
}




/* @funcstatic featCompByEnd **************************************************
**
** Compare two features by their end.
**
** @param [r] a [const void *] feature
** @param [r] b [const void *] another feature
**
** @return [ajint] -1 if a is less than b, 0 if a is equal to b else +1.
** @@
******************************************************************************/

static ajint featCompByEnd(const void *a, const void *b)
{
    const AjPFeature gfa;
    const AjPFeature gfb;

    ajint val = 0;

    gfa = *(AjPFeature const *) a;
    gfb = *(AjPFeature const *) b;

    val = gfa->End - gfb->End;

    if(val)
	return val;
    else
    {
	val = gfa->Start - gfb->Start;
	if(val)
	    return val;
    }

    return 0;
}




/* @funcstatic featCompByGroup ************************************************
**
** Compare two features by their group and exon numbers
**
** @param [r] a [const void *] feature
** @param [r] b [const void *] another feature
**
** @return [ajint] -1 if a is less than b, 0 if a is equal to b else +1.
** @@
******************************************************************************/

static ajint featCompByGroup(const void *a, const void *b)
{
    const AjPFeature gfa;
    const AjPFeature gfb;

    ajint val = 0;

    gfa = *(AjPFeature const *) a;
    gfb = *(AjPFeature const *) b;

    val = gfa->Group - gfb->Group;

    if(val)
	return val;

    val = gfa->Exon - gfb->Exon;

    return val;
}




/* @funcstatic featCompByType *************************************************
**
** Compare two features by their type (key).
**
** @param [r] a [const void *] feature
** @param [r] b [const void *] another feature
**
** @return [ajint] -1 if a is less than b, 0 if a is equal to b else +1.
** @@
******************************************************************************/

static ajint featCompByType(const void *a, const void *b)
{
    const AjPFeature gfa;
    const AjPFeature gfb;

    ajint val = 0;

    gfa = *(AjPFeature const *) a;
    gfb = *(AjPFeature const *) b;

    val = ajStrVcmp(&gfa->Type,&gfb->Type);

    if(val)
	return val;
    else
    {
	val = gfa->Start - gfb->Start;

	if(val)
	    return val;
	else
	{
	    val = gfa->End - gfb->End;

	    if(val)
		return val;
	}
    }

    return 0;
}




/******************************************************************************
**
** Utility classes...
**
******************************************************************************/




/* @funcstatic featFeatNew ****************************************************
**
** Constructor for a new feature,
** automatically added to the specified table.
**
** @param  [u]   thys     [AjPFeattable] Pointer to the ajFeattable which
**                         will own the feature
** @param  [r] source   [const AjPStr]      Analysis basis for feature
** @param  [r] type     [const AjPStr]      Type of feature (e.g. exon)
** @param  [r]  Start    [ajint]  Start position of the feature
** @param  [r]  End      [ajint]  End position of the feature
** @param  [r] score    [float]      Analysis score for the feature
** @param  [r]  strand   [char]  Strand of the feature
** @param  [r]  frame    [ajint]   Frame of the feature
** @param  [r]  exon     [ajint]  exon number (0 for default value)
** @param  [r]  Start2   [ajint]  2nd Start position of the feature
** @param  [r]  End2     [ajint]  2nd End position of the feature
** @param  [r] entryid  [const AjPStr] Entry ID for location in
**                                        another entry
** @param  [r] label    [const AjPStr] Label for location (non-numeric)
** @param  [r]  flags    [ajint]  flags.
** @return [AjPFeature] newly allocated feature object
** @@
******************************************************************************/

static AjPFeature featFeatNew(AjPFeattable thys,
			      const AjPStr source,
			      const AjPStr type,
			      ajint        Start,
			      ajint        End,
			      float        score,
			      char         strand,
			      ajint        frame,
			      ajint        exon,
			      ajint        Start2,
			      ajint        End2,
			      const AjPStr entryid,
			      const AjPStr label,
			      ajint        flags)
{
    AjPFeature ret          = NULL;
    static ajint maxexon    = 0;
    
    ajDebug("featFeatNew %d %d '%c' type: '%S'\n", Start, End, strand, type);

    if(!featDefSource)
	ajStrAssignS(&featDefSource, ajUtilGetProgram());
    
    /* ajDebug("\nfeatFeatNew '%S' %d .. %d %x\n",
       type, Start, End, flags); */
    
    if(!ajStrGetLen(type))
	return NULL;
    
    featInit();
    
    /* Allocate the object... */
    
    ret = featFeatureNew();
    
    if(flags & FEATFLAG_CHILD)
    {
	ret->Group = thys->Groups;

	if(exon)
	    ret->Exon  = exon;
	else
	    ret->Exon  = ++maxexon;
    }
    else
    {
	thys->Groups++;
	ret->Group = thys->Groups;
	ret->Exon  = 0;
    }
    
    if(ajStrGetLen(source))
	ajStrAssignS(&ret->Source, source);
    else
	ajStrAssignS(&ret->Source, featDefSource);

    /*ajDebug("featFeatNew type '%S'\n", thys->Type);*/

    if(ajStrMatchC(thys->Type, "P"))
	ajStrAssignS(&ret->Type, featTypeProt(type));
    else
	ajStrAssignS(&ret->Type, featTypeDna(type));
    
    ajDebug("featFeatNew feature type '%S' => '%S'\n", type, ret->Type);
    ret->Score = score;
    
    ret->Flags = flags;

    if(strand == '+' || strand == '-')
        ret->Strand = strand;
    else ret->Strand = '\0';

    ret->Frame  = frame ;
    ret->Start  = Start;
    ret->End    = End;
    ret->Start2 = Start2;
    ret->End2   = End2;
    
    if(ajStrGetLen(entryid))
	ajStrAssignS(&ret->Remote, entryid);
    else
    {
	if(!(ret->Flags & FEATFLAG_REMOTEID) &&
	   !(ret->Flags & FEATFLAG_LABEL))
	{
	    thys->Len = AJMAX(thys->Len, ret->Start);
	    thys->Len = AJMAX(thys->Len, ret->End);
	}
    }
    
    if(ajStrGetLen(label))
    {
	ajStrAssignS(&ret->Label, label);
	featWarn("%S: Feature label '%S' used",
	       thys->Seqid, label);
    }
    
    ajFeattableAdd(thys,ret) ;
    
    return ret ;
}




/* @funcstatic featFeatNewProt ************************************************
**
** Constructor for a new protein feature,
** automatically added to the specified table.
**
** @param  [u]   thys     [AjPFeattable] Pointer to the ajFeattable which
**                         will own the feature
** @param  [r] source   [const AjPStr]      Analysis basis for feature
** @param  [r] type     [const AjPStr]      Type of feature (e.g. exon)
** @param  [r]  Start    [ajint]  Start position of the feature
** @param  [r]  End      [ajint]  End position of the feature
** @param  [r] score    [float]      Analysis score for the feature
** @param  [r]  flags    [ajint]  flags.
** @return [AjPFeature] newly allocated feature object
** @@
**
******************************************************************************/

static AjPFeature featFeatNewProt(AjPFeattable thys,
				  const AjPStr source,
				  const AjPStr type,
				  ajint        Start,
				  ajint        End,
				  float        score,
				  ajint        flags)
{
    AjPFeature ret          = NULL;
    static ajint maxexon    = 0;
    
    if(!featDefSource)
	ajStrAssignS(&featDefSource, ajUtilGetProgram());
    
    ajDebug("\nfeatFeatNewProt '%S' %d .. %d %x\n", type, Start, End, flags);
    
    if(!ajStrGetLen(type))
	return NULL;
    
    featInit();
    
    /* Allocate the object... and a new Tags list */
    ret = featFeatureNew() ;
    
    if(flags & FEATFLAG_CHILD)
    {
	ret->Group = thys->Groups;
	ret->Exon  = ++maxexon;
    }
    else
    {
	thys->Groups++;
	ret->Group = thys->Groups;
	ret->Exon  = 0;
    }
    
    if(ajStrGetLen(source))
	ajStrAssignS(&ret->Source, source);
    else
	ajStrAssignS(&ret->Source, featDefSource);
    
    ajStrAssignS(&ret->Type, featTypeProt(type));
    ajDebug("featFeatNewProt feature type '%S' => '%S'\n", type, ret->Type);
    
    ret->Score = score;
    
    ret->Flags = flags;
    
    ret->Strand = '\0' ;
    
    ret->Frame  = 0 ;
    ret->Start  = Start;
    ret->End    = End;
    ret->Start2 = 0;
    ret->End2   = 0;
    
    ret->Protein = ajTrue;
    
    if(!(ret->Flags & FEATFLAG_REMOTEID) &&
       !(ret->Flags & FEATFLAG_LABEL))
    {
	thys->Len = AJMAX(thys->Len, ret->Start);
	thys->Len = AJMAX(thys->Len, ret->End);
    }
    
    ajFeattableAdd(thys,ret) ;
    
    return ret ;
}




/* ==================================================================== */
/* =========================== destructor ============================= */
/* ==================================================================== */




/* @section Feature Object Destructors ****************************************
**
** (Simple minded) object destruction by release of memory.
**
** No reference counting (for now).
**
******************************************************************************/




/* @func ajFeattabInDel *******************************************************
**
** Destructor for a feature table input object
**
** @param [d] pthis [AjPFeattabIn*] Feature table input object
** @return [void]
** @category delete [AjPFeattabIn] Destructor
** @@
******************************************************************************/

void ajFeattabInDel(AjPFeattabIn* pthis)
{
    AjPFeattabIn thys;

    thys = *pthis;

    if(!thys)
	return;

    if(thys->Handle)
        ajDebug("ajFeattabInDel %x Handle %x Local:%B\n",
            thys, thys->Handle, thys->Local);
    else
        ajDebug("ajFeattabInDel %x Handle %x fp:UNKNOWN Local:%B\n",
            thys, thys->Handle, thys->Local);

    if(!thys->Local)
	ajFilebuffDel(&thys->Handle);    

    ajStrDel(&thys->Ufo);
    ajStrDel(&thys->Formatstr);
    ajStrDel(&thys->Filename);
    ajStrDel(&thys->Seqid);
    ajStrDel(&thys->Seqname);
    ajStrDel(&thys->Type);
    AJFREE(*pthis);

    return;
}




/* @func ajFeattableDel *******************************************************
**
** Destructor for ajFeattable objects.
** If the given object (pointer) is NULL, or a NULL pointer, simply returns.
**
** @param  [d] pthis [AjPFeattable*] Pointer to the object to be deleted.
**         The pointer is always deleted.
** @return [void]
** @category delete [AjPFeattable] Default destructor
** @@
******************************************************************************/

void ajFeattableDel(AjPFeattable *pthis)
{
    AjPFeattable thys;

    if(!pthis)
	return;

    thys = *pthis;

    /*ajDebug("ajFeattableDel %x\n", thys);*/

    if(!thys)
	return;

    ajFeattableClear(thys);

    ajStrDel(&thys->Type);
    ajListFree(&thys->Features);

    AJFREE(*pthis);
    *pthis = NULL;

    return;
}




/* @func ajFeatDel ************************************************************
**
** Destructor for AjPFeature objects.
** If the given object (pointer) is NULL, or a NULL pointer, simply returns.
**
** @param  [d] pthis [AjPFeature*] Pointer to the object to be deleted.
**         The pointer is always deleted.
** @return [void]
** @category delete [AjPFeature] Default destructor
** @@
******************************************************************************/

void ajFeatDel(AjPFeature *pthis)
{
    if(!pthis)
	return ;

    if(!*pthis)
	return ;

    featClear(*pthis);

    AJFREE(*pthis);
    *pthis = NULL ;

    return;
}




/* @funcstatic featClear ******************************************************
**
** Deletes all feature tag-value pairs from a feature line
**
** @param [u] thys [AjPFeature] Feature
** @return [void]
** @@
******************************************************************************/

static void featClear(AjPFeature thys)
{
    AjIList        iter = NULL ;
    FeatPTagval    item = NULL ;

    if(!thys)
	return;

    /* We need to delete the associated Tag data structures too!!!*/

    if(thys->Tags)
    {
	iter = ajListIterNew(thys->Tags);
	while(!ajListIterDone(iter))
	{
	    item = (FeatPTagval)ajListIterGet(iter);
	    /* assuming a simple block memory free for now...*/
	    ajStrDel(&item->Value);
	    ajStrDel(&item->Tag);
	    AJFREE(item);
	    ajListIterRemove(iter);
	}
	ajListIterDel(&iter);
    }

    ajListFree(&(thys->Tags));

    ajStrDel(&thys->Source);
    ajStrDel(&thys->Type);
    ajStrDel(&thys->Remote);
    ajStrDel(&thys->Label);

    return;
}




/* ==================================================================== */
/* ========================== Assignments ============================= */
/* ==================================================================== */




/* @section Feature Assignments ***********************************************
**
******************************************************************************/




/* @funcstatic featFormatSet **************************************************
**
** Sets the input format for a feature table using the feature input
** object's defined format.
**
** @param [u] featin [AjPFeattabIn] Feature table input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool featFormatSet(AjPFeattabIn featin)
{
    if(ajStrGetLen(featin->Formatstr))
    {
	/*ajDebug("... input format value '%S'\n", featin->Formatstr);*/
	if(featFindInFormatS(featin->Formatstr, &featin->Format))
	{
	    /* we may need to set feature table format too? */

	    /*
	       (void) ajStrAssignS(&thys->Formatstr, featin->Formatstr);
	       thys->Format = featin->Format;
	       ajDebug("...format OK '%S' = %d\n", featin->Formatstr,
	       featin->Format);
	       */
	}
	else
	{
	    /*ajDebug("...format unknown '%S'\n", featin->Formatstr);*/
	}

	return ajTrue;
    }
    else
    {
	/*ajDebug("...input format not set\n");*/
    }


    return ajFalse;
}




/* @func ajFeattableWriteUfo **************************************************
**
** Parses a UFO, opens an output file, and writes a feature table to it.
**
** @param [u] thys [AjPFeattabOut] Feature output object
** @param [r] ftable [const AjPFeattable] Feature table to be written
** @param [r] ufo [const AjPStr] UFO feature spec (ignored if already open)
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajFeattableWriteUfo(AjPFeattabOut thys, const AjPFeattable ftable,
		      const AjPStr ufo)
{
    if(!ajFeattabOutIsOpen(thys))
       ajFeattabOutOpen(thys, ufo);

    return ajFeattableWrite(thys, ftable);
}




/* @obsolete ajFeatUfoWrite
** @replace ajFeattableWriteUfo (1,2,3/2,1,3)
*/

__deprecated AjBool ajFeatUfoWrite(const AjPFeattable thys,
                                   AjPFeattabOut featout,
                                   const AjPStr ufo)
{
    return ajFeattableWriteUfo(featout, thys, ufo);
}





/* @func ajFeatSortByType *****************************************************
**
** Sort Feature table by Type.
**
** @param [u] Feattab [AjPFeattable] Feature table to be sorted.
**
** @return [void]
** @@
******************************************************************************/

void ajFeatSortByType(AjPFeattable Feattab)
{
    ajListSort(Feattab->Features,*featCompByType);

    return;
}




/* @func ajFeatSortByStart ****************************************************
**
** Sort Feature table by Start position.
**
** @param [u] Feattab [AjPFeattable] Feature table to be sorted.
**
** @return [void]
** @@
******************************************************************************/

void ajFeatSortByStart(AjPFeattable Feattab)
{
    ajListSort(Feattab->Features,*featCompByStart);
}




/* @func ajFeatSortByEnd ******************************************************
**
** Sort Feature table by End position.
**
** @param [u] Feattab [AjPFeattable] Feature table to be sorted.
**
** @return [void]
** @@
******************************************************************************/

void ajFeatSortByEnd(AjPFeattable Feattab)
{
    ajListSort(Feattab->Features,*featCompByEnd);

    return;
}




/* ==================================================================== */
/* ========================== Modifiers ============================= */
/* ==================================================================== */




/* @section Feature Table Modifiers ******************************************
**
******************************************************************************/




/* @func ajFeattableAdd *******************************************************
**
** Method to add a new AjPFeature to a AjPFeattable
**
** @param  [u] thys    [AjPFeattable] The feature table
** @param  [u] feature [AjPFeature]        Feature to be added to the set
** @return [void]
** @category modify [AjPFeattable] Adds an AjPFeature to a set
** @@
******************************************************************************/

void ajFeattableAdd(AjPFeattable thys, AjPFeature feature)
{
    if(!(feature->Flags & FEATFLAG_REMOTEID) &&
       !(feature->Flags & FEATFLAG_LABEL))
    {
	thys->Len = AJMAX(thys->Len, feature->Start);
	thys->Len = AJMAX(thys->Len, feature->End);
    }
    ajListPushAppend(thys->Features, feature);

/*
    if(feature->Type)
	ajDebug("ajFeattableAdd list size %d '%S' %d %d\n",
		ajListGetLength(thys->Features), feature->Type,
		feature->Start, feature->End);
    else
	ajDebug("ajFeattableAdd list size %d '%S' %d %d\n",
		ajListGetLength(thys->Features), NULL,
		feature->Start, feature->End);
*/

    return;
}




/* ==================================================================== */
/* ======================== Operators ==================================*/
/* ==================================================================== */




/* @section Feature Object Operators ******************************************
**
** These functions use the contents of a feature object,
** but do not make any changes.
**
******************************************************************************/




/* @funcstatic featTableInit **************************************************
**
** Initialise the components of a previously allocated AjPFeattable object.
**
** @param [u]   thys       [AjPFeattable]   Target feature table object
** @param [r]  name       [const AjPStr]   Name of the table (e.g.
**                                           sequence name)
** @return [void]
** @@
**
******************************************************************************/

static void featTableInit(AjPFeattable thys,
			  const AjPStr name)
{
    /*ajDebug("featTableInit Entering...\n");*/

    /*ajDebug("featTableInit initialising seqid: '%S'\n", name);*/
    ajStrAssignS(&thys->Seqid,name) ;
    thys->DefFormat = 0;

    return;
}




/* @func ajFeattableClear *****************************************************
**
** Clears a feature table of all features
**
** @param [u] thys [AjPFeattable] Feature table
** @return [void]
** @@
******************************************************************************/

void ajFeattableClear(AjPFeattable thys)
{
    AjIList iter       = NULL ;
    AjPFeature feature = NULL ;

    if(!thys)
	return ;

    /* Format and Version are simple variables, non-allocated...*/
    /* Don't worry about the Date... probably static...*/

    /* AJB: you should worry about any static variable! */
 
    ajStrDel(&thys->Seqid);

    if(thys->Features)
    {
	iter = ajListIterNew(thys->Features) ;
	while(!ajListIterDone(iter))
	{
	    feature = (AjPFeature)ajListIterGet(iter) ;
	    ajFeatDel(&feature) ;
	    ajListIterRemove(iter) ;
	}
	ajListIterDel(&iter) ;
    }

    return;
}




/* @funcstatic featoutUfoProcess **********************************************
**
** Converts a UFO Uniform Feature Object into an open output file.
**
** First tests for "format::" and sets format if it is found
**
** Then checks for a filename, and defaults to the sequence name (as stored
** in the AjPFeattabOut) or "unknown" if there is no name.
**
** @param [u] thys [AjPFeattabOut] Feature table to be written.
** @param [r] ufo [const AjPStr] UFO.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool featoutUfoProcess(AjPFeattabOut thys, const AjPStr ufo)
{   
    AjBool fmtstat = ajFalse;	/* status returns from regex tests */
    AjBool filstat = ajFalse;	/* status returns from regex tests */

    AjPFeattabOut featout = thys;
    
    if(!featRegUfoFmt)
	featRegUfoFmt = ajRegCompC("^([A-Za-z0-9]*):+(.*)$");

    /*
    ** \1 format
    ** \2 remainder
    */
    if(!featRegUfoFile)
	featRegUfoFile = ajRegCompC("^([^:]+)$");
    
    /*ajDebug("featoutUfoProcess UFO '%S'\n", ufo);*/
    
    ajStrAssignS(&featUfoTest, ufo);
    
    if(ajStrGetLen(ufo))
    {
	fmtstat = ajRegExec(featRegUfoFmt, featUfoTest);
	/*ajDebug("feat format regexp: %B\n", fmtstat);*/
    }
    
    if(fmtstat)
    {
	ajRegSubI(featRegUfoFmt, 1, &featout->Formatstr);
	ajStrAssignEmptyC(&featout->Formatstr,
		  featOutFormat[0].Name); /* unknown */
	ajRegSubI(featRegUfoFmt, 2, &featUfoTest);   /* trim off the format */
	/*ajDebug("found feat format %S\n", featout->Formatstr);*/
    }
    else
    {
	/*ajDebug("no feat format specified in UFO '%S' try '%S' or 'gff'\n",
		ufo, featout->Formatstr);*/
	ajStrAssignEmptyC(&featout->Formatstr, "gff");
    }
    
    if(!featFindOutFormatS(featout->Formatstr, &featout->Format))
	ajErr("unknown output feature format '%S' "
	      "will write as gff3 instead\n",
	      featout->Formatstr );

    /* now go for the filename */
    
    filstat = ajRegExec(featRegUfoFile, featUfoTest);

    /*ajDebug("featRegUfoFile: %B\n", filstat);*/

    if(filstat)
    {
	ajRegSubI(featRegUfoFile, 1, &featout->Filename);
	/*ajDebug("set from UFO featout Filename '%S'\n",
	  featout->Filename);*/
    }
    else
    {
	if(ajStrGetLen(featout->Basename))
	    ajFmtPrintS(&featUfoTest, "%S.%S", featout->Basename,
			featout->Formatstr);
	else if(ajStrGetLen(featout->Seqname))
	    ajFmtPrintS(&featUfoTest, "%S.%S", featout->Seqname,
			featout->Formatstr);
	else
	    ajFmtPrintS(&featUfoTest, "unknown.%S", featout->Formatstr);

	ajStrAssignEmptyS(&featout->Filename, featUfoTest);
	/*ajDebug("generate featout filename '%S' dir '%S'\n",
	        featout->Filename, featout->Directory);*/
    }
    
    /*ajDebug("\n");*/
    
    return ajTrue;
}




/* @funcstatic featFindInFormatC **********************************************
**
** Looks for the specified format(s) in the internal definitions and
** returns the index.
**
** Given a single format, sets iformat.
**
** @param [r] format [const char*] Format required.
** @param [w] iformat [ajint*] Index
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool featFindInFormatC(const char* format, ajint* iformat)
{
    ajint i = 0;

    /*ajDebug("featFindInFormatC '%s'\n", format);*/
    if(!*format)
	return ajFalse;

    ajStrAssignC(&featFormatTmp, format);
    ajStrFmtLower(&featFormatTmp);

    for(i=0; featInFormat[i].Name; i++)
    {
	/*ajDebug("test %d '%s' \n", i, featInFormat[i].Name);*/
	if(ajStrMatchC(featFormatTmp,
		       featInFormat[i].Name))
	{
	    *iformat = i;
	    (void) ajStrDelStatic(&featFormatTmp);
	    /*ajDebug("found '%s' at %d\n", featInFormat[i].Name, i);*/
	    return ajTrue;
	}
    }

    ajErr("Unknown input feat format '%s'", format);

    ajStrDelStatic(&featFormatTmp);

    return ajFalse;
}




/* @funcstatic featFindInFormatS **********************************************
**
** Looks for the specified format(s) in the internal definitions and
** returns the index.
**
** Given a single format, sets iformat.
**
** @param [r] format [const AjPStr] Format required.
** @param [w] iformat [ajint*] Index
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool featFindInFormatS(const AjPStr format, ajint* iformat)
{
    return featFindInFormatC(ajStrGetPtr(format), iformat);
}




/* @funcstatic featFindOutFormatC *********************************************
**
** Looks for the specified format(s) in the internal definitions and
** returns the index.
**
** Given a single format, sets iformat.
**
** @param [r] format [const char*] Format required.
** @param [w] iformat [ajint*] Index
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool featFindOutFormatC(const char* format, ajint* iformat)
{
    ajint i = 0;

    /*ajDebug("featFindOutFormatC '%s'\n", format);*/
    if(!*format)
	return ajFalse;

    ajStrAssignC(&featFormatTmp, format);
    ajStrFmtLower(&featFormatTmp);

    for(i=0; featOutFormat[i].Name; i++)
    {
	/*ajDebug("test %d '%s' len=%d\n",
		i, featOutFormat[i].Name,ajStrGetLen(featFormatTmp));*/
	if(ajStrMatchC(featFormatTmp,
		       featOutFormat[i].Name))
	{
	    *iformat = i;
	    ajStrDelStatic(&featFormatTmp);
	    /*ajDebug("found '%s' at %d\n", featOutFormat[i].Name, i);*/

	    return ajTrue;
	}
    }


    ajStrDelStatic(&featFormatTmp);
    *iformat = 1;

    return ajFalse;
}




/* @funcstatic featFindOutFormatS *********************************************
**
** Looks for the specified format(s) in the internal definitions and
** returns the index.
**
** Given a single format, sets iformat.
**
** @param [r] format [const AjPStr] Format required.
** @param [w] iformat [ajint*] Index
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool featFindOutFormatS(const AjPStr format, ajint* iformat)
{
    return featFindOutFormatC(ajStrGetPtr(format), iformat);
}




/* @func ajFeatOutFormatDefault ***********************************************
**
** Sets the default output format.
** Checks the _OUTFEATFORMAT variable,
** and uses GFF3 if no other definition is found.
**
** @param [w] pformat [AjPStr*] Default output feature format.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajFeatOutFormatDefault(AjPStr* pformat)
{
    if(ajStrGetLen(*pformat))
    {
	/*ajDebug("... output feature format '%S'\n", *pformat);*/
    }
    else
    {
	if(ajNamGetValueC("outfeatformat", pformat))
	{
	    /*ajDebug("ajFeatOutFormatDefault '%S' from EMBOSS_OUTFEATFORMAT\n",
		     *pformat);*/
	}
	else
	{
	    ajStrAssignEmptyC(pformat, "gff"); /* use the real name */
	    /*ajDebug("... output feature format not set, default to '%S'\n",
		    *pformat);*/
	}
    }

    return ajTrue;
}




/* @func ajFeattableWrite *****************************************************
**
** Generic interface function for writing features to a file
** defined by a feature table output object.
**
** @param  [u] ftout   [AjPFeattabOut]  Specifies the external source
**                                       (file) of the features to be written
** @param  [r] features [const AjPFeattable]  Feature set to be written out
** @return [AjBool]  Returns ajTrue if successful; ajFalse otherwise
** @@
******************************************************************************/

AjBool ajFeattableWrite(AjPFeattabOut ftout, const AjPFeattable features)
{
    AjBool result = ajFalse;

    AjPFile file;
    ajint format;
    AjBool isnuc = ajFalse;
    AjBool isprot = ajFalse;
    AjBool ok = ajFalse;

    if(features)
    {
	if(!ftout)
	    return ajFalse;

	if(ajFeattableIsNuc(features))
	    isnuc = ajTrue;

	if(ajFeattableIsProt(features))
	    isprot = ajTrue;

	ok = ajFalse;

	if(isnuc && featOutFormat[ftout->Format].Nucleotide)
	    ok = ajTrue;
	else if(isprot && featOutFormat[ftout->Format].Protein)
	    ok = ajTrue;

	if(!ok)
	{
	    if(isnuc)
		ajErr("Feature format '%S' not supported for nucleotide "
                      "sequence features",
		      ftout->Formatstr);
	    else if(isprot)
		ajErr("Feature format '%S' not supported for protein "
                      "sequence features",
		      ftout->Formatstr);
	    else
		ajErr("Feature format '%S' failed: unknown sequence feature "
                      "type",
		      ftout->Formatstr);
	    return ajFalse;
	}

	/*ajDebug("ajFeattableWrite Validating arguments\n");*/

	file = ftout->Handle;

	if(!file)
	    return ajFalse;

	format  = ftout->Format;

	/*ajDebug("ajFeattableWrite format is %d OK\n",ftout->Format);*/

	featOutFormat[format].VocInit();
	result = featOutFormat[format].Write(ftout, features);

        ftout->Count++;
	return result;
    }

   /* ajDebug(" NO Features to output\n");*/
    return AJTRUE;
}




/* @obsolete ajFeatWrite
** @rename ajFeattableWrite
*/

__deprecated AjBool ajFeatWrite(AjPFeattabOut ftout,
                                const AjPFeattable features)
{
    return ajFeattableWrite(ftout, features);
}




/* @funcstatic featReadEmbl ***************************************************
**
** Reads feature data in EMBL format. Also handles GenBank, RefSeq, RefSeqP
**
** @param [u] thys [AjPFeattable] Feature table
** @param [u] file [AjPFilebuff] Buffered input file
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool featReadEmbl(AjPFeattable thys, AjPFilebuff file)
{
    AjBool found           = ajFalse;
    AjPStr savefeat = NULL;
    AjPStr saveline = NULL;
    AjPStr saveloc  = NULL;
    AjBool isGenbank = ajFalse;

    if(!featReadLine)
	featReadLine = ajStrNewRes(100);

    ajFeattableSetNuc(thys);

    while(ajBuffreadLine(file, &featReadLine))
    {
	/* if it's an EMBL feature do stuff */
	if(ajStrPrefixC(featReadLine, "FEATURES  "))
            isGenbank = ajTrue;
	else if(ajStrPrefixC(featReadLine, "FT   "))
	{
	    ajStrTrimWhiteEnd(&featReadLine); /* remove newline */

	    if(featEmblFromLine(thys, featReadLine,
				&savefeat, &saveloc, &saveline))
		found = ajTrue ;
	}

	/* if it's a GenBank feature do stuff */
	else if(isGenbank && ajStrPrefixC(featReadLine, "     "))
	{
	    ajStrTrimWhiteEnd(&featReadLine); /* remove newline */

	    if(featEmblFromLine(thys, featReadLine,
				&savefeat, &saveloc, &saveline))
		found = ajTrue ;
	}
        else if(isGenbank) 
        {
            isGenbank = ajFalse;
        }
        
    }

    if(featEmblFromLine(thys, NULL, &savefeat, &saveloc, &saveline))
	found = ajTrue;

    ajStrDel(&saveloc);
    ajStrDel(&saveline);
    ajStrDel(&savefeat);

    return found;
}




/* @funcstatic featReadGenpept ************************************************
**
** Reads feature data in GENPEPT format
**
** @param [u] thys [AjPFeattable] Feature table
** @param [u] file [AjPFilebuff] Buffered input file
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool featReadGenpept(AjPFeattable thys, AjPFilebuff file)
{
    AjBool found           = ajFalse;
    AjPStr savefeat = NULL;
    AjPStr saveline = NULL;
    AjPStr saveloc  = NULL;

    if(!featReadLine)
	featReadLine = ajStrNewRes(100);

    ajFeattableSetProt(thys);

    while(ajBuffreadLine(file, &featReadLine))
    {
	if(ajStrPrefixC(featReadLine, "     "))
	{
	    ajStrTrimWhiteEnd(&featReadLine); /* remove newline */

	    if(featEmblFromLine(thys, featReadLine,
				&savefeat, &saveloc, &saveline))
		found = ajTrue ;
	}
    }

    if(featEmblFromLine(thys, NULL, &savefeat, &saveloc, &saveline))
	found = ajTrue;

    ajStrDel(&saveloc);
    ajStrDel(&saveline);
    ajStrDel(&savefeat);

    return found;
}




/* @funcstatic featReadRefseq *************************************************
**
** Reads feature data in REFSEQ format
**
** @param [u] thys [AjPFeattable] Feature table
** @param [u] file [AjPFilebuff] Buffered input file
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool featReadRefseq(AjPFeattable thys, AjPFilebuff file)
{
    AjBool found           = ajFalse;
    AjPStr savefeat = NULL;
    AjPStr saveline = NULL;
    AjPStr saveloc  = NULL;

    if(!featReadLine)
	featReadLine = ajStrNewRes(100);

    ajFeattableSetNuc(thys);

    while(ajBuffreadLine(file, &featReadLine))
    {
	if(ajStrPrefixC(featReadLine, "     "))
	{
	    ajStrTrimWhiteEnd(&featReadLine); /* remove newline */

	    if(featEmblFromLine(thys, featReadLine,
				&savefeat, &saveloc, &saveline))
		found = ajTrue ;
	}
    }

    if(featEmblFromLine(thys, NULL, &savefeat, &saveloc, &saveline))
	found = ajTrue;

    ajStrDel(&saveloc);
    ajStrDel(&saveline);
    ajStrDel(&savefeat);

    return found;
}




/* @funcstatic featReadRefseqp ************************************************
**
** Reads feature data in REFSEQP format
**
** @param [u] thys [AjPFeattable] Feature table
** @param [u] file [AjPFilebuff] Buffered input file
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool featReadRefseqp(AjPFeattable thys, AjPFilebuff file)
{
    AjBool found           = ajFalse;
    AjPStr savefeat = NULL;
    AjPStr saveline = NULL;
    AjPStr saveloc  = NULL;

    if(!featReadLine)
	featReadLine = ajStrNewRes(100);

    ajFeattableSetProt(thys);

    while(ajBuffreadLine(file, &featReadLine))
    {
	if(ajStrPrefixC(featReadLine, "     "))
	{
	    ajStrTrimWhiteEnd(&featReadLine); /* remove newline */

	    if(featRefseqpFromLine(thys, featReadLine,
                                   &savefeat, &saveloc, &saveline))
		found = ajTrue ;
	}
    }

    if(featRefseqpFromLine(thys, NULL, &savefeat, &saveloc, &saveline))
	found = ajTrue;

    ajStrDel(&saveloc);
    ajStrDel(&saveline);
    ajStrDel(&savefeat);

    return found;
}




/* @funcstatic featReadPir ****************************************************
**
** Reads feature data in PIR format
**
** @param [u] thys [AjPFeattable] Feature table
** @param [u] file [AjPFilebuff] Buffered input file
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool featReadPir(AjPFeattable thys, AjPFilebuff file)
{
    AjBool found = ajFalse;

    /*ajDebug("featReadPir..........\n");*/

    if(!featReadLine)
	featReadLine = ajStrNewRes(100);

    while(ajBuffreadLine(file, &featReadLine))
    {
	ajStrTrimWhite(&featReadLine);

	/* ajDebug("++ line '%S'\n", line); */

	if(ajStrPrefixC(featReadLine, "F;"))
	{
	    if(featPirFromLine(thys, featReadLine))
		found = ajTrue;
	}
    }

    return found;
}




/* @funcstatic featPirFromLine ************************************************
**
** Read input file line in PIR format
**
** Format is :-
** F;position/type: note #comment
**
** @param [u] thys [AjPFeattable] Feature table
** @param [r] origline [const AjPStr] Input line
** @return [AjPFeature] New feature.
** @@
******************************************************************************/

static AjPFeature featPirFromLine(AjPFeattable thys,
				  const AjPStr origline)
{
    AjPStr temp     = NULL;
    static AjPFeature gf   = NULL ;    /* made static so that it's easy
					  to add second line of description */
    AjPStr locstr   = NULL;
    AjPStr typstr   = NULL;
    AjPStr notestr  = NULL;
    AjPStr comstr   = NULL;
    AjPStr exonstr  = NULL;
    AjPStr posstr   = NULL;
    ajint i = 0;
    AjBool mother = ajTrue;
    ajint Start = 0;
    ajint End   = 0;
    ajint Flags = 0;
    
    /*ajDebug("featPirFromLine..........\n'%S'\n", origline);*/
    
    if(!featTagNote)
	ajStrAssignC(&featTagNote, "note");

    if(!featTagComm)
	ajStrAssignC(&featTagComm, "comment");

    if(!featSourcePir)
	featSourcePir = ajStrNewC("PIR");
    
    if(!ajRegExec(PirRegexAll, origline))
	return NULL;
    
    ajRegSubI(PirRegexAll, 1, &locstr);
    ajRegSubI(PirRegexAll, 2, &typstr);
    ajRegSubI(PirRegexAll, 3, &notestr);
    
    /* remove spaces in feature type so we can look it up */
    
    ajStrExchangeCC(&typstr, " ", "_");
    
    featTypePirIn(&typstr);
    ajStrRemoveWhiteExcess(&notestr);
    
    /* decode the position(s) */
    
    while(ajRegExec(PirRegexLoc, locstr)) /* split at ',' */
    {
	ajRegSubI(PirRegexLoc, 1, &exonstr);
	ajRegPost(PirRegexLoc, &temp);
	ajStrAssignS(&locstr, temp);
	i = 0;

	while(ajRegExec(PirRegexPos, exonstr)) /* split at '-' */
	{
	    ajRegSubI(PirRegexPos, 1, &posstr);

	    if(!i++)
		if(!ajStrToInt(posstr, &Start))
		    Start = 1;

	    ajRegPost(PirRegexPos, &temp);
	    ajStrAssignS(&exonstr, temp);
	}

	if(!ajStrToInt(posstr, &End))
	    End = 1;

	gf = featFeatNewProt(thys,
			     featSourcePir,	/* source sequence */
			     typstr,
			     Start, End,
			     0.0,
			     Flags);

	/* for the first feature, process the rest of the tags */

	if(mother)
	{
	    if(ajStrGetLen(notestr))
		ajFeatTagAdd(gf, featTagNote, notestr);

	    ajRegPost(PirRegexAll, &temp);

	    while(ajRegExec(PirRegexCom, temp))
	    {
		ajRegSubI(PirRegexCom, 1, &comstr);
		ajStrRemoveWhiteExcess(&comstr);
		ajFeatTagAdd(gf, featTagComm, comstr);
		ajRegPost(PirRegexCom, &temp);
	    }
	}

	mother = ajFalse;
	Flags |= FEATFLAG_CHILD;
    }
    
    if(mother)
	featWarn("featPirFromLine: Did not understand location '%S'", locstr);

    ajStrDel(&temp);
    ajStrDel(&locstr);
    ajStrDel(&typstr);
    ajStrDel(&notestr);
    ajStrDel(&comstr);
    ajStrDel(&exonstr);
    ajStrDel(&posstr);

    return gf;
}




/* @funcstatic featReadSwiss **************************************************
**
** Reads feature data in SwissProt format
**
** @param [u] thys [AjPFeattable] Feature table
** @param [u] file [AjPFilebuff] Buffered input file
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool featReadSwiss(AjPFeattable thys, AjPFilebuff file)
{
    AjBool found           = ajFalse;
    AjPStr savefeat = NULL;
    AjPStr saveline = NULL;
    AjPStr savefrom = NULL;
    AjPStr saveto   = NULL;

    /*ajDebug("featReadSwiss..........\n");*/

    if(!featReadLine)
	featReadLine = ajStrNewRes(100);

    while(ajBuffreadLine(file, &featReadLine))
    {
	ajStrTrimWhite(&featReadLine);

	/* ajDebug("++ line '%S'\n", line); */

	if(ajStrPrefixC(featReadLine, "FT   "))
	    if(featSwissFromLine(thys, featReadLine,
				 &savefeat, &savefrom, &saveto, &saveline))
		found = ajTrue ;
    }

    if(featSwissFromLine(thys, NULL,
			  &savefeat, &savefrom, &saveto, &saveline))
	found = ajTrue;

    /*ajDebug("featReadSwiss returns %B\n", found);*/

    ajStrDel(&savefeat);
    ajStrDel(&savefrom);
    ajStrDel(&saveto);
    ajStrDel(&saveline);

    return found;
}




/* ==================================================================== */
/* ======================== GFF Processing functions ================== */
/* =================================================================== */

/* ajfeat defaults to version 3 GFF only...*/
#define DEFAULT_GFF_VERSION 3




/* @funcstatic featFlagSet ****************************************************
**
** Sets the flags for a feature.
**
** @param [u] gf       [AjPFeature]  Feature
** @param [r] flagstr  [const AjPStr] Flags as a hexadecimal value
** @return [void]
** @@
******************************************************************************/

static void featFlagSet(AjPFeature gf, const AjPStr flagstr)
{
    AjPStr savstr     = NULL;
    AjPStr typstr     = NULL;
    AjPStr valstr     = NULL;
    ajint flags = 0;
    ajint num   = 0;

    if(!featRegFlag)
	featRegFlag = ajRegCompC("[ \"]*(0x[0-9a-f]+)");

    if(!featRegMore)
	featRegMore = ajRegCompC("[ \"]*([^:]+):([^: \"]+)");

    /*ajDebug("featFlagSet '%S'\n", flagstr);*/
    ajStrAssignS(&savstr, flagstr);

    if(ajRegExec(featRegFlag, savstr))
    {
	ajRegSubI(featRegFlag, 1, &featTmpStr);

	if(ajStrToHex(featTmpStr, &flags))
        {
            if(flags & FEATFLAG_GROUP)
                flags |= FEATFLAG_ORDER;

            if(flags & FEATFLAG_ONEOF)
                flags |= FEATFLAG_ORDER;

	    gf->Flags = flags;
        }

	/*ajDebug("flags: %x", gf->Flags);*/
	ajRegPost(featRegFlag, &featTmpStr);
	ajStrAssignS(&savstr, featTmpStr);
    }

    while(ajRegExec(featRegMore, savstr))
    {
	ajRegSubI(featRegMore, 1, &typstr);
	ajRegSubI(featRegMore, 2, &valstr);

	/*ajDebug("flag type '%S' val '%S'\n", typstr, valstr);*/

	if(ajStrMatchCaseC(typstr, "start2"))
	{
	    if(ajStrToInt(valstr, &num))
		gf->Start2 = num;
	}
	else if(ajStrMatchCaseC(typstr, "end2"))
	{
	    if(ajStrToInt(valstr, &num))
	       gf->End2 = num;
	}
	else if(ajStrMatchCaseC(typstr, "remoteid"))
	    ajStrAssignS(&gf->Remote, valstr);
	else if(ajStrMatchCaseC(typstr, "label"))
	{
	    featWarn("GFF label '%S' used", valstr);
	    ajStrAssignS(&gf->Label, valstr);
	}
	else
	    featWarn("Unknown GFF FeatFlags type '%S:%S'", typstr, valstr);

	ajRegPost(featRegMore, &featTmpStr);
	ajStrAssignS(&savstr, featTmpStr);
    }

    ajStrDel(&savstr);
    ajStrDel(&typstr);
    ajStrDel(&valstr);

    return;
}




/* @funcstatic featGroupSet ***************************************************
**
** Sets the group tag for a feature.
**
** @param [u] gf       [AjPFeature]  Feature
** @param [u] table    [AjPFeattable] Feature table
** @param [r] grouptag [const AjPStr]      Group field identifier
** @return [void]
** @@
******************************************************************************/

static void featGroupSet(AjPFeature gf, AjPFeattable table,
			 const AjPStr grouptag)
{
    AjPStr namstr      = NULL;
    AjPStr grpstr      = NULL;
    ajint grpnum;

    if(!featRegGroup)
	featRegGroup = ajRegCompC("^\"(([^.]*)[.])?([0-9]+)");

    if(ajStrGetLen(grouptag) && ajStrMatchCaseS(grouptag, featSaveGroupStr))
    {
	gf->Group = featSaveGroup;
	gf->Exon  = ++featSaveExon;

	return;
    }


    if(ajStrGetLen(grouptag) && ajRegExec(featRegGroup, grouptag))
    {
	ajStrAssignS(&featSaveGroupStr, grouptag);
	ajRegSubI(featRegGroup, 2, &namstr);
	ajRegSubI(featRegGroup, 3, &grpstr);

	/*ajDebug("featGroupSet '%S' name: '%S' group: '%S'\n",
	        grouptag, namstr, grpstr);*/

	if(ajStrToInt(grpstr, &grpnum)) /* true, if the regex worked */
	{
	    gf->Group = grpnum;
	    featSaveGroup = grpnum;
	}
	else
	    gf->Group = ++(table->Groups);

	if(ajStrGetLen(namstr))
	{
	    if(!ajStrMatchCaseS(namstr, table->Seqid))
	    {
		ajDebug("GFF group field '%S' table '%S'\n",
			 grouptag, table->Seqid);
		featWarn("GFF group field '%S' for table '%S'",
		       grouptag, table->Seqid);
	    }
	}
    }
    else			 /* regex failed, make something up */
    {
	ajStrAssignS(&grpstr, grouptag);
	gf->Group = ++(table->Groups);
	featSaveGroup = gf->Group;
	gf->Exon  = 0;
	featSaveExon  = 0;
    }
    ajStrDel(&namstr);
    ajStrDel(&grpstr);

    return;
}




/* @funcstatic featGffProcessTagval *******************************************
**
** Parses and adds a tag-value set to the specified AjPFeature;
** looked at 'parse_group' method in GFF::GeneFeature.pm Perl module
** for inspiration
**
** @param [u] gf [AjPFeature] Feature
** @param [u] table [AjPFeattable] Feature table
** @param [r] groupfield [const AjPStr] Group field identifier
** @param [r] version [float] GFF version
** @return [void]
** @@
******************************************************************************/

static void featGffProcessTagval(AjPFeature gf, AjPFeattable table,
				 const AjPStr groupfield, float version)
{
    AjPStr  TvString  = NULL;
    AjPStr  tmptag      = NULL;
    AjBool  grpset      = ajFalse;
    
    /*ajDebug("featGffProcessTagval version %3.1f '%S'\n",
      version, groupfield); */
    
    /* Validate arguments */
    if(!ajStrGetLen(groupfield))	/* no tags, must be new */
	return;
    
    if(E_FPEQ(version,1.0F,U_FEPS))
    {
	featGroupSet(gf, table, groupfield) ;
	/*ajDebug("V1.0 group: '%S'\n", groupfield);*/
	grpset = ajTrue;

	return;
    }

    /*
     *     Version 2 or greater: parse groupfield for semicolon ';'
     *     delimited tag-value structures, taking special care about
     *     double quoted string context. rbsk first version of code was
     *     adapted from GFF.pm (th/rbsk), itself inherited from AceParse.pm,
     *     courtesy of James Gilbert
     */
    
    ajStrAssignS(&TvString, groupfield);

    while(ajStrGetLen(TvString))
    {
	if(ajRegExec(GffRegexTvTagval, TvString))
	{
	    ajRegSubI(GffRegexTvTagval, 1, &tmptag);
	    ajRegSubI(GffRegexTvTagval, 2, &featValTmp);
	    ajStrTrimWhite(&featValTmp);
	    /*ajDebug("GffTv '%S' '%S'\n", tmptag, featValTmp);*/
	    ajRegPost(GffRegexTvTagval, &TvString);

	    if(ajStrMatchC(tmptag, "Sequence"))
	    {
		featGroupSet(gf, table, featValTmp);
		grpset = ajTrue;
	    }
	    else if(ajStrMatchC(tmptag, "ID"))
	    {
		featGroupSet(gf, table, featValTmp);
		grpset = ajTrue;
	    }
	    else if(ajStrMatchC(tmptag, "FeatFlags"))
		featFlagSet(gf, featValTmp);
	    else if(ajStrMatchC(tmptag, "featflags"))
		featFlagSet(gf, featValTmp);
	    else
	    {
		/*ajDebug("Before QuoteStrip: '%S'\n", featValTmp);*/
		ajStrQuoteStrip(&featValTmp);
		/*ajDebug(" After QuoteStrip: '%S'\n", featValTmp);*/
		ajFeatTagAdd(gf,tmptag,featValTmp) ;
	    }
	}
	else
	{
	    /*ajDebug("Choked on '%S'\n", TvString);*/
	    featWarn("GFF tag parsing failed on '%S' in\n'%S'",
		   TvString, groupfield);
	}
    }
    
    if(!grpset)
    {
	featGroupSet(gf, table, NULL);
	grpset = ajTrue;
    }

    ajStrDel(&TvString);
    ajStrDel(&tmptag);
    
    return;
}




/* @funcstatic featGff3ProcessTagval ******************************************
**
** Parses and adds a tag-value set to the specified AjPFeature
**
** @param [u] gf [AjPFeature] Feature
** @param [u] table [AjPFeattable] Feature table
** @param [r] groupfield [const AjPStr] Group field identifier
** @return [void]
** @@
******************************************************************************/

static void featGff3ProcessTagval(AjPFeature gf, AjPFeattable table,
				 const AjPStr groupfield)
{
    AjPStr  TvString  = NULL;
    AjPStr  tmptag      = NULL;
    AjBool  grpset      = ajFalse;
    
    /*ajDebug("featGff3ProcessTagval '%S'\n",
      groupfield); */
    
    /* Validate arguments */
    if(!ajStrGetLen(groupfield))	/* no tags, must be new */
	return;
    
    /*
     *     parse groupfield for semicolon ';'
     *     delimited tag-value structures, taking special care about
     *     double quoted string context.
     */
    
    ajStrAssignS(&TvString, groupfield);
    while(ajStrGetLen(TvString))
    {
	if(ajRegExec(Gff3RegexTvTagval, TvString))
	{
	    ajRegSubI(Gff3RegexTvTagval, 1, &tmptag);
	    ajRegSubI(Gff3RegexTvTagval, 2, &featValTmp);
	    ajStrTrimWhite(&featValTmp);
	    /*ajDebug("Gff3Tv '%S' '%S'\n", tmptag, featValTmp);*/
	    ajRegPost(Gff3RegexTvTagval, &TvString);

	    if(ajStrMatchC(tmptag, "Sequence"))
	    {
		featGroupSet(gf, table, featValTmp);
		grpset = ajTrue;
	    }
	    else if(ajStrMatchC(tmptag, "ID"))
	    {
		featGroupSet(gf, table, featValTmp);
		grpset = ajTrue;
	    }
	    else if(ajStrMatchC(tmptag, "FeatFlags"))
		featFlagSet(gf, featValTmp);
	    else if(ajStrMatchC(tmptag, "featflags"))
		featFlagSet(gf, featValTmp);
	    else
	    {
		/*ajDebug("GFF3 before QuoteStrip: '%S'\n", featValTmp);*/
		ajStrQuoteStrip(&featValTmp);
		/*ajDebug(" GFF3 after QuoteStrip: '%S'\n", featValTmp);*/
		ajFeatTagAdd(gf,tmptag,featValTmp) ;
	    }
	}
	else
	{
	    /*ajDebug("GFF3 choked on '%S'\n", TvString);*/
	    featWarn("GFF3 tag parsing failed on '%S' in\n'%S'",
		   TvString, groupfield);
	}
    }
    
    if(!grpset)
    {
	featGroupSet(gf, table, NULL);
	grpset = ajTrue;
    }

    ajStrDel(&TvString);
    ajStrDel(&tmptag);
    
    return;
}




/* @funcstatic featSwissFromLine **********************************************
**
** Read input file in Swiss format
**
** Format is :-
** 0-1    FT
** 5-12   Keyname
** 14-19  From
** 21-26  To
** 34-74  Description
**
** @param [u] thys [AjPFeattable] Feature table
** @param [r] origline [const AjPStr] Input line
** @param [w] savefeat [AjPStr*] Stored feature type
** @param [w] savefrom [AjPStr*] Continued from position
** @param [w] saveto   [AjPStr*] Continued to position
** @param [w] saveline [AjPStr*] Continued tag-value pairs
** @return [AjPFeature] New feature.
** @@
******************************************************************************/

static AjPFeature featSwissFromLine(AjPFeattable thys,
				    const AjPStr origline,
				    AjPStr* savefeat,
				    AjPStr* savefrom,
				    AjPStr* saveto,
				    AjPStr* saveline)
{
    AjPStr temp     = NULL;
    static AjPFeature gf   = NULL;     /* made static so that it's easy
					  to add second line of description */
    AjBool newft = ajFalse;
    
    /*ajDebug("featSwissFromLine..........\n'%S'\n", origline);*/
    
    if(!featSourceSwiss)
 	featSourceSwiss = ajStrNewC("SWISSPROT");
    
    if(origline)
	newft = ajRegExec(SwRegexNew, origline);

    
    if(newft || !origline)		/* process the last feature */
    {
	/* ajDebug("++ feat+from+to '%S' '%S' '%S'\n+ saveline '%S'\n",
		 *savefeat, *savefrom, *saveto, *saveline); */

	if(ajStrGetLen(*savefrom))      /* finish the current feature */
	    gf = featSwissProcess(thys, *savefeat, *savefrom, *saveto,
				  featSourceSwiss, *saveline);
	else 			   /*  maybe there were no features */
	    gf = NULL;

	ajStrDel(savefeat);
	ajStrDel(savefrom);
	ajStrDel(saveto);
	ajStrDel(saveline);
    }
    
    if(!origline)		/* we are only cleaning up */
	return gf;
    
    if(newft) 		/* if new feature initialise for it */
    {
	ajRegSubI(SwRegexNew, 2, savefeat);
	ajRegSubI(SwRegexNew, 3, savefrom);
	ajRegSubI(SwRegexNew, 4, saveto);
	ajRegSubI(SwRegexNew, 5, saveline);
	ajStrTrimWhite(savefeat);
	/*ajDebug(" newft type '%S' from '%S' to '%S' rest '%S'\n",
		*savefeat, *savefrom, *saveto, *saveline);*/
	return gf;
    }
    else              /* more tag-values */
    {
	if(ajRegExec(SwRegexNext, origline))
	{
	    ajRegSubI(SwRegexNext, 1, &temp);
	    ajStrAppendC(saveline, " ");
	    ajStrAppendS(saveline, temp);
	}
	else
	    featWarn("%S: Bad SwissProt feature line:\n%S",
		   thys->Seqid, origline);
    }
    
    ajStrDel(&temp);

    return gf;    
}




/* @funcstatic featSwissProcess ***********************************************
**
** Processes one feature location and qualifier tags for SwissProt
**
** @param [u] thys [AjPFeattable] Feature table
** @param [r] feature [const AjPStr] Feature type key
** @param [r] fromstr [const AjPStr] Feature start
** @param [r] tostr [const AjPStr] Feature end
** @param [r] source [const AjPStr] Feature table source
** @param [r] tags [const AjPStr] Feature qualifier tags string
** @return [AjPFeature] Feature as inserted into the feature table
** @@
******************************************************************************/

static AjPFeature featSwissProcess(AjPFeattable thys, const AjPStr feature,
				   const AjPStr fromstr, const AjPStr tostr,
				   const AjPStr source,
				   const AjPStr tags)
{   
    AjPFeature ret;
    ajint Start = 0;
    ajint End   = 0;
    ajint flags = 0;

    AjPStr note    = NULL;
    AjPStr comment = NULL;
    AjPStr ftid    = NULL;
    AjPStr tagstr  = NULL;
    
    if(!featTagNote)
	ajStrAssignC(&featTagNote, "note");

    if(!featTagComm)
	ajStrAssignC(&featTagComm, "comment");

    if(!featTagFtid)
	ajStrAssignC(&featTagFtid, "ftid");
    
    switch(ajStrGetCharFirst(fromstr))
    {
        case '?':
            flags |= FEATFLAG_START_UNSURE;
            ajStrAssignS(&featTmpStr, fromstr);
            ajStrCutStart(&featTmpStr, 1);

            if(!ajStrToInt(featTmpStr, &Start))
                Start = 0;

            break;
        case '<':
        case '>':				/* just to be sure */
            flags |= FEATFLAG_START_BEFORE_SEQ;
            ajStrAssignS(&featTmpStr, fromstr);
            ajStrCutStart(&featTmpStr, 1);

            if(!ajStrToInt(featTmpStr, &Start))
                Start = 0;

            break;

        default:
            if(!ajStrToInt(fromstr, &Start))
                Start = 0;
    }
    
    switch(ajStrGetCharFirst(tostr))
    {
        case '?':
            flags |= FEATFLAG_END_UNSURE;
            ajStrAssignS(&featTmpStr, tostr);
            ajStrCutStart(&featTmpStr, 1);

            if(!ajStrToInt(featTmpStr, &End))
                End = 0;

            break;
        case '<':				/* just to be sure */
        case '>':
            flags |= FEATFLAG_END_AFTER_SEQ;
            ajStrAssignS(&featTmpStr, tostr);
            ajStrCutStart(&featTmpStr, 1);

            if(!ajStrToInt(featTmpStr, &End))
                End = 0;

            break;
        default:
            if(!ajStrToInt(tostr, &End))
                End = 0;
    }

    ajStrDelStatic(&featTmpStr);

    ret = featFeatNewProt(thys,
			  source,	/* source sequence */
			  feature,
			  Start, End,
			  0.0,
			  flags);
    
    ajStrAssignS(&tagstr, tags);
    ajStrTrimC(&tagstr, " .");
    
    if(ajRegExec(SwRegexFtid, tagstr))
    {
	ajRegSubI(SwRegexFtid, 1, &note);
	ajRegSubI(SwRegexFtid, 2, &ftid);
	/*ajDebug("Swiss ftid found\n");*/
	/*ajDebug("ftid: '%S'\n",ftid);*/
	ajStrAssignS(&tagstr, note);
	ajStrTrimC(&tagstr, " .");
    }
    
    if(ajRegExec(SwRegexComment, tagstr))
    {
	ajRegSubI(SwRegexComment, 1, &note);
	ajRegSubI(SwRegexComment, 2, &comment);
	/*ajDebug("Swiss comment found\nNote:  '%S'\nComment: '%S'\n",
		note, comment);*/
	ajStrTrimC(&note, " .");

	if(ajStrGetLen(note))
	    ajFeatTagAdd(ret, featTagNote, note);

	if(ajStrGetLen(comment))
	    ajFeatTagAdd(ret, featTagComm, comment);
    }
    else 
    {
	/*ajDebug("Simple swiss note: '%S'\n", tagstr);*/
	if(ajStrGetLen(tagstr))
	    ajFeatTagAdd(ret, featTagNote, tagstr);
    }

    if(ajStrGetLen(ftid))
        ajFeatTagAdd(ret, featTagFtid, ftid);

    ajStrDel(&note);
    ajStrDel(&comment);
    ajStrDel(&ftid);
    ajStrDel(&tagstr);

    return ret;
}




/* @funcstatic featEmblFromLine ***********************************************
**
** Converts an input EMBL format line into a feature.
** Starts a new feature by processing any existing feature data.
** Creates or appends the type, location and tag-value pairs.
** With a NULL as the input line, simply processes the type, location
** and tag-values.
**
** @param [u] thys     [AjPFeattable] Feature table
** @param [r] origline [const AjPStr] Input line (NULL to process last
**                                    feature at end of input)
** @param [w] savefeat [AjPStr*] Stored feature type
** @param [w] saveloc  [AjPStr*] Continued location
** @param [w] saveline [AjPStr*] Continued tag-value pairs
** @return [AjPFeature] New feature
** @@
******************************************************************************/

static AjPFeature featEmblFromLine(AjPFeattable thys,
				   const AjPStr origline,
				   AjPStr* savefeat,
				   AjPStr* saveloc,
				   AjPStr* saveline)
{
    static AjPFeature gf = NULL;      /* so tag-values can be added LATER */
    AjPStr temp   = NULL;
    AjBool newft         = ajFalse;
    AjBool doft         = ajFalse;

    if(!featSourceEmbl)
	featSourceEmbl = ajStrNewC("EMBL");
    
    /* ajDebug("featEmblFromLine '%S'\n", origline); */
    
    if(origline)
    {
        /* As BufferFile can't be edited */
	ajStrAssignS(&featProcessLine,origline);
        /* chop first 5 characters */
	ajStrCutStart(&featProcessLine, 5);

	/* look for the feature key */
        if(ajStrGetCharFirst(featProcessLine) != ' ')
	{
	    newft = ajTrue;

	    if(ajStrGetLen(*saveloc))
		doft = ajTrue;
	}
    }
    else
    {
	ajStrAssignClear(&featProcessLine);
	newft = ajFalse;		/* no new data, just process */

	if(ajStrGetLen(*saveloc))
	    doft = ajTrue;
    }

    /*
       ajDebug("+ newft: %B doft: %B\n+ line '%S'\n",
		newft, doft, line);
    */
    
    if(doft) 		/* process the last feature */
    {
	/*
        **   ajDebug("++ saveloc '%S'\n+ saveline '%S'\n",
	**           *saveloc, *saveline);
	*/

	gf = featEmblProcess(thys, *savefeat, featSourceEmbl,
			     saveloc, saveline);

	ajStrDelStatic(saveloc);
	ajStrDelStatic(saveline);
    }
    
    if(!origline)		/* we are only cleaning up */
	return gf;
    
    ajStrRemoveWhiteExcess(&featProcessLine);

    if(newft) 		/* if new feature initialise for it */
    {
	ajStrTokenAssignC(&featEmblSplit, featProcessLine, " ");
	ajStrTokenNextParse(&featEmblSplit, savefeat);

	if(ajStrTokenNextParseC(&featEmblSplit, " /", saveloc))
	    ajStrTokenRestParse(&featEmblSplit, saveline);
	else
	    ajStrAssignResC(saveline, 512, "");	/* location only */

	ajStrTokenDel(&featEmblSplit);

	return gf;
    }
    else if(!ajStrGetLen(*saveline))  /* no tag-values yet, more location? */
    {
	if(ajStrGetCharFirst(featProcessLine) != '/')
	{
	    ajStrTokenAssignC(&featEmblSplit, featProcessLine, " ");
	    ajStrTokenNextParse(&featEmblSplit, &temp);

	    if(ajStrGetLen(temp))
		ajStrAppendS(saveloc, temp);

	    ajStrTokenRestParse(&featEmblSplit, &temp);

	    if(ajStrGetLen(temp))
		ajStrAppendS(saveline, temp);

	    ajStrDel(&temp);
	    ajStrTokenDel(&featEmblSplit);

	    return gf;
	}
    }

    /* tag-values continued */
    ajStrAppendK(saveline, ' ');
    ajStrAppendS(saveline, featProcessLine);
    
    return gf;
}




/* @funcstatic featEmblProcess ************************************************
**
** Processes one feature location and qualifier tags for EMBL
**
** @param [u] thys [AjPFeattable] Feature table
** @param [r] feature [const AjPStr] Feature type key
** @param [r] source [const AjPStr] Feature table source
** @param [w] loc [AjPStr*] Feature location
** @param [w] tags [AjPStr*] Feature qualifier tags string
** @return [AjPFeature] Feature as inserted into the feature table
** @@
******************************************************************************/

static AjPFeature featEmblProcess(AjPFeattable thys, const AjPStr feature,
				  const AjPStr source,
				  AjPStr* loc, AjPStr* tags)
{    
    AjPFeature ret  = NULL;
    AjPFeature gf   = NULL;
    AjPStr tag      = NULL;
    AjPStr val      = NULL;
    AjPStr opnam    = NULL;
    AjPStr opval    = NULL;
    AjPStr prestr   = NULL;
    AjBool Fwd      = ajTrue;
    AjBool LocFwd   = ajTrue;
    AjPStr begstr   = NULL;
    AjPStr delstr   = NULL;
    AjPStr endstr   = NULL;
    AjPStr locstr   = NULL;
    AjPStr rest     = NULL;
    AjBool Simple   = ajFalse;	/* Simple - single position (see also label) */
    AjBool BegBound = ajFalse;
    AjBool EndBound = ajFalse;
    ajint BegNum = 0;
    ajint EndNum = 0;
    ajint Beg2   = 0;
    ajint End2   = 0;
    AjBool Between = ajFalse;
    AjBool Join    = ajFalse;
    AjBool Order   = ajFalse;
    ajint Flags;
    ajint ExonFlags;
    char Strand     = '+';
    AjBool Mother   = ajTrue;
    ajint Frame     = 0;
    float Score     = 0.0;
    AjBool HasOper  = ajFalse;
    AjBool RemoteId = ajFalse;
    AjBool IsLabel  = ajFalse;	/* uses obsolete label  */
    ajint Exon      = 0;
    ajint ipos;
    ajint itags = 0;
    const AjPStr tmpft = NULL;
    
    ajStrRemoveWhite(loc);	/* no white space needed */
    ajStrRemoveWhiteExcess(tags);		/* single spaces only */

    /*ajDebug("Clean location '%S'\n", *loc);*/
    /*ajDebug("Clean tags '%S'\n", *tags);*/
    
    ajStrAssignS(&opval, *loc);
    ipos = ajStrFindAnyK(opval, ',');	/* multiple locations */
    if(ipos >= 0)
    {
	/* ajDebug("Multiple locations, test operator(s)\n"); */
	while(ajStrGetLen(opval) &&
	      featEmblOperOut(opval, &opnam, &featTmpStr))
	{
	    if(!ajStrHasParentheses(featTmpStr))
		break;

	    /* ajDebug("OperOut %S( '%S' )\n", opnam, featTmpStr); */
	    if(ajStrMatchCaseC(opnam, "complement"))
		Fwd = !Fwd;

	    else if(ajStrMatchCaseC(opnam, "one_of"))
		Order = ajTrue;

	    else if(ajStrMatchCaseC(opnam, "join"))
		Join = ajTrue;

	    else if(ajStrMatchCaseC(opnam, "order"))
		Order = ajTrue;

	    else if(ajStrMatchCaseC(opnam, "group"))
		Order = ajTrue;

	    else
		featWarn("%S: unrecognised operator '%S()' in '%S'",
		       thys->Seqid, opnam, opval);

	    ajStrAssignS(&opval, featTmpStr);
	}
    }
    
    while(ajStrGetLen(opval))
    {
	LocFwd   = Fwd;
	BegBound = ajFalse;
	EndBound = ajFalse;
	Simple   = ajFalse;
	Between  = ajFalse;
	BegNum   = EndNum = Beg2 = End2 = 0;
	HasOper  = ajFalse;
	RemoteId = ajFalse;
	IsLabel  = ajFalse;

	ajStrDelStatic(&featId);
	ajStrDelStatic(&featLabel);

	/* check for complement() */
	/* set locstr as the whole (or rest) of the location */

	if(featEmblOperIn(opval, &opnam, &locstr, &featTmpStr))
	{
	    /* ajDebug("OperIn %S( '%S' )\n", opnam, locstr); */
	    if(ajStrMatchCaseC(opnam, "complement"))
		LocFwd = !LocFwd;

	    ajStrAssignS(&opval, featTmpStr);
	    /* ajDebug("rest: '%S'\n", opval); */
	    HasOper = ajTrue;
	}
	else
	{
	    ajStrAssignS(&locstr, opval);
	    /* ajDebug("OperIn simple '%S'\n", locstr); */
	}

	if(featEmblOperNone(locstr,
			    &featId, &featTmpStr, &rest))  /* one exon */
	{
	    /* ajDebug("OperNone '%S' \n", featTmpStr); */
	    if(ajStrGetLen(featId))
	    {
		/* ajDebug("External entryid '%S'\n", featId); */
		RemoteId = ajTrue;
	    }
	    if(!featEmblLoc(featTmpStr, &begstr, &Between, &Simple, &endstr))
	    {
		ajStrAssignS(&begstr, featTmpStr);
		ajStrAssignS(&endstr, begstr);
		Simple = ajTrue;
		ajDebug("Bad feature numeric location '%S' in '%S' - "
		   "test later for label",
		   begstr, locstr);
	    }

	    ajStrAssignS(&featTmpStr, rest);

	    if(!HasOper)
		ajStrAssignS(&opval, featTmpStr);
	    
	    if(featEmblLocNum(begstr, &BegBound, &BegNum))
	    {
		Beg2 = 0;
		/* ajDebug("Begin '%S' %d  Bound: %B\n",
		   begstr, BegNum, BegBound); */
	    }
	    else if(featEmblLocRange(begstr, &BegNum, &Beg2))
	    {
		BegBound = ajFalse;
		/* ajDebug("Begin range (%d . %d)\n", BegNum, Beg2); */
	    }
	    else
	    {
		/* ajDebug("Begin is a label '%S'\n", begstr); */
		IsLabel = ajTrue;
		Simple  = ajTrue;
		ajStrAssignS(&featLabel, begstr);
		featWarn("%S: Simple feature location '%S' in '%S'",
		       thys->Seqid, begstr, locstr);
	    }
	    
	    if(featEmblLocNum(endstr, &EndBound, &EndNum))
	    {
		End2 = 0;
		/* ajDebug("  End '%S' %d  Bound: %B\n",
		   endstr, EndNum, EndBound); */
	    }
	    else if(featEmblLocRange(endstr, &End2, &EndNum))
	    {
		EndBound = ajFalse;
		/* ajDebug("  End range (%d . %d)\n", End2, EndNum); */
	    }
	    else
	    {
		IsLabel = ajTrue;
		Simple  = ajTrue;
		ajStrAssignS(&featLabel, endstr);
		/* ajDebug("  End is a label '%S'\n", endstr); */
		ajErr("%S: Simple feature end '%S' in '%S'",
		      thys->Seqid, begstr, locstr);
	    }
	}
	else
	{
	    ajErr("Unable to parse location:\n'%S'", opval);
	}
	/* location has been read in, now store it */
	
	Flags = ExonFlags = 0;
	if(LocFwd)
	    Strand = '+';
	else
	    Strand = '-';
	
	if(Simple)
	    Flags |= FEATFLAG_POINT;
	if(Between)
	    Flags |= FEATFLAG_BETWEEN_SEQ;
	if(End2)
	    Flags |= FEATFLAG_END_TWO;
	if(Beg2)
	    Flags |= FEATFLAG_START_TWO;
	if(BegBound)
	    Flags |= FEATFLAG_START_BEFORE_SEQ;
	if(EndBound)
	    Flags |= FEATFLAG_END_AFTER_SEQ;
	if(RemoteId)
	    Flags |= FEATFLAG_REMOTEID;
	if(IsLabel)
	    Flags |= FEATFLAG_LABEL;
	if(IsLabel)
	    featWarn("%S: Feature location with label '%S'",
		   thys->Seqid, locstr);
	if(Join || Order)
	    Flags |= FEATFLAG_MULTIPLE;
	if(Order)
	    Flags |= FEATFLAG_ORDER;
	
        ExonFlags = Flags;
        
	if(Mother)
	{
	    ExonFlags |= FEATFLAG_CHILD;
	    if(!Fwd)
		Flags |= FEATFLAG_COMPLEMENT_MAIN;
	}
	else
	    Flags |= FEATFLAG_CHILD;

	/* ajDebug("Calling featFeatNew, Flags: %x\n", Flags); */
	tmpft = featTableTypeInternal(feature, FeatTypeTableEmbl);

        if(!Mother)
            Exon++;
	gf = featFeatNew(thys,
			 source,	/* source sequence */
			 tmpft,
			 BegNum, EndNum,
			 Score,
			 Strand,
			 Frame,
			 Exon, Beg2, End2,
                         featId, featLabel, Flags);
	if(Mother)
        {
	    ret = gf;
            Mother = ajFalse;
            Exon = 1;
        }
    }
    
    while(ajStrGetLen(*tags))
    {
	itags++;

	if(featEmblTvTagVal(tags, &tag, &val))
	{
	    ajStrQuoteStrip(&val);

	    if(!ajFeatTagAdd(ret, tag, val))
		featWarn("%S: Bad value '%S' for tag '/%S'",
		       thys->Seqid, val, tag) ;

            if(ajStrMatchC(tag, "codon_start"))
            {
                ajStrToInt(val, &Frame);
                gf->Frame = Frame;
            }
	}
	else if(featEmblTvRest(tags, &featTmpStr))
	{
	    /* anything non-whitespace up to '/' is bad */
	    featWarn("Bad feature syntax %S: skipping '%S'",
		   thys->Seqid, featTmpStr);
	}
	else
	{
	    featWarn("Bad feature syntax %S: giving up at '%S'",
		   thys->Seqid, *tags);
	    ajStrAssignClear(tags);
	}
   
    }

    ajDebug("featEmblProcess found %d feature tags\n", itags);

    ajStrDelStatic(&featTmpStr);
    ajStrDel(&prestr);
    ajStrDel(&val);
    ajStrDel(&tag);
    ajStrDel(&begstr);
    ajStrDel(&delstr);
    ajStrDel(&opnam);
    ajStrDel(&opval);
    ajStrDel(&locstr);
    ajStrDel(&endstr);
    ajStrDel(&rest);
    
    return ret;
}




/* @funcstatic featEmblOperOut ************************************************
**
** Process operator and contents up to matching right parenthesis at end
**
** Return operator and content of parentheses
**
** Ignores internal operators
** For example:
**
** join(complement(12..15),complement(1..3))
** returns "join" and "complement(12..15),complement(1..3)"
** 
** complement(12..15),complement(1..3)
** fails because both complement() operators cover only part of the string
** and will be processed as separate locations.
**
** @param [r] loc [const AjPStr] Feature location
** @param [w] opnam [AjPStr*] Operator name
** @param [w] value [AjPStr*] Operator value
** @return [AjBool] ajTrue if an operator was found
******************************************************************************/

static AjBool featEmblOperOut(const AjPStr loc, AjPStr* opnam, AjPStr* value)
{
    ajint left=0;
    ajint right=0;
    ajuint ipos=0;
    ajint bracepos = 0;
    const char* cp = ajStrGetPtr(loc);

    if(*cp == '(') return ajFalse;	/* starts with '(' */
    if(!isalpha((ajint)*cp)) return ajFalse; /* starts with location */
    if(!islower((ajint)*cp)) return ajFalse; /* starts with location */

    while(*cp)
    {
	ipos ++;

	switch(*cp++)
	{
            case ',':
                if(!left) return ajFalse;	/* loc,loc,oper(loc),loc */
                break;
            case '(':
                if(!left) bracepos = ipos-1;
                left++;
                break;
            case ')':
                right++;
                if(right == left)
                {
                    if(ipos != ajStrGetLen(loc)) /* partial operator */
                        return ajFalse;
                }
                else if(right > left) return ajFalse;
                break;
            default:
                break;
	}
    }

    if(!left)
        return ajFalse;		/* no parentheses */

    if(right != left)
        return ajFalse;	        /* unmatched '(' */

    
    ajStrAssignSubS(opnam, loc, 0, bracepos-1);
    ajStrAssignSubS(value, loc, bracepos+1, ipos-2);

    return ajTrue;
}




/* @funcstatic featEmblOperIn ************************************************
**
** Process operator and contents up to matching right parenthesis at end
**
** Return operator and content of parentheses
**
** For operators with single locations, though the location can include ()
** For example:
**
** complement(12..15)
**
** complement((11.12)..(14.15))
**
** @param [r] loc [const AjPStr] Feature location
** @param [w] opnam [AjPStr*] Operator name
** @param [w] value [AjPStr*] Operator value
** @param [w] rest [AjPStr*] Remainder of location
** @return [AjBool] ajTrue if an operator was found
******************************************************************************/

static AjBool featEmblOperIn(const AjPStr loc, AjPStr* opnam, AjPStr* value,
			     AjPStr* rest)
{
    ajint left=0;
    ajint right=0;
    ajint depth;
    ajuint ipos=0;
    ajint bracepos = 0;
    const char* cp = ajStrGetPtr(loc);

    if(*cp == '(')
        return ajFalse;	/* starts with '(' */

    while(*cp)
    {
	if(*cp == ',')
            break;

	ipos ++;

	switch(*cp++)
	{
            case '(':
                if(!left)
                    bracepos = ipos-1;

                left++;

                if(left>3)
                    return ajFalse;

                depth = left - right;

                if(depth > 2)
                    return ajFalse;

                break;
            case ')':
                right++;

                if(right>3)
                    return ajFalse;

                if(right > left)
                    return ajFalse;

                break;
            default:
                break;
	}
    }

    if(!left)
        return ajFalse;		/* no parentheses */

    if(right != left)
        return ajFalse;	        /* unmatched '(' */

    
    ajStrAssignSubS(opnam, loc, 0, bracepos-1);
    ajStrAssignSubS(value, loc, bracepos+1, ipos-2);

    if(ipos < ajStrGetLen(loc))
	ajStrAssignSubS(rest, loc, ipos+1, -1);
    else
	ajStrAssignClear(rest);

    return ajTrue;
}




/* @funcstatic featEmblOperNone ***********************************************
**
** Process anything up to ':' as an entry ID (a feature in another entry)
** Anything up to ',' is this location
**
** @param [r] loc [const AjPStr] Feature location
** @param [w] entryid [AjPStr*] Entry ID if found, else empty
** @param [w] value [AjPStr*] Location with entryID removed
** @param [w] rest [AjPStr*] Remainder of location
** @return [AjBool] ajTrue if a match was found
**                  ajFalse means an error occurred
******************************************************************************/

static AjBool featEmblOperNone(const AjPStr loc, AjPStr* entryid,
			       AjPStr* value, AjPStr* rest)
{
    ajuint ipos=0;
    ajuint idpos = 0;
    const char* cp = ajStrGetPtr(loc);

    while(*cp)
    {
	if(*cp == ',')
            break;

	ipos ++;

	switch(*cp++)
	{
            case ':':
                idpos = ipos;
                break;
            default:
                break;
	}
    }

    if(idpos)
    {
	ajStrAssignSubS(entryid, loc, 0, idpos-2);
	ajStrAssignSubS(value, loc, idpos, ipos-1);
    }
    else
    {
        ajStrAssignClear(entryid);
	ajStrAssignSubS(value, loc, 0, ipos-1);
    }

    if(ipos < ajStrGetLen(loc))
	ajStrAssignSubS(rest, loc, ipos+1, -1);
    else
	ajStrAssignClear(rest);

   return ajTrue;
}




/* @funcstatic featEmblLoc ***********************************************
**
** Process simple feature location in the forms begin..end or begin^end
**
** @param [r] loc [const AjPStr] Feature location
** @param [w] begstr [AjPStr*] Start position
** @param [w] between [AjBool*] ajTrue for '^' as a position between 2 bases
** @param [w] simple [AjBool*] ajTrue for single base location
** @param [w] endstr [AjPStr*] End position
** @return [AjBool] ajTrue if a match was found
**                  ajFalse means an error occurred
******************************************************************************/

static AjBool featEmblLoc(const AjPStr loc, AjPStr* begstr, AjBool* between,
			  AjBool* simple, AjPStr* endstr)
{
    ajint ipos=0;
    ajint bracket=0;
    AjBool numeric=ajTrue;
    AjBool dot = ajFalse;
    AjBool end = ajFalse;
    ajint iend = 0;
    ajint ibeg = 0;
    const char* cp = ajStrGetPtr(loc);

    *between = ajFalse;
    *simple  = ajFalse;
    ajStrAssignClear(begstr);
    ajStrAssignClear(endstr);

    while(*cp)
    {
	ipos ++;

	switch(*cp)
	{
            case '.':
                if(bracket)
                    break;

                if(dot)
                {
                    if(end)
                        return ajFalse;

                    end = ajTrue;
                    ibeg = ipos - 2;
                    iend = ipos - 1;
                }
                else
                    dot = ajTrue;
                break;
            case '^':
                ibeg = ipos - 1;
                iend = ipos - 1;
                end = ajTrue;
                *between = ajTrue;
                break;
            case '(':
                bracket++;
                numeric = ajFalse;
                break;
            case ')':
                if(!bracket)
                    return ajFalse;
                bracket--;
                break;
            default:
                if(!isdigit((ajint) *cp))
                    numeric = ajFalse;

                break;
	}

	cp++;
    }

    if(dot && !end)
	return ajFalse;

    if(end)
    {
	ajStrAssignSubS(begstr, loc, 0, ibeg-1);
	ajStrAssignSubS(endstr, loc, iend+1, ipos-1);
    }
    else			/* simple location e.g. 43 */
    {
	*simple = ajTrue;
	ajStrAssignSubS(begstr, loc, 0, -1);

	if(numeric)
	    ajStrAssignSubS(endstr, loc, 0, -1);
	else
	    ajStrAssignClear(endstr);
    }

    return ajTrue;
}




/* @funcstatic featEmblLocNum *************************************************
**
** Process a single base position with a possible less than or greater than
** character in front to mark a position beyond the bounds of the sequence.
**
** @param [r] loc [const AjPStr] Feature location
** @param [w] bound [AjBool*] ajTrue if less than or greater than specified
** @param [w] num [ajint*] Base position
** @return [AjBool] ajTrue if a match was found
**                  ajFalse means an error occurred
******************************************************************************/

static AjBool featEmblLocNum(const AjPStr loc, AjBool* bound, ajint* num)
{
    const char* cp = ajStrGetPtr(loc);

    *bound = ajFalse;
    *num = 0;

    if((*cp == '<') || (*cp == '>'))
    {
	*bound = ajTrue;
	cp++;
    }

    while(*cp)
    {
	if(!isdigit((ajint)*cp))
	    return ajFalse;

	*num = 10* (*num) + (*cp - '0');
	cp++;
    }

   return ajTrue;
}




/* @funcstatic featEmblLocRange ***********************************************
**
** Process a range position (12.14) meaning any base from 12 to 14 as the
** start or end of a feature location.
**
** @param [r] loc [const AjPStr] Feature location
** @param [w] num1 [ajint*] First base position
** @param [w] num2 [ajint*] Last base position
** @return [AjBool] ajTrue if a match was found
**                  ajFalse means an error occurred
******************************************************************************/

static AjBool featEmblLocRange(const AjPStr loc, ajint* num1, ajint* num2)
{
    ajuint ipos=0;
    AjBool dot = ajFalse;
    const char* cp = ajStrGetPtr(loc);

    if(*cp != '(')
	return ajFalse;

    if(ajStrGetCharLast(loc) != ')')
	return ajFalse;

    *num1 = 0;
    *num2 = 0;

    while(*cp)
    {
	ipos ++;

	switch(*cp)
	{
            case '.':
                if(dot)
                    return ajFalse;

                dot = ajTrue;
                break;
            case '(':
                if(ipos != 1)
                    return ajFalse;

                break;
            case ')':
                if(ipos != ajStrGetLen(loc))
                    return ajFalse;

                break;
            default:
                if(!isdigit((ajint)*cp))
                    return ajFalse;

                if(dot)
                    *num2 = 10* (*num2) + (*cp - '0');
                else
                    *num1 = 10* (*num1) + (*cp - '0');

                break;
	}
	cp++;
    }

    if(!dot)
	return ajFalse;

    return ajTrue;
}




/* @funcstatic featEmblTvTagVal ***********************************************
**
** Tests for a feature tag value next in the input string 
**
** @param [u] tags [AjPStr*] Feature tag-values string, returned with
**                           current tag-value removed
** @param [w] name [AjPStr*] Feature tag name
** @param [w] value [AjPStr*] Feature tag value
** @return [AjBool] ajTrue if a match was found
**                  ajFalse means an error occurred
******************************************************************************/

static AjBool featEmblTvTagVal(AjPStr* tags, AjPStr* name, AjPStr* value)
{
    const char* cp;
    const char* cq;
    ajint i;
    AjPStr testtags = NULL;
    AjPStr testname = NULL;
    AjPStr testvalue = NULL;
    AjPStr tmptag = NULL;
    static ajint depth = 0;

    tmptag = ajStrNewS(*tags);
    cp = ajStrGetPtr(tmptag);
    ajStrAssignClear(value);

    while((*cp == ' '))
	cp++;

    if(*cp++ != '/')
    {
        ajStrDel(&tmptag);      
	return ajFalse;
    }

    cq = cp;
    i=0;

    while(isalpha((ajint)*cp) || (*cp == '_'))
    {
	i++;
	cp++;
    }

    ajStrAssignLenC(name, cq, i);

    while(*cp == ' ')
	cp++;

    if(!ajStrGetLen(*name))
    {
        ajStrDel(&tmptag);

	return ajFalse;
    }
    
    switch(*cp)
    {
        case '\0':			 /* /name is end of input */
            ajStrAssignClear(tags);
            ajStrAssignClear(value);
            ajStrDel(&tmptag);

            return ajTrue;
        case '/':			 /* /name then next tag, no value */
            ajStrAssignC(tags, cp);
            ajStrAssignClear(value);
            ajStrDel(&tmptag);

            return ajTrue;
        case '=':			 /* /name=value */
            break;
        default:			 /* anything else is bad */
            ajStrDel(&tmptag);

            return ajFalse;
    }

    cp++;

    if(*cp == '"')			/* /name="... */
    {
	cq = cp;
	cp++;
	i=0;

	while(*cp)
	{
	    i++;

	    if(*cp == '"')
	    {
		ajStrAppendLenC(value, cq, i);
		i = 0;
		cp++;

		if(!*cp || (*cp != '"')) /* all done */
		{
		    while(*cp == ' ')
			cp++;

		    if(*cp)
			ajStrAssignC(tags, cp);
		    else
			ajStrAssignClear(tags);

		    ajStrDel(&testtags);
		    ajStrDel(&testname);
		    ajStrDel(&testvalue);
                    ajStrDel(&tmptag);

		    return ajTrue;
		}
		else			/* "" but is it really internal */
		{
		    if(depth)
		    {
			ajStrDel(&testtags);
			ajStrDel(&testname);
			ajStrDel(&testvalue);
                        ajStrDel(&tmptag);

			return ajTrue;
		    }

		    depth++;
		    ajStrAssignC(&testtags, cp);

		    if(featEmblTvTagVal(&testtags, &testname, &testvalue))
		    {	       /* looks like an extra closing quote */
			depth--;
			ajStrDel(&testtags);
			ajStrDel(&testname);
			ajStrDel(&testvalue);
			cp++;

			while(*cp == ' ')
			    cp++;

			ajStrAssignC(tags, cp);
                        ajStrDel(&tmptag);

			return ajTrue;
		    }
		    else		/* really an internal " */
		    {
			depth--;
			cq = cp-1;
			i = 1;
		    }
		}
	    }
	    cp++;
	}
	ajStrAssignClear(tags);
    }
    else
    {
	cq = cp;
	i=0;

	while(*cp && (*cp != ' ') && (*cp != '/'))
	{
	    cp++;
	    i++;
	}

	ajStrAssignLenC(value, cq, i);
	ajStrAssignC(tags, cp);
        ajStrDel(&tmptag);

	return ajTrue;
    }

    ajStrDel(&testtags);
    ajStrDel(&testname);
    ajStrDel(&testvalue);
    ajStrDel(&tmptag);

    return ajFalse;

}




/* @funcstatic featEmblTvRest ***********************************************
**
** Process bad tag-value text up to next qualifier
**
** @param [u] tags [AjPStr*] Feature tag-values string, returned with
**                          skipped text removed
** @param [w] skip [AjPStr*] Skipped text
** @return [AjBool] ajTrue if a match was found
**                  ajFalse means an error occurred
******************************************************************************/

static AjBool featEmblTvRest(AjPStr* tags, AjPStr* skip)
{
    AjPStr testtags = NULL;
    AjPStr testname = NULL;
    AjPStr testvalue = NULL;
    AjBool ok = ajFalse;
    const char* cp = ajStrGetPtr(*tags);

    ajStrAssignClear(skip);

    while(*cp)
    {
	if(*cp == '/')
	{
	    ajStrAssignC(&testtags,cp);
	    ok = featEmblTvTagVal(&testtags, &testname, &testvalue);
	    ajStrDel(&testtags);
	    ajStrDel(&testname);
	    ajStrDel(&testvalue);

	    if(ok)
	    {
		ajStrAssignC(tags, cp);

		return ajTrue;
	    }

	    ajStrAppendK(skip, *cp);
	}
	else
	{
	    ajStrAppendK(skip, *cp);
	}
	cp++;
    }

    ajStrAssignClear(tags);

    return ajTrue;
}




/* @funcstatic featTagName ****************************************************
**
** Process anything up to ':' as an entry ID (a feature in another entry)
** Anything up to ',' is this location
**
** @param [r] line [const AjPStr] Tags file line
** @param [w] name [AjPStr*] Tag name
** @param [w] type [AjPStr*] Tag type
** @param [w] rest [AjPStr*] Remainder of line
** @return [AjBool] ajTrue if a match was found
**                  ajFalse means an error occurred
******************************************************************************/

static AjBool featTagName(const AjPStr line, AjPStr* name, AjPStr* type,
			  AjPStr* rest)
{
    const char* cp = ajStrGetPtr(line);

    ajStrAssignClear(name);
    ajStrAssignClear(type);
    ajStrAssignClear(rest);

    while(isspace((ajint)*cp))
	cp++;

    while(*cp && !isspace((ajint)*cp))
	ajStrAppendK(name, *cp++);

    while(isspace((ajint)*cp))
	cp++;

    if(!*cp)
    {
	ajStrAssignClear(name);

	return ajFalse;
    }

    while(isalpha((ajint)*cp))
	ajStrAppendK(type, *cp++);

    if(!ajStrMatchC(*type, "LIMITED") || ajStrMatchC(*type, "QLIMITED"))
	return ajTrue;

    while(isspace((ajint)*cp))
	cp++;

    while(*cp)
	ajStrAppendK(rest, *cp++);

    return ajTrue;
}




/* @funcstatic featFeatType ***************************************************
**
** Process Efeatures file line
**
** Lines starting with text are new types
**
** Lines with '=' are aliases that update a previous defined type name
** and can have only + and - tag lines.
**
** Lines starting with / or m/ are tags for the current type
**
** Lines starting with +/ or -/ add and remove tags for the current type
**
** @param [r] line [const AjPStr] Tags file line
** @param [w] type [AjPStr*] Feature type
** @param [w] intids [AjPStr*] Internal feature keys, preferably based on the
**                             Sequence Ontology Feature Annotation ID
** @param [w] tag [AjPStr*] Tag name
** @param [w] req [AjPStr*] Required (mandatory) code
**                          M = mandatory, m = alternate mandatory,
**                          + = add to existing tags,
**                          - = remove from existing tags
** @return [AjBool] ajTrue if a match was found
**                  ajFalse means an error occurred
******************************************************************************/

static AjBool featFeatType(const AjPStr line, AjPStr* type,
			   AjPStr* intids, AjPStr* tag, AjPStr* req)
{
    const char* cp = ajStrGetPtr(line);
    const char* cq;
    ajint i;
    AjBool istag = ajFalse;

    ajStrAssignClear(type);
    ajStrAssignClear(intids);
    ajStrAssignClear(tag);
    ajStrAssignClear(req);

    while(isspace((ajint)*cp))
	cp++;

    if(!*cp)
        return ajFalse;

    if(*cp == '/')
	istag = ajTrue;
    else if(*cp == 'm' || *cp == 'M' || *cp == '-')
    {
	if(*(cp+1) == '/')
	{
	    istag = ajTrue;
	    ajStrAppendK(req, *cp++);
	}
    }
    else if(*cp == '+')
    {
	if(*(cp+1) == 'm' || *(cp+1) == 'M')
	{
	    if(*(cp+2) == '/')
	    {
		istag = ajTrue;
		ajStrAppendK(req, *cp++);
		ajStrAppendK(req, *cp++);
	    }
	}
	else
	{
	    if(*(cp+1) == '/')
	    {
		istag = ajTrue;
		ajStrAppendK(req, *cp++);
	    }
	}
    }

    if(istag)
    {
	/* /tag name */
	cp++;
	cq = cp;
	i = 0;

	while(*cp && !isspace((ajint)*cp++))
	{
	    i++;
	}

	ajStrAssignLenC(tag, cq, i);

	if(!ajStrGetLen(*tag))
            return ajFalse;
    }
    else
    {
	/* type internal id */
	cq = cp;
	i = 0;

	while(*cp && !isspace((ajint)*cp++))
	{
	    i++;
	}

	ajStrAssignLenC(type, cq, i);

	if(!ajStrGetLen(*type))
            return ajFalse;

	while(*cp && isspace((ajint)*cp))
	    cp++;

	if(!*cp)
	    return ajTrue;

	if(*cp == '=')
	    ajStrAppendK(req, *cp++);

	while(*cp && isspace((ajint)*cp))
	    cp++;
	    
	cq = cp;
	i = 0;

	while(*cp && !isspace((ajint)*cp++))
	    i++;

	ajStrAppendLenC(intids, cq, i);

	while(*cp)
	{
	    while(*cp && isspace((ajint)*cp))
		cp++;

	    if(!*cp)
		return ajTrue;

	    cq = cp;
	    i = 0;

	    while(*cp && !isspace((ajint)*cp++))
		i++;

	    ajStrAppendK(intids, ' ');
	    ajStrAppendLenC(intids, cq, i);
	}
    }

    return ajTrue;
}




/* @funcstatic featGffFromLine ************************************************
**
** Converts an input GFF format line into a feature
**
** @param [u] thys [AjPFeattable] Feature table
** @param [r] line [const AjPStr] Input line
** @param [r] version [float] GFF version (1.0 for old format behaviour)
** @return [AjPFeature] New feature
** @@
******************************************************************************/

static AjPFeature featGffFromLine(AjPFeattable thys, const AjPStr line,
				  float version)
{
    AjPFeature gf           = NULL;
    AjPStr token   = NULL;
    ajint Start  = 0;
    ajint End    = 0;
    float fscore = 0.0;
    ajint itemp;

    char   strand;
    ajint   frame;

    if(!ajStrGetLen(line))
	return NULL ;
    
    ajStrTokenAssignC(&featGffSplit, line, "\t") ;
    
    if(!ajStrTokenNextParse(&featGffSplit, &token))	/* seqname */
        goto Error;

    if(!ajStrTokenNextParse(&featGffSplit, &featSource)) /* source  */
        goto Error;

    if(!ajStrTokenNextParse(&featGffSplit, &featFeature)) /* feature */
        goto Error;

    if(!ajStrTokenNextParse(&featGffSplit, &token)) /* start   */
        goto Error;
    if(!ajStrToInt(token, &Start))
	Start = 0;

    if(!ajStrTokenNextParse(&featGffSplit, &token)) /* end     */
        goto Error;

    if(!ajStrToInt(token,   &End))
	End   = 0;

    if(!ajStrTokenNextParse(&featGffSplit, &token)) /* score   */
        goto Error;

    if(!ajStrToFloat(token,   &fscore))
	fscore = 0.0;

    if(!ajStrTokenNextParse(&featGffSplit, &token)) /* strand  */
        goto Error;

    if(!ajStrCmpC(token,"+"))
	strand = '+';
    else if(!ajStrCmpC(token,"-"))
	strand = '-';
    else
	strand = '\0';		/* change to \0 later */

    if(!ajStrTokenNextParse(&featGffSplit, &token)) /* frame   */
        goto Error;

    if(!ajStrCmpC(token,"0"))
	frame = 1;
    else if(!ajStrCmpC(token,"1"))
	frame = 2;
    else if(!ajStrCmpC(token,"2"))
	frame = 3;
    else
	frame = 0;

    if(strand == '-') 
    {
        if(Start < End) 
        {
            itemp = Start;
            Start = End;
            End = itemp;
        }
    }
    
	/* feature object construction
	   and group tag */
    if(ajStrMatchC(thys->Type, "P"))
        gf = featFeatNewProt(thys,
                             featSource,
                             featFeature,
                             Start, End,
                             fscore,
                             0);
    else
        gf = featFeatNew(thys,
                         featSource,
                         featFeature,
                         Start, End,
                         fscore,
                         strand,
                         frame,
                         0,0,0, NULL, NULL, 0);
    if(ajStrTokenRestParse(&featGffSplit, &featGroup))
	featGffProcessTagval(gf, thys, featGroup, version) ;

    ajStrDel(&token);

    return gf;
    
 Error:
    
    ajStrTokenDel(&featGffSplit);
    
    ajStrDelStatic(&featSource);
    ajStrDelStatic(&featFeature);
    ajStrDel(&token);
    
    return gf;
}




/* @funcstatic featGff3FromLine ************************************************
**
** Converts an input GFF3 format line into a feature
**
** @param [u] thys [AjPFeattable] Feature table
** @param [r] line [const AjPStr] Input line
** @return [AjPFeature] New feature
** @@
******************************************************************************/

static AjPFeature featGff3FromLine(AjPFeattable thys, const AjPStr line)
{
    AjPFeature gf           = NULL;
    AjPStr token   = NULL;
    ajint Start  = 0;
    ajint End    = 0;
    float fscore = 0.0;
    
    char   strand;
    ajint   frame;

    if(!ajStrGetLen(line))
	return NULL ;
    
    ajStrTokenAssignC(&featGffSplit, line, "\t") ;
    
    if(!ajStrTokenNextParse(&featGffSplit, &token))	/* seqname */
        goto Error;

    if(!ajStrTokenNextParse(&featGffSplit, &featSource)) /* source  */
        goto Error;

    if(!ajStrTokenNextParse(&featGffSplit, &featFeature)) /* feature */
        goto Error;

    if(!ajStrTokenNextParse(&featGffSplit, &token)) /* start   */
        goto Error;

    if(!ajStrToInt(token, &Start))
	Start = 0;

    if(!ajStrTokenNextParse(&featGffSplit, &token)) /* end     */
        goto Error;

    if(!ajStrToInt(token,   &End))
	End   = 0;

    if(!ajStrTokenNextParse(&featGffSplit, &token)) /* score   */
        goto Error;

    if(!ajStrToFloat(token,   &fscore))
	fscore = 0.0;

    if(!ajStrTokenNextParse(&featGffSplit, &token)) /* strand  */
        goto Error;

    if(!ajStrCmpC(token,"+"))
	strand = '+';
    else if(!ajStrCmpC(token,"-"))
	strand = '-';
    else
	strand = '\0';		/* change to \0 later */
	
    if(!ajStrTokenNextParse(&featGffSplit, &token)) /* frame   */
        goto Error;

    if(!ajStrCmpC(token,"0"))
	frame = 1;
    else if(!ajStrCmpC(token,"1"))
	frame = 2;
    else if(!ajStrCmpC(token,"2"))
	frame = 3;
    else
	frame = 0;

	/* feature object construction
	   and group tag */

    if(ajStrMatchC(thys->Type, "P"))
        gf = featFeatNewProt(thys,
                             featSource,
                             featFeature,
                             Start, End,
                             fscore,
                            0);
    else
        gf = featFeatNew(thys,
                         featSource,
                         featFeature,
                         Start, End,
                         fscore,
                         strand,
                         frame,
                         0,0,0, NULL, NULL, 0);

    if(ajStrTokenRestParse(&featGffSplit, &featGroup))
	featGff3ProcessTagval(gf, thys, featGroup) ;

    ajStrDel(&token);

    return gf;
    
 Error:
    
    ajStrTokenDel(&featGffSplit);
    
    ajStrDelStatic(&featSource);
    ajStrDelStatic(&featFeature);
    ajStrDel(&token);
    
    return gf;
}




/* @funcstatic featReadGff ****************************************************
**
** Read input file in GFF format
**
** @param [u] thys [AjPFeattable] Feature table
** @param [u] file [AjPFilebuff] Input buffered file
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool featReadGff(AjPFeattable thys, AjPFilebuff file)
{
    AjPStr line  = NULL;
    AjPStr verstr       = NULL;
    AjPStr start        = NULL;
    AjPStr end          = NULL;
    AjPStr type  = NULL;

    AjBool found  = ajFalse;
    float version = 2.0;
    
    /* ajDebug("featReadGff..........\n"); */
    
    while(ajBuffreadLine(file, &line))
    {	
	ajStrTrimWhite(&line);
	
	/* Header information */
	
	if(ajRegExec(GffRegexblankline, line))
	    version = 2.0;
	else if(ajRegExec(GffRegexversion,line))
	{
	    verstr = ajStrNew();
	    ajRegSubI(GffRegexversion, 1, &verstr);
	    ajStrToFloat(verstr, &version);
	    ajStrDel(&verstr);
	}
	   /*
	       else if(ajRegExec(GffRegexdate,line))
	    {
	         AjPStr year  = NULL ;
	         AjPStr month = NULL ;
	         AjPStr day   = NULL ;
	         ajint nYear, nMonth, nDay ;
	         ajRegSubI(GffRegexdate, 1, &year);
	         ajRegSubI(GffRegexdate, 2, &month);
	         ajRegSubI(GffRegexdate, 3, &day);
	         ajStrToInt(year,  &nYear);
	         ajStrToInt(month, &nMonth);
	         ajStrToInt(day,   &nDay);
	         ajStrDel(&year);
	         ajStrDel(&month);
	         ajStrDel(&day);
	       }
	   */
	else if(ajRegExec(GffRegexregion,line))
	{
	    start = ajStrNew();
	    end   = ajStrNew();
	    ajRegSubI(GffRegexregion, 2, &start);
	    ajRegSubI(GffRegexregion, 3, &end);
	    ajStrToUint(start, &(thys->Start));
	    ajStrToUint(end,   &(thys->End));
	    ajStrDel(&start);
	    ajStrDel(&end);
	}
	else if(ajRegExec(GffRegextype,line))
	{
	    ajRegSubI(GffRegextype, 1, &type);

	    if(ajStrMatchCaseC(type, "Protein"))
		ajFeattableSetProt(thys);
	    else
		ajFeattableSetNuc(thys);

	    ajStrDel(&type);
	    ajRegSubI(GffRegextype, 3, &thys->Seqid);
	}
	else if(ajRegExec(GffRegexcomment,line))
	    version = 2.0;      /* ignore for now... could store them in
				 ajFeattable for future reference though?...*/
	/* the real feature stuff */
	else		       /* must be a real feature at last !! */
	    if(featGffFromLine(thys, line, version)) /* does ajFeattableAdd */
		found = ajTrue ;

    }
    ajStrDel(&line);

    return found;
}




/* @funcstatic featReadGff3 ***************************************************
**
** Read input file in GFF3 format
**
** @param [u] thys [AjPFeattable] Feature table
** @param [u] file [AjPFilebuff] Input buffered file
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool featReadGff3(AjPFeattable thys, AjPFilebuff file)
{
    AjPStr line  = NULL;
    AjPStr verstr       = NULL;
    AjPStr start        = NULL;
    AjPStr end          = NULL;

    AjBool found  = ajFalse;
    float version = 3.0;
    
    /* ajDebug("featReadGff3..........\n"); */
    
    while(ajBuffreadLine(file, &line))
    {	
	ajStrTrimWhite(&line);
	
	/* Header information */
	
	if(ajRegExec(Gff3Regexblankline, line))
	    version = 3.0;
	else if(ajRegExec(Gff3Regexversion,line))
	{
	    verstr = ajStrNew();
	    ajRegSubI(Gff3Regexversion, 1, &verstr);
	    ajStrToFloat(verstr, &version);
	    ajStrDel(&verstr);

            if(version < 3.0)
            {
                ajStrDel(&line);

                return ajFalse;
            }
	}
	else if(ajRegExec(Gff3Regexregion,line))
	{
	    start = ajStrNew();
	    end   = ajStrNew();
	    ajRegSubI(Gff3Regexregion, 1, &thys->Seqid);
	    ajRegSubI(Gff3Regexregion, 2, &start);
	    ajRegSubI(Gff3Regexregion, 3, &end);
	    ajStrToUint(start, &(thys->Start));
	    ajStrToUint(end,   &(thys->End));
	    ajStrDel(&start);
	    ajStrDel(&end);
	}
	/* the real feature stuff */
	else		       /* must be a real feature at last !! */
	    if(featGff3FromLine(thys, line)) /* does ajFeattableAdd */
		found = ajTrue ;

    }
    ajStrDel(&line);

    return found;
}




/* @funcstatic featRefseqpFromLine ********************************************
**
** Converts an input RefSeq protein format line into a feature.
** Starts a new feature by processing any existing feature data.
** Creates or appends the type, location and tag-value pairs.
** With a NULL as the input line, simply processes the type, location
** and tag-values.
**
** @param [u] thys     [AjPFeattable] Feature table
** @param [r] origline [const AjPStr] Input line (NULL to process last
**                                    feature at end of input)
** @param [w] savefeat [AjPStr*] Stored feature type
** @param [w] saveloc  [AjPStr*] Continued location
** @param [w] saveline [AjPStr*] Continued tag-value pairs
** @return [AjPFeature] New feature
** @@
******************************************************************************/

static AjPFeature featRefseqpFromLine(AjPFeattable thys,
                                      const AjPStr origline,
                                      AjPStr* savefeat,
                                      AjPStr* saveloc,
                                      AjPStr* saveline)
{
    static AjPFeature gf = NULL;      /* so tag-values can be added LATER */
    AjPStr temp   = NULL;
    AjBool newft         = ajFalse;
    AjBool doft         = ajFalse;

    if(!featSourceRefseqp)
	featSourceRefseqp = ajStrNewC("REFSEQP");
    
    /* ajDebug("featRefseqpFromLine '%S'\n", origline); */
    
    if(origline)
    {
        /* As BufferFile can't be edited */
	ajStrAssignS(&featProcessLine,origline);
        /* chop first 5 characters */
	ajStrCutStart(&featProcessLine, 5);

	/* look for the feature key */
        if(ajStrGetCharFirst(featProcessLine) != ' ')
	{
	    newft = ajTrue;

	    if(ajStrGetLen(*saveloc))
		doft = ajTrue;
	}
    }
    else
    {
	ajStrAssignClear(&featProcessLine);
	newft = ajFalse;		/* no new data, just process */

	if(ajStrGetLen(*saveloc))
	    doft = ajTrue;
    }

    /*
       ajDebug("+ newft: %B doft: %B\n+ line '%S'\n",
		newft, doft, line);
    */
    
    if(doft) 		/* process the last feature */
    {
	/*
        **   ajDebug("++ saveloc '%S'\n+ saveline '%S'\n",
	**           *saveloc, *saveline);
	*/

	gf = featRefseqpProcess(thys, *savefeat, featSourceRefseqp,
                                saveloc, saveline);

	ajStrDelStatic(saveloc);
	ajStrDelStatic(saveline);
    }
    
    if(!origline)		/* we are only cleaning up */
	return gf;
    
    ajStrRemoveWhiteExcess(&featProcessLine);

    if(newft) 		/* if new feature initialise for it */
    {
	ajStrTokenAssignC(&featEmblSplit, featProcessLine, " ");
	ajStrTokenNextParse(&featEmblSplit, savefeat);

	if(ajStrTokenNextParseC(&featEmblSplit, " /", saveloc))
	    ajStrTokenRestParse(&featEmblSplit, saveline);
	else
	    ajStrAssignResC(saveline, 512, "");	/* location only */

	ajStrTokenDel(&featEmblSplit);

	return gf;
    }
    else if(!ajStrGetLen(*saveline))  /* no tag-values yet, more location? */
    {
	if(ajStrGetCharFirst(featProcessLine) != '/')
	{
	    ajStrTokenAssignC(&featEmblSplit, featProcessLine, " ");
	    ajStrTokenNextParse(&featEmblSplit, &temp);

	    if(ajStrGetLen(temp))
		ajStrAppendS(saveloc, temp);

	    ajStrTokenRestParse(&featEmblSplit, &temp);

	    if(ajStrGetLen(temp))
		ajStrAppendS(saveline, temp);

	    ajStrDel(&temp);
	    ajStrTokenDel(&featEmblSplit);

	    return gf;
	}
    }

    /* tag-values continued */
    ajStrAppendK(saveline, ' ');
    ajStrAppendS(saveline, featProcessLine);
    
    return gf;
}




/* @funcstatic featRefseqpProcess *********************************************
**
** Processes one feature location and qualifier tags for RefSeq protein
**
** @param [u] thys [AjPFeattable] Feature table
** @param [r] feature [const AjPStr] Feature type key
** @param [r] source [const AjPStr] Feature table source
** @param [w] loc [AjPStr*] Feature location
** @param [w] tags [AjPStr*] Feature qualifier tags string
** @return [AjPFeature] Feature as inserted into the feature table
** @@
******************************************************************************/

static AjPFeature featRefseqpProcess(AjPFeattable thys, const AjPStr feature,
                                     const AjPStr source,
                                     AjPStr* loc, AjPStr* tags)
{    
    AjPFeature ret  = NULL;
    AjPFeature gf   = NULL;
    AjPStr tag      = NULL;
    AjPStr val      = NULL;
    AjPStr opnam    = NULL;
    AjPStr opval    = NULL;
    AjPStr prestr   = NULL;
    AjBool Fwd      = ajTrue;
    AjBool LocFwd   = ajTrue;
    AjPStr begstr   = NULL;
    AjPStr delstr   = NULL;
    AjPStr endstr   = NULL;
    AjPStr locstr   = NULL;
    AjPStr rest     = NULL;
    AjBool Simple   = ajFalse;	/* Simple - single position (see also label) */
    AjBool BegBound = ajFalse;
    AjBool EndBound = ajFalse;
    ajint BegNum = 0;
    ajint EndNum = 0;
    ajint Beg2   = 0;
    ajint End2   = 0;
    AjBool Between = ajFalse;
    AjBool Join    = ajFalse;
    AjBool Order   = ajFalse;
    ajint Flags;
    ajint ExonFlags;
    AjBool Mother   = ajTrue;
    ajint Frame     = 0;
    float Score     = 0.0;
    AjBool HasOper  = ajFalse;
    AjBool RemoteId = ajFalse;
    AjBool IsLabel  = ajFalse;	/* uses obsolete label  */
    ajint Exon      = 0;
    ajint ipos;
    ajint itags = 0;
    const AjPStr tmpft = NULL;
    
    ajStrRemoveWhite(loc);	/* no white space needed */
    ajStrRemoveWhiteExcess(tags);		/* single spaces only */

    /*ajDebug("Clean location '%S'\n", *loc);*/
    /*ajDebug("Clean tags '%S'\n", *tags);*/
    
    ajStrAssignS(&opval, *loc);
    ipos = ajStrFindAnyK(opval, ',');	/* multiple locations */
    if(ipos >= 0)
    {
	/* ajDebug("Multiple locations, test operator(s)\n"); */
	while(ajStrGetLen(opval) &&
	      featEmblOperOut(opval, &opnam, &featTmpStr))
	{
	    if(!ajStrHasParentheses(featTmpStr))
		break;

	    /* ajDebug("OperOut %S( '%S' )\n", opnam, featTmpStr); */
	    if(ajStrMatchCaseC(opnam, "complement"))
		Fwd = !Fwd;

	    else if(ajStrMatchCaseC(opnam, "one_of"))
		Order = ajTrue;

	    else if(ajStrMatchCaseC(opnam, "join"))
		Join = ajTrue;

	    else if(ajStrMatchCaseC(opnam, "order"))
		Order = ajTrue;

	    else if(ajStrMatchCaseC(opnam, "group"))
		Order = ajTrue;

	    else
		featWarn("%S: unrecognised operator '%S()' in '%S'",
		       thys->Seqid, opnam, opval);

	    ajStrAssignS(&opval, featTmpStr);
	}
    }
    
    while(ajStrGetLen(opval))
    {
	LocFwd   = Fwd;
	BegBound = ajFalse;
	EndBound = ajFalse;
	Simple   = ajFalse;
	Between  = ajFalse;
	BegNum   = EndNum = Beg2 = End2 = 0;
	HasOper  = ajFalse;
	RemoteId = ajFalse;
	IsLabel  = ajFalse;

	ajStrDelStatic(&featId);
	ajStrDelStatic(&featLabel);

	/* check for complement() */
	/* set locstr as the whole (or rest) of the location */

	if(featEmblOperIn(opval, &opnam, &locstr, &featTmpStr))
	{
	    /* ajDebug("OperIn %S( '%S' )\n", opnam, locstr); */
	    if(ajStrMatchCaseC(opnam, "complement"))
		LocFwd = !LocFwd;

	    ajStrAssignS(&opval, featTmpStr);
	    /* ajDebug("rest: '%S'\n", opval); */
	    HasOper = ajTrue;
	}
	else
	{
	    ajStrAssignS(&locstr, opval);
	    /* ajDebug("OperIn simple '%S'\n", locstr); */
	}

	if(featEmblOperNone(locstr,
			    &featId, &featTmpStr, &rest))  /* one exon */
	{
	    /* ajDebug("OperNone '%S' \n", featTmpStr); */
	    if(ajStrGetLen(featId))
	    {
		/* ajDebug("External entryid '%S'\n", featId); */
		RemoteId = ajTrue;
	    }
	    if(!featEmblLoc(featTmpStr, &begstr, &Between, &Simple, &endstr))
	    {
		ajStrAssignS(&begstr, featTmpStr);
		ajStrAssignS(&endstr, begstr);
		Simple = ajTrue;
		ajDebug("Bad feature numeric location '%S' in '%S' - "
		   "test later for label",
		   begstr, locstr);
	    }

	    ajStrAssignS(&featTmpStr, rest);

	    if(!HasOper)
		ajStrAssignS(&opval, featTmpStr);
	    
	    if(featEmblLocNum(begstr, &BegBound, &BegNum))
	    {
		Beg2 = 0;
		/* ajDebug("Begin '%S' %d  Bound: %B\n",
		   begstr, BegNum, BegBound); */
	    }
	    else if(featEmblLocRange(begstr, &BegNum, &Beg2))
	    {
		BegBound = ajFalse;
		/* ajDebug("Begin range (%d . %d)\n", BegNum, Beg2); */
	    }
	    else
	    {
		/* ajDebug("Begin is a label '%S'\n", begstr); */
		IsLabel = ajTrue;
		Simple  = ajTrue;
		ajStrAssignS(&featLabel, begstr);
		featWarn("%S: Simple feature location '%S' in '%S'",
		       thys->Seqid, begstr, locstr);
	    }
	    
	    if(featEmblLocNum(endstr, &EndBound, &EndNum))
	    {
		End2 = 0;
		/* ajDebug("  End '%S' %d  Bound: %B\n",
		   endstr, EndNum, EndBound); */
	    }
	    else if(featEmblLocRange(endstr, &End2, &EndNum))
	    {
		EndBound = ajFalse;
		/* ajDebug("  End range (%d . %d)\n", End2, EndNum); */
	    }
	    else
	    {
		IsLabel = ajTrue;
		Simple  = ajTrue;
		ajStrAssignS(&featLabel, endstr);
		/* ajDebug("  End is a label '%S'\n", endstr); */
		ajErr("%S: Simple feature end '%S' in '%S'",
		      thys->Seqid, begstr, locstr);
	    }
	}
	else
	{
	    ajErr("Unable to parse location:\n'%S'", opval);
	}
	/* location has been read in, now store it */
	
	Flags = ExonFlags = 0;
	
	if(Simple)
	    Flags |= FEATFLAG_POINT;
	if(Between)
	    Flags |= FEATFLAG_BETWEEN_SEQ;
	if(End2)
	    Flags |= FEATFLAG_END_TWO;
	if(Beg2)
	    Flags |= FEATFLAG_START_TWO;
	if(BegBound)
	    Flags |= FEATFLAG_START_BEFORE_SEQ;
	if(EndBound)
	    Flags |= FEATFLAG_END_AFTER_SEQ;
	if(RemoteId)
	    Flags |= FEATFLAG_REMOTEID;
	if(IsLabel)
	    Flags |= FEATFLAG_LABEL;
	if(IsLabel)
	    featWarn("%S: Feature location with label '%S'",
		   thys->Seqid, locstr);
	if(Join || Order)
	    Flags |= FEATFLAG_MULTIPLE;
	if(Order)
	    Flags |= FEATFLAG_ORDER;
	
        ExonFlags = Flags;
        
	if(Mother)
	{
	    ExonFlags |= FEATFLAG_CHILD;
	    if(!Fwd)
		Flags |= FEATFLAG_COMPLEMENT_MAIN;
	}
	else
	    Flags |= FEATFLAG_CHILD;

	/* ajDebug("Calling featFeatNew, Flags: %x\n", Flags); */
	tmpft = featTableTypeInternal(feature, FeatTypeTableRefseqp);

        if(!Mother)
            Exon++;
	gf = featFeatNewProt(thys,
                             source,	/* source sequence */
                             tmpft,
                             BegNum, EndNum,
                             Score,
                             Flags);
	if(Mother)
        {
	    ret = gf;
            Mother = ajFalse;
            Exon = 1;
        }
    }
    
    while(ajStrGetLen(*tags))
    {
	itags++;

	if(featEmblTvTagVal(tags, &tag, &val))
	{
	    ajStrQuoteStrip(&val);

	    if(!ajFeatTagAdd(ret, tag, val))
		featWarn("%S: Bad value '%S' for tag '/%S'",
		       thys->Seqid, val, tag) ;

            if(ajStrMatchC(tag, "codon_start"))
            {
                ajStrToInt(val, &Frame);
                gf->Frame = Frame;
            }
	}
	else if(featEmblTvRest(tags, &featTmpStr))
	{
	    /* anything non-whitespace up to '/' is bad */
	    featWarn("Bad feature syntax %S: skipping '%S'",
		   thys->Seqid, featTmpStr);
	}
	else
	{
	    featWarn("Bad feature syntax %S: giving up at '%S'",
		   thys->Seqid, *tags);
	    ajStrAssignClear(tags);
	}
   
    }

    ajDebug("featRefseqpProcess found %d feature tags\n", itags);

    ajStrDelStatic(&featTmpStr);
    ajStrDel(&prestr);
    ajStrDel(&val);
    ajStrDel(&tag);
    ajStrDel(&begstr);
    ajStrDel(&delstr);
    ajStrDel(&opnam);
    ajStrDel(&opval);
    ajStrDel(&locstr);
    ajStrDel(&endstr);
    ajStrDel(&rest);
    
    return ret;
}




/* @func ajFeattableWriteGff2 **************************************************
**
** Write feature table in GFF 2.0 format
**
** @param [u] ftout [AjPFeattabOut] Feature table output object
** @param [r] Feattab [const AjPFeattable] feature table
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeattableWriteGff2(AjPFeattabOut ftout, const AjPFeattable Feattab)
{
    AjIList    iter = NULL;
    AjPFeature gf   = NULL;
    AjPFile file = ftout->Handle;

    /* Check arguments */
    /* ajDebug("ajFeattableWriteGff2 Checking arguments\n"); */
    if(!file)
	return ajFalse;
    
    /* Print header first */
    ajFmtPrintF(file, "##gff-version 2.0\n") ;
    
    ajFmtPrintF(file, "##date %D\n", ajTimeRefTodayFmt("GFF")) ;
    
    if(ajStrMatchC(Feattab->Type, "N"))
	ajFmtPrintF(file, "##Type %s %S\n", "DNA",
		    Feattab->Seqid);
    else if(ajStrMatchC(Feattab->Type, "P"))
	ajFmtPrintF(file, "##Type %s %S\n", "Protein",
		    Feattab->Seqid);
    else
	ajFmtPrintF(file, "##Type unknown <%S> %S\n",
		    Feattab->Type, Feattab->Seqid);
    

  /* For all features... relatively simple because internal structures
     are deliberately styled on GFF */

    if(Feattab->Features)
    {
	iter = ajListIterNewread(Feattab->Features);

	while(!ajListIterDone(iter))
	{
	    gf = ajListIterGet(iter);
	    featDumpGff2(gf, Feattab, file);
	}

	ajListIterDel(&iter);
    }

    return ajTrue;
}




/* @func ajFeattableWriteGff3 *************************************************
**
** Write feature table in GFF format
**
** @param [u] ftout [AjPFeattabOut] Feature table output object
** @param [r] Feattab [const AjPFeattable] feature table
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeattableWriteGff3(AjPFeattabOut ftout, const AjPFeattable Feattab)
{
    AjIList    iter = NULL;
    AjPFeature gf   = NULL;
    AjPFile file = ftout->Handle;

    /* Check arguments */
    /* ajDebug("ajFeattableWriteGff3 Checking arguments\n"); */
    if(!file)
	return ajFalse;
    
    /* Print GFF3-specific header first with ## tags */

    ajFmtPrintF(file, "##gff-version 3\n") ;
    
    ajFmtPrintF(file, "##sequence-region %S %u %u\n",
		Feattab->Seqid, ajFeattableGetBegin(Feattab),
		ajFeattableGetEnd(Feattab));

    /* EMBOSS specific header with #! tags */

    ajFmtPrintF(file, "#!Date %D\n", ajTimeRefTodayFmt("GFF"));

    if(ajStrMatchC(Feattab->Type, "P"))
	ajFmtPrintF(file, "#!Type Protein\n");
    else
	ajFmtPrintF(file, "#!Type DNA\n");

    ajFmtPrintF(file, "#!Source-version EMBOSS %S\n", ajNamValueVersion());


  /* For all features... relatively simple because internal structures
     are deliberately styled on GFF */

    if(Feattab->Features)
    {
	iter = ajListIterNewread(Feattab->Features);

	while(!ajListIterDone(iter))
	{
	    gf = ajListIterGet(iter);
	    featDumpGff3(gf, Feattab, file);
	}

	ajListIterDel(&iter);
    }

    return ajTrue;
}




/* @funcstatic featRegInitEmbl ************************************************
**
** Initialise regular expressions and data structures for
** EMBL/GenBank/DDBJ format
**
** @return [AjBool] ajTrue if successful
** @@
******************************************************************************/

static AjBool featRegInitEmbl(void)
{
    if(FeatInitEmbl)
	return ajTrue;

    featInit();

    featVocabInitEmbl();

    /*ajDebug("Tables embl Type: %x Tags: %x\n",
	    FeatTypeTableEmbl, FeatTagsTableEmbl);*/
 
    FeatInitEmbl = ajTrue;

    return ajTrue;
}




/* @funcstatic featRegInitRefseqp *********************************************
**
** Initialise regular expressions and data structures for
** RefSeq protein format
**
** @return [AjBool] ajTrue if successful
** @@
******************************************************************************/

static AjBool featRegInitRefseqp(void)
{
    if(FeatInitRefseqp)
	return ajTrue;

    featInit();

    featVocabInitRefseqp();

    /*ajDebug("Tables refseqp Type: %x Tags: %x\n",
	    FeatTypeTableRefseqp, FeatTagsTableRefseqp);*/
 
    FeatInitRefseqp = ajTrue;

    return ajTrue;
}




/* @funcstatic featRegInitSwiss ***********************************************
**
** Initialise regular expressions and data structures for
** SwissProt format
**
** @return [AjBool] ajTrue if successful
** @@
******************************************************************************/

static AjBool featRegInitSwiss(void)
{
    if(FeatInitSwiss)
	return ajTrue;

    featInit();

    featVocabInitSwiss();

    /*ajDebug("Tables swiss Type: %x Tags: %x\n",
	    FeatTypeTableSwiss, FeatTagsTableSwiss);*/

    /*ajDebug("featRegInitSwiss Compiling regexps\n");*/
    if(!SwRegexNew)
	SwRegexNew = ajRegCompC("^FT   (([^ ]+) +([?<]?[0-9]+|[?]) +"
				"([?>]?[0-9]+|[?]) *)(.*)$");
    if(!SwRegexNext)
	SwRegexNext = ajRegCompC("^FT    +(.*)$");

    if(!SwRegexComment)
	SwRegexComment = ajRegCompC("^(.*)[(]([^)]+)[)]$") ;

    if(!SwRegexFtid)
	SwRegexFtid = ajRegCompC("^(.*)/FTId=([^ .]+)$") ;

    FeatInitSwiss = ajTrue;

    return ajTrue;
}




/* @funcstatic featRegInitPir *************************************************
**
** Initialise regular expressions and data structures for ajFeat in
** PIR format
**
** @return [AjBool] ajTrue if successful
** @@
******************************************************************************/

static AjBool featRegInitPir(void)
{
    if(FeatInitPir)
	return ajTrue;

    featInit();

    featVocabInitPir();

    /*ajDebug("Tables PIR Type: %x Tags: %x\n",
	    FeatTypeTablePir, FeatTagsTablePir);*/

    /*ajDebug("featRegInitPir Compiling regexps\n");*/
    if(!PirRegexAll)
	PirRegexAll = ajRegCompC("^F;([^/]+)/([^:]+):([^#]*)") ;

    if(!PirRegexCom)
	PirRegexCom = ajRegCompC("^#([^#]*)") ;

    if(!PirRegexLoc)
	PirRegexLoc = ajRegCompC("^([^,]+),?") ;

    if(!PirRegexPos)
	PirRegexPos = ajRegCompC("^([^-]+)-?") ;

    FeatInitPir = ajTrue;

    return ajTrue;
}




/* @funcstatic featRegInitGff *************************************************
**
** Initialise regular expressions and data structures for ajFeat GFF format
**
** @return [AjBool] ajTrue if successful
** @@
******************************************************************************/

static AjBool featRegInitGff(void)
{
    /* Setup any global static runtime resources here
       for example, regular expression compilation calls */

    if(FeatInitGff)
	return ajTrue;

    featInit();

    featVocabInitGff();

    /*ajDebug("featRegInitGff Compiling regexps\n");*/

    GffRegexNumeric   = ajRegCompC("^[\\+-]?[0-9]+\\.?[0-9]*$");
    GffRegexblankline = ajRegCompC("^[ ]*$");
    GffRegexversion   = ajRegCompC("^##gff-version[ ]+([0-9]+)");
    GffRegexdate      = ajRegCompC("^##date[ ]+([0-9][0-9][0-9][0-9])-"
				   "([0-9][0-9]?)-([0-9][0-9]?)");
    GffRegexregion    = ajRegCompC("^##sequence-region[ ]+([0-9a-zA-Z]+)"
				   "[ ]+([\\+-]?[0-9]+)[ ]+([\\+-]?[0-9]+)");
    GffRegexcomment   = ajRegCompC("^#[ ]*(.*)");
    GffRegextype      = ajRegCompC("^##[Tt]ype +(\\S+)( +(\\S+))?");

    GffRegexTvTagval  = ajRegCompC(" *([^ =]+)[ =]((\"(\\.|[^\\\"])*\"|"
			 	   "[^;]+)*)(;|$)"); /* "tag name */

    FeatInitGff = ajTrue;

    return ajTrue;
}




/* @funcstatic featRegInitGff3 ************************************************
**
** Initialise regular expressions and data structures for ajFeat GFF3 format
**
** @return [AjBool] ajTrue if successful
** @@
******************************************************************************/

static AjBool featRegInitGff3(void)
{
    /* Setup any global static runtime resources here
       for example, regular expression compilation calls */

    if(FeatInitGff3)
	return ajTrue;

    featInit();

    featVocabInitGff3();

    /*ajDebug("featRegInitGff3 Compiling regexps\n");*/

    Gff3RegexNumeric   = ajRegCompC("^[\\+-]?[0-9]+\\.?[0-9]*$");
    Gff3Regexblankline = ajRegCompC("^[ ]*$");
    Gff3Regexversion   = ajRegCompC("^##gff-version[ ]+([0-9]+)");
    Gff3Regexregion    = ajRegCompC("^##sequence-region[ ]+([0-9a-zA-Z]+)"
				   "[ ]+([\\+-]?[0-9]+)[ ]+([\\+-]?[0-9]+)");
    Gff3Regexcomment   = ajRegCompC("^#[ ]*(.*)");
    Gff3RegexTvTagval  = ajRegCompC(" *([^ =]+)[ =]((\"(\\.|[^\\\"])*\"|"
			 	   "[^;]+)*)(;|$)"); /* "tag name */

    FeatInitGff3 = ajTrue;

    if(!featRegInitGff())
        return ajFalse;

    return ajTrue;
}




/* @funcstatic featDelRegEmbl *************************************************
**
** Cleanup and exit routines. Free and destroy regular expressions
**
** @return [AjBool] ajFalse if unsuccessful
** @@
******************************************************************************/

static AjBool featDelRegEmbl(void)
{
    if(!FeatInitEmbl)
	return ajTrue;

    ajTablestrFree(&FeatTypeTableEmbl);
    ajTablestrFree(&FeatTagsTableEmbl);

    FeatInitEmbl = ajFalse;

    return ajTrue;
}




/* @funcstatic featDelRegPir **************************************************
**
** Cleanup and exit routines. Free and destroy regular expressions
**
** @return [AjBool] ajFalse if unsuccessful
** @@
******************************************************************************/

static AjBool featDelRegPir(void)
{
    if(!FeatInitPir)
	return ajTrue;

    ajRegFree(&PirRegexAll);
    ajRegFree(&PirRegexCom);
    ajRegFree(&PirRegexLoc);
    ajRegFree(&PirRegexPos);

    ajTablestrFree(&FeatTypeTablePir);
    ajTablestrFree(&FeatTagsTablePir);

    FeatInitPir = ajFalse;

    return ajTrue;
}




/* @funcstatic featDelRegRefseqp **********************************************
**
** Cleanup and exit routines. Free and destroy regular expressions
**
** @return [AjBool] ajFalse if unsuccessful
** @@
******************************************************************************/

static AjBool featDelRegRefseqp(void)
{
    if(!FeatInitRefseqp)
	return ajTrue;

    ajTablestrFree(&FeatTypeTableRefseqp);
    ajTablestrFree(&FeatTagsTableRefseqp);

    FeatInitRefseqp = ajFalse;

    return ajTrue;
}




/* @funcstatic featDelRegSwiss ************************************************
**
** Cleanup and exit routines. Free and destroy regular expressions
**
** @return [AjBool] ajFalse if unsuccessful
** @@
******************************************************************************/

static AjBool featDelRegSwiss(void)
{
    if(!FeatInitSwiss)
	return ajTrue;

    ajRegFree(&SwRegexComment);
    ajRegFree(&SwRegexFtid);
    ajRegFree(&SwRegexNew);
    ajRegFree(&SwRegexNext);

    ajTablestrFree(&FeatTypeTableSwiss);
    ajTablestrFree(&FeatTagsTableSwiss);

    FeatInitSwiss = ajFalse;

    return ajTrue;
}




/* @funcstatic featDelRegGff **************************************************
**
** Cleanup and exit routines. Free and destroy regular expressions
**
** @return [AjBool] ajFalse if unsuccessful
** @@
******************************************************************************/

static AjBool featDelRegGff(void)
{
    if(!FeatInitGff)
	return ajTrue;

    /* Clean-up any global static runtime resources here
       for example, regular expression pattern variables */

    ajRegFree(&GffRegexNumeric);
    ajRegFree(&GffRegexblankline);
    ajRegFree(&GffRegexversion);
    ajRegFree(&GffRegexdate);
    ajRegFree(&GffRegexregion);
    ajRegFree(&GffRegexcomment);
    ajRegFree(&GffRegextype);
    ajRegFree(&GffRegexTvTagval);

    ajTablestrFree(&FeatTypeTableGff);
    ajTablestrFree(&FeatTagsTableGff);
    ajTablestrFree(&FeatTypeTableGffprotein);
    ajTablestrFree(&FeatTagsTableGffprotein);

    FeatInitGff = ajFalse;

    return ajTrue;
}




/* @funcstatic featDelRegGff3 *************************************************
**
** Cleanup and exit routines. Free and destroy regular expressions
**
** @return [AjBool] ajFalse if unsuccessful
** @@
******************************************************************************/

static AjBool featDelRegGff3(void)
{
    if(!FeatInitGff3)
	return ajTrue;

    /* Clean-up any global static runtime resources here
       for example, regular expression pattern variables */

    ajRegFree(&Gff3RegexNumeric);
    ajRegFree(&Gff3Regexblankline);
    ajRegFree(&Gff3Regexversion);
    ajRegFree(&Gff3Regexdate);
    ajRegFree(&Gff3Regexregion);
    ajRegFree(&Gff3Regexcomment);
    ajRegFree(&Gff3Regextype);
    ajRegFree(&Gff3RegexTvTagval);

    ajTablestrFree(&FeatTypeTableGff3);
    ajTablestrFree(&FeatTagsTableGff3);
    ajTablestrFree(&FeatTypeTableGff3protein);
    ajTablestrFree(&FeatTagsTableGff3protein);

    FeatInitGff3 = ajFalse;

    featDelRegGff();

    return ajTrue;
}




/* @func ajFeattableWriteDdbj *************************************************
**
** Write a feature table in DDBJ format.
**
** @param [u] ftout [AjPFeattabOut] Feature table output object
** @param [r] thys [const AjPFeattable] feature table
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeattableWriteDdbj(AjPFeattabOut ftout, const AjPFeattable thys)
{
    return feattableWriteEmbl(thys,ftout->Handle,ajFalse);
}




/* @func ajFeattableWriteEmbl *************************************************
**
** Write a feature table in EMBL format.
**
** @param [u] ftout [AjPFeattabOut] Feature table output object
** @param [r] thys [const AjPFeattable] feature table
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeattableWriteEmbl(AjPFeattabOut ftout, const AjPFeattable thys)
{
    return feattableWriteEmbl(thys,ftout->Handle,ajTrue);
}




/* @func ajFeattableWriteGenbank **********************************************
**
** Write a feature table in GenBank format.
**
** @param [u] ftout [AjPFeattabOut] Feature table output object
** @param [r] thys [const AjPFeattable] feature table
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeattableWriteGenbank(AjPFeattabOut ftout, const AjPFeattable thys)
{
    return feattableWriteEmbl(thys,ftout->Handle,ajFalse);
}




/* @func ajFeattableWriteRefseq **********************************************
**
** Write a feature table in Refseq format.
**
** @param [u] ftout [AjPFeattabOut] Feature table output object
** @param [r] thys [const AjPFeattable] feature table
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeattableWriteRefseq(AjPFeattabOut ftout, const AjPFeattable thys)
{
    return feattableWriteRefseq(thys,ftout->Handle);
}




/* @func ajFeattableWriteRefseqp *********************************************
**
** Write a feature table in Refseqp format.
**
** @param [u] ftout [AjPFeattabOut] Feature table output object
** @param [r] thys [const AjPFeattable] feature table
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeattableWriteRefseqp(AjPFeattabOut ftout, const AjPFeattable thys)
{
    return feattableWriteRefseqp(thys,ftout->Handle);
}




/* @funcstatic feattableWriteEmbl *********************************************
**
** Write a feature table in EMBL format.
**
** @param [r] thys [const AjPFeattable] Feature table
** @param [u] file [AjPFile] Output file
** @param [r] IsEmbl [AjBool] ajTrue for EMBL (different line prefix)
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool feattableWriteEmbl(const AjPFeattable thys, AjPFile file,
			         AjBool IsEmbl)
{
    AjIList    iter     = NULL;
    AjPFeature gf       = NULL;
    AjPFeature gfprev   = NULL;
    AjBool join         = ajFalse;
    AjBool whole        = ajFalse;           /* has "complement(" been added */
    AjPStr location     = NULL;              /* location list as a string */
    AjPStr temp         = NULL;
    AjPStr pos          = NULL;
    ajuint oldgroup = UINT_MAX;
    
    /* Check arguments */
    
    /* ajDebug("feattableWriteEmbl Checking arguments\n"); */

    if(!file)
	return ajFalse;
    
    if(!ajFeattableIsNuc(thys))
	return ajFalse;
    
    /* feature table heading */
    
    if(IsEmbl)
    {
	ajFmtPrintF(file, "FH   Key             Location/Qualifiers\n");
	ajFmtPrintF(file, "FH\n");
    }
    else
	ajFmtPrintF(file, "FEATURES             Location/Qualifiers\n");
    
    location = ajStrNewRes(80);
    temp     = ajStrNewRes(80);
    pos      = ajStrNewRes(80);
    
    /* For all features... we need to process a group at a time */
    
    ajListSort(thys->Features,*featCompByGroup);
    
    if(thys->Features)
    {
	iter = ajListIterNewread(thys->Features);

	while(!ajListIterDone(iter))
	{
	    gf = ajListIterGet(iter);

	    if((oldgroup != gf->Group) && gfprev) /* previous location ready */
	    {
		if(join)
		{
		    ajStrAppendC(&location,")"); /* close bracket for join */
		    /* ajDebug("join: closing ')' appended\n"); */
		}

		if(whole)
		{
		    ajStrInsertC(&location,0,"complement(");
		    ajStrAppendC(&location,")");
		    /* ajDebug("wrap with complement(), reset whole %b to N\n",
		       whole); */
		    whole = ajFalse;
		}
		
		/* ajDebug("calling featDumpEmbl for gfprev\n"); */
		/* ajDebug("location: '%S'\n", location); */
		featDumpEmbl(gfprev, location, file,
			     thys->Seqid, IsEmbl); /* gfprev has tag data */
		
		/* reset the values from previous */
		/* ajDebug("reset location\n"); */
		ajStrSetClear(&location);
		/* ajDebug("reset join  %b to N\n", join); */
		join = ajFalse;
	    }
	    
	    oldgroup = gf->Group;
	    
	    /* process the new gf */
	    
	    /* ajDebug("\n'%S' group: %d exon: %d flags:%x tags: %d\n",
	       gf->Type, gf->Group,gf->Exon, gf->Flags,
	       ajListGetLength(gf->Tags)); */
	    
	    if(gf->Flags & FEATFLAG_COMPLEMENT_MAIN)
	    {
		/* ajDebug("set2 whole %b to Y\n", whole); */
		whole =ajTrue;
	    }
	    
	    if(ajStrGetLen(location))   /* one location already there */
	    {
		if(!join)
		{
		    /* ajDebug("insert 'join(', set join Y\n"); */
		    if(gf->Flags & FEATFLAG_ORDER)
			ajStrInsertC(&location,0,"order(");
		    else
			ajStrInsertC(&location,0,"join(");
		    join = ajTrue;
		}

		ajStrAppendC(&location,",");
		/* ajDebug("append ','\n"); */
	    }
	    
	    ajStrSetClear(&temp);
	    ajStrSetClear(&pos);
	    
	    if(gf->Flags & FEATFLAG_REMOTEID)
	    {
		ajFmtPrintAppS(&pos,"%S:",gf->Remote);
		/* ajDebug("remote: %S\n", gf->Remote); */
	    }
	    
	    if(gf->Flags & FEATFLAG_LABEL)
	    {
		ajFmtPrintAppS(&pos,"%S",gf->Label);
		/* ajDebug("label: %S\n", gf->Label); */
	    }
	    else if(gf->Flags & FEATFLAG_START_BEFORE_SEQ)
	    {
		ajFmtPrintAppS(&pos,"<%d",gf->Start);
		/* ajDebug("<start\n"); */
	    }
	    else if(gf->Flags & FEATFLAG_START_TWO)
	    {
		ajFmtPrintAppS(&pos,"(%d.%d)",gf->Start,gf->Start2);
		/* ajDebug("start2 (%d.%d)\n", gf->Start, gf->Start2); */
	    }
	    else
	    {
		ajFmtPrintAppS(&pos,"%d",gf->Start);
		/* ajDebug("start\n"); */
	    }
	    
	    if(!(gf->Flags & FEATFLAG_POINT))
	    {
		if(gf->Flags & FEATFLAG_BETWEEN_SEQ)
		{
		    ajFmtPrintAppS(&pos,"^%d",gf->End);
		    /* ajDebug("between ^end\n"); */
		}
		else if(gf->Flags & FEATFLAG_END_AFTER_SEQ)
		{
		    ajFmtPrintAppS(&pos,"..>%d",gf->End);
		    /* ajDebug(">end\n"); */
		}
		else if(gf->Flags & FEATFLAG_END_TWO)
		{
		    ajFmtPrintAppS(&pos,"..(%d.%d)",gf->End2,gf->End);
		    /* ajDebug("end2 (%d.%d)\n", gf->End2, gf->End); */
		}
		else
		{
		    ajFmtPrintAppS(&pos,"..%d",gf->End);
		    /* ajDebug(".. end\n"); */
		}
	    }
	    
	    if(gf->Strand == '-' && !whole)
	    {
		ajStrAssignC(&temp,"complement(");
		ajStrAppendS(&temp,pos);
		ajStrAppendC(&temp,")");
		/* ajDebug("strand '-', wrap exon with complement()\n"); */
	    }
	    else 
	    {
		ajStrAssignS(&temp,pos);
		/* ajDebug("simple exon\n"); */
	    }
	    ajStrSetClear(&pos);
	    ajStrAppendS(&location,temp);
	    /* this is the parent/only feature */
	    if(!(gf->Flags & FEATFLAG_CHILD))
		gfprev=gf;
	}
	
	ajListIterDel(&iter);

	if(gfprev)
	{
	    /* Don't forget the last one !!! */
	    if(join)
	    {
		ajStrAppendC(&location,")");	/* close bracket for join */
		/* ajDebug("last: join: closing ')' appended\n"); */
	    }
	    if(whole)
	    {
		ajStrInsertC(&location,0,"complement(");
		ajStrAppendC(&location,")");
		/*
                ** ajDebug("last: wrap with complement(), reset whole %b "
                **            "to N\n", whole);
                */
		whole = ajFalse;
	    }
	
	    /* ajDebug("last: calling featDumpEmbl for gfprev\n"); */
	    /* ajDebug("location: '%S'\n", location); */
	
	    featDumpEmbl(gfprev, location, file,
			 thys->Seqid, IsEmbl) ; /* gfprev has tag data */
	}

	ajStrDel(&location);
	ajStrDel(&pos);
	ajStrDel(&temp);
    }
    
    /* ajDebug("ajFeattableWriteEmbl Done\n"); */
    
    return ajTrue;
}




/* @funcstatic feattableWriteRefseq *******************************************
**
** Write a feature table in REFSEQ format.
**
** @param [r] thys [const AjPFeattable] Feature table
** @param [u] file [AjPFile] Output file
* @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool feattableWriteRefseq(const AjPFeattable thys, AjPFile file)
{
    AjIList    iter     = NULL;
    AjPFeature gf       = NULL;
    AjPFeature gfprev   = NULL;
    AjBool join         = ajFalse;
    AjBool whole        = ajFalse;           /* has "complement(" been added */
    AjPStr location     = NULL;              /* location list as a string */
    AjPStr temp         = NULL;
    AjPStr pos          = NULL;
    ajuint oldgroup = UINT_MAX;
    
    /* Check arguments */
    
    /* ajDebug("feattableWriteRefseq Checking arguments\n"); */

    if(!file)
	return ajFalse;
    
    if(!ajFeattableIsNuc(thys))
	return ajFalse;
    
    /* feature table heading */
    
    ajFmtPrintF(file, "FEATURES             Location/Qualifiers\n");
    
    location = ajStrNewRes(80);
    temp     = ajStrNewRes(80);
    pos      = ajStrNewRes(80);
    
    /* For all features... we need to process a group at a time */
    
    ajListSort(thys->Features,*featCompByGroup);
    
    if(thys->Features)
    {
	iter = ajListIterNewread(thys->Features);

	while(!ajListIterDone(iter))
	{
	    gf = ajListIterGet(iter);

	    if((oldgroup != gf->Group) && gfprev) /* previous location ready */
	    {
		if(join)
		{
		    ajStrAppendC(&location,")"); /* close bracket for join */
		    /* ajDebug("join: closing ')' appended\n"); */
		}

		if(whole)
		{
		    ajStrInsertC(&location,0,"complement(");
		    ajStrAppendC(&location,")");
		    /* ajDebug("wrap with complement(), reset whole %b to N\n",
		       whole); */
		    whole = ajFalse;
		}
		
		/* ajDebug("calling featDumpRefseq for gfprev\n"); */
		/* ajDebug("location: '%S'\n", location); */
		featDumpRefseq(gfprev, location, file,
                               thys->Seqid); /* gfprev has tag data */
		
		/* reset the values from previous */
		/* ajDebug("reset location\n"); */
		ajStrSetClear(&location);
		/* ajDebug("reset join  %b to N\n", join); */
		join = ajFalse;
	    }
	    
	    oldgroup = gf->Group;
	    
	    /* process the new gf */
	    
	    /* ajDebug("\n'%S' group: %d exon: %d flags:%x tags: %d\n",
	       gf->Type, gf->Group,gf->Exon, gf->Flags,
	       ajListGetLength(gf->Tags)); */
	    
	    if(gf->Flags & FEATFLAG_COMPLEMENT_MAIN)
	    {
		/* ajDebug("set2 whole %b to Y\n", whole); */
		whole =ajTrue;
	    }
	    
	    if(ajStrGetLen(location))   /* one location already there */
	    {
		if(!join)
		{
		    /* ajDebug("insert 'join(', set join Y\n"); */
		    if(gf->Flags & FEATFLAG_ORDER)
			ajStrInsertC(&location,0,"order(");
		    else
			ajStrInsertC(&location,0,"join(");
		    join = ajTrue;
		}

		ajStrAppendC(&location,",");
		/* ajDebug("append ','\n"); */
	    }
	    
	    ajStrSetClear(&temp);
	    ajStrSetClear(&pos);
	    
	    if(gf->Flags & FEATFLAG_REMOTEID)
	    {
		ajFmtPrintAppS(&pos,"%S:",gf->Remote);
		/* ajDebug("remote: %S\n", gf->Remote); */
	    }
	    
	    if(gf->Flags & FEATFLAG_LABEL)
	    {
		ajFmtPrintAppS(&pos,"%S",gf->Label);
		/* ajDebug("label: %S\n", gf->Label); */
	    }
	    else if(gf->Flags & FEATFLAG_START_BEFORE_SEQ)
	    {
		ajFmtPrintAppS(&pos,"<%d",gf->Start);
		/* ajDebug("<start\n"); */
	    }
	    else if(gf->Flags & FEATFLAG_START_TWO)
	    {
		ajFmtPrintAppS(&pos,"(%d.%d)",gf->Start,gf->Start2);
		/* ajDebug("start2 (%d.%d)\n", gf->Start, gf->Start2); */
	    }
	    else
	    {
		ajFmtPrintAppS(&pos,"%d",gf->Start);
		/* ajDebug("start\n"); */
	    }
	    
	    if(!(gf->Flags & FEATFLAG_POINT))
	    {
		if(gf->Flags & FEATFLAG_BETWEEN_SEQ)
		{
		    ajFmtPrintAppS(&pos,"^%d",gf->End);
		    /* ajDebug("between ^end\n"); */
		}
		else if(gf->Flags & FEATFLAG_END_AFTER_SEQ)
		{
		    ajFmtPrintAppS(&pos,"..>%d",gf->End);
		    /* ajDebug(">end\n"); */
		}
		else if(gf->Flags & FEATFLAG_END_TWO)
		{
		    ajFmtPrintAppS(&pos,"..(%d.%d)",gf->End2,gf->End);
		    /* ajDebug("end2 (%d.%d)\n", gf->End2, gf->End); */
		}
		else
		{
		    ajFmtPrintAppS(&pos,"..%d",gf->End);
		    /* ajDebug(".. end\n"); */
		}
	    }
	    
	    if(gf->Strand == '-' && !whole)
	    {
		ajStrAssignC(&temp,"complement(");
		ajStrAppendS(&temp,pos);
		ajStrAppendC(&temp,")");
		/* ajDebug("strand '-', wrap exon with complement()\n"); */
	    }
	    else 
	    {
		ajStrAssignS(&temp,pos);
		/* ajDebug("simple exon\n"); */
	    }
	    ajStrSetClear(&pos);
	    ajStrAppendS(&location,temp);
	    /* this is the parent/only feature */
	    if(!(gf->Flags & FEATFLAG_CHILD))
		gfprev=gf;
	}
	
	ajListIterDel(&iter);

	if(gfprev)
	{
	    /* Don't forget the last one !!! */
	    if(join)
	    {
		ajStrAppendC(&location,")");	/* close bracket for join */
		/* ajDebug("last: join: closing ')' appended\n"); */
	    }
	    if(whole)
	    {
		ajStrInsertC(&location,0,"complement(");
		ajStrAppendC(&location,")");
		/*
                ** ajDebug("last: wrap with complement(), reset whole %b "
                **            "to N\n", whole);
                */
		whole = ajFalse;
	    }
	
	    /* ajDebug("last: calling featDumpRefseq for gfprev\n"); */
	    /* ajDebug("location: '%S'\n", location); */
	
	    featDumpRefseq(gfprev, location, file,
                            thys->Seqid) ; /* gfprev has tag data */
	}

	ajStrDel(&location);
	ajStrDel(&pos);
	ajStrDel(&temp);
    }
    
    /* ajDebug("ajFeattableWriteRefseq Done\n"); */
    
    return ajTrue;
}




/* @funcstatic feattableWriteRefseqp *******************************************
**
** Write a feature table in REFSEQP format.
**
** @param [r] thys [const AjPFeattable] Feature table
** @param [u] file [AjPFile] Output file
* @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool feattableWriteRefseqp(const AjPFeattable thys, AjPFile file)
{
    AjIList    iter     = NULL;
    AjPFeature gf       = NULL;
    AjPFeature gfprev   = NULL;
    AjBool join         = ajFalse;
    AjBool whole        = ajFalse;           /* has "complement(" been added */
    AjPStr location     = NULL;              /* location list as a string */
    AjPStr temp         = NULL;
    AjPStr pos          = NULL;
    ajuint oldgroup = UINT_MAX;
    
    /* Check arguments */
    
    /* ajDebug("feattableWriteRefseqp Checking arguments\n"); */

    if(!file)
	return ajFalse;
    
    if(!ajFeattableIsNuc(thys))
	return ajFalse;
    
    /* feature table heading */
    
    ajFmtPrintF(file, "FEATURES             Location/Qualifiers\n");
    
    location = ajStrNewRes(80);
    temp     = ajStrNewRes(80);
    pos      = ajStrNewRes(80);
    
    /* For all features... we need to process a group at a time */
    
    ajListSort(thys->Features,*featCompByGroup);
    
    if(thys->Features)
    {
	iter = ajListIterNewread(thys->Features);

	while(!ajListIterDone(iter))
	{
	    gf = ajListIterGet(iter);

	    if((oldgroup != gf->Group) && gfprev) /* previous location ready */
	    {
		if(join)
		{
		    ajStrAppendC(&location,")"); /* close bracket for join */
		    /* ajDebug("join: closing ')' appended\n"); */
		}

		if(whole)
		{
		    ajStrInsertC(&location,0,"complement(");
		    ajStrAppendC(&location,")");
		    /* ajDebug("wrap with complement(), reset whole %b to N\n",
		       whole); */
		    whole = ajFalse;
		}
		
		/* ajDebug("calling featDumpRefseqp for gfprev\n"); */
		/* ajDebug("location: '%S'\n", location); */
		featDumpRefseqp(gfprev, location, file,
                                thys->Seqid); /* gfprev has tag data */
		
		/* reset the values from previous */
		/* ajDebug("reset location\n"); */
		ajStrSetClear(&location);
		/* ajDebug("reset join  %b to N\n", join); */
		join = ajFalse;
	    }
	    
	    oldgroup = gf->Group;
	    
	    /* process the new gf */
	    
	    /* ajDebug("\n'%S' group: %d exon: %d flags:%x tags: %d\n",
	       gf->Type, gf->Group,gf->Exon, gf->Flags,
	       ajListGetLength(gf->Tags)); */
	    
	    if(gf->Flags & FEATFLAG_COMPLEMENT_MAIN)
	    {
		/* ajDebug("set2 whole %b to Y\n", whole); */
		whole =ajTrue;
	    }
	    
	    if(ajStrGetLen(location))   /* one location already there */
	    {
		if(!join)
		{
		    /* ajDebug("insert 'join(', set join Y\n"); */
		    if(gf->Flags & FEATFLAG_ORDER)
			ajStrInsertC(&location,0,"order(");
		    else
			ajStrInsertC(&location,0,"join(");
		    join = ajTrue;
		}

		ajStrAppendC(&location,",");
		/* ajDebug("append ','\n"); */
	    }
	    
	    ajStrSetClear(&temp);
	    ajStrSetClear(&pos);
	    
	    if(gf->Flags & FEATFLAG_REMOTEID)
	    {
		ajFmtPrintAppS(&pos,"%S:",gf->Remote);
		/* ajDebug("remote: %S\n", gf->Remote); */
	    }
	    
	    if(gf->Flags & FEATFLAG_LABEL)
	    {
		ajFmtPrintAppS(&pos,"%S",gf->Label);
		/* ajDebug("label: %S\n", gf->Label); */
	    }
	    else if(gf->Flags & FEATFLAG_START_BEFORE_SEQ)
	    {
		ajFmtPrintAppS(&pos,"<%d",gf->Start);
		/* ajDebug("<start\n"); */
	    }
	    else if(gf->Flags & FEATFLAG_START_TWO)
	    {
		ajFmtPrintAppS(&pos,"(%d.%d)",gf->Start,gf->Start2);
		/* ajDebug("start2 (%d.%d)\n", gf->Start, gf->Start2); */
	    }
	    else
	    {
		ajFmtPrintAppS(&pos,"%d",gf->Start);
		/* ajDebug("start\n"); */
	    }
	    
	    if(!(gf->Flags & FEATFLAG_POINT))
	    {
		if(gf->Flags & FEATFLAG_BETWEEN_SEQ)
		{
		    ajFmtPrintAppS(&pos,"^%d",gf->End);
		    /* ajDebug("between ^end\n"); */
		}
		else if(gf->Flags & FEATFLAG_END_AFTER_SEQ)
		{
		    ajFmtPrintAppS(&pos,"..>%d",gf->End);
		    /* ajDebug(">end\n"); */
		}
		else if(gf->Flags & FEATFLAG_END_TWO)
		{
		    ajFmtPrintAppS(&pos,"..(%d.%d)",gf->End2,gf->End);
		    /* ajDebug("end2 (%d.%d)\n", gf->End2, gf->End); */
		}
		else
		{
		    ajFmtPrintAppS(&pos,"..%d",gf->End);
		    /* ajDebug(".. end\n"); */
		}
	    }
	    
	    if(gf->Strand == '-' && !whole)
	    {
		ajStrAssignC(&temp,"complement(");
		ajStrAppendS(&temp,pos);
		ajStrAppendC(&temp,")");
		/* ajDebug("strand '-', wrap exon with complement()\n"); */
	    }
	    else 
	    {
		ajStrAssignS(&temp,pos);
		/* ajDebug("simple exon\n"); */
	    }
	    ajStrSetClear(&pos);
	    ajStrAppendS(&location,temp);
	    /* this is the parent/only feature */
	    if(!(gf->Flags & FEATFLAG_CHILD))
		gfprev=gf;
	}
	
	ajListIterDel(&iter);

	if(gfprev)
	{
	    /* Don't forget the last one !!! */
	    if(join)
	    {
		ajStrAppendC(&location,")");	/* close bracket for join */
		/* ajDebug("last: join: closing ')' appended\n"); */
	    }
	    if(whole)
	    {
		ajStrInsertC(&location,0,"complement(");
		ajStrAppendC(&location,")");
		/*
                ** ajDebug("last: wrap with complement(), reset whole %b "
                **            "to N\n", whole);
                */
		whole = ajFalse;
	    }
	
	    /* ajDebug("last: calling featDumpRefseqp for gfprev\n"); */
	    /* ajDebug("location: '%S'\n", location); */
	
	    featDumpRefseqp(gfprev, location, file,
                            thys->Seqid) ; /* gfprev has tag data */
	}

	ajStrDel(&location);
	ajStrDel(&pos);
	ajStrDel(&temp);
    }
    
    /* ajDebug("ajFeattableWriteRefseqp Done\n"); */
    
    return ajTrue;
}




/* @func ajFeattableWriteSwiss ************************************************
**
** Write a feature table in SwissProt format.
**
** @param [u] ftout [AjPFeattabOut] Feature table output object
** @param [r] thys [const AjPFeattable] feature table
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeattableWriteSwiss(AjPFeattabOut ftout,
                             const AjPFeattable thys)
{
    AjIList iter     = NULL;
    AjPFeature gf    = NULL;
    AjPFeature gftop = NULL;
    ajuint oldgroup   = UINT_MAX;
    AjPFile file = ftout->Handle;

    /* Check arguments */
    ajDebug("ajFeattableWriteSwiss Checking arguments\n");

    if(!file)
	return ajFalse;

    if(!ajFeattableIsProt(thys))
	return ajFalse;
    
    /* no FH header in SwissProt */

    /* For all features... */

    if(thys->Features)
    {
	iter = ajListIterNewread(thys->Features);

	while(!ajListIterDone(iter))
	{
	    gf = ajListIterGet(iter);

	    if(oldgroup != gf->Group)
	    {
		oldgroup = gf->Group;
		gftop = gf;
	    }
	    else
	    {
		if(!(gf->Flags & FEATFLAG_CHILD))
		    gftop = gf; /* this is the parent/only feature */
	    }

	    featDumpSwiss(gf, file, gftop) ;
	}

	ajListIterDel(&iter) ;
    }

    return ajTrue ;
}




/* @func ajFeattableWritePir **************************************************
**
** Write a feature table in PIR format.
**
** @param [u] ftout [AjPFeattabOut] Feature table output object
** @param [r] thys [const AjPFeattable] feature table
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeattableWritePir(AjPFeattabOut ftout, const AjPFeattable thys)
{
    AjIList iter      = NULL;
    AjPFeature gf     = NULL;
    AjPFeature gfprev = NULL;
    AjPStr location   = NULL;        /* location list as a string */
    AjPStr temp       = NULL;
    AjPStr pos        = NULL;
    ajuint oldgroup    = UINT_MAX;
    AjPFile file = ftout->Handle;

    /* Check arguments */
    
    /*ajDebug("ajFeattableWritePir Checking arguments\n");*/
    if(!file)
	return ajFalse;
    
    if(!ajFeattableIsProt(thys))
	return ajFalse;
    
    location = ajStrNewRes(80);
    temp     = ajStrNewRes(80);
    pos      = ajStrNewRes(80);
    
    /* For all features... we need to process a group at a time */
    
    ajListSort(thys->Features,*featCompByGroup);
    
    if(thys->Features)
    {
	iter = ajListIterNewread(thys->Features);
	
	while(!ajListIterDone(iter))
	{
	    gf = ajListIterGet(iter);
	    
	    if((oldgroup != gf->Group) && gfprev) /* previous location ready */
	    {
		/*ajDebug("calling featDumpPir for gfprev\n");*/
		/*ajDebug("location: '%S'\n", location);*/
		featDumpPir(gfprev, location, file) ; /* gfprev has tag data */

		/* reset the values from previous */
		/*ajDebug("reset location\n");*/
		ajStrSetClear(&location);
	    }
	    
	    oldgroup = gf->Group;

	    /* process the new gf */

	    /*ajDebug("\n'%S' group: %d exon: %d flags:%x tags: %d\n",
		    gf->Type, gf->Group,gf->Exon, gf->Flags,
		    ajListGetLength(gf->Tags));*/

	    if(ajStrGetLen(location))  /* one location already there */
	    {
		ajStrAppendC(&location,",");
		/*ajDebug("append ','\n");*/
	    }

	    ajStrSetClear(&temp);
	    ajStrSetClear(&pos);

	    ajFmtPrintAppS(&pos,"%d",gf->Start);
	    /*ajDebug("start\n");*/

	    if(gf->End != gf->Start)
		ajFmtPrintAppS(&pos,"-%d",gf->End);

	    ajStrAssignS(&temp,pos);

	    ajStrSetClear(&pos);
	    ajStrAppendS(&location,temp);

	    if(!(gf->Flags & FEATFLAG_CHILD))
		gfprev=gf;	 /* this is the parent/only feature */
	}

	ajListIterDel(&iter);

	/* Don't forget the last one !!! */

	/*ajDebug("last: calling featDumpPir for gfprev\n");*/
	/*ajDebug("location: '%S'\n", location);*/

        if(gfprev)
            featDumpPir(gfprev,location, file); /* gfprev has tag data */
	ajStrDel(&location);
	ajStrDel(&pos);
	ajStrDel(&temp);
    }
    
    /*ajDebug("ajFeattableWritePir Done\n");*/
    
    return ajTrue;
}




/* @func ajFeattableWriteDasgff ***********************************************
**
** Write a feature table in DAS GFF format.
**
** @param [u] ftout [AjPFeattabOut] Feature table output object
** @param [r] thys [const AjPFeattable] Feature table
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeattableWriteDasgff(AjPFeattabOut ftout, const AjPFeattable thys)
{
    /*
    ** Things to do:
    **
    ** Where do the tag-values go? In <NOTE> for now
    **
    ** Check standards for id, type etc. for table and features
    */

/*#define MULTINOTE 1*/
    AjPFile file = ftout->Handle;
    AjIList iter     = NULL;
    AjIList tagiter     = NULL;
    const AjPStr outtyp = NULL;		/* these come from AjPTable */
    const AjPStr outtag = NULL;		/* so please, please */
    FeatPTagval item = NULL;
    AjPFeature gf    = NULL;
    AjPFeature gftop = NULL;
    AjPTable tagstable = NULL;
    AjPTable typetable = NULL;
    ajuint oldgroup   = UINT_MAX;
    ajuint nfeat = 0;
    char frame;
    AjBool knowntag = ajTrue;

    AjPStr mytagname = NULL;
    AjPStr mytagval = NULL;

    ajuint ntags;
    ajuint itag;
    const char* cp;

    /* Check arguments */
    ajDebug("ajFeattableWriteDasgff Checking arguments\n");

    if(!file)
	return ajFalse;

    if(!ftout->Count)
    {
        ftout->Cleanup = featCleanDasgff;
        ajFmtPrintF(file,
                    "<?xml version=\"1.0\" standalone=\"no\"?>\n");
        ajFmtPrintF(file,
                    "<!DOCTYPE DASGFF SYSTEM \"http://www.biodas.org/dtd"
                    "/dasgff.dtd\">\n");
        ajFmtPrintF(file,
                    "<DASGFF>\n");
        ajFmtPrintF(file,
                    "  <GFF version=\"1.0\" href=\"url\">\n");
    }

    ajFmtPrintF(file,
                "    <SEGMENT id=\"%S\" start=\"%d\" stop=\"%d\"\n",
                thys->Seqid,
                1+thys->Offset+thys->Start,
                1+thys->Offset+thys->Len);

    /* label is optional */
    /*
    ajFmtPrintF(outseq->File,
                    "                label=\"%s\">", "");
    */

    /* type attribute is optional and may not mean this */
    /*
    if(ajFeattableIsProt(thys))
        ajFmtPrintF(outseq->File,
                    " type=\"Protein\">");
    else
        ajFmtPrintF(outseq->File,
                    " type=\"DNA\">");
    */
    ajFmtPrintF(file,
                "                version=\"%s\">\n",
                "0.0");

    
    /* For all features... */

    /* SEGMENT requires at least one feature - if none, make one up */

    if(thys->Features)
    {
	iter = ajListIterNewread(thys->Features);

	while(!ajListIterDone(iter))
	{
	    gf = ajListIterGet(iter);
            nfeat++;

            if(gf->Protein)
            {
                typetable = FeatTypeTableGff3protein;
                tagstable = FeatTagsTableGff3protein;
                ajDebug("ajFeattableWriteDasgff GFF3 protein table\n");
            }
            else
            {
                typetable = FeatTypeTableGff3;
                tagstable = FeatTagsTableGff3;
                ajDebug("ajFeattableWriteDasgff GFF3 nucleotide table\n");
            }

            outtyp = featTableTypeExternal(gf->Type, typetable);

	    if(oldgroup != gf->Group)
	    {
		oldgroup = gf->Group;
		gftop = gf;
	    }
	    else
	    {
		if(!(gf->Flags & FEATFLAG_CHILD))
		    gftop = gf; /* this is the parent/only feature */
	    }

            ajFmtPrintF(file,
                        "      <FEATURE id=\"%S.%S.%d\">\n",
                        thys->Seqid, gf->Source, gf->Group);
            ajFmtPrintF(file,
                        "         <TYPE id=\"%S\" category=\"%S\" "
                        "reference=\"no\" subparts=\"no\">%S</TYPE>\n",
                        gf->Type, ajFeatTypeGetCategory(gf->Type), outtyp);

            /*
            ** METHOD id should be something the server can return
            ** For application results this is the application name
            */
            if(ajStrGetLen(gf->Source))
                ajFmtPrintF(file,
                            "         <METHOD id=\"%S\"></METHOD>\n",
                            gf->Source);
            else
                ajFmtPrintF(file,
                        "         <METHOD></METHOD>\n",
                        gf->Source);

            /*
            ** START and END are optional in DAS 1.6 but we always have them
            */
            ajFmtPrintF(file,
                        "         <START>%d</START>\n",
                        gf->Start);
            ajFmtPrintF(file,
                        "         <END>%d</END>\n",
                        gf->End);
            /*
            ** SCORE is optional in DAS 1.6
            */
            ajFmtPrintF(file,
                        "         <SCORE>%.f</SCORE>\n",
                        gf->Score);
            /*
            ** ORIENTATION is optional in DAS 1.6
            ** for proteins or non-transcriptional features
            */
            if(gf->Strand)
                ajFmtPrintF(file,
                            "         <ORIENTATION>%c</ORIENTATION>\n",
                            gf->Strand); /* 0 or - or + */

            else
                ajFmtPrintF(file,
                            "         <ORIENTATION>0</ORIENTATION>\n");

            /*
            ** PHASE is optional in DAS 1.6
            ** for proteins or non-translational features
            */
            if(ajFeatTypeIsCds(gf))
                frame = featFrameNuc(gf->Frame);
            else
                frame = featFrame(gf->Frame);

            if(gf->Frame == 0)
                ajFmtPrintF(file,
                            "         <PHASE>-</PHASE>\n");
            else
                ajFmtPrintF(file,
                            "         <PHASE>%c</PHASE>\n",
                            frame);

            /*
            ** tag-value pairs are written as NOTES
            ** We write one note for each
            ** This displays well in various DAS client browsers
            */
            if(gf->Tags)
            {
                ntags = ajListGetLength(gf->Tags);
                itag = 0;
                tagiter = ajListIterNewread(gf->Tags);

                while(!ajListIterDone(tagiter))
                {
                    item = (FeatPTagval)ajListIterGet(tagiter);
                    outtag = featTableTag(item->Tag, tagstable, &knowntag);

                    if(!outtag)
                    {
                        featWarn("Unknown GFF3 feature tag '%S'",
                                 item->Tag);
                        continue;
                    }

                    featTagFmt(outtag, tagstable, &featFmtTmp);

#ifdef MULTINOTE
                    if(ntags == 1)
                    {
                        ajFmtPrintF(file,
                                        "         <NOTE>");
                    }
                    else 
                    {
                        if(!itag)
                            ajFmtPrintF(file,
                                        "         <NOTE>\n");
                    }
#else
                    ajFmtPrintF(file,
                                "         <NOTE>");
#endif                    

                    ajFmtPrintS(&featOutStr, "%S", outtag);
                    ajStrAssignS(&featValTmp, item->Value);
                    cp = ajStrGetPtr(featFmtTmp);

                    switch(CASE2(cp[0], cp[1]))
                    {
                        case CASE2('L','I') :  /* limited */
                            /*ajDebug("case limited\n");*/
                            featTagLimit(outtag, tagstable, &featLimTmp);
                            featTagAllLimit(&featValTmp, featLimTmp);
                            ajFmtPrintAppS(&featOutStr, ":%S", featValTmp);
                            break;
                        case CASE2('Q', 'L') : /* limited, escape quotes */
                            /*ajDebug("case qlimited\n");*/
                            featTagLimit(outtag, tagstable, &featLimTmp);
                            featTagAllLimit(&featValTmp, featLimTmp);
                            /*featTagQuoteGff3(&featValTmp);*/
                            ajFmtPrintAppS(&featOutStr, ":%S", featValTmp);
                            break;
                        case CASE2('T','E') : /* no space/quotes, wrap margin */
                            /*ajDebug("case text\n");*/
                            ajStrRemoveWhite(&featValTmp);
                            ajFmtPrintAppS(&featOutStr, ":%S", featValTmp);
                            break;
                        case CASE2('Q','T') : /* escape quotes, wrap at space */
                            /*ajDebug("case qtext\n");*/
                            /*featTagQuoteGff3(&featValTmp);*/

                            if(ajStrMatchC(outtag, "note") &&
                               ajStrGetCharFirst(featValTmp) == '*')
                            {
                                ajStrCutStart(&featValTmp,1);
                                ajStrExtractWord(featValTmp, &mytagval,
                                                 &mytagname);
                                ajFmtPrintS(&featOutStr, "%S:%S",
                                            mytagname, mytagval);
                                ajStrDel(&mytagname);
                                ajStrDel(&mytagval);
                            }
                            else
                                ajFmtPrintAppS(&featOutStr, ":%S", featValTmp);

                            break;
                        case CASE2('Q','W') : /* escape quotes, remove space */
                            /*ajDebug("case qtext\n");*/
                            /*featTagQuoteGff3(&featValTmp);*/
                            ajStrRemoveWhite(&featValTmp);
                            ajFmtPrintAppS(&featOutStr, ":%S", featValTmp);
                            break;
                        case CASE2('Q', 'S') : /* special regexp, quoted */
                            /*ajDebug("case qspecial\n");*/

                            if(!featTagGff3Special(&featValTmp, outtag))
                                featTagDasgffDefault(&featOutStr, outtag,
                                                     &featValTmp);
                             else
                            {
                                /*featTagQuoteGff3(&featValTmp);*/
                                ajFmtPrintAppS(&featOutStr, ":%S", featValTmp);
                            }

                            break;
                        case CASE2('S','P') :	/* special regexp */
                            /*ajDebug("case special\n");*/

                            if(!featTagGff3Special(&featValTmp, outtag))
                                featTagDasgffDefault(&featOutStr, outtag,
                                                     &featValTmp);
                            else
                                ajFmtPrintAppS(&featOutStr, ":%S", featValTmp);
                            
                            break;
                        case CASE2('V','O') :	/* no value, so an error here */
                            /*ajDebug("case void\n");*/
                            break;
                        default:
                            featWarn("Unknown GFF3 feature tag type '%S' "
                                     "for '%S'",
                                     featFmtTmp, outtag);
                    }
                    
/* TESTING: for single NOTE write ';' after each tag-value pair */

#ifdef MULTINOTE
                    if(++itag < ntags)
                        ajFmtPrintAppS(&featOutStr,";");

                    if(ntags == 1)
                    {
                        ajFmtPrintF(file,
                                    "%S</NOTE>\n",featOutStr);

                    }
                    else 
                    {
                        ajFmtPrintF(file, "           %S\n", featOutStr);

                        if(itag == ntags)
                            ajFmtPrintF(file,
                                        "         </NOTE>\n");

                    }
#else
                    ajFmtPrintF(file,
                                "%S</NOTE>\n",featOutStr);
#endif
                    
                }
                ajListIterDel(&tagiter);
                
            }

            /* link to more information about the feature */
            /*
            ajFmtPrintF(file,
                        "         <LINK href=\"%s\">%s</LINK>\n",
                        "url-here", "text-here);
            */
            
            /* target in an alignment */
            /*
            ajFmtPrintF(file,
                        "         <TARGET id\"%s\" start=\"%d\" "
                        "stop=\"%d\">%s</TARGET>\n",
                        "target-id", target->start, target->stop,"target-name");
            */


/* GROUP is deprecated in DAS 1.6 - replaced by PARENT and PART */
/*           ajFmtPrintF(file,
                       "         <GROUP id=\"%S.%d\" label=\"%S.%d\" "
                       "type=\"%S.%d\">\n",
                       thys->Seqid, gf->Group,
                       thys->Seqid, gf->Group,
                       thys->Seqid, gf->Group);*/
/* link to more GROUP information */
/*
            ajFmtPrintF(file,
                        "               <LINK href=\"%s\">%s</LINK>\n",
                        "url-here", "text-here);
*/

/* target in a GROUP alignment */
/*
            ajFmtPrintF(file,
                        "               <TARGET id\"%s\" start=\"%d\" "
                        "stop=\"%d\">%s</TARGET>\n",
                        "target-id", target->start, target->stop,"target-name");
           ajFmtPrintF(file,
                       "         </GROUP>\n");
*/

           ajFmtPrintF(file,
                        "      </FEATURE>\n");
 	}
	ajListIterDel(&iter) ;
    }

    /* it seems unclear whether DASGFF allows empty segments
    ** this block can be used if some dummy features is mandatory
    ** but note that any feature will appear in a DASGFF viewer
    ** unless there is some official dummy type available
    **
    ** Common practice is to ignore the DTD and write an empty SEGMENT
    ** so this section should not be needed. DAS 1.6 will correct the spec
    */

/*
  if(!nfeat)
    {
            ajFmtPrintF(file,
                        "      <FEATURE id=\"%S\">\n",
                        thys->Seqid);

            ajFmtPrintF(file,
                        "         <TYPE id=\"%s\" category=\"%s\" reference=\"no\" subparts=\"no\">%s</TYPE>\n",
                        "dummy", "feature", "dummy)";
            if(ajStrGetLen(gf->Source))
                ajFmtPrintF(file,
                            "         <METHOD id=\"%S\"></METHOD>\n",
                            gf->Source);
            else
                ajFmtPrintF(file,
                        "         <METHOD></METHOD>\n");
            ajFmtPrintF(file,
                        "         <START>1</START>\n");
            ajFmtPrintF(file,
                        "         <END>%d</END>\n",
                        thys->Len);
            ajFmtPrintF(file,
                        "         <SCORE>0.0</SCORE>\n");
            ajFmtPrintF(file,
                        "         <ORIENTATION>0</ORIENTATION>\n");
            ajFmtPrintF(file,
                        "         <PHASE>-</PHASE>\n");
            ajFmtPrintF(file,
                        "         <NOTE>%s</NOTE>\n",
                        "Written by EMBOSS");
            ajFmtPrintF(file,
                        "      </FEATURE>\n");
    }
*/   
    ajFmtPrintF(file,
                "    </SEGMENT>\n");

    return ajTrue;
}




/* @funcstatic featCleanDasgff *************************************************
**
** Writes the remaining lines to complete and close a DASGFF XML file
**
** @param [u] file [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/


static void featCleanDasgff(AjPFile file)
{

    ajFmtPrintF(file,
                "  </GFF>\n");
    ajFmtPrintF(file,
                "</DASGFF>\n");

    return;
}




/* @func ajFeatTypeGetCategory ************************************************
**
** returns the category name for a feature type
**
** Used to identify category for DASGFF output
**
** @param [r] type [const AjPStr] Feature type
** @return [const AjPStr] Feature category
**
******************************************************************************/

const AjPStr ajFeatTypeGetCategory(const AjPStr type)
{
    ajuint i;
    AjPStrTok catsplit  = NULL;
    AjPStr token = NULL;
    AjPStr name = NULL;
    AjPStr types = NULL;
    AjPStr refname = NULL;
    const AjPStr ret = NULL;

    if(FeatCategoryTable == NULL)
    {
        FeatCategoryTable = ajTablestrNewCaseLen(200);

        for(i=0;featCategory[i].Name;i++)
        {
            name = ajStrNewC(featCategory[i].Name);
            types = ajStrNewC(featCategory[i].Types);

            if(!i)
            {
                token = ajStrNewC("");
                refname = ajStrNewRef(name);
                ajTablePut(FeatCategoryTable, token, refname);
                token = NULL;
            }
            
            catsplit = ajStrTokenNewC(types,",");

            while(ajStrTokenNextParse(&catsplit,&token))
            {
                refname = ajStrNewRef(name);
                ajTablePut(FeatCategoryTable, token, refname);
                token = NULL;
            }

            ajStrDel(&name);
            ajStrDel(&types);
            ajStrTokenDel(&catsplit);
        }
        
    }
    
    ret = ajTablestrFetch(FeatCategoryTable,type);
    if(!ret)
    {
        token = ajStrNewC("");
        ret = ajTablestrFetch(FeatCategoryTable,token);
        ajStrDel(&token);
    }

    return ret;
}




/* @func ajFeattableGetTypeC **************************************************
**
** Returns the name of a feature table object. This is a copy of the
** pointer to the name, and is still owned by the feature table
** and is not to be destroyed.
**
** @param [r] thys [const AjPFeattable] Feature table
** @return [const char*] Feature table type.
** @@
******************************************************************************/

const char* ajFeattableGetTypeC(const AjPFeattable thys)
{
    return ajStrGetPtr(thys->Type);
}




/* @func ajFeattableGetTypeS **************************************************
**
** Returns the name of a feature table object. This is a copy of the
** pointer to the name, and is still owned by the feature table
** and is not to be destroyed.
**
** @param [r] thys [const AjPFeattable] Feature table
** @return [const AjPStr] Feature table name.
** @@
******************************************************************************/

const AjPStr ajFeattableGetTypeS(const AjPFeattable thys)
{
    return thys->Type;
}




/* @func ajFeattableGetXrefs ***************************************************
**
** Returns all cross-references from a feature table
**
** @param [r] thys [const AjPFeattable] Feature table
** @param [u] Pxreflist [AjPList*] List of sequence cross-reference objects
** @param [w] Ptaxid [ajuint*] Taxon ID
** @return [AjBool] True on success
** @@
******************************************************************************/

AjBool ajFeattableGetXrefs(const AjPFeattable thys, AjPList *Pxreflist,
                           ajuint *Ptaxid)
{
    AjIList iterfeat     = NULL;
    AjIList itertags     = NULL;
    FeatPTagval item = NULL;
    AjPSeqXref  xref = NULL;
    ajint ipos;
    ajuint inum = 0;
    AjPFeature feat  = NULL;
    AjPList xreflist;

    if(!*Pxreflist)
        *Pxreflist = ajListNew();
    xreflist = *Pxreflist;

    *Ptaxid = 0;
    
    if(thys->Features)
    {
	iterfeat = ajListIterNewread(thys->Features);

	while(!ajListIterDone(iterfeat))
	{
            feat = (AjPFeature)ajListIterGet(iterfeat);
            if(feat->Tags)
            {
                itertags = ajListIterNewread(feat->Tags);

                while(!ajListIterDone(itertags))
                {
                    item = (FeatPTagval)ajListIterGet(itertags);

                    if(ajStrMatchCaseC(item->Tag, "db_xref"))
                    {
                        ipos = ajStrFindAnyK(item->Value, ':');
                        if(ipos > 0) 
                        {
                            inum++;
                            xref = ajSeqxrefNew();
                            ajStrAssignSubS(&xref->Db, item->Value, 0, ipos-1);
                            ajStrAssignSubS(&xref->Id, item->Value, ipos+1, -1);
                            ajListPushAppend(xreflist, xref);
                            xref->Start = ajFeatGetStart(feat);
                            xref->End   = ajFeatGetEnd(feat);
                            xref->Type = XREF_DBXREF;
                            if(!*Ptaxid && ajStrMatchCaseC(xref->Db, "taxon"))
                            {
                                if(!ajStrToUint(xref->Id, Ptaxid))
                                    *Ptaxid = 0;
                            }
                            xref = NULL;
                        }
                    }
                }
            }
            ajListIterDel(&itertags);
        }
    }

    ajListIterDel(&iterfeat);

    if(!inum)
        return ajFalse;

    return ajTrue;
}




/* @funcstatic featFrame ******************************************************
**
** Converts a frame number in the range 0 to 3 to a GFF frame character
** or '.' as the general default
**
** @param [r] frame [ajint] Feature frame number
** @return [char] character for this frame in GFF
******************************************************************************/

static char featFrame(ajint frame)
{
    static char framestr[] = ".012";

    if(frame < 0)
	return '.';

    if(frame > 3)
	return '.';

    return framestr[frame];
}




/* @funcstatic featFrameNuc ***************************************************
**
** Converts a frame number in the range 0 to 3 to a GFF frame character
** or '0' for the nucleotide default
**
** @param [r] frame [ajint] Feature frame number
** @return [char] character for this frame in GFF
******************************************************************************/

static char featFrameNuc(ajint frame)
{
    static char framestr[] = ".012";

    if(frame <= 0)
	return '0';

    if(frame > 3)
	return '0';

    return framestr[frame];
}




/* @funcstatic featStrand *****************************************************
**
** Converts a strand number to a GFF strand character. NULL characters
** are converted to '+' All other values are simply cast to character.
**
** @param [r] strand [ajint] Strand
** @return [char] GFF character for this strand.
** @@
******************************************************************************/

static char featStrand(ajint strand)
{
    if(ajSysCastItoc(strand) != '-')
	return '+';

    return '-';
}




/* @func ajFeattableIsNuc *****************************************************
**
** Returns ajTrue if a feature table is nucleotide
**
** @param [r] thys [const AjPFeattable] Feature table
** @return [AjBool] ajTrue for a protein feature table
** @@
******************************************************************************/

AjBool ajFeattableIsNuc(const AjPFeattable thys)
{
    if(ajStrMatchC(thys->Type, "N"))
	return ajTrue;

    if(ajStrMatchC(thys->Type, "P"))
	return ajFalse;

    return ajTrue;
}




/* @func ajFeattableIsProt ****************************************************
**
** Returns ajTrue if a feature table is protein
**
** @param [r] thys [const AjPFeattable] Feature table
** @return [AjBool] ajTrue for a protein feature table
** @@
******************************************************************************/

AjBool ajFeattableIsProt(const AjPFeattable thys)
{
    if(ajStrMatchC(thys->Type, "P"))
	return ajTrue;

    if(ajStrMatchC(thys->Type, "N"))
	return ajFalse;

    return ajTrue;
}




/* @func ajFeattableGetBegin ***************************************************
**
** Returns the feature table start position, or 1 if no start has been set.
**
** @param [r] thys [const AjPFeattable] feature table object
** @return [ajint] Start position.
** @@
******************************************************************************/

ajint ajFeattableGetBegin(const AjPFeattable thys)
{
    if(!thys->Start)
	return 1;

    return ajFeattablePos(thys, thys->Start);
}




/* @obsolete ajFeattableBegin
** @rename ajFeattableGetBegin
*/

__deprecated ajint ajFeattableBegin(const AjPFeattable thys)
{
    return ajFeattableGetBegin(thys);
}




/* @func ajFeattableGetEnd *****************************************************
**
** Returns the features table end position, or the feature table length if
** no end has been set.
**
** @param [r] thys [const AjPFeattable] feature table object
** @return [ajint] End position.
** @@
******************************************************************************/

ajint ajFeattableGetEnd(const AjPFeattable thys)
{
    if(!thys->End)
	return (ajFeattableGetLen(thys));

    return ajFeattablePosI(thys, ajFeattableGetBegin(thys), thys->End);
}




/* @obsolete ajFeattableEnd
** @rename ajFeattableGetEnd
*/

__deprecated ajint ajFeattableEnd(const AjPFeattable thys)
{
    return ajFeattableGetEnd(thys);
}




/* @func ajFeattableGetLen *****************************************************
**
** Returns the sequence length of a feature table
**
** @param [r] thys [const AjPFeattable] Feature table
** @return [ajint] Length in bases or residues
** @@
******************************************************************************/

ajint ajFeattableGetLen(const AjPFeattable thys)
{
    if(!thys)
	return 0;

    return (thys->Len);
}




/* @obsolete ajFeattableLen
** @rename ajFeattableGetLen
*/

__deprecated ajint ajFeattableLen(const AjPFeattable thys)
{
    return ajFeattableGetLen(thys);
}




/* @func ajFeattableGetName ***************************************************
**
** Returns the name of a feature table object. This is a copy of the
** pointer to the name, and is still owned by the feature table
** and is not to be destroyed.
**
** @param [r] thys [const AjPFeattable] Feature table
** @return [const AjPStr] Feature table name.
** @@
******************************************************************************/

const AjPStr ajFeattableGetName(const AjPFeattable thys)
{
    return thys->Seqid;
}




/* @func ajFeattableGetSize ***************************************************
**
** Returns the size of a feature table object.
**
** @param [r] thys [const AjPFeattable] Feature table
** @return [ajuint] Feature table size.
** @@
******************************************************************************/

ajuint ajFeattableGetSize(const AjPFeattable thys)
{
    if(!thys)
        return 0;

    return ajListGetLength(thys->Features);
}




/* @obsolete ajFeattableSize
** @rename ajFeattableGetSize
*/

__deprecated ajint ajFeattableSize(const AjPFeattable thys)
{
    return ajFeattableGetSize(thys);
}




/* @func ajFeattabInClear *****************************************************
**
** Clears a Tabin input object back to "as new" condition, except
** for the USA list which must be preserved.
**
** @param [u] thys [AjPFeattabIn] Sequence input
** @return [void]
** @@
******************************************************************************/

void ajFeattabInClear(AjPFeattabIn thys)
{
    ajDebug("ajFeattabInClear called Local:%B\n", thys->Local);

    ajStrSetClear(&thys->Ufo);
    ajStrSetClear(&thys->Seqname);
    ajStrSetClear(&thys->Formatstr);
    ajStrSetClear(&thys->Filename);
    ajStrSetClear(&thys->Seqid);
    ajStrSetClear(&thys->Type);

    if(!thys->Local)
    {
        ajFilebuffDel(&thys->Handle);

        if(thys->Handle)
            ajFatal("ajFeattabInClear did not delete Handle");
    }

    return;
}




/* @func ajFeatGetFlags *******************************************************
**
** Returns the sequence matching a feature. For multiple location features
** (joins in an EMBL/GenBank feature table) the full feature table is used
** to find all exons.
**
** The database name is used to retrieve sequences from other entries
**
** @param [r] thys [const AjPFeature] Feature
** @param [u] Pflagstr [AjPStr*] Sequence for this feature
** @return [AjBool] True on success
** @@
******************************************************************************/

AjBool ajFeatGetFlags(const AjPFeature thys,  AjPStr* Pflagstr)
{
    ajStrAssignC(Pflagstr, "");

    if(thys->Flags & FEATFLAG_START_BEFORE_SEQ)
        ajStrAppendC(Pflagstr, "<start ");
    if(thys->Flags & FEATFLAG_END_AFTER_SEQ)
        ajStrAppendC(Pflagstr, ">end ");
    if(thys->Flags & FEATFLAG_CHILD)
        ajStrAppendC(Pflagstr, "child-exon ");
    if(thys->Flags & FEATFLAG_BETWEEN_SEQ)
        ajStrAppendC(Pflagstr, "x^y ");
    if(thys->Flags & FEATFLAG_START_TWO)
        ajStrAppendC(Pflagstr, "startrange ");
    if(thys->Flags & FEATFLAG_END_TWO)
        ajStrAppendC(Pflagstr, "endrange ");
    if(thys->Flags & FEATFLAG_POINT)
        ajStrAppendC(Pflagstr, "single-base ");
    if(thys->Flags & FEATFLAG_COMPLEMENT_MAIN)
        ajStrAppendC(Pflagstr, "complement(join) ");
    if(thys->Flags & FEATFLAG_MULTIPLE)
        ajStrAppendC(Pflagstr, "multiple ");
    if(thys->Flags & FEATFLAG_GROUP)
        ajStrAppendC(Pflagstr, "group ");
    if(thys->Flags & FEATFLAG_ORDER)
        ajStrAppendC(Pflagstr, "order ");
    if(thys->Flags & FEATFLAG_ONEOF)
        ajStrAppendC(Pflagstr, "oneof ");
    if(thys->Flags & FEATFLAG_REMOTEID)
        ajStrAppendC(Pflagstr, "remoteid ");
    if(thys->Flags & FEATFLAG_LABEL)
        ajStrAppendC(Pflagstr, "LABEL ");
    if(thys->Flags & FEATFLAG_START_UNSURE)
        ajStrAppendC(Pflagstr, "start-unsure ");
    if(thys->Flags & FEATFLAG_END_UNSURE)
        ajStrAppendC(Pflagstr, "end-unsure ");

    ajStrTrimWhite(Pflagstr);

    return ajTrue;
}




/* @func ajFeatGetSeq *********************************************************
**
** Returns the sequence matching a feature. 
**
** The database name is used to retrieve sequences from other entries
**
** @param [r] thys [const AjPFeature] Feature
** @param [r] seq [const AjPSeq] Sequence for the current feature table
** @param [u] Pseqstr [AjPStr*] Sequence for this feature
** @return [AjBool] True on success
** @@
******************************************************************************/

AjBool ajFeatGetSeq(const AjPFeature thys,
                    const AjPSeq seq, AjPStr* Pseqstr)
{
    AjPSeq remoteseq = NULL;
    AjBool isjoin = ajFalse;
    AjPStr tmpseq = NULL;
    AjBool compjoin = ajFalse;

    ajStrSetClear(Pseqstr);

    isjoin = ajFeatIsMultiple(thys);

    ajDebug("ajFeatGetSeq usa:%S\n",
            ajSeqGetUsaS(seq));

    if(thys->Flags & FEATFLAG_BETWEEN_SEQ)
        return ajTrue;

    ajFeatTrace(thys);

    if(thys->Flags & FEATFLAG_REMOTEID)
    {
        if(!remoteseq)
            remoteseq = ajSeqNew();

        ajFeatGetRemoteseq(thys, ajSeqGetUsaS(seq), remoteseq);
        ajStrAppendS(Pseqstr, ajSeqGetSeqS(remoteseq));
    }
    else
    {
        if(thys->Strand == '-' && !compjoin)
        {
            ajStrAppendSubS(&tmpseq, ajSeqGetSeqS(seq),
                            ajFeatGetStart(thys)-1, ajFeatGetEnd(thys)-1);
            ajSeqstrReverse(&tmpseq);
            ajStrInsertS(Pseqstr, 0, tmpseq);
            ajStrDel(&tmpseq);
        }
        else
        {
            ajStrAppendSubS(Pseqstr, ajSeqGetSeqS(seq),
                            ajFeatGetStart(thys)-1, ajFeatGetEnd(thys)-1);
        }
    }

    ajSeqDel(&remoteseq);

    return ajTrue;
}




/* @func ajFeatGetSeqJoin ******************************************************
**
** Returns the sequence matching a feature. For multiple location features
** (joins in an EMBL/GenBank feature table) the full feature table is used
** to find all exons.
**
** The database name is used to retrieve sequences from other entries
**
** @param [r] thys [const AjPFeature] Feature
** @param [r] table [const AjPFeattable] Full feature table
** @param [r] seq [const AjPSeq] Sequence for the current feature table
** @param [u] Pseqstr [AjPStr*] Sequence for this feature
** @return [AjBool] True on success
** @@
******************************************************************************/

AjBool ajFeatGetSeqJoin(const AjPFeature thys, const AjPFeattable table,
                        const AjPSeq seq, AjPStr* Pseqstr)
{
    const AjPFeature gf;
    AjIList iter = NULL;
    AjPSeq remoteseq = NULL;
    AjBool isjoin = ajFalse;
    AjPStr tmpseq = NULL;
    AjBool compjoin = ajFalse;
    AjPStr flags = NULL;
    ajuint count=0;

    ajStrSetClear(Pseqstr);

    isjoin = ajFeatIsMultiple(thys);
    if(thys->Flags & FEATFLAG_COMPLEMENT_MAIN)
        compjoin = ajTrue;

    ajDebug("ajFeatGetSeqJoin nfeat:%u usa:%S\n",
            ajFeattableGetSize(table), ajSeqGetUsaS(seq));
    iter = ajListIterNewread(table->Features);

    while(!ajListIterDone(iter))
    {
        count++;
        gf = (const AjPFeature) ajListIterGet(iter);

        if(gf->Group == thys->Group)
        {
            ajFeatGetFlags(gf, &flags);
            ajFeatTrace(gf);

            if(gf->Flags & FEATFLAG_BETWEEN_SEQ)
                continue;

            if(gf->Flags & FEATFLAG_REMOTEID)
            {
                if(!remoteseq)
                    remoteseq = ajSeqNew();

                ajFeatGetRemoteseq(gf, ajSeqGetUsaS(seq), remoteseq);
                ajStrAppendS(Pseqstr, ajSeqGetSeqS(remoteseq));
                               
            }
            else
            {
                if(gf->Strand == '-' && !compjoin)
                {
                    ajStrAppendSubS(&tmpseq, ajSeqGetSeqS(seq),
                                    ajFeatGetStart(gf)-1, ajFeatGetEnd(gf)-1);
                    ajSeqstrReverse(&tmpseq);
                    ajStrAppendS(Pseqstr, tmpseq);
                    ajStrDel(&tmpseq);
                }
                else
                {
                    ajStrAppendSubS(Pseqstr, ajSeqGetSeqS(seq),
                                    ajFeatGetStart(gf)-1, ajFeatGetEnd(gf)-1);
                }
            }
        }
    }

    if(compjoin)
        ajSeqstrReverse(Pseqstr);

    ajListIterDel(&iter);
    ajSeqDel(&remoteseq);
    ajStrDel(&flags);

    return ajTrue;
}




/* @func ajFeatGetRemoteseq ****************************************************
**
** Returns a sequence entry from a feature location which points to
** another entry
**
** @param [r] thys [const AjPFeature] Feature
** @param [r] usa [const AjPStr] usa of query
** @param [u] seq [AjPSeq] Sequence object for results
** @return [AjBool] True on success
** @@
******************************************************************************/

AjBool ajFeatGetRemoteseq(const AjPFeature thys, const AjPStr usa,
                                AjPSeq seq)
{
    AjPStr baseusa = NULL;
    ajuint ilen;
    const AjPStr remoteid = NULL;

    remoteid = ajSeqtestIsSeqversion(thys->Remote);

    if(!remoteid)
        remoteid = thys->Remote;

    ajSeqUsaGetBase(usa, &baseusa);
    
    ajStrAppendK(&baseusa, ':');
    ajStrAppendS(&baseusa, remoteid);
    ajSeqGetFromUsa(baseusa, thys->Protein, seq);

    ilen = ajSeqGetLen(seq);

    if(thys->Strand == '-')
        ajSeqSetRangeRev(seq, ajFeatGetStart(thys), ajFeatGetEnd(thys));
    else
        ajSeqSetRange(seq, ajFeatGetStart(thys), ajFeatGetEnd(thys));

    ajSeqTrim(seq);

    ajDebug("ajFeatGetRemoteseq (%S) '%S' => '%S' %u %u..%u (%u)\n",
            thys->Remote, usa, baseusa, ilen,
            ajFeatGetStart(thys), ajFeatGetEnd(thys), ajSeqGetLen(seq));

    ajStrDel(&baseusa);

    return ajTrue;
}




/* @func ajFeatGetXrefs *******************************************************
**
** Returns all cross-references from a feature
**
** @param [r] thys [const AjPFeature] Feature
** @param [u] Pxreflist [AjPList*] List of sequence cross-reference objects
** @return [AjBool] True on success
** @@
******************************************************************************/

AjBool ajFeatGetXrefs(const AjPFeature thys, AjPList *Pxreflist)
{
    AjIList iter     = NULL;
    FeatPTagval item = NULL;
    AjPSeqXref  xref = NULL;
    ajint ipos;
    ajuint inum = 0;
    AjPList xreflist;

    if(!*Pxreflist)
        *Pxreflist = ajListNew();
    xreflist = *Pxreflist;

    if(thys->Tags)
    {
	iter = ajListIterNewread(thys->Tags);

	while(!ajListIterDone(iter))
	{
	    item = (FeatPTagval)ajListIterGet(iter);

	    if(ajStrMatchCaseC(item->Tag, "db_xref"))
	    {
                ipos = ajStrFindAnyK(item->Value, ':');
                if(ipos > 0) 
                {
                    inum++;
                    xref = ajSeqxrefNew();
                    ajStrAssignSubS(&xref->Db, item->Value, 0, ipos-1);
                    ajStrAssignSubS(&xref->Id, item->Value, ipos+1, -1);
                    xref->Start = ajFeatGetStart(thys)-1;
                    xref->End   = ajFeatGetEnd(thys)-1;
                    ajListPushAppend(xreflist, xref);
                    xref->Type = XREF_DBXREF;
                    xref = NULL;
                }
            }
	}
    }

    ajListIterDel(&iter);

    if(!inum)
        return ajFalse;

    return ajTrue;
}




/* @func ajFeatLocMark *******************************************************
**
** Returns a sequence entry converted to lower case where a feature
** location matches.
**
** @param [r] thys [const AjPFeature] Feature
** @param [r] table [const AjPFeattable] Feature table
** @param [u] Pseqstr [AjPStr*] Sequence to be marked in lower case
** @return [AjBool] true on success
** @@
******************************************************************************/

AjBool ajFeatLocMark(const AjPFeature thys, const AjPFeattable table,
                     AjPStr* Pseqstr)
{
    const AjPFeature gf;
    AjIList iter = NULL;

    iter = ajListIterNewread(table->Features);

    while(!ajListIterDone(iter))
    {
        gf = (const AjPFeature) ajListIterGet(iter);

        if(gf->Group == thys->Group)
        {
            if(gf->Flags & FEATFLAG_BETWEEN_SEQ)
                continue;

            if(gf->Flags & FEATFLAG_REMOTEID)
                continue;

            ajStrFmtLowerSub(Pseqstr,ajFeatGetStart(gf)-1,ajFeatGetEnd(gf)-1);
        }
    }

    ajListIterDel(&iter);
    
    return ajTrue;
}




/* @func ajFeatGetNoteC *******************************************************
**
** Finds a named note tag (with a * prefix)
**
** @param [r] thys [const AjPFeature] Feature object
** @param [r] name [const char*] Tag name
** @param [w] val [AjPStr*] Tag value (if found)
**
** @return [AjBool] ajTrue on success (feature tag found)
** @@
******************************************************************************/

AjBool ajFeatGetNoteC(const AjPFeature thys, const char* name, AjPStr* val)
{
    return ajFeatGetNoteCI(thys, name, 0, val);
}




/* @func ajFeatGetNoteCI ******************************************************
**
** Finds a named note tag (with a * prefix)
**
** @param [r] thys [const AjPFeature] Feature object
** @param [r] name [const char*] Tag name
** @param [r] count [ajint] Tag count: zero for any, 1 for first, 2 for second
** @param [w] val [AjPStr*] Tag value (if found)
**
** @return [AjBool] ajTrue on success (feature tag found)
** @@
******************************************************************************/

AjBool ajFeatGetNoteCI(const AjPFeature thys, const char* name, ajint count,
		      AjPStr* val)
{
    AjIList iter     = NULL;
    FeatPTagval item = NULL;
    ajint icount     = 0;
    ajuint ilen = strlen(name);

    /*ajDebug("ajFeatGetNoteCI '%s'\n", name);*/

    if(thys->Tags)
    {
	iter = ajListIterNewread(thys->Tags);

	while(!ajListIterDone(iter))
	{
	    item = (FeatPTagval)ajListIterGet(iter);
	    /*ajDebug("  try /%S=\"%S\"\n", item->Tag, item->Value);*/
	    if(ajFeattagIsNote(item->Tag))
	    {
		if(ajStrGetCharFirst(item->Value) == '*')
		{
		    /*ajDebug("  testing *name\n");*/
		    if(ajCharPrefixCaseC(ajStrGetPtr(item->Value)+1, name))
		    {
			icount++;
			/*ajDebug("  found [%d] '%S'\n", icount, name);*/

			if(icount >= count)
			{
			    if(ajStrGetLen(item->Value) > (ilen+1))
			    {
				if(ajStrGetCharPos(item->Value, ilen+1) != ' ')
				    return ajFalse;

				ajStrAssignC(val,
					     ajStrGetPtr(item->Value) +
					     ilen+2);
				    
			    }
			    else	/* no value */
				ajStrAssignClear(val);

			    ajListIterDel(&iter);
			    return ajTrue;
			}
		    }
		}
	    }
	}
    }

    ajStrDel(val);
    ajListIterDel(&iter);

    return ajFalse;
}




/* @func ajFeatGetNoteSI ******************************************************
**
** Finds a named note tag (with a * prefix)
**
** @param [r] thys [const AjPFeature] Feature object
** @param [r] name [const AjPStr] Tag name
** @param [r] count [ajint] Tag count: zero for any, 1 for first, 2 for second
** @param [w] val [AjPStr*] Tag value (if found)
**
** @return [AjBool] ajTrue on success (feature tag found)
** @@
******************************************************************************/

AjBool ajFeatGetNoteSI(const AjPFeature thys, const AjPStr name, ajint count,
                       AjPStr* val)
{
    return ajFeatGetNoteCI(thys, ajStrGetPtr(name), count, val);
}




/* @obsolete ajFeatGetNoteI
** @rename ajFeatGetNoteSI
*/
__deprecated AjBool ajFeatGetNoteI(const AjPFeature thys,
                                   const AjPStr name, ajint count,
                                   AjPStr* val)
{
    return ajFeatGetNoteSI(thys, name, count, val);
}




/* @func ajFeatGetNoteS ********************************************************
**
** Finds a named note tag (with a * prefix)
**
** @param [r] thys [const AjPFeature] Feature object
** @param [r] name [const AjPStr] Tag name
** @param [w] val [AjPStr*] Tag value (if found)
**
** @return [AjBool] ajTrue on success (feature tag found)
** @@
******************************************************************************/

AjBool ajFeatGetNoteS(const AjPFeature thys, const AjPStr name, AjPStr* val)
{
    return ajFeatGetNoteSI(thys, name, 0, val);
}




/* @obsolete ajFeatGetNote
** @rename ajFeatGetNoteS
*/

__deprecated AjBool ajFeatGetNote(const AjPFeature thys, const AjPStr name,
                                  AjPStr* val)
{
    return ajFeatGetNoteS(thys, name, val);
}




/* @func ajFeatGetTagC ********************************************************
**
** Returns the nth value of a named feature tag.
**
** If not found as a tag, also searches for a named note
**
** @param [r] thys [const AjPFeature] Feature object
** @param [r] tname [const char*] Tag name
** @param [r] num [ajint] Tag number
** @param [w] Pval [AjPStr*] Tag value (if found)
**
** @return [AjBool] ajTrue on success (feature tag found)
** @@
******************************************************************************/

AjBool ajFeatGetTagC(const AjPFeature thys, const char* tname, ajint num,
		    AjPStr* Pval)
{
    AjIList iter     = NULL;
    FeatPTagval item = NULL;
    ajint inum       = 0;
    AjBool isnote;
    ajint noteposcolon=0;
    ajint noteposvalue=0;

    ajDebug("ajFeatGetTagC '%s'\n", tname);
    isnote = ajCharMatchC(tname, "note");
    noteposcolon = strlen(tname) + 1;
    noteposvalue = strlen(tname) + 3;

    if(thys->Tags)
    {
	iter = ajListIterNewread(thys->Tags);
	while(!ajListIterDone(iter))
	{
	    item = (FeatPTagval)ajListIterGet(iter);
            ajDebug("testing '%S'\n", item->Tag);

	    if(ajStrMatchCaseC(item->Tag, tname))
	    {
		inum++;
                ajDebug("test1 inum %d\n", inum);

		if(num == inum)
		{
                    ajDebug("++match1\n");
		    ajStrAssignS(Pval, item->Value);
		    ajListIterDel(&iter);
		    return ajTrue;
		}
	    }
	    else if(!isnote &&
		    ajStrMatchCaseC(item->Tag, "note") &&
		    ajStrGetCharFirst(item->Value) == '*' &&
		    ajCharPrefixCaseC(ajStrGetPtr(item->Value)+1, tname) &&
		    ajStrGetCharPos(item->Value, noteposcolon) == ':')
	    {
		inum++;
                ajDebug("test2 inum %d\n", inum);

		if(num == inum)
		{
                    ajDebug("++match2 from %d\n", noteposvalue);
		    ajStrAssignSubS(Pval, item->Value, noteposvalue, -1);
		    ajListIterDel(&iter);
		    return ajTrue;
		}
	    }
	}
    }

    ajDebug("No match to '%s'\n", tname);

    ajStrDel(Pval);
    ajListIterDel(&iter);

    return ajFalse;
}




/* @func ajFeatGetTagS *********************************************************
**
** Returns the nth value of a named feature tag.
**
** If not found as a tag, also searches for a named note
**
** @param [r] thys [const AjPFeature] Feature object
** @param [r] name [const AjPStr] Tag name
** @param [r] num [ajint] Tag number
** @param [w] val [AjPStr*] Tag value (if found)
**
** @return [AjBool] ajTrue on success (feature tag found)
** @@
******************************************************************************/

AjBool ajFeatGetTagS(const AjPFeature thys, const AjPStr name, ajint num,
                     AjPStr* val)
{
    AjIList iter     = NULL;
    FeatPTagval item = NULL;
    ajint inum       = 0;
    AjBool isnote;
    ajint noteposcolon=0;
    ajint noteposvalue=0;

    isnote = ajStrMatchC(name, "note");
    noteposcolon = ajStrGetLen(name) + 1;
    noteposvalue = ajStrGetLen(name) + 3;

    if(thys->Tags)
    {
	iter = ajListIterNewread(thys->Tags);

	while(!ajListIterDone(iter))
	{
	    item = (FeatPTagval)ajListIterGet(iter);

	    if(ajStrMatchCaseS(item->Tag, name))
	    {
		inum++;

		if(num == inum)
		{
		    ajStrAssignS(val, item->Value);
		    ajListIterDel(&iter);
		    return ajTrue;
		}
	    }
	    else if(!isnote &&
		    ajStrMatchCaseC(item->Tag, "note") &&
		    ajStrGetCharFirst(item->Value) == '*' &&
		    ajCharPrefixCaseS(ajStrGetPtr(item->Value)+1, name) &&
		    ajStrGetCharPos(item->Value, noteposcolon) == ':')
	    {
		inum++;

		if(num == inum)
		{
		    ajStrAssignSubS(val, item->Value, noteposvalue, -1);
		    ajListIterDel(&iter);
		    return ajTrue;
		}
	    }
	}
    }

    ajStrDel(val);
    ajListIterDel(&iter);

    return ajFalse;
}




/* @func ajFeatGetType ********************************************************
**
** Returns the type (key) of a feature object. This is a copy of the
** pointer to the type, and is still owned by the feature
** and is not to be destroyed.
**
** @param [r] thys [const AjPFeature] Feature object
**
** @return [const AjPStr] Feature type, read only
** @@
******************************************************************************/

const AjPStr ajFeatGetType(const AjPFeature thys)
{
    if(thys->Protein)
	return featTableTypeExternal(thys->Type, FeatTypeTableProtein);
    else
	return featTableTypeExternal(thys->Type, FeatTypeTableDna);
}




/* @func ajFeatGetSource*******************************************************
**
** Returns the source name of a feature object.
**
** @param [r] thys [const AjPFeature] Feature object
**
** @return [const AjPStr] Feature source name
** @@
******************************************************************************/

const AjPStr ajFeatGetSource(const AjPFeature thys)
{
    return thys->Source;
}




/* @func ajFeatGetStart *******************************************************
**
** Returns the start position of a feature object.
**
** @param [r] thys [const AjPFeature] Feature object
**
** @return [ajuint] Feature start position
** @@
******************************************************************************/

ajuint ajFeatGetStart(const AjPFeature thys)
{
    if((thys->Flags & FEATFLAG_START_TWO) &&
       (thys->Start2 < thys->Start))
        return thys->Start2;

    return thys->Start;
}




/* @func ajFeatGetStrand ******************************************************
**
** Returns the strand of a feature object.
**
** @param [r] thys [const AjPFeature] Feature object
**
** @return [char] Feature strand code
** @@
******************************************************************************/

char ajFeatGetStrand(const AjPFeature thys)
{
    if(thys->Strand == '-')
	return '-';
    else
	return '+';
}




/* @func ajFeatGetEnd *********************************************************
**
** Returns the end position of a feature object.
**
** @param [r] thys [const AjPFeature] Feature object
**
** @return [ajuint] Feature end position
** @@
******************************************************************************/

ajuint ajFeatGetEnd(const AjPFeature thys)
{
    if((thys->Flags & FEATFLAG_END_TWO) &&
       (thys->End2 < thys->End))
        return thys->End2;

    return thys->End;
}




/* @func ajFeatGetLength ******************************************************
**
** Returns the sequence length of a feature object.
**
** @param [r] thys [const AjPFeature] Feature object
**
** @return [ajuint] Feature length
** @@
******************************************************************************/

ajuint ajFeatGetLength(const AjPFeature thys)
{
    return thys->End - thys->Start + 1;
}




/* @func ajFeatGetScore *******************************************************
**
** Returns the score of a feature object.
**
** @param [r] thys [const AjPFeature] Feature object
**
** @return [float] Feature score
** @@
******************************************************************************/

float ajFeatGetScore(const AjPFeature thys)
{
    return thys->Score;
}




/* @func ajFeatGetForward *****************************************************
**
** Returns the direction of a feature object.
**
** @param [r] thys [const AjPFeature] Feature object
**
** @return [AjBool] ajTrue for a forward direction, ajFalse for reverse
** @@
******************************************************************************/

AjBool ajFeatGetForward(const AjPFeature thys)
{
    if(ajSysCastItoc(thys->Strand) != '-')
	return ajTrue;

    return ajFalse;
}




/* @func ajFeatGetFrame *******************************************************
**
** Returns the reading frame of a feature object.
**
** @param [r] thys [const AjPFeature] Feature object
**
** @return [ajint] Feature reading frame (zero for undefined)
** @@
******************************************************************************/

ajint ajFeatGetFrame(const AjPFeature thys)
{
    return thys->Frame;
}




/* @func ajFeatGetTranslation **************************************************
**
** Returns translation tag value
**
** @param [r] thys [const AjPFeature] Feature
** @param [w] Ptrans [AjPStr*] Translations
**
** @return [AjBool] True if translation tag was found
** @@
******************************************************************************/

AjBool ajFeatGetTranslation(const AjPFeature thys, AjPStr *Ptrans)
{
    return ajFeatGetTagC(thys, "translation", 1, Ptrans);
}




/*========================================================================
======================= NEW FUNCTIONS ====================================
========================================================================*/




/* @func ajFeatTest ***********************************************************
**
** Temporary testing function for feature handling
** to be deleted when all is working happily
**
** @return [void]
** @@
******************************************************************************/

void ajFeatTest(void)
{
    AjPFeattable table = NULL;
    AjPStr desc        = NULL;
    AjPStr source      = NULL;
    AjPStr type        = NULL;
    AjPFeature ft      = NULL;

    featInit();
    table = ajFeattableNew(NULL);

    ajStrAssignC(&source, "testft");
    ajStrAssignC(&type, "misc_feature");
    ajStrAssignC(&desc, "Just testing");

    ft = ajFeatNew(table, source, type, 5, 7, (float)1.23, '+', 0);
    ajFeatSetDesc(ft, desc);

    ajStrAssignC(&desc, "Testing again");
    ft = ajFeatNew(table, source, type, 9, 19, (float)4.56, '-', 3);
    ajFeatSetDesc(ft, desc);

    ajFeattableTrace(table);

    ajFeattableDel(&table);
    ajStrDel(&desc);
    ajStrDel(&source);
    ajStrDel(&type);

    return;
}




/* @funcstatic featInit *******************************************************
**
** Initialises everything needed for feature handling
**
** @return [void]
** @@
******************************************************************************/

static void featInit(void)
{
    if(FeatInitDone)
	return;

    FeatInitDone = ajTrue;

    if(!FeatTypeTableDna)
    {
	FeatTypeTableDna = ajTablestrNewCaseLen(200);
	FeatTagsTableDna = ajTablestrNewCaseLen(200);
	featVocabRead("emboss",FeatTypeTableDna, FeatTagsTableDna);
    }

    if(!FeatTypeTableProtein)
    {
	FeatTypeTableProtein = ajTablestrNewCaseLen(200);
	FeatTagsTableProtein = ajTablestrNewCaseLen(200);
	featVocabRead("protein", FeatTypeTableProtein, FeatTagsTableProtein);
    }

    /*ajDebug("Tables internal (Dna, Prot) Type: %x %x Tags: %x %x\n",
	    FeatTypeTableDna, FeatTypeTableProtein,
	    FeatTagsTableDna, FeatTagsTableProtein);*/


    return;
}




/* @funcstatic featVocabRead **************************************************
**
** Reads the possible feature types (keys) and tags (qualifiers)
** from files.
**
** @param [r] name [const char*] Feature type ("emboss", "protein", or external
**                               feature types "swiss", "gff", "embl", etc.
** @param [w] pTypeTable [AjPTable] Feature type table
** @param [w] pTagsTable [AjPTable] Feature tags table
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool featVocabRead(const char* name,
			    AjPTable pTypeTable, AjPTable pTagsTable)
{
    AjPStr TypeFName = NULL;
    AjPStr TagsFName = NULL;

    /* First read in the list of all possible tags */
    
    ajFmtPrintS(&TagsFName, "Etags.%s", name);
    ajDebug("featVocabRead '%S' typetable %x tagstable %x\n",
	    TagsFName, pTypeTable, pTagsTable);
    
    if(!featVocabReadTags(TagsFName, pTagsTable, ajTrue))
	return ajFalse;

    /*ajDebug("Trace tagstable '%S'", TagsFName);*/
    /*ajTablestrTrace(pTagsTable);*/

  /* Efeatures file
  ** format: featuretype
  **            M/mandatorytag
  **             /tag
  **             /tag
  **
  ** All tags must be defined in the Etags file (read earlier into pTagsTable)
  */

    ajFmtPrintS(&TypeFName, "Efeatures.%s", name);
    /*ajDebug("Trying to open %S...\n",TypeFName);*/
    if(!featVocabReadTypes(TypeFName, pTypeTable, pTagsTable, ajTrue))
	return ajFalse;

    /*ajDebug("Trace typetable '%S'", TypeFName);*/
    /*ajTablestrTrace(pTypeTable);*/

    ajStrDel(&TagsFName);
    ajStrDel(&TypeFName);

    return ajTrue;
}




/* @funcstatic featVocabReadTags **********************************************
**
** Reads the possible feature tags (qualifiers)
** from files.
**
** @param [r] fname [const AjPStr] Feature tags definitions datafile name
** @param [w] pTagsTable [AjPTable] Feature tags table
** @param [r] recursion [AjBool] If true process any include lines
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool featVocabReadTags(const AjPStr fname, AjPTable pTagsTable,
				       AjBool recursion)
{
    AjPFile TagsFile = NULL;
    AjPStr TagsFName = NULL;
    AjPStr line      = NULL;
    AjPStr tagname   = NULL;		/* stored in table */
    AjPStr tagstr    = NULL;		/* stored in table */
    AjPStr defname   = NULL;		/* stored in table */
    AjPStr tagtype   = NULL;
    AjPStr token     = NULL;
    AjPStr rest      = NULL;
    ajint linecount = 0;
    ajint tagscount = 0;
    ajint numtype   = -1;
    ajint i;

    const char* TagType[] =
    {
	"QTEXT",			/* quoted text */
	"QWORD",			/* quoted text, no space */
	"BTEXT",			/* bracketed text */
	"TEXT",				/* unquoted, no white space */
	"VOID",				/* no value */
	"LIMITED",			/* limited vocabulary */
	"QLIMITED",		 /* limited vocabulary, with quotes */
	"SPECIAL",			/* special formatting */
	"QSPECIAL",		 /* special formatting, with quotes */
	"TAGVAL",		       /* /type=value for swissprot */
	NULL
    };
    
    TagsFile = ajDatafileNewInNameS(fname);

    if(!TagsFile)
    {
	ajErr("Unable to read feature tags data file '%S'\n", fname);
	return ajFalse;
    }
    
    tagscount = ajTableGetLength(pTagsTable);
    linecount = 0;

    while(ajReadlineTrim(TagsFile,&line))
    {
	linecount++;
	ajStrRemoveWhiteExcess(&line);

	if(ajStrPrefixC(line, "#"))
	{
	    if(recursion && ajStrPrefixCaseC(line, "#include "))
	    {
		ajStrCutStart(&line, 9);
		ajStrExtractWord(line, &rest, &TagsFName);
		ajStrDel(&rest);
		if(!featVocabReadTags(TagsFName, pTagsTable, ajTrue))
		{
		    ajStrDel(&line);
		    ajStrDel(&TagsFName);
		    return ajFalse;
		}
		tagscount = ajTableGetLength(pTagsTable);
	    }

	    if(recursion && ajStrPrefixCaseC(line, "#includeonly "))
	    {
		ajStrCutStart(&line, 13);
		ajStrExtractWord(line, &rest, &TagsFName);
		ajStrDel(&rest);

		if(!featVocabReadTags(TagsFName, pTagsTable, ajFalse))
		{
		    ajStrDel(&line);
		    ajStrDel(&TagsFName);
		    return ajFalse;
		}
		tagscount = ajTableGetLength(pTagsTable);
	    }
	}
	else if(ajStrGetLen(line)) /* skip comments */
	{
	    ajStrDel(&tagname);		/* create a new tag */

	    if(featTagName(line, &tagname, &tagtype, &featTmpStr))
	    {
		numtype = -1;
		
		for(i=0; TagType[i]; i++)
		    if(!ajStrCmpC(tagtype,TagType[i]))
		    {
			numtype = i;
			break;
		    }

		if(numtype < 0)
		{
		    /*ajDebug("Bad feature tag type '%S' in %F line %d\n",
			    tagtype, TagsFile, linecount);*/
		    ajErr("Bad feature tag type '%S' in %F line %d",
			  tagtype, TagsFile, linecount);
		    break;
		}
		ajStrDel(&tagtype);
		
		tagscount++;

		if(tagscount == 1) /* save first tag as the default */
		{
		    tagstr = NULL;
		    ajStrAssignClear(&defname);
		    ajStrAssignS(&tagstr, tagname);

		    if(ajTablePut (pTagsTable, defname, tagstr))
			ajErr("%S duplicate tag for '%S'",
			      fname, defname);

		    tagstr  = NULL;
		    defname = NULL;
		}

		tagstr = NULL;
		ajFmtPrintS(&tagstr, "%s;", TagType[numtype]);
		
		/*
		 ** Controlled vocabulary :
		 ** read the list of valid values
		 */
		
		if(ajCharMatchCaseC(TagType[numtype], "LIMITED") ||
		   ajCharMatchCaseC(TagType[numtype], "QLIMITED"))
		{
		    ajStrTokenAssignC(&featVocabSplit, featTmpStr, "\", \t");

		    while(ajStrTokenNextParse(&featVocabSplit, &token))
		    {
			ajFmtPrintAppS(&tagstr, "%S;", token);
		    }

		    ajStrDelStatic(&featTmpStr);
		    ajStrTokenDel(&featVocabSplit);
		}
		
		if(ajTablePut(pTagsTable, tagname, tagstr))
		    ajErr("%S duplicate tag for '%S'", fname, tagname);

		tagstr  = NULL;
		tagname = NULL;
	    }
	    else
	    {
		/*ajDebug("** line format bad **\n%S", line);*/
	    }
	}
    }

    ajFileClose(&TagsFile);
    ajStrDel(&TagsFName);
    ajStrDel(&line);
    ajStrDel(&token);
    ajStrDel(&tagname);
    ajStrDel(&tagtype);

    /*
       ajTableTrace(pTagsTable);
       ajTablestrPrint(pTagsTable);
       */
    
    return ajTrue;
}




/* @funcstatic featVocabReadTypes *********************************************
**
** Reads the possible feature types (keys)
** from files.
**
** @param [r] fname [const AjPStr] Feature type definitions datafile name
** @param [w] pTypeTable [AjPTable] Feature type table
** @param [r] pTagsTable [const AjPTable] Feature tags table
** @param [r] recursion [AjBool] If true process any include lines
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool featVocabReadTypes(const AjPStr fname, AjPTable pTypeTable,
				 const AjPTable pTagsTable,
				 AjBool recursion)
{
    AjPStr TypeFName = NULL;
    AjPFile TypeFile = NULL;
    AjPStr line      = NULL;
    AjPStr intids    = NULL;
    AjPStr tag       = NULL;
    AjPStr req       = NULL;
    AjPStr localname = NULL;
    AjPStr sofaname  = NULL;
    AjPStr type      = NULL;
    AjPStr defname   = NULL;
    AjPStr defid     = NULL;
    AjPStr typtagstr = NULL;
    AjPStr savetype  = NULL;
    AjPStr firstid   = NULL;
    AjPStr token     = NULL;
    AjPStr rest      = NULL;
    AjPStr editstr   = NULL;
    AjPStr modtype   = NULL;
    AjPStr tablestr  = NULL;
    const AjPStr sofaid    = NULL;
    const AjPStr storetype  = NULL;

    ajint typecount = 0;
    ajint filetypecount = 0;

    char reqchar;
    char modchar;
    ajint ipos;
    AjBool ismodtype = ajFalse;
    AjPStr* Ptyptagstr = NULL;
    AjBool taginternal = ajFalse;
    AjBool newdefid    = ajFalse;

    TypeFile = ajDatafileNewInNameS(fname);

    if(!TypeFile)
    {
	ajErr("Unable to read feature types data file '%S'\n", fname);
	return ajFalse;
    }
    
    filetypecount = 0;
    typecount = ajTableGetLength(pTypeTable);

    while(ajReadlineTrim(TypeFile,&line))
    {
	ajStrRemoveWhiteExcess(&line);

	if(ajStrPrefixC(line,"#")) /* if a comment skip it */
	{
	    if(recursion && ajStrPrefixCaseC(line, "#include "))
	    {
		ajStrCutStart(&line, 9);
		ajStrExtractFirst(line, &rest, &TypeFName);
		ajStrDel(&rest);

		if(!featVocabReadTypes(TypeFName, pTypeTable, pTagsTable,
				       ajTrue))
		{
		    ajStrDel(&line);
		    ajStrDel(&TypeFName);
                    ajStrDel(&firstid);
                    ajStrDel(&savetype);
                    ajStrDel(&sofaname);
                    return ajFalse;
		}

		typecount = ajTableGetLength(pTypeTable);
	    }

	    if(recursion && ajStrPrefixCaseC(line, "#includeonly "))
	    {
		ajStrCutStart(&line, 13);
		ajStrExtractFirst(line, &rest, &TypeFName);
		ajStrDel(&rest);

                if(!featVocabReadTypes(TypeFName, pTypeTable, pTagsTable,
				       ajFalse))
		{
		    ajStrDel(&line);
		    ajStrDel(&TypeFName);
                    ajStrDel(&firstid);
                    ajStrDel(&savetype);
                    ajStrDel(&sofaname);
		    return ajFalse;

		}
		typecount = ajTableGetLength(pTypeTable);
	    }
	}
	else if(ajStrGetLen(line))
	{
	    if(featFeatType(line, &type, &intids, &tag, &req))
	    {
		if(ajStrGetLen(type))	/* new feature type */
		{
		    if(!ismodtype &&
		       ajStrGetLen(savetype)) /* save previous type and tags */
		    {
			ajDebug("%S %d saved '%S' as '%S'\n",
                                fname, typecount, savetype, typtagstr);
			ajDebug("+type %S='%S'\n",
                                savetype, typtagstr);
/*
			tablestr = ajTablePut(pTypeTable,
					      savetype, typtagstr);
*/
			tablestr = ajTableFetch(pTypeTable, savetype);

                        if(tablestr)
                        {
                            if(recursion)
                            {
                                ajDebug("ERROR:%S duplicate type %S='%S' "
                                        "already defined as '%S'\n",
                                        fname, savetype, typtagstr, tablestr);
                                ajErr("%S duplicate type %S='%S' "
                                      "already defined as '%S'",
                                      fname, savetype, typtagstr, tablestr);
                            }
                            else
                            {
                                ajDebug("found savetype '%S' with value '%S'\n",
                                       savetype, tablestr);
                            }

                            ajStrDel(&typtagstr);
                            ajStrDel(&savetype);
                        }
                        else
                        {
                            ajTablePut(pTypeTable, savetype, typtagstr);
                            typtagstr = NULL;
                            savetype = NULL;
                        }
		    }

		    if(ajStrMatchC(req, "="))
                    {
			ismodtype = ajTrue;
			ajStrAssignS(&modtype, type);

			if(ajStrMatchC(type, ".")) /* replace default */
			    ajStrAssignClear(&type);

			ajStrAssignS(&localname, type);
			ajStrAssignS(&sofaname, intids);
			tablestr = ajTableFetch(pTypeTable, localname);

			if(tablestr)
			{
			    ajDebug("%S duplicate alias type "
				  "%S='%S' already defined as '%S'\n",
				  fname, localname,
				  sofaname, tablestr);
			    ajStrDel(&localname);
			}
                        else
                        {
                            ajTablePut(pTypeTable, localname,
                                       sofaname);
                            localname = NULL;
                            sofaname = NULL;
			}

			Ptyptagstr = ajTablestrFetchmod(pTypeTable, intids);

                        if(!Ptyptagstr)
                            ajWarn("%S undefined internal ID '%S'",
                                   fname, intids);

			ipos = ajStrFindAnyK(*Ptyptagstr, ';');

			if(ipos >= 0)
			    ajStrCutStart(Ptyptagstr, ipos);
			else
			    ajStrInsertK(Ptyptagstr, 0, ';');

			ajStrInsertS(Ptyptagstr, 0, type);
                        ajStrDel(&savetype);
			savetype  = type;
                        type = NULL;
			typtagstr = *Ptyptagstr;
                        taginternal = ajTrue;
		    }
		    else
		    {
			typecount++;
			filetypecount++;

			if(typecount == 1)  /* type saved as "" default */
			{
			    defname   = NULL;
			    defid = NULL;
			    ajStrAssignClear(&defname);
			    ajStrAssignS(&defid, type);
                            newdefid = ajTrue;
			    tablestr = ajTableFetch(pTypeTable, defname);

			    if(tablestr)
                            {
                                ajDebug("ERROR:%S duplicate type %S='%S' "
                                      "already defined as '%S'\n",
				      fname, defname, typtagstr, tablestr);
				ajErr("%S duplicate type %S='%S' "
                                      "already defined as '%S'",
				      fname, defname, typtagstr, tablestr);
                                ajStrDel(&typtagstr);
                                ajStrDel(&defname);
                            }
                            else
                            {
                                ajTablePut(pTypeTable, defname, defid);
                                defname = NULL;
                            }
                            
			}

			ismodtype = ajFalse;		    

			/*
			** set up new feature type and type-tag strings
			** ready to save the details
			*/

			typtagstr = ajStrNewResC(";", 256);
                        taginternal = ajFalse;

			if(ajStrGetLen(intids))
			{
			    sofaid = ajStrParseWhite(intids);
			    ajStrAssignS(&firstid, sofaid);
			    storetype = type;

			    while(sofaid)
			    {
                                if(newdefid)
                                {
                                    ajStrAssignS(&defid, sofaid);
                                    newdefid = ajFalse;
                                    defid = NULL;
                                }

                                ajDebug("+type %B storetype:'%S' "
					"firstid:'%S'\n",
					ajStrMatchCaseS(storetype, firstid),
					storetype, firstid);

				if(!ajStrMatchCaseS(storetype, firstid))
				{
				    ajStrAssignS(&localname, storetype);
				    ajStrAssignS(&sofaname, firstid);
				    ajDebug("%S sofaid "
                                            "'%S' = '%S'\n",
                                            fname,localname, sofaname);
				    ajDebug("+type (alias) %S='%S'\n",
                                            localname, sofaname);
				    tablestr = ajTableFetch(pTypeTable,
                                                            localname);
				    if(tablestr)
                                    {
                                        if(recursion)
                                        {
                                            ajDebug("ERROR:%S"
                                                    " duplicate localname "
                                                    "type "
                                                  "%S='%S' already defined "
                                                    "as '%S'\n",
                                                  fname, localname,
                                                  sofaname, tablestr);
                                            ajErr("%S duplicate localname type "
                                                  "%S='%S' already defined "
                                                  "as '%S'",
                                                  fname, localname,
                                                  sofaname, tablestr);
                                        }
                                        ajStrDel(&localname);
                                        ajStrDel(&sofaname);
                                    }
                                    else
                                    {
                                        ajTablePut(pTypeTable,
                                                   localname, sofaname);
                                        localname = NULL;
                                        sofaname = NULL;
                                        storetype  = sofaid;
                                    }
                                    
				}

				sofaid = ajStrParseWhite(NULL);
			    }

			    ajStrInsertS(&typtagstr, 0, type);
			    ajStrAssignClear(&type);
                            ajStrDel(&savetype);
			    savetype  = firstid;
			    firstid = NULL;
			    sofaid = NULL;
			    ajStrAssignClear(&intids);
			}
			else
			{
                            ajStrDel(&savetype);
			    savetype  = type;
			    type      = NULL;
			    ajStrAssignClear(&intids);
			}
		    }
		}
		else			/* tag name */
		{
		    if(!ajTableFetch(pTagsTable, tag))
		    {
			if(ismodtype)
			    featWarn("%S: tag %S (feature %S) "
				     "not in Etags file",
				     fname, tag, modtype);
			else
			    featWarn("%S: tag %S (feature %S) "
				     "not in Etags file",
				     fname, tag, savetype);
		    }

		    modchar = ajStrGetCharFirst(req);
		    reqchar = ajStrGetCharLast(req);

		    if(modchar == '-')
		    {
			ajFmtPrintS(&editstr, ";%S;", tag);
			ajStrExchangeSC(&typtagstr, editstr, ";");

			if(!modtype)
			    featWarn("%S: delete tag '%S' for %S not modified",
					 fname, tag, modtype);
		    }
		    else
		    {
			if(reqchar == 'M' ||
			   reqchar == 'm')
			    ajFmtPrintAppS(&typtagstr, "*");

			ajFmtPrintAppS(&typtagstr, "%S;", tag);

			if(modchar == '+')
			{
			    if(!modtype)
				featWarn("%S: new tag '%S' for %S not modified",
					 fname, tag, modtype);
			}
		    }
		}		
	    }
	}
    }
    
    if(filetypecount > 0)		/* save the last feature type */
    {
	ajDebug("+type (final) %S='%S'\n",
                savetype, typtagstr);
	tablestr = ajTableFetch(pTypeTable, savetype);
	if(tablestr)
        {
            if(recursion)
            {
                ajDebug("ERROR: %S: duplicate type %S='%S' already "
                        "defined as '%S'\n",
                        fname, savetype, typtagstr, tablestr); 
                ajErr("%S: duplicate type %S='%S' already defined as '%S'",
                      fname, savetype, typtagstr, tablestr);
            }
            ajStrDel(&typtagstr);
            ajStrDel(&savetype);
        }
        else 
        {
            ajTablePut(pTypeTable, savetype, typtagstr);
            typtagstr = NULL;
            savetype = NULL;
        }
    }
    
    ajFileClose(&TypeFile);
   
    ajDebug("Total types...: %d\n", typecount);

    ajStrDel(&line);
    ajStrDel(&token);
    ajStrDel(&intids);


    /*
       ajTableTrace(pTypeTable);
       ajTablestrPrint(pTypeTable);
    */

    ajStrDelStatic(&featTmpStr);
    ajStrDel(&TypeFName);
    ajStrDel(&req);
    ajStrDel(&tag);
    ajStrDel(&type);
    ajStrDel(&intids);
    ajStrDel(&modtype);
    ajStrDel(&localname);
    ajStrDel(&sofaname);
    ajStrDel(&firstid);
    ajStrDel(&savetype);
    
    return ajTrue;    
}




/* @funcstatic featVocabInitEmbl **********************************************
**
** Initialises feature table internals for EMBL format
**
** @return [AjBool] ajTrue on success
******************************************************************************/

static AjBool featVocabInitEmbl(void)
{
    if(!FeatTypeTableEmbl)
    {
	FeatTypeTableEmbl = ajTablestrNewCaseLen(200);
	FeatTagsTableEmbl = ajTablestrNewCaseLen(200);
	return featVocabRead("embl", FeatTypeTableEmbl, FeatTagsTableEmbl);
    }

    return ajTrue;
}




/* @funcstatic featVocabInitGff ***********************************************
**
** Initialises feature table internals for GFF format
**
** @return [AjBool] ajTrue on success
******************************************************************************/

static AjBool featVocabInitGff(void)
{
    AjBool ret1;
    AjBool ret2;

    if(!FeatTypeTableGff)
    {
	FeatTypeTableGff = ajTablestrNewCaseLen(200);
	FeatTagsTableGff = ajTablestrNewCaseLen(200);
	ret1 = featVocabInitGffprotein();
	ret2 = featVocabRead("gff", FeatTypeTableGff, FeatTagsTableGff);

	if(ret1 && ret2)
	    return ajTrue;

	return ajFalse;
    }

    return ajTrue;
}




/* @funcstatic featVocabInitGff3 ***********************************************
**
** Initialises feature table internals for GFF format
**
** @return [AjBool] ajTrue on success
******************************************************************************/

static AjBool featVocabInitGff3(void)
{
    AjBool ret1;
    AjBool ret2;

    if(!featVocabInitGff())
        return ajFalse;

    if(!FeatTypeTableGff3)
    {
	FeatTypeTableGff3 = ajTablestrNewCaseLen(200);
	FeatTagsTableGff3 = ajTablestrNewCaseLen(200);
	ret1 = featVocabInitGff3protein();
	ret2 = featVocabRead("gff3", FeatTypeTableGff3, FeatTagsTableGff3);

	if(ret1 && ret2)
	    return ajTrue;

	return ajFalse;
    }

    return ajTrue;
}




/* @funcstatic featVocabInitGffprotein ****************************************
**
** Initialises feature table internals for GFF format
**
** @return [AjBool] ajTrue on success
******************************************************************************/

static AjBool featVocabInitGffprotein(void)
{
    if(!FeatTypeTableGffprotein)
    {
	FeatTypeTableGffprotein = ajTablestrNewCaseLen(200);
	FeatTagsTableGffprotein = ajTablestrNewCaseLen(200);

	return featVocabRead("gffprotein",
			     FeatTypeTableGffprotein,
			     FeatTagsTableGffprotein);
    }

    return ajTrue;
}




/* @funcstatic featVocabInitGff3protein ****************************************
**
** Initialises feature table internals for GFF3 format
**
** @return [AjBool] ajTrue on success
******************************************************************************/

static AjBool featVocabInitGff3protein(void)
{
    if(!FeatTypeTableGff3protein)
    {
	FeatTypeTableGff3protein = ajTablestrNewCaseLen(200);
	FeatTagsTableGff3protein = ajTablestrNewCaseLen(200);

	return featVocabRead("gff3protein",
			     FeatTypeTableGff3protein,
			     FeatTagsTableGff3protein);
    }

    return ajTrue;
}




/* @funcstatic featVocabInitPir ***********************************************
**
** Initialises feature table internals for PIR format
**
** @return [AjBool] ajTrue on success
******************************************************************************/

static AjBool featVocabInitPir(void)
{
    if(!FeatTypeTablePir)
    {
	FeatTypeTablePir = ajTablestrNewCaseLen(200);
	FeatTagsTablePir = ajTablestrNewCaseLen(200);

	return featVocabRead("pir", FeatTypeTablePir, FeatTagsTablePir);
    }

    return ajTrue;
}




/* @funcstatic featVocabInitRefseqp *******************************************
**
** Initialises feature table internals for refSeq protein format
**
** @return [AjBool] ajTrue on success
******************************************************************************/

static AjBool featVocabInitRefseqp(void)
{
    if(!FeatTypeTableRefseqp)
    {
	FeatTypeTableRefseqp = ajTablestrNewCaseLen(50);
	FeatTagsTableRefseqp = ajTablestrNewCaseLen(5);

	return featVocabRead("refseqp", FeatTypeTableRefseqp,
                             FeatTagsTableRefseqp);
    }

    return ajTrue;
}




/* @funcstatic featVocabInitSwiss *********************************************
**
** Initialises feature table internals for SwissProt format
**
** @return [AjBool] ajTrue on success
******************************************************************************/

static AjBool featVocabInitSwiss(void)
{
    if(!FeatTypeTableSwiss)
    {
	FeatTypeTableSwiss = ajTablestrNewCaseLen(50);
	FeatTagsTableSwiss = ajTablestrNewCaseLen(5);

	return featVocabRead("swiss", FeatTypeTableSwiss, FeatTagsTableSwiss);
    }

    return ajTrue;
}




/* @func ajFeatSetDescApp *****************************************************
**
** Sets the description for a feature
**
** @param [u] thys [AjPFeature] Feature
** @param [r] desc [const AjPStr] Feature description (simple text)
** @return [void]
** @@
******************************************************************************/

void ajFeatSetDescApp(AjPFeature thys, const AjPStr desc)
{
    FeatPTagval tv        = NULL;

    if(!featTagNote)
	ajStrAssignC(&featTagNote, "note");

    tv = featTagval(thys, featTagNote);

    if(tv)
    {
	ajStrAppendC(&tv->Value, ", ");
	ajStrAppendS(&tv->Value, desc);
    }
    else
	ajFeatTagSet(thys, featTagNote, desc);

    return;
}




/* @func ajFeatSetDesc ********************************************************
**
** Sets the description for a feature
**
** @param [u] thys [AjPFeature] Feature
** @param [r] desc [const AjPStr] Feature description (simple text)
** @return [void]
** @@
******************************************************************************/

void ajFeatSetDesc(AjPFeature thys, const AjPStr desc)
{
    ajFeatTagSetC(thys, "note", desc);

    return;
}




/* @func ajFeatSetScore *******************************************************
**
** Sets the score for a feature
**
** @param [u] thys [AjPFeature] Feature
** @param [r] score [float] Score value
** @return [void]
** @@
******************************************************************************/

void ajFeatSetScore(AjPFeature thys, float score)
{
    thys->Score = score;
    return;
}




/* @func ajFeatSetStrand ******************************************************
**
** Sets the strand for a feature
**
** @param [u] thys [AjPFeature] Feature
** @param [r] rev [AjBool] True if reverse strand
** @return [void]
** @@
******************************************************************************/

void ajFeatSetStrand(AjPFeature thys, AjBool rev)
{
    if(rev)
	thys->Strand = '-';
    else
	thys->Strand = '+';

    return;
}




/* @func ajFeattabInSetType **************************************************
**
** Sets the type for feature input
**
** @param [u] thys [AjPFeattabIn] Feature input object
** @param [r] type [const AjPStr] Feature type "nucleotide" "protein"
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeattabInSetType(AjPFeattabIn thys, const AjPStr type)
{
    return ajFeattabInSetTypeC(thys, ajStrGetPtr(type));
}




/* @func ajFeattabInSetTypeC **************************************************
**
** Sets the type for feature input
**
** @param [u] thys [AjPFeattabIn] Feature input object
** @param [r] type [const char*] Feature type "nucleotide" "protein"
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeattabInSetTypeC(AjPFeattabIn thys, const char* type)
{
    ajint i = 0;

    if(!*type)
	return ajTrue;

    for(i=0; featInTypes[i].Name; i++)
    {
	if(ajCharMatchCaseC(featInTypes[i].Name, type))
	{
	    if(featInTypes[i].Value)
		ajStrAssignC(&thys->Type, featInTypes[i].Value);

	    return ajTrue;
	}
	i++;
    }

    ajErr("Unrecognized feature input type '%s'", type);

    return ajFalse;
}




/* @func ajFeattabOutSetType **************************************************
**
** Sets the type for feature output
**
** @param [u] thys [AjPFeattabOut] Feature output object
** @param [r] type [const AjPStr] Feature type "nucleotide" "protein"
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeattabOutSetType(AjPFeattabOut thys, const AjPStr type)
{
    return ajFeattabOutSetTypeC(thys, ajStrGetPtr(type));
}




/* @func ajFeattabOutSetTypeC *************************************************
**
** Sets the type for feature output
**
** @param [u] thys [AjPFeattabOut] Feature output object
** @param [r] type [const char*] Feature type "nucleotide" "protein"
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeattabOutSetTypeC(AjPFeattabOut thys, const char* type)
{
    ajint i = 0;

    if(!*type)
	return ajTrue;

    for(i=0; featOutTypes[i].Name; i++)
    {
	if(ajCharMatchCaseC(featOutTypes[i].Name, type))
	{
	    if(featInTypes[i].Value)
		ajStrAssignC(&thys->Type, featOutTypes[i].Value);
	    return ajTrue;
	}
    }

    ajErr("Unrecognised feature output type '%s'", type);

    return ajFalse;
}




/* @func ajFeatTagSetC ********************************************************
**
** Sets a feature tag value
**
** @param [u] thys [AjPFeature] Feature
** @param [r] tag [const char*] Feature tag
** @param [r] value [const AjPStr] Feature tag value
** @return [AjBool] ajTrue is value was valid
**                  ajFalse if it was "corrected"
** @@
******************************************************************************/

AjBool ajFeatTagSetC(AjPFeature thys, const char* tag, const AjPStr value)
{
    AjPStr tmptag = NULL;
    AjBool ret = ajFalse;

    ajStrAssignC(&tmptag, tag);

    ret = ajFeatTagSet(thys, tmptag, value);
    ajStrDel(&tmptag);

    return ret;
}




/* @func ajFeatTagSet *********************************************************
**
** Sets a feature tag value
**
** @param [u] thys [AjPFeature] Feature
** @param [r] tag [const AjPStr] Feature tag
** @param [r] value [const AjPStr] Feature tag value
** @return [AjBool] ajTrue is value was valid
**                  ajFalse if it was "corrected"
** @@
******************************************************************************/

AjBool ajFeatTagSet(AjPFeature thys, const AjPStr tag,const  AjPStr value)
{
    AjBool ret     = ajTrue;
    FeatPTagval tv = NULL;
    AjBool knowntag = ajTrue;

    static AjPStr oldvalue = NULL;

    const AjPStr tmptag        = NULL;		/* this comes from AjPTable */
                                           /* so please, please don't delete */
    const char* cp;
    
    /* ajDebug("ajFeatTagSet '%S' '%S' Prot: %B\n",
       tag, value, thys->Protein); */
    
    featInit();
    
    if(thys->Protein)
    {
	tmptag = featTableTag(tag, FeatTagsTableProtein, &knowntag);
	featTagFmt(tmptag,  FeatTagsTableProtein, &featFmtTmp);
    }
    else
    {
	tmptag = featTableTag(tag, FeatTagsTableDna, &knowntag);
	featTagFmt(tmptag,  FeatTagsTableDna, &featFmtTmp);
    }
    
    ajStrAssignS(&featValTmp, value);
    ajStrAssignS(&featTagTmp, tmptag);
    
    cp = ajStrGetPtr(featFmtTmp);

    switch(CASE2(cp[0], cp[1]))
    {
        case CASE2('L','I') :			/* limited */
            /* ajDebug("case limited\n"); */
            break;
        case CASE2('Q', 'L') :		/* limited, escape quotes */
            /* ajDebug("case qlimited\n"); */
            break;
        case CASE2('Q', 'S') :		/* special regexp, quoted */
            /* ajDebug("case qspecial\n"); */
            if(!featTagSpecial(&featValTmp, tmptag))
            {
                ret = ajFalse;
                featTagSetDefault(thys, tmptag, value, &featTagTmp,
                                  &featValTmp);
            }
            break;
        case CASE2('S','P') :	/* special regexp */
            /* ajDebug("case special\n"); */
            if(!featTagSpecial(&featValTmp, tmptag))
            {
                ret = ajFalse;
                featTagSetDefault(thys, tmptag, value, &featTagTmp,
                                  &featValTmp);
            }
            break;
        case CASE2('T','E') :	     /* no space, no quotes, wrap at margin */
            /* ajDebug("case text\n"); */
            break;
        case CASE2('V','O') :		      /* no value, so an error here */
            /*ajDebug("case void\n");*/
            break;
        case CASE2('Q','T') :		    /* escape quotes, wrap at space */
            /* ajDebug("case qtext\n"); */
            break;
        case CASE2('Q','W') :		    /* escape quotes, remove space */
            /* ajDebug("case qword\n"); */
            break;
        default:
            featWarn("Unknown internal feature tag type '%S' for '%S'",
                     featFmtTmp, tmptag);
    }
    
    tv = featTagval(thys, featTagTmp);

    if(tv)				/* replace current value */
    {
	ajStrAssignS(&oldvalue, tv->Value);
	ajStrAssignS(&tv->Value, featValTmp);
	/*ajDebug("...replaced old value '%S'\n", oldvalue);*/
	return ret;
    }

    /* new tag-value */
    tv = featTagvalNew(thys, featTagTmp, featValTmp);
    ajListPushAppend(thys->Tags, tv);
    /* ajDebug("...new tag-value\n"); */

    return ret;
}




/* @func ajFeatTagAddCC *******************************************************
**
** Sets a feature tag value, creating a new feature tag even if one
** already exists.
**
** @param [u] thys [AjPFeature] Feature
** @param [r] tag [const char*] Feature tag
** @param [r] value [const char*] Feature tag value
** @return [AjBool] ajTrue if value was valid
**                  ajFalse if it was bad and was "corrected"
** @@
******************************************************************************/

AjBool ajFeatTagAddCC(AjPFeature thys, const char* tag, const char* value)
{
    AjBool ret;
    AjPStr tagstr = NULL;
    AjPStr valstr = NULL;

    ajStrAssignC(&tagstr, tag);
    ajStrAssignC(&valstr, value);

    ret = ajFeatTagAdd(thys, tagstr, valstr);
    ajStrDel(&tagstr);
    ajStrDel(&valstr);

    return ret;
}




/* @func ajFeatTagAddC ********************************************************
**
** Sets a feature tag value, creating a new feature tag even if one
** already exists.
**
** @param [u] thys [AjPFeature] Feature
** @param [r] tag [const char*] Feature tag
** @param [r] value [const AjPStr] Feature tag value
** @return [AjBool] ajTrue if value was valid
**                  ajFalse if it was bad and was "corrected"
** @@
******************************************************************************/

AjBool  ajFeatTagAddC(AjPFeature thys, const char* tag, const AjPStr value)
{
    static AjPStr tagstr = NULL;

    ajStrAssignC(&tagstr, tag);

    return ajFeatTagAdd(thys, tagstr, value);
}




/* @func ajFeatTagAdd *********************************************************
**
** Sets a feature tag value, creating a new feature tag even if one
** already exists.
**
** @param [u] thys [AjPFeature] Feature
** @param [r] tag [const AjPStr] Feature tag
** @param [r] value [const AjPStr] Feature tag value
** @return [AjBool] ajTrue if value was valid
**                  ajFalse if it was bad and was "corrected"
** @@
******************************************************************************/

AjBool ajFeatTagAdd(AjPFeature thys, const AjPStr tag, const AjPStr value)
{  
    AjBool ret     = ajTrue;
    FeatPTagval tv = NULL;
    const AjPStr tmptag  = NULL;		/* this comes from AjPTable */
                                       /* so please, please don't delete */
    AjBool knowntag = ajTrue;
    const char* cp;
    
    ajDebug("ajFeatTagAdd '%S' '%S' Prot: %B\n",
	    tag, value, thys->Protein);
    
    featInit();
    
    if(thys->Protein)
    {
	tmptag = featTableTag(tag, FeatTagsTableProtein, &knowntag);
	featTagFmt(tmptag,  FeatTagsTableProtein, &featFmtTmp);
    }
    else
    {
	tmptag = featTableTag(tag, FeatTagsTableDna, &knowntag);
	featTagFmt(tmptag, FeatTagsTableDna, &featFmtTmp);
    }
    
    /* ajDebug("tag: '%S' format: '%S'\n", tmptag, featFmtTmp); */
    ajStrAssignS(&featValTmp2, value);
    ajStrAssignS(&featTagTmp2, tmptag);
    
    cp = ajStrGetPtr(featFmtTmp);

    switch(CASE2(cp[0], cp[1]))
    {
        case CASE2('L','I') :			/* limited */
            /* ajDebug("case limited\n"); */
            break;
        case CASE2('Q', 'L') :		/* limited, escape quotes */
            /* ajDebug("case qlimited\n"); */
            break;
        case CASE2('Q', 'S') :		/* special regexp, quoted */
            /* ajDebug("case qspecial\n"); */
            if(!featTagSpecial(&featValTmp2, tmptag))
            {
                ret = ajFalse;
                featTagSetDefault(thys, tmptag, value, &featTagTmp2,
                                  &featValTmp2);
            }
            break;
        case CASE2('S','P') :			/* special regexp */
            /* ajDebug("case special\n");*/
            if(!featTagSpecial(&featValTmp2, tmptag))
            {
                ret = ajFalse;
                featTagSetDefault(thys, tmptag, value, &featTagTmp2,
                                  &featValTmp2);
            }
            break;
        case CASE2('T','E') :	     /* no space, no quotes, wrap at margin */
            /* ajDebug("case text\n"); */
            break;
        case CASE2('V','O') :		      /* no value, so an error here */
            /* ajDebug("case void\n"); */
            break;
        case CASE2('Q','T') :		    /* escape quotes, wrap at space */
            /* ajDebug("case qtext\n"); */
            if(!knowntag && tag)
                ajFmtPrintS(&featValTmp2, "*%S: %S", tag, value);
            /*ajDebug("ajFeatTagAdd qtext knowntag %B tag '%S' Tmp '%S' '%S'\n",
              knowntag, tag, featTagTmp2, featValTmp2);*/
            break;
        case CASE2('Q','W') :		    /* escape quotes, remove space */
            /* ajDebug("case qword\n"); */
            break;
        default:
            featWarn("Unknown internal feature tag type '%S' for '%S'",
                     featFmtTmp, tmptag);
    }
    
    tv = featTagvalNew(thys, featTagTmp2, featValTmp2);

    ajListPushAppend(thys->Tags, tv);
    /* ajDebug("...new tag-value\n"); */
    
    return ret;
}




/* @funcstatic featTagSetDefault **********************************************
**
** Sets a feature tag value, using the default feature tag
**
** @param [u] thys [AjPFeature] Feature
** @param [r] tag [const AjPStr] Feature tag
** @param [r] value [const AjPStr] feature tag value
** @param [w] Pdeftag [AjPStr*] Default tag
** @param [w] Pdefval [AjPStr*] Default tag value as "*tag: value"
** @return [void]
** @@
******************************************************************************/

static void featTagSetDefault(AjPFeature thys,
			      const AjPStr tag, const AjPStr value,
			      AjPStr* Pdeftag, AjPStr* Pdefval)
{
    featInit();

    if(thys->Protein)
	featTagSetDefaultDna(tag, value, Pdeftag, Pdefval);
    else
	featTagSetDefaultProt(tag, value, Pdeftag, Pdefval);

    return;
}




/* @funcstatic featTagSetDefaultDna *******************************************
**
** Sets a feature tag value, using the default DNA feature tag
**
** @param [r] tag [const AjPStr] Feature tag
** @param [r] value [const AjPStr] feature tag value
** @param [w] Pdeftag [AjPStr*] Default tag
** @param [w] Pdefval [AjPStr*] Default tag value as "*tag: value"
** @return [void]
** @@
******************************************************************************/

static void featTagSetDefaultDna(const AjPStr tag, const AjPStr value,
				  AjPStr* Pdeftag, AjPStr* Pdefval)
{
    featInit();

    ajStrAssignS(Pdeftag, (AjPStr) ajTableFetch(FeatTagsTableDna, ajStrNew()));
    ajFmtPrintS(Pdefval, "*%S: %S", tag, value);

    return;
}




/* @funcstatic featTagSetDefaultProt ******************************************
**
** Sets a feature tag value, using the default protein feature tag
**
** @param [r] tag [const AjPStr] Feature tag
** @param [r] value [const AjPStr] feature tag value
** @param [w] Pdeftag [AjPStr*] Default tag
** @param [w] Pdefval [AjPStr*] Default tag value as "*tag: value"
** @return [void]
** @@
******************************************************************************/

static void featTagSetDefaultProt(const AjPStr tag, const AjPStr value,
				   AjPStr* Pdeftag, AjPStr* Pdefval)
{
    featInit();

    ajStrAssignS(Pdeftag,
		 (AjPStr) ajTableFetch(FeatTagsTableProtein, ajStrNew()));
    ajFmtPrintS(Pdefval, "*%S: %S", tag, value);

    return;
}




/* @func ajFeattableNew *******************************************************
**
** Constructor for a new (generic) feature table.
** Does not define the feature table type.
**
** @param [r] name [const AjPStr] Name for new feature table
**                                (or NULL for unnamed)
** @return [AjPFeattable] Pointer to a new (empty) feature table
** @category new [AjPFeattable] Constructor
** @@
******************************************************************************/

AjPFeattable ajFeattableNew(const AjPStr name )
{
    AjPFeattable thys = NULL;

    /* Allocate the object... */
    thys = featTableNewS(name) ;

    /*ajDebug("ajFeattableNew %x\n", thys);*/

    return thys;
}




/* @func ajFeattableNewRead ****************************************************
**
** Generic interface function for reading in features from a file
** given the file handle.
**
** @param  [u] ftin   [AjPFeattabIn]  Specifies the external source (file)
**                                     of the features to be read in
** @return [AjPFeattable] Pointer to a new feature table containing
**                        the features read in
** @category new [AjPFeattable] Reads in a feature set in a specified format
** @@
******************************************************************************/

AjPFeattable ajFeattableNewRead(AjPFeattabIn  ftin)
{
    AjPFilebuff  file ;
    ajint format ;

    AjPFeattable features = NULL ;
    AjBool result         = ajFalse ;

    if(!ftin)
	return NULL;

    file = ftin->Handle ;

    if(!file)
	return NULL;

    format = ftin->Format ;

    if(!format)
	return NULL;

    ajDebug("ajFeatRead format %d '%s' file %x type: '%S'\n",
	    format, featInFormat[format].Name, file, ftin->Type);

    if(!featInFormat[format].Used)
    {
	/* Calling funclist featInFormatDef() */
	if(!featInFormat[format].InitReg())
	{
	    ajDebug("Initialisation failed for %s\n",
		    featInFormat[format].Name);
	    ajErr("Initialisation failed for feature format %s",
		  featInFormat[format].Name);
	}

	featInFormat[format].Used = ajTrue;
    }

    features = ajFeattableNew(ftin->Seqname);

    /* Calling funclist featInFormatDef() */
    result = featInFormat[format].Read(features, file);

    if(result)
    {
	/* ajFeattableTrace(features); */
	ajDebug("ajFeatRead read %d features\n", ajFeattableGetSize(features));

	return features ;
    }
    else
	ajFeattableDel(&(features)) ;

    return NULL;
}





/* @obsolete ajFeatRead
** @rename ajFeattableNewRead
*/

__deprecated AjPFeattable ajFeatRead(AjPFeattabIn  ftin)
{
    return ajFeattableNewRead(ftin);
}




/* @func ajFeattableNewReadUfo *************************************************
**
** Parses a UFO, opens an input file, and reads a feature table
**
** @param [u] featin [AjPFeattabIn] Feature input object
** @param [r] ufo [const AjPStr] UFO feature spec
** @return [AjPFeattable] Feature table created, (or NULL if failed)
** @category new [AjPFeattable] Parses a UFO, opens an input file,
**                              and reads a feature table
** @@
******************************************************************************/

AjPFeattable ajFeattableNewReadUfo(AjPFeattabIn featin,
                                   const AjPStr ufo)
{
    AjPFeattable ret = NULL;
    
    AjBool fmtstat = ajFalse;	/* status returns from regex tests */
    AjBool filstat = ajFalse;	/* status returns from regex tests */
    ajint i;
    
    if(!featRegUfoFmt)
	featRegUfoFmt = ajRegCompC("^([A-Za-z0-9][A-Za-z0-9]+):+(.*)$");
    /* \1 format */
    /* \2 remainder */
    
    if(!featRegUfoFile)
	featRegUfoFile = ajRegCompC("^(([A-Za-z]:)?[^:]+)$");
    
    /*ajDebug("ajFeattableNewReadUfo UFO '%S'\n", ufo);*/
    
    ajStrAssignS(&featUfoTest, ufo);
    
    if(ajStrGetLen(ufo))
    {
	fmtstat = ajRegExec(featRegUfoFmt, featUfoTest);
	/*ajDebug("feat format regexp: %B\n", fmtstat);*/
    }
    
    if(fmtstat)
    {
	ajRegSubI(featRegUfoFmt, 1, &featin->Formatstr);
	ajStrAssignEmptyC(&featin->Formatstr,
		  featInFormat[0].Name); /* unknown */
	ajRegSubI(featRegUfoFmt, 2, &featUfoTest); /* trim off the format */
	/*ajDebug("found feat format %S\n", featin->Formatstr);*/

	if(!featFindInFormatS(featin->Formatstr, &featin->Format))
	    ajErr("unknown input feature table format '%S'\n"
		  " NO Features will be read in", featin->Formatstr);
    }
    else
    {
	/*ajDebug("no format specified in UFO");*/
    }
    
    featFormatSet(featin);
    
    filstat = ajRegExec(featRegUfoFile, featUfoTest);
    /*ajDebug("featRegUfoFile: %B\n", filstat);*/

    if(filstat)
	ajRegSubI(featRegUfoFile, 1, &featin->Filename);
    else
    {
	if(ajStrGetLen(featin->Seqname) && ajStrGetLen(featin->Formatstr))
	{
	    ajFmtPrintS(&featUfoTest, "%S.%S",
			featin->Seqname, featin->Formatstr);
	    ajStrAssignEmptyS(&featin->Filename, featUfoTest);
	    /*ajDebug("generate filename  '%S'\n", featin->Filename);*/
	}
	else
	{
	    /*ajDebug("unable to generate filename "
		    "Featin Seqname '%S' Formatstr '%S'\n",
		    featin->Seqname, featin->Formatstr);*/
	    return NULL;
	}
    }
    
    /* Open the file so that we can try to read it */
    
   /* ajDebug("trying to open '%S'\n", featin->Filename);*/
    featin->Handle = ajFilebuffNewNameS(featin->Filename);

    if(!featin->Handle)
	return NULL;
    /*ajDebug("after opening '%S'\n", featin->Filename);*/
    
    
    /* OKAY if we have a format specified try this and this ONLY */
    if(featin->Format)
	ret = ajFeattableNewRead(featin);
    /* else loop through all types and try to read features */
    else
	for(i=1;featInFormat[i].Name;i++)
	{
	    featin->Format = i;

	    ret = ajFeattableNewRead(featin);

	    if(ret)
		break;

	    /* Reset buffer to start */
	    ajFilebuffReset(featin->Handle);

	}

    ajFilebuffDel(&featin->Handle);
    
    return ret;
}




/* @obsolete ajFeatUfoRead
** @rename ajFeattableNewReadUfo
*/

__deprecated AjPFeattable ajFeatUfoRead(AjPFeattabIn featin,
		     const AjPStr ufo)
{

    return ajFeattableNewReadUfo (featin, ufo);
}




/* @func ajFeattableSetDefname *************************************************
**
** Provides a unique (for this program run) name for a feature table.
**
** @param [w] thys [AjPFeattable] Feature table
** @param [r] setname [const AjPStr] Name set by caller
** @return [void]
** @@
******************************************************************************/

void ajFeattableSetDefname(AjPFeattable thys, const AjPStr setname)
{
    if(ajStrGetLen(thys->Seqid))
    {
	ajDebug("ajFeattableSetDefname already has a name '%S'\n", thys->Seqid);
	return;
    }

    if(ajStrGetLen(setname))
	ajStrAssignS(&thys->Seqid, setname);

    ajDebug("ajFeattableSetDefname set to  '%S'\n", setname);

    return;
}




/* @obsolete ajFeatDefName
** @rename ajFeattableSetDefname
*/

__deprecated void ajFeatDefName(AjPFeattable thys, const AjPStr setname)
{
    ajFeattableSetDefname(thys, setname);
    return;
}




/* @func ajFeattableSetLength **************************************************
**
** Sets the length of a feature table with the length of the source sequence.
**
** This is needed to reverse the table correctly
**
** @param [u] thys [AjPFeattable] Feature table object
** @param [r] len [ajuint] Length
** @return [void]
** @@
**
******************************************************************************/

void ajFeattableSetLength(AjPFeattable thys, ajuint len)
{
    thys->Len = len;

    return;
}




/* @func ajFeattableSetNuc ****************************************************
**
** Sets the type of a feature table as nucleotide
**
** @param [u] thys [AjPFeattable] Feature table object
** @return [void]
** @@
**
******************************************************************************/

void ajFeattableSetNuc(AjPFeattable thys)
{
    ajStrAssignEmptyC(&thys->Type, "N");

    return;
}




/* @func ajFeattableSetProt ***************************************************
**
** Sets the type of a feature table as Protein
**
** @param [u] thys [AjPFeattable] Feature table object
** @return [void]
** @@
**
******************************************************************************/

void ajFeattableSetProt(AjPFeattable thys)
{
    ajStrAssignEmptyC(&thys->Type, "P");

    return;
}




/* @func ajFeattableSetRange **************************************************
**
** Set the begin and end range for a feature table
**
** @param [u] thys [AjPFeattable] Feature table object
** @param [r] fbegin [ajint] Begin position
** @param [r] fend   [ajint] End position
** @return [void]
******************************************************************************/

void ajFeattableSetRange(AjPFeattable thys, ajint fbegin, ajint fend)
{
    thys->Start = ajFeattablePosI(thys, 1, fbegin);
    thys->End   = ajFeattablePosII(thys->Len, thys->Start, fend);

    return;
}




/* @func ajFeattableReverse ***************************************************
**
** Reverse the features in a feature table by iterating through and
** reversing all positions and strands.
**
** @param [u] thys [AjPFeattable] Feature table object
** @return [void]
******************************************************************************/

void ajFeattableReverse(AjPFeattable  thys)
{
    AjIList    iter = NULL;
    AjPFeature gf   = NULL;

    if(ajFeattableIsProt(thys))
	return;

    iter = ajListIterNewread(thys->Features);

    while(!ajListIterDone(iter))
    {
	gf = ajListIterGet(iter);

	if(gf->Flags & FEATFLAG_REMOTEID ||
	   gf->Flags & FEATFLAG_LABEL)
	    continue;

	ajFeatReverse(gf, thys->Len) ;
    }

    ajListIterDel(&iter);

    return;
}




/* @func ajFeatReverse ********************************************************
**
** Reverse one feature by reversing all positions and strand.
**
** @param [u] thys [AjPFeature] Feature object
** @param [r] ilen [ajint] Sequence length
** @return [void]
******************************************************************************/

void ajFeatReverse(AjPFeature  thys, ajint ilen)
{
    ajint itmp;
    ajint saveflags;
    
    /*ajDebug("ajFeatReverse ilen %d %x '%c' %d..%d %d..%d\n",
	    ilen, thys->Flags, thys->Strand,
	    thys->Start, thys->End, thys->Start2, thys->End2);*/
    
    saveflags = thys->Flags;
    
    if(thys->Strand == '-')
	thys->Strand = '+';
    else
	thys->Strand = '-';
    
    itmp = thys->Start;

    if(thys->End)
	thys->Start = 1 + ilen - thys->End;
    else
	thys->Start = 0;
    
    if(itmp)
	thys->End = 1 + ilen - itmp;
    else
	thys->End = 0;
    
    itmp = thys->Start2;
    if(thys->End2)
	thys->Start2 = 1 + ilen - thys->End2;
    else
	thys->Start2 = 0;
    
    /* reverse the flags */
    
    if(saveflags & FEATFLAG_START_BEFORE_SEQ)
	thys->Flags |= FEATFLAG_END_AFTER_SEQ;
    else
	thys->Flags  &= ~FEATFLAG_END_AFTER_SEQ;

    if(saveflags & FEATFLAG_END_AFTER_SEQ)
	thys->Flags |=  FEATFLAG_START_BEFORE_SEQ;
    else
	thys->Flags &=  ~FEATFLAG_START_BEFORE_SEQ;

    if(saveflags & FEATFLAG_START_TWO)
	thys->Flags |=  FEATFLAG_END_TWO;
    else
	thys->Flags &=  ~FEATFLAG_END_TWO;

    if(saveflags & FEATFLAG_END_TWO)
	thys->Flags |=  FEATFLAG_START_TWO;
    else
	thys->Flags &=  ~FEATFLAG_START_TWO;

    if(saveflags & FEATFLAG_START_UNSURE)
	thys->Flags |=  FEATFLAG_END_UNSURE;
    else
	thys->Flags &=  ~FEATFLAG_END_UNSURE;

    if(saveflags & FEATFLAG_END_UNSURE)
	thys->Flags |=  FEATFLAG_START_UNSURE;
    else
	thys->Flags &=  ~FEATFLAG_START_UNSURE;
    
    if(itmp)
	thys->End2 = 1 + ilen - itmp;
    else
	thys->End2 = 0;
    
    /* thys->Frame is rather hard to guess ... leave alone for now */
    
    thys->Frame = 0;		/* set to unknown */
    
    /*ajDebug("     Reversed %x '%c' %d..%d %d..%d\n",
	    thys->Flags, thys->Strand,
	    thys->Start, thys->End, thys->Start2, thys->End2);*/

    return;    
}




/* @func ajFeattableNewDna ****************************************************
**
** Constructor for a new DNA feature table
**
** @param [r] name [const AjPStr] Name for new feature table
**                                (or NULL for unnamed)
** @return [AjPFeattable] Pointer to a new (empty) feature table
** @exception  'Mem_Failed' from memory allocation
** @@
**
******************************************************************************/

AjPFeattable ajFeattableNewDna(const AjPStr name)
{
    AjPFeattable thys = NULL;

    /* Allocate the object... */
    thys = featTableNewS(name);

    ajStrAssignC(&thys->Type, "N");

    /*ajDebug("ajFeattableNewDna %x\n", thys);*/

    return thys;
}




/* @func ajFeattableNewSeq ****************************************************
**
** Constructor for a new feature table for an existing sequence.
** The feature table type is determined by the sequence type.
**
** @param [r] seq [const AjPSeq] Sequence object to provide the name and type
** @return [AjPFeattable] Pointer to a new (empty) feature table
** @exception  'Mem_Failed' from memory allocation
** @@
**
******************************************************************************/

AjPFeattable ajFeattableNewSeq(const AjPSeq seq)
{
    AjPFeattable thys = NULL;

    /* Allocate the object... */
    /*  AJNEW0(thys);  deleted by AJB */

    if(ajSeqIsProt(seq))
	thys = ajFeattableNewProt(ajSeqGetNameS(seq));
    else
	thys = ajFeattableNewDna(ajSeqGetNameS(seq));

    thys->Len = ajSeqGetLen(seq);

    return thys;
}




/* @func ajFeattableNewProt ***************************************************
**
** Constructor for a new protein feature table
**
** @param [r] name [const AjPStr] Name for new feature table
**                                (or NULL for unnamed)
** @return [AjPFeattable] Pointer to a new (empty) feature table
** @exception  'Mem_Failed' from memory allocation
** @@
**
******************************************************************************/

AjPFeattable ajFeattableNewProt(const AjPStr name)
{
    AjPFeattable thys = NULL;

    /* Allocate the object... */
    thys = featTableNewS(name);

    ajStrAssignC(&thys->Type, "P");

    /*ajDebug("ajFeattableNewProt %x\n", thys);*/

    return thys;
}




/* @funcstatic featTagvalNew **************************************************
**
** Constructor for a feature tag-value pair
**
** @param [r]   thys   [const AjPFeature]   Feature
** @param [r]   tag    [const AjPStr]   Tag name
** @param [r]   value  [const AjPStr]   Tag value
** @return [FeatPTagval] New tag-value pair object
** @@
******************************************************************************/

static FeatPTagval featTagvalNew(const AjPFeature thys,
			         const AjPStr tag, const AjPStr value)
{
    FeatPTagval ret;

    if(thys->Protein)
	ret = featTagvalNewProt(tag, value);
    else
	ret = featTagvalNewDna(tag, value);

    return ret;
}




/* @funcstatic featTagvalNewDna ***********************************************
**
** Constructor for a feature tag-value pair
**
** @param [r]   tag    [const AjPStr]   Tag name
** @param [r]   value  [const AjPStr]   Tag value
** @return [FeatPTagval] New tag-value pair object
** @@
******************************************************************************/

static FeatPTagval featTagvalNewDna(const AjPStr tag, const AjPStr value)
{
    FeatPTagval ret;
    const AjPStr tmptag = NULL;	     /* from AjPTable, don't delete */
    AjBool knowntag = ajTrue;

    featInit();

    AJNEW0(ret);

    tmptag = featTableTag(tag, FeatTagsTableDna, &knowntag);

    ajStrAssignS(&ret->Tag, tmptag);
    ajStrAssignS(&ret->Value, value);

    return ret;
}




/* @funcstatic featTagvalNewProt **********************************************
**
** Constructor for a protein feature tag-value pair
**
** @param [r]   tag    [const AjPStr]   Tag name
** @param [r]   value  [const AjPStr]   Tag value
** @return [FeatPTagval] New tag-value pair object
** @@
******************************************************************************/

static FeatPTagval featTagvalNewProt(const AjPStr tag, const AjPStr value)
{
    FeatPTagval ret;
    const AjPStr tmptag = NULL;	     /* from AjPTable, don't delete */
    AjBool knowntag = ajTrue;

    featInit();

    AJNEW0(ret);

    tmptag = featTableTag(tag, FeatTagsTableProtein, &knowntag);

    ajStrAssignS(&ret->Tag, tmptag);
    ajStrAssignS(&ret->Value, value);

    return ret;
}




/* @funcstatic featTagval *****************************************************
**
** Checks for the existence of a defined tag for a feature.
**
** @param [r]   thys [const AjPFeature]  Feature object
** @param [r]   tag  [const AjPStr]      Tag name
** @return [FeatPTagval] Returns the tag-value pair if found,
**                       NULL if not found.
** @@
******************************************************************************/

static FeatPTagval featTagval(const AjPFeature thys, const AjPStr tag)
{
    AjIList iter    = NULL;
    FeatPTagval ret = NULL;
    FeatPTagval tv  = NULL;

    iter = ajListIterNewread(thys->Tags);

    while(!ajListIterDone(iter))
    {
	tv = ajListIterGet(iter);

	if(ajStrMatchCaseS(tv->Tag, tag)) 
	{
	    /* ajDebug("featTagval '%S' found value '%S'\n",
	       tag, tv->Value); */
	    ret = tv;
	    break;
	}
    }

    ajListIterDel(&iter);

    /*
       if(!ret)
       ajDebug("featTagval '%S' not found\n", tag);
       */


    return ret;
}




/* @func ajFeattableNewFtable **************************************************
**
** Makes a copy of a feature table.
**
** For cases where we need a copy we can safely change and/or delete.
**
** @param [r]   orig  [const AjPFeattable]  Original feature table
** @return [AjPFeattable] Feature table copy of the original
** @@
******************************************************************************/

AjPFeattable ajFeattableNewFtable(const AjPFeattable orig)
{
    AjPFeattable ret = NULL;
    AjIList iter;
    AjPFeature featorig;
    AjPFeature feat = NULL;

    if(!orig)
	return NULL;

    ret = featTableNew();

    ajStrAssignS(&ret->Seqid, orig->Seqid);
    ajStrAssignS(&ret->Type, orig->Type);
    ret->DefFormat = orig->DefFormat;
    ret->Start     = orig->Start;
    ret->End       = orig->End;
    ret->Len       = orig->Len;
    ret->Groups    = orig->Groups;

    iter = ajListIterNewread(orig->Features);

    while(!ajListIterDone(iter))
    {
	featorig = ajListIterGet(iter);
	feat = ajFeatNewFeat(featorig);
	ajFeattableAdd(ret, feat);
    }

    ajListIterDel(&iter);

    return ret;
}




/* @obsolete ajFeattableCopy
** @rename ajFeattableNewFtable
*/

__deprecated AjPFeattable ajFeattableCopy(const AjPFeattable orig)
{
    return ajFeattableNewFtable(orig);
}




/* @func ajFeattableNewFtableLimit *********************************************
**
** Makes a copy of a feature table using only a limited number of features.
**
** For cases where we need a copy we can safely change and/or delete.
**
** @param [r]   orig  [const AjPFeattable]  Original feature table
** @param [r]   limit  [ajint]  Limit to number of features copied
** @return [AjPFeattable] Feature table copy of the original
** @@
******************************************************************************/

AjPFeattable ajFeattableNewFtableLimit(const AjPFeattable orig, ajint limit)
{
    AjPFeattable ret = NULL;
    AjIList iter;
    AjPFeature featorig;
    AjPFeature feat = NULL;
    ajint ift = 0;

    if(!orig)
	return NULL;

    ret = featTableNew();

    ajStrAssignS(&ret->Seqid, orig->Seqid);
    ajStrAssignS(&ret->Type, orig->Type);
    ret->DefFormat = orig->DefFormat;
    ret->Start     = orig->Start;
    ret->End       = orig->End;
    ret->Len       = orig->Len;
    ret->Groups    = orig->Groups;

    iter = ajListIterNewread(orig->Features);

    while(!ajListIterDone(iter) && (ift++ < limit))
    {
	featorig = ajListIterGet(iter);
	feat = ajFeatNewFeat(featorig);
	ajFeattableAdd(ret, feat);
    }

    ajListIterDel(&iter);

    return ret;
}




/* @obsolete ajFeattableCopyLimit
** @rename ajFeattableNewFtableLimit
*/

__deprecated AjPFeattable ajFeattableCopyLimit(const AjPFeattable orig, ajint limit)
{
    return ajFeattableNewFtableLimit(orig, limit);
}




/* @func ajFeatNewFeat *********************************************************
**
** Makes a copy of a feature.
**
** For cases where we need a copy we can safely change and/or delete.
**
** @param [r]   orig  [const AjPFeature]  Original feature
** @return [AjPFeature] Feature  copy of the original
** @category new [AjPFeature] Copy constructor
** @@
******************************************************************************/

AjPFeature ajFeatNewFeat(const AjPFeature orig)
{
    AjPFeature ret;
    AjIList iter;
    FeatPTagval tvorig;

    ret = featFeatureNew();

    ajStrAssignS(&ret->Source, orig->Source);
    ajStrAssignS(&ret->Type, orig->Type);
    ajStrAssignS(&ret->Remote, orig->Remote);
    ajStrAssignS(&ret->Label, orig->Label);

    ret->Protein = orig->Protein;
    ret->Start   = orig->Start;
    ret->End     = orig->End;
    ret->Start2  = orig->Start2;
    ret->End2    = orig->End2;
    ret->Score   = orig->Score;
    ret->Strand  = orig->Strand;
    ret->Frame   = orig->Frame;
    ret->Flags   = orig->Flags;
    ret->Group   = orig->Group;
    ret->Exon    = orig->Exon;

    iter = ajListIterNewread(orig->Tags);

    while(!ajListIterDone(iter))
    {
	tvorig = ajListIterGet(iter);
	ajFeatTagAdd(ret, tvorig->Tag, tvorig->Value);
    }

    ajListIterDel(&iter);

    return ret;
}




/* @obsolete ajFeatCopy
** @rename ajFeatNewFeat
*/

__deprecated AjPFeature ajFeatCopy(const AjPFeature orig)
{
    return ajFeatNewFeat(orig);
}




/* @func ajFeatTrace **********************************************************
**
** Traces (to the debug file) a feature object
**
** @param [r]   thys  [const AjPFeature]  Feature
** @return [void]
** @@
******************************************************************************/

void ajFeatTrace(const AjPFeature thys)
{
    AjPStr flagstr = NULL;

    ajDebug("  Source: '%S'\n", thys->Source);
    ajDebug("  Type: '%S' protein: %B\n", thys->Type, thys->Protein);
    ajDebug("  Location: %d..%d\n", thys->Start, thys->End);
    ajDebug("  Strand: '%c'\n", thys->Strand);
    ajDebug("  Frame: '%d'\n", thys->Frame);
    ajDebug("  Score: '%f'\n", thys->Score);
    ajDebug("  Start2: '%d'\n", thys->Start2);
    ajDebug("  End2: '%d'\n", thys->Start2);
    ajDebug("  RemoteId: '%S'\n", thys->Remote);
    ajDebug("  Label: '%S'\n", thys->Label);

    if(thys->Flags) 
    {
        if(thys->Flags & FEATFLAG_START_BEFORE_SEQ)
            ajStrAppendC(&flagstr, "start_before ");

        if(thys->Flags & FEATFLAG_END_AFTER_SEQ)
            ajStrAppendC(&flagstr, "end_after ");

        if(thys->Flags & FEATFLAG_CHILD)
            ajStrAppendC(&flagstr, "exon ");

        if(thys->Flags & FEATFLAG_BETWEEN_SEQ)
            ajStrAppendC(&flagstr, "between ");

        if(thys->Flags & FEATFLAG_START_TWO)
            ajStrAppendC(&flagstr, "start2 ");

        if(thys->Flags & FEATFLAG_END_TWO)
            ajStrAppendC(&flagstr, "end2 ");

        if(thys->Flags & FEATFLAG_POINT)
            ajStrAppendC(&flagstr, "point ");

        if(thys->Flags & FEATFLAG_COMPLEMENT_MAIN)
            ajStrAppendC(&flagstr, "overall_complement ");

        if(thys->Flags & FEATFLAG_MULTIPLE)
            ajStrAppendC(&flagstr, "multiple ");

        if(thys->Flags & FEATFLAG_ORDER)
            ajStrAppendC(&flagstr, "order ");

        if(thys->Flags & FEATFLAG_REMOTEID)
            ajStrAppendC(&flagstr, "remote_id ");

        if(thys->Flags & FEATFLAG_LABEL)
            ajStrAppendC(&flagstr, "label ");

        if(thys->Flags & FEATFLAG_START_UNSURE)
            ajStrAppendC(&flagstr, "start_unsure ");

        if(thys->Flags & FEATFLAG_END_UNSURE)
            ajStrAppendC(&flagstr, "end_unsure ");

        ajStrCutEnd(&flagstr, 1);
        ajDebug("  Flags: '%x' (%S)\n", thys->Flags, flagstr);
    }
    else
    {
        ajDebug("  Flags: '%x'\n", thys->Flags);
    }

    ajDebug("  Group: '%u' exon: %u\n", thys->Group, thys->Exon);

    ajFeatTagTrace(thys);
    ajStrDel(&flagstr);

    return;
}




/* @func ajFeatTagTrace *******************************************************
**
** Traces (to the debug file) the tag-value pairs of a feature object
**
** @param [r]   thys  [const AjPFeature]  Feature
** @return [void]
** @@
******************************************************************************/

void ajFeatTagTrace(const AjPFeature thys)
{
    AjIList iter;
    ajint i = 0;
    FeatPTagval tv = NULL;

    iter = ajListIterNewread(thys->Tags);

    while(!ajListIterDone(iter))
    {
	tv = ajListIterGet(iter);
	ajDebug(" %3d  %S : '%S'\n", ++i, tv->Tag, tv->Value);
    }

    ajListIterDel(&iter);

    return;
}




/* @func ajFeatTagIter ********************************************************
**
** Returns an iterator over all feature tag-value pairs
**
** @param [r]   thys  [const AjPFeature]  Feature
** @return [AjIList] List iterator
** @@
******************************************************************************/

AjIList ajFeatTagIter(const AjPFeature thys)
{
    return ajListIterNewread(thys->Tags);
}




/* @func ajFeatTagval *********************************************************
**
** Returns the tag-value pairs of a feature object
**
** @param [u]  iter  [AjIList] List iterator from ajFeatTagIter
** @param [w] Ptagnam [AjPStr*] Tag name
** @param [w] Ptagval [AjPStr*] Tag val
** @return [AjBool] ajTrue if another tag-value pair was returned
** @@
******************************************************************************/

AjBool ajFeatTagval(AjIList iter, AjPStr* Ptagnam, AjPStr* Ptagval)
{
    FeatPTagval tv = NULL;

    tv = ajListIterGet(iter);

    if(!tv)
	return ajFalse;

    ajStrAssignS(Ptagnam, tv->Tag);
    ajStrAssignS(Ptagval, tv->Value);

    return ajTrue;
}




/* @func ajFeattableTrace *****************************************************
**
** Traces (to the debug file) a complete feature table
**
** @param [r]   thys  [const AjPFeattable]  Feature table
** @return [void]
** @@
******************************************************************************/

void ajFeattableTrace(const AjPFeattable thys)
{
    AjIList iter  = NULL;
    AjPFeature ft = NULL;
    ajint i = 0;

    ajDebug("== ajFeattableTrace Start ==\n");

    if(!thys)
    {
	ajDebug("NULL table\n");
	return;
    }

    ajDebug("  Seqid: '%S'\n", thys->Seqid);

    iter = ajListIterNew(thys->Features);

    while(!ajListIterDone(iter))
    {
	ft = ajListIterGet(iter);
	ajDebug("Features[%d]\n", ++i);
	ajFeatTrace(ft);
    }

    ajListIterDel(&iter);

    ajDebug("== ajFeattableTrace Done ==\n");
    return;
}




/* @func ajFeatTypeNuc ****************************************************
**
** Given a feature type name,
** returns the valid feature type for the internal DNA feature table
**
** @param [r]   type  [const AjPStr] Type name
** @return [const AjPStr] Valid feature type
** @@
******************************************************************************/

const AjPStr ajFeatTypeNuc(const AjPStr type)
{
    featInit();

    return featTableTypeExternal(type, FeatTypeTableDna);
}




/* @func ajFeatTypeProt ****************************************************
**
** Given a feature type name,
** returns the valid feature type for the internal protein feature table
**
** @param [r]   type  [const AjPStr] Type name
** @return [const AjPStr] Valid feature type
** @@
******************************************************************************/

const AjPStr ajFeatTypeProt(const AjPStr type)
{
    featInit();

    return featTableTypeExternal(type, FeatTypeTableProtein);
}




/* @funcstatic featTypeDna ****************************************************
**
** Given a feature type name,
** returns the valid feature type for the internal DNA feature table
**
** @param [r]   type  [const AjPStr] Type name
** @return [const AjPStr] Valid feature type
** @@
******************************************************************************/

static const AjPStr featTypeDna(const AjPStr type)
{
    featInit();

    return featTableTypeInternal(type, FeatTypeTableDna);
}




/* @funcstatic featTypeProt ***************************************************
**
** Given a feature type name,
** returns the valid feature type for the internal protein feature table
**
** @param [r]   type  [const AjPStr] Type name
** @return [const AjPStr] Valid feature type
** @@
******************************************************************************/

static const AjPStr featTypeProt(const AjPStr type)
{
    featInit();

    return featTableTypeInternal(type, FeatTypeTableProtein);
}




/* @funcstatic featTypeDnaLimit ***********************************************
**
** Given a feature type name,
** returns the valid feature type for the internal DNA feature table
** following alternative names until a unique name is found
**
** @param [r]   type  [const AjPStr] Type name
** @return [const AjPStr] Valid feature type
** @@
******************************************************************************/

static const AjPStr featTypeDnaLimit(const AjPStr type)
{
    featInit();

    return featTableTypeInternalLimit(type, FeatTypeTableDna);
}




/* @funcstatic featTypeProtLimit **********************************************
**
** Given a feature type name,
** returns the valid feature type for the internal protein feature table
** following alternative names until a unique name is found
**
** @param [r]   type  [const AjPStr] Type name
** @return [const AjPStr] Valid feature type
** @@
******************************************************************************/

static const AjPStr featTypeProtLimit(const AjPStr type)
{
    featInit();

    return featTableTypeInternalLimit(type, FeatTypeTableProtein);
}




/* @funcstatic featTypeTestDnaWild *********************************************
**
** Given a feature type name,
** returns the valid feature type for the internal DNA feature table
** following alternative names until a matching wildcard name is found
**
** @param [r]   type  [const AjPStr] Type name
** @param [r]   str   [const AjPStr] Wildcard name
** @return [AjBool] True if a match is found
** @@
******************************************************************************/

static AjBool featTypeTestDnaWild(const AjPStr type, const AjPStr str)
{
    featInit();

    return featTableTypeTestWild(type, FeatTypeTableDna, str);
}




/* @funcstatic featTypeTestProtWild ********************************************
**
** Given a feature type name,
** returns the valid feature type for the internal protein feature table
** following alternative names until a unique name is found
**
** @param [r]   type  [const AjPStr] Type name
** @param [r]   str   [const AjPStr] Wildcard name
** @return [AjBool] True if a match is found
** @@
******************************************************************************/

static AjBool featTypeTestProtWild(const AjPStr type, const AjPStr str)
{
    featInit();

    return featTableTypeTestWild(type, FeatTypeTableProtein, str);
}




/* @funcstatic featTagDna *****************************************************
**
** Given a feature tag name,
** returns the valid feature tag for the internal DNA feature table
**
** @param [r]   thys  [const AjPStr] Tag name
** @param [w]   knowntag  [AjBool*] ajTrue if the tag was found in the
**                                  list of known tags
** @return [const AjPStr] Valid feature tag name
** @@
******************************************************************************/

static const AjPStr featTagDna(const AjPStr thys, AjBool* knowntag)
{
    featInit();

    return featTableTag(thys, FeatTagsTableDna, knowntag);
}




/* @funcstatic featTagProt ****************************************************
**
** Given a feature tag name,
** returns the valid feature tag for the internal protein feature table
**
** @param [r]   thys  [const AjPStr] Tag name
** @param [w]   knowntag  [AjBool*] ajTrue if the tag was found in the
**                                  list of known tags
** @return [const AjPStr] Valid feature tag name
** @@
******************************************************************************/

static const AjPStr featTagProt(const AjPStr thys, AjBool* knowntag)
{
    featInit();

    return featTableTag(thys, FeatTagsTableProtein, knowntag);
}




/* @funcstatic featTableTypeExternal ******************************************
**
** Given a feature type name,
** returns the valid feature type for a feature table
**
** @param [r]   type  [const AjPStr] Type name
** @param [r]   table [const AjPTable]  Feature table
** @return [const AjPStr] Valid feature type
** @@
******************************************************************************/

static const AjPStr featTableTypeExternal(const AjPStr type,
					  const AjPTable table)
{
    static const AjPStr ret = NULL;
    static const AjPStr retkey = NULL;
    AjPStr defaultid = NULL;
    AjPStr tmpstr = NULL;
    ajint i;

    retkey = (const AjPStr) ajTableFetchKey(table, type);

    if(!retkey)
    {
        defaultid = ajStrNew();
        retkey = (AjPStr) ajTableFetch(table, defaultid);
        ajStrDel(&defaultid);
        ajDebug("featTableTypeExternal '%S' not in external table %x, "
                "use default '%S'\n", type, table, retkey);
    }
    
    ret = (AjPStr)ajTableFetch(table, retkey);
    ajDebug("featTableTypeExternal a '%S' found in internal table as"
            " '%S' = '%S\n", type, retkey, ret);

    if(ajStrGetCharLast(ret) != ';')
    {
        retkey = ret;
        ret = (AjPStr)ajTableFetch(table, retkey);
        ajDebug("featTableTypeExternal b '%S' found in internal table"
                " as '%S' = '%S\n", type, retkey, ret);
    }

    if(ajStrGetCharLast(ret) == ';')
    {
        if(ajStrGetCharFirst(ret) == ';')
        {
            ret = retkey;
        }
        else
        {
            i = ajStrFindAnyK(ret, ';');
            ajStrAssignSubS(&tmpstr, ret, 0, i-1);
            ajDebug("featTableTypeExternal '%S' is an alias for '%S'\n",
                    retkey, tmpstr);
            ret = (const AjPStr) ajTableFetchKey(table, tmpstr);

            if(!ret)	  /* oops, back to the previous one */
            {
                featWarn("featTableTypeExternal failed to find"
                         " '%S' alias '%S",
                         type, tmpstr);
                ret = (AjPStr)ajTableFetch(table, retkey);
            }

            ajStrDel(&tmpstr);
        }
    }

    ajDebug("featTableTypeExternal result '%S'\n",
            ret);

    return ret;
}




/* @funcstatic featTableTypeInternal ******************************************
**
** Given a feature type name,
** returns the valid feature type for a feature table
**
** @param [r]   type  [const AjPStr] Type name
** @param [r]   table [const AjPTable]  Feature table
** @return [const AjPStr] Valid feature type
** @@
******************************************************************************/

static const AjPStr featTableTypeInternal(const AjPStr type,
					  const AjPTable table)
{
    const AjPStr ret = NULL;
    const AjPStr retkey = NULL;
    const AjPStr tmpstr = NULL;

    retkey = (const AjPStr) ajTableFetchKey(table, type);

    if(retkey)
    {
	ajDebug("featTableTypeInternal '%S' found in internal table"
	  " as '%S'\n",type, retkey);
	tmpstr = (AjPStr)ajTableFetch(table, retkey);

	if(ajStrGetCharLast(tmpstr) != ';')
	{
	    ajDebug("featTableTypeInternal '%S' is an alias for '%S'\n",
		 retkey, tmpstr);
	    retkey = (const AjPStr) ajTableFetchKey(table, tmpstr);
	}

	ajDebug("featTableTypeInternal result '%S'\n",
		 retkey);
	return retkey;
    }

    ret = (AjPStr) ajTableFetch(table, ajStrNew());
    ajDebug("featTableTypeInternal '%S' not in internal table %x, "
	    "default to '%S'\n", type, table, ret);

    /* ajTablestrTrace(table); */

    /*ajDebug("featTableTypeInternal result '%S'\n",
	     ret);*/
    return ret;
}




/* @funcstatic featTableTypeInternalLimit **************************************
**
** Given a feature type name,
** returns the valid feature type for a feature table
** following alternative names which are common in the internal table
** which is a combination of multiple definitions
**
** @param [r]   type  [const AjPStr] Type name
** @param [r]   table [const AjPTable]  Feature table
** @return [const AjPStr] Valid feature type or NULL if not found
** @@
******************************************************************************/

static const AjPStr featTableTypeInternalLimit(const AjPStr type,
                                               const AjPTable table)
{
    const AjPStr retkey = NULL;
    const AjPStr tmpstr = NULL;
    const AjPStr savekey = NULL;
    ajuint i = 0;

    retkey = (const AjPStr) ajTableFetchKey(table, type);

    while(retkey && i < 5)
    {
        savekey = retkey;
	tmpstr = (AjPStr)ajTableFetch(table, retkey);
	ajDebug("featTableTypeInternalLimit '%S' found in internal table"
                " as '%S' (%S)\n",type, retkey, tmpstr);
	if(ajStrGetCharLast(tmpstr) != ';')
	{
	    ajDebug("featTableTypeInternalLimit '%S' is an alias for '%S'\n",
		 retkey, tmpstr);
	    retkey = (const AjPStr) ajTableFetchKey(table, tmpstr);
	}
        else 
        {
           tmpstr = (AjPStr)ajTableFetch(table, retkey);
           retkey = (const AjPStr) ajTableFetchKey(table, tmpstr);

           if(ajStrMatchS(retkey, savekey))
               retkey = NULL;
	}
	ajDebug("featTableTypeInternalLimit result %u '%S'\n",
                ++i, savekey);
    }

    if(savekey)
        return savekey;

    ajDebug("featTableTypeInternalLimit '%S' not in internal table %x, "
	    "default to NULL\n", type, table);

    /* ajTablestrTrace(table); */

    /*ajDebug("featTableTypeInternalLimit result '%S'\n",
	     ret);*/
    return NULL;
}




/* @funcstatic featTableTypeTestWild *******************************************
**
** Given a feature type name,
** returns the valid feature type for a feature table
** following alternative names which are common in the internal table
** which is a combination of multiple definitions
**
** @param [r]   type  [const AjPStr] Type name
** @param [r]   table [const AjPTable]  Feature table
** @param [r]   str   [const AjPStr] Wildcard name
** @return [AjBool] True if a match is found
** @@
******************************************************************************/

static AjBool featTableTypeTestWild(const AjPStr type,
                                    const AjPTable table,
                                    const AjPStr str)
{
    const AjPStr retkey = NULL;
    const AjPStr tmpstr = NULL;
    AjPStr tmpkey = NULL;
    ajuint i = 0;
    ajuint nkeys;
    void **keys = NULL;
    AjPStr key = NULL;

    if(ajStrMatchWildS(type, str))
        return ajTrue;

    retkey = (const AjPStr) ajTableFetchKey(table, type);

    while(retkey && i < 5)
    {
	tmpstr = (AjPStr)ajTableFetch(table, retkey);
	ajDebug("featTableTypeTestWild '%S' found in internal table"
                " as '%S' (%S)\n",type, retkey, tmpstr);

	if(ajStrGetCharLast(tmpstr) != ';')
	{
            if(ajStrMatchWildS(tmpstr, str))
                return ajTrue;

            tmpstr = (AjPStr)ajTableFetch(table, retkey);
	    retkey = (const AjPStr) ajTableFetchKey(table, tmpstr);
	}
        else 
        {
            ajStrAssignSubS(&tmpkey, tmpstr, 0, ajStrFindAnyK(tmpstr, ';')-1);

            if(ajStrMatchWildS(tmpkey, str))
            {
                ajStrDel(&tmpkey);
                return ajTrue;
            }

            ajStrDel(&tmpkey);

	    ajDebug("featTableTypeTestWild '%S' is an alias for '%S'\n",
		 retkey, tmpstr);

            retkey = (const AjPStr) ajTableFetchKey(table, tmpstr);

	}

	ajDebug("featTableTypeTestWild result %u '%S'\n",
                ++i, retkey);
    }

    nkeys = ajTableToarrayKeys(table, &keys);
    for (i=0; i<nkeys; i++) 
    {
        key = (AjPStr) keys[i];
        if(ajStrMatchWildS(key, str))
        {
            if(ajStrMatchS(featTableTypeInternalLimit(key,table),
                           featTableTypeInternalLimit(type,table)))
            {
                AJFREE(keys);
                return ajTrue;
            }
        }
    }

    AJFREE(keys);
    ajDebug("featTableTypeTestWild '%S' not in internal table %x, "
	    "default to NULL\n", type, table);

    return ajFalse;
}




/* @funcstatic featTableTag ***************************************************
**
** Given a feature tag name,
** returns the valid feature tag name for a feature table
**
** @param [r]   tag  [const AjPStr] Type name
** @param [r]   table [const AjPTable]  Feature table
** @param [w]   knowntag  [AjBool*] ajTrue if the tag was found in the
**                                  list of known tags
** @return [const AjPStr] Valid feature tag name
** @@
******************************************************************************/

static const AjPStr featTableTag(const AjPStr tag, const AjPTable table,
				 AjBool* knowntag)
{
    static const AjPStr ret     = NULL;

    if(!featTypeEmpty)
	featTypeEmpty = ajStrNewC("");

    if(tag)
    {
	ret = (const AjPStr) ajTableFetchKey(table, tag);

	if(ret)
	{
	    *knowntag = ajTrue;
	    /* ajDebug("featTag '%S' found in internal table as '%S'\n",
	       tag, ret); */

	    return ret;
	}
	else
	{
	    *knowntag = ajFalse;
	    ret = (AjPStr) ajTableFetch(table, featTypeEmpty);
	    /* ajDebug("featTag '%S' not in internal table %x,"
	       " default to '%S'\n",
	       tag, table, ret); */
	    /* ajTablestrTrace(table); */
	}
    }
    else
    {
	*knowntag = ajFalse;
	ret = (AjPStr) ajTableFetch(table, featTypeEmpty);
	/* ajDebug("featTag '%S' use default '%S'\n",
	   tag, ret); */
    }

    return ret;
}




/* @funcstatic featTableTagC **************************************************
**
** Given a feature tag name,
** returns the valid feature tag name for a feature table
**
** @param [r]   tag  [const char*] Type name
** @param [r]   table [const AjPTable]  Feature table
** @param [w] knowntag [AjBool*] ajTrue if the tag name is known
**                               ajFalse if the default name was substituted
** @return [const AjPStr] Valid feature tag name
** @@
******************************************************************************/

static const AjPStr featTableTagC(const char* tag, const AjPTable table,
				  AjBool* knowntag)
{
    static const AjPStr ret    = NULL;
    static AjPStr tmptag = NULL;

    ajStrAssignC(&tmptag, tag);

    ret = (const AjPStr) ajTableFetchKey(table, tmptag);

    if(ret)
    {
	*knowntag = ajTrue;
	/*
	   ajDebug("featTag '%S' found in internal table as '%S'\n",
	   tag, ret);
	   */
	return ret;
    }
    else
    {
	*knowntag = ajFalse;
	ret = (AjPStr) ajTableFetch(table, ajStrNew());
	/* ajDebug("featTag '%S' not in internal table %x, default to '%S'\n",
	   tag, table, ret); */
	/* ajTablestrTrace(table); */
	featWarn("Unrecognised feature tag '%s'", tag);
    }

    return ret;
}




/* @funcstatic featTagSpecialAllBiomaterial ************************************
**
** Tests a string as a valid internal (EMBL) feature /bio_material tag
**
** The format is a known type and optional :name
**
** @param  [r] val [const AjPStr] parameter value
** @return [AjBool] ajTrue for a valid value, possibly corrected
**                  ajFalse if invalid, to be converted to default (note) type
** @@
******************************************************************************/

static AjBool featTagSpecialAllBiomaterial(const AjPStr val)
{
    AjPStr inststr = NULL;
    AjPStr collstr = NULL;
    AjPStr idstr = NULL;
    AjBool ret = ajFalse;

    if(!featRegSpecialBiomaterial)
	featRegSpecialBiomaterial = ajRegCompC("^([^:]+)(:([^:]+))?(:(.*))?$");

    if(ajRegExec(featRegSpecialBiomaterial, val))
    {
	ajRegSubI(featRegSpecialBiomaterial, 1, &inststr);
	ajRegSubI(featRegSpecialBiomaterial, 3, &collstr);
	ajRegSubI(featRegSpecialBiomaterial, 5, &idstr);
        ret = ajTrue;
    }

    if(!ret)
    {
	featWarn("bad /bio_material value '%S'", val);
    }

    ajStrDel(&inststr);
    ajStrDel(&collstr);
    ajStrDel(&idstr);

    return ret;
}




/* @funcstatic featTagSpecialAllAnticodon *************************************
**
** Tests a string as a valid internal (EMBL) feature /anticodon tag
**
** The format is  (pos:base_range,aa:amino_acid)
**
** @param  [r] val [const AjPStr] parameter value
** @return [AjBool] ajTrue for a valid value, possibly corrected
**                  ajFalse if invalid, to be converted to default (note) type
** @@
******************************************************************************/

static AjBool featTagSpecialAllAnticodon(const AjPStr val)
{
    /*
       static AjPStr begstr = NULL;
       static AjPStr endstr = NULL;
       static AjPStr aastr  = NULL;
       AjBool saveit = ajFalse;
    */
    AjBool ret = ajFalse;

    if(!featRegSpecialAnticodon)
	featRegSpecialAnticodon =
	    ajRegCompC("^[(]pos:([0-9]+)[.][.]([0-9]+),aa:([^)]+)[)]$");

    if(ajRegExec(featRegSpecialAnticodon, val))
    {
	ret = ajTrue;
	/*
	if(saveit)
	{
	    ajRegSubI(featRegSpecialAnticodon, 1, &begstr);
	    ajRegSubI(featRegSpecialAnticodon, 2, &endstr);
	    ajRegSubI(featRegSpecialAnticodon, 3, &aastr);
	}
	*/

    }

    if(!ret)
    {
	featWarn("bad /anticodon value '%S'",   val);
    }

    return ret;
}




/* @funcstatic featTagSpecialAllCitation **************************************
**
** Tests a string as a valid internal (EMBL) feature /citation tag
**
** The format is [1] where the number is a citation in an EMBL entry.
**
** @param  [r] val [const AjPStr] parameter value
** @return [AjBool] ajTrue for a valid value, possibly corrected
**                  ajFalse if invalid, to be converted to default (note) type
** @@
******************************************************************************/

static AjBool featTagSpecialAllCitation(const AjPStr val)
{
    const char* cp = ajStrGetPtr(val);
    ajint i = 0;
    AjBool ret = ajFalse;

    if(*cp++ == '[')
    {
	while(*cp)
	{
	    if(!isdigit((ajint)*cp))
	       break;

	    i++;
	    cp++;
	}

	if(*cp++ == ']')
	    if(!*cp)
		ret = ajTrue;
	if(!i)
	    ret = ajFalse;
    }

    if(!ret)
    {
	featWarn("bad /citation value '%S'",   val);
    }

    return ret;
}




/* @funcstatic featTagSpecialAllCodon *****************************************
**
** Tests a string as a valid internal (EMBL) feature /codon tag
**
** The format is (seq:"ttt",aa:Leu)
**
** Corrects missing quotes
**
** @param  [u] pval [AjPStr*] parameter value
** @return [AjBool] ajTrue for a valid value, possibly corrected
**                  ajFalse if invalid, to be converted to default (note) type
** @@
******************************************************************************/

static AjBool featTagSpecialAllCodon(AjPStr* pval)
{
    AjPStr seqstr = NULL;
    AjPStr aastr  = NULL;
    AjBool saveit = ajFalse;
    AjBool ret = ajFalse;

    if(!featRegSpecialCodon)
	featRegSpecialCodon =
	    ajRegCompC("^[(]seq:\"([acgt][acgt][acgt])\",aa:([^)]+)[)]$");

    if(!featRegSpecialCodonBad)	/* sometimes fails to quote sequence */
	featRegSpecialCodonBad =
	    ajRegCompC("^[(]seq:([acgt][acgt][acgt]),aa:([^)]+)[)]$");

    if(ajRegExec(featRegSpecialCodon, *pval))
    {
	ret = ajTrue;

	if(saveit)
	{
	    ajRegSubI(featRegSpecialCodon, 1, &seqstr);
	    ajRegSubI(featRegSpecialCodon, 2, &aastr);
	}
    }

    else if(ajRegExec(featRegSpecialCodonBad, *pval))
    {
	ret = ajTrue;
	ajRegSubI(featRegSpecialCodonBad, 1, &seqstr);	/* needed correction */
	ajRegSubI(featRegSpecialCodonBad, 2, &aastr);
	ajFmtPrintS(pval, "(seq:\"%S\",aa:%S)",seqstr, aastr);
	featWarn("unquoted /codon value corrected to '%S'", *pval);
    }

    if(!ret)
    {
	featWarn("bad /codon value '%S'",   *pval);
    }

    ajStrDel(&seqstr);
    ajStrDel(&aastr);

    return ret;
}




/* @funcstatic featTagSpecialAllCollectiondate ********************************
**
** Tests a string as a valid internal (EMBL) feature /collection_date tag
**
** The format is YYYY Mmm-YYYY or DD-Mmm-YYYY
**
** @param  [r] val [const AjPStr] parameter value
** @return [AjBool] ajTrue for a valid value, possibly corrected
**                  ajFalse if invalid, to be converted to default (note) type
** @@
******************************************************************************/

static AjBool featTagSpecialAllCollectiondate(const AjPStr val)
{
    AjPStr daystr = NULL;
    AjPStr monstr  = NULL;
    ajint day;
    ajint i;
    AjBool ret = ajFalse;
    AjBool ok;
    const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
			    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    if(!featRegSpecialColdate)
	featRegSpecialColdate =
	    ajRegCompC("^((\\d\\d)[-])?(([ADFJMNOS][A-Za-z][A-Za-z])[-])?([12]"
                       "[7890]\\d\\d)$");

    if(ajRegExec(featRegSpecialColdate, val))
    {
	ret = ajTrue;
	ajRegSubI(featRegSpecialColdate, 2, &daystr);
	ajRegSubI(featRegSpecialColdate, 4, &monstr);

	if(ajStrGetLen(daystr))
	{
	    if(!ajStrGetLen(monstr))
		ret = ajFalse;

	    ajStrToInt(daystr, &day);

	    if(day<1 || day > 31)
		ret = ajFalse;
	}

	if(ajStrGetLen(monstr))
	{
	    ok = ajFalse;

	    for(i=0;i<12;i++)
		if(ajStrMatchCaseC(monstr, months[i]))
		{
		    ok = ajTrue;
		    break;
		}

	    if(!ok)
                ret = ajFalse;
	}
    }

    if(!ret)
	featWarn("bad /collection_date value '%S'", val);

    ajStrDel(&daystr);
    ajStrDel(&monstr);

    return ret;
}




/* @funcstatic featTagSpecialAllConssplice ************************************
**
** Tests a string as a valid internal (EMBL) feature /cons_splice tag
**
** The format is (5'site:YES,3'site:NO) where the booleans can
** be YES, NO, or ABSENT (intended for use where one site is NO and
** the other is missing)
**
** Corrects missing parts of the value and bad case.
**
** @param  [u] pval [AjPStr*] parameter value
** @return [AjBool] ajTrue for a valid value, possibly corrected
**                  ajFalse if invalid, to be converted to default (note) type
** @@
******************************************************************************/

static AjBool featTagSpecialAllConssplice(AjPStr* pval)
{
    AjPStr begstr = NULL;
    AjPStr endstr = NULL;
    const char* cp;
    const char* cq;
    ajint i;
    AjBool islow = ajFalse;
    AjBool ret = ajFalse;

    if(!featRegSpecialConssplice)
	featRegSpecialConssplice =
	    ajRegCompC("^[(]5'site:([A-Za-z]+),3'site:([A-Za-z]+)[)]$");


    if(ajStrPrefixC(*pval, "(5'site:") && ajStrGetCharLast(*pval) == ')')
    {
	cp = ajStrGetPtr(*pval);
	cp += 8;
	cq = cp;
	i=0;

	while(*cp && (*cp != ','))
	{
	    if(islower((ajint)*cp++))
		islow = ajTrue;

	    i++;
	}

	ajStrAssignLenC(&begstr, cq, i);

	if(ajCharPrefixC(cp, ",3'site:"))
	{
	    cp += 8;
	    cq = cp;
	    i=0;

	    while(*cp && (*cp != ')'))
	    {
		if(islower((ajint)*cp++))
		    islow = ajTrue;

		i++;
	    }
	    ajStrAssignLenC(&endstr, cq, i);

	    if(*cp == ')' && !*(cp+1) &&
	       ajStrGetLen(begstr) && ajStrGetLen(endstr))
		ret = ajTrue;

	    switch(ajStrGetCharFirst(begstr))
	    {
                case 'Y':
                    if(!ajStrMatchCaseC(begstr, "yes"))
                        ret = ajFalse;

                    break;
                case 'N':
                    if(!ajStrMatchCaseC(begstr, "no"))
                        ret = ajFalse;

                    break;
                case 'A':
                    if(!ajStrMatchCaseC(begstr, "absent"))
                        ret = ajFalse;

                    break;
                default:
                    ret = ajFalse;
	    }

	    switch(ajStrGetCharFirst(endstr))
	    {
                case 'Y':
                    if(!ajStrMatchCaseC(endstr, "yes"))
                        ret = ajFalse;

                    break;
                case 'N':
                    if(!ajStrMatchCaseC(endstr, "no"))
                        ret = ajFalse;

                    break;
                case 'A':
                    if(!ajStrMatchCaseC(endstr, "absent"))
                        ret = ajFalse;

                    break;
                default:
                    ret = ajFalse;
	    }
	}
    }

    if(!ret)
    {
	featWarn("bad /cons_splice value '%S'",   *pval);
	ajDebug("bad /cons_splice value '%S' beg: '%S' end: '%S'",
		*pval, begstr, endstr);
    }

    if(islow)
    {
	ajFmtPrintS(pval, "(5'site:%S,3'site:%S)", begstr, endstr);
	featWarn("bad /cons_splice value corrected to '%S'", *pval);
    }

    ajStrDel(&begstr);
    ajStrDel(&endstr);

    return ret;
}




/* @funcstatic featTagSpecialAllInference *************************************
**
** Tests a string as a valid internal (EMBL) feature /inference tag
**
** The format is TYPE:EVIDENCE_BASIS
** where TYPE is a list of defined values with optionally " (same species)"
** and EVIDENCE is dbname:accesion.version of algorithm:version
**
** @param  [r] val [const AjPStr] parameter value
** @return [AjBool] ajTrue for a valid value, possibly corrected
**                  ajFalse if invalid, to be converted to default (note) type
** @@
******************************************************************************/

static AjBool featTagSpecialAllInference(const AjPStr val)
{
    AjPStr typstr = NULL;
    ajint i;
    AjBool ret = ajFalse;
    AjBool ok;
    const char* types[] =
    {
	"non-experimental evidence, no additional details recorded",
	"similar to sequence",
	"similar to AA sequence",
	"similar to AA sequence sequence", /* found in EMBL 104 */
	"similar to DNA sequence",
	"similar to RNA sequence",
	"similar to RNA sequence, mRNA",
	"similar to RNA sequence,mRNA", /* nonstandard, found in EMBL 104 */
	"similar to RNA sequence, EST",
	"similar to RNA sequence, other RNA",
	"profile",
	"nucleotide motif",
	"protein motif",
	"ab initio prediction",
        "alignment",
	NULL
    };

    if(!featRegSpecialInference)
	featRegSpecialInference = ajRegCompC("^([^:]+)(:([^:]+)?)*$");

    if(ajRegExec(featRegSpecialInference, val))
    {
	ret = ajTrue;
	ajRegSubI(featRegSpecialInference, 1, &typstr);

	ok = ajFalse;
	for(i=0; types[i]; i++)
	{
	    if(ajStrPrefixC(typstr, types[i]))
	    {
		if(ajStrMatchC(typstr, types[i]))
		{
		    ok = ajTrue;
		    break;
		}

		if(ajStrSuffixC(typstr, " (same species)"))
		{
		    if(ajStrGetLen(typstr) == strlen(types[i]) + 15)
		    {
			ok = ajTrue;
			break;
		    }
		}

		else if(ajStrSuffixC(typstr, "(same species)"))
		{       /* found in EMBL 104 without expected space */
		    if(ajStrGetLen(typstr) == strlen(types[i]) + 14)
		    {
			ok = ajTrue;
			break;
		    }
		}

                else if(ajStrSuffixC(typstr, " (same species) "))
		{       /* found in EMBL 104 with extra trailing space */
		    if(ajStrGetLen(typstr) == strlen(types[i]) + 16)
		    {
			ok = ajTrue;
			break;
		    }
		}
	    }
	}

	if(!ok)
	    ret = ajFalse;
    }

    if(!ret)
	featWarn("bad /inference value '%S' type: '%S'", val, typstr);

    ajStrDel(&typstr);
    return ret;
}




/* @funcstatic featTagSpecialAllLatlon ************************************
**
** Tests a string as a valid internal (EMBL) feature /lat_lon tag
**
** @param  [r] val [const AjPStr] parameter value
** @return [AjBool] ajTrue for a valid value, possibly corrected
**                  ajFalse if invalid, to be converted to default (note) type
** @@
******************************************************************************/

static AjBool featTagSpecialAllLatlon(const AjPStr val)
{
    AjPStr labstr = NULL;
    AjBool saveit = ajFalse;
    AjBool ret = ajFalse;

    if(!featRegSpecialLatlon)
      featRegSpecialLatlon = ajRegCompC("^[0-9.]+ [NS] [0-9.]+ [EW]$");

    if(ajRegExec(featRegSpecialLatlon, val))
    {
	ret = ajTrue;
	if(saveit)
	    ajRegSubI(featRegSpecialLatlon, 1, &labstr);
    }

    if(!ret)
    {
	featWarn("bad /lat_lon value '%S'",   val);
    }

    return ret;
}




/* @funcstatic featTagSpecialAllPcrprimers ************************************
**
** Tests a string as a valid internal (EMBL) feature /PCR_primers tag
**
** @param  [r] val [const AjPStr] parameter value
** @return [AjBool] ajTrue for a valid value
**                  ajFalse if invalid, to be converted to default (note) type
** @@
******************************************************************************/

static AjBool featTagSpecialAllPcrprimers(const AjPStr val)
{
    AjBool ret = ajFalse;

    if(!featRegSpecialPrimer)
      featRegSpecialPrimer = ajRegCompC("^((fwd|rev)_name: [^,]+, )?"
					"(fwd|rev)_seq: [acgtubdhkmnrsvwxy<>i]+"
					"(, ((fwd|rev)_name: [^,]+, )?"
					"(fwd|rev)_seq: [acgtubdhkmnrsvwxy<>i]+)*");

    if(ajRegExec(featRegSpecialPrimer, val))
    {
	ret = ajTrue;
    }

    if(!ret)
    {
	featWarn("bad /PCR_primers value '%S'",   val);
    }

    return ret;
}




/* @funcstatic featTagSpecialAllRptunit ***************************************
**
** Tests a string as a valid internal (EMBL) feature /rpt_unit tag
**
** The format is 123..789
** Labels are also allowed which should be feature tags in the entry.
** Genbank (NCBI) appear to be putting the sequence consensus in. Usually
** this is a valid "label" - except of course that the label does not exist.
** One horror (July 2002) was: /rpt_unit=TCCTCACGTAG(T/C)
** others are /rpt_unit=gaa;taa /rpt_unit=(CA)2(TG)6(CA)2
** /rpt_unit=attatatgata(n)6-7gttt(n)3gtagactagtt(n)3ttatgttt
**
** Version 6 of the feature table allows /rpt_unit="sequence" to include
** these nasties trim spaces and resolve ambiguities. Not to be used
** for "alu" etc.
**
** Location can be complement(nn..nn)
**
** @param  [r] val [const AjPStr] parameter value
** @return [AjBool] ajTrue for a valid value, possibly corrected
**                  ajFalse if invalid, to be converted to default (note) type
** @@
******************************************************************************/

static AjBool featTagSpecialAllRptunit(const AjPStr val)
{
    AjBool ret = ajFalse;

    /*
    AjBool saveit = ajFalse;
    AjPStr begstr = NULL;
    AjPStr endstr = NULL;
    AjPStr labstr = NULL;
    */

    if(!featRegSpecialRptRange)				/* base range */
	featRegSpecialRptRange =
	    ajRegCompC("^(([a-zA-Z0-9_]+:)?[0-9]+)[.][.]([0-9]+)$");
    if(!featRegSpecialRptRangeLab)			/* feature label */
	featRegSpecialRptRangeLab =
	    ajRegCompC("^(([a-zA-Z0-9_]+:)?[a-zA-Z0-9_]+)$");
    if(!featRegSpecialRptRangeComp)	/* complement(base range) */
	featRegSpecialRptRangeComp =
	    ajRegCompC("^(complement[(]([a-zA-Z0-9_]+:)?[0-9]+)"
		       "[.][.]([0-9]+)[)]$");
    if(!featRegSpecialRptunitSeq)
        featRegSpecialRptunitSeq =
            ajRegCompC("^([abcdghkmnrstuvwxyABCDGHKMNRSTUVWXY0-9/();-]+)$");

    if(ajRegExec(featRegSpecialRptRange, val))
    {
	ret = ajTrue;
	/*
	if(saveit)
	{
	    ajRegSubI(featRegSpecialRptRange, 1, &begstr);
	    ajRegSubI(featRegSpecialRptRange, 3, &endstr);
	}
        */
    }

    else if(ajRegExec(featRegSpecialRptRangeLab, val))
    {
	ret = ajTrue;
	/*
	if(saveit)
	    ajRegSubI(featRegSpecialRptRangeLab, 1, &labstr);
        */
    }

    else if(ajRegExec(featRegSpecialRptRangeComp, val))
    {
	ret = ajTrue;
	/*
	if(saveit)
	{
	    ajRegSubI(featRegSpecialRptRangeComp, 3, &begstr);
	    ajRegSubI(featRegSpecialRptRangeComp, 1, &endstr);
	}
        */
    }

    if(ajRegExec(featRegSpecialRptRangeComp, val))
    {
	ret = ajTrue;
	/*
	if(saveit)
	{
	  ajRegSubI(featRegSpecialRptRangeComp, 3, &begstr);
	  ajRegSubI(featRegSpecialRptRangeComp, 1, &endstr);
	}
        */
    }

    /* provided because EMBL/Genbank has things like this:
       /rpt_unit=TCCTCACGTAG(T/C)
       /rpt_unit=(A)n
       */

    else if(ajRegExec(featRegSpecialRptunitSeq, val))
    {
	ret = ajTrue;
	/*
	if(saveit)
	    ajRegSubI(featRegSpecialRptunitSeq, 1, &labstr);
        */
    }

    if(!ret)
    {
	featWarn("bad /rpt_unit value '%S'",   val);
    }

    return ret;
}




/* @funcstatic featTagSpecialAllRange *****************************************
**
** Tests a string as a valid internal (EMBL) feature /rpt_unit_range tag
**
** The format is 123..789
** Labels are also allowed which should be feature tags in the entry.
** Genbank (NCBI) appear to be putting the sequence consensus in. Usually
** this is a valid "label" - except of course that the label does not exist.
** One horror (July 2002) was: /rpt_unit=TCCTCACGTAG(T/C)
** others are /rpt_unit=gaa;taa /rpt_unit=(CA)2(TG)6(CA)2
** /rpt_unit=attatatgata(n)6-7gttt(n)3gtagactagtt(n)3ttatgttt
**
** Version 6 of the feature table allows /rpt_unit="sequence" to include
** these nasties trim spaces and resolve ambiguities. Not to be used
** for "alu" etc.
**
** Location can be complement(nn..nn)
**
** @param  [r] val [const AjPStr] parameter value
** @return [AjBool] ajTrue for a valid value, possibly corrected
**                  ajFalse if invalid, to be converted to default (note) type
** @@
******************************************************************************/

static AjBool featTagSpecialAllRange(const AjPStr val)
{
     AjBool ret = ajFalse;
    /*
    AjBool saveit = ajFalse;
    AjPStr begstr = NULL;
    AjPStr endstr = NULL;
    AjPStr labstr = NULL;
    */

     if(!featRegSpecialRptRange)				/* base range */
	featRegSpecialRptRange =
	    ajRegCompC("^(([a-zA-Z0-9_]+:)?[0-9]+)[.][.]([0-9]+)$");

     if(!featRegSpecialRptRangeLab)			/* feature label */
	featRegSpecialRptRangeLab =
	    ajRegCompC("^(([a-zA-Z0-9_]+:)?[a-zA-Z0-9_]+)$");

     if(!featRegSpecialRptRangeComp)	/* complement(base range) */
	featRegSpecialRptRangeComp =
	    ajRegCompC("^(complement[(]([a-zA-Z0-9_]+:)?[0-9]+)"
		       "[.][.]([0-9]+)[)]$");

    if(ajRegExec(featRegSpecialRptRange, val))
    {
	ret = ajTrue;
	/*
	if(saveit)
	{
	    ajRegSubI(featRegSpecialRptRange, 1, &begstr);
	    ajRegSubI(featRegSpecialRptRange, 3, &endstr);
	}
        */
    }

    else if(ajRegExec(featRegSpecialRptRangeLab, val))
    {
	ret = ajTrue;
	/*
	if(saveit)
	    ajRegSubI(featRegSpecialRptRangeLab, 1, &labstr);
        */
    }

    else if(ajRegExec(featRegSpecialRptRangeComp, val))
    {
	ret = ajTrue;
	/*
	if(saveit)
	{
	    ajRegSubI(featRegSpecialRptRangeComp, 3, &begstr);
	    ajRegSubI(featRegSpecialRptRangeComp, 1, &endstr);
	}
        */
    }


    if(!ret)
    {
	featWarn("bad /rpt_unit_tange value '%S'",   val);
    }

    return ret;
}




/* @funcstatic featTagSpecialAllRptunitseq ************************************
**
** Tests a string as a valid internal (EMBL) feature /rpt_unit_seq tag
**
** examples from GenBank include /rpt_unit_seq=TCCTCACGTAG(T/C)
** others are /rpt_unit_seq=gaa;taa /rpt_unit_seq=(CA)2(TG)6(CA)2
** /rpt_unit_seq=attatatgata(n)6-7gttt(n)3gtagactagtt(n)3ttatgttt
**
** Corrects extra spaces from long sequence values
**
** @param  [u] pval [AjPStr*] parameter value
** @return [AjBool] ajTrue for a valid value, possibly corrected
**                  ajFalse if invalid, to be converted to default (note) type
** @@
******************************************************************************/

static AjBool featTagSpecialAllRptunitseq(AjPStr *pval)
{
    AjBool ret = ajFalse;
    ajuint i;
    const char* specials[] =
    {
        "a-rich",               /* found in EMBL 104 */
        "at-rich",              /* found in EMBL 104 */
        "c-rich",               /* found in EMBL 104 */
        "ct-rich",              /* found in EMBL 104 */
        "CT-rich",              /* found in EMBL 104 */
        "g-rich",               /* found in EMBL 104 */
        "ga-rich",              /* found in EMBL 104 */
        "GA-rich",              /* found in EMBL 104 */
        "gc-rich",              /* found in EMBL 104 */
        "t-rich",               /* found in EMBL 104 */
        "CT_AT",                /* found in EMBL 104 */
        "TA_CA",                /* found in EMBL 104 */
        "AGAT_AGAC",            /* found in EMBL 104 */
        "GAA-repeat",           /* found in EMBL 104 */
        "GGA-repeat",           /* found in EMBL 104 */
        "tgtc (imperfect)",     /* found in EMBL 104 */
        "score",                /* found in EMBL 104 */
        "direct",               /* found in EMBL 104 */
        "dispersed",            /* found in EMBL 104 */
        "inverted",             /* found in EMBL 104 */
        "ca repeat",            /* found in EMBL 104 */
        "cac/caa/cag compound repeat", /* found in EMBL 104 */
        "cata interrupted repeat",     /* found in EMBL 104 */
        "CA dinucleotide microsatellite",          /* found in EMBL 104 */
        "gaa/gac compound repeat",     /* found in EMBL 104 */
        "dodecanucleotide",          /* found in EMBL 104 */
        "heptamer",             /* found in EMBL 104 */
        "gttt..gt",             /* found in EMBL 104 */
        "telunit",              /* found in EMBL 104 */
        "af(12)",               /* found in EMBL 104 */
        "alu",                  /* found in EMBL 104 */
        "Alu",                  /* found in EMBL 104 */
        "AluJo",                /* found in EMBL 104 */
        "AluSg",                /* found in EMBL 104 */
        "AluY",                 /* found in EMBL 104 */
        "hif2DR",               /* found in EMBL 104 */
        "B1element",            /* found in EMBL 104 */
        "L1MC/D",               /* found in EMBL 104 */
        "L1MC3",                /* found in EMBL 104 */
        "L1MD3",                /* found in EMBL 104 */
        "L1ME",                 /* found in EMBL 104 */
        "RMER17B",              /* found in EMBL 104 */
        "RLTR11A",              /* found in EMBL 104 */
        "poly A",               /* found in EMBL 104 */
        NULL
    };

    /*
    AjBool saveit = ajFalse;
    AjPStr labstr = NULL;
    */

    if(!featRegSpecialRptunitSeq)
        featRegSpecialRptunitSeq =
            ajRegCompC("^([ abcdghkmnrstuvwxyABCDGHKMNRSTUVWXY0-9_/\\[\\]\\(\\):;.,+-]+)$");

    if(!featRegSpecialRptunitSeqPos)
        featRegSpecialRptunitSeqPos =
            ajRegCompC("^((complement[\\(])?[<]?\\d+[.][.]?[>]?\\d+[\\)]?)$");

    if(ajRegExec(featRegSpecialRptunitSeq, *pval))
    {
        ajStrRemoveWhite(pval);   /* remove wrapping spaces in long seq. */

	ret = ajTrue;
	/*
	if(saveit)
	    ajRegSubI(featRegSpecialRptunitSeq, 1, &labstr);
        */
    }
    else if(ajRegExec(featRegSpecialRptunitSeqPos, *pval))
    {
	ret = ajTrue;
    }
    else
    {
	for(i=0; specials[i]; i++)
	{
            if(ajStrMatchC(*pval, specials[i]))
            {
                ret = ajTrue;
                break;
            }
        }
    }
    
    if(!ret)
    {
	featWarn("bad /rpt_unit_seq value '%S'",   *pval);
    }

    return ret;
}




/* @funcstatic featTagSpecialAllTranslexcept **********************************
**
** Tests a string as a valid internal (EMBL) feature /transl_except tag
**
** The format is (pos:213..215,aa:Trp)
**
** FT 6.2 todo aa 3 letter codes, or Sec (U) or TERM or OTHER
**
** FT 6.2 todo pos can be 1 or 2 bases for polyA-completed stops (TERM)
** which should have a /note
**
** @param  [r] val [const AjPStr] parameter value
** @return [AjBool] ajTrue for a valid value, possibly corrected
**                  ajFalse if invalid, to be converted to default (note) type
** @@
******************************************************************************/

static AjBool featTagSpecialAllTranslexcept(const AjPStr val)
{
    AjBool saveit = ajFalse;
    AjBool ret = ajFalse;

    if(!featRegSpecialTrans)
	featRegSpecialTrans =
	    ajRegCompC("^[(]pos:([0-9]+)[.][.]([0-9]+),aa:([^)]+)[)]$");

    if(!featRegSpecialTransBad)		/* start position only */
	featRegSpecialTransBad =
	    ajRegCompC("^[(]pos:([0-9]+),aa:([^)]+)[)]$");

    if(!featRegSpecialTransComp)
	featRegSpecialTransComp =
	    ajRegCompC("^[(]pos:complement\\(([0-9]+)[.][.]([0-9]+)\\),"
		       "aa:([^)]+)[)]$");

    if(!featRegSpecialTransBadComp)
	featRegSpecialTransBadComp =
	    ajRegCompC("^[(]pos:complement\\(([0-9]+)\\),"
		       "aa:([^)]+)[)]$");

    if(ajRegExec(featRegSpecialTrans, val))
    {
	ret = ajTrue;

	if(saveit)
	{
	    ajRegSubI(featRegSpecialTrans, 1, &featTransBegStr);
	    ajRegSubI(featRegSpecialTrans, 2, &featTransEndStr);
	    ajRegSubI(featRegSpecialTrans, 3, &featTransAaStr);
	}
    }
    else if(ajRegExec(featRegSpecialTransComp, val))
    {
	ret = ajTrue;

	if(saveit)
	{
	    ajRegSubI(featRegSpecialTransComp, 1, &featTransBegStr);
	    ajRegSubI(featRegSpecialTransComp, 2, &featTransEndStr);
	    ajRegSubI(featRegSpecialTransComp, 3, &featTransAaStr);
	}
    }

/* Can have single base (or 2 base) positions where trailing As are
   added as post-processing in some species */

    else if(ajRegExec(featRegSpecialTransBad, val))
    {
	ret = ajTrue;

	if(saveit)
	{
	    ajRegSubI(featRegSpecialTransBad, 1, &featTransBegStr);
	    ajRegSubI(featRegSpecialTransBad, 2, &featTransAaStr);
	}
    }
    else if(ajRegExec(featRegSpecialTransBadComp, val))
    {
	ret = ajTrue;

	if(saveit)
	{
	    ajRegSubI(featRegSpecialTransBadComp, 1, &featTransBegStr);
	    ajRegSubI(featRegSpecialTransBadComp, 2, &featTransAaStr);
	}
    }

    if(!ret)
    {
	featWarn("bad /transl_except value '%S'",   val);
    }

    return ret;
}




/* @funcstatic featTagSpecialAllDbxref ****************************************
**
** Tests a string as a valid internal (EMBL) feature /db_xref tag
**
** The format is <database>:<identifier>
**
** @param  [r] val [const AjPStr] parameter value
** @return [AjBool] ajTrue for a valid value, possibly corrected
**                  ajFalse if invalid, to be converted to default (note) type
** @@
******************************************************************************/

static AjBool featTagSpecialAllDbxref(const AjPStr val)
{
    const char* cp = NULL;
    AjBool ret           = ajTrue;

    /*
    AjBool saveit = ajFalse;
    AjPStr dbstr  = NULL;
    AjPStr idstr  = NULL;
    */

    cp = ajStrGetPtr(val);
    /*
    if(saveit)
	ajStrAssignResC(&dbstr, ajStrGetLen(val), "");
    */
    while(*cp && (*cp != ':'))
    {
	/*
	if(saveit)
	    ajStrAppendK(&dbstr, *cp);
	*/
	cp++;
    }

    if(!*cp)
	ret = ajFalse;
    else
    {
	cp++;
	if(!*cp)
	    ret = ajFalse;
    }

    /*
    if(saveit)
	ajStrAssignC(&idstr, cp);
    */

    if(!ret)
    {
	featWarn("bad /db_xref value '%S'",   val);
    }

    return ret;
}




/* @funcstatic featTagSpecialAllProteinid *************************************
**
** Tests a string as a valid internal (EMBL) feature /protein_id tag
**
** The format is AAA12345.1
**
** @param  [r] val [const AjPStr] parameter value
** @return [AjBool] ajTrue for a valid value, possibly corrected
**                  ajFalse if invalid, to be converted to default (note) type
** @@
******************************************************************************/

static AjBool featTagSpecialAllProteinid(const AjPStr val)
{
    const char* cp = ajStrGetPtr(val);
/*    const char* cq; */
    ajint icp;
    ajint i = 0;
    ajint idot = 0;
    AjBool ret = ajFalse;

    /*
    AjBool saveit = ajFalse;
    AjPStr idstr    = NULL;
    AjPStr preidstr = NULL;
    */

    if(ajStrPrefixC(val, "ENSP"))  /* older entries had ENSEMBL Protein IDs */
    {
	cp += 4;

	while(*cp)
	{
	    if(!isdigit((ajint)*cp++))
		break;
	    i++;
	}

        /* ENSP then all numbers */
	if(!*cp && i > 5)
            ret = ajTrue;

/*	if(ret)
        featWarn("ENSEMBL /protein_id value '%S'",   val);*/

	/*
	if(saveit)
	{
	    ajStrAssignS(&preidstr, val);
	    ajStrAssignS(&idstr, val);
	}
	*/
    }
    else
    {
	cp = ajStrGetPtr(val);
/*	cq = cp; */
	i=0;
	while(*cp)
	{
	    i++;
	    icp = *cp;

	    if(i <= 3)
	    {
	      if(icp == '_')
	      {
		if(i!=3)
		  break;
	      }
	      else if(!isalpha(icp) || !isupper(icp))
		    break;
	    }
	    else if(*cp == '.')
	    {
		if(idot)		/* once only */
		    break;

		idot = i;

		if(idot < 9)	/* at least ABC12345. */
		    break;
	    }
	    else
	    {
		if(!isdigit(icp))
		    break;
	    }
	    cp++;
	}

	/*
	if(saveit)
	{
	    ajStrAssignLenC(&preidstr, cq, idot-1);
	    ajStrAssignLenC(&idstr, cq, i);
	}
        */

	if(!*cp && idot && (i > idot))
	    ret = ajTrue;
    }

    if(!ret)
    {
	featWarn("bad /protein_id value '%S'", val);
    }

    return ret;
}




/* @funcstatic featTagSpecialAllReplace ***************************************
**
** Tests a string as a valid internal (EMBL) feature /replace tag
**
** The format is "<sequence>" where sequence is empty for a deletion
**
** Corrects extra spaces from long sequence values
**
** @param  [u] pval [AjPStr*] parameter value
** @return [AjBool] ajTrue for a valid value, possibly corrected
**                  ajFalse if invalid, to be converted to default (note) type
** @@
******************************************************************************/

static AjBool featTagSpecialAllReplace(AjPStr* pval)
{
    AjBool ret = ajFalse;

    /* n is used in old_sequence */
    /* and in misc_difference features */
    if(!featRegTagReplace)
	featRegTagReplace =
	    ajRegCompC("^([abcdghkmnrstuvwxyABCDGHKMNRSTUVWXY/-]*)$");

    /* if(!featRegTagReplace)
       featRegTagReplace = ajRegCompC("^\"([acgt]*)\"$");*/

    /* no need to add quotes here - we will add them if needed on output */

    /*
       ajDebug("Before quote '%S' %c %c\n", *pval,
       ajStrGetCharFirst(*pval), ajStrGetCharLast(*pval));
       ajStrFmtQuote(pval);
       ajDebug(" After quote '%S' %c %c\n", *pval,
       ajStrGetCharFirst(*pval), ajStrGetCharLast(*pval));
       */

    ajStrRemoveWhite(pval);   /* remove wrapping spaces in long seq. */

    if(ajRegExec(featRegTagReplace, *pval))
	ret = ajTrue;	    /* substring 1 has the matched sequence */

    if(!ret)
    {
	featWarn("bad /replace value '%S'",   *pval);
    }

    return ret;
}




/* @funcstatic featTagSpecialAllTranslation ***********************************
**
** Tests a string as a valid internal (EMBL) feature /translation tag
**
** The format is valid amino acid codes, no white space
**
** Corrects extra spaces from long sequence values
**
** @param  [u] pval [AjPStr*] parameter value
** @return [AjBool] ajTrue for a valid value, possibly corrected
**                  ajFalse if invalid, to be converted to default (note) type
** @@
******************************************************************************/

static AjBool featTagSpecialAllTranslation(AjPStr* pval)
{
    AjPStr seqstr = NULL;
    const char* cp;
    ajint icp;
    AjBool saveit = ajFalse;
    AjBool ret = ajFalse;

    ajStrRemoveWhite(pval);   /* remove wrapping spaces in long seq. */
    cp = ajStrGetPtr(*pval);

    if(saveit)
	ajStrAssignResC(&seqstr, ajStrGetLen(*pval), "");

    while(*cp)
    {
	icp = *cp;

	if(!isalpha(icp))
	    break;

	if(!isupper(icp))
	    break;

/*	if(strchr("JO",icp))
        break; */

	if(saveit)
	    ajStrAppendK(&seqstr, *cp);
	cp++;
    }

    if(!*cp)
	ret = ajTrue;

    if(!ret)
    {
	featWarn("bad /translation value '%S'",   *pval);
    }

    return ret;
}




/* @funcstatic featTagSpecialAllEstimatedlength *******************************
**
** Tests a string as a valid internal (EMBL) feature /estimated_length tag
**
** The format is a positive integer, or unknown (unquoted)
**
** @param [u] pval [AjPStr*] parameter value
** @return [AjBool] ajTrue for a valid value, possibly corrected
**                  ajFalse if invalid, to be converted to default (note) type
** @@
******************************************************************************/

static AjBool featTagSpecialAllEstimatedlength(AjPStr* pval)
{
    AjPStr numstr = NULL;
    AjBool saveit = ajFalse;
    AjBool ret = ajFalse;

    if(!featRegSpecialEstlen)
	featRegSpecialEstlen = ajRegCompC("^(unknown|[1-9][0-9]*)$");

    if(ajRegExec(featRegSpecialEstlen, *pval))
    {
	ret = ajTrue;

	if(saveit)
	    ajRegSubI(featRegSpecialEstlen, 1, &numstr);
    }

    if(!ret)
    {
	featWarn("bad /estimated_length value '%S' corrected to 'unknown'",
	       *pval);
	ajStrAssignC(pval, "unknown");
    }

    ajStrDel(&numstr);

    return ret;
}




/* @funcstatic featTagSpecialAllCompare ***************************************
**
** Tests a string as a valid internal (EMBL) feature /compare tag
**
** The format is a sequence versin
**
** @param  [r] val [const AjPStr] parameter value
** @return [AjBool] ajTrue for a valid value, possibly corrected
**                  ajFalse if invalid, to be converted to default (note) type
** @@
******************************************************************************/

static AjBool featTagSpecialAllCompare(const AjPStr val)
{
    AjPStr numstr = NULL;
    AjBool saveit = ajFalse;
    AjBool ret = ajFalse;

/* value is a sequence version accnumber.number */
    if(!featRegSpecialCompare)
	featRegSpecialCompare = ajRegCompC("^([A-Z]+[0-9]+([.][1-9][0-9]*)?)$");

    if(ajRegExec(featRegSpecialCompare, val))
    {
	ret = ajTrue;
	if(saveit)
	    ajRegSubI(featRegSpecialCompare, 1, &numstr);
    }

    if(!ret)
    {
	featWarn("bad /compare value '%S'", val);
    }
    ajStrDel(&numstr);

    return ret;
}




/* @funcstatic featTagSpecialAllMobile ****************************************
**
** Tests a string as a valid internal (EMBL) feature /mobile_element tag
**
** The format is a known type and optional :name
**
** @param  [r] val [const AjPStr] parameter value
** @return [AjBool] ajTrue for a valid value, possibly corrected
**                  ajFalse if invalid, to be converted to default (note) type
** @@
******************************************************************************/

static AjBool featTagSpecialAllMobile(const AjPStr val)
{
    AjPStr typstr = NULL;
    AjPStr namstr = NULL;
    AjBool saveit = ajFalse;
    AjBool ret = ajFalse;
    ajuint i;

    const char* elements[] =
    {
	"transposon", "retrotransposon", "integron",
	"insertion sequence", "non-LTR retrotransposon",
	"SINE", "MITE", "LINE", "other", NULL
    };

    /* value is a type or type:name */
    if(!featRegSpecialMobile)
	featRegSpecialMobile = ajRegCompC("^([^:]+)(:(.*))?$");

    if(ajRegExec(featRegSpecialMobile, val))
    {
	ajRegSubI(featRegSpecialMobile, 1, &typstr);

	for(i=0;elements[i];i++)
	    if(ajStrMatchC(typstr, elements[i])) break;

	if(elements[i])
	    ret = ajTrue;

	if(saveit)
	{
	    ajRegSubI(featRegSpecialMobile, 3, &namstr);
	}
    }

    if(!ret)
    {
	featWarn("bad /mobile_element value '%S'", val);
    }
    ajStrDel(&typstr);
    ajStrDel(&namstr);

    return ret;
}




/* @funcstatic featTagSpecialAllNcrnaclass *************************************
**
** Tests a string as a valid internal (EMBL) feature /ncRNA_class tag
**
** The format is a known type and optional :name
**
** value is a term taken from the INSDC controlled vocabulary for ncRNA classes
** http://www.insdc.org/page.php?page=rna_vocab
** or http://www.ebi.ac.uk/embl/Documentation/ncRNA_class.html
**
** @param  [r] val [const AjPStr] parameter value
** @return [AjBool] ajTrue for a valid value, possibly corrected
**                  ajFalse if invalid, to be converted to default (note) type
** @@
******************************************************************************/

static AjBool featTagSpecialAllNcrnaclass(const AjPStr val)
{
    AjPStr typstr = NULL;
    AjPStr namstr = NULL;
    AjBool ret = ajFalse;
    ajuint i;
    
    const char* classes[] =
    {
	"antisense_RNA", "autocatalytically_spliced_intron",
        "ribozyme", "hammerhead_ribozyme",
        "RNase_P_RNA",
	"RNase_MRP_RNA", "telomerase_RNA", "guide_RNA", "rasiRNA",
	"scRNA", "siRNA", "miRNA", "piRNA",
	"snoRNA", "snRNA", "SRP_RNA", "vault_RNA", "Y_RNA",
        "other",
        NULL
    };

    for(i=0;classes[i];i++)
        if(ajStrMatchC(val, classes[i])) break;

    if(classes[i])
        ret = ajTrue;
    
    if(!ret)
    {
	featWarn("bad /ncRNA_class value '%S'", val);
    }

    ajStrDel(&typstr);
    ajStrDel(&namstr);

    return ret;
}




/* @funcstatic featTagQuoteEmbl ***********************************************
**
** Internal quotes converted to two double quotes
** for EMBL feature tag values.
**
** Quotes added around the outside.
**
** @param  [u] pval [AjPStr*] parameter value
** @return [void]
** @@
******************************************************************************/

static void featTagQuoteEmbl(AjPStr* pval)
{
    const char* cp;

    /* ajDebug("featTagQuoteEmbl '%S'\n", *pval); */

    if(ajStrFindAnyK(*pval, '"') < 0)
    {
	ajStrFmtQuote(pval);
    }
    else /* double up internal quotes */
    {
	ajStrAssignS(&featTmpStr, *pval);
	ajStrAssignC(pval, "\"");

	cp = ajStrGetPtr(featTmpStr);

	while(*cp)
	{
	    if(*cp == '"')
		ajStrAppendK(pval, '"');

	    ajStrAppendK(pval, *cp++);
	}

	ajStrAppendK(pval, '"');
    }

    ajStrDelStatic(&featTmpStr);

    return;
}




/* @funcstatic featTagQuoteGff2 ***********************************************
**
** Internal quotes converted to escaped quotes
** for EMBL feature tag values
**
** @param  [u] pval [AjPStr*] parameter value
** @return [void]
** @@
******************************************************************************/

static void featTagQuoteGff2(AjPStr* pval)
{

    if(!featRegQuote)
	featRegQuote = ajRegCompC("([^\"]*)\"");

    /* ajDebug("featTagQuoteGff2 '%S'\n", *pval); */

    ajStrAssignS(&featValCopy, *pval);
    ajStrDelStatic(pval);

    while(ajRegExec(featRegQuote, featValCopy))
    {
	ajRegSubI(featRegQuote, 1, &featSubStr);
	/* ajDebug("part '%S'\n", substr); */
	ajStrAppendS(pval, featSubStr);
	ajStrAppendC(pval, "\\\"");
	ajRegPost(featRegQuote, &featTmpStr);
	ajStrAssignS(&featValCopy, featTmpStr);
    }

    /* ajDebug("rest '%S'\n", featValCopy); */
    ajStrAppendS(pval, featValCopy);
    ajStrFmtQuote(pval);

    return;
}




/* @funcstatic featTagQuoteGff3 ************************************************
**
** Internal quotes converted to escaped quotes
** for EMBL feature tag values
**
** @param  [u] pval [AjPStr*] parameter value
** @return [void]
** @@
******************************************************************************/

static void featTagQuoteGff3(AjPStr* pval)
{

    if(!featRegQuote)
	featRegQuote = ajRegCompC("([^\"]*)\"");

    /* ajDebug("featTagQuoteGff3 '%S'\n", *pval); */

    ajStrAssignS(&featValCopy, *pval);
    ajStrDelStatic(pval);

    while(ajRegExec(featRegQuote, featValCopy))
    {
	ajRegSubI(featRegQuote, 1, &featSubStr);
	/* ajDebug("part '%S'\n", substr); */
	ajStrAppendS(pval, featSubStr);
	ajStrAppendC(pval, "\\\"");
	ajRegPost(featRegQuote, &featTmpStr);
	ajStrAssignS(&featValCopy, featTmpStr);
    }

    /* ajDebug("rest '%S'\n", featValCopy); */
    ajStrAppendS(pval, featValCopy);
    ajStrFmtQuote(pval);

    return;
}




/* @funcstatic featLocEmblWrapC ***********************************************
**
** Splits EMBL feature location at the last possible comma
** and adds the appropriate prefix (e.g. the EMBL FT line type)
**
** @param  [u] Ploc [AjPStr*] location as a string
** @param  [r] margin [ajuint] Right margin
** @param  [r] prefix [const char*] Left margin prefix string
** @param  [r] preftyp [const char*] Left margin prefix string for first line
**                            (includes the feature key)
** @param [w] retstr [AjPStr*] string with formatted value.
** @return [void]
** @@
******************************************************************************/

static void featLocEmblWrapC(AjPStr *Ploc, ajuint margin,
			     const char* prefix, const char* preftyp,
			     AjPStr* retstr)
{
    ajint left  = 0;
    ajint width = 0;
    ajint len   = 0;
    ajint i;
    ajint j;
    ajint k;
    ajint last;

    left = strlen(prefix);
    width = margin - left;	    /* available width for printing */

    ajStrRemoveWhite(Ploc);	     /* no white space in locations */
    len = ajStrGetLen(*Ploc);

    k = width;			/* for safety - will be set in time */

    /* ajDebug("featLocEmblWrapC %d <%d> '%S'\n", len, width, *Ploc); */
    for(i=0; i < len; i+= k)
    {
	last = i + width - 1;

	/* ajDebug("try %d to %d (len %d)\n", i, last, len); */

	if((last+1) >= len)		/* no need to split */
	{
	    ajStrAssignSubS(&featTmpStr, *Ploc, i, len-1);
	    /* ajDebug("last %d >= len %d\n", last, len); */
	    j = 0;
	}
	else
	{
	    ajStrAssignSubS(&featTmpStr, *Ploc, i, last); /* save max string */
	    j = ajStrFindlastC(featTmpStr, ","); /* last comma in featTmpStr */
	    /* ajDebug("comma at %d\n", j); */
	}

	if(j < 1)			/* no comma found */
	{
	    /* ajDebug("no comma j=%d k=%d\n", j, ajStrGetLen(featTmpStr)); */
	    j = ajStrGetLen(featTmpStr);
	    k = j;
	}
	else
	{				/* print up to last comma */
	    j++;
	    k = j;			/* start after the comma */
	}
	/* ajDebug("%d +%d k=%d featTmpStr: '%.*S'\n",
                   i, j, k, j, featTmpStr); */
	if(!i)
	    ajFmtPrintAppS(retstr, "%s%.*S\n", preftyp,j, featTmpStr);
	else
	    ajFmtPrintAppS(retstr, "%s%.*S\n", prefix,j, featTmpStr);
    }

    return;
}




/* @funcstatic featTagEmblWrapC ***********************************************
**
** Splits feature table output at the last possible space (or
** the last column if there are no spaces) and adds the appropriate
** prefix (e.g. the EMBL FT line type)
**
** @param  [u] pval [AjPStr*] parameter value
** @param  [r] margin [ajuint] Right margin
** @param  [r] prefix [const char*] Left margin prefix string
** @param  [w] retstr [AjPStr*] string with formatted value.
** @return [void]
** @@
******************************************************************************/

static void featTagEmblWrapC(AjPStr *pval, ajuint margin, const char* prefix,
			     AjPStr* retstr)
{
    ajint left  = 0;
    ajint width = 0;
    ajint len   = 0;
    ajint i;
    ajint j;
    ajint k;
    ajint last;

    left = strlen(prefix);
    width = margin - left;	    /* available width for printing */

    ajStrRemoveWhiteExcess(pval);	/* single spaces only */
    len = ajStrGetLen(*pval);

    k = width;			/* for safety - will be set in time */

    /* ajDebug("featTagEmblWrapC %d <%d> '%S'\n", len, width, *pval); */

    for(i=0; i < len; i+= k)
    {
	last = i + width - 1;

	/* ajDebug("try %d to %d (len %d)\n", i, last, len); */

	if((last+1) >= len)		/* no need to split */
	{
	    ajStrAssignSubS(&featTmpStr, *pval, i, len-1);
	    /* ajDebug("last %d >= len %d\n", last, len); */
	    j = 0;
	}
	else if(ajStrGetCharPos(*pval, (last+1)) == ' ') /* split at max width */
	{
	    ajStrAssignSubS(&featTmpStr, *pval, i, last);
	    j = last + 1 - i;
	}
	else
	{
	    ajStrAssignSubS(&featTmpStr, *pval, i, last); /* save max string */
	    j = ajStrFindlastC(featTmpStr, " "); /* last space in featTmpStr */
	    /* ajDebug("space at %d\n", j); */
	}

	if(j < 1)			/* no space found */
	{
	    j = ajStrGetLen(featTmpStr);
	    k = j;
	}
	else				/* print up to last space */
	{
	    k = j + 1;			/* start after the space */
	}

	/* ajDebug("%d +%d '%.*S'\n", i, j, j, featTmpStr); */
	ajFmtPrintAppS(retstr, "%s%.*S\n", prefix,j, featTmpStr);
    }

    return;
}




/* @funcstatic featTagSwissWrapC **********************************************
**
** Splits feature table output at the last possible space (or
** the last column if there are no spaces) and adds the appropriate
** prefix (e.g. the SwissProt FT line type)
**
** @param  [u] pval [AjPStr*] parameter value
** @param  [r] margin [ajuint] Right margin
** @param  [r] prefix [const char*] Left margin prefix string
** @param  [w] retstr [AjPStr*] string with formatted value.
** @return [void]
** @@
******************************************************************************/

static void featTagSwissWrapC(AjPStr *pval, ajuint margin, const char* prefix,
			      AjPStr* retstr)
{
    ajuint left  = 0;
    ajuint width = 0;
    ajuint len   = 0;
    ajuint i;
    ajuint j;
    ajuint k;
    ajint iftid = -1;
    AjBool isftidstart = ajFalse;
    AjBool isftid = ajFalse;

    AjPStr valstr = NULL;
    ajuint last;
    
    left = strlen(prefix);
    width = margin - left;	/* available width for printing */
    
    k = width; /* will be reset in the loop */
    
    ajDebug("featTagSwissWrapC %d <%d> '%S'\n",
       ajStrGetLen(*pval), width, *pval);
    
    if(ajStrGetLen(*pval) <= left)	/* no need to wrap */
    {
	ajStrAssignS(retstr, *pval);
	ajStrAppendK(retstr, '\n');
	ajDebug("simple '%S'\n", *retstr);

	return;
    }   

    ajStrAssignSubS(retstr, *pval, 0, left-1);
    ajStrAssignSubS(&valstr, *pval, left, -1);
    len = ajStrGetLen(valstr);
    ajDebug("rest '%S'\n", valstr);
    
    if(ajStrPrefixC(valstr, "/FTId="))
    {
        ajStrTrimEndC(retstr, " ");
        ajStrAppendK(retstr, '\n');
        isftidstart = ajTrue;
    }
    

    for(i=0; i < len; i+= k)
    {
	last = i + width - 1;

        ajStrAssignSubS(&featTmpStr, valstr, i, len-1);
        iftid = ajStrFindC(featTmpStr, " /FTId=");
        isftid = ajFalse;
	ajDebug("try %d to %d (len %d) iftid:%d\n", i, last, len, iftid);

        if(iftid >= 0 && iftid+(ajint)i <= (ajint)last)
        {
            if(iftid)
            {
                isftid = ajTrue;
                ajStrAssignSubS(&featTmpStr, valstr, i, i+iftid-1);
                j = iftid;
            }
            else
            {
                ajStrAssignSubS(&featTmpStr, valstr, i, i+iftid-2);
                j = 0;
            }
            
	    ajDebug("' /FTId=' found iftid:%d isftid:%B, j:%d\n",
                    iftid, isftid, j);
        }
	else if((last+1) >= len)		/* no need to split */
	{
	    ajStrAssignSubS(&featTmpStr, valstr, i, len-1);
	    ajDebug("last %d >= len %d\n", last, len);
	    j = 0;
	}
	else if(ajStrGetCharPos(valstr, (last+1)) == ' ') /* split at max width */
	{
	    ajStrAssignSubS(&featTmpStr, valstr, i, last);
	    j = width;
	    ajDebug("split at max width last+1 %d\n", last+1);
	}
	else
	{
	    ajStrAssignSubS(&featTmpStr, valstr, i, last); /* save max string */
	    j = ajStrFindlastC(featTmpStr, " "); /* last space in featTmpStr */
	    ajDebug("space at %d\n", j);
	}

	if(j < 1)			/* no space found */
	{
	    j = ajStrGetLen(featTmpStr);
	    k = j;
	}
	else				/* print up to last space */
	{
	    k = j + 1;			/* start after the space */
	}
	ajDebug("%d +%d (%d) '%.*S'\n", i, j, k, j, featTmpStr);

	if(i || isftidstart)
	    ajFmtPrintAppS(retstr, "%s%.*S\n", prefix,j, featTmpStr);
	else
	    ajFmtPrintAppS(retstr, "%.*S\n", j, featTmpStr);
    }

    ajStrDel(&valstr);

    return;
}




/* @funcstatic featTagAllLimit ************************************************
**
** Tests a string as a valid feature value, given a
** list of possible values.
**
** @param  [u] pval [AjPStr*] parameter value
** @param  [r] values [const AjPStr] comma delimited list of values
** @return [AjBool] ajTrue for a valid value, possibly corrected
**                  ajFalse if invalid, to be converted to default (note) type
** @@
******************************************************************************/

static AjBool featTagAllLimit(AjPStr* pval, const AjPStr values)
{
    AjPStr limstr = NULL;
    const char* cp = NULL;
    const char* cq = NULL;
    ajint i=0;
    AjBool ret = ajFalse;

    /* ajDebug("featTagAllLimit '%S' '%S'\n", *pval, values); */

    ajStrAssignResC(&limstr, 16, "");
    cp = ajStrGetPtr(values);
    cq = cp;

    while(*cp)
    {
	if(*cp == ',')
	{
	    ajStrAssignLenC(&limstr, cq, i);

	    if(ajStrMatchCaseS(*pval, limstr))
	    {
		if(!ajStrMatchS(*pval, limstr))
		{
		    ajStrAssignS(pval, limstr);
		}
		ret = ajTrue;
		break;
	    }

	    ajStrAssignClear(&limstr);
	    cq = cp+1;
	    i=0;
	}
	else
	    i++;
	cp++;
    }

    ajStrDel(&limstr);

    return ret;
}




/* @funcstatic featTagEmblDefault *********************************************
**
** Give up, and generate a default EMBL/Genbank feature tag
**
** @param  [w] pout [AjPStr*] Output string
** @param  [r] tag [const AjPStr] original tag name
** @param  [u] pval [AjPStr*] parameter value
** @return [void]
** @@
******************************************************************************/

static void featTagEmblDefault(AjPStr* pout, const AjPStr tag, AjPStr* pval)
{
    /*ajDebug("featTagEmblDefault '%S' '%S'\n", tag, *pval);*/

    featTagQuoteEmbl(pval);
    ajFmtPrintS(pout, "/note=\"%S: %S\"", tag, *pval);

    return;
}




/* @funcstatic featTagGff2Default **********************************************
**
** Give up, and generate a default GFF feature tag
**
** @param  [w] pout [AjPStr*] Output string
** @param  [r] tag [const AjPStr] original tag name
** @param  [u] pval [AjPStr*] parameter value
** @return [void]
** @@
******************************************************************************/

static void featTagGff2Default(AjPStr* pout, const AjPStr tag, AjPStr* pval)
{
    /*ajDebug("featTagGff2Default '%S' '%S'\n", tag, *pval);*/

    featTagQuoteGff2(pval);
    ajFmtPrintS(pout, "note \"%S: %S\"", tag, *pval);

    return;
}




/* @funcstatic featTagGff3Default **********************************************
**
** Give up, and generate a default GFF3 feature tag
**
** @param  [w] pout [AjPStr*] Output string
** @param  [r] tag [const AjPStr] original tag name
** @param  [u] pval [AjPStr*] parameter value
** @return [void]
** @@
******************************************************************************/

static void featTagGff3Default(AjPStr* pout, const AjPStr tag, AjPStr* pval)
{
    /*ajDebug("featTagGff3Default '%S' '%S'\n", tag, *pval);*/

    featTagQuoteGff3(pval);
    ajFmtPrintS(pout, "note \"%S: %S\"", tag, *pval);

    return;
}




/* @funcstatic featTagDasgffDefault ********************************************
**
** Give up, and generate a default DASGFF feature tag
**
** @param  [w] pout [AjPStr*] Output string
** @param  [r] tag [const AjPStr] original tag name
** @param  [u] pval [AjPStr*] parameter value
** @return [void]
** @@
******************************************************************************/

static void featTagDasgffDefault(AjPStr* pout, const AjPStr tag, AjPStr* pval)
{

    /*ajDebug("featTagDasgffDefault '%S' '%S'\n", tag, *pval);*/


    featTagQuoteGff3(pval);

    ajFmtPrintS(pout, "%S:%S", tag, *pval);
    
    return;
}




/* @funcstatic featTagSpecial *************************************************
**
** Special processing for known internal (EMBL) tags
**
** @param  [u] pval [AjPStr*] parameter value
** @param  [r] tag [const AjPStr] original tag name
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool featTagSpecial(AjPStr* pval, const AjPStr tag)
{
    /*ajDebug("featTagSpecial '%S'\n", tag);*/

    if(ajStrMatchC(tag, "anticodon"))
	return featTagSpecialAllAnticodon(*pval);

    else if(ajStrMatchC(tag, "bio_material") ||
            ajStrMatchC(tag, "culture_collection") ||
            ajStrMatchC(tag, "specimen_voucher"))
	return featTagSpecialAllBiomaterial(*pval);

    else if(ajStrMatchC(tag, "citation"))
	return featTagSpecialAllCitation(*pval);

    else if(ajStrMatchC(tag, "codon"))
	return featTagSpecialAllCodon(pval);

    else if(ajStrMatchC(tag, "collection_date"))
	return featTagSpecialAllCollectiondate(*pval);

    else if(ajStrMatchC(tag, "compare"))
	return featTagSpecialAllCompare(*pval);

    else if(ajStrMatchC(tag, "cons_splice"))
	return featTagSpecialAllConssplice(pval);

    else if(ajStrMatchC(tag, "db_xref"))
	return featTagSpecialAllDbxref(*pval);

    else if(ajStrMatchC(tag, "estimated_length"))
	return featTagSpecialAllEstimatedlength(pval);

    else if(ajStrMatchC(tag, "inference"))
	return featTagSpecialAllInference(*pval);

    else if(ajStrMatchC(tag, "lat_lon"))
	return featTagSpecialAllLatlon(*pval);

    else if(ajStrMatchC(tag, "mobile_element"))
	return featTagSpecialAllMobile(*pval);

    else if(ajStrMatchC(tag, "ncRNA_class"))
	return featTagSpecialAllNcrnaclass(*pval);

    else if(ajStrMatchC(tag, "PCR_primers"))
	return featTagSpecialAllPcrprimers(*pval);

    else if(ajStrMatchC(tag, "protein_id"))
	return featTagSpecialAllProteinid(*pval);

    else if(ajStrMatchC(tag, "replace"))
	return featTagSpecialAllReplace(pval);

    else if(ajStrMatchC(tag, "rpt_unit"))
	return featTagSpecialAllRptunit(*pval);

    else if(ajStrMatchC(tag, "rpt_unit_range") ||
            ajStrMatchC(tag, "tag_peptide"))
	return featTagSpecialAllRange(*pval);

    else if(ajStrMatchC(tag, "rpt_unit_seq"))
	return featTagSpecialAllRptunitseq(pval);

    else if(ajStrMatchC(tag, "transl_except"))
	return featTagSpecialAllTranslexcept(*pval);

    else if(ajStrMatchC(tag, "translation"))
	return featTagSpecialAllTranslation(pval);

    /*ajDebug("Unrecognised special EMBL feature tag '%S'\n", tag);*/
    featWarn("Unrecognised special EMBL feature tag '%S'",   tag);

    return ajFalse;
}




/* @funcstatic featTagGffSpecial **********************************************
**
** Special processing for known GFF tags
**
** This function will be very similar to featTagSpecial, with scope
** for future GFF-specific extensions
**
** @param  [u] pval [AjPStr*] tag value
** @param  [r] tag [const AjPStr] original tag name
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool featTagGffSpecial(AjPStr* pval, const AjPStr tag)
{
    /*ajDebug("featTagGffSpecial '%S' '%S'\n", tag, *pval);*/

    if(ajStrMatchC(tag, "anticodon"))
	return featTagSpecialAllAnticodon(*pval);

    else if(ajStrMatchC(tag, "bio_material") ||
            ajStrMatchC(tag, "culture_collection") ||
            ajStrMatchC(tag, "specimen_voucher"))
	return featTagSpecialAllBiomaterial(*pval);

    else if(ajStrMatchC(tag, "citation"))
	return featTagSpecialAllCitation(*pval);

    else if(ajStrMatchC(tag, "codon"))
	return featTagSpecialAllCodon(pval);

    else if(ajStrMatchC(tag, "collection_date"))
	return featTagSpecialAllCollectiondate(*pval);

    else if(ajStrMatchC(tag, "compare"))
	return featTagSpecialAllCompare(*pval);

    else if(ajStrMatchC(tag, "cons_splice"))
	return featTagSpecialAllConssplice(pval);

    else if(ajStrMatchC(tag, "db_xref"))
	return featTagSpecialAllDbxref(*pval);

    else if(ajStrMatchC(tag, "estimated_length"))
	return featTagSpecialAllEstimatedlength(pval);

     else if(ajStrMatchC(tag, "inference"))
	return featTagSpecialAllInference(*pval);

    else if(ajStrMatchC(tag, "lat_lon"))
	return featTagSpecialAllLatlon(*pval);

    else if(ajStrMatchC(tag, "mobile_element"))
	return featTagSpecialAllMobile(*pval);

    else if(ajStrMatchC(tag, "PCR_primers"))
	return featTagSpecialAllPcrprimers(*pval);

    else if(ajStrMatchC(tag, "protein_id"))
	return featTagSpecialAllProteinid(*pval);

    else if(ajStrMatchC(tag, "replace"))
	return featTagSpecialAllReplace(pval);

    else if(ajStrMatchC(tag, "rpt_unit"))
	return featTagSpecialAllRptunit(*pval);

    else if(ajStrMatchC(tag, "rpt_unit_range") ||
            ajStrMatchC(tag, "tag_peptide"))
	return featTagSpecialAllRange(*pval);

    else if(ajStrMatchC(tag, "rpt_unit_seq"))
	return featTagSpecialAllRptunitseq(pval);

    else if(ajStrMatchC(tag, "transl_except"))
	return featTagSpecialAllTranslexcept(*pval);

    else if(ajStrMatchC(tag, "translation"))
	return featTagSpecialAllTranslation(pval);

    /*ajDebug("Unrecognised special GFF feature tag '%S'\n", tag);*/
    featWarn("Unrecognised special GFF feature tag '%S'",   tag);

    return ajFalse;
}




/* @funcstatic featTagGff3Special *********************************************
**
** Special processing for known GFF3 tags
**
** This function will be very similar to featTagSpecial, with scope
** for future GFF3-specific extensions
**
** @param  [u] pval [AjPStr*] tag value
** @param  [r] tag [const AjPStr] original tag name
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool featTagGff3Special(AjPStr* pval, const AjPStr tag)
{
    /*ajDebug("featTagGff3Special '%S' '%S'\n", tag, *pval);*/

    if(ajStrMatchC(tag, "anticodon"))
	return featTagSpecialAllAnticodon(*pval);

    else if(ajStrMatchC(tag, "bio_material") ||
            ajStrMatchC(tag, "culture_collection") ||
            ajStrMatchC(tag, "specimen_voucher"))
	return featTagSpecialAllBiomaterial(*pval);

    else if(ajStrMatchC(tag, "citation"))
	return featTagSpecialAllCitation(*pval);

    else if(ajStrMatchC(tag, "codon"))
	return featTagSpecialAllCodon(pval);

    else if(ajStrMatchC(tag, "collection_date"))
	return featTagSpecialAllCollectiondate(*pval);

    else if(ajStrMatchC(tag, "compare"))
	return featTagSpecialAllCompare(*pval);

    else if(ajStrMatchC(tag, "cons_splice"))
	return featTagSpecialAllConssplice(pval);

    else if(ajStrMatchC(tag, "db_xref"))
	return featTagSpecialAllDbxref(*pval);

    else if(ajStrMatchC(tag, "estimated_length"))
	return featTagSpecialAllEstimatedlength(pval);

    else if(ajStrMatchC(tag, "inference"))
	return featTagSpecialAllInference(*pval);

    else if(ajStrMatchC(tag, "lat_lon"))
	return featTagSpecialAllLatlon(*pval);

    else if(ajStrMatchC(tag, "mobile_element"))
	return featTagSpecialAllMobile(*pval);

    else if(ajStrMatchC(tag, "ncRNA_class"))
	return featTagSpecialAllNcrnaclass(*pval);

    else if(ajStrMatchC(tag, "PCR_primers"))
	return featTagSpecialAllPcrprimers(*pval);

    else if(ajStrMatchC(tag, "protein_id"))
	return featTagSpecialAllProteinid(*pval);

    else if(ajStrMatchC(tag, "replace"))
	return featTagSpecialAllReplace(pval);

    else if(ajStrMatchC(tag, "rpt_unit"))
	return featTagSpecialAllRptunit(*pval);

    else if(ajStrMatchC(tag, "rpt_unit_range") ||
            ajStrMatchC(tag, "tag_peptide"))
	return featTagSpecialAllRange(*pval);

    else if(ajStrMatchC(tag, "rpt_unit_seq"))
	return featTagSpecialAllRptunitseq(pval);

    else if(ajStrMatchC(tag, "transl_except"))
	return featTagSpecialAllTranslexcept(*pval);

    else if(ajStrMatchC(tag, "translation"))
	return featTagSpecialAllTranslation(pval);

    /*ajDebug("Unrecognised special GFF feature tag '%S'\n", tag);*/
    featWarn("Unrecognised special GFF feature tag '%S'",   tag);

    return ajFalse;
}




/* @funcstatic featDumpEmbl ***************************************************
**
** Write details of single feature to file in EMBL/GenBank/DDBJ format
**
** @param [r] feat     [const AjPFeature] Feature
** @param [r] location [const AjPStr] location list
** @param [u] file     [AjPFile] Output file
** @param [r] Seqid    [const AjPStr] Sequence ID
** @param [r] IsEmbl   [AjBool] ajTrue if writing EMBL format (FT prefix)
** @return [void]
** @@
******************************************************************************/

static void featDumpEmbl(const AjPFeature feat, const AjPStr location,
			 AjPFile file, const AjPStr Seqid, AjBool IsEmbl)
{
    AjIList iter   = NULL;
    ajint i        = 0;
    FeatPTagval tv = NULL;
    const AjPStr tmptyp  = NULL;		/* these come from AjPTable */
    const AjPStr tmptag  = NULL;		/* so please, please */
    /* don't delete them */
    AjPStr tmplim = NULL;
    AjBool knowntag = ajTrue;
    const char* cp;
    AjPStr wrapstr           = NULL;
    AjPStr preftyploc = NULL;
    AjPStr preftyptag = NULL;
    AjPStr tmploc     = NULL;
    
    /* ajDebug("featDumpEmbl Start\n"); */
    
    /* print the location */
    
    ajStrAssignS(&tmploc, location);
    tmptyp = featTableTypeExternal(feat->Type, FeatTypeTableEmbl);
    
    if(IsEmbl)
    {
	ajFmtPrintS(&preftyploc, "%s   %-15.15S ", "FT", tmptyp);
	ajFmtPrintS(&preftyptag, "%s                   ", "FT");
    }
    else
    {
	ajFmtPrintS(&preftyploc, "%s   %-15.15S ", "  ", tmptyp);
	ajFmtPrintS(&preftyptag, "%s                   ", "  ");
    }
    
    if(IsEmbl)
	featLocEmblWrapC(&tmploc, 80,
			 ajStrGetPtr(preftyptag),
			 ajStrGetPtr(preftyploc), &wrapstr);
    else
	featLocEmblWrapC(&tmploc, 79,
			 ajStrGetPtr(preftyptag),
			 ajStrGetPtr(preftyploc), &wrapstr);

    ajFmtPrintF(file, "%S", wrapstr);
    ajStrDel(&wrapstr);
    
    /* print the qualifiers */
    
    iter = ajListIterNewread(feat->Tags);

    while(!ajListIterDone(iter))
    {
	tv = ajListIterGet(iter);
	++i;
	tmptag = featTableTag(tv->Tag, FeatTagsTableEmbl, &knowntag);
	featTagFmt(tmptag, FeatTagsTableEmbl, &featFmtTmp);
	/* ajDebug(" %3d  %S value: '%S'\n", i, tv->Tag, tv->Value); */
	/* ajDebug(" %3d  %S format: '%S'\n", i, tmptag, featFmtTmp); */
	ajFmtPrintS(&featOutStr, "/%S", tmptag);

	if(tv->Value)
	{
	    ajStrAssignS(&featValTmp, tv->Value);
	    cp = ajStrGetPtr(featFmtTmp);

	    switch(CASE2(cp[0], cp[1]))
	    {
	    case CASE2('L','I') :		/* limited */
		/* ajDebug("case limited\n"); */
		featTagLimit(tmptag, FeatTagsTableEmbl, &tmplim);
		featTagAllLimit(&featValTmp, tmplim);
		ajFmtPrintAppS(&featOutStr, "=%S\n", featValTmp);
		ajStrDel(&tmplim);
		break;
	    case CASE2('Q', 'L') :	/* limited, escape quotes */
		/* ajDebug("case qlimited\n"); */
		featTagLimit(tmptag, FeatTagsTableEmbl, &tmplim);
		featTagAllLimit(&featValTmp, tmplim);
		featTagQuoteEmbl(&featValTmp);
		ajFmtPrintAppS(&featOutStr, "=%S\n", featValTmp);
		ajStrDel(&tmplim);
		break;
	    case CASE2('Q', 'S') :	/* special regexp, quoted */
		/* ajDebug("case qspecial\n"); */
		if(!featTagSpecial(&featValTmp, tmptag))
		{
		    featWarn("%S: Bad special tag value", Seqid);
		    featTagEmblDefault(&featOutStr, tmptag, &featValTmp);
		}
		else
		{
		    featTagQuoteEmbl(&featValTmp);
		    ajFmtPrintAppS(&featOutStr, "=%S\n", featValTmp);
		}
		break;
	    case CASE2('S','P') :	/* special regexp */
		/* ajDebug("case special\n"); */
		if(!featTagSpecial(&featValTmp, tmptag))
		{
		    featWarn("%S: Bad special tag value", Seqid);
		    featTagEmblDefault(&featOutStr, tmptag, &featValTmp);
		}
		else
		    ajFmtPrintAppS(&featOutStr, "=%S\n", featValTmp);

		break;
	    case CASE2('T','E') :     /* no space, no quotes, wrap at margin */
		/* ajDebug("case text\n"); */
		ajStrRemoveWhite(&featValTmp);
		ajFmtPrintAppS(&featOutStr, "=%S\n", featValTmp);
		break;
	    case CASE2('V','O') :	     /* no value, so an error here */
		/*ajDebug("case void\n");*/
		break;
	    case CASE2('Q','T') :	   /* escape quotes, wrap at space */
		/* ajDebug("case qtext\n"); */
		featTagQuoteEmbl(&featValTmp);
		ajFmtPrintAppS(&featOutStr, "=%S\n", featValTmp);
		break;
	    case CASE2('Q','W') :	   /* escape quotes, remove space */
		/* ajDebug("case qword\n"); */
		featTagQuoteEmbl(&featValTmp);
		ajStrRemoveWhite(&featValTmp);	/* no white space needed */
		ajFmtPrintAppS(&featOutStr, "=%S\n", featValTmp);
		break;
	    default:
		featWarn("Unknown EMBL feature tag type '%S' for '%S'",
		       featFmtTmp, tmptag);
	    }
	}
	else
	{
	    /*ajDebug("no value, hope it is void: '%S'\n", featFmtTmp);*/
	}

	if(IsEmbl)
	    featTagEmblWrapC(&featOutStr, 80,
			     ajStrGetPtr(preftyptag), &wrapstr);
	else
	    featTagEmblWrapC(&featOutStr, 79,
			     ajStrGetPtr(preftyptag), &wrapstr);

	ajFmtPrintF(file, "%S", wrapstr);
	ajStrDel(&wrapstr);
    }
    
    /* ajDebug("featDumpEmbl Done %d tags\n", i); */
    
    ajListIterDel(&iter);
    ajStrDel(&preftyploc);
    ajStrDel(&preftyptag);
    ajStrDel(&tmploc);
    ajStrDel(&tmplim);

    return;
}




/* @funcstatic featDumpRefseq *************************************************
**
** Write details of single feature to file in Refseq format
**
** @param [r] feat     [const AjPFeature] Feature
** @param [r] location [const AjPStr] location list
** @param [u] file     [AjPFile] Output file
** @param [r] Seqid    [const AjPStr] Sequence ID
** @return [void]
** @@
******************************************************************************/

static void featDumpRefseq(const AjPFeature feat, const AjPStr location,
                           AjPFile file, const AjPStr Seqid)
{
    AjIList iter   = NULL;
    ajint i        = 0;
    FeatPTagval tv = NULL;
    const AjPStr tmptyp  = NULL;		/* these come from AjPTable */
    const AjPStr tmptag  = NULL;		/* so please, please */
    /* don't delete them */
    AjPStr tmplim = NULL;
    AjBool knowntag = ajTrue;
    const char* cp;
    AjPStr wrapstr           = NULL;
    AjPStr preftyploc = NULL;
    AjPStr preftyptag = NULL;
    AjPStr tmploc     = NULL;
    
    /* ajDebug("featDumpRefseq Start\n"); */
    
    /* print the location */
    
    ajStrAssignS(&tmploc, location);
    tmptyp = featTableTypeExternal(feat->Type, FeatTypeTableEmbl);
    
    ajFmtPrintS(&preftyploc, "%s   %-15.15S ", "  ", tmptyp);
    ajFmtPrintS(&preftyptag, "%s                   ", "  ");

    
    featLocEmblWrapC(&tmploc, 79,
                     ajStrGetPtr(preftyptag),
                     ajStrGetPtr(preftyploc), &wrapstr);

    ajFmtPrintF(file, "%S", wrapstr);
    ajStrDel(&wrapstr);
    
    /* print the qualifiers */
    
    iter = ajListIterNewread(feat->Tags);

    while(!ajListIterDone(iter))
    {
	tv = ajListIterGet(iter);
	++i;
	tmptag = featTableTag(tv->Tag, FeatTagsTableEmbl, &knowntag);
	featTagFmt(tmptag, FeatTagsTableEmbl, &featFmtTmp);
	/* ajDebug(" %3d  %S value: '%S'\n", i, tv->Tag, tv->Value); */
	/* ajDebug(" %3d  %S format: '%S'\n", i, tmptag, featFmtTmp); */
	ajFmtPrintS(&featOutStr, "/%S", tmptag);

	if(tv->Value)
	{
	    ajStrAssignS(&featValTmp, tv->Value);
	    cp = ajStrGetPtr(featFmtTmp);

	    switch(CASE2(cp[0], cp[1]))
	    {
	    case CASE2('L','I') :		/* limited */
		/* ajDebug("case limited\n"); */
		featTagLimit(tmptag, FeatTagsTableEmbl, &tmplim);
		featTagAllLimit(&featValTmp, tmplim);
		ajFmtPrintAppS(&featOutStr, "=%S\n", featValTmp);
		ajStrDel(&tmplim);
		break;
	    case CASE2('Q', 'L') :	/* limited, escape quotes */
		/* ajDebug("case qlimited\n"); */
		featTagLimit(tmptag, FeatTagsTableEmbl, &tmplim);
		featTagAllLimit(&featValTmp, tmplim);
		featTagQuoteEmbl(&featValTmp);
		ajFmtPrintAppS(&featOutStr, "=%S\n", featValTmp);
		ajStrDel(&tmplim);
		break;
	    case CASE2('Q', 'S') :	/* special regexp, quoted */
		/* ajDebug("case qspecial\n"); */
		if(!featTagSpecial(&featValTmp, tmptag))
		{
		    featWarn("%S: Bad special tag value", Seqid);
		    featTagEmblDefault(&featOutStr, tmptag, &featValTmp);
		}
		else
		{
		    featTagQuoteEmbl(&featValTmp);
		    ajFmtPrintAppS(&featOutStr, "=%S\n", featValTmp);
		}
		break;
	    case CASE2('S','P') :	/* special regexp */
		/* ajDebug("case special\n"); */
		if(!featTagSpecial(&featValTmp, tmptag))
		{
		    featWarn("%S: Bad special tag value", Seqid);
		    featTagEmblDefault(&featOutStr, tmptag, &featValTmp);
		}
		else
		    ajFmtPrintAppS(&featOutStr, "=%S\n", featValTmp);

		break;
	    case CASE2('T','E') :     /* no space, no quotes, wrap at margin */
		/* ajDebug("case text\n"); */
		ajStrRemoveWhite(&featValTmp);
		ajFmtPrintAppS(&featOutStr, "=%S\n", featValTmp);
		break;
	    case CASE2('V','O') :	     /* no value, so an error here */
		/*ajDebug("case void\n");*/
		break;
	    case CASE2('Q','T') :	   /* escape quotes, wrap at space */
		/* ajDebug("case qtext\n"); */
		featTagQuoteEmbl(&featValTmp);
		ajFmtPrintAppS(&featOutStr, "=%S\n", featValTmp);
		break;
	    case CASE2('Q','W') :	   /* escape quotes, remove space */
		/* ajDebug("case qword\n"); */
		featTagQuoteEmbl(&featValTmp);
		ajStrRemoveWhite(&featValTmp);	/* no white space needed */
		ajFmtPrintAppS(&featOutStr, "=%S\n", featValTmp);
		break;
	    default:
		featWarn("Unknown EMBL feature tag type '%S' for '%S'",
		       featFmtTmp, tmptag);
	    }
	}
	else
	{
	    /*ajDebug("no value, hope it is void: '%S'\n", featFmtTmp);*/
	}

        featTagEmblWrapC(&featOutStr, 79,
                         ajStrGetPtr(preftyptag), &wrapstr);

	ajFmtPrintF(file, "%S", wrapstr);
	ajStrDel(&wrapstr);
    }
    
    /* ajDebug("featDumpRefseq Done %d tags\n", i); */
    
    ajListIterDel(&iter);
    ajStrDel(&preftyploc);
    ajStrDel(&preftyptag);
    ajStrDel(&tmploc);
    ajStrDel(&tmplim);

    return;
}




/* @funcstatic featDumpRefseqp ************************************************
**
** Write details of single feature to file in RefSeq protein format
**
** @param [r] feat     [const AjPFeature] Feature
** @param [r] location [const AjPStr] location list
** @param [u] file     [AjPFile] Output file
** @param [r] Seqid    [const AjPStr] Sequence ID
** @return [void]
** @@
******************************************************************************/

static void featDumpRefseqp(const AjPFeature feat, const AjPStr location,
                            AjPFile file, const AjPStr Seqid)
{
    AjIList iter   = NULL;
    ajint i        = 0;
    FeatPTagval tv = NULL;
    const AjPStr tmptyp  = NULL;		/* these come from AjPTable */
    const AjPStr tmptag  = NULL;		/* so please, please */
                                                /* don't delete them */
    AjPStr tmplim = NULL;
    AjBool knowntag = ajTrue;
    const char* cp;
    AjPStr wrapstr           = NULL;
    AjPStr preftyploc = NULL;
    AjPStr preftyptag = NULL;
    AjPStr tmploc     = NULL;
    
    /* ajDebug("featDumpRefseqp Start\n"); */
    
    /* print the location */
    
    ajStrAssignS(&tmploc, location);
    tmptyp = featTableTypeExternal(feat->Type, FeatTypeTableRefseqp);
    
    ajFmtPrintS(&preftyploc, "%s   %-15.15S ", "  ", tmptyp);
    ajFmtPrintS(&preftyptag, "%s                   ", "  ");

    
    featLocEmblWrapC(&tmploc, 79,
                     ajStrGetPtr(preftyptag),
                     ajStrGetPtr(preftyploc), &wrapstr);

    ajFmtPrintF(file, "%S", wrapstr);
    ajStrDel(&wrapstr);
    
    /* print the qualifiers */
    
    iter = ajListIterNewread(feat->Tags);

    while(!ajListIterDone(iter))
    {
	tv = ajListIterGet(iter);
	++i;
	tmptag = featTableTag(tv->Tag, FeatTagsTableRefseqp, &knowntag);
	featTagFmt(tmptag, FeatTagsTableRefseqp, &featFmtTmp);
	/* ajDebug(" %3d  %S value: '%S'\n", i, tv->Tag, tv->Value); */
	/* ajDebug(" %3d  %S format: '%S'\n", i, tmptag, featFmtTmp); */
	ajFmtPrintS(&featOutStr, "/%S", tmptag);

	if(tv->Value)
	{
	    ajStrAssignS(&featValTmp, tv->Value);
	    cp = ajStrGetPtr(featFmtTmp);

	    switch(CASE2(cp[0], cp[1]))
	    {
	    case CASE2('L','I') :		/* limited */
		/* ajDebug("case limited\n"); */
		featTagLimit(tmptag, FeatTagsTableRefseqp, &tmplim);
		featTagAllLimit(&featValTmp, tmplim);
		ajFmtPrintAppS(&featOutStr, "=%S\n", featValTmp);
		ajStrDel(&tmplim);
		break;
	    case CASE2('Q', 'L') :	/* limited, escape quotes */
		/* ajDebug("case qlimited\n"); */
		featTagLimit(tmptag, FeatTagsTableRefseqp, &tmplim);
		featTagAllLimit(&featValTmp, tmplim);
		featTagQuoteEmbl(&featValTmp);
		ajFmtPrintAppS(&featOutStr, "=%S\n", featValTmp);
		ajStrDel(&tmplim);
		break;
	    case CASE2('Q', 'S') :	/* special regexp, quoted */
		/* ajDebug("case qspecial\n"); */
		if(!featTagSpecial(&featValTmp, tmptag))
		{
		    featWarn("%S: Bad special tag value", Seqid);
		    featTagEmblDefault(&featOutStr, tmptag, &featValTmp);
		}
		else
		{
		    featTagQuoteEmbl(&featValTmp);
		    ajFmtPrintAppS(&featOutStr, "=%S\n", featValTmp);
		}
		break;
	    case CASE2('S','P') :	/* special regexp */
		/* ajDebug("case special\n"); */
		if(!featTagSpecial(&featValTmp, tmptag))
		{
		    featWarn("%S: Bad special tag value", Seqid);
		    featTagEmblDefault(&featOutStr, tmptag, &featValTmp);
		}
		else
		    ajFmtPrintAppS(&featOutStr, "=%S\n", featValTmp);

		break;
	    case CASE2('T','E') :     /* no space, no quotes, wrap at margin */
		/* ajDebug("case text\n"); */
		ajStrRemoveWhite(&featValTmp);
		ajFmtPrintAppS(&featOutStr, "=%S\n", featValTmp);
		break;
	    case CASE2('V','O') :	     /* no value, so an error here */
		/*ajDebug("case void\n");*/
		break;
	    case CASE2('Q','T') :	   /* escape quotes, wrap at space */
		/* ajDebug("case qtext\n"); */
		featTagQuoteEmbl(&featValTmp);
		ajFmtPrintAppS(&featOutStr, "=%S\n", featValTmp);
		break;
	    case CASE2('Q','W') :	   /* escape quotes, remove space */
		/* ajDebug("case qword\n"); */
		featTagQuoteEmbl(&featValTmp);
		ajStrRemoveWhite(&featValTmp);	/* no white space needed */
		ajFmtPrintAppS(&featOutStr, "=%S\n", featValTmp);
		break;
	    default:
		featWarn("Unknown EMBL feature tag type '%S' for '%S'",
		       featFmtTmp, tmptag);
	    }
	}
	else
	{
	    /*ajDebug("no value, hope it is void: '%S'\n", featFmtTmp);*/
	}

        featTagEmblWrapC(&featOutStr, 79,
                         ajStrGetPtr(preftyptag), &wrapstr);

	ajFmtPrintF(file, "%S", wrapstr);
	ajStrDel(&wrapstr);
    }
    
    /* ajDebug("featDumpRefseqp Done %d tags\n", i); */
    
    ajListIterDel(&iter);
    ajStrDel(&preftyploc);
    ajStrDel(&preftyptag);
    ajStrDel(&tmploc);
    ajStrDel(&tmplim);

    return;
}




/* @funcstatic featDumpPir ****************************************************
**
** Write details of single feature to output file
**
** @param [r] thys [const AjPFeature] Feature
** @param [r] location [const AjPStr] Location as a string
** @param [u] file [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

static void featDumpPir(const AjPFeature thys, const AjPStr location,
			AjPFile file)
{
    AjIList iter  = NULL;
    AjPStr outtyp = NULL;		/* these come from AjPTable */
    const AjPStr outtag = NULL;		/* so please, please */
                                        /* don't delete them */
    AjPStr outcomm = NULL;
    AjPStr outfmt  = NULL;
    FeatPTagval tv        = NULL;
    AjBool knowntag = ajTrue;
    const char* cp;
    AjBool typmod;
    AjPFeature copy = NULL;

    copy = ajFeatNewFeat(thys);

    ajStrAssignClear(&outcomm);
    
    ajStrAssignS(&outtyp, copy->Type);
    typmod = featTypePirOut(&outtyp);	/* try to pick the best type if any */

    /* if changed, we append the original internal type */
    if(typmod)
    {
	ajDebug("typmod %B append ', %S'\n",
                typmod, ajFeatTypeProt(copy->Type));
	ajFeatSetDescApp(copy, ajFeatTypeProt(copy->Type));
    }

/* make sure it's PIR */

    ajStrFmtTitle(&outtyp);
    ajStrExchangeCC(&outtyp, "_", " ");
    
    ajFmtPrintF(file, "F;%S/%S:", location, outtyp);
    
    /* For all tag-values... */
    
    iter = ajFeatTagIter(copy);
    
    while(!ajListIterDone(iter))
    {
	tv = ajListIterGet(iter);
	outtag = featTableTag(tv->Tag, FeatTagsTablePir, &knowntag);
	featTagFmt(outtag, FeatTagsTablePir, &outfmt);
	/*ajDebug("Tag '%S' => '%S' %S '%S'\n",
		tv->Tag, outtag, outfmt, tv->Value);*/

	if(tv->Value)
	{
	    ajStrAssignS(&featValTmp, tv->Value);

	    if(ajStrMatchCaseC(outtag, "comment"))
	    {
		ajFmtPrintAppS(&outcomm, " #%S", featValTmp);
		continue;
	    }

	    cp = ajStrGetPtr(outfmt);

	    switch(CASE2(cp[0], cp[1]))
	    {
                default:
                    ajFmtPrintAppS(&featOutStr, " %S", featValTmp);
	    }
	}
	else
	{
	    /*ajDebug("no value, hope it is void: '%S'\n", outfmt);*/
	}
	
	ajFmtPrintF(file, "%S", featOutStr);
	ajStrDelStatic(&featOutStr);
    }
    
    ajListIterDel(&iter);
    ajFeatDel(&copy);
    ajFmtPrintF(file, "%S\n", outcomm);
    ajStrDel(&outcomm);
    ajStrDel(&outfmt);
    ajStrDel(&outtyp);

    return;
}




/* @funcstatic featDumpSwiss **************************************************
**
** Write details of single feature to output file
**
** @param [r] thys [const AjPFeature] Feature
** @param [u] file [AjPFile] Output file
** @param [r] gftop [const AjPFeature] Parent feature
** @return [void]
** @@
******************************************************************************/

static void featDumpSwiss(const AjPFeature thys, AjPFile file,
			  const AjPFeature gftop)
{
    AjIList iter  = NULL;
    const AjPStr outtyp = NULL;		/* these come from AjPTable */
    const AjPStr outtag = NULL;		/* so please, please */
    /* don't delete them */
    AjPStr outfmt = NULL;
    AjPStr tmplim = NULL;
    FeatPTagval tv       = NULL;
    ajint i =0;
    AjBool knowntag = ajTrue;
    const char* cp;
    AjPStr wrapstr        = NULL;
    AjPStr fromstr = NULL;
    AjPStr tostr   = NULL;
    AjBool wasnote = ajFalse;
    
    outtyp = featTableTypeExternal(thys->Type, FeatTypeTableSwiss);
    
    if(thys->Flags & FEATFLAG_START_UNSURE)
    {
	if(thys->Start)
	    ajFmtPrintS(&fromstr, "?%d", thys->Start);
	else
	    ajFmtPrintS(&fromstr, "?");
    }
    else if(thys->Flags & FEATFLAG_START_BEFORE_SEQ)
	ajFmtPrintS(&fromstr, "<%d", thys->Start);
    else
	ajFmtPrintS(&fromstr, "%d", thys->Start);

    
    if(thys->Flags & FEATFLAG_END_UNSURE)
    {
	if(thys->End)
	    ajFmtPrintS(&tostr, "?%d", thys->End);
	else
	    ajFmtPrintS(&tostr, "?");
    }
    else if(thys->Flags & FEATFLAG_END_AFTER_SEQ)
	ajFmtPrintS(&tostr, ">%d", thys->End);
    else
	ajFmtPrintS(&tostr, "%d", thys->End);
    
    ajFmtPrintS(&featOutStr, "FT   %-8.8S %6.6S %6.6S",
		outtyp, fromstr, tostr);
    
    /* For all tag-values... from gftop which could be the same as thys */
    
    iter = ajFeatTagIter(gftop);
    
    while(!ajListIterDone(iter))
    {
	tv = ajListIterGet(iter);
	outtag = featTableTag(tv->Tag, FeatTagsTableSwiss, &knowntag);
	featTagFmt(outtag, FeatTagsTableSwiss, &outfmt);
	ajDebug("Tag '%S' => '%S' %S '%S'\n",
		tv->Tag, outtag, outfmt, tv->Value);

	if(i++)
	    ajFmtPrintAppS(&featOutStr, " ") ;
	else
	    ajFmtPrintAppS(&featOutStr, "       ") ;

	/* ajFmtPrintAppS(&featOutStr, "%S", outtag); */ /* tag type is silent */

	if(tv->Value)
	{
	    ajStrAssignS(&featValTmp, tv->Value);
	    cp = ajStrGetPtr(outfmt);

	    switch(CASE2(cp[0], cp[1]))
	    {
	    case CASE2('L','I') :	/* limited */
	    case CASE2('Q', 'L') :	/* limited, escape quotes */
		/*ajDebug("case limited\n");*/
		featTagLimit(outtag, FeatTagsTableSwiss, &tmplim);
		featTagAllLimit(&featValTmp, tmplim);
		ajFmtPrintAppS(&featOutStr, "%S.", featValTmp);
		ajStrDel(&tmplim);
                wasnote = ajFalse;
		break;
	    case CASE2('T','A') :	/* tag=text */
		/*ajDebug("case tagval\n");*/
		if(ajStrMatchCaseC(outtag, "ftid")) /* fix case for tag */
		    ajFmtPrintAppS(&featOutStr, "/FTId=%S.",featValTmp);
		else			/* lower case is fine */
		    ajFmtPrintAppS(&featOutStr, "/%S=%S",outtag, featValTmp);
                wasnote = ajFalse;
		break;
	    case CASE2('T','E') :     /* simple text, wrap at space */
		/*ajDebug("case text\n");*/
		ajFmtPrintAppS(&featOutStr, "%S.", featValTmp);
                wasnote = ajTrue;
		break;
	    case CASE2('B','T') :	/* bracketed, wrap at space */
		/*ajDebug("case btext\n");*/
                if(wasnote)
                {
                    ajStrCutEnd(&featOutStr, 2); /* remove ". " */
                    ajStrAppendK(&featOutStr, ' '); /* replace the space */
                }
                ajFmtPrintAppS(&featOutStr, "(%S).", featValTmp);
                wasnote = ajFalse;
		break;
	    default:
		featWarn("Unknown SWISS feature tag type '%S' for '%S'",
		       outfmt, outtag);
                wasnote = ajFalse;
	    }
	}
	else
	{
	    /*ajDebug("no value, hope it is void: '%S'\n", outfmt);*/
	}
    }
    
    ajListIterDel(&iter);
    
    
    featTagSwissWrapC(&featOutStr, 75, "FT                                ",
		      &wrapstr);
    ajFmtPrintF(file, "%S", wrapstr);
    ajStrDelStatic(&featOutStr);
    ajStrDel(&wrapstr);
    ajStrDel(&tostr);
    ajStrDel(&fromstr);
    ajStrDel(&outfmt);
    ajStrDel(&tmplim);

    return;
}




/* @funcstatic featDumpGff2 ***************************************************
**
** Write details of single feature to GFF 2.0 output file
**
** @param [r] thys [const AjPFeature] Feature
** @param [r] owner [const AjPFeattable] Feature table
**                                       (used for the sequence name)
** @param [u] file [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

static void featDumpGff2(const AjPFeature thys, const AjPFeattable owner,
			 AjPFile file)
{
    AjIList iter  = NULL;
    const AjPStr outtyp = NULL;		/* these come from AjPTable */
    const AjPStr outtag = NULL;		/* so please, please */
    /* don't delete them */
    FeatPTagval tv       = NULL;
    ajint i = 0;
    AjBool knowntag = ajTrue;
    const char* cp;
    AjPStr flagdata      = NULL;
    AjPTable tagstable = NULL;
    AjPTable typetable = NULL;

    if(thys->Protein)
    {
	typetable = FeatTypeTableGffprotein;
	tagstable = FeatTagsTableGffprotein;
    }
    else
    {
	typetable = FeatTypeTableGff;
	tagstable = FeatTagsTableGff;
    }
    
    /* header done by calling routine */
    
    /*ajDebug("featDumpGff...\n");*/
    
    /* simple line-by line with Gff tags */
    
    outtyp = featTableTypeExternal(thys->Type, typetable);
    
    /*ajDebug("Type '%S' => '%S'\n", thys->Type, outtyp);*/
    
    if(featStrand(thys->Strand) == '-' && thys->End < thys->Start)
        ajFmtPrintF(file, "%S\t%S\t%S\t%d\t%d\t%.3f\t%c\t%c\t",
                    owner->Seqid,
                    thys->Source,
                    outtyp,
                    thys->End,
                    thys->Start,
                    thys->Score,
                    featStrand(thys->Strand),
                    featFrame(thys->Frame));
    else
        ajFmtPrintF(file, "%S\t%S\t%S\t%d\t%d\t%.3f\t%c\t%c\t",
                    owner->Seqid,
                    thys->Source,
                    outtyp,
                    thys->Start,
                    thys->End,
                    thys->Score,
                    featStrand(thys->Strand),
                    featFrame(thys->Frame));

    if(thys->Flags)
	ajFmtPrintS(&flagdata, "0x%x", thys->Flags);

    if(thys->Start2)
    {
	if(ajStrGetLen(flagdata))
	    ajStrAppendC(&flagdata, " ");

	ajFmtPrintAppS(&flagdata, "start2:%d", thys->Start2);
    }

    if(thys->End2)
    {
	if(ajStrGetLen(flagdata))
	    ajStrAppendC(&flagdata, " ");

	ajFmtPrintAppS(&flagdata, "end2:%d", thys->End2);
    }

    if(ajStrGetLen(thys->Remote))
    {
	if(ajStrGetLen(flagdata))
	    ajStrAppendC(&flagdata, " ");

	ajFmtPrintAppS(&flagdata, "remoteid:%S", thys->Remote);
    }

    if(ajStrGetLen(thys->Label))
    {
	if(ajStrGetLen(flagdata))
	    ajStrAppendC(&flagdata, " ");

	ajFmtPrintAppS(&flagdata, "label:%S", thys->Label);
    }
    
    /* group and flags */
    
    ajFmtPrintF(file, "Sequence \"%S.%d\"",
		owner->Seqid, thys->Group) ;
    i++;
    
    if(ajStrGetLen(flagdata))
    {
	/*
	 ** Move this code up to run for all features - to preserve the order
	 ** when rewriting in EMBL format
	     if(FEATFLAG_MULTIPLE)
             {
	       (void) ajFmtPrintF(file, "Sequence \"%S.%d\" ; ",
	                          owner->Seqid, thys->Group) ;
	       i++;
	     }
	 */
	if(i++)
	    ajFmtPrintF(file, " ; ") ;

	ajFmtPrintF(file, "FeatFlags \"%S\"", flagdata) ;
    }
    
    /* For all tag-values... */
    
    iter = ajFeatTagIter(thys);
    
    while(!ajListIterDone(iter))
    {
	tv     = ajListIterGet(iter);
	outtag = featTableTag(tv->Tag, tagstable, &knowntag);
	featTagFmt(outtag, tagstable, &featFmtTmp);

	/*ajDebug("Tag '%S' => '%S' %S '%S'\n",
		tv->Tag, outtag, featFmtTmp, tv->Value);*/
	if(i++)
	    ajFmtPrintF(file, " ; ") ;

	ajFmtPrintAppS(&featOutStr, "%S", outtag);
	
	if(tv->Value)
	{
	    ajStrAssignS(&featValTmp, tv->Value);
	    cp = ajStrGetPtr(featFmtTmp);

	    switch(CASE2(cp[0], cp[1]))
	    {
	    case CASE2('L','I') :	/* limited */
		/*ajDebug("case limited\n");*/
		featTagLimit(outtag, tagstable, &featLimTmp);
		featTagAllLimit(&featValTmp, featLimTmp);
		ajFmtPrintAppS(&featOutStr, " %S", featValTmp);
		break;
	    case CASE2('Q', 'L') :	/* limited, escape quotes */
		/*ajDebug("case qlimited\n");*/
		featTagLimit(outtag, tagstable, &featLimTmp);
		featTagAllLimit(&featValTmp, featLimTmp);
		featTagQuoteGff2(&featValTmp);
		ajFmtPrintAppS(&featOutStr, " %S", featValTmp);
		break;
	    case CASE2('T','E') : /* no space, no quotes, wrap at margin */
		/*ajDebug("case text\n");*/
		ajStrRemoveWhite(&featValTmp);
		ajFmtPrintAppS(&featOutStr, " %S", featValTmp);
		break;
	    case CASE2('Q','T') :	/* escape quotes, wrap at space */
		/*ajDebug("case qtext\n");*/
		featTagQuoteGff2(&featValTmp);
		ajFmtPrintAppS(&featOutStr, " %S", featValTmp);
		break;
	    case CASE2('Q','W') :	/* escape quotes, remove space */
		/*ajDebug("case qtext\n");*/
		featTagQuoteGff2(&featValTmp);
		ajStrRemoveWhite(&featValTmp);
		ajFmtPrintAppS(&featOutStr, " %S", featValTmp);
		break;
	    case CASE2('Q', 'S') :	/* special regexp, quoted */
		/*ajDebug("case qspecial\n");*/
		if(!featTagGffSpecial(&featValTmp, outtag))
		    featTagGff2Default(&featOutStr, outtag, &featValTmp);
		else
		{
		    featTagQuoteGff2(&featValTmp);
		    ajFmtPrintAppS(&featOutStr, " %S", featValTmp);
		}
		break;
	    case CASE2('S','P') :	/* special regexp */
		/*ajDebug("case special\n");*/
		if(!featTagGffSpecial(&featValTmp, outtag))
		    featTagGff2Default(&featOutStr, outtag, &featValTmp);
		else
		    ajFmtPrintAppS(&featOutStr, " %S", featValTmp);

		break;
	    case CASE2('V','O') :	/* no value, so an error here */
		/*ajDebug("case void\n");*/
		break;
	    default:
		featWarn("Unknown GFF 2.0 feature tag type '%S' for '%S'",
		       featFmtTmp, outtag);
	    }
	}
	else
	{
	    /*ajDebug("no value, hope it is void: '%S'\n", featFmtTmp);*/
	}
	
	ajFmtPrintF(file, "%S", featOutStr);
	ajStrDelStatic(&featOutStr);
    }

    ajFmtPrintF(file, "\n") ;    
    ajListIterDel(&iter);
    ajStrDel(&flagdata);
    
    return;
}




/* @funcstatic featDumpGff3 ***************************************************
**
** Write details of single feature to GFF 3.0 output file
**
** @param [r] thys [const AjPFeature] Feature
** @param [r] owner [const AjPFeattable] Feature table
**                                       (used for the sequence name)
** @param [u] file [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

static void featDumpGff3(const AjPFeature thys, const AjPFeattable owner,
			 AjPFile file)
{
    AjIList iter  = NULL;
    const AjPStr outtyp = NULL;		/* these come from AjPTable */
    const AjPStr outtag = NULL;		/* so please, please */
    /* don't delete them */
    FeatPTagval tv       = NULL;
    ajint i = 0;
    AjBool knowntag = ajTrue;
    const char* cp;
    AjPStr flagdata      = NULL;
    AjPTable tagstable = NULL;
    AjPTable typetable = NULL;
    char frame;

    if(thys->Protein)
    {
	typetable = FeatTypeTableGff3protein;
	tagstable = FeatTagsTableGff3protein;
    }
    else
    {
	typetable = FeatTypeTableGff3;
	tagstable = FeatTagsTableGff3;
    }
    
    /* header done by calling routine */
    
    /*ajDebug("featDumpGff3...\n");*/
    
    /* simple line-by line with Gff3 tags */
    
    outtyp = featTableTypeExternal(thys->Type, typetable);
    
    /*ajDebug("Type '%S' => '%S'\n", thys->Type, outtyp);*/

    if(ajFeatTypeIsCds(thys))
	frame = featFrameNuc(thys->Frame);
    else
	frame = featFrame(thys->Frame);

    if(featStrand(thys->Strand) == '-' && thys->End < thys->Start)
        ajFmtPrintF(file, "%S\t%S\t%S\t%d\t%d\t%.3f\t%c\t%c\t",
                    owner->Seqid,
                    thys->Source,
                    outtyp,
                    thys->End,
                    thys->Start,
                    thys->Score,
                    featStrand(thys->Strand),
                    frame);
    else
        ajFmtPrintF(file, "%S\t%S\t%S\t%d\t%d\t%.3f\t%c\t%c\t",
                    owner->Seqid,
                    thys->Source,
                    outtyp,
                    thys->Start,
                    thys->End,
                    thys->Score,
                    featStrand(thys->Strand),
                    frame);
        
    if(thys->Flags)
	ajFmtPrintS(&flagdata, "0x%x", thys->Flags);

    if(thys->Start2)
    {
	if(ajStrGetLen(flagdata))
	    ajStrAppendC(&flagdata, " ");

	ajFmtPrintAppS(&flagdata, "start2:%d", thys->Start2);
    }

    if(thys->End2)
    {
	if(ajStrGetLen(flagdata))
	    ajStrAppendC(&flagdata, " ");

	ajFmtPrintAppS(&flagdata, "end2:%d", thys->End2);
    }

    if(ajStrGetLen(thys->Remote))
    {
	if(ajStrGetLen(flagdata))
	    ajStrAppendC(&flagdata, " ");

	ajFmtPrintAppS(&flagdata, "remoteid:%S", thys->Remote);
    }

    if(ajStrGetLen(thys->Label))
    {
	if(ajStrGetLen(flagdata))
	    ajStrAppendC(&flagdata, " ");

	ajFmtPrintAppS(&flagdata, "label:%S", thys->Label);
    }
    
    /* group and flags */
    
    ajFmtPrintF(file, "ID=\"%S.%d\"",
		owner->Seqid, thys->Group) ;
    i++;
    
    if(ajStrGetLen(flagdata))
    {
	/*
	 ** Move this code up to run for all features - to preserve the order
	 ** when rewriting in EMBL format
	     if(FEATFLAG_MULTIPLE)
             {
	       (void) ajFmtPrintF(file, "Sequence=\"%S.%d\";",
	 			 owner->Seqid, thys->Group) ;
	       i++;
	     }
	 */
	if(i++)
	    ajFmtPrintF(file, ";") ;
	ajFmtPrintF(file, "featflags=\"%S\"", flagdata) ;
    }
    
    /* For all tag-values... */
    
    iter = ajFeatTagIter(thys);
    
    while(!ajListIterDone(iter))
    {
	tv     = ajListIterGet(iter);
	outtag = featTableTag(tv->Tag, tagstable, &knowntag);
	if(!outtag)
	{
		featWarn("Unknown GFF3 feature tag '%S'",
		       tv->Tag);
	    continue;
	}

	featTagFmt(outtag, tagstable, &featFmtTmp);
	/*ajDebug("Tag '%S' => '%S' %S '%S'\n",
		tv->Tag, outtag, featFmtTmp, tv->Value);*/
	if(i++)
	    ajFmtPrintF(file, ";") ;

	ajFmtPrintAppS(&featOutStr, "%S", outtag);
	
	if(tv->Value)
	{
	    ajStrAssignS(&featValTmp, tv->Value);
	    cp = ajStrGetPtr(featFmtTmp);

	    switch(CASE2(cp[0], cp[1]))
	    {
	    case CASE2('L','I') :	/* limited */
		/*ajDebug("case limited\n");*/
		featTagLimit(outtag, tagstable, &featLimTmp);
		featTagAllLimit(&featValTmp, featLimTmp);
		ajFmtPrintAppS(&featOutStr, "=%S", featValTmp);
		break;
	    case CASE2('Q', 'L') :	/* limited, escape quotes */
		/*ajDebug("case qlimited\n");*/
		featTagLimit(outtag, tagstable, &featLimTmp);
		featTagAllLimit(&featValTmp, featLimTmp);
		featTagQuoteGff3(&featValTmp);
		ajFmtPrintAppS(&featOutStr, "=%S", featValTmp);
		break;
	    case CASE2('T','E') : /* no space, no quotes, wrap at margin */
		/*ajDebug("case text\n");*/
		ajStrRemoveWhite(&featValTmp);
		ajFmtPrintAppS(&featOutStr, "=%S", featValTmp);
		break;
	    case CASE2('Q','T') :	/* escape quotes, wrap at space */
		/*ajDebug("case qtext\n");*/
		featTagQuoteGff3(&featValTmp);
		ajFmtPrintAppS(&featOutStr, "=%S", featValTmp);
		break;
	    case CASE2('Q','W') :	/* escape quotes, remove space */
		/*ajDebug("case qtext\n");*/
		featTagQuoteGff3(&featValTmp);
		ajStrRemoveWhite(&featValTmp);
		ajFmtPrintAppS(&featOutStr, "=%S", featValTmp);
		break;
	    case CASE2('Q', 'S') :	/* special regexp, quoted */
		/*ajDebug("case qspecial\n");*/
		if(!featTagGff3Special(&featValTmp, outtag))
		    featTagGff3Default(&featOutStr, outtag, &featValTmp);
		else
		{
		    featTagQuoteGff3(&featValTmp);
		    ajFmtPrintAppS(&featOutStr, "=%S", featValTmp);
		}
		break;
	    case CASE2('S','P') :	/* special regexp */
		/*ajDebug("case special\n");*/
		if(!featTagGff3Special(&featValTmp, outtag))
		    featTagGff3Default(&featOutStr, outtag, &featValTmp);
		else
		    ajFmtPrintAppS(&featOutStr, "=%S", featValTmp);

		break;
	    case CASE2('V','O') :	/* no value, so an error here */
		/*ajDebug("case void\n");*/
		break;
	    default:
		featWarn("Unknown GFF3 feature tag type '%S' for '%S'",
		       featFmtTmp, outtag);
	    }
	}
	else
	{
	    /*ajDebug("no value, hope it is void: '%S'\n", featFmtTmp);*/
	}
	
	ajFmtPrintF(file, "%S", featOutStr);
	ajStrDelStatic(&featOutStr);
    }


    ajFmtPrintF(file, "\n") ;
    ajListIterDel(&iter);
    ajStrDel(&flagdata);
    
    return;
}




/* @funcstatic featTypePirIn **************************************************
**
** Converts a PIR feature type into the corresponding internal type,
** because internal types are based on SwissProt.
**
** @param [u] type [AjPStr*] PIR feature type in, returned as internal type
** @return [AjBool] ajTrue if the type name was found and changed
******************************************************************************/

static AjBool featTypePirIn(AjPStr* type)
{
    const AjPStr pirintype = NULL;

    pirintype = featTableTypeInternal(*type, FeatTypeTablePir);
    ajStrAssignS(type, pirintype);

    ajDebug("featTypePirin '%S' in '%S'\n",
	    *type, pirintype);

    return ajFalse;
}




/* @funcstatic featTypePirOut *************************************************
**
** Converts an internal feature type into the corresponding PIR type,
** because internal types are based on GFF3.
**
** @param [u] type [AjPStr*] PIR feature type in, returned as internal type
** @return [AjBool] ajTrue if the type name was found and changed
******************************************************************************/

static AjBool featTypePirOut(AjPStr* type)
{
    const AjPStr pirtype = NULL;
    const AjPStr pirintype = NULL;
    AjBool ret = ajFalse;

    pirtype = featTableTypeExternal(*type, FeatTypeTablePir);

    pirintype = featTableTypeInternal(pirtype, FeatTypeTablePir);

    ajDebug("featTypePirOut '%S' in '%S' ext '%S'\n",
	    *type, pirintype, pirtype);

    if(!ajStrMatchCaseS(*type, pirintype))
	   ret = ajTrue;

    ajStrAssignS(type, pirtype);

    return ret;

}




/* @funcstatic featTagFmt *****************************************************
**
** Converts a feature tag value into the correct format, after
** checking it is an acceptable value
**
** @param [r] name  [const AjPStr] Tag name
** @param [r] table [const AjPTable] Tag table
** @param [w] retstr [AjPStr*] string with formatted value.
** @return [void]
** @@
******************************************************************************/

static void featTagFmt(const AjPStr name, const AjPTable table,
		       AjPStr* retstr)
{
    AjPStr tagstr    = NULL;
    const char* cp;
    const char* cq;
    ajint i;

    tagstr = (AjPStr) ajTableFetch(table, name);

    cp = ajStrGetPtr(tagstr);

    ajStrAssignClear(retstr);

    cq = cp;
    i=0;

    while(*cp && (*cp++ != ';'))
	i++;

    ajStrAssignLenC(retstr, cq, i);

    /* ajDebug("featTagFmt '%S' type '%S' (%S)\n",
       name, *retstr, tagstr); */

    return;
}




/* @funcstatic featTagLimit ***************************************************
**
** Returns the controlled vocabulary list for a limited value.
**
** @param [r] name  [const AjPStr] Tag name
** @param [r] table [const AjPTable] Tag table
** @param [w] retstr [AjPStr*] string with formatted value.
** @return [void]
** @@
******************************************************************************/

static void featTagLimit(const AjPStr name, const AjPTable table,
			 AjPStr* retstr)
{
    AjPStr tagstr;
    const char* cp = NULL;

    ajStrAssignClear(retstr);
    tagstr = (AjPStr) ajTableFetch(table, name);
    cp = ajStrGetPtr(tagstr);

    while(*cp && (*cp != ';'))
	cp++;

    if(!*cp)
	return;

    ajStrAssignC(retstr, cp);

    return;
}




/* @func ajFeatExit ***********************************************************
**
** Prints a summary of file usage with debug calls
**
** Cleans up feature table internal memory
**
** @return [void]
** @@
******************************************************************************/

void ajFeatExit(void)
{
    ajint i;

    for(i=1;featInFormat[i].Name;i++)
    {
	if(featInFormat[i].Used)
	{
	    /* Calling funclist featInFormatDef() */
	    if(!featInFormat[i].DelReg())
	    {
		/*ajDebug("No DelReg yet for %s\n",featInFormat[i].Name);*/
		ajErr("No DelReg yet for %s\n",featInFormat[i].Name);
	    }
	}
    }

    ajTablestrFree(&FeatTypeTableEmbl);
    ajTablestrFree(&FeatTagsTableEmbl);

    ajTablestrFree(&FeatTypeTableGff);
    ajTablestrFree(&FeatTagsTableGff);

    ajTablestrFree(&FeatTypeTableGff3);
    ajTablestrFree(&FeatTagsTableGff3);

    ajTablestrFree(&FeatTypeTablePir);
    ajTablestrFree(&FeatTagsTablePir);

    ajTablestrFree(&FeatTypeTableGffprotein);
    ajTablestrFree(&FeatTagsTableGffprotein);

    ajTablestrFree(&FeatTypeTableGff3protein);
    ajTablestrFree(&FeatTagsTableGff3protein);

    ajTablestrFree(&FeatTypeTableSwiss);
    ajTablestrFree(&FeatTagsTableSwiss);

    ajTablestrFree(&FeatTypeTableDna);
    ajTablestrFree(&FeatTagsTableDna);

    ajTablestrFree(&FeatTypeTableProtein);
    ajTablestrFree(&FeatTagsTableProtein);

    ajStrDel(&featTypeEmpty);
    ajStrDel(&featTypeMiscfeat);
    ajStrDel(&featDefSource);
    ajStrDel(&featFmtTmp);
    ajStrDel(&featLimTmp);
    ajStrDel(&featTagTmp);
    ajStrDel(&featTagTmp2);
    ajStrDel(&featValTmp);
    ajStrDel(&featValTmp2);
    ajStrDel(&featUfoTest);
    ajStrDel(&featFormatTmp);
    ajStrDel(&featOutStr);
    ajStrDel(&featReadLine);
    ajStrDel(&featProcessLine);
    ajStrDel(&featTmpStr);
    ajStrDel(&featValCopy);
    ajStrDel(&featSubStr);
    ajStrDel(&featGroup);
    ajStrDel(&featSource);
    ajStrDel(&featFeature);

    ajStrDel(&featTagNote);
    ajStrDel(&featTagComm);
    ajStrDel(&featTagFtid);

    ajStrDel(&featSourcePir);
    ajStrDel(&featSourceSwiss);
    ajStrDel(&featSourceEmbl);
    ajStrDel(&featSourceRefseqp);
    ajStrDel(&featId);
    ajStrDel(&featLabel);

    ajStrDel(&featLocStr);
    ajStrDel(&featLocToken);
    ajStrDel(&featLocDb);
    ajStrDel(&featSaveGroupStr);

    ajStrDel(&featTransBegStr);
    ajStrDel(&featTransEndStr);
    ajStrDel(&featTransAaStr);

    ajStrTokenDel(&featGffSplit);
    ajStrTokenDel(&featEmblSplit);
    ajStrTokenDel(&featVocabSplit);

    ajRegFree(&featRegUfoFmt);
    ajRegFree(&featRegUfoFile);
    ajRegFree(&featRegTagReplace);
    ajRegFree(&featRegFlag);
    ajRegFree(&featRegMore);
    ajRegFree(&featRegQuote);

    ajRegFree(&featTagTrans);
    ajRegFree(&featRegGroup);

    ajRegFree(&featRegSpecialAnticodon);
    ajRegFree(&featRegSpecialBiomaterial);
    ajRegFree(&featRegSpecialCodon);
    ajRegFree(&featRegSpecialCodonBad);
    ajRegFree(&featRegSpecialColdate);
    ajRegFree(&featRegSpecialCompare);
    ajRegFree(&featRegSpecialConssplice);
    ajRegFree(&featRegSpecialEstlen);
    ajRegFree(&featRegSpecialInference);
    ajRegFree(&featRegSpecialLatlon);
    ajRegFree(&featRegSpecialMobile);
    ajRegFree(&featRegSpecialPrimer);
    ajRegFree(&featRegSpecialRptRange);
    ajRegFree(&featRegSpecialRptRangeLab);
    ajRegFree(&featRegSpecialRptRangeComp);
    ajRegFree(&featRegSpecialRptunitSeq);
    ajRegFree(&featRegSpecialRptunitSeqPos);
    ajRegFree(&featRegSpecialTrans);
    ajRegFree(&featRegSpecialTransBad);
    ajRegFree(&featRegSpecialTransComp);
    ajRegFree(&featRegSpecialTransBadComp);

    return;
}




/* @func ajFeatUnused *********************************************************
**
** Dummy function to prevent compiler warnings
**
** @return [void]
******************************************************************************/

void ajFeatUnused(void)
{
    AjBool knowntag = ajTrue;

    if(!DummyRegExec)
	DummyRegExec = ajRegCompC(".*");

    featTagDna(NULL, &knowntag);
    featTagProt(NULL, &knowntag);
    featTableTagC(NULL, NULL, &knowntag);
}




/* @funcstatic featFeatureNew *************************************************
**
** Constructor for a feature
**
** @return [AjPFeature] New empty feature
******************************************************************************/

static AjPFeature featFeatureNew(void)
{
    AjPFeature ret;

    AJNEW0(ret);

    ret->Tags = ajListNew() ; /* Assume empty until otherwise needed */

    return ret;
}




/* @funcstatic featTableNew ***************************************************
**
** Constructor for a feature table object.
**
** The type is left uninitialised
**
** @return [AjPFeattable] New empty feature table
******************************************************************************/

static AjPFeattable featTableNew(void)
{
    AjPFeattable ret;

    AJNEW0(ret);

    ret->Features = ajListNew() ; /* assume empty until otherwise needed */

    return ret;
}




/* @funcstatic featTableNewS **************************************************
**
** Constructor for a feature table object with a defined name
**
** The type is left uninitialised
**
** @param [r] name [const AjPStr] Name for new feature table
**                                (or NULL for unnamed)
** @return [AjPFeattable] New empty feature table
******************************************************************************/

static AjPFeattable featTableNewS(const AjPStr name)
{
    AjPFeattable ret;

    ret = featTableNew();
    featTableInit(ret, name);

    return ret;
}




/* @func ajFeatTypeIsCds ******************************************************
**
** Tests whether the feature is a CDS feature
**
** @param [r] gf       [const AjPFeature]  Feature
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeatTypeIsCds(const AjPFeature gf)
{
    if(ajStrMatchC(gf->Type, "SO:0000316"))
	return ajTrue;

    return ajFalse;
}




/* @func ajFeatTypeMatchC ******************************************************
**
** Tests whether the feature type matches a given string
** including testing for alternative names
**
** @param [r] gf       [const AjPFeature]  Feature
** @param [r] txt      [const char*]  Feature type name to test
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeatTypeMatchC(const AjPFeature gf, const char* txt)
{
    AjBool ret = ajFalse;
    AjPStr tmpstr;
    const AjPStr tmptype = NULL;

    if(ajStrMatchC(gf->Type, txt))
	return ajTrue;

    tmpstr = ajStrNewC(txt);

    if(gf->Protein)
    {
        tmptype = featTypeProtLimit(tmpstr);
        if(tmptype)
            ret = ajStrMatchS(featTypeProtLimit(gf->Type),
                              tmptype);
        ajDebug("ajFeatTypeMatch: %B '%S' prot: '%S' <=> '%S'\n",
                ret, tmpstr, tmptype, gf->Type);
    }
    
    else
    {
	tmptype = featTypeDnaLimit(tmpstr);
        if(tmptype)
            ret = ajStrMatchS(featTypeDnaLimit(gf->Type),
                              tmptype);
        ajDebug("ajFeatTypeMatch: %B '%S' dna: '%S' <=> '%S'\n",
                ret, tmpstr, tmptype, gf->Type);
    }

    ajStrDel(&tmpstr);

    return ret;
}




/* @func ajFeatTypeMatchS ******************************************************
**
** Tests whether the feature type matches a given string
** including testing for alternative names
**
** @param [r] gf       [const AjPFeature]  Feature
** @param [r] str      [const AjPStr]  Feature type name to test
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeatTypeMatchS(const AjPFeature gf, const AjPStr str)
{
    AjBool ret = ajFalse;

    if(ajStrMatchS(gf->Type, str))
	return ajTrue;

    if(gf->Protein)
    {
	ret =   ajStrMatchS(featTypeProtLimit(gf->Type),featTypeProtLimit(str));
        ajDebug("ajFeatTypeMatch: %B '%S' '%S' prot: '%S' <=> '%S'\n",
                ret, str, gf->Type,
                featTypeProtLimit(str), featTypeProtLimit(gf->Type));
    }
    
    else
    {
	ret =   ajStrMatchS(featTypeDnaLimit(gf->Type),featTypeDnaLimit(str));
        ajDebug("ajFeatTypeMatch: %B '%S' '%S' dna: '%S' <=> '%S'\n",
                ret, str, gf->Type,
                featTypeDnaLimit(str), featTypeDnaLimit(gf->Type));
    }

    return ret;
}




/* @func ajFeatTypeMatchWildS **************************************************
**
** Tests whether the feature type matches a given wildcard string
** including testing for alternative names
**
** @param [r] gf       [const AjPFeature]  Feature
** @param [r] str      [const AjPStr]  Feature type name to test
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeatTypeMatchWildS(const AjPFeature gf, const AjPStr str)
{
    AjBool ret = ajFalse;
    AjPStr mystr = NULL;

    mystr = ajStrNewS(str);
    ajStrFmtLower(&mystr);
    
    if(gf->Protein)
    {
	ret =  featTypeTestProtWild(gf->Type, mystr);

        if(!ret)
        {
            if(ajStrIsWild(mystr))
                ajStrTrimEndC(&mystr, "*");
            ret = ajFeatTypeMatchS(gf, mystr);
        }
    
        ajDebug("ajFeatTypeMatchWildS: %B '%S' '%S' prot:'%S'\n",
                ret, mystr, gf->Type,
                featTypeProtLimit(gf->Type));
    }
    
    else
    {
	ret =  featTypeTestDnaWild(gf->Type, mystr);

        if(!ret)
        {
            if(ajStrIsWild(mystr))
                ajStrTrimEndC(&mystr, "*");
            ret = ajFeatTypeMatchS(gf, mystr);
        }

        ajDebug("ajFeatTypeMatchWildS: %B '%S' '%S' dna: '%S'\n",
                ret, mystr, gf->Type,
                featTypeDnaLimit(gf->Type));
    }
    ajStrDel(&mystr);

    return ret;
}




/* @func ajFeatIsLocal ********************************************************
**
** Tests whether the feature is local to the sequence.
** Returns AJTRUE if it is local, AJFALSE if remote.
**
** @param [r] gf       [const AjPFeature]  Feature
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeatIsLocal(const AjPFeature gf)
{
    return !(gf->Flags & FEATFLAG_REMOTEID);
}




/* @func ajFeatIsLocalRange ***************************************************
**
** Tests whether the feature is local and in the specified range of the
** sequence.
** Returns AJTRUE if it is local and within the range.
** (Any label location is assumed to be outside the range.)
**
** @param [r] gf       [const AjPFeature]  Feature
** @param [r] start    [ajuint]  start of range
** @param [r] end      [ajuint]  end of range
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeatIsLocalRange(const AjPFeature gf, ajuint start, ajuint end)
{
    if(gf->Flags & FEATFLAG_REMOTEID)
	return AJFALSE;

    if(gf->Flags & FEATFLAG_LABEL)
	return AJFALSE;

    if(gf->End < start || gf->Start > end)
	return AJFALSE;

    return AJTRUE;
}




/* @func ajFeatIsChild ********************************************************
**
** Tests whether the feature is a child member of a join
** The parent (first) feature of a join gives:
** ajFeatIsChild == ajFalse && ajFeatIsMultiple == ajTrue
**
** @param [r] gf       [const AjPFeature]  Feature
** @return [AjBool] Returns AJTRUE if it is a child,
**                  AJFALSE if it is not a child
** @@
******************************************************************************/

AjBool ajFeatIsChild(const AjPFeature gf)
{
    return (gf->Flags & FEATFLAG_CHILD);
}




/* @func ajFeatIsMultiple *****************************************************
**
** Tests whether the feature is a member of a join, group order or one_of
**
** @param [r] gf       [const AjPFeature]  Feature
** @return [AjBool] Returns AJTRUE if it is a member
** @@
******************************************************************************/

AjBool ajFeatIsMultiple(const AjPFeature gf)
{
    return (gf->Flags & FEATFLAG_MULTIPLE);
}




/* @func ajFeatIsCompMult *****************************************************
**
** Tests whether the feature is a member of a complement around a
** multiple (join, etc.)
**
** @param [r] gf       [const AjPFeature]  Feature
** @return [AjBool] Returns AJTRUE if it is a complemented multiple
** @@
******************************************************************************/

AjBool ajFeatIsCompMult(const AjPFeature gf)
{
    return (gf->Flags & FEATFLAG_COMPLEMENT_MAIN);
}




/* @func ajFeattabOutClear ****************************************************
**
** Clears all data for a feature table output object
**
** @param [d] thys [AjPFeattabOut *] feature format
** @return [void] Feature table output object
** @category delete [AjPFeattabOut] Destructor
** @@
******************************************************************************/

void ajFeattabOutClear(AjPFeattabOut *thys)
{
    AjPFeattabOut pthis;

    pthis = *thys;

    if(!pthis)
        return;

    ajStrSetClear(&pthis->Ufo);
    ajStrSetClear(&pthis->Formatstr);
    ajStrSetClear(&pthis->Filename);
    ajStrSetClear(&pthis->Directory);
    ajStrSetClear(&pthis->Seqid);
    ajStrSetClear(&pthis->Type);
    ajStrSetClear(&pthis->Seqname);
    ajStrSetClear(&pthis->Basename);

    if(pthis->Cleanup)
        pthis->Cleanup(pthis->Handle);

    pthis->Cleanup = NULL;
    pthis->Count = 0;

    /* "Local" tables have borrowed an open file with NewSSF */
    if(!pthis->Local)
	ajFileClose(&pthis->Handle);

    return;
}




/* @func ajFeattabOutDel ******************************************************
**
** Destructor for a feature table output object
**
** @param [d] thys [AjPFeattabOut *] feature format
** @return [void] Feature table output object
** @category delete [AjPFeattabOut] Destructor
** @@
******************************************************************************/

void ajFeattabOutDel(AjPFeattabOut *thys)
{
    AjPFeattabOut pthis;

    pthis = *thys;
    if(!pthis)
        return;

    ajStrDel(&pthis->Ufo);
    ajStrDel(&pthis->Formatstr);
    ajStrDel(&pthis->Filename);
    ajStrDel(&pthis->Directory);
    ajStrDel(&pthis->Seqid);
    ajStrDel(&pthis->Type);
    ajStrDel(&pthis->Seqname);
    ajStrDel(&pthis->Basename);

    if(pthis->Cleanup)
        pthis->Cleanup(pthis->Handle);

    pthis->Cleanup = NULL;
    pthis->Count = 0;

    /* "Local" tables have borrowed an open file with NewSSF */
    if(!pthis->Local)
	ajFileClose(&pthis->Handle);

    AJFREE(pthis);

    return;
}




/* @func ajFeattablePos *******************************************************
**
** Converts a string position into a true position. If ipos is negative,
** it is counted from the end of the string rather than the beginning.
**
** For strings, the result can go off the end to the terminating NULL.
** For sequences the maximum is the last base.
**
** @param [r] thys [const AjPFeattable] Target feature table.
** @param [r] ipos [ajint] Position.
** @return [ajuint] string position between 1 and length.
** @@
******************************************************************************/

ajuint ajFeattablePos(const AjPFeattable thys, ajint ipos)
{
    return ajFeattablePosII(ajFeattableGetLen(thys), 1, ipos);
}




/* @func ajFeattablePosI ******************************************************
**
** Converts a string position into a true position. If ipos is negative,
** it is counted from the end of the string rather than the beginning.
**
** imin is a minimum relative position, also counted from the end
** if negative. Usually this is the start position when the end of a range
** is being tested.
**
** @param [r] thys [const AjPFeattable] Target feature table.
** @param [r] imin [ajuint] Start position.
** @param [r] ipos [ajint] Position.
** @return [ajuint] string position between 1 and length.
** @@
******************************************************************************/

ajuint ajFeattablePosI(const AjPFeattable thys, ajuint imin, ajint ipos)
{
    return ajFeattablePosII(ajFeattableGetLen(thys), imin, ipos);
}




/* @func ajFeattablePosII *****************************************************
**
** Converts a position into a true position. If ipos is negative,
** it is counted from the end of the sequence rather than the beginning.
**
** imin is a minimum relative position, also counted from the end
** if negative. Usually this is the start position when the end of a range
** is being tested.
**
** For strings, the result can go off the end to the terminating NULL.
** For sequences the maximum is the last base.
**
** @param [r] ilen [ajuint] maximum length.
** @param [r] imin [ajuint] Start position.
** @param [r] ipos [ajint] Position.
** @return [ajuint] string position between 1 and length.
** @@
******************************************************************************/

ajuint ajFeattablePosII(ajuint ilen, ajuint imin, ajint ipos)
{
    ajuint jpos;

    if(ipos < 0)
	jpos = ilen + ipos + 1;
    else
    {
	if(ipos)
	    jpos = ipos;
	else
	    jpos = 1;
    }

    if(jpos > ilen)
	jpos = ilen;

    if(jpos < imin)
	jpos = imin;

    /*ajDebug("ajFeattablePosII(ilen: %d imin: %d ipos: %d) = %d\n",
	    ilen, imin, ipos, jpos);*/

    return jpos;
}




/* @func ajFeattableTrimOff ***************************************************
**
** Trim a feature table using the Begin and Ends.
**
** Called where a sequence has been trimmed, so we have to allow for
** missing sequence positions at the start (ioffset) or at the end (ilen).
**
** @param [u] thys [AjPFeattable] Target feature table.
** @param [r] ioffset [ajuint] Offset from start of sequence
** @param [r] ilen [ajuint] Length of sequence
** @return [AjBool] AjTrue returned if successful.
** @@
******************************************************************************/

AjBool ajFeattableTrimOff(AjPFeattable thys, ajuint ioffset, ajuint ilen)
{
    AjBool ok      = ajTrue;
    AjBool dobegin = ajFalse;
    AjBool doend   = ajFalse;
    ajuint begin    = 0;
    ajuint end      = 0;
    ajuint iseqlen;
    AjIList     iter = NULL ;
    AjPFeature  ft   = NULL ;
    
    /*ajDebug("ajFeattableTrimOff offset %d len %d\n", ioffset, ilen);*/
   /* ajDebug("ajFeattableTrimOff table Start %d End %d Len %d Features %d\n",
	     thys->Start, thys->End, thys->Len,
	     ajListGetLength(thys->Features));*/

    iseqlen = ilen + ioffset;
    
    begin = ajFeattablePos(thys, thys->Start);

    if(begin <= ioffset)
	begin = ioffset + 1;

    if(thys->End)
	end = ajFeattablePosI(thys, begin, thys->End);
    else
	end = thys->Len;
    if(end > iseqlen)
	end = iseqlen;
    
    if(begin > 1)
	dobegin = ajTrue;

    if(end < thys->Len)
	doend = ajTrue;
    
    /*ajDebug("  ready to trim dobegin %B doend %B begin %d end %d\n",
	     dobegin, doend, begin, end);*/
    
    iter = ajListIterNew(thys->Features) ;

    while(!ajListIterDone(iter))
    {
	ft = (AjPFeature)ajListIterGet(iter);

	if(!ajFeatTrimOffRange(ft, ioffset, begin, end, dobegin, doend))
	{
	    ajFeatDel(&ft);
	    ajListIterRemove(iter);
	}
    }

    ajListIterDel(&iter);
    thys->Offset = ioffset;

    return ok;
}




/* @func ajFeattableTrim ******************************************************
**
** Trim a feature table using the Begin and Ends.
**
** @param [u] thys [AjPFeattable] Target feature table.
** @return [AjBool] AjTrue returned if successful.
** @@
******************************************************************************/

AjBool ajFeattableTrim(AjPFeattable thys)
{
    AjBool ok      = ajTrue;
    AjBool dobegin = ajFalse;
    AjBool doend   = ajFalse;
    ajuint begin    = 0;
    ajuint end      = 0;
    AjIList     iter = NULL ;
    AjPFeature  ft   = NULL ;
    
   /* ajDebug("ajFeattableTrim table Start %d End %d Len %d Features %d\n",
	     thys->Start, thys->End, thys->Len,
	     ajListGetLength(thys->Features));*/

    begin = ajFeattablePos(thys, thys->Start);

    if(thys->End)
	end = ajFeattablePosI(thys, begin, thys->End);
    else
	end = thys->Len;
    
    if(begin > 1)
	dobegin = ajTrue;

    if(end < thys->Len)
	doend = ajTrue;
    
    /*ajDebug("  ready to trim dobegin %B doend %B begin %d end %d\n",
	     dobegin, doend, begin, end);*/
    
    iter = ajListIterNew(thys->Features) ;

    while(!ajListIterDone(iter))
    {
	ft = (AjPFeature)ajListIterGet(iter);

	if(!ajFeatTrimOffRange(ft, 0, begin, end, dobegin, doend))
	{
	    ajFeatDel(&ft);
	    ajListIterRemove(iter);
	}
    }

    ajListIterDel(&iter);
    thys->Offset = 0;

    return ok;
}




/* @func ajFeatTrimOffRange ***************************************************
**
** Trim a feature table using the Begin and Ends.
**
** Called where a sequence has been trimmed, so we have to allow for
** missing sequence positions at the start (ioffset)
**
** @param [u] ft [AjPFeature] Target feature
** @param [r] ioffset [ajuint] Offset from start of sequence
** @param [r] begin [ajuint] Range start of sequence
** @param [r] end [ajuint] Range end of sequence
** @param [r] dobegin [AjBool] Reset begin
** @param [r] doend [AjBool] Reset end
** @return [AjBool] AjTrue returned if successful.
** @@
******************************************************************************/

AjBool ajFeatTrimOffRange(AjPFeature ft, ajuint ioffset,
			  ajuint begin, ajuint end,
			  AjBool dobegin, AjBool doend)
{
    AjBool ok = ajTrue;

    /* ajDebug("ft flags %x %d..%d %d..%d\n",
	     ft->Flags, ft->Start, ft->End, ft->Start2, ft->End2); */
    
    if(ft->Flags & FEATFLAG_REMOTEID) /* feature in another sequence */
	return ajTrue;

    if(ft->Flags & FEATFLAG_LABEL) /* label, no positions */
	return ajTrue;
    
    if(doend)
    {
	if(ft->Start > end)
	    /* beyond the end - delete this feature */
	    return ajFalse;

	if(ft->Start2 > end)
	    ft->Start2 = end;

	if(ft->End > end)
	{
	    ft->End = end;
	    ft->Flags |= FEATFLAG_END_AFTER_SEQ;
	}

	if(ft->End2 > end)
	    ft->End2 = end;
    }

    if(dobegin)
    {
	if(ft->End < begin)
	    return ajFalse;

	if(begin > (ioffset + 1) && ft->End2 < begin)
	    ft->End2 = begin;

	if(ft->Start && ft->Start < begin)
	{
	    ft->Start = begin;
	    ft->Flags |= FEATFLAG_START_BEFORE_SEQ;
	}

	if(ft->Start2 && ft->Start2 < begin)
	    ft->Start2 = begin;
    }

    if(ioffset)			/* shift to sequence offset */
    {
	if(ft->Start)
	    ft->Start -= ioffset;
	if(ft->Start2)
	    ft->Start2 -= ioffset;
	if(ft->End)
	    ft->End -= ioffset;
	if(ft->End2)
	    ft->End2 -= ioffset;
    }

    return ok;
}




/* @funcstatic featWarn *******************************************************
**
** Formatted write as a warning message.
**
** @param [r] fmt [const char*] Format string
** @param [v] [...] Format arguments.
** @return [void]
** @@
******************************************************************************/

static void featWarn(const char* fmt, ...)
{
    va_list args;
    static AjBool dowarn = AJTRUE;
    AjPStr tmpstr = NULL;
    AjPStr errstr = NULL;

    if(!featWarnCount)
    {
	if(ajNamGetValueC("featwarn", &tmpstr))
	{
	    ajStrToBool(tmpstr, &dowarn);
	}

	ajStrDel(&tmpstr);
    }

    featWarnCount++;

    if(!dowarn)
	return;

    va_start(args, fmt) ;
    ajFmtVPrintS(&errstr, fmt, args);
    va_end(args) ;

    ajWarn("%S", errstr);
    ajStrDel(&errstr);

    return;
}




/* @func ajFeattableWriteDebug ************************************************
**
** Print contents of a feature table to a file
**
** @param [u] ftout [AjPFeattabOut] Feature table output object
** @param [r] ftable [const AjPFeattable] Feature table
** @return [AjBool] Always true
******************************************************************************/

AjBool ajFeattableWriteDebug(AjPFeattabOut ftout, const AjPFeattable ftable)
{
    return ajFeattablePrint(ftable, ftout->Handle);
}




/* @func ajFeattablePrint ******************************************************
**
** Print contents of a feature table to a file
**
** @param [r] ftable [const AjPFeattable] Feature table
** @param [u] outf [AjPFile] Output file object
** @return [AjBool] Always true
******************************************************************************/

AjBool ajFeattablePrint(const AjPFeattable ftable, AjPFile outf)
{
    AjIList iterft     = NULL;
    AjPFeature feature = NULL;
    AjIList itertag    = NULL;
    FeatPTagval tv = NULL;
    ajint i=0;
    ajint j=0;

    ajFmtPrintF(outf, "  DefFormat: %u\n",
		ftable->DefFormat);
    ajFmtPrintF(outf, "  Start: %u\n",
		ftable->Start);
    ajFmtPrintF(outf, "  End: %u\n",
		ftable->End);
    ajFmtPrintF(outf, "  Len: %u\n",
		ftable->Len);
    ajFmtPrintF(outf, "  Offset: %u\n",
		ftable->Offset);
    ajFmtPrintF(outf, "  Groups: %u\n",
		ftable->Groups);

    iterft = ajListIterNewread(ftable->Features);

    while(!ajListIterDone(iterft))
    {
	feature = (AjPFeature)ajListIterGet(iterft);
	i++;

	ajFmtPrintF(outf, "\n  Feature %d\n", i);
	ajFmtPrintF(outf, "    Source: '%S'\n", feature->Source);
	ajFmtPrintF(outf, "    Type: '%S'\n", feature->Type);
	ajFmtPrintF(outf, "    Score: %.6f\n", feature->Score);
	ajFmtPrintF(outf, "    Protein: %B\n", feature->Protein);
	ajFmtPrintF(outf, "    Strand: '%c'\n", feature->Strand);
	ajFmtPrintF(outf, "    Start: %d\n", feature->Start);
	ajFmtPrintF(outf, "    End: %d\n", feature->End);
	ajFmtPrintF(outf, "    Start2: %d\n", feature->Start2);
	ajFmtPrintF(outf, "    End2: %d\n", feature->End2);
	ajFmtPrintF(outf, "    Remote: '%S'\n", feature->Remote);
	ajFmtPrintF(outf, "    Label: '%S'\n", feature->Label);
	ajFmtPrintF(outf, "    Frame: %d\n", feature->Frame);
	ajFmtPrintF(outf, "    Exon: %u\n", feature->Exon);
	ajFmtPrintF(outf, "    Group: %u\n", feature->Group);
	ajFmtPrintF(outf, "    Flags: %x\n", feature->Flags);

	if(feature->Flags & FEATFLAG_START_BEFORE_SEQ)
	    ajFmtPrintF(outf, "      START_BEFORE_SEQ\n");

	if(feature->Flags & FEATFLAG_END_AFTER_SEQ)
	    ajFmtPrintF(outf, "      END_AFTER_SEQ\n");

	if(feature->Flags & FEATFLAG_CHILD)
	    ajFmtPrintF(outf, "      CHILD\n");

	if(feature->Flags & FEATFLAG_BETWEEN_SEQ)
	    ajFmtPrintF(outf, "      BETWEEN_SEQ\n");

	if(feature->Flags & FEATFLAG_START_TWO)
	    ajFmtPrintF(outf, "      START_TWO\n");

	if(feature->Flags & FEATFLAG_END_TWO)
	    ajFmtPrintF(outf, "      END_TWO\n");

	if(feature->Flags & FEATFLAG_POINT)
	    ajFmtPrintF(outf, "      POINT\n");

	if(feature->Flags & FEATFLAG_COMPLEMENT_MAIN)
	    ajFmtPrintF(outf, "      COMPLEMENT_MAIN\n");

	if(feature->Flags & FEATFLAG_MULTIPLE)
	    ajFmtPrintF(outf, "      MULTIPLE\n");

	if(feature->Flags & FEATFLAG_ORDER)
	    ajFmtPrintF(outf, "      ORDER\n");

	if(feature->Flags & FEATFLAG_REMOTEID)
	    ajFmtPrintF(outf, "      REMOTEID\n");

	if(feature->Flags & FEATFLAG_LABEL)
	    ajFmtPrintF(outf, "      LABEL\n");

	if(feature->Flags & FEATFLAG_START_UNSURE)
	    ajFmtPrintF(outf, "      START_UNSURE\n");

	if(feature->Flags & FEATFLAG_END_UNSURE)
	    ajFmtPrintF(outf, "      END_UNSURE\n");

	ajFmtPrintF(outf, "    Tags: %u tags\n",
		    ajListGetLength(feature->Tags));

	j=0;
	itertag = ajListIterNewread(feature->Tags);

	while(!ajListIterDone(itertag))
	{
	    tv = ajListIterGet(itertag);
	    ajFmtPrintF(outf, "      Tag %3d %S : '%S'\n",
			++j, tv->Tag, tv->Value);
	}

	ajListIterDel(&itertag);

    }

    ajListIterDel(&iterft);

    return ajTrue;
}




/* @func ajFeatPrintFormat **************************************************
**
** Reports the internal data structures
**
** @param [u] outf [AjPFile] Output file
** @param [r] full [AjBool] Full report (usually ajFalse)
** @return [void]
** @@
******************************************************************************/

void ajFeatPrintFormat(AjPFile outf, AjBool full)
{
    ajint i = 0;

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# feature input formats\n");
    ajFmtPrintF(outf, "# Name  Format name (or alias)\n");
    ajFmtPrintF(outf, "# Alias Name is an alias\n");
    ajFmtPrintF(outf, "# Nuc   Valid for nucleotide sequences\n");
    ajFmtPrintF(outf, "# Pro   Valid for protein sequences\n");
    ajFmtPrintF(outf, "# Name         Alias   Nuc   Pro "
		"Description\n");
    ajFmtPrintF(outf, "InFormat {\n");

    for(i=0; featInFormatDef[i].Name; i++)
    {
	if(full || !featInFormatDef[i].Alias)
	    ajFmtPrintF(outf, "  %-12s %5B %5B %5B \"%s\"\n",
			featInFormatDef[i].Name,
			featInFormatDef[i].Alias,
			featInFormatDef[i].Nucleotide,
			featInFormatDef[i].Protein,
			featInFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "}\n\n");

    ajFmtPrintF(outf, "# feature output formats\n");
    ajFmtPrintF(outf, "# Name  Format name (or alias)\n");
    ajFmtPrintF(outf, "# Alias Name is an alias\n");
    ajFmtPrintF(outf, "# Nuc   Valid for nucleotide sequences\n");
    ajFmtPrintF(outf, "# Pro   Valid for protein sequences\n");
    ajFmtPrintF(outf, "# Name         Alias   Nuc   Pro "
		"Description\n");
    ajFmtPrintF(outf, "OutFormat {\n");

    for(i=0; featOutFormatDef[i].Name; i++)
    {
	if(full || !featOutFormatDef[i].Alias)
	    ajFmtPrintF(outf, "  %-12s %5B %5B %5B \"%s\"\n",
			featOutFormatDef[i].Name,
			featOutFormatDef[i].Alias,
			featOutFormatDef[i].Nucleotide,
			featOutFormatDef[i].Protein,
			featOutFormatDef[i].Desc);
    }

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajFeatPrintbookFormat ************************************************
**
** Reports the input feature format internals in wiki text format
**
** @param [u] outf [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

void ajFeatPrintbookFormat(AjPFile outf)
{
    ajint i = 0;
    ajint j = 0;
    AjPStr namestr = NULL;

    AjPList fmtlist;
    AjPStr* names;

    fmtlist = ajListstrNew();

    ajFmtPrintF(outf, "<para>The supported feature formats are summarised "
                "in the table below. The columns are as follows: "
                "<emphasis>Output format</emphasis> (format name), "
                "<emphasis>Nuc</emphasis> (\"true\" indicates nucleotide "
                "sequence data may be represented), <emphasis>Pro</emphasis> "
                "(\"true\" indicates protein sequence data may be represented) "
                "and <emphasis>Description</emphasis> (short description of "
                "the format).</para>\n\n");

    ajFmtPrintF(outf, "<table frame=\"box\" rules=\"cols\">\n");
    ajFmtPrintF(outf, "  <caption>Input feature formats</caption>\n");
    ajFmtPrintF(outf, "  <thead>\n");
    ajFmtPrintF(outf, "    <tr align=\"center\">\n");
    ajFmtPrintF(outf, "      <th>Output Format</th>\n");
    ajFmtPrintF(outf, "      <th>Nuc</th>\n");
    ajFmtPrintF(outf, "      <th>Pro</th>\n");
    ajFmtPrintF(outf, "      <th>Description</th>\n");
    ajFmtPrintF(outf, "    </tr>\n");
    ajFmtPrintF(outf, "  </thead>\n");
    ajFmtPrintF(outf, "  <tbody>\n");

    for(i=1; featInFormatDef[i].Name; i++)
    {
	if(!featInFormatDef[i].Alias)
        {
            namestr = ajStrNewC(featInFormatDef[i].Name);
            ajListPush(fmtlist, namestr);
            namestr = NULL;
        }
    }

    ajListSort(fmtlist, ajStrVcmp);
    ajListstrToarray(fmtlist, &names);

    for(i=0; names[i]; i++)
    {
        for(j=0; featInFormatDef[j].Name; j++)
        {
            if(ajStrMatchC(names[i],featInFormatDef[j].Name))
            {
                ajFmtPrintF(outf, "    <tr>\n");
                ajFmtPrintF(outf, "      <td>%s</td>\n",
                            featInFormatDef[j].Name);
                ajFmtPrintF(outf, "      <td>%B</td>\n",
                            featInFormatDef[j].Nucleotide);
                ajFmtPrintF(outf, "      <td>%B</td>\n",
                            featInFormatDef[j].Protein);
                ajFmtPrintF(outf, "      <td>%s</td>\n",
                            featInFormatDef[j].Desc);
                ajFmtPrintF(outf, "    </tr>\n");
            }
        }
    }

    ajFmtPrintF(outf, "  </tbody>\n");
    ajFmtPrintF(outf, "</table>\n");
    ajStrDel(&namestr);

    names = NULL;
    ajListstrFreeData(&fmtlist);

    return;
}




/* @func ajFeatPrintwikiFormat ************************************************
**
** Reports the input feature format internals in wiki text format
**
** @param [u] outf [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

void ajFeatPrintwikiFormat(AjPFile outf)
{
    ajint i = 0;
    ajint j = 0;
    AjPStr namestr = NULL;

    ajFmtPrintF(outf, "{| class=\"wikitable sortable\" border=\"2\"\n");
    ajFmtPrintF(outf, "|-\n");
    ajFmtPrintF(outf, "!Format!!Nuc!!Pro!!"
                "class=\"unsortable\"|Description\n");

    for(i=1; featInFormatDef[i].Name; i++)
    {
        if(!featInFormatDef[i].Alias)
        {
            ajFmtPrintF(outf, "|-\n");
            ajStrAssignC(&namestr, featInFormatDef[i].Name);


            for(j=i+1; featInFormatDef[j].Name; j++)
            {
                if(featInFormatDef[j].Read == featInFormatDef[i].Read)
                {
                    ajFmtPrintAppS(&namestr, "<br>%s", featInFormatDef[j].Name);
                    if(!featInFormatDef[j].Alias) 
                    {
                        ajWarn("Feature input format '%s' same as '%s' "
                               "but not alias",
                               featInFormatDef[j].Name,
                               featInFormatDef[i].Name);
                    }
                }
            }
            ajFmtPrintF(outf, "|%S||%B||%B||%s\n",
			namestr,
			featInFormatDef[i].Nucleotide,
			featInFormatDef[i].Protein,
			featInFormatDef[i].Desc);
        }
    }

    ajFmtPrintF(outf, "|}\n\n");

    ajStrDel(&namestr);

    return;
}




/* @func ajFeatoutPrintbookFormat *********************************************
**
** Reports the output feature format internals in docbook format
**
** @param [u] outf [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

void ajFeatoutPrintbookFormat(AjPFile outf)
{
    ajint i = 0;
    ajint j = 0;
    AjPStr namestr = NULL;
    AjPList fmtlist;
    AjPStr* names;

    fmtlist = ajListstrNew();

    ajFmtPrintF(outf, "<table frame=\"box\" rules=\"cols\">\n");
    ajFmtPrintF(outf, "  <caption>Output feature formats</caption>\n");
    ajFmtPrintF(outf, "  <thead>\n");
    ajFmtPrintF(outf, "    <tr align=\"center\">\n");
    ajFmtPrintF(outf, "      <th>Output Format</th>\n");
    ajFmtPrintF(outf, "      <th>Nuc</th>\n");
    ajFmtPrintF(outf, "      <th>Pro</th>\n");
    ajFmtPrintF(outf, "      <th>Description</th>\n");
    ajFmtPrintF(outf, "    </tr>\n");
    ajFmtPrintF(outf, "  </thead>\n");
    ajFmtPrintF(outf, "  <tbody>\n");

    for(i=1; featOutFormatDef[i].Name; i++)
    {
	if(!featOutFormatDef[i].Alias)
        {
            namestr = ajStrNewC(featOutFormatDef[i].Name);
            ajListPush(fmtlist, namestr);
            namestr = NULL;
        }
    }

    ajListSort(fmtlist, ajStrVcmp);
    ajListstrToarray(fmtlist, &names);

    for(i=0; names[i]; i++)
    {
        for(j=0; featOutFormatDef[j].Name; j++)
        {
            if(ajStrMatchC(names[i],featOutFormatDef[j].Name))
            {
                ajFmtPrintF(outf, "    <tr>\n");
                ajFmtPrintF(outf, "      <td>%s</td>\n",
                            featOutFormatDef[j].Name);
                ajFmtPrintF(outf, "      <td>%B</td>\n",
                            featOutFormatDef[j].Nucleotide);
                ajFmtPrintF(outf, "      <td>%B</td>\n",
                            featOutFormatDef[j].Protein);
                ajFmtPrintF(outf, "      <td>%s</td>\n",
                            featOutFormatDef[j].Desc);
                ajFmtPrintF(outf, "    </tr>\n");
            }
        }
    }

    ajFmtPrintF(outf, "  </tbody>\n");
    ajFmtPrintF(outf, "</table>\n");
    ajStrDel(&namestr);

    names = NULL;
    ajListstrFreeData(&fmtlist);

    return;
}




/* @func ajFeatoutPrintwikiFormat *********************************************
**
** Reports the output feature format internals in wiki text format
**
** @param [u] outf [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

void ajFeatoutPrintwikiFormat(AjPFile outf)
{
    ajint i = 0;
    ajint j = 0;
    AjPStr namestr = NULL;

    ajFmtPrintF(outf, "{| class=\"wikitable sortable\" border=\"2\"\n");
    ajFmtPrintF(outf, "|-\n");
    ajFmtPrintF(outf, "!Format!!Nuc!!Pro!!"
                "class=\"unsortable\"|Description\n");

    for(i=1; featOutFormatDef[i].Name; i++)
    {
        if(!featOutFormatDef[i].Alias)
        {
            ajFmtPrintF(outf, "|-\n");
            ajStrAssignC(&namestr, featOutFormatDef[i].Name);


            for(j=i+1; featOutFormatDef[j].Name; j++)
            {
                if(featOutFormatDef[j].Write == featOutFormatDef[i].Write)
                {
                    ajFmtPrintAppS(&namestr, " %s", featOutFormatDef[j].Name);
                    if(!featOutFormatDef[j].Alias) 
                    {
                        ajWarn("Feature output format '%s' same as '%s' "
                               "but not alias",
                               featOutFormatDef[j].Name,
                               featOutFormatDef[i].Name);
                    }
                }
            }
            ajFmtPrintF(outf, "|%S||%B||%B||%s\n",
			namestr,
			featOutFormatDef[i].Nucleotide,
			featOutFormatDef[i].Protein,
			featOutFormatDef[i].Desc);
        }
    }

    ajFmtPrintF(outf, "|}\n\n");

    ajStrDel(&namestr);

    return;
}




/* @func ajFeattagIsNote *****************************************************
**
** Tests whether the feature tag is a note (the default feature tag)
**
** @param [r] tag      [const AjPStr]  Feature tag
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeattagIsNote(const AjPStr tag)
{
    if(ajStrMatchC(tag, "note"))
	return ajTrue;

    return ajFalse;
}

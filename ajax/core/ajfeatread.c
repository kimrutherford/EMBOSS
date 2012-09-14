/* @source ajfeatread *********************************************************
**
** AJAX feature reading functions
**
** These functions control all aspects of AJAX feature reading
**
** @author Copyright (C) 1999 Richard Bruskiewich
** @version $Revision: 1.56 $
** @modified 2000 Ian Longden.
** @modified 2001 Peter Rice.
** @modified $Date: 2012/07/17 15:04:04 $ by $Author: rice $
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

#include "ajfeatread.h"
#include "ajcall.h"
#include "ajfeat.h"
#include "ajtagval.h"
#include "ajreg.h"
#include "ajdom.h"
#include "ajtextread.h"
#include "ajfileio.h"
#include "ajquery.h"
#include "ajnam.h"


#include <limits.h>
#include <math.h>
#include <errno.h>

#ifdef WIN32
#define fileno _fileno
#endif

AjPTable feattabDbMethods = NULL;

static AjPRegexp featinRegUfoFmt = NULL;
static AjPRegexp featinRegUfoFile = NULL;
static AjPRegexp featRegFlag = NULL;
static AjPRegexp featRegMore = NULL;
static AjPRegexp featRegGroup = NULL;
static AjPRegexp featRegGff3Group = NULL;

static AjPStr    featinUfoTest = NULL;
static AjPStr    featReadLine   = NULL;
static AjPStr    featinTagNote = NULL;
static AjPStr    featinTagComm = NULL;
static AjPStr    featinTagFtid = NULL;
static AjPStr    featinSourcePir      = NULL;
static AjPStr    featinSourceSwiss    = NULL;
static AjPStr    featinSourceRefseqp  = NULL;
static AjPStr    featinSourceEmbl     = NULL;

static AjBool   FeatInitGff2      = AJFALSE;
static AjBool   FeatInitGff3      = AJFALSE;
static AjBool   FeatInitEmbl      = AJFALSE;
static AjBool   FeatInitPir      = AJFALSE;
static AjBool   FeatInitSwiss      = AJFALSE;
static AjBool   FeatInitRefseqp    = AJFALSE;
static AjPStr featProcessLine   = NULL;

static AjPStr featGroup = NULL;
static AjPStr featId = NULL;
static AjPStr featLabel   = NULL;

static AjPStr featSeqid    = NULL;
static AjPStr featSource    = NULL;
static AjPStr featFeature   = NULL;
static AjPStrTok featGffSplit  = NULL;
static AjPStrTok featEmblSplit  = NULL;
static AjPStr featLocStr   = NULL;
static AjPStr featLocToken = NULL;
static AjPStr featLocDb = NULL;
static AjPStr featSaveGroupStr   = NULL;
static ajint featSaveExon  = 0;
static ajint featSaveGroup = 0;
static AjPStr featinFormatTmp = NULL;
static AjPStr featinValTmp = NULL;
static AjPStr featinTmpStr  = NULL;



static void         featGff3Matchtable(AjPFeattable ftable,
                                       AjPTable *idtable, AjPList *childlist);
static AjBool       featFindInformatC(const char* format, ajint* iformat);
static AjBool       featFindInformatS(const AjPStr format, ajint* iformat);
static AjBool       featDelRegEmbl(void);
static AjBool       featDelRegGff2(void);
static AjBool       featDelRegGff3(void);
static AjBool       featDelRegPir(void);
static AjBool       featDelRegRefseqp(void);
static AjBool       featDelRegSwiss(void);
static AjBool       featEmblLoc(const AjPStr loc,
				AjPStr* begstr, AjBool* between,
				AjBool* simple, AjPStr* endstr);
static AjBool       featEmblLocNum(const AjPStr loc,
				   AjBool* bound, ajuint* num);
static AjBool       featEmblLocRange(const AjPStr loc,
				     ajuint* num1, ajuint* num2);
static AjBool       featEmblOperIn(const AjPStr loc, AjPStr* opnam,
				   AjPStr* value, AjPStr* rest);
static AjBool       featEmblOperNone(const AjPStr loc, AjPStr* entryid,
				     AjPStr* value, AjPStr* rest);
static AjBool       featEmblOperOut(const AjPStr loc,
				    AjPStr* opnam, AjPStr* value);
static AjBool       featFormatSet(AjPFeattabin featin);
static AjPFeature   featPirFromLine(AjPFeattable thys,
				    const AjPStr origline);
static AjPFeature   featSwissFromLine(AjPFeattable thys, const AjPStr line,
				      AjPStr* savefeat, AjPStr* savefrom,
				      AjPStr* saveto, AjPStr* saveline);
static AjPFeature   featSwissProcess(AjPFeattable thys, const AjPStr feature,
				     const AjPStr fromstr, const AjPStr tostr,
				     const AjPStr source,
				     const AjPStr tags);
static AjBool       featReadChado(AjPFeattabin feattabin,
                                  AjPFeattable ftable);
static AjBool       featReadDasgff(AjPFeattabin feattabin,
                                   AjPFeattable ftable);
static AjBool       featReadEmbl(AjPFeattabin feattabin,
                                 AjPFeattable ftable);
static AjBool       featReadGenpept(AjPFeattabin feattabin,
                                    AjPFeattable ftable);
static AjBool       featReadRefseq(AjPFeattabin feattabin,
                                   AjPFeattable ftable);
static AjBool       featReadRefseqp(AjPFeattabin feattabin,
                                    AjPFeattable ftable);
static AjBool       featReadGff2(AjPFeattabin feattabin,
                                AjPFeattable ftable);
static AjBool       featReadGff3(AjPFeattabin feattabin,
                                 AjPFeattable ftable);
static AjBool       featReadGff3old(AjPFeattabin feattabin,
                                    AjPFeattable ftable);
static AjBool       featReadPir(AjPFeattabin feattabin,
                                AjPFeattable ftable);
static AjBool       featReadSwiss(AjPFeattabin feattabin,
                                  AjPFeattable ftable);

static AjBool       featRegInitEmbl(void);
static AjBool       featRegInitGff2(void);
static AjBool       featRegInitGff3(void);
static AjBool       featRegInitPir(void);
static AjBool       featRegInitRefseqp(void);
static AjBool       featRegInitSwiss(void);

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
static AjPFeature  featGff2FromLine(AjPFeattable thys, const AjPStr line,
                                    float version);

static AjPFeature  featGff3FromLine(AjPFeattable thys, const AjPStr line,
                                    AjPTable idtable, AjPList childlist);
static AjPFeature  featGff3oldFromLine(AjPFeattable thys, const AjPStr line);

static void        featGff2ProcessTagval(AjPFeature gf,
					AjPFeattable table,
					const AjPStr groupfield,
					float version);

static AjPStr      featGff3ProcessTagval(AjPFeature gf,
                                         AjPFeattable table,
                                         const AjPStr groupfield,
                                         AjBool *parent);
static void         featGff3oldProcessTagval(AjPFeature gf,
                                             AjPFeattable table,
                                             const AjPStr groupfield);
static AjBool       featEmblTvRest(AjPStr* tags, AjPStr* skip);
static AjBool       featEmblTvTagVal(AjPStr* tags, AjPStr* tag, AjPStr* value);
static void         featGff3FlagSet(AjPFeature gf, const AjPStr flags);
static void         featFlagSet(AjPFeature gf, const AjPStr flags);
static void         featGroupSet(AjPFeature gf, AjPFeattable table,
				 const AjPStr grouptag);
static void         featGff3GroupSet(AjPFeature gf, AjPFeattable table,
                                     const AjPStr grouptag);

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
static AjPRegexp Gff3Regexdirective = NULL;
static AjPRegexp Gff3Regextype      = NULL;

static AjPRegexp Gff3RegexTvTagval  = NULL;
static AjPRegexp Gff3oldRegexTvTagval  = NULL;

static AjPRegexp PirRegexAll          = NULL;
static AjPRegexp PirRegexCom          = NULL;
static AjPRegexp PirRegexLoc          = NULL;
static AjPRegexp PirRegexPos          = NULL;

static AjPRegexp SwRegexComment       = NULL;
static AjPRegexp SwRegexFtid          = NULL;
static AjPRegexp SwRegexNew           = NULL;
static AjPRegexp SwRegexNext          = NULL;

static AjPRegexp featTagTrans = NULL;    



/* @datastatic FeatPListUfo ***************************************************
**
** Usa processing list of UFOs from a list file.
**
** Includes data from the original UFO (@listfile)
**
** @alias FeatSListUfo
** @alias FeatOListUfo
**
** @attr Begin [ajint] Begin if defined in original UFO
** @attr End [ajint] End if defined in original UFO
** @attr Rev [AjBool] Reverse if defined in original UFO
** @attr Format [ajuint] Format number from original UFO
** @attr Formatstr [AjPStr] Format name from original UFO
** @attr Ufo [AjPStr] Current UFO
** @attr Fpos [ajulong] Start position offset
** @attr Features [AjBool] if true, process features
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct FeatSListUfo
{
    ajint Begin;
    ajint End;
    AjBool Rev;
    ajuint Format;
    AjPStr Formatstr;
    AjPStr Ufo;
    ajulong Fpos;
    AjBool Features;
    char Padding[4];
} FeatOListUfo;

#define FeatPListUfo FeatOListUfo*




/* @datastatic FeatPInformat **************************************************
**
** Feature input format definition
**
** @alias FeatSInformat
** @alias FeatOInformat
**
** @attr Name [const char*] Input format name
** @attr Obo  [const char*] Ontology term id from EDAM
** @attr Alias [AjBool] True if name is an alias for an identical definition
** @attr Try [AjBool] If true, try for an unknown input. Duplicate names
**                    and read-anything formats are set false
** @attr Nucleotide [AjBool] True if suitable for nucleotide data
** @attr Protein [AjBool] True if suitable for protein data
** @attr Used [AjBool] True if already used (initialised)
** @attr Padding [AjBool] Padding to alignment boundary
** @attr Read [AjBool function] Function to read feature data
** @attr InitReg [AjBool function] Function to initialise regular expressions
** @attr DelReg [AjBool function] Function to clean up regular expressions
** @attr Desc [const char*] Description
** @@
******************************************************************************/

typedef struct FeatSInformat
{
    const char *Name;
    const char *Obo;
    AjBool Alias;
    AjBool Try;
    AjBool Nucleotide;
    AjBool Protein;
    AjBool Used;
    AjBool Padding;
    AjBool (*Read) (AjPFeattabin thys, AjPFeattable ftable);
    AjBool (*InitReg) (void);
    AjBool (*DelReg) (void);
    const char *Desc;
} FeatOInformat;

#define FeatPInformat FeatOInformat*

/* name             Dna   Protein
   input-function   init-regex-function del-regex-function */




/* @funclist featinformatDef **************************************************
**
** Input feature formats
**
** Includes the read function (featRead), and initialising (featRegInit)
** and deletion (featDelReg) of parsing regular expression.
**
******************************************************************************/

static FeatOInformat featinformatDef[] =
{
    /*Name            OBO
         Alias    Try      Dna      Prot     Used (initially false) Padding
         ReadFunction      RegInitFunction    RegDelFunction
         Description*/
    {"unknown",       "0000",
         AJFALSE, AJFALSE, AJFALSE, AJFALSE, AJFALSE, AJFALSE,
         &featReadGff3,    NULL,               NULL,
         "unknown format"},
    {"embl",          "1927",
         AJFALSE, AJTRUE,  AJTRUE,  AJFALSE, AJFALSE, AJFALSE,
	 &featReadEmbl,    &featRegInitEmbl,    &featDelRegEmbl,
	 "embl/genbank/ddbj format"},
    {"em",            "1927",
         AJTRUE,  AJFALSE, AJTRUE,  AJFALSE, AJFALSE, AJFALSE,
	 &featReadEmbl,    &featRegInitEmbl,    &featDelRegEmbl,
	 "embl/genbank/ddbj format"},
    {"genbank",       "1936",
         AJTRUE,  AJFALSE, AJTRUE,  AJFALSE, AJFALSE, AJFALSE,
	 &featReadEmbl,    &featRegInitEmbl,    &featDelRegEmbl,
	 "embl/genbank/ddbj format"},
    {"gb",            "1936",
         AJTRUE,  AJFALSE, AJTRUE,  AJFALSE, AJFALSE, AJFALSE,
	 &featReadEmbl,    &featRegInitEmbl,    &featDelRegEmbl,
	 "embl/genbank/ddbj format"},
    {"ddbj",          "1936",
         AJTRUE,  AJFALSE, AJTRUE,  AJFALSE, AJFALSE, AJFALSE,
	 &featReadEmbl,    &featRegInitEmbl,    &featDelRegEmbl,
	 "embl/genbank/ddbj format"},
    {"refseq",        "1958",
         AJFALSE, AJTRUE,  AJTRUE,  AJFALSE, AJFALSE, AJFALSE,
	 &featReadRefseq, & featRegInitEmbl,    &featDelRegEmbl,
	 "embl/genbank/ddbj format"},
    {"refseqp",       "0000",
         AJFALSE, AJTRUE,  AJFALSE, AJTRUE, AJFALSE, AJFALSE,
	 &featReadRefseqp, &featRegInitRefseqp, &featDelRegRefseqp,
	 "RefSeq protein format"},
    {"genpept",       "0000",
         AJFALSE, AJTRUE,  AJFALSE, AJTRUE,  AJFALSE, AJFALSE,
	 &featReadGenpept, &featRegInitSwiss,    &featDelRegSwiss,
	 "genpept format"},
    {"gff3",          "1939",
         AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  AJFALSE, AJFALSE,
	 &featReadGff3,    &featRegInitGff3,    &featDelRegGff3,
	 "GFF version 3"},
    {"gff3emboss",    "0000",
         AJFALSE, AJFALSE, AJTRUE,  AJTRUE,  AJFALSE, AJFALSE,
	 &featReadGff3old, &featRegInitGff3,    &featDelRegGff3,
	 "GFF version 3 written by EMBOSS before release 6.4.0"},
    {"gff2",          "1938",
         AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  AJFALSE, AJFALSE,
	 &featReadGff2,    &featRegInitGff2,    &featDelRegGff2,
	 "GFF version 1 or 2"},
    {"gff",           "1939",
         AJTRUE,  AJFALSE, AJTRUE,  AJTRUE,  AJFALSE, AJFALSE,
	 &featReadGff3,    &featRegInitGff3,    &featDelRegGff3,
	 "GFF version3"},
    {"swiss",         "1963",
         AJFALSE, AJTRUE,  AJFALSE, AJTRUE,  AJFALSE, AJFALSE,
	 &featReadSwiss,   &featRegInitSwiss,   &featDelRegSwiss,
	 "SwissProt format"},
    {"sw",            "2187",
         AJTRUE,  AJFALSE, AJFALSE, AJTRUE,  AJFALSE, AJFALSE,
	 &featReadSwiss,   &featRegInitSwiss,   &featDelRegSwiss,
	 "SwissProt format"},
    {"uniprot",       "2188",
         AJTRUE,  AJFALSE, AJFALSE, AJTRUE,  AJFALSE, AJFALSE,
	 &featReadSwiss,   &featRegInitSwiss,   &featDelRegSwiss,
	 "SwissProt format"},
    {"swissprot",     "0000",
         AJTRUE,  AJTRUE,  AJFALSE, AJTRUE,  AJFALSE, AJFALSE,
	 &featReadSwiss,   &featRegInitSwiss,   &featDelRegSwiss,
	 "SwissProt format"},
    {"pir",           "1948",
         AJFALSE, AJTRUE,  AJFALSE, AJTRUE,  AJFALSE, AJFALSE,
	 &featReadPir,     &featRegInitPir,     &featDelRegPir,
	 "PIR format"},
    {"nbrf",          "1948",
         AJTRUE,  AJFALSE, AJFALSE, AJTRUE,  AJFALSE, AJFALSE,
	 &featReadPir,     &featRegInitPir,     &featDelRegPir,
	 "PIR format"},
    {"dasgff",        "1978",
         AJFALSE, AJTRUE,  AJTRUE,  AJFALSE, AJFALSE, AJFALSE,
	 &featReadDasgff,  NULL,  NULL,
	 "DAS versions 1.5 or 1.6"},
    {"chado",         "0000",
         AJFALSE, AJTRUE,  AJTRUE,  AJFALSE, AJFALSE, AJFALSE,
	 &featReadChado,  NULL,  NULL,
	 "CHADO"},
    {NULL,            NULL,
         AJFALSE, AJFALSE, AJFALSE, AJFALSE, AJFALSE, AJFALSE,
         NULL,             NULL,                NULL,
         NULL}
};




/* @datastatic FeatPTypein ****************************************************
**
** feature input types
**
** @alias FeatSTypein
** @alias FeatOTypein
**
** @attr Name [const char*] Specified name
** @attr Value [const char*] Internal type "P" or "N"
** @@
******************************************************************************/

typedef struct FeatSTypein
{
    const char* Name;
    const char* Value;
} FeatOTypein;

#define FeatPTypein FeatOTypein*




static FeatOTypein featinTypes[] =
{
    {"P", "P"},
    {"protein", "P"},
    {"protfeatures", "P"},
    {"N", "N"},
    {"nucleotide", "N"},
    {"nucfeatures", "N"},
    {"any", ""},
    {"features", ""},
    {NULL, NULL}
};




static ajuint feattabinReadFmt(AjPFeattabin feattabin,
                               AjPFeattable ftable,
                                ajuint format);
static AjBool feattabinRead(AjPFeattabin feattabin,
                             AjPFeattable ftable);
static AjBool feattabinformatFind(const AjPStr format,
                                  ajint* iformat);
static AjBool feattabinFormatSet(AjPFeattabin feattabin,
                                  AjPFeattable ftable);
static AjBool feattabinListProcess(AjPFeattabin feattabin,
                                   AjPFeattable ftable,
                                    const AjPStr listfile);
static void   feattabinListNoComment(AjPStr* text);
static void   feattabinQryRestore(AjPFeattabin feattabin,
                                   const FeatPListUfo node);
static void   feattabinQrySave(FeatPListUfo node,
                               const AjPFeattabin feattabin);
static AjBool feattabinQryProcess(AjPFeattabin feattabin,
                                   AjPFeattable ftable);
static AjBool feattabinQueryMatch(const AjPQuery thys,
                                   const AjPFeattable ftable);
static AjBool feattableDefine(AjPFeattable thys,
                              AjPFeattabin feattabin);




/* @func ajFeattabinNew *******************************************************
**
** Constructor for an empty feature table input object
**
** @return [AjPFeattabin] Feature table input object
** @category new [AjPFeattabin] Constructor
**
** @release 6.4.0
** @@
******************************************************************************/

AjPFeattabin ajFeattabinNew(void)
{
    AjPFeattabin pthis;
    AJNEW0(pthis);

    pthis->Input = ajTextinNewDatatype(AJDATATYPE_FEATURES);

   /* ajDebug("ajFeatTabinNew %x\n", pthis);*/

    return pthis;
}




/* @func ajFeattabinNewSS *****************************************************
**
** Constructor for an empty feature table input object. The format and
** name are read.
**
** @param [r] fmt [const AjPStr] feature format
** @param [r] name [const AjPStr] sequence name
** @param [r] type [const char*] feature type
** @return [AjPFeattabin] Feature table input object
** @category new [AjPFeattabin] Constructor with format, name and type
**
** @release 6.4.0
** @@
******************************************************************************/

AjPFeattabin ajFeattabinNewSS(const AjPStr fmt, const AjPStr name,
			      const char* type)
{
    AjPFeattabin pthis;
    ajint iformat = 0;

    if(!featFindInformatS(fmt, &iformat))
	return NULL;

    pthis = ajFeattabinNew();
    ajStrAssignC(&pthis->Formatstr, featinformatDef[pthis->Input->Format].Name);
    pthis->Input->Format = iformat;
    ajStrAssignC(&pthis->Type, type);
    ajStrAssignS(&pthis->Seqname, name);
    pthis->Input->Filebuff = ajFilebuffNewNofile();

    return pthis;
}




/* @func ajFeattabinNewCSF ****************************************************
**
** Constructor for an empty feature table input object. The format and
** name are read. The file buffer is moved to the feature table input
** object and should not be deleted by the calling program.
**
** @param [r] fmt [const char*] feature format
** @param [r] name [const AjPStr] sequence name
** @param [r] type [const char*] feature type
** @param [u] buff [AjPFilebuff] Buffer containing feature data
** @return [AjPFeattabin] Feature table input object
** @category new [AjPFeattabin] Constructor with format, name, type
**                              and input file
**
** @release 6.4.0
** @@
******************************************************************************/

AjPFeattabin ajFeattabinNewCSF(const char* fmt, const AjPStr name,
				const char* type, AjPFilebuff buff)
{
    AjPFeattabin pthis;
    ajint iformat = 0;

    if(!featFindInformatC(fmt, &iformat))
	return NULL;

    pthis = ajFeattabinNew();
    ajStrAssignC(&pthis->Formatstr, featinformatDef[iformat].Name);
    pthis->Input->Format = iformat;
    ajStrAssignC(&pthis->Type, type);
    ajStrAssignS(&pthis->Seqname, name);
    pthis->Local = ajTrue;
    pthis->Input->Filebuff = buff;

    return pthis;
}




/* @func ajFeattabinNewSSF ****************************************************
**
** Constructor for an empty feature table input object. The format and
** name are read. The file buffer is moved to the feature table input
** object and should not be deleted by the calling program.
**
** @param [r] fmt [const AjPStr] feature format
** @param [r] name [const AjPStr] sequence name
** @param [r] type [const char*] feature type
** @param [u] buff [AjPFilebuff] Buffer containing feature data
** @return [AjPFeattabin] Feature table input object
** @category new [AjPFeattabin] Constructor with format, name, type
**                              and input file
**
** @release 6.4.0
** @@
******************************************************************************/

AjPFeattabin ajFeattabinNewSSF(const AjPStr fmt, const AjPStr name,
				const char* type, AjPFilebuff buff)
{
    AjPFeattabin pthis;
    ajint iformat = 0;

    if(!featFindInformatS(fmt, &iformat))
	return NULL;

    pthis = ajFeattabinNew();
    ajStrAssignC(&pthis->Formatstr, featinformatDef[iformat].Name);
    pthis->Input->Format = iformat;
    ajStrAssignC(&pthis->Type, type);
    ajStrAssignS(&pthis->Seqname, name);
    pthis->Local = ajTrue;
    pthis->Input->Filebuff = buff;

    return pthis;
}




/* @func ajFeattabinDel *******************************************************
**
** Destructor for a feature table input object
**
** @param [d] pthis [AjPFeattabin*] Feature table input object
** @return [void]
** @category delete [AjPFeattabin] Destructor
**
** @release 6.4.0
** @@
******************************************************************************/

void ajFeattabinDel(AjPFeattabin* pthis)
{
    AjPFeattabin thys;

    thys = *pthis;

    if(!thys)
	return;

    ajTextinDel(&thys->Input);
    
    ajStrDel(&thys->Ufo);
    ajStrDel(&thys->Formatstr);
    ajStrDel(&thys->Filename);
    ajStrDel(&thys->Filename);
    ajStrDel(&thys->Seqid);
    ajStrDel(&thys->Seqname);
    ajStrDel(&thys->Type);
    AJFREE(*pthis);

    return;
}





/* @func ajFeattabinClear *****************************************************
**
** Clears a feature table input object back to "as new" condition, except
** for the USA list which must be preserved.
**
** @param [u] thys [AjPFeattabin] Sequence input
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajFeattabinClear(AjPFeattabin thys)
{
    ajDebug("ajFeattabinClear called Local:%B\n", thys->Local);

    if(!thys)
        return;

    ajStrSetClear(&thys->Ufo);
    ajStrSetClear(&thys->Seqname);
    ajStrSetClear(&thys->Formatstr);
    ajStrSetClear(&thys->Filename);
    ajStrSetClear(&thys->Seqid);
    ajStrSetClear(&thys->Type);

    if(!thys->Local)
        ajTextinClear(thys->Input);
    else
        ajTextinClearNofile(thys->Input);

    thys->Start = 0;
    thys->End = 0;
    thys->Rev = ajFalse;

    return;
}




/* @func ajFeattabinSetRange **************************************************
**
** Set the begin and end range for a feature table
**
** @param [u] thys [AjPFeattabin] Feature table input object
** @param [r] fbegin [ajint] Begin position
** @param [r] fend   [ajint] End position
** @return [void]
**
** @release 6.4.0
******************************************************************************/

void ajFeattabinSetRange(AjPFeattabin thys, ajint fbegin, ajint fend)
{
    if(fbegin)
	thys->Start = fbegin;

    if(fend)
	thys->End = fend;

    return;
}




/* @func ajFeattabinSetTypeC **************************************************
**
** Sets the type for feature input
**
** @param [u] thys [AjPFeattabin] Feature input object
** @param [r] type [const char*] Feature type "nucleotide" "protein"
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajFeattabinSetTypeC(AjPFeattabin thys, const char* type)
{
    ajint i = 0;

    if(!*type)
	return ajTrue;

    for(i=0; featinTypes[i].Name; i++)
    {
	if(ajCharMatchCaseC(featinTypes[i].Name, type))
	{
	    if(featinTypes[i].Value)
		ajStrAssignC(&thys->Type, featinTypes[i].Value);

	    return ajTrue;
	}
	i++;
    }

    ajErr("Unrecognized feature input type '%s'", type);

    return ajFalse;
}




/* @func ajFeattabinSetTypeS **************************************************
**
** Sets the type for feature input
**
** @param [u] thys [AjPFeattabin] Feature input object
** @param [r] type [const AjPStr] Feature type "nucleotide" "protein"
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajFeattabinSetTypeS(AjPFeattabin thys, const AjPStr type)
{
    return ajFeattabinSetTypeC(thys, ajStrGetPtr(type));
}




/* @func ajFeattabinQryC ******************************************************
**
** Resets a feature table input object using a new Universal
** Query Address
**
** @param [u] thys [AjPFeattabin] Feature table input object.
** @param [r] txt [const char*] Query
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajFeattabinQryC(AjPFeattabin thys, const char* txt)
{
    ajFeattabinClear(thys);
    ajStrAssignC(&thys->Input->Qry, txt);

    return;
}





/* @func ajFeattabinQryS ******************************************************
**
** Resets a feature table input object using a new Universal
** Query Address
**
** @param [u] thys [AjPFeattabin] Feature table input object.
** @param [r] str [const AjPStr] Query
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajFeattabinQryS(AjPFeattabin thys, const AjPStr str)
{
    ajFeattabinClear(thys);
    ajStrAssignS(&thys->Input->Qry, str);

    return;
}




/* @func ajFeattabinRead ******************************************************
**
** Generic interface function for reading in features from a file
** given the file handle.
**
** @param [u] feattabin [AjPFeattabin] Feature table input
** @param [u] ftable    [AjPFeattable] Feature table containing
**                                     the features read in
** @return [AjBool] True on success
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajFeattabinRead(AjPFeattabin feattabin, AjPFeattable ftable)
{
    AjBool ret       = ajFalse;
    FeatPListUfo node = NULL;
    AjBool listdata  = ajFalse;

    if(feattabin->Input->Filebuff)
    {
	/* (a) if file still open, keep reading */
	ajDebug("ajFeattabinRead: input file '%F' still there, try again\n",
		feattabin->Input->Filebuff->File);
	ret = feattabinRead(feattabin, ftable);
	ajDebug("ajFeattabinRead: open buffer  qry: '%S' returns: %B\n",
		feattabin->Input->Qry, ret);
    }
    else
    {
	/* (b) if we have a list, try the next query in the list */
	if(ajListGetLength(feattabin->Input->List))
	{
	    listdata = ajTrue;
	    ajListPop(feattabin->Input->List, (void**) &node);

	    ajDebug("++pop from list '%S'\n", node->Ufo);
	    ajFeattabinQryS(feattabin, node->Ufo);
	    ajDebug("++SAVE FEATTABIN '%S' '%S' %d\n",
		    feattabin->Input->Qry,
		    feattabin->Input->Formatstr, feattabin->Input->Format);

            feattabinQryRestore(feattabin, node);

	    ajStrDel(&node->Ufo);
	    ajStrDel(&node->Formatstr);
	    AJFREE(node);

	    ajDebug("ajFeattabinRead: open list, try '%S'\n",
                    feattabin->Input->Qry);

	    if(!feattabinQryProcess(feattabin, ftable) &&
               !ajListGetLength(feattabin->Input->List))
		return ajFalse;

	    ret = feattabinRead(feattabin, ftable);
	    ajDebug("ajFeattabinRead: list qry: '%S' returns: %B\n",
		    feattabin->Input->Qry, ret);
	}
	else
	{
	    ajDebug("ajFeattabinRead: no file yet - test query '%S'\n",
                    feattabin->Input->Qry);

	    /* (c) Must be a query - decode it */
	    if(!feattabinQryProcess(feattabin, ftable) &&
               !ajListGetLength(feattabin->Input->List))
		return ajFalse;

	    if(ajListGetLength(feattabin->Input->List)) /* could be new list */
		listdata = ajTrue;

	    ret = feattabinRead(feattabin, ftable);
	    ajDebug("ajFeattabinRead: new qry: '%S' returns: %B\n",
		    feattabin->Input->Qry, ret);
	}
    }

    /* Now read whatever we got */

    while(!ret && ajListGetLength(feattabin->Input->List))
    {
	/* Failed, but we have a list still - keep trying it */
        if(listdata)
	    ajErr("Failed to read data data '%S'",
                  feattabin->Input->Qry);

	listdata = ajTrue;
	ajListPop(feattabin->Input->List,(void**) &node);
	ajDebug("++try again: pop from list '%S'\n", node->Ufo);
	ajFeattabinQryS(feattabin, node->Ufo);
	ajDebug("++SAVE (AGAIN) FEATTABIN '%S' '%S' %d\n",
		feattabin->Input->Qry,
		feattabin->Input->Formatstr, feattabin->Input->Format);

	feattabinQryRestore(feattabin, node);

	ajStrDel(&node->Ufo);
	ajStrDel(&node->Formatstr);
	AJFREE(node);

	if(!feattabinQryProcess(feattabin, ftable))
	    continue;

	ret = feattabinRead(feattabin, ftable);
	ajDebug("ajFeattabinRead: list retry qry: '%S' returns: %B\n",
		feattabin->Input->Qry, ret);
    }

    if(!ret)
    {
	if(listdata)
	    ajErr("Failed to read data data '%S'",
                  feattabin->Input->Qry);

	return ajFalse;
    }


    feattableDefine(ftable, feattabin);

    return ajTrue;
}




/* @funcstatic feattabinQueryMatch ********************************************
**
** Compares a feature table item to a query and returns true if they match.
**
** @param [r] thys [const AjPQuery] query.
** @param [r] ftable [const AjPFeattable] Feature table data.
** @return [AjBool] ajTrue if the data matches the query.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool feattabinQueryMatch(const AjPQuery thys,
                                  const AjPFeattable ftable)
{
    AjBool tested = ajFalse;
    AjIList iterfield  = NULL;
    AjPQueryField field = NULL;
    AjBool ok = ajFalse;

    ajDebug("feattabinQueryMatch '%S' fields: %Lu Case %B Done %B\n",
	    ftable->Seqid, ajListGetLength(thys->QueryFields),
            thys->CaseId, thys->QryDone);

    if(!thys)			   /* no query to test, that's fine */
	return ajTrue;

    if(thys->QryDone)			/* do we need to test here? */
	return ajTrue;

    /* test the query field(s) */

    iterfield = ajListIterNewread(thys->QueryFields);
    while(!ajListIterDone(iterfield))
    {
        field = ajListIterGet(iterfield);

        ajDebug("  field: '%S' Query: '%S'\n",
                field->Field, field->Wildquery);
        if(ajStrMatchC(field->Field, "id"))
        {
            ajDebug("  id test: '%S'\n",
                    ftable->Seqid);
            if(thys->CaseId)
            {
                if(ajStrMatchWildS(ftable->Seqid, field->Wildquery))
                {
                    ajListIterDel(&iterfield);
                    return ajTrue;
                }
            }
            else
            {
                if(ajStrMatchWildCaseS(ftable->Seqid, field->Wildquery))
               {
                    ajListIterDel(&iterfield);
                    return ajTrue;
                }
            }
            
            ajDebug("id test failed\n");
            tested = ajTrue;
            ok = ajFalse;
        }

        if(ajStrMatchC(field->Field, "acc")) /* test id, use trueid */
        {
            if(ajStrMatchWildCaseS(ftable->Seqid, field->Wildquery))
            {
                ajListIterDel(&iterfield);
                return ajTrue;
            }
        }

    }
        
    ajListIterDel(&iterfield);

    if(!tested)		    /* nothing to test, so accept it anyway */
    {
        ajDebug("  no tests: assume OK\n");
	return ajTrue;
    }
    
    ajDebug("result: %B\n", ok);

    return ok;
}




/* @funcstatic feattableDefine ************************************************
**
** Make sure all feature table object attributes are defined
** using values from the feature table input object if needed
**
** @param [u] thys [AjPFeattable] feature table data returned.
** @param [u] feattabin [AjPFeattabin] Feature table input definitions
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool feattableDefine(AjPFeattable thys, AjPFeattabin feattabin)
{

    /* if values are missing in the data object, we can use defaults
       from feattabin or calculate where possible */

    /* assign the dbname if defined in the AjPFeattabin object */
    if(ajStrGetLen(feattabin->Input->Db))
      ajStrAssignS(&thys->Db, feattabin->Input->Db);

    return ajTrue;
}





/* @funcstatic feattabinReadFmt ***********************************************
**
** Tests whether data data can be read using the specified format.
** Then tests whether the data data matches data data query criteria
** and checks any specified type. Applies upper and lower case.
**
** @param [u] feattabin [AjPFeattabin] Feature table input object
** @param [w] ftable [AjPFeattable] Feature table data object
** @param [r] format [ajuint] input format code
** @return [ajuint] 0 if successful.
**                  1 if the query match failed.
**                  2 if the data data type failed
**                  3 if it failed to read any data data
**
** @release 6.4.0
** @@
** This is the only function that calls the appropriate Read function
** feattabinReadXxxxxx where Xxxxxxx is the supported data data format.
**
** Some of the feattabinReadXxxxxx functions fail to reset the buffer correctly,
** which is a very serious problem when cycling through all of them to
** identify an unknown format. The extra ajFileBuffReset call at the end is
** intended to address this problem. The individual functions should still
** reset the buffer in case they are called from elsewhere.
**
******************************************************************************/

static ajuint feattabinReadFmt(AjPFeattabin feattabin, AjPFeattable ftable,
                               ajuint format)
{
    ajDebug("++feattabinReadFmt format %d (%s) '%S'\n",
	    format, featinformatDef[format].Name,
	    feattabin->Input->Qry);

    feattabin->Input->Records = 0;

    if(!featinformatDef[format].Used)
    {
        if(featinformatDef[format].InitReg &&
           !(*featinformatDef[format].InitReg)())
        {
            ajDebug("Initialisation failed for %s\n",
                    featinformatDef[format].Name);
            ajErr("Initialisation failed for feature format %s",
                  featinformatDef[format].Name);
        }
        featinformatDef[format].Used = ajTrue;
    }

    /* Calling funclist featinFormatDef() */
    if((*featinformatDef[format].Read)(feattabin, ftable))
    {
	ajDebug("feattabinReadFmt success with format %d (%s)\n",
		format, featinformatDef[format].Name);
        ajDebug("id: '%S'\n",
                ftable->Seqid);
	feattabin->Input->Format = format;
	ajStrAssignC(&feattabin->Input->Formatstr,
                     featinformatDef[format].Name);
	ajStrAssignC(&ftable->Formatstr, featinformatDef[format].Name);
	ajStrAssignEmptyS(&ftable->Db, feattabin->Input->Db);
	ajStrAssignS(&ftable->Filename, feattabin->Input->Filename);

	if(feattabinQueryMatch(feattabin->Input->Query, ftable))
	{
            /* ajFeattabinTrace(feattabin); */

            return FMT_OK;
        }

	ajDebug("query match failed, continuing ...\n");
	ajFeattableClear(ftable);

	return FMT_NOMATCH;
    }
    else
    {
	ajDebug("Testing input buffer: IsBuff: %B Eof: %B\n",
		ajFilebuffIsBuffered(feattabin->Input->Filebuff),
		ajFilebuffIsEof(feattabin->Input->Filebuff));

	if (!ajFilebuffIsBuffered(feattabin->Input->Filebuff) &&
	    ajFilebuffIsEof(feattabin->Input->Filebuff))
	    return FMT_EOF;

	ajFilebuffReset(feattabin->Input->Filebuff);
	ajDebug("Format %d (%s) failed, file buffer reset by feattabinReadFmt\n",
		format, featinformatDef[format].Name);
	/* ajFilebuffTraceFull(feattabin->Filebuff, 10, 10);*/
    }

    ajDebug("++feattabinReadFmt failed - nothing read\n");

    return FMT_FAIL;
}




/* @funcstatic feattabinRead **************************************************
**
** Given data in a feature table input structure,
** tries to read everything needed
** using the specified format or by trial and error.
**
** @param [u] feattabin [AjPFeattabin] Feature table input object
** @param [w] ftable [AjPFeattable] Feature table object
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool feattabinRead(AjPFeattabin feattabin, AjPFeattable ftable)
{
    ajuint i;
    ajuint istat = 0;
    ajuint jstat = 0;

    AjPFilebuff buff = feattabin->Input->Filebuff;
/*    AjBool regfile = ajFalse; */
    AjBool ok;

    AjPTextAccess  textaccess  = feattabin->Input->Query->TextAccess;
    AjPFeattabAccess feattabaccess = feattabin->Input->Query->Access;

    ajFeattableClear(ftable);
    ajDebug("feattabinRead: cleared\n");

    if(feattabin->Input->Single && feattabin->Input->Count)
    {
	/*
	** One data data item at a time is read.
	** The first data data item was read by ACD
	** for the following ones we need to reset the AjPFeattabin
	**
	** Single is set by the access method
	*/

	ajDebug("feattabinRead: single access - count %d - call access"
		" routine again\n",
		feattabin->Input->Count);
	/* Calling funclist feattabinAccess() */
	if(textaccess)
        {
            if(!(*textaccess->Access)(feattabin->Input))
            {
                ajDebug("feattabinRead: (*textaccess->Access)(feattabin->Input) "
                        "*failed*\n");

                return ajFalse;
            }
        }

	if(feattabaccess)
        {
            if(!(*feattabaccess->Access)(feattabin))
            {
                ajDebug("feattabinRead: (*feattabaccess->Access)(feattabin) "
                        "*failed*\n");

                return ajFalse;
            }
        }

        buff = feattabin->Input->Filebuff;
    }

    ajDebug("feattabinRead: feattabin format %d '%S'\n",
            feattabin->Input->Format,
	    feattabin->Input->Formatstr);

    feattabin->Input->Count++;

    if(!feattabin->Input->Filebuff && !feattabin->Input->TextData)
	return ajFalse;

    ok = ajFilebuffIsBuffered(feattabin->Input->Filebuff);

    while(ok)
    {				/* skip blank lines */
        ok = ajBuffreadLine(feattabin->Input->Filebuff, &featReadLine);

        if(ok && !ajStrIsWhite(featReadLine))
        {
            ajFilebuffClear(feattabin->Input->Filebuff,1);
            break;
        }
    }

    if(!feattabin->Input->Format)
    {			   /* no format specified, try all defaults */
/*
        regfile = ajFileIsFile(ajFilebuffGetFile(feattabin->Input->Filebuff));
*/
	for(i = 1; featinformatDef[i].Name; i++)
	{
	    if(!featinformatDef[i].Try)	/* skip if Try is ajFalse */
		continue;

            if(!featinformatDef[i].Used)
            {
                if(featinformatDef[i].InitReg &&
                   !(*featinformatDef[i].InitReg)())
                {
                    ajDebug("Initialisation failed for %s\n",
                            featinformatDef[i].Name);
                    ajErr("Initialisation failed for feature format %s",
                          featinformatDef[i].Name);
                }
                featinformatDef[i].Used = ajTrue;
            }
            
	    ajDebug("feattabinRead:try format %d (%s)\n",
		    i, featinformatDef[i].Name);

	    istat = feattabinReadFmt(feattabin, ftable, i);

	    switch(istat)
	    {
	    case FMT_OK:
		ajDebug("++feattabinRead OK, set format %d\n",
                        feattabin->Input->Format);
		feattableDefine(ftable, feattabin);

		return ajTrue;
	    case FMT_BADTYPE:
		ajDebug("feattabinRead: (a1) "
                        "feattabinReadFmt stat == BADTYPE *failed*\n");

		return ajFalse;
	    case FMT_FAIL:
		ajDebug("feattabinRead: (b1) "
                        "feattabinReadFmt stat == FAIL *failed*\n");
		break;			/* we can try next format */
	    case FMT_NOMATCH:
		ajDebug("feattabinRead: (c1) "
                        "feattabinReadFmt stat==NOMATCH try again\n");
		break;
	    case FMT_EOF:
		ajDebug("feattabinRead: (d1) "
                        "feattabinReadFmt stat == EOF *failed*\n");
		return ajFalse;			/* EOF and unbuffered */
	    case FMT_EMPTY:
		ajWarn("data data '%S' has zero length, ignored",
		       ajFeattableGetQryS(ftable));
		ajDebug("feattabinRead: (e1) "
                        "feattabinReadFmt stat==EMPTY try again\n");
		break;
	    default:
		ajDebug("unknown code %d from feattabinReadFmt\n", stat);
	    }

	    ajFeattableClear(ftable);

	    if(feattabin->Input->Format)
		break;			/* we read something */

            ajFilebuffTrace(feattabin->Input->Filebuff);
	}

	if(!feattabin->Input->Format)
	{		     /* all default formats failed, give up */
	    ajDebug("feattabinRead:all default formats failed, give up\n");

	    return ajFalse;
	}

	ajDebug("++feattabinRead set format %d\n",
                feattabin->Input->Format);
    }
    else
    {					/* one format specified */
	ajDebug("feattabinRead: one format specified\n");
	ajFilebuffSetUnbuffered(feattabin->Input->Filebuff);

	ajDebug("++feattabinRead known format %d\n",
                feattabin->Input->Format);
	istat = feattabinReadFmt(feattabin, ftable, feattabin->Input->Format);

	switch(istat)
	{
	case FMT_OK:
	    feattableDefine(ftable, feattabin);

	    return ajTrue;
	case FMT_BADTYPE:
	    ajDebug("feattabinRead: (a2) "
                    "feattabinReadFmt stat == BADTYPE *failed*\n");

	    return ajFalse;

        case FMT_FAIL:
	    ajDebug("feattabinRead: (b2) "
                    "feattabinReadFmt stat == FAIL *failed*\n");

	    return ajFalse;

        case FMT_NOMATCH:
	    ajDebug("feattabinRead: (c2) "
                    "feattabinReadFmt stat == NOMATCH *try again*\n");
	    break;
	case FMT_EOF:
	    ajDebug("feattabinRead: (d2) "
                    "feattabinReadFmt stat == EOF *try again*\n");
            if(feattabin->Input->Records)
                ajErr("Error reading file '%F' with format '%s': "
                      "end-of-file before end of data "
                      "(read %u records)",
                      ajFilebuffGetFile(feattabin->Input->Filebuff),
                      featinformatDef[feattabin->Input->Format].Name,
                      feattabin->Input->Records);
	    break;		     /* simply end-of-file */
	case FMT_EMPTY:
	    ajWarn("feature table data '%S' has zero length, ignored",
		   ajFeattableGetQryS(ftable));
	    ajDebug("feattabinRead: (e2) "
                    "feattabinReadFmt stat == EMPTY *try again*\n");
	    break;
	default:
	    ajDebug("unknown code %d from feattabinReadFmt\n", stat);
	}

	ajFeattableClear(ftable); /* 1 : read, failed to match id/acc/query */
    }

    /* failed - probably entry/accession query failed. Can we try again? */

    ajDebug("feattabinRead failed - try again with format %d '%s' code %d\n",
	    feattabin->Input->Format,
            featinformatDef[feattabin->Input->Format].Name, istat);

    ajDebug("Search:%B Chunk:%B Data:%x ajFileBuffEmpty:%B\n",
	    feattabin->Input->Search, feattabin->Input->ChunkEntries,
            feattabin->Input->TextData, ajFilebuffIsEmpty(buff));

    if(ajFilebuffIsEmpty(buff) && feattabin->Input->ChunkEntries)
    {
	if(textaccess && !(*textaccess->Access)(feattabin->Input))
            return ajFalse;
	else if(feattabaccess && !(*feattabaccess->Access)(feattabin))
            return ajFalse;
        buff = feattabin->Input->Filebuff;
    }


    /* need to check end-of-file to avoid repeats */
    while(feattabin->Input->Search &&
          (feattabin->Input->TextData || !ajFilebuffIsEmpty(buff)))
    {
	jstat = feattabinReadFmt(feattabin, ftable, feattabin->Input->Format);

	switch(jstat)
	{
	case FMT_OK:
	    feattableDefine(ftable, feattabin);

	    return ajTrue;

        case FMT_BADTYPE:
	    ajDebug("feattabinRead: (a3) "
                    "feattabinReadFmt stat == BADTYPE *failed*\n");

	    return ajFalse;

        case FMT_FAIL:
	    ajDebug("feattabinRead: (b3) "
                    "feattabinReadFmt stat == FAIL *failed*\n");

	    return ajFalse;
            
	case FMT_NOMATCH:
	    ajDebug("feattabinRead: (c3) "
                    "feattabinReadFmt stat == NOMATCH *try again*\n");
	    break;
	case FMT_EOF:
	    ajDebug("feattabinRead: (d3) "
                    "feattabinReadFmt stat == EOF *failed*\n");

	    return ajFalse;			/* we already tried again */

        case FMT_EMPTY:
	    if(istat != FMT_EMPTY)
                ajWarn("assmebly data '%S' has zero length, ignored",
                       ajFeattableGetQryS(ftable));
	    ajDebug("feattabinRead: (e3) "
                    "feattabinReadFmt stat == EMPTY *try again*\n");
	    break;

        default:
	    ajDebug("unknown code %d from feattabinReadFmt\n", stat);
	}

	ajFeattableClear(ftable); /* 1 : read, failed to match id/acc/query */
    }

    if(feattabin->Input->Format)
	ajDebug("feattabinRead: *failed* to read data data %S "
                "using format %s\n",
		feattabin->Input->Qry,
                featinformatDef[feattabin->Input->Format].Name);
    else
	ajDebug("feattabinRead: *failed* to read data data %S "
                "using any format\n",
		feattabin->Input->Qry);

    return ajFalse;
}




/* @func ajFeattableNewRead ***************************************************
**
** Generic interface function for reading in features from a file
** given the file handle.
**
** @param  [u] ftin   [AjPFeattabin]  Specifies the external source (file)
**                                     of the features to be read in
** @return [AjPFeattable] Pointer to a new feature table containing
**                        the features read in
** @category new [AjPFeattable] Reads in a feature set in a specified format
**
** @release 6.2.0
** @@
******************************************************************************/

AjPFeattable ajFeattableNewRead(AjPFeattabin  ftin)
{
    AjPTextin textin = ftin->Input;
    AjPFilebuff  file;
    ajint format;
    AjBool ok = ajTrue;

    AjPFeattable features = NULL;
    AjBool result         = ajFalse;

    if(!ftin)
	return NULL;

    textin = ftin->Input;
    file = textin->Filebuff;

    if(!file)
	return NULL;

    format = ftin->Input->Format;

    if(!format)
	return NULL;

    ajDebug("ajFeattableNewRead format %d '%s' file %x type: '%S'\n",
	    format, featinformatDef[format].Name, file, ftin->Type);

    if(!featinformatDef[format].Used)
    {
	/* Calling funclist featinformatDef() */
	if(featinformatDef[format].InitReg &&
           !(*featinformatDef[format].InitReg)())
	{
	    ajDebug("Initialisation failed for %s\n",
		    featinformatDef[format].Name);
	    ajErr("Initialisation failed for feature format %s",
		  featinformatDef[format].Name);
	}

	featinformatDef[format].Used = ajTrue;
    }

    features = ajFeattableNew(ftin->Seqname);

    while(ok)
    {                               /* skip blank lines */
        ok = ajBuffreadLine(textin->Filebuff, &featReadLine);

        if(ok && !ajStrIsWhite(featReadLine))
        {
            ajFilebuffClear(textin->Filebuff,1);
            break;
        }
    }

    /* Calling funclist featinformatDef() */
    result = (*featinformatDef[format].Read)(ftin, features);

    if(result)
    {
        if(ftin->Circular)
            features->Circular = ajTrue;

	/* ajFeattableTrace(features); */
	ajDebug("ajFeattableNewRead read %d features\n",
                ajFeattableGetSize(features));

	return features;
    }
    else
    {
	ajDebug("ajFeattableNewRead failed, read %d features\n",
                ajFeattableGetSize(features));
	ajFeattableDel(&(features));
    }

    return NULL;
}




/* @func ajFeattableNewReadUfo ************************************************
**
** Parses a UFO, opens an input file, and reads a feature table
**
** @param [u] featin [AjPFeattabin] Feature input object
** @param [r] ufo [const AjPStr] UFO feature spec
** @return [AjPFeattable] Feature table created, (or NULL if failed)
** @category new [AjPFeattable] Parses a UFO, opens an input file,
**                              and reads a feature table
**
** @release 6.2.0
** @@
******************************************************************************/

AjPFeattable ajFeattableNewReadUfo(AjPFeattabin featin,
                                   const AjPStr ufo)
{
    AjPFeattable ret = NULL;
    
    AjBool fmtstat = ajFalse;	/* status returns from regex tests */
    AjBool filstat = ajFalse;	/* status returns from regex tests */
    ajint i;

    ajDebug("ajFeattableNewReadUfo ufo: '%S' filebuff: %x\n",
            ufo, featin->Input->Filebuff);

    if(!featinRegUfoFmt)
	featinRegUfoFmt = ajRegCompC("^([A-Za-z0-9][A-Za-z0-9]+):+(.*)$");
    /* \1 format */
    /* \2 remainder */
    
    if(!featinRegUfoFile)
	featinRegUfoFile = ajRegCompC("^(([A-Za-z]:)?[^:]+)$");
    
    /*ajDebug("ajFeattableNewReadUfo UFO '%S'\n", ufo);*/
    
    ajStrAssignS(&featinUfoTest, ufo);
    
    if(ajStrGetLen(ufo))
    {
	fmtstat = ajRegExec(featinRegUfoFmt, featinUfoTest);
	/*ajDebug("feat format regexp: %B\n", fmtstat);*/
    }
    
    if(fmtstat)
    {
	ajRegSubI(featinRegUfoFmt, 1, &featin->Formatstr);
	ajStrAssignEmptyC(&featin->Formatstr,
		  featinformatDef[0].Name); /* unknown */
	ajRegSubI(featinRegUfoFmt, 2, &featinUfoTest); /* trim off the format */
	/*ajDebug("found feat format %S\n", featin->Formatstr);*/

	if(!featFindInformatS(featin->Formatstr, &featin->Input->Format))
	    ajErr("unknown input feature table format '%S'\n"
		  "NO Features will be read in", featin->Formatstr);
    }
    else
    {
	/*ajDebug("no format specified in UFO");*/
    }
    
    featFormatSet(featin);
    
    filstat = ajRegExec(featinRegUfoFile, featinUfoTest);
    /*ajDebug("featinRegUfoFile: %B\n", filstat);*/

    if(filstat)
	ajRegSubI(featinRegUfoFile, 1, &featin->Filename);
    else
    {
	if(ajStrGetLen(featin->Seqname) && ajStrGetLen(featin->Formatstr))
	{
	    ajFmtPrintS(&featinUfoTest, "%S.%S",
			featin->Seqname, featin->Formatstr);
	    ajStrAssignEmptyS(&featin->Filename, featinUfoTest);
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
    ajFilebuffDel(&featin->Input->Filebuff);
    featin->Input->Filebuff = ajFilebuffNewNameS(featin->Filename);

    if(!featin->Input->Filebuff)
	return NULL;
    /*ajDebug("after opening '%S'\n", featin->Filename);*/
    
    
    /* OKAY if we have a format specified try this and this ONLY */
    if(featin->Input->Format)
	ret = ajFeattableNewRead(featin);
    /* else loop through all types and try to read features */
    else
	for(i=1;featinformatDef[i].Name;i++)
	{
	    featin->Input->Format = i;

	    ret = ajFeattableNewRead(featin);

	    if(ret)
		break;

	    /* Reset buffer to start */
	    ajFilebuffReset(featin->Input->Filebuff);

	}

    ajFilebuffDel(&featin->Input->Filebuff);
    
    return ret;
}




/* @funcstatic featFormatSet **************************************************
**
** Sets the input format for a feature table using the feature input
** object's defined format.
**
** @param [u] featin [AjPFeattabin] Feature table input.
** @return [AjBool] ajTrue on success.
**
** @release 2.0.0
** @@
******************************************************************************/

static AjBool featFormatSet(AjPFeattabin featin)
{
    if(ajStrGetLen(featin->Formatstr))
    {
	/*ajDebug("... input format value '%S'\n", featin->Formatstr);*/
	if(featFindInformatS(featin->Formatstr, &featin->Input->Format))
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




/* @funcstatic featReadChado **************************************************
**
** Copies features attached to fttabin object to fttab object.
**
** @param [u] fttabin [AjPFeattabIn] Feature table input
** @param [u] fttab [AjPFeattable] Feature table
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool featReadChado(AjPFeattabIn fttabin, AjPFeattable fttab)
{

    if(fttabin->Input->TextData && fttabin->Input->Records==0)
    {
	AjPFeattable ft = fttabin->Input->TextData;

	ajFeattableSetDefname(fttab, ft->Seqid);

	ajListFree(&fttab->Features);
	fttab->Features = ft->Features;
	ft->Features = NULL;
	fttab->Start = ft->Start;
	fttab->End = ft->End;
	fttab->Len = ft->Len;

	ajFeattableDel(&ft);

	return ajTrue;
    }

    return ajFalse;
}




/* @funcstatic featReadDasgff *************************************************
**
** Reads feature data in DAS 1.5/1.6 features format (XML).
**
** @param [u] feattabin [AjPFeattabin] Feature table input
** @param [u] ftable [AjPFeattable] Feature table
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool featReadDasgff(AjPFeattabin feattabin, AjPFeattable ftable)
{
    AjPDomDocument doc      = NULL;
    AjPDomNodeList features = NULL;
    AjPDomNodeList segments = NULL;
    AjPDomNodeList notes    = NULL;
    AjPDomNodeList links    = NULL;
    AjPDomNodeList targets  = NULL;
    AjPDomNodeList parents  = NULL;
    AjPDomNodeList parts    = NULL;
    AjPDomNodeList groups   = NULL;

    AjPDomNode dasfeature   = NULL;
    AjPDomNode segment      = NULL;

    AjPDomElement e    = NULL;
    AjPFeature feature = NULL;
    AjPStr attval      = NULL;
    AjPStr elmtxt      = NULL;
    AjPStr type        = NULL;
    AjPStr label       = NULL;

    AjBool ret = AJTRUE;

    ajint i;
    ajint j;
    ajuint start;
    ajuint end;
    AjPFilebuff inf = feattabin->Input->Filebuff;

    doc = ajDomImplementationCreateDocument(NULL,NULL,NULL);

    if (ajDomReadFilebuff(doc,inf) == -1)
    {
	ajDomDocumentDestroyNode(doc,doc);
	return AJFALSE;
    }


    /*
     * <SEGMENT> (required; one or more)
     * provides information on the reference segment queried.
     * The id attribute is required.
     * The version attribute (optional) indicates the version of the reference
     * object being annotated,
     * used for coordinate systems which are not themselves versioned.
     *
     * The optional label attribute provides a human readable label
     * for display purposes.
     *
     * TODO: here we assume one segment, spec suggests it can be more than one
     */

    segments = ajDomDocumentGetElementsByTagNameC(doc, "SEGMENT");

    if(segments==NULL || ajDomNodeListGetLen(segments) == 0)
    {
	ajDomDocumentDestroyNodeList(doc,segments,AJDOMKEEP);
	ajDomDocumentDestroyNode(doc,doc);
	ajDebug("featReadDasgff: no <SEGMENT> tag found");
	return AJFALSE;
    }

    segment = ajDomNodeListItem(segments, 0);

    attval = ajDomElementGetAttributeC(segment,"id");

    if(ajStrGetLen(attval) == 0)
    {
	ajStrDel(&attval);
	ajDomDocumentDestroyNodeList(doc,segments,AJDOMKEEP);
	ajDomDocumentDestroyNode(doc,doc);
	return AJFALSE;
    }

    ajStrAssignS(&ftable->Seqid,attval);
    ajStrDel(&attval);

    attval = ajDomElementGetAttributeC(segment,"start");
    ajStrToUint(attval, &ftable->Start);
    ftable->Start--;
    ajStrDel(&attval);

    attval = ajDomElementGetAttributeC(segment,"stop");
    ajStrToUint(attval, &ftable->End);
    ftable->End--;
    ajStrDel(&attval);

    ftable->Len = ftable->End - ftable->Start;

    features = ajDomDocumentGetElementsByTagNameC(doc, "FEATURE");

    if(features)
    {

	for (i = 0; i < ajDomNodeListGetLen(features); i++)
	{
	    dasfeature = ajDomNodeListItem(features, i);


	    /*
	     * <START>, <END> (optional; one apiece per feature)
	     *
	     * indicates the start and end of the feature in the coordinate
	     * system of the reference object given in the <SEGMENT> tag.
	     * If one element is present, the other must be also.
	     * If start and end are zero, or omitted entirely, it is assumed
	     * that the feature is an annotation of the reference object
	     * as a whole rather than a region of sequence.
	     */

	    e = ajDomElementGetFirstChildByTagNameC(doc, dasfeature, "START");
	    elmtxt = ajDomElementGetText(e);
	    ajStrToUint(elmtxt, &start);
	    ajDebug(" start = %S\n",elmtxt);

	    e = ajDomElementGetFirstChildByTagNameC(doc, dasfeature, "END");
	    elmtxt = ajDomElementGetText(e);
	    ajDebug("    end = %S\n",elmtxt);
	    ajStrToUint(elmtxt, &end);

	    feature = ajFeatNewII(ftable, start, end);


	    /*
	     * The id attribute (required) is an identifier for the feature.
	     * it must be unique to the feature across the data source.
	     *
	     * The label attribute (optional) is a suggested label to display
	     * for the feature. If not present, it is assumed the id attribute
	     * is suitable for display.
	     */

	    attval = ajDomElementGetAttributeC(dasfeature, "id");

	    if(ajStrGetLen(attval) == 0)
	    {
		ret = AJFALSE;
		break;
	    }

	    ajDebug("    id = %S\n",attval);
	    ajFeatGfftagAddCS(feature, "ID", attval);
	    ajStrDel(&attval);


	    /* <TYPE> (required; one per feature)
	     * indicates the type of the annotation.
	     * attributes are id (required), which is a unique ID
	     * for the annotation type,
	     * category (optional, recommended), which provides functional
	     * grouping to related types, and cvId (optional, recommended)
	     * which is the ID of a term from a relevant controlled vocabulary
	     *
	     * gff3: type of the feature (previously called the "method").
	     * This is constrained to be either:
	     *   (a) a term from the "lite" sequence ontology, SOFA; or
	     *   (b) a SOFA accession number.
	     * The latter alternative is distinguished using the syntax
	     * SO:000000. This field is required.
	     */

	    e = ajDomElementGetFirstChildByTagNameC(doc,dasfeature,"TYPE");

	    if(e == NULL)
	    {
		ret = AJFALSE;
		break;
	    }

	    attval = ajDomElementGetAttributeC(e, "id");

	    if(ajStrGetLen(attval) == 0)
	    {
		ajStrDel(&attval);
		ajDebug("featReadDasgff: required 'id' attribute of annotation"
			" type <TYPE> is empty\n");
		/* normally parsing should stop here
		 * but many DAS sources are missing this requirement */
		/* ret = AJFALSE; break; */
	    }

	    ajDebug("    type - id = %S\n",attval);
	    ajStrAssignS(&feature->Type, attval);
	    ajStrDel(&attval);
	    attval = ajDomElementGetAttributeC(e, "cvId");

	    if(ajStrGetLen(attval)>0)
		ajFeatGfftagAddCS(feature,"Ontology_term",attval);

	    ajStrDel(&attval);
	    attval = ajDomElementGetAttributeC(e, "category");

	    if(ajStrGetLen(attval)>0)
		ajFeatGfftagAddCS(feature,"category",attval);

	    ajStrDel(&attval);


	    /*
	     * <METHOD>   -  "source" on gff
	     *
	     * method used to identify the feature.
	     * The id attribute is required.
	     * The cvId (optional, recommended) attribute is an ontology term
	     * ID from the Evidence Codes Ontology, and as such is a generic
	     * (potentially less specific) representation of the method.
	     *
	     */

	    e = ajDomElementGetFirstChildByTagNameC(doc,dasfeature,"METHOD");

	    if(e == NULL)
	    {
		ret = AJFALSE;
		break;
	    }

	    attval = ajDomElementGetAttributeC(e, "id");
	    ajDebug("    method id = %S\n",attval);
	    ajStrAssignRef(&feature->Source, attval);
	    ajStrDel(&attval);
	    attval = ajDomElementGetAttributeC(e, "cvId");

	    if(ajStrGetLen(attval) > 0)
		ajFeatSetSource(feature, attval);

	    ajStrDel(&attval);


	    /*
	     * <SCORE> (optional; one per feature)
	     *
	     * integer or floating point number indicating the "score" of
	     * the method used to find the current feature.
	     * If this field is inapplicable, the contents of the tag can be
	     * replaced with a - symbol.
	     * This is the assumed value if the tag is omitted entirely.
	     */

	    e = ajDomElementGetFirstChildByTagNameC(doc,dasfeature,"SCORE");

	    if(e!=NULL)
	    {
		elmtxt = ajDomElementGetText(e);
		ajDebug("    score = %S\n",elmtxt);
		ajStrToFloat(elmtxt, &feature->Score);
	    }


	    /*
	     * <ORIENTATION> (optional; one per feature)
	     *
	     * orientation of the feature relative to the direction of
	     * transcription. It may be 0 for features that are unrelated to
	     * transcription, +, for features that are on the sense strand,
	     * and -, for features on the antisense strand.
	     * If this tag is omitted, a value of 0 is assumed.
	     */

	    e = ajDomElementGetFirstChildByTagNameC(doc,dasfeature,
	                                            "ORIENTATION");
	    if(e != NULL)
	    {
		elmtxt = ajDomElementGetText(e);
		ajDebug("    strand = %S\n",elmtxt);
		feature->Strand = ajStrGetCharFirst(elmtxt);
	    }


	    /*
	     * <PHASE> (optional; one per feature)
	     *
	     * position of the feature relative to open reading frame, if any.
	     * It may be one of the integers 0, 1 or 2, corresponding to
	     * each of the three reading frames,
	     * or - if the feature is unrelated to a reading frame.
	     * If this tag is omitted, a value of - is assumed.
	     */

	    e = ajDomElementGetFirstChildByTagNameC(doc, dasfeature, "PHASE");

	    if(e != NULL)
	    {
		elmtxt = ajDomElementGetText(e);
		ajDebug("    phase = %S\n",elmtxt);

		/* TODO: new integer value to represent features
		 * unrelated to a reading frame */
		if ( ajStrGetCharFirst(elmtxt) == '-' )
		    feature->Frame = 0;

		ajStrToInt(elmtxt, &feature->Frame);
	    }


	    /*
	     * <NOTE> (optional; zero or more per feature)
	     *
	     * A human-readable note in plain text format only
	     *
	     */

	    notes = ajDomElementGetElementsByTagNameC(dasfeature,"NOTE");

	    if (notes!=NULL)
	    {
		for(j=0; j<ajDomNodeListGetLen(notes); j++)
		{
		    e = ajDomNodeListItem(notes,j);
		    elmtxt = ajDomElementGetText(e);

		    /* the following check covers empty nodes: <NOTE/> */
		    if(elmtxt != NULL)
			ajFeatGfftagAddCS(feature, "Note", elmtxt);
		}

		ajDomDocumentDestroyNodeList(doc,notes,AJDOMKEEP);
	    }


	    /*
	     * <LINK> (optional; zero or more per feature)
	     *
	     * link to a web page somewhere that provides more information
	     * about this feature.
	     * element text is an optional human readable label
	     * for display purposes.
	     *
	     */

	    links = ajDomElementGetElementsByTagNameC(dasfeature,"LINK");

	    if (links!=NULL)
	    {
		for(j=0; j<ajDomNodeListGetLen(links); j++)
		{
		    e = ajDomNodeListItem(links,j);
		    attval = ajDomElementGetAttributeC(e, "href");
		    ajDebug("    link - href = %S\n",attval);
		    ajFeatGfftagAddCS(feature, "dasgff_link", attval);
		    ajStrDel(&attval);
		}

		ajDomDocumentDestroyNodeList(doc,links,AJDOMKEEP);
	    }


	    /*
	     * <TARGET> (optional; zero or more per feature)
	     *
	     * target sequence in a sequence similarity match.
	     * The id attribute provides
	     * the reference ID for the target sequence,
	     * and the start and stop attributes indicate the segment
	     * that matched across the target sequence.
	     * All three attributes are required.
	     * content of the tag (optional) is a human readable label.
	     *
	     * gff mapping: attributes column - Target tag
	     */

	    targets = ajDomElementGetElementsByTagNameC(dasfeature,"TARGET");

	    if(targets!=NULL)
	    {
		for(j=0;j<ajDomNodeListGetLen(targets);j++)
		{
		    e = ajDomNodeListItem(targets,j);

		    attval = ajDomElementGetAttributeC(e, "start");
		    ajStrToUint(attval, &start);
		    ajStrDel(&attval);
		    attval = ajDomElementGetAttributeC(e, "stop");
		    ajStrToUint(attval, &end);
		    ajStrDel(&attval);

		    attval = ajDomElementGetAttributeC(e, "id");
		    ajDebug("    target id = %S\n",attval);

		    if(end)
			ajFmtPrintAppS(&attval, " %d %d", start, end);

		    ajFeatGfftagAddCS(feature,"Target",attval);

		    ajStrDel(&attval);
		}

		ajDomDocumentDestroyNodeList(doc,targets,AJDOMKEEP);
	    }


	    /*
	     * <PARENT>, <PART> (optional; zero or more per feature)
	     *
	     * A replacement for the <GROUP> tag,
	     * these tags identify other features that are parents or children
	     * of this feature within a hierarchy.
	     * Each has a single required attribute, id,
	     * which refers to a separate <FEATURE> tag.
	     * This mechanism means a parent or child feature need
	     * only be defined once and may be referred to multiple times.
	     * This is preferable to the use of <GROUP> tags,
	     * where a parent must be defined separately for every child.
	     * It also allows more than two levels of hierarchy to be defined
	     * (e.g. a gene has parts - transcripts, and transcripts have parts
	     *  - exons).
	     *  In addition, parent features may have start/end positions.
	     *
	     */

	    groups = ajDomElementGetElementsByTagNameC(dasfeature,"GROUP");

	    if (groups!=NULL)
	    {
		for(j=0; j<ajDomNodeListGetLen(groups); j++)
		{
		    e = ajDomNodeListItem(groups,j);
		    attval = ajDomElementGetAttributeC(e, "id");

		    if(ajStrGetLen(attval) == 0)
		    {
			ret = AJFALSE;
			ajStrDel(&attval);
			break;
		    }

		    type = ajDomElementGetAttributeC(e, "type");
		    
		    if(ajStrGetLen(type))
			ajFmtPrintAppS(&attval," type=%S", type);
		    
		    label = ajDomElementGetAttributeC(e, "label");
		    
		    if(ajStrGetLen(label))
			ajFmtPrintAppS(&attval," label=%S", label);

		    ajDebug("    group - id = %S\n",attval);
		    ajFeatGfftagAddCS(feature,"group",attval);

		    ajStrDel(&type);
		    ajStrDel(&label);
		    ajStrDel(&attval);
		}

		ajDomDocumentDestroyNodeList(doc,groups,AJDOMKEEP);
	    }


	    parents = ajDomElementGetElementsByTagNameC(dasfeature,"PARENT");

	    if (parents!=NULL)
	    {
		for(j=0; j<ajDomNodeListGetLen(parents); j++)
		{
		    e = ajDomNodeListItem(parents,j);
		    attval = ajDomElementGetAttributeC(e, "id");

		    if(ajStrGetLen(attval) == 0)
		    {
			ret = AJFALSE;
			break;
		    }

		    ajDebug("    parent - id = %S\n",attval);
		    ajFeatGfftagAddCS(feature,"Parent",attval);
		    ajStrDel(&attval);
		}

		ajDomDocumentDestroyNodeList(doc,parents,AJDOMKEEP);
	    }

	    parts = ajDomElementGetElementsByTagNameC(dasfeature,"PART");

	    if (parts!=NULL)
	    {
		for(j=0; j<ajDomNodeListGetLen(parts); j++)
		{
		    e = ajDomNodeListItem(parts,j);
		    attval = ajDomElementGetAttributeC(e, "id");

		    if(ajStrGetLen(attval) == 0)
		    {
			ret = AJFALSE;
			break;
		    }

		    ajDebug("    part - id = %S\n",attval);
		    ajFeatTagAddCS(feature,"part",attval);
		    ajStrDel(&attval);
		}

		ajDomDocumentDestroyNodeList(doc,parts,AJDOMKEEP);
	    }

	}

	ajDomDocumentDestroyNodeList(doc,features,AJDOMKEEP);

    }

    ajDomDocumentDestroyNodeList(doc,segments,AJDOMKEEP);
    ajDomDocumentDestroyNode(doc,doc);

    return ret;
}




/* @funcstatic featReadEmbl ***************************************************
**
** Reads feature data in EMBL format. Also handles GenBank, RefSeq, RefSeqP
**
** @param [u] feattabin [AjPFeattabin] Feature table input
** @param [u] ftable [AjPFeattable] Feature table
** @return [AjBool] ajTrue on success
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool featReadEmbl(AjPFeattabin feattabin, AjPFeattable ftable)
{
    AjBool found    = ajFalse;
    AjPStr savefeat = NULL;
    AjPStr saveline = NULL;
    AjPStr saveloc  = NULL;

    AjPStr token  = NULL;
    AjPStrTok handle = NULL;

    AjBool isGenbank = ajFalse;
    ajlong fpos     = 0;

    AjPFilebuff file = feattabin->Input->Filebuff;

    if(!featReadLine)
	featReadLine = ajStrNewRes(100);

    ajFeattableSetNuc(ftable);

    while(ajBuffreadLinePosStore(file, &featReadLine, &fpos,
                                 feattabin->Input->Text,
                                 &ftable->TextPtr))
    {
	if(ajStrPrefixC(featReadLine, "//"))
        {
            break;
        }
	else if(ajStrPrefixC(featReadLine, "ID   "))
        {
            isGenbank = ajFalse;
            ajStrTokenAssignC(&handle, featReadLine, " ;\t\n\r");
            ajStrTokenNextParse(&handle, &token);	/* 'ID' */
            ajStrTokenNextParse(&handle, &ftable->Seqid); /* entry name */
            ajStrTokenDel(&handle);
            ajStrDel(&token);
        }
	else if(ajStrPrefixC(featReadLine, "LOCUS "))
        {
            isGenbank = ajTrue;
        }	/* if it's an EMBL feature do stuff */
	else if(ajStrPrefixC(featReadLine, "FEATURES  "))
        {
            isGenbank = ajTrue;
        }
	else if(ajStrPrefixC(featReadLine, "FT   "))
	{
	    ajStrTrimWhiteEnd(&featReadLine); /* remove newline */

	    if(featEmblFromLine(ftable, featReadLine,
				&savefeat, &saveloc, &saveline))
		found = ajTrue;
	}

	/* if it's a GenBank feature do stuff */
	else if(isGenbank && ajStrPrefixC(featReadLine, "     "))
	{
	    ajStrTrimWhiteEnd(&featReadLine); /* remove newline */

	    if(featEmblFromLine(ftable, featReadLine,
				&savefeat, &saveloc, &saveline))
		found = ajTrue;
	}
        else if(isGenbank) 
        {
            isGenbank = ajFalse;
        }
        
    }

    if(featEmblFromLine(ftable, NULL, &savefeat, &saveloc, &saveline))
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
** @param [u] feattabin [AjPFeattabin] Feature table input
** @param [u] ftable [AjPFeattable] Feature table
** @return [AjBool] ajTrue on success
**
** @release 6.2.0
** @@
******************************************************************************/

static AjBool featReadGenpept(AjPFeattabin feattabin, AjPFeattable ftable)
{
    AjBool found           = ajFalse;
    AjPStr savefeat = NULL;
    AjPStr saveline = NULL;
    AjPStr saveloc  = NULL;

    AjPFilebuff file = feattabin->Input->Filebuff;

    if(!featReadLine)
	featReadLine = ajStrNewRes(100);

    ajFeattableSetProt(ftable);

    while(ajBuffreadLine(file, &featReadLine))
    {
	if(ajStrPrefixC(featReadLine, "     "))
	{
	    ajStrTrimWhiteEnd(&featReadLine); /* remove newline */

	    if(featEmblFromLine(ftable, featReadLine,
				&savefeat, &saveloc, &saveline))
		found = ajTrue;
	}
    }

    if(featEmblFromLine(ftable, NULL, &savefeat, &saveloc, &saveline))
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
** @param [u] feattabin [AjPFeattabin] Feature table input
** @param [u] ftable [AjPFeattable] Feature table
** @return [AjBool] ajTrue on success
**
** @release 6.2.0
** @@
******************************************************************************/

static AjBool featReadRefseq(AjPFeattabin feattabin, AjPFeattable ftable)
{
    AjBool found           = ajFalse;
    AjPStr savefeat = NULL;
    AjPStr saveline = NULL;
    AjPStr saveloc  = NULL;

    AjPFilebuff file = feattabin->Input->Filebuff;

    if(!featReadLine)
	featReadLine = ajStrNewRes(100);

    ajFeattableSetNuc(ftable);

    while(ajBuffreadLine(file, &featReadLine))
    {
	if(ajStrPrefixC(featReadLine, "     "))
	{
	    ajStrTrimWhiteEnd(&featReadLine); /* remove newline */

	    if(featEmblFromLine(ftable, featReadLine,
				&savefeat, &saveloc, &saveline))
		found = ajTrue;
	}
    }

    if(featEmblFromLine(ftable, NULL, &savefeat, &saveloc, &saveline))
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
** @param [u] feattabin [AjPFeattabin] Feature table input
** @param [u] ftable [AjPFeattable] Feature table
** @return [AjBool] ajTrue on success
**
** @release 6.2.0
** @@
******************************************************************************/

static AjBool featReadRefseqp(AjPFeattabin feattabin, AjPFeattable ftable)
{
    AjBool found  = ajFalse;
    AjPStr savefeat = NULL;
    AjPStr saveline = NULL;
    AjPStr saveloc  = NULL;
    AjPFilebuff file = feattabin->Input->Filebuff;

    if(!featReadLine)
	featReadLine = ajStrNewRes(100);

    ajFeattableSetProt(ftable);

    while(ajBuffreadLine(file, &featReadLine))
    {
	if(ajStrPrefixC(featReadLine, "     "))
	{
	    ajStrTrimWhiteEnd(&featReadLine); /* remove newline */

	    if(featRefseqpFromLine(ftable, featReadLine,
                                   &savefeat, &saveloc, &saveline))
		found = ajTrue;
	}
    }

    if(featRefseqpFromLine(ftable, NULL, &savefeat, &saveloc, &saveline))
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
** @param [u] feattabin [AjPFeattabin] Feature table input
** @param [u] ftable [AjPFeattable] Feature table
** @return [AjBool] ajTrue on success
**
** @release 2.0.0
** @@
******************************************************************************/

static AjBool featReadPir(AjPFeattabin feattabin, AjPFeattable ftable)
{
    AjBool found = ajFalse;

    AjPFilebuff file = feattabin->Input->Filebuff;

    /*ajDebug("featReadPir..........\n");*/

    if(!featReadLine)
	featReadLine = ajStrNewRes(100);

    while(ajBuffreadLine(file, &featReadLine))
    {
	ajStrTrimWhite(&featReadLine);

	/* ajDebug("++ line '%S'\n", line); */

	if(ajStrPrefixC(featReadLine, "F;"))
	{
	    if(featPirFromLine(ftable, featReadLine))
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
**
** @release 2.0.0
** @@
******************************************************************************/

static AjPFeature featPirFromLine(AjPFeattable thys,
				  const AjPStr origline)
{
    AjPStr temp     = NULL;
    static AjPFeature gf   = NULL;    /* made static so that it's easy
					  to add second line of description */
/*    AjPFeature gfpos = NULL; */
    AjPStr locstr   = NULL;
    AjPStr typstr   = NULL;
    AjPStr notestr  = NULL;
    AjPStr comstr   = NULL;
    AjPStr exonstr  = NULL;
    AjPStr posstr   = NULL;
    ajint i = 0;
    AjBool mother = ajTrue;
    ajuint Start = 0;
    ajuint End   = 0;
    ajuint Flags = 0;
    const AjPStr pirtype = NULL;
   
    /*ajDebug("featPirFromLine..........\n'%S'\n", origline);*/
    
    if(!featinTagNote)
	ajStrAssignC(&featinTagNote, "note");

    if(!featinTagComm)
	ajStrAssignC(&featinTagComm, "comment");

    if(!featinSourcePir)
	featinSourcePir = ajStrNewC("PIR");
    
    if(!ajRegExec(PirRegexAll, origline))
	return NULL;
    
    ajRegSubI(PirRegexAll, 1, &locstr);
    ajRegSubI(PirRegexAll, 2, &typstr);
    ajRegSubI(PirRegexAll, 3, &notestr);
    
    /* remove spaces in feature type so we can look it up */
    
    ajStrExchangeCC(&typstr, " ", "_");
    
    pirtype = ajFeattypeGetInternalPir(typstr);
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
		if(!ajStrToUint(posstr, &Start))
		    Start = 1;

	    ajRegPost(PirRegexPos, &temp);
	    ajStrAssignS(&exonstr, temp);
	}

	if(!ajStrToUint(posstr, &End))
	    End = 1;

        if(mother)
            gf = ajFeatNewProtFlags(thys,
                                    featinSourcePir,	/* source sequence */
                                    pirtype,
                                    Start, End,
                                    0.0,
                                    Flags);
        if(!mother || ajStrGetLen(locstr))
        {
            /* gfpos = */ ajFeatNewProtFlagsSub(thys,
                                       gf,
                                       featinSourcePir,	/* source sequence */
                                       pirtype,
                                       Start, End,
                                       0.0,
                                       Flags);
            if(!gf->Start || gf->Start > Start)
                gf->Start = Start;
            if(!gf->End || gf->End < End)
                gf->End = End;
        }
        
	/* for the first feature, process the rest of the tags */

	if(mother)
	{
	    if(ajStrGetLen(notestr))
		ajFeatTagAddSS(gf, featinTagNote, notestr);

	    ajRegPost(PirRegexAll, &temp);

	    while(ajRegExec(PirRegexCom, temp))
	    {
		ajRegSubI(PirRegexCom, 1, &comstr);
		ajStrRemoveWhiteExcess(&comstr);
		ajFeatTagAddSS(gf, featinTagComm, comstr);
		ajRegPost(PirRegexCom, &temp);
	    }
	}

	mother = ajFalse;
    }
    
    if(mother)
	ajFeatWarn("featPirFromLine: Did not understand location '%S'", locstr);

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
** @param [u] feattabin [AjPFeattabin] Feature table input
** @param [u] ftable [AjPFeattable] Feature table
** @return [AjBool] ajTrue on success
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool featReadSwiss(AjPFeattabin feattabin, AjPFeattable ftable)
{
    AjBool found           = ajFalse;
    AjPStr savefeat = NULL;
    AjPStr saveline = NULL;
    AjPStr savefrom = NULL;
    AjPStr saveto   = NULL;

    AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    AjPFilebuff file = feattabin->Input->Filebuff;
    AjPStr defname = NULL;
    AjBool setname = ajFalse;

    /*ajDebug("featReadSwiss..........\n");*/

    if(!featReadLine)
	featReadLine = ajStrNewRes(100);

    while(ajBuffreadLine(file, &featReadLine))
    {
	ajStrTrimWhite(&featReadLine);

	/* ajDebug("++ line '%S'\n", line); */

	if(ajStrPrefixC(featReadLine, "//"))
        {
            break;
        }
        else if(ajStrPrefixC(featReadLine, "ID   "))
        {
            ajStrTokenAssignC(&handle, featReadLine, " \n\r");
            ajStrTokenNextParse(&handle, &token);	/* 'ID' */
            ajStrTokenNextParse(&handle, &defname);	/* entry name */
            if(!setname)
            {
                ajFeattableSetDefname(ftable, defname);
                setname = ajTrue;
            }
        }
        else if(ajStrPrefixC(featReadLine, "AC   "))
        {
            ajStrTokenAssignC(&handle, featReadLine, " \n\r,;");
            ajStrTokenNextParse(&handle, &token);	/* 'AC' */
            ajStrTokenNextParse(&handle, &defname);	/* accession */
            if(!setname)
            {
                ajFeattableSetDefname(ftable, defname);
                setname = ajTrue;
            }
        }
	else if(ajStrPrefixC(featReadLine, "FT   "))
        {
	    if(featSwissFromLine(ftable, featReadLine,
				 &savefeat, &savefrom, &saveto, &saveline))
		found = ajTrue;
        }
    }

    if(featSwissFromLine(ftable, NULL,
                         &savefeat, &savefrom, &saveto, &saveline))
	found = ajTrue;

    if(found)
    {
        ajFeattableSetProt(ftable);
    }

    /*ajDebug("featReadSwiss returns %B\n", found);*/

    ajStrDel(&savefeat);
    ajStrDel(&savefrom);
    ajStrDel(&saveto);
    ajStrDel(&saveline);
    ajStrDel(&token);
    ajStrDel(&defname);

    ajStrTokenDel(&handle);

    return found;
}




/* @funcstatic featGff2ProcessTagval ******************************************
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
**
** @release 6.4.0
** @@
******************************************************************************/

static void featGff2ProcessTagval(AjPFeature gf, AjPFeattable table,
                                  const AjPStr groupfield, float version)
{
    AjPStr  TvString  = NULL;
    AjPStr  tmptag      = NULL;
    AjBool  grpset      = ajFalse;
    AjBool escapedquote = ajFalse;
  
    /*ajDebug("featGff2ProcessTagval version %3.1f '%S'\n",
      version, groupfield); */
    
    /* Validate arguments */
    if(!ajStrGetLen(groupfield))	/* no tags, must be new */
	return;
    
    if(E_FPEQ(version,1.0F,U_FEPS))
    {
	featGroupSet(gf, table, groupfield);
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
    if(ajStrFindC(TvString, "\\\"") >= 0)
    {
        escapedquote = ajTrue;
        ajStrExchangeCC(&TvString, "\\\"", "\001");
    }

    while(ajStrGetLen(TvString))
    {
	if(ajRegExec(GffRegexTvTagval, TvString))
	{
	    ajRegSubI(GffRegexTvTagval, 1, &tmptag);
	    ajRegSubI(GffRegexTvTagval, 2, &featinValTmp);
	    ajStrTrimWhite(&featinValTmp);
	    /*ajDebug("GffTv '%S' '%S'\n", tmptag, featinValTmp);*/
	    ajRegPost(GffRegexTvTagval, &TvString);

	    if(ajStrMatchC(tmptag, "Sequence"))
	    {
		featGroupSet(gf, table, featinValTmp);
		grpset = ajTrue;
	    }
	    else if(ajStrMatchC(tmptag, "ID"))
	    {
		featGroupSet(gf, table, featinValTmp);
		grpset = ajTrue;
	    }
	    else if(ajStrMatchC(tmptag, "FeatFlags"))
		featFlagSet(gf, featinValTmp);
	    else if(ajStrMatchC(tmptag, "featflags"))
		featFlagSet(gf, featinValTmp);
	    else
	    {
		/*ajDebug("Before QuoteStrip: '%S'\n", featinValTmp);*/
		ajStrQuoteStrip(&featinValTmp);
		/*ajDebug(" After QuoteStrip: '%S'\n", featinValTmp);*/
		ajFeatTagAddSS(gf,tmptag,featinValTmp);
	    }
	}
	else
	{
            if(escapedquote)
                ajStrExchangeCC(&featinValTmp, "\001", "\"");
	    /*ajDebug("Choked on '%S'\n", TvString);*/
	    ajFeatWarn("GFF tag parsing failed on '%S' in\n'%S'",
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
** @param [w] parent [AjBool*] True if any Parent tag is found
** @return [AjPStr] Identifier if any
**
** @release 6.0.0
** @@
******************************************************************************/

static AjPStr featGff3ProcessTagval(AjPFeature gf, AjPFeattable table,
                                    const AjPStr groupfield,
                                    AjBool *parent)
{
    AjPStr  TvString  = NULL;
    AjPStr  tmptag      = NULL;
    AjBool  grpset      = ajFalse;
    char cp;
    AjPStr idstr = NULL;

    /*ajDebug("featGff3ProcessTagval '%S'\n",
      groupfield); */

    *parent = ajFalse;

    /* Validate arguments */
    if(!ajStrGetLen(groupfield))	/* no tags, must be new */
	return idstr;
    
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
	    ajRegSubI(Gff3RegexTvTagval, 2, &featinValTmp);
            ajStrFmtPercentDecode(&featinValTmp);
	    ajStrTrimWhite(&featinValTmp);
	    /*ajDebug("Gff3Tv '%S' '%S'\n", tmptag, featinValTmp);*/
	    ajRegPost(Gff3RegexTvTagval, &TvString);

            cp = ajStrGetCharFirst(tmptag);
            if(isupper((int)cp))
            {
                if(ajStrMatchC(tmptag, "ID"))
                {
                    ajStrAssignS(&idstr, featinValTmp);
                }
                else if(ajStrMatchC(tmptag, "Parent"))
                {
                    *parent = ajTrue;
                    featGff3GroupSet(gf, table, featinValTmp);
                    grpset = ajTrue;
                }
                else if(ajStrMatchC(tmptag, "Is_circular"))
                {
                    ajFeattableSetCircular(table);
                }

                ajFeatGfftagAddSS(gf,tmptag,featinValTmp);
            }
            
	    else{
                if(ajStrMatchC(tmptag, "featflags"))
                {
                    featGff3FlagSet(gf, featinValTmp);
                }
                else
                {
                    ajFeatTagAddSS(gf,tmptag,featinValTmp);
                }
            }
	}
	else
	{
	    /*ajDebug("GFF3 choked on '%S'\n", TvString);*/
	    ajFeatWarn("GFF3 tag parsing failed on '%S' in\n'%S'",
		   TvString, groupfield);
	    break;
	}
    }
    
    if(!grpset)
    {
	featGroupSet(gf, table, NULL);
	grpset = ajTrue;
    }

    ajStrDel(&TvString);
    ajStrDel(&tmptag);
    
    return idstr;
}




/* @funcstatic featGff3oldProcessTagval ***************************************
**
** Parses and adds a tag-value set to the specified AjPFeature fror GFF3 before
** EMBOSS 6.4.0
**
** @param [u] gf [AjPFeature] Feature
** @param [u] table [AjPFeattable] Feature table
** @param [r] groupfield [const AjPStr] Group field identifier
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void featGff3oldProcessTagval(AjPFeature gf, AjPFeattable table,
                                     const AjPStr groupfield)
{
    AjPStr  TvString  = NULL;
    AjPStr  tmptag      = NULL;
    AjBool  grpset      = ajFalse;
    AjBool escapedquote = ajFalse;
    
    /*ajDebug("featGff3oldProcessTagval '%S'\n",
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
    if(ajStrFindC(TvString, "\\\"") >= 0)
    {
        escapedquote = ajTrue;
        ajStrExchangeCC(&TvString, "\\\"", "\001");
    }

    while(ajStrGetLen(TvString))
    {
	if(ajRegExec(Gff3oldRegexTvTagval, TvString))
	{
	    ajRegSubI(Gff3oldRegexTvTagval, 1, &tmptag);
	    ajRegSubI(Gff3oldRegexTvTagval, 2, &featinValTmp);
	    ajStrTrimWhite(&featinValTmp);
	    /*ajDebug("Gff3Tv '%S' '%S'\n", tmptag, featinValTmp);*/
	    ajRegPost(Gff3oldRegexTvTagval, &TvString);

	    if(ajStrMatchC(tmptag, "Sequence"))
	    {
		featGroupSet(gf, table, featinValTmp);
		grpset = ajTrue;
	    }
	    else if(ajStrMatchC(tmptag, "ID"))
	    {
		featGroupSet(gf, table, featinValTmp);
		grpset = ajTrue;
	    }
	    else if(ajStrMatchC(tmptag, "FeatFlags"))
		featFlagSet(gf, featinValTmp);
	    else if(ajStrMatchC(tmptag, "featflags"))
		featFlagSet(gf, featinValTmp);
	    else
	    {
                if(escapedquote)
                    ajStrExchangeCC(&featinValTmp, "\001", "\"");
		/*ajDebug("GFF3 before QuoteStrip: '%S'\n", featinValTmp);*/
		ajStrQuoteStrip(&featinValTmp);
		/*ajDebug(" GFF3 after QuoteStrip: '%S'\n", featinValTmp);*/
		ajFeatTagAddSS(gf,tmptag,featinValTmp);
	    }
	}
	else
	{
	    /*ajDebug("GFF3 choked on '%S'\n", TvString);*/
	    ajFeatWarn("GFF3 tag parsing failed on '%S' in\n'%S'",
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
**
** @release 1.0.0
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
    
    if(!featinSourceSwiss)
 	featinSourceSwiss = ajStrNewC("SWISSPROT");
    
    if(origline)
	newft = ajRegExec(SwRegexNew, origline);

    
    if(newft || !origline)		/* process the last feature */
    {
	/* ajDebug("++ feat+from+to '%S' '%S' '%S'\n+ saveline '%S'\n",
		 *savefeat, *savefrom, *saveto, *saveline); */

	if(ajStrGetLen(*savefrom))      /* finish the current feature */
	    gf = featSwissProcess(thys, *savefeat, *savefrom, *saveto,
				  featinSourceSwiss, *saveline);
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
	    ajFeatWarn("%S: Bad SwissProt feature line:\n%S",
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
**
** @release 2.0.0
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
    
    if(!featinTagNote)
	ajStrAssignC(&featinTagNote, "note");

    if(!featinTagComm)
	ajStrAssignC(&featinTagComm, "comment");

    if(!featinTagFtid)
	ajStrAssignC(&featinTagFtid, "ftid");
    
    switch(ajStrGetCharFirst(fromstr))
    {
        case '?':
            flags |= AJFEATFLAG_START_UNSURE;
            ajStrAssignS(&featinTmpStr, fromstr);
            ajStrCutStart(&featinTmpStr, 1);

            if(!ajStrToInt(featinTmpStr, &Start))
                Start = 0;

            break;
        case '<':
        case '>':				/* just to be sure */
            flags |= AJFEATFLAG_START_BEFORE_SEQ;
            ajStrAssignS(&featinTmpStr, fromstr);
            ajStrCutStart(&featinTmpStr, 1);

            if(!ajStrToInt(featinTmpStr, &Start))
                Start = 0;

            break;

        default:
            if(!ajStrToInt(fromstr, &Start))
                Start = 0;
    }
    
    switch(ajStrGetCharFirst(tostr))
    {
        case '?':
            flags |= AJFEATFLAG_END_UNSURE;
            ajStrAssignS(&featinTmpStr, tostr);
            ajStrCutStart(&featinTmpStr, 1);

            if(!ajStrToInt(featinTmpStr, &End))
                End = 0;

            break;
        case '<':				/* just to be sure */
        case '>':
            flags |= AJFEATFLAG_END_AFTER_SEQ;
            ajStrAssignS(&featinTmpStr, tostr);
            ajStrCutStart(&featinTmpStr, 1);

            if(!ajStrToInt(featinTmpStr, &End))
                End = 0;

            break;
        default:
            if(!ajStrToInt(tostr, &End))
                End = 0;
    }

    ajStrDelStatic(&featinTmpStr);

    ret = ajFeatNewProtFlags(thys,
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
	    ajFeatTagAddSS(ret, featinTagNote, note);

	if(ajStrGetLen(comment))
	    ajFeatTagAddSS(ret, featinTagComm, comment);
    }
    else 
    {
	/*ajDebug("Simple swiss note: '%S'\n", tagstr);*/
	if(ajStrGetLen(tagstr))
	    ajFeatTagAddSS(ret, featinTagNote, tagstr);
    }

    if(ajStrGetLen(ftid))
        ajFeatTagAddSS(ret, featinTagFtid, ftid);

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
**
** @release 1.0.0
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

    if(!featinSourceEmbl)
	featinSourceEmbl = ajStrNewC("EMBL");
    
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

    /* ajDebug("+ newft: %B doft: %B\n+ line '%S'\n",
       newft, doft, featProcessLine); */
    
    if(doft) 		/* process the last feature */
    {
        /* ajDebug("++ saveloc '%S'\n+ saveline '%S'\n",
         *saveloc, *saveline); */

	gf = featEmblProcess(thys, *savefeat, featinSourceEmbl,
			     saveloc, saveline);

	ajStrDelStatic(saveloc);
	ajStrDelStatic(saveline);
    }

    if(!origline)		/* we are only cleaning up */
    {
	if(doft)
            return gf;
        else
            return NULL;
    }

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
**
** @release 2.0.0
** @@
******************************************************************************/

static AjPFeature featEmblProcess(AjPFeattable thys, const AjPStr feature,
				  const AjPStr source,
				  AjPStr* loc, AjPStr* tags)
{    
    AjPFeature ret  = NULL;
    AjPFeature gf   = NULL;
/*    AjPFeature gfpos = NULL; */
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
    AjPStr tagname  = NULL;
    AjPStr tagrest  = NULL;
    AjBool Simple   = ajFalse;	/* Simple - single position (see also label) */
    AjBool BegBound = ajFalse;
    AjBool EndBound = ajFalse;
    ajuint BegNum = 0;
    ajuint EndNum = 0;
    ajuint Beg2   = 0;
    ajuint End2   = 0;
    ajuint tmpbeg = 0;
    ajuint tmpend = 0;
    ajuint Flags    = 0;
    ajuint tmpflags = 0;
    AjBool Between = ajFalse;
    AjBool Join    = ajFalse;
    AjBool Order   = ajFalse;
    char Strand     = '+';
    AjBool Parent   = ajTrue;
    ajint Frame     = 0;
    float Score     = 0.0;
    AjBool HasOper  = ajFalse;
    AjBool RemoteId = ajFalse;
    AjBool IsLabel  = ajFalse;	/* uses obsolete label  */
    ajint Exon      = 0;
    ajlong ipos;
    ajint itags = 0;
    const AjPStr tmpft = NULL;
    const AjPStr tmpfeatid = NULL;
    ajuint noparentflags = 0;

    noparentflags = ~(AJFEATFLAG_POINT | AJFEATFLAG_BETWEEN_SEQ |
                      AJFEATFLAG_REMOTEID | AJFEATFLAG_LABEL |
                      AJFEATFLAG_START_TWO | AJFEATFLAG_END_TWO |
                      AJFEATFLAG_START_BEFORE_SEQ | AJFEATFLAG_END_AFTER_SEQ);

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
	      featEmblOperOut(opval, &opnam, &featinTmpStr))
	{
	    if(!ajStrHasParentheses(featinTmpStr))
		break;

	    /* ajDebug("OperOut %S( '%S' )\n", opnam, featinTmpStr); */
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
		ajFeatWarn("%S: unrecognised operator '%S()' in '%S'",
		       thys->Seqid, opnam, opval);

	    ajStrAssignS(&opval, featinTmpStr);
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

	if(featEmblOperIn(opval, &opnam, &locstr, &featinTmpStr))
	{
	    /* ajDebug("OperIn %S( '%S' )\n", opnam, locstr); */
	    if(ajStrMatchCaseC(opnam, "complement"))
		LocFwd = !LocFwd;

	    ajStrAssignS(&opval, featinTmpStr);
	    /* ajDebug("rest: '%S'\n", opval); */
	    HasOper = ajTrue;
	}
	else
	{
	    ajStrAssignS(&locstr, opval);
	    /* ajDebug("OperIn simple '%S'\n", locstr); */
	}

	if(featEmblOperNone(locstr,
			    &featId, &featinTmpStr, &rest))  /* one exon */
	{
	    /* ajDebug("OperNone '%S' \n", featinTmpStr); */
	    if(ajStrGetLen(featId))
	    {
		/* ajDebug("External entryid '%S'\n", featId); */
		RemoteId = ajTrue;
	    }
	    if(!featEmblLoc(featinTmpStr, &begstr, &Between, &Simple, &endstr))
	    {
		ajStrAssignS(&begstr, featinTmpStr);
		ajStrAssignS(&endstr, begstr);
		Simple = ajTrue;
		ajDebug("Bad feature numeric location '%S' in '%S' - "
		   "test later for label",
		   begstr, locstr);
	    }

	    ajStrAssignS(&featinTmpStr, rest);

	    if(!HasOper)
		ajStrAssignS(&opval, featinTmpStr);
	    
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
		ajFeatWarn("%S: Simple feature location '%S' in '%S'",
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
	
	Flags =  0;
	if(LocFwd)
	    Strand = '+';
	else
	    Strand = '-';
	
	if(Simple)
	    Flags |= AJFEATFLAG_POINT;
	if(Between)
	    Flags |= AJFEATFLAG_BETWEEN_SEQ;
	if(End2)
	    Flags |= AJFEATFLAG_END_TWO;
	if(Beg2)
	    Flags |= AJFEATFLAG_START_TWO;
	if(BegBound)
	    Flags |= AJFEATFLAG_START_BEFORE_SEQ;
	if(EndBound)
	    Flags |= AJFEATFLAG_END_AFTER_SEQ;
	if(RemoteId)
	    Flags |= AJFEATFLAG_REMOTEID;
	if(IsLabel)
	    Flags |= AJFEATFLAG_LABEL;
	if(IsLabel)
	    ajFeatWarn("%S: Feature location with label '%S'",
		   thys->Seqid, locstr);
	if(Join || Order)
	    Flags |= AJFEATFLAG_MULTIPLE;
	if(Order)
	    Flags |= AJFEATFLAG_ORDER;
	
	if(Parent)
	{
	    if(!Fwd)
		Flags |= AJFEATFLAG_COMPLEMENT_MAIN;
	}

	/* ajDebug("Calling featFeatNew, Flags: %x\n", Flags); */
	tmpft = ajFeattypeGetInternalNuc(feature);

        if(Parent)
        {
            if(ajStrGetLen(opval) && RemoteId)
            {
                tmpbeg = 0;
                tmpend = 0;
                tmpflags = Flags & noparentflags;
                tmpfeatid = NULL;
            }
            else
            {
                tmpbeg = BegNum;
                tmpend = EndNum;
                tmpflags = Flags;
                tmpfeatid = featId;
            }

            gf = ajFeatNewNucFlags(thys,
                                   source,	/* source sequence */
                                   tmpft,
                                   tmpbeg, tmpend,
                                   Score,
                                   Strand,
                                   Frame,
                                   Exon, Beg2, End2,
                                   tmpfeatid, featLabel, tmpflags);
/*            gfpos = gf; */
        }

        if(!Parent || ajStrGetLen(opval))
        {
            Exon++;
            /*gfpos = */ ajFeatNewNucFlagsSub(thys,
                                         gf,
                                         source,	/* source sequence */
                                         tmpft,
                                         BegNum, EndNum,
                                         Score,
                                         Strand,
                                         Frame,
                                         Exon, Beg2, End2,
                                         featId, featLabel, Flags);
            if(!RemoteId)
            {
                if(Fwd)
                {
                    if(!gf->Start || gf->Start > BegNum)
                        gf->Start = BegNum;
                    if(!gf->End || gf->End < EndNum)
                        gf->End = EndNum;
                }
                else
                {
                    if(!gf->End || gf->End > BegNum)
                        gf->End = BegNum;
                    if(!gf->Start || gf->Start < EndNum)
                        gf->Start = EndNum;
                }
            }
        }

	if(Parent)
        {
	    ret = gf;
            Parent = ajFalse;
            Exon = 1;
        }
    }
    
    while(ajStrGetLen(*tags))
    {
	itags++;

	if(featEmblTvTagVal(tags, &tag, &val))
	{
	    ajStrQuoteStrip(&val);

            if(ajStrMatchC(tag, "codon_start"))
            {
                ajStrToInt(val, &Frame);
                gf->Frame = Frame;
            }
            if(ajStrMatchC(tag, "note") && ajStrGetCharFirst(val) == '*')
            {
                ajUser("special note '%S'", val);
                ajStrCutStart(&val, 1);
                ajStrExtractFirst(val, &tagrest, &tagname);

                if(ajStrMatchC(tagname, "Type"))
                {
                    ajStrExtractFirst(tagrest, &tagname, &gf->Type);
                }
                else
                {
                    ajFeatGfftagAddSS(gf, tagname, tagrest);
                }
                continue;

            }
	    if(!ajFeatTagAddSS(ret, tag, val))
		ajFeatWarn("%S: Bad value '%S' for tag '/%S'",
		       thys->Seqid, val, tag);

	}
	else if(featEmblTvRest(tags, &featinTmpStr))
	{
	    /* anything non-whitespace up to '/' is bad */
	    ajFeatWarn("Bad feature syntax %S: skipping '%S'",
		   thys->Seqid, featinTmpStr);
	}
	else
	{
	    ajFeatWarn("Bad feature syntax %S: giving up at '%S'",
		   thys->Seqid, *tags);
	    ajStrAssignClear(tags);
	}
   
    }

    ajDebug("featEmblProcess found %d feature tags\n", itags);

    ajStrDelStatic(&featinTmpStr);
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
    ajStrDel(&tagname);
    ajStrDel(&tagrest);
    
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
**
** @release 3.0.0
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




/* @funcstatic featEmblOperIn *************************************************
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
**
** @release 3.0.0
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
**
** @release 3.0.0
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




/* @funcstatic featEmblLoc ****************************************************
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
**
** @release 3.0.0
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
** @param [w] num [ajuint*] Base position
** @return [AjBool] ajTrue if a match was found
**                  ajFalse means an error occurred
**
** @release 3.0.0
******************************************************************************/

static AjBool featEmblLocNum(const AjPStr loc, AjBool* bound, ajuint* num)
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
** @param [w] num1 [ajuint*] First base position
** @param [w] num2 [ajuint*] Last base position
** @return [AjBool] ajTrue if a match was found
**                  ajFalse means an error occurred
**
** @release 3.0.0
******************************************************************************/

static AjBool featEmblLocRange(const AjPStr loc, ajuint* num1, ajuint* num2)
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
**
** @release 3.0.0
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




/* @funcstatic featEmblTvRest *************************************************
**
** Process bad tag-value text up to next qualifier
**
** @param [u] tags [AjPStr*] Feature tag-values string, returned with
**                          skipped text removed
** @param [w] skip [AjPStr*] Skipped text
** @return [AjBool] ajTrue if a match was found
**                  ajFalse means an error occurred
**
** @release 3.0.0
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




/* @funcstatic featGff2FromLine ***********************************************
**
** Converts an input GFF format line into a feature
**
** @param [u] thys [AjPFeattable] Feature table
** @param [r] line [const AjPStr] Input line
** @param [r] version [float] GFF version (1.0 for old format behaviour)
** @return [AjPFeature] New feature
**
** @release 6.4.0
** @@
******************************************************************************/

static AjPFeature featGff2FromLine(AjPFeattable thys, const AjPStr line,
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
	return NULL;
    
    ajStrTokenAssignC(&featGffSplit, line, "\t");
    
    if(!ajStrTokenNextParse(&featGffSplit, &featSeqid))	/* seqname */
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
        gf = ajFeatNewProt(thys,
                           featSource,
                           featFeature,
                           Start, End,
                           fscore);
    else
        gf = ajFeatNewNuc(thys,
                          featSource,
                          featFeature,
                          Start, End,
                          fscore,
                          strand,
                          frame,
                          0,0,0, NULL, NULL);
    if(ajStrTokenRestParse(&featGffSplit, &featGroup))
	featGff2ProcessTagval(gf, thys, featGroup, version);

    ajStrDel(&token);

    return gf;
    
 Error:
    
    ajStrTokenDel(&featGffSplit);
    
    ajStrDelStatic(&featSeqid);
    ajStrDelStatic(&featSource);
    ajStrDelStatic(&featFeature);
    ajStrDel(&token);
    
    return gf;
}




/* @funcstatic featGff3FromLine ***********************************************
**
** Converts an input GFF3 format line into a feature
**
** @param [u] thys [AjPFeattable] Feature table
** @param [r] line [const AjPStr] Input line
** @param [u] idtable [AjPTable] Table of identifiers and features
** @param [u] childlist [AjPList] List of child features
** @return [AjPFeature] New feature
**
** @release 6.0.0
** @@
******************************************************************************/

static AjPFeature featGff3FromLine(AjPFeattable thys, const AjPStr line,
                                   AjPTable idtable, AjPList childlist)
{
    AjPFeature gf           = NULL;
    AjPFeature gftop        = NULL;
    AjPFeature gfknown      = NULL;
    AjPStr token   = NULL;
    ajint Start  = 0;
    ajint End    = 0;
    float fscore = 0.0;
    
    char   strand;
    ajint   frame;

    AjPStr idstr = NULL;
    AjPStr idtag = NULL;
    AjBool hasparent = ajFalse;
    AjIList tagiter = NULL;
    AjPTagval tagval = NULL;

    if(!ajStrGetLen(line))
	return NULL;
    
    ajStrTokenAssignC(&featGffSplit, line, "\t");
    
    if(!ajStrTokenNextParse(&featGffSplit, &featSeqid))	/* seqname */
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

    if(!ajStrGetLen(thys->Seqid))
        ajStrAssignS(&thys->Seqid, featSeqid);

	/* feature object construction
	   and group tag */

    if(ajStrMatchC(thys->Type, "P"))
        gf = ajFeatNewProt(thys,
                           featSource,
                           featFeature,
                           Start, End,
                           fscore);
    else
        gf = ajFeatNewNuc(thys,
                          featSource,
                          featFeature,
                          Start, End,
                          fscore,
                          strand,
                          frame,
                          0,0,0, NULL, NULL);

    if(!ajStrMatchS(featSeqid, thys->Seqid))
    {
        ajStrAssignS(&gf->Remote, featSeqid);
        gf->Flags |= AJFEATFLAG_REMOTEID;
    }

    if(ajStrTokenRestParse(&featGffSplit, &featGroup))
    {
        idstr = featGff3ProcessTagval(gf, thys, featGroup, &hasparent);
        if(idstr)
        {
            gfknown = ajTableFetchmodS(idtable, idstr);
            if(gfknown) 
            {
                if(ajListGetLength(gfknown->Subfeatures))
                {
                    gftop = gfknown;
                    ajDebug("known gff3 ID add '%S' %Lu '%S' %u..%u "
                            "match '%S' %u..%u\n",
                            idstr, ajListGetLength(gftop->Subfeatures),
                            gf->Type, gf->Start, gf->End,
                            gfknown->Type, gfknown->Start, gfknown->End);
                    ajStrDel(&idstr);
                }
                else 
                {
                    ajDebug("known gff3 ID new top '%S' '%S' %u..%u %p "
                            "match '%S' %u..%u %p\n",
                            idstr, gf->Type, gf->Start, gf->End, gf,
                            gfknown->Type, gfknown->Start, gfknown->End,
                            gfknown);

                    gftop = gfknown;
                    gfknown = ajFeatNewFeat(gftop);
                    gftop->Flags |= AJFEATFLAG_GENERATED;
                    gftop->Flags |= AJFEATFLAG_MULTIPLE;
                    if(!gftop->Subfeatures)
                        gftop->Subfeatures = ajListNew();
                    idtag = ajStrNewS(idstr);
                    ajStrAppendC(&idtag, ".emboss");
                    ajFeatGfftagAddCS(gftop, "ID", idtag);
                    ajStrDel(&idtag);
                    ajListPushAppend(gftop->Subfeatures, gfknown);
                    ajTablePut(idtable, idstr, gftop);
                }

                if(gf->Start < gftop->Start)
                    gftop->Start = gf->Start;
                if(gf->End > gftop->End)
                    gftop->End = gf->End;
                ajListPopLast(thys->Features, (void**) &gf);
                ajListPushAppend(gftop->Subfeatures, gf);

                tagiter = ajListIterNewread(gf->GffTags);

                while(!ajListIterDone(tagiter))
                {
                    tagval = ajListIterGet(tagiter);

                    if(ajStrMatchC(MAJTAGVALGETTAG(tagval), "ID"))
                       continue;
                    if(ajStrMatchC(MAJTAGVALGETTAG(tagval), "Parent"))
                       continue;
                    ajFeatGfftagAddSS(gftop, MAJTAGVALGETTAG(tagval),
                                      MAJTAGVALGETVALUE(tagval));
                }

                ajListIterDel(&tagiter);
            }
            else
            {
                ajTablePut(idtable, idstr, gf);
                if(hasparent)
                {
                    ajListPopLast(thys->Features, (void**) &gf);
                    ajListPushAppend(childlist, gf);
                }
            }
        }
        else if(hasparent)
        {
            ajListPopLast(thys->Features, (void**) &gf);
            ajListPushAppend(childlist, gf);
        }
    }

    ajStrDel(&token);

    return gf;
    
 Error:
    
    ajStrTokenDel(&featGffSplit);
    
    ajStrDelStatic(&featSeqid);
    ajStrDelStatic(&featSource);
    ajStrDelStatic(&featFeature);
    ajStrDel(&token);
    
    return gf;
}




/* @funcstatic featGff3oldFromLine ********************************************
**
** Converts an input GFF3 format line into a feature for EMBOSS before 6.4.0
**
** @param [u] thys [AjPFeattable] Feature table
** @param [r] line [const AjPStr] Input line
** @return [AjPFeature] New feature
**
** @release 6.4.0
** @@
******************************************************************************/

static AjPFeature featGff3oldFromLine(AjPFeattable thys, const AjPStr line)
{
    AjPFeature gf           = NULL;
    AjPStr token   = NULL;
    ajint Start  = 0;
    ajint End    = 0;
    float fscore = 0.0;
    
    char   strand;
    ajint   frame;

    if(!ajStrGetLen(line))
	return NULL;
    
    ajStrTokenAssignC(&featGffSplit, line, "\t");
    
    if(!ajStrTokenNextParse(&featGffSplit, &featSeqid))	/* seqname */
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
        gf = ajFeatNewProt(thys,
                           featSource,
                           featFeature,
                           Start, End,
                           fscore);
    else
        gf = ajFeatNewNuc(thys,
                          featSource,
                          featFeature,
                          Start, End,
                          fscore,
                          strand,
                          frame,
                          0,0,0, NULL, NULL);

    if(ajStrTokenRestParse(&featGffSplit, &featGroup))
	featGff3oldProcessTagval(gf, thys, featGroup);

    ajStrDel(&token);

    return gf;
    
 Error:
    
    ajStrTokenDel(&featGffSplit);
    
    ajStrDelStatic(&featSeqid);
    ajStrDelStatic(&featSource);
    ajStrDelStatic(&featFeature);
    ajStrDel(&token);
    
    return gf;
}




/* @funcstatic featReadGff2 ***************************************************
**
** Read input file in GFF 2 format
**
** @param [u] feattabin [AjPFeattabin] Feature table input
** @param [u] ftable [AjPFeattable] Feature table
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool featReadGff2(AjPFeattabin feattabin, AjPFeattable ftable)
{
    AjPStr line  = NULL;
    AjPStr verstr       = NULL;
    AjPStr start        = NULL;
    AjPStr end          = NULL;
    AjPStr type  = NULL;

    AjBool found  = ajFalse;
    float version = 2.0;
    
    AjPFilebuff file = feattabin->Input->Filebuff;

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
	         AjPStr year  = NULL;
	         AjPStr month = NULL;
	         AjPStr day   = NULL;
	         ajint nYear, nMonth, nDay;
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
	    ajRegSubI(GffRegexregion, 1, &ftable->Seqid);
	    ajRegSubI(GffRegexregion, 2, &start);
	    ajRegSubI(GffRegexregion, 3, &end);
	    ajStrToUint(start, &(ftable->Start));
	    ajStrToUint(end,   &(ftable->End));
	    ajStrDel(&start);
	    ajStrDel(&end);
	}
	else if(ajRegExec(GffRegextype,line))
	{
	    ajRegSubI(GffRegextype, 1, &type);

	    if(ajStrMatchCaseC(type, "Protein"))
		ajFeattableSetProt(ftable);
	    else
		ajFeattableSetNuc(ftable);

	    ajStrDel(&type);
	    ajRegSubI(GffRegextype, 3, &ftable->Seqid);
	}
	else if(ajRegExec(GffRegexcomment,line))
	    version = 2.0;      /* ignore for now... could store them in
				 ajFeattable for future reference though?...*/
	/* the real feature stuff */
	else		       /* must be a real feature at last !! */
	    if(featGff2FromLine(ftable, line, version)) /* for ajFeattableAdd */
		found = ajTrue;

    }
    ajStrDel(&line);

    return found;
}




/* @funcstatic featReadGff3 ***************************************************
**
** Read input file in GFF3 format
**
** @param [u] feattabin [AjPFeattabin] Feature table input
** @param [u] ftable [AjPFeattable] Feature table
** @return [AjBool] ajTrue on success
**
** @release 6.0.0
** @@
******************************************************************************/

static AjBool featReadGff3(AjPFeattabin feattabin, AjPFeattable ftable)
{
    AjPStr line  = NULL;
    AjPStr verstr       = NULL;
    AjPStr start        = NULL;
    AjPStr end          = NULL;
    ajlong fpos     = 0;

    AjBool found  = ajFalse;
    AjBool oldemboss = ajFalse;
    float version = 3.0;
    char cp;
    AjPFilebuff file = feattabin->Input->Filebuff;
    AjBool haveversion = ajFalse;

    AjPTable idtable = NULL;
    AjPList childlist = NULL;

    idtable = ajTablestrNew(1000);
    childlist = ajListNew();

    /* ajDebug("featReadGff3..........\n"); */
    
    while(ajBuffreadLinePosStore(file, &line, &fpos,
                                 feattabin->Input->Text,
                                 &ftable->TextPtr))
    {	
	ajStrTrimWhite(&line);
	
	/* Header information */            
	
	if(ajRegExec(Gff3Regexdirective,line))
	{
	    if(ajRegExec(Gff3Regexversion,line))
            {
                if(haveversion) /* starting another feature table */
                {
                    ajFilebuffClearStore(file, 1, line,
                                         feattabin->Input->Text,
                                         &ftable->TextPtr);

                    featGff3Matchtable(ftable, &idtable, &childlist);
    
                    ajStrDel(&line);

                    return found;
                }

                ajRegSubI(Gff3Regexversion, 1, &verstr);
                ajStrToFloat(verstr, &version);
                ajStrDel(&verstr);

                haveversion = ajTrue;
                if(version < 3.0)
                {
                    ajStrDel(&line);
                    ajTableDel(&idtable);
                    ajListFree(&childlist);

                    return ajFalse;
                }
            }
            else if(ajRegExec(Gff3Regexregion,line))
            {
                start = ajStrNew();
                end   = ajStrNew();
                ajRegSubI(Gff3Regexregion, 1, &ftable->Seqid);
                ajRegSubI(Gff3Regexregion, 2, &start);
                ajRegSubI(Gff3Regexregion, 3, &end);
                ajStrToUint(start, &(ftable->Start));
                ajStrToUint(end,   &(ftable->End));
                ajStrDel(&start);
                ajStrDel(&end);
            }
        }
	else if(ajRegExec(Gff3Regexcomment,line))
	{
            if(ajStrPrefixC(line, "#!Source-version EMBOSS 6."))
            {
                cp = ajStrGetCharPos(line, 26);
                if(cp >= '0'&& cp <= '3')
                    oldemboss = ajTrue;
            }
            if(ajStrPrefixC(line, "#!Type Protein"))
            {
                ajFeattableSetProt(ftable);
            }
        }
	/* the real feature stuff */
	else		       /* must be a real feature at last !! */
        {
            if(oldemboss)
            {
                if(featGff3oldFromLine(ftable, line)) /* does ajFeattableAdd */
                    found = ajTrue;
            }
            else
            {
                if(featGff3FromLine(ftable, line, idtable, childlist))
                    found = ajTrue;
            }
        }
    }

    featGff3Matchtable(ftable, &idtable, &childlist);
    
    ajStrDel(&line);

    return found;
}




/* @funcstatic featGff3Matchtable *********************************************
**
** Match GFF3 features with parent values to their parent features
**
** @param [u] ftable [AjPFeattable] Feature table
** @param [d] idtable [AjPTable*] Pointer to table of features by identifier
** @param [d] childlist [AjPList*] Pointer to list of child features
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void featGff3Matchtable(AjPFeattable ftable,
                               AjPTable *idtable, AjPList *childlist)
{
    AjPFeature gf = NULL;
    AjPFeature gfid = NULL;
    const AjPStr idparent = NULL;

    ajDebug("featGff3Matchtable feats %Lu idtable: %Lu childlist: %Lu\n",
            ajListGetLength(ftable->Features),
            ajTableGetLength(*idtable), ajListGetLength(*childlist));

    while(ajListGetLength(*childlist))
    {
        ajListPop(*childlist, (void**) &gf);
        ajListDrop(ftable->Features, gf);
        idparent = ajFeatGetParent(gf);
        if(idparent)
        {
            ajDebug("  parent '%S' '%S' %u..%u\n",
                    idparent, gf->Type, gf->Start, gf->End);
            gfid = ajTableFetchmodV(*idtable, idparent);
            if(gfid)
            {
                ajDebug(" parent '%S' matching\n",
                           idparent);
                if(ajStrMatchS(gf->Type, gfid->Type))
                    gfid->Flags |= AJFEATFLAG_MULTIPLE;
                ajListPushAppend(gfid->Subfeatures, gf);
            }
            else
            {
                ajDebug(" ++ parent '%S' no matching id\n",
                           idparent);
                ajFeatWarn("featGff3Matchtable GFF3 Parent '%S' no matching id",
                           idparent);
                ajListPushAppend(ftable->Features, gf);
            }
        }
        else
        {
            ajDebug("  ++ child with no parent %S %u..%u\n",
                   gf->Type, gf->Start, gf->End);
            ajFeatWarn("Child feature with no parent %S %u..%u",
                   gf->Type, gf->Start, gf->End);
            ajListPushAppend(ftable->Features, gf);
        }
    }

    ajDebug("featGff3Matchtable done feats %Lu\n",
            ajListGetLength(ftable->Features));

    ajTableDel(idtable);
    ajListFree(childlist);

    return;
}




/* @funcstatic featReadGff3old ************************************************
**
** Read input file in GFF3 format from EMBOSS releases before 6.4.0
**
** @param [u] feattabin [AjPFeattabin] Feature table input
** @param [u] ftable [AjPFeattable] Feature table
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool featReadGff3old(AjPFeattabin feattabin, AjPFeattable ftable)
{
    AjPStr line  = NULL;
    AjPStr verstr       = NULL;
    AjPStr start        = NULL;
    AjPStr end          = NULL;

    AjBool found  = ajFalse;
    float version = 3.0;
    
    AjPFilebuff file = feattabin->Input->Filebuff;

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
	    ajRegSubI(Gff3Regexregion, 1, &ftable->Seqid);
	    ajRegSubI(Gff3Regexregion, 2, &start);
	    ajRegSubI(Gff3Regexregion, 3, &end);
	    ajStrToUint(start, &(ftable->Start));
	    ajStrToUint(end,   &(ftable->End));
	    ajStrDel(&start);
	    ajStrDel(&end);
	}
	/* the real feature stuff */
	else		       /* must be a real feature at last !! */
	    if(featGff3oldFromLine(ftable, line)) /* does ajFeattableAdd */
		found = ajTrue;

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
**
** @release 6.2.0
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

    if(!featinSourceRefseqp)
	featinSourceRefseqp = ajStrNewC("REFSEQP");
    
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

	gf = featRefseqpProcess(thys, *savefeat, featinSourceRefseqp,
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
**
** @release 6.2.0
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
    ajuint BegNum = 0;
    ajuint EndNum = 0;
    ajuint Beg2   = 0;
    ajuint End2   = 0;
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
    ajlong ipos;
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
	      featEmblOperOut(opval, &opnam, &featinTmpStr))
	{
	    if(!ajStrHasParentheses(featinTmpStr))
		break;

	    /* ajDebug("OperOut %S( '%S' )\n", opnam, featinTmpStr); */
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
		ajFeatWarn("%S: unrecognised operator '%S()' in '%S'",
		       thys->Seqid, opnam, opval);

	    ajStrAssignS(&opval, featinTmpStr);
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

	if(featEmblOperIn(opval, &opnam, &locstr, &featinTmpStr))
	{
	    /* ajDebug("OperIn %S( '%S' )\n", opnam, locstr); */
	    if(ajStrMatchCaseC(opnam, "complement"))
		LocFwd = !LocFwd;

	    ajStrAssignS(&opval, featinTmpStr);
	    /* ajDebug("rest: '%S'\n", opval); */
	    HasOper = ajTrue;
	}
	else
	{
	    ajStrAssignS(&locstr, opval);
	    /* ajDebug("OperIn simple '%S'\n", locstr); */
	}

	if(featEmblOperNone(locstr,
			    &featId, &featinTmpStr, &rest))  /* one exon */
	{
	    /* ajDebug("OperNone '%S' \n", featinTmpStr); */
	    if(ajStrGetLen(featId))
	    {
		/* ajDebug("External entryid '%S'\n", featId); */
		RemoteId = ajTrue;
	    }
	    if(!featEmblLoc(featinTmpStr, &begstr, &Between, &Simple, &endstr))
	    {
		ajStrAssignS(&begstr, featinTmpStr);
		ajStrAssignS(&endstr, begstr);
		Simple = ajTrue;
		ajDebug("Bad feature numeric location '%S' in '%S' - "
		   "test later for label",
		   begstr, locstr);
	    }

	    ajStrAssignS(&featinTmpStr, rest);

	    if(!HasOper)
		ajStrAssignS(&opval, featinTmpStr);
	    
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
		ajFeatWarn("%S: Simple feature location '%S' in '%S'",
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
	    Flags |= AJFEATFLAG_POINT;
	if(Between)
	    Flags |= AJFEATFLAG_BETWEEN_SEQ;
	if(End2)
	    Flags |= AJFEATFLAG_END_TWO;
	if(Beg2)
	    Flags |= AJFEATFLAG_START_TWO;
	if(BegBound)
	    Flags |= AJFEATFLAG_START_BEFORE_SEQ;
	if(EndBound)
	    Flags |= AJFEATFLAG_END_AFTER_SEQ;
	if(RemoteId)
	    Flags |= AJFEATFLAG_REMOTEID;
	if(IsLabel)
	    Flags |= AJFEATFLAG_LABEL;
	if(IsLabel)
	    ajFeatWarn("%S: Feature location with label '%S'",
		   thys->Seqid, locstr);
	if(Join || Order)
	    Flags |= AJFEATFLAG_MULTIPLE;
	if(Order)
	    Flags |= AJFEATFLAG_ORDER;
	
        ExonFlags = Flags;
        
	if(Mother)
	{
	    if(!Fwd)
		Flags |= AJFEATFLAG_COMPLEMENT_MAIN;
	}

	/* ajDebug("Calling featFeatNew, Flags: %x\n", Flags); */
	tmpft = ajFeattypeGetInternalRefseqp(feature);

        if(!Mother)
            Exon++;

        if(Mother)
            gf = ajFeatNewProtFlags(thys,
                                    source,	/* source sequence */
                                    tmpft,
                                    BegNum, EndNum,
                                    Score,
                                    Flags);
        else
            gf = ajFeatNewProtFlagsSub(thys,
                                       gf,
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

	    if(!ajFeatTagAddSS(ret, tag, val))
		ajFeatWarn("%S: Bad value '%S' for tag '/%S'",
		       thys->Seqid, val, tag);

            if(ajStrMatchC(tag, "codon_start"))
            {
                ajStrToInt(val, &Frame);
                gf->Frame = Frame;
            }
	}
	else if(featEmblTvRest(tags, &featinTmpStr))
	{
	    /* anything non-whitespace up to '/' is bad */
	    ajFeatWarn("Bad feature syntax %S: skipping '%S'",
		   thys->Seqid, featinTmpStr);
	}
	else
	{
	    ajFeatWarn("Bad feature syntax %S: giving up at '%S'",
		   thys->Seqid, *tags);
	    ajStrAssignClear(tags);
	}
   
    }

    ajDebug("featRefseqpProcess found %d feature tags\n", itags);

    ajStrDelStatic(&featinTmpStr);
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




/* @funcstatic featRegInitEmbl ************************************************
**
** Initialise regular expressions and data structures for
** EMBL/GenBank/DDBJ format
**
** @return [AjBool] ajTrue if successful
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool featRegInitEmbl(void)
{
    if(FeatInitEmbl)
	return ajTrue;

    ajFeatVocabInit("embl");

    FeatInitEmbl = ajTrue;

    return ajTrue;
}




/* @funcstatic featRegInitRefseqp *********************************************
**
** Initialise regular expressions and data structures for
** RefSeq protein format
**
** @return [AjBool] ajTrue if successful
**
** @release 6.2.0
** @@
******************************************************************************/

static AjBool featRegInitRefseqp(void)
{
    if(FeatInitRefseqp)
	return ajTrue;

    ajFeatVocabInit("refseqp");

    FeatInitRefseqp = ajTrue;

    return ajTrue;
}




/* @funcstatic featRegInitSwiss ***********************************************
**
** Initialise regular expressions and data structures for
** SwissProt format
**
** @return [AjBool] ajTrue if successful
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool featRegInitSwiss(void)
{
    if(FeatInitSwiss)
	return ajTrue;

    ajFeatVocabInit("swiss");

    if(!SwRegexNew)
	SwRegexNew = ajRegCompC("^FT   (([^ ]+) +([?<]?[0-9]+|[?]) +"
				"([?>]?[0-9]+|[?]) *)(.*)$");
    if(!SwRegexNext)
	SwRegexNext = ajRegCompC("^FT    +(.*)$");

    if(!SwRegexComment)
	SwRegexComment = ajRegCompC("^(.*)[(]([^)]+)[)]$");

    if(!SwRegexFtid)
	SwRegexFtid = ajRegCompC("^(.*)/FTId=([^ .]+)$");

    FeatInitSwiss = ajTrue;

    return ajTrue;
}




/* @funcstatic featRegInitPir *************************************************
**
** Initialise regular expressions and data structures for ajFeat in
** PIR format
**
** @return [AjBool] ajTrue if successful
**
** @release 2.0.0
** @@
******************************************************************************/

static AjBool featRegInitPir(void)
{
    if(FeatInitPir)
	return ajTrue;

    ajFeatVocabInit("pir");

    if(!PirRegexAll)
	PirRegexAll = ajRegCompC("^F;([^/]+)/([^:]+):([^#]*)");

    if(!PirRegexCom)
	PirRegexCom = ajRegCompC("^#([^#]*)");

    if(!PirRegexLoc)
	PirRegexLoc = ajRegCompC("^([^,]+),?");

    if(!PirRegexPos)
	PirRegexPos = ajRegCompC("^([^-]+)-?");

    FeatInitPir = ajTrue;

    return ajTrue;
}




/* @funcstatic featRegInitGff2 ************************************************
**
** Initialise regular expressions and data structures for ajFeat GFF format
**
** @return [AjBool] ajTrue if successful
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool featRegInitGff2(void)
{
    /* Setup any global static runtime resources here
       for example, regular expression compilation calls */

    if(FeatInitGff2)
	return ajTrue;

    ajFeatVocabInit("gff2");
    ajFeatVocabInit("gff2protein");

    /*ajDebug("featRegInitGff2 Compiling regexps\n");*/

    GffRegexNumeric   = ajRegCompC("^[\\+-]?[0-9]+\\.?[0-9]*$");
    GffRegexblankline = ajRegCompC("^[ ]*$");
    GffRegexversion   = ajRegCompC("^##gff-version[ ]+([0-9]+)");
    GffRegexdate      = ajRegCompC("^##date[ ]+([0-9][0-9][0-9][0-9])-"
				   "([0-9][0-9]?)-([0-9][0-9]?)");
    GffRegexregion    = ajRegCompC("^##sequence-region[ ]+([0-9a-zA-Z]+)"
				   "[ ]+([\\+-]?[0-9]+)[ ]+([\\+-]?[0-9]+)");
    GffRegexcomment   = ajRegCompC("^#[ ]*(.*)");
    GffRegextype      = ajRegCompC("^##[Tt]ype +(\\S+)( +(\\S+))?");

    GffRegexTvTagval  = ajRegCompC(" *([^ =]+)[ =](\"[^\"]*\"|"
			 	   "[^;]+)(;|$)"); /* "tag name */

    FeatInitGff2 = ajTrue;

    return ajTrue;
}




/* @funcstatic featRegInitGff3 ************************************************
**
** Initialise regular expressions and data structures for ajFeat GFF3 format
**
** @return [AjBool] ajTrue if successful
**
** @release 6.0.0
** @@
******************************************************************************/

static AjBool featRegInitGff3(void)
{
    /* Setup any global static runtime resources here
       for example, regular expression compilation calls */

    if(FeatInitGff3)
	return ajTrue;

    ajFeatVocabInit("gff3");
    ajFeatVocabInit("gff3protein");

    /*ajDebug("featRegInitGff3 Compiling regexps\n");*/

    Gff3RegexNumeric   = ajRegCompC("^[\\+-]?[0-9]+\\.?[0-9]*$");
    Gff3Regexblankline = ajRegCompC("^[ ]*$");
    Gff3Regexversion   = ajRegCompC("^##gff-version[ ]+([0-9]+)");
    Gff3Regexregion    = ajRegCompC("^##sequence-region[ ]+([0-9a-zA-Z]+)"
				   "[ ]+([\\+-]?[0-9]+)[ ]+([\\+-]?[0-9]+)");
    Gff3Regexdirective = ajRegCompC("^##(.*)");
    Gff3Regexcomment   = ajRegCompC("^#(.*)");
    Gff3RegexTvTagval  = ajRegCompC(" *([^ =]+)[ =]([^;]+)*(;|$)"); /* "tag name */
    Gff3oldRegexTvTagval  = ajRegCompC(" *([^ =]+)[ =](\"[^\"]*\"|"
			 	   "[^;]+)(;|$)"); /* "tag name */

    FeatInitGff3 = ajTrue;

    if(!featRegInitGff2())
        return ajFalse;

    return ajTrue;
}




/* @funcstatic featDelRegEmbl *************************************************
**
** Cleanup and exit routines. Free and destroy regular expressions
**
** @return [AjBool] ajFalse if unsuccessful
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool featDelRegEmbl(void)
{
    if(!FeatInitEmbl)
	return ajTrue;

    FeatInitEmbl = ajFalse;

    return ajTrue;
}




/* @funcstatic featDelRegPir **************************************************
**
** Cleanup and exit routines. Free and destroy regular expressions
**
** @return [AjBool] ajFalse if unsuccessful
**
** @release 2.0.0
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

    FeatInitPir = ajFalse;

    return ajTrue;
}




/* @funcstatic featDelRegRefseqp **********************************************
**
** Cleanup and exit routines. Free and destroy regular expressions
**
** @return [AjBool] ajFalse if unsuccessful
**
** @release 6.2.0
** @@
******************************************************************************/

static AjBool featDelRegRefseqp(void)
{
    if(!FeatInitRefseqp)
	return ajTrue;

    FeatInitRefseqp = ajFalse;

    return ajTrue;
}




/* @funcstatic featDelRegSwiss ************************************************
**
** Cleanup and exit routines. Free and destroy regular expressions
**
** @return [AjBool] ajFalse if unsuccessful
**
** @release 1.0.0
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

    FeatInitSwiss = ajFalse;

    return ajTrue;
}




/* @funcstatic featDelRegGff2 *************************************************
**
** Cleanup and exit routines. Free and destroy regular expressions
**
** @return [AjBool] ajFalse if unsuccessful
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool featDelRegGff2(void)
{
    if(!FeatInitGff2)
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

    FeatInitGff2 = ajFalse;

    return ajTrue;
}




/* @funcstatic featDelRegGff3 *************************************************
**
** Cleanup and exit routines. Free and destroy regular expressions
**
** @return [AjBool] ajFalse if unsuccessful
**
** @release 6.0.0
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
    ajRegFree(&Gff3Regexdirective);
    ajRegFree(&Gff3Regextype);
    ajRegFree(&Gff3RegexTvTagval);
    ajRegFree(&Gff3oldRegexTvTagval);

    FeatInitGff3 = ajFalse;

    featDelRegGff2();

    return ajTrue;
}




/* @func ajFeatreadExit *******************************************************
**
** Cleans up feature table input internal memory
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajFeatreadExit(void)
{
    ajint i;

    for(i=1;featinformatDef[i].Name;i++)
    {
	if(featinformatDef[i].Used)
	{
	    /* Calling funclist featinformatDef() */
	    if(featinformatDef[i].DelReg &&
               !(*featinformatDef[i].DelReg)())
	    {
		/*ajDebug("No DelReg yet for %s\n",featinformatDef[i].Name);*/
		ajErr("No DelReg yet for %s\n",featinformatDef[i].Name);
	    }
	}
    }

    ajRegFree(&featRegFlag);
    ajRegFree(&featRegMore);
    ajRegFree(&featRegGroup);
    ajRegFree(&featRegGff3Group);

    ajStrDel(&featinFormatTmp);
    ajStrDel(&featinValTmp);
    ajStrDel(&featinUfoTest);
    ajStrDel(&featReadLine);
    ajStrDel(&featProcessLine);
    ajStrDel(&featinTmpStr);
    ajStrDel(&featGroup);
    ajStrDel(&featSeqid);
    ajStrDel(&featSource);
    ajStrDel(&featFeature);

    ajStrDel(&featinTagNote);
    ajStrDel(&featinTagComm);
    ajStrDel(&featinTagFtid);

    ajStrDel(&featinSourcePir);
    ajStrDel(&featinSourceSwiss);
    ajStrDel(&featinSourceEmbl);
    ajStrDel(&featinSourceRefseqp);
    ajStrDel(&featId);
    ajStrDel(&featLabel);

    ajStrDel(&featLocStr);
    ajStrDel(&featLocToken);
    ajStrDel(&featLocDb);
    ajStrDel(&featSaveGroupStr);

    ajStrTokenDel(&featGffSplit);
    ajStrTokenDel(&featEmblSplit);

    ajRegFree(&featinRegUfoFmt);
    ajRegFree(&featinRegUfoFile);

    ajRegFree(&featTagTrans);

    ajTableDel(&feattabDbMethods);

    return;
}





/* @section Internals *********************************************************
**
** Functions to return internal values
**
** @nam3rule Type Internals for feature table datatype
** @nam4rule Get  Return a value
** @nam5rule Fields  Known query fields for ajFeatRead
** @nam5rule Qlinks  Known query link operators for ajFeatRead
**
** @valrule * [const char*] Internal value
**
** @fcategory misc
**
******************************************************************************/




/* @func ajFeattabinTypeGetFields *********************************************
**
** Returns the listof known field names for ajFeatRead
**
** @return [const char*] List of field names
**
** @release 6.4.0
** @@
******************************************************************************/

const char* ajFeattabinTypeGetFields(void)
{
    return "id acc";
}




/* @func ajFeattabinTypeGetQlinks *********************************************
**
** Returns the listof known query link operators for ajFeatRead
**
** @return [const char*] List of field names
**
** @release 6.4.0
** @@
******************************************************************************/

const char* ajFeattabinTypeGetQlinks(void)
{
    return "|";
}




/* @funcstatic featFindInformatC **********************************************
**
** Looks for the specified format(s) in the internal definitions and
** returns the index.
**
** Given a single format, sets iformat.
**
** @param [r] format [const char*] Format required.
** @param [w] iformat [ajint*] Index
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool featFindInformatC(const char* format, ajint* iformat)
{
    ajint i = 0;

    /*ajDebug("featFindInformatC '%s'\n", format);*/
    if(!*format)
	return ajFalse;

    ajStrAssignC(&featinFormatTmp, format);
    ajStrFmtLower(&featinFormatTmp);

    for(i=0; featinformatDef[i].Name; i++)
    {
	/*ajDebug("test %d '%s' \n", i, featinformatDef[i].Name);*/
	if(ajStrMatchC(featinFormatTmp,
		       featinformatDef[i].Name))
	{
	    *iformat = i;
	    (void) ajStrDelStatic(&featinFormatTmp);
	    /*ajDebug("found '%s' at %d\n", featinformatDef[i].Name, i);*/
	    return ajTrue;
	}
    }

    ajErr("Unknown input feat format '%s'", format);

    ajStrDelStatic(&featinFormatTmp);

    return ajFalse;
}




/* @funcstatic featFindInformatS **********************************************
**
** Looks for the specified format(s) in the internal definitions and
** returns the index.
**
** Given a single format, sets iformat.
**
** @param [r] format [const AjPStr] Format required.
** @param [w] iformat [ajint*] Index
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool featFindInformatS(const AjPStr format, ajint* iformat)
{
    return featFindInformatC(ajStrGetPtr(format), iformat);
}




/* @func ajFeatinPrintFormat **************************************************
**
** Reports the internal data structures for input feature formats
**
** @param [u] outf [AjPFile] Output file
** @param [r] full [AjBool] Full report (usually ajFalse)
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajFeatinPrintFormat(AjPFile outf, AjBool full)
{
    ajint i = 0;

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# Feature input formats\n");
    ajFmtPrintF(outf, "# Name  Format name (or alias)\n");
    ajFmtPrintF(outf, "# Alias Name is an alias\n");
    ajFmtPrintF(outf, "# Nuc   Valid for nucleotide sequences\n");
    ajFmtPrintF(outf, "# Pro   Valid for protein sequences\n");
    ajFmtPrintF(outf, "# Name         Alias   Nuc   Pro "
		"Description\n");
    ajFmtPrintF(outf, "Informat {\n");

    for(i=0; featinformatDef[i].Name; i++)
    {
	if(full || !featinformatDef[i].Alias)
	    ajFmtPrintF(outf, "  %-12s %5B %5B %5B \"%s\"\n",
			featinformatDef[i].Name,
			featinformatDef[i].Alias,
			featinformatDef[i].Nucleotide,
			featinformatDef[i].Protein,
			featinformatDef[i].Desc);
    }

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajFeatinPrinthtmlFormat **********************************************
**
** Reports the internal data structures for input feature formats
**
** @param [u] outf [AjPFile] Output file
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajFeatinPrinthtmlFormat(AjPFile outf)
{
    ajint i = 0;

    ajFmtPrintF(outf, "<table border=3>");
    ajFmtPrintF(outf, "<tr><th>Features Input Format</th><th>Alias</th>\n");
    ajFmtPrintF(outf, "<th>Nuc</th><th>Pro</th>\n");
    ajFmtPrintF(outf, "<th>Description</th></tr>\n");

    for(i=0; featinformatDef[i].Name; i++)
    {
	if(!featinformatDef[i].Alias)
	    ajFmtPrintF(outf, "<tr><td>\n%-12s\n</td><td>%5B\n</td>"
                        "<td>%5B\n</td><td>%5B\n</td><td>\"%s\"</td></tr>\n",
			featinformatDef[i].Name,
			featinformatDef[i].Alias,
			featinformatDef[i].Nucleotide,
			featinformatDef[i].Protein,
			featinformatDef[i].Desc);
    }

    ajFmtPrintF(outf, "</table>\n");

    return;
}




/* @func ajFeatinPrintbookFormat **********************************************
**
** Reports the input feature format internals in docbook text format
**
** @param [u] outf [AjPFile] Output file
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajFeatinPrintbookFormat(AjPFile outf)
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
                "(\"true\" indicates protein sequence data may be "
                "represented) and <emphasis>Description</emphasis> "
                "(short description of the format).</para>\n\n");

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

    for(i=1; featinformatDef[i].Name; i++)
    {
	if(!featinformatDef[i].Alias)
        {
            namestr = ajStrNewC(featinformatDef[i].Name);
            ajListPush(fmtlist, namestr);
            namestr = NULL;
        }
    }

    ajListSort(fmtlist, &ajStrVcmp);
    ajListstrToarray(fmtlist, &names);

    for(i=0; names[i]; i++)
    {
        for(j=0; featinformatDef[j].Name; j++)
        {
            if(ajStrMatchC(names[i],featinformatDef[j].Name))
            {
                ajFmtPrintF(outf, "    <tr>\n");
                ajFmtPrintF(outf, "      <td>%s</td>\n",
                            featinformatDef[j].Name);
                ajFmtPrintF(outf, "      <td>%B</td>\n",
                            featinformatDef[j].Nucleotide);
                ajFmtPrintF(outf, "      <td>%B</td>\n",
                            featinformatDef[j].Protein);
                ajFmtPrintF(outf, "      <td>%s</td>\n",
                            featinformatDef[j].Desc);
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




/* @func ajFeatinPrintwikiFormat **********************************************
**
** Reports the input feature format internals in wiki text format
**
** @param [u] outf [AjPFile] Output file
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajFeatinPrintwikiFormat(AjPFile outf)
{
    ajint i = 0;
    ajint j = 0;
    AjPStr namestr = NULL;

    ajFmtPrintF(outf, "{| class=\"wikitable sortable\" border=\"2\"\n");
    ajFmtPrintF(outf, "|-\n");
    ajFmtPrintF(outf, "!Format!!Nuc!!Pro!!"
                "class=\"unsortable\"|Description\n");

    for(i=1; featinformatDef[i].Name; i++)
    {
        if(!featinformatDef[i].Alias)
        {
            ajFmtPrintF(outf, "|-\n");
            ajStrAssignC(&namestr, featinformatDef[i].Name);


            for(j=i+1; featinformatDef[j].Name; j++)
            {
                if(featinformatDef[j].Read == featinformatDef[i].Read)
                {
                    ajFmtPrintAppS(&namestr, "<br>%s",
                                   featinformatDef[j].Name);
                    if(!featinformatDef[j].Alias) 
                    {
                        ajWarn("Feature input format '%s' same as '%s' "
                               "but not alias",
                               featinformatDef[j].Name,
                               featinformatDef[i].Name);
                    }
                }
            }
            ajFmtPrintF(outf, "|%S||%B||%B||%s\n",
			namestr,
			featinformatDef[i].Nucleotide,
			featinformatDef[i].Protein,
			featinformatDef[i].Desc);
        }
    }

    ajFmtPrintF(outf, "|}\n\n");

    ajStrDel(&namestr);

    return;
}




/* #datasection [AjPFeattaball] Feature Input Stream **************************
**
** Function is for manipulating feature input stream objects
**
** #nam2rule Feattaball Feature input stream objects
**
******************************************************************************/




/* #section Feature Input StreamConstructors **********************************
**
** All constructors return a new feature input stream object by pointer.
** It is the responsibility of the user to first destroy any previous
** feature input object. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so
** anyway.
**
** #fdata [AjPFeattaball]
**
** #nam3rule New Constructor
**
** #valrule * [AjPFeattaball] Feature input stream object
**
** #fcategory new
**
******************************************************************************/




/* @func ajFeattaballNew ******************************************************
**
** Creates a new feature input stream object.
**
** @return [AjPFeattaball] New feature input stream object.
**
** @release 6.4.0
** @@
******************************************************************************/

AjPFeattaball ajFeattaballNew(void)
{
    AjPFeattaball pthis;

    AJNEW0(pthis);

    pthis->Feattabin = ajFeattabinNew();
    pthis->Feattable = ajFeattableNew(NULL);

    return pthis;
}





/* ==================================================================== */
/* ========================== destructors ============================= */
/* ==================================================================== */




/* #section Feature Input Stream Destructors **********************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the feature input stream object.
**
** #fdata [AjPFeattaball]
**
** #nam3rule Del Destructor
**
** #argrule Del pthis [AjPFeattaball*] Feature input stream
**
** #valrule * [void]
**
** #fcategory delete
**
******************************************************************************/




/* @func ajFeattaballDel ******************************************************
**
** Deletes a feature input stream object.
**
** @param [d] pthis [AjPFeattaball*] Feature input stream
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajFeattaballDel(AjPFeattaball* pthis)
{
    AjPFeattaball thys;

    if(!pthis)
        return;

    thys = *pthis;

    if(!thys)
        return;

    ajFeattabinDel(&thys->Feattabin);
    if(!thys->Returned)
        ajFeattableDel(&thys->Feattable);

    AJFREE(*pthis);

    return;
}




/* ==================================================================== */
/* =========================== Modifiers ============================== */
/* ==================================================================== */




/* #section feature input stream modifiers ************************************
**
** These functions use the contents of a feature input stream object and
** update them.
**
** #fdata [AjPFeattaball]
**
** #nam3rule Clear Clear all values
**
** #argrule * thys [AjPFeattaball] Feature input stream object
**
** #valrule * [void]
**
** #fcategory modify
**
******************************************************************************/




/* @func ajFeattaballClear ****************************************************
**
** Clears a feature input stream object back to "as new" condition,
** except for the query list which must be preserved.
**
** @param [w] thys [AjPFeattaball] Feature input stream
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajFeattaballClear(AjPFeattaball thys)
{

    ajDebug("ajFeattaballClear called\n");

    if(!thys)
        return;

    ajFeattabinClear(thys->Feattabin);

    if(!thys->Returned)
        ajFeattableClear(thys->Feattable);

    return;
}




/* #section Feature input stream casts ****************************************
**
** These functions return the contents of a feature input stream object
**
** #fdata [AjPFeattaball]
**
** #nam3rule Get Get feature input stream values
** #nam3rule Getfeattab Get feature table values
** #nam4rule Id Get identifier of current term
**
** #argrule * thys [const AjPFeattaball] Feature input stream object
**
** #valrule * [const AjPStr] String value
**
** #fcategory cast
**
******************************************************************************/




/* @func ajFeattaballGetfeattableId *******************************************
**
** Returns the identifier of the current feature table in an input stream
**
** @param [r] thys [const AjPFeattaball] Feature input stream
** @return [const AjPStr] Identifier
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPStr ajFeattaballGetfeattableId(const AjPFeattaball thys)
{
    if(!thys)
        return NULL;

    ajDebug("ajFeattaballGetfeattableId called\n");

    return ajFeattableGetName(thys->Feattable);
}




/* @func ajFeattaballNext *****************************************************
**
** Parse a feature query into format, access, file and entry
**
** Split at delimiters. Check for the first part as a valid format
** Check for the remaining first part as a database name or as a file
** that can be opened.
** Anything left is an entryname spec.
**
** Return the results in the AjPFeattable object but leave the file open for
** future calls.
**
** @param [w] thys [AjPFeattaball] Feature input stream
** @param [u] Pfeattable [AjPFeattable*] Feature table returned
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajFeattaballNext(AjPFeattaball thys, AjPFeattable *Pfeattable)
{
    if(!thys->Count)
    {
	thys->Count = 1;

	thys->Totfeattables++;

	*Pfeattable= thys->Feattable;
	thys->Returned = ajTrue;

	return ajTrue;
    }


    if(ajFeattabinRead(thys->Feattabin, thys->Feattable))
    {
	thys->Count++;

	thys->Totfeattables++;

	*Pfeattable = thys->Feattable;
	thys->Returned = ajTrue;

	ajDebug("ajFeattaballNext success\n");

	return ajTrue;
    }

    ajDebug("ajFeattaballNext failed\n");

    ajFeattaballClear(thys);

    return ajFalse;
}




/* #section Cast **************************************************************
**
** Return a reference to the call table
**
**
******************************************************************************/




/* @func ajFeattabaccessGetDb *************************************************
**
** returns the table in which feature table database access details
** are registered
**
** @return [AjPTable] Access functions hash table
**
** @release 6.4.0
** @@
******************************************************************************/

AjPTable ajFeattabaccessGetDb(void)
{
    if(!feattabDbMethods)
        feattabDbMethods = ajCallTableNew();

    return feattabDbMethods;
}





/* @func ajFeattabaccessMethodGetQlinks ***************************************
**
** Tests for a named method for feature table term reading and returns the 
** known query link operators
**
** @param [r] method [const AjPStr] Method required.
** @return [const char*] Known link operators
**
** @release 6.4.0
** @@
******************************************************************************/

const char* ajFeattabaccessMethodGetQlinks(const AjPStr method)
{
    AjPFeattabAccess methoddata; 

    methoddata = ajCallTableGetS(feattabDbMethods, method);
    if(!methoddata)
        return NULL;

    return methoddata->Qlink;
}




/* @func ajFeattabaccessMethodGetScope ****************************************
**
** Tests for a named method for feature table term reading and returns
** the scope (entry, query or all).
*
** @param [r] method [const AjPStr] Method required.
** @return [ajuint] Scope flags
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ajFeattabaccessMethodGetScope(const AjPStr method)
{
    AjPFeattabAccess methoddata; 
    ajuint ret = 0;

    methoddata = ajCallTableGetS(feattabDbMethods, method);
    if(!methoddata)
        return 0;

    if(methoddata->Entry)
        ret |= AJMETHOD_ENTRY;
    if(methoddata->Query)
        ret |= AJMETHOD_QUERY;
    if(methoddata->All)
        ret |= AJMETHOD_ALL;

    return ret;
}




/* @func ajFeattabaccessMethodTest ********************************************
**
** Tests for a named method for feature table reading.
**
** @param [r] method [const AjPStr] Method required.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajFeattabaccessMethodTest(const AjPStr method)
{
    if(ajCallTableGetS(feattabDbMethods, method))
      return ajTrue;

    return ajFalse;
}




/* @funcstatic feattabinQryRestore ********************************************
**
** Restores a feature input specification from a FeatPListUfo node
**
** @param [w] feattabin [AjPFeattabin] feature table input object
** @param [r] node [const FeatPListUfo] Feature list node
** @return [void]
**
** @release 6.4.0
******************************************************************************/

static void feattabinQryRestore(AjPFeattabin feattabin,
                                const FeatPListUfo node)
{
    feattabin->Input->Format = node->Format;
    feattabin->Input->Fpos   = node->Fpos;
    ajStrAssignS(&feattabin->Input->Formatstr, node->Formatstr);

    return;
}




/* @funcstatic feattabinQrySave ***********************************************
**
** Saves a data input specification in a FeatPListUfo node
**
** @param [w] node [FeatPListUfo] Feature list node
** @param [r] feattabin [const AjPFeattabin] Feature table input object
** @return [void]
**
** @release 6.4.0
******************************************************************************/

static void feattabinQrySave(FeatPListUfo node,
                             const AjPFeattabin feattabin)
{
    node->Format = feattabin->Input->Format;
    node->Fpos   = feattabin->Input->Fpos;
    ajStrAssignS(&node->Formatstr, feattabin->Input->Formatstr);

    return;
}




/* @funcstatic feattabinQryProcess ********************************************
**
** Converts a feature table query into an open file.
**
** Tests for "format::" and sets this if it is found
**
** Then tests for "list:" or "@" and processes as a list file
** using feattabinListProcess which in turn invokes feattabinQryProcess
** until a valid query is found.
**
** Then tests for dbname:query and opens the file (at the correct position
** if the database definition defines it)
**
** If there is no database, looks for file:query and opens the file.
** In this case the file position is not known and data data reading
** will have to scan for the entry/entries we need.
**
** @param [u] feattabin [AjPFeattabin] feature table input structure.
** @param [u] ftable [AjPFeattable] Feature table data to be read.
**                         The format will be replaced
**                         if defined in the query string.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool feattabinQryProcess(AjPFeattabin feattabin,
                                  AjPFeattable ftable)
{
    AjBool ret = ajTrue;
    AjPStr qrystr = NULL;
    AjBool featmethod = ajFalse;
    const AjPStr fmtstr = NULL;
    AjPTextin textin;
    AjPQuery qry;
    AjPFeattabAccess feataccess = NULL;

    textin = feattabin->Input;
    qry = textin->Query;

    /* pick up the original query string */
    qrystr = ajStrNewS(textin->Qry);

    ajDebug("feattabinQryProcess '%S'\n", qrystr);

    /* look for a format:: prefix */
    fmtstr = ajQuerystrParseFormat(&qrystr, textin, feattabinformatFind);
    ajDebug("feattabinQryProcess ... fmtstr '%S' '%S'\n", fmtstr, qrystr);

    /* (seq/feat) look for a [range] suffix */
    ajQuerystrParseRange(&qrystr, &feattabin->Start, &feattabin->End,
                         &feattabin->Rev);
    ajDebug("feattabinQryProcess ... range %d..%d rev:%B '%S'\n",
           feattabin->Start, feattabin->End, feattabin->Rev, qrystr);

    /* look for a list:: or (at):: listfile of queries  - process and return */
    if(ajQuerystrParseListfile(&qrystr))
    {
        ajDebug("feattabinQryProcess ... listfile '%S'\n", qrystr);
        ret = feattabinListProcess(feattabin, ftable, qrystr);
        ajStrDel(&qrystr);
        return ret;
    }

    /* try general text access methods (file, asis, text database access */
    ajDebug("feattabinQryProcess ... no listfile '%S'\n", qrystr);
    if(!ajQuerystrParseRead(&qrystr, textin, feattabinformatFind, &featmethod))
    {
        ajStrDel(&qrystr);
        return ajFalse;
    }
    
    feattabinFormatSet(feattabin, ftable);

    ajDebug("feattabinQryProcess ... read nontext: %B '%S'\n",
            featmethod, qrystr);
    ajStrDel(&qrystr);

    /* we found a non-text method */
    if(featmethod)
    {
        ajDebug("feattabinQryProcess ... call method '%S'\n", qry->Method);
        ajDebug("feattabinQryProcess ... textin format %d '%S'\n",
                textin->Format, textin->Formatstr);
        ajDebug("feattabinQryProcess ...  query format  '%S'\n",
                qry->Formatstr);
        qry->Access = ajCallTableGetS(feattabDbMethods,qry->Method);
        feataccess = qry->Access;
        return (*feataccess->Access)(feattabin);
    }

    ajDebug("feattabinQryProcess text method '%S' success\n", qry->Method);

    return ajTrue;
}





/* #datasection [AjPList] Query field list ************************************
**
** Query fields lists are handled internally. Only static functions
** should appear here
**
******************************************************************************/




/* @funcstatic feattabinListProcess *******************************************
**
** Processes a file of queries.
** This function is called by, and calls, feattabinQryProcess. There is
** a depth check to avoid infinite loops, for example where a list file
** refers to itself.
**
** This function produces a list (AjPList) of queries with all list references
** expanded into lists of queries.
**
** Because queries in a list can have their own format
** the prior settings are stored with each query in the list node so that they
** can be restored after.
**
** @param [u] feattabin [AjPFeattabin] Feature table input
** @param [u] ftable [AjPFeattable] Feature table data
** @param [r] listfile [const AjPStr] Name of list file.,
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool feattabinListProcess(AjPFeattabin feattabin,
                                   AjPFeattable ftable,
                                   const AjPStr listfile)
{
    AjPList list  = NULL;
    AjPFile file  = NULL;
    AjPStr token  = NULL;
    AjPStrTok handle = NULL;
    AjBool ret       = ajFalse;
    FeatPListUfo node = NULL;

    static ajint depth    = 0;
    static ajint MAXDEPTH = 16;

    depth++;
    ajDebug("++feattabinListProcess %S depth %d\n",
	    listfile, depth);

    if(depth > MAXDEPTH)
	ajFatal("Query list too deep");

    if(!feattabin->Input->List)
	feattabin->Input->List = ajListNew();

    list = ajListNew();

    file = ajFileNewInNameS(listfile);

    if(!file)
    {
	ajErr("Failed to open list file '%S'", listfile);
	depth--;

	return ret;
    }

    while(ajReadlineTrim(file, &featReadLine))
    {
	feattabinListNoComment(&featReadLine);

	if(ajStrGetLen(featReadLine))
	{
	    ajStrTokenAssignC(&handle, featReadLine, " \t\n\r");
	    ajStrTokenNextParse(&handle, &token);
	    /* ajDebug("Line  '%S'\n");*/
	    /* ajDebug("token '%S'\n", featReadLine, token); */

	    if(ajStrGetLen(token))
	    {
	        ajDebug("++Add to list: '%S'\n", token);
	        AJNEW0(node);
	        ajStrAssignS(&node->Ufo, token);
	        feattabinQrySave(node, feattabin);
	        ajListPushAppend(list, node);
	    }

	    ajStrDel(&token);
	    token = NULL;
	}
    }

    ajFileClose(&file);
    ajStrDel(&token);

    ajDebug("Trace feattabin->Input->List\n");
    ajQuerylistTrace(feattabin->Input->List);
    ajDebug("Trace new list\n");
    ajQuerylistTrace(list);
    ajListPushlist(feattabin->Input->List, &list);

    ajDebug("Trace combined feattabin->Input->List\n");
    ajQuerylistTrace(feattabin->Input->List);

    /*
     ** now try the first item on the list
     ** this can descend recursively if it is also a list
     ** which is why we check the depth above
     */

    if(ajListPop(feattabin->Input->List, (void**) &node))
    {
        ajDebug("++pop first item '%S'\n", node->Ufo);
	ajFeattabinQryS(feattabin, node->Ufo);
	feattabinQryRestore(feattabin, node);
	ajStrDel(&node->Ufo);
	ajStrDel(&node->Formatstr);
	AJFREE(node);
	ajDebug("descending with query '%S'\n", feattabin->Input->Qry);
	ret = feattabinQryProcess(feattabin, ftable);
    }

    ajStrTokenDel(&handle);
    depth--;
    ajDebug("++feattabinListProcess depth: %d returns: %B\n", depth, ret);

    return ret;
}




/* @funcstatic feattabinListNoComment *****************************************
**
** Strips comments from a character string (a line from a list file).
** Comments are blank lines or any text following a "#" character.
**
** @param [u] text [AjPStr*] Line of text from input file.
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void feattabinListNoComment(AjPStr* text)
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




/* @funcstatic feattabinFormatSet *********************************************
**
** Sets the input format for data data using the data data
** input object's defined format
**
** @param [u] feattabin [AjPFeattabin] Dfeature table input.
** @param [u] ftable [AjPFeattable] Feature table data
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool feattabinFormatSet(AjPFeattabin feattabin, AjPFeattable ftable)
{

    if(ajStrGetLen(feattabin->Input->Formatstr))
    {
	ajDebug("... input format value '%S'\n",
                feattabin->Input->Formatstr);

	if(feattabinformatFind(feattabin->Input->Formatstr,
                               &feattabin->Input->Format))
	{
	    ajStrAssignS(&ftable->Formatstr,
                         feattabin->Input->Formatstr);
	    ftable->Format = feattabin->Input->Format;
	    ajDebug("...format OK '%S' = %d\n",
                    feattabin->Input->Formatstr,
		    feattabin->Input->Format);
	}
	else
	    ajDebug("...format unknown '%S'\n",
                    feattabin->Input->Formatstr);

	return ajTrue;
    }
    else
	ajDebug("...input format not set\n");


    return ajFalse;
}




/* @funcstatic featGff3FlagSet ************************************************
**
** Sets the flags for a GFF3 feature.
**
** @param [u] gf       [AjPFeature]  Feature
** @param [r] flagstr  [const AjPStr] Flags as a hexadecimal value
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void featGff3FlagSet(AjPFeature gf, const AjPStr flagstr)
{
    AjPStr savstr     = NULL;
    AjPStr typstr     = NULL;
    AjPStr valstr     = NULL;
    ajint flags = 0;
    ajint num   = 0;

    if(!featRegFlag)
	featRegFlag = ajRegCompC("[ \"]*(0x[0-9a-f]+)");

    if(!featRegMore)
	featRegMore = ajRegCompC("[,]*([^:]+):([^,]+)");

    /*ajDebug("featGff3FlagSet '%S'\n", flagstr);*/
    ajStrAssignS(&savstr, flagstr);

    if(ajRegExec(featRegFlag, savstr))
    {
	ajRegSubI(featRegFlag, 1, &featinTmpStr);

	if(ajStrToHex(featinTmpStr, &flags))
        {
            if(flags & AJFEATFLAG_GROUP)
                flags |= AJFEATFLAG_ORDER;

            if(flags & AJFEATFLAG_ONEOF)
                flags |= AJFEATFLAG_ORDER;

	    gf->Flags |= flags;
        }

	/*ajDebug("flags: %x", gf->Flags);*/
	ajRegPost(featRegFlag, &featinTmpStr);
	ajStrAssignS(&savstr, featinTmpStr);
    }

    while(ajRegExec(featRegMore, savstr))
    {
	ajRegSubI(featRegMore, 1, &typstr);
	ajRegSubI(featRegMore, 2, &valstr);

	/*ajDebug("flag type '%S' val '%S'\n", typstr, valstr);*/

	if(ajStrMatchCaseC(typstr, "start_before"))
	{
	    if(ajStrMatchC(valstr, "true"))
                gf->Flags |= AJFEATFLAG_START_BEFORE_SEQ;
	}
	else if(ajStrMatchCaseC(typstr, "end_after"))
	{
	    if(ajStrMatchC(valstr, "true"))
                gf->Flags |= AJFEATFLAG_END_AFTER_SEQ;
	}
	else if(ajStrMatchCaseC(typstr, "between"))
	{
	    if(ajStrMatchC(valstr, "true"))
                gf->Flags |= AJFEATFLAG_BETWEEN_SEQ;
	}
	else if(ajStrMatchCaseC(typstr, "start2"))
	{
	    if(ajStrToInt(valstr, &num))
            {
		gf->Start2 = num;
                gf->Flags |= AJFEATFLAG_START_TWO;
            }
	}
	else if(ajStrMatchCaseC(typstr, "end2"))
	{
	    if(ajStrToInt(valstr, &num))
            {
		gf->End2 = num;
                gf->Flags |= AJFEATFLAG_END_TWO;
            }
	}
	else if(ajStrMatchCaseC(typstr, "label"))
	{
	    ajFeatWarn("GFF3 label '%S' used", valstr);
	    ajStrAssignS(&gf->Label, valstr);
	}
	else if(ajStrMatchCaseC(typstr, "start_unsure"))
	{
	    if(ajStrMatchC(valstr, "true"))
                gf->Flags |= AJFEATFLAG_START_UNSURE;
	}
	else if(ajStrMatchCaseC(typstr, "end_unsure"))
	{
	    if(ajStrMatchC(valstr, "true"))
                gf->Flags |= AJFEATFLAG_END_UNSURE;
	}
	else if(ajStrMatchCaseC(typstr, "type"))
	{
            ajStrAssignS(&gf->Type, ajFeattypeGetInternal(valstr));

	}
	else
	    ajFeatWarn("Unknown GFF3 featflags type '%S:%S'", typstr, valstr);

	ajRegPost(featRegMore, &featinTmpStr);
	ajStrAssignS(&savstr, featinTmpStr);
    }

    ajStrDel(&savstr);
    ajStrDel(&typstr);
    ajStrDel(&valstr);

    return;
}




/* @funcstatic featFlagSet ****************************************************
**
** Sets the flags for a feature.
**
** @param [u] gf       [AjPFeature]  Feature
** @param [r] flagstr  [const AjPStr] Flags as a hexadecimal value
** @return [void]
**
** @release 2.0.0
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
	ajRegSubI(featRegFlag, 1, &featinTmpStr);

	if(ajStrToHex(featinTmpStr, &flags))
        {
            if(flags & AJFEATFLAG_GROUP)
                flags |= AJFEATFLAG_ORDER;

            if(flags & AJFEATFLAG_ONEOF)
                flags |= AJFEATFLAG_ORDER;

	    gf->Flags = flags;
        }

	/*ajDebug("flags: %x", gf->Flags);*/
	ajRegPost(featRegFlag, &featinTmpStr);
	ajStrAssignS(&savstr, featinTmpStr);
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
	    ajFeatWarn("GFF label '%S' used", valstr);
	    ajStrAssignS(&gf->Label, valstr);
	}
	else
	    ajFeatWarn("Unknown GFF FeatFlags type '%S:%S'", typstr, valstr);

	ajRegPost(featRegMore, &featinTmpStr);
	ajStrAssignS(&savstr, featinTmpStr);
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
**
** @release 2.0.0
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
		ajFeatWarn("GFF group field '%S' for table '%S'",
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




/* @funcstatic featGff3GroupSet ***********************************************
**
** Sets the group tag for a feature.
**
** @param [u] gf       [AjPFeature]  Feature
** @param [u] table    [AjPFeattable] Feature table
** @param [r] grouptag [const AjPStr]      Group field identifier
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void featGff3GroupSet(AjPFeature gf, AjPFeattable table,
			 const AjPStr grouptag)
{
    AjPStr namstr      = NULL;
    AjPStr grpstr      = NULL;
    ajint grpnum;

    if(!featRegGff3Group)
	featRegGff3Group = ajRegCompC("^(([^.]*)[.])?([0-9]+)");

    if(ajStrGetLen(grouptag) && ajStrMatchCaseS(grouptag, featSaveGroupStr))
    {
	gf->Group = featSaveGroup;
	gf->Exon  = ++featSaveExon;

	return;
    }


    if(ajStrGetLen(grouptag) && ajRegExec(featRegGff3Group, grouptag))
    {
	ajStrAssignS(&featSaveGroupStr, grouptag);
	ajRegSubI(featRegGff3Group, 2, &namstr);
	ajRegSubI(featRegGff3Group, 3, &grpstr);

	/*ajDebug("featGroupSet '%S' name: '%S' group: '%S'\n",
	        grouptag, namstr, grpstr);*/

	if(ajStrToInt(grpstr, &grpnum)) /* true, if the regex worked */
	{
	    gf->Group = grpnum;
	    featSaveGroup = grpnum;
	}
	else
	    gf->Group = ++(table->Groups);

	/* TODO: this validation should be updated and uncommented back
	if(ajStrGetLen(namstr))
	{
	    if(!ajStrMatchCaseS(namstr, table->Seqid))
	    {
		ajDebug("GFF group field '%S' table '%S'\n",
			 grouptag, table->Seqid);
		ajFeatWarn("GFF group field '%S' for table '%S'",
		       grouptag, table->Seqid);
	    }
	}
	*/
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




/* #datasection [none] Input formats ******************************************
**
** Input formats internals
**
** #nam2rule Featinformat Data data input format specific
**
******************************************************************************/




/* #section cast **************************************************************
**
** Values for input formats
**
** #fdata [none]
**
** #nam3rule Find Return index to named format
** #nam3rule Term Test format EDAM term
** #nam3rule Test Test format value
**
** #argrule Find format [const AjPStr] Format name
** #argrule Term term [const AjPStr] Format EDAM term
** #argrule Test format [const AjPStr] Format name
** #argrule Find iformat [ajint*] Index matching format name
**
** #valrule * [AjBool] True if found
**
** #fcategory cast
**
******************************************************************************/




/* @funcstatic feattabinformatFind ********************************************
**
** Looks for the specified format(s) in the internal definitions and
** returns the index.
**
** Sets iformat as the recognised format, and returns ajTrue.
**
** @param [r] format [const AjPStr] Format required.
** @param [w] iformat [ajint*] Index
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool feattabinformatFind(const AjPStr format, ajint* iformat)
{
    AjPStr tmpformat = NULL;
    ajuint i = 0;

    /* ajDebug("feattabinformatFind '%S'\n", format); */
    if(!ajStrGetLen(format))
	return ajFalse;

    ajStrAssignS(&tmpformat, format);
    ajStrFmtLower(&tmpformat);

    for(i=0; featinformatDef[i].Name; i++)
    {
	/* ajDebug("test %d '%s' '%s' '%s'\n",
           i, featinformatDef[i].Name, 
           featinformatDef[i].Obo,
           featinformatDef[i].Desc); */
	if(ajStrMatchC(tmpformat, featinformatDef[i].Name) ||
           ajStrMatchC(format, featinformatDef[i].Obo))
	{
	    *iformat = i;
	    ajStrDel(&tmpformat);
	    /* ajDebug("found '%s' at %d\n", featinformatDef[i].Name, i); */
	    return ajTrue;
	}
    }

    ajErr("Unknown input format '%S'", format);

    ajStrDel(&tmpformat);

    return ajFalse;
}




/* @func ajFeattabinformatTerm ************************************************
**
** Tests whether a feature table data input format term is known
**
** @param [r] term [const AjPStr] Format term EDAM ID
** @return [AjBool] ajTrue if term was accepted
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajFeattabinformatTerm(const AjPStr term)
{
    ajuint i;

    for(i=0; featinformatDef[i].Name; i++)
	if(ajStrMatchC(term, featinformatDef[i].Obo))
	    return ajTrue;

    return ajFalse;
}




/* @func ajFeattabinformatTest ************************************************
**
** Tests whether a named feature table data input format is known
**
** @param [r] format [const AjPStr] Format
** @return [AjBool] ajTrue if format was accepted
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajFeattabinformatTest(const AjPStr format)
{
    ajuint i;

    for(i=0; featinformatDef[i].Name; i++)
    {
	if(ajStrMatchCaseC(format, featinformatDef[i].Name))
	    return ajTrue;
	if(ajStrMatchC(format, featinformatDef[i].Obo))
	    return ajTrue;
    }

    return ajFalse;
}




#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif




#ifdef AJ_COMPILE_DEPRECATED
/* @obsolete ajFeattabInNew
** @rename ajFeattabinNew
*/
__deprecated AjPFeattabIn ajFeattabInNew(void)
{
    return ajFeattabinNew();
}




/* @obsolete ajFeattabInNewSS
** @rename ajFeattabinNewSS
*/
__deprecated AjPFeattabIn ajFeattabInNewSS(const AjPStr fmt, const AjPStr name,
			      const char* type)
{
    return ajFeattabinNewSS(fmt, name, type);
}




/* @obsolete ajFeattabInNewCSF
** @rename ajFeattabinNewCSF
*/
__deprecated AjPFeattabIn ajFeattabInNewCSF(const char* fmt, const AjPStr name,
                                            const char* type, AjPFilebuff buff)
{
    return ajFeattabinNewCSF(fmt, name, type, buff);
}




/* @obsolete ajFeattabInNewSSF
** @rename ajFeattabinNewSSF
*/
__deprecated AjPFeattabIn ajFeattabInNewSSF(const AjPStr fmt, const AjPStr name,
                                            const char* type, AjPFilebuff buff)
{
    return ajFeattabinNewSSF(fmt, name, type, buff);
}




/* @obsolete ajFeattabInDel
** @rename ajFeattabinDel
*/
__deprecated void ajFeattabInDel(AjPFeattabIn* pthis)
{
    ajFeattabinDel(pthis);
    return;
}




/* @obsolete ajFeattabInClear
** @rename ajFeattabinClear
*/
__deprecated void ajFeattabInClear(AjPFeattabIn thys)
{
    ajFeattabinClear(thys);
    return;
}




/* @obsolete ajFeattabInSetTypeC
** @rename ajFeattabinSetTypeC
*/
__deprecated AjBool ajFeattabInSetTypeC(AjPFeattabIn thys, const char* type)
{
   return ajFeattabinSetTypeC(thys, type);
}




/* @obsolete ajFeattabInSetType
** @rename ajFeattabinSetTypeS
*/
__deprecated AjBool ajFeattabInSetType(AjPFeattabIn thys, const AjPStr type)
{
   return ajFeattabinSetTypeC(thys, ajStrGetPtr(type));
}





/* @obsolete ajFeatRead
** @rename ajFeattableNewRead
*/

__deprecated AjPFeattable ajFeatRead(AjPFeattabIn  ftin)
{
    return ajFeattableNewRead(ftin);
}




/* @obsolete ajFeatUfoRead
** @rename ajFeattableNewReadUfo
*/

__deprecated AjPFeattable ajFeatUfoRead(AjPFeattabIn featin,
		     const AjPStr ufo)
{

    return ajFeattableNewReadUfo (featin, ufo);
}
#endif

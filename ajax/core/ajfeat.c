/* @source ajfeat *************************************************************
**
** A genome feature (in AJAX program context) is a description of a
** genomic entity which was determined by some 'source' analysis
** (which may be of 'wet lab' experimental or 'in silico'
** computational nature), has a 'primary' descriptor ('Primary_Tag'),
** may have some 'score' asserting the level of analysis confidence in
** its identity (e.g. log likelihood relative to a null hypothesis or
** other similar entity), has a 'Position' in the genome, and may have
** any arbitrary number of descriptor tags associated with it.
**
** @author Copyright (C) 1999 Richard Bruskiewich
** @version $Revision: 1.190 $
** @modified 2000 Ian Longden.
** @modified 2001 Peter Rice.
** @modified $Date: 2012/07/10 09:27:41 $ by $Author: rice $
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

#include "ajlib.h"

#include "ajfeat.h"
#include "ajfeatread.h"
#include "ajfeatwrite.h"
#include "ajtagval.h"
#include "ajmath.h"
#include "ajlist.h"
#include "ajtable.h"
#include "ajutil.h"
#include "ajreg.h"
#include "ajsys.h"
#include "ajnam.h"
#include "ajseq.h"
#include "ajseqread.h"
#include "ajfiledata.h"
#include "ajfileio.h"

#define FEATDEBUG 0

static AjPRegexp featRegTagReplace = NULL;

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





static AjBool   FeatInitDone         = AJFALSE;
static AjPTable FeatTypeTableDna     = NULL;
static AjPTable FeatTagsTableDna     = NULL;
static AjPTable FeatTypeTableProtein = NULL;
static AjPTable FeatTagsTableProtein = NULL;

static AjPTable FeatTypeTableGff2 = NULL;
static AjPTable FeatTagsTableGff2 = NULL;
static AjPTable FeatTypeTableGff2protein = NULL;
static AjPTable FeatTagsTableGff2protein = NULL;

static AjPTable FeatTypeTableGff3 = NULL;
static AjPTable FeatTagsTableGff3 = NULL;
static AjPTable FeatTypeTableGff3protein = NULL;
static AjPTable FeatTagsTableGff3protein = NULL;

static AjPTable FeatTypeTableEmbl = NULL;
static AjPTable FeatTagsTableEmbl = NULL;

static AjPTable FeatTypeTablePir = NULL;
static AjPTable FeatTagsTablePir = NULL;

static AjPTable FeatTypeTableSwiss = NULL;
static AjPTable FeatTagsTableSwiss = NULL;

static AjPTable FeatTypeTableRefseqp = NULL;
static AjPTable FeatTagsTableRefseqp = NULL;

static AjPTable FeatCategoryTable     = NULL;
static ajint  featWarnCount = 0;

static AjPStr featTypeMiscfeat   = NULL;
static AjPStr featTypeEmpty      = NULL;
static AjPStr featDefSource = NULL;
static AjPStr featFmtTmp = NULL;
static AjPStr featTagTmp = NULL;
static AjPStr featTagTmp2 = NULL;
static AjPStr featValTmp = NULL;
static AjPStr featValTmp2 = NULL;
static AjPStr featTagNote = NULL;

static AjPStrTok featVocabSplit  = NULL;

static AjPStr featTransBegStr = NULL;
static AjPStr featTransEndStr = NULL;
static AjPStr featTransAaStr  = NULL;
static AjPStr featTmpStr      = NULL;
static AjPStr featTempQry = NULL;


static const AjPTable featVocabInit(ajuint ivocab);
static void         featClear(AjPFeature thys );
static AjBool       featFeatType(const AjPStr line, AjPStr* type,
				 AjPStr* sofaid, AjPStr* tag, AjPStr* req);
static AjPFeature   featFeatureNew(void);
static void         featInit(void);
static void         featTableInit(AjPFeattable thys,
				  const AjPStr name);
static AjPFeattable featTableNew(void);
static AjPFeattable featTableNewS(const AjPStr name);
static const AjPStr featTableTypeInternal(const AjPStr type,
					  const AjPTable table);
static const AjPStr featTableTypeInternalLimit(const AjPStr type,
                                               const AjPTable table);
static const AjPStr featTagDna(const AjPStr type, AjBool* known);
static AjBool       featTagGff3PredefinedTag(const AjPStr tag);
static AjBool       featTagName(const AjPStr line, AjPStr* name, AjPStr* type,
				AjPStr* rest);
static const AjPStr featTagProt(const AjPStr type, AjBool* known);
static void         featTagSetDefault(AjPFeature thys,
				      const AjPStr tag, const AjPStr value,
				      AjPStr* pdeftag, AjPStr* pdefval);
static void         featTagSetDefaultDna(const AjPStr tag, const AjPStr value,
					 AjPStr* pdeftag, AjPStr* pdefval);
static void         featTagSetDefaultProt(const AjPStr tag,
					  const AjPStr value,
					  AjPStr* pdeftag, AjPStr* pdefval);
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
static AjPTagval    featTagval(const AjPFeature thys, const AjPStr tag);
static AjPTagval    featTagvalNew(const AjPFeature thys,
				  const AjPStr tag, const AjPStr value);
static AjPTagval    featTagvalNewDna(const AjPStr tag, const AjPStr value);
static AjPTagval    featTagvalNewProt(const AjPStr tag, const AjPStr value);
static const AjPStr featTypeDna(const AjPStr type);
static const AjPStr featTypeDnaLimit(const AjPStr type);
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
static AjBool featTypeTestDnaWild(const AjPStr type, const AjPStr str);
static AjBool featTypeTestProtWild(const AjPStr type, const AjPStr str);
static AjBool featTableTypeTestWild(const AjPStr type,
                                    const AjPTable table,
                                    const AjPStr str);


static void feattableMakeQry(const AjPFeattable thys, AjPStr* qry);



/* @datastatic FeatPVocab *****************************************************
**
** Feature vocabularies
**
** @alias FeatSVocab
** @alias FeatOVocab
**
** @attr Name [const char*] Vocabulary name
** @attr Truename [const char*] Vocabulary true name used for definition files
** @attr Nucleotide [AjBool] True if suitable for nucleotide data
** @attr Protein [AjBool] True if suitable for protein data
** @attr Typetable [AjPTable*] Type table
** @attr Tagstable [AjPTable*] Tags table
** @attr Desc [const char*] Description
** @attr Alias [AjBool] True if name is an alias for an identical definition
** @attr Padding [ajint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct FeatSVocab
{
    const char* Name;
    const char* Truename;
    AjBool Nucleotide;
    AjBool Protein;
    AjPTable* Typetable;
    AjPTable* Tagstable;
    const char* Desc;
    AjBool Alias;
    ajint Padding;
} FeatOVocab;

#define FeatPVocab FeatOVocab*




/* @funclist featVocabDef *****************************************************
**
** Feature vocabularies
**
** Includes pointers to the type and tag tables to be initialised
**
******************************************************************************/

static FeatOVocab featVocabDef[] =
{
    /* Name       Truename     Nucleotide Prot
         Typetable              Tagstable
	 Description         Alias   Padding */
    {"gff3",      "gff3",        AJTRUE,    AJFALSE,
         &FeatTypeTableGff3,    &FeatTagsTableGff3,
	 "GFF version 3",    AJFALSE, 0},
    {"gff3",      "gff3protein", AJFALSE,    AJTRUE,
	 &FeatTypeTableGff3protein,    &FeatTagsTableGff3protein,
	 "GFF version 3",    AJTRUE,  0},
    {"gff",       "gff3",        AJTRUE,    AJFALSE,
         &FeatTypeTableGff3,    &FeatTagsTableGff3,
	 "GFF version 3",    AJFALSE, 0},
    {"gff",       "gff3protein", AJFALSE,    AJTRUE,
         &FeatTypeTableGff3protein,    &FeatTagsTableGff3protein,
	 "GFF version 3",    AJFALSE, 0},
    {"gff2",      "gff2",        AJTRUE,    AJFALSE,
	 &FeatTypeTableGff2,    &FeatTagsTableGff2,
	 "GFF version 2",    AJFALSE, 0},
    {"gff2",      "gff2protein", AJFALSE,    AJTRUE,
	 &FeatTypeTableGff2protein,    &FeatTagsTableGff2protein,
	 "GFF version 2",    AJFALSE, 0},
    {"embl",      "embl", AJTRUE,    AJFALSE,
         &FeatTypeTableEmbl,    &FeatTagsTableEmbl,
	 "embl format",      AJFALSE, 0},
    {"em",        "embl", AJTRUE,    AJFALSE,
	 &FeatTypeTableEmbl,    &FeatTagsTableEmbl,
	 "embl format",      AJTRUE,  0},
    {"genbank",   "embl", AJTRUE,    AJFALSE,
	 &FeatTypeTableEmbl,    &FeatTagsTableEmbl,
	 "genbank format",   AJFALSE, 0},
    {"gb",        "embl", AJTRUE,    AJFALSE,
         &FeatTypeTableEmbl,    &FeatTagsTableEmbl,
	 "genbank format",   AJTRUE,  0},
    {"ddbj",      "embl", AJTRUE,    AJFALSE,
	 &FeatTypeTableEmbl,    &FeatTagsTableEmbl,
	 "ddbj format",      AJFALSE, 0},
    {"refseq",    "embl", AJTRUE,    AJFALSE,
	 &FeatTypeTableEmbl,    &FeatTagsTableEmbl,
	 "genbank format",   AJTRUE,  0},
    {"refseqp",   "refseqp", AJFALSE,   AJTRUE,
	 &FeatTypeTableRefseqp, &FeatTagsTableRefseqp,
	 "genbank format",   AJTRUE,  0},
    {"pir",       "pir", AJFALSE,   AJTRUE,
	 &FeatTypeTablePir,     &FeatTagsTablePir,
	 "PIR format",       AJFALSE, 0},
    {"nbrf",      "pir", AJFALSE,   AJTRUE,
	 &FeatTypeTablePir,     &FeatTagsTablePir,
	 "PIR format",       AJTRUE,  0},
    {"sw",        "swiss", AJFALSE,   AJTRUE,
	 &FeatTypeTableSwiss,   &FeatTagsTableSwiss,
	 "SwissProt format", AJTRUE,  0},
    {"swissprot", "swiss", AJFALSE,   AJTRUE,
	 &FeatTypeTableSwiss,   &FeatTagsTableSwiss,
	 "SwissProt format", AJTRUE,  0},
    {"dasgff",    "gff3", AJTRUE,    AJFALSE,
	 &FeatTypeTableGff3,    &FeatTagsTableGff3,
	 "DAS GFF format",   AJFALSE, 0},
    {"dasgff",    "gff3protein",    AJFALSE, AJTRUE,
	 &FeatTypeTableGff3protein,    &FeatTagsTableGff3protein,
	 "DAS GFF format",   AJFALSE, 0},
    {"debug",     "emboss", AJTRUE,    AJFALSE,
	 &FeatTypeTableDna,    &FeatTagsTableDna,
	 "Debugging trace",  AJFALSE, 0},
    {"debug",     "protein", AJFALSE,    AJTRUE,
         &FeatTypeTableProtein,   &FeatTagsTableProtein,
	 "Debugging trace",  AJFALSE, 0},
    {NULL, NULL, AJFALSE, AJFALSE, NULL, NULL, NULL, AJFALSE, 0}
};

static FeatPVocab featVocab = featVocabDef;






/* @datastatic FeatPCategory **************************************************
**
** Feature output types
**
** @alias FeatSCategory
** @alias FeatOCategory
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
    {"structural", "mapping, sequence and assembly - with no "
                   "biological information",
     "SO:0005850,SO:0000002,SO:0000331,SO:0000730"},
    {"similarity", "areas similar to other sequences",
     ""},
    {"repeat", "areas of repetitive sequence",
     "SO:0000657,SO:0000726,SO:0000005,SO:0000286"},
    {"experimental", "experimental results",
     ""},
    {"immunoglobulin", "immunoglobulin processing regions",
     "SO:0000001_C_region,SO:0000001_J_segment,SO:0000001_N_region,"
     "SO:0000001_S_region,SO:0000001_V_region,SO:0000001_V_segment,"
     "SO:0000458"},
    {NULL, NULL,
     NULL}
};




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
**
** @release 2.0.0
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
    ajuint flags    = 0;
    AjPFeature ret = NULL;

    if(ajStrMatchC(thys->Type, "P"))
        ret = ajFeatNewProtFlags(thys,source,type,Start,End,score,flags);
    else
        ret = ajFeatNewNucFlags(thys,source,type,Start,End,score,strand,frame,
                                0,0,0,NULL, NULL,flags);

    return ret;
}




/* @func ajFeatNewBetween ******************************************************
**
** Simple constructor with location between the start positions and
** the previous base.
**
** User must specify associated 'ajFeattable' to which the new feature
** is automatically added!
**
** @param  [u]   thys    [AjPFeattable] Pointer to the ajFeattable which
**                         will own the feature
** @param  [r]  Start    [ajint]  Start position of the feature
** @return [AjPFeature] newly allocated feature object
**
** @release 6.5.0
** @@
******************************************************************************/

AjPFeature ajFeatNewBetween(AjPFeattable thys,
                            ajint Start)
{
    AjPStr source = NULL;
    float score   = 0.0;
    char strand   = '.';
    ajint frame   = 0;
    ajint flags   = AJFEATFLAG_BETWEEN_SEQ;

    AjPFeature ret = NULL ;

    /*ajDebug("ajFeatNewBetween %d\n", Start);*/

    if(!featTypeMiscfeat)
	featTypeMiscfeat = ajStrNewC("misc_feature");

    ret = ajFeatNewNucFlags(thys,source,featTypeMiscfeat,
                            Start-1,Start,score,strand,frame,
                            0,0,0,NULL,NULL,flags);

    return ret;
}




/* @func ajFeatNewSub *********************************************************
**
** Constructor - must specify associated 'ajFeattable'
**               to which the new feature is automatically added!
**
** @param  [u]   thys    [AjPFeattable] Pointer to the ajFeattable which
**                         will own the feature
** @param  [u] parent   [AjPFeature]   Parent feature
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
**
** @release 6.4.0
** @@
******************************************************************************/

AjPFeature ajFeatNewSub(AjPFeattable thys,
                        AjPFeature parent,
                        const AjPStr source,
                        const AjPStr type,
                        ajint Start, ajint End,
                        float score,
                        char  strand,
                        ajint frame)
{
    ajuint flags    = 0;
    AjPFeature ret = NULL;

    if(ajStrMatchC(thys->Type, "P"))
        ret = ajFeatNewProtFlagsSub(thys,parent,
                                    source,type,Start,End,score,flags);
    else
        ret = ajFeatNewNucFlagsSub(thys,parent,
                                   source,type,Start,End,score,strand,frame,
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
**
** @release 2.1.0
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
	ret = ajFeatNewNucFlags(thys,source,featTypeMiscfeat,
                                End,Start,score,'-',frame,
                                0,0,0,NULL, NULL,flags);
    else
	ret = ajFeatNewNucFlags(thys,source,featTypeMiscfeat,
                                Start,End,score,strand,frame,
                                0,0,0,NULL, NULL,flags);

    return ret;
}




/* @func ajFeatNewIISub *******************************************************
**
** Simple constructor with only start and end positions
**
** User must specify associated 'ajFeattable' to which the new feature
** is automatically added!
**
** @param  [u]   thys    [AjPFeattable] Pointer to the ajFeattable which
**                         will own the feature
** @param  [u] parent   [AjPFeature]   Parent feature
** @param  [r]  Start    [ajint]  Start position of the feature
** @param  [r]  End      [ajint]  End position of the feature
** @return [AjPFeature] newly allocated feature object
** @category new [AjPFeature] Simple constructor with only start and end
**                            positions
**
** @release 6.4.0
** @@
******************************************************************************/

AjPFeature ajFeatNewIISub(AjPFeattable thys,
                        AjPFeature parent,
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
	ret = ajFeatNewNucFlagsSub(thys,parent,
                                   source,featTypeMiscfeat,
                                   End,Start,score,'-',frame,
                                   0,0,0,NULL, NULL,flags);
    else
	ret = ajFeatNewNucFlagsSub(thys,parent,
                                   source,featTypeMiscfeat,
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
**
** @release 2.8.0
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




/* @func ajFeatNewIIRevSub ****************************************************
**
** Simple constructor with only start and end positions, sets feature to be
** on the reverse strand
**
** User must specify associated 'ajFeattable' to which the new feature
** is automatically added!
**
** @param  [u]   thys    [AjPFeattable] Pointer to the ajFeattable which
**                         will own the feature
** @param  [u] parent   [AjPFeature]   Parent feature
** @param  [r]  Start    [ajint]  Start position of the feature
** @param  [r]  End      [ajint]  End position of the feature
** @return [AjPFeature] newly allocated feature object
** @category new [AjPFeature] Simple constructor with only start and end
**                            positions, sets feature to be
**                            on the reverse strand
**
** @release 6.4.0
** @@
******************************************************************************/

AjPFeature ajFeatNewIIRevSub(AjPFeattable thys, AjPFeature parent,
                             ajint Start, ajint End)
{
    AjPFeature ret = NULL ;

    /*ajDebug("ajFeatNewIIRev %d %d\n", Start, End);*/

    if(Start > End)
	ret = ajFeatNewIISub(thys,parent,End,Start);
    else
	ret = ajFeatNewIISub(thys,parent,Start,End);

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
**
** @release 2.0.0
** @@
**
******************************************************************************/

AjPFeature ajFeatNewProt(AjPFeattable thys,
			 const AjPStr source,
			 const AjPStr type,
			 ajint Start, ajint End,
			 float score)
{
    return ajFeatNewProtFlags(thys, source, type, Start, End, score, 0);
}




/* @func ajFeatNewProtSub *****************************************************
**
** Constructor - must specify associated 'ajFeattable'
**               to which the new feature is automatically added!
**
** @param  [u]   thys    [AjPFeattable] Pointer to the ajFeattable which
**                         will own the feature
** @param  [u] parent   [AjPFeature]   Parent feature
** @param  [r] source   [const AjPStr]      Analysis basis for feature
** @param  [r] type     [const AjPStr]      Type of feature (e.g. exon)
** @param  [r]  Start    [ajint]  Start position of the feature
** @param  [r]  End      [ajint]  End position of the feature
** @param  [r] score    [float]      Analysis score for the feature
** @return [AjPFeature] newly allocated feature object
** @category new [AjPFeature] Protein-specific constructor - 
**                            must specify the associated
**                            (non-null) AjPFeattable
**
** @release 6.4.0
** @@
**
******************************************************************************/

AjPFeature ajFeatNewProtSub(AjPFeattable thys, AjPFeature parent,
                            const AjPStr source,
                            const AjPStr type,
                            ajint Start, ajint End,
                            float score)
{
    return ajFeatNewProtFlagsSub(thys, parent,
                                 source, type, Start, End, score, 0);
}




/* @func ajFeatNewProtFlags ***************************************************
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
** @param  [r]  flags    [ajuint]  flags.
** @return [AjPFeature] newly allocated feature object
** @category new [AjPFeature] Protein-specific constructor - 
**                            must specify the associated
**                            (non-null) AjPFeattable
**
** @release 6.4.0
** @@
**
******************************************************************************/

AjPFeature ajFeatNewProtFlags(AjPFeattable thys,
                              const AjPStr source,
                              const AjPStr type,
                              ajint Start, ajint End,
                              float score,
                              ajuint flags)
{
    AjPFeature ret = NULL ;

    if(!featDefSource)
	ajStrAssignS(&featDefSource, ajUtilGetProgram());
    
    ajDebug("\najFeatNewProtFlags '%S' %d .. %d %x\n",
            type, Start, End, flags);
    
    if(!ajStrGetLen(type))
	return NULL;
    
    featInit();
    
    /* Allocate the object... and a new Tags list */
    ret = featFeatureNew() ;
    
    thys->Groups++;
    ret->Group = thys->Groups;
    ret->Exon  = 0;
    
    if(ajStrGetLen(source))
	ajStrAssignS(&ret->Source, source);
    else
	ajStrAssignS(&ret->Source, featDefSource);
    
    ajStrAssignS(&ret->Type, featTypeProt(type));
    ajDebug("ajFeatNewProt feature type '%S' => '%S'\n", type, ret->Type);
    
    ret->Score = score;
    
    ret->Flags = flags;
    
    ret->Strand = '\0' ;
    
    ret->Frame  = 0 ;
    ret->Start  = Start;
    ret->End    = End;
    ret->Start2 = 0;
    ret->End2   = 0;
    
    ret->Protein = ajTrue;
    
    if(!(ret->Flags & AJFEATFLAG_REMOTEID) &&
       !(ret->Flags & AJFEATFLAG_LABEL))
    {
	thys->Len = AJMAX(thys->Len, ret->Start);
	thys->Len = AJMAX(thys->Len, ret->End);
    }
    
    ajFeattableAdd(thys,ret) ;
    
    return ret;
}




/* @func ajFeatNewProtFlagsSub ************************************************
**
** Constructor - must specify associated 'ajFeattable'
**               to which the new feature is automatically added!
**
** @param  [u]   thys    [AjPFeattable] Pointer to the ajFeattable which
**                         will own the feature
** @param  [u] parent   [AjPFeature]   Parent feature
** @param  [r] source   [const AjPStr]      Analysis basis for feature
** @param  [r] type     [const AjPStr]      Type of feature (e.g. exon)
** @param  [r]  Start    [ajint]  Start position of the feature
** @param  [r]  End      [ajint]  End position of the feature
** @param  [r] score    [float]      Analysis score for the feature
** @param  [r]  flags    [ajuint]  flags.
** @return [AjPFeature] newly allocated feature object
** @category new [AjPFeature] Protein-specific constructor - 
**                            must specify the associated
**                            (non-null) AjPFeattable
**
** @release 6.4.0
** @@
**
******************************************************************************/

AjPFeature ajFeatNewProtFlagsSub(AjPFeattable thys,
                                 AjPFeature parent,
                                 const AjPStr source,
                                 const AjPStr type,
                                 ajint Start, ajint End,
                                 float score,
                                 ajuint flags)
{
    AjPFeature ret = NULL ;

    if(!featDefSource)
	ajStrAssignS(&featDefSource, ajUtilGetProgram());
    
    ajDebug("\najFeatNewProtFlagsSub '%S' %d .. %d %x\n",
            type, Start, End, flags);
    
    if(!ajStrGetLen(type))
	return NULL;
    
    featInit();
    
    /* Allocate the object... and a new Tags list */
    ret = featFeatureNew() ;
    
    thys->Groups++;
    ret->Group = thys->Groups;
    ret->Exon  = 0;
    
    if(ajStrGetLen(source))
	ajStrAssignS(&ret->Source, source);
    else
	ajStrAssignS(&ret->Source, featDefSource);
    
    ajStrAssignS(&ret->Type, featTypeProt(type));
    ajDebug("ajFeatNewProt feature type '%S' => '%S'\n", type, ret->Type);
    
    ret->Score = score;
    
    ret->Flags = flags;
    
    ret->Strand = '\0' ;
    
    ret->Frame  = 0 ;
    ret->Start  = Start;
    ret->End    = End;
    ret->Start2 = 0;
    ret->End2   = 0;
    
    ret->Protein = ajTrue;
    
    if(!(ret->Flags & AJFEATFLAG_REMOTEID) &&
       !(ret->Flags & AJFEATFLAG_LABEL))
    {
	thys->Len = AJMAX(thys->Len, ret->Start);
	thys->Len = AJMAX(thys->Len, ret->End);
    }

    if(!parent->Subfeatures)
        parent->Subfeatures = ajListNew();

    ajListPushAppend(parent->Subfeatures, ret);
    
    return ret;
}




/* @func ajFeatCompByStart ****************************************************
**
** Compare two features by their start.
**
** @param [r] a [const void *] feature
** @param [r] b [const void *] another feature
**
** @return [ajint] -1 if a is less than b, 0 if a is equal to b else +1.
**
** @release 6.4.0
** @@
******************************************************************************/

ajint ajFeatCompByStart(const void *a, const void *b)
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




/* @func ajFeatCompByEnd ******************************************************
**
** Compare two features by their end.
**
** @param [r] a [const void *] feature
** @param [r] b [const void *] another feature
**
** @return [ajint] -1 if a is less than b, 0 if a is equal to b else +1.
**
** @release 6.4.0
** @@
******************************************************************************/

ajint ajFeatCompByEnd(const void *a, const void *b)
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




/* @func ajFeatCompByGroup ****************************************************
**
** Compare two features by their group and exon numbers
**
** @param [r] a [const void *] feature
** @param [r] b [const void *] another feature
**
** @return [ajint] -1 if a is less than b, 0 if a is equal to b else +1.
**
** @release 6.4.0
** @@
******************************************************************************/

ajint ajFeatCompByGroup(const void *a, const void *b)
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




/* @func ajFeatCompByType *****************************************************
**
** Compare two features by their type (key).
**
** @param [r] a [const void *] feature
** @param [r] b [const void *] another feature
**
** @return [ajint] -1 if a is less than b, 0 if a is equal to b else +1.
**
** @release 6.4.0
** @@
******************************************************************************/

ajint ajFeatCompByType(const void *a, const void *b)
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




/* @func ajFeatNewNuc *********************************************************
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
** @return [AjPFeature] newly allocated feature object
**
** @release 6.4.0
** @@
******************************************************************************/

AjPFeature ajFeatNewNuc(AjPFeattable thys,
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
                        const AjPStr label)
{
    return ajFeatNewNucFlags(thys, source, type, Start, End,
                             score, strand, frame, exon, Start2, End2,
                             entryid, label, 0);
}




/* @func ajFeatNewNucSub ******************************************************
**
** Constructor for a new feature,
** automatically added to the specified table.
**
** @param  [u]   thys     [AjPFeattable] Pointer to the ajFeattable which
**                         will own the feature
** @param  [u] parent   [AjPFeature]   Parent feature
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
** @return [AjPFeature] newly allocated feature object
**
** @release 6.4.0
** @@
******************************************************************************/

AjPFeature ajFeatNewNucSub(AjPFeattable thys,
                           AjPFeature parent,
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
                           const AjPStr label)
{
    return ajFeatNewNucFlagsSub(thys, parent,
                                source, type, Start, End,
                                score, strand, frame, exon, Start2, End2,
                                entryid, label, 0);
}




/* @func ajFeatNewNucFlags ****************************************************
**
** Constructor for a new feature,
** automatically added to the specified table.
**
** @param  [u]   thys    [AjPFeattable] Pointer to the ajFeattable which
**                         will own the feature
** @param  [r] source    [const AjPStr]      Analysis basis for feature
** @param  [r] type      [const AjPStr]      Type of feature (e.g. exon)
** @param  [r]  Start    [ajint]  Start position of the feature
** @param  [r]  End      [ajint]  End position of the feature
** @param  [r] score     [float]      Analysis score for the feature
** @param  [r]  strand   [char]  Strand of the feature
** @param  [r]  frame    [ajint]   Frame of the feature
** @param  [r]  exon     [ajint]  exon number (0 for default value)
** @param  [r]  Start2   [ajint]  2nd Start position of the feature
** @param  [r]  End2     [ajint]  2nd End position of the feature
** @param  [r] entryid   [const AjPStr] Entry ID for location in
**                                        another entry
** @param  [r] label     [const AjPStr] Label for location (non-numeric)
** @param  [r]  flags    [ajuint]  flags.
** @return [AjPFeature] newly allocated feature object
**
** @release 6.4.0
** @@
******************************************************************************/

AjPFeature ajFeatNewNucFlags(AjPFeattable thys,
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
                             ajuint       flags)
{
    AjPFeature ret          = NULL;
    
    ajDebug("ajFeatNewNucFlags %d %d '%c' type: '%S'\n",
            Start, End, strand, type);

    if(!featDefSource)
	ajStrAssignS(&featDefSource, ajUtilGetProgram());
    
    if(!ajStrGetLen(type))
	return NULL;
    
    featInit();
    
    /* Allocate the object... */
    
    ret = featFeatureNew();
    
    thys->Groups++;
    ret->Group = thys->Groups;
    ret->Exon  = exon;
    
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
	if(!(ret->Flags & AJFEATFLAG_REMOTEID) &&
	   !(ret->Flags & AJFEATFLAG_LABEL))
	{
	    thys->Len = AJMAX(thys->Len, ret->Start);
	    thys->Len = AJMAX(thys->Len, ret->End);
	}
    }
    
    if(ajStrGetLen(label))
    {
	ajStrAssignS(&ret->Label, label);
	ajFeatWarn("%S: Feature label '%S' used",
	       thys->Seqid, label);
    }
    
    ajFeattableAdd(thys,ret) ;
    
    return ret ;
}




/* @func ajFeatNewNucFlagsSub *************************************************
**
** Constructor for a new feature,
** automatically added to the specified table.
**
** @param  [u]   thys    [AjPFeattable] Pointer to the ajFeattable which
**                         will own the feature
** @param  [u] parent   [AjPFeature]   Parent feature
** @param  [r] source    [const AjPStr]      Analysis basis for feature
** @param  [r] type      [const AjPStr]      Type of feature (e.g. exon)
** @param  [r]  Start    [ajint]  Start position of the feature
** @param  [r]  End      [ajint]  End position of the feature
** @param  [r] score     [float]      Analysis score for the feature
** @param  [r]  strand   [char]  Strand of the feature
** @param  [r]  frame    [ajint]   Frame of the feature
** @param  [r]  exon     [ajint]  exon number (0 for default value)
** @param  [r]  Start2   [ajint]  2nd Start position of the feature
** @param  [r]  End2     [ajint]  2nd End position of the feature
** @param  [r] entryid   [const AjPStr] Entry ID for location in
**                                        another entry
** @param  [r] label     [const AjPStr] Label for location (non-numeric)
** @param  [r]  flags    [ajuint]  flags.
** @return [AjPFeature] newly allocated feature object
**
** @release 6.4.0
** @@
******************************************************************************/

AjPFeature ajFeatNewNucFlagsSub(AjPFeattable thys,
                                AjPFeature parent,
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
                                ajuint       flags)
{
    AjPFeature ret          = NULL;
    
    ajDebug("ajFeatNewNucFlagsSub %d %d '%c' type: '%S'\n",
            Start, End, strand, type);

    if(!featDefSource)
	ajStrAssignS(&featDefSource, ajUtilGetProgram());
    
    if(!ajStrGetLen(type))
	return NULL;
    
    featInit();
    
    /* Allocate the object... */
    
    ret = featFeatureNew();
    
    thys->Groups++;
    ret->Group = thys->Groups;
    ret->Exon  = exon;
    
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
	if(!(ret->Flags & AJFEATFLAG_REMOTEID) &&
	   !(ret->Flags & AJFEATFLAG_LABEL))
	{
	    thys->Len = AJMAX(thys->Len, ret->Start);
	    thys->Len = AJMAX(thys->Len, ret->End);
	}
    }
    
    if(ajStrGetLen(label))
    {
	ajStrAssignS(&ret->Label, label);
	ajFeatWarn("%S: Feature label '%S' used",
	       thys->Seqid, label);
    }
    
    if(!parent->Subfeatures)
        parent->Subfeatures = ajListNew();

    ajListPushAppend(parent->Subfeatures, ret);
    
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




/* @func ajFeattableDel *******************************************************
**
** Destructor for ajFeattable objects.
** If the given object (pointer) is NULL, or a NULL pointer, simply returns.
**
** @param  [d] pthis [AjPFeattable*] Pointer to the object to be deleted.
**         The pointer is always deleted.
** @return [void]
** @category delete [AjPFeattable] Default destructor
**
** @release 2.1.0
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

    ajStrDel(&thys->Seqid);
    ajStrDel(&thys->Type);
    ajStrDel(&thys->Db);
    ajStrDel(&thys->Setdb);
    ajStrDel(&thys->Full);
    ajStrDel(&thys->Qry);
    ajStrDel(&thys->Formatstr);
    ajStrDel(&thys->Filename);
    ajStrDel(&thys->TextPtr);

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
**
** @release 1.0.0
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
**
** @release 1.0.0
** @@
******************************************************************************/

static void featClear(AjPFeature thys)
{
    AjIList       iter = NULL;
    AjPTagval     item = NULL;
    AjPFeature subfeat = NULL;

    if(!thys)
	return;

    /* We need to delete the associated Tag data structures too!!!*/

    if(thys->Subfeatures)
    {
        iter = ajListIterNew(thys->Subfeatures);
	while(!ajListIterDone(iter))
	{
	    subfeat = (AjPFeature)ajListIterGet(iter);
            ajFeatDel(&subfeat);
        }
	ajListIterDel(&iter);
    }
    
    ajListFree(&(thys->Subfeatures));

    if(thys->Tags)
    {
	iter = ajListIterNew(thys->Tags);
	while(!ajListIterDone(iter))
	{
	    item = (AjPTagval)ajListIterGet(iter);
	    /* assuming a simple block memory free for now...*/
	    ajTagvalDel(&item);
	    ajListIterRemove(iter);
	}
	ajListIterDel(&iter);
    }

    ajListFree(&(thys->Tags));

    if(thys->GffTags)
    {
	iter = ajListIterNew(thys->GffTags);
	while(!ajListIterDone(iter))
	{
	    item = (AjPTagval)ajListIterGet(iter);
	    /* assuming a simple block memory free for now...*/
	    ajTagvalDel(&item);
	    ajListIterRemove(iter);
	}
	ajListIterDel(&iter);
    }

    ajListFree(&(thys->GffTags));

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




/* @func ajFeatSortByType *****************************************************
**
** Sort Feature table by Type.
**
** @param [u] Feattab [AjPFeattable] Feature table to be sorted.
**
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajFeatSortByType(AjPFeattable Feattab)
{
    ajListSort(Feattab->Features, &ajFeatCompByType);

    return;
}




/* @func ajFeatSortByStart ****************************************************
**
** Sort Feature table by Start position.
**
** @param [u] Feattab [AjPFeattable] Feature table to be sorted.
**
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajFeatSortByStart(AjPFeattable Feattab)
{
    ajListSort(Feattab->Features, &ajFeatCompByStart);
}




/* @func ajFeatSortByEnd ******************************************************
**
** Sort Feature table by End position.
**
** @param [u] Feattab [AjPFeattable] Feature table to be sorted.
**
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajFeatSortByEnd(AjPFeattable Feattab)
{
    ajListSort(Feattab->Features, &ajFeatCompByEnd);

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
**
** @release 2.1.0
** @@
******************************************************************************/

void ajFeattableAdd(AjPFeattable thys, AjPFeature feature)
{
    if(!(feature->Flags & AJFEATFLAG_REMOTEID) &&
       !(feature->Flags & AJFEATFLAG_LABEL))
    {
	thys->Len = AJMAX(thys->Len, feature->Start);
	thys->Len = AJMAX(thys->Len, feature->End);
    }
    ajListPushAppend(thys->Features, feature);

/*
    if(feature->Type)
	ajDebug("ajFeattableAdd list size %Lu '%S' %d %d\n",
		ajListGetLength(thys->Features), feature->Type,
		feature->Start, feature->End);
    else
	ajDebug("ajFeattableAdd list size %Lu '%S' %d %d\n",
		ajListGetLength(thys->Features), NULL,
		feature->Start, feature->End);
*/

    return;
}




/* @func ajFeattableMerge *****************************************************
**
** Merges a feature table into an existing feature table.
**
** @param [u] thys     [AjPFeattable]        Feature table to be appended to
** @param [r] srctable [const AjPFeattable]  Additional feature table
** @return [ajuint] Size of the new feature table.
**
** @release 6.5.0
** @@
******************************************************************************/

ajuint ajFeattableMerge(AjPFeattable thys, const AjPFeattable srctable)
{
    AjIList iter;
    AjPFeature featsrc;
    AjPFeature feat = NULL;

    if(!thys)
	return 0;

    if(!srctable)
	return 0;

    iter = ajListIterNewread(srctable->Features);

    while(!ajListIterDone(iter))
    {
	featsrc = ajListIterGet(iter);
	feat = ajFeatNewFeat(featsrc);
	ajFeattableAdd(thys, feat);
    }

    ajListIterDel(&iter);

    return (ajuint) ajListGetLength(thys->Features);
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
**
** @release 2.1.0
** @@
**
******************************************************************************/

static void featTableInit(AjPFeattable thys,
			  const AjPStr name)
{
    /*ajDebug("featTableInit Entering...\n");*/

    /*ajDebug("featTableInit initialising seqid: '%S'\n", name);*/
    ajStrAssignS(&thys->Seqid,name) ;
    thys->Format = 0;

    return;
}




/* @func ajFeattableClear *****************************************************
**
** Clears a feature table of all features
**
** @param [u] thys [AjPFeattable] Feature table
** @return [void]
**
** @release 2.1.0
** @@
******************************************************************************/

void ajFeattableClear(AjPFeattable thys)
{
    AjIList iter       = NULL ;
    AjPFeature feature = NULL ;

    if(!thys)
	return ;

    if(MAJSTRGETLEN(thys->Seqid))
        ajStrSetClear(&thys->Seqid);
    if(MAJSTRGETLEN(thys->Type))
        ajStrSetClear(&thys->Type);
    if(MAJSTRGETLEN(thys->Db))
        ajStrSetClear(&thys->Db);
    if(MAJSTRGETLEN(thys->Setdb))
        ajStrSetClear(&thys->Setdb);
    if(MAJSTRGETLEN(thys->Full))
        ajStrSetClear(&thys->Full);
    if(MAJSTRGETLEN(thys->Qry))
        ajStrSetClear(&thys->Qry);
    if(MAJSTRGETLEN(thys->Formatstr))
        ajStrSetClear(&thys->Formatstr);
    if(MAJSTRGETLEN(thys->Filename))
        ajStrSetClear(&thys->Filename);
    if(MAJSTRGETLEN(thys->TextPtr))
        ajStrSetClear(&thys->TextPtr);

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
**
** @release 3.0.0
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




/* @func ajFeatTypeGetCategory ************************************************
**
** returns the category name for a feature type
**
** Used to identify category for DASGFF output
**
** @param [r] type [const AjPStr] Feature type
** @return [const AjPStr] Feature category
**
**
** @release 6.1.0
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
        FeatCategoryTable = ajTablestrNewCase(200);

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

            ajStrDel(&token);
            ajStrDel(&name);
            ajStrDel(&types);
            ajStrTokenDel(&catsplit);
        }
        
    }
    
    ret = ajTablestrFetchS(FeatCategoryTable,type);
    if(!ret)
    {
        token = ajStrNewC("");
        ret = ajTablestrFetchS(FeatCategoryTable,token);
        ajStrDel(&token);
    }

    return ret;
}




/* @func ajFeattableGetEntry **************************************************
**
** Return the full text
**
** @param [r] thys [const AjPFeattable] Feature table object
**
** @return [const AjPStr] Returned full text
**
**
** @release 6.4.0
******************************************************************************/

const AjPStr ajFeattableGetEntry(const AjPFeattable thys)
{
    if(thys->TextPtr)
        return thys->TextPtr;

    return ajStrConstEmpty();
}




/* @func ajFeattableGetQryC ***************************************************
**
** Returns the query string of a feature table data object.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] ftable [const AjPFeattable] Feature table data object.
** @return [const char*] Query as a character string.
**
** @release 6.4.0
** @@
******************************************************************************/

const char* ajFeattableGetQryC(const AjPFeattable ftable)
{
    return MAJSTRGETPTR(ajFeattableGetQryS(ftable));
}




/* @func ajFeattableGetQryS ***************************************************
**
** Returns the query string of a feature table data object.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] ftable [const AjPFeattable] Feature table data object.
** @return [const AjPStr] Query as a string.
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPStr ajFeattableGetQryS(const AjPFeattable ftable)
{
    ajDebug("ajFeattableGetQryS '%S'\n", ftable->Qry);

    if(ajStrGetLen(ftable->Qry))
	return ftable->Qry;

    feattableMakeQry(ftable, &featTempQry);

    return featTempQry;
}




/* @func ajFeattableGetTypeC **************************************************
**
** Returns the name of a feature table object. This is a copy of the
** pointer to the name, and is still owned by the feature table
** and is not to be destroyed.
**
** @param [r] thys [const AjPFeattable] Feature table
** @return [const char*] Feature table type.
**
** @release 4.0.0
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
**
** @release 4.0.0
** @@
******************************************************************************/

const AjPStr ajFeattableGetTypeS(const AjPFeattable thys)
{
    return thys->Type;
}




/* @func ajFeattableGetXrefs **************************************************
**
** Returns all cross-references from a feature table
**
** @param [r] thys [const AjPFeattable] Feature table
** @param [u] Pxreflist [AjPList*] List of sequence cross-reference objects
** @param [w] Ptaxid [ajuint*] Taxon ID
** @return [AjBool] True on success
**
** @release 6.1.0
** @@
******************************************************************************/

AjBool ajFeattableGetXrefs(const AjPFeattable thys, AjPList *Pxreflist,
                           ajuint *Ptaxid)
{
    AjIList iterfeat     = NULL;
    AjIList itertags     = NULL;
    AjPTagval item = NULL;
    AjPSeqXref  xref = NULL;
    ajlong ipos;
    ajuint inum = 0;
    AjPFeature feat  = NULL;
    AjPList xreflist;
    AjPStrTok handle = NULL;
    AjPStr tmpstr = NULL;
    const AjPStr tagval = NULL;

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
                    item = (AjPTagval)ajListIterGet(itertags);

                    if(ajStrMatchCaseC(MAJTAGVALGETTAG(item), "db_xref"))
                    {
                        tagval = ajTagvalGetValue(item);
                        ipos = ajStrFindAnyK(tagval, ':');
                        if(ipos > 0) 
                        {
                            inum++;
                            xref = ajSeqxrefNew();
                            ajStrAssignSubS(&xref->Db,
                                            tagval, 0, ipos-1);
                            ajStrAssignSubS(&xref->Id,
                                            tagval, ipos+1, -1);
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

                ajListIterDel(&itertags);
            }

            if(feat->GffTags)
            {
                itertags = ajListIterNewread(feat->GffTags);

                while(!ajListIterDone(itertags))
                {
                    item = (AjPTagval)ajListIterGet(itertags);

                    if(ajStrMatchCaseC(MAJTAGVALGETTAG(item), "Dbxref"))
                    {
                        tagval = ajTagvalGetValue(item);
                        handle = ajStrTokenNewC(tagval, ",");
                        while(ajStrTokenNextParse(&handle, &tmpstr))
                        {
                            ipos = ajStrFindAnyK(tmpstr, ':');
                            if(ipos > 0) 
                            {
                                inum++;
                                xref = ajSeqxrefNew();
                                ajStrAssignSubS(&xref->Db,
                                                tmpstr, 0, ipos-1);
                                ajStrAssignSubS(&xref->Id,
                                                tmpstr, ipos+1, -1);
                                ajListPushAppend(xreflist, xref);
                                xref->Start = ajFeatGetStart(feat);
                                xref->End   = ajFeatGetEnd(feat);
                                xref->Type = XREF_DBXREF;
                                if(!*Ptaxid &&
                                   ajStrMatchCaseC(xref->Db, "taxon"))
                                {
                                    if(!ajStrToUint(xref->Id, Ptaxid))
                                        *Ptaxid = 0;
                                }
                                xref = NULL;
                            }
                        }

                        ajStrTokenDel(&handle);
                        ajStrDel(&tmpstr);
                    }
                }

                ajListIterDel(&itertags);
            }
        }
     }

    ajListIterDel(&iterfeat);

    if(!inum)
        return ajFalse;

    return ajTrue;
}




/* @funcstatic feattableMakeQry ***********************************************
**
** Sets the query for a feature table data object.
**
** @param [r] thys [const AjPFeattable] Feature table data object
** @param [w] qry [AjPStr*] Query string in full
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void feattableMakeQry(const AjPFeattable thys, AjPStr* qry)
{
    ajDebug("feattableMakeQry (Seqid <%S> Formatstr <%S> Db <%S> "
	    "Filename <%S>)\n",
	    thys->Seqid, thys->Formatstr, thys->Db,
	    thys->Filename);

    /* ajResourceTrace(thys); */

    if(ajStrGetLen(thys->Db))
	ajFmtPrintS(qry, "%S-id:%S", thys->Db, thys->Seqid);
    else
    {
	ajFmtPrintS(qry, "%S::%S:%S", thys->Formatstr,
                    thys->Filename,thys->Seqid);
    }

    ajDebug("      result: <%S>\n",
	    *qry);

    return;
}




/* @func ajFeatframeGetFrame **************************************************
**
** Converts a frame number in the range 0 to 3 to a GFF frame character
** or '.' as the general default
**
** @param [r] frame [ajint] Feature frame number
** @return [char] character for this frame in GFF
**
** @release 6.4.0
******************************************************************************/

char ajFeatframeGetFrame(ajint frame)
{
    static char framestr[] = ".012";

    if(frame < 0)
	return '.';

    if(frame > 3)
	return '.';

    return framestr[frame];
}




/* @func ajFeatframeGetFrameNuc ***********************************************
**
** Converts a frame number in the range 0 to 3 to a GFF frame character
** or '0' for the nucleotide default
**
** @param [r] frame [ajint] Feature frame number
** @return [char] character for this frame in GFF
**
** @release 6.4.0
******************************************************************************/

char ajFeatframeGetFrameNuc(ajint frame)
{
    static char framestr[] = ".012";

    if(frame <= 0)
	return '0';

    if(frame > 3)
	return '0';

    return framestr[frame];
}




/* @func ajFeatstrandGetStrand ************************************************
**
** Converts a strand number to a GFF strand character. NULL characters
** are converted to '+' All other values are simply cast to character.
**
** @param [r] strand [ajint] Strand
** @return [char] GFF character for this strand.
**
** @release 6.4.0
** @@
******************************************************************************/

char ajFeatstrandGetStrand(ajint strand)
{
    if(ajSysCastItoc(strand) != '-')
	return '+';

    return '-';
}




/* @func ajFeattableIsCircular ************************************************
**
** Returns true if a feature table is circular
**
** @param [r] thys [const AjPFeattable] Feature table
** @return [AjBool] ajTrue for a circular feature table
**
** @release 2.5.0
** @@
******************************************************************************/

AjBool ajFeattableIsCircular(const AjPFeattable thys)
{
    return thys->Circular;
}




/* @func ajFeattableIsNuc *****************************************************
**
** Returns ajTrue if a feature table is nucleotide
**
** @param [r] thys [const AjPFeattable] Feature table
** @return [AjBool] ajTrue for a protein feature table
**
** @release 2.5.0
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
**
** @release 2.5.0
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




/* @func ajFeattableGetBegin **************************************************
**
** Returns the feature table start position, or 1 if no start has been set.
**
** @param [r] thys [const AjPFeattable] feature table object
** @return [ajint] Start position.
**
** @release 6.2.0
** @@
******************************************************************************/

ajint ajFeattableGetBegin(const AjPFeattable thys)
{
    if(!thys->Start)
	return 1;

    return ajFeattablePos(thys, thys->Start);
}




/* @func ajFeattableGetEnd ****************************************************
**
** Returns the features table end position, or the feature table length if
** no end has been set.
**
** @param [r] thys [const AjPFeattable] feature table object
** @return [ajint] End position.
**
** @release 6.2.0
** @@
******************************************************************************/

ajint ajFeattableGetEnd(const AjPFeattable thys)
{
    if(!thys->End)
	return (ajFeattableGetLen(thys));

    return ajFeattablePosI(thys, ajFeattableGetBegin(thys), thys->End);
}




/* @func ajFeattableGetLen ****************************************************
**
** Returns the sequence length of a feature table
**
** @param [r] thys [const AjPFeattable] Feature table
** @return [ajint] Length in bases or residues
**
** @release 6.2.0
** @@
******************************************************************************/

ajint ajFeattableGetLen(const AjPFeattable thys)
{
    if(!thys)
	return 0;

    return (thys->Len);
}




/* @func ajFeattableGetName ***************************************************
**
** Returns the name of a feature table object. This is a copy of the
** pointer to the name, and is still owned by the feature table
** and is not to be destroyed.
**
** @param [r] thys [const AjPFeattable] Feature table
** @return [const AjPStr] Feature table name.
**
** @release 2.1.0
** @@
******************************************************************************/

const AjPStr ajFeattableGetName(const AjPFeattable thys)
{
    if(!thys)
        return NULL;

    return thys->Seqid;
}




/* @func ajFeattableGetSize ***************************************************
**
** Returns the size of a feature table object.
**
** @param [r] thys [const AjPFeattable] Feature table
** @return [ajuint] Feature table size.
**
** @release 6.1.0
** @@
******************************************************************************/

ajuint ajFeattableGetSize(const AjPFeattable thys)
{
    if(!thys)
        return 0;

    return (ajuint) ajListGetLength(thys->Features);
}




/* @func ajFeattableGetScorerange *********************************************
**
** Returns the range of scores for feature table object.
**
** @param [r] thys [const AjPFeattable] Feature table
** @param [w] minscore [float*] Minimum score
** @param [w] maxscore [float*] Maximum score
** @return [AjBool] True if scores were found
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ajFeattableGetScorerange(const AjPFeattable thys,
                                float *minscore, float *maxscore)
{
    AjBool ret = ajFalse;
    float curmin = 0.0;
    float curmax = 0.0;
    AjIList iter;
    AjPFeature gf = NULL;

    if(!thys)
        return 0;

    *minscore = 0.0;
    *maxscore = 0.0;

    iter = ajListIterNewread(thys->Features);
    while(!ajListIterDone(iter))
    {
        gf = ajListIterGet(iter);

        curmin = 0.0;
        curmax = 0.0;

        if(ajFeatGetScorerange(gf, &curmin, &curmax))
        {
            if(curmin < *minscore)
            {
                ret = ajTrue;
                *minscore = curmin;
            }
            if(curmax > *maxscore)
            {
                ret = ajTrue;
                *maxscore = curmax;
            }
        }
    }
    
    ajListIterDel(&iter);

    return ret;    
}




/* @func ajFeatGetScorerange **************************************************
**
** Returns the range of scores for a feature object.
**
** @param [r] thys [const AjPFeature] Feature
** @param [w] minscore [float*] Minimum score
** @param [w] maxscore [float*] Maximum score
** @return [AjBool] True if scores were found
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ajFeatGetScorerange(const AjPFeature thys,
                           float *minscore, float *maxscore)
{
    AjBool ret = ajFalse;
    float curmin = 0.0;
    float curmax = 0.0;
    AjIList iter;
    AjPFeature gf = NULL;

    if(!thys)
        return 0;

    *minscore = 0.0;
    *maxscore = 0.0;

    if(thys->Score < *minscore)
    {
        ret = ajTrue;
        *minscore = curmin;
    }
    if(thys->Score > *maxscore)
    {
        ret = ajTrue;
        *maxscore = curmax;
    }

    iter = ajListIterNewread(thys->Subfeatures);
    while(!ajListIterDone(iter))
    {
        gf = ajListIterGet(iter);

        curmin = 0.0;
        curmax = 0.0;

        if(ajFeatGetScorerange(gf, &curmin, &curmax))
        {
            if(curmin < *minscore)
            {
                ret = ajTrue;
                *minscore = curmin;
            }
            if(curmax > *maxscore)
            {
                ret = ajTrue;
                *maxscore = curmax;
            }
        }
    }
    
    ajListIterDel(&iter);

    return ret;    
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
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ajFeatGetFlags(const AjPFeature thys,  AjPStr* Pflagstr)
{
    ajStrAssignC(Pflagstr, "");

    if(thys->Flags & AJFEATFLAG_START_BEFORE_SEQ)
        ajStrAppendC(Pflagstr, "<start ");
    if(thys->Flags & AJFEATFLAG_END_AFTER_SEQ)
        ajStrAppendC(Pflagstr, ">end ");
    if(thys->Flags & AJFEATFLAG_BETWEEN_SEQ)
        ajStrAppendC(Pflagstr, "x^y ");
    if(thys->Flags & AJFEATFLAG_START_TWO)
        ajStrAppendC(Pflagstr, "startrange ");
    if(thys->Flags & AJFEATFLAG_END_TWO)
        ajStrAppendC(Pflagstr, "endrange ");
    if(thys->Flags & AJFEATFLAG_POINT)
        ajStrAppendC(Pflagstr, "single-base ");
    if(thys->Flags & AJFEATFLAG_COMPLEMENT_MAIN)
        ajStrAppendC(Pflagstr, "complement(join) ");
    if(thys->Flags & AJFEATFLAG_MULTIPLE)
        ajStrAppendC(Pflagstr, "multiple ");
    if(thys->Flags & AJFEATFLAG_GROUP)
        ajStrAppendC(Pflagstr, "group ");
    if(thys->Flags & AJFEATFLAG_ORDER)
        ajStrAppendC(Pflagstr, "order ");
    if(thys->Flags & AJFEATFLAG_ONEOF)
        ajStrAppendC(Pflagstr, "oneof ");
    if(thys->Flags & AJFEATFLAG_REMOTEID)
        ajStrAppendC(Pflagstr, "remoteid ");
    if(thys->Flags & AJFEATFLAG_LABEL)
        ajStrAppendC(Pflagstr, "LABEL ");
    if(thys->Flags & AJFEATFLAG_START_UNSURE)
        ajStrAppendC(Pflagstr, "start-unsure ");
    if(thys->Flags & AJFEATFLAG_END_UNSURE)
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
**
** @release 6.1.0
** @@
******************************************************************************/

AjBool ajFeatGetSeq(const AjPFeature thys,
                    const AjPSeq seq, AjPStr* Pseqstr)
{
    AjPSeq remoteseq = NULL;
/*    AjBool isjoin = ajFalse; */
    AjPStr tmpseq = NULL;
    AjBool compjoin = ajFalse;

    ajStrSetClear(Pseqstr);

/*    isjoin = ajFeatIsMultiple(thys); */

    ajDebug("ajFeatGetSeq usa:%S\n",
            ajSeqGetUsaS(seq));

    if(thys->Flags & AJFEATFLAG_BETWEEN_SEQ)
        return ajTrue;

    ajFeatTrace(thys);

    if(thys->Flags & AJFEATFLAG_REMOTEID)
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




/* @func ajFeatGetSeqJoin *****************************************************
**
** Returns the sequence matching a feature. For multiple location features
** (joins in an EMBL/GenBank feature table) the full feature table is used
** to find all exons.
**
** The database name is used to retrieve sequences from other entries
**
** @param [r] thys [const AjPFeature] Feature
** @param [r] seq [const AjPSeq] Sequence for the current feature table
** @param [u] Pseqstr [AjPStr*] Sequence for this feature
** @return [AjBool] True on success
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ajFeatGetSeqJoin(const AjPFeature thys,
                        const AjPSeq seq, AjPStr* Pseqstr)
{
    const AjPFeature gf;
    AjIList iter = NULL;
    AjPSeq remoteseq = NULL;
/*    AjBool isjoin = ajFalse; */
    AjPStr tmpseq = NULL;
    AjBool compjoin = ajFalse;
    AjPStr flags = NULL;
    ajuint count=0;

    ajDebug("ajFeatGetSeqJoin nfeat:%Lu usa:%S %u..%u %S\n",
            ajListGetLength(thys->Subfeatures), ajSeqGetUsaS(seq),
            thys->Start, thys->End, thys->Type);

    if(!ajListGetLength(thys->Subfeatures))
        return ajFeatGetSeq(thys, seq, Pseqstr);

    ajStrSetClear(Pseqstr);

/*    isjoin = ajFeatIsMultiple(thys); */
    if(thys->Flags & AJFEATFLAG_COMPLEMENT_MAIN)
        compjoin = ajTrue;


    iter = ajListIterNewread(thys->Subfeatures);

    while(!ajListIterDone(iter))
    {
        count++;
        gf = (const AjPFeature) ajListIterGet(iter);

        ajFeatGetFlags(gf, &flags);
        ajFeatTrace(gf);

        if(gf->Flags & AJFEATFLAG_BETWEEN_SEQ)
            continue;

        if(gf->Flags & AJFEATFLAG_REMOTEID)
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

    if(compjoin)
        ajSeqstrReverse(Pseqstr);

    ajListIterDel(&iter);
    ajSeqDel(&remoteseq);
    ajStrDel(&flags);

    return ajTrue;
}




/* @func ajFeatGetRemoteseq ***************************************************
**
** Returns a sequence entry from a feature location which points to
** another entry
**
** @param [r] thys [const AjPFeature] Feature
** @param [r] usa [const AjPStr] usa of query
** @param [u] seq [AjPSeq] Sequence object for results
** @return [AjBool] True on success
**
** @release 6.1.0
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




/* @func ajFeatGetSubtype *****************************************************
**
** Returns the feature type of the first subfeature
**
** @param [r] thys [const AjPFeature] Feature
** @return [const AjPStr] Subfeature type
**
** @release 6.5.0
** @@
******************************************************************************/

const AjPStr ajFeatGetSubtype(const AjPFeature thys)
{
    AjPFeature subft= NULL;

    if(!ajListGetLength(thys->Subfeatures))
        return NULL;

    ajListPeek(thys->Subfeatures, (void**) &subft);

    return subft->Type;
}




/* @func ajFeatGetXrefs *******************************************************
**
** Returns all cross-references from a feature
**
** @param [r] thys [const AjPFeature] Feature
** @param [u] Pxreflist [AjPList*] List of sequence cross-reference objects
** @return [AjBool] True on success
**
** @release 6.1.0
** @@
******************************************************************************/

AjBool ajFeatGetXrefs(const AjPFeature thys, AjPList *Pxreflist)
{
    AjIList iter     = NULL;
    AjPTagval item = NULL;
    AjPSeqXref  xref = NULL;
    ajlong ipos;
    ajuint inum = 0;
    AjPList xreflist;
    AjPStrTok handle = NULL;
    AjPStr tmpstr = NULL;
    const AjPStr tagval = NULL;

    if(!*Pxreflist)
        *Pxreflist = ajListNew();
    xreflist = *Pxreflist;

    if(thys->Tags)
    {
	iter = ajListIterNewread(thys->Tags);

	while(!ajListIterDone(iter))
	{
	    item = (AjPTagval)ajListIterGet(iter);

	    if(ajStrMatchCaseC(MAJTAGVALGETTAG(item), "db_xref"))
	    {
                tagval = ajTagvalGetValue(item);
                ipos = ajStrFindAnyK(tagval, ':');

                if(ipos > 0) 
                {
                    inum++;
                    xref = ajSeqxrefNew();
                    ajStrAssignSubS(&xref->Db, tagval, 0, ipos-1);
                    ajStrAssignSubS(&xref->Id, tagval, ipos+1, -1);
                    xref->Start = ajFeatGetStart(thys)-1;
                    xref->End   = ajFeatGetEnd(thys)-1;
                    ajListPushAppend(xreflist, xref);
                    xref->Type = XREF_DBXREF;
                    xref = NULL;
                }
            }
	}

        ajListIterDel(&iter);
    }

    if(thys->GffTags)
    {
        iter = ajListIterNewread(thys->GffTags);

        while(!ajListIterDone(iter))
        {
            item = (AjPTagval)ajListIterGet(iter);

            if(ajStrMatchCaseC(MAJTAGVALGETTAG(item), "Dbxref"))
            {
                tagval = ajTagvalGetValue(item);
                handle = ajStrTokenNewC(tagval, ",");

                while(ajStrTokenNextParse(&handle, &tmpstr))
                {
                    ipos = ajStrFindAnyK(tmpstr, ':');

                    if(ipos > 0) 
                    {
                        inum++;
                        xref = ajSeqxrefNew();
                        ajStrAssignSubS(&xref->Db,
                                        tmpstr, 0, ipos-1);
                        ajStrAssignSubS(&xref->Id,
                                        tmpstr, ipos+1, -1);
                        ajListPushAppend(xreflist, xref);
                        xref->Start = ajFeatGetStart(thys);
                        xref->End   = ajFeatGetEnd(thys);
                        xref->Type = XREF_DBXREF;
                    }
                }

                ajStrTokenDel(&handle);
                ajStrDel(&tmpstr);
            }
        }

        ajListIterDel(&iter);
    }

    if(!inum)
        return ajFalse;

    return ajTrue;
}




/* @func ajFeatLocMark ********************************************************
**
** Returns a sequence entry converted to lower case where a feature
** location matches.
**
** @param [r] thys [const AjPFeature] Feature
** @param [r] table [const AjPFeattable] Feature table
** @param [u] Pseqstr [AjPStr*] Sequence to be marked in lower case
** @return [AjBool] true on success
**
** @release 4.1.0
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
            if(gf->Flags & AJFEATFLAG_BETWEEN_SEQ)
                continue;

            if(gf->Flags & AJFEATFLAG_REMOTEID)
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
**
** @release 4.0.0
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
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajFeatGetNoteCI(const AjPFeature thys, const char* name, ajint count,
		      AjPStr* val)
{
    AjIList iter     = NULL;
    AjPTagval item = NULL;
    ajint icount     = 0;
    ajuint ilen = strlen(name);
    const AjPStr tagval = NULL;

    /*ajDebug("ajFeatGetNoteCI '%s'\n", name);*/

    if(thys->Tags)
    {
	iter = ajListIterNewread(thys->Tags);

	while(!ajListIterDone(iter))
	{
	    item = (AjPTagval)ajListIterGet(iter);
	    if(ajFeattagIsNote(MAJTAGVALGETTAG(item)))
	    {
                tagval = ajTagvalGetValue(item);
		if(ajStrGetCharFirst(tagval) == '*')
		{
		    /*ajDebug("  testing *name\n");*/
		    if(ajCharPrefixCaseC(ajStrGetPtr(tagval)+1, name))
		    {
			icount++;
			/*ajDebug("  found [%d] '%S'\n", icount, name);*/

			if(icount >= count)
			{
			    if(ajStrGetLen(tagval) > (ilen+1))
			    {
				if(ajStrGetCharPos(tagval, ilen+1) != ' ')
				    return ajFalse;

				ajStrAssignC(val,
					     ajStrGetPtr(tagval) +
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
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ajFeatGetNoteSI(const AjPFeature thys, const AjPStr name, ajint count,
                       AjPStr* val)
{
    return ajFeatGetNoteCI(thys, ajStrGetPtr(name), count, val);
}




/* @func ajFeatGetNoteS *******************************************************
**
** Finds a named note tag (with a * prefix)
**
** @param [r] thys [const AjPFeature] Feature object
** @param [r] name [const AjPStr] Tag name
** @param [w] val [AjPStr*] Tag value (if found)
**
** @return [AjBool] ajTrue on success (feature tag found)
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ajFeatGetNoteS(const AjPFeature thys, const AjPStr name, AjPStr* val)
{
    return ajFeatGetNoteSI(thys, name, 0, val);
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
**
** @release 6.1.0
** @@
******************************************************************************/

AjBool ajFeatGetTagC(const AjPFeature thys, const char* tname, ajint num,
		    AjPStr* Pval)
{
    AjIList iter     = NULL;
    AjPTagval item = NULL;
    ajint inum       = 0;
    AjBool isnote;
    ajint noteposcolon=0;
    ajint noteposvalue=0;
    const AjPStr tagval = NULL;

    ajDebug("ajFeatGetTagC '%s'\n", tname);
    isnote = ajCharMatchC(tname, "note");
    noteposcolon = strlen(tname) + 1;
    noteposvalue = strlen(tname) + 3;

    if(thys->Tags)
    {
	iter = ajListIterNewread(thys->Tags);
	while(!ajListIterDone(iter))
	{
	    item = (AjPTagval)ajListIterGet(iter);
            tagval = ajTagvalGetValue(item);

            ajDebug("testing '%S'\n", ajTagvalGetTag(item));

	    if(ajStrMatchCaseC(MAJTAGVALGETTAG(item), tname))
	    {
		inum++;
                ajDebug("test1 inum %d\n", inum);

		if(num == inum)
		{
                    ajDebug("++match1\n");
		    ajStrAssignS(Pval, tagval);
		    ajListIterDel(&iter);
		    return ajTrue;
		}
	    }
	    else if(!isnote &&
		    ajStrMatchCaseC(MAJTAGVALGETTAG(item), "note") &&
		    ajStrGetCharFirst(tagval) == '*' &&
		    ajCharPrefixCaseC(ajStrGetPtr(tagval)+1, tname) &&
		    ajStrGetCharPos(tagval, noteposcolon) == ':')
	    {
		inum++;
                ajDebug("test2 inum %d\n", inum);

		if(num == inum)
		{
                    ajDebug("++match2 from %d\n", noteposvalue);
		    ajStrAssignSubS(Pval, tagval, noteposvalue, -1);
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




/* @func ajFeatGetTagS ********************************************************
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
**
** @release 6.1.0
** @@
******************************************************************************/

AjBool ajFeatGetTagS(const AjPFeature thys, const AjPStr name, ajint num,
                     AjPStr* val)
{
    AjIList iter     = NULL;
    AjPTagval item = NULL;
    ajint inum       = 0;
    AjBool isnote;
    ajint noteposcolon=0;
    ajint noteposvalue=0;
    const AjPStr tagval = NULL;

    isnote = ajStrMatchC(name, "note");
    noteposcolon = ajStrGetLen(name) + 1;
    noteposvalue = ajStrGetLen(name) + 3;

    if(thys->Tags)
    {
	iter = ajListIterNewread(thys->Tags);

	while(!ajListIterDone(iter))
	{
	    item = (AjPTagval)ajListIterGet(iter);
            tagval = ajTagvalGetValue(item);
	    if(ajStrMatchCaseS(MAJTAGVALGETTAG(item), name))
	    {
		inum++;

		if(num == inum)
		{
		    ajStrAssignS(val, tagval);
		    ajListIterDel(&iter);
		    return ajTrue;
		}
	    }
	    else if(!isnote &&
		    ajStrMatchCaseC(MAJTAGVALGETTAG(item), "note") &&
		    ajStrGetCharFirst(tagval) == '*' &&
		    ajCharPrefixCaseS(ajStrGetPtr(tagval)+1, name) &&
		    ajStrGetCharPos(tagval, noteposcolon) == ':')
	    {
		inum++;

		if(num == inum)
		{
		    ajStrAssignSubS(val, tagval, noteposvalue, -1);
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
**
** @release 2.1.0
** @@
******************************************************************************/

const AjPStr ajFeatGetType(const AjPFeature thys)
{
    if(!FeatInitDone)
	featInit();

    if(thys->Protein)
	return ajFeattypeGetExternal(thys->Type, FeatTypeTableProtein);
    else
	return ajFeattypeGetExternal(thys->Type, FeatTypeTableDna);
}




/* @func ajFeatGetSource*******************************************************
**
** Returns the source name of a feature object.
**
** @param [r] thys [const AjPFeature] Feature object
**
** @return [const AjPStr] Feature source name
**
** @release 4.0.0
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
**
** @release 2.1.0
** @@
******************************************************************************/

ajuint ajFeatGetStart(const AjPFeature thys)
{
    if((thys->Flags & AJFEATFLAG_START_TWO) &&
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
**
** @release 4.0.0
** @@
******************************************************************************/

char ajFeatGetStrand(const AjPFeature thys)
{
    if(thys->Protein)
        return '.';

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
**
** @release 2.1.0
** @@
******************************************************************************/

ajuint ajFeatGetEnd(const AjPFeature thys)
{
    if((thys->Flags & AJFEATFLAG_END_TWO) &&
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
**
** @release 4.0.0
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
**
** @release 4.0.0
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
**
** @release 2.1.0
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
**
** @release 2.1.0
** @@
******************************************************************************/

ajint ajFeatGetFrame(const AjPFeature thys)
{
    return thys->Frame;
}




/* @func ajFeatGetTranslation *************************************************
**
** Returns translation tag value
**
** @param [r] thys [const AjPFeature] Feature
** @param [w] Ptrans [AjPStr*] Translations
**
** @return [AjBool] True if translation tag was found
**
** @release 6.1.0
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
**
** @release 2.0.0
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
**
** @release 2.0.0
** @@
******************************************************************************/

static void featInit(void)
{
    if(FeatInitDone)
	return;

    FeatInitDone = ajTrue;

    if(!FeatTypeTableDna)
    {
	FeatTypeTableDna = ajTablestrNewCase(200);
	FeatTagsTableDna = ajTablestrNewCase(200);
	featVocabRead("emboss",FeatTypeTableDna, FeatTagsTableDna);
    }

    if(!FeatTypeTableProtein)
    {
	FeatTypeTableProtein = ajTablestrNewCase(200);
	FeatTagsTableProtein = ajTablestrNewCase(200);
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
**
** @release 2.0.0
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
**
** @release 6.0.0
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
    ajlong tagscount = 0;
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
**
** @release 3.0.0
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
**
** @release 6.0.0
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
    const AjPStr tablestr  = NULL;
    const AjPStr sofaid    = NULL;
    const AjPStr storetype  = NULL;

    ajlong typecount = 0;
    ajint filetypecount = 0;

    char reqchar;
    char modchar;
    ajlong ipos;
    AjBool ismodtype = ajFalse;
    AjPStr* Ptyptagstr = NULL;
/*    AjBool taginternal = ajFalse; */
    AjBool newdefid    = ajFalse;
    static ajuint level = 0;

    level++;

    TypeFile = ajDatafileNewInNameS(fname);

    if(!TypeFile)
    {
	ajErr("Unable to read feature types data file '%S'\n", fname);
        level--;
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
                    level--;
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
                    level--;
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
#if FEATDEBUG
			ajDebug("%S %Ld saved '%S' as '%S'\n",
                                fname, typecount, savetype, typtagstr);
			ajDebug("+type %S='%S'\n",
                                savetype, typtagstr);
#endif
/*
			tablestr = ajTablePut(pTypeTable,
					      savetype, typtagstr);
*/
			tablestr = ajTableFetchS(pTypeTable, savetype);

                        if(tablestr)
                        {
                            if(recursion)
                            {
                                ajErr("%S duplicate type %S='%S' "
                                      "already defined as '%S'",
                                      fname, savetype, typtagstr, tablestr);
                            }
                            else
                            {
#if FEATDEBUG
                                ajDebug("found savetype '%S' with value '%S'\n",
                                       savetype, tablestr);
#endif
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
			tablestr = ajTableFetchS(pTypeTable, localname);

			if(tablestr)
			{
#if FEATDEBUG
			    ajDebug("%S duplicate alias type "
				  "%S='%S' already defined as '%S'\n",
				  fname, localname,
				  sofaname, tablestr);
#endif
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
                            ajWarn("%S (level %u) undefined internal ID '%S'",
                                   fname, level, intids);

			ipos = ajStrFindAnyK(*Ptyptagstr, ';');

			if(ipos >= 0)
			  ajStrCutStart(Ptyptagstr, (size_t) ipos);
			else
			    ajStrInsertK(Ptyptagstr, 0, ';');

			ajStrInsertS(Ptyptagstr, 0, type);
                        ajStrDel(&savetype);
			savetype  = type;
                        type = NULL;
			typtagstr = *Ptyptagstr;
/*                        taginternal = ajTrue; */
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
			    tablestr = ajTableFetchS(pTypeTable, defname);

			    if(tablestr)
                            {
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
/*                        taginternal = ajFalse; */

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

#if FEATDEBUG
                                ajDebug("+type %B storetype:'%S' "
					"firstid:'%S'\n",
					ajStrMatchCaseS(storetype, firstid),
					storetype, firstid);
#endif

				if(!ajStrMatchCaseS(storetype, firstid))
				{
				    ajStrAssignS(&localname, storetype);
				    ajStrAssignS(&sofaname, firstid);
#if FEATDEBUG
				    ajDebug("%S sofaid "
                                            "'%S' = '%S'\n",
                                            fname,localname, sofaname);
				    ajDebug("+type (alias) %S='%S'\n",
                                            localname, sofaname);
#endif
				    tablestr = ajTableFetchS(pTypeTable,
                                                             localname);
				    if(tablestr)
                                    {
                                        if(recursion)
                                        {
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
		    if(!ajTableFetchS(pTagsTable, tag))
		    {
			if(ismodtype)
			    ajFeatWarn("%S: tag %S (feature %S) "
				     "not in Etags file",
				     fname, tag, modtype);
			else
			    ajFeatWarn("%S: tag %S (feature %S) "
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
			    ajFeatWarn("%S: delete tag '%S' for %S not modified",
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
				ajFeatWarn("%S: new tag '%S' for %S not modified",
					 fname, tag, modtype);
			}
		    }
		}		
	    }
	}
    }
    
    if(filetypecount > 0)		/* save the last feature type */
    {
#if FEATDEBUG
	ajDebug("+type (final) %S='%S'\n",
                savetype, typtagstr);
#endif
	tablestr = ajTableFetchS(pTypeTable, savetype);
	if(tablestr)
        {
            if(recursion)
            {
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
   
    ajDebug("Total types...: %Ld\n", typecount);

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

    level--;

    return ajTrue;    
}




/* @func ajFeatVocabGetTags ***************************************************
**
** Returns tags table for a named feature format
**
** @param [r] name [const char*] Feature format name
** @return [const AjPTable] Tags table on success
**                          NULL on failure
**
** @release 6.4.0
******************************************************************************/

const AjPTable ajFeatVocabGetTags(const char* name)
{
    ajuint i;

    if(!FeatInitDone)
        featInit();

    for(i=0; featVocab[i].Name; i++)
    {
        if(!strcmp(name, featVocab[i].Truename) ||
           !strcmp(name, featVocab[i].Name))
        {
            if(!*(featVocab[i].Tagstable))
                if(!featVocabInit(i))
                    return NULL;

            return *(featVocab[i].Tagstable);
        }
    }

    return NULL;
}




/* @func ajFeatVocabGetTagsNuc ************************************************
**
** Returns tags table for a named nucleotide feature format
**
** @param [r] name [const char*] Feature format name
** @return [const AjPTable] Tags table on success
**                          NULL on failure
**
** @release 6.4.0
******************************************************************************/

const AjPTable ajFeatVocabGetTagsNuc(const char* name)
{
    ajuint i;

    if(!FeatInitDone)
        featInit();

    for(i=0; featVocab[i].Name; i++)
    {
        if(!featVocab[i].Nucleotide)
            continue;

        if(!strcmp(name, featVocab[i].Truename) ||
           !strcmp(name, featVocab[i].Name))
        {
            if(!*(featVocab[i].Tagstable))
                if(!featVocabInit(i))
                    return NULL;

            return *(featVocab[i].Tagstable);
        }
    }

    return NULL;
}




/* @func ajFeatVocabGetTagsProt ***********************************************
**
** Returns tags table for a named protein feature format
**
** @param [r] name [const char*] Feature format name
** @return [const AjPTable] Tags table on success
**                          NULL on failure
**
** @release 6.4.0
******************************************************************************/

const AjPTable ajFeatVocabGetTagsProt(const char* name)
{
    ajuint i;

    if(!FeatInitDone)
        featInit();

    for(i=0; featVocab[i].Name; i++)
    {
        if(!featVocab[i].Protein)
            continue;

        if(!strcmp(name, featVocab[i].Truename) ||
           !strcmp(name, featVocab[i].Name))
        {
            if(!*(featVocab[i].Tagstable))
                if(!featVocabInit(i))
                    return NULL;

            return *(featVocab[i].Tagstable);
        }
    }

    return NULL;
}




/* @func ajFeatVocabGetTypes **************************************************
**
** Returns types table for a named feature format
**
** @param [r] name [const char*] Feature format name
** @return [const AjPTable] Type table on success
**                          NULL on failure
**
** @release 6.4.0
******************************************************************************/

const AjPTable ajFeatVocabGetTypes(const char* name)
{
    ajuint i;

    if(!FeatInitDone)
        featInit();

    for(i=0; featVocab[i].Name; i++)
    {
        if(!strcmp(name, featVocab[i].Truename) ||
           !strcmp(name, featVocab[i].Name))
        {
            if(!*(featVocab[i].Typetable))
                if(!featVocabInit(i))
                    return NULL;

            return *(featVocab[i].Typetable);
        }
    }

    return NULL;
}




/* @func ajFeatVocabGetTypesNuc ***********************************************
**
** Returns types table for a named nucleotide feature format
**
** @param [r] name [const char*] Feature format name
** @return [const AjPTable] Type table on success
**                          NULL on failure
**
** @release 6.4.0
******************************************************************************/

const AjPTable ajFeatVocabGetTypesNuc(const char* name)
{
    ajuint i;

    if(!FeatInitDone)
        featInit();

    for(i=0; featVocab[i].Name; i++)
    {
        if(!featVocab[i].Nucleotide)
            continue;

        if(!strcmp(name, featVocab[i].Truename) ||
           !strcmp(name, featVocab[i].Name))
        {
            if(!*(featVocab[i].Typetable))
                if(!featVocabInit(i))
                    return NULL;

            return *(featVocab[i].Typetable);
        }
    }

    return NULL;
}




/* @func ajFeatVocabGetTypesProt **********************************************
**
** Returns types table for a named protein feature format
**
** @param [r] name [const char*] Feature format name
** @return [const AjPTable] Type table on success
**                          NULL on failure
**
** @release 6.4.0
******************************************************************************/

const AjPTable ajFeatVocabGetTypesProt(const char* name)
{
    ajuint i;

    if(!FeatInitDone)
        featInit();

    for(i=0; featVocab[i].Name; i++)
    {
        if(!featVocab[i].Protein)
            continue;

        if(!strcmp(name, featVocab[i].Truename) ||
           !strcmp(name, featVocab[i].Name))
        {
            if(!*(featVocab[i].Typetable))
                if(!featVocabInit(i))
                    return NULL;

            return *(featVocab[i].Typetable);
        }
    }

    return NULL;
}




/* @func ajFeatVocabInit ******************************************************
**
** Initialises feature table internals for a named feature format
**
** @param [r] name [const char*] Feature format name
** @return [const AjPTable] Type table on success
**                          NULL on failure
**
** @release 6.4.0
******************************************************************************/

const AjPTable ajFeatVocabInit(const char* name)
{
    ajuint i;

    if(!FeatInitDone)
        featInit();

    for(i=0; featVocab[i].Name; i++)
    {
        if(!strcmp(name, featVocab[i].Truename) ||
           !strcmp(name, featVocab[i].Name))
        {
            if(*(featVocab[i].Typetable))
                return *(featVocab[i].Typetable);
            else
                return featVocabInit(i);
        }
    }

    return NULL;
}




/* @func ajFeatVocabInitNuc ***************************************************
**
** Initialises feature table internals for a named feature format
**
** @param [r] name [const char*] Feature format name
** @return [const AjPTable] Type table on success
**                          NULL on failure
**
** @release 6.4.0
******************************************************************************/

const AjPTable ajFeatVocabInitNuc(const char* name)
{
    ajuint i;

    if(!FeatInitDone)
        featInit();

    for(i=0; featVocab[i].Name; i++)
    {
        if(!featVocab[i].Nucleotide)
            continue;

        if(!strcmp(name, featVocab[i].Truename) ||
           !strcmp(name, featVocab[i].Name))
        {
            if(*(featVocab[i].Typetable))
                return *(featVocab[i].Typetable);
            else
                return featVocabInit(i);
        }
    }

    return NULL;
}




/* @func ajFeatVocabInitProt **************************************************
**
** Initialises feature table internals for a named feature format
**
** @param [r] name [const char*] Feature format name
** @return [const AjPTable] Type table on success
**                          NULL on failure
**
** @release 6.4.0
******************************************************************************/

const AjPTable ajFeatVocabInitProt(const char* name)
{
    ajuint i;

    if(!FeatInitDone)
        featInit();

    for(i=0; featVocab[i].Name; i++)
    {
        if(!featVocab[i].Protein)
            continue;

        if(!strcmp(name, featVocab[i].Truename) ||
           !strcmp(name, featVocab[i].Name))
        {
            if(*(featVocab[i].Typetable))
                return *(featVocab[i].Typetable);
            else
                return featVocabInit(i);
        }
    }

    return NULL;
}




/* @funcstatic featVocabInit **************************************************
**
** Initialises feature table internals for a named feature format
** and returns the types table
**
** @param [r] ivocab [ajuint] Vocabulary index
** @return [const AjPTable] Table if found
**
** @release 6.4.0
******************************************************************************/

static const AjPTable featVocabInit(ajuint ivocab)
{
    const char* name = featVocab[ivocab].Truename;
    AjPTable *Ptypetable = featVocab[ivocab].Typetable;
    AjPTable *Ptagstable = featVocab[ivocab].Tagstable;
    
    if(!FeatInitDone)
        featInit();

    if(!*Ptypetable)
    {
	*Ptypetable = ajTablestrNewCase(200);
	*Ptagstable = ajTablestrNewCase(200);

	featVocabRead(name, *Ptypetable, *Ptagstable);

        if(!*Ptagstable)
            return NULL;

        return *Ptypetable;
    }

    return *Ptypetable;
}




/* @func ajFeatSetDescApp *****************************************************
**
** Sets the description for a feature
**
** @param [u] thys [AjPFeature] Feature
** @param [r] desc [const AjPStr] Feature description (simple text)
** @return [void]
**
** @release 2.3.0
** @@
******************************************************************************/

void ajFeatSetDescApp(AjPFeature thys, const AjPStr desc)
{
    AjPTagval tv        = NULL;

    if(!featTagNote)
	ajStrAssignC(&featTagNote, "note");

    tv = featTagval(thys, featTagNote);

    if(tv)
    {
	ajTagvalAppendC(tv, ", ");
	ajTagvalAppendS(tv, desc);
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
**
** @release 2.0.0
** @@
******************************************************************************/

void ajFeatSetDesc(AjPFeature thys, const AjPStr desc)
{
    ajFeatTagSetC(thys, "note", desc);

    return;
}




/* @func ajFeatSetSource ******************************************************
**
** Sets the source for a feature
**
** @param [u] thys [AjPFeature] Feature
** @param [r] source [const AjPStr] Feature source
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajFeatSetSource(AjPFeature thys, const AjPStr source)
{
    ajStrAssignS(&thys->Source, source);

    return;
}




/* @func ajFeatSetScore *******************************************************
**
** Sets the score for a feature
**
** @param [u] thys [AjPFeature] Feature
** @param [r] score [float] Score value
** @return [void]
**
** @release 4.0.0
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
**
** @release 4.0.0
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




/* @func ajFeatTagSetC ********************************************************
**
** Sets a feature tag value
**
** @param [u] thys [AjPFeature] Feature
** @param [r] tag [const char*] Feature tag
** @param [r] value [const AjPStr] Feature tag value
** @return [AjBool] ajTrue is value was valid
**                  ajFalse if it was "corrected"
**
** @release 2.0.0
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
**
** @release 2.0.0
** @@
******************************************************************************/

AjBool ajFeatTagSet(AjPFeature thys, const AjPStr tag, const  AjPStr value)
{
    AjBool ret     = ajTrue;
    AjPTagval tv = NULL;
    AjBool knowntag = ajTrue;

    const AjPStr tmptag        = NULL;		/* this comes from AjPTable */
                                           /* so please, please don't delete */
    const char* cp;
    
    ajDebug("ajFeatTagSet '%S' '%S' type: '%S' Prot: %B\n",
            tag, value, thys->Type, thys->Protein);
    
    featInit();
    
    if(thys->Protein)
    {
	tmptag = ajFeattagGetNameS(tag, FeatTagsTableProtein, &knowntag);
	ajFeattagFormat(tmptag,  FeatTagsTableProtein, &featFmtTmp);
    }
    else
    {
	tmptag = ajFeattagGetNameS(tag, FeatTagsTableDna, &knowntag);
	ajFeattagFormat(tmptag,  FeatTagsTableDna, &featFmtTmp);
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
            if(!ajFeattagSpecial(tmptag, &featValTmp))
            {
                ret = ajFalse;
                featTagSetDefault(thys, tmptag, value, &featTagTmp,
                                  &featValTmp);
            }
            break;
        case CASE2('S','P') :	/* special regexp */
            /* ajDebug("case special\n"); */
            if(!ajFeattagSpecial(tmptag, &featValTmp))
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
            ajFeatWarn("Unknown internal feature tag type '%S' for '%S'",
                     featFmtTmp, tmptag);
    }
    
    tv = featTagval(thys, featTagTmp);

    if(tv)				/* replace current value */
    {
	ajTagvalReplaceS(tv, featValTmp);
	return ret;
    }

    /* new tag-value */
    tv = featTagvalNew(thys, featTagTmp, featValTmp);
    ajListPushAppend(thys->Tags, tv);
    /* ajDebug("...new tag-value\n"); */

    return ret;
}




/* @func ajFeattagSpecialGff2 *************************************************
**
** Special processing for known GFF tags
**
** This function will be very similar to featTagSpecial, with scope
** for future GFF-specific extensions
**
** @param  [r] tag [const AjPStr] original tag name
** @param  [u] pval [AjPStr*] tag value
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajFeattagSpecialGff2(const AjPStr tag, AjPStr* pval)
{
    /*ajDebug("ajFeattagSpecialGff2 '%S' '%S'\n", tag, *pval);*/

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
    ajFeatWarn("Unrecognised special GFF feature tag '%S'",   tag);

    return ajFalse;
}




/* @funcstatic featTagGff3PredefinedTag ***************************************
**
** Checks whether the specified tag is one of GFF3 tags
** that have predefined meanings
**
** @param  [r] tag [const AjPStr] tag name
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool featTagGff3PredefinedTag(const AjPStr tag)
{
    const char* tags[] = {"ID", "Name", "Alias", "Parent", "Target", "Gap",
                          "Derives_from", "Note", "Dbxref",
                          "Ontology_term", "Is_circular", NULL};
    int i=0;
    
    while(tags[i])
	if(ajStrMatchC(tag, tags[i++]))
	    return ajTrue;

    return ajFalse;
}




/* @func ajFeattagSpecialGff3 *************************************************
**
** Special processing for known GFF3 tags
**
** This function will be very similar to featTagSpecial, with scope
** for future GFF3-specific extensions
**
** @param  [r] tag [const AjPStr] original tag name
** @param  [u] pval [AjPStr*] tag value
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajFeattagSpecialGff3(const AjPStr tag, AjPStr* pval)
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
    ajFeatWarn("Unrecognised special GFF feature tag '%S'",   tag);

    return ajFalse;
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
**
** @release 2.4.0
** @@
******************************************************************************/

AjBool ajFeatTagAddCC(AjPFeature thys, const char* tag, const char* value)
{
    AjBool ret;
    AjPStr tagstr = NULL;
    AjPStr valstr = NULL;

    ajStrAssignC(&tagstr, tag);
    ajStrAssignC(&valstr, value);

    ret = ajFeatTagAddSS(thys, tagstr, valstr);
    ajStrDel(&tagstr);
    ajStrDel(&valstr);

    return ret;
}




/* @func ajFeatTagAddCS *******************************************************
**
** Sets a feature tag value, creating a new feature tag even if one
** already exists.
**
** @param [u] thys [AjPFeature] Feature
** @param [r] tag [const char*] Feature tag
** @param [r] value [const AjPStr] Feature tag value
** @return [AjBool] ajTrue if value was valid
**                  ajFalse if it was bad and was "corrected"
**
** @release 2.4.0
** @@
******************************************************************************/

AjBool  ajFeatTagAddCS(AjPFeature thys, const char* tag, const AjPStr value)
{
    AjBool ret;
    static AjPStr tagstr = NULL;

    ajStrAssignC(&tagstr, tag);

    ret = ajFeatTagAddSS(thys, tagstr, value);

    ajStrDel(&tagstr);

    return ret;
}




/* @func ajFeatTagAddSS *******************************************************
**
** Sets a feature tag value, creating a new feature tag even if one
** already exists.
**
** @param [u] thys [AjPFeature] Feature
** @param [r] tag [const AjPStr] Feature tag
** @param [r] value [const AjPStr] Feature tag value
** @return [AjBool] ajTrue if value was valid
**                  ajFalse if it was bad and was "corrected"
**
** @release 2.0.0
** @@
******************************************************************************/

AjBool ajFeatTagAddSS(AjPFeature thys, const AjPStr tag, const AjPStr value)
{  
    AjBool ret     = ajTrue;
    AjPTagval tv = NULL;
    const AjPStr tmptag  = NULL;		/* this comes from AjPTable */
                                       /* so please, please don't delete */
    AjBool knowntag = ajTrue;
    const char* cp;
    
#if FEATDEBUG
    ajDebug("ajFeatTagAddSS '%S' '%S' type: '%S' Prot: %B\n",
	    tag, value, thys->Type, thys->Protein);
#endif
    
    featInit();
    
    if(thys->Protein)
    {
	tmptag = ajFeattagGetNameS(tag, FeatTagsTableProtein, &knowntag);
	ajFeattagFormat(tmptag,  FeatTagsTableProtein, &featFmtTmp);
    }
    else
    {
	tmptag = ajFeattagGetNameS(tag, FeatTagsTableDna, &knowntag);
	ajFeattagFormat(tmptag, FeatTagsTableDna, &featFmtTmp);
    }
    
#if FEATDEBUG
    ajDebug("tag: '%S' format: '%S'\n", tmptag, featFmtTmp);
#endif

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
            if(!ajFeattagSpecial(tmptag, &featValTmp2))
            {
                ret = ajFalse;
                featTagSetDefault(thys, tmptag, value, &featTagTmp2,
                                  &featValTmp2);
            }
            break;
        case CASE2('S','P') :			/* special regexp */
            /* ajDebug("case special\n");*/
            if(!ajFeattagSpecial(tmptag, &featValTmp2))
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
            ajFeatWarn("Unknown internal feature tag type '%S' for '%S'",
                     featFmtTmp, tmptag);
    }
    
    tv = featTagvalNew(thys, featTagTmp2, featValTmp2);

    ajListPushAppend(thys->Tags, tv);
    /* ajDebug("...new tag-value\n"); */
    
    return ret;
}




/* @func ajFeatTagAddTag ******************************************************
**
** Sets a feature tag value, creating a new feature tag even if one
** already exists.
**
** @param [u] thys [AjPFeature] Feature
** @param [r] tagval [const AjPTagval] Tag value pair
** @return [AjBool] ajTrue if value was valid
**                  ajFalse if it was bad and was "corrected"
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ajFeatTagAddTag(AjPFeature thys, const AjPTagval tagval)
{
    return ajFeatTagAddSS(thys,
                          MAJTAGVALGETTAG(tagval),
                          MAJTAGVALGETVALUE(tagval));
}




/* @func ajFeattagSpecial *****************************************************
**
** Special processing for known internal (EMBL) tags
**
** @param  [r] tag [const AjPStr] original tag name
** @param  [u] pval [AjPStr*] parameter value
** @return [AjBool] ajTrue on success
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajFeattagSpecial(const AjPStr tag, AjPStr* pval)
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
    ajFeatWarn("Unrecognised special EMBL feature tag '%S'",   tag);

    return ajFalse;
}




/* @funcstatic featTagSpecialAllBiomaterial ***********************************
**
** Tests a string as a valid internal (EMBL) feature /bio_material tag
**
** The format is a known type and optional :name
**
** @param  [r] val [const AjPStr] parameter value
** @return [AjBool] ajTrue for a valid value, possibly corrected
**                  ajFalse if invalid, to be converted to default (note) type
**
** @release 6.1.0
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
	ajFeatWarn("bad /bio_material value '%S'", val);
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
**
** @release 2.0.0
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
	ajFeatWarn("bad /anticodon value '%S'",   val);
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
**
** @release 2.0.0
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
	ajFeatWarn("bad /citation value '%S'",   val);
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
**
** @release 2.0.0
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
	ajFeatWarn("unquoted /codon value corrected to '%S'", *pval);
    }

    if(!ret)
    {
	ajFeatWarn("bad /codon value '%S'",   *pval);
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
**
** @release 4.0.0
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
	ajFeatWarn("bad /collection_date value '%S'", val);

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
**
** @release 2.0.0
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
	ajFeatWarn("bad /cons_splice value '%S'",   *pval);
	ajDebug("bad /cons_splice value '%S' beg: '%S' end: '%S'",
		*pval, begstr, endstr);
    }

    if(islow)
    {
	ajFmtPrintS(pval, "(5'site:%S,3'site:%S)", begstr, endstr);
	ajFeatWarn("bad /cons_splice value corrected to '%S'", *pval);
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
**
** @release 4.0.0
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
	ajFeatWarn("bad /inference value '%S' type: '%S'", val, typstr);

    ajStrDel(&typstr);
    return ret;
}




/* @funcstatic featTagSpecialAllLatlon ****************************************
**
** Tests a string as a valid internal (EMBL) feature /lat_lon tag
**
** @param  [r] val [const AjPStr] parameter value
** @return [AjBool] ajTrue for a valid value, possibly corrected
**                  ajFalse if invalid, to be converted to default (note) type
**
** @release 4.0.0
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
	ajFeatWarn("bad /lat_lon value '%S'",   val);
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
**
** @release 4.0.0
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
	ajFeatWarn("bad /PCR_primers value '%S'",   val);
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
**
** @release 2.0.0
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
	ajFeatWarn("bad /rpt_unit value '%S'",   val);
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
**
** @release 6.1.0
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
	ajFeatWarn("bad /rpt_unit_tange value '%S'",   val);
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
**
** @release 4.0.0
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
        "core-vntr1",           /* found in EMBL 104 */
        "core-vntr2",           /* found in EMBL 104 */
        NULL
    };

    /*
    AjBool saveit = ajFalse;
    AjPStr labstr = NULL;
    */

    if(!featRegSpecialRptunitSeq)
        featRegSpecialRptunitSeq =
            ajRegCompC("^([ abcdghkmnrstuvwxyABCDGHKMNRSTUVWXY0-9"
                       "_/\\[\\]\\(\\):;.,+-]+)$");

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
	ajFeatWarn("bad /rpt_unit_seq value '%S'",   *pval);
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
**
** @release 2.0.0
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
	ajFeatWarn("bad /transl_except value '%S'",   val);
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
**
** @release 2.0.0
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
	ajFeatWarn("bad /db_xref value '%S'",   val);
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
**
** @release 2.0.0
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
        ajFeatWarn("ENSEMBL /protein_id value '%S'",   val);*/

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
	ajFeatWarn("bad /protein_id value '%S'", val);
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
**
** @release 2.0.0
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
	ajFeatWarn("bad /replace value '%S'",   *pval);
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
**
** @release 2.0.0
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
	ajFeatWarn("bad /translation value '%S'",   *pval);
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
**
** @release 3.0.0
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
	ajFeatWarn("bad /estimated_length value '%S' corrected to 'unknown'",
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
**
** @release 3.0.0
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
	ajFeatWarn("bad /compare value '%S'", val);
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
**
** @release 5.0.0
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
	ajFeatWarn("bad /mobile_element value '%S'", val);
    }
    ajStrDel(&typstr);
    ajStrDel(&namstr);

    return ret;
}




/* @funcstatic featTagSpecialAllNcrnaclass ************************************
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
**
** @release 6.1.0
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
	ajFeatWarn("bad /ncRNA_class value '%S'", val);
    }

    ajStrDel(&typstr);
    ajStrDel(&namstr);

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
**
** @release 2.0.0
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
**
** @release 2.0.0
** @@
******************************************************************************/

static void featTagSetDefaultDna(const AjPStr tag, const AjPStr value,
				  AjPStr* Pdeftag, AjPStr* Pdefval)
{
    featInit();

    ajStrAssignS(Pdeftag,
                 (const AjPStr) ajTableFetchC(FeatTagsTableDna, ""));
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
**
** @release 2.0.0
** @@
******************************************************************************/

static void featTagSetDefaultProt(const AjPStr tag, const AjPStr value,
				   AjPStr* Pdeftag, AjPStr* Pdefval)
{
    featInit();

    ajStrAssignS(Pdeftag,
		 (const AjPStr) ajTableFetchC(FeatTagsTableProtein, ""));
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
**
** @release 2.0.0
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




/* @func ajFeattableSetCircular ***********************************************
**
** Sets a feature table to be circular
**
** @param [u] thys [AjPFeattable] Feature table
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ajFeattableSetCircular(AjPFeattable thys)
{
    thys->Circular = ajTrue;

    return;
}




/* @func ajFeattableSetDefname ************************************************
**
** Provides a unique (for this program run) name for a feature table.
**
** @param [w] thys [AjPFeattable] Feature table
** @param [r] setname [const AjPStr] Name set by caller
** @return [void]
**
** @release 6.2.0
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




/* @func ajFeattableSetLength *************************************************
**
** Sets the length of a feature table with the length of the source sequence.
**
** This is needed to reverse the table correctly
**
** @param [u] thys [AjPFeattable] Feature table object
** @param [r] len [ajuint] Length
** @return [void]
**
** @release 6.0.0
** @@
**
******************************************************************************/

void ajFeattableSetLength(AjPFeattable thys, ajuint len)
{
    thys->Len = len;

    return;
}




/* @func ajFeattableSetLinear *************************************************
**
** Sets a feature table to be linear
**
** @param [w] thys [AjPFeattable] Feature table
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ajFeattableSetLinear(AjPFeattable thys)
{
    thys->Circular = ajFalse;

    return;
}




/* @func ajFeattableSetNuc ****************************************************
**
** Sets the type of a feature table as nucleotide
**
** @param [u] thys [AjPFeattable] Feature table object
** @return [void]
**
** @release 4.0.0
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
**
** @release 2.1.0
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
**
** @release 2.5.0
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
**
** @release 2.5.0
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

	if((gf->Flags & AJFEATFLAG_REMOTEID) ||
	   (gf->Flags & AJFEATFLAG_LABEL))
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
**
** @release 2.5.0
******************************************************************************/

void ajFeatReverse(AjPFeature  thys, ajint ilen)
{
    AjIList    iter = NULL;
    AjPFeature gf   = NULL;
    ajint itmp;
    ajint saveflags;
    
    ajDebug("ajFeatReverse ilen %d %x '%c' %d..%d %d..%d\n",
	    ilen, thys->Flags, thys->Strand,
	    thys->Start, thys->End, thys->Start2, thys->End2);
    
    saveflags = thys->Flags;
    
    if(thys->Start != thys->End)
    {
        if(thys->Strand == '-')
            thys->Strand = '+';
        else
            thys->Strand = '-';
    }

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
    
    if(saveflags & AJFEATFLAG_START_BEFORE_SEQ)
	thys->Flags |= AJFEATFLAG_END_AFTER_SEQ;
    else
	thys->Flags  &= ~AJFEATFLAG_END_AFTER_SEQ;

    if(saveflags & AJFEATFLAG_END_AFTER_SEQ)
	thys->Flags |=  AJFEATFLAG_START_BEFORE_SEQ;
    else
	thys->Flags &=  ~AJFEATFLAG_START_BEFORE_SEQ;

    if(saveflags & AJFEATFLAG_START_TWO)
	thys->Flags |=  AJFEATFLAG_END_TWO;
    else
	thys->Flags &=  ~AJFEATFLAG_END_TWO;

    if(saveflags & AJFEATFLAG_END_TWO)
	thys->Flags |=  AJFEATFLAG_START_TWO;
    else
	thys->Flags &=  ~AJFEATFLAG_START_TWO;

    if(saveflags & AJFEATFLAG_START_UNSURE)
	thys->Flags |=  AJFEATFLAG_END_UNSURE;
    else
	thys->Flags &=  ~AJFEATFLAG_END_UNSURE;

    if(saveflags & AJFEATFLAG_END_UNSURE)
	thys->Flags |=  AJFEATFLAG_START_UNSURE;
    else
	thys->Flags &=  ~AJFEATFLAG_START_UNSURE;
    
    if(itmp)
	thys->End2 = 1 + ilen - itmp;
    else
	thys->End2 = 0;
    
    /* thys->Frame is rather hard to guess ... leave alone for now */
    
    thys->Frame = 0;		/* set to unknown */
    
    /*ajDebug("     Reversed %x '%c' %d..%d %d..%d\n",
	    thys->Flags, thys->Strand,
	    thys->Start, thys->End, thys->Start2, thys->End2);*/

    if(thys->Subfeatures)
    {
        iter = ajListIterNewread(thys->Subfeatures);
        while(!ajListIterDone(iter))
        {
            gf = ajListIterGet(iter);
            ajFeatReverse(gf, ilen);
        }
        ajListIterDel(&iter);
    }

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
**
** @release 2.0.0
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
**
** @release 2.0.0
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
**
** @release 2.0.0
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
** @return [AjPTagval] New tag-value pair object
**
** @release 2.0.0
** @@
******************************************************************************/

static AjPTagval featTagvalNew(const AjPFeature thys,
                               const AjPStr tag, const AjPStr value)
{
    AjPTagval ret;

    if(thys->Protein)
	ret = featTagvalNewProt(tag, value);
    else
	ret = featTagvalNewDna(tag, value);

    return ret;
}




/* @func ajFeatGfftagAddCS ****************************************************
**
** Constructor for a feature GFF3 tag-value pair
**
** @param [u]   thys   [AjPFeature]   Feature
** @param [r]   tag    [const char*]  Tag name
** @param [r]   value  [const AjPStr] Tag value list
** @return [ajuint] Number of values added
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ajFeatGfftagAddCS(AjPFeature thys,
                         const char* tag, const AjPStr value)
{
    ajuint ret;

    AjPStr tagstr = NULL;

    tagstr= ajStrNewC(tag);

    ret = ajFeatGfftagAddSS(thys, tagstr, value);

    ajStrDel(&tagstr);

    return ret;
}




/* @func ajFeatGfftagAddSS ****************************************************
**
** Constructor for a feature GFF3 tag-value pair
**
** @param [u]   thys   [AjPFeature]   Feature
** @param [r]   tag    [const AjPStr]   Tag name
** @param [r]   value  [const AjPStr]   Tag value list
** @return [ajuint] Number of values added
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ajFeatGfftagAddSS(AjPFeature thys,
                         const AjPStr tag, const AjPStr value)
{
    ajuint ret = 0;
    AjPTagval tagval;
    AjPStrTok handle = NULL;
    AjPStr tmpstr = NULL;
    AjBool predefinedTag = ajFalse;

    featInit();
    predefinedTag = featTagGff3PredefinedTag(tag);

    if(!thys->GffTags && predefinedTag)
        thys->GffTags = ajListNew();

    handle = ajStrTokenNewC(value, ",");

    while(ajStrTokenNextParse(&handle, &tmpstr))
    {
        tagval = ajTagvalNewS(tag, tmpstr);
    
        ret++;

        if(predefinedTag)
            ajListPushAppend(thys->GffTags, tagval);
        else
            ajListPushAppend(thys->Tags, tagval);
        tagval = NULL;
    }

    ajStrTokenDel(&handle);
    ajStrDel(&tmpstr);

    return ret;
}




/* @func ajFeatGfftagAddTag ***************************************************
**
** Sets a feature tag value, creating a new feature tag even if one
** already exists.
**
** @param [u] thys [AjPFeature] Feature
** @param [r] tagval [const AjPTagval] Tag value pair
** @return [AjBool] ajTrue if value was valid
**                  ajFalse if it was bad and was "corrected"
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ajFeatGfftagAddTag(AjPFeature thys, const AjPTagval tagval)
{
    return ajFeatGfftagAddSS(thys,
                             MAJTAGVALGETTAG(tagval),
                             MAJTAGVALGETVALUE(tagval));
}




/* @funcstatic featTagvalNewDna ***********************************************
**
** Constructor for a feature tag-value pair
**
** @param [r]   tag    [const AjPStr]   Tag name
** @param [r]   value  [const AjPStr]   Tag value
** @return [AjPTagval] New tag-value pair object
**
** @release 2.0.0
** @@
******************************************************************************/

static AjPTagval featTagvalNewDna(const AjPStr tag, const AjPStr value)
{
    AjPTagval ret;
    const AjPStr tmptag = NULL;	     /* from AjPTable, don't delete */
    AjBool knowntag = ajTrue;

    featInit();

    tmptag = ajFeattagGetNameS(tag, FeatTagsTableDna, &knowntag);

    ret = ajTagvalNewS(tmptag, value);

    return ret;
}




/* @funcstatic featTagvalNewProt **********************************************
**
** Constructor for a protein feature tag-value pair
**
** @param [r]   tag    [const AjPStr]   Tag name
** @param [r]   value  [const AjPStr]   Tag value
** @return [AjPTagval] New tag-value pair object
**
** @release 2.0.0
** @@
******************************************************************************/

static AjPTagval featTagvalNewProt(const AjPStr tag, const AjPStr value)
{
    AjPTagval ret;
    const AjPStr tmptag = NULL;	     /* from AjPTable, don't delete */
    AjBool knowntag = ajTrue;

    featInit();

    tmptag = ajFeattagGetNameS(tag, FeatTagsTableProtein, &knowntag);

    ret = ajTagvalNewS(tmptag, value);

    return ret;
}




/* @funcstatic featTagval *****************************************************
**
** Checks for the existence of a defined tag for a feature.
**
** @param [r]   thys [const AjPFeature]  Feature object
** @param [r]   tag  [const AjPStr]      Tag name
** @return [AjPTagval] Returns the tag-value pair if found,
**                       NULL if not found.
**
** @release 2.0.0
** @@
******************************************************************************/

static AjPTagval featTagval(const AjPFeature thys, const AjPStr tag)
{
    AjIList iter    = NULL;
    AjPTagval ret = NULL;
    AjPTagval tv  = NULL;

    iter = ajListIterNewread(thys->Tags);

    while(!ajListIterDone(iter))
    {
	tv = ajListIterGet(iter);

	if(ajStrMatchCaseS(MAJTAGVALGETTAG(tv), tag)) 
	{
	    /* ajDebug("featTagval '%S' found value '%S'\n",
	       tag, MAJTAGVALGETVALUE(tv)); */
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




/* @func ajFeattableNewFtable *************************************************
**
** Makes a copy of a feature table.
**
** For cases where we need a copy we can safely change and/or delete.
**
** @param [r]   orig  [const AjPFeattable]  Original feature table
** @return [AjPFeattable] Feature table copy of the original
**
** @release 6.2.0
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
    ajStrAssignS(&ret->Formatstr, orig->Formatstr);
    ret->Format    = orig->Format;
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




/* @func ajFeattableNewFtableLimit ********************************************
**
** Makes a copy of a feature table using only a limited number of features.
**
** For cases where we need a copy we can safely change and/or delete.
**
** @param [r]   orig  [const AjPFeattable]  Original feature table
** @param [r]   limit  [ajint]  Limit to number of features copied
** @return [AjPFeattable] Feature table copy of the original
**
** @release 6.2.0
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
    ajStrAssignS(&ret->Formatstr, orig->Formatstr);
    ret->Format    = orig->Format;
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




/* @func ajFeatNewFeat ********************************************************
**
** Makes a copy of a feature.
**
** For cases where we need a copy we can safely change and/or delete.
**
** @param [r]   orig  [const AjPFeature]  Original feature
** @return [AjPFeature] Feature  copy of the original
** @category new [AjPFeature] Copy constructor
**
** @release 6.2.0
** @@
******************************************************************************/

AjPFeature ajFeatNewFeat(const AjPFeature orig)
{
    AjPFeature ret;
    AjPFeature subfeat;
    AjIList iter;
    AjPTagval tvorig;

    ret = featFeatureNew();

    if(orig->Source)
        ajStrAssignS(&ret->Source, orig->Source);
    if(orig->Type)
        ajStrAssignS(&ret->Type, orig->Type);
    if(orig->Remote)
        ajStrAssignS(&ret->Remote, orig->Remote);
    if(orig->Label)
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

    if(orig->Subfeatures)
    {
        iter = ajListIterNewread(orig->Subfeatures);

        while(!ajListIterDone(iter))
        {
            subfeat = ajListIterGet(iter);
            ajListPushAppend(ret->Subfeatures, ajFeatNewFeat(subfeat));
        }

        ajListIterDel(&iter);
    }

    if(orig->Tags)
    {
        iter = ajListIterNewread(orig->Tags);

        while(!ajListIterDone(iter))
        {
            tvorig = ajListIterGet(iter);
            ajFeatTagAddTag(ret, tvorig);
        }

        ajListIterDel(&iter);
    }

    if(orig->GffTags)
    {
        iter = ajListIterNewread(orig->GffTags);

        while(!ajListIterDone(iter))
        {
            tvorig = ajListIterGet(iter);
            ajFeatGfftagAddTag(ret, tvorig);
        }

        ajListIterDel(&iter);
    }

    return ret;
}




/* @func ajFeatTrace **********************************************************
**
** Traces (to the debug file) a feature object
**
** @param [r]   thys  [const AjPFeature]  Feature
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajFeatTrace(const AjPFeature thys)
{
    AjPStr flagstr = NULL;
    AjPFeature subft = NULL;
    AjIList iter = NULL;
    ajuint isub = 0;

    ajDebug("  address: %p\n", thys);
    ajDebug("  Source: '%S'\n", thys->Source);
    if(thys->Protein)
        ajDebug("  Type: '%S' protein: %B '%S'\n",
                thys->Type, thys->Protein, ajFeatTypeProt(thys->Type));
    else
        ajDebug("  Type: '%S' protein: %B '%S'\n",
                thys->Type, thys->Protein, ajFeatTypeNuc(thys->Type));
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
        if(thys->Flags & AJFEATFLAG_START_BEFORE_SEQ)
            ajStrAppendC(&flagstr, "start_before ");

        if(thys->Flags & AJFEATFLAG_END_AFTER_SEQ)
            ajStrAppendC(&flagstr, "end_after ");

        if(thys->Flags & AJFEATFLAG_GENERATED)
            ajStrAppendC(&flagstr, "generated ");

        if(thys->Flags & AJFEATFLAG_BETWEEN_SEQ)
            ajStrAppendC(&flagstr, "between ");

        if(thys->Flags & AJFEATFLAG_START_TWO)
            ajStrAppendC(&flagstr, "start2 ");

        if(thys->Flags & AJFEATFLAG_END_TWO)
            ajStrAppendC(&flagstr, "end2 ");

        if(thys->Flags & AJFEATFLAG_POINT)
            ajStrAppendC(&flagstr, "point ");

        if(thys->Flags & AJFEATFLAG_COMPLEMENT_MAIN)
            ajStrAppendC(&flagstr, "overall_complement ");

        if(thys->Flags & AJFEATFLAG_MULTIPLE)
            ajStrAppendC(&flagstr, "multiple ");

        if(thys->Flags & AJFEATFLAG_ORDER)
            ajStrAppendC(&flagstr, "order ");

        if(thys->Flags & AJFEATFLAG_REMOTEID)
            ajStrAppendC(&flagstr, "remote_id ");

        if(thys->Flags & AJFEATFLAG_LABEL)
            ajStrAppendC(&flagstr, "label ");

        if(thys->Flags & AJFEATFLAG_START_UNSURE)
            ajStrAppendC(&flagstr, "start_unsure ");

        if(thys->Flags & AJFEATFLAG_END_UNSURE)
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

    if(ajListGetLength(thys->Subfeatures))
    {
        ajDebug("  Subfeatures: %Lu\n\n", ajListGetLength(thys->Subfeatures));
        isub = 0;
        iter = ajListIterNewread(thys->Subfeatures);
        while(!ajListIterDone(iter))
        {
            ajDebug("Subfeature[%u] ...\n", ++isub); 
            subft = ajListIterGet(iter);
            ajFeatTrace(subft);
        }
        ajListIterDel(&iter);
        ajDebug("  Sub %u done\n\n", isub);
    }

    return;
}




/* @func ajFeatTagTrace *******************************************************
**
** Traces (to the debug file) the tag-value pairs of a feature object
**
** @param [r]   thys  [const AjPFeature]  Feature
** @return [void]
**
** @release 2.0.0
** @@
******************************************************************************/

void ajFeatTagTrace(const AjPFeature thys)
{
    AjIList iter;
    ajint i = 0;
    AjPTagval tv = NULL;

    ajFeatGfftagTrace(thys);

    iter = ajListIterNewread(thys->Tags);

    while(!ajListIterDone(iter))
    {
	tv = ajListIterGet(iter);
	ajDebug(" %3d  %S : '%S'\n",
                ++i, ajTagvalGetTag(tv), ajTagvalGetValue(tv));
    }

    ajListIterDel(&iter);

    return;
}




/* @func ajFeatGfftagTrace ****************************************************
**
** Traces (to the debug file) the GFF tag-value pairs of a feature object
**
** @param [r]   thys  [const AjPFeature]  Feature
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ajFeatGfftagTrace(const AjPFeature thys)
{
    AjIList iter;
    ajint i = 0;
    AjPTagval tv = NULL;

    iter = ajListIterNewread(thys->GffTags);

    while(!ajListIterDone(iter))
    {
	tv = ajListIterGet(iter);
	ajDebug(" %3d  %S : '%S'\n",
                ++i, ajTagvalGetTag(tv), ajTagvalGetValue(tv));
    }

    ajListIterDel(&iter);

    return;
}




/* @func ajFeatGfftagsNew *****************************************************
**
** Creates a structure holding GFF tag value pairs
**
** @return [AjPFeatGfftags] GFF tag-values structure
**
** @release 6.4.0
** @@
******************************************************************************/

AjPFeatGfftags ajFeatGfftagsNew(void)
{
    AjPFeatGfftags ret = NULL;

    AJNEW0(ret);
    
    return ret;
}




/* @func ajFeatGfftagsDel *****************************************************
**
** Destroys a structure holding GFF tag value pairs
**
** @param [d] Pthys [AjPFeatGfftags*] GFF tag-values structure
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajFeatGfftagsDel(AjPFeatGfftags *Pthys)
{
    AjPFeatGfftags thys;

    if(!Pthys)
        return;
    if(!*Pthys)
        return;

    thys = *Pthys;

    ajStrDel(&thys->Id);
    ajStrDel(&thys->Name);
    ajStrDel(&thys->Alias);
    ajStrDel(&thys->Parent);
    ajStrDel(&thys->Target);
    ajStrDel(&thys->Gap);
    ajStrDel(&thys->DerivesFrom);
    ajStrDel(&thys->Note);
    ajStrDel(&thys->Dbxref);
    ajStrDel(&thys->OntologyTerm);
    ajStrDel(&thys->IsCircular);

    AJFREE(*Pthys);
    
    return;
}




/* @func ajFeatGetGfftags *****************************************************
**
** Returns an object with all feature GFF tag-value pairs
**
** @param [r]   thys  [const AjPFeature]  Feature
** @return [AjPFeatGfftags] Gff tags object
**
** @release 6.4.0
** @@
******************************************************************************/

AjPFeatGfftags ajFeatGetGfftags(const AjPFeature thys)
{
    AjPFeatGfftags ret = NULL;
    AjIList iter;
    AjPStr *Pvalstr = NULL;
    AjPStr tmpstr = NULL;
    AjPTagval tv = NULL;
    AjBool multivalues = ajFalse;
    const AjPStr tvtag = NULL;

    ret = ajFeatGfftagsNew();

    if(!thys)
        return ret;

    if(!ajListGetLength(thys->GffTags))
        return ret;

    iter = ajListIterNewread(thys->GffTags);

    while(!ajListIterDone(iter))
    {
        tv = ajListIterGet(iter);
        tvtag = ajTagvalGetTag(tv);

        Pvalstr = NULL;
        multivalues = ajFalse;

        if(ajStrMatchC(tvtag, "ID"))
            Pvalstr = &ret->Id;
        else if(ajStrMatchC(tvtag, "Name"))
            Pvalstr = &ret->Name;
        else if(ajStrMatchC(tvtag, "Alias"))
        {
            multivalues = ajTrue;
            Pvalstr = &ret->Alias;
        }
        else if(ajStrMatchC(tvtag, "Parent"))
        {
            multivalues = ajTrue;
            Pvalstr = &ret->Parent;
        }
        else if(ajStrMatchC(tvtag, "Target"))
            Pvalstr = &ret->Target;
        else if(ajStrMatchC(tvtag, "Gap"))
            Pvalstr = &ret->Gap;
        else if(ajStrMatchC(tvtag, "Derives_from"))
            Pvalstr = &ret->DerivesFrom;
        else if(ajStrMatchC(tvtag, "Note"))
        {
            multivalues = ajTrue;
            Pvalstr = &ret->Note;
        }
        else if(ajStrMatchC(tvtag, "Dbxref"))
        {
            multivalues = ajTrue;
            Pvalstr = &ret->Dbxref;
        }
        else if(ajStrMatchC(tvtag, "Ontology_term"))
        {
            multivalues = ajTrue;
            Pvalstr = &ret->OntologyTerm;
        }
        else if(ajStrMatchC(tvtag, "Is_circular"))
            Pvalstr = &ret->IsCircular;

        if(Pvalstr)
        {
            ajStrAssignS(&tmpstr, ajTagvalGetValue(tv));
            ajStrFmtPercentEncodeC(&tmpstr, ";=&%,");
            if(multivalues)
            {
                if(ajStrGetLen(*Pvalstr))
                    ajStrAppendC(Pvalstr, ",");
                ajStrAppendS(Pvalstr, tmpstr);
            }
            else
                ajStrAssignS(Pvalstr, tmpstr);
        }
    }

    ajListIterDel(&iter);
    ajStrDel(&tmpstr);
            
    return ret;
}




/* @func ajFeatGetId **********************************************************
**
** Returns the GFF ID tag of a feature with GFF tag-value pairs
**
** @param [r]   thys  [const AjPFeature]  Feature
** @return [const AjPStr] ID tag
**
** @release 6.5.0
** @@
******************************************************************************/

const AjPStr ajFeatGetId(const AjPFeature thys)
{
    AjIList iter;
    AjPTagval tv = NULL;

    if(!thys)
        return NULL;

    if(!ajListGetLength(thys->GffTags))
        return NULL;

    iter = ajListIterNewread(thys->GffTags);

    while(!ajListIterDone(iter))
    {
        tv = ajListIterGet(iter);

        if(ajStrMatchC(MAJTAGVALGETTAG(tv), "Id"))
        {
            ajListIterDel(&iter);
            return MAJTAGVALGETVALUE(tv);
        }
    }

    return NULL;
}




/* @func ajFeatGetParent ******************************************************
**
** Returns the GFF Parent tag of a feature with GFF tag-value pairs
**
** @param [r]   thys  [const AjPFeature]  Feature
** @return [const AjPStr] Parent tag
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPStr ajFeatGetParent(const AjPFeature thys)
{
    AjIList iter;
    AjPTagval tv = NULL;

    if(!thys)
        return NULL;

    if(!ajListGetLength(thys->GffTags))
        return NULL;

    iter = ajListIterNewread(thys->GffTags);

    while(!ajListIterDone(iter))
    {
        tv = ajListIterGet(iter);

        if(ajStrMatchC(MAJTAGVALGETTAG(tv), "Parent"))
        {
            ajListIterDel(&iter);
            return MAJTAGVALGETVALUE(tv);
        }
    }

    return NULL;
}




/* @func ajFeatTagIter ********************************************************
**
** Returns an iterator over all feature tag-value pairs
**
** @param [r]   thys  [const AjPFeature]  Feature
** @return [AjIList] List iterator
**
** @release 2.0.0
** @@
******************************************************************************/

AjIList ajFeatTagIter(const AjPFeature thys)
{
    return ajListIterNewread(thys->Tags);
}




/* @func ajFeatSubIter ********************************************************
**
** Returns an iterator over all subfeaturers
**
** @param [r]   thys  [const AjPFeature]  Feature
** @return [AjIList] List iterator
**
** @release 6.4.0
** @@
******************************************************************************/

AjIList ajFeatSubIter(const AjPFeature thys)
{
    if(!ajListGetLength(thys->Subfeatures))
        return NULL;

    return ajListIterNewread(thys->Subfeatures);
}




/* @func ajFeatTagval *********************************************************
**
** Returns the tag-value pairs of a feature object
**
** @param [u]  iter  [AjIList] List iterator from ajFeatTagIter
** @param [w] Ptagnam [AjPStr*] Tag name
** @param [w] Ptagval [AjPStr*] Tag val
** @return [AjBool] ajTrue if another tag-value pair was returned
**
** @release 2.0.0
** @@
******************************************************************************/

AjBool ajFeatTagval(AjIList iter, AjPStr* Ptagnam, AjPStr* Ptagval)
{
    AjPTagval tv = NULL;

    tv = ajListIterGet(iter);

    if(!tv)
	return ajFalse;

    ajStrAssignS(Ptagnam, MAJTAGVALGETTAG(tv));
    ajStrAssignS(Ptagval, MAJTAGVALGETVALUE(tv));

    return ajTrue;
}




/* @func ajFeattableTrace *****************************************************
**
** Traces (to the debug file) a complete feature table
**
** @param [r]   thys  [const AjPFeattable]  Feature table
** @return [void]
**
** @release 2.0.0
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




/* @func ajFeatTypeNuc ********************************************************
**
** Given a feature type name,
** returns the valid feature type for the internal DNA feature table
**
** @param [r]   type  [const AjPStr] Type name
** @return [const AjPStr] Valid feature type
**
** @release 6.0.0
** @@
******************************************************************************/

const AjPStr ajFeatTypeNuc(const AjPStr type)
{
    if(!FeatInitDone)
	featInit();

    return ajFeattypeGetExternal(type, FeatTypeTableDna);
}




/* @func ajFeatTypeProt *******************************************************
**
** Given a feature type name,
** returns the valid feature type for the internal protein feature table
**
** @param [r]   type  [const AjPStr] Type name
** @return [const AjPStr] Valid feature type
**
** @release 6.0.0
** @@
******************************************************************************/

const AjPStr ajFeatTypeProt(const AjPStr type)
{
    if(!FeatInitDone)
        featInit();

    return ajFeattypeGetExternal(type, FeatTypeTableProtein);
}




/* @funcstatic featTypeDna ****************************************************
**
** Given a feature type name,
** returns the valid feature type for the internal DNA feature table
**
** @param [r]   type  [const AjPStr] Type name
** @return [const AjPStr] Valid feature type
**
** @release 2.0.0
** @@
******************************************************************************/

static const AjPStr featTypeDna(const AjPStr type)
{
    if(!FeatInitDone)
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
**
** @release 2.0.0
** @@
******************************************************************************/

static const AjPStr featTypeProt(const AjPStr type)
{
    if(!FeatInitDone)
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
**
** @release 6.1.0
** @@
******************************************************************************/

static const AjPStr featTypeDnaLimit(const AjPStr type)
{
    if(!FeatInitDone)
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
**
** @release 6.1.0
** @@
******************************************************************************/

static const AjPStr featTypeProtLimit(const AjPStr type)
{
    if(!FeatInitDone)
        featInit();

    return featTableTypeInternalLimit(type, FeatTypeTableProtein);
}




/* @funcstatic featTypeTestDnaWild ********************************************
**
** Given a feature type name,
** returns the valid feature type for the internal DNA feature table
** following alternative names until a matching wildcard name is found
**
** @param [r]   type  [const AjPStr] Type name
** @param [r]   str   [const AjPStr] Wildcard name
** @return [AjBool] True if a match is found
**
** @release 6.2.0
** @@
******************************************************************************/

static AjBool featTypeTestDnaWild(const AjPStr type, const AjPStr str)
{
    if(!FeatInitDone)
        featInit();

    return featTableTypeTestWild(type, FeatTypeTableDna, str);
}




/* @funcstatic featTypeTestProtWild *******************************************
**
** Given a feature type name,
** returns the valid feature type for the internal protein feature table
** following alternative names until a unique name is found
**
** @param [r]   type  [const AjPStr] Type name
** @param [r]   str   [const AjPStr] Wildcard name
** @return [AjBool] True if a match is found
**
** @release 6.2.0
** @@
******************************************************************************/

static AjBool featTypeTestProtWild(const AjPStr type, const AjPStr str)
{
    if(!FeatInitDone)
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
**
** @release 2.0.0
** @@
******************************************************************************/

static const AjPStr featTagDna(const AjPStr thys, AjBool* knowntag)
{
    featInit();

    return ajFeattagGetNameS(thys, FeatTagsTableDna, knowntag);
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
**
** @release 2.0.0
** @@
******************************************************************************/

static const AjPStr featTagProt(const AjPStr thys, AjBool* knowntag)
{
    featInit();

    return ajFeattagGetNameS(thys, FeatTagsTableProtein, knowntag);
}




/* @func ajFeattypeGetExternal ************************************************
**
** Given a feature type name,
** returns the valid feature type for a feature table
**
** @param [r]   type  [const AjPStr] Type name
** @param [r]   table [const AjPTable]  Feature table
** @return [const AjPStr] Valid feature type
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPStr ajFeattypeGetExternal(const AjPStr type,
                                   const AjPTable table)
{
    static const AjPStr ret = NULL;
    static const AjPStr retkey = NULL;
    AjPStr defaultid = NULL;
    AjPStr tmpstr = NULL;
    ajlong i;

    retkey = ajTablestrFetchkeyS(table, type);

    if(!retkey)
    {
        defaultid = ajStrNew();
        retkey = ajTableFetchS(table, defaultid);
        ajStrDel(&defaultid);
        ajDebug("ajFeattypeGetExternal '%S' not in external table %x, "
                "use default '%S'\n", type, table, retkey);
    }
    
    ret = ajTableFetchS(table, retkey);
    /*ajDebug("ajFeattypeGetExternal a '%S' found in internal table as"
      " '%S' = '%S\n", type, retkey, ret);*/

    if(ajStrGetCharLast(ret) != ';')
    {
        retkey = ret;
        ret = ajTableFetchS(table, retkey);
        ajDebug("ajFeattypeGetExternal b '%S' found in internal table"
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
            /*ajDebug("ajFeattypeGetExternal '%S' is an alias for '%S'\n",
              retkey, tmpstr);*/
            ret = ajTablestrFetchkeyS(table, tmpstr);

            if(!ret)	  /* oops, back to the previous one */
            {
                ajFeatWarn("ajFeattypeGetExternal failed to find"
                         " '%S' alias '%S",
                         type, tmpstr);
                ret = ajTableFetchS(table, retkey);
            }

            ajStrDel(&tmpstr);
        }
    }

    /*ajDebug("ajFeattypeGetExternal result '%S'\n",
      ret);*/

    return ret;
}




/* @func ajFeattypeGetInternal ************************************************
**
** Given a feature type name,
** returns the valid feature type for an internal feature table
**
** @param [r]   type  [const AjPStr] Type name
** @return [const AjPStr] Valid feature type
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPStr ajFeattypeGetInternal(const AjPStr type)
{
    const AjPStr ret = NULL;

    if(!FeatInitDone)
        featInit();

    ret = featTableTypeInternal(type, FeatTypeTableDna);
    if(ret)
        return ret;

    return featTableTypeInternal(type, FeatTypeTableProtein);
}




/* @func ajFeattypeGetInternalEmbl ********************************************
**
** Given a feature type name,
** returns the valid feature type for an EMBL format feature table
**
** @param [r]   type  [const AjPStr] Type name
** @return [const AjPStr] Valid feature type
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPStr ajFeattypeGetInternalEmbl(const AjPStr type)
{
    if(!FeatInitDone)
	featInit();

    if(!FeatTypeTableEmbl)
        ajFeatVocabInitNuc("embl");

    return featTableTypeInternal(type, FeatTypeTableEmbl);
}




/* @func ajFeattypeGetInternalNuc *********************************************
**
** Given a feature type name,
** returns the valid feature type for an internal feature table
**
** @param [r]   type  [const AjPStr] Type name
** @return [const AjPStr] Valid feature type
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPStr ajFeattypeGetInternalNuc(const AjPStr type)
{
    if(!FeatInitDone)
	featInit();

    return featTableTypeInternal(type, FeatTypeTableDna);
}




/* @func ajFeattypeGetInternalPir *********************************************
**
** Given a feature type name,
** returns the valid feature type for an internal feature table
**
** @param [r]   type  [const AjPStr] Type name
** @return [const AjPStr] Valid feature type
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPStr ajFeattypeGetInternalPir(const AjPStr type)
{
    if(!FeatInitDone)
	featInit();

    if(!FeatTypeTablePir)
        ajFeatVocabInitProt("pir");

    return featTableTypeInternal(type, FeatTypeTablePir);
}




/* @func ajFeattypeGetInternalProt ********************************************
**
** Given a feature type name,
** returns the valid feature type for an internal feature table
**
** @param [r]   type  [const AjPStr] Type name
** @return [const AjPStr] Valid feature type
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPStr ajFeattypeGetInternalProt(const AjPStr type)
{
    if(!FeatInitDone)
	featInit();

    return featTableTypeInternal(type, FeatTypeTableProtein);
}




/* @func ajFeattypeGetInternalRefseqp *****************************************
**
** Given a feature type name,
** returns the valid feature type for a REFSEQP format feature table
**
** @param [r]   type  [const AjPStr] Type name
** @return [const AjPStr] Valid feature type
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPStr ajFeattypeGetInternalRefseqp(const AjPStr type)
{
    if(!FeatInitDone)
	featInit();

    if(!FeatTypeTableRefseqp)
        ajFeatVocabInitProt("refseqp");

    return featTableTypeInternal(type, FeatTypeTableRefseqp);
}




/* @funcstatic featTableTypeInternal ******************************************
**
** Given a feature type name,
** returns the valid feature type for a feature table
**
** @param [r]   type  [const AjPStr] Type name
** @param [r]   table [const AjPTable]  Feature table
** @return [const AjPStr] Valid feature type
**
** @release 4.0.0
** @@
******************************************************************************/

static const AjPStr featTableTypeInternal(const AjPStr type,
					  const AjPTable table)
{
    const AjPStr ret = NULL;
    const AjPStr retkey = NULL;
    const AjPStr tmpstr = NULL;

    retkey = ajTablestrFetchkeyS(table, type);

    if(retkey)
    {
#if FEATDEBUG
	ajDebug("featTableTypeInternal '%S' found in internal table"
	  " as '%S'\n",type, retkey);
#endif

	tmpstr = ajTableFetchS(table, retkey);

	if(ajStrGetCharLast(tmpstr) != ';')
	{
#if FEATDEBUG
	    ajDebug("featTableTypeInternal '%S' is an alias for '%S'\n",
		 retkey, tmpstr);
#endif
	    retkey = ajTablestrFetchkeyS(table, tmpstr);
	}

	ajDebug("featTableTypeInternal result '%S'\n",
		 retkey);
	return retkey;
    }

    ret = ajTableFetchS(table, ajStrNew());
#if FEATDEBUG
    ajDebug("featTableTypeInternal '%S' not in internal table %x, "
	    "default to '%S'\n", type, table, ret);

    /* ajTablestrTrace(table); */

    /*ajDebug("featTableTypeInternal result '%S'\n",
	     ret);*/
#endif

    return ret;
}




/* @funcstatic featTableTypeInternalLimit *************************************
**
** Given a feature type name,
** returns the valid feature type for a feature table
** following alternative names which are common in the internal table
** which is a combination of multiple definitions
**
** @param [r]   type  [const AjPStr] Type name
** @param [r]   table [const AjPTable]  Feature table
** @return [const AjPStr] Valid feature type or NULL if not found
**
** @release 6.1.0
** @@
******************************************************************************/

static const AjPStr featTableTypeInternalLimit(const AjPStr type,
                                               const AjPTable table)
{
    const AjPStr retkey = NULL;
    const AjPStr tmpstr = NULL;
    const AjPStr savekey = NULL;
    ajuint i = 0;

    retkey = ajTablestrFetchkeyS(table, type);

    while(retkey && i < 5)
    {
        savekey = retkey;
	tmpstr = ajTableFetchS(table, retkey);

#if FEATDEBUG
	ajDebug("featTableTypeInternalLimit '%S' found in internal table"
                " as '%S' (%S)\n",type, retkey, tmpstr);
#endif

	if(ajStrGetCharLast(tmpstr) != ';')
	{
#if FEATDEBUG
	    ajDebug("featTableTypeInternalLimit '%S' is an alias for '%S'\n",
		 retkey, tmpstr);
#endif
	    retkey = ajTablestrFetchkeyS(table, tmpstr);
	}
        else 
        {
           tmpstr = ajTableFetchS(table, retkey);
           retkey = ajTablestrFetchkeyS(table, tmpstr);

           if(ajStrMatchS(retkey, savekey))
               retkey = NULL;
	}
#if FEATDEBUG
	ajDebug("featTableTypeInternalLimit result %u '%S'\n",
                ++i, savekey);
#endif
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




/* @funcstatic featTableTypeTestWild ******************************************
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
**
** @release 6.2.0
** @@
******************************************************************************/

static AjBool featTableTypeTestWild(const AjPStr type,
                                    const AjPTable table,
                                    const AjPStr str)
{
    const AjPStr retkey = NULL;
    const AjPStr tmpstr = NULL;
    AjPStr tmpkey = NULL;
    ajulong i = 0UL;
    ajulong nkeys;
    void **keys = NULL;
    AjPStr key = NULL;

    if(ajStrMatchWildS(type, str))
        return ajTrue;

    retkey = ajTablestrFetchkeyS(table, type);

    while(retkey && i < 5)
    {
	tmpstr = ajTableFetchS(table, retkey);
	ajDebug("featTableTypeTestWild '%S' found in internal table"
                " as '%S' (%S)\n",type, retkey, tmpstr);

	if(ajStrGetCharLast(tmpstr) != ';')
	{
            if(ajStrMatchWildS(tmpstr, str))
                return ajTrue;

            tmpstr = ajTableFetchS(table, retkey);
	    retkey = ajTablestrFetchkeyS(table, tmpstr);
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

            retkey = ajTablestrFetchkeyS(table, tmpstr);

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




/* @func ajFeattagGetNameC ****************************************************
**
** Given a feature tag name,
** returns the valid feature tag name for a feature table
**
** @param [r]   tag  [const char*] Type name
** @param [r]   table [const AjPTable]  Feature table
** @param [w] knowntag [AjBool*] ajTrue if the tag name is known
**                               ajFalse if the default name was substituted
** @return [const AjPStr] Valid feature tag name
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPStr ajFeattagGetNameC(const char* tag, const AjPTable table,
                               AjBool* knowntag)
{
    static const AjPStr ret    = NULL;

    static const AjPStr deftag  = NULL;

    deftag = ajTableFetchC(table, "");

    if(tag)
    {
	ret = ajTablestrFetchkeyC(table, tag);

	if(ret)
	{
	    *knowntag = ajTrue;
#if FEATDEBUG
	    if(ajStrMatchS(ret, deftag))
            {
                ajDebug("featTag '%s' found in internal table as '%S' "
                        "(default)\n",
                        tag, ret);
            }
            else
            {
                ajDebug("featTag '%s' found in internal table as '%S'\n",
                        tag, ret);
            }
#endif
	    return ret;
	}
	else
	{
	    *knowntag = ajFalse;
            ret = deftag;
#if FEATDEBUG
	    ajDebug("featTag '%s' not in internal table %x,"
	       " default to '%S'\n",
	       tag, table, deftag);
	    /* ajTablestrTrace(table); */
#endif
	}
    }
    else
    {
	*knowntag = ajFalse;
	ret = deftag;
#if FEATDEBUG
	ajDebug("featTag '%s' use default '%S'\n",
                tag, deftag);
#endif
    }

    return ret;
}




/* @func ajFeattagGetNameS ****************************************************
**
** Given a feature tag name,
** returns the valid feature tag name for a feature table
**
** @param [r]   tag  [const AjPStr] Type name
** @param [r]   table [const AjPTable]  Feature table
** @param [w]   knowntag  [AjBool*] ajTrue if the tag was found in the
**                                  list of known tags
** @return [const AjPStr] Valid feature tag name
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPStr ajFeattagGetNameS(const AjPStr tag, const AjPTable table,
                              AjBool* knowntag)
{
    static const AjPStr ret     = NULL;
    static const AjPStr deftag  = NULL;

    deftag = ajTableFetchC(table, "");

    if(tag)
    {
	ret = ajTablestrFetchkeyS(table, tag);

	if(ret)
	{
	    *knowntag = ajTrue;
#if FEATDEBUG
	    if(ajStrMatchS(ret, deftag))
            {
                ajDebug("featTag '%S' found in internal table as '%S' (default)\n",
                        tag, ret);
            }
            else
            {
                ajDebug("featTag '%S' found in internal table as '%S'\n",
                        tag, ret);
            }
#endif
	    return ret;
	}
	else
	{
	    *knowntag = ajFalse;
            ret = deftag;
#if FEATDEBUG
	    ajDebug("featTag '%S' not in internal table %x,"
	       " default to '%S'\n",
	       tag, table, deftag);
	    /* ajTablestrTrace(table); */
#endif
	}
    }
    else
    {
	*knowntag = ajFalse;
	ret = deftag;
#if FEATDEBUG
	ajDebug("featTag '%S' use default '%S'\n",
                tag, deftag);
#endif
    }

    return ret;
}




/* @func ajFeattagFormat ******************************************************
**
** Converts a feature tag value into the correct format, after
** checking it is an acceptable value
**
** @param [r] name  [const AjPStr] Tag name
** @param [r] table [const AjPTable] Tag table
** @param [w] retstr [AjPStr*] string with formatted value.
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajFeattagFormat(const AjPStr name, const AjPTable table,
                     AjPStr* retstr)
{
    const AjPStr tagstr    = NULL;
    const char* cp;
    const char* cq;
    ajint i;

    tagstr = ajTableFetchS(table, name);

    cp = ajStrGetPtr(tagstr);

    ajStrAssignClear(retstr);

    cq = cp;
    i=0;

    while(*cp && (*cp++ != ';'))
	i++;

    ajStrAssignLenC(retstr, cq, i);

    /* ajDebug("ajFeattagFormat '%S' type '%S' (%S)\n",
       name, *retstr, tagstr); */

    return;
}




/* @func ajFeattagGetLimit ****************************************************
**
** Returns the controlled vocabulary list for a limited value.
**
** @param [r] name  [const AjPStr] Tag name
** @param [r] table [const AjPTable] Tag table
** @param [w] retstr [AjPStr*] string with formatted value.
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajFeattagGetLimit(const AjPStr name, const AjPTable table,
                       AjPStr* retstr)
{
    const AjPStr tagstr;
    const char* cp = NULL;

    ajStrAssignClear(retstr);
    tagstr = ajTableFetchS(table, name);
    cp = ajStrGetPtr(tagstr);

    while(*cp && (*cp != ';'))
	cp++;

    if(!*cp)
	return;

    ajStrAssignC(retstr, cp);

    return;
}




/* @func ajFeatUnused *********************************************************
**
** Dummy function to prevent compiler warnings
**
** @return [void]
**
** @release 1.0.0
******************************************************************************/

void ajFeatUnused(void)
{
    AjBool knowntag = ajTrue;

    featTagDna(NULL, &knowntag);
    featTagProt(NULL, &knowntag);
    ajFeattagGetNameC(NULL, NULL, &knowntag);
}




/* @funcstatic featFeatureNew *************************************************
**
** Constructor for a feature
**
** @return [AjPFeature] New empty feature
**
** @release 2.1.0
******************************************************************************/

static AjPFeature featFeatureNew(void)
{
    AjPFeature ret;

    AJNEW0(ret);

    ret->Subfeatures = ajListNew() ; /* Assume empty until otherwise needed */
    ret->Tags = ajListNew() ; /* Assume empty until otherwise needed */
    ret->GffTags = ajListNew() ; /* Assume empty until otherwise needed */

    return ret;
}




/* @funcstatic featTableNew ***************************************************
**
** Constructor for a feature table object.
**
** The type is left uninitialised
**
** @return [AjPFeattable] New empty feature table
**
** @release 2.1.0
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
**
** @release 2.1.0
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
**
** @release 6.0.0
** @@
******************************************************************************/

AjBool ajFeatTypeIsCds(const AjPFeature gf)
{
    if(ajStrMatchC(gf->Type, "SO:0000316"))
	return ajTrue;

    return ajFalse;
}




/* @func ajFeatTypeMatchC *****************************************************
**
** Tests whether the feature type matches a given string
** including testing for alternative names
**
** @param [r] gf       [const AjPFeature]  Feature
** @param [r] txt      [const char*]  Feature type name to test
** @return [AjBool] ajTrue on success
**
** @release 6.1.0
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




/* @func ajFeatTypeMatchS *****************************************************
**
** Tests whether the feature type matches a given string
** including testing for alternative names
**
** @param [r] gf       [const AjPFeature]  Feature
** @param [r] str      [const AjPStr]  Feature type name to test
** @return [AjBool] ajTrue on success
**
** @release 6.1.0
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




/* @func ajFeatTypeMatchWildS *************************************************
**
** Tests whether the feature type matches a given wildcard string
** including testing for alternative names
**
** @param [r] gf       [const AjPFeature]  Feature
** @param [r] str      [const AjPStr]  Feature type name to test
** @return [AjBool] ajTrue on success
**
** @release 6.2.0
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
**
** @release 2.1.0
** @@
******************************************************************************/

AjBool ajFeatIsLocal(const AjPFeature gf)
{
    return !(gf->Flags & AJFEATFLAG_REMOTEID);
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
**
** @release 2.1.0
** @@
******************************************************************************/

AjBool ajFeatIsLocalRange(const AjPFeature gf, ajuint start, ajuint end)
{
    if(gf->Flags & AJFEATFLAG_REMOTEID)
	return AJFALSE;

    if(gf->Flags & AJFEATFLAG_LABEL)
	return AJFALSE;

    if(gf->End < start || gf->Start > end)
	return AJFALSE;

    return AJTRUE;
}




/* @func ajFeatIsMultiple *****************************************************
**
** Tests whether the feature is a member of a join, group order or one_of
**
** @param [r] gf       [const AjPFeature]  Feature
** @return [AjBool] Returns AJTRUE if it is a member
**
** @release 2.5.0
** @@
******************************************************************************/

AjBool ajFeatIsMultiple(const AjPFeature gf)
{
    return (gf->Flags & AJFEATFLAG_MULTIPLE);
}




/* @func ajFeatIsCompMult *****************************************************
**
** Tests whether the feature is a member of a complement around a
** multiple (join, etc.)
**
** @param [r] gf       [const AjPFeature]  Feature
** @return [AjBool] Returns AJTRUE if it is a complemented multiple
**
** @release 2.5.0
** @@
******************************************************************************/

AjBool ajFeatIsCompMult(const AjPFeature gf)
{
    return (gf->Flags & AJFEATFLAG_COMPLEMENT_MAIN);
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
**
** @release 2.5.0
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
**
** @release 2.5.0
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
**
** @release 2.5.0
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
**
** @release 2.7.0
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
   /* ajDebug("ajFeattableTrimOff table Start %d End %d Len %d Features %Lu\n",
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
**
** @release 6.3.0
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
    
   /* ajDebug("ajFeattableTrim table Start %d End %d Len %d Features %Lu\n",
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
**
** @release 2.7.0
** @@
******************************************************************************/

AjBool ajFeatTrimOffRange(AjPFeature ft, ajuint ioffset,
			  ajuint begin, ajuint end,
			  AjBool dobegin, AjBool doend)
{
    AjBool ok = ajTrue;

    /* ajDebug("ft flags %x %d..%d %d..%d\n",
	     ft->Flags, ft->Start, ft->End, ft->Start2, ft->End2); */
    
    if(ft->Flags & AJFEATFLAG_REMOTEID) /* feature in another sequence */
	return ajTrue;

    if(ft->Flags & AJFEATFLAG_LABEL) /* label, no positions */
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
	    ft->Flags |= AJFEATFLAG_END_AFTER_SEQ;
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
	    ft->Flags |= AJFEATFLAG_START_BEFORE_SEQ;
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




/* @func ajFeattagIsNote ******************************************************
**
** Tests whether the feature tag is a note (the default feature tag)
**
** @param [r] tag      [const AjPStr]  Feature tag
** @return [AjBool] ajTrue on success
**
** @release 6.0.0
** @@
******************************************************************************/

AjBool ajFeattagIsNote(const AjPStr tag)
{
    if(ajStrMatchC(tag, "note"))
	return ajTrue;

    return ajFalse;
}




/* @func ajFeatExit ***********************************************************
**
** Cleans up feature table internal memory
**
** @return [void]
**
** @release 2.0.0
** @@
******************************************************************************/

void ajFeatExit(void)
{

    ajFeatreadExit();
    ajFeatwriteExit();

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
    ajRegFree(&featRegTagReplace);

    ajTablestrFree(&FeatTypeTableEmbl);
    ajTablestrFree(&FeatTagsTableEmbl);

    ajTablestrFree(&FeatTypeTableGff2);
    ajTablestrFree(&FeatTagsTableGff2);

    ajTablestrFree(&FeatTypeTableGff3);
    ajTablestrFree(&FeatTagsTableGff3);

    ajTablestrFree(&FeatTypeTablePir);
    ajTablestrFree(&FeatTagsTablePir);

    ajTablestrFree(&FeatTypeTableGff2protein);
    ajTablestrFree(&FeatTagsTableGff2protein);

    ajTablestrFree(&FeatTypeTableGff3protein);
    ajTablestrFree(&FeatTagsTableGff3protein);

    ajTablestrFree(&FeatTypeTableSwiss);
    ajTablestrFree(&FeatTagsTableSwiss);

    ajTablestrFree(&FeatTypeTableDna);
    ajTablestrFree(&FeatTagsTableDna);

    ajTablestrFree(&FeatTypeTableProtein);
    ajTablestrFree(&FeatTagsTableProtein);

    ajTablestrFree(&FeatTypeTableRefseqp);
    ajTablestrFree(&FeatTagsTableRefseqp);

    ajTablestrFree(&FeatCategoryTable);

    ajStrDel(&featTypeMiscfeat);
    ajStrDel(&featTypeEmpty);
    ajStrDel(&featDefSource);
    ajStrDel(&featFmtTmp);
    ajStrDel(&featTagTmp);
    ajStrDel(&featTagTmp2);
    ajStrDel(&featValTmp);
    ajStrDel(&featValTmp2);
    ajStrDel(&featTmpStr);
    ajStrDel(&featTagNote);

    ajStrDel(&featTransBegStr);
    ajStrDel(&featTransEndStr);
    ajStrDel(&featTransAaStr);
    ajStrDel(&featTempQry);

    ajStrTokenDel(&featVocabSplit);

    return;
}




/* @func ajFeatWarn ***********************************************************
**
** Formatted write as a warning message.
**
** @param [r] fmt [const char*] Format string
** @param [v] [...] Format arguments.
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajFeatWarn(const char* fmt, ...)
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




#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif




#ifdef AJ_COMPILE_DEPRECATED
/* @obsolete ajFeattableBegin
** @rename ajFeattableGetBegin
*/

__deprecated ajint ajFeattableBegin(const AjPFeattable thys)
{
    return ajFeattableGetBegin(thys);
}




/* @obsolete ajFeattableEnd
** @rename ajFeattableGetEnd
*/

__deprecated ajint ajFeattableEnd(const AjPFeattable thys)
{
    return ajFeattableGetEnd(thys);
}




/* @obsolete ajFeattableLen
** @rename ajFeattableGetLen
*/

__deprecated ajint ajFeattableLen(const AjPFeattable thys)
{
    return ajFeattableGetLen(thys);
}




/* @obsolete ajFeattableSize
** @rename ajFeattableGetSize
*/

__deprecated ajint ajFeattableSize(const AjPFeattable thys)
{
    return ajFeattableGetSize(thys);
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




/* @obsolete ajFeatGetNote
** @rename ajFeatGetNoteS
*/

__deprecated AjBool ajFeatGetNote(const AjPFeature thys, const AjPStr name,
                                  AjPStr* val)
{
    return ajFeatGetNoteS(thys, name, val);
}




/* @obsolete ajFeatTagAddC
** @rename ajFeatTagAddCS
*/

__deprecated AjBool ajFeatTagAddC(AjPFeature thys, const char* tag,
                                  const AjPStr value)
{
    return ajFeatTagAddCS(thys, tag, value);
}




/* @obsolete ajFeatTagAdd
** @rename ajFeatTagAddS
*/

__deprecated AjBool ajFeatTagAdd(AjPFeature thys,
                                 const AjPStr tag, const AjPStr value)
{  
    return ajFeatTagAddSS(thys, tag, value);
}




/* @obsolete ajFeatDefName
** @rename ajFeattableSetDefname
*/

__deprecated void ajFeatDefName(AjPFeattable thys, const AjPStr setname)
{
    ajFeattableSetDefname(thys, setname);
    return;
}




/* @obsolete ajFeatGfftagAddC
** @rename ajFeatGfftagAddCS
*/

__deprecated ajuint ajFeatGfftagAddC(AjPFeature thys,
                                     const char* tag, const AjPStr value)
{
    return ajFeatGfftagAddCS(thys, tag, value);
}




/* @obsolete ajFeatGfftagAdd
** @rename ajFeatGfftagAddSS
*/

__deprecated ajuint ajFeatGfftagAdd(AjPFeature thys,
                       const AjPStr tag, const AjPStr value)
{
    return ajFeatGfftagAddSS(thys, tag, value);
}




/* @obsolete ajFeattableCopy
** @rename ajFeattableNewFtable
*/

__deprecated AjPFeattable ajFeattableCopy(const AjPFeattable orig)
{
    return ajFeattableNewFtable(orig);
}




/* @obsolete ajFeattableCopyLimit
** @rename ajFeattableNewFtableLimit
*/

__deprecated AjPFeattable ajFeattableCopyLimit(const AjPFeattable orig,
                                               ajint limit)
{
    return ajFeattableNewFtableLimit(orig, limit);
}




/* @obsolete ajFeatCopy
** @rename ajFeatNewFeat
*/

__deprecated AjPFeature ajFeatCopy(const AjPFeature orig)
{
    return ajFeatNewFeat(orig);
}




/* @obsolete ajFeatIsChild
** @remove Always returns false, AJFEATFLAG_CHILD was no longer set
*/

__deprecated AjBool ajFeatIsChild(const AjPFeature gf)
{
    (void) gf;
    /*return (gf->Flags & AJFEATFLAG_CHILD);*/
    return ajFalse;
}
#endif

/* @source ajacd **************************************************************
**
** These functions control all aspects of AJAX command definition
** syntax, command line handling and prompting of the user.
**
** The only major functions visible to callers are the initialisation
** function ajAcdInit and a series of retrieval functions ajGetAcdObject
** for each defined object type (integer, sequence, and so on).
**
** Future extensions are planned, including the ability to write out
** the ACD internal structures in a number of other interface formats.
**
** @author Copyright (C) 1998 Peter Rice
** @version $Revision: 1.91 $
** @modified Jun 25 pmr First version
** @modified May 06 2004 Jon Ison Minor mods.
** @modified $Date: 2012/12/07 10:22:55 $ by $Author: rice $
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

#include "ajacd.h"

#include "ajnam.h"
#include "ajfiledata.h"
#include "ajutil.h"
#include "ajfileio.h"
#include "ajsys.h"

#include "ajassem.h"
#include "ajassemread.h"
#include "ajassemwrite.h"
#include "ajfeat.h"
#include "ajfeatread.h"
#include "ajfeatwrite.h"
#include "ajobo.h"
#include "ajoboread.h"
#include "ajobowrite.h"
#include "ajrefseqread.h"
#include "ajrefseqwrite.h"
#include "ajresourceread.h"
#include "ajresourcewrite.h"
#include "ajseq.h"
#include "ajseqtype.h"
#include "ajseqread.h"
#include "ajseqread.h"
#include "ajseqwrite.h"
#include "ajtaxread.h"
#include "ajtaxwrite.h"
#include "ajtextread.h"
#include "ajtextwrite.h"
#include "ajurlread.h"
#include "ajurlwrite.h"
#include "ajvarread.h"
#include "ajvarwrite.h"
#include "ajxmlread.h"
#include "ajxmlwrite.h"

#include <stddef.h>
#include <stdarg.h>
#include <float.h>
#include <limits.h>
#include <math.h>


#ifdef WIN32
#include "win32.h"
#endif



#define DEFDLIST  "."
#define DEFBLOSUM "EBLOSUM62"
#define DEFDNA    "EDNAFULL"

#define ACD_SEQ_BEGIN 0
#define	ACD_SEQ_END 1
#define	ACD_SEQ_LENGTH 2
#define	ACD_SEQ_PROTEIN 3
#define	ACD_SEQ_NUCLEIC 4
#define	ACD_SEQ_NAME 5
#define	ACD_SEQ_USA 6
#define	ACD_SEQ_WEIGHT 7
#define	ACD_SEQ_COUNT 8
#define	ACD_SEQ_MULTICOUNT 9

#define USED_GET 1
#define USED_ACD 2

/*static AjBool acdDebug = 0;*/
/*static AjBool acdDebugSet = 0;*/
static AjBool acdDoHelp = AJFALSE;
static AjBool acdDoLog = AJFALSE;
static AjBool acdDoWarnRange =AJTRUE;
static AjBool acdDoPretty = AJFALSE;
static AjBool acdDoTable = AJFALSE;
static AjBool acdDoTrace = AJFALSE;
static AjBool acdDoValid = AJFALSE;
static AjBool acdDoGalaxy = AJFALSE;
static AjBool acdDoGalaxyNuc = AJFALSE;
static AjBool acdDoGalaxyProt = AJFALSE;
static AjBool acdDoXsd = AJFALSE;
static AjBool acdDoVersion = AJFALSE;
static AjBool acdVerbose = AJFALSE;
static AjBool acdCommandLine = AJTRUE;
static AjBool acdAuto = AJFALSE;
static AjBool acdFilter = AJFALSE;
static AjBool acdOptions = AJFALSE;
static AjBool acdStdout = AJFALSE;
static AjBool acdCodeSet = AJFALSE;
static AjBool acdWrapper = AJFALSE;
static AjPTable acdCodeTable = NULL;
static AjBool acdKnowntypeSet = AJFALSE;
static AjPTable acdKnowntypeTypeTable = NULL;
static AjPTable acdKnowntypeDescTable = NULL;

/* static AjBool acdQualTestSkip = AJFALSE; */

static ajint acdInFile = 0;
static ajint acdInFileSet = AJFALSE;
static ajint acdOutFile = 0;
static ajint acdPromptTry = 2;

static AjPStr acdTmpStr = NULL;
static AjPStr acdTmpStr2 = NULL;

static AjPStr acdPrefName = NULL;
static AjPStr acdPrefToken = NULL;

static AjPStr acdPackName = NULL;
static AjPStr acdPackVersion = NULL;

static AjPStr acdInputName = NULL;
static ajint acdInputLen = 0;
static AjPStr acdInFName = NULL;
static AjPStr acdInTypeFeatName = NULL;
static AjPStr acdInTypeSeqName = NULL;
static AjPStr acdOutFName = NULL;
static AjPStr acdOutFExt = NULL;
static AjPStr acdTmpOutFName = NULL;

static AjPList acdListComments = NULL;
static AjPList acdListCommentsCount = NULL;
static AjPList acdListCommentsColumn = NULL;
static AjPStr acdLogFName = NULL;
static AjPFile acdLogFile = NULL;
static AjPList acdSecList = NULL;
static AjPTable acdSecTable = NULL;
static AjPTable acdExternalTable = NULL;

static AjPStr acdOutFullFName = NULL;
static AjPStr acdPrettyFName = NULL;
static AjPFile acdPrettyFile = NULL;
static ajint acdPrettyMargin = 0;
static ajint acdPrettyIndent = 2;

static AjPStr acdFName = NULL;
static ajint acdLineNum = 0;
static ajint acdCmtWord  = 0;
static ajint acdWordNum = 0;
static ajint acdWordSave = 0;
static ajint acdErrorCount = 0;

static AjPStr acdStrName = NULL;

static AjPStr acdVarAcdProtein = NULL;

static ajint acdUseData = 0;
static ajint acdUseFeatures = 0;
static ajint acdUseInfile = 0;
static ajint acdUseIn = 0;
static ajint acdUseSeq = 0;

static ajint acdUseAlign = 0;
static ajint acdUseFeatout = 0;
static ajint acdUseOut = 0;
static ajint acdUseOutfile = 0;
static ajint acdUseReport = 0;
static ajint acdUseSeqout = 0;
static ajint acdUseGraph = 0;

static ajint acdUseMisc = 0;

static AjPRegexp acdRegQualParse  = NULL;
static AjPRegexp acdRegResolveVar = NULL;
static AjPRegexp acdRegResolveFun = NULL;

static AjPRegexp acdRegExpPlusI = NULL;
static AjPRegexp acdRegExpPlusD = NULL;
static AjPRegexp acdRegExpMinusI = NULL;
static AjPRegexp acdRegExpMinusD = NULL;
static AjPRegexp acdRegExpStarI = NULL;
static AjPRegexp acdRegExpStarD = NULL;
static AjPRegexp acdRegExpDivI = NULL;
static AjPRegexp acdRegExpDivD = NULL;
static AjPRegexp acdRegExpEqualI = NULL;
static AjPRegexp acdRegExpEqualD = NULL;
static AjPRegexp acdRegExpEqualT = NULL;
static AjPRegexp acdRegExpNeI = NULL;
static AjPRegexp acdRegExpNeD = NULL;
static AjPRegexp acdRegExpNeT = NULL;
static AjPRegexp acdRegExpGtI = NULL;
static AjPRegexp acdRegExpGtD = NULL;
static AjPRegexp acdRegExpGtT = NULL;
static AjPRegexp acdRegExpLtI = NULL;
static AjPRegexp acdRegExpLtD = NULL;
static AjPRegexp acdRegExpLtT = NULL;
static AjPRegexp acdRegExpOrI = NULL;
static AjPRegexp acdRegExpOrD = NULL;
static AjPRegexp acdRegExpOrT = NULL;
static AjPRegexp acdRegExpAndI = NULL;
static AjPRegexp acdRegExpAndD = NULL;
static AjPRegexp acdRegExpAndT = NULL;
static AjPRegexp acdRegExpCond = NULL;
static AjPRegexp acdRegExpNot = NULL;
static AjPRegexp acdRegExpOneofCase = NULL;
static AjPRegexp acdRegExpOneofList = NULL;
static AjPRegexp acdRegExpCaseCase = NULL;
static AjPRegexp acdRegExpCaseList = NULL;
static AjPRegexp acdRegExpFilename = NULL;
static AjPRegexp acdRegExpFileExists = NULL;
static AjPRegexp acdRegExpValue = NULL;

static AjPRegexp acdRegVarname  = NULL;
static AjPRegexp acdRegFunction = NULL;
static AjPRegexp acdRegToggle   = NULL;

static AjPStr acdParseReturn  = NULL;
static AjPStr acdReply        = NULL;
static AjPStr acdReplyDef     = NULL;
static AjPStr acdReplyPrompt  = NULL;
static AjPStr acdUserMsg      = NULL;
static AjPStr acdUserReplyDef = NULL;
static AjPStr acdDirectoryDef = NULL;
static AjPStr acdQNameTmp     = NULL;
static AjPStr acdQTypeTmp     = NULL;
static AjPStr acdAttrValTmp   = NULL;
static AjPStr acdQualNameTmp  = NULL;
static AjPStr acdQualNumTmp   = NULL;

static AjBool acdParseQuotes = AJFALSE;
static AjBool acdRegVarInit  = AJFALSE;

static AjPStr  acdEdamPath = NULL;
static AjPFile acdEdamFile = NULL;
static AjPOboData  acdEdam = NULL;

static AjPStr acdGalaxyCmdStr = NULL;

/*
static ajint acdLineCount = 0;
static AjPList acdListCount = NULL;
*/

/* keywords (other than qualifier types) */




/* @enumstatic AcdEStage ******************************************************
**
** ACD level of parsing
**
** @value QUAL_STAGE Qualifier parsing
** @value APPL_STAGE Application parsing
** @value VAR_STAGE Variable parsing
** @value REL_STAGE Relation parsing
** @value SEC_STAGE Section parsing
** @value ENDSEC_STAGE End section parsing
** @value BAD_STAGE Stage parsing error
** @value NO_STAGE  No parsing stage defined
******************************************************************************/

typedef enum
{
    QUAL_STAGE, APPL_STAGE, VAR_STAGE, REL_STAGE,
    SEC_STAGE, ENDSEC_STAGE, BAD_STAGE, NO_STAGE
} AcdEStage;

static AcdEStage acdCurrentStage = NO_STAGE;




/* @enumstatic AcdELevel ******************************************************
**
** ACD item level as defined in the ACD structure
**
** @value ACD_APPL     Application
** @value ACD_PARAM    Positonal parameter
** @value ACD_QUAL     Qualifier
** @value ACD_VAR      Internal variable
** @value ACD_RELATION Standalone relation
** @value ACD_SEC      Section
** @value ACD_ENDSEC   End of section
******************************************************************************/

typedef enum
{
    ACD_APPL,
    ACD_PARAM,
    ACD_QUAL,
    ACD_VAR,
    ACD_RELATION,
    ACD_SEC,
    ACD_ENDSEC
}  AcdELevel;




/* Levels as text, only for use in logging report */

static const char* acdLevel[] =
{
    "APPL", "PARAM", "QUAL", "VAR",
    "IF", "SEC", "ENDSEC"
};

/* Attribute value types */

enum AcdEValtype
{
    VT_APPL, VT_STR, VT_WORD,
    VT_BOOL, VT_INT, VT_FLOAT, VT_CHAR,
    VT_NULL
};


enum AcdEReftype
{
    REF_NONE=0,
    REF_ALL,
    REF_SINGLE
};


/* Attribute value types as text for use in logging report */

static const char* acdValNames[] =
{
    "application", "string", "word",
    "boolean", "integer", "float", "character",
    NULL
};




/* @filesection ajacd ********************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
*/




/* @datasection [none] ACD internals ***********************************
**
** Function is for processing ACD internals.
**
** @nam2rule Acd ACD processing
** @suffix    P  [char*]     Package name provided
** @suffix    V  [char*]     Package version provided
*/




/* @section data definitions **************************************************
**
** Data definitions and function lists
**
** @fdata [none]
** @fcategory misc
**
******************************************************************************/




/* @datastatic AcdPAttrAlias **************************************************
**
** ACD attribute old names and their new equivalents
**
** Commonly used abbreviations of the old names are also included.
**
** @alias AcdSAttrAlias
** @alias AcdOAttrAlias
**
** @attr OldName [const char*] Attribute name in a previous EMBOSS/AJAX release
** @attr NewName [const char*] Current attribute name
** @@
******************************************************************************/

typedef struct AcdSAttrAlias
{
    const char* OldName;
    const char* NewName;
} AcdOAttrAlias;
#define AcdPAttrAlias AcdOAttrAlias*

static AcdOAttrAlias acdAttrAlias[] =
{
    {"required", "standard"},
    {"req", "standard"},
    {"optional", "additional"},
    {"option", "additional"},
    {"opt", "additional"},
    {"standardtype", "knowntype"},
    {NULL, NULL}
};




/* @datastatic AcdPAttr *******************************************************
**
** ACD attribute definition structure
**
** @alias AcdSAttr
** @alias AcdOAttr
**
** @attr Name [const char*] Attribute name
** @attr Type [enum AcdEValtype] Type code
** @attr Multiple [AjBool] True if multiple values are allowed
** @attr Default [const char*] Default value as a string for help
**                             and documentation
** @attr Help [const char*] Descriptive short text for documentation
** @@
******************************************************************************/

typedef struct AcdSAttr
{
    const char* Name;
    enum AcdEValtype Type;
    AjBool Multiple;
    const char* Default;
    const char* Help;
} AcdOAttr;
#define AcdPAttr AcdOAttr*




/* @datastatic AcdPQual *******************************************************
**
** ACD qualifier definition structure
**
** @alias AcdSQual
** @alias AcdOQual
**
** @attr Name [const char*] Qualifier name
** @attr Default [const char*] Default value as a string for help
**                             and documentation
** @attr Type [const char*] Type boolean, integer, float or string
** @attr Help [const char*] Help text for documentation and for -help output
** @@
******************************************************************************/

typedef struct AcdSQual
{
    const char* Name;
    const char* Default;
    const char* Type;
    const char* Help;
} AcdOQual;
#define AcdPQual AcdOQual*




/* @datastatic AcdPTableItem **************************************************
**
** Help table structure
**
** @alias AcdSTableItem
** @alias AcdOTableItem
**
** @attr Qual [AjPStr] Qualifier name
** @attr Type [AjPStr] Qualifier type
** @attr Help [AjPStr] Help text
** @attr Valid [AjPStr] Valid input
** @attr Expect [AjPStr] Expected value(s)
** @attr Title [AjPStr] Section title
** @@
******************************************************************************/

typedef struct AcdSTableItem
{
    AjPStr Qual;
    AjPStr Type;
    AjPStr Help;
    AjPStr Valid;
    AjPStr Expect;
    AjPStr Title;
} AcdOTableItem;
#define AcdPTableItem AcdOTableItem*




/* @datastatic AcdPXsdItem ****************************************************
**
** XSD data structure
**
** @alias AcdSXsdItem
** @alias AcdOXsdItem
**
** @attr Qual [AjPStr] Qualifier name
** @attr Type [AjPStr] Qualifier type
** @attr Annotation [AjPStr] Annotation text
** @attr Relation [AjPStr] Relation attributes text
** @attr Valid [AjPStr] Valid input
** @attr Expect [AjPStr] Expected value(s)
** @attr Required [AjBool] True if a required value
** @attr Optional[AjBool] True if an optional value
** @@
******************************************************************************/

typedef struct AcdSXsdItem
{
    AjPStr Qual;
    AjPStr Type;
    AjPStr Annotation;
    AjPStr Relation;
    AjPStr Valid;
    AjPStr Expect;
    AjBool Required;
    AjBool Optional;
} AcdOXsdItem;
#define AcdPXsdItem AcdOXsdItem*




/* @datastatic AcdPGalaxyItem *************************************************
**
** Galaxy data structure
**
** @alias AcdSGalaxyItem
** @alias AcdOGalaxyItem
**
** @attr Qual [AjPStr] Qualifier name
** @attr Type [AjPStr] Qualifier type
** @attr Prompt [AjPStr] Prompt text
** @attr Annotation [AjPStr] Annotation text
** @attr Relation [AjPStr] Relation attributes text
** @attr Valid [AjPStr] Valid input
** @attr Expect [AjPStr] Expected value(s)
** @attr Required [AjBool] True if a required value
** @attr Optional[AjBool] True if an optional value
** @@
******************************************************************************/

typedef struct AcdSGalaxyItem
{
    AjPStr Qual;
    AjPStr Type;
    AjPStr Prompt;
    AjPStr Annotation;
    AjPStr Relation;
    AjPStr Valid;
    AjPStr Expect;
    AjBool Required;
    AjBool Optional;
} AcdOGalaxyItem;
#define AcdPGalaxyItem AcdOGalaxyItem*




/* @datastatic AcdPAcd ********************************************************
**
** AJAX Command Definition item data.
**
** ACDs are built as an ACD file is parsed, and are processed as a
** list in sequential order.
**
** ACDs contain type information. Some functions will only work on certain
** types of ACDs.
**
** All ACD items are in a list. The base ACD item points to the next,
** and so on until the Next pointer is NULL.
**
** There is currently no destructor. No need for one at present.
**
** @new acdNewQual Creates a qualifier ACD
** @new acdNewQualQual Creates an associated qualifier ACD
** @new acdNewAppl Creates an ACD application
** @new acdNewVar Creates an ACD variable
** @new acdNewAcd General constructor
**
** @attr Next [struct AcdSAcd*] Pointer to next ACD item
** @attr Name [AjPStr] ACD item name
** @attr Token [AjPStr] Command line qualifier (usually the same as Name)
** @attr PNum [ajint] Parameter number, or zero if not a parameter
** @attr Level [AcdELevel] ACD type class (qual/param, var, appl, etc.)
** @attr Type [ajint] Index into acdType or acdKeywords
** @attr NAttr [ajint] Number of ACD type-specific attributes
** @attr AttrStr [AjPStr*] Definitions of ACD type-specific attributes
** @attr SetAttr [AcdPAttr] Definitions of calculated attributes
** @attr SetStr [AjPPStr] Values for the calculated attributes (SetAttr)
** @attr DefStr [AjPPStr] Values for the default attributes
** @attr Defined [AjBool] Set when a value is defined on the commandline
**                        or by an associated qualifier ACD attribute
** @attr UserDefined [AjBool] Set when a value is defined by the user
**                            including replies to prompts
** @attr UserSetNull [AjBool] Set when a value is undefined by the user
**                            with -no before a qualifier name
** @attr Used [ajint] Use count, saved for a possible diagnostic message to
**                    catch ACD items declared and never referenced by the
**                    calling program.
** @attr RefPassed [ajint] Enumerated value for reference 0= not passed,
**                         1= values only passed,
**                         2= values and reference passed
** @attr LineNum [ajint] Source file line number of definition start,
**                       saved for use in diagnostic messages 
** @attr SAttr [ajint] Number of calculated attributes for this ACD type
** @attr Assoc [AjBool] ajTrue if this is an associated qualifier, listed
**                      in the AssocQuals structure of another ACD item
** @attr AssocQuals [struct AcdSAcd*] Associated qualifiers list, or
**                                    NULL if there are none for this ACD type
** @attr StdPrompt [AjPStr] Standard prompt set for some types by
**                          an acdPrompt function
** @attr OrigStr [AjPStr] Original string saved for later processing
** @attr ValStr [AjPStr] Value as a string for printing
** @attr Value [void*] Value as a pointer to the native object to be
**                     returned by an ajAcdGet function call
** @@
******************************************************************************/

typedef struct AcdSAcd
{
    struct AcdSAcd* Next;
    AjPStr Name;
    AjPStr Token;
    ajint PNum;
    AcdELevel Level;
    ajint Type;
    ajint NAttr;
    AjPStr* AttrStr;
    AcdPAttr SetAttr;
    AjPPStr SetStr;
    AjPPStr DefStr;
    AjBool Defined;
    AjBool UserDefined;
    AjBool UserSetNull;
    ajint Used;
    ajint RefPassed;
    ajint LineNum;
    ajint SAttr;
    AjBool Assoc;
    struct AcdSAcd* AssocQuals;
    AjPStr StdPrompt;
    AjPStr OrigStr;
    AjPStr ValStr;
    void* Value;
} AcdOAcd;
#define AcdPAcd AcdOAcd*




/* @datastatic AcdPSection ****************************************************
**
** ACD section definition
**
** @alias AcdSSection
** @alias AcdOSection
**
** @attr Name [const char*] Section name
** @attr Description [const char*] Section description
** @attr Type [const char*] Section type "page"
** @@
******************************************************************************/

typedef struct AcdSSection
{
    const char* Name;
    const char* Description;
    const char* Type;
} AcdOSection;
#define AcdPSection AcdOSection*




AcdOSection acdSecInput[] =
{
    {"input", "Input section", "page"},
    {NULL, NULL, NULL}
};


AcdOSection acdSecRequired[] = 
{
    {"required", "Required section", "page"},
    {NULL, NULL, NULL}
};


AcdOSection acdSecAdditional[] = 
{
    {"additional", "Additional section", "page"},
    {NULL, NULL, NULL}
};


AcdOSection acdSecAdvanced[] = 
{
    {"advanced", "Advanced section", "page"},
    {NULL, NULL, NULL}
};


AcdOSection acdSecOutput[] =
{
    {"output", "Output section", "page"},
    {NULL, NULL, NULL}
};


AcdPSection acdSections[] =
{
    acdSecInput,
    acdSecRequired,
    acdSecAdditional,
    acdSecAdvanced,
    acdSecOutput,
    NULL
};




/* @datastatic AcdPType *******************************************************
**
** ACD data type structure
**
** @alias AcdSType
** @alias AcdOType
**
** @attr Name [const char*] Attribute type name
** @attr Group [const char*] Attribute group name
** @attr Section [AcdPSection] Expected section
** @attr Attr [AcdPAttr] Type-specific attributes
** @attr Quals [AcdPQual] Type-specific associated qualifiers
** @attr TypeSet [void function] Function to set value and prompt user
** @attr HelpSet [void function] Function to set help text
** @attr TypeDel [void function] Function to delete value
** @attr PassByRef [AjBool] Pass by reference - so caller owns the object
** @attr Stdprompt [AjBool] Expect a standard prompt
** @attr Prompt [const AjPStr function] Function to set standard prompt
** @attr UseCount [ajint*] Number of times this type has been used
** @attr UseClassCount [ajint*] Number of times this class of types
**                              has been used
** @attr Valid [const char*] Valid data help message and description for
**                     documentation
** @@
******************************************************************************/

typedef struct AcdSType
{
    const char* Name;
    const char* Group;
    AcdPSection Section;
    AcdPAttr Attr;
    AcdPQual Quals;
    void (*TypeSet)(AcdPAcd thys);
    void (*HelpSet)(const AcdPAcd thys, AjPStr* msg);
    void (*TypeDel)(void**);
    AjBool PassByRef;
    AjBool Stdprompt;
    const AjPStr (*Prompt)(AcdPAcd thys);
    ajint* UseCount;
    ajint* UseClassCount;
    const char* Valid;
} AcdOType;
#define AcdPType AcdOType*

static AjBool* acdParamSet;

static AcdPAcd acdNewCurr = NULL;
static AcdPAcd acdApplAcd = NULL;
static AcdPAcd acdMasterQual = NULL;

/*
AcdOAcd acdList = {NULL, NULL, NULL, 0, ACD_APPL, 0, 0, NULL, 0, NULL,
		   NULL, NULL, NULL, 0, 0, NULL, NULL, NULL};
*/

static AcdPAcd acdList = NULL;
static AcdPAcd acdListLast = NULL;
static AcdPAcd acdListCurr = NULL;
static AcdPAcd acdProcCurr = NULL;
static AcdPAcd acdSetCurr = NULL;

static ajint acdNParam=0;

static AjPTable acdGrpTable = NULL;

static void      acdAmbigApp(AjPStr* pambiglist, const AjPStr str);
static void      acdAmbigAppC(AjPStr* pambiglist, const char* txt);
static void      acdArgsParse(ajint argc, char * const argv[]);
static void      acdArgsScan(ajint argc, char * const argv[]);
static ajint     acdAttrCount(ajint itype);
static ajint     acdAttrKeyCount(ajint ikey);
static ajint     acdAttrListCount(const AcdPAttr attr);
static AjBool    acdAttrResolve(const AcdPAcd thys, const char *attr,
				AjPStr *result);
static AjBool    acdAttrToBool(const AcdPAcd thys,
			       const char *attr, AjBool defval,
			       AjBool *result);
static AjBool    acdAttrToBoolTest(const AcdPAcd thys,
				   const char *attr, AjBool defval,
				   AjBool *result);
static AjBool    acdAttrToDouble(const AcdPAcd thys,
                                 const char *attr, double defval,
                                 double *result);
static AjBool    acdAttrToFloat(const AcdPAcd thys,
				const char *attr, float defval,
				float *result);
static AjBool    acdAttrTest(const AcdPAcd thys, const char *attrib);
static AjBool    acdAttrTestDefined(const AcdPAcd thys, const char *attrib);
static AjBool    acdAttrTestValue(const AcdPAcd thys,const  char *attrib);
static AjBool    acdAttrToChar(const AcdPAcd thys,
			       const char *attr, char defval, char *result);
static AjBool    acdAttrToInt(const AcdPAcd thys,
			      const char *attr, ajint defval, ajint *result);
static AjBool    acdAttrToLong(const AcdPAcd thys,
                               const char *attr, ajlong defval, ajlong *result);
static AjBool    acdAttrToStr(const AcdPAcd thys,
			      const char *attr, const char* defval,
			      AjPStr *result);
static AjBool    acdAttrToUint(const AcdPAcd thys,
			      const char *attr, ajuint defval, ajuint *result);
static const AjPStr acdAttrValue(const AcdPAcd thys, const char *attrib);
static AjBool    acdAttrValueStr(const AcdPAcd thys,
				 const char *attrib, const char* def,
				 AjPStr *str);
static void      acdBadRetry(const AcdPAcd thys);
static void      acdBadVal(const AcdPAcd thys, AjBool required,
			   const char *fmt, ...);
static AjBool    acdCodeDef(const AcdPAcd thys, AjPStr *msg);
static AjBool    acdCodeGet(const AjPStr code, AjPStr *msg);
static void      acdCodeInit(void);
static ajint     acdCountType(const char* type);
static AjBool    acdDataFilename(AjPStr* datafname,
				 const AjPStr name, const AjPStr ext,
				 AjBool nullok);
static AjBool    acdDef(AcdPAcd thys, const AjPStr value);
static void      acdDel(AcdPAcd *Pacd);
static AjBool    acdDefinedEmpty (const AcdPAcd thys);
static const AjPStr acdEdamTest(const AjPStr relation);

__noreturn static void      acdError(const char* fmt, ...);
__noreturn static void      acdErrorAcd(const AcdPAcd thys,
					const char* fmt, ...);

/* expression processing */

static AjBool    acdExpPlus(AjPStr* result, const AjPStr str);
static AjBool    acdExpMinus(AjPStr* result, const AjPStr str);
static AjBool    acdExpStar(AjPStr* result, const AjPStr str);
static AjBool    acdExpDiv(AjPStr* result, const AjPStr str);
static AjBool    acdExpNot(AjPStr* result, const AjPStr str);
static AjBool    acdExpEqual(AjPStr* result, const AjPStr str);
static AjBool    acdExpNotEqual(AjPStr* result, const AjPStr str);
static AjBool    acdExpGreater(AjPStr* result, const AjPStr str);
static AjBool    acdExpLesser(AjPStr* result, const AjPStr str);
static AjBool    acdExpAnd(AjPStr* result, const AjPStr str);
static AjBool    acdExpOr(AjPStr* result, const AjPStr str);
static AjBool    acdExpCond(AjPStr* result, const AjPStr str);
static AjBool    acdExpOneof(AjPStr* result, const AjPStr str);
static AjBool    acdExpCase(AjPStr* result, const AjPStr str);
static AjBool    acdExpFilename(AjPStr* result, const AjPStr str);
static AjBool    acdExpExists(AjPStr* result, const AjPStr str);
static AjBool    acdExpValue(AjPStr* result, const AjPStr str);

static AcdPAcd   acdFindAcd(const AjPStr name, const AjPStr token);
static AcdPAcd   acdFindAcdTest(const AjPStr name, const AjPStr token);
static AcdPAcd   acdFindAssoc(const AcdPAcd thys,
			      const AjPStr name, const AjPStr altname);
static ajint     acdFindAttr(const AcdPAttr attr, const AjPStr attrib);
static ajint     acdFindAttrC(const AcdPAttr attr, const char* attrib);
static AcdPAcd   acdFindItem(const AjPStr item, ajint number);
static ajint     acdFindKeyC(const char* key);
static AcdPAcd   acdFindParam(ajint PNum);
static AcdPAcd   acdFindQual(AjPStr* pqual);
static AcdPAcd   acdFindQualAssoc(const AcdPAcd pa, const AjPStr qual,
				  const AjPStr noqual,
				  const AjPStr master, ajint PNum);
static AcdPAcd   acdFindQualDetail(const AjPStr qual, const AjPStr noqual,
				   const AjPStr master,
				   ajint PNum, ajint *iparam);
static AcdPAcd   acdFindQualMaster(const AjPStr qual, const AjPStr noqual,
				   const AjPStr master,
				   ajint PNum);
static ajint     acdFindType(const AjPStr type);
static ajint     acdFindTypeC(const char* type);
static void      acdFree(void** PPval);
static AjBool    acdFunResolve(AjPStr* result, const AjPStr str);
static AjBool    acdGetAttr(AjPStr* result, const AjPStr name,
			    const AjPStr attrib);
static void*     acdGetValue(const char *token, const char* type);
static void*     acdGetValueRef(const char *token, const char* type);
static void*     acdGetValueSingle(const char *token, const char* type);
static AjBool    acdGetValueAssoc(const AcdPAcd thys, const char *token,
				  AjPStr *result);
static AjBool    acdGetValueAssocDefault(const AcdPAcd thys, const char *token,
                                         AjPStr *result);
static void*     acdGetValueNumC(const char *token, const char* type,
				ajint pnum, AjBool passbyref);
static void*     acdGetValueNumS(const AjPStr token, const char* type,
				ajint pnum, AjBool passbyref);
static const AjPStr acdGetValDefault(const char *token);
static const AjPStr acdGetValStr(const char *token);
static void      acdHelp(void);
static void      acdHelpAppend(const AcdPAcd thys, AjPStr* str, char flag);
static void      acdHelpAssoc(const AcdPAcd thys, AjPStr *str,
			      const char *name);
static void      acdHelpAssocTable(const AcdPAcd thys, AjPList tablist);
static AjBool    acdHelpCodeDef(const AcdPAcd thys, AjPStr *msg);
static void      acdHelpExpect(const AcdPAcd thys, AjBool table, AjPStr *str);
static void      acdHelpGalaxy(AcdPAcd thys, AjPList tablist);
static void      acdHelpGalaxyShow(const AjPList inlist, const AjPList outlist);
static void      acdHelpShow(const AjPStr str, const char* title);
static void      acdHelpTable(const AcdPAcd thys, AjPList tablist);
static void      acdHelpTableShow(const AjPList tablist, const char* title);
static void      acdHelpText(const AcdPAcd thys, AjPStr* msg);
static void      acdHelpValid(const AcdPAcd thys, AjBool table, AjPStr *str);
static AjBool    acdHelpVarResolve(AjPStr* str, const AjPStr src);
static void      acdHelpXsd(const AcdPAcd thys, AjPList tablist);
static void      acdHelpXsdShow(const AjPList inlist, const AjPList outlist);
static AjBool    acdInFilename(AjPStr* infname);
static AjBool    acdInFileSave(const AjPStr infname, const AjPStr seqname,
                               AjBool reset);
static AjBool    acdInTypeFeat(AjPStr* intype);
static AjBool    acdInTypeFeatSave(const AjPStr intype);
static AjBool    acdInTypeFeatSaveC(const char* intype);
static AjBool    acdInTypeSeq(AjPStr* intype);
static AjBool    acdInTypeSeqSave(const AjPStr intype);
static AjBool    acdIsLeftB(AjPList listwords);
static AjBool    acdIsAtype(const AcdPAcd thys);
static AjBool    acdIsParam(const char* arg, AjPStr* param, ajint* iparam,
			    AcdPAcd* acd);
static AjBool    acdIsParamValue(const AjPStr pval);
static ajint     acdIsQual(const char* arg, const char* arg2, ajint *iparam,
			   AjPStr *pqual, AjPStr *pnoqual, AjPStr *pvalue,
			   ajint* number, AjPStr *pmaster, AcdPAcd* acd);
static AjBool    acdIsQtype(const AcdPAcd thys);
static AjBool    acdIsRequired(const AcdPAcd thys);
static AjBool    acdIsRightB(AjPStr* pstr, AjPList listwords);
static AjBool    acdIsStype(const AcdPAcd thys);
static AjBool    acdIsVtype(const AcdPAcd thys);
static const AjPStr acdKnowntypeDesc(const AcdPAcd thys);
static void      acdKnowntypeInit(void);
static void      acdListAttr(const AcdPAttr attr, const AjPPStr valstr,
			     ajint nattr);
static void      acdListPrompt(const AcdPAcd thys);
static void      acdListReport(const char *title);
static AjPStr*   acdListValue(const AcdPAcd thys, ajint min, ajint max,
			      const AjPStr reply);
static void      acdLog(const char *fmt, ...);
static AcdPAcd   acdNewAcd(const AjPStr name, const AjPStr token,
			   ajint itype);
static AcdPAcd   acdNewAcdKey(const AjPStr name, const AjPStr token,
			      ajint ikey);
static AcdPAcd   acdNewAppl(const AjPStr name);
static AcdPAcd   acdNewEndsec(const AjPStr name);
static AcdPAcd   acdNewQual(const AjPStr name, const AjPStr token,
			    AjPStr* type);
static AcdPAcd   acdNewQualQual(const AjPStr name, AjPStr* type);
static AcdPAcd   acdNewRel(const AjPStr name);
static AcdPAcd   acdNewSec(const AjPStr name);
static AcdPAcd   acdNewVar(const AjPStr name);
static ajint     acdNextParam(ajint pnum);
static AjBool    acdNotLeftB(const AjPList listwords);
static AjBool    acdOutDirectory(AjPStr* outdir);
static AjBool    acdOutFilename(AjPStr* outfname, const AjPStr name,
				const AjPStr ext);
static AjBool    acdOutFormatCpdb(const AjPStr format, ajint* iformat);
static AjBool    acdOutFormatData(const AjPStr format, ajint* iformat);
static AjBool    acdOutFormatDiscrete(const AjPStr format, ajint* iformat);
static AjBool    acdOutFormatDistance(const AjPStr format, ajint* iformat);
static AjBool    acdOutFormatFreq(const AjPStr format, ajint* iformat);
static AjBool    acdOutFormatMatrix(const AjPStr forma, ajint* iformatt);
static AjBool    acdOutFormatMatrixf(const AjPStr format, ajint* iformat);
static AjBool    acdOutFormatProperties(const AjPStr format, ajint* iformat);
static AjBool    acdOutFormatScop(const AjPStr format, ajint* iformat);
static AjBool    acdOutFormatTree(const AjPStr format, ajint* iformat);
static void      acdParse(AjPList listwords, AjPList listcount);
static void      acdParseAlpha(AjPList listwords, AjPStr* pword);
static void      acdParseAttributes(const AcdPAcd acd,
				    AjPList listwords);
static void      acdParseName(AjPList listwords, AjPStr* pword);
static AjPStr    acdParseValue(AjPList listwords);
static void      acdPretty(const char *fmt, ...);
static void      acdPrettyClose(void);
static void      acdPrettyComment(const AjPStr comment);
static void      acdPrettyShift(void);
static void      acdPrettyWrap(ajint left, const char *fmt, ...);
static void      acdPrettyUnShift(void);
static void      acdPrintCalcattr(AjPFile outf, const char* acdtype,
				  const AcdOAttr calcattr[]);
static void      acdPrintUsed(void);
static void      acdProcess(void);
static const AjPStr acdPromptAlign(AcdPAcd thys);
static const AjPStr acdPromptAssembly(AcdPAcd thys);
static const AjPStr acdPromptCodon(AcdPAcd thys);
static const AjPStr acdPromptCpdb(AcdPAcd thys);
static const AjPStr acdPromptDatafile(AcdPAcd thys);
static const AjPStr acdPromptDirectory(AcdPAcd thys);
static const AjPStr acdPromptDirlist(AcdPAcd thys);
static const AjPStr acdPromptDiscretestates(AcdPAcd thys);
static const AjPStr acdPromptDistances(AcdPAcd thys);
static const AjPStr acdPromptFeatout(AcdPAcd thys);
static const AjPStr acdPromptFeatures(AcdPAcd thys);
static const AjPStr acdPromptFilelist(AcdPAcd thys);
static const AjPStr acdPromptFrequencies(AcdPAcd thys);
static const AjPStr acdPromptGraph(AcdPAcd thys);
static const AjPStr acdPromptInfile(AcdPAcd thys);
static const AjPStr acdPromptMatrix(AcdPAcd thys);
static const AjPStr acdPromptObo(AcdPAcd thys);
static const AjPStr acdPromptOutassembly(AcdPAcd thys);
static const AjPStr acdPromptOutcodon(AcdPAcd thys);
static const AjPStr acdPromptOutcpdb(AcdPAcd thys);
static const AjPStr acdPromptOutdata(AcdPAcd thys);
static const AjPStr acdPromptOutdir(AcdPAcd thys);
static const AjPStr acdPromptOutdiscrete(AcdPAcd thys);
static const AjPStr acdPromptOutdistance(AcdPAcd thys);
static const AjPStr acdPromptOutfile(AcdPAcd thys);
static const AjPStr acdPromptOutfreq(AcdPAcd thys);
static const AjPStr acdPromptOutmatrix(AcdPAcd thys);
static const AjPStr acdPromptOutobo(AcdPAcd thys);
static const AjPStr acdPromptOutproperties(AcdPAcd thys);
static const AjPStr acdPromptOutrefseq(AcdPAcd thys);
static const AjPStr acdPromptOutresource(AcdPAcd thys);
static const AjPStr acdPromptOutscop(AcdPAcd thys);
static const AjPStr acdPromptOuttaxon(AcdPAcd thys);
static const AjPStr acdPromptOuttext(AcdPAcd thys);
static const AjPStr acdPromptOuttree(AcdPAcd thys);
static const AjPStr acdPromptOuturl(AcdPAcd thys);
static const AjPStr acdPromptOutvariation(AcdPAcd thys);
static const AjPStr acdPromptOutxml(AcdPAcd thys);
static const AjPStr acdPromptPattern(AcdPAcd thys);
static const AjPStr acdPromptProperties(AcdPAcd thys);
static const AjPStr acdPromptRefseq(AcdPAcd thys);
static const AjPStr acdPromptRegexp(AcdPAcd thys);
static const AjPStr acdPromptReport(AcdPAcd thys);
static const AjPStr acdPromptResource(AcdPAcd thys);
static const AjPStr acdPromptScop(AcdPAcd thys);
static const AjPStr acdPromptSeq(AcdPAcd thys);
static const AjPStr acdPromptSeqout(AcdPAcd thys);
static void         acdPromptStandard(AcdPAcd thys, const char* type,
				   ajint* count);
static void         acdPromptStandardS(AcdPAcd thys, const AjPStr str);
static void         acdPromptStandardAlt(AcdPAcd thys, const char* firsttype,
				      const char* type, ajint* count);
static void         acdPromptStandardAppend(AcdPAcd thys, const char* str);
static const AjPStr acdPromptTaxon(AcdPAcd thys);
static const AjPStr acdPromptText(AcdPAcd thys);
static const AjPStr acdPromptTree(AcdPAcd thys);
static const AjPStr acdPromptUrl(AcdPAcd thys);
static const AjPStr acdPromptVariation(AcdPAcd thys);
static const AjPStr acdPromptXml(AcdPAcd thys);
static void      acdQualParse(AjPStr* pqual, AjPStr* pnoqual,
			      AjPStr* pqmaster, ajint* number);
static AjBool    acdQualToBool(const AcdPAcd thys, const char *qual,
			       AjBool defval, AjBool *result, AjPStr* valstr);
static AjBool    acdQualToFloat(const AcdPAcd thys, const char *qual,
 				float defval, ajint precision,
				float *result, AjPStr* valstr);
static AjBool    acdQualToInt(const AcdPAcd thys, const char *qual,
			      ajint defval, ajint *result, AjPStr* valstr);
static AjBool    acdQualToLong(const AcdPAcd thys, const char *qual,
                               ajlong defval, ajlong *result, AjPStr* valstr);
static AjBool    acdQualToSeqbegin(const AcdPAcd thys, const char *qual,
				   ajint defval, ajint *result,
				   AjPStr* valstr);
static AjBool    acdQualToSeqend(const AcdPAcd thys, const char *qual,
				 ajint defval, ajint *result,
				 AjPStr* valstr);
static AjBool     acdRangeTestCalc(const AcdPAcd thys);
static AjPTable  acdReadGroups(void);
static AjPTable  acdReadKeywords(void);
static void      acdReadKnowntype(AjPTable* desctable, AjPTable* infotable);
static void      acdReadSections(AjPTable* typetable, AjPTable* infotable);
static void      acdRegVarDefine(void);
static AjBool    acdReplyInitC(const AcdPAcd thys,
                               const char *defval, AjPStr* reply);
static AjBool    acdReplyInitS(const AcdPAcd thys,
                               const AjPStr defval, AjPStr* reply);
static void      acdReset(void);
static AjBool    acdSet(const AcdPAcd thys, AjPStr* attrib,
			const AjPStr value);
static void      acdSetAll(void);
static AjBool    acdSetDef(AcdPAcd thys, const AjPStr value);
static AjBool    acdSetDefC(AcdPAcd thys, const char* value);
static AjPOutfile acdSetOutType(AcdPAcd thys, const char* type);
static AjBool    acdSetQualAppl(const AcdPAcd thys, AjBool val);
static AjBool    acdSetQualDefBool(AcdPAcd thys,
				   const char* name, AjBool value);
static AjBool    acdSetQualDefInt(AcdPAcd thys,
				  const char* name, ajint value);
static AjBool    acdSetKey(const AcdPAcd thys, AjPStr* attrib,
			   const AjPStr value);
static AjBool    acdSetVarDef(AcdPAcd thys, const AjPStr value);
static void      acdSelectPrompt(const AcdPAcd thys);
static AjPStr*   acdSelectValue(const AcdPAcd thys, ajint min, ajint max,
				const AjPStr reply);
static AcdEStage acdStage(const AjPStr token);
static AcdPAcd   acdTestAssoc(const AcdPAcd thys,
			      const AjPStr name, const AjPStr altname);
static void      acdTestAssocUnknown(const AjPStr name);
static AjBool    acdTestQualC(const char *name);
static void      acdTestUnknown(const AjPStr name, const AjPStr alias);
static AjBool    acdTextFormat(AjPStr* text);
static void      acdTextTrim(AjPStr* text);
static void      acdTokenToLowerS(AjPStr *token, ajint* number);
static AjBool    acdUserGet(AcdPAcd thys, AjPStr* reply);
static AjBool    acdUserGetPrompt(const AcdPAcd thys, const char* assocqual,
				  const char* prompt, AjPStr* reply);
static void      acdUserSavereply(const AcdPAcd thys, const char* assocqual,
				  AjBool userset, const AjPStr reply);
static void      acdValidAppl(const AcdPAcd thys);
static void      acdValidApplGroup(const AjPStr groups);
static void      acdValidApplKeywords(const AjPStr groups);
static void      acdValidKnowntype(const AcdPAcd thys);
static void      acdValidRelation(const AcdPAcd thys);
static void      acdValidSection(const AcdPAcd thys);
static void      acdValidSectionFull(AjPStr* secname);
static AjBool    acdValidSectionMatch(const char* secname);
static void      acdValidQual(AcdPAcd thys);
static AjBool    acdValIsBool(const char* arg);
static AjBool    acdVarResolve(AjPStr* str);
static AjBool    acdVarSimple(const AjPStr var, AjPStr* varname);
static AjBool    acdVarSimpleResolve(const AjPStr var, AjPStr* newvar);
static AjBool    acdVarSplit(const AjPStr var, AjPStr* name, AjPStr* attrname);
static AjBool    acdVarTest(const AjPStr var);
static AjBool    acdVarTestValid(const AjPStr var, AjBool* toggle);
static AjBool    acdVocabCheck(const AjPStr str, const char** vocab);
static void      acdWarn(const char* fmt, ...);
static void      acdWarnObsolete(const AjPStr str);
static AjBool    acdWordNext(AjPList listwords, AjPStr* pword);
static AjBool    acdWordNextLower(AjPList listwords, AjPStr* pword);
static AjBool    acdWordNextName(AjPList listwords, AjPStr* pword);




/* @datastatic AcdPExpList ****************************************************
**
** Exression list structure for named expressions @plus etc.
**
** @alias AcdSExpList
** @alias AcdOExpList
**
** @attr Name [const char*] Expression name
** @attr Func [AjBool function] Function to evaluate expression
** @@
******************************************************************************/

typedef struct AcdSExpList
{
    const char* Name;
    AjBool (*Func) (AjPStr *result, const AjPStr str);
} AcdOExpList;
#define AcdPExpList AcdOExpList*




/* @funclist acdExpList *******************************************************
**
** Functions for processing expressions in ACD dependencies
**
******************************************************************************/

static AcdOExpList explist[] =
{
    {"plus", &acdExpPlus},
    {"minus", &acdExpMinus},
    {"star", &acdExpStar},
    {"div", &acdExpDiv},
    {"not", &acdExpNot},
    {"equal", &acdExpEqual},
    {"notequal", &acdExpNotEqual},
    {"greater", &acdExpGreater},
    {"lesser", &acdExpLesser},
    {"or", &acdExpOr},
    {"and", &acdExpAnd},
    {"cond", &acdExpCond},
    {"oneof", &acdExpOneof},
    {"case", &acdExpCase},
    {"filename", &acdExpFilename},
    {"exists", &acdExpExists},
    {"value", &acdExpValue},
    {NULL, NULL}
};




/* Dummy model routine for new data types - but these must not be static
   and will be defined in ajacd.h instead */

/*static void*  ajAcdGetXxxx (const char *token);*/

static void acdHelpExpectCodon(const AcdPAcd thys, AjBool table, AjPStr* str);
static void acdHelpExpectDirlist(const AcdPAcd thys, AjBool table,
				 AjPStr* str);
static void acdHelpExpectData(const AcdPAcd thys, AjBool table, AjPStr* str);
static void acdHelpExpectFeatout(const AcdPAcd thys, AjBool table,
				 AjPStr* str);
static void acdHelpExpectFeatures(const AcdPAcd thys, AjBool table,
                                  AjPStr* str);
static void acdHelpExpectFilelist(const AcdPAcd thys, AjBool table,
				  AjPStr* str);
static void acdHelpExpectFloat(const AcdPAcd thys, AjBool table, AjPStr* str);
static void acdHelpExpectGraph(const AcdPAcd thys, AjBool table, AjPStr* str);
static void acdHelpExpectIn(const AcdPAcd thys, AjBool table, AjPStr* str);
static void acdHelpExpectInt(const AcdPAcd thys, AjBool table, AjPStr* str);
static void acdHelpExpectMatrix(const AcdPAcd thys, AjBool table, AjPStr* str);
static void acdHelpExpectOut(const AcdPAcd thys, AjBool table, AjPStr* str);
static void acdHelpExpectRange(const AcdPAcd thys, AjBool table, AjPStr* str);
static void acdHelpExpectRegexp(const AcdPAcd thys, AjBool table, AjPStr* str);
static void acdHelpExpectSeq(const AcdPAcd thys, AjBool table, AjPStr* str);
static void acdHelpExpectSeqout(const AcdPAcd thys, AjBool table, AjPStr* str);
static void acdHelpExpectString(const AcdPAcd thys, AjBool table, AjPStr* str);

static void acdHelpTextObo(const AcdPAcd thys, AjPStr* str);
static void acdHelpTextResource(const AcdPAcd thys, AjPStr* str);
static void acdHelpTextSeq(const AcdPAcd thys, AjPStr* str);
static void acdHelpTextSeqout(const AcdPAcd thys, AjPStr* str);
static void acdHelpTextTaxon(const AcdPAcd thys, AjPStr* str);
static void acdHelpTextText(const AcdPAcd thys, AjPStr* str);
static void acdHelpTextUrl(const AcdPAcd thys, AjPStr* str);
static void acdHelpTextVariation(const AcdPAcd thys, AjPStr* str);
static void acdHelpTextXml(const AcdPAcd thys, AjPStr* str);

static void acdHelpValidAlign(const AcdPAcd thys, AjBool table, AjPStr* str);
static void acdHelpValidCodon(const AcdPAcd thys, AjBool table, AjPStr* str);
static void acdHelpValidDirlist(const AcdPAcd thys, AjBool table, AjPStr* str);
static void acdHelpValidData(const AcdPAcd thys, AjBool table, AjPStr* str);
static void acdHelpValidFeatout(const AcdPAcd thys, AjBool table, AjPStr* str);
static void acdHelpValidFeatures(const AcdPAcd thys, AjBool table, AjPStr* str);
static void acdHelpValidFilelist(const AcdPAcd thys, AjBool table,
				 AjPStr* str);
static void acdHelpValidFloat(const AcdPAcd thys, AjBool table, AjPStr* str);
static void acdHelpValidGraph(const AcdPAcd thys, AjBool table, AjPStr* str);
static void acdHelpValidIn(const AcdPAcd thys, AjBool table, AjPStr* str);
static void acdHelpValidInt(const AcdPAcd thys, AjBool table, AjPStr* str);
static void acdHelpValidList(const AcdPAcd thys, AjBool table, AjPStr* str);
static void acdHelpValidMatrix(const AcdPAcd thys, AjBool table, AjPStr* str);
static void acdHelpValidOut(const AcdPAcd thys, AjBool table, AjPStr* str);
static void acdHelpValidRange(const AcdPAcd thys, AjBool table, AjPStr* str);
static void acdHelpValidRegexp(const AcdPAcd thys, AjBool table, AjPStr* str);
static void acdHelpValidReport(const AcdPAcd thys, AjBool table, AjPStr* str);
static void acdHelpValidSelect(const AcdPAcd thys, AjBool table, AjPStr* str);
static void acdHelpValidSeq(const AcdPAcd thys, AjBool table, AjPStr* str);
static void acdHelpValidSeqout(const AcdPAcd thys, AjBool table, AjPStr* str);
static void acdHelpValidString(const AcdPAcd thys, AjBool table, AjPStr* str);

static AjBool acdKnownValueList(const AcdPAcd thys, AjPStr* value);
static AjBool acdKnownValueSelect(const AcdPAcd thys, AjPStr* value);
static AjBool acdResourceList(const AcdPAcd thys,
			      const AjPStr list, AjPStr* value);

/* Type-specific routines to prompt user and set the value.  each new
** type requires one of these routines */

static void acdDelAlign(void** PPval);
static void acdDelAssembly(void** PPval);
static void acdDelCod(void** PPval);
static void acdDelDir(void** PPval);
static void acdDelDirout(void** PPval);
static void acdDelFeattabOut(void** PPval);
static void acdDelFeattable(void** PPval);
static void acdDelFile(void** PPval);
static void acdDelFloat(void** PPval);
static void acdDelList(void** PPval);
static void acdDelMatrix(void** PPval);
static void acdDelMatrixf(void** PPval);
static void acdDelObo(void** PPval);
static void acdDelOutfile(void** PPval);
static void acdDelPatlist(void** PPval);
static void acdDelPhyloDist(void** PPval);
static void acdDelPhyloFreq(void** PPval);
static void acdDelPhyloProp(void** PPval);
static void acdDelPhyloState(void** PPval);
static void acdDelPhyloTree(void** PPval);
static void acdDelRange(void** PPval);
static void acdDelRefseq(void** PPval);
static void acdDelReg(void** PPval);
static void acdDelReport(void** PPval);
static void acdDelResource(void** PPval);
static void acdDelSeq(void** PPval);
static void acdDelSeqall(void** PPval);
static void acdDelSeqout(void** PPval);
static void acdDelSeqset(void** PPval);
static void acdDelSeqsetArray(void** PPval);
static void acdDelStr(void** PPval);
static void acdDelStrArray(void** PPval);
static void acdDelTaxon(void** PPval);
static void acdDelText(void** PPval);
static void acdDelUrl(void** PPval);
static void acdDelVariation(void** PPval);
static void acdDelXml(void** PPval);

static void acdSetXxxx(AcdPAcd thys);
static void acdSetAppl(AcdPAcd thys);
static void acdSetEndsec(AcdPAcd thys);
static void acdSetRel(AcdPAcd thys);
static void acdSetSec(AcdPAcd thys);
static void acdSetVar(AcdPAcd thys);
static void acdSetAlign(AcdPAcd thys);
static void acdSetArray(AcdPAcd thys);
static void acdSetAssembly(AcdPAcd thys);
static void acdSetBoolean(AcdPAcd thys);
static void acdSetCodon(AcdPAcd thys);
static void acdSetCpdb(AcdPAcd thys);
static void acdSetDirlist(AcdPAcd thys);
static void acdSetDatafile(AcdPAcd thys);
static void acdSetDirectory(AcdPAcd thys);
static void acdSetDiscretestates(AcdPAcd thys);
static void acdSetDistances(AcdPAcd thys);
static void acdSetFeatout(AcdPAcd thys);
static void acdSetFeatures(AcdPAcd thys);
static void acdSetFilelist(AcdPAcd thys);
static void acdSetFloat(AcdPAcd thys);
static void acdSetFrequencies(AcdPAcd thys);
static void acdSetGraph(AcdPAcd thys);
static void acdSetGraphxy(AcdPAcd thys);
static void acdSetInt(AcdPAcd thys);
static void acdSetInfile(AcdPAcd thys);
static void acdSetList(AcdPAcd thys);
static void acdSetMatrix(AcdPAcd thys);
static void acdSetMatrixf(AcdPAcd thys);
static void acdSetObo(AcdPAcd thys);
static void acdSetOutassembly(AcdPAcd thys);
static void acdSetOutcodon(AcdPAcd thys);
static void acdSetOutcpdb(AcdPAcd thys);
static void acdSetOutdata(AcdPAcd thys);
static void acdSetOutdir(AcdPAcd thys);
static void acdSetOutdiscrete(AcdPAcd thys);
static void acdSetOutdistance(AcdPAcd thys);
static void acdSetOutfile(AcdPAcd thys);
static void acdSetOutfreq(AcdPAcd thys);
static void acdSetOutmatrix(AcdPAcd thys);
static void acdSetOutmatrixf(AcdPAcd thys);
static void acdSetOutobo(AcdPAcd thys);
static void acdSetOutproperties(AcdPAcd thys);
static void acdSetOutrefseq(AcdPAcd thys);
static void acdSetOutresource(AcdPAcd thys);
static void acdSetOutscop(AcdPAcd thys);
static void acdSetOuttaxon(AcdPAcd thys);
static void acdSetOuttext(AcdPAcd thys);
static void acdSetOuttree(AcdPAcd thys);
static void acdSetOuturl(AcdPAcd thys);
static void acdSetOutvariation(AcdPAcd thys);
static void acdSetOutxml(AcdPAcd thys);
static void acdSetPattern(AcdPAcd thys);
static void acdSetProperties(AcdPAcd thys);
static void acdSetRange(AcdPAcd thys);
static void acdSetRefseq(AcdPAcd thys);
static void acdSetRegexp(AcdPAcd thys);
/*static void acdSetRegions(AcdPAcd thys);*/
static void acdSetReport(AcdPAcd thys);
static void acdSetResource(AcdPAcd thys);
static void acdSetScop(AcdPAcd thys);
static void acdSetSelect(AcdPAcd thys);
static void acdSetSeq(AcdPAcd thys);
static void acdSetSeqall(AcdPAcd thys);
static void acdSetSeqout(AcdPAcd thys);
static void acdSetSeqoutall(AcdPAcd thys);
static void acdSetSeqoutset(AcdPAcd thys);
static void acdSetSeqset(AcdPAcd thys);
static void acdSetSeqsetall(AcdPAcd thys);
static void acdSetString(AcdPAcd thys);
static void acdSetTaxon(AcdPAcd thys);
static void acdSetText(AcdPAcd thys);
static void acdSetToggle(AcdPAcd thys);
static void acdSetTree(AcdPAcd thys);
static void acdSetUrl(AcdPAcd thys);
static void acdSetVariation(AcdPAcd thys);
static void acdSetXml(AcdPAcd thys);

/*
** Known item types
**
** Each has 2 functions, used below in the definition of "types"
** The first is global, and passes the results to the application.
** The other is static and are used in command line and user management
**
** The argument definition is the same for each.
** Items have a type and undefined pointers to the actual data.
** Each function knows the structure they must use, and the validations
** needed.
**
** Type   ajAcdGetType : globally available. returns the value
**
** ajBool acdSetType   : sets the value using the default value (if any)
**
*/

/* Default attributes available for all types */

static ajint nDefAttr = 18;

enum AcdEDef
{
    DEF_DEFAULT,
    DEF_INFO,
    DEF_PROMPT,
    DEF_CODE,
    DEF_HELP,
    DEF_PARAMETER,
    DEF_STANDARD,
    DEF_ADDITIONAL,
    DEF_MISSING,
    DEF_VALID,
    DEF_EXPECTED,
    DEF_NEEDED,
    DEF_KNOWNTYPE,
    DEF_RELATIONS,
    DEF_OUTPUTMODIFIER,
    DEF_STYLE,
    DEF_QUALIFIER,
    DEF_TEMPLATE,
    DEF_COMMENT
};


AcdOAttr acdAttrDef[] =
{
    {"default", VT_STR, AJFALSE, "",
	 "Default value"},
    {"information", VT_STR, AJFALSE, "",
	 "Information for menus etc., and default prompt"},
    {"prompt", VT_STR, AJFALSE, "",
	 "Prompt (if information string is unclear)"},
    {"code", VT_STR, AJFALSE, "",
	 "Code name for information/prompt to be looked up in standard table"},
    {"help", VT_STR, AJFALSE, "",
	 "Text for help documentation"},
    {"parameter", VT_BOOL, AJFALSE, "N",
	 "Command line parameter. "
	     "Can be on the command line with no qualifier name. "
		 "Implies 'standard' qualifier"},
    {"standard", VT_BOOL, AJFALSE, "N",
	 "Standard qualifier, value required. Interactive prompt if missing"},
    {"additional", VT_BOOL, AJFALSE, "N",
	 "Additional qualifier. "
	     "Value required if -options is on the command line, "
		 "or set by default"},
    {"missing", VT_BOOL, AJFALSE, "N",
	 "Allow with no value on the command line to set to ''"},
    {"valid", VT_STR, AJFALSE, "",
	 "Help: String description of allowed values for -help output, "
	     "used if the default help is nuclear"},
    {"expected", VT_STR, AJFALSE, "",
	 "Help: String description of the expected value for -help output, "
	     "used if the default help is nuclear"},
    {"needed", VT_BOOL, AJFALSE, "y",
	 "Include in GUI form, "
	     "used to hide options if they are unclear in GUIs"},
    {"knowntype", VT_STR, AJFALSE, "",
	 "Known standard type, "
	     "used to define input and output types for workflows"},
    {"relations", VT_STR, AJTRUE, "",
	 "Relationships between this ACD item and others, "
	     "defined as specially formatted text"},
    {"outputmodifier", VT_BOOL, AJFALSE, "N",
	 "Modifies the output in ways that can break parsers"},
    {"style", VT_STR, AJFALSE, "",
	 "Style for SoapLab's ACD files"},
    {"qualifier", VT_STR, AJFALSE, "",
	 "Qualifier name for SoapLab's ACD files"},
    {"template", VT_STR, AJFALSE, "",
	 "Commandline template for SoapLab's ACD files"},
    {"comment", VT_STR, AJTRUE, "",
	 "Comment for SoapLab's ACD files"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


/* Type-specific attributes
** each must end with "NULL, VT_NULL" to define the end of the list
*/

AcdOAttr acdAttrXxxx[] =
{
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrAppl[] =
{
    {"documentation", VT_STR, AJFALSE, "",
	 "Short description of the application function"},
    {"groups", VT_STR, AJTRUE, "",
	 "Standard application group(s) for wossname and GUIs"},
    {"keywords", VT_STR, AJTRUE, "",
	 "Standard application keyword(s) for wossname and GUIs"},
    {"gui", VT_STR, AJFALSE, "",
	 "Suitability for launching in a GUI"},
    {"batch", VT_STR, AJFALSE,"",
	 "Suitability for running in batch"},
    {"obsolete", VT_STR, AJFALSE,"",
	 "Reason for obsolete status"},
    {"embassy", VT_STR, AJFALSE,"",
	 "EMBASSY package name"},
    {"external", VT_STR, AJTRUE, "",
	 "Third party tool(s) required by this program"},
    {"wrapper", VT_BOOL, AJFALSE, "N",
	 "Wrapper around original command line"},
    {"cpu", VT_STR, AJFALSE, "",
	 "Estimated maximum CPU usage"},
    {"supplier", VT_STR, AJFALSE, "",
	 "Supplier name"},
    {"versionnumber", VT_STR, AJFALSE, "",
	 "Version number"},
    {"nonemboss", VT_STR, AJFALSE, "",
	 "Non-emboss application name for SoapLab"},
    {"executable", VT_STR, AJFALSE,"",
	 "Non-emboss executable for SoapLab"},
    {"template", VT_STR, AJFALSE, "",
	 "Commandline template for SoapLab's ACD files"},
    {"comment", VT_STR, AJFALSE, "",
	 "Comment for SoapLab's ACD files"},
    {"relations", VT_STR, AJTRUE, "",
	 "Relationships between this ACD item and others, "
	     "defined as specially formatted text"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrAlign[] =
{
    {"type", VT_STR, AJFALSE, "",
	 "P:protein or N:nucleotide"},
    {"taglist", VT_STR, AJFALSE, "",
	 "Extra tags to report"},
    {"minseqs", VT_INT, AJFALSE, "1",
	 "Minimum number of sequences"},
    {"maxseqs", VT_INT, AJFALSE, "(INT_MAX)",
	 "Maximum number of sequences"},
    {"multiple", VT_BOOL, AJFALSE, "N",
	 "More than one alignment in one file"},
    {"nulldefault", VT_BOOL, AJFALSE, "N",
	 "Defaults to 'no file'"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrArray[] =
{
    {"minimum", VT_FLOAT, AJFALSE, "(-FLT_MAX)",
	 "Minimum value"},
    {"maximum", VT_FLOAT, AJFALSE, "(FLT_MAX)",
	 "Maximum value"},
    {"increment", VT_FLOAT, AJFALSE, "0",
	 "(Not used by ACD) Increment for GUIs"},
    {"precision", VT_INT, AJFALSE, "3",
	 "(Not used by ACD) Floating precision for GUIs"},
    {"warnrange", VT_BOOL, AJFALSE, "Y",
	 "Warning if values are out of range"},
    {"size", VT_INT, AJFALSE, "1",
	 "Number of values required"},
    {"sum", VT_FLOAT, AJFALSE, "1.0",
	 "Total for all values"},
    {"sumtest", VT_BOOL, AJFALSE, "Y",
         "Test sum of all values"},
    {"trueminimum", VT_BOOL, AJFALSE, "N",
         "If max/min overlap, use minimum"},
    {"failrange", VT_BOOL, AJTRUE, "",
	 "Fail if (calculated) ranges overlap"},
    {"rangemessage", VT_STR, AJFALSE, "",
	 "Failure message if (calculated ranges) overlap"},
    {"tolerance", VT_FLOAT, AJFALSE, "0.01",
	 "Tolerance (sum +/- tolerance) of the total"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrAssembly[] =
{
    {"entry", VT_BOOL, AJFALSE, "N",
	 "Read whole entry text"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrBool[] =
{
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrCodon[] =
{
    {"name", VT_STR, AJFALSE, "Ehum.cut",
	 "Codon table name"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrCpdb[] =
{
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrDatafile[] =
{
    {"name", VT_STR, AJFALSE, "",
	 "Default file base name"},
    {"extension", VT_STR, AJFALSE, "",
	 "Default file extension"},
    {"directory", VT_STR, AJFALSE, "",
	 "Default installed data directory"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrDirectory[] =
{
    {"fullpath", VT_BOOL, AJFALSE, "N",
	 "Require full path in value"},
    {"nulldefault", VT_BOOL, AJFALSE, "N",
	 "Defaults to 'no file'"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrDirlist[] =
{
    {"fullpath", VT_BOOL, AJFALSE, "N",
	 "Require full path in value"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrDiscretestates[] =
{
    {"length", VT_INT, AJFALSE, "0",
	 "Number of discrete state values per set"},
    {"size", VT_INT, AJFALSE, "1",
	 "Number of discrete state set"},
    {"characters", VT_STR, AJFALSE, "01",
	 "Allowed discrete state characters (default is '' "
         "for all non-space characters"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrDistances[] =
{
    {"size", VT_INT, AJFALSE, "1",
	 "Number of rows"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {"missval", VT_BOOL, AJFALSE, "N",
	 "Can have missing values (replicates zero)"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrEndsec[] =
{
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrFeatures[] =
{
    {"type", VT_STR, AJFALSE, "",
	 "Feature type (protein, nucleotide, etc.)"},
    {"entry", VT_BOOL, AJFALSE, "N",
	 "Read whole entry text"},
    {"minreads", VT_INT, AJFALSE, "1",
	 "Minimum number of inputs"},
    {"maxreads", VT_INT, AJFALSE, "(INT_MAX)",
	 "Maximum number of inputs"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrFeatout[] =
{
    {"name", VT_STR, AJFALSE, "",
	 "Default base file name (use of -ofname preferred)"},
    {"extension", VT_STR, AJFALSE, "",
	 "Default file extension (use of -offormat preferred)"},
    {"type", VT_STR, AJFALSE, "",
	 "Feature type (protein, nucleotide, etc.)"},
    {"multiple", VT_BOOL, AJFALSE, "N",
	 "Features for multiple sequences"},
    {"nulldefault", VT_BOOL, AJFALSE, "N",
	 "Defaults to 'no file'"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null UFO as 'no output'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrFilelist[] =
{
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {"binary", VT_BOOL, AJFALSE, "N",
	 "File contains binary data"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrFloat[] =
{
    {"minimum", VT_FLOAT, AJFALSE, "(-FLT_MAX)",
	 "Minimum value"},
    {"maximum", VT_FLOAT, AJFALSE, "(FLT_MAX)",
	 "Maximum value"},
    {"increment", VT_FLOAT, AJFALSE, "1.0",
	 "(Not used by ACD) Increment for GUIs"},
    {"precision", VT_INT, AJFALSE, "3",
	 "Precision for printing values"},
    {"warnrange", VT_BOOL, AJFALSE, "Y",
	 "Warning if values are out of range"},
    {"trueminimum", VT_BOOL, AJFALSE, "N",
         "If max/min overlap, use minimum"},
    {"failrange", VT_BOOL, AJTRUE, "",
	 "Fail if calculated ranges overlap"},
    {"rangemessage", VT_STR, AJFALSE, "",
	 "Failure message if calculated ranges overlap"},
    {"large", VT_BOOL, AJFALSE, "N",
	 "Large values returned as double"},
	{"trueminimum", VT_BOOL, AJFALSE, "N",
	 "If max/min overlap, use minimum"},
   {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrFrequencies[] =
{
    {"length", VT_INT, AJFALSE, "0",
	 "Number of frequency loci/values per set"},
    {"size", VT_INT, AJFALSE, "1",
	 "Number of frequency sets"},
    {"continuous", VT_BOOL, AJFALSE, "N",
	 "Continuous character data only"},
    {"genedata", VT_BOOL, AJFALSE, "N",
	 "Gene frequency data only"},
    {"within", VT_BOOL, AJFALSE, "N",
	 "Continuous data for multiple individuals"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrGraph[] =
{
    {"sequence", VT_BOOL, AJFALSE, "N",
	 "Sequence on x axis"},
    {"nulldefault", VT_BOOL, AJFALSE, "N",
	 "Defaults to 'no file'"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null graph type as 'no graph'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrGraphxy[] =
{
    {"multiple", VT_INT, AJFALSE, "1",
	 "Number of graphs"},
    {"sequence", VT_BOOL, AJFALSE, "N",
	 "Sequence on x axis"},
    {"nulldefault", VT_BOOL, AJFALSE, "N",
	 "Defaults to 'no file'"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null graph type as 'no graph'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrInt[] =
{
    {"minimum", VT_INT, AJFALSE, "(INT_MIN)",
	 "Minimum value"},
    {"maximum", VT_INT, AJFALSE, "(INT_MAX)",
	 "Maximum value"},
    {"increment", VT_INT, AJFALSE, "0",
	 "(Not used by ACD) Increment for GUIs"},
    {"warnrange", VT_BOOL, AJFALSE, "Y",
	 "Warning if values are out of range"},
    {"failrange", VT_BOOL, AJTRUE, "",
	 "Fail if calculated ranges overlap"},
    {"rangemessage", VT_STR, AJFALSE, "",
	 "Failure message if calculated ranges overlap"},
    {"large", VT_BOOL, AJFALSE, "N",
	 "Large values returned as long"},
    {"trueminimum", VT_BOOL, AJFALSE, "N",
	 "If max/min overlap, use minimum"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrInfile[] =
{
    {"directory", VT_STR, AJFALSE, "",
	 "Default directory"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {"trydefault", VT_BOOL, AJFALSE, "N",
	 "Default filename may not exist if nullok is true"},
    {"binary", VT_BOOL, AJFALSE, "N",
	 "File contains binary data"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrList[] =
{
    {"minimum", VT_INT, AJFALSE, "1",
	 "Minimum number of selections"},
    {"maximum", VT_INT, AJFALSE, "1",
	 "Maximum number of selections"},
    {"button", VT_BOOL, AJFALSE, "N",
	 "(Not used by ACD) Prefer check boxes in GUI"},
    {"casesensitive", VT_BOOL, AJFALSE, "N",
	 "Case sensitive"},
    {"header", VT_STR, AJFALSE, "",
	 "Header description for list"},
    {"delimiter", VT_STR, AJFALSE, ";",
	 "Delimiter for parsing values"},
    {"codedelimiter", VT_STR, AJFALSE, ":",
	 "Delimiter for parsing"},
    {"values", VT_STR, AJFALSE, "",
	 "Codes and values with delimiters"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrMatrix[] =
{
    {"pname", VT_STR, AJFALSE, "EBLOSUM62",
	 "Default name for protein matrix"},
    {"nname", VT_STR, AJFALSE, "EDNAFULL",
	 "Default name for nucleotide matrix"},
    {"protein", VT_BOOL, AJFALSE, "Y",
	 "Protein matrix"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrMatrixf[] =
{
    {"pname", VT_STR, AJFALSE, "EBLOSUM62",
	 "Default name for protein matrix"},
    {"nname", VT_STR, AJFALSE, "EDNAFULL",
	 "Default name for nucleotide matrix"},
    {"protein", VT_BOOL, AJFALSE, "Y",
	 "Protein matrix"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrObo[] =
{
    {"entry", VT_BOOL, AJFALSE, "N",
	 "Read whole entry text"},
    {"minreads", VT_INT, AJFALSE, "1",
	 "Minimum number of inputs"},
    {"maxreads", VT_INT, AJFALSE, "(INT_MAX)",
	 "Maximum number of inputs"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrOutassembly[] =
{
    {"name", VT_STR, AJFALSE, "",
	 "Default file name"},
    {"extension", VT_STR, AJFALSE, "",
	 "Default file extension"},
    {"nulldefault", VT_BOOL, AJFALSE, "N",
	 "Defaults to 'no file'"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrOutcodon[] =
{
    {"name", VT_STR, AJFALSE, "",
	 "Default file name"},
    {"extension", VT_STR, AJFALSE, "",
	 "Default file extension"},
    {"nulldefault", VT_BOOL, AJFALSE, "N",
	 "Defaults to 'no file'"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrOutcpdb[] =
{
    {"nulldefault", VT_BOOL, AJFALSE, "N",
	 "Defaults to 'no file'"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrOutdata[] =
{
    {"type", VT_STR, AJFALSE, "",
	 "Data type"},
    {"nulldefault", VT_BOOL, AJFALSE, "N",
	 "Defaults to 'no file'"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {"binary", VT_BOOL, AJFALSE, "N",
	 "File contains binary data"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrOutdir[] =
{
    {"fullpath", VT_BOOL, AJFALSE, "N",
	 "Require full path in value"},
    {"nulldefault", VT_BOOL, AJFALSE, "N",
	 "Defaults to 'no file'"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {"binary", VT_BOOL, AJFALSE, "N",
	 "Files contain binary data"},
    {"create", VT_BOOL, AJFALSE, "N",
	 "Can create directory if not found"},
    {"temporary", VT_BOOL, AJFALSE, "N",
	 "Scratch directory for temporary files deleted on completion"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrOutdiscrete[] =
{
    {"nulldefault", VT_BOOL, AJFALSE, "N",
	 "Defaults to 'no file'"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrOutdistance[] =
{
    {"nulldefault", VT_BOOL, AJFALSE, "N",
	 "Defaults to 'no file'"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrOutfile[] =
{
    {"name", VT_STR, AJFALSE, "",
	 "Default file name"},
    {"extension", VT_STR, AJFALSE, "",
	 "Default file extension"},
    {"append", VT_BOOL, AJFALSE, "N",
	 "Append to an existing file"},
    {"nulldefault", VT_BOOL, AJFALSE, "N",
	 "Defaults to 'no file'"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {"binary", VT_BOOL, AJFALSE, "N",
	 "File contains binary data"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrOutfreq[] =
{
    {"nulldefault", VT_BOOL, AJFALSE, "N",
	 "Defaults to 'no file'"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrOutmatrix[] =
{
    {"nulldefault", VT_BOOL, AJFALSE, "N",
	 "Defaults to 'no file'"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrOutmatrixf[] =
{
    {"nulldefault", VT_BOOL, AJFALSE, "N",
	 "Defaults to 'no file'"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrOutobo[] =
{
    {"name", VT_STR, AJFALSE, "",
	 "Default file name"},
    {"extension", VT_STR, AJFALSE, "",
	 "Default file extension"},
    {"nulldefault", VT_BOOL, AJFALSE, "N",
	 "Defaults to 'no file'"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrOutproperties[] =
{
    {"nulldefault", VT_BOOL, AJFALSE, "N",
	 "Defaults to 'no file'"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrOutrefseq[] =
{
    {"name", VT_STR, AJFALSE, "",
	 "Default file name"},
    {"extension", VT_STR, AJFALSE, "",
	 "Default file extension"},
    {"nulldefault", VT_BOOL, AJFALSE, "N",
	 "Defaults to 'no file'"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrOutresource[] =
{
    {"name", VT_STR, AJFALSE, "",
	 "Default file name"},
    {"extension", VT_STR, AJFALSE, "",
	 "Default file extension"},
    {"nulldefault", VT_BOOL, AJFALSE, "N",
	 "Defaults to 'no file'"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrOutscop[] =
{
    {"nulldefault", VT_BOOL, AJFALSE, "N",
	 "Defaults to 'no file'"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrOuttaxon[] =
{
    {"name", VT_STR, AJFALSE, "",
	 "Default file name"},
    {"extension", VT_STR, AJFALSE, "",
	 "Default file extension"},
    {"nulldefault", VT_BOOL, AJFALSE, "N",
	 "Defaults to 'no file'"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrOuttext[] =
{
    {"name", VT_STR, AJFALSE, "",
	 "Default file name"},
    {"extension", VT_STR, AJFALSE, "",
	 "Default file extension"},
    {"nulldefault", VT_BOOL, AJFALSE, "N",
	 "Defaults to 'no file'"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrOuttree[] =
{
    {"name", VT_STR, AJFALSE, "",
	 "Default file name"},
    {"extension", VT_STR, AJFALSE, "",
	 "Default file extension"},
    {"nulldefault", VT_BOOL, AJFALSE, "N",
	 "Defaults to 'no file'"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrOuturl[] =
{
    {"name", VT_STR, AJFALSE, "",
	 "Default file name"},
    {"extension", VT_STR, AJFALSE, "",
	 "Default file extension"},
    {"nulldefault", VT_BOOL, AJFALSE, "N",
	 "Defaults to 'no file'"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrOutvariation[] =
{
    {"name", VT_STR, AJFALSE, "",
	 "Default file name"},
    {"extension", VT_STR, AJFALSE, "",
	 "Default file extension"},
    {"nulldefault", VT_BOOL, AJFALSE, "N",
	 "Defaults to 'no file'"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrOutxml[] =
{
    {"name", VT_STR, AJFALSE, "",
	 "Default file name"},
    {"extension", VT_STR, AJFALSE, "",
	 "Default file extension"},
    {"nulldefault", VT_BOOL, AJFALSE, "N",
	 "Defaults to 'no file'"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};



AcdOAttr acdAttrPattern[] =
{
    {"minlength", VT_INT, AJFALSE, "1",
	 "Minimum pattern length"},
    {"maxlength", VT_INT, AJFALSE, "(INT_MAX)",
	 "Maximum pattern length"},
    {"maxsize", VT_INT, AJFALSE, "(INT_MAX)",
	 "Maximum number of patterns"},
    {"upper", VT_BOOL, AJFALSE, "N",
	 "Convert to upper case"},
    {"lower", VT_BOOL, AJFALSE, "N",
	 "Convert to lower case"},
    {"type", VT_STR, AJFALSE, "string",
	 "Type (nucleotide, protein)"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrProperties[] =
{
    {"length", VT_INT, AJFALSE, "0",
	 "Number of property values per set"},
    {"size", VT_INT, AJFALSE, "1",
	 "Number of property sets"},
    {"characters", VT_STR, AJFALSE, "",
	 "Allowed property characters (default is '' for all non-space)"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrRange[] =
{
    {"minimum", VT_INT, AJFALSE, "1",
	 "Minimum value"},
    {"maximum", VT_INT, AJFALSE, "(INT_MAX)",
	 "Maximum value"},
    {"trueminimum", VT_BOOL, AJFALSE, "N",
         "If max/min overlap, use minimum"},
    {"warnrange", VT_BOOL, AJFALSE, "",
	 "Warning if values are out of range"},
    {"failrange", VT_BOOL, AJTRUE, "",
	 "Fail if calculated ranges overlap"},
    {"rangemessage", VT_STR, AJFALSE, "",
	 "Failure message if calculated ranges overlap"},
    {"size", VT_INT, AJFALSE, "0",
	 "Exact number of values required"},
    {"minsize", VT_INT, AJFALSE, "0",
	 "Minimum number of values required"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrRegexp[] =
{
    {"minlength", VT_INT, AJFALSE, "1",
	 "Minimum pattern length"},
    {"maxlength", VT_INT, AJFALSE, "(INT_MAX)",
	 "Maximum pattern length"},
    {"maxsize", VT_INT, AJFALSE, "(INT_MAX)",
	 "Maximum number of patterns"},
    {"upper", VT_BOOL, AJFALSE, "N",
	 "Convert to upper case"},
    {"lower", VT_BOOL, AJFALSE, "N",
	 "Convert to lower case"},
    {"type", VT_STR, AJFALSE, "string",
	 "Type (string, nucleotide, protein)"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrRel[] =
{
    {"relations", VT_STR, AJTRUE, "",
	 "Relationships between this ACD item and others, "
	     "defined as specially formatted text"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrReport[] =
{
    {"type", VT_STR, AJFALSE, "",
	 "P:protein or N:nucleotide"},
    {"taglist", VT_STR, AJFALSE, "",
	 "Extra tag names to report"},
    {"multiple", VT_BOOL, AJFALSE, "N",
	 "Multiple sequences in one report"},
    {"precision", VT_INT, AJFALSE, "3",
	 "Score precision"},
    {"nulldefault", VT_BOOL, AJFALSE, "N",
	 "Defaults to 'no file'"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrRefseq[] =
{
    {"entry", VT_BOOL, AJFALSE, "N",
	 "Read whole entry text"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrResource[] =
{
    {"entry", VT_BOOL, AJFALSE, "N",
	 "Read whole entry text"},
    {"minreads", VT_INT, AJFALSE, "1",
	 "Minimum number of inputs"},
    {"maxreads", VT_INT, AJFALSE, "(INT_MAX)",
	 "Maximum number of inputs"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrScop[] =
{
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrSec[] =
{
    {"information", VT_STR, AJFALSE, "",
	 "(Not used by ACD) Section description"},
    {"type", VT_STR, AJFALSE, "",
	 "(Not used by ACD) Type (frame, page)"},
    {"comment", VT_STR, AJFALSE, "",
	 "(Not used by ACD) Free text comment"},
    {"border", VT_INT, AJFALSE, "1",
	 "(Not used by ACD) Border width"},
    {"side", VT_STR, AJFALSE, "",
	 "(Not used by ACD) Side (top, bottom, left, right) "
	 "for type:frame"},
    {"folder", VT_STR, AJFALSE, "",
	 "(Not used by ACD) Folder name for type:page"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrSelect[] =
{
    {"minimum", VT_INT, AJFALSE, "1",
	 "Minimum number of selections"},
    {"maximum", VT_INT, AJFALSE, "1",
	 "Maximum number of selections"},
    {"button", VT_BOOL, AJFALSE, "N",
	 "(Not used by ACD) Prefer radio buttons in GUI"},
    {"casesensitive", VT_BOOL, AJFALSE, "N",
	 "Case sensitive matching"},
    {"header", VT_STR, AJFALSE, "",
	 "Header description for selection list"},
    {"delimiter", VT_STR, AJFALSE, ":",
	 "Delimiter for parsing values"},
    {"values", VT_STR, AJFALSE, "",
	 "Values with delimiters"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrSeq[] =
{
    {"type", VT_STR, AJFALSE, "",
	 "Input sequence type (protein, gapprotein, etc.)"},
    {"features", VT_BOOL, AJFALSE, "N",
	 "Read features if any"},
    {"entry", VT_BOOL, AJFALSE, "N",
	 "Read whole entry text"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrSeqall[] =
{
    {"type", VT_STR, AJFALSE, "",
	 "Input sequence type (protein, gapprotein, etc.)"},
    {"features", VT_BOOL, AJFALSE, "N",
	 "Read features if any"},
    {"entry", VT_BOOL, AJFALSE, "N",
	 "Read whole entry text"},
    {"minseqs", VT_INT, AJFALSE, "1",
	 "Minimum number of sequences"},
    {"maxseqs", VT_INT, AJFALSE, "(INT_MAX)",
	 "Maximum number of sequences"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrSeqout[] =
{
    {"name", VT_STR, AJFALSE, "",
	 "Output base name (use of -osname preferred)"},
    {"extension", VT_STR, AJFALSE, "",
	 "Output extension (use of -osextension preferred)"},
    {"features", VT_BOOL, AJFALSE, "N",
	 "Write features if any"},
    {"type", VT_STR, AJFALSE, "",
	 "Output sequence type (protein, gapprotein, etc.)"},
    {"nulldefault", VT_BOOL, AJFALSE, "N",
	 "Defaults to 'no file'"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null USA as 'no output'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrSeqoutall[] =
{
    {"name", VT_STR, AJFALSE, "",
	 "Output base name (use of -osname preferred)"},
    {"extension", VT_STR, AJFALSE, "",
	 "Output extension (use of -osextension preferred)"},
    {"features", VT_BOOL, AJFALSE, "N",
	 "Write features if any"},
    {"type", VT_STR, AJFALSE, "",
	 "Output sequence type (protein, gapprotein, etc.)"},
    {"minseqs", VT_INT, AJFALSE, "1",
	 "Minimum number of sequences"},
    {"maxseqs", VT_INT, AJFALSE, "(INT_MAX)",
	 "Maximum number of sequences"},
    {"nulldefault", VT_BOOL, AJFALSE, "N",
	 "Defaults to 'no file'"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null USA as 'no output'"},
    {"aligned", VT_BOOL, AJFALSE, "N",
         "Sequences are aligned"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrSeqoutset[] =
{
    {"name", VT_STR, AJFALSE, "",
	 "Output base name (use of -osname preferred)"},
    {"extension", VT_STR, AJFALSE, "",
	 "Output extension (use of -osextension preferred)"},
    {"features", VT_BOOL, AJFALSE, "N",
	 "Write features if any"},
    {"type", VT_STR, AJFALSE, "",
	 "Output sequence type (protein, gapprotein, etc.)"},
    {"minseqs", VT_INT, AJFALSE, "1",
	 "Minimum number of sequences"},
    {"maxseqs", VT_INT, AJFALSE, "(INT_MAX)",
	 "Maximum number of sequences"},
    {"nulldefault", VT_BOOL, AJFALSE, "N",
	 "Defaults to 'no file'"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null USA as 'no output'"},
    {"aligned", VT_BOOL, AJFALSE, "N",
	 "Sequences are aligned"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrSeqset[] =
{
    {"type", VT_STR, AJFALSE, "",
	 "Input sequence type (protein, gapprotein, etc.)"},
    {"features", VT_BOOL, AJFALSE, "N",
	 "Read features if any"},
    {"aligned", VT_BOOL, AJFALSE, "N",
	 "Sequences are aligned"},
    {"minseqs", VT_INT, AJFALSE, "1",
	 "Minimum number of sequences"},
    {"maxseqs", VT_INT, AJFALSE, "(INT_MAX)",
	 "Maximum number of sequences"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrSeqsetall[] =
{
    {"type", VT_STR, AJFALSE, "",
	 "Input sequence type (protein, gapprotein, etc.)"},
    {"features", VT_BOOL, AJFALSE, "N",
	 "Read features if any"},
    {"aligned", VT_BOOL, AJFALSE, "N",
	 "Sequences are aligned"},
    {"minseqs", VT_INT, AJFALSE, "1",
	 "Minimum number of sequences"},
    {"maxseqs", VT_INT, AJFALSE, "(INT_MAX)",
	 "Maximum number of sequences"},
    {"minsets", VT_INT, AJFALSE, "1",
	 "Minimum number of sequence sets"},
    {"maxsets", VT_INT, AJFALSE, "(INT_MAX)",
	 "Maximum number of sequence sets"},
    {"nulldefault", VT_BOOL, AJFALSE, "N",
	 "Defaults to 'no file'"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrString[] =
{
    {"minlength", VT_INT, AJFALSE, "0",
	 "Minimum length"},
    {"maxlength", VT_INT, AJFALSE, "(INT_MAX)",
	 "Maximum length"},
    {"pattern", VT_STR, AJFALSE, "",
	 "Regular expression for validation"},
    {"upper", VT_BOOL, AJFALSE, "N",
	 "Convert to upper case"},
    {"lower", VT_BOOL, AJFALSE, "N",
	 "Convert to lower case"},
    {"word", VT_BOOL, AJFALSE, "N",
	 "Disallow whitespace in strings"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrTaxon[] =
{
    {"entry", VT_BOOL, AJFALSE, "N",
	 "Read whole entry text"},
    {"minreads", VT_INT, AJFALSE, "1",
	 "Minimum number of inputs"},
    {"maxreads", VT_INT, AJFALSE, "(INT_MAX)",
	 "Maximum number of inputs"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrText[] =
{
    {"entry", VT_BOOL, AJFALSE, "N",
	 "Read whole entry text"},
    {"minreads", VT_INT, AJFALSE, "1",
	 "Minimum number of inputs"},
    {"maxreads", VT_INT, AJFALSE, "(INT_MAX)",
	 "Maximum number of inputs"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrToggle[] =
{
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrTree[] =
{
    {"size", VT_INT, AJFALSE, "0",
	 "Number of trees (0 means any number)"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrUrl[] =
{
    {"entry", VT_BOOL, AJFALSE, "N",
	 "Read whole entry text"},
    {"minreads", VT_INT, AJFALSE, "1",
	 "Minimum number of inputs"},
    {"maxreads", VT_INT, AJFALSE, "(INT_MAX)",
	 "Maximum number of inputs"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrVariation[] =
{
    {"entry", VT_BOOL, AJFALSE, "N",
	 "Read whole entry text"},
    {"minreads", VT_INT, AJFALSE, "1",
	 "Minimum number of inputs"},
    {"maxreads", VT_INT, AJFALSE, "(INT_MAX)",
	 "Maximum number of inputs"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrXml[] =
{
    {"entry", VT_BOOL, AJFALSE, "N",
	 "Read whole entry text"},
    {"minreads", VT_INT, AJFALSE, "1",
	 "Minimum number of inputs"},
    {"maxreads", VT_INT, AJFALSE, "(INT_MAX)",
	 "Maximum number of inputs"},
    {"nullok", VT_BOOL, AJFALSE, "N",
	 "Can accept a null filename as 'no file'"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


AcdOAttr acdAttrVar[] =
{
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


/* Calculated attributes */

static AcdOAttr acdCalcDiscrete[] =
{
    {"discretelength", VT_INT, AJFALSE, "",
	 "Number of discrete state values per set"},
    {"discretesize", VT_INT, AJFALSE, "",
	 "Number of discrete state sets"},
    {"discretecount", VT_INT, AJFALSE, "",
	 "Number of sets of discrete states"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


static AcdOAttr acdCalcDistances[] =
{
    {"distancecount", VT_INT, AJFALSE, "",
	 "Number of distance matrices"},
    {"distancesize", VT_INT, AJFALSE, "",
	 "Number of distance rows"},
    {"replicates", VT_BOOL, AJFALSE, "",
	 "Replicates data found in input"},
    {"hasmissing", VT_BOOL, AJFALSE, "",
	 "Missing values found (replicates=N)"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


static AcdOAttr acdCalcFeat[] =
{
    {"fbegin", VT_INT, AJFALSE, "(0 if unspecified)",
	 "Start of the features to be used"},
    {"fend", VT_INT, AJFALSE, "(0 if unspecified)",
	 "End of the features to be used"},
    {"flength", VT_INT, AJFALSE, "",
	 "Total length of sequence (fsize is feature count)"},
    {"fprotein", VT_BOOL, AJFALSE, "",
	 "Feature table is protein"},
    {"fnucleic", VT_BOOL, AJFALSE, "",
	 "Feature table is nucleotide"},
    {"fname", VT_STR, AJFALSE, "",
	 "The name of the feature table"},
    {"fsize", VT_INT, AJFALSE, "",
	 "Number of features"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


static AcdOAttr acdCalcFrequencies[] =
{
    {"freqlength", VT_INT, AJFALSE, "",
	 "Number of frequency values per set"},
    {"freqsize", VT_INT, AJFALSE, "",
	 "Number of frequency sets"},
    {"freqloci", VT_INT, AJFALSE, "",
	 "Number of frequency loci"},
    {"freqgenedata", VT_BOOL, AJFALSE, "",
	 "Gene frequency data"},
    {"freqcontinuous", VT_BOOL, AJFALSE, "",
	 "Continuous frequency data"},
    {"freqwithin", VT_BOOL, AJFALSE, "",
	 "Individual within species frequency data"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


static AcdOAttr acdCalcProperties[] =
{
    {"propertylength", VT_INT, AJFALSE, "",
	 "Number of property values per set"},
    {"propertysize", VT_INT, AJFALSE, "",
	 "Number of property sets"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


static AcdOAttr acdCalcRegexp[] =
{
    {"length", VT_INT, AJFALSE, "",
	 "The length of the regular expression"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


static AcdOAttr acdCalcSeq[] =
{
    {"begin", VT_INT, AJFALSE, "",
	 "Start of the sequence used"},
    {"end", VT_INT, AJFALSE, "",
	 "End of the sequence used"},
    {"length", VT_INT, AJFALSE, "",
	  "Total length of the sequence"},
    {"protein", VT_BOOL, AJFALSE, "",
	 "Sequence is protein"},
    {"nucleic", VT_BOOL, AJFALSE, "",
	 "Sequence is nucleotide"},
    {"name", VT_STR, AJFALSE, "",
	 "The name/ID/accession of the sequence"},
    {"usa", VT_STR, AJFALSE, "",
	 "The USA of the sequence"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


static AcdOAttr acdCalcSeqall[] =
{
    {"begin", VT_INT, AJFALSE, "",
	 "Start of the first sequence used"},
    {"end", VT_INT, AJFALSE, "",
	 "End of the first sequence used"},
    {"length", VT_INT, AJFALSE, "",
	 "Total length of the first sequence"},
    {"protein", VT_BOOL, AJFALSE, "",
	 "Sequence is protein"},
    {"nucleic", VT_BOOL, AJFALSE, "",
	 "Sequence is nucleotide"},
    {"name", VT_STR, AJFALSE, "",
	 "The name/ID/accession of the sequence"},
    {"usa", VT_STR, AJFALSE, "",
	 "The USA of the sequence"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


static AcdOAttr acdCalcSeqset[] =
{
    {"begin", VT_INT, AJFALSE, "",
	 "The beginning of the selection of the sequence"},
    {"end", VT_INT, AJFALSE, "",
	 "The end of the selection of the sequence"},
    {"length", VT_INT, AJFALSE, "",
	 "The maximum length of the sequence set"},
    {"protein", VT_BOOL, AJFALSE, "",
	 "Sequence set is protein"},
    {"nucleic", VT_BOOL, AJFALSE, "",
	 "Sequence set is nucleotide"},
    {"name", VT_STR, AJFALSE, "",
	 "The name of the sequence set"},
    {"usa", VT_STR, AJFALSE, "",
	 "The USA of the sequence set"},
    {"totweight", VT_FLOAT, AJFALSE, "",
	 "Total sequence weight for a set"},
    {"count", VT_INT, AJFALSE, "",
	 "Number of sequences in the set"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


static AcdOAttr acdCalcSeqsetall[] =
{
    {"begin", VT_INT, AJFALSE, "",
	 "The beginning of the selection of the sequence"},
    {"end", VT_INT, AJFALSE, "",
	 "The end of the selection of the sequence"},
    {"length", VT_INT, AJFALSE, "",
	 "The maximum length of the sequence set"},
    {"protein", VT_BOOL, AJFALSE, "",
	 "Sequence set is protein"},
    {"nucleic", VT_BOOL, AJFALSE, "",
	 "Sequence set is nucleotide"},
    {"name", VT_STR, AJFALSE, "",
	 "The name of the sequence set"},
    {"usa", VT_STR, AJFALSE, "",
	 "The USA of the sequence set"},
    {"totweight", VT_FLOAT, AJFALSE, "",
	 "Total sequence weight for each set"},
    {"count", VT_INT, AJFALSE, "",
	 "Number of sequences in each set"},
    {"multicount", VT_INT, AJFALSE, "",
	 "Number of sets of sequences"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


static AcdOAttr acdCalcString[] =
{
    {"length", VT_INT, AJFALSE, "",
	 "The length of the string"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};


static AcdOAttr acdCalcTree[] =
{
    {"treecount", VT_INT, AJFALSE, "",
	 "Number of trees"},
    {"speciescount", VT_INT, AJFALSE, "",
	 "Number of species"},
    {"haslengths", VT_BOOL, AJFALSE, "",
	 "Branch lengths defined"},
    {NULL, VT_NULL, AJFALSE, NULL,
	 NULL}
};




/* @datastatic AcdPKey ********************************************************
**
** Keywords data structure for non-ACD types (application, variable, section,
** endsection) data
**
** @alias AcdSKey
** @alias AcdOKey
**
** @attr Name [const char*] Keyword name
** @attr Stage [AcdEStage] Enumerated stage 
** @attr Padding [ajint] Padding to alignment boundary
** @attr Attr [AcdPAttr] Type-specific attributes 
** @attr KeySet [void function] Function to set value and prompt user
** @@
******************************************************************************/

typedef struct AcdSKey
{
    const char* Name;
    AcdEStage Stage;
    ajint Padding;
    AcdPAttr Attr;
    void (*KeySet)(AcdPAcd thys);
} AcdOKey;

#define AcdPKey AcdOKey*




/* @funclist acdKeywords ******************************************************
**
** Processing predefined ACD keywords (application, variable, section,
** endsection)
**
******************************************************************************/

AcdOKey acdKeywords[] =
{
    {"qualifier",   QUAL_STAGE,   0,  NULL,          NULL},
    {"application", APPL_STAGE,   0,  acdAttrAppl,   acdSetAppl},
    {"variable",    VAR_STAGE,    0,  acdAttrVar,    acdSetVar},
    {"relations",   REL_STAGE,    0,  acdAttrRel,    acdSetRel},
    {"section",     SEC_STAGE,    0,  acdAttrSec,    acdSetSec},
    {"endsection",  ENDSEC_STAGE, 0,  acdAttrEndsec, acdSetEndsec},
    {NULL, BAD_STAGE, 0, NULL, NULL}
};




/* @datastatic AcdPOuttype ****************************************************
**
** Output types data structure for utput data types and format validation
**
** @alias AcdSOuttype
** @alias AcdOouttype
**
** @attr Name [const char*] Output type name
** @attr Format [const char*] Default format (needs to be stored elsewhere)
** @attr Type [AjEOutfileType] AJAX Output File enumeration
** @attr Padding [ajint] Padding to alignment boundary
** @attr Prompt [const AjPStr function] Function to define standard prompt
** @attr Outformat [AjBool function] Function to test output format
** @@
******************************************************************************/

typedef struct AcdSOuttype
{
    const char* Name;
    const char* Format;
    AjEOutfileType Type;
    ajint Padding;
    
    const AjPStr (*Prompt)(AcdPAcd thys);
    AjBool (*Outformat)(const AjPStr format, ajint* iformat);

} AcdOOuttype;

#define AcdPOuttype AcdOOuttype*




/* @funclist acdOuttype *******************************************************
**
** Processing output types and their formats
**
******************************************************************************/

AcdOOuttype acdOuttype[] =
{
/*    Name             Format         Type              Padding
         Prompt                  Outformat */
    {"outassembly",   "bam",           ajEOutfileTypeAssembly, 0,
	 acdPromptOutassembly,   ajAssemoutformatFind},
    {"outcodon",      "cut",           ajEOutfileTypeCodon, 0,
	 acdPromptOutcodon,      ajCodoutformatFind},
    {"outcpdb",       "cpdb",          ajEOutfileTypeCPDB, 0,
	 acdPromptOutcpdb,       acdOutFormatCpdb},
    {"outdata",       "text",          ajEOutfileTypeUnknown, 0,
	 acdPromptOutdata,       acdOutFormatData},
    {"outdiscrete",   "phylip",        ajEOutfileTypeDiscrete, 0,
	 acdPromptOutdiscrete,   acdOutFormatDiscrete},
    {"outdistance",   "phylip",        ajEOutfileTypeDistance, 0,
	 acdPromptOutdistance,   acdOutFormatDistance},
    {"outfreq",       "phylip",        ajEOutfileTypeFrequency, 0,
	 acdPromptOutfreq,       acdOutFormatFreq},
    {"outmatrix",     "emboss",        ajEOutfileTypeMatrix, 0,
	 acdPromptOutmatrix,     acdOutFormatMatrix},
    {"outmatrixf",    "emboss",        ajEOutfileTypeMatrixF, 0,
	 acdPromptOutmatrix,     acdOutFormatMatrixf},
    {"outobo",        "obo",           ajEOutfileTypeOBO, 0,
	 acdPromptOutobo,        ajObooutformatFind},
    {"outproperties", "phylip",        ajEOutfileTypeProperties, 0,
	 acdPromptOutproperties, acdOutFormatProperties},
    {"outrefseq",     "embl",          ajEOutfileTypeRefseq, 0,
	 acdPromptOutrefseq,     ajRefseqoutformatFind},
    {"outresource",   "drcat",         ajEOutfileTypeResource, 0,
	 acdPromptOutresource,   ajResourceoutformatFind},
    {"outscop",       "scop",          ajEOutfileTypeSCOP, 0,
	 acdPromptOutscop,       acdOutFormatScop},
    {"outtaxon",      "tax",           ajEOutfileTypeTaxon, 0,
	 acdPromptOuttaxon,      ajTaxoutformatFind},
    {"outtext",       "text",           ajEOutfileTypeText, 0,
	 acdPromptOuttext,       ajTextoutformatFind},
    {"outtree",       "phylip",        ajEOutfileTypeTree, 0,
	 acdPromptOuttree,       acdOutFormatTree},
    {"outurl",        "url",           ajEOutfileTypeURL, 0,
	 acdPromptOuturl,        ajUrloutformatFind},
    {"outvariation",  "vcf",           ajEOutfileTypeVariation, 0,
	 acdPromptOutvariation,  ajVaroutformatFind},
    {"outxml",        "xml",           ajEOutfileTypeXml, 0,
	 acdPromptOutxml,        ajXmloutformatFind},
    {NULL, NULL, ajEOutfileTypeUnknown, 0, NULL, NULL}
};


/* Type-specific associated qualifiers which can be used positionally
** or numbered if tied to a parameter */

/* "qualifier"  "default" "type" */

AcdOQual acdQualAppl[] =	  /* careful: index numbers used in */
				  /* acdSetQualAppl */
{
    {"auto",       "N", "boolean", "Turn off prompts"},
    {"stdout",     "N", "boolean", "Write first file to standard output"},
    {"filter",     "N", "boolean", "Read first file from standard input, "
	                           "write first file to standard output"},
    /* after auto and stdout so it can replace */
    {"options",    "N", "boolean", "Prompt for standard and additional values"},
    {"debug",      "N", "boolean", "Write debug output to program.dbg"},
    /* deprecated - to be removed in a future version */
/* removed in 2.8.0
    {"acdlog",     "N", "boolean", "Write ACD processing log "
                                     "to program.acdlog"},
    {"acdpretty",  "N", "boolean", "Rewrite ACD file as program.acdpretty"},
    {"acdtable",   "N", "boolean", "Write HTML table of options"},
*/

    /* end of deprecated set */
    {"verbose",    "N", "boolean", "Report some/full command line options"},
    {"help",       "N", "boolean", "Report command line options and exit. "
	                           "More information on associated and "
				   "general qualifiers "
				   "can be found with -help -verbose"},
    {"warning",    "Y", "boolean", "Report warnings"},
    {"error",      "Y", "boolean", "Report errors"},
    {"fatal",      "Y", "boolean", "Report fatal errors"},
    {"die",        "Y", "boolean", "Report dying program messages"},
    {"version",    "N", "boolean", "Report version number and exit"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualAlign[] =
{
    {"aformat",    "",  "string",  "Alignment format"},
    {"aextension", "",  "string",  "File name extension"},
    {"adirectory", "",  "string",  "Output directory"},
    {"aname",      "",  "string",  "Base file name"},
    {"awidth",     "0", "integer", "Alignment width"},
    {"aaccshow",   "N", "boolean", "Show accession number in the header"},
    {"adesshow",   "N", "boolean", "Show description in the header"},
    {"ausashow",   "N", "boolean", "Show the full USA in the alignment"},
    {"aglobal",    "N", "boolean", "Show the full sequence in alignment"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualAssembly[] =
{
    {"cbegin",     "0", "integer", "Start of the contig/consensus sequences"},
    {"cend",       "0", "integer", "End of the contig/consensus sequences"},
    {"iformat",    "",  "string",  "Input assembly format"},
    {"iquery",     "",  "string",  "Input query fields or ID list"},
    {"ioffset",    "",  "integer", "Input start position offset"},
    {"idbname",    "",  "string",  "User-provided database name"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualCodon[] =
{
    {"format",    "",  "string",   "Data format"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualCpdb[] =
{
    {"format",     "",  "string",   "Data format"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualDirectory[] =
{
    {"extension",  "",  "string",    "Default file extension"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualDirlist[] =
{
    {"extension",  "",  "string",    "Default file extension"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualFeatures[] =
{
    {"fformat",    "",  "string",  "Features format"},
    {"iquery",     "",  "string",  "Input query fields or ID list"},
    {"ioffset",    "",  "integer", "Input start position offset"},
    {"fopenfile",  "",  "string",  "Features file name"},
    {"fask",       "N", "boolean", "Prompt for begin/end/reverse"},
    {"fbegin",     "0", "integer", "Start of the features to be used"},
    {"fend",       "0", "integer", "End of the features to be used"},
    {"freverse",   "N", "boolean", "Reverse (if DNA)"},
    {"fcircular",  "N", "boolean", "Circular sequence features"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualFeatout[] =
{
    {"offormat",   "",  "string",  "Output feature format"},
    {"ofopenfile", "",  "string",  "Features file name"},
    {"ofextension","",  "string",  "File name extension"},
    {"ofdirectory","",  "string",  "Output directory"},
    {"ofname",     "",  "string",  "Base file name"},
    {"ofsingle",   "N", "boolean", "Separate file for each entry"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualGraph[] =
{
    {"gprompt",   "N", "boolean", "Graph prompting"},
    {"gdesc",     "",  "string",  "Graph description"},
    {"gtitle",    "",  "string",  "Graph title"},
    {"gsubtitle", "",  "string",  "Graph subtitle"},
    {"gxtitle",   "",  "string",  "Graph x axis title"},
    {"gytitle",   "",  "string",  "Graph y axis title"},
    {"goutfile",  "",  "string",  "Output file for non interactive displays"},
    {"gdirectory","",  "string",  "Output directory"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualGraphxy[] =
{
    {"gprompt",   "N", "boolean", "Graph prompting"},
    {"gdesc",     "",  "string",  "Graph description"},
    {"gtitle",    "",  "string",  "Graph title"},
    {"gsubtitle", "",  "string",  "Graph subtitle"},
    {"gxtitle",   "",  "string",  "Graph x axis title"},
    {"gytitle",   "",  "string",  "Graph y axis title"},
    {"goutfile",  "",  "string",  "Output file for non interactive displays"},
    {"gdirectory","",  "string",  "Output directory"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualObo[] =
{
    {"iformat",    "",  "string",  "Input obo format"},
    {"iquery",     "",  "string",  "Input query fields or ID list"},
    {"ioffset",    "",  "integer", "Input start position offset"},
    {"idbname",    "",  "string",  "User-provided database name"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualOutassembly[] =
{
    {"odirectory", "",  "string",  "Output directory"},
    {"oformat",    "",  "string",  "Assembly output format"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualOutcodon[] =
{
    {"odirectory", "",  "string",  "Output directory"},
    {"oformat",    "",  "string",  "Output format specific to this data type"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualOutcpdb[] =
{
    {"odirectory", "",  "string",  "Output directory"},
    {"oformat",    "",  "string",  "Output format specific to this data type"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualOutdata[] =
{
    {"odirectory", "",  "string",  "Output directory"},
    {"oformat",    "",  "string",  "Output format specific to this data type"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualOutdir[] =
{
    {"extension",  "",  "string",    "Default file extension"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualOutdiscrete[] =
{
    {"odirectory", "",  "string",  "Output directory"},
    {"oformat",    "",  "string",  "Output format specific to this data type"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualOutdistance[] =
{
    {"odirectory", "",  "string",  "Output directory"},
    {"oformat",    "",  "string",  "Output format specific to this data type"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualOutfile[] =
{
    {"odirectory", "",  "string",  "Output directory"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualOutfreq[] =
{
    {"odirectory", "",  "string",  "Output directory"},
    {"oformat",    "",  "string",  "Output format specific to this data type"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualOutmatrix[] =
{
    {"odirectory", "",  "string",  "Output directory"},
    {"oformat",    "",  "string",  "Output format specific to this data type"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualOutmatrixf[] =
{
    {"odirectory", "",  "string",  "Output directory"},
    {"oformat",    "",  "string",  "Output format specific to this data type"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualOutobo[] =
{
    {"odirectory", "",  "string",  "Output directory"},
    {"oformat",    "",  "string",  "Ontology term output format"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualOutproperties[] =
{
    {"odirectory", "",  "string",  "Output directory"},
    {"oformat",    "",  "string",  "Output format specific to this data type"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualOutrefseq[] =
{
    {"odirectory", "",  "string",  "Output directory"},
    {"oformat",    "",  "string",  "Referece sequence output format"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualOutresource[] =
{
    {"odirectory", "",  "string",  "Output directory"},
    {"oformat",    "",  "string",  "Data resource output format"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualOutscop[] =
{
    {"odirectory", "",  "string",  "Output directory"},
    {"oformat",    "",  "string",  "Output format specific to this data type"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualOuttaxon[] =
{
    {"odirectory", "",  "string",  "Output directory"},
    {"oformat",    "",  "string",  "Taxonomy output format"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualOuttext[] =
{
    {"odirectory", "",  "string",  "Output directory"},
    {"oformat",    "",  "string",  "Text output format"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualOuttree[] =
{
    {"odirectory", "",  "string",  "Output directory"},
    {"oformat",    "",  "string",  "Output format specific to this data type"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualOuturl[] =
{
    {"odirectory", "",  "string",  "Output directory"},
    {"oformat",    "",  "string",  "URL output format"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualOutvariation[] =
{
    {"odirectory", "",  "string",  "Output directory"},
    {"oformat",    "",  "string",  "Variation output format"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualOutxml[] =
{
    {"odirectory", "",  "string",  "Output directory"},
    {"oformat",    "",  "string",  "Xml output format"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualPattern[] =
{
    {"pformat",    "",  "string",  "File format"},
    {"pmismatch", "0",  "integer", "Pattern mismatch"},
    {"pname",      "",  "string",  "Pattern base name"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualRefseq[] =
{
    {"iformat",    "",  "string",  "Input reference sequence format"},
    {"iquery",     "",  "string",  "Input query fields or ID list"},
    {"ioffset",    "",  "integer", "Input start position offset"},
    {"idbname",    "",  "string",  "User-provided database name"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualRegexp[] =
{
    {"pformat",    "",  "string",  "File format"},
    {"pname",      "",  "string",  "Pattern base name"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualReport[] =
{
    {"rformat",    "",  "string",  "Report format"},
    {"rname",      "",  "string",  "Base file name"},
    {"rextension", "",  "string",  "File name extension"},
    {"rdirectory", "",  "string",  "Output directory"},
    {"raccshow",   "N", "boolean", "Show accession number in the report"},
    {"rdesshow",   "N", "boolean", "Show description in the report"},
    {"rscoreshow", "Y", "boolean", "Show the score in the report"},
    {"rstrandshow","Y", "boolean", "Show the nucleotide strand in the report"},
    {"rusashow",   "N", "boolean", "Show the full USA in the report"},
    {"rmaxall",    "0", "integer", "Maximum total hits to report"},
    {"rmaxseq",    "0", "integer", "Maximum hits to report for one sequence"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualResource[] =
{
    {"iformat",    "",  "string",  "Input resource format"},
    {"iquery",     "",  "string",  "Input query fields or ID list"},
    {"ioffset",    "",  "integer", "Input start position offset"},
    {"idbname",    "",  "string",  "User-provided database name"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualScop[] =
{
    {"format",    "",  "string",   "Data format"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualSeq[] =
{
    {"sbegin",     "0", "integer", "Start of the sequence to be used"},
    {"send",       "0", "integer", "End of the sequence to be used"},
    {"sreverse",   "N", "boolean", "Reverse (if DNA)"},
    {"sask",       "N", "boolean", "Ask for begin/end/reverse"},
    {"snucleotide","N", "boolean", "Sequence is nucleotide"},
    {"sprotein",   "N", "boolean", "Sequence is protein"},
    {"slower",     "N", "boolean", "Make lower case"},
    {"supper",     "N", "boolean", "Make upper case"},
    {"scircular",  "N", "boolean", "Sequence is circular"},
    {"squick",     "N", "boolean", "Read id and sequence only"},
    {"sformat",    "",  "string",  "Input sequence format"},
/*    {"sopenfile",  "", "string",   "Input filename"},*/ /* obsolete */
    {"iquery",     "",  "string",  "Input query fields or ID list"},
    {"ioffset",    "",  "integer", "Input start position offset"},
    {"sdbname",    "",  "string",  "Database name"},
    {"sid",        "",  "string",  "Entryname"},
    {"ufo",        "",  "string",  "UFO features"},
    {"fformat",    "",  "string",  "Features format"},
    {"fopenfile",  "",  "string",  "Features file name"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualSeqall[] =
{
    {"sbegin",     "0", "integer", "Start of each sequence to be used"},
    {"send",       "0", "integer", "End of each sequence to be used"},
    {"sreverse",   "N", "boolean", "Reverse (if DNA)"},
    {"sask",       "N", "boolean", "Ask for begin/end/reverse"},
    {"snucleotide","N", "boolean", "Sequence is nucleotide"},
    {"sprotein",   "N", "boolean", "Sequence is protein"},
    {"slower",     "N", "boolean", "Make lower case"},
    {"supper",     "N", "boolean", "Make upper case"},
    {"scircular",  "N", "boolean", "Sequence is circular"},
    {"squick",     "N", "boolean", "Read id and sequence only"},
    {"sformat",    "",  "string",  "Input sequence format"},
/*    {"sopenfile",  "",  "string",  "Input filename"},*/ /* obsolete */
    {"iquery",     "",  "string",  "Input query fields or ID list"},
    {"ioffset",    "",  "integer", "Input start position offset"},
    {"sdbname",    "",  "string",  "Database name"},
    {"sid",        "",  "string",  "Entryname"},
    {"ufo",        "",  "string",  "UFO features"},
    {"fformat",    "",  "string",  "Features format"},
    {"fopenfile",  "",  "string",  "Features file name"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualSeqset[] =
{
    {"sbegin",     "0", "integer", "Start of each sequence to be used"},
    {"send",       "0", "integer", "End of each sequence to be used"},
    {"sreverse",   "N", "boolean", "Reverse (if DNA)"},
    {"sask",       "N", "boolean", "Ask for begin/end/reverse"},
    {"snucleotide","N", "boolean", "Sequence is nucleotide"},
    {"sprotein",   "N", "boolean", "Sequence is protein"},
    {"slower",     "N", "boolean", "Make lower case"},
    {"supper",     "N", "boolean", "Make upper case"},
    {"scircular",  "N", "boolean", "Sequence is circular"},
    {"squick",     "N", "boolean", "Read id and sequence only"},
    {"sformat",    "",  "string",  "Input sequence format"},
/*    {"sopenfile",  "",  "string",  "Input filename"},*/ /* obsolete */
    {"iquery",     "",  "string",  "Input query fields or ID list"},
    {"ioffset",    "",  "integer", "Input start position offset"},
    {"sdbname",    "",  "string",  "Database name"},
    {"sid",        "",  "string",  "Entryname"},
    {"ufo",        "",  "string",  "UFO features"},
    {"fformat",    "",  "string",  "Features format"},
    {"fopenfile",  "",  "string",  "Features file name"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualSeqsetall[] =
{
    {"sbegin",     "0", "integer", "Start of each sequence to be used"},
    {"send",       "0", "integer", "End of each sequence to be used"},
    {"sreverse",   "N", "boolean", "Reverse (if DNA)"},
    {"sask",       "N", "boolean", "Ask for begin/end/reverse"},
    {"snucleotide","N", "boolean", "Sequence is nucleotide"},
    {"sprotein",   "N", "boolean", "Sequence is protein"},
    {"slower",     "N", "boolean", "Make lower case"},
    {"supper",     "N", "boolean", "Make upper case"},
    {"scircular",  "N", "boolean", "Sequence is circular"},
    {"squick",     "N", "boolean", "Read id and sequence only"},
    {"sformat",    "",  "string",  "Input sequence format"},
/*    {"sopenfile",  "",  "string",  "Input filename"},*/ /* obsolete */
    {"iquery",     "",  "string",  "Input query fields or ID list"},
    {"ioffset",    "",  "integer", "Input start position offset"},
    {"sdbname",    "",  "string",  "Database name"},
    {"sid",        "",  "string",  "Entryname"},
    {"ufo",        "",  "string",  "UFO features"},
    {"fformat",    "",  "string",  "Features format"},
    {"fopenfile",  "",  "string",  "Features file name"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualSeqout[] =
{
    {"osformat",   "",  "string",  "Output seq format"},
    {"osextension","",  "string",  "File name extension"},
    {"osname",     "",  "string",  "Base file name"},
    {"osdirectory","",  "string",  "Output directory"},
    {"osdbname",   "",  "string",  "Database name to add"},
    {"ossingle",   "N", "boolean", "Separate file for each entry"},
    {"oufo",       "",  "string",  "UFO features"},
    {"offormat",   "",  "string",  "Features format"},
    {"ofname",     "",  "string",  "Features file name"},
    {"ofdirectory","",  "string",  "Output directory"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualSeqoutset[] =
{
    {"osformat",   "",  "string",  "Output seq format"},
    {"osextension","",  "string",  "File name extension"},
    {"osname",     "",  "string",  "Base file name"},
    {"osdirectory","",  "string",  "Output directory"},
    {"osdbname",   "",  "string",  "Database name to add"},
    {"ossingle",   "N", "boolean", "Separate file for each entry"},
    {"oufo",       "",  "string",  "UFO features"},
    {"offormat",   "",  "string",  "Features format"},
    {"ofname",     "",  "string",  "Features file name"},
    {"ofdirectory","",  "string",  "Output directory"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualSeqoutall[] =
{
    {"osformat",   "",  "string",  "Output seq format"},
    {"osextension","",  "string",  "File name extension"},
    {"osname",     "",  "string",  "Base file name"},
    {"osdirectory","",  "string",  "Output directory"},
    {"osdbname",   "",  "string",  "Database name to add"},
    {"ossingle",   "N", "boolean", "Separate file for each entry"},
    {"oufo",       "",  "string",  "UFO features"},
    {"offormat",   "",  "string",  "Features format"},
    {"ofname",     "",  "string",  "Features file name"},
    {"ofdirectory","",  "string",  "Output directory"},
    {NULL, NULL, NULL, NULL}
};

AcdOQual acdQualTaxon[] =
{
    {"iformat",    "",  "string",  "Input taxonomy format"},
    {"iquery",     "",  "string",  "Input query fields or ID list"},
    {"ioffset",    "",  "integer", "Input start position offset"},
    {"idbname",    "",  "string",  "User-provided database name"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualText[] =
{
    {"iformat",    "",  "string",  "Input text format"},
    {"iquery",     "",  "string",  "Input query fields or ID list"},
    {"ioffset",    "",  "integer", "Input start position offset"},
    {"idbname",    "",  "string",  "User-provided database name"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualUrl[] =
{
    {"iformat",    "",  "string",  "Input URL format"},
    {"idbname",    "",  "string",  "User-provided database name"},
    {"swiss",      "N", "boolean", "Swissprot cross-reference"},
    {"embl",       "N", "boolean", "EMBL/GenBank/DDBJ cross-reference"},
    {"accession",  "",  "string",  "Primary accession for source data"},
    {"identifier", "",  "string",  "Identifier term name in EDAM"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualVariation[] =
{
    {"iformat",    "",  "string",  "Input variation format"},
    {"iquery",     "",  "string",  "Input query fields or ID list"},
    {"ioffset",    "",  "integer", "Input start position offset"},
    {"idbname",    "",  "string",  "User-provided database name"},
    {NULL, NULL, NULL, NULL}
};


AcdOQual acdQualXml[] =
{
    {"iformat",    "",  "string",  "Input xml format"},
    {"iquery",     "",  "string",  "Input query fields or ID list"},
    {"ioffset",    "",  "integer", "Input start position offset"},
    {"idbname",    "",  "string",  "User-provided database name"},
    {NULL, NULL, NULL, NULL}
};


/* Type definitions - must be after attributes and functions are defined
** Add new types here as needed
** Create attribute list acdAttrType
*/

/*
   "Type"         Attributes        Function
   Qualifiers     "Help Text"
*/




/* @funclist acdType **********************************************************
**
** Processing for ACD types
**
** Includes the acdSet functions for each ACD type
**
******************************************************************************/

AcdOType acdType[] =
{
/*   Name               Group       Section
**       Attributes             Qualifiers
**       SetFunction            HelpFunction           Destructor
**       PassRef  StdPrompt PromptFunction     UseCount      UseClassCount
**       Description of valid string for help */
    {"align",          "output",    acdSecOutput,
	 acdAttrAlign,          acdQualAlign,
	 &acdSetAlign,          NULL,                  &acdDelAlign,
	 AJTRUE,  AJTRUE,  &acdPromptAlign,    &acdUseAlign, &acdUseOut,
	 "Alignment output file" },
    {"array",          "simple",    NULL,
	 acdAttrArray,          NULL,
	 &acdSetArray,          NULL,                  &acdDelFloat,
	 AJTRUE,  AJFALSE, NULL,               &acdUseMisc, &acdUseMisc,
	 "List of floating point numbers" },
    {"assembly",       "input",     acdSecInput,
	 acdAttrAssembly,       acdQualAssembly,
	 &acdSetAssembly,       NULL,                  &acdDelAssembly,
	 AJTRUE,  AJTRUE,  &acdPromptAssembly, &acdUseData, &acdUseIn,
	 "Assembly of sequence reads" },
    {"boolean",        "simple",    NULL,
	 acdAttrBool,           NULL,
	 &acdSetBoolean,        NULL,                  &acdFree,
	 AJFALSE, AJFALSE, NULL,               &acdUseMisc, &acdUseMisc,
	 "Boolean value Yes/No" },
    {"codon",	       "input",     acdSecInput,
	 acdAttrCodon,          acdQualCodon,
	 &acdSetCodon,          NULL,                  &acdDelCod,
	 AJTRUE,  AJTRUE,  &acdPromptCodon,    &acdUseData, &acdUseIn,
	 "Codon usage file in EMBOSS data path" },
    {"cpdb",           "input",     acdSecInput,
	 acdAttrCpdb,           acdQualCpdb,
	 &acdSetCpdb,           NULL,                  &acdDelFile,
	 AJTRUE,  AJTRUE,  acdPromptCpdb,      &acdUseInfile, &acdUseIn,
	 "Clean PDB file" },
    {"datafile",       "input",     acdSecInput,
	 acdAttrDatafile,       NULL,
	 &acdSetDatafile,       NULL,                  &acdDelFile,
	 AJTRUE,  AJTRUE,  &acdPromptDatafile, &acdUseData, &acdUseIn,
	 "Data file" },
    {"directory",      "input",     acdSecInput,
	 acdAttrDirectory,      acdQualDirectory,
	 &acdSetDirectory,      NULL,                  &acdDelDir,
	 AJTRUE,  AJTRUE,  &acdPromptDirectory,&acdUseMisc, &acdUseIn,
	 "Directory" },
    {"dirlist",	       "input",     acdSecInput,
	 acdAttrDirlist,        acdQualDirlist,
	 &acdSetDirlist,        NULL,                  &acdDelList,
	 AJTRUE,  AJTRUE,  &acdPromptDirlist,  &acdUseMisc, &acdUseIn,
	 "Directory with files" },
    {"discretestates", "input",     acdSecInput,
	 acdAttrDiscretestates, NULL,
	 &acdSetDiscretestates, NULL,                  &acdDelPhyloState,
	 AJTRUE,  AJTRUE,  &acdPromptDiscretestates, &acdUseData, &acdUseIn,
	 "Discrete states file" },
    {"distances",      "input",     acdSecInput,
	 acdAttrDistances,      NULL,
	 &acdSetDistances,      NULL,                  &acdDelPhyloDist,
	 AJTRUE,  AJTRUE,  &acdPromptDistances, &acdUseData, &acdUseIn,
	 "Distance matrix" },
    {"featout",        "output",    acdSecOutput,
	 acdAttrFeatout,        acdQualFeatout,
	 &acdSetFeatout,        NULL,                  &acdDelFeattabOut,
	 AJTRUE,  AJTRUE,  &acdPromptFeatout,  &acdUseFeatout, &acdUseOut,
	 "Writeable feature table" },
    {"features",       "input",     acdSecInput,
	 acdAttrFeatures,       acdQualFeatures,
	 &acdSetFeatures,       NULL,                  &acdDelFeattable,
	 AJTRUE,  AJTRUE,  &acdPromptFeatures, &acdUseFeatures, &acdUseIn,
	 "Readable feature table" },
    {"filelist",       "input",     acdSecInput,
	 acdAttrFilelist,       NULL,
	 &acdSetFilelist,       NULL,                  &acdDelList,
	 AJTRUE,  AJTRUE,  &acdPromptFilelist, &acdUseMisc, &acdUseIn,
	 "Comma-separated file list" },
    {"float",          "simple",    NULL,
	 acdAttrFloat,          NULL,
	 &acdSetFloat,          NULL,                  &acdFree,
	 AJFALSE, AJFALSE, NULL,               &acdUseMisc, &acdUseMisc,
	 "Floating point number" },
    {"frequencies",    "input",     acdSecInput,
	 acdAttrFrequencies,    NULL,
	 &acdSetFrequencies,    NULL,                  &acdDelPhyloFreq,
	 AJTRUE,  AJTRUE,   &acdPromptFrequencies, &acdUseData, &acdUseIn,
	 "Frequency value(s)" },
    {"graph",          "graph",     acdSecOutput,
	 acdAttrGraph,          acdQualGraph,
	 &acdSetGraph,          NULL,                  &acdFree,
	 AJTRUE,  AJTRUE,  &acdPromptGraph,    &acdUseGraph, &acdUseOut,
	 "Graph device for a general graph" },
    {"infile",         "input",     acdSecInput,
	 acdAttrInfile,         NULL,
	 &acdSetInfile,          NULL,                  &acdDelFile,
	 AJTRUE,  AJTRUE,  &acdPromptInfile,   &acdUseInfile, &acdUseIn,
	 "Input file" },
    {"integer",        "simple",    NULL,
	 acdAttrInt,            NULL,
	 &acdSetInt,            NULL,                  &acdFree,
	 AJFALSE, AJFALSE, NULL,               &acdUseMisc, &acdUseMisc,
	 "Integer" },
    {"list",           "selection", NULL,
	 acdAttrList,           NULL,
	 acdSetList,            NULL,                  acdDelStrArray,
	 AJTRUE,  AJFALSE, NULL,               &acdUseMisc, &acdUseMisc,
	 "Choose from menu list of values" },
    {"matrix",         "input",     acdSecInput,
	 acdAttrMatrix,         NULL,
	 &acdSetMatrix,         NULL,                  &acdDelMatrix,
	 AJTRUE,  AJFALSE, &acdPromptMatrix,   &acdUseData, &acdUseIn,
	 "Comparison matrix file in EMBOSS data path" },
    {"matrixf",        "input",     acdSecInput,
	 acdAttrMatrixf,        NULL,
	 &acdSetMatrixf,        NULL,                  &acdDelMatrixf,
	 AJTRUE,  AJFALSE, &acdPromptMatrix,   &acdUseData, &acdUseIn,
	 "Comparison matrix file in EMBOSS data path" },
    {"obo",            "input",     acdSecInput,
	 acdAttrObo,            acdQualObo,
	 &acdSetObo,            &acdHelpTextObo,       &acdDelObo,
	 AJTRUE,  AJTRUE,  &acdPromptObo,      &acdUseData, &acdUseIn,
	 "OBO bio-ontology term(s)" },
    {"outassembly",     "output",    acdSecOutput,
	 acdAttrOutassembly,    acdQualOutassembly,
	 &acdSetOutassembly,    NULL,                  &acdDelOutfile,
	 AJTRUE,  AJTRUE,  &acdPromptOutassembly, &acdUseOutfile, &acdUseOut,
	 "Assembly of sequence reads" },
    {"outcodon",       "output",    acdSecOutput,
	 acdAttrOutcodon,       acdQualOutcodon,
	 &acdSetOutcodon,       NULL,                  &acdDelOutfile,
	 AJTRUE,  AJTRUE,  &acdPromptOutcodon, &acdUseOutfile, &acdUseOut,
	 "Codon usage file" },
    {"outcpdb",        "output",    acdSecOutput,
	 acdAttrOutcpdb,        NULL,
	 &acdSetOutcpdb,        NULL,                  &acdDelOutfile,
	 AJTRUE,  AJTRUE,  &acdPromptOutcpdb,  &acdUseOutfile, &acdUseOut,
	 "Cleaned PDB file" },
    {"outdata",        "output",    acdSecOutput,
	 acdAttrOutdata,        acdQualOutdata,
	 &acdSetOutdata,        NULL,                  &acdDelOutfile,
	 AJTRUE,  AJTRUE,  &acdPromptOutdata,  &acdUseOutfile, &acdUseOut,
	 "Formatted output file" },
    {"outdir",         "output",    acdSecOutput,
	 acdAttrOutdir,         acdQualOutdir,
	 &acdSetOutdir,         NULL,                  &acdDelDirout,
	 AJTRUE,  AJTRUE,  &acdPromptOutdir,   &acdUseMisc, &acdUseOut,
	 "Output directory" },
    {"outdiscrete",    "output",    acdSecOutput,
	 acdAttrOutdiscrete,    acdQualOutdiscrete,
	 &acdSetOutdiscrete,    NULL,                  &acdDelOutfile,
	 AJTRUE,  AJTRUE,  &acdPromptOutdiscrete, &acdUseOutfile, &acdUseOut,
	 "Discrete states file" },
    {"outdistance",    "output",    acdSecOutput,
	 acdAttrOutdistance,    NULL,
	 &acdSetOutdistance,    NULL,                  &acdDelOutfile,
	 AJTRUE,  AJTRUE,  &acdPromptOutdistance, &acdUseOutfile, &acdUseOut,
	 "Distance matrix" },
    {"outfile",        "output",     acdSecOutput,
	 acdAttrOutfile,        acdQualOutfile,
	 &acdSetOutfile,        NULL,                  &acdDelFile,
	 AJTRUE,  AJTRUE,  &acdPromptOutfile,    &acdUseOutfile, &acdUseOut,
	 "Output file" },
    {"outfreq",        "output",    acdSecOutput,
	 acdAttrOutfreq,        acdQualOutfreq,
	 &acdSetOutfreq,        NULL,                  &acdDelOutfile,
	 AJTRUE,  AJTRUE,  &acdPromptOutfreq,    &acdUseOutfile, &acdUseOut,
	 "Frequency value(s)" },
    {"outmatrix",      "output",    acdSecOutput,
	 acdAttrOutmatrix,      acdQualOutmatrix,
	 &acdSetOutmatrix,      NULL,                  &acdDelOutfile,
	 AJTRUE,  AJTRUE,  &acdPromptOutmatrix,  &acdUseOutfile, &acdUseOut,
	 "Comparison matrix file" },
    {"outmatrixf",     "output",    acdSecOutput,
	 acdAttrOutmatrixf,     acdQualOutmatrixf,
	 &acdSetOutmatrixf,     NULL,                  &acdDelOutfile,
	 AJTRUE,  AJTRUE,  &acdPromptOutmatrix,  &acdUseOutfile, &acdUseOut,
	 "Comparison matrix file" },
    {"outobo",         "output",    acdSecOutput,
         acdAttrOutobo,         acdQualOutobo,
	 &acdSetOutobo,         NULL,                  &acdDelOutfile,
	 AJTRUE,  AJTRUE,  &acdPromptOutobo,     &acdUseOutfile, &acdUseOut,
	 "OBO ontology term(s)" },
    {"outproperties",  "output",    acdSecOutput,
	 acdAttrOutproperties,  acdQualOutproperties,
	 &acdSetOutproperties,  NULL,                  &acdDelOutfile,
	 AJTRUE,  AJTRUE,  &acdPromptOutproperties, &acdUseOutfile, &acdUseOut,
	 "Property value(s)" },
    {"outrefseq",       "output",    acdSecOutput,
	 acdAttrOutrefseq,      acdQualOutrefseq,
	 &acdSetOutrefseq,      NULL,                  &acdDelOutfile,
	 AJTRUE,  AJTRUE,  &acdPromptOutrefseq,   &acdUseOutfile, &acdUseOut,
	 "Reference sequence" },
    {"outresource",    "output",    acdSecOutput,
         acdAttrOutresource,    acdQualOutresource,
	 &acdSetOutresource,    NULL,                  &acdDelOutfile,
         AJTRUE,  AJTRUE,  &acdPromptOutresource, &acdUseOutfile, &acdUseOut,
	 "Data resource entry" },
    {"outscop",        "output",    acdSecOutput,
	 acdAttrOutscop,        acdQualOutscop,
	 &acdSetOutscop,        NULL,                  &acdDelOutfile,
	 AJTRUE,  AJTRUE,  &acdPromptOutscop,    &acdUseOutfile, &acdUseOut,
	 "Scop entry" },
    {"outtaxon",       "output",    acdSecOutput,
         acdAttrOuttaxon,       acdQualOuttaxon,
	 &acdSetOuttaxon,       NULL,                  &acdDelOutfile,
	 AJTRUE,  AJTRUE,  &acdPromptOuttaxon,   &acdUseOutfile, &acdUseOut,
	 "NCBI taxonomy entries" },
    {"outtext",        "output",    acdSecOutput,
         acdAttrOuttext,        acdQualOuttext,
	 &acdSetOuttext,        NULL,                  &acdDelOutfile,
	 AJTRUE,  AJTRUE,  &acdPromptOuttext,    &acdUseOutfile, &acdUseOut,
	 "Text entries" },
    {"outtree",        "output",    acdSecOutput,
	 acdAttrOuttree,        acdQualOuttree,
	 &acdSetOuttree,        NULL,                  &acdDelOutfile,
	 AJTRUE,  AJTRUE,  &acdPromptOuttree,    &acdUseOutfile, &acdUseOut,
	 "Phylogenetic tree" },
    {"outurl",         "output",    acdSecOutput,
         acdAttrOuturl,         acdQualOuturl,
	 &acdSetOuturl,         NULL,                  &acdDelOutfile,
	 AJTRUE,  AJTRUE,  &acdPromptOuturl,     &acdUseOutfile, &acdUseOut,
	 "URL entries" },
    {"outvariation",   "output",    acdSecOutput,
         acdAttrOutvariation,   acdQualOutvariation,
	 &acdSetOutvariation,   NULL,                  &acdDelOutfile,
	 AJTRUE,  AJTRUE,  &acdPromptOutvariation, &acdUseOutfile, &acdUseOut,
	 "Variation entries" },
    {"outxml",     "output",    acdSecOutput,
	 acdAttrOutxml,    acdQualOutxml,
	 acdSetOutxml,     NULL,                  acdDelOutfile,
	 AJTRUE,  AJTRUE,  acdPromptOutxml,  &acdUseOutfile, &acdUseOut,
	 "Xml" },
    {"pattern",       "input",     acdSecInput,
	 acdAttrPattern,        acdQualPattern,
	 &acdSetPattern,        NULL,                  &acdDelPatlist,
	 AJTRUE,  AJTRUE,  &acdPromptPattern,     &acdUseData, &acdUseIn,
	 "Property value(s)" },
    {"properties",     "input",     acdSecInput,
	 acdAttrProperties,     NULL,
	 &acdSetProperties,     NULL,                  &acdDelPhyloProp,
	 AJTRUE,  AJTRUE,  &acdPromptProperties, &acdUseData, &acdUseIn,
	 "Property value(s)" },
    {"range",	       "simple",    NULL,
	 acdAttrRange,          NULL,
	 &acdSetRange,          NULL,                  &acdDelRange,
	 AJTRUE,  AJFALSE, NULL,               &acdUseMisc, &acdUseMisc,
	 "Sequence range" },
    {"refseq",         "input",     acdSecInput,
	 acdAttrRefseq,         acdQualRefseq,
	 &acdSetRefseq,         NULL,                  &acdDelRefseq,
	 AJTRUE,  AJTRUE,  &acdPromptRefseq, &acdUseData, &acdUseIn,
	 "Reference sequence" },
    {"regexp",	       "input",     acdSecInput,
	 acdAttrRegexp,         acdQualRegexp,
	 &acdSetRegexp,         NULL,                  &acdDelReg,
	 AJTRUE,  AJTRUE, &acdPromptRegexp,    &acdUseMisc, &acdUseIn,
	 "Regular expression pattern" },
    {"report",         "output",    acdSecOutput,
	 acdAttrReport,         acdQualReport,
	 &acdSetReport,         NULL,                  &acdDelReport,
	 AJTRUE,  AJTRUE,  &acdPromptReport,   &acdUseReport, &acdUseOut,
	 "Report output file" },
    {"resource",       "input",     acdSecInput,
         acdAttrResource,       acdQualResource,
	 &acdSetResource,       &acdHelpTextResource,  &acdDelResource,
	 AJTRUE,  AJTRUE,  &acdPromptResource, &acdUseData, &acdUseIn,
	 "Data resource catalogue entry(s)" },
    {"scop",           "input",     acdSecInput,
	 acdAttrScop,           acdQualScop,
	 &acdSetScop,           NULL,                  &acdDelFile,
	 AJTRUE,  AJTRUE,  &acdPromptScop,     &acdUseInfile, &acdUseIn,
	 "Clean PDB file" },
    {"selection",      "selection", NULL,
	 acdAttrSelect,         NULL,
	 &acdSetSelect,         NULL,                  &acdDelStrArray,
	 AJTRUE,  AJFALSE, NULL,               &acdUseMisc, &acdUseMisc,
	 "Choose from selection list of values" },
    {"sequence",       "input",     acdSecInput,
	 acdAttrSeq,            acdQualSeq,
	 &acdSetSeq,            &acdHelpTextSeq,       &acdDelSeq,
	 AJTRUE,  AJTRUE,  &acdPromptSeq,      &acdUseSeq, &acdUseIn,
	 "Readable sequence" },
    {"seqall",         "input",     acdSecInput,
	 acdAttrSeqall,         acdQualSeqall,
	 &acdSetSeqall,         &acdHelpTextSeq,       &acdDelSeqall,
	 AJTRUE,  AJTRUE,  &acdPromptSeq,      &acdUseSeq, &acdUseIn,
	 "Readable sequence(s)" },
    {"seqout",         "output",    acdSecOutput,
	 acdAttrSeqout,         acdQualSeqout,
	 &acdSetSeqout,         &acdHelpTextSeqout,    &acdDelSeqout, 
	 AJTRUE,  AJTRUE,  &acdPromptSeqout,   &acdUseSeqout, &acdUseOut,
	 "Writeable sequence" },
    {"seqoutall",      "output",    acdSecOutput,
	 acdAttrSeqoutall,      acdQualSeqoutall,
	 &acdSetSeqoutall,      &acdHelpTextSeqout,    &acdDelSeqout,
	 AJTRUE,  AJTRUE,  &acdPromptSeqout,   &acdUseSeqout, &acdUseOut,
	 "Writeable sequence(s)" },
    {"seqoutset",      "output",    acdSecOutput,
	 acdAttrSeqoutset,      acdQualSeqoutset,
	 &acdSetSeqoutset,      &acdHelpTextSeqout,    &acdDelSeqout,
	 AJTRUE,  AJTRUE,  &acdPromptSeqout,   &acdUseSeqout, &acdUseOut,
	 "Writeable sequences" },
    {"seqset",         "input",     acdSecInput,
	 acdAttrSeqset,         acdQualSeqset,
	 &acdSetSeqset,         &acdHelpTextSeq,       &acdDelSeqset,
	 AJTRUE,  AJTRUE,  &acdPromptSeq,      &acdUseSeq, &acdUseIn,
	 "Readable set of sequences" },
    {"seqsetall",      "input",     acdSecInput,
	 acdAttrSeqsetall,      acdQualSeqsetall,
	 &acdSetSeqsetall,      &acdHelpTextSeq,       &acdDelSeqsetArray,
	 AJTRUE,  AJTRUE,  &acdPromptSeq,      &acdUseSeq, &acdUseIn,
	 "Readable sets of sequences" },
    {"string",         "simple",    NULL,
	 acdAttrString,          NULL,
	 acdSetString,           NULL,                 acdDelStr,
	 AJTRUE,  AJFALSE, NULL,               &acdUseMisc, &acdUseMisc,
	 "String value" },
    {"taxon",          "input",     acdSecInput,
	 acdAttrTaxon,           acdQualTaxon,
	 &acdSetTaxon,           &acdHelpTextTaxon,    &acdDelTaxon,
	 AJTRUE,  AJTRUE,  &acdPromptTaxon,      &acdUseData, &acdUseIn,
	 "NCBI taxonomy entries" },
    {"text",           "input",     acdSecInput,
	 acdAttrText,            acdQualText,
	 &acdSetText,            &acdHelpTextText,     &acdDelText,
	 AJTRUE,  AJTRUE,  &acdPromptText,       &acdUseData, &acdUseIn,
	 "Text entries" },
    {"toggle",         "simple",     NULL,
	 acdAttrToggle,          NULL,
	 &acdSetToggle,          NULL,                 &acdFree,
	 AJFALSE, AJFALSE, NULL,               &acdUseMisc, &acdUseMisc,
	 "Toggle value Yes/No" },
    {"tree",           "input",      acdSecInput,
	 acdAttrTree,            NULL,
	 &acdSetTree,            NULL,                 &acdDelPhyloTree,
	 AJTRUE,  AJTRUE,  &acdPromptTree,     &acdUseData, &acdUseIn,
	 "Phylogenetic tree" },
    {"url",            "input",     acdSecInput,
	 acdAttrUrl,             acdQualUrl,
	 &acdSetUrl,             &acdHelpTextUrl,       &acdDelUrl,
	 AJTRUE,  AJTRUE,  &acdPromptUrl,        &acdUseData, &acdUseIn,
	 "URL entries" },
    {"variation",      "input",     acdSecInput,
	 acdAttrVariation,       acdQualVariation,
	 &acdSetVariation,       &acdHelpTextVariation, &acdDelVariation,
	 AJTRUE,  AJTRUE,  &acdPromptVariation,  &acdUseData, &acdUseIn,
	 "Variation entries" },
    {"xml",       "input",     acdSecInput,
	 acdAttrXml,       acdQualXml,
	 acdSetXml,        acdHelpTextXml,   acdDelXml,
	 AJTRUE,  AJTRUE,  acdPromptXml,  &acdUseData, &acdUseIn,
	 "Xml" },
    {"xygraph",        "graph",      acdSecOutput,
	 acdAttrGraphxy,         acdQualGraphxy,
	 &acdSetGraphxy,         NULL,                 &acdFree,
	 AJTRUE,  AJTRUE,  &acdPromptGraph,    &acdUseGraph, &acdUseOut,
	 "Graph device for a 2D graph" },
     {NULL, NULL, NULL,   NULL, NULL,   NULL, NULL, NULL,
	  AJFALSE, AJFALSE, 0, NULL, NULL,    NULL}
};




/* @datastatic AcdPValid ******************************************************
**
** ACD valid input structure. For each ACD type, stores functions
** that reports on valid input and expected values
**
** @alias AcdSValid
** @alias AcdOValid
**
** @attr Name [const char*] ACD type name
** @attr Valid [void function]  Function to report valid values in general
** @attr Expect [void function] Function to report expected values
**                              for this definition
** @@
******************************************************************************/

typedef struct AcdSValid
{
    const char* Name;
    void (*Valid) (const AcdPAcd thys, AjBool table, AjPStr* str);
    void (*Expect) (const AcdPAcd thys, AjBool table, AjPStr* str);
} AcdOValid;

#define AcdPValid AcdOValid*




/* @funclist acdValue *********************************************************
**
** ACD type help processing, includes functions to describe valid
** values and expected values in -help output and -acdtable output
**
******************************************************************************/

AcdOValid acdValue[] =
{
    {"align",     acdHelpValidAlign,    acdHelpExpectOut},
    {"codon",     acdHelpValidCodon,    acdHelpExpectCodon},
    {"datafile",  acdHelpValidData,     acdHelpExpectData},
    {"dirlist",   acdHelpValidDirlist,  acdHelpExpectDirlist},
    {"featout",   acdHelpValidFeatout,  acdHelpExpectFeatout},
    {"features",  acdHelpValidFeatures, acdHelpExpectFeatures},
    {"filelist",  acdHelpValidFilelist, acdHelpExpectFilelist},
    {"float",     acdHelpValidFloat,    acdHelpExpectFloat},
    {"graph",     acdHelpValidGraph,    acdHelpExpectGraph},
    {"infile",    acdHelpValidIn,       acdHelpExpectIn},
    {"integer",   acdHelpValidInt,      acdHelpExpectInt},
    {"list",      acdHelpValidList,     NULL},
    {"matrix",    acdHelpValidMatrix,   acdHelpExpectMatrix},
    {"matrixf",   acdHelpValidMatrix,   acdHelpExpectMatrix},
    {"outfile",   acdHelpValidOut,      acdHelpExpectOut},
    {"outobo",    acdHelpValidOut,      acdHelpExpectOut},
    {"outresource", acdHelpValidOut,    acdHelpExpectOut},
    {"regexp",    acdHelpValidRegexp,   acdHelpExpectRegexp},
    {"report",    acdHelpValidReport,   acdHelpExpectOut},
    {"select",    acdHelpValidSelect,   NULL},
    {"seqall",    acdHelpValidSeq,      acdHelpExpectSeq},
    {"seqset",    acdHelpValidSeq,      acdHelpExpectSeq},
    {"seqsetall", acdHelpValidSeq,      acdHelpExpectSeq},
    {"sequence",  acdHelpValidSeq,      acdHelpExpectSeq},
    {"seqout",    acdHelpValidSeqout,   acdHelpExpectSeqout},
    {"seqoutall", acdHelpValidSeqout,   acdHelpExpectSeqout},
    {"seqoutset", acdHelpValidSeqout,   acdHelpExpectSeqout},
    {"string",    acdHelpValidString,   acdHelpExpectString},
    {"range",     acdHelpValidRange,    acdHelpExpectRange},
    {"xygraph",   acdHelpValidGraph,    acdHelpExpectGraph},
    {NULL,        NULL, NULL}
};


static const char* acdResource[] =
{
    "genetic code",
    NULL
};


/*** command line retrieval routines ***/




/* @section initialisation ****************************************************
**
** Initialises everything needed for ACD processing. 
**
** @fdata [none]
**
** @nam3rule  Init    Initialise internals
**
** @argrule   Init    pgm [const char*] Program name
** @argrule   Init    argc [ajint] Number of command line arguments
** @argrule   Init    argv [char* const[]] Command line arguments
** @argrule   P    package [const char*] Package name (empty for default name)
** @argrule   V    packversion [const char*] Package version (empty for default)
**
** @valrule   *  [void]
** @fcategory misc
**
******************************************************************************/




/* @func ajAcdInit ************************************************************
**
** Initialises everything. Reads an ACD (AJAX Command Definition) file
** prompts the user for any missing information, reads all sequences
** and other input into local structures which applications can request.
**
** @param [r] pgm [const char*] Application name, used as the name
**                              of the ACD file
** @param [r] argc [ajint] Number of arguments provided on the command line,
**        usually passed as-is by the calling application.
** @param [r] argv [char* const[]] Actual arguments as an array of text.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajAcdInit(const char *pgm, ajint argc, char * const argv[])
{
    ajAcdInitPV(pgm, argc, argv, "", "");

    return;
}




/* @func ajAcdInitPV **********************************************************
**
** Initialises everything. Reads an ACD (AJAX Command Definition) file
** prompts the user for any missing information, reads all sequences
** and other input into local structures which applications can request.
**
** @param [r] pgm [const char*] Application name, used as the name
**                              of the ACD file
** @param [r] argc [ajint] Number of arguments provided on the command line,
**        usually passed as-is by the calling application.
** @param [r] argv [char* const[]] Actual arguments as an array of text.
** @param [r] package [const char*] Package name, used to find the ACD file
** @param [r] packversion [const char*] Package version
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ajAcdInitPV(const char *pgm, ajint argc, char * const argv[],
                 const char *package, const char *packversion)
{    
    static AjPFile acdFile = NULL;
    static AjPStr acdLine = NULL;
    static AjPStr acdRoot = NULL;
    static AjPStr acdRootInst = NULL;
    static AjPStr acdPack = NULL;
    static AjPStr acdPackRoot = NULL;
    static AjPStr acdPackRootName = NULL;
    static AjPStr acdUtilRoot = NULL;
    AjPStr applversion = NULL;
    AjPStr versionstr = NULL;
    AjPStr comment = NULL;
    AjPStrTok tokenhandle = NULL;
    char white[] = " \t\n\r";
    AjPList acdListWords = NULL;
    AjPList acdListCount = NULL;
    AjPStr tmpword = NULL;	    /* words to add to acdListWords */
    ajint i;
    ajint *k = NULL;
    ajint *kc = NULL;
    ajuint *kp = NULL;
    size_t pos;

    acdProgram = ajStrNewC(pgm);

    acdSecList = ajListstrNew();
    acdSecTable = ajTablestrNewCase(50);

    acdLog("testing acdprompts");
    if(ajNamGetValueC("acdprompts", &acdTmpStr))
    {
	acdLog("acdprompts '%S'", acdTmpStr);
	if(ajStrToInt(acdTmpStr, &i))
	    acdPromptTry = i;
	if(acdPromptTry < 1) acdPromptTry = 1;
	acdLog("acdPromptTry %d", acdPromptTry);
    }
    
    if(ajNamGetValueC("acdlog", &acdTmpStr))
	ajStrToBool(acdTmpStr, &acdDoLog);

    if(ajNamGetValueC("acdwarnrange", &acdTmpStr))
	ajStrToBool(acdTmpStr, &acdDoWarnRange);

    /* pre-parse the command line for special options */
    
    acdArgsScan(argc, argv);
    
    ajDebug("ajAcdInitPV pgm '%s' package '%s'\n", pgm, package);
    ajDebug("  version '%S' system '%S'\n",
            ajNamValueVersion(),
            ajNamValueSystem());

    /* open the command definition file */
    
    if(*package)
        ajStrAssignC(&acdPackName, package);

    if(*packversion)
        ajStrAssignC(&acdPackVersion, packversion);

    ajStrAssignS(&acdPack, ajNamValuePackage());
    ajStrAssignS(&acdRootInst, ajNamValueInstalldir());
    ajDirnameFix(&acdRootInst);
    
    if(ajNamGetValueC("acdroot", &acdRoot))
    {				       /* _acdroot variable defined */
	ajDirnameFix(&acdRoot);
	ajFmtPrintS(&acdFName, "%S%s.acd", acdRoot, pgm);
	acdLog("Trying acdfile '%S' (acdroot)\n", acdFName);
	acdFile = ajFileNewInNameS(acdFName);
    }
    else if(*package)
    {					/* separate package */
	/*	ajFmtPrintS(&acdFName, "%Sshare/%S/acd/%s.acd",
		acdRootInst, acdPack, pgm);*/
	ajFmtPrintS(&acdFName, "%Sshare/EMBOSS/acd/%s.acd",
		    acdRootInst, pgm);
	acdLog("Trying acdfile '%S' (package '%s' installed)\n",
	       acdFName, package);
	acdFile = ajFileNewInNameS(acdFName);
	if(!acdFile)
	{
	    acdLog("acdfile '%S' not opened\n", acdFName);
	    ajStrAssignC(&acdPack, package); /* package name for acdInitPV */
	    ajStrFmtLower(&acdPack);

	    ajStrAssignS(&acdPackRootName, acdPack);
	    ajStrAppendC(&acdPackRootName, "acdroot");
	    if(ajNamGetValueS(acdPackRootName, &acdPackRoot))
	    {
		ajDirnameFix(&acdPackRoot);
		ajFmtPrintS(&acdFName, "%S%s.acd", acdPackRoot, pgm);
		acdLog("Trying acdfile '%S' (package %sacdroot)\n",
		       acdFName, package);
	    }
	    else
	    {
		ajStrAssignS(&acdPackRoot, ajNamValueRootdir());
		ajDirnameUp(&acdPackRoot);
		ajFmtPrintS(&acdFName, "%Sembassy/%S/emboss_acd/%s.acd",
			    acdPackRoot, acdPack, pgm);
		acdLog("Trying acdfile '%S' (package %s source)\n",
		       acdFName, package);
	    }
	    acdFile = ajFileNewInNameS(acdFName);
	}
    }
    else
    {					/* main package */
	ajFmtPrintS(&acdFName, "%Sshare/%S/acd/%s.acd",
		    acdRootInst, acdPack, pgm);
	acdLog("Trying acdfile '%S' (installed)\n", acdFName);
	acdFile = ajFileNewInNameS(acdFName);
	if(!acdFile)
	{
	    acdLog("acdfile '%S' not opened\n", acdFName);
	    ajStrAssignC(&acdPack, "emboss");

	    if(ajNamGetValueC("acdutilroot", &acdUtilRoot))
	    {
		ajDirnameFix(&acdUtilRoot);
		ajFmtPrintS(&acdFName, "%S%s.acd", acdUtilRoot, pgm);
		acdLog("Trying acdfile '%S' (acdutilroot)\n", acdFName);
		acdFile = ajFileNewInNameS(acdFName);
	    }
	}
	if(!acdFile)
	{
	    acdLog("acdfile '%S' not opened\n", acdFName);
	    ajStrAssignS(&acdRoot, ajNamValueRootdir());
	    ajDirnameFix(&acdRoot);
	    ajFmtPrintS(&acdFName, "%Sacd/%s.acd", acdRoot, pgm);
	    acdLog("Trying acdfile '%S' (original main source)\n", acdFName);
	    acdFile = ajFileNewInNameS(acdFName);
	    if(!acdFile)
	    {
		acdLog("acdfile '%S' not opened\n", acdFName);
	    }
	}
    }
    
    
    if(!acdFile)			/* test by nofile.acd */
	acdError("ACD file not opened\n");
    
    /* read the whole file into a string [change to use a list later] */
    
    acdListWords = ajListstrNew();
    acdListCount = ajListNew();
    
    acdListComments = ajListstrNew();
    acdListCommentsCount = ajListNew();
    acdListCommentsColumn = ajListNew();
    
    while(ajReadlineTrim(acdFile, &acdLine))
    {
	AJNEW0(k);
	*k = (ajuint) ajListGetLength(acdListWords);
	ajListPushAppend(acdListCount, k);
	if(ajStrCutCommentsRestpos(&acdLine, &comment, &pos))
	{
	    tokenhandle = ajStrTokenNewC(acdLine, white);
	  
	    while(ajStrTokenNextParse(tokenhandle, &tmpword))
	    {
		if(ajStrGetLen(tmpword)) /* nothing before first whitespace */
		{
		    ajListstrPushAppend(acdListWords, tmpword);
		    tmpword = NULL;
		}
		else
		{
		    ajStrDel(&tmpword);
		}
	    }
	    ajStrTokenDel(&tokenhandle);
	    ajStrDel(&tmpword); 	/* empty token at the end */
	}

	if (ajStrGetLen(comment))
        {
	    ajListstrPushAppend(acdListComments, comment);
	    comment = NULL;
	    AJNEW(kc);
	    *kc = (ajuint) ajListGetLength(acdListWords);
	    ajListPushAppend(acdListCommentsCount, kc);
	    kc = NULL;
	    AJNEW(kp);
	    *kp = pos;
	    ajListPushAppend(acdListCommentsColumn, kp);
	    kp = NULL;
	}
    }
    ajFileClose(&acdFile);
    ajStrDel(&acdLine);
    
    AJNEW0(k);
    *k = (ajuint) ajListGetLength(acdListWords);
    ajListPushAppend(acdListCount, k);
    
    acdCodeInit();
    acdKnowntypeInit();

    /* Parse the input to set the initial definitions */
    
    acdParse(acdListWords, acdListCount);
    
    ajListstrFreeData(&acdListWords);
    ajListFreeData(&acdListCount);
        
    if(acdDoVersion)
    {
        ajFmtPrintS(&versionstr, "EMBOSS:%s", VERSION);

        if(ajStrGetLen(versionstr) < 14)
            ajStrAppendC(&versionstr, ".0");

        if(ajStrGetLen(acdPackVersion))
            ajFmtPrintAppS(&versionstr, " %S:%S", acdPackName, acdPackVersion);

        acdAttrResolve(acdApplAcd, "versionnumber", &applversion);

        if(ajStrGetLen(applversion))
        {
            ajFmtPrintAppS(&versionstr, " %s:%S", pgm, applversion);
            ajStrDel(&applversion);
        }

        ajUserDumpS(versionstr);
        ajStrDel(&versionstr);
        ajExit();
    }

    if(acdDoPretty || acdDoValid)
	ajExit();
    
    /* Fill in incomplete information like parameter numbers */
    
    acdProcess();
    
    AJCNEW0(acdParamSet, acdNParam+1);
    
    /* report on what we have so far */
    
    acdListReport("Definitions in ACD file");
    
    /* parse the command line and update the values */
    
    acdArgsParse(argc, argv);
    
    /* report on what we have so far */
    
    acdListReport("Results of parsing command line arguments");
    
    /* set the true values and prompt for missing standard values */

    if(acdDoTable || acdDoXsd || acdDoGalaxy)
	acdHelp();

    acdSetAll();
    
    /* report on what we have now */
    
    acdListReport("Final results after setting values and prompting the user");
    
    /* all done */
    
    ajStrDel(&acdRoot);
    ajStrDel(&acdRootInst);
    ajStrDel(&acdPack);
    ajStrDel(&acdPackRoot);
    ajStrDel(&acdPackRootName);
    ajStrDel(&acdFName);
    ajStrDel(&comment);
    
    return;
}




/*===========================================================================*/
/*========================= ACD File Parsing ================================*/
/*===========================================================================*/




/* @funcstatic acdStage *******************************************************
**
** Tests next token to set the next parsing stage.
**
** @param [r] token [const AjPStr] Current token
** @return [AcdEStage] Stage enumerated code
**
** @release 1.0.0
** @@
******************************************************************************/

static AcdEStage acdStage(const AjPStr token)
{
    ajint i;
    ajint ifound=0;
    AcdEStage j=BAD_STAGE;
    
    AjPStr ambigList = NULL;
    
    ambigList = ajStrNew();
    
    if(!ajStrGetLen(token))
	return BAD_STAGE;
    
    i = QUAL_STAGE + 1;
    while(acdKeywords[i].Name)		/* ACD keywords */
    {
	if(ajStrMatchC(token, acdKeywords[i].Name))
	    return acdKeywords[i].Stage;
	if(ajCharPrefixS(acdKeywords[i].Name, token))
	{
	    ifound++;
	    j = acdKeywords[i].Stage;
	    acdAmbigAppC(&ambigList, acdKeywords[i].Name);
	}
	i++;
    }
    
    i =  0;
    while(acdType[i].Name)		/* ACD types as qualifiers */
    {
	if(ajStrMatchC(token, acdType[i].Name))
	    return QUAL_STAGE;
	if(ajCharPrefixS(acdType[i].Name, token))
	{
	    ifound++;
	    j = QUAL_STAGE;
	    acdAmbigAppC(&ambigList, acdType[i].Name);
	}
	i++;
    }

    if(ifound == 1)
    {
	if (acdDoValid)
	    acdWarn("Abbreviated stage '%S' (%S)", token, ambigList);
	return j;
    }


    if(ifound > 1)
    {					/* test ambigtype.acd */
	acdError("ambiguous acd type %S (%S)", token, ambigList);
	ajStrDel(&ambigList);
    }

    ajStrDel(&ambigList);

    return BAD_STAGE;
}




/* @funcstatic acdParse *******************************************************
**
** Parse the command line definition and build data structures from it.
**
** @param [u] listwords [AjPList] List of words (as strings)
**                      from ACD file. List empty on completion
** @param [u] listcount [AjPList] List of word count before each line.
**                      List empty on completion
** @return [void]
** @see acdNewAppl
** @see acdNewQual
** @see acdNewVar
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdParse(AjPList listwords, AjPList listcount)
{
    AjPStr acdStrType  = NULL;
    AjPStr acdStrAlias = NULL;
    AjPStr acdStrValue = NULL;
    AjPStr secname  = NULL;
    ajint linecount = 0;
    ajint lineword  = 0;
    ajint *iword    = NULL;
    ajint *icword    = NULL;
    ajint *icpos    = NULL;
    AjPStr cmtstr = NULL;
    AjPTime today = NULL;

    /* initialise the global line number counter to zero */
    acdLineNum = 0;

    if(acdDoValid)
    {
        if(ajNamGetValueC("edam", &acdEdamPath))
        {
            acdEdamFile = ajFileNewInNameS(acdEdamPath);
            if(acdEdamFile)
                acdEdam = ajObodataParseObofile(acdEdamFile, "none");
        }
    }

    while(ajListGetLength(acdListCommentsCount) && !acdCmtWord)
    {
	ajListPeek(acdListCommentsCount, (void**) &icword);
	acdCmtWord = *icword;
	if(!acdCmtWord)
	{
	    ajStrDel(&cmtstr);
	    ajListPop(acdListCommentsCount, (void**) &icword);
	    ajListPop(acdListCommentsColumn, (void**) &icpos);
	    ajListstrPop(acdListComments, &cmtstr);
	    acdPrettyComment(cmtstr);
	    AJFREE(icword);
	    AJFREE(icpos);
	}
    }

    if(ajStrGetLen(cmtstr))
    {
	acdPretty("\n");
	ajStrDel(&cmtstr);
    }

    while(ajListGetLength(listcount) && (!lineword))
    {
	ajListPeek(listcount, (void**) &iword);

	if(*iword)
	    lineword = *iword;
	else
	{
	    ajListPop(listcount, (void**) &iword);
	    linecount++;
	    acdLineNum = linecount - 1;
	    AJFREE(iword);
	}
    }
    lineword = 0;
    acdWordNum = 0;
    
    while(ajListGetLength(listwords))
    {
 	acdWordNextName(listwords, &acdStrType);
	
	while(ajListGetLength(listcount) && (lineword < acdWordNum))
	{
	    ajListPop(listcount, (void**) &iword);
	    lineword = *iword;
	    linecount++;
	    acdLineNum = linecount - 1;
	    AJFREE(iword);
	}
	
	acdCurrentStage = acdStage(acdStrType);
	if(acdWordNum == 1)
	    if(acdCurrentStage != APPL_STAGE) /* test noappl.acd */
		acdError("Application definition required at start");
	
	switch(acdCurrentStage)
	{	    
	case APPL_STAGE:
	    if(acdWordNum != 1)
		acdError("Application definition allowed only at start");

	    /* application: then the appl name */
	    acdParseName(listwords, &acdStrName);
	    if (!ajStrMatchS(acdStrName, acdProgram))
		acdError("Application name '%S' does not match filename '%S'",
			 acdStrName, acdProgram);
	    
	    acdNewCurr = acdNewAppl(acdStrName);
            acdApplAcd = acdNewCurr;
	    
	    acdWordNum++;		/* add one for '[' */

	    acdPretty("%s: %S [\n",
		      acdKeywords[acdCurrentStage].Name, acdStrName);
	    
	    acdWordNum--;		/* not yet parsed '[' */
	    
	    acdParseAttributes(acdNewCurr, listwords);

            acdAttrToBool(acdNewCurr, "wrapper", ajFalse, &acdWrapper);

	    acdValidAppl(acdNewCurr);
	    
	    /* automatic $(today) variable */
	    ajStrAssignC(&acdStrName, "today");
	    today = ajTimeNewToday();
	    ajFmtPrintS(&acdStrValue, "%D", today);
	    ajTimeDel(&today);
	    acdNewCurr = acdNewVar(acdStrName);
	    acdSetVarDef(acdNewCurr, acdStrValue);
	    break;
	    
	    /* type: qualname alias[ attr: value ]
	     **
	     ** The alias name is optional (defaults to the qualifier name)
	     ** The [] are required so the token can be detected.  Attributes
	     ** are defined for each "type", as are associated
	     ** qualifiers. There is no distinction between them here.  The
	     ** difference is that the qualifier values are defaults which
	     ** can be overridden on the command line
	     */
	    
	case QUAL_STAGE:
	    acdParseAlpha(listwords, &acdStrName);
	    
	    if(acdNotLeftB(listwords))
	    {				/* test badalias.acd */
		if(!acdWordNextLower(listwords, &acdStrAlias) ||
		   !ajStrIsAlpha(acdStrAlias))
		    acdError("Bad syntax qualifier alias name '%S'",
			     acdStrAlias);
	    }
	    else	/* we have an alternate name before the '[' */
		ajStrAssignS(&acdStrAlias, acdStrName);
	    
	    acdNewCurr = acdNewQual(acdStrName, acdStrAlias, &acdStrType);

	    acdWordNum++;		/* add one for '[' */

	    if(!ajStrMatchS(acdStrName, acdStrAlias))
		acdPretty("\n%S: %S %S [\n", acdStrType,
			  acdStrName, acdStrAlias);
	    else
		acdPretty("\n%S: %S [\n", acdStrType,
			  acdStrName);

	    acdWordNum--;		/* not yet parsed '[' */
	    
	    acdParseAttributes(acdNewCurr, listwords);
	    
	    acdValidQual(acdNewCurr);

	    break;
	    
	case SEC_STAGE:
	    /* section: name [ attrlist ] */
	    acdParseName(listwords, &acdStrName);
	    
	    acdNewCurr = acdNewSec(acdStrName);
	    
	    acdWordNum++;		/* add one for '[' */

	    acdPretty("\n%s: %S [\n",
		      acdKeywords[acdCurrentStage].Name, acdStrName);
	    
	    acdWordNum--;		/* not yet parsed '[' */
	    
	    acdParseAttributes(acdNewCurr, listwords);
	    
	    acdValidSection(acdNewCurr);

	    acdPrettyShift();
	    
	    break;
	    
	case ENDSEC_STAGE:
	    /* endsection: name */
	    /* remove from list of current sections */
	    acdParseName(listwords, &acdStrName);
	    acdNewCurr = acdNewEndsec(acdStrName);
	    acdValidSection(acdNewCurr);
	    acdPrettyUnShift();
	    acdPretty("\n%s: %S\n",
		      acdKeywords[acdCurrentStage].Name, acdStrName);
	    break;
	    
	    /* catch-all for failed parsing */
	    
	case VAR_STAGE:
	    /* then the variable name and the value */
	    acdParseName(listwords, &acdStrName);
	    ajStrAssignS(&acdStrValue, acdParseValue(listwords));
	    acdNewCurr = acdNewVar(acdStrName);
	    acdSetVarDef(acdNewCurr, acdStrValue);
	    acdPretty("\n%s: %S \"%S\"\n",
		      acdKeywords[acdCurrentStage].Name,
		      acdStrName, acdStrValue);
	    break;
	    
	case REL_STAGE:
	    /* relation: name [ attrlist ] */
	    acdParseName(listwords, &acdStrName);
	    
	    acdNewCurr = acdNewRel(acdStrName);
	    
	    acdWordNum++;		/* add one for '[' */

	    acdPretty("\n%s: %S [\n",
		      acdKeywords[acdCurrentStage].Name, acdStrName);
	    
	    acdWordNum--;		/* not yet parsed '[' */
	    
	    acdParseAttributes(acdNewCurr, listwords);
	    
	    acdValidRelation(acdNewCurr);

	    acdPrettyShift();
	    
	    break;
	    
	case BAD_STAGE:			/* test badstage.acd */
	default:		     /* Fatal - should never happen */
	    acdError("Unrecognized token '%S'\n", acdStrType);
	    break;
	}
    }
    acdLog("-- All Done --\n");
    
    acdLog("-- All Done : acdSecList length %d\n",
	   ajListstrGetLength(acdSecList));
    
    acdLineNum = linecount;
    
    if(ajListstrGetLength(acdSecList)) /* fatal error, unclosed section(s) */
    {
	while(ajListstrPop(acdSecList, &secname))
	{
	    ajDebug("Section '%S' has no endsection\n", secname);
	    ajErr("Section '%S' has no endsection", secname); /* fails below */
	    ajStrDel(&secname);
	}
	acdLog("Unclosed sections in ACD file\n");
	acdError("Unclosed sections in ACD file"); /* test noendsec.acd */
    }

    acdPrettyClose();
    
    ajStrDel(&acdStrName); /* the global string ... no longer needed */
    
    ajStrDel(&acdStrAlias);
    ajStrDel(&acdStrType);
    ajStrDel(&acdStrValue);
    ajListstrFreeData(&acdSecList);
    
    acdLineNum = 0;
    
    while(ajListGetLength(listcount))
    {
	ajListPop(listcount, (void**) &iword);
	AJFREE(iword);
    }

    return;
}




/* @funcstatic acdParseValue **************************************************
**
** Uses ajStrTok to complete a (possibly) quoted value.
** Note that ajStrTok has a stored internal copy of the text string
** which is set up at the start of acdParse and is being used here.
**
** Quotes can be single or double.
**
** The early versions also allowed any kind of parentheses,
** depending on the first character of the next token examined.
** This is now obsolete, to simplify the syntax and to allow
** future reuse of parentheses.
**
** @param [u] listwords [AjPList] List of strings for each word
**                                      to be parsed
** @return [AjPStr] String containing next value from list
**
** @release 1.0.0
** @@
******************************************************************************/


static AjPStr acdParseValue(AjPList listwords)
{
    AjPStr strp=NULL;
    char  endq[]=" ";
    ajint iquote;
    char *cq;
    AjBool done = ajFalse;
    
    const char *quotes = "\"'";
    const char *endquotes = "\"'";

    acdParseQuotes = ajFalse;

    if(!acdWordNext(listwords, &strp))	/* test: novalue.acd */
	acdErrorAcd(acdNewCurr,
		    "Unexpected end of file, attribute value not found\n");
    
    cq = strchr(quotes, ajStrGetCharFirst(strp));

    if(!cq)				/* no quotes, simple return */
    {
	ajStrAssignS(&acdParseReturn, strp);
	ajStrDel(&strp);
	return acdParseReturn;
    }

    acdParseQuotes = ajTrue;

    /* quote found: parse up to closing quote then strip white space */
    
    iquote = cq - quotes;
    endq[0] = endquotes[iquote];
    
    ajStrCutStart(&strp, 1);
    ajStrDel(&acdParseReturn);
    
    while(!done)
    {
	if(ajStrSuffixC(strp, endq))
	{			       /* check for trailing quotes */
	    ajStrCutEnd(&strp, 1);
	    done = ajTrue;
	}

	if(ajStrGetLen(strp))
	{
	    if(ajStrGetLen(acdParseReturn))
	    {
		ajStrAppendC(&acdParseReturn, " ");
		ajStrAppendS(&acdParseReturn, strp);
	    }
	    else
		ajStrAssignS(&acdParseReturn, strp);
	}
	if(!done)
	    if(!acdWordNext(listwords, &strp)) /* test noquote.acd */
		acdErrorAcd(acdNewCurr,
			    "Unexpected end of file, no closing quote\n");
    }
    
    ajStrDel(&strp);
    return acdParseReturn;
}




/* @funcstatic acdWordNext ****************************************************
**
** Returns the next word from a list
**
** @param [u] listwords [AjPList] List of words parsed from ACD file
** @param [w] pword [AjPStr*] Next word from the list
** @return [AjBool] ajTrue on success
**
** @release 2.7.0
** @@
******************************************************************************/

static AjBool acdWordNext(AjPList listwords, AjPStr* pword)
{
    ajStrDel(pword);

    if(ajListstrPop(listwords, pword))
    {
	acdWordNum++;
	return ajTrue;
    }

    ajStrAssignClear(pword);
    return ajFalse;
}




/* @funcstatic acdWordNextLower ***********************************************
**
** Returns the next word from a list, in lower case.
**
** @param [u] listwords [AjPList] List of words parsed from ACD file
** @param [w] pword [AjPStr*] Next word from the list
** @return [AjBool] ajTrue on success
**
** @release 2.7.0
** @@
******************************************************************************/

static AjBool acdWordNextLower(AjPList listwords, AjPStr* pword)
{
    if(acdWordNext(listwords, pword))
    {
	if(!ajStrIsLower(*pword))
	{
	    acdWarn("Automatically converting '%S' to lower case", *pword);
	    ajStrFmtLower(pword);
	}
	return ajTrue;
    }

    return ajFalse;
}




/* @funcstatic acdWordNextName ************************************************
**
** Returns the next word from a list, in lower case
**
** This must be an ACD name (type or attribute) alphabetic only,
** with a trailing ':'
**
** @param [u] listwords [AjPList] List of words parsed from ACD file
** @param [w] pword [AjPStr*] Next word from the list
** @return [AjBool] ajTrue on success
**
** @release 2.7.0
** @@
******************************************************************************/

static AjBool acdWordNextName(AjPList listwords, AjPStr* pword)
{
    if(acdWordNext(listwords, pword))
    {
	if(ajStrGetCharLast(*pword) != ':')
	{
	    if(ajStrGetCharFirst(*pword) == ':')/* test nocolon.acd */
		acdError("Found ':' at start of word '%S'", *pword);
	    else if(ajStrFindAnyK(*pword, ':') > 0)
		acdError("Expected space missing after ':' in '%S'", *pword);
	    else
		acdError("Expected ':' not found after '%S'", *pword);
	    return ajFalse;
	}
	ajStrCutEnd(pword, 1);
	if(!ajStrIsAlpha(*pword))
	    return ajFalse;
	ajStrFmtLower(pword);
	return ajTrue;
    }

    return ajFalse;
}




/* @funcstatic acdParseName ***************************************************
**
** Returns the next word from a list, in lower case.
**
** This must be an ACD name (alphanumeric allowed as this is not used
** for qualifier names)
**
** @param [u] listwords [AjPList] List of words parsed from ACD file
** @param [w] pword [AjPStr*] Next word from the list
** @return [void]
**
** @release 2.7.0
** @@
******************************************************************************/

static void acdParseName(AjPList listwords, AjPStr* pword)
{
    if(acdWordNextLower(listwords, pword))
	if(ajStrIsAlnum(*pword))
	    return;

    /* test noapplname.acd badapplname.acd */
    acdError("Bad or missing %s name '%S'",
	     acdKeywords[acdCurrentStage].Name, *pword);

    return;
}




/* @funcstatic acdParseAlpha **************************************************
**
** Returns the next word from a list, in lower case.
**
** This must be an alphabetic word, no numbers or underscores allowed.
**
** @param [u] listwords [AjPList] List of words parsed from ACD file
** @param [w] pword [AjPStr*] Next word from the list
** @return [void]
**
** @release 2.7.0
** @@
******************************************************************************/

static void acdParseAlpha(AjPList listwords, AjPStr* pword)
{
    if(acdWordNextLower(listwords, pword))
	if(ajStrIsAlpha(*pword))
	    return;

    /* test noqualname.acd badqualname.acd */
    acdError("Bad or missing %s alphabetic name '%S'",
	     acdKeywords[acdCurrentStage].Name, *pword);

    return;
}




/* @funcstatic acdNotLeftB ****************************************************
**
** Tests the start of the next word in the list for '[' at the start
**
** Does not remove the '['
**
** @param [r] listwords [const AjPList] List of words parsed from ACD file
** @return [AjBool] ajTrue if start of string does not match '['
**
** @release 2.7.0
** @@
******************************************************************************/

static AjBool acdNotLeftB(const AjPList listwords)
{
    char ch;
    AjPStr pstr = NULL;
 
    if(!ajListstrPeek(listwords, &pstr))
	return ajFalse;

    ch = ajStrGetCharFirst(pstr);

    if(ch == '[')
	return ajFalse;

    /* do not delete pstr - we only peeked */

    return ajTrue;
}




/* @funcstatic acdIsLeftB *****************************************************
**
** Tests the start of the next word in the list for '[' at the start
**
** @param [u] listwords [AjPList] List of words parsed from ACD file
** @return [AjBool] ajTrue if start of string matches '['
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdIsLeftB(AjPList listwords)
{
    char ch;
    AjPStr teststr = NULL;
    AjPStr pstr = NULL;
 
    if(!ajListstrPeek(listwords, &teststr))
	return ajFalse;

    ch = ajStrGetCharFirst(teststr);

    if(ch == '[')
    {
	ajStrCutStart(&teststr, 1);	/* trim the leading '[' in the list */
	if(!ajStrGetLen(teststr))
	{		    /* only the '[' so remove from the list */
	    acdWordNext(listwords, &pstr); /*  must succeed - Peeked */
	    ajStrDel(&pstr);	     /* empty - ignored - so delete */
	    teststr = NULL;
	}
	return ajTrue;
    }
    /* do not delete teststr - it was only peeked  - deleted as pstr */

    return ajFalse;
}




/* @funcstatic acdIsRightB ****************************************************
**
** Tests for ']' to look for ascent to a higher level of parsing.
**
** Tests the end of the current string
** If that fails, tests the start of the next word in the list.
**
** Afterwards, the value of pstr is the last word with any ']' removed
**
** @param [w] pstr [AjPStr*] String which has a trailing ']' removed if found
** @param [u] listwords [AjPList] List of remaining words to be parsed
** @return [AjBool] ajTrue if end of string matches ']'
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdIsRightB(AjPStr* pstr, AjPList listwords)
{
    AjPStr teststr = NULL;
    char ch;

    if(*pstr && !acdParseQuotes)
    {
	ch = ajStrGetCharLast(*pstr);

	if(ch == ']')	    /* test input pstr value for ']' at end */
	{
	    ajStrCutEnd(pstr, 1);
	    return ajTrue;
	}
    }

    /* go on to the next word in the list */

    if(!ajListstrPeek(listwords, &teststr)) /* leftend.acd valend.acd */
	acdErrorAcd(acdNewCurr, "End of file looking for ']'");

    ch = ajStrGetCharFirst(teststr);

    if(ch == ']')		       /* next word starts with ']' */
    {
	ajStrCutStart(&teststr, 1);	     /* trim the word - in the list */
	if(!ajStrGetLen(teststr))		/*  only "]" so delete it */
	{
	    acdWordNext(listwords, &acdTmpStr); /* works - used ajListstrPeek */
	    teststr = NULL;
	}
	return ajTrue;
    }

    return ajFalse;
}




/* @funcstatic acdParseAttributes *********************************************
**
** Parse the attribute list for an ACD type
**
** @param [r] acd [const AcdPAcd] Acd object
** @param [u] listwords [AjPList] List of parsed words
** @return [void]
**
** @release 2.7.0
** @@
******************************************************************************/

static void acdParseAttributes(const AcdPAcd acd, AjPList listwords)
{
    AjPStr strAttr = NULL;
    AjPStr strValue = NULL;
    AjPStr strFixValue = NULL;
    AjBool done=ajFalse;
    ajint i=0;

    if(!acdIsLeftB(listwords)) /* test noleftappl.acd noleftsec.acd */
	/* noleftq.acd */
	acdErrorAcd(acdNewCurr, "Failed to find '[' for %s %S\n",
		    acdKeywords[acdCurrentStage].Name, acdStrName);

    acdPrettyShift();

    done = acdIsRightB(&strAttr, listwords); /* could be [ ] */

    /* continue parsing until we reach a true closing ']' character */

    while(!done)
    {
	if(!acdWordNextName(listwords, &strAttr)) /* test: noattname.acd */
	    acdErrorAcd(acdNewCurr, "Bad or missing attribute name '%S'",
			strAttr);

	for (i=0;acdAttrAlias[i].OldName;i++)
	    if (ajStrMatchC(strAttr, acdAttrAlias[i].OldName))
	    {
		ajStrAssignC(&strAttr, acdAttrAlias[i].NewName);
		break;
	    }

	ajStrAssignS(&strValue, acdParseValue(listwords));
	done = acdIsRightB(&strValue, listwords); /* will this be last pair? */

	ajStrAssignS(&strFixValue, strValue);
	acdTextFormat(&strFixValue);
	if(acdCurrentStage == QUAL_STAGE)
	    acdSet(acd, &strAttr, strFixValue);
	else
	    acdSetKey(acd, &strAttr, strFixValue);

	if(done)
	    acdWordNum--;

	acdPrettyWrap(ajStrGetLen(strAttr)+3, "%S: \"%S\"",
		      strAttr, strValue);
	if(done)
	    acdWordNum++;
    }

    acdPrettyUnShift();
    acdPretty("]\n");

    ajStrDel(&strAttr);
    ajStrDel(&strValue);
    ajStrDel(&strFixValue);

    return;
}




/* @funcstatic acdNewAppl *****************************************************
**
** Constructor front end for an application ACD object.
**
** @param [r] name [const AjPStr] Token name to be used by applications
** @return [AcdPAcd] ACD application object for name.
**
** @release 1.0.0
** @@
******************************************************************************/

static AcdPAcd acdNewAppl(const AjPStr name)
{
    AcdPAcd acd;
    AcdPAcd qacd;
    AcdPAcd saveqacd = NULL;
    AcdPQual quals;
    AjPStr qname = NULL;
    AjPStr qtype = NULL;
    static ajint firstcall = 1;
    static ajint ikey;
    ajint i;

    if(firstcall)
    {
	ikey = acdFindKeyC("application");
	firstcall = 0;
    }

    i = 0;
    quals = acdQualAppl;

    if(quals)
    {
	while(quals[i].Name)
	{
	    ajStrAssignC(&qname, quals[i].Name);
	    ajStrAssignC(&qtype, quals[i].Type);
	    /*   qacd = acdNewQual(qname, qname, &qtype);*/
	    qacd = acdNewQualQual(qname, &qtype);
	    if(*quals[i].Default)
		acdSetDefC(qacd, quals[i].Default);

	    if(!i)
		saveqacd = qacd; /* save the location of the first one */
	    i++;
	}
    }

    acd = acdNewAcdKey(name, name, ikey);
    acd->Level = ACD_APPL;
    if(saveqacd)
        acd->AssocQuals = saveqacd;

    ajStrDel(&qname);
    ajStrDel(&qtype);

    return acd;
}




/* @funcstatic acdNewVar ******************************************************
**
** Constructor front end for a variable ACD object.
**
** @param [r] name [const AjPStr] Token name to be used by applications
** @return [AcdPAcd] ACD variable object for name.
**
** @release 1.0.0
** @@
******************************************************************************/

static AcdPAcd acdNewVar(const AjPStr name)
{
    AcdPAcd acd;
    static ajint firstcall = 1;
    static ajint ikey;

    if(firstcall)
    {
	ikey = acdFindKeyC("variable");
	firstcall = 0;
    }

    acd = acdNewAcdKey(name, name, ikey);
    acd->Level = ACD_VAR;

    return acd;
}




/* @funcstatic acdNewRel ******************************************************
**
** Constructor front end for a relation ACD object.
**
** @param [r] name [const AjPStr] Token name to be used by applications
** @return [AcdPAcd] ACD variable object for name.
**
** @release 2.9.0
** @@
******************************************************************************/

static AcdPAcd acdNewRel(const AjPStr name)
{
    AcdPAcd acd;
    static ajint firstcall = 1;
    static ajint ikey;

    if(firstcall)
    {
	ikey = acdFindKeyC("relation");
	firstcall = 0;
    }

    acdLog("acdNewRel '%S'\n", name);

    acd = acdNewAcdKey(name, name, ikey);
    acd->Level = ACD_RELATION;

    return acd;
}




/* @funcstatic acdNewSec ******************************************************
**
** Constructor front end for a section ACD object.
**
** @param [r] name [const AjPStr] Token name to be used by applications
** @return [AcdPAcd] ACD variable object for name.
**
** @release 2.0.0
** @@
******************************************************************************/

static AcdPAcd acdNewSec(const AjPStr name)
{
    AcdPAcd acd;
    static ajint firstcall = 1;
    static ajint ikey;
    AjPStr secname=NULL;
    AjPStr secfull=NULL;
    AjPStr secfullvalue=NULL;
    
    if(firstcall)
    {
	ikey = acdFindKeyC("section");
	firstcall = 0;
    }

    acdLog("acdNewSec '%S' acdSecList length %d\n",
	   name, ajListstrGetLength(acdSecList));

    acd = acdNewAcdKey(name, name, ikey);
    acd->Level = ACD_SEC;

    ajStrAssignS(&secname, name);
    ajListstrPush(acdSecList, secname);

    acdValidSectionFull(&secfull);
    ajStrAssignS(&secfullvalue, name);
    acdLog("Full section '%S'\n", secfull);

    if (ajTablePut(acdSecTable, secfull, secfullvalue))
	acdError("Duplicate section '%S'", name);

    acdLog("acdNewSec acdSecList push '%S' new length %d\n",
	   secname, ajListstrGetLength(acdSecList));

    return acd;
}




/* @funcstatic acdNewEndsec ***************************************************
**
** Constructor front end for an end of section ACD object.
**
** @param [r] name [const AjPStr] Token name to be used by applications
** @return [AcdPAcd] ACD variable object for name.
**
** @release 2.0.0
** @@
******************************************************************************/

static AcdPAcd acdNewEndsec(const AjPStr name)
{
    AcdPAcd acd;
    static ajint firstcall = 1;
    static ajint ikey;
    AjPStr secname=NULL;

    if(firstcall)
    {
	ikey = acdFindKeyC("endsection");
	firstcall = 0;
    }

    acdLog("acdNewEndsec '%S' acdSecList length %d\n",
	   name, ajListstrGetLength(acdSecList));

    if(!ajListstrGetLength(acdSecList))	/* test endsecextra.acd */
    {
	acdLog("Bad endsection '%S', not in a section\n", name);
	acdError("Bad endsection '%S', not in a section", name);
    }
    else
    {
	ajListstrPop(acdSecList, &secname);
	acdLog("Pop from acdSecList '%S' new length %d\n",
	       secname, ajListstrGetLength(acdSecList));

	if(!ajStrMatchS(name, secname))	/* test badendsec.acd */
	{
	    acdLog("Bad endsection '%S', current section is '%S\n'",
		   name, secname);
	    acdError("Bad endsection '%S', current section is '%S'",
		     name, secname);
	}
	ajStrDel(&secname);
    }

    acd = acdNewAcdKey(name, name, ikey);
    acd->Level = ACD_ENDSEC;

    return acd;
}




/* @funcstatic acdNewQual *****************************************************
**
** Constructor front end for a qualifier ACD object.
**
** @param [r] name [const AjPStr] Token name to be used by applications
** @param [r] token [const AjPStr] Qualifier name to be used on command line
** @param [u] type [AjPStr*] Type of value to be defined. Expanded to full
**                           type name.
** @return [AcdPAcd] ACD parameter object for name.
**
** @release 1.0.0
** @@
******************************************************************************/

static AcdPAcd acdNewQual(const AjPStr name, const AjPStr token,
			  AjPStr* type)
{
    AcdPAcd acd;
    AcdPAcd qacd;
    AcdPAcd vacd;
    AcdPAcd saveqacd = NULL;
    AcdPQual quals;
    AjPStr protName = NULL;
    AjPStr qname = NULL;
    AjPStr qtype = NULL;
    ajint itype;
    ajint i;
    
    itype = acdFindType(*type);
    ajStrAssignC(type, acdType[itype].Name);
    
    /* do any associated qualifiers first so they are already complete
       when we come to the parameter later in processing */
    
    i = 0;
    quals = acdType[itype].Quals; /* any associated qualifiers for itype?  */
    if(quals)
	while(quals[i].Name)
	{
	    ajStrAssignC(&acdQNameTmp, quals[i].Name);
	    ajStrAssignC(&acdQTypeTmp, quals[i].Type);
	    qacd = acdNewQualQual(acdQNameTmp, &acdQTypeTmp);
	    if(*quals[i].Default)
		acdSetDefC(qacd, quals[i].Default);
	    if(!i)
		saveqacd = qacd; /* save the location of the first one */
	    i++;
	}

    
    /* 
     **  now set up the new parameter, and link in the list of qualifiers
     ** (if any) from earlier
     */
    
    acdTestUnknown(name, token);
    acd = acdNewAcd(name, token, itype);
    acd->Level = ACD_QUAL;

    if(saveqacd)
	acd->AssocQuals = saveqacd;
    
    ajStrDel(&qname);
    ajStrDel(&qtype);
    
    /*
    ** For the first sequence, set the sequence type variable
    */

    if(!ajStrGetLen(acdVarAcdProtein))
	if((acdType[itype].Attr == acdAttrSeq) ||
	   (acdType[itype].Attr == acdAttrSeqall) ||
	   (acdType[itype].Attr == acdAttrSeqset) ||
	   (acdType[itype].Attr == acdAttrSeqsetall) ||
	   (acdType[itype].Attr == acdAttrFeatures))
	{
	    ajStrAssignC(&protName, "acdprotein");
	    ajFmtPrintS(&acdVarAcdProtein, "$(%S.protein)", name);
	    vacd = acdNewVar(protName);
	    acdSetVarDef(vacd, acdVarAcdProtein);
	    ajDebug("Set acdprotein value '%S'\n", acdVarAcdProtein);
	    ajStrDel(&protName);
	}

    ajStrDel(&protName);
    return acd;
}




/* @funcstatic acdNewQualQual *************************************************
**
** Constructor front end for an associated qualifier ACD object.
**
** @param [r] name [const AjPStr] Qualifier name to be used on command line
** @param [u] type [AjPStr*] Type of value to be defined. Expanded to full
**                           type name.
** @return [AcdPAcd] ACD parameter object for name.
**
** @release 1.0.0
** @@
******************************************************************************/

static AcdPAcd acdNewQualQual(const AjPStr name, AjPStr* type)
{
    AcdPAcd acd;
    ajint itype;

    itype = acdFindType(*type);
    ajStrAssignC(type, acdType[itype].Name);

    acdTestAssocUnknown(name);
    acd = acdNewAcd(name, name, itype);
    acd->Level = ACD_QUAL;
    acd->Assoc = ajTrue;

    return acd;
}




/* @funcstatic acdNewAcd ******************************************************
**
** General constructor for a new ACD qualifier object. Initialises all values
** in the ACD structure as appropriate.
**
** @param [r] name [const AjPStr] Token name to be used by applications
** @param [r] token [const AjPStr] Qualifier name to be used on command line
** @param [r] itype [ajint] Integer type of value to be defined
**        as defined in acdFindType
** @return [AcdPAcd] ACD parameter object for name.
**
** @release 1.0.0
** @@
******************************************************************************/

static AcdPAcd acdNewAcd(const AjPStr name, const AjPStr token, ajint itype)
{
    ajint i;

    if(acdListLast)
	acdListLast = AJNEW0(acdListLast->Next);
    else
	acdListLast = AJNEW0(acdList);

    acdListLast->LineNum = acdLineNum;
    ajStrAssignS(&acdListLast->Name, name);
    ajStrAssignS(&acdListLast->Token, token);
    acdListLast->Type = itype;
    ++(*acdType[itype].UseCount);
    ++(*acdType[itype].UseClassCount);

    /* we do NAttr and AttrStr explicitly for clarity, */
    /* though they are 0 and NULL from the AJNEW0 */

    switch(acdCurrentStage)
    {
    case QUAL_STAGE:
	acdListLast->NAttr = acdAttrCount(itype);
	break;
    default:
	acdListLast->NAttr = 0;
	break;
    }

    if(acdListLast->NAttr)
    {
	acdListLast->AttrStr = AJCALLOC(acdListLast->NAttr, sizeof(AjPStr));
	for(i = 0; i < acdListLast->NAttr; i++)
	    acdListLast->AttrStr[i] = ajStrNew();
    }
    else
	acdListLast->AttrStr = NULL;

    acdListLast->DefStr = AJCALLOC(nDefAttr, sizeof(AjPStr));
    for(i = 0; i < nDefAttr; i++)
	acdListLast->DefStr[i] = ajStrNew();

    acdListLast->Defined = ajFalse;
    acdListLast->Assoc = ajFalse;
    acdListLast->ValStr = NULL;

    return acdListLast;
}




/* @funcstatic acdNewAcdKey ***************************************************
**
** General constructor for a new ACD general object. Initialises all values
** in the ACD structure as appropriate.
**
** @param [r] name [const AjPStr] Token name to be used by applications
** @param [r] token [const AjPStr] Qualifier name to be used on command line
** @param [r] ikey [ajint] Integer type of value to be defined
**        as defined in acdFindKey
** @return [AcdPAcd] ACD parameter object for name.
**
** @release 1.0.0
** @@
******************************************************************************/

static AcdPAcd acdNewAcdKey(const AjPStr name, const AjPStr token, ajint ikey)
{
    ajint i;
    
    if(acdListLast)
	acdListLast = AJNEW0(acdListLast->Next);
    else
	acdListLast = AJNEW0(acdList);

    acdListLast->Next = NULL;
    acdListLast->LineNum = acdLineNum;
    
    ajStrAssignS(&acdListLast->Name, name);
    ajStrAssignS(&acdListLast->Token, token);
    acdListLast->PNum = 0;
    acdListLast->Level = ACD_APPL;
    acdListLast->Type = ikey;

    switch(acdCurrentStage)
    {
    case APPL_STAGE:
	acdListLast->NAttr = acdAttrKeyCount(ikey);
	break;
    case VAR_STAGE:
	acdListLast->NAttr = acdAttrKeyCount(ikey);
	break;
    case SEC_STAGE:
	acdListLast->NAttr = acdAttrKeyCount(ikey);
	break;
    default:
	acdListLast->NAttr = 0;
	break;
    }
    
    if(acdListLast->NAttr)
    {
	acdListLast->AttrStr = AJCALLOC(acdListLast->NAttr, sizeof(AjPStr));
	for(i = 0; i < acdListLast->NAttr; i++)
	    acdListLast->AttrStr[i] = ajStrNew();
    }
    else
	acdListLast->AttrStr = NULL;
    
    acdListLast->DefStr = NULL;
    
    acdListLast->SAttr = 0;
    acdListLast->SetAttr = NULL;
    acdListLast->SetStr = NULL;
    
    acdListLast->Defined = ajFalse;
    acdListLast->Assoc = ajFalse;
    acdListLast->AssocQuals = NULL;
    acdListLast->ValStr = NULL;
    acdListLast->Value = NULL;
    
    return acdListLast;
}




/* @funcstatic acdDel *********************************************************
**
** General destructor for any ACD object.
**
** @param [d] Pacd [AcdPAcd*] Acd object
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

static void acdDel(AcdPAcd *Pacd)
{
    AcdPAcd pa = *Pacd;
    ajint i;

    if(pa->AttrStr)
    {
	for(i = 0; i < pa->NAttr; i++)
	    ajStrDel(&pa->AttrStr[i]);
	AJFREE(pa->AttrStr);
    }

    if(pa->DefStr)
    {
	for(i = 0; i < nDefAttr; i++)
	    ajStrDel(&pa->DefStr[i]);
	AJFREE(pa->DefStr);
    }

    if(pa->SetStr)
    {
	for(i = 0; i < pa->SAttr; i++)
	    ajStrDel(&pa->SetStr[i]);
	AJFREE(pa->SetStr);
    }

/* for variables and relations, clear the value */

    /*if(pa->Level ==ACD_VAR || pa->Level == ACD_RELATION )
	ajStrDel(&pa->Value);
    else if(!pa->PassByRef)*/

    if(pa->Assoc && ajCharMatchC(acdType[pa->Type].Name,"string"))
    {
	ajStrDel((AjPStr*)&pa->Value);
    }
    else if(pa->Level == ACD_QUAL || pa->Level == ACD_PARAM)
    {
	if(!acdType[pa->Type].PassByRef)
	{
	    AJFREE(pa->Value);
	}
	else if(pa->RefPassed != REF_ALL) /* pass-by-ref but never passed */
	{
	    if(pa->RefPassed == REF_NONE)
	    {
		(*acdType[pa->Type].TypeDel)(&pa->Value);
	    }
	    else
	    {
		AJFREE(pa->Value);
	    }
	}
    }


    ajStrDel(&pa->Name);
    ajStrDel(&pa->Token);
    ajStrDel(&pa->StdPrompt);
    ajStrDel(&pa->OrigStr);
    ajStrDel(&pa->ValStr);

    AJFREE(*Pacd);

    return;
}




/* @funcstatic acdTestUnknown *************************************************
**
** Makes sure that a name, token and pnum do not match any
** current ACD object.
**
** Aborts the program with a fatal error in case of problems.
**
** @param [r] name [const AjPStr] Token name to be used by applications
** @param [r] alias [const AjPStr] Qualifier name to be used on command line
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdTestUnknown(const AjPStr name, const AjPStr alias)
{
    AcdPAcd pa;
    
    pa = acdFindAcdTest(name, alias);

    if(!pa)
      return;

    if(ajStrMatchS(name, alias))
      acdErrorAcd(pa, "Name '%S' not unique at line %d\n",
		  name, acdLineNum);
    else
      acdErrorAcd(pa,
		  "Name/Alias '%S'/'%S' not unique at line %d\n",
		  name, alias, acdLineNum);
    return;
}




/* @funcstatic acdCountType ***************************************************
**
** Counts number of qualifiers with a given type.
**
** @param [r] type [const char*] ACD type
** @return [ajint] Number of qualifiers of this type
**
** @release 4.0.0
** @@
******************************************************************************/

static ajint acdCountType(const char* type)
{
    ajint ret = 0;
    AcdPAcd pa;
    ajint itype = 0;

    itype = acdFindTypeC(type);

    for(pa=acdList; pa; pa=pa->Next)
    {
	if(pa->Type == itype) ret++;
    }

    return ret;
}




/* @funcstatic acdTestAssocUnknown ********************************************
**
** Makes sure that a name does not match any known ACD object name or token
** for all associated qualifiers.
**
** Aborts the program with a fatal error in case of problems.
**
** @param [r] name [const AjPStr] Name or token name
** @return [void]
**
** @release 2.7.0
** @@
******************************************************************************/

static void acdTestAssocUnknown(const AjPStr name)
{
    AcdPAcd pa;

    for(pa=acdList; pa; pa=pa->Next)
    {
	if(acdIsStype(pa))
            continue;

	if(!pa->Assoc &&(ajStrMatchS(pa->Name, name) ||
			 ajStrMatchS(pa->Token, name)))
	{
	    if(ajStrMatchS(pa->Name, pa->Token)) /* test: dupassoc.acd */
		acdErrorAcd(pa,
			    "Associated qualifier '%S' clashes with '%S' "
			    "in ACD file\n",
			    name, pa->Name);
	    else
		acdErrorAcd(pa,		/* test: dupassoc2.acd */
			    "Associated qualifier '%S' clashes with '%S'/'%S' "
			    "in ACD file\n",
			    name, pa->Name, pa->Token);
	    break;
	}
    }

    return;
}




/* @funcstatic acdFindAcd *****************************************************
**
** Locates an ACD object by name, token and parameter number.
**
** @param [r] name [const AjPStr] Token name to be used by applications
** @param [r] token [const AjPStr] Qualifier name to be used on command line
** @return [AcdPAcd] ACD object or NULL if not found
**
** @release 1.0.0
** @@
******************************************************************************/

static AcdPAcd acdFindAcd(const AjPStr name, const AjPStr token)
{
    AcdPAcd pa;
    AjBool sametoken = ajFalse;

    sametoken = ajStrMatchS(name, token);

    acdLog("acdFindAcd ('%S', '%S', %d)\n", name, token);

    for(pa=acdList; pa; pa=pa->Next)
    {
      if(acdIsStype(pa))
	continue;

      if(ajStrMatchS(pa->Name, name))
      {
	acdLog("..found '%S' %d\n", pa->Name, pa->PNum);
	return pa;
      }

      if(ajStrMatchS(pa->Token, name))
      {
	acdLog("..found '%S' %d\n", pa->Name, pa->PNum);
	return pa;
      }

      if(!sametoken)
	continue;

      if(ajStrMatchS(pa->Name, token))
      {
	acdLog("..found '%S' %d\n", pa->Name, pa->PNum);
	return pa;
      }

      if(ajStrMatchS(pa->Token, token))
      {
	acdLog("..found '%S' %d\n", pa->Name, pa->PNum);
	return pa;
      }
    }

    return NULL;
}




/* @funcstatic acdFindAcdTest *************************************************
**
** Locates an ACD object by name, and (if different) by token.
**
** Tests for unique 6 character prefix and issues a warning if
** a clash is found with any already defined qualifier.
**
** All other tests are the same as in acdFindAcd
**
** @param [r] name [const AjPStr] Token name to be used by applications
** @param [r] token [const AjPStr] Qualifier name to be used on command line
** @return [AcdPAcd] ACD object or NULL if not found
**
** @release 6.1.0
** @@
******************************************************************************/

static AcdPAcd acdFindAcdTest(const AjPStr name, const AjPStr token)
{
    AcdPAcd savepa = NULL;
    AcdPAcd pa;
    AjBool usetoken = ajFalse;

    ajuint minunique=7;

    ajStrAssignSubS(&acdPrefName, name, 0, minunique-1);

    usetoken = !ajStrMatchS(name, token);

    if(usetoken)
    {
        ajStrAssignSubS(&acdPrefToken, token, 0, minunique-1);
        acdLog("acdFindAcdTest ('%S' ['%S'], '%S' ['%S'])\n",
               name, acdPrefName, token, acdPrefToken);
    }
    else
    {
        acdLog("acdFindAcdTest ('%S' ['%S'])\n",
               name, acdPrefName);
    }

    for(pa=acdList; pa; pa=pa->Next)
    {
        if(acdIsStype(pa))
        {
            if(ajStrMatchS(pa->Name, name))
                acdWarn("Qualifier '%S' matches known section '%S'",
                        name, pa->Name);
            continue;
        }

        if(acdIsAtype(pa))
        {
            if(ajStrMatchS(pa->Name, name))
                acdWarn("Qualifier '%S' matches application name '%S'",
                        name, pa->Name);
            continue;
        }

        if(acdIsVtype(pa))
        {
            if(ajStrMatchS(pa->Name, name))
                acdWarn("Qualifier '%S' matches variable name '%S'",
                        name, pa->Name);
            continue;
        }

        if(ajStrMatchS(pa->Name, name))
        {
            acdLog("..found '%S' %d\n", pa->Name, pa->PNum);
            savepa = pa;
        }
        else if(ajStrPrefixS(pa->Name, acdPrefName))
        {
            if(!acdWrapper)
                acdWarn("Qualifier '%S' matches start of known '%S'",
                        name, pa->Name);
        }
        else if(ajStrPrefixS(acdPrefName, pa->Name))
        {
            if(!acdWrapper)
                acdWarn("Known qualifier '%S' matches start of '%S'",
                        pa->Name, name);
        }


        if(!ajStrMatchS(pa->Token, pa->Name)) /* qualifier has a token */
        {
            if(ajStrMatchS(pa->Token, name))
            {
                acdLog("..found '%S' %d\n", pa->Name, pa->PNum);
                savepa = pa;
            }
            else if(ajStrPrefixS(pa->Token, acdPrefName))
            {
                if(!acdWrapper)
                    acdWarn("Qualifier '%S' matches start of known token '%S'",
                            name, pa->Token);
            }
        }

        if(!usetoken)
            continue;

        if(ajStrMatchS(pa->Name, token))
        {
            acdLog("..found '%S' %d\n", pa->Name, pa->PNum);
            savepa = pa;
        }
        else if(ajStrPrefixS(acdPrefToken, pa->Name))
        {
            if(!acdWrapper)
                acdWarn("Qualifier token '%S' matches start of known '%S'",
                        token, pa->Name);
        }
        else if(ajStrPrefixS(pa->Name, acdPrefToken))
        {
            if(!acdWrapper)
                acdWarn("Known qualifier '%S' matches start of token '%S'",
                        pa->Name, token);
        }

        if(!ajStrMatchS(pa->Token, pa->Name)) /* qualifier has a token */
        {
            if(ajStrMatchS(pa->Token, token))
            {
                acdLog("..found '%S' %d\n", pa->Name, pa->PNum);
                savepa = pa;
            }
            else if(ajStrPrefixS(pa->Token, acdPrefToken))
            {
                if(!acdWrapper)
                    acdWarn("Qualifier token '%S' matches start of "
                            "known token '%S'",
                            token, pa->Token);
            }
        }
    }

    return savepa;
}




/* @funcstatic acdFindAssoc ***************************************************
**
** Locates an ACD object for an associated qualifier by name.
**
** Aborts the program with a fatal error in case of problems.
**
** Used in defining ACD objects and in processing the commandline.
**
** @param [r] thys [const AcdPAcd] ACD object for the parameter
** @param [r] name [const AjPStr] Token name to be used by applications
** @param [r] noname [const AjPStr] Alternative token name (e.g. qualifier
**            with "no" prefix removed)
** @return [AcdPAcd] ACD object for the selected qualifier
**
** @release 1.0.0
** @@
******************************************************************************/

static AcdPAcd acdFindAssoc(const AcdPAcd thys, const AjPStr name,
			    const AjPStr noname)
{
    AcdPAcd pa;
    ajint ifound=0;
    AcdPAcd ret=NULL;
    AjPStr ambigList = NULL;

    ambigList = ajStrNew();

    for(pa=thys->AssocQuals; pa && pa->Assoc; pa=pa->Next)
	if(ajStrPrefixS(pa->Name, name) ||
	   ajStrPrefixS(pa->Name, noname))
	{
	    if(ajStrMatchS(pa->Name, name) ||
	       ajStrMatchS(pa->Name, noname))
		return pa;
	    ifound++;
	    ret = pa;
	    acdAmbigApp(&ambigList, pa->Name);
	}



    if(ifound == 1)
    {
	if (acdDoValid)
	    acdWarn("Abbreviated associated qualifier '%S' (%S)",
		    name, ambigList);
	ajStrDel(&ambigList);
	return ret;
    }

    if(ifound > 1)
    {
	ajWarn("Ambiguous name/token '%S' (%S)", name, ambigList);
	acdErrorAcd(thys,		/* ambigdefattr.acd */
		    "Attribute or qualifier '%S' ambiguous (%S)\n",
		    name, ambigList);
	ajStrDel(&ambigList);
    }

    ajStrDel(&ambigList);

    return NULL;
}




/* @funcstatic acdTestAssoc ***************************************************
**
** Locates an ACD object for an associated qualifier by name.
**
** Only tests silently for a possible qualifier.
**
** If this fails, we check properly later.
**
** Used in defining ACD objects and in processing the commandline.
**
** @param [r] thys [const AcdPAcd] ACD object for the parameter
** @param [r] name [const AjPStr] Token name to be used by applications
** @param [r] noname [const AjPStr] Alternative token name (e.g. qualifier
**            with "no" prefix removed)
** @return [AcdPAcd] ACD object for the selected qualifier
**
** @release 1.0.0
** @@
******************************************************************************/

static AcdPAcd acdTestAssoc(const AcdPAcd thys, const AjPStr name,
			    const AjPStr noname)
{
    AcdPAcd pa;
    ajint ifound=0;
    AcdPAcd ret=NULL;
    AjPStr ambigList = NULL;

    ambigList = ajStrNew();

    for(pa=thys->AssocQuals; pa && pa->Assoc; pa=pa->Next)
	if(ajStrPrefixS(pa->Name, name) ||
	   ajStrPrefixS(pa->Name, noname))
	{
	    if(ajStrMatchS(pa->Name, name) ||
	       ajStrMatchS(pa->Name, noname))
	    {
		ajStrDel(&ambigList);
		return pa;
	    }
	    ifound++;
	    ret = pa;
	}

    if(ifound == 1)
    {
	if (acdDoValid)
	    acdWarn("Abbreviated associated qualifier '%S' (%S)",
		    name, ambigList);
	ajStrDel(&ambigList);
	return ret;
    }

    ajStrDel(&ambigList);

    return NULL;
}




/* @funcstatic acdTestQualC ***************************************************
**
** Tests whether "name" is a valid qualifier name.
** To be valid, it must begin with "-" or '/'.
** If not, it can be taken as a value for the previous qualifier
**
** Used after a boolean option to check for a possible value
**
** Should run silently - if not valid, we will test it next anyway
**
** @param [r] name [const char*] Qualifier name
** @return [AjBool] ajTrue if
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdTestQualC(const char *name)
{
    static AjPStr  qstr = NULL;
    static AjPStr  qnostr = NULL;
    static AjPStr  qmaster = NULL;
    AcdPAcd pa;
    AcdPAcd qa;
    AcdPAcd savepa=NULL;
    ajint qnum = 0;
    ajlong i;
    ajint ifound=0;
    AjPStr ambigList = NULL;
    
    acdLog("acdTestQualC '%s'\n", name);
    
    if(*name != '-' && *name != '/' && !strstr(name, "="))
	return ajFalse;		/* not a qualifier name             */
    
    ambigList = ajStrNew();
    
    ajStrAssignC(&qstr, name+1);   /* lose the - or / prefix           */
    
    i = ajStrFindC(qstr, "=");	/* qualifier with value             */

    if(i > 0)
	ajStrKeepRange(&qstr, 0, i-1); /* strip any value and keep testing */
    
    if(ajStrPrefixC(qstr, "no")) /* check for -no qualifiers          */
	ajStrAssignSubS(&qnostr, qstr, 2, -1);
    else
	ajStrAssignClear(&qnostr);
    
    acdQualParse(&qstr, &qnostr, &qmaster, &qnum);
    
    if(ajStrGetLen(qmaster))      /* master specified as -qstr_qmaster */
    {
	for(pa=acdList; pa; pa=pa->Next)
	{
	    if(acdIsStype(pa)) continue;
	    if(ajStrMatchS(pa->Name, qmaster))
	    {
		acdLog("  *master matched* '%S'\n", pa->Name);
		qa = acdTestAssoc(pa, qstr, qnostr);
		ajStrDel(&ambigList);
		if(qa)
		    return ajTrue;
		else
		    return ajFalse;
	    }
	}

	if(ajStrPrefixS(pa->Name, qstr))
	{
	    ifound++;
	    savepa = pa;
	    acdAmbigApp(&ambigList, pa->Name);
	}
	acdLog("   ifound: %d\n", ifound);

	if(ifound == 1)
	{
	    qa = acdTestAssoc(savepa, qstr, qnostr);
	    ajStrDel(&ambigList);
	    ajStrDel(&qstr);
	    ajStrDel(&qnostr);
	    ajStrDel(&qmaster);
	    if(qa)
		return ajTrue;
	    else
		return ajFalse;
	}

	if(ifound > 1)		/* master should be checked earlier */
	{
	    /* ajWarn("Ambiguous associated qualifier '%s' (%S)",
	       name, ambigList);
	       ajStrDel(&ambigList); */
	    ajStrDel(&ambigList);
	    ajStrDel(&qstr);
	    ajStrDel(&qnostr);
	    ajStrDel(&qmaster);
	    return ajFalse;
	}
    }
    else			       /* just qualifier name -qstr */
    {
	for(pa=acdList; pa; pa=pa->Next)
	{
	    if(acdIsStype(pa))
                continue;

	    if(ajStrMatchS(pa->Name, qstr))
	    {
		acdLog("   *matched* '%S'\n", pa->Name);
		ajStrDel(&ambigList);
		ajStrDel(&qstr);
		ajStrDel(&qnostr);
		ajStrDel(&qmaster);
		return ajTrue;
	    }

	    if(ajStrPrefixS(pa->Name, qstr))
	    {
		ifound++;
		acdAmbigApp(&ambigList, pa->Name);
	    }
	}

	acdLog("   ifound: %d\n", ifound);

	if(ifound == 1)
	{
	    ajStrDel(&ambigList);
	    ajStrDel(&qstr);
	    ajStrDel(&qnostr);
	    ajStrDel(&qmaster);
	    return ajTrue;
	}

	if(ifound > 1)
	{
	    /* ajWarn("Ambiguous qualifier '%s' (%S)", name, ambigList);
	       ajStrDel(&ambigList); */
	    ajStrDel(&ambigList);
	    ajStrDel(&qstr);
	    ajStrDel(&qnostr);
	    ajStrDel(&qmaster);
	    return ajFalse;
	}
    }
    
    ajStrDel(&ambigList);
    ajStrDel(&qstr);
    ajStrDel(&qnostr);
    ajStrDel(&qmaster);

    return ajFalse;
}




/* @funcstatic acdFindType ****************************************************
**
** Looks for a Type by name, and returns the number in acdType
**
** @param [r] type [const AjPStr] String containing the type name
** @return [ajint] Integer representing the type (if know). Can be
**         used as position in the acdType array.
** @error If not found, the return value points to the maximum position in
**        acdType which is set to NULL throughout.
**
** @release 1.0.0
** @@
******************************************************************************/

static ajint acdFindType(const AjPStr type)
{
    ajint i;
    ajint ifound = 0;
    ajint j = 0;
    AjPStr ambigList = NULL;

    ambigList = ajStrNew();

    for(i=0; acdType[i].Name; i++)
    {
	if(ajStrMatchC(type, acdType[i].Name))
	{
	    ajStrDel(&ambigList);
	    return i;
	}

	if(ajCharPrefixS(acdType[i].Name, type))
	{
	    ifound++;
	    j = i;
	    acdAmbigAppC(&ambigList, acdType[i].Name);
	}
    }

    if(ifound > 1)
    {			      /* warn now with the list, fail below */
	ajWarn("ambiguous type %S (%S)", type, ambigList);
    }

    if(ifound != 1)	       /* Fatal: but covered by other tests */
	acdError("unknown type: '%S'\n", type);

    ajStrDel(&ambigList);

    return j;
}




/* @funcstatic acdFindTypeC ***************************************************
**
** Looks for a Type by name, and returns the number in acdType
**
** @param [r] type [const char*] Text string containing the type name
** @return [ajint] Integer representing the type (if known). Can be
**         used as position in the acdType array.
** @error If not found, the return value points to the maximum position in
**        acdType which is set to NULL throughout.
**
** @release 1.0.0
** @@
******************************************************************************/

static ajint acdFindTypeC(const char* type)
{
    ajint i;
    ajint ifound=0;
    ajint j=-1;
    ajint ilen = strlen(type);
    AjPStr ambigList = NULL;

    ambigList = ajStrNew();

    for(i=0; acdType[i].Name; i++)
    {
	if(!strcmp(type, acdType[i].Name))
	{
	    ajStrDel(&ambigList);
	    return i;
	}

	if(!strncmp(acdType[i].Name, type, ilen))
	{
	    ifound++;
	    j = i;
	    acdAmbigAppC(&ambigList, acdType[i].Name);
	}
    }

    if(ifound > 1)
    {			      /* warn now with the list, fail below */
	ajWarn("ambiguous type %s (%S)", type, ambigList);
	ajStrDel(&ambigList);
    }

    if(ifound != 1)	       /* Fatal: but covered by other tests */
	acdError("unknown type: '%s'\n", type);

    ajStrDel(&ambigList);

    return j;
}




/* @funcstatic acdFindKeyC ****************************************************
**
** Looks for a Keyword by name, and returns the number in acdKeywords
***
** @param [r] key [const char*] Text string containing the keyword name
** @return [ajint] Integer representing the keyword (if known). Can be
**         used as position in the acdKeywords array.
** @error If not found, the return value points to the maximum position in
**        acdKeywords which is set to NULL throughout.
**
** @release 1.0.0
** @@
******************************************************************************/

static ajint acdFindKeyC(const char* key)
{
    ajint i;
    ajint ifound=0;
    ajint j=0;
    ajint ilen = strlen(key);
    AjPStr ambigList = NULL;

    ajStrAssignClear(&ambigList);

    for(i=QUAL_STAGE+1; acdKeywords[i].Name; i++)
    {
	if(!strcmp(key, acdKeywords[i].Name))
	{
	    ajStrDel(&ambigList);
	    return i;
	}

	if(strncmp(acdKeywords[i].Name, key, ilen))
	{
	    ifound++;
	    j = i;
	    acdAmbigAppC(&ambigList, acdKeywords[i].Name);
	}
    }

    if(ifound > 1)
    {
	ajWarn("ambiguous keyword %s (%S)", key, ambigList);
    }

    if(ifound != 1)		/* Fatal: but strings are hard coded */
	acdError("unknown keyword: '%s'\n", key);

    ajStrDel(&ambigList);

    return j;
}




/*===========================================================================*/
/*======================== Talking to the User ==============================*/
/*===========================================================================*/




/* @funcstatic acdReplyInitC **************************************************
**
** Builds a default value for the reply first time around. Uses a default
** specially set in the ACD, or (if none) uses the default string passed in
** parameter "defval" and also sets this as the default in the ACD.
**
** @param [r] thys [const AcdPAcd] ACD object for current item.
** @param [r] defval [const char*] Default value, as a C string
** @param [w] reply [AjPStr*] String containing default reply
** @return [AjBool] ajTrue if a value in the ACD was used.
**
** @release 6.1.0
** @@
******************************************************************************/

static AjBool acdReplyInitC(const AcdPAcd thys, const char *defval,
                            AjPStr* reply)
{
    AjPStr def;

    if(thys->DefStr)
    {
	def = thys->DefStr[DEF_DEFAULT];
	acdLog("acdReplyInitC '%S' : '%S'\n", thys->Name, def);

	if(ajStrGetLen(def) || thys->Defined)
	{
	    ajStrAssignS(reply, def);
	    acdVarResolve(reply);
	    return ajTrue;
	}
    }

    ajStrAssignC(reply, defval);
    ajStrAssignC(&thys->DefStr[DEF_DEFAULT], defval);

    return ajFalse;
}




/* @funcstatic acdReplyInitS **************************************************
**
** Builds a default value for the reply first time around. Uses a default
** specially set in the ACD, or (if none) uses the default string passed in
** parameter "defval" and also sets this as the default in the ACD.
**
** @param [r] thys [const AcdPAcd] ACD object for current item.
** @param [r] defval [const AjPStr] Default value, as a string object
** @param [w] reply [AjPStr*] String containing default reply
** @return [AjBool] ajTrue if a value in the ACD was used.
**
** @release 6.1.0
** @@
******************************************************************************/

static AjBool acdReplyInitS(const AcdPAcd thys, const AjPStr defval,
                            AjPStr* reply)
{
    AjPStr def;

    if(thys->DefStr)
    {
	def = thys->DefStr[DEF_DEFAULT];
	acdLog("acdReplyInitS '%S' : '%S'\n", thys->Name, def);

	if(ajStrGetLen(def) || thys->Defined)
	{
	    ajStrAssignS(reply, def);
	    acdVarResolve(reply);
	    return ajTrue;
	}
    }

    ajStrAssignS(reply, defval);
    ajStrAssignS(&thys->DefStr[DEF_DEFAULT], defval);

    return ajFalse;
}




/* @funcstatic acdDefinedEmpty ************************************************
**
** Tests for a defined ACD value of an empty string
**
** @param [r] thys [const AcdPAcd] ACD object for current item.
** @return [AjBool] ajTrue if a value in the ACD was used but is empty
**
** @release 2.9.0
** @@
******************************************************************************/

static AjBool acdDefinedEmpty (const AcdPAcd thys)
{
    AjPStr def;

    if(thys->DefStr)
    {
	def = thys->DefStr[DEF_DEFAULT];
	if(thys->Defined && !thys->UserSetNull && !ajStrGetLen(def))
	    return ajTrue;
    }

    return ajFalse;
}




/* @funcstatic acdUserGet *****************************************************
**
** Given an ACD containing a defined prompt, a help string,
** or a default (string) value, prompts
** the user for a string value and returns it.
**
** If prompt is set, it is used. Otherwise, info can be used.
** If neither are defined, the item name and type are used to make a
** meaningful prompt.
**
** The default value is offered if it is set.
**
** The user response is returned in "reply"
**
** If -auto is in effect, fails if there is no value.
**
** @param [u] thys [AcdPAcd] ACD object for current item.
** @param [w] reply [AjPStr*] The user response, or
**        the default value if accepted.
** @return [AjBool] ajTrue if reply contains any text.
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdUserGet(AcdPAcd thys, AjPStr* reply)
{
    AjBool ret = ajFalse;
    
    AjPStr prompt;
    AjPStr info;
    AjPStr code;
    AjPStr help;
    
    
    acdLog("acdUserGet '%S' reply '%S'\n", thys->Name, *reply);
    
    if(thys->DefStr && !acdAuto)
    {
	prompt = thys->DefStr[DEF_PROMPT];
	info = thys->DefStr[DEF_INFO];
	code = thys->DefStr[DEF_CODE];
	help = thys->DefStr[DEF_HELP];
	
	ajStrAssignS(&acdUserReplyDef, *reply);
	
	if(ajStrGetLen(code))
	    acdCodeGet(code, &acdUserMsg);
	else if(ajStrGetLen(prompt))
	    ajStrAssignS(&acdUserMsg, prompt);
	else if(ajStrGetLen(info))
	    ajStrAssignS(&acdUserMsg, info);
	else if(ajStrGetLen(thys->StdPrompt))
	    ajStrAssignS(&acdUserMsg, thys->StdPrompt);
	else if(ajStrGetLen(help))
	    ajStrAssignS(&acdUserMsg, help);
	else
	{
	    if(!acdCodeDef(thys, &acdUserMsg))
	    {
		ajStrAssignResC(&acdUserMsg, 512, "");
		ajFmtPrintS(&acdUserMsg, "-%S : enter %s value",
			    thys->Name, acdType[thys->Type].Name);
	    }
	}
	
	acdVarResolve(&acdUserMsg);
	
	acdLog("acdUserGet '%S' replydef '%S' msg '%S'\n",
	       thys->Name, acdUserReplyDef, acdUserMsg);
	
	if(ajStrGetLen(acdUserReplyDef))
	    ret = ajUserGet(reply, "%S [%S]: ", acdUserMsg, acdUserReplyDef);
	else
	    ret = ajUserGet(reply, "%S: ", acdUserMsg);

	if(!ret)
	    ajStrAssignS(reply, acdUserReplyDef);

	if(ret)
	  thys->UserDefined = ajTrue;

	acdUserSavereply(thys, NULL, ret, *reply);
    }
    
    if(ajStrGetLen(*reply))
	ret = ajTrue;
    
    ajStrDelStatic(&acdUserMsg);
    ajStrDelStatic(&acdUserReplyDef);
    
    return ret;
}




/* @funcstatic acdUserGetPrompt ***********************************************
**
** Given a defined prompt, prompts
** the user for a string value and returns it.
**
** The default value is offered if it is set.
**
** The user response is returned in "reply"
**
** If -auto is in effect, fails if there is no value.
**
** @param [r] thys [const AcdPAcd] ACD object for current item
** @param [r] assocqual [const char*] Associated qualifier
** @param [r] prompt [const char*] prompt string
** @param [w] reply [AjPStr*] The user response, or
**        the default value if accepted.
** @return [AjBool] ajTrue if reply contains any text.
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdUserGetPrompt(const AcdPAcd thys, const char* assocqual,
			       const char* prompt, AjPStr* reply)
{
    AjBool ret = ajFalse;

    if(!acdAuto)
    {
	ajStrAssignS(&acdUserReplyDef, *reply);

	if(ajStrGetLen(acdUserReplyDef))
	    ret = ajUserGet(reply, "    %s [%S]: ", prompt, acdUserReplyDef);
	else
	    ret = ajUserGet(reply, "    %s: ", prompt);

	if(!ret)
	    ajStrAssignS(reply, acdUserReplyDef);
	acdUserSavereply(thys, assocqual, ret, *reply);
    }

    if(ajStrGetLen(*reply))
	ret = ajTrue;


    ajStrDelStatic(&acdUserReplyDef);

    return ret;
}




/* @funcstatic acdUserSavereply ***********************************************
**
** Save the reply from prompting the user
**
** @param [r] thys [const AcdPAcd] ACD object for current item.
** @param [r] assocqual [const char*] Associated qualifier
** @param [r] userset [AjBool] Reply set by user
** @param [r] reply [const AjPStr] Reply string
** @return [void]
**
** @release 4.0.0
******************************************************************************/

static void acdUserSavereply(const AcdPAcd thys, const char* assocqual,
			     AjBool userset, const AjPStr reply)
{
    AjPStr qualname = NULL;

    if(assocqual)
    {
	ajStrAssignC(&qualname, assocqual);
	ajStrAppendK(&qualname, '_');
	ajStrAppendS(&qualname, thys->Name);
    }
    else
	ajStrAssignS(&qualname, thys->Name);

    if(ajStrMatchS(acdInputName, qualname))
	ajStrTruncateLen(&acdInputSave, acdInputLen);
    else
    {
	ajStrAssignS(&acdInputName, qualname);
	acdInputLen = ajStrGetLen(acdInputSave);
    }

    if(userset)
    {
	if(acdInputLen)
	    ajStrAppendK(&acdInputSave, '\n');

	ajStrAppendK(&acdInputSave, '-');
	ajStrAppendS(&acdInputSave, qualname);
	ajStrAppendK(&acdInputSave, ' ');
	if(ajStrIsWord(reply) && (ajStrFindAnyC(reply, "*?[]{}|!&^") < 0))
	    ajStrAppendS(&acdInputSave, reply);
	else
	{
	    ajStrAppendK(&acdInputSave, '\"');
	    ajStrAppendS(&acdInputSave, reply);
	    ajStrAppendK(&acdInputSave, '\"');
	}
    }

    ajStrDel(&qualname);

    return;
}




/* @funcstatic acdBadRetry ****************************************************
**
** Writes a message to stderr, and kills the application.
**
** @param [r] thys [const AcdPAcd] ACD object.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdBadRetry(const AcdPAcd thys)
{
    /* test acdc-retry */
    ajDie("%S terminated: Bad value for '-%S' and no more retries",
	  acdProgram, thys->Name);
}




/* @funcstatic acdBadVal ******************************************************
**
** Writes a message to stderr, returns only if this is a standard value
** and we are prompting for values.
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [r] required [AjBool] If true, prompting for the value
**                              was possible.
** @param [r] fmt [const char*] Format with ajFmt extensions
** @param [v] [...] Optional arguments
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdBadVal(const AcdPAcd thys, AjBool required,
		      const char *fmt, ...)
{
    va_list args;
    static AjPStr msg = NULL;

    /*
     ** replaced line below with following 2 to make msg more obvious to
     ** the user
     **/

    acdLog("Failure for option '%S'\n",thys->Name);
    ajFmtPrintS(&msg, "%s", fmt);

    va_start(args, fmt) ;
    ajVErr(ajStrGetPtr(msg), args);
    va_end(args) ;

    if(!required && !acdAuto)		/* test acdc-badadvanced */
	ajDie("%S terminated: Bad value for '-%S' and no prompt",
	      acdProgram, thys->Name);

    if(acdAuto)				/* test acdc-badauto */
	ajDie("%S terminated: Bad value for '-%S' with -auto defined",
	      acdProgram, thys->Name);

    return;
}




/* @section return ************************************************************
**
** Returns values to the calling program. 
**
** @fdata [none]
**
** @nam3rule  Get    Get value
** @nam4rule  GetAlign    ACD alignment datatype
** @nam4rule  GetArray    ACD array datatype
** @nam4rule  GetAssembly   ACD assembly datatype
** @nam4rule  GetBoolean  ACD boolean datatype
** @nam4rule  GetCodon    ACD codon usage datatype
** @nam4rule  GetCpdb    ACD clean PDB data datatype
** @nam4rule  GetDatafile    ACD datafile datatype
** @nam4rule  GetDirectory    ACD directory datatype
** @nam4rule  GetDirlist    ACD directory list datatype
** @nam4rule  GetDiscretestates   ACD discrete states datatype
** @nam4rule  GetDistances    ACD distances datatype
** @nam4rule  GetFeatout    ACD features output datatype
** @nam4rule  GetFeatures   ACD features datatype
** @nam4rule  GetFeaturesall   ACD features stream datatype
** @nam4rule  GetFilelist   ACD file list datatype
** @nam4rule  GetFloat    ACD floating point number datatype
** @nam4rule  GetFrequencies    ACD frequencies datatype
** @nam4rule  GetGraph    ACD graphical output datatype
** @nam4rule  GetGraphxy   ACD XY plot graphical output datatype
** @nam4rule  GetInfile    ACD input file datatype
** @nam4rule  GetInt    ACD integer number datatype
** @nam4rule  GetList    ACD list menu items datatype
** @nam4rule  GetMatrix    ACD integer comparison matrix datatype
** @nam4rule  GetMatrixf    ACD floating point comparison matrix datatype
** @nam4rule  GetObo   ACD obo term datatype
** @nam4rule  GetOboall   ACD obo term stream datatype
** @nam4rule  GetOutassembly   ACD assembly output datatype
** @nam4rule  GetOutcodon    ACD codon usage output datatype
** @nam4rule  GetOutcpdb    ACD clean PDB output datatype
** @nam4rule  GetOutdata    ACD output datafile datatype
** @nam4rule  GetOutdir    ACD output directory datatype
** @nam4rule  GetOutdiscrete    ACD discrete states output datatype
** @nam4rule  GetOutdistance    ACD distance output datatype
** @nam4rule  GetOutfile   ACD output file datatype
** @nam4rule  GetOutfreq   ACD frequency output datatype
** @nam4rule  GetOutmatrix   ACD integer comparison matrix output datatype
** @nam4rule  GetOutmatrixf  ACD floating point comparison matrix output
**                           datatype
** @nam4rule  GetOutobo   ACD OBO ontology term output datatype
** @nam4rule  GetOutproperties   ACD properties output datatype
** @nam4rule  GetOutrefseq   ACD reference sequence output datatype
** @nam4rule  GetOutresource   ACD data resource output datatype
** @nam4rule  GetOutscop   ACD SCOP output datatype
** @nam4rule  GetOuttaxon  ACD taxon output datatype
** @nam4rule  GetOuttext  ACD text output datatype
** @nam4rule  GetOuttree   ACD tree output datatype
** @nam4rule  GetOuturl  ACD URL output datatype
** @nam4rule  GetOutvariation  ACD variation output datatype
** @nam4rule  GetOutxml   ACD xml output datatype
** @nam4rule  GetPattern   ACD prosite pattern datatype
** @nam4rule  GetProperties   ACD properties datatype
** @nam4rule  GetRange   ACD position ranges datatype
** @nam4rule  GetRefseq   ACD reference sequence datatype
** @nam4rule  GetRegexp    ACD regular expression datatype
** @nam4rule  GetReport   ACD report output datatype
** @nam4rule  GetResource   ACD data resource datatype
** @nam4rule  GetResourceall   ACD data resource stream datatype
** @nam4rule  GetScop   ACD SCOP data file datatype
** @nam4rule  GetSelect   ACD selection menu items datatype
** @nam4rule  GetSeq   ACD sequence input datatype
** @nam4rule  GetSeqall   ACD sequence stream input datatype
** @nam4rule  GetSeqout   ACD sequence output datatype
** @nam4rule  GetSeqoutall   ACD sequence stream output datatype
** @nam4rule  GetSeqoutset   ACD sequence set output datatype
** @nam4rule  GetSeqset   ACD sequence set input datatype
** @nam4rule  GetSeqsetall   ACD sequence sets datatype
** @nam4rule  GetString   ACD string datatype
** @nam4rule  GetTaxon   ACD taxon datatype
** @nam4rule  GetTaxonall   ACD taxon stream datatype
** @nam4rule  GetText   ACD text datatype
** @nam4rule  GetTextall   ACD text stream datatype
** @nam4rule  GetToggle   ACD boolean toggle datatype
** @nam4rule  GetTree   ACD phylogenetic tree datatype
** @nam4rule  GetUrl   ACD url datatype
** @nam4rule  GetUrlall   ACD url stream datatype
** @nam4rule  GetVariation   ACD variation datatype
** @nam4rule  GetVariationall   ACD variation stream datatype
** @nam4rule  GetXml   ACD xml datatype
** @nam4rule  GetXmlall   ACD xml stream datatype
** @nam4rule  GetValue   ACD datatype string value
** @nam5rule  GetValueDefault   ACD datatype default value
** @nam5rule  Name    Name of ACD datatype value
** @nam5rule  Single  First in array of ACD datatype values
** @nam3rule  Getpath  Return path for a program defined as external
** @nam3rule  Is    Test value
** @nam4rule  IsUserdefined    Test value is defined by the user
** @suffix C Character string data
** @suffix S String object data
**
** @suffix Double Return double precision
** @suffix Long   Return long integer
**
** @argrule   Get    token [const char*] Token name
** @argrule   C      token [const char*] Token name
** @argrule   S   strtoken [const AjPStr] Token name
** @argrule   Num    token [const char*] Token name
**
** @valrule   Align            [AjPAlign]
** @valrule   Array            [AjPFloat]
** @valrule   Assembly         [AjPAssemload]
** @valrule   Boolean          [AjBool]
** @valrule   Codon            [AjPCod]
** @valrule   Cpdb             [AjPFile]
** @valrule   Datafile         [AjPFile]
** @valrule   Directory        [AjPDir]
** @valrule   Dirlist          [AjPList]
** @valrule   Discretestates   [AjPPhyloState*]
** @valrule   *DiscretestatesSingle  [AjPPhyloState]
** @valrule   Distances        [AjPPhyloDist*]
** @valrule   *DistancesSingle [AjPPhyloDist]
** @valrule   Featout          [AjPFeattabOut]
** @valrule   Features         [AjPFeattable]
** @valrule   Featuresall      [AjPFeattaball]
** @valrule   Filelist         [AjPList]
** @valrule   Float            [float]
** @valrule   *FloatDouble     [double]
** @valrule   Frequencies      [AjPPhyloFreq]
** @valrule   Graph            [AjPGraph]
** @valrule   Graphxy          [AjPGraph]
** @valrule   Infile           [AjPFile]
** @valrule   Int              [ajint]
** @valrule   *IntLong         [ajlong]
** @valrule   List             [AjPStr*]
** @valrule   *ListSingle      [AjPStr]
** @valrule   Matrix           [AjPMatrix]
** @valrule   Matrixf          [AjPMatrixf]
** @valrule   Obo              [AjPObo]
** @valrule   Oboall           [AjPOboall]
** @valrule   Outassembly      [AjPOutfile]
** @valrule   Outcodon         [AjPOutfile]
** @valrule   Outcpdb          [AjPOutfile]
** @valrule   Outdata          [AjPOutfile]
** @valrule   Outdir           [AjPDirout]
** @valrule   Outdiscrete      [AjPOutfile]
** @valrule   Outdistance      [AjPOutfile]
** @valrule   Outfile          [AjPFile]
** @valrule   *OutfileName     [AjPStr]
** @valrule   Outfreq          [AjPOutfile]
** @valrule   Outmatrix        [AjPOutfile]
** @valrule   Outmatrixf       [AjPOutfile]
** @valrule   Outobo           [AjPOutfile]
** @valrule   Outproperties    [AjPOutfile]
** @valrule   Outrefseq        [AjPOutfile]
** @valrule   Outresource      [AjPOutfile]
** @valrule   Outscop          [AjPOutfile]
** @valrule   Outtaxon         [AjPOutfile]
** @valrule   Outtext          [AjPOutfile]
** @valrule   Outtree          [AjPOutfile]
** @valrule   Outurl           [AjPOutfile]
** @valrule   Outvariation     [AjPOutfile]
** @valrule   Outxml           [AjPOutfile]
** @valrule   Pattern          [AjPPatlistSeq]
** @valrule   Properties       [AjPPhyloProp]
** @valrule   Range            [AjPRange]
** @valrule   Refseq           [AjPRefseq]
** @valrule   Regexp           [AjPPatlistRegex]
** @valrule   *RegexpSingle    [AjPRegexp]
** @valrule   Report           [AjPReport]
** @valrule   Resource         [AjPResource]
** @valrule   Resourceall      [AjPResourceall]
** @valrule   Scop             [AjPFile]
** @valrule   Select           [AjPStr*]
** @valrule   *SelectSingle    [AjPStr]
** @valrule   Seq              [AjPSeq]
** @valrule   Seqall           [AjPSeqall]
** @valrule   Seqout           [AjPSeqout]
** @valrule   Seqoutall        [AjPSeqout]
** @valrule   Seqoutset        [AjPSeqout]
** @valrule   Seqset           [AjPSeqset]
** @valrule   Seqsetall        [AjPSeqset*]
** @valrule   *SeqsetallSingle [AjPSeqset]
** @valrule   String           [AjPStr]
** @valrule   Taxon            [AjPTax]
** @valrule   Taxonall         [AjPTaxall]
** @valrule   Text             [AjPText]
** @valrule   Textall          [AjPTextall]
** @valrule   Toggle           [AjBool]
** @valrule   Tree             [AjPPhyloTree*]
** @valrule   *TreeSingle      [AjPPhyloTree]
** @valrule   Url              [AjPUrl]
** @valrule   Urlall           [AjPUrlall]
** @valrule   Variation        [AjPVarload]
** @valrule   Variationall     [AjPVarall]
** @valrule   Xml              [AjPXml]
** @valrule   Xmlall           [AjPXmlall]
** @valrule   Value            [const AjPStr]
** @valrule   *Name            [AjPStr]
** @valrule   Getpath          [const AjPStr]
** @valrule   Is               [AjBool]
** @fcategory misc
**
******************************************************************************/




/* @funcstatic acdSetXxxx *****************************************************
**
** Dummy function to handle prompting and validation for an ACD item type.
** A similar routine is needed for any new item type. See the other acdSet
** functions for ideas on what to include.
**
** This functions knows all the attributes, defaults, associated qualifiers
** and validation rules for type Xxxx. If any attributes (etc) are added,
** this is where the processing will be done.
**
** The final value (of type Xxxx) is set in thys as Value
** and a string equivalent for easy printing is set in thys as Valstr
**
** The function does not need to return a value. It either succeeds in filling
** in all values, or aborts with a suitable error messages.
**
** @param [u] thys [AcdPAcd] ACD for current item (which is
**        always of type Xxxx)
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdSetXxxx(AcdPAcd thys)
{
    AjPStr val;

    AjBool required = ajFalse;
    AjBool ok       = ajFalse;
    ajint itry;

    /*
     ** create storage for val if needed, e.g. with AJNEW0(val)
     */

    val = NULL;				/* set a default value */

    required = acdIsRequired(thys);
    acdReplyInitC(thys, "", &acdReply);

    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	if(required)			/* need to prompt? */
	    acdUserGet(thys, &acdReply);

	ok = ajTrue;			/* test the value somehow */
    }

    if(!ok)
	acdBadRetry(thys);

    ajStrAssignS(&val, acdReply);	/* use the validated reply */
    thys->Value = val;			/* set the value           */
    ajStrAssignS(&thys->ValStr, val);	/* set the string value    */

    return;
}




/* @funcstatic acdSetAppl *****************************************************
**
** Makes sure all application ACD item values have been set.
**
** Called when an "appl" type ACD item is checked. Should not be called
** for any other item. The "appl" item may not be the first, as there
** are some 'standard' qualifiers set in advance for debugging, help and
** so on.
**
** At present there is nothing to prompt for here, though there could
** be, for example, a report of what the program does which would appear
** before any user prompts.
**
** @param [u] thys [AcdPAcd] ACD for the application item.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdSetAppl(AcdPAcd thys)
{
    AjPStr appldoc = NULL;
    AjPStr applobsolete = NULL;
    AjPStr applexternal = NULL;
    AjPStrTok handle = NULL;
    AjPStr token = NULL;
    AjPStr message = NULL;
    AjPStr appname = NULL;      /* saved to table */
    AjPStr appfullname = NULL;  /* saved to table */
    
    acdAttrResolve(thys, "documentation", &appldoc);
    acdAttrResolve(thys, "obsolete", &applobsolete);
    acdAttrResolve(thys, "external", &applexternal);

    acdAppldoc = ajStrNewS(appldoc);

    if(!acdExternalTable)
        acdExternalTable = ajTablestrNewCase(50);

    if(!acdAuto && ajStrGetLen(appldoc))
    {
	ajStrFmtWrap(&appldoc, 75);
	ajUserDumpS(appldoc);
    }

    ajStrAssignS(&thys->ValStr, thys->Name);

    if(ajStrGetLen(applobsolete))
    {
        acdWarnObsolete(applobsolete);
    }

    if(ajStrGetLen(applexternal))
    {
        ajStrTokenAssignC(&handle, applexternal, "|");

        while(ajStrTokenNextParse(handle, &token))
        {
            ajStrExtractFirst(token, &message, &appname);
            ajStrAssignS(&appfullname, appname);
            
            if(!ajSysFileWhich(&appfullname))
            {
                ajStrFmtWrapLeft(&message, 70, 5, 0);
                ajDie("%S uses external program '%S' "
                      "which is not in the PATH or defined as %S_%US\n%S",
                      acdProgram, appname,
                      ajNamValuePackage(), appname, message);
            }

            ajTablePut(acdExternalTable, appname, appfullname);
            appname = NULL;
            appfullname = NULL;
        }
        
    }

    ajStrDel(&appldoc);
    ajStrDel(&applobsolete);
    ajStrDel(&applexternal);
    ajStrDel(&message);
    ajStrDel(&token);

    ajStrTokenDel(&handle);

    return;
}




/* @funcstatic acdSetEndsec ***************************************************
**
** Ends the current ACD section
**
** Called when an "endsection" type ACD item is checked. Should not be called
** for any other item.
**
** At present there is nothing to prompt for here, though there could
** be, for example, a blank line at the end of a section where
** something was prompted for.
**
** @param [u] thys [AcdPAcd] ACD for the endsection item.
** @return [void]
**
** @release 2.0.0
** @@
******************************************************************************/

static void acdSetEndsec(AcdPAcd thys)
{
    if(thys->DefStr)
	ajStrAssignS(&thys->ValStr, thys->DefStr[DEF_DEFAULT]);

    acdVarResolve(&thys->ValStr);

    return;
}




/* @funcstatic acdSetSec ******************************************************
**
** Starts a new ACD section.
**
** Called when a "section" type ACD item is checked. Should not be called
** for any other item.
**
** At present there is nothing to prompt for here, though there could
** be, for example, a prompt issued before the first prompt for the section.
** This would be stored (the 'info' attribute) and used in the standard
** prompting functions.
**
** @param [u] thys [AcdPAcd] ACD for the section item.
** @return [void]
**
** @release 2.0.0
** @@
******************************************************************************/

static void acdSetSec(AcdPAcd thys)
{
    ajint border   = 1;
    AjPStr comment = NULL;
    AjPStr folder  = NULL;
    AjPStr info    = NULL;
    AjPStr side    = NULL;
    AjPStr type    = NULL;
    
    const char* sideVal[] = {"top", "bottom", "left", "right", NULL};
    const char* typeVal[] = {"frame", "page", NULL};
    
    if(acdAttrToStr(thys, "type", "", &type))
	if(!acdVocabCheck(type, typeVal))
	    acdErrorAcd(thys,
			"section %S, bad attribute value type: %S",
			thys->Name, type);
    
    if(acdAttrToInt(thys, "border", 1, &border))
    {
	if(!ajStrMatchCaseC(type, "frame"))
	    ajWarn("section %S, border only used by type: frame",
		   thys->Name);

	if(border < 1)
	{
	    acdAttrToStr(thys, "border", "", &acdTmpStr);
	    acdErrorAcd(thys,
			"section %S, bad attribute value type: %S",
			acdTmpStr);
	}
    }
    
    acdAttrToStr(thys, "comment", "", &comment);
    
    if(acdAttrToStr(thys, "folder", "", &folder))
    {
	if(!ajStrMatchCaseC(type, "page"))
	    ajWarn("section %S, folder only used by type: page",
		   thys->Name);
    }
    
    acdAttrToStr(thys, "information", "", &info);
    
    if(acdAttrToStr(thys, "side", "", &side))
    {
	if(!acdVocabCheck(side, sideVal))
	    acdErrorAcd(thys,
			"section %S, bad attribute value side: %S",
			thys->Name, side);

	if(!ajStrMatchCaseC(type, "frame"))
	    ajWarn("section %S, side only used by type: frame",
		   thys->Name);
    }
    
    ajStrDel(&comment);
    ajStrDel(&folder);
    ajStrDel(&info);
    ajStrDel(&side);
    ajStrDel(&type);

    return;
}




/* @funcstatic acdSetVar ******************************************************
**
** Defines an ACD variable.
**
** Called when a "variable" type ACD item is checked. Should not be called
** for any other item.
**
** At present there is nothing to prompt for here, though there could
** be, for example, a report of what the program does which would appear
** before any user prompts.
**
** @param [u] thys [AcdPAcd] ACD for the application item.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdSetVar(AcdPAcd thys)
{
    if(thys->DefStr)
	ajStrAssignS(&thys->ValStr, thys->DefStr[DEF_DEFAULT]);

    acdVarResolve(&thys->ValStr);

    return;
}




/* @func ajAcdGetAlign ********************************************************
**
** Returns an item of type Align as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPAlign] Alignment output object. Already opened
**                      by ajAlignOpent so this just returns the object
** @cre failure to find an item with the right name and type aborts.
**
** @release 2.1.0
** @@
******************************************************************************/

AjPAlign ajAcdGetAlign(const char *token)
{
    return acdGetValueRef(token, "align");
}




/* @funcstatic acdSetAlign ****************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD alignment output item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other available) is a null string, which
** is invalid.
**
** Associated qualifier "-aformat"
** is applied when writing the sequences.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajSeqRead
**
** @release 2.1.0
** @@
******************************************************************************/

static void acdSetAlign(AcdPAcd thys)
{
    AjPAlign val = NULL;
    
    AjBool required = ajFalse;
    AjBool ok       = ajFalse;

    ajint itry;
    AjBool nullok;
    AjBool nulldefault;
    
    AjPStr name      = NULL;
    AjPStr ext       = NULL;
    AjPStr fmt       = NULL;
    AjPStr dir       = NULL;
    
    val = NULL;

    
    acdAttrToBool(thys, "nullok", ajFalse, &nullok);
    acdAttrToBool(thys, "nulldefault", ajFalse, &nulldefault);

    acdGetValueAssoc(thys, "aformat", &fmt);
    acdGetValueAssoc(thys, "aextension", &ext);
    acdGetValueAssoc(thys, "aname", &name);
    acdGetValueAssoc(thys, "adirectory", &dir);
    
    acdOutDirectory(&dir);
    required = acdIsRequired(thys);

    if(nullok && nulldefault)
    {
	if (acdDefinedEmpty(thys))  /* user set to empty - make default name */
	    acdOutFilename(&acdReplyDef, name, ext);
	else				/* leave empty */
	    acdReplyInitC(thys, "", &acdReplyDef);
    }
    else
    {
	acdOutFilename(&acdTmpOutFName, name, ext);
	acdReplyInitS(thys, acdTmpOutFName, &acdReplyDef);
    }

    acdPromptAlign(thys);
    ajStrDel(&name);
    ajStrDel(&ext);
    
    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */

	ajStrAssignS(&acdReply, acdReplyDef);

	if(required)
	    acdUserGet(thys, &acdReply);

	if(ajStrGetLen(acdReply))
	{
	    val = ajAlignNew();
	    ajStrAssignEmptyS(&val->Formatstr, fmt);
	    acdAttrToStr(thys, "type", "", &val->Type);
	    acdAttrToInt(thys, "minseqs", 0, &val->Nmin);
	    acdAttrToInt(thys, "maxseqs", 0, &val->Nmax);
	    acdAttrToBool(thys, "multiple", ajFalse, &val->Multi);
	    acdQualToInt(thys, "awidth", 50, &val->Width, &acdTmpStr);
	    acdQualToBool(thys, "aglobal", ajFalse,
			  &val->Global, &acdTmpStr);
	    acdQualToBool(thys, "aaccshow", ajFalse,
			  &val->Showacc, &acdTmpStr);
	    acdQualToBool(thys, "adesshow", ajFalse,
			  &val->Showdes, &acdTmpStr);
	    acdQualToBool(thys, "ausashow", ajFalse,
			  &val->Showusa, &acdTmpStr);
	    if(!ajAlignValid(val))
	    {					/* test acdc-alignbadformat */
		ajDie("Alignment option -%S: Validation failed",
		      thys->Name);
	    }
    
	    ajStrAssignS(&acdOutFullFName, acdReply);
	    ajFilenameReplacePathS(&acdOutFullFName, dir);
	    ok = ajAlignOpen(val, acdOutFullFName);

	    if(!ok)
	    {
		acdBadVal(thys, required,
			  "Unable to open alignment file '%S'",
			  acdOutFullFName);
		ajAlignDel(&val);
	    }
	}
	else
	    if(!nullok)
	    {
		acdBadVal(thys, required,
			  "Alignment file is required");
		ok = ajFalse;
	    }
    }

    if(!ok)
	acdBadRetry(thys);
    
    thys->Value = val;
    ajStrAssignS(&thys->ValStr, acdOutFullFName);

    ajStrDel(&fmt);
    ajStrDel(&dir);

    return;
}




/* @func ajAcdGetArray ********************************************************
**
** Returns an item of type array as defined in a named ACD item. Called by the
** application after all ACD values have been set, and simply returns
** what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPFloat] Floating point array object
** @cre failure to find an item with the right name and type aborts.
**
** @release 1.0.0
** @@
******************************************************************************/

AjPFloat ajAcdGetArray(const char *token)
{
    return acdGetValueRef(token, "array");
}




/* @funcstatic acdSetArray ****************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD floating point array item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other available) is "0.0" with a size of 1.
**
** Min and max limits, if set, are applied without comment.
** Precision is provided for logging purposes but otherwise not (yet) used.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajStrToFloat
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdSetArray(AcdPAcd thys)
{
    AjPFloat val;
    
    AjBool required = ajFalse;
    AjBool ok       = ajFalse;

    ajint itry;
    AjBool warnrange;
    AjBool failrange;
    AjBool sumtest;
    AjBool truemin;
    
    float vfmin;
    float vfmax;
    ajint precision;
    ajuint size;
    float sum;
    float tolerance;
    float fdef;
    float ftol;
    float ftot;
    AjPStr deflist = NULL;
    ajuint i;
    float* array;
    AjPStr failmsg = NULL;

    acdRangeTestCalc(thys);

    acdAttrToFloat(thys, "minimum", -FLT_MAX, &vfmin);
    acdLog("minimum: %e\n", vfmin);
    
    acdAttrToFloat(thys, "maximum", FLT_MAX, &vfmax);
    acdLog("maximum: %e\n", vfmax);
    
    acdAttrToFloat(thys, "sum", (float)1.0, &sum);
    acdLog("sum: %e\n", sum);
    
    acdAttrToFloat(thys, "tolerance", (float)0.01, &tolerance);
    acdLog("tolerance: %e\n", tolerance);
    
    acdAttrToInt(thys, "precision", 3, &precision);
    acdLog("precision: %d\n", precision);
    
    acdAttrToBool(thys, "failrange", ajTrue, &failrange);
    acdLog("failrange: %B\n", failrange);

    if(failrange && (vfmin > vfmax))
    {
        acdAttrResolve(thys, "rangemessage", &failmsg);
        if(ajStrGetLen(failmsg))
            acdErrorAcd(thys, "Invalid range: %S", failmsg);
        else
            acdErrorAcd(thys,
                        "Invalid range: "
                        "minimum value %.3f more than maximum %.3f",
                        vfmin, vfmax);
    }

    acdAttrToBool(thys, "warnrange", acdDoWarnRange, &warnrange);
    acdLog("warnrange: %B\n", warnrange);
    acdAttrToBool(thys, "sumtest", ajTrue, &sumtest);
    acdLog("sumtest: %B\n", sumtest);
    acdAttrToBool(thys, "trueminimum", ajFalse, &truemin);
    acdLog("trueminimum: %B\n", truemin);
    
    acdAttrToUint(thys, "size", 1, &size);
    acdLog("size: %d\n", size);
    if(size < 1)
	acdErrorAcd(thys, "Array attribute size: %d less than 1", size);
    
    fdef = sum / ((float) size);
    
    for(i=0; i < size; i++)
    {
	if(i)
	    ajStrAppendK(&deflist, ' ');
	ajFmtPrintAppS(&deflist, "%.*f", precision, fdef);
    }
    
    val = ajFloatNewRes(size);	   /* create storage for the result */
    
    required = acdIsRequired(thys);
    acdReplyInitS(thys, deflist, &acdReplyDef);
    
    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ajStrAssignS(&acdReply, acdReplyDef);
	
	if(required)
	    acdUserGet(thys, &acdReply);
	
	ok = ajFloatParse(acdReply, &val);

	if(ok && ajFloatLen(val) != size)
	{
	    ajErr("Bad array value, expected %d values, found %d",
		  size, ajFloatLen(val));
	    ok = ajFalse;
	}

	if(!ok)
	    acdBadVal(thys, required,
		      "Invalid array value '%S', please try again",
		      acdReply);

	array = ajFloatFloat(val);
	ftot = 0.0;

	for(i=0; i< size; i++)
	{
		if(!truemin && array[i] < vfmin)
	    {				/* reset within limits */
		if(warnrange)
		    ajWarn("floating point value [%d] out of range %.*f "
			   "less than (reset to) %.*f",
			   i+1, precision, array[i], precision, vfmin);
		array[i] = vfmin;
	    }

	    if(array[i] > vfmax)
	    {
		if(warnrange)
		    ajWarn("floating point value [%d] out of range %.*f "
			   "more than (reset to) %.*f",
			   i+1, precision, array[i], precision, vfmax);
		array[i] = vfmax;
	    }

		if(truemin && array[i] < vfmin)
	    {				/* reset within limits */
		if(warnrange)
		    ajWarn("floating point value [%d] out of range %.*f "
			   "less than (reset to) %.*f",
			   i+1, precision, array[i], precision, vfmin);
		array[i] = vfmin;
	    }

	    ftot += array[i];
	}
	
	ftol = (float) fabs(ftot -sum);

	if(sumtest && ftol > tolerance)
	{
	    ajWarn("Bad total %.*f, required total is %.*f with "
		   "tolerance %.*f",
		   precision, ftot, precision, sum,precision, tolerance);
	    acdBadVal(thys, required,
		      "Invalid array value '%S', please try again",
		      acdReply);
	    ok = ajFalse;
	}
    }

    if(!ok)
	acdBadRetry(thys);
    
    thys->Value = val;
    ajFloatStr(val, precision, &thys->ValStr);

    ajStrDel(&deflist);

    return;
}




/* @func ajAcdGetAssembly *****************************************************
**
** Returns an item of type assembly as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
**
** @return [AjPAssemload] Assembly object
**
** @cre failure to find an item with the right name and type aborts.
**
** @release 6.5.0
** @@
******************************************************************************/

AjPAssemload ajAcdGetAssembly(const char *token)
{
    AjPAssemload val = acdGetValueRef(token, "assembly");

    return val;
}




/* @funcstatic acdSetAssembly *************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD assembly input
**
** Understands all attributes and associated qualifiers for this item type.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void acdSetAssembly(AcdPAcd thys)
{
    AjPAssemload val;

    AjBool required = ajFalse;
    AjBool ok       = ajFalse;
    AjBool nullok   = ajFalse;
    ajint itry;

    AjPStr infname = NULL;

    val = ajAssemloadNew();        /* set the default value */

    acdAttrToBool(thys, "nullok", ajFalse, &nullok);

    acdInFilename(&infname);
    required = acdIsRequired(thys);
    acdReplyInitS(thys, infname, &acdReplyDef);
    ajStrDel(&infname);

    acdAttrToBool(thys, "entry", ajFalse, &val->Assemin->Input->Text);

    acdGetValueAssoc(thys, "cbegin", &acdTmpStr);
    ajStrToInt(acdTmpStr, &val->Assemin->cbegin);

    acdGetValueAssoc(thys, "cend", &acdTmpStr);
    ajStrToInt(acdTmpStr, &val->Assemin->cend);

    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */

	ajStrAssignS(&acdReply, acdReplyDef);

	if(required)
	    acdUserGet(thys, &acdReply);

	if(!ajStrGetLen(acdReply) && nullok)
        {
            ajAssemloadDel(&val);
	    break;
	}

	ajAsseminQryS(val->Assemin, acdReply);

	acdGetValueAssoc(thys, "iformat", &val->Assemin->Input->Formatstr);
	acdGetValueAssoc(thys, "iquery", &val->Assemin->Input->QryFields);
	acdQualToLong(thys, "ioffset", 0L, &val->Assemin->Input->Fpos,
                      &acdTmpStr);

        ok = ajAsseminLoad(val->Assemin, val->Assem);
    }

    if(!ok)
	acdBadRetry(thys);

    acdInFileSave(acdReply, ajAssemGetId(val->Assem), ajTrue);

    thys->Value = val;
    ajStrAssignS(&thys->ValStr, acdReply);
    
    return;
}




/* @func ajAcdGetBoolean ******************************************************
**
** Returns an item of type Bool as defined in a named ACD item. Called by the
** application after all ACD values have been set, and simply returns
** what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjBool] Boolean value from ACD item
** @cre failure to find an item with the right name and type aborts.
**
** @release 6.0.0
** @@
******************************************************************************/

AjBool ajAcdGetBoolean(const char *token)
{
    AjBool* val;

    val = acdGetValue(token, "boolean");
    return *val;
}




/* @funcstatic acdSetBoolean **************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD boolean item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other available) is "N" for ajFalse.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajStrToBool
**
** @release 6.0.0
** @@
******************************************************************************/

static void acdSetBoolean(AcdPAcd thys)
{
    AjBool* val;

    AjBool required = ajFalse;
    AjBool ok       = ajFalse;

    ajint itry;

    AJNEW0(val);		   /* create storage for the result */

    *val = ajFalse;			/* set the default value */

    required = acdIsRequired(thys);
    acdReplyInitC(thys, "N", &acdReplyDef);

    acdLog("acdSetBool -%S def: %S\n", thys->Name, acdReplyDef);

    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ajStrAssignS(&acdReply, acdReplyDef);

	if(required)
	    acdUserGet(thys, &acdReply);

	ok = ajStrToBool(acdReply, val);

	if(!ok)
	    acdBadVal(thys, required, "Invalid Y/N value '%S'", acdReply);
    }

    if(!ok)
	acdBadRetry(thys);

    thys->Value = val;
    ajFmtPrintS(&thys->ValStr, "%B", *val);

    acdSetQualAppl(thys, *val);		/* check special application
					   booleans */

    acdLog("acdSetBool -%S val: %B\n", thys->Name, *val);

    if(ajStrMatchC(thys->Name, "help"))
	acdHelp();

    return;
}




/* @func ajAcdGetCodon ********************************************************
**
** Returns an item of type Codon as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPCod] Codon object.
** @cre failure to find an item with the right name and type aborts.
**
** @release 1.0.0
** @@
******************************************************************************/

AjPCod ajAcdGetCodon(const char *token)
{
    return acdGetValueRef(token, "codon");
}




/* @funcstatic acdSetCodon ****************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD outfile item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdSetCodon(AcdPAcd thys)
{
    AjPCod val;

    AjPStr name     = NULL;
    AjBool required = ajFalse;
    AjBool ok       = ajFalse;

    ajint itry;
    AjBool nullok;
    AjPStr fmt    = NULL;

    val = ajCodNew();			/* set the default value */
    acdAttrResolve(thys, "name", &name);

    acdAttrToBool(thys, "nullok", ajFalse, &nullok);
    acdLog("nullok: %B\n", nullok);
    
    if (!acdGetValueAssoc(thys, "format", &fmt))
	ajStrAssignClear(&fmt);

    required = acdIsRequired(thys);
    acdReplyInitS(thys, name, &acdReplyDef);
    acdPromptCodon(thys);

    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */

	ajStrAssignS(&acdReply, acdReplyDef);

	if(required)
	    acdUserGet(thys, &acdReply);

	if(ajStrGetLen(acdReply))
	{
	    if(!ajCodRead(val, acdReply, fmt))
	    {
		acdBadVal(thys, required,
			  "Unable to read codon usage '%S'", acdReply);
		ok = ajFalse;
	    }
	}
	else
	{
	    if(nullok)
	    {
		ajCodDel(&val);
	    }
	    else
	    {
		acdBadVal(thys, required, "Codon file is required");
		ok = ajFalse;
	    }
	}
    }

    if(!ok)
	acdBadRetry(thys);

    acdInFileSave(acdReply, NULL, ajFalse);

    thys->Value = val;
    ajStrAssignS(&thys->ValStr, acdReply);

    ajStrDel(&name);
    ajStrDel(&fmt);

    return;
}




/* @func ajAcdGetCpdb *********************************************************
**
** Returns an item of type Cpdb as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPFile] Cpdb input file.
** @cre failure to find an item with the right name and type aborts.
**
** @release 1.8.0
** @@
******************************************************************************/

AjPFile ajAcdGetCpdb(const char *token)
{
    return acdGetValueRef(token, "cpdb");
}




/* @funcstatic acdSetCpdb *****************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD clean pdb file item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
**
** @release 1.8.0
** @@
******************************************************************************/

static void acdSetCpdb(AcdPAcd thys)
{
    AjPFile val;

    AjPStr name     = NULL;
    AjBool required = ajFalse;
    AjBool ok       = ajFalse;

    ajint itry;
    AjBool nullok;
    AjPStr fmt    = NULL;

    val = NULL;				/* set the default value */

    acdAttrResolve(thys, "name", &name);

    acdAttrToBool(thys, "nullok", ajFalse, &nullok);
    acdLog("nullok: %B\n", nullok);
    
    if (!acdGetValueAssoc(thys, "format", &fmt))
	ajStrAssignClear(&fmt);

    required = acdIsRequired(thys);
    acdReplyInitS(thys, name, &acdReplyDef);
    acdPromptCpdb(thys);

    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */

	ajStrAssignS(&acdReply, acdReplyDef);

	if(required)
	    acdUserGet(thys, &acdReply);

	if(ajStrGetLen(acdReply))
	{
	    val = ajFileNewInNameS(acdReply);

	    if(!val)
	    {
		acdBadVal(thys, required,
			  "Unable to read cleaned PDB data '%S'", acdReply);
		ok = ajFalse;
	    }
	}
	else
	    if(!nullok)
	    {
		acdBadVal(thys, required, "Cleaned PDB data file is required");
		ok = ajFalse;
	    }
    }

    if(!ok)
	acdBadRetry(thys);

    thys->Value = val;
    ajStrAssignS(&thys->ValStr, acdReply);

    ajStrDel(&name);
    ajStrDel(&fmt);

    return;
}




/* @func ajAcdGetDatafile *****************************************************
**
** Returns an item of type Datafile as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPFile] File object. The file was already opened by
**         ajDatafileNewInNameS so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
**
** @release 1.0.0
** @@
******************************************************************************/

AjPFile ajAcdGetDatafile(const char *token)
{
    return acdGetValueRef(token, "datafile");
}




/* @funcstatic acdSetDatafile *************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD datafile item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value is "programname.dat"
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajFileNewOut
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdSetDatafile(AcdPAcd thys)
{
    AjPFile val;
    
    AjBool required = ajFalse;
    AjBool ok       = ajFalse;

    ajint itry;
    AjBool nullok;
    AjPStr name = NULL;
    AjPStr ext  = NULL;
    AjPStr dir  = NULL;
    
    AjPStr datafname = NULL;
    
    val = NULL;				/* set the default value */
    
    acdAttrResolve(thys, "name", &name);
    acdAttrResolve(thys, "extension", &ext);
    acdAttrResolve(thys, "directory", &dir);
    
    acdAttrToBool(thys, "nullok", ajFalse, &nullok);
    acdLog("nullok: %B\n", nullok);
    
    acdDataFilename(&datafname, name, ext, nullok);
    required = acdIsRequired(thys);
    acdReplyInitS(thys, datafname, &acdReplyDef);
/*    acdPromptInfile(thys);*/
    ajStrDel(&datafname);
    ajStrDel(&name);
    ajStrDel(&ext);
    
    for(itry=acdPromptTry; itry && !ok; itry--)
    {	
	ok = ajTrue;	   /* accept the default if nothing changes */
	
	ajStrAssignS(&acdReply, acdReplyDef);
	
	if(required)
	    acdUserGet(thys, &acdReply);
	
	if(ajStrGetLen(acdReply))
	{
	    val = ajDatafileNewInNamePathS(acdReply, dir);

	    if(!val)
	    {
		acdBadVal(thys, required,
			  "Unable to open data file '%S' for input",
			  acdReply);
		ok = ajFalse;
	    }
	}
	else
	    if(!nullok)
	    {
		acdBadVal(thys, required, "Input file is required");
		ok = ajFalse;
	    }
    }

    if(!ok)
	acdBadRetry(thys);
    
    thys->Value = val;
    ajStrAssignS(&thys->ValStr, acdReply);

    ajStrDel(&dir);

    return;
}




/* @func ajAcdGetDirectory ****************************************************
**
** Returns an item of type AjPDir which has been validated as a
** directory.
**
** Optionally can be forced to have a fully qualified path when returned.
**
** @param [r] token [const char*] Text token name
** @return [AjPDir] Directory object
** @cre failure to find an item with the right name and type aborts.
**
** @release 1.0.0
** @@
******************************************************************************/

AjPDir ajAcdGetDirectory(const char *token)
{
    return acdGetValueRef(token, "directory");
}




/* @func ajAcdGetDirectoryName ************************************************
**
** Returns an item of type AjPStr which has been validated as a
** directory name
**
** Optionally can be forced to have a fully qualified path when returned.
**
** @param [r] token [const char*] Text token name
** @return [AjPStr] Directory path
** @cre failure to find an item with the right name and type aborts.
**
** @release 2.9.0
** @@
******************************************************************************/

AjPStr ajAcdGetDirectoryName(const char *token)
{
    AjPStr ret = NULL;
    AjPDir dir;

    dir = acdGetValue(token, "directory");
    ret = ajStrNewS(ajDirGetPath(dir));
    return ret;
}




/* @funcstatic acdSetDirectory ************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD directory item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value is "." the current directory.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdSetDirectory(AcdPAcd thys)
{
    AjPDir val;

    AjBool required = ajFalse;
    AjBool ok       = ajFalse;

    ajint itry;

    AjBool nullok = ajFalse;
    AjBool nulldefault = ajFalse;
    AjBool dopath = ajFalse;
    AjPStr ext = NULL;

    val = NULL;				/* set the default value */

    acdAttrToBool(thys, "fullpath", ajFalse, &dopath);
    acdAttrToBool(thys, "nullok", ajFalse, &nullok);
    acdAttrToBool(thys, "nulldefault", ajFalse, &nulldefault);
    acdGetValueAssoc(thys, "extension", &ext);

    required = acdIsRequired(thys);
    if(nullok && nulldefault)
    {
	if (acdDefinedEmpty(thys))  /* user set to empty - make default name */
	    acdReplyInitC(thys, ".", &acdReplyDef);
	else				/* leave empty */
	    acdReplyInitC(thys, "", &acdReplyDef);
    }
    else
        acdReplyInitC(thys, ".", &acdReplyDef);

    acdPromptDirectory(thys);

    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */

	ajStrAssignS(&acdReply, acdReplyDef);

	if(required)
	    acdUserGet(thys, &acdReply);

	if(ajStrGetLen(acdReply))
	{
	    if(dopath)
		ok = ajDirnameFillPath(&acdReply);
	    else
		ok = ajDirnameFixExists(&acdReply);

	    if (ok)
	    {
		val = ajDirNewPathExt(acdReply, ext);
		if (!val)
		    ok = ajFalse;
	    }
	    if(!ok)
		acdBadVal(thys, required,
			  "Unable to open directory '%S'",
			  acdReply);
	}
	else
	    if(!nullok)
	    {
		acdBadVal(thys, required,
			  "Directory path is required");
		ok = ajFalse;
	    }
    }

    if(!ok)
	acdBadRetry(thys);

    thys->Value = val;
    ajStrAssignS(&thys->ValStr, acdReply);

    ajStrDel(&ext);

    return;
}




/* @func ajAcdGetDirlist ******************************************************
**
** Returns a list of files in a given directory.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPList] List of files.
** @cre failure to find an item with the right name and type aborts.
**
** @release 1.9.0
** @@
******************************************************************************/

AjPList ajAcdGetDirlist(const char *token)
{
    return acdGetValueRef(token, "dirlist");
}




/* @funcstatic acdSetDirlist **************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD directory item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value is "." the current directory.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
**
** @release 1.9.0
** @@
******************************************************************************/

static void acdSetDirlist(AcdPAcd thys)
{
    AjPList val;
    AjPStr  t;
    AjPStr  v;
    AjBool required = ajFalse;
    AjBool ok       = ajFalse;

    ajint itry;

    AjBool nullok = ajFalse;
    AjBool dopath;
    AjPStr ext = NULL;
    ajint n;
    ajint i;
    
    val = NULL;
    
    acdAttrToBool(thys, "fullpath", ajFalse, &dopath);
    acdAttrToBool(thys, "nullok", ajFalse, &nullok);
    acdGetValueAssoc(thys, "extension", &ext);

    required = acdIsRequired(thys);
    acdReplyInitC(thys, ".", &acdReplyDef);
    acdPromptDirlist(thys);
    
    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */

	ajStrAssignS(&acdReply, acdReplyDef);

	if(required)
	    acdUserGet(thys, &acdReply);

        if(ajStrGetLen(acdReply))
	{
	    if(dopath)
		ok = ajDirnameFillPath(&acdReply);
	    else
		ok = ajDirnameFixExists(&acdReply);

	    if(!ok)
		acdBadVal(thys, required,
			  "Unable to open directories '%S' for input",
			  acdReply);
	}
	else
	    if(!nullok)
	    {
		acdBadVal(thys, required,
			  "Input file is required");
		ok = ajFalse;
	    }
    }

    if(!ok)
	acdBadRetry(thys);
    
    val = ajListstrNew();		/* set the default value */
    t   = ajStrNewC("*");

    if(MAJSTRGETLEN(ext))
    {
	if(ajStrGetCharFirst(ext) != '.')
	    ajStrInsertC(&ext, 0, ".");

	ajStrAppendS(&t, ext);
    }    

    ajFilelistAddPathWild(val, acdReply, t);

    /* Sort list so that list of files is system-independent */
    ajListSort(val, &ajStrVcmp);
	


    n = (ajuint) ajListGetLength(val);

    ajDebug("acdSetDirlist '%S' listlength %d\n",
	    acdReply, n);

    for(i=0;i<n;++i)
    {
	ajFmtPrintS(&t,"");
	ajListPop(val,(void **)&v);
	ajStrAppendS(&t,v);
	ajStrAssignS(&v,t);
	ajListPushAppend(val,(void *)v);
    }
    
    thys->Value = val;
    ajStrAssignS(&thys->ValStr, acdReply);
    
    ajStrDel(&t);
    ajStrDel(&ext);
    
    return;
}




/* @func ajAcdGetDiscretestates ***********************************************
**
** Returns an item of type Discrete states as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPPhyloState*] Discrete states object.
**         The string was already set by
**         acdSetDiscretestates so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
**
** @release 2.8.0
** @@
******************************************************************************/

AjPPhyloState* ajAcdGetDiscretestates(const char *token)
{
    return acdGetValueRef(token, "discretestates");
}




/* @func ajAcdGetDiscretestatesSingle *****************************************
**
** Returns an from an array item of type Discrete states as defined in a named
** ACD item, which is an array of objects terminated by a null value.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPPhyloState] Discrete states object. The data was already set by
**         acdSetDiscretestates so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
**
** @release 4.0.0
** @@
******************************************************************************/

AjPPhyloState ajAcdGetDiscretestatesSingle(const char *token)
{
    AjPPhyloState* val;
    ajint i;

    val = acdGetValueSingle(token, "discretestates");
    for(i=0; val[i]; i++)
	continue;

    if(i > 1)
	ajWarn("Single list value %s, but can choose %d values",
	       token, i);

    if(i < 1)
	ajWarn("Single list value %s, no value found: returning NULL value",
	       token);

    return val[0];
}




/* @funcstatic acdSetDiscretestates *******************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD weights file item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other is available) is an empty string.
**
** Attributes for length and maximum property character are applied with error
** messages if exceeded.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
**
** @release 2.8.0
** @@
******************************************************************************/

static void acdSetDiscretestates(AcdPAcd thys)
{
    AjPPhyloState* val = NULL;

    AjBool required = ajFalse;
    AjBool ok       = ajFalse;

    AjPStr infname  = NULL;
    ajint itry;
    
    AjBool nullok = ajFalse;

    ajint size;
    ajint len;
    AjPStr statechars = NULL;
    ajint i;

    val = NULL;

    acdAttrToBool(thys, "nullok", ajFalse, &nullok);
    acdAttrToInt(thys, "size", 1, &size);
    acdAttrToInt(thys, "length", 1, &len);
    acdAttrToStr(thys, "characters", "", &statechars);

    acdInFilename(&infname);

    required = acdIsRequired(thys);
    acdReplyInitS(thys, infname, &acdReplyDef);
    acdPromptInfile(thys);
    ajStrDel(&infname);
    
    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */

	ajStrAssignS(&acdReply, acdReplyDef);

	if(required)
	    acdUserGet(thys, &acdReply);

	if(ajStrGetLen(acdReply))
	{
	    val = ajPhyloStateRead(acdReply, statechars);

	    if(!val)
	    {
		acdBadVal(thys, required,
			  "Unable to read discrete states from '%S'",
			  acdReply);
		ok = ajFalse;
	    }
	}
	else
	    if(!nullok)
	    {
		acdBadVal(thys, required,
			  "Input discrete states file is required");
		ok = ajFalse;
	    }
    }

    if(!ok)
	acdBadRetry(thys);

    acdInFileSave(acdReply, NULL, ajTrue);
    
    /* properties have special set attributes */
    
    thys->SAttr = acdAttrListCount(acdCalcDiscrete);
    thys->SetAttr = &acdCalcDiscrete[0];
    thys->SetStr = AJCALLOC0(thys->SAttr, sizeof(AjPStr));
    
    if(val)
    {
	for(i=0;val[i];i++)
	    continue;
	ajStrFromInt(&thys->SetStr[0],val[0]->Len); /* string length */
	ajStrFromInt(&thys->SetStr[1],val[0]->Size); /* string count */
	ajStrFromInt(&thys->SetStr[2],i); /* number of sets */
 	ajStrAssignS(&thys->ValStr, acdReply);
	ajDebug("acdSetDiscretestates calc len: %d size: %d sets: %d\n",
		val[0]->Len, val[0]->Size, i);
    }
    else
    {
	ajStrFromInt(&thys->SetStr[0],0); /* string length */
	ajStrFromInt(&thys->SetStr[1],0); /* string count */
	ajStrFromInt(&thys->SetStr[2],0); /* number of sets */
	ajStrAssignClear(&thys->ValStr);
    }

    ajStrDel(&statechars);

    thys->Value = val;
   
    return;
}




/* @func ajAcdGetDistances ****************************************************
**
** Returns an item of type Distances as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPPhyloDist*] Distances object. The string was already set by
**         acdSetDistances so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
**
** @release 2.8.0
** @@
******************************************************************************/

AjPPhyloDist* ajAcdGetDistances(const char *token)
{
    return acdGetValueRef(token, "distances");
}




/* @func ajAcdGetDistancesSingle **********************************************
**
** Returns an item of type Distances as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPPhyloDist] Distances object. The string was already set by
**         acdSetDistances so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
**
** @release 5.0.0
** @@
******************************************************************************/

AjPPhyloDist ajAcdGetDistancesSingle(const char *token)
{
    AjPPhyloDist *val;
    ajint i;

    val = acdGetValueSingle(token, "distances");

    for(i=0; val[i]; i++)
	continue;

    if(i > 1)
	ajWarn("Single list value %s, but can choose %d values",
	       token, i);

    if(i < 1)
	ajWarn("Single list value %s, no value found: returning NULL value",
	       token);

    return val[0];
}




/* @funcstatic acdSetDistances ************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD weights file item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other is available) is an empty string.
**
** Attributes for length and maximum property character are applied with error
** messages if exceeded.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
**
** @release 2.8.0
** @@
******************************************************************************/

static void acdSetDistances(AcdPAcd thys)
{
    AjPPhyloDist* val = NULL;

    AjBool required = ajFalse;
    AjBool ok       = ajFalse;

    AjPStr infname  = NULL;
    ajint itry;
    
    AjBool nullok = ajFalse;
    ajint size;
    AjBool missing;
    ajint i;

    val = NULL;

    acdAttrToBool(thys, "nullok", ajFalse, &nullok);
    acdAttrToInt(thys, "size", 0, &size);
    acdAttrToBool(thys, "missval", ajFalse, &missing);

    acdInFilename(&infname);
    required = acdIsRequired(thys);
    acdReplyInitS(thys, infname, &acdReplyDef);
    acdPromptInfile(thys);
    ajStrDel(&infname);
    
    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */

	ajStrAssignS(&acdReply, acdReplyDef);

	if(required)
	    acdUserGet(thys, &acdReply);

	if(ajStrGetLen(acdReply))
	{
	    val = ajPhyloDistRead(acdReply, size, missing);

	    if(!val)
	    {
		acdBadVal(thys, required,
			  "Unable to read distances file '%S'",
			  acdReply);
		ok = ajFalse;
	    }
	}
	else
	    if(!nullok)
	    {
		acdBadVal(thys, required,
			  "Distances file is required");
		ok = ajFalse;
	    }
    }

    if(!ok)
	acdBadRetry(thys);

    acdInFileSave(acdReply, NULL, ajTrue);
    
    /* properties have special set attributes */
    
    thys->SAttr = acdAttrListCount(acdCalcDistances);
    thys->SetAttr = &acdCalcDistances[0];
    thys->SetStr = AJCALLOC0(thys->SAttr, sizeof(AjPStr));

    if(val)
    {
	for(i=0;val[i];i++)
	    continue;
	ajStrFromInt(&thys->SetStr[0],i); /* matrix count */
	ajStrFromInt(&thys->SetStr[1],val[0]->Size); /* string count */
	ajStrFromBool(&thys->SetStr[2],val[0]->HasReplicates);
	ajStrFromBool(&thys->SetStr[3],val[0]->HasMissing);
 	ajStrAssignS(&thys->ValStr, acdReply);
    }
    else
    {
	ajStrFromInt(&thys->SetStr[0],0); /* matrix count */
	ajStrFromInt(&thys->SetStr[1],0); /* string count */
	ajStrFromBool(&thys->SetStr[2],ajFalse);
	ajStrFromBool(&thys->SetStr[3],ajFalse);
	ajStrAssignClear(&thys->ValStr);
    }

    thys->Value = val;
   
    return;
}




/* @func ajAcdGetFeatout ******************************************************
**
** Returns an item of type FeatOut as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPFeattabOut] Feature Table output object. Already opened
**                      by acdSetFeatout so this just returns the object
** @cre failure to find an item with the right name and type aborts.
**
** @release 1.0.0
** @@
******************************************************************************/

AjPFeattabOut ajAcdGetFeatout(const char *token)
{
    return acdGetValueRef(token, "featout");
}




/* @funcstatic acdSetFeatout **************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD feature table item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other available) is a null string, which
** is invalid.
**
** Associated qualifiers "-offormat", "-ofopenfile"
** are applied to the UFO before opening the output file.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajFeatTabOutOpen
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdSetFeatout(AcdPAcd thys)
{
    AjPFeattabOut val = NULL;
    
    AjBool required = ajFalse;
    AjBool ok       = ajFalse;

    ajint itry;
    AjBool nullok;
    AjBool nulldefault;
   
    AjPStr name     = NULL;
    AjPStr ext      = NULL;
    AjPStr fmt      = NULL;
    AjPStr type     = NULL;
   
    val = NULL;

    if(!acdGetValueAssoc(thys, "ofname", &name))
	acdAttrResolve(thys, "name", &name);

    if(acdGetValueAssoc(thys, "offormat", &fmt))
	ajStrAssignS(&ext, fmt);
    else
	acdAttrResolve(thys, "extension", &ext);

    if(!ajStrGetLen(ext))
	ajFeatOutFormatDefault(&ext);

    acdAttrToBool(thys, "nullok", ajFalse, &nullok);
    acdAttrToBool(thys, "nulldefault", ajFalse, &nulldefault);

    ajDebug("acdSetFeatout checking type\n");

    if(!acdAttrToStr(thys, "type", "", &type))
    {
	ajDebug("no type, try '%S'\n", type);

 	if(!acdInTypeFeat(&type))
	    ajWarn("No output type specified for '%S'", thys->Name);
    }


    required = acdIsRequired(thys);

    if(nullok && nulldefault)
    {
	if (acdDefinedEmpty(thys))  /* user set to empty - make default name */
	    acdOutFilename(&acdReplyDef, name, ext);
	else				/* leave empty */
	    acdReplyInitC(thys, "", &acdReplyDef);
    }
    else
    {
	acdOutFilename(&acdTmpOutFName, name, ext);
	acdReplyInitS(thys, acdTmpOutFName, &acdReplyDef);
    }

    ajStrDel(&name);
    ajStrDel(&ext);

    acdPromptFeatout(thys);

    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */
	
	ajStrAssignS(&acdReply, acdReplyDef);
	
	if(required)
	    acdUserGet(thys, &acdReply);
	
	if(ajStrGetLen(acdReply))
	{
	    val = ajFeattabOutNew();
	    ajDebug("Type '%S' try ajFeattabOutSetType\n", type);
	    if(!ajFeattabOutSetType(val, type))
		acdError("Invalid type for feature output");
	    acdGetValueAssoc(thys, "ofopenfile", &val->Filename);
	    acdGetValueAssoc(thys, "ofdirectory", &val->Directory);
	    acdOutDirectory(&val->Directory);
	    ajStrAssignEmptyS(&val->Formatstr, fmt);
	
	    ok = ajFeattabOutOpen(val, acdReply);

	    if(!ok)
	    {
		if(ajStrGetLen(val->Directory))
		    acdBadVal(thys, required,
			      "Unable to open features output '%S%S'",
			      val->Directory, acdReply);
		else
		    acdBadVal(thys, required,
			      "Unable to open features output '%S'",
			      acdReply);

		ajFeattabOutDel(&val);
	    }
	}
	else
	    if(!nullok)
	    {
		acdBadVal(thys, required,
			  "Output UFO is required");
		ok = ajFalse;
	    }
    }

    if(!ok)
	acdBadRetry(thys);
    
    thys->Value = val;
    ajStrAssignS(&thys->ValStr, acdReply);
    
    ajStrDel(&fmt);
    ajStrDel(&type);

    return;
}




/* @func ajAcdGetFeatures *****************************************************
**
** Returns an item of type Features as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPFeattable] Feature Table object. The table was already loaded by
**         acdSetFeat so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
**
** @release 6.0.0
** @@
******************************************************************************/

AjPFeattable ajAcdGetFeatures(const char *token)
{
    AjPFeattaball val = acdGetValue(token, "features");
    if(val->Multi)
        ajWarn("ajAcdGetFeatures request single feature table "
               "but maxreads > 1");

    val->Returned = ajTrue;

    return val->Feattable;
}




/* @func ajAcdGetFeaturesall **************************************************
**
** Returns an input stream of an item of type Features as defined in a
** named ACD item.
**
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPFeattaball] Features input stream
** @cre failure to find an item with the right name and type aborts.
**
** @release 6.4.0
** @@
******************************************************************************/

AjPFeattaball ajAcdGetFeaturesall(const char *token)
{
    AjPFeattaball val = acdGetValueRef(token, "features");

    if(!val->Multi)
        ajWarn("ajAcdGetFeattaball request features input stream "
               "but maxreads is 1");

    return val;
}




/* @funcstatic acdSetFeatures *************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD feature table item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other available) is a null string, which
** is invalid.
**
** Associated qualifiers "-fformat", "-fopenfile"
** are applied to the UFO before reading the feature table.
**
** Associated qualifiers "-fbegin", "-fend" and "-freverse"
** are applied as appropriate, with prompting for values,
** after the feature table has been read.
** They are applied to the feature table,
** and the resulting table is what is set in the ACD item.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
**
** @release 6.0.0
** @@
******************************************************************************/

static void acdSetFeatures(AcdPAcd thys)
{
    AjPFeattaball val   = NULL;
    
    AjBool required = ajFalse;
    AjBool ok       = ajFalse;

    ajint itry;
    
    AjPStr infname = NULL;
    static AjPStr type    = NULL;
    
    ajint fbegin = 0;
    ajint fend   = 0;
    AjBool freverse = ajFalse;
    AjBool fprompt  = ajFalse;
    ajint iattr;
    
    ajint maxreads;

    val = ajFeattaballNew();		/* set the default value */
    
    acdQualToBool(thys, "fask", ajFalse, &fprompt, &acdReplyDef);
    acdAttrToInt(thys, "maxreads", INT_MAX, &maxreads);
    
    acdAttrToBool(thys, "entry", ajFalse, &val->Feattabin->Input->Text);

    if(acdAttrToStr(thys, "type", "", &type))
    {
	if(!ajFeattabinSetTypeS(val->Feattabin, type))
	    acdError("Invalid type for feature input");
	acdInTypeFeatSave(type);
    }
    else
    {
	acdInTypeFeatSave(NULL);
    }

    acdInFilename(&infname);
    required = acdIsRequired(thys);
    acdReplyInitS(thys, infname, &acdReplyDef);
    acdPromptFeatures(thys);
    ajStrDel(&infname);
    
    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */

	ajStrAssignS(&acdReply, acdReplyDef);

	if(required)
	    acdUserGet(thys, &acdReply);

	acdGetValueAssoc(thys, "fformat", &val->Feattabin->Formatstr);
	acdGetValueAssoc(thys, "fopenfile", &val->Feattabin->Filename);
	acdGetValueAssoc(thys, "iquery", &val->Feattabin->Input->QryFields);
	acdQualToLong(thys, "ioffset", 0L, &val->Feattabin->Input->Fpos,
                      &acdTmpStr);

	ajFeattabinQryS(val->Feattabin, acdReply);
	ok = ajFeattabinRead(val->Feattabin, val->Feattable);
    }

    if(!ok)
	acdBadRetry(thys);
    
    acdInFileSave(acdReply, ajFeattableGetName(val->Feattable),
                  ajTrue); /* save sequence name */
    
    /* now process the begin, end and reverse options */
    
    ok = acdQualToInt(thys, "fbegin", 1, &fbegin, &acdReplyDef);
    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ajStrAssignS(&acdReply, acdReplyDef);

	if(fprompt)
	    acdUserGetPrompt(thys, "fbegin", " Begin at position", &acdReply);

	ok = ajStrToInt(acdReply, &fbegin);

	if(!ok)
	    acdBadVal(thys, ajTrue,
		      "Invalid integer value '%S'", acdReply);
    }

    if(!ok)
	acdBadRetry(thys);
    
    ok = acdQualToInt(thys, "fend", ajFeattableGetLen(val->Feattable), &fend,
                      &acdReplyDef);

    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ajStrAssignS(&acdReply, acdReplyDef);

	if(fprompt)
	    acdUserGetPrompt(thys, "fend", "   End at position", &acdReply);

	ok = ajStrToInt(acdReply, &fend);

	if(!ok)
	    acdBadVal(thys, ajTrue,
		      "Invalid integer value '%S'", acdReply);
    }

    if(!ok)
	acdBadRetry(thys);
    
    ok = acdQualToBool(thys, "freverse", ajFalse, &freverse, &acdReplyDef);

    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ajStrAssignS(&acdReply, acdReplyDef);

	if(fprompt)
	    acdUserGetPrompt(thys, "freverse", "    Reverse strand", &acdReply);
	ok = ajStrToBool(acdReply, &freverse);

        
	if(!ok)
	    acdBadVal(thys, ajTrue,
		      "Invalid Y/N value '%S'", acdReply);
    }

    if(!ok)
	acdBadRetry(thys);
    
    acdLog("sbegin: %d, send: %d, freverse: %B\n",
	   fbegin, fend, freverse);
    
    if(freverse)
    {
        val->Feattabin->Rev = freverse;
	ajFeattableReverse(val->Feattable);
    }

    ajFeattabinSetRange(val->Feattabin, fbegin, fend);
    ajFeattableSetRange(val->Feattable, fbegin, fend);
    
    if(maxreads > 1)
        val->Multi = ajTrue;

    /* features tables have special set attributes */
    
    thys->SAttr = acdAttrListCount(acdCalcFeat);
    thys->SetAttr = &acdCalcFeat[0];
    thys->SetStr = AJCALLOC0(thys->SAttr, sizeof(AjPStr));
    
    iattr = 0;
    ajStrFromInt(&thys->SetStr[iattr++], ajFeattableGetBegin(val->Feattable));
    ajStrFromInt(&thys->SetStr[iattr++], ajFeattableGetEnd(val->Feattable));
    ajStrFromInt(&thys->SetStr[iattr++], ajFeattableGetLen(val->Feattable));
    ajStrFromBool(&thys->SetStr[iattr++], ajFeattableIsProt(val->Feattable));
    ajStrFromBool(&thys->SetStr[iattr++], ajFeattableIsNuc(val->Feattable));
    ajStrAssignS(&thys->SetStr[iattr++], ajFeattableGetName(val->Feattable));
    ajStrFromInt(&thys->SetStr[iattr++], ajFeattableGetSize(val->Feattable));
    
    thys->Value = val;
    ajStrAssignS(&thys->ValStr, acdReply);

    ajStrDel(&type);

    return;
}




/* @func ajAcdGetFilelist *****************************************************
**
** Returns a list of files given a comma-separated list.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPList] List of files.
** @cre failure to find an item with the right name and type aborts.
**
** @release 2.5.0
** @@
******************************************************************************/

AjPList ajAcdGetFilelist(const char *token)
{
    return acdGetValueRef(token, "filelist");
}




/* @funcstatic acdSetFilelist *************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD file list item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** There is no default value
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
**
** @release 2.5.0
** @@
******************************************************************************/

static void acdSetFilelist(AcdPAcd thys)
{
    AjPList val;
    AjBool required = ajFalse;
    AjBool ok       = ajFalse;

    ajint itry;

    AjBool nullok = ajFalse;

    val = ajListNew();

    acdAttrToBool(thys, "nullok", ajFalse, &nullok);

    required = acdIsRequired(thys);
    acdReplyInitC(thys, "", &acdReplyDef);
    acdPromptFilelist(thys);

    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */

	ajStrAssignS(&acdReply, acdReplyDef);

	if(required)
	    acdUserGet(thys, &acdReply);

	if(!ajStrGetLen(acdReply))
	    if(!nullok)
	    {
		acdBadVal(thys, required,
			  "File list is required");
		ok = ajFalse;
	    }
    }

    if(!ok)
	acdBadRetry(thys);

    ajFilelistAddListname(val, acdReply);

    thys->Value = val;
    ajStrAssignS(&thys->ValStr, acdReply);

    return;
}




/* @func ajAcdGetFloat ********************************************************
**
** Returns an item of type Float as defined in a named ACD item. Called by the
** application after all ACD values have been set, and simply returns
** what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [float] Floating point value from ACD item
** @cre failure to find an item with the right name and type aborts.
**
** @release 1.0.0
** @@
******************************************************************************/

float ajAcdGetFloat(const char *token)
{
    double *val;

    val = acdGetValue(token, "float");

    return (float) *val;
}




/* @func ajAcdGetFloatDouble***************************************************
**
** Returns an item of type Float as defined in a named ACD item. Called by the
** application after all ACD values have been set, and simply returns
** what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [double] Floating point value from ACD item
** @cre failure to find an item with the right name and type aborts.
**
** @release 6.1.0
** @@
******************************************************************************/

double ajAcdGetFloatDouble(const char *token)
{
    double *val;

    val = acdGetValue(token, "float");

    return *val;
}




/* @funcstatic acdSetFloat ****************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD floating point item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other available) is "0.0".
**
** Min and max limits, if set, are applied without comment.
** Precision is provided for logging purposes but otherwise not (yet) used.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajStrToFloat
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdSetFloat(AcdPAcd thys)
{
    double* val;
    float  fval;
    
    AjBool required = ajFalse;
    AjBool ok       = ajFalse;

    ajint itry;
    AjBool warnrange;
    AjBool failrange;
    AjBool isdouble;
    AjBool truemin;
    
    double vfmin;
    double vfmax;
    ajint precision;
    AjPStr failmsg = NULL;
    
    acdRangeTestCalc(thys);

    acdAttrToDouble(thys, "minimum", -FLT_MAX, &vfmin);
    acdLog("minimum: %e\n", vfmin);
    
    acdAttrToDouble(thys, "maximum", FLT_MAX, &vfmax);
    acdLog("maximum: %e\n", vfmax);
    
    acdAttrToInt(thys, "precision", 3, &precision);
    acdLog("precision: %d\n", precision);
    
    acdAttrToBool(thys, "failrange", ajTrue, &failrange);
    acdLog("failrange: %B\n", failrange);
    
    acdAttrToBool(thys, "warnrange", acdDoWarnRange, &warnrange);
    acdLog("warnrange: %B\n", warnrange);
    
    acdAttrToBool(thys, "large", AJFALSE, &isdouble);
    acdLog("large: %B\n", isdouble);

    acdAttrToBool(thys, "trueminimum", AJFALSE, &truemin);
    acdLog("trueminimum: %B\n", truemin);

    if(failrange && (vfmin > vfmax))
    {
        acdAttrResolve(thys, "rangemessage", &failmsg);
        if(ajStrGetLen(failmsg))
            acdErrorAcd(thys, "Invalid range: %S", failmsg);
        else
            acdErrorAcd(thys,
                        "Invalid range: "
                        "minimum value %.3f more than maximum %.3f",
                        vfmin, vfmax);
    }

    AJNEW0(val);		   /* create storage for the result */
    
    *val = 0.0;				/* set the default value */
    
    required = acdIsRequired(thys);
    acdReplyInitC(thys, "0.0", &acdReplyDef);
    
    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ajStrAssignS(&acdReply, acdReplyDef);

	if(required)
	    acdUserGet(thys, &acdReply);

        if(isdouble)
            ok = ajStrToDouble(acdReply, val);
        else
        {
            ok = ajStrToFloat(acdReply, &fval);
            *val = fval;
        }

	if(!ok)
	    acdBadVal(thys, required,
		      "Invalid decimal value '%S', please try again",
		      acdReply);
    }

    if(!ok)
	acdBadRetry(thys);
    
    if(isdouble && vfmin == FLT_MIN)
        vfmin = DBL_MIN;

    if(isdouble && vfmax == FLT_MAX)
        vfmax = DBL_MAX;
    
    if(!truemin && *val < vfmin)
    {					/* reset within limits */
	if(warnrange)
	    ajWarn("floating point value out of range %.*f "
		   "less than (reset to) %.*f",
		   precision, *val, precision, vfmin);
	*val = vfmin;
    }

    if(*val > vfmax)
    {
	if(warnrange)
	    ajWarn("floating point value out of range %.*f "
		   "more than (reset to) %.*f",
		   precision, *val, precision, vfmax);
	*val = vfmax;
    }
    
    if(truemin && *val < vfmin)
    {					/* reset within limits */
	if(warnrange)
	    ajWarn("floating point value out of range %.*f "
		   "less than (reset to) %.*f",
		   precision, *val, precision, vfmin);
	*val = vfmin;
    }

   thys->Value = val;
    ajStrFromFloat(&thys->ValStr, (float) *val, precision);
    
    return;
}




/* @func ajAcdGetFrequencies **************************************************
**
** Returns an item of type Frequencies as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPPhyloFreq] Frequencies object. The string was already set by
**         acdSetFrequencies so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
**
** @release 2.8.0
** @@
******************************************************************************/

AjPPhyloFreq ajAcdGetFrequencies(const char *token)
{
    return acdGetValueRef(token, "frequencies");
}




/* @funcstatic acdSetFrequencies **********************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD weights file item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other is available) is an empty string.
**
** Attributes for length and maximum property character are applied with error
** messages if exceeded.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
**
** @release 2.8.0
** @@
******************************************************************************/

static void acdSetFrequencies(AcdPAcd thys)
{
    AjPPhyloFreq val;

    AjBool required = ajFalse;
    AjBool ok       = ajFalse;

    AjPStr infname  = NULL;
    ajint itry;
    
    AjBool nullok   = ajFalse;
    AjBool contchar = ajFalse;
    AjBool genedata = ajFalse;
    AjBool within   = ajFalse;
    ajint size;

    val = NULL;

    acdAttrToBool(thys, "nullok", ajFalse, &nullok);
    acdAttrToInt(thys, "size", 1, &size);
    acdAttrToBool(thys, "continuous", ajFalse, &contchar);
    acdAttrToBool(thys, "genedata", ajFalse, &genedata);
    acdAttrToBool(thys, "within", ajFalse, &within);

    acdInFilename(&infname);

    required = acdIsRequired(thys);
    acdReplyInitS(thys, infname, &acdReplyDef);
    acdPromptInfile(thys);
    ajStrDel(&infname);
    
    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */

	ajStrAssignS(&acdReply, acdReplyDef);

	if(required)
	    acdUserGet(thys, &acdReply);

	if(ajStrGetLen(acdReply))
	{
	    val = ajPhyloFreqRead(acdReply, contchar, genedata, within);

	    if(!val)
	    {
		acdBadVal(thys, required,
			  "Unable to read frequencies file '%S'",
			  acdReply);
		ok = ajFalse;
	    }
	}
	else
	    if(!nullok)
	    {
		acdBadVal(thys, required,
			  "Input frequencies file is required");
		ok = ajFalse;
	    }
    }

    if(!ok)
	acdBadRetry(thys);

    acdInFileSave(acdReply, NULL, ajTrue);
    
    /* properties have special set attributes */
    
    thys->SAttr = acdAttrListCount(acdCalcFrequencies);
    thys->SetAttr = &acdCalcFrequencies[0];
    thys->SetStr = AJCALLOC0(thys->SAttr, sizeof(AjPStr));
    
    if(val)
    {
	ajStrFromInt(&thys->SetStr[0],val->Len); /* string count */
	ajStrFromInt(&thys->SetStr[1],val->Size); /* string count */
	ajStrFromInt(&thys->SetStr[2],val->Loci); /* string count */
	ajStrFromBool(&thys->SetStr[3],!val->ContChar); /* genes */
	ajStrFromBool(&thys->SetStr[4],val->ContChar); /* contin */
	ajStrFromBool(&thys->SetStr[5],val->Within); /* indivs */
 	ajStrAssignS(&thys->ValStr, acdReply);
    }
    else
    {
	ajStrFromInt(&thys->SetStr[0],0); /* string count */
	ajStrFromInt(&thys->SetStr[1],0); /* string count */
	ajStrFromInt(&thys->SetStr[2],0); /* string count */
	ajStrFromBool(&thys->SetStr[3],0); /* genes */
	ajStrFromBool(&thys->SetStr[4],0); /* contin */
	ajStrFromBool(&thys->SetStr[5],0); /* indivs */
	ajStrAssignClear(&thys->ValStr);
    }

    thys->Value = val;
   
    return;
}




/* @func ajAcdGetGraph ********************************************************
**
** Returns a graph object which hold user graphics options.
**
** @param [r] token [const char*] Text token name
** @return [AjPGraph] Graph object.
** @cre failure to find an item with the right name and type aborts.
**
** @release 1.0.0
** @@
******************************************************************************/

AjPGraph ajAcdGetGraph(const char *token)
{
    return acdGetValueRef(token, "graph");
}




/* @funcstatic acdSetGraph ****************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD graph item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdSetGraph(AcdPAcd thys)
{
    AjPGraph val = NULL;
    
    AjBool required = ajFalse;
    AjBool ok       = ajFalse;

    AjPStr title    = NULL;
    AjPStr gdev     = NULL;

    ajint itry;
    AjBool nullok;
    AjBool nulldefault;
    AjBool hasseq   = ajFalse;
    
    acdAttrToBool(thys, "nullok", ajFalse, &nullok);
    acdAttrToBool(thys, "nulldefault", ajFalse, &nulldefault);
    required = acdIsRequired(thys);
    if(nullok && nulldefault)
    {
	if (acdDefinedEmpty(thys))  /* user set to empty - make default name */
	{
	    if(ajNamGetValueC("GRAPHICS",&gdev))
		acdReplyInitS(thys, gdev, &acdReplyDef);
	    else
#ifndef WIN32
#ifndef X_DISPLAY_MISSING /* X11 is available */
		acdReplyInitC(thys, "x11", &acdReplyDef);
#else
#ifdef PLD_png          /* if png/gd/zlib libraries available for png driver */
		acdReplyInitC(thys, "png", &acdReplyDef);
#else
		acdReplyInitC(thys, "ps", &acdReplyDef);
#endif
#endif
#else
	    acdReplyInitC(thys, "win3", &acdReplyDef);
#endif
	}
	else				/* leave empty */
	    acdReplyInitC(thys, "", &acdReplyDef);
    }
    else
    {
	if(ajNamGetValueC("GRAPHICS",&gdev))
	    acdReplyInitS(thys, gdev, &acdReplyDef);
	else
#ifndef WIN32
#ifndef X_DISPLAY_MISSING /* X11 is available */
	    acdReplyInitC(thys, "x11", &acdReplyDef);
#else
#ifdef PLD_png          /* if png/gd/zlib libraries available for png driver */
	    acdReplyInitC(thys, "png", &acdReplyDef);
#else
	    acdReplyInitC(thys, "ps", &acdReplyDef);
#endif
#endif
#else
	    acdReplyInitC(thys, "win3", &acdReplyDef);
#endif
    }

    ajStrDel(&gdev);
    acdPromptGraph(thys);
    
    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;

	ajStrAssignS(&acdReply, acdReplyDef);

	if(required)
	    acdUserGet(thys, &acdReply);

	if(ajStrGetLen(acdReply))	/* valid no graph type */
	{

	    if(!val)
	      val = ajGraphNew();
	    ok = ajGraphSetDevicetype(val, acdReply);

	    if(!ok)
	    {
	        ajGraphicsDumpDevices();
		acdBadVal(thys, required,
			  "Invalid graph value '%S'", acdReply);
	    }
	}
	else
	    if(!nullok)
	    {
		acdBadVal(thys, required,
			  "Graph is required");
		ok = ajFalse;
	    }
    }
    
    if(!ok)
	acdBadRetry(thys);
    
    thys->Value = val;
    ajStrAssignC(&thys->ValStr, "graph definition");

    if(val)
    {
        acdAttrToBool(thys, "sequence", ajFalse, &hasseq);

	if(acdGetValueAssoc(thys, "gdesc", &title))
	    ajGraphSetDescS(val,title);
    
	if(acdGetValueAssoc(thys, "gtitle", &title))
	    ajGraphSetTitleS(val,title);
    
	if(acdGetValueAssoc(thys, "gsubtitle", &title))
	    ajGraphSetSubtitleS(val,title);
    
	if(acdGetValueAssoc(thys, "gxtitle", &title))
	    ajGraphSetXlabelS(val,title);
        else if(hasseq)
            ajGraphSetXlabelS(val, acdInFName);

	if(acdGetValueAssoc(thys, "gytitle", &title))
	    ajGraphSetYlabelS(val,title);
    
	if(acdGetValueAssoc(thys, "goutfile", &title))
	    ajGraphSetOutfileS(val,title);
    
	if(acdGetValueAssoc(thys, "gdirectory", &title))
	    ajGraphSetOutdirS(val,title);
	else
	{
	    ajStrAssignClear(&title);
	    if(acdOutDirectory(&title))
	        ajGraphSetOutdirS(val,title);
	}

	ajStrDel(&title);
	ajGraphTrace(val);
    }
    
    return;
}




/* @func ajAcdGetGraphxy ******************************************************
**
** Returns a graph object which hold user graphics options.
**
** @param [r] token [const char*] Text token name
** @return [AjPGraph] Graph object.
** @cre failure to find an item with the right name and type aborts.
**
** @release 1.0.0
** @@
******************************************************************************/

AjPGraph ajAcdGetGraphxy(const char *token)
{
    return acdGetValueRef(token, "xygraph");
}




/* @funcstatic acdSetGraphxy **************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD XY graph item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdSetGraphxy(AcdPAcd thys)
{
    AjPGraph val;
    AjPStr gdev = NULL;
    
    AjBool required = ajFalse;
    AjBool ok       = ajFalse;
    
    AjPStr title    = NULL;
    ajint itry;
    AjBool nullok;
    AjBool nulldefault;
    ajint multi;
    AjBool hasseq   = ajFalse;

    val = NULL;

    acdAttrToBool(thys, "nullok", ajFalse, &nullok);
    acdAttrToBool(thys, "nulldefault", ajFalse, &nulldefault);
    acdAttrToInt(thys, "multi", 1, &multi);
    if(multi < 1) multi = 1;
    acdLog("multi: %d\n", multi);
    
    required = acdIsRequired(thys);
    if(nullok && nulldefault)
    {
	if (acdDefinedEmpty(thys))  /* user set to empty - make default name */
	{
	    if(ajNamGetValueC("GRAPHICS",&gdev))
		acdReplyInitS(thys, gdev, &acdReplyDef);
	    else
#ifndef WIN32
#ifndef X_DISPLAY_MISSING /* X11 is available */
		acdReplyInitC(thys, "x11", &acdReplyDef);
#else
#ifdef PLD_png          /* if png/gd/zlib libraries available for png driver */
		acdReplyInitC(thys, "png", &acdReplyDef);
#else
		acdReplyInitC(thys, "ps", &acdReplyDef);
#endif
#endif
#else
	    acdReplyInitC(thys, "win3", &acdReplyDef);
#endif
	}
	else				/* leave empty */
	    acdReplyInitC(thys, "", &acdReplyDef);
    }
    else
    {
	if(ajNamGetValueC("GRAPHICS",&gdev))
	    acdReplyInitS(thys, gdev, &acdReplyDef);
	else
#ifndef WIN32
#ifndef X_DISPLAY_MISSING /* X11 is available */
	    acdReplyInitC(thys, "x11", &acdReplyDef);
#else
#ifdef PLD_png          /* if png/gd/zlib libraries available for png driver */
	    acdReplyInitC(thys, "png", &acdReplyDef);
#else
	    acdReplyInitC(thys, "ps", &acdReplyDef);
#endif
#endif
#else
	    acdReplyInitC(thys, "win3", &acdReplyDef);
#endif
    }
    ajStrDel(&gdev);
    acdPromptGraph(thys);
    
    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */

	ajStrAssignS(&acdReply, acdReplyDef);

	if(required)
	    acdUserGet(thys, &acdReply);

	if(ajStrGetLen(acdReply))	/* valid no graph type */
	{
	    if(!val)
	        val = ajGraphxyNewI(multi);
    
	    ok = ajGraphxySetDevicetype(val, acdReply);

	    if(!ok)
	    {
	        ajGraphicsDumpDevices();
		acdBadVal(thys, required,
			  "Invalid XY graph value '%S'", acdReply);
	    }
	}
	else
	    if(!nullok)
	    {
		acdBadVal(thys, required,
			  "Graph is required");
		ok = ajFalse;
	    }
    }
    
    if(!ok)
	acdBadRetry(thys);
    
    thys->Value = val;
    ajStrAssignC(&thys->ValStr, "XY graph definition");
    
    if(val)
    {
        acdAttrToBool(thys, "sequence", ajFalse, &hasseq);
        if(hasseq)
            ajGraphSetDatanameS(val, acdInFName);

	if(acdGetValueAssoc(thys, "gdesc", &title))
	    ajGraphSetDescS(val,title);
    
	if(acdGetValueAssoc(thys, "gtitle", &title))
	    ajGraphSetTitleS(val,title);

	if(acdGetValueAssoc(thys, "gsubtitle", &title))
	    ajGraphSetSubtitleS(val,title);

	if(acdGetValueAssoc(thys, "gxtitle", &title))
	    ajGraphSetXlabelS(val,title);
        else if(hasseq)
            ajGraphSetXlabelS(val, acdInFName);

	if(acdGetValueAssoc(thys, "gytitle", &title))
	    ajGraphSetYlabelS(val,title);

	if(acdGetValueAssoc(thys, "goutfile", &title))
	    ajGraphSetOutfileS(val,title);

	if(acdGetValueAssoc(thys, "gdirectory", &title))
	    ajGraphSetOutdirS(val,title);
	else
	{
	    ajStrAssignClear(&title);
	    if(acdOutDirectory(&title))
                ajGraphSetOutdirS(val,title);
	}

	ajStrDel(&title);
	ajGraphTrace(val);
    }

    return;
}




/* @func ajAcdGetInfile *******************************************************
**
** Returns an item of type file as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPFile] File object. The file was already opened by
**         acdSetInfile so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
**
** @release 1.0.0
** @@
******************************************************************************/

AjPFile ajAcdGetInfile(const char *token)
{
    return acdGetValueRef(token, "infile");
}




/* @funcstatic acdSetInfile ***************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD infile item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if filtering is on) is "stdin", but then
** prompting is turned off.
**
** Otherwise there is no default value unless the ACD file has one.
**
** Various file naming options are defined, but not yet implemented here.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajFileNewIn
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdSetInfile(AcdPAcd thys)
{
    AjPFile val;
    
    AjBool required = ajFalse;
    AjBool ok       = ajFalse;

    ajint itry;
    AjBool nullok;
    AjBool trydefault;
    
    AjPStr infname = NULL;
    AjPStr inpath = NULL;
    
    val = NULL;				/* set the default value */
    
    acdAttrToStr(thys, "directory", "", &inpath);
    acdAttrToBool(thys, "nullok", ajFalse, &nullok);
    acdAttrToBool(thys, "trydefault", ajFalse, &trydefault);

    acdInFilename(&infname);
    required = acdIsRequired(thys);
    acdReplyInitS(thys, infname, &acdReplyDef);
    acdPromptInfile(thys);
    ajStrDel(&infname);
    
    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */

	ajStrAssignS(&acdReply, acdReplyDef);

	if(required)
	    acdUserGet(thys, &acdReply);

	if(ajStrGetLen(acdReply))
	{
	    if(!ajFilenameHasPath(acdReply))
                val = ajFileNewInNamePathS(acdReply,inpath);
            else
                val = ajFileNewInNameS(acdReply);

	    if(!val)
	    {
		if(!nullok ||
		   !trydefault ||
		   !ajStrMatchS(acdReply, acdReplyDef))
		{
                    if(ajStrGetLen(inpath) && !ajFilenameHasPath(acdReply))
                        acdBadVal(thys, required,
                                  "Unable to open file '%S/%S' for input",
                                  inpath, acdReply);
                    else
                        acdBadVal(thys, required,
                                  "Unable to open file '%S' for input",
                                  acdReply);
		    ok = ajFalse;
		}
	    }
	}
	else
	    if(!nullok)
	    {
		acdBadVal(thys, required,
			  "Input file is required");
		ok = ajFalse;
	    }
    }

    if(!ok)
	acdBadRetry(thys);
    
    acdInFileSave(acdReply, NULL, ajTrue);
    
    thys->Value = val;
    ajStrAssignS(&thys->ValStr, acdReply);

    ajStrDel(&inpath);

    return;
}




/* @func ajAcdGetInt **********************************************************
**
** Returns an item of type ajint as defined in a named ACD item. Called by the
** application after all ACD values have been set, and simply returns
** what the ACD item already has.
**
** The ACD item is expected to have the large attribute set to false.
**
** @param [r] token [const char*] Text token name
** @return [ajint] Integer value from ACD item
** @cre failure to find an item with the right name and type aborts.
**
** @release 1.0.0
** @@
******************************************************************************/

ajint ajAcdGetInt(const char *token)
{
    ajlong *val;

    val = acdGetValue(token, "integer");

    return (ajint) *val;
}




/* @func ajAcdGetIntLong ******************************************************
**
** Returns an item of type ajlong as defined in a named ACD item. Called by the
** application after all ACD values have been set, and simply returns
** what the ACD item already has.
**
** The ACD item is expected to have the large attribute set to true.
**
** @param [r] token [const char*] Text token name
** @return [ajlong] Integer value from ACD item
** @cre failure to find an item with the right name and type aborts.
**
** @release 6.1.0
** @@
******************************************************************************/

ajlong ajAcdGetIntLong(const char *token)
{
    ajlong *val;

    val = acdGetValue(token, "integer");

    return *val;
}




/* @funcstatic acdSetInt ******************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD integer item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other available) is "0".
**
** Min and max limits, if set, are applied without comment.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajStrToInt
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdSetInt(AcdPAcd thys)
{
    ajlong* val;
    ajint ival;
    
    AjBool required = ajFalse;
    AjBool ok       = ajFalse;

    ajint itry;
    AjBool failrange;
    AjBool warnrange;
    AjBool islong;
    AjBool truemin;
    
    ajlong imin;
    ajlong imax;
    AjPStr failmsg = NULL;
    AjBool iscalc = ajFalse;

    acdRangeTestCalc(thys);

    acdAttrToLong(thys, "minimum", INT_MIN, &imin);
    acdLog("minimum: %Ld\n", imin);
    
    acdAttrToLong(thys, "maximum", INT_MAX, &imax);
    acdLog("maximum: %Ld\n", imax);
    
    acdAttrToBool(thys, "failrange", ajTrue, &failrange);
    acdLog("failrange: %B\n", failrange);
    
    acdAttrToBool(thys, "warnrange", acdDoWarnRange, &warnrange);
    acdLog("warnrange: %B\n", warnrange);
    
    acdAttrToBool(thys, "large", AJFALSE, &islong);
    acdLog("large: %B\n", islong);
    
    acdAttrToBool(thys, "trueminimum", AJFALSE, &truemin);
    acdLog("trueminimum: %B\n", truemin);

    if(failrange && (imin > imax))
    {
        acdAttrResolve(thys, "rangemessage", &failmsg);
        if(ajStrGetLen(failmsg))
            acdErrorAcd(thys, "Invalid range: %S", failmsg);
        else
            acdErrorAcd(thys,
                        "Invalid range: "
                        "minimum value %d more than maximum %d",
                        imin, imax);
    }

    AJNEW0(val);		   /* create storage for the result */
    
    *val = 0;				/* set the default value */
    
    required = acdIsRequired(thys);
    acdReplyInitC(thys, "0", &acdReplyDef);
    
    acdLog("acdSetInt %S default '%S' Required: %B\n",
	   thys->Name, acdReplyDef, required);
    
    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ajStrAssignS(&acdReply, acdReplyDef);

	if(required)
	    acdUserGet(thys, &acdReply);

	acdLog(" reply: '%S' \n", acdReply);

	if(ajStrMatchC(acdReply, "default"))
	    ajStrAssignC(&acdReply, "0");

        if(islong)
            ok = ajStrToLong(acdReply, val);
        else
        {
            ok = ajStrToInt(acdReply, &ival);
            *val = ival;
        }
        
	acdLog(" modified reply: '%S' val: %Ld ok: %B\n", acdReply, *val, ok);

	if(!ok)
	    acdBadVal(thys, required,
		      "Invalid integer value '%S'", acdReply);
    }

    if(!ok)
	acdBadRetry(thys);

    if(islong && imin == INT_MIN)
        imin = LONG_MIN;

    if(islong && imax == INT_MAX)
        imax = LONG_MAX;

    if(iscalc) 
    {
        if(!truemin && *val < imin)
        {					/* reset within limits */
            if(warnrange)
                ajWarn("integer value out of range %Ld less than "
                       "(reset to) %Ld",
                       *val, imin);
            *val = imin;
        }

        if(*val > imax)
        {
            if(warnrange)
                ajWarn("integer value out of range %Ld more than "
                       "(reset to) %Ld",
                       *val, imax);
            *val = imax;
        }
    
        if(truemin && *val < imin)
        {					/* reset within limits */
            if(warnrange)
                ajWarn("integer value out of range %Ld less than "
                       "(reset to) %Ld",
                       *val, imin);
            *val = imin;
        }
    }

    else
    {
        if(*val < imin)
        {					/* reset within limits */
            if(warnrange)
                ajWarn("integer value out of range %Ld less than "
                       "(reset to) %Ld",
                       *val, imin);
            *val = imin;
        }

        if(*val > imax)
        {
            if(warnrange)
                ajWarn("integer value out of range %Ld more than "
                       "(reset to) %Ld",
                       *val, imax);
            *val = imax;
        }
    }

    thys->Value = val;
    ajStrFromLong(&thys->ValStr, *val);

    return;
}




/* @func ajAcdGetList *********************************************************
**
** Returns an item of type List as defined in a named ACD item,
** which is an array of strings terminated by a null value.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPStr*] String array of values with NULL for last element.
** @cre failure to find an item with the right name and type aborts.
**
** @release 1.0.0
** @@
******************************************************************************/

AjPStr* ajAcdGetList(const char *token)
{
    return acdGetValueRef(token, "list");
}




/* @func ajAcdGetListSingle ***************************************************
**
** Returns a single item from an array of type List as defined in a named
** ACD item, which is an array of strings terminated by a null value.
** Called by the application after all ACD values have been set, and
** simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPStr] String array of values with NULL for last element.
** @cre failure to find an item with the right name and type aborts.
**
** @release 4.0.0
** @@
******************************************************************************/

AjPStr ajAcdGetListSingle(const char *token)
{
    AjPStr *val;
    ajint i;

    val = acdGetValueSingle(token, "list");

    for(i=0; val[i]; i++)
	continue;

    if(i > 1)
	ajWarn("Single list value %s, but can choose %d values",
	       token, i);

    if(i < 1)
	ajWarn("Single list value %s, no value found: returning NULL value",
	       token);

    return val[0];
}




/* @funcstatic acdSetList *****************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD outfile item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** If a value is required and not yet given, prints out a header
** and the list of options, then asks for a selection or (if max is
** more than 1) a list of selections.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdSetList(AcdPAcd thys)
{
    AjPStr *val;
    
    AjBool required = ajFalse;
    AjBool ok       = ajFalse;

    ajint itry;
    ajint i;
    
    ajint min, max;
    
    val = NULL;				/* set the default value */
    
    required = acdIsRequired(thys);
    acdReplyInitC(thys, "", &acdReplyDef);
    
    acdAttrToInt(thys, "minimum", 1, &min);
    acdAttrToInt(thys, "maximum", 1, &max);
    
    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */
	
	ajStrAssignS(&acdReply, acdReplyDef);
	
	if(required)
	{
	    acdListPrompt(thys);
	    acdUserGet(thys, &acdReply);
	}
	
	val = acdListValue(thys, min, max, acdReply);

	if(!val)
	{
	    acdBadVal(thys, required, "Bad menu option '%S'", acdReply);
	    ok = ajFalse;
	}
    }

    if(!ok)
	acdBadRetry(thys);
    
    thys->Value = val;

    for(i=0; val[i]; i++)
    {
	acdLog("Storing val[%d] '%S'\n", i,val[i]);

	if(i)
	    ajStrAppendC(&thys->ValStr, ";");

	ajStrAppendS(&thys->ValStr, val[i]);
    }
    
    return;
}




/* @func ajAcdGetMatrix *******************************************************
**
** Returns an item of type Matrix as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPMatrix] Matrix object.
** @cre failure to find an item with the right name and type aborts.
**
** @release 1.0.0
** @@
******************************************************************************/

AjPMatrix ajAcdGetMatrix(const char *token)
{
    return acdGetValueRef(token, "matrix");
}




/* @func ajAcdGetMatrixf ******************************************************
**
** Returns an item of type Matrix as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPMatrixf] Float Matrix object.
** @cre failure to find an item with the right name and type aborts.
**
** @release 1.0.0
** @@
******************************************************************************/

AjPMatrixf ajAcdGetMatrixf(const char *token)
{
    return acdGetValueRef(token, "matrixf");
}




/* @funcstatic acdSetMatrix ***************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD matrix item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajMatrixNewFile
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdSetMatrix(AcdPAcd thys)
{
    AjPMatrix val;
    
    AjBool required = ajFalse;
    AjBool ok       = ajFalse;

    ajint itry;
    AjBool isprot;
    
    AjPStr infname = NULL;
    
    val = NULL;				/* set the default value */
    
    acdAttrToBool(thys, "protein", ajTrue, &isprot);

    if(isprot)
    {
	acdAttrResolve(thys, "pname", &infname);
	if(!ajStrGetLen(infname))
	    ajStrAssignC(&infname, DEFBLOSUM);
    }
    else
    {
	acdAttrResolve(thys, "nname", &infname);

	if(!ajStrGetLen(infname))
	    ajStrAssignC(&infname, DEFDNA);
    }

    required = acdIsRequired(thys);
    acdReplyInitS(thys, infname, &acdReplyDef);
    ajStrDel(&infname);
    
    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */
	
	ajStrAssignS(&acdReply, acdReplyDef);
	
	if(required)
	    acdUserGet(thys, &acdReply);
	
	if(ajStrGetLen(acdReply))
	{
	    val = ajMatrixNewFile(acdReply);
            if(!val)
	    {
		acdBadVal(thys, required,
			  "Unable to read matrix '%S'", acdReply);
		ok = ajFalse;
	    }
	}
	else
	{
	    acdBadVal(thys, required, "Matrix is required");
	    ok = ajFalse;
	}
    }
    
    if(!ok)
	acdBadRetry(thys);
    
    thys->Value = val;
    ajStrAssignS(&thys->ValStr, acdReply);
    
    return;
}




/* @funcstatic acdSetMatrixf **************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD matrix item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajMatrixfNewFile
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdSetMatrixf(AcdPAcd thys)
{
    AjPMatrixf val;
    
    AjBool required = ajFalse;
    AjBool ok       = ajFalse;

    ajint itry;
    AjBool isprot;
    
    AjPStr infname = NULL;
    

    val = NULL;				/* set the default value */
    
    acdAttrToBool(thys, "protein", ajTrue, &isprot);

    if(isprot)
    {
	acdAttrResolve(thys, "pname", &infname);

	if(!ajStrGetLen(infname))
	    ajStrAssignC(&infname, DEFBLOSUM);
    }
    else
    {
	acdAttrResolve(thys, "nname", &infname);

	if(!ajStrGetLen(infname))
	    ajStrAssignC(&infname, DEFDNA);
    }

    required = acdIsRequired(thys);
    acdReplyInitS(thys, infname, &acdReplyDef);
    ajStrDel(&infname);
    
    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */

	ajStrAssignS(&acdReply, acdReplyDef);

	if(required)
	    acdUserGet(thys, &acdReply);

	if(ajStrGetLen(acdReply))
	{
	    val = ajMatrixfNewFile(acdReply);
	    if(!val)
            {
		acdBadVal(thys, required,
			  "Unable to read matrix '%S'", acdReply);
		ok = ajFalse;
	    }
	}
	else
	{
	    acdBadVal(thys, required, "Matrix is required");
	    ok = ajFalse;
	}
    }

    if(!ok)
	acdBadRetry(thys);
    
    thys->Value = val;
    ajStrAssignS(&thys->ValStr, acdReply);
    
    return;
}




/* @func ajAcdGetObo **********************************************************
**
** Returns an item of type Obo as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPObo] Obo term object
** @cre failure to find an item with the right name and type aborts.
**
** @release 6.4.0
** @@
******************************************************************************/

AjPObo ajAcdGetObo(const char *token)
{
    AjPOboall val = acdGetValue(token, "obo");
    if(val->Multi)
        ajWarn("ajAcdGetObo request single obo term but maxreads > 1");

    val->Returned = ajTrue;

    return val->Obo;
}




/* @func ajAcdGetOboall *******************************************************
**
** Returns an input stream of an item of type Obo as defined in a
** named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPOboall] Obo input stream
** @cre failure to find an item with the right name and type aborts.
**
** @release 6.4.0
** @@
******************************************************************************/

AjPOboall ajAcdGetOboall(const char *token)
{
    AjPOboall val = acdGetValueRef(token, "obo");

    if(!val->Multi)
        ajWarn("ajAcdGetOboall request obo input stream but maxreads is 1");

    return val;
}




/* @funcstatic acdSetObo ******************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD obo term input
**
** Understands all attributes and associated qualifiers for this item type.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void acdSetObo(AcdPAcd thys)
{
    AjPOboall val;

    AjBool required = ajFalse;
    AjBool ok       = ajFalse;
    AjBool nullok   = ajFalse;
    ajint itry;

    ajint maxreads;

    AjPStr infname = NULL;
    
    val = ajOboallNew();        /* set the default value */

    acdAttrToBool(thys, "nullok", ajFalse, &nullok);
    acdAttrToInt(thys, "maxreads", INT_MAX, &maxreads);

    acdInFilename(&infname);
    required = acdIsRequired(thys);
    acdReplyInitS(thys, infname, &acdReplyDef);
    ajStrDel(&infname);

    acdAttrToBool(thys, "entry", ajFalse, &val->Oboin->Input->Text);

    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */

	ajStrAssignS(&acdReply, acdReplyDef);

	if(required)
	    acdUserGet(thys, &acdReply);

	if(!ajStrGetLen(acdReply) && nullok)
        {
            ajOboallDel(&val);
	    break;
	}

	ajOboinQryS(val->Oboin, acdReply);

	acdGetValueAssoc(thys, "iformat", &val->Oboin->Input->Formatstr);
	acdGetValueAssoc(thys, "iquery", &val->Oboin->Input->QryFields);
	acdQualToLong(thys, "ioffset", 0L, &val->Oboin->Input->Fpos,
                      &acdTmpStr);

        ok = ajOboinRead(val->Oboin, val->Obo);
    }

    if(!ok)
	acdBadRetry(thys);

    if(maxreads > 1)
        val->Multi = ajTrue;

    acdInFileSave(acdReply, ajOboallGetoboId(val), ajTrue);

    thys->Value = val;
    ajStrAssignS(&thys->ValStr, acdReply);
    
    return;
}




/* @funcstatic acdSetOutType **************************************************
**
** Generic definition for any of the ACD formatted output types.
** May be replaced by a specific acdSet function if additional attributes
** or qualifiers are to be processed.
**
** Understands all attributes and associated qualifiers for these item types.
**
** The default value (if no other available) is a null string, which
** is invalid.
**
** Associated qualifiers "-oformat"
** are stored in the object and applied to the data on output.
**
** @param [u] thys [AcdPAcd] ACD item.
** @param [r] type [const char*] Standard output type name.
** @return [AjPOutfile] Output file object of the specified type
**
** @release 2.9.0
** @@
******************************************************************************/

static AjPOutfile acdSetOutType(AcdPAcd thys, const char* type)
{
    AjPOutfile val = NULL;
    
    AjBool required = ajFalse;
    AjBool ok       = ajFalse;

    ajint itry;
    AjBool nullok;
    AjBool nulldefault;
    
    AjPStr name      = NULL;
    AjPStr ext       = NULL;
    AjPStr fmt       = NULL;
    AjPStr dir       = NULL;
    ajint itype = -1;
    ajint i;
    
    val = NULL;

    for (i=0; acdOuttype[i].Name; i++)
	if (ajCharMatchC(acdOuttype[i].Name, type))
	{
	    itype = i;
	    break;
	}

    if (itype < 0)
	acdError("Unknown output type '%s'", type);

    acdAttrToBool(thys, "nullok", ajFalse, &nullok);
    acdAttrToBool(thys, "nulldefault", ajFalse, &nulldefault);
    acdAttrResolve(thys, "name", &name);

    if (!acdGetValueAssoc(thys, "oformat", &fmt))
	ajStrAssignC(&fmt, acdOuttype[itype].Format);

    if(!acdAttrResolve(thys, "extension", &ext))
	ajStrAssignS(&ext, fmt);

    acdGetValueAssoc(thys, "odirectory", &dir);

    acdOutDirectory(&dir);
    required = acdIsRequired(thys);
    if(nullok && nulldefault)
    {
	if (acdDefinedEmpty(thys))  /* user set to empty - make default name */
	    acdOutFilename(&acdReplyDef, name, ext);
	else				/* leave empty */
	    acdReplyInitC(thys, "", &acdReplyDef);
    }
    else
    {
	acdOutFilename(&acdTmpOutFName, name, ext);
	acdReplyInitS(thys, acdTmpOutFName, &acdReplyDef);
    }

    ajStrDel(&name);
    ajStrDel(&ext);

    if(acdOuttype[itype].Prompt)
	(*acdOuttype[itype].Prompt)(thys);
    
    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */

	ajStrAssignS(&acdReply, acdReplyDef);

	if(required)
	    acdUserGet(thys, &acdReply);

	if(ajStrGetLen(acdReply))
	{
	    ajStrAssignS(&acdOutFullFName, acdReply);
	    ajFilenameReplacePathS(&acdOutFullFName, dir);
	    val = ajOutfileNewNameS(acdOutFullFName);

	    if(!ok)
	    {
		acdBadVal(thys, required,
			  "Unable to open output file '%S'",
			  acdOutFullFName);
		ajOutfileClose(&val);
	    }

	    ajStrAssignEmptyS(&val->Formatstr, fmt);
	    ajStrAssignEmptyC(&val->Type, type);

	    if(acdOuttype[itype].Outformat)
	    {
		if(!(*acdOuttype[itype].Outformat)(val->Formatstr,
                                                   &val->Format))
		{			/* test acdc-outbadformat */
		    ajDie("Output option -%S: "
			  "Format validation failed for type '%s'",
			  thys->Name, type);
		}
	    }
    
	}
	else
	    if(!nullok)
	    {
		acdBadVal(thys, required,
			  "Output file is required");
		ok = ajFalse;
	    }
    }

    if(!ok)
	acdBadRetry(thys);
    
    thys->Value = val;
    ajStrAssignS(&thys->ValStr, acdOutFullFName);

    ajStrDel(&fmt);
    ajStrDel(&dir);

    return val;
}




/* @func ajAcdGetOutassembly **************************************************
**
** Returns an item of type Outassembly as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPOutfile] File object. The file was already opened by
**         acdSetOut so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
**
** @release 6.5.0
** @@
******************************************************************************/

AjPOutfile ajAcdGetOutassembly(const char *token)
{
    return acdGetValueRef(token, "outassembly");
}




/* @funcstatic acdSetOutassembly **********************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD outassembly item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other available) is a null string, which
** is invalid.
**
** Associated qualifiers "-oformat"
** are stored in the object and applied to the data on output.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void acdSetOutassembly(AcdPAcd thys)
{
    acdSetOutType(thys, "outassembly");

    return;
}




/* @func ajAcdGetOutcodon *****************************************************
**
** Returns an item of type Outcodon as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPOutfile] File object. The file was already opened by
**         acdSetOut so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
**
** @release 2.9.0
** @@
******************************************************************************/

AjPOutfile ajAcdGetOutcodon(const char *token)
{
    return acdGetValueRef(token, "outcodon");
}




/* @funcstatic acdSetOutcodon *************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD outcodon item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other available) is a null string, which
** is invalid.
**
** Associated qualifiers "-oformat"
** are stored in the object and applied to the data on output.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajSeqRead
**
** @release 2.9.0
** @@
******************************************************************************/

static void acdSetOutcodon(AcdPAcd thys)
{
    acdSetOutType(thys, "outcodon");
    return;
}




/* @func ajAcdGetOutcpdb ******************************************************
**
** Returns an item of type Outcpdb as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPOutfile] File object. The file was already opened by
**         acdSetOut so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
**
** @release 2.9.0
** @@
******************************************************************************/

AjPOutfile ajAcdGetOutcpdb(const char *token)
{
    return acdGetValueRef(token, "outcpdb");
}




/* @funcstatic acdSetOutcpdb **************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD outcpdb item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other available) is a null string, which
** is invalid.
**
** Associated qualifiers "-oformat"
** are stored in the object and applied to the data on output.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajSeqRead
**
** @release 2.9.0
** @@
******************************************************************************/

static void acdSetOutcpdb(AcdPAcd thys)
{
    acdSetOutType(thys, "outcpdb");
    return;
}




/* @func ajAcdGetOutdata ******************************************************
**
** Returns an item of type Outdata as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPOutfile] File object. The file was already opened by
**         acdSetOut so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
**
** @release 2.9.0
** @@
******************************************************************************/

AjPOutfile ajAcdGetOutdata(const char *token)
{
    return acdGetValueRef(token, "outdata");
}




/* @funcstatic acdSetOutdata **************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD outdata item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other available) is a null string, which
** is invalid.
**
** Associated qualifiers "-oformat"
** are stored in the object and applied to the data on output.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajSeqRead
**
** @release 2.9.0
** @@
******************************************************************************/

static void acdSetOutdata(AcdPAcd thys)
{
    acdSetOutType(thys, "outdata");

    return;
}




/* @func ajAcdGetOutdir *******************************************************
**
** Returns an item of type AjPDirout which has been validated as an output
** directory.
**
** Optionally can be forced to have a fully qualified path when returned.
**
** @param [r] token [const char*] Text token name
** @return [AjPDirout] Output directory object
** @cre failure to find an item with the right name and type aborts.
**
** @release 2.9.0
** @@
******************************************************************************/

AjPDirout ajAcdGetOutdir(const char *token)
{
    return acdGetValueRef(token, "outdir");
}




/* @func ajAcdGetOutdirName ***************************************************
**
** Returns an item of type AjPStr which has been validated as an output
** directory.
**
** Optionally can be forced to have a fully qualified path when returned.
**
** @param [r] token [const char*] Text token name
** @return [AjPStr] String containing a directory name
** @cre failure to find an item with the right name and type aborts.
**
** @release 2.9.0
** @@
******************************************************************************/

AjPStr ajAcdGetOutdirName(const char *token)
{
    AjPStr ret = NULL;
    AjPDirout dirout;

    dirout =  acdGetValue(token, "outdir");
    ret = ajStrNewS(ajDiroutGetPath(dirout));

    return ret;
}




/* @funcstatic acdSetOutdir ***************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD output directory item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value is "." the current directory.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
**
** @release 2.9.0
** @@
******************************************************************************/

static void acdSetOutdir(AcdPAcd thys)
{
    AjPDirout val;

    AjBool required = ajFalse;
    AjBool ok       = ajFalse;

    ajint itry;

    AjBool nullok = ajFalse;
    AjBool dopath = ajFalse;
    AjBool create = ajFalse;
    AjPStr ext = NULL;

    val = NULL;				/* set the default value */

    acdAttrToBool(thys, "fullpath", ajFalse, &dopath);
    acdAttrToBool(thys, "nullok", ajFalse, &nullok);
    acdAttrToBool(thys, "create", ajFalse, &create);
    acdGetValueAssoc(thys, "extension", &ext);

    required = acdIsRequired(thys);
    acdReplyInitC(thys, ".", &acdReplyDef);

    acdPromptOutdir(thys);
    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */

	ajStrAssignS(&acdReply, acdReplyDef);

	if(required)
	    acdUserGet(thys, &acdReply);

	if(ajStrGetLen(acdReply))
	{
	    ajDebug("acdSetOutdir start reply '%S' dopath:%B ok:%B\n",
		    acdReply, dopath,ok);
	    if(dopath)
		ok = ajDirnameFillPath(&acdReply);

	    ajDebug("acdSetOutdir dir done reply '%S' dopath:%B ok:%B\n",
		    acdReply, dopath,ok);
	    if (ok)
	    {
		val = ajDiroutNewPathExt(acdReply, ext);

		if (!val)
		    ok = ajFalse;
                else
                {
                    if(create)
                        ok = ajDiroutOpen(val);
                    else
                        ok = ajDiroutExists(val);
                }
                
	    }

	    if(!ok)
		acdBadVal(thys, required,
			  "Unable to open output directory '%S'",
			  acdReply);
	}
	else
	    if(!nullok)
	    {
		acdBadVal(thys, required,
			  "Directory path is required");
		ok = ajFalse;
	    }
    }
    if(!ok)
	acdBadRetry(thys);

    thys->Value = val;
    ajStrAssignS(&thys->ValStr, acdReply);

    ajStrDel(&ext);

    return;
}




/* @func ajAcdGetOutdiscrete **************************************************
**
** Returns an item of type Outdiscrete as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPOutfile] File object. The file was already opened by
**         acdSetOut so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
**
** @release 2.9.0
** @@
******************************************************************************/

AjPOutfile ajAcdGetOutdiscrete(const char *token)
{
    return acdGetValueRef(token, "outdiscrete");
}




/* @funcstatic acdSetOutdiscrete **********************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD outdiscrete item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other available) is a null string, which
** is invalid.
**
** Associated qualifiers "-oformat"
** are stored in the object and applied to the data on output.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajSeqRead
**
** @release 2.9.0
** @@
******************************************************************************/

static void acdSetOutdiscrete(AcdPAcd thys)
{
    acdSetOutType(thys, "outdiscrete");
    return;
}




/* @func ajAcdGetOutdistance **************************************************
**
** Returns an item of type Outdistance as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPOutfile] File object. The file was already opened by
**         acdSetOut so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
**
** @release 2.9.0
** @@
******************************************************************************/

AjPOutfile ajAcdGetOutdistance(const char *token)
{
    return acdGetValueRef(token, "outdistance");
}




/* @funcstatic acdSetOutdistance **********************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD outdistance item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other available) is a null string, which
** is invalid.
**
** Associated qualifiers "-oformat"
** are stored in the object and applied to the data on output.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajSeqRead
**
** @release 2.9.0
** @@
******************************************************************************/

static void acdSetOutdistance(AcdPAcd thys)
{
    acdSetOutType(thys, "outdistance");
    return;
}




/* @func ajAcdGetOutfile ******************************************************
**
** Returns an item of type Outfile as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPFile] File object. The file was already opened by
**         acdSetOutfile so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
**
** @release 1.0.0
** @@
******************************************************************************/

AjPFile ajAcdGetOutfile(const char *token)
{
    return acdGetValueRef(token, "outfile");
}




/* @func ajAcdGetOutfileName **************************************************
**
** Returns the filename of an item of type Outfile as defined in a
** named ACD item. The file is closed and can be reused. If the file
** had the append attribute set it still has the original contents.
**
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPStr] Filename.
** @cre failure to find an item with the right name and type aborts.
**
** @release 6.3.0
** @@
******************************************************************************/

AjPStr ajAcdGetOutfileName(const char *token)
{
    AjPStr ret;
    AjPFile outfile;

    outfile = acdGetValueRef(token, "outfile");

    if(!outfile)
        return NULL;

    ret = ajStrNewS(ajFileGetPrintnameS(outfile));

    ajFileClose(&outfile);

    return ret;
}




/* @funcstatic acdSetOutfile **************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD outfile item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value, if stdout or filtering is on is "stdout" for the
** first file.
**
** Otherwise an output file name is constructed.
**
** Various file naming options are defined, but not yet implemented here.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajFileNewOut
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdSetOutfile(AcdPAcd thys)
{
    AjPFile val;
    
    AjBool required = ajFalse;
    AjBool ok       = ajFalse;

    ajint itry;
    AjBool nullok;
    AjBool nulldefault;
    AjBool append;
    
    AjPStr name      = NULL;
    AjPStr ext       = NULL;
    AjPStr dir       = NULL;
    
    val = NULL;				/* set the default value */
    
    acdAttrResolve(thys, "name", &name);
    acdAttrResolve(thys, "extension", &ext);
    acdGetValueAssoc(thys, "odirectory", &dir);
    
    acdAttrToBool(thys, "nullok", ajFalse, &nullok);
    acdAttrToBool(thys, "nulldefault", ajFalse, &nulldefault);
    acdAttrToBool(thys, "append", ajFalse, &append);
    acdOutDirectory(&dir);
    
    required = acdIsRequired(thys);

    if(nullok && nulldefault)
    {
	if (acdDefinedEmpty(thys))  /* user set to empty - make default name */
	    acdOutFilename(&acdReplyDef, name, ext);
	else				/* leave empty */
	    acdReplyInitC(thys, "", &acdReplyDef);
    }
    else
    {
	acdOutFilename(&acdTmpOutFName, name, ext);
	acdReplyInitS(thys, acdTmpOutFName, &acdReplyDef);
    }
    ajStrDel(&name);
    ajStrDel(&ext);

    acdPromptOutfile(thys);
    
    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */
	
	ajStrAssignS(&acdReply, acdReplyDef);
	
	if(required)
	    acdUserGet(thys, &acdReply);
	
	ajStrAssignS(&acdOutFullFName, acdReply);

	if(ajStrGetLen(acdReply))
	{
	    ajFilenameReplacePathS(&acdOutFullFName, dir);

	    if(append)
		val = ajFileNewOutappendNameS(acdOutFullFName);
	    else
		val = ajFileNewOutNameS(acdOutFullFName);

	    if(!val)
	    {
		acdBadVal(thys, required,
			  "Unable to open file '%S' for output",
			  acdOutFullFName);
		ok = ajFalse;
	    }
	}
	else
	    if(!nullok)
	    {
		acdBadVal(thys, required,
			  "Output file is required");
		ok = ajFalse;
	    }
    }
    
    if(!ok)
	acdBadRetry(thys);
    
    thys->Value = val;
    ajStrAssignS(&thys->ValStr, acdOutFullFName);
    
    ajStrDel(&dir);

    return;
}




/* @func ajAcdGetOutfreq ******************************************************
**
** Returns an item of type Outfreq as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPOutfile] File object. The file was already opened by
**         acdSetOut so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
**
** @release 2.9.0
** @@
******************************************************************************/

AjPOutfile ajAcdGetOutfreq(const char *token)
{
    return acdGetValueRef(token, "outfreq");
}




/* @funcstatic acdSetOutfreq **************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD outfreq item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other available) is a null string, which
** is invalid.
**
** Associated qualifiers "-oformat"
** are stored in the object and applied to the data on output.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajSeqRead
**
** @release 2.9.0
** @@
******************************************************************************/

static void acdSetOutfreq(AcdPAcd thys)
{
    acdSetOutType(thys, "outfreq");
    return;
}




/* @func ajAcdGetOutmatrix ****************************************************
**
** Returns an item of type Outmatrix as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPOutfile] File object. The file was already opened by
**         acdSetOut so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
**
** @release 2.9.0
** @@
******************************************************************************/

AjPOutfile ajAcdGetOutmatrix(const char *token)
{
    return acdGetValueRef(token, "outmatrix");
}




/* @funcstatic acdSetOutmatrix ************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD outmatrix item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other available) is a null string, which
** is invalid.
**
** Associated qualifiers "-oformat"
** are stored in the object and applied to the data on output.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajSeqRead
**
** @release 2.9.0
** @@
******************************************************************************/

static void acdSetOutmatrix(AcdPAcd thys)
{
    acdSetOutType(thys, "outmatrix");

    return;
}




/* @func ajAcdGetOutmatrixf ***************************************************
**
** Returns an item of type Outmatrixf as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPOutfile] File object. The file was already opened by
**         acdSetOut so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
**
** @release 2.9.0
** @@
******************************************************************************/

AjPOutfile ajAcdGetOutmatrixf(const char *token)
{
    return acdGetValueRef(token, "outmatrixf");
}




/* @funcstatic acdSetOutmatrixf ***********************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD outmatrixf item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other available) is a null string, which
** is invalid.
**
** Associated qualifiers "-oformat"
** are stored in the object and applied to the data on output.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajSeqRead
**
** @release 2.9.0
** @@
******************************************************************************/

static void acdSetOutmatrixf(AcdPAcd thys)
{
    acdSetOutType(thys, "outmatrixf");
    return;
}




/* @func ajAcdGetOutobo *******************************************************
**
** Returns an item of type Outobo as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPOutfile] File object. The file was already opened by
**         acdSetOut so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
**
** @release 6.4.0
** @@
******************************************************************************/

AjPOutfile ajAcdGetOutobo(const char *token)
{
    return acdGetValueRef(token, "outobo");
}




/* @funcstatic acdSetOutobo ***************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD outobo item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other available) is a null string, which
** is invalid.
**
** Associated qualifiers "-oformat"
** are stored in the object and applied to the data on output.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajSeqRead
**
** @release 6.4.0
** @@
******************************************************************************/

static void acdSetOutobo(AcdPAcd thys)
{
    acdSetOutType(thys, "outobo");

    return;
}




/* @func ajAcdGetOutproperties ************************************************
**
** Returns an item of type Outproperties as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPOutfile] File object. The file was already opened by
**         acdSetOut so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
**
** @release 2.9.0
** @@
******************************************************************************/

AjPOutfile ajAcdGetOutproperties(const char *token)
{
    return acdGetValueRef(token, "outproperties");
}




/* @funcstatic acdSetOutproperties ********************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD outproperties item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other available) is a null string, which
** is invalid.
**
** Associated qualifiers "-oformat"
** are stored in the object and applied to the data on output.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajSeqRead
**
** @release 2.9.0
** @@
******************************************************************************/

static void acdSetOutproperties(AcdPAcd thys)
{
    acdSetOutType(thys, "properties");

    return;
}




/* @func ajAcdGetOutrefseq ****************************************************
**
** Returns an item of type Outrefseq as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPOutfile] File object. The file was already opened by
**         acdSetOut so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
**
** @release 6.5.0
** @@
******************************************************************************/

AjPOutfile ajAcdGetOutrefseq(const char *token)
{
    return acdGetValueRef(token, "outrefseq");
}




/* @funcstatic acdSetOutrefseq **********************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD outrefseq item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other available) is a null string, which
** is invalid.
**
** Associated qualifiers "-oformat"
** are stored in the object and applied to the data on output.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void acdSetOutrefseq(AcdPAcd thys)
{
    acdSetOutType(thys, "outrefseq");

    return;
}




/* @func ajAcdGetOutresource **************************************************
**
** Returns an item of type Outresource as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPOutfile] File object. The file was already opened by
**         acdSetOut so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
**
** @release 6.4.0
** @@
******************************************************************************/

AjPOutfile ajAcdGetOutresource(const char *token)
{
    return acdGetValueRef(token, "outresource");
}




/* @funcstatic acdSetOutresource **********************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD data resource output item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other available) is a null string, which
** is invalid.
**
** Associated qualifiers "-oformat"
** are stored in the object and applied to the data on output.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajSeqRead
**
** @release 6.4.0
** @@
******************************************************************************/

static void acdSetOutresource(AcdPAcd thys)
{
    acdSetOutType(thys, "outresource");

    return;
}




/* @func ajAcdGetOutscop ******************************************************
**
** Returns an item of type Outscop as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPOutfile] File object. The file was already opened by
**         acdSetOut so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
**
** @release 2.9.0
** @@
******************************************************************************/

AjPOutfile ajAcdGetOutscop(const char *token)
{
    return acdGetValueRef(token, "outscop");
}




/* @funcstatic acdSetOutscop **************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD outscop item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other available) is a null string, which
** is invalid.
**
** Associated qualifiers "-oformat"
** are stored in the object and applied to the data on output.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajSeqRead
**
** @release 2.9.0
** @@
******************************************************************************/

static void acdSetOutscop(AcdPAcd thys)
{
    acdSetOutType(thys, "outscop");

    return;
}




/* @func ajAcdGetOuttaxon *****************************************************
**
** Returns an item of type Outtaxon as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPOutfile] File object. The file was already opened by
**         acdSetOut so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
**
** @release 6.4.0
** @@
******************************************************************************/

AjPOutfile ajAcdGetOuttaxon(const char *token)
{
    return acdGetValueRef(token, "outtaxon");
}




/* @funcstatic acdSetOuttaxon *************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD outtaxon item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other available) is a null string, which
** is invalid.
**
** Associated qualifiers "-oformat"
** are stored in the object and applied to the data on output.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajSeqRead
**
** @release 6.4.0
** @@
******************************************************************************/

static void acdSetOuttaxon(AcdPAcd thys)
{
    acdSetOutType(thys, "outtaxon");

    return;
}




/* @func ajAcdGetOuttext ******************************************************
**
** Returns an item of type Outtext as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPOutfile] File object. The file was already opened by
**         acdSetOut so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
**
** @release 6.4.0
** @@
******************************************************************************/

AjPOutfile ajAcdGetOuttext(const char *token)
{
    return acdGetValueRef(token, "outtext");
}




/* @funcstatic acdSetOuttext **************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD outtext item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other available) is a null string, which
** is invalid.
**
** Associated qualifiers "-oformat"
** are stored in the object and applied to the data on output.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajSeqRead
**
** @release 6.4.0
** @@
******************************************************************************/

static void acdSetOuttext(AcdPAcd thys)
{
    acdSetOutType(thys, "outtext");

    return;
}




/* @func ajAcdGetOuttree ******************************************************
**
** Returns an item of type Outtree as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPOutfile] File object. The file was already opened by
**         acdSetOut so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
**
** @release 2.9.0
** @@
******************************************************************************/

AjPOutfile ajAcdGetOuttree(const char *token)
{
    return acdGetValueRef(token, "outtree");
}




/* @funcstatic acdSetOuttree **************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD outtree item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other available) is a null string, which
** is invalid.
**
** Associated qualifiers "-oformat"
** are stored in the object and applied to the data on output.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajSeqRead
**
** @release 2.9.0
** @@
******************************************************************************/

static void acdSetOuttree(AcdPAcd thys)
{
    acdSetOutType(thys, "outtree");

    return;
}




/* @func ajAcdGetOuturl *******************************************************
**
** Returns an item of type Outurl as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPOutfile] File object. The file was already opened by
**         acdSetOut so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
**
** @release 6.4.0
** @@
******************************************************************************/

AjPOutfile ajAcdGetOuturl(const char *token)
{
    return acdGetValueRef(token, "outurl");
}




/* @funcstatic acdSetOuturl ***************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD outurl item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other available) is a null string, which
** is invalid.
**
** Associated qualifiers "-oformat"
** are stored in the object and applied to the data on output.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void acdSetOuturl(AcdPAcd thys)
{
    acdSetOutType(thys, "outurl");

    return;
}




/* @func ajAcdGetOutvariation *************************************************
**
** Returns an item of type Outvariation as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPOutfile] File object. The file was already opened by
**         acdSetOut so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
**
** @release 6.4.0
** @@
******************************************************************************/

AjPOutfile ajAcdGetOutvariation(const char *token)
{
    return acdGetValueRef(token, "outvariation");
}




/* @funcstatic acdSetOutvariation *********************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD outvariation item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other available) is a null string, which
** is invalid.
**
** Associated qualifiers "-oformat"
** are stored in the object and applied to the data on output.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void acdSetOutvariation(AcdPAcd thys)
{
    acdSetOutType(thys, "outvariation");

    return;
}




/* @func ajAcdGetOutxml *******************************************************
**
** Returns an item of type Outxml as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPOutfile] File object. The file was already opened by
**         acdSetOut so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
**
** @release 6.6.0
** @@
******************************************************************************/

AjPOutfile ajAcdGetOutxml(const char *token)
{
    return acdGetValueRef(token, "outxml");
}




/* @funcstatic acdSetOutxml ***************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD outxml item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other available) is a null string, which
** is invalid.
**
** Associated qualifiers "-oformat"
** are stored in the object and applied to the data on output.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
**
** @release 6.6.0
** @@
******************************************************************************/

static void acdSetOutxml(AcdPAcd thys)
{
    acdSetOutType(thys, "outxml");

    return;
}




/* @func ajAcdGetPattern ******************************************************
**
** Returns an item of type Pattern as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPPatlistSeq] Compiled pattern list of sequence patterns.
**                     The original pattern string is available
**                     through a call to ajAcdGetValue
** @cre failure to find an item with the right name and type aborts.
**
** @release 4.0.0
** @@
******************************************************************************/

AjPPatlistSeq ajAcdGetPattern(const char *token)
{
    return acdGetValueRef(token, "pattern");
}




/* @funcstatic acdSetPattern **************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD sequence pattern item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other is available) is NULL.
**
** Attributes for minimum and maximum length are applied with error
** messages if exceeded.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

static void acdSetPattern(AcdPAcd thys)
{    
    AjPPatlistSeq val = NULL;
    
    AjBool required = ajFalse;
    AjBool ok       = ajFalse;

    AjBool upper;
    AjBool lower;
    ajint itry;
    
    ajint minlen;
    ajint maxlen;
    ajint maxsize;
    ajint len;
    AjPStr type = NULL;
    AjPStr patname = NULL;
    AjPStr fmt = NULL;
    ajint mismatch = 0;
    AjBool isprotein = ajTrue;
    
    acdAttrToInt(thys, "minlength", 1, &minlen);
    
    acdAttrToInt(thys, "maxlength", INT_MAX, &maxlen);
    acdAttrToBool(thys, "upper", ajFalse, &upper);
    acdAttrToBool(thys, "lower", ajFalse, &lower);
    acdAttrToStr(thys, "type", "protein", &type);
    acdAttrToInt(thys, "maxsize", INT_MAX, &maxsize);
    acdQualToInt(thys, "pmismatch", 0, &mismatch, &acdTmpStr);
    acdGetValueAssoc(thys, "pname", &patname);
    acdGetValueAssoc(thys, "pformat", &fmt);
    ajStrFmtLower(&type);

    if(ajStrGetCharFirst(type) != 'p')
	isprotein = ajFalse;

    if(mismatch < 0)
	mismatch = 0;

    ajDebug("acdSetPattern name '%S' mismatch %d type '%S' protein %B\n",
	    patname, mismatch, type, isprotein);
    
    required = acdIsRequired(thys);
    acdReplyInitC(thys, "", &acdReplyDef);
    
    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */

	if(val)
	    ajPatlistSeqDel(&val);
	
	ajStrAssignS(&acdReply, acdReplyDef);
	
	if(required)
	    acdUserGet(thys, &acdReply);
	
	len = ajStrGetLen(acdReply);
	
	if(len < minlen)
	{
	    acdBadVal(thys, required,
		      "Too short (%S) - minimum length is %d characters",
		      thys->Name, minlen);
	    ok = ajFalse;
	}

	if(len > maxlen)
	{
	    acdBadVal(thys, required,
		      "Too long (%S) - maximum length is %d characters",
		      thys->Name,maxlen);
	    ok = ajFalse;
	}

	if(ok)
	    val = ajPatlistSeqRead(acdReply, patname, fmt,
				   isprotein, mismatch);

	if(ok && !val)
	{
	    acdBadVal(thys, required,
		      "Bad pattern definition:\n   '%S'",
		      acdReply);
	    ok = ajFalse;
	}	
    }

    if(!ok)
	acdBadRetry(thys);

    ajStrDel(&patname);
    ajStrDel(&fmt);
    ajStrDel(&type);

    /* regexps have special set attributes the same as strings */
    
    thys->SAttr = acdAttrListCount(acdCalcRegexp);
    thys->SetAttr = &acdCalcRegexp[0];
    thys->SetStr = AJCALLOC0(thys->SAttr, sizeof(AjPStr));
    
    ajStrFromInt(&thys->SetStr[0], ajStrGetLen(acdReply));
    
    thys->Value = val;
    ajStrAssignS(&thys->ValStr, acdReply);
    
    return;
}




/* @func ajAcdGetProperties ***************************************************
**
** Returns an item of type Properties as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPPhyloProp] Properties array. The data was already set by
**         acdSetProperties so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
**
** @release 2.8.0
** @@
******************************************************************************/

AjPPhyloProp ajAcdGetProperties(const char *token)
{
    return acdGetValueRef(token, "properties");
}




/* @funcstatic acdSetProperties ***********************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD proerties file item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other is available) is an empty string.
**
** Attributes for length and maximum property character are applied with error
** messages if exceeded.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
**
** @release 2.8.0
** @@
******************************************************************************/

static void acdSetProperties(AcdPAcd thys)
{
    AjPPhyloProp val;
    
    AjBool required = ajFalse;
    AjBool ok       = ajFalse;

    AjPStr infname  = NULL;
    ajint itry;
    
    AjBool nullok = ajFalse;
    ajint size;
    ajint len;
    AjPStr propchars = NULL;

    val = NULL;

    acdAttrToBool(thys, "nullok", ajFalse, &nullok);
    acdAttrToInt(thys, "length", 0, &len);
    acdAttrToInt(thys, "size", 1, &size);
    acdAttrToStr(thys, "characters", "", &propchars);

    ajDebug("acdSetProperties len: %d size: %d\n", len, size);

    acdInFilename(&infname);
    required = acdIsRequired(thys);
    acdReplyInitS(thys, infname, &acdReplyDef);
    acdPromptInfile(thys);
    ajStrDel(&infname);
  
    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */

	ajStrAssignS(&acdReply, acdReplyDef);

	if(required)
	    acdUserGet(thys, &acdReply);

	if(ajStrGetLen(acdReply))
	{
	    val = ajPhyloPropRead(acdReply, propchars, len, size);

	    if(!val)
	    {
		acdBadVal(thys, required,
			  "Unable to read properties from '%S'",
			  acdReply);
		ok = ajFalse;
	    }
	}
	else
	    if(!nullok)
	    {
		acdBadVal(thys, required,
			  "Input file is required");
		ok = ajFalse;
	    }
    }

    if(!ok)
	acdBadRetry(thys);
    acdInFileSave(acdReply, NULL, ajTrue);
    
    /* properties have special set attributes */
    
    thys->SAttr = acdAttrListCount(acdCalcProperties);
    thys->SetAttr = &acdCalcProperties[0];
    thys->SetStr = AJCALLOC0(thys->SAttr, sizeof(AjPStr));
    
    if(val)
    {
	ajStrFromInt(&thys->SetStr[0],val->Len); /* string length */
	ajStrFromInt(&thys->SetStr[1],val->Size); /* string count */
 	ajStrAssignS(&thys->ValStr, val->Str[0]);
    }
    else
    {
	ajStrFromInt(&thys->SetStr[0],0); /* string length */
	ajStrFromInt(&thys->SetStr[1],0); /* string count */
	ajStrAssignClear(&thys->ValStr);
    }

    thys->Value = val;
    ajStrDel(&propchars);

    return;
}




/* @func ajAcdGetRange ********************************************************
**
** Returns an item of type Range as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPRange] Range object.
** @cre failure to find an item with the right name and type aborts.
**
** @release 1.0.0
** @@
******************************************************************************/

AjPRange ajAcdGetRange(const char *token)
{
    return acdGetValueRef(token, "range");
}




/* @funcstatic acdSetRange ****************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD range item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdSetRange(AcdPAcd thys)
{
    AjPRange val = NULL;

    AjBool required = ajFalse;
    AjBool ok       = ajFalse;

    AjBool warnrange;
    AjBool failrange;

    ajint itry;

    ajuint imin;
    ajuint imax;
    ajuint isize;
    ajuint iminsize;
    AjPStr failmsg = NULL;

    acdRangeTestCalc(thys);

    acdAttrToUint(thys, "minimum", 1, &imin);
    acdLog("minimum: %d\n", imin);
    
    acdAttrToUint(thys, "maximum", UINT_MAX, &imax);
    acdLog("maximum: %d\n", imax);
    
    acdAttrToUint(thys, "minsize", 0, &iminsize);
    acdLog("minsize: %d\n", iminsize);
    
    acdAttrToUint(thys, "size", 0, &isize);
    acdLog("size: %d\n", isize);
    
    acdAttrToBool(thys, "failrange", ajTrue, &failrange);
    acdLog("failrange: %B\n", failrange);
    
    acdAttrToBool(thys, "warnrange", acdDoWarnRange, &warnrange);
    acdLog("warnrange: %B\n", warnrange);
    
    if(failrange && (imin > imax))
    {
        acdAttrResolve(thys, "rangemessage", &failmsg);
        if(ajStrGetLen(failmsg))
            acdErrorAcd(thys, "Invalid range: %S", failmsg);
        else
            acdErrorAcd(thys,
                        "Invalid range: "
                        "minimum value %d more than maximum %d",
                        imin, imax);
    }

    required = acdIsRequired(thys);
    acdReplyInitC(thys, "", &acdReplyDef);

    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */

	ajStrAssignS(&acdReply, acdReplyDef);

	if(required)
	    acdUserGet(thys, &acdReply);

	val = ajRangeNewStringLimits(acdReply, imin, imax, iminsize, isize);

	if(!val)
	{
	    acdBadVal(thys, required,
		      "Bad range specification '%S'", acdReply);
	    ok = ajFalse;
	}
    }

    if(!ok)
	acdBadRetry(thys);

    thys->Value = val;
    ajStrAssignS(&thys->ValStr, acdReply);

    return;
}




/* @func ajAcdGetRefseq *******************************************************
**
** Returns an item of type reference sequence as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPRefseq] Reference sequence object
** @cre failure to find an item with the right name and type aborts.
**
** @release 6.5.0
** @@
******************************************************************************/

AjPRefseq ajAcdGetRefseq(const char *token)
{
    AjPRefseq val = acdGetValueRef(token, "refseq");

    return val;
}




/* @funcstatic acdSetRefseq ***************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD reference sequence input
**
** Understands all attributes and associated qualifiers for this item type.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void acdSetRefseq(AcdPAcd thys)
{
    AjPRefseq val;
    AjPRefseqin refseqin;

    AjBool required = ajFalse;
    AjBool ok       = ajFalse;
    AjBool nullok   = ajFalse;
    ajint itry;

    AjPStr infname = NULL;
  
    val = ajRefseqNew();        /* set the default value */
    refseqin = ajRefseqinNew();  

    acdAttrToBool(thys, "nullok", ajFalse, &nullok);

    acdInFilename(&infname);
    required = acdIsRequired(thys);
    acdReplyInitS(thys, infname, &acdReplyDef);
    ajStrDel(&infname);

    acdAttrToBool(thys, "entry", ajFalse, &refseqin->Input->Text);

    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */

	ajStrAssignS(&acdReply, acdReplyDef);

	if(required)
	    acdUserGet(thys, &acdReply);

	if(!ajStrGetLen(acdReply) && nullok)
        {
            ajRefseqinDel(&refseqin);
	    break;
	}

	ajRefseqinQryS(refseqin, acdReply);

	acdGetValueAssoc(thys, "iformat", &refseqin->Input->Formatstr);
	acdGetValueAssoc(thys, "iquery", &refseqin->Input->QryFields);
	acdQualToLong(thys, "ioffset", 0L, &refseqin->Input->Fpos,
                      &acdTmpStr);

        ok = ajRefseqinRead(refseqin, val);
    }

    if(!ok)
	acdBadRetry(thys);

    acdInFileSave(acdReply, ajRefseqGetId(val), ajTrue);

    thys->Value = val;
    ajStrAssignS(&thys->ValStr, acdReply);
    
    ajRefseqinDel(&refseqin);

    return;
}




/* @func ajAcdGetRegexp *******************************************************
**
** Returns an item of type Regexp as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPPatlistRegex] Compiled regular expression pattern.
**                            The original pattern string is available
**                            through a call to ajAcdGetValue
** @cre failure to find an item with the right name and type aborts.
**
** @release 1.0.0
** @@
******************************************************************************/

AjPPatlistRegex ajAcdGetRegexp(const char *token)
{
    return acdGetValueRef(token, "regexp");
}




/* @func ajAcdGetRegexpSingle *************************************************
**
** Returns an item of type Regexp as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPRegexp] Compiled regular expression.
**                     The original pattern string is available
**                     through a call to ajAcdGetValue
** @cre failure to find an item with the right name and type aborts.
**
** @release 4.0.0
** @@
******************************************************************************/

AjPRegexp ajAcdGetRegexpSingle(const char *token)
{
    AjPPatlistRegex val;
    AjPPatternRegex patreg = NULL;
    ajint i = 0;

    val = acdGetValueSingle(token, "regexp");

    while (ajPatlistRegexGetNext(val, &patreg))
	i++;

    if(i > 1)
	ajWarn("Single list value %s, but can choose %d values",
	       token, i);

    if(i < 1)
	ajWarn("Single list value %s, no value found: returning NULL value",
	       token);

    return ajPatternRegexGetCompiled(patreg);
}




/* @funcstatic acdSetRegexp ***************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD regular expression item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other is available) is NULL.
**
** Attributes for minimum and maximum length are applied with error
** messages if exceeded.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdSetRegexp(AcdPAcd thys)
{    
    AjPPatlistRegex val = NULL;
    
    AjBool required = ajFalse;
    AjBool ok       = ajFalse;

    AjBool upper;
    AjBool lower;
    ajint itry;
    
    ajint minlen;
    ajint maxlen;
    ajint maxsize;
    ajint len;
    AjPStr type = NULL;
    AjPStr patname = NULL;
    AjPStr fmt = NULL;
    ajint itype = 0;
    
    acdAttrToInt(thys, "minlength", 1, &minlen);
    
    acdAttrToInt(thys, "maxlength", INT_MAX, &maxlen);
    acdAttrToBool(thys, "upper", ajFalse, &upper);
    acdAttrToBool(thys, "lower", ajFalse, &lower);
    acdAttrToStr(thys, "type", "string", &type);
    acdAttrToInt(thys, "maxsize", INT_MAX, &maxsize);
    acdGetValueAssoc(thys, "pname", &patname);
    acdGetValueAssoc(thys, "pformat", &fmt);
    ajStrFmtLower(&type);
    itype = ajPatternRegexType(type);

    required = acdIsRequired(thys);
    acdReplyInitC(thys, "", &acdReplyDef);
    
    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */
	if(val)
	    ajPatlistRegexDel(&val);
	
	ajStrAssignS(&acdReply, acdReplyDef);
	
	if(required)
	    acdUserGet(thys, &acdReply);
	
	len = ajStrGetLen(acdReply);
	
	if(len < minlen)
	{
	    acdBadVal(thys, required,
		      "Too short (%S) - minimum length is %d characters",
		      thys->Name, minlen);
	    ok = ajFalse;
	}

	if(len > maxlen)
	{
	    acdBadVal(thys, required,
		      "Too long (%S) - maximum length is %d characters",
		      thys->Name,maxlen);
	    ok = ajFalse;
	}

/*
	if(upper)
	    ajStrFmtUpper(&acdReply);
	
	if(lower)
	    ajStrFmtLower(&acdReply);
*/
	if(ok)
	    val = ajPatlistRegexRead(acdReply, patname, fmt,
				     itype, upper, lower);

	if(ok && !val)
	{
	    acdBadVal(thys, required,
		      "Bad regular expression pattern:\n   '%S'",
		      acdReply);
	    ok = ajFalse;
	}	
    }

    if(!ok)
	acdBadRetry(thys);
    
    ajStrDel(&patname);
    ajStrDel(&fmt);
    ajStrDel(&type);

    /* regexps have special set attributes the same as strings */
    
    thys->SAttr = acdAttrListCount(acdCalcRegexp);
    thys->SetAttr = &acdCalcRegexp[0];
    thys->SetStr = AJCALLOC0(thys->SAttr, sizeof(AjPStr));
    
    ajStrFromInt(&thys->SetStr[0], ajStrGetLen(acdReply));

    thys->Value = val;
    ajStrAssignS(&thys->ValStr, acdReply);
    
    return;
}




/* @funcstatic acdSetRel ******************************************************
**
** Defines an ACD relation.
**
** Called when a "variable" type ACD item is checked. Should not be called
** for any other item.
**
** At present there is nothing to prompt for here, though there could
** be, for example, a report of what the program does which would appear
** before any user prompts.
**
** @param [u] thys [AcdPAcd] ACD for the application item.
** @return [void]
**
** @release 2.9.0
** @@
******************************************************************************/

static void acdSetRel(AcdPAcd thys)
{
    acdAttrToStr(thys, "relations", "", &thys->ValStr);
    
    return;
}




/* @func ajAcdGetReport *******************************************************
**
** Returns an item of type Report as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPReport] Report output object. Already opened
**                      by ajReportOpen so this just returns the object
** @cre failure to find an item with the right name and type aborts.
**
** @release 2.0.0
** @@
******************************************************************************/

AjPReport ajAcdGetReport(const char *token)
{
    return acdGetValueRef(token, "report");
}




/* @funcstatic acdSetReport ***************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD report item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other available) is a null string, which
** is invalid.
**
** Associated qualifiers "-rformat", "-ropenfile"
** are applied to the URO before opening the output file.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajSeqRead
**
** @release 2.0.0
** @@
******************************************************************************/

static void acdSetReport(AcdPAcd thys)
{
    AjPReport val = NULL;
    
    AjBool required = ajFalse;
    AjBool ok       = ajFalse;

    ajint itry;
    AjBool nullok;
    AjBool nulldefault;
    
    AjPStr name      = NULL;
    AjPStr ext       = NULL;
    AjPStr fmt       = NULL;
    AjPStr dir       = NULL;
    AjPStr taglist   = NULL;
    
    val = NULL;

    
    acdAttrToBool(thys, "nullok", ajFalse, &nullok);
    acdAttrToBool(thys, "nulldefault", ajFalse, &nulldefault);

    acdGetValueAssoc(thys, "rdirectory", &dir);
    acdGetValueAssoc(thys, "rextension", &ext);
    acdGetValueAssoc(thys, "rname", &name);
    acdGetValueAssoc(thys, "rformat", &fmt);
    acdOutDirectory(&dir);
    required = acdIsRequired(thys);

    if(nullok && nulldefault)
    {
	if (acdDefinedEmpty(thys))  /* user set to empty - make default name */
	    acdOutFilename(&acdReplyDef, name, ext);
	else				/* leave empty */
	    acdReplyInitC(thys, "", &acdReplyDef);
    }
    else
    {
	acdOutFilename(&acdTmpOutFName, name, ext);
	acdReplyInitS(thys, acdTmpOutFName, &acdReplyDef);
    }

    acdPromptReport(thys);
    
    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */

	ajStrAssignS(&acdReply, acdReplyDef);

	if(required)
	    acdUserGet(thys, &acdReply);

	if(ajStrGetLen(acdReply))
	{
	    val = ajReportNew();
	    ajStrAssignEmptyS(&val->Formatstr, fmt);
	    acdAttrToStr(thys, "type", "", &val->Type);
	    acdAttrToStr(thys, "taglist", "", &taglist);
	    acdAttrToBool(thys, "multiple", ajFalse, &val->Multi);
	    acdAttrToInt(thys, "precision", 3, &val->Precision);
	    acdQualToBool(thys, "raccshow", ajFalse,
			  &val->Showacc, &acdReplyDef);
	    acdQualToBool(thys, "rdesshow", ajFalse,
			  &val->Showdes, &acdReplyDef);
	    acdQualToBool(thys, "rscoreshow", ajTrue,
			  &val->Showscore, &acdReplyDef);
	    acdQualToBool(thys, "rstrandshow", ajTrue,
			  &val->Showstrand, &acdReplyDef);
	    acdQualToBool(thys, "rusashow", ajFalse,
			  &val->Showusa, &acdReplyDef);
	    acdQualToInt(thys, "rmaxall", 0,
			  &val->MaxHitAll, &acdReplyDef);
	    acdQualToInt(thys, "rmaxseq", 0,
			  &val->MaxHitSeq, &acdReplyDef);
    
	    /* test acdc-reportbadtaglist */
	    if(!ajReportSetTagsS(val, taglist))
		acdErrorAcd(thys, "Bad tag list for report");

	    /* test acdc-reportbadtags */
	    if(!ajReportValid(val))
		ajDie("Report option -%S: Validation failed",
		      thys->Name);
    
	    ajStrAssignS(&acdOutFullFName, acdReply);
	    ajFilenameReplacePathS(&acdOutFullFName, dir);
	    ok = ajReportOpen(val, acdOutFullFName);

	    if(!ok)
	    {
		acdBadVal(thys, required,
			  "Unable to open report file '%S'",
			  acdOutFullFName);
		ajReportDel(&val);
	    }
	}
	else
	    if(!nullok)
	    {
		acdBadVal(thys, required,
			  "Report file is required");
		ok = ajFalse;
	    }
    }

    if(!ok)
	acdBadRetry(thys);
    
    thys->Value = val;
    ajStrAssignS(&thys->ValStr, acdOutFullFName);

    ajStrDel(&acdReply);
    ajStrDel(&acdReplyDef);

    ajStrDel(&name);
    ajStrDel(&ext);
    ajStrDel(&fmt);
    ajStrDel(&dir);
    ajStrDel(&taglist);

    return;
}




/* @func ajAcdGetResource *****************************************************
**
** Returns an item of type resource as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPResource] Data resource object
** @cre failure to find an item with the right name and type aborts.
**
** @release 6.4.0
** @@
******************************************************************************/

AjPResource ajAcdGetResource(const char *token)
{
    AjPResourceall val = acdGetValue(token, "resource");
    if(val->Multi)
        ajWarn("ajAcdGetResource request single resource but maxreads > 1");

    val->Returned = ajTrue;

    return val->Resource;
}




/* @func ajAcdGetResourceall **************************************************
**
** Returns an input stream of an item of type Resource as defined in a
** named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPResourceall] Data resource input stream
** @cre failure to find an item with the right name and type aborts.
**
** @release 6.4.0
** @@
******************************************************************************/

AjPResourceall ajAcdGetResourceall(const char *token)
{
    AjPResourceall val = acdGetValueRef(token, "resource");

    if(!val->Multi)
        ajWarn("ajAcdGetResourceall request data resource input stream "
               "but maxreads is 1");

    return val;
}




/* @funcstatic acdSetResource *************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD data resource input
**
** Understands all attributes and associated qualifiers for this item type.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void acdSetResource(AcdPAcd thys)
{
    AjPResourceall val;

    AjBool required = ajFalse;
    AjBool ok       = ajFalse;
    AjBool nullok   = ajFalse;
    ajint itry;

    ajint maxreads;

    AjPStr infname = NULL;
    
    val = ajResourceallNew();        /* set the default value */

    acdAttrToBool(thys, "nullok", ajFalse, &nullok);
    acdAttrToInt(thys, "maxreads", INT_MAX, &maxreads);

    acdAttrToBool(thys, "entry", ajFalse, &val->Resourcein->Input->Text);

    acdInFilename(&infname);
    required = acdIsRequired(thys);
    acdReplyInitS(thys, infname, &acdReplyDef);
    ajStrDel(&infname);

    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */

	ajStrAssignS(&acdReply, acdReplyDef);

	if(required)
	    acdUserGet(thys, &acdReply);

	if(!ajStrGetLen(acdReply) && nullok)
        {
            ajResourceallDel(&val);
	    break;
	}

	ajResourceinQryS(val->Resourcein, acdReply);

	acdGetValueAssoc(thys, "iformat", &val->Resourcein->Input->Formatstr);
	acdGetValueAssoc(thys, "iquery", &val->Resourcein->Input->QryFields);
	acdQualToLong(thys, "ioffset", 0L, &val->Resourcein->Input->Fpos,
                      &acdTmpStr);

        ok = ajResourceinRead(val->Resourcein, val->Resource);
    }

    if(!ok)
	acdBadRetry(thys);

    if(maxreads > 1)
        val->Multi = ajTrue;

    acdInFileSave(acdReply, ajResourceallGetresourceId(val), ajTrue);

    thys->Value = val;
    ajStrAssignS(&thys->ValStr, acdReply);
    
    return;
}




/* @func ajAcdGetScop *********************************************************
**
** Returns an item of type Scop as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPFile] Scop input file.
** @cre failure to find an item with the right name and type aborts.
**
** @release 1.8.0
** @@
******************************************************************************/

AjPFile ajAcdGetScop(const char *token)
{
    return acdGetValueRef(token, "scop");
}




/* @funcstatic acdSetScop *****************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD clean pdb file item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
**
** @release 1.8.0
** @@
******************************************************************************/

static void acdSetScop(AcdPAcd thys)
{
    AjPFile val;

    AjPStr name     = NULL;
    AjBool required = ajFalse;
    AjBool ok       = ajFalse;

    ajint itry;
    AjPStr fmt    = NULL;

    val = NULL;				/* set the default value */

    acdAttrResolve(thys, "name", &name);

    if (!acdGetValueAssoc(thys, "format", &fmt))
	ajStrAssignClear(&fmt);

    required = acdIsRequired(thys);
    acdReplyInitS(thys, name, &acdReplyDef);
    acdPromptScop(thys);

    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */

	ajStrAssignS(&acdReply, acdReplyDef);

	if(required)
	    acdUserGet(thys, &acdReply);

	if(ajStrGetLen(acdReply))
	{
	    val = ajFileNewInNameS(acdReply);

	    if(!val)
	    {
		acdBadVal(thys, required,
			  "Unable to read scop data '%S'", acdReply);
		ok = ajFalse;
	    }
	}
	else
	{
	    acdBadVal(thys, required, "Scop data file is required");
	    ok = ajFalse;
	}
    }

    if(!ok)
	acdBadRetry(thys);

    thys->Value = val;
    ajStrAssignS(&thys->ValStr, acdReply);

    ajStrDel(&name);
    ajStrDel(&fmt);

    return;
}




/* @func ajAcdGetSelect *******************************************************
**
** Returns an item of type Select as defined in a named ACD item,
** which is an array of strings terminated by a null value.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPStr*] String array of values with NULL as last element.
** @cre failure to find an item with the right name and type aborts.
**
** @release 1.0.0
** @@
******************************************************************************/

AjPStr* ajAcdGetSelect(const char *token)
{
    return acdGetValueRef(token, "selection");
}




/* @func ajAcdGetSelectSingle *************************************************
**
** Returns one item from an array of type Select as defined in a named
** ACD item, which is an array of strings terminated by a null value.
** Called by the application after all ACD values have been set, and
** simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPStr] String array of values with NULL as last element.
** @cre failure to find an item with the right name and type aborts.
**
** @release 4.0.0
** @@
******************************************************************************/

AjPStr ajAcdGetSelectSingle(const char *token)
{
    AjPStr* val;
    ajint i;

    val =  acdGetValueSingle(token, "select");

    for(i=0; val[i]; i++)
	continue;

    if(i > 1)
	ajWarn("Single list value %s, but can choose %d values",
	       token, i);

    if(i < 1)
	ajWarn("Single list value %s, no value found: returning NULL value",
	       token);

    return val[0];
}




/* @funcstatic acdSetSelect ***************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD select menu item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajFileNewOut
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdSetSelect(AcdPAcd thys)
{
    AjPStr* val;

    AjBool required = ajFalse;
    AjBool ok       = ajFalse;

    ajint itry;
    ajint i;

    ajint min=0, max=5;

    val = NULL;				/* set the default value */

    required = acdIsRequired(thys);
    acdReplyInitC(thys, "", &acdReplyDef);

    acdAttrToInt(thys, "minimum", 1, &min);
    acdAttrToInt(thys, "maximum", 1, &max);

    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */

	ajStrAssignS(&acdReply, acdReplyDef);

	if(required)
	{
	    acdSelectPrompt(thys);
	    acdUserGet(thys, &acdReply);
	}

	val = acdSelectValue(thys, min, max, acdReply);

	if(!val)
	{
	    acdBadVal(thys, required, "Bad select option '%S'", acdReply);
	    ok = ajFalse;
	}
    }

    if(!ok)
	acdBadRetry(thys);

    thys->Value = val;
    for(i=0; val[i]; i++)
    {
	if(i)
	    ajStrAppendC(&thys->ValStr, ";");

	ajStrAppendS(&thys->ValStr, val[i]);
    }

    return;
}




/* @func ajAcdGetSeq **********************************************************
**
** Returns an item of type Seq as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPSeq] Sequence object. The sequence was already loaded by
**         acdSetSeq so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
**
** @release 1.0.0
** @@
******************************************************************************/

AjPSeq ajAcdGetSeq(const char *token)
{
    return acdGetValueRef(token, "sequence");
}




/* @funcstatic acdSetSeq ******************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD sequence item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other available) is a null string, which
** is invalid.
**
** Associated qualifiers "-sformat", "-sdbname", "-sopenfile", "-sid"
** are applied to the USA before reading the sequence.
**
** Associated qualifiers "-supper", "-slower" and "-sask" are applied
** after reading.
**
** Associated qualifiers "-sbegin", "-send" and "-sreverse"
** are applied as appropriate, with prompting for values,
** after the sequence has been read. They are applied to the sequence,
** and the resulting sequence is what is set in the ACD item.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajSeqRead
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdSetSeq(AcdPAcd thys)
{
    AjPSeq val = NULL;
    AjPSeqin seqin;
    
    AjBool required = ajFalse;
    AjBool ok       = ajFalse;
    AjBool okbeg    = ajFalse;
    AjBool okend    = ajFalse;
    AjBool okrev    = ajFalse;

    ajint itry;
    ajint i;
    
    AjPStr infname = NULL;

    ajint sqbegin = 0;
    ajint sqend   = 0;

    AjBool sreverse = ajFalse;
    AjBool sprompt  = ajFalse;
    AjBool snuc     = ajFalse;
    AjBool sprot    = ajFalse;
    AjBool nullok   = ajFalse;
    AjPStr typestr  = NULL;
    
    
    val   = ajSeqNew();		        /* set the default value */
    seqin = ajSeqinNew();		/* set the default value */
    
    acdQualToBool(thys, "snucleotide", ajFalse, &snuc, &acdReplyDef);
    acdQualToBool(thys, "sprotein", ajFalse, &sprot, &acdReplyDef);
    acdAttrToBool(thys, "nullok", ajFalse, &nullok);
    acdAttrToStr(thys, "type", "", &typestr);
    
    acdInFilename(&infname);
    required = acdIsRequired(thys);
    acdReplyInitS(thys, infname, &acdReplyDef);
    acdPromptSeq(thys);
    ajStrDel(&infname);
 
    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */
	
	ajStrAssignS(&acdReply, acdReplyDef);
	
	if(required)
	    acdUserGet(thys, &acdReply);
	
	if(!ajStrGetLen(acdReply) && nullok)
        {
            ajSeqDel(&val);
	    break;
	}

	ajSeqinUsa(&seqin, acdReply);
	
	if(ajStrGetLen(typestr))
	{
	   ajStrAssignS(&seqin->Inputtype, typestr);
	   acdInTypeSeqSave(seqin->Inputtype);
        }
	else
	    acdInTypeSeqSave(NULL);
	
	acdAttrToBool(thys, "features", ajFalse, &seqin->Features);
	acdAttrToBool(thys, "entry", ajFalse, &seqin->Input->Text);
	
	acdGetValueAssoc(thys, "sformat", &seqin->Input->Formatstr);
	acdGetValueAssoc(thys, "sdbname", &seqin->Input->Db);
/*	acdGetValueAssoc(thys, "sopenfile", &seqin->Filename);*/ /* obsolete */
	acdGetValueAssoc(thys, "sid", &seqin->Entryname);
	acdGetValueAssoc(thys, "iquery", &seqin->Input->QryFields);
	acdQualToLong(thys, "ioffset", 0L, &seqin->Input->Fpos,
                      &acdTmpStr);
	
	acdGetValueAssoc(thys, "ufo", &seqin->Ufo);
	
	acdGetValueAssoc(thys, "fformat", &seqin->Ftquery->Formatstr);
	acdGetValueAssoc(thys, "fopenfile", &seqin->Ftquery->Filename);
	
	acdQualToBool(thys, "squick", ajFalse,
		      &seqin->Minimal, &acdTmpStr);
	acdQualToBool(thys, "scircular", ajFalse,
		      &seqin->Circular, &acdTmpStr);
	acdQualToBool(thys, "supper", ajFalse,
		      &seqin->Upper, &acdTmpStr);
	acdQualToBool(thys, "slower", ajFalse,
		      &seqin->Lower, &acdTmpStr);
	okbeg = acdQualToSeqbegin(thys, "sbegin", 0,
				  &sqbegin, &acdTmpStr);
	okend = acdQualToSeqend(thys, "send", 0,
				&sqend, &acdTmpStr);
	okrev = acdQualToBool(thys, "sreverse", ajFalse,
			      &sreverse, &acdTmpStr);
	
	if(snuc)
	    ajSeqinSetNuc(seqin);

	if(sprot)
	    ajSeqinSetProt(seqin);
	
	i = ajStrGetLen(seqin->Ufo) + ajStrGetLen(seqin->Ftquery->Formatstr)
	    + ajStrGetLen(seqin->Ftquery->Filename);
	
	if(i && !seqin->Features)
	    ajWarn("Feature table ignored");
	
	if(seqin->Features)
	    acdLog("acdSetSeq with features UFO '%S'\n", seqin->Ufo);
	
      	/* (try to) read the sequence */
	
	ok = ajSeqRead(val, seqin);

	if(!ok)
	{
	    acdBadVal(thys, required,
		      "Unable to read sequence '%S'", acdReply);
	}
    }
    
    if(!ok)
	acdBadRetry(thys);

    acdInFileSave(acdReply, ajSeqGetNameS(val),
                  ajTrue);	/* save sequence name */
    
    /* some standard options using associated qualifiers */
    
    acdQualToBool(thys, "sask", ajFalse, &sprompt, &acdReplyDef);
    
    if(val) 
    {

        /* now process the begin, end and reverse options */
    
        if(seqin->Begin)
            okbeg = ajTrue;
    
        for(itry=acdPromptTry; itry && !okbeg; itry--)
        {
            ajStrAssignC(&acdReplyPrompt, "start");

            if(sprompt)
                acdUserGetPrompt(thys, "sbegin",
                                 " Begin at position", &acdReplyPrompt);
            if(ajStrMatchCaseC(acdReplyPrompt, "start"))
                ajStrAssignC(&acdReplyPrompt, "0");

            okbeg = ajStrToInt(acdReplyPrompt, &sqbegin);

            if(!okbeg)
                acdBadVal(thys, sprompt,
                          "Invalid sequence position '%S'", acdReplyPrompt);
        }

        if(!okbeg)
            acdBadRetry(thys);
    
        if(sqbegin)
        {
            seqin->Begin = sqbegin;
            val->Begin = sqbegin;
            acdSetQualDefInt(thys, "sbegin", sqbegin);
        }
    
        if(seqin->End)
            okend = ajTrue;
    
        for(itry=acdPromptTry; itry && !okend; itry--)
        {
            ajStrAssignC(&acdReplyPrompt, "end");

            if(sprompt)
                acdUserGetPrompt(thys, "send",
                                 "   End at position", &acdReplyPrompt);
            if(ajStrMatchCaseC(acdReplyPrompt, "end"))
                ajStrAssignC(&acdReplyPrompt, "0");

            okend = ajStrToInt(acdReplyPrompt, &sqend);

            if(!okend)
                acdBadVal(thys, sprompt,
                          "Invalid sequence position '%S'", acdReplyPrompt);
        }

        if(!okend)
            acdBadRetry(thys);
    
        if(sqend)
        {
            seqin->End = sqend;
            val->End = sqend;
            acdSetQualDefInt(thys, "send", sqend);
        }

        if(ajSeqIsNuc(val))
        {
            for(itry=acdPromptTry; itry && !okrev; itry--)
            {
                ajStrAssignC(&acdReplyPrompt, "N");

                if(sprompt)
                    acdUserGetPrompt(thys, "sreverse",
                                     "    Reverse strand", &acdReplyPrompt);

                okrev = ajStrToBool(acdReplyPrompt, &sreverse);

                if(!okrev)
                    acdBadVal(thys, sprompt,
                              "Invalid Y/N value '%S'", acdReplyPrompt);
            }

            if(!okrev)
                acdBadRetry(thys);

            if(sreverse)
            {
                seqin->Rev = sreverse;
                val->Rev = sreverse;
                acdSetQualDefBool(thys, "sreverse", sreverse);
            }
        }
    
        acdLog("sbegin: %d, send: %d, sreverse: %B\n",
               sqbegin, sqend, sreverse);
    
        if(val->Rev)
            ajSeqReverseDo(val);
    
    
        /* sequences have special set attributes */
    
        thys->SAttr = acdAttrListCount(acdCalcSeq);
        thys->SetAttr = &acdCalcSeq[0];
        thys->SetStr = AJCALLOC0(thys->SAttr, sizeof(AjPStr));
    
        ajStrFromInt(&thys->SetStr[ACD_SEQ_BEGIN], ajSeqGetBegin(val));
        ajStrFromInt(&thys->SetStr[ACD_SEQ_END], ajSeqGetEnd(val));
        ajStrFromInt(&thys->SetStr[ACD_SEQ_LENGTH], ajSeqGetLen(val));
        ajStrFromBool(&thys->SetStr[ACD_SEQ_NUCLEIC], ajSeqIsNuc(val));
        ajStrFromBool(&thys->SetStr[ACD_SEQ_PROTEIN], ajSeqIsProt(val));
        ajStrAssignS(&thys->SetStr[ACD_SEQ_NAME], val->Name);
        ajStrAssignS(&thys->SetStr[ACD_SEQ_USA], ajSeqGetUsaS(val));
    }
    else 
    {
       /* sequences have special set attributes */
    
        thys->SAttr = acdAttrListCount(acdCalcSeq);
        thys->SetAttr = &acdCalcSeq[0];
        thys->SetStr = AJCALLOC0(thys->SAttr, sizeof(AjPStr));
    
        ajStrFromInt(&thys->SetStr[ACD_SEQ_BEGIN], 0);
        ajStrFromInt(&thys->SetStr[ACD_SEQ_END], 0);
        ajStrFromInt(&thys->SetStr[ACD_SEQ_LENGTH], 0);
        ajStrFromBool(&thys->SetStr[ACD_SEQ_NUCLEIC], ajFalse);
        ajStrFromBool(&thys->SetStr[ACD_SEQ_PROTEIN], ajFalse);
        ajStrAssignC(&thys->SetStr[ACD_SEQ_NAME], "");
        ajStrAssignC(&thys->SetStr[ACD_SEQ_USA], "");
    }
    
    thys->Value = val;
    ajStrAssignS(&thys->ValStr, acdReply);

    ajSeqinDel(&seqin);
    ajStrDel(&typestr);

    return;
}




/* @func ajAcdGetSeqall *******************************************************
**
** Returns an item of type Seq as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPSeqall] Sequence stream object. The sequence was already
**         loaded by acdSetSeqall so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
**
** @release 1.0.0
** @@
******************************************************************************/

AjPSeqall ajAcdGetSeqall(const char *token)
{
    return acdGetValueRef(token, "seqall");
}




/* @funcstatic acdSetSeqall ***************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD sequence item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other available) is a null string, which
** is invalid.
**
** Associated qualifiers "-sformat", "-sdbname", "-sopenfile", "-sid"
** are applied to the USA before reading the sequence.
**
** Associated qualifier "-sask" is applied
** after reading.
**
** Associated qualifiers "-supper", "-slower" are applied
** globally
**
** Associated qualifiers "-sbegin", "-send" and "-sreverse"
** are applied as appropriate, with prompting for values,
** after the sequence has been read. They are applied to the sequence,
** and the resulting sequence is what is set in the ACD item.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajSeqRead
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdSetSeqall(AcdPAcd thys)
{
    AjPSeqall val = NULL;
    AjPSeqin seqin;
    AjPSeq seq;
    
    AjBool required = ajFalse;
    AjBool ok       = ajFalse;
    AjBool okbeg    = ajFalse;
    AjBool okend    = ajFalse;
    AjBool okrev    = ajFalse;

    ajint itry;
    AjBool nullok = ajFalse;
    
    AjPStr infname = NULL;

    ajint sqbegin = 0;
    ajint sqend   = 0;
    AjBool sreverse = ajFalse;
    AjBool sprompt  = ajFalse;
    AjBool snuc     = ajFalse;
    AjBool sprot    = ajFalse;
    AjPStr typestr  = NULL;
    
    val = ajSeqallNew();		/* set the default value */
    seqin = val->Seqin;
    seqin->Input->Multi = ajTrue;
    seq = val->Seq;
    
    acdQualToBool(thys, "snucleotide", ajFalse, &snuc, &acdReplyDef);
    acdQualToBool(thys, "sprotein", ajFalse, &sprot, &acdReplyDef);
    acdAttrToBool(thys, "nullok", ajFalse, &nullok);
    acdAttrToStr(thys, "type", "", &typestr);
    
    acdInFilename(&infname);
    required = acdIsRequired(thys);
    acdReplyInitS(thys, infname, &acdReplyDef);
    acdPromptSeq(thys);
    ajStrDel(&infname);

    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */
	
	ajStrAssignS(&acdReply, acdReplyDef);
	
	if(required)
	    acdUserGet(thys, &acdReply);
	
	if(!ajStrGetLen(acdReply) && nullok)
        {
            ajSeqallDel(&val);
	    break;
	}

	ajSeqinUsa(&seqin, acdReply);
	
	if(ajStrGetLen(typestr))
	{
	    ajStrAssignS(&seqin->Inputtype, typestr);
	    acdInTypeSeqSave(seqin->Inputtype);
	}
	else
	    acdInTypeSeqSave(NULL);
	
	acdAttrToBool(thys, "features", ajFalse, &seqin->Features);
	acdAttrToBool(thys, "entry", ajFalse, &seqin->Input->Text);
	
	acdGetValueAssoc(thys, "sformat", &seqin->Input->Formatstr);
	acdGetValueAssoc(thys, "sdbname", &seqin->Input->Db);
/*	acdGetValueAssoc(thys, "sopenfile", &seqin->Filename);*/ /* obsolete */
	acdGetValueAssoc(thys, "sid", &seqin->Entryname);
	acdGetValueAssoc(thys, "iquery", &seqin->Input->QryFields);
	acdQualToLong(thys, "ioffset", 0L, &seqin->Input->Fpos,
                      &acdTmpStr);
	
	acdGetValueAssoc(thys, "ufo", &seqin->Ufo);
	acdGetValueAssoc(thys, "fformat", &seqin->Ftquery->Formatstr);
	acdGetValueAssoc(thys, "fopenfile", &seqin->Ftquery->Filename);
	
	acdQualToBool(thys, "squick", ajFalse,
		      &seqin->Minimal, &acdTmpStr);
	acdQualToBool(thys, "scircular", ajFalse,
		      &seqin->Circular, &acdTmpStr);
	acdQualToBool(thys, "supper", ajFalse,
		      &seqin->Upper, &acdTmpStr);
	acdQualToBool(thys, "slower", ajFalse,
		      &seqin->Lower, &acdTmpStr);
	okbeg = acdQualToSeqbegin(thys, "sbegin", 0,
				  &seqin->Begin, &acdTmpStr);
	okend = acdQualToSeqend(thys, "send", 0,
				&seqin->End, &acdTmpStr);
	okrev = acdQualToBool(thys, "sreverse", ajFalse,
			      &sreverse, &acdTmpStr);

	if(snuc)
	    ajSeqinSetNuc(seqin);
	
	if(sprot)
	    ajSeqinSetProt(seqin);
	
	if(ajStrGetLen(seqin->Ufo))
	    seqin->Features = ajTrue;
	
	ok = ajSeqAllRead(seq, seqin);

        if(!ok)
	    acdBadVal(thys, required,
		      "Unable to read sequence '%S'", acdReply);
    }

    if(!ok)
	acdBadRetry(thys);
    
    acdInFileSave(acdReply, ajSeqallGetseqName(val), ajTrue);

    /*  commentedout__ajSeqinDel(&seqin);*/
        
    acdQualToBool(thys, "sask", ajFalse, &sprompt, &acdReplyDef);

    /* now process the begin, end and reverse options */

    if(val)
    {

        if(seqin->Begin)
        {
            okbeg = ajTrue;
            val->Begin = seq->Begin = seqin->Begin;
        }
    
        for(itry=acdPromptTry; itry && !okbeg; itry--)
        {
            ajStrAssignC(&acdReplyPrompt, "start");

            if(sprompt)
                acdUserGetPrompt(thys, "sbegin",
                                 " Begin at position", &acdReplyPrompt);
            if(ajStrMatchCaseC(acdReplyPrompt, "start"))
                ajStrAssignC(&acdReplyPrompt, "0");

            okbeg = ajStrToInt(acdReplyPrompt, &sqbegin);

            if(!okbeg)
                acdBadVal(thys, sprompt,
                          "Invalid integer value '%S'", acdReplyPrompt);
        }

        if(!okbeg)
            acdBadRetry(thys);
    
        if(sqbegin)
        {
            seqin->Begin = sqbegin;
            seq->Begin = sqbegin;
            val->Begin = sqbegin;
            acdSetQualDefInt(thys, "sbegin", sqbegin);
        }
    
        if(seqin->End)
        {
            okend = ajTrue;
            val->End = seq->End = seqin->End;
        }
    
        for(itry=acdPromptTry; itry && !okend; itry--)
        {
            ajStrAssignC(&acdReplyPrompt, "end");

            if(sprompt)
                acdUserGetPrompt(thys, "send",
                                 "   End at position", &acdReplyPrompt);
            if(ajStrMatchCaseC(acdReplyPrompt, "end"))
                ajStrAssignC(&acdReplyPrompt, "0");

            okend = ajStrToInt(acdReplyPrompt, &sqend);

            if(!okend)
                acdBadVal(thys, sprompt,
                          "Invalid integer value '%S'", acdReplyPrompt);
        }

        if(!okend)
            acdBadRetry(thys);
    
        if(sqend)
        {
            seqin->End = sqend;
            seq->End = sqend;
            val->End = sqend;
            acdSetQualDefInt(thys, "send", sqend);
        }

        if(ajSeqIsNuc(seq))
        {
            for(itry=acdPromptTry; itry && !okrev; itry--)
            {
                ajStrAssignC(&acdReplyPrompt, "N");

                if(sprompt)
                    acdUserGetPrompt(thys, "sreverse",
                                     "    Reverse strand", &acdReplyPrompt);
                okrev = ajStrToBool(acdReplyPrompt, &sreverse);

                if(!okrev)
                    acdBadVal(thys, sprompt,
                              "Invalid Y/N value '%S'", acdReplyPrompt);
            }

            if(!okrev)
                acdBadRetry(thys);

            if(sreverse)
            {
                seqin->Rev = sreverse;
                seq->Rev = sreverse;
                val->Rev = sreverse;
                acdSetQualDefBool(thys, "sreverse", sreverse);
            }
        }


        acdLog("sbegin: %d, send: %d, sreverse: %B\n",
               sqbegin, sqend, sreverse);
    
        /* sequences have special set attributes */
    
        thys->SAttr = acdAttrListCount(acdCalcSeqall);
        thys->SetAttr = &acdCalcSeqall[0];
        thys->SetStr = AJCALLOC0(thys->SAttr, sizeof(AjPStr));

        ajStrFromInt(&thys->SetStr[ACD_SEQ_BEGIN], ajSeqallGetseqBegin(val));
        ajStrFromInt(&thys->SetStr[ACD_SEQ_END], ajSeqallGetseqEnd(val));
        ajStrFromInt(&thys->SetStr[ACD_SEQ_LENGTH], ajSeqGetLen(seq));
        ajStrFromBool(&thys->SetStr[ACD_SEQ_NUCLEIC], ajSeqIsNuc(seq));
        ajStrFromBool(&thys->SetStr[ACD_SEQ_PROTEIN], ajSeqIsProt(seq));
        ajStrAssignS(&thys->SetStr[ACD_SEQ_NAME], seq->Name);
        ajStrAssignS(&thys->SetStr[ACD_SEQ_USA], ajSeqallGetUsa(val));
    }
    else 
    {
       /* sequences have special set attributes */
    
        thys->SAttr = acdAttrListCount(acdCalcSeqall);
        thys->SetAttr = &acdCalcSeq[0];
        thys->SetStr = AJCALLOC0(thys->SAttr, sizeof(AjPStr));
    
        ajStrFromInt(&thys->SetStr[ACD_SEQ_BEGIN], 0);
        ajStrFromInt(&thys->SetStr[ACD_SEQ_END], 0);
        ajStrFromInt(&thys->SetStr[ACD_SEQ_LENGTH], 0);
        ajStrFromBool(&thys->SetStr[ACD_SEQ_NUCLEIC], ajFalse);
        ajStrFromBool(&thys->SetStr[ACD_SEQ_PROTEIN], ajFalse);
        ajStrAssignC(&thys->SetStr[ACD_SEQ_NAME], "");
        ajStrAssignC(&thys->SetStr[ACD_SEQ_USA], "");
    }

    thys->Value = val;
    ajStrAssignS(&thys->ValStr, acdReply);

    ajStrDel(&typestr);

    return;
}




/* @funcstatic acdSetSeqsetall ************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD sequence item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other available) is a null string, which
** is invalid.
**
** Associated qualifiers "-sformat", "-sdbname", "-sopenfile", "-sid"
** are applied to the USA before reading the sequence.
**
** Associated qualifiers "-supper", "-slower" and "-sask" are applied
** after reading.
**
** Associated qualifiers "-sbegin", "-send" and "-sreverse"
** are applied as appropriate, with prompting for values,
** after the sequence has been read. They are applied to the sequence,
** and the resulting sequence is what is set in the ACD item.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajSeqRead
**
** @release 2.8.0
** @@
******************************************************************************/


static void acdSetSeqsetall(AcdPAcd thys)
{
    AjPSeqset *val = NULL;
    AjPSeqin seqin;
    
    AjBool required = ajFalse;
    AjBool ok       = ajFalse;
    AjBool okbeg    = ajFalse;
    AjBool okend    = ajFalse;
    AjBool okrev    = ajFalse;

    ajint itry;
    ajint iattr;
    
    AjPStr infname = NULL;

    ajint sqbegin = 0;
    ajint sqend   = 0;
    AjBool sreverse = ajFalse;
    AjBool sprompt  = ajFalse;
    AjBool snuc     = ajFalse;
    AjBool sprot    = ajFalse;
    AjBool nullok   = ajFalse;
    AjBool aligned  = ajFalse;
    AjPStr typestr  = NULL;

    void **sets     = NULL;
    AjPList seqlist;

    ajint iset = 0;
    ajint nsets;

    seqlist = ajListNew();
    seqin = ajSeqinNew();		/* set the default value */
    
    seqin->Input->Multi = ajTrue;
    
    acdQualToBool(thys, "snucleotide", ajFalse, &snuc, &acdReplyDef);
    acdQualToBool(thys, "sprotein", ajFalse, &sprot, &acdReplyDef);
    acdAttrToBool(thys, "nullok", ajFalse, &nullok);
    acdAttrToBool(thys, "aligned", ajFalse, &aligned);
    acdAttrToStr(thys, "type", "", &typestr);
    
    acdInFilename(&infname);
    required = acdIsRequired(thys);
    acdReplyInitS(thys, infname, &acdReplyDef);
    acdPromptSeq(thys);
    ajStrDel(&infname);

    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */
	
	ajStrAssignS(&acdReply, acdReplyDef);
	
	if(required)
	    acdUserGet(thys, &acdReply);
	
	if(!ajStrGetLen(acdReply) && nullok)
	    break;

	ajSeqinUsa(&seqin, acdReply);
	
	if(ajStrGetLen(typestr))
	{
	    ajStrAssignS(&seqin->Inputtype, typestr);
	    acdInTypeSeqSave(seqin->Inputtype);
	}
	else
	    acdInTypeSeqSave(NULL);
	
	acdAttrToBool(thys, "features", ajFalse, &seqin->Features);
	
	acdGetValueAssoc(thys, "sformat", &seqin->Input->Formatstr);
	acdGetValueAssoc(thys, "sdbname", &seqin->Input->Db);
/*	acdGetValueAssoc(thys, "sopenfile", &seqin->Filename);*/ /* obsolete */
	acdGetValueAssoc(thys, "sid", &seqin->Entryname);
	acdGetValueAssoc(thys, "iquery", &seqin->Input->QryFields);
	acdQualToLong(thys, "ioffset", 0L, &seqin->Input->Fpos,
                      &acdTmpStr);
	
	acdGetValueAssoc(thys, "ufo", &seqin->Ufo);
	acdGetValueAssoc(thys, "fformat", &seqin->Ftquery->Formatstr);
	acdGetValueAssoc(thys, "fopenfile", &seqin->Ftquery->Filename);
	
	acdQualToBool(thys, "squick", ajFalse,
		      &seqin->Minimal, &acdTmpStr);
	acdQualToBool(thys, "scircular", ajFalse,
		      &seqin->Circular, &acdTmpStr);
	acdQualToBool(thys, "supper", ajFalse,
		      &seqin->Upper, &acdTmpStr);
	acdQualToBool(thys, "slower", ajFalse,
		      &seqin->Lower, &acdTmpStr);
	okbeg = acdQualToSeqbegin(thys, "sbegin", 0,
				  &seqin->Begin, &acdTmpStr);
	okend = acdQualToSeqend(thys, "send", 0,
				&seqin->End, &acdTmpStr);
	okrev = acdQualToBool(thys, "sreverse", ajFalse,
			      &sreverse, &acdTmpStr);
	
	if(snuc)
	    ajSeqinSetNuc(seqin);
	
	if(sprot)
	    ajSeqinSetProt(seqin);
	
	if(ajStrGetLen(seqin->Ufo))
	    seqin->Features = ajTrue;
	
	ok = ajSeqsetallRead(seqlist, seqin);

	if(!ok)
	    acdBadVal(thys, required,
		      "Unable to read sequence '%S'", acdReply);
    }

    if(!ok)
	acdBadRetry(thys);

    nsets = (ajuint) ajListToarray(seqlist,(void***) &sets);

    if(!nsets)
    {
        acdInFileSave(acdReply, ajStrConstEmpty(),
                      ajTrue); 
       /* sequences have special set attributes */
    
        thys->SAttr = acdAttrListCount(acdCalcSeqsetall);
        thys->SetAttr = &acdCalcSeq[0];
        thys->SetStr = AJCALLOC0(thys->SAttr, sizeof(AjPStr));
    
        ajStrFromInt(&thys->SetStr[ACD_SEQ_BEGIN], 0);
        ajStrFromInt(&thys->SetStr[ACD_SEQ_END], 0);
        ajStrFromInt(&thys->SetStr[ACD_SEQ_LENGTH], 0);
        ajStrFromBool(&thys->SetStr[ACD_SEQ_NUCLEIC], ajFalse);
        ajStrFromBool(&thys->SetStr[ACD_SEQ_PROTEIN], ajFalse);
        ajStrAssignC(&thys->SetStr[ACD_SEQ_NAME], "");
        ajStrAssignC(&thys->SetStr[ACD_SEQ_USA], "");
        ajStrFromFloat(&thys->SetStr[ACD_SEQ_WEIGHT],
                       0.0, 3);
        ajStrFromInt(&thys->SetStr[ACD_SEQ_COUNT], 0);
        ajStrFromInt(&thys->SetStr[ACD_SEQ_MULTICOUNT], 0);
    }
    else
    {
        val   = (AjPSeqset*) sets;
        ajListFree(&seqlist);

        acdInFileSave(acdReply, ajSeqsetGetNameS(val[0]),
                      ajTrue);      /* save sequence name */
    
        acdQualToBool(thys, "sask", ajFalse, &sprompt, &acdReplyDef);
    
        /* now process the begin, end and reverse options */
    
        if(seqin->Begin)
        {
            okbeg = ajTrue;

            for(iset=0;iset<nsets;iset++)
                val[iset]->Begin = seqin->Begin;
        }
    
        for(itry=acdPromptTry; itry && !okbeg; itry--)
        {
            ajStrAssignC(&acdReplyPrompt, "start");

            if(sprompt)
                acdUserGetPrompt(thys, "sbegin",
                                 " Begin at position", &acdReplyPrompt);
            if(ajStrMatchCaseC(acdReplyPrompt, "start"))
                ajStrAssignC(&acdReplyPrompt, "0");

            okbeg = ajStrToInt(acdReplyPrompt, &sqbegin);

            if(!okbeg)
                acdBadVal(thys, sprompt,
                          "Invalid integer value '%S'", acdReplyPrompt);
        }

        if(!okbeg)
            acdBadRetry(thys);
    
        if(sqbegin)
        {
            seqin->Begin = sqbegin;

            for(iset=0;iset<nsets;iset++)
                val[iset]->Begin = sqbegin;

            acdSetQualDefInt(thys, "sbegin", sqbegin);
        }
    
        if(seqin->End)
        {
            okend = ajTrue;

            for(iset=0;iset<nsets;iset++)
                val[iset]->End = seqin->End;
        }
    
        for(itry=acdPromptTry; itry && !okend; itry--)
        {
            ajStrAssignC(&acdReplyPrompt, "end");

            if(sprompt)
                acdUserGetPrompt(thys, "send",
                                 "   End at position", &acdReplyPrompt);
            if(ajStrMatchCaseC(acdReplyPrompt, "end"))
                ajStrAssignC(&acdReplyPrompt, "0");

            okend = ajStrToInt(acdReplyPrompt, &sqend);

            if(!okend)
                acdBadVal(thys, sprompt,
                          "Invalid integer value '%S'", acdReplyPrompt);
        }

        if(!okend)
            acdBadRetry(thys);
    
        if(sqend)
        {
            seqin->End = sqend;

            for(iset=0;iset<nsets;iset++)
                val[iset]->End = sqend;

            acdSetQualDefInt(thys, "send", sqend);
        }

        if(ajSeqsetIsNuc(val[0]))
        {
            for(itry=acdPromptTry; itry && !okrev; itry--)
            {
                ajStrAssignC(&acdReplyPrompt, "N");

                if(sprompt)
                    acdUserGetPrompt(thys, "sreverse",
                                     "    Reverse strand", &acdReplyPrompt);

                okrev = ajStrToBool(acdReplyPrompt, &sreverse);

                if(!okrev)
                    acdBadVal(thys, sprompt,
                              "Invalid Y/N value '%S'", acdReplyPrompt);
            }

            if(!okrev)
                acdBadRetry(thys);

            if(sreverse)
            {
                seqin->Rev = sreverse;

                for(iset=0;iset<nsets;iset++)
                    val[iset]->Rev = sreverse;

                acdSetQualDefBool(thys, "sreverse", sreverse);
            }
        }
    
        acdLog("sbegin: %d, send: %d, sreverse: Bs\n",
               sqbegin, sqend, sreverse);
    
        if(aligned)
            for(iset=0;iset<nsets;iset++)
                ajSeqsetFill(val[iset]);

        for(iset=0;iset<nsets;iset++)
            if(val[iset]->Rev)
                ajSeqsetReverse(val[iset]);
    
    
        /* sequences have special set attributes */

        thys->SAttr = acdAttrListCount(acdCalcSeqsetall);
        thys->SetAttr = &acdCalcSeqsetall[0];
        thys->SetStr = AJCALLOC0(thys->SAttr, sizeof(AjPStr));
    
        ajStrFromInt(&thys->SetStr[ACD_SEQ_BEGIN], ajSeqsetGetBegin(val[0]));
        ajStrFromInt(&thys->SetStr[ACD_SEQ_END], ajSeqsetGetEnd(val[0]));
        ajStrFromInt(&thys->SetStr[ACD_SEQ_LENGTH], ajSeqsetGetLen(val[0]));
        ajStrFromBool(&thys->SetStr[ACD_SEQ_PROTEIN],
                      ajSeqsetIsProt(val[0]));
        ajStrFromBool(&thys->SetStr[ACD_SEQ_NUCLEIC],
                      ajSeqsetIsNuc(val[0]));
        ajStrAssignS(&thys->SetStr[ACD_SEQ_NAME], val[0]->Name);
        ajStrAssignS(&thys->SetStr[ACD_SEQ_USA], ajSeqsetGetUsa(val[0]));
        ajStrFromFloat(&thys->SetStr[ACD_SEQ_WEIGHT],
                       ajSeqsetGetTotweight(val[0]), 3);
        ajStrFromInt(&thys->SetStr[ACD_SEQ_COUNT], ajSeqsetGetSize(val[0]));
        ajStrFromInt(&thys->SetStr[ACD_SEQ_MULTICOUNT], nsets);
    
        acdInFileSave(acdReply, ajSeqsetGetNameS(val[0]), ajTrue);

        for(iattr=0; iattr < thys->SAttr; iattr++)
            ajDebug("CalcAttr %s: '%S'\n",
                    acdCalcSeqset[iattr].Name, thys->SetStr[iattr]);
    }

    thys->Value = val;
    ajStrAssignS(&thys->ValStr, acdReply);

    ajSeqinDel(&seqin);
    ajStrDel(&typestr);

    return;
}




/* @func ajAcdGetSeqout *******************************************************
**
** Returns an item of type Seqout as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPSeqout] Sequence output object. The file was already opened by
**         acdSetSeqout so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
**
** @release 1.0.0
** @@
******************************************************************************/

AjPSeqout ajAcdGetSeqout(const char *token)
{
    return acdGetValueRef(token, "seqout");
}




/* @funcstatic acdSetSeqout ***************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD sequence output item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other is available) is "stdout".
**
** Associated qualifier "-osformat"
** is applied to the USA before opening the output file.
**
** Associated qualifiers are defined but not yet implemented.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajFileNewOut
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdSetSeqout(AcdPAcd thys)
{
    AjPSeqout val = NULL;
    
    AjBool required = ajFalse;
    AjBool ok       = ajFalse;

    ajint itry;
    AjBool nullok;
    AjBool nulldefault;
    
    AjPStr name     = NULL;
    AjPStr ext      = NULL;
    AjPStr fmt      = NULL;
    AjPStr typestr  = NULL;
    AjBool osfeat;

    val = NULL;

    if(!acdGetValueAssoc(thys, "osname", &name))
	acdAttrResolve(thys, "name", &name);
    
    if(!acdGetValueAssoc(thys, "osextension", &ext))
	acdAttrResolve(thys, "extension", &ext);
    
    acdGetValueAssoc(thys, "osformat", &fmt);
    ajStrAssignEmptyS(&ext, fmt);

    if(!ajStrGetLen(ext))
	ajSeqoutstrGetFormatDefault(&ext);
    
    acdAttrToBool(thys, "features", ajFalse, &osfeat);
    
    acdAttrToBool(thys, "nullok", ajFalse, &nullok);
    acdAttrToBool(thys, "nulldefault", ajFalse, &nulldefault);

    if(!acdAttrToStr(thys, "type", "", &typestr))
	acdInTypeSeq(&typestr);

    required = acdIsRequired(thys);
    if(nullok && nulldefault)
    {
	if (acdDefinedEmpty(thys))  /* user set to empty - make default name */
	    acdOutFilename(&acdReplyDef, name, ext);
	else				/* leave empty */
	    acdReplyInitC(thys, "", &acdReplyDef);
    }
    else
    {
	acdOutFilename(&acdTmpOutFName, name, ext);
	acdReplyInitS(thys, acdTmpOutFName, &acdReplyDef);
    }

    ajStrDel(&name);

    acdPromptSeqout(thys);
    
    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */
	
	ajStrAssignS(&acdReply, acdReplyDef);
	
	if(required)
	    acdUserGet(thys, &acdReply);
	
        if(ajStrGetLen(acdReply))
	{
	    val = ajSeqoutNew();	/* set the default value */
    
	    ajSeqoutClearUsa(val, acdReply);	/* resets the AjPSeqout */
	    val->Features = osfeat;
	    acdGetValueAssoc(thys, "osdirectory", &val->Directory);
	    acdGetValueAssoc(thys, "osdbname", &val->Setoutdb);
    
	    acdOutDirectory(&val->Directory);
	    acdLog("acdSetSeqout features: %B dir '%S'\n",
		   val->Features, val->Directory);

	    ajStrAssignEmptyS(&val->Formatstr, fmt);

	    if(!ajStrGetLen(val->Formatstr))
		ajSeqoutstrGetFormatDefault(&val->Formatstr);

	    ajStrAssignEmptyS(&val->Extension, ext);
	    ajStrAssignEmptyS(&val->Extension, val->Formatstr);

	    acdGetValueAssoc(thys, "oufo", &val->Ufo);
	    acdGetValueAssoc(thys, "offormat", &val->Ftquery->Formatstr);

	    if(!ajStrGetLen(val->Ftquery->Formatstr))
		ajFeatOutFormatDefault(&val->Ftquery->Formatstr);

	    acdGetValueAssoc(thys, "ofname", &val->Ftquery->Filename);
	    acdGetValueAssoc(thys, "ofdirectory",
			     &val->Ftquery->Directory);
	    acdOutDirectory(&val->Ftquery->Directory);

	    acdQualToBool(thys, "ossingle",
			  ajFalse,
			  &val->Single, &acdTmpStr);

	    if(ajStrGetLen(typestr))
		ajStrAssignS(&val->Outputtype, typestr);
	    else
	    {
		if(!acdInTypeSeq(&val->Outputtype))
		    ajWarn("No output type specified for '%S'", thys->Name);
	    }

	    if(!ajSeqoutOpen(val))
	    {
		if(ajStrGetLen(val->Directory))
		    acdBadVal(thys, required,
			      "Unable to write sequence to '%S%S'",
			      val->Directory, acdReply);
		else
		    acdBadVal(thys, required,
			      "Unable to write sequence to '%S'",
			      acdReply);
		ok = ajFalse;
		ajSeqoutDel(&val);
	    }
	}
	else
	    if(!nullok)
	    {
		acdBadVal(thys, required,
			  "Output USA is required");
		ok = ajFalse;
	    }
    }

    if(!ok)
	acdBadRetry(thys);
    
    thys->Value = val;
    ajStrAssignS(&thys->ValStr, acdReply);

    if (val)
    {
	acdLog("acdSetSeqout features: %B\n", val->Features);
	if(val->Features)
	    acdLog("acdSetSeqout with features UFO '%S'\n", val->Ufo);
    }

    ajStrDel(&typestr);
    ajStrDel(&fmt);
    ajStrDel(&ext);

    return;
}




/* @func ajAcdGetSeqoutall ****************************************************
**
** Returns an item of type Seqoutall as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPSeqout] Sequence output object. The file was already
**         opened by acdSetSeqoutall so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
**
** @release 1.0.0
** @@
******************************************************************************/

AjPSeqout ajAcdGetSeqoutall(const char *token)
{
    return acdGetValueRef(token, "seqoutall");
}




/* @funcstatic acdSetSeqoutall ************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD sequence output item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other is available) is "stdout".
**
** Associated qualifier "-osformat"
** is applied to the USA before opening the output file.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajFileNewOut
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdSetSeqoutall(AcdPAcd thys)
{
    AjPSeqout val = NULL;
    
    AjBool required = ajFalse;
    AjBool ok       = ajFalse;

    ajint itry;
    AjBool nullok;
    AjBool nulldefault;
    
    AjPStr name     = NULL;
    AjPStr ext      = NULL;
    AjPStr fmt      = NULL;
    AjPStr typestr  = NULL;
    AjBool osfeat;
    
    val = NULL;

    if(!acdGetValueAssoc(thys, "osname", &name))
	acdAttrResolve(thys, "name", &name);
    
    if(!acdGetValueAssoc(thys, "osextension", &ext))
	acdAttrResolve(thys, "extension", &ext);
    
    acdGetValueAssoc(thys, "osformat", &fmt);
    ajStrAssignEmptyS(&ext, fmt);

    if(!ajStrGetLen(ext))
	ajSeqoutstrGetFormatDefault(&ext);
    
    
    acdAttrToBool(thys, "features", ajFalse, &osfeat);
    
    acdAttrToBool(thys, "nullok", ajFalse, &nullok);
    acdAttrToBool(thys, "nulldefault", ajFalse, &nulldefault);

    if(!acdAttrToStr(thys, "type", "", &typestr))
	acdInTypeSeq(&typestr);

    required = acdIsRequired(thys);

    if(nullok && nulldefault)
    {
	if (acdDefinedEmpty(thys))  /* user set to empty - make default name */
	    acdOutFilename(&acdReplyDef, name, ext);
	else				/* leave empty */
	    acdReplyInitC(thys, "", &acdReplyDef);
    }
    else
    {
	acdOutFilename(&acdTmpOutFName, name, ext);
        acdReplyInitS(thys, acdTmpOutFName, &acdReplyDef);
    }
    ajStrDel(&name);
    acdPromptSeqout(thys);
    
    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */
	
	ajStrAssignS(&acdReply, acdReplyDef);
	
	if(required)
	    acdUserGet(thys, &acdReply);
	
	if(ajStrGetLen(acdReply))
	{
	    val = ajSeqoutNew();		/* set the default value */
	    ajSeqoutClearUsa(val, acdReply);
	    val->Features = osfeat;
	    acdGetValueAssoc(thys, "osdirectory", &val->Directory);
	    acdGetValueAssoc(thys, "osdbname", &val->Setoutdb);
	    acdOutDirectory(&val->Directory);
	    acdLog("acdSetSeqoutall features: %B\n", val->Features);

	    ajStrAssignEmptyS(&val->Formatstr, fmt);

	    if(!ajStrGetLen(val->Formatstr))
		ajSeqoutstrGetFormatDefault(&val->Formatstr);

	    ajStrAssignEmptyS(&val->Extension, ext);
	    ajStrAssignEmptyS(&val->Extension, val->Formatstr);

	    acdGetValueAssoc(thys, "oufo", &val->Ufo);
	    acdGetValueAssoc(thys, "offormat", &val->Ftquery->Formatstr);

	    if(!ajStrGetLen(val->Ftquery->Formatstr))
		ajFeatOutFormatDefault(&val->Ftquery->Formatstr);

	    acdGetValueAssoc(thys, "ofname", &val->Ftquery->Filename);
	    acdGetValueAssoc(thys, "ofdirectory",
			     &val->Ftquery->Directory);
	    acdOutDirectory(&val->Ftquery->Directory);

	    acdLog("acdSetSeqoutall ossingle default: %B\n",
		   ajSeqoutstrIsFormatSingle(val->Formatstr));

	    acdQualToBool(thys, "ossingle",
			  ajSeqoutstrIsFormatSingle(val->Formatstr),
			  &val->Single, &acdTmpStr);
	    acdLog("acdSetSeqoutall ossingle value %B '%S'\n",
		   val->Single, acdTmpStr);

	    if(ajStrGetLen(typestr))
		ajStrAssignS(&val->Outputtype, typestr);
	    else
	    {
		if(!acdInTypeSeq(&val->Outputtype))
		    ajWarn("No output type specified for '%S'", thys->Name);
	    }

	    if(!ajSeqoutOpen(val))
	    {
		if(ajStrGetLen(val->Directory))
		    acdBadVal(thys, required,
			      "Unable to write sequence to '%S%S'",
			      val->Directory, acdReply);
		else
		    acdBadVal(thys, required,
			      "Unable to write sequence to '%S'",
			      acdReply);
		ok = ajFalse;
		ajSeqoutDel(&val);
	    }
	}
	else
	    if(!nullok)
	    {
		acdBadVal(thys, required,
			  "Output USA is required");
		ok = ajFalse;
	    }
    }

    if(!ok)
	acdBadRetry(thys);
    
    thys->Value = val;
    ajStrAssignS(&thys->ValStr, acdReply);
    
    ajStrDel(&typestr);
    ajStrDel(&fmt);
    ajStrDel(&ext);

    return;
}




/* @func ajAcdGetSeqoutset ****************************************************
**
** Returns an item of type Seqoutset as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPSeqout] Sequence output object. The file was already
**        opened by acdSetSeqoutset so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
**
** @release 1.0.0
** @@
******************************************************************************/

AjPSeqout ajAcdGetSeqoutset(const char *token)
{
    return acdGetValueRef(token, "seqoutset");
}




/* @funcstatic acdSetSeqoutset ************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD sequence output item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other is available) is "stdout".
**
** Associated qualifier "-osformat"
** is applied to the USA before opening the output file.
**
** Associated qualifiers are defined but not yet implemented.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajFileNewOut
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdSetSeqoutset(AcdPAcd thys)
{
    AjPSeqout val = NULL;
    
    AjBool required = ajFalse;
    AjBool ok       = ajFalse;
    AjBool nullok;
    AjBool nulldefault;

    ajint itry;
    
    AjPStr name     = NULL;
    AjPStr ext      = NULL;
    AjPStr fmt      = NULL;
    AjPStr typestr  = NULL;
    AjBool osfeat;
    
    val = NULL;

    if(!acdGetValueAssoc(thys, "osname", &name))
	acdAttrResolve(thys, "name", &name);
    
    if(!acdGetValueAssoc(thys, "osextension", &ext))
	acdAttrResolve(thys, "extension", &ext);
    
    acdGetValueAssoc(thys, "osformat", &fmt);
    ajStrAssignEmptyS(&ext, fmt);

    if(!ajStrGetLen(ext))
	ajSeqoutstrGetFormatDefault(&ext);
    
    acdAttrToBool(thys, "features", ajFalse, &osfeat);
    
    required = acdIsRequired(thys);
    acdAttrToBool(thys, "nullok", ajFalse, &nullok);
    acdAttrToBool(thys, "nulldefault", ajFalse, &nulldefault);

    if(!acdAttrToStr(thys, "type", "", &typestr))
	acdInTypeSeq(&typestr);
    
    if(nullok && nulldefault)
    {
	if (acdDefinedEmpty(thys))  /* user set to empty - make default name */
	    acdOutFilename(&acdReplyDef, name, ext);
	else				/* leave empty */
	    acdReplyInitC(thys, "", &acdReplyDef);
    }
    else
    {
	acdOutFilename(&acdTmpOutFName, name, ext);
	acdReplyInitS(thys, acdTmpOutFName, &acdReplyDef);
    }
    ajStrDel(&name);

    acdPromptSeqout(thys);
    
    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */
	
	ajStrAssignS(&acdReply, acdReplyDef);
	
	if(required)
	    acdUserGet(thys, &acdReply);
	
	if(ajStrGetLen(acdReply))
	{
	    val = ajSeqoutNew();		/* set the default value */
    
	    ajSeqoutClearUsa(val, acdReply);
	    acdGetValueAssoc(thys, "osdbname", &val->Setoutdb);
	    val->Features = osfeat;
	    acdGetValueAssoc(thys, "osdirectory", &val->Directory);
	    acdOutDirectory(&val->Directory);
	    acdLog("acdSetSeqoutset features: %B\n", val->Features);

	    ajStrAssignEmptyS(&val->Formatstr, fmt);

	    if(!ajStrGetLen(val->Formatstr))
		ajSeqoutstrGetFormatDefault(&val->Formatstr);

	    ajStrAssignEmptyS(&val->Extension, ext);
	    ajStrAssignEmptyS(&val->Extension, val->Formatstr);

	    acdGetValueAssoc(thys, "oufo", &val->Ufo);
	    acdGetValueAssoc(thys, "offormat", &val->Ftquery->Formatstr);
	    if(!ajStrGetLen(val->Ftquery->Formatstr))
		ajFeatOutFormatDefault(&val->Ftquery->Formatstr);
	    acdGetValueAssoc(thys, "ofname", &val->Ftquery->Filename);
	    acdGetValueAssoc(thys, "ofdirectory",
			     &val->Ftquery->Directory);
	    acdOutDirectory(&val->Ftquery->Directory);

	    acdQualToBool(thys, "ossingle",
			  ajSeqoutstrIsFormatSingle(val->Formatstr),
			  &val->Single, &acdTmpStr);

	    if(ajStrGetLen(typestr))
		ajStrAssignS(&val->Outputtype, typestr);
	    else
	    {
		if(!acdInTypeSeq(&val->Outputtype))
		    ajWarn("No output type specified for '%S'", thys->Name);
	    }

	    if(!ajSeqoutOpen(val))
	    {
		if(ajStrGetLen(val->Directory))
		    acdBadVal(thys, required,
			      "Unable to write sequence to '%S%S'",
			      val->Directory, acdReply);
		else
		    acdBadVal(thys, required,
			      "Unable to write sequence to '%S'",
			      acdReply);

		ok = ajFalse;
		ajSeqoutDel(&val);
	    }
	}
	else
	    if(!nullok)
	    {
		acdBadVal(thys, required,
			  "Output USA is required");
		ok = ajFalse;
	    }
    }

    if(!ok)
	acdBadRetry(thys);
    
    thys->Value = val;
    ajStrAssignS(&thys->ValStr, acdReply);
    
    ajStrDel(&typestr);
    ajStrDel(&fmt);
    ajStrDel(&ext);

    return;
}




/* @func ajAcdGetSeqset *******************************************************
**
** Returns an item of type Seqset as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPSeqset] Sequence set object. The sequence was already loaded by
**         acdSetSeqset so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
**
** @release 1.0.0
** @@
******************************************************************************/

AjPSeqset ajAcdGetSeqset(const char *token)
{
    return acdGetValueRef(token, "seqset");
}




/* @funcstatic acdSetSeqset ***************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD sequence item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other available) is a null string, which
** is invalid.
**
** Associated qualifiers "-sformat", "-sdbname", "-sopenfile", "-sid"
** are applied to the USA before reading the sequence.
**
** Associated qualifiers "-supper", "-slower" and "-sask" are applied
** after reading.
**
** Associated qualifiers "-sbegin", "-send" and "-sreverse"
** are applied as appropriate, with prompting for values,
** after the sequence has been read. They are applied to the sequence,
** and the resulting sequence is what is set in the ACD item.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajSeqRead
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdSetSeqset(AcdPAcd thys)
{
    AjPSeqset val = NULL;
    AjPSeqin seqin;
    
    AjBool required = ajFalse;
    AjBool ok       = ajFalse;
    AjBool okbeg    = ajFalse;
    AjBool okend    = ajFalse;
    AjBool okrev    = ajFalse;
    AjBool aligned  = ajFalse;

    ajint itry;
    
    AjPStr infname = NULL;

    ajint sqbegin = 0;
    ajint sqend   = 0;
    AjBool sreverse = ajFalse;
    AjBool sprompt  = ajFalse;
    AjBool snuc     = ajFalse;
    AjBool sprot    = ajFalse;
    AjBool nullok   = ajFalse;
    AjPStr typestr  = NULL;
    
    val   = ajSeqsetNew();		/* set the default value */
    seqin = ajSeqinNew();		/* set the default value */

    seqin->Input->Multi = ajTrue;

    acdQualToBool(thys, "snucleotide", ajFalse, &snuc, &acdReplyDef);
    acdQualToBool(thys, "sprotein", ajFalse, &sprot, &acdReplyDef);
    acdAttrToBool(thys, "nullok", ajFalse, &nullok);
    acdAttrToBool(thys, "aligned", ajFalse, &aligned);
    acdAttrToStr(thys, "type", "", &typestr);
    
    acdInFilename(&infname);
    required = acdIsRequired(thys);
    acdReplyInitS(thys, infname, &acdReplyDef);
    acdPromptSeq(thys);
    ajStrDel(&infname);

    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */

	ajStrAssignS(&acdReply, acdReplyDef);

	if(required)
	    acdUserGet(thys, &acdReply);


	if(!ajStrGetLen(acdReply) && nullok)
        {
            ajSeqsetDel(&val);
	    break;
        }

	ajSeqinUsa(&seqin, acdReply);

	if(ajStrGetLen(typestr))
	{
	    ajStrAssignS(&seqin->Inputtype, typestr);
	    acdInTypeSeqSave(seqin->Inputtype);
	}
	else
	    acdInTypeSeqSave(NULL);

	acdAttrToBool(thys, "features", ajFalse, &seqin->Features);
	
	acdGetValueAssoc(thys, "sformat", &seqin->Input->Formatstr);
	acdGetValueAssoc(thys, "sdbname", &seqin->Input->Db);
/*	acdGetValueAssoc(thys, "sopenfile", &seqin->Filename);*/ /* obsolete */
	acdGetValueAssoc(thys, "sid", &seqin->Entryname);
	acdGetValueAssoc(thys, "iquery", &seqin->Input->QryFields);
	acdQualToLong(thys, "ioffset", 0L, &seqin->Input->Fpos,
                      &acdTmpStr);

	acdGetValueAssoc(thys, "ufo", &seqin->Ufo);
	acdGetValueAssoc(thys, "fformat", &seqin->Ftquery->Formatstr);
	acdGetValueAssoc(thys, "fopenfile", &seqin->Ftquery->Filename);

	acdQualToBool(thys, "squick", ajFalse,
		      &seqin->Minimal, &acdTmpStr);
	acdQualToBool(thys, "scircular", ajFalse,
		      &seqin->Circular, &acdTmpStr);
	acdQualToBool(thys, "supper", ajFalse,
		      &seqin->Upper, &acdTmpStr);
	acdQualToBool(thys, "slower", ajFalse,
		      &seqin->Lower, &acdTmpStr);
	okbeg = acdQualToSeqbegin(thys, "sbegin", 0,
				  &seqin->Begin, &acdTmpStr);
	okend = acdQualToSeqend(thys, "send", 0,
				&seqin->End, &acdTmpStr);
	okrev = acdQualToBool(thys, "sreverse", ajFalse,
			      &sreverse, &acdTmpStr);

	if(snuc)
	    ajSeqinSetNuc(seqin);

	if(sprot)
	    ajSeqinSetProt(seqin);

	if(ajStrGetLen(seqin->Ufo))
	    seqin->Features = ajTrue;

	ok = ajSeqsetRead(val, seqin);

	if(!ok)
	    acdBadVal(thys, required,
		      "Unable to read sequence '%S'", acdReply);
    }

    if(!ok)
	acdBadRetry(thys);
    
    acdInFileSave(acdReply, ajSeqsetGetNameS(val),
                  ajTrue);      /* save sequence name */

    acdQualToBool(thys, "sask", ajFalse, &sprompt, &acdReplyDef);

    if(val) 
    {

        /* now process the begin, end and reverse options */
    
        if(seqin->Begin)
        {
            okbeg = ajTrue;
            val->Begin = seqin->Begin;
        }
    
        for(itry=acdPromptTry; itry && !okbeg; itry--)
        {
            ajStrAssignC(&acdReplyPrompt, "start");

            if(sprompt)
                acdUserGetPrompt(thys, "sbegin",
                                 " Begin at position", &acdReplyPrompt);
            if(ajStrMatchCaseC(acdReplyPrompt, "start"))
                ajStrAssignC(&acdReplyPrompt, "0");

            okbeg = ajStrToInt(acdReplyPrompt, &sqbegin);

            if(!okbeg)
                acdBadVal(thys, sprompt,
                          "Invalid integer value '%S'", acdReplyPrompt);
        }

        if(!okbeg)
            acdBadRetry(thys);
    
        if(sqbegin)
        {
            seqin->Begin = sqbegin;
            val->Begin = sqbegin;
            acdSetQualDefInt(thys, "sbegin", sqbegin);
        }
    
        if(seqin->End)
        {
            okend = ajTrue;
            val->End = seqin->End;
        }
    
        for(itry=acdPromptTry; itry && !okend; itry--)
        {
            ajStrAssignC(&acdReplyPrompt, "end");

            if(sprompt)
                acdUserGetPrompt(thys, "send",
                                 "   End at position", &acdReplyPrompt);

            if(ajStrMatchCaseC(acdReplyPrompt, "end"))
                ajStrAssignC(&acdReplyPrompt, "0");

            okend = ajStrToInt(acdReplyPrompt, &sqend);

            if(!okend)
                acdBadVal(thys, sprompt,
                          "Invalid integer value '%S'", acdReplyPrompt);
        }

        if(!okend)
            acdBadRetry(thys);
    
        if(sqend)
        {
            seqin->End = sqend;
            val->End = sqend;
            acdSetQualDefInt(thys, "send", sqend);
        }

        if(ajSeqsetGetSize(val) && ajSeqsetIsNuc(val))
        {
            for(itry=acdPromptTry; itry && !okrev; itry--)
            {
                ajStrAssignC(&acdReplyPrompt, "N");

                if(sprompt)
                    acdUserGetPrompt(thys, "sreverse",
                                     "    Reverse strand", &acdReplyPrompt);

                okrev = ajStrToBool(acdReplyPrompt, &sreverse);

                if(!okrev)
                    acdBadVal(thys, sprompt,
                              "Invalid Y/N value '%S'", acdReplyPrompt);
            }

            if(!okrev)
                acdBadRetry(thys);

            if(sreverse)
            {
                seqin->Rev = sreverse;
                val->Rev = sreverse;
                acdSetQualDefBool(thys, "sreverse", sreverse);
            }
        }

        acdLog("sbegin: %d, send: %d, sreverse: %B\n",
               sqbegin, sqend, sreverse);

        if(val && aligned)
            ajSeqsetFill(val);

        if(val && val->Rev)
            ajSeqsetReverse(val);

        /* sequences have special set attributes */
    
        thys->SAttr = acdAttrListCount(acdCalcSeqset);
        thys->SetAttr = &acdCalcSeqset[0];
        thys->SetStr = AJCALLOC0(thys->SAttr, sizeof(AjPStr));
    
        ajStrFromInt(&thys->SetStr[ACD_SEQ_BEGIN], ajSeqsetGetBegin(val));
        ajStrFromInt(&thys->SetStr[ACD_SEQ_END], ajSeqsetGetEnd(val));
        ajStrFromInt(&thys->SetStr[ACD_SEQ_LENGTH], ajSeqsetGetLen(val));
        ajStrFromBool(&thys->SetStr[ACD_SEQ_PROTEIN], ajSeqsetIsProt(val));
        ajStrFromBool(&thys->SetStr[ACD_SEQ_NUCLEIC], ajSeqsetIsNuc(val));
        if(ajStrGetLen(val->Name))
            ajStrAssignS(&thys->SetStr[ACD_SEQ_NAME],
                         val->Name);
        else
            ajStrAssignS(&thys->SetStr[ACD_SEQ_NAME],
                         ajSeqsetGetseqNameS(val,0));
        ajStrAssignS(&thys->SetStr[ACD_SEQ_USA], ajSeqsetGetUsa(val));
        ajStrFromFloat(&thys->SetStr[ACD_SEQ_WEIGHT],
                       ajSeqsetGetTotweight(val), 3);
        ajStrFromInt(&thys->SetStr[ACD_SEQ_COUNT], ajSeqsetGetSize(val));
    
        acdInFileSave(acdReply, ajSeqsetGetNameS(val), ajTrue);
    }
    else 
    {
       /* sequences have special set attributes */
    
        thys->SAttr = acdAttrListCount(acdCalcSeqset);
        thys->SetAttr = &acdCalcSeq[0];
        thys->SetStr = AJCALLOC0(thys->SAttr, sizeof(AjPStr));
    
        ajStrFromInt(&thys->SetStr[ACD_SEQ_BEGIN], 0);
        ajStrFromInt(&thys->SetStr[ACD_SEQ_END], 0);
        ajStrFromInt(&thys->SetStr[ACD_SEQ_LENGTH], 0);
        ajStrFromBool(&thys->SetStr[ACD_SEQ_NUCLEIC], ajFalse);
        ajStrFromBool(&thys->SetStr[ACD_SEQ_PROTEIN], ajFalse);
        ajStrAssignC(&thys->SetStr[ACD_SEQ_NAME], "");
        ajStrAssignC(&thys->SetStr[ACD_SEQ_USA], "");
        ajStrFromFloat(&thys->SetStr[ACD_SEQ_WEIGHT],
                       0.0, 3);
        ajStrFromInt(&thys->SetStr[ACD_SEQ_COUNT], 0);
    }

    thys->Value = val;
    ajStrAssignS(&thys->ValStr, acdReply);
    
    ajSeqinDel(&seqin);
    ajStrDel(&typestr);

    return;
}




/* @func ajAcdGetSeqsetall ****************************************************
**
** Returns an item of type Seqset array as defined in a named ACD item.
** The array is terminated by a NULL.
**
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPSeqset*] Sequence setall object.
**         The sequence was already loaded by
**         acdSetSeqset so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
**
** @release 2.8.0
** @@
******************************************************************************/

AjPSeqset* ajAcdGetSeqsetall(const char *token)
{
    return acdGetValueRef(token, "seqsetall");
}




/* @func ajAcdGetSeqsetallSingle **********************************************
**
** Returns an item of type Seqset as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPSeqset] Sequence set object. The sequence was already loaded by
**         acdSetSeqset so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
**
** @release 4.0.0
** @@
******************************************************************************/

AjPSeqset ajAcdGetSeqsetallSingle(const char *token)
{
    AjPSeqset *val;
    ajint i;

    val = acdGetValueSingle(token, "seqsetall");

    for(i=0; val[i]; i++)
	continue;

    if(i > 1)
	ajWarn("Single list value %s, but can choose %d values",
	       token, i);

    if(i < 1)
	ajWarn("Single list value %s, no value found: returning NULL value",
	       token);

    return val[0];
}




/* @func ajAcdGetString *******************************************************
**
** Returns an item of type String as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPStr] String object. The string was already set by
**         acdSetString so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
**
** @release 1.0.0
** @@
******************************************************************************/

AjPStr ajAcdGetString(const char *token)
{
    return acdGetValueRef(token, "string");
}




/* @funcstatic acdSetString ***************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD string item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other is available) is an empty string.
**
** Attributes for minimum and maximum length are applied with error
** messages if exceeded.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajFileNewOut
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdSetString(AcdPAcd thys)
{
    AjPStr val;
    
    AjBool required = ajFalse;
    AjBool ok       = ajFalse;

    AjPStr pattern  = NULL;
    AjBool upper;
    AjBool lower;
    AjBool word;
    ajint itry;
    
    AjPRegexp patexp = NULL;
    
    ajint minlen;
    ajint maxlen;
    ajint len;
    
    val = ajStrNew();			/* set the default value */
    
    acdAttrToInt(thys, "minlength", 0, &minlen);
    acdAttrToInt(thys, "maxlength", INT_MAX, &maxlen);
    acdAttrToStr(thys, "pattern", "", &pattern);
    acdAttrToBool(thys, "upper", ajFalse, &upper);
    acdAttrToBool(thys, "lower", ajFalse, &lower);
    acdAttrToBool(thys, "word",  ajFalse, &word);
    
    if(ajStrGetLen(pattern))
	patexp = ajRegComp(pattern);
    
    required = acdIsRequired(thys);
    acdReplyInitC(thys, "", &acdReplyDef);
    
    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */

	ajStrAssignS(&acdReply, acdReplyDef);

	if(required)
	    acdUserGet(thys, &acdReply);

	len = ajStrGetLen(acdReply);

	if(len < minlen)
	{
	    acdBadVal(thys, required,
		      "Too short (%S) - minimum length is %d characters",
		      thys->Name, minlen);
	    ok = ajFalse;
	}

	if(len > maxlen)
	{
	    acdBadVal(thys, required,
		      "Too long (%S) - maximum length is %d characters",
		      thys->Name, maxlen);
	    ok = ajFalse;
	}

	if(patexp && !ajRegExec(patexp, acdReply))
	{
	    acdBadVal(thys, required,
		      "String does not match pattern '%S'",
		      pattern);
	    ok = ajFalse;
	}

	if(word && !(ajStrIsWord(acdReply)))
	{
	    acdBadVal(thys, required,
		      "String contains disallowed whitespace characters");
	    ok = ajFalse;
	}	
    }

    if(!ok)
	acdBadRetry(thys);
    
    if(patexp)
	ajRegFree(&patexp);
    
    if(upper)
	ajStrFmtUpper(&val);
    
    if(lower)
	ajStrFmtLower(&val);
    
    /* strings have special set attributes */
    
    thys->SAttr = acdAttrListCount(acdCalcString);
    thys->SetAttr = &acdCalcString[0];
    thys->SetStr = AJCALLOC0(thys->SAttr, sizeof(AjPStr));
    
    ajStrFromInt(&thys->SetStr[0], ajStrGetLen(acdReply));
    
    ajStrAssignS(&val, acdReply);
    thys->Value = val;
    ajStrAssignS(&thys->ValStr, val);

    ajStrDel(&pattern);
    return;
}




/* @func ajAcdGetTaxon ********************************************************
**
** Returns an item of type Taxon as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPTax] Taxon object
** @cre failure to find an item with the right name and type aborts.
**
** @release 6.4.0
** @@
******************************************************************************/

AjPTax ajAcdGetTaxon(const char *token)
{
    AjPTaxall val = acdGetValue(token, "taxon");
    if(val->Multi)
        ajWarn("ajAcdGetTaxon request single taxon but maxreads > 1");

    val->Returned = ajTrue;

    return val->Tax;
}




/* @func ajAcdGetTaxonall *****************************************************
**
** Returns an input stream of an item of type Taxon as defined in a
** named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPTaxall] Taxon input stream
** @cre failure to find an item with the right name and type aborts.
**
** @release 6.4.0
** @@
******************************************************************************/

AjPTaxall ajAcdGetTaxonall(const char *token)
{
    AjPTaxall val = acdGetValueRef(token, "taxon");

    if(!val->Multi)
        ajWarn("ajAcdGetTaxonall request taxon input stream but maxreads is 1");

    return val;
}




/* @funcstatic acdSetTaxon ****************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD taxon input
**
** Understands all attributes and associated qualifiers for this item type.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void acdSetTaxon(AcdPAcd thys)
{
    AjPTaxall val;

    AjBool required = ajFalse;
    AjBool ok       = ajFalse;
    AjBool nullok   = ajFalse;
    ajint itry;

    ajint maxreads;

    AjPStr infname = NULL;
    
    val = ajTaxallNew();        /* set the default value */

    acdAttrToBool(thys, "nullok", ajFalse, &nullok);
    acdAttrToInt(thys, "maxreads", INT_MAX, &maxreads);

    acdInFilename(&infname);
    required = acdIsRequired(thys);
    acdReplyInitS(thys, infname, &acdReplyDef);
    ajStrDel(&infname);

    acdAttrToBool(thys, "entry", ajFalse, &val->Taxin->Input->Text);

    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */

	ajStrAssignS(&acdReply, acdReplyDef);

	if(required)
	    acdUserGet(thys, &acdReply);

	if(!ajStrGetLen(acdReply) && nullok)
        {
            ajTaxallDel(&val);
	    break;
	}

	ajTaxinQryS(val->Taxin, acdReply);

	acdGetValueAssoc(thys, "iformat", &val->Taxin->Input->Formatstr);
	acdGetValueAssoc(thys, "iquery", &val->Taxin->Input->QryFields);
	acdQualToLong(thys, "ioffset", 0L, &val->Taxin->Input->Fpos,
                      &acdTmpStr);

        ok = ajTaxinRead(val->Taxin, val->Tax);
    }

    if(!ok)
	acdBadRetry(thys);

    if(maxreads > 1)
        val->Multi = ajTrue;

    acdInFileSave(acdReply, ajTaxallGettaxId(val), ajTrue);

    thys->Value = val;
    ajStrAssignS(&thys->ValStr, acdReply);
    
    return;
}




/* @func ajAcdGetText *********************************************************
**
** Returns an item of type Text as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPText] Text object
** @cre failure to find an item with the right name and type aborts.
**
** @release 6.4.0
** @@
******************************************************************************/

AjPText ajAcdGetText(const char *token)
{
    AjPTextall val = acdGetValue(token, "text");
    if(val->Multi)
        ajWarn("ajAcdGetText request single text entry but maxreads > 1");

    val->Returned = ajTrue;

    return val->Text;
}




/* @func ajAcdGetTextall ******************************************************
**
** Returns an input stream of an item of type Text as defined in a
** named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPTextall] Text input stream
** @cre failure to find an item with the right name and type aborts.
**
** @release 6.4.0
** @@
******************************************************************************/

AjPTextall ajAcdGetTextall(const char *token)
{
    AjPTextall val = acdGetValueRef(token, "text");

    if(!val->Multi)
        ajWarn("ajAcdGetTextall request text input stream but maxreads is 1");

    return val;
}




/* @funcstatic acdSetText *****************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD text input
**
** Understands all attributes and associated qualifiers for this item type.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void acdSetText(AcdPAcd thys)
{
    AjPTextall val;

    AjBool required = ajFalse;
    AjBool ok       = ajFalse;
    AjBool nullok   = ajFalse;
    ajint itry;

    ajint maxreads;

    AjPStr infname = NULL;

    val = ajTextallNew();        /* set the default value */

    acdAttrToBool(thys, "nullok", ajFalse, &nullok);
    acdAttrToInt(thys, "maxreads", INT_MAX, &maxreads);

    acdInFilename(&infname);
    required = acdIsRequired(thys);
    acdReplyInitS(thys, infname, &acdReplyDef);
    ajStrDel(&infname);

    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */

	ajStrAssignS(&acdReply, acdReplyDef);

	if(required)
	    acdUserGet(thys, &acdReply);

	if(!ajStrGetLen(acdReply) && nullok)
        {
            ajTextallDel(&val);
	    break;
	}

	ajTextinQryS(val->Textin, acdReply);

	acdGetValueAssoc(thys, "iformat", &val->Textin->Formatstr);
	acdGetValueAssoc(thys, "iquery", &val->Textin->QryFields);
	acdQualToLong(thys, "ioffset", 0L, &val->Textin->Fpos,
                      &acdTmpStr);

        ok = ajTextinRead(val->Textin, val->Text);
    }

    if(!ok)
	acdBadRetry(thys);

    if(maxreads > 1)
        val->Multi = ajTrue;

    acdAttrToBool(thys, "entry", ajFalse, &val->Textin->Text);

    /* acdInFileSave(acdReply, ajTextallGettextId(val), ajTrue); */

    thys->Value = val;
    ajStrAssignS(&thys->ValStr, acdReply);
    
    return;
}




/* @func ajAcdGetToggle *******************************************************
**
** Returns an item of type Toggle as defined in a named ACD item. Called by the
** application after all ACD values have been set, and simply returns
** what the ACD item already has.
**
** Toggle is an AjBool that is used to switch (toggle) other ACD types.
**
** @param [r] token [const char*] Text token name
** @return [AjBool] Boolean value from ACD item
** @cre failure to find an item with the right name and type aborts.
**
** @release 2.9.0
** @@
******************************************************************************/

AjBool ajAcdGetToggle(const char *token)
{
    AjBool* val;

    val = acdGetValue(token, "toggle");

    return *val;
}




/* @funcstatic acdSetToggle ***************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD toggle item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other available) is "N" for ajFalse.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajStrToBool
**
** @release 2.9.0
** @@
******************************************************************************/


static void acdSetToggle(AcdPAcd thys)
{
    AjBool* val;

    AjBool required = ajFalse;
    AjBool ok       = ajFalse;

    ajint itry;

    AJNEW0(val);		   /* create storage for the result */

    *val = ajFalse;			/* set the default value */

    required = acdIsRequired(thys);
    acdReplyInitC(thys, "N", &acdReplyDef);

    acdLog("acdSetToggle -%S def: %S\n", thys->Name, acdReplyDef);


    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ajStrAssignS(&acdReply, acdReplyDef);

	if(required)
	    acdUserGet(thys, &acdReply);

	ok = ajStrToBool(acdReply, val);

	if(!ok)
	    acdBadVal(thys, required, "Invalid Y/N value '%S'", acdReply);
    }

    if(!ok)
	acdBadRetry(thys);

    thys->Value = val;
    ajFmtPrintS(&thys->ValStr, "%B", *val);

    acdLog("acdSetToggle -%S val: %B\n", thys->Name, *val);

    if(ajStrMatchC(thys->Name, "help"))
	acdHelp();

    return;
}




/* @func ajAcdGetTree *********************************************************
**
** Returns an item of type Tree as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPPhyloTree*] Tree object. The data was already set by
**         acdSetTree so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
**
** @release 2.8.0
** @@
******************************************************************************/

AjPPhyloTree* ajAcdGetTree(const char *token)
{
    return acdGetValueRef(token, "tree");
}




/* @func ajAcdGetTreeSingle ***************************************************
**
** Returns an item of type Tree as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPPhyloTree] Tree object. The data was already set by
**         acdSetTree so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
**
** @release 4.0.0
** @@
******************************************************************************/

AjPPhyloTree ajAcdGetTreeSingle(const char *token)
{
    AjPPhyloTree *val;
    ajint i;

    val = acdGetValueSingle(token, "tree");

    for(i=0; val[i]; i++)
	continue;

    if(i > 1)
	ajWarn("Single list value %s, but can choose %d values",
	       token, i);

    if(i < 1)
	ajWarn("Single list value %s, no value found: returning NULL value",
	       token);

    return val[0];
}




/* @funcstatic acdSetTree *****************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD tree file item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other is available) is an empty string.
**
** Attributes for number of trees (size) are applied with error
** messages if exceeded.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
**
** @release 2.8.0
** @@
******************************************************************************/

static void acdSetTree(AcdPAcd thys)
{
    AjPPhyloTree *val;

    AjBool required = ajFalse;
    AjBool ok       = ajFalse;

    AjPStr infname  = NULL;
    ajint itry;
    ajint i;
    
    ajint size;
    AjBool nullok = ajFalse;

    val = NULL;

    acdAttrToBool(thys, "nullok", ajFalse, &nullok);
    acdAttrToInt(thys, "size", 0, &size);

    acdInFilename(&infname);
    required = acdIsRequired(thys);
    acdReplyInitS(thys, infname, &acdReplyDef);
    acdPromptTree(thys);
    ajStrDel(&infname);

    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */

	ajStrAssignS(&acdReply, acdReplyDef);

	if(required)
	    acdUserGet(thys, &acdReply);

	if(ajStrGetLen(acdReply))
	{
	    val = ajPhyloTreeRead(acdReply, size);

	    if(!val)
	    {
		acdBadVal(thys, required,
			  "Unable to read tree data from '%S'",
			  acdReply);
		ok = ajFalse;
	    }
	}
	else
	    if(!nullok)
	    {
		acdBadVal(thys, required,
			  "Input file is required");
		ok = ajFalse;
	    }
    }

    if(!ok)
	acdBadRetry(thys);
    acdInFileSave(acdReply, NULL, ajTrue);
    
    /* trees have special set attributes */

    thys->SAttr = acdAttrListCount(acdCalcTree);
    thys->SetAttr = &acdCalcTree[0];
    thys->SetStr = AJCALLOC0(thys->SAttr, sizeof(AjPStr));

    if(val)
    {
	for(i=0;val[i];i++)
	    continue;

	ajStrFromInt(&thys->SetStr[0],i); /* number of trees */
	ajStrFromInt(&thys->SetStr[1],val[0]->Size);
	ajStrFromBool(&thys->SetStr[2],val[0]->HasLengths);
	ajStrAssignS(&thys->ValStr, val[0]->Tree);
    }
    else
    {
	ajStrFromInt(&thys->SetStr[0],0); /* number of trees */
	ajStrFromInt(&thys->SetStr[1],0);
	ajStrFromBool(&thys->SetStr[2],ajFalse);
	ajStrAssignClear(&thys->ValStr);
    }

    thys->Value = val;
    
    return;
}




/* @func ajAcdGetUrl **********************************************************
**
** Returns an item of type URL as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPUrl] URL object
** @cre failure to find an item with the right name and type aborts.
**
** @release 6.4.0
** @@
******************************************************************************/

AjPUrl ajAcdGetUrl(const char *token)
{
    AjPUrlall val = acdGetValue(token, "url");
    if(val->Multi)
        ajWarn("ajAcdGetUrl request single URL but maxreads > 1");

    val->Returned = ajTrue;

    return val->Url;
}




/* @func ajAcdGetUrlall *******************************************************
**
** Returns an input stream of an item of type URL as defined in a
** named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPUrlall] URL input stream
** @cre failure to find an item with the right name and type aborts.
**
** @release 6.4.0
** @@
******************************************************************************/

AjPUrlall ajAcdGetUrlall(const char *token)
{
    AjPUrlall val = acdGetValueRef(token, "url");

    if(!val->Multi)
        ajWarn("ajAcdGetUrlall request url input stream but maxreads is 1");

    return val;
}




/* @funcstatic acdSetUrl ******************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD URL input
**
** Understands all attributes and associated qualifiers for this item type.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void acdSetUrl(AcdPAcd thys)
{
    AjPUrlall val;

    AjBool required = ajFalse;
    AjBool ok       = ajFalse;
    AjBool nullok   = ajFalse;
    ajint itry;

    ajint maxreads;

    AjPStr infname = NULL;

    val = ajUrlallNew();        /* set the default value */

    acdAttrToBool(thys, "nullok", ajFalse, &nullok);
    acdAttrToInt(thys, "maxreads", INT_MAX, &maxreads);

    acdInFilename(&infname);
    required = acdIsRequired(thys);
    acdReplyInitS(thys, infname, &acdReplyDef);
    ajStrDel(&infname);

    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */

	ajStrAssignS(&acdReply, acdReplyDef);

	if(required)
	    acdUserGet(thys, &acdReply);

	if(!ajStrGetLen(acdReply) && nullok)
        {
            ajUrlallDel(&val);
	    break;
	}

	ajUrlinQryS(val->Urlin, acdReply);

        /* these are reset by ajUrlinQryS - can safely set now */
        acdQualToBool(thys, "swiss", ajFalse, &val->Urlin->IsSwiss,
                      &acdTmpStr);
        acdQualToBool(thys, "embl", ajFalse, &val->Urlin->IsEmbl,
                      &acdTmpStr);
        acdGetValueAssoc(thys, "accession", &val->Urlin->Accession);
        acdGetValueAssoc(thys, "identifier", &val->Urlin->IdTypes);

	acdGetValueAssoc(thys, "iformat", &val->Urlin->Input->Formatstr);

        ok = ajUrlinRead(val->Urlin, val->Url);
    }

    if(!ok)
	acdBadRetry(thys);

    if(maxreads > 1)
        val->Multi = ajTrue;

    acdAttrToBool(thys, "entry", ajFalse, &val->Urlin->Input->Text);

    acdInFileSave(acdReply, ajUrlallGeturlId(val), ajTrue);

    thys->Value = val;
    ajStrAssignS(&thys->ValStr, acdReply);
    
    return;
}




/* @func ajAcdGetValue ********************************************************
**
** Returns the string value of any ACD item
**
** @param [r] token [const char*] Text token name
** @return [const AjPStr] String object. The string was already set by
**         acdSetString so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
**
** @release 4.0.0
** @@
******************************************************************************/

const AjPStr ajAcdGetValue(const char *token)
{
    return acdGetValStr(token);
}




/* @func ajAcdGetValueDefault *************************************************
**
** Returns the default value of any ACD item
**
** @param [r] token [const char*] Text token name
** @return [const AjPStr] Default value.
** @cre failure to find an item with the right name and type returns NULL.
**
** @release 6.0.0
** @@
******************************************************************************/

const AjPStr ajAcdGetValueDefault(const char *token)
{
    return acdGetValDefault(token);
}




/* @func ajAcdGetVariation ****************************************************
**
** Returns an item of type variation loader as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPVarload] Variation loader object
** @cre failure to find an item with the right name and type aborts.
**
** @release 6.4.0
** @@
******************************************************************************/

AjPVarload ajAcdGetVariation(const char *token)
{
    AjPVarload val = acdGetValueRef(token, "variation");

    /*if(val->Multi)
        ajWarn("ajAcdGetVariation request single variation but maxreads > 1");
    */

    val->Returned = ajTrue;

    return val;
}




/* @funcstatic acdSetVariation ************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD variation input
**
** Understands all attributes and associated qualifiers for this item type.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void acdSetVariation(AcdPAcd thys)
{
    AjPVarload val;

    AjBool required = ajFalse;
    AjBool ok       = ajFalse;
    AjBool nullok   = ajFalse;
    ajint itry;

    AjPStr infname = NULL;

    val = ajVarloadNew();        /* set the default value */

    acdAttrToBool(thys, "nullok", ajFalse, &nullok);

    acdInFilename(&infname);
    required = acdIsRequired(thys);
    acdReplyInitS(thys, infname, &acdReplyDef);
    ajStrDel(&infname);

    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */

	ajStrAssignS(&acdReply, acdReplyDef);

	if(required)
	    acdUserGet(thys, &acdReply);

	if(!ajStrGetLen(acdReply) && nullok)
        {
            ajVarloadDel(&val);
	    break;
	}

        ajVarinQryS(val->Varin, acdReply);

	acdGetValueAssoc(thys, "iformat", &val->Varin->Input->Formatstr);
	acdGetValueAssoc(thys, "iquery", &val->Varin->Input->QryFields);
	acdQualToLong(thys, "ioffset", 0L, &val->Varin->Input->Fpos,
                      &acdTmpStr);

        ok = ajVarinLoad(val->Varin, val->Var);
    }

    if(!ok)
	acdBadRetry(thys);

    acdAttrToBool(thys, "entry", ajFalse, &val->Varin->Input->Text);

    acdInFileSave(acdReply, ajVarloadGetvarId(val), ajTrue);

    thys->Value = val;
    ajStrAssignS(&thys->ValStr, acdReply);
    
    return;
}




/* @func ajAcdGetXml **********************************************************
**
** Returns an item of type xml as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPXml] Xml object
** @cre failure to find an item with the right name and type aborts.
**
** @release 6.6.0
** @@
******************************************************************************/

AjPXml ajAcdGetXml(const char *token)
{
    AjPXml val = acdGetValueRef(token, "xml");

    return val;
}




/* @func ajAcdGetXmlall *******************************************************
**
** Returns an input stream of an item of type XML as defined in a
** named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [const char*] Text token name
** @return [AjPXmlall] URL input stream
** @cre failure to find an item with the right name and type aborts.
**
** @release 6.6.0
** @@
******************************************************************************/

AjPXmlall ajAcdGetXmlall(const char *token)
{
    AjPXmlall val = acdGetValueRef(token, "xml");

    if(!val->Multi)
        ajWarn("ajAcdGetXmlall request xml input stream but maxreads is 1");

    return val;
}




/* @funcstatic acdSetXml ******************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD xml input
**
** Understands all attributes and associated qualifiers for this item type.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
**
** @release 6.6.0
** @@
******************************************************************************/

static void acdSetXml(AcdPAcd thys)
{
    AjPXmlall val;

    AjBool required = ajFalse;
    AjBool ok       = ajFalse;
    AjBool nullok   = ajFalse;
    ajint itry;

    ajint maxreads;

    AjPStr infname = NULL;
    
    val = ajXmlallNew();        /* set the default value */

    acdAttrToBool(thys, "nullok", ajFalse, &nullok);
    acdAttrToInt(thys, "maxreads", INT_MAX, &maxreads);

    acdInFilename(&infname);
    required = acdIsRequired(thys);
    acdReplyInitS(thys, infname, &acdReplyDef);
    ajStrDel(&infname);

    acdAttrToBool(thys, "entry", ajFalse, &val->Xmlin->Input->Text);

    for(itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */

	ajStrAssignS(&acdReply, acdReplyDef);

	if(required)
	    acdUserGet(thys, &acdReply);

	if(!ajStrGetLen(acdReply) && nullok)
        {
            ajXmlallDel(&val);
	    break;
	}

	ajXmlinQryS(val->Xmlin, acdReply);

	acdGetValueAssoc(thys, "iformat", &val->Xmlin->Input->Formatstr);
	acdGetValueAssoc(thys, "iquery", &val->Xmlin->Input->QryFields);
	acdQualToLong(thys, "ioffset", 0L, &val->Xmlin->Input->Fpos,
                      &acdTmpStr);

        ok = ajXmlinRead(val->Xmlin, val->Xml);
    }

    if(!ok)
	acdBadRetry(thys);

    if(maxreads > 1)
        val->Multi = ajTrue;

    acdInFileSave(acdReply, NULL, ajTrue);

    thys->Value = val;
    ajStrAssignS(&thys->ValStr, acdReply);
    
    return;
}




/* @func ajAcdGetpathC ********************************************************
**
** Returns the full path of an application defined in an external: attribute
** within the application definition. If the application was defined by an
** EMBOSS_appname environment variable this will use the substitution made
** when the external attribute was validated.
**
** @param [r] token [const char*] External application name
** @return [const AjPStr] Executable application name
**
** @release 6.2.0
** @@
******************************************************************************/

const AjPStr ajAcdGetpathC(const char* token)
{
    const AjPStr value;	       /* not static - copy of a table text */
    AjPStr tmpname = NULL;

    ajStrAssignC(&tmpname, token);

    value = ajAcdGetpathS(tmpname);

    ajStrDel(&tmpname);

    return value;
}




/* @func ajAcdGetpathS ********************************************************
**
** Returns the full path of an application defined in an external: attribute
** within the application definition. If the application was defined by an
** EMBOSS_appname environment variable this will use the substitution made
** when the external attribute was validated.
**
** @param [r] strtoken [const AjPStr] External application name
** @return [const AjPStr] Executable application name
**
** @release 6.2.0
** @@
******************************************************************************/

const AjPStr ajAcdGetpathS(const AjPStr strtoken)
{
    const AjPStr value;

    value = ajTableFetchS(acdExternalTable, strtoken);

    if(!value) 
        ajWarn("Cannot find '%S', no ACD external definition found", strtoken);

    return value;
}




/* @func ajAcdIsUserdefinedC **************************************************
**
** Tests whether an ACD item has a value set by the user.
**
** @param [r] token [const char*] Text token name
** @return [AjBool] True if token is not found as an ACD object name
**
** @cre failure to find an item with the right name gives an error message
** and continues
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ajAcdIsUserdefinedC(const char *token)
{
    AcdPAcd acd;
    ajint pnum = 0;		   /* need to get from end of token */
    AjPStr tmpstr = NULL;

    tmpstr = ajStrNewC(token);

    acdTokenToLowerS(&tmpstr, &pnum);

    acd = acdFindAcd(tmpstr, tmpstr);

    if(!acd)
    {
        ajErr("Qualifier '-%s' not found", token);
        return ajFalse;
    }

    ajStrDel(&tmpstr);

    return acd->UserDefined;
}




/* @func ajAcdIsUserdefinedS **************************************************
**
** Tests whether an ACD item has a value set by the user.
**
** @param [r] strtoken [const AjPStr] Text token name
** @return [AjBool] True if token is not found as an ACD object name
**
** @cre failure to find an item with the right name gives an error message
** and continues
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ajAcdIsUserdefinedS(const AjPStr strtoken)
{
    AcdPAcd acd;
    ajint pnum = 0;		   /* need to get from end of token */
    AjPStr tmpstr = NULL;

    tmpstr = ajStrNewS(strtoken);

    acdTokenToLowerS(&tmpstr, &pnum);

    acd = acdFindAcd(tmpstr, tmpstr);

    if(!acd)
    {
        ajErr("Qualifier '-%S' not found", strtoken);
        return ajFalse;
    }

    ajStrDel(&tmpstr);

    return acd->UserDefined;
}




/* @funcstatic acdAttrCount ***************************************************
**
** Simply counts all attributes for a numbered ACD type,
**
** @param [r] itype [ajint] Numbered type as returned by acdFindType
** @return [ajint] number of attributes defined.
**
** @release 1.0.0
** @@
******************************************************************************/

static ajint acdAttrCount(ajint itype)
{
    AcdPAttr attr = acdType[itype].Attr;

    return acdAttrListCount(attr);
}




/* @funcstatic acdAttrKeyCount ************************************************
**
** Simply counts all attributes for a numbered ACD keyword,
**
** @param [r] ikey [ajint] Numbered type as returned by acdFindKey
** @return [ajint] number of attributes defined.
**
** @release 1.0.0
** @@
******************************************************************************/

static ajint acdAttrKeyCount(ajint ikey)
{
    AcdPAttr attr = acdKeywords[ikey].Attr;

    return acdAttrListCount(attr);
}




/* @funcstatic acdAttrListCount ***********************************************
**
** Simply counts all attributes for an attribute list.
**
** @param [r] attr [const AcdPAttr] Attribute list
** @return [ajint] number of attributes defined.
**
** @release 1.0.0
** @@
******************************************************************************/

static ajint acdAttrListCount(const AcdPAttr attr)
{
    static ajint i;

    i = 0;

    while(attr[i].Name)
	i++;

    return i;
}




/* @funcstatic acdGetValue ****************************************************
**
** Picks up a token by name and tests the type.
** The value is returned as type "void*", to be cast by the calling
** routine which is supposed to know what to expect. For example, only
** ajAcdGetInt should call with a type of "integer".
**
** @param [r] token [const char*] Token name, optionally including a
**                                numeric suffix.
** @param [r] type [const char*] Type.
** @return [void*] Value.
**
**
** @release 1.0.0
******************************************************************************/

static void* acdGetValue(const char *token, const char* type)
{
    void *ret;
    ajint pnum = 0;		   /* need to get from end of token */
    AjPStr tmpstr = NULL;

    tmpstr = ajStrNewC(token);

    acdLog("acdGetValue '%s' (%s)\n", token, type);

    acdTokenToLowerS(&tmpstr, &pnum);

    ret = acdGetValueNumS(tmpstr, type, pnum, REF_NONE);

    acdLog("acdGetValue '%s' result %x\n", token, ret);

    ajStrDel(&tmpstr);

    return ret;
}




/* @funcstatic acdGetValueRef *************************************************
**
** Picks up a token by name and tests the type.
** The value is returned as type "void*", to be cast by the calling
** routine which is supposed to know what to expect. For example, only
** ajAcdGetInt should call with a type of "integer".
**
** The value reference is passed up to the original caller who now
** 'owns' the pointer. Some values will remain owned by ACD. Examples
** include integers and booleans, but also lists of options where only
** the first option is returned.
**
** @param [r] token [const char*] Token name, optionally including a
**                                numeric suffix.
** @param [r] type [const char*] Type.
** @return [void*] Value.
**
**
** @release 4.0.0
******************************************************************************/

static void* acdGetValueRef(const char *token, const char* type)
{
    void *ret;
    ajint pnum = 0;		   /* need to get from end of token */
    AjPStr tmpstr = NULL;

    tmpstr = ajStrNewC(token);

    acdLog("acdGetValue '%s' (%s)\n", token, type);

    acdTokenToLowerS(&tmpstr, &pnum);

    ret = acdGetValueNumS(tmpstr, type, pnum, REF_ALL);

    acdLog("acdGetValueRef '%s' result %x\n", token, ret);

    ajStrDel(&tmpstr);

    return ret;
}




/* @funcstatic acdGetValueSingle **********************************************
**
** Picks up a token by name and tests the type.
** The value is returned as type "void*", to be cast by the calling
** routine which is supposed to know what to expect. For example, only
** ajAcdGetInt should call with a type of "integer".
**
** The value reference is passed up to the original caller who now
** 'owns' the pointer to the first data value, but the value array will
** need to be freed on exit.
**
** @param [r] token [const char*] Token name, optionally including a
**                                numeric suffix.
** @param [r] type [const char*] Type.
** @return [void*] Value.
**
**
** @release 4.0.0
******************************************************************************/

static void* acdGetValueSingle(const char *token, const char* type)
{
    void *ret;
    ajint pnum = 0;		   /* need to get from end of token */
    AjPStr tmpstr = NULL;

    tmpstr = ajStrNewC(token);

    acdLog("acdGetValue '%s' (%s)\n", token, type);

    acdTokenToLowerS(&tmpstr, &pnum);

    ret = acdGetValueNumS(tmpstr, type, pnum, REF_SINGLE);

    acdLog("acdGetValueSingle '%s' result %x\n", token, ret);

    ajStrDel(&tmpstr);

    return ret;
}




/* @funcstatic acdGetValStr ***************************************************
**
** Picks up a token by name and tests the type.
** The string value is returned for any data type.
**
** @param [r] token [const char*] Token name, optionally including
**                                a numeric suffix.
** @return [const AjPStr] String.
**
**
** @release 1.0.0
******************************************************************************/

static const AjPStr acdGetValStr(const char *token)
{
    AcdPAcd acd;
    ajint pnum = 0;		   /* need to get from end of token */
    AjPStr tmpstr = NULL;

    tmpstr = ajStrNewC(token);

    acdLog("acdGetValStr '%s' (%s)\n", token);

    acdTokenToLowerS(&tmpstr, &pnum);

    acd = acdFindAcd(tmpstr, tmpstr);
    ajStrDel(&tmpstr);

    if(!acd)
        return NULL;

    return acd->ValStr;
}




/* @funcstatic acdGetValDefault ***********************************************
**
** Picks up a token by name and tests the type.
** The default string value is returned for any data type.
**
** @param [r] token [const char*] Token name, optionally including
**                                a numeric suffix.
** @return [const AjPStr] String.
**
**
** @release 6.0.0
******************************************************************************/

static const AjPStr acdGetValDefault(const char *token)
{
    AcdPAcd acd;
    ajint pnum = 0;		   /* need to get from end of token */
    AjPStr tmpstr = NULL;

    tmpstr = ajStrNewC(token);

    acdLog("acdGetValStr '%s' (%s)\n", token);

    acdTokenToLowerS(&tmpstr, &pnum);

    acd = acdFindAcd(tmpstr, tmpstr);
    ajStrDel(&tmpstr);

    if(!acd)
        return NULL;

    return acd->DefStr[DEF_DEFAULT];
}




/* @funcstatic acdGetValueAssoc ***********************************************
**
** Picks up the value for an associated qualifier as a string.
**
** @param [r] thys [const AcdPAcd] ACD item for the master parameter/qualifier
** @param [r] token [const char*] Token name, optionally including a
**                                numeric suffix.
** @param [w] result [AjPStr*] String for the resulting value.
** @return [AjBool] ajTrue if found.
** @cre Aborts if not found.
**
**
** @release 1.0.0
******************************************************************************/

static AjBool acdGetValueAssoc(const AcdPAcd thys, const char *token,
                                      AjPStr *result)
{
    ajint pnum = 0;		   /* need to get from end of token */
    AcdPAcd pa;
    AjPStr tmpstr = NULL;

    tmpstr = ajStrNewC(token);
    acdLog("acdGetValueAssocDefault '%s' (%S)\n", token, thys->Name);

    acdTokenToLowerS(&tmpstr, &pnum);
    ajStrDel(&tmpstr);

    if(pnum)
	acdErrorAcd(thys,
		    "associated token '%s' is numbered - not allowed\n",
		    token);

    for(pa=thys->AssocQuals; pa && pa->Assoc; pa=pa->Next)
	if(ajStrMatchC(pa->Token, token))
	{
	    ajStrAssignS(result, pa->ValStr);
	    return pa->Defined;
	}

    acdErrorAcd(thys,
		"Token '%s' not found\n",
		token);

    return ajFalse;
}




/* @funcstatic acdGetValueAssocDefault ****************************************
**
** Picks up the default value for an associated qualifier as a string.
**
** @param [r] thys [const AcdPAcd] ACD item for the master parameter/qualifier
** @param [r] token [const char*] Token name, optionally including a
**                                numeric suffix.
** @param [w] result [AjPStr*] String for the resulting value.
** @return [AjBool] ajTrue if found.
** @cre Aborts if not found.
**
**
** @release 6.4.0
******************************************************************************/

static AjBool acdGetValueAssocDefault(const AcdPAcd thys, const char *token,
                                      AjPStr *result)
{
    ajint pnum = 0;		   /* need to get from end of token */
    AcdPAcd pa;
    AjPStr tmpstr = NULL;

    tmpstr = ajStrNewC(token);
    acdLog("acdGetValueAssocDefault '%s' (%S)\n", token, thys->Name);

    acdTokenToLowerS(&tmpstr, &pnum);
    ajStrDel(&tmpstr);

    if(pnum)
	acdErrorAcd(thys,
		    "associated token '%s' is numbered - not allowed\n",
		    token);

    for(pa=thys->AssocQuals; pa && pa->Assoc; pa=pa->Next)
	if(ajStrMatchC(pa->Token, token))
	{
	    ajStrAssignS(result, pa->DefStr[DEF_DEFAULT]);
	    return pa->Defined;
	}

    acdErrorAcd(thys,
		"Token '%s' not found\n",
		token);

    return ajFalse;
}




/* @funcstatic acdGetValueNumC ************************************************
**
** Picks up the value by name, type and number.
**
** @param [r] token [const char*] Token name
** @param [r] type [const char*] ACD type
** @param [r] pnum [ajint] parameter number, or 0 for a general qualifier.
** @param [r] reftype [ajint] Reference passing type.
**                            REF_NONE means ACD still owns the value,
**                            REF_ALL means the caller owns the value,
**                            REF_SINGLE means the caller owns one value,
**                            but ACD owns the array
** @return [void*] Value of unknown type.
** @cre Aborts if not found.
**
**
** @release 4.0.0
******************************************************************************/

static void* acdGetValueNumC(const char *token, const char* type,
			     ajint pnum, ajint reftype)
{
    AcdPAcd pa;
    AcdPAcd ret = NULL;

    ajint itype  = 0;
    ajint ifound = 0;

    AjPStr ambigList = NULL;

    ambigList = ajStrNew();

    if(type)
	itype = acdFindTypeC(type);

    for(pa=acdList; pa; pa=pa->Next)
    {
	if(acdIsStype(pa))
            continue;

	if(ajStrMatchC(pa->Token, token))
	{
	    acdLog("Found pa->Token '%S' pa->Type %d itype: %d\n",
		   pa->Token, pa->Type, itype);

	    if(pa->Level != ACD_QUAL && pa->Level != ACD_PARAM )
		ajDie("Unknown qualifier '-%S'", pa->Token);

	    if((itype>=0) && (pa->Type != itype)) /* program source error */
		ajDie("Value for '-%S' is not of type %s (found type %s)",
                      pa->Token, type, acdType[pa->Type].Name);

	    if(pa->PNum == pnum)
	    {
		acdLog("found %S [%d] '%S'\n",
		       pa->Name, pa->PNum, pa->ValStr);

		if(pa->Used & USED_GET)
		    ajWarn("Value for '-%S' used more than once", pa->Token);

		pa->Used |= USED_GET;
		ajStrDel(&ambigList);
		pa->RefPassed = reftype;

		return pa->Value;
	    }
	    else if(!pnum) /* matches any if no number, so count them */
	    {
		ifound++;
		ret = pa;
		acdAmbigApp(&ambigList, pa->Token);
	    }
	}
    }

    if(ifound > 1)
    {
	ajWarn("Ambiguous qualifier '-%s' (%S)", token, ambigList);
	ajStrDel(&ambigList);
    }

    if(ifound == 1)
    {
	acdLog("found %S [%d] '%S'\n",
	       ret->Name, ret->PNum, ret->ValStr);
	ret->Used |= USED_GET;
	if (acdDoValid)
	    acdWarn("Abbreviated qualifier '%S' (%S)", token, ambigList);
	ajStrDel(&ambigList);
	ret->RefPassed = reftype;

	return ret->Value;
    }

    ajStrDel(&ambigList);

    /* program source error */
    ajDie("Qualifier '-%s' not found", token);

    return NULL;
}




/* @funcstatic acdGetValueNumS ************************************************
**
** Picks up the value by name, type and number.
**
** @param [r] token [const AjPStr] Token name
** @param [r] type [const char*] ACD type
** @param [r] pnum [ajint] parameter number, or 0 for a general qualifier.
** @param [r] reftype [ajint] Reference passing type.
**                            REF_NONE means ACD still owns the value,
**                            REF_ALL means the caller owns the value,
**                            REF_SINGLE means the caller owns one value,
**                            but ACD owns the array
** @return [void*] Value of unknown type.
** @cre Aborts if not found.
**
**
** @release 4.0.0
******************************************************************************/

static void* acdGetValueNumS(const AjPStr token, const char* type,
                             ajint pnum, ajint reftype)
{
    return acdGetValueNumC(ajStrGetPtr(token), type,
			   pnum, reftype);
}




/* @funcstatic acdHelp ********************************************************
**
** Reports on program options if acdDoHelp is set, either by
** -help on the command line or by the prefix_HELP variable..
**
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdHelp(void)
{
    AcdPAcd pa;
    static AjPStr helpReq = NULL;
    static AjPStr helpOpt = NULL;
    static AjPStr helpAdv = NULL;
    static AjPStr helpAss = NULL;
    static AjPStr helpGen = NULL;
    static AjPStr helpStr = NULL;

    enum
    {
	HELP_UNK, HELP_APP, HELP_REQ, HELP_OPT,
	HELP_ADV, HELP_ASS, HELP_GEN
    } helpType;

    AjPStr* def;
    AjBool tmpBool;
    char hlpFlag;
    ajint iattr = 0;
    ajlong igrp = 0;
    AjPStr groupname = NULL;
    AjPStr tmpstr = NULL;
    AjPStr applversion = NULL;

    AjBool flagReq = ajFalse;
    AjBool flagOpt = ajFalse;

    AjPList reqlist = NULL;
    AjPList optlist = NULL;
    AjPList advlist = NULL;
    AjPList genlist = NULL;
    AjPList asslist = NULL;
    AjPList inlist = NULL;
    AjPList outlist = NULL;
    
    acdLog("acdHelp %B\n", acdDoHelp);
    
    if(!acdDoHelp)
        return;
    
    if(acdDoTable)
    {
	reqlist = ajListNew();
	optlist = ajListNew();
	advlist = ajListNew();
	genlist = ajListNew();
	if(acdVerbose)
	    asslist = ajListNew();
	ajUserDumpC("<table border cellspacing=0 cellpadding=3 "
	       "bgcolor=\"#ccccff\">");
	/* was #f5f5ff */
    }

    if(acdDoXsd || acdDoGalaxy)
    {
        inlist = ajListNew();
        outlist = ajListNew();
    }

    acdLog("++ acdHelp\n");

    for(pa=acdList; pa; pa=pa->Next)
    {
	if(acdIsStype(pa))
            continue;

	hlpFlag = ' ';
	acdLog("++ Name %S Level %d Assoc %B AssocQuals %x\n",
	       pa->Name, pa->Level, pa->Assoc, pa->AssocQuals);
	helpType = HELP_ADV;

	if(pa->Level == ACD_APPL)
	    helpType = HELP_APP;
	else
	    if(!acdIsQtype(pa))
                continue;
	
	def = pa->DefStr;
	
	if(def && ajStrGetLen(def[DEF_ADDITIONAL]))
	{
	    if(acdHelpVarResolve(&helpStr, def[DEF_ADDITIONAL]))
	    {
		if(!ajStrToBool(helpStr, &tmpBool))
		    acdErrorAcd(pa, "Bad additional flag %S\n",
				def[DEF_ADDITIONAL]);
	    }
	    else
	    {
		tmpBool = ajTrue;
		hlpFlag = '*';
		flagOpt = ajTrue;
	    }

	    if(tmpBool)
		helpType = HELP_OPT;
	}
	
	if(def && ajStrGetLen(def[DEF_STANDARD]))
	{
	    if(acdHelpVarResolve(&helpStr, def[DEF_STANDARD]))
	    {
		if(!ajStrToBool( helpStr, &tmpBool))
		    acdErrorAcd(pa, "Bad standard flag %S\n",
				def[DEF_STANDARD]);
	    }
	    else
	    {
		tmpBool = ajTrue;
		hlpFlag = '*';
		flagReq = ajTrue;
	    }

	    if(tmpBool)
		helpType = HELP_REQ;
	}
	
	if(pa->Assoc)
            helpType = HELP_ASS;

	acdLog("++ helpType %d\n", helpType);
	
	switch(helpType)
	{
            case HELP_APP:			/* application, do nothing */
                iattr = acdFindAttrC(acdAttrAppl, "groups");
                ajStrAssignS(&groupname, pa->AttrStr[iattr]);
                iattr = acdFindAttrC(acdAttrAppl, "documentation");
                ajStrAssignS(&acdAppldoc, pa->AttrStr[iattr]);

                igrp = ajStrFindAnyC(groupname, ",|");

                if(igrp != -1)
		  ajStrTruncateLen(&groupname, (size_t) igrp);

                ajStrFmtLower(&groupname);
                ajStrExchangeKK(&groupname, ':', '_');
                ajStrExchangeKK(&groupname, ' ', '_');

                if(acdDoGalaxy)
                {
                    ajUser("<tool id=\"EMBOSS64: %S1\" name=\"%S\" "
                           "version=\"%S\">\n"
                           "  <description>%S</description>\n"
                           "  <requirements>"
                           "<requirement type=\"package\" version=\"%S\">"
                           "emboss</requirement></requirements>\n"
                           "  <version_command>embossversion -auto"
                           "</version_command>",
                           acdProgram, acdProgram,
                           ajNamValueVersion(), acdAppldoc,
                           ajNamValueVersion());

                }

                if(acdDoXsd)
                {
                    ajUserDumpC("<?xml version=\"1.0\" encoding=\"UTF-8\" "
                                "standalone=\"yes\"?>");
                    ajUser("<xs:schema version=\"1.0\" "
                           "targetNamespace=\"http://embossws.ebi/%S\"",
                           acdProgram);
                    ajUser("  xmlns:tns=\"http://embossws.ebi/%S\"",
                           acdProgram);
                    ajUser("  xmlns:xs=\"http://www.w3.org/2001/XMLSchema\"");
                    ajUser("  xmlns:emboss=\"http://embossws.ebi/common\">");
                }
                break;

            case HELP_REQ:
                acdHelpAppend(pa, &helpReq, hlpFlag);
                acdHelpTable(pa, reqlist);
                break;

            case HELP_OPT:
                acdHelpAppend(pa, &helpOpt, hlpFlag);
                acdHelpTable(pa, optlist);
                break;

            case HELP_ADV:
                acdHelpAppend(pa, &helpAdv, hlpFlag);
                acdHelpTable(pa, advlist);
                break;

            case HELP_ASS:	   /* associated - process after the master */
                break;

            case HELP_GEN:	   /* associated - process after the app    */
                break;

            default:
                acdErrorAcd(pa, "unknown qualifier type %d in acdHelp",
                            helpType);
	}

	if(acdType[pa->Type].Section == acdSecOutput)
        {
            if(acdDoXsd)
                acdHelpXsd(pa, outlist);
            else if(acdDoGalaxy)
                acdHelpGalaxy(pa, outlist);
        }
        else if(pa->Level != ACD_APPL)
        {
            if(acdDoXsd)
                acdHelpXsd(pa, inlist);
            else if(acdDoGalaxy)
                acdHelpGalaxy(pa, inlist);
        }

	if(pa->AssocQuals)
	{
	    if(helpType == HELP_APP)
	    {
		if(acdVerbose)
		    acdHelpAssoc(pa, &helpGen, NULL);
		else
		    acdHelpAssoc(pa, &helpGen, "help");

                acdHelpAssocTable(pa, genlist);
	    }
	    else
	    {
		if(acdVerbose)
		{
		    acdHelpAssoc(pa, &helpAss, NULL);
		    acdHelpAssocTable(pa, asslist);
		}
	    }
	}
    }

    if(!acdDoXsd && !acdDoGalaxy)
    {
        /*
        ** report 1-line documentation
        ** report version (s)
         */

        acdAttrResolve(acdApplAcd, "documentation", &tmpstr);

        if(!acdAuto && !acdDoTable && ajStrGetLen(tmpstr))
        {
            ajStrFmtWrap(&tmpstr, 75);
            ajUserDumpS(tmpstr);
        }
        
        ajFmtPrintS(&tmpstr, "EMBOSS:%s", VERSION);

        if(ajStrGetLen(tmpstr) < 14)
            ajStrAppendC(&tmpstr, ".0");

        if(ajStrGetLen(acdPackVersion))
            ajFmtPrintAppS(&tmpstr, " %S:%S", acdPackName, acdPackVersion);

        acdAttrResolve(acdApplAcd, "versionnumber", &applversion);

        if(ajStrGetLen(applversion))
        {
            ajFmtPrintAppS(&tmpstr, " %S:%S", acdApplAcd->Name, applversion);
            ajStrDel(&applversion);
        }

        if(!acdDoTable)
            ajUser("Version: %S\n", tmpstr);
        ajStrDel(&tmpstr);
        ajStrDel(&applversion);

        if(flagReq)
            acdHelpShow(helpReq,
                        "Standard (Mandatory) qualifiers "
                        "(* if not always prompted)");
        else
            acdHelpShow(helpReq, "Standard (Mandatory) qualifiers");

        acdHelpTableShow(NULL, "");
        acdHelpTableShow(reqlist, "Standard (Mandatory) qualifiers");

        if(flagOpt)
            acdHelpShow(helpOpt,
                        "Additional (Optional) qualifiers "
                        "(* if not always prompted)");
        else
            acdHelpShow(helpOpt, "Additional (Optional) qualifiers");

        acdHelpTableShow(optlist, "Additional (Optional) qualifiers");
        acdHelpShow(helpAdv, "Advanced (Unprompted) qualifiers");
        acdHelpTableShow(advlist, "Advanced (Unprompted) qualifiers");

        if(acdVerbose) acdHelpShow
                           (helpAss, "Associated qualifiers");

        acdHelpShow(helpGen, "General qualifiers");

        if(acdVerbose && acdDoTable)
            acdHelpTableShow(asslist, "Associated qualifiers");

        if(acdVerbose && acdDoTable)
            acdHelpTableShow(genlist, "General qualifiers");
    }    

    if(acdDoXsd)
    {
        acdHelpXsdShow(inlist, outlist);
        ajUserDumpC("</xs:schema>");
    }
    else if(acdDoGalaxy)
    {
        acdHelpGalaxyShow(inlist, outlist);
        ajUserDumpC("");
    }
    else if(acdDoTable)
        ajUserDumpC("</table>");
    
    ajExit();

    return;
}




/* @funcstatic acdHelpAssoc ***************************************************
**
** Processes all associated qualifiers for a qualifier or for the application.
**
** If a qualifier name is given (e.g. "help") then only that qualifier
** is processed.
**
** @param [r] thys [const AcdPAcd]  ACD object
** @param [u] str [AjPStr*] Help text being built
** @param [r] name [const char*] Single name to process
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdHelpAssoc(const AcdPAcd thys, AjPStr *str, const char* name)
{
    static AjPStr line  = NULL;
    static AjPStr qname = NULL;
    static AjPStr qtype = NULL;
    static AjPStr text  = NULL;
    AcdPQual quals;
    ajint i;

    acdLog("++ acdHelpAssoc %S\n", thys->Name);

    if(thys->Level == ACD_APPL)
	quals = acdQualAppl;
    else
    {
	ajFmtPrintS(&line, "\n   \"-%S\" associated qualifiers\n",
		    thys->Name);
	ajStrAppendS(str, line);
	quals = acdType[thys->Type].Quals;
    }

    acdLog("++ type %d quals %x\n", thys->Type, quals);

    if(quals)
    {
	for(i=0; quals[i].Name; i++)
	{
	    acdLog("++ quals[%d].Name %s\n", i, quals[i].Name);

	    if(name && strcmp(name, quals[i].Name))
		continue;

	    if(thys->PNum)
		ajFmtPrintS(&qname, " -%s%d",
			    quals[i].Name, thys->PNum);
	    else
		ajFmtPrintS(&qname, " -%s", quals[i].Name);

	    ajStrAssignC(&qtype, quals[i].Type);
	    ajFmtPrintS(&line, "  %-20S %-10S ",
			qname,  qtype);
	    ajStrAssignC(&text, quals[i].Help);
	    acdTextFormat(&text);
	    ajStrFmtWrapLeft(&text, 45, 34, 0);
	    ajStrCutStart(&text, 34);
	    ajStrAppendS(&line, text);
	    ajStrAppendC(&line, "\n");
	    ajStrAppendS(str, line);
	}
    }

    return;
}




/* @funcstatic acdHelpAppend **************************************************
**
** Appends a qualifier and its help text to a help category string.
**
** @param [r] thys [const AcdPAcd]  ACD object
** @param [u] str [AjPStr*] Help text being built
** @param [r] flag [char] Flag character. Usually blank, but an asterisk
**                         is used if the status (optional/required) is
**                         uncertain.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdHelpAppend(const AcdPAcd thys, AjPStr *str, char flag)
{
    static AjPStr name    = NULL;
    static AjPStr valstr  = NULL;
    static AjPStr nostr   = NULL;
    static AjPStr nullstr = NULL;
    static AjPStr text    = NULL;
    static AjPStr line    = NULL;
    static AjPStr type    = NULL;
    AjBool boolval;
    
    AjPStr defstr;
    
    nullstr = ajStrNew();
    
    if(ajCharMatchC("list", acdType[thys->Type].Name))
	ajStrAssignC(&type, "menu");
    else
	ajStrAssignC(&type, acdType[thys->Type].Name);
    
    if(thys->DefStr)
	defstr = thys->OrigStr;
    else
	defstr = nullstr;
    
    ajStrAssignClear(&nostr);

    if(acdIsQtype(thys) &&
       (ajCharMatchC("boolean", acdType[thys->Type].Name) ||
	ajCharMatchC("toggle", acdType[thys->Type].Name) ))
    {
	if(ajStrToBool(defstr, &boolval) && boolval)
	    ajStrAssignC(&nostr, "[no]");
	defstr = nullstr;
    }
    
    ajStrAssignS(&valstr, defstr);
    
    /*
     **  warning - don't try acdVarResolve here because we have not yet
     **  read in the data and things like calculated attributes do not exist
     */
    
    if(thys->Level == ACD_PARAM)
	ajFmtPrintS(&name, "[-%S%S]", nostr, thys->Name);
    else
	ajFmtPrintS(&name, " -%S%S", nostr, thys->Name);
    
    ajFmtPrintS(&line, "%c %-20S %-10S ", flag, name, type);

    if(!acdDoTable)
    {
	acdHelpExpect(thys, ajFalse, &text);

	if(ajStrGetLen(text))
	{
	    ajStrInsertC(&text, 0, "[");
	    ajStrAppendC(&text, "] ");
	}
    }

    acdHelpText(thys, &text);

    if(ajStrGetLen(text))
       ajStrAppendC(&text, " ");

    if(!acdDoTable)
	acdHelpValid(thys, ajFalse, &text);

    ajStrRemoveWhiteSpaces(&text);
    acdTextFormat(&text);
    ajStrFmtWrapLeft(&text, 45, 34, 0);
    ajStrCutStart(&text, 34);
    ajStrAppendS(&line, text);
    ajStrAppendC(&line, "\n");
    ajStrAppendS(str, line);

    ajStrDel(&name);
    ajStrDel(&valstr);
    ajStrDel(&nostr);
    ajStrDel(&nullstr);
    ajStrDel(&text);
    ajStrDel(&line);
    ajStrDel(&type);

    return;
}




/* @funcstatic acdHelpValidAlign **********************************************
**
** Generates valid description for a alignment output
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [r] table [AjBool] True if writing acdtable HTML table
** @param [w] str [AjPStr*] Help text (if any) generated
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void acdHelpValidAlign(const AcdPAcd thys, AjBool table, AjPStr* str)
{
    AjPStr fmt = NULL;

    if(!thys)
        return;

    acdGetValueAssocDefault(thys, "aformat", &fmt);

    if(ajStrGetLen(fmt))
        ajFmtPrintAppS(str, "(default -aformat %S)", fmt);
        
    else if(table)
	ajStrAssignClear(str);

    ajStrDel(&fmt);

    return;
}




/* @funcstatic acdHelpValidReport *********************************************
**
** Generates valid description for a report output
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [r] table [AjBool] True if writing acdtable HTML table
** @param [w] str [AjPStr*] Help text (if any) generated
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void acdHelpValidReport(const AcdPAcd thys, AjBool table, AjPStr* str)
{
    AjPStr fmt = NULL;

    if(!thys)
        return;

    acdGetValueAssocDefault(thys, "rformat", &fmt);

    if(ajStrGetLen(fmt))
        ajFmtPrintAppS(str, "(default -rformat %S)", fmt);
        
    else if(table)
	ajStrAssignClear(str);

    ajStrDel(&fmt);

    return;
}




/* @funcstatic acdHelpValidSeq ************************************************
**
** Generates valid description for an input sequence type.
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [r] table [AjBool] True if writing acdtable HTML table
** @param [w] str [AjPStr*] Help text (if any) generated
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdHelpValidSeq(const AcdPAcd thys, AjBool table, AjPStr* str)
{
    if(!thys)
        return;

    if(table)
	ajStrAssignClear(str);

    return;
}




/* @funcstatic acdHelpValidSeqout *********************************************
**
** Generates valid description for an output sequence type.
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [r] table [AjBool] True if writing acdtable HTML table
** @param [w] str [AjPStr*] Help text (if any) generated
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdHelpValidSeqout(const AcdPAcd thys, AjBool table, AjPStr* str)
{
    if(!thys)
        return;

    if(table)
	ajStrAssignClear(str);

    return;
}




/* @funcstatic acdHelpValidOut ************************************************
**
** Generates valid description for an outfile type.
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [r] table [AjBool] True if writing acdtable HTML table
** @param [w] str [AjPStr*] Help text (if any) generated
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdHelpValidOut(const AcdPAcd thys, AjBool table, AjPStr* str)
{
    if(!thys)
        return;

    if(table)
	ajStrAssignClear(str);

    return;
}




/* @funcstatic acdHelpValidIn *************************************************
**
** Generates valid description for an infile type.
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [r] table [AjBool] True if writing acdtable HTML table
** @param [w] str [AjPStr*] Help text (if any) generated
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdHelpValidIn(const AcdPAcd thys, AjBool table, AjPStr* str)
{
    if(!thys)
        return;

    if(table)
	ajStrAssignClear(str);

    return;
}




/* @funcstatic acdHelpValidData ***********************************************
**
** Generates valid description for a datafile type.
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [r] table [AjBool] True if writing acdtable HTML table
** @param [w] str [AjPStr*] Help text (if any) generated
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdHelpValidData(const AcdPAcd thys, AjBool table, AjPStr* str)
{
    if(!thys)
        return;

    if(table)
	ajStrAssignClear(str);

    return;
}




/* @funcstatic acdHelpValidInt ************************************************
**
** Generates valid description for an integer type.
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [r] table [AjBool] True if writing acdtable HTML table
** @param [w] str [AjPStr*] Help text (if any) generated
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdHelpValidInt(const AcdPAcd thys, AjBool table, AjPStr* str)
{
    ajint imin;
    ajint imax;
    
    acdAttrValueStr(thys, "minimum", "$", &acdTmpStr);
    if(!ajStrToInt(acdTmpStr, &imin))
	imin = INT_MIN;
    
    acdAttrValueStr(thys, "maximum", "$", &acdTmpStr);
    if(!ajStrToInt(acdTmpStr, &imax))
	imax = INT_MAX;

    if(table)
	ajStrAssignClear(str);
    else
	ajStrAppendC(str, " (");

    if(imax != INT_MAX)
    {
	if(imin != INT_MIN)
	    ajFmtPrintAppS(str, "Integer from %d to %d", imin, imax);
	else
	    ajFmtPrintAppS(str, "Integer up to %d", imax);
    }
    else
    {
	if(imin != INT_MIN)
	    ajFmtPrintAppS(str, "Integer %d or more", imin);
	else
	    ajFmtPrintAppS(str, "Any integer value");
    }
    
    if(!table)
	ajStrAppendC(str, ")");

    return;
}




/* @funcstatic acdHelpValidFloat **********************************************
**
** Generates valid description for a floating point type.
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [r] table [AjBool] True if writing acdtable HTML table
** @param [w] str [AjPStr*] Help text (if any) generated
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdHelpValidFloat(const AcdPAcd thys, AjBool table, AjPStr* str)
{
    float vfmin;
    float vfmax;
    ajint iprec;

    acdAttrValueStr(thys, "minimum", "$", &acdTmpStr);

    if(!ajStrToFloat(acdTmpStr, &vfmin))
	vfmin = -FLT_MAX;

    acdAttrValueStr(thys, "maximum", "$", &acdTmpStr);

    if(!ajStrToFloat(acdTmpStr, &vfmax))
	vfmax = FLT_MAX;

    acdAttrValueStr(thys, "precision", "$", &acdTmpStr);

    if(!ajStrToInt(acdTmpStr, &iprec))
	iprec = 3;

    if(table)
	ajStrAssignClear(str);
    else
	ajStrAppendC(str, " (");

    if(vfmax != FLT_MAX)
    {
	if(vfmin != -FLT_MAX)
	    ajFmtPrintAppS(str, "Number from %.*f to %.*f",
			iprec, vfmin, iprec, vfmax);
	else
	    ajFmtPrintAppS(str, "Number up to %.*f", iprec, vfmax);
    }
    else
    {
	if(vfmin != -FLT_MAX)
	    ajFmtPrintAppS(str, "Number %.*f or more", iprec, vfmin);
	else
	    ajFmtPrintAppS(str, "Any numeric value");
    }

    if(!table)
	ajStrAppendC(str, ")");

    return;
}




/* @funcstatic acdHelpValidCodon **********************************************
**
** Generates valid description for a codon usage table type.
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [r] table [AjBool] True if writing acdtable HTML table
** @param [w] str [AjPStr*] Help text (if any) generated
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdHelpValidCodon(const AcdPAcd thys, AjBool table, AjPStr* str)
{
    if(!thys)
        return;

    if(table)
	ajStrAssignClear(str);

    return;
}




/* @funcstatic acdHelpValidDirlist ********************************************
**
** Generates valid description for a dirlist type.
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [r] table [AjBool] True if writing acdtable HTML table
** @param [w] str [AjPStr*] Help text (if any) generated
** @return [void]
**
** @release 1.9.0
** @@
******************************************************************************/

static void acdHelpValidDirlist(const AcdPAcd thys, AjBool table, AjPStr* str)
{
    if(!thys)
        return;

    if(table)
	ajStrAssignClear(str);

    return;
}




/* @funcstatic acdHelpValidFilelist *******************************************
**
** Generates valid description for a filelist type.
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [r] table [AjBool] True if writing acdtable HTML table
** @param [w] str [AjPStr*] Help text (if any) generated
** @return [void]
**
** @release 2.5.0
** @@
******************************************************************************/

static void acdHelpValidFilelist(const AcdPAcd thys, AjBool table, AjPStr* str)
{
    if(!thys)
        return;

    if(table)
	ajStrAssignClear(str);

    return;
}




/* @funcstatic acdHelpValidMatrix *********************************************
**
** Generates valid description for a comparison matrix type.
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [r] table [AjBool] True if writing acdtable HTML table
** @param [w] str [AjPStr*] Help text (if any) generated
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdHelpValidMatrix(const AcdPAcd thys, AjBool table, AjPStr* str)
{
    if(!thys)
        return;

    if(table)
	ajStrAssignClear(str);

    return;
}




/* @funcstatic acdHelpValidFeatout ********************************************
**
** Generates valid description for a feature output type.
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [r] table [AjBool] True if writing acdtable HTML table
** @param [w] str [AjPStr*] Help text (if any) generated
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdHelpValidFeatout(const AcdPAcd thys, AjBool table, AjPStr* str)
{
    if(!thys)
        return;

    if(table)
	ajStrAssignClear(str);

    return;
}




/* @funcstatic acdHelpValidFeatures *******************************************
**
** Generates valid description for an input feature type.
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [r] table [AjBool] True if writing acdtable HTML table
** @param [w] str [AjPStr*] Help text (if any) generated
** @return [void]
**
** @release 6.0.0
** @@
******************************************************************************/

static void acdHelpValidFeatures(const AcdPAcd thys, AjBool table, AjPStr* str)
{
    if(!thys)
        return;

    if(table)
	ajStrAssignClear(str);

    return;
}




/* @funcstatic acdHelpValidRange **********************************************
**
** Generates valid description for a sequence range.
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [r] table [AjBool] True if writing acdtable HTML table
** @param [w] str [AjPStr*] Help text (if any) generated
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdHelpValidRange(const AcdPAcd thys, AjBool table, AjPStr* str)
{
    if(!thys)
        return;

    if(table)
	ajStrAssignClear(str);

    return;
}




/* @funcstatic acdHelpValidGraph **********************************************
**
** Generates valid description for a graphics device type.
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [r] table [AjBool] True if writing acdtable HTML table
** @param [w] str [AjPStr*] Help text (if any) generated
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdHelpValidGraph(const AcdPAcd thys, AjBool table, AjPStr* str)
{
    AjPList list;
    AjPStr name = NULL;
    ajint i     = 0;

    if(!thys)
        return;

    list = ajListstrNew();

    ajGraphicsListDevices(list);

    if(table)
	ajFmtPrintAppS(str, "EMBOSS has a list of known devices, including ");
    else
	ajFmtPrintAppS(str, " (");

    while(ajListstrPop(list, &name))
    {
	if(i)
	    ajFmtPrintAppS(str, ", ");

	ajFmtPrintAppS(str, "%S", name);
	ajStrDel(&name);
	i++;
    }

    if(!table)
	ajFmtPrintAppS(str, ")");

    ajListFree(&list);

    return;
}




/* @funcstatic acdHelpValidString *********************************************
**
** Generates valid description for a string type.
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [r] table [AjBool] True if writing acdtable HTML table
** @param [w] str [AjPStr*] Help text (if any) generated
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdHelpValidString(const AcdPAcd thys, AjBool table, AjPStr* str)
{
    ajint minlen;
    ajint maxlen;
    AjBool  word;

    acdAttrValueStr(thys, "minlength", "0", &acdTmpStr);

    if(!ajStrToInt(acdTmpStr, &minlen))
	minlen = 0;
    acdAttrValueStr(thys, "maxlength", "0", &acdTmpStr);

    if(!ajStrToInt(acdTmpStr, &maxlen))
	maxlen = 0;
    acdAttrValueStr(thys, "word", "0", &acdTmpStr);

    if(!ajStrToBool(acdTmpStr, &word))
	ajFatal("Bad boolean value");
    
    if(table)
	ajStrAssignClear(str);
    else
	ajStrAppendC(str, " (");

    if(word)
	ajFmtPrintAppS(str, "Any word");
    else
    	ajFmtPrintAppS(str, "Any string");

    if(maxlen > 0)
    {
	if(minlen > 0)
	    ajFmtPrintAppS(str, " from %d to %d characters",
			   minlen, maxlen);
	else
	    ajFmtPrintAppS(str, " up to %d characters", maxlen);
    }
    else
    {
	if(minlen > 0)
	    ajFmtPrintAppS(str, " of at least %d characters", minlen);
    }
    

    acdAttrValueStr(thys, "pattern", "", &acdTmpStr);

    if(ajStrGetLen(acdTmpStr))
	ajFmtPrintAppS(str, ", matching regular expression /%S/", acdTmpStr);

    if(!table)
	ajStrAppendC(str, ")");

    return;
}




/* @funcstatic acdHelpValidRegexp *********************************************
**
** Generates valid description for a regular expression type.
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [r] table [AjBool] True if writing acdtable HTML table
** @param [w] str [AjPStr*] Help text (if any) generated
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdHelpValidRegexp(const AcdPAcd thys, AjBool table, AjPStr* str)
{
    ajint minlen;
    ajint maxlen;

    acdAttrValueStr(thys, "minlength", "0", &acdTmpStr);

    if(!ajStrToInt(acdTmpStr, &minlen))
	minlen = 0;

    acdAttrValueStr(thys, "maxlength", "0", &acdTmpStr);

    if(!ajStrToInt(acdTmpStr, &maxlen))
	maxlen = 0;

    if(table)
	ajStrAssignClear(str);
    else
	ajStrAppendC(str, " (");

    if(maxlen > 0)
    {
	if(minlen > 0)
	    ajFmtPrintS(str,
			"A regular expression pattern from %d to "
			"%d characters",
			minlen, maxlen);
	else
	    ajFmtPrintS(str,
			"A regular expression pattern up to %d characters",
			maxlen);
    }
    else
    {
	if(minlen > 0)
	    ajFmtPrintS(str,
			"A regular expression pattern of at least "
			"%d characters",
			minlen);
	else
	    ajStrAssignC(str,
		      "Any regular expression pattern is accepted");
    }

    if(!table)
	ajStrAppendC(str, ")");

    return;
}




/* @funcstatic acdHelpValidList ***********************************************
**
** Generates valid description for a list type.
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [r] table [AjBool] True if writing acdtable HTML table
** @param [w] str [AjPStr*] Help text (if any) generated
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdHelpValidList(const AcdPAcd thys, AjBool table, AjPStr* str)
{
    AjPStr codedelim   = NULL;
    AjPStr delim       = NULL;
    AjPStr value       = NULL;

    AjPStrTok handle;
    AjPStrTok codehandle;

    static AjPStr code = NULL;
    static AjPStr desc = NULL;
    static AjPStr line = NULL;

    acdAttrValueStr(thys, "delimiter", ";", &delim);

    acdAttrValueStr(thys, "value", "", &value);

    if(!ajStrGetLen(value))
	if(!acdKnownValueList(thys, &value))
	    acdError("No value defined for list");

    handle = ajStrTokenNewS(value, delim);

    if(table)
	ajFmtPrintAppS(str, "<table>");
    else
	ajFmtPrintAppS(str, " (Values: ");

    acdAttrValueStr(thys, "codedelimiter", ":", &codedelim);

    while(ajStrTokenNextFind(handle, &line))
    {
	codehandle = ajStrTokenNewS(line, codedelim);
	ajStrTokenNextParse(codehandle, &code);
	ajStrTokenNextParseS(codehandle, delim, &desc);
	acdTextTrim(&code);
	acdTextTrim(&desc);
	if(table)
	    ajFmtPrintAppS(str, "<tr><td>%S</td> <td><i>(%S)</i></td></tr>",
			   code, desc);
	else
	    ajFmtPrintAppS(str, "%S (%S); ",
			   code, desc);
	ajStrTokenDel(&codehandle);
    }

    if(table)
	ajFmtPrintAppS(str, "</table>");
    else
    {
	ajStrCutEnd(str, 2);
	ajStrAppendC(str, ")");
    }

    ajStrTokenDel(&handle);

    return;
}




/* @funcstatic acdHelpValidSelect *********************************************
**
** Generates valid description for a select type.
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [r] table [AjBool] True if writing acdtable HTML table
** @param [w] str [AjPStr*] Help text (if any) generated
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdHelpValidSelect(const AcdPAcd thys, AjBool table, AjPStr* str)
{
    AjPStr delim       = NULL;
    AjPStr value       = NULL;
    AjPStrTok handle;
    static AjPStr desc = NULL;
    static const char* white = " \t\n\r";

    acdAttrValueStr(thys, "delimiter", ";", &delim);

    acdAttrValueStr(thys, "value", "", &value);

    if(!ajStrGetLen(value))
	if(!acdKnownValueSelect(thys, &value))
	    acdError("No value defined for selection");

    handle = ajStrTokenNewS(value, delim);

    while(ajStrTokenNextFind(handle, &desc))
    {
	ajStrTrimC(&desc, white);
	if(table)
	    ajFmtPrintAppS(str, "%S<br>", desc);
	else
	{
	    if(ajStrGetLen(*str))
		ajStrAppendK(str, ' ');
	    ajFmtPrintAppS(str, "%S", desc);
	}
    }

    ajStrTokenDel(&handle);

    return;
}




/* @funcstatic acdHelpValid ***************************************************
**
** Generates help text for an ACD object using the help, info, prompt
** and code settings.
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [r] table [AjBool] True if writing acdtable HTML table
** @param [w] str [AjPStr*] Help text (if any) generated. Only written to if
**                          initially empty
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdHelpValid(const AcdPAcd thys, AjBool table, AjPStr* str)
{
    ajint i;
    AjPStr tmpstr = NULL;

    if(table && ajStrGetLen(*str))
	return;

    if(acdAttrValueStr(thys, "valid", "", &tmpstr))
    {
	if(table)
	    ajStrAppendS(str, tmpstr);
	else
	    ajFmtPrintAppS(str, "(%S)", tmpstr);

	return;
    }

    /* special processing for sequences, outseq, outfile */

    for(i=0; acdValue[i].Name; i++)
	if(ajCharMatchC(acdType[thys->Type].Name, acdValue[i].Name))
	{
	    /* Calling funclist acdValue() */
	    if(acdValue[i].Valid)
		(*acdValue[i].Valid)(thys, table, str);
	    break;
	}

    if(ajStrGetLen(*str))
	return;

    ajStrAppendC(str, acdType[thys->Type].Valid);

    return;
}




/* @funcstatic acdHelpExpectSeq ***********************************************
**
** Generates expected value description for an input sequence type.
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [r] table [AjBool] True if writing acdtable HTML table
** @param [w] str [AjPStr*] Help text (if any) generated
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdHelpExpectSeq(const AcdPAcd thys, AjBool table, AjPStr* str)
{
    if(!thys)
        return;

    if(table)
	ajFmtPrintAppS(str, "<b>Required</b>");

    return;
}




/* @funcstatic acdHelpExpectSeqout ********************************************
**
** Generates expected value description for an output sequence type.
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [r] table [AjBool] True if writing acdtable HTML table
** @param [w] str [AjPStr*] Help text (if any) generated
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdHelpExpectSeqout(const AcdPAcd thys, AjBool table, AjPStr* str)
{
    if(!thys)
        return;

    if(table)
	ajFmtPrintAppS(str, "<i>&lt;*&gt;</i>.<i>format</i>");
    else
	ajFmtPrintAppS(str, "<sequence>.<format>");

    return;
}




/* @funcstatic acdHelpExpectOut ***********************************************
**
** Generates expected value description for an outfile type.
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [r] table [AjBool] True if writing acdtable HTML table
** @param [w] str [AjPStr*] Help text (if any) generated
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdHelpExpectOut(const AcdPAcd thys, AjBool table, AjPStr* str)
{
    if(!thys)
        return;

    if(table)
	ajFmtPrintAppS(str, "<i>&lt;*&gt;</i>.%S", acdProgram);
    else
	ajFmtPrintAppS(str, "*.%S", acdProgram);

    return;
}




/* @funcstatic acdHelpExpectInt ***********************************************
**
** Generates expected value description for an integer type.
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [r] table [AjBool] True if writing acdtable HTML table
** @param [w] str [AjPStr*] Help text (if any) generated
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdHelpExpectInt(const AcdPAcd thys, AjBool table, AjPStr* str)
{
    ajint i;

    acdAttrValueStr(thys, "default", "0", &acdTmpStr);

    if(ajStrToInt(acdTmpStr, &i))
	ajFmtPrintAppS(str, "%d", i);
    else
    {
	if(table)
	    ajFmtPrintAppS(str, "<i>calculated value</i>");
    }

    return;
}




/* @funcstatic acdHelpExpectFloat *********************************************
**
** Generates expected value description for a floating point type.
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [r] table [AjBool] True if writing acdtable HTML table
** @param [w] str [AjPStr*] Help text (if any) generated
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdHelpExpectFloat(const AcdPAcd thys, AjBool table, AjPStr* str)
{
    float f;
    ajint iprec;

    acdAttrValueStr(thys, "default", "0.0", &acdTmpStr);

    if(!ajStrToFloat(acdTmpStr, &f))
	f = 0.0;

    acdAttrValueStr (thys, "precision", "3", &acdTmpStr);
    if(!ajStrToInt(acdTmpStr, &iprec))
	iprec = 3;

    if(table)
	ajFmtPrintAppS(str, "%.*f", iprec, f);
    else
	ajFmtPrintAppS(str, "%.*f", iprec, f);

    return;
}




/* @funcstatic acdHelpExpectIn ************************************************
**
** Generates expected value description for an infile type.
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [r] table [AjBool] True if writing acdtable HTML table
** @param [w] str [AjPStr*] Help text (if any) generated
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdHelpExpectIn(const AcdPAcd thys, AjBool table, AjPStr* str)
{
    if(!thys)
        return;

    if(table)
	ajFmtPrintS(str, "<b>Required</b>");

    return;
}




/* @funcstatic acdHelpExpectData **********************************************
**
** Generates expected value description for a datafile type.
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [r] table [AjBool] True if writing acdtable HTML table
** @param [w] str [AjPStr*] Help text (if any) generated
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdHelpExpectData(const AcdPAcd thys, AjBool table, AjPStr* str)
{
    if(!thys)
        return;

    if(table)
	ajFmtPrintS(str, "<i>File in the data file path</i>");

    return;
}




/* @funcstatic acdHelpExpectCodon *********************************************
**
** Generates expected value description for a codon usage table type.
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [r] table [AjBool] True if writing acdtable HTML table
** @param [w] str [AjPStr*] Help text (if any) generated
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdHelpExpectCodon(const AcdPAcd thys, AjBool table, AjPStr* str)
{
    AjPStr tmpstr = NULL;

    if(!thys)
        return;

    acdAttrResolve(thys, "name", &tmpstr);
    if(ajStrGetLen(tmpstr))
    {
	if(table)
	    ajStrAppendS(str, tmpstr);
	else
	    ajStrAppendS(str, tmpstr);
	ajStrDel(&tmpstr);

	return;
    }
    
    return;
}




/* @funcstatic acdHelpExpectDirlist *******************************************
**
** Generates expected value description for a dirlist type.
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [r] table [AjBool] True if writing acdtable HTML table
** @param [w] str [AjPStr*] Help text (if any) generated
** @return [void]
**
** @release 1.9.0
** @@
******************************************************************************/

static void acdHelpExpectDirlist(const AcdPAcd thys, AjBool table, AjPStr* str)
{
    AjPStr tmpstr = NULL;
 
    if(!thys)
        return;

    acdAttrResolve(thys, "default", &tmpstr);

    if(ajStrGetLen(tmpstr))
    {
	if(table)
	    ajStrAppendS(str, tmpstr);
	else
	    ajStrAppendS(str, tmpstr);
	ajStrDel(&tmpstr);

	return;
    }

    if(table)
	ajStrAssignC(str, DEFDLIST);
    else
	ajStrAssignC(str, DEFDLIST);

    return;
}




/* @funcstatic acdHelpExpectFilelist ******************************************
**
** Generates expected value description for a filelist type.
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [r] table [AjBool] True if writing acdtable HTML table
** @param [w] str [AjPStr*] Help text (if any) generated
** @return [void]
**
** @release 2.5.0
** @@
******************************************************************************/

static void acdHelpExpectFilelist(const AcdPAcd thys, AjBool table,
				  AjPStr* str)
{
    if(!thys)
        return;

    if(table)
	ajStrAppendC(str, "<i>comma-separated file list</i>");

    return;
}




/* @funcstatic acdHelpExpectMatrix ********************************************
**
** Generates expected value description for a comparison matrix type.
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [r] table [AjBool] True if writing acdtable HTML table
** @param [w] str [AjPStr*] Help text (if any) generated
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdHelpExpectMatrix(const AcdPAcd thys, AjBool table, AjPStr* str)
{
    if(!thys)
        return;

    if(table)
	ajStrAppendC(str, "EBLOSUM62 for protein<br>EDNAFULL for DNA");
    else
	ajStrAppendC(str, "EBLOSUM62 for protein, EDNAFULL for DNA");

    return;
}




/* @funcstatic acdHelpExpectFeatout *******************************************
**
** Generates expected value description for a feature output type.
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [r] table [AjBool] True if writing acdtable HTML table
** @param [w] str [AjPStr*] Help text (if any) generated
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdHelpExpectFeatout(const AcdPAcd thys, AjBool table, AjPStr* str)
{
    if(!thys)
        return;

    if(table)
	ajStrAppendC(str, "<i>unknown.gff</i>");
    else
	ajStrAppendC(str, "unknown.gff");

    return;
}




/* @funcstatic acdHelpExpectFeatures ******************************************
**
** Generates expected value description for an input feature type.
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [r] table [AjBool] True if writing acdtable HTML table
** @param [w] str [AjPStr*] Help text (if any) generated
** @return [void]
**
** @release 6.0.0
** @@
******************************************************************************/

static void acdHelpExpectFeatures(const AcdPAcd thys, AjBool table, AjPStr* str)
{
    if(!thys)
        return;

    if(table)
	ajFmtPrintAppS(str, "<b>Required</b>");

    return;
}




/* @funcstatic acdHelpExpectRange *********************************************
**
** Generates expected value description for a sequence range type.
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [r] table [AjBool] True if writing acdtable HTML table
** @param [w] str [AjPStr*] Help text (if any) generated
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdHelpExpectRange(const AcdPAcd thys, AjBool table, AjPStr* str)
{
    if(!thys)
        return;

    if(table)
	ajStrAppendC(str, "<i>full sequence</i>");
    else
	ajStrAppendC(str, "(full sequence)");

    return;
}




/* @funcstatic acdHelpExpectGraph *********************************************
**
** Generates expected value description for a graphics device type.
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [r] table [AjBool] True if writing acdtable HTML table
** @param [w] str [AjPStr*] Help text (if any) generated
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdHelpExpectGraph(const AcdPAcd thys, AjBool table, AjPStr* str)
{
#ifndef WIN32
#ifndef X_DISPLAY_MISSING /* X11 is available */
    const char* defdev = "x11";
#else
#ifdef PLD_png          /* if png/gd/zlib libraries available for png driver */
    const char* defdev = "png";
#else
    const char* defdev = "ps";
#endif
#endif
#else
    const char* defdev = "win3";
#endif

    if(!thys)
        return;

    if(table)
	ajFmtPrintAppS(str, "<i>EMBOSS_GRAPHICS</i> value, or %s", defdev);
    else
	ajFmtPrintAppS(str, "$EMBOSS_GRAPHICS value, or %s", defdev);
    return;
}




/* @funcstatic acdHelpExpectRegexp ********************************************
**
** Generates expected value description for a regular expression type.
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [r] table [AjBool] True if writing acdtable HTML table
** @param [w] str [AjPStr*] Help text (if any) generated
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdHelpExpectRegexp(const AcdPAcd thys, AjBool table, AjPStr* str)
{
    ajint minlen;

    acdAttrValueStr(thys, "min", "1", &acdTmpStr);

    if(!ajStrToInt(acdTmpStr, &minlen))
	minlen = 0;

    if(minlen > 0)
    {
	if(table)
	    ajStrAppendC(str, "<b>Required</b>");
    }
    else
    {
	if(table)
	    ajStrAppendC(str,
			 "<i>An empty regular expression is accepted</i>");
    }

    return;
}




/* @funcstatic acdHelpExpectString ********************************************
**
** Generates expected value description for a string type.
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [r] table [AjBool] True if writing acdtable HTML table
** @param [w] str [AjPStr*] Help text (if any) generated
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdHelpExpectString(const AcdPAcd thys, AjBool table, AjPStr* str)
{
    ajint minlen;

    acdAttrValueStr(thys, "min", "0", &acdTmpStr);

    if(!ajStrToInt(acdTmpStr, &minlen))
	minlen = 0;

    if(minlen > 0)
    {
	if(table)
	    ajStrAppendC(str, "<b>Required</b>");
    }
    else
    {
	if(table)
	    ajStrAppendC(str, "&nbsp;");
    }

    return;
}




/* @funcstatic acdHelpExpect **************************************************
**
** Generates expected value text for an ACD object code settings.
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [r] table [AjBool] True if writing acdtable HTML table
** @param [w] str [AjPStr*] Help text (if any) generated
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdHelpExpect(const AcdPAcd thys, AjBool table, AjPStr* str)
{
    ajint i;
    AjPStr tmpstr = NULL;

    if(ajStrGetLen(*str))
	return;
    
    if(!thys->AssocQuals && !acdDoGalaxy)
	if(acdAttrValueStr(thys, "expected", "", str))
	    return;
    
    if(acdAttrValueStr(thys, "default", "", str))
    {
        if(acdDoGalaxy)
        {
            tmpstr = ajStrNewS(*str);

            if(!acdVarSimpleResolve(tmpstr, str))
                ajStrAssignS(str, tmpstr);

            ajStrDel(&tmpstr);
        }

        return;
    }

    /* special processing for sequences, outseq, outfile */
    
    for(i=0; acdValue[i].Name; i++)
	if(ajCharMatchC(acdType[thys->Type].Name, acdValue[i].Name))
	{
	    /* Calling funclist acdValue() */
	    if(acdValue[i].Expect)
		(*acdValue[i].Expect)(thys, table, str);
	    break;
	}


    if(ajStrGetLen(*str))
	return;
    
    ajStrAppendS(str, thys->DefStr[DEF_DEFAULT]);
    
    if(ajStrGetLen(*str))
	return;

    if(table)
	ajStrAppendC(str, "&nbsp;");
    
    return;
}




/* @funcstatic acdHelpText ****************************************************
**
** Generates help text for an ACD object using the help, info, prompt
** and code settings.
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [w] str [AjPStr*] Help text (if any) generated
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdHelpText(const AcdPAcd thys, AjPStr* str)
{
    AjPStr prompt;
    AjPStr info;
    AjPStr code;
    AjPStr help;
    static AjPStr msg = NULL;
    
    if(thys->DefStr)
    {
	prompt = thys->DefStr[DEF_PROMPT];
	info = thys->DefStr[DEF_INFO];
	code = thys->DefStr[DEF_CODE];
	help = thys->DefStr[DEF_HELP];
	
	if(ajStrGetLen(help))
	    ajStrAssignS(&msg, help);
	else if(ajStrGetLen(code))
	    acdCodeGet(code, &msg);
	else if(ajStrGetLen(info))
	    ajStrAssignS(&msg, info);
	else if(ajStrGetLen(prompt))
	    ajStrAssignS(&msg, prompt);
	else if(acdType[thys->Type].HelpSet)
	    (*acdType[thys->Type].HelpSet)(thys, &msg);
	else
	{
	    if(!acdHelpCodeDef(thys, &msg))
	    {
		ajStrAssignResC(&msg, 512, "");
		if(thys->Assoc)
		    ajFmtPrintS(&msg, "%s value",
				acdType[thys->Type].Name);
		else
		    ajFmtPrintS(&msg, "(no help text) %s value",
				acdType[thys->Type].Name);
	    }
	}
	
	acdVarResolve(&msg);
	ajStrAppendS(str, msg);
	ajStrDel(&msg);
    }

    return;
}




/* @funcstatic acdHelpShow ****************************************************
**
** Prints the qualifier category and the help for any
** qualifiers in that category (or "(none)" if there are none).
**
** @param [r] str [const AjPStr] Help text (if any)
** @param [r] title [const char*] Title line for this call
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdHelpShow(const AjPStr str, const char* title)
{
    if(acdDoTable)
	return;

    if(!ajStrGetLen(str))
    {
	ajUser("   %s: (none)", title);
	return;
    }

    ajUser("   %s:", title);
    ajUserDumpS(str);

    return;
}




/* @funcstatic acdHelpTableShow ***********************************************
**
** Prints the qualifier category and the help for any
** qualifiers in that category (or "(none)" if there are none).
**
** @param [r] tablist [const AjPList] Help text (if any).
** @param [r] title [const char*] Title line for this call
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdHelpTableShow(const AjPList tablist, const char* title)
{
    AcdPTableItem item;
    AjIList iter = NULL;
    ajuint nitem = 0;

    if(!acdDoTable)
	return;

    if(!tablist)
    {
        ajUserDumpC("<tr bgcolor=\"#FFFFCC\">"); /* was #FFFFD0 */
        ajUserDumpC("<th align=\"left\">Qualifier</th>");
        ajUserDumpC("<th align=\"left\">Type</th>");
        ajUserDumpC("<th align=\"left\">Description</th>");
        ajUserDumpC("<th align=\"left\">Allowed values</th>");
        ajUserDumpC("<th align=\"left\">Default</th>");
        ajUserDumpC("</tr>\n");
        return;
    }

    /* new section */
    
    if(title){
        ajUserDumpC("<tr bgcolor=\"#FFFFCC\">"); /* was #FFFFD0 */
        ajUser("<th align=\"left\" colspan=5>%s</th>", title);
        ajUserDumpC("</tr>\n");
    }

    iter = ajListIterNewread(tablist);
    while((item = ajListIterGet(iter)))
    {
        ajUserDumpC("<tr bgcolor=\"#FFFFCC\">");
        if(ajStrGetLen(item->Title))
        {
            ajUser("<td align=\"left\" colspan=5>%S</td>", item->Title);
        }
        else
        {
            nitem++;
            acdTextTrim(&item->Help);
            ajUser("<td>%S</td>", item->Qual);
            ajUser("<td>%S</td>", item->Type);
            ajUser("<td>%S</td>", item->Help);
            ajUser("<td>%S</td>", item->Valid);
            ajUser("<td>%S</td>", item->Expect);
        }
        ajUserDumpC("</tr>\n");
    }

    if(!nitem || !ajListGetLength(tablist))
    {
        ajUserDumpC("<tr>");
        ajUserDumpC("<td colspan=5>(none)</td>");
        ajUserDumpC("</tr>\n");
    }

    ajListIterDel(&iter);

    return;
}




/* @funcstatic acdHelpXsdShow *************************************************
**
** Prints the qualifier category and the help for any
** qualifiers in that category (or "(none)" if there are none).
**
** @param [r] inlist [const AjPList] Help text (if any).
** @param [r] outlist [const AjPList] Help text (if any).
** @return [void]
**
** @release 6.1.0
** @@
******************************************************************************/

static void acdHelpXsdShow(const AjPList inlist, const AjPList outlist)
{
    AcdPXsdItem item;
    AjIList iter = NULL;
    AjPStrTok handle = NULL;
    AjPStr rest = NULL;
    AjPStr word = NULL;
    AjPStr tmpstr = NULL;

    if(!acdDoXsd)
	return;

    ajUserDumpC("  <xs:import namespace=\"http://embossws.ebi/common\"");
    ajUserDumpC("    schemaLocation=\"sequence_input.xsd\" />");
    ajUserDumpC("  <xs:complexType name=\"appInputs\">");
    ajUserDumpC("    <xs:sequence>");

    if(ajListGetLength(inlist))
    {
        iter = ajListIterNewread(inlist);
        while((item = ajListIterGet(iter)))
        {
            acdTextTrim(&item->Annotation);

            /* Need specific strings for each datatype */
            /* add a block here as a temporary solution */

            /*
            ** Later create a functions for each datatype 
            ** and a default function for general datatypes
            */

            /*
            ** Datatypes with more than one value - sequences see above, others
            ** may explicitly write their choices
            */
#if 0
            /*
            //ajUser("      <xs:choice id=\"%S\" "
            //        "minOccurs=\"0\" maxOccurs=\"1\">",
            //        item->Qual);
            //ajUser("      </xs:choice>");
            */
#endif

            /*
            ** sequences: 1 for required values - e.g. parameters - 0 for
            ** others
            */            

            if (ajStrMatchC(item->Type, "long"))
            {
                ajUser("      <xs:element name=\"%S\"",
                       item->Qual);
                if(ajStrGetLen(item->Relation))
                {
                    handle = ajStrTokenNewC(item->Relation, "|");
                    while(ajStrTokenNextParse(handle, &tmpstr))
                    {
                        ajStrExtractWord(tmpstr, &rest, &word);
                        ajUser("    sawsdl:modelReference="
                               "\"http://purl.org%S\"",
                               word);
                    }
                    ajStrTokenDel(&handle);
                }
                ajUser("        minOccurs=\"1\">");
                ajUser("        <xs:annotation>");
                ajUser("          <xs:documentation>");
                ajUser("            %S", item->Annotation);
                ajUser("          </xs:documentation>");
                ajUser("        </xs:annotation>");
                ajUser("        <xs:simpleType>");
                ajUser("          <xs:restriction base=\"xs:long\">");
                ajUser("            <xs:minInclusive value=\"1\">");
                /* TODO actual min value */
                ajUser("            </xs:minInclusive>");
                ajUser("          </xs:restriction>");
                ajUser("        </xs:simpleType>");
            }
            else if (ajStrMatchC(item->Type, "seqall")){
                ajUser("      <xs:element name=\"%S\" type=\"emboss:"
                       "sequenceInput\"",
                        item->Qual);
                if(ajStrGetLen(item->Relation))
                {
                    handle = ajStrTokenNewC(item->Relation, "|");
                    while(ajStrTokenNextParse(handle, &tmpstr))
                    {
                        ajStrExtractWord(tmpstr, &rest, &word);
                        ajUser("    sawsdl:modelReference="
                               "\"http://purl.org%S\"",
                               word);
                    }
                    ajStrTokenDel(&handle);
                }
                ajUser("        minOccurs=\"1\">");
                ajUser("        <xs:annotation>");
                ajUser("          <xs:documentation>");
                ajUser("            %S", item->Annotation);
                ajUser("          </xs:documentation>");
                ajUser("        </xs:annotation>");                
            }
            else
            {
                ajUser("      <xs:element name=\"%S\" type=\"xs:%S\"",
                       item->Qual, item->Type);
                        
                if(ajStrGetLen(item->Relation))
                {
                    handle = ajStrTokenNewC(item->Relation, "|");
                    while(ajStrTokenNextParse(handle, &tmpstr))
                    {
                        ajStrExtractWord(tmpstr, &rest, &word);
                        ajUser("    sawsdl:modelReference="
                               "\"http://purl.org%S\"",
                               word);
                    }
                    ajStrTokenDel(&handle);
                }
                ajUser("        minOccurs=\"1\">");
                ajUser("        <xs:annotation>");
                ajUser("          <xs:documentation>");
                ajUser("            %S", item->Annotation);
                ajUser("          </xs:documentation>");
                ajUser("        </xs:annotation>");
            }
            ajUser("      </xs:element>");
        }
    }

    ajListIterDel(&iter);
    ajUserDumpC("    </xs:sequence>");
    ajUserDumpC("  </xs:complexType>");
    
    ajUserDumpC("  <xs:complexType name=\"appResults\">");
    ajUserDumpC("    <xs:sequence>");

    if(ajListGetLength(outlist))
    {
        iter = ajListIterNewread(outlist);

        while((item = ajListIterGet(iter)))
        {
            if (ajStrMatchC(item->Type, "seqoutall"))
            {
                ajUser("      <xs:element name=\"%S\" type=\"xs:string\"",
                       item->Qual, item->Type);
            }
            else
            {
                ajUser("      <xs:element name=\"%S\" type=\"xs:%S\"",
                       item->Qual, item->Type);
            }

            if(ajStrGetLen(item->Relation))
            {
                ajUser("relation: '%S'", item->Relation);
                handle = ajStrTokenNewC(item->Relation, "|");
                while(ajStrTokenNextParse(handle, &tmpstr))
                {
                    ajStrExtractWord(tmpstr, &rest, &word);
                    ajUser("    sawsdl:modelReference=\"http://purl.org%S\"",
                           word);
                }
                ajStrTokenDel(&handle);
            }
            ajUser("        minOccurs=\"1\">");
            ajUser("        <xs:annotation>");
            ajUser("          <xs:documentation>");
            ajUser("            %S", item->Annotation);
            ajUser("          </xs:documentation>");
            ajUser("        </xs:annotation>");
            ajUser("    </xs:element>");
        }
    }

    ajUserDumpC("    </xs:sequence>");
    ajUserDumpC("  </xs:complexType>");

    ajListIterDel(&iter);
    ajStrDel(&tmpstr);
    ajStrDel(&rest);
    ajStrDel(&word);

    return;
}




/* @funcstatic acdHelpGalaxyShow **********************************************
**
** Prints the qualifier category and the help for any
** qualifiers in that category (or "(none)" if there are none).
**
** @param [r] inlist [const AjPList] Help text (if any).
** @param [r] outlist [const AjPList] Help text (if any).
** @return [void]
**
** @release 6.3.0
** @@
******************************************************************************/

static void acdHelpGalaxyShow(const AjPList inlist, const AjPList outlist)
{
    AcdPGalaxyItem item;
    AjIList iter = NULL;
/*    AjPStrTok handle = NULL;*/
    AjPStr rest = NULL;
    AjPStr word = NULL;
    AjPStr tmpstr = NULL;
    AjPStr galaxyvar = NULL;
    ajuint ninputs = 0;
    ajuint noutputs = 0;
    ajuint i;
    AjPStrTok handle = NULL;

    AjPStr galaxyin = NULL;
    AjPStr galaxyout = NULL;
    AjPStr fmtname = NULL;
    AjPStr fmtdesc = NULL;

    const char* acdinputs[] = {
        "sequence", "seqall", "seqset", "seqsetall",
        "features",
        NULL
    };
    
    const char* acdoutputs[] = {
        "seqout", "seqoutall", "seqoutset",
        "align", "report", "featout",
        "graph", "xygraph",
        NULL
    };
    
    const char* acdoutformats[] = {
        "seqout", "seqoutall", "seqoutset",
        "align",
        "gff:GFF,pir:PIR,swiss:SWISS,excel:Excel (TAB Delimited)",
        "featout",
        "png:PNG", "png:PNG",
        NULL
    };
    
    const char* acdoutfmtqual[] = {
        "format", "format", "format",
        "aformat",
        "rformat",
        "fformat",
        "", "",
        NULL
    };
    
    if(!acdDoGalaxy)
	return;

    if(!acdGalaxyCmdStr)
        acdGalaxyCmdStr = ajStrNewS(acdProgram);

    if(ajListGetLength(inlist))
    {
        iter = ajListIterNewread(inlist);
        while((item = ajListIterGet(iter)))
        {
            acdTextTrim(&item->Annotation);

/*
            ajUser("AcdSGalaxyItem input");
            ajUser("        qual: '%S'", item->Qual);
            ajUser("        type: '%S'", item->Type);
            ajUser("  annotation: '%S'", item->Annotation);
            ajUser("    relation: '%S'", item->Relation);
            ajUser("       valid: '%S'", item->Valid);
            ajUser("      expect: '%S'", item->Expect);
            ajUser("    required: '%B'", item->Required);
            ajUser("    optional: '%B'", item->Optional);
*/

/*
** test Type
** use to select format, input1/output1 name, type
** need for output format (note: this is an input!)
** set size=4 for int or float
** type text for string, integer for int, float for float
*/
            for(i=0; acdinputs[i]; i++)
                if(ajStrMatchC(item->Type, acdinputs[i])) break;

            if(acdinputs[i])
            {
                ajFmtPrintAppS(&galaxyin,
                               "    <param format=\"data\" name=\"input%u\" "
                               "type=\"data\">\n",
                               ++ninputs);
                ajFmtPrintAppS(&galaxyin,
                               "      <label>On query</label>\n");
                ajFmtPrintAppS(&galaxyin,
                               "    </param>\n");
                ajFmtPrintAppS(&acdGalaxyCmdStr,
                               " -%S $input%u",
                               item->Qual, ninputs);
            }
            else if(ajStrMatchC(item->Expect, "<b>Required</b>"))
            {
                ajFmtPrintAppS(&galaxyin,
                               "    <param format=\"data\" name=\"%S\" "
                               "type=\"data\">\n",
                               item->Qual);
                ajFmtPrintAppS(&galaxyin,
                               "      <label>On query</label>\n");
                ajFmtPrintAppS(&galaxyin,
                               "    </param>\n");
                ajFmtPrintAppS(&acdGalaxyCmdStr,
                               " -%S $%S",
                               item->Qual, item->Qual);
            }
            else
            {
                if(ajStrMatchC(item->Type, "string"))
                    ajFmtPrintAppS(&galaxyin,
                                   "    <param name=\"%S\" size=\"50\" "
                                   "type=\"text\" value=\"%S\">\n",
                                   item->Qual, item->Expect);
                else if(ajStrMatchC(item->Type, "boolean") ||
                        ajStrMatchC(item->Type, "toggle"))
                {
                     ajFmtPrintAppS(&galaxyin,
                                       "    <param name=\"%S\" "
                                       "type=\"boolean\" value=\"%S\">\n",
                                       item->Qual, item->Expect);
                }
                else if(ajStrMatchC(item->Type, "float") ||
                        ajStrMatchC(item->Type, "double"))
                    ajFmtPrintAppS(&galaxyin,
                                   "    <param name=\"%S\" "
                                   "type=\"float\" value=\"%S\">\n",
                                   item->Qual, item->Expect);
                else if(ajStrMatchC(item->Type, "integer") ||
                        ajStrMatchC(item->Type, "long"))
                    ajFmtPrintAppS(&galaxyin,
                                   "    <param name=\"%S\" "
                                   "type=\"integer\" value=\"%S\">\n",
                                   item->Qual, item->Expect);
                else if(ajStrMatchC(item->Type, "select") ||
                        ajStrMatchC(item->Type, "list"))
                    ajFmtPrintAppS(&galaxyin,
                                   "    <param name=\"%S\" "
                                   "type=\"select\" value=\"%S\">\n",
                                   item->Qual, item->Expect);
                else
                    ajFmtPrintAppS(&galaxyin,
                                   "    <param name=\"%S\" size=\"50\" "
                                   "type=\"text\" value=\"%S\">\n",
                                   item->Qual, item->Expect);

                ajFmtPrintAppS(&galaxyin,
                               "      <label>%S</label>\n",
                               item->Prompt);
                ajFmtPrintAppS(&galaxyin,
                               "    </param>\n");

                ajStrDel(&galaxyvar);

                if(!ajStrGetLen(galaxyvar))
                    galaxyvar = item->Qual;

                ajFmtPrintAppS(&acdGalaxyCmdStr,
                               " -%S $%S",
                               item->Qual, galaxyvar);
            }



            /* Need specific strings for each datatype */
            /* add a block here as a temporary solution */

            /*
            ** sequences: 1 for required values - e.g. parameters - 0 for
            ** others
            */            

            /*
            if (ajStrMatchC(item->Type, "long"))
            {
                ajUserDumpC("long");
            }
            else if (ajStrMatchC(item->Type, "seqall")){
                ajUserDumpC("seqall");                
            }
            else
            {
                ajUserDumpC("other");
            }
            ajUserDumpC("...initemdone");
            */            
        }
    }

    ajListIterDel(&iter);
    if(ajListGetLength(outlist))
    {
        iter = ajListIterNewread(outlist);

        while((item = ajListIterGet(iter)))
        {
            for(i=0; acdoutputs[i]; i++)
                if(ajStrMatchC(item->Type, acdoutputs[i])) break;

            if(acdoutputs[i])
            {
                if(ajCharMatchC(acdoutputs[i], "graph") ||
                   ajCharMatchC(acdoutputs[i], "xygraph"))
                {
                    ajFmtPrintAppS(&acdGalaxyCmdStr,
                                   " -%S png -goutfile $goutfile",
                                   item->Qual);
                    ajFmtPrintAppS(&galaxyout,
                               "    <data format=\"%s\" "
                                   "name=\"goutfile%u\"/>\n",
                               acdoutformats[i], acdProgram, ++noutputs);
                }
                else if(ajCharMatchC(acdoutputs[i], "report"))
                {
                    ajFmtPrintAppS(&galaxyout,
                               "    <data format=\"%S\" name=\"outfile%u\"/>\n",
                               acdProgram, ++noutputs);
                    if(ajCharMatchC(acdoutputs[i], "report"))
                    {
                        ajFmtPrintAppS(&galaxyin,
                                       "    <param name=\"out_format%u\" "
                                       "type=\"select\">\n",
                                       noutputs);
                        ajFmtPrintAppS(&galaxyin,
                                       "      <label>Output format</label>\n");
                        

                        ajStrTokenAssigncharC(&handle, acdoutformats[i], ":,");
                        while(ajStrTokenNextParse(handle, &fmtname))
                        {
                            ajStrTokenNextParse(handle, &fmtdesc);
                            if(ajStrMatchC(fmtname, "gff2"))
                                ajFmtPrintAppS(&galaxyin,
                                               "      <option value=\"%S\" "
                                               "selected=\"true\">%S</option>"
                                               "\n",
                                           fmtname, fmtdesc);
                            else
                                ajFmtPrintAppS(&galaxyin,
                                               "      <option value=\"%S\">"
                                               "%S</option>\n",
                                               fmtname, fmtdesc);
                        }

                        ajFmtPrintAppS(&galaxyin,
                                       "    </param>");
                        ajFmtPrintAppS(&acdGalaxyCmdStr,
                                       " -rformat $out_format%u",
                                       noutputs);
                    }
                }
                else
                {
                    ajFmtPrintAppS(&galaxyout,
                               "    <data format=\"%S\" name=\"outfile%u\"/>\n",
                               acdProgram, ++noutputs);
                    if(ajCharMatchC(acdoutputs[i], "report"))
                    {
                        ajFmtPrintAppS(&galaxyin,
                                       "    <param name=\"out_format%u\" "
                                       "type=\"select\">\n",
                                       noutputs);
                        ajFmtPrintAppS(&galaxyin,
                                       "      <label>Output format</label>\n");
                        

                        ajStrTokenAssigncharC(&handle, acdoutformats[i], ":,");
                        while(ajStrTokenNextParse(handle, &fmtname))
                        {
                            ajStrTokenNextParse(handle, &fmtdesc);
                            if(ajStrMatchS(fmtname, item->Expect))
                                ajFmtPrintAppS(&galaxyin,
                                               "      <option value=\"%s\" "
                                               "selected=\"true\">%s</option>"
                                               "\n",
                                           fmtname, fmtdesc);
                            else
                                ajFmtPrintAppS(&galaxyin,
                                               "      <option value=\"%s\">"
                                               "%s</option>\n",
                                               fmtname, fmtdesc);
                        }

                        ajFmtPrintAppS(&galaxyin,
                                       "    </param>");
                        ajFmtPrintAppS(&acdGalaxyCmdStr, " -%s $out_format%u",
                                       acdoutfmtqual[i], noutputs);
                    }
                }
            }
            else 
            {
                /* use type=file or type=data (which?) */
                ajFmtPrintAppS(&galaxyout,
                               "    <param format=\"%S\" name=\"%S1\" "
                               "type=\"file\">\n"
                               "      <label>%S</label>\n"
                               "    </param>\n",
                               item->Qual, item->Qual,
                               item->Annotation);
            }

/*
            ajUser("AcdSGalaxyItem output\n");
            ajUser("        qual: '%S'", item->Qual);
            ajUser("        type: '%S'", item->Type);
            ajUser("  annotation: '%S'", item->Annotation);
            ajUser("    relation: '%S'", item->Relation);
            ajUser("       valid: '%S'", item->Valid);
            ajUser("      expect: '%S'", item->Expect);
            ajUser("    required: '%B'", item->Required);
            ajUser("    optional: '%B'", item->Optional);
*/

/*
            if (ajStrMatchC(item->Type, "seqoutall"))
            {
                ajUserDumpC("seqoutall");
            }
            else
            {
                 ajUserDumpC("other");
            }

            if(ajStrGetLen(item->Relation))
            {
                handle = ajStrTokenNewC(item->Relation, "|");
                while(ajStrTokenNextParse(handle, &tmpstr))
                {
                    ajStrExtractWord(tmpstr, &rest, &word);
                    ajUserDumpC("");
                }
                ajStrTokenDel(&handle);
            }
            ajUserDumpC("...outitemdone");
*/
        }
    }


    ajUser("  <command>%S -auto</command>", acdGalaxyCmdStr);

    ajUserDumpC("  <inputs>");
    ajUserDumpS(galaxyin);
    ajUserDumpC("  </inputs>");

    ajUserDumpC("  <outputs>");
    ajUserDumpS(galaxyout);
    ajUserDumpC("  </outputs>");

    /*
    ajUser("  <tests>");
    */
    /* for each QA test: */

    /*ajUser("    <test>\n"
           "      <param name=\"input1\" value=\"2.fasta\"/>\n"
           "      <param name=\"minlen\" value=\"6\"/>\n"
           "      <param name=\"out_format1\" value=\"excel\"/>\n"
           "      <output name=\"out_file1\" "
           "file=\"emboss_antigenic_out.tabular\"/>\n"
           "    </test>"
           );
    */
    
    /* end of QA tests */

/*
  ajUser("  </tests>\n");
*/

    ajUser("  <code file=\"emboss_format_corrector.py\" />\n"
           "  <help>\n"
           "\n"
           "    You can view the original documentation here_.\n"
           "\n"    
           "    .. _here: http://emboss.sourceforge.net/apps/release/6.4/"
           "emboss/apps/%S.html\n"
           "  </help>\n"
           "</tool>",
           acdProgram);

    ajListIterDel(&iter);

    ajStrDel(&tmpstr);
    ajStrDel(&rest);
    ajStrDel(&word);
    ajStrDel(&fmtname);
    ajStrDel(&fmtdesc);

    ajStrTokenDel(&handle);

    return;
}




/* @funcstatic acdHelpAssocTable **********************************************
**
** Appends an associated qualifier and its help text to the table list.
**
** @param [r] thys [const AcdPAcd]  ACD object
** @param [u] tablist [AjPList] Help text list being built
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdHelpAssocTable(const AcdPAcd thys, AjPList tablist)
{
    AcdPTableItem item;

    AcdPQual quals;
    ajint i;
    AcdPAcd pa;

   if(!acdDoTable)
	return;

    acdLog("++ acdHelpAssocTable %S\n", thys->Name);

    if(thys->Level == ACD_APPL)
    {
        quals = acdQualAppl;
    }
    else
    {
        AJNEW0(item);
        ajFmtPrintS(&item->Title, "\"-%S\" associated %s qualifiers\n",
		    thys->Name,acdType[thys->Type].Name);
        ajListPushAppend(tablist, item);
	quals = acdType[thys->Type].Quals;
    }

    acdLog("++ type %d quals %x\n", thys->Type, quals);

    if(quals)
    {
        i=0;
        for(pa=thys->AssocQuals; pa && pa->Assoc; pa=pa->Next)
	{
            acdLog("++ assoc[%d].Name %s\n", i, quals[i].Name);
            AJNEW0(item);

            if(thys->PNum)
                ajFmtPrintS(&item->Qual, " -%s%d<br>-%s_%S",
                            quals[i].Name, pa->PNum,
                            quals[i].Name, thys->Name);
            else
                ajFmtPrintS(&item->Qual, " -%s", quals[i].Name);

            ajStrAssignC(&item->Type, quals[i].Type);
            ajStrAssignC(&item->Help, quals[i].Help);
            acdHelpValid(pa, ajTrue, &item->Valid);
            acdHelpExpect(pa, ajTrue, &item->Expect);
            ajListPushAppend(tablist, item);
            i++;
        }
    }

    return;
}




/* @funcstatic acdHelpTable ***************************************************
**
** Appends a qualifier and its help text to the table list.
**
** @param [r] thys [const AcdPAcd]  ACD object
** @param [u] tablist [AjPList] Help text list being built
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdHelpTable(const AcdPAcd thys, AjPList tablist)
{    
    AcdPTableItem item;
    
    static AjPStr nostr   = NULL;
    static AjPStr nullstr = NULL;
    static AjPStr type    = NULL;
    AjBool boolval;
    
    AjPStr defstr;
    
    if(!acdDoTable)
	return;
    
    AJNEW0(item);
    
    if(!nullstr)
	nullstr = ajStrNew();
    
    if(thys->DefStr)
	defstr = thys->OrigStr;
    else
	defstr = nullstr;
    
    ajStrAssignClear(&nostr);
    if(acdIsQtype(thys) &&
       (ajCharMatchC("boolean", acdType[thys->Type].Name) ||
	ajCharMatchC("toggle", acdType[thys->Type].Name) ))
    {
	if(ajStrToBool(defstr, &boolval))
	{
	    if(boolval)
		ajStrAssignC(&nostr, "[no]");

	    ajFmtPrintS(&item->Expect, "%B", boolval);
	}
	else
	    if(!ajStrGetLen(defstr))
		ajFmtPrintS(&item->Expect, "%B", ajFalse);

	defstr = nullstr;
    }
    
    if(thys->Level == ACD_PARAM)
	ajFmtPrintS(&item->Qual, "[-%S%S]<br>(Parameter %d)",
		    nostr, thys->Name, thys->PNum);
    else
	ajFmtPrintS(&item->Qual, "-%S%S", nostr, thys->Name);
    
    ajStrAssignC(&type, acdType[thys->Type].Name);
    
    acdHelpExpect(thys, ajTrue, &item->Expect);
    
    /*
     **  warning - don't try acdVarResolve here because we have not yet
     **  read in the data and things like calculated attributes do not exist
     */
    
    ajStrAssignC(&item->Type, acdType[thys->Type].Name);
    acdHelpValid(thys, ajTrue, &item->Valid);
    acdHelpText(thys, &item->Help);
    
    ajListPushAppend(tablist, item);
    
    return;
}




/* @funcstatic acdHelpXsd *****************************************************
**
** Appends a qualifier and its help text to the XSD list.
**
** @param [r] thys [const AcdPAcd]  ACD object
** @param [u] tablist [AjPList] Xsd list being built
** @return [void]
**
** @release 6.1.0
** @@
******************************************************************************/

static void acdHelpXsd(const AcdPAcd thys, AjPList tablist)
{    
    AcdPXsdItem item;
    
    static AjPStr nostr   = NULL;
    static AjPStr nullstr = NULL;
    AjBool boolval;
    AjBool tmpBool;
    AjPStr helpStr = NULL;

    AjPStr defstr = NULL;
    AjPStr* def = NULL;

    if(!acdDoXsd)
	return;

    /* skip associated qualifiers:
    ** handled separatedly in XSD for types that use them
    ** Note: this also excludes application associated general qualifiers
    **       -auto -verbose -filter and so on
    */

    if(thys->Assoc)
        return;
    
    AJNEW0(item);
    if(!nullstr)
	nullstr = ajStrNew();
    
    if(thys->DefStr)
	defstr = thys->OrigStr;
    else
	defstr = nullstr;
    
    ajStrAssignClear(&nostr);
    if(acdIsQtype(thys))
    {
        if((ajCharMatchC("boolean", acdType[thys->Type].Name) ||
            ajCharMatchC("toggle", acdType[thys->Type].Name) ))
        {
            if(ajStrToBool(defstr, &boolval))
            {
                if(boolval)
                    ajStrAssignC(&nostr, "no");

                ajFmtPrintS(&item->Expect, "%B", boolval);
            }
            else
                if(!ajStrGetLen(defstr))
                    ajFmtPrintS(&item->Expect, "%B", ajFalse);
        }
    
	def = thys->DefStr;
	
	if(def && ajStrGetLen(def[DEF_ADDITIONAL]))
	{
	    if(acdHelpVarResolve(&helpStr, def[DEF_ADDITIONAL]))
	    {
		if(!ajStrToBool(helpStr, &tmpBool))
		    acdErrorAcd(thys, "Bad additional flag %S\n",
				def[DEF_ADDITIONAL]);
	    }
	    else
	    {
		tmpBool = ajTrue;
	    }

	    if(tmpBool)
		item->Optional = ajTrue;
	}
	
	if(def && ajStrGetLen(def[DEF_STANDARD]))
	{
	    if(acdHelpVarResolve(&helpStr, def[DEF_STANDARD]))
	    {
		if(!ajStrToBool( helpStr, &tmpBool))
		    acdErrorAcd(thys, "Bad standard flag %S\n",
				def[DEF_STANDARD]);
	    }
	    else
		tmpBool = ajTrue;

	    if(tmpBool)
		item->Required = ajTrue;
	}
	
    }

    ajStrAssignC(&item->Type, acdType[thys->Type].Name);
    ajStrAssignS(&item->Qual, thys->Name);
    acdHelpExpect(thys, ajTrue, &item->Expect);
    
    /*
     **  warning - don't try acdVarResolve here because we have not yet
     **  read in the data and things like calculated attributes do not exist
     */
    
    acdHelpValid(thys, ajTrue, &item->Valid);
    acdHelpText(thys, &item->Annotation);
    acdAttrToStr(thys, "relation", "", &item->Relation);
    ajListPushAppend(tablist, item);
    ajStrDel (&helpStr);

    return;
}




/* @funcstatic acdHelpGalaxy **************************************************
**
** Appends a qualifier and its help text to the Galaxy list.
**
** @param [u] thys [AcdPAcd]  ACD object, may have prompt set
** @param [u] tablist [AjPList] Galaxy list being built
** @return [void]
**
** @release 6.3.0
** @@
******************************************************************************/

static void acdHelpGalaxy(AcdPAcd thys, AjPList tablist)
{    
    AcdPGalaxyItem item;
    
    static AjPStr nostr   = NULL;
    static AjPStr nullstr = NULL;
    AjBool boolval;
    AjBool tmpBool;
    AjPStr helpStr = NULL;
    const AjPStr prompt = NULL;
    AjPStr defstr = NULL;
    AjPStr* def = NULL;
    ajuint itype;
    AjPStr galaxyExpectStr = NULL;

    ajint ipos;
    ajint cpos;
    ajint bpos;
    ajint qpos;

    if(!acdDoGalaxy)
	return;

    /* associated qualifiers:*/
       if(thys->Assoc)
           return;
    
    AJNEW0(item);
    if(!nullstr)
	nullstr = ajStrNew();
    
    if(thys->DefStr)
	defstr = thys->OrigStr;
    else
	defstr = nullstr;
    
    ajStrAssignClear(&nostr);
    if(acdIsQtype(thys))
    {
        if((ajCharMatchC("boolean", acdType[thys->Type].Name) ||
            ajCharMatchC("toggle", acdType[thys->Type].Name) ))
        {
            if(ajStrToBool(defstr, &boolval))
            {
                if(boolval)
                    ajStrAssignC(&item->Expect, "Yes");
                else
                    ajStrAssignC(&item->Expect, "No");
            }
            else
                if(!ajStrGetLen(defstr))
                    ajFmtPrintS(&item->Expect, "%B", ajFalse);
        }
    
	def = thys->DefStr;
	
	if(def && ajStrGetLen(def[DEF_ADDITIONAL]))
	{
	    if(acdHelpVarResolve(&helpStr, def[DEF_ADDITIONAL]))
	    {
		if(!ajStrToBool(helpStr, &tmpBool))
		    acdErrorAcd(thys, "Bad additional flag %S\n",
				def[DEF_ADDITIONAL]);
	    }
	    else
	    {
		tmpBool = ajTrue;
	    }

	    if(tmpBool)
		item->Optional = ajTrue;
	}
	
	if(def && ajStrGetLen(def[DEF_STANDARD]))
	{
	    if(acdHelpVarResolve(&helpStr, def[DEF_STANDARD]))
	    {
		if(!ajStrToBool( helpStr, &tmpBool))
		    acdErrorAcd(thys, "Bad standard flag %S\n",
				def[DEF_STANDARD]);
	    }
	    else
		tmpBool = ajTrue;

	    if(tmpBool)
		item->Required = ajTrue;
	}
	
    }

    ajStrAssignC(&item->Type, acdType[thys->Type].Name);
    ajStrAssignS(&item->Qual, thys->Name);
    acdHelpExpect(thys, ajTrue, &item->Expect);

    ipos = (ajint) ajStrFindC(item->Expect, " for protein<br>"); 

    if(ipos >= 0)               /* matrix expected value */
    {
        ajStrExchangeCC(&item->Expect, " for DNA", "");
        if(acdDoGalaxyProt)
        {
            ajStrAssignSubS(&galaxyExpectStr, item->Expect, 0, ipos-1);
        }
        else if(acdDoGalaxyNuc)
        {
            ajStrAssignSubS(&galaxyExpectStr, item->Expect, ipos+16, -1);
        }
        else
        {
            ajStrAssignSubS(&galaxyExpectStr, item->Expect, ipos+16, -1);
        }
        ajStrTrimWhite(&galaxyExpectStr);
        ajStrAssignS(&item->Expect, galaxyExpectStr);
    }

    ajStrExchangeCC(&item->Expect, "&nbsp;", "");
    ajStrExchangeCC(&item->Expect, " for any sequence type", "");
    ajStrExchangeCC(&item->Expect, " for any sequence", "");
    ajStrExchangeCC(&item->Expect, "<i>full sequence</i>", "");

    if(ajStrFindC(item->Expect, ".end)") >= 0)
        ajStrAssignC(&item->Expect, "-1");

    if(ajStrFindC(item->Expect, ".begin)") >= 0)
        ajStrAssignC(&item->Expect, "00");

    if(ajStrFindC(item->Expect, "@($(acdprotein)") >= 0) 
    {
        qpos = (ajint) ajStrFindAnyK(item->Expect, '?');
        cpos = (ajint) ajStrFindAnyK(item->Expect, ':');
        bpos = (ajint) ajStrFindlastK(item->Expect, ')');
        if(acdDoGalaxyProt)
        {
            ajStrAssignSubS(&galaxyExpectStr, item->Expect, qpos+1, cpos-1);
        }
        else if(acdDoGalaxyNuc)
        {
            ajStrAssignSubS(&galaxyExpectStr, item->Expect, cpos+1, bpos-1);
        }
        else
        {
            ajStrAssignSubS(&galaxyExpectStr, item->Expect, cpos+1, bpos-1);
        }
        ajStrTrimWhite(&galaxyExpectStr);
        ajStrAssignS(&item->Expect, galaxyExpectStr);
    }
    else if(ajStrFindC(item->Expect, "@(!$(acdprotein)") >= 0) 
    {
        qpos = (ajint) ajStrFindAnyK(item->Expect, '?');
        cpos = (ajint) ajStrFindAnyK(item->Expect, ':');
        bpos = (ajint) ajStrFindlastK(item->Expect, ')');
        if(acdDoGalaxyProt)
        {
            ajStrAssignSubS(&galaxyExpectStr, item->Expect, cpos+1, bpos-1);
        }
        else if(acdDoGalaxyNuc)
        {
            ajStrAssignSubS(&galaxyExpectStr, item->Expect, qpos+1, cpos-1);
        }
        else
        {
            ajStrAssignSubS(&galaxyExpectStr, item->Expect, qpos+1, cpos-1);
        }
        ajStrTrimWhite(&galaxyExpectStr);
        ajStrAssignS(&item->Expect, galaxyExpectStr);
    }

    prompt = acdAttrValue(thys, "code");
    if(!ajStrGetLen(prompt))
        prompt = acdAttrValue(thys, "prompt");
    if(!ajStrGetLen(prompt))
        prompt = acdAttrValue(thys, "information");
    if(!ajStrGetLen(prompt))
    {
        for (itype=0; acdType[itype].Name; itype++)
            if (ajStrMatchC(item->Type, acdType[itype].Name))
                break;
        if(acdType[itype].Prompt)
            (*acdType[itype].Prompt)(thys);
        prompt = thys->StdPrompt;
    }
    if(!ajStrGetLen(prompt))
        prompt = acdAttrValue(thys, "help");

    ajStrAssignS(&item->Prompt, prompt);

    /*
     **  warning - don't try acdVarResolve here because we have not yet
     **  read in the data and things like calculated attributes do not exist
     */
    
    acdHelpValid(thys, ajTrue, &item->Valid);
    acdHelpText(thys, &item->Annotation);
    acdAttrToStr(thys, "relation", "", &item->Relation);
    ajListPushAppend(tablist, item);
    ajStrDel (&helpStr);

    return;
}




/* @funcstatic acdListReport **************************************************
**
** Reports the current status of the ACD internal structures, converting
** values to a printable form as appropriate.
**
** @param [r] title [const char*] Title line for this call
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdListReport(const char* title)
{
    AcdPAcd pa;
    
    ajint i = 0;
    ajint j = 0;
    char underline[256];
    
    
    if(!acdDoLog)
	return;
    
    j = strlen(title);
    if(j > 255)
	j = 255;
    
    memset(underline, '=', j);
    underline[j] = '\0';
    
    acdLog("\n");
    acdLog("%s\n", title);
    acdLog("%s\n", underline);
    acdLog("\n");
    
    for(pa=acdList; pa; pa=pa->Next)
    {
	acdLog("ACD %d\n", i);

	if(pa->PNum)
	{
	    acdLog("       Name: '%S[%d]'\n", pa->Name, pa->PNum);
	    acdLog("      Token: '%S[%d]'\n", pa->Token, pa->PNum);
	}
	else
	{
	    acdLog("       Name: '%S'\n", pa->Name);
	    acdLog("      Token: '%S'\n", pa->Token);
	}
	acdLog("      Param: %d\n", pa->PNum);
	acdLog("      Level: %d   (%s)\n", pa->Level, acdLevel[pa->Level]);

	if(acdIsQtype(pa))
	    acdLog("  Qual Type: %d   (%s)\n", pa->Type,
		   acdType[pa->Type].Name);
	else
	    acdLog("   Key Type: %d   (%s)\n", pa->Type,
		   acdKeywords[pa->Type].Name);
	acdLog("      NAttr: %d\n", pa->NAttr);
	acdLog("      Assoc: %B\n", pa->Assoc);

	if(pa->AssocQuals)
	    acdLog(" AssocQuals: %S\n", pa->AssocQuals->Name);
	else
	    acdLog(" AssocQuals: <undefined>\n");
	acdLog("    Defined: %B\n", pa->Defined);
	acdLog("    Userdefined: %B\n", pa->UserDefined);
	acdLog("Orig. Value: '%S'\n", pa->OrigStr);

	if(pa->ValStr)
	    acdLog("      Value: '%S'\n", pa->ValStr);
	else
	    acdLog("      Value: <undefined>\n");
	acdLog("\n");

	if(pa->DefStr)
	{
	    acdLog("      Default Attributes:\n");
	    acdListAttr(acdAttrDef, pa->DefStr, nDefAttr);
	    acdLog("\n");
	}
	acdLog("      Attributes:\n");

	if(acdIsQtype(pa))
	    acdListAttr(acdType[pa->Type].Attr, pa->AttrStr, pa->NAttr);
	else
	    acdListAttr(acdKeywords[pa->Type].Attr, pa->AttrStr, pa->NAttr);

	acdLog("\n");
	i++;
    }
    
    return;
}




/* @funcstatic acdListAttr ****************************************************
**
** Reports all attributes for an ACD attribute list.
**
** @param [r] attr [const AcdPAttr] Attribute list
** @param [r] valstr [const AjPPStr] Array of string attribute values
** @param [r] nattr [ajint] Number of attributes in list
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdListAttr(const AcdPAttr attr, const AjPPStr valstr, ajint nattr)
{
    ajint i;

    if(!valstr)
	return;

    for(i=0; i < nattr; i++)
    {
	if(valstr[i])
	    acdLog(" %15.15s: '%S'\n", attr[i].Name, valstr[i] );
	else
	    acdLog(" %15.15s: <null>\n", attr[i].Name);
    }

    return;
}




/* @funcstatic acdSet *********************************************************
**
** Sets an attribute or associated qualifier value for an ACD item.
**
** All attributes, of whatever type, are treated as strings at this stage.
**
** @param [r] thys [const AcdPAcd] ACD item.
** @param [u] attrib [AjPStr*] Attribute name - converted to full name
** @param [r] value [const AjPStr] Attribute value
** @return [AjBool] ajTrue if attribute is valid.
** @cre Aborts if attribute is not found.
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdSet(const AcdPAcd thys, AjPStr* attrib, const AjPStr value)
{
    ajint iattr = -1;
    ajint idef  = -1;
    
    AcdPAttr attr   = acdType[thys->Type].Attr;
    AjPStr *attrstr = thys->AttrStr;
    AcdPAcd aqual;
    
    acdLog("acdSet attr '%S' val '%S' type '%S'\n",
	   thys->Name, *attrib, value);
    
    /* recursion with associated qualifiers */
    aqual = NULL;
    if(thys->AssocQuals)
	aqual = acdFindAssoc(thys, *attrib, NULL);
    
    iattr = acdFindAttr(attr, *attrib);
    
    if(thys->DefStr)	       /* try again with default attributes */
	idef = acdFindAttr(acdAttrDef, *attrib);
    
    if(iattr >= 0 && idef >= 0)		/* should never happen */
	acdErrorAcd(thys, "Duplicate type and default attribute '%S'",
		    *attrib);
    
    if(aqual)
    {
	if(iattr >= 0)
	    acdErrorAcd(thys,		/* no known case */
			"'%S' matches attribute '%s' and "
			"associated qualifier '%S'",
			*attrib, attr[iattr].Name, aqual->Name);
	if(idef >= 0)
	    acdErrorAcd(thys,		/* no known case */
			"'%S' matches default attribute '%s' and "
			"associated qualifier '%S'",
			*attrib, acdAttrDef[idef].Name, aqual->Name);
    }
    
    if(iattr >= 0)
    {
	ajStrAssignC(attrib, attr[iattr].Name);
        if(attr[iattr].Multiple)
        {
            if(ajStrGetLen(attrstr[iattr]))
                ajStrAppendK(&attrstr[iattr], '|');
            ajStrAppendS(&attrstr[iattr], value);
        }
        else 
        {
            if(acdDoValid && ajStrGetLen(attrstr[iattr]))
                acdWarn("duplicate attribute '%S'", *attrib);
            ajStrAssignS(&attrstr[iattr], value);
        }

	return ajTrue;
    }
    
    if(idef >= 0)
    {
	ajStrAssignC(attrib, acdAttrDef[idef].Name);
        if(acdAttrDef[idef].Multiple)
        {
            if(ajStrGetLen(thys->DefStr[idef]))
                ajStrAppendK(&thys->DefStr[idef], '|');
            ajStrAppendS(&thys->DefStr[idef], value);
        }

        else
        {
            if(acdDoValid && ajStrGetLen(thys->DefStr[idef]))
                acdWarn("duplicate attribute '%S'", *attrib);
            ajStrAssignS(&thys->DefStr[idef], value);
        }

	return ajTrue;
    }
    
    if(aqual)
	return acdDef(aqual, value);

    /* test: wrongattr.acd */
    acdErrorAcd(thys, "Attribute '%S' unknown\n", *attrib );

    return ajFalse;
}




/* @funcstatic acdSetKey ******************************************************
**
** Sets an attribute for an ACD key item.
**
** All attributes, of whatever type, are treated as strings at this stage.
**
** @param [r] thys [const AcdPAcd] ACD item.
** @param [u] attrib [AjPStr*] Attribute name, converted to full name
** @param [r] value [const AjPStr] Attribute value
** @return [AjBool] ajTrue if attribute is valid.
** @cre Aborts if attribute is not found.
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdSetKey(const AcdPAcd thys, AjPStr* attrib, const AjPStr value)
{
    ajint iattr = -1;

    AcdPAttr attr = acdKeywords[thys->Type].Attr;
    AjPStr* attrstr = thys->AttrStr;
    AcdPAcd aqual;

    /* recursion with associated qualifiers - e.g.for application */
    aqual = NULL;

    if(thys->AssocQuals)
	aqual = acdFindAssoc(thys, *attrib, NULL);

    iattr = acdFindAttr(attr, *attrib);

    if(aqual)
	if(iattr >= 0)
	    acdErrorAcd(thys,		/* no known case */
			"'%S' matches attribute '%s' and qualifier '%S'",
			*attrib, attr[iattr].Name, aqual->Name);

    if(iattr >= 0)
    {
	ajStrAssignC(attrib, attr[iattr].Name);
        if(attr[iattr].Multiple)
        {
            if(ajStrGetLen(attrstr[iattr]))
                ajStrAppendK(&attrstr[iattr], '|');
            ajStrAppendS(&attrstr[iattr], value);
        }
        else
        {
            if(acdDoValid && ajStrGetLen(attrstr[iattr]))
                acdWarn("duplicate attribute '%S'", *attrib);
            ajStrAssignS(&attrstr[iattr], value);
        }
        
	return ajTrue;
    }
  
    if(aqual)
	return acdDef(aqual, value);
  
    /* test: wrongattr.acd */
    acdErrorAcd(thys, "Attribute '%S' unknown\n", *attrib );

    return ajFalse;
}




/* @funcstatic acdDef *********************************************************
**
** Sets the default value for an ACD item, and flags in thys as Defined.
**
** @param [u] thys [AcdPAcd] ACD item
** @param [r] value [const AjPStr] Default value
** @return [AjBool] ajTrue always.
** @see acdSetDef
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdDef(AcdPAcd thys, const AjPStr value)
{
    AjPStr* attrstr = thys->DefStr;

    acdLog("acdDef %S '%S' %x\n", thys->Name, value, attrstr);

    acdSetDef(thys, value);
    thys->Defined = ajTrue;

    return ajTrue;
}




/* @funcstatic acdSetDef ******************************************************
**
** Sets the default value for an ACD item.
**
** @param [u] thys [AcdPAcd] ACD item
** @param [r] value [const AjPStr] Default value
** @return [AjBool] ajTrue always.
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdSetDef(AcdPAcd thys, const AjPStr value)
{
    AjPStr* attrstr = thys->DefStr;

    acdLog("acdSetDef %S '%S' %x\n", thys->Name, value, attrstr);

    if(!thys->DefStr)
	return ajFalse;

    ajStrAssignS(&attrstr[DEF_DEFAULT], value);

    return ajTrue;
}




/* @funcstatic acdSetDefC *****************************************************
**
** Sets the default value for an ACD item.
**
** @param [u] thys [AcdPAcd] ACD item
** @param [r] value [const char *] Default value
** @return [AjBool] ajTrue always.
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdSetDefC(AcdPAcd thys, const char* value)
{
    AjPStr *attrstr = thys->DefStr;

    acdLog("acdSetDefC %S '%s' %x\n", thys->Name, value, attrstr);

    if(!thys->DefStr)
	return ajFalse;

    ajStrAssignC(&attrstr[DEF_DEFAULT], value);

    return ajTrue;
}




/* @funcstatic acdSetQualDefBool **********************************************
**
** Sets the default value for an ACD item or one of its associated qualifiers
**
** @param [u] thys [AcdPAcd] ACD item
** @param [r] name [const char *] Qualifier name
** @param [r] value [AjBool] Default value
** @return [AjBool] ajTrue always.
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdSetQualDefBool(AcdPAcd thys,
				const char* name, AjBool value)
{
    AjPStr *attrstr;
    AjPStr qname = NULL;
    AcdPAcd acd;

    ajStrAssignC(&qname, name);
    acd = acdFindQualAssoc(thys, qname, NULL, NULL, 0);
    ajStrDel(&qname);

    if(!acd)
	return ajFalse;

    attrstr = acd->DefStr;

    acdLog("acdSetQualDefBool %S [%d] '%s' %B\n",
	   thys->Name, thys->PNum, name, value);

    if(!thys->DefStr)
	return ajFalse;

    ajFmtPrintS(&attrstr[DEF_DEFAULT], "%b", value);

    return ajTrue;
}




/* @funcstatic acdSetQualDefInt ***********************************************
**
** Sets the default value for an ACD item or one of its associated qualifiers
**
** @param [u] thys [AcdPAcd] ACD item
** @param [r] name [const char *] Qualifier name
** @param [r] value [ajint] Default value
** @return [AjBool] ajTrue always.
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdSetQualDefInt(AcdPAcd thys, const char* name,
			       ajint value)
{
    AjPStr *attrstr;
    AjPStr qname = NULL;
    AcdPAcd acd;

    ajStrAssignC(&qname, name);
    acd = acdFindQualAssoc(thys, qname, NULL, NULL, 0);

    if(!acd)
	return ajFalse;

    attrstr = acd->DefStr;

    acdLog("acdSetQualDefInt %S [%d] '%s' %S [%d] %d\n",
	   thys->Name, thys->PNum, name, acd->Name, acd->PNum, value);

    if(!thys->DefStr)
	return ajFalse;

    ajFmtPrintS(&attrstr[DEF_DEFAULT], "%d", value);

    ajStrDel(&qname);

    return ajTrue;
}




/* @funcstatic acdSetVarDef ***************************************************
**
** Sets the default value for a variable ACD item.
**
** @param [u] thys [AcdPAcd] ACD item
** @param [r] value [const AjPStr] Default value
** @return [AjBool] ajTrue always.
**
** @release 1.0.0
** @@
**
** Note: we have to set the ValStr directly as variables have no defstr.
** Variable references are resolved later by acdSetVar
******************************************************************************/

static AjBool acdSetVarDef(AcdPAcd thys, const AjPStr value)
{
    acdLog("acdSetVarDef %S '%S' %x\n", thys->Name, value, thys->ValStr);

    ajStrAssignS(&thys->ValStr, value);

    return ajTrue;
}




/* @funcstatic acdFindAttr ****************************************************
**
** Locates an attribute by name in an attribute list.
**
** @param [r] attr [const AcdPAttr] Attribute list
** @param [r] attrib [const AjPStr] Attribute name to be found
** @return [ajint] offset in "attr" if found
** @error -1 if not found.
**
** @release 1.0.0
** @@
******************************************************************************/

static ajint acdFindAttr(const AcdPAttr attr, const AjPStr attrib)
{
    static ajint i;
    static ajint j;
    ajint ifound = 0;

    AjPStr ambigList = NULL;

    ambigList = ajStrNew();

    for(i=0; attr[i].Name; i++)
    {
	if(ajStrMatchC(attrib, attr[i].Name))
	{
	    ajStrDel(&ambigList);
	    return i;
	}

	if(ajCharPrefixS(attr[i].Name, attrib))
	{
	    ifound++;
	    j = i;
	    acdAmbigAppC(&ambigList, attr[i].Name);
	}
    }

    if(ifound == 1)
    {
	if (acdDoValid)
	    acdWarn("Abbreviated attribute '%S' (%S)", attrib, ambigList);
	ajStrDel(&ambigList);
	return j;
    }

    if(ifound > 1)
    {
	ajWarn("Ambiguous attribute %S (%S)", attrib, ambigList);
    }

    ajStrDel(&ambigList);

    return -1;
}




/* @funcstatic acdFindAttrC ***************************************************
**
** Locates an attribute by name in an attribute list.
**
** @param [r] attr [const AcdPAttr] Attribute list
** @param [r] attrib [const char*] Attribute name to be found
** @return [ajint] offset in "attr" if found
** @error -1 if not found.
**
** @release 1.0.0
** @@
******************************************************************************/

static ajint acdFindAttrC(const AcdPAttr attr, const char* attrib)
{
    static ajint i;
    static ajint j;
    ajint k;
    ajint ifound = 0;
    AjPStr ambigList = NULL;

    k = strlen(attrib);

    ambigList = ajStrNew();

    for(i=0; attr[i].Name; i++)
	if(!strncmp(attr[i].Name, attrib, k))
	{
	    if(!strcmp(attr[i].Name, attrib))
	    {
		ajStrDel(&ambigList);
		return i;
	    }

	    ifound++;
	    j = i;
	    acdAmbigAppC(&ambigList, attr[i].Name);
	}

    if(ifound == 1)
    {
	if (acdDoValid)
	    acdWarn("Abbreviated attribute '%s', %S", attrib, ambigList);
	ajStrDel(&ambigList);

	return j;
    }

    if(ifound > 1)
    {
	ajWarn("ambiguous attribute %s (%S)", attrib, ambigList);
    }

    ajStrDel(&ambigList);

    return -1;
}




/* @funcstatic acdProcess *****************************************************
**
** Steps through all the ACD items, filling in missing information.
** Parameters are defined in the default attributes. The parameter
** number is generated here in the order they are found.
**
** Associated qualifiers (if any) also get a copy of the parameter number.
**
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdProcess(void)
{
    AcdPAcd pa;
    AcdPAcd qa = NULL;

    AjPStr reqstr = NULL;
    AjPStr yesstr = NULL;
    AjBool isreq;
    AjBool isparam;

    if(!reqstr)
    {
	ajStrAssignC(&reqstr, "standard");
	ajStrAssignC(&yesstr, "Y");
    }

    for(acdProcCurr=acdList; acdProcCurr; acdProcCurr=acdProcCurr->Next)
    {
	pa = acdProcCurr;

	if(pa->DefStr)
	    ajStrAssignS(&pa->OrigStr, pa->DefStr[DEF_DEFAULT]);

	acdLog("acdProcess '%S' defstr '%x' test parameter\n",
	       pa->Name, pa->DefStr);

	if(!pa->Assoc && pa->DefStr && acdAttrToBoolTest(pa, "parameter",
                                                         ajFalse, &isparam))
	{
	    if(isparam)
	    {
		acdNParam++;
		pa->PNum = acdNParam;
		pa->Level = ACD_PARAM;
		acdLog("acdProcess '%S' required\n", pa->Name);

		/* no unresolvable variables */
		if(!(acdVarTest(acdAttrValue(pa, "standard"))))
		    if(!(acdAttrToBool(pa, "standard", ajFalse, &isreq)))
		    {
			acdSet(pa, &reqstr, yesstr);
		    }

		qa = pa->AssocQuals;
		if(qa)
		    while(qa->Assoc)
		    {
			qa->PNum = acdNParam;
			qa = qa->Next;
		    }
	    }
	}
    }

    ajStrDel(&reqstr);
    ajStrDel(&yesstr);

    return;
}




/* @funcstatic acdSetAll ******************************************************
**
** Steps through all the ACD items, calling the acdSet... function
** for each in turn to prompt the user for any missing values and
** to check that all is OK.
**
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdSetAll(void)
{
    AcdPAcd pa;

    AjBool isstd;
    AjBool isadd;

    const char* stdstring = "std";
    const char* addstring = "opt";
    const char* advstring = "   ";
    const char* nostring  = "   ";
    const char* level = NULL;
    ajint iendsec = 0;

    if (acdDoTrace)
    {
	iendsec = acdFindKeyC("endsection");
	ajUserDumpC("Trace:");
	ajUserDumpC("Trace: Line Std        ACD_Type  Name and 'value'");
	ajUserDumpC("Trace: ---- --- ---------------  ----------------");
    }

    for(acdSetCurr=acdList; acdSetCurr; acdSetCurr = acdSetCurr->Next)
    {
	pa = acdSetCurr;

	if(!pa->Assoc)
	    acdMasterQual = pa;

	if(!acdIsStype(pa))
	{
	    if (acdIsQtype(pa))
		(*acdType[pa->Type].TypeSet)(pa);
	    else
		(*acdKeywords[pa->Type].KeySet)(pa);
	}

	if (acdDoTrace)
	{
	    if(acdIsQtype(pa))
	    {
		acdAttrToBool(pa, "standard", ajFalse, &isstd);
		acdAttrToBool(pa, "parameter", isstd, &isstd);
		acdAttrToBool(pa, "additional", ajFalse, &isadd);
		if (isstd)
		    level = stdstring;
		else if (isadd)
		    level = addstring;
		else
		    level = advstring;
		
		if (pa->Assoc)
		    continue;
		else
		    ajUser("Trace: %4d %s %15s: %15S '%S'",
			   pa->LineNum, level, acdType[pa->Type].Name,
			   pa->Name, pa->ValStr);
	    }
	    else if(acdIsStype(pa))
	    {
		ajUser("Trace: %4d %s %15s: %15S",
		       pa->LineNum, level, acdKeywords[pa->Type].Name,
		       pa->Name);
		if (pa->Type == iendsec)
		    ajUserDumpC("Trace:");
	    }
	    else
	    {
		ajUser("Trace: %4d %s %15s: %15S '%S'",
		       pa->LineNum, nostring, acdKeywords[pa->Type].Name,
		       pa->Name, pa->ValStr);
	    }
	}
    }

    return;
}




/* @funcstatic acdQualToBool **************************************************
**
** Converts an associated qualifier value into a boolean.
** Any variable references are resolved at this stage.
**
** @param [r] thys [const AcdPAcd] ACD item of master parameter or qualifier.
** @param [r] qual [const char*] Qualifier name
** @param [r] defval [AjBool] default value
** @param [w] result [AjBool*] Resulting value.
** @param [w] valstr [AjPStr*] Resulting value as a string
** @return [AjBool] ajTrue on success
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdQualToBool(const AcdPAcd thys, const char *qual,
			    AjBool defval, AjBool *result, AjPStr* valstr)
{
    AjBool ret;

    ret = acdGetValueAssoc(thys, qual, valstr);
    acdLog("acdQualToBool item: %S qual: %s defval: %B str: '%S', ret: %B\n",
	   thys->Name, qual, defval, *valstr, ret);
    if(ret)
    {
	acdVarResolve(valstr);
	acdLog("resolved to: '%S'\n", *valstr);

	if(ajStrGetLen(*valstr))
	{
	    if(!ajStrToBool(*valstr, result))
	    {
		acdErrorAcd(thys,
			    "Bad associated qualifier "
			    "boolean value -%s = %S\n",
			    thys->Name , qual, *valstr) ;
	    }

	    return ajTrue;
	}
    }

    *result = defval;
    ajFmtPrintS(valstr, "%b", defval);

    return ajFalse;
}




/* @funcstatic acdQualToFloat *************************************************
**
** Converts an associated qualifier value into a floating point number.
** Any variable references are resolved at this stage.
**
** @param [r] thys [const AcdPAcd] ACD item of master parameter or qualifier.
** @param [r] qual [const char*] Qualifier name
** @param [r] defval [float] default value
** @param [r] precision [ajint] floating point precision
** @param [w] result [float*] Resulting value.
** @param [w] valstr [AjPStr*] Resulting value as a string
** @return [AjBool] ajTrue on success
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdQualToFloat(const AcdPAcd thys, const char *qual,
			     float defval, ajint precision,
			     float *result, AjPStr* valstr)
{
    AjBool ret;

    ret = acdGetValueAssoc(thys, qual, valstr);
    acdLog("acdQualToFloat item: %S qual: %s defval: %.3f "
	   "str: '%S' ret: %B\n",
	   thys->Name, qual, defval, *valstr, ret);

    if(ret)
    {
	acdVarResolve(valstr);
	acdLog("resolved to: '%S'\n", *valstr);


	if(ajStrGetLen(*valstr))
	{
	    if(!ajStrToFloat(*valstr, result))
	    {
		acdErrorAcd(thys,
			    "%S: Bad associated qualifier "
			    "float value -%s = %S\n",
			    qual, *valstr) ;
	    }
	    return ajTrue;
	}
    }

    *result = defval;
    ajStrFromFloat(valstr, defval, precision);

    return ajFalse;
}




/* @funcstatic acdQualToInt ***************************************************
**
** Converts an associated qualifier value into an integer.
** Any variable references are resolved at this stage.
**
** @param [r] thys [const AcdPAcd] ACD item of master parameter or qualifier.
** @param [r] qual [const char*] Qualifier name
** @param [r] defval [ajint] default value
** @param [w] result [ajint*] Resulting value.
** @param [w] valstr [AjPStr*] Qualifier value as a string
** @return [AjBool] ajTrue on success
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdQualToInt(const AcdPAcd thys, const char *qual,
			   ajint defval, ajint *result, AjPStr* valstr)
{
    AjBool ret;

    ret = acdGetValueAssoc(thys, qual, valstr);
    acdLog("acdQualToInt item: %S qual: %s defval: %d str: '%S' ret: %B\n",
	   thys->Name, qual, defval, *valstr, ret);

    if(ret)
    {
	acdVarResolve(valstr);
	acdLog("resolved to: '%S'\n", *valstr);

	if(ajStrGetLen(*valstr))
	{
	    if(ajStrMatchC(*valstr, "default"))
		ajStrAssignC(valstr, "0");

	    if(!ajStrToInt(*valstr, result))
		acdErrorAcd(thys,
			    "%S: Bad associated qualifier "
			    "integer value -%s = %S\n",
			    qual, *valstr);

	    return ajTrue;
	}
    }

    *result = defval;
    ajStrFromInt(valstr, defval);

    return ajFalse;
}




/* @funcstatic acdQualToLong **************************************************
**
** Converts an associated qualifier value into a long integer.
** Any variable references are resolved at this stage.
**
** @param [r] thys [const AcdPAcd] ACD item of master parameter or qualifier.
** @param [r] qual [const char*] Qualifier name
** @param [r] defval [ajlong] default value
** @param [w] result [ajlong*] Resulting value.
** @param [w] valstr [AjPStr*] Qualifier value as a string
** @return [AjBool] ajTrue on success
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdQualToLong(const AcdPAcd thys, const char *qual,
                            ajlong defval, ajlong *result, AjPStr* valstr)
{
    AjBool ret;

    ret = acdGetValueAssoc(thys, qual, valstr);
    acdLog("acdQualToLong item: %S qual: %s defval: %Ld str: '%S' ret: %B\n",
	   thys->Name, qual, defval, *valstr, ret);

    if(ret)
    {
	acdVarResolve(valstr);
	acdLog("resolved to: '%S'\n", *valstr);

	if(ajStrGetLen(*valstr))
	{
	    if(ajStrMatchC(*valstr, "default"))
		ajStrAssignC(valstr, "0");

	    if(!ajStrToLong(*valstr, result))
		acdErrorAcd(thys,
			    "%S: Bad associated qualifier "
			    "long integer value -%s = %S\n",
			    qual, *valstr);

	    return ajTrue;
	}
    }

    *result = defval;
    ajStrFromLong(valstr, defval);

    return ajFalse;
}




/* @funcstatic acdQualToSeqbegin **********************************************
**
** Converts an associated qualifier value into an integer, or the
** string "start".
**
** Any variable references are resolved at this stage.
**
** @param [r] thys [const AcdPAcd] ACD item of master parameter or qualifier.
** @param [r] qual [const char*] Qualifier name
** @param [r] defval [ajint] default value
** @param [w] result [ajint*] Resulting value.
** @param [w] valstr [AjPStr*] Qualifier value as a string
** @return [AjBool] ajTrue on success
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdQualToSeqbegin(const AcdPAcd thys, const char *qual,
				ajint defval, ajint *result, AjPStr* valstr)
{
    AjBool ret;

    ret = acdGetValueAssoc(thys, qual, valstr);
    acdLog("acdQualToSeqpos item: %S qual: %s defval: %d str: '%S' ret: %B\n",
	   thys->Name, qual, defval, *valstr, ret);

    if(ret)
    {
	acdVarResolve(valstr);
	acdLog("resolved to: '%S'\n", *valstr);

	if(ajStrGetLen(*valstr))
	{
	    if(!ajStrMatchCaseC(*valstr, "default"))
		if(!ajStrToInt(*valstr, result))
		{
		    acdErrorAcd(thys,
				"Bad associated qualifier "
				"integer value -%s = %S\n",
				qual, *valstr);
		}

	    acdLog("return value %B '%S'\n", ajTrue, *valstr);
	    return ajTrue;
	}
    }

    *result = defval;

    if(!defval)
	ajStrAssignC(valstr, "start");
    else
	ajStrFromInt(valstr, defval);


    acdLog("return default %B '%S'\n", ajFalse, *valstr);

    return ajFalse;
}




/* @funcstatic acdQualToSeqend ************************************************
**
** Converts an associated qualifier value into an integer, or the
** string "end".
**
** Any variable references are resolved at this stage.
**
** @param [r] thys [const AcdPAcd] ACD item of master parameter or qualifier.
** @param [r] qual [const char*] Qualifier name
** @param [r] defval [ajint] default value
** @param [w] result [ajint*] Resulting value.
** @param [w] valstr [AjPStr*] Qualifier value as a string
** @return [AjBool] ajTrue on success
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdQualToSeqend(const AcdPAcd thys, const char *qual,
			      ajint defval, ajint *result, AjPStr* valstr)
{   
    AjBool ret;
    
    ret = acdGetValueAssoc(thys, qual, valstr);
    acdLog("acdQualToSeqpos item: %S qual: %s defval: %d str: '%S' ret: %B\n",
	   thys->Name, qual, defval, *valstr, ret);

    if(ret)
    {
	acdVarResolve(valstr);
	acdLog("resolved to: '%S'\n", *valstr);

	if(ajStrGetLen(*valstr))
	{
	    if(!ajStrMatchCaseC(*valstr, "default"))
		if(!ajStrToInt(*valstr, result))
		{
		    acdErrorAcd(thys,
				"Bad associated qualifier "
				"integer value -%s = %S\n",
				qual, *valstr);
		}

	    acdLog("return value %B '%S'\n", ajTrue, *valstr);
	    return ajTrue;
	}
    }
    
    *result = defval;

    if(!defval)
	ajStrAssignC(valstr, "end");
    else
	ajStrFromInt(valstr, defval);
    
    acdLog("return default %B '%S'\n", ajFalse, *valstr);

    return ajFalse;
}




/* @funcstatic acdAttrToBoolTest **********************************************
**
** Resolves and tests an attribute string. If it has a boolean value, returns
** true and sets the value. Otherwise returns false and the default value.
**
** Has to take care to test for variables, as their values are not yet
** set when this is called.
**
** @param [r] thys [const AcdPAcd] ACD item
** @param [r] attr [const char*] Attribute name
** @param [r] defval [AjBool] Default value
** @param [w] result [AjBool*] Resulting value.
** @return [AjBool] ajTrue if a value was defined, ajFalse if the
**         default value was used.
**
** @release 2.7.0
** @@
******************************************************************************/

static AjBool acdAttrToBoolTest(const AcdPAcd thys,
				const char *attr, AjBool defval,
				AjBool *result)
{
    if(acdVarTest(acdAttrValue(thys, attr))) /* test calcparam.acd */
	acdErrorAcd(thys,
		    "'%s' attribute cannot use a variable. "
		    "It is used to define "
		    "the command line before values have been set",
		    attr);
 
    return acdAttrToBool(thys, attr, defval, result);
}




/* @funcstatic acdAttrToBool **************************************************
**
** Resolves and tests an attribute string. If it has a boolean value, returns
** true and sets the value. Otherwise returns false and the default value.
**
** @param [r] thys [const AcdPAcd] ACD item
** @param [r] attr [const char*] Attribute name
** @param [r] defval [AjBool] Default value
** @param [w] result [AjBool*] Resulting value.
** @return [AjBool] ajTrue if a value was defined, ajFalse if the
**         default value was used.
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdAttrToBool(const AcdPAcd thys,
			    const char *attr, AjBool defval, AjBool *result)
{
    ajint i;

    acdAttrResolve(thys, attr, &acdAttrValTmp);
    
    if(ajStrGetLen(acdAttrValTmp))
    {
	if(ajStrToBool(acdAttrValTmp, result))
	{
	    ajStrDelStatic(&acdAttrValTmp);

	    return ajTrue;
	}

	if(ajStrToInt(acdAttrValTmp, &i))
	{
	    if(i)
                *result = ajTrue;
	    else
                *result = ajFalse;

	    ajStrDelStatic(&acdAttrValTmp);

	    return ajTrue;
	}
	else
	{
	    acdErrorAcd(thys, "%S: Bad attribute boolean value %s = %S\n",
			thys->Name, attr, acdAttrValTmp);
	}
    }

    *result = defval;
    ajStrDelStatic(&acdAttrValTmp);

    return ajFalse;
}




/* @funcstatic acdAttrToDouble ************************************************
**
** Resolves and tests an attribute string. If it has a float value, returns
** true and sets the value. Otherwise returns false and the default value.
**
** @param [r] thys [const AcdPAcd] ACD item
** @param [r] attr [const char*] Attribute name
** @param [r] defval [double] Default value
** @param [w] result [double*] Resulting value.
** @return [AjBool] ajTrue if a value was defined, ajFalse if the
**         default value was used.
**
** @release 6.1.0
** @@
******************************************************************************/

static AjBool acdAttrToDouble(const AcdPAcd thys,
                              const char *attr, double defval, double *result)
{
    acdAttrResolve(thys, attr, &acdAttrValTmp);

    if(ajStrGetLen(acdAttrValTmp))
    {
	if(ajStrToDouble(acdAttrValTmp, result))
	{
	    ajStrDelStatic(&acdAttrValTmp);

	    return ajTrue;
	}
	else
	{
	    acdErrorAcd(thys,
			"Bad attribute double value %s = %S\n",
			attr, acdAttrValTmp);
	}
    }

    *result = defval;
    ajStrDelStatic(&acdAttrValTmp);

    return ajFalse;
}




/* @funcstatic acdAttrToFloat *************************************************
**
** Resolves and tests an attribute string. If it has a float value, returns
** true and sets the value. Otherwise returns false and the default value.
**
** @param [r] thys [const AcdPAcd] ACD item
** @param [r] attr [const char*] Attribute name
** @param [r] defval [float] Default value
** @param [w] result [float*] Resulting value.
** @return [AjBool] ajTrue if a value was defined, ajFalse if the
**         default value was used.
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdAttrToFloat(const AcdPAcd thys,
			     const char *attr, float defval, float *result)
{
    acdAttrResolve(thys, attr, &acdAttrValTmp);

    if(ajStrGetLen(acdAttrValTmp))
    {
	if(ajStrToFloat(acdAttrValTmp, result))
	{
	    ajStrDelStatic(&acdAttrValTmp);

	    return ajTrue;
	}
	else
	{
	    acdErrorAcd(thys,
			"Bad attribute float value %s = %S\n",
			attr, acdAttrValTmp);
	}
    }

    *result = defval;
    ajStrDelStatic(&acdAttrValTmp);

    return ajFalse;
}




/* @funcstatic acdAttrToInt ***************************************************
**
** Resolves and tests an attribute string. If it has an integer value, returns
** true and sets the value. Otherwise returns false and the default value.
**
** @param [r] thys [const AcdPAcd] ACD item
** @param [r] attr [const char*] Attribute name
** @param [r] defval [ajint] Default value
** @param [w] result [ajint*] Resulting value.
** @return [AjBool] ajTrue if a value was defined, ajFalse if the
**         default value was used.
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdAttrToInt(const AcdPAcd thys,
			   const char *attr, ajint defval, ajint *result)
{
    acdAttrResolve(thys, attr, &acdAttrValTmp);

    if(ajStrGetLen(acdAttrValTmp))
    {
	if(ajStrToInt(acdAttrValTmp, result))
	{
	    ajStrDelStatic(&acdAttrValTmp);

	    return ajTrue;
	}
	else
	    acdErrorAcd(thys, "Bad attribute integer value %s = %S\n",
			attr, acdAttrValTmp);
    }

    *result = defval;
    ajStrDelStatic(&acdAttrValTmp);

    return ajFalse;
}




/* @funcstatic acdAttrToLong **************************************************
**
** Resolves and tests an attribute string. If it has an integer value, returns
** true and sets the value. Otherwise returns false and the default value.
**
** @param [r] thys [const AcdPAcd] ACD item
** @param [r] attr [const char*] Attribute name
** @param [r] defval [ajlong] Default value
** @param [w] result [ajlong*] Resulting value.
** @return [AjBool] ajTrue if a value was defined, ajFalse if the
**         default value was used.
**
** @release 6.1.0
** @@
******************************************************************************/

static AjBool acdAttrToLong(const AcdPAcd thys,
                            const char *attr, ajlong defval, ajlong *result)
{
    acdAttrResolve(thys, attr, &acdAttrValTmp);

    if(ajStrGetLen(acdAttrValTmp))
    {
	if(ajStrToLong(acdAttrValTmp, result))
	{
	    ajStrDelStatic(&acdAttrValTmp);

	    return ajTrue;
	}
	else
	    acdErrorAcd(thys, "Bad attribute long integer value %s = %S\n",
			attr, acdAttrValTmp);
    }

    *result = defval;
    ajStrDelStatic(&acdAttrValTmp);

    return ajFalse;
}




/* @funcstatic acdAttrToStr ***************************************************
**
** Resolves an attribute to a string with translation of variable name(s).
**
** @param [r] thys [const AcdPAcd] ACD item
** @param [r] attr [const char*] Attribute name
** @param [r] defval [const char*] Default value
** @param [w] result [AjPStr*] Resulting value.
** @return [AjBool] ajTrue if a value was defined, ajFalse if the
**         default value was used.
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdAttrToStr(const AcdPAcd thys,
			   const char *attr, const char* defval,
			   AjPStr *result)
{
    if(acdAttrResolve(thys, attr, result))
	return ajTrue;

    ajStrAssignC(result, defval);

    return ajFalse;
}




/* @funcstatic acdAttrToChar **************************************************
**
** Resolves an attribute to a character with translation of variable name(s).
**
** @param [r] thys [const AcdPAcd] ACD item
** @param [r] attr [const char*] Attribute name
** @param [r] defval [char] Default value
** @param [w] result [char*] Resulting value.
** @return [AjBool] ajTrue if a value was defined, ajFalse if the
**         default value was used.
**
** @release 2.8.0
** @@
******************************************************************************/

static AjBool acdAttrToChar(const AcdPAcd thys,
			    const char *attr, char defval,
			    char* result)
{
    static AjPStr str = NULL;

    acdAttrResolve(thys, attr, &str);

    if(ajStrGetLen(str))
    {
	*result = ajStrGetCharFirst(str);

	return ajTrue;
    }

    *result = defval;

    return ajFalse;
}




/* @funcstatic acdAttrToUint **************************************************
**
** Resolves and tests an attribute string. If it has an integer value, returns
** true and sets the value. Otherwise returns false and the default value.
**
** @param [r] thys [const AcdPAcd] ACD item
** @param [r] attr [const char*] Attribute name
** @param [r] defval [ajuint] Default value
** @param [w] result [ajuint*] Resulting value.
** @return [AjBool] ajTrue if a value was defined, ajFalse if the
**         default value was used.
**
** @release 4.1.0
** @@
******************************************************************************/

static AjBool acdAttrToUint(const AcdPAcd thys,
			   const char *attr, ajuint defval, ajuint *result)
{
    acdAttrResolve(thys, attr, &acdAttrValTmp);

    if(ajStrGetLen(acdAttrValTmp))
    {
	if(ajStrToUint(acdAttrValTmp, result))
	{
	    ajStrDelStatic(&acdAttrValTmp);

	    return ajTrue;
	}
	else
	    acdErrorAcd(thys, "Bad attribute unsigned integer value %s = %S\n",
			attr, acdAttrValTmp);
    }

    *result = defval;
    ajStrDelStatic(&acdAttrValTmp);

    return ajFalse;
}




/* @funcstatic acdAttrResolve *************************************************
**
** Resolves an attribute to a string with translation of variable name(s).
**
** @param [r] thys [const AcdPAcd] ACD item
** @param [r] attr [const char*] Attribute name
** @param [w] result [AjPStr*] Resulting value.
** @return [AjBool] ajTrue if a value was defined,
**                  ajFalse if an empty string is returned.
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdAttrResolve(const AcdPAcd thys, const char *attr,
			     AjPStr *result)
{
    ajStrAssignS(result, acdAttrValue(thys, attr));
    acdVarResolve(result);

    if(ajStrGetLen(*result))
	return ajTrue;

    return ajFalse;
}




/* @funcstatic acdVarTest *****************************************************
**
** tests for any variable (but not function) references in a string.
**
** Used to check whether variables might be used before we have set
** their values..
**
** @param [r] var [const AjPStr] String value
** @return [AjBool] ajTrue if a variable was found
**
** @release 2.7.0
** @@
******************************************************************************/

static AjBool acdVarTest(const AjPStr var)
{
    if(!acdRegVarInit)
        acdRegVarDefine();

    if(ajRegExec(acdRegVarname, var))
	return ajTrue;
  
    return ajFalse;
}




/* @funcstatic acdVarTestValid ************************************************
**
** Tests for any variable (but not function) references in a string.
**
** Used by acdvalid to check for dependencies that are too complex
**
** @param [r] var [const AjPStr] String value
** @param [w] toggle [AjBool*] ajTrue if the value is a simple true or false
**                             test of a toggle variable
** @return [AjBool] ajTrue if a variable was found
**
** @release 2.8.0
** @@
******************************************************************************/

static AjBool acdVarTestValid(const AjPStr var, AjBool* toggle)
{
    AjPStr varref = NULL;
    AjPStr varname = NULL;
    AcdPAcd acd = NULL;

 
    if(!acdRegVarInit)
        acdRegVarDefine();

    *toggle = ajFalse;

    if(!ajRegExec(acdRegVarname, var))
	return ajFalse;

    /*
     ** A variable - is it a simple (toggle) dependency?
     ** Toggles can be $(varname) or \@($(varname)) or \@(!$(varname))
     ** Also allowed is automatic variable acdprotein
     */

    acdLog("acdVarTestValid variable '%S'\n", var);
    if(ajRegExec(acdRegToggle, var))
    {
	ajRegSubI(acdRegToggle, 2, &varref); /* returns $(varname) */
	if(acdVarSimple(varref, &varname))
	{
	    acd = acdFindAcd(varname, varname);
	    if (acd)
	    {
		acdLog("acdVarTestValid varname %S acd %S type %s\n",
		       varname, acd->Name, acdType[acd->Type].Name);
		if(!strcmp(acdType[acd->Type].Name, "toggle"))
		    *toggle = ajTrue;
		if(ajStrMatchCaseC(varname, "acdprotein"))
		    *toggle = ajTrue;
	    }
	    else
		acdLog("acdVarTestValid varname %S not found\n",
		       varname);
	}

	acdLog("varname '%S' toggle %B\n", varname, *toggle);
	ajStrDel(&varref);
	ajStrDel(&varname);
    }

    return ajTrue;
}




/* @funcstatic acdVarSimple ***************************************************
**
** Tests a variable reference is non recursive
**
** @param [r] var [const AjPStr] String value to be tested
** @param [w] varname [AjPStr*] Variable name if found, else unchanged
** @return [AjBool] ajTrue if no further variable or function is found
**
** @release 2.8.0
** @@
******************************************************************************/

static AjBool acdVarSimple(const AjPStr var, AjPStr* varname)
{
    AjPStr attrname = NULL;
    AjPStr result   = NULL;
    AjPStr token    = NULL;
    AjPStr newvar    = NULL;
    AjPStr restvar   = NULL;
    AjBool ret = ajTrue;

    if(!acdRegVarInit)
        acdRegVarDefine();

    if(ajRegExec(acdRegVarname, var))
    {
	ajRegSubI(acdRegVarname, 2, &token);	/* variable name */
	acdVarSplit(token, varname, &attrname);

	if(!ajStrGetLen(attrname))
	    ajStrAssignC(&attrname, "default");

	if(!acdGetAttr(&result, *varname, attrname))
	{
	    acdLog("acdVarSimple failed to resolve '%S.%S\n",
		   *varname, attrname);
	    ajStrAssignClear(&result);
	}

	ajRegSubI(acdRegVarname, 1, &newvar);
	ajStrAppendS(&newvar, result);

	if(ajRegPost(acdRegVarname, &restvar)) /* any more? */
	    ajStrAppendS(&newvar, restvar);

	acdLog("acdVarSimple name %S resolved to '%S'\n", *varname, newvar);

	if(ajRegExec(acdRegVarname, newvar))
	    ret = ajFalse;

	else if(ajRegExec(acdRegFunction, newvar))
	    ret = ajFalse;
    }

    /* else no variable reference at found */

    ajStrDel(&token);
    ajStrDel(&attrname);
    ajStrDel(&result);
    ajStrDel(&newvar);
    ajStrDel(&restvar);

    return ret;
}




/* @funcstatic acdVarSimpleResolve ********************************************
**
** Tests a variable reference is a simple copy and resolves
**
** @param [r] var [const AjPStr] String value to be tested
** @param [w] newvar [AjPStr*] Substituted value if found, else unchanged
** @return [AjBool] ajTrue if no further variable or function is found
**
** @release 2.8.0
** @@
******************************************************************************/

static AjBool acdVarSimpleResolve(const AjPStr var, AjPStr* newvar)
{
    AjPStr attrname = NULL;
    AjPStr result   = NULL;
    AjPStr token    = NULL;
    AjPStr varname  = NULL;
    AjPStr restvar   = NULL;
    AjBool ret = ajTrue;
    AjPStr tmpvar = NULL;

    if(!acdRegVarInit)
        acdRegVarDefine();

    if(ajRegExec(acdRegVarname, var))
    {
        tmpvar = ajStrNewS(*newvar);
        
	ajRegSubI(acdRegVarname, 2, &token);	/* variable name */
	acdVarSplit(token, &varname, &attrname);

	if(!ajStrGetLen(attrname))
	    ajStrAssignC(&attrname, "default");

	if(!acdGetAttr(&result, varname, attrname))
	{
	    acdLog("acdVarSimpleResolve failed to resolve '%S.%S\n",
		   varname, attrname);
            if(ajStrMatchC(attrname, "begin"))
                ajStrAssignC(&result, "0");
            else if(ajStrMatchC(attrname, "end"))
                ajStrAssignC(&result, "0");
            else
                ajStrAssignClear(&result);
	}

	ajRegSubI(acdRegVarname, 1, newvar);
	ajStrAppendS(newvar, result);

	if(ajRegPost(acdRegVarname, &restvar)) /* any more? */
	    ajStrAppendS(newvar, restvar);

	acdLog("acdVarSimpleResolve name %S resolved to '%S'\n",
               varname, *newvar);

	if(ajRegExec(acdRegVarname, *newvar))
	    ret = ajFalse;

	else if(ajRegExec(acdRegFunction, *newvar))
	    ret = ajFalse;

        if(!ret)
            ajStrAssignS(newvar, tmpvar);

        ajStrDel(&tmpvar);
    }

    /* else no variable reference at found */

    ajStrDel(&token);
    ajStrDel(&attrname);
    ajStrDel(&result);
    ajStrDel(&varname);
    ajStrDel(&restvar);

    return ret;
}




/* @funcstatic acdVarResolve **************************************************
**
** Resolves any variable or function references in a string.
**
** First resolves variables in the form $(name) or $(name.attribute).
** This cunningly resolves internal () pairs.
**
** Then looks for function references and resolves them.
**
** @param [w] var [AjPStr*] String value
** @return [AjBool] Always ajTrue so far
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdVarResolve(AjPStr* var)
{
    AjPStr varname  = NULL;
    AjPStr attrname = NULL;
    AjPStr result   = NULL;
    AjPStr token    = NULL;
    AjPStr newvar    = NULL;
    AjPStr restvar   = NULL;
    AjPStr savein    = NULL;

    ajint ivar = 0;
    ajint ifun = 0;
    
    if(!acdRegResolveVar)
	acdRegResolveVar = ajRegCompC("^(.*)\\$\\(([a-zA-Z0-9_.]+)\\)");

    if(!acdRegResolveFun)
	acdRegResolveFun = ajRegCompC("^(.*)\\@\\(([^()]+)\\)");
    
    /* resolve variable references first to resolve internal parentheses */
    if(!var)
    {
	ajStrAssignClear(var);
	return ajTrue;
    }
    
    if(!ajStrGetLen(*var))
	return ajTrue;
    
    ajStrAssignS(&savein, *var);
    acdLog("acdVarResolve '%S'\n", savein);
    
    while(ajRegExec(acdRegResolveVar, *var))
    {
	ivar++;
	ajRegSubI(acdRegResolveVar, 2, &token);	/* variable name */
	acdVarSplit(token, &varname, &attrname);

	if(!acdGetAttr(&result, varname, attrname))
	    ajStrAssignClear(&result);

	ajRegSubI(acdRegResolveVar, 1, &newvar);
	ajStrAppendS(&newvar, result);

	if(ajRegPost(acdRegResolveVar, &restvar)) /* any more? */
	    ajStrAppendS(&newvar, restvar);

	ajStrAssignS(var, newvar);
	acdLog("... name %S resolved to '%S'\n", varname, newvar);
    }
    
    /* now resolve any function */
    
    while(ajRegExec(acdRegResolveFun, *var))
    {
	ifun++;
	ajRegSubI(acdRegResolveFun, 2, &token);	/* function statement */
	acdFunResolve(&result, token);
	ajRegSubI(acdRegResolveFun, 1, &newvar);
	ajStrAppendS(&newvar, result);

	if(ajRegPost(acdRegResolveFun, &restvar)) /* any more? */
	    ajStrAppendS(&newvar, restvar);

	ajStrAssignS(var, newvar);
	acdLog("... function %S resolved to '%S'\n", token, newvar);
    }
    
    if(ivar > 1)
	acdLog("Recursive variables in '%S'\n", savein);
    
    if(ifun > 1)
	acdLog("Recursive expressions in '%S'\n", savein);
    
    if(acdDoTrace)
    {
	if(ifun || ivar)
	    ajUser("Trace:                           resolved '%S' => '%S'",
		   savein, *var);
    }
    ajStrDel(&savein);
    ajStrDel(&result);
    ajStrDel(&varname);
    ajStrDel(&newvar);
    ajStrDel(&restvar);
    ajStrDel(&token);
    ajStrDel(&attrname);
    
    return ajTrue;
}




/* @funcstatic acdHelpVarResolve **********************************************
**
** Resolves any variable or function references in a string if clearly
** defined, otherwise returns ajFalse and sets the string to "". For
** use with strings that cannot be resolved in help processing
** because of functions, variable dependencies etc.
**
** @param [w] str [AjPStr*] String value
** @param [r] var [const AjPStr] Source string value
** @return [AjBool] ajTrue if it could be resolved cleanly
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdHelpVarResolve(AjPStr* str, const AjPStr var)
{
    if(!acdRegVarInit)
        acdRegVarDefine();

    if(!var)
    {
	ajStrAssignClear(str);

	return ajTrue;
    }

    /* reject variable references first to resolve internal parentheses */
    if(ajRegExec(acdRegVarname, var))
    {
        ajStrAssignClear(str);

        return ajFalse;
    }

    /* reject any function */
    if(ajRegExec(acdRegFunction, var))
    {
	ajStrAssignClear(str);

	return ajFalse;
    }

    ajStrAssignS(str, var);

    return ajTrue;
}




/* @funcstatic acdFunResolve **************************************************
**
** Resolves a function reference.
**
** Has a list of all accepted function syntax.
**
** @param [w] result [AjPStr*] Result returned
** @param [r] str [const AjPStr] Function statement input
** @return [AjBool] Always ajTrue so far
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdFunResolve(AjPStr* result, const AjPStr str)
{
    ajint i;

    acdLog("acdFunResolve '%S'\n", str);

    for(i = 0; explist[i].Name; i++)
    {
	/* Calling funclist acdexplist() */

	if((*explist[i].Func)(result, str))
	{
	    acdLog("resolved '%S' using '%s'\n", str, explist[i].Name);
	    acdLog("  result '%S'\n", *result);
	    return ajTrue;
	}
    }

    ajWarn("ACD expression invalid @(%S)", str);
    acdLog("@(%S) *failed**\n", str);

    ajStrAssignS(result, str);

    return ajFalse;
}




/* @funcstatic acdExpPlus *****************************************************
**
** Looks for and resolves an expression \@( num + num )
**
** @param [w] result [AjPStr*] Expression result
** @param [r] str [const AjPStr] String with possible expression
** @return [AjBool] ajTrue if successfully resolved
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdExpPlus(AjPStr* result, const AjPStr str)
{
    ajint ia, ib;
    double da, db;

    if(!acdRegExpPlusI)				/* ajint + ajint */
	acdRegExpPlusI = ajRegCompC("^[ \t]*([0-9+-]+)[ \t]*[+][ \t]*"
			  "([0-9+-]+)[ \t]*$");

    if(ajRegExec(acdRegExpPlusI, str))
    {
	acdLog("acdRegExpPlusI matched  '%S'\n", str);
	ajRegSubI(acdRegExpPlusI, 1, &acdTmpStr);
	ajStrToInt(acdTmpStr, &ia);
	ajRegSubI(acdRegExpPlusI, 2, &acdTmpStr);
	ajStrToInt(acdTmpStr, &ib);
	ajFmtPrintS(result, "%d", ia+ib);
	acdLog("ia: %d + ib: %d = '%S'\n", ia, ib, *result);

	return ajTrue;
    }

    if(!acdRegExpPlusD)				/* float + float */
	acdRegExpPlusD = ajRegCompC("^[ \t]*([0-9.+-]+)[ \t]*[+][ \t]*"
			  "([0-9.+-]+)[ \t]*$");

    if(ajRegExec(acdRegExpPlusD, str))
    {
	acdLog("dexp matched  '%S'\n", str);
	ajRegSubI(acdRegExpPlusD, 1, &acdTmpStr);
	ajStrToDouble(acdTmpStr, &da);
	ajRegSubI(acdRegExpPlusD, 2, &acdTmpStr);
	ajStrToDouble(acdTmpStr, &db);
	ajFmtPrintS(result, "%f", da+db);
	acdLog("da: %f + db: %f = '%S'\n", da, db, *result);

	return ajTrue;
    }

    return ajFalse;
}




/* @funcstatic acdExpMinus ****************************************************
**
** Looks for and resolves an expression \@( num - num )
**
** @param [w] result [AjPStr*] Expression result
** @param [r] str [const AjPStr] String with possible expression
** @return [AjBool] ajTrue if successfully resolved
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdExpMinus(AjPStr* result, const AjPStr str)
{
    ajint ia, ib;
    double da, db;

    if(!acdRegExpMinusI)				/* ajint + ajint */
	acdRegExpMinusI = ajRegCompC("^[ \t]*([0-9+-]+)[ \t]*[-][ \t]*"
			  "([0-9+-]+)[ \t]*$");

    if(ajRegExec(acdRegExpMinusI, str))
    {
	acdLog("acdRegExpMinusI matched  '%S'\n", str);
	ajRegSubI(acdRegExpMinusI, 1, &acdTmpStr);
	ajStrToInt(acdTmpStr, &ia);
	ajRegSubI(acdRegExpMinusI, 2, &acdTmpStr);
	ajStrToInt(acdTmpStr, &ib);
	ajFmtPrintS(result, "%d", ia-ib);
	acdLog("ia: %d - ib: %d = '%S'\n", ia, ib, *result);

	return ajTrue;
    }

    if(!acdRegExpMinusD)				/* float + float */
	acdRegExpMinusD = ajRegCompC("^[ \t]*([0-9.+-]+)[ \t]*[-][ \t]*"
			  "([0-9.+-]+)[ \t]*$");

    if(ajRegExec(acdRegExpMinusD, str))
    {
	acdLog("acdRegExpMinusD matched  '%S'\n", str);
	ajRegSubI(acdRegExpMinusD, 1, &acdTmpStr);
	ajStrToDouble(acdTmpStr, &da);
	ajRegSubI(acdRegExpMinusD, 2, &acdTmpStr);
	ajStrToDouble(acdTmpStr, &db);
	ajFmtPrintS(result, "%f", da-db);
	acdLog("da: %f - db: %f = '%S'\n", da, db, *result);

	return ajTrue;
    }

    return ajFalse;
}




/* @funcstatic acdExpStar *****************************************************
**
** Looks for and resolves an expression \@( num * num )
**
** @param [w] result [AjPStr*] Expression result
** @param [r] str [const AjPStr] String with possible expression
** @return [AjBool] ajTrue if successfully resolved
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdExpStar(AjPStr* result, const AjPStr str)
{
    ajint ia, ib;
    double da, db;

    if(!acdRegExpStarI)				/* ajint + ajint */
	acdRegExpStarI = ajRegCompC("^[ \t]*([0-9+-]+)[ \t]*[*][ \t]*"
			  "([0-9+-]+)[ \t]*$");

    if(ajRegExec(acdRegExpStarI, str))
    {
	acdLog("acdRegExpStarI matched  '%S'\n", str);
	ajRegSubI(acdRegExpStarI, 1, &acdTmpStr);
	ajStrToInt(acdTmpStr, &ia);
	ajRegSubI(acdRegExpStarI, 2, &acdTmpStr);
	ajStrToInt(acdTmpStr, &ib);
	ajFmtPrintS(result, "%d", ia*ib);
	acdLog("ia: %d * ib: %d = '%S'\n", ia, ib, *result);

	return ajTrue;
    }

    if(!acdRegExpStarD)				/* float + float */
	acdRegExpStarD = ajRegCompC("^[ \t]*([0-9.+-]+)[ \t]*[*][ \t]*"
			  "([0-9.+-]+)[ \t]*$");

    if(ajRegExec(acdRegExpStarD, str))
    {
	acdLog("acdRegExpStarD matched  '%S'\n", str);
	ajRegSubI(acdRegExpStarD, 1, &acdTmpStr);
	ajStrToDouble(acdTmpStr, &da);
	ajRegSubI(acdRegExpStarD, 2, &acdTmpStr);
	ajStrToDouble(acdTmpStr, &db);
	ajFmtPrintS(result, "%f", da*db);
	acdLog("da: %f * db: %f = '%S'\n", da, db, *result);

	return ajTrue;
    }

    return ajFalse;
}




/* @funcstatic acdExpDiv ******************************************************
**
** Looks for and resolves an expression \@( num / num )
**
** @param [w] result [AjPStr*] Expression result
** @param [r] str [const AjPStr] String with possible expression
** @return [AjBool] ajTrue if successfully resolved
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdExpDiv(AjPStr* result, const AjPStr str)
{
    ajint ia;
    ajint ib;
    double da;
    double db;

    if(!acdRegExpDivI)				/* ajint + ajint */
	acdRegExpDivI = ajRegCompC("^[ \t]*([0-9+-]+)[ \t]*[/][ \t]*"
			  "([0-9+-]+)[ \t]*$");

    if(ajRegExec(acdRegExpDivI, str))
    {
	acdLog("acdRegExpDivI matched  '%S'\n", str);
	ajRegSubI(acdRegExpDivI, 1, &acdTmpStr);
	ajStrToInt(acdTmpStr, &ia);
	ajRegSubI(acdRegExpDivI, 2, &acdTmpStr);
	ajStrToInt(acdTmpStr, &ib);
	ajFmtPrintS(result, "%d", ia/ib);
	acdLog("ia: %d / ib: %d = '%S'\n", ia, ib, *result);

	return ajTrue;
    }

    if(!acdRegExpDivD)				/* float + float */
	acdRegExpDivD = ajRegCompC("^[ \t]*([0-9.+-]+)[ \t]*[/][ \t]*"
			  "([0-9.+-]+)[ \t]*$");

    if(ajRegExec(acdRegExpDivD, str))
    {
	acdLog("acdRegExpDivD matched  '%S'\n", str);
	ajRegSubI(acdRegExpDivD, 1, &acdTmpStr);
	ajStrToDouble(acdTmpStr, &da);
	ajRegSubI(acdRegExpDivD, 2, &acdTmpStr);
	ajStrToDouble(acdTmpStr, &db);
	ajFmtPrintS(result, "%f", da/db);
	acdLog("da: %f / db: %f = '%S'\n", da, db, *result);

	return ajTrue;
    }

    return ajFalse;
}




/* @funcstatic acdExpNot ******************************************************
**
** Looks for and resolves an expression \@(! bool ) or \@(NOT bool)
** or \@(not bool). An invalid bool value is treated as false,
** so it will return a true value.
**
** @param [w] result [AjPStr*] Expression result
** @param [r] str [const AjPStr] String with possible expression
** @return [AjBool] ajTrue if successfully resolved
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdExpNot(AjPStr* result, const AjPStr str)
{
    AjBool ba;

    if(!acdRegExpNot)				/* ajint + ajint */
	acdRegExpNot = ajRegCompC("^[ \t]*(!|[Nn][Oo][Tt])"
			  "[ \t]*([A-Za-z0-9]+)[ \t]*$");

    if(ajRegExec(acdRegExpNot, str))
    {
	acdLog("nexp matched  '%S'\n", str);
	ajRegSubI(acdRegExpNot, 2, &acdTmpStr);

	if(!ajStrToBool(acdTmpStr, &ba))
	{
	    acdLog("invalid bool value '%S' in acdExpNot\n", acdTmpStr);
	    ba = ajFalse;
	}

	if(ba)
	    ajFmtPrintS(result, "%b", ajFalse);
	else
	    ajFmtPrintS(result, "%b", ajTrue);

	acdLog("ta: ! '%S' = '%S'\n", acdTmpStr, *result);

	return ajTrue;
    }

    return ajFalse;
}




/* @funcstatic acdExpEqual ****************************************************
**
** Looks for and resolves an expression \@( num == num )
**
** @param [w] result [AjPStr*] Expression result
** @param [r] str [const AjPStr] String with possible expression
** @return [AjBool] ajTrue if successfully resolved
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdExpEqual(AjPStr* result, const AjPStr str)
{
    ajint ia;
    ajint ib;
    double da;
    double db;
    
    if(!acdRegExpEqualI)				/* ajint + ajint */
	acdRegExpEqualI = ajRegCompC("^[ \t]*([0-9+-]+)[ \t]*[=][=][ \t]*"
			  "([0-9+-]+)[ \t]*$");
    
    if(ajRegExec(acdRegExpEqualI, str))
    {
	acdLog("acdRegExpEqualI matched  '%S'\n", str);
	ajRegSubI(acdRegExpEqualI, 1, &acdTmpStr);
	ajStrToInt(acdTmpStr, &ia);
	ajRegSubI(acdRegExpEqualI, 2, &acdTmpStr);
	ajStrToInt(acdTmpStr, &ib);

	if(ia == ib)
	    ajFmtPrintS(result, "%b", ajTrue);
	else
	    ajFmtPrintS(result, "%b", ajFalse);

	acdLog("ia: %d == ib: %d = '%S'\n", ia, ib, *result);

	return ajTrue;
    }
    
    if(!acdRegExpEqualD)				/* float == float */
	acdRegExpEqualD = ajRegCompC("^[ \t]*([0-9.+-]+)[ \t]*[=][=][ \t]*"
			  "([0-9.+-]+)[ \t]*$");
    
    if(ajRegExec(acdRegExpEqualD, str))
    {
	acdLog("acdRegExpEqualD matched  '%S'\n", str);
	ajRegSubI(acdRegExpEqualD, 1, &acdTmpStr);
	ajStrToDouble(acdTmpStr, &da);
	ajRegSubI(acdRegExpEqualD, 2, &acdTmpStr);
	ajStrToDouble(acdTmpStr, &db);

	if(da == db)
	    ajFmtPrintS(result, "%b", ajTrue);
	else
	    ajFmtPrintS(result, "%b", ajFalse);

	acdLog("da: %f == db: %f = '%S'\n", da, db, *result);

	return ajTrue;
    }
    
    if(!acdRegExpEqualT)				/* string == string */
	acdRegExpEqualT = ajRegCompC("^[ \t]*([^ \t]+)[ \t]*[=][=][ \t]*"
			  "([^ \t{}]+)[ \t]*$"); /* for {} see acdExpOneof */
    
    if(ajRegExec(acdRegExpEqualT, str))
    {
	acdLog("acdRegExpEqualT matched  '%S'\n", str);
	ajRegSubI(acdRegExpEqualT, 1, &acdTmpStr);
	ajRegSubI(acdRegExpEqualT, 2, &acdTmpStr2);

	if(ajStrMatchCaseS(acdTmpStr, acdTmpStr2))
	    ajFmtPrintS(result, "%b", ajTrue);
	else
	    ajFmtPrintS(result, "%b", ajFalse);

	acdLog("ta: '%S' == tb: '%S' = '%S'\n", acdTmpStr, acdTmpStr2,
	       *result);

	return ajTrue;
    }
    
    return ajFalse;
}




/* @funcstatic acdExpNotEqual *************************************************
**
** Looks for and resolves an expression \@( num != num )
**
** @param [w] result [AjPStr*] Expression result
** @param [r] str [const AjPStr] String with possible expression
** @return [AjBool] ajTrue if successfully resolved
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdExpNotEqual(AjPStr* result, const AjPStr str)
{    
    ajint ia;
    ajint ib;
    double da;
    double db;
    
    if(!acdRegExpNeI)				/* int != int */
	acdRegExpNeI = ajRegCompC("^[ \t]*([0-9+-]+)[ \t]*[!][=][ \t]*"
			  "([0-9+-]+)[ \t]*$");
    
    if(ajRegExec(acdRegExpNeI, str))
    {
	acdLog("acdRegExpNeI matched  '%S'\n", str);
	ajRegSubI(acdRegExpNeI, 1, &acdTmpStr);
	ajStrToInt(acdTmpStr, &ia);
	ajRegSubI(acdRegExpNeI, 2, &acdTmpStr);
	ajStrToInt(acdTmpStr, &ib);

	if(ia != ib)
	    ajFmtPrintS(result, "%b", ajTrue);
	else
	    ajFmtPrintS(result, "%b", ajFalse);

	acdLog("ia: %d != ib: %d = '%S'\n", ia, ib, *result);

	return ajTrue;
    }
    
    if(!acdRegExpNeD)				/* float != float */
	acdRegExpNeD = ajRegCompC("^[ \t]*([0-9.+-]+)[ \t]*[!][=][ \t]*"
			  "([0-9.+-]+)[ \t]*$");
    
    if(ajRegExec(acdRegExpNeD, str))
    {
	acdLog("acdRegExpNeD matched  '%S'\n", str);
	ajRegSubI(acdRegExpNeD, 1, &acdTmpStr);
	ajStrToDouble(acdTmpStr, &da);
	ajRegSubI(acdRegExpNeD, 2, &acdTmpStr);
	ajStrToDouble(acdTmpStr, &db);

	if(da != db)
	    ajFmtPrintS(result, "%b", ajTrue);
	else
	    ajFmtPrintS(result, "%b", ajFalse);

	acdLog("da: %f != db: %f = '%S'\n", da, db, *result);

	return ajTrue;
    }
    
    if(!acdRegExpNeT)				/* string != string*/
	acdRegExpNeT = ajRegCompC("^[ \t]*([^ \t]+)[ \t]*[!][=][ \t]*"
			  "([^ \t{}]+)[ \t]*$"); /* for {} see acdExpOneof */
    
    if(ajRegExec(acdRegExpNeT, str))
    {
	acdLog("acdRegExpNeT matched  '%S'\n", str);
	ajRegSubI(acdRegExpNeT, 1, &acdTmpStr);
	ajRegSubI(acdRegExpNeT, 2, &acdTmpStr2);

	if(!ajStrMatchCaseS(acdTmpStr, acdTmpStr2))
	    ajFmtPrintS(result, "%b", ajTrue);
	else
	    ajFmtPrintS(result, "%b", ajFalse);

	acdLog("ta: '%S' != tb: '%S' = '%S'\n",
	       acdTmpStr, acdTmpStr2, *result);

	return ajTrue;
    }
    
    return ajFalse;
}




/* @funcstatic acdExpGreater **************************************************
**
** Looks for and resolves an expression \@( num > num )
**
** @param [w] result [AjPStr*] Expression result
** @param [r] str [const AjPStr] String with possible expression
** @return [AjBool] ajTrue if successfully resolved
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdExpGreater(AjPStr* result, const AjPStr str)
{
    ajint ia;
    ajint ib;
    double da;
    double  db;
    
    if(!acdRegExpGtI)				/* ajint + ajint */
	acdRegExpGtI = ajRegCompC("^[ \t]*([0-9+-]+)[ \t]*[>][ \t]*"
			  "([0-9+-]+)[ \t]*$");
    
    if(ajRegExec(acdRegExpGtI, str))
    {
	acdLog("acdRegExpGtI matched  '%S'\n", str);
	ajRegSubI(acdRegExpGtI, 1, &acdTmpStr);
	ajStrToInt(acdTmpStr, &ia);
	ajRegSubI(acdRegExpGtI, 2, &acdTmpStr);
	ajStrToInt(acdTmpStr, &ib);

	if(ia > ib)
	    ajFmtPrintS(result, "%b", ajTrue);
	else
	    ajFmtPrintS(result, "%b", ajFalse);

	acdLog("ia: %d > ib: %d = '%S'\n", ia, ib, *result);

	return ajTrue;
    }
    
    if(!acdRegExpGtD)				/* float + float */
	acdRegExpGtD = ajRegCompC("^[ \t]*([0-9.+-]+)[ \t]*[>][ \t]*"
			  "([0-9.+-]+)[ \t]*$");
    
    if(ajRegExec(acdRegExpGtD, str))
    {
	acdLog("acdRegExpGtD matched  '%S'\n", str);
	ajRegSubI(acdRegExpGtD, 1, &acdTmpStr);
	ajStrToDouble(acdTmpStr, &da);
	ajRegSubI(acdRegExpGtD, 2, &acdTmpStr);
	ajStrToDouble(acdTmpStr, &db);

	if(da > db)
	    ajFmtPrintS(result, "%b", ajTrue);
	else
	    ajFmtPrintS(result, "%b", ajFalse);

	acdLog("da: %f > db: %f = '%S'\n", da, db, *result);

	return ajTrue;
    }
    
    if(!acdRegExpGtT)				/* float + float */
	acdRegExpGtT = ajRegCompC("^[ \t]*([^ \t]+)[ \t]*[>][ \t]*"
			  "([^ \t]+)[ \t]*$");
    
    if(ajRegExec(acdRegExpGtT, str))
    {
	acdLog("acdRegExpGtT matched  '%S'\n", str);
	ajRegSubI(acdRegExpGtT, 1, &acdTmpStr);
	ajRegSubI(acdRegExpGtT, 2, &acdTmpStr2);

	if(0 > ajStrCmpCaseS(acdTmpStr2, acdTmpStr))
	    ajFmtPrintS(result, "%b", ajTrue);
	else
	    ajFmtPrintS(result, "%b", ajFalse);

	acdLog("ta: '%S' > tb: '%S' = '%S'\n", acdTmpStr, acdTmpStr2,
	       *result);

	return ajTrue;
    }
    
    return ajFalse;
}




/* @funcstatic acdExpLesser ***************************************************
**
** Looks for and resolves an expression \@( num < num )
**
** @param [w] result [AjPStr*] Expression result
** @param [r] str [const AjPStr] String with possible expression
** @return [AjBool] ajTrue if successfully resolved
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdExpLesser(AjPStr* result, const AjPStr str)
{
    ajint ia;
    ajint ib;
    double da;
    double db;
    
    if(!acdRegExpLtI)				/* ajint + ajint */
	acdRegExpLtI = ajRegCompC("^[ \t]*([0-9+-]+)[ \t]*[<][ \t]*"
			  "([0-9+-]+)[ \t]*$");
    
    if(ajRegExec(acdRegExpLtI, str))
    {
	acdLog("acdRegExpLtI matched  '%S'\n", str);
	ajRegSubI(acdRegExpLtI, 1, &acdTmpStr);
	ajStrToInt(acdTmpStr, &ia);
	ajRegSubI(acdRegExpLtI, 2, &acdTmpStr);
	ajStrToInt(acdTmpStr, &ib);

	if(ia < ib)
	    ajFmtPrintS(result, "%b", ajTrue);
	else
	    ajFmtPrintS(result, "%b", ajFalse);

	acdLog("ia: %d < ib: %d = '%S'\n", ia, ib, *result);

	return ajTrue;
    }
    
    if(!acdRegExpLtD)				/* float + float */
	acdRegExpLtD = ajRegCompC("^[ \t]*([0-9.+-]+)[ \t]*[<][ \t]*"
			  "([0-9.+-]+)[ \t]*$");
    
    if(ajRegExec(acdRegExpLtD, str))
    {
	acdLog("acdRegExpLtD matched  '%S'\n", str);
	ajRegSubI(acdRegExpLtD, 1, &acdTmpStr);
	ajStrToDouble(acdTmpStr, &da);
	ajRegSubI(acdRegExpLtD, 2, &acdTmpStr);
	ajStrToDouble(acdTmpStr, &db);

	if(da < db)
	    ajFmtPrintS(result, "%b", ajTrue);
	else
	    ajFmtPrintS(result, "%b", ajFalse);

	acdLog("da: %f < db: %f = '%S'\n", da, db, *result);

	return ajTrue;
    }
    
    if(!acdRegExpLtT)				/* float + float */
	acdRegExpLtT = ajRegCompC("^[ \t]*([^ \t]+)[ \t]*[<][ \t]*"
			  "([^ \t]+)[ \t]*$");
    
    if(ajRegExec(acdRegExpLtT, str))
    {
	acdLog("acdRegExpLtT matched  '%S'\n", str);
	ajRegSubI(acdRegExpLtT, 1, &acdTmpStr);
	ajRegSubI(acdRegExpLtT, 2, &acdTmpStr2);

	if(0 < ajStrCmpCaseS(acdTmpStr2, acdTmpStr))
	    ajFmtPrintS(result, "%b", ajTrue);
	else
	    ajFmtPrintS(result, "%b", ajFalse);

	acdLog("ta: '%S' < tb: '%S' = '%S'\n", acdTmpStr, acdTmpStr2,
	       *result);

	return ajTrue;
    }
    
    return ajFalse;
}




/* @funcstatic acdExpOr *******************************************************
**
** Looks for and resolves an expression \@( num | num )
**
** @param [w] result [AjPStr*] Expression result
** @param [r] str [const AjPStr] String with possible expression
** @return [AjBool] ajTrue if successfully resolved
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdExpOr(AjPStr* result, const AjPStr str)
{
    ajint ia;
    ajint ib;
    double da;
    double db;
    AjBool ba;
    AjBool bb;
    
    if(!acdRegExpOrI)				/* ajint + ajint */
	acdRegExpOrI = ajRegCompC("^[ \t]*([0-9+-]+)[ \t]*[|][ \t]*"
			  "([0-9+-]+)[ \t]*$");
    
    if(ajRegExec(acdRegExpOrI, str))
    {
	acdLog("acdRegExpOrI matched  '%S'\n", str);
	ajRegSubI(acdRegExpOrI, 1, &acdTmpStr);
	ajStrToInt(acdTmpStr, &ia);
	ajRegSubI(acdRegExpOrI, 2, &acdTmpStr);
	ajStrToInt(acdTmpStr, &ib);

	if(ia || ib)
	    ajFmtPrintS(result, "%b", ajTrue);
	else
	    ajFmtPrintS(result, "%b", ajFalse);

	acdLog("ia: %d | ib: %d = '%S'\n", ia, ib, *result);

	return ajTrue;
    }

    if(!acdRegExpOrD)				/* float + float */
	acdRegExpOrD = ajRegCompC("^[ \t]*([0-9.+-]+)[ \t]*[|][ \t]*"
			  "([0-9.+-]+)[ \t]*$");
    
    if(ajRegExec(acdRegExpOrD, str))
    {
	acdLog("acdRegExpOrD matched  '%S'\n", str);
	ajRegSubI(acdRegExpOrD, 1, &acdTmpStr);
	ajStrToDouble(acdTmpStr, &da);
	ajRegSubI(acdRegExpOrD, 2, &acdTmpStr);
	ajStrToDouble(acdTmpStr, &db);

	if(da || db)
	    ajFmtPrintS(result, "%b", ajTrue);
	else
	    ajFmtPrintS(result, "%b", ajFalse);

	acdLog("da: %f | db: %f = '%S'\n", da, db, *result);

	return ajTrue;
    }
    
    if(!acdRegExpOrT)				/* char + char */
	acdRegExpOrT = ajRegCompC("^[ \t]*([^ \t]+)[ \t]*[|][ \t]*"
			  "([^ \t]+)[ \t]*$");
    
    if(ajRegExec(acdRegExpOrT, str))
    {
	acdLog("acdRegExpOrT matched  '%S'\n", str);
	ajRegSubI(acdRegExpOrT, 1, &acdTmpStr);
	ajRegSubI(acdRegExpOrT, 2, &acdTmpStr2);
	ajStrToBool(acdTmpStr2,&ba);
	ajStrToBool(acdTmpStr, &bb);

	if( ba || bb )
	    ajFmtPrintS(result, "%b", ajTrue);
	else
	    ajFmtPrintS(result, "%b", ajFalse);

	acdLog("ta: '%S' | tb: '%S' = '%S'\n", acdTmpStr, acdTmpStr2,
	       *result);

	return ajTrue;
    }
    
    return ajFalse;
}




/* @funcstatic acdExpAnd ******************************************************
**
** Looks for and resolves an expression \@( num & num )
**
** @param [w] result [AjPStr*] Expression result
** @param [r] str [const AjPStr] String with possible expression
** @return [AjBool] ajTrue if successfully resolved
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdExpAnd(AjPStr* result, const AjPStr str)
{
    ajint ia;
    ajint ib;
    double da;
    double db;
    AjBool ba;
    AjBool bb;
    
    if(!acdRegExpAndI)				/* ajint + ajint */
	acdRegExpAndI = ajRegCompC("^[ \t]*([0-9+-]+)[ \t]*[&][ \t]*"
			  "([0-9+-]+)[ \t]*$");
    
    if(ajRegExec(acdRegExpAndI, str))
    {
	acdLog("acdRegExpAndI matched  '%S'\n", str);
	ajRegSubI(acdRegExpAndI, 1, &acdTmpStr);
	ajStrToInt(acdTmpStr, &ia);
	ajRegSubI(acdRegExpAndI, 2, &acdTmpStr);
	ajStrToInt(acdTmpStr, &ib);

	if(ia && ib)
	    ajFmtPrintS(result, "%b", ajTrue);
	else
	    ajFmtPrintS(result, "%b", ajFalse);

	acdLog("ia: %d & ib: %d = '%S'\n", ia, ib, *result);

	return ajTrue;
    }
    
    if(!acdRegExpAndD)				/* float + float */
	acdRegExpAndD = ajRegCompC("^[ \t]*([0-9.+-]+)[ \t]*[&][ \t]*"
			  "([0-9.+-]+)[ \t]*$");
    if(ajRegExec(acdRegExpAndD, str))
    {
	acdLog("acdRegExpAndD matched  '%S'\n", str);
	ajRegSubI(acdRegExpAndD, 1, &acdTmpStr);
	ajStrToDouble(acdTmpStr, &da);
	ajRegSubI(acdRegExpAndD, 2, &acdTmpStr);
	ajStrToDouble(acdTmpStr, &db);

	if(da && db)
	    ajFmtPrintS(result, "%b", ajTrue);
	else
	    ajFmtPrintS(result, "%b", ajFalse);

	acdLog("da: %f & db: %f = '%S'\n", da, db, *result);

	return ajTrue;
    }

    if(!acdRegExpAndT)				/* char + char */
	acdRegExpAndT = ajRegCompC("^[ \t]*([^ \t]+)[ \t]*[&][ \t]*"
			  "([^ \t]+)[ \t]*$");
    
    if(ajRegExec(acdRegExpAndT, str))
    {
	acdLog("acdRegExpAndT matched  '%S'\n", str);
	ajRegSubI(acdRegExpAndT, 1, &acdTmpStr);
	ajRegSubI(acdRegExpAndT, 2, &acdTmpStr2);
	ajStrToBool(acdTmpStr2,&ba);
	ajStrToBool(acdTmpStr, &bb);

	if( ba && bb )
	    ajFmtPrintS(result, "%b", ajTrue);
	else
	    ajFmtPrintS(result, "%b", ajFalse);

	acdLog("ta: '%S' & tb: '%S' = '%S'\n", acdTmpStr, acdTmpStr2,
	       *result);

	return ajTrue;
    }
    
    return ajFalse;
}




/* @funcstatic acdExpCond *****************************************************
**
** Looks for and resolves an expression \@( bool ? trueval : falseval )
**
** @param [w] result [AjPStr*] Expression result
** @param [r] str [const AjPStr] String with possible expression
** @return [AjBool] ajTrue if successfully resolved
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdExpCond(AjPStr* result, const AjPStr str)
{
    AjBool ba;

    if(!acdRegExpCond)			/* bool ? iftrue : iffalse */
	acdRegExpCond = ajRegCompC("^[ \t]*([.A-Za-z0-9+-]*)[ \t]*[?]"
				   "[ \t]*([^: \t]+)[ \t]*[:]"
				   "[ \t]*([^: \t]+)[ \t]*$");

    if(ajRegExec(acdRegExpCond, str))
    {
	ajRegSubI(acdRegExpCond, 1, &acdTmpStr);
	ajStrToBool(acdTmpStr, &ba);

	if(ba)
	    ajRegSubI(acdRegExpCond, 2, result);
	else
	    ajRegSubI(acdRegExpCond, 3, result);

	acdLog("ba: %B = '%S'\n", ba, *result);

	return ajTrue;
    }

    return ajFalse;
}




/* @funcstatic acdExpOneof ****************************************************
**
** Looks for and resolves an expression as a test for a list of values
** \@( var == { vala | valb | valc } )
** \@( var != { vala | valb | valc } )
**
** @param [w] result [AjPStr*] Expression result
** @param [r] str [const AjPStr] String with possible expression
** @return [AjBool] ajTrue if successfully resolved
**
** @release 2.9.0
** @@
******************************************************************************/

static AjBool acdExpOneof(AjPStr* result, const AjPStr str)
{
    AjBool todo;
    
    AjPStr testvar = NULL;
    AjPStr notvar = NULL;
    AjPStr restvar = NULL;
    AjPStr elsevar = NULL;

    if(!acdRegExpOneofCase)	 /* value = ( vala | valb | valc} ) */
	acdRegExpOneofCase = ajRegCompC("^[ \t]*([A-Za-z0-9+-]+)[ \t]*"
					"([!=])[=][ \t]*[{]");
    if(!acdRegExpOneofList)			/* case : value */
	acdRegExpOneofList = ajRegCompC("^[ \t]*([^| \t]+)[ \t]*[|}]");
    
    if(ajRegExec(acdRegExpOneofCase, str))
    {
	ajRegSubI(acdRegExpOneofCase, 1, &testvar);
	ajRegSubI(acdRegExpOneofCase, 2, &notvar);	/* "!" or empty */
	
	if(!ajRegPost(acdRegExpOneofCase, &restvar)) /* any more? */
	{
	    ajStrDel(&testvar);
	    ajStrDel(&notvar);

	    return ajFalse;
	}

	ajStrAssignClear(&elsevar);
	todo = ajTrue;

	while(todo && ajRegExec(acdRegExpOneofList, restvar))
	{
	    ajRegSubI(acdRegExpOneofList, 1, &acdTmpStr);

	    if(ajStrMatchS(acdTmpStr, testvar)) /* match, but did we
						     want to find it? */
	    {
		if (ajStrGetCharFirst(notvar) == '=')
		    ajStrAssignC(result, "Y");
		else
		    ajStrAssignC(result, "N");
		ajStrDel(&testvar);
		ajStrDel(&notvar);
		ajStrDel(&elsevar);
		ajStrDel(&restvar);

		return ajTrue;
	    }

	    todo = ajRegPost(acdRegExpOneofList, &restvar);
	}
	/* no match, but did we not
	   want to find it? */
	if (ajStrGetCharFirst(notvar) == '=')
	    ajStrAssignC(result, "N");
	else
	    ajStrAssignC(result, "Y");

	ajStrDel(&testvar);
	ajStrDel(&notvar);
	ajStrDel(&elsevar);
	ajStrDel(&restvar);

	return ajTrue;
    }
    
    ajStrDel(&testvar);
    ajStrDel(&notvar);
    ajStrDel(&elsevar);
    ajStrDel(&restvar);

    return ajFalse;
}




/* @funcstatic acdExpCase *****************************************************
**
** Looks for and resolves an expression as a switch/case statement
** \@( var = casea : vala, caseb: valb else: val )
**
** @param [w] result [AjPStr*] Expression result
** @param [r] str [const AjPStr] String with possible expression
** @return [AjBool] ajTrue if successfully resolved
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdExpCase(AjPStr* result, const AjPStr str)
{
    ajint ifound;
    AjBool todo;
    
    static AjPStr testvar = NULL;
    static AjPStr restvar = NULL;
    static AjPStr elsevar = NULL;

    if(!acdRegExpCaseCase)		    /* value = (case : value,  ...) */
	acdRegExpCaseCase = ajRegCompC("^[ \t]*([A-Za-z0-9+-]+)[ \t]*[=]");
    if(!acdRegExpCaseList)			/* case : value */
	acdRegExpCaseList = ajRegCompC("^[ \t]*([^: \t]+)[ \t]*[:]+"
				       "[ \t]*([^: \t,]+)[ \t,]*");
    
    if(ajRegExec(acdRegExpCaseCase, str))
    {
	ajRegSubI(acdRegExpCaseCase, 1, &testvar);
	
	if(!ajRegPost(acdRegExpCaseCase, &restvar)) /* any more? */
	{
	    ajStrDel(&testvar);
	    ajStrDel(&elsevar);
	    ajStrDel(&restvar);

	    return ajFalse;
	}

	ajStrAssignClear(&elsevar);
	todo = ajTrue;
	ifound = 0;

	while(todo && ajRegExec(acdRegExpCaseList, restvar))
	{
	    ajRegSubI(acdRegExpCaseList, 1, &acdTmpStr);

	    if(ajStrMatchC(acdTmpStr, "else")) /* default */
		ajRegSubI(acdRegExpCaseList, 2, &elsevar);

	    if(ajStrMatchS(acdTmpStr, testvar)) /* match, take the value */
	    {
		ajRegSubI(acdRegExpCaseList, 2, result);
		acdLog("%S == %S : '%S'\n", testvar, acdTmpStr, *result);
		ajStrDel(&testvar);
		ajStrDel(&elsevar);
		ajStrDel(&restvar);

		return ajTrue;
	    }

	    if(ajStrPrefixS(testvar, acdTmpStr))
	    {
		ifound++;
		ajRegSubI(acdRegExpCaseList, 2, result);
	    }

	    todo = ajRegPost(acdRegExpCaseList, &restvar);
	}
	
	if(ifound)		   /* let ambiguous matches through */
	{
	    if(ifound > 1)
	    {
		acdLog("@(=) ambiguous match, last match accepted %S\n",
		       testvar);
		acdLog("@(=) ambiguous match, last match accepted %S\n",
		       testvar);
	    }

	    acdLog("%S ~= %S : '%S'\n", testvar, acdTmpStr, *result);

	    if (acdDoValid)
		acdWarn("Ambiguous case expression '%S' (%S)",
			testvar, *result);
	    ajStrDel(&testvar);
	    ajStrDel(&elsevar);
	    ajStrDel(&restvar);

	    return ajTrue;
	}

	if(ifound == 0)
	{
	    ajStrAssignS(result, elsevar);
	    acdLog("%S != else : '%S'\n", testvar, *result);
	    ajStrDel(&testvar);
	    ajStrDel(&elsevar);
	    ajStrDel(&restvar);

	    return ajTrue;
	}
    }
    
    ajStrDel(&testvar);
    ajStrDel(&elsevar);
    ajStrDel(&restvar);

    return ajFalse;
}




/* @funcstatic acdExpFilename *************************************************
**
** Looks for an expression \@(file: string) and returns a trimmed
** lower case file name prefix or suffix.
**
** @param [w] result [AjPStr*] Expression result
** @param [r] str [const AjPStr] String with possible expression
** @return [AjBool] ajTrue if successfully resolved
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdExpFilename(AjPStr* result, const AjPStr str)
{
    if(!acdRegExpFilename)				/* file: name */
	acdRegExpFilename =
	    ajRegCompC("^[ \t]*[Ff][Ii][Ll][Ee]:[ \t]*([^ \t]+)[ \t]*$");

    if(ajRegExec(acdRegExpFilename, str))
    {
	acdLog("acdRegExpFilename matched  '%S'\n", str);
	ajRegSubI(acdRegExpFilename, 1, &acdTmpStr);
	ajStrAssignS(result, acdTmpStr);
	ajFilenameTrimAll(result);
	ajStrFmtLower(result);
	acdLog("file: %S = '%S'\n", acdTmpStr, *result);

	return ajTrue;
    }

    return ajFalse;
}




/* @funcstatic acdExpExists ***************************************************
**
** Looks for an expression \@(is string) and returns ajTrue
** if there is a value, and ajFalse if there is none
**
** @param [w] result [AjPStr*] Expression result
** @param [r] str [const AjPStr] String with possible expression
** @return [AjBool] ajTrue if successfully resolved
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdExpExists(AjPStr* result, const AjPStr str)
{
    AjBool test;

    if(!acdRegExpFileExists)				/* file: name */
	acdRegExpFileExists =
	    ajRegCompC("^[ \t]*[Ii][Ss]:[ \t]*([^ \t]*)[ \t]*$");

    if(ajRegExec(acdRegExpFileExists, str))
    {
	acdLog("acdRegExpFileExists matched  '%S'\n", str);
	ajRegSubI(acdRegExpFileExists, 1, &acdTmpStr);

	if(ajStrGetLen(acdTmpStr))
	    test = ajTrue;
	else
	    test = ajFalse;

	ajFmtPrintS(result, "%b", test);
	acdLog("test: '%S' = '%S'\n", acdTmpStr, *result);

	return ajTrue;
    }

    return ajFalse;
}




/* @funcstatic acdExpValue ****************************************************
**
** Looks for an expression \@(value string) and returns ajTrue
** if there is a value, and ajFalse if there is none
**
** @param [w] result [AjPStr*] Expression result
** @param [r] str [const AjPStr] String with possible expression
** @return [AjBool] ajTrue if successfully resolved
**
** @release 6.1.0
** @@
******************************************************************************/

static AjBool acdExpValue(AjPStr* result, const AjPStr str)
{
    if(!acdRegExpValue)				/* file: name */
	acdRegExpValue =
	    ajRegCompC("^[ \t]*[Vv][Aa][Ll][Uu][Ee]:[ \t]*([^ \t]*)[ \t]*$");

    if(ajRegExec(acdRegExpValue, str))
    {
	acdLog("acdRegExpValue matched  '%S'\n", str);
	ajRegSubI(acdRegExpValue, 1, &acdTmpStr);

        if(!ajNamGetValueS(acdTmpStr, result))
            ajNamGetenvS(acdTmpStr, result);
	acdLog("test: '%S' = '%S'\n", acdTmpStr, *result);

	return ajTrue;
    }

    return ajFalse;
}




/* @funcstatic acdVarSplit ****************************************************
**
** Splits a variable reference into name and attribute.
** Attribute is "default" if not specified
**
** @param [r] var [const AjPStr] Variable reference
** @param [w] name [AjPStr*] Variable name
** @param [w] pattrname [AjPStr*] Attribute name, or "default" if not set.
** @return [AjBool] ajTrue if successfully split
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdVarSplit(const AjPStr var, AjPStr* name, AjPStr* pattrname)
{
    ajlong i;

    ajStrAssignS(name, var);

    i = ajStrFindC(*name, ".");		/* qualifier with value */

    if(i > 0)
    {
	ajStrAssignS(pattrname, var);
	ajStrKeepRange(name, 0, i-1); /* strip any value and keep testing */
	ajStrCutStart(pattrname, (size_t) (i+1));
    }
    else
	ajStrDelStatic(pattrname);

    return ajTrue;
}




/* @funcstatic acdAttrTest ****************************************************
**
** Tests for the existence of a named attribute
**
** @param [r] thys [const AcdPAcd] ACD item
** @param [r] attrib [const char*] Attribute name
** @return [AjBool] ajTrue if the named attribute exists
**
** @release 2.7.0
** @@
******************************************************************************/

static AjBool acdAttrTest(const AcdPAcd thys,const  char *attrib)
{
    AcdPAttr attr;
    AcdPAttr defattr = acdAttrDef;
    ajint i;

    if(acdIsQtype(thys))
	attr = acdType[thys->Type].Attr;
    else
	attr = acdKeywords[thys->Type].Attr;

    i = acdFindAttrC(attr, attrib);

    if(i >= 0)
	return ajTrue;

    if(thys->DefStr)
    {
	i = acdFindAttrC(defattr, attrib);

	if(i >= 0)
	    return ajTrue;
    }

    return ajFalse;
}




/* @funcstatic acdAttrTestDefined *********************************************
**
** Tests for the existence of a named attribute with a value
**
** @param [r] thys [const AcdPAcd] ACD item
** @param [r] attrib [const char*] Attribute name
** @return [AjBool] ajTrue if the named attribute exists
**
** @release 3.0.0
** @@
******************************************************************************/

static AjBool acdAttrTestDefined(const AcdPAcd thys,const  char *attrib)
{
    AcdPAttr attr;
    AjPStr  *attrstr;
    AcdPAttr defattr = acdAttrDef;
    AjPStr  *defstr;
    ajint i;

    attrstr = thys->AttrStr;
    defstr = thys->DefStr;


    if(acdIsQtype(thys))
	attr = acdType[thys->Type].Attr;
    else
	attr = acdKeywords[thys->Type].Attr;

    i = acdFindAttrC(attr, attrib);

    if(i >= 0)
    {
	if (ajStrGetLen(attrstr[i]))
	    return ajTrue;
	else
	    return ajFalse;
    }


    if(thys->DefStr)
    {
	i = acdFindAttrC(defattr, attrib);

	if(i >= 0)
	{
	    if (ajStrGetLen(defstr[i]))
		return ajTrue;
	    else
		return ajFalse;
	}
    }

    return ajFalse;
}




/* @funcstatic acdAttrTestValue ***********************************************
**
** Tests for the existence of a named attribute with a simple value
**
** @param [r] thys [const AcdPAcd] ACD item
** @param [r] attrib [const char*] Attribute name
** @return [AjBool] ajTrue if the named attribute exists
**
** @release 4.1.0
** @@
******************************************************************************/

static AjBool acdAttrTestValue(const AcdPAcd thys,const  char *attrib)
{
    AcdPAttr attr;
    AjPStr  *attrstr;
    AcdPAttr defattr = acdAttrDef;
    AjPStr  *defstr;
    ajint i;

    attrstr = thys->AttrStr;
    defstr = thys->DefStr;


    if(acdIsQtype(thys))
	attr = acdType[thys->Type].Attr;
    else
	attr = acdKeywords[thys->Type].Attr;

    i = acdFindAttrC(attr, attrib);

    if(i >= 0)
    {
	if (ajStrGetLen(attrstr[i]) && ajStrFindAnyK(attrstr[i], '$') < 0)
	    return ajTrue;
	else
	    return ajFalse;
    }


    if(thys->DefStr)
    {
	i = acdFindAttrC(defattr, attrib);

	if(i >= 0)
	{
	    if (ajStrGetLen(defstr[i]) && ajStrFindAnyK(defstr[i], '$') < 0)
		return ajTrue;
	    else
		return ajFalse;
	}
    }

    return ajFalse;
}




/* @funcstatic acdAttrValue ***************************************************
**
** Returns the string value for a named attribute
**
** @param [r] thys [const AcdPAcd] ACD item
** @param [r] attrib [const char*] Attribute name
** @return [const AjPStr] Attribute value.
** @cre Aborts if attribute is not found.
**
** @release 1.0.0
** @@
******************************************************************************/

static const AjPStr acdAttrValue(const AcdPAcd thys, const char *attrib)
{
    AcdPAttr attr;
    AjPStr  *attrstr;
    AcdPAttr defattr;
    AjPStr  *defstr;
    ajint i;

    attrstr = thys->AttrStr;
    defattr = acdAttrDef;
    defstr = thys->DefStr;


    if(acdIsQtype(thys))
	attr = acdType[thys->Type].Attr;
    else
	attr = acdKeywords[thys->Type].Attr;

    i = acdFindAttrC(attr, attrib);

    if(i >= 0)
	return attrstr[i];

    if(thys->DefStr)
    {
	i = acdFindAttrC(defattr, attrib);

	if(i >= 0)
	    return defstr[i];
    }

    if(i < 0)
	acdErrorAcd(thys, "Unknown attribute '%s'\n", attrib);

    return NULL;
}




/* @funcstatic acdAttrValueStr ************************************************
**
** Returns the string value for a named attribute
**
** @param [r] thys [const AcdPAcd] ACD item
** @param [r] attrib [const char*] Attribute name
** @param [r] def [const char*] Default value
** @param [w] str [AjPStr*] Attribute value
** @return [AjBool] ajTrue if success.
** @cre Aborts if attribute is not found.
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdAttrValueStr(const AcdPAcd thys,
			      const char *attrib, const char* def,
			      AjPStr *str)
{
    AcdPAttr attr;
    AjPStr  *attrstr;
    AcdPAttr defattr;
    AjPStr  *defstr;
    ajint i;

    attrstr = thys->AttrStr;
    defattr = acdAttrDef;
    defstr = thys->DefStr;

    if(acdIsQtype(thys))
	attr = acdType[thys->Type].Attr;
    else
	attr = acdKeywords[thys->Type].Attr;

    i = acdFindAttrC(attr, attrib);

    if(i >= 0)
    {
	ajStrAssignS(str, attrstr[i]);
	if(ajStrGetLen(*str))
	    return ajTrue;
	ajStrAssignC(str, def);
	return ajFalse;
    }

    if(thys->DefStr)
    {
	i = acdFindAttrC(defattr, attrib);

	if(i >= 0)
	{
	    ajStrAssignS(str, defstr[i]);

	    if(ajStrGetLen(*str))
		return ajTrue;

	    ajStrAssignC(str, def);

	    return ajFalse;
	}
    }

    if(i < 0)
	acdErrorAcd(thys, "Unknown attribute %s\n", attrib);

    return ajFalse;
}




/* @section internals *********************************************************
**
** Sets internal values
**
** @fdata [none]
**
** @nam3rule Set Set an internal value
** @nam4rule SetControl Set a control variable
**
** @argrule SetControl optionName [const char*] Control variable name
**
** @valrule * [AjBool]
** @fcategory misc
**
******************************************************************************/




/* @func ajAcdSetControl ******************************************************
**
** Sets special qualifiers which were originally provided via the
** command line.
**
** Sets special internal variables to reflect their presence.
**
** Currently these are "acdhelp", "acdlog", "acdpretty", "acdtable",
** "acdtrace", "acdvalid", "acdverbose", "acdxsd" and "acdnocommandline"
**
** @param [r] optionName [const char*] option name
** @return [AjBool] ajTrue if option was recognised
**
** @release 2.7.0
** @@
******************************************************************************/

AjBool ajAcdSetControl(const char* optionName)
{
    AcdPAcd vacd;
    AjPStr varname = NULL;

    if(!ajCharCmpCase(optionName, "acdhelp"))
    {
	acdDoHelp = ajTrue;

	return ajTrue;
    }

    if(!ajCharCmpCase(optionName, "acdlog"))
    {
	acdDoLog = ajTrue;

	return ajTrue;
    }

    if(!ajCharCmpCase(optionName, "acdnocommandline"))
    {
	acdCommandLine = ajFalse;

	return ajTrue;
    }

    if(!ajCharCmpCase(optionName, "acdpretty"))
    {
	acdDoPretty = ajTrue;

	return ajTrue;
    }

    if(!ajCharCmpCase(optionName, "acdtable"))
    {
	acdDoTable = ajTrue;

	return ajTrue;
    }

    if(!ajCharCmpCase(optionName, "acdtrace"))
    {
	acdDoTrace = ajTrue;

	return ajTrue;
    }

    if(!ajCharCmpCase(optionName, "acdvalid"))
    {
	acdDoValid = ajTrue;

	return ajTrue;
    }

    if(!ajCharCmpCase(optionName, "acdverbose"))
    {
	acdVerbose = ajTrue;

	return ajTrue;
    }

    if(!ajCharCmpCase(optionName, "acdgalaxy"))
    {
	acdDoGalaxy = ajTrue;

	return ajTrue;
    }
    if(!ajCharCmpCase(optionName, "acdgalaxynucleotide"))
    {
	acdDoGalaxyNuc = ajTrue;

        ajStrAssignC(&varname, "acdprotein");
        ajStrAssignC(&acdVarAcdProtein, "N");
        vacd = acdNewVar(varname);
        acdSetVarDef(vacd, acdVarAcdProtein);
        ajStrDel(&varname);

	return ajTrue;
    }
    if(!ajCharCmpCase(optionName, "acdgalaxyprotein"))
    {
	acdDoGalaxyProt = ajTrue;

        ajStrAssignC(&varname, "acdprotein");
        ajStrAssignC(&acdVarAcdProtein, "Y");
        vacd = acdNewVar(varname);
        acdSetVarDef(vacd, acdVarAcdProtein);
        ajStrDel(&varname);

	return ajTrue;
    }
    if(!ajCharCmpCase(optionName, "acdxsd"))
    {
	acdDoXsd = ajTrue;

	return ajTrue;
    }

    /* program source error */
    ajDie("Unknown ajAcdSetControl control option '%s'", optionName);

    return ajFalse;
}




/* @funcstatic acdArgsScan ****************************************************
**
** Steps through the command line and checks for special qualifiers.
** Sets special internal variables to reflect their presence.
**
** Currently these are "-debug", "-stdout", "-filter", "-options"
** "-help" and "-auto", plus the message controls
** "-warning", "-error", "-fatal", "-die"
**
** @param [r] argc [ajint] Number of arguments
** @param [r] argv [char* const[]] Actual arguments as a text array.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdArgsScan(ajint argc, char * const argv[])
{
    ajint i;
    char* cp;

    for(i=0; i < argc; i++)
    {
	cp = argv[i];

	if(*cp && strchr("-/", *cp))  /* first character vs. qualifier starts */
	{
	    cp++;

	    if(*cp == '-')   /* allow --qualifier */
		cp++;
	}
	else
	    continue;

	if(!*cp)
	    continue;

	if(!strcmp(cp, "debug"))
	{
	    acdDebug = ajTrue;
	    acdDebugSet = ajTrue;
	}

	if(!strcmp(cp, "nodebug"))
	{
	    acdDebug = ajFalse;
	    acdDebugSet = ajTrue;
	}

	if(!strcmp(cp, "stdout"))
	    acdStdout = ajTrue;

	if(!strcmp(cp, "filter"))
	    acdFilter = ajTrue;

	if(!strcmp(cp, "options"))
	    acdOptions = ajTrue;

	if(!strcmp(cp, "verbose"))
	    acdVerbose = ajTrue;

	if(!strcmp(cp, "version"))
	    acdDoVersion = ajTrue;

	if(!strcmp(cp, "help"))
	    acdDoHelp = ajTrue;

	if(!strcmp(cp, "auto"))
        {
            acdPromptTry = 1;
	    acdAuto = ajTrue;
        }

	if(!strcmp(cp, "warning"))
	    AjErrorLevel.warning = ajTrue;

	if(!strcmp(cp, "nowarning"))
	    AjErrorLevel.warning = ajFalse;

	if(!strcmp(cp, "error"))
	    AjErrorLevel.error = ajTrue;

	if(!strcmp(cp, "noerror"))
	    AjErrorLevel.error = ajFalse;

	if(!strcmp(cp, "-fatal"))
	    AjErrorLevel.fatal = ajTrue;

	if(!strcmp(cp, "nofatal"))
	    AjErrorLevel.fatal = ajFalse;

	if(!strcmp(cp, "die"))
	    AjErrorLevel.die = ajTrue;

	if(!strcmp(cp, "nodie"))
	    AjErrorLevel.die = ajFalse;

	if(!strcmp(cp, "help"))
	    acdLog("acdArgsScan -help argv[%d]\n", i);
    }

    acdLog("acdArgsScan acdDebug %B acdDoHelp %B\n", acdDebug, acdDoHelp);

    return;
}




/* @funcstatic acdArgsParse ***************************************************
**
** Steps through the command line and compares to the stored command structure.
** Capable of cunning tricks such as matching values to qualifiers if they fit,
** and otherwise treating them as parameters.
**
** @param [r] argc [ajint] Number of arguments
** @param [r] argv [char* const[]] Actual arguments as a text array.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdArgsParse(ajint argc, char * const argv[])
{    
    ajint i;
    ajint j;
    ajint number;
    
    ajint iparam = 0;			/* expected next param */
    ajint jparam = 0;			/* param found */
    ajint itestparam = 0;
    ajint jtestparam = 0;
    AcdPAcd acd;
    AcdPAcd acd2;
    
    const char *cp;
    const char *cq;
    
    AjPStr qual  = NULL;
    AjPStr noqual = NULL;
    AjPStr value = NULL;
    AjPStr param = NULL;
    AjPStr token = NULL;
    AjPStr master = NULL;
    AjPStr argvalstr = NULL;
    
    acdLog("ArgsParse\n=========\n");
    
    acdLog("\n");
    
    acdMasterQual = NULL;
/*    acdQualTestSkip = ajFalse; */

    i = 1;

    while(i < argc)
    {
	acdLog("%s ", argv[i]);
	i++;
    }

    acdLog("\n");
    acdLog("\n");
    
    i = 1;				/* skip the program name */
    
    while(i < argc)
    {
	cp = argv[i];

	if((i+1) < argc)
            cq = argv[i+1];
	else
            cq = NULL;

	if(acdArgSave)
	    ajStrAppendK(&acdArgSave, '\n');

	acdLog("\n");
	acdLog("argv[%d] <%s>", i, cp);

	if(cq)
	    acdLog(" + argv[%d] <%s>", i+1, cq);

	acdLog("\n");
	jparam = 0;

	if((j = acdIsQual(cp, cq, &jparam, &qual, &noqual, 
			  &value, &number, &master, &acd)))
	{
	    if(jparam)
	    {
		acdLog("Parameter (%d) ", jparam);
		acdParamSet[jparam-1] = ajTrue;

		if(iparam == (jparam-1))
		{
		    iparam = acdNextParam(iparam);
		    acdLog("reset iparam = %d\n", iparam);
		}
		else
		    acdLog("keep iparam = %d\n", iparam);
	    }
	    else
		acdLog("Qualifier ");

	    acdLog("-%S ", noqual);

	    if(number)
		acdLog("[%d] ", number);

	    if(ajStrGetLen(value))
		acdLog("= '%S'", value);

	    acdLog("\n");
	    
	    /*
	     ** acdFindQualDetail dies (Unknown qualifier) if acd is not set,
	     ** so we are safe here
	     */
	    
	    acdDef(acd, value);
	    acdLog("set qualifier -%S[%d] (param %d) = %S\n",
		   acd->Name, acd->PNum, jparam, value);

	    /* loop over any associated qualifiers for the rest */
	    acdLog("number: %d jparam: %d acd->PNum: %d acdNParam: %d\n",
		   number, jparam, acd->PNum, acdNParam);
	    
            if(!ajStrGetLen(master) && !number && !jparam && acd->PNum)
	    {
		for(itestparam = acd->PNum+1; itestparam <= acdNParam;
		    itestparam++)
		{
		    acdLog("test [%d] '%S'\n", itestparam, qual);
		    acd2 = acdFindQualDetail(qual, NULL, NULL,
					    itestparam, &jtestparam);
		    if(acd2)
		    {
			acdDef(acd2, value);
			acdLog("set next qualifier -%S[%d] (param %d) = %S\n",
			       acd2->Name, acd2->PNum, jparam, value);
		    }
		    else
			acdLog("no -%S[%d]\n", qual, itestparam);
		}
	    }
	    
	    acd->UserDefined = ajTrue;
	    ajStrAppendK(&acdArgSave, '-');
	    ajStrAppendS(&acdArgSave, noqual);

	    if(number)
		ajFmtPrintAppS(&acdArgSave, "%d", number);
	    else if(ajStrGetLen(master))
		ajFmtPrintAppS(&acdArgSave, "_%S", master);

	    if(j==2)
	    {
		i++;
	    }

	    ajStrAssignS(&argvalstr, value);

	    if(!strcmp(acdType[acd->Type].Name, "boolean") ||
	       !strcmp(acdType[acd->Type].Name, "toggle"))
	    {
		if(ajStrMatchS(qual, noqual)) /* -boolqual */
		{
		    if(ajStrMatchC(value, "Y"))
			ajStrAssignClear(&argvalstr);
		}
		else if(ajStrMatchC(value, "N")) /* -noboolqual */
			ajStrAssignClear(&argvalstr);

		if(ajStrGetLen(argvalstr)) /* non-trivial boolean values */
		{
		    ajStrAppendK(&acdArgSave, ' ');
		    ajStrAppendS(&acdArgSave, argvalstr);
		}
	    }
	    else if(ajStrIsWord(value) &&
		    (ajStrFindAnyC(value, "*?[]{}|!&^") < 0))
	    {
		ajStrAppendK(&acdArgSave, ' ');
		ajStrAppendS(&acdArgSave, value);
	    }
	    else if(ajStrMatchS(qual, noqual)) /* not -nomissfile */
	    {
		ajStrAppendK(&acdArgSave, ' ');
		ajStrAppendK(&acdArgSave, '\"');
		ajStrAppendS(&acdArgSave, value);
		ajStrAppendK(&acdArgSave, '\"');
	    }
	}
	else		    /* not a qualifier - assume a parameter */
	{
	    iparam = acdNextParam(0);	/* first free parameter */
	    acdIsParam(cp, &param, &iparam, &acd); /* die if too many */

	    acd->UserDefined = ajTrue;
	    ajStrAppendC(&acdArgSave, "[-");
	    ajStrAppendS(&acdArgSave, acd->Name);
	    ajStrAppendC(&acdArgSave, "] ");

	    if(acdIsParamValue(param))
	    {
		acdLog("Parameter %d: %S = %S\n",
		       iparam, acd->Name, param);
		acdDef(acd, param);
		acdParamSet[iparam-1] = ajTrue;
		ajStrAssignC(&argvalstr, cp);

		if(ajStrIsWord(argvalstr) &&
		   (ajStrFindAnyC(argvalstr, "*?[]{}|!&^") < 0))
		    ajStrAppendS(&acdArgSave, argvalstr);
		else
		{
		    ajStrAppendK(&acdArgSave, '\"');
		    ajStrAppendS(&acdArgSave, argvalstr);
		    ajStrAppendK(&acdArgSave, '\"');
		}
	    }
	    else		 /* missing value "" ignored */
	    {
		acdLog("Parameter %d: %S = '%S' ** missing value **\n",
		       iparam, acd->Name, param);
		ajStrAssignClear(&param);
		acdDef(acd, param);
		acdParamSet[iparam-1] = ajTrue;
		ajStrAppendC(&acdArgSave, "\"\"");
	    }
	}

	i++;
    }
    
    ajStrDel(&qual);
    ajStrDel(&noqual);
    ajStrDel(&value);
    ajStrDel(&param);
    ajStrDel(&token);
    ajStrDel(&argvalstr);
    
    return;
}




/* @funcstatic acdIsParamValue ************************************************
**
** Tests whether a parameter value is 'missing', in which case
** it will be ignored for now.
**
** @param [r] pval [const AjPStr] Parameter value
**
** @return [AjBool] ajFalse for a missing value.
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdIsParamValue(const AjPStr pval)
{
    if(!ajStrGetLen(pval))
	return ajFalse;

    return ajTrue;
}




/* @funcstatic acdNextParam ***************************************************
**
** Returns the next unknown parameter. Used for cases where parameters
** are specified by qualifier before their turn on the command line
**
** @param [r] pnum [ajint] Current parameter number
**
** @return [ajint] next undefined parameter
**
** @release 1.0.0
** @@
******************************************************************************/

static ajint acdNextParam(ajint pnum)
{
    ajint i;

    if(pnum > acdNParam)
	return pnum+1;                  /* all done */

    for(i=pnum;i<acdNParam;i++)
	if(!acdParamSet[i])
	    return i;	                /* next free parameter */

    return acdNParam+1;			/* all done */
}




/* @funcstatic acdIsParam *****************************************************
**
** Tests an argument to see whether it could be a parameter.
** Qualifiers start with "-" or "/", or are built as qual=value where
** "qual" is a known qualifier.
** Parameters are any other text.
**
** @param [r] arg [const char*] Argument
** @param [w] param [AjPStr*] Parameter text copied on success
** @param [w] iparam [ajint*] Parameter number incremented on success
** @param [w] acd [AcdPAcd*] ACD item for the current parameter
** @return [AjBool] ajTrue if "arg" could be a parameter
**         ajFalse if it appears to be a qualifier (starts with "-" and
**         ends with a name)
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdIsParam(const char* arg, AjPStr* param, ajint* iparam,
			 AcdPAcd* acd)
{
    const char *cp;

    cp = arg;

    acdLog("acdIsParam arg: '%s' param: '%S' iparam: %d\n",
	   arg, *param, *iparam);

    if(*iparam >= acdNParam)		/* test acdc-toomanyparam */
    {
	ajErr("Argument '%s' : Too many parameters %d/%d",
	      arg, (*iparam), acdNParam);
	ajExitBad();
    }

    (*iparam)++;
    *acd = acdFindParam(*iparam);

    if(!*cp)                                    /* missing value */
    {
	ajStrAssignClear(param);		/* clear the parameter */

	return ajTrue;
    }

    ajStrAssignC(param, arg);		/* copy the argument value */
    if(*acd)
    {
	if((*acd)->AssocQuals)
	{
	    acdLog("acdMasterQual [param] set to -%S\n", (*acd)->Name);
	    acdMasterQual = *acd;
	}
	else if(acdMasterQual)
	{
	    acdLog("acdMasterQual cleared, was -%S\n", acdMasterQual->Name);
	    acdMasterQual = NULL;
	}

	return ajTrue;
    }

    return ajFalse;
}




/* @funcstatic acdIsQual ******************************************************
**
** Tests an argument to see whether it is a qualifier
** Qualifiers start with "-".
** Qualifiers are assumed to take a value, which is either
** delimited by an "=" sign or is the next argument.
** Qualifiers can also have a numbered suffix if matching one of the parameters
** or a specific master qualifier as a suffix after an underscore.
**
** @param [r] arg [const char*] Argument
** @param [r] arg2 [const char*] Next argument
** @param [w] iparam [ajint*] Parameter number
** @param [w] pqual [AjPStr*] Qualifier name copied on success
** @param [w] pnoqual [AjPStr*] Qualifier name with possible 'no' prefix
** @param [w] pvalue [AjPStr*] Qualifier value copied on success
** @param [w] number [ajint*] Qualifier number
** @param [w] pmaster [AjPStr*] Named master qualifier
** @param [w] acd [AcdPAcd*] Qualifier data
** @return [ajint] Number of arguments consumed
**
** @release 1.0.0
** @@
******************************************************************************/

static ajint acdIsQual(const char* arg, const char* arg2,
		       ajint *iparam, AjPStr *pqual, AjPStr *pnoqual,
		       AjPStr *pvalue, ajint* number, AjPStr *pmaster,
		       AcdPAcd* acd)
{
    ajint ret=0;
    const char *cp;
    ajlong i;
    AjBool gotvalue  = ajFalse;
    AjBool ismissing = ajFalse;
    AjBool qstart    = ajFalse;
    AjBool nullok    = ajFalse;
    AjBool attrok    = ajFalse;
    AjPStr noqual  = NULL;

    acdLog("acdIsQual '%s' '%s'\n", arg, arg2);
    cp = arg;
    *number = 0;
    *acd = NULL;

    ajStrDel(pmaster);

    if(!strcmp(cp, "-"))	       /* stdin or stdout parameter */
	return 0;
    if(!*cp)                           /* dummy parameter */
	return 0;
    
    if(!strcmp(cp, "--"))	       /* special -- to turn off processing */
    {
/*	acdQualTestSkip = ajTrue; */
	return 0;
    }

    if(*cp && strchr("-/", *cp))   /* first character vs. qualifier starts */
    {
	cp++;
	qstart = ajTrue;

	if(*cp == '-')   /* allow --qualifier */
	{
	    cp++;
	}
    }

    ajStrAssignClear(pqual);
    ajStrAssignClear(pnoqual);

    if(!*cp)
	return 0;
    
    /* qualifier: now play hunt the value */
    
    ret = 1;
    ajStrAssignC(pqual, cp);      /* qualifier with '-' or '/' removed */
    
    /*
     ** pqual could be
     ** qualname (unless boolean, look for next arg as the value)
     ** qualname=value (value as part of arg)
     ** noqualname (boolean negative, or nullok set to empty string
     */
    
    /*
     ** First check whether we have a value (set gotvalue) in the first arg
     */
    
    i = ajStrFindC(*pqual, "=");

    if(i >= 0)
    {
        if((i+1) == (ajint) ajStrGetLen(*pqual)) /* ended with '=' */
            ajStrAssignC(pvalue, "");
        else
            ajStrAssignSubS(pvalue, *pqual, (i+1), -1);
	
	acdLog("qualifier value '%S' '%S' %d .. %d\n",
	       *pvalue, *pqual, (i+1), -1);
	ajStrKeepRange(pqual, 0, (i-1));
	gotvalue = ajTrue;
    }
    else
    {
	if(!qstart)	/* no start, no "=" assume it's a parameter */
	    return 0;
	if(!ajStrIsAlnum(*pqual))	/* funny characters, fail */
	    return 0;
    }
    
    acdQualParse(pqual, &noqual, pmaster, number);

    if(ajStrGetLen(*pmaster)) /* specific master, turn off auto processing */
	acdMasterQual = NULL; /* pmaster resets this in acdFindQualDetail  */
    
    if(acdMasterQual)	      /* we are still working with a master */
    {
	acdLog("(a) master, try associated with acdFindQualAssoc\n");
	*acd = acdFindQualAssoc(acdMasterQual, *pqual, noqual, *pmaster,
                                *number);

	if(!*acd)
	{
	    acdLog("acdMasterQual cleared, was -%S\n", acdMasterQual->Name);
	    acdMasterQual = NULL;
	}
	else
	{
	    *number = acdMasterQual->PNum;
	    acdLog("Qualifier -%S associated with -%S\n",
		   *pqual, acdMasterQual->Name);
	}
    }
    
    if(!acdMasterQual)
    {
	acdLog("(b) no master, general test with acdFindQualDetail\n");
	*acd = acdFindQualDetail(*pqual, noqual, *pmaster, *number, iparam);
    }
    
    if(!*acd)				/* test acdc-badqual */
	ajDie("Unknown qualifier %s", arg);

    ajStrAssignS(pqual, (*acd)->Name);
    ajStrAssignS(pnoqual, (*acd)->Name);

    if((*acd)->AssocQuals)		/* this one is a new master */
    {
	acdLog("acdMasterQual set to -%S\n", (*acd)->Name);
	acdMasterQual = *acd;
    }
    
    if(gotvalue)
    {
	/* test acdc-noprefixvalue */
	if(ajStrPrefixS((*acd)->Name, noqual))
	    ajDie("'no' prefix used with value for '%s'", arg);
    }
    else
    {
	acdLog("testing for a value\n");
	
	/*
	 ** Bool qualifiers can have no value
	 ** or can be followed by a valid Bool value
	 */
	
	if(ajStrPrefixS((*acd)->Name, noqual))
	{			       /* we have a -noqual matched */
	    acdLog("we matched with -no\n");
	    ajStrAssignC(pnoqual, "no");
	    ajStrAppendS(pnoqual, (*acd)->Name);

	    if(!strcmp(acdType[acdListCurr->Type].Name, "boolean") ||
	       !strcmp(acdType[acdListCurr->Type].Name, "toggle"))
	    {
		acdLog("-no%S=N boolean or toggle accepted\n", noqual);
		gotvalue = ajTrue;
		ret = 1;
		ajStrAssignC(pvalue, "N");
		ajStrDel(&noqual);

		return ret;
	    }

	    if(acdAttrTest(*acd, "nullok"))
	    {
		if(acdAttrTestValue(*acd, "nullok"))
		    attrok = acdAttrToBool(*acd, /* -no for null value */
					   "nullok", ajFalse, &nullok);
		else			/* assume it can be true */
		{
		    attrok = ajTrue;
		    nullok = ajTrue;
		}
		acdLog("check for nullok, found:%B value:%B\n",
		       attrok, nullok);
	    }
	    else
		nullok = ajFalse;

	    if(nullok)
	    {
		acdLog("-no%S='' nullOK accepted\n", noqual);
		gotvalue = ajTrue;
		ret = 1;
                (*acd)->UserSetNull = ajTrue;
		ajStrAssignClear(pvalue);
		ajStrDel(&noqual);

		return ret;
	    }
	    else	 /* test acdc-noprefixbad acdc-noprefixbad2 */
		ajDie("'no' prefix invalid for '%s'", arg);
	}
	
	/*
	 ** just the qualifier name with no value
	 */
	
	if(!strcmp(acdType[acdListCurr->Type].Name, "boolean") ||
	   !strcmp(acdType[acdListCurr->Type].Name, "toggle"))
	{
	    if(acdValIsBool(arg2))	/* bool value, accept */
	    {
		acdLog("acdValIsBool -%s '%s'\n", arg, arg2);
		gotvalue = ajTrue;
		ret = 2;
		ajStrAssignC(pvalue, arg2);
	    }
	    else			/* we must mean true */
		ajStrAssignC(pvalue, "Y");
	}
	else
	{
	    if(!arg2)			/* no value - can be missing? */
	    {
		ajStrToBool((*acd)->DefStr[DEF_MISSING], &ismissing);

		if(!ismissing)		/* test acdc-novalue */
		    ajDie("Value required for '%s'", arg);
	    }
	    /* test for known qualifiers */
	    else
	    {
		if(*arg2 == '-')
		{
		    if(!acdTestQualC(arg2)) /* not known qualifier */
			/* must be value */
			gotvalue = ajTrue;
		    else
		    {
			ajStrToBool((*acd)->DefStr[DEF_MISSING],
				    &ismissing);
			if(!ismissing)	/* test acdc-novalue2 */
			    ajDie("Value required for '%s' before '%s'",
				  arg, arg2);
		    }
		}
		else
		    gotvalue = ajTrue;
	    }
	    if(gotvalue)
	    {
		ret = 2;
		ajStrAssignC(pvalue, arg2);
	    }
	    else
		ajStrAssignClear(pvalue);
	}
    }

    ajStrDel(&noqual);

    return ret;
}




/* @funcstatic acdValIsBool ***************************************************
**
** Tests whether a value on the command line is a valid Boolean value
**
** @param [r] arg [const char*] COmmand live argument value
** @return [AjBool] ajTrue if the value is boolean,
**                  but not whether it is true or false.
**
** @release 2.0.0
******************************************************************************/

static AjBool acdValIsBool(const char* arg)
{
    if(!arg)
	return ajFalse;

    switch(*arg)
    {
        case 'n':
        case 'N':
            if(!arg[1])
                return ajTrue;
            return ajCharMatchCaseC(arg, "no");

        case 'y':
        case 'Y':
            if(!arg[1])
                return ajTrue;
            return ajCharMatchCaseC(arg, "yes");


        case 't':
        case 'T':
            if(!arg[1])
                return ajTrue;

            return ajCharMatchCaseC(arg, "true");


        case 'f':
        case 'F':
            if(!arg[1])
                return ajTrue;

            return ajCharMatchCaseC(arg, "false");


        case '0':
            if(!arg[1])
                return ajTrue;

            return ajCharMatchCaseC(arg, "0");


        case '1':
            if(!arg[1])
                return ajTrue;

            return ajCharMatchCaseC(arg, "1");


        default:
            break;
    }

    return ajFalse;
}




/* @funcstatic acdFindItem ****************************************************
**
** Returns the ACD definition for a named item and
** (optionally) a given qualifier number. If the qualifier number
** is given, it is checked. If not, the first hit is used.
**
** Section and Endsection do not count
**
** @param [r] item [const AjPStr] Item name
** @param [r] number [ajint] Item number (zero if a general item)
** @return [AcdPAcd] ACD item required
**
** @release 1.0.0
** @@
******************************************************************************/

static AcdPAcd acdFindItem(const AjPStr item, ajint number)
{
    AcdPAcd ret  = NULL;
    AcdPAcd pa;
    AjBool found = ajFalse;
    ajint ifound = 0;
    AjPStr ambigList = NULL;

    ambigList = ajStrNew();

    for(pa=acdList; pa; pa=pa->Next)
    {
	if(acdIsStype(pa))
	    continue;

	found = ajFalse;
	if(ajStrPrefixS(pa->Name, item))
	    if(!number || number == pa->PNum)
		found = ajTrue;

	if(found)
	{
	    if(ajStrMatchS(pa->Name, item))
	    {
		ajStrDel(&ambigList);
		return pa;
	    }

	    ifound++;
	    ret = pa;
	    acdAmbigApp(&ambigList, pa->Name);
	}
    }

    if(ifound == 1)
    {
	if (acdDoValid)
	    acdWarn("Abbreviated item '%S' (%S)", item, ambigList);
	ajStrDel(&ambigList);
	return ret;
    }

    if(ifound > 1)
    {
	ajWarn("ambiguous item %S (%S)", item, ambigList);
    }

    ajStrDel(&ambigList);

    return NULL;
}




/* @funcstatic acdFindQual ****************************************************
**
** Finds a qualifier by name, and returns the full name
**
** @param [u] pqual [AjPStr*] Qualifier name
** @return [AcdPAcd] ACD item for qualifier
**
** @release 1.0.0
** @@
******************************************************************************/

static AcdPAcd acdFindQual(AjPStr *pqual)
{
    AcdPAcd ret    = NULL;
    AcdPAcd pa;
    ajint ifound = 0;
    AjBool found = ajFalse;

    if(!ajStrGetLen(*pqual)) return NULL;

    for(pa=acdList; pa; pa=pa->Next)
    {
	found = ajFalse;

	if(acdIsStype(pa))
	    continue;

	found = ajFalse;

	if(pa->Level == ACD_QUAL || pa->Level == ACD_PARAM)
	{
	    if(ajStrPrefixS(pa->Name, *pqual))
		found = ajTrue;

	    if(found)
	    {
		if(ajStrMatchS(pa->Name, *pqual))
		    return pa;

		ifound++;
		ret = pa;
	    }
	}
    }

    if(ifound == 1)
    {
	ajStrAssignS(pqual, ret->Name);

	return ret;
    }

    return NULL;
}




/* @funcstatic acdFindQualDetail **********************************************
**
** Returns the parameter definition for a named qualifier and
** (optionally) a given qualifier number. If the qualifier number
** is given, it is checked. If not, the current parameter number is checked.
** General qualifiers have no specified number and can match at any time.
**
** @param [r] qual [const AjPStr] Qualifier name
** @param [r] noqual [const AjPStr] Alternative qualifier name
**        (qual with "no" prefix removed, or empty, or NULL)
** @param [rN] master [const AjPStr] Master qualifier name
** @param [r] PNum [ajint] Qualifier number (zero if a general qualifier)
** @param [u] iparam [ajint*]  Current parameter number
** @return [AcdPAcd] ACD item for qualifier
**
** @release 4.0.0
** @@
******************************************************************************/

static AcdPAcd acdFindQualDetail(const AjPStr qual, const AjPStr noqual,
				 const AjPStr master,
				 ajint PNum, ajint *iparam)
{
    /* test for match of parameter number and type */
    
    /* PNum : number encoded in qualifier name ==> forced match */
    /* iparam : current parameter number ==> possible match */
    /* when both are zero, could be a generic match, like "-begin" for
       all sequences. Just return the first and let caller find the rest */
    
    AcdPAcd ret    = NULL;
    AcdPAcd pa;
    AjBool found   = ajFalse;
    AjBool isparam = ajFalse;
    ajint ifound   = 0;
    AjPStr ambigList = NULL;
    
    if(ajStrGetLen(master))
    {
        acdLog("acdFindQualDetail ... call acdFindQualMaster\n",
	   qual, noqual, PNum, *iparam);
    
	*iparam = 0;
	return acdFindQualMaster(qual, noqual, master, PNum);
    }
    
    ambigList = ajStrNew();
    
    acdLog("acdFindQualDetail '%S' (%S) PNum: %d iparam: %d\n",
	   qual, noqual, PNum, *iparam);
    
    for(pa=acdList; pa; pa=pa->Next)
    {
	if(acdIsStype(pa))
	    continue;

	found = ajFalse;
	if(pa->Level == ACD_QUAL)
	{
	    if(ajStrPrefixS(pa->Name, qual) ||
	       ajStrPrefixS(pa->Name, noqual))
	    {
		acdLog("..matched qualifier '%S' [%d]\n", pa->Name, pa->PNum);
		if(PNum)	      /* -begin2 forces match to #2 */
		{
		    if(PNum == pa->PNum)
		    {
			acdLog("..matched PNum '%S' [%d]\n",
			       pa->Name, pa->PNum);
			found = ajTrue;
		    }
		}
		else if(pa->PNum) /* defined for parameter pa->PNum */
		{
		    acdLog("..hit PNum '%S' [%d] (ambigList '%S')\n",
			   pa->Name, pa->PNum, ambigList);
		    if(!ifound  || !ajStrMatchS(pa->Name, ambigList))
			found = ajTrue;
		}
		else			/* general match */
		    found = ajTrue;

		if(found)
		{
		    if(ajStrMatchS(pa->Name, qual) ||
		       ajStrMatchS(pa->Name, noqual))
		    {
			acdListCurr = pa;
			ajStrDel(&ambigList);
			return pa;
		    }

		    acdAmbigApp(&ambigList, pa->Name);
		    ifound++;
		    ret = pa;
		    acdLog("..prefix only '%S', ifound %d\n",
			   pa->Name, ifound);
		}
	    }
	}
	else if(pa->Level == ACD_PARAM)
	{
	    if(ajStrPrefixS(pa->Name, qual) ||
	       ajStrPrefixS(pa->Name, noqual))
	    {
		acdLog("..matched param '%S' [%d]\n", pa->Name, pa->PNum);
		if(ajStrMatchS(pa->Name, qual) ||
		   ajStrMatchS(pa->Name, noqual))
		{
		    acdListCurr = pa;
		    *iparam = pa->PNum;
		    ajStrDel(&ambigList);
		    return pa;
		}

		acdAmbigApp(&ambigList, pa->Name);
		ifound++;
		isparam = ajTrue;
		ret = pa;
		acdLog("..prefix only '%S', ifound %d\n", pa->Name, ifound);
	    }
	}
    }
    
    if(ifound == 1)
    {
	acdListCurr = ret;

	if(isparam)
	    *iparam = ret->PNum;

	if (acdDoValid)
	    acdWarn("Abbreviated qualifier '%S' (%S)", qual, ambigList);

	ajStrDel(&ambigList);
	return ret;
    }

    if(ifound > 1)
    {
	ajWarn("ambiguous qualifier '%S' (%S)", qual, ambigList);
    }

    ajStrDel(&ambigList);
    
    return NULL;
}




/* @funcstatic acdFindQualMaster **********************************************
**
** Returns the parameter definition for a named qualifier and
** (optionally) a given qualifier number. If the qualifier number
** is given, it is checked. If not, the current parameter number is checked.
** General qualifiers have no specified number and can match at any time.
**
** @param [r] qual [const AjPStr] Qualifier name
** @param [r] noqual [const AjPStr] Alternative qualifier name
**        (qual with "no" prefix removed, or empty, or NULL)
** @param [rN] master [const AjPStr] Master qualifier name
** @param [r] PNum [ajint] Qualifier number (zero if a general qualifier)
** @return [AcdPAcd] ACD item for qualifier
**
** @release 1.0.0
** @@
******************************************************************************/

static AcdPAcd acdFindQualMaster(const AjPStr qual, const AjPStr noqual,
				 const AjPStr master,
				 ajint PNum)
{
    /* test for match of parameter number and type */
    
    /*
     ** PNum : number encoded in qualifier name ==> forced match
     ** iparam : current parameter number ==> possible match
     ** when both are zero, could be a generic match, like "-begin" for
     ** all sequences. Just return the first and let caller find the rest
     */
    
    AcdPAcd ret  = NULL;
    AcdPAcd pa;
    AjBool found = ajFalse;
    ajint ifound  =0;
    AjPStr ambigList = NULL;
    
    ambigList = ajStrNew();
    
    acdLog("acdFindQualMaster '%S_%S' (%S) PNum: %d\n",
	   qual, master, noqual, PNum);
    
    for(pa=acdList; pa; pa=pa->Next)
    {
	if(acdIsStype(pa))
	    continue;

	found = ajFalse;

	if(pa->Level == ACD_QUAL)
	{
	    if(ajStrPrefixS(pa->Name, master))
	    {
		acdLog("..matched qualifier '%S' [%d]\n", pa->Name, pa->PNum);
		if(PNum)	      /* -begin2 forces match to #2 */
		{
		    if(PNum == pa->PNum)
		    {
			acdLog("..matched PNum '%S' [%d]\n",
			       pa->Name, pa->PNum);
			found = ajTrue;
		    }
		}
		else if(pa->PNum) /* defined for parameter pa->PNum */
		{
		    acdLog("..hit PNum '%S' [%d] (ambigList '%S')\n",
			   pa->Name, pa->PNum, ambigList);
		    if(!ifound  || !ajStrMatchS(pa->Name, ambigList))
			found = ajTrue;
		}
		else			/* general match */
		    found = ajTrue;

		if(found)
		{
		    if(ajStrMatchS(pa->Name, master))
		    {
			ret = pa;
			ifound = 1;
			break;
		    }
		    acdAmbigApp(&ambigList, pa->Name);
		    ifound++;
		    ret = pa;
		    acdLog("..prefix only, ifound %d\n", ifound);
		}
	    }
	}
	else if(pa->Level == ACD_PARAM)
	{
	    if(ajStrPrefixS(pa->Name, master))
	    {
		acdLog("..matched param '%S' [%d]\n", pa->Name, pa->PNum);

		if(ajStrMatchS(pa->Name, master))
		{
		    ret = pa;
		    ifound = 1;
		    break;
		}
		acdAmbigApp(&ambigList, pa->Name);
		ifound++;
		ret = pa;
		acdLog("..prefix only, ifound %d\n", ifound);
	    }
	}
    }
    
    if(ifound > 1)
    {
	acdLog("..ambiguous master qualifier for %S_%S (%S)",
	       qual, master, ambigList);
	ajWarn("Ambiguous master qualifier '%S' in %S_%S (%S)",
	       master, qual, master, ambigList);
	ajStrDel(&ambigList);

	return NULL;
    }

    if(!ifound)
    {
	acdLog("..master qualifier for %S_%S not found\n", qual, master);
	ajStrDel(&ambigList);
	return NULL;
    }
    
    acdLog("..master qualifier found '%S' %d\n", ret->Name, ret->PNum);
    
    ifound = 0;

    for(pa=ret->AssocQuals; pa && pa->Assoc; pa=pa->Next)
    {
	found = ajFalse;
	if(ajStrPrefixS(pa->Name, qual) ||
	   ajStrPrefixS(pa->Name, noqual))
	{
	    acdLog("..matched qualifier '%S' [%d]\n", pa->Name, pa->PNum);
	    if(PNum)		      /* -begin2 forces match to #2 */
	    {
		if(PNum == pa->PNum)
		{
		    acdLog("..matched PNum '%S' [%d]\n",
			   pa->Name, pa->PNum);
		    found = ajTrue;
		}
	    }
	    else if(pa->PNum)	  /* defined for parameter pa->PNum */
	    {
		acdLog("..hit PNum '%S' [%d] (ambigList '%S')\n",
		       pa->Name, pa->PNum, ambigList);
		if(!ifound  || !ajStrMatchS(pa->Name, ambigList))
		{
		    found = ajTrue;
		}
	    }
	    else			/* general match */
		found = ajTrue;

	    if(found)
	    {
		if(ajStrMatchS(pa->Name, qual) ||
		   ajStrMatchS(pa->Name, noqual))
		{
		    acdListCurr = pa;
		    ajStrDel(&ambigList);
		    return pa;
		}

		acdAmbigApp(&ambigList, pa->Name);
		ifound++;
		ret = pa;
		acdLog("..prefix only, ifound %d\n", ifound);
	    }
	}
    }

    if(ifound == 1)
    {
	acdListCurr = ret;
	if (acdDoValid)
	    acdWarn("Abbreviated associated qualifier '%S_%S' (%S)",
		    qual, master, ambigList);
	ajStrDel(&ambigList);
	return ret;
    }

    if(ifound > 1)
    {
	acdLog("..ambiguous associated qualifier %S_%S (%S)",
	       qual, master, ambigList);
	ajWarn("ambiguous associated qualifier %S_%S (%S)",
	       qual, master, ambigList);
    }
    
    ajStrDel(&ambigList);

    acdLog("..associated qualifier %S_%S not found", qual, master);
    
    return NULL;
}




/* @funcstatic acdFindQualAssoc ***********************************************
**
** Returns the definition for a named associated qualifier.
** If the qualifier number
** is given, it is checked. If not, the current parameter number is checked.
** General qualifiers have no specified number and can match at any time.
**
** @param [r] thys [const AcdPAcd] Master ACD item
** @param [r] qual [const AjPStr] Qualifier name
** @param [r] noqual [const AjPStr] Alternative qualifier name
**        (qual with "no" prefix removed, or empty, or NULL)
** @param [rN] master [const AjPStr] Master qualifier name
** @param [r] PNum [ajint] Qualifier number (zero if a general qualifier)
** @return [AcdPAcd] ACD item for associated qualifier
** @error NULL returned if not found.
**
** @release 1.0.0
** @@
******************************************************************************/

static AcdPAcd acdFindQualAssoc(const AcdPAcd thys,
				const AjPStr qual, const AjPStr noqual,
				const AjPStr master,
				ajint PNum)
{
    /* test for match of parameter number and type */

    /*
     **  PNum : number encoded in qualifier name ==> forced match
     ** iparam : current parameter number ==> possible match
     ** when both are zero, could be a generic match, like "-sbegin" for
     ** all sequences. Just return the first and let caller find the rest
     */

    AcdPAcd pa   = thys->AssocQuals;
    ajint ifound = 0;
    AcdPAcd ret  = NULL;
    AjPStr ambigList = NULL;
    ajint iparam;

    /* acdLog("acdFindQualAssoc '%S' pnum: %d\n", qual, pnum); */

    if(PNum  && (pa->PNum != PNum)) /* must be for same number (if any) */
	return NULL;

    ambigList = ajStrNew();

    for(; pa && pa->Assoc; pa=pa->Next)
    {
	if(ajStrPrefixS(pa->Name, qual) ||
	   ajStrPrefixS(pa->Name, noqual))
	{
	    if(ajStrMatchS(pa->Name, qual) ||
	       ajStrMatchS(pa->Name, noqual))
	    {
		/* acdLog("   *matched* '%S'\n", pa->Name); */
		acdListCurr = pa;
		ajStrDel(&ambigList);
		return acdListCurr;
	    }

	    ifound++;
	    ret = pa;
	    acdAmbigApp(&ambigList, pa->Name);
	}
    }

    /* acdLog("   ifound: %d\n", ifound); */

    if(ifound == 1)
    {
	acdListCurr = ret;

        pa = acdFindQualDetail(qual,noqual,master,PNum, &iparam);

        if(pa == ret)
        {
            if (acdDoValid)
                acdWarn("Abbreviated associated qualifier '%S' (%S)",
                        qual, ambigList);

            ajStrDel(&ambigList);

            return acdListCurr;
        }
    }

    if(ifound > 1)
	ajWarn("ambiguous qualifier %S (%S)", qual, ambigList);

    ajStrDel(&ambigList);

    return NULL;
}




/* @funcstatic acdFindParam ***************************************************
**
** Returns the paremeter definition for a given parameter number
**
** @param [r] PNum [ajint] Parameter number
** @return [AcdPAcd] ACD item for parameter number PNum
** @error NULL if not found
**
** @release 1.0.0
** @@
******************************************************************************/

static AcdPAcd acdFindParam(ajint PNum)
{
    /* test for match of parameter number and type */

    AcdPAcd pa;

    for(pa=acdList; pa; pa=pa->Next)
    {
	if(acdIsStype(pa))
            continue;

	if((pa->Level == ACD_PARAM) && (pa->PNum == PNum))
	{
	    acdListCurr = pa;
	    return pa;
	}
    }

    return NULL;
}




/* @funcstatic acdGetAttr *****************************************************
**
** Pick up a defined attribute for variable handling.
** The ACD item must be defined (already processed).
** Attributes include any specially calculated by the acdSet function
** for that type.
**
** @param [u] presult [AjPStr*] Resulting attribute value
** @param [r] name [const AjPStr] ACD item name
** @param [r] attrib [const AjPStr] attribute name
** @return [AjBool] ajTrue on success
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdGetAttr(AjPStr* presult,
			 const AjPStr name, const AjPStr attrib)
{
    const char *cp;
    const char *cq;
    ajint ilen;
    ajint number = 0;

    AjPStr tempstr=NULL;

    AcdPAcd pa    = NULL;
    AcdPAttr attr = NULL;
    ajint i;

    acdLog("acdGetAttr name '%S' attrib '%S'\n", name, attrib);
    ajStrDelStatic(presult);
    
    ajStrAssignS(&tempstr, name);
    ajStrFmtLower(&tempstr);
    cp = ajStrGetPtr(tempstr);
    cq = &cp[ajStrGetLen(tempstr)];

    if(isdigit((ajint)*--cq))
    {
	while(isdigit((ajint)*--cq));
	++cq;

	number = (ajint) strtol(cq, NULL, 0);
	ilen = cq - cp - 1;
	ajStrKeepRange(&tempstr, 0, ilen);
    }
    
    pa = acdFindItem(tempstr, number);

    if(!pa)				/* test ambigvar.acd */
	acdError("Failed to resolve variable '%S'\n", name);

    ajStrDel(&tempstr);

    if(!pa->ValStr)
    {
	if (!acdDoValid && !acdDoGalaxy)
	{
	    if(!acdDoHelp)			/* test undefvar.acd */
		acdError("Variable '%S' not yet defined\n", name);

            acdAttrValueStr(pa, "default", "...", presult);

	    return ajTrue;
	}
    }
    
    if(!ajStrGetLen(attrib))		/* just use valstr */
    {
	if (!acdDoValid && !acdDoGalaxy)
	{
	    ajStrAssignS(presult, pa->ValStr);
	    acdLog("no attribute name, use valstr for %S '%S'\n",
		   pa->Name, *presult);
	    pa->Used |= USED_ACD;

	    return ajTrue;
	}
    }
    
    if(pa->DefStr)
    {
	attr = acdAttrDef;
	i = acdFindAttr(attr, attrib);

	if(i >= 0)
	{
	    ajStrAssignS(presult, pa->DefStr[i]);
	    acdLog("default attribute %S found for %S '%S'\n",
		   attrib, pa->Name, *presult);

	    return ajTrue;
	}
    }
    
    if(pa->NAttr)
    {
	attr = acdType[pa->Type].Attr;
	i = acdFindAttr(attr, attrib);

	if(i >= 0)
	{
	    ajStrAssignS(presult, pa->AttrStr[i]);
	    acdLog("type attribute %S found for %S '%S'\n",
		   attrib, pa->Name, *presult);

	    return ajTrue;
	}
    }
    
    if(pa->SAttr)
    {
	acdLog("++calc++ Testing SAttr %d\n", pa->SAttr);
	attr = pa->SetAttr;

	for(i=0; i < pa->SAttr; i++)
	    acdLog("calcattr[%d] '%s'\n",
		   i, attr[i].Name);
	i = acdFindAttr(attr, attrib);

	if(i >= 0)
	{
	    ajStrAssignS(presult, pa->SetStr[i]);
	    acdLog("calculated attribute %S found for %S '%S'\n",
		   attrib, pa->Name, *presult);

	    return ajTrue;
	}
    }
    
    if(ajStrMatchCaseC(attrib, "isdefined"))
    {
	acdLog("++isdefined++ Testing\n");

	if (ajStrGetLen(pa->ValStr))
	{
	    ajStrAssignC(presult, "Y");
	}
	else
	{
	    ajStrAssignC(presult, "N");
	}
	acdLog("isdefined attribute found for %S '%S'\n",
	       pa->Name, *presult);

	return ajTrue;
    }
    
    acdLog("*attribute %S not found for %S*\n", attrib, pa->Name);

    return ajFalse;
}




/* @funcstatic acdQualParse ***************************************************
**
** Converts a qualifier name to lower case and looks for a
** master qualifier name and a trailing number.
**
** @param [w] pqual [AjPStr*] Qualifier name set to lower case
**        with number suffix removed
** @param [w] pnoqual [AjPStr*] Qualifier name as pqual, with "no" prefix
**        removed, or empty string id pqual doesn't start with "no"
** @param [w] pqmaster [AjPStr*] Master name for associated qualifier
** @param [w] number [ajint*] Qualifier number suffix if any
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdQualParse(AjPStr* pqual, AjPStr* pnoqual, AjPStr* pqmaster,
			 ajint* number)
{
    if(!acdRegQualParse)
	acdRegQualParse = ajRegCompC("^([a-z]+)(_([a-z]+))?([0-9]+)?$");

    ajStrFmtLower(pqual);
    ajStrAssignS(&acdQualNameTmp, *pqual);

    if(!ajRegExec(acdRegQualParse, acdQualNameTmp))
    {
	ajStrAssignClear(pqual);
	ajStrAssignClear(pnoqual);
	ajStrAssignClear(pqmaster);
	*number = 0;

	return;
    }
    ajRegSubI(acdRegQualParse, 1, pqual);
    ajRegSubI(acdRegQualParse, 3, pqmaster);
    ajRegSubI(acdRegQualParse, 4, &acdQualNumTmp);

    if(ajStrPrefixC(*pqual, "no"))
	ajStrAssignSubS(pnoqual, *pqual, 2, -1);
    else
	ajStrAssignClear(pnoqual);

    *number = 0;

    if(ajStrGetLen(acdQualNumTmp))
	ajStrToInt(acdQualNumTmp, number);

    if(ajStrGetLen(*pqmaster))
	acdFindQual(pqmaster);

    return;
}




/* @funcstatic acdTokenToLowerS ***********************************************
**
** Converts a token name to lower case and looks for a trailing number.
**
** @param [u] ptoken [AjPStr*] Qualifier name set to lower case
**        with number suffix removed
** @param [w] number [ajint*] Qualifier number suffix if any.
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

static void acdTokenToLowerS(AjPStr *ptoken, ajint* number)
{
    const char *cp;
    const char *cq;
    ajint ilen;

    ajStrFmtLower(ptoken);
    cp = ajStrGetPtr(*ptoken);
    cq = cp+ajStrGetLen(*ptoken);

    if(!isdigit((ajint)*--cq))
    {
        *number = 0;

        return;
    }

    while(isdigit((ajint)*--cq))
	;

    ++cq;

    *number = (ajint) strtol(cq, NULL, 0);
    ilen = cq - cp;
    ajStrCutEnd(ptoken, ajStrGetLen(*ptoken) - ilen);

    return;
}




/* @funcstatic acdIsRequired **************************************************
**
** Returns true if an ACD item is required but not yet defined.
** Required means the standard attribute is set (which it is by
** default for a parameter),
** or the additional flag is set and -options was specified.
**
** @param [r] thys [const AcdPAcd] ACD item
** @return [AjBool] ajTrue if "thys" is required but no value set yet.
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdIsRequired(const AcdPAcd thys)
{
    AjPStr *def     = thys->DefStr;
    AjBool required = ajFalse;

    if (thys->DefStr)
	acdLog("acdIsRequired '%S' Defined %B DefStr '%x' std '%S' "
               "add '%S' def '%S'\n",
	       thys->Name,
	       thys->Defined,
	       thys->DefStr,
	       def[DEF_STANDARD],
	       def[DEF_ADDITIONAL],
	       def[DEF_DEFAULT]);
    else
    	acdLog("acdIsRequired '%S' Defined %B\n",
	       thys->Name,
	       thys->Defined);

    if(thys->Defined)
	return ajFalse;

    if(!thys->DefStr)
	return ajFalse;

    if(ajStrGetLen(def[DEF_STANDARD]))
    {
	acdVarResolve(&def[DEF_STANDARD]);

	if(!ajStrToBool(def[DEF_STANDARD], &required))
	    acdErrorAcd(thys, "Bad standard flag %S\n",
			def[DEF_STANDARD]);
	return required;
    }

    if(acdOptions && ajStrGetLen(def[DEF_ADDITIONAL]))
    {
	acdVarResolve(&def[DEF_ADDITIONAL]);

	if(!ajStrToBool(def[DEF_ADDITIONAL], &required))
	    acdErrorAcd(thys, "Bad additional flag %S\n",
			def[DEF_ADDITIONAL]);
	return required;
    }

    return ajFalse;
}




/* @funcstatic acdPromptCodon *************************************************
**
** Sets the default prompt for this ACD object to be a codon usage file
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 1.0.0
** @@
******************************************************************************/

static const AjPStr acdPromptCodon(AcdPAcd thys)
{
    static ajint count=0;

    const AjPStr knowntype;

    knowntype = acdKnowntypeDesc(thys);
    if(ajStrGetLen(knowntype))
    {
	count++;
	acdPromptStandardS(thys, knowntype);
	if(!ajStrSuffixC(knowntype, " file"))
	   acdPromptStandardAppend(thys, " file");
    }
    else
	acdPromptStandard(thys, "Codon usage file", &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    return thys->StdPrompt;
}




/* @funcstatic acdPromptDirectory *********************************************
**
** Sets the default prompt for this ACD object to be a directory
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 4.0.0
** @@
******************************************************************************/

static const AjPStr acdPromptDirectory(AcdPAcd thys)
{
    static ajint count=0;

    const AjPStr knowntype;

    knowntype = acdKnowntypeDesc(thys);
    if(ajStrGetLen(knowntype))
    {
	count++;
	acdPromptStandardS(thys, knowntype);

	if(!ajStrSuffixC(knowntype, " directory"))
	   acdPromptStandardAppend(thys, " directory");
    }
    else
	acdPromptStandard(thys, "Directory of input files", &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    return thys->StdPrompt;
}




/* @funcstatic acdPromptDirlist ***********************************************
**
** Sets the default prompt for this ACD object to be a dirlist
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 1.9.0
** @@
******************************************************************************/

static const AjPStr acdPromptDirlist(AcdPAcd thys)
{
    static ajint count=0;

    const AjPStr knowntype;

    knowntype = acdKnowntypeDesc(thys);
    if(ajStrGetLen(knowntype))
    {
	count++;
	acdPromptStandardS(thys, knowntype);

	if(!ajStrSuffixC(knowntype, " directories"))
	   acdPromptStandardAppend(thys, " directories");
    }
    else
	acdPromptStandard(thys, "Directories with files", &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    return thys->StdPrompt;
}




/* @funcstatic acdPromptFilelist **********************************************
**
** Sets the default prompt for this ACD object to be a filelist
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 2.5.0
** @@
******************************************************************************/

static const AjPStr acdPromptFilelist(AcdPAcd thys)
{
    static ajint count=0;

    const AjPStr knowntype;

    knowntype = acdKnowntypeDesc(thys);
    if(ajStrGetLen(knowntype))
    {
	count++;
	acdPromptStandardS(thys, knowntype);

	if(!ajStrSuffixC(knowntype, " file list"))
	   acdPromptStandardAppend(thys, " file list");
    }
    else
	acdPromptStandard(thys, "Comma-separated file list", &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    return thys->StdPrompt;
}




/* @funcstatic acdPromptFeatures **********************************************
**
** Sets the default prompt for this ACD object to be a feature table
** prompt with "first", "second" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 6.0.0
** @@
******************************************************************************/

static const AjPStr acdPromptFeatures(AcdPAcd thys)
{
    static ajint count=0;

    const AjPStr knowntype;

    ajint maxreads = 0;

    acdAttrToInt(thys, "maxreads", 1, &maxreads);

    knowntype = acdKnowntypeDesc(thys);
    if(ajStrGetLen(knowntype))
    {
	count++;
	acdPromptStandardS(thys, knowntype);

	if(!ajStrSuffixC(knowntype, " feature table"))
	   acdPromptStandardAppend(thys, " feature table");
    }
    else
	acdPromptStandard(thys, "input feature table", &count);

    if(maxreads > 1)
	 acdPromptStandardAppend(thys, "(s)");

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
     return thys->StdPrompt;
}




/* @funcstatic acdPromptCpdb **************************************************
**
** Sets the default prompt for this ACD object to be a clean PDB file
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 1.8.0
** @@
******************************************************************************/

static const AjPStr acdPromptCpdb(AcdPAcd thys)
{
    static ajint count=0;

    const AjPStr knowntype;

    knowntype = acdKnowntypeDesc(thys);
    if(ajStrGetLen(knowntype))
    {
	count++;
	acdPromptStandardS(thys, knowntype);

	if(ajStrSuffixC(knowntype, " file"))
	   acdPromptStandardAppend(thys, " file");
    }
    else
	acdPromptStandard(thys, "clean PDB file", &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    return thys->StdPrompt;
}




/* @funcstatic acdPromptScop **************************************************
**
** Sets the default prompt for this ACD object to be a scop entry
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 1.8.0
** @@
******************************************************************************/

static const AjPStr acdPromptScop(AcdPAcd thys)
{
    static ajint count=0;

    const AjPStr knowntype;

    knowntype = acdKnowntypeDesc(thys);
    if(ajStrGetLen(knowntype))
    {
	count++;
	acdPromptStandardS(thys, knowntype);

	if(!ajStrSuffixC(knowntype, " file"))
	   acdPromptStandardAppend(thys, " file");
    }
    else
	acdPromptStandard(thys, "Scop entry vile", &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    return thys->StdPrompt;
}




/* @funcstatic acdHelpTextSeq *************************************************
**
** Sets the help text for this ACD object to be a sequence
** description with the type included.
**
** @param [r] thys [const AcdPAcd] Current ACD object.
** @param [w] Pstr [AjPStr*] Help text
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

static void acdHelpTextSeq(const AcdPAcd thys, AjPStr* Pstr)
{
    const AjPStr typestr = NULL;
    AjPStr tmptype = NULL;
    AjBool gaps = AJFALSE;

    typestr = acdAttrValue(thys, "type");

    ajSeqTypeSummary(typestr, &tmptype, &gaps);
    ajStrAssignClear(Pstr);

    if(ajStrGetLen(tmptype))
    {
	ajStrAssignS(Pstr, tmptype);
	ajStrAppendC(Pstr, " ");
	ajStrDel(&tmptype);
    }

    ajStrAppendC(Pstr, "sequence");


    if(ajCharMatchC(acdType[thys->Type].Name, "seqset"))
    {
	acdAttrToBool(thys, "aligned", gaps, &gaps);
	ajStrAppendC(Pstr, " set");

	if(gaps)
	    ajStrInsertC(Pstr, 0, "(aligned) ");
    }
    else if(ajCharMatchC(acdType[thys->Type].Name, "seqsetall"))
    {
	acdAttrToBool(thys, "aligned", gaps, &gaps);
	ajStrAppendC(Pstr, " set(s)");

	if(gaps)
	    ajStrInsertC(Pstr, 0, "(aligned) ");
    }
    else if(ajCharMatchC(acdType[thys->Type].Name, "seqall"))
    {
	ajStrAppendC(Pstr, "(s)");

	if(gaps)
	    ajStrInsertC(Pstr, 0, "(gapped) ");
    }
    else
    {
	if(gaps)
	    ajStrInsertC(Pstr, 0, "(gapped) ");
    }

    ajStrFmtTitle(Pstr);

    ajStrAppendC(Pstr,
		 " filename and optional format, or reference (input USA)");

    return;
}




/* @funcstatic acdHelpTextSeqout **********************************************
**
** Sets the help text for this ACD object to be an output sequence
** description with the type included.
**
** @param [r] thys [const AcdPAcd] Current ACD object.
** @param [w] Pstr [AjPStr*] Help text
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

static void acdHelpTextSeqout(const AcdPAcd thys, AjPStr* Pstr)
{
    const AjPStr typestr = NULL;
    AjPStr tmptype = NULL;
    AjBool gaps = AJFALSE;

    typestr = acdAttrValue(thys, "type");

    ajSeqTypeSummary(typestr, &tmptype, &gaps);
    ajStrAssignClear(Pstr);

    if(ajStrGetLen(tmptype))
    {
	ajStrAssignS(Pstr, tmptype);
	ajStrAppendC(Pstr, " ");
	ajStrDel(&tmptype);
    }

    ajStrAppendC(Pstr, "sequence");


    if(ajCharMatchC(acdType[thys->Type].Name, "seqoutset"))
    {
	ajStrAppendC(Pstr, " set");

	if(gaps)
	    ajStrInsertC(Pstr, 0, "(aligned) ");
    }
    else if(ajCharMatchC(acdType[thys->Type].Name, "seqoutall"))
    {
	ajStrAppendC(Pstr, " set(s)");

	if(gaps)
	    ajStrInsertC(Pstr, 0, "(aligned) ");
    }
    else
    {
	if(gaps)
	    ajStrInsertC(Pstr, 0, "(gapped) ");
    }

    ajStrFmtTitle(Pstr);

    ajStrAppendC(Pstr, " filename and optional format (output USA)");

    return;
}




/* @funcstatic acdPromptSeq ***************************************************
**
** Sets the default prompt for this ACD object to be a sequence
** prompt with "first", "second" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 1.0.0
** @@
******************************************************************************/

static const AjPStr acdPromptSeq(AcdPAcd thys)
{
    static ajint count=0;
    AjPStr tmptype = NULL;
    AjBool gaps = AJFALSE;
    AjPStr seqPrompt = NULL;
    AjPStr seqPromptAlt = NULL;
    const AjPStr knowntype;
    const AjPStr type = NULL;
    AjBool aligned=ajFalse;

    knowntype = acdKnowntypeDesc(thys);

    type = acdAttrValue(thys, "type");

    if(ajCharPrefixC(acdType[thys->Type].Name,"seqset"))
       acdAttrToBool(thys, "aligned", ajFalse, &aligned);

    ajSeqTypeSummary(type, &tmptype, &gaps);

    seqPrompt = ajStrNewRes(32);
    seqPromptAlt = ajStrNewRes(32);

    if(ajStrGetLen(knowntype))
	ajFmtPrintAppS(&seqPromptAlt, "%S ", knowntype);

    if(ajStrGetLen(tmptype))
	ajFmtPrintAppS(&seqPromptAlt, "%S ", tmptype);

    ajFmtPrintAppS(&seqPromptAlt, "sequence");


    if(ajCharMatchC(acdType[thys->Type].Name, "seqset"))
    {
	ajStrAppendC(&seqPromptAlt, " set");

	if(aligned)
	    ajStrInsertC(&seqPromptAlt, 0, "(aligned) ");
    }
    else if(ajCharMatchC(acdType[thys->Type].Name, "seqsetall"))
    {
	ajStrAppendC(&seqPromptAlt, " set(s)");
	if(aligned)
	    ajStrInsertC(&seqPromptAlt, 0, "(aligned) ");
    }
    else if(ajCharMatchC(acdType[thys->Type].Name, "seqall"))
    {
	ajStrAppendC(&seqPromptAlt, "(s)");

	if(gaps)
	    ajStrInsertC(&seqPromptAlt, 0, "(gapped) ");
    }
    else
    {
	if(gaps)
	    ajStrInsertC(&seqPromptAlt, 0, "(gapped) ");
    }

    ajFmtPrintS(&seqPrompt, "Input %S", seqPromptAlt);

    if(knowntype)
    {
	count++;
	acdPromptStandardS(thys, seqPromptAlt);
    }
    else
	acdPromptStandardAlt(thys, ajStrGetPtr(seqPrompt),
			     ajStrGetPtr(seqPromptAlt), &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    ajStrDel(&tmptype);
    ajStrDel(&seqPrompt);
    ajStrDel(&seqPromptAlt);

    return thys->StdPrompt;
}




/* @funcstatic acdPromptTree **************************************************
**
** Sets the default prompt for this ACD object to be a tree file
** prompt with "first", "second" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 2.8.0
** @@
******************************************************************************/

static const AjPStr acdPromptTree(AcdPAcd thys)
{
    static ajint count=0;

    const AjPStr knowntype;

    knowntype = acdKnowntypeDesc(thys);

    if(ajStrGetLen(knowntype))
    {
	count++;
	acdPromptStandardS(thys, knowntype);

	if(!ajStrSuffixC(knowntype, " file"))
	   acdPromptStandardAppend(thys, " file");
    }
    else
	acdPromptStandardAlt(thys, "Input tree file", "tree file", &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    return thys->StdPrompt;
}




/* @funcstatic acdPromptGraph *************************************************
**
** Sets the default prompt for this ACD object to be a sequence
** prompt with "first", "second" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 1.0.0
** @@
******************************************************************************/

static const AjPStr acdPromptGraph(AcdPAcd thys)
{
    static ajint count=0;

    const AjPStr knowntype;

    knowntype = acdKnowntypeDesc(thys);

    if(ajStrGetLen(knowntype))
    {
	count++;
	acdPromptStandardS(thys, knowntype);
	if(!ajStrSuffixC(knowntype, " graph"))
	   acdPromptStandardAppend(thys, " graph");
    }
    else
	acdPromptStandard(thys, "graph type", &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    return thys->StdPrompt;
}




/* @funcstatic acdPromptFeatout ***********************************************
**
** Sets the default prompt for this ACD object to be a features output
** prompt with "first", "second" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 1.0.0
** @@
******************************************************************************/

static const AjPStr acdPromptFeatout(AcdPAcd thys)
{
    static ajint count=0;

    const AjPStr knowntype;

    knowntype = acdKnowntypeDesc(thys);

    if(ajStrGetLen(knowntype))
    {
	count++;
	acdPromptStandardS(thys, knowntype);
	if(ajStrSuffixC(knowntype, " features"))
	   acdPromptStandardAppend(thys, " features");
	acdPromptStandardAppend(thys, " output");
    }
    else
	acdPromptStandard(thys, "features output", &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    return thys->StdPrompt;
}




/* @funcstatic acdPromptAlign *************************************************
**
** Sets the default prompt for this ACD object to be a report output
** prompt with "first", "second" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 2.1.0
** @@
******************************************************************************/

static const AjPStr acdPromptAlign(AcdPAcd thys)
{
    static ajint count=0;

    const AjPStr knowntype;

    knowntype = acdKnowntypeDesc(thys);

    if(ajStrGetLen(knowntype))
    {
	count++;
	acdPromptStandardS(thys, knowntype);

	if(ajStrSuffixC(knowntype, " output"))
	   acdPromptStandardAppend(thys, " alignment");
	else
	   acdPromptStandardAppend(thys, " output alignment");
    }
    else
	acdPromptStandard(thys, "output alignment", &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    return thys->StdPrompt;
}




/* @funcstatic acdPromptReport ************************************************
**
** Sets the default prompt for this ACD object to be a report output
** prompt with "first", "second" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 2.0.0
** @@
******************************************************************************/

static const AjPStr acdPromptReport(AcdPAcd thys)
{
    static ajint count=0;

    const AjPStr knowntype;

    knowntype = acdKnowntypeDesc(thys);
    if(ajStrGetLen(knowntype))
    {
	count++;
	acdPromptStandardS(thys, knowntype);

	if(ajStrSuffixC(knowntype, " output"))
	   acdPromptStandardAppend(thys, " report");
	else
	   acdPromptStandardAppend(thys, " output report");
    }
    else
    acdPromptStandard(thys, "output report", &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    return thys->StdPrompt;
}




/* @funcstatic acdPromptSeqout ************************************************
**
** Sets the default prompt for this ACD object to be a sequence output
** prompt with "first", "second" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 1.0.0
** @@
******************************************************************************/

static const AjPStr acdPromptSeqout(AcdPAcd thys)
{
    static ajint count=0;
    AjPStr tmptype = NULL;
    AjBool gaps = AJFALSE;
    AjPStr seqPrompt = NULL;
    AjPStr seqPromptAlt = NULL;
    const AjPStr knowntype;
    const AjPStr type = NULL;
    AjBool aligned=ajFalse;

    knowntype = acdKnowntypeDesc(thys);

    type = acdAttrValue(thys, "type");

    if(ajCharPrefixC(acdType[thys->Type].Name,"seqoutset"))
       acdAttrToBool(thys, "aligned", ajFalse, &aligned);

    ajSeqTypeSummary(type, &tmptype, &gaps);

    seqPrompt = ajStrNewRes(32);
    seqPromptAlt = ajStrNewRes(32);

    if(ajStrGetLen(knowntype))
	ajFmtPrintAppS(&seqPromptAlt, "%S ", knowntype);

    if(ajStrGetLen(tmptype))
	ajFmtPrintAppS(&seqPromptAlt, "%S ", tmptype);

    ajFmtPrintAppS(&seqPromptAlt, "output sequence");


    if(ajCharMatchC(acdType[thys->Type].Name, "seqoutset"))
    {
	ajStrAppendC(&seqPromptAlt, " set");
	if(aligned)
	    ajStrInsertC(&seqPromptAlt, 0, "(aligned) ");
    }
    else if(ajCharMatchC(acdType[thys->Type].Name, "seqoutsetall"))
    {
	ajStrAppendC(&seqPromptAlt, " set(s)");

	if(aligned)
	    ajStrInsertC(&seqPromptAlt, 0, "(aligned) ");
    }
    else if(ajCharMatchC(acdType[thys->Type].Name, "seqoutall"))
    {
	ajStrAppendC(&seqPromptAlt, "(s)");

	if(gaps)
	    ajStrInsertC(&seqPromptAlt, 0, "(gapped) ");
    }
    else
    {
	if(gaps)
	    ajStrInsertC(&seqPromptAlt, 0, "(gapped) ");
    }

    ajFmtPrintS(&seqPrompt, "%S", seqPromptAlt);

    if(knowntype)
    {
	count++;
	acdPromptStandardS(thys, seqPromptAlt);
    }
    else
	acdPromptStandardAlt(thys, ajStrGetPtr(seqPrompt),
			     ajStrGetPtr(seqPromptAlt), &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    ajStrDel(&tmptype);
    ajStrDel(&seqPrompt);
    ajStrDel(&seqPromptAlt);
    return thys->StdPrompt;
}




/* @funcstatic acdHelpTextObo *************************************************
**
** Sets the help text for this ACD object to be an obo term description
**
** @param [r] thys [const AcdPAcd] Current ACD object.
** @param [w] Pstr [AjPStr*] Help text
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void acdHelpTextObo(const AcdPAcd thys, AjPStr* Pstr)
{
    ajint maxreads;

    ajStrAssignClear(Pstr);

    acdAttrToInt(thys, "maxreads", 1, &maxreads);

    if(maxreads <= 1)
        ajStrAssignC(Pstr, "obo term");
    else
        ajStrAssignC(Pstr, "obo term(s)");

    ajStrFmtTitle(Pstr);

    ajStrAppendC(Pstr,
		 " filename and optional format, or reference (input query)");

    return;
}




/* @funcstatic acdHelpTextTaxon ***********************************************
**
** Sets the help text for this ACD object to be an NCBI taxonomy entry
**
** @param [r] thys [const AcdPAcd] Current ACD object.
** @param [w] Pstr [AjPStr*] Help text
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void acdHelpTextTaxon(const AcdPAcd thys, AjPStr* Pstr)
{
    ajint maxreads;

    ajStrAssignClear(Pstr);

    acdAttrToInt(thys, "maxreads", 1, &maxreads);

    if(maxreads <= 1)
        ajStrAssignC(Pstr, "taxon");
    else
        ajStrAssignC(Pstr, "taxon(s)");

    ajStrFmtTitle(Pstr);

    ajStrAppendC(Pstr,
		 " filename and optional format, or reference (input query)");

    return;
}




/* @funcstatic acdHelpTextText ************************************************
**
** Sets the help text for this ACD object to be a text entry
**
** @param [r] thys [const AcdPAcd] Current ACD object.
** @param [w] Pstr [AjPStr*] Help text
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void acdHelpTextText(const AcdPAcd thys, AjPStr* Pstr)
{
    ajint maxreads;

    ajStrAssignClear(Pstr);

    acdAttrToInt(thys, "maxreads", 1, &maxreads);

    if(maxreads <= 1)
        ajStrAssignC(Pstr, "text");
    else
        ajStrAssignC(Pstr, "text(s)");

    ajStrFmtTitle(Pstr);

    ajStrAppendC(Pstr,
		 " filename and optional format, or reference (input query)");

    return;
}




/* @funcstatic acdHelpTextUrl *************************************************
**
** Sets the help text for this ACD object to be a URL data entry
**
** @param [r] thys [const AcdPAcd] Current ACD object.
** @param [w] Pstr [AjPStr*] Help text
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void acdHelpTextUrl(const AcdPAcd thys, AjPStr* Pstr)
{
    ajint maxreads;

    ajStrAssignClear(Pstr);

    acdAttrToInt(thys, "maxreads", 1, &maxreads);

    if(maxreads <= 1)
        ajStrAssignC(Pstr, "url");
    else
        ajStrAssignC(Pstr, "url(s)");

    ajStrFmtTitle(Pstr);

    ajStrAppendC(Pstr,
		 " filename and optional format, or reference (input query)");

    return;
}




/* @funcstatic acdHelpTextVariation *******************************************
**
** Sets the help text for this ACD object to be a variation data entry
**
** @param [r] thys [const AcdPAcd] Current ACD object.
** @param [w] Pstr [AjPStr*] Help text
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void acdHelpTextVariation(const AcdPAcd thys, AjPStr* Pstr)
{
    ajint maxreads;

    ajStrAssignClear(Pstr);

    acdAttrToInt(thys, "maxreads", 1, &maxreads);

    if(maxreads <= 1)
        ajStrAssignC(Pstr, "variation");
    else
        ajStrAssignC(Pstr, "variation(s)");

    ajStrFmtTitle(Pstr);

    ajStrAppendC(Pstr,
		 " filename and optional format, or reference (input query)");

    return;
}




/* @funcstatic acdHelpTextXml *************************************************
**
** Sets the help text for this ACD object to be a xml description
**
** @param [r] thys [const AcdPAcd] Current ACD object.
** @param [w] Pstr [AjPStr*] Help text
** @return [void]
**
** @release 6.6.0
** @@
******************************************************************************/

static void acdHelpTextXml(const AcdPAcd thys, AjPStr* Pstr)
{
    ajint maxreads;

    ajStrAssignClear(Pstr);

    acdAttrToInt(thys, "maxreads", 1, &maxreads);

    if(maxreads <= 1)
        ajStrAssignC(Pstr, "xml");
    else
        ajStrAssignC(Pstr, "xml(s)");

    ajStrFmtTitle(Pstr);

    ajStrAppendC(Pstr,
		 " filename and optional format, or reference (input query)");

    return;
}




/* @funcstatic acdPromptAssembly **********************************************
**
** Sets the default prompt for this ACD object to be an assembly
** prompt with "first", "second" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 6.5.0
** @@
******************************************************************************/

static const AjPStr acdPromptAssembly(AcdPAcd thys)
{
    static ajint count=0;
    AjPStr assemPrompt = NULL;
    AjPStr assemPromptAlt = NULL;
    const AjPStr knowntype;

    knowntype = acdKnowntypeDesc(thys);

    assemPrompt = ajStrNewRes(32);
    assemPromptAlt = ajStrNewRes(32);

    if(ajStrGetLen(knowntype))
	ajFmtPrintAppS(&assemPromptAlt, "%S ", knowntype);

    ajFmtPrintAppS(&assemPromptAlt, "assembly");

    ajFmtPrintS(&assemPrompt, "Input %S", assemPromptAlt);

    if(knowntype)
    {
	count++;
	acdPromptStandardS(thys, assemPromptAlt);
    }
    else
	acdPromptStandardAlt(thys, ajStrGetPtr(assemPrompt),
			     ajStrGetPtr(assemPromptAlt), &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    ajStrDel(&assemPrompt);
    ajStrDel(&assemPromptAlt);

    return thys->StdPrompt;
}




/* @funcstatic acdPromptObo ***************************************************
**
** Sets the default prompt for this ACD object to be an obo term
** prompt with "first", "second" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 6.4.0
** @@
******************************************************************************/

static const AjPStr acdPromptObo(AcdPAcd thys)
{
    static ajint count=0;
    AjPStr oboPrompt = NULL;
    AjPStr oboPromptAlt = NULL;
    const AjPStr knowntype;
    ajint maxreads = 0;

    acdAttrToInt(thys, "maxreads", 1, &maxreads);

    knowntype = acdKnowntypeDesc(thys);

    oboPrompt = ajStrNewRes(32);
    oboPromptAlt = ajStrNewRes(32);

    if(ajStrGetLen(knowntype))
	ajFmtPrintAppS(&oboPromptAlt, "%S ", knowntype);

    ajFmtPrintAppS(&oboPromptAlt, "obo term");

    if(maxreads > 1)
	ajStrAppendC(&oboPromptAlt, "(s)");

    ajFmtPrintS(&oboPrompt, "Input %S", oboPromptAlt);

    if(knowntype)
    {
	count++;
	acdPromptStandardS(thys, oboPromptAlt);
    }
    else
	acdPromptStandardAlt(thys, ajStrGetPtr(oboPrompt),
			     ajStrGetPtr(oboPromptAlt), &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    ajStrDel(&oboPrompt);
    ajStrDel(&oboPromptAlt);

    return thys->StdPrompt;
}




/* @funcstatic acdPromptOutassembly *******************************************
**
** Sets the default prompt for this ACD object to be a simple
** prompt with "second", "third" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 2.9.0
** @@
******************************************************************************/

static const AjPStr acdPromptOutassembly(AcdPAcd thys)
{
    static ajint count=0;

    const AjPStr knowntype;

    knowntype = acdKnowntypeDesc(thys);

    if(ajStrGetLen(knowntype))
    {
	count++;
	acdPromptStandardS(thys, knowntype);
	acdPromptStandardAppend(thys, " output file");
    }
    else
	acdPromptStandard(thys, "assembly output file", &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    return thys->StdPrompt;
}




/* @funcstatic acdPromptOutcodon **********************************************
**
** Sets the default prompt for this ACD object to be a simple
** prompt with "second", "third" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 2.9.0
** @@
******************************************************************************/

static const AjPStr acdPromptOutcodon(AcdPAcd thys)
{
    static ajint count=0;

    const AjPStr knowntype;

    knowntype = acdKnowntypeDesc(thys);

    if(ajStrGetLen(knowntype))
    {
	count++;
	acdPromptStandardS(thys, knowntype);
	acdPromptStandardAppend(thys, " output file");
    }
    else
	acdPromptStandard(thys, "codon usage output file", &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    return thys->StdPrompt;
}




/* @funcstatic acdPromptOutcpdb ***********************************************
**
** Sets the default prompt for this ACD object to be a simple
** prompt with "second", "third" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 2.9.0
** @@
******************************************************************************/

static const AjPStr acdPromptOutcpdb(AcdPAcd thys)
{
    static ajint count=0;

    const AjPStr knowntype;

    knowntype = acdKnowntypeDesc(thys);

    if(ajStrGetLen(knowntype))
    {
	count++;
	acdPromptStandardS(thys, knowntype);
	acdPromptStandardAppend(thys, " output file");
    }
    else
	acdPromptStandard(thys, "clean PDB output file", &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    return thys->StdPrompt;
}




/* @funcstatic acdPromptOutdata ***********************************************
**
** Sets the default prompt for this ACD object to be a simple
** prompt with "second", "third" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 2.9.0
** @@
******************************************************************************/

static const AjPStr acdPromptOutdata(AcdPAcd thys)
{
    static ajint count=0;

    const AjPStr knowntype;

    knowntype = acdKnowntypeDesc(thys);

    if(ajStrGetLen(knowntype))
    {
	count++;
	acdPromptStandardS(thys, knowntype);
	acdPromptStandardAppend(thys, " output file");
    }
    else
	acdPromptStandard(thys, "output data file", &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    return thys->StdPrompt;
}




/* @funcstatic acdPromptOutdir ************************************************
**
** Sets the default prompt for this ACD object to be a simple
** prompt with "second", "third" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 2.9.0
** @@
******************************************************************************/

static const AjPStr acdPromptOutdir(AcdPAcd thys)
{
    static ajint count=0;

    const AjPStr knowntype;

    knowntype = acdKnowntypeDesc(thys);

    if(ajStrGetLen(knowntype))
    {
	count++;
	acdPromptStandardS(thys, knowntype);

	if(!ajStrSuffixC(knowntype, " file"))
	    acdPromptStandardAppend(thys, " file");
	acdPromptStandardAppend(thys, " output directory");
    }
    else
	acdPromptStandard(thys, "output directory", &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    return thys->StdPrompt;
}




/* @funcstatic acdPromptOutdiscrete *******************************************
**
** Sets the default prompt for this ACD object to be a simple
** prompt with "second", "third" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 2.9.0
** @@
******************************************************************************/

static const AjPStr acdPromptOutdiscrete(AcdPAcd thys)
{
    static ajint count=0;

    const AjPStr knowntype;

    knowntype = acdKnowntypeDesc(thys);

    if(ajStrGetLen(knowntype))
    {
	count++;
	acdPromptStandardS(thys, knowntype);
	acdPromptStandardAppend(thys, " output discrete data file");
    }
    else
	acdPromptStandard(thys, "discrete data output file", &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    return thys->StdPrompt;
}




/* @funcstatic acdPromptOutdistance *******************************************
**
** Sets the default prompt for this ACD object to be a simple
** prompt with "second", "third" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 2.9.0
** @@
******************************************************************************/

static const AjPStr acdPromptOutdistance(AcdPAcd thys)
{
    static ajint count=0;

    const AjPStr knowntype;

    knowntype = acdKnowntypeDesc(thys);
    if(ajStrGetLen(knowntype))
    {
	count++;
	acdPromptStandardS(thys, knowntype);
	acdPromptStandardAppend(thys, " distance data output file");
    }
    else
	acdPromptStandard(thys, "output distance data file", &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    return thys->StdPrompt;
}




/* @funcstatic acdPromptOutfreq ***********************************************
**
** Sets the default prompt for this ACD object to be a simple
** prompt with "second", "third" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 2.9.0
** @@
******************************************************************************/

static const AjPStr acdPromptOutfreq(AcdPAcd thys)
{
    static ajint count=0;

    const AjPStr knowntype;

    knowntype = acdKnowntypeDesc(thys);

    if(ajStrGetLen(knowntype))
    {
	count++;
	acdPromptStandardS(thys, knowntype);
	acdPromptStandardAppend(thys, " frequency data output file");
    }
    else
    acdPromptStandard(thys, "frequency data output file", &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    return thys->StdPrompt;
}




/* @funcstatic acdPromptOutmatrix *********************************************
**
** Sets the default prompt for this ACD object to be a simple
** prompt with "second", "third" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 2.9.0
** @@
******************************************************************************/

static const AjPStr acdPromptOutmatrix(AcdPAcd thys)
{
    static ajint count=0;

    const AjPStr knowntype;

    knowntype = acdKnowntypeDesc(thys);
    if(ajStrGetLen(knowntype))
    {
	count++;
	acdPromptStandardS(thys, knowntype);
	acdPromptStandardAppend(thys, " matrix output file");
    }
    else
	acdPromptStandard(thys, "matrix output file", &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    return thys->StdPrompt;
}




/* @funcstatic acdPromptOutobo ************************************************
**
** Sets the default prompt for this ACD object to be a simple
** prompt with "second", "third" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 6.4.0
** @@
******************************************************************************/

static const AjPStr acdPromptOutobo(AcdPAcd thys)
{
    static ajint count=0;

    const AjPStr knowntype;

    knowntype = acdKnowntypeDesc(thys);

    if(ajStrGetLen(knowntype))
    {
	count++;
	acdPromptStandardS(thys, knowntype);
	acdPromptStandardAppend(thys, " obo output file");
    }
    else
	acdPromptStandard(thys, "obo output file", &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    return thys->StdPrompt;
}




/* @funcstatic acdPromptOutproperties *****************************************
**
** Sets the default prompt for this ACD object to be a simple
** prompt with "second", "third" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 2.9.0
** @@
******************************************************************************/

static const AjPStr acdPromptOutproperties(AcdPAcd thys)
{
    static ajint count=0;

    const AjPStr knowntype;

    knowntype = acdKnowntypeDesc(thys);

    if(ajStrGetLen(knowntype))
    {
	count++;
	acdPromptStandardS(thys, knowntype);
	acdPromptStandardAppend(thys, " properties data output file");
    }
    else
	acdPromptStandard(thys, "properties data output file", &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    return thys->StdPrompt;
}




/* @funcstatic acdPromptOutrefseq *********************************************
**
** Sets the default prompt for this ACD object to be a simple
** prompt with "second", "third" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 2.9.0
** @@
******************************************************************************/

static const AjPStr acdPromptOutrefseq(AcdPAcd thys)
{
    static ajint count=0;

    const AjPStr knowntype;

    knowntype = acdKnowntypeDesc(thys);

    if(ajStrGetLen(knowntype))
    {
	count++;
	acdPromptStandardS(thys, knowntype);
	acdPromptStandardAppend(thys, " output file");
    }
    else
	acdPromptStandard(thys, "reference sequence output file", &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    return thys->StdPrompt;
}




/* @funcstatic acdPromptOutresource *******************************************
**
** Sets the default prompt for this ACD object to be a simple
** prompt with "second", "third" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 6.4.0
** @@
******************************************************************************/

static const AjPStr acdPromptOutresource(AcdPAcd thys)
{
    static ajint count=0;

    const AjPStr knowntype;

    knowntype = acdKnowntypeDesc(thys);

    if(ajStrGetLen(knowntype))
    {
	count++;
	acdPromptStandardS(thys, knowntype);
	acdPromptStandardAppend(thys, " data resource output file");
    }
    else
	acdPromptStandard(thys, "data resource output file", &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    return thys->StdPrompt;
}




/* @funcstatic acdPromptOutscop ***********************************************
**
** Sets the default prompt for this ACD object to be a simple
** prompt with "second", "third" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 2.9.0
** @@
******************************************************************************/

static const AjPStr acdPromptOutscop(AcdPAcd thys)
{
    static ajint count=0;

    const AjPStr knowntype;

    knowntype = acdKnowntypeDesc(thys);

    if(ajStrGetLen(knowntype))
    {
	count++;
	acdPromptStandardS(thys, knowntype);
	acdPromptStandardAppend(thys, " scop output file");
    }
    else
	acdPromptStandard(thys, "scop output file", &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    return thys->StdPrompt;
}




/* @funcstatic acdPromptOuttaxon **********************************************
**
** Sets the default prompt for this ACD object to be a simple
** prompt with "second", "third" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 6.4.0
** @@
******************************************************************************/

static const AjPStr acdPromptOuttaxon(AcdPAcd thys)
{
    static ajint count=0;

    const AjPStr knowntype;

    knowntype = acdKnowntypeDesc(thys);

    if(ajStrGetLen(knowntype))
    {
	count++;
	acdPromptStandardS(thys, knowntype);
	acdPromptStandardAppend(thys, " taxon output file");
    }
    else
	acdPromptStandard(thys, "taxon output file", &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    return thys->StdPrompt;
}




/* @funcstatic acdPromptOuttext ***********************************************
**
** Sets the default prompt for this ACD object to be a simple
** prompt with "second", "third" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 6.4.0
** @@
******************************************************************************/

static const AjPStr acdPromptOuttext(AcdPAcd thys)
{
    static ajint count=0;

    const AjPStr knowntype;

    knowntype = acdKnowntypeDesc(thys);

    if(ajStrGetLen(knowntype))
    {
	count++;
	acdPromptStandardS(thys, knowntype);
	acdPromptStandardAppend(thys, " text output file");
    }
    else
	acdPromptStandard(thys, "text output file", &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    return thys->StdPrompt;
}




/* @funcstatic acdPromptOuttree ***********************************************
**
** Sets the default prompt for this ACD object to be a simple
** prompt with "second", "third" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 2.9.0
** @@
******************************************************************************/

static const AjPStr acdPromptOuttree(AcdPAcd thys)
{
    static ajint count=0;

    const AjPStr knowntype;

    knowntype = acdKnowntypeDesc(thys);

    if(ajStrGetLen(knowntype))
    {
	count++;
	acdPromptStandardS(thys, knowntype);
	acdPromptStandardAppend(thys, " tree output file");
    }
    else
	acdPromptStandard(thys, "tree output file", &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    return thys->StdPrompt;
}




/* @funcstatic acdPromptOuturl ************************************************
**
** Sets the default prompt for this ACD object to be a simple
** prompt with "second", "third" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 6.4.0
** @@
******************************************************************************/

static const AjPStr acdPromptOuturl(AcdPAcd thys)
{
    static ajint count=0;

    const AjPStr knowntype;

    knowntype = acdKnowntypeDesc(thys);

    if(ajStrGetLen(knowntype))
    {
	count++;
	acdPromptStandardS(thys, knowntype);
	acdPromptStandardAppend(thys, " URL output file");
    }
    else
	acdPromptStandard(thys, "URL output file", &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    return thys->StdPrompt;
}




/* @funcstatic acdPromptOutvariation ******************************************
**
** Sets the default prompt for this ACD object to be a simple
** prompt with "second", "third" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 6.4.0
** @@
******************************************************************************/

static const AjPStr acdPromptOutvariation(AcdPAcd thys)
{
    static ajint count=0;

    const AjPStr knowntype;

    knowntype = acdKnowntypeDesc(thys);

    if(ajStrGetLen(knowntype))
    {
	count++;
	acdPromptStandardS(thys, knowntype);
	acdPromptStandardAppend(thys, " variation output file");
    }
    else
	acdPromptStandard(thys, "variation output file", &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    return thys->StdPrompt;
}




/* @funcstatic acdPromptOutxml ************************************************
**
** Sets the default prompt for this ACD object to be a simple
** prompt with "second", "third" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 6.6.0
** @@
******************************************************************************/

static const AjPStr acdPromptOutxml(AcdPAcd thys)
{
    static ajint count=0;

    const AjPStr knowntype;

    knowntype = acdKnowntypeDesc(thys);

    if(ajStrGetLen(knowntype))
    {
	count++;
	acdPromptStandardS(thys, knowntype);
	acdPromptStandardAppend(thys, " output file");
    }
    else
	acdPromptStandard(thys, "xml output file", &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    return thys->StdPrompt;
}




/* @funcstatic acdPromptRefseq **********************************************
**
** Sets the default prompt for this ACD object to be a reference sequence
** prompt with "first", "second" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 6.5.0
** @@
******************************************************************************/

static const AjPStr acdPromptRefseq(AcdPAcd thys)
{
    static ajint count=0;
    AjPStr refseqPrompt = NULL;
    AjPStr refseqPromptAlt = NULL;
    const AjPStr knowntype;

    knowntype = acdKnowntypeDesc(thys);

    refseqPrompt = ajStrNewRes(32);
    refseqPromptAlt = ajStrNewRes(32);

    if(ajStrGetLen(knowntype))
	ajFmtPrintAppS(&refseqPromptAlt, "%S ", knowntype);

    ajFmtPrintAppS(&refseqPromptAlt, "reference sequence");

    ajFmtPrintS(&refseqPrompt, "Input %S", refseqPromptAlt);

    if(knowntype)
    {
	count++;
	acdPromptStandardS(thys, refseqPromptAlt);
    }
    else
	acdPromptStandardAlt(thys, ajStrGetPtr(refseqPrompt),
			     ajStrGetPtr(refseqPromptAlt), &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    ajStrDel(&refseqPrompt);
    ajStrDel(&refseqPromptAlt);

    return thys->StdPrompt;
}




/* @funcstatic acdPromptTaxon *************************************************
**
** Sets the default prompt for this ACD object to be an NCBI taxonomy entry
** prompt with "first", "second" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 6.4.0
** @@
******************************************************************************/

static const AjPStr acdPromptTaxon(AcdPAcd thys)
{
    static ajint count=0;
    AjPStr taxPrompt = NULL;
    AjPStr taxPromptAlt = NULL;
    const AjPStr knowntype;
    ajint maxreads = 0;

    acdAttrToInt(thys, "maxreads", 1, &maxreads);

    knowntype = acdKnowntypeDesc(thys);

    taxPrompt = ajStrNewRes(32);
    taxPromptAlt = ajStrNewRes(32);

    if(ajStrGetLen(knowntype))
	ajFmtPrintAppS(&taxPromptAlt, "%S ", knowntype);

    ajFmtPrintAppS(&taxPromptAlt, "taxon");

    if(maxreads > 1)
	ajStrAppendC(&taxPromptAlt, "(s)");

    ajFmtPrintS(&taxPrompt, "Input %S", taxPromptAlt);

    if(knowntype)
    {
	count++;
	acdPromptStandardS(thys, taxPromptAlt);
    }
    else
	acdPromptStandardAlt(thys, ajStrGetPtr(taxPrompt),
			     ajStrGetPtr(taxPromptAlt), &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    ajStrDel(&taxPrompt);
    ajStrDel(&taxPromptAlt);

    return thys->StdPrompt;
}




/* @funcstatic acdPromptText **************************************************
**
** Sets the default prompt for this ACD object to be a text entry
** prompt with "first", "second" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 6.4.0
** @@
******************************************************************************/

static const AjPStr acdPromptText(AcdPAcd thys)
{
    static ajint count=0;
    AjPStr taxPrompt = NULL;
    AjPStr taxPromptAlt = NULL;
    const AjPStr knowntype;
    ajint maxreads = 0;

    acdAttrToInt(thys, "maxreads", 1, &maxreads);

    knowntype = acdKnowntypeDesc(thys);

    taxPrompt = ajStrNewRes(32);
    taxPromptAlt = ajStrNewRes(32);

    if(ajStrGetLen(knowntype))
	ajFmtPrintAppS(&taxPromptAlt, "%S ", knowntype);

    ajFmtPrintAppS(&taxPromptAlt, "text");

    if(maxreads > 1)
	ajStrAppendC(&taxPromptAlt, "(s)");

    ajFmtPrintS(&taxPrompt, "Input %S", taxPromptAlt);

    if(knowntype)
    {
	count++;
	acdPromptStandardS(thys, taxPromptAlt);
    }
    else
	acdPromptStandardAlt(thys, ajStrGetPtr(taxPrompt),
			     ajStrGetPtr(taxPromptAlt), &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    ajStrDel(&taxPrompt);
    ajStrDel(&taxPromptAlt);

    return thys->StdPrompt;
}




/* @funcstatic acdPromptUrl ***************************************************
**
** Sets the default prompt for this ACD object to be a URL data entry
** prompt with "first", "second" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 6.4.0
** @@
******************************************************************************/

static const AjPStr acdPromptUrl(AcdPAcd thys)
{
    static ajint count=0;
    AjPStr urlPrompt = NULL;
    AjPStr urlPromptAlt = NULL;
    const AjPStr knowntype;
    ajint maxreads = 0;

    acdAttrToInt(thys, "maxreads", 1, &maxreads);

    knowntype = acdKnowntypeDesc(thys);

    urlPrompt = ajStrNewRes(32);
    urlPromptAlt = ajStrNewRes(32);

    if(ajStrGetLen(knowntype))
	ajFmtPrintAppS(&urlPromptAlt, "%S ", knowntype);

    ajFmtPrintAppS(&urlPromptAlt, "url");

    if(maxreads > 1)
	ajStrAppendC(&urlPromptAlt, "(s)");

    ajFmtPrintS(&urlPrompt, "Input %S", urlPromptAlt);

    if(knowntype)
    {
	count++;
	acdPromptStandardS(thys, urlPromptAlt);
    }
    else
	acdPromptStandardAlt(thys, ajStrGetPtr(urlPrompt),
			     ajStrGetPtr(urlPromptAlt), &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    ajStrDel(&urlPrompt);
    ajStrDel(&urlPromptAlt);

    return thys->StdPrompt;
}




/* @funcstatic acdPromptVariation *********************************************
**
** Sets the default prompt for this ACD object to be a variation entry
** prompt with "first", "second" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 6.4.0
** @@
******************************************************************************/

static const AjPStr acdPromptVariation(AcdPAcd thys)
{
    static ajint count=0;
    AjPStr varPrompt = NULL;
    AjPStr varPromptAlt = NULL;
    const AjPStr knowntype;
    ajint maxreads = 0;

    acdAttrToInt(thys, "maxreads", 1, &maxreads);

    knowntype = acdKnowntypeDesc(thys);

    varPrompt = ajStrNewRes(32);
    varPromptAlt = ajStrNewRes(32);

    if(ajStrGetLen(knowntype))
	ajFmtPrintAppS(&varPromptAlt, "%S ", knowntype);

    ajFmtPrintAppS(&varPromptAlt, "variation");

    if(maxreads > 1)
	ajStrAppendC(&varPromptAlt, "(s)");

    ajFmtPrintS(&varPrompt, "Input %S", varPromptAlt);

    if(knowntype)
    {
	count++;
	acdPromptStandardS(thys, varPromptAlt);
    }
    else
	acdPromptStandardAlt(thys, ajStrGetPtr(varPrompt),
			     ajStrGetPtr(varPromptAlt), &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    ajStrDel(&varPrompt);
    ajStrDel(&varPromptAlt);

    return thys->StdPrompt;
}




/* @funcstatic acdPromptXml ***************************************************
**
** Sets the default prompt for this ACD object to be a xml
** prompt with "first", "second" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 6.6.0
** @@
******************************************************************************/

static const AjPStr acdPromptXml(AcdPAcd thys)
{
    static ajint count=0;
    AjPStr xmlPrompt = NULL;
    AjPStr xmlPromptAlt = NULL;
    const AjPStr knowntype;
    ajint maxreads = 0;

    acdAttrToInt(thys, "maxreads", 1, &maxreads);

    knowntype = acdKnowntypeDesc(thys);

    xmlPrompt = ajStrNewRes(32);
    xmlPromptAlt = ajStrNewRes(32);

    if(ajStrGetLen(knowntype))
	ajFmtPrintAppS(&xmlPromptAlt, "%S ", knowntype);

    ajFmtPrintAppS(&xmlPromptAlt, "xml");

    if(maxreads > 1)
	ajStrAppendC(&xmlPromptAlt, "(s)");

    ajFmtPrintS(&xmlPrompt, "Input %S", xmlPromptAlt);

    if(knowntype)
    {
	count++;
	acdPromptStandardS(thys, xmlPromptAlt);
    }
    else
	acdPromptStandardAlt(thys, ajStrGetPtr(xmlPrompt),
			     ajStrGetPtr(xmlPromptAlt), &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    ajStrDel(&xmlPrompt);
    ajStrDel(&xmlPromptAlt);

    return thys->StdPrompt;
}




/* @funcstatic acdPromptStandard **********************************************
**
** Sets the default prompt for this ACD object to be an output
** file with "first", "second" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @param [r] type [const char*] Data type for prompt
** @param [w] count [ajint*] count for number of calls
** @return [void]
**
** @release 2.9.0
** @@
******************************************************************************/

static void acdPromptStandard(AcdPAcd thys, const char* type, ajint* count)
{
    AjPStr *prompt;

    (*count)++;

    acdLog("acdPromptStandard '%s' count %d\n", type, *count);

    if(!thys->DefStr)
    {
	acdLog("acdPromptStandard '%s' thys->DefStr NULL\n", type);
	return;
    }

    prompt = &thys->DefStr[DEF_PROMPT];

    if(ajStrGetLen(*prompt))
    {
	acdLog("acdPromptStandard '%s' found thys->DefStr[DEF_PROMPT] '%S'\n",
	       type, *prompt);
	/*ajStrTrace(*prompt);*/
	/*ajStrTrace(thys->DefStr[DEF_PROMPT]);*/
	return;
    }

    switch(*count)
    {
        case 1:
            ajFmtPrintS(&thys->StdPrompt,
                        "%s", type);
            ajStrFmtTitle(&thys->StdPrompt);
            break;
        case 2:
            ajFmtPrintS(&thys->StdPrompt,
                            "Second %s", type);
            break;
        case 3:
            ajFmtPrintS(&thys->StdPrompt,
                            "Third %s", type);
            break;
        case 11:
        case 12:
        case 13:
            ajFmtPrintS(&thys->StdPrompt,
                        "%dth %s", count, type);
            break;
        default:
            switch(*count % 10)
            {
                case 1:
                    ajFmtPrintS(&thys->StdPrompt,
                                    "%dst %s", count, type);
                    break;
                case 2:
                    ajFmtPrintS(&thys->StdPrompt,
                                    "%dnd %s", count, type);
                    break;
                case 3:
                    ajFmtPrintS(&thys->StdPrompt,
                                    "%drd %s", count, type);
                    break;
                default:
                    ajFmtPrintS(&thys->StdPrompt,
                                     "%dth %s", count, type);
                    break;
            }
            break;
    }

    return;
}




/* @funcstatic acdPromptStandardAlt *******************************************
**
** Sets the default prompt for this ACD object to as specified, with
** "second", "third" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @param [r] firsttype [const char*] Data type for prompt first time
** @param [r] type [const char*] Data type for prompt subsequent times
** @param [w] count [ajint*] count for number of calls
** @return [void]
**
** @release 2.9.0
** @@
******************************************************************************/

static void acdPromptStandardAlt(AcdPAcd thys, const char* firsttype,
				 const char* type, ajint* count)
{
    AjPStr *prompt;

    (*count)++;

    acdLog("acdPromptStandardAlt '%s' count %d\n", type, *count);

    if(!thys->DefStr)
    {
	acdLog("acdPromptStandardAlt '%s' thys->DefStr NULL\n", type);
	return;
    }

    prompt = &thys->DefStr[DEF_PROMPT];

    if(ajStrGetLen(*prompt))
    {
	acdLog("acdPromptStandardAlt '%s' "
	       "found thys->DefStr[DEF_PROMPT] '%S'\n",
	       type, *prompt);
	/*ajStrTrace(*prompt);*/
	/*ajStrTrace(thys->DefStr[DEF_PROMPT]);*/
	return;
    }

    switch(*count)
    {
        case 1:
            ajFmtPrintS(&thys->StdPrompt,
                        "%s", firsttype);
            break;
        case 2:
            ajFmtPrintS(&thys->StdPrompt,
                            "Second %s", type);
            break;
        case 3:
            ajFmtPrintS(&thys->StdPrompt,
                            "Third %s", type);
            break;
        case 11:
        case 12:
        case 13:
            ajFmtPrintS(&thys->StdPrompt,
                        "%dth %s", count, type);
            break;
        default:
            switch(*count % 10)
            {
                case 1:
                    ajFmtPrintS(&thys->StdPrompt,
                                    "%dst %s", count, type);
                    break;
                case 2:
                    ajFmtPrintS(&thys->StdPrompt,
                                    "%dnd %s", count, type);
                    break;
                case 3:
                    ajFmtPrintS(&thys->StdPrompt,
                                    "%drd %s", count, type);
                    break;
                default:
                    ajFmtPrintS(&thys->StdPrompt,
                                     "%dth %s", count, type);
                    break;
            }
            break;
    }

    return;
}




/* @funcstatic acdPromptStandardAppend ****************************************
**
** Appends to the default prompt for this ACD object
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @param [r] str[const char*] Suffix to append to prompt
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

static void acdPromptStandardAppend(AcdPAcd thys, const char* str)
{
    ajStrAppendC(&thys->StdPrompt, str);
}




/* @funcstatic acdPromptStandardS *********************************************
**
** Appends to the default prompt for this ACD object
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @param [r] str [const AjPStr] Data type for prompt
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

static void acdPromptStandardS(AcdPAcd thys, const AjPStr str)
{
    ajStrAssignS(&thys->StdPrompt, str);
}




/* @funcstatic acdPromptOutfile ***********************************************
**
** Sets the default prompt for this ACD object to be an output
** file with "first", "second" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 1.0.0
** @@
******************************************************************************/

static const AjPStr acdPromptOutfile(AcdPAcd thys)
{
    static ajint count = 0;

    const AjPStr knowntype;

    knowntype = acdKnowntypeDesc(thys);

    if(ajStrGetLen(knowntype))
    {
	count++;
	acdPromptStandardS(thys, knowntype);

	if(ajStrSuffixC(knowntype, " output"))
	   acdPromptStandardAppend(thys, " file");
	else
	   acdPromptStandardAppend(thys, " output file");
    }
    else
	acdPromptStandard(thys, "output file", &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    return thys->StdPrompt;
}




/* @funcstatic acdPromptInfile ************************************************
**
** Sets the default prompt for this ACD object to be an input
** file with "first", "second" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 1.0.0
** @@
******************************************************************************/

static const AjPStr acdPromptInfile(AcdPAcd thys)
{
    static ajint count = 0;

    const AjPStr knowntype;

    knowntype = acdKnowntypeDesc(thys);

    if(ajStrGetLen(knowntype))
    {
	count++;
	acdPromptStandardS(thys, knowntype);

	if(!ajStrSuffixC(knowntype, " file"))
	   acdPromptStandardAppend(thys, " file");
    }
    else
	acdPromptStandard(thys, "input file", &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    return thys->StdPrompt;
}




/* @funcstatic acdPromptDatafile **********************************************
**
** Sets the default prompt for this ACD object to be an input data
** file with "first", "second" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 4.0.0
** @@
******************************************************************************/

static const AjPStr acdPromptDatafile(AcdPAcd thys)
{
    static ajint count = 0;

    const AjPStr knowntype;

    knowntype = acdKnowntypeDesc(thys);

    if(ajStrGetLen(knowntype))
    {
	count++;
	acdPromptStandardS(thys, knowntype);

	if(ajStrSuffixC(knowntype, " data"))
	   acdPromptStandardAppend(thys, " file");
	else
	   acdPromptStandardAppend(thys, " data file");
    }
    else
	acdPromptStandard(thys, "data file", &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    return thys->StdPrompt;
}




/* @funcstatic acdPromptMatrix ************************************************
**
** Sets the default prompt for this ACD object to be an input data
** file with "first", "second" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 4.0.0
** @@
******************************************************************************/

static const AjPStr acdPromptMatrix(AcdPAcd thys)
{
    static ajint count = 0;
    AjBool protein = ajFalse;

    const AjPStr knowntype;

    acdAttrToBool(thys, "protein", ajFalse, &protein);

    knowntype = acdKnowntypeDesc(thys);

    if(ajStrGetLen(knowntype))
    {
	count++;
	acdPromptStandardS(thys, knowntype);

	if(!ajStrSuffixC(knowntype, " file"))
	   acdPromptStandardAppend(thys, " file");
    }
    else
    {
	if(acdDoValid)
	    acdPromptStandard(thys, "comparison matrix file",
			      &count);
	else
	{
	    if(protein)
		acdPromptStandard(thys, "(protein) comparison matrix file",
				  &count);
	    else
		acdPromptStandard(thys, "(nucleotide) comparison matrix file",
				  &count);
	}
    }

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    return thys->StdPrompt;
}




/* @funcstatic acdPromptDiscretestates ****************************************
**
** Sets the default prompt for this ACD object to be an input discrete states
** file with "first", "second" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 4.0.0
** @@
******************************************************************************/

static const AjPStr acdPromptDiscretestates(AcdPAcd thys)
{
    static ajint count = 0;

    const AjPStr knowntype;

    knowntype = acdKnowntypeDesc(thys);

    if(ajStrGetLen(knowntype))
    {
	count++;
	acdPromptStandardS(thys, knowntype);
	if(!ajStrSuffixC(knowntype, " file"))
	   acdPromptStandardAppend(thys, " file");
    }
    else
	acdPromptStandard(thys, "discrete states file", &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    return thys->StdPrompt;
}




/* @funcstatic acdPromptDistances *********************************************
**
** Sets the default prompt for this ACD object to be an input distances
** file with "first", "second" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 4.0.0
** @@
******************************************************************************/

static const AjPStr acdPromptDistances(AcdPAcd thys)
{
    static ajint count = 0;

    const AjPStr knowntype;

    knowntype = acdKnowntypeDesc(thys);

    if(ajStrGetLen(knowntype))
    {
	count++;
	acdPromptStandardS(thys, knowntype);

	if(!ajStrSuffixC(knowntype, " file"))
	   acdPromptStandardAppend(thys, " file");
    }
    else
	acdPromptStandard(thys, "distances file", &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    return thys->StdPrompt;
}




/* @funcstatic acdPromptFrequencies *******************************************
**
** Sets the default prompt for this ACD object to be an input frequencies
** file with "first", "second" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 4.0.0
** @@
******************************************************************************/

static const AjPStr acdPromptFrequencies(AcdPAcd thys)
{
    static ajint count = 0;

    const AjPStr knowntype;

    knowntype = acdKnowntypeDesc(thys);

    if(ajStrGetLen(knowntype))
    {
	count++;
	acdPromptStandardS(thys, knowntype);

	if(!ajStrSuffixC(knowntype, " file"))
	   acdPromptStandardAppend(thys, " file");
    }
    else
	acdPromptStandard(thys, "frequencies file", &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    return thys->StdPrompt;
}




/* @funcstatic acdPromptProperties ********************************************
**
** Sets the default prompt for this ACD object to be an input properties
** file with "first", "second" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 4.0.0
** @@
******************************************************************************/

static const AjPStr acdPromptProperties(AcdPAcd thys)
{
    static ajint count = 0;

    const AjPStr knowntype;

    knowntype = acdKnowntypeDesc(thys);

    if(ajStrGetLen(knowntype))
    {
	count++;
	acdPromptStandardS(thys, knowntype);

	if(!ajStrSuffixC(knowntype, " file"))
	   acdPromptStandardAppend(thys, " file");
    }
    else
	acdPromptStandard(thys, "properties file", &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    return thys->StdPrompt;
}




/* @funcstatic acdPromptPattern ***********************************************
**
** Sets the default prompt for this ACD object to be a pattern string or
** file with "first", "second" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 4.0.0
** @@
******************************************************************************/

static const AjPStr acdPromptPattern(AcdPAcd thys)
{
    static ajint count = 0;
    AjPStr type = NULL;

    const AjPStr knowntype;

    acdAttrToStr(thys, "type", "", &type);

    knowntype = acdKnowntypeDesc(thys);

    if(ajStrGetLen(knowntype))
    {
	count++;
	acdPromptStandardS(thys, knowntype);

	if(!ajStrSuffixC(knowntype, " pattern"))
	   acdPromptStandardAppend(thys, " pattern");
    }
    else
    {
	if(ajStrPrefixCaseC(type, "p"))
	    acdPromptStandard(thys,
			      "protein pattern string or @file", &count);
	else if(ajStrPrefixCaseC(type, "n"))
	    acdPromptStandard(thys,
			      "nucleotide pattern string or @file", &count);
	else
	    acdPromptStandard(thys,
			      "pattern string or @file", &count);
    }

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    return thys->StdPrompt;
}




/* @funcstatic acdPromptRegexp ************************************************
**
** Sets the default prompt for this ACD object to be a regular expression
** string or file with "first", "second" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 4.0.0
** @@
******************************************************************************/

static const AjPStr acdPromptRegexp(AcdPAcd thys)
{
    static ajint count = 0;
    AjPStr type = NULL;

    const AjPStr knowntype;

    acdAttrToStr(thys, "type", "", &type);

    knowntype = acdKnowntypeDesc(thys);

    if(ajStrGetLen(knowntype))
    {
	count++;
	acdPromptStandardS(thys, knowntype);

	if(!ajStrSuffixC(knowntype, " regular expression"))
	   acdPromptStandardAppend(thys, " regular expression");
    }
    else
    {
	if(ajStrPrefixCaseC(type, "p"))
	    acdPromptStandard(thys,
			      "protein regular expression string or @file",
			      &count);
	else if(ajStrPrefixCaseC(type, "n"))
	    acdPromptStandard(thys,
			      "nucleotide regular expression string or @file",
			      &count);
	else
	    acdPromptStandard(thys,
			      "regular expression string or @file",
			      &count);
    }

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    return thys->StdPrompt;
}




/* @funcstatic acdHelpTextResource ********************************************
**
** Sets the help text for this ACD object to be a data resource description
**
** @param [r] thys [const AcdPAcd] Current ACD object.
** @param [w] Pstr [AjPStr*] Help text
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void acdHelpTextResource(const AcdPAcd thys, AjPStr* Pstr)
{
    ajint maxreads = 0;

    ajStrAssignClear(Pstr);

    acdAttrToInt(thys, "maxreads", 1, &maxreads);

    if(maxreads <= 1)
        ajStrAssignC(Pstr, "data resource");
    else
        ajStrAssignC(Pstr, "data resource(s)");

    ajStrFmtTitle(Pstr);

    ajStrAppendC(Pstr,
		 " filename and optional format, or reference (input query)");

    return;
}




/* @funcstatic acdPromptResource **********************************************
**
** Sets the default prompt for this ACD object to be a data resource
** prompt with "first", "second" etc. added.
**
** @param [u] thys [AcdPAcd] Current ACD object.
** @return [const AjPStr] Generated standard prompt
**
** @release 6.4.0
** @@
******************************************************************************/

static const AjPStr acdPromptResource(AcdPAcd thys)
{
    static ajint count=0;
    AjPStr resourcePrompt = NULL;
    AjPStr resourcePromptAlt = NULL;
    const AjPStr knowntype;
    ajint maxreads = 0;

    acdAttrToInt(thys, "maxreads", 1, &maxreads);

    knowntype = acdKnowntypeDesc(thys);

    resourcePrompt = ajStrNewRes(32);
    resourcePromptAlt = ajStrNewRes(32);

    if(ajStrGetLen(knowntype))
	ajFmtPrintAppS(&resourcePromptAlt, "%S ", knowntype);

    ajFmtPrintAppS(&resourcePromptAlt, "data resource");

    if(maxreads > 1)
	ajStrAppendC(&resourcePromptAlt, "(s)");

    ajFmtPrintS(&resourcePrompt, "Input %S", resourcePromptAlt);

    if(knowntype)
    {
	count++;
	acdPromptStandardS(thys, resourcePromptAlt);
    }
    else
	acdPromptStandardAlt(thys, ajStrGetPtr(resourcePrompt),
			     ajStrGetPtr(resourcePromptAlt), &count);

    if(!acdAttrTestDefined(thys, "default") &&
       acdAttrTestDefined(thys, "nullok"))
	acdPromptStandardAppend(thys, " (optional)");
    
    ajStrDel(&resourcePrompt);
    ajStrDel(&resourcePromptAlt);

    return thys->StdPrompt;
}




/* @funcstatic acdCodeGet *****************************************************
**
** Translates a code into a message text using the code table
** for the current language.
**
** @param [r] code [const AjPStr] Code name
** @param [w] msg [AjPStr*] Message text for this code in current language
** @return [AjBool] ajTrue on success
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdCodeGet(const AjPStr code, AjPStr *msg)
{
    const AjPStr value;	       /* not static - copy of a table text */

    AjBool ret = ajFalse;

    acdLog("acdCodeGet ('%S')\n", code);

    if(!acdCodeSet)
	acdCodeInit();

    value = ajTableFetchS(acdCodeTable, code);

    if(value)
    {
	ajStrAssignS(msg, value);
	acdLog("%S value '%S'\n", code, *msg);
	ret = ajTrue;
    }

    return ret;
}




/* @funcstatic acdCodeDef *****************************************************
**
** Generates a default code name of 'def' + qualifier type.
** Translates into a message text using the code table
** for the current language.
**
** @param [r] thys [const AcdPAcd] Current ACD object
** @param [w] msg [AjPStr*] Message text for default message
**                          in current language
** @return [AjBool] ajTrue on success
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdCodeDef(const AcdPAcd thys, AjPStr *msg)
{
    AjPStr code  = NULL;
    AjPStr value = NULL;

    AjBool ret = ajFalse;

    acdLog("acdCodeDef '%s'\n", acdType[thys->Type].Name);

    if(!acdCodeSet)
	acdCodeInit();

    code = ajStrNewC("def");
    ajStrAppendC(&code, acdType[thys->Type].Name);
    ajStrFmtLower(&code);
    acdLog("look for defcode '%S'\n", code);

    if(acdCodeGet(code, &value))
    {
	ajFmtPrintS(msg, "-%S : %S",
		    thys->Name, value);
	ajStrDel(&value);
	ret = ajTrue;
    }
    else
	acdLog("defcode not found '%S'\n", code);

    ajStrDel(&code);

    return ret;
}




/* @funcstatic acdHelpCodeDef *************************************************
**
** Generates a default code name of 'help' + qualifier type.
** Translates into a message text using the code table
** for the current language.
**
** @param [r] thys [const AcdPAcd] Current ACD object
** @param [w] msg [AjPStr*] Help text in current language
** @return [AjBool] ajTrue on success
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdHelpCodeDef(const AcdPAcd thys, AjPStr *msg)
{
    AjPStr code         = NULL;
    AjPStr value = NULL;

    AjBool ret = ajFalse;

    acdLog("acdHelpCodeDef '%s'\n", acdType[thys->Type].Name);

    if(!acdCodeSet)
	acdCodeInit();

    code = ajStrNewC("help");
    ajStrAppendC(&code, acdType[thys->Type].Name);
    ajStrFmtLower(&code);
    acdLog("look for helpcode '%S'\n", code);

    if(acdCodeGet(code, &value))
    {
	ajFmtPrintS(msg, "%S", value);
	ajStrDel(&value);
	ret = ajTrue;
    }
    else
	acdLog("helpcode not found '%S'\n", code);

    ajStrDel(&code);

    return ret;
}




/* @funcstatic acdCodeInit ****************************************************
**
** Sets up the code file data for the current language when needed
**
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdCodeInit(void)
{
    AjPFile codeFile           = NULL;
    AjPStr codeFName    = NULL;
    AjPStr codeRoot     = NULL;
    AjPStr codeRootInst = NULL;
    AjPStr codePack     = NULL;
    AjPStr codeCode     = NULL;
    AjPStr codeValue    = NULL;
    AjPStr codeLine     = NULL;
    AjPStr codeText     = NULL;
    AjPStr codeLanguage = NULL;
    AjPStr tmpstr = NULL;
    AjPRegexp codexp = NULL;
    
    if(acdCodeSet)
	return;
    
    ajStrAssignS(&codePack, ajNamValuePackage());
    ajStrAssignS(&codeRootInst,ajNamValueInstalldir());
    ajDirnameFix(&codeRootInst);
    
    if(!ajNamGetValueC("language", &codeLanguage))
	ajStrAssignC(&codeLanguage, "english");
    
    if(ajNamGetValueC("acdroot", &codeRoot))
    {
	ajDirnameFix(&codeRoot);
	ajFmtPrintS(&codeFName, "%Scodes.%S", codeRoot, codeLanguage);
	codeFile = ajFileNewInNameS(codeFName);
	acdLog("Code file in acdroot: '%S'\n", codeFName);
    }
    else
    {
	ajFmtPrintS(&codeFName, "%Sshare/%S/acd/codes.%S",
		    codeRootInst, codePack, codeLanguage);
	acdLog("Code file installed: '%S'\n", codeFName);
	codeFile = ajFileNewInNameS(codeFName);

	if(!codeFile)
	{
	    acdLog("Code file '%S' not opened\n", codeFName);
	    ajStrAssignS(&codeRoot, ajNamValueRootdir());
	    ajDirnameFix(&codeRoot);
	    ajFmtPrintS(&codeFName, "%Sacd/codes.%S", codeRoot, codeLanguage);
	    acdLog("Code file from source dir: '%S'\n", codeFName);
	    codeFile = ajFileNewInNameS(codeFName);
	}
    }
    
    if(!codeFile)			/* test acdc-codemissing */
	ajWarn("Code file %S not found", codeFName);
    else
	acdLog("Code file %F used\n", codeFile);
    
    codeText = ajStrNew();
    
    /* fix by Nicolas Joly <njoly@pasteur.fr> */
    
    while(codeFile && ajReadlineTrim(codeFile, &codeLine))
	if(ajStrCutComments(&codeLine))
	{
	    ajStrAppendS(&codeText, codeLine);
	    ajStrAppendC(&codeText, " ");
	}

    ajFileClose(&codeFile);
    
    ajStrDel(&codeLine);
    
    acdCodeTable = ajTablestrNewCase(100);
    
    codexp = ajRegCompC("^ *([^ ]+) +\"([^\"]*)\"");

    while(ajRegExec(codexp, codeText))
    {
	codeCode = codeValue = NULL; /* need to save in table each time */
	ajRegSubI(codexp, 1, &codeCode);
	ajRegSubI(codexp, 2, &codeValue);
	ajStrFmtLower(&codeCode);
	ajTablePut(acdCodeTable, codeCode, codeValue);
	acdLog("add to table %S '%S'\n", codeCode, codeValue);
	ajRegPost(codexp, &tmpstr);
        ajStrAssignS(&codeText, tmpstr);
    }

    if(!ajStrIsWhite(codeText))		/* test acdc-codebad */
	ajDie("Bad format in codes file %S after '%S \"%S\"'",
	      codeFName, codeCode, codeValue);

    codeCode = codeValue = NULL; /* saved in the table */
    
    ajRegFree(&codexp);
    ajStrDel(&codeText);
    ajStrDel(&codeFName);
    ajStrDel(&codeRoot);
    ajStrDel(&codeRootInst);
    ajStrDel(&codePack);
    ajStrDel(&codeLanguage);
    ajStrDel(&tmpstr);

    acdCodeSet = ajTrue;

    return;
}




/* @funcstatic acdKnowntypeInit ***********************************************
**
** Sets up the known type data
**
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

static void acdKnowntypeInit(void)
{
    if(acdKnowntypeSet)
        return;

    acdReadKnowntype(&acdKnowntypeDescTable, &acdKnowntypeTypeTable);
    acdKnowntypeSet = ajTrue;

    return;
}




/* @funcstatic acdSetQualAppl *************************************************
**
** Sets internal variables for the application booleans -debug etc.
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [r] val [AjBool] Value
** @return [AjBool] ajTrue if this was an application-wide variable.
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdSetQualAppl(const AcdPAcd thys, AjBool val)
{
    ajint i = 0;
    AjBool setval;
    AjPStr setstr = NULL;
    static AjPStr valstr = NULL;
    static AjPStr bufstr = NULL;
    
    acdLog("acdSetQualAppl '%S'\n", thys->Name);
    
    for(i=0; acdQualAppl[i].Name; i++)
    {
	if(ajStrMatchC(thys->Name, acdQualAppl[i].Name))
	{
	    if(thys->Defined)	 /* User put it on the command line */
	    {
		setval = val;
		acdLog("Appl qualifier defined %S = %b\n",
		       thys->Name, setval);
	    }
	    else			/* look for a variable */
	    {
		ajFmtPrintS(&setstr, "%S", thys->Name);

		if(ajNamGetValueS(setstr, &valstr))
		{
		    ajStrToBool(valstr, &setval);
		    acdLog("Appl qualifier variable %S = %b\n",
			   setstr, setval);
		}
		else	    /* nothing found, use the default value */
		    setval = val;
		ajStrDel(&setstr);
	    }

	    switch(i)	   /* see acdQualAppl for the correct order */
	    {
                case 0:
                    acdAuto = setval;
                    break;
                case 1:
                    acdStdout = setval;
                    break;
                case 2:
                    acdFilter = setval;

                    if(acdFilter)
                    {
                        acdAuto   = ajTrue;
                        acdStdout = ajTrue;
                    }

                    break;
                case 3:
                    acdOptions = setval;
                    break;
                case 4:
                    acdDebug = setval;
                    /* acdLog("acdSetQualAppl acdDebug %B\n", acdDebug); */
                    acdDebugSet = ajTrue;
                    if(ajNamGetValueC("debugbuffer", &bufstr))
                    {
                        ajStrToBool(bufstr, &acdDebugBuffer);
                    }
                    break;
				
                case 5:
                    acdVerbose = setval;
                    break;
                case 6:
                    acdDoHelp = setval;
                    break;
                case 7:
                    AjErrorLevel.warning = setval;
                    break;
                case 8:
                    AjErrorLevel.error = setval;
                    break;
                case 9:
                    AjErrorLevel.fatal = setval;
                    break;
                case 10:
                    AjErrorLevel.die = setval;
                    break;
	    }

	    return ajTrue;
	}
    }
    
    return ajFalse;
}




/* @funcstatic acdSelectPrompt ************************************************
**
** Present the options as a simple numbered list
**
** @param [r] thys [const AcdPAcd] ACD object
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdSelectPrompt(const AcdPAcd thys)
{
    const AjPStr hdrstr;
    AjPStr delim = NULL;
    AjPStr value = NULL;
    AjPStrTok handle;
    AjPStr line = NULL;
    static const char* white = " \t\n\r";
    ajint i = 0;
    
    if(acdAuto)
        return;
    
    hdrstr = acdAttrValue(thys, "header");

    if(ajStrGetLen(hdrstr))
	ajUserDumpS(hdrstr);
    
    ajStrAssignS(&delim,acdAttrValue(thys, "delimiter"));

    if(!ajStrGetLen(delim))
	ajStrAssignC(&delim, ";");

    ajStrAssignS(&value, acdAttrValue(thys, "values"));
    acdVarResolve(&value);

    if(!ajStrGetLen(value))
	if(!acdKnownValueSelect(thys, &value))
	    acdError("No value defined for selection");

    handle = ajStrTokenNewS(value, delim);

    while(ajStrTokenNextFind(handle, &line))
    {
	ajStrTrimC(&line, white);
	ajUser("  %5d : %S", ++i, line);
    }
    
    ajStrTokenDel(&handle);
    ajStrDel(&line);
    ajStrDel(&value);
    ajStrDel(&delim);

    return;
}




/* @funcstatic acdListPrompt **************************************************
**
** Present the options as a list with option codes selectable by the user
**
** @param [r] thys [const AcdPAcd] ACD object
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdListPrompt(const AcdPAcd thys)
{
    const AjPStr hdrstr;
    AjPStr codedelim = NULL;
    AjPStr delim     = NULL;

    AjPStr value     = NULL;
    AjPStrTok handle;
    AjPStrTok codehandle;

    AjPStr line = NULL;
    AjPStr code = NULL;
    AjPStr desc = NULL;
    ajuint margin = 8;

    static const char* white = " \t\n\r";

    if(acdAuto)
	return;

    hdrstr = acdAttrValue(thys, "header");

    if(ajStrGetLen(hdrstr))
	ajUserDumpS(hdrstr);

    ajStrAssignS(&delim,acdAttrValue(thys, "delimiter"));

    if(!ajStrGetLen(delim))
    {
	ajStrAssignC(&delim, ";");
    }

    ajStrAssignS(&codedelim,acdAttrValue(thys, "codedelimiter"));

    if(!ajStrGetLen(codedelim))
    {
	ajStrAssignC(&codedelim, ":");
    }

    ajStrAssignS(&value, acdAttrValue(thys, "values"));
    acdVarResolve(&value);

    if(!ajStrGetLen(value))
	if(!acdKnownValueList(thys, &value))
	    acdError("No value defined for list");

    handle = ajStrTokenNewS(value, delim);

    while(ajStrTokenNextFind(handle, &line))
    {
	codehandle = ajStrTokenNewS(line, codedelim);
	ajStrTokenNextParse(codehandle, &code);
	ajStrTrimC(&code, white);

        if(ajStrGetLen(code) > margin)
            margin = ajStrGetLen(code);

	ajStrTokenDel(&codehandle);
    }

    ajStrTokenAssignS(&handle, value, delim);

    while(ajStrTokenNextFind(handle, &line))
    {
	codehandle = ajStrTokenNewS(line, codedelim);
	ajStrTokenNextParse(codehandle, &code);
	ajStrTokenNextParseS(codehandle, delim, &desc);
	ajStrTrimC(&code, white);
	ajStrTrimC(&desc, white);
	ajUser("  %*S : %S", margin, code, desc);
	ajStrTokenDel(&codehandle);
    }

    ajStrTokenDel(&handle);
    ajStrDel(&line);
    ajStrDel(&code);
    ajStrDel(&desc);

    ajStrDel(&delim);

    ajStrDel(&codedelim);

    ajStrDel(&value);
    return;
}




/* @funcstatic acdListValue ***************************************************
**
** Checks the user setting against the menu list of codes and descriptions.
**
** An unambiguous match to the codes counts as valid.
** If this fails, an unambiguous match to the descriptions counts.
**
** @param [r] thys [const AcdPAcd] ACD Object
** @param [r] min [ajint] Minimum number of values required
** @param [r] max [ajint] Maximum number of values required
** @param [r] reply [const AjPStr] Default value
** @return [AjPStr*] Array of accepted matches, ending with a NULL.
**
** @release 1.0.0
** @@
******************************************************************************/

static AjPStr* acdListValue(const AcdPAcd thys, ajint min, ajint max,
			    const AjPStr reply)
{
    AjPStr* val = NULL;
    
    AjPStr codedelim = NULL;
    AjPStr delim     = NULL;
    AjPStr value  = NULL;
    AjBool exactcase;
    AjPStrTok handle = NULL;
    AjPStrTok rephandle = NULL;
    AjPStrTok codehandle = NULL;

    AjPStr line      = NULL;
    AjPStr code      = NULL;
    AjPStr desc      = NULL;
    AjPList list            = NULL;
    AjPStr repstr    = NULL;
    AjPStr hitstr    = NULL;
    AjPStr validstr  = NULL;
    AjPStr hitstr1          = NULL;
    AjPStr hitstr2          = NULL;
    AjPStr ambigList = NULL;
    AjPStr repdelim  = NULL;
    static const char* white = " \t\n\r";
    ajint k = 0;
    
    ajint ifound = 0;
    ajint jfound = 0;
    ajint ilen;
    ajint itoken = 0;
    
    AjBool ok = ajTrue;
    
    list = ajListstrNew();
    
    acdAttrToBool(thys, "casesensitive", ajFalse, &exactcase);
    
    acdAttrValueStr(thys, "delimiter", ";", &delim);
    acdAttrValueStr(thys, "codedelimiter", ":", &codedelim);
    
    if(!repdelim)
    {
	repdelim = ajStrNewRes(10);
	ajStrAssignC(&repdelim, ",");
    }
    
    ajStrAssignS(&value,acdAttrValue(thys, "values"));
    acdVarResolve(&value);

    if(!ajStrGetLen(value))
	if(!acdKnownValueList(thys, &value))
	    acdError("No value defined for list");
    
    ambigList = ajStrNew();
    ajStrAssignClear(&validstr);
    rephandle = ajStrTokenNewS(reply, repdelim);
    while(ajStrTokenNextParse(rephandle, &repstr))
    {
	itoken++;
	acdLog("testing '%S'\n", repstr);
	ajStrTokenAssignS(&handle,value, delim);
	ifound = jfound = 0;
	ajStrAssignClear(&validstr);

        if(ajStrMatchC(repstr, "*"))
        {
            while(ajStrTokenNextFind(handle, &line))
            {
                ajStrTokenAssignS(&codehandle, line, codedelim);
                ajStrTokenNextParse(codehandle, &code);
                ajStrTrimC(&code, white);

                ajStrAssignS(&hitstr, code);
                ajListstrPushAppend(list, hitstr);
                hitstr = NULL;
                ajStrTokenReset(codehandle);
            }
        }
        else
        {
            while(ajStrTokenNextFind(handle, &line))
            {
                ajStrTokenAssignS(&codehandle, line, codedelim);
                ajStrTokenNextParse(codehandle, &code);
                ajStrTokenNextParseS(codehandle, delim, &desc);
                ajStrTrimC(&code, white);
                ajStrTrimC(&desc, white);
 
                if(ajStrGetLen(validstr))
                    ajStrAppendK(&validstr, ',');

                ajStrAppendS(&validstr, code);

                if(ajStrMatchS(code, repstr) ||
                   (!exactcase && ajStrMatchCaseS(code, repstr)))
                {
                    ifound = 1;
                    ajStrAssignS(&hitstr1, code);
                    break;
                }

                if(ajStrMatchS(desc, repstr) ||
                   (!exactcase && ajStrMatchCaseS(desc, repstr)))
                {
                    jfound = 1;
                    ajStrAssignS(&hitstr2, code);
                    break;
                }

                if(ajStrPrefixS(code, repstr) ||
                   (!exactcase && ajStrPrefixCaseS(code, repstr)))
                {
                    ifound++;
                    ajStrAssignS(&hitstr1, code);
                    acdAmbigApp(&ambigList, code);
                }

                if(ajStrPrefixS(desc, repstr) ||
                   (!exactcase && ajStrPrefixCaseS(desc, repstr)))
                {
                    jfound++;
                    ajStrAssignS(&hitstr2, code);
                    acdAmbigApp(&ambigList, desc);
                }

                ajStrTokenReset(codehandle);
            } /* end of while */
	
            if(ifound == 1)
            {
                ajStrAssignS(&hitstr,hitstr1);
                ajListstrPushAppend(list, hitstr);
                hitstr = NULL;
            }
            else if(jfound == 1)
            {
                ajStrAssignS(&hitstr, hitstr2);
                ajListstrPushAppend(list, hitstr);
                hitstr = NULL;
            }
            else
            {
                if(ifound || jfound) /* test acdc-listambig1 acdc-listambig2 */
                    ajErr("'%S' is ambiguous (%S)", repstr, ambigList);
                else			/* test acdc-listbad */
                    ajErr("'%S' is not a valid menu option\n"
                          "Accepted short codes are: %S",
                          repstr, validstr);

                ok = ajFalse;
                break;
            }
        }
    }

    ajStrTokenDel(&handle);
    ajStrTokenDel(&rephandle);
    ajStrTokenDel(&codehandle);
    ajStrDel(&repstr);
    
    ilen = (ajuint) ajListstrGetLength(list);
    acdLog("Found %d matches OK: %b min: %d max: %d\n",
	   ilen, ok, min, max);
    if(ok)
    {
	if(ilen < min)
	{				/* test acdc-listmin */
	    if(min <= 1)
		ajErr("Menu needs %d value", min);
	    else
		ajErr("Menu needs %d values", min);
	    ok = ajFalse;
	}

	if(ilen > max)			/* test acdc-listmax */
	{
	    if(max <= 1)
		ajErr("Menu allows no more than %d value", max);
	    else
		ajErr("Menu allows no more than %d values", max);
	    ok = ajFalse;
	}
    }
    
    if(ok)
    {
	AJCNEW0(val, ilen+1);
	for(k = 0; k < ilen; k++)
	{
	    ajListstrPop(list, &val[k]);
	    acdLog("Accept[%d]: '%S'\n", k,val[k]);
	}
    }
    
    acdLog("Found %d matches\n", ilen);
    acdLog("Menu length now %d\n", ajListstrGetLength(list));

    if(ok)
	acdLog("Before return val[0] '%S'\n", val[0]);
    
    ajListstrFreeData(&list);
    ajStrDel(&delim);
    ajStrDel(&codedelim);
    ajStrDel(&repdelim);
    ajStrDel(&line);
    ajStrDel(&code);
    ajStrDel(&desc);

    ajStrDel(&ambigList);
    ajStrDel(&hitstr1);
    ajStrDel(&hitstr2);
    
    ajStrDel(&repstr);
    
    ajStrDel(&validstr);
    ajStrDel(&value);

    if(!ok)
	return NULL;
    
    return val;
}




/* @funcstatic acdSelectValue *************************************************
**
** Checks the user setting against the selection list set of codes
**
** An unambiguous match to the codes counts as valid.
** If this fails, an unambiguous match to the descriptions counts.
**
** @param [r] thys [const AcdPAcd] ACD Object
** @param [r] min [ajint] Minimum number of values required
** @param [r] max [ajint] Maximum number of values required
** @param [r] reply [const AjPStr] Default value
** @return [AjPStr*] Array of accepted matches, ending with a NULL.
**
** @release 1.0.0
** @@
******************************************************************************/

static AjPStr* acdSelectValue(const AcdPAcd thys, ajint min, ajint max,
			      const AjPStr reply)
{
    AjPStr *val = NULL;
    
    AjPStr delim=NULL;
    const AjPStr tmpstr = NULL;
    AjPStr value = NULL;
    AjBool exactcase;
    AjPStrTok handle = NULL;
    AjPStrTok rephandle = NULL;
    AjPStr line      = NULL;
    AjPStr code      = NULL;
    AjPStr desc      = NULL;
    AjPList list            = NULL;
    AjPStr repstr    = NULL;
    AjPStr hitstr    = NULL;
    AjPStr validstr  = NULL;
    AjPStr hitstr2 = NULL;
    AjPStr ambigList = NULL;
    AjPStr repdelim  = NULL;
    static const char* white = " \t\n\r";
    ajint i = 0;
    ajint k = 0;
    
    ajint jfound = 0;
    ajint icnt   = 0;
    ajint ilen;
    ajint itoken = 0;
    
    AjBool ok = ajTrue;

    list = ajListstrNew();
    
    acdAttrToBool(thys, "casesensitive", ajFalse, &exactcase);
    
    tmpstr = acdAttrValue(thys, "delimiter");

    if(ajStrGetLen(tmpstr))
	ajStrAssignS(&delim, tmpstr);
    else
	ajStrAssignC(&delim, ";");
    
    if(!repdelim)
    {
	repdelim = ajStrNewRes(10);
	ajStrAssignC(&repdelim, ",");
    }
    
    ajStrAssignS(&value, acdAttrValue(thys, "values"));
    acdVarResolve(&value);

    if(!ajStrGetLen(value))
	if(!acdKnownValueSelect(thys, &value))
	    acdError("No value defined for selection");
    
    ajStrAssignClear(&ambigList);
    ajStrAssignClear(&validstr);
    rephandle = ajStrTokenNewS(reply, repdelim);
    while(ajStrTokenNextParse(rephandle, &repstr))
    {
	itoken++;
	
	acdLog("testing '%S'\n", repstr);
	ajStrTokenAssignS(&handle, value, delim);
	i = jfound = 0;

        if(ajStrMatchC(repstr, "*"))
        {
            for(icnt = 1; ajStrTokenNextFind(handle, &desc); icnt++)
            {
                ajStrAssignS(&hitstr, desc);
                ajListstrPushAppend(list, hitstr);
                hitstr = NULL;
            }
        }
        else
        {
            for(icnt = 1; ajStrTokenNextFind(handle, &desc); icnt++)
            {
                ajStrTrimC(&desc, white);
 
                if(itoken == 1)
                {
                    if(ajStrGetLen(validstr))
                        ajStrAppendK(&validstr, ',');
                    ajStrAppendS(&validstr, desc);
                }

                if(ajStrMatchS(desc, repstr) ||
                   (!exactcase && ajStrMatchCaseS(desc, repstr)))
                {
                    jfound = 1;
                    ajStrAssignS(&hitstr2, desc);
                    break;
                }

                if(ajStrPrefixS(desc, repstr) ||
                   (!exactcase && ajStrPrefixCaseS(desc,repstr)))
                {
                    jfound++;
                    ajStrAssignS(&hitstr2, desc);
                    acdAmbigApp(&ambigList, desc);
                }

                if(ajStrToInt(repstr, &i) && i == icnt)
                {
                    jfound++;
                    ajStrAssignS(&hitstr2, desc);
                    acdAmbigApp(&ambigList, repstr);
                }
            } /* end of while */
	
            if(jfound == 1)
            {
                ajStrAssignS(&hitstr, hitstr2);
                ajListstrPushAppend(list, hitstr);
                hitstr = NULL;
            }
            else
            {
                if(jfound)			/* test acdc-selectambig */
                    ajErr("'%S' is ambiguous (%S)", repstr, ambigList);
                else			/* test acdc-selectbad */
                    ajErr("'%S' is not a valid selection list option\n"
                          "Accepted values are: %S",
                          repstr, validstr);
                ok = ajFalse;
                break;
            }
        }
    }

    ajStrTokenDel(&handle);
    ajStrTokenDel(&rephandle);
    ajStrDel(&repstr);
    
    ilen = (ajuint) ajListstrGetLength(list);
    
    if(ok)
    {
	if(ilen < min)			/* test acdc-selectmin */
	{
	    if(min <= 1)
		ajErr("Selection list needs %d value", min);
	    else
		ajErr("Selection list needs %d values", min);
	    ok = ajFalse;
	}
	if(ilen > max)			/* test acdc-selectmax */
	{
	    if(max <= 1)
		ajErr("Selection list allows no more than %d value", max);
	    else
		ajErr("Selection list allows no more than %d values", max);
	    ok = ajFalse;
	}
    }
    
    if(ok)
    {
	AJCNEW0(val, ilen+1);

	for(k = 0; k < ilen; k++)
	    ajListstrPop(list, &val[k]);
    }
    
    acdLog("Found %d matches\n", ilen);
    
    ajListstrFreeData(&list);
    ajStrDel(&value);
    ajStrDel(&line);
    ajStrDel(&code);
    ajStrDel(&desc);
    ajStrDel(&ambigList);
    ajStrDel(&hitstr2);
    ajStrDel(&delim);
    ajStrDel(&repdelim);
    ajStrDel(&validstr);

    if(!ok)
	return NULL;
    
    return val;
}




/* @funcstatic acdAmbigApp ****************************************************
**
** Appends a token to a list, with commas as delimiters. Used to
** build a list of ambiguous matches for messages.
**
** @param [w] pambigList [AjPStr*] List of tokens with ',' delimiter
** @param [r] str [const AjPStr] Latest token to add
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdAmbigApp(AjPStr* pambigList, const AjPStr str)
{
    if(ajStrGetLen(*pambigList))
	ajStrAppendC(pambigList, ",");
    ajStrAppendS(pambigList, str);

    return;
}




/* @funcstatic acdAmbigAppC ***************************************************
**
** Appends a token to a list, with commas as delimiters. Used to
** build a list of ambiguous matches for messages.
**
** @param [w] pambigList [AjPStr*] List of tokens with ',' delimiter
** @param [r] txt [const char*] Latest token to add
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdAmbigAppC(AjPStr* pambigList, const char* txt)
{
    if(ajStrGetLen(*pambigList))
	ajStrAppendC(pambigList, ",");

    ajStrAppendC(pambigList, txt);

    return;
}




/* @funcstatic acdDataFilename ************************************************
**
** Sets a default data file name. If no values are provided, it will be
** programname.dat
**
** @param [w] infname [AjPStr*] Resulting file name
** @param [r] name [const AjPStr] File name
** @param [r] ext [const AjPStr] File extension
** @param [r] nullok [AjBool] Can set as an empty string if true
** @return [AjBool] ajTrue if a name was successfully set
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdDataFilename(AjPStr* infname,
			      const AjPStr name, const AjPStr ext,
			      AjBool nullok)
{
    AjBool ret = ajTrue;

    if(ajStrGetLen(name))
	ajStrAssignS(infname, name);
    else if(!nullok)
	ajStrAssignS(infname, acdProgram);
    else
	ajStrAssignClear(infname);

    if(ajStrGetLen(ext))
	ajFilenameReplaceExtS(infname, ext);
    else
	if (!nullok)
	    ajFilenameReplaceExtC(infname, "dat");

    return ret;
}




/* @funcstatic acdInFilename **************************************************
**
** Sets a default input file name. If filtering is on, this will be stdin.
** Otherwise it is blank.
**
** @param [w] infname [AjPStr*] Input file name
** @return [AjBool] ajTrue if a name was successfully set
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdInFilename(AjPStr* infname)
{
    AjBool ret = ajFalse;

    if(!acdInFile && acdFilter)
    {
	ajStrAssignC(infname, "stdin");
	ret = ajTrue;
    }
    else
	ajStrAssignClear(infname);

    acdInFile++;

    return ret;
}




/* @funcstatic acdOutDirectory ************************************************
**
** Sets a default output file directory. Uses the _OUTDIRECTORY variable
** as a default value, but any input string overrides it.
**
** The recommendation is that the directory should always be provided
** in the emboss.defaults file or by an environment variable.
**
** As for all associated qualifiers, it is also possible to set a value
** in the ACD file.
**
** @param [w] dir [AjPStr*] Specified directory
** @return [AjBool] ajTrue if a directory was successfully set
**
** @release 2.7.0
** @@
******************************************************************************/

static AjBool acdOutDirectory(AjPStr* dir)
{
    AjBool ret = ajFalse;

    AjPStr mydir  = NULL;

    acdLog("acdOutDirectory ('%S')\n",
	   *dir);

    if(!acdDirectoryDef)
	if(!ajNamGetValueC("outdirectory", &acdDirectoryDef))
	    ajStrAssignClear(&acdDirectoryDef);

    if(dir && ajStrGetLen(*dir))
	ajStrAssignS(&mydir, *dir);
    else
	ajStrAssignS(&mydir, acdDirectoryDef);

    if(ajStrGetLen(mydir))
    {
	ajDirnameFix(&mydir);
	ajStrAssignS(dir, mydir);
	ret = ajTrue;
    }
    else
    {
	ajStrAssignClear(dir);
	ret = ajFalse;
    }

    acdLog(". . . dir '%S' ret: %B\n",
	   *dir, ret);

    ajStrDel(&mydir);

    return ret;
}




/* @funcstatic acdOutFilename *************************************************
**
** Sets a default output file name. If stdout or filtering are on,
** this will be stdout for the first output file.
** Otherwise it is built from the defaults provided.
**
** The base file name is usually specified in the ACD file as name:
** and passed in by the calling acdSet function. The default will
** be the base file name saved from the first input file, or "outfile"
**
** The extension is usually specified in the ACD file as extension:
** and passed in by the calling acdSet function. The default will
** be the program name for the first file, and "out2", "out3" and so
** on for later files.
**
** The recommendation is that the extension should always be provided
** in the ACD file, but that the base file name should be taken from
** the input file in most cases.
**
** @param [w] outfname [AjPStr*] Input file name
** @param [r] name [const AjPStr] Specified base file name
** @param [r] ext [const AjPStr] Specified extension
** @return [AjBool] ajTrue if a name was successfully set
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdOutFilename(AjPStr* outfname,
			     const AjPStr name, const AjPStr ext)
{
    AjBool ret = ajFalse;

    acdLog("acdOutFilename ('%S', '%S', '%S') acdStdout: %B\n",
	   *outfname, name, ext, acdStdout);

    if(!acdOutFile && acdStdout) /* first outfile, running as a filter */
    {
	ajStrAssignC(outfname, "stdout");
	acdLog("outfile <first> '%S'\n", *outfname);
	acdOutFile++;
	return ajTrue;
    }

    ajStrAssignEmptyS(&acdOutFName, name);		/* use name if given */
    ajStrFmtWord(&acdOutFName);
    ajStrAssignEmptyS(&acdOutFName, acdInFName); /* else use saved name */
    ajStrAssignEmptyC(&acdOutFName, "outfile"); /* else, use "outfile" */

    ajStrAssignEmptyS(&acdOutFExt, ext); /* use extension if given */

    if(!acdOutFile)
	ajStrAssignEmptyS(&acdOutFExt, acdProgram);

    /* else try program name for first file */
    if(!ajStrGetLen(acdOutFExt))	/* if all else fails, use out2 etc. */
	ajFmtPrintS(&acdOutFExt, "out%d", acdOutFile+1);

    acdLog(". . . acdOutFName '%S', acdOutFExt '%S'\n",
	   acdOutFName, acdOutFExt);

    if(ext && ajStrGetLen(acdOutFExt))	 /* NULL ext means add no extension */
	ajFmtPrintS(outfname, "%S.%S", acdOutFName, acdOutFExt);
    else
	ajStrAppendS(outfname, acdOutFName);

    acdOutFile++;

    acdLog("outfile %d %S.%S\n", acdOutFile, acdOutFName, acdOutFExt);

    ajStrDelStatic(&acdOutFName);
    ajStrDelStatic(&acdOutFExt);

    return ret;
}




/* @funcstatic acdInFileSave **************************************************
**
** For the first call, saves the input filename for use in building output
** file name(s).
**
** @param [r] infname [const AjPStr] Input file name
** @param [r] objname [const AjPStr] Input object name (or NULL)
** @param [r] reset [AjBool] Reset the saved name if this is the first time
**                           a true value has been passed.
** @return [AjBool] ajTrue if a name was successfully set
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdInFileSave(const AjPStr infname, const AjPStr objname,
                            AjBool reset)
{
    static AjBool usefile = AJFALSE;
    static ajint called = 0;

    AjBool useobj = ajTrue;

    if(!called)
    {
        if(ajNamGetValueC("acdfilename", &acdTmpStr))
            ajStrToBool(acdTmpStr, &usefile);
        called = 1;
    }

    if(acdInFileSet)			/* already have a name */
	return ajFalse;

    if(!reset && ajStrGetLen(acdInFName)) /* have a name, no reset forced */
	return ajFalse;

    if(usefile)
    {
        useobj = ajFalse;
        if(ajStrMatchC(infname, "stdin")) 
            useobj = ajTrue;
    }
    
    acdLog("acdInFileSave (%S,%S) reset: %B usefile: %B, saved name '%S'\n",
	   infname, objname, reset, usefile, acdInFName);

    if(useobj && ajStrGetLen(objname))
    {
        if(!ajStrGetLen(objname))
            return ajFalse;

        ajStrAssignS(&acdInFName, objname);
        ajFilenameTrimAll(&acdInFName);
        ajStrFmtLower(&acdInFName);
    }
    else
    {
        if(!ajStrGetLen(infname))
            return ajFalse;

        ajStrAssignS(&acdInFName, infname);
        ajFilenameTrimAll(&acdInFName);

        if(useobj)
            ajStrFmtLower(&acdInFName);
    }
    
    if(reset)
	acdInFileSet = ajTrue;

    acdLog("acdInFileSave (%S, %S) input file set to '%S'\n",
	   infname, objname, acdInFName);

    return ajTrue;
}




/* @funcstatic acdInTypeSeqSave ***********************************************
**
** For the first call, saves the input filename for use in building output
** file name(s).
**
** @param [r] intype [const AjPStr] Input sequence type
** @return [AjBool] ajTrue if a type was successfully set
**
** @release 2.7.0
** @@
******************************************************************************/

static AjBool acdInTypeSeqSave(const AjPStr intype)
{
    if(acdInTypeSeqName)
	return ajFalse;

    acdLog("acdInTypeSeqSave (%S)\n",
	   intype);

    if(!ajStrGetLen(intype))
    {
	ajStrAssignClear(&acdInTypeSeqName);
	acdLog("Input sequence type defaults to ''\n", acdInTypeSeqName);
	acdInTypeFeatSave(NULL);
    }
    else
    {
	ajStrAssignS(&acdInTypeSeqName, intype);
	ajStrFmtLower(&acdInTypeSeqName);

	if(ajSeqTypeIsAny(intype))
	    acdInTypeFeatSaveC("");
	else if(ajSeqTypeIsProt(intype))
	    acdInTypeFeatSaveC("protein");
	else if(ajSeqTypeIsNuc(intype))
	    acdInTypeFeatSaveC("nucleotide");
	else
	    acdInTypeFeatSaveC("");
    }
 
    acdLog("acdInTypeSeqSave (%S) input type set to '%S'\n",
	   intype, acdInTypeSeqName);

    return ajTrue;
}




/* @funcstatic acdInTypeSeq ***************************************************
**
** Returns the input sequence type (if known)
**
** @param [w] typename [AjPStr*] Input sequence type
** @return [AjBool] ajTrue if a type was successfully set
**
** @release 2.7.0
** @@
******************************************************************************/

static AjBool acdInTypeSeq(AjPStr* typename)
{
    AjBool ret = ajFalse;

    acdLog("acdInTypeSeq saved acdInTypeSeqName '%S'\n", acdInTypeSeqName);

    if(acdInTypeSeqName)		/* could be an empty string */
    {
	ajStrAssignS(typename, acdInTypeSeqName);
	ret = ajTrue;
    }
    else
    {
	ajStrAssignClear(typename);  /* allow anything, return ajFalse */
	ret = ajFalse;
    }

    return ret;
}




/* @funcstatic acdInTypeFeatSave **********************************************
**
** Saves the input feature type for use in setting the default output type
**
** @param [r] intype [const AjPStr] Input feature type
** @return [AjBool] ajTrue if a type was successfully set
**
** @release 2.7.0
** @@
******************************************************************************/

static AjBool acdInTypeFeatSave(const AjPStr intype)
{
    if(acdInTypeFeatName)
	return ajFalse;

    acdLog("acdInTypeFeatSave (%S)\n",
	   intype);

    if(!ajStrGetLen(intype))
    {
	ajStrAssignClear(&acdInTypeFeatName);
	acdLog("Input feature type defaults to '%S'\n", acdInTypeFeatName);
    }
    else
	ajStrAssignS(&acdInTypeFeatName, intype);

    ajStrFmtLower(&acdInTypeFeatName);

    acdLog("acdInTypeFeatSave (%S) input feature type set to '%S'\n",
	   intype, acdInTypeFeatName);

    return ajTrue;
}




/* @funcstatic acdInTypeFeatSaveC *********************************************
**
** Saves the input feature type for use in setting the default output type
**
** @param [r] intype [const char*] Input feature type
** @return [AjBool] ajTrue if a type was successfully set
**
** @release 2.7.0
** @@
******************************************************************************/

static AjBool acdInTypeFeatSaveC(const char* intype)
{
    AjBool ret;
    AjPStr tmpstr = NULL;

    tmpstr = ajStrNewC(intype);
    ret = acdInTypeFeatSave(tmpstr);
    ajStrDel(&tmpstr);

    return ret;
}




/* @funcstatic acdInTypeFeat **************************************************
**
** Returns the input feature type (if known)
**
** @param [w] typename [AjPStr*] Input feature type
** @return [AjBool] ajTrue if a type was successfully set
**
** @release 2.7.0
** @@
******************************************************************************/

static AjBool acdInTypeFeat(AjPStr* typename)
{
    AjBool ret = ajFalse;

    acdLog("acdInTypeFeat saved acdInTypeFeatName '%S'\n", acdInTypeFeatName);

    if(acdInTypeFeatName)		/* could be an empty string */
    {
	ajStrAssignS(typename, acdInTypeFeatName);
	ret = ajTrue;
    }
    else
    {
	ajStrAssignClear(typename);  /* allow anything, return ajFalse */
	ret = ajFalse;
    }

    return ret;
}




/* @funcstatic acdLog *********************************************************
**
** Writes a message to the .acdlog file
**
** @param [r] fmt [const char*] Format with ajFmt extensions
** @param [v] [...] Optional arguments
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdLog(const char *fmt, ...)
{
    va_list args;

    if(!acdDoLog)
	return;

    if(!acdLogFName)
    {
	ajFmtPrintS(&acdLogFName, "%S.acdlog", acdProgram);
	acdLogFile = ajFileNewOutNameS(acdLogFName);
	ajFileSetUnbuffer(acdLogFile);
    }

    va_start(args, fmt) ;
    ajFmtVPrintF(acdLogFile, fmt, args);
    va_end(args) ;

    return;
}




/* @funcstatic acdPretty ******************************************************
**
** Writes a pretty formatted version of the .acd syntax
** message to the .acdpretty file
**
** @param [r] fmt [const char*] Format with ajFmt extensions
** @param [v] [...] Optional arguments
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void acdPretty(const char *fmt, ...)
{
    va_list args ;
    static AjPStr tmpstr = NULL;
    ajint *icword    = NULL;
    ajint *icpos    = NULL;
    AjPStr cmtstr = NULL;
    ajint lastword;
    ajint wordcount;
    ajuint ipos;
    ajint i = 0;
    ajint j = 0;
    ajint ccnt=0;

    if(!acdDoPretty)
	return;

    if(!acdPrettyFName)
    {
	if(acdStdout)
	    acdPrettyFile = ajFileNewFromCfile(stdout);
	else
	{
	    ajFmtPrintS(&acdPrettyFName, "%S.acdpretty", acdProgram);
	    acdPrettyFile = ajFileNewOutNameS(acdPrettyFName);
	    ajFileSetUnbuffer(acdPrettyFile);
	    ajFmtPrint("Created %S\n", acdPrettyFName);
	}
    }

    va_start(args, fmt);
    ajFmtVPrintS(&tmpstr, fmt, args);
    va_end(args);

    /*
    ** test for comment block before this line
    */

    while(ajListGetLength(acdListCommentsCount) && (acdCmtWord <= acdWordSave))
    {
	ajListPeek(acdListCommentsCount, (void**) &icword);
	ajListPeek(acdListCommentsColumn, (void**) &icpos);
	acdCmtWord = *icword;

	if((acdCmtWord == acdWordSave && !*icpos) ||
	    (acdCmtWord<=acdWordSave))
	{
	    if(!ajStrGetLen(cmtstr))
		ajFmtPrintF(acdPrettyFile, "\n");
	    
	    ajListPop(acdListCommentsCount, (void**) &icword);
	    ajListPop(acdListCommentsColumn, (void**) &icpos);
	    ajListstrPop(acdListComments, &cmtstr);
	    acdPrettyComment(cmtstr);
	    ajStrDel(&cmtstr);
	    AJFREE(icword);
	    AJFREE(icpos);
	    ccnt++;
	}
    }

    if(ccnt && ajStrGetCharFirst(tmpstr) != '\n')
	ajFmtPrintF(acdPrettyFile, "\n");

    while(ajStrGetCharFirst(tmpstr) == '\n')
    {
	ajFmtPrintF(acdPrettyFile, "\n");
	ajStrCutStart(&tmpstr, 1);
    }

    wordcount = ajStrParseCount(tmpstr);
    lastword = acdWordSave + wordcount;

    /*
    ** now check for inline comment on this line of text
    */

    if(ajListGetLength(acdListCommentsCount) && (acdCmtWord <= lastword))
    {
	ajListPeek(acdListCommentsCount, (void**) &icword);
	ajListPeek(acdListCommentsColumn, (void**) &icpos);
	acdCmtWord = *icword;
	if((acdCmtWord == lastword && *icpos) |
	   (acdCmtWord < lastword))
	{
	    ajListPop(acdListCommentsCount, (void**) &icword);
	    ajListPop(acdListCommentsColumn, (void**) &icpos);
	    ajListstrPop(acdListComments, &cmtstr);

	    ajStrRemoveWhiteExcess(&cmtstr);

	    if(!ajStrPrefixC(cmtstr, "#"))
		ajStrInsertC(&cmtstr, 0, "#");

	    if((ajStrGetLen(cmtstr) > 1) && !ajStrPrefixC(cmtstr, "# "))
		ajStrInsertC(&cmtstr, 1, " ");

	    i=0;
	    while(ajStrGetCharLast(tmpstr) == '\n')
	    {
		ajStrCutEnd(&tmpstr, 1);
		i++;
	    }
	    ipos = 75;

	    if(ajStrGetLen(cmtstr) > (79 - ipos))
		ipos = 79 - ajStrGetLen(cmtstr);

	    if(ipos <= (ajStrGetLen(tmpstr)+acdPrettyMargin))
		ipos = ajStrGetLen(tmpstr)+acdPrettyMargin+1;

	    j = ipos-ajStrGetLen(tmpstr)-acdPrettyMargin;
	    ajStrAppendCountK(&tmpstr, ' ', j);
	    ajStrAppendS(&tmpstr, cmtstr);
	    if(i)
		ajStrAppendCountK(&tmpstr, '\n', i);
	    ajStrDel(&cmtstr);
	    AJFREE(icword);
	    AJFREE(icpos);
	}
    }

    if(acdPrettyMargin)
	ajFmtPrintF(acdPrettyFile, "%.*s", acdPrettyMargin,
		    "                                                       ");
    ajFmtPrintF(acdPrettyFile, "%S", tmpstr);

    acdWordSave += wordcount;

    return;
}




/* @funcstatic acdPrettyClose *************************************************
**
** Writes any remaining comments and closes the pretty output file
**
** @return [void]
**
** @release 6.0.0
** @@
******************************************************************************/

static void acdPrettyClose(void)
{
    ajint *icword    = NULL;
    ajint *icpos    = NULL;
    AjPStr cmtstr = NULL;

    if(!acdDoPretty)
	return;

    if(!acdPrettyFName)
    {
	if(acdStdout)
	    acdPrettyFile = ajFileNewFromCfile(stdout);
	else
	{
	    ajFmtPrintS(&acdPrettyFName, "%S.acdpretty", acdProgram);
	    acdPrettyFile = ajFileNewOutNameS(acdPrettyFName);
	    ajFileSetUnbuffer(acdPrettyFile);
	    ajFmtPrint("Created %S\n", acdPrettyFName);
	}
    }

    while(ajListGetLength(acdListCommentsCount) && (acdCmtWord <= acdWordSave))
    {
	ajListPeek(acdListCommentsCount, (void**) &icword);
	acdCmtWord = *icword;

	if(acdCmtWord <= acdWordSave)
	{
	    if(!ajStrGetLen(cmtstr))
		ajFmtPrintF(acdPrettyFile, "\n");
	    
	    ajStrDel(&cmtstr);
	    ajListPop(acdListCommentsCount, (void**) &icword);
	    ajListPop(acdListCommentsColumn, (void**) &icpos);
	    ajListstrPop(acdListComments, &cmtstr);
	    acdPrettyComment(cmtstr);
	    AJFREE(icword);
	    AJFREE(icpos);
	}
    }

    ajStrDel(&cmtstr);

    ajFileClose(&acdPrettyFile);

    return;
}




/* @funcstatic acdPrettyComment ***********************************************
**
** Writes an indented one-line comment
**
** @param [r] comment [const AjPStr] OComment to be printer
** @return [void]
**
** @release 6.0.0
** @@
******************************************************************************/

static void acdPrettyComment(const AjPStr comment)
{
    AjPStr tmpstr = NULL;

    if(!acdDoPretty)
	return;

    if(!acdPrettyFName)
    {
	if(acdStdout)
	    acdPrettyFile = ajFileNewFromCfile(stdout);
	else
	{
	    ajFmtPrintS(&acdPrettyFName, "%S.acdpretty", acdProgram);
	    acdPrettyFile = ajFileNewOutNameS(acdPrettyFName);
	    ajFileSetUnbuffer(acdPrettyFile);
	    ajFmtPrint("Created %S\n", acdPrettyFName);
	}
    }

    ajStrAssignS(&tmpstr, comment);
    ajStrRemoveWhiteExcess(&tmpstr);

    if(!ajStrPrefixC(tmpstr, "#"))
	ajStrInsertC(&tmpstr, 0, "#");

    if((ajStrGetLen(tmpstr) > 1) && !ajStrPrefixC(tmpstr, "# "))
	ajStrInsertC(&tmpstr, 1, " ");


    /*
    ** margin is set for attributes
    ** reduce by 2 to align with definitions
    */

    if(acdPrettyMargin)
	ajFmtPrintF(acdPrettyFile, "%.*s", acdPrettyMargin,
		    "                                                       ");
    ajFmtPrintF(acdPrettyFile, "%S\n", tmpstr);

    ajStrDel(&tmpstr);

    return;
}




/* @funcstatic acdPrettyWrap **************************************************
**
** Writes a pretty formatted version of the .acd syntax
** line to the .acdpretty file
**
** @param [r] left [ajint] Extra left margin for follow-on lines
** @param [r] fmt [const char*] Format with ajFmt extensions
** @param [v] [...] Optional arguments
** @return [void]
**
** @release 2.7.0
** @@
******************************************************************************/

static void acdPrettyWrap(ajint left, const char *fmt, ...)
{
    va_list args;
    static AjPStr tmpstr = NULL;
    ajint leftmargin     = left + acdPrettyMargin;
    ajint width          = 78 - leftmargin;
    ajint *icword    = NULL;
    ajint *icpos    = NULL;
    AjPStr cmtstr = NULL;
    ajint lastword;
    ajint wordcount;
    ajuint ipos;
    ajint ccnt = 0;
    ajlong jpos;
    ajlong kpos;
    ajlong ilen;
    AjIList itercmt;
    AjIList iterpos;
    AjIList iterword;

    if(!acdDoPretty)
	return;

    if(!acdPrettyFName)
    {
	if(acdStdout)
	    acdPrettyFile = ajFileNewFromCfile(stdout);
	else
	{
	    ajFmtPrintS(&acdPrettyFName, "%S.acdpretty", acdProgram);
	    acdPrettyFile = ajFileNewOutNameS(acdPrettyFName);
	    ajFileSetUnbuffer(acdPrettyFile);
	}
    }

    /*
    ** whole line comments before wrapped text
    */

    while(ajListGetLength(acdListCommentsCount) && (acdCmtWord <= acdWordSave))
    {
	ajListPeek(acdListCommentsCount, (void**) &icword);
	acdCmtWord = *icword;

	if(acdCmtWord <= acdWordSave)
	{
	    if(!ajStrGetLen(cmtstr))
		ajFmtPrintF(acdPrettyFile, "\n");
	    
	    ajListPop(acdListCommentsCount, (void**) &icword);
	    ajListPop(acdListCommentsColumn, (void**) &icpos);
	    ajListstrPop(acdListComments, &cmtstr);
	    acdPrettyComment(cmtstr);
	    ajStrDel(&cmtstr);
	    AJFREE(icword);
	    AJFREE(icpos);
	}
    }

    va_start(args, fmt);
    ajFmtVPrintS(&tmpstr, fmt, args);
    va_end(args);

    ajStrExchangeCC(&tmpstr, " \\ ", " \\\n"); /* force newlines at '\' */

    wordcount = ajStrParseCount(tmpstr);
    lastword = acdWordSave + wordcount;

    /*
    ** Now check for inline comments in these lines of text
    */

    /*
    ** First iterate to get comment max length
    */

    ipos = 75;

    if(ajListGetLength(acdListCommentsCount) && (acdCmtWord <= lastword))
    {
        ajListPeek(acdListCommentsCount, (void**) &icword);
        acdCmtWord = *icword;
	itercmt  = ajListIterNewread(acdListComments);
	iterpos  = ajListIterNewread(acdListCommentsColumn);
	iterword = ajListIterNewread(acdListCommentsCount);

	while(!ajListIterDone(iterword) && (*icword <= lastword))
	{
	    icword = ajListIterGet(iterword);
	    icpos = ajListIterGet(iterpos);
	    cmtstr = ajListIterGet(itercmt);

	    if(*icword <= lastword && *icpos > (acdPrettyMargin+3))
	    {
		ccnt++;
		if(ajStrGetLen(cmtstr) > (79 - ipos))
		    ipos = 79 - ajStrGetLen(cmtstr);
		/*if(*icpos < (ajint) ipos)
		    ipos = *icpos;*/
	    }

            cmtstr = NULL;
	}

	ajListIterDel(&itercmt);
	ajListIterDel(&iterpos);
	ajListIterDel(&iterword);
    }

    if((ajint) ipos <= (leftmargin + 20))
       ipos = leftmargin + 20;

    width = ipos - leftmargin - 1;

    ajStrFmtWrapLeft(&tmpstr, width, acdPrettyMargin, left);

    /*
    ** Insert any comments at ends of lines
    */

    jpos=-1;

    while(ccnt--)
    {
	ajListPop(acdListCommentsCount, (void**) &icword);
	ajListPop(acdListCommentsColumn, (void**) &icpos);
	ajListstrPop(acdListComments, &cmtstr);
        ajStrRemoveWhiteExcess(&cmtstr);

	if(!ajStrPrefixC(cmtstr, "#"))
	    ajStrInsertC(&cmtstr, 0, "#");

	if((ajStrGetLen(cmtstr) > 1) && !ajStrPrefixC(cmtstr, "# "))
	    ajStrInsertC(&cmtstr, 1, " ");

	kpos = ajStrFindNextK(tmpstr, jpos+1, '\n');

	if(kpos < 0)
	{
	    if(jpos == (ajint) ajStrGetLen(tmpstr))
	    {
		ajStrAppendK(&tmpstr, '\n');
		ilen = 0;
	    }
	    else
		ilen = ajStrGetLen(tmpstr) - jpos - 1;

	    while(ilen < (ajlong) ipos)
	    {
		ajStrAppendK(&tmpstr, ' ');
		ilen++;
	    }

	    ajStrAppendS(&tmpstr, cmtstr);
	    jpos = ajStrGetLen(tmpstr);
	}
	else
	{
	    ilen = kpos - jpos - 1;
	    jpos = kpos;

	    while(ilen < (ajlong) ipos)
	    {
		ajStrInsertK(&tmpstr, jpos, ' ');
		ilen++;
		jpos++;
	    }
	    ajStrInsertS(&tmpstr, jpos, cmtstr);
	    jpos += ajStrGetLen(cmtstr);
	}

	ajStrDel(&cmtstr);
	AJFREE(icword);
	AJFREE(icpos);
    }


    ajFmtPrintF(acdPrettyFile, "%S\n", tmpstr);

    acdWordSave += wordcount;

    return;
}




/* @funcstatic acdPrettyShift *************************************************
**
** Right shifts (indents) acdpretty printing
**
** @return [void]
**
** @release 2.7.0
** @@
******************************************************************************/

static void acdPrettyShift(void)
{
    acdPrettyMargin += acdPrettyIndent;

    return;
}




/* @funcstatic acdPrettyUnShift ***********************************************
**
** Left shifts acdpretty printing
**
** @return [void]
**
** @release 2.7.0
** @@
******************************************************************************/

static void acdPrettyUnShift(void)
{
    acdPrettyMargin -= acdPrettyIndent;

    if(acdPrettyMargin < 0)
    {
	ajWarn("acdpretty printing indent error - too many left shifts");
	acdPrettyMargin = 0;
    }

    return;
}




/* @funcstatic acdIsQtype *****************************************************
**
** Tests whether an ACD object is a qualifier or parameter type. If not we
** assume it is a keyword type.
**
** @param [r] thys [const AcdPAcd] ACD object
** @return [AjBool] ajTrue if the object is a qualifier or parameter
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdIsQtype(const AcdPAcd thys)
{
    if((thys->Level == ACD_QUAL) || (thys->Level == ACD_PARAM))
	return ajTrue;

    return ajFalse;
}




/* @funcstatic acdIsStype *****************************************************
**
** Tests whether an ACD object is a section or endsection type.
**
** @param [r] thys [const AcdPAcd] ACD object
** @return [AjBool] ajTrue if the object is a section or endsection
**                  type definition
**
** @release 2.3.0
** @@
******************************************************************************/

static AjBool acdIsStype(const AcdPAcd thys)
{
    if((thys->Level == ACD_SEC) || (thys->Level == ACD_ENDSEC))
	return ajTrue;

    return ajFalse;
}




/* @funcstatic acdIsAtype *****************************************************
**
** Tests whether an ACD object is an application type.
**
** @param [r] thys [const AcdPAcd] ACD object
** @return [AjBool] ajTrue if the object is an application type definition
**
** @release 6.1.0
** @@
******************************************************************************/

static AjBool acdIsAtype(const AcdPAcd thys)
{
    if(thys->Level == ACD_APPL)
	return ajTrue;

    return ajFalse;
}




/* @funcstatic acdIsVtype *****************************************************
**
** Tests whether an ACD object is a variable type.
**
** @param [r] thys [const AcdPAcd] ACD object
** @return [AjBool] ajTrue if the object is a variable type definition
**
** @release 6.1.0
** @@
******************************************************************************/

static AjBool acdIsVtype(const AcdPAcd thys)
{
    if(thys->Level == ACD_VAR)
	return ajTrue;

    return ajFalse;
}




/* @funcstatic acdTextFormat **************************************************
**
** Converts backslash codes in a string into special characters
**
** @param [u] text [AjPStr*] Text with backslash codes
** @return [AjBool] ajTrue if successful
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool acdTextFormat(AjPStr* text)
{
    ajStrExchangeCC(text, " \\ ", "\n");

    return ajTrue;
}




/* @funcstatic acdTextTrim*****************************************************
**
** Trims white space. For HTML output, replaces angled brackets
**
** @param [u] text [AjPStr*] Text to be processed
** @return [void]
**
** @release 4.1.0
** @@
******************************************************************************/

static void acdTextTrim(AjPStr* text)
{
    ajStrTrimC(text, " \t\n\r");

    if(acdDoTable)
    {
	ajStrExchangeCC(text, "<", "&lt;");
	ajStrExchangeCC(text, ">", "&gt;");
    }

    return;
}




/* @section print *************************************************************
**
** @fdata [none]
**
** Print internal details for use by entrails
**
** @nam3rule Print Print internal details
** @nam4rule PrintAppl Print details of ACD application attributes
** @nam4rule PrintQual Print details of ACD datatype known qualifiers
** @nam4rule PrintType Print details of ACD datatypes
**
** @argrule Print outf [AjPFile] Output file
** @argrule Print full [AjBool] Print full details
**
** @valrule * [void]
** @fcategory misc
**
******************************************************************************/




/* @func ajAcdPrintAppl *******************************************************
**
** Report details of all known ACD attributes for all applications.
** For use by EMBOSS entrails.
**
** @param [u] outf [AjPFile] Output file
** @param [r] full [AjBool] Full report - currently no extra details printed
** @return [void]
**
**
** @release 4.0.0
******************************************************************************/

void ajAcdPrintAppl(AjPFile outf, AjBool full)
{
    ajint i;
    AcdPAttr attr;
    AjPStr tmpstr = NULL;
    ajuint maxtmp = 0;

    if(full)
	ajFmtPrintF(outf, "\n");
    else
	ajFmtPrintF(outf, "\n");

    ajFmtPrintF(outf, "# ACD Application Attributes\n");
    ajFmtPrintF(outf, "# Attribute       Type       Default      Helptext\n");
    
    ajFmtPrintF(outf, "AttrAppl {\n");

    for(i=0; acdAttrAppl[i].Name; i++)
    {
	attr = &acdAttrAppl[i];
	ajFmtPrintF(outf, "  %-15s", attr->Name);
	ajFmtPrintF(outf, " %-10s", acdValNames[attr->Type]);
	ajFmtPrintS(&tmpstr, "\"%s\"", attr->Default);

	if(ajStrGetLen(tmpstr) > maxtmp)
	    maxtmp = ajStrGetLen(tmpstr);

	ajFmtPrintF(outf, " %-12S", tmpstr);
	ajFmtPrintF(outf, " \"%s\"", attr->Help);
	ajFmtPrintF(outf, "\n");
    }

    ajFmtPrintF(outf, "}\n\n");

    if(maxtmp > 12) ajWarn("ajAcdPrintAppl max tmpstr len %d",
			maxtmp);	      
    ajStrDel(&tmpstr);

    return;
}




/* @func ajAcdPrintQual *******************************************************
**
** Report details of all known ACD qualifiers for all applications.
** For use by EMBOSS entrails.
**
** @param [u] outf [AjPFile] Output file
** @param [r] full [AjBool] Full report - currently no extra details printed
** @return [void]
**
**
** @release 2.8.0
******************************************************************************/

void ajAcdPrintQual(AjPFile outf, AjBool full)
{
    ajint i;
    AcdPQual qual;
    AjPStr tmpstr = NULL;
    ajuint maxtmp = 0;

    if(full)
	ajFmtPrintF(outf, "\n");
    else
	ajFmtPrintF(outf, "\n");

    ajFmtPrintF(outf, "# ACD Application Qualifiers\n");
    ajFmtPrintF(outf, "# Qualifier       Type        Default     Helptext\n");
    ajFmtPrintF(outf, "QualAppl {\n");

    for(i=0; acdQualAppl[i].Name; i++)
    {
	qual = &acdQualAppl[i];
	ajFmtPrintF(outf, "  %-15s", qual->Name);
	ajFmtPrintF(outf, " %-10s", qual->Type);
	ajFmtPrintS(&tmpstr, " \"%s\"", qual->Default);

	if(ajStrGetLen(tmpstr) > maxtmp)
	    maxtmp = ajStrGetLen(tmpstr);
	ajFmtPrintF(outf, " %-12S", tmpstr);
	ajFmtPrintF(outf, " \"%s\"", qual->Help);
	ajFmtPrintF(outf, "\n");
    }

    ajFmtPrintF(outf, "}\n\n");
    if(maxtmp > 12) ajWarn("ajAcdPrintQual max tmpstr len %d",
			maxtmp);	      
    ajStrDel(&tmpstr);

    return;
}




/* @func ajAcdPrintType *******************************************************
**
** Report details of all known ACD types.
** For use by EMBOSS entrails.
**
** @param [u] outf [AjPFile] Output file
** @param [r] full [AjBool] Full report
** @return [void]
**
**
** @release 1.0.0
******************************************************************************/

void ajAcdPrintType(AjPFile outf, AjBool full)
{
    AcdPType pat;
    AcdPAttr attr;
    AcdPQual qual;
    ajint i;
    AjPStr tmpstr = NULL;
    ajuint maxtmp = 0;

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# ACD Types\n");
    ajFmtPrintF(outf, "# Name           Group      Description\n");
    ajFmtPrintF(outf, "#     Attribute       Type       "
		"Default      Comment\n");
    ajFmtPrintF(outf, "#     Qualifier       Type       "
		"Default      Helptext\n");
    ajFmtPrintF(outf, "AcdType {\n");
    
    for(i=0; acdType[i].Name; i++) 
    {
	pat = &acdType[i];
	ajFmtPrintF(outf, "  %-14s", pat->Name);
	ajFmtPrintF(outf, " %-10s", pat->Group);
	ajFmtPrintF(outf, " \"%s\"", pat->Valid);
	ajFmtPrintF(outf, "\n");

	if(full && pat->Attr)
	{
	    ajFmtPrintF(outf, "    attributes {\n");

	    for(attr=pat->Attr; attr->Name; attr++)
	    {
		ajFmtPrintF(outf, "      %-15s", attr->Name);
		ajFmtPrintF(outf, " %-10s", acdValNames[attr->Type]);
		ajFmtPrintS(&tmpstr, "\"%s\"", attr->Default);

		if(ajStrGetLen(tmpstr) > maxtmp)
		    maxtmp = ajStrGetLen(tmpstr);

		ajFmtPrintF(outf, " %-12S", tmpstr);
		ajFmtPrintF(outf, " \"%s\"", attr->Help);
		ajFmtPrintF(outf, "\n");
	    }

	    ajFmtPrintF(outf, "    }\n");
	}

	if(pat->Quals)
	{
	    ajFmtPrintF(outf, "    qualifiers {\n");

	    for(qual=pat->Quals; qual->Name; qual++)
	    {
		ajFmtPrintF(outf, "      %-15s", qual->Name);
		ajFmtPrintF(outf, " %-10s", qual->Type);
		ajFmtPrintS(&tmpstr, "\"%s\"", qual->Default);

		if(ajStrGetLen(tmpstr) > maxtmp)
		    maxtmp = ajStrGetLen(tmpstr);

		ajFmtPrintF(outf, " %-12S", tmpstr);
		ajFmtPrintF(outf, " \"%s\"", qual->Help);
		ajFmtPrintF(outf, "\n");
	    }
	    ajFmtPrintF(outf, "    }\n");
	}
    }
    ajFmtPrintF(outf, "}\n");
    
    ajFmtPrintF(outf, "# ACD Default attributes\n");
    ajFmtPrintF(outf, "# Name             Type     Default        Comment\n");

    for(i=0; acdAttrDef[i].Name; i++)
    {
	ajFmtPrintF(outf, "  %-15s", acdAttrDef[i].Name);
	ajFmtPrintF(outf, "  %-10s", acdValNames[acdAttrDef[i].Type]);
	ajFmtPrintS(&tmpstr, "\"%s\"", acdAttrDef[i].Default);

	if(ajStrGetLen(tmpstr) > maxtmp)
	    maxtmp = ajStrGetLen(tmpstr);

	ajFmtPrintF(outf, " %-12S", tmpstr);
	ajFmtPrintF(outf, " \"%s\"", acdAttrDef[i].Help);
	ajFmtPrintF(outf, "\n");
    }

    ajFmtPrintF(outf, "\n");

    ajFmtPrintF(outf, "# ACD Calculated attributes\n");
    ajFmtPrintF(outf, "# Name\n");
    ajFmtPrintF(outf, "#     Attribute      Type       "
		"Default              Comment\n");

    acdPrintCalcattr(outf, "distances", acdCalcDistances);
    acdPrintCalcattr(outf, "features", acdCalcFeat);
    acdPrintCalcattr(outf, "frequencies", acdCalcFrequencies);
    acdPrintCalcattr(outf, "properties", acdCalcProperties);
    acdPrintCalcattr(outf, "regexp", acdCalcRegexp);
    acdPrintCalcattr(outf, "sequence", acdCalcSeq);
    acdPrintCalcattr(outf, "seqall", acdCalcSeqall);
    acdPrintCalcattr(outf, "seqset", acdCalcSeqset);
    acdPrintCalcattr(outf, "seqsetall", acdCalcSeqsetall);
    acdPrintCalcattr(outf, "string", acdCalcString);
    acdPrintCalcattr(outf, "tree", acdCalcTree);
    ajFmtPrintF(outf, "\n");

    if(maxtmp > 12)
        ajWarn("ajAcdPrintType max tmpstr len %d",
               maxtmp);	      

    ajStrDel(&tmpstr);

    return;
}




/* @funcstatic acdPrintCalcattr ***********************************************
**
** Report calculated attributes set
** For use by EMBOSS entrails.
**
** @param [u] outf [AjPFile] Output file
** @param [r] acdtype [const char*] ACD type name
** @param [r] calcattr [const AcdOAttr[]] Acd calculated attributes
** @return [void]
**
**
** @release 4.0.0
******************************************************************************/

static void acdPrintCalcattr(AjPFile outf, const char* acdtype,
			     const AcdOAttr calcattr[])
{
    ajint i;

    AjPStr tmpstr = NULL;
    ajuint maxtmp = 0;

    ajFmtPrintF(outf, "  %s",acdtype);
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "    attributes {\n");

    for(i=0; calcattr[i].Name; i++)
    {
	ajFmtPrintF(outf, "      %-14s", calcattr[i].Name);
	ajFmtPrintF(outf, " %-10s", acdValNames[calcattr[i].Type]);
	ajFmtPrintS(&tmpstr, "\"%s\"", calcattr[i].Default);

	if(ajStrGetLen(tmpstr) > maxtmp)
	    maxtmp = ajStrGetLen(tmpstr);

	ajFmtPrintF(outf, " %-20S", tmpstr);
	ajFmtPrintF(outf, " \"%s\"", calcattr[i].Help);
	ajFmtPrintF(outf, "\n");
    }

    ajFmtPrintF(outf, "    }\n");

    if(maxtmp > 20)
        ajWarn("acdPrintCalcAttr max tmpstr len %d",
               maxtmp);	      

    ajStrDel(&tmpstr);

    return;
}




/* @funcstatic acdVocabCheck **************************************************
**
** Checks for a string in a controlled vocabulary of character strings,
** ended with a NULL.
**
** @param [r] str [const AjPStr] Test string
** @param [r] vocab [const char**] Controlled vocabulary
** @return [AjBool] ajTrue if the string matched on of the words
**
** @release 2.0.0
******************************************************************************/

static AjBool acdVocabCheck(const AjPStr str, const char** vocab)
{
    ajint i = 0;

    while(vocab[i])
    {
	if(ajStrMatchCaseC(str, vocab[i]))
	    return ajTrue;

	i++;
    }

    return ajFalse;
}




/* @funcstatic acdError *******************************************************
**
** Formatted write as an error message, then exits with ajExitBad
**
** @param [r] fmt [const char*] Format string
** @param [v] [...] Format arguments.
** @return [void]
**
** @release 2.7.0
** @@
******************************************************************************/

__noreturn static void acdError(const char* fmt, ...)
{
    va_list args ;
    AjPStr errstr = NULL;
    ajint linenum;

    acdErrorCount++;

    va_start(args, fmt) ;
    ajFmtVPrintS(&errstr, fmt, args);
    va_end(args) ;

    if(acdLineNum > 0)
	linenum = acdLineNum;
    else if(acdSetCurr)
	linenum = acdSetCurr->LineNum;
    else if(acdProcCurr)
	linenum = acdProcCurr->LineNum;
    else if(acdListCurr)
	linenum = acdListCurr->LineNum;
    else
	linenum = 0;

    ajErr("File %S line %d: %S", acdFName, linenum, errstr);
    ajStrDel(&errstr);
    ajExitBad();
}




/* @funcstatic acdErrorValid **************************************************
**
** Formatted write as an error message, then continues (acdError exits)
**
** @param [r] fmt [const char*] Format string
** @param [v] [...] Format arguments.
** @return [void]
**
** @release 2.7.0
** @@
******************************************************************************/

static void acdErrorValid(const char* fmt, ...)
{
    va_list args;
    AjPStr errstr=NULL;
    ajint linenum;
    acdErrorCount++;

    va_start(args, fmt) ;
    ajFmtVPrintS(&errstr, fmt, args);
    va_end(args) ;

    if(acdLineNum > 0)
	linenum = acdLineNum;
    else if(acdSetCurr)
	linenum = acdSetCurr->LineNum;
    else if(acdProcCurr)
	linenum = acdProcCurr->LineNum;
    else if(acdListCurr)
	linenum = acdListCurr->LineNum;
    else
	linenum = 0;

    ajErr("File %S line %d: %S", acdFName, linenum, errstr);
    ajStrDel(&errstr);

    return;
}




/* @funcstatic acdWarnObsolete ************************************************
**
** Warning about obsolete status of an application
**
** @param [r] str [const AjPStr] Format string
** @return [void]
**
** @release 6.0.0
** @@
******************************************************************************/

static void acdWarnObsolete(const AjPStr str)
{
    AjPStr outstr = NULL;
    AjPStr tmpstr = NULL;
    AjBool warnobsolete = ajTrue;

    if(ajNamGetValueC("warnobsolete", &tmpstr))
	ajStrToBool(tmpstr, &warnobsolete);

    ajStrDel(&tmpstr);

    if(!warnobsolete)
        return;
    
    ajFmtPrintS(&outstr, "Application %S is marked as obsolete.\n%S",
                acdProgram, str);
    ajStrFmtWrap(&outstr, 75);
    ajWarn("%S", outstr);

    ajStrDel(&outstr);

    return;
}




/* @funcstatic acdWarn ********************************************************
**
** Formatted write as an error message.
**
** @param [r] fmt [const char*] Format string
** @param [v] [...] Format arguments.
** @return [void]
**
** @release 2.7.0
** @@
******************************************************************************/

static void acdWarn(const char* fmt, ...)
{
    va_list args;
    AjPStr errstr = NULL;
    ajint linenum;

    acdErrorCount++;

    va_start(args, fmt) ;
    ajFmtVPrintS(&errstr, fmt, args);
    va_end(args) ;

    if(acdLineNum > 0)
	linenum = acdLineNum;
    else if(acdSetCurr)
	linenum = acdSetCurr->LineNum;
    else if(acdProcCurr)
	linenum = acdProcCurr->LineNum;
    else if(acdListCurr)
	linenum = acdListCurr->LineNum;
    else
	linenum = 0;

    ajWarn("File %S line %d: %S", acdFName, linenum, errstr);
    ajStrDel(&errstr);

    return;
}




/* @funcstatic acdErrorAcd ****************************************************
**
** Formatted write as an error message, for a specified ACD object
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [r] fmt [const char*] Format string
** @param [v] [...] Format arguments.
** @return [void]
**
** @release 2.7.0
** @@
******************************************************************************/

__noreturn static void acdErrorAcd(const AcdPAcd thys, const char* fmt, ...)
{
    va_list args;
    AjPStr errstr = NULL;

    acdErrorCount++;
    ajUtilCatch();
    va_start(args, fmt) ;
    ajFmtVPrintS(&errstr, fmt, args);
    va_end(args) ;

    ajErr("File %S line %d: (%S) %S",
	  acdFName, thys->LineNum, thys->Name, errstr);
    ajStrDel(&errstr);
    ajExitBad();
}




/* @section exit **************************************************************
**
** Functions called on exit
**
** @fdata [none]
**
** @nam3rule Exit Cleanup of internals when program exits
**
** @argrule Exit silent [AjBool] If false, produce warnings for unused
**                               ACD datatype definitions
**
** @valrule * [void]
** @fcategory misc
**
******************************************************************************/




/* @func ajAcdExit ************************************************************
**
** Reports any unused ACD values
**
** Cleans up ACD processing  internal memory
**
** @param [r] silent [AjBool] Turn off messages (used when some messages
**                            are expected but can be ignored).
** @return [void]
**
** @release 2.4.0
** @@
******************************************************************************/

void ajAcdExit(AjBool silent)
{
    AcdPAcd pa;
    static AjBool staySilent = AJFALSE;
    AjPFile cmdlogfile = NULL;
    AjPStr  cmdlog     = NULL;
    AjPStr  cmdstr     = NULL;

    if(silent)
	staySilent = ajTrue;

    if(acdDoHelp)
	staySilent = ajTrue;

    if(acdDoPretty)
	staySilent = ajTrue;

    if(!staySilent)
    {
	/* turn off the warnings for now ... comment out this line to
	 ** enable them. 
	 ** the problem is that some programs have conditionals around the
	 ** ajAcdGet calls so they can sometimes fail to use values, even where
	 ** the test cases say they are fine.
	 ** also some ACD files (showdb for example) use values,
	 ** but in some cases (showdb with all booleans on the command line)
	 ** the value is not needed
	 */

/*	ajDebug("ajAcdExit Name................ Assoc Level  Used\n");*/
	for(pa=acdList; pa; pa=pa->Next)
	{
	    /*ajDebug("ajAcdExit %20S   %3B   %3d   %3B\n",
		    pa->Name, pa->Assoc, pa->Level, pa->Used); */
	    if(pa->Assoc)
		continue;
	    if(pa->Level != ACD_PARAM && pa->Level != ACD_QUAL)
		continue;
	    if(!pa->Used)
		acdLog("ACD qualifier never used: %S = '%S' (assoc %B)",
		       pa->Name, pa->ValStr, pa->Assoc);
	}
    }


    /* report the command line to a log if requested.
     ** added mainly to log command lines for the QA tests
     ** so they can also be used in memory leak tests
     **
     ** Need to test acdProgram is still set - acdc calls this twice
     */

    if(acdCommandLine &&
       ajStrGetLen(acdProgram) &&
       ajNamGetValueC("acdcommandlinelog", &cmdlog))
    {
	cmdlogfile = ajFileNewOutappendNameS(cmdlog);
	ajStrAssignS(&cmdstr, acdArgSave);
	if(ajStrGetLen(acdInputSave))
	{
	    ajStrAppendK(&cmdstr, ' ');
	    ajStrAppendS(&cmdstr, acdInputSave);
	}
	ajStrRemoveWhiteExcess(&cmdstr);
	ajFmtPrintF(cmdlogfile, "%S %S\n", acdProgram, cmdstr);
	ajFileClose(&cmdlogfile);
	ajStrDel(&cmdlog);
	ajStrDel(&cmdstr);
    }


    /*
    ** clean up memory:
    **
    */

    acdReset();

    return;
}




/* @funcstatic acdPrintUsed ***************************************************
**
** Quick printoout of all qualifiers believed to have been used
**
** @return [void]
**
** @release 6.0.0
******************************************************************************/

static void acdPrintUsed(void)
{
    AcdPAcd pa;

    ajUserDumpS(acdArgSave);

    for(pa=acdList; pa; pa=pa->Next)
        if(pa->UserDefined)
            ajUser("ACD qual defined: '%S'", pa->Name);

    return;
}




/* @funcstatic acdReset *******************************************************
**
** Cleans up all memory allocated to ACD internals.
**
** All ACD objects are assumed to have been passed to the calling program
** and are no longer "owned" by ACD.
**
** In future, we could consider removing them once they have been fetched.
**
** @return [void]
**
** @release 4.0.0
******************************************************************************/

static void acdReset(void)
{
    AcdPAcd pa;
    AcdPAcd qa = NULL;

    /* acdPrintUsed(); */

    for(pa=acdList; pa; pa=qa)
    {
	qa = pa->Next;
	acdDel(&pa);
    }

    ajStrDel(&acdPackName);
    ajStrDel(&acdPackVersion);

    ajTablestrFree(&acdKnowntypeTypeTable);
    ajTablestrFree(&acdKnowntypeDescTable);
    ajTablestrFree(&acdCodeTable);
    ajTablestrFree(&acdGrpTable);

    ajStrDel(&acdTmpStr);
    ajStrDel(&acdTmpStr2);

    ajStrDel(&acdPrefName);
    ajStrDel(&acdPrefToken);

    ajStrDel(&acdArgSave);
    ajStrDel(&acdInputSave);
    ajStrDel(&acdInputName);
    ajStrDel(&acdInFName);
    ajStrDel(&acdOutFName);
    ajStrDel(&acdOutFExt);
    ajStrDel(&acdInTypeFeatName);
    ajStrDel(&acdInTypeSeqName);
    ajStrDel(&acdTmpOutFName);

    ajStrDel(&acdLogFName);
    ajFileClose(&acdLogFile);

    ajListstrFreeData(&acdSecList);
    ajTablestrFree(&acdSecTable);
    ajTablestrFree(&acdExternalTable);

    ajStrDel(&acdPrettyFName);
    ajFileClose(&acdPrettyFile);
    ajStrDel(&acdFName);
    ajStrDel(&acdOutFullFName);
    ajStrDel(&acdVarAcdProtein);

    acdParseQuotes = ajFalse;
    ajStrDel(&acdParseReturn);
    ajStrDel(&acdReply);
    ajStrDel(&acdReplyDef);
    ajStrDel(&acdReplyPrompt);
    ajStrDel(&acdUserMsg);
    ajStrDel(&acdUserReplyDef);
    ajStrDel(&acdDirectoryDef);
    ajStrDel(&acdQNameTmp);
    ajStrDel(&acdQTypeTmp);
    ajStrDel(&acdAttrValTmp);
    ajStrDel(&acdQualNameTmp);
    ajStrDel(&acdQualNumTmp);

    ajRegFree(&acdRegQualParse);
    ajRegFree(&acdRegVarname);
    ajRegFree(&acdRegFunction);
    ajRegFree(&acdRegToggle);
    ajRegFree(&acdRegResolveVar);
    ajRegFree(&acdRegResolveFun);

    ajRegFree(&acdRegExpPlusI);
    ajRegFree(&acdRegExpPlusD);
    ajRegFree(&acdRegExpMinusI);
    ajRegFree(&acdRegExpMinusD);
    ajRegFree(&acdRegExpStarI);
    ajRegFree(&acdRegExpStarD);
    ajRegFree(&acdRegExpDivI);
    ajRegFree(&acdRegExpDivD);
    ajRegFree(&acdRegExpEqualI);
    ajRegFree(&acdRegExpEqualD);
    ajRegFree(&acdRegExpEqualT);
    ajRegFree(&acdRegExpNeI);
    ajRegFree(&acdRegExpNeD);
    ajRegFree(&acdRegExpNeT);
    ajRegFree(&acdRegExpGtI);
    ajRegFree(&acdRegExpGtD);
    ajRegFree(&acdRegExpGtT);
    ajRegFree(&acdRegExpLtI);
    ajRegFree(&acdRegExpLtD);
    ajRegFree(&acdRegExpLtT);
    ajRegFree(&acdRegExpOrI);
    ajRegFree(&acdRegExpOrD);
    ajRegFree(&acdRegExpOrT);
    ajRegFree(&acdRegExpAndI);
    ajRegFree(&acdRegExpAndD);
    ajRegFree(&acdRegExpAndT);
    ajRegFree(&acdRegExpCond);
    ajRegFree(&acdRegExpNot);
    ajRegFree(&acdRegExpOneofCase);
    ajRegFree(&acdRegExpOneofList);
    ajRegFree(&acdRegExpCaseCase);
    ajRegFree(&acdRegExpCaseList);
    ajRegFree(&acdRegExpFilename);
    ajRegFree(&acdRegExpFileExists);
    ajRegFree(&acdRegExpValue);

    ajListFreeData(&acdListCommentsCount);
    ajListFreeData(&acdListCommentsColumn);
    ajListstrFreeData(&acdListComments);

    ajStrDel(&acdProgram);
    ajStrDel(&acdAppldoc);
    AJFREE(acdParamSet);

    acdDoHelp = AJFALSE;
    acdDoLog = AJFALSE;
    acdDoWarnRange =AJTRUE;
    acdDoPretty = AJFALSE;
    acdDoTable = AJFALSE;
    acdDoValid = AJFALSE;
    acdVerbose = AJFALSE;
    acdAuto = AJFALSE;
    acdFilter = AJFALSE;
    acdOptions = AJFALSE;
    acdStdout = AJFALSE;
    acdCodeSet = AJFALSE;
    acdKnowntypeSet = AJFALSE;
    acdWrapper = AJFALSE;
/*    acdQualTestSkip = AJFALSE; */
    acdInFile = 0;
    acdInFileSet = AJFALSE;
    acdOutFile = 0;
    acdPromptTry = 2;
    acdPrettyMargin = 0;
    acdPrettyIndent = 2;
    acdLineNum = 0;
    acdWordNum = 0;
    acdErrorCount = 0;
    acdUseData = 0;
    acdUseFeatures = 0;
    acdUseInfile = 0;
    acdUseSeq = 0;
    acdUseAlign = 0;
    acdUseFeatout = 0;
    acdUseOutfile = 0;
    acdUseReport = 0;
    acdUseSeqout = 0;
    acdUseGraph = 0;
    acdUseMisc = 0;
    acdCurrentStage = NO_STAGE;
    acdNParam=0;

    acdNewCurr = NULL;
    acdMasterQual = NULL;
    acdList = NULL;
    acdListLast = NULL;
    acdListCurr = NULL;
    acdProcCurr = NULL;
    acdSetCurr = NULL;

    return;
}




/* @funcstatic acdValidAppl ***************************************************
**
** Validation for an application definition
**
** @param [r] thys [const AcdPAcd] ACD object
** @return [void]
**
** @release 2.7.0
** @@
******************************************************************************/

static void acdValidAppl(const AcdPAcd thys)
{
    ajint i;
    ajuint idocmax = 70;			/* maximum length of
					   documentation string */

    AjPStrTok tokenhandle = NULL;
    AjPStr relation = NULL;
    const AjPStr namespace;
    ajuint ireloper = 0;
    ajuint ireltopic = 0;

    if(!acdDoValid)
	return;

    /* must have a documentation attribute */

    i = acdFindAttrC(acdAttrAppl, "documentation");
    if(!ajStrGetLen(thys->AttrStr[i]))
	acdErrorValid("Application has no documentation defined");

    else
    {
	if(!isupper((int)ajStrGetCharFirst(thys->AttrStr[i])))
	{
	    if (islower((int)ajStrGetCharFirst(thys->AttrStr[i])))
		acdWarn("Documentation string starts in lower case");
	    else
		acdWarn("Documentation string starts non-alphabetic");
	}
	if (ajStrGetLen(thys->AttrStr[i]) > idocmax)
		acdWarn("Documentation string %d exceeds %d characters",
			ajStrGetLen(thys->AttrStr[i]), idocmax);
    }

    /* must have a group attribute */

    i = acdFindAttrC(acdAttrAppl, "groups");

    if(!ajStrGetLen(thys->AttrStr[i]))
	acdErrorValid("Application has no groups defined");

    /* group must be a known group (and subgroup) */

    acdValidApplGroup(thys->AttrStr[i]);

    i = acdFindAttrC(acdAttrAppl, "keywords");
    acdValidApplKeywords(thys->AttrStr[i]);

        /* for now, skip EMBASSY applications */
/*
    if(acdEdam)
    {
        i = acdFindAttrC(acdAttrAppl, "embassy");
        if(ajStrGetLen(thys->AttrStr[i]))
            acdEdam = NULL;
    }
*/
    if(acdEdam)
    {
        i = acdFindAttrC(acdAttrAppl, "relations");
        tokenhandle = ajStrTokenNewC(thys->AttrStr[i], "|");

        while(ajStrTokenNextParse(tokenhandle, &relation))
        {
            namespace = acdEdamTest(relation);
            if(!namespace)
                acdErrorValid("Application relation '%S' not found",
                              relation);
            else if(ajStrMatchC(namespace, "operation"))
                ireloper++;
            else if(ajStrMatchC(namespace, "topic"))
                ireltopic++;
            else
                acdErrorValid("Application has relation with namespace '%S'",
                              namespace);
        }

        if(!ireltopic)
            acdErrorValid("Application has no 'topic' relation");
        if(!ireloper)
            acdErrorValid("Application has no 'operation' relation");

        ajStrDel(&relation);
        ajStrTokenDel(&tokenhandle);
    }
    
    /*
    ** test for wrapper attribute
    ** if true, we use the original command line
    ** so we turn off EMBOSS validation of qualifier names
    */

    return;
}




/* @funcstatic acdValidRelation ***********************************************
**
** Validation for a relation definition
**
** @param [r] thys [const AcdPAcd] ACD object
** @return [void]
**
** @release 2.9.0
** @@
******************************************************************************/

static void acdValidRelation(const AcdPAcd thys)
{
    const AjPStr tmpstr   = NULL;

    if(!acdDoValid)
	return;

    tmpstr = acdAttrValue(thys, "relations");

    if(!ajStrGetLen(tmpstr))
	return;

    return;
}




/* @funcstatic acdValidSection ************************************************
**
** Validation for a section definition
**
** @param [r] thys [const AcdPAcd] ACD object
** @return [void]
**
** @release 2.7.0
** @@
******************************************************************************/

static void acdValidSection(const AcdPAcd thys)
{
    static AjPTable typeTable = NULL;
    static AjPTable infoTable = NULL;

    static ajint sectLevel = 0;
    static ajint sectNumber= -1;

    const AjPStr sectType = NULL; /* string from table - no delete */
    const AjPStr sectInfo = NULL; /* string from table - no delete */
    const AjPStr tmpstr   = NULL;
    static AjPStr sectNameTop;
    ajint i;


    if(!acdDoValid)
	return;

    if(!typeTable)
	acdReadSections(&typeTable, &infoTable);

    if(ajCharMatchC(acdKeywords[thys->Type].Name, "endsection"))
    {
	--sectLevel;
	if(sectLevel < 0)
	    acdErrorValid("Too many endsections");
	return;
    }

    ++sectLevel;

    if(sectLevel == 1)
    {
	ajStrAssignS(&sectNameTop, thys->Name);
	for(i=0; acdSections[i]; i++)
	{
	    if(ajStrMatchC(thys->Name, acdSections[i]->Name))
	    {
		if(i < sectNumber)
		    acdWarn("Section '%S' follows section '%s'",
				 thys->Name, acdSections[sectNumber]->Name);
		else
		    sectNumber = i;
		break;
	    }
	}

	if(!acdSections[i])
	    acdWarn("No defined order for primary section '%S'",
		    thys->Name);
    }
    /* should have a known name */

    sectType = ajTableFetchS(typeTable, thys->Name);
    tmpstr = acdAttrValue(thys, "type");

    if(!sectType)
    {
	if(sectLevel == 1)
	    acdErrorValid("Section '%S' not defined in sections.standard file",
			  thys->Name);
	else
	    acdWarn("Sub level section '%S' not defined in "
		    "sections.standard file",
		    thys->Name);
	/* if unknown, must be distinctive - check for common words? */

	return;
    }
    else
    {			     /* if known, must have a standard type */
	if(sectLevel == 1)
	{
	    if(!ajStrMatchC(tmpstr, "page"))
		acdErrorValid("Top level section '%S' not of type 'page'",
			      thys->Name);

	    else if(!ajStrMatchC(sectType, "page"))
		acdErrorValid("Top level section '%S' defined as "
			      "sub type of '%S' in sections.standard",
			      thys->Name, sectType);
	}
	else
	{
	    if(ajStrMatchC(tmpstr, "page"))
		acdErrorValid("Sub level section '%S' not of type 'frame'",
			      thys->Name);
	    else if(ajStrPrefixC(sectType, "page"))
		acdErrorValid("Sub level section '%S' not defined "
			      "as type 'frame' in sections.standard",
			      thys->Name);
	    else if(!ajStrMatchS(sectType, sectNameTop))
		acdErrorValid("Sub level section '%S' should be under '%S'",
			      thys->Name, sectType);
	}
    }

    sectInfo = ajTableFetchS(infoTable, thys->Name);
    tmpstr = acdAttrValue(thys, "information");

    /* must have an information attribute */

    if(!ajStrGetLen(tmpstr))
	acdWarn("Section has no information string");

    /* if known, must have a standard info string */

    else
    {
	if(!ajStrMatchS(sectInfo, tmpstr))
	{
	    if(ajStrMatchCaseS(sectInfo, tmpstr))
		acdWarn("Section info '%S' expected, case mismatch",
			sectInfo);
	    else
		acdErrorValid("Section info '%S' expected, found '%S'",
			      sectInfo, tmpstr);
	}
    }

    return;
}




/* @funcstatic acdValidQual ***************************************************
**
** Validation for a qualifier definition
**
** @param [u] thys [AcdPAcd] ACD object
** @return [void]
**
** @release 2.7.0
** @@
******************************************************************************/

static void acdValidQual(AcdPAcd thys)
{
    ajint itype;
    AjBool isparam = ajFalse;
    AjBool boolval;
    AjBool toggle;

    AjBool isParameter = ajFalse;
    AjBool isStandard = ajFalse;
    AjBool isAdditional = ajFalse;
    AjBool isToggle = ajFalse;

    static AjPStr secname   = NULL;
    const AjPStr relstr    = NULL;
    const AjPStr tmpstr    = NULL;
    const AjPStr tmpinfo   = NULL;
    const AjPStr tmpprompt = NULL;
    const AjPStr tmphelp   = NULL;
    const AjPStr tmpstandard = NULL;

    static ajint qualCountSeq     = 0;
    static ajint qualCountSeqout  = 0;
    static ajint qualCountInfile  = 0;
    static ajint qualCountOutfile = 0;
    static ajint qualCountIn      = 0;
    static ajint qualCountOut     = 0;
    static ajint qualCountFeat    = 0;
    static ajint qualCountFeatout = 0;

    static AjPStr qualName         = NULL;
    static AjPStr qualSeqFirst     = NULL;
    static AjPStr qualSeqoutFirst  = NULL;
    static AjPStr seqTypeIn        = NULL;
    static AjPStr qualFeatFirst    = NULL;
    static AjPStr qualFeatoutFirst = NULL;

    static AjBool seqMulti     = AJFALSE;
    static AjBool seqoutMulti  = AJFALSE;
    static AjBool featMulti    = AJFALSE;
    static AjBool featoutMulti = AJFALSE;
    const AjPStr namespace = NULL;
    AjPStr relation = NULL;
    AjPStrTok tokenhandle = NULL;
    ajuint ireldata = 0;

    if(!acdDoValid)
	return;

    if(acdEdam)
    {
        relstr = acdAttrValue(thys, "relations");
        tokenhandle = ajStrTokenNewC(relstr, "|");
        while(ajStrTokenNextParse(tokenhandle, &relation))
        {
            namespace = acdEdamTest(relation);
            if(!namespace)
                acdErrorValid("Qualifier relation '%S' not found",
                              relation);
            else if(ajStrMatchC(namespace, "data"))
                ireldata++;
            else if(ajStrMatchC(namespace, "format"))
                ireldata++;
            else if(ajStrMatchC(namespace, "identifier"))
                ireldata++;
            else
                acdErrorValid("Qualifier has relation '%S' with namespace '%S'",
                              relation, namespace);
        }

        ajStrDel(&relation);
        ajStrTokenDel(&tokenhandle);
    }

    if(ajListGetLength(acdSecList))
    {
	acdValidSectionFull(&secname);
    }
    else
    {
	acdErrorValid("No section defined for qualifier '%S'", thys->Name);
	ajStrAssignC(&secname, "<none>");
    }

    if (acdType[thys->Type].Section)
    {
	if (!acdValidSectionMatch(acdType[thys->Type].Section->Name))
	    acdErrorValid("Qualifier '%S' type '%s' not in section '%s'",
			  thys->Name, acdType[thys->Type].Name,
			  acdType[thys->Type].Section->Name);
    }

    if(ajCharMatchC(acdType[thys->Type].Name, "toggle"))
	isToggle = ajTrue;

    /* parameter, standard, additional only once, with 'Y' */

    itype = 0;

    tmpstr = acdAttrValue(thys, "parameter");

    if(ajStrGetLen(tmpstr))
    {
	itype++;

	if(acdVarTestValid(tmpstr, &toggle))
	{
	    isParameter = ajTrue;
	    acdErrorValid("Calculated parameter value for '%S'",
			  thys->Name);
	}
	else
	{
	    if(ajStrToBool(tmpstr,&isparam))
	    {
		if(isparam)
		{
		    isParameter = ajTrue;
		}
		else
		{
		    acdErrorValid("Parameter defined as false '%S'", tmpstr);
		    itype--;
		}
	    }
	}
    }

    tmpstr = acdAttrValue(thys, "standard");

    if(ajStrGetLen(tmpstr))
    {
	itype++;

	if(acdVarTestValid(tmpstr, &toggle))
	{
	    isStandard = ajTrue;

	    if (!toggle)
		acdWarn("Calculated standard value for '%S'",
			thys->Name);

	    if (acdValidSectionMatch("advanced"))
		acdWarn("Calculated standard value for qualifier '%S' "
			"in section '%S'",
			thys->Name, secname);
	}
	else
	{
	    if(ajStrToBool(tmpstr,&boolval))
	    {
		if(boolval)
		{
		    isStandard = ajTrue;
		}
		else
		{
		    acdErrorValid("Standard defined as false '%S'",
				  tmpstr);
		    itype--;
		}
	    }
	}
    }

    tmpstr = acdAttrValue(thys, "additional");

    if(ajStrGetLen(tmpstr))
    {
	itype++;
	if(acdVarTestValid(tmpstr, &toggle))
	{
	    isAdditional = ajTrue;

	    if (!toggle)
		acdWarn("Calculated additional value for '%S'",
			thys->Name);
	    if (acdValidSectionMatch("advanced"))
		acdWarn("Calculated additional value for qualifier '%S' "
			"in section '%S'",
			thys->Name, secname);
	}
	else
	{
	    if(ajStrToBool(tmpstr,&boolval))
	    {
		if(boolval)
		    isAdditional = ajTrue;
		else
		{
		    acdErrorValid("Additional defined as false '%S'",
				  tmpstr);
		    itype--;
		}
	    }
	}
    }

    if(itype > 1)
	acdErrorValid("Multiple definition of parameter/standard/additional");

    if (isParameter)
    {
		    if (acdValidSectionMatch("advanced") ||
			acdValidSectionMatch("additional"))
			acdErrorValid("Parameter '%S' in section '%S'",
				      thys->Name, secname);
    }

    else if (isStandard)
    {
	if (acdValidSectionMatch("advanced") ||
	    acdValidSectionMatch("additional"))
	    acdWarn("Standard qualifier '%S' "
		    "in section '%S'",
		    thys->Name, secname);
    }

    else if (isAdditional)
    {
	if (acdValidSectionMatch("advanced") ||
	    (!isToggle && acdValidSectionMatch("required")))
	    acdWarn("Additional qualifier '%S' "
		    "in section '%S'",
		    thys->Name, secname);
    }

    else				/* must be advanced */
    {
	if (!isToggle)
	{
	    if (acdValidSectionMatch("additional") ||
		acdValidSectionMatch("required"))
		acdWarn("Advanced qualifier '%S' "
			"in section '%S'",
			thys->Name, secname);
	}
    }

    tmpinfo = acdAttrValue(thys, "information");
    tmpprompt = acdAttrValue(thys, "prompt");
    tmphelp = acdAttrValue(thys, "help");

    if(acdType[thys->Type].Prompt)
    {
	tmpstandard = (*acdType[thys->Type].Prompt)(thys);

	if(ajStrGetLen(tmpstandard) &&
	   acdKnowntypeDesc(thys))
	{
	    if(ajStrGetLen(tmpinfo))
	    {
		if(!ajStrMatchS(tmpinfo, tmpstandard))
		    acdWarn("Information string for '%S' '%S' not standard '%S'",
			    thys->Name, tmpinfo, tmpstandard);
	    }
	    else
	    {
		    acdWarn("Missing standard information '%S' expected '%S'",
			    thys->Name, tmpstandard);
	    }
	}
/*
	else if(ajStrGetLen(tmpinfo))
	{
		acdWarn("Standard prompt for '%S' is '%S' information '%S'",
			thys->Name, tmpstandard, tmpinfo);
			
	}
*/
    }

    if(ajStrGetLen(tmpprompt) && !ajStrGetLen(tmpinfo))
	acdErrorValid("Prompt specified but no information");

    if(ajStrGetLen(tmpinfo) && !isupper((int)ajStrGetCharFirst(tmpinfo)))
    {
	if (islower((int)ajStrGetCharFirst(tmpinfo)))
	    acdWarn("Information string for '%S' starts in lower case",
		thys->Name);
	else
	    acdWarn("Information string for '%S' starts non-alphabetic",
		    thys->Name);
    }

    if(ajStrGetLen(tmpprompt) && !isupper((int)ajStrGetCharFirst(tmpprompt)))
    {
	if (islower((int)ajStrGetCharFirst(tmpprompt)))
	    acdWarn("Prompt string for '%S' starts in lower case",
		thys->Name);
	else
	    acdWarn("Prompt string for '%S' starts non-alphabetic",
		    thys->Name);
    }

    if(ajStrGetLen(tmphelp) && !isupper((int)ajStrGetCharFirst(tmphelp)))
    {
	if (islower((int)ajStrGetCharFirst(tmphelp)))
	    acdWarn("Help string for '%S' starts in lower case",
		thys->Name);
	else
	    acdWarn("Help string for '%S' starts non-alphabetic",
		    thys->Name);
    }

    /*
     ** if known, must have a standard info string
     ** but we can make allowances for possibly confusing 2nd occurrence
     ** and beyond
     ** and also allow for those with a default value that are usually not
     ** prompted for
     */

    if(acdType[thys->Type].Stdprompt &&
       (ajStrGetLen(tmpinfo) || ajStrGetLen(tmpprompt)) &&
       !acdAttrTestDefined(thys, "default") &&
       !acdAttrTestDefined(thys, "knowntype") &&
       *acdType[thys->Type].UseClassCount == 1)
    {
	acdWarn("Unexpected information value for type '%s'",
		acdType[thys->Type].Name);
    }

    /* else it must have an info attribute or a knowntype */

    if(!acdType[thys->Type].Stdprompt &&
       !ajStrGetLen(tmpinfo) &&
       !ajStrGetLen(tmpprompt) &&
       !acdAttrTestDefined(thys, "knowntype"))
    {
	acdErrorValid("Missing information value for type '%s'",
		      acdType[thys->Type].Name);
    }

    /* if known, must have a standard type */

    /* expected types should have a known name */
    /* sequence seqset seqall see below - need to do seqout, infile, outfile */

    if(ajCharMatchC(acdType[thys->Type].Name, "sequence") ||
       ajCharMatchC(acdType[thys->Type].Name, "seqall") ||
       ajCharMatchC(acdType[thys->Type].Name, "seqsetall") ||
       ajCharMatchC(acdType[thys->Type].Name, "seqset"))
    {
	if(!isparam &&
	   !acdAttrTestDefined(thys, "default") &&
	   !acdAttrTestDefined(thys, "nullok"))
	{
	    if(*acdType[thys->Type].UseClassCount == 1)
		acdErrorValid("First sequence input '%S' is not a parameter",
			      thys->Token);
	    else
		acdWarn("Subsequent sequence input '%S' is not a parameter",
			thys->Token);
	}

	qualCountIn++;
	qualCountSeq++;

	if(qualCountSeq == 1)
	    ajStrAssignS(&qualSeqFirst, thys->Token);
	ajFmtPrintS(&qualName, "%csequence",
		    (char) ('a' - 1 + qualCountSeq));

	if(!(ajStrSuffixC(thys->Token, "sequence") ||
	     (ajCharMatchC(acdType[thys->Type].Name, "seqall") &&
	      ajStrSuffixC(thys->Token, "seqall")) ||
	     (ajCharMatchC(acdType[thys->Type].Name, "seqset") &&
	      ajStrSuffixC(thys->Token, "sequences")) ||
	     (ajCharMatchC(acdType[thys->Type].Name, "seqsetall") &&
	      ajStrSuffixC(thys->Token, "sequences")) ))
	{
	    if(ajCharMatchC(acdType[thys->Type].Name, "seqall"))
		acdWarn("Sequence qualifier '%S' is not 'sequence' "
			"or '*sequence' or 'seqall' "
			"(should be '%Ssequence')",
			thys->Token, thys->Token);
	    else if(ajCharMatchC(acdType[thys->Type].Name, "seqset"))
		acdWarn("Sequence qualifier '%S' is not 'sequence' "
			"or '*sequence' or 'sequences' "
			"(should be '%Ssequence')",
			thys->Token, thys->Token);
	    else if(ajCharMatchC(acdType[thys->Type].Name, "seqsetall"))
		acdWarn("Sequence qualifier '%S' is not 'sequence' "
			"or '*sequence' or 'sequences' "
			"(should be '%Ssequence')",
			thys->Token, thys->Token);
	    else
		acdWarn("Sequence qualifier '%S' is not 'sequence' "
			"or '*sequence' "
			"(should be '%Ssequence')",
			thys->Token, thys->Token);
	}
	else
	{
	    if((qualCountSeq > 1) ||
	       !(ajStrMatchC(thys->Token, "sequence") ||
		 (ajCharMatchC(acdType[thys->Type].Name, "seqall") &&
		  ajStrSuffixC(thys->Token, "seqall")) ||
		 (ajCharMatchC(acdType[thys->Type].Name, "seqsetall") &&
		  ajStrSuffixC(thys->Token, "sequences")) ||
		 (ajCharMatchC(acdType[thys->Type].Name, "seqset") &&
		  ajStrSuffixC(thys->Token, "sequences")) ))
		seqMulti = ajTrue;

	    if(seqMulti)
            {
                if ((ajStrGetLen(thys->Token) == ajStrGetLen(qualName)) &&
		    ajStrSuffixC(thys->Token, "sequence"))
		{
		    if(!ajStrMatchS(thys->Token, qualName))
			acdWarn("Expected sequence qualifier is '%S' "
				"found '%S'",
				qualName, thys->Token);
		}
            }
	}
	
	tmpstr = acdAttrValue(thys, "type");
	if(!ajStrGetLen(tmpstr))
	    acdErrorValid("No type specified for input sequence");
	else
	    if(qualCountSeq == 1)
		ajStrAssignS(&seqTypeIn, tmpstr);

	if(ajCharMatchC(acdType[thys->Type].Name, "seqset") ||
	   ajCharMatchC(acdType[thys->Type].Name, "seqsetall"))
	{
	    if(!acdAttrTestDefined(thys, "aligned"))
	       acdErrorValid("Sequence set '%S' has no 'aligned' attribute",
			     thys->Token);
	}
    }

    if(ajCharMatchC(acdType[thys->Type].Name, "feature"))
    {
	if(!isparam && 
	   !acdAttrTestDefined(thys, "default") &&
	   !acdAttrTestDefined(thys, "nullok"))
	{
	    if(*acdType[thys->Type].UseClassCount == 1)
		acdErrorValid("First feature input '%S' is not a parameter",
			      thys->Token);
	    else
		acdWarn("Subsequent feature input '%S' is not a parameter",
			thys->Token);
	}

	qualCountFeat++;
	if(qualCountFeat == 1)
	    ajStrAssignS(&qualFeatFirst, thys->Token);
	ajFmtPrintS(&qualName, "%cfeature",
		    (char) ('a' - 1 + qualCountFeat));

	if(!(ajStrSuffixC(thys->Token, "feature")))
	{
	    acdWarn("Feature qualifier '%S' is not 'feature' "
			"or '*feature' "
			"(should be '%Sfeature')",
			thys->Token, thys->Token);
	}
	else
	{
	    if((qualCountFeat > 1) ||
	       !(ajStrMatchC(thys->Token, "feature")))
		featMulti = ajTrue;

	    if(featMulti)
		if ((ajStrGetLen(thys->Token) == ajStrGetLen(qualName)) &&
		    ajStrSuffixC(thys->Token, "feature"))
		{
		    if(!ajStrMatchS(thys->Token, qualName))
			acdWarn("Expected feature qualifier is '%S' "
				"found '%S'",
				qualName, thys->Token);
		}
	}
	
	/* still to do - check for type */

	tmpstr = acdAttrValue(thys, "type");
	if(!ajStrGetLen(tmpstr) && !ajStrGetLen(seqTypeIn))
	    acdErrorValid("No type specified for input feature, "
			  "and no input sequence type as a default");
    }

    /* infile - assume parameter  -infile */
    /* check for type */

    if(ajCharMatchC(acdType[thys->Type].Name, "infile") ||
       ajCharMatchC(acdType[thys->Type].Name, "filelist") ||
       ajCharMatchC(acdType[thys->Type].Name, "directory") ||
       ajCharMatchC(acdType[thys->Type].Name, "dirlist"))
    {
	if(!isparam && 
	   !acdAttrTestDefined(thys, "default") &&
	   !acdAttrTestDefined(thys, "nullok"))
	{
	    if(ajCharMatchC(acdType[thys->Type].Name, "directory") ||
	       ajCharMatchC(acdType[thys->Type].Name, "dirlist"))
	    {
		if(*acdType[thys->Type].UseClassCount == 1)
		    acdErrorValid("First input directory '%S' is not a "
				  "parameter",
				  thys->Token);
		else
		    acdWarn("Subsequent input directory '%S' is not a "
			    "parameter",
			    thys->Token);
	    }
	    else
	    {
		if(*acdType[thys->Type].UseClassCount == 1)
		    acdErrorValid("First input file '%S' is not a parameter",
				  thys->Token);
		else
		    acdWarn("Subsequent input file '%S' is not a parameter",
			    thys->Token);
	    }
	}

	qualCountIn++;
	qualCountInfile++;

	if(ajCharMatchC(acdType[thys->Type].Name, "infile"))
	{
	    if(!ajStrSuffixC(thys->Token, "file"))
		acdWarn("Infile qualifier '%S' is not 'infile' or '*file' "
			"(should be '%Sfile')",
			thys->Token, thys->Token);
	    else
	    {
		if((qualCountIn == 1) &&
		   !ajStrMatchC(thys->Token, "infile") &&
		   !ajStrSuffixC(thys->Token, "file"))
		    acdWarn("First input file qualifier '%S' is not "
			    "'infile' or '*file' "
			    "(should be '%Sfile')",
			    thys->Token, thys->Token);
	    }
	}
	else if(ajCharMatchC(acdType[thys->Type].Name, "filelist"))
	{
	    if(!ajStrSuffixC(thys->Token, "files"))
		acdWarn("Filelist qualifier '%S' is not '*files'",
			thys->Token);
	    /* no fixed qualifier name for first input filelist */
	}
	else if(ajCharMatchC(acdType[thys->Type].Name, "directory") ||
		ajCharMatchC(acdType[thys->Type].Name, "dirlist"))
	{
	    if(ajStrSuffixC(thys->Token, "outdir"))
		acdWarn("Directory qualifier '%S' has outdir style name "
			"'*outdir'",
			thys->Token);
	    else if(!ajStrSuffixC(thys->Token, "dir") &&
		    !ajStrSuffixC(thys->Token, "path") &&
		    !ajStrSuffixC(thys->Token, "directory"))
		acdWarn("Directory qualifier '%S' is not '*directory or *dir'"
			" or *path' "
			"(should be '%Sdir')",
			thys->Token, thys->Token);
	    /* no fixed qualifier name for first input directory */
	}

	tmpstr = acdAttrValue(thys, "knowntype");

	if(!ajStrGetLen(tmpstr))
	    acdWarn("No knowntype specified for input file");
    }

    /* datafile - no standard qualifier name yet */
    /* check for knowntype */

    if(ajCharMatchC(acdType[thys->Type].Name, "datafile"))
    {
	tmpstr = acdAttrValue(thys, "knowntype");

	if(!ajStrGetLen(tmpstr))
	    acdWarn("No knowntype specified for data file");
    }

    /* outfile - assume parameter unless default is stdout -outfile */
    /* check for type */

    if(ajCharMatchC(acdType[thys->Type].Name, "outfile"))
    {
	/* Skip this test - there is a good default for output files */

	/*
	if(!isparam && 
	   !acdAttrTestDefined(thys, "default") &&
	   !acdAttrTestDefined(thys, "nullok"))
	{
	    if(*acdType[thys->Type].UseClassCount == 1)
		acdErrorValid("First output file '%S' is not a parameter",
			      thys->Token);
	    else
		acdWarn("Subsequent output file '%S' is not a parameter",
			thys->Token);
	}
	*/
	
	qualCountOut++;
	qualCountOutfile++;

	if(ajCharMatchC(acdType[thys->Type].Name, "outfile") &&
	   !ajStrSuffixC(thys->Token, "file"))
	    acdWarn("Outfile qualifier '%S' is not 'outfile' or '*file' "
		    "(should be '%Sfile')",
		    thys->Token, thys->Token);
	else
	{
	    if((qualCountOut == 1) &&
	       !ajStrMatchC(thys->Token, "outfile"))
	    {
		acdWarn("First output file qualifier '%S' is not 'outfile'",
			thys->Token);
	    }
	}
	
	/* still to do - check for type */

	tmpstr = acdAttrValue(thys, "knowntype");

	if(!ajStrGetLen(tmpstr))
	    acdWarn("No knowntype specified for output file");
    }

    /* outdirectory - assume parameter unless default is stdout -outfile */
    /* check for type */

    if(ajCharMatchC(acdType[thys->Type].Name, "outdir"))
    {
	if(!isparam && 
	   !acdAttrTestDefined(thys, "default") &&
	   !acdAttrTestDefined(thys, "nullok"))
	{
	    if(*acdType[thys->Type].UseClassCount == 1)
		acdErrorValid("First output directory '%S' is not a parameter",
			      thys->Token);
	    else
		acdWarn("Subsequent output directory '%S' is not a parameter",
			thys->Token);
	}
	
	qualCountOut++;
	qualCountOutfile++;

	if(ajCharMatchC(acdType[thys->Type].Name, "outdir") &&
	   !ajStrSuffixC(thys->Token, "outdir"))
	    acdWarn("Outdir qualifier '%S' is not 'outdir' or '*outdir'",
		    thys->Token);
	else
	{
	    if((qualCountOut == 1) &&
	       !ajStrMatchC(thys->Token, "outdir") &&
	       !ajStrSuffixC(thys->Token, "outdir"))
		acdWarn("First output directory qualifier '%S' "
			"is not 'outdir' or '*outdir' "
			"(should be '%Sfile')",
			thys->Token, thys->Token);
	}
	
	/* still to do - check for type */

	tmpstr = acdAttrValue(thys, "knowntype");
	if(!ajStrGetLen(tmpstr))
	    acdWarn("No knowntype specified for output directory");
    }

    /* align - as for outfile? */

    if(ajCharMatchC(acdType[thys->Type].Name, "align"))
    {
	if(!isparam && 
	   !acdAttrTestDefined(thys, "default") &&
	   !acdAttrTestDefined(thys, "nullok"))
	{
	    if(*acdType[thys->Type].UseClassCount == 1)
		acdErrorValid("First alignment file '%S' is not a parameter",
			      thys->Token);
	    else
		acdWarn("Subsequent alignment file '%S' is not a parameter",
			thys->Token);
	}

	qualCountOut++;
	qualCountOutfile++;

	if(!ajStrSuffixC(thys->Token, "file"))
	    acdWarn("Align qualifier '%S' is not 'outfile' or '*file' "
		    "(should be '%Sfile')",
		    thys->Token, thys->Token);
	else
	{
	    if((qualCountOut == 1) &&
	       !ajStrMatchC(thys->Token, "outfile"))
		acdWarn("First alignment file qualifier '%S' is not 'outfile'",
			thys->Token);
	}
    }

    /* report - as for outfile? */

    if(ajCharMatchC(acdType[thys->Type].Name, "report"))
    {
	if(!isparam && 
	   !acdAttrTestDefined(thys, "default") &&
	   !acdAttrTestDefined(thys, "nullok"))
	{
	    if(*acdType[thys->Type].UseClassCount == 1)
		acdErrorValid("First report file '%S' is not a parameter",
			      thys->Token);
	    else
		acdWarn("Subsequent report file '%S' is not a parameter",
			thys->Token);
	}

	qualCountOut++;
	qualCountOutfile++;
	if(!ajStrSuffixC(thys->Token, "file"))
	    acdWarn("Report qualifier '%S' is not 'outfile' or '*file' "
		    "(should be '%Sfile')",
		    thys->Token, thys->Token);
	else
	{
	    if((qualCountOut == 1) &&
	       !ajStrMatchC(thys->Token, "outfile"))
		acdWarn("First report file qualifier '%S' is not 'outfile'",
			thys->Token);
	}	
    }

    /* seqout* - assume parameter - what names? -outseq? */
    /* type only if there is no sequence input */

    if(ajCharMatchC(acdType[thys->Type].Name, "seqout") ||
       ajCharMatchC(acdType[thys->Type].Name, "seqoutall") ||
       ajCharMatchC(acdType[thys->Type].Name, "seqoutset"))
    {
	/* skip this test - there is a good default for sequence output */

	/*
	if(!isparam && 
	   !acdAttrTestDefined(thys, "default") &&
	   !acdAttrTestDefined(thys, "nullok"))
	{
	    if(*acdType[thys->Type].UseClassCount == 1)
		acdErrorValid("First sequence output '%S' is not a parameter",
			      thys->Token);
	    else
		acdWarn("Subsequent sequence output '%S' is not a parameter",
			thys->Token);
	}
	*/

	qualCountOut++;
	qualCountSeqout++;

	if(qualCountSeqout == 1)
	    ajStrAssignS(&qualSeqoutFirst, thys->Token);
	ajFmtPrintS(&qualName, "%coutseq",
		    (char) ('a' - 1 + qualCountSeqout));

	if(!ajStrSuffixC(thys->Token, "outseq") &&
	   !ajStrSuffixC(thys->Token, "outfile"))
	    acdWarn("Sequence output qualifier '%S' is not 'outseq' "
		    "or '*outseq' or '*outfile'"
		    "(should be '%Soutseq')",
		    thys->Token, thys->Token);

	else
	{
	    if((qualCountOut > 1) ||
	       (!ajStrMatchC(thys->Token, "outseq") &&
		!ajStrMatchC(thys->Token, "outfile")))
		seqoutMulti = ajTrue;
	    if(seqoutMulti)
	    {
		if ((ajStrGetLen(thys->Token) == ajStrGetLen(qualName)) &&
		    ajStrSuffixC(thys->Token, "outseq"))
		{
		    if(!ajStrMatchS(thys->Token, qualName))
			acdWarn("Expected sequence output qualifier is '%S' "
				"found '%S'",
				qualName, thys->Token);
		}
	    }
	}
	
	/* still to do - check for type */

	tmpstr = acdAttrValue(thys, "type");

	if(!ajStrGetLen(tmpstr) && !ajStrGetLen(seqTypeIn))
	    acdErrorValid("No type specified for output sequence, "
			  "and no input sequence type as a default");
    }

    /* featout - assume parameter - what names? -outfeat? */
    /* type only if there is no sequence input */

    if(ajCharMatchC(acdType[thys->Type].Name, "featout"))
    {
	if(!isparam && 
	   !acdAttrTestDefined(thys, "default") &&
	   !acdAttrTestDefined(thys, "nullok"))
	{
	    if(*acdType[thys->Type].UseClassCount == 1)
		acdErrorValid("First feature output '%S' is not a parameter",
			      thys->Token);
	    else
		acdWarn("Subsequent feature output '%S' is not a parameter",
			thys->Token);
	}

	qualCountFeatout++;

	if(qualCountFeatout == 1)
	    ajStrAssignS(&qualFeatoutFirst, thys->Token);
	ajFmtPrintS(&qualName, "%coutfeat",
		    (char) ('a' - 1 + qualCountFeatout));

	if(!ajStrMatchC(thys->Token, "outfeat") &&
	   !ajStrSuffixC(thys->Token, "outfeat"))
	    acdWarn("Feature output qualifier '%S' is not 'outfeat' "
		    "or '*outfeat'"
		    "(should be '%Soutfeat')",
		    thys->Token, thys->Token);

	else
	{
	    if((qualCountFeatout > 1) ||
	       (!ajStrMatchC(thys->Token, "outfeat")))
		featoutMulti = ajTrue;

	    if(featoutMulti)
	    {
		if ((ajStrGetLen(thys->Token) == ajStrGetLen(qualName)) &&
		    ajStrSuffixC(thys->Token, "outfeat"))
		{
		    if(!ajStrMatchS(thys->Token, qualName))
			acdWarn("Expected feature output qualifier is '%S' "
				"found '%S'",
				qualName, thys->Token);
		}
	    }
	}
	
	/* still to do - check for type */

	tmpstr = acdAttrValue(thys, "type");

	if(!ajStrGetLen(tmpstr) && !ajStrGetLen(seqTypeIn))
	    acdErrorValid("No type specified for output feature, "
			  "and no input sequence type as a default");
    }

    /* string - we don't ask much, but we do prefer strings to have a
       known type that does not suggest some other datatype can be
       used */

    if(ajCharMatchC(acdType[thys->Type].Name, "string"))
    {
	tmpstr = acdAttrValue(thys, "knowntype");

	if(!ajStrGetLen(tmpstr))
	{
	    tmpstr = acdAttrValue(thys, "pattern");

	    if (!ajStrGetLen(tmpstr))
		acdWarn("No knowntype specified for string");
	    else
		acdWarn("Pattern but no knowntype specified for string");
	}
    }

    acdValidKnowntype(thys);

    return;
}




/* @funcstatic acdValidKnowntype **********************************************
**
** Validation for Known type
**
** @param [r] thys [const AcdPAcd] Current ACD object
** @return [void]
**
** @release 2.9.0
** @@
******************************************************************************/

static void acdValidKnowntype(const AcdPAcd thys)
{
    const AjPStr typestr         = NULL;
    const AjPStr acdKnownType    = NULL;
    static AjPStr defType = NULL;
    AjBool typeok = ajFalse;

    if(!acdDoValid)
	return;

    typestr = acdAttrValue(thys, "knowntype");

    if (!ajStrGetLen(typestr))
	return;


    if (!defType)
	ajFmtPrintS(&defType, "%S output", acdProgram);

    acdKnownType = ajTableFetchS(acdKnowntypeTypeTable, typestr);
    if (!acdKnownType)
    {
	if(ajStrFindAnyK(typestr, '_') >= 0)
	{
		acdWarn("Knowntype '%S' replace underscore(s) with spaces",
			typestr);
	}
	else
	{
	    if (!ajStrMatchS(typestr, defType))
		acdWarn("Knowntype '%S' not defined in knowntypes.standard",
			typestr);
	}

	return;
    }

    if (ajStrMatchC(acdKnownType, "file"))
    {
	if (!ajCharMatchC(acdType[thys->Type].Name, "infile") &&
	    !ajCharMatchC(acdType[thys->Type].Name, "datafile") &&
	    !ajCharMatchC(acdType[thys->Type].Name, "directory") &&
	    !ajCharMatchC(acdType[thys->Type].Name, "dirlist") &&
	    !ajCharMatchC(acdType[thys->Type].Name, "outfile") &&
	    !ajCharMatchC(acdType[thys->Type].Name, "outdir") &&
	    !ajCharMatchC(acdType[thys->Type].Name, "filelist"))
	{
	    acdWarn("Knowntype '%S' defined for type '%S', used for '%s'",
		    typestr, acdKnownType, acdType[thys->Type].Name);
	}
    }
    else if (ajStrMatchC(acdKnownType, "sequence"))
    {
	if (!ajCharMatchC(acdType[thys->Type].Name, "sequence") &&
	    !ajCharMatchC(acdType[thys->Type].Name, "seqall") &&
	    !ajCharMatchC(acdType[thys->Type].Name, "seqset") &&
	    !ajCharMatchC(acdType[thys->Type].Name, "seqsetall") &&
	    !ajCharMatchC(acdType[thys->Type].Name, "seqout") &&
	    !ajCharMatchC(acdType[thys->Type].Name, "seqoutall") &&
	    !ajCharMatchC(acdType[thys->Type].Name, "seqoutset") &&
	    !ajCharMatchC(acdType[thys->Type].Name, "seqoutsetall"))
	{
	    acdWarn("Knowntype '%S' defined for type '%S', used for '%s'",
		    typestr, acdKnownType, acdType[thys->Type].Name);
	}
    }
    else if (ajStrMatchC(acdKnownType, "codon") ||
	     ajStrMatchC(acdKnownType, "cpdb") ||
	     ajStrMatchC(acdKnownType, "matrix") ||
	     ajStrMatchC(acdKnownType, "discretestates") ||
	     ajStrMatchC(acdKnownType, "distances") ||
	     ajStrMatchC(acdKnownType, "frequencies") ||
	     ajStrMatchC(acdKnownType, "properties") ||
	     ajStrMatchC(acdKnownType, "scop") ||
	     ajStrMatchC(acdKnownType, "tree"))
    {
	typeok = ajFalse;
	if (ajStrMatchC(acdKnownType, "codon"))
	    if(ajCharMatchC(acdType[thys->Type].Name, "codon") ||
	       ajCharMatchC(acdType[thys->Type].Name, "outcodon"))
	       typeok = ajTrue;

	if (ajStrMatchC(acdKnownType, "cpdb"))
	    if(ajCharMatchC(acdType[thys->Type].Name, "cpdb") ||
	       ajCharMatchC(acdType[thys->Type].Name, "outcpdb"))
	       typeok = ajTrue;

	if (ajStrMatchC(acdKnownType, "discretestates"))
	    if(ajCharMatchC(acdType[thys->Type].Name, "discretestates") ||
	       ajCharMatchC(acdType[thys->Type].Name, "outdiscrete"))
	       typeok = ajTrue;

	if (ajStrMatchC(acdKnownType, "distances"))
	    if(ajCharMatchC(acdType[thys->Type].Name, "distances") ||
	       ajCharMatchC(acdType[thys->Type].Name, "outdistances"))
	       typeok = ajTrue;

	if (ajStrMatchC(acdKnownType, "frequencies"))
	    if(ajCharMatchC(acdType[thys->Type].Name, "frequencies") ||
	       ajCharMatchC(acdType[thys->Type].Name, "outfreq"))
	       typeok = ajTrue;

	if (ajStrMatchC(acdKnownType, "obo"))
	    if(ajCharMatchC(acdType[thys->Type].Name, "obo") ||
	       ajCharMatchC(acdType[thys->Type].Name, "outobo"))
	       typeok = ajTrue;

	if (ajStrMatchC(acdKnownType, "properties"))
	    if(ajCharMatchC(acdType[thys->Type].Name, "properties") ||
	       ajCharMatchC(acdType[thys->Type].Name, "outproperties"))
	       typeok = ajTrue;

	if (ajStrMatchC(acdKnownType, "resource"))
	    if(ajCharMatchC(acdType[thys->Type].Name, "resource") ||
	       ajCharMatchC(acdType[thys->Type].Name, "outresource"))
	       typeok = ajTrue;

	if (ajStrMatchC(acdKnownType, "scop"))
	    if(ajCharMatchC(acdType[thys->Type].Name, "scop") ||
	       ajCharMatchC(acdType[thys->Type].Name, "outscop"))
	       typeok = ajTrue;

	if (ajStrMatchC(acdKnownType, "tree"))
	    if(ajCharMatchC(acdType[thys->Type].Name, "tree") ||
	       ajCharMatchC(acdType[thys->Type].Name, "outtree"))
	       typeok = ajTrue;

	if (ajStrMatchC(acdKnownType, "matrix"))
	    if(ajCharMatchC(acdType[thys->Type].Name, "matrix") ||
	       ajCharMatchC(acdType[thys->Type].Name, "matrixf") ||
	       ajCharMatchC(acdType[thys->Type].Name, "outmatrix") ||
	       ajCharMatchC(acdType[thys->Type].Name, "outmatrixf"))
	       typeok = ajTrue;

	if (!ajCharMatchC(acdType[thys->Type].Name, "infile") &&
	    !ajCharMatchC(acdType[thys->Type].Name, "datafile") &&
	    !ajCharMatchC(acdType[thys->Type].Name, "directory") &&
	    !ajCharMatchC(acdType[thys->Type].Name, "dirlist") &&
	    !ajCharMatchC(acdType[thys->Type].Name, "outfile") &&
	    !ajCharMatchC(acdType[thys->Type].Name, "outdir") &&
	    !ajCharMatchC(acdType[thys->Type].Name, "filelist"))
	{
	    if(!typeok)
		acdWarn("Knowntype '%S' defined for type '%S', used for '%s'",
			typestr, acdKnownType, acdType[thys->Type].Name);
	}
    }
    else if (!ajStrMatchC(acdKnownType, acdType[thys->Type].Name))
    {
	acdWarn("Knowntype '%S' defined for type '%S', used for '%s'",
		typestr, acdKnownType, acdType[thys->Type].Name);
    }

    return;
}




/* @funcstatic acdKnowntypeDesc ***********************************************
**
** Return description of known type
**
** @param [r] thys [const AcdPAcd] Acd object
** @return [const AjPStr] Known type description or NULL if not defined.
**
** @release 4.0.0
** @@
******************************************************************************/

static const AjPStr acdKnowntypeDesc(const AcdPAcd thys)
{
    const AjPStr knowntype = NULL;
    const AjPStr knowndesc;

    knowntype = acdAttrValue(thys, "knowntype");

    if(!ajStrGetLen(knowntype))
        return NULL;

    knowndesc = ajTableFetchS(acdKnowntypeDescTable, knowntype);

    return knowndesc;
}




/* @funcstatic acdReadKnowntype ***********************************************
**
** Read standard file of ACD sections and store in new AjPTable objects
**
** @param [wN] desctable [AjPTable*] String table of section names and types
** @param [wN] typetable [AjPTable*] String table of section names and
**                                 descriptions
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

static void acdReadKnowntype(AjPTable* desctable, AjPTable* typetable)
{
    AjPFile knownFile    = NULL;
    AjPStr knownFName    = NULL;
    AjPStr knownRoot     = NULL;
    AjPStr knownRootInst = NULL;
    AjPStr knownPack     = NULL;
    AjPStr knownLine     = NULL;
    AjBool ok            = ajFalse;
    AjPStrTok handle     = NULL;
    AjPStr knownName     = NULL;
    AjPStr knownName2     = NULL;
    AjPStr knownType     = NULL;
    AjPStr knownDesc     = NULL;
    AjPStr knownEdam     = NULL;
    AjPStr knownRest     = NULL;
    ajint iline = 0;

    ajStrAssignS(&knownPack, ajNamValuePackage());
    ajStrAssignS(&knownRootInst, ajNamValueInstalldir());
    ajDirnameFix(&knownRootInst);
    
    *desctable = ajTablestrNewCase(2048);
    *typetable = ajTablestrNewCase(2048);

    if(ajNamGetValueC("acdroot", &knownRoot))
    {
	ajDirnameFix(&knownRoot);
	ajFmtPrintS(&knownFName, "%Sknowntypes.standard", knownRoot);
	knownFile = ajFileNewInNameS(knownFName);
	acdLog("Knowntypes file in acdroot: '%S'\n", knownFName);
    }
    else
    {
	ajFmtPrintS(&knownFName, "%Sshare/%S/acd/knowntypes.standard",
		    knownRootInst, knownPack);
	acdLog("Knowntypes file installed: '%S'\n", knownFName);
	knownFile = ajFileNewInNameS(knownFName);
	if(!knownFile)
	{
	    acdLog("Knowntypes file '%S' not opened\n", knownFName);
	    ajStrAssignS(&knownRoot, ajNamValueRootdir());
	    ajDirnameFix(&knownRoot);
	    ajFmtPrintS(&knownFName, "%Sacd/knowntypes.standard", knownRoot);
	    acdLog("Knowntypes file from source dir: '%S'\n", knownFName);
	    knownFile = ajFileNewInNameS(knownFName);
	}
    }
    
    if(!knownFile)			/* test acdc-knownmissing */
	ajDie("Knowntypes file %S not found", knownFName);
    else
	acdLog("Knowntypes file %F used\n", knownFile);
    
    while(knownFile && ajReadlineTrim(knownFile, &knownLine))
    {
	iline++;

	if(ajStrCutComments(&knownLine))
	{
            handle = ajStrTokenNewC(knownLine, "|");

            ok = ajStrTokenNextParse(handle, &knownName);

	    if(ok)
	      ok = ajStrTokenNextParse(handle, &knownType);
	    if(ok)
	      ok = ajStrTokenNextParse(handle, &knownEdam);
	    if(ok)
	      ajStrTokenRestParse(handle, &knownDesc);

            ajStrTokenDel(&handle);

	    if(ok)
	    {
                ajStrRemoveWhiteExcess(&knownName);
		ajStrExchangeKK(&knownName, '_', ' ');
                ajStrRemoveWhiteExcess(&knownType);
		if(ajStrMatchCaseC(knownType, "infile") ||
		   ajStrMatchCaseC(knownType, "filelist") ||
		   ajStrMatchCaseC(knownType, "datafile") ||
		   ajStrMatchCaseC(knownType, "outfile"))
		{
		    ajWarn("Knowntype '%S' in file %S line %d - use 'file'",
			   knownType, knownFName, iline);
		}
		if(ajTablePut(*typetable, knownName, knownType))
		    ajWarn("Duplicate knowntype name '%S' in file %S line %d",
			   knownName, knownFName, iline);
		ajStrAssignS(&knownName2, knownName);
                ajStrRemoveWhiteExcess(&knownEdam);
                ajStrQuoteStrip(&knownEdam);
                if(ajStrPrefixC(knownEdam, "EDAM_data: "))
		{
                    ajStrCutStart(&knownEdam, 11);
		}
                else if(ajStrPrefixC(knownEdam, "EDAM_data:"))
		{
                    ajStrCutStart(&knownEdam, 14);
		}
                else if(ajStrGetLen(knownEdam))
		    ajWarn("Knowntype '%S' in file %S line %d - EDAM badly formatted '%S'",
			   knownType, knownFName, iline, knownEdam);
                ajStrRemoveWhiteExcess(&knownDesc);
		ajTablePut(*desctable, knownName2, knownDesc);

/*                ajUser("name '%S' type '%S' desc '%S' edam '%S'",
                  knownName, knownType, knownDesc, knownEdam);*/
	        knownName = NULL;
	        knownName2 = NULL;
		knownType = NULL;
		knownDesc = NULL;
	    }
	    else
		ajErr("Bad record in file %S:\n%S",
		      knownFName, knownLine);
	}
    }

    ajFileClose(&knownFile);
    ajStrDel(&knownFName);
    ajStrDel(&knownRoot);
    ajStrDel(&knownRootInst);
    ajStrDel(&knownPack);
    ajStrDel(&knownLine);
    ajStrDel(&knownName);
    ajStrDel(&knownType);
    ajStrDel(&knownEdam);
    ajStrDel(&knownDesc);
    ajStrDel(&knownRest);

    return;
}




/* @funcstatic acdValidApplGroup **********************************************
**
** Validation for application groups
**
** @param [r] groups [const AjPStr] Group name(s)
** @return [void]
**
** @release 2.7.0
** @@
******************************************************************************/

static void acdValidApplGroup(const AjPStr groups)
{
    AjPRegexp grpexp         = NULL;
    AjPStr tmpGroups         = NULL;
    AjPStr grpName           = NULL;
    const AjPStr grpDesc     = NULL;

    if(!acdDoValid)
	return;

    acdGrpTable = acdReadGroups();

    ajStrAssignS(&tmpGroups, groups);

    /* step through each group */
    grpexp = ajRegCompC("([^,|]+),?");

    while(ajRegExec(grpexp, tmpGroups))
    {
	ajRegSubI(grpexp, 1, &grpName);
	ajStrRemoveWhiteExcess(&grpName);
	grpDesc = ajTableFetchS(acdGrpTable, grpName);

	if(!grpDesc)
	    acdErrorValid("Unknown group '%S' for application", grpName);

	ajRegPost(grpexp, &tmpGroups);
    }

    ajStrDel(&grpName);
    ajStrDel(&tmpGroups);
    ajRegFree(&grpexp);

    return;
}




/* @funcstatic acdValidApplKeywords *******************************************
**
** Validation for application keywords
**
** @param [r] keys [const AjPStr] Keyword(s)
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

static void acdValidApplKeywords(const AjPStr keys)
{
    static AjPTable keyTable = NULL;
    AjPRegexp keyexp         = NULL;
    AjPStr tmpKeys           = NULL;
    AjPStr keyName           = NULL;
    const AjPStr keyDesc     = NULL;

    if(!acdDoValid)
	return;

    keyTable = acdReadKeywords();

    ajStrAssignS(&tmpKeys, keys);

    /* step through each group */
    keyexp = ajRegCompC("([^,|]+),?");

    while(ajRegExec(keyexp, tmpKeys))
    {
	ajRegSubI(keyexp, 1, &keyName);
	ajStrRemoveWhiteExcess(&keyName);
	keyDesc = ajTableFetchS(keyTable, keyName);

	if(!keyDesc)
	    acdErrorValid("Unknown keyword '%S' for application", keyName);

	ajRegPost(keyexp, &tmpKeys);
    }

    ajStrDel(&keyName);
    ajStrDel(&tmpKeys);
    ajRegFree(&keyexp);

    return;
}




/* @funcstatic acdReadGroups **************************************************
**
** Read standard table of application groups
**
** @return [AjPTable] String table of group names and descriptions
**
** @release 2.7.0
** @@
******************************************************************************/

static AjPTable acdReadGroups(void)
{
    AjPTable ret = ajTablestrNewCase(50);

    AjPFile grpFile    = NULL;
    AjPStr grpFName    = NULL;
    AjPStr grpRoot     = NULL;
    AjPStr grpRootInst = NULL;
    AjPStr grpPack     = NULL;
    AjPStr grpLine     = NULL;
    AjPRegexp grpxp    = NULL;
    AjPStr grpName     = NULL;
    AjPStr grpDesc     = NULL;

    ajStrAssignS(&grpPack, ajNamValuePackage());
    ajStrAssignS(&grpRootInst, ajNamValueInstalldir());
    ajDirnameFix(&grpRootInst);
    
    if(ajNamGetValueC("acdroot", &grpRoot))
    {
	ajDirnameFix(&grpRoot);
	ajFmtPrintS(&grpFName, "%Sgroups.standard", grpRoot);
	grpFile = ajFileNewInNameS(grpFName);
	acdLog("Group file in acdroot: '%S'\n", grpFName);
    }
    else
    {
	ajFmtPrintS(&grpFName, "%Sshare/%S/acd/groups.standard",
		    grpRootInst, grpPack);
	acdLog("Group file installed: '%S'\n", grpFName);
	grpFile = ajFileNewInNameS(grpFName);

	if(!grpFile)
	{
	    acdLog("Grp file '%S' not opened\n", grpFName);
	    ajStrAssignS(&grpRoot, ajNamValueRootdir());
	    ajDirnameFix(&grpRoot);
	    ajFmtPrintS(&grpFName, "%Sacd/groups.standard", grpRoot);
	    acdLog("Grp file from source dir: '%S'\n", grpFName);
	    grpFile = ajFileNewInNameS(grpFName);
	}
    }
    
    if(!grpFile)			/* test acdc-grpmissing */
	ajDie("Group file %S not found", grpFName);
    else
	acdLog("Group file %F used\n", grpFile);
    
    grpxp = ajRegCompC("^EDAM_[a-z]+:\\d+\\s+([^ ]+) +([^ ].*)");

    while(grpFile && ajReadlineTrim(grpFile, &grpLine))
    {
	if(ajStrCutComments(&grpLine))
	{
	    ajStrRemoveWhiteExcess(&grpLine);

	    if(ajRegExec(grpxp, grpLine))
	    {
		ajRegSubI(grpxp, 1, &grpName);
		ajRegSubI(grpxp, 2, &grpDesc);
		ajStrExchangeKK(&grpName, '_', ' ');

		if(ajTablePut(ret, grpName, grpDesc))
		    ajWarn("Duplicate group name '%S' in file %S",
			   grpName, grpFName);
		grpName = NULL;
		grpDesc = NULL;
	    }
	    else
		ajErr("Bad record in file %S:\n%S",
		      grpFName, grpLine);
	}
    }
    ajFileClose(&grpFile);

    ajStrDel(&grpFName);
    ajStrDel(&grpRoot);
    ajStrDel(&grpRootInst);
    ajStrDel(&grpPack);
    ajStrDel(&grpLine);
    ajStrDel(&grpName);
    ajStrDel(&grpDesc);

    ajRegFree(&grpxp);

    return ret;
}




/* @funcstatic acdReadKeywords ************************************************
**
** Read standard table of application keywords
**
** @return [AjPTable] String table of keyword names and descriptions
**
** @release 4.0.0
** @@
******************************************************************************/

static AjPTable acdReadKeywords(void)
{
    AjPTable ret = ajTablestrNewCase(50);

    AjPFile keyFile    = NULL;
    AjPStr keyFName    = NULL;
    AjPStr keyRoot     = NULL;
    AjPStr keyRootInst = NULL;
    AjPStr keyPack     = NULL;
    AjPStr keyLine     = NULL;
    AjPRegexp keyxp    = NULL;
    AjPStr keyName     = NULL;
    AjPStr keyDesc     = NULL;

    if(!acdGrpTable)
	acdGrpTable = acdReadGroups();

    ajStrAssignS(&keyPack, ajNamValuePackage());
    ajStrAssignS(&keyRootInst, ajNamValueInstalldir());
    ajDirnameFix(&keyRootInst);
    
    if(ajNamGetValueC("acdroot", &keyRoot))
    {
	ajDirnameFix(&keyRoot);
	ajFmtPrintS(&keyFName, "%Skeywords.standard", keyRoot);
	keyFile = ajFileNewInNameS(keyFName);
	acdLog("Keyword file in acdroot: '%S'\n", keyFName);
    }
    else
    {
	ajFmtPrintS(&keyFName, "%Sshare/%S/acd/keywords.standard",
		    keyRootInst, keyPack);
	acdLog("Keyword file installed: '%S'\n", keyFName);
	keyFile = ajFileNewInNameS(keyFName);

	if(!keyFile)
	{
	    acdLog("keyword file '%S' not opened\n", keyFName);
	    ajStrAssignS(&keyRoot, ajNamValueRootdir());
	    ajDirnameFix(&keyRoot);
	    ajFmtPrintS(&keyFName, "%Sacd/keywords.standard", keyRoot);
	    acdLog("Keywords file from source dir: '%S'\n", keyFName);
	    keyFile = ajFileNewInNameS(keyFName);
	}
    }
    
    if(!keyFile)			/* test acdc-keymissing */
	ajDie("Keyword file %S not found", keyFName);
    else
	acdLog("Keyword file %F used\n", keyFile);
    
    keyxp = ajRegCompC("([^ ]+) +([^ ].*)");
    while(keyFile && ajReadlineTrim(keyFile, &keyLine))
    {
	if(ajStrCutComments(&keyLine))
	{
	    ajStrRemoveWhiteExcess(&keyLine);

	    if(ajRegExec(keyxp, keyLine))
	    {
		ajRegSubI(keyxp, 1, &keyName);
		ajRegSubI(keyxp, 2, &keyDesc);
		ajStrExchangeKK(&keyName, '_', ' ');

		if(ajTableFetchS(acdGrpTable, keyName))
		    ajWarn("Keyword %S in file %S is a known group",
			   keyName, keyFName);
		if(ajTablePut(ret, keyName, keyDesc))
		    ajWarn("Duplicate keyword in file %S",
			   keyFName);
		keyName = NULL;
		keyDesc = NULL;
	    }
	    else
		ajErr("Bad record in file %S:\n%S",
		      keyFName, keyLine);
	}
    }

    ajFileClose(&keyFile);

    ajStrDel(&keyFName);
    ajStrDel(&keyRoot);
    ajStrDel(&keyRootInst);
    ajStrDel(&keyPack);
    ajStrDel(&keyLine);
    ajStrDel(&keyName);
    ajStrDel(&keyDesc);

    ajRegFree(&keyxp);

    return ret;
}




/* @funcstatic acdReadSections ************************************************
**
** Read standard file of ACD sections and store in AjPTable objects
**
** @param [wN] typetable [AjPTable*] String table of section names and types
** @param [wN] infotable [AjPTable*] String table of section names and
**                                 descriptions
** @return [void]
**
** @release 2.8.0
** @@
******************************************************************************/

static void acdReadSections(AjPTable* typetable, AjPTable* infotable)
{
    AjPFile sectFile    = NULL;
    AjPStr sectFName    = NULL;
    AjPStr sectRoot     = NULL;
    AjPStr sectRootInst = NULL;
    AjPStr sectPack     = NULL;
    AjPStr sectLine     = NULL;
    AjPRegexp sectxp    = NULL;
    AjPStr sectName     = NULL;
    AjPStr sectType     = NULL;
    AjPStr sectDesc     = NULL;

    ajStrAssignS(&sectPack, ajNamValuePackage());
    ajStrAssignS(&sectRootInst, ajNamValueInstalldir());
    ajDirnameFix(&sectRootInst);
    
    *typetable = ajTablestrNewCase(50);
    *infotable = ajTablestrNewCase(50);

    if(ajNamGetValueC("acdroot", &sectRoot))
    {
	ajDirnameFix(&sectRoot);
	ajFmtPrintS(&sectFName, "%Ssections.standard", sectRoot);
	sectFile = ajFileNewInNameS(sectFName);
	acdLog("Section file in acdroot: '%S'\n", sectFName);
    }
    else
    {
	ajFmtPrintS(&sectFName, "%Sshare/%S/acd/sections.standard",
		    sectRootInst, sectPack);
	acdLog("Section file installed: '%S'\n", sectFName);
	sectFile = ajFileNewInNameS(sectFName);

	if(!sectFile)
	{
	    acdLog("Sect file '%S' not opened\n", sectFName);
	    ajStrAssignS(&sectRoot, ajNamValueRootdir());
	    ajDirnameFix(&sectRoot);
	    ajFmtPrintS(&sectFName, "%Sacd/sections.standard", sectRoot);
	    acdLog("Sect file from source dir: '%S'\n", sectFName);
	    sectFile = ajFileNewInNameS(sectFName);
	}
    }
    
    if(!sectFile)			/* test acdc-sectmissing */
	ajDie("Section file %S not found", sectFName);
    else
	acdLog("Section file %F used\n", sectFile);
    
    sectxp = ajRegCompC("([^ ]+) +([^ ]+) +([^ ].*)");

    while(sectFile && ajReadlineTrim(sectFile, &sectLine))
    {
	if(ajStrCutComments(&sectLine))
	{
	    ajStrRemoveWhiteExcess(&sectLine);

	    if(ajRegExec(sectxp, sectLine))
	    {
		ajRegSubI(sectxp, 1, &sectName);
		ajRegSubI(sectxp, 2, &sectType);
		ajRegSubI(sectxp, 3, &sectDesc);
		ajStrExchangeKK(&sectName, '_', ' ');

		if(ajTablePut(*typetable, sectName, sectType))
		    ajWarn("Duplicate section name in file %S",
			   sectFName);

		if(ajTablePut(*infotable, sectName, sectDesc))
		    ajWarn("Duplicate section name in file %S",
			   sectFName);
		sectName = NULL;
		sectType = NULL;
		sectDesc = NULL;
	    }
	    else
		ajErr("Bad record in file %S:\n%S",
		      sectFName, sectLine);
	}
    }

    ajFileClose(&sectFile);
    ajStrDel(&sectFName);
    ajStrDel(&sectRoot);
    ajStrDel(&sectRootInst);
    ajStrDel(&sectPack);
    ajStrDel(&sectLine);
    ajStrDel(&sectName);
    ajStrDel(&sectType);
    ajStrDel(&sectDesc);
    ajRegFree(&sectxp);


    return;
}




/* @funcstatic acdValidSectionMatch *******************************************
**
** Tests whether a named section appears in the current section list
**
** @param [r] secname [const char*] Section name
** @return [AjBool] ajTrue if the named section was found
**
** @release 2.9.0
** @@
******************************************************************************/

static AjBool acdValidSectionMatch(const char* secname)
{
    AjIList iter = NULL;
    AjPStr listsecname;
    AjBool ret = ajFalse;

    if(!ajListGetLength(acdSecList))
	return ajFalse;

    iter = ajListIterNewread(acdSecList);

    while (!ajListIterDone(iter))
    {
	listsecname = ajListIterGet(iter);

	if(ajStrMatchC(listsecname, secname))
	{
	    ret = ajTrue;
	    break;
	}
    }

    ajListIterDel(&iter);

    return ret;
}




/* @funcstatic acdValidSectionFull ********************************************
**
** Returns the full section name with the top level and any frame
** sub-sections
**
** @param [w] secname [AjPStr*] Section name
** @return [void]
**
** @release 2.9.0
******************************************************************************/

static void acdValidSectionFull(AjPStr* secname)
{
    AjIList iter = NULL;
    AjPStr listsecname;

    ajStrAssignClear(secname);

    if(!ajListGetLength(acdSecList))
	return;

    iter = ajListIterNewread(acdSecList);

    while (!ajListIterDone(iter))
    {
	listsecname = ajListIterGet(iter);

	if (ajStrGetLen(*secname))
	    ajStrAppendK(secname,':');

	ajStrAppendS(secname, listsecname);
    }

    ajListIterDel(&iter);

    return;
}




/* @funcstatic acdOutFormatCpdb ***********************************************
**
** Tests the output format for an outcpdb ACD type
**
** @param [r] name [const AjPStr] Format name
** @param [w] iformat [ajint*] Index
** @return [AjBool] ajTrue on success.
**
** @release 2.9.0
** @@
******************************************************************************/

static AjBool acdOutFormatCpdb(const AjPStr name, ajint* iformat)
{
    ajint i;
    const char* format[] = 
    {
	"cpdb",
	NULL
    };

    for (i=0; format[i]; i++)
	if(ajStrMatchCaseC(name, format[i]))
        {
            *iformat = i;
            return ajTrue;
        }

    return ajFalse;
}




/* @funcstatic acdOutFormatData ***********************************************
**
** Tests the output format for an out ACD type
**
** @param [r] name [const AjPStr] Format name
** @param [w] iformat [ajint*] Index
** @return [AjBool] ajTrue on success.
**
** @release 2.9.0
** @@
******************************************************************************/

static AjBool acdOutFormatData(const AjPStr name, ajint* iformat)
{
    ajint i;
    const char* format[] = 
    {
	"text",
	NULL
    };

    for (i=0; format[i]; i++)
	if(ajStrMatchCaseC(name, format[i]))
        {
            *iformat = i;
            return ajTrue;
        }

    return ajFalse;
}




/* @funcstatic acdOutFormatDiscrete *******************************************
**
** Tests the output format for an out ACD type
**
** @param [r] name [const AjPStr] Format name
** @param [w] iformat [ajint*] Index
** @return [AjBool] ajTrue on success.
**
** @release 2.9.0
** @@
******************************************************************************/

static AjBool acdOutFormatDiscrete(const AjPStr name, ajint* iformat)
{
    ajint i;
    const char* format[] = 
    {
	"phylip",
	NULL
    };

    for (i=0; format[i]; i++)
	if(ajStrMatchCaseC(name, format[i]))
        {
            *iformat = i;
            return ajTrue;
        }

    return ajFalse;
}




/* @funcstatic acdOutFormatDistance *******************************************
**
** Tests the output format for an outdistance ACD type
**
** @param [r] name [const AjPStr] Format name
** @param [w] iformat [ajint*] Index
** @return [AjBool] ajTrue on success.
**
** @release 2.9.0
** @@
******************************************************************************/

static AjBool acdOutFormatDistance(const AjPStr name, ajint* iformat)
{
    ajint i;
    const char* format[] = 
    {
	"phylip",
	NULL
    };

   for (i=0; format[i]; i++)
	if(ajStrMatchCaseC(name, format[i]))
        {
            *iformat = i;
            return ajTrue;
        }

    return ajFalse;
}




/* @funcstatic acdOutFormatFreq ***********************************************
**
** Tests the output format for an outfreq ACD type
**
** @param [r] name [const AjPStr] Format name
** @param [w] iformat [ajint*] Index
** @return [AjBool] ajTrue on success.
**
** @release 2.9.0
** @@
******************************************************************************/

static AjBool acdOutFormatFreq(const AjPStr name, ajint* iformat)
{
    ajint i;
    const char* format[] = 
    {
	"phylip",
	NULL
    };

   for (i=0; format[i]; i++)
	if(ajStrMatchCaseC(name, format[i]))
        {
            *iformat = i;
            return ajTrue;
        }

    return ajFalse;
}




/* @funcstatic acdOutFormatMatrix *********************************************
**
** Tests the output format for an outmatrix ACD type
**
** @param [r] name [const AjPStr] Format name
** @param [w] iformat [ajint*] Index
** @return [AjBool] ajTrue on success.
**
** @release 2.9.0
** @@
******************************************************************************/

static AjBool acdOutFormatMatrix(const AjPStr name, ajint* iformat)
{
    ajint i;
    const char* format[] = 
    {
	"emboss",
	NULL
    };

   for (i=0; format[i]; i++)
	if(ajStrMatchCaseC(name, format[i]))
        {
            *iformat = i;
            return ajTrue;
        }

    return ajFalse;
}




/* @funcstatic acdOutFormatMatrixf ********************************************
**
** Tests the output format for an outmatrixf ACD type
**
** @param [r] name [const AjPStr] Format name
** @param [w] iformat [ajint*] Index
** @return [AjBool] ajTrue on success.
**
** @release 2.9.0
** @@
******************************************************************************/

static AjBool acdOutFormatMatrixf(const AjPStr name, ajint* iformat)
{
    ajint i;
    const char* format[] = 
    {
	"emboss",
	NULL
    };

   for (i=0; format[i]; i++)
	if(ajStrMatchCaseC(name, format[i]))
        {
            *iformat = i;
            return ajTrue;
        }

    return ajFalse;
}




/* @funcstatic acdOutFormatProperties *****************************************
**
** Tests the output format for an outproperties ACD type
**
** @param [r] name [const AjPStr] Format name
** @param [w] iformat [ajint*] Index
** @return [AjBool] ajTrue on success.
**
** @release 2.9.0
** @@
******************************************************************************/

static AjBool acdOutFormatProperties(const AjPStr name, ajint* iformat)
{
    ajint i;
    const char* format[] = 
    {
	"phylip",
	NULL
    };

   for (i=0; format[i]; i++)
	if(ajStrMatchCaseC(name, format[i]))
        {
            *iformat = i;
            return ajTrue;
        }

    return ajFalse;
}




/* @funcstatic acdOutFormatScop ***********************************************
**
** Tests the output format for an outscop ACD type
**
** @param [r] name [const AjPStr] Format name
** @param [w] iformat [ajint*] Index
** @return [AjBool] ajTrue on success.
**
** @release 2.9.0
** @@
******************************************************************************/

static AjBool acdOutFormatScop(const AjPStr name, ajint* iformat)
{
    ajint i;
    const char* format[] = 
    {
	"scop",
	NULL
    };

   for (i=0; format[i]; i++)
	if(ajStrMatchCaseC(name, format[i]))
        {
            *iformat = i;
            return ajTrue;
        }

    return ajFalse;
}




/* @funcstatic acdOutFormatTree ***********************************************
**
** Tests the output format for an outtree ACD type
**
** @param [r] name [const AjPStr] Format name
** @param [w] iformat [ajint*] Index
** @return [AjBool] ajTrue on success.
**
** @release 2.9.0
** @@
******************************************************************************/

static AjBool acdOutFormatTree(const AjPStr name, ajint* iformat)
{
    ajint i;
    const char* format[] = 
    {
	"phylip",
	"newick",
	NULL
    };

   for (i=0; format[i]; i++)
	if(ajStrMatchCaseC(name, format[i]))
        {
            *iformat = i;
            return ajTrue;
        }

    return ajFalse;
}




/* @funcstatic acdKnownValueList **********************************************
**
** Finds a list value associated with a known type
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [w] value [AjPStr*] Standard value
** @return [AjBool] ajTrue if a value was set
**
** @release 3.0.0
** @@
******************************************************************************/

static AjBool acdKnownValueList(const AcdPAcd thys, AjPStr* value)
{
    const AjPStr type = NULL;
    AjPStr resource = NULL;
    AjPStr list = NULL;
    ajint i;

    type = acdAttrValue(thys, "knowntype");

    if(!type)
	return ajFalse;

    for(i=0; acdResource[i]; i++)
	if(ajStrMatchCaseC(type, acdResource[i]))
	{
	    resource = ajStrNewS(type);
	    ajStrExchangeKK(&resource, ' ', '_');

	    if(ajNamRsListValue(resource, &list))
	    {
		if(acdResourceList(thys, list, value))
		    return ajTrue;
	    }
	}

    return ajFalse;
}




/* @funcstatic acdKnownValueSelect ********************************************
**
** Finds a selection value associated with a known type
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [w] value [AjPStr*] Standard value
** @return [AjBool] ajTrue if a value was set
**
** @release 3.0.0
** @@
******************************************************************************/

static AjBool acdKnownValueSelect(const AcdPAcd thys, AjPStr* value)
{
    const AjPStr type = NULL;
    AjPStr resource = NULL;
    AjPStr list = NULL;
    ajint i;

    type = acdAttrValue(thys, "knowntype");
    if(!type)
	return ajFalse;

    for(i=0; acdResource[i]; i++)
	if(ajStrMatchCaseC(type, acdResource[i]))
	{
	    resource = ajStrNewS(type);
	    ajStrExchangeKK(&resource, ' ', '_');

	    if(ajNamRsListValue(resource, &list))
	    {
		if(acdResourceList(thys, list, value))
		    return ajTrue;
	    }
	}

    return ajFalse;
}




/* @funcstatic acdResourceList ************************************************
**
** Finds a list value associated with a known type
**
** @param [r] thys [const AcdPAcd] ACD object
** @param [r] list [const AjPStr] Resource value
** @param [w] value [AjPStr*] Standard list value
** @return [AjBool] ajTrue if a value was set
**
** @release 3.0.0
** @@
******************************************************************************/

static AjBool acdResourceList(const AcdPAcd thys,
			      const AjPStr list, AjPStr* value)
{
    AjPStr delim = NULL;
    AjPStr codedelim = NULL;
    AjPFile infile = NULL;
    AjPStr line = NULL;
    AjPStr tok1 = NULL;
    AjPStr tok2 = NULL;
    AjPStrTok handle = NULL;
    AjPStr liststr = NULL;

    if (!ajStrGetLen(list))
	return ajFalse;

    liststr = ajStrNewS(list);

    acdAttrValueStr(thys, "delimiter", ";", &delim);
    acdAttrValueStr(thys, "codedelimiter", ":", &codedelim);

    if(ajStrGetCharFirst(list) == '@')
    {
	ajStrAssignClear(value);
	ajStrCutStart(&liststr, 1);
	infile = ajDatafileNewInNameS(liststr);

	if(!infile)
	    return ajFalse;

	while(ajReadlineTrim(infile, &line))
	{
	    ajStrTrimWhite(&line);

	    if(ajStrGetCharFirst(line) == '#')
		continue;
	    ajStrTokenAssignC(&handle, line, " ");
	    ajStrTokenNextParse(handle, &tok1);
	    ajStrTokenRestParse(handle, &tok2);

	    if(ajStrGetLen(*value))
		ajStrAppendS(value, delim);

	    ajStrAppendS(value, tok1);
	    ajStrAppendS(value, codedelim);
	    ajStrAppendS(value, tok2);
	}

	ajFileClose(&infile);

	return ajTrue;
    }

    /* value will use : and ; as delimiters */

    ajStrAssignS(value, liststr);
    ajStrExchangeKK(value, ';', '\1');
    ajStrExchangeKK(value, ':', '\2');
    ajStrExchangeKK(value, '\1', ajStrGetCharFirst(delim));
    ajStrExchangeKK(value, '\2', ajStrGetCharFirst(codedelim));

    return ajTrue;
}




/* @funcstatic acdDelAlign ****************************************************
**
** Function with void** prototype to delete ACD alignment data
**
** @param [d] PPval [void**] Value to be deleted
** @return [void]
**
** @release 4.0.0
******************************************************************************/

static void acdDelAlign(void** PPval)
{
    if(!*PPval)
        return;

    ajAlignDel((AjPAlign*)PPval);

    return;
}




/* @funcstatic acdDelAssembly *************************************************
**
** Function with void** prototype to delete ACD assembly input data
**
** @param [d] PPval [void**] Value to be deleted
** @return [void]
**
** @release 6.5.0
**
******************************************************************************/

static void acdDelAssembly(void** PPval)
{
    if(!*PPval)
        return;

    ajAssemDel((AjPAssem*)PPval);

    return;
}




/* @funcstatic acdDelCod ******************************************************
**
** Function with void** prototype to delete ACD codon usage data
**
** @param [d] PPval [void**] Value to be deleted
** @return [void]
**
** @release 4.0.0
******************************************************************************/

static void acdDelCod(void** PPval)
{
    if(!*PPval)
        return;

    ajCodDel((AjPCod*)PPval);

    return;
}




/* @funcstatic acdDelDir ******************************************************
**
** Function with void** prototype to delete ACD directory data
**
** @param [d] PPval [void**] Value to be deleted
** @return [void]
**
** @release 4.0.0
******************************************************************************/

static void acdDelDir(void** PPval)
{
    if(!*PPval)
        return;

    ajDirDel((AjPDir*)PPval);

    return;
}




/* @funcstatic acdDelDirout ***************************************************
**
** Function with void** prototype to delete ACD directory data
**
** @param [d] PPval [void**] Value to be deleted
** @return [void]
**
** @release 6.0.0
******************************************************************************/

static void acdDelDirout(void** PPval)
{
    if(!*PPval)
        return;

    ajDiroutDel((AjPDirout*)PPval);

    return;
}




/* @funcstatic acdDelFeattable ************************************************
**
** Function with void** prototype to delete ACD feature table data
**
** @param [d] PPval [void**] Value to be deleted
** @return [void]
**
** @release 4.0.0
******************************************************************************/

static void acdDelFeattable(void** PPval)
{
    if(!*PPval)
        return;

    ajFeattaballDel((AjPFeattaball*)PPval);

    return;
}




/* @funcstatic acdDelFeattabOut ***********************************************
**
** Function with void** prototype to delete ACD output feature table data
**
** @param [d] PPval [void**] Value to be deleted
** @return [void]
**
** @release 4.0.0
******************************************************************************/

static void acdDelFeattabOut(void** PPval)
{
    if(!*PPval)
        return;

    ajFeattabOutDel((AjPFeattabOut*)PPval);

    return;
}




/* @funcstatic acdDelFile *****************************************************
**
** Function with void** prototype to delete ACD input file data
**
** @param [d] PPval [void**] Value to be deleted
** @return [void]
**
** @release 4.0.0
******************************************************************************/

static void acdDelFile(void** PPval)
{
    if(!*PPval)
        return;

    ajFileClose((AjPFile*)PPval);

    return;
}




/* @funcstatic acdDelFloat ****************************************************
**
** Function with void** prototype to delete ACD floating point array data
**
** @param [d] PPval [void**] Value to be deleted
** @return [void]
**
** @release 4.0.0
******************************************************************************/

static void acdDelFloat(void** PPval)
{
    if(!*PPval)
        return;

    ajFloatDel((AjPFloat*)PPval);

    return;
}




/* @funcstatic acdDelList *****************************************************
**
** Function with void** prototype to delete ACD list/selection data
**
** @param [d] PPval [void**] Value to be deleted
** @return [void]
**
** @release 4.0.0
******************************************************************************/

static void acdDelList(void** PPval)
{
    if(!*PPval)
        return;

    ajListstrFreeData((AjPList*)PPval);

    return;
}




/* @funcstatic acdDelMatrix ***************************************************
**
** Function with void** prototype to delete ACD integer comparison matrix data
**
** @param [d] PPval [void**] Value to be deleted
** @return [void]
**
** @release 4.0.0
******************************************************************************/

static void acdDelMatrix(void** PPval)
{
    if(!*PPval)
        return;

    ajMatrixDel((AjPMatrix*)PPval);

    return;
}




/* @funcstatic acdDelMatrixf **************************************************
**
** Function with void** prototype to delete ACD floating point comparison
** matrix data
**
** @param [d] PPval [void**] Value to be deleted
** @return [void]
**
** @release 4.0.0
******************************************************************************/

static void acdDelMatrixf(void** PPval)
{
    if(!*PPval)
        return;

    ajMatrixfDel((AjPMatrixf*)PPval);

    return;
}




/* @funcstatic acdDelObo ******************************************************
**
** Function with void** prototype to delete ACD obo term input data
**
** @param [d] PPval [void**] Value to be deleted
** @return [void]
**
** @release 6.4.0
******************************************************************************/

static void acdDelObo(void** PPval)
{
    if(!*PPval)
        return;

    ajOboallDel((AjPOboall*)PPval);

    return;
}




/* @funcstatic acdDelOutfile **************************************************
**
** Function with void** prototype to delete ACD output file data
**
** @param [d] PPval [void**] Value to be deleted
** @return [void]
**
** @release 4.0.0
******************************************************************************/

static void acdDelOutfile(void** PPval)
{
    if(!*PPval)
        return;

    ajOutfileClose((AjPOutfile*)PPval);

    return;
}




/* @funcstatic acdDelPatlist **************************************************
**
** Function with void** prototype to delete ACD pattern list data
**
** @param [d] PPval [void**] Value to be deleted
** @return [void]
**
** @release 4.0.0
******************************************************************************/

static void acdDelPatlist(void** PPval)
{
    if(!*PPval)
        return;

    ajPatlistSeqDel((AjPPatlistSeq*)PPval);

    return;
}




/* @funcstatic acdDelPhyloDist ************************************************
**
** Function with void** prototype to delete ACD phylogenetic distance data
**
** @param [d] PPval [void**] Value to be deleted
** @return [void]
**
** @release 4.0.0
******************************************************************************/

static void acdDelPhyloDist(void** PPval)
{
    if(!*PPval)
        return;

    ajPhyloDistDel((AjPPhyloDist*)PPval);

    return;
}




/* @funcstatic acdDelPhyloFreq ************************************************
**
** Function with void** prototype to delete ACD phylogenetic frequency data
**
** @param [d] PPval [void**] Value to be deleted
** @return [void]
**
** @release 4.0.0
******************************************************************************/

static void acdDelPhyloFreq(void** PPval)
{
    ajUser("acdDelPhyloFreq '%x'", *PPval);

    if(!*PPval)
        return;

    ajPhyloFreqDel((AjPPhyloFreq*)PPval);

    return;
}




/* @funcstatic acdDelPhyloProp ************************************************
**
** Function with void** prototype to delete ACD phylogenetic properties data
**
** @param [d] PPval [void**] Value to be deleted
** @return [void]
**
** @release 4.0.0
******************************************************************************/

static void acdDelPhyloProp(void** PPval)
{
    if(!*PPval)
        return;

    ajPhyloPropDel((AjPPhyloProp*)PPval);

    return;
}




/* @funcstatic acdDelPhyloState ***********************************************
**
** Function with void** prototype to delete ACD phylogenetic state data
**
** @param [d] PPval [void**] Value to be deleted
** @return [void]
**
** @release 4.0.0
******************************************************************************/

static void acdDelPhyloState(void** PPval)
{
    if(!*PPval)
        return;

    ajPhyloStateDel((AjPPhyloState*)PPval);

    return;
}




/* @funcstatic acdDelPhyloTree ************************************************
**
** Function with void** prototype to delete ACD phylogenetic tree data
**
** @param [d] PPval [void**] Value to be deleted
** @return [void]
**
** @release 4.0.0
******************************************************************************/

static void acdDelPhyloTree(void** PPval)
{
    if(!*PPval)
        return;

    ajPhyloTreeDelarray((AjPPhyloTree**)PPval);

    return;
}




/* @funcstatic acdDelRange ****************************************************
**
** Function with void** prototype to delete ACD range data
**
** @param [d] PPval [void**] Value to be deleted
** @return [void]
**
** @release 4.0.0
******************************************************************************/

static void acdDelRange(void** PPval)
{
    if(!*PPval)
        return;

    ajRangeDel((AjPRange*)PPval);

    return;
}




/* @funcstatic acdDelRefseq ***************************************************
**
** Function with void** prototype to delete ACD reference sequence input data
**
** @param [d] PPval [void**] Value to be deleted
** @return [void]
**
** @release 6.5.0
**
******************************************************************************/

static void acdDelRefseq(void** PPval)
{
    if(!*PPval)
        return;

    ajRefseqDel((AjPRefseq*)PPval);

    return;
}




/* @funcstatic acdDelReg ******************************************************
**
** Function with void** prototype to delete ACD regular expression data
**
** @param [d] PPval [void**] Value to be deleted
** @return [void]
**
** @release 4.0.0
******************************************************************************/

static void acdDelReg(void** PPval)
{
    if(!*PPval)
        return;

    ajPatlistRegexDel((AjPPatlistRegex*)PPval);

    return;
}




/* @funcstatic acdDelReport ***************************************************
**
** Function with void** prototype to delete ACD report data
**
** @param [d] PPval [void**] Value to be deleted
** @return [void]
**
** @release 4.0.0
******************************************************************************/

static void acdDelReport(void** PPval)
{
    if(!*PPval)
        return;

    ajReportDel((AjPReport*)PPval);

    return;
}




/* @funcstatic acdDelResource *************************************************
**
** Function with void** prototype to delete ACD data resource input data
**
** @param [d] PPval [void**] Value to be deleted
** @return [void]
**
** @release 6.4.0
******************************************************************************/

static void acdDelResource(void** PPval)
{
    if(!*PPval)
        return;

    ajResourceallDel((AjPResourceall*)PPval);

    return;
}




/* @funcstatic acdDelSeq ******************************************************
**
** Function with void** prototype to delete ACD sequence data
**
** @param [d] PPval [void**] Value to be deleted
** @return [void]
**
** @release 4.0.0
******************************************************************************/

static void acdDelSeq(void** PPval)
{
    if(!*PPval)
        return;

    ajSeqDel((AjPSeq*)PPval);

    return;
}




/* @funcstatic acdDelSeqall ***************************************************
**
** Function with void** prototype to delete ACD sequence stream data
**
** @param [d] PPval [void**] Value to be deleted
** @return [void]
**
** @release 4.0.0
******************************************************************************/

static void acdDelSeqall(void** PPval)
{
    if(!*PPval)
        return;

    ajSeqallDel((AjPSeqall*)PPval);

    return;
}




/* @funcstatic acdDelSeqout ***************************************************
**
** Function with void** prototype to delete ACD sequence output data
**
** @param [d] PPval [void**] Value to be deleted
** @return [void]
**
** @release 4.0.0
******************************************************************************/

static void acdDelSeqout(void** PPval)
{
    if(!*PPval)
        return;

    ajSeqoutDel((AjPSeqout*)PPval);

    return;
}




/* @funcstatic acdDelSeqset ***************************************************
**
** Function with void** prototype to delete ACD sequence set data
**
** @param [d] PPval [void**] Value to be deleted
** @return [void]
**
** @release 4.0.0
******************************************************************************/

static void acdDelSeqset(void** PPval)
{
    if(!*PPval)
        return;

    ajSeqsetDel((AjPSeqset*)PPval);

    return;
}




/* @funcstatic acdDelSeqsetArray **********************************************
**
** Function with void** prototype to delete ACD sequence sets data
**
** @param [d] PPval [void**] Value to be deleted
** @return [void]
**
** @release 4.0.0
******************************************************************************/

static void acdDelSeqsetArray(void** PPval)
{
    if(!*PPval)
        return;

    ajSeqsetDelarray((AjPSeqset**)PPval);

    return;
}




/* @funcstatic acdDelStr ******************************************************
**
** Function with void** prototype to delete ACD string data
**
** @param [d] PPval [void**] Value to be deleted
** @return [void]
**
** @release 4.0.0
******************************************************************************/

static void acdDelStr(void** PPval)
{
    if(!*PPval)
        return;

    ajStrDel((AjPStr*)PPval);

    return;
}




/* @funcstatic acdDelStrArray *************************************************
**
** Function with void** prototype to delete ACD string array data
**
** @param [d] PPval [void**] Value to be deleted
** @return [void]
**
** @release 4.0.0
******************************************************************************/

static void acdDelStrArray(void** PPval)
{
    if(!*PPval)
        return;

    ajStrDelarray((AjPStr**)PPval);

    return;
}




/* @funcstatic acdDelTaxon ****************************************************
**
** Function with void** prototype to delete ACD taxonomy input data
**
** @param [d] PPval [void**] Value to be deleted
** @return [void]
**
** @release 6.4.0
******************************************************************************/

static void acdDelTaxon(void** PPval)
{
    if(!*PPval)
        return;

    ajTaxallDel((AjPTaxall*)PPval);

    return;
}




/* @funcstatic acdDelText *****************************************************
**
** Function with void** prototype to delete ACD text input data
**
** @param [d] PPval [void**] Value to be deleted
** @return [void]
**
** @release 6.4.0
******************************************************************************/

static void acdDelText(void** PPval)
{
    if(!*PPval)
        return;

    ajTextallDel((AjPTextall*)PPval);

    return;
}




/* @funcstatic acdDelUrl ******************************************************
**
** Function with void** prototype to delete ACD URL input data
**
** @param [d] PPval [void**] Value to be deleted
** @return [void]
**
** @release 6.4.0
******************************************************************************/

static void acdDelUrl(void** PPval)
{
    if(!*PPval)
        return;

    ajUrlallDel((AjPUrlall*)PPval);

    return;
}




/* @funcstatic acdDelVariation ************************************************
**
** Function with void** prototype to delete ACD variation input data
**
** @param [d] PPval [void**] Value to be deleted
** @return [void]
**
** @release 6.4.0
******************************************************************************/

static void acdDelVariation(void** PPval)
{
    if(!*PPval)
        return;

    ajVarloadDel((AjPVarload*)PPval);

    return;
}




/* @funcstatic acdDelXml ******************************************************
**
** Function with void** prototype to delete ACD xml input data
**
** @param [d] PPval [void**] Value to be deleted
** @return [void]
**
** @release 6.6.0
**
******************************************************************************/

static void acdDelXml(void** PPval)
{
    if(!*PPval)
        return;

    ajXmlallDel((AjPXmlall*)PPval);

    return;
}




/* @funcstatic acdFree ********************************************************
**
** Function to delete ACD data using the standard free function
**
** @param [d] PPval [void**] Value to be deleted
** @return [void]
**
** @release 4.0.0
******************************************************************************/

static void acdFree(void** PPval)
{
    if(!*PPval)
        return;

    AJFREE(*PPval);

    return;
}




/* @section unused ************************************************************
**
** @fdata [none]
**
** @nam3rule Unused Contains dummy calls to unused functions to keep
**                 compilers happy
**
** @valrule * [void]
** @fcategory misc
**
******************************************************************************/




/* @func ajAcdUnused **********************************************************
**
** Dummy function to catch all unused functions defined in the ajacd
** source file.
**
** @return [void]
**
**
** @release 4.0.0
******************************************************************************/

void ajAcdUnused(void)
{
    AjPStr ajpstr=NULL;
    AcdPAcd acdpacd=NULL;
    float f=0.0;
    char c;

    acdSetXxxx(acdpacd);	    /* template function for acdSet */
    acdAttrToChar(acdpacd, "attr", '.', &c);
    acdQualToFloat(acdpacd, "", 0.0, 0, &f, &ajpstr);
    acdCountType("outfile");
    acdPrintUsed();

    return;
}




/* @section deprecated ********************************************************
**
** @fdata [none]
** @fcategory misc
**
** @nam3rule Graphics Graphics application
** @suffix P Package name given
** @suffix V Package version given
**
** @argrule Graphics pgm [const char*] Program name
** @argrule Graphics argc [ajint] Number of arguments
** @argrule Graphics argv [char* const[]] Command line arguments
** @argrule P package [const char*] Package name
** @argrule V packversion [const char*] Package version
**
** @valrule * [void]
******************************************************************************/




/* @funcstatic acdRangeTestCalc ***********************************************
**
** Tests for calculated minimum and maximum attributes. If found, requires
** other attributes to be defined to control the bahviour if the range
** is impossible in the current case
**
** @param [r] thys [const AcdPAcd] Acd object
** @return [AjBool] True if a calculated value was found
**
**
** @release 6.3.0
******************************************************************************/

static AjBool acdRangeTestCalc(const AcdPAcd thys)
{
    const AjPStr tmpstr;
    AjBool toggle = ajFalse;
    AjBool iscalcmin = ajFalse;
    AjBool iscalcmax = ajFalse;
    AjBool iscalc = ajFalse;
    AjBool failrange = ajFalse;
    AjBool truemin = ajFalse;

    tmpstr = acdAttrValue(thys, "minimum");
    if(!MAJSTRGETLEN(tmpstr))
        return ajFalse;

    if(acdVarTestValid(tmpstr, &toggle))
        iscalcmin = ajTrue;

    tmpstr = acdAttrValue(thys, "maximum");
    if(!MAJSTRGETLEN(tmpstr))
        return ajFalse;

    if(acdVarTestValid(tmpstr, &toggle))
        iscalcmax = ajTrue;

    if(iscalcmin || iscalcmax)
    {
        iscalc = ajTrue;
        if(!acdAttrToBool(thys, "failrange", ajFalse, &failrange))
        {
            acdErrorAcd(thys,
                        "Attribute %s: required with any calculated min/max",
                        "failrange", acdType[thys->Type].Name);
        }
        else
        {
            if(failrange)
            {
                tmpstr = acdAttrValue(thys, "rangemessage");
                if(!tmpstr)
                    acdErrorAcd(thys,
                                "Attribute %s: required for failrange: \"Y\"",
                                "rangemessage", acdType[thys->Type].Name);
            }
            else 
            {
                if(!acdAttrToBool(thys, "trueminimum",ajFalse, &truemin))
                    acdErrorAcd(thys,
                                "Attribute %s: required for failrange: \"N\"",
                                "trueminimum", acdType[thys->Type].Name);
            }

        }

    }

    return iscalc;
}




/* @funcstatic acdRegVarDefine ************************************************
**
** Define regular expressions for parsing variables and functions
**
** @return [void]
**
** @release 6.3.0
******************************************************************************/

static void acdRegVarDefine(void)
{
    if(!acdRegVarname)
	acdRegVarname = ajRegCompC("^(.*)\\$\\(([a-zA-Z0-9_.]+)\\)");

    if(!acdRegToggle)
	acdRegToggle = ajRegCompC("^(@\\([!])?(\\$\\([a-zA-Z0-9_.]+\\))\\)?$");

    if(!acdRegFunction)
        acdRegFunction = ajRegCompC("^(.*)\\@\\(([^()]+)\\)");

    return;
}




/* @section edam **************************************************************
**
** @fdata [none]
** @fcategory misc
**
** @nam2rule Acdedam EDAM parsing
** @nam3rule Parse Parse an ACD relation string
** @nam2rule Acdtype ACD type groups 
** @nam3rule AcdtypeGet ACD type cast
** @nam4rule AcdtypeGetGroup Return ACD type group classification
**
** @argrule Parse  relation [const AjPStr] Relation single attribute value
** @argrule Parse  id [AjPStr*] EDAM term numeric id
** @argrule Parse  namespace [AjPStr*] EDAM term namespace
** @argrule Parse  name [AjPStr*] EDAM term name
** @argrule Get    type [const AjPStr] ACD type name
**
** @valrule *Parse [AjBool] True on success
** @valrule *Get [const char*] Group name
******************************************************************************/




/* @funcstatic acdEdamTest ****************************************************
**
** Test an EDAM term from a relations attribute
**
** @param [r] relation [const AjPStr] Relation single attribute value
** @return [const AjPStr] Namespace
**
** @release 6.3.0
******************************************************************************/

static const AjPStr acdEdamTest(const AjPStr relation)
{
    AjPStr term = NULL;
    AjPStr name = NULL;
    AjPStr id = NULL;
    AjPStrTok handle = NULL;
    AjPStr token = NULL;
    AjPStr namespace = NULL;
    AjPObo oboterm = NULL;
    AjPObo obonameterm = NULL;
    AjPStr namequery = NULL;
    AjBool obonamefound = ajFalse;

    if(!acdEdam)
        return NULL;

    if(!ajStrPrefixC(relation, "EDAM"))
    {
        ajFmtPrintS(&namequery, "data:%S", relation);        
        obonameterm = ajObodataFetchName(acdEdam, namequery);
        if(obonameterm)
        {
            obonamefound = ajTrue;
            acdErrorValid("Relation '%S' matches EDAM term EDAM_data:%S %S",
                          relation, oboterm->Namespace, oboterm->Id);
        }

        ajFmtPrintS(&namequery, "topic:%S", relation);        
        obonameterm = ajObodataFetchName(acdEdam, namequery);
        if(obonameterm)
        {
            obonamefound = ajTrue;
            acdErrorValid("Relation '%S' matches EDAM term EDAM_topic:%S %S",
                          relation, oboterm->Id, oboterm->Namespace);
        }

        ajFmtPrintS(&namequery, "operation:%S", relation);        
        obonameterm = ajObodataFetchName(acdEdam, namequery);
        if(obonameterm)
        {
            obonamefound = ajTrue;
            acdErrorValid("Relation '%S' matches EDAM term EDAM_operation:%S %S",
                          relation, oboterm->Id, oboterm->Namespace);
        }

        if(!obonamefound)
            acdErrorValid("Not an EDAM term in relation '%S'", relation);

        return NULL;
    }

    ajStrExtractFirst(relation, &name, &term);

    handle = ajStrTokenNewC(term, ":");
    if(!ajStrTokenNextParse(handle, &namespace)) /* EDAM */
        acdErrorValid("Bad relation term '%S'", term);
    else if(!ajStrTokenNextParse(handle, &token)) /* ID */
        acdErrorValid("Bad relation term '%S'", term);

    if(ajStrPrefixC(namespace, "EDAM_"))
        ajStrCutStart(&namespace, 5);

    if(!ajStrGetLen(namespace))
        acdErrorValid("Bad relation no namespace");

    ajFmtPrintS(&id, "EDAM_%S:%S", namespace, token);

    oboterm =  ajObodataFetchId(acdEdam, id);

    if(!oboterm)
    {
        acdErrorValid("Unknown relation term '%S' fetch failed for '%S'",
                      term, id);
        return NULL;
    }

    if(ajOboIsObsolete(oboterm))
    {
        if(ajOboGetReplaced(oboterm))
        {
            obonameterm = ajObodataFetchId(acdEdam, ajOboGetReplaced(oboterm));
            if(obonameterm)
                acdErrorValid("Relation term '%S' obsolete, "
                              "possible replacement %S %S %S",
                              term, ajOboGetReplaced(oboterm),
                              obonameterm->Namespace, obonameterm->Name);
            else
                acdErrorValid("Relation term '%S' obsolete, "
                              "possible replacement '%S'",
                              term, ajOboGetReplaced(oboterm));
        }
        else
        {
                acdErrorValid("Relation term '%S' obsolete",
                              term);
        }
    }

    if(!ajStrMatchS(id, oboterm->Id))
        acdErrorValid("Relation term '%S' bad id, expected '%S' (%S)",
                      term, oboterm->Id, id);
       
    if(!ajStrMatchS(name, oboterm->Name))
    {
        obonamefound = ajFalse;

        ajFmtPrintS(&namequery, "%S:%S", namespace, name);        
        obonameterm = ajObodataFetchName(acdEdam, namequery);
        if(obonameterm)
        {
            obonamefound = ajTrue;
            if(ajStrMatchS(id, obonameterm->Id))
                acdErrorValid("Relation term '%S' bad name, expected '%S' "
                              "but matches synonym",
                              term, oboterm->Name);
            else
                acdErrorValid("Relation term '%S' bad name, expected '%S' "
                              "but matches name for %S %S %S",
                              term, oboterm->Name,
                              obonameterm->Id, obonameterm->Namespace,
                              obonameterm->Name);
        }

        if(!ajStrMatchC(namespace, "topic")){
            ajFmtPrintS(&namequery, "topic:%S", name);        
            obonameterm = ajObodataFetchName(acdEdam, namequery);
            if(obonameterm)
            {
                obonamefound = ajTrue;
                acdErrorValid("Relation term '%S' bad name, expected '%S' "
                              "but matches name for %S %S %S",
                              term, oboterm->Name,
                              obonameterm->Id, obonameterm->Namespace,
                              obonameterm->Name);
            }
        }

        if(!ajStrMatchC(namespace, "operation")){
            ajFmtPrintS(&namequery, "operation:%S", name);        
            obonameterm = ajObodataFetchName(acdEdam, namequery);
            if(obonameterm)
            {
                obonamefound = ajTrue;
                acdErrorValid("Relation term '%S' bad name, expected '%S' "
                              "but matches name for %S %S %S",
                              term, oboterm->Name,
                              obonameterm->Id, obonameterm->Namespace,
                              obonameterm->Name);
            }
        }

        if(!obonamefound)
            acdErrorValid("Relation term '%S' bad name, expected '%S'",
                    term, oboterm->Name);
    }

    if(!ajStrMatchS(namespace, oboterm->Namespace))
        acdErrorValid("Relation term '%S' bad namespace, expected '%S'",
                    term, oboterm->Namespace);
       
    return oboterm->Namespace;
}




/* @func ajAcdedamParse *******************************************************
**
** Parse an EDAM term from a relations attribute
**
** @param [r] relation [const AjPStr] Relation single attribute value
** @param [w] id [AjPStr*] EDAM term numeric id
** @param [w] namespace [AjPStr*] EDAM term namespace
** @param [w] name [AjPStr*] EDAM term name
** @return [AjBool] True on success
**
**
** @release 6.4.0
******************************************************************************/

AjBool ajAcdedamParse(const AjPStr relation, AjPStr* id,
                      AjPStr* namespace, AjPStr* name)
{
    AjPStr token = NULL;
    AjPStr term = NULL;
    AjPStr rest = NULL;
    AjPStrTok handle = NULL;

    if(!ajStrPrefixC(relation, "EDAM_"))
        return ajFalse;

    ajStrExtractFirst(relation, &rest, &term);

    handle = ajStrTokenNewC(term, ":_");
    if(!ajStrTokenNextParse(handle, &token)) /* EDAM */
        return ajFalse;
    else if(!ajStrTokenNextParse(handle, namespace)) /* namespace */
        return ajFalse;
    else if(!ajStrTokenNextParse(handle, id)) /* ID */
    {
        ajStrSetClear(namespace);
        return ajFalse;
    }

    ajStrAssignS(name, rest);

    ajStrDel(&token);
    ajStrDel(&term);
    ajStrDel(&rest);
    ajStrTokenDel(&handle);
    
    return ajTrue;
}




/* @func ajAcdtypeGetGroup ****************************************************
**
** Return the ACD group for an ACD datatype
**
** @param [r] type [const AjPStr] ACD type name
** @return [const char*] Group name
**
**
** @release 6.4.0
******************************************************************************/

const char* ajAcdtypeGetGroup(const AjPStr type)
{
    ajuint i;

    i =  0;
    while(acdType[i].Name)		/* ACD types as qualifiers */
    {
        if(ajStrMatchC(type, acdType[i].Name))
            return acdType[i].Group;
        i++;
    }

    return NULL;
}




#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif
#ifdef AJ_COMPILE_DEPRECATED
static AjBool    acdTestDebugIsSet(void);
static AjBool    acdTestDebug(void);
static AjBool    acdTestFilter(void);
static AjBool    acdTestStdout(void);
#endif




#ifdef AJ_COMPILE_DEPRECATED
/* @funcstatic acdTestDebugIsSet **********************************************
**
** Tests whether the command line switch for debug messages has been set
** by testing internal variable 'acdDebugSet'
**
** @return [AjBool] Debugging status.
**
** @release 4.0.0
** @@
******************************************************************************/

static AjBool acdTestDebugIsSet(void)
{
    ajDebug("acdTestDebugIsSet returning %B\n", acdDebugSet);
    return acdDebugSet;
}




/* @funcstatic acdTestDebug ***************************************************
**
** Tests whether debug messages are required by checking
** internal variable 'acdDebug'
**
** @return [AjBool] Debugging status.
**
** @release 4.0.0
** @@
******************************************************************************/

static AjBool acdTestDebug(void)
{
    ajDebug("acdDebug returning %B\n", acdDebug);
    return acdDebug;
}




/* @funcstatic acdTestFilter **************************************************
**
** Tests whether input and output use stdin and stdout as a filter
** by returning internal variable 'acdFilter'
**
** @return [AjBool] Filter status.
**
** @release 4.0.0
** @@
******************************************************************************/

static AjBool acdTestFilter(void)
{
    return acdFilter;
}




/* @funcstatic acdTestStdout **************************************************
**
** Tests whether output uses stdout for output by default
** by returning internal variable 'acdStdout'
**
** @return [AjBool] Stdout status.
**
** @release 4.0.0
** @@
******************************************************************************/

static AjBool acdTestStdout(void)
{
    return acdStdout;
}




/* @obsolete ajAcdGetDiscretestatesI
** @replace ajAcdGetDiscretestatesSingle (1,2/1)
*/

__deprecated AjPPhyloState  ajAcdGetDiscretestatesI(const char *token,
						   ajint num)
{
    AjPPhyloState* val;
    ajint i;

    val = acdGetValue(token, "discretestates");
    for(i=0; val[i]; i++)
	continue;

    if(num > i)
	ajWarn("value %d not found for %s, last value was %d",
	       num, token, i-1);

    return val[num-1];
}




/* @obsolete ajAcdGetFeat
** @rename ajAcdGetFeatures
*/

__deprecated AjPFeattable ajAcdGetFeat(const char *token)
{
    return ajAcdGetFeatures(token);
}




/* @obsolete ajAcdGetListI
** @replace ajAcdGetListSingle (1,2/1)
*/

__deprecated AjPStr  ajAcdGetListI(const char *token, ajint num)
{
    AjPStr *val;
    ajint i;

    val = acdGetValue(token, "list");

    for(i=0; val[i]; i++)
	continue;

    if(num > i)
	ajWarn("value %d not found for %s, last value was %d",
	       num, token, i-1);

    return val[num-1];
}




/* @obsolete ajAcdGetSelectI
** @replace ajAcdGetSelectSingle (1,2/1)
*/

__deprecated AjPStr  ajAcdGetSelectI(const char *token, ajint num)
{
    AjPStr* val;
    ajint i;

    val =  acdGetValue(token, "select");

    for(i=0; val[i]; i++)
	continue;

    if(num > i)
	ajWarn("value %d not found for %s, last value was %d",
	       num, token, i-1);

    return val[num-1];
}




/* @obsolete ajAcdGetSeqsetallI
** @replace ajAcdGetSeqsetallSingle (1,2/1)
*/

__deprecated AjPSeqset  ajAcdGetSeqsetallI(const char *token, ajint num)
{
    AjPSeqset *val;
    ajint i;

    val = acdGetValue(token, "seqsetall");

    for(i=0; val[i]; i++)
	continue;

    if(num > i)
	ajWarn("value %d not found for %s, last value was %d",
	       num, token, i-1);

    return val[num-1];
}




/* @obsolete ajAcdGetTreeI
** @replace ajAcdGetTreeSingle (1,2/1)
*/

__deprecated AjPPhyloTree  ajAcdGetTreeI(const char *token, ajint num)
{
    AjPPhyloTree *val;
    ajint i;

    val = acdGetValue(token, "tree");

    for(i=0; val[i]; i++)
	continue;

    if(num > i)
	ajWarn("value %d not found for %s, last value was %d",
	       num, token, i-1);

    return val[num-1];
}




/* @obsolete ajAcdValue
** @rename ajAcdGetValue
*/

__deprecated const AjPStr  ajAcdValue(const char *token)
{
    return ajAcdGetValue(token);
}




/* @obsolete ajAcdIsUserdefined
** @rename ajAcdIsUserdefinedC
*/
__deprecated AjBool ajAcdIsUserdefined(const char *token)
{
    return ajAcdIsUserdefinedC(token);
}




/* @obsolete ajAcdDebug
** @remove No longer public
*/

__deprecated AjBool  ajAcdDebug(void)
{
    return acdTestDebug();
}




/* @obsolete ajAcdDebugIsSet
** @remove No longer public
*/

__deprecated AjBool  ajAcdDebugIsSet(void)
{
    return acdTestDebugIsSet();
}




/* @obsolete ajAcdStdout
** @remove No longer public
*/

__deprecated AjBool  ajAcdStdout(void)
{
    return acdTestStdout();
}




/* @obsolete ajGraphicsInit
** @rename ajAcdGraphicsInit
*/
__deprecated void ajGraphicsInit(const char *pgm,
				 ajint argc, char * const argv[])
{
    ajAcdGraphicsInit(pgm, argc, argv);
    return;
}




/* @obsolete ajGraphInit
** @rename ajAcdGraphicsInit
*/

__deprecated void ajGraphInit(const char *pgm, ajint argc, char * const argv[])
{
    ajAcdGraphicsInit(pgm, argc, argv);
    return;
}




/* @obsolete ajGraphicsInitPV
** @rename ajAcdGraphicsInitPV
*/

__deprecated void ajGraphicsInitPV(const char *pgm,
				   ajint argc, char *const argv[],
				   const char *package,
				   const char *packversion)
{
    ajAcdGraphicsInitPV(pgm, argc, argv, package, packversion);
    return;
}




/* @obsolete ajGraphInitPV
** @rename ajAcdGraphicsInitPV
*/

__deprecated void ajGraphInitPV(const char *pgm, ajint argc, char *const argv[],
                                const char *package,const char *packversion)
{
    ajAcdGraphicsInitPV(pgm, argc, argv, package, packversion);
    return;
}




/* @obsolete ajAcdGetBool
** @rename ajAcdGetBoolean
*/

__deprecated AjBool ajAcdGetBool(const char *token)
{
    return ajAcdGetBoolean(token);
}




/*
** @obsolete ajAcdGraphicsInit
** @rename embInit
*/

__deprecated void ajAcdGraphicsInit(const char *pgm,
                                    ajint argc, char * const argv[])
{
    ajSeqdbInit();
    ajNamInit("emboss");
    ajAcdInit(pgm, argc, argv);

    return;
}




/*
** @obsolete ajAcdGraphicsInitPV
** @rename embInitPV
*/

__deprecated void ajAcdGraphicsInitPV(const char *pgm,
                                      ajint argc, char *const argv[],
                                      const char *package,
                                      const char *packversion)
{
    ajSeqdbInit();
    ajNamInit("emboss");
    ajAcdInitPV(pgm, argc, argv,package, packversion);

    return;
}
#endif

/* @source ajvarread **********************************************************
**
** AJAX variation data reading functions
**
** These functions control all aspects of AJAX variation data reading
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.27 $
** @modified Oct 5 pmr First version
** @modified $Date: 2012/12/07 10:07:16 $ by $Author: rice $
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

#include "ajvarread.h"
#include "ajvar.h"
#include "ajcall.h"
#include "ajlist.h"
#include "ajquery.h"
#include "ajtextread.h"
#include "ajnam.h"
#include "ajfileio.h"
#include "ajtagval.h"
#include "ajreg.h"

#include "ajseqbam.h"
#include "ajvarbcf.h"

#include <string.h>


AjPTable varDbMethods = NULL;

static AjPStrTok varHandle = NULL;
static AjPStrTok varDataHandle = NULL;
static AjPStrTok varFieldHandle = NULL;
static AjPStr varToken = NULL;
static AjPStr varDataToken = NULL;
static AjPStr varFieldToken = NULL;
static AjPStr varinReadLine     = NULL;
static AjPStr varFieldName = NULL;

static AjBool   VarInitVcf = AJFALSE;

static AjPRegexp varRegId = NULL;
static AjPRegexp varRegType = NULL;
static AjPRegexp varRegNumber = NULL;
static AjPRegexp varRegDesc = NULL;
static AjPRegexp varRegDesc2 = NULL;
static AjPRegexp varRegGenomes = NULL;
static AjPRegexp varRegMixture = NULL;

static AjBool	    varinReadVcf41Header(AjPVarin varin, AjPVar var);
static AjBool       varinLoadBcf(AjPVarin thys, AjPVar var);
static AjBool       varinLoadVcf3x(AjPVarin thys, AjPVar var);
static AjBool       varinLoadVcf40(AjPVarin thys, AjPVar var);
static AjBool       varinLoadVcf41(AjPVarin thys, AjPVar var);
static const AjPStr varinFieldVcf(AjPVarField var, const AjPStr str);
static const AjPStr varinFieldVcf3x(AjPVarField var, const AjPStr str);
static AjPVarSample varinSampleVcf(const AjPStr str);
static ajuint       varinDataVcf(const AjPStr str, AjPVar var);
static ajuint       varinParseListVcf(const AjPStr str, AjPStr **Parray);
static ajuint       varinParseFloatVcf(const AjPStr str, float **Parray);
static void         varRegInitVcf(void);
static void         varinVcfCheckLastrecordForRefseqIds(AjPVar var);




/* @datastatic VarPBcfdata ****************************************************
**
** Variation BCF format internals
**
** @alias VarSBcfdata
** @alias VarOBcfdata
**
** @attr gzfile [AjPVarBcfFile] BGZF file
** @attr Header [AjPVarBcfHeader] BCF Header object
** @@
******************************************************************************/

typedef struct VarSBcfdata
{
    AjPVarBcfFile gzfile;
    AjPVarBcfHeader Header;
} VarOBcfdata;

#define VarPBcfdata VarOBcfdata*




/* @datastatic VarPId *********************************************************
**
** Variation known identifiers
**
** @alias VarSId
** @alias VarOId
**
** @attr Id [const char*] Identifier
** @attr Num  [ajint] Number of values
** @attr Type [AjEVarType] Format description
** @attr Desc [const char*] Description
** @@
******************************************************************************/

typedef struct VarSId
{
    const char *Id;
    ajint Num;
    AjEVarType Type;
    const char *Desc;
} VarOId;

#define VarPId VarOId*


VarOId varInfoIds40[] = {
    {"AA",        1, AJVAR_STR,   "Ancestral Allele"},
    {"AC",varNumAlt, AJVAR_INT,   "Allele count"},
    {"AF",varNumAlt, AJVAR_FLOAT, "Allele Frequency"},
    {"AN",        1, AJVAR_INT,   "Number of alleles in called genotypes"},
    {"BQ",        1, AJVAR_INT,   "RMS base quality"},
    {"CIGAR",     1, AJVAR_STR,   "cigar string"},
    {"DB",        0, AJVAR_FLAG,  "dbSNP membership"},
    {"DP",        1, AJVAR_INT,   "Total Depth"},
    {"END",       1, AJVAR_INT,   "End position of variant"},
    {"H2",        0, AJVAR_FLAG,  "Membership in hapmap2"},
    {"MQ",        1, AJVAR_INT,   "RMS mapping quality"},
    {"MQ0",       1, AJVAR_INT,   "Number of MAPQ zero reads"},
    {"NS",        1, AJVAR_INT,   "Number of Samples With Data"},
    {"SB",        1, AJVAR_STR,   "Strand bias"},
    {"SOMATIC",   0, AJVAR_FLAG,  "Somatic mutation"},
    {"VALIDATED", 0, AJVAR_FLAG,  "Validated by experiment"},
    {NULL, 0, 0, NULL}
};


VarOId varInfoIds41[] = {
    {"1000G",     0, AJVAR_FLAG,  "Membership in 1000 Genomes"},
    {"H3",        0, AJVAR_FLAG,  "Membership in hapmap3"},

    /* structural variation fields (include END, DP*/

    {"IMPRECISE", 0, AJVAR_FLAG,  "Imprecise structural variation"},
    {"NOVEL",     0, AJVAR_FLAG,  "Indicates a novel structural variation"},
    {"SVTYPE",    1, AJVAR_STR,   "Type of structural variant"},
    {"SVLEN",varNumAny,AJVAR_INT,
                            "Difference in length between REF an ALT alleles"},

    {"CIPOS",     2, AJVAR_INT,
                      "Confidence interval around POS for imprecise variants"},
    {"CIEND",     2, AJVAR_INT,
                      "Confidence interval around END for imprecise variants"},

    {"HOMLEN",varNumAny, AJVAR_INT,
          "Length of base pair identical micro-homology at event breakpoints"},
    {"HOMSEQ",varNumAny, AJVAR_STR,
        "Sequence of base pair identical micro-homology at event breakpoints"},
    {"BKPTID",varNumAny, AJVAR_STR,
                  "ID of the assembled alternate allele in the assembly file"},
    {"MEINFO",    4, AJVAR_STR,
                    "Mobile element info of the form NAME,START,END,POLARITY"},
    {"METRANS",   4, AJVAR_STR,
        "Mobile element transduction info of the form CHR,START,END,POLARITY"},

    {"DGVID",     1, AJVAR_STR,
                        "ID of this element in Database of Genomic Variation"},
    {"DBVARID",   1, AJVAR_STR, "ID of this element in DBVAR"},
    {"DBRIPID",   1, AJVAR_STR, "ID of this element in DBRIP"},

    {"MATEID",varNumAny, AJVAR_STR, "ID of mate breakends"},
    {"PARID",     1, AJVAR_STR, "ID of partner breakend"},
    {"EVENT",     1, AJVAR_STR, "ID of event associated to breakend"},
    {"CILEN",     2, AJVAR_INT, "Confidence interval around the length of the "
                                "inserted material between breakends"},
    {"DPADJ",varNumAny, AJVAR_INT, "Read Depth of adjacency"},
    {"CN",        1, AJVAR_INT, "Copy number of segment containing breakend"},
    {"CNADJ",varNumAny, AJVAR_INT, "Copy number of adjacency"},
    {"CICN",      2, AJVAR_INT,
                    "Confidence interval around copy number for the segment"},
    {"CICNADJ",varNumAny, AJVAR_INT,
                  "Confidence interval around copy number for the adjacency"},
    {NULL, 0, 0, NULL}
};
    



VarOId varGenotypeIds41[] = {
    {"GT",varNumGen, AJVAR_STR,   "Genotype"},
    {"DP",        1, AJVAR_INT,   "Read Depth"},
    {"FT",        1, AJVAR_STR,   "Sample genotype filter"},
    {"GL",        1, AJVAR_FLOAT, "Genotype likelihood"},
    {"GQ",        1, AJVAR_INT,   "Genotype Quality"},
    {"HQ",        2, AJVAR_INT,   "Haplotype Quality"},

    {"GLE",varNumGen,AJVAR_STR,   "Genotype likelihoods of heterogeneous "
                                  "ploidy"},
    {"PL",varNumGen, AJVAR_INT,   "Phred-scaled genotype likelihoods"},
    {"GP",varNumGen, AJVAR_FLOAT, "Phred-scaled genotype posterior "
                                  "probabilities"},
    {"PS",        1, AJVAR_INT,   "Phase set"},
    {"PQ",        1, AJVAR_FLOAT, "Phasing quality"},
    {"EC",        1, AJVAR_FLOAT, "Expected count"},
    {"CN",        1, AJVAR_INT,   "Copy number genotype for imprecise events"},
    {"CNQ",       1, AJVAR_FLOAT,
                          "Copy number genotype quality for imprecise events"},
    {"CNL",varNumAny,AJVAR_FLOAT,
                       "Copy number genotype likelihood for imprecise events"},
    {"MQ",        1, AJVAR_INT,   "Phred style probability score that the "
                     "variant is novel with respect to the genome's ancestor"},
    {"HAP",       1, AJVAR_INT,   "Unique haplotype identifier"},
    {"AHAP",      1, AJVAR_INT, "Unique identifier of ancestral haplotype"},
    {NULL, 0, 0, NULL}
};




/* @datastatic VarPInFormat *************************************************
**
** Variation input formats data structure
**
** @alias VarSInFormat
** @alias VarOInFormat
**
** @attr Name [const char*] Format name
** @attr Obo  [const char*] Ontology term id from EDAM
** @attr Desc [const char*] Format description
** @attr Alias [AjBool] Name is an alias for an identical definition
** @attr Try [AjBool] If true, try for an unknown input. Duplicate names
**                    and read-anything formats are set false
** @attr Load [AjBool function] Loading function, returns ajTrue on success
** @@
******************************************************************************/

typedef struct VarSInFormat
{
    const char *Name;
    const char *Obo;
    const char *Desc;
    AjBool Alias;
    AjBool Try;
    AjBool (*Load) (AjPVarin thys, AjPVar var);
} VarOInFormat;

#define VarPInFormat VarOInFormat*

static VarOInFormat varinFormatDef[] =
{
/* "Name",        "OBOterm", "Description" */
/*     Alias,   Try,     */
/*     ReadFunction LoadFunction */
  {"unknown",     "0000", "Unknown format",
       AJFALSE, AJFALSE,
       &varinLoadVcf40}, /* default to first format */
   {"bcf",         "0000", "BCF format",
	AJFALSE,  AJFALSE,
       &varinLoadBcf},
  {"vcf40",       "0000", "VCFv4.0 format",
       AJFALSE, AJTRUE,
       &varinLoadVcf40},
  {"vcf",         "0000", "VCFv4.0 format",
       AJTRUE,  AJFALSE,
       &varinLoadVcf40},
  {"vcf41",       "0000", "VCFv4.1 format",
       AJFALSE, AJTRUE,
       &varinLoadVcf41},
  {"vcf3",        "0000", "VCFv3.x format",
       AJFALSE, AJTRUE,
       &varinLoadVcf3x},
  {NULL, NULL, NULL, 0, 0, NULL}
};



static ajuint varinLoadFmt(AjPVarin varin, AjPVar var,
                           ajuint format);
static AjBool varinLoad(AjPVarin varin, AjPVar var);
static AjBool varinformatFind(const AjPStr format, ajint* iformat);
static AjBool varinFormatSet(AjPVarin varin, AjPVar var);
static AjBool varinListProcess(AjPVarin varin, AjPVar var,
                               const AjPStr listfile);
static void varinListNoComment(AjPStr* text);
static void varinQryRestore(AjPVarin varin, const AjPQueryList node);
static void varinQrySave(AjPQueryList node, const AjPVarin varin);
static AjBool varDefine(AjPVar thys, AjPVarin varin);
static AjBool varinQryProcess(AjPVarin varin, AjPVar var);
static AjBool varinQueryMatch(const AjPQuery thys, const AjPVar var);




/* @filesection ajvarread ****************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
*/




/* @datasection [AjPVarin] Variation input objects ***************************
**
** Function is for manipulating variation input objects
**
** @nam2rule Varin
******************************************************************************/




/* @section Variation input constructors ***************************************
**
** All constructors return a new variation input object by pointer. It
** is the responsibility of the user to first destroy any previous
** variation input object. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so
** anyway.
**
** @fdata [AjPVarin]
**
** @nam3rule  New     Construct a new variation input object
**
** @valrule   *  [AjPVarin] New variation input object
**
** @fcategory new
**
******************************************************************************/




/* @func ajVarinNew ***********************************************************
**
** Creates a new variation input object.
**
** @return [AjPVarin] New variation input object.
** @category new [AjPVarin] Default constructor
**
** @release 6.4.0
** @@
******************************************************************************/

AjPVarin ajVarinNew(void)
{
    AjPVarin pthis;

    AJNEW0(pthis);

    pthis->Input = ajTextinNewDatatype(AJDATATYPE_VARIATION);

    pthis->VarData      = NULL;

    return pthis;
}





/* @section variation input destructors ****************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the variation input object.
**
** @fdata [AjPVarin]
**
** @nam3rule Del Destructor
**
** @argrule Del pthis [AjPVarin*] Variation input object
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajVarinDel ***********************************************************
**
** Deletes a variation input object.
**
** @param [d] pthis [AjPVarin*] Variation input
** @return [void]
** @category delete [AjPVarin] Default destructor
**
** @release 6.4.0
** @@
******************************************************************************/

void ajVarinDel(AjPVarin* pthis)
{
    AjPVarin thys;

    if(!pthis)
        return;

    if(!*pthis)
        return;

    thys = *pthis;

    ajDebug("ajVarinDel called qry:'%S'\n", thys->Input->Qry);

    ajTextinDel(&thys->Input);

    AJFREE(*pthis);

    return;
}




/* @section variation input modifiers ******************************************
**
** These functions use the contents of a variation input object and
** update them.
**
** @fdata [AjPVarin]
**
** @nam3rule Clear Clear all values
** @nam3rule Qry Reset using a query string
** @suffix C Character string input
** @suffix S String input
**
** @argrule * thys [AjPVarin] Variation input object
** @argrule C txt [const char*] Query text
** @argrule S str [const AjPStr] query string
**
** @valrule * [void]
**
** @fcategory modify
**
******************************************************************************/




/* @func ajVarinClear *********************************************************
**
** Clears a variation input object back to "as new" condition, except
** for the query list which must be preserved.
**
** @param [w] thys [AjPVarin] Variation input
** @return [void]
**
** @category modify [AjPVarin] Resets ready for reuse.
**
** @release 6.4.0
** @@
******************************************************************************/

void ajVarinClear(AjPVarin thys)
{

    ajDebug("ajVarinClear called\n");

    ajTextinClear(thys->Input);

    thys->Loading = ajFalse;
    thys->VarData = NULL;

    return;
}




/* @func ajVarinQryC **********************************************************
**
** Resets a variation input object using a new Universal
** Query Address
**
** @param [u] thys [AjPVarin] Variation input object.
** @param [r] txt [const char*] Query
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajVarinQryC(AjPVarin thys, const char* txt)
{
    ajVarinClear(thys);
    ajStrAssignC(&thys->Input->Qry, txt);

    return;
}





/* @func ajVarinQryS **********************************************************
**
** Resets a variation input object using a new Universal
** Query Address
**
** @param [u] thys [AjPVarin] Variation input object.
** @param [r] str [const AjPStr] Query
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajVarinQryS(AjPVarin thys, const AjPStr str)
{
    ajVarinClear(thys);
    ajStrAssignS(&thys->Input->Qry, str);

    return;
}




/* @section casts *************************************************************
**
** Return values
**
** @fdata [AjPVarin]
**
** @nam3rule Trace Write debugging output
**
** @argrule * thys [const AjPVarin] Variation input object
**
** @valrule * [void]
**
** @fcategory cast
**
******************************************************************************/




/* @func ajVarinTrace *********************************************************
**
** Debug calls to trace the data in a variation input object.
**
** @param [r] thys [const AjPVarin] Variation input object.
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajVarinTrace(const AjPVarin thys)
{
    ajDebug("variation input trace\n");
    ajDebug("====================\n\n");

    ajTextinTrace(thys->Input);

    if(thys->VarData)
	ajDebug( "  VarData: exists\n");

    return;
}




/* @section Variation data inputs **********************************************
**
** These functions read the wxyxdesc data provided by the first argument
**
** @fdata [AjPVarin]
**
** @nam3rule Load Load variation data
** @nam3rule Read Read variation data
**
** @argrule Load varin [AjPVarin] Variation input object
** @argrule Load var [AjPVar] Variation data
** @argrule Read varin [AjPVarin] Variation input object
** @argrule Read var [AjPVar] Variation data
**
** @valrule * [AjBool] true on success
**
** @fcategory input
**
******************************************************************************/




/* @func ajVarinLoad **********************************************************
**
** If the file is not yet open, calls varinQryProcess to convert the query
** into an open file stream.
**
** Uses varinLoad for the actual data loading.
**
** Returns the results in the AjPVar object.
**
** @param [u] varin [AjPVarin] Variation data input definitions
** @param [w] var [AjPVar] Variation data returned.
** @return [AjBool] ajTrue on success.
** @category input [AjPVar] Master variation data input,
**                  calls specific functions for file access type
**                  and variation data format.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajVarinLoad(AjPVarin varin, AjPVar var)
{
    AjBool ret       = ajFalse;
    AjPQueryList node = NULL;
    AjBool listdata  = ajFalse;
    AjPTextin input = varin->Input;
    AjPFilebuff buff = input->Filebuff;

    ajDebug("ajVarinLoad '%F' EOF: %B records: %u dataread: %B datadone: %B\n",
           ajFilebuffGetFile(buff), ajFilebuffIsEof(buff),
           input->Records, input->Dataread, input->Datadone);

    if(buff)
    {
	/* (a) if file still open, keep loading */
	ajDebug("ajVarinLoad: input file '%F' still there, try again\n",
		buff->File);
	ret = varinLoad(varin, var);
	ajDebug("ajVarinLoad: open buffer  qry: '%S' returns: %B\n",
		input->Qry, ret);
    }
    else
    {
	/* (b) if we have a list, try the next query in the list */
	if(ajListGetLength(input->List))
	{
	    listdata = ajTrue;
	    ajListPop(input->List, (void**) &node);

	    ajDebug("++pop from list '%S'\n", node->Qry);
	    ajVarinQryS(varin, node->Qry);
	    ajDebug("++SAVE WXYZIN '%S' '%S' %d\n",
		    input->Qry,
		    input->Formatstr, input->Format);

            varinQryRestore(varin, node);

	    ajStrDel(&node->Qry);
	    ajStrDel(&node->Formatstr);
	    AJFREE(node);

	    ajDebug("ajVarinLoad: open list, try '%S'\n",
                    input->Qry);

	    if(!varinQryProcess(varin, var) &&
               !ajListGetLength(input->List))
		return ajFalse;

	    ret = varinLoad(varin, var);
	    ajDebug("ajVarinLoad: list qry: '%S' returns: %B\n",
		    input->Qry, ret);
	}
	else
	{
	    ajDebug("ajVarinLoad: no file yet - test query '%S'\n",
                    input->Qry);

	    /* (c) Must be a query - decode it */
	    if(!varinQryProcess(varin, var) &&
               !ajListGetLength(input->List))
		return ajFalse;

	    if(ajListGetLength(input->List)) /* could be a new list */
		listdata = ajTrue;

	    ret = varinLoad(varin, var);
	    ajDebug("ajVarinLoad: new qry: '%S' returns: %B\n",
		    input->Qry, ret);
	}
    }

    /* Now load whatever we got */

    while(!ret && ajListGetLength(input->List))
    {
	/* Failed, but we have a list still - keep trying it */
        if(listdata)
	    ajErr("Failed to load variation data '%S'",
                  input->Qry);

	listdata = ajTrue;
	ajListPop(input->List,(void**) &node);
	ajDebug("++try again: pop from list '%S'\n", node->Qry);
	ajVarinQryS(varin, node->Qry);
	ajDebug("++SAVE (AGAIN) VARIN '%S' '%S' %d\n",
		input->Qry,
		input->Formatstr, input->Format);

	varinQryRestore(varin, node);

	ajStrDel(&node->Qry);
	ajStrDel(&node->Formatstr);
	AJFREE(node);

	if(!varinQryProcess(varin, var))
	    continue;

	ret = varinLoad(varin, var);
	ajDebug("ajVarinLoad: list retry qry: '%S' returns: %B\n",
		input->Qry, ret);
    }

    if(!ret)
    {
	if(listdata)
	    ajErr("Failed to load variation data '%S'",
                  input->Qry);

	return ajFalse;
    }


    varDefine(var, varin);

    return ajTrue;
}




/* @funcstatic varinQueryMatch ************************************************
**
** Compares a variation data item to a query and returns true if they match.
**
** @param [r] thys [const AjPQuery] query.
** @param [r] var [const AjPVar] Variation data.
** @return [AjBool] ajTrue if the variation data matches the query.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool varinQueryMatch(const AjPQuery thys, const AjPVar var)
{
    AjBool tested = ajFalse;
    AjIList iterfield  = NULL;
    AjPQueryField field = NULL;
    AjBool ok = ajFalse;

    ajDebug("varinQueryMatch '%S' fields: %Lu Case %B Done %B\n",
	    var->Id, ajListGetLength(thys->QueryFields),
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
                    var->Id);
            if(thys->CaseId)
            {
                if(ajStrMatchWildS(var->Id, field->Wildquery))
                {
                    ajListIterDel(&iterfield);
                    return ajTrue;
                }
            }
            else
            {
                if(ajStrMatchWildCaseS(var->Id, field->Wildquery))
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
            if(ajStrMatchWildCaseS(var->Id, field->Wildquery))
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




/* @funcstatic varDefine ******************************************************
**
** Make sure all variation data object attributes are defined
** using values from the variation input object if needed
**
** @param [w] thys [AjPVar] Variation data returned.
** @param [u] varin [AjPVarin] Variation data input definitions
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool varDefine(AjPVar thys, AjPVarin varin)
{

    /* if values are missing in the variation object, we can use defaults
       from varin or calculate where possible */

    /* assign the dbname if defined in the varin object */
    if(ajStrGetLen(varin->Input->Db))
      ajStrAssignS(&thys->Db, varin->Input->Db);

    return ajTrue;
}





/* @funcstatic varinLoadFmt ***************************************************
**
** Tests whether variation data can be read using the specified format.
** Then tests whether the variation data matches variation data query criteria
** and checks any specified type. Applies upper and lower case.
**
** @param [u] varin [AjPVarin] Variation data input object
** @param [w] var [AjPVar] Variation data object
** @param [r] format [ajuint] input format code
** @return [ajuint] 0 if successful.
**                  1 if the query match failed.
**                  2 if the variation data type failed
**                  3 if it failed to read any variation data
**
** @release 6.4.0
** @@
** This is the only function that calls the appropriate Load function
** varinLoadXxxxxx where Xxxxxxx is the supported variation data format.
**
** Some of the varLoadXxxxxx functions fail to reset the buffer correctly,
** which is a very serious problem when cycling through all of them to
** identify an unknown format. The extra ajFileBuffReset call at the end is
** intended to address this problem. The individual functions should still
** reset the buffer in case they are called from elsewhere.
**
******************************************************************************/

static ajuint varinLoadFmt(AjPVarin varin, AjPVar var,
                           ajuint format)
{
    AjPTextin input = varin->Input;
    AjPFilebuff buff = input->Filebuff;

    ajDebug("++varinLoadFmt format %d (%s) '%S'\n",
	    format, varinFormatDef[format].Name,
	    input->Qry);

    if(!input->Dataread)
        input->Records = 0;

    /* Calling funclist varinFormatDef() */
    if((*varinFormatDef[format].Load)(varin, var))
    {
        input->Dataread = ajTrue;
	ajDebug("varinLoadFmt success with format %d (%s)\n",
		format, varinFormatDef[format].Name);
        ajDebug("varinLoadFmt status id: '%S' data: %Lu count: %u done: %B\n",
                var->Id, ajListGetLength(var->Data),
                input->Datacount, input->Datadone);
	input->Format = format;
	ajStrAssignC(&input->Formatstr,
	             varinFormatDef[format].Name);
	ajStrAssignC(&var->Formatstr,
	             varinFormatDef[format].Name);
	ajStrAssignEmptyS(&var->Db, input->Db);
	ajStrAssignS(&var->Filename, input->Filename);

	if(varinQueryMatch(input->Query, var))
	{
            /* ajVarinTrace(varin); */

            return FMT_OK;
        }

	ajDebug("query match failed, continuing ...\n");
	ajVarClear(var);

	return FMT_NOMATCH;
    }
    else if (buff)
    {
	ajDebug("Testing input buffer: IsBuff: %B Eof: %B\n",
		ajFilebuffIsBuffered(buff),
		ajFilebuffIsEof(buff));

	if (!ajFilebuffIsBuffered(buff) &&
	    ajFilebuffIsEof(buff))
	    return FMT_EOF;

	ajFilebuffReset(buff);
	ajDebug("Format %d (%s) failed, file buffer reset by varinLoadFmt\n",
		format, varinFormatDef[format].Name);
	/* ajFilebuffTraceFull(varin->Filebuff, 10, 10);*/
    }

    ajDebug("++varinLoadFmt failed - nothing read\n");

    return FMT_FAIL;
}




/* @funcstatic varinLoad ******************************************************
**
** Given data in a varin structure, tries to read the next chunk
** using the specified format or by trial and error.
**
** @param [u] varin [AjPVarin] Variation data input object
** @param [w] var [AjPVar] Variation data object
** @return [AjBool] ajTrue on success
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool varinLoad(AjPVarin varin, AjPVar var)
{
    ajuint i;
    ajuint istat = 0;
    ajuint jstat = 0;
    AjPTextin input = varin->Input;
    AjPFilebuff buff = input->Filebuff;
    AjBool ok;

    AjPTextAccess  textaccess  = input->Query->TextAccess;
    AjPVarAccess varaccess = input->Query->Access;

    if(!input->Records)
    {
        ajVarClear(var);
        ajDebug("varinLoad: cleared\n");
    }

/* test here for a new value */

    if(input->Single && input->Count && !input->Datacount)
    {
	/*
	** One variation data item at a time is read.
	** The first variation data item was read by ACD
	** for the following ones we need to reset the AjPVarin
	**
	** Single is set by the access method
	*/

	ajDebug("varinLoad: single access - count %d - call access"
		" routine again\n",
		input->Count);
	/* Calling funclist varinAccess() */
	if(textaccess)
        {
            if(!(*textaccess->Access)(input))
            {
                ajDebug("varinLoad: (*textaccess->Access)(varin->Input) "
                        "*failed*\n");

                return ajFalse;
            }
        }

	if(varaccess)
        {
            if(!(*varaccess->Access)(varin))
            {
                ajDebug("varinLoad: (*varaccess->Access)(varin) "
                        "*failed*\n");

                return ajFalse;
            }
        }

        buff = input->Filebuff;
    }

    ajDebug("varinLoad: varin format %d '%S'\n", input->Format,
	    input->Formatstr);

    if(!input->Filebuff)
	return ajFalse;

    ok = ajFilebuffIsBuffered(input->Filebuff);

    while(ok)
    {				/* skip blank lines */
        ok = ajBuffreadLine(input->Filebuff, &varinReadLine);

        if(!ajStrIsWhite(varinReadLine))
        {
            ajFilebuffClear(input->Filebuff,1);
            break;
        }
    }

    if(!input->Format)
    {			   /* no format specified, try all defaults */

	for(i = 1; varinFormatDef[i].Name; i++)
	{
	    if(!varinFormatDef[i].Try)	/* skip if Try is ajFalse */
		continue;

	    ajDebug("varinLoad:try format %d (%s)\n",
		    i, varinFormatDef[i].Name);

	    istat = varinLoadFmt(varin, var, i);

	    switch(istat)
	    {
	    case FMT_OK:
		ajDebug("++varinLoad OK, set format %d\n",
                        input->Format);
		varDefine(var, varin);

		return ajTrue;
	    case FMT_BADTYPE:
		ajDebug("varinLoad: (a1) "
                        "varinLoadFmt stat == BADTYPE *failed*\n");

		return ajFalse;
	    case FMT_FAIL:
		ajDebug("varinLoad: (b1) "
                        "varinLoadFmt stat == FAIL *failed*\n");
		break;			/* we can try next format */
	    case FMT_NOMATCH:
		ajDebug("varinLoad: (c1) "
                        "varinLoadFmt stat==NOMATCH try again\n");
		break;
	    case FMT_EOF:
		ajDebug("varinLoad: (d1) "
                        "varinLoadFmt stat == EOF *failed*\n");
		return ajFalse;			/* EOF and unbuffered */
	    case FMT_EMPTY:
		ajWarn("variation data '%S' has zero length, ignored",
		       ajVarGetQryS(var));
		ajDebug("varinLoad: (e1) "
                        "varinLoadFmt stat==EMPTY try again\n");
		break;
	    default:
		ajDebug("unknown code %d from varinLoadFmt\n", stat);
	    }

	    ajVarClear(var);

	    if(input->Format)
		break;			/* we read something */

            ajFilebuffTrace(input->Filebuff);
	}

	if(!input->Format)
	{		     /* all default formats failed, give up */
	    ajDebug("varinLoad:all default formats failed, give up\n");

	    return ajFalse;
	}

	ajDebug("++varinLoad set format %d\n",
                input->Format);
    }
    else
    {					/* one format specified */
	ajDebug("varinLoad: one format specified\n");
	ajFilebuffSetUnbuffered(input->Filebuff);

	ajDebug("++varinLoad known format %d\n",
                input->Format);
	istat = varinLoadFmt(varin, var, input->Format);

	switch(istat)
	{
	case FMT_OK:
	    varDefine(var, varin);

	    return ajTrue;
	case FMT_BADTYPE:
	    ajDebug("varinLoad: (a2) "
                    "varinLoadFmt stat == BADTYPE *failed*\n");

	    return ajFalse;

        case FMT_FAIL:
	    ajDebug("varinLoad: (b2) "
                    "varinLoadFmt stat == FAIL *failed*\n");

	    return ajFalse;

        case FMT_NOMATCH:
	    ajDebug("varinLoad: (c2) "
                    "varinLoadFmt stat == NOMATCH *try again*\n");
	    break;
	case FMT_EOF:
	    ajDebug("varinLoad: (d2) "
                    "varinLoadFmt stat == EOF *try again*\n");
            if(input->Records)
                ajErr("Error reading file '%F' with format '%s': "
                      "end-of-file before end of data "
                      "(read %u records)",
                      ajFilebuffGetFile(input->Filebuff),
                      varinFormatDef[input->Format].Name,
                      input->Records);
	    break;		     /* simply end-of-file */
	case FMT_EMPTY:
	    ajWarn("variation data '%S' has zero length, ignored",
		   ajVarGetQryS(var));
	    ajDebug("varinLoad: (e2) "
                    "varinLoadFmt stat == EMPTY *try again*\n");
	    break;
	default:
	    ajDebug("unknown code %d from varinReadFmt\n", stat);
	}

	ajVarClear(var); /* 1 : read, failed to match id/acc/query */
    }

    /* failed - probably entry/accession query failed. Can we try again? */

    ajDebug("varinLoad failed - try again with format %d '%s' code %d\n",
	    input->Format,
            varinFormatDef[input->Format].Name, istat);

    ajDebug("Search:%B Chunk:%B Data:%x ajFileBuffEmpty:%B\n",
	    input->Search, input->ChunkEntries,
            input->TextData, ajFilebuffIsEmpty(buff));

    if(ajFilebuffIsEmpty(buff) && input->ChunkEntries)
    {
	if(textaccess && !(*textaccess->Access)(input))
            return ajFalse;
	else if(varaccess && !(*varaccess->Access)(varin))
            return ajFalse;
        buff = input->Filebuff;
    }


    /* need to check end-of-file to avoid repeats */
    while(input->Search &&
          (input->TextData || !ajFilebuffIsEmpty(buff)))
    {
	jstat = varinLoadFmt(varin, var, input->Format);

	switch(jstat)
	{
	case FMT_OK:
	    varDefine(var, varin);

	    return ajTrue;

        case FMT_BADTYPE:
	    ajDebug("varinLoad: (a3) "
                    "varinLoadFmt stat == BADTYPE *failed*\n");

	    return ajFalse;

        case FMT_FAIL:
	    ajDebug("varinLoad: (b3) "
                    "varinLoadFmt stat == FAIL *failed*\n");

	    return ajFalse;
            
	case FMT_NOMATCH:
	    ajDebug("varinLoad: (c3) "
                    "varinLoadFmt stat == NOMATCH *try again*\n");
	    break;
	case FMT_EOF:
	    ajDebug("varinLoad: (d3) "
                    "varinLoadFmt stat == EOF *failed*\n");

	    return ajFalse;			/* we already tried again */

        case FMT_EMPTY:
	    if(istat != FMT_EMPTY)
                ajWarn("variation data '%S' has zero length, ignored",
                       ajVarGetQryS(var));
	    ajDebug("varinLoad: (e3) "
                    "varinLoadFmt stat == EMPTY *try again*\n");
	    break;

        default:
	    ajDebug("unknown code %d from varinLoadFmt\n", stat);
	}

	ajVarClear(var); /* 1 : read, failed to match id/acc/query */
    }

    if(input->Format)
	ajDebug("varinLoad: *failed* to read variation data %S "
                "using format %s\n",
		input->Qry,
                varinFormatDef[input->Format].Name);
    else
	ajDebug("varinLoad: *failed* to read variation data %S "
                "using any format\n",
		input->Qry);

    return ajFalse;
}




/* @funcstatic varinLoadBcf ***************************************************
**
** Given data in a variation structure, tries to load data
** using VCFv3.x format.
**
** @param [u] varin [AjPVarin] Variation input object
** @param [w] var [AjPVar] Variation object
** @return [AjBool] ajTrue on success
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool varinLoadBcf(AjPVarin varin, AjPVar var)
{
    AjPTextin input = varin->Input;
    AjPFilebuff buff= input->Filebuff;
    AjBool ok = ajTrue;
    AjPVarBcfFile fp = NULL;
    AjPVarBcfHeader h = NULL;
    AjOVarBcf *b = NULL;
    VarPBcfdata bcfdata = NULL;
    AjPStr s = NULL;
    ajuint n;
    ajuint nsamples = 0;
    AjBool firstsample = AJTRUE;
    ajuint iread;
    ajuint maxread = 1000;
    int bcfstat = 1;
    int nsmpl;

    ajDebug("varinLoadBcf '%F' EOF: %B records: %u "
            "dataread: %B datadone: %B hasdata: %B\n",
            ajFilebuffGetFile(buff), ajFilebuffIsEof(buff),
            input->Records, input->Dataread, input->Datadone,
            var->Hasdata);

    if(input->Datadone)
    {
        return ajFalse;
    }

    if(ajFilebuffIsEnded(buff))
    {
        ajFileSeek(ajFilebuffGetFile(buff), 0L, SEEK_END);
        return ajFalse;
    }

    if(!input->Records)
    {
        ajVarClear(var);

        fp = ajSeqBamBgzfNew(ajFilebuffGetFileptr(buff),"r");
        AJNEW0(bcfdata);
        input->TextData = bcfdata;
        bcfdata->gzfile = fp;

        bcfdata->Header = ajVarbcfHdrRead(fp);
        h = bcfdata->Header;

        ajStrTokenAssigncharC(&varHandle, h->txt, "\n");

        while (ajStrTokenNextFind(varHandle, &varinReadLine))
        {
            input->Records++;
            if(!varinReadVcf41Header(varin, var))
            {
                break;
            }
        }

        ajStrTokenReset(varHandle);

        return ajTrue;
    }

    /*
    ** On later calls, read a chunk of data
    */

    /* clear current chunk */

    ajDebug("continue processing: maxread %u Datacount %u\n",
            maxread, input->Datacount);

    ajVarReset(var);
    var->Hasdata = ajTrue;
    AJNEW0(b);
    s = ajStrNew();
    iread = 0;
    bcfdata = input->TextData;
    fp = bcfdata->gzfile;
    h = bcfdata->Header;
    nsmpl = h->n_smpl;

    while ((iread < maxread) &&
           (bcfstat >= 0))
    {
        bcfstat = ajVarbcfRead(fp, nsmpl, b);
        if(bcfstat < 0)
            break;

        ajStrAssignClear(&s);

        iread++;
	input->Records++;
	input->Entrycount++;

	ajVarbcfFmtCore(h, b, &s);
	n = varinDataVcf(s, var);

	if(firstsample)
	{
	    firstsample = ajFalse;
	    nsamples = n;
	}
	else
	{
	    if(n != nsamples)
		ajWarn("varinLoadBcf: expected %u samples, read %u",
		       nsamples, n);
	}

	varinVcfCheckLastrecordForRefseqIds(var);

    }

    input->Datacount += iread;

    ajDebug("data done ok: %B Datacount: %u iread: %u/%u\n",
            ok, input->Datacount, iread, maxread);

    input->Search = ajFalse;

    ajVarbcfHdrDel(h);
    ajVarbcfDel(b);

    ajStrDel(&s);

    if(bcfstat < 0)
    {
        input->Datadone = ajTrue;
        ajSeqBamBgzfClose(fp);
        fp=NULL;
        ajFilebuffClear(buff, -1);
        buff->File->End = ajTrue;
        AJFREE(bcfdata);
        input->TextData = NULL;
    }

    return ajTrue;
}




/* @funcstatic varinLoadVcf3x *************************************************
**
** Given data in a variation structure, tries to load data
** using VCFv3.x format.
**
** @param [u] varin [AjPVarin] Variation input object
** @param [w] var [AjPVar] Variation object
** @return [AjBool] ajTrue on success
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool varinLoadVcf3x(AjPVarin varin, AjPVar var)
{
    AjPFilebuff buff;
    AjPTextin input = varin->Input;
    AjBool ok = ajTrue;
    AjBool header = ajTrue;
    AjPTagval tagval = NULL;
    AjPVarField varfield = NULL;
    AjPVarSample varsample = NULL;
    AjPVarHeader varheader = var->Header;
    const AjPStr varname = NULL;
    AjPStr token = NULL;
    ajuint i;
    ajulong n;
    ajuint nsamples = 0;
    AjBool firstsample = AJTRUE;
    ajuint iread;
    ajuint maxread = 1000;

    buff = input->Filebuff;
    ajDebug("varinLoadBcf '%F' EOF: %B records: %u "
            "dataread: %B datadone: %B hasdata: %B\n",
            ajFilebuffGetFile(buff), ajFilebuffIsEof(buff),
            input->Records, input->Dataread, input->Datadone,
            var->Hasdata);

    if(input->Datadone)
    {
        return ajFalse;
    }

    if(!input->Records)
    {
        /* ajFilebuffTrace(buff); */

        while (header && ajBuffreadLineStore(buff, &varinReadLine,
                                             input->Text, &var->Textptr))
        {
            ajStrTrimWhiteEnd(&varinReadLine);
            input->Records++;
            if(!ajStrPrefixC(varinReadLine, "##"))
            {
                header = ajFalse;
                break;
            }
            /* ##type= */
            ajDebug("Header:\n%S\n", varinReadLine);
            if(input->Records == 1)
            {
                if(!ajStrPrefixC(varinReadLine, "##fileformat=VCFv3."))
                {
                    ajFilebuffResetStore(buff, input->Text, &var->Textptr);
                    return ajFalse;
                }
            }
            else 
            {
                /* parse ID=id,Number=number,Type=type,Description="desc" */

                if(!varfield)
                    varfield = ajVarfieldNew();

                varname = varinFieldVcf3x(varfield, varinReadLine);
                if(varname)
                    ajDebug("%S=<ID=%S,Type=%d(%s),Number=%d,"
                            "Description=\"%S\">\n",
                            varname,
                            varfield->Id, varfield->Type,
                            ajVarfieldGetType(varfield),
                            varfield->Number, varfield->Desc);
                else
                    ajDebug("..failed\n");

                if(ajStrMatchC(varname, "INFO"))
                {
                    ajListPushAppend(varheader->Fields, varfield);
                    varfield = ajVarfieldNew();
                }
                else if(ajStrMatchC(varname, "FILTER"))
                {
                    tagval = ajTagvalNewS(varfield->Id, varfield->Desc);
                    ajListPushAppend(varheader->Filters, tagval);
                    tagval = NULL;
                }
                else if(ajStrMatchC(varname, "FORMAT"))
                {
                    ajListPushAppend(varheader->Fields, varfield);
                    varfield = ajVarfieldNew();
                }
                else if(ajStrMatchC(varname, "ALT"))
                {
                    tagval = ajTagvalNewS(varfield->Id, varfield->Desc);
                    ajListPushAppend(varheader->Alts, tagval);
                    tagval = NULL;
                }
                else if(ajStrMatchC(varname, "SAMPLE"))
                {
                    varsample = varinSampleVcf(varinReadLine);
                    ajListPushAppend(varheader->Samples, varsample);
                    varsample = NULL;
                }
                else if(ajStrMatchC(varname, "PEDIGREE"))
                {
                    tagval = ajTagvalNewS(varfield->Id, varfield->Desc);
                    ajListPushAppend(varheader->Pedigrees, tagval);
                    tagval = NULL;
                }
                else
                {
                    ajDebug("other header field tag '%S'\n", varname);
                    tagval = ajTagvalNewS(varname, varfield->Desc);
                    ajListPushAppend(varheader->Header, tagval);
                    tagval = NULL;
                }
            }
        }

        if(!input->Records)
        {
            ajFilebuffResetStore(buff, input->Text, &var->Textptr);
            return ajFalse;
        }

        /* #CHROM POS ... column headers */
        ajDebug("Columns:\n%S", varinReadLine);

        if(!ajStrPrefixC(varinReadLine,
                         "#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER"
                         "\tINFO\tFORMAT"))
        {
            ajFilebuffResetStore(buff, input->Text, &var->Textptr);
            return ajFalse;
        }

        n = ajStrCalcCountK(varinReadLine, '\t') - 8;

        ajStrTokenAssignC(&varHandle, varinReadLine, "\t");
        i = 0;
        while(ajStrTokenNextParse(varHandle, &varToken))
        {
            if(++i <= 9) continue;

            token = ajStrNewS(varToken);
            ajListPushAppend(varheader->SampleIds, token);
            token = NULL;
        }

        ajVarfieldDel(&varfield);
        ajStrTokenReset(varHandle);
        ajStrDelStatic(&varToken);

        ajDebug("varinLoadVcf3x header done\n");

        return ajTrue;
    }

    /*
    ** On later calls, read a chunk of data
    */

    /* clear current chunk */

    ajDebug("continue processing: maxread %u Datacount %u\n",
            maxread, input->Datacount);

    ajVarReset(var);
    var->Hasdata = ajTrue;

    iread = 0;
    while (ok && iread < maxread)
    {
        ok = ajBuffreadLineStore(buff, &varinReadLine,
                                 input->Text, &var->Textptr);
        if(!ok) break;

        iread++;
        input->Records++;
        /* data */
        ajDebug("Data:\n%S", varinReadLine);
        if(ajStrPrefixC(varinReadLine,"#"))
        {
            ajFilebuffClearStore(buff, 1, varinReadLine,
                                 input->Text, &var->Textptr);
            return ok;
        }
        n = varinDataVcf(varinReadLine, var);
        if(firstsample)
        {
            firstsample = ajFalse;
            nsamples = (ajuint) n;
        }
        else
        {
            if(n != nsamples)
                ajWarn("VCF format: expected %u samples, read %u",
                       nsamples, n);
        }

        varinVcfCheckLastrecordForRefseqIds(var);
    }

    input->Datacount += iread;

    ajDebug("data done ok: %B Datacount: %u iread: %u/%u\n",
            ok, input->Datacount, iread, maxread);

    if(!ok)
    {
        input->Datadone = ajTrue;
        ajFilebuffClear(buff, 0);
    }

    ajVarfieldDel(&varfield);
    ajStrTokenReset(varHandle);
    ajStrDelStatic(&varToken);

    return ajTrue;
}




/* @funcstatic varinLoadVcf40 *************************************************
**
** Given data in a variation structure, tries to load data
** using VCFv4.0 format.
**
** @param [u] varin [AjPVarin] Variation input object
** @param [w] var [AjPVar] Variation object
** @return [AjBool] ajTrue on success
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool varinLoadVcf40(AjPVarin varin, AjPVar var)
{
    AjPTextin input = varin->Input;
    AjPFilebuff buff = input->Filebuff;
    AjBool ok = ajTrue;
    AjPTagval tagval = NULL;
    AjPVarField varfield = NULL;
    AjPVarSample varsample = NULL;
    AjPVarHeader varheader = var->Header;
    const AjPStr varname = NULL;
    AjPStr token = NULL;
    ajuint i;
    ajulong n;
    ajuint nsamples = 0;
    AjBool firstsample = AJTRUE;
    ajuint iread = 0;
    ajuint maxread = 1000;

    ajDebug("varinLoadVcf40 '%F' EOF: %B records: %u "
            "dataread: %B datadone: %B hasdata: %B\n",
            ajFilebuffGetFile(buff), ajFilebuffIsEof(buff),
            input->Records, input->Dataread, input->Datadone,
            var->Hasdata);

    /* ajFilebuffTrace(buff); */

    if(input->Datadone)
    {
        return ajFalse;
    }

    /*
    ** On first call, we read the header only
    */

    if(!input->Records)
    {
        while (ajBuffreadLineStore(buff, &varinReadLine,
                                   input->Text, &var->Textptr))
        {
            ajStrTrimWhiteEnd(&varinReadLine);
            input->Records++;
            if(!ajStrPrefixC(varinReadLine, "##"))
            {
                break;
            }
            /* ##type= */
            ajDebug("Header:\n%S\n", varinReadLine);
            if(input->Records == 1)
            {
                if(!ajStrPrefixC(varinReadLine, "##fileformat=VCFv4.0"))
                {
                    ajFilebuffResetStore(buff, input->Text, &var->Textptr);
                    return ajFalse;
                }
            }
            else 
            {
                /* parse ID=id,Number=number,Type=type,Description="desc" */

                if(!varfield)
                    varfield = ajVarfieldNew();

                varname = varinFieldVcf(varfield, varinReadLine);
                if(varname)
                    ajDebug("%S=<ID=%S,Type=%d(%s),Number=%d,"
                            "Description=\"%S\">\n",
                            varname,
                            varfield->Id, varfield->Type,
                            ajVarfieldGetType(varfield),
                            varfield->Number, varfield->Desc);
                else
                    ajDebug("..failed\n");

                if(ajStrMatchC(varname, "INFO"))
                {
                    ajListPushAppend(varheader->Fields, varfield);
                    varfield = NULL;
                }
                else if(ajStrMatchC(varname, "FILTER"))
                {
                    tagval = ajTagvalNewS(varfield->Id, varfield->Desc);
                    ajListPushAppend(varheader->Filters, tagval);
                    tagval = NULL;
                }
                else if(ajStrMatchC(varname, "FORMAT"))
                {
                    ajListPushAppend(varheader->Fields, varfield);
                    varfield = NULL;
                }
                else if(ajStrMatchC(varname, "ALT"))
                {
                    tagval = ajTagvalNewS(varfield->Id, varfield->Desc);
                    ajListPushAppend(varheader->Alts, tagval);
                    tagval = NULL;
                }
                else if(ajStrMatchC(varname, "SAMPLE"))
                {
                    varsample = varinSampleVcf(varinReadLine);
                    ajListPushAppend(varheader->Samples, varsample);
                    varsample = NULL;
                }
                else if(ajStrMatchC(varname, "PEDIGREE"))
                {
                    tagval = ajTagvalNewS(varfield->Id, varfield->Desc);
                    ajListPushAppend(varheader->Pedigrees, tagval);
                    tagval = NULL;
                }
                else
                {
                    ajDebug("other header field tag '%S'\n", varname);
                    tagval = ajTagvalNewS(varname, varfield->Desc);
                    ajListPushAppend(varheader->Header, tagval);
                    tagval = NULL;
                }
            }
        }

        if(!input->Records)
        {
            ajFilebuffResetStore(buff, input->Text, &var->Textptr);
            return ajFalse;
        }

        /* #CHROM POS ... column headers */
        ajDebug("Columns:\n%S", varinReadLine);

        if(!ajStrPrefixC(varinReadLine,
                         "#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER"
                         "\tINFO\tFORMAT"))
        {
            ajFilebuffResetStore(buff, input->Text, &var->Textptr);
            return ajFalse;
        }

        n = ajStrCalcCountK(varinReadLine, '\t') - 8;

        ajStrTokenAssignC(&varHandle, varinReadLine, "\t");
        i = 0;

        while(ajStrTokenNextParse(varHandle, &varToken))
        {
            if(++i <= 9) continue;

            token = ajStrNewS(varToken);
            ajListPushAppend(varheader->SampleIds, token);
            token = NULL;
        }

        ajVarfieldDel(&varfield);
        ajStrTokenReset(varHandle);
        ajStrDelStatic(&varToken);

        ajDebug("varinLoadVcf41 header done\n");

        return ajTrue;
    }

    /*
    ** On later calls, read a chunk of data
    */

    /* clear current chunk */

    ajDebug("continue processing: maxread %u Datacount %u\n",
            maxread, input->Datacount);

    ajVarReset(var);
    var->Hasdata = ajTrue;

    iread = 0;
    while (ok && iread < maxread)
    {
        ok = ajBuffreadLineStore(buff, &varinReadLine,
                                 input->Text, &var->Textptr);
        if(!ok) break;

        iread++;
        input->Records++;
        /* data */
        ajDebug("Data:\n%S", varinReadLine);
        if(ajStrPrefixC(varinReadLine,"#"))
        {
            ajFilebuffClearStore(buff, 1, varinReadLine,
                                 input->Text, &var->Textptr);
            return ok;
        }
        n = varinDataVcf(varinReadLine, var);
        if(firstsample)
        {
            firstsample = ajFalse;
            nsamples = (ajuint) n;
        }
        else
        {
            if(n != nsamples)
                ajWarn("VCF format: expected %u samples, read %u",
                       nsamples, n);
        }

        varinVcfCheckLastrecordForRefseqIds(var);
    }

    input->Datacount += iread;

    ajDebug("data done ok: %B Datacount: %u iread: %u/%u\n",
            ok, input->Datacount, iread, maxread);

    if(!ok)
    {
        input->Datadone = ajTrue;
        ajFilebuffClear(buff, 0);
    }

    ajVarfieldDel(&varfield);
    ajStrTokenReset(varHandle);
    ajStrDelStatic(&varToken);

    return ajTrue;
}




/* @funcstatic varinReadVcf41Header *******************************************
**
** Tries reading VCF header lines using VCFv4.1 format.
**
** @param [u] varin [AjPVarin] Variation input object
** @param [w] var [AjPVar] Variation object
** @return [AjBool] ajTrue on success
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool varinReadVcf41Header(AjPVarin varin, AjPVar var)
{
    AjPFilebuff buff;
    AjPTextin input = varin->Input;
    AjPTagval tagval = NULL;
    AjPVarField varfield = NULL;
    AjPVarSample varsample = NULL;
    AjPVarHeader varheader = var->Header;
    const AjPStr varname = NULL;

    ajDebug("varinReadVcf41Header\n");

    buff = input->Filebuff;

    ajStrTrimWhiteEnd(&varinReadLine);
    input->Records++;
    if(!ajStrPrefixC(varinReadLine, "##"))
    {
        return ajFalse;
    }
    /* ##type= */
    ajDebug("Header:\n%S\n", varinReadLine);

    if(input->Records == 1)
    {
        if(!ajStrPrefixC(varinReadLine, "##fileformat=VCFv4.1"))
        {
            ajFilebuffResetStore(buff, input->Text, &var->Textptr);
            return ajFalse;
        }
    }
    else
    {
        /* parse ID=id,Number=number,Type=type,Description="desc" */

	varfield = ajVarfieldNew();

        varname = varinFieldVcf(varfield, varinReadLine);
        if(varname)
            ajDebug("%S=<ID=%S,Type=%d(%s),Number=%d,Description=\"%S\">\n",
                    varname,
                    varfield->Id, varfield->Type,
                    ajVarfieldGetType(varfield),
                    varfield->Number, varfield->Desc);
        else
            ajDebug("..failed\n");

        if(ajStrMatchC(varname, "INFO"))
        {
            ajListPushAppend(varheader->Fields, varfield);
            varfield = NULL;
        }
        else if(ajStrMatchC(varname, "FILTER"))
        {
            tagval = ajTagvalNewS(varfield->Id, varfield->Desc);
            ajListPushAppend(varheader->Filters, tagval);
            tagval = NULL;
        }
        else if(ajStrMatchC(varname, "FORMAT"))
        {
            ajListPushAppend(varheader->Fields, varfield);
            varfield = NULL;
        }
        else if(ajStrMatchC(varname, "ALT"))
        {
            tagval = ajTagvalNewS(varfield->Id, varfield->Desc);
            ajListPushAppend(varheader->Alts, tagval);
            tagval = NULL;
        }
        else if(ajStrMatchC(varname, "SAMPLE"))
        {
            varsample = varinSampleVcf(varinReadLine);
            ajListPushAppend(varheader->Samples, varsample);
            varsample = NULL;
        }
        else if(ajStrMatchC(varname, "PEDIGREE"))
        {
            tagval = ajTagvalNewS(varfield->Id, varfield->Desc);
            ajListPushAppend(varheader->Pedigrees, tagval);
            tagval = NULL;
        }
        /*TODO contig lines */
        else
        {
            ajDebug("other header field tag '%S'\n", varname);
            tagval = ajTagvalNewS(varname, varfield->Desc);
            ajListPushAppend(varheader->Header, tagval);
            tagval = NULL;
        }

        ajVarfieldDel(&varfield);
    }

    return ajTrue;
}




/* @funcstatic varinLoadVcf41 *************************************************
**
** Given data in a variation structure, tries to read the next chunk
** using VCFv4.1 format.
**
** @param [u] varin [AjPVarin] Variation input object
** @param [w] var [AjPVar] Variation object
** @return [AjBool] ajTrue on success
**
** @release 6.5.0
** @@
******************************************************************************/

static AjBool varinLoadVcf41(AjPVarin varin, AjPVar var)
{
    AjPTextin input = varin->Input;
    AjPFilebuff buff = input->Filebuff;
    AjBool ok = ajTrue;
    AjPVarField varfield = NULL;
    AjPVarHeader varheader = var->Header;
    AjPStr token = NULL;
    ajuint i;
    ajulong n;
    ajuint nsamples = 0;
    AjBool firstsample = AJTRUE;
    ajuint iread = 0;
    ajuint maxread = 1000;

    ajDebug("varinLoadVcf41 '%F' EOF: %B records: %u "
            "dataread: %B datadone: %B hasdata: %B\n",
            ajFilebuffGetFile(buff), ajFilebuffIsEof(buff),
            input->Records, input->Dataread, input->Datadone,
            var->Hasdata);


    /* ajFilebuffTrace(buff); */

    if(input->Datadone)
    {
        return ajFalse;
    }

    /*
    ** On first call, we read the header only
    */

    if(!input->Records)
    {
        ajVarClear(var);
        while (ajBuffreadLineStore(buff, &varinReadLine,
                                   input->Text, &var->Textptr))
        {
            if(!varinReadVcf41Header(varin, var))
            {
                break;
            }
        }

        if(!input->Records)
        {
            ajFilebuffResetStore(buff, input->Text, &var->Textptr);
            return ajFalse;
        }

        /* #CHROM POS ... column headers */
        ajDebug("Columns:\n%S\n", varinReadLine);

        if(!ajStrPrefixC(varinReadLine,
                         "#CHROM\tPOS\tID\tREF\tALT\tQUAL\tFILTER"
                         "\tINFO\tFORMAT"))
        {
            ajFilebuffResetStore(buff, input->Text, &var->Textptr);
            return ajFalse;
        }

        n = ajStrCalcCountK(varinReadLine, '\t') - 8;

        ajStrTokenAssignC(&varHandle, varinReadLine, "\t");
        i = 0;

        while(ajStrTokenNextParse(varHandle, &varToken))
        {
            if(++i <= 9) continue;

            token = ajStrNewS(varToken);
            ajListPushAppend(varheader->SampleIds, token);
            token = NULL;
        }

        ajVarfieldDel(&varfield);
        ajStrTokenReset(varHandle);
        ajStrDelStatic(&varToken);

        ajDebug("varinLoadVcf41 header done\n");
        return ajTrue;
    }
    
    /*
    ** On later calls, read a chunk of data
    */

    /* clear current chunk */

    ajDebug("continue processing: maxread %u Datacount %u\n",
            maxread, input->Datacount);

    ajVarReset(var);
    var->Hasdata = ajTrue;

    iread = 0;
    while (ok && iread < maxread)
    {
        ok = ajBuffreadLineStore(buff, &varinReadLine,
                                 input->Text, &var->Textptr);
        if(!ok) break;

        iread++;
        input->Records++;

        /* data */
        ajDebug("Data:\n%S", varinReadLine);

        if(ajStrPrefixC(varinReadLine,"#"))
        {
            ajFilebuffClearStore(buff, 1, varinReadLine,
                                 input->Text, &var->Textptr);
            input->Datadone = ajTrue;
            return ok;
        }

        n = varinDataVcf(varinReadLine, var);
        if(firstsample)
        {
            firstsample = ajFalse;
            nsamples = (ajuint) n;
        }
        else
        {
            if(n != nsamples)
                ajWarn("VCFformat: expected %u samples, read %u",
                       nsamples, n);
        }

        varinVcfCheckLastrecordForRefseqIds(var);
    }

    input->Datacount += iread;

    ajDebug("data done ok: %B Datacount: %u iread: %u/%u\n",
            ok, input->Datacount, iread, maxread);

    if(!ok)
    {
        input->Datadone = ajTrue;
        ajFilebuffClear(buff, 0);
    }

    ajVarfieldDel(&varfield);
    ajStrTokenReset(varHandle);
    ajStrDelStatic(&varToken);

    return ajTrue;
}




/* @funcstatic varinVcfCheckLastrecordForRefseqIds ****************************
**
** Check last record, whether its reference sequence was recorded
** in the RefSeqIds table
**
** @param [u] var [AjPVar] Variation object
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void varinVcfCheckLastrecordForRefseqIds(AjPVar var)
{
    int* tid = NULL;
    const int* ctid = NULL;
    AjPVarData vardata = NULL;
    AjPVarHeader varheader = var->Header;


    if(ajListPeekLast(var->Data, (void**)&vardata))
        ctid = (const int*)ajTableFetchS(varheader->RefseqIds, vardata->Chrom);

    if(!ctid)
    {
	AJNEW0(tid);
	*tid = (ajuint) ajTableGetLength(varheader->RefseqIds);
	ajTablePut(varheader->RefseqIds, ajStrNewS(vardata->Chrom), tid);
	ctid=tid;
    }

    return;
}




/* @funcstatic varinFieldVcf **************************************************
**
** Reads field data from a VCF header record
**
** @param [u] var [AjPVarField] Variation field object
** @param [r] str [const AjPStr] Input header record
** @return [const AjPStr] Field name
**
** @release 6.5.0
** @@
******************************************************************************/

static const AjPStr varinFieldVcf(AjPVarField var, const AjPStr str)
{
    ajlong ipos;
    AjPStr tmpstr = NULL;
    ajuint i;
    AjBool isknown = ajFalse;

    /* PEDIGREE not listed: has no ID field */

    const char* fieldnames[] = {
        "INFO", "FILTER", "FORMAT", "ALT", "SAMPLE", NULL
    };

    if(!VarInitVcf)
        varRegInitVcf();

    ajVarfieldClear(var);

    if(!ajStrPrefixC(str, "##"))
        return NULL;

    ipos = ajStrFindAnyK(str, '=');
    if(ipos < 3)
    {
        /*valid as a comment in VCF 3.x */
        ajDebug("no equal, pos:%u '%S'\n", ipos, var->Desc);
        ajStrAssignSubS(&var->Desc, str, 2, -1);
        ajStrAssignC(&varFieldName, "comment");
        return varFieldName;
    }

    ajStrAssignSubS(&varFieldName, str, 2, ipos-1);

    for(i=0; fieldnames[i]; i++)
        if(ajStrMatchC(varFieldName, fieldnames[i]))
            isknown = ajTrue;
    
    if(!isknown)
    {
        ajStrAssignSubS(&var->Desc, str, ipos+1, -1);
        ajDebug("simple desc '%S'\n", var->Desc);
        return varFieldName;
    }

    if(!ajRegExec(varRegId, str))
        return NULL;
    ajRegSubI(varRegId, 1, &var->Id);
    ajDebug("id '%S'\n", var->Id);

    if(ajRegExec(varRegDesc, str))
    {
        ajRegSubI(varRegDesc, 1, &var->Desc);
        ajDebug("Desc '%S'\n", var->Desc);
    }
    else
    {
        if(!ajRegExec(varRegDesc2, str))
            return NULL;
        ajRegSubI(varRegDesc2, 1, &var->Desc);
        ajDebug("Desc2 '%S'\n", var->Desc);
    }

    if(!ajStrMatchC(varFieldName, "INFO") &&
       !ajStrMatchC(varFieldName, "FORMAT"))
        return varFieldName;

    if(!ajRegExec(varRegNumber, str))
        return varFieldName;    /* valid for v3.x */

    ajRegSubI(varRegNumber, 1, &tmpstr);
    ajDebug("Number '%S'\n", tmpstr);

    switch(ajStrGetCharFirst(tmpstr))
    {
        case 'A':
            var->Number = varNumAlt;
            break;
        case 'G':
            var->Number = varNumGen;
            break;
        case '.':
            var->Number = varNumAny;
            break;
        default:
            if(!ajStrToInt(tmpstr, &var->Number))
                return NULL;
            break;
    }
        
    if(!ajRegExec(varRegType, str))
        return NULL;
    ajRegSubI(varRegType, 1, &tmpstr);
    ajDebug("Type '%S'\n", tmpstr);

    if(ajStrMatchC(tmpstr, "Integer"))
        var->Type = AJVAR_INT;
    else if(ajStrMatchC(tmpstr, "Float"))
        var->Type = AJVAR_FLOAT;
    else if(ajStrMatchC(tmpstr, "Flag"))
        var->Type = AJVAR_FLAG;
    else if(ajStrMatchC(tmpstr, "Character"))
        var->Type = AJVAR_CHAR;
    else if(ajStrMatchC(tmpstr, "String"))
        var->Type = AJVAR_STR;
    else
        return NULL;

    ajStrDel(&tmpstr);

    ajStrAssignS(&var->Field, varFieldName);

    return varFieldName;
}




/* @funcstatic varinFieldVcf3x ************************************************
**
** Reads field data from a VCF version 3.x header record
**
** @param [u] var [AjPVarField] Variation field object
** @param [r] str [const AjPStr] Input header record
** @return [const AjPStr] Field name
**
** @release 6.5.0
** @@
******************************************************************************/

static const AjPStr varinFieldVcf3x(AjPVarField var, const AjPStr str)
{
    ajlong ipos;
    AjPStr tmpstr = NULL;
    ajuint i;
    AjBool isknown = ajFalse;

    /* PEDIGREE not listed: has no ID field */

    const char* fieldnames[] = {
        "INFO", "FILTER", "FORMAT", NULL
    };

    ajVarfieldClear(var);

    if(!ajStrPrefixC(str, "##"))
        return NULL;

    ipos = ajStrFindAnyK(str, '=');
    if(ipos < 3)
    {
        /*valid as a comment in VCF 3.x */
        ajDebug("no equal, pos:%u '%S'\n", ipos, var->Desc);
        ajStrAssignSubS(&var->Desc, str, 2, -1);
        ajStrAssignC(&varFieldName, "comment");
        return varFieldName;
    }

    ajStrAssignSubS(&varFieldName, str, 2, ipos-1);

    for(i=0; fieldnames[i]; i++)
        if(ajStrMatchC(varFieldName, fieldnames[i]))
            isknown = ajTrue;
    
    if(!isknown)
    {
        ajStrAssignSubS(&var->Desc, str, ipos+1, -1);
        ajDebug("simple desc '%S'\n", var->Desc);
        return varFieldName;
    }

    ajStrTokenAssignC(&varFieldHandle, varinReadLine, "=");

    if(!ajStrTokenNextParse(varFieldHandle, &varFieldToken))
    {
        ajStrDelStatic(&varFieldToken);
        ajStrTokenReset(varFieldHandle);
        return NULL;
    }

    if(!ajStrTokenNextParseC(varFieldHandle, ",", &var->Id))
    {
        ajStrDelStatic(&varFieldToken);
        ajStrTokenReset(varFieldHandle);
        return NULL;
    }
    
    ajDebug("id '%S'\n", var->Id);

    if(ajStrMatchC(varFieldName, "FILTER"))
    {
        ajStrTokenRestParse(varFieldHandle, &var->Desc);
        ajStrTrimC(&var->Desc, "\"");
        ajStrDelStatic(&varFieldToken);
        ajStrTokenReset(varFieldHandle);
        return varFieldName;
    }
    
    if(!ajStrTokenNextParse(varFieldHandle, &varFieldToken))
    {
        ajStrDelStatic(&varFieldToken);
        ajStrTokenReset(varFieldHandle);
        return NULL;
    }
    
    ajDebug("Number '%S'\n", tmpstr);

    switch(ajStrGetCharFirst(tmpstr))
    {
        case 'A':
            var->Number = varNumAlt;
            break;
        case 'G':
            var->Number = varNumGen;
            break;
        case '.':
            var->Number = varNumAny;
            break;
        default:
            if(!ajStrToInt(tmpstr, &var->Number))
            {
                ajStrDelStatic(&varFieldToken);
                ajStrTokenReset(varFieldHandle);
                return NULL;
            }
            break;
    }
        
    if(!ajStrTokenNextParse(varFieldHandle, &varFieldToken))
    {
        ajStrDelStatic(&varFieldToken);
        ajStrTokenReset(varFieldHandle);
        return NULL;
    }

    ajDebug("Type '%S'\n", tmpstr);
    ajStrTokenRestParse(varFieldHandle, &var->Desc);
    ajStrTrimC(&var->Desc, "\"");

    if(ajStrMatchC(tmpstr, "Integer"))
        var->Type = AJVAR_INT;
    else if(ajStrMatchC(tmpstr, "Float"))
        var->Type = AJVAR_FLOAT;
    else if(ajStrMatchC(tmpstr, "Flag"))
        var->Type = AJVAR_FLAG;
    else if(ajStrMatchC(tmpstr, "Character"))
        var->Type = AJVAR_CHAR;
    else if(ajStrMatchC(tmpstr, "String"))
        var->Type = AJVAR_STR;
    else
        return NULL;


    ajStrDelStatic(&varFieldToken);
    ajStrTokenReset(varFieldHandle);

    ajStrAssignS(&var->Field, varFieldName);

    return varFieldName;
}




/* @funcstatic varinSampleVcf *************************************************
**
** Parses a VCF header SAMPLE record
**
** @param [r] str [const AjPStr] Input record
** @return [AjPVarSample] Variation sample object
**
** @release 6.5.0
** @@
******************************************************************************/

static AjPVarSample varinSampleVcf(const AjPStr str)
{
    AjPVarSample varsample = NULL;
    ajuint ipos;
    AjPStr tmpstr = NULL;
    ajuint n;

    if(!VarInitVcf)
        varRegInitVcf();

    if(!ajStrPrefixC(str, "##SAMPLE"))
        return NULL;

    ipos = (ajuint) ajStrFindAnyK(str, '=');
    if(ipos < 3)
        return NULL;

    if(!ajRegExec(varRegId, str))
        return NULL;

    varsample = ajVarsampleNew();

    ajRegSubI(varRegId, 1, &varsample->Id);
    ajDebug("id '%S'\n", varsample->Id);

    if(ajRegExec(varRegDesc, str))
    {
        ajRegSubI(varRegDesc, 1, &tmpstr);
        ajDebug("Desc '%S'\n", tmpstr);
    }
    else
    {
        if(!ajRegExec(varRegDesc2, str))
            return NULL;
        ajRegSubI(varRegDesc2, 1, &tmpstr);
        ajDebug("Desc2 '%S'\n", tmpstr);
    }
    varsample->Number = varinParseListVcf(tmpstr, &varsample->Desc);

    if(!ajRegExec(varRegGenomes, str))
        return NULL;
    ajRegSubI(varRegGenomes, 1, &tmpstr);
    ajDebug("Genomes '%S'\n", tmpstr);
    n = varinParseListVcf(tmpstr, &varsample->Genomes);
    if(n != varsample->Number)
    {
        ajWarn("expected %u genomes, read %u",
               varsample->Number, n);
        return NULL;
    }

    if(!ajRegExec(varRegMixture, str))
        return NULL;
    ajRegSubI(varRegMixture, 1, &tmpstr);
    ajDebug("Mixture '%S'\n", tmpstr);
    n = varinParseFloatVcf(tmpstr, &varsample->Mixture);
    if(n != varsample->Number)
    {
        ajWarn("expected %u mixtures, read %u",
               varsample->Number, n);
        return NULL;
    }

    ajStrDel(&tmpstr);

    return varsample;
}




/* @funcstatic varinParseFloatVcf *********************************************
**
** Parse an array of semicolon-delimited floating point values
**
** @param [r] str [const AjPStr] Input string
** @param [w] Parray [float**] Array of strings
** @return [ajuint] Number of values parsed
**
******************************************************************************/

static ajuint varinParseFloatVcf(const AjPStr str, float **Parray)
{
    ajuint ret;
    ajuint i = 0;
    ajlong ipos = 0;
    ajlong lastpos = 0;
    AjPStr tmpstr = NULL;

    AJFREE(*Parray);
    ret = 1 + (ajuint) ajStrCalcCountK(str, ';');
    AJCNEW(*Parray, ret);

    for(i=0; i < ret; i++)
    {
        ipos = ajStrFindNextK(str, lastpos, ';');
        if(ipos < 0)
            ipos = ajStrGetLen(str)+1;
        ajStrAssignSubS(&tmpstr, str, lastpos, ipos-1);
        ajStrToFloat(tmpstr, &(*Parray)[i]);
        lastpos = ipos+1;
    }

    for(i=0; i < ret; i++)
    {
        ajDebug("Parsed float[%u] '%f'\n", i, (*Parray)[i]);
    }

    ajStrDel(&tmpstr);

    return ret;
}




/* @funcstatic varinParseListVcf **********************************************
**
** Parse an array of semicolon-delimited strings
**
** @param [r] str [const AjPStr] Input string
** @param [w] Parray [AjPStr**] Array of strings
** @return [ajuint] Number of strings parsed
**
******************************************************************************/

static ajuint varinParseListVcf(const AjPStr str, AjPStr **Parray)
{
    ajuint ret;
    ajuint i = 0;
    ajuint n = 0;
    ajlong ipos = 0;
    ajlong lastpos = 0;

    ajStrDelarray(&(*Parray));
    n = 1 + (ajuint) ajStrCalcCountK(str, ';');
    ret = n + 1;
    AJCNEW(*Parray, ret);

    for(i=0; i < n; i++)
    {
        ipos = ajStrFindNextK(str, lastpos, ';');
        if(ipos < 0)
            ipos = ajStrGetLen(str)+1;
        ajStrAssignSubS(&(*Parray)[i], str, lastpos, ipos-1); 
        lastpos = ipos+1;
    }

    for(i=0; i < n; i++)
    {
        ajDebug("Parsed str[%u] '%S'\n", i, (*Parray)[i]);
    }

    return n;
}




/* @funcstatic varinDataVcf ***************************************************
**
** Parse a VCF file data record
**
** @param [r] str [const AjPStr] INput data record
** @param [u] var [AjPVar] Variation data object
** @return [ajuint] Number of sample columns
**
******************************************************************************/

static ajuint varinDataVcf(const AjPStr str, AjPVar var)
{
    AjPVarData vardata = ajVardataNew();
    AjPStr token = NULL;
    ajuint n;
    ajuint i = 0;
 
    n = (ajuint) ajStrCalcCountK(str, '\t') - 8;

    ajStrTokenAssignC(&varDataHandle, str, "\t\n");
    while(ajStrTokenNextParse(varDataHandle, &varDataToken))
    {
        switch (i++) 
        {
            case 0:             /* CHROM */
                ajStrAssignS(&vardata->Chrom, varDataToken);
                break;
            case 1:             /* POS */
                ajStrToUint(varDataToken, &vardata->Pos);
                break;
            case 2:             /* ID */
                ajStrAssignS(&vardata->Id, varDataToken);
                break;
            case 3:             /* REF */
                ajStrAssignS(&vardata->Ref, varDataToken);
                break;
            case 4:             /* ALT */
                ajStrAssignS(&vardata->Alt, varDataToken);
                break;
            case 5:             /* QUAL */
                ajStrAssignS(&vardata->Qual, varDataToken);
                break;
            case 6:             /* FILTER */
                ajStrAssignS(&vardata->Filter, varDataToken);
                break;
            case 7:             /* INFO */
                ajStrAssignS(&vardata->Info, varDataToken);
                break;
            case 8:             /* FORMAT */
                ajStrAssignS(&vardata->Format, varDataToken);
                break;
            default:            /* sample(s) */
                token = ajStrNewS(varDataToken);
                ajListPushAppend(vardata->Samples, token);
                token = NULL;
                break;
        }
    }

    ajListPushAppend(var->Data, vardata);
    vardata = NULL;

    ajStrDelStatic(&varDataToken);
    ajStrTokenReset(varDataHandle);

    return n;
}




/* @datasection [none] Miscellaneous ******************************************
**
** Variation input internals
**
** @nam2rule Varin Variation input
**
******************************************************************************/




/* @section Printing **********************************************************
**
** Printing details of the internals to a file
**
** @fdata [none]
**
** @nam2rule Varinprint
**
** @fcategory output
**
******************************************************************************/




/* @section Print *************************************************************
**
** Printing to a file
**
** @fdata [none]
**
** @nam3rule Book Print as docbook table
** @nam3rule Html Print as html table
** @nam3rule Wiki Print as wiki table
** @nam3rule Text Print as text
**
** @argrule * outf [AjPFile] output file
** @argrule Text full [AjBool] Print all details
**
** @valrule * [void]
**
** @fcategory cast
**
******************************************************************************/




/* @func ajVarinprintBook *****************************************************
**
** Reports the internal data structures as a Docbook table
**
** @param [u] outf [AjPFile] Output file
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajVarinprintBook(AjPFile outf)
{
    ajuint i = 0;
    ajuint j = 0;
    AjPStr namestr = NULL;
    AjPList fmtlist;
    AjPStr* names;

    fmtlist = ajListstrNew();

    ajFmtPrintF(outf, "<para>The supported variation formats are summarised "
                "in the table below. "
                "The columns are as follows: "
                "<emphasis>Input format</emphasis> (format name), "
                "<emphasis>Try</emphasis> (indicates whether the "
                "format can be detected automatically on input), and "
                "<emphasis>Description</emphasis> (short description of "
                "the format).</para>\n\n");

    ajFmtPrintF(outf, "<table frame=\"box\" rules=\"cols\">\n");
    ajFmtPrintF(outf, "  <caption>Input variation formats</caption>\n");
    ajFmtPrintF(outf, "  <thead>\n");
    ajFmtPrintF(outf, "    <tr align=\"center\">\n");
    ajFmtPrintF(outf, "      <th>Input Format</th>\n");
    ajFmtPrintF(outf, "      <th>Try</th>\n");
    ajFmtPrintF(outf, "      <th>Description</th>\n");
    ajFmtPrintF(outf, "    </tr>\n");
    ajFmtPrintF(outf, "  </thead>\n");
    ajFmtPrintF(outf, "  <tbody>\n");

    for(i=1; varinFormatDef[i].Name; i++)
    {
	if(!varinFormatDef[i].Alias)
        {
            namestr = ajStrNewC(varinFormatDef[i].Name);
            ajListPushAppend(fmtlist, namestr);
            namestr = NULL;
        }
    }

    ajListSort(fmtlist, &ajStrVcmp);
    ajListstrToarray(fmtlist, &names);

    for(i=0; names[i]; i++)
    {
        for(j=0; varinFormatDef[j].Name; j++)
        {
            if(ajStrMatchC(names[i],varinFormatDef[j].Name))
            {
                ajFmtPrintF(outf, "    <tr>\n");
                ajFmtPrintF(outf, "      <td>%s</td>\n",
                            varinFormatDef[j].Name);
                ajFmtPrintF(outf, "      <td>%B</td>\n",
                            varinFormatDef[j].Try);
                ajFmtPrintF(outf, "      <td>%s</td>\n",
                            varinFormatDef[j].Desc);
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




/* @func ajVarinprintHtml *****************************************************
**
** Reports the internal data structures as an HTML table
**
** @param [u] outf [AjPFile] Output file
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajVarinprintHtml(AjPFile outf)
{
    ajuint i = 0;
    ajuint j = 0;

    AjPStr namestr = NULL;

    ajFmtPrintF(outf, "<table border=3>");
    ajFmtPrintF(outf, "<tr><th>Input Format</th><th>Auto</th>\n");
    ajFmtPrintF(outf, "<th>Multi</th><th>Description</th></tr>\n");

    for(i=1; varinFormatDef[i].Name; i++)
    {
        ajStrAssignC(&namestr, varinFormatDef[i].Name);

	if(!varinFormatDef[i].Alias)
        {
            for(j=i+1; varinFormatDef[j].Name; j++)
            {
                if(varinFormatDef[j].Load == varinFormatDef[i].Load)
                {
                    ajFmtPrintAppS(&namestr, " %s",
                                   varinFormatDef[j].Name);
                    if(!varinFormatDef[j].Alias) 
                    {
                        ajWarn("Input format '%s' same as '%s' but not alias",
                               varinFormatDef[j].Name,
                               varinFormatDef[i].Name);
                    }
                }
            }

	    ajFmtPrintF(outf, "<tr><td>\n%S\n</td><td>%B</td>\n",
                        namestr,
			varinFormatDef[i].Try);
            ajFmtPrintF(outf, "<td>\n%s\n</td></tr>\n",
			varinFormatDef[i].Desc);
        }

    }

    ajFmtPrintF(outf, "</table>\n");
    ajStrDel(&namestr);

    return;
}




/* @func ajVarinprintText *****************************************************
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

void ajVarinprintText(AjPFile outf, AjBool full)
{
    ajuint i = 0;

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# Variation input formats\n");
    ajFmtPrintF(outf, "# Name  Format name (or alias)\n");
    ajFmtPrintF(outf, "# Alias Alias name\n");
    ajFmtPrintF(outf, "# Try   Test for unknown input files\n");
    ajFmtPrintF(outf, "# Name         Alias Try "
		"Description");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "InFormat {\n");

    for(i=0; varinFormatDef[i].Name; i++)
	if(full || !varinFormatDef[i].Alias)
	    ajFmtPrintF(outf,
			"  %-12s %5B %3B \"%s\"\n",
			varinFormatDef[i].Name,
			varinFormatDef[i].Alias,
			varinFormatDef[i].Try,
			varinFormatDef[i].Desc);

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajVarinprintWiki *****************************************************
**
** Reports the internal data structures as a wiki table
**
** @param [u] outf [AjPFile] Output file
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajVarinprintWiki(AjPFile outf)
{
    ajuint i = 0;
    ajuint j = 0;

    AjPStr namestr = NULL;

    ajFmtPrintF(outf, "{| class=\"wikitable sortable\" border=\"2\"\n");
    ajFmtPrintF(outf, "|-\n");
    ajFmtPrintF(outf, "!Format!!Try!!"
                "class=\"unsortable\"|Description\n");

    for(i=1; varinFormatDef[i].Name; i++)
    {
        ajStrAssignC(&namestr, varinFormatDef[i].Name);

	if(!varinFormatDef[i].Alias)
        {
            for(j=i+1; varinFormatDef[j].Name; j++)
            {
                if(varinFormatDef[j].Load == varinFormatDef[i].Load)
                {
                    ajFmtPrintAppS(&namestr, "<br>%s",
                                   varinFormatDef[j].Name);
                    if(!varinFormatDef[j].Alias) 
                    {
                        ajWarn("Input format '%s' same as '%s' but not alias",
                               varinFormatDef[j].Name,
                               varinFormatDef[i].Name);
                    }
                }
            }

            ajFmtPrintF(outf, "|-\n");
	    ajFmtPrintF(outf,
			"|%S||%B||%s\n",
			namestr,
			varinFormatDef[i].Try,
			varinFormatDef[i].Desc);
        }

    }

    ajFmtPrintF(outf, "|}\n\n");
    ajStrDel(&namestr);

    return;
}




/* @section Miscellaneous *****************************************************
**
** Functions to initialise and clean up internals
**
** @fdata [none]
**
** @nam3rule Exit Clean up and exit
**
** @valrule * [void]
**
** @fcategory misc
**
******************************************************************************/




/* @func ajVarinExit **********************************************************
**
** Cleans up variation input internal memory
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajVarinExit(void)
{
    ajStrDel(&varinReadLine);
    ajStrDel(&varFieldName);

    ajTableDel(&varDbMethods);

    ajRegFree(&varRegId);
    ajRegFree(&varRegNumber);
    ajRegFree(&varRegType);
    ajRegFree(&varRegDesc);
    ajRegFree(&varRegDesc2);
    ajRegFree(&varRegGenomes);
    ajRegFree(&varRegMixture);

    ajStrTokenDel(&varHandle);
    ajStrTokenDel(&varDataHandle);
    ajStrTokenDel(&varFieldHandle);

    ajStrDel(&varToken);
    ajStrDel(&varDataToken);
    ajStrDel(&varFieldToken);

    VarInitVcf = ajFalse;

    return;
}




/* @section Internals *********************************************************
**
** Functions to return internal values
**
** @fdata [none]
**
** @nam3rule Type Internals for variation datatype
** @nam4rule Get  Return a value
** @nam5rule Fields  Known query fields for ajVarinRead
** @nam5rule Qlinks  Known query link operators for ajVarinRead
**
** @valrule * [const char*] Internal value
**
** @fcategory misc
**
******************************************************************************/




/* @func ajVarinTypeGetFields *************************************************
**
** Returns the list of known field names for ajVarinRead
**
** @return [const char*] List of field names
**
** @release 6.4.0
** @@
******************************************************************************/

const char* ajVarinTypeGetFields(void)
{
    return "id acc";
}




/* @func ajVarinTypeGetQlinks *************************************************
**
** Returns the listof known query link operators for ajVarinRead
**
** @return [const char*] List of field names
**
** @release 6.4.0
** @@
******************************************************************************/

const char* ajVarinTypeGetQlinks(void)
{
    return "|";
}




/* @datasection [AjPTable] Internal call register table ***********************
**
** Functions to manage the internal call register table that links the
** ajaxdb library functions with code in the core AJAX library.
**
** @nam2rule Varaccess Functions to manage vardb call tables.
**
******************************************************************************/




/* @section Cast **************************************************************
**
** Return a reference to the call table
**
** @fdata [AjPTable] vardb functions call table
**
** @nam3rule Get Return a value
** @nam4rule Db Database access functions table
** @nam3rule Method Lookup an access method by name
** @nam4rule Test Return true if the access method exists
** @nam4rule MethodGet Return a method value
** @nam5rule Qlinks Return known query links for a named method
** @nam5rule Scope Return scope (entry, query or all) for a named method
**
** @argrule Method method [const AjPStr] Method name
**
** @valrule *Db [AjPTable] Call table of function names and references
** @valrule *Qlinks [const char*] Query link operators
** @valrule *Scope [ajuint] Scope flags
** @valrule *Test [AjBool] True if found
**
** @fcategory cast
**
******************************************************************************/




/* @func ajVaraccessGetDb *****************************************************
**
** Returns the table in which variation database access details are registered
**
** @return [AjPTable] Access functions hash table
**
** @release 6.4.0
** @@
******************************************************************************/

AjPTable ajVaraccessGetDb(void)
{
    if(!varDbMethods)
        varDbMethods = ajCallTableNew();
    return varDbMethods;
    
}




/* @func ajVaraccessMethodGetQlinks *******************************************
**
** Tests for a named method for variation data reading returns the 
** known query link operators
**
** @param [r] method [const AjPStr] Method required.
** @return [const char*] Known link operators
**
** @release 6.4.0
** @@
******************************************************************************/

const char* ajVaraccessMethodGetQlinks(const AjPStr method)
{
    AjPVarAccess methoddata; 

    methoddata = ajCallTableGetS(varDbMethods, method);
    if(!methoddata)
        return NULL;

    return methoddata->Qlink;
}




/* @func ajVaraccessMethodGetScope ********************************************
**
** Tests for a named method for variation data reading and returns the scope
** (entry, query or all).
*
** @param [r] method [const AjPStr] Method required.
** @return [ajuint] Scope flags
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ajVaraccessMethodGetScope(const AjPStr method)
{
    AjPVarAccess methoddata; 
    ajuint ret = 0;

    methoddata = ajCallTableGetS(varDbMethods, method);
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




/* @func ajVaraccessMethodTest ************************************************
**
** Tests for a named method for variation data reading.
**
** @param [r] method [const AjPStr] Method required.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajVaraccessMethodTest(const AjPStr method)
{
    if(ajCallTableGetS(varDbMethods, method))
      return ajTrue;

    return ajFalse;
}




/* @funcstatic varinQryRestore ************************************************
**
** Restores a variation input specification from an AjPQueryList node
**
** @param [w] varin [AjPVarin] Variation input object
** @param [r] node [const AjPQueryList] Query list node
** @return [void]
**
** @release 6.4.0
******************************************************************************/

static void varinQryRestore(AjPVarin varin, const AjPQueryList node)
{
    AjPTextin input = varin->Input;

    input->Format = node->Format;
    input->Fpos   = node->Fpos;
    ajStrAssignS(&input->Formatstr, node->Formatstr);
    ajStrAssignS(&input->QryFields, node->QryFields);

    return;
}




/* @funcstatic varinQrySave ***************************************************
**
** Saves a variation input specification in an AjPQueryList node
**
** @param [w] node [AjPQueryList] Query list node
** @param [r] varin [const AjPVarin] Variation input object
** @return [void]
**
** @release 6.4.0
******************************************************************************/

static void varinQrySave(AjPQueryList node, const AjPVarin varin)
{
    AjPTextin input = varin->Input;

    node->Format   = input->Format;
    node->Fpos     = input->Fpos;
    ajStrAssignS(&node->Formatstr, input->Formatstr);
    ajStrAssignS(&node->QryFields, input->QryFields);

    return;
}




/* @funcstatic varinQryProcess ************************************************
**
** Converts a variation data query into an open file.
**
** Tests for "format::" and sets this if it is found
**
** Then tests for "list:" or "@" and processes as a list file
** using varinListProcess which in turn invokes varinQryProcess
** until a valid query is found.
**
** Then tests for dbname:query and opens the file (at the correct position
** if the database definition defines it)
**
** If there is no database, looks for file:query and opens the file.
** In this case the file position is not known and variation data reading
** will have to scan for the entry/entries we need.
**
** @param [u] varin [AjPVarin] Variation data input structure.
** @param [u] var [AjPVar] Variation data to be read.
**                         The format will be replaced
**                         if defined in the query string.
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool varinQryProcess(AjPVarin varin, AjPVar var)
{
    AjBool ret = ajTrue;
    AjPStr qrystr = NULL;
    AjBool varmethod = ajFalse;
    const AjPStr fmtstr = NULL;
    AjPTextin textin;
    AjPQuery qry;
    AjPVarAccess varaccess = NULL;
    AjBool ignorerev = ajFalse;

    textin = varin->Input;
    qry = textin->Query;
    
    /* pick up the original query string */
    qrystr = ajStrNewS(textin->Qry);

    ajDebug("varinQryProcess '%S'\n", qrystr);

    /* look for a format:: prefix */
    fmtstr = ajQuerystrParseFormat(&qrystr, textin, varinformatFind);
    ajDebug("varinQryProcess ... fmtstr '%S' '%S'\n", fmtstr, qrystr);

    ajQuerystrParseRange(&qrystr, &varin->Begin, &varin->End, &ignorerev);

    /* look for a list:: or @:: listfile of queries  - process and return */
    if(ajQuerystrParseListfile(&qrystr))
    {
        ajDebug("varinQryProcess ... listfile '%S'\n", qrystr);
        ret = varinListProcess(varin, var, qrystr);
        ajStrDel(&qrystr);
        return ret;
    }

    /* try general text access methods (file, asis, text database access */
    ajDebug("varinQryProcess ... no listfile '%S'\n", qrystr);
    if(!ajQuerystrParseRead(&qrystr, textin, varinformatFind, &varmethod))
    {
        ajStrDel(&qrystr);
        return ajFalse;
    }
    
    varinFormatSet(varin, var);

    ajDebug("varinQryProcess ... read nontext: %B '%S'\n",
            varmethod, qrystr);
    ajStrDel(&qrystr);

    /* we found a non-text method */
    if(varmethod)
    {
        ajDebug("varinQryProcess ... call method '%S'\n", qry->Method);
        ajDebug("varinQryProcess ... textin format %d '%S'\n",
                textin->Format, textin->Formatstr);
        ajDebug("varinQryProcess ...  query format  '%S'\n",
                qry->Formatstr);
        qry->Access = ajCallTableGetS(varDbMethods,qry->Method);
        varaccess = qry->Access;
        return (*varaccess->Access)(varin);
    }

    ajDebug("varinQryProcess text method '%S' success\n", qry->Method);

    return ajTrue;
}





/* @datasection [AjPList] Query field list ************************************
**
** Query fields lists are handled internally. Only static functions
** should appear here
**
******************************************************************************/




/* @funcstatic varinListProcess ***********************************************
**
** Processes a file of queries.
** This function is called by, and calls, varinQryProcess. There is
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
** @param [u] varin [AjPVarin] Variation data input
** @param [u] var [AjPVar] Variation data
** @param [r] listfile [const AjPStr] Name of list file.,
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool varinListProcess(AjPVarin varin, AjPVar var,
                               const AjPStr listfile)
{
    AjPList list  = NULL;
    AjPFile file  = NULL;
    AjPStr token  = NULL;
    AjPStr rest  = NULL;
    AjBool ret       = ajFalse;
    AjPQueryList node = NULL;

    ajuint recnum = 0;
    static ajint depth    = 0;
    static ajint MAXDEPTH = 16;

    depth++;
    ajDebug("++varinListProcess %S depth %d\n",
	    listfile, depth);

    if(depth > MAXDEPTH)
	ajFatal("Query list too deep");

    if(!varin->Input->List)
	varin->Input->List = ajListNew();

    list = ajListNew();

    file = ajFileNewInNameS(listfile);

    if(!file)
    {
	ajErr("Failed to open list file '%S'", listfile);
	depth--;

	return ret;
    }

    while(ajReadlineTrim(file, &varinReadLine))
    {
        ++recnum;
	varinListNoComment(&varinReadLine);

        if(ajStrExtractWord(varinReadLine, &rest, &token))
        {
            if(ajStrGetLen(rest)) 
            {
                ajErr("Bad record %u in list file '%S'\n'%S'",
                      recnum, listfile, varinReadLine);
            }
            else if(ajStrGetLen(token))
            {
                ajDebug("++Add to list: '%S'\n", token);
                AJNEW0(node);
                ajStrAssignS(&node->Qry, token);
                varinQrySave(node, varin);
                ajListPushAppend(list, node);
            }
        }
    }

    ajFileClose(&file);
    ajStrDel(&token);
    ajStrDel(&rest);

    ajDebug("Trace varin->Input->List\n");
    ajQuerylistTrace(varin->Input->List);
    ajDebug("Trace new list\n");
    ajQuerylistTrace(list);
    ajListPushlist(varin->Input->List, &list);

    ajDebug("Trace combined varin->Input->List\n");
    ajQuerylistTrace(varin->Input->List);

    /*
     ** now try the first item on the list
     ** this can descend recursively if it is also a list
     ** which is why we check the depth above
     */

    if(ajListPop(varin->Input->List, (void**) &node))
    {
        ajDebug("++pop first item '%S'\n", node->Qry);
	ajVarinQryS(varin, node->Qry);
	varinQryRestore(varin, node);
	ajStrDel(&node->Qry);
	ajStrDel(&node->Formatstr);
	AJFREE(node);
	ajDebug("descending with query '%S'\n", varin->Input->Qry);
	ret = varinQryProcess(varin, var);
    }

    depth--;
    ajDebug("++varinListProcess depth: %d returns: %B\n", depth, ret);

    return ret;
}




/* @funcstatic varinListNoComment *********************************************
**
** Strips comments from a character string (a line from an ACD file).
** Comments are blank lines or any text following a "#" character.
**
** @param [u] text [AjPStr*] Line of text from input file.
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void varinListNoComment(AjPStr* text)
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




/* @funcstatic varinFormatSet *************************************************
**
** Sets the input format for variation data using the variation data
** input object's defined format
**
** @param [u] varin [AjPVarin] Variation data input.
** @param [u] var [AjPVar] Variation data
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool varinFormatSet(AjPVarin varin, AjPVar var)
{

    if(ajStrGetLen(varin->Input->Formatstr))
    {
	ajDebug("... input format value '%S'\n",
                varin->Input->Formatstr);

	if(varinformatFind(varin->Input->Formatstr,
                             &varin->Input->Format))
	{
	    ajStrAssignS(&var->Formatstr,
                         varin->Input->Formatstr);
	    var->Format = varin->Input->Format;
	    ajDebug("...format OK '%S' = %d\n",
                    varin->Input->Formatstr,
		    varin->Input->Format);
	}
	else
	    ajDebug("...format unknown '%S'\n",
                    varin->Input->Formatstr);

	return ajTrue;
    }
    else
	ajDebug("...input format not set\n");


    return ajFalse;
}




/* @datasection [AjPVarall] Variation Input Stream ****************************
**
** Function is for manipulating variation input stream objects
**
** @nam2rule Varall Variation input stream objects
**
******************************************************************************/




/* @section Variation Input Constructors **************************************
**
** All constructors return a new variation input stream object by pointer. It
** is the responsibility of the user to first destroy any previous
** variation input object. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so
** anyway.
**
** @fdata [AjPVarall]
**
** @nam3rule New Constructor
**
** @valrule * [AjPVarall] Variation input stream object
**
** @fcategory new
**
******************************************************************************/




/* @func ajVarallNew **********************************************************
**
** Creates a new variation input stream object.
**
** @return [AjPVarall] New variation input stream object.
**
** @release 6.4.0
** @@
******************************************************************************/

AjPVarall ajVarallNew(void)
{
    AjPVarall pthis;

    AJNEW0(pthis);

    pthis->Loader   = ajVarloadNew();

    return pthis;
}





/* ==================================================================== */
/* ========================== destructors ============================= */
/* ==================================================================== */




/* @section Variation Input Stream Destructors ********************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the variation input stream object.
**
** @fdata [AjPVarall]
**
** @nam3rule Del Destructor
**
** @argrule Del pthis [AjPVarall*] Variation input stream
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajVarallDel **********************************************************
**
** Deletes a variation input stream object.
**
** @param [d] pthis [AjPVarall*] Variation input stream
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajVarallDel(AjPVarall* pthis)
{
    AjPVarall thys;

    if(!pthis)
        return;

    thys = *pthis;

    if(!thys)
        return;

    ajVarloadDel(&thys->Loader);

    AJFREE(*pthis);

    return;
}




/* ==================================================================== */
/* =========================== Modifiers ============================== */
/* ==================================================================== */




/* @section Variation input stream modifiers **********************************
**
** These functions use the contents of a variation input stream object and
** update them.
**
** @fdata [AjPVarall]
**
** @nam3rule Clear Clear all values
**
** @argrule * thys [AjPVarall] Variation input stream object
**
** @valrule * [void]
**
** @fcategory modify
**
******************************************************************************/




/* @func ajVarallClear ********************************************************
**
** Clears a variation input stream object back to "as new" condition, except
** for the query list which must be preserved.
**
** @param [w] thys [AjPVarall] Variation input stream
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajVarallClear(AjPVarall thys)
{

    ajDebug("ajVarallClear called\n");

    ajVarloadClear(thys->Loader);

    thys->Returned = ajFalse;

    return;
}




/* @section Variation input stream casts **************************************
**
** These functions return the contents of a variation input stream object
**
** @fdata [AjPVarall]
**
** @nam3rule Get Get variation input stream values
** @nam3rule Getvar Get variation values
** @nam4rule GetvarId Get variation identifier
**
** @argrule * thys [const AjPVarall] Variation input stream object
**
** @valrule * [const AjPStr] String value
**
** @fcategory cast
**
******************************************************************************/




/* @func ajVarallGetvarId *****************************************************
**
** Returns the identifier of the current variation in an input stream
**
** @param [r] thys [const AjPVarall] Variation input stream
**
** @return [const AjPStr] Identifier
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPStr ajVarallGetvarId(const AjPVarall thys)
{
    if(!thys)
        return NULL;

    ajDebug("ajVarallGetvarId called\n");

    return ajVarloadGetvarId(thys->Loader);
}




/* @section Variation input ***************************************************
**
** These functions use a variation input stream object to read data
**
** @fdata [AjPVarall]
**
** @nam3rule Next Read next variation
**
** @argrule * thys [AjPVarall] Variation input stream object
** @argrule * Pvarload [AjPVarload*] Variation loader object
**
** @valrule * [AjBool] True on success
**
** @fcategory input
**
******************************************************************************/




/* @func ajVarallNext *********************************************************
**
** Load the next set of initial variation data in an input stream.
**
** Return the results in the AjPVarload object but leave the file open for
** future calls.
**
** @param [w] thys [AjPVarall] Variation input stream
** @param [u] Pvarload [AjPVarload*] Variation loader returned
** @return [AjBool] ajTrue on success.
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajVarallNext(AjPVarall thys, AjPVarload *Pvarload)
{
    ajDebug("ajVarallNext count:%u\n", thys->Count);

    if(!thys->Count)
    {
	thys->Count = 1;

	thys->Totterms++;

	*Pvarload = thys->Loader;
	thys->Returned = ajTrue;

	ajDebug("ajVarallNext initial\n");

	return ajTrue;
    }


    if(ajVarinLoad(thys->Loader->Varin, thys->Loader->Var))
    {
	thys->Count++;

	thys->Totterms++;

	*Pvarload = thys->Loader;
	thys->Returned = ajTrue;

	ajDebug("ajVarallNext success\n");

	return ajTrue;
    }

    *Pvarload = NULL;

    ajDebug("ajVarallNext failed\n");

    ajVarallClear(thys);

    return ajFalse;
}




/* @datasection [AjPVarload] Variation Loader *********************************
**
** Function is for manipulating variation loader objects
**
** @nam2rule Varload Variation loader objects
**
******************************************************************************/




/* @section Variation Loader Constructors *************************************
**
** All constructors return a new variation loader object by pointer. It
** is the responsibility of the user to first destroy any previous
** variation loader object. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so
** anyway.
**
** @fdata [AjPVarload]
**
** @nam3rule New Constructor
**
** @valrule * [AjPVarload] Variation loader object
**
** @fcategory new
**
******************************************************************************/




/* @func ajVarloadNew *********************************************************
**
** Creates a new variation loader object.
**
** @return [AjPVarload] New variation loader object.
**
** @release 6.5.0
** @@
******************************************************************************/

AjPVarload ajVarloadNew(void)
{
    AjPVarload pthis;

    AJNEW0(pthis);

    pthis->Varin = ajVarinNew();
    pthis->Var   = ajVarNew();

    return pthis;
}





/* ==================================================================== */
/* ========================== destructors ============================= */
/* ==================================================================== */




/* @section Variation Loader Destructors **************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the variation loader object.
**
** @fdata [AjPVarload]
**
** @nam3rule Del Destructor
**
** @argrule Del pthis [AjPVarload*] Variation loader
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajVarloadDel *********************************************************
**
** Deletes a variation loader object.
**
** @param [d] pthis [AjPVarload*] Variation loader
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ajVarloadDel(AjPVarload* pthis)
{
    AjPVarload thys;

    if(!pthis)
        return;

    thys = *pthis;

    if(!thys)
        return;

    ajVarinDel(&thys->Varin);
    if(!thys->Returned)
        ajVarDel(&thys->Var);

    AJFREE(*pthis);

    return;
}




/* ==================================================================== */
/* =========================== Modifiers ============================== */
/* ==================================================================== */




/* @section Variation loader modifiers ****************************************
**
** These functions use the contents of a variation loader object and
** update them.
**
** @fdata [AjPVarload]
**
** @nam3rule Clear Clear all values
**
** @argrule * thys [AjPVarload] Variation loader object
**
** @valrule * [void]
**
** @fcategory modify
**
******************************************************************************/




/* @func ajVarloadClear *******************************************************
**
** Clears a variation loader object back to "as new" condition, except
** for the query list which must be preserved.
**
** @param [w] thys [AjPVarload] Variation loader
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ajVarloadClear(AjPVarload thys)
{

    ajDebug("ajVarloadClear called\n");

    ajVarinClear(thys->Varin);

    ajVarClear(thys->Var);

    thys->Loading = ajFalse;

    return;
}




/* @section Variation loader casts ********************************************
**
** These functions return the contents of a variation loader object
**
** @fdata [AjPVarload]
**
** @nam3rule Get Get variation loader values
** @nam3rule Getvar Get variation values
** @nam4rule GetvarId Get variation identifier
**
** @argrule * thys [const AjPVarload] Variation loader object
**
** @valrule * [const AjPStr] String value
**
** @fcategory cast
**
******************************************************************************/




/* @func ajVarloadGetvarId ****************************************************
**
** Returns the identifier of the current variation in a loader
**
** @param [r] thys [const AjPVarload] Variation loader
**
** @return [const AjPStr] Identifier
**
** @release 6.5.0
** @@
******************************************************************************/

const AjPStr ajVarloadGetvarId(const AjPVarload thys)
{
    if(!thys)
        return NULL;

    ajDebug("ajVarloadGetvarId called\n");

    return ajVarGetId(thys->Var);
}




/* @section Variation loading *************************************************
**
** These functions use a variation loader object to read data
**
** @fdata [AjPVarload]
**
** @nam3rule More Test whether more data can be loaded
** @nam3rule Next Read next variation
**
** @argrule * thys [AjPVarload] Variation loader object
** @argrule * Pvar [AjPVar*] Variation object
**
** @valrule * [AjBool] True on success
**
** @fcategory input
**
******************************************************************************/




/* @func ajVarloadMore ********************************************************
**
** Test whether more data can be loaded
**
** Return the results in the AjPVar object but leave the file open for
** future calls.
**
** @param [w] thys [AjPVarload] Variation loader
** @param [u] Pvar [AjPVar*] Variation returned
** @return [AjBool] ajTrue on success.
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ajVarloadMore(AjPVarload thys, AjPVar *Pvar)
{
    AjPVarin  varin = thys->Varin;
    AjPTextin input = varin->Input;

    ajDebug("ajVarloadMore count:%u dataread: %B datadone: %B"
           " loading varload: %B varin: %B\n",
            thys->Count, input->Dataread, input->Datadone,
            thys->Loading, varin->Loading);

    if(input->Datadone)
    {
        input->Dataread = ajFalse;
        input->Datadone = ajFalse;
        ajDebug("ajVarloadMore done\n");
        return ajFalse;
    }
    
    if(!varin->Loading)
    {
        varin->Loading = ajTrue;
        thys->Count++;

	*Pvar = thys->Var;
	thys->Returned = ajTrue;

	ajDebug("ajVarloadMore initial\n");

	return ajTrue;
    }

    if(ajVarinLoad(thys->Varin, thys->Var))
    {
        *Pvar = thys->Var;
	thys->Returned = ajTrue;

	ajDebug("ajVarloadMore success\n");

	return ajTrue;
    }

    *Pvar = NULL;
    thys->Returned = ajFalse;

    ajDebug("ajVarloadMore failed to read\n");

    return ajFalse;
}




/* @func ajVarloadNext ********************************************************
**
** Parse a variation query into format, access, file and entry
**
** Split at delimiters. Check for the first part as a valid format
** Check for the remaining first part as a database name or as a file
** that can be opened.
** Anything left is an entryname spec.
**
** Return the results in the AjPVar object but leave the file open for
** future calls.
**
** @param [w] thys [AjPVarload] Variation loader
** @param [u] Pvar [AjPVar*] Variation returned
** @return [AjBool] ajTrue on success.
**
** @release 6.5.0
** @@
******************************************************************************/

AjBool ajVarloadNext(AjPVarload thys, AjPVar *Pvar)
{
    AjPVarin  varin = thys->Varin;
    AjPTextin input = varin->Input;

    ajDebug("..ajVarloadNext count:%u dataread: %B datadone: %B"
           " loading varload: %B varin: %B\n",
            thys->Count, input->Dataread, input->Datadone,
            thys->Loading, varin->Loading);

    if(!thys->Loading)
    {
        thys->Loading = ajTrue;
        varin->Loading = ajFalse;

	*Pvar = thys->Var;
	thys->Returned = ajTrue;

	ajDebug("..ajVarloadNext initial\n");

	return ajTrue;
    }


    if(ajVarinLoad(thys->Varin, thys->Var))
    {
        varin->Loading = ajFalse;

	*Pvar = thys->Var;
	thys->Returned = ajTrue;

	ajDebug("..ajVarloadNext success\n");

	return ajTrue;
    }

    *Pvar = NULL;
    thys->Returned = ajFalse;

    ajDebug("..ajVarloadNext failed\n");

    return ajFalse;
}




/* @datasection [none] Input formats ******************************************
**
** Input formats internals
**
** @nam2rule Varinformat Variation data input format specific
**
******************************************************************************/




/* @section cast **************************************************************
**
** Values for input formats
**
** @fdata [none]
**
** @nam3rule Find Return index to named format
** @nam3rule Term Test format EDAM term
** @nam3rule Test Test format value
**
** @argrule Find format [const AjPStr] Format name
** @argrule Term term [const AjPStr] Format EDAM term
** @argrule Test format [const AjPStr] Format name
** @argrule Find iformat [ajint*] Index matching format name
**
** @valrule * [AjBool] True if found
**
** @fcategory cast
**
******************************************************************************/




/* @funcstatic varinformatFind ************************************************
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

static AjBool varinformatFind(const AjPStr format, ajint* iformat)
{
    AjPStr tmpformat = NULL;
    ajuint i = 0;

    /* ajDebug("varinformatFind '%S'\n", format); */
    if(!ajStrGetLen(format))
	return ajFalse;

    ajStrAssignS(&tmpformat, format);
    ajStrFmtLower(&tmpformat);

    for(i=0; varinFormatDef[i].Name; i++)
    {
	/* ajDebug("test %d '%s' '%s' '%s'\n",
           i, varinFormatDef[i].Name,
           varinFormatDef[i].Obo,
           varinFormatDef[i].Desc); */
	if(ajStrMatchCaseC(tmpformat, varinFormatDef[i].Name) ||
            ajStrMatchC(format, varinFormatDef[i].Obo))
	{
	    *iformat = i;
	    ajStrDel(&tmpformat);
	    /* ajDebug("found '%s' at %d\n", varinFormatDef[i].Name, i); */
	    return ajTrue;
	}
    }

    ajErr("Unknown input format '%S'", format);

    ajStrDel(&tmpformat);

    return ajFalse;
}




/* @func ajVarinformatTerm ****************************************************
**
** Tests whether a variation data input format term is known
**
** @param [r] term [const AjPStr] Format term EDAM ID
** @return [AjBool] ajTrue if term was accepted
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajVarinformatTerm(const AjPStr term)
{
    ajuint i;

    for(i=0; varinFormatDef[i].Name; i++)
	if(ajStrMatchC(term, varinFormatDef[i].Obo))
	    return ajTrue;

    return ajFalse;
}




/* @func ajVarinformatTest ****************************************************
**
** Tests whether a named variation data input format is known
**
** @param [r] format [const AjPStr] Format
** @return [AjBool] ajTrue if format was accepted
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ajVarinformatTest(const AjPStr format)
{
    ajuint i;

    for(i=0; varinFormatDef[i].Name; i++)
    {
	if(ajStrMatchCaseC(format, varinFormatDef[i].Name))
	    return ajTrue;
	if(ajStrMatchC(format, varinFormatDef[i].Obo))
	    return ajTrue;
    }

    return ajFalse;
}




/* @funcstatic varRegInitVcf **************************************************
**
** Initialise regular expressions and data structures for
** VCF format
**
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void varRegInitVcf(void)
{
    if(VarInitVcf)
	return;

    VarInitVcf  = ajTrue;

    varRegId = ajRegCompC("ID[=]([^,>]+)[,>]");
    varRegType = ajRegCompC("Type[=]([^,>]+)[,>]");
    varRegNumber = ajRegCompC("Number[=]([^,>]+)[,>]");
    varRegDesc = ajRegCompC("Description[=]\"(.*)\"");
    varRegDesc2 = ajRegCompC("Description[=]([^,>]+)[,>]");
    varRegGenomes = ajRegCompC("Genomes[=]([^,>]+)[,>]");
    varRegMixture = ajRegCompC("Mixture[=]([^,>]+)[,>]");

    return;
}

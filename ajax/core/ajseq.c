/* @source ajseq **************************************************************
**
** AJAX SEQ (sequence) functions
**
** These functions control all aspects of AJAX sequence
** reading and writing and include simple utilities.
**
** @author Copyright (C) 1998 Peter Rice
** @version $Revision: 1.183 $
** @modified Jun 25 pmr First version
** @modified $Date: 2013/07/15 20:56:02 $ by $Author: rice $
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

#include "ajseq.h"
#include "ajseqread.h"
#include "ajseqwrite.h"
#include "ajseqtype.h"
#include "ajsys.h"
#include "ajlist.h"
#include "ajtable.h"
#include "ajtime.h"
#include "ajfeat.h"
#include "ajmath.h"
#include "ajsort.h"
#include "ajbase.h"

#include <string.h>
#include <limits.h>
#include <float.h>
#include <math.h>

static AjPStr seqVersionAccnum = NULL;

static AjPStr seqTempUsa = NULL;


static void seqMakeUsa(const AjPSeq thys, AjPStr* usa);
static AjBool seqDateSet(AjPTime* date, const AjPStr datestr);
static int seqxrefSortDb(const void* refa, const void* refb);
static void seqxrefDel(void** Pxref, void *cl);

static void seqclsInit(void);
static void seqdivInit(void);
static void seqmolInit(void);

static AjPStr seqMoleculeDef = NULL;
static AjPTable seqTableMol = NULL;
static AjPTable seqTableMolEmbl = NULL;
static AjPTable seqTableMolDdbj = NULL;
static AjPTable seqTableMolGb = NULL;

static AjPStr seqDivisionDef = NULL;
static AjPTable seqTableDiv = NULL;
static AjPTable seqTableDivEmbl = NULL;
static AjPTable seqTableDivDdbj = NULL;
static AjPTable seqTableDivGb = NULL;

static AjPStr seqClassDef = NULL;
static AjPTable seqTableCls = NULL;
static AjPTable seqTableClsEmbl = NULL;
static AjPTable seqTableClsDdbj = NULL;
static AjPTable seqTableClsGb = NULL;

/*static AjPTable seqTableXref = NULL;*/




/* @datastatic SeqOClass ******************************************************
**
** Classes of entries in the sequence databases
**
** Reference for this is
** http://www.ncbi.nlm.nih.gov/HTGS/table1.html
**
** @attr Name [const char*] Name used internally
** @attr Embl [const char*] Name in EMBL
** @attr Ddbj [const char*] Name in DDBJ
** @attr Genbank [const char*] Name in Genbank
** @attr Desc [const char*] Description
******************************************************************************/

typedef struct SeqSClass
{
  const char* Name;
  const char* Embl;
  const char* Ddbj;
  const char* Genbank;
  const char* Desc;
} SeqOClass;

static SeqOClass seqClass[] =
{
/*   Name      Embl   DDBJ   Genbank Description */
    {"STD", "STD", "",    "",    "Standard (all other entries)"},
    {"ANN", "ANN", "",    "",    "CON entries with own annotation"},
    {"CON", "CON", "CON", "CON", "Constructed from segment sequences"},
    {"EST", "EST", "EST", "EST", "Expressed Sequence Tag"},
    {"GSS", "GSS", "GSS", "GSS", "Genome Survey Sequence"},
    {"HTC", "HTC", "HTC", "",    "High Thoughput CDNA sequencing"},
    {"HTG", "HTG", "HTG", "HTG", "High Thoughput Genome sequencing"},
    {"MGA", "MGA", "",    "",    "Mass Genome Annotation"},
    {"PAT", "PAT", "PAT", "PAT", "Patent"},
    {"STS", "STS", "STS", "STS", "Sequence Tagged Site"},
    {"TPA", "TPA", "TPA", "",    "Third Party Annotation"},  /* 2 DDBJ types */
    {"TSA", "TSA", "TSA", "",    "Transcriptome shotgun assembly"},
    {"WGS", "WGS", "",    "",    "Whole Genome Shotgun"},
    {"", "", "", "", ""},
    {NULL, NULL, NULL, NULL, NULL}
};




/* @datastatic SeqODivision ***************************************************
**
** Taxonomic divisions in the sequence databases
**
** Reference for this is
** http://www.ncbi.nlm.nih.gov/HTGS/table1.html
**
** @attr Name [const char*] Name used internally
** @attr Embl [const char*] Name in EMBL
** @attr Ddbj [const char*] Name in DDBJ
** @attr Genbank [const char*] Name in Genbank
** @attr Desc [const char*] Description
******************************************************************************/

typedef struct SeqSDivision
{
  const char* Name;
  const char* Embl;
  const char* Ddbj;
  const char* Genbank;
  const char* Desc;
} SeqODivision;


static SeqODivision seqDivision[] =
{
/*   Name      Embl   DDBJ   Genbank Description */
    {"default","UNC", "UNA", "UNA",  "Unclassified"},
    {"CON",    "CON", "CON", "CON",  "Contig"},
    {"ENV",    "ENV", "",    "",     "Environmental Sample"},
    {"FUN",    "FUN", "",    "PLN",  "Fungal"},
    {"HUM",    "HUM", "HUM", "PRI",  "Human"},
    {"INV",    "INV", "INV", "INV",  "Invertebrate"},
    {"MAM",    "MAM", "MAM", "MAM",  "Other Mammal"},
    {"MAMPRI", "MAM", "",    "PRI",  "Primate"},
    {"MUS",    "MUS", "ROD", "ROD",  "Mus musculus"},
    {"PHG",    "PHG", "PHG", "PHG",  "Bacteriophage"},
    {"PLN",    "PLN", "PLN", "PLN",  "Plant"},
    {"PRO",    "PRO", "BCT", "BCT",  "Prokaryote"},
    {"ROD",    "ROD", "ROD", "ROD",  "Other Rodent"},
    {"SYN",    "SYN", "SYN", "SYN",  "Synthetic and chimeric"},
    {"TGN",    "TGN", "",    "",     "Transgenic"},
    {"UNC",    "UNC", "",    "",     "Unclassified"},
    {"VRL",    "VRL", "VRL", "VRL",  "Viral"},
    {"VRT",    "VRT", "VRT", "VRT",  "Other Vertebrate"},
    {"UNA",    "UNC", "UNA", "UNA",  "Unannotated"},
    {"UNCRNA", "UNC", "",    "RNA",  "Structural RNA"}, /* obsolete ? */
    {"", "", "", "", ""},
    {NULL, NULL, NULL, NULL, NULL}
};




/* @datastatic SeqOMolecule ***************************************************
**
** Molecule types in the sequence databases
**
** For EMBL and EMBOSS internal, see the
** feature source qualifier /mol_type values in
** http://www.ebi.ac.uk/embl/WebFeat/index.html
**
** @attr Name [const char*] Name used internally
** @attr Embl [const char*] Name in EMBL
** @attr Ddbj [const char*] Name in DDBJ
** @attr Genbank [const char*] Name in Genbank
** @attr Desc [const char*] Description
******************************************************************************/

typedef struct SeqSMolecule
{
  const char* Name;
  const char* Embl;
  const char* Ddbj;
  const char* Genbank;
  const char* Desc;
} SeqOMolecule;


static SeqOMolecule seqMolecule[] =
{
/*   Name      Embl   DDBJ   Genbank Description */
    {"unassigned DNA", "unassigned DNA", "DNA", "DNA", "unassigned DNA"},
    {"genomic DNA", "genomic DNA", "DNA", "DNA", "genomic DNA"},
    {"genomic RNA", "genomic RNA", "DNA", "RNA", "genomic RNA"},
    {"mRNA", "mRNA", "mRNA", "mRNA", "mRNA"},
    {"tRNA", "tRNA", "RNA", "RNA", "tRNA"},
    {"rRNA", "rRNA", "RNA", "RNA", "rRNA"},
    {"snoRNA", "snoRNA", "RNA", "RNA", "snoRNA"},
    {"snRNA", "snRNA", "RNA", "RNA", "snRNA"},
    {"snRNA", "snRNA", "RNA", "RNA", "snRNA"},
    {"scRNA", "scRNA", "RNA", "RNA", "scRNA"},
    {"pre-RNA", "pre-RNA", "RNA", "RNA", "pre-RNA"},
    {"viral cRNA", "viral cRNA", "RNA", "RNA", "viral cRNA"},
    {"other RNA", "other RNA", "RNA", "RNA", "other RNA"},
    {"other DNA", "other DNA", "DNA", "DNA", "other DNA"},
    {"unassigned RNA", "unassigned RNA", "RNA", "RNA", "unassigned RNA"},
    {NULL, NULL, NULL, NULL, NULL}
};




/* @datastatic SeqOTaxon ******************************************************
**
** Known major taxa in the sequence databases
**
** Reference for this is
** http://www.ncbi.nlm.nih.gov/Taxonomy/
**
** @attr Taxid [ajuint] NCBI taxon id
** @attr Standard [AjBool] Standard known taxon
** @attr Name [const char*] Common name
******************************************************************************/

typedef struct SeqSTaxon
{
  ajuint Taxid;
  AjBool Standard;  
  const char* Name;
} SeqOTaxon;

static SeqOTaxon seqTaxon[] =
{
    { 3702, AJTRUE, "Arabidopsis thaliana"},
    { 1432, AJTRUE, "Bacillus subtilis"},
    { 9913, AJTRUE, "Bos taurus"},
    { 6239, AJTRUE, "Caenorhabditis elegans"},
    { 3055, AJTRUE, "Chlamydomonas reinhardtii"},
    { 7955, AJTRUE, "Danio rario"},
    {44689, AJTRUE, "Dictyostelium discoideum"},
    { 7227, AJTRUE, "Drosophila melanogaster"},
    {  562, AJTRUE, "Escherichia coli"},
    {11103, AJTRUE, "Hepatitis C virus"},
    { 9606, AJTRUE, "Homo sapiens"},
    { 2104, AJTRUE, "Mycoplasma pneumoniae"},
    {10090, AJTRUE, "Mus musculus"},
    { 4530, AJTRUE, "Oryza sativa"},
    { 5833, AJTRUE, "Plasmodium falciparum"},
    { 4754, AJTRUE, "Pneumocystis carinii"},
    {10116, AJTRUE, "Rattus norvegicus"},
    { 4932, AJTRUE, "Saccharomyces cerevisiae"},
    { 4896, AJTRUE, "Schizosaccharomyces pombe"},
    {31033, AJTRUE, "Takifugu rubripes"},
    { 8355, AJTRUE, "Xenopus laevis"},
    { 4577, AJTRUE, "Zea mays"},
    {    0, AJFALSE, NULL}
    
};




/* @filesection ajseq ********************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
******************************************************************************/




/* @datasection [AjPSeq] Sequence ********************************************
**
** Function is for manipulating sequence objects
**
** @nam2rule Seq
**
******************************************************************************/




/* @section constructors *********************************************
**
** All constructors return a new sequence by pointer. It is the
** responsibility of the user to first destroy any previous
** sequence. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [AjPSeq]
** @fcategory new
**
** @nam3rule New Constructor
** @nam4rule NewName Constructor with new name
** @nam4rule NewRange Constructor with range and direction
** @nam4rule NewRefseq Constructor from a reference sequence
** @nam4rule NewSeq Constructor with all details in an existing sequence
**
** @suffix Res [size_t] Reserved length
** @suffix S [const AjPSeq] Source sequence
** @suffix C [const char*] Source sequence
**
** @argrule Res size [size_t] Reserved size including the terminating NULL
** @argrule C txt [const char*] Source sequence
** @argrule S str [const AjPStr] Source sequence
** @argrule NewRefseq refseq [const AjPRefseq] Reference sequence to be copied
** @argrule NewSeq seq [const AjPSeq] Source sequence to be copied
** @argrule NameC name [const char*] Sequence name
** @argrule NameS name [const AjPStr] Sequence name
** @argrule Range offset [ajint] Offset at start
** @argrule Range offend [ajint] Offset at end
** @argrule Range rev [AjBool] True if sequence is to be reversed
**
** @valrule * [AjPSeq]
**
******************************************************************************/




/* @func ajSeqNew *************************************************************
**
** Creates and initialises a sequence object.
**
** @return [AjPSeq] New sequence object.
**
** @release 1.0.0
** @@
******************************************************************************/

AjPSeq ajSeqNew(void)
{
    return ajSeqNewRes(0);
}




/* @func ajSeqNewNameC ********************************************************
**
** Creates and initialises a sequence object with a specified existing
** sequence as a char*
**
** @param [r] txt[const char*] Sequence string
** @param [r] name [const char*] Sequence name
** @return [AjPSeq] New sequence object.
**
** @release 4.0.0
** @@
******************************************************************************/

AjPSeq ajSeqNewNameC(const char* txt, const char* name)
{
    AjPSeq pthis;

    pthis = ajSeqNew();

    ajStrAssignC(&pthis->Name, name);
    ajStrAssignC(&pthis->Seq, txt);

    return pthis;
}




/* @func ajSeqNewNameS ********************************************************
**
** Creates and initialises a sequence object with a specified existing
** sequence as a string
**
** @param [r] str [const AjPStr] Sequence string
** @param [r] name [const AjPStr] Sequence name
** @return [AjPSeq] New sequence object.
**
** @release 4.0.0
** @@
******************************************************************************/

AjPSeq ajSeqNewNameS(const AjPStr str, const AjPStr name)
{
    AjPSeq pthis;

    pthis = ajSeqNew();

    pthis->Name = ajStrNewS(name);
    ajStrAssignS(&pthis->Seq, str);

    return pthis;
}




/* @func ajSeqNewRangeC *******************************************************
**
** Creates and initialises a sequence object with a specified existing
** sequence as a string, and provides offsets and direction.
**
** The sequence is set to be already trimmed and if necessary reversed.
**
** Start and end positions are 0 (full sequence), as it is trimmed.
** Any start and end are represented by the offsets.
**
** @param [r] txt [const char*] Sequence string
** @param [r] offset [ajint] Offset at start
** @param [r] offend [ajint] Offset at end
** @param [r] rev [AjBool] Reversed if true (reverses offsets)
** @return [AjPSeq] New sequence object.
**
** @release 4.0.0
** @@
******************************************************************************/

AjPSeq ajSeqNewRangeC(const char* txt,
		       ajint offset, ajint offend, AjBool rev)
{
    AjPSeq pthis;

    pthis = ajSeqNew();

    ajDebug("ajSeqNewRangeC %d %d %B\n",
	    offset, offend, rev);

    ajStrAssignC(&pthis->Seq, txt);

    pthis->Rev      = ajFalse;
    pthis->Reversed = rev;		/* we are setting everything here */
    pthis->Trimmed  = ajTrue;		/* we are setting everything here */

    pthis->EType  = 0;
    pthis->Format = 0;
    pthis->Begin  = 0;
    pthis->End    = 0;

    if (rev)
    {
	pthis->Offset = offend;
	pthis->Offend = offset;
    }
    else
    {
	pthis->Offset = offset;
	pthis->Offend = offend;
    }

    pthis->Weight = 1.0;

    ajDebug("ajSeqNewRangeC rev:%B offset:%d/%d offend:%d/%d\n",
	    rev, offset, pthis->Offset, offend, pthis->Offend);
    
    return pthis;
}




/* @func ajSeqNewRangeS *******************************************************
**
** Creates and initialises a sequence object with a specified existing
** sequence as a string, and provides offsets and direction.
**
** The sequence is set to be already trimmed and if necessary reversed.
**
** Start and end positions are 0 (full sequence), as it is trimmed.
** Any start and end are represented by the offsets.
**
** @param [r] str [const AjPStr] Sequence string
** @param [r] offset [ajint] Offset at start
** @param [r] offend [ajint] Offset at end
** @param [r] rev [AjBool] Reversed if true (reverses offsets)
** @return [AjPSeq] New sequence object.
**
** @release 4.0.0
** @@
******************************************************************************/

AjPSeq ajSeqNewRangeS(const AjPStr str,
			ajint offset, ajint offend, AjBool rev)
{
    return ajSeqNewRangeC(ajStrGetPtr(str), offset, offend, rev);
}




/* @func ajSeqNewRefseq *******************************************************
**
** Creates and initialises a sequence object with a specified existing
** reference sequence.
**
** @param [r] refseq [const AjPRefseq] Reference sequence object
** @return [AjPSeq] New sequence object.
**
** @release 4.0.0
** @@
******************************************************************************/

AjPSeq ajSeqNewRefseq(const AjPRefseq refseq)
{
    AjPSeq pthis;

    AJNEW0(pthis);

    if(refseq->Id)
        ajStrAssignS(&pthis->Name, refseq->Id);

    if(refseq->Db)
        ajStrAssignS(&pthis->Db, refseq->Db);
    if(refseq->Desc)
        ajStrAssignS(&pthis->Desc, refseq->Desc);

    if(refseq->Qry)
        ajStrAssignS(&pthis->Usa, refseq->Qry);

    if(refseq->Filename)
        ajStrAssignS(&pthis->Filename, refseq->Filename);

    ajStrAssignS(&pthis->Seq, refseq->Seq);

    return pthis;
}




/* @func ajSeqNewRes **********************************************************
**
** Creates and initialises a sequence object with a specified sequence length.
**
** @param [r] size [size_t] Reserved space for the sequence, including
**                          a trailing null character.
** @return [AjPSeq] New sequence object.
**
** @release 4.0.0
** @@
******************************************************************************/

AjPSeq ajSeqNewRes(size_t size)
{
    AjPSeq pthis;

    AJNEW0(pthis);

    pthis->Name = ajStrNew();
    pthis->Acc  = ajStrNew();
/*
//    pthis->Sv   = ajStrNew();
//    pthis->Gi   = ajStrNew();
//    pthis->Tax  = ajStrNew();
//    pthis->Taxcommon  = ajStrNew();
//    pthis->Taxid = ajStrNew();
//    pthis->Organelle = ajStrNew();
//    pthis->Type = ajStrNew();
//    pthis->Db   = ajStrNew();
//    pthis->Full = ajStrNew();
//    pthis->Desc = ajStrNew();
//    pthis->Doc  = ajStrNew();
//    pthis->Usa  = ajStrNew();
//    pthis->Ufo  = ajStrNew();
//
//    pthis->Formatstr = ajStrNew();
//    pthis->Filename  = ajStrNew();
//    pthis->Entryname = ajStrNew();
//    pthis->TextPtr   = ajStrNew();
*/
    if(size)
	pthis->Seq = ajStrNewRes(size);
    else
	pthis->Seq = ajStrNew();

    pthis->Rev      = ajFalse;
    pthis->Reversed = ajFalse;
    pthis->Trimmed = ajFalse;

    pthis->EType   = 0;
    pthis->Format  = 0;
    pthis->Begin   = 0;
    pthis->End     = 0;
    pthis->Offset  = 0;
    pthis->Offend  = 0;
    pthis->Weight  = 1.0;
/*
//    pthis->Acclist = ajListstrNew();
//    pthis->Keylist = ajListstrNew();
//    pthis->Taxlist = ajListstrNew();
//    pthis->Genelist = ajListNew();
//    pthis->Cmtlist = ajListstrNew();
//    pthis->Xreflist = ajListNew();
//    pthis->Reflist = ajListNew();
//
//    pthis->Fulldesc = ajSeqdescNew();
*/
     return pthis;
}




/* @func ajSeqNewSeq **********************************************************
**
** Creates and initialises a sequence object with a specified existing
** sequence.
**
** @param [r] seq [const AjPSeq] Old sequence object
** @return [AjPSeq] New sequence object.
**
** @release 4.0.0
** @@
******************************************************************************/

AjPSeq ajSeqNewSeq(const AjPSeq seq)
{
    AjPSeq pthis;

    AJNEW0(pthis);

    if(seq->Name)
        ajStrAssignS(&pthis->Name, seq->Name);
    if(seq->Acc)
        ajStrAssignS(&pthis->Acc, seq->Acc);
    if(seq->Sv)
        ajStrAssignS(&pthis->Sv, seq->Sv);
    if(seq->Gi)
        ajStrAssignS(&pthis->Gi, seq->Gi);
    if(seq->Tax)
        ajStrAssignS(&pthis->Tax, seq->Tax);
    if(seq->Taxcommon)
        ajStrAssignS(&pthis->Taxcommon, seq->Taxcommon);
    if(seq->Taxid)
        ajStrAssignS(&pthis->Taxid, seq->Taxid);
    if(seq->Organelle)
        ajStrAssignS(&pthis->Organelle, seq->Organelle);
    if(seq->Type)
        ajStrAssignS(&pthis->Type, seq->Type);

    pthis->EType  = seq->EType;

    if(seq->Db)
        ajStrAssignS(&pthis->Db, seq->Db);
    if(seq->Setdb)
        ajStrAssignS(&pthis->Setdb, seq->Setdb);
    if(seq->Full)
        ajStrAssignS(&pthis->Full, seq->Full);
    if(seq->Desc)
        ajStrAssignS(&pthis->Desc, seq->Desc);
    if(seq->Doc)
        ajStrAssignS(&pthis->Doc, seq->Doc);

    pthis->Rev      = seq->Rev;
    pthis->Reversed = seq->Reversed;
    pthis->Trimmed  = seq->Trimmed;

    pthis->Begin  = seq->Begin;
    pthis->End    = seq->End;
    pthis->Offset = seq->Offset;
    pthis->Offend = seq->Offend;
    pthis->Weight = seq->Weight;
    pthis->Fpos   = seq->Fpos;

    if(seq->Usa)
        ajStrAssignS(&pthis->Usa, seq->Usa);
    if(seq->Ufo)
        ajStrAssignS(&pthis->Ufo, seq->Ufo);
    if(seq->Formatstr)
        ajStrAssignS(&pthis->Formatstr, seq->Formatstr);
    pthis->Format = seq->Format;

    if(seq->Filename)
        ajStrAssignS(&pthis->Filename, seq->Filename);
    if(seq->Entryname)
        ajStrAssignS(&pthis->Entryname, seq->Entryname);

    if(seq->TextPtr)
	ajStrAssignS(&pthis->TextPtr, seq->TextPtr);

    if(seq->Acclist)
        pthis->Acclist = ajListstrNewList(seq->Acclist);
    if(seq->Keylist)
        pthis->Keylist = ajListstrNewList(seq->Keylist);
    if(seq->Taxlist)
        pthis->Taxlist = ajListstrNewList(seq->Taxlist);
    if(seq->Cmtlist)
        pthis->Cmtlist = ajListstrNewList(seq->Cmtlist);
    if(seq->Hostlist)
        pthis->Hostlist = ajListstrNewList(seq->Hostlist);

    if(seq->Xreflist)
    {
        pthis->Xreflist = ajListNew();
        ajSeqxreflistClone(seq->Xreflist, pthis->Xreflist);
    }

    if(seq->Genelist)
    {
        pthis->Genelist = ajListNew();
        ajSeqgenelistClone(seq->Genelist, pthis->Genelist);
    }

    if(seq->Reflist)
    {
        pthis->Reflist = ajListNew();
        ajSeqreflistClone(seq->Reflist, pthis->Reflist);
    }

    if(seq->Fulldesc)
        pthis->Fulldesc = ajSeqdescNewDesc(seq->Fulldesc);

    ajStrAssignS(&pthis->Seq, seq->Seq);

    if (seq->Fttable)
	pthis->Fttable = ajFeattableNewFtable(seq->Fttable);

    if (seq->Accuracy && seq->Qualsize)
    {
        pthis->Qualsize = seq->Qualsize;
    	AJCNEW0(pthis->Accuracy,pthis->Qualsize);
    	memmove(pthis->Accuracy,seq->Accuracy,pthis->Qualsize*sizeof(float));
    }

    return pthis;
}




/* @section destructors **********************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the sequence.
**
** @fdata [AjPSeq]
** @fcategory delete
**
** @nam3rule Del Destroy (free) a sequence object
** @nam3rule Delarray Array destructor
**
** @argrule Del Pseq [AjPSeq*] Sequence object address
** @argrule Delarray PPseq [AjPSeq**] Sequence object array
**
** @valrule * [void]
**
******************************************************************************/




/* @func ajSeqDel *************************************************************
**
** Deletes a sequence object.
**
** @param [d] Pseq [AjPSeq*] Sequence object
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajSeqDel(AjPSeq* Pseq)
{
    AjPSeq seq;
    AjPSeqRef  tmpref  = NULL;
    AjPSeqXref tmpxref = NULL;
    AjPSeqGene tmpgene = NULL;

    seq = Pseq ? *Pseq : 0;

    if(!Pseq)
	return;

    if(!*Pseq)
	return;

    ajStrDel(&seq->Name);
    ajStrDel(&seq->Acc);
    ajStrDel(&seq->Sv);
    ajStrDel(&seq->Gi);
    ajStrDel(&seq->Tax);
    ajStrDel(&seq->Taxcommon);
    ajStrDel(&seq->Taxid);
    ajStrDel(&seq->Organelle);
    ajStrDel(&seq->Type);
    ajStrDel(&seq->Molecule);
    ajStrDel(&seq->Class);
    ajStrDel(&seq->Division);
    ajStrDel(&seq->Evidence);
    ajStrDel(&seq->Db);
    ajStrDel(&seq->Setdb);
    ajStrDel(&seq->Full);
    ajStrDel(&seq->Desc);
    ajStrDel(&seq->Doc);
    ajStrDel(&seq->Usa);
    ajStrDel(&seq->Ufo);
    ajStrDel(&seq->Formatstr);
    ajStrDel(&seq->Filename);
    ajStrDel(&seq->Entryname);
    ajStrDel(&seq->TextPtr);
    ajStrDel(&seq->Seq);
    AJFREE(seq->Accuracy);

    if(seq->Fttable)
	ajFeattableDel(&seq->Fttable);

    ajListstrFreeData(&seq->Acclist);

    ajListstrFreeData(&seq->Keylist);

    ajListstrFreeData(&seq->Taxlist);

    while(ajListPop(seq->Genelist,(void **)&tmpgene))
	ajSeqgeneDel(&tmpgene);

    ajListFree(&seq->Genelist);

    while(ajListPop(seq->Reflist,(void **)&tmpref))
	ajSeqrefDel(&tmpref);

    ajListFree(&seq->Reflist);

    ajListstrFreeData(&seq->Cmtlist);
    ajListstrFreeData(&seq->Hostlist);

    while(ajListPop(seq->Xreflist,(void **)&tmpxref))
	ajSeqxrefDel(&tmpxref);

    ajListFree(&seq->Xreflist);

    ajSeqdateDel(&seq->Date);
    ajSeqdescDel(&seq->Fulldesc);

    AJFREE(*Pseq);

    return;
}




/* @func ajSeqDelarray ********************************************************
**
** Destructor for array of sequence objects
**
** @param [d] PPseq [AjPSeq**] Sequence object array
** @return [void]
**
** @release 6.0.0
** @@
******************************************************************************/

void ajSeqDelarray(AjPSeq **PPseq)
{
    ajuint i = 0;

    if(!PPseq || !*PPseq)
	return;

    while((*PPseq)[i])
    {
	ajSeqDel(&(*PPseq)[i]);
	i++;
    }

    ajDebug("ajSeqDelarray size: %d\n", i);

    AJFREE(*PPseq);

    return;
}




/* @section modifiers ************************************************
**
** These functions update contents of a sequence object.
**
** @fdata [AjPSeq]
** @fcategory modify
**
** @nam3rule Assign Assign one attribute of a sequence
** @nam4rule AssignAcc Assign accession number
** @nam4rule AssignDesc Assign description text
** @nam4rule AssignEntry Assign entry name
** @nam4rule AssignFile Assign file name
** @nam4rule AssignFull Assign full name
** @nam4rule AssignGi Assign GI number
** @nam4rule AssignName Assign sequence name
** @nam4rule AssignQual Assign quality scores
** @nam4rule AssignSeq Assign sequence
** @nam4rule AssignSv Assign sequence version number
** @nam4rule AssignUfo Assign feature address
** @nam4rule AssignUsa Assign sequence address
**
** @nam3rule Clear     Clear all contents
**
** @nam3rule Add     Add to a list of data
** @nam4rule AddCmt Add to a gene list
** @nam4rule AddGene Add to a gene list
** @nam4rule AddHost Add to a virus host list
** @nam4rule AddKey Add to a keyword list
** @nam4rule AddRef  Add to a reference list
** @nam4rule AddXref  Add to a cross-reference list
**
** @nam3rule Set Set sequence properties
** @nam4rule SetCircular Set sequence as circular
** @nam4rule SetName Set sequence name
** @nam5rule SetNameMulti Set sequence name, adding a number for later calls
** @nam4rule SetOffsets Set sequence offsets as a subsequence of an original
** @nam4rule SetRange Set start and end position within sequence
** @nam5rule SetRangeRev Set start and end position and reverse direction
**                       of a sequence
** @nam4rule SetUnique Make sure sequence is modifiable (no other pointer
**                     uses the same internal string)
** @nam3rule Tag       Add tag to the sequence description
** @nam4rule TagRev    Add 'Reversed' tag to the sequence description
**
** @suffix Len [ajint] Length of character string
** @suffix C [const char*] Character string
** @suffix S [const AjPStr] String
** 
** @argrule * seq [AjPSeq] Sequence object
** @argrule Qual qual [const float*] Array of quality scores
** @argrule C txt [const char*] Character string to assign
** @argrule Len len [ajuint] Character string length
** @argrule S str [const AjPStr] String to assign
** @argrule Offsets offset [ajint] Offset at start
** @argrule Offsets origlen [ajint] Length of original sequence
** @argrule Range pos1 [ajint] Start position
** @argrule Range pos2 [ajint] End  position
** @argrule SetName setname [const AjPStr] User-defined sequence name
** @argrule Cmt str [AjPStr] Comment added to internal list
** @argrule Gene gene [AjPSeqGene] Gene object added to internal list
** @argrule Host str [AjPStr] Viral host added to internal list
** @argrule Key str [AjPStr] Keyword added to internal list
** @argrule Ref ref [AjPSeqRef] Citation object added to internal list
** @argrule Xref xref [AjPSeqXref] Cross-reference object added to internal list
**
** @valrule * [void]
**
******************************************************************************/




/* @func ajSeqAddCmt **********************************************************
**
** Adds a comment to a sequence object
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [u] str [AjPStr] Comment.
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ajSeqAddCmt(AjPSeq seq,  AjPStr str)
{
    if(!seq->Cmtlist)
        seq->Cmtlist = ajListstrNew();

    ajListstrPushAppend(seq->Cmtlist, str);

    return;
}




/* @func ajSeqAddGene *********************************************************
**
** Adds a gene to a sequence object
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [u] gene [AjPSeqGene] Gene object
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ajSeqAddGene(AjPSeq seq, AjPSeqGene gene)
{
    if(!seq->Genelist)
        seq->Genelist = ajListNew();

    ajListPushAppend(seq->Genelist, gene);

    return;
}




/* @func ajSeqAddHost *********************************************************
**
** Adds a host organism to a sequence object
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [u] str [AjPStr] Host organism swissprot formatted string
** @return [void]
**
** @release 6.6.0
** @@
******************************************************************************/

void ajSeqAddHost(AjPSeq seq,  AjPStr str)
{
    if(!seq->Hostlist)
        seq->Hostlist = ajListstrNew();

    ajListstrPushAppend(seq->Hostlist, str);

    return;
}




/* @func ajSeqAddKey **********************************************************
**
** Adds a keyword to a sequence object
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [u] str [AjPStr] Comment.
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ajSeqAddKey(AjPSeq seq, AjPStr str)
{
    if(!seq->Keylist)
        seq->Keylist = ajListstrNew();

    ajListstrPushAppend(seq->Keylist, str);

    return;
}




/* @func ajSeqAddRef **********************************************************
**
** Adds a literature reference to a sequence object
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [u] ref [AjPSeqRef] Literature reference.
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ajSeqAddRef(AjPSeq seq, AjPSeqRef ref)
{
    if(!seq->Reflist)
        seq->Reflist = ajListNew();

    ajListPushAppend(seq->Reflist, ref);

    return;
}




/* @func ajSeqAddXref *********************************************************
**
** Adds a cross-reference to a sequence object
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [u] xref [AjPSeqXref] Comment.
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ajSeqAddXref(AjPSeq seq, AjPSeqXref xref)
{
    if(!seq->Xreflist)
        seq->Xreflist = ajListNew();

    ajListPushAppend(seq->Xreflist, xref);

    return;
}




/* @func ajSeqAssignAccC ******************************************************
**
** Assigns the sequence accession number.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] txt [const char*] Accession number as a C character string.
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajSeqAssignAccC(AjPSeq seq, const char* txt)
{
    ajStrAssignC(&seq->Acc, txt);

    return;
}




/* @func ajSeqAssignAccS ******************************************************
**
** Assigns the sequence accession number.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] str [const AjPStr] Accession number as a string.
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajSeqAssignAccS(AjPSeq seq, const AjPStr str)
{
    ajStrAssignS(&seq->Acc, str);

    return;
}




/* @func ajSeqAssignDescC *****************************************************
**
** Assigns a modified description to an existing AjPSeq sequence.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] txt [const char*] New description as a C character string.
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajSeqAssignDescC(AjPSeq seq, const char* txt)
{
    ajStrAssignC(&seq->Desc, txt);

    return;
}




/* @func ajSeqAssignDescS *****************************************************
**
** Assigns a modified description to an existing AjPSeq sequence.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] str [const AjPStr] New description as a string.
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajSeqAssignDescS(AjPSeq seq, const AjPStr str)
{
    ajStrAssignS(&seq->Desc, str);

    return;
}




/* @func ajSeqAssignEntryC ****************************************************
**
** Assigns the sequence entryname.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] txt [const char*] Entry name as a C character string.
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajSeqAssignEntryC(AjPSeq seq, const char* txt)
{
    ajStrAssignC(&seq->Entryname, txt);

    return;
}




/* @func ajSeqAssignEntryS ****************************************************
**
** Assigns the sequence entryname.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] str [const AjPStr] Entry name as a string.
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajSeqAssignEntryS(AjPSeq seq, const AjPStr str)
{
    ajStrAssignS(&seq->Entryname, str);

    return;
}




/* @func ajSeqAssignFileC *****************************************************
**
** Assigns the sequence filename.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] txt [const char*] File name as a C character string.
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajSeqAssignFileC(AjPSeq seq, const char* txt)
{
    ajStrAssignC(&seq->Filename, txt);

    return;
}




/* @func ajSeqAssignFileS *****************************************************
**
** Assigns the sequence file name.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] str [const AjPStr] File name as a string.
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajSeqAssignFileS(AjPSeq seq, const AjPStr str)
{
    ajStrAssignS(&seq->Filename, str);

    return;
}




/* @func ajSeqAssignFullC *****************************************************
**
** Assigns the sequence name.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] txt [const char*] Full name as a C character string.
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajSeqAssignFullC(AjPSeq seq, const char* txt)
{
    ajStrAssignC(&seq->Full, txt);

    return;
}




/* @func ajSeqAssignFullS *****************************************************
**
** Assigns the sequence full name.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] str [const AjPStr] Full name as a string.
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajSeqAssignFullS(AjPSeq seq, const AjPStr str)
{
    ajStrAssignS(&seq->Full, str);

    return;
}





/* @func ajSeqAssignGiC *******************************************************
**
** Assigns the GI version number.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] txt [const char*] GI number as a C character string.
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajSeqAssignGiC(AjPSeq seq, const char* txt)
{
    ajStrAssignC(&seq->Gi, txt);

    return;
}




/* @func ajSeqAssignGiS *******************************************************
**
** Assigns the GI version number.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] str [const AjPStr] GI number as a string.
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajSeqAssignGiS(AjPSeq seq, const AjPStr str)
{
    ajStrAssignS(&seq->Gi, str);

    return;
}




/* @func ajSeqAssignNameC *****************************************************
**
** Assigns the sequence name.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] txt [const char*] Name as a string.
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajSeqAssignNameC(AjPSeq seq, const char* txt)
{
    ajStrAssignC(&seq->Name, txt);

    return;
}




/* @func ajSeqAssignNameS *****************************************************
**
** Assigns the sequence name.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] str [const AjPStr] Name as a string.
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajSeqAssignNameS(AjPSeq seq, const AjPStr str)
{
    ajStrAssignS(&seq->Name, str);

    return;
}




/* @func ajSeqAssignQualLen ***************************************************
**
** Assigns base quality scores to an existing sequence.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] qual [const float*] Base quality scores.
** @param [r] len [ajuint] Number of quality scores to use
** @return [void]
**
** @release 6.3.0
** @@
******************************************************************************/

void ajSeqAssignQualLen(AjPSeq seq, const float* qual, ajuint len)
{
    seq->Qualsize = len;

    AJCRESIZE(seq->Accuracy,seq->Qualsize);

    memmove(seq->Accuracy, qual, seq->Qualsize*sizeof(float));

    return;
}




/* @func ajSeqAssignSeqC ******************************************************
**
** Assigns a modified sequence to an existing AjPSeq sequence.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] txt [const char*] New sequence as a C character string.
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajSeqAssignSeqC(AjPSeq seq, const char* txt)
{
    ajint i = strlen(txt);
    ajSeqAssignSeqLenC(seq, txt, i);

    return;
}




/* @func ajSeqAssignSeqLenC ***************************************************
**
** Assigns a modified sequence to an existing AjPSeq sequence.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] txt [const char*] New sequence as a C character string.
** @param [r] len [ajuint] Number of characters to use
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajSeqAssignSeqLenC(AjPSeq seq, const char* txt, ajuint len)
{
    ajStrAssignLenC(&seq->Seq, txt, len);

    seq->Begin  = 0;
    seq->End    = 0;
    seq->Offset = 0;
    seq->Offend = 0;
    seq->Rev      = ajFalse;
    seq->Reversed = ajFalse;
    seq->Trimmed  = ajFalse;

    return;
}




/* @func ajSeqAssignSeqS ******************************************************
**
** Assigns a modified sequence to an existing AjPSeq sequence.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] str [const AjPStr] New sequence as a string.
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajSeqAssignSeqS(AjPSeq seq, const AjPStr str)
{
    ajSeqAssignSeqLenC(seq, ajStrGetPtr(str), ajStrGetLen(str));

    return;
}




/* @func ajSeqAssignSvC *******************************************************
**
** Assigns the sequence version number.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] txt [const char*] SeqVersion number as a C character string.
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajSeqAssignSvC(AjPSeq seq, const char* txt)
{
    ajStrAssignC(&seq->Sv, txt);

    return;
}




/* @func ajSeqAssignSvS *******************************************************
**
** Assigns the sequence version number.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] str [const AjPStr] SeqVersion number as a string.
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajSeqAssignSvS(AjPSeq seq, const AjPStr str)
{
    ajStrAssignS(&seq->Sv, str);

    return;
}




/* @func ajSeqAssignUfoC ******************************************************
**
** Assigns the sequence feature file name.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] txt [const char*] UFO as a C character string.
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajSeqAssignUfoC(AjPSeq seq, const char* txt)
{
    ajStrAssignC(&seq->Ufo, txt);

    return;
}




/* @func ajSeqAssignUfoS ******************************************************
**
** Assigns the sequence feature file name.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] str [const AjPStr] UFO as a string.
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajSeqAssignUfoS(AjPSeq seq, const AjPStr str)
{
    ajStrAssignS(&seq->Ufo, str);

    return;
}




/* @func ajSeqAssignUsaC ******************************************************
**
** Assigns the sequence full name.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] txt [const char*] USA as a C character string.
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajSeqAssignUsaC(AjPSeq seq, const char* txt)
{
    ajStrAssignC(&seq->Usa, txt);

    return;
}




/* @func ajSeqAssignUsaS ******************************************************
**
** Assigns the sequence full name.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] str [const AjPStr] USA as a string.
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajSeqAssignUsaS(AjPSeq seq, const AjPStr str)
{
    ajStrAssignS(&seq->Usa, str);

    return;
}




/* @func ajSeqClear ***********************************************************
**
** Resets all data for a sequence object so that it can be reused.
**
** @param [u] seq [AjPSeq] Sequence
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajSeqClear(AjPSeq seq)
{
    AjPStr ptr = NULL;
    AjPSeqRef  tmpref  = NULL;
    AjPSeqXref tmpxref = NULL;
    AjPSeqGene tmpgene = NULL;

    if(!seq)
        return;

    if(MAJSTRGETLEN(seq->Name))
       ajStrSetClear(&seq->Name);
    if(MAJSTRGETLEN(seq->Acc))
       ajStrSetClear(&seq->Acc);
    if(MAJSTRGETLEN(seq->Sv))
       ajStrSetClear(&seq->Sv);
    if(MAJSTRGETLEN(seq->Gi))
       ajStrSetClear(&seq->Gi);
    if(MAJSTRGETLEN(seq->Tax))
       ajStrSetClear(&seq->Tax);
    if(MAJSTRGETLEN(seq->Taxcommon))
       ajStrSetClear(&seq->Taxcommon);
    if(MAJSTRGETLEN(seq->Taxid))
       ajStrSetClear(&seq->Taxid);
    if(MAJSTRGETLEN(seq->Organelle))
       ajStrSetClear(&seq->Organelle);
    if(MAJSTRGETLEN(seq->Type))
       ajStrSetClear(&seq->Type);
    if(MAJSTRGETLEN(seq->Molecule))
       ajStrSetClear(&seq->Molecule);
    if(MAJSTRGETLEN(seq->Class))
       ajStrSetClear(&seq->Class);
    if(MAJSTRGETLEN(seq->Division))
       ajStrSetClear(&seq->Division);
    if(MAJSTRGETLEN(seq->Evidence))
       ajStrSetClear(&seq->Evidence);
    if(MAJSTRGETLEN(seq->Db))
       ajStrSetClear(&seq->Db);
    if(MAJSTRGETLEN(seq->Full))
       ajStrSetClear(&seq->Full);
    if(MAJSTRGETLEN(seq->Desc))
       ajStrSetClear(&seq->Desc);
    if(MAJSTRGETLEN(seq->Doc))
       ajStrSetClear(&seq->Doc);
    if(MAJSTRGETLEN(seq->Usa))
       ajStrSetClear(&seq->Usa);
    if(MAJSTRGETLEN(seq->Ufo))
       ajStrSetClear(&seq->Ufo);

    if(MAJSTRGETLEN(seq->Formatstr))
       ajStrSetClear(&seq->Formatstr);
    if(MAJSTRGETLEN(seq->Filename))
       ajStrSetClear(&seq->Filename);
    if(MAJSTRGETLEN(seq->Entryname))
       ajStrSetClear(&seq->Entryname);
    if(MAJSTRGETLEN(seq->TextPtr))
       ajStrSetClear(&seq->TextPtr);
    if(MAJSTRGETLEN(seq->Seq))
       ajStrSetClear(&seq->Seq);

    if(seq->Accuracy && seq->Qualsize)
        AJCSET0(seq->Accuracy, seq->Qualsize);

    seq->Begin = 0;
    seq->End   = 0;
    seq->Rev      = ajFalse;
    seq->Reversed = ajFalse;
    seq->Trimmed  = ajFalse;
    seq->Circular = ajFalse;

    if(seq->Acclist)   
        while(ajListstrPop(seq->Acclist,&ptr))
            ajStrDel(&ptr);

    if(seq->Keylist)
        while(ajListstrPop(seq->Keylist,&ptr))
            ajStrDel(&ptr);

    if(seq->Taxlist)
        while(ajListstrPop(seq->Taxlist,&ptr))
            ajStrDel(&ptr);

    if(seq->Genelist)
        while(ajListPop(seq->Genelist,(void **)&tmpgene))
            ajSeqgeneDel(&tmpgene);

    if(seq->Reflist)
        while(ajListPop(seq->Reflist,(void **)&tmpref))
            ajSeqrefDel(&tmpref);

    if(seq->Cmtlist)
        while(ajListPop(seq->Cmtlist,(void **)&ptr))
            ajStrDel(&ptr);

    if(seq->Hostlist)
        while(ajListPop(seq->Hostlist,(void **)&ptr))
            ajStrDel(&ptr);

    if(seq->Xreflist)
        while(ajListPop(seq->Xreflist,(void **)&tmpxref))
            ajSeqxrefDel(&tmpxref);

    if(seq->Date)
        ajSeqdateClear(seq->Date);
    if(seq->Fulldesc)
        ajSeqdescClear(seq->Fulldesc);

    if(seq->Fttable)
        ajFeattableDel(&seq->Fttable);

    return;
}




/* @func ajSeqSetCircular *****************************************************
**
** Sets a sequence to be circular
**
** @param [u] seq [AjPSeq] Sequence
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ajSeqSetCircular(AjPSeq seq)
{
    seq->Circular = ajTrue;

    return;
}




/* @func ajSeqSetName *********************************************************
**
** Provides a unique (for this program run) name for a sequence.
**
** @param [w] seq [AjPSeq] Sequence object
** @param [r] setname [const AjPStr] Name set by caller
** @return [void]
**
** @release 4.1.0
** @@
******************************************************************************/

void ajSeqSetName(AjPSeq seq, const AjPStr setname)
{
    if(ajStrGetLen(seq->Name))
    {
	ajDebug("ajSeqSetName already has a name '%S'\n", seq->Name);

	return;
    }

    if (ajStrGetLen(setname))
    {
	ajStrAssignS(&seq->Name, setname);
    }
    else
    {
	ajStrAssignC(&seq->Name, "EMBOSS");
    }

    ajDebug("ajSeqSetName set to  '%S'\n", seq->Name);

    return;
}




/* @func ajSeqSetNameMulti ****************************************************
**
** Provides a unique (for this program run) name for a sequence.
** If a name is generated, append a count
**
** @param [w] seq [AjPSeq] Sequence object
** @param [r] setname [const AjPStr] Name set by caller
** @return [void]
**
** @release 4.1.0
** @@
******************************************************************************/

void ajSeqSetNameMulti(AjPSeq seq, const AjPStr setname)
{
    static ajint count = 0;

    if(ajStrGetLen(seq->Name))
    {
	ajDebug("ajSeqSetNameMulti already has a name '%S'\n", seq->Name);

	return;
    }

    if (ajStrGetLen(setname))
    {
	if(count)
	    ajFmtPrintS(&seq->Name, "%S_%3.3d", setname, ++count);
	else
	{
	    ajStrAssignS(&seq->Name, setname);
	    ++count;
	}
    }
    else
    {
	ajFmtPrintS(&seq->Name, "EMBOSS_%3.3d", ++count);
    }

    ajDebug("ajSeqSetNameMulti set to  '%S'\n", seq->Name);

    return;
}




/* @func ajSeqSetOffsets ******************************************************
**
** Sets the offsets for each end of a subsequence.
**
** Needed mainly for local alignments so the original sequence numbering
** can be preserved.
**
** @param [u] seq [AjPSeq] Sequence object to be set.
** @param [r] offset [ajint] Offset from start of original sequence
** @param [r] origlen [ajint] Original length, used to calculate the offset
**                             from the end.
** @return [void]
**
** @release 3.0.0
** @@
******************************************************************************/

void ajSeqSetOffsets(AjPSeq seq, ajint offset, ajint origlen)
{
    ajDebug("ajSeqSetOffsets(len:%d gap:%d off:%d origlen:%d) "
	    "Offset:%d Offend:%d\n",
	    ajSeqGetLen(seq), ajSeqCountGaps(seq),
	    offset, origlen, seq->Offset, seq->Offend);

    if(seq->Trimmed)
    {
	ajWarn("Sequence '%S' already trimmed in ajSeqSetOffsets",
	       ajSeqGetNameS(seq));
    }

    if(seq->Reversed)
    {
	if(offset && !seq->Offend)
	    seq->Offend = offset;

	if(origlen && !seq->Offset)
	{
	    seq->Offset = origlen - offset - ajSeqGetLen(seq)
		+ ajSeqCountGaps(seq);
	}
    }
    else
    {
	if(offset && !seq->Offset)
	    seq->Offset = offset;

	if(origlen && !seq->Offend)
	{
	    seq->Offend = origlen - offset - ajSeqGetLen(seq)
		+ ajSeqCountGaps(seq);
	}
    }

    ajDebug("      result: (len: %d truelen:%d Offset:%d Offend:%d)\n",
	    ajSeqGetLen(seq), ajSeqGetLen(seq)-ajSeqCountGaps(seq),
	    seq->Offset, seq->Offend);

    return;
}




/* @func ajSeqSetRange ********************************************************
**
** Sets the start and end positions for a sequence (not for a sequence set).
** Trim the sequence to convert to the subsequence.
**
** @param [u] seq [AjPSeq] Sequence object to be set.
** @param [r] pos1 [ajint] Start position. Negative values are from the end.
** @param [r] pos2 [ajint] End position. Negative values are from the end.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajSeqSetRange(AjPSeq seq, ajint pos1, ajint pos2)
{
    ajDebug("ajSeqSetRange (len: %d %d..%d old %d..%d) rev:%B reversed:%B\n",
	    ajSeqGetLen(seq), pos1, pos2,
	    seq->Begin, seq->End, seq->Rev, seq->Reversed);

    if(seq->Trimmed)
    {
	ajWarn("Sequence '%S' already trimmed in ajSeqSetRange",
	       ajSeqGetNameS(seq));
    }

    if(pos1 && !seq->Begin)
	seq->Begin = pos1;

    if(pos2 && !seq->End)
	seq->End = pos2;

    ajDebug("      result: (len: %d %d..%d)\n",
	    ajSeqGetLen(seq), seq->Begin, seq->End);

    if(seq->Rev && !seq->Reversed)
	ajSeqReverseDo(seq);

    return;
}




/* @func ajSeqSetRangeRev *****************************************************
**
** Sets the start and end positions for a sequence (not for a sequence set),
** and set the sequence to be reversed.
**
** @param [u] seq [AjPSeq] Sequence object to be set.
** @param [r] pos1 [ajint] Start position. Negative values are from the end.
** @param [r] pos2 [ajint] End position. Negative values are from the end.
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajSeqSetRangeRev(AjPSeq seq, ajint pos1, ajint pos2)
{
    
    ajDebug("ajSeqSetRange (len: %d %d..%d old %d..%d) rev:%B reversed:%B\n",
	    ajSeqGetLen(seq), pos1, pos2,
	    seq->Begin, seq->End, seq->Rev, seq->Reversed);

    if(seq->Trimmed)
	ajWarn("Sequence '%S' already trimmed in ajSeqSetRangeRev",
	       ajSeqGetNameS(seq));

    if(pos1 && !seq->Begin)
	seq->Begin = pos1;

    if(pos2 && !seq->End)
	seq->End = pos2;

    ajDebug("      result: (len: %d %d..%d)\n",
	    ajSeqGetLen(seq), seq->Begin, seq->End);

    if(!seq->Rev)
    {
	seq->Rev = ajTrue;
	seq->Reversed = ajFalse;
    }

    ajSeqReverseDo(seq);

    return;
}




/* @func ajSeqSetUnique *******************************************************
**
** Makes a sequence modifiable by making sure there is no duplicate
** copy of the sequence.
**
** @param [u] seq [AjPSeq] Sequence
** @return [void]
**
** @release 4.1.0
** @@
******************************************************************************/

void ajSeqSetUnique(AjPSeq seq)
{
    ajStrGetuniqueStr(&seq->Seq);

    return;
}




/* @func ajSeqTagRev **********************************************************
**
** Adds a tag to the sequence description to note it has been reversed
**
** @param [u] seq [AjPSeq] Sequence object to be set.
** @return [void]
**
** @release 6.1.0
******************************************************************************/

void ajSeqTagRev(AjPSeq seq)
{
    if(ajStrGetLen(seq->Desc))
        ajStrInsertC(&seq->Desc, 0, "Reversed: ");
    else
        ajStrAssignC(&seq->Desc, "Reversed:");

    return;
}




/* @funcstatic seqMakeUsa *****************************************************
**
** Sets the USA for a sequence.
**
** @param [r] thys [const AjPSeq] Sequence object
** @param [w] usa [AjPStr*] USA in full
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

static void seqMakeUsa(const AjPSeq thys, AjPStr* usa)
{
    AjPStr tmpstr = NULL;

    ajDebug("seqMakeUsa (Name <%S> Formatstr <%S> Db <%S> "
	    "Entryname <%S> Filename <%S>)\n",
	    thys->Name, thys->Formatstr, thys->Db,
	    thys->Entryname, thys->Filename);

    ajDebug("Begin:%d End:%d Rev:%B Reversed:%B\n",
	    thys->Begin, thys->End, thys->Rev, thys->Reversed);
    /* ajSeqTrace(thys); */

    if(ajStrGetLen(thys->Db))
	ajFmtPrintS(usa, "%S-id:%S", thys->Db, thys->Entryname);
    else
    {
	/*ajFmtPrintS(&thys->Usa, "%S::%S (%S)",
	  thys->Formatstr, thys->Filename, thys->Entryname);*/
	if(ajStrMatchC(thys->Formatstr, "asis"))
        {
	    if(thys->Reversed)
	    {
		ajStrAssignS(&tmpstr, thys->Seq);
		ajSeqstrReverse(&tmpstr);
		ajFmtPrintS(usa, "asis::%S", tmpstr);
		ajStrDel(&tmpstr);
	    }
	    else
		ajFmtPrintS(usa, "asis::%S", thys->Seq);
        }
	else if(ajStrGetLen(thys->Entryname))
	    ajFmtPrintS(usa, "%S::%S:%S", thys->Formatstr,
			thys->Filename,thys->Entryname);
	else
	    ajFmtPrintS(usa, "%S::%S", thys->Formatstr,
			thys->Filename);

    }

    ajFmtPrintS(&tmpstr, "[");

    if(thys->Reversed)
    {
	if(thys->End)
	    ajFmtPrintAppS(&tmpstr, "%d", -thys->End);

	ajFmtPrintAppS(&tmpstr, ":");

	if(thys->Begin)
	    ajFmtPrintAppS(&tmpstr, "%d", -thys->Begin);

	ajFmtPrintAppS(&tmpstr, ":r");
    }
    else
    {
	if(thys->Begin)
	    ajFmtPrintAppS(&tmpstr, "%d", thys->Begin);

	ajFmtPrintAppS(&tmpstr, ":");

	if(thys->End)
	    ajFmtPrintAppS(&tmpstr, "%d", thys->End);
    }

    ajFmtPrintAppS(&tmpstr, "]");

    if(ajStrGetLen(tmpstr) > 3)
	ajStrAppendS(usa, tmpstr);

    ajStrDel(&tmpstr);
    ajDebug("      result: <%S>\n",
	    *usa);

    return;
}




/* @section process *******************************************************
**
** These functions use the contents of a sequence object to produce a
** subsequence from the range, or to reverse a sequence whose direction
** has been set to be reversed.
**
** @fdata [AjPSeq]
** @fcategory modify
**
** @nam3rule Complement Complement the bases
** @nam3rule Fmt Format sequence characters
** @nam4rule FmtLower Format sequence characters to lower case
** @nam4rule FmtUpper Format sequence characters to upper case
** @nam3rule Gap Process gaps in sequence
** @nam4rule GapFill Fill sequence to a given length with end gaps
** @nam4rule GapStandard Make all gap characters use a given character
** @nam3rule Reverse Reverse the sequence
** @nam4rule ReverseDo Reverse if Rev attribute is set
** @nam4rule ReverseForce Reverse the sequence without testing the
**                        Rev attribute
** @nam4rule ReverseOnly Reverse the sequence but do not complement the bases
** @nam3rule Trim Trim sequence using defined range
**
** @argrule * seq [AjPSeq] Sequence to be processed
** @argrule GapFill len [ajuint] Padded sequence length
** @argrule GapStandard gapchar [char] Preferred gap character
**
** @valrule * [void]
******************************************************************************/




/* @func ajSeqComplement ******************************************************
**
** Complements but does not reverse a nucleotide sequence.
**
** @param [u] seq [AjPSeq] Sequence
** @return [void]
**
** @release 4.1.0
** @@
******************************************************************************/

void ajSeqComplement(AjPSeq seq)
{
    ajSeqstrComplement(&seq->Seq);

    return;
}




/* @func ajSeqFmtLower ********************************************************
**
** Converts a sequence to lower case.
**
** @param [u] seq [AjPSeq] Sequence
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajSeqFmtLower(AjPSeq seq)
{
    ajStrFmtLower(&seq->Seq);

    return;
}




/* @func ajSeqFmtUpper ********************************************************
**
** Converts a sequence to upper case.
**
** @param [u] seq [AjPSeq] Sequence
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajSeqFmtUpper(AjPSeq seq)
{
    ajStrFmtUpper(&seq->Seq);

    return;
}




/* @func ajSeqGapFill *********************************************************
**
** Fills a single sequence with gaps up to a specified length.
**
** @param [u] seq [AjPSeq] Sequence object to be set.
** @param [r] len [ajuint] Length to pad fill to.
** @return [void]
**
** @release 4.1.0
** @@
******************************************************************************/

void ajSeqGapFill(AjPSeq seq, ajuint len)
{
    ajuint ilen = 0;

    ajDebug("ajSeqGapFill(len: %d -> ilen:%d)\n", ajSeqGetLen(seq), ilen);

    if(ajSeqGetLen(seq) < len)
    {
	ilen = len - ajSeqGetLen(seq);
	ajStrAppendCountK(&seq->Seq, '-', ilen);
    }

    ajDebug("      result: (len: %d added: %d\n",
	     ajSeqGetLen(seq), ilen);

    return;
}




/* @func ajSeqGapStandard *****************************************************
**
** Makes all gaps in a string use a standard gap character
**
** @param [w] seq [AjPSeq] Sequence object
** @param [r] gapchar [char] Gap character (or '-' if zero)
** @return [void]
**
** @release 2.1.0
******************************************************************************/

void ajSeqGapStandard(AjPSeq seq, char gapchar)
{
    char newgap = '-';
    static char testchars[] = "-~.? "; /* all known gap characters */
    char *testgap;

    testgap = testchars;

    if(gapchar)
	newgap = gapchar;

    /*ajDebug("ajSeqGapStandardS '%c'=>'%c' '%S'\n",
            gapch, newgap, seq->Seq);*/

    while(*testgap)
    {
	if(newgap != *testgap)
	{
	    ajStrExchangeKK(&seq->Seq, *testgap, newgap);
	    /*ajDebug(" ajSeqGapStandardS replaced         '%c'=>'%c' '%S'\n",
		    *testgap, newgap, thys);*/
	}

	testgap++;
    }

    return;
}




/* @func ajSeqReverseDo *******************************************************
**
** Reverses and complements a nucleotide sequence, unless it is already done.
**
** If the sequence is not flagged for reversal, use ajSeqReverseForce instead.
**
** @param [u] seq [AjPSeq] Sequence
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajSeqReverseDo(AjPSeq seq)
{
    ajint ibegin;
    ajint iend;
    ajint itemp;
    ajuint i;
    ajuint j;
    float atemp;

    ajDebug("ajSeqReverseDo len: %d Begin: %d End: %d Rev: %B Reversed: %B\n",
	    ajSeqGetLen(seq), seq->Begin, seq->End,
	    seq->Rev, seq->Reversed);

    if(!seq->Rev)			/* Not flagged for reversal */
	return;

    ibegin = seq->Begin;
    iend   = seq->End;

    seq->End   = -(ibegin);
    seq->Begin = -(iend);

    itemp = seq->Offend;
    seq->Offend = seq->Offset;
    seq->Offset = itemp;

    seq->Rev = ajFalse;

    if(seq->Reversed)
	seq->Reversed = ajFalse;
    else
	seq->Reversed = ajTrue;

    ajSeqstrReverse(&seq->Seq);

    ajDebug("      result len: %d Begin: %d End: %d\n",
	    ajSeqGetLen(seq), seq->Begin, seq->End);

    if(seq->Fttable)
    {
        ajFeattableSetReverse(seq->Fttable);
	ajFeattableReverse(seq->Fttable);
    }

    if(seq->Accuracy)
    {
        j = seq->Qualsize;
        for(i=0;i<seq->Qualsize;i++)
        {
            if(--j <= i) break;
            atemp = seq->Accuracy[j];
            seq->Accuracy[j] = seq->Accuracy[i];
            seq->Accuracy[i] = atemp;
        }
    }
    
    return;
}




/* @func ajSeqReverseForce ****************************************************
**
** Reverses and complements a nucleotide sequence.
** Forces reversal to be done even if the sequence is flagged
** as already reversed.
**
** This happens, for example, where an input sequence has been reversed
** with -sreverse on the command line, but the application needs to reverse it
** in processing both directions.
**
** @param [u] seq [AjPSeq] Sequence
** @return [void]
**
** @release 2.7.0
** @@
******************************************************************************/

void ajSeqReverseForce(AjPSeq seq)
{
    ajDebug("ajSeqReverseForce len: %d Begin: %d End: %d Rev: %B "
	    "Reversed: %B\n",
	    ajSeqGetLen(seq), seq->Begin, seq->End,
	    seq->Rev, seq->Reversed);

    seq->Rev = ajTrue;
    ajSeqReverseDo(seq);

    return;
}




/* @func ajSeqReverseOnly *****************************************************
**
** Reverses but does not complement a nucleotide sequence.
**
** Intended for sequence display, but can be used to fix incorrect
** sequence entries.
**
** @param [u] seq [AjPSeq] Sequence
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajSeqReverseOnly(AjPSeq seq)
{
    ajint ibegin;
    ajint iend;

    ajDebug("ajSeqRevOnly len: %d Begin: %d End: %d\n",
	    ajSeqGetLen(seq), seq->Begin, seq->End);

    ibegin = seq->Begin;
    iend   = seq->End;

    if(ibegin)
	seq->End   = -(ibegin);

    if(iend)
	seq->Begin = -(iend);

    ajStrReverse(&seq->Seq);

    ajDebug(" only result len: %d Begin: %d End: %d\n",
	    ajSeqGetLen(seq), seq->Begin, seq->End);

    return;
}




/* @func ajSeqTrim ************************************************************
**
** Trim a sequence using the Begin and Ends.
** Also reverse complements a nucleotide sequence if required.
**
** @param [u] seq [AjPSeq] Sequence to be trimmed.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajSeqTrim(AjPSeq seq)
{
    AjBool okay = ajTrue;
    ajint begin;
    ajint end;

    ajint jbegin;
    ajint jend;

    /*ajDebug("ajSeqTrim '%S'\n", seq->Seq);*/
    ajDebug("ajSeqTrim '%S' Rev:%B Reversed:%B Begin:%d End:%d "
	    "Offset:%d Offend:%d Len:%d\n",
	    ajSeqGetNameS(seq), seq->Rev, seq->Reversed,
	    seq->Begin, seq->End,
	    seq->Offset, seq->Offend, seq->Seq->Len);

    if(seq->Trimmed)
    {
	ajWarn("Sequence '%S' already trimmed", ajSeqGetNameS(seq));

	return;
    }

    /*ajDebug("ajSeqTrim '%S'\n", seq->Seq);*/
    ajDebug("ajSeqTrim Rev:%B Reversed:%B Begin:%d End:%d "
	   "Offset:%d Offend:%d Len:%d okay:%B\n",
	    seq->Rev, seq->Reversed, seq->Begin, seq->End,
	    seq->Offset, seq->Offend, seq->Seq->Len, okay);

    jbegin = seq->Begin;

    if(jbegin > 0)
	jbegin--;

    jend = seq->End;

    if(jend > 0)
	jend--;

/*
    if(jbegin > (ajint) ajSeqGetLen(seq))
        ajWarn("sequence begin %d more than length %u",
               jbegin, ajSeqGetLen(seq));
    if(jend > (ajint) ajSeqGetLen(seq))
        ajWarn("sequence end %d more than length %u",
               jbegin, ajSeqGetLen(seq));
*/

    begin = 1 + ajCvtSposToPosStart(ajSeqGetLen(seq), 0, jbegin);
    end   = 1 + ajCvtSposToPosStart(ajSeqGetLen(seq), begin-1, jend);

    ajDebug("Trimming %d from %d (%d) to %d (%d) "
	    "Rev: %B Reversed: %B Trimmed: %B\n",
	    seq->Seq->Len,seq->Begin,begin, seq->End, end,
	    seq->Rev, seq->Reversed, seq->Trimmed);

    if(seq->End)
    {
	if(end < begin)
	    return;

	seq->Offend = seq->Seq->Len-(end);
	okay = ajStrCutEnd(&(seq->Seq),seq->Seq->Len-(end));
	seq->End    = 0;
    }

    if(seq->Begin)
    {
	okay = ajStrCutStart(&seq->Seq,begin-1);
	seq->Offset = begin-1;
	seq->Begin = 0;
    }

    ajDebug("After Trimming len = %d off = %d offend = %d\n",
	    seq->Seq->Len, seq->Offset, seq->Offend);
    /*ajDebug("After Trimming len = %d '%S'\n",thys->Seq->Len, thys->Seq);*/


    if(okay && seq->Fttable)
	okay = ajFeattableTrimOff(seq->Fttable, seq->Offset, seq->Seq->Len);

    if(seq->Rev)
	ajSeqReverseDo(seq);

    /*ajDebug("ajSeqTrim '%S'\n", seq->Seq);*/
    ajDebug("ajSeqTrim 'Rev:%B Reversed:%B Begin:%d End:%d "
	    "Offset:%d Offend:%d Len:%d okay:%B\n",
	    seq->Rev, seq->Reversed, seq->Begin, seq->End,
	    seq->Offset, seq->Offend, seq->Seq->Len, okay);
    
    return;
}




/* @section element retrieval
**
** These functions return the contents of a sequence object.
**
** @fdata [AjPSeq]
** @fcategory use
**
** @nam3rule Get Return sequence attribute(s)
** @nam4rule GetAcc Return sequence accession number
** @nam4rule GetBegin Return sequence begin
** @nam4rule GetDb Return database name
** @nam4rule GetDesc Return sequence description
** @nam4rule GetEnd Return sequence end
** @nam4rule GetEntry Return entry text
** @nam4rule GetFeat Return sequence feature table
** @nam4rule GetGi Return sequence GI number
** @nam4rule GetLen Return sequence length
** @nam4rule GetName Return sequence name
** @nam4rule GetOffend Return sequence end offset
** @nam4rule GetOffset Return sequence start offset
** @nam4rule GetQual Return quality scores
** @nam4rule GetRange Return sequence begin and end
** @nam4rule GetRev Return sequence reverse attribute (not yet reversed)
** @nam4rule GetSeq Return sequence
** @nam4rule GetSv Return sequence version
** @nam4rule GetTax Return taxonomy
** @nam4rule GetTaxid Return taxonomy id
** @nam4rule GetUsa Return sequence USA
** @nam4rule GetXrefs return cross-references
**
** @suffix S Return a string
** @suffix C Return a character string
** @suffix Trimmed After trimming begin and end positions
** @suffix True True position in original sequence
** @suffix Ungapped Ignoring all gap characters
** @suffix Copy Editable copy
**
** @argrule * seq [const AjPSeq] Sequence
** @argrule Range begin [ajint*] Start position
** @argrule Range end [ajint*] End position
** @argrule Xrefs list [AjPList] Cross-references
**
** @valrule C [const char*]
** @valrule S [const AjPStr]
** @valrule Begin [ajuint] Sequence position
** @valrule End [ajuint] Sequence position
** @valrule Len [ajuint] Sequence length
** @valrule Offend [ajuint] Sequence end offset
** @valrule Offset [ajuint] Sequence start offset
** @valrule Qual [const float*] Sequence phred quality scores
** @valrule Range [ajuint] Sequence length
** @valrule Rev [AjBool] Reverse attribute
** @valrule Revtext [AjPStr] Reverse text
** @valrule *Feat [const AjPFeattable] Link to internal feature table
** @valrule *FeatCopy [AjPFeattable] New feature table with original contents
** @valrule *SeqCopyC [char*] New sequence with original contents
** @valrule *SeqCopyS [AjPStr] New sequence with original contents
** @valrule Taxid [ajuint] NCBI taxonomy ID
** @valrule Xrefs [ajuint] Number of cross-references
******************************************************************************/




/* @func ajSeqGetAccC *********************************************************
**
** Returns the sequence accession number.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeq] Sequence object.
** @return [const char*] Accession number as a character string.
**
** @release 4.0.0
** @@
******************************************************************************/

const char* ajSeqGetAccC(const AjPSeq seq)
{
    return MAJSTRGETPTR(seq->Acc);
}




/* @func ajSeqGetAccS *********************************************************
**
** Returns the sequence accession number.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeq] Sequence object.
** @return [const AjPStr] Accession number as a string.
**
** @release 4.0.0
** @@
******************************************************************************/

const AjPStr ajSeqGetAccS(const AjPSeq seq)
{
    if(seq->Acc)
        return seq->Acc;
    return ajStrConstEmpty();
}




/* @func ajSeqGetBegin ********************************************************
**
** Returns the sequence start position within the current stored sequence,
** or 1 if no start has been set.
**
** To return the position within the original sequence, which may be different
** if the sequence has been trimmed, use ajSeqGetBeginTrue
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [ajuint] Start position.
**
** @release 4.0.0
** @@
******************************************************************************/

ajuint ajSeqGetBegin(const AjPSeq seq)
{
    ajint i;
    ajint j;

    if(!seq->Begin)
	return 1;

    i = seq->Begin;

    if(seq->Begin > 0)
	i = seq->Begin - 1;

    j =  1 + ajCvtSposToPosStart(ajSeqGetLen(seq), 0, i);

    return j;
}




/* @func ajSeqGetBeginTrue ****************************************************
**
** Returns the sequence start position in the original sequence,
** which may have been trimmed.
**
** To return the position within the current stored sequence,
** which may be different if the sequence has been trimmed, use ajSeqBegin
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [ajuint] Start position.
**
** @release 4.0.0
** @@
******************************************************************************/

ajuint ajSeqGetBeginTrue(const AjPSeq seq)
{
    if(!seq->Begin)
	return ajSeqCalcTruepos(seq, 1);

    return ajSeqCalcTruepos(seq, seq->Begin);
}




/* @func ajSeqGetDbC **********************************************************
**
** Returns the sequence database name.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeq] Sequence object.
** @return [const char*] Database name as a character string.
**
** @release 5.0.0
** @@
******************************************************************************/

const char* ajSeqGetDbC(const AjPSeq seq)
{
    return MAJSTRGETPTR(seq->Db);
}




/* @func ajSeqGetDbS **********************************************************
**
** Returns the sequence database name.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeq] Sequence object.
** @return [const AjPStr] Database name as a string.
**
** @release 5.0.0
** @@
******************************************************************************/

const AjPStr ajSeqGetDbS(const AjPSeq seq)
{
    if(seq->Db)
        return seq->Db;
    return ajStrConstEmpty();
}




/* @func ajSeqGetDescC ********************************************************
**
** Returns the sequence description.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeq] Sequence object.
** @return [const char*] Description as a character string.
**
** @release 4.0.0
** @@
******************************************************************************/

const char* ajSeqGetDescC(const AjPSeq seq)
{
    return MAJSTRGETPTR(seq->Desc);
}




/* @func ajSeqGetDescS ********************************************************
**
** Returns the sequence description.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeq] Sequence object.
** @return [const AjPStr] Description as a string.
**
** @release 4.0.0
** @@
******************************************************************************/

const AjPStr ajSeqGetDescS(const AjPSeq seq)
{
    if(seq->Desc)
        return seq->Desc;
    return ajStrConstEmpty();
}




/* @func ajSeqGetEnd **********************************************************
**
** Returns the sequence end position, or the sequence length if no end
** has been set.
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [ajuint] End position.
**
** @release 4.0.0
** @@
******************************************************************************/

ajuint ajSeqGetEnd(const AjPSeq seq)
{
    ajint i;

    if(!seq->End)
	return (ajSeqGetLen(seq));

    i = seq->End;

    if(seq->End > 0)
	i--;

    i = 1 +ajCvtSposToPosStart(ajSeqGetLen(seq), ajSeqGetBegin(seq)-1, i);

    return i;
}




/* @func ajSeqGetEndTrue ******************************************************
**
** Returns the sequence end position, or the sequence length if no end
** has been set.
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [ajuint] End position.
**
** @release 4.0.0
** @@
******************************************************************************/

ajuint ajSeqGetEndTrue(const AjPSeq seq)
{
    if(!seq->End)
    {
	if(ajSeqIsReversed(seq))
	    return seq->Offend + ajSeqGetLen(seq);
	else
	    return seq->Offset + ajSeqGetLen(seq);
    }

    return ajSeqCalcTrueposMin(seq, ajSeqGetBeginTrue(seq), seq->End);
}




/* @func ajSeqGetEntryC *******************************************************
**
** Returns the sequence full text entry.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeq] Sequence object.
** @return [const char*] Entry as a character string.
**
** @release 4.0.0
** @@
******************************************************************************/

const char* ajSeqGetEntryC(const AjPSeq seq)
{
    return MAJSTRGETPTR(seq->TextPtr);
}




/* @func ajSeqGetEntryS *******************************************************
**
** Returns the sequence full text entry.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeq] Sequence object.
** @return [const AjPStr] Entry as a string.
**
** @release 4.0.0
** @@
******************************************************************************/

const AjPStr ajSeqGetEntryS(const AjPSeq seq)
{
    if(seq->TextPtr)
        return seq->TextPtr;
    return ajStrConstEmpty();
}




/* @func ajSeqGetFeat *********************************************************
**
** Returns the sequence feature table.
** Because this is a pointer to the real internal table
** the caller must take care not to change it in any way,
** or to delete it.
**
** If the table is to be changed or deleted then it must first
** be copied with ajSeqCopyFeat
**
** @param [r] seq [const AjPSeq] Sequence object.
** @return [const AjPFeattable] feature table (if any)
**
** @release 1.0.0
** @@
******************************************************************************/

const AjPFeattable ajSeqGetFeat(const AjPSeq seq)
{
    return seq->Fttable;
}




/* @func ajSeqGetFeatCopy *****************************************************
**
** Returns a copy of the sequence feature table.
** Because this is a copy of all the data, the caller is responsible
** for deleting it after use.
**
** If the table is not to be changed or deleted then ajSeqGetFeat
** can return a copy of the internal pointer.
**
** @param [r] seq [const AjPSeq] Sequence object.
** @return [AjPFeattable] feature table (if any)
**
** @release 4.0.0
** @@
******************************************************************************/

AjPFeattable ajSeqGetFeatCopy(const AjPSeq seq)
{
    return ajFeattableNewFtable(seq->Fttable);
}




/* @func ajSeqGetGiC **********************************************************
**
** Returns the GI version number.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeq] Sequence object.
** @return [const char*] SeqVersion number as a character string.
**
** @release 4.0.0
** @@
******************************************************************************/

const char* ajSeqGetGiC(const AjPSeq seq)
{
    return MAJSTRGETPTR(seq->Gi);
}




/* @func ajSeqGetGiS **********************************************************
**
** Returns the GI version number.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeq] Sequence object.
** @return [const AjPStr] SeqVersion number as a string.
**
** @release 4.0.0
** @@
******************************************************************************/

const AjPStr ajSeqGetGiS(const AjPSeq seq)
{
    if(seq->Gi)
        return seq->Gi;
    return ajStrConstEmpty();
}




/* @func ajSeqGetLen **********************************************************
**
** Returns the sequence length.
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [ajuint] Sequence length.
**
** @release 4.0.0
** @@
******************************************************************************/

ajuint ajSeqGetLen(const AjPSeq seq)
{
    return ajStrGetLen(seq->Seq);
}




/* @func ajSeqGetLenTrimmed ***************************************************
**
** Returns the sequence length after trimming begin and end positions
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [ajuint] Sequence length.
**
** @release 6.1.0
** @@
******************************************************************************/

ajuint ajSeqGetLenTrimmed(const AjPSeq seq)
{
    return ajSeqGetEnd(seq) - ajSeqGetBegin(seq) + 1 ;
}




/* @func ajSeqGetLenTrue ******************************************************
**
** Returns the length of the original sequence, including any gap characters.
**
** @param [r] seq [const AjPSeq] Target sequence.
** @return [ajuint] string position between 1 and length.
**
** @release 4.0.0
** @@
******************************************************************************/

ajuint ajSeqGetLenTrue(const AjPSeq seq)
{
    return (ajStrGetLen(seq->Seq) + seq->Offset + seq->Offend);
}




/* @func ajSeqGetLenUngapped **************************************************
**
** Returns the sequence length excluding all gap characters.
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [ajuint] Sequence length.
**
** @release 6.1.0
** @@
******************************************************************************/

ajuint ajSeqGetLenUngapped(const AjPSeq seq)
{
    ajint ret = 0;
    const char *gapchars = "-.~";
    const char* cp;
    size_t i;

    cp = ajStrGetPtr(seq->Seq);
    i = strspn(cp, gapchars);
    cp += i;

    while (*cp)
    {
        i = strcspn(cp, gapchars);
        ret += i;
        cp += i;
        cp += strspn(cp, gapchars);
    }
    
    return ret;
}




/* @func ajSeqGetNameC ********************************************************
**
** Returns the sequence name.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeq] Sequence object.
** @return [const char*] Name as a character string.
**
** @release 4.0.0
** @@
******************************************************************************/

const char* ajSeqGetNameC(const AjPSeq seq)
{
    if(!seq)
        return "";

    return MAJSTRGETPTR(seq->Name);
}




/* @func ajSeqGetNameS ********************************************************
**
** Returns the sequence name.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeq] Sequence object.
** @return [const AjPStr] Name as a string.
**
** @release 4.0.0
** @@
******************************************************************************/

const AjPStr ajSeqGetNameS(const AjPSeq seq)
{
    if(!seq)
        return ajStrConstEmpty();

    if(seq->Name)
        return seq->Name;

    return ajStrConstEmpty();
}




/* @func ajSeqGetOffend *******************************************************
**
** Returns the sequence offend value.
** This is the number of positions removed from the original end.
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [ajuint] Sequence offend.
**
** @release 4.0.0
** @@
******************************************************************************/

ajuint ajSeqGetOffend(const AjPSeq seq)
{
    return seq->Offend;
}




/* @func ajSeqGetOffset *******************************************************
**
** Returns the sequence offset from -sbegin originally.
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [ajuint] Sequence offset.
**
** @release 4.0.0
** @@
******************************************************************************/

ajuint ajSeqGetOffset(const AjPSeq seq)
{
    return seq->Offset;
}




/* @func ajSeqGetQual *********************************************************
**
** Returns the base quality scores as an array.
** Because this is a pointer to the real internal array
** the caller must take care not to change the array in any way.
**
** @param [r] seq [const AjPSeq] Sequence.
** @return [const float*] Base quality scores.
**
** @release 6.3.0
** @@
******************************************************************************/

const float* ajSeqGetQual(const AjPSeq seq)
{
    if(!seq)
	return NULL;

    return seq->Accuracy;
}




/* @func ajSeqGetRange ********************************************************
**
** Returns the sequence range for a sequence.
**
** @param [r] seq [const AjPSeq] Sequence object.
** @param [w] begin [ajint*] Sequence range begin
** @param [w] end [ajint*] Sequence range end
** @return [ajuint] Sequence range length
**
** @release 1.0.0
** @@
******************************************************************************/

ajuint ajSeqGetRange(const AjPSeq seq, ajint* begin, ajint* end)
{
    ajint jbegin;
    ajint jend;

    jbegin = seq->Begin;

    if(seq->Begin > 0)
	jbegin--;

    jend = seq->End;

    if(seq->End > 0)
	jend--;

    ajDebug("ajSeqGetRange '%S'\n", seq->Name);
    *begin = ajCvtSposToPos(ajSeqGetLen(seq), jbegin); /* string position */

    if(seq->End)
	*end = 1 + ajCvtSposToPosStart(ajSeqGetLen(seq), *begin, jend);
    else
	*end = 1 + ajCvtSposToPosStart(ajSeqGetLen(seq),
                                       *begin, ajSeqGetLen(seq));

    (*begin)++;				/* sequence positions are 1..end */

    return (*end - *begin + 1);
}




/* @func ajSeqGetRev **********************************************************
**
** Returns the sequence direction.
**
** See ajSeqReversed for whether it has already been reverse-complemented
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [AjBool] Sequence Direction.
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajSeqGetRev(const AjPSeq seq)
{
    return seq->Rev;
}




/* @func ajSeqGetSeqC *********************************************************
**
** Returns the sequence in a sequence object as a character string.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeq] Sequence.
** @return [const char*] Sequence as a character string.
**
** @release 4.0.0
** @@
******************************************************************************/

const char* ajSeqGetSeqC(const AjPSeq seq)
{
    if(!seq)
	return "";

    return MAJSTRGETPTR(seq->Seq);
}




/* @func ajSeqGetSeqS *********************************************************
**
** Returns the sequence in a sequence object as a string.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeq] Sequence.
** @return [const AjPStr] Sequence as a string.
**
** @release 4.0.0
** @@
******************************************************************************/

const AjPStr ajSeqGetSeqS(const AjPSeq seq)
{
    if(!seq)
	return NULL;
    if(seq->Seq)
        return seq->Seq;
    return ajStrConstEmpty();
}




/* @func ajSeqGetSeqCopyC *****************************************************
**
** Returns a sequence as a C character string. This is a copy of the string
** so the caller can do anything with it.
** It must be copied back
** to a sequence (e.g. with ajSeqReplace) before output.
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [char*] Sequence as a null terminated character string.
**
** @release 4.0.0
** @@
******************************************************************************/

char* ajSeqGetSeqCopyC(const AjPSeq seq)
{
    return ajCharNewS(seq->Seq);
}




/* @func ajSeqGetSeqCopyS *****************************************************
**
** Returns the sequence in a sequence object as a string.
** Because this is a copy of the internal string
** the caller may change the string. It must be copied back
** to a sequence (e.g. with ajSeqReplace) before output.
**
** @param [r] seq [const AjPSeq] Sequence.
** @return [AjPStr] Sequence as a string.
**
** @release 4.0.0
** @@
******************************************************************************/

AjPStr ajSeqGetSeqCopyS(const AjPSeq seq)
{
    static AjPStr str;

    str = ajStrNewS(seq->Seq);

    return str;
}




/* @func ajSeqGetSvC **********************************************************
**
** Returns the sequence version number.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeq] Sequence object.
** @return [const char*] SeqVersion number as a character string.
**
** @release 4.0.0
** @@
******************************************************************************/

const char* ajSeqGetSvC(const AjPSeq seq)
{
    return MAJSTRGETPTR(seq->Sv);
}




/* @func ajSeqGetSvS **********************************************************
**
** Returns the sequence version number.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeq] Sequence object.
** @return [const AjPStr] SeqVersion number as a string.
**
** @release 4.0.0
** @@
******************************************************************************/

const AjPStr ajSeqGetSvS(const AjPSeq seq)
{
    if(seq->Sv)
        return seq->Sv;
    return ajStrConstEmpty();
}




/* @func ajSeqGetTaxC *********************************************************
**
** Returns the sequence primary taxon (species).
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeq] Sequence object.
** @return [const char*] Description as a character string.
**
** @release 4.0.0
** @@
******************************************************************************/

const char* ajSeqGetTaxC(const AjPSeq seq)
{
    return MAJSTRGETPTR(seq->Tax);
}




/* @func ajSeqGetTaxS *********************************************************
**
** Returns the sequence primary taxon (species).
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeq] Sequence object.
** @return [const AjPStr] Description as a string.
**
** @release 4.0.0
** @@
******************************************************************************/

const AjPStr ajSeqGetTaxS(const AjPSeq seq)
{
    if(seq->Tax)
        return seq->Tax;
    return ajStrConstEmpty();
}




/* @func ajSeqGetTaxid ********************************************************
**
** Returns the numeric identifier of the sequence primary taxon (species).
** This is an identifier from the NCBI taxonomy
**
** @param [r] seq [const AjPSeq] Sequence object.
** @return [ajuint] Taxon ID.
**
** @release 6.1.0
** @@
******************************************************************************/

ajuint ajSeqGetTaxid(const AjPSeq seq)
{
    ajuint ret = 0;
    ajuint i = 0;

    if(ajStrGetLen(seq->Taxid))
    {
        ajStrToUint(seq->Taxid, &ret);

        return ret;
    }

    while(seqTaxon[i].Name)
    {
        if(ajStrPrefixCaseC(seq->Tax, seqTaxon[i].Name))
        {
            return seqTaxon[i].Taxid;
        }
        
        i++;
    }
    
    return 0;
}




/* @func ajSeqGetUsaC *********************************************************
**
** Returns the original USA of a sequence.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeq] Sequence object.
** @return [const char*] USA as a character string.
**
** @release 4.0.0
** @@
******************************************************************************/

const char* ajSeqGetUsaC(const AjPSeq seq)
{
    return MAJSTRGETPTR(ajSeqGetUsaS(seq));
}




/* @func ajSeqGetUsaS *********************************************************
**
** Returns the original USA of a sequence.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeq] Sequence object.
** @return [const AjPStr] USA as a string.
**
** @release 4.0.0
** @@
******************************************************************************/

const AjPStr ajSeqGetUsaS(const AjPSeq seq)
{
    ajDebug("ajSeqGetUsa '%S'\n", seq->Usa);

    if(ajStrGetLen(seq->Usa))
	return seq->Usa;

    seqMakeUsa(seq, &seqTempUsa);

    return seqTempUsa;
}




/* @func ajSeqGetXrefs ********************************************************
**
** Returns the list of cross-references derived from the sequence
**
** @param [r] seq [const AjPSeq] Sequence object.
** @param [u] list [AjPList] List of cross-reference objects
** @return [ajuint] Number of cross-references returned
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ajSeqGetXrefs(const AjPSeq seq, AjPList list)
{
    ajDebug("ajSeqGetXrefs '%S'\n", seq->Usa);

    ajSeqxreflistClone(seq->Xreflist, list);

    return (ajuint) ajListGetLength(list);
}




/* @section testing properties ***********************************************
**
** @fdata [AjPSeq]
** @fcategory use
**
** @nam3rule Is Test sequence property
** @nam4rule IsCircular Sequence is circular
** @nam4rule IsNuc Sequence is nucleotide
** @nam4rule IsProt Sequence is protein
** @nam4rule IsReversed Sequence is reversed
** @nam4rule IsSpecies Sequence is annotated as from a particular species
** @nam5rule IsSpeciesBsubtilis Sequence is from B. subtilis
** @nam5rule IsSpeciesHuman Sequence is from H. sapiens
** @nam5rule IsSpeciesMouse Sequence is from M. musculus
** @nam5rule IsSpeciesYeast Sequence is from S. cerevisiae
** @nam4rule IsTaxon Sequence is annotated as from a particular taxon id
** @nam4rule IsTrimmed Sequence is trimmed to a subsequence
**
** @suffix True Sequence properties relative to the original sequence
**
** @argrule Is seq [const AjPSeq] Sequence
** @argrule IsTaxon taxid [ajuint] NCBI Taxonomy ID
**
** @valrule * [AjBool] Sequence boolean property
**
******************************************************************************/




/* @func ajSeqIsCircular ******************************************************
**
** Returns true if a sequence is circular
**
** @param [r] seq [const AjPSeq] Sequence
** @return [AjBool] ajTrue for a circular sequence
**
** @release 2.5.0
** @@
******************************************************************************/

AjBool ajSeqIsCircular(const AjPSeq seq)
{
    return seq->Circular;
}




/* @func ajSeqIsNuc ***********************************************************
**
** Tests whether a sequence is nucleotide.
**
** @param [r] seq [const AjPSeq] Sequence
** @return [AjBool] ajTrue for a nucleotide sequence.
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajSeqIsNuc(const AjPSeq seq)
{
    ajDebug("ajSeqIsNuc Type '%S'\n", seq->Type);

    if(ajStrMatchC(seq->Type, "N"))
	return ajTrue;

    if(ajStrMatchC(seq->Type, "P"))
	return ajFalse;

    if(ajSeqTypeGapnucS(seq->Seq)) /* returns char 0 on success */
    {
	ajDebug ("ajSeqIsNuc failed\n", seq->Type);
	return ajFalse;
    }

    return ajTrue;
}




/* @func ajSeqIsProt **********************************************************
**
** Tests whether a sequence is protein.
**
** @param [r] seq [const AjPSeq] Sequence
** @return [AjBool] ajTrue for a protein sequence.
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajSeqIsProt(const AjPSeq seq)
{
    ajDebug("ajSeqIsProt Type '%S'\n", seq->Type);

    if(ajStrMatchC(seq->Type, "P"))
	return ajTrue;

    if(ajStrMatchC(seq->Type, "N"))
	return ajFalse;

    if(ajSeqTypeAnyprotS(seq->Seq))	/* returns char 0 on success */
    {
	ajDebug ("ajSeqIsProt failed\n", seq->Type);

	return ajFalse;
    }

    return ajTrue;
}




/* @func ajSeqIsReversed ******************************************************
**
** Returns whether the sequence has been reversed
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [AjBool] Sequence Direction.
**
** @release 3.0.0
** @@
******************************************************************************/

AjBool ajSeqIsReversed(const AjPSeq seq)
{
    return seq->Reversed;
}




/* @func ajSeqIsReversedTrue **************************************************
**
** Returns ajTrue if the sequence is reversed relative to the original sequence
**
** If the sequence has already been reversed, or is set to be reversed,
** the result will be true.
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [AjBool] ajTrue if sequence is set to be reversed
**
** @release 4.0.0
** @@
******************************************************************************/

AjBool ajSeqIsReversedTrue(const AjPSeq seq)
{
    if (seq->Reversed)
    {
	if (seq->Rev)
	    return ajFalse;
	else
	    return ajTrue;
    }

    return seq->Rev;
}




/* @func ajSeqIsSpeciesBsubtilis **********************************************
**
** Tests whether a sequence is annotated as B. subtilis
**
** @param [r] seq [const AjPSeq] Sequence
** @return [AjBool] ajTrue for a nucleotide sequence.
**
** @release 6.1.0
** @@
******************************************************************************/

AjBool ajSeqIsSpeciesBsubtilis(const AjPSeq seq)
{
    return ajSeqIsTaxon(seq, 1432);
}




/* @func ajSeqIsSpeciesHuman **************************************************
**
** Tests whether a sequence is annotated as human
**
** @param [r] seq [const AjPSeq] Sequence
** @return [AjBool] ajTrue for a nucleotide sequence.
**
** @release 6.1.0
** @@
******************************************************************************/

AjBool ajSeqIsSpeciesHuman(const AjPSeq seq)
{
    return ajSeqIsTaxon(seq, 9606);
}




/* @func ajSeqIsSpeciesMouse **************************************************
**
** Tests whether a sequence is annotated as mouse
**
** @param [r] seq [const AjPSeq] Sequence
** @return [AjBool] ajTrue for a nucleotide sequence.
**
** @release 6.1.0
** @@
******************************************************************************/

AjBool ajSeqIsSpeciesMouse(const AjPSeq seq)
{
    return ajSeqIsTaxon(seq, 10090);
}




/* @func ajSeqIsSpeciesYeast **************************************************
**
** Tests whether a sequence is annotated as yeast (S. cerevisiae)
**
** @param [r] seq [const AjPSeq] Sequence
** @return [AjBool] ajTrue for a nucleotide sequence.
**
** @release 6.1.0
** @@
******************************************************************************/

AjBool ajSeqIsSpeciesYeast(const AjPSeq seq)
{
    return ajSeqIsTaxon(seq, 4932);
}




/* @func ajSeqIsTaxon *********************************************************
**
** Tests whether a sequence is annotated as from a specific taxon ID.
**
** @param [r] seq [const AjPSeq] Sequence
** @param [r] taxid [ajuint] NCBI Taxonomy ID
** @return [AjBool] ajTrue for a nucleotide sequence.
**
** @release 6.1.0
** @@
******************************************************************************/

AjBool ajSeqIsTaxon(const AjPSeq seq, ajuint taxid)
{
    ajuint itax;

    itax = ajSeqGetTaxid(seq);
    if(itax != taxid)
        return ajFalse;

    return ajTrue;
}




/* @func ajSeqIsTrimmed *******************************************************
**
** Returns ajTrue if the sequence is already trimmed
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [AjBool] ajTrue if sequence is set to be reversed
**
** @release 3.0.0
** @@
******************************************************************************/

AjBool ajSeqIsTrimmed(const AjPSeq seq)
{
    return seq->Trimmed;
}




/* @section conversion *******************************************************
**
** Convert sequence to numbers for efficient processing
**
** @fdata [AjPSeq]
** @fcategory derive
**
** @nam3rule Convert Convert sequence to some other datatype
** @nam4rule ConvertNum Convert to integers
**
** @argrule * seq [const AjPSeq]
** @argrule ConvertNum cvt [const AjPSeqCvt] Conversion table
** @argrule Num Pnumseq [AjPStr*] Output numeric version of the sequence
**
** @valrule * [AjBool] True on success
******************************************************************************/




/* @func ajSeqConvertNum ******************************************************
**
** Converts a string of sequence characters to numbers using
** a conversion table.
**
** @param [r] seq [const AjPSeq] Sequence as a string
** @param [r] cvt [const AjPSeqCvt] Conversion table.
** @param [w] Pnumseq [AjPStr*] Output numeric version of the sequence.
** @return [AjBool] ajTrue on success.
**
** @release 4.1.0
** @@
******************************************************************************/

AjBool ajSeqConvertNum(const AjPSeq seq, const AjPSeqCvt cvt, AjPStr* Pnumseq)
{
    const char *cp;
    char *ncp;

    cp = ajStrGetPtr(seq->Seq);

    ajStrAssignS(Pnumseq, seq->Seq);
    ncp = ajStrGetuniquePtr(Pnumseq);

    while(*cp)
    {
	*ncp = cvt->table[(ajint)*cp];
	cp++;
	ncp++;
    }

    return ajTrue;
}




/* @section calculated properties
**
** @fdata [AjPSeq]
** @fcategory use
**
** @nam3rule Calc Calculate a value
** @nam4rule CalcCheckgcg Calculate GCG checksum
** @nam4rule CalcCount Count nucleotide bases
** @nam4rule CalcCrc Calculate cyclic redundancy checksum
** @nam4rule CalcMolwt Calculate molecular weight
** @nam4rule CalcTruepos Calculate sequence position
** @nam5rule CalcTrueposMin Calculate sequence position in range
** @nam3rule Count Count statistics over a sequence
** @nam4rule CountGaps Count gap characters
**
** @argrule Seq seq [const AjPSeq] Sequence
** @argrule CalcCount b [ajint*] Nucleotide base count
** @argrule Min imin [ajint] Minimum relative position
** @argrule Truepos ipos [ajint] Relative position
**
** @valrule CalcCheckgcg [ajint] GCG checksum
** @valrule CalcMolwt [float] Molecular weight
** @valrule CalcCrc [ajuint] Cyclic redundancy checksum
** @valrule CalcCount [void]
** @valrule CalcTruepos [ajint] Sequence position in original
** @valrule CountGaps [ajuint] Number of gap characters
******************************************************************************/




/* @func ajSeqCalcCheckgcg ****************************************************
**
** Calculates a GCG checksum for a sequence.
**
** @param [r] seq [const AjPSeq] Sequence.
** @return [ajint] GCG checksum.
**
** @release 4.0.0
** @@
******************************************************************************/

ajint ajSeqCalcCheckgcg(const AjPSeq seq)
{
    register ajlong  i;
    register ajlong  check = 0;
    register ajlong  count = 0;
    const char *cp;
    ajint ilen;

    cp   = ajStrGetPtr(seq->Seq);
    ilen = ajStrGetLen(seq->Seq);

    for(i = 0; i < ilen; i++)
    {
	count++;
	check += count * toupper((ajint) cp[i]);
	if(count == 57)
	    count = 0;
    }

    check %= 10000;

    return (ajint) check;
}




/* @func ajSeqCalcCount *******************************************************
**
** Counts the numbers of A, C, G and T in a nucleotide sequence.
**
** @param [r] seq [const AjPSeq] Sequence object
** @param [w] b [ajint*] integer array, minimum size 5, to hold the results.
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajSeqCalcCount(const AjPSeq seq, ajint* b)
{
    const char* cp;

    ajDebug("ajSeqCount %d bases\n", ajSeqGetLen(seq));

    b[0] = b[1] = b[2] = b[3] = b[4] = 0;

    cp = ajStrGetPtr(seq->Seq);

    while(*cp)
    {
	switch (*cp)
	{
            case 'A':
            case 'a':
                b[0]++;
                break;
            case 'C':
            case 'c':
                b[1]++;
                break;
            case 'G':
            case 'g':
                b[2]++;
                break;
            case 'T':
            case 't':
            case 'U':
            case 'u':
                b[3]++;
                break;
            default:
                break;
	}
	cp++;
    }

    b[4] = ajSeqGetLen(seq) - b[0] - b[1] - b[2] - b[3];

    return;
}




/* @func ajSeqCalcMolwt *******************************************************
**
** Calculates the molecular weight of a protein sequence.
**
** @param [r] seq [const AjPSeq] Sequence
** @return [float] Molecular weight.
**
** @release 4.0.0
** @@
******************************************************************************/

float ajSeqCalcMolwt(const AjPSeq seq)
{
    return ajSeqstrCalcMolwt(seq->Seq);
}




/* @func ajSeqCalcTruepos *****************************************************
**
** Converts a string position into a true position. If ipos is negative,
** it is counted from the end of the string rather than the beginning.
**
** For strings, the result can go off the end to the terminating NULL.
** For sequences the maximum is the last base
**
** @param [r] seq [const AjPSeq] Target sequence.
** @param [r] ipos [ajint] Position.
** @return [ajint] string position between 1 and length.
**
** @release 4.0.0
** @@
******************************************************************************/

ajint ajSeqCalcTruepos(const AjPSeq seq, ajint ipos)
{
    ajint jpos;

    jpos = ipos;

    if (ipos > 0)
	jpos = ipos - 1;

    if(ajSeqGetRev(seq))
	return 1 + seq->Offend + ajCvtSposToPosStart(ajSeqGetLen(seq), 0, jpos);

    return 1 + seq->Offset + ajCvtSposToPosStart(ajSeqGetLen(seq), 0, jpos);
}




/* @func ajSeqCalcTrueposMin **************************************************
**
** Converts a string position into a true position. If ipos is negative,
** it is counted from the end of the string rather than the beginning.
**
** imin is a minimum relative position, also counted from the end
** if negative. Usually this is the start position when the end of a range
** is being tested.
**
** @param [r] seq [const AjPSeq] Target sequence.
** @param [r] imin [ajint] Start position.
** @param [r] ipos [ajint] Position.
** @return [ajint] string position between 1 and length.
**
** @release 4.0.0
** @@
******************************************************************************/

ajint ajSeqCalcTrueposMin(const AjPSeq seq, ajint imin, ajint ipos)
{
    ajint jmin;
    ajint jpos;

    jmin = imin;

    if (imin > 0)
	jmin--;

    jpos = ipos;

    if(ipos > 0)
	jpos--;

    if(ajSeqGetRev(seq))
	return 1 + seq->Offend + ajCvtSposToPosStart(ajSeqGetLen(seq),
			  jmin, jpos);
    else
	return 1 + seq->Offset + ajCvtSposToPosStart(ajSeqGetLen(seq),
			  jmin, jpos);
}




/* @func ajSeqCountGaps *******************************************************
**
** Returns the number of gaps in a sequence (counting any possible
** gap character
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [ajuint] Number of gaps
**
** @release 4.1.0
******************************************************************************/

ajuint ajSeqCountGaps(const AjPSeq seq)
{
    ajuint ret = 0;

    static char testchars[] = "-~.? "; /* all known gap characters */
    const char *testgap;

    testgap = testchars;

    while(*testgap)
    {
      ret += (ajuint) ajStrCalcCountK(seq->Seq, *testgap);
	testgap++;
    }

    return ret;
}




/* @section exit **************************************************************
**
** Functions called on exit from the program by ajExit to do
** any necessary cleanup and to report internal statistics to the debug file
**
** @fdata      [AjPSeq]
** @fnote     general exit functions, no arguments
**
** @nam3rule Exit Cleanup and report on exit
**
** @valrule * [void]
**
** @fcategory misc
******************************************************************************/




/* @func ajSeqExit ************************************************************
**
** Cleans up sequence processing  internal memory
**
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajSeqExit(void)
{
    ajSeqReadExit();
    ajSeqoutExit();
    ajSeqTypeExit();

    ajStrDel(&seqVersionAccnum);
    ajStrDel(&seqTempUsa);

    ajStrDel(&seqMoleculeDef);
    ajStrDel(&seqDivisionDef);
    ajStrDel(&seqClassDef);

    ajTableDel(&seqTableMol);
    ajTableDel(&seqTableMolEmbl);
    ajTableDel(&seqTableMolDdbj);
    ajTableDel(&seqTableMolGb);

    ajTableDel(&seqTableDiv);
    ajTableDel(&seqTableDivEmbl);
    ajTableDel(&seqTableDivDdbj);
    ajTableDel(&seqTableDivGb);

    ajTableDel(&seqTableCls);
    ajTableDel(&seqTableClsEmbl);
    ajTableDel(&seqTableClsDdbj);
    ajTableDel(&seqTableClsGb);

    return;
}




/* @section debug *************************************************************
**
** Reports sequence contents for debugging purposes
**
** @fdata [AjPSeq]
** @fcategory misc
**
** @nam3rule Trace    Print report to debug file (if any)
** @nam4rule TraceTitle  Print report to debug file (if any) with title
**
** @argrule * seq [const AjPSeq]
** @argrule Title title [const char*]
**
** @valrule * [void]
******************************************************************************/




/* @func ajSeqTrace ***********************************************************
**
** Debug calls to trace the data in a sequence object.
**
** @param [r] seq [const AjPSeq] Sequence.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajSeqTrace(const AjPSeq seq)
{
    AjIList it;
    AjPStr cur;
    ajint i;

    ajDebug("Sequence trace\n");
    ajDebug( "==============\n\n");
    ajDebug( "  Name: '%S'\n", seq->Name);

    if(ajStrGetLen(seq->Acc))
	ajDebug( "  Accession: '%S'\n", seq->Acc);

    if(ajListGetLength(seq->Acclist))
    {
	ajDebug( "  Acclist: (%Lu) ", ajListGetLength(seq->Acclist));
	it = ajListIterNewread(seq->Acclist);

	while((cur = (AjPStr) ajListIterGet(it)))
	    ajDebug(" %S", cur);

	ajListIterDel(&it);
	ajDebug(" \n");
    }

    if(ajStrGetLen(seq->Sv))
	ajDebug( "  SeqVersion: '%S'\n", seq->Sv);

    if(ajStrGetLen(seq->Gi))
	ajDebug( "  GenInfo Id: '%S'\n", seq->Gi);

    if(ajStrGetLen(seq->Type))
	ajDebug( "  Type: '%S' (%d)\n", seq->Type, seq->EType);

    if(ajStrGetLen(seq->Desc))
	ajDebug( "  Description: '%S'\n", seq->Desc);

    if(ajStrGetLen(seq->Tax))
	ajDebug( "  Taxonomy: '%S'\n", seq->Tax);

    if(ajStrGetLen(seq->Taxcommon))
	ajDebug( "  Taxcommon: '%S'\n", seq->Taxcommon);

    if(ajStrGetLen(seq->Taxid))
	ajDebug( "  Taxid: '%S'\n", seq->Taxid);

    if(ajStrGetLen(seq->Organelle))
	ajDebug( "  Organelle: '%S'\n", seq->Organelle);

    if(ajListGetLength(seq->Taxlist))
    {
	ajDebug( "  Taxlist: (%Lu)", ajListGetLength(seq->Taxlist));
	it = ajListIterNewread(seq->Taxlist);

	while((cur = (AjPStr) ajListIterGet(it)))
	    ajDebug(" '%S'", cur);

	ajListIterDel(&it);
	ajDebug("\n");
    }

    if(ajListGetLength(seq->Keylist))
    {
	ajDebug( "  Keywordlist: (%Lu)", ajListGetLength(seq->Keylist));
	it = ajListIterNewread(seq->Keylist);

	while((cur = (AjPStr) ajListIterGet(it)))
	    ajDebug(" '%S'", cur);

	ajListIterDel(&it);
	ajDebug("\n");
    }

    if(ajSeqGetLen(seq))
	ajDebug( "  Length: %d\n", ajSeqGetLen(seq));

    i = ajSeqCountGaps(seq);

    if(i)
	ajDebug( "  Gap count: %d\n", i);

    if(seq->Rev)
	ajDebug( "     Rev: %B\n", seq->Rev);

    if(seq->Reversed)
	ajDebug( "Reversed: %B\n", seq->Reversed);

    if(seq->Begin)
	ajDebug( "   Begin: %d\n", ajSeqGetBegin(seq));

    if(seq->End)
	ajDebug( "     End: %d\n", ajSeqGetEnd(seq));

    if(seq->Offset)
	ajDebug( "  Offset: %d\n", seq->Offset);

    if(seq->Offend)
	ajDebug( "  Offend: %d\n", seq->Offend);

    if(ajStrGetRes(seq->Seq))
	ajDebug( "  Reserved: %d\n", ajStrGetRes(seq->Seq));

    if(ajStrGetLen(seq->Db))
	ajDebug( "  Database: '%S'\n", seq->Db);

    if(ajStrGetLen(seq->Full))
	ajDebug( "  Full name: '%S'\n", seq->Full);

    if(ajStrGetLen(seq->Usa))
	ajDebug( "  Usa: '%S'\n", seq->Usa);

    if(ajStrGetLen(seq->Ufo))
	ajDebug( "  Ufo: '%S'\n", seq->Ufo);

    if(seq->Fttable)
	ajDebug( "  Fttable: exists\n");

    if(ajStrGetLen(seq->Formatstr))
	ajDebug( "  Input format: '%S' (%d)\n", seq->Formatstr, seq->Format);

    if(ajStrGetLen(seq->Filename))
	ajDebug( "  Filename: '%S'\n", seq->Filename);

    if(ajStrGetLen(seq->Entryname))
	ajDebug( "  Entryname: '%S'\n", seq->Entryname);

    if(!E_FPZERO(seq->Weight,U_FEPS))
	ajDebug( "  Weight: %.3f\n", seq->Weight);

    if(ajStrGetLen(seq->Doc))
	ajDebug( "  Documentation:...\n%S\n", seq->Doc);

	ajDebug( "Sequence:...\n%S\n", seq->Seq);
    ajDebug( "\n");

    return;
}




/* @func ajSeqTraceTitle ******************************************************
**
** Reports an AjPSeq object to debug output
**
** @param [r] seq [const AjPSeq] alignment object
** @param [r] title [const char*] Trace report title
** @return [void]
**
** @release 4.1.0
******************************************************************************/

void ajSeqTraceTitle(const AjPSeq seq, const char* title)
{
    ajDebug("\n%s\n",title);
    ajSeqTrace(seq);

    return;
}




/* @datasection [AjPSeqall] Sequence stream************************************
**
** Function is for manipulating sequence stream objects
**
** @nam2rule Seqall
**
******************************************************************************/




/* @section Sequence Stream Constructors **************************************
**
** @fdata [AjPSeqall]
**
** All constructors return a new sequence stream object by pointer. It
** is the responsibility of the user to first destroy any previous
** sequence stream object. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so
** anyway.
**
** @nam3rule New Constructor
**
** @valrule * [AjPSeqall]
** @fcategory new
**
******************************************************************************/




/* @func ajSeqallNew **********************************************************
**
** Creates a new sequence stream object to hold one sequence at a time.
**
** @return [AjPSeqall] New sequence stream object.
**
** @release 1.0.0
** @@
******************************************************************************/

AjPSeqall ajSeqallNew(void)
{
    AjPSeqall pthis;

    AJNEW0(pthis);

    pthis->Seq   = ajSeqNew();
    pthis->Seqin = ajSeqinNew();
    pthis->Count = 0;
    pthis->Totseqs = 0;
    pthis->Totlength = 0;

    return pthis;
}




/* @section  destructors ***************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the sequence.
**
** @fdata [AjPSeqall]
**
** @nam3rule Del Destructor
**
** @argrule Del Pseq [AjPSeqall*] Sequence stream object
** @valrule * [void]
** @fcategory delete
**
******************************************************************************/




/* @func ajSeqallDel **********************************************************
**
** Destructor for sequence stream objects
**
** @param [d] Pseq [AjPSeqall*] Sequence stream object reference
** @return [void]
**
** @release 2.0.0
** @@
******************************************************************************/

void ajSeqallDel(AjPSeqall *Pseq)
{
    if(!*Pseq)
	return;

    if(!(*Pseq)->Returned)
	ajSeqDel(&(*Pseq)->Seq);

    ajDebug("ajSeqallDel\n");
    ajSeqinDel(&(*Pseq)->Seqin);

    AJFREE(*Pseq);

    return;
}




/* @section modifiers *****************************************
**
** These functions use the contents of a sequence stream object and
** update them.
**
** @fdata [AjPSeqall]
** @fcategory modify
**
** @nam3rule Clear       Clear all sequence reading internals except statistics
** @nam4rule ClearStats  Clear all contents and reset statistics
**
** @nam3rule Set      Set properties within sequence stream
** @nam4rule SetRange Set start and end position within sequence stream
** @nam5rule SetRangeRev Set start and end position and reverse direction
**                       of a sequence stream
**
** @argrule * seq [AjPSeqall] Sequence stream object
** @argrule Range pos1 [ajint] Start position
** @argrule Range pos2 [ajint] End  position
**
** @valrule * [void]
**
******************************************************************************/




/* @func ajSeqallClear ********************************************************
**
** Resets all data internals for a sequence stream object when
** input has been read, but leaves statistics for reporting.
**
** @param [u] seq [AjPSeqall] Sequence stream
** @return [void]
**
** @release 2.7.0
** @@
******************************************************************************/

void ajSeqallClear(AjPSeqall seq)
{
    if(!seq)
        return;

    ajSeqClear(seq->Seq);
    ajSeqinClear(seq->Seqin);
    seq->Count = 0;
    seq->Begin = 0;
    seq->End   = 0;
    seq->Rev   = ajFalse;
    seq->Returned = ajFalse;
 
    return;
}




/* @func ajSeqallClearStats ***************************************************
**
** Resets all data for a sequence stream object so that it can be reused,
** including statistics.
**
** @param [u] seq [AjPSeqall] Sequence stream
** @return [void]
**
** @release 6.1.0
** @@
******************************************************************************/

void ajSeqallClearStats(AjPSeqall seq)
{
    ajSeqallClear(seq);

    seq->Totlength = 0;
    seq->Totseqs = 0;
 
    return;
}




/* @func ajSeqallSetRange *****************************************************
**
** Sets the start and end positions for a sequence stream.
**
** @param [u] seq [AjPSeqall] Sequence stream object to be set.
** @param [r] pos1 [ajint] Start position. Negative values are from the end.
** @param [r] pos2 [ajint] End position. Negative values are from the end.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajSeqallSetRange(AjPSeqall seq, ajint pos1, ajint pos2)
{
    ajDebug("ajSeqallSetRange (len: %d %d, %d)\n",
	    ajSeqGetLen(seq->Seq), pos1, pos2);

    if(pos1)
	seq->Begin = seq->Seq->Begin = pos1;

    if(pos2)
	seq->End = seq->Seq->End = pos2;

    ajDebug("      result: (len: %d %d, %d)\n",
	    ajSeqGetLen(seq->Seq), seq->Begin, seq->End);

    return;
}




/* @func ajSeqallSetRangeRev **************************************************
**
** Sets the start and end positions for a sequence stream,
** and set the sequences to be reversed.
**
** @param [u] seq [AjPSeqall] Sequence stream object to be set.
** @param [r] pos1 [ajint] Start position. Negative values are from the end.
** @param [r] pos2 [ajint] End position. Negative values are from the end.
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajSeqallSetRangeRev(AjPSeqall seq, ajint pos1, ajint pos2)
{
    ajDebug("ajSeqallSetRange (len: %d %d, %d)\n",
	    ajSeqGetLen(seq->Seq), pos1, pos2);

    if(pos1)
	seq->Begin = seq->Seq->Begin = pos1;

    if(pos2)
	seq->End = seq->Seq->End = pos2;

    ajDebug("      result: (len: %d %d, %d)\n",
	    ajSeqGetLen(seq->Seq), seq->Begin, seq->End);

    if(!seq->Rev)
    {
	seq->Rev = ajTrue;
    }

    return;
}




/* @section casts *********************************************
**
** These functions examine the contents of a sequence stream object
** and return some derived information. Some of them provide access to
** the internal components of a sequence stream object. They are
** provided for programming convenience but should be used with
** caution.
**
** @fdata [AjPSeqall]
** @fcategory cast
**
** @nam3rule Get Return properties of sequence stream
** @nam4rule GetCount Return number of sequences read
** @nam4rule GetFilename Return filename used for stream
** @nam4rule GetName Return name of sequence stream
** @nam4rule GetTotlength Return total length of sequences read
** @nam4rule GetUsa Return USA of sequence stream
**
** @nam3rule Getseq Return properties of current sequence from stream
** @nam4rule GetseqBegin Return begin position set for stream
** @nam4rule GetseqEnd Return begin position set for stream
** @nam4rule GetseqLen Return length of current sequence from stream
** @nam4rule GetseqName Return name of current sequence from stream
** @nam4rule GetseqRange Return begin and end of sequence from stream
**
** @argrule * seq [const AjPSeqall] Sequence stream object
** @argrule Range begin [ajint*] Returns begin position of range
** @argrule Range end [ajint*] Returns end position of range
**
** @valrule Begin [ajint] Begin position
** @valrule Count [ajlong] Count of sequences read
** @valrule End [ajint] End position
** @valrule Filename [const AjPStr] Filename
** @valrule Name [const AjPStr] Sequence name
** @valrule Len [ajint] Sequence length
** @valrule Range [ajint] Sequence length
** @valrule Totlength [ajlong] Total length of sequences read
** @valrule Usa [const AjPStr] Sequence USA
**
******************************************************************************/




/* @func ajSeqallGetCount *****************************************************
**
** Returns the number of sequences read by a seqall object
**
** @param [r] seq [const AjPSeqall] Seqall object pointer.
** @return [ajlong] Number of sequences read.
**
** @release 6.1.0
** @@
******************************************************************************/

ajlong ajSeqallGetCount(const AjPSeqall seq)
{
    if(!seq)
	return 0;

    if(!seq->Seqin)
	return 0;

    return seq->Totseqs;
}




/* @func ajSeqallGetFilename **************************************************
**
** Returns the filename of a seqall object.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeqall] Seqall object pointer.
** @return [const AjPStr] Name as a string.
**
** @release 4.0.0
** @@
******************************************************************************/

const AjPStr ajSeqallGetFilename(const AjPSeqall seq)
{
    if(!seq)
	return NULL;

    if(!seq->Seqin)
	return NULL;

    ajDebug("ajSeqallGetFilename '%S' usa: '%S'\n",
	    seq->Seqin->Name, seq->Seqin->Input->Qry);



    if(ajStrGetLen(seq->Seqin->Input->Filename))
	return seq->Seqin->Input->Filename;

    return NULL;
}




/* @func ajSeqallGetName ******************************************************
**
** Returns the sequence name of a sequence stream.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeqall] Sequence stream object.
** @return [const AjPStr] Name as a string.
**
** @release 1.0.0
** @@
******************************************************************************/

const AjPStr ajSeqallGetName(const AjPSeqall seq)
{
    ajDebug("ajSeqallGetName '%S'\n", seq->Seqin->Name);

    return seq->Seqin->Name;
}




/* @func ajSeqallGetTotlength *************************************************
**
** Returns the total length of sequences read by a seqall object
**
** @param [r] seq [const AjPSeqall] Seqall object pointer.
** @return [ajlong] Total length of sequences read.
**
** @release 6.1.0
** @@
******************************************************************************/

ajlong ajSeqallGetTotlength(const AjPSeqall seq)
{
    if(!seq)
	return 0;

    return seq->Totlength;
}




/* @func ajSeqallGetUsa *******************************************************
**
** Returns the sequence name of a sequence stream.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeqall] Sequence object.
** @return [const AjPStr] Name as a string.
**
** @release 1.8.0
** @@
******************************************************************************/

const AjPStr ajSeqallGetUsa(const AjPSeqall seq)
{
    ajDebug("ajSeqallGetUsa '%S'\n", seq->Seqin->Input->Qry);

    return seq->Seqin->Input->Qry;
}




/* @func ajSeqallGetseqBegin **************************************************
**
** Returns the sequence stream start position, or 1 if no start has been set.
**
** @param [r] seq [const AjPSeqall] Sequence stream object
** @return [ajint] Start position.
**
** @release 4.1.0
** @@
******************************************************************************/

ajint ajSeqallGetseqBegin(const AjPSeqall seq)
{
    ajint jbegin;

    if (seq->Begin)
    {
	jbegin = seq->Begin;

	if(jbegin > 0)
	    jbegin--;

	return 1 + ajCvtSposToPosStart(ajSeqGetLen(seq->Seq), 0, jbegin);
    }

    if(seq->Seq->Begin)
    {
	jbegin = seq->Seq->Begin;

	if(jbegin > 0)
	    jbegin--;

	return 1 + ajCvtSposToPosStart(ajSeqGetLen(seq->Seq), 0, jbegin);
    }

    return 1;
}




/* @func ajSeqallGetseqEnd ****************************************************
**
** Returns the sequence stream end position, or the sequence length if no end
** has been set.
**
** @param [r] seq [const AjPSeqall] Sequence stream object
** @return [ajint] Start position.
**
** @release 4.1.0
** @@
******************************************************************************/

ajint ajSeqallGetseqEnd(const AjPSeqall seq)
{
    ajint jend;

    if (seq->End)
    {
	jend = seq->End;

	if(jend > 0)
	    jend--;

	return 1 + ajCvtSposToPosStart(ajSeqGetLen(seq->Seq),
                                       ajSeqallGetseqBegin(seq)-1, jend);
    }

    if(seq->Seq->End)
    {
	jend = seq->Seq->End;

	if(jend > 0)
	    jend--;

	return 1 + ajCvtSposToPosStart(ajSeqGetLen(seq->Seq),
                                       ajSeqallGetseqBegin(seq)-1, jend);
    }

    return ajSeqGetLen(seq->Seq);
}




/* @func ajSeqallGetseqLen ****************************************************
**
** Returns the length of a sequence stream, which is the length of the
** latest sequence read.
**
** @param [r] seq [const AjPSeqall] Sequence stream object
** @return [ajint] sequence length.
**
** @release 4.1.0
** @@
******************************************************************************/

ajint ajSeqallGetseqLen(const AjPSeqall seq)
{
    return ajSeqGetLen(seq->Seq);
}




/* @func ajSeqallGetseqName ***************************************************
**
** Returns the sequence name of a sequence stream.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeqall] Sequence stream object.
** @return [const AjPStr] Name as a string.
**
** @release 4.1.0
** @@
******************************************************************************/

const AjPStr ajSeqallGetseqName(const AjPSeqall seq)
{
    if(!seq)
        return ajStrConstEmpty();
    
    ajDebug("ajSeqallGetseqName '%S'\n", seq->Seq->Name);

    return ajSeqGetNameS(seq->Seq);
}




/* @func ajSeqallGetseqRange **************************************************
**
** Returns the sequence range for a sequence stream
**
** @param [r] seq [const AjPSeqall] Sequence stream object.
** @param [w] begin [ajint*] Sequence range begin
** @param [w] end [ajint*] Sequence range end
** @return [ajint] Sequence range length
**
** @release 4.1.0
** @@
******************************************************************************/

ajint ajSeqallGetseqRange(const AjPSeqall seq, ajint* begin, ajint* end)
{
    ajDebug("ajSeqallGetRange '%S'\n", seq->Seq->Name);

    return ajSeqGetRange(seq->Seq, begin, end);
}




/* @datasection [AjPSeqset] Sequence set **************************************
**
** Function is for manipulating sequence set objects
**
** @nam2rule Seqset
**
******************************************************************************/




/* @section constructors *****************************************
**
** All constructors return a new sequence set object by pointer. It is the
** responsibility of the user to first destroy any previous
** sequence. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [AjPSeqset]
** @fcategory new
**
** @nam3rule New Constructor
**
** @valrule * [AjPSeqset]
**
******************************************************************************/




/* @func ajSeqsetNew **********************************************************
**
** Creates a new sequence set object to hold all sequences in memory.
**
** @return [AjPSeqset] New sequence set object.
**
** @release 1.0.0
** @@
******************************************************************************/

AjPSeqset ajSeqsetNew(void)
{
    AjPSeqset pthis;

    AJNEW0(pthis);

    pthis->Size      = 0;
    pthis->Len       = 0;
    pthis->Begin     = 0;
    pthis->End       = 0;
    pthis->Totweight = 0.0;

    pthis->Name = ajStrNew();
    pthis->Type = ajStrNew();
    pthis->Full = ajStrNew();
    pthis->Usa  = ajStrNew();
    pthis->Ufo  = ajStrNew();

    pthis->Formatstr = ajStrNew();
    pthis->Filename  = ajStrNew();

    pthis->Seq       = NULL;
    pthis->Seqweight = NULL;

    pthis->EType  = 0;
    pthis->Format = 0;

    return pthis;
}




/* @section  destructors ***************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the sequence.
**
** @fdata [AjPSeqset]
**
** @nam3rule Del Destructor
** @nam3rule Delarray Array destructor
**
** @argrule Del Pseq [AjPSeqset*] Sequence set object
** @argrule Delarray PPseq [AjPSeqset**] Sequence set object array
** @valrule * [void]
** @fcategory delete
**
******************************************************************************/




/* @func ajSeqsetDel **********************************************************
**
** Destructor for sequence set objects
**
** @param [d] Pseq [AjPSeqset*] Sequence set object reference
** @return [void]
**
** @release 2.1.0
** @@
******************************************************************************/

void ajSeqsetDel(AjPSeqset *Pseq)
{
    ajuint i;
    AjPSeqset seq = NULL;

    if(!Pseq || !*Pseq)
	return;

    seq = *Pseq;

    ajDebug("ajSeqsetDel size: %d\n", seq->Size);

    ajStrDel(&seq->Type);
    ajStrDel(&seq->Formatstr);
    ajStrDel(&seq->Filename);
    ajStrDel(&seq->Full);
    ajStrDel(&seq->Name);
    ajStrDel(&seq->Usa);
    ajStrDel(&seq->Ufo);

    for(i=0; i<seq->Size; ++i)
	ajSeqDel(&seq->Seq[i]);

    AJFREE(seq->Seq);
    AJFREE(seq->Seqweight);

    AJFREE(*Pseq);

    return;
}




/* @func ajSeqsetDelarray *****************************************************
**
** Destructor for array of sequence set objects
**
** @param [d] PPseq [AjPSeqset**] Sequence set object array
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajSeqsetDelarray(AjPSeqset **PPseq)
{
    ajuint i = 0;

    if(!PPseq || !*PPseq)
	return;

    while((*PPseq)[i])
    {
	ajSeqsetDel(&(*PPseq)[i]);
	i++;
    }

    ajDebug("ajSeqsetDelarray size: %d\n", i);

    AJFREE(*PPseq);

    return;
}




/* @section casts *************************************************************
**
** These functions examine the contents of a sequence set object
** and return some derived information. Some of them provide access to
** the internal components of a sequence stream object. They are
** provided for programming convenience but should be used with
** caution.
**
** @fdata [AjPSeqset]
** @fcategory cast
**
** @nam3rule Get         Return element value
** @nam4rule GetFilename Return filename
** @nam4rule GetFormat   Return input sequence format
** @nam4rule GetOffend   Return end offset
** @nam4rule GetOffset   Return start offset
** @nam4rule GetRange    Return start and end
** @nam4rule GetTotlength Return total sequence length
** @nam4rule GetUsa      Return input USA
**
** @argrule * seq [const AjPSeqset] Sequence object
** @argrule GetRange begin [ajint*] Sequence start position
** @argrule GetRange end   [ajint*] Sequence end position
**
** @valrule GetFilename [const AjPStr] Filename
** @valrule GetFormat [const AjPStr] Input sequence format
** @valrule GetOffend [ajint] End offset
** @valrule GetOffset [ajint] Start offset
** @valrule GetRange [ajint] Sequence length
** @valrule GetUsa [const AjPStr] Input sequence USA
** @valrule GetTotlength [ajint] Total ungapped sequence length
**
******************************************************************************/




/* @func ajSeqsetGetFilename **************************************************
**
** Returns the filename of a sequence set.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeqset] Sequence set object.
** @return [const AjPStr] Name as a string.
**
** @release 4.0.0
** @@
******************************************************************************/

const AjPStr ajSeqsetGetFilename(const AjPSeqset seq)
{
    ajDebug("ajSeqsetGetFilename '%S' usa: '%S'\n", seq->Name, seq->Usa);

    if(!seq)
	return NULL;

    if(ajStrGetLen(seq->Filename))
	return seq->Filename;

    return NULL;
}




/* @func ajSeqsetGetFormat ****************************************************
**
** Returns the sequence format for a sequence set
**
** @param [r] seq [const AjPSeqset] Sequence set object.
** @return [const AjPStr] Sequence format
**
** @release 2.0.1
** @@
******************************************************************************/

const AjPStr ajSeqsetGetFormat(const AjPSeqset seq)
{
    return seq->Formatstr;
}




/* @func ajSeqsetGetOffend ****************************************************
**
** Returns the sequence set offend value.
** This is the number of positions removed from the original end.
**
** @param [r] seq [const AjPSeqset] Sequence set object
** @return [ajint] Sequence offend.
**
** @release 4.1.0
** @@
******************************************************************************/

ajint ajSeqsetGetOffend(const AjPSeqset seq)
{
    return seq->Offend;
}




/* @func ajSeqsetGetOffset ****************************************************
**
** Returns the sequence set offset value.
** This is the number of positions removed from the original end.
**
** @param [r] seq [const AjPSeqset] Sequence set object
** @return [ajint] Sequence offset.
**
** @release 4.1.0
** @@
******************************************************************************/

ajint ajSeqsetGetOffset(const AjPSeqset seq)
{
    return seq->Offset;
}




/* @func ajSeqsetGetRange *****************************************************
**
** Returns the sequence range for a sequence set
**
** @param [r] seq [const AjPSeqset] Sequence set object.
** @param [w] begin [ajint*] Sequence range begin
** @param [w] end [ajint*] Sequence range end
** @return [ajint] Sequence range length
**
** @release 1.0.0
** @@
******************************************************************************/

ajint ajSeqsetGetRange(const AjPSeqset seq, ajint* begin, ajint* end)
{
    ajint jbegin;
    ajint jend;

    jbegin = seq->Begin;

    if(jbegin > 0)
	jbegin--;

    jend = seq->End;

    if(jend > 0)
	jend--;

    ajDebug("ajSeqsetGetRange '%S' begin %d end %d len: %d\n",
	    seq->Name, seq->Begin, seq->End, seq->Len);
    *begin = ajCvtSposToPosStart(seq->Len, 0, jbegin);

    if(seq->End)
	*end = 1 + ajCvtSposToPosStart(seq->Len, *begin, jend);
    else
	*end = 1 + ajCvtSposToPosStart(seq->Len, *begin, seq->Len);

    (*begin)++;

    return (*end - *begin + 1);
}




/* @func ajSeqsetGetTotlength *************************************************
**
** Returns the total number of bases or residues in a sequence set,
** ignoring any gap characters in aligned sequences.
**
** @param [r] seq [const AjPSeqset] Sequence set object.
** @return [ajint] Total ungapped length
**
** @release 6.1.0
** @@
******************************************************************************/

ajint ajSeqsetGetTotlength(const AjPSeqset seq)
{
    ajuint n;
    ajuint i;
    ajint ret = 0;

    n = seq->Size;

    for(i=0; i < n; i++)
        ret += ajSeqGetLenUngapped(seq->Seq[i]);
    
    return ret;
}




/* @func ajSeqsetGetUsa *******************************************************
**
** Returns the sequence name of a sequence set.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeqset] Sequence set object.
** @return [const AjPStr] Name as a string.
**
** @release 2.1.0
** @@
******************************************************************************/

const AjPStr ajSeqsetGetUsa(const AjPSeqset seq)
{
    ajDebug("ajSeqetGetUsa '%S'\n", seq->Usa);

    return seq->Usa;
}




/* @section modifiers ********************************************
**
** These functions use the contents of a sequence set object and
** update them.
**
**
** @fdata [AjPSeqset]
** @fcategory modify
**
** @nam3rule Clear Reset object
** @nam3rule Fill Fills shorter sequences with gaps at end
** @nam3rule Fmt Reformats sequence
** @nam4rule FmtLower reformats sequence to lower case
** @nam4rule FmtUpper reformats sequence to upper case
** @nam3rule Sort Reorders sequences
** @nam4rule SortLen Reorders sequences by length
**
** @nam3rule Reverse Reverse complements all sequences
** @nam3rule Trim    Trim sequences to defined range
**
** @argrule * seq [AjPSeqset] Sequence set
**
** @valrule * [void]
** @valrule *Fill [ajint] Maximum number of gaps inserted
**
******************************************************************************/




/* @func ajSeqsetClear ********************************************************
**
** Reset a sequence set object
**
** @param [u] seq [AjPSeqset] Sequence set object reference
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ajSeqsetClear(AjPSeqset seq)
{
    ajuint i;

    if(!seq)
	return;

    ajDebug("ajSeqsetDel size: %d\n", seq->Size);

    if(MAJSTRGETLEN(seq->Type))
       ajStrSetClear(&seq->Type);
    if(MAJSTRGETLEN(seq->Formatstr))
       ajStrSetClear(&seq->Formatstr);
    if(MAJSTRGETLEN(seq->Filename))
       ajStrSetClear(&seq->Filename);
    if(MAJSTRGETLEN(seq->Full))
       ajStrSetClear(&seq->Full);
    if(MAJSTRGETLEN(seq->Name))
       ajStrSetClear(&seq->Name);
    if(MAJSTRGETLEN(seq->Usa))
       ajStrSetClear(&seq->Usa);
    if(MAJSTRGETLEN(seq->Ufo))
       ajStrSetClear(&seq->Ufo);

    for(i=0; i<seq->Size; ++i)
	ajSeqDel(&seq->Seq[i]);

    AJFREE(seq->Seq);
    AJFREE(seq->Seqweight);

    seq->Size = 0;
    seq->Begin = 0;
    seq->End = 0;
    seq->Len = 0;
    seq->Offset = 0;
    seq->Offend = 0;

    seq->Rev = ajTrue;
    seq->Trimmed = ajTrue;

    return;
}




/* @func ajSeqsetFill *********************************************************
**
** Fills a sequence set with gaps at the ends of any shorter sequences.
**
** @param [u] seq [AjPSeqset] Sequence set object to be set.
** @return [ajint] Number of gaps inserted
**
** @release 1.0.0
** @@
******************************************************************************/

ajint ajSeqsetFill(AjPSeqset seq)
{
    ajuint i;
    ajuint ifix = 0;
    ajuint nfix = 0;
    ajuint ilen;

    ajDebug("ajSeqsetFill(len: %d)\n", seq->Len);

    for(i=0; i< seq->Size; i++)
	if(ajSeqGetLen(seq->Seq[i]) < seq->Len)
	{
	    nfix++;
	    ilen = seq->Len - ajSeqGetLen(seq->Seq[i]);

	    if(ilen > ifix)
		ifix = ilen;

	    ajStrAppendCountK(&seq->Seq[i]->Seq, '-', ilen);
	}

    ajDebug("      result: (len: %d added: %u number of seqs fixed: %u\n",
	    seq->Len, ifix, nfix);

    return ifix;
}




/* @func ajSeqsetFmtLower *****************************************************
**
** Converts all sequences in a set to lower case.
**
** @param [u] seq [AjPSeqset] Sequence set object
** @return [void]
**
** @release 4.1.0
** @@
******************************************************************************/

void ajSeqsetFmtLower(AjPSeqset seq)
{
    ajuint i;

    for(i=0; i < seq->Size; i++)
	ajSeqFmtLower(seq->Seq[i]);

    return;
}




/* @func ajSeqsetFmtUpper *****************************************************
**
** Converts all sequences in a set to upper case.
**
** @param [u] seq [AjPSeqset] Sequence set object
** @return [void]
**
** @release 4.1.0
** @@
******************************************************************************/

void ajSeqsetFmtUpper(AjPSeqset seq)
{
    ajuint i;

    for(i=0; i < seq->Size; i++)
	ajSeqFmtUpper(seq->Seq[i]);

    return;
}




/* @func ajSeqsetReverse ******************************************************
**
** Reverse complements all sequences in a sequence set.
**
** @param [u] seq [AjPSeqset] Sequence set object
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajSeqsetReverse(AjPSeqset seq)
{
    ajuint i;
    ajint ibegin;
    ajint iend;

    ajDebug("ajSeqsetReverse len: %d Begin: %d End: %d\n",
	    ajSeqsetGetLen(seq), seq->Begin, seq->End);

    ibegin = seq->Begin;
    iend   = seq->End;

    if(ibegin)
	seq->End = -(ibegin);

    if(iend)
	seq->Begin = -(iend);

    for(i=0; i < seq->Size; i++)
    {
        seq->Seq[i]->Rev = ajTrue;
	ajSeqReverseDo(seq->Seq[i]);
    }
    
    ajDebug("  set result len: %d Begin: %d End: %d\n",
	    ajSeqsetGetLen(seq), seq->Begin, seq->End);

    return;
}




/* @func ajSeqsetSortLen ******************************************************
**
** Sorts a sequence set by sequence length
**
** @param [u] seq [AjPSeqset] Sequence set object
** @return [void]
**
** @release 6.1.0
** @@
******************************************************************************/

void ajSeqsetSortLen(AjPSeqset seq)
{
    ajuint i;
    ajuint j;
    ajuint k;
    ajuint * lengths;
    ajuint * seqnum;
    AjPSeq tmp;

    AJCNEW0(lengths,seq->Size);
    AJCNEW0(seqnum,seq->Size);

    for(i=0;i<seq->Size;i++)
    {
        lengths[i] = ajSeqGetLenUngapped(seq->Seq[i]);
        seqnum[i] = i;
    }

    ajSortUintIncI(lengths,seqnum,seq->Size);

    j = 0;

    for(i=0;i<seq->Size;i++)
    {
        tmp = seq->Seq[i];

        for(k=i; seqnum[k] != i; k=seqnum[j], seqnum[j]=j)
        {
            j = k;
            seq->Seq[k] = seq->Seq[seqnum[k]];
        }

        seq->Seq[k] = tmp;
        seqnum[k] = k;
    }
    
    AJFREE(lengths);
    AJFREE(seqnum);

    return;
}




/* @func ajSeqsetTrim *********************************************************
**
** Trims a sequence set to start and end positions
**
** @param [u] seq [AjPSeqset] Sequence set object
** @return [void]
**
** @release 2.9.0
** @@
******************************************************************************/

void ajSeqsetTrim(AjPSeqset seq)
{
    ajuint i;

    ajint begin;
    ajint end;
    ajint jbegin;
    ajint jend;

    if(seq->Trimmed)
    {
	ajWarn("Sequence set '%S' already trimmed", ajSeqsetGetNameS(seq));

	return;
    }

    ajDebug("ajSeqsetTrim len: %d begin: %d end: %d\n",
	    seq->Len, seq->Begin, seq->End);

    for(i=0; i < seq->Size; i++)
	ajSeqTrim(seq->Seq[i]);

    jbegin = seq->Begin;

    if(jbegin > 0)
	jbegin--;

    jend = seq->End;

    if(jend > 0)
	jend--;

    begin = 1 + ajCvtSposToPosStart(seq->Len, 0, jbegin);
    end   = 1 + ajCvtSposToPosStart(seq->Len, begin-1, jend);

    if(seq->End)
    {
	if(end < begin)
	    return;

	seq->Offend = seq->Len - end;
	seq->End    = 0;
	seq->Len = end;
    }

    if(seq->Begin)
    {
	seq->Offset = begin-1;
	seq->Begin = 0;
	seq->Len -= begin;
    }

    ajDebug("ajSeqsetTrim result len: %d begin: %d end: %d\n",
	    seq->Len, seq->Begin, seq->End);

    return;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of a sequence set object.
**
** @fdata       [AjPSeqset]
** @fcategory modify
**
** @nam3rule Set Assigns value to an element
** @nam4rule SetRange Assigns begin and end values for whole set
**
** @argrule * seq [AjPSeqset] Sequence set
** @argrule Range pos1 [ajint] Start position
** @argrule Range pos2 [ajint] End position
**
** @valrule * [void]
******************************************************************************/




/* @func ajSeqsetSetRange *****************************************************
**
** Sets the start and end positions for a sequence set.
**
** @param [u] seq [AjPSeqset] Sequence set object to be set.
** @param [r] pos1 [ajint] Start position. Negative values are from the end.
** @param [r] pos2 [ajint] End position. Negative values are from the end.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajSeqsetSetRange(AjPSeqset seq, ajint pos1, ajint pos2)
{
    ajuint i;

    ajDebug("ajSeqsetSetRange(len: %d %d, %d)\n", seq->Len, pos1, pos2);

    if(pos1)
	seq->Begin = pos1;

    if(pos2)
	seq->End = pos2;

    for(i=0; i< seq->Size; i++)
    {
	if(pos1)
	    seq->Seq[i]->Begin = pos1;
	if(pos2)
	    seq->Seq[i]->End   = pos2;
    }

    ajDebug("      result: (len: %u %d, %d)\n",
	    seq->Len, seq->Begin, seq->End);

    return;
}




/* @section casts ************************************************
**
** These functions examine the contents of a sequence set object and
** return some derived information. Some of them provide access to the
** internal components of a sequence set object. They are provided for
** programming convenience but should be used with caution.
**
** @fdata [AjPSeqset]
** @fcategory cast
**
** @nam3rule Get          Return sequence set attribute(s)
** @nam4rule GetBegin     Return sequence set end
** @nam4rule GetEnd       Return sequence set end
** @nam4rule GetLen       Return sequence set length
** @nam4rule GetName      Return sequence set name
** @nam4rule GetSeqarray  Return array of sequence objects
** @nam4rule GetSize      Return sequence set size (number of sequences)
** @nam4rule GetTotweight  Return total weighting for sequence set
** @nam3rule Getseq       Return attribute(s) from one sequence
** @nam4rule GetseqAcc    Return sequence accession number
** @nam4rule GetseqName   Return sequence name
** @nam4rule GetseqSeq    Return sequence object
** @nam4rule GetseqWeight Return sequence weight
**
** @suffix S Return a string
** @suffix C Return a character string
**
** @argrule * seq [const AjPSeqset] Sequence set object
** @argrule Getseq i [ajuint] Number of sequence in set
**
** @valrule Begin    [ajuint]        Sequence set start
** @valrule End      [ajuint]        Sequence set end
** @valrule Len      [ajuint]        Sequence set length
** @valrule Seq      [const AjPSeq] Sequence object
** @valrule Seqarray [AjPSeq*]      Array of sequences, NULL terminated
** @valrule Size     [ajuint]        Number of sequences
** @valrule Totweight [float]       Sequence weight total
** @valrule Weight   [float]        Sequence weight total
** @valrule *C [const char*] Character string
** @valrule *S [const AjPStr] String object
**
******************************************************************************/




/* @func ajSeqsetGetBegin *****************************************************
**
** Returns the sequence set start position, or 1 if no start has been set.
**
** @param [r] seq [const AjPSeqset] Sequence set object
** @return [ajuint] Start position.
**
** @release 4.1.0
** @@
******************************************************************************/

ajuint ajSeqsetGetBegin(const AjPSeqset seq)
{
    ajint jbegin;

    if(!seq->Begin)
	return 1;

    jbegin = seq->Begin;

    if(jbegin > 0)
	jbegin--;

    return 1 + ajCvtSposToPosStart(seq->Len, 0, jbegin);
}




/* @func ajSeqsetGetEnd *******************************************************
**
** Returns the sequence set end position, or the sequence length if no end
** has been set.
**
** @param [r] seq [const AjPSeqset] Sequence set object
** @return [ajuint] Start position.
**
** @release 4.1.0
** @@
******************************************************************************/

ajuint ajSeqsetGetEnd(const AjPSeqset seq)
{
    ajint jend;

    if(!seq->End)
	return (seq->Len);

    jend = seq->End;

    if(jend > 0)
	jend--;

    return 1 + ajCvtSposToPosStart(seq->Len, ajSeqsetGetBegin(seq)-1, jend);
}




/* @func ajSeqsetGetLen *******************************************************
**
** Returns the length of a sequence set, which is the maximum sequence
** length in the set.
**
** @param [r] seq [const AjPSeqset] Sequence set object
** @return [ajuint] sequence set length.
**
** @release 4.1.0
** @@
******************************************************************************/

ajuint ajSeqsetGetLen(const AjPSeqset seq)
{
    return seq->Len;
}




/* @func ajSeqsetGetNameC *****************************************************
**
** Returns the sequence name of a sequence set.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeqset] Sequence set object.
** @return [const char*] Name as a C character string.
**
** @release 4.1.0
** @@
******************************************************************************/

const char* ajSeqsetGetNameC(const AjPSeqset seq)
{
    ajDebug("ajSeqsetGetName '%S' usa: '%S'\n", seq->Name, seq->Usa);

    if(ajStrGetLen(seq->Name))
      return MAJSTRGETPTR(seq->Name);

    return MAJSTRGETPTR(seq->Usa);
}




/* @func ajSeqsetGetNameS *****************************************************
**
** Returns the sequence name of a sequence set.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeqset] Sequence set object.
** @return [const AjPStr] Name as a string.
**
** @release 4.1.0
** @@
******************************************************************************/

const AjPStr ajSeqsetGetNameS(const AjPSeqset seq)
{
    if(!seq)
        return ajStrConstEmpty();

    ajDebug("ajSeqsetGetNameS '%S' usa: '%S'\n", seq->Name, seq->Usa);

    if(ajStrGetLen(seq->Name))
      return seq->Name;

    return seq->Usa;
}




/* @func ajSeqsetGetSeqarray **************************************************
**
** Returns an array of sequences.
** Because this is a pointer to the real internal sequence
** the caller must take care not to change the data in any way.
** If the sequence is to be changed (case for example) then it must first
** be copied.
**
** The array is 1 larger than the sequence set,
** with the last element set to NULL.
** @param [r] seq [const AjPSeqset] Sequence set object.
** @return [AjPSeq*] Sequence object.
**
** @release 4.1.0
** @@
******************************************************************************/

AjPSeq* ajSeqsetGetSeqarray(const AjPSeqset seq)
{
    AjPSeq* ret;
    ajuint i;

    ajDebug("ajSeqsetGetSeqArray '%S' %d\n", seq->Name, seq->Size);
    AJCNEW0(ret, (seq->Size+1));

    for (i=0; i<seq->Size;i++)
    {
	ret[i] = ajSeqNewSeq(seq->Seq[i]);
    }

    return ret;
}




/* @func ajSeqsetGetSize ******************************************************
**
** Returns the number of sequences in a sequence set
**
** @param [r] seq [const AjPSeqset] Sequence set object
** @return [ajuint] sequence set size.
**
** @release 4.1.0
** @@
******************************************************************************/

ajuint ajSeqsetGetSize(const AjPSeqset seq)
{
    return seq->Size;
}




/* @func ajSeqsetGetTotweight *************************************************
**
** Returns the weight of all sequences in a sequence set
**
** @param [r] seq [const AjPSeqset] Sequence set object
** @return [float] sequence weight as a float.
**
** @release 4.1.0
** @@
******************************************************************************/

float ajSeqsetGetTotweight(const AjPSeqset seq)
{
    ajuint i;
    float ret = 0.0;

    for(i=0; i < seq->Size; i++)
	ret += seq->Seq[i]->Weight;

    return ret;
}




/* @func ajSeqsetGetseqAccC ***************************************************
**
** Returns the accession number of a sequence in a sequence set
**
** @param [r] seq [const AjPSeqset] Sequence set object
** @param [r] i [ajuint] Sequence index
** @return [const char*] accession number as a string.
**
** @release 4.1.0
** @@
******************************************************************************/

const char* ajSeqsetGetseqAccC(const AjPSeqset seq, ajuint i)
{
    if(i >= seq->Size)
	return NULL;

    return MAJSTRGETPTR(seq->Seq[i]->Acc);
}




/* @func ajSeqsetGetseqAccS ***************************************************
**
** Returns the accession number of a sequence in a sequence set
**
** @param [r] seq [const AjPSeqset] Sequence set object
** @param [r] i [ajuint] Sequence index
** @return [const AjPStr] accession number as a string.
**
** @release 4.1.0
** @@
******************************************************************************/

const AjPStr ajSeqsetGetseqAccS(const AjPSeqset seq, ajuint i)
{
    if(i >= seq->Size)
	return NULL;

    if(seq->Seq[i]->Acc)
        return seq->Seq[i]->Acc;
    return ajStrConstEmpty();
}




/* @func ajSeqsetGetseqNameC **************************************************
**
** Returns the name of a sequence in a sequence set
**
** @param [r] seq [const AjPSeqset] Sequence set object
** @param [r] i [ajuint] Sequence index
** @return [const char*] sequence name as a string.
**
** @release 4.1.0
** @@
******************************************************************************/

const char* ajSeqsetGetseqNameC(const AjPSeqset seq, ajuint i)
{
    if(!seq)
        return "";

    if(i >= seq->Size)
	return "";

    if(seq->Seq[i]->Name)
        return MAJSTRGETPTR(seq->Seq[i]->Name);

    return "";
}




/* @func ajSeqsetGetseqNameS **************************************************
**
** Returns the name of a sequence in a sequence set
**
** @param [r] seq [const AjPSeqset] Sequence set object
** @param [r] i [ajuint] Sequence index
** @return [const AjPStr] sequence name as a string.
**
** @release 4.1.0
** @@
******************************************************************************/

const AjPStr ajSeqsetGetseqNameS(const AjPSeqset seq, ajuint i)
{
    if(!seq)
        return ajStrConstEmpty();

    if(i >= seq->Size)
	return ajStrConstEmpty();

    if(seq->Seq[i]->Name)
        return seq->Seq[i]->Name;

    return ajStrConstEmpty();
}




/* @func ajSeqsetGetseqSeq ****************************************************
**
** Returns one sequence from a sequence set.
** Because this is a pointer to the real internal sequence
** the caller must take care not to change the data in any way.
** If the sequence is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeqset] Sequence set object.
** @param [r] i [ajuint] Sequence index number in set
** @return [const AjPSeq] Sequence object.
**
** @release 4.1.0
** @@
******************************************************************************/

const AjPSeq ajSeqsetGetseqSeq(const AjPSeqset seq, ajuint i)
{
    if(i >= seq->Size)
	return NULL;

    return seq->Seq[i];
}




/* @func ajSeqsetGetseqSeqC ***************************************************
**
** Returns one sequence from a sequence set.
** Because this is a pointer to the real internal sequence
** the caller must take care not to change the data in any way.
** If the sequence is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeqset] Sequence set object.
** @param [r] i [ajuint] Sequence index number in set
** @return [const char*] Sequence as a C string.
**
** @release 4.1.0
** @@
******************************************************************************/

const char* ajSeqsetGetseqSeqC(const AjPSeqset seq, ajuint i)
{
    if(i >= seq->Size)
	return NULL;

    return MAJSTRGETPTR(seq->Seq[i]->Seq);
}




/* @func ajSeqsetGetseqSeqS ***************************************************
**
** Returns one sequence from a sequence set.
** Because this is a pointer to the real internal sequence
** the caller must take care not to change the data in any way.
** If the sequence is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeqset] Sequence set object.
** @param [r] i [ajuint] Sequence index number in set
** @return [const AjPStr] Sequence object.
**
** @release 4.1.0
** @@
******************************************************************************/

const AjPStr ajSeqsetGetseqSeqS(const AjPSeqset seq, ajuint i)
{
    ajDebug("ajSeqsetGetseqSeq '%S' %d/%d\n", seq->Name, i, seq->Size);

    if(i >= seq->Size)
	return NULL;

    if(seq->Seq[i]->Seq)
        return seq->Seq[i]->Seq;
    return ajStrConstEmpty();
}




/* @func ajSeqsetGetseqWeight *************************************************
**
** Returns the weight of a sequence in a sequence set
**
** @param [r] seq [const AjPSeqset] Sequence set object
** @param [r] i [ajuint] Sequence index
** @return [float] sequence weight as a float.
**
** @release 4.1.0
** @@
******************************************************************************/

float ajSeqsetGetseqWeight(const AjPSeqset seq, ajuint i)
{
    if(i >= seq->Size)
	return 0.0;

    return seq->Seq[i]->Weight;
}




/* @section testing properties ************************************************
**
** Tests properties of a sequence set
**
** @fdata [AjPSeqset]
** @fcategory cast
**
** @nam3rule Is Test sequence property
** @nam4rule IsDna Sequence is DNA
** @nam4rule IsNuc Sequence is nucleotide
** @nam4rule IsProt Sequence is protein
** @nam4rule IsRna Sequence is RNA
**
** @argrule * seq [const AjPSeqset] Sequence set object
**
** @valrule Is [AjBool] True or false
**
******************************************************************************/




/* @func ajSeqsetIsDna ********************************************************
**
** Tests whether a sequence set is DNA.
**
** @param [r] seq [const AjPSeqset] Sequence set
** @return [AjBool] ajTrue for a nucleotide sequence set.
**
** @release 2.3.0
** @@
******************************************************************************/

AjBool ajSeqsetIsDna(const AjPSeqset seq)
{
    AjPSeq myseq;

    if(ajStrMatchC(seq->Type, "P"))
	return ajFalse;

    myseq = seq->Seq[0];

    if(ajSeqTypeGapdnaS(myseq->Seq))
    	return ajFalse;

    return ajTrue;
}




/* @func ajSeqsetIsNuc ********************************************************
**
** Tests whether a sequence set is nucleotide.
**
** @param [r] seq [const AjPSeqset] Sequence set
** @return [AjBool] ajTrue for a nucleotide sequence set.
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajSeqsetIsNuc(const AjPSeqset seq)
{
    AjPSeq myseq;

    if(ajStrMatchC(seq->Type, "N"))
	return ajTrue;

    myseq = seq->Seq[0];

    if(ajSeqTypeGapnucS(myseq->Seq))
    	return ajFalse;

    return ajTrue;
}




/* @func ajSeqsetIsProt *******************************************************
**
** Tests whether a sequence set is protein.
**
** @param [r] seq [const AjPSeqset] Sequence set
** @return [AjBool] ajTrue for a protein sequence set.
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajSeqsetIsProt(const AjPSeqset seq)
{
    AjPSeq myseq;

    if(ajStrMatchC(seq->Type, "P"))
	return ajTrue;

    if(ajSeqsetIsNuc(seq))
	return ajFalse;

    myseq = seq->Seq[0];

    return ajSeqIsProt(myseq);
}




/* @func ajSeqsetIsRna ********************************************************
**
** Tests whether a sequence set is RNA.
**
** @param [r] seq [const AjPSeqset] Sequence set
** @return [AjBool] ajTrue for a nucleotide sequence set.
**
** @release 2.0.1
** @@
******************************************************************************/

AjBool ajSeqsetIsRna(const AjPSeqset seq)
{
    AjPSeq myseq;

    if(ajStrMatchC(seq->Type, "P"))
	return ajFalse;

    myseq = seq->Seq[0];

    if(ajSeqTypeGaprnaS(myseq->Seq))
    	return ajFalse;

    return ajTrue;
}




/* @datasection [AjPStr] sequence strings *************************************
**
** Sequences represented as string objects
**
** @nam2rule Seqstr
**
******************************************************************************/




/* @section conversion *******************************************************
**
** Convert sequence to numbers for efficient processing
**
** @fdata [AjPStr]
** @fcategory derive
**
** @nam3rule Convert Convert sequence to some other datatype
** @nam4rule ConvertNum Convert to integers
**
** @argrule * seq [const AjPStr]
** @argrule Num cvt [const AjPSeqCvt] Conversion table
** @argrule Num Pnumseq [AjPStr*] Output numeric version of the sequence
**
** @valrule * [AjBool] True on success
******************************************************************************/




/* @func ajSeqstrConvertNum ***************************************************
**
** Converts a string of sequence characters to numbers using
** a conversion table.
**
** @param [r] seq [const AjPStr] Sequence as a string
** @param [r] cvt [const AjPSeqCvt] Conversion table.
** @param [w] Pnumseq [AjPStr*] Output numeric version of the sequence.
** @return [AjBool] ajTrue on success.
**
** @release 4.1.0
** @@
******************************************************************************/

AjBool ajSeqstrConvertNum(const AjPStr seq, const AjPSeqCvt cvt,
			  AjPStr* Pnumseq)
{
    const char *cp;
    char *ncp;

    cp = ajStrGetPtr(seq);

    ajStrAssignS(Pnumseq, seq);
    ncp = ajStrGetuniquePtr(Pnumseq);

    while(*cp)
    {
	*ncp = cvt->table[(ajint)*cp];
	cp++;
	ncp++;
    }

    return ajTrue;
}




/* @datasection [AjPSeqCvt] sequence conversion *******************************
**
** Sequences represented as string objects
**
** @nam2rule Seqcvt
**
******************************************************************************/




/* @section Sequence Conversion Functions *************************************
**
** @fdata [AjPSeqCvt]
** @fcategory misc
**
** @nam3rule Trace Report contents to debug output
**
** @argrule * cvt [const AjPSeqCvt]
**
** @valrule * [void]
**
******************************************************************************/




/* @func ajSeqcvtTrace ********************************************************
**
** Traces a conversion table with debug calls.
**
** @param [r] cvt [const AjPSeqCvt] Conversion table.
** @return [void]
**
** @release 4.1.0
** @@
******************************************************************************/

void ajSeqcvtTrace(const AjPSeqCvt cvt)
{
    ajint i;

    ajDebug("Cvt table for '%S'\n\n", cvt->bases);
    ajDebug("index num ch\n");
    ajDebug("----- --- --\n");
    for(i=0; i < cvt->size; i++)
	if(cvt->table[i])
	    ajDebug("%5d %3d <%c>\n", i, cvt->table[i], ajSysCastItoc(i));

    ajDebug("... all others are zero ...\n", cvt->bases);

    return;
}




/* @section constructors *********************************************
**
** @fdata [AjPSeqCvt]
** @fcategory new
**
** @nam3rule New Constructor by default starting numbers from one,
**           with zero used for characters not in the set of bases
** @nam4rule NewEnd Constructor starting numbers from zero,
**           with the next number
**           used for characters not in the set of bases
** @nam4rule NewNumber Conversion table uses base letters and numbers
** @nam4rule NewStr Conversion table uses labels longer than one
**           character passed as an array of strings
** @nam5rule NewStrAsym Comparison matrix uses labels longer than one
**           character passed as an array of strings. Table is
**           asymmetric ... rows and columns have different labels.
**
** @suffix C [char*] C character string
** @suffix S [AjPStr] string object
**
** @argrule C bases [const char*] Allowed sequence characters
** @argrule S basestr [const AjPStr] Allowed sequence characters
** @argrule Str basearray [const AjPPStr] Allowed sequence characters
**                                          string array
** @argrule Str numbases [ajint] Size of sequence characters
**                                        string array
** @argrule Asym matchbases [const AjPPStr] Allowed matching
**                                          sequence characters
** @argrule Asym nummatch [ajint] Size of matching sequence characters
**                                         string array
** @valrule * [AjPSeqCvt]
**
******************************************************************************/





/* @func ajSeqcvtNewC *********************************************************
**
** Generates a new conversion table in which the first character in the
** string provided is converted to 1, the second to 2, and so on.
** Upper and lower case characters are converted to the same numbers.
** All other characters are set to zero.
**
** @param [r] bases [const char*] Allowed sequence characters.
** @return [AjPSeqCvt] Conversion table.
**
** @release 4.1.0
** @@
******************************************************************************/

AjPSeqCvt ajSeqcvtNewC(const char* bases)
{
    static AjPSeqCvt ret;
    ajint i;
    const char *cp;

    cp = bases;

    AJNEW0(ret);
    ret->len     = strlen(bases);
    ret->size    = CHAR_MAX - CHAR_MIN + 1;
    ret->table   = AJCALLOC0(ret->size, sizeof(char));
    ret->bases   = ajStrNewC(bases);
    ret->missing = 0;

    i = 0;

    while(*cp)
    {
	i++;
	ret->table[toupper((ajint) *cp)] = ajSysCastItoc(i);
	ret->table[tolower((ajint) *cp)] = ajSysCastItoc(i);
	cp++;
    }

    return ret;
}




/* @func ajSeqcvtNewEndC ******************************************************
**
** Generates a new conversion table in which the first character in the
** string provided is converted to 0, the second to 1, and so on.
** Upper and lower case characters are converted to the same numbers.
** All other characters are converted to one more than the highest char.
**
** @param [r] bases [const char*] Allowed sequence characters.
** @return [AjPSeqCvt] Conversion table.
**
** @release 4.1.0
** @@
******************************************************************************/

AjPSeqCvt ajSeqcvtNewEndC(const char* bases)
{
    static AjPSeqCvt ret;
    ajint i;
    ajint j;
    ajint imax;
    const char *cp;

    cp = bases;

    imax = strlen(bases);

    AJNEW0(ret);
    ret->len     = imax;
    ret->size    = CHAR_MAX - CHAR_MIN + 1;
    ret->table   = AJCALLOC0(ret->size, sizeof(char));
    ret->bases   = ajStrNewC(bases);
    ret->missing = imax;

    for(j=0; j < ret->size; j++)
	ret->table[j] = ajSysCastItoc(imax);

    i = 0;

    while(*cp)
    {
	ret->table[toupper((ajint) *cp)] = ajSysCastItoc(i);
	ret->table[tolower((ajint) *cp)] = ajSysCastItoc(i);
	cp++;
	i++;
    }

    return ret;
}




/* @func ajSeqcvtNewNumberC ***************************************************
**
** Generates a new conversion table in which the characters are retained
** as upper case, numbers are set to -1 and all other characters
** are set to -2.
**
** @param [r] bases [const char*] Allowed sequence characters.
** @return [AjPSeqCvt] Conversion table.
**
** @release 4.1.0
** @@
******************************************************************************/

AjPSeqCvt ajSeqcvtNewNumberC(const char* bases)
{
    static AjPSeqCvt ret;
    ajint i;
    ajint j;
    const char *cp;
    char c;

    cp = bases;

    AJNEW0(ret);
    ret->len     = strlen(bases);
    ret->size    = CHAR_MAX - CHAR_MIN + 1;
    ret->table   = AJCALLOC0(ret->size, sizeof(char));
    ret->bases   = ajStrNewC(bases);
    ret->missing = -1;

    for(j=0; j < ret->size; j++)
	if(isdigit(j))
	    ret->table[j] = -1;
	else
	    ret->table[j] = -2;


    i = 0;

    while(*cp)
    {
	c = ajSysCastItoc(toupper((ajint)*cp));
	ret->table[toupper((ajint) *cp)] = c;
	ret->table[tolower((ajint) *cp)] = c;
	cp++;
	i++;
    }

    return ret;
}




/* @func ajSeqcvtNewStr *******************************************************
**
** Generates a new conversion table in which the first character of the first 
** string in the array provided is converted to 1, the first character of the 
** second string is converted to 2, the first character of the third string is
** converted to 3 and so on.
** Upper and lower case characters are converted to the same numbers.
** All other characters are set to zero.
**
** @param [r] basearray [const AjPPStr] Allowed sequence character strings
**                            (size specified by parameter n)
** @param [r] numbases [ajint] Number of strings
** @return [AjPSeqCvt] Conversion table.
**
** @release 4.1.0
** @@
******************************************************************************/

AjPSeqCvt ajSeqcvtNewStr (const AjPPStr basearray, ajint numbases)
{
    static AjPSeqCvt ret;
    ajint i;
    

    AJNEW0(ret);
    ret->len = numbases;
    ret->size = CHAR_MAX - CHAR_MIN + 1;
    ret->table = AJCALLOC0(ret->size, sizeof(char));
    ret->bases = ajStrNew();
    ret->missing = 0;


    AJCNEW0(ret->rlabels, numbases);
    for(i=0; i<numbases; i++)
	ret->rlabels[i] = ajStrNew();
    for(i=0; i<numbases; i++)
	ajStrAssignS(&ret->rlabels[i], basearray[i]);


    AJCNEW0(ret->clabels, numbases);

    for(i=0; i<numbases; i++)
	ret->clabels[i] = ajStrNew();

    for(i=0; i<numbases; i++)
	ajStrAssignS(&ret->clabels[i], basearray[i]);

    for(i=0; i<numbases; i++)
    {
	ajStrAppendK(&ret->bases, ajStrGetCharFirst(basearray[i]));
	ret->table[toupper((ajint) ajStrGetCharFirst(basearray[i]))] =
	    ajSysCastItoc(i+1);
	ret->table[tolower((ajint) ajStrGetCharFirst(basearray[i]))] =
	    ajSysCastItoc(i+1);
    }

    return ret;
}




/* @func ajSeqcvtNewStrAsym ***************************************************
**
** Generates a new conversion table in which the first character of the first 
** string in the array provided is converted to 1, the first character of the 
** second string is converted to 2, the first character of the third string is
** converted to 3 and so on.
** Upper and lower case characters are converted to the same numbers.
** All other characters are set to zero.
** For use with asymmetrical matrices. 
**
** @param [r] basearray [const AjPPStr] Allowed sequence character strings
**                                      (size specified by parameter n)
** @param [r] numbases [ajint] Number of strings
** @param [r] matchbases [const AjPPStr] Allowed sequence character strings for
**                                       rows (size specified by parameter rn)
** @param [r] nummatch [ajint] Number of strings (rows)
** @return [AjPSeqCvt] Conversion table.
**
** @release 4.1.0
** @@
******************************************************************************/

AjPSeqCvt ajSeqcvtNewStrAsym (const AjPPStr basearray, ajint numbases, 
				 const AjPPStr matchbases, ajint nummatch)
{
    static AjPSeqCvt ret;
    ajint i;
    

    AJNEW0(ret);
    ret->len = numbases;
    ret->nclabels = numbases;
    ret->nrlabels = nummatch;
    ret->size = CHAR_MAX - CHAR_MIN + 1;
    ret->table = AJCALLOC0(ret->size, sizeof(char));
    ret->bases = ajStrNew();
    ret->missing = 0;


    AJCNEW0(ret->rlabels, nummatch);

    for(i=0; i<nummatch; i++)
	ret->rlabels[i] = ajStrNew();

    for(i=0; i<nummatch; i++)
	ajStrAssignS(&ret->rlabels[i], matchbases[i]);


    AJCNEW0(ret->clabels, numbases);

    for(i=0; i<numbases; i++)
	ret->clabels[i] = ajStrNew();

    for(i=0; i<numbases; i++)
	ajStrAssignS(&ret->clabels[i], basearray[i]);


    for(i=0; i<numbases; i++)
    {
	/* ajStrAssignS(&ret->labels[i], bases[i]); */
	ajStrAppendK(&ret->bases, ajStrGetCharFirst(basearray[i]));
	ret->table[toupper((ajint) ajStrGetCharFirst(basearray[i]))] =
	    ajSysCastItoc(i+1);
	ret->table[tolower((ajint) ajStrGetCharFirst(basearray[i]))] =
	    ajSysCastItoc(i+1);
    }

    return ret;
}




/* @section Sequence Destructors **********************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the sequence.
**
** @fdata [AjPSeqCvt]
** @fcategory delete
**
** @nam3rule Del Destroy (free) a sequence conversion table object
**
** @argrule * Pcvt [AjPSeqCvt*] Sequence conversion table object address
**
** @valrule * [void]
**
******************************************************************************/




/* @func ajSeqcvtDel **********************************************************
**
** Delete a conversion table
**
** @param [w] Pcvt [AjPSeqCvt*] Conversion table reference
** @return [void]
**
** @release 4.1.0
** @@
******************************************************************************/

void ajSeqcvtDel (AjPSeqCvt* Pcvt)
{
    ajint i=0;
    
    if(!*Pcvt|| !Pcvt)
	return;

    AJFREE((*Pcvt)->table);
    ajStrDel(&(*Pcvt)->bases);

    if((*Pcvt)->rlabels)
    {
	for(i=0;i<(*Pcvt)->nrlabels;i++)
	    ajStrDel(&(*Pcvt)->rlabels[i]);

	AJFREE((*Pcvt)->rlabels);
    }
    
    if((*Pcvt)->clabels)
    {
	for(i=0;i<(*Pcvt)->nclabels;i++)
	    ajStrDel(&(*Pcvt)->clabels[i]);

	AJFREE((*Pcvt)->clabels);
    }
    
    AJFREE(*Pcvt);

    return;
}




/* @section element retrieval ********************************************
**
** These functions use the contents of a sequence conversion object
**
** @fdata [AjPSeqCvt]
** @fcategory use
**
** @nam3rule Get Return sequence conversion attribute(s)
** @nam4rule GetLen Return length (number of bases defined)
** @nam4rule GetCode Return conversion code
** @nam5rule GetCodeAsym Return conversion code from asymmetric table column
** @nam5rule GetCodeAsymrow Return conversion code from asymmetric table row
**
** @suffix K Single character code
** @suffix S String label code
** @argrule * cvt [const AjPSeqCvt] Conversion table
** @argrule K ch [char] base character
** @argrule S str [const AjPStr] base character
** @valrule GetLen [ajuint] Table length
** @valrule GetCode [ajint] Table code value
******************************************************************************/




/* @func ajSeqcvtGetCodeK *****************************************************
**
** Returns the integer code corresponding to a sequence character
** in a conversion table
**
** @param [r] cvt [const AjPSeqCvt] Conversion table
** @param [r] ch [char] Sequence character
**
** @return [ajint] Conversion code
**
** @release 4.1.0
** @@
******************************************************************************/

ajint ajSeqcvtGetCodeK(const AjPSeqCvt cvt, char ch)
{
    return cvt->table[(ajint)ch];
}




/* @func ajSeqcvtGetCodeS *****************************************************
**
** Returns the integer code corresponding to a sequence character string
** in a conversion table (for rows in asymmetrical matrices).
**
** @param [r] cvt [const AjPSeqCvt] Conversion table
** @param [r] str [const AjPStr] Sequence character string
**
** @return [ajint] Conversion code
**
** @release 4.1.0
** @@
******************************************************************************/

ajint ajSeqcvtGetCodeS (const AjPSeqCvt cvt, const AjPStr str)
{
    ajint i=0;

    for(i=0;i<cvt->nrlabels;i++)
	if(ajStrMatchS(str, cvt->rlabels[i]))
	    return i+1;

    /* i+1 is returned because the size of a matrix is always 1 bigger than
       the number of labels. This is the "padding" first row/column which 
       has all values of 0. */


    ajWarn("Sequence character string not found in ajSeqcvtGetCodeS");
    return 0;
}




/* @func ajSeqcvtGetCodeAsymS *************************************************
**
** Returns the integer code corresponding to a sequence character string
** in a conversion table (for columns in asymmetrical matrices).
**
** @param [r] cvt [const AjPSeqCvt] Conversion table
** @param [r] str [const AjPStr] Sequence character string
**
** @return [ajint] Conversion code
**
** @release 4.1.0
** @@
******************************************************************************/

ajint ajSeqcvtGetCodeAsymS (const AjPSeqCvt cvt, const AjPStr str)
{
    ajint i=0;
    
    for(i=0;i<cvt->nclabels;i++)
	if(ajStrMatchS(str, cvt->clabels[i]))
	    return i+1;

    /* i+1 is returned because the size of a matrix is always 1 bigger than
       the number of labels. This is the "padding" first row/column which 
       has all values of 0. */


    ajWarn("Sequence character string not found in ajSeqcvtGetCodeAsymS");
    return 0;
}




/* @func ajSeqcvtGetCodeAsymrowS **********************************************
**
** Returns the integer code corresponding to a sequence character string
** in a conversion table (for rows in asymmetrical matrices).
**
** @param [r] cvt [const AjPSeqCvt] Conversion table
** @param [r] str [const AjPStr] Sequence character string
**
** @return [ajint] Conversion code
**
** @release 6.3.0
** @@
******************************************************************************/

ajint ajSeqcvtGetCodeAsymrowS (const AjPSeqCvt cvt, const AjPStr str)
{
    ajint i=0;
    
    for(i=0;i<cvt->nrlabels;i++)
	if(ajStrMatchS(str, cvt->rlabels[i]))
	    return i+1;

    /* i+1 is returned because the size of a matrix is always 1 bigger than
       the number of labels. This is the "padding" first row/column which 
       has all values of 0. */


    ajWarn("Sequence character string not found in ajSeqcvtGetCodeAsymrowS");
    return 0;
}




/* @func ajSeqcvtGetLen *******************************************************
**
** Returns the length of a conversion table string (number of sequence
** characters explicitly included)
**
** @param [r] cvt [const AjPSeqCvt] Conversion table
**
** @return [ajuint] Length
**
** @release 4.1.0
** @@
******************************************************************************/

ajuint ajSeqcvtGetLen(const AjPSeqCvt cvt)
{
    return cvt->len;
}




/* @datasection [AjPSeqDate] sequence dates ***********************************
**
** Functions handling sequence dates
**
** @nam2rule Seqdate
**
******************************************************************************/




/* @section sequence date constructors ****************************************
**
** @fdata [AjPSeqDate]
** @fcategory new
**
** @nam3rule New Constructor
** @nam4rule NewDate Copy constructor
**
** @argrule Date date [const AjPSeqDate] Original date to be copied
**
** @valrule * [AjPSeqDate]
**
******************************************************************************/




/* @func ajSeqdateNew *********************************************************
**
** Constructor for empty sequence date object
**
** @return [AjPSeqDate] Empty sequence date object
**
** @release 5.0.0
******************************************************************************/

AjPSeqDate ajSeqdateNew(void)
{
    AjPSeqDate ret;
    AJNEW0(ret);

    return ret;
}




/* @func ajSeqdateNewDate *****************************************************
**
** Constructor for copy of a sequence date object
**
** @param [r] date [const AjPSeqDate] Sequence date object
** @return [AjPSeqDate] Empty sequence date object
**
** @release 5.0.0
******************************************************************************/

AjPSeqDate ajSeqdateNewDate(const AjPSeqDate date)
{
    AjPSeqDate ret;
    AJNEW0(ret);

    if(!date)
	return ret;

    if(date->CreDate)
	ret->CreDate = ajTimeNewTime(date->CreDate);

    if(date->ModDate)
	ret->ModDate = ajTimeNewTime(date->ModDate);

    if(date->SeqDate)
	ret->SeqDate = ajTimeNewTime(date->SeqDate);

    if(date->CreRel)
	ajStrAssignS(&ret->CreRel, date->CreRel);

    if(date->ModRel)
	ajStrAssignS(&ret->ModRel, date->ModRel);

    if(date->SeqRel)
	ajStrAssignS(&ret->SeqRel, date->SeqRel);

    if(date->CreVer)
	ajStrAssignS(&ret->CreVer, date->CreVer);

    if(date->ModVer)
	ajStrAssignS(&ret->ModVer, date->ModVer);

    if(date->SeqVer)
	ajStrAssignS(&ret->SeqVer, date->SeqVer);

    return ret;
}




/* @section destructors **********************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the sequence.
**
** @fdata [AjPSeqDate]
** @fcategory delete
**
** @nam3rule Del Destroy (free) a sequence date object
**
** @argrule * Pdate [AjPSeqDate*] Sequence date object address
**
** @valrule * [void]
**
******************************************************************************/




/* @func ajSeqdateDel *********************************************************
**
** Deletes a sequence date object.
**
** @param [d] Pdate [AjPSeqDate*] Sequence date object
** @return [void]
**
** @release 5.0.0
** @@
******************************************************************************/

void ajSeqdateDel(AjPSeqDate* Pdate)
{
    AjPSeqDate date;

    if(!Pdate)
	return;

    if(!*Pdate)
	return;

    date = *Pdate;

    ajTimeDel(&date->CreDate);
    ajTimeDel(&date->ModDate);
    ajTimeDel(&date->SeqDate);

    ajStrDel(&date->CreRel);
    ajStrDel(&date->ModRel);
    ajStrDel(&date->SeqRel);
    ajStrDel(&date->CreVer);
    ajStrDel(&date->ModVer);
    ajStrDel(&date->SeqVer);

    AJFREE(*Pdate);

    return;
}




/* @section tests *************************************************************
**
** These functions test properties of a sequence date
**
** @fdata [AjPSeqDate]
** @fcategory use
**
** @nam3rule Exists    Date is defined
**
** @argrule *     date    [const AjPSeqDate] Sequence date object
**
** @valrule *      [AjBool] True on success
**
******************************************************************************/




/* @func ajSeqdateExists ******************************************************
**
** Returns true if a sequence has a creation or modification date defined
**
** @param [r] date [const AjPSeqDate] Sequence date
** @return [AjBool] ajTrue if date(s) are defined
**
** @release 6.6.0
** @@
******************************************************************************/

AjBool ajSeqdateExists(const AjPSeqDate date)
{
    if(!date)
        return ajFalse;

    if(date->CreDate || date->ModDate || date->SeqDate)
        return ajTrue;
    
    return ajFalse;
}




/* @section modifiers *********************************************************
**
** These functions update contents of a sequence date object.
**
** @fdata [AjPSeqDate]
** @fcategory modify
**
**
** @nam3rule Clear     Clear all contents
**
** @nam3rule Set       Set sequence date properties
** @nam4rule SetCreate Set creation date
** @nam4rule SetModify Set modified date
** @nam4rule SetModseq Set sequence modified date
**
** @suffix   C         Date as a C string
** @suffix   S         Date as a string object
**
** @argrule *     date    [AjPSeqDate] Sequence date object
** @argrule C     datestr [const char*] Character string
** @argrule S     datestr [const AjPStr] String
**
** @valrule *      [AjBool] True on success
** @valrule *Clear [void]
**
******************************************************************************/




/* @func ajSeqdateClear *******************************************************
**
** Resets all data for a sequence date object.
**
** @param [u] date [AjPSeqDate] Sequence date object
** @return [void]
**
** @release 6.1.0
** @@
******************************************************************************/

void ajSeqdateClear(AjPSeqDate date)
{
    if(!date)
        return;

    ajTimeDel(&date->CreDate);
    ajTimeDel(&date->ModDate);
    ajTimeDel(&date->SeqDate);

    ajStrSetClear(&date->CreRel);
    ajStrSetClear(&date->ModRel);
    ajStrSetClear(&date->SeqRel);
    ajStrSetClear(&date->CreVer);
    ajStrSetClear(&date->ModVer);
    ajStrSetClear(&date->SeqVer);

    return;
}




/* @func ajSeqdateSetCreateS **************************************************
**
** Set the sequence creation date
**
** @param [w] date [AjPSeqDate] Sequence date
** @param [r] datestr [const AjPStr] Date string
** @return [AjBool] ajTrue on success
**
** @release 5.0.0
******************************************************************************/

AjBool ajSeqdateSetCreateS(AjPSeqDate date, const AjPStr datestr)
{
    if(!date)
	return ajFalse;

    if(!seqDateSet(&date->CreDate, datestr))
        return ajFalse;

    if(!date->CreVer)
        date->CreVer = ajStrNewC("1");

    return ajTrue;
}




/* @func ajSeqdateSetModifyS **************************************************
**
** Set the entry modification date
**
** @param [w] date [AjPSeqDate] Sequence date
** @param [r] datestr [const AjPStr] Date string
** @return [AjBool] ajTrue on success
**
** @release 5.0.0
******************************************************************************/

AjBool ajSeqdateSetModifyS(AjPSeqDate date, const AjPStr datestr)
{
    if(!date)
	return ajFalse;

    if(!seqDateSet(&date->ModDate, datestr))
        return ajFalse;

    if(!date->ModVer)
        date->ModVer = ajStrNewC("1");

    return ajTrue;
}




/* @func ajSeqdateSetModseqS **************************************************
**
** Set the sequence modification date
**
** @param [w] date [AjPSeqDate] Sequence date
** @param [r] datestr [const AjPStr] Date string
** @return [AjBool] ajTrue on success
**
** @release 5.0.0
******************************************************************************/

AjBool ajSeqdateSetModseqS(AjPSeqDate date, const AjPStr datestr)
{
    if(!date)
	return ajFalse;

    if(!seqDateSet(&date->SeqDate, datestr))
        return ajFalse;

    if(!date->SeqVer)
        date->SeqVer = ajStrNewC("1");

    return ajTrue;
}




/* @funcstatic seqDateSet *****************************************************
**
** Sets a sequence date object
**
** @param [w] date [AjPTime*] Sequence date object
** @param [r] datestr [const AjPStr] Date as a string in EMBL/UniProt style
** @return [AjBool] ajTrue on success
**
** @release 5.0.0
******************************************************************************/

static AjBool seqDateSet(AjPTime* date, const AjPStr datestr)
{
    AjBool ok = ajTrue;
    AjPStrTok handle = NULL;
    AjPStr tmpstr = NULL;
    const char* months[] =
    {
	"JAN", "FEB", "MAR", "APR", "MAY", "JUN",
	"JUL", "AUG", "SEP", "OCT", "NOV", "DEC", NULL
    };
    ajint day;
    ajint month;
    ajint year;

    ajStrTokenAssignC(&handle, datestr, "-");

    ajStrTokenNextParse(handle, &tmpstr);
    ajStrToInt(tmpstr, &day);

    ajStrTokenNextParse(handle, &tmpstr);
    month = 0;

    while(months[month] && !ajStrMatchC(tmpstr, months[month]))
	month++;

    if(!months[month])
    {
        ok = ajFalse;
	month = 1;
    }
    else
	month++;

    ajStrTokenNextParse(handle, &tmpstr);
    ajStrToInt(tmpstr, &year);

    if(year > 1900)
	year -= 1900;

    if(!day)
    {
        ok = ajFalse;
        day = 1;
    }

    if(ok)
        *date = ajTimeNewDayFmt("dtline", day, month, year);

    ajStrTokenDel(&handle);
    ajStrDel(&tmpstr);

    return ok;
}




/* @datasection [AjPSeqDesc] sequence descriptions *****************************
**
** Functions handling complex descriptions (e.g. in UniProt)
**
** @nam2rule Seqdesc
**
******************************************************************************/




/* @section descriptions constructors ************************************
**
** @fdata [AjPSeqDesc]
** @fcategory new
**
** @nam3rule New Constructor
** @nam4rule NewDesc Copy constructor
**
** @argrule Desc desc [const AjPSeqDesc] Source description object
**
** @valrule * [AjPSeqDesc]
**
******************************************************************************/




/* @func ajSeqdescNew *********************************************************
**
** Constructor for empty description object
**
** @return [AjPSeqDesc] Empty sequence description object
**
** @release 6.1.0
******************************************************************************/

AjPSeqDesc ajSeqdescNew(void)
{
    AjPSeqDesc ret;

    AJNEW0(ret);

    ret->Name     = ajStrNew();

    ret->Short    = ajListstrNew();
    ret->EC       = ajListstrNew();
    ret->Multi    = ajListstrNew();


    ret->AltNames = ajListNew();
    ret->SubNames = ajListNew();
    ret->Includes = ajListNew();
    ret->Contains = ajListNew();
    return ret;
}




/* @func ajSeqdescNewDesc *****************************************************
**
** Constructor for copy of a description object
**
** @param [r] desc [const AjPSeqDesc] Description object
** @return [AjPSeqDesc] Copied description object
**
** @release 6.1.0
******************************************************************************/

AjPSeqDesc ajSeqdescNewDesc(const AjPSeqDesc desc)
{
    AjPSeqDesc ret;

    AJNEW0(ret);

    if(!desc)
	return ret;

    ret->AltNames = ajListNew();
    ret->SubNames = ajListNew();
    ret->Includes = ajListNew();
    ret->Contains = ajListNew();

    ajStrAssignS(&ret->Name, desc->Name);
    ret->Short = ajListstrNewList(desc->Short);
    ret->EC = ajListstrNewList(desc->EC);
    ret->Multi = ajListstrNewList(desc->Multi);
    ajSeqsubdesclistClone(desc->AltNames, ret->AltNames);
    ajSeqsubdesclistClone(desc->SubNames, ret->SubNames);
    ajSeqdesclistClone(desc->Includes, ret->Includes);
    ajSeqdesclistClone(desc->Contains, ret->Contains);

    ret->Precursor = desc->Precursor;
    ret->Fragments = desc->Fragments;

    return ret;
}




/* @section destructors **********************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for a description object
**
** @fdata [AjPSeqDesc]
** @fcategory delete
**
** @nam3rule Del Destroy (free) a description object
**
** @argrule * Pdesc [AjPSeqDesc*] Description object address
**
** @valrule * [void]
**
******************************************************************************/




/* @func ajSeqdescDel *********************************************************
**
** Deletes a description object.
**
** @param [d] Pdesc [AjPSeqDesc*] Description object
** @return [void]
**
** @release 6.1.0
** @@
******************************************************************************/

void ajSeqdescDel(AjPSeqDesc* Pdesc)
{
    AjPSeqDesc sdesc;
    AjPSeqDesc desc;
    AjPSeqSubdesc sub;

    if(!Pdesc)
	return;

    if(!*Pdesc)
	return;

    sdesc = *Pdesc;

    ajStrDel(&sdesc->Name);

    ajListstrFreeData(&sdesc->Short);
    ajListstrFreeData(&sdesc->EC);
    ajListstrFreeData(&sdesc->Multi);

    while(ajListPop(sdesc->AltNames,(void **)&sub))
	ajSeqsubdescDel(&sub);

    ajListFree(&sdesc->AltNames);

    while(ajListPop(sdesc->SubNames,(void **)&sub))
	ajSeqsubdescDel(&sub);

    ajListFree(&sdesc->SubNames);

    while(ajListPop(sdesc->Includes,(void **)&desc))
	ajSeqdescDel(&desc);

    ajListFree(&sdesc->Includes);

    while(ajListPop(sdesc->Contains,(void **)&desc))
	ajSeqdescDel(&desc);

    ajListFree(&sdesc->Contains);

    AJFREE(*Pdesc);

    return;
}




/* @section modifiers ************************************************
**
** These functions update contents of a description object.
**
** @fdata [AjPSeqDesc]
** @fcategory modify
**
** @nam3rule Append     Append to a description object
** @nam3rule Clear      Clear all contents
** @nam3rule Set        Set a description object
** @nam4rule AppendName Append to name of a description object
** @nam4rule SetName    Set name of a description object
**
** @argrule * desc [AjPSeqDesc] Description object
**
** @argrule Append str [const AjPStr] Text to append
** @argrule Set    str [const AjPStr] Text to assign
**
** @valrule *      [AjBool] True on success
** @valrule *Clear [void]
******************************************************************************/




/* @func ajSeqdescAppendName **************************************************
**
** Append to the name of a description
**
** @param [u] desc [AjPSeqDesc] Description object
** @param [r] str [const AjPStr] Name string
** @return [AjBool] True on success
**
** @release 6.1.0
******************************************************************************/

AjBool ajSeqdescAppendName(AjPSeqDesc desc, const AjPStr str)
{
    if(ajStrGetLen(desc->Name))
	ajStrAppendK(&desc->Name, ' ');

    ajStrAppendS(&desc->Name, str);

    return ajTrue;
}




/* @func ajSeqdescClear *******************************************************
**
** Resets data for a description object.
**
** @param [u] desc [AjPSeqDesc] Description object
** @return [void]
**
** @release 6.1.0
** @@
******************************************************************************/

void ajSeqdescClear(AjPSeqDesc desc)
{
    AjPSeqDesc sdesc;
    AjPSeqSubdesc sub;
    AjPStr ptr = NULL;

    if(!desc) return;

    ajStrSetClear(&desc->Name);

    while(ajListstrPop(desc->Short,&ptr))
	ajStrDel(&ptr);

    while(ajListstrPop(desc->EC,&ptr))
	ajStrDel(&ptr);

    while(ajListstrPop(desc->Multi,&ptr))
	ajStrDel(&ptr);

    while(ajListPop(desc->AltNames,(void **)&sub))
	ajSeqsubdescDel(&sub);

    while(ajListPop(desc->SubNames,(void **)&sub))
	ajSeqsubdescDel(&sub);

    while(ajListPop(desc->Includes,(void **)&sdesc))
	ajSeqdescDel(&sdesc);

    while(ajListPop(desc->Contains,(void **)&sdesc))
	ajSeqdescDel(&sdesc);

    desc->Precursor = ajFalse;
    desc->Fragments = 0;

    return;
}




/* @func ajSeqdescSetName *****************************************************
**
** Set the name of a description object
**
** @param [u] desc [AjPSeqDesc] Description object
** @param [r] str [const AjPStr] Synonyms string
** @return [AjBool] True on success
**
** @release 6.1.0
******************************************************************************/

AjBool ajSeqdescSetName(AjPSeqDesc desc, const AjPStr str)
{
    ajStrAssignS(&desc->Name, str);

    return ajTrue;
}




/* @datasection [AjPList] Description list operations *************************
**
** Manipulating lists of descriptions
**
** @nam2rule Seqdesclist
**
******************************************************************************/




/* @section Description list operations ****************************************
**
** Manipulating lists of descriptions
**
** @fdata [AjPList]
** @fcategory use
**
** @nam3rule Clone Clone list of sequence descriptions
**
** @argrule * src [const AjPList] List of sequence description objects
** @argrule Clone dest [AjPList] Empty list to hold sequence description objects
**
** @valrule * [AjBool] True on success
**
******************************************************************************/




/* @func ajSeqdesclistClone ***************************************************
**
** Copy a list of genes to another list
**
** @param [r] src [const AjPList] Source list of descriptions
** @param [w] dest [AjPList] Destination list of descriptions
** @return [AjBool] True on success
**
** @release 6.1.0
******************************************************************************/

AjBool ajSeqdesclistClone(const AjPList src, AjPList dest)
{
    AjIList iter;
    AjPSeqDesc descout = NULL;
    AjPSeqDesc descin = NULL;

    if(ajListGetLength(dest))
	return ajFalse;

    iter = ajListIterNewread(src);

    while ((descin = (AjPSeqDesc) ajListIterGet(iter)))
    {
	descout = ajSeqdescNewDesc(descin);
	ajListPushAppend(dest, descout);
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @datasection [AjPSeqSubdesc] sequence sub-descriptions **********************
**
** Functions handling complex sub-descriptions (e.g. in UniProt)
**
** @nam2rule Seqsubdesc
**
******************************************************************************/




/* @section sub-descriptions constructors ************************************
**
** @fdata [AjPSeqSubdesc]
** @fcategory new
**
** @nam3rule New        Constructor
** @nam4rule NewSubdesc Copy constructor
**
** @argrule Subdesc desc [const AjPSeqSubdesc] Source sub-description object
**
** @valrule * [AjPSeqSubdesc]
**
******************************************************************************/




/* @func ajSeqsubdescNew ******************************************************
**
** Constructor for empty sub-description object
**
** @return [AjPSeqSubdesc] Empty sequence sub-description object
**
** @release 6.1.0
******************************************************************************/

AjPSeqSubdesc ajSeqsubdescNew(void)
{
    AjPSeqSubdesc ret;
    AJNEW0(ret);

    ret->Short     = ajListstrNew();
    ret->EC        = ajListstrNew();
    ret->Allergen  = ajListstrNew();
    ret->Biotech   = ajListstrNew();
    ret->Cdantigen = ajListstrNew();
    ret->Inn       = ajListstrNew();

    return ret;
}




/* @func ajSeqsubdescNewSubdesc ***********************************************
**
** Constructor for copy of a sub-description object
**
** @param [r] desc [const AjPSeqSubdesc] Sub-description object
** @return [AjPSeqSubdesc] Copied sub-description object
**
** @release 6.1.0
******************************************************************************/

AjPSeqSubdesc ajSeqsubdescNewSubdesc(const AjPSeqSubdesc desc)
{
    AjPSeqSubdesc ret;
    AJNEW0(ret);

    if(!desc)
	return ret;

    ajStrAssignS(&ret->Name, desc->Name);
    ret->Short = ajListstrNewList(desc->Short);
    ret->EC = ajListstrNewList(desc->EC);
    ret->Allergen = ajListstrNewList(desc->Allergen);
    ret->Biotech = ajListstrNewList(desc->Biotech);
    ret->Cdantigen = ajListstrNewList(desc->Cdantigen);
    ret->Inn = ajListstrNewList(desc->Inn);

    return ret;
}




/* @section destructors **********************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for a sub-description object
**
** @fdata [AjPSeqSubdesc]
** @fcategory delete
**
** @nam3rule Del Destroy (free) a sub-description object
**
** @argrule * Pdesc [AjPSeqSubdesc*] Sub-description object address
**
** @valrule * [void]
**
******************************************************************************/




/* @func ajSeqsubdescDel ******************************************************
**
** Deletes a sub-description object.
**
** @param [d] Pdesc [AjPSeqSubdesc*] Sub-description object
** @return [void]
**
** @release 6.1.0
** @@
******************************************************************************/

void ajSeqsubdescDel(AjPSeqSubdesc* Pdesc)
{
    AjPSeqSubdesc sdesc;

    if(!Pdesc)
	return;

    if(!*Pdesc)
	return;

    sdesc = *Pdesc;

    ajStrDel(&sdesc->Name);
    ajListstrFreeData(&sdesc->Short);
    ajListstrFreeData(&sdesc->EC);
    ajListstrFreeData(&sdesc->Allergen);
    ajListstrFreeData(&sdesc->Biotech);
    ajListstrFreeData(&sdesc->Cdantigen);
    ajListstrFreeData(&sdesc->Inn);

    AJFREE(*Pdesc);

    return;
}




/* @section modifiers ************************************************
**
** These functions update contents of a sub-description object.
**
** @fdata [AjPSeqSubdesc]
** @fcategory modify
**
** @nam3rule Append Append to a sub-description object
** @nam3rule Clear     Clear all contents
** @nam3rule Set Set a sub-description object
** @nam4rule AppendName Append to name of a sub-description object
** @nam4rule SetName Set name of a sub-description object
**
** @argrule *  desc [AjPSeqSubdesc] Sub-description object
**
** @argrule Append str [const AjPStr] Text to append
** @argrule Set str [const AjPStr] Text to assign
**
** @valrule * [AjBool] True on success
** @valrule *Clear [void]
******************************************************************************/




/* @func ajSeqsubdescAppendName ***********************************************
**
** Append to the name of a description
**
** @param [u] desc [AjPSeqSubdesc] Description object
** @param [r] str [const AjPStr] Name string
** @return [AjBool] True on success
**
** @release 6.1.0
******************************************************************************/

AjBool ajSeqsubdescAppendName(AjPSeqSubdesc desc, const AjPStr str)
{
    if(ajStrGetLen(desc->Name))
	ajStrAppendK(&desc->Name, ' ');
    ajStrAppendS(&desc->Name, str);

    return ajTrue;
}




/* @func ajSeqsubdescClear ****************************************************
**
** Resets data for a sub-description object.
**
** @param [u] desc [AjPSeqSubdesc] Sub-description object
** @return [void]
**
** @release 6.1.0
** @@
******************************************************************************/

void ajSeqsubdescClear(AjPSeqSubdesc desc)
{
    AjPStr ptr = NULL;

    if(!desc)
        return;

    ajStrSetClear(&desc->Name);
    
    while(ajListstrPop(desc->Short,&ptr))
	ajStrDel(&ptr);

    while(ajListstrPop(desc->EC,&ptr))
	ajStrDel(&ptr);

    while(ajListstrPop(desc->Allergen,&ptr))
	ajStrDel(&ptr);

    while(ajListstrPop(desc->Biotech,&ptr))
	ajStrDel(&ptr);

    while(ajListstrPop(desc->Cdantigen,&ptr))
	ajStrDel(&ptr);

    while(ajListstrPop(desc->Inn,&ptr))
	ajStrDel(&ptr);

    return;
}




/* @func ajSeqsubdescSetName **************************************************
**
** Set the name of a sub-description object
**
** @param [u] desc [AjPSeqSubdesc] Sub-description object
** @param [r] str [const AjPStr] Synonyms string
** @return [AjBool] True on success
**
** @release 6.1.0
******************************************************************************/

AjBool ajSeqsubdescSetName(AjPSeqSubdesc desc, const AjPStr str)
{
    ajStrAssignS(&desc->Name, str);

    return ajTrue;
}




/* @datasection [AjPList] Description list operations *************************
**
** Manipulating lists of sub-descriptions
**
** @nam2rule Seqsubdesclist
**
******************************************************************************/




/* @section Description list operations ****************************************
**
** Manipulating lists of sub-descriptions
**
** @fdata [AjPList]
** @fcategory use
**
** @nam3rule Clone Clone list of sequence sub-descriptions
**
** @argrule *     src  [const AjPList] List of sequence sub-description objects
** @argrule Clone dest [AjPList] Empty list to hold sequence sub-description
**                               objects
**
** @valrule * [AjBool] True on success
**
******************************************************************************/




/* @func ajSeqsubdesclistClone ************************************************
**
** Copy a list of sub-descriptions to another list
**
** @param [r] src [const AjPList] Source list of sub-descriptions
** @param [w] dest [AjPList] Destination list of sub-descriptions
** @return [AjBool] True on success
**
** @release 6.1.0
******************************************************************************/

AjBool ajSeqsubdesclistClone(const AjPList src, AjPList dest)
{
    AjIList iter;
    AjPSeqSubdesc descout = NULL;
    AjPSeqSubdesc descin = NULL;

    if(ajListGetLength(dest))
	return ajFalse;

    iter = ajListIterNewread(src);

    while ((descin = (AjPSeqSubdesc) ajListIterGet(iter)))
    {
	descout = ajSeqsubdescNewSubdesc(descin);
	ajListPushAppend(dest, descout);
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @datasection [AjPSeqGene] gene names ***************************************
**
** Functions handling gene names, synonyms and other terms
**
** @nam2rule Seqgene
**
******************************************************************************/




/* @section gene names constructors ************************************
**
** @fdata [AjPSeqGene]
** @fcategory new
**
** @nam3rule New Constructor
** @nam4rule NewGene Copy constructor
** @nam4rule NewName Constructor with given name
**
** @argrule Gene gene [const AjPSeqGene] Source gene object
** @argrule Name str [const AjPStr] Gene name
**
** @valrule * [AjPSeqGene]
**
******************************************************************************/




/* @func ajSeqgeneNew *********************************************************
**
** Constructor for empty gene object
**
** @return [AjPSeqGene] Empty sequence gene object
**
** @release 6.1.0
******************************************************************************/

AjPSeqGene ajSeqgeneNew(void)
{
    AjPSeqGene ret;
    AJNEW0(ret);

    return ret;
}




/* @func ajSeqgeneNewGene *****************************************************
**
** Constructor for copy of a gene object
**
** @param [r] gene [const AjPSeqGene] Gene object
** @return [AjPSeqGene] Copied gene object
**
** @release 6.1.0
******************************************************************************/

AjPSeqGene ajSeqgeneNewGene(const AjPSeqGene gene)
{
    AjPSeqGene ret;
    AJNEW0(ret);

    if(!gene)
	return ret;

    ajStrAssignS(&ret->Name, gene->Name);
    ajStrAssignS(&ret->Synonyms, gene->Synonyms);
    ajStrAssignS(&ret->Orf, gene->Orf);
    ajStrAssignS(&ret->Oln, gene->Oln);

    return ret;
}




/* @func ajSeqgeneNewName *****************************************************
**
** Constructor for gene with given name
**
** @param [r] str [const AjPStr] Gene name
** @return [AjPSeqGene] New gene object
**
** @release 6.1.0
******************************************************************************/

AjPSeqGene ajSeqgeneNewName(const AjPStr str)
{
    AjPSeqGene ret;
    AJNEW0(ret);

    if(!str)
	return ret;

    ajStrAssignS(&ret->Name, str);

    return ret;
}




/* @section destructors **********************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for a gene object
**
** @fdata [AjPSeqGene]
** @fcategory delete
**
** @nam3rule Del Destroy (free) a gene object
**
** @argrule * Pgene [AjPSeqGene*] Gene object address
**
** @valrule * [void]
**
******************************************************************************/




/* @func ajSeqgeneDel *********************************************************
**
** Deletes a gene object.
**
** @param [d] Pgene [AjPSeqGene*] Gene object
** @return [void]
**
** @release 6.1.0
** @@
******************************************************************************/

void ajSeqgeneDel(AjPSeqGene* Pgene)
{
    AjPSeqGene sgene;

    if(!Pgene)
	return;

    if(!*Pgene)
	return;

    sgene = *Pgene;

    ajStrDel(&sgene->Name);
    ajStrDel(&sgene->Synonyms);
    ajStrDel(&sgene->Orf);
    ajStrDel(&sgene->Oln);

    AJFREE(*Pgene);

    return;
}




/* @section modifiers ************************************************
**
** These functions update contents of a gene object.
**
** @fdata [AjPSeqGene]
** @fcategory modify
**
** @nam3rule Append         Append to names of a gene object
** @nam3rule Set            Set names of a gene object
** @nam4rule AppendName     Append to name of a gene object
** @nam4rule AppendOrf      Append to ORF names of a gene object
** @nam4rule AppendOln      Append to ordered locus names of a gene object
** @nam4rule AppendSynonyms Append to synonyms of a gene object
** @nam4rule SetName        Set name of a gene object
** @nam4rule SetOrf         Set ORF names of a gene object
** @nam4rule SetOln         Set ordered locus names of a gene object
** @nam4rule SetSynonyms    Set synonyms of a gene object
**
** @argrule Append gene [AjPSeqGene] Gene object
** @argrule Set    gene [AjPSeqGene] Gene object
**
** @argrule Append str [const AjPStr] Text to append
** @argrule Set    str [const AjPStr] Text to assign
**
** @valrule * [AjBool] True on success
******************************************************************************/




/* @func ajSeqgeneAppendName **************************************************
**
** Append to the name of a gene
**
** @param [u] gene [AjPSeqGene] Gene object
** @param [r] str [const AjPStr] Name string
** @return [AjBool] True on success
**
** @release 6.1.0
******************************************************************************/

AjBool ajSeqgeneAppendName(AjPSeqGene gene, const AjPStr str)
{
    if(ajStrGetLen(gene->Name))
	ajStrAppendK(&gene->Name, ' ');

    ajStrAppendS(&gene->Name, str);

    return ajTrue;
}




/* @func ajSeqgeneAppendOln ***************************************************
**
** Append to ordered locus names of a gene
**
** @param [u] gene [AjPSeqGene] Gene object
** @param [r] str [const AjPStr] Ordered locus names string
** @return [AjBool] True on success
**
** @release 6.1.0
******************************************************************************/

AjBool ajSeqgeneAppendOln(AjPSeqGene gene, const AjPStr str)
{
    if(ajStrGetLen(gene->Oln))
	ajStrAppendK(&gene->Oln, ' ');

    ajStrAppendS(&gene->Oln, str);

    return ajTrue;
}




/* @func ajSeqgeneAppendOrf ***************************************************
**
** Append to the ORF name of a gene
**
** @param [u] gene [AjPSeqGene] Gene object
** @param [r] str [const AjPStr] ORF name string
** @return [AjBool] True on success
**
** @release 6.1.0
******************************************************************************/

AjBool ajSeqgeneAppendOrf(AjPSeqGene gene, const AjPStr str)
{
    if(ajStrGetLen(gene->Orf))
	ajStrAppendK(&gene->Orf, ' ');

    ajStrAppendS(&gene->Orf, str);

    return ajTrue;
}




/* @func ajSeqgeneAppendSynonyms **********************************************
**
** Append to the name of a gene
**
** @param [u] gene [AjPSeqGene] Gene object
** @param [r] str [const AjPStr] Synonyms string
** @return [AjBool] True on success
**
** @release 6.1.0
******************************************************************************/

AjBool ajSeqgeneAppendSynonyms(AjPSeqGene gene, const AjPStr str)
{
    if(ajStrGetLen(gene->Synonyms))
	ajStrAppendK(&gene->Synonyms, ' ');

    ajStrAppendS(&gene->Synonyms, str);

    return ajTrue;
}




/* @func ajSeqgeneSetName *****************************************************
**
** Set the name of a gene
**
** @param [u] gene [AjPSeqGene] Gene object
** @param [r] str [const AjPStr] Name string
** @return [AjBool] True on success
**
** @release 6.1.0
******************************************************************************/

AjBool ajSeqgeneSetName(AjPSeqGene gene, const AjPStr str)
{
    ajStrAssignS(&gene->Name, str);

    return ajTrue;
}




/* @func ajSeqgeneSetOln ******************************************************
**
** Set the ordered locus names of a gene
**
** @param [u] gene [AjPSeqGene] Gene object
** @param [r] str [const AjPStr] Ordered locus names string
** @return [AjBool] True on success
**
** @release 6.1.0
******************************************************************************/

AjBool ajSeqgeneSetOln(AjPSeqGene gene, const AjPStr str)
{
    ajStrAssignS(&gene->Oln, str);

    return ajTrue;
}




/* @func ajSeqgeneSetOrf ******************************************************
**
** Set the ORF name of a gene
**
** @param [u] gene [AjPSeqGene] Gene object
** @param [r] str [const AjPStr] ORF name string
** @return [AjBool] True on success
**
** @release 6.1.0
******************************************************************************/

AjBool ajSeqgeneSetOrf(AjPSeqGene gene, const AjPStr str)
{
    ajStrAssignS(&gene->Orf, str);

    return ajTrue;
}




/* @func ajSeqgeneSetSynonyms *************************************************
**
** Set the name of a gene
**
** @param [u] gene [AjPSeqGene] Gene object
** @param [r] str [const AjPStr] Synonyms string
** @return [AjBool] True on success
**
** @release 6.1.0
******************************************************************************/

AjBool ajSeqgeneSetSynonyms(AjPSeqGene gene, const AjPStr str)
{
    ajStrAssignS(&gene->Synonyms, str);

    return ajTrue;
}




/* @datasection [AjPList] Reference list operations ***************************
**
** Manipulating lists of genes
**
** @nam2rule Seqgenelist
**
******************************************************************************/




/* @section Reference list operations *****************************************
**
** Manipulating lists of genes
**
** @fdata [AjPList]
** @fcategory use
**
** @nam3rule Clone Clone list of sequence citations
**
** @argrule * src [const AjPList] List of sequence citation objects
** @argrule Clone dest [AjPList] Empty list to hold sequence citation objects
**
** @valrule * [AjBool] True on success
**
******************************************************************************/




/* @func ajSeqgenelistClone ***************************************************
**
** Copy a list of genes to another list
**
** @param [r] src [const AjPList] Source list of genes
** @param [w] dest [AjPList] Destination list of genes
** @return [AjBool] True on success
**
** @release 6.1.0
******************************************************************************/

AjBool ajSeqgenelistClone(const AjPList src, AjPList dest)
{
    AjIList iter;
    AjPSeqGene geneout = NULL;
    AjPSeqGene genein = NULL;

    if(ajListGetLength(dest))
	return ajFalse;

    iter = ajListIterNewread(src);

    while ((genein = (AjPSeqGene) ajListIterGet(iter)))
    {
	geneout = ajSeqgeneNewGene(genein);
	ajListPushAppend(dest, geneout);
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @datasection [AjPSeqXref] sequence citations ********************************
**
** Functions handling sequence cross-references
**
** @nam2rule Seqxref
**
******************************************************************************/




/* @section sequence cross-reference constructors ******************************
**
** @fdata [AjPSeqXref]
** @fcategory new
**
** @nam3rule New Constructor
** @nam4rule NewDb Constructor with database and id
** @nam4rule NewRef Copy constructor
** @suffix C Data provided as char*
** @suffix S Data provided as AjPStr
**
** @argrule Ref xref [const AjPSeqXref] Source sequence cross-reference object
** @argrule Db id      [const AjPStr] Identifier
** @argrule C  db      [const char*] Database name
** @argrule S  db      [const AjPStr] Database name
** @argrule Db reftype [ajuint] Enumerated reference type
**
** @valrule * [AjPSeqXref]
**
******************************************************************************/




/* @func ajSeqxrefNew *********************************************************
**
** Constructor for empty sequence citation object
**
** @return [AjPSeqXref] Empty sequence cross-reference object
**
** @release 6.1.0
******************************************************************************/

AjPSeqXref ajSeqxrefNew(void)
{
    AjPSeqXref ret;
    AJNEW0(ret);

    return ret;
}




/* @func ajSeqxrefNewDbC ******************************************************
**
** Constructor for copy of a sequence citation object
**
** @param [r] id [const AjPStr] Primary identifier
** @param [r] db [const char *] Database name
** @param [r] reftype [ajuint] Reference type
** @return [AjPSeqXref] New sequence cross-reference object
**
** @release 6.1.0
******************************************************************************/

AjPSeqXref ajSeqxrefNewDbC(const AjPStr id, const char* db, ajuint reftype)
{
    AjPSeqXref ret;

    AJNEW0(ret);

    ajStrAssignC(&ret->Db, db);
    ajStrAssignS(&ret->Id, id);

    if(ajStrPrefixCaseS(ret->Id, ret->Db) &&
       ajStrGetCharPos(ret->Id, ajStrGetLen(ret->Db)) == ':')
        ajStrCutStart(&ret->Id, 1+ajStrGetLen(ret->Db));

    ret->Type = reftype;

    return ret;
}




/* @func ajSeqxrefNewDbS ******************************************************
**
** Constructor for copy of a sequence citation object
**
** @param [r] id [const AjPStr] Primary identifier
** @param [r] db [const AjPStr] Database name
** @param [r] reftype [ajuint] Reference type
** @return [AjPSeqXref] New sequence cross-reference object
**
** @release 6.1.0
******************************************************************************/

AjPSeqXref ajSeqxrefNewDbS(const AjPStr id, const AjPStr db, ajuint reftype)
{
    AjPSeqXref ret;

    AJNEW0(ret);

    ajStrAssignS(&ret->Db, db);
    ajStrAssignS(&ret->Id, id);

    if(ajStrPrefixCaseS(ret->Id, ret->Db) &&
       ajStrGetCharPos(ret->Id, ajStrGetLen(ret->Db)) == ':')
        ajStrCutStart(&ret->Id, 1+ajStrGetLen(ret->Db));

    ret->Type = reftype;

    return ret;
}




/* @func ajSeqxrefNewRef ******************************************************
**
** Constructor for copy of a sequence citation object
**
** @param [r] xref [const AjPSeqXref] Sequence cross-reference object
** @return [AjPSeqXref] Copied sequence cross-reference object
**
** @release 6.1.0
******************************************************************************/

AjPSeqXref ajSeqxrefNewRef(const AjPSeqXref xref)
{
    AjPSeqXref ret;
    AJNEW0(ret);

    if(!xref)
	return ret;

    ajStrAssignS(&ret->Db, xref->Db);
    ajStrAssignS(&ret->Id, xref->Id);
    ajStrAssignS(&ret->Secid, xref->Secid);
    ajStrAssignS(&ret->Terid, xref->Terid);
    ajStrAssignS(&ret->Quatid, xref->Quatid);
    ret->Type = xref->Type;
    ret->Start = xref->Start;
    ret->End = xref->End;

    return ret;
}




/* @section destructors **********************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the sequence cross-reference
**
** @fdata [AjPSeqXref]
** @fcategory delete
**
** @nam3rule Del Destroy (free) a sequence cross-reference object
**
** @argrule * Pxref [AjPSeqXref*] Sequence cross-reference object address
**
** @valrule * [void]
**
******************************************************************************/




/* @func ajSeqxrefDel *********************************************************
**
** Deletes a sequence cross-reference object.
**
** @param [d] Pxref [AjPSeqXref*] Sequence cross-reference object
** @return [void]
**
** @release 6.1.0
** @@
******************************************************************************/

void ajSeqxrefDel(AjPSeqXref* Pxref)
{
    AjPSeqXref xref;

    if(!Pxref)
	return;

    if(!*Pxref)
	return;

    xref = *Pxref;

    ajStrDel(&xref->Db);
    ajStrDel(&xref->Id);
    ajStrDel(&xref->Secid);
    ajStrDel(&xref->Terid);
    ajStrDel(&xref->Quatid);

    AJFREE(*Pxref);

    return;
}




/* @datasection [AjPList] Cross-reference list operations *********************
**
** Manipulating lists of sequence cross-references
**
** @nam2rule Seqxreflist
**
******************************************************************************/




/* @section Reference list operations *****************************************
**
** Manipulating lists of sequence cross-references
**
** @fdata [AjPList]
** @fcategory use
**
** @nam3rule Clone Clone list of sequence cross-references
**
** @argrule * src [const AjPList] List of sequence cross-reference objects
** @argrule Clone dest [AjPList] Empty list to hold sequence cross-reference
**                               objects
**
** @valrule * [AjBool] True on success
**
******************************************************************************/




/* @func ajSeqxreflistClone ***************************************************
**
** Copy a list of cross-references to another list
**
** @param [r] src [const AjPList] Source list of cross-references
** @param [w] dest [AjPList] Destination list of cross-references
** @return [AjBool] True on success
**
** @release 6.1.0
******************************************************************************/

AjBool ajSeqxreflistClone(const AjPList src, AjPList dest)
{
    AjIList iter;
    AjPSeqXref refout = NULL;
    AjPSeqXref refin = NULL;

    if(ajListGetLength(dest))
	return ajFalse;

    iter = ajListIterNewread(src);

    while ((refin = (AjPSeqXref) ajListIterGet(iter)))
    {
	refout = ajSeqxrefNewRef(refin);
	ajListPushAppend(dest, refout);
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @section Reference list operations *****************************************
**
** Manipulating lists of sequence cross-references
**
** @fdata [AjPList]
** @fcategory modify
**
** @nam3rule Sort Sort list of sequence cross-references
**
** @argrule * list [AjPList] List of sequence cross-reference objects
**
** @valrule * [AjBool] True on success
**
******************************************************************************/




/* @func ajSeqxreflistSort ****************************************************
**
** Sort a list of cross-references
**
** @param [u] list [AjPList] Source list of cross-references
** @return [AjBool] True on success
**
** @release 6.4.0
******************************************************************************/

AjBool ajSeqxreflistSort(AjPList list)
{
ajListSortUnique(list, &seqxrefSortDb, &seqxrefDel);

    return ajTrue;
}




/* @funcstatic seqxrefSortDb **************************************************
**
** Sort a cross-reference list by database name
**
** @param [r] refa [const void*] First xref
** @param [r] refb [const void*] Second xref
** @return [int] -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
**
** @release 6.4.0
** @@
******************************************************************************/

static int seqxrefSortDb(const void* refa, const void* refb)
{
    const AjPSeqXref xrefa = *(AjPSeqXref const *) refa;
    const AjPSeqXref xrefb = *(AjPSeqXref const *)refb;

    int ret = 0;

    ret = ajStrCmpCaseS(xrefa->Db, xrefb->Db);
    if(ret) return ret;

    ret = ajStrCmpCaseS(xrefa->Id, xrefb->Id);
    if(ret) return ret;
    
    ret = ajStrCmpCaseS(xrefa->Secid, xrefb->Secid);
    if(ret) return ret;
    
    ret = ajStrCmpCaseS(xrefa->Terid, xrefb->Terid);
    if(ret) return ret;

    ret = ajStrCmpCaseS(xrefa->Quatid, xrefb->Quatid);
    if(ret) return ret;

    return 0;
}




/* @funcstatic seqxrefDel *****************************************************
**
** Delete a sequence cross-reference object
**
** @param [d] Pxref [void**] Pointer to cross-reference object
** @param [r] cl [void*] Second argumant (NULL)
** @return [void]
**
** @release 6.4.0
******************************************************************************/

static void seqxrefDel(void** Pxref, void *cl)
{
    (void) cl;

    ajSeqxrefDel((AjPSeqXref*) Pxref);
    return;
}




/* @datasection [AjPSeqRef] sequence citations ********************************
**
** Functions handling sequence citations
**
** @nam2rule Seqref
**
******************************************************************************/




/* @section sequence citation constructors ************************************
**
** @fdata [AjPSeqRef]
** @fcategory new
**
** @nam3rule New Constructor
** @nam4rule NewRef Copy constructor
**
** @argrule Ref ref [const AjPSeqRef] Source sequence citation object
**
** @valrule * [AjPSeqRef]
**
******************************************************************************/




/* @func ajSeqrefNew **********************************************************
**
** Constructor for empty sequence citation object
**
** @return [AjPSeqRef] Empty sequence citation object
**
** @release 5.0.0
******************************************************************************/

AjPSeqRef ajSeqrefNew(void)
{
    AjPSeqRef ret;
    AJNEW0(ret);

    return ret;
}




/* @func ajSeqrefNewRef *******************************************************
**
** Constructor for copy of a sequence citation object
**
** @param [r] ref [const AjPSeqRef] Sequence citation object
** @return [AjPSeqRef] Copied sequence citation object
**
** @release 5.0.0
******************************************************************************/

AjPSeqRef ajSeqrefNewRef(const AjPSeqRef ref)
{
    AjPSeqRef ret;
    AJNEW0(ret);

    if(!ref)
	return ret;

    ajStrAssignS(&ret->Position, ref->Position);
    ajStrAssignS(&ret->Groupname, ref->Groupname);
    ajStrAssignS(&ret->Authors, ref->Authors);
    ajStrAssignS(&ret->Title, ref->Title);
    ajStrAssignS(&ret->Comment, ref->Comment);
    ajStrAssignS(&ret->Xref, ref->Xref);
    ajStrAssignS(&ret->Location, ref->Location);
    ajStrAssignS(&ret->Loctype, ref->Loctype);
    ret->Number = ref->Number;

    return ret;
}




/* @section destructors **********************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the sequence citation
**
** @fdata [AjPSeqRef]
** @fcategory delete
**
** @nam3rule Del Destroy (free) a sequence citation object
**
** @argrule * Pref [AjPSeqRef*] Sequence citation object address
**
** @valrule * [void]
**
******************************************************************************/




/* @func ajSeqrefDel **********************************************************
**
** Deletes a sequence citation object.
**
** @param [d] Pref [AjPSeqRef*] Sequence citation object
** @return [void]
**
** @release 5.0.0
** @@
******************************************************************************/

void ajSeqrefDel(AjPSeqRef* Pref)
{
    AjPSeqRef sref;

    if(!Pref)
	return;

    if(!*Pref)
	return;

    sref = *Pref;

    ajStrDel(&sref->Position);
    ajStrDel(&sref->Groupname);
    ajStrDel(&sref->Authors);
    ajStrDel(&sref->Title);
    ajStrDel(&sref->Comment);
    ajStrDel(&sref->Xref);
    ajStrDel(&sref->Location);
    ajStrDel(&sref->Loctype);

    AJFREE(*Pref);

    return;
}




/* @section modifiers ************************************************
**
** These functions update contents of a sequence citation object.
**
** @fdata [AjPSeqRef]
** @fcategory modify
**
** @nam3rule Append Append to part of a citation
** @nam4rule AppendAuthors Append to authors part of a citation
** @nam4rule AppendComment Append to comment part of a citation
** @nam4rule AppendGroupname Append to group name part of a citation
** @nam4rule AppendLocation Append to location part of a citation
** @nam4rule AppendPosition Append to position part of a citation
** @nam4rule AppendTitle Append to title part of a citation
** @nam4rule AppendXref Append to cross-reference part of a citation
** @nam3rule Fmt Reformat part of a citation
** @nam4rule FmtAuthors Reformat authors part of a citation
** @nam5rule FmtAuthorsEmbl Reformat authors part of a citation for EMBL
** @nam5rule FmtAuthorsGb Reformat authors part of a citation for GenBank
** @nam4rule FmtLocation Reformat location part of a citation
** @nam5rule FmtLocationEmbl Reformat location part of a citation for EMBL
** @nam5rule FmtLocationGb Reformat location part of a citation for GenBank
** @nam4rule FmtTitle Reformat title part of a citation
** @nam5rule FmtTitleEmbl Reformat title part of a citation for EMBL
** @nam5rule FmtTitleGb Reformat title part of a citation for GenBank
** @nam3rule Set Set text part of a citation
** @nam4rule SetAuthors Set authors part of a citation
** @nam4rule SetComment Set comment part of a citation
** @nam4rule SetGroupname Set group name part of a citation
** @nam4rule SetLocation Set location part of a citation
** @nam4rule SetLoctype Set location type of a citation
** @nam4rule SetPosition Set position part of a citation
** @nam4rule SetTitle Set title part of a citation
** @nam4rule SetXref Set cross-reference part of a citation
** @nam3rule Setnum Set numeric part of a citation
** @nam4rule SetnumNumber Set number of a citation
** @nam3rule Standard Standardise internal representation of a citation
**
** @argrule Append ref [AjPSeqRef] Sequence citation object
** @argrule Fmt ref [const AjPSeqRef] Sequence citation object
** @argrule Set ref [AjPSeqRef] Sequence citation object
** @argrule Setnum ref [AjPSeqRef] Sequence citation object
** @argrule Standard ref [AjPSeqRef] Sequence citation object
**
** @argrule Append str [const AjPStr] Text to append
** @argrule Fmt Pdest [AjPStr*] Returned formatted string
** @argrule Set str [const AjPStr] Text to assign
** @argrule Setnum num [ajuint] Number to assign
**
** @valrule * [AjBool] True on success
**
******************************************************************************/




/* @func ajSeqrefAppendAuthors ************************************************
**
** Append to the Authors string of a citation
**
** @param [u] ref [AjPSeqRef] Sequence citation object
** @param [r] str [const AjPStr] Authors string
** @return [AjBool] True on success
**
** @release 5.0.0
******************************************************************************/

AjBool ajSeqrefAppendAuthors(AjPSeqRef ref, const AjPStr str)
{
    if(ajStrGetLen(ref->Authors))
	ajStrAppendK(&ref->Authors, ' ');

    ajStrAppendS(&ref->Authors, str);

    return ajTrue;
}




/* @func ajSeqrefAppendComment ************************************************
**
** Append to the Comment string of a citation
**
** @param [u] ref [AjPSeqRef] Sequence citation object
** @param [r] str [const AjPStr] Comment string
** @return [AjBool] True on success
**
** @release 5.0.0
******************************************************************************/

AjBool ajSeqrefAppendComment(AjPSeqRef ref, const AjPStr str)
{
    if(ajStrGetLen(ref->Comment))
	ajStrAppendK(&ref->Comment, ' ');

    ajStrAppendS(&ref->Comment, str);

    return ajTrue;
}




/* @func ajSeqrefAppendGroupname **********************************************
**
** Append to the group name string of a citation
**
** @param [u] ref [AjPSeqRef] Sequence citation object
** @param [r] str [const AjPStr] Group name string
** @return [AjBool] True on success
**
** @release 5.0.0
******************************************************************************/

AjBool ajSeqrefAppendGroupname(AjPSeqRef ref, const AjPStr str)
{
    if(ajStrGetLen(ref->Groupname))
	ajStrAppendK(&ref->Groupname, ' ');

    ajStrAppendS(&ref->Groupname, str);

    return ajTrue;
}




/* @func ajSeqrefAppendLocation ***********************************************
**
** Append to the location string of a citation
**
** @param [u] ref [AjPSeqRef] Sequence citation object
** @param [r] str [const AjPStr] Location string
** @return [AjBool] True on success
**
** @release 5.0.0
******************************************************************************/

AjBool ajSeqrefAppendLocation(AjPSeqRef ref, const AjPStr str)
{
    if(ajStrGetLen(ref->Location))
    {
	if(ajStrGetCharLast(ref->Location) == '.')
	    ajStrAppendK(&ref->Location, '\n');
	else if(ajStrGetCharLast(ref->Location) == ';')
	    ajStrAppendK(&ref->Location, '\n');
	else
	    ajStrAppendK(&ref->Location, ' ');
    }
    else
	ajSeqrefSetLoctype(ref, str);

    ajStrAppendS(&ref->Location, str);

    return ajTrue;
}




/* @func ajSeqrefAppendPosition *q**********************************************
**
** Append to the position string of a citation
**
** @param [u] ref [AjPSeqRef] Sequence citation object
** @param [r] str [const AjPStr] Position string
** @return [AjBool] True on success
**
** @release 5.0.0
******************************************************************************/

AjBool ajSeqrefAppendPosition(AjPSeqRef ref, const AjPStr str)
{
    if(ajStrGetLen(ref->Position))
	ajStrAppendK(&ref->Position, ' ');

    /* EMBL positions are nn-nn */
    /* GenBank positions are (residues nn to nn) */
    ajStrAppendS(&ref->Position, str);

    return ajTrue;
}




/* @func ajSeqrefAppendTitle **************************************************
**
** Append to the title string of a citation
**
** @param [u] ref [AjPSeqRef] Sequence citation object
** @param [r] str [const AjPStr] Title string
** @return [AjBool] True on success
**
** @release 5.0.0
******************************************************************************/

AjBool ajSeqrefAppendTitle(AjPSeqRef ref, const AjPStr str)
{
    if(ajStrGetLen(ref->Title))
	ajStrAppendK(&ref->Title, ' ');

    ajStrAppendS(&ref->Title, str);

    return ajTrue;
}




/* @func ajSeqrefAppendXref ***************************************************
**
** Append to the Cross reference string of a citation
**
** @param [u] ref [AjPSeqRef] Sequence citation object
** @param [r] str [const AjPStr] Xref string
** @return [AjBool] True on success
**
** @release 5.0.0
******************************************************************************/

AjBool ajSeqrefAppendXref(AjPSeqRef ref, const AjPStr str)
{
    if(ajStrGetLen(ref->Xref))
    {
	if(ajStrGetCharLast(ref->Xref) == '.')
	    ajStrAppendK(&ref->Xref, '\n');
	else
	    ajStrAppendK(&ref->Xref, ' ');
    }

    ajStrAppendS(&ref->Xref, str);

    return ajTrue;
}




/* @func ajSeqrefFmtAuthorsEmbl ***********************************************
**
** Return the Authors string of a citation in EMBL format
**
** @param [r] ref [const AjPSeqRef] Sequence citation object
** @param [w] Pdest [AjPStr*] Authors string in EMBL format
** @return [AjBool] True if author list exists
**
** @release 5.0.0
******************************************************************************/

AjBool ajSeqrefFmtAuthorsEmbl(const AjPSeqRef ref, AjPStr* Pdest)
{
    ajStrAssignClear(Pdest);

    if(!ref->Authors)
	return ajFalse;

    ajStrAssignS(Pdest, ref->Authors);

    return ajTrue;
}




/* @func ajSeqrefFmtAuthorsGb *************************************************
**
** Return the Authors string of a citation in Genbank format
**
** @param [r] ref [const AjPSeqRef] Sequence citation object
** @param [w] Pdest [AjPStr*] Authors string in Genbank format
** @return [AjBool] True if author list exists
**
** @release 5.0.0
******************************************************************************/

AjBool ajSeqrefFmtAuthorsGb(const AjPSeqRef ref, AjPStr* Pdest)
{
    ajlong i;
    ajint imax;
    char* cp;

    ajStrAssignClear(Pdest);

    if(!ref->Authors)
	return ajFalse;

    ajStrAssignS(Pdest, ref->Authors);
    i = ajStrFindlastC(*Pdest, ", "); /* replace , with and */

    if(i != -1)
    {
	ajStrCutRange(Pdest, i, i);
	ajStrInsertC(Pdest, i, " and");
    }

    cp = ajStrGetuniquePtr(Pdest);
    imax=ajStrGetLen(*Pdest)-2;

    for(i=0;i<imax;i++)
    {
	if(*cp == ' ' && cp[2] == '.' && isalpha((int)cp[1]))
	    *cp = ',';

	cp++;
    }

    return ajTrue;
}




/* @func ajSeqrefFmtLocationEmbl **********************************************
**
** Return the location string of a citation in EMBL format
**
** @param [r] ref [const AjPSeqRef] Sequence citation object
** @param [w] Pdest [AjPStr*] Authors string in EMBL format
** @return [AjBool] True if author list exists
**
** @release 5.0.0
******************************************************************************/

AjBool ajSeqrefFmtLocationEmbl(const AjPSeqRef ref, AjPStr* Pdest)
{
    ajStrAssignClear(Pdest);

    if(!ref->Location)
	return ajFalse;

    ajStrAssignS(Pdest, ref->Location);

    return ajTrue;
}




/* @func ajSeqrefFmtLocationGb ************************************************
**
** Return the location string of a citation in Genbank format
**
** @param [r] ref [const AjPSeqRef] Sequence citation object
** @param [w] Pdest [AjPStr*] Authors string in Genbank format
** @return [AjBool] True if author list exists
**
** @release 5.0.0
******************************************************************************/

AjBool ajSeqrefFmtLocationGb(const AjPSeqRef ref, AjPStr* Pdest)
{
    ajStrAssignClear(Pdest);

    if(!ref->Location)
	return ajFalse;

    ajStrAssignS(Pdest, ref->Location);

    ajStrExchangeCC(Pdest, "):", "), ");
    ajStrExchangeCC(Pdest, "(", "\1");
    ajStrExchangeCC(Pdest, "\1", " (");
    ajStrExchangeCC(Pdest, "  (", " (");

    return ajTrue;
}




/* @func ajSeqrefFmtTitleGb ***************************************************
**
** Return the title string of a citation in Genbank format
**
** @param [r] ref [const AjPSeqRef] Sequence citation object
** @param [w] Pdest [AjPStr*] Authors string in Genbank format
** @return [AjBool] True if author list exists
**
** @release 5.0.0
******************************************************************************/

AjBool ajSeqrefFmtTitleGb(const AjPSeqRef ref, AjPStr* Pdest)
{
    ajStrAssignClear(Pdest);

    if(!ajStrGetLen(ref->Title))
    {
	if(ajStrMatchC(ref->Loctype, "submission"))
	{
	    ajStrAssignC(Pdest, "Direct Submission");

	    return ajTrue;
	}

	return ajFalse;
    }

    ajStrAssignS(Pdest, ref->Title);

    return ajTrue;
}




/* @func ajSeqrefSetAuthors ***************************************************
**
** Set the Authors string of a citation
**
** @param [u] ref [AjPSeqRef] Sequence citation object
** @param [r] str [const AjPStr] Authors string
** @return [AjBool] True on success
**
** @release 5.0.0
******************************************************************************/

AjBool ajSeqrefSetAuthors(AjPSeqRef ref, const AjPStr str)
{
    ajStrAssignS(&ref->Authors, str);

    return ajTrue;
}




/* @func ajSeqrefSetComment ***************************************************
**
** Set the Comment string of a citation
**
** @param [u] ref [AjPSeqRef] Sequence citation object
** @param [r] str [const AjPStr] Comment string
** @return [AjBool] True on success
**
** @release 5.0.0
******************************************************************************/

AjBool ajSeqrefSetComment(AjPSeqRef ref, const AjPStr str)
{
    ajStrAssignS(&ref->Comment, str);

    return ajTrue;
}




/* @func ajSeqrefSetGroupname *************************************************
**
** Set the group name string of a citation
**
** @param [u] ref [AjPSeqRef] Sequence citation object
** @param [r] str [const AjPStr] Groupname string
** @return [AjBool] True on success
**
** @release 5.0.0
******************************************************************************/

AjBool ajSeqrefSetGroupname(AjPSeqRef ref, const AjPStr str)
{
    ajStrAssignS(&ref->Groupname, str);

    return ajTrue;
}




/* @func ajSeqrefSetLocation **************************************************
**
** Set the location string of a citation
**
** @param [u] ref [AjPSeqRef] Sequence citation object
** @param [r] str [const AjPStr] Location string
** @return [AjBool] True on success
**
** @release 5.0.0
******************************************************************************/

AjBool ajSeqrefSetLocation(AjPSeqRef ref, const AjPStr str)
{
    ajStrAssignS(&ref->Location, str);
    ajSeqrefSetLoctype(ref, str);

    return ajTrue;
}




/* @func ajSeqrefSetLoctype ***************************************************
**
** Set the location type of a citation
**
** @param [u] ref [AjPSeqRef] Sequence citation object
** @param [r] str [const AjPStr] Location string
** @return [AjBool] True on success
**
** @release 5.0.0
******************************************************************************/

AjBool ajSeqrefSetLoctype(AjPSeqRef ref, const AjPStr str)
{
    if(ajStrPrefixC(str, "Submitted "))
	ajStrAssignC(&ref->Loctype, "submission");
    else if(ajStrPrefixC(str, "(in) "))
	ajStrAssignC(&ref->Loctype, "book");
    else if(ajStrPrefixC(str, "(er) "))
	ajStrAssignC(&ref->Loctype, "electronic");
    else if(ajStrPrefixC(str, "Thesis "))
	ajStrAssignC(&ref->Loctype, "thesis");
    else if(ajStrPrefixC(str, "Patent "))
	ajStrAssignC(&ref->Loctype, "patent");
    else if(ajStrPrefixC(str, "(misc) "))
	ajStrAssignC(&ref->Loctype, "misc");
    else
	ajStrAssignC(&ref->Loctype, "journal");

    return ajTrue;
}




/* @func ajSeqrefSetPosition **************************************************
**
** Set the position string of a citation
**
** @param [u] ref [AjPSeqRef] Sequence citation object
** @param [r] str [const AjPStr] pPosition string
** @return [AjBool] True on success
**
** @release 5.0.0
******************************************************************************/

AjBool ajSeqrefSetPosition(AjPSeqRef ref, const AjPStr str)
{
    ajStrAssignS(&ref->Position, str);

    return ajTrue;
}




/* @func ajSeqrefSetTitle *****************************************************
**
** Set the title string of a citation
**
** @param [u] ref [AjPSeqRef] Sequence citation object
** @param [r] str [const AjPStr] Title string
** @return [AjBool] True on success
**
** @release 5.0.0
******************************************************************************/

AjBool ajSeqrefSetTitle(AjPSeqRef ref, const AjPStr str)
{
    ajStrAssignS(&ref->Title, str);

    return ajTrue;
}




/* @func ajSeqrefSetXref ******************************************************
**
** Set the cross reference string of a citation
**
** @param [u] ref [AjPSeqRef] Sequence citation object
** @param [r] str [const AjPStr] Xref string
** @return [AjBool] True on success
**
** @release 5.0.0
******************************************************************************/

AjBool ajSeqrefSetXref(AjPSeqRef ref, const AjPStr str)
{
    ajStrAssignS(&ref->Xref, str);

    return ajTrue;
}




/* @func ajSeqrefSetnumNumber *************************************************
**
** Sets the citation number
**
** @param [u] ref [AjPSeqRef] Sequence citation object
** @param [r] num [ajuint] Citation number
** @return [AjBool] True on success
**
** @release 5.0.0
******************************************************************************/

AjBool ajSeqrefSetnumNumber(AjPSeqRef ref, ajuint num)
{
    ref->Number = num;

    return ajTrue;
}




/* @func ajSeqrefStandard *****************************************************
**
** Standardise internal representation of a sequence reference
**
** @param [u] ref [AjPSeqRef] Sequence citation object
** @return [AjBool] True on success
**
** @release 5.0.0
******************************************************************************/

AjBool ajSeqrefStandard(AjPSeqRef ref)
{

    if(ajStrGetCharLast(ref->Xref) == '.')
	ajStrTrimEndC(&ref->Xref, ".");

    if(ajStrGetCharLast(ref->Location) == '.')
	ajStrTrimEndC(&ref->Location, ".");

    if(ajStrGetCharLast(ref->Title) == '.')
	ajStrTrimEndC(&ref->Title, ".");

    if(ajStrGetCharLast(ref->Title) == ';')
	ajStrTrimEndC(&ref->Title, ";");

    ajStrQuoteStrip(&ref->Title);

    if(ajStrGetCharLast(ref->Authors) == ';')
	ajStrTrimEndC(&ref->Authors, ";");

    return ajTrue;
}




/* @datasection [AjPList] Reference list operations ***************************8
**
** Manipulating lists of sequence citations
**
** @nam2rule Seqreflist
**
******************************************************************************/




/* @section Reference list operations *****************************************
**
** Manipulating lists of sequence citations
**
** @fdata [AjPList]
** @fcategory use
**
** @nam3rule Clone Clone list of sequence citations
**
** @argrule * src [const AjPList] List of sequence citation objects
** @argrule Clone dest [AjPList] Empty list to hold sequence citation objects
**
** @valrule * [AjBool] True on success
**
******************************************************************************/




/* @func ajSeqreflistClone ****************************************************
**
** Copy a list of citations to another list
**
** @param [r] src [const AjPList] Source list of citations
** @param [w] dest [AjPList] Destination list of citations
** @return [AjBool] True on success
**
** @release 5.0.0
******************************************************************************/

AjBool ajSeqreflistClone(const AjPList src, AjPList dest)
{
    AjIList iter;
    AjPSeqRef refout = NULL;
    AjPSeqRef refin = NULL;

    if(ajListGetLength(dest))
	return ajFalse;

    iter = ajListIterNewread(src);

    while ((refin = (AjPSeqRef) ajListIterGet(iter)))
    {
	refout = ajSeqrefNewRef(refin);
	ajListPushAppend(dest, refout);
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @section element retrieval ************************************************
**
** These functions return contents of a list of sequence cross-reference
** objects.
**
** @fdata [AjPList]
** @fcategory use
**
** @nam3rule Get return reference attributes
** @nam4rule GetXrefs Return cross-references
**
** @argrule Get   src   [const AjPList] List of sequence citation object
** @argrule Xrefs Plist [AjPList*]      List of cross-reference objects
**
** @valrule GetXrefs [ajuint] Number of cross-references returned
**
******************************************************************************/




/* @func ajSeqreflistGetXrefs *************************************************
**
** Copy a cross-reference list
**
** @param [r] src [const AjPList] Source cross-reference list
** @param [w] Plist [AjPList*] Target list,
**                             created if it does not already exist
** @return [ajuint] number of cross-references copied
**
** @release 6.2.0
******************************************************************************/

ajuint ajSeqreflistGetXrefs(const AjPList src, AjPList *Plist)
{
    AjPStrTok handle = NULL;
    AjPStr token = NULL;
    AjPStr dbtoken = NULL;
    AjIList iter = NULL;
    ajulong oldnum = 0UL;
    AjPSeqRef tmpref = NULL;
    AjPSeqXref xref = NULL;
    AjPList list;

    if(!*Plist)
        *Plist = ajListNew();

    list = *Plist;

    oldnum = ajListGetLength(list);
    iter = ajListIterNewread(src);
    while(!ajListIterDone(iter))
    {
        tmpref = ajListIterGet(iter);
        if(!ajStrGetLen(tmpref->Xref))
            continue;

        ajStrTokenAssignC(&handle, tmpref->Xref, " =;\r\n");

        while (ajStrTokenNextParseC(handle, " =;\r\n", &dbtoken))
        {
            ajStrTokenNextParseC(handle, " ;\r\n", &token);

            if(ajStrGetCharLast(token) == '.')
                ajStrCutEnd(&token, 1);

            if(!ajStrGetLen(dbtoken))
                continue;
            if(!ajStrGetLen(token))
                continue;

            if(ajStrMatchCaseC(dbtoken, "MEDLINE"))
            {
                xref = ajSeqxrefNewDbC(token, "MEDLINE", XREF_RX);
                ajListPushAppend(list, xref);
                xref = NULL;
            }
            else if(ajStrMatchCaseC(dbtoken, "PubMed"))
            {
                xref = ajSeqxrefNewDbC(token, "PubMed", XREF_RX);
                ajListPushAppend(list, xref);
                xref = NULL;
            }
            else if(ajStrMatchCaseC(dbtoken, "DOI"))
            {
                xref = ajSeqxrefNewDbC(token, "DOI", XREF_RX);
                ajListPushAppend(list, xref);
                xref = NULL;
            }
            else if(ajStrMatchCaseC(dbtoken, "AGRICOLA"))
            {
                xref = ajSeqxrefNewDbC(token, "AGRICOLA", XREF_RX);
                ajListPushAppend(list, xref);
                xref = NULL;
            }
            else
            {
                xref = ajSeqxrefNewDbS(token, dbtoken, XREF_RX);
                ajListPushAppend(list, xref);
                xref = NULL;
            }
        }
    }
    ajListIterDel(&iter);
    
    ajStrTokenDel(&handle);
    ajStrDel(&dbtoken);
    ajStrDel(&token);

    return (ajuint) (ajListGetLength(list) - oldnum);
}




/* @datasection [AjPSeqRange] Sequence ranges **********************************
**
** Functions handling sequence ranges
**
** @nam2rule Seqrange
**
******************************************************************************/




/* @section Sequence range constructors ****************************************
**
** @fdata [AjPSeqRange]
** @fcategory new
**
** @nam3rule New Constructor
** @nam4rule NewRange Copy constructor
** @nam4rule NewValues Constructor with given values
**
** @argrule Range range [const AjPSeqRange] Source range object
** @argrule Values pos [ajulong] Start position
** @argrule Values len [ajulong] Length
** @argrule Values qry [const AjPStr] Query
**
** @valrule * [AjPSeqRange]
**
******************************************************************************/




/* @func ajSeqrangeNew ********************************************************
**
** Constructor for empty range object
**
** @return [AjPSeqRange] Empty sequence range object
**
** @release 6.5.0
******************************************************************************/

AjPSeqRange ajSeqrangeNew(void)
{
    AjPSeqRange ret;
    AJNEW0(ret);

    return ret;
}




/* @func ajSeqrangeNewRange ***************************************************
**
** Constructor for copy of a range object
**
** @param [r] range [const AjPSeqRange] Range object
** @return [AjPSeqRange] Copied range object
**
** @release 6.5.0
******************************************************************************/

AjPSeqRange ajSeqrangeNewRange(const AjPSeqRange range)
{
    AjPSeqRange ret;

    AJNEW0(ret);

    if(!range)
	return ret;

    ajStrAssignS(&ret->Query, range->Query);
    ret->Start = range->Start;
    ret->Length = range->Length;

    return ret;
}




/* @func ajSeqrangeNewValues **************************************************
**
** Constructor for range with given values
**
** @param [r] pos [ajulong] Start position
** @param [r] len [ajulong] Length
** @param [r] qry [const AjPStr] Query
** @return [AjPSeqRange] New range object
**
** @release 6.5.0
******************************************************************************/

AjPSeqRange ajSeqrangeNewValues(ajulong pos, ajulong len, const AjPStr qry)
{
    AjPSeqRange ret;
    AJNEW0(ret);

    ret->Start = pos;
    ret->Length = len;

    if(qry)
        ret->Query = ajStrNewS(qry);

    return ret;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for a sequence range object
**
** @fdata [AjPSeqRange]
** @fcategory delete
**
** @nam3rule Del Destroy (free) a range object
**
** @argrule * Prange [AjPSeqRange*] Range object address
**
** @valrule * [void]
**
******************************************************************************/




/* @func ajSeqrangeDel *********************************************************
**
** Deletes a range object.
**
** @param [d] Prange [AjPSeqRange*] Range object
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ajSeqrangeDel(AjPSeqRange* Prange)
{
    AjPSeqRange srange;

    if(!Prange)
	return;

    if(!*Prange)
	return;

    srange = *Prange;

    ajStrDel(&srange->Query);

    AJFREE(*Prange);

    return;
}




/* @section modifiers *********************************************************
**
** These functions update contents of a range object.
**
** @fdata [AjPSeqRange]
** @fcategory modify
**
** @nam3rule Set            Set data in a range object
** @nam4rule SetQuery       Set query of a range object
** @nam4rule SetRange       Set Start and length of a range object
**
** @argrule Set    range [AjPSeqRange] Range object
**
** @argrule Query    qry [const AjPStr] Query
** @argrule Range    pos [ajulong] Start position
** @argrule Range    len [ajulong] Length
**
** @valrule * [AjBool] True on success
******************************************************************************/




/* @func ajSeqrangeSetQuery ***************************************************
**
** Set the query of a range
**
** @param [u] range [AjPSeqRange] Range object
** @param [r] qry [const AjPStr] Query string
** @return [AjBool] True on success
**
** @release 6.1.0
******************************************************************************/

AjBool ajSeqrangeSetQuery(AjPSeqRange range, const AjPStr qry)
{
    ajStrAssignS(&range->Query, qry);

    return ajTrue;
}




/* @func ajSeqrangeSetRange ***************************************************
**
** Set the start and length of a range
**
** @param [u] range [AjPSeqRange] Range object
** @param [r] pos [ajulong] Start position
** @param [r] len [ajulong] Length
** @return [AjBool] True on success
**
** @release 6.5.0
******************************************************************************/

AjBool ajSeqrangeSetRange(AjPSeqRange range, ajulong pos, ajulong len)
{
    range->Start = pos;
    range->Length = len;

    return ajTrue;
}




/* @datasection [AjPStr] string tests *****************************************
**
** Functions handling strings for specialist sequence-related tests
**
** @nam2rule Seqtest
**
******************************************************************************/




/* @section string tests ******************************************************
**
** @fdata [AjPStr]
** @fcategory use
**
** @nam3rule Is Test string matches some type
** @nam4rule IsAccession Tests string is an accession number for any
**                       known database
** @nam4rule IsSeqversion Tests string is a sequence version number for any
**                       known database
**
** @argrule * str [const AjPStr] String value to test
**
** @valrule IsAccession [AjBool] True if string passes the test
** @valrule IsSeqversion [const AjPStr] Accession number part of the version
******************************************************************************/




/* @func ajSeqtestIsAccession *************************************************
**
** Tests whether a string is a potential sequence accession number.
** The current definition is one or two alpha characters,
** then a possible underscore (for REFSEQ accessions),
** followed by a string of digits and a minimum length of 6.
**
** Revised for new Swiss-Prot accession number format AnXXXn
**
** @param [r] str [const AjPStr] String to be tested
** @return [AjBool] ajTrue if the string is a possible accession number.
**
** @release 4.1.0
** @@
******************************************************************************/

AjBool ajSeqtestIsAccession(const AjPStr str)
{
    ajint i;
    const char *cp;

    if(!str)
	return ajFalse;

    i = ajStrGetLen(str);

    if(i < 6)
	return ajFalse;

    cp = ajStrGetPtr(str);

    /* must have an alphabetic start */

    if(!isalpha((ajint)*cp++))
	return ajFalse;

    /* two choices for the next character */

    if(isalpha((ajint)*cp))
    {					/* EMBL/GenBank AAnnnnnn */
	cp++;

	if(*cp == '_') cp++;		/* REFSEQ NM_nnnnnn */

	while(*cp)
	    if(isdigit((ajint)*cp))
		++cp;
	    else
		return ajFalse;

	return ajTrue;
    }
    else if(isdigit((ajint)*cp))
    {					/* EMBL/GenBank old Annnnn */
	cp++;				/* or SWISS AnXXXn */

	for(i=0; i<3; i++)
	    if(isalpha((ajint)*cp) || isdigit((ajint)*cp))
		cp++;
	    else
		return ajFalse;

	if(!isdigit((ajint)*cp))
	    return ajFalse;

	while(*cp)
	    if(isdigit((ajint)*cp))
		++cp;
	    else
		return ajFalse;

	return ajTrue;
    }

    return ajFalse;
}




/* @func ajSeqtestIsSeqversion ************************************************
**
** Tests whether a string is a potential sequence version number.
** The current definition is an accession number, followed by a dot and
** a number.
**
** Revised for new Swiss-Prot accession number format AnXXXn
** Revised for REFSEQ accession number format NM_nnnnnn
** Revised for protein ID format XXXnnnnnn.nnn
**
** @param [r] str [const AjPStr] String to be tested
** @return [const AjPStr] accession number part of the string if successful
**
** @release 4.1.0
** @@
******************************************************************************/

const AjPStr ajSeqtestIsSeqversion(const AjPStr str)
{
    ajint i;
    const char *cp;
    AjBool dot = ajFalse;		/* have we found the '.' */
    AjBool v = 0;	   /* number of digits of version after '.' */

    if(!str)
	return NULL;

    i = ajStrGetLen(str);

    if(i < 8)
	return NULL;

    cp = ajStrGetPtr(str);

    /* must have an alphabetic start */

    if(!isalpha((ajint)*cp))
	return NULL;

    ajStrAssignResC(&seqVersionAccnum, 12, "");
    ajStrAppendK(&seqVersionAccnum, *cp++);

    /* two choices for the next character */

    if(isalpha((ajint)*cp))
    {					/* EMBL/GenBank AAnnnnnn */
        ajStrAppendK(&seqVersionAccnum, *cp);
	cp++;

	if(isalpha((ajint)*cp))
	{			/* EMBL/GenBank protein_id AAAnnnnnn */
	    ajStrAppendK(&seqVersionAccnum, *cp);
	    cp++;
	}
	else if(*cp == '_')		/* REFSEQ NM_nnnnnn */
	{
	    ajStrAppendK(&seqVersionAccnum, *cp);
	    cp++;
	}
	while(*cp)		      /* optional trailing .version */
	{
	    if(isdigit((ajint)*cp) || *cp=='.')
	    {
		if(*cp == '.')
		{
		    if(dot)
			return NULL;	/* one '.' only */
		    dot = ajTrue;
		}
		else
		{
		    if(dot)
			v++;
		    else
			ajStrAppendK(&seqVersionAccnum, *cp);
		}
		++cp;
	    }
	    else
		return NULL;
	}

	if(v)
	    return seqVersionAccnum;
	else
	    return NULL;
    }
    else if(isdigit((ajint)*cp))
    {					/* EMBL/GenBank old Annnnn */
	/* or SWISS AnXXXn */
        ajStrAppendK(&seqVersionAccnum, *cp);
	cp++;

	for(i=0; i<3; i++)
	    if(isalpha((ajint)*cp) || isdigit((ajint)*cp))
	    {
	        ajStrAppendK(&seqVersionAccnum, *cp);
		cp++;
	    }
	    else
		return NULL;

	if(!isdigit((ajint)*cp))
	    return NULL;

	while(*cp)		      /* optional trailing .version */
	{
	    if(isdigit((ajint)*cp) || *cp=='.')
	    {
		if(*cp == '.')
		{
		    if(dot)
			return NULL; /* one '.' only */
		    dot = ajTrue;
		}
		else
		{
		    if(dot)
			v++;
		    else
			ajStrAppendK(&seqVersionAccnum, *cp);
		}
		++cp;
	    }
	    else
		return NULL;
	}

	if(v)
	    return seqVersionAccnum;
	else
	    return NULL;
    }

    return NULL;
}




/* @datasection [AjPStr] string sequences *************************************
**
** Handles a string as a sequence
**
** Example uses are in sequence output object processing
**
** @nam2rule Seqstr
**
******************************************************************************/




/* @section string properties
**
** @fdata [AjPStr]
** @fcategory use
**
** @nam3rule Calc Calculate sequence properties
** @nam4rule CalcMolwt Calculate molecular weight of a protein
** @nam3rule Count Count statistics over a sequence
** @nam4rule CountGaps Count gap characters
**
** @argrule * seq [const AjPStr] Sequence string to be processed
**
** @valrule CalcMolwt [float] Molecular weight
** @valrule CountGaps [ajuint] Number of gap characters
******************************************************************************/




/* @func ajSeqstrCalcMolwt ****************************************************
**
** Calculates the molecular weight of a protein sequence
** using average molecular weights
**
** @param [r] seq [const AjPStr] Sequence
** @return [float] Molecular weight.
**
** @release 4.0.0
** @@
******************************************************************************/

float ajSeqstrCalcMolwt(const AjPStr seq)
{
    /* source: www.expasy.org/tools/findmod/findmod_masses.html */
    static double aa[26] =
    {
        89.0894,  132.6068,  121.1494,  133.0992, /* A-D */
       147.1261,  165.1872,   75.0625,  155.1517, /* E-H */
       131.1700,  131.1700,  146.1847,  131.1700, /* I-L */
       149.2032,  132.1144,  255.3124,  115.1273, /* M-P */
       146.1413,  174.1981,  105.0888,  119.1157, /* Q-T */
       168.0494,  117.1432,  204.2238,  136.8966, /* U-X */
       181.1866,  146.6337                        /* Y-Z */
    };
    double mw;
    ajint i;
    const char* cp;

    cp = ajStrGetPtr(seq);
    mw = (double) 18.01057;
    
    while(*cp)
    {
        if(*cp == '-')
        {
            cp++;
            continue;
        }

	i = toupper((ajint) *cp)-'A';

	if(i > 25 || i < 0)
	{
	    ajDebug("seqMW bad character '%c' %d\n", *cp, *cp);
	    i = 'X' - 'A';
	}

	mw += (double) aa[i] - (double) 18.01057;
	cp++;
    }

    ajDebug("seqMW calculated %.2f\n", mw);

    return (float) mw;
}




/* @func ajSeqstrCountGaps ****************************************************
**
** Complements but does not reverse a nucleotide sequence provided as a string.
**
** @param [r] seq [const AjPStr] Sequence as a string.
**
** @return [ajuint] Number of gap characters
**
** @release 4.1.0
******************************************************************************/

ajuint ajSeqstrCountGaps(const AjPStr seq)
{
    ajuint ret = 0;

    static char testchars[] = "-~.? "; /* all known gap characters */
    char *testgap;

    testgap = testchars;

    while(*testgap)
    {
      ret += (ajuint) ajStrCalcCountK(seq, *testgap);
	testgap++;
    }

    return ret;
}




/* @section string processing
**
** @fdata [AjPStr]
** @fcategory modify
**
** @nam3rule Complement Complement a sequence but do not reverse it
** @nam3rule Reverse Reverse complement a sequence
**
** @argrule  * Pseq [AjPStr*] Sequence string to be processed
**
** @valrule * [void]
******************************************************************************/




/* @func ajSeqstrComplement ***************************************************
**
** Complements but does not reverse a nucleotide sequence provided as a string.
**
** @param [u] Pseq [AjPStr*] Sequence as a string.
** @return [void]
**
** @release 4.1.0
** @@
******************************************************************************/

void ajSeqstrComplement(AjPStr* Pseq)
{
    char *cp;

    cp = ajStrGetuniquePtr(Pseq);

    while(*cp)
    {
	*cp = ajBaseAlphacharComp(*cp);
	cp++;
    }

    return;
}




/* @func ajSeqstrReverse ******************************************************
**
** Reverses and complements a nucleotide sequence provided as a string.
**
** @param [u] Pseq [AjPStr*] Sequence as a string.
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

void ajSeqstrReverse(AjPStr* Pseq)
{
    char *cp;
    char *cq;
    char tmp;

    cp = ajStrGetuniquePtr(Pseq);
    cq = cp + ajStrGetLen(*Pseq) - 1;

    while(cp < cq)
    {
	tmp = ajBaseAlphacharComp(*cp);
	*cp = ajBaseAlphacharComp(*cq);
	*cq = tmp;
	cp++;
	cq--;
    }

    if(cp == cq)
	*cp = ajBaseAlphacharComp(*cp);

    return;
}




/* @datasection [AjPStr] Sequence entry classes *******************************
**
** Sequence entry class vocabularies
**
** @nam2rule Seqcls
**
******************************************************************************/




/* @section assignment ********************************************************
**
** Assign values for sequence entry classes
**
** @fdata [AjPStr]
** @fcategory assign
**
** @nam3rule Set Assign value
** @nam4rule Embl Assign with an EMBL class name
** @nam4rule Gb   Assign with a GenBank class name
**
** @argrule Set Pcls [AjPStr*] Target sequence class string
** @argrule Embl clsembl [const AjPStr] EMBL class name
** @argrule Gb   clsgb   [const AjPStr] Genbank class name
**
** @valrule * [AjBool] True on success
**
******************************************************************************/




/* @func ajSeqclsSetEmbl ******************************************************
**
** Sets the internal entry class for a sequence
**
** @param [u] Pcls [AjPStr*] Internal entry class name
** @param [r] clsembl [const AjPStr] EMBL database entry class
** @return [AjBool] True if a known type
**
** @release 5.0.0
******************************************************************************/

AjBool ajSeqclsSetEmbl(AjPStr* Pcls, const AjPStr clsembl)
{
    static AjBool called = AJFALSE;
    const AjPStr clsname = NULL;

    if(!called)
    {
	seqclsInit();
	called = ajTrue;
    }

    clsname = ajTablestrFetchS(seqTableClsEmbl, clsembl);

    if(!clsname)
	return ajFalse;

    ajStrAssignS(Pcls, clsname);

    return ajTrue;
    
}




/* @func ajSeqclsSetGb ********************************************************
**
** Sets the internal entry class for a sequence
**
** @param [u] Pcls [AjPStr*] Internal entry class name
** @param [r] clsgb [const AjPStr] Genbank database entry class
** @return [AjBool] True if a known type
**
** @release 5.0.0
******************************************************************************/

AjBool ajSeqclsSetGb(AjPStr* Pcls, const AjPStr clsgb)
{
    static AjBool called = AJFALSE;
    const AjPStr clsname = NULL;

    if(!called)
    {
	seqclsInit();
	called = ajTrue;
    }

    clsname = ajTablestrFetchS(seqTableClsGb, clsgb);

    if(!clsname)
	return ajFalse;

    ajStrAssignS(Pcls, clsname);

    return ajTrue;
    
}




/* @section retrieval ********************************************************
**
** Return values for sequence entry class
**
** @fdata [AjPStr]
** @fcategory use
**
** @nam3rule Get Retrieve value
** @nam4rule Embl Retrieve as an EMBL class name
** @nam4rule Gb   Retrieve as a GenBank class name
**
** @argrule Get cls [const AjPStr] Internal sequence class string
**
** @valrule * [const char*] Databank-specific class name
**
******************************************************************************/




/* @func ajSeqclsGetEmbl ******************************************************
**
** Returns the EMBL entry class for a sequence
**
** @param [r] cls [const AjPStr] Internal entry class name
** @return [const char*] EMBL entry class name
**
** @release 5.0.0
******************************************************************************/

const char* ajSeqclsGetEmbl(const AjPStr cls)
{
    static AjBool called = AJFALSE;
    const SeqOClass *clsdef = NULL;

    if(!called)
    {
	seqclsInit();
	called = ajTrue;
    }

    ajDebug("ajSeqclsGetEmbl '%S'\n", cls);

    if(ajStrGetLen(cls))
	clsdef = ajTableFetchS(seqTableCls, cls);

    if(!clsdef)
	clsdef = ajTableFetchS(seqTableCls, seqClassDef);

    if(!clsdef)
	return ajStrGetPtr(seqClassDef);

    ajDebug("ajSeqclsGetEmbl '%S' => '%s'\n",
	    cls, clsdef->Embl);

    return clsdef->Embl;
    
}




/* @func ajSeqclsGetGb ********************************************************
**
** Returns the GenBank entry class for a sequence
**
** @param [r] cls [const AjPStr] Internal entry class name
** @return [const char*] Genbank entry class name
**
** @release 6.6.0
******************************************************************************/

const char* ajSeqclsGetGb(const AjPStr cls)
{
    static AjBool called = AJFALSE;
    const SeqOClass *clsdef = NULL;

    if(!called)
    {
	seqclsInit();
	called = ajTrue;
    }

    ajDebug("ajSeqclsGetGb '%S'\n", cls);

    if(ajStrGetLen(cls))
	clsdef = ajTableFetchS(seqTableCls, cls);

    if(!clsdef)
	clsdef = ajTableFetchS(seqTableCls, seqClassDef);

    if(!clsdef)
	return ajStrGetPtr(seqClassDef);

    ajDebug("ajSeqclsGetGb '%S' => '%s'\n",
	    cls, clsdef->Genbank);

    return clsdef->Genbank;
    
}




/* @funcstatic seqclsInit *****************************************************
**
** Initialises the tables of molecule types
**
** @return [void]
**
** @release 5.0.0
******************************************************************************/

static void seqclsInit(void)
{
    ajuint i;
    AjPStr keystr = NULL;
    AjPStr valstr = NULL;

    if(seqTableCls)
	return;

    seqTableCls = ajTablestrNewCase(16);
    seqTableClsEmbl = ajTablestrNewCase(16);
    seqTableClsDdbj = ajTablestrNewCase(16);
    seqTableClsGb = ajTablestrNewCase(16);

    seqClassDef = ajStrNewC(seqClass[0].Name);

    for(i=0;seqClass[i].Name;i++)
    {
	keystr = ajStrNewC(seqClass[i].Name);

	ajTablePut(seqTableCls, keystr, &seqClass[i]);

	if(seqClass[i].Embl[0])
	{
	    valstr = ajStrNewC(seqClass[i].Embl);

	    ajTablePut(seqTableClsEmbl, valstr, keystr);
	}

	if(seqClass[i].Ddbj[0])
	{
	    valstr = ajStrNewC(seqClass[i].Ddbj);

	    ajTablePut(seqTableClsDdbj, valstr, keystr);
	}

	if(seqClass[i].Genbank[0])
	{
	    valstr = ajStrNewC(seqClass[i].Genbank);

	    ajTablePut(seqTableClsGb, valstr, keystr);
	}
    }

    return;
}




/* @datasection [AjPStr] Sequence entry divisions *****************************
**
** Sequence entry division vocabularies
**
** @nam2rule Seqdiv
**
******************************************************************************/




/* @section assignment ********************************************************
**
** Assign values for sequence entry division
**
** @fdata [AjPStr]
** @fcategory assign
**
** @nam3rule Set Assign value
** @nam4rule Embl Assign with an EMBL division name
** @nam4rule Gb   Assign with a GenBank division name
**
** @argrule Set Pdivi [AjPStr*] Target sequence division string
** @argrule Embl divembl [const AjPStr] EMBL division name
** @argrule Gb   divgb   [const AjPStr] Genbank division name
**
** @valrule * [AjBool] True on success
**
******************************************************************************/




/* @func ajSeqdivSetEmbl ******************************************************
**
** Sets the internal database division for a sequence
**
** @param [u] Pdivi [AjPStr*] Internal database division
** @param [r] divembl [const AjPStr] EMBL database division
** @return [AjBool] True if a known type
**
** @release 5.0.0
******************************************************************************/

AjBool ajSeqdivSetEmbl(AjPStr* Pdivi, const AjPStr divembl)
{
    static AjBool called = AJFALSE;
    const AjPStr divname = NULL;

    if(!called)
    {
	seqdivInit();
	called = ajTrue;
    }

    divname = ajTableFetchS(seqTableDivEmbl, divembl);

    if(!divname)
	return ajFalse;

    ajStrAssignS(Pdivi, divname);

    return ajTrue;
    
}




/* @func ajSeqdivSetGb ********************************************************
**
** Sets the internal database division for a sequence
**
** @param [u] Pdivi [AjPStr*] Internal database division
** @param [r] divgb [const AjPStr] Genbank database division
** @return [AjBool] True if a known type
**
** @release 5.0.0
******************************************************************************/

AjBool ajSeqdivSetGb(AjPStr* Pdivi, const AjPStr divgb)
{
    static AjBool called = AJFALSE;
    const AjPStr divname = NULL;

    if(!called)
    {
	seqdivInit();
	called = ajTrue;
    }

    divname = ajTableFetchS(seqTableDivGb, divgb);

    if(!divname)		/* Genbank mixes division and class */
	divname = ajTableFetchS(seqTableClsGb, divgb);

    if(!divname)
	return ajFalse;

    ajStrAssignS(Pdivi, divname);

    return ajTrue;
    
}




/* @section retrieval ********************************************************
**
** Return values for sequence entry division
**
** @fdata [AjPStr]
** @fcategory use
**
** @nam3rule Get Retrieve value
** @nam4rule Embl Retrieve as an EMBL division name
** @nam4rule Gb   Retrieve as a GenBank division name
**
** @argrule Get divi [const AjPStr] Internal sequence division string
**
** @valrule * [const char*] Databank-specific division name
**
******************************************************************************/




/* @func ajSeqdivGetEmbl ******************************************************
**
** Returns the EMBL database division for a sequence
**
** @param [r] divi [const AjPStr] Internal database division
** @return [const char*] EMBL database division
**
** @release 5.0.0
******************************************************************************/

const char* ajSeqdivGetEmbl(const AjPStr divi)
{
    static AjBool called = AJFALSE;
    const SeqODivision *divdef = NULL;

    if(!called)
    {
	seqdivInit();
	called = ajTrue;
    }

    ajDebug("ajSeqdivGetEmbl '%S'\n", divi);

    if(ajStrGetLen(divi))
	divdef = ajTableFetchS(seqTableDiv, divi);

    if(!divdef)
	divdef = ajTableFetchS(seqTableDiv, seqDivisionDef);

    if(!divdef)
	return ajStrGetPtr(seqDivisionDef);

    ajDebug("ajSeqdivGetEmbl '%S' => '%s'\n",
	    divi, divdef->Embl);

    return divdef->Embl;
    
}




/* @func ajSeqdivGetGb ********************************************************
**
** Returns the Genbank database division for a sequence
**
** @param [r] divi [const AjPStr] Internal database division
** @return [const char*] Genbank database division
**
** @release 5.0.0
******************************************************************************/

const char* ajSeqdivGetGb(const AjPStr divi)
{
    static AjBool called = AJFALSE;
    const SeqODivision *divdef = NULL;

    if(!called)
    {
	seqdivInit();
	called = ajTrue;
    }

    ajDebug("ajSeqdivGetGb '%S'\n", divi);

    if(ajStrGetLen(divi))
	divdef = ajTableFetchS(seqTableDiv, divi);

    if(!divdef)
	divdef = ajTableFetchS(seqTableDiv, seqDivisionDef);

    if(!divdef)
	return ajStrGetPtr(seqDivisionDef);

    ajDebug("ajSeqdivGetGb '%S' => '%s'\n",
	    divi, divdef->Genbank);

    return divdef->Genbank;
    
}




/* @funcstatic seqdivInit *****************************************************
**
** Initialises the tables of sequence database divisions
**
** @return [void]
**
** @release 5.0.0
******************************************************************************/

static void seqdivInit(void)
{
    ajuint i;
    AjPStr keystr = NULL;
    AjPStr valstr = NULL;

    if(seqTableDiv)
	return;

    seqTableDiv = ajTablestrNewCase(16);
    seqTableDivEmbl = ajTablestrNewCase(16);
    seqTableDivDdbj = ajTablestrNewCase(16);
    seqTableDivGb = ajTablestrNewCase(16);

    seqDivisionDef = ajStrNewC(seqDivision[0].Name);

    for(i=0;seqDivision[i].Name;i++)
    {
	keystr = ajStrNewC(seqDivision[i].Name);

	ajTablePut(seqTableDiv, keystr, &seqDivision[i]);

	if(seqDivision[i].Embl[0])
	{
	    valstr = ajStrNewC(seqDivision[i].Embl);

	    ajTablePut(seqTableDivEmbl, valstr, keystr);
	}

	if(seqDivision[i].Ddbj[0])
	{
	    valstr = ajStrNewC(seqDivision[i].Ddbj);

	    ajTablePut(seqTableDivDdbj, valstr, keystr);
	}

	if(seqDivision[i].Genbank[0])
	{
	    valstr = ajStrNewC(seqDivision[i].Genbank);

	    ajTablePut(seqTableDivGb, valstr, keystr);
	}

    }

    return;
}




/* @datasection [AjPStr] Sequence entry molecule types ************************
**
** Sequence entry molecule type vocabularies
**
** @nam2rule Seqmol
**
******************************************************************************/




/* @section assignment ********************************************************
**
** Assign values for sequence entry molecule types
**
** @fdata [AjPStr]
** @fcategory assign
**
** @nam3rule Set Assign value
** @nam4rule Embl Assign with an EMBL molecule type name
** @nam4rule Gb   Assign with a GenBank molecule type name
**
** @argrule Set Pmol [AjPStr*] Target sequence molecule type string
** @argrule Embl molembl [const AjPStr] EMBL molecule type name
** @argrule Gb   molgb   [const AjPStr] Genbank molecule type name
**
** @valrule * [AjBool] True on success
**
******************************************************************************/




/* @func ajSeqmolSetEmbl ******************************************************
**
** Sets the internal molecule type for a sequence
**
** @param [u] Pmol [AjPStr*] Sequence object
** @param [r] molembl [const AjPStr] Molecule type
** @return [AjBool] True if a known type
**
** @release 5.0.0
******************************************************************************/

AjBool ajSeqmolSetEmbl(AjPStr* Pmol, const AjPStr molembl)
{
    static AjBool called = AJFALSE;
    const AjPStr molname = NULL;

    if(!called)
    {
	seqmolInit();
	called = ajTrue;
    }

    molname = ajTableFetchS(seqTableMolEmbl, molembl);

    if(!molname)
	return ajFalse;

    ajStrAssignS(Pmol, molname);

    return ajTrue;
}




/* @func ajSeqmolSetGb ********************************************************
**
** Sets the internal molecule type for a sequence
**
** @param [u] Pmol [AjPStr*] Sequence object
** @param [r] molgb [const AjPStr] Molecule type
** @return [AjBool] True if a known type
**
** @release 5.0.0
******************************************************************************/

AjBool ajSeqmolSetGb(AjPStr* Pmol, const AjPStr molgb)
{
    static AjBool called = AJFALSE;
    const AjPStr molname = NULL;

    if(!called)
    {
	seqmolInit();
	called = ajTrue;
    }

    molname = ajTableFetchS(seqTableMolGb, molgb);

    if(!molname)
	return ajFalse;

    ajStrAssignS(Pmol, molname);

    return ajTrue;
}




/* @section retrieval ********************************************************
**
** Return values for sequence molecule type division
**
** @fdata [AjPStr]
** @fcategory use
**
** @nam3rule Get Retrieve value
** @nam4rule Embl Retrieve as an EMBL molecule type name
** @nam4rule Gb   Retrieve as a GenBank molecule type name
**
** @argrule Get mol [const AjPStr] Internal sequence molecule type string
**
** @valrule * [const char*] Databank-specific molecule type name
**
******************************************************************************/




/* @func ajSeqmolGetEmbl ******************************************************
**
** Returns the EMBL molecule type for a sequence
**
** @param [r] mol [const AjPStr] Internal molecule type
** @return [const char*] EMBL molecule type
**
** @release 5.0.0
******************************************************************************/

const char* ajSeqmolGetEmbl(const AjPStr mol)
{
    static AjBool called = AJFALSE;
    const SeqOMolecule *moldef = NULL;

    if(!called)
    {
	seqmolInit();
	called = ajTrue;
    }

    ajDebug("ajSeqMoleculeGetEmbl '%S'\n", mol);

    if(ajStrGetLen(mol))
	moldef = ajTableFetchS(seqTableMol, mol);

    if(!moldef)
	moldef = ajTableFetchS(seqTableMol, seqMoleculeDef);

    if(!moldef)
	return ajStrGetPtr(seqMoleculeDef);

    ajDebug("ajSeqMoleculeGetEmbl '%S' => '%s'\n",
	    mol, moldef->Embl);

    return moldef->Embl;
}




/* @func ajSeqmolGetGb ********************************************************
**
** Returns the Genbank molecule type for a sequence
**
** @param [r] mol [const AjPStr] Internal molecule type
** @return [const char*] Genbank molecule type
**
** @release 5.0.0
******************************************************************************/

const char* ajSeqmolGetGb(const AjPStr mol)
{
    static AjBool called = AJFALSE;
    const SeqOMolecule *moldef = NULL;

    if(!called)
    {
	seqmolInit();
	called = ajTrue;
    }

    ajDebug("ajSeqMoleculeGetGb '%S'\n", mol);

    if(ajStrGetLen(mol))
	moldef = ajTableFetchS(seqTableMol, mol);

    if(!moldef)
	moldef = ajTableFetchS(seqTableMol, seqMoleculeDef);

    if(!moldef)
	return ajStrGetPtr(seqMoleculeDef);

    ajDebug("ajSeqMoleculeGetGb '%S' => '%s'\n",
	    mol, moldef->Genbank);

    return moldef->Genbank;
}




/* @funcstatic seqmolInit *****************************************************
**
** Initialises the tables of molecule types
**
** @return [void]
**
** @release 5.0.0
******************************************************************************/

static void seqmolInit(void)
{
    ajuint i;
    AjPStr keystr = NULL;
    AjPStr valstr = NULL;
 
    if(seqTableMol)
	return;

    seqTableMol = ajTablestrNewCase(16);
    seqTableMolEmbl = ajTablestrNewCase(16);
    seqTableMolDdbj = ajTablestrNewCase(16);
    seqTableMolGb = ajTablestrNewCase(16);

    seqMoleculeDef = ajStrNewC(seqMolecule[0].Name);

    for(i=0;seqMolecule[i].Name;i++)
    {
	if(!ajTableMatchC(seqTableMol, seqMolecule[i].Name)){
            keystr = ajStrNewC(seqMolecule[i].Name);

            ajTablePut(seqTableMol, keystr, &seqMolecule[i]);
        }

	if(seqMolecule[i].Embl[0])
	{
	    if(!ajTableMatchC(seqTableMolEmbl, seqMolecule[i].Embl))
            {
                valstr = ajStrNewC(seqMolecule[i].Embl);

                ajTablePut(seqTableMolEmbl, valstr, keystr);
            }
        }

	if(seqMolecule[i].Ddbj[0])
	{
	    if(!ajTableMatchC(seqTableMolDdbj, seqMolecule[i].Ddbj))
            {
                valstr = ajStrNewC(seqMolecule[i].Ddbj);

                ajTablePut(seqTableMolDdbj, valstr, keystr);
            }
	}

	if(seqMolecule[i].Genbank[0])
	{
	    if(!ajTableMatchC(seqTableMolGb, seqMolecule[i].Genbank))
            {
                valstr = ajStrNewC(seqMolecule[i].Genbank);

                ajTablePut(seqTableMolGb, valstr, keystr);
            }
	}

    }

    return;
}




#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif /* AJ_COMPILE_DEPRECATED_BOOK */




#ifdef AJ_COMPILE_DEPRECATED
/* @obsolete ajSeqNewC
** @rename ajSeqNewNameC
*/

__deprecated AjPSeq  ajSeqNewC(const char* seq, const char* name)
{
    return ajSeqNewNameC(seq, name);
}




/* @obsolete ajSeqNewStr
** @rename ajSeqNewNameS
*/

__deprecated AjPSeq  ajSeqNewStr(const AjPStr seq)
{
    AjPStr name = ajStrNew();

    return ajSeqNewNameS(seq, name);
}




/* @obsolete ajSeqNewRange
** @rename ajSeqNewRangeS
*/

__deprecated AjPSeq  ajSeqNewRange(const AjPStr seq,
				  ajint offset, ajint offend, AjBool rev)
{
    return ajSeqNewRangeS(seq, offset, offend, rev);
}




/* @obsolete ajSeqNewRangeCI
** @replace ajSeqNewRangeC (1,2,3,4,5/1,3,4,5)
*/

__deprecated AjPSeq  ajSeqNewRangeCI(const char* seq, ajint len,
				    ajint offset, ajint offend, AjBool rev)
{
    (void)len;

    return ajSeqNewRangeC(seq, offset, offend, rev);
}




/* @obsolete ajSeqNewL
** @rename ajSeqNewRes
*/
__deprecated AjPSeq  ajSeqNewL(size_t size)
{
    return ajSeqNewRes(size);
}




/* @obsolete ajSeqNewS
** @rename ajSeqNewSeq
*/

__deprecated AjPSeq  ajSeqNewS(const AjPSeq seq)
{
    return ajSeqNewSeq(seq);
}




/* @obsolete ajSeqAssAccC
** @rename ajSeqAssignAccC
*/

__deprecated void  ajSeqAssAccC(AjPSeq thys, const char* text)
{
    ajSeqAssignAccC(thys, text);

    return;
}




/* @obsolete ajSeqAssAcc
** @rename ajSeqAssignAccS
*/

__deprecated void  ajSeqAssAcc(AjPSeq thys, const AjPStr str)
{
    ajSeqAssignAccS(thys, str);

    return;
}




/* @obsolete ajSeqAssDescC
** @rename ajSeqAssignDescC
*/

__deprecated void  ajSeqAssDescC(AjPSeq thys, const char* txt)
{
    ajSeqAssignDescC(thys, txt);

    return;
}




/* @obsolete ajSeqAssDesc
** @rename ajSeqAssignDescS
*/

__deprecated void  ajSeqAssDesc(AjPSeq thys, const AjPStr str)
{
    ajSeqAssignDescS(thys, str);

    return;
}




/* @obsolete ajSeqAssEntryC
** @rename ajSeqAssignEntryC
*/

__deprecated void  ajSeqAssEntryC(AjPSeq thys, const char* text)
{
    ajSeqAssignEntryC(thys, text);

    return;
}




/* @obsolete ajSeqAssEntry
** @rename ajSeqAssignEntryS
*/

__deprecated void  ajSeqAssEntry(AjPSeq thys, const AjPStr str)
{
    ajSeqAssignEntryS(thys, str);

    return;
}




/* @obsolete ajSeqAssFileC
** @rename ajSeqAssignFileC
*/

__deprecated void  ajSeqAssFileC(AjPSeq thys, const char* text)
{
    ajSeqAssignFileC(thys, text);

    return;
}




/* @obsolete ajSeqAssFile
** @rename ajSeqAssignFileS
*/

__deprecated void  ajSeqAssFile(AjPSeq thys, const AjPStr str)
{
    ajSeqAssignFileS(thys, str);

    return;
}




/* @obsolete ajSeqAssFullC
** @rename ajSeqAssignFullC
*/

__deprecated void  ajSeqAssFullC(AjPSeq thys, const char* text)
{
    ajSeqAssignFullC(thys, text);

    return;
}




/* @obsolete ajSeqAssFull
** @rename ajSeqAssignFullS
*/

__deprecated void  ajSeqAssFull(AjPSeq thys, const AjPStr str)
{
    ajSeqAssignFullS(thys, str);

    return;
}




/* @obsolete ajSeqAssGiC
** @rename ajSeqAssignGiC
*/

__deprecated void  ajSeqAssGiC(AjPSeq thys, const char* text)
{
    ajSeqAssignGiC(thys, text);

    return;
}




/* @obsolete ajSeqAssGi
** @rename ajSeqAssignGiS
*/

__deprecated void  ajSeqAssGi(AjPSeq thys, const AjPStr str)
{
    ajSeqAssignGiS(thys, str);

    return;
}




/* @obsolete ajSeqAssName
** @rename ajSeqAssignNameS
*/

__deprecated void  ajSeqAssName(AjPSeq thys, const AjPStr str)
{
    ajSeqAssignNameS(thys, str);

    return;
}




/* @obsolete ajSeqAssNameC
** @rename ajSeqAssignNameS
*/

__deprecated void  ajSeqAssNameC(AjPSeq thys, const char* str)
{
    ajSeqAssignNameC(thys, str);

    return;
}




/* @obsolete ajSeqAssSeq
** @rename ajSeqAssignSeqS
*/
__deprecated void  ajSeqAssSeq(AjPSeq seq, const AjPStr str)
{
    ajSeqAssignSeqS(seq, str);

    return;
}




/* @obsolete ajSeqAssSeqC
** @rename ajSeqAssignSeqC
*/
__deprecated void  ajSeqAssSeqC (AjPSeq thys, const char* text)
{
    ajSeqAssignSeqC(thys, text);

    return;
}




/* @obsolete ajSeqAssSeqCI
** @replace ajSeqAssignSeqC (1,2,3/1,2)
*/
__deprecated void  ajSeqAssSeqCI (AjPSeq thys, const char* text, ajint ilen)
{
/*
    static ajint savelen;
    savelen = ilen;
*/
    (void) ilen;
    ajSeqAssignSeqC(thys, text);

    return;
}




/* @obsolete ajSeqAssSvC
** @rename ajSeqAssignSvC
*/
__deprecated void  ajSeqAssSvC(AjPSeq thys, const char* text)
{
    ajSeqAssignSvC(thys, text);

    return;
}




/* @obsolete ajSeqAssSv
** @rename ajSeqAssignSvS
*/
__deprecated void  ajSeqAssSv(AjPSeq thys, const AjPStr str)
{
    ajSeqAssignSvS(thys, str);

    return;
}




/* @obsolete ajSeqAssUfoC
** @rename ajSeqAssignUfoC
*/
__deprecated void  ajSeqAssUfoC(AjPSeq thys, const char* text)
{
    ajSeqAssignUfoC(thys, text);

    return;
}




/* @obsolete ajSeqAssUfo
** @rename ajSeqAssignUfoS
*/
__deprecated void  ajSeqAssUfo(AjPSeq thys, const AjPStr str)
{
    ajSeqAssignUfoS(thys, str);

    return;
}




/* @obsolete ajSeqAssUsaC
** @rename ajSeqAssignUsaC
*/
__deprecated void  ajSeqAssUsaC(AjPSeq thys, const char* text)
{
    ajSeqAssignUsaC(thys, text);

    return;
}




/* @obsolete ajSeqAssUsa
** @rename ajSeqAssignUsaS
*/
__deprecated void  ajSeqAssUsa(AjPSeq thys, const AjPStr str)
{
    ajSeqAssignUsaS(thys, str);

    return;
}




/* @obsolete ajSeqDefName
** @replace ajSeqSetName (1,2,ajFalse/1,2)
** @replace ajSeqSetNameMulti (1,2,ajTrue/1,2)

*/
__deprecated void  ajSeqDefName(AjPSeq thys, const AjPStr setname, AjBool multi)
{
    if(multi)
	ajSeqSetNameMulti(thys, setname);
    else
	ajSeqSetName(thys, setname);

    return;
}




/* @obsolete ajSeqSetRangeDir
** @replace ajSeqSetRange (1,2,3,ajFalse/1,2,3)
** @replace ajSeqSetRangeRev (1,2,3,ajTrue/1,2,3)
*/
__deprecated void  ajSeqSetRangeDir(AjPSeq seq,
				   ajint ibegin, ajint iend, AjBool rev)
{
    if(rev)
	ajSeqSetRangeRev(seq, ibegin, iend);
    else
	ajSeqSetRange(seq, ibegin, iend);

    return;
}




/* @obsolete ajSeqMod
** @rename ajSeqSetUnique
*/
__deprecated void  ajSeqMod(AjPSeq seq)
{
    ajSeqSetUnique(seq);

    return;
}




/* @obsolete ajSeqReplace
** @rename ajSeqAssignSeqS
*/
__deprecated void  ajSeqReplace(AjPSeq thys, const AjPStr seq)
{
    ajSeqAssignSeqS(thys, seq);

    return;
}




/* @obsolete ajSeqReplaceC
** @rename ajSeqAssignSeqC
*/
__deprecated void  ajSeqReplaceC(AjPSeq thys, const char* seq)
{
    ajSeqAssignSeqC(thys, seq);

    return;
}




/* @obsolete ajSeqMakeUsa
** @remove made static
*/

__deprecated void  ajSeqMakeUsa(AjPSeq thys, const AjPSeqin seqin)
{
    (void)seqin;
    seqMakeUsa(thys, &thys->Usa);

    return;
}




/* @obsolete ajSeqMakeUsaS
** @remove made static
*/

__deprecated void  ajSeqMakeUsaS(const AjPSeq thys,
				const AjPSeqin seqin, AjPStr* usa)
{
    (void)seqin;
    seqMakeUsa(thys, usa);

    return;
}




/* @obsolete ajSeqComplementOnly
** @rename ajSeqComplement
*/
__deprecated void  ajSeqComplementOnly(AjPSeq pthis)
{
    ajSeqComplement(pthis);

    return;
}




/* @obsolete ajSeqCompOnly
** @rename ajSeqComplement
*/
__deprecated void  ajSeqCompOnly(AjPSeq seq)
{
    ajSeqComplement(seq);

    return;
}




/* @obsolete ajSeqToLower
** @rename ajSeqFmtLower
*/

__deprecated void  ajSeqToLower(AjPSeq seq)
{
    ajStrFmtLower(&seq->Seq);

    return;
}




/* @obsolete ajSeqToUpper
** @rename ajSeqFmtUpper
*/

__deprecated void  ajSeqToUpper(AjPSeq seq)
{
    ajStrFmtUpper(&seq->Seq);

    return;
}




/* @obsolete ajSeqFill
** @rename ajSeqGapFill
*/
__deprecated ajint  ajSeqFill(AjPSeq seq, ajint len)
{
    ajint ilen;
    ilen = ajSeqGetLen(seq);
    ajSeqGapFill(seq, len);

    return len-ilen;
}




/* @obsolete ajSeqReverse
** @rename ajSeqReverseDo
*/
__deprecated AjBool  ajSeqReverse(AjPSeq seq)
{
    if(!seq->Rev)
	return ajFalse;

    ajSeqReverseDo(seq);

    return ajTrue;
}




/* @obsolete ajSeqRevOnly
** @rename ajSeqReverseOnly
*/

__deprecated void  ajSeqRevOnly(AjPSeq seq)
{
    ajSeqReverseOnly(seq);

    return;
}




/* @obsolete ajSeqGetAcc
** @rename ajSeqGetAccS
*/
__deprecated const AjPStr  ajSeqGetAcc(const AjPSeq seq)
{
    return ajSeqGetAccS(seq);
}




/* @obsolete ajSeqBegin
** @rename ajSeqGetBegin
*/

__deprecated ajint  ajSeqBegin(const AjPSeq seq)
{
    return ajSeqGetBegin(seq);
}




/* @obsolete ajSeqTrueBegin
** @rename ajSeqGetBeginTrue
*/
__deprecated ajint  ajSeqTrueBegin(const AjPSeq seq)
{
    return ajSeqGetBeginTrue(seq);
}




/* @obsolete ajSeqGetDesc
** @rename ajSeqGetDescS
*/

__deprecated const AjPStr  ajSeqGetDesc(const AjPSeq seq)

{
    return seq->Desc;
}




/* @obsolete ajSeqEnd
** @rename ajSeqGetEnd
*/
__deprecated ajint  ajSeqEnd(const AjPSeq seq)
{
    return ajSeqGetEnd(seq);
}




/* @obsolete ajSeqTrueEnd
** @rename ajSeqGetEndTrue
*/
__deprecated ajint  ajSeqTrueEnd(const AjPSeq seq)
{
    return ajSeqGetEndTrue(seq);
}




/* @obsolete ajSeqGetEntry
** @rename ajSeqGetEntryS
*/
__deprecated const AjPStr  ajSeqGetEntry(const AjPSeq seq)
{
    return ajSeqGetEntryS(seq);
}




/* @obsolete ajSeqCopyFeat
** @rename ajSeqGetFeatCopy
*/
__deprecated AjPFeattable  ajSeqCopyFeat(const AjPSeq seq)
{
    return ajFeattableNewFtable(seq->Fttable);
}




/* @obsolete ajSeqGetGi
** @rename ajSeqGetGiS
*/
__deprecated const AjPStr  ajSeqGetGi(const AjPSeq seq)
{
    return seq->Gi;
}




/* @obsolete ajSeqLen
** @rename ajSeqGetLen
*/
__deprecated ajint  ajSeqLen(const AjPSeq seq)
{
    return ajStrGetLen(seq->Seq);
}




/* @obsolete ajSeqTrueLen
** @rename ajSeqGetLenTrue
*/
__deprecated ajint  ajSeqTrueLen(const AjPSeq seq)
{
    return ajSeqGetLenTrue(seq);
}




/* @obsolete ajSeqName
** @rename ajSeqGetNameC
*/
__deprecated const char*  ajSeqName(const AjPSeq seq)
{
    return ajStrGetPtr(seq->Name);
}




/* @obsolete ajSeqGetName
** @rename ajSeqGetNameS
*/
__deprecated const AjPStr  ajSeqGetName(const AjPSeq seq)
{
    return seq->Name;
}




/* @obsolete ajSeqOffend
** @rename ajSeqGetOffend
*/
__deprecated ajint  ajSeqOffend(const AjPSeq seq)
{
    return ajSeqGetOffend(seq);
}




/* @obsolete ajSeqOffset
** @rename ajSeqGetOffset
*/
__deprecated ajint  ajSeqOffset(const AjPSeq seq)
{
    return ajSeqGetOffset(seq);
}




/* @obsolete ajSeqGetReverse
** @rename ajSeqGetRev
*/
__deprecated AjBool  ajSeqGetReverse(const AjPSeq seq)
{
    return seq->Rev;
}




/* @obsolete ajSeqGetReversed
** @rename ajSeqIsReversed
*/
__deprecated AjBool  ajSeqGetReversed(const AjPSeq seq)
{
    return seq->Reversed;
}




/* @obsolete ajSeqStr
** @rename ajSeqGetSeqS
*/
__deprecated const AjPStr  ajSeqStr(const AjPSeq seq)
{
    return ajSeqGetSeqS(seq);
}




/* @obsolete ajSeqChar
** @rename ajSeqGetSeqC
*/
__deprecated const char*  ajSeqChar(const AjPSeq seq)
{
    if(!seq)
	return "";

    return ajStrGetPtr(seq->Seq);
}




/* @obsolete ajSeqCharCopy
** @rename ajSeqGetSeqCopyC
*/
__deprecated char*  ajSeqCharCopy(const AjPSeq seq)
{
    return ajCharNewS(seq->Seq);
}




/* @obsolete ajSeqStrCopy
** @rename ajSeqGetSeqCopyS
*/
__deprecated AjPStr  ajSeqStrCopy(const AjPSeq seq)
{
    return ajSeqGetSeqCopyS(seq);
}




/* @obsolete ajSeqCharCopyL
** @replace ajSeqGetSeqCopyC (1,2/1,ajSeqGetLen[2])
*/
__deprecated char*  ajSeqCharCopyL(const AjPSeq seq, size_t size)
{
    return ajCharNewResS(seq->Seq, size);
}




/* @obsolete ajSeqGetSv
** @rename ajSeqGetSvS
*/
__deprecated const AjPStr  ajSeqGetSv(const AjPSeq seq)
{
    return seq->Sv;
}




/* @obsolete ajSeqGetTax
** @rename ajSeqGetTaxS
*/
__deprecated const AjPStr  ajSeqGetTax(const AjPSeq seq)
{
    return seq->Tax;
}




/* @obsolete ajSeqGetUsa
** @rename ajSeqGetUsaS
*/
__deprecated const AjPStr  ajSeqGetUsa(const AjPSeq seq)
{
    return ajSeqGetUsaS(seq);
}




/* @obsolete ajSeqRev
** @rename ajSeqIsReversed
*/
__deprecated AjBool  ajSeqRev(const AjPSeq seq)
{
    return ajSeqIsReversed(seq);
}




/* @obsolete ajSeqNum
** @rename ajSeqConvertNum
*/

__deprecated AjBool  ajSeqNum(const AjPSeq seq, const AjPSeqCvt cvt,
			     AjPStr* numseq)
{
    return ajSeqConvertNum(seq, cvt, numseq);
}




/* @obsolete ajSeqCheckGcg
** @rename ajSeqCalcCheckgcg
*/
__deprecated ajint  ajSeqCheckGcg(const AjPSeq seq)
{
    return ajSeqCalcCheckgcg(seq);
}




/* @obsolete ajSeqCount
** @rename ajSeqCalcCount
*/
__deprecated void  ajSeqCount(const AjPSeq seq, ajint* b)
{
    ajSeqCalcCount(seq, b);

    return;
}




/* @obsolete ajSeqPos
** @replace ajCvtSposToPos (1,2/'ajSeqGetLen[1]',2)
*/

__deprecated ajint  ajSeqPos(const AjPSeq seq, ajint ipos)
{
    return 1+ajCvtSposToPosStart(ajSeqGetLen(seq), 0, ipos);
}




/* @obsolete ajSeqPosI
** @replace ajCvtSposToPosStart (1,2,3/'ajSeqGetLen[1]',2,3)
*/
__deprecated ajint  ajSeqPosI(const AjPSeq seq, ajint imin, ajint ipos)
{
    return 1+ajCvtSposToPosStart(ajSeqGetLen(seq), imin, ipos);
}




/* @obsolete ajSeqPosII
** @rename ajCvtSposToPosStart
*/

__deprecated ajint  ajSeqPosII(ajint ilen, ajint imin, ajint ipos)
{
    ajint jpos;

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

    ajDebug("ajSeqPosII (ilen: %d imin: %d ipos: %d) = %d\n",
	    ilen, imin, ipos, jpos);

    return jpos;
}




/* @obsolete ajSeqTruePos
** @rename ajSeqCalcTruepos
*/
__deprecated ajint  ajSeqTruePos(const AjPSeq thys, ajint ipos)
{
    return ajSeqCalcTruepos(thys, ipos);
}




/* @obsolete ajSeqTruePosI
** @rename ajSeqCalcTrueposMin
*/

__deprecated ajint  ajSeqTruePosI(const AjPSeq thys, ajint imin, ajint ipos)
{
    return ajSeqCalcTrueposMin(thys, imin, ipos);
}




/* @obsolete ajSeqTruePosII
** @rename ajCvtSposToPosStart
*/
__deprecated ajint  ajSeqTruePosII(ajint ilen, ajint imin, ajint ipos)
{
    return ajCvtSposToPosStart(ilen, imin, ipos);
}




/* @obsolete ajSeqGapCount
* @rename ajSeqCountGaps
*/

__deprecated ajint  ajSeqGapCount(const AjPSeq seq)
{
    return ajSeqCountGaps(seq);
}




/* @obsolete ajSeqTraceT
** @rename ajSeqTraceTitle
*/
__deprecated void  ajSeqTraceT(const AjPSeq seq, const char* title)
{
    ajSeqTraceTitle(seq, title);

    return;
}




/* @obsolete ajSeqallReverse
** @remove sequence processed separately after ajSeqallNext
*/
__deprecated void  ajSeqallReverse(AjPSeqall seq)
{
    ajint ibegin;
    ajint iend;

    ajDebug("ajSeqallReverse len: %d Begin: %d End: %d\n",
	    ajSeqallGetseqLen(seq), seq->Begin, seq->End);

    ibegin = seq->Begin;
    iend   = seq->End;

    seq->End   = -(ibegin);
    seq->Begin = -(iend);

    ajSeqReverseDo(seq->Seq);

    ajDebug("  all result len: %d Begin: %d End: %d\n",
	    ajSeqallGetseqLen(seq), seq->Begin, seq->End);

    return;
}




/* @obsolete ajSeqallToLower
** @remove done when sequence is read
*/
__deprecated void  ajSeqallToLower(AjPSeqall seqall)
{
    ajSeqFmtLower(seqall->Seq);

    return;
}




/* @obsolete ajSeqallToUpper
** @remove done when sequence is read
*/
__deprecated void  ajSeqallToUpper(AjPSeqall seqall)
{
    ajSeqFmtUpper(seqall->Seq);

    return;
}




/* @obsolete ajSeqallBegin
** @rename ajSeqallGetseqBegin
*/
__deprecated ajint  ajSeqallBegin(const AjPSeqall seq)
{
    return ajSeqallGetseqBegin(seq);
}




/* @obsolete ajSeqallEnd
** @rename ajSeqallGetseqEnd
*/
__deprecated ajint  ajSeqallEnd(const AjPSeqall seq)
{
    return ajSeqallGetseqEnd(seq);
}




/* @obsolete ajSeqallLen
** @rename ajSeqallGetseqLen
*/
__deprecated ajint  ajSeqallLen(const AjPSeqall seqall)
{
    return ajSeqallGetseqLen(seqall);
}




/* @obsolete ajSeqallGetNameSeq
** @rename ajSeqallGetseqName
*/

__deprecated const AjPStr  ajSeqallGetNameSeq(const AjPSeqall seq)
{
    return ajSeqallGetseqName(seq);
}




/* @obsolete ajSeqallGetRange
** @rename   ajSeqallGetseqRange
*/

__deprecated ajint  ajSeqallGetRange(const AjPSeqall seq,
				    ajint* begin, ajint* end)
{
    return ajSeqallGetseqRange(seq, begin, end);
}




/* @obsolete ajSeqsetToLower
** @rename ajSeqsetFmtLower
*/
__deprecated void  ajSeqsetToLower(AjPSeqset seqset)
{
    ajSeqsetFmtLower(seqset);

    return;
}




/* @obsolete ajSeqsetToUpper
** @rename ajSeqsetFmtUpper
*/
__deprecated void  ajSeqsetToUpper(AjPSeqset seqset)
{
    ajSeqsetFmtUpper(seqset);

    return;
}




/* @obsolete ajSeqsetBegin
** @rename ajSeqsetGetBegin
*/
__deprecated ajint  ajSeqsetBegin(const AjPSeqset seq)
{
    return ajSeqsetGetBegin(seq);
}




/* @obsolete ajSeqsetEnd
** @rename ajSeqsetGetEnd
*/
__deprecated ajint  ajSeqsetEnd(const AjPSeqset seq)
{
    return ajSeqsetGetEnd(seq);
}




/* @obsolete ajSeqsetLen
** @rename ajSeqsetGetLen
*/

__deprecated ajint  ajSeqsetLen(const AjPSeqset seq)
{
    return ajSeqsetGetLen(seq);
}




/* @obsolete ajSeqsetGetName
** @rename ajSeqsetGetNameS
*/
__deprecated const AjPStr  ajSeqsetGetName(const AjPSeqset thys)
{
    return ajSeqsetGetNameS(thys);
}




/* @obsolete ajSeqsetGetSeqArray
** @rename ajSeqsetGetSeqarray
*/
__deprecated AjPSeq*  ajSeqsetGetSeqArray(const AjPSeqset thys)
{
    return ajSeqsetGetSeqarray(thys);
}




/* @obsolete ajSeqsetSize
** @rename ajSeqsetGetSize
*/

__deprecated ajint  ajSeqsetSize(const AjPSeqset seq)
{
    return ajSeqsetGetSize(seq);
}




/* @obsolete ajSeqsetTotweight
** @rename ajSeqsetGetTotweight
*/
__deprecated float  ajSeqsetTotweight(const AjPSeqset seq)
{
    return ajSeqsetGetTotweight(seq);
}




/* @obsolete ajSeqsetAcc
** @rename ajSeqsetGetseqAccS
*/
__deprecated const AjPStr  ajSeqsetAcc(const AjPSeqset seq, ajint i)
{
    return ajSeqsetGetseqAccS(seq, i);
}




/* @obsolete ajSeqsetName
** @rename ajSeqsetGetseqNameS
*/
__deprecated const AjPStr  ajSeqsetName(const AjPSeqset seq, ajint i)
{
    return ajSeqsetGetseqNameS(seq, i);
}




/* @obsolete ajSeqsetGetSeq
** @rename ajSeqsetGetseqSeq
*/
__deprecated const AjPSeq  ajSeqsetGetSeq(const AjPSeqset thys, ajint i)
{
    return ajSeqsetGetseqSeq(thys, i);
}




/* @obsolete ajSeqsetSeq
** @rename ajSeqsetGetseqSeqC
*/
__deprecated const char*   ajSeqsetSeq(const AjPSeqset thys, ajint i)
{
    return ajSeqsetGetseqSeqC(thys, i);
}




/* @obsolete ajSeqsetWeight
** @rename ajSeqsetGetseqWeight
*/
__deprecated float  ajSeqsetWeight(const AjPSeqset seq, ajint i)
{
    return ajSeqsetGetseqWeight(seq, i);
}




/* @obsolete ajSeqNumS
** @rename ajSeqstrConvertNum
*/

__deprecated AjBool  ajSeqNumS(const AjPStr seqstr,
			     const AjPSeqCvt cvt,
			     AjPStr* numseq)
{
    return ajSeqstrConvertNum(seqstr, cvt, numseq);
}




/* @obsolete ajSeqCvtTrace
** @rename ajSeqcvtTrace
*/
__deprecated void  ajSeqCvtTrace(const AjPSeqCvt cvt)
{
    ajSeqcvtTrace(cvt);

    return;
}




/* @obsolete ajSeqCvtNewZero
** @rename ajSeqcvtNewC
*/
__deprecated AjPSeqCvt  ajSeqCvtNewZero(const char* bases)
{
    return ajSeqcvtNewC(bases);
}




/* @obsolete ajSeqCvtNew
** @rename ajSeqcvtNewEndC
*/
__deprecated AjPSeqCvt  ajSeqCvtNew(const char* bases)
{
    return ajSeqcvtNewEndC(bases);
}




/* @obsolete ajSeqCvtNewText
** @rename ajSeqcvtNewNumberC
*/
__deprecated AjPSeqCvt  ajSeqCvtNewText(const char* bases)
{
    return ajSeqcvtNewNumberC(bases);
}




/* @obsolete ajSeqCvtNewZeroS
** @rename ajSeqcvtNewStr
*/
__deprecated AjPSeqCvt  ajSeqCvtNewZeroS (const AjPPStr bases, ajint n)
{
    return ajSeqcvtNewStr(bases, n);
}




/* @obsolete ajSeqCvtNewZeroSS
** @rename ajSeqcvtNewStrAsym
*/
__deprecated AjPSeqCvt  ajSeqCvtNewZeroSS (const AjPPStr bases, int n, 
			     const AjPPStr rbases, int rn)
{
    return ajSeqcvtNewStrAsym (bases, n, rbases, rn);
}




/* @obsolete ajSeqCvtDel
** @rename ajSeqcvtDel
*/
__deprecated void  ajSeqCvtDel (AjPSeqCvt* Pcvt)
{
    ajSeqcvtDel(Pcvt);

    return;
}




/* @obsolete ajSeqCvtK
** @rename ajSeqcvtGetCodeK
*/
__deprecated ajint  ajSeqCvtK(const AjPSeqCvt cvt, char ch)
{
    return ajSeqcvtGetCodeK(cvt, ch);
}




/* @obsolete ajSeqCvtKS
** @rename ajSeqcvtGetCodeS
*/
__deprecated ajint  ajSeqCvtKS (const AjPSeqCvt cvt, const AjPStr ch)
{
    return(ajSeqcvtGetCodeS(cvt, ch));
}




/* @obsolete ajSeqCvtKSRow
** @rename ajSeqcvtGetCodeS
*/
__deprecated ajint  ajSeqCvtKSRow (const AjPSeqCvt cvt, const AjPStr ch)
{
    return(ajSeqcvtGetCodeS(cvt, ch));
}




/* @obsolete ajSeqCvtKSColumn
** @rename ajSeqcvtGetCodeAsymS
*/
__deprecated ajint  ajSeqCvtKSColumn (const AjPSeqCvt cvt, const AjPStr ch)
{
    return ajSeqcvtGetCodeAsymS(cvt, ch);
}




/* @obsolete ajSeqCvtLen
** @rename ajSeqcvtGetLen
*/
__deprecated ajint  ajSeqCvtLen(const AjPSeqCvt cvt)
{
    return ajSeqcvtGetLen(cvt);
}




/* @obsolete ajIsAccession
** @rename ajSeqtestIsAccession
*/
__deprecated AjBool  ajIsAccession(const AjPStr accnum)
{
    return ajSeqtestIsAccession(accnum);
}




/* @obsolete ajIsSeqversion
** @rename ajSeqtestIsSeqversion
*/
__deprecated const AjPStr  ajIsSeqversion(const AjPStr sv)
{
    return ajSeqtestIsSeqversion(sv);
}




/* @obsolete ajSeqMW
** @rename ajSeqstrCalcMolwt
*/
__deprecated float  ajSeqMW(const AjPStr seq)
{
    return ajSeqstrCalcMolwt(seq);
}




/* @obsolete ajSeqGapCountS
** @rename ajSeqstrCountGaps
*/
__deprecated ajint  ajSeqGapCountS(const AjPStr str)
{
    return ajSeqstrCountGaps(str);
}




/* @obsolete ajSeqstrComplementOnly
** @rename ajSeqstrComplement
*/
__deprecated void  ajSeqstrComplementOnly(AjPStr* pthis)
{
    ajSeqstrComplement(pthis);
    return;
}




/* @obsolete ajSeqCompOnlyStr
** @rename ajSeqstrComplement
*/
__deprecated void  ajSeqCompOnlyStr(AjPStr* pthis)
{
    ajSeqstrComplement(pthis);

    return;
}




/* @obsolete ajSeqReverseStr
** @rename ajSeqstrReverse
*/
__deprecated void  ajSeqReverseStr(AjPStr* pthis)
{
    ajSeqstrReverse(pthis);

    return;
}




/* @obsolete ajSeqCrc
** @rename ajSeqstrCalcCrc
*/
__deprecated ajuint  ajSeqCrc(const AjPStr seq)
{
    (void) seq;				/* just so it is used */

    return 0;
}




/* @obsolete ajSeqGapStandardS
** @remove only used internally
*/
__deprecated void  ajSeqGapStandardS(AjPStr thys, char gapch)
{
    char newgap = '-';
    static char testchars[] = "-~.? "; /* all known gap characters */
    char *testgap;

    testgap = testchars;

    if(gapch)
	newgap = gapch;

    /*ajDebug("ajSeqGapStandardS '%c'=>'%c' '%S'\n",
            gapch, newgap, thys->Seq);*/

    while(*testgap)
    {
	if(newgap != *testgap)
	{
	    ajStrExchangeKK(&thys, *testgap, newgap);
	    /*ajDebug(" ajSeqGapStandardS replaced         '%c'=>'%c' '%S'\n",
		    *testgap, newgap, thys);*/
	}

        testgap++;
    }

    return;
}




/* @obsolete ajSeqCalcCrc
** @remove use 64bit call
*/
__deprecated ajuint  ajSeqCalcCrc(const AjPSeq seq)
{
    (void) seq;				/* just so it is used */

    return 0;
}

#endif /* AJ_COMPILE_DEPRECATED */

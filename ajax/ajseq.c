/******************************************************************************
** @source AJAX SEQ (sequence) functions
**
** These functions control all aspects of AJAX sequence
** reading and writing and include simple utilities.
**
** @author Copyright (C) 1998 Peter Rice
** @version 1.0
** @modified Jun 25 pmr First version
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
******************************************************************************/

#include "ajax.h"
#include <limits.h>
#include <float.h>

static AjPStr seqVersionAccnum = NULL;

static AjPStr seqTempUsa = NULL;


static void seqMakeUsa(const AjPSeq thys, AjPStr* usa);
static AjBool seqDateSet(AjPTime* date, const AjPStr datestr);

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

typedef struct SeqSClass {
  const char* Name;
  const char* Embl;
  const char* Ddbj;
  const char* Genbank;
  const char* Desc;
} SeqOClass;

static SeqOClass seqClass[] = {
/*   Name      Embl   DDBJ   Genbank Description */
    {"STD", "STD", "", "", "Standard (all other entries)"},
    {"CON", "CON", "CON", "CON", "Constructed from segment sequences"},
    {"ANN", "ANN", "", "", "CON entries with own annotation"},
    {"PAT", "PAT", "PAT", "PAT", "Patent"},
    {"EST", "EST", "EST", "EST", "Expressed Sequence Tag"},
    {"GSS", "GSS", "GSS", "GSS", "Genome Survey Sequence"},
    {"HTC", "HTC", "HTC", "", "High Thoughput CDNA sequencing"},
    {"HTG", "HTG", "HTG", "HTG", "High Thoughput Genome sequencing"},
    {"MGA", "MGA", "", "", "Mass Genome Annotation"},
    {"WGS", "WGS", "", "", "Whole Genome Shotgun"},
    {"TPA", "TPA", "TPA", "", "Third Party Annotation"},  /* 2 types in DDBJ */
    {"STS", "STS", "STS", "STS", "Sequence Tagged Site"},
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

typedef struct SeqSDivision {
  const char* Name;
  const char* Embl;
  const char* Ddbj;
  const char* Genbank;
  const char* Desc;
} SeqODivision;

static SeqODivision seqDivision[] = {
/*   Name      Embl   DDBJ   Genbank Description */
    {"default","UNC", "UNA", "UNA",  "Unclassified"},
    {"UNC",    "UNC", "",    "",     "Unclassified"},
    {"UNA",    "UNC", "UNA", "UNA",  "Unannotated"},
    {"PHG",    "PHG", "PHG", "PHG",  "Bacteriophage"},
    {"ENV",    "ENV", "",    "",     "Environmental Sample"},
    {"PLN",    "PLN", "PLN", "PLN",  "Plant"},
    {"FUN",    "FUN", "",    "PLN",  "Fungal"},
    {"HUM",    "HUM", "HUM", "PRI",  "Human"},
    {"INV",    "INV", "INV", "INV",  "Invertebrate"},
    {"MAM",    "MAM", "MAM", "MAM",  "Other Mammal"},
    {"VRT",    "VRT", "VRT", "VRT",  "Other Vertebrate"},
    {"MUS",    "MUS", "ROD", "ROD",  "Mus musculus"},
    {"PRO",    "PRO", "BCT", "BCT",  "Prokaryote"},
    {"ROD",    "ROD", "ROD", "ROD",  "Other Rodent"},
    {"SYN",    "SYN", "SYN", "SYN",  "Synthetic and chimeric"},
    {"TGN",    "TGN", "",    "",     "Transgenic"},
    {"VRL",    "VRL", "VRL", "VRL",  "Viral"},
    {"MAMPRI", "MAM", "",    "PRI",  "Primate"},
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

typedef struct SeqSMolecule {
  const char* Name;
  const char* Embl;
  const char* Ddbj;
  const char* Genbank;
  const char* Desc;
} SeqOMolecule;

static SeqOMolecule seqMolecule[] = {
/*   Name      Embl   DDBJ   Genbank Description */
    {"unassigned DNA", "unassigned DNA", "DNA", "DNA", "unassigned DNA"},
    {"genomic DNA", "genomic DNA", "DNA", "DNA", "genomic DNA"},
    {"genomic RNA", "genomic RNA", "DNA", "RNA", "genomic RNA"},
    {"mRNA", "mRNA", "RNA", "RNA", "mRNA"},
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
** @nam4rule NewSeq Constructor with all details in an existing sequence
**
** @suffix Res [size_t] Reserved length
** @suffix S [const AjPSeq] Source sequence
** @suffix C [const char*] Source sequence
**
** @argrule Res size [size_t] Reserved size including the terminating NULL
** @argrule C txt [const char*] Source sequence
** @argrule S str [const AjPStr] Source sequence
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
** @@
******************************************************************************/

AjPSeq ajSeqNewNameC(const char* txt, const char* name)
{
    AjPSeq pthis;

    AJNEW0(pthis);

    ajStrAssignC(&pthis->Name, name);
    pthis->Acc  = ajStrNew();
    pthis->Sv   = ajStrNew();
    pthis->Gi   = ajStrNew();
    pthis->Tax  = ajStrNew();
    pthis->Type = ajStrNew();
    pthis->Db   = ajStrNew();
    pthis->Full = ajStrNew();
    pthis->Desc = ajStrNew();
    pthis->Doc  = ajStrNew();
    pthis->Usa  = ajStrNew();
    pthis->Ufo  = ajStrNew();

    pthis->Formatstr = ajStrNew();
    pthis->Filename  = ajStrNew();
    pthis->Entryname = ajStrNew();
    pthis->TextPtr   = ajStrNew();

    ajStrAssignC(&pthis->Seq, txt);

    pthis->Rev      = ajFalse;
    pthis->Reversed = ajFalse;
    pthis->Trimmed  = ajFalse;

    pthis->EType  = 0;
    pthis->Format = 0;
    pthis->Begin  = 0;
    pthis->End    = 0;
    pthis->Offset = 0;
    pthis->Offend = 0;
    pthis->Weight = 1.0;

    pthis->Acclist = ajListstrNew();
    pthis->Keylist = ajListstrNew();
    pthis->Taxlist = ajListstrNew();

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
** @@
******************************************************************************/

AjPSeq ajSeqNewNameS(const AjPStr str, const AjPStr name)
{
    AjPSeq pthis;

    AJNEW0(pthis);

    pthis->Name = ajStrNewS(name);
    pthis->Acc  = ajStrNew();
    pthis->Sv   = ajStrNew();
    pthis->Gi   = ajStrNew();
    pthis->Tax  = ajStrNew();
    pthis->Type = ajStrNew();
    pthis->Db   = ajStrNew();
    pthis->Full = ajStrNew();
    pthis->Desc = ajStrNew();
    pthis->Doc  = ajStrNew();
    pthis->Usa  = ajStrNew();
    pthis->Ufo  = ajStrNew();

    pthis->Formatstr = ajStrNew();
    pthis->Filename  = ajStrNew();
    pthis->Entryname = ajStrNew();
    pthis->TextPtr   = ajStrNew();

    ajStrAssignS(&pthis->Seq, str);

    pthis->Rev      = ajFalse;
    pthis->Reversed = ajFalse;
    pthis->Trimmed  = ajFalse;

    pthis->EType  = 0;
    pthis->Format = 0;
    pthis->Begin  = 0;
    pthis->End    = 0;
    pthis->Offset = 0;
    pthis->Offend = 0;
    pthis->Weight = 1.0;

    pthis->Acclist = ajListstrNew();
    pthis->Keylist = ajListstrNew();
    pthis->Taxlist = ajListstrNew();

    return pthis;
}





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


/* @func ajSeqNewRangeC *******************************************************
**
** Creates and initialises a sequence object with a specified existing
** sequence as a string,and provides offsets, and direction.
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
** @@
******************************************************************************/

AjPSeq ajSeqNewRangeC(const char* txt,
		       ajint offset, ajint offend, AjBool rev)
{
    AjPSeq pthis;

    AJNEW0(pthis);

    ajDebug("ajSeqNewRangeC %d %d %B\n",
	    offset, offend, rev);
    pthis->Name = ajStrNew();
    pthis->Acc  = ajStrNew();
    pthis->Sv   = ajStrNew();
    pthis->Gi   = ajStrNew();
    pthis->Tax  = ajStrNew();
    pthis->Type = ajStrNew();
    pthis->Db   = ajStrNew();
    pthis->Full = ajStrNew();
    pthis->Desc = ajStrNew();
    pthis->Doc  = ajStrNew();
    pthis->Usa  = ajStrNew();
    pthis->Ufo  = ajStrNew();

    pthis->Formatstr = ajStrNew();
    pthis->Filename  = ajStrNew();
    pthis->Entryname = ajStrNew();
    pthis->TextPtr   = ajStrNew();

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

    pthis->Acclist = ajListstrNew();
    pthis->Keylist = ajListstrNew();
    pthis->Taxlist = ajListstrNew();

    ajDebug("ajSeqNewRangeC rev:%B offset:%d/%d offend:%d/%d\n",
	    rev, offset, pthis->Offset, offend, pthis->Offend);
    
    return pthis;
}




/* @func ajSeqNewRangeS *******************************************************
**
** Creates and initialises a sequence object with a specified existing
** sequence as a string,and provides offsets, and direction.
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
** @@
******************************************************************************/

AjPSeq ajSeqNewRangeS(const AjPStr str,
			ajint offset, ajint offend, AjBool rev)
{
    return ajSeqNewRangeC(ajStrGetPtr(str), offset, offend, rev);
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

/* @func ajSeqNewRes **********************************************************
**
** Creates and initialises a sequence object with a specified sequence length.
**
** @param [r] size [size_t] Reserved space for the sequence, including
**                          a trailing null character.
** @return [AjPSeq] New sequence object.
** @@
******************************************************************************/

AjPSeq ajSeqNewRes(size_t size)
{
    AjPSeq pthis;

    AJNEW0(pthis);

    pthis->Name = ajStrNew();
    pthis->Acc  = ajStrNew();
    pthis->Sv   = ajStrNew();
    pthis->Gi   = ajStrNew();
    pthis->Tax  = ajStrNew();
    pthis->Type = ajStrNew();
    pthis->Db   = ajStrNew();
    pthis->Full = ajStrNew();
    pthis->Desc = ajStrNew();
    pthis->Doc  = ajStrNew();
    pthis->Usa  = ajStrNew();
    pthis->Ufo  = ajStrNew();

    pthis->Formatstr = ajStrNew();
    pthis->Filename  = ajStrNew();
    pthis->Entryname = ajStrNew();
    pthis->TextPtr   = ajStrNew();
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
    pthis->Acclist = ajListstrNew();
    pthis->Keylist = ajListstrNew();
    pthis->Taxlist = ajListstrNew();
    pthis->Cmtlist = ajListstrNew();
    pthis->Xreflist = ajListstrNew();
    pthis->Reflist = ajListNew();

    return pthis;
}

/* @obsolete ajSeqNewL
** @rename ajSeqNewRes
*/
__deprecated AjPSeq  ajSeqNewL(size_t size)
{
    return ajSeqNewRes(size);
}





/* @func ajSeqNewSeq **********************************************************
**
** Creates and initialises a sequence object with a specified existing
** sequence.
**
** @param [r] seq [const AjPSeq] Old sequence object
** @return [AjPSeq] New sequence object.
** @@
******************************************************************************/

AjPSeq ajSeqNewSeq(const AjPSeq seq)
{
    AjPSeq pthis;

    AJNEW0(pthis);

    ajStrAssignS(&pthis->Name, seq->Name);
    ajStrAssignS(&pthis->Acc, seq->Acc);
    ajStrAssignS(&pthis->Sv, seq->Sv);
    ajStrAssignS(&pthis->Gi, seq->Gi);
    ajStrAssignS(&pthis->Tax, seq->Tax);
    ajStrAssignS(&pthis->Type, seq->Type);

    pthis->EType  = seq->EType;

    ajStrAssignS(&pthis->Db, seq->Db);
    ajStrAssignS(&pthis->Setdb, seq->Setdb);
    ajStrAssignS(&pthis->Full, seq->Full);
    ajStrAssignS(&pthis->Desc, seq->Desc);
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

    ajStrAssignS(&pthis->Usa, seq->Usa);
    ajStrAssignS(&pthis->Ufo, seq->Ufo);
    ajStrAssignS(&pthis->Formatstr, seq->Formatstr);
    pthis->Format = seq->Format;

    ajStrAssignS(&pthis->Filename, seq->Filename);
    ajStrAssignS(&pthis->Entryname, seq->Entryname);

    if(seq->TextPtr)
	ajStrAssignS(&pthis->TextPtr, seq->TextPtr);


    pthis->Acclist = ajListstrNewList(seq->Acclist);

    pthis->Keylist = ajListstrNewList(seq->Keylist);

    pthis->Taxlist = ajListstrNewList(seq->Taxlist);

    ajStrAssignS(&pthis->Seq, seq->Seq);
    if (seq->Fttable)
	pthis->Fttable = ajFeattableCopy(seq->Fttable);

    return pthis;
}


/* @obsolete ajSeqNewS
** @rename ajSeqNewSeq
*/

__deprecated AjPSeq  ajSeqNewS(const AjPSeq seq)
{
    return ajSeqNewSeq(seq);
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
**
** @argrule * Pseq [AjPSeq*] Sequence object address
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
** @@
******************************************************************************/

void ajSeqDel(AjPSeq* Pseq)
{
    AjPSeq seq;
    AjPStr ptr = NULL;
    AjPSeqRef tmpref = NULL;

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
    ajStrDel(&seq->Type);
    ajStrDel(&seq->Molecule);
    ajStrDel(&seq->Class);
    ajStrDel(&seq->Division);
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

    while(ajListstrPop(seq->Acclist,&ptr))
	ajStrDel(&ptr);
    ajListFree(&seq->Acclist);

    while(ajListstrPop(seq->Keylist,&ptr))
	ajStrDel(&ptr);
    ajListFree(&seq->Keylist);

    while(ajListstrPop(seq->Taxlist,&ptr))
	ajStrDel(&ptr);
    ajListFree(&seq->Taxlist);

    while(ajListPop(seq->Reflist,(void **)&tmpref))
	ajSeqrefDel(&tmpref);
    ajListFree(&seq->Reflist);

    while(ajListPop(seq->Cmtlist,(void **)&ptr))
	ajStrDel(&ptr);
    ajListFree(&seq->Cmtlist);

    while(ajListPop(seq->Xreflist,(void **)&ptr))
	ajStrDel(&ptr);
    ajListFree(&seq->Xreflist);

    ajSeqdateDel(&seq->Date);

    AJFREE(*Pseq);
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
** @nam4rule AssignSeq Assign sequence
** @nam4rule AssignSv Assign sequence version number
** @nam4rule AssignUfo Assign feature address
** @nam4rule AssignUsa Assign sequence address
**
** @nam3rule Clear     Clear all contents
**
** @nam3rule Set Set sequence properties
** @nam4rule SetName Set sequence name
** @nam5rule SetNameMulti Set sequence name, adding a number for later calls
** @nam4rule SetOffsets Set sequence offsets as a subsequence of an original
** @nam4rule SetRange Set start and end position within sequence
** @nam5rule SetRangeRev Set start and end position and reverse direction
**                       of a sequence
** @nam4rule SetUnique Make sure sequence is modifiable (no other pointer
**                     uses the same internal string)
**
** @suffix Len [ajint] Length of character string
** @suffix C [const char*] Character string
** @suffix S [const AjPStr] String
** 
** @argrule * seq [AjPSeq] Sequence object
** @argrule C txt [const char*] Character string to assign
** @argrule Len len [ajint] Character string length
** @argrule S str [const AjPStr] String to assign
** @argrule Offsets offset [ajint] Offset at start
** @argrule Offsets origlen [ajint] Length of original sequence
** @argrule Range pos1 [ajint] Start position
** @argrule Range pos2 [ajint] End  position
** @argrule SetName setname [const AjPStr] User-defined sequence name
**
** @valrule * [void]
**
******************************************************************************/




/* @func ajSeqAssignAccC ******************************************************
**
** Assigns the sequence accession number.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] txt [const char*] Accession number as a C character string.
** @return [void]
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
** @@
******************************************************************************/

void ajSeqAssignAccS(AjPSeq seq, const AjPStr str)
{
    ajStrAssignS(&seq->Acc, str);

    return;
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

/* @func ajSeqAssignDescC *****************************************************
**
** Assigns a modified description to an existing AjPSeq sequence.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] txt [const char*] New description as a C character string.
** @return [void]
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
** @@
******************************************************************************/

void ajSeqAssignDescS(AjPSeq seq, const AjPStr str)
{
    ajStrAssignS(&seq->Desc, str);

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


/* @func ajSeqAssignEntryC ****************************************************
**
** Assigns the sequence entryname.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] txt [const char*] Entry name as a C character string.
** @return [void]
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
** @@
******************************************************************************/

void ajSeqAssignEntryS(AjPSeq seq, const AjPStr str)
{
    ajStrAssignS(&seq->Entryname, str);

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

/* @func ajSeqAssignFileC *****************************************************
**
** Assigns the sequence filename.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] txt [const char*] File name as a C character string.
** @return [void]
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
** @@
******************************************************************************/

void ajSeqAssignFileS(AjPSeq seq, const AjPStr str)
{
    ajStrAssignS(&seq->Filename, str);

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



/* @func ajSeqAssignFullC *****************************************************
**
** Assigns the sequence name.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] txt [const char*] Full name as a C character string.
** @return [void]
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
** @@
******************************************************************************/

void ajSeqAssignFullS(AjPSeq seq, const AjPStr str)
{
    ajStrAssignS(&seq->Full, str);

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




/* @func ajSeqAssignGiC *******************************************************
**
** Assigns the GI version number.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] txt [const char*] GI number as a C character string.
** @return [void]
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
** @@
******************************************************************************/

void ajSeqAssignGiS(AjPSeq seq, const AjPStr str)
{
    ajStrAssignS(&seq->Gi, str);

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




/* @func ajSeqAssignNameC *****************************************************
**
** Assigns the sequence name.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] txt [const char*] Name as a string.
** @return [void]
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
** @@
******************************************************************************/

void ajSeqAssignNameS(AjPSeq seq, const AjPStr str)
{
    ajStrAssignS(&seq->Name, str);

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


/* @func ajSeqAssignSeqC ******************************************************
**
** Assigns a modified sequence to an existing AjPSeq sequence.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] txt [const char*] New sequence as a C character string.
** @return [void]
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
** @param [r] len [ajint] Numbur of characters to use
** @return [void]
** @@
******************************************************************************/

void ajSeqAssignSeqLenC(AjPSeq seq, const char* txt, ajint len)
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
** @@
******************************************************************************/

void ajSeqAssignSeqS(AjPSeq seq, const AjPStr str)
{
    ajSeqAssignSeqLenC(seq, ajStrGetPtr(str), ajStrGetLen(str));

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
    ajSeqAssignSeqC(thys, text);
    return;
}




/* @func ajSeqAssignSvC *******************************************************
**
** Assigns the sequence version number.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] txt [const char*] SeqVersion number as a C character string.
** @return [void]
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
** @@
******************************************************************************/

void ajSeqAssignSvS(AjPSeq seq, const AjPStr str)
{
    ajStrAssignS(&seq->Sv, str);

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


/* @func ajSeqAssignUfoC ******************************************************
**
** Assigns the sequence feature file name.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] txt [const char*] UFO as a C character string.
** @return [void]
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
** @@
******************************************************************************/

void ajSeqAssignUfoS(AjPSeq seq, const AjPStr str)
{
    ajStrAssignS(&seq->Ufo, str);

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


/* @func ajSeqAssignUsaC ******************************************************
**
** Assigns the sequence full name.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] txt [const char*] USA as a C character string.
** @return [void]
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
** @@
******************************************************************************/

void ajSeqAssignUsaS(AjPSeq seq, const AjPStr str)
{
    ajStrAssignS(&seq->Usa, str);

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




/* @func ajSeqClear ***********************************************************
**
** Resets all data for a sequence object so that it can be reused.
**
** @param [u] seq [AjPSeq] Sequence
** @return [void]
** @@
******************************************************************************/

void ajSeqClear(AjPSeq seq)
{
    AjPStr ptr = NULL;
    AjPSeqRef tmpref = NULL;

    ajStrSetClear(&seq->Name);
    ajStrSetClear(&seq->Acc);
    ajStrSetClear(&seq->Sv);
    ajStrSetClear(&seq->Gi);
    ajStrSetClear(&seq->Tax);
    ajStrSetClear(&seq->Type);
    ajStrSetClear(&seq->Molecule);
    ajStrSetClear(&seq->Class);
    ajStrSetClear(&seq->Division);
    ajStrSetClear(&seq->Db);
    ajStrSetClear(&seq->Full);
    ajStrSetClear(&seq->Desc);
    ajStrSetClear(&seq->Doc);
    ajStrSetClear(&seq->Usa);
    ajStrSetClear(&seq->Ufo);

    ajStrSetClear(&seq->Formatstr);
    ajStrSetClear(&seq->Filename);
    ajStrSetClear(&seq->Entryname);
    ajStrSetClear(&seq->TextPtr);
    ajStrSetClear(&seq->Seq);

    AJFREE(seq->Accuracy);

    seq->Begin = 0;
    seq->End   = 0;
    seq->Rev      = ajFalse;
    seq->Reversed = ajFalse;
    seq->Trimmed  = ajFalse;

    while(ajListstrPop(seq->Acclist,&ptr))
	ajStrDel(&ptr);

    while(ajListstrPop(seq->Keylist,&ptr))
	ajStrDel(&ptr);

    while(ajListstrPop(seq->Taxlist,&ptr))
	ajStrDel(&ptr);

    while(ajListPop(seq->Reflist,(void **)&tmpref))
	ajSeqrefDel(&tmpref);

    while(ajListPop(seq->Cmtlist,(void **)&ptr))
	ajStrDel(&ptr);

    while(ajListPop(seq->Xreflist,(void **)&ptr))
	ajStrDel(&ptr);

    ajSeqdateDel(&seq->Date);

    ajFeattableDel(&seq->Fttable);

    return;
}




/* @func ajSeqSetName *********************************************************
**
** Provides a unique (for this program run) name for a sequence.
**
** @param [w] seq [AjPSeq] Sequence object
** @param [r] setname [const AjPStr] Name set by caller
** @return [void]
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
** @@
******************************************************************************/

void ajSeqSetRangeRev(AjPSeq seq, ajint pos1, ajint pos2)
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

    if(!seq->Rev)
    {
	seq->Rev = ajTrue;
	seq->Reversed = ajFalse;
    }

    ajSeqReverseDo(seq);

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


/* @func ajSeqSetUnique *******************************************************
**
** Makes a sequence modifiable by making sure there is no duplicate
** copy of the sequence.
**
** @param [u] seq [AjPSeq] Sequence
** @return [void]
** @@
******************************************************************************/

void ajSeqSetUnique(AjPSeq seq)
{
    ajStrGetuniqueStr(&seq->Seq);

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

/* @funcstatic seqMakeUsa ****************************************************
**
** Sets the USA for a sequence.
**
** @param [r] thys [const AjPSeq] Sequence object
** @param [w] usa [AjPStr*] USA in full
** @return [void]
** @@
******************************************************************************/

static void seqMakeUsa(const AjPSeq thys, AjPStr* usa)
{
    AjPStr tmpstr = NULL;

    ajDebug("ajSeqMakeUsa (Name <%S> Formatstr <%S> Db <%S> "
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
	if(ajStrMatchC(thys->Formatstr, "text"))
	    if(thys->Reversed)
	    {
		ajStrAssignS(&tmpstr, thys->Seq);
		ajSeqstrReverse(&tmpstr);
		ajFmtPrintS(usa, "asis::%S", tmpstr);
		ajStrDel(&tmpstr);
	    }
	    else
		ajFmtPrintS(usa, "asis::%S", thys->Seq);
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


/* @func ajSeqComplement **************************************************
**
** Complements but does not reverse a nucleotide sequence.
**
** @param [u] seq [AjPSeq] Sequence
** @return [void]
** @@
******************************************************************************/

void ajSeqComplement(AjPSeq seq)
{
    ajSeqstrComplement(&seq->Seq);

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




/* @func ajSeqFmtLower ********************************************************
**
** Converts a sequence to lower case.
**
** @param [u] seq [AjPSeq] Sequence
** @return [void]
** @@
******************************************************************************/

void ajSeqFmtLower(AjPSeq seq)
{
    ajStrFmtLower(&seq->Seq);

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




/* @func ajSeqFmtUpper ********************************************************
**
** Converts a sequence to upper case.
**
** @param [u] seq [AjPSeq] Sequence
** @return [void]
** @@
******************************************************************************/

void ajSeqFmtUpper(AjPSeq seq)
{
    ajStrFmtUpper(&seq->Seq);

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


/* @func ajSeqGapFill *********************************************************
**
** Fills a single sequence with gaps up to a specified length.
**
** @param [u] seq [AjPSeq] Sequence object to be set.
** @param [r] len [ajuint] Length to pad fill to.
** @return [void]
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

/* @func ajSeqGapStandard ****************************************************
**
** Makes all gaps in a string use a standard gap character
**
** @param [w] seq [AjPSeq] Sequence object
** @param [r] gapchar [char] Gap character (or '-' if zero)
** @return [void]
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
** @@
******************************************************************************/

void ajSeqReverseDo(AjPSeq seq)
{
    ajint ibegin;
    ajint iend;
    ajint itemp;

    ajDebug("ajSeqReverse len: %d Begin: %d End: %d Rev: %B Reversed: %B\n",
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
	ajFeattableReverse(seq->Fttable);

    return;
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

/* @obsolete ajSeqRevOnly
** @rename ajSeqReverseOnly
*/

__deprecated void  ajSeqRevOnly(AjPSeq seq)
{
    ajSeqReverseOnly(seq);
    return;
}




/* @func ajSeqTrim ************************************************************
**
** Trim a sequence using the Begin and Ends.
** Also reverse complements a nucleotide sequence if required.
**
** @param [u] seq [AjPSeq] Sequence to be trimmed.
** @return [void]
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

    if(seq->Rev)
	ajSeqReverseDo(seq);

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

    begin = 1 + ajMathPosI(ajSeqGetLen(seq), 0, jbegin);
    end   = 1 + ajMathPosI(ajSeqGetLen(seq), begin-1, jend);

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
** @nam4rule GetEntry Return sequence ID
** @nam4rule GetFeat Return sequence feature table
** @nam4rule GetGi Return sequence GI number
** @nam4rule GetLen Return sequence length
** @nam4rule GetName Return sequence name
** @nam4rule GetOffend Return sequence end offset
** @nam4rule GetOffset Return sequence start offset
** @nam4rule GetRange Return sequence begin and end
** @nam4rule GetRev Return sequence reverse attribute (not yet reversed)
** @nam4rule GetSeq Return sequence
** @nam4rule GetSv Return sequence version
** @nam4rule GetTax Return taxonomy
** @nam4rule GetUsa Return sequence USA
**
** @suffix S Return a string
** @suffix C Return a character string
** @suffix True True position in original sequence
** @suffix Copy Editable copy
**
** @argrule * seq [const AjPSeq] Sequence
** @argrule Range begin [ajint*] Start position
** @argrule Range end [ajint*] End position
**
** @valrule C [const char*]
** @valrule S [const AjPStr]
** @valrule Begin [ajuint] Sequence position
** @valrule End [ajuint] Sequence position
** @valrule Len [ajuint] Sequence length
** @valrule Offend [ajuint] Sequence end offset
** @valrule Offset [ajuint] Sequence start offset
** @valrule Range [ajuint] Sequence length
** @valrule Rev [AjBool] Reverse attribute
** @valrule *Feat [const AjPFeattable] Link to internal feature table
** @valrule *FeatCopy [AjPFeattable] New feature table with original contents
** @valrule *SeqCopyC [char*] New sequence with original contents
** @valrule *SeqCopyS [AjPStr] New sequence with original contents
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
** @@
******************************************************************************/

const AjPStr ajSeqGetAccS(const AjPSeq seq)
{
    return seq->Acc;
}




/* @obsolete ajSeqGetAcc
** @rename ajSeqGetAccS
*/
__deprecated const AjPStr  ajSeqGetAcc(const AjPSeq seq)
{
    return ajSeqGetAccS(seq);
}


/* @func ajSeqGetBegin ********************************************************
**
** Returns the sequence start position within the current stored sequence,
** or 1 if no start has been set.
**
** To return the position within the original sequence, which may be different
** if the sequence has been trimmed, use ajSeqTrueBegin
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [ajuint] Start position.
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

    j =  1 + ajMathPosI(ajSeqGetLen(seq), 0, i);

    return j;
}

/* @obsolete ajSeqBegin
** @rename ajSeqGetBegin
*/

__deprecated ajint  ajSeqBegin(const AjPSeq seq)
{
    return ajSeqGetBegin(seq);
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
** @@
******************************************************************************/

ajuint ajSeqGetBeginTrue(const AjPSeq seq)
{
    if(!seq->Begin)
	return ajSeqCalcTruepos(seq, 1);

    return ajSeqCalcTruepos(seq, seq->Begin);
}


/* @obsolete ajSeqTrueBegin
** @rename ajSeqGetBeginTrue
*/
__deprecated ajint  ajSeqTrueBegin(const AjPSeq seq)
{
    return ajSeqGetBeginTrue(seq);
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
** @@
******************************************************************************/

const AjPStr ajSeqGetDbS(const AjPSeq seq)
{
    return seq->Db;
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
** @@
******************************************************************************/

const AjPStr ajSeqGetDescS(const AjPSeq seq)
{
    return seq->Desc;
}

/* @obsolete ajSeqGetDesc
** @rename ajSeqGetDescS
*/

__deprecated const AjPStr  ajSeqGetDesc(const AjPSeq seq)

{
    return seq->Desc;
}




/* @func ajSeqGetEnd **********************************************************
**
** Returns the sequence end position, or the sequence length if no end
** has been set.
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [ajuint] End position.
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

    i = 1 + ajMathPosI(ajSeqGetLen(seq), ajSeqGetBegin(seq)-1, i);

    return i;
}



/* @obsolete ajSeqEnd
** @rename ajSeqGetEnd
*/
__deprecated ajint  ajSeqEnd(const AjPSeq seq)
{
    return ajSeqGetEnd(seq);
}

/* @func ajSeqGetEndTrue ******************************************************
**
** Returns the sequence end position, or the sequence length if no end
** has been set.
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [ajuint] End position.
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



/* @obsolete ajSeqTrueEnd
** @rename ajSeqGetEndTrue
*/
__deprecated ajint  ajSeqTrueEnd(const AjPSeq seq)
{
    return ajSeqGetEndTrue(seq);
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
** @@
******************************************************************************/

const AjPStr ajSeqGetEntryS(const AjPSeq seq)
{
    return seq->TextPtr;
}

/* @obsolete ajSeqGetEntry
** @rename ajSeqGetEntryS
*/
__deprecated const AjPStr  ajSeqGetEntry(const AjPSeq seq)
{
    return ajSeqGetEntryS(seq);
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
** @@
******************************************************************************/

AjPFeattable ajSeqGetFeatCopy(const AjPSeq seq)
{
    return ajFeattableCopy(seq->Fttable);
}


/* @obsolete ajSeqCopyFeat
** @rename ajSeqGetFeatCopy
*/
__deprecated AjPFeattable  ajSeqCopyFeat(const AjPSeq seq)
{
    return ajFeattableCopy(seq->Fttable);
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
** @@
******************************************************************************/

const AjPStr ajSeqGetGiS(const AjPSeq seq)
{
    return seq->Gi;
}

/* @obsolete ajSeqGetGi
** @rename ajSeqGetGiS
*/
__deprecated const AjPStr  ajSeqGetGi(const AjPSeq seq)
{
    return seq->Gi;
}




/* @func ajSeqGetLen **********************************************************
**
** Returns the sequence length.
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [ajuint] Sequence length.
** @@
******************************************************************************/

ajuint ajSeqGetLen(const AjPSeq seq)
{
    return ajStrGetLen(seq->Seq);
}



/* @obsolete ajSeqLen
** @rename ajSeqGetLen
*/
__deprecated ajint  ajSeqLen(const AjPSeq seq)
{
    return ajStrGetLen(seq->Seq);
}




/* @func ajSeqGetLenTrue ******************************************************
**
** Returns the length of the original sequence, including any gap characters.
**
** @param [r] seq [const AjPSeq] Target sequence.
** @return [ajuint] string position between 1 and length.
** @@
******************************************************************************/

ajuint ajSeqGetLenTrue(const AjPSeq seq)
{
    return (ajStrGetLen(seq->Seq) + seq->Offset + seq->Offend);
}


/* @obsolete ajSeqTrueLen
** @rename ajSeqGetLenTrue
*/
__deprecated ajint  ajSeqTrueLen(const AjPSeq seq)
{
    return ajSeqGetLenTrue(seq);
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
** @@
******************************************************************************/

const char* ajSeqGetNameC(const AjPSeq seq)
{
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
** @@
******************************************************************************/

const AjPStr ajSeqGetNameS(const AjPSeq seq)
{
    return seq->Name;
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




/* @func ajSeqGetOffend *******************************************************
**
** Returns the sequence offend value.
** This is the number of positions removed from the original end.
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [ajuint] Sequence offend.
** @@
******************************************************************************/

ajuint ajSeqGetOffend(const AjPSeq seq)
{
    return seq->Offend;
}




/* @obsolete ajSeqOffend
** @rename ajSeqGetOffend
*/
__deprecated ajint  ajSeqOffend(const AjPSeq seq)
{
    return ajSeqGetOffend(seq);
}

/* @func ajSeqGetOffset *******************************************************
**
** Returns the sequence offset from -sbegin originally.
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [ajuint] Sequence offset.
** @@
******************************************************************************/

ajuint ajSeqGetOffset(const AjPSeq seq)
{
    return seq->Offset;
}


/* @obsolete ajSeqOffset
** @rename ajSeqGetOffset
*/
__deprecated ajint  ajSeqOffset(const AjPSeq seq)
{
    return ajSeqGetOffset(seq);
}

/* @func ajSeqGetRange ********************************************************
**
** Returns the sequence range for a sequence.
**
** @param [r] seq [const AjPSeq] Sequence object.
** @param [w] begin [ajint*] Sequence range begin
** @param [w] end [ajint*] Sequence range end
** @return [ajuint] Sequence range length
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
    *begin = ajMathPos(ajSeqGetLen(seq), jbegin); /* string position */

    if(seq->End)
	*end = 1 + ajMathPosI(ajSeqGetLen(seq), *begin, jend);
    else
	*end = 1 + ajMathPosI(ajSeqGetLen(seq), *begin, ajSeqGetLen(seq));

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
** @@
******************************************************************************/

AjBool ajSeqGetRev(const AjPSeq seq)
{
    return seq->Rev;
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
** @@
******************************************************************************/

const AjPStr ajSeqGetSeqS(const AjPSeq seq)
{
    if(!seq)
	return NULL;

    return seq->Seq;
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




/* @func ajSeqGetSeqCopyC *****************************************************
**
** Returns a sequence as a C character string. This is a copy of the string
** so the caller can do anything with it.
** It must be copied back
** to a sequence (e.g. with ajSeqReplace) before output.
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [char*] Sequence as a null terminated character string.
** @@
******************************************************************************/

char* ajSeqGetSeqCopyC(const AjPSeq seq)
{
    return ajCharNewS(seq->Seq);
}

/* @obsolete ajSeqCharCopy
** @rename ajSeqGetSeqCopyC
*/
__deprecated char*  ajSeqCharCopy(const AjPSeq seq)
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
** @@
******************************************************************************/

AjPStr ajSeqGetSeqCopyS(const AjPSeq seq)
{
    static AjPStr str;

    str = ajStrNewS(seq->Seq);

    return str;
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
** @@
******************************************************************************/

const AjPStr ajSeqGetSvS(const AjPSeq seq)
{
    return seq->Sv;
}

/* @obsolete ajSeqGetSv
** @rename ajSeqGetSvS
*/
__deprecated const AjPStr  ajSeqGetSv(const AjPSeq seq)
{
    return seq->Sv;
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
** @@
******************************************************************************/

const AjPStr ajSeqGetTaxS(const AjPSeq seq)
{
    return seq->Tax;
}

/* @obsolete ajSeqGetTax
** @rename ajSeqGetTaxS
*/
__deprecated const AjPStr  ajSeqGetTax(const AjPSeq seq)
{
    return seq->Tax;
}




/* @func ajSeqGetUsaC *********************************************************
**
** Returns the sequence name of a sequence stream.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeq] Sequence object.
** @return [const char*] Name as a character string.
** @@
******************************************************************************/

const char* ajSeqGetUsaC(const AjPSeq seq)
{
    return MAJSTRGETPTR(ajSeqGetUsaS(seq));
}

/* @func ajSeqGetUsaS *********************************************************
**
** Returns the sequence name of a sequence stream.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeq] Sequence object.
** @return [const AjPStr] Name as a string.
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


/* @obsolete ajSeqGetUsa
** @rename ajSeqGetUsaS
*/
__deprecated const AjPStr  ajSeqGetUsa(const AjPSeq seq)
{
    return ajSeqGetUsaS(seq);
}

/* @section testing properties ***********************************************
**
** @fdata [AjPSeq]
** @fcategory use
**
** @nam3rule Is Test sequence property
** @nam4rule IsNuc Sequence is nucleotide
** @nam4rule IsProt Sequence is protein
** @nam4rule IsReversed Sequence is reversed
** @nam4rule IsTrimmed Sequence is trimmed to a subsequence
**
** @suffix True Sequence properties relative to the original sequence
**
** @argrule Is seq [const AjPSeq] Sequence
**
** @valrule * [AjBool] Sequence boolean property
**
******************************************************************************/

/* @func ajSeqIsNuc ***********************************************************
**
** Tests whether a sequence is nucleotide.
**
** @param [r] seq [const AjPSeq] Sequence
** @return [AjBool] ajTrue for a nucleotide sequence.
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


/* @func ajSeqIsReversedTrue **************************************************
**
** Returns ajTrue if the sequence is reversed relative to the original sequence
**
** If the sequence has already been reversed, or is set to be reversed,
** the result will be true.
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [AjBool] ajTrue if sequence is set to be reversed
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


/* @obsolete ajSeqRev
** @rename ajSeqIsReversed
*/
__deprecated AjBool  ajSeqRev(const AjPSeq seq)
{
    return ajSeqIsReversed(seq);
}

/* @func ajSeqIsReversed ******************************************************
**
** Returns whether the sequence has been reversed
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [AjBool] Sequence Direction.
** @@
******************************************************************************/

AjBool ajSeqIsReversed(const AjPSeq seq)
{
    return seq->Reversed;
}




/* @func ajSeqIsTrimmed ******************************************************
**
** Returns ajTrue if the sequence is already trimmed
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [AjBool] ajTrue if sequence is set to be reversed
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




/* @obsolete ajSeqNum
** @rename ajSeqConvertNum
*/

__deprecated AjBool  ajSeqNum(const AjPSeq seq, const AjPSeqCvt cvt,
			     AjPStr* numseq)
{
    return ajSeqConvertNum(seq, cvt, numseq);;
}




/* @section calculated properties
**
** @fdata [AjPSeq]
** @fcategory use
**
** @nam3rule Calc Calculate a value
** @nam4rule CalcCheckgcg Calculate GCG checksum
** @nam4rule CalcCount Count nucleotide bases
** @nam4rule CalcCrc Calculate cyclic reduncancy checksum
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
** @valrule CountGaps [ajuint] Number of gap chanacrters
******************************************************************************/

/* @func ajSeqCalcCheckgcg ****************************************************
**
** Calculates a GCG checksum for a sequence.
**
** @param [r] seq [const AjPSeq] Sequence.
** @return [ajint] GCG checksum.
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

    return check;
}

/* @obsolete ajSeqCheckGcg
** @rename ajSeqCalcCheckgcg
*/
__deprecated ajint  ajSeqCheckGcg(const AjPSeq seq)
{
    return ajSeqCalcCheckgcg(seq);
}

/* @func ajSeqCalcCount *******************************************************
**
** Counts the numbers of A, C, G and T in a nucleotide sequence.
**
** @param [r] seq [const AjPSeq] Sequence object
** @param [w] b [ajint*] integer array, minimum size 5, to hold the results.
** @return [void]
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


/* @obsolete ajSeqCount
** @rename ajSeqCalcCount
*/
__deprecated void  ajSeqCount(const AjPSeq seq, ajint* b)
{
    ajSeqCalcCount(seq, b);
    return;
}


/* @func ajSeqCalcMolwt *******************************************************
**
** Calculates the molecular weight of a protein sequence.
**
** @param [r] seq [const AjPSeq] Sequence
** @return [float] Molecular weight.
** @@
******************************************************************************/

float ajSeqCalcMolwt(const AjPSeq seq)
{
    return ajSeqstrCalcMolwt(seq->Seq);
}



/* @obsolete ajSeqPos
** @replace ajMathPos (1,2/'ajSeqGetLen[1]',2)
*/

__deprecated ajint  ajSeqPos(const AjPSeq seq, ajint ipos)
{
    return 1+ajMathPosI(ajSeqGetLen(seq), 0, ipos);
}




/* @obsolete ajSeqPosI
** @replace ajMathPosI (1,2,3/'ajSeqGetLen[1]',2,3)
*/
__deprecated ajint  ajSeqPosI(const AjPSeq seq, ajint imin, ajint ipos)
{
    return 1+ajMathPosI(ajSeqGetLen(seq), imin, ipos);
}




/* @obsolete ajSeqPosII
** @rename ajMathPosI
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
	return 1 + seq->Offend + ajMathPosI(ajSeqGetLen(seq),
			  jmin, jpos);
    else
	return 1 + seq->Offset + ajMathPosI(ajSeqGetLen(seq),
			  jmin, jpos);
}

/* @obsolete ajSeqTruePosI
** @rename ajSeqCalcTrueposMin
*/

__deprecated ajint  ajSeqTruePosI(const AjPSeq thys, ajint imin, ajint ipos)
{
    return ajSeqCalcTrueposMin(thys, imin, ipos);
}

/* @obsolete ajSeqTruePosII
** @rename ajMathPosI
*/
__deprecated ajint  ajSeqTruePosII(ajint ilen, ajint imin, ajint ipos)
{
    return ajMathPosI(ilen, imin, ipos);
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
** @@
******************************************************************************/

ajint ajSeqCalcTruepos(const AjPSeq seq, ajint ipos)
{
    ajint jpos;

    jpos = ipos;
    if (ipos > 0)
	jpos = ipos - 1;

    if(ajSeqGetRev(seq))
	return 1 + seq->Offend + ajMathPosI(ajSeqGetLen(seq), 0, jpos);
    else
	return 1 + seq->Offset + ajMathPosI(ajSeqGetLen(seq), 0, jpos);
}


/* @obsolete ajSeqTruePos
** @rename ajSeqCalcTruepos
*/
__deprecated ajint  ajSeqTruePos(const AjPSeq thys, ajint ipos)
{
    return ajSeqCalcTruepos(thys, ipos);
}


/* @func ajSeqCountGaps *******************************************************
**
** Returns the number of gaps in a sequence (counting any possible
** gap character
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [ajuint] Number of gaps
******************************************************************************/

ajuint ajSeqCountGaps(const AjPSeq seq)
{
    ajuint ret = 0;

    static char testchars[] = "-~.? "; /* all known gap characters */
    const char *testgap;

    testgap = testchars;

    while(*testgap)
    {
	ret += ajStrCalcCountK(seq->Seq, *testgap);
	testgap++;
    }

    return ret;
}

/* @obsolete ajSeqGapCount
* @rename ajSeqCountGaps
*/

__deprecated ajint  ajSeqGapCount(const AjPSeq seq)
{
    return ajSeqCountGaps(seq);
}

/* @section exit
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
*/





/* @func ajSeqExit ************************************************************
**
** Cleans up sequence processing  internal memory
**
** @return [void]
** @@
******************************************************************************/

void ajSeqExit(void)
{
    ajSeqReadExit();
    ajSeqoutExit();
    ajSeqDbExit();
    ajSeqTypeExit();

    ajStrDel(&seqVersionAccnum);
    ajStrDel(&seqTempUsa);

    ajStrDel(&seqMoleculeDef);
    ajStrDel(&seqDivisionDef);
    ajStrDel(&seqClassDef);

    ajTablestrFreeKey(&seqTableMol);
    ajTablestrFreeKey(&seqTableMolEmbl);
    ajTablestrFreeKey(&seqTableMolDdbj);
    ajTablestrFreeKey(&seqTableMolGb);

    ajTablestrFreeKey(&seqTableDiv);
    ajTablestrFreeKey(&seqTableDivEmbl);
    ajTablestrFreeKey(&seqTableDivDdbj);
    ajTablestrFreeKey(&seqTableDivGb);

    ajTablestrFreeKey(&seqTableCls);
    ajTablestrFreeKey(&seqTableClsEmbl);
    ajTablestrFreeKey(&seqTableClsDdbj);
    ajTablestrFreeKey(&seqTableClsGb);

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
	ajDebug( "  Acclist: (%d) ", ajListGetLength(seq->Acclist));
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

    if(ajListGetLength(seq->Taxlist))
    {
	ajDebug( "  Taxlist: (%d)", ajListGetLength(seq->Taxlist));
	it = ajListIterNewread(seq->Taxlist);
	while((cur = (AjPStr) ajListIterGet(it)))
	    ajDebug(" '%S'", cur);

	ajListIterDel(&it);
	ajDebug("\n");
    }

    if(ajListGetLength(seq->Keylist))
    {
	ajDebug( "  Keywordlist: (%d)", ajListGetLength(seq->Keylist));
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

    if(seq->Weight)
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
******************************************************************************/

void ajSeqTraceTitle(const AjPSeq seq, const char* title)
{
    ajDebug("\n%s\n",title);
    ajSeqTrace(seq);

    return;
}


/* @obsolete ajSeqTraceT
** @rename ajSeqTraceTitle
*/
__deprecated void  ajSeqTraceT(const AjPSeq seq, const char* title)
{
    ajSeqTraceTitle(seq, title);
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
** @@
******************************************************************************/

AjPSeqall ajSeqallNew(void)
{
    AjPSeqall pthis;

    AJNEW0(pthis);

    pthis->Seq   = ajSeqNew();
    pthis->Seqin = ajSeqinNew();
    pthis->Count = 0;

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
** @@
******************************************************************************/

void ajSeqallDel(AjPSeqall *Pseq)
{
    if(!*Pseq)
	return;

    if(!(*Pseq)->Returned)
	ajSeqDel(&(*Pseq)->Seq);

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
** @nam3rule Clear     Clear all contents
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
** Resets all data for a sequence stream object so that it can be reused.
**
** @param [u] seq [AjPSeqall] Sequence stream
** @return [void]
** @@
******************************************************************************/

void ajSeqallClear(AjPSeqall seq)
{
    ajSeqClear(seq->Seq);
    ajSeqinClear(seq->Seqin);
    seq->Count = 0;
    seq->Begin = 0;
    seq->End   = 0;
    seq->Rev   = ajFalse;
    seq->Returned = ajFalse;
 
    return;
}





/* @obsolete ajSeqallReverse
** @remove sequence processed separately afetr ajSeqallNext
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




/* @func ajSeqallSetRange *****************************************************
**
** Sets the start and end positions for a sequence stream.
**
** @param [u] seq [AjPSeqall] Sequence stream object to be set.
** @param [r] pos1 [ajint] Start position. Negative values are from the end.
** @param [r] pos2 [ajint] End position. Negative values are from the end.
** @return [void]
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
** @nam4rule GetFilename Return filename used for stream
** @nam4rule GetName Return name of sequence stream
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
** @valrule End [ajint] End position
** @valrule Filename [const AjPStr] Filename
** @valrule Name [const AjPStr] Sequence name
** @valrule Len [ajint] Sequence length
** @valrule Range [ajint] Sequence length
** @valrule Usa [const AjPStr] Sequence USA
**
******************************************************************************/




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
** @@
******************************************************************************/

const AjPStr ajSeqallGetFilename(const AjPSeqall seq)
{
    if(!seq)
	return NULL;
    if(!seq->Seqin)
	return NULL;

    ajDebug("ajSeqallGetFilename '%S' usa: '%S'\n",
	    seq->Seqin->Name, seq->Seqin->Usa);



    if(ajStrGetLen(seq->Seqin->Filename))
	return seq->Seqin->Filename;

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
** @@
******************************************************************************/

const AjPStr ajSeqallGetName(const AjPSeqall seq)
{
    ajDebug("ajSeqallGetName '%S'\n", seq->Seqin->Name);

    return seq->Seqin->Name;
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
** @@
******************************************************************************/

const AjPStr ajSeqallGetUsa(const AjPSeqall seq)
{
    ajDebug("ajSeqallGetUsa '%S'\n", seq->Seqin->Usa);

    return seq->Seqin->Usa;
}




/* @func ajSeqallGetseqBegin **************************************************
**
** Returns the sequence stream start position, or 1 if no start has been set.
**
** @param [r] seq [const AjPSeqall] Sequence stream object
** @return [ajint] Start position.
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

	return 1 + ajMathPosI(ajSeqGetLen(seq->Seq), 0, jbegin);
    }
    if(seq->Seq->Begin)
    {
	jbegin = seq->Seq->Begin;
	if(jbegin > 0)
	    jbegin--;

	return 1 + ajMathPosI(ajSeqGetLen(seq->Seq), 0, jbegin);
    }
    return 1;
}

/* @obsolete ajSeqallBegin
** @rename ajSeqallGetseqBegin
*/
__deprecated ajint  ajSeqallBegin(const AjPSeqall seq)
{
    return ajSeqallGetseqBegin(seq);
}


/* @func ajSeqallGetseqEnd ****************************************************
**
** Returns the sequence stream end position, or the sequence length if no end
** has been set.
**
** @param [r] seq [const AjPSeqall] Sequence stream object
** @return [ajint] Start position.
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
	return 1 + ajMathPosI(ajSeqGetLen(seq->Seq),
			      ajSeqallGetseqBegin(seq)-1, jend);
    }

    if(seq->Seq->End)
    {
	jend = seq->Seq->End;
	if(jend > 0)
	    jend--;
	return 1 + ajMathPosI(ajSeqGetLen(seq->Seq),
			      ajSeqallGetseqBegin(seq)-1, jend);
    }
    return ajSeqGetLen(seq->Seq);
}


/* @obsolete ajSeqallEnd
** @rename ajSeqallGetseqEnd
*/
__deprecated ajint  ajSeqallEnd(const AjPSeqall seq)
{
    return ajSeqallGetseqEnd(seq);
}


/* @func ajSeqallGetseqLen ****************************************************
**
** Returns the length of a sequence stream, which is the length of the
** latest sequence read.
**
** @param [r] seq [const AjPSeqall] Sequence stream object
** @return [ajint] sequence length.
** @@
******************************************************************************/

ajint ajSeqallGetseqLen(const AjPSeqall seq)
{
    return ajSeqGetLen(seq->Seq);
}

/* @obsolete ajSeqallLen
** @rename ajSeqallGetseqLen
*/
__deprecated ajint  ajSeqallLen(const AjPSeqall seqall)
{
    return ajSeqallGetseqLen(seqall);
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
** @@
******************************************************************************/

const AjPStr ajSeqallGetseqName(const AjPSeqall seq)
{
    ajDebug("ajSeqallGetseqName '%S'\n", seq->Seq->Name);

    return ajSeqGetNameS(seq->Seq);
}

/* @obsolete ajSeqallGetNameSeq
** @rename ajSeqallGetseqName
*/

__deprecated const AjPStr  ajSeqallGetNameSeq(const AjPSeqall seq)
{
    return ajSeqallGetseqName(seq);
}


/* @func ajSeqallGetseqRange **************************************************
**
** Returns the sequence range for a sequence stream
**
** @param [r] seq [const AjPSeqall] Sequence stream object.
** @param [w] begin [ajint*] Sequence range begin
** @param [w] end [ajint*] Sequence range end
** @return [ajint] Sequence range length
** @@
******************************************************************************/

ajint ajSeqallGetseqRange(const AjPSeqall seq, ajint* begin, ajint* end)
{
    ajDebug("ajSeqallGetRange '%S'\n", seq->Seq->Name);

    return ajSeqGetRange(seq->Seq, begin, end);
}

/* @obsolete ajSeqallGetRange
** @rename   ajSeqallGetseqRange
*/

__deprecated ajint  ajSeqallGetRange(const AjPSeqall seq,
				    ajint* begin, ajint* end)
{
    return ajSeqallGetseqRange(seq, begin, end);
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

    AJFREE(seq);

    return;
}




/* @func ajSeqsetDelarray *****************************************************
**
** Destructor for array of sequence set objects
**
** @param [d] PPseq [AjPSeqset**] Sequence set object reference
** @return [void]
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

    ajDebug("ajSeqsetallDel size: %d\n", i);

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
    *begin = ajMathPosI(seq->Len, 0, jbegin);

    if(seq->End)
	*end = 1 + ajMathPosI(seq->Len, *begin, jend);
    else
	*end = 1 + ajMathPosI(seq->Len, *begin, seq->Len);

    (*begin)++;

    return (*end - *begin + 1);
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
** @nam3rule Fill Fills shorter sequences with gaps at end
** @nam3rule Fmt Reformats sequence
** @nam4rule FmtLower reformats sequence to lower case
** @nam4rule FmtUpper reformats sequence to upper case
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




/* @func ajSeqsetFill *********************************************************
**
** Fills a sequence set with gaps at the ends of any shorter sequences.
**
** @param [u] seq [AjPSeqset] Sequence set object to be set.
** @return [ajint] Number of gaps inserted
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
    {
	if(ajSeqGetLen(seq->Seq[i]) < seq->Len)
	{
	    nfix++;
	    ilen = seq->Len - ajSeqGetLen(seq->Seq[i]);
	    if(ilen > ifix)
		ifix = ilen;
	    ajStrAppendCountK(&seq->Seq[i]->Seq, '-', ilen);
	}
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
** @@
******************************************************************************/

void ajSeqsetFmtLower(AjPSeqset seq)
{
    ajuint i;

    for(i=0; i < seq->Size; i++)
	ajSeqFmtLower(seq->Seq[i]);

    return;
}


/* @obsolete ajSeqsetToLower
** @rename ajSeqsetFmtLower
*/
__deprecated void  ajSeqsetToLower(AjPSeqset seqset)
{
    ajSeqsetFmtLower(seqset);
    return;
}





/* @func ajSeqsetFmtUpper *****************************************************
**
** Converts all sequences in a set to upper case.
**
** @param [u] seq [AjPSeqset] Sequence set object
** @return [void]
** @@
******************************************************************************/

void ajSeqsetFmtUpper(AjPSeqset seq)
{
    ajuint i;

    for(i=0; i < seq->Size; i++)
	ajSeqFmtUpper(seq->Seq[i]);

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

/* @func ajSeqsetReverse ******************************************************
**
** Reverse complements all sequences in a sequence set.
**
** @param [u] seq [AjPSeqset] Sequence set object
** @return [void]
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
	ajSeqReverseDo(seq->Seq[i]);

    ajDebug("  set result len: %d Begin: %d End: %d\n",
	    ajSeqsetGetLen(seq), seq->Begin, seq->End);

    return;
}



/* @func ajSeqsetTrim ******************************************************
**
** Trims a sequence set to start and end positions
**
** @param [u] seq [AjPSeqset] Sequence set object
** @return [void]
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

    begin = 1 + ajMathPosI(seq->Len, 0, jbegin);
    end   = 1 + ajMathPosI(seq->Len, begin-1, jend);

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

    return 1 + ajMathPosI(seq->Len, 0, jbegin);
}


/* @obsolete ajSeqsetBegin
** @rename ajSeqsetGetBegin
*/
__deprecated ajint  ajSeqsetBegin(const AjPSeqset seq)
{
    return ajSeqsetGetBegin(seq);
}


/* @func ajSeqsetGetEnd *******************************************************
**
** Returns the sequence set end position, or the sequence length if no end
** has been set.
**
** @param [r] seq [const AjPSeqset] Sequence set object
** @return [ajuint] Start position.
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

    return 1 + ajMathPosI(seq->Len, ajSeqsetGetBegin(seq)-1, jend);
}



/* @obsolete ajSeqsetEnd
** @rename ajSeqsetGetEnd
*/
__deprecated ajint  ajSeqsetEnd(const AjPSeqset seq)
{
    return ajSeqsetGetEnd(seq);
}

/* @func ajSeqsetGetLen *******************************************************
**
** Returns the length of a sequence set, which is the maximum sequence
** length in the set.
**
** @param [r] seq [const AjPSeqset] Sequence set object
** @return [ajuint] sequence set length.
** @@
******************************************************************************/

ajuint ajSeqsetGetLen(const AjPSeqset seq)
{
    return seq->Len;
}


/* @obsolete ajSeqsetLen
** @rename ajSeqsetGetLen
*/

__deprecated ajint  ajSeqsetLen(const AjPSeqset seq)
{
    return ajSeqsetGetLen(seq);
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
** @@
******************************************************************************/

const char* ajSeqsetGetNameC(const AjPSeqset seq)
{
    ajDebug("ajSeqsetGetName '%S' usa: '%S'\n", seq->Name, seq->Usa);

    if(ajStrGetLen(seq->Name))
      return MAJSTRGETPTR(seq->Name);

    return MAJSTRGETPTR(seq->Usa);
}



/* @obsolete ajSeqsetGetName
** @rename ajSeqsetGetNameS
*/
__deprecated const AjPStr  ajSeqsetGetName(const AjPSeqset thys)
{
    return ajSeqsetGetNameS(thys);
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
** @@
******************************************************************************/

const AjPStr ajSeqsetGetNameS(const AjPSeqset seq)
{
    ajDebug("ajSeqsetGetName '%S' usa: '%S'\n", seq->Name, seq->Usa);

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


/* @obsolete ajSeqsetGetSeqArray
** @rename ajSeqsetGetSeqarray
*/
__deprecated AjPSeq*  ajSeqsetGetSeqArray(const AjPSeqset thys)
{
    return ajSeqsetGetSeqarray(thys);
}

/* @func ajSeqsetGetSize ******************************************************
**
** Returns the number of sequences in a sequence set
**
** @param [r] seq [const AjPSeqset] Sequence set object
** @return [ajuint] sequence set size.
** @@
******************************************************************************/

ajuint ajSeqsetGetSize(const AjPSeqset seq)
{
    return seq->Size;
}


/* @obsolete ajSeqsetSize
** @rename ajSeqsetGetSize
*/
__deprecated ajint  ajSeqsetSize(const AjPSeqset seq)
{
    return ajSeqsetGetSize(seq);
}

/* @func ajSeqsetGetTotweight *************************************************
**
** Returns the weight of all sequences in a sequence set
**
** @param [r] seq [const AjPSeqset] Sequence set object
** @return [float] sequence weight as a float.
** @@
******************************************************************************/

float ajSeqsetGetTotweight(const AjPSeqset seq)
{
    ajuint i;
    float ret = 0.0;

    for(i=0; i < seq->Size; i++)
    {
	ret += seq->Seq[i]->Weight;
    }

    return ret;
}


/* @obsolete ajSeqsetTotweight
** @rename ajSeqsetGetTotweight
*/
__deprecated float  ajSeqsetTotweight(const AjPSeqset seq)
{
    return ajSeqsetGetTotweight(seq);
}

/* @func ajSeqsetGetseqAccC ***************************************************
**
** Returns the accession number of a sequence in a sequence set
**
** @param [r] seq [const AjPSeqset] Sequence set object
** @param [r] i [ajuint] Sequence index
** @return [const char*] accession number as a string.
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
** @@
******************************************************************************/

const AjPStr ajSeqsetGetseqAccS(const AjPSeqset seq, ajuint i)
{
    if(i >= seq->Size)
	return NULL;

    return seq->Seq[i]->Acc;
}


/* @obsolete ajSeqsetAcc
** @rename ajSeqsetGetseqAccS
*/
__deprecated const AjPStr  ajSeqsetAcc(const AjPSeqset seq, ajint i)
{
    return ajSeqsetGetseqAccS(seq, i);
}



/* @func ajSeqsetGetseqNameC **************************************************
**
** Returns the name of a sequence in a sequence set
**
** @param [r] seq [const AjPSeqset] Sequence set object
** @param [r] i [ajuint] Sequence index
** @return [const char*] sequence name as a string.
** @@
******************************************************************************/

const char* ajSeqsetGetseqNameC(const AjPSeqset seq, ajuint i)
{
    if(i >= seq->Size)
	return NULL;

    return MAJSTRGETPTR(seq->Seq[i]->Name);
}


/* @func ajSeqsetGetseqNameS **************************************************
**
** Returns the name of a sequence in a sequence set
**
** @param [r] seq [const AjPSeqset] Sequence set object
** @param [r] i [ajuint] Sequence index
** @return [const AjPStr] sequence name as a string.
** @@
******************************************************************************/

const AjPStr ajSeqsetGetseqNameS(const AjPSeqset seq, ajuint i)
{
    if(i >= seq->Size)
	return NULL;

    return seq->Seq[i]->Name;
}


/* @obsolete ajSeqsetName
** @rename ajSeqsetGetseqNameS
*/
__deprecated const AjPStr  ajSeqsetName(const AjPSeqset seq, ajint i)
{
    return ajSeqsetGetseqNameS(seq, i);
}

/* @func ajSeqsetGetseqSeq ***************************************************
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
** @@
******************************************************************************/

const AjPStr ajSeqsetGetseqSeqS(const AjPSeqset seq, ajuint i)
{
    ajDebug("ajSeqsetGetseqSeq '%S' %d/%d\n", seq->Name, i, seq->Size);
    if(i >= seq->Size)
	return NULL;

    return seq->Seq[i]->Seq;
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


/* @func ajSeqsetGetseqWeight *************************************************
**
** Returns the weight of a sequence in a sequence set
**
** @param [r] seq [const AjPSeqset] Sequence set object
** @param [r] i [ajuint] Sequence index
** @return [float] sequence weight as a float.
** @@
******************************************************************************/

float ajSeqsetGetseqWeight(const AjPSeqset seq, ajuint i)
{
    if(i >= seq->Size)
	return 0.0;

    return seq->Seq[i]->Weight;
}


/* @obsolete ajSeqsetWeight
** @rename ajSeqsetGetseqWeight
*/
__deprecated float  ajSeqsetWeight(const AjPSeqset seq, ajint i)
{
    return ajSeqsetGetseqWeight(seq, i);
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

    return ajTrue;
}


/* @func ajSeqsetIsRna ********************************************************
**
** Tests whether a sequence set is RNA.
**
** @param [r] seq [const AjPSeqset] Sequence set
** @return [AjBool] ajTrue for a nucleotide sequence set.
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



/* @obsolete ajSeqNumS
** @rename ajSeqstrConvertNum
*/

__deprecated AjBool  ajSeqNumS(const AjPStr seqstr,
			     const AjPSeqCvt cvt,
			     AjPStr* numseq)
{
    return ajSeqstrConvertNum(seqstr, cvt, numseq);;
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


/* @obsolete ajSeqCvtTrace
** @rename ajSeqcvtTrace
*/
__deprecated void  ajSeqCvtTrace(const AjPSeqCvt cvt)
{
    ajSeqcvtTrace(cvt);
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





/* @func ajSeqcvtNewC *****************************************************
**
** Generates a new conversion table in which the first character in the
** string provided is converted to 1, the second to 2, and so on.
** Upper and lower case characters are converted to the same numbers.
** All other characters are set to zero.
**
** @param [r] bases [const char*] Allowed sequence characters.
** @return [AjPSeqCvt] Conversion table.
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



/* @obsolete ajSeqCvtNewZero
** @rename ajSeqcvtNewC
*/
__deprecated AjPSeqCvt  ajSeqCvtNewZero(const char* bases)
{
    return ajSeqcvtNewC(bases);
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


/* @obsolete ajSeqCvtNew
** @rename ajSeqcvtNewEndC
*/
__deprecated AjPSeqCvt  ajSeqCvtNew(const char* bases)
{
    return ajSeqcvtNewEndC(bases);
}

/* @func ajSeqcvtNewNumberC ***************************************************
**
** Generates a new conversion table in which the characters are retained
** as upper case, numbers are set to -1 and all other characters
** are set to -2.
**
** @param [r] bases [const char*] Allowed sequence characters.
** @return [AjPSeqCvt] Conversion table.
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



/* @obsolete ajSeqCvtNewText
** @rename ajSeqcvtNewNumberC
*/
__deprecated AjPSeqCvt  ajSeqCvtNewText(const char* bases)
{
    return ajSeqcvtNewNumberC(bases);
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



/* @obsolete ajSeqCvtNewZeroS
** @rename ajSeqcvtNewStr
*/
__deprecated AjPSeqCvt  ajSeqCvtNewZeroS (const AjPPStr bases, ajint n)
{
    return ajSeqcvtNewStr(bases, n);
}


/* @func ajSeqcvtNewStrAsym **************************************************
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
**                            (size specified by parameter n)
** @param [r] numbases [ajint] Number of strings
** @param [r] matchbases [const AjPPStr] Allowed sequence character strings for
** rows (size specified by parameter rn)
** @param [r] nummatch [ajint] Number of strings (rows)
** @return [AjPSeqCvt] Conversion table.
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



/* @obsolete ajSeqCvtNewZeroSS
** @rename ajSeqcvtNewStrAsym
*/
__deprecated AjPSeqCvt  ajSeqCvtNewZeroSS (const AjPPStr bases, int n, 
			     const AjPPStr rbases, int rn)
{
    return ajSeqcvtNewStrAsym (bases, n, rbases, rn);
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



/* @obsolete ajSeqCvtDel
** @rename ajSeqcvtDel
*/
__deprecated void  ajSeqCvtDel (AjPSeqCvt* Pcvt)
{
    ajSeqcvtDel(Pcvt);
    return;
}

/* @section element retrieval ********************************************
**
** These functions use the contents of a sequence conversion object
**
** @fdata [AjPSeqCvt]
** @fcategory use
**
** @nam3rule Get Return sequence conversionattribute(s)
** @nam4rule GetLen Return length (number of bases defined)
** @nam4rule GetCode Return conversion code
** @nam5rule GetCodeAsym Return conversion code from asymmetric table column
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
** @@
******************************************************************************/

ajint ajSeqcvtGetCodeK(const AjPSeqCvt cvt, char ch)
{
    return cvt->table[(ajint)ch];
}


/* @obsolete ajSeqCvtK
** @rename ajSeqcvtGetCodeK
*/
__deprecated ajint  ajSeqCvtK(const AjPSeqCvt cvt, char ch)
{
    return ajSeqcvtGetCodeK(cvt, ch);
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


/* @func ajSeqcvtGetCodeAsymS *************************************************
**
** Returns the integer code corresponding to a sequence character string
** in a conversion table (for columns in asymmetrical matrices).
**
** @param [r] cvt [const AjPSeqCvt] Conversion table
** @param [r] str [const AjPStr] Sequence character string
**
** @return [ajint] Conversion code
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


    ajWarn("Sequence character string not found in ajSeqCvtKSColumn");
    return 0;
}





/* @obsolete ajSeqCvtKSColumn
** @rename ajSeqcvtGetCodeAsymS
*/
__deprecated ajint  ajSeqCvtKSColumn (const AjPSeqCvt cvt, const AjPStr ch)
{
    return ajSeqcvtGetCodeAsymS(cvt, ch);
}


/* @func ajSeqcvtGetLen *******************************************************
**
** Returns the length of a conversion table string (number of sequence
** characterers explicitly included)
**
** @param [r] cvt [const AjPSeqCvt] Conversion table
**
** @return [ajuint] Length
** @@
******************************************************************************/

ajuint ajSeqcvtGetLen(const AjPSeqCvt cvt)
{
    return cvt->len;
}


/* @obsolete ajSeqCvtLen
** @rename ajSeqcvtGetLen
*/
__deprecated ajint  ajSeqCvtLen(const AjPSeqCvt cvt)
{
    return ajSeqcvtGetLen(cvt);
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


/* @section modifiers ************************************************
**
** These functions update contents of a sequence date object.
**
** @fdata [AjPSeqDate]
** @fcategory modify
**
** @nam3rule Set Set sequence date properties
** @nam4rule SetCreate Set creation date
** @nam4rule SetModify Set modified date
** @nam4rule SetModseq Set sequence modified date
**
** @suffix C Date as a C string
** @suffix S Date as a string object
**
** @argrule Set date [AjPSeqDate] Sequence date object
** @argrule C datestr [const char*] Character string
** @argrule S datestr [const AjPStr] String
**
** @valrule * [AjBool] True on success
**
******************************************************************************/

/* @func ajSeqdateSetCreateS **************************************************
**
** Set the sequence creation date
**
** @param [w] date [AjPSeqDate] Sequence date
** @param [r] datestr [const AjPStr] Date string
** @return [AjBool] ajTrue on success
******************************************************************************/

AjBool ajSeqdateSetCreateS(AjPSeqDate date, const AjPStr datestr)
{
    if(!date)
	return ajFalse;

    seqDateSet(&date->CreDate, datestr);

    return ajTrue;
}

/* @func ajSeqdateSetModifyS **************************************************
**
** Set the entry modification date
**
** @param [w] date [AjPSeqDate] Sequence date
** @param [r] datestr [const AjPStr] Date string
** @return [AjBool] ajTrue on success
******************************************************************************/

AjBool ajSeqdateSetModifyS(AjPSeqDate date, const AjPStr datestr)
{
    if(!date)
	return ajFalse;

    seqDateSet(&date->ModDate, datestr);

    return ajTrue;
}


/* @func ajSeqdateSetModseqS **************************************************
**
** Set the sequence modification date
**
** @param [w] date [AjPSeqDate] Sequence date
** @param [r] datestr [const AjPStr] Date string
** @return [AjBool] ajTrue on success
******************************************************************************/

AjBool ajSeqdateSetModseqS(AjPSeqDate date, const AjPStr datestr)
{
    if(!date)
	return ajFalse;

    seqDateSet(&date->SeqDate, datestr);

    return ajTrue;
}

/* @funcstatic seqDateSet *****************************************************
**
** Sets a sequence date object
**
** @param [w] date [AjPTime*] Sequence date object
** @param [r] datestr [const AjPStr] Date as a string in EMBL/UniProt style
** @return [AjBool] ajTrue on success
******************************************************************************/

static AjBool seqDateSet(AjPTime* date, const AjPStr datestr)
{
    AjPStrTok handle = NULL;
    AjPStr tmpstr = NULL;
    const char* months[] = {
	"JAN", "FEB", "MAR", "APR", "MAY", "JUN",
	"JUL", "AUG", "SEP", "OCT", "NOV", "DEC", NULL};
    ajint day;
    ajint month;
    ajint year;

    ajStrTokenAssignC(&handle, datestr, "-");

    ajStrTokenNextParse(&handle, &tmpstr);
    ajStrToInt(tmpstr, &day);

    ajStrTokenNextParse(&handle, &tmpstr);
    month = 0;
    while(months[month] && !ajStrMatchC(tmpstr, months[month]))
	month++;

    if(!months[month])
	month = 1;
    else
	month++;

    ajStrTokenNextParse(&handle, &tmpstr);
    ajStrToInt(tmpstr, &year);
    if(year > 1900)
	year -= 1900;
    *date = ajTimeNewDayFmt("dtline", day, month, year);

    ajStrTokenDel(&handle);
    ajStrDel(&tmpstr);

    return ajTrue;
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

/* @func ajSeqrefNew *********************************************************
**
** Constructor for empty sequence citation object
**
** @return [AjPSeqRef] Empty sequence citation object
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
** @param [r] ref [const AjPSeqRef] Sequence date object
** @return [AjPSeqRef] Empty sequence date object
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




/* @func ajSeqrefDel *********************************************************
**
** Deletes a sequence citation object.
**
** @param [d] Pref [AjPSeqRef*] Sequence citation object
** @return [void]
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
** @nam4rule AppendGroupname Append to grouop name part of a citation
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
** @nam3rule Standard Standardize internal representation of a citation
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

/* @func ajSeqrefAppendAuthors ***********************************************
**
** Append to the Authors string of a citation
**
** @param [u] ref [AjPSeqRef] Sequence citation object
** @param [r] str [const AjPStr] Authors string
** @return [AjBool] True on success
******************************************************************************/

AjBool ajSeqrefAppendAuthors(AjPSeqRef ref, const AjPStr str)
{
    if(ajStrGetLen(ref->Authors))
	ajStrAppendK(&ref->Authors, ' ');
    ajStrAppendS(&ref->Authors, str);

    return ajTrue;
}

/* @func ajSeqrefAppendComment ***********************************************
**
** Append to the Comment string of a citation
**
** @param [u] ref [AjPSeqRef] Sequence citation object
** @param [r] str [const AjPStr] Comment string
** @return [AjBool] True on success
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

/* @func ajSeqrefAppendPosition ***********************************************
**
** Append to the position string of a citation
**
** @param [u] ref [AjPSeqRef] Sequence citation object
** @param [r] str [const AjPStr] Position string
** @return [AjBool] True on success
******************************************************************************/

AjBool ajSeqrefAppendPosition(AjPSeqRef ref, const AjPStr str)
{
    if(ajStrGetLen(ref->Position))
	ajStrAppendK(&ref->Position, ' ');
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
******************************************************************************/

AjBool ajSeqrefAppendTitle(AjPSeqRef ref, const AjPStr str)
{
    if(ajStrGetLen(ref->Title))
	ajStrAppendK(&ref->Title, ' ');

    ajStrAppendS(&ref->Title, str);

    return ajTrue;
}

/* @func ajSeqrefAppendXref ***********************************************
**
** Append to the Cross reference string of a citation
**
** @param [u] ref [AjPSeqRef] Sequence citation object
** @param [r] str [const AjPStr] Xref string
** @return [AjBool] True on success
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
******************************************************************************/

AjBool ajSeqrefFmtAuthorsEmbl(const AjPSeqRef ref, AjPStr* Pdest)
{
    ajStrAssignC(Pdest, "");

    if(!ref->Authors)
	return ajFalse;

    ajStrAssignS(Pdest, ref->Authors);

    return ajTrue;
}

/* @func ajSeqrefFmtAuthorsGb ***********************************************
**
** Return the Authors string of a citation in Genbank format
**
** @param [r] ref [const AjPSeqRef] Sequence citation object
** @param [w] Pdest [AjPStr*] Authors string in Genbank format
** @return [AjBool] True if author list exists
******************************************************************************/

AjBool ajSeqrefFmtAuthorsGb(const AjPSeqRef ref, AjPStr* Pdest)
{
    ajint i;
    ajint imax;
    char* cp;

    ajStrAssignC(Pdest, "");

    if(!ref->Authors)
	return ajFalse;

    ajStrAssignS(Pdest, ref->Authors);
    i = ajStrFindlastC(*Pdest, ","); /* replace , with and */
    if(i != -1) {
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
******************************************************************************/

AjBool ajSeqrefFmtLocationEmbl(const AjPSeqRef ref, AjPStr* Pdest)
{
    ajStrAssignC(Pdest, "");

    if(!ref->Location)
	return ajFalse;

    ajStrAssignS(Pdest, ref->Location);

    return ajTrue;
}

/* @func ajSeqrefFmtLocationGb ***********************************************
**
** Return the location string of a citation in Genbank format
**
** @param [r] ref [const AjPSeqRef] Sequence citation object
** @param [w] Pdest [AjPStr*] Authors string in Genbank format
** @return [AjBool] True if author list exists
******************************************************************************/

AjBool ajSeqrefFmtLocationGb(const AjPSeqRef ref, AjPStr* Pdest)
{
    ajStrAssignC(Pdest, "");

    if(!ref->Location)
	return ajFalse;

    ajStrAssignS(Pdest, ref->Location);

    ajStrExchangeCC(Pdest, "):", "), ");
    ajStrExchangeCC(Pdest, "(", "\1");
    ajStrExchangeCC(Pdest, "\1", " (");
    ajStrExchangeCC(Pdest, "  (", " (");

    return ajTrue;
}

/* @func ajSeqrefFmtTitleGb ***********************************************
**
** Return the title string of a citation in Genbank format
**
** @param [r] ref [const AjPSeqRef] Sequence citation object
** @param [w] Pdest [AjPStr*] Authors string in Genbank format
** @return [AjBool] True if author list exists
******************************************************************************/

AjBool ajSeqrefFmtTitleGb(const AjPSeqRef ref, AjPStr* Pdest)
{
    ajStrAssignC(Pdest, "");

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

/* @func ajSeqrefSetAuthors **************************************************
**
** Set the Authors string of a citation
**
** @param [u] ref [AjPSeqRef] Sequence citation object
** @param [r] str [const AjPStr] Authors string
** @return [AjBool] True on success
******************************************************************************/

AjBool ajSeqrefSetAuthors(AjPSeqRef ref, const AjPStr str)
{
    ajStrAssignS(&ref->Authors, str);

    return ajTrue;
}

/* @func ajSeqrefSetComment **************************************************
**
** Set the Comment string of a citation
**
** @param [u] ref [AjPSeqRef] Sequence citation object
** @param [r] str [const AjPStr] Comment string
** @return [AjBool] True on success
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

/* @datasection [AjPList] Reference list operations ***************************
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
** @return [AjBool] True on success#
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

/* @datasection [AjPStr] string tests *****************************************
**
** Functions handling strings for specialist sequence-related tests
**
** @nam2rule Seqtest
**
******************************************************************************/

/* @section string tests ********************************************
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


/* @func ajSeqtestIsAccession ************************************************
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


/* @obsolete ajIsAccession
** @rename ajSeqtestIsAccession
*/
__deprecated AjBool  ajIsAccession(const AjPStr accnum)
{
    return ajSeqtestIsAccession(accnum);
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




/* @obsolete ajIsSeqversion
** @rename ajSeqtestIsSeqversion
*/
__deprecated const AjPStr  ajIsSeqversion(const AjPStr sv)
{
    return ajSeqtestIsSeqversion(sv);
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
** @valrule CountGaps [ajuint] Number of gap chanacrters
******************************************************************************/

/* @func ajSeqstrCalcMolwt ****************************************************
**
** Calculates the molecular weight of a protein sequence.
**
** @param [r] seq [const AjPStr] Sequence
** @return [float] Molecular weight.
** @@
******************************************************************************/

float ajSeqstrCalcMolwt(const AjPStr seq)
{
    /* source: Biochemistry LABFAX */
    static double aa[26] = {     89.10, 132.61, 121.16, 133.11, /* A-D */
				147.13, 165.19,  75.07, 155.16,	/* E-H */
				131.18,   0.00, 146.19, 131.18,	/* I-L */
				149.21, 132.12,   0.00, 115.13,	/* M-P */
				146.15, 174.20, 105.09, 119.12,	/* Q-T */
				  0.00, 117.15, 204.23, 128.16,	/* U-X */
				181.19, 146.64};
    float mw;
    ajint i;
    const char* cp;

    cp = ajStrGetPtr(seq);
    mw = (float) 18.015;
    
    while(*cp)
    {
	i = toupper((ajint) *cp)-'A';
	if(i > 25 || i < 0)
	{
	    ajDebug("seqMW bad character '%c' %d\n", *cp, *cp);
	    i = 'X' - 'A';
	}
	mw += (float) aa[i] - (float) 18.015;
	cp++;
    }

    ajDebug("seqMW calculated %.2f\n", mw);

    return mw;
}


/* @obsolete ajSeqMW
** @rename ajSeqstrCalcMolwt
*/
__deprecated float  ajSeqMW(const AjPStr seq)
{
    return ajSeqstrCalcMolwt(seq);
}



/* @func ajSeqstrCountGaps ***************************************************
**
** Complements but does not reverse a nucleotide sequence provided as a string.
**
** @param [r] seq [const AjPStr] Sequence as a string.
**
** @return [ajuint] Number of gap characters
******************************************************************************/

ajuint ajSeqstrCountGaps(const AjPStr seq)
{
    ajuint ret = 0;

    static char testchars[] = "-~.? "; /* all known gap characters */
    char *testgap;

    testgap = testchars;

    while(*testgap)
    {
	ret += ajStrCalcCountK(seq, *testgap);
	testgap++;
    }

    return ret;
}

/* @obsolete ajSeqGapCountS
** @rename ajSeqstrCountGaps
*/
__deprecated ajint  ajSeqGapCountS(const AjPStr str)
{
    return ajSeqstrCountGaps(str);
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

/* @func ajSeqstrComplement ***********************************************
**
** Complements but does not reverse a nucleotide sequence provided as a string.
**
** @param [u] Pseq [AjPStr*] Sequence as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqstrComplement(AjPStr* Pseq)
{
    char *cp;

    cp = ajStrGetuniquePtr(Pseq);

    while(*cp)
    {
	*cp = ajBaseComp(*cp);
	cp++;
    }

    return;
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


/* @func ajSeqstrReverse ******************************************************
**
** Reverses and complements a nucleotide sequence provided as a string.
**
** @param [u] Pseq [AjPStr*] Sequence as a string.
** @return [void]
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
	tmp = ajBaseComp(*cp);
	*cp = ajBaseComp(*cq);
	*cq = tmp;
	cp++;
	cq--;
    }

    if(cp == cq)
	*cp = ajBaseComp(*cp);

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

/* @func ajSeqclsSetEmbl *****************************************************
**
** Sets the internal entry class for a sequence
**
** @param [u] Pcls [AjPStr*] Internal entry class name
** @param [r] clsembl [const AjPStr] EMBL database entry class
** @return [AjBool] True if a known type
******************************************************************************/

AjBool ajSeqclsSetEmbl(AjPStr* Pcls, const AjPStr clsembl)
{
    static AjBool called = AJFALSE;
    const AjPStr clsname = NULL;

    if(!called) {
	seqclsInit();
	called = ajTrue;
    }

    clsname = ajTableFetch(seqTableClsEmbl, clsembl);
    if(!clsname)
	return ajFalse;

    ajStrAssignS(Pcls, clsname);

    return ajTrue;
    
}


/* @func ajSeqclsSetGb *****************************************************
**
** Sets the internal entry class for a sequence
**
** @param [u] Pcls [AjPStr*] Internal entry class name
** @param [r] clsgb [const AjPStr] Genbank database entry class
** @return [AjBool] True if a known type
******************************************************************************/

AjBool ajSeqclsSetGb(AjPStr* Pcls, const AjPStr clsgb)
{
    static AjBool called = AJFALSE;
    const AjPStr clsname = NULL;

    if(!called) {
	seqclsInit();
	called = ajTrue;
    }

    clsname = ajTableFetch(seqTableClsGb, clsgb);
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
******************************************************************************/

const char* ajSeqclsGetEmbl(const AjPStr cls)
{
    static AjBool called = AJFALSE;
    SeqOClass *clsdef = NULL;

    if(!called)
    {
	seqclsInit();
	called = ajTrue;
    }

    ajDebug("ajSeqclsGetEmbl '%S'\n", cls);

    if(ajStrGetLen(cls))
	clsdef = ajTableFetch(seqTableCls, cls);

    if(!clsdef)
	clsdef = ajTableFetch(seqTableCls, seqClassDef);

    if(!clsdef)
	return ajStrGetPtr(seqClassDef);

    ajDebug("ajSeqclsGetEmbl '%S' => '%s'\n",
	    cls, clsdef->Embl);

    return clsdef->Embl;
    
}


/* @funcstatic seqclsInit *****************************************************
**
** Initialises the tables of molecule types
**
** @return [void]
******************************************************************************/

static void seqclsInit(void)
{
    ajuint i;
    AjPStr keystr = NULL;
    AjPStr valstr = NULL;

    if(seqTableCls)
	return;

    seqTableCls = ajTablestrNewCaseLen(16);
    seqTableClsEmbl = ajTablestrNewCaseLen(16);
    seqTableClsDdbj = ajTablestrNewCaseLen(16);
    seqTableClsGb = ajTablestrNewCaseLen(16);

    seqClassDef = ajStrNewC(seqClass[0].Name);

    for(i=0;seqClass[i].Name;i++)
    {
	keystr = ajStrNewC(seqClass[i].Name);
	if(ajTableFetch(seqTableCls, keystr))
	    ajStrDel(&keystr);
	else
	    ajTablePut(seqTableCls, keystr, &seqClass[i]);

	if(seqClass[i].Embl[0])
	{
	    valstr = ajStrNewC(seqClass[i].Embl);
	    if(ajTableFetch(seqTableClsEmbl, valstr))
		ajStrDel(&valstr);
	    else
		ajTablePut(seqTableClsEmbl, valstr, keystr);
	}

	if(seqClass[i].Ddbj[0])
	{
	    valstr = ajStrNewC(seqClass[i].Ddbj);
	    if(ajTableFetch(seqTableClsDdbj, valstr))
		ajStrDel(&valstr);
	    else
		ajTablePut(seqTableClsDdbj, valstr, keystr);
	}

	if(seqClass[i].Genbank[0])
	{
	    valstr = ajStrNewC(seqClass[i].Genbank);
	    if(ajTableFetch(seqTableClsGb, valstr))
		ajStrDel(&valstr);
	    else
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
******************************************************************************/

AjBool ajSeqdivSetEmbl(AjPStr* Pdivi, const AjPStr divembl)
{
    static AjBool called = AJFALSE;
    const AjPStr divname = NULL;

    if(!called) {
	seqdivInit();
	called = ajTrue;
    }

    divname = ajTableFetch(seqTableDivEmbl, divembl);
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
******************************************************************************/

AjBool ajSeqdivSetGb(AjPStr* Pdivi, const AjPStr divgb)
{
    static AjBool called = AJFALSE;
    const AjPStr divname = NULL;

    if(!called) {
	seqdivInit();
	called = ajTrue;
    }

    divname = ajTableFetch(seqTableDivGb, divgb);

    if(!divname)		/* Genbank mixes division and class */
	divname = ajTableFetch(seqTableClsGb, divgb);

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
******************************************************************************/

const char* ajSeqdivGetEmbl(const AjPStr divi)
{
    static AjBool called = AJFALSE;
    SeqODivision *divdef = NULL;

    if(!called)
    {
	seqdivInit();
	called = ajTrue;
    }

    ajDebug("ajSeqdivGetEmbl '%S'\n", divi);

    if(ajStrGetLen(divi))
	divdef = ajTableFetch(seqTableDiv, divi);

    if(!divdef)
	divdef = ajTableFetch(seqTableDiv, seqDivisionDef);

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
******************************************************************************/

const char* ajSeqdivGetGb(const AjPStr divi)
{
    static AjBool called = AJFALSE;
    SeqODivision *divdef = NULL;

    if(!called)
    {
	seqdivInit();
	called = ajTrue;
    }

    ajDebug("ajSeqdivGetGb '%S'\n", divi);

    if(ajStrGetLen(divi))
	divdef = ajTableFetch(seqTableDiv, divi);

    if(!divdef)
	divdef = ajTableFetch(seqTableDiv, seqDivisionDef);

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
******************************************************************************/

static void seqdivInit(void)
{
    ajuint i;
    AjPStr keystr = NULL;
    AjPStr valstr = NULL;

    if(seqTableDiv)
	return;

    seqTableDiv = ajTablestrNewCaseLen(16);
    seqTableDivEmbl = ajTablestrNewCaseLen(16);
    seqTableDivDdbj = ajTablestrNewCaseLen(16);
    seqTableDivGb = ajTablestrNewCaseLen(16);

    seqDivisionDef = ajStrNewC(seqDivision[0].Name);

    for(i=0;seqDivision[i].Name;i++)
    {
	keystr = ajStrNewC(seqDivision[i].Name);
	if(ajTableFetch(seqTableDiv, keystr))
	    ajStrDel(&keystr);
	else
	    ajTablePut(seqTableDiv, keystr, &seqDivision[i]);

	if(seqDivision[i].Embl[0])
	{
	    valstr = ajStrNewC(seqDivision[i].Embl);
	    if(ajTableFetch(seqTableDivEmbl, valstr))
		ajStrDel(&valstr);
	    else
		ajTablePut(seqTableDivEmbl, valstr, keystr);
	}

	if(seqDivision[i].Ddbj[0])
	{
	    valstr = ajStrNewC(seqDivision[i].Ddbj);
	    if(ajTableFetch(seqTableDivDdbj, valstr))
		ajStrDel(&valstr);
	    else
		ajTablePut(seqTableDivDdbj, valstr, keystr);
	}

	if(seqDivision[i].Genbank[0])
	{
	    valstr = ajStrNewC(seqDivision[i].Genbank);
	    if(ajTableFetch(seqTableDivGb, valstr))
		ajStrDel(&valstr);
	    else
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

/* @func ajSeqmolSetEmbl *****************************************************
**
** Sets the internal molecule type for a sequence
**
** @param [u] Pmol [AjPStr*] Sequence object
** @param [r] molembl [const AjPStr] Molecule type
** @return [AjBool] True if a known type
******************************************************************************/

AjBool ajSeqmolSetEmbl(AjPStr* Pmol, const AjPStr molembl)
{
    static AjBool called = AJFALSE;
    const AjPStr molname = NULL;

    if(!called) {
	seqmolInit();
	called = ajTrue;
    }

    molname = ajTableFetch(seqTableMolEmbl, molembl);
    if(!molname)
	return ajFalse;

    ajStrAssignS(Pmol, molname);

    return ajTrue;
    
}


/* @func ajSeqmolSetGb *****************************************************
**
** Sets the internal molecule type for a sequence
**
** @param [u] Pmol [AjPStr*] Sequence object
** @param [r] molgb [const AjPStr] Molecule type
** @return [AjBool] True if a known type
******************************************************************************/

AjBool ajSeqmolSetGb(AjPStr* Pmol, const AjPStr molgb)
{
    static AjBool called = AJFALSE;
    const AjPStr molname = NULL;

    if(!called) {
	seqmolInit();
	called = ajTrue;
    }

    molname = ajTableFetch(seqTableMolGb, molgb);
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
******************************************************************************/

const char* ajSeqmolGetEmbl(const AjPStr mol)
{
    static AjBool called = AJFALSE;
    SeqOMolecule *moldef = NULL;

    if(!called)
    {
	seqmolInit();
	called = ajTrue;
    }

    ajDebug("ajSeqMoleculeGetEmbl '%S'\n", mol);

    if(ajStrGetLen(mol))
	moldef = ajTableFetch(seqTableMol, mol);

    if(!moldef)
	moldef = ajTableFetch(seqTableMol, seqMoleculeDef);

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
******************************************************************************/

const char* ajSeqmolGetGb(const AjPStr mol)
{
    static AjBool called = AJFALSE;
    SeqOMolecule *moldef = NULL;

    if(!called)
    {
	seqmolInit();
	called = ajTrue;
    }

    ajDebug("ajSeqMoleculeGetGb '%S'\n", mol);

    if(ajStrGetLen(mol))
	moldef = ajTableFetch(seqTableMol, mol);

    if(!moldef)
	moldef = ajTableFetch(seqTableMol, seqMoleculeDef);

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
******************************************************************************/

static void seqmolInit(void)
{
    ajuint i;
    AjPStr keystr = NULL;
    AjPStr valstr = NULL;
 
    if(seqTableMol)
	return;

    seqTableMol = ajTablestrNewCaseLen(16);
    seqTableMolEmbl = ajTablestrNewCaseLen(16);
    seqTableMolDdbj = ajTablestrNewCaseLen(16);
    seqTableMolGb = ajTablestrNewCaseLen(16);

    seqMoleculeDef = ajStrNewC(seqMolecule[0].Name);

    for(i=0;seqMolecule[i].Name;i++)
    {
	keystr = ajStrNewC(seqMolecule[i].Name);
	if(ajTableFetch(seqTableMol, keystr))
	    ajStrDel(&keystr);
	else
	    ajTablePut(seqTableMol, keystr, &seqMolecule[i]);

	if(seqMolecule[i].Embl[0])
	{
	    valstr = ajStrNewC(seqMolecule[i].Embl);
	    if(ajTableFetch(seqTableMolEmbl, valstr))
		ajStrDel(&valstr);
	    else
		ajTablePut(seqTableMolEmbl, valstr, keystr);
	}

	if(seqMolecule[i].Ddbj[0])
	{
	    valstr = ajStrNewC(seqMolecule[i].Ddbj);
	    if(ajTableFetch(seqTableMolDdbj, valstr))
		ajStrDel(&valstr);
	    else
		ajTablePut(seqTableMolDdbj, valstr, keystr);
	}

	if(seqMolecule[i].Genbank[0])
	{
	    valstr = ajStrNewC(seqMolecule[i].Genbank);
	    if(ajTableFetch(seqTableMolGb, valstr))
		ajStrDel(&valstr);
	    else
		ajTablePut(seqTableMolGb, valstr, keystr);
	}

    }

    return;
}

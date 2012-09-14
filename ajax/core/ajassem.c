/* @source ajassem ************************************************************
**
** AJAX assembly functions
**
** These functions control all aspects of AJAX assembly
** parsing and include simple utilities.
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.28 $
** @modified Oct 5 pmr First version
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

#include "ajassem.h"
#include "ajassemread.h"
#include "ajassemwrite.h"
#include "ajtext.h"
#include "ajseqbam.h"
#include "ajcigar.h"



static AjPStr assemTempQry = NULL;

static void assemMakeQry(const AjPAssem thys, AjPStr* qry);
static void assemReadgroupDel(AjPAssemReadgroup *Prg);



/* @conststatic platfNames ****************************************************
**
** Platform types as enumerated in SAM spec
**
******************************************************************************/

static const char* platfNames[] =
{
 "unknown", "capillary", "LS454", "illumina", "solid",
 "helicos", "iontorrent", "pacbio", NULL
};


/* @conststatic sortorderNames ************************************************
**
** Sort-order names as enumerated in SAM spec
**
******************************************************************************/

static const char* sortorderNames[] =
{
 "unknown", "unsorted", "queryname", "coordinate", NULL
};




/* @filesection ajassem *******************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
******************************************************************************/





/* @datasection [AjPAssem] Assembly data **************************************
**
** Function is for manipulating assembly data objects
**
** @nam2rule Assem Assem data objects
**
******************************************************************************/




/* @section constructors ******************************************************
**
** Constructors
**
** @fdata [AjPAssem]
**
** @nam3rule New Constructor
**
** @valrule * [AjPAssem] Assem data object
**
** @fcategory new
**
******************************************************************************/




/* @func ajAssemNew ***********************************************************
**
** Assem data constructor
**
** @return [AjPAssem] New object
**
** @release 6.4.0
** @@
******************************************************************************/

AjPAssem ajAssemNew(void)
{
    AjPAssem ret;

    AJNEW0(ret);

    ret->Contigs = ajTablestrNewCaseConst(16);
    ret->ContigsIgnored = ajTablestrNew(10);

    ret->ContigsOrder = ajListNew();
    ret->Reads = ajListNew();

    ret->Readgroups = ajTablestrNewConst(16);
    ajTableSetDestroyvalue(ret->Readgroups,
	    (void(*)(void**))&assemReadgroupDel);

    return ret;
}




/* @section Assem data destructors ********************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the assembly data object.
**
** @fdata [AjPAssem]
**
** @nam3rule Del Destructor
**
** @argrule Del Passem [AjPAssem*] Assem data
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajAssemDel ***********************************************************
**
** Assem data destructor
**
** @param [d] Passem       [AjPAssem*]  Assem data object to delete
** @return [void] 
**
** @release 6.4.0
** @@
******************************************************************************/

void ajAssemDel(AjPAssem *Passem)
{
    AjPAssem assem = NULL;
    AjPAssemContig contig = NULL;
    AjPAssemRead r = NULL;
    AjPAssemTag tag = NULL;
    AjPAssemContig* contigs = NULL;
    ajint i = 0;

    if(!Passem) return;
    if(!(*Passem)) return;

    assem = *Passem;

    ajStrDel(&assem->Id);
    ajStrDel(&assem->Db);
    ajStrDel(&assem->Setdb);
    ajStrDel(&assem->Full);
    ajStrDel(&assem->Qry);
    ajStrDel(&assem->Formatstr);
    ajStrDel(&assem->Filename);
    ajStrDel(&assem->Textptr);

    ajSeqBamHeaderDel(&assem->BamHeader);

    while(ajListPop(assem->Reads, (void**) &r))
    {
	ajAssemreadDel(&r);
    }
    ajListFree(&assem->Reads);

    ajTableToarrayValues(assem->Contigs, (void***)&contigs);

    while (contigs[i])   /* contigs */
    {
    	contig = contigs[i++];
	ajStrDel(&contig->Name);
	ajStrDel(&contig->Consensus);
	ajStrDel(&contig->ConsensusQ);
	ajStrDel(&contig->AssemblyID);
	ajStrDel(&contig->URI);
	ajStrDel(&contig->Species);
	ajStrDel(&contig->MD5);

	while(ajListPop(contig->Tags, (void**) &tag))
	{
	    ajStrDel(&tag->Comment);
	    ajStrDel(&tag->Name);
	    AJFREE(tag);
	}
	ajListFree(&contig->Tags);

	AJFREE(contig);
    }

    AJFREE(assem->ContigArray);

    ajTableDel(&assem->Contigs);
    ajTableFree(&assem->ContigsIgnored);

    ajListFree(&assem->ContigsOrder);

    ajTableDel(&assem->Readgroups);

    AJFREE(*Passem);
    AJFREE(contigs);
    *Passem = NULL;

    return;
}




/* @section Casts *************************************************************
**
** Return values from an assembly data object
**
** @fdata [AjPAssem]
**
** @nam3rule Get Return a value
** @nam4rule Id Identifier string
** @nam4rule Qry Return a query field
** @nam4rule Sortorder Sort order
** @suffix C Character string result
** @suffix S String object result
**
** @argrule * assem [const AjPAssem] Assem data object.
**
** @valrule *C [const char*] Query as a character string.
** @valrule *S [const AjPStr] Query as a string object.
** @valrule *GetId [const AjPStr] Identifier string
**
** @fcategory cast
**
******************************************************************************/




/* @func ajAssemGetId *********************************************************
**
** Return the identifier
**
** @param [r] assem [const AjPAssem] Assembly
**
** @return [const AjPStr] Returned id
**
**
** @release 6.5.0
******************************************************************************/

const AjPStr ajAssemGetId(const AjPAssem assem)
{
    return assem->Id;
}




/* @func ajAssemGetQryC *******************************************************
**
** Returns the query string of an assembly data object.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] assem [const AjPAssem] Assem data object.
** @return [const char*] Query as a character string.
**
** @release 6.4.0
** @@
******************************************************************************/

const char* ajAssemGetQryC(const AjPAssem assem)
{
    return MAJSTRGETPTR(ajAssemGetQryS(assem));
}




/* @func ajAssemGetQryS *******************************************************
**
** Returns the query string of an assembly data object.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] assem [const AjPAssem] Assem data object.
** @return [const AjPStr] Query as a string.
**
** @release 6.4.0
** @@
******************************************************************************/

const AjPStr ajAssemGetQryS(const AjPAssem assem)
{
    ajDebug("ajAssemGetQryS '%S'\n", assem->Qry);

    if(ajStrGetLen(assem->Qry))
	return assem->Qry;

    assemMakeQry(assem, &assemTempQry);

    return assemTempQry;
}




/* @func ajAssemGetSortorderC *************************************************
**
** Returns sort-order name of the given assembly
**
** @param [r] assem [const AjPAssem] assembly
** @return [const char*] sort-order name
**
**
** @release 6.5.0
******************************************************************************/

const char* ajAssemGetSortorderC(const AjPAssem assem)
{
    if(!assem)
	return NULL;

    return sortorderNames[assem->SO];
}




/* @funcstatic assemMakeQry ***************************************************
**
** Sets the query for an assembly data object.
**
** @param [r] thys [const AjPAssem] Assem data object
** @param [w] qry [AjPStr*] Query string in full
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

static void assemMakeQry(const AjPAssem thys, AjPStr* qry)
{
    ajDebug("assemMakeQry (Id <%S> Formatstr <%S> Db <%S> "
	    "Filename <%S>)\n",
	    thys->Id, thys->Formatstr, thys->Db,
	    thys->Filename);

    /* ajAssemTrace(thys); */

    if(ajStrGetLen(thys->Db))
	ajFmtPrintS(qry, "%S-id:%S", thys->Db, thys->Id);
    else
    {
	ajFmtPrintS(qry, "%S::%S:%S", thys->Formatstr,
                    thys->Filename,thys->Id);
    }

    ajDebug("      result: <%S>\n",
	    *qry);

    return;
}




/* @section assembly data modifiers *******************************************
**
** Assem data modifiers
**
** @fdata [AjPAssem]
**
** @nam3rule Clear clear internal values
** @nam3rule Reset Reset to empty list of reads
** @nam3rule Set   Set internals
** @nam4rule SetContigattributes   Set contig attributes
** @nam4rule SetReadgroups         Set read group attributes
**
** @argrule *   assem [AjPAssem]       Assem data object
** @argrule Set tags  [const AjPTable] Table of tables for tags
**
** @valrule * [void]
**
** @fcategory modify
**
******************************************************************************/




/* @func ajAssemClear *********************************************************
**
** Resets all data for an assembly data object so that it can be reused.
**
** @param [u] assem [AjPAssem] assem data
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajAssemClear(AjPAssem assem)
{
    AjPAssemRead r = NULL;

    if(MAJSTRGETLEN(assem->Id))
       ajStrSetClear(&assem->Id);

    if(MAJSTRGETLEN(assem->Db))
       ajStrSetClear(&assem->Db);

    if(MAJSTRGETLEN(assem->Setdb))
       ajStrSetClear(&assem->Setdb);

    if(MAJSTRGETLEN(assem->Full))
       ajStrSetClear(&assem->Full);

    if(MAJSTRGETLEN(assem->Qry))
       ajStrSetClear(&assem->Qry);

    if(MAJSTRGETLEN(assem->Formatstr))
       ajStrSetClear(&assem->Formatstr);

    if(MAJSTRGETLEN(assem->Filename))
       ajStrSetClear(&assem->Filename);

    ajStrDel(&assem->Textptr);

    while(ajListPop(assem->Reads, (void**) &r))
    {
	ajAssemreadDel(&r);
    }

    assem->Count = 0;
    assem->Fpos = 0L;
    assem->Format = 0;

    assem->Hasdata = ajFalse;

    return;
}




/* @func ajAssemReset *********************************************************
**
** Resets read data for an assembly data object so that it can be reused.
**
** @param [u] assem [AjPAssem] assem data
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ajAssemReset(AjPAssem assem)
{
    AjPAssemRead r = NULL;

    while(ajListPop(assem->Reads, (void**) &r))
    {
	ajAssemreadDel(&r);
    }

    return;
}




/* @func ajAssemSetContigattributes *******************************************
**
** Set contig attributes uri, md5, species and assemblyid
** using the values provided in contigtags table.
**
** @param [u] assem [AjPAssem] Assembly object
** @param [r] tags [const AjPTable] Table of tables for contig tags.
** 				    Main table have an entry for each contig.
** 				    Sub-tables include tag values for a contig.
** @return [void]
**
**
** @release 6.5.0
******************************************************************************/

void ajAssemSetContigattributes(AjPAssem assem, const AjPTable tags)
{
    char** refseqnames=NULL;
    char* refseq;
    AjPTable* tagtables=NULL;
    AjPAssemContig c = NULL;
    AjPTable hline;
    AjPTable contigs;
    int i=0;

    if(tags == 0 || assem==0)
	return;

    ajDebug("assemSetContigAttributes\n");

    contigs = assem->Contigs;

    ajTableToarrayKeysValues(tags,
			    (void***)&refseqnames, (void***)&tagtables);

    while(refseqnames[i])
    {
	refseq = refseqnames[i];
	hline = tagtables[i++];

	c = ajTableFetchmodC(contigs, refseq);

	if(!c)
	{
	    ajDebug("ref seq '%s' listed in contigstags table "
		    "but not in actual contig list of the assembly",
		    refseq);
	    continue;
	}

	if(ajTableFetchC(hline,"UR"))
	    c->URI  = ajStrNewC(ajTableFetchmodC(hline,"UR"));

	if(ajTableFetchC(hline,"M5"))
	    c->MD5 = ajStrNewC(ajTableFetchmodC(hline,"M5"));

	if(ajTableFetchC(hline,"SP"))
	    c->Species = ajStrNewC(ajTableFetchmodC(hline,"SP"));

	if(ajTableFetchC(hline,"AS"))
	    c->AssemblyID  = ajStrNewC(ajTableFetchmodC(hline,"AS"));


    }

    AJFREE(refseqnames);
    AJFREE(tagtables);

    return;
}




/* @func ajAssemSetReadgroups *************************************************
**
** Set assembly readgroups using the values in readgrouptags table
**
** @param [u] assem [AjPAssem] Assembly object
** @param [r] tags [const AjPTable] Table of tables for readgroup tags.
** 				Main table have an entry for each read-group.
** 				Sub-tables include tag values for a read-group.
** @return [void]
**
**
** @release 6.5.0
******************************************************************************/

void ajAssemSetReadgroups(AjPAssem assem, const AjPTable tags)
{
    char** rgids=NULL;
    AjPTable* tagtables=NULL;
    AjPAssemReadgroup rg = NULL;
    AjPTable hline = NULL;
    AjPStr tmp = NULL;
    int i=0;

    if(tags == 0 || assem==0)
	return;

    ajDebug("assemSetReadgroups\n");

    ajTableToarrayKeysValues(tags,
			    (void***)&rgids, (void***)&tagtables);

    while(rgids[i])
    {
	AJNEW0(rg);

	rg->ID = ajStrNewC(rgids[i]);

	hline = tagtables[i++];


	if(ajTableFetchC(hline,"CN"))
	    rg->CN  = ajStrNewC(ajTableFetchmodC(hline,"CN"));

	if(ajTableFetchC(hline,"DS"))
	    rg->Desc = ajStrNewC(ajTableFetchmodC(hline,"DS"));

	if(ajTableFetchC(hline,"DT"))
	    rg->Date = ajStrNewC(ajTableFetchmodC(hline,"DT"));

	if(ajTableFetchC(hline,"FO"))
	    rg->FlowOrder = ajStrNewC(ajTableFetchmodC(hline,"FO"));

	if(ajTableFetchC(hline,"KS"))
	    rg->KeySeq = ajStrNewC(ajTableFetchmodC(hline,"KS"));

	if(ajTableFetchC(hline,"LB"))
	    rg->Library = ajStrNewC(ajTableFetchmodC(hline,"LB"));

	if(ajTableFetchC(hline,"PG"))
	    rg->Programs = ajStrNewC(ajTableFetchmodC(hline,"PG"));

	if(ajTableFetchC(hline,"PI"))
	{
	    tmp = ajStrNewC(ajTableFetchmodC(hline,"PI"));
	    ajStrToInt(tmp, &rg->Isize);
	    ajStrDel(&tmp);
	}

	if(ajTableFetchC(hline,"PL"))
	    rg->Platform = ajAssemplatformGetType(
		    ajTableFetchmodC(hline,"PL"));

	if(ajTableFetchC(hline,"PU"))
	    rg->Unit = ajStrNewC(ajTableFetchmodC(hline,"PU"));

	if(ajTableFetchC(hline,"SM"))
	    rg->Sample = ajStrNewC(ajTableFetchmodC(hline,"SM"));

	ajTablePut(assem->Readgroups, rg->ID, rg);
    }

    AJFREE(rgids);
    AJFREE(tagtables);

    return;
}




/* @datasection [AjPAssemRead] Assembly read data *****************************
**
** Function is for manipulating assembly read objects
**
** @nam2rule Assemread Assembly read data objects
**
******************************************************************************/




/* @section Assembly read data destructors ************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the assembly read data object.
**
** @fdata [AjPAssemRead]
**
** @nam3rule Del Destructor
**
** @argrule Del Passemread [AjPAssemRead*] Assem data
**
** @valrule * [void]
**
** @fcategory delete
**
******************************************************************************/




/* @func ajAssemreadDel *******************************************************
**
** Delete assembly read/alignment records
**
** @param [d] Passemread [AjPAssemRead*] Assembly read object to delete
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ajAssemreadDel(AjPAssemRead *Passemread)
{
    AjPAssemRead r = NULL;
    AjPAssemTag tag = NULL;

    if(!Passemread) return;
    if(!(*Passemread)) return;

    r = *Passemread;

    ajStrDel(&r->File);
    ajStrDel(&r->Name);
    ajStrDel(&r->Cigar);
    ajStrDel(&r->Seq);
    ajStrDel(&r->SeqQ);
    ajStrDel(&r->Technology);
    ajStrDel(&r->Template);

    while(ajListPop(r->Tags, (void**) &tag))
    {
	ajStrDel(&tag->Comment);
	ajStrDel(&tag->Name);
	AJFREE(tag);
    }
    ajListFree(&r->Tags);

    AJFREE(r);

    *Passemread = NULL;

    return;
}




/* @section Casts *************************************************************
**
** Return values from an assembly read data object
**
** @fdata [AjPAssemRead]
**
** @nam3rule Get Return a value
** @nam4rule Alignmentend   End of alignment
** @nam4rule Alignmentstart Start of alignment
** @nam4rule GetFlag Return a flagvalue
** @nam5rule Firstofpair Return forward paired read flag
** @nam5rule Negativestrand Return true if on the negative (reverse) strand
** @nam5rule Paired Return paired read flag
** @nam5rule Unmapped Return unmapped read flag
**
** @argrule * r [const AjPAssemRead] Assem read data object.
**
** @valrule * [AjBool] Flag boolean value
** @valrule *Alignmentend [ajint] Flag boolean value
** @valrule *Alignmentstart [ajint] Flag boolean value
**
** @fcategory cast
**
******************************************************************************/




/* @func ajAssemreadGetAlignmentend *******************************************
**
** 1-based inclusive rightmost position of the clipped sequence,
** or 0 if read unmapped
**
** @param [r] r [const AjPAssemRead] read
** @return [ajint] end position of the alignment on the reference sequence
**
**
** @release 6.5.0
******************************************************************************/

ajint ajAssemreadGetAlignmentend(const AjPAssemRead r)
{
    AjPCigar cigar = NULL;
    ajint ret=0;

    if(ajAssemreadGetFlagUnmapped(r))
	return 0;

    cigar = ajCigarNewS(r->Cigar);

    ret = r->y1 + ajCigarGetReferenceLength(cigar);

    ajCigarDel(&cigar);

    return ret;
}




/* @func ajAssemreadGetAlignmentstart *****************************************
**
** Return alignment start (1-based)
**
** @param [r] r [const AjPAssemRead] read
** @return [ajint] start position of the alignment on the reference sequence
**
**
** @release 6.5.0
******************************************************************************/

ajint ajAssemreadGetAlignmentstart(const AjPAssemRead r)
{
    if(ajAssemreadGetFlagUnmapped(r))
	return 0;

    if(r->Reversed)
	return r->y1;

    return r->x1;
}




/* @func ajAssemreadGetFlagFirstofpair ****************************************
**
** Return whether the read is the first read in a pair
**
** @param [r] r [const AjPAssemRead] read
** @return [AjBool] true if the read is the first read in a pair
**
**
** @release 6.5.0
******************************************************************************/

AjBool ajAssemreadGetFlagFirstofpair(const AjPAssemRead r)
{
    return (r->Flag & BAM_FREAD1) != 0;
}




/* @func ajAssemreadGetFlagNegativestrand *************************************
**
** Return strand of the query (false for forward; true for reverse strand)
**
** @param [r] r [const AjPAssemRead] read
** @return [AjBool] true if the read is reverse strand
**
**
** @release 6.5.0
******************************************************************************/

AjBool ajAssemreadGetFlagNegativestrand(const AjPAssemRead r)
{
    return (r->Flag & BAM_FREVERSE) != 0;
}




/* @func ajAssemreadGetFlagPaired *****************************************
**
** Return whether the read is paired in sequencing,
** no matter whether it is mapped in a pair
**
** @param [r] r [const AjPAssemRead] read
** @return [AjBool] true if the read is paired
**
**
** @release 6.5.0
******************************************************************************/

AjBool ajAssemreadGetFlagPaired(const AjPAssemRead r)
{
    return (r->Flag & BAM_FPAIRED) != 0;
}




/* @func ajAssemreadGetFlagUnmapped *******************************************
**
** Return whether the read is unmapped
**
** @param [r] r [const AjPAssemRead] read
** @return [AjBool] true if the read is unmapped
**
**
** @release 6.5.0
******************************************************************************/

AjBool ajAssemreadGetFlagUnmapped(const AjPAssemRead r)
{
    return (r->Flag & BAM_FUNMAP) != 0;
}




/* @section assembly read data modifiers **************************************
**
** Assem data modifiers
**
** @fdata [AjPAssemRead]
**
** @nam3rule Set Set internal values
** @nam4rule Alignblocks Blocks of read aligned directly to reference
**
** @argrule * r [AjPAssemRead] Assem read data object.
** @valrule *Alignblocks [AjPList] List if blocks from internal structures
**
** @fcategory modify
**
******************************************************************************/




/* @func ajAssemreadSetAlignblocks ********************************************
**
** Returns blocks of the read sequence that have been aligned directly to the
** reference sequence. Clipped portions of the read and inserted and deleted
** bases (vs. the reference) are not represented in the alignment blocks.
**
** Results are stored in the AlignmentBlocks attribute for reuse.
**
** @param [u] r [AjPAssemRead] read/alignment
*8
** @return [AjPList] list of alignment blocks
**
** @release 6.5.0
******************************************************************************/

AjPList ajAssemreadSetAlignblocks(AjPAssemRead r)
{
    AjPCigar cigar = NULL;
    AjPList alignmentBlocks=NULL;
    int readBase=0;
    int refBase=0;
    int i=0;
    int length=0;
    AjPAssemReadalignmentblock ablock=NULL;

    if (r->AlignmentBlocks != NULL)
	return r->AlignmentBlocks;

    /* todo: we should store the cigar object as part of the read? */
    cigar = ajCigarNewS(r->Cigar);

    if (cigar == NULL)
	return NULL;


    alignmentBlocks = ajListNew();
    readBase = 1;
    refBase  = ajAssemreadGetAlignmentstart(r);

    for (i=0; i<cigar->n; i++)
    {
	switch (cigar->cigoperator[i])
	{
	    case 'H' :
		break; /* ignore hard clips */
	    case 'P' :
		break; /* ignore pads */
	    case 'S' :
		readBase += cigar->length[i];
		break; /* soft clip read bases */
	    case 'N' :
		refBase += cigar->length[i];
		break;  /* reference skip */
	    case 'D' :
		refBase += cigar->length[i];
		break;
	    case 'I' :
		readBase += cigar->length[i];
		break;
	    case 'M' :
	    case '=' :
	    case 'X' :
		length = cigar->length[i];
		AJNEW0(ablock);
		ablock->readStart = readBase;

		if(refBase<=0)
		    ajErr("referenceBlockStart: %d is <= 0", refBase);

		ablock->referenceStart = refBase;
		ablock->length = length;
		ajListPushAppend(alignmentBlocks, ablock);
		ablock=NULL;
		readBase += length;
		refBase  += length;
		break;
	    default :
		ajErr("unrecognised cigar op: " + cigar->cigoperator[i]);
	}
    }

    ajCigarDel(&cigar);

    r->AlignmentBlocks = alignmentBlocks;

    return alignmentBlocks;
}




/* @datasection [AjPAssemReadgroup] Assembly read group data ******************
**
** Function is for manipulating assembly data objects
**
** @nam2rule Assemreadgroup Assembly read group objects
**
******************************************************************************/




/* @section Assembly read group functions *************************************
**
** Function is for manipulating read group data
**
** @fdata [AjPAssemReadgroup]
**
** @nam3rule Get           Return value
** @nam4rule Platformname  Return platform name
**
** @argrule  Get  rg [const AjPAssemReadgroup] read group
**
** @valrule * [const char*] platform name
**
** @fcategory misc
******************************************************************************/




/* @funcstatic assemReadgroupDel **********************************************
**
** Delete assembly read-group data
**
** @param [d] Prg [AjPAssemReadgroup*]  read-group object to delete
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

static void assemReadgroupDel(AjPAssemReadgroup *Prg)
{
    AjPAssemReadgroup rg = NULL;

    if(!Prg || !(*Prg))
	return;

    rg = *Prg;

    ajStrDel(&rg->CN);
    ajStrDel(&rg->Date);
    ajStrDel(&rg->Desc);
    ajStrDel(&rg->FlowOrder);
    ajStrDel(&rg->ID);
    ajStrDel(&rg->KeySeq);
    ajStrDel(&rg->Library);
    ajStrDel(&rg->Programs);
    ajStrDel(&rg->Sample);
    ajStrDel(&rg->Unit);

    AJFREE(rg);

    *Prg = NULL;

    return;
}




/* @func ajAssemreadgroupGetPlatformname **************************************
**
** Returns platform name of the given read-group
**
** @param [r] rg [const AjPAssemReadgroup] read group
** @return [const char*] platform name
**
**
** @release 6.5.0
******************************************************************************/

const char* ajAssemreadgroupGetPlatformname(const AjPAssemReadgroup rg)
{
    if(!rg)
	return NULL;

    return platfNames[rg->Platform];
}




/* @datasection [AjEAssemPlatform] Assembly platform **************************
**
** Function is for manipulating assembly platform enumerations
**
** @nam2rule Assemplatform Assembly platform enumerations
**
******************************************************************************/




/* @section Assembly platform functions ***************************************
**
** Function is for manipulating assembly platform enumerations
**
** @fdata [AjEAssemPlatform]
**
** @nam3rule Get           Return value
** @nam4rule Type          Return platform type
**
** @argrule  Get name [const char*] Platform name
**
** @valrule * [AjEAssemPlatform] Platform type enumeration
**
** @fcategory misc
******************************************************************************/




/* @func ajAssemplatformGetType ***********************************************
**
** Returns platform type enumeration for the given platform
**
** @param [r] name [const char*] platform name
** @return [AjEAssemPlatform] platform type
**
**
** @release 6.5.0
******************************************************************************/

AjEAssemPlatform ajAssemplatformGetType(const char* name)
{
    int i=0;

    while(platfNames[i])
    {
	if(ajCharCmpCase(platfNames[i], name)==0)
	    return i;
	i++;
    }

    return 0;
}




/* @datasection [AjEAssemSortOrder] Assembly sort order ***********************
**
** Function is for manipulating assembly sort order enumerations
**
** @nam2rule Assemsortorder Assembly sort order enumerations
**
******************************************************************************/




/* @section Assembly sort order functions *************************************
**
** Function is for manipulating assembly sort-order enumerations
**
** @fdata [AjEAssemSortOrder]
**
** @nam3rule Get           Return value
** @nam4rule Type          Return sort order type
**
** @argrule  Get name [const char*] sort-order name
**
** @valrule * [AjEAssemSortOrder] Assembly sort order type enumeration
**
** @fcategory misc
******************************************************************************/




/* @func ajAssemsortorderGetType **********************************************
**
** Returns sort-order type for the given sort-order-name
**
** @param [r] name [const char*] sort order name
** @return [AjEAssemSortOrder] sort order type
**
**
** @release 6.5.0
******************************************************************************/

AjEAssemSortOrder ajAssemsortorderGetType(const char* name)
{
    int i=0;

    if(!name)
	return 0;

    while(sortorderNames[i])
    {
	if(ajCharCmpCase(sortorderNames[i], name)==0)
	    return i;
	i++;
    }

    return 0;
}




/* @datasection [none] Miscellaneous functions ********************************
**
** Functions to initialise and clean up internals
**
** @nam2rule Assem Assem internals
**
******************************************************************************/




/* @section exit **************************************************************
**
** Functions called on exit from the program by ajExit to do
** any necessary cleanup and to report internal statistics to the debug file
**
** @fdata      [none]
** @fnote     general exit functions, no arguments
**
** @nam3rule Exit Cleanup and report on exit
**
** @valrule * [void]
**
** @fcategory misc
******************************************************************************/




/* @func ajAssemExit **********************************************************
**
** Cleans up assembly processing internal memory
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ajAssemExit(void)
{
    ajAsseminExit();
    ajAssemoutExit();

    return;
}




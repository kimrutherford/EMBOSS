/******************************************************************************
** @source AJAX ALIGN (ajax alignment output) functions
**
** These functions align AJAX sequences and report them in a variety
** of formats.
**
** @author Copyright (C) 2001 Peter Rice, LION Bioscience Ltd.
** @version 1.0
** @modified Aug 21 2001 First version
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

#include <stddef.h>
#include <stdarg.h>
#include <float.h>
#include <limits.h>
#include <math.h>

#include "ajax.h"




/* @datastatic AlignPData *****************************************************
**
** Ajax Align Data object.
**
** Holds definition of feature alignment data. Stored in AjPAlign structure as
** an AjPList.
**
** @alias AlignSData
** @alias AlignOData
**
** @attr Start [ajint*] Start position for each sequence
** @attr End [ajint*] End position for each sequence
** @attr Len [ajint*] Original length for each sequence
** @attr Offset [ajint*] Offset for numbering start of each sequence
** @attr Offend [ajint*] Offset for numbering end of each sequence
** @attr SubOffset [ajint*] Offset of used subsequence within given sequence
**                          generally for cases where SeqExternal is set
**                          in the AjPAlign object that includes this
**                          AlignPData object. Used for fetching the
**                          actual sequence from the stored sequence,
**                          but ignored when calculating position.
** @attr Rev [AjBool*] Reverse each sequence (so far not used)
** @attr RealSeq [AjPSeq*] Sequences usually as copies of data from the
**                     application unless SeqExternal in the AjPAlign
**                     object is true
** @attr Seq [const AjPSeq*] Pointer to sequence in RealSeq
**                           unless SeqExternal in the AjPAlign
**                           object is true when pointer is to
**                           sequence elsewhere
** @attr LenAli [ajint] Length of the alignment
** @attr NumId [ajint] Number of identical positions, usually calculated
** @attr NumSim [ajint] Number of similar positions, usually calculated
** @attr NumGap [ajint] Number of gap positions, usually calculated
** @attr Score [AjPStr] Score to be reported - stored as a string for output
**                      set by functions using ajint or float input
** @attr Nseqs [ajint] Number of sequences, size of each array. This is
**                     duplicated in the AjPAlign object but is useful
**                     here for constructor and destructor efficiency.
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AlignSData
{
    ajint* Start;
    ajint* End;
    ajint* Len;
    ajint* Offset;
    ajint* Offend;
    ajint* SubOffset;
    AjBool* Rev;
    AjPSeq* RealSeq;
    const AjPSeq* Seq;
    ajint LenAli;
    ajint NumId;
    ajint NumSim;
    ajint NumGap;
    AjPStr Score;
    ajint  Nseqs;
    char Padding[4];
} AlignOData;

#define AlignPData AlignOData*




/* @datastatic AlignPFormat ***************************************************
**
** Ajax Align Formats
**
** List of alignment output formats available
**
** @alias AlignSFormat
** @alias AlignOFormat
**
** @attr Name [const char*] format name
** @attr Desc [const char*] Format description
** @attr Alias [AjBool] Name is an alias for an identical definition
** @attr Nucleotide [AjBool] ajTrue if format can work with nucleotide sequences
** @attr Protein [AjBool] ajTrue if format can work with protein sequences
** @attr Showheader [AjBool] ajTrue if header appears in output
** @attr Showseqs [AjBool] ajTrue if sequences appear in output
** @attr Padding [ajint] Padding to alignment boundary
** @attr Minseq [ajint] Minimum number of sequences, 2 for pairwise
** @attr Maxseq [ajint] Maximum number of sequences, 2 for pairwise
** @attr Write [(void*)] Function to write alignment
** @@
******************************************************************************/

typedef struct AlignSFormat
{
    const char *Name;
    const char *Desc;
    AjBool Alias;
    AjBool Nucleotide;
    AjBool Protein;
    AjBool Showheader;
    AjBool Showseqs;
    ajint  Padding;
    ajint  Minseq;
    ajint  Maxseq;
    void (*Write) (AjPAlign thys);
} AlignOFormat;

#define AlignPFormat AlignOFormat*



static ajint      alignCigar(const AjPSeq qryseq, const AjPSeq refseq,
	                     ajint qstart, ajint qend,
                             ajint rstart,
                             AjPStr *mseq, AjPStr* cigar, AjBool seqExternal);
static void       alignConsStats(AjPAlign thys, ajint iali, AjPStr *cons,
				 ajint* retident, ajint* retsim, ajint* retgap,
				 ajint* retlen);
static AjBool     alignConsSet(const AjPAlign thys, ajint iali, ajint seqnum,
			       AjPStr *cons);
static AlignPData alignData(const AjPAlign thys, ajint iali);
static void       alignDataDel(AlignPData* pthys, AjBool external);
static AlignPData alignDataNew(ajint nseqs, AjBool external);
static void       alignDataSetSequence(AlignPData thys, const AjPSeq seq,
                    ajint i, AjBool external);
static void       alignDiff(AjPStr* pmark, const AjPStr seq, char idchar);
static ajint      alignLen(const AjPAlign thys, ajint iali);
static const AjPSeq alignSeq(const AjPAlign thys, ajint iseq, ajint iali);
static ajint      alignSeqBegin(const AlignPData thys, ajint iseq);
static ajint      alignSeqEnd(const AlignPData thys, ajint iseq);
static ajint      alignSeqGapBegin(const AlignPData data, ajint iseq);
static ajint      alignSeqGapEnd(const AlignPData data, ajint iseq);
static ajint      alignSeqIncrement(const AlignPData thys, ajint iseq);
static const AjPStr alignSeqName(const AjPAlign thys, const AjPSeq seq);
static AjBool     alignSeqRev(const AlignPData thys, ajint iseq);
static const AjPSeq* alignSeqs(const AjPAlign thys, ajint iali);
static void       alignSame(AjPStr* pmark, const AjPStr seq, char idchar);
static void       alignSim(AjPStr* pmark, const char idch, const char simch,
			   const char misch, const char gapch);
static float      alignTotweight(const AjPAlign thys, ajint iali);
static void       alignTraceData(const AjPAlign thys);

static void       alignWriteBam(AjPAlign thys);
static void       alignWriteClustal(AjPAlign thys);
static void       alignWriteFasta(AjPAlign thys);
static void       alignWriteMark(AjPAlign thys, ajint iali, ajint markx);
static void       alignWriteMarkX0(AjPAlign thys);
static void       alignWriteMarkX1(AjPAlign thys);
static void       alignWriteMarkX2(AjPAlign thys);
static void       alignWriteMarkX3(AjPAlign thys);
static void       alignWriteMarkX10(AjPAlign thys);
static void       alignWriteMatch(AjPAlign thys);
static void       alignWriteMega(AjPAlign thys);
static void       alignWriteMeganon(AjPAlign thys);
static void       alignWriteMsf(AjPAlign thys);
static void       alignWriteNexus(AjPAlign thys);
static void       alignWriteNexusnon(AjPAlign thys);
static void       alignWritePhylip(AjPAlign thys);
static void       alignWritePhylipnon(AjPAlign thys);
static void       alignWriteSam(AjPAlign thys);
static void       alignWriteScore(AjPAlign thys);
static void       alignWriteSelex(AjPAlign thys);
static void       alignWriteSeqformat(AjPAlign thys, ajint iali,
				      const char* sqfmt);
static void       alignWriteSimple(AjPAlign thys);
static void       alignWriteSrs(AjPAlign thys);
static void       alignWriteSrsAny(AjPAlign thys,
				   ajint imax, AjBool mark);
static void       alignWriteSrsPair(AjPAlign thys);
static void       alignWriteTCoffee(AjPAlign thys);
static void       alignWriteTrace(AjPAlign thys);
static void       alignWriteTreecon(AjPAlign thys);
static void       alignWriteHeaderNum(AjPAlign thys, ajint iali);




/* @funclist alignFormat ******************************************************
**
** Functions to write alignments
**
******************************************************************************/

static AlignOFormat alignFormat[] =
{
  /* name       description       function */
  /*   Alias    nucleic  protein  header min max zero */
  /* standard sequence formats */
  {"unknown",   "Unknown format",
     AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJTRUE,  0, 0, 0, alignWriteSimple},
  {"fasta",     "Fasta format sequence",
     AJFALSE, AJTRUE,  AJTRUE,  AJFALSE, AJTRUE, 0, 0, 0, alignWriteFasta},
  {"a2m",     "A2M (fasta) format sequence", /* same as fasta */
     AJTRUE,  AJTRUE,  AJTRUE,  AJFALSE, AJTRUE, 0, 0, 0, alignWriteFasta},
  {"msf",       "MSF format sequence",
     AJFALSE, AJTRUE,  AJTRUE,  AJFALSE, AJTRUE, 0, 0, 0, alignWriteMsf},
  {"clustal",   "clustalw format sequence",
     AJFALSE, AJTRUE,  AJTRUE,  AJFALSE, AJTRUE, 0, 0, 0, alignWriteClustal},
  {"mega",       "Mega format sequence",
     AJFALSE, AJTRUE,  AJTRUE,  AJFALSE, AJTRUE, 0, 0, 0, alignWriteMega},
  {"meganon",       "Mega non-interleaved format sequence",
     AJFALSE, AJTRUE,  AJTRUE,  AJFALSE, AJTRUE, 0, 0, 0, alignWriteMeganon},
  {"nexus",   "nexus/paup format sequence",
     AJFALSE, AJTRUE,  AJTRUE,  AJFALSE, AJTRUE, 0, 0, 0, alignWriteNexus},
  {"paup",    "nexus/paup format sequence (alias)",
     AJTRUE,  AJTRUE,  AJTRUE,  AJFALSE, AJTRUE, 0, 0, 0, alignWriteNexus},
  {"nexusnon",   "nexus/paup non-interleaved format sequence",
     AJFALSE, AJTRUE,  AJTRUE,  AJFALSE, AJTRUE, 0, 0, 0, alignWriteNexusnon},
  {"paupnon",    "nexus/paup non-interleaved format sequence (alias)",
     AJTRUE,  AJTRUE,  AJTRUE,  AJFALSE, AJTRUE, 0, 0, 0, alignWriteNexusnon},
  {"phylip",   "phylip format sequence",
     AJFALSE, AJTRUE,  AJTRUE,  AJFALSE, AJTRUE, 0, 0, 0, alignWritePhylip},
  {"phylipnon", "phylip non-interleaved format sequence",
     AJFALSE, AJTRUE,  AJTRUE,  AJFALSE, AJTRUE, 0, 0, 0, alignWritePhylipnon},
  {"selex",       "SELEX format sequence",
     AJFALSE, AJTRUE,  AJTRUE,  AJFALSE, AJTRUE, 0, 0, 0, alignWriteSelex},
  {"treecon",       "Treecon format sequence",
     AJFALSE, AJTRUE,  AJTRUE,  AJFALSE, AJTRUE, 0, 0, 0, alignWriteTreecon},
  /* trace  for debug */
  {"debug", "Debugging trace of full internal data content",
      AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  AJTRUE, 0,  0, 0, alignWriteTrace},
  /* alignment formats */
  {"markx0",    "Pearson MARKX0 format",
     AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  AJTRUE, 0,  2, 2, alignWriteMarkX0},
  {"markx1",    "Pearson MARKX1 format",
     AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  AJTRUE, 0,  2, 2, alignWriteMarkX1},
  {"markx2",    "Pearson MARKX2 format",
     AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  AJTRUE, 0,  2, 2, alignWriteMarkX2},
  {"markx3",    "Pearson MARKX3 format",
     AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  AJTRUE, 0,  2, 2, alignWriteMarkX3},
  {"markx10",   "Pearson MARKX10 format",
     AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  AJTRUE, 0, 2, 2, alignWriteMarkX10},
  {"match","Start and end of matches between sequence pairs",
     AJFALSE, AJTRUE,  AJTRUE,  AJTRUE, AJFALSE, 0,  2, 2, alignWriteMatch},
  {"multiple",  "Simple multiple alignment",
     AJTRUE,  AJTRUE,  AJTRUE,  AJTRUE,  AJTRUE, 0,  0, 0, alignWriteSimple},
  {"pair",      "Simple pairwise alignment",
     AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  AJTRUE, 0,  2, 2, alignWriteSimple},
  {"simple",    "Simple multiple alignment",
     AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  AJTRUE, 0,  0, 0, alignWriteSimple},
  {"sam",       "Sequence alignent/map (SAM) format",
     AJFALSE, AJTRUE,  AJTRUE,  AJFALSE, AJTRUE, 0,  2, 2, alignWriteSam},
  {"bam",       "Binary sequence alignent/map (BAM) format",
     AJFALSE, AJTRUE,  AJTRUE,  AJFALSE, AJTRUE, 0,  2, 2, alignWriteBam},
  {"score",     "Score values for pairs of sequences",
     AJFALSE, AJTRUE,  AJTRUE,  AJTRUE, AJFALSE, 0,  2, 2, alignWriteScore},
  {"srs",       "Simple multiple sequence format for SRS",
     AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  AJTRUE, 0,  0, 0, alignWriteSrs},
  {"srspair",   "Simple pairwise sequence format for SRS",
     AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  AJTRUE, 0,  2, 2, alignWriteSrsPair},
  {"tcoffee",   "TCOFFEE program format",
     AJFALSE, AJTRUE,  AJTRUE,  AJFALSE, AJTRUE, 0, 0, 0, alignWriteTCoffee},
  {NULL, NULL,
     AJFALSE, AJFALSE, AJFALSE, AJFALSE, AJTRUE, 0, 0, 0, NULL}
};

static char alignDefformat[] = "simple";




/* pair only works if the alignment is defined as 2 sequences */

/* other formats to be defined:
**
** markx1 .. markx10 as for FASTA (code in matcher.c)
** blast to be implemented
*/




/* @funcstatic alignWriteTrace ************************************************
**
** Writes an alignment in Trace format
**
** @param [u] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

static void alignWriteTrace(AjPAlign thys)
{
    const AjPSeq seq = NULL;
    ajint nali;
    ajint iali;
    ajint iseq;
    ajint nseq;

    ajint identity   = 0;
    ajint similarity = 0;
    ajint gaps       = 0;
    ajint seqlen     = 0;

    AlignPData* pdata = NULL;
    AlignPData data = NULL;

    AjPStr tmpstr    = NULL;
    AjPFile outf = NULL;

    nali = ajListToarray(thys->Data, (void***) &pdata);
    ajFmtPrintF(thys->File, "Trace output\n");
    ajFmtPrintF(thys->File, "============\n");

    ajFmtPrintF(thys->File,
		"a: Type:'%S' Formatstr:'%S' Format:%d\n",
		thys->Type, thys->Formatstr, thys->Format);
		
    ajFmtPrintF(thys->File,
		"b: File:%F\n",
		thys->File);

    ajFmtPrintF(thys->File,
		"Show:     ShowAcc:%B  ShowDes:%B  ShowUsa:%B\n",
		thys->Showacc, thys->Showdes, thys->Showusa);

   ajFmtPrintF(thys->File,
		"Booleans: Multi:%B  Global:%B  SeqOnly:%B  SeqExternal:%B\n",
		thys->Multi, thys->Global, thys->SeqOnly, thys->SeqExternal);

    ajFmtPrintF(thys->File,
		"Numbers:  NMin:%d  NMax:%d  Nseqs:%d  Count:%d  Width:%d\n",
		thys->Nmin, thys->Nmax, thys->Nseqs,
		thys->Count, thys->Width);

    ajFmtPrintF(thys->File,
		"Matrices: IMatrix:'%S'(%d)  FMatrix:'%S'(%d)\n",
		ajMatrixGetName(thys->IMatrix),
                ajMatrixGetSize(thys->IMatrix),
		ajMatrixfGetName(thys->FMatrix),
                ajMatrixfGetSize(thys->FMatrix));

    ajFmtPrintF(thys->File,
		"Strings:  Matrix:'%S'  GapPen:'%S'  ExtPen:'%S'\n",
		thys->Matrix, thys->GapPen, thys->ExtPen);

    ajFmtPrintF(thys->File,
		"Header: '%S'\n",
		thys->Header);
    ajFmtPrintF(thys->File,
		"SubHeader: '%S'\n",
		thys->SubHeader);
    ajFmtPrintF(thys->File,
		"Tail: '%S'\n",
		thys->Tail);
    ajFmtPrintF(thys->File,
		"SubTail: '%S'\n",
		thys->Tail);

    ajFmtPrintF(thys->File,
		"Key: seqlen/len offset> start..end <offend (suboffset) rev "
		"Begin..End GapBegin..End\n");

    for(iali=0; iali<nali; iali++)
    {
	data = pdata[iali];
	ajFmtPrintF(thys->File,
		    "\nalign%d: Nseqs:%d  LenAli:%d  "
		    "NumId:%d  NumSim:%d  NumGap:%d  Score:'%S'\n",
		    iali, data->Nseqs, data->LenAli,
		    data->NumId, data->NumSim, data->NumGap,
		    data->Score);
	
	alignConsStats(thys, iali, &tmpstr,
		       &identity, &similarity, &gaps, &seqlen);
	
	ajAlignSetStats(thys, iali, seqlen, identity, similarity, gaps, NULL);
	ajFmtPrintF(thys->File,
		    "fixed%d: Nseqs:%d  LenAli:%d  "
		    "NumId:%d  NumSim:%d  NumGap:%d  Score:'%S'\n",
		    iali, data->Nseqs, data->LenAli,
		    data->NumId, data->NumSim, data->NumGap,
		    data->Score);
	ajAlignSetSubStandard(thys, iali);
	outf = thys->File;

	/* turn off printing of the header, keep the calculation */
	thys->File = NULL;
	alignWriteHeaderNum(thys,iali);
	thys->File = outf;

	nseq = thys->Nseqs;

	for(iseq=0; iseq < nseq; iseq++)
	{
	    seq = alignSeq(thys, iseq, iali);
	    ajFmtPrintF(thys->File,
			"Num%d.%d: %d/%d %d> %d..%d <%d (%d) Rev:%B %d..%d "
                        "%d..%d\n",
			iali, iseq,
			(ajSeqGetLen(seq) - ajSeqCountGaps(seq)),
			data->Len[iseq],
			data->Offset[iseq],
			data->Start[iseq], data->End[iseq],
			data->Offend[iseq], data->SubOffset[iseq],
			data->Rev[iseq],
			alignSeqBegin(data, iseq), alignSeqEnd(data, iseq),
			alignSeqGapBegin(data, iseq),
			alignSeqGapEnd(data, iseq));

	    if (ajSeqGetLen(seq) <= 40)
		ajFmtPrintF(thys->File,
			    "Seq%d.%d: %d '%S'\n",
			    iali, iseq, ajSeqGetLen(seq), ajSeqGetSeqS(seq));
	    else
	    {
		ajStrAssignSubS(&tmpstr, ajSeqGetSeqS(seq), -20, -1);
		ajFmtPrintF(thys->File,
			    "Seq%d.%d: %d '%20.20S...%20S'\n",
			    iali, iseq, ajSeqGetLen(seq),
			    ajSeqGetSeqS(seq), tmpstr);
	    }
	}
    }

    ajStrDel(&tmpstr);

    AJFREE(pdata);
    
    return;
}




/* @funcstatic alignWriteClustal ***********************************************
**
** Writes an alignment in ClustalW format
**
** @param [u] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

static void alignWriteClustal(AjPAlign thys)
{
    alignWriteSeqformat(thys, 0, "clustal");

    return;
}




/* @funcstatic alignWriteMega *************************************************
**
** Writes an alignment in Mega format
**
** @param [u] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

static void alignWriteMega(AjPAlign thys)
{
    alignWriteSeqformat(thys, 0, "mega");

    return;
}




/* @funcstatic alignWriteMeganon **********************************************
**
** Writes an alignment in Mega non-interleaved format
**
** @param [u] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

static void alignWriteMeganon(AjPAlign thys)
{
    alignWriteSeqformat(thys, 0, "meganon");

    return;
}




/* @funcstatic alignWriteMsf **************************************************
**
** Writes an alignment in MSF format
**
** @param [u] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

static void alignWriteMsf(AjPAlign thys)
{
    alignWriteSeqformat(thys, 0, "msf");

    return;
}




/* @funcstatic alignWriteNexus ************************************************
**
** Writes an alignment in Nexus/PAUP interleaved format
**
** @param [u] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

static void alignWriteNexus(AjPAlign thys)
{
    alignWriteSeqformat(thys, 0, "nexus");

    return;
}




/* @funcstatic alignWriteNexusnon *********************************************
**
** Writes an alignment in Nexus/PAUP non-interleaved format
**
** @param [u] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

static void alignWriteNexusnon(AjPAlign thys)
{
    alignWriteSeqformat(thys, 0, "nexusnon");

    return;
}




/* @funcstatic alignWritePhylip ************************************************
**
** Writes an alignment in Phylip interleaved format
**
** @param [u] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

static void alignWritePhylip(AjPAlign thys)
{
    alignWriteSeqformat(thys, 0, "phylip");

    return;
}




/* @funcstatic alignWritePhylipnon *********************************************
**
** Writes an alignment in Phylip non-interleaved format
**
** @param [u] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

static void alignWritePhylipnon(AjPAlign thys)
{
    alignWriteSeqformat(thys, 0, "phylipnon");

    return;
}




/* @funcstatic alignWriteSelex ***********************************************
**
** Writes an alignment in SELEX format
**
** @param [u] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

static void alignWriteSelex(AjPAlign thys)
{
    alignWriteSeqformat(thys, 0, "selex");

    return;
}




/* @funcstatic alignWriteTreecon ***********************************************
**
** Writes an alignment in Treecon format
**
** @param [u] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

static void alignWriteTreecon(AjPAlign thys)
{
    alignWriteSeqformat(thys, 0, "treecon");

    return;
}




/* @funcstatic alignWriteFasta ***********************************************
**
** Writes an alignment in MSF format
**
** @param [u] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

static void alignWriteFasta(AjPAlign thys)
{
    alignWriteSeqformat(thys, 0, "fasta");

    return;
}




/* @funcstatic alignWriteSeqformat ********************************************
**
** Writes an alignment in a sequence format.
** Usually called for only one alignment, ignoring any
** sub-alignments as they would overwrite.
**
** @param [u] thys [AjPAlign] Alignment object
** @param [r] iali [ajint] Alignment number
** @param [r] sqfmt [const char*] Sequence output format
** @return [void]
** @@
******************************************************************************/

static void alignWriteSeqformat(AjPAlign thys, ajint iali, const char* sqfmt)
{
    AlignPData data = NULL;

    AjPSeq seq = NULL;
    AjPSeqout seqout;

    ajint iseq;
    ajint istart;
    ajint iend;
    ajint ilen;
    AjPStr tmpstr  = NULL;

    seqout = ajSeqoutNewFile(thys->File);

    thys->SeqOnly = ajTrue;

    ajStrAssignC(&seqout->Formatstr, sqfmt);
    seqout->File = thys->File;

    ajSeqoutOpen(seqout);

    ajListPeekNumber(thys->Data, 0, (void**) &data);
    ilen = data->LenAli;

    for(iseq=0; iseq < thys->Nseqs; iseq++)
    {
	seq = ajSeqNewSeq(alignSeq(thys, iseq, iali));
	istart = data->SubOffset[iseq];
	iend = istart + ilen - 1;
	ajStrAssignSubS(&tmpstr, ajSeqGetSeqS(seq), istart, iend);
	ajSeqAssignSeqS(seq, tmpstr);
	ajSeqoutWriteSeq(seqout, seq);
	ajSeqDel(&seq);
	ajStrDel(&tmpstr);
    }

    ajSeqoutClose(seqout);
    seqout->File = NULL;

    ajSeqoutDel(&seqout);

    return;
}




/* @funcstatic alignWriteMarkX0 ***********************************************
**
** Writes an alignment in Fasta MarkX 0 format.
**
** This is the standard default output format for FASTA programs.
**
** @param [u] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

static void alignWriteMarkX0(AjPAlign thys)
{
    ajint iali;
    ajint nali;

    nali = ajListGetLength(thys->Data);

    for(iali=0; iali < nali; iali++)
	alignWriteMark(thys, iali, 0);

    return;
}




/* @funcstatic alignWriteMarkX1 ***********************************************
**
** Writes an alignment in Fasta MarkX 1 format.
**
** This is an alternative output format for FASTA programs in which
** identities are not marked.
** Instead conservative replacements are denoted by 'x'
** and non-conservative substitutions by 'X'.
**
** @param [u] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

static void alignWriteMarkX1(AjPAlign thys)
{
    ajint iali;
    ajint nali;

    nali = ajListGetLength(thys->Data);

    for(iali=0; iali < nali; iali++)
	alignWriteMark(thys, iali, 1);

    return;
}




/* @funcstatic alignWriteMarkX2 ***********************************************
**
** Writes an alignment in Fasta MarkX 2 format
**
** This is an alternative output format for FASTA programs in which
** the residues in the second sequence are only shown if they are
** different from the first.
**
** @param [u] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

static void alignWriteMarkX2(AjPAlign thys)
{
    ajint iali;
    ajint nali;

    nali = ajListGetLength(thys->Data);

    for(iali=0; iali < nali; iali++)
	alignWriteMark(thys, iali, 2);

    return;
}




/* @funcstatic alignWriteMarkX3 ***********************************************
**
** Writes an alignment in Fasta MarkX 3 format
**
** This is an alternative output format for FASTA programs in which
** the aligned library sequences are displayed in FASTA format
** These can be used to build a primitive multiple alignment.
**
** @param [u] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

static void alignWriteMarkX3(AjPAlign thys)
{
    ajint iali;
    ajint nali;

    nali = ajListGetLength(thys->Data);

    for(iali=0; iali < nali; iali++)
	alignWriteMark(thys, iali, 3);

    return;
}




/* @funcstatic alignWriteMarkX10 **********************************************
**
** Writes an alignment in Fasta MarkX 10 format
**
** @param [u] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

static void alignWriteMarkX10(AjPAlign thys)
{
    ajint iali;
    ajint nali;

    nali = ajListGetLength(thys->Data);

    for(iali=0; iali < nali; iali++)
	alignWriteMark(thys, iali, 10);

    return;
}




/* @funcstatic alignWriteMark *************************************************
**
** Writes a pairwise alignment in a Fasta MarkX format.
**
** For now, seems to work with 0, 1, 2, 3 or 10.
** FASTA 3.4 has 4, 5, 6 and 9 as possible options
** but most seem to make no difference on pairwise comparisons.
**
** 0: identities marked with ':' similarities with '.'
**
** 1: differences marked with 'X'
**
** 2: identities marked with '.', differences with seq2 character
**
** 3: fasta format sequences with no description
**
** 10: aligned regions with associated data
**
** @param [u] thys [AjPAlign] Alignment object
** @param [r] iali [ajint] Alignment number
** @param [r] markx [ajint] Markup type (as defined in Bill Pearson's
**                          FASTA suite
** @return [void]
** @@
** Note: Modified 30-nov-04 (pmr) to replace the original code which had nasty
** dependencies on global variable settings in the original - which we had
** failed to reproduce in all possible circumstances, with code derived from
** the existing and working EMBOSS alignment formats.
******************************************************************************/

static void alignWriteMark(AjPAlign thys, ajint iali, ajint markx)
{    

    AjPFile outf;
    int nseq;

    const AjPStr seq = NULL;
    const AjPSeq seqp = NULL;

    AlignPData data = NULL;

    ajint iseq;
    ajint i;
    ajint j;
    ajint jj;
    ajint kk;
    ajint istart;
    ajint iend;
    ajint ilen;
    ajint jstart;
    ajint jend;
    ajint iwidth = 50;
    const char* cp;

    AjPStr tmpstr  = NULL;
    AjPStr mrkstr  = NULL;
    AjPStr mrkcons = NULL;
    AjPStr cons    = NULL;

    ajint identity   = 0;
    ajint similarity = 0;
    ajint gaps       = 0;
    ajint seqlen     = 0;

    ajint* ipos = NULL;
    ajint* incs = NULL;
    AjBool* num = NULL;
    ajint jpos;
    ajint icnt;
    ajint increment = 1;

    AjPStr tmphdr = NULL;
    AjPStr tmpnum = NULL;
    AjBool lastline = ajFalse;
    
    AjBool isnuc = ajFalse;
    static ajint icount = 0;
    float ident = 0.0;

    ajDebug("alignWriteMark iali:%d markx:%d\n", iali, markx);

    icount++;

    nseq = thys->Nseqs;
    outf = thys->File;
    
    if(thys->Width)
	iwidth = thys->Width;
    
    ajListPeekNumber(thys->Data, iali, (void**) &data);
    
    AJCNEW0(ipos, nseq);
    AJCNEW0(incs, nseq);
    AJCNEW0(num, nseq);
    
    ajStrDel(&cons);
    ajStrDel(&tmphdr);
	
    ilen = data->LenAli;
	
    alignConsStats(thys, iali, &cons,
		   &identity, &similarity, &gaps, &seqlen);
	
    ajAlignSetStats(thys, iali, seqlen, identity, similarity, gaps, NULL);
    ajAlignSetSubStandard(thys, iali);
    alignWriteHeaderNum(thys,iali);
    isnuc = ajSeqIsNuc(data->Seq[0]);
    ident = (float) data->NumId / (float) data->LenAli;
	
    /*ajDebug("# Consens: '%S'\n\n", cons);*/

    if(markx == 1)
	alignSim(&cons, ' ', ' ', 'X', ' ');
    else if(markx == 2)
    {
	seq = ajSeqGetSeqS(data->Seq[0]);
	alignConsSet(thys, iali, 1, &cons);
	ajDebug("alignWriteMark(%d)\nseq0:%S\ncons:%S\nseq1:%S\n",
		markx, seq, cons, ajSeqGetSeqS(data->Seq[1]));
	alignDiff(&cons, seq, '.');
	ajDebug("alignWriteMark(%d)\nseq0:%S\ncons:%S\nseq1:%S\n",
		markx, seq, cons, ajSeqGetSeqS(data->Seq[1]));
    }
    else
	alignSim(&cons, ':', '.', ' ', ' ');

    /*ajDebug("# Modcons: '%S'\n\n", cons);*/
    ajDebug("# nseq:%d\n", nseq);
	
    ajDebug("# AliData [%d] len %d \n", iali, ilen);
    for(iseq=0; iseq < nseq; iseq++)
    {
	incs[iseq] = alignSeqIncrement(data, iseq);
	ipos[iseq] = alignSeqBegin(data, iseq)-incs[iseq];
	/*ajDebug("#   Seq[%d]'%S'\n",
	  iseq, ajSeqGetSeqS(data->Seq[iseq]));*/
    }

    /*
    for(iseq=0; iseq < nseq; iseq++)
    {
	ajDebug("#   Seq[%d]  Len:%d Start:%d End:%d Rev:%B Off:%d\n",
		iseq, data->Len[iseq],
		data->Start[iseq], data->End[iseq],
		data->Rev[iseq],
		data->Offset[iseq]);
    }
    */

    if(markx==10)
    {
	if (isnuc)
	    ajFmtPrintF(outf,">>>%s, %d nt vs %s, %d nt\n",
			ajSeqGetNameC(data->Seq[0]), data->Len[0],
			ajSeqGetNameC(data->Seq[1]), data->Len[1]);
	else
	    ajFmtPrintF(outf,">>>%s, %d aa vs %s, %d aa\n",
			ajSeqGetNameC(data->Seq[0]), data->Len[0],
			ajSeqGetNameC(data->Seq[1]), data->Len[1]);

	ajFmtPrintF(outf,"; mp_name: EMBOSS\n");
	ajFmtPrintF(outf,"; mp_ver: %S\n", ajNamValueVersion());
	ajFmtPrintF(outf,"; pg_name: %S\n", ajUtilGetProgram());
	ajFmtPrintF(outf,"; pg_ver: %S\n", ajNamValueVersion());
	ajFmtPrintF(outf,"; pg_matrix: %S\n", thys->Matrix);
	ajFmtPrintF(outf,"; pg_gap-pen: -%S -%S\n",
		    thys->GapPen, thys->ExtPen);
	ajFmtPrintF(outf,">>#%d\n", icount);
	ajFmtPrintF(outf,"; sw_score: %S\n", data->Score);
	ident = (float) data->NumId / (float) data->LenAli;
	ajFmtPrintF(outf,"; sw_ident: %5.3f\n", ident);
	ajFmtPrintF(outf,"; sw_overlap: %d\n", data->LenAli);
    }

    if(markx <= 2)
    {
	for(i=0; i < ilen; i += iwidth)
	{	    
	    for(iseq=0; iseq < nseq; iseq++)
	    {
	        seqp = data->Seq[iseq];
		seq = ajSeqGetSeqS(seqp);
		istart = i + data->SubOffset[iseq];
		iend = (istart+iwidth-1);

		if (iend >= (data->SubOffset[iseq]+ilen-1))
		{
		    iend = data->SubOffset[iseq]+ilen-1;
		    lastline = ajTrue;
		}

		increment = incs[iseq];
		ajStrAssignSubS(&tmpstr, seq, istart, iend);
		ajStrAssignSubS(&mrkcons, cons,
			    istart - data->SubOffset[iseq],
			    iend - data->SubOffset[iseq]);

		ajStrExchangeCC(&tmpstr, ".", "-");
		icnt = ajStrGetLen(tmpstr)
		    - ajStrCalcCountK(tmpstr, '-')
			- ajStrCalcCountK(tmpstr, ' ');

		/* number the top sequence */
		if (!iseq)
		{
		    jpos = ipos[iseq];
		    ajStrAssignClear(&tmpnum);
		    jj=7;
		    cp = ajStrGetPtr(tmpstr);

		    while (*cp)
                    {
			jj ++;

			if ((*cp != '-') && (*cp != ' '))
			{
			    jpos += increment;

			    if (jpos%10 == 0)
			    {
				ajFmtPrintAppS(&tmpnum,"%*d",jj,jpos);
				jj=0;
				num[iseq] = ajTrue;
			    }
			}
			cp++;
		    }

		    if (lastline && !num[iseq])
		    {
			ajFmtPrintAppS(&tmpnum,"%*d",jj,jpos);
			jj=0;
		    }

		    if(jj)
			ajStrAppendCountK(&tmpnum, ' ', jj);

		    ajFmtPrintF(outf, "%S\n", tmpnum);
		}

		if(!iseq)
		    ajStrAssignS(&mrkstr, tmpstr);
		else
		    alignSame(&mrkstr, tmpstr, ' ');

		if(markx==2 && nseq==2 && iseq==1)
                {
		    ajFmtPrintF(outf,
				"%6.6S %S\n",
				alignSeqName(thys, seqp), mrkcons);
                }
		else
		{
		    if(nseq==2 && iseq==1)
			ajFmtPrintF(outf,
				    "       %S\n",
				    mrkcons);

		    if(ajStrGetLen(tmpstr))
		    {
			ajFmtPrintF(outf,
				    "%6.6S %S\n",
				    alignSeqName(thys, seqp),
				    tmpstr);
		    }
		    else
			ajFmtPrintF(outf,
				    "%6.6S\n",
				    alignSeqName(thys, seqp));

		    /* number the bottom sequence */
		    if (iseq+1 == nseq)
		    {
			jpos = ipos[iseq];
			ajStrAssignClear(&tmpnum);
			jj=7;
			kk=7;
			cp = ajStrGetPtr(tmpstr);

			while(*cp)
                        {
			    jj++;
			    if((*cp != '-') && (*cp != ' '))
			    {
				kk = jj;
				jpos += increment;

				if(jpos%10 == 0)
				{
				    ajFmtPrintAppS(&tmpnum,"%*d",jj,jpos);
				    jj=0;
				    kk=0;
				    num[iseq] = ajTrue;
				}
			    }

			    cp++;
			}

			if(lastline && !num[iseq])
                        {
			    ajFmtPrintAppS(&tmpnum,"%*d",kk,jpos);
			    jj = jj - kk;
			}

			if(jj)
			    ajStrAppendCountK(&tmpnum, ' ', jj);

			ajFmtPrintF(outf, "%S\n", tmpnum);
		    }

		    if(increment > 0)
			ipos[iseq] += icnt;
		    else
			ipos[iseq] -= icnt;
		}
	    }

	    if(nseq > 2) 	    /* 3 or more seqs, markup under */
		ajFmtPrintF(outf,
			    "                     %S\n",
			    mrkcons);

	    ajFmtPrintF(outf, "\n");
	}
    }
    else
    {
	for(iseq=0; iseq < nseq; iseq++)
	{
	    seqp = data->Seq[iseq];
	    seq = ajSeqGetSeqS(seqp);
	    istart = data->SubOffset[iseq];
	    iend = istart + ilen - 1;
	    ajStrAssignSubS(&tmpstr, seq, istart, iend);
	    ajStrAssignSubS(&mrkcons, cons,
			istart - data->SubOffset[iseq],
			iend - data->SubOffset[iseq]);

	    ajStrExchangeCC(&tmpstr, ".", "-");
	    icnt = ajStrGetLen(tmpstr)
		- ajStrCalcCountK(tmpstr, '-')
		    - ajStrCalcCountK(tmpstr, ' ');

	    if(!iseq)
		ajStrAssignS(&mrkstr, tmpstr);
	    else
		alignSame(&mrkstr, tmpstr, ' ');

	    if(markx==3)
	    {
		ajFmtPrintF(outf,
			    ">%S ..\n",
			    alignSeqName(thys, seqp));
	    }
	    else if(markx==10)
	    {
		ajFmtPrintF(outf,">%S ..\n", alignSeqName(thys, seqp));
		ajFmtPrintF(outf,"; sq_len: %d\n",
			    data->Len[iseq]);

		if(ajSeqIsNuc(data->Seq[iseq]))
		    ajFmtPrintF(outf,"; sq_type: D\n");
		else
		    ajFmtPrintF(outf,"; sq_type: p\n");

		ajFmtPrintF(outf,"; al_start: %d\n",
			    data->Offset[iseq] + data->Start[iseq]);
		ajFmtPrintF(outf,"; al_stop: %d\n",
			    data->Offset[iseq] + data->Start[iseq] + icnt - 1);
		ajFmtPrintF(outf,"; al_display_start: %d\n",
			    data->Offset[iseq] + data->Start[iseq]);
	    }
	    else
	    {
	    }

	    for(j=0; j < ilen; j += iwidth)
	    {	    
		jstart = j + data->SubOffset[iseq];
		jend = AJMIN(data->SubOffset[iseq]+ilen-1, jstart+iwidth-1);
		ajStrAssignSubS(&tmpstr, seq, jstart, jend);

		if(ajStrGetLen(tmpstr))
		{
		    ajFmtPrintF(outf,
				"%S\n",
				tmpstr);
		}
	    }

	    ipos[iseq] += icnt;
	}

    }
    
    ajStrDel(&cons);
    ajStrDel(&mrkcons);
    ajStrDel(&mrkstr);
    ajStrDel(&tmpstr);
    ajStrDel(&tmphdr);
    ajStrDel(&tmpnum);
    AJFREE(ipos);
    AJFREE(incs);
    AJFREE(num);
    
    return;
}





/* @funcstatic alignWriteMatch ************************************************
**
** Writes an alignment in Match format
**
** @param [u] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

static void alignWriteMatch(AjPAlign thys)
{
    AjPFile outf;
    const AjPSeq seq1;
    const AjPSeq seq2;
    ajint nali;
    ajint iali;
    ajint len0;
    char fwd0 = '+';
    char fwd1 = '+';
    ajint endgaps = 0;
    ajint begingaps = 0;

    AlignPData* pdata = NULL;
    AlignPData data = NULL;

    outf = thys->File;
    nali = ajListToarray(thys->Data, (void***) &pdata);

    ajAlignWriteHeader(thys);

    for(iali=0; iali<nali; iali++)
    {
	data = pdata[iali];
	len0 = data->End[0] - data->Start[0] + 1;

	if (alignSeqRev(data, 0))
	    fwd0 = '-';

	if (alignSeqRev(data, 1))
	    fwd1 = '-';

	begingaps = alignSeqGapBegin(data,0) + alignSeqGapBegin(data,1);
	endgaps = alignSeqGapEnd(data,0) + alignSeqGapEnd(data,1);
	seq1 = alignSeq(thys, 0, iali);
	seq2 = alignSeq(thys, 1, iali);

	if(thys->Global)
	{
	    len0 = len0 - endgaps - begingaps;
	    ajFmtPrintF(outf,
			"%6d %-15.15S %c %8d..%-8d %-15.15S %c %8d..%d\n",
			len0, alignSeqName(thys, seq1),  fwd0,
			alignSeqBegin(data,0) + alignSeqGapBegin(data,1),
			alignSeqEnd(data,0) - endgaps -
                        alignSeqGapBegin(data,0),
			alignSeqName(thys, seq2), fwd1,
			alignSeqBegin(data,1) + alignSeqGapBegin(data,0),
			alignSeqEnd(data,1) - endgaps -
                        alignSeqGapBegin(data,1));
	}
	else
	{
	    ajFmtPrintF(outf,
			"%6d %-15.15S %c %8d..%-8d %-15.15S %c %8d..%d\n",
			len0, alignSeqName(thys, seq1),  fwd0,
			alignSeqBegin(data,0),
			alignSeqEnd(data,0),
			alignSeqName(thys, seq2), fwd1,
			alignSeqBegin(data,1),
			alignSeqEnd(data,1));
	}
    }

    AJFREE(pdata);

    return;
}




/* @funcstatic alignWriteSimple ***********************************************
**
** Writes an alignment in Simple format
**
** @param [u] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

static void alignWriteSimple(AjPAlign thys)
{
    AjPFile outf;
    int nseq;
    int nali;

    const AjPStr seq = NULL;

    AlignPData* pdata = NULL;
    AlignPData data = NULL;

    ajint iali;
    ajint iseq;
    ajint i;
    ajint istart;
    ajint iend;
    ajint ilen;
    ajint iwidth = 50;

    AjPStr tmpstr  = NULL;
    AjPStr mrkstr  = NULL;
    AjPStr mrkcons = NULL;
    AjPStr cons    = NULL;

    ajint identity   = 0;
    ajint similarity = 0;
    ajint gaps       = 0;
    ajint seqlen     = 0;
    ajint icnt;

    ajint* ipos   = NULL;
    ajint* incs = NULL;

    AjPStr tmphdr = NULL;
    
    ajDebug("alignWriteSimple\n");


    nseq = thys->Nseqs;
    outf = thys->File;
    
    if(thys->Width)
	iwidth = thys->Width;
    
    nali = ajListToarray(thys->Data, (void***) &pdata);
    
    AJCNEW0(ipos, nseq);
    AJCNEW0(incs, nseq);
    
    for(iali=0; iali<nali; iali++)
    {
	ajStrDel(&cons);
	ajStrDel(&tmphdr);
	
	data = pdata[iali];
	ilen = data->LenAli;
	
	alignConsStats(thys, iali, &cons,
		       &identity, &similarity, &gaps, &seqlen);
	
	ajAlignSetStats(thys, iali, seqlen, identity, similarity, gaps, NULL);
	ajAlignSetSubStandard(thys, iali);
	alignWriteHeaderNum(thys, iali);
	
	/*ajDebug("# Consens: '%S'\n\n", cons);*/
	
	alignSim(&cons, '|', ':', '.', ' ');
	
	/*ajDebug("# Modcons: '%S'\n\n", cons);*/
	ajDebug("# Nali:%d nseq:%d\n", nali, nseq);
	
	ajDebug("# AliData [%d] len %d \n", iali, ilen);

	for(iseq=0; iseq < nseq; iseq++)
	{
	    incs[iseq] = alignSeqIncrement(data, iseq);
	    ipos[iseq] = alignSeqBegin(data, iseq)-incs[iseq];
	    /* ajDebug("#   Seq[%d]'%S'\n",
	       iseq, ajSeqGetSeqS(data->Seq[iseq]));*/
	}

	/*
	for(iseq=0; iseq < nseq; iseq++)
	{
	    ajDebug("#   Seq[%d]  Len:%d Start:%d End:%d Rev:%B\n",
		    iseq, data->Len[iseq],
		    data->Start[iseq], data->End[iseq],
		    data->Rev[iseq]);
	}
	*/
	
	for(i=0; i < ilen; i += iwidth)
	{
	    for(iseq=0; iseq < nseq; iseq++)
	    {
		seq = ajSeqGetSeqS(data->Seq[iseq]);
		istart = i + data->SubOffset[iseq];
		iend = AJMIN(data->SubOffset[iseq]+ilen-1, istart+iwidth-1);
		ajStrAssignSubS(&tmpstr, seq, istart, iend);
		ajStrAssignSubS(&mrkcons, cons,
			    istart - data->SubOffset[iseq],
			    iend - data->SubOffset[iseq]);
		
		ajStrExchangeCC(&tmpstr, ".", "-");
		icnt = incs[iseq] * (ajStrGetLen(tmpstr)
				     - ajStrCalcCountK(tmpstr, '-')
				     - ajStrCalcCountK(tmpstr, ' '));
		
		if(!iseq)
		    ajStrAssignS(&mrkstr, tmpstr);
		else
		    alignSame(&mrkstr, tmpstr, ' ');

		
		if(nseq==2 && iseq==1) /* 2 seqs, markup between them */
		    ajFmtPrintF(outf,
				"                     %S\n",
				mrkcons);

		if(ajStrGetLen(tmpstr))
		{
		    ajFmtPrintF(outf,
				"%-13.13S %6d %S %6d\n",
				alignSeqName(thys, data->Seq[iseq]),
				ipos[iseq]+incs[iseq], tmpstr,
				ipos[iseq]+icnt);
		}
		else
		    ajFmtPrintF(outf,
				"%-13.13S\n",
				alignSeqName(thys, data->Seq[iseq]));

		ipos[iseq] += icnt;
	    }

	    if(nseq > 2) 	    /* 3 or more seqs, markup under */
		ajFmtPrintF(outf,
			    "                     %S\n",
			    mrkcons);

	    ajFmtPrintF(outf, "\n");
	}
    }
    
    
    ajStrDel(&cons);
    ajStrDel(&mrkcons);
    ajStrDel(&mrkstr);
    ajStrDel(&tmpstr);
    AJFREE(ipos);
    AJFREE(incs);
    AJFREE(pdata);
    
    return;
}




/* @funcstatic alignWriteScore ************************************************
**
** Writes an alignment in Score-only format
**
** @param [u] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

static void alignWriteScore(AjPAlign thys)
{
    AjPFile outf;
    ajint nali;
    ajint iali;
    AlignPData* pdata = NULL;
    AlignPData data = NULL;
    const AjPSeq seq1;
    const AjPSeq seq2;

    outf = thys->File;
    nali = ajListToarray(thys->Data, (void***) &pdata);

    for(iali=0; iali<nali; iali++)
    {
	data = pdata[iali];
	seq1 = data->Seq[0];
	seq2 = data->Seq[1];

	if(ajStrGetLen(data->Score))
	    ajFmtPrintF(outf, "%S %S %d (%S)\n",
			alignSeqName(thys, seq1),
                        alignSeqName(thys, seq2),
			data->LenAli, data->Score);
	else
	    ajFmtPrintF(outf, "%S %S %d\n",
			alignSeqName(thys, seq1),
                        alignSeqName(thys, seq2),
			data->LenAli);
    }

    AJFREE(pdata);

    return;
}




/* @funcstatic alignWriteSam ***************************************************
**
** Writes an alignment in sequence alignment/map (SAM) format
**
** @param [u] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

static void alignWriteSam(AjPAlign thys)
{
    AjPFile outf;
    ajint nali;
    ajint iali;
    ajint iseq;
    ajint j;
    ajint qrystart;
    ajint qryend;
    ajint refstart;

    ajint nmismatches;
    AlignPData* pdata = NULL;
    AlignPData data = NULL;

    ajint refpos = 0; /* 1-based leftmost position on the reference sequence */

    const AjPSeq refseq;
    const AjPSeq qryseq;
    AjPStr seqacc = NULL;

    ajuint flag = 0;     /* bitwise flag, section 2.2.2 in SAM specification */
    ajuint mapq = 255;   /* mapping quality, 255 means not available */
    AjPStr cigar = NULL; /* extended CIGAR string */
    AjPStr mrnm = NULL;  /* mate reference sequence name */
    ajuint mpos = 0;     /* leftmost mate position of the clipped sequence */
    ajuint isize = 0;    /* inferred insert size */

    AjPStr argstr;
    AjPStr mseq = NULL;  /* match region of the query sequence */

    mseq = ajStrNew();
    mrnm = ajStrNewC("*");/* TODO: mate sequences not yet supported */

    outf = thys->File;
    nali = ajListToarray(thys->Data, (void***) &pdata);

    if(!thys->Count++)
    {
        ajFmtPrintF(outf, "@HD\tVN:1.0\n");

        /* Program record */
        argstr = ajStrNewS(ajUtilGetCmdline());
        ajStrExchangeKK(&argstr, '\n', ' ');
        ajFmtPrintF(outf, "@PG\tID:%S\tVN:%S\tCL:%S\n",
                    ajUtilGetProgram(), ajNamValueVersion(), argstr);
        ajStrDel(&argstr);
    }

    for(iali=0; iali<nali; iali++)
    {
	data = pdata[iali];
	refseq = data->Seq[thys->RefSeq];

	refpos = alignSeqBegin(data, thys->RefSeq) + 1
			- alignSeqIncrement(data, thys->RefSeq);

	for(iseq=0; iseq < data->Nseqs; iseq++)
	{
	    if (iseq==thys->RefSeq)
		continue;

	    qryseq = data->Seq[iseq];

	    if(ajSeqIsReversed(qryseq))
		flag |= 0x0010;

	    if(thys->Global ||thys->SeqExternal)
	    {
		qrystart = alignSeqBegin(data,iseq) -
			alignSeqIncrement(data,iseq);
		qryend = alignSeqEnd(data,iseq);
		refstart = refpos-1;
	    }
	    else
	    {
		qrystart = 0;
		qryend = data->LenAli;
		refstart = 0;
	    }

	    if(qryseq->Accuracy)
	    {
		ajStrAssignClear(&seqacc);

		/* ASCII-33 gives the Phred base quality */
		for(j=qrystart;j< qryend;j++)
		    ajStrAppendK(&seqacc, (char) (33 + qryseq->Accuracy[j]));

	    }
	    else
		ajStrAssignC(&seqacc,"*");

	    ajStrAssignClear(&mseq);
	    ajStrAssignClear(&cigar);

	    /* get CIGAR, match region of the query sequence and #mismatches */
	    nmismatches = alignCigar(qryseq, refseq,
	                             qrystart,
	                             qryend,
	                             refstart,
	                             &mseq, &cigar, thys->SeqExternal);

	    ajFmtPrintF(outf, "%S\t%u\t%S\t%d\t%d\t%S\t%S\t%u\t%u\t%S\t%S\t"
		    "AS:i:%S\tNM:i:%d\n",
		    alignSeqName(thys, qryseq),
		    flag,
		    alignSeqName(thys, refseq),
		    refpos,
		    mapq,
		    cigar,
		    mrnm,
		    mpos,
		    isize,
		    mseq,
		    seqacc,
		    data->Score,
		    nmismatches);
	}
    }

    AJFREE(pdata);
    ajStrDel(&cigar);
    ajStrDel(&mrnm);
    ajStrDel(&seqacc);
    ajStrDel(&mseq);

    return;
}




/* @funcstatic alignWriteBam ***************************************************
**
** Writes an alignment in binary sequence alignment/map (BAM) format
**
** @param [u] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

static void alignWriteBam(AjPAlign thys)
{
    ajint nali;
    ajint iali;
    AlignPData* pdata = NULL;


    nali = ajListToarray(thys->Data, (void***) &pdata);

    for(iali=0; iali<nali; iali++)
    {
    }

    AJFREE(pdata);

    return;
}




/* @funcstatic alignWriteSrs **************************************************
**
** Writes an alignment in SRS format
**
** @param [u] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

static void alignWriteSrs(AjPAlign thys)
{
    alignWriteSrsAny(thys, 0, ajFalse);

    return;
}




/* @funcstatic alignWriteSrsPair **********************************************
**
** Writes an alignment in SrsPair format
**
** @param [u] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

static void alignWriteSrsPair(AjPAlign thys)
{
    alignWriteSrsAny(thys, 2, ajTrue);

    return;
}




/* @funcstatic alignWriteSrsAny ***********************************************
**
** Writes an alignment in SRS format (with switches for pairwise and general
** formatting)
**
** @param [u] thys [AjPAlign] Alignment object
** @param [r] imax [ajint] Maximum number of sequences (0 for unknown)
** @param [r] mark [AjBool] Markup the alignment
** @return [void]
** @@
******************************************************************************/

static void alignWriteSrsAny(AjPAlign thys, ajint imax, AjBool mark)
{    
    AjPFile outf;
    int nseq;
    int nali;

    const AjPStr seq = NULL;

    AlignPData* pdata = NULL;
    AlignPData data = NULL;

    ajint iali;
    ajint iseq;
    ajint i;
    ajint istart;
    ajint iend;
    ajint ilen;

    AjPStr tmpstr  = NULL;
    AjPStr mrkstr  = NULL;
    AjPStr mrkcons = NULL;
    AjPStr cons    = NULL;

    ajint iwidth     = 50;
    ajint identity   = 0;
    ajint similarity = 0;
    ajint gaps=0;
    ajint seqlen=0;
    ajint icnt;

    ajint* ipos = NULL;
    ajint* incs = NULL;
    AjBool pair = ajFalse;
    
    AjPStr tmphdr = NULL;


    outf = thys->File;
    nseq = thys->Nseqs;
    
    if(nseq < 1)
    {
	ajAlignWriteHeader(thys);

	return;
    }
    
    if(thys->Width)
	iwidth = thys->Width;
    
    /*
     ** pair: if true, change consensus into |:. markup
     ** and put markup line between sequences
     */
    
    if(imax == 2 && nseq == 2 && mark)
	pair = ajTrue;
    
    nali = ajListToarray(thys->Data, (void***) &pdata);
    
    AJCNEW0(ipos, nseq);
    AJCNEW0(incs, nseq);
    
    for(iali=0; iali<nali; iali++)
    {
	data = pdata[iali];
	ilen = data->LenAli;
	
	ajStrDel(&cons);
	ajStrDel(&tmphdr);
	alignConsStats(thys, iali, &cons,
		       &identity, &similarity, &gaps, &seqlen);
	
	ajAlignSetStats(thys, iali, seqlen, identity, similarity, gaps,
			NULL);
	ajAlignSetSubStandard(thys, iali);
	alignWriteHeaderNum(thys, iali);
	
	/*ajDebug("# Consens: '%S'\n\n", cons);*/
	
	if(pair)
	    alignSim(&cons, '|', ':', '.', ' ');
	
	/*ajDebug("# Modcons: '%S'\n\n", cons);*/
	ajDebug("# Nali:%d nseq:%d\n", nali, nseq);
	
	ajDebug("# AliData [%d] len %d \n", iali, ilen);
	
	for(iseq=0; iseq < nseq; iseq++)
	{
	    incs[iseq] = alignSeqIncrement(data, iseq);
	    ipos[iseq] = alignSeqBegin(data, iseq)-incs[iseq];
	    /*ajDebug("#   Seq[%d] Off:%d Sta:%d End:%d '%S'\n",
		    iseq, data->Offset[iseq], data->Start[iseq],
		    data->End[iseq], ajSeqGetSeqS(data->Seq[iseq]));*/
	}

	/*
	for(iseq=0; iseq < nseq; iseq++)
	    ajDebug("#   Seq[%d] Start:%d End:%d Rev:%B\n",
		    iseq, data->Start[iseq], data->End[iseq],
		    data->Rev[iseq]);
	*/

	for(i=0; i < ilen; i += iwidth)
	{	    
	    for(iseq=0; iseq < nseq; iseq++)
	    {
		seq = ajSeqGetSeqS(data->Seq[iseq]);
		istart = i + data->SubOffset[iseq];
		iend = AJMIN(data->SubOffset[iseq]+ilen-1, istart+iwidth-1);
		ajStrAssignSubS(&tmpstr, seq, istart, iend);
		ajStrAssignSubS(&mrkcons, cons,
			    istart - data->SubOffset[iseq],
			    iend - data->SubOffset[iseq]);

		ajStrExchangeCC(&tmpstr, ".", "-");
		icnt = incs[iseq] * (ajStrGetLen(tmpstr)
				     - ajStrCalcCountK(tmpstr, '-')
				     - ajStrCalcCountK(tmpstr, ' '));

		if(!iseq)
		    ajStrAssignS(&mrkstr, tmpstr);
		else
		    alignSame(&mrkstr, tmpstr, ' ');

		if(pair && iseq==1)  /* 2 seqs, markup between them */
		    ajFmtPrintF(outf,
				"                     %S\n",
				mrkcons);

		if(ajStrGetLen(tmpstr))
		{
		    if(icnt)
			ajFmtPrintF(outf,
				    "%-13.13S %6d %S %6d\n",
				    alignSeqName(thys, data->Seq[iseq]),
				    ipos[iseq]+incs[iseq], tmpstr,
				    ipos[iseq]+icnt);
		    else
			ajFmtPrintF(outf,
				    "%-13.13S %6d %S %6d\n",
				    alignSeqName(thys, data->Seq[iseq]),
				    ipos[iseq], tmpstr,
				    ipos[iseq]);
		}
		else
		    ajFmtPrintF(outf,
				"%-13.13S\n",
				alignSeqName(thys, data->Seq[iseq]));

		ipos[iseq] += icnt;
	    }

	    if(mark && !pair)	    /* 3 or more seqs, markup under */
		ajFmtPrintF(outf,
			    "                     %S\n",
			    mrkcons);

	    ajFmtPrintF(outf, "\n");
	}
    }
    
    ajStrDel(&mrkcons);
    ajStrDel(&tmpstr);
    ajStrDel(&mrkstr);
    ajStrDel(&cons);
    AJFREE(ipos);
    AJFREE(incs);
    AJFREE(pdata);
    
    return;
}




/* @funcstatic alignWriteTCoffee **********************************************
**
** Writes an alignment as a T-COFFEE library
**
** @param [u] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

static void alignWriteTCoffee (AjPAlign thys)
{
    AjPFile outf = thys->File;
    int nseq;
    int nali;
    AlignPData* pdata = NULL;
    AlignPData data = NULL;
    ajint iali;
    ajint iseq;
    ajint i;
    ajint s1,s2;
    ajint n1,n2;
    ajint ilen;
    AjPStr sseq= NULL;
    const AjPStr sseq1= NULL;
    const AjPStr sseq2= NULL;
    ajint pidentity=0;

    ajDebug("alignWriteTCoffee\n");

    nseq = thys->Nseqs;
    nali = ajListToarray(thys->Data, (void***) &pdata);
    thys->SeqOnly = ajTrue; /* suppress output of tail */

    /* print header */
    ajFmtPrintF (outf, "%d\n", nseq); /* number of sequences */

    for (iseq=0; iseq < nseq; iseq++) {
	ajStrAssignS(&sseq,ajSeqGetSeqS(pdata[0]->Seq[iseq]));
	ajStrRemoveWhite(&sseq);
	ajStrExchangeCC(&sseq, "-","");
	/* <seqname> <seqlen> <sequence> */
	ajFmtPrintF(outf, "%S %d %S\n",
		    alignSeqName(thys, pdata[0]->Seq[iseq]),
		    ajStrGetLen(sseq),
		    sseq);
	ajStrDel(&sseq);
    }


    for (iali = 0; iali<nali; iali++)
    {
	data = pdata[iali];
	ilen = data->LenAli;
	pidentity = (int)(0.5 + 100.0 *
			  (float) data->NumId / (float) data->LenAli);
	ajFmtPrintF(outf, "! score=%S\n",data->Score);
	ajFmtPrintF(outf, "! matrix=%S\n",thys->Matrix);
	ajFmtPrintF(outf, "! gapopen=%S gapext=%S\n",
		    thys->GapPen,thys->ExtPen);

	/* go through all pairwise alignments */
	for (s1=0; s1<nseq-1; s1++)
	{
	    sseq1 = ajSeqGetSeqS(pdata[0]->Seq[s1]);

	    for (s2=s1+1; s2<nseq; s2++) 
	    {
		ajFmtPrintF (outf, "#%d %d\n", s1+1, s2+1);
		n1 = 1; n2 = 1;
		sseq2 = ajSeqGetSeqS(pdata[0]->Seq[s2]);

		for (i=0; i<ilen; i++)
		{
		    /* check if position is ungapped */
		    if (ajStrGetCharPos(sseq1,i)>='A' && ajStrGetCharPos(sseq2,i)>='A')
		    {
			/* output aligned pair,
			   sequence weight,
			   residue weight,
			   match weight
			   (format as guessed from the T-coffee docs) */
			ajFmtPrintF (outf, "%d %d %d %d %d\n", 
				     n1, n2, pidentity, 1, 0);
		    }

		    if (ajStrGetCharPos(sseq1,i)>='A')
			n1++;

		    if (ajStrGetCharPos(sseq2,i)>='A')
			n2++;
		}
	    }
	}
    }

    /* write footer */
    ajFmtPrintF (outf, "! SEQ_1_TO_N\n");

    AJFREE (pdata);

    return;
}




/* @func ajAlignDefine ********************************************************
**
** Defines a sequence set as an alignment. The sequences are stored internally
** and may be edited by alignment processing.
**
** @param [u] thys [AjPAlign] Alignment object
** @param [u] seqset [AjPSeqset] Sequence set object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajAlignDefine(AjPAlign thys, AjPSeqset seqset)
{    
    AlignPData data = NULL;
    ajint i;
    const AjPSeq seq = NULL;
    
    if(!thys->Nseqs)
	thys->Nseqs = ajSeqsetGetSize(seqset);

       
    data = alignDataNew(thys->Nseqs, thys->SeqExternal);
    
    
    for(i=0; i < thys->Nseqs; i++)
    {
	seq = ajSeqsetGetseqSeq(seqset, i);
	alignDataSetSequence(data, seq, i, thys->SeqExternal);
    }
    
    data->LenAli = ajSeqsetGetLen(seqset);
    
    ajListPushAppend(thys->Data, data);
    
    return ajTrue;
}




/* @func ajAlignDefineSS ******************************************************
**
** Defines a sequence pair as an alignment. Either new copies of the
** sequences or direct references to them are made depending on the value
** of the SeqExternal attribute of the alignment object. 
**
** @param [u] thys [AjPAlign] Alignment object
** @param [r] seqa [const AjPSeq] Sequence object
** @param [r] seqb [const AjPSeq] Second sequence object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajAlignDefineSS(AjPAlign thys, const AjPSeq seqa, const AjPSeq seqb)
{
    AlignPData data = NULL;

    if (!thys->Nseqs)
	thys->Nseqs = 2;

    if(thys->Nseqs != 2)
    {
	ajErr("ajAlignDefineSS called with %d sequences in alignment",
	      thys->Nseqs);
    }

    data = alignDataNew(2, thys->SeqExternal);

    ajDebug("ajAlignDefineSS '%S' '%S'\n",
	    ajSeqGetNameS(seqa), ajSeqGetNameS(seqb));

    alignDataSetSequence(data, seqa, 0, thys->SeqExternal);
    
    if(!thys->SeqExternal && !ajSeqIsTrimmed(data->RealSeq[0]))
    {
	    ajSeqTrim(data->RealSeq[0]);
    }

    alignDataSetSequence(data, seqb, 1, thys->SeqExternal);

    if(!thys->SeqExternal && !ajSeqIsTrimmed(data->RealSeq[1]))
    {
	    ajSeqTrim(data->RealSeq[1]);
    }

    data->LenAli = AJMIN(ajSeqGetLen(seqa), ajSeqGetLen(seqb));

    ajListPushAppend(thys->Data, data);

    return ajTrue;
}




/* @func ajAlignDefineCC ******************************************************
**
** Defines a pair of char* strings as an alignment
**
** @param [u] thys [AjPAlign] Alignment object
** @param [r] seqa [const char*] First sequence
** @param [r] seqb [const char*] Second sequence
** @param [r] namea [const char*] Name of first sequence
** @param [r] nameb [const char*] Name of second sequence
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajAlignDefineCC(AjPAlign thys, const char* seqa, const char* seqb,
		       const char* namea, const char* nameb)
{
    AlignPData data = NULL;

    AjPStr tmpstr = NULL;

    if(!thys->Nseqs)
	thys->Nseqs = 2;

    data = alignDataNew(2, AJFALSE);

    ajStrAssignC(&tmpstr, seqa);
    data->Start[0] = 1;
    data->End[0] = ajStrGetLen(tmpstr);
    data->Len[0] = ajStrGetLen(tmpstr) - ajSeqstrCountGaps(tmpstr);
    data->Offset[0] = 0;
    data->Offend[0] = 0;
    data->SubOffset[0] = 0;
    data->Rev[0] = ajFalse;

    /* no external option - we do need to create the AjPSeqs */

    data->RealSeq[0] = ajSeqNewNameC(seqa, namea);
    ajSeqGapStandard(data->RealSeq[0], '-');
    data->Seq[0] = data->RealSeq[0];

    ajStrAssignC(&tmpstr, seqb);
    data->Start[1] = 1;
    data->End[1] = ajStrGetLen(tmpstr);
    data->Len[1] = ajStrGetLen(tmpstr) - ajSeqstrCountGaps(tmpstr);
    data->Offset[1] = 0;
    data->Offend[1] = 0;
    data->SubOffset[1] = 0;
    data->Rev[1] = ajFalse;

    data->RealSeq[1] = ajSeqNewNameC(seqb, nameb);
    ajSeqGapStandard(data->RealSeq[1], '-');
    data->Seq[1] = data->RealSeq[1];

    data->LenAli = AJMIN(strlen(seqa), strlen(seqb));

    ajDebug("ajAlignDefineCC %s %d %s %d\n",
	    namea, ajSeqGetLen(data->Seq[0]),
	    nameb, ajSeqGetLen(data->Seq[1]));
    ajListPushAppend(thys->Data, data);

    ajStrDel(&tmpstr);

    return ajTrue;
}




/* @func ajAlignDel ***********************************************************
**
** Destructor for Alignment objects
**
** @param [d] pthys [AjPAlign*] Alignment object reference
** @return [void]
** @category delete [AjPAlign] Default destructor
** @@
******************************************************************************/

void ajAlignDel(AjPAlign* pthys)
{
    AlignPData data = NULL;
    AjPAlign thys;

    thys = *pthys;

    if(!thys)
        return;

    ajDebug("ajAlignDel %d seqs\n", thys->Nseqs);
    ajStrDel(&thys->Formatstr);
    ajStrDel(&thys->Type);
    ajStrDel(&thys->Header);
    ajStrDel(&thys->SubHeader);
    ajStrDel(&thys->Tail);
    ajStrDel(&thys->SubTail);
    ajStrDel(&thys->Matrix);
    ajStrDel(&thys->GapPen);
    ajStrDel(&thys->ExtPen);

    ajMatrixDel(&thys->IMatrix);
    ajMatrixfDel(&thys->FMatrix);

    while(ajListPop(thys->Data, (void**) &data))
	alignDataDel(&data, thys->SeqExternal);

    ajListFree(&thys->Data);

    AJFREE(*pthys);

    return;
}




/* @func ajAlignReset *********************************************************
**
** Reset for Alignment objects
**
** @param [w] thys [AjPAlign] Alignment object reference
** @return [void]
** @category modify [AjPAlign] Resets ready for reuse.
** @@
******************************************************************************/

void ajAlignReset(AjPAlign thys)
{
    AlignPData data = NULL;

    while(ajListPop(thys->Data, (void**) &data))
	alignDataDel(&data, thys->SeqExternal);

    ajListFree(&thys->Data);
    thys->Data = ajListNew();

    thys->Nseqs = 0;

    return;
}




/* @func ajAlignOpen **********************************************************
**
** Opens a new align file
**
** @param [u] thys [AjPAlign] Alignment object
** @param [r] name [const AjPStr] File name
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajAlignOpen(AjPAlign thys, const AjPStr name)
{
    if(!ajAlignValid(thys))
	return ajFalse;

    thys->File = ajFileNewOutNameS(name);

    if(thys->File)
	return ajTrue;

    return ajFalse;
}




/* @func ajAlignFormatDefault *************************************************
**
** Sets the default format for an alignment
**
** @param [w] pformat [AjPStr*] Default format returned
** @return [AjBool] ajTrue if format was returned
** @@
******************************************************************************/

AjBool ajAlignFormatDefault(AjPStr* pformat)
{
    if(ajStrGetLen(*pformat))
	ajDebug("... output format '%S'\n", *pformat);
    else
    {
	/* ajStrAssignEmptyC(pformat, alignFormat[0].Name);*/
	ajStrAssignEmptyC(pformat, "gff");	/* use the real name */
	ajDebug("... output format not set, default to '%S'\n", *pformat);
    }

    return ajTrue;
}




/* @func ajAlignGetLen ********************************************************
**
** Returns the filename for an alignment. If the alignment has more than one
** sub-alignment, returns the total.
**
** @param [r] thys [const AjPAlign] Alignment object.
** @return [ajint] Alignment length.
** @@
******************************************************************************/

ajint ajAlignGetLen(const AjPAlign thys)
{
    ajint ret = 0;
    ajint i;
    ajint nali;

    AlignPData* pdata = NULL;
    AlignPData data = NULL;

    if(!thys)
	return 0;

    if(!thys->Data)
	return 0;

    nali = ajListToarray(thys->Data, (void***) &pdata);

    for(i=0; i<nali; i++)
    {
	data = pdata[i];
	ret += data->LenAli;
    }

    AJFREE(pdata);
     
    return ret;
}




/* @func ajAlignGetFilename ***************************************************
**
** Returns the filename for an alignment.
**
** @param [r] thys [const AjPAlign] Alignment object.
** @return [const char*] Filename.
** @@
******************************************************************************/

const char * ajAlignGetFilename(const AjPAlign thys)
{
    if(!thys)
	return NULL;

    if(!thys->File)
	return NULL;
    
    return ajFileGetNameC(thys->File);
}




/* @func ajAlignGetFormat ****************************************************
**
** Returns the sequence format for an alignment.
**
** @param [r] thys [const AjPAlign] Alignment object.
** @return [const AjPStr] Alignment format
** @@
******************************************************************************/

const AjPStr ajAlignGetFormat(const AjPAlign thys)
{
    if(!thys)
	return NULL;
    
    return thys->Formatstr;
}




/* @func ajAlignFormatShowsSequences ****************************************
**
** Returns whether current alignment format includes sequences in output
**
** @param [r] thys [const AjPAlign] Alignment object
** @return [AjBool] true if sequences appear in output
** @@
******************************************************************************/

AjBool ajAlignFormatShowsSequences(const AjPAlign thys)
{
    if(!thys)
    return AJTRUE;
    
    return alignFormat[thys->Format].Showseqs;
}




/* @func ajAlignFindFormat ****************************************************
**
** Looks for the specified align format in the internal definitions and
** returns the index.
**
** @param [r] format [const AjPStr] Format required.
** @param [w] iformat [ajint*] Index
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajAlignFindFormat(const AjPStr format, ajint* iformat)
{
    AjPStr tmpformat = NULL;
    ajint i = 0;

    ajDebug("ajAlignFindFormat '%S'\n", format);

    if(!ajStrGetLen(format))
	return ajFalse;

    ajStrAssignS(&tmpformat, format);
    ajStrFmtLower(&tmpformat);

    while(alignFormat[i].Name)
    {
	if(ajStrMatchCaseC(tmpformat, alignFormat[i].Name))
	{
	    *iformat = i;
	    ajStrDel(&tmpformat);
	    ajDebug("... found at %d\n", i);

	    return ajTrue;
	}
	i++;
    }

    ajDebug("... not found\n");
    ajStrDel(&tmpformat);
    return ajFalse;
}




/* @func ajAlignValid *********************************************************
**
** Test for an alignment object.
**
** Checks the format works with the number of sequences.
** Checks the format works with the type (protein or nucleotide).
**
** @param [u] thys [AjPAlign] Alignment object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajAlignValid(AjPAlign thys)
{
    ajDebug("ajAlignValid format '%S' %d Nmin %d Nmax %d\n",
	    thys->Formatstr, thys->Format, thys->Nmin, thys->Nmax);

    if(!thys->Format)			/* test acdc-alignbadformat */
    {
        if(ajStrGetLen(thys->Formatstr))
        {
            if(!ajAlignFindFormat(thys->Formatstr, &thys->Format))
            {
                ajErr("Unknown alignment format '%S'", thys->Formatstr);
                return ajFalse;
            }
        }
        else 
        {
            ajStrAssignC(&thys->Formatstr,alignDefformat);
            ajAlignFindFormat(thys->Formatstr, &thys->Format);
        }
    }

    if( alignFormat[thys->Format].Minseq  &&
       thys->Nmin < alignFormat[thys->Format].Minseq)
    {
	ajErr("Alignment format %s specifies at least %d sequences, "
	      "alignment has only %d",
	      alignFormat[thys->Format].Name,
              alignFormat[thys->Format].Minseq, thys->Nmin);
	return ajFalse;
    }

    if( alignFormat[thys->Format].Maxseq  &&
       thys->Nmax > alignFormat[thys->Format].Maxseq)
    {
	ajErr("Alignment format %s specifies at most %d sequences, "
	      "alignment has %d",
	      alignFormat[thys->Format].Name,
              alignFormat[thys->Format].Maxseq, thys->Nmax);
	return ajFalse;
    }

    if(thys->Width < 10)
    {
	ajWarn("Alignment width (-awidth=%d) too narrow, reset to 10",
	       thys->Width);
	thys->Width=10;
    }

    return ajTrue;
}




/* @func ajAlignNew ***********************************************************
**
** Constructor for an alignment object
**
** @return [AjPAlign] New Alignment object
** @category new [AjPAlign] Default constructor
** @@
******************************************************************************/

AjPAlign ajAlignNew(void)
{
    AjPAlign pthis;

    AJNEW0(pthis);

    pthis->Count     = 0;
    pthis->Formatstr = ajStrNew();
    pthis->Format    = 0;
    pthis->File      = NULL;
    pthis->Data      = ajListNew();
    pthis->RefSeq    = 0;

    return pthis;
}




/* @func ajAlignWrite *********************************************************
**
** Writes an alignment file
**
** @param [u] thys [AjPAlign] Alignment object
** @return [void]
** @category output [AjPAlign] Master alignment output routine
** @@
******************************************************************************/

void ajAlignWrite(AjPAlign thys)
{
    ajDebug("ajAlignWrite\n");

    ajAlignTraceT(thys, "ajAlignWrite start");

    if(!thys->Format)
	if(!ajAlignFindFormat(thys->Formatstr, &thys->Format))
	    ajErr("unknown align format '%S'", thys->Formatstr);

    ajDebug("ajAlignWrite %d '%s'\n",
	    thys->Format, alignFormat[thys->Format].Name);

    ajAlignSetType(thys);

    /* Calling funclist alignFormat() */

    alignFormat[thys->Format].Write(thys);

    return;
}




/* @func ajAlignClose *********************************************************
**
** Closes an alignment
**
** @param [u] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

void ajAlignClose(AjPAlign thys)
{
    ajDebug("ajAlignClose '%F'\n", thys->File);

    if(!thys->SeqOnly)
	ajAlignWriteTail(thys);

    ajFileClose(&thys->File);

    return;
}




/* @func ajAlignWriteHeader ***************************************************
**
** Writes an alignment header.
**
** @param [u] thys [AjPAlign] Alignment object. Internal count is updated
**                                  to avoid duplicate headers
** @return [void]
** @category output [AjPAlign] Master header output routine
** @@
******************************************************************************/

void ajAlignWriteHeader(AjPAlign thys)
{
    alignWriteHeaderNum(thys, 0);

    return;
}




/* @funcstatic alignWriteHeaderNum *********************************************
**
** Writes an alignment header.
**
** @param [u] thys [AjPAlign] Alignment object. Internal count is updated
**                                  to avoid duplicate headers
** @param [r] iali [ajint] Alignment number
** @return [void]
** @category output [AjPAlign] Master header output routine
** @@
******************************************************************************/

static void alignWriteHeaderNum(AjPAlign thys, ajint iali)
{
    AjPFile outf;
    AjPStr tmpstr = NULL;
    AjBool doSingle;
    ajint i;
    const AjPSeq seq;

    if(!alignFormat[thys->Format].Showheader)
        return;

    outf     = thys->File;
    doSingle = ajFalse;	/* turned off for now - always multi format */    

    if(!thys->Count)
    {
	ajFmtPrintF(outf, "########################################\n");
	ajFmtPrintF(outf, "# Program: %S\n", ajUtilGetProgram());
	ajFmtPrintF(outf, "# Rundate: %D\n", ajTimeRefTodayFmt("report"));
	ajFmtPrintF(outf, "# Commandline: %S\n", ajUtilGetProgram());
	ajStrAssignS(&tmpstr, ajUtilGetCmdline());

	if(ajStrGetLen(tmpstr))
	{
	    ajStrExchangeCC(&tmpstr, "\n", "\1#    ");
	    ajStrExchangeCC(&tmpstr, "\1", "\n");
	    ajFmtPrintF(outf, "#    %S\n", tmpstr);
	}

	ajStrAssignS(&tmpstr, ajUtilGetInputs());

	if(ajStrGetLen(tmpstr))
	{
	    ajStrExchangeCC(&tmpstr, "\n", "\1#    ");
	    ajStrExchangeCC(&tmpstr, "\1", "\n");
	    ajFmtPrintF(outf, "#    %S\n", tmpstr);
	}

	ajFmtPrintF(outf, "# Align_format: %S\n", thys->Formatstr);
	ajFmtPrintF(outf, "# Report_file: %F\n", outf);

	if(!doSingle || thys->Multi)
	    ajFmtPrintF(outf, "########################################\n");
	else
	    ajFmtPrintF(outf, "#\n");
    }
    
    if(!doSingle || thys->Multi)
	ajFmtPrintF(outf, "\n#=======================================\n#\n");
    
    ajFmtPrintF(outf, "# Aligned_sequences: %d\n", thys->Nseqs);

    for(i=0; i < thys->Nseqs; i++)
    {
	ajStrAssignClear(&tmpstr);
	seq = alignSeq(thys,i, iali);

	if(thys->Showacc)
	    ajFmtPrintAppS(&tmpstr, " (%S)",
			   ajSeqGetAccS(seq));
	if(thys->Showdes)
	    ajFmtPrintAppS(&tmpstr, " %S",
			   ajSeqGetDescS(seq));
	ajFmtPrintF(outf, "# %d: %S%S\n",
		    i+1, alignSeqName(thys, seq), tmpstr);
    }

    if(ajStrGetLen(thys->Matrix))
	ajFmtPrintF(outf, "# Matrix: %S\n", thys->Matrix);
    
    if(ajStrGetLen(thys->GapPen))
	ajFmtPrintF(outf, "# Gap_penalty: %S\n", thys->GapPen);
    
    if(ajStrGetLen(thys->ExtPen))
	ajFmtPrintF(outf, "# Extend_penalty: %S\n", thys->ExtPen);
    
    if(ajStrGetLen(thys->Header))
    {
	ajStrAssignS(&tmpstr, thys->Header);
	ajStrExchangeCC(&tmpstr, "\n", "\1# ");
	ajStrExchangeCC(&tmpstr, "\1", "\n");
	ajFmtPrintF(outf, "#\n");
	ajFmtPrintF(outf, "# %S\n", tmpstr);
	ajFmtPrintF(outf, "#\n");
    }
    
    if(ajStrGetLen(thys->SubHeader))
    {
	ajStrAssignS(&tmpstr, thys->SubHeader);
	ajStrExchangeCC(&tmpstr, "\n", "\1# ");
	ajStrExchangeCC(&tmpstr, "\1", "\n");
	ajFmtPrintF(outf, "#\n");
	ajFmtPrintF(outf, "# %S\n", tmpstr);
	ajFmtPrintF(outf, "#\n");
	ajStrDel(&thys->SubHeader);
    }
    
    if(!doSingle || thys->Multi)
	ajFmtPrintF(outf, "#=======================================\n\n");
    else
	ajFmtPrintF(outf, "########################################\n\n");
    
    ++thys->Count;
    
    ajStrDel(&tmpstr);
    
    return;
}




/* @func ajAlignWriteTail *****************************************************
**
** Writes an alignment tail
**
** @param [u] thys [AjPAlign] Alignment object
** @return [void]
** @category output [AjPAlign] Master footer output routine
** @@
******************************************************************************/

void ajAlignWriteTail(AjPAlign thys)
{
    AjPFile outf;
    AjPStr tmpstr = NULL;
    AjBool doSingle;

    if(!alignFormat[thys->Format].Showheader)
        return;

    outf     = thys->File;
    doSingle = ajFalse; /* turned off for now - always multi format */
    
    ajFmtPrintF(outf, "\n");
    
    if(!doSingle || thys->Multi)
	ajFmtPrintF(outf, "#---------------------------------------\n");
    else
	ajFmtPrintF(outf, "\n########################################\n");
    
    if(ajStrGetLen(thys->SubTail))
    {
	ajStrAssignS(&tmpstr, thys->SubTail);
	ajStrExchangeCC(&tmpstr, "\n", "\1# ");
	ajStrExchangeCC(&tmpstr, "\1", "\n");
	ajFmtPrintF(outf, "#\n");
	ajFmtPrintF(outf, "# %S\n", tmpstr);
	ajFmtPrintF(outf, "#\n");
	ajStrDel(&thys->SubTail);
    }

    if(ajStrGetLen(thys->Tail))
    {
	ajStrAssignS(&tmpstr, thys->Tail);
	ajStrExchangeCC(&tmpstr, "\n", "\1# ");
	ajStrExchangeCC(&tmpstr, "\1", "\n");
	ajFmtPrintF(outf, "#\n");
	ajFmtPrintF(outf, "# %S\n", tmpstr);
	ajFmtPrintF(outf, "#\n");
    }

    if(!doSingle || thys->Multi)
	ajFmtPrintF(outf, "#---------------------------------------\n");
    else
	ajFmtPrintF(outf, "########################################\n");

    ajStrDel(&tmpstr);

    return;
}




/* @func ajAlignSetHeader *****************************************************
**
** Defines an alignment header
**
** @param [u] thys [AjPAlign] Alignment object
** @param [r] header [const AjPStr] Align header with embedded newlines
** @return [void]
** @@
******************************************************************************/

void ajAlignSetHeader(AjPAlign thys, const AjPStr header)
{
    ajStrAssignS(&thys->Header, header);

    ajDebug("ajAlignSetHeader len %d '%S'\n",
	    ajStrGetLen(thys->Header), header);

    return;
}




/* @func ajAlignSetHeaderC ****************************************************
**
** Defines an alignment header
**
** @param [u] thys [AjPAlign] Alignment object
** @param [r] header [const char*] Align header with embedded newlines
** @return [void]
** @@
******************************************************************************/

void ajAlignSetHeaderC(AjPAlign thys, const char* header)
{
    ajStrAssignC(&thys->Header, header);

    ajDebug("ajAlignSetHeaderC len %d '%S'\n",
	    ajStrGetLen(thys->Header), header);

    return;
}




/* @func ajAlignSetHeaderApp **************************************************
**
** Appends to an alignment header
**
** @param [u] thys [AjPAlign] Alignment object
** @param [r] header [const AjPStr] Align header with embedded newlines
** @return [void]
** @@
******************************************************************************/

void ajAlignSetHeaderApp(AjPAlign thys, const AjPStr header)
{
    if(ajStrGetLen(thys->Header) && ajStrGetCharLast(thys->Header) != '\n')
	ajStrAppendC(&thys->Header, "\n");

    ajStrAppendS(&thys->Header, header);

    ajDebug("ajAlignSetHeaderApp len %d '%S'\n",
	    ajStrGetLen(thys->Header), header);

    return;
}




/* @func ajAlignSetTail *******************************************************
**
** Defines an alignment tail
**
** @param [u] thys [AjPAlign] Alignment object
** @param [r] tail [const AjPStr] Align tail with embedded newlines
** @return [void]
** @@
******************************************************************************/

void ajAlignSetTail(AjPAlign thys, const AjPStr tail)
{
    ajStrAssignS(&thys->Tail, tail);

    ajDebug("ajAlignSetTail len %d '%S'\n",
	    ajStrGetLen(thys->Tail), tail);

    return;
}




/* @func ajAlignSetTailC ******************************************************
**
** Defines an alignment tail
**
** @param [u] thys [AjPAlign] Alignment object
** @param [r] tail [const char*] Align tail with embedded newlines
** @return [void]
** @@
******************************************************************************/

void ajAlignSetTailC(AjPAlign thys, const char* tail)
{
    ajStrAssignC(&thys->Tail, tail);

    ajDebug("ajAlignSetTailC len %d '%S'\n",
	    ajStrGetLen(thys->Tail), tail);

    return;
}




/* @func ajAlignSetTailApp ****************************************************
**
** Appends to an alignment tail
**
** @param [u] thys [AjPAlign] Alignment object
** @param [r] tail [const AjPStr] Align tail with embedded newlines
** @return [void]
** @@
******************************************************************************/

void ajAlignSetTailApp(AjPAlign thys, const AjPStr tail)
{
    if(ajStrGetLen(thys->Tail) && ajStrGetCharLast(thys->Tail) != '\n')
	ajStrAppendC(&thys->Tail, "\n");

    ajStrAppendS(&thys->Tail, tail);

    ajDebug("ajAlignSetTailApp len %d '%S'\n",
	    ajStrGetLen(thys->Tail), tail);

    return;
}




/* @func ajAlignSetSubTail ****************************************************
**
** Defines an alignment tail
**
** @param [u] thys [AjPAlign] Alignment object
** @param [r] tail [const AjPStr] Align tail with embedded newlines
** @return [void]
** @@
******************************************************************************/

void ajAlignSetSubTail(AjPAlign thys, const AjPStr tail)
{
    ajStrAssignS(&thys->SubTail, tail);

    ajDebug("ajAlignSetSubTail len %d '%S'\n",
	    ajStrGetLen(thys->SubTail), tail);

    return;
}




/* @func ajAlignSetSubTailC ***************************************************
**
** Defines an alignment tail
**
** @param [u] thys [AjPAlign] Alignment object
** @param [r] tail [const char*] Align tail with embedded newlines
** @return [void]
** @@
******************************************************************************/

void ajAlignSetSubTailC(AjPAlign thys, const char* tail)
{
    ajStrAssignC(&thys->SubTail, tail);

    ajDebug("ajAlignSetSubTailC len %d '%S'\n",
	    ajStrGetLen(thys->SubTail), tail);

    return;
}




/* @func ajAlignSetSubTailApp *************************************************
**
** Appends to an alignment tail
**
** @param [u] thys [AjPAlign] Alignment object
** @param [r] tail [const AjPStr] Align tail with embedded newlines
** @return [void]
** @@
******************************************************************************/

void ajAlignSetSubTailApp(AjPAlign thys, const AjPStr tail)
{
    if(ajStrGetLen(thys->SubTail) && ajStrGetCharLast(thys->SubTail) != '\n')
	ajStrAppendC(&thys->SubTail, "\n");

    ajStrAppendS(&thys->SubTail, tail);

    ajDebug("ajAlignSetSubTailApp len %d '%S'\n",
	    ajStrGetLen(thys->SubTail), tail);

    return;
}




/* @func ajAlignSetSubHeader **************************************************
**
** Defines an alignment subheader (cleared after printing so it can
** be set again for the next alignment)
**
** @param [u] thys [AjPAlign] Alignment object
** @param [r] subheader [const AjPStr] Align subheader with embedded newlines
** @return [void]
** @@
******************************************************************************/

void ajAlignSetSubHeader(AjPAlign thys, const AjPStr subheader)
{
  ajStrAssignS(&thys->SubHeader, subheader);

  ajDebug("ajAlignSetSubHeader len %d '%S'\n",
	  ajStrGetLen(thys->SubHeader), subheader);

  return;
}




/* @func ajAlignSetSubHeaderC *************************************************
**
** Defines an alignment header (cleared after printing so it can
** be set again for the next alignment)
**
** @param [u] thys [AjPAlign] Alignment object
** @param [r] subheader [const char*] Align subheader with embedded newlines
** @return [void]
** @@
******************************************************************************/

void ajAlignSetSubHeaderC(AjPAlign thys, const char* subheader)
{
    ajStrAssignC(&thys->Header, subheader);

    ajDebug("ajAlignSetSubHeaderC len %d '%S'\n",
	    ajStrGetLen(thys->SubHeader), subheader);

    return;
}




/* @func ajAlignSetSubHeaderApp ***********************************************
**
** Appends to an alignment subheader (cleared after printing so it can
** be set again for the next alignment)
**
** @param [u] thys [AjPAlign] Alignment object
** @param [r] subheader [const AjPStr] Align subheader with embedded newlines
** @return [void]
** @@
******************************************************************************/

void ajAlignSetSubHeaderApp(AjPAlign thys, const AjPStr subheader)
{
    if(ajStrGetLen(thys->SubHeader) &&
       ajStrGetCharLast(thys->SubHeader) != '\n')
	ajStrAppendC(&thys->SubHeader, "\n");

    ajStrAppendS(&thys->SubHeader, subheader);

    ajDebug("ajAlignSetSubHeaderApp len %d '%S'\n",
	    ajStrGetLen(thys->SubHeader), subheader);

    return;
}




/* @func ajAlignSetSubHeaderPre ***********************************************
**
** Prepends to an alignment subheader (cleared after printing so it can
** be set again for the next alignment)
**
** @param [u] thys [AjPAlign] Alignment object
** @param [r] subheader [const AjPStr] Align subheader with embedded newlines
** @return [void]
** @@
******************************************************************************/

void ajAlignSetSubHeaderPre(AjPAlign thys, const AjPStr subheader)
{
    if(ajStrGetLen(thys->SubHeader) && ajStrGetCharLast(subheader) != '\n')
	ajStrInsertC(&thys->SubHeader, 0, "\n");

    ajStrInsertS(&thys->SubHeader, 0, subheader);

    ajDebug("ajAlignSetSubHeaderPre len %d '%S'\n",
	    ajStrGetLen(thys->SubHeader), subheader);

    return;
}




/* @func ajAlignSetMatrixName *************************************************
**
** Defines an alignment matrix
**
** @param [u] thys [AjPAlign] Alignment object
** @param [r] matrix [const AjPStr] Matrix name
** @return [void]
** @@
******************************************************************************/

void ajAlignSetMatrixName(AjPAlign thys, const AjPStr matrix)
{
    ajAlignSetMatrixNameC(thys, ajStrGetPtr(matrix));

    return;
}




/* @func ajAlignSetMatrixNameC ************************************************
**
** Defines an alignment matrix
**
** @param [u] thys [AjPAlign] Alignment object
** @param [r] matrix [const char*] Matrix name
** @return [void]
** @@
******************************************************************************/

void ajAlignSetMatrixNameC(AjPAlign thys, const char* matrix)
{
    ajStrAssignC(&thys->Matrix, matrix);

    return;
}




/* @func ajAlignSetMatrixInt **************************************************
**
** Defines an alignment matrix
**
** @param [u] thys [AjPAlign] Alignment object
** @param [u] matrix [AjPMatrix] Matrix object
** @return [void]
** @@
******************************************************************************/

void ajAlignSetMatrixInt(AjPAlign thys, AjPMatrix matrix)
{
    if(!thys->IMatrix)
    {
	thys->IMatrix = matrix;
	ajAlignSetMatrixName(thys, ajMatrixGetName(matrix));
    }

    if(thys->FMatrix)
	ajMatrixfDel(&thys->FMatrix);

    return;
}




/* @func ajAlignSetMatrixFloat ************************************************
**
** Defines an alignment matrix
**
** @param [u] thys [AjPAlign] Alignment object
** @param [u] matrix [AjPMatrixf] Matrix (floating point version) object
** @return [void]
** @@
******************************************************************************/

void ajAlignSetMatrixFloat(AjPAlign thys, AjPMatrixf matrix)
{
    if(!thys->FMatrix)
    {
	thys->FMatrix = matrix;
	ajAlignSetMatrixName(thys, ajMatrixfGetName(matrix));
    }

    if(thys->IMatrix)
	ajMatrixDel(&thys->IMatrix);

    return;
}




/* @func ajAlignSetGapI *******************************************************
**
** Defines alignment gap penalties
**
** @param [u] thys [AjPAlign] Alignment object
** @param [r] gappen [ajint] Gap penalty
** @param [r] extpen [ajint] Gap extension penalty
** @return [void]
** @@
******************************************************************************/

void ajAlignSetGapI(AjPAlign thys, ajint gappen, ajint extpen)
{
    AjPStr tmpstr = NULL;

    ajFmtPrintS(&tmpstr, "%d", gappen);
    ajStrAssignS(&thys->GapPen, tmpstr);

    ajFmtPrintS(&tmpstr, "%d", extpen);
    ajStrAssignS(&thys->ExtPen, tmpstr);

    ajStrDel(&tmpstr);

    return;
}




/* @func ajAlignSetGapR *******************************************************
 **
 ** Defines alignment gap penalties
 **
 ** @param [u] thys [AjPAlign] Alignment object
 ** @param [r] gappen [float] Gap penalty
 ** @param [r] extpen [float] Gap extension penalty
 ** @return [void]
 ** @@
 *****************************************************************************/

void ajAlignSetGapR(AjPAlign thys, float gappen, float extpen)
{
    AjPStr tmpstr = NULL;

    ajint precision = 3;
    ajint i;

    ajFmtPrintS(&tmpstr, "%.*f", precision, gappen);

    for(i=1; i<precision; i++)
    {
	if(ajStrGetCharLast(tmpstr) != '0') break;
	ajStrCutEnd(&tmpstr, 1);
    }

    ajStrAssignS(&thys->GapPen, tmpstr);

    ajFmtPrintS(&tmpstr, "%.*f", precision, extpen);

    for(i=1; i<precision; i++)
    {
	if(ajStrGetCharLast(tmpstr) != '0') break;
	ajStrCutEnd(&tmpstr, 1);
    }

    ajStrAssignS(&thys->ExtPen, tmpstr);

    ajStrDel(&tmpstr);
    return;
}




/* @func ajAlignSetScoreI *****************************************************
**
** Defines alignment score
**
** @param [u] thys [AjPAlign] Alignment object
** @param [r] score [ajint] score
** @return [void]
** @@
******************************************************************************/

void ajAlignSetScoreI(AjPAlign thys, ajint score)
{
    AjPStr tmpstr = NULL;
    AlignPData data = NULL;

    ajListPeekLast(thys->Data, (void**) &data);
    ajFmtPrintS(&tmpstr, "%d", score);
    ajStrAssignS(&data->Score, tmpstr);

    ajDebug("ajAlignSetScoreI: %d '%S' %d\n",
	    score, data->Score, data->LenAli);

    ajStrDel(&tmpstr);

    return;
}




/* @func ajAlignSetScoreL *****************************************************
**
** Defines alignment score
**
** @param [u] thys [AjPAlign] Alignment object
** @param [r] score [ajlong] score
** @return [void]
** @@
******************************************************************************/

void ajAlignSetScoreL(AjPAlign thys, ajlong score)
{
    AjPStr tmpstr = NULL;
    AlignPData data = NULL;

    ajListPeekLast(thys->Data, (void**) &data);
    ajFmtPrintS(&tmpstr, "%Ld", score);
    ajStrAssignS(&data->Score, tmpstr);

    ajDebug("ajAlignSetScoreI: %Ld '%S' %d\n",
	    score, data->Score, data->LenAli);
    ajStrDel(&tmpstr);

    return;
}




/* @func ajAlignSetScoreR *****************************************************
**
** Defines alignment score
**
** @param [u] thys [AjPAlign] Alignment object
** @param [r] score [float] score
** @return [void]
** @@
******************************************************************************/

void ajAlignSetScoreR(AjPAlign thys, float score)
{
    AjPStr tmpstr = NULL;

    ajint precision = 3;
    ajint i;
    AlignPData data = NULL;

    ajListPeekLast(thys->Data, (void**) &data);
    ajFmtPrintS(&tmpstr, "%.*f", precision, score);

    for(i=1; i<precision; i++)
    {
	if(ajStrGetCharLast(tmpstr) != '0') break;
	ajStrCutEnd(&tmpstr, 1);
    }

    ajStrAssignS(&data->Score, tmpstr);

    ajStrDel(&tmpstr);

    return;
}




/* @func ajAlignSetStats ******************************************************
**
** Sets standard properties for an alignment subheader. These are:
** Length, Identity, Gaps, Similarity, Score
**
** @param [u] thys [AjPAlign] Alignment object
** @param [r] iali [ajint] Alignment number
** @param [r] len [ajint] Alignment length
** @param [r] ident [ajint] Number of identities
** @param [r] sim [ajint] Number of similarities
** @param [r] gaps [ajint] Number of gaps
** @param [r] score [const AjPStr] Alignment score (as saved by
**                           ajAlignSetScoreI or ajAlignSetScoreR)
** @return [void]
******************************************************************************/

void ajAlignSetStats(AjPAlign thys, ajint iali, ajint len,
		     ajint ident, ajint sim, ajint gaps,
		     const AjPStr score)
{
    AlignPData* pdata = NULL;
    AlignPData data = NULL;
    ajint nali;
    
    nali = ajListToarray(thys->Data, (void***) &pdata);

    if(iali < 0)
	data = pdata[nali-1];
    else
	data = pdata[iali];
    
    /* ajDebug("ajAlignSetStats iali:%d len:%d id:%d "
               "sim:%d gap:%d score:'%S'\n",
	    iali, len, ident, sim, gaps, score); */
    
    data->LenAli = len;
    
    if(len > 0)
    {
	if(ident >= 0)
	    data->NumId = ident;
	else
	    data->NumId = -1;

	if(sim >= 0)
	    data->NumSim = sim;
	else
	    data->NumSim = -1;

	if(gaps >= 0)
	    data->NumGap = gaps;
	else
	    data->NumGap = -1;
    }

    if(ajStrGetLen(score))
	ajStrAssignS(&data->Score, score);
    
    AJFREE(pdata);
    
    return;
}




/* @func ajAlignSetSubStandard ************************************************
**
** Sets standard subheader using the properties for an alignment.
** These are:
** Length, Identity, Gaps, Similarity, Score
**
** @param [u] thys [AjPAlign] Alignment object
** @param [r] iali [ajint] Alignment number (or -1 for the latest)
** @return [void]
******************************************************************************/

void ajAlignSetSubStandard(AjPAlign thys, ajint iali)
{
    AjPStr tmphdr = NULL;
    AlignPData* pdata = NULL;
    AlignPData data = NULL;
    ajint nali;
    float pct;

    nali = ajListToarray(thys->Data, (void***) &pdata);

    if(iali < 0)
	data = pdata[nali-1];
    else
	data = pdata[iali];

    ajFmtPrintAppS(&tmphdr, "Length: %d\n", data->LenAli);

    if(data->LenAli > 0)
    {
	if(data->NumId >= 0)
	{
	    pct = (float)100. * (float) data->NumId / (float) data->LenAli;
	    ajFmtPrintAppS(&tmphdr, "Identity:   %5d/%d (%4.1f%%)\n",
			   data->NumId, data->LenAli, pct);
	}

	if(data->NumSim >= 0)
	{
	    pct = (float)100. * (float) data->NumSim / (float) data->LenAli;
	    ajFmtPrintAppS(&tmphdr, "Similarity: %5d/%d (%4.1f%%)\n",
			   data->NumSim, data->LenAli, pct);
	}

	if(data->NumGap >= 0)
	{
	    pct = (float)100. * (float) data->NumGap / (float) data->LenAli;
	    ajFmtPrintAppS(&tmphdr, "Gaps:       %5d/%d (%4.1f%%)\n",
			   data->NumGap, data->LenAli, pct);
	}
    }

    if(ajStrGetLen(data->Score))
	ajFmtPrintAppS(&tmphdr, "Score: %S\n", data->Score);

    ajAlignSetSubHeaderPre(thys, tmphdr);

    ajStrDel(&tmphdr);
    AJFREE(pdata);

    return;
}




/* @funcstatic alignSeqs ******************************************************
**
** Returns the sequences for the nth alignment
**
** @param [r] thys [const AjPAlign] Alignment object
** @param [r] iali [ajint] Alignment number
** @return [const AjPSeq*] Pointer to the internal sequence array
******************************************************************************/

static const AjPSeq* alignSeqs(const AjPAlign thys, ajint iali)
{
    AlignPData data = NULL;

    ajListPeekNumber(thys->Data, iali, (void**) &data);

    return data->Seq;
}




/* @funcstatic alignSeq *******************************************************
**
** Returns the nth sequence for an alignment
**
** @param [r] thys [const AjPAlign] Alignment object
** @param [r] iseq [ajint] Sequence number
** @param [r] iali [ajint] Alignment number
** @return [const AjPSeq] Pointer to the internal sequence
******************************************************************************/

static const AjPSeq alignSeq(const AjPAlign thys, ajint iseq, ajint iali)
{
    AlignPData data = NULL;

    ajListPeekNumber(thys->Data, iali, (void **) &data);

    return data->Seq[iseq];
}




/* @funcstatic alignSeqBegin **************************************************
**
** Returns the start position for the nth sequence for an alignment.
**
** Reverse direction is tested and the reverse complement numbering is used.
**
** @param [r] data [const AlignPData] Alignment data object
** @param [r] iseq [ajint] Sequence number
** @return [ajint] Start position
******************************************************************************/

static ajint alignSeqBegin(const AlignPData data, ajint iseq)
{
    ajint ret = 0;

    if (data->Rev[iseq])
    {
	ret = data->Len[iseq] - data->Offend[iseq]
	    - data->Start[iseq] + 1;
	ajDebug(" alignSeqBegin %d rev:%B len:%d Offend:%d Start:%d ret:%d\n",
		iseq, data->Rev[iseq], data->Len[iseq], data->Offend[iseq],
		data->Start[iseq], ret);
    }
    else
    {
	ret = data->Offset[iseq] + data->Start[iseq];
	ajDebug(" alignSeqBegin %d rev:%B Offset:%d Start:%d ret:%d\n",
		iseq, data->Rev[iseq], data->Offset[iseq],
		data->Start[iseq], ret);
    }

    return ret;
}




/* @funcstatic alignSeqEnd ****************************************************
**
** Returns the end position for the nth sequence for an alignment.
**
** Reverse direction is tested and the reverse complement numbering is used.
**
** @param [r] data [const AlignPData] Alignment data object
** @param [r] iseq [ajint] Sequence number
** @return [ajint] Start position
******************************************************************************/

static ajint alignSeqEnd(const AlignPData data, ajint iseq)
{
    ajint ret = 0;
    ajint iend;

    if (data->End[iseq])
	iend = data->End[iseq];
    else
	iend = data->Len[iseq];

    if (data->Rev[iseq])
	ret = data->Len[iseq] - data->Offend[iseq] - iend + 1;
    else
	ret = data->Offset[iseq] + iend;

    return ret;
}




/* @funcstatic alignSeqIncrement **********************************************
**
** Returns the position increment for the nth sequence for an alignment.
**
** Reverse direction gives a value of -1 for counting down.
**
** Forward direction gives +1.
**
** @param [r] data [const AlignPData] Alignment data object
** @param [r] iseq [ajint] Sequence number
** @return [ajint] Increment for position counting
******************************************************************************/

static ajint alignSeqIncrement(const AlignPData data, ajint iseq)
{
    ajint ret = 0;

    if (data->Rev[iseq])
	ret = -1;
    else
	ret = 1;

    return ret;
}




/* @funcstatic alignSeqGapBegin ***********************************************
**
** Counts the gaps at the start of the nth sequence for an alignment.
**
** Reverse direction is tested and the reverse complement numbering is used.
**
** @param [r] data [const AlignPData] Alignment data object
** @param [r] iseq [ajint] Sequence number
** @return [ajint] Start position
******************************************************************************/

static ajint alignSeqGapBegin(const AlignPData data, ajint iseq)
{
    ajint ret = 0;
    static char testchars[] = "-~.? "; /* all known gap characters */
    const char* cp;
    ajint i;

    if (data->Rev[iseq])
	ret = data->Offend[iseq];
    else
	ret = data->Offset[iseq];

    cp = ajSeqGetSeqC(data->Seq[iseq]);
    i = strspn(cp+data->Start[iseq]-1, testchars);

    return ret+i;
}




/* @funcstatic alignSeqGapEnd *************************************************
**
** Counts the gaps at the end of the nth sequence for an alignment.
**
** Reverse direction is tested and the reverse complement numbering is used.
**
** @param [r] data [const AlignPData] Alignment data object
** @param [r] iseq [ajint] Sequence number
** @return [ajint] End position
******************************************************************************/

static ajint alignSeqGapEnd(const AlignPData data, ajint iseq)
{
    ajint ret = 0;
    static char testchars[] = "-~.? "; /* all known gap characters */
    const char* cp;
    const char* cpstart;
    ajint i;

    if (data->Rev[iseq])
	ret = data->Offset[iseq];
    else
	ret = data->Offend[iseq];

    cpstart = ajSeqGetSeqC(data->Seq[iseq]);
    cp = cpstart + data->End[iseq];

    for(i=0; cp > cpstart; cp--)
    {
	if (!strchr(testchars, *cp))
	    return ret+i-1;
	i++;
    }

    return ret+data->End[iseq];
}




/* @funcstatic alignSeqRev **********************************************
**
** Returns the direction for the nth sequence for an alignment.
**
** @param [r] data [const AlignPData] Alignment data object
** @param [r] iseq [ajint] Sequence number
** @return [AjBool] ajTrue if sequence is reversed
******************************************************************************/

static AjBool alignSeqRev(const AlignPData data, ajint iseq)
{
    AjBool ret = ajFalse;

    if(data->Rev[iseq])
	ret = ajTrue;
    else
	ret = ajFalse;

    return ret;
}




/* @funcstatic alignData ******************************************************
**
** Returns the nth data structure for an alignment
**
** @param [r] thys [const AjPAlign] Alignment object
** @param [r] iali [ajint] Alignment number
** @return [AlignPData] Pointer to the internal alignment structure
******************************************************************************/

static AlignPData alignData(const AjPAlign thys, ajint iali)
{
    AlignPData data = NULL;

    ajListPeekNumber(thys->Data, iali, (void**) &data);

    return data;
}




/* @funcstatic alignLen *******************************************************
**
** Returns the length of the nth sequence for an alignment
**
** @param [r] thys [const AjPAlign] Alignment object
** @param [r] iali [ajint] Alignment number
** @return [ajint] Length of the internal sequence
******************************************************************************/

static ajint alignLen(const AjPAlign thys, ajint iali)
{
    AlignPData data = NULL;

    ajListPeekNumber(thys->Data, iali, (void**) &data);

    return data->LenAli;
}




/* @func ajAlignSetType *******************************************************
**
** Sets the align type (if it is not set already)
**
** @param [u] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

void ajAlignSetType(AjPAlign thys)
{
    const AjPSeq seq;
    ajint i;

    ajDebug("ajAlignSetType '%S'\n",
	    thys->Type);

    if(ajStrGetLen(thys->Type))
	return;

    if(!thys->Nseqs)
	return;

    for(i=0; i < thys->Nseqs; i++)
    {
	ajDebug("Calling alignSeq d 0\n", i);
	seq = alignSeq(thys, i, 0);

	if(ajStrGetLen(seq->Type))
	{
	    ajStrAssignS(&thys->Type, seq->Type);
	    return;
	}
    }

    ajDebug("testing alignSeq 0 0\n", i);

    if (ajSeqIsNuc(alignSeq(thys, 0, 0)))
	ajStrAssignC(&thys->Type, "N");
    else
	ajStrAssignC(&thys->Type, "P");

    return;
}




/* @func ajAlignSetExternal ***************************************************
**
** Sets the align object to use external sequence references, which are
** to be copied pointers rather than clones of the whole sequence.
**
** Intended for alignments of large sequences where there is no need to
** keep many copies. An example is the EMBOSS application wordmatch.
**
** @param [u] thys [AjPAlign] Alignment object
** @param [r] external [AjBool] If true, do not make copies of sequence data
**                              and do not delete internal sequence data
** @return [void]
** @@
******************************************************************************/

void ajAlignSetExternal(AjPAlign thys, AjBool external)
{
    ajDebug("ajAlignSetExternal old:%B new:%B\n",
	    thys->SeqExternal, external);

    thys->SeqExternal = external;

    return;
}




/* @func ajAlignSetRefSeqIndx ***************************************************
**
** Sets the index of the reference sequence.
**
** @param [u] thys [AjPAlign] Alignment object
** @param [r] refseq [ajint] index of the reference sequence
** @return [void]
** @@
******************************************************************************/

void ajAlignSetRefSeqIndx(AjPAlign thys, ajint refseq)
{

    thys->RefSeq = refseq;

    return;
}




/* @func ajAlignSetRange ******************************************************
**
** Sets the alignment range in each sequence, but only for a
** pairwise alignment
**
** @param [u] thys [AjPAlign] Alignment object
** @param [r] start1 [ajint] Start in sequence 1
** @param [r] end1 [ajint] End in sequence 1
** @param [r] len1 [ajint] Length of sequence 1
** @param [r] off1 [ajint] Offset of sequence 1
** @param [r] start2 [ajint] Start in sequence 2
** @param [r] end2 [ajint] End in sequence 2
** @param [r] len2 [ajint] Length of sequence 2
** @param [r] off2 [ajint] Offset of sequence 2
** @return [AjBool] ajTrue on success. Failure also writes an error message.
** @@
******************************************************************************/

AjBool ajAlignSetRange(AjPAlign thys,
		       ajint start1, ajint end1, ajint len1, ajint off1,
		       ajint start2, ajint end2, ajint len2, ajint off2)
{
    AlignPData data = NULL;
    ajint nali;

    ajDebug("ajAlignSetRange %d..%d (%d) %d..%d (%d)\n",
	    start1, end1, len1, start2, end2, len2);

    if(thys->Nseqs != 2)
    {
	ajErr("ajAlignSetRange requires alignment of 2 sequences only");
	return ajFalse;
    }

    ajListPeekLast(thys->Data, (void**) &data);

    nali = ajListGetLength(thys->Data);
    ajDebug("nali:%d set range %d\n", nali, nali-1);

    data->Start[0]  = start1;
    data->End[0]    = end1;
    data->Len[0]    = len1;
    data->Offset[0] = off1;
    data->Rev[0]    = ajFalse;

    data->Start[1]  = start2;
    data->End[1]    = end2;
    data->Len[1]    = len2;
    data->Offset[1] = off2;
    data->Rev[1]    = ajFalse;

    if(thys->SeqExternal)
    {
	data->LenAli = (end1 - start1) + 1;

	if(data->LenAli < (end2 - start2 + 1))
	    data->LenAli = (end2 - start2) + 1;

	ajDebug("len:  %d\n", data->LenAli);
    }

    return ajTrue;
}




/* @func ajAlignSetSubRange ***************************************************
**
** Sets the local alignment sub range in each sequence, but only for a
** pairwise alignment.
**
** This sets the SubOffset in addition to the Start and End values,
** for use where the alignment has a large sequence, but only part of
** it is to be reported.
**
** The usual example is where there are many matches in a long sequence,
** defined with SeqExternal so we use pointers to one original to avoid
** making multiple copies in memory while building the AjPAlign and AlignPData
** structure.
**
** Resets the alignment length to be the length of the longest subsequence
**
** @param [u] thys [AjPAlign] Alignment object
** @param [r] substart1 [ajint] Subsequence offset in sequence 1
** @param [r] start1 [ajint] Subsequence start in sequence 1
** @param [r] end1 [ajint] Subsequence end in sequence 1
** @param [r] rev1 [AjBool] ajTrue if sequence 1 is reversed
** @param [r] len1 [ajint] Length of sequence 1
** @param [r] substart2 [ajint] Subsequence offset in sequence 2
** @param [r] start2 [ajint] Subsequence start in sequence 2
** @param [r] end2 [ajint] Subsequence end in sequence 2
** @param [r] rev2 [AjBool] ajTrue if sequence 2 is reversed
** @param [r] len2 [ajint] Length of sequence 2
** @return [AjBool] ajTrue on success. Failure also writes an error message.
** @@
******************************************************************************/

AjBool ajAlignSetSubRange(AjPAlign thys,
			  ajint substart1, ajint start1,
			  ajint end1, AjBool rev1, ajint len1,
			  ajint substart2, ajint start2,
			  ajint end2, AjBool rev2, ajint len2)
{
    AlignPData data = NULL;
    ajint nali;

    ajDebug("ajAlignSetSubRange %d(%d)..%d (%d) %d(%d)..%d (%d)\n",
	    start1, substart1, end1, len1,
	    start2, substart2, end2, len2);

    if(thys->Nseqs != 2)
    {
	ajErr("ajAlignSetSubRange requires alignment of 2 sequences only");

	return ajFalse;
    }

    ajListPeekLast(thys->Data, (void**) &data);

    nali = ajListGetLength(thys->Data);
    ajDebug("nali:%d set range %d\n", nali, nali-1);

    data->SubOffset[0] = substart1;
    data->Start[0]     = start1;
    data->End[0]       = end1;
    data->Len[0]       = len1;
    data->Offset[0]    = substart1;
    data->Offend[0]    = len1 - (substart1 + end1 - start1 + 1);
    data->Rev[0]       = rev1;

    data->SubOffset[1] = substart2;
    data->Start[1]     = start2;
    data->End[1]       = end2;
    data->Len[1]       = len2;
    data->Offset[1]    = substart2;
    data->Offend[1]    = len2 - (substart2 + end2 - start2 + 1);
    data->Rev[1]       = rev2;

	data->LenAli = (end1 - start1) + 1;

	if(data->LenAli < (end2 - start2 + 1))
	{
	    data->LenAli = (end2 - start2) + 1;
	}

	ajDebug("len:  %d\n", data->LenAli);

    return ajTrue;
}




/* @funcstatic alignSeqName ***************************************************
**
** Returns the sequence name or USA depending on the setting in the
** Alignment object (derived from the ACD and command line -ausa option)
**
** @param [r] thys [const AjPAlign] Alignment object
** @param [r] seq [const AjPSeq] Sequence object
** @return [const AjPStr] Sequence name for this alignment
******************************************************************************/

static const AjPStr alignSeqName(const AjPAlign thys, const AjPSeq seq)
{
    if(thys->Showusa)
    return ajSeqGetUsaS(seq);

    return ajSeqGetNameS(seq);
}




/* @funcstatic alignDataDel ***************************************************
**
** Deletes an alignment data structure
**
** @param [d] pthys [AlignPData*] Alignment data structure
** @param [r] external [AjBool] Sequence is a pointer to an external
**                              object, do not delete.
** @return [void]
******************************************************************************/

static void alignDataDel(AlignPData* pthys, AjBool external)
{
    AlignPData thys;
    ajint i;
    void *freeptr = NULL;
    
    thys = *pthys;

    AJFREE(thys->Start);
    AJFREE(thys->End);
    AJFREE(thys->Len);
    AJFREE(thys->Offset);
    AJFREE(thys->Offend);
    AJFREE(thys->SubOffset);
    AJFREE(thys->Rev);

    ajStrDel(&thys->Score);
    ajDebug("alignDataDel NSeqs: %d External: %B\n",
	    thys->Nseqs, external);

    if(!external)
	for(i=0;i<thys->Nseqs;++i)
	{
	    ajDebug("alignDataDel seq[%d] %S %d\n",
		    i, ajSeqGetNameS(thys->Seq[i]), ajSeqGetLen(thys->Seq[i]));
	    ajSeqDel(&thys->RealSeq[i]);
        }

    AJFREE(thys->RealSeq);
    freeptr = (void *)thys->Seq;
    AJFREE(freeptr);
    AJFREE(*pthys);

    return;
}




/* @funcstatic alignDataNew ***************************************************
**
** Creates and initialises an alignment data object
** with a specified number of sequences
**
** @param [r] nseqs [ajint] Number of sequences alignment data object will
**                          store
** @param [r] external [AjBool] Sequence is a pointer to an external
**                              object, no need for an own copy.
** @return [AlignPData] Pointer to the new alignment data object
******************************************************************************/

static AlignPData alignDataNew(ajint nseqs, AjBool external)
{
    AlignPData data = NULL;

    AJNEW0(data);

    data->Nseqs = nseqs;
    AJCNEW0(data->Start, nseqs);
    AJCNEW0(data->End, nseqs);
    AJCNEW0(data->Len, nseqs);
    AJCNEW0(data->Offset, nseqs);
    AJCNEW0(data->Offend, nseqs);
    AJCNEW0(data->SubOffset, nseqs);
    AJCNEW0(data->Rev, nseqs);
    AJCNEW0(data->Seq, nseqs);
    if(!external)
        AJCNEW0(data->RealSeq, nseqs);

    return data;
}




/* @funcstatic alignDataSetSequence *******************************************
**
** Sets specified sequence in the alignment data object
**
** Note: In pre EMBOSS-6.3 the external option is set only by wordmatch
** and seqmatchall, both producing exact multiple matches and calling
** ajAlignSetSubRange after calling ajAlignDefineSS which repeats the settings
** made in this function when called via ajAlignDefineSS.
** Since ajSeqCountGaps function is time consuming for long sequences,
** function was modified to return quickly when the external option is true.
** After release 6.3 this function call hierarchy should be revised.
**
**
** @param [u] thys [AlignPData] Alignment data object
** @param [r] seq [const AjPSeq] Input sequence, either a new copy is made
**                               or a direct reference is made depending on
**                               the external parameter
** @param [r] iseq [ajint] Index of the sequence
** @param [r] external [AjBool] Sequence is a pointer to an external
**                              object
** @return [void]
******************************************************************************/

static void alignDataSetSequence(AlignPData thys, const AjPSeq seq, ajint iseq, 
        AjBool external)
{
    if(external)
    {
        thys->Seq[iseq] = seq;
        return;
    }

    thys->Start[iseq] = ajSeqGetBegin(seq);
    thys->End[iseq]   = ajSeqGetEnd(seq);
    thys->Len[iseq]   = ajSeqGetLen(seq) + ajSeqGetOffset(seq)
                        + ajSeqGetOffend(seq) - ajSeqCountGaps(seq);
    thys->Offset[iseq] = ajSeqGetOffset(seq);
    thys->Offend[iseq] = ajSeqGetOffend(seq);
    thys->SubOffset[iseq] = 0;
    thys->Rev[iseq]    = ajSeqIsReversed(seq);

    thys->RealSeq[iseq] = ajSeqNewSeq(seq);
    ajSeqGapStandard(thys->RealSeq[iseq], '-');
    thys->Seq[iseq] = thys->RealSeq[iseq];
    
    return;
}




/* @funcstatic alignDiff ******************************************************
**
** Blank out identities between two strings
**
** @param [w] pmark [AjPStr*] Mark string with spaces for identities
** @param [r] seq [const AjPStr] String (sequence) to compare
** @param [r] idchar [char] Character for identities
** @return [void]
******************************************************************************/

static void alignDiff(AjPStr* pmark, const AjPStr seq, char idchar)
{
    ajuint i;
    ajuint ilen;
    char c;
    char d;

    ilen = ajStrGetLen(seq);

    ajStrGetuniqueStr(pmark);

    if(ajStrGetLen(*pmark) < ilen)
	ilen = ajStrGetLen(*pmark);

    for(i=0; i < ilen; i++)
    {
	c = ajStrGetCharPos(*pmark, i);

	if(c == ' ')
            continue;

	if(c == '-')
            continue;

	d = ajStrGetCharPos(seq, i);

	if(d == ' ')
            continue;

	if(d == '-')
            continue;

	if(toupper((int)c) == toupper((int)d))
	    ajStrPasteCountK(pmark, i, idchar, 1);
    }

    return;
}




/* @funcstatic alignSame ******************************************************
**
** Blank out differences between two strings
**
** @param [w] pmark [AjPStr*] Mark string with spaces for differences
** @param [r] seq [const AjPStr] String (sequence) to compare
** @param [r] diffchar [char] Character for differences
** @return [void]
******************************************************************************/

static void alignSame(AjPStr* pmark, const AjPStr seq, char diffchar)
{
    ajuint i;
    ajuint ilen;
    char c;
    char d;

    ilen = ajStrGetLen(seq);

    ajStrGetuniqueStr(pmark);

    if(ajStrGetLen(*pmark) < ilen)
	ilen = ajStrGetLen(*pmark);

    for(i=0; i < ilen; i++)
    {
	c = ajStrGetCharPos(*pmark, i);

	if(c == ' ')
            continue;

	d = ajStrGetCharPos(seq, i);

	if(toupper((int)c) == toupper((int)d))
            continue;

	ajStrPasteCountK(pmark, i, diffchar, 1);
    }

    return;
}




/* @funcstatic alignSim *******************************************************
**
** Convert upper case (identical) positions to an identity character,
** and lower case (similar) positions to a similarity character
**
** @param [w] pmark [AjPStr*] Mark string with spaces for differences
** @param [r] idch [const char] Identity character
** @param [r] simch [const char] Similarity character
** @param [r] misch [const char] Mismatch character
** @param [r] gapch [const char] Gap character
** @return [void]
******************************************************************************/

static void alignSim(AjPStr* pmark, const char idch, const char simch,
		     const char misch, const char gapch)
{
    ajint i;
    ajint ilen;
    char c;

    /*ajDebug("alignSim '%S'\n", *pmark);*/

    ajStrGetuniqueStr(pmark);

    ilen = ajStrGetLen(*pmark);

    for(i=0; i < ilen; i++)
    {
	c = ajStrGetCharPos(*pmark, i);

	if(tolower((int)c) == '#')
	    ajStrPasteCountK(pmark, i, misch, 1);
	else if(isupper((int)c))
	    ajStrPasteCountK(pmark, i, idch, 1);
	else if(islower((int)c))
	    ajStrPasteCountK(pmark, i, simch, 1);
	else
	    ajStrPasteCountK(pmark, i,gapch, 1);
    }

    /*ajDebug("  result '%S'\n", *pmark);*/

    return;
}




/* @funcstatic alignConsStats *************************************************
**
** Calculates alignment statistics (and a consensus).
**
** @param [u] thys [AjPAlign] Alignment object. Matrix will be defined
**                            if not already set.
** @param [r] iali [ajint] alignment number
** @param [w] cons [AjPStr*] the created consensus sequence
** @param [w] retident [ajint*] number of residues identical in all sequences
** @param [w] retsim   [ajint*] number of residues similar in all sequences
** @param [w] retgap   [ajint*] number of residues with a gap in 1 sequence
** @param [w] retlen [ajint*] length of the alignment
** @return [void]
******************************************************************************/

static void alignConsStats(AjPAlign thys, ajint iali, AjPStr *cons,
			   ajint* retident, ajint* retsim, ajint* retgap,
			   ajint* retlen)
{
    ajint   imat;     /* iterate over identical and matching arrays */
    ajint   iseq;	     /* iterate over sequences (outer loop) */
    ajint   jseq;	     /* iterate over sequences (inner loop) */
    
    
    ajint   **matrix  = NULL;
    float   **fmatrix = NULL;

    ajint   m1 = 0;
    ajint   m2 = 0;

    ajint   matsize;
    ajint   matchingmaxindex;
    ajint   identicalmaxindex;
    ajint   nseqs;
    ajint   mlen;
    
    float   max;
    float   contri = 0;
    float   contrj = 0;
    
    AjPSeqCvt cvt  = 0;
    
    AjPFloat  posScore = NULL; /* cumulative similarity scores by sequence */
    /* for matching all other sequences */
    float   *identical;	    /* cum. weight for each valid character */
    float   *matching;	 /* cum. weight for matching this character */
    ajint   highindex;	   /* position of highest score in posScore */
    ajint   kkpos;		/* alignment position loop variable */
    ajint   kipos;	   /* alignment position kkpos + iseq start */
    ajint   kjpos;	   /* alignment position kkpos + jseq start */
    ajint   khpos;		 /* alignment position in highindex */
    
    float himatch = 0.0;	/* highest match score (often used) */
    
    const char **seqcharptr;
    char res;
    char nocon;
    char gapch;
    float fplural;
    float fplurality = 51.0;
    float ident;
    AjBool isident;
    AjBool issim;
    AjBool isgap;
    const AjPSeq* seqs;
    ajint numres;		 /* number of residues (not spaces) */
    AlignPData data = NULL;
    void *freeptr;
    
    
    data = alignData(thys, iali);

    if(!thys->IMatrix && !thys->FMatrix)
    {
	if(ajSeqIsNuc(alignSeq(thys, 0, iali)))
	    ajStrAssignC(&thys->Matrix, "EDNAFULL");
	else
	    ajStrAssignC(&thys->Matrix, "EBLOSUM62");

	thys->IMatrix = ajMatrixNewFile(thys->Matrix);
    }
    
    *retident=0;
    *retsim=0;
    *retgap=0;
    
    seqs    = alignSeqs(thys, iali);
    nseqs   = thys->Nseqs;
    mlen    = alignLen(thys, iali);
    fplural = alignTotweight(thys, 0) * fplurality / (float)100.;
    ident   = alignTotweight(thys, 0);
    
    ajDebug("alignConsStats ali:%d mlen:%d\n", iali, mlen);

    /* ajDebug("fplural:%.1f ident:%.1f mlen: %d\n",
	    fplural, ident, mlen); */
    
    if(thys->IMatrix)
    {
	matrix  = ajMatrixGetMatrix(thys->IMatrix);
	cvt     = ajMatrixGetCvt(thys->IMatrix); /* return conversion table */
	matsize = ajMatrixGetSize(thys->IMatrix);
    }
    else
    {
	fmatrix = ajMatrixfGetMatrix(thys->FMatrix);
	cvt     = ajMatrixfGetCvt(thys->FMatrix); /* return conversion table */
	matsize = ajMatrixfGetSize(thys->FMatrix);
    }
    
    AJCNEW(seqcharptr,nseqs);
    AJCNEW(identical,matsize);
    AJCNEW(matching,matsize);
    
    posScore = ajFloatNew();
    
    gapch = '-';
    nocon = '#';
    
    for(iseq=0;iseq<nseqs;iseq++)	/* get sequence as string */
	seqcharptr[iseq] = ajSeqGetSeqC(alignSeq(thys, iseq, iali));

    /* For each position in the alignment, calculate consensus character */
    
    for(kkpos=0; kkpos< mlen; kkpos++)
    {
	res = gapch;
	
	isident = ajFalse;
	issim   = ajFalse;
	isgap   = ajFalse;
	
	/*
	 ** reset identities and +ve matches
	 */
	
	for(imat=0;imat<matsize;imat++)
	{
	    identical[imat] = 0.0; /* weights of all sequence chars in col. */
	    matching[imat]  = 0.0;
	}
	
	/*
	 ** reset the posScore array
	 */
	
	for(iseq=0;iseq<nseqs;iseq++)
	    ajFloatPut(&posScore,iseq,0.);
	
	/*
	 ** generate scores (identical, posScore) for columns
	 */
	
	for(iseq=0;iseq<nseqs;iseq++)
	{
	    kipos = kkpos + data->SubOffset[iseq];
	    m1 = ajSeqcvtGetCodeK(cvt,seqcharptr[iseq][kipos]);

	    if (m1 < 0)
	    {
		ajUser("Bad character for matrix iseq:%d kipos:%d %x\n",
			iseq, kipos, (int) seqcharptr[iseq][kipos]);
		m1 = 0;
	    }

	    if(m1)
		identical[m1] += seqs[iseq]->Weight;
	    
	    for(jseq=iseq+1;jseq<nseqs;jseq++)
	    {
		kjpos = kkpos + data->SubOffset[jseq];
		m2 = ajSeqcvtGetCodeK(cvt,seqcharptr[jseq][kjpos]);

		if (m2 < 0)
		{
		    ajDebug("Bad character for matrix jseq:%d kjpos:%d %x\n",
			    jseq, kjpos, (int) seqcharptr[jseq][kjpos]);
		    m2 = 0;
		}

		if(m1 && m2)
		{
		    if(matrix)
		    {
			contri = (float)matrix[m1][m2]*seqs[jseq]->Weight
			  +ajFloatGet(posScore,iseq);
			contrj = (float)matrix[m1][m2]*seqs[iseq]->Weight
			  +ajFloatGet(posScore,jseq);
		    }
		    else
		    {
			contri = fmatrix[m1][m2]*seqs[jseq]->Weight
			  +ajFloatGet(posScore,iseq);
			contrj = fmatrix[m1][m2]*seqs[iseq]->Weight
			  +ajFloatGet(posScore,jseq);
		    }

		    ajFloatPut(&posScore,iseq,contri);
		    ajFloatPut(&posScore,jseq,contrj);
		}
	    }
	}
	
	/*
	 ** highindex is the highest scoring position (seq no.) in posScore
	 ** for 2 sequences this appears to be usually 0
	 */
	
	highindex = -1;
	max       = -FLT_MAX;
	numres    = 0;
	for(iseq=0;iseq<nseqs;iseq++)
	{
	    kipos = kkpos + data->SubOffset[iseq];

	    if(seqcharptr[iseq][kipos] != ' ' &&
	       seqcharptr[iseq][kipos] != '-')
		numres++;

	    if(ajFloatGet(posScore,iseq) > max)
	    {
		highindex = iseq;
		max       = ajFloatGet(posScore,iseq);
	    }
	}
	
	/* highindex is now set */
	
	/*
	 ** find +ve matches in the column
	 ** m1 is non-zero for a valid character in iseq
	 ** m2 is non-zero for a valid character in jseq
	 */
	
	for(iseq=0;iseq<nseqs;iseq++)
	{
	    kipos = kkpos + data->SubOffset[iseq];
	    m1    = ajSeqcvtGetCodeK(cvt, seqcharptr[iseq][kipos]);

	    if (m1 < 0)
	    {
		ajDebug("Bad character for matrix iseq:%d kipos:%d %x\n",
			iseq, kipos, (int) seqcharptr[iseq][kipos]);
		m1 = 0;
	    }

	    if(E_FPZERO(matching[m1],U_FEPS))
	    {   /* first time we have met this character */
		for(jseq=0;jseq<nseqs;jseq++) /* all (other) sequences */
		{
		    kjpos = kkpos + data->SubOffset[jseq];
		    m2    = ajSeqcvtGetCodeK(cvt, seqcharptr[jseq][kjpos]);

		    if (m2 < 0)
		    {
			ajDebug("Bad character for matrix jseq:%d kjpos:%d "
                                "%x\n",
				jseq, kjpos, (int) seqcharptr[jseq][kjpos]);
			m2 = 0;
		    }

		    if(matrix)
		    {
			/* 'matching' if positive */
			if(m1 && m2 && matrix[m1][m2] > 0) 
			    matching[m1] += seqs[jseq]->Weight;
		    }
		    else
			if(m1 && m2 && fmatrix[m1][m2] > 0.0)
			    matching[m1] += seqs[jseq]->Weight;


		    /*
		       if( iseq != jseq)
		       /# skip the sequence we are on #/
		       {
		       m2 = ajSeqcvtGetCodeK(cvt, seqcharptr[jseq][kjpos]);
		       if(matrix)
		       {
		       if(m1 && m2 && matrix[m1][m2] > 0)
		       /# 'matching' if positive #/
		       {
		       matching[m1] += seqs[jseq]->Weight;
		       }
		       }
		       else
		       {
		       if(m1 && m2 && fmatrix[m1][m2] > 0.0)
		       {
		       matching[m1] += seqs[jseq]->Weight;
		       }
		       }
		       }
		       */
		}
	    }
	}
	
	matchingmaxindex  = 0;	  /* get max matching and identical */
	identicalmaxindex = 0;

	for(iseq=0;iseq<nseqs;iseq++)
	{
	    kipos = kkpos + data->SubOffset[iseq];
	    m1 = ajSeqcvtGetCodeK(cvt,seqcharptr[iseq][kipos]);

	    if(identical[m1] > identical[identicalmaxindex])
		identicalmaxindex= m1;
	}
	
	for(iseq=0;iseq<nseqs;iseq++)
	{
	    kipos = kkpos + data->SubOffset[iseq];
	    m1 = ajSeqcvtGetCodeK(cvt,seqcharptr[iseq][kipos]);

	    if(matching[m1] > matching[matchingmaxindex])
		matchingmaxindex= m1;
	    else if(E_FPEQ(matching[m1],matching[matchingmaxindex],U_FEPS))
	    {
		if(identical[m1] > identical[matchingmaxindex])
		    matchingmaxindex= m1;
	    }

	    if(seqcharptr[iseq][kipos] == '-' ||
	       seqcharptr[iseq][kipos] == ' ')
		isgap=ajTrue;
	}

	/*
	   ajDebug("index[%d] ident:%d matching:%d high:%d\n",
	   kpos,
	   identicalmaxindex,
	   matchingmaxindex, highindex);
	   */
	khpos   = kkpos + data->SubOffset[highindex];
	himatch = matching[ajSeqcvtGetCodeK(cvt,seqcharptr[highindex][khpos])];
	
	if(thys->IMatrix)
	{
/*	    debugstr1 = ajMatrixGetLabelNum(thys->IMatrix, identicalmaxindex-1);
	    debugstr2 = ajMatrixGetLabelNum(thys->IMatrix, matchingmaxindex-1);
*/

	    /*ajDebug("index[%d] ident:%d '%S' %.1f matching:%d '%S' %.1f %.1f "
		    "high:%d '%c' %.1f\n",
		    kkpos,
		    identicalmaxindex,
		    debugstr1, 
		    identical[identicalmaxindex],
		    matchingmaxindex,
		    debugstr2,      
		    matching[matchingmaxindex],
		    himatch,
		    highindex, seqcharptr[highindex][khpos],
		    seqs[highindex]->Weight); */
	}
	else
	{
	    /*debugstr1 = ajMatrixfGetLabelNum(thys->FMatrix,identicalmaxindex-1);
              debugstr2 = ajMatrixfGetLabelNum(thys->FMatrix, matchingmaxindex-1);*/
	    /* ajDebug("index[%d] ident:%d '%S' %.1f matching:%d '%S' %.1f "
                       "%.1f "
		    "high:%d '%c' %.1f\n",
		    kkpos,
		    identicalmaxindex,
		    debugstr1, 
		    identical[identicalmaxindex],
		    matchingmaxindex,
		    debugstr2, 
		    matching[matchingmaxindex],
		    himatch,
		    highindex, seqcharptr[highindex][khpos],
		    seqs[highindex]->Weight); */
	}
		
	if(identical[identicalmaxindex] >= ident)
	    isident=ajTrue;

	if(matching[matchingmaxindex] >= fplural)
	    issim=ajTrue;
	
	/* plurality check */
	if(himatch >= fplural)
	    if(seqcharptr[highindex][khpos] != '-')
		res = toupper((int)seqcharptr[highindex][khpos]);


	if(E_FPEQ(himatch,seqs[highindex]->Weight,U_FEPS))
	{
	    if(numres > 1)
		res = nocon;
	    else
		res = gapch;
	}
	
	if(issim && ! isident)
	    res = tolower((int)res);
	
	ajStrAppendK(cons,res);

	if(isident)
	    ++*retident;

	if(issim)
	    ++*retsim;

	if(isgap)
	    ++*retgap;
	
	/* ajDebug("id:%b sim:%b gap:%b res:%c '",
                   isident, issim, isgap, res); */
	for(iseq=0; iseq<nseqs; iseq++)
	{
	    kipos = kkpos + data->SubOffset[iseq];
	    /* ajDebug("%c", seqcharptr[iseq][kipos]); */
	}
	/* ajDebug("\n"); */	
    }
    
    *retlen = alignLen(thys, iali);
    
    /* ajDebug("ret ident:%d sim:%d gap:%d len:%d\n",
	    *retident, *retsim, *retgap, *retlen); */

    freeptr = (void *) seqcharptr;
    AJFREE(freeptr);

    AJFREE(matching);
    AJFREE(identical);
    ajFloatDel(&posScore);

    return;    
}




/* @funcstatic alignTotweight *************************************************
**
** Returns the total weight for all sequences in an alignment.
**
** @param [r] thys [const AjPAlign] Alignment object
** @param [r] iali [ajint] Alignment number
** @return [float] Total weight for all sequences
******************************************************************************/

static float alignTotweight(const AjPAlign thys, ajint iali)
{
    ajint i;
    const AjPSeq seq = NULL;
    float ret  = 0.0;

    for(i=0; i < thys->Nseqs; i++)
    {
	seq = alignSeq(thys, i, iali);
	ret += seq->Weight;
    }

    return ret;
}




/* @func ajAlignTraceT ********************************************************
**
** Reports an AjPAlign object to debug output
**
** @param [r] thys [const AjPAlign] alignment object
** @param [r] title [const char*] Trace report title
** @return [void]
******************************************************************************/

void ajAlignTraceT(const AjPAlign thys, const char* title)
{
    ajDebug("%s\n",title);
    ajAlignTrace(thys);

    return;
}




/* @func ajAlignTrace *********************************************************
**
** Reports an AjPAlign object to debug output
**
** @param [r] thys [const AjPAlign] alignment object
** @return [void]
******************************************************************************/

void ajAlignTrace(const AjPAlign thys)
{
    ajDebug("AjAlignTrace\n");
    ajDebug("============\n");
    ajDebug("Type: '%S'\n", thys->Type);
    ajDebug("Formatstr: '%S'\n", thys->Formatstr);
    ajDebug("Format: %d\n", thys->Format);
    ajDebug("File: '%F'\n", thys->File);
    ajDebug("Header: '%S'\n", thys->Header);
    ajDebug("SubHeader: '%S'\n", thys->SubHeader);
    ajDebug("Tail: '%S'\n", thys->Tail);
    ajDebug("SubTail: '%S'\n", thys->SubTail);
    ajDebug("Showusa: %B\n", thys->Showusa);
    ajDebug("Multi: %B\n", thys->Multi);
    ajDebug("Global: %B\n", thys->Global);

    alignTraceData(thys);

    ajDebug("Nseqs: %d\n", thys->Nseqs);
    ajDebug("WidthNmin: %d\n", thys->Nmin);
    ajDebug("Nmax: %d\n", thys->Nmax);
    ajDebug("Width: %d\n", thys->Width);
    ajDebug("Count: %d\n", thys->Count);

    ajDebug("IMatrix: %x\n", thys->IMatrix);
    ajDebug("FMatrix: %x\n", thys->FMatrix);
    ajDebug("Matrix: '%S'\n", thys->Matrix);
    ajDebug("GapPen: '%S'\n", thys->GapPen);
    ajDebug("ExtPen: '%S'\n", thys->ExtPen);
    ajDebug("SeqOnly: %B\n", thys->SeqOnly);
    ajDebug("SeqExternal: %B\n", thys->SeqExternal);

    return;
}




/* @funcstatic alignTraceData *************************************************
**
** Report alignment internal list data to debug output
**
** @param [r] thys [const AjPAlign] Alignment object
** @return [void]
******************************************************************************/

static void alignTraceData(const AjPAlign thys)
{
    AlignPData* pdata = NULL;
    AlignPData data = NULL;
    ajint nali;
    ajint iali;
    ajint i;
    ajint nseq;
    AjPStr tmpstr = NULL;
    
    nseq = thys->Nseqs;
    
    nali = ajListToarray(thys->Data, (void***) &pdata);
    ajDebug("Data list length: %d\n", nali);

    if(!nali)
	return;
    
    for(iali=0; iali<nali; iali++)
    {
	data = pdata[iali];
	
	ajDebug("%d LenAli: %d\n", iali, data->LenAli);
	ajDebug("%d NumId: %d\n", iali, data->NumId);
	ajDebug("%d NumSim: %d\n", iali, data->NumSim);
	ajDebug("%d NumGap: %d\n", iali, data->NumGap);
	ajDebug("%d Score: '%S'\n", iali, data->Score);
	
	ajDebug("%d Start: {", iali);

	for(i=0; i < nseq; i++)
	    ajDebug(" %d", data->Start[i]);

	ajDebug(" }\n");
	
	ajDebug("%d End: {", iali);

	for(i=0; i < nseq; i++)
	    ajDebug(" %d", data->End[i]);

	ajDebug(" }\n");
	
	ajDebug("%d Len: {", iali);

	for(i=0; i < nseq; i++)
	    ajDebug(" %d", data->Len[i]);

	ajDebug(" }\n");
	
	ajDebug("%d Offset: {", iali);

	for(i=0; i < nseq; i++)
	    ajDebug(" %d", data->Offset[i]);

	ajDebug(" }\n");
	
	ajDebug("%d SubOffset: {", iali);

	for(i=0; i < nseq; i++)
	    ajDebug(" %d", data->SubOffset[i]);

	ajDebug(" }\n");
	
	ajDebug("%d Rev: {", iali);

	for(i=0; i < nseq; i++)
	    ajDebug(" %b", data->Rev[i]);

	ajDebug(" }\n");
	
	ajDebug("%d Seq: {\n", iali);

	for(i=0; i < nseq; i++)
	    if(ajSeqGetLen(data->Seq[i]) > 40)
	    {
		ajStrAssignSubS(&tmpstr, ajSeqGetSeqS(data->Seq[i]), -20, -1);
		ajDebug("%6d [%d] '%20.20S...%20S'\n",
			ajSeqGetLen(data->Seq[i]), i,
			ajSeqGetSeqS(data->Seq[i]),
			tmpstr);
	    }
	    else
		ajDebug("  %d '%S'\n", i, ajSeqGetSeqS(data->Seq[i]));

	ajDebug("  }\n");	
    }
    AJFREE(pdata);
    ajStrDel(&tmpstr);
    
    return;
}




/* @func ajAlignPrintFormat ***************************************************
**
** Reports the internal data structures
**
** @param [u] outf [AjPFile] Output file
** @param [r] full [AjBool] Full report (usually ajFalse)
** @return [void]
** @@
******************************************************************************/

void ajAlignPrintFormat(AjPFile outf, AjBool full)
{
    ajint i = 0;

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# alignment output formats\n");
    ajFmtPrintF(outf, "# Name    Format name (or alias)\n");
    ajFmtPrintF(outf, "# Minseq  Minimum number of sequences\n");
    ajFmtPrintF(outf, "# Maxseq  Minimum number of sequences\n");
    ajFmtPrintF(outf, "# Nuc     Valid for nucleotide sequences\n");
    ajFmtPrintF(outf, "# Pro     Valid for protein sequences\n");
    ajFmtPrintF(outf, "# Header  Include standard header/footer blocks\n");
    ajFmtPrintF(outf, "# Desc    Format description\n");
    ajFmtPrintF(outf, "# Name         Alias Nuc Nuc Pro Minseq Maxseq "
		"Description\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "AFormat {\n");

    for(i=0; alignFormat[i].Name; i++)
    {
	if(full || !alignFormat[i].Alias)
	    ajFmtPrintF(outf, "  %-12s %5B %3B %3B %3B  %6d %6d \"%s\"\n",
			alignFormat[i].Name,
			alignFormat[i].Alias,
			alignFormat[i].Nucleotide,
			alignFormat[i].Protein,
			alignFormat[i].Showheader,
			alignFormat[i].Minseq,
			alignFormat[i].Maxseq,
			alignFormat[i].Desc);
    }

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajAlignPrintbookFormat ************************************************
**
** Reports the alignment format internals as Docbook text
**
** @param [u] outf [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

void ajAlignPrintbookFormat(AjPFile outf)
{
    ajint i = 0;
    ajint j = 0;
    AjPStr namestr = NULL;
    AjPList fmtlist;
    AjPStr* names;

    fmtlist = ajListstrNew();

    ajFmtPrintF(outf, "<para>The supported alignment formats are summarised "
                "in the table below. The columns are as follows: "
                "<emphasis>Output format</emphasis> (format name), "
                "<emphasis>Nuc</emphasis> (\"true\" indicates nucleotide "
                "sequence data may be represented), "
                "<emphasis>Pro</emphasis> (\"true\" indicates protein "
                "sequence data may be represented, "
                "<emphasis>Header</emphasis> (whether the standard EMBOSS "
                "alignment header is included), "
                "<emphasis>Minseq</emphasis> "
                "(minimum sequences in alignment), "
                "<emphasis>Maxseq</emphasis> "
                "(maximum sequences in alignment) and "
                "<emphasis>Description</emphasis> (short description of "
                "the format).</para> \n\n");

    ajFmtPrintF(outf, "<table frame=\"box\" rules=\"cols\">\n");
    ajFmtPrintF(outf, "  <caption>Alignment formats</caption>\n");
    ajFmtPrintF(outf, "  <thead>\n");
    ajFmtPrintF(outf, "    <tr align=\"center\">\n");
    ajFmtPrintF(outf, "      <th>Output Format</th>\n");
    ajFmtPrintF(outf, "      <th>Nuc</th>\n");
    ajFmtPrintF(outf, "      <th>Pro</th>\n");
    ajFmtPrintF(outf, "      <th>Header</th>\n");
    ajFmtPrintF(outf, "      <th>Minseq</th>\n");
    ajFmtPrintF(outf, "      <th>Maxseq</th>\n");
    ajFmtPrintF(outf, "      <th>Description</th>\n");
    ajFmtPrintF(outf, "    </tr>\n");
    ajFmtPrintF(outf, "  </thead>\n");
    ajFmtPrintF(outf, "  <tbody>\n");

    for(i=1; alignFormat[i].Name; i++)
    {
	if(!alignFormat[i].Alias)
        {
            namestr = ajStrNewC(alignFormat[i].Name);
            ajListPush(fmtlist, namestr);
            namestr = NULL;
        }
    }

    ajListSort(fmtlist, ajStrVcmp);
    ajListstrToarray(fmtlist, &names);

    for(i=0; names[i]; i++)
    {
        for(j=0; alignFormat[j].Name; j++)
        {
            if(ajStrMatchC(names[i],alignFormat[j].Name))
            {
                ajFmtPrintF(outf, "    <tr>\n");
                ajFmtPrintF(outf, "      <td>%s</td>\n",
                            alignFormat[j].Name);
                ajFmtPrintF(outf, "      <td>%B</td>\n",
                            alignFormat[j].Nucleotide);
                ajFmtPrintF(outf, "      <td>%B</td>\n",
                            alignFormat[j].Protein);
                ajFmtPrintF(outf, "      <td>%B</td>\n",
                            alignFormat[j].Showheader);
                ajFmtPrintF(outf, "      <td>%d</td>\n",
                            alignFormat[j].Minseq);
                ajFmtPrintF(outf, "      <td>%d</td>\n",
                            alignFormat[j].Maxseq);
                ajFmtPrintF(outf, "      <td>%s</td>\n",
                            alignFormat[j].Desc);
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




/* @func ajAlignPrintwikiFormat ************************************************
**
** Reports the alignment format internals as wikitext
**
** @param [u] outf [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

void ajAlignPrintwikiFormat(AjPFile outf)
{
    ajint i = 0;
    ajint j = 0;
    AjPStr namestr = NULL;

    ajFmtPrintF(outf, "{| class=\"wikitable sortable\" border=\"2\"\n");
    ajFmtPrintF(outf, "|-\n");
    ajFmtPrintF(outf, "!Format!!Nuc!!Pro!!Header!!Min||Max!!"
                "class=\"unsortable\"|Description\n");

    for(i=1; alignFormat[i].Name; i++)
    {
        if(!alignFormat[i].Alias)
        {
            ajFmtPrintF(outf, "|-\n");
            ajStrAssignC(&namestr, alignFormat[i].Name);


            for(j=i+1; alignFormat[j].Name; j++)
            {
                if(alignFormat[j].Write == alignFormat[i].Write)
                {
                    ajFmtPrintAppS(&namestr, "<br>%s", alignFormat[j].Name);
                    if(!alignFormat[j].Alias) 
                    {
                        ajWarn("Align output format '%s' same as '%s' "
                               "but not alias",
                               alignFormat[j].Name,
                               alignFormat[i].Name);
                    }
                }
            }
            ajFmtPrintF(outf, "|%S||%B||%B||%B||%d||%d||%s\n",
			namestr,
			alignFormat[i].Nucleotide,
			alignFormat[i].Protein,
			alignFormat[i].Showheader,
			alignFormat[i].Minseq,
			alignFormat[i].Maxseq,
			alignFormat[i].Desc);
        }
    }

    ajFmtPrintF(outf, "|}\n");

    ajStrDel(&namestr);

    return;
}




/* @funcstatic alignConsSet ***************************************************
**
** Sets consensus to be a copy of a numbered sequence
**
** @param [r] thys [const AjPAlign] Alignment object
** @param [r] iali [ajint] alignment number
** @param [r] numseq [ajint] Sequence number to use as the consensus
** @param [w] cons [AjPStr*] the created consensus sequence
** @return [AjBool] True on success
******************************************************************************/

static AjBool alignConsSet(const AjPAlign thys, ajint iali,
			   ajint numseq, AjPStr *cons)
{
    ajint   nseqs;
    ajint   mlen;
    
    ajint   kkpos;		/* alignment position loop variable */
    
    const char *seqcharptr;
    char gapch;
    AlignPData data = NULL;
    const AjPSeq* seqs;
    const AjPSeq seq;

    data = alignData(thys, iali);
    seqs = alignSeqs(thys, iali);

    nseqs   = thys->Nseqs;
    mlen    = data->LenAli;
    
    ajDebug("alignConsSet iali:%d numseq:%d nseqs:%d mlen:%d\n",
	    iali, numseq, nseqs, mlen);
        
    gapch = '-';
    
    seq = seqs[numseq];
    seqcharptr = ajSeqGetSeqC(seq);

    ajStrAssignClear(cons);

    /* For each position in the alignment, calculate consensus character */
    
    for(kkpos=0; kkpos<data->SubOffset[0]; kkpos++)
	ajStrAppendK(cons, gapch);

    for(kkpos=0; kkpos< mlen; kkpos++)
	ajStrAppendK(cons,seqcharptr[kkpos+data->SubOffset[numseq]]);	
    
    /* ajDebug("ret ident:%d sim:%d gap:%d len:%d\n",
	    *retident, *retsim, *retgap, *retlen); */

    return ajTrue;    
}




/* @func ajAlignConsAmbig *************************************************
**
** Calculates alignment consensus of ambiguity characters from a sequence set.
**
** @param [r] thys [const AjPSeqset] Sequence set.
** @param [w] cons [AjPStr*] the created consensus sequence
** @return [AjBool] ajTrue on success
******************************************************************************/

AjBool ajAlignConsAmbig(const AjPSeqset thys, AjPStr *cons)
{
    if(ajSeqsetIsNuc(thys))
        return ajAlignConsAmbigNuc(thys, cons);

    return ajAlignConsAmbigProt(thys, cons);
}




/* @func ajAlignConsAmbigNuc **************************************************
**
** Calculates alignment consensus of ambiguity characters from a sequence set.
**
** @param [r] thys [const AjPSeqset] Sequence set.
** @param [w] cons [AjPStr*] the created consensus sequence
** @return [AjBool] ajTrue on success
******************************************************************************/

AjBool ajAlignConsAmbigNuc(const AjPSeqset thys, AjPStr *cons)
{
    ajint   iseq;	     /* iterate over sequences (outer loop) */
    
    ajint   nseqs;
    ajint   mlen;
    AjBool  isgap;
    ajint   kipos;	   /* alignment position */
    
    const char **seqcharptr;
    char res;
    ajuint binres;

    void *freeptr = NULL;

    nseqs   = thys->Size;
    mlen    = thys->Len;
    
    AJCNEW(seqcharptr,nseqs);

    /* ajDebug("fplural:%.2f ident:%.1f mlen: %d\n",
	    fplural, ident, mlen); */
    
    for(iseq=0;iseq<nseqs;iseq++)	/* get sequence as string */
	seqcharptr[iseq] =  ajSeqsetGetseqSeqC(thys, iseq);
    
    /* For each position in the alignment, calculate consensus character */
    
    for(kipos=0; kipos< mlen; kipos++)
    {
        binres = 0;
        isgap = ajFalse;

	for(iseq=0;iseq<nseqs;iseq++)
	{
	    if(seqcharptr[iseq][kipos] == ' ' ||
	       seqcharptr[iseq][kipos] == '-')
                isgap = ajTrue;

            binres |= ajBaseAlphaToBin(seqcharptr[iseq][kipos]);
	}

        res = ajBaseBinToAlpha(binres);

        if(isgap)
            res = (char)tolower((int)res);
	
	ajStrAppendK(cons,res);
    }

    freeptr = (void *) seqcharptr;
    AJFREE(freeptr);

    return ajTrue;    
}




/* @func ajAlignConsAmbigProt *************************************************
**
** Calculates alignment consensus of ambiguity characters from a sequence set.
**
** @param [r] thys [const AjPSeqset] Sequence set.
** @param [w] cons [AjPStr*] the created consensus sequence
** @return [AjBool] ajTrue on success
******************************************************************************/

AjBool ajAlignConsAmbigProt(const AjPSeqset thys, AjPStr *cons)
{
    ajint   iseq;	     /* iterate over sequences (outer loop) */
    
    ajint   nseqs;
    ajint   mlen;
    AjBool  isgap;
    ajint   kipos;	   /* alignment position */
    
    const char **seqcharptr;
    char res;
    ajuint binres;

    void *freeptr = NULL;

    nseqs   = thys->Size;
    mlen    = thys->Len;
    
    AJCNEW(seqcharptr,nseqs);

    /* ajDebug("fplural:%.2f ident:%.1f mlen: %d\n",
	    fplural, ident, mlen); */
    
    for(iseq=0;iseq<nseqs;iseq++)	/* get sequence as string */
	seqcharptr[iseq] =  ajSeqsetGetseqSeqC(thys, iseq);
    
    /* For each position in the alignment, calculate consensus character */
    
    for(kipos=0; kipos< mlen; kipos++)
    {
        binres = 0;
        isgap = ajFalse;

	for(iseq=0;iseq<nseqs;iseq++)
	{
	    if(seqcharptr[iseq][kipos] == ' ' ||
	       seqcharptr[iseq][kipos] == '-')
                isgap = ajTrue;
            binres |= ajResidueAlphaToBin(seqcharptr[iseq][kipos]);
	}

        res = ajResidueBinToAlpha(binres);

        if(isgap)
            res = (char)tolower((int)res);
	
	ajStrAppendK(cons,res);
    }

    freeptr = (void *) seqcharptr;
    AJFREE(freeptr);

    return ajTrue;    
}




/* @func ajAlignConsStats ****************************************************
**
** Calculates alignment statistics (and a consensus) from a sequence set.
**
** @param [r] thys [const AjPSeqset] Sequence set.
** @param [w] mymatrix [AjPMatrix] User-defined matrix, or NULL for the default
** @param [w] cons [AjPStr*] the created consensus sequence
** @param [w] retident [ajint*] number of residues identical in all sequences
** @param [w] retsim   [ajint*] number of residues similar in all sequences
** @param [w] retgap   [ajint*] number of residues with a gap in 1 sequence
** @param [w] retlen [ajint*] length of the alignment
** @return [AjBool] ajTrue on success
******************************************************************************/

AjBool ajAlignConsStats(const AjPSeqset thys, AjPMatrix mymatrix, AjPStr *cons,
			 ajint* retident, ajint* retsim, ajint* retgap,
			 ajint* retlen)
{
    AjPMatrix imatrix = NULL;
    AjPStr matname    = NULL;
    ajint   imat;     /* iterate over identical and matching arrays */
    ajint   iseq;	     /* iterate over sequences (outer loop) */
    ajint   jseq;	     /* iterate over sequences (inner loop) */
    
    ajint   **matrix  = NULL;
    float   **fmatrix = NULL;
    ajint   m1 = 0;
    ajint   m2 = 0;
    ajint   matsize;
    ajint   matchingmaxindex;
    ajint   identicalmaxindex;
    ajint   nseqs;
    ajint   mlen;
    
    float   max;
    float   contri = 0;
    float   contrj = 0;
    
    AjPSeqCvt cvt = 0;
    AjPFloat  posScore=NULL; /* cumulative similarity scores by sequence */
    /* for matching all other sequences */
    float   *identical;	    /* cum. weight for each valid character */
    float   *matching;	 /* cum. weight for matching this character */
    ajint   highindex;	   /* position of highest score in posScore */
    ajint   kkpos;		/* alignment position loop variable */
    ajint   kipos;	   /* alignment position kkpos + iseq start */
    ajint   kjpos;	   /* alignment position kkpos + jseq start */
    ajint   khpos;		 /* alignment position in highindex */
    
    float himatch = 0.0;	/* highest match score (often used) */
    
    const char **seqcharptr;
    char res;
    char nocon;
    char gapch;
    float fplural;
    float fplurality = 51.0;
    float ident;
    AjBool isident;
    AjBool issim;
    AjBool isgap;
    const AjPSeq* seqs;
    ajint numres;		 /* number of residues (not spaces) */
    void *freeptr;
    
    if(mymatrix)
	imatrix = mymatrix;

    if(!imatrix)
    {
	if(ajSeqsetIsNuc(thys))
	    ajStrAssignC(&matname, "EDNAFULL");
	else
	    ajStrAssignC(&matname, "EBLOSUM62");
	imatrix = ajMatrixNewFile(matname);
    }

    ajStrDel(&matname);

    *retident = 0;
    *retsim   = 0;
    *retgap   = 0;
    
    nseqs   = thys->Size;
    mlen    = thys->Len;
    fplural = ajSeqsetGetTotweight(thys) * fplurality / (float)100.;
    ident   = ajSeqsetGetTotweight(thys);
    
    /* ajDebug("fplural:%.2f ident:%.1f mlen: %d\n",
	    fplural, ident, mlen); */
    
    matrix  = ajMatrixGetMatrix(imatrix);
    cvt     = ajMatrixGetCvt(imatrix);	/* return conversion table */
    matsize = ajMatrixGetSize(imatrix);
    
    AJCNEW(seqs,nseqs);
    AJCNEW(seqcharptr,nseqs);
    AJCNEW(identical,matsize);
    AJCNEW(matching,matsize);
    
    posScore = ajFloatNew();
    
    gapch = '-';
    nocon = 'x';
    
    for(iseq=0;iseq<nseqs;iseq++)	/* get sequence as string */
    {
	seqcharptr[iseq] =  ajSeqsetGetseqSeqC(thys, iseq);
	seqs[iseq]       =  ajSeqsetGetseqSeq(thys, iseq);
    }
    
    /* For each position in the alignment, calculate consensus character */
    
    for(kkpos=0; kkpos< mlen; kkpos++)
    {
	res = gapch;
	
	isident = ajFalse;
	issim   = ajFalse;
	isgap   = ajFalse;
	
	/*
	 ** reset identities and +ve matches
	 */
	
	for(imat=0;imat<matsize;imat++)
	{
	    identical[imat] = 0.0; /* weights of all sequence chars in col. */
	    matching[imat] = 0.0;
	}
	
	/*
	 ** reset the posScore array
	 */
	
	for(iseq=0;iseq<nseqs;iseq++)
	    ajFloatPut(&posScore,iseq,0.);
	
	/*
	 ** generate scores (identical, posScore) for columns
	 */
	
	for(iseq=0;iseq<nseqs;iseq++)
	{
	    kipos = kkpos;
	    m1 = ajSeqcvtGetCodeK(cvt,seqcharptr[iseq][kipos]);

	    if(m1)
		identical[m1] += seqs[iseq]->Weight;
	    
	    for(jseq=iseq+1;jseq<nseqs;jseq++)
	    {
		kjpos = kkpos;
		m2 = ajSeqcvtGetCodeK(cvt,seqcharptr[jseq][kjpos]);

		if(m1 && m2)
		{
		    if(matrix)
		    {
			contri = (float)matrix[m1][m2]*seqs[jseq]->Weight
			  +ajFloatGet(posScore,iseq);
			contrj = (float)matrix[m1][m2]*seqs[iseq]->Weight
			  +ajFloatGet(posScore,jseq);
		    }
		    else
		    {
			contri = fmatrix[m1][m2]*seqs[jseq]->Weight
			  +ajFloatGet(posScore,iseq);
			contrj = fmatrix[m1][m2]*seqs[iseq]->Weight
			  +ajFloatGet(posScore,jseq);
		    }

		    ajFloatPut(&posScore,iseq,contri);
		    ajFloatPut(&posScore,jseq,contrj);
		}
	    }
	}

	/*
	 ** highindex is the highest scoring position (seq no.) in posScore
	 ** for 2 sequences this appears to be usually 0
	 */
	
	highindex = -1;
	max       = -FLT_MAX;
	numres    = 0;

	for(iseq=0;iseq<nseqs;iseq++)
	{
	    kipos = kkpos;

            if(seqcharptr[iseq][kipos] != ' ' &&
	       seqcharptr[iseq][kipos] != '-')
		numres++;
	    
	    if(ajFloatGet(posScore,iseq) > max)
	    {
		highindex = iseq;
		max       = ajFloatGet(posScore,iseq);
	    }
	}
	
	/* highindex is now set */
	
	/*
	 ** find +ve matches in the column
	 ** m1 is non-zero for a valid character in iseq
	 ** m2 is non-zero for a valid character in jseq
	 */
	
	for(iseq=0;iseq<nseqs;iseq++)
	{
	    kipos = kkpos;
	    m1 = ajSeqcvtGetCodeK(cvt, seqcharptr[iseq][kipos]);

	    if(E_FPZERO(matching[m1],U_FEPS))
	    {   /* first time we have met this character */
		for(jseq=0;jseq<nseqs;jseq++) /* all (other) sequences */
		{
		    kjpos = kkpos;
		    m2    = ajSeqcvtGetCodeK(cvt, seqcharptr[jseq][kjpos]);

		    if(matrix)
		    {
			if(m1 && m2 && matrix[m1][m2] > 0) 
			{		/* 'matching' if positive */
			    matching[m1] += seqs[jseq]->Weight;
			}
		    }
		    else
		    {
			if(m1 && m2 && fmatrix[m1][m2] > 0.0)
			{
			    matching[m1] += seqs[jseq]->Weight;
			}
		    }

		}
	    }
	}
	
	matchingmaxindex  = 0;	  /* get max matching and identical */
	identicalmaxindex = 0;

	for(iseq=0;iseq<nseqs;iseq++)
	{
	    kipos = kkpos;
	    m1 = ajSeqcvtGetCodeK(cvt,seqcharptr[iseq][kipos]);

	    if(identical[m1] > identical[identicalmaxindex])
		identicalmaxindex= m1;
	}
	
	for(iseq=0;iseq<nseqs;iseq++)
	{
	    kipos = kkpos;
	    m1 = ajSeqcvtGetCodeK(cvt,seqcharptr[iseq][kipos]);

	    if(matching[m1] > matching[matchingmaxindex])
	    {
		matchingmaxindex= m1;
	    }
	    else if(E_FPEQ(matching[m1],matching[matchingmaxindex],U_FEPS))
	    {
		if(identical[m1] > identical[matchingmaxindex])
		    matchingmaxindex= m1;
	    }

	    if(seqcharptr[iseq][kipos] == '-' ||
	       seqcharptr[iseq][kipos] == ' ')
		isgap=ajTrue;
	}

	khpos = kkpos;
	himatch = matching[ajSeqcvtGetCodeK(cvt,seqcharptr[highindex][khpos])];
	
	if(identical[identicalmaxindex] >= ident)
            isident=ajTrue;
        
	if(matching[matchingmaxindex] >= fplural)
            issim=ajTrue;
	
	/* plurality check */
	res = gapch;
	if(himatch >= fplural)
	    if(seqcharptr[highindex][khpos] != '-')
		res = toupper((int)seqcharptr[highindex][khpos]);

	if(nseqs > 1 && E_FPEQ(himatch,seqs[highindex]->Weight,U_FEPS))
	{
	    if(numres > 1)
		res = nocon;
	    else
		res = gapch;
	}
	
	if(issim && ! isident)
	    res = tolower((int)res);
	
	ajStrAppendK(cons,res);
	if(isident) ++*retident;
	if(issim) ++*retsim;
	if(isgap) ++*retgap;
	
	/* ajDebug("id:%b sim:%b gap:%b res:%c '",
	           isident, issim, isgap, res); */
	for(iseq=0; iseq<nseqs; iseq++)
	{
	    kipos = kkpos;
	    /* ajDebug("%c", seqcharptr[iseq][kipos]); */
	}
	/* ajDebug("'\n");	 */
    }
    
    *retlen = ajSeqsetGetLen(thys);
    
    /* ajDebug("ret ident:%d sim:%d gap:%d len:%d\n",
	    *retident, *retsim, *retgap, *retlen); */


    freeptr = (void *) seqs;
    AJFREE(freeptr);

    freeptr = (void *) seqcharptr;
    AJFREE(freeptr);

    AJFREE(matching);
    AJFREE(identical);
    ajFloatDel(&posScore);
    ajMatrixDel(&imatrix);

    return ajTrue;    
}




/* @funcstatic alignCigar *****************************************************
**
** Return CIGAR string for a pairwise alignment, as well as match region
** of the query sequence and the number of mismatches
**
** @param [r] qryseq [const AjPSeq] query sequence
** @param [r] refseq [const AjPSeq] reference sequence
** @param [r] qstart [ajint] query sequence start point
** @param [r] qend [ajint] query sequence end point
** @param [r] rstart [ajint] reference sequence start point
** @param [w] qseq [AjPStr*] query sequence
** @param [w] cigar [AjPStr*] CIGAR string
** @param [r] seqexternal [AjBool] Sequences are external references,
**              ignore their offsets and use qstart and qend
** @return [ajint] number of mismatches
******************************************************************************/

static ajint alignCigar(const AjPSeq qryseq, const AjPSeq refseq,
	                ajint qstart, ajint qend,
	                ajint rstart,
	                AjPStr *qseq, AjPStr* cigar, AjBool seqexternal)
{
    const char* qryseqc;
    const char* refseqc;

    static char gapchars[] = "-~.? "; /* all known gap characters */

    ajint m=0;
    ajint i=0; /* insertions to the reference sequence */
    ajint d=0; /* deletions from the reference sequence */
    ajuint qoffset=0;
    ajuint qoffend=0;
    ajint nmismatches=0;
    ajint qseqlen;

    qseqlen = ajSeqGetLen(qryseq);
    qryseqc = ajSeqGetSeqC(qryseq);
    refseqc = ajSeqGetSeqC(refseq);

    if(seqexternal)
    {
	qoffset = qstart;
	qoffend = qend;
    }
    else
    {
	qoffset = ajSeqGetOffset(qryseq);
	qoffend = ajSeqGetOffend(qryseq);
    }

    if(ajSeqIsReversed(qryseq)&&qoffend>0)
	ajFmtPrintAppS(cigar, "%dH",qoffend);
    else
	if(qoffset>0)
	    ajFmtPrintAppS(cigar, "%dH",qoffset);


    for(; qstart<qend; qstart++, rstart++)
    {
	if(strchr(gapchars,qryseqc[qstart]))
	{
	    d++;

	    if(m>0)
	    {
		ajFmtPrintAppS(cigar, "%uM",m);
		m=0;
	    }

	    if(i>0)
	    {
		ajFmtPrintAppS(cigar, "%uI",d);
		i=0;
	    }
	}
	else if(strchr(gapchars,refseqc[rstart]))
	{
	    i++;

	    if(m>0)
	    {
		ajFmtPrintAppS(cigar, "%uM",m);
		m=0;
	    }

	    if(d>0)
	    {
		ajFmtPrintAppS(cigar, "%uD",i);
		d=0;
	    }

	    ajStrAppendK(qseq,qryseqc[qstart]);

	}
	else /* match */
	{
	    m++;
	    ajStrAppendK(qseq,qryseqc[qstart]);

	    if(toupper(qryseqc[qstart])!=toupper(refseqc[rstart]))
		nmismatches ++;

	    if(d>0)
	    {
		ajFmtPrintAppS(cigar, "%uD",d);
		d=0;

	    }

	    if(i>0)
	    {
		ajFmtPrintAppS(cigar, "%uI",i);
		i=0;
	    }
	}
    }

    if(m>0)
	ajFmtPrintAppS(cigar, "%uM",m);
    else if(d>0)
	ajFmtPrintAppS(cigar, "%uD",d);
    else if(i>0)
	ajFmtPrintAppS(cigar, "%uI",i);

    if(ajSeqIsReversed(qryseq)&&qend>qseqlen)
	ajFmtPrintAppS(cigar, "%dH",qseqlen-qend);
    else if(qend<qseqlen)
	    ajFmtPrintAppS(cigar, "%dH",qseqlen-qend);

    return nmismatches;
}




/* @func ajAlignExit **********************************************************
**
** Cleans up report processing internal memory
**
** @return [void]
** @@
******************************************************************************/

void ajAlignExit(void)
{
    return;
}

/** @source infoassembly
**
** Get information from assembly files of sequence reads
**
** @version $Revision: 1.11 $
** @modified Dec 2011 uludag, first version
** @modified $Date: 2012/07/14 14:52:41 $ by $Author: rice $
** @@
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
********************************************************************/


/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "emboss.h"
#include "ajcigar.h"

#include <math.h>

/* Inclusion of system and local header files goes here */



/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */

/* #define and enum statements go here */



/* ==================================================================== */
/* ======================== global variables ========================== */
/* ==================================================================== */

/* Global variables definitions go here */




/*
 * Histograms to track the number of windows at each GC,
 * and the number of read starts at windows of each GC
 */

static int windowsByGc[101];
static int readsByGc[101];
static long basesByGc[101];
static long errorsByGc[101];


/* ==================================================================== */
/* ============================== data ================================ */
/* ==================================================================== */

/** Keeps track of current GC calculation state. */
typedef struct CalculateGcState
{
    AjBool init;
    int nCount ;
    int gcCount ;
    char priorBase ;
} CalculateGcState;




/* @datastatic ContigStatsP ***************************************************
**
**  summary information for a contig
**
** @alias ContigStatsS
** @alias ContigStatsO
**
** @attr rdepth       [unsigned char*] read depth for each contig sequence
**                                     position (1-based)
** @attr avdepth      [double] average coverage for the contig sequence
** @attr nReads       [int] Number of reads
** @attr mxdepth      [int]  maximum read depth
** @attr sumqualvals  [int] Total quality values
** @attr sumreadbases [int] Total bases read
** @attr astart       [int] beginning of the contig region covered by the reads
** @attr aend         [int] end of the contig region covered by the reads
******************************************************************************/

typedef struct ContigStatsS
{
    unsigned char* rdepth;
    double avdepth;
    int nReads;
    int mxdepth;
    int sumqualvals;
    int sumreadbases;
    int astart;
    int aend;
} ContigStatsO;

#define ContigStatsP ContigStatsO*




/* @datastatic AjPAssemStats **************************************************
**
** Store statistical information about assemblies
**
** @attr contigstats  [ContigStatsP*] Contig statistics object
** @attr qCounts     [ajlong[128]] quality scores
** @attr oqCounts    [ajlong[128]] original quality scores,
** 			     stored using OG tag in sam format
** @attr nContigs [ajuint] Number of contigs
** @attr nReads [ajuint] Number of reads
** @attr nFirstofPair [ajuint] Number of forward paired reads
** @attr nSecondofPair [ajuint] Number of reverse paired reads
** @attr nPair [ajuint] Number of paired reads
** @attr nUnpaired [ajuint] Number of unpaired reads
**
** @@
******************************************************************************/

typedef struct AjSAssemStats
{
    ContigStatsP* contigstats;
    ajlong qCounts[128];
    ajlong oqCounts[128];
    ajuint nContigs;
    ajuint nReads;
    ajuint nFirstofPair;
    ajuint nSecondofPair;
    ajuint nPair;
    ajuint nUnpaired;
} AjOAssemStats;

#define AjPAssemStats AjOAssemStats*


/* ==================================================================== */
/* ==================== function prototypes =========================== */
/* ==================================================================== */


static int getPhredScoreFromErrorProbability(double probability);
static int getPhredScoreFromObsAndErrors(double observations, double errors);

static AjBool infoassembly_bisulfiteBasesEqual(AjBool negativeStrand,
                                               char readbase,
                                               char reference);

static char* infoassembly_calculateAllGcs(const char* refBases,
                                          ajint refLength,
                                          ajint windowsize,
                                          ajint lastWindowStart);

static int infoassembly_calculateGc(const char* bases,
                                    int startIndex,
                                    int endIndex,
                                    CalculateGcState state);

static int infoassembly_countMismatches(AjPAssemRead read,
                                        const char* referenceBases,
                                        int referenceOffset,
                                        AjBool bisulfiteSequence);

static void infoassembly_processRead(AjPAssemStats stats,
                                     ContigStatsP cstats,
                                     ajint windowsize,
                                     AjPAssemContig c,
                                     char* gc,
                                     AjPAssemRead r,
                                     char* qmin);

static ajint infoassembly_updatereaddepth(const AjPCigar c,
                                          unsigned char* rdepth);

static void printQualDist(AjPAssemStats stats, AjPFile qdist, ajint poffset);
static void printGcBias(AjPFile gcbias);
static void printContigStats(AjPAssemContig* contigs,
			     AjPAssemStats avqualvals,
			     AjPFile outf,
			     ajint poffset);

static int getPhredEncodingOffset(char qmin, const char**name);




/* @prog infoassembly *********************************************************
**
** Get information from assembly files of sequence reads
**
** GC bias calculations are based on CollectGcBiasMetrics class
** in Picard project (picard.sf.net).
**
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPOutfile outfile = NULL;    
    AjPFile gcbias = NULL;
    AjPFile qdist  = NULL;

    AjPAssemload assemload = NULL;
    AjPAssem assembly = NULL;
    AjPSeqset refseqs = NULL;
    AjPAssemContig c  = NULL;
    AjPAssemStats stats = NULL;
    AjPAssemRead r = NULL;

    ContigStatsP cstats = NULL;

    const AjPStr refBases = NULL;
    const char* refBasesc = NULL;

    const AjPSeq refSeq;

    char** gc = NULL;

    ajuint i = 0;
    double totcov = 0;
    double totlengthunpaired=0;
    double totlengthfirstofpair=0;
    double totlengthsecondofpair=0;
    ajushort mxdepth = 0;
    ajint basescovered = 0;
    ajint windowsize   = 0; /* used in GC bias calculation */

    int refLength = 0;
    int lastWindowStart = 0;

    unsigned char* rdepth = NULL;
    unsigned char* rdepthend = NULL;

    char qmin = 0x7F;
    const char* phredencoding = NULL;
    int poffset = 0; /* phred encoding offset */

    /* ACD processing */
    embInit("infoassembly", argc, argv);

    assemload = ajAcdGetAssembly("assembly");
    outfile  = ajAcdGetOutassembly("outassembly");
    refseqs  = ajAcdGetSeqset("refsequence");
    gcbias   = ajAcdGetOutfile("gcbiasmetricsoutfile");
    qdist    = ajAcdGetOutfile("qualvaluesdistoutfile");
    windowsize = ajAcdGetInt("windowsize");

    if(!ajAssemloadMore(assemload, &assembly))
        ajDie("Failed to read assembly data");

    if(gcbias && !refseqs)
	ajDie("gcbias option selected without specifying a reference"
		"sequence file");

    AJNEW0(stats);
    stats->nContigs = (ajuint) ajTableGetLength(assembly->Contigs);
    AJCNEW0(stats->contigstats, stats->nContigs);

    if(refseqs)
	AJCNEW0(gc,  stats->nContigs);

    /* todo: replace seqset with seqall for better scalability ???
     *       moving to the next contig when all reads in a contig consumed?
     *       we will need to assume sorted assembly and reference sequences
     */

    /* todo: calculate N50? but then we need to know the genome size? */

    if(refseqs)
	ajSeqsetFmtUpper(refseqs);

    /* iterate over contigs for gcbias calculation + memory allocation */

    for(i=0; i < stats->nContigs; i++)
    {
	c = assembly->ContigArray[i];
	AJNEW0(stats->contigstats[i]);
	AJCNEW0(stats->contigstats[i]->rdepth, c->Length+1); /* 1-based coord*/

	stats->contigstats[i]->astart = c->Length+1;
	stats->contigstats[i]->aend = 1;

	if(refseqs)
	{
	    refSeq = ajSeqsetGetseqSeq(refseqs, i);
	    refBases = ajSeqGetSeqS(refSeq);
	    refLength = ajStrGetLen(refBases);
	    lastWindowStart = refLength - windowsize;

	    refBasesc = ajStrGetPtr(refBases);

	    gc[i] = infoassembly_calculateAllGcs(refBasesc, refLength,
                                                 windowsize,
                                                 lastWindowStart);
	    ajStrAssignRef(&c->Consensus, refSeq->Seq);
	}
    }

    /*
    **  main loop for reading individual assembly records (reads/alignments)
    **/

    while(ajAssemloadMore(assemload, &assembly))
    {
	r = assembly->rec;

	if(!r)
	    break;

	stats->nReads++;

	if(ajAssemreadGetFlagPaired(r))
	{
	    stats->nPair++;

	    if(ajAssemreadGetFlagFirstofpair(r))
	    {
		totlengthfirstofpair += ajStrGetLen(r->Seq);
		stats->nFirstofPair++;
	    }
	    else /* double check whether we should explicitly check this*/
	    {
		totlengthsecondofpair += ajStrGetLen(r->Seq);
		stats->nSecondofPair++;
	    }
	}
	else
	{
	    stats->nUnpaired++;
	    totlengthunpaired += ajStrGetLen(r->Seq);
	}


	if(r->Reference == -1)
	{
	    /* todo: what else we can do when contig is unknown ??? */
	    continue;
	}

	i = (ajuint) r->Reference;

	if(i >= (ajuint) ajListGetLength(assembly->ContigsOrder))
	{
	    ajDie("infoassembly: reference sequence number"
		    " '%d' is larger than or equal to known number of reference"
		    " sequences '%Lu'. Problem while processing read '%S'.",
		    i,
		    ajListGetLength(assembly->ContigsOrder),
		    assembly->rec->Name);
	    continue;
	}

	c= assembly->ContigArray[i];
	cstats = stats->contigstats[i];
	cstats->nReads++;

	if(refseqs || gcbias || qdist)
	    infoassembly_processRead(stats,
                                     cstats,
                                     windowsize,
                                     c,
                                     (gc ? gc[i] : NULL),
                                     r,
                                     &qmin);

	assembly->rec=NULL;
    };

    if(qmin != 0x7f)
    {
	poffset = getPhredEncodingOffset(qmin, &phredencoding);

	ajFmtPrintF(outfile->File,"Phred encoding: %s\n", phredencoding);
    }


    ajFmtPrintF(outfile->File,"\nRead distribution:"
	    "\n------------------\n\n");

    ajFmtPrintF(outfile->File, "READ_CATEGORY\t#READS\tAv_Read_Length\n");

    ajFmtPrintF(outfile->File, "all reads\t%u", stats->nReads);
    if(stats->nReads)
	ajFmtPrintF(outfile->File, "\t%.2f",
	            (totlengthfirstofpair+totlengthsecondofpair
	        	    +totlengthunpaired) / stats->nReads);

    ajFmtPrintF(outfile->File, "\nfirst of pair \t%u",
	    stats->nFirstofPair);
    if(stats->nFirstofPair)
	ajFmtPrintF(outfile->File, "\t%.2f",
	            totlengthfirstofpair/stats->nFirstofPair);

    ajFmtPrintF(outfile->File, "\nsecond of pair\t%u",
	    stats->nSecondofPair);
    if(stats->nSecondofPair)
	ajFmtPrintF(outfile->File, "\t%.2f",
	            totlengthsecondofpair/stats->nSecondofPair);

    ajFmtPrintF(outfile->File, "\npair          \t%u",
	    stats->nPair);
    if(stats->nPair)
	ajFmtPrintF(outfile->File, "\t%.2f",
	            (totlengthfirstofpair+totlengthsecondofpair)/stats->nPair);

    ajFmtPrintF(outfile->File, "\nunpaired      \t%u",
	    stats->nUnpaired);
    if(stats->nUnpaired)
	ajFmtPrintF(outfile->File, "\t%.2f",
	            totlengthunpaired/stats->nUnpaired);

    if(gcbias)
	printGcBias(gcbias);


    /* calculate contig stats, then print */

    for(i=0; i < stats->nContigs; i++)
    {

	c = assembly->ContigArray[i];
	cstats = stats->contigstats[i];

	if(cstats->nReads == 0)
	    continue;

	rdepth = cstats->rdepth+cstats->astart;
	rdepthend = cstats->rdepth+cstats->aend;

	for(; rdepth <rdepthend;)
	{
	    if(*rdepth) /* 1-based array/coord */
	    {
		totcov += *rdepth;
		basescovered++;
	    }

	    if(*rdepth++ > mxdepth)
		mxdepth = *rdepth;
	}

	if(basescovered)
	    cstats->avdepth = totcov/basescovered;

	totcov = 0;
	basescovered = 0;
	stats->contigstats[i]->mxdepth = mxdepth;
	mxdepth = 0;
    }

    ajFmtPrintF(outfile->File,"\n\nContig stats:"
			        "\n-------------\n\n");

    printContigStats(assembly->ContigArray, stats, outfile->File, poffset);

    if(qdist)
	printQualDist(stats, qdist, poffset);


    /* Memory clean-up and exit */

    if(refseqs)
    {
	for(i=0; i < stats->nContigs; i++)
	    AJFREE(gc[i]);

	AJFREE(gc);
    }

    if(stats)
    {
	for(i=0; i < stats->nContigs; i++)
	{
	    AJFREE(stats->contigstats[i]->rdepth);
	    AJFREE(stats->contigstats[i]);
	}

	AJFREE(stats->contigstats);
	AJFREE(stats);
    }

    ajAssemDel(&assembly);
    ajAssemloadDel(&assemload);
    ajSeqsetDel(&refseqs);

    ajOutfileClose(&outfile);
    ajFileClose(&gcbias);
    ajFileClose(&qdist);

    embExit();

    return 0;
}


/* ==================================================================== */
/* ============================ functions ============================= */
/* ==================================================================== */


/* simple function to sum numbers in a given integer array */

static double sum(int* a, int n)
{
    int i=0;
    double s=0;

    for (i=0; i<n; i++)
	s += a[i];

    return s;
}




/*
 * print MIRA style contig stats
 * contig, length, av.qual, #reads, mx cov, av cov, GC%
 *
 */

static void printContigStats(AjPAssemContig* contigs, AjPAssemStats stats,
			     AjPFile outf, ajint poffset)
{
    AjPAssemContig c=NULL;
    float gc = 0;
    ajint i=0;
    ajulong l=0;
    double avqual = 0;

    ajFmtPrintF(outf, "CONTIG\tLENGTH\tMean_QUAL\t#READS\tMax DEPTH\t"
	    "Mean_DEPTH\t%%GC\n");

    while (contigs[i])   /* contigs */
    {
	c = contigs[i];

	l = c->Length;

	if(c->Consensus && ajStrGetLen(c->Consensus)==l)
	    gc = ajMeltGC(c->Consensus, c->Length);

	if(stats && stats->contigstats[i]->sumreadbases)
	    avqual = stats->contigstats[i]->sumqualvals * 1.0 /
	    stats->contigstats[i]->sumreadbases;
	else
	    avqual = 0;

	ajFmtPrintF(outf, "%S\t%d\t%.2f\t%d\t%d\t%.2f\t%.2f\n",
		c->Name,
		c->Length,
		(E_FPZERO(avqual, U_FEPS) ? 0 : avqual-poffset),
		(stats ? (stats->contigstats[i]->nReads) : c->Nreads),
		(stats ? (stats->contigstats[i]->mxdepth) : 0),
		(stats ? (stats->contigstats[i]->avdepth) : 0),
		gc*100);
	i++;
    }

}




/*
 * print quality values distribution to the given output file
 *
 */

static void printQualDist(AjPAssemStats stats, AjPFile qdist, ajint poffset)
{
    ajint i=0;

    ajFmtPrintF(qdist, "#QUALITY: Phred quality score\n");
    ajFmtPrintF(qdist, "#BASES: Number of bases with the quality score\n\n");
    ajFmtPrintF(qdist, "QUALITY\t#BASES\n");

    for (i=0; i<128; i++)
    {
	if(stats->qCounts[i]>0)
	    ajFmtPrintF(qdist, "%d\t%d\n",
	                i-poffset,
	                stats->qCounts[i]);

    }
}




/*
 *  Get the phred score for any given probability of error.
 *
 */

static int getPhredScoreFromErrorProbability(double probability)
{
    int ret=0;
    double calc;

    calc = -10.0 * log10(probability);
    if(calc < 0)
      ret = (int) (calc - 0.5);
    else
      ret = (int) (calc + 0.5);

    return ret;
}




/*
 * Get the phred score given the specified observations and errors.
 *
 */

static int getPhredScoreFromObsAndErrors(double observations, double errors)
{
    return getPhredScoreFromErrorProbability(errors / observations);
}




/*
 * Print GC bias metrics values to the output file gcbias
 *
 */

static void printGcBias(AjPFile gcbias)
{
    int i=0;
    int meanBaseQuality =0;

    double totalWindows = 0;
    double totalReads   = 0;
    double meanReadsPerWindow = 0;

    totalWindows = sum(windowsByGc,100);
    totalReads   = sum(readsByGc,100);
    meanReadsPerWindow = totalReads / totalWindows;

    ajDebug("total windows=%f  reads=%f  meanreadsperwin=%f\n",
	    totalWindows, totalReads, meanReadsPerWindow);

    ajFmtPrintF(gcbias,
	    "# GC bias metrics as described in GcBiasDetailMetrics class\n"
	    "# in picard  project (picard.sf.net).\n"
	    "# The mean base quality are determined via the error rate\n"
	    "# of all bases of all reads that are assigned to windows of"
	    " a GC.\n");

    ajFmtPrintF(gcbias,
	    "GC\tWINDOWS\tREAD_STARTS\tMEAN_BASE_QUALITY\t"
	    "NORMALIZED_COVERAGE\n");

    for (i=0; i<=100; i++)
    {

	if (errorsByGc[i] > 0)
	    meanBaseQuality = getPhredScoreFromObsAndErrors(
		    basesByGc[i], errorsByGc[i]);
	else
	    meanBaseQuality = 0;

	if(windowsByGc[i]>0)
	    ajFmtPrintF(gcbias, "%d\t%d\t%d\t%d\t%g\n",
	                i,
	                windowsByGc[i],
	                readsByGc[i],
	                meanBaseQuality,/* mean base quality */
	                (readsByGc[i]*1.0f)/windowsByGc[i]/meanReadsPerWindow
	    );

    }

    return;
}




/**
 * Calculates GC as a number from 0 to 100 in the specified window.
 * If the window includes more than 4 no-calls then -1 is returned.
 *
 */

static int infoassembly_calculateGc(const char* bases,
                                    int startIndex,
                                    int endIndex,
                                    CalculateGcState state)
{
    int i=0;

    if (state.init)
    {
	state.init = ajFalse ;
	state.gcCount = 0;
	state.nCount  = 0;

	for (i=startIndex; i<endIndex; ++i)
	{
	    char base = bases[i];

	    if (base == 'G' || base == 'C')
		++state.gcCount;
	    else if (base == 'N')
		++state.nCount;
	}
    }
    else
    {
	char newBase = bases[endIndex-1];

	if (newBase == 'G' || newBase == 'C')
	    ++state.gcCount;
	else if (newBase == 'N')
	    ++state.nCount;

	if (state.priorBase == 'G' || state.priorBase == 'C')
	    --state.gcCount;
	else if (state.priorBase == 'N')
	    --state.nCount;
    }

    state.priorBase = bases[startIndex];

    if (state.nCount > 4)
	return -1;
    else
	return (state.gcCount * 100) / (endIndex - startIndex);
}




/* @funcstatic infoassembly_calculateAllGcs ***********************************
**
** Calculate GC values for all windows.
** Computes the number of windows (of size specified by windowsize)
** in the reference sequence at each GC%.
**
** @param [r] refBases [const char*] reference sequence
** @param [r] refLength [ajint] reference sequence length
** @param [r] windowsize [ajint] window size
** @param [r] lastWindowStart[ajint] last window start
**
** @return [char*] gc percentage for each position of the reference sequence
** @@
******************************************************************************/

static char* infoassembly_calculateAllGcs(const char* refBases,
                                          ajint refLength,
                                          ajint windowsize,
                                          ajint lastWindowStart)
{
    char* gc = NULL;
    int i=0;

    CalculateGcState* state;

    AJNEW0(state);
    state->init=ajTrue;

    AJCNEW0(gc, refLength + 1);

    /* picard  use 1-based index for gc calculations */
    for (i=1; i<lastWindowStart; ++i)
    {
	int windowEnd = i + windowsize;
	int windowGc = infoassembly_calculateGc(refBases, i,
                                                windowEnd, *state);

	gc[i] = (char) windowGc;

	if (windowGc != -1)
	    windowsByGc[windowGc]++;
    }

    AJFREE(state);

    return gc;
}




/* @funcstatic infoassembly_bisulfiteBasesEqual *******************************
**
** Check whether read and reference bases are equal, for bisulfite treated
** samples.
**
** @param [r] negativeStrand [AjBool] whether read aligns to the negative
** 				      strand
** @param [r] readbase  [char] read base
** @param [r] reference [char] reference base
**
** @return [AjBool] true if the bases are equal
** @@
******************************************************************************/

static AjBool infoassembly_bisulfiteBasesEqual(AjBool negativeStrand,
                                               char readbase,
                                               char reference)
{
    if(readbase == reference)
	return ajTrue;

    if (negativeStrand)
    {
	if( readbase == 'G' && reference == 'A')

	    return ajTrue;
    }
    else if (readbase == 'C' && reference == 'T')
	return ajTrue;

    return ajFalse;
}




/* @funcstatic infoassembly_countMismatches ***********************************
**
** Calculates the number of mismatches between the read and the reference
** sequence provided. (copy of a function from picard project, picard.sf.net)
**
** @param [u] readalgn [AjPAssemRead] read/alignment record
** @param [r] referenceBases [const char*] bases that covers at least the
** 					   portion of the reference sequence
** 					   to which read is aligned.
** @param [r] referenceOffset [int] 0-based offset of the first element of
** 				    referenceBases relative to the start
**				    of the reference sequence.
** @param [r] bisulfiteSequence [AjBool] If this is true, it is assumed that
**     					 the reads were bisulfite treated
**      and C->T on the positive strand and G->A on the negative strand
**      will not be counted as mismatches.
**
** @return [int] Number of mismatches
******************************************************************************/

static int infoassembly_countMismatches(AjPAssemRead readalgn,
                                        const char* referenceBases,
                                        int referenceOffset,
                                        AjBool bisulfiteSequence)
{
    int mismatches = 0;
    AjPList alblocks = NULL;
    const char* readBases = NULL;
    AjIList i = NULL;
    AjPAssemReadalignmentblock block = NULL;
    int j = 0;
    int readBlockStart;
    int referenceBlockStart;
    int length;

    alblocks = ajAssemreadSetAlignblocks(readalgn);
    readBases = ajStrGetPtr(readalgn->Seq);

    i = ajListIterNewread(alblocks);

    while(!ajListIterDone(i))
    {
	block = ajListIterGet(i);
	readBlockStart = block->readStart - 1;
	referenceBlockStart = block->referenceStart - 1 - referenceOffset;
	length = block->length;

	if(referenceBlockStart<0)
	    ajUser("minus referenceBlockStart: %d", referenceBlockStart);

	for (j=0; j<length; ++j)
	{
	    /* reference bases are already uppercase, updated in main() */
	    if (!bisulfiteSequence)
	    {
	      if (toupper((int)readBases[readBlockStart+j]) !=
			referenceBases[referenceBlockStart+j])
		    ++mismatches;
	    }
	    else
	    {
		if (!infoassembly_bisulfiteBasesEqual(
			ajAssemreadGetFlagNegativestrand(readalgn),
			toupper((int)readBases[readBlockStart+j]),
			referenceBases[referenceBlockStart+j]))
		    ++mismatches;
	    }
	}
	AJFREE(block);
    }

    ajListIterDel(&i);
    ajListFree(&alblocks);

    return mismatches;
}




#define SANGER_ENCODING_OFFSET 33
#define ILLUMINA_1_3_ENCODING_OFFSET 64
#define ILLUMINA_1_ENCODING_OFFSET 59

static int getPhredEncodingOffset(char qmin, const char**name)
{
    int ret = 0;

    if (qmin < 33)
    {
	ajErr("No known encodings with chars < 33. "
		"smallest score was %d.", qmin);
	return 0;
    }
    else if (qmin < 59)
    {
	*name = "Sanger / Illumina 1.9";
	ret = SANGER_ENCODING_OFFSET;
    }
    else if (qmin < 64)
    {
	*name = "Illumina <1.3";
	ret = ILLUMINA_1_ENCODING_OFFSET;
    }
    /* There are potentially two encodings using an offset of 64.  Illumina
    ** v1.3 allowed quality values of 1, whereas from v1.5 onwards the lowest
    ** value allowed was 2.  If we guess wrong between these two then it's not
    ** the end of the world since they use the same offset (fastqc project).
    */
    else if (qmin == ILLUMINA_1_3_ENCODING_OFFSET+1)
    {
	*name = "Illumina 1.3";
	ret = ILLUMINA_1_3_ENCODING_OFFSET;
    }
    else if (qmin <= 126)
    {
	*name = "Illumina 1.5";
	ret = ILLUMINA_1_3_ENCODING_OFFSET;
    }
    else
    {
	ajErr("No known encodings with chars > 126");
	return 0;
    }

    ajDebug("getPhredEncodingOffset: encoding=%s, offset=%d\n", *name, ret);

    return ret;
}




/* @funcstatic infoassembly_processRead ***************************************
 *
 * Collect information about GC bias in the reads in a given assembly file.
 * Computes the number of windows (of size specified by WINDOW_SIZE)
 * in the genome at each GC% and counts the number of read starts
 * in each GC bin.  What is output is
 * the "normalized coverage" in each bin
 * - i.e. the number of reads per window normalized to the average number
 * of reads per window across the whole genome.
 *
 * This code is based on Tim Fennell's CollectGcBiasMetrics class in Picard
 * project.
 *
 * return average qual values array for contigs
**
** @param [u] stats [AjPAssemStats] object to gather statistics
** @param [u] cstats [ContigStatsP] part of the above object
** @param [r] windowsize [ajint] Window size
** @param [u] c [AjPAssemContig] Contig object
** @param [u] gc [char*] GC bins
** @param [u] r [AjPAssemRead] Read data object
** @param [u] qmin [char*] Quality minima
**
** @return [void]
** @@
******************************************************************************/

static void infoassembly_processRead(AjPAssemStats stats,
                                     ContigStatsP cstats,
                                     ajint windowsize,
                                     AjPAssemContig c,
                                     char* gc,
                                     AjPAssemRead r,
                                     char* qmin)
{
    AjPCigar cigar = NULL;
    const char *seq = NULL; /* pointer to the read-sequence,
			       used when iterating over qual values */
    const char* qual = NULL;

    int iqual=0;

    int totalAlignedReads=0;
    ajint l =0;
    ajint qmax = 126; /* No known encodings with chars > 126 */
    int sumqualvals=0;
    int pos = 0;


    if (ajAssemreadGetFlagUnmapped(r))
	return;

    if(ajAssemreadGetFlagNegativestrand(r))
	pos = ajAssemreadGetAlignmentend(r) - windowsize - 1;
    else
	pos= ajAssemreadGetAlignmentstart(r);

    ajDebug("%S  pos=%d  cigar=%S\n", r->Name, pos, r->Cigar);

    ++totalAlignedReads;

    if(gc && pos>0)
    {/* picard use 1-based index */
	int windowGc = gc[pos];

	if(windowGc >= 0)
	{
	    ++readsByGc[windowGc];
	    basesByGc[windowGc] += ajStrGetLen(r->Seq);

	    cigar = ajCigarNewS(r->Cigar);

	    errorsByGc[windowGc] +=
		    infoassembly_countMismatches(r, ajStrGetPtr(c->Consensus),
		                    0, ajFalse)
		                    +
		                    ajCigarCountInsertedBases(cigar)
		                    +
		                    ajCigarCountDeletedBases(cigar);
	}

    }

    /* recalculate the start pos, above calculation includes windowsize ??? */
    if(ajAssemreadGetFlagNegativestrand(r))
	pos= ajAssemreadGetAlignmentstart(r);


    if(pos > 0)
    {
	if(!cigar)
	    cigar = ajCigarNewS(r->Cigar);

	l = infoassembly_updatereaddepth(cigar, cstats->rdepth+pos);

	if(pos < cstats->astart)
	    cstats->astart = pos;

	if(pos+l > cstats->aend)
	    cstats->aend = pos+l;

    }
    else
	ajWarn("infoassembly_processRead:check me please: read:%S pos=%d",
	       r->Name,
	       pos);


    /* todo: put the following lines into a new function */
    qual= ajStrGetPtr(r->SeqQ);
    if(*qual!='*')
    {

	seq = ajStrGetPtr(r->Seq);

	while (*seq)
	{
	    /* todo: should we go over the aligned region only,
	     *       like for the read-depth calculation above ??
	     *       no???
	     *       picard gets it for all bases/scores ?
	     *
	     *       how does fastqc??? all bases as well?
	     */
	    if(*seq != 'N' && *seq != 'n' && *seq != '.')
	    {

		iqual = *qual;

		if(iqual < SANGER_ENCODING_OFFSET)
		{
		    ajWarn("sequence '%S' "
			    "quality value %d '%c' too low",
			    r->Name,
			    iqual,
			    (char) iqual);
		    iqual = SANGER_ENCODING_OFFSET;
		}
		else if(iqual > qmax)
		{
		    ajWarn("sequence '%S' "
			    "quality value %d '%c' too high",
			    r->Name,
			    iqual,
			    (char) iqual);
		    iqual = qmax;
		}

		if(iqual<*qmin)
		    *qmin = iqual;

		stats->qCounts[iqual]++;
		sumqualvals += iqual;

		/* TODO: ???
		 * assem->Stats->oqCounts[oqual]++;
		 * get original qual values using OG tag if available
		 */

	    }
	    seq++;
	    qual++;
	}
    }


    ajCigarDel(&cigar);


    cstats->sumreadbases += ajStrGetLen(r->Seq);
    cstats->sumqualvals += sumqualvals;
    /* todo: bases covered by an alignment only?? */

    return;
}




/* @funcstatic infoassembly_updatereaddepth ***********************************
**
** Updates a contig read depth array from a CIGAR string
**
** @param [r] c [const AjPCigar] cigar object
** @param [u] rdepth [unsigned char*] array for read depth
**
** @return [ajint] length of the aligned/covered reference sequence
** @@
******************************************************************************/

static ajint infoassembly_updatereaddepth(const AjPCigar c,
                                          unsigned char* rdepth)
{
    ajint i=0;
    ajint length = 0;
    int j=0;

    for (i=0;i<c->n;i++)
    {
	switch (c->cigoperator[i])
	{
	    case 'M':
	    case 'D':
	    case 'N':
	    case '=':
	    case 'X':
		length += c->length[i];
		for(j=0; j<c->length[i]; j++)
		{
		    if(*rdepth != 0xff)
			(*rdepth++)++;
		    else
			rdepth++;
		}
	}
    }
    return length;
}

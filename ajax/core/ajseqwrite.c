/* @source ajseqwrite *********************************************************
**
** AJAX seqwrite  functions
**
** @author Copyright (C) 2001 Peter Rice
** @version $Revision: 1.162 $
** @modified 2001-2011 Peter Rice
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
******************************************************************************/

#include "ajlib.h"

#include "ajseqwrite.h"
#include "ajseq.h"
#include "ajseqtype.h"
#include "ajseqbam.h"
#include "ajfeat.h"
#include "ajfeatwrite.h"

#include "ajmath.h"
#include "ajreg.h"
#include "ajfileio.h"
#include "ajnam.h"
#include "ajutil.h"

#include <math.h>
#include <float.h>

#ifdef WIN32
#define fileno _fileno
#endif /* WIN32 */

static AjPRegexp seqoutRegFmt = NULL;
static AjPRegexp seqoutRegId  = NULL;

static AjPStr seqoutUsaTest = NULL;

static ajuint seqQualPhredToIndex[] = { 0, /* 0 */
   1,  2,  5,  7,  9, 10, 11, 13, 14, 15, /* 1-10 */
  16, 17, 18, 19, 20, 21, 22, 23, 24, 25, /* 11-20 */
  26, 27, 28, 29, 30, 31, 32, 33, 34, 35, /* 21-30 */
  36, 37, 38, 39, 40, 41, 42, 43, 44, 45, /* 31-40 */
  46, 47, 48, 49, 50, 51, 52, 53, 54, 55, /* 41-50 */
  56, 57, 58, 59, 60, 61, 62, 63, 64, 65, /* 51-60 */
  66, 67, 68                              /* 61-63 */
};

static double seqQualIndexToSolexa[] = {       -FLT_MAX, /* 0 -5 */
  1.318795,  1.603736,  1.937759,  2.324741,  2.767492, /* 1-5 -4-0 */
  3.267492,  3.824741,  4.437759,  5.103736,  5.818795, /* 76-10 1-5 */
  6.578332,  7.377360,  8.210819,  9.073822,  9.961836, /* 11-15 6-10 */
 10.870778, 11.797062, 12.737602, 13.689784, 14.651423, /* 16-20 11-14 */
 15.620708, 16.596154, 17.576551, 18.560916, 19.548457, /* 21-25 16-19 */
 20.538535, 21.530637, 22.524354, 23.519356, 24.515382, /* 26-30 21-24 */
 25.512223, 26.509712, 27.507716, 28.506130, 29.504870, /* 31-35 26-29 */
 30.503869, 31.503073, 32.502442, 33.501939, 34.501541, /* 36-40 31-34 */
 35.501224, 36.500972, 37.500772, 38.500613, 39.500487, /* 41-45 36-39 */
 40.500387, 41.500307, 42.500244, 43.500194, 44.500154, /* 46-50 41-44 */
 45.500122, 46.500097, 47.500077, 48.500061, 49.500049, /* 51-55 46-49 */
 50.500039, 51.500031, 52.500024, 53.500019, 54.500015, /* 56-60 51-54 */
 55.500012, 56.500010, 57.500008, 58.500006, 59.500005, /* 61-65 56-59 */
 60.500004, 61.500003, FLT_MAX, 999.999                 /* 66-67 61-62 */
                };




/* @filesection ajseqwrite ****************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
******************************************************************************/




/* @datastatic SeqPOutFormat **************************************************
**
** Sequence output formats
**
** @attr Name [const char*] Format name
** @attr Desc [const char*] Format description
** @attr Alias [AjBool] Name is an alias for an identical definition
** @attr Single [AjBool] Write each sequence to a new file if true (e.g. GCG)
** @attr Save [AjBool] Save in memory and write at end (e.g. MSF alignments)
** @attr Nucleotide [AjBool] True if nucleotide data is supported
** @attr Protein [AjBool] True if protein data is supported
** @attr Feature [AjBool] True if feature data can be written
** @attr Gap [AjBool] True if gap characters are supported
** @attr Multiset [AjBool] True if sets of sets (seqsetall) are supported
** @attr Write [void function] Function to write the format
** @@
******************************************************************************/

typedef struct SeqSOutFormat
{
    const char *Name;
    const char *Desc;
    AjBool Alias;
    AjBool Single;
    AjBool Save;
    AjBool Nucleotide;
    AjBool Protein;
    AjBool Feature;
    AjBool Gap;
    AjBool Multiset;
    void (*Write) (AjPSeqout outseq);
} SeqOOutFormat;

#define SeqPOutFormat SeqOOutFormat*




/* @datastatic SeqPSeqFormat **************************************************
**
** Data structure to hold definitions when writing sequence data.
**
** Most output functions generate the sequence header, then build
** this data structure for the actual output using function seqWriteSeq
**
** @attr linepos [ajuint] Undocumented
** @attr namewidth [ajuint] Name format width
** @attr numline [ajuint] Undocumented
** @attr numwidth [ajuint] Number format width
** @attr spacer [ajint] Spacer - can be negative
** @attr tab [ajuint] Undocumented
** @attr width [ajuint] Number of bases per line
** @attr baseonlynum [AjBool] Undocumented
** @attr degap [AjBool] Remove gap characters
** @attr domatch [AjBool] Show matching line
** @attr isactive [AjBool] Undocumented
** @attr nameright [AjBool] Sequence name in right margin
** @attr nameleft [AjBool] Sequence name in left margin
** @attr noleaves [AjBool] Undocumented
** @attr numjust [AjBool] Justify numbers
** @attr numleft [AjBool] Base number on left
** @attr numright [AjBool] Base number on right
** @attr pretty [AjBool] Undocumented
** @attr skipafter [AjBool] Undocumented
** @attr skipbefore [AjBool] Undocumented
** @attr gapchar [char] gap character
** @attr matchchar [char] matching character
** @attr endstr [char[20]] Last line(s)
** @attr leftstr [char[20]] string in left margin
** @attr Padding [char[2]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct SeqSSeqFormat
{
    ajuint linepos;
    ajuint namewidth;
    ajuint numline;
    ajuint numwidth;
    ajint spacer;
    ajuint tab;
    ajuint width;
    AjBool baseonlynum;
    AjBool degap;
    AjBool domatch;
    AjBool isactive;
    AjBool nameright;
    AjBool nameleft;
    AjBool noleaves;
    AjBool numjust;
    AjBool numleft;
    AjBool numright;
    AjBool pretty;
    AjBool skipafter;
    AjBool skipbefore;
    char gapchar;
    char matchchar;
    char endstr[20];
    char leftstr[20];
    char Padding[2];
} SeqOSeqFormat;

#define SeqPSeqFormat SeqOSeqFormat*




static ajint seqSpaceAll = -9;




static void       seqClone(AjPSeqout outseq, const AjPSeq seq);
static void       seqDbName(AjPStr* name, const AjPStr db);
static void       seqDeclone(AjPSeqout outseq);
static AjBool     seqFileReopen(AjPSeqout outseq);
static void       seqFormatDel(SeqPSeqFormat* pformat);
static AjBool     seqNcbiKnowndb(const AjPStr dbname);
static AjBool     seqoutUfoLocal(const AjPSeqout thys);
static AjBool     seqoutUsaProcess(AjPSeqout thys);
static void       seqsetClone(AjPSeqout outseq, const AjPSeqset seq, ajint i);
static AjBool     seqoutFindOutFormat(const AjPStr format, ajint* iformat);

static void       seqCleanBam(AjPSeqout outseq);
static void       seqCleanDasdna(AjPSeqout outseq);
static void       seqCleanDasseq(AjPSeqout outseq);
static void       seqSeqFormat(ajint seqlen, SeqPSeqFormat* psf);
static void       seqWriteAcedb(AjPSeqout outseq);
static void       seqWriteAsn1(AjPSeqout outseq);
static void       seqWriteBam(AjPSeqout outseq);
static void       seqWriteClustal(AjPSeqout outseq);
static void       seqWriteCodata(AjPSeqout outseq);
static void       seqWriteDasdna(AjPSeqout outseq);
static void       seqWriteDasseq(AjPSeqout outseq);
static void       seqWriteDebug(AjPSeqout outseq);
static void       seqWriteEmbl(AjPSeqout outseq);
static void       seqWriteEmblnew(AjPSeqout outseq);
static void       seqWriteExperiment(AjPSeqout outseq);
static void       seqWriteFasta(AjPSeqout outseq);
static void       seqWriteFastqIllumina(AjPSeqout outseq);
/*static void       seqWriteFastqInt(AjPSeqout outseq);*/
static void       seqWriteFastqSanger(AjPSeqout outseq);
static void       seqWriteFastqSolexa(AjPSeqout outseq);
static void       seqWriteFitch(AjPSeqout outseq);
static void       seqWriteGcg(AjPSeqout outseq);
static void       seqWriteGenbank(AjPSeqout outseq);
static void       seqWriteGenpept(AjPSeqout outseq);
static void       seqWriteGifasta(AjPSeqout outseq);
static void       seqWriteGff2(AjPSeqout outseq);
static void       seqWriteGff3(AjPSeqout outseq);
static void       seqWriteHennig86(AjPSeqout outseq);
static void       seqWriteIg(AjPSeqout outseq);
static void       seqWriteJackknifer(AjPSeqout outseq);
static void       seqWriteJackknifernon(AjPSeqout outseq);
static void       seqWriteListAppend(AjPSeqout outseq, const AjPSeq seq);
static void       seqWriteListClear(AjPSeqout outseq);
static void       seqWriteMase(AjPSeqout outseq);
static void       seqWriteMega(AjPSeqout outseq);
static void       seqWriteMeganon(AjPSeqout outseq);
static void       seqWriteMsf(AjPSeqout outseq);
static void       seqWriteNbrf(AjPSeqout outseq);
static void       seqWriteNcbi(AjPSeqout outseq);
static void       seqWriteNexus(AjPSeqout outseq);
static void       seqWriteNexusnon(AjPSeqout outseq);
static void       seqWritePhylip(AjPSeqout outseq);
static void       seqWritePhylipnon(AjPSeqout outseq);
static void       seqWriteRefseq(AjPSeqout outseq);
static void       seqWriteRefseqp(AjPSeqout outseq);
static void       seqWriteSam(AjPSeqout outseq);
static void       seqWriteScf(AjPSeqout outseq);
static void       seqWriteSelex(AjPSeqout outseq);
static void       seqWriteSeq(AjPSeqout outseq, const SeqPSeqFormat sf);
static void       seqWriteStaden(AjPSeqout outseq);
static void       seqWriteStrider(AjPSeqout outseq);
static void       seqWriteSwiss(AjPSeqout outseq);
static void       seqWriteSwissnew(AjPSeqout outseq);
static void       seqWriteText(AjPSeqout outseq);
static void       seqWriteTreecon(AjPSeqout outseq);




/* @funclist seqOutFormat *****************************************************
**
** Functions to write each sequence format
**
******************************************************************************/

static SeqOOutFormat seqOutFormat[] =
{
/*   Name,         Description */
/*      Alias     Single,  Save,    Nucleotide, Protein */
/*      Feature, Gap,     Multiset, WriteFunction */
    {"unknown",    "Unknown format",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, &seqWriteFasta}, /* internal default
							writes FASTA */
    /* set 'fasta' in ajSeqoutstrGetFormatDefault */
    {"gcg",        "GCG sequence format",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, &seqWriteGcg},
    {"gcg8",       "GCG old (version 8) sequence format",
	 AJTRUE,  AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, &seqWriteGcg}, /* alias for gcg */
    {"embl",       "EMBL entry format",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE, AJFALSE,
	 AJTRUE,  AJTRUE,  AJFALSE, &seqWriteEmblnew},
    {"emold",      "EMBL entry format (alias)",
	 AJTRUE,  AJFALSE, AJFALSE, AJTRUE,  AJFALSE,
	 AJTRUE,  AJTRUE,  AJFALSE, &seqWriteEmbl},
    {"emblold",    "EMBL entry format (alias)",
	 AJTRUE,  AJFALSE, AJFALSE, AJTRUE,  AJFALSE,
	 AJTRUE,  AJTRUE,  AJFALSE, &seqWriteEmbl}, /* embl pre-87 format*/
    {"em",         "EMBL entry format (alias)",
	 AJTRUE,  AJFALSE, AJFALSE, AJTRUE,  AJFALSE,
	 AJTRUE,  AJTRUE,  AJFALSE, &seqWriteEmblnew}, /* alias for embl */
    {"emblnew",    "EMBL new entry format",
	 AJTRUE,  AJFALSE, AJFALSE, AJTRUE,  AJFALSE,
	 AJTRUE,  AJTRUE,  AJFALSE, &seqWriteEmblnew},
    {"swiss",      "Swissprot entry format",
	 AJFALSE, AJFALSE, AJFALSE, AJFALSE, AJTRUE,
	 AJTRUE,  AJTRUE,  AJFALSE, &seqWriteSwissnew},
    {"swold",      "Swissprot entry format",
	 AJTRUE,  AJFALSE, AJFALSE, AJFALSE, AJTRUE,
	 AJTRUE,  AJTRUE,  AJFALSE, &seqWriteSwiss},
    {"swissold",   "Swissprot entry format", /* format before 2006 release */
	 AJTRUE,  AJFALSE, AJFALSE, AJFALSE, AJTRUE,
	 AJTRUE,  AJTRUE,  AJFALSE, &seqWriteSwiss},
    {"swissprotold","Swissprot entry format",
	 AJTRUE,  AJFALSE, AJFALSE, AJFALSE, AJTRUE,
	 AJTRUE,  AJTRUE,  AJFALSE, &seqWriteSwiss},
    {"sw",         "Swissprot entry format(alias)",
	 AJTRUE,  AJFALSE, AJFALSE, AJFALSE, AJTRUE,
	 AJTRUE,  AJTRUE,  AJFALSE, &seqWriteSwissnew}, /* alias for swiss */
    {"swissprot",  "Swissprot entry format(alias)",
	 AJTRUE,  AJFALSE, AJFALSE, AJFALSE, AJTRUE,
	 AJTRUE,  AJTRUE,  AJFALSE, &seqWriteSwissnew}, /* alias for swiss */
    {"swissnew",   "Swissprot entry format",
	 AJTRUE,  AJFALSE, AJFALSE, AJFALSE, AJTRUE,
	 AJTRUE,  AJTRUE,  AJFALSE, &seqWriteSwissnew},
    {"swnew",      "Swissprot entry format(alias)",
	 AJTRUE,  AJFALSE, AJFALSE, AJFALSE, AJTRUE,
	 AJTRUE,  AJTRUE,  AJFALSE, &seqWriteSwissnew}, /* alias for swiss */
    {"swissprotnew","Swissprot entry format(alias)",
	 AJTRUE,  AJFALSE, AJFALSE, AJFALSE, AJTRUE,
	 AJTRUE,  AJTRUE,  AJFALSE, &seqWriteSwissnew}, /* alias for swiss */
    {"fasta",      "FASTA format",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, &seqWriteFasta},
    {"pearson",    "FASTA format (alias)",
	 AJTRUE,  AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, &seqWriteFasta}, /* alias for fasta */
    {"ncbi",       "NCBI fasta format with NCBI-style IDs",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, &seqWriteNcbi},
    {"gifasta",    "NCBI fasta format with NCBI-style IDs using GI number",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, &seqWriteGifasta},
    {"nbrf",       "NBRF/PIR entry format",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJTRUE,  AJTRUE,  AJFALSE, &seqWriteNbrf},
    {"pir",        "NBRF/PIR entry format (alias)",
	 AJTRUE,  AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJTRUE,  AJTRUE,  AJFALSE, &seqWriteNbrf}, /* alias for nbrf */
    {"genbank",    "Genbank entry format",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJTRUE,  AJTRUE,  AJFALSE, &seqWriteGenbank},
    {"gb",         "Genbank entry format (alias)",
	 AJTRUE,  AJFALSE, AJFALSE, AJTRUE,  AJFALSE,
	 AJTRUE,  AJTRUE,  AJFALSE, &seqWriteGenbank}, /* alias for genbank */
    {"ddbj",       "Genbank/DDBJ entry format (alias)",
	 AJTRUE,  AJFALSE, AJFALSE, AJTRUE,  AJFALSE,
	 AJTRUE,  AJTRUE,  AJFALSE, &seqWriteGenbank}, /* alias for genbank */
    {"genpept",    "Genpept entry format",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJTRUE,  AJTRUE,  AJFALSE, &seqWriteGenpept},
    {"refseq",     "Refseq entry format",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJFALSE,
	 AJTRUE,  AJTRUE,  AJFALSE, &seqWriteRefseq},
    {"refseqp",    "Refseqp entry format",
	 AJFALSE, AJFALSE, AJFALSE, AJFALSE, AJTRUE,
	 AJTRUE,  AJTRUE,  AJFALSE, &seqWriteRefseqp},
    {"gff2",       "GFF2 feature file with sequence in the header",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJTRUE,  AJTRUE,  AJFALSE, &seqWriteGff2},
    {"gff3",       "GFF3 feature file with sequence in FASTA format after",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJTRUE,  AJTRUE,  AJFALSE, &seqWriteGff3},
    {"gff",        "GFF3 feature file with sequence in FASTA format after",
	 AJTRUE,  AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJTRUE,  AJTRUE,  AJFALSE, &seqWriteGff3},
    {"ig",         "Intelligenetics sequence format",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, &seqWriteIg},
    {"codata",     "Codata entry format",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, &seqWriteCodata},
    {"strider",    "DNA strider output format",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, &seqWriteStrider},
    {"acedb",      "ACEDB sequence format",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, &seqWriteAcedb},
    {"experiment", "Staden experiment file",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, &seqWriteExperiment},
    {"staden",     "Old staden package sequence format",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, &seqWriteStaden},
    {"text",       "Plain text",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, &seqWriteText},
    {"plain",      "Plain text (alias)",
	 AJTRUE,  AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, &seqWriteText}, /* alias for text */
    {"raw",        "Plain text (alias)",
	 AJTRUE,  AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, &seqWriteText}, /* alias for text output */
    {"fitch",      "Fitch program format",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, &seqWriteFitch},
    {"msf",        "GCG MSF (multiple sequence file) file format",
	 AJFALSE, AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, &seqWriteMsf},
    {"clustal",    "Clustalw multiple alignment format",
	 AJFALSE, AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, &seqWriteClustal},
    {"aln",        "Clustalw output format (alias)",
	 AJTRUE,  AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, &seqWriteClustal}, /* alias for clustal */
    {"selex",      "Selex format",
	 AJFALSE, AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, &seqWriteSelex},
    {"phylip",     "Phylip interleaved format",
	 AJFALSE, AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJTRUE,  &seqWritePhylip},
    {"phylipnon",  "Phylip non-interleaved format",
	 AJFALSE, AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, &seqWritePhylipnon},
    {"phylip3",    "Phylip non-interleaved format (alias)",
	 AJTRUE,  AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  /* alias for phylipnon*/
	 AJFALSE, AJTRUE,  AJFALSE, &seqWritePhylipnon},
    {"asn1",       "NCBI ASN.1 format",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, &seqWriteAsn1},
    {"hennig86",   "Hennig86 output format",
	 AJFALSE, AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, &seqWriteHennig86},
    {"mega",       "Mega interleaved output format",
	 AJFALSE, AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, &seqWriteMega},
    {"meganon",    "Mega non-interleaved output format",
	 AJFALSE, AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, &seqWriteMeganon},
    {"nexus",      "Nexus/paup interleaved format",
	 AJFALSE, AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, &seqWriteNexus},
    {"paup",       "Nexus/paup interleaved format (alias)",
	 AJTRUE,  AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, &seqWriteNexus}, /* alias for nexus */
    {"nexusnon",   "Nexus/paup non-interleaved format",
	 AJFALSE, AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, &seqWriteNexusnon},
    {"paupnon",    "Nexus/paup non-interleaved format (alias)",
	 AJTRUE,  AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, &seqWriteNexusnon},	/* alias for nexusnon*/
    {"jackknifer", "Jackknifer output interleaved format",
	 AJFALSE, AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, &seqWriteJackknifer},
    {"jackknifernon", "Jackknifer output non-interleaved format",
	 AJFALSE, AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, &seqWriteJackknifernon},
    {"treecon",    "Treecon output format",
	 AJFALSE, AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, &seqWriteTreecon},
    {"mase",       "Mase program format",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, &seqWriteMase},
    {"dasdna",     "DASDNA DAS nucleotide-only sequence",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJFALSE,
	 AJFALSE, AJTRUE,  AJFALSE, &seqWriteDasdna},
    {"das",        "DASSEQUENCE DAS any sequence",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, &seqWriteDasseq},
    {"fastq-sanger", "FASTQ short read format with phred quality",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJFALSE,
         AJFALSE, AJFALSE, AJFALSE, &seqWriteFastqSanger},
    {"fastq", "FASTQ short read format with phred quality",
	 AJTRUE,  AJFALSE, AJFALSE, AJTRUE,  AJFALSE,
         AJFALSE, AJFALSE, AJFALSE, &seqWriteFastqSanger},
    {"fastq-illumina", "FASTQ Illumina 1.3 short read format",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJFALSE,
         AJFALSE, AJFALSE, AJFALSE, &seqWriteFastqIllumina},
    {"fastq-solexa", "FASTQ Solexa/Illumina 1.0 short read format",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJFALSE,
         AJFALSE, AJFALSE, AJFALSE, &seqWriteFastqSolexa},
/*
**     {"fastq-int", "FASTQ short read format with integer phred quality",
**         AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJFALSE,
**         AJFALSE, AJFALSE, AJFALSE, seqWriteFastqInt},
*/
    {"sam",        "Sequence alignment/map (SAM) format",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
         AJFALSE, AJFALSE, AJFALSE, &seqWriteSam},
    {"bam",        "Binary sequence alignment/map (BAM) format",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
         AJFALSE, AJFALSE, AJFALSE, &seqWriteBam},
    {"scf",        "SCF format",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
         AJFALSE, AJFALSE, AJFALSE, &seqWriteScf},
    {"debug",      "Debugging trace of full internal data content",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, &seqWriteDebug}, /* trace report */
    {NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, NULL}
};




/* @datasection [AjPSeqout] Sequence output *********************************
**
** Function is for manipulating sequence output objects
**
** @nam2rule Seqout
**
******************************************************************************/




/* @section Sequence Output Constructors **************************************
**
** All constructors return a new sequence output object by pointer. It
** is the responsibility of the user to first destroy any previous
** sequence output object. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so
** anyway.
**
** @fdata [AjPSeqout]
** @fcategory new
**
** @nam3rule New Constructor
** @nam4rule NewFile Constructor using an open output file
** @nam4rule NewFormat Constructor using a named format
** @suffix C [char*] C character string
** @suffix S [AjPStr] string object
**
** @argrule NewFile file [AjPFile] Open output file
** @argrule C txt [const char*] Format name
** @argrule S str [const AjPStr] Format name
**
** @valrule * [AjPSeqout]
******************************************************************************/




/* @func ajSeqoutNew **********************************************************
**
** Creates a new sequence output object.
**
** @return [AjPSeqout] New sequence output object.
**
** @release 1.0.0
** @@
******************************************************************************/

AjPSeqout ajSeqoutNew(void)
{
    AjPSeqout pthis;

    AJNEW0(pthis);

    pthis->Name  = ajStrNew();
    /* pthis->Acc = ajStrNew(); */
    pthis->Sv    = ajStrNew();
    pthis->Gi    = ajStrNew();

/*
//    pthis->Tax   = ajStrNew();
//    pthis->Taxid = ajStrNew();
//    pthis->Organelle = ajStrNew();
*/

    pthis->Desc  = ajStrNew();
    pthis->Type  = ajStrNew();
    pthis->EType = 0;

    pthis->Outputtype = ajStrNew();

/*
//    pthis->Db    = ajStrNew();
//    pthis->Setdb = ajStrNew();
//    pthis->Setoutdb = ajStrNew();
//    pthis->Full  = ajStrNew();
//    pthis->Doc   = ajStrNew();
//    pthis->Rev   = ajFalse;
//    pthis->Ufo   = ajStrNew();
*/

    pthis->Usa   = ajStrNew();
    pthis->Informatstr = ajStrNew();
    pthis->Formatstr   = ajStrNew();

    pthis->Format    = 0;
    pthis->Filename  = ajStrNew();
    pthis->Directory = ajStrNew();
    pthis->Entryname = ajStrNew();
    pthis->Seq       = ajStrNew();
    pthis->File      = NULL;
    pthis->Count     = 0;
    pthis->Single    = ajFalse;
    pthis->Features  = ajFalse;
    pthis->Extension = ajStrNew();
    pthis->Savelist  = NULL;

    pthis->Ftquery = ajFeattabOutNew();
    pthis->Fttable = NULL;

    pthis->Acclist = ajListstrNew();

/*
//    pthis->Keylist = ajListstrNew();
//    pthis->Taxlist = ajListstrNew();
//    pthis->Genelist = ajListstrNew();
//    pthis->Cmtlist = ajListstrNew();
//    pthis->Xreflist = ajListNew();
//    pthis->Reflist = ajListNew();
//    pthis->Fulldesc = ajSeqdescNew();
*/

    return pthis;
}




/* @func ajSeqoutNewFile ******************************************************
**
** Creates a new sequence output object using a pre-opened file.
**
** @param [u] file [AjPFile] Open file object
** @return [AjPSeqout] New sequence output object.
**
** @release 4.1.0
** @@
******************************************************************************/

AjPSeqout ajSeqoutNewFile(AjPFile file)
{
    AjPSeqout pthis;

    pthis = ajSeqoutNew();
    pthis->Knownfile = file;
    pthis->File = file;

    return pthis;
}




/* @func ajSeqoutNewFormatC ***************************************************
**
** Creates a new sequence output object with a specified format.
**
** @param [r] txt [const char*] Output sequence format
** @return [AjPSeqout] New sequence output object.
**
** @release 5.0.0
** @@
******************************************************************************/

AjPSeqout ajSeqoutNewFormatC(const char* txt)
{
    AjPSeqout thys;

    thys = ajSeqoutNew();

    ajSeqoutSetFormatC(thys, txt);

    return thys;
}




/* @func ajSeqoutNewFormatS ***************************************************
**
** Creates a new sequence output object with a specified format.
**
** @param [r] str [const AjPStr] Output sequence format
** @return [AjPSeqout] New sequence output object.
**
** @release 5.0.0
** @@
******************************************************************************/

AjPSeqout ajSeqoutNewFormatS(const AjPStr str)
{
    AjPSeqout thys;

    thys = ajSeqoutNew();

    ajSeqoutSetFormatS(thys, str);

    return thys;
}




/* @section destructors ***************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the sequence output object.
**
**
** @fdata [AjPSeqout]
** @fcategory delete
**
** @nam3rule Del Destroy (free) a sequence output object
**
** @argrule * Pseqout [AjPSeqout*] Sequence output object address
**
** @valrule * [void]
**
******************************************************************************/




/* @func ajSeqoutDel **********************************************************
**
** Destructor for AjPSeqout objects
**
** @param [d] Pseqout [AjPSeqout*] Sequence output object
** @return [void]
**
** @release 2.9.0
** @@
******************************************************************************/

void ajSeqoutDel(AjPSeqout* Pseqout)
{
    AjPSeqout seqout;
    AjPSeq    seq    = NULL;
    AjPStr    tmpstr = NULL;
    AjPSeqRef  tmpref  = NULL;
    AjPSeqXref tmpxref = NULL;
    AjPSeqGene tmpgene = NULL;

    seqout = *Pseqout;

    if(!seqout)
	return;

    ajStrDel(&seqout->Name);
    ajStrDel(&seqout->Acc);
    MAJSTRDEL(&seqout->Sv);
    MAJSTRDEL(&seqout->Gi);
    MAJSTRDEL(&seqout->Tax);
    MAJSTRDEL(&seqout->Taxid);
    MAJSTRDEL(&seqout->Organelle);
    ajStrDel(&seqout->Desc);
    ajStrDel(&seqout->Type);
    ajStrDel(&seqout->Outputtype);
    MAJSTRDEL(&seqout->Molecule);
    MAJSTRDEL(&seqout->Class);
    MAJSTRDEL(&seqout->Division);
    MAJSTRDEL(&seqout->Evidence);
    MAJSTRDEL(&seqout->Db);
    MAJSTRDEL(&seqout->Setdb);
    MAJSTRDEL(&seqout->Setoutdb);
    MAJSTRDEL(&seqout->Full);
    MAJSTRDEL(&seqout->Doc);
    MAJSTRDEL(&seqout->Usa);
    MAJSTRDEL(&seqout->Ufo);
    MAJSTRDEL(&seqout->FtFormat);
    MAJSTRDEL(&seqout->FtFilename);
    ajStrDel(&seqout->Informatstr);
    ajStrDel(&seqout->Formatstr);
    ajStrDel(&seqout->Filename);
    MAJSTRDEL(&seqout->Directory);
    ajStrDel(&seqout->Entryname);
    ajStrDel(&seqout->Seq);
    ajStrDel(&seqout->Extension);

    if(seqout->Acclist)
    {
        while(ajListPop(seqout->Acclist,(void **)&tmpstr))
            ajStrDel(&tmpstr);

        ajListFree(&seqout->Acclist);
    }

    if(seqout->Keylist)
    {
        while(ajListPop(seqout->Keylist,(void **)&tmpstr))
            ajStrDel(&tmpstr);

        ajListFree(&seqout->Keylist);
    }

    if(seqout->Taxlist)
    {
        while(ajListPop(seqout->Taxlist,(void **)&tmpstr))
            ajStrDel(&tmpstr);

        ajListFree(&seqout->Taxlist);
    }

    if(seqout->Genelist)
    {
        while(ajListPop(seqout->Genelist,(void **)&tmpgene))
            ajSeqgeneDel(&tmpgene);

        ajListFree(&seqout->Genelist);
    }

    if(seqout->Reflist)
    {
        while(ajListPop(seqout->Reflist,(void **)&tmpref))
            ajSeqrefDel(&tmpref);

        ajListFree(&seqout->Reflist);
    }

    if(seqout->Cmtlist)
    {
        while(ajListPop(seqout->Cmtlist,(void **)&tmpstr))
            ajStrDel(&tmpstr);

        ajListFree(&seqout->Cmtlist);
    }

    if(seqout->Xreflist)
    {
        while(ajListPop(seqout->Xreflist,(void **)&tmpxref))
            ajSeqxrefDel(&tmpxref);

        ajListFree(&seqout->Xreflist);
    }

    if(seqout->Savelist)
    {
        while(ajListPop(seqout->Savelist,(void **)&seq))
            ajSeqDel(&seq);

        ajListFree(&seqout->Savelist);
    }

    if(seqout->Ftquery)
        ajFeattabOutDel(&seqout->Ftquery);

    if(seqout->Cleanup)
        (*seqout->Cleanup)(seqout);

    seqout->Cleanup = NULL;

    if(seqout->Knownfile)
	seqout->File = NULL;
    else
	ajFileClose(&seqout->File);

    if(seqout->Date)
        ajSeqdateDel(&seqout->Date);
    if(seqout->Fulldesc)
        ajSeqdescDel(&seqout->Fulldesc);

    AJFREE(seqout->Accuracy);
    AJFREE(*Pseqout);

    return;
}




/* @datasection [AjPSeqout] Sequence stream output ****************************
**
** Function is for writing sequence streams using sequence output objects
**
** @nam2rule Seqall
**
******************************************************************************/




/* @section Sequence Stream output *****************************************
**
** These functions output the contents of a sequence stream object.
** As the input is a stream they expect to be called again.
**
** @fdata [AjPSeqout]
** @fcategory output
**
** @nam2rule Seqout
**
** @nam3rule Write Write sequence data
** @nam4rule WriteSet Write sequence set data
** @nam4rule WriteSeq Write single sequence data
**
** @argrule * outseq [AjPSeqout] Sequence output object
** @argrule WriteSet seq [const AjPSeqset] sequence set current object
** @argrule WriteSeq seq [const AjPSeq] sequence object
**
** @valrule * [AjBool] True on success
**
******************************************************************************/




/* @func ajSeqoutWriteSeq *****************************************************
**
** Write next sequence out - continue until done.
**
** @param [u] outseq [AjPSeqout] Sequence output.
** @param [r] seq [const AjPSeq] Sequence.
** @return [AjBool] True if sequence was successfully written.
**                  Note if the save flag is true this means it was saved
**                  to be written later when the output is closed.
**
** @release 4.1.0
** @@
******************************************************************************/

AjBool ajSeqoutWriteSeq(AjPSeqout outseq, const AjPSeq seq)
{
    AjBool isnuc = ajFalse;
    AjBool isprot = ajFalse;
    AjBool ok;
    
    ajDebug("ajSeqoutWriteSeq '%S' len: %d\n",
	    ajSeqGetNameS(seq), ajSeqGetLen(seq));
    
    if(!outseq->Format)
    {
	if(!seqoutFindOutFormat(outseq->Formatstr, &outseq->Format))
        {
            if(!outseq->Count++)
                ajErr("unknown output sequence format '%S'",
                      outseq->Formatstr);
            return ajFalse;
        }
    }

    ajDebug("ajSeqoutWriteSeq %d '%s' single: %B feat: %B Save: %B\n",
	    outseq->Format,
	    seqOutFormat[outseq->Format].Name,
	    seqOutFormat[outseq->Format].Single,
	    outseq->Features,
	    seqOutFormat[outseq->Format].Save);
    
    if(ajSeqIsNuc(seq))
	isnuc = ajTrue;

    if(ajSeqIsProt(seq))
	isprot = ajTrue;

    ok = ajFalse;

    if(isnuc && seqOutFormat[outseq->Format].Nucleotide)
	ok = ajTrue;
    else if(isprot && seqOutFormat[outseq->Format].Protein)
	ok = ajTrue;

    if(!ok)
    {
	if(isnuc)
	    ajErr("Sequence format '%S' not supported for nucleotide sequences",
		  outseq->Formatstr);
	else if(isprot)
	    ajErr("Sequence format '%S' not supported for protein sequences",
		  outseq->Formatstr);
	else
	    ajErr("Sequence format '%S' failed: unknown sequence type",
		  outseq->Formatstr);

	return ajFalse;
    }

    seqClone(outseq, seq);

    if(seqOutFormat[outseq->Format].Save)
    {
	seqWriteListAppend(outseq, seq);
	outseq->Count++;

	return ajTrue;
    }
           
    ajSeqoutSetNameDefaultS(outseq, !outseq->Single, outseq->Entryname);

    if(outseq->Fttable)
	ajFeattableSetDefname(outseq->Fttable, outseq->Name);

    if(outseq->Single)
	seqFileReopen(outseq);
    
    if (outseq->Knownfile && !outseq->File)
	outseq->File = outseq->Knownfile;
    
    /* Calling funclist seqOutFormat() */
    (*seqOutFormat[outseq->Format].Write)(outseq);
    outseq->Count++;
    
    ajDebug("ajSeqoutWriteSeq tests features %B taboutisopen %B "
	    "UfoLocal %B ftlocal %B\n",
	    outseq->Features, ajFeattabOutIsOpen(outseq->Ftquery),
	    seqoutUfoLocal(outseq), ajFeattabOutIsLocal(outseq->Ftquery));

    if(outseq->Features &&
       !ajFeattabOutIsLocal(outseq->Ftquery)) /* not already done */
    {
	if(!ajFeattabOutIsOpen(outseq->Ftquery))
	{
	    ajDebug("ajSeqoutWriteSeq features output needed\n");
	    ajFeattabOutSetBasename(outseq->Ftquery, outseq->Filename);

	    if(!ajFeattabOutOpen(outseq->Ftquery, outseq->Ufo))
	    {
		ajWarn("ajSeqoutWriteSeq features output file open failed "
                       "'%S%S'",
		       outseq->Ftquery->Directory, outseq->Ftquery->Filename);

		return ajFalse;
	    }

	    ajStrAssignEmptyS(&outseq->Ftquery->Seqname, seq->Name);
	    ajStrAssignEmptyS(&outseq->Ftquery->Type, seq->Type);
	}

	/* ajFeattableTrace(outseq->Fttable); */
	if(!ajFeattableWriteUfo(outseq->Ftquery, outseq->Fttable,
			   outseq->Ufo))
	{
	    ajWarn("ajSeqWriteSeq features output failed UFO: '%S'",
		   outseq->Ufo);

	    return ajFalse;
	}
    }
    
    seqDeclone(outseq);
    
    return ajTrue;
}




/* @func ajSeqoutWriteSet *****************************************************
**
** Write a set of sequences out.
**
** @param [u] outseq [AjPSeqout] Sequence output.
** @param [r] seq [const AjPSeqset] Sequence set.
** @return [AjBool] True on success
**
** @release 4.1.0
** @@
******************************************************************************/

AjBool ajSeqoutWriteSet(AjPSeqout outseq, const AjPSeqset seq)
{
    ajuint i = 0;

    ajDebug("ajSeqoutWriteSet\n");

    if(!outseq->Format)
	if(!seqoutFindOutFormat(outseq->Formatstr, &outseq->Format))
	    ajErr("unknown output sequence set format '%S'", outseq->Formatstr);

    ajDebug("ajSeqoutWriteSet %d '%s' single: %B feat: %B Save: %B\n",
	    outseq->Format,
	    seqOutFormat[outseq->Format].Name,
	    seqOutFormat[outseq->Format].Single,
	    outseq->Features,
	    seqOutFormat[outseq->Format].Save);

    for(i=0; i < seq->Size; i++)
    {
	seqsetClone(outseq, seq, i);

	if(seqOutFormat[outseq->Format].Save)
	{
	    seqWriteListAppend(outseq, seq->Seq[i]);
	    outseq->Count++;
	    continue;
	}

	ajSeqoutSetNameDefaultS(outseq, !outseq->Single, outseq->Entryname);

	if(outseq->Fttable)
	    ajFeattableSetDefname(outseq->Fttable, outseq->Name);

	if(outseq->Single)
	    seqFileReopen(outseq);

	/* Calling funclist seqOutFormat() */
	(*seqOutFormat[outseq->Format].Write)(outseq);
	outseq->Count++;

	ajDebug("ajSeqoutWriteSet tests features %B taboutisopen %B "
		"UfoLocal %B ftlocal %B\n",
		outseq->Features, ajFeattabOutIsOpen(outseq->Ftquery),
		seqoutUfoLocal(outseq), ajFeattabOutIsLocal(outseq->Ftquery));

	if(outseq->Features &&
	   !ajFeattabOutIsLocal(outseq->Ftquery))
	{
	    /* not already done */
	    if(!ajFeattabOutIsOpen(outseq->Ftquery))
	    {
		ajDebug("ajSeqoutWriteSet features output needed\n");
		ajFeattabOutSetBasename(outseq->Ftquery, outseq->Filename);

		if(!ajFeattabOutOpen(outseq->Ftquery, outseq->Ufo))
		{
		    ajWarn("ajSeqoutWriteSet features output "
			   "failed to open UFO '%S'",
			   outseq->Ufo);

		    return ajFalse;
		}

		ajStrAssignEmptyS(&outseq->Ftquery->Seqname, seq->Name);
		ajStrAssignEmptyS(&outseq->Ftquery->Type, seq->Type);
	    }

	    /* ajFeattableTrace(outseq->Fttable); */

	    if(!ajFeattableWriteUfo(outseq->Ftquery, outseq->Fttable,
                                    outseq->Ufo))
	    {
		ajWarn("ajSeqoutWriteSet features output failed UFO: '%S'",
		       outseq->Ufo);

		return ajFalse;
	    }
	}
	
	seqDeclone(outseq);
    }
    
    ajSeqoutFlush(outseq);

    return ajTrue;
}




/* @section non-standard output ************************************************
**
** Writes a sequence string in SWISSPROT format with a user-defined line code
**
** @fdata [AjPSeqout]
** @fcategory output
**
** @nam3rule Dump Print sequence to an output file
** @nam4rule Swisslike  Dump in swissprot-like format with a user-defined
**                      sequence line code
**
** @argrule Dump outseq [AjPSeqout] Sequence output object
** @argrule Dump seq [const AjPStr] Sequence string
** @argrule Swisslike prefix [const char*] Line prefix code
**
** @valrule * [void]
******************************************************************************/




/* @func ajSeqoutDumpSwisslike ************************************************
**
** Writes a sequence in SWISSPROT format with a user-defined line code
**
** @param [w] outseq [AjPSeqout] Sequence output object
** @param [r] seq [const AjPStr] sequence
** @param [r] prefix [const char *] line prefix code - should be 2 characters
** @return [void]
**
** @release 5.0.0
** @@
******************************************************************************/

void ajSeqoutDumpSwisslike(AjPSeqout outseq,
			   const AjPStr seq, const char *prefix)
{
    static SeqPSeqFormat sf = NULL;
    AjPFile outf = outseq->File;
    ajint mw;
    unsigned long long crc;

    crc = ajMathCrc64(seq);
    mw = (ajint) (0.5+ajSeqstrCalcMolwt(seq));
    ajFmtPrintF(outf,
		"%-5sSEQUENCE %5d AA; %6d MW;  %08X",
		prefix, ajStrGetLen(seq), mw, 
		(crc>>32)&0xffffffff);

    ajFmtPrintF(outf,
		"%08X CRC64;\n",
		crc&0xffffffff);

    ajStrAssignS(&outseq->Seq,seq);

    seqSeqFormat(ajStrGetLen(outseq->Seq), &sf);
    strcpy(sf->endstr, "");
    sf->tab    = 4;
    sf->spacer = 11;
    sf->width  = 60;

    seqWriteSeq(outseq, sf);
    seqFormatDel(&sf);

    return;
}




/* @funcstatic seqWriteListAppend *********************************************
**
** Add the latest sequence to the output list. If we are in single
** sequence mode, also write it out now though it does not seem
** a great idea in most cases to ask for this.
**
** @param [u] outseq [AjPSeqout] Sequence output
** @param [r] seq [const AjPSeq] Sequence to be appended
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void seqWriteListAppend(AjPSeqout outseq, const AjPSeq seq)
{
    AjPSeq listseq;

    ajDebug("seqWriteListAppend '%F' %S\n", outseq->File, ajSeqGetNameS(seq));

    if(!outseq->Savelist)
	outseq->Savelist = ajListNew();

    listseq = ajSeqNewSeq(seq);
    ajSeqTrim(listseq);

    /* if(listseq->Rev)
       ajSeqReverseDo(listseq); */ /* already done */

    if(outseq->Single)
	ajSeqSetName(listseq, outseq->Entryname);
    else
	ajSeqSetNameMulti(listseq, outseq->Entryname);

    if(listseq->Fttable)
	ajFeattableSetDefname(listseq->Fttable, listseq->Name);

    ajListPushAppend(outseq->Savelist, listseq);

    if(outseq->Single)
    {
	ajDebug("single sequence mode: write immediately\n");
	ajSeqoutSetNameDefaultS(outseq, !outseq->Single, outseq->Entryname);

	if(outseq->Fttable)
	    ajFeattableSetDefname(outseq->Fttable, outseq->Name);

	/* Calling funclist seqOutFormat() */
	(*seqOutFormat[outseq->Format].Write)(outseq);
    }

    ajDebug("seqWriteListAppend Features: %B IsLocal: %B Count: %d\n",
	    outseq->Features, ajFeattabOutIsLocal(outseq->Ftquery),
	    ajFeattableGetSize(outseq->Fttable));

    if(outseq->Features &&
       !ajFeattabOutIsLocal(outseq->Ftquery))
    {
/*	seqClone(outseq, seq);	*/    /* already cloned feature table */
	ajDebug("seqWriteListAppend after seqClone Count: %d\n",
		ajFeattableGetSize(outseq->Fttable));

	if(!ajFeattabOutIsOpen(outseq->Ftquery))
	{
	    ajDebug("seqWriteListAppend features output needed table\n");
	    
	    ajFeattabOutSetBasename(outseq->Ftquery, outseq->Filename);

	    if(!ajFeattabOutOpen(outseq->Ftquery, outseq->Ufo))
	    {
		ajWarn("seqWriteListAppend features output "
		       "failed to open UFO '%S'",
		       outseq->Ufo);
		return;
	    }

	    ajStrAssignEmptyS(&outseq->Ftquery->Seqname, seq->Name);
	    ajStrAssignEmptyS(&outseq->Ftquery->Type, seq->Type);
	}

	ajDebug("seqWriteListAppend after ajFeattabOutOpen Count: %d\n",
		ajFeattableGetSize(outseq->Fttable));
	ajFeattableTrace(outseq->Fttable);

	if(!ajFeattableWriteUfo(outseq->Ftquery, outseq->Fttable, outseq->Ufo))
	{
	    ajWarn("seqWriteListAppend features output failed UFO: '%S'",
		   outseq->Ufo);

	    return;
	}
	    
	seqDeclone(outseq);
    }
    
    return;
}




/* @funcstatic seqWriteListClear **********************************************
**
** Removes the sequences from the output list. Called after sequences
** have been written.
**
** @param [u] outseq [AjPSeqout] Sequence output
** @return [void]
**
** @release 6.1.0
** @@
******************************************************************************/

static void seqWriteListClear(AjPSeqout outseq)
{
    AjPSeq seq;

    while(ajListPop(outseq->Savelist,(void **)&seq))
	ajSeqDel(&seq);

    return;
}




/* @funcstatic seqWriteFasta **************************************************
**
** Writes a sequence in FASTA format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void seqWriteFasta(AjPSeqout outseq)
{
    ajuint i;
    ajuint ilen;
    AjPStr seq = NULL;
    ajuint linelen     = 60;
    ajuint iend;

    ajDebug("seqWriteFasta Name '%S'\n",
	    outseq->Name);

    seqDbName(&outseq->Name, outseq->Setoutdb);

    ajWritebinByte(outseq->File, '>');
    ajWriteline(outseq->File, outseq->Name);

    if(ajStrGetLen(outseq->Sv))
	ajWritelineSpace(outseq->File, outseq->Sv);
    else if(ajStrGetLen(outseq->Acc))
	ajWritelineSpace(outseq->File, outseq->Acc);

    /* no need to bother with outseq->Gi because we have Sv anyway */

    if(ajStrGetLen(outseq->Desc))
        ajWritelineSpace(outseq->File, outseq->Desc);

    ajWritebinNewline(outseq->File);
    ilen = ajStrGetLen(outseq->Seq);

    for(i=0; i < ilen; i += linelen)
    {
	iend = AJMIN(ilen-1, i+linelen-1);
	ajStrAssignSubS(&seq, outseq->Seq, i, iend);
        ajWritelineNewline(outseq->File, seq);
    }

    ajStrDel(&seq);

    return;
}




/* @funcstatic seqWriteFastqSanger ********************************************
**
** Writes a sequence in FASTA format with phred scores
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 6.1.0
** @@
******************************************************************************/

static void seqWriteFastqSanger(AjPSeqout outseq)
{
    ajuint i;
    ajuint ilen;

    /* ajuint j; */
    /* ajuint jlen; */

    AjPStr seq = NULL;
    /*ajuint linelen     = 60;*/
    /*ajuint iend;*/
    /*char qchar;*/

    ajDebug("seqWriteFastqSanger Name '%S'\n",
	    outseq->Name);

    seqDbName(&outseq->Name, outseq->Setoutdb);

    ajWritebinByte(outseq->File, '@');
    ajWriteline(outseq->File, outseq->Name);

    if(ajStrGetLen(outseq->Sv))
	ajWritelineSpace(outseq->File, outseq->Sv);
    else if(ajStrGetLen(outseq->Acc))
	ajWritelineSpace(outseq->File, outseq->Acc);

    /* no need to bother with outseq->Gi because we have Sv anyway */

    if(ajStrGetLen(outseq->Desc))
       ajWritelineSpace(outseq->File, outseq->Desc);

    ajWritebinNewline(outseq->File);

    ajWritelineNewline(outseq->File, outseq->Seq);
    ajWritebinByte(outseq->File, '+');
    ajWritebinNewline(outseq->File);

    ilen = ajStrGetLen(outseq->Seq);
    seq = ajStrNewRes(ilen+1);

    if(outseq->Accuracy)
    {
        for(i=0;i<ilen;i++)
	{
	    ajStrAppendK(&seq, 33 + (int) outseq->Accuracy[i]);
	}
	ajWritelineNewline(outseq->File, seq);
    }

    else 
    {
        ajStrAppendCountK(&seq,'\"', ilen);
	ajWritelineNewline(outseq->File, seq);
    }
    
    ajStrDel(&seq);

    return;
}




/* @funcstatic seqWriteFastqIllumina ******************************************
**
** Writes a sequence in FASTA format with Illumina scores
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 6.1.0
** @@
******************************************************************************/

static void seqWriteFastqIllumina(AjPSeqout outseq)
{
    ajuint j;
    ajuint ilen;
    AjPStr seq = NULL;
    ajint  qchar;

    ajDebug("seqWriteFastqIllumina Name '%S'\n",
	    outseq->Name);

    seqDbName(&outseq->Name, outseq->Setoutdb);

    ajWritebinByte(outseq->File, '@');
    ajWriteline(outseq->File, outseq->Name);

    if(ajStrGetLen(outseq->Sv))
	ajWritelineSpace(outseq->File, outseq->Sv);
    else if(ajStrGetLen(outseq->Acc))
	ajWritelineSpace(outseq->File, outseq->Acc);

    /* no need to bother with outseq->Gi because we have Sv anyway */

    if(ajStrGetLen(outseq->Desc))
       ajWritelineSpace(outseq->File, outseq->Desc);

    ajWritebinNewline(outseq->File);
    ilen = ajStrGetLen(outseq->Seq);

    ajWritelineNewline(outseq->File, outseq->Seq);
    ajWritebinByte(outseq->File, '+');
    ajWritebinNewline(outseq->File);

    ilen = ajStrGetLen(outseq->Seq);

    if(outseq->Accuracy)
    {
        ajStrAssignClear(&seq);

        for(j=0;j<ilen;j++)
        {
            qchar = 64 + (int) (0.5 + outseq->Accuracy[j]);
            if(qchar > 126)
                qchar = 126;
            else if(qchar < 33)
                qchar = 33;
            ajStrAppendK(&seq, (char) qchar);
        }

	ajWritelineNewline(outseq->File, seq);
    }

    else 
    {
        /*
        ** default to a score of 1 (0.75 error : 1 base in 4 is right)
        */

        ajStrAssignClear(&seq);

        ajStrAppendCountK(&seq,'A', ilen);

	ajWritelineNewline(outseq->File, seq);
    }
    
    ajStrDel(&seq);

    return;
}




/* @funcstatic seqWriteFastqSolexa ********************************************
**
** Writes a sequence in FASTA format with Solexa/Illumina scores
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 6.1.0
** @@
******************************************************************************/

static void seqWriteFastqSolexa(AjPSeqout outseq)
{
    ajuint i;
    ajuint j;
    ajuint ilen;
    AjPStr seq = NULL;
    double sval;

    ajDebug("seqWriteFastqSolexa Name '%S'\n",
	    outseq->Name);

    seqDbName(&outseq->Name, outseq->Setoutdb);

    ajWritebinByte(outseq->File, '@');
    ajWriteline(outseq->File, outseq->Name);

    if(ajStrGetLen(outseq->Sv))
	ajWritelineSpace(outseq->File, outseq->Sv);
    else if(ajStrGetLen(outseq->Acc))
	ajWritelineSpace(outseq->File, outseq->Acc);

    /* no need to bother with outseq->Gi because we have Sv anyway */

    if(ajStrGetLen(outseq->Desc))
       ajWritelineSpace(outseq->File, outseq->Desc);

    ajWritebinNewline(outseq->File);

    ilen = ajStrGetLen(outseq->Seq);

    ajWritelineNewline(outseq->File, outseq->Seq);
    ajWritebinByte(outseq->File, '+');
    ajWritebinNewline(outseq->File);

    if(outseq->Accuracy)
    {
        ajStrAssignClear(&seq);

        for(j=0;j<ilen;j++)
        {
            sval = outseq->Accuracy[j];

            i = (int) sval;
            if(i > 62)
                i = 62;

            i = seqQualPhredToIndex[i];

            while(seqQualIndexToSolexa[i] < sval)
                i++;

            ajStrAppendK(&seq, (char) (58+i));
        }

	ajWritelineNewline(outseq->File, seq);
    }

    else 
    {
        /*
        ** default to a score of -5 (0.75 error : 1 base in 4 is right)
        */

        ajStrAssignClear(&seq);
        
        ajStrAppendCountK(&seq,';', ilen); 

	ajWritelineNewline(outseq->File, seq);
    }
    
    ajStrDel(&seq);

    return;
}




/* #funcstatic seqWriteFastqInt ***********************************************
**
** Writes a sequence in FASTA format with Solexa integer scores
**
** #param [u] outseq [AjPSeqout] Sequence output object.
** #return [void]
** ##
******************************************************************************/

/*
//static void seqWriteFastqInt(AjPSeqout outseq)
//{
//    ajuint i;
//    ajuint j;
//    ajuint ilen;
//    ajuint jlen;
//    AjPStr seq = NULL;
//    ajuint linelen     = 60;
//    ajuint numcount = 20;
//    ajuint iend;
//    AjPStr db = NULL;
//    double sval;
//    double pval;
//    double qval;
//
//    ajStrAssignS(&db, outseq->Setoutdb);
//
//    ajDebug("seqWriteFastqInt outseq Db '%S' Setdb '%S' Setoutdb '%S' "
//            "Name '%S'\n",
//	    outseq->Db, outseq->Setdb, outseq->Setoutdb, outseq->Name);
//
//    seqDbName(&outseq->Name, db);
//
//    ajFmtPrintF(outseq->File, "@%S", outseq->Name);
//
//    if(ajStrGetLen(outseq->Sv))
//	ajFmtPrintF(outseq->File, " %S", outseq->Sv);
//    else if(ajStrGetLen(outseq->Acc))
//	ajFmtPrintF(outseq->File, " %S", outseq->Acc);
//
//    if(ajStrGetLen(outseq->Desc))
//	ajFmtPrintF(outseq->File, " %S", outseq->Desc);
//
//    ajFmtPrintF(outseq->File, "\n");
//    ilen = ajStrGetLen(outseq->Seq);
//
//    for(i=0; i < ilen; i += linelen)
//    {
//	iend = AJMIN(ilen-1, i+linelen-1);
//	ajStrAssignSubS(&seq, outseq->Seq, i, iend);
//	ajFmtPrintF(outseq->File, "%S\n", seq);
//    }
//
//    ajFmtPrintF(outseq->File, "+%S", outseq->Name);
//
//    if(ajStrGetLen(outseq->Sv))
//	ajFmtPrintF(outseq->File, " %S", outseq->Sv);
//    else if(ajStrGetLen(outseq->Acc))
//	ajFmtPrintF(outseq->File, " %S", outseq->Acc);
//
//    ajFmtPrintF(outseq->File, "\n");
//
//    ilen = ajStrGetLen(outseq->Seq);
//
//    if(outseq->Accuracy)
//    {
//        for(i=0; i < ilen; i += numcount)
//        {
//            iend = AJMIN(ilen-1, i+numcount-1);
//            ajStrAssignClear(&seq);
//            for(j=i;j<=iend;j++)
//            {
//                sval = outseq->Accuracy[j];
//                pval = 1.0 / pow(10.0, (sval/10.0));
//                qval = -10.0 * log10(pval/(1.0 - pval));
//                if(j==i)
//                    ajFmtPrintAppS(&seq, "%2d", (ajint) qval);
//                else
//                    ajFmtPrintAppS(&seq, " %2d", (ajint) qval);
//            }
//            ajFmtPrintF(outseq->File, "%S\n", seq);
//        }
//    }
//
//    else 
//    {
//        for(i=0; i < ilen; i += linelen)
//        {
//            iend = AJMIN(ilen-1, i+linelen-1);
//            jlen = (iend - i + 1);
//            ajStrAssignClear(&seq);
//            ajStrAppendC(&seq," 0"); 
//            ajFmtPrintF(outseq->File, "%S\n", seq);
//        }
//
//    }
//    
//    ajStrDel(&seq);
//    ajStrDel(&db);
//
//    return;
//}
*/




/* @funcstatic seqNcbiKnowndb *************************************************
**
** Tests whether a database name is valid for use in NCBI ids.
**
** @param [r] dbname [const AjPStr] Database name
** @return [AjBool] True if found
**
** @release 4.1.0
** @@
******************************************************************************/

static AjBool seqNcbiKnowndb(const AjPStr dbname)
{
    static const char* ncbidbs[] = {
	"gb", "emb", "dbj",		/* big three, listed by NCBI for nr */
	"sp", "ref", "pir", "prf",	/* others listed by NCBI for nr */
	"tpd", "tpe", "tpg",		/* third party annotation */
	NULL
    };
    ajint i;

    for (i=0;ncbidbs[i];i++)
    {
	if(ajStrMatchC(dbname, ncbidbs[i]))
	    return ajTrue;
    }

    return ajFalse;
}




/* @funcstatic seqWriteNcbi ***************************************************
**
** Writes a sequence in NCBI format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void seqWriteNcbi(AjPSeqout outseq)
{

    ajuint i;
    ajuint ilen;
    AjPStr seq = NULL;
    ajuint linelen     = 60;
    ajuint iend;
    AjPStr version = NULL;
    AjPStr dbname = NULL;
    static ajuint blordnum=0;

    ajDebug("seqWriteNcbi SetDb '%S' Db '%S'\n", outseq->Setdb, outseq->Db);

    if(ajStrGetLen(outseq->Setdb))
	ajStrAssignS(&dbname, outseq->Setdb);
    else if(ajStrGetLen(outseq->Db))
	ajStrAssignS(&dbname, outseq->Db);
    else
	ajStrAssignC(&dbname, "unk");


    if(ajStrGetLen(outseq->Sv))
	ajStrAssignS(&version, outseq->Sv);
    else if(ajStrGetLen(outseq->Acc))
	ajStrAssignS(&version, outseq->Acc);
    else
	ajStrAssignClear(&version);

    ajDebug("seqWriteNcbi version '%S' dbname: '%S' KnownDb: %B\n",
	    version, dbname, seqNcbiKnowndb(dbname));

    if(ajStrGetLen(outseq->Gi) &&
       !ajStrGetLen(outseq->Db) &&
       ajStrMatchCaseS(outseq->Gi, outseq->Name))
    {
        ajWritebinChar(outseq->File, ">gi|", 4);
        ajWriteline(outseq->File, outseq->Gi);
    }
    else
    {
        ajWritebinByte(outseq->File, '>');

	if(ajStrGetLen(outseq->Gi))
        {
            ajWritebinChar(outseq->File, "gi|", 3);
            ajWriteline(outseq->File, outseq->Gi);
            ajWritebinByte(outseq->File, '|');
        }
        
	if(seqNcbiKnowndb(dbname))
        {
            ajWriteline(outseq->File, dbname);
            ajWritebinByte(outseq->File, '|');
            ajWriteline(outseq->File, version);
            ajWritebinByte(outseq->File, '|');
        }
        
	else if(ajStrMatchCaseC(dbname, "lcl"))
            ajWritebinChar(outseq->File, "lcl|", 4);
        
	else if(ajStrMatchCaseC(dbname, "bbs"))
            ajWritebinChar(outseq->File, "bbs|", 4);

	else if(ajStrMatchCaseC(dbname, "BL_ORD_ID"))
	    ajFmtPrintF(outseq->File, "gnl|%S|%d ", dbname, blordnum++);

	else
        {
            ajWritebinChar(outseq->File, "gnl|", 4);
            ajWriteline(outseq->File, dbname);
            ajWritebinByte(outseq->File, '|');
        }

	if (!ajStrMatchCaseS(version, outseq->Name))
	    ajWriteline(outseq->File, outseq->Name);
    }

    if(ajStrGetLen(version) && !seqNcbiKnowndb(dbname))
    {
        ajWritebinChar(outseq->File, " (", 2);
        ajWriteline(outseq->File, version);
        ajWritebinByte(outseq->File, ')');
    }

    if(ajStrGetLen(outseq->Desc))
	ajWritelineSpace(outseq->File, outseq->Desc);

    ajWritebinNewline(outseq->File);

    ilen = ajStrGetLen(outseq->Seq);

    for(i=0; i < ilen; i += linelen)
    {
	iend = AJMIN(ilen-1, i+linelen-1);
	ajStrAssignSubS(&seq, outseq->Seq, i, iend);
	ajWritelineNewline(outseq->File, seq);
    }

    ajStrDel(&seq);
    ajStrDel(&dbname);
    ajStrDel(&version);

    return;
}




/* @funcstatic seqWriteGifasta ************************************************
**
** Writes a sequence in NCBI format using only the GI number
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 4.1.0
** @@
******************************************************************************/

static void seqWriteGifasta(AjPSeqout outseq)
{

    ajuint i;
    ajuint ilen;
    AjPStr seq = NULL;
    ajuint linelen     = 60;
    ajuint iend;
    AjPStr version = NULL;
    AjPStr dbname = NULL;
    static ajuint blordnum=0;

    if(ajStrGetLen(outseq->Setdb))
	ajStrAssignS(&dbname, outseq->Setdb);
    else if(ajStrGetLen(outseq->Db))
	ajStrAssignS(&dbname, outseq->Db);
    else
	ajStrAssignC(&dbname, "unk");


    if(ajStrGetLen(outseq->Sv))
	ajStrAssignS(&version, outseq->Sv);
    else if(ajStrGetLen(outseq->Acc))
	ajStrAssignS(&version, outseq->Acc);

    else
	ajStrAssignClear(&version);

    ajDebug("seqWriteGifasta version '%S' dbname: '%S' KnownDb: %B\n",
	    version, dbname, seqNcbiKnowndb(dbname));

    if(ajStrGetLen(outseq->Gi) &&
       !ajStrGetLen(outseq->Db) &&
       ajStrMatchCaseS(outseq->Gi, outseq->Name))
    {
	    ajFmtPrintF(outseq->File, ">gi|%S", outseq->Gi);
    }
    else
    {
	ajWritebinByte(outseq->File, '>');

	if(ajStrGetLen(outseq->Gi))
	    ajFmtPrintF(outseq->File, "gi|%S|", outseq->Gi);
	else
	    ajFmtPrintF(outseq->File, "gi|000000|");

	if(seqNcbiKnowndb(dbname))
	    ajFmtPrintF(outseq->File, "%S|%S|", dbname, version);
	else if(ajStrMatchCaseC(dbname, "lcl"))
	    ajFmtPrintF(outseq->File, "%S|", dbname);
	else if(ajStrMatchCaseC(dbname, "BL_ORD_ID"))
	    ajFmtPrintF(outseq->File, "gnl|%S|%d ", dbname, blordnum++);
	else
	    ajFmtPrintF(outseq->File, "gnl|%S|", dbname);

	if (!ajStrMatchCaseS(version, outseq->Name))
	    ajWriteline(outseq->File, outseq->Name);
    }

    if(ajStrGetLen(version) && !seqNcbiKnowndb(dbname))
	ajFmtPrintF(outseq->File, " (%S)", version);

    if(ajStrGetLen(outseq->Desc))
	ajWritelineSpace(outseq->File, outseq->Desc);

    ajWritebinNewline(outseq->File);

    ilen = ajStrGetLen(outseq->Seq);

    for(i=0; i < ilen; i += linelen)
    {
	iend = AJMIN(ilen-1, i+linelen-1);
	ajStrAssignSubS(&seq, outseq->Seq, i, iend);
	ajWritelineNewline(outseq->File, seq);
    }

    ajStrDel(&seq);
    ajStrDel(&dbname);
    ajStrDel(&version);

    return;
}




/* @funcstatic seqWriteGcg ****************************************************
**
** Writes a sequence in GCG format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void seqWriteGcg(AjPSeqout outseq)
{

    ajuint ilen;
    char ctype = 'N';
    ajuint check;
    SeqPSeqFormat sf = NULL;

    ilen = ajStrGetLen(outseq->Seq);

    if(!outseq->Type)
	ajFmtPrintF(outseq->File, "!!NA_SEQUENCE 1.0\n\n");
    else if(ajStrMatchC(outseq->Type, "P"))
    {
	ajFmtPrintF(outseq->File, "!!AA_SEQUENCE 1.0\n\n");
	ctype = 'P';
    }
    else
	ajFmtPrintF(outseq->File, "!!NA_SEQUENCE 1.0\n\n");

    ajSeqGapS(&outseq->Seq, '.');
    check = ajSeqoutGetCheckgcg(outseq);

    if(ajStrGetLen(outseq->Desc))
	ajFmtPrintF(outseq->File, "%S\n\n", outseq->Desc);

    ajFmtPrintF(outseq->File,
		"%S  Length: %d  Type: %c  Check: %4d ..\n",
		outseq->Name, ilen, ctype, check);

    if(sf)
	seqSeqFormat(ajStrGetLen(outseq->Seq), &sf);
    else
    {
	seqSeqFormat(ajStrGetLen(outseq->Seq), &sf);
	sf->spacer = 11;
	sf->numleft = ajTrue;
	sf->skipbefore = ajTrue;
	strcpy(sf->endstr, "\n");   /* to help with misreads at EOF */
    }

    seqWriteSeq(outseq, sf);
    seqFormatDel(&sf);

    return;
}




/* @funcstatic seqWriteStaden *************************************************
**
** Writes a sequence in Staden format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void seqWriteStaden(AjPSeqout outseq)
{
    static SeqPSeqFormat sf = NULL;

    ajFmtPrintF(outseq->File, "<%S---->\n", outseq->Name);
    seqSeqFormat(ajStrGetLen(outseq->Seq), &sf);

    sf->width = 60;
    seqWriteSeq(outseq, sf);
    seqFormatDel(&sf);

    return;
}




/* @funcstatic seqWriteText ***************************************************
**
** Writes a sequence in plain Text format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void seqWriteText(AjPSeqout outseq)
{
    static SeqPSeqFormat sf = NULL;

    seqSeqFormat(ajStrGetLen(outseq->Seq), &sf);

    seqWriteSeq(outseq, sf);
    seqFormatDel(&sf);

    return;
}




/* @funcstatic seqWriteHennig86 ***********************************************
**
** Writes a sequence in Hennig86 format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void seqWriteHennig86(AjPSeqout outseq)
{
    ajulong isize;
    ajuint ilen = 0;
    ajulong i    = 0UL;
    void** seqs = NULL;
    AjPSeq seq;
    AjPSeq* seqarr;
    ajulong itest;
    AjPStr sseq = NULL;
    char* cp;
    
    ajDebug("seqWriteHennig86 list size %Lu\n",
	    ajListGetLength(outseq->Savelist));
    
    isize = ajListGetLength(outseq->Savelist);

    if(!isize)
	return;
    
    itest = ajListToarray(outseq->Savelist, (void***) &seqs);
    ajDebug("ajListToarray listed %Lu items\n", itest);
    seqarr = (AjPSeq*) seqs;

    for(i=0UL; i < isize; i++)
    {
	seq = seqarr[i];

	if(ilen < ajSeqGetLen(seq))
	    ilen = ajSeqGetLen(seq);
    }
    
    ajFmtPrintF(outseq->File,		/* header text */
		"xread\n");
    
    ajFmtPrintF(outseq->File,		/* title text */
		"' Written by EMBOSS %D '\n", ajTimeRefToday());
    
    ajFmtPrintF(outseq->File,		/* length, count */
		"%u %Lu\n", ilen, isize);
    
    for(i=0UL; i < isize; i++)
    {
	/* loop over sequences */
	seq = seqarr[i];
	ajStrAssignS(&sseq, seq->Seq);
	
	cp = ajStrGetuniquePtr(&sseq);

	while(*cp)
	{
	    switch(*cp)
	    {
                case 'A':
                case 'a':
                    *cp = '0';
                    break;
                case 'T':
                case 't':
                case 'U':
                case 'u':
                    *cp = '1';
                    break;
                case 'G':
                case 'g':
                    *cp = '2';
                    break;
                case 'C':
                case 'c':
                    *cp = '3';
                    break;
                default:
                    *cp = '?';
                    break;
	    }
	    cp++;
	}

	ajWritelineNewline(outseq->File, seq->Name);
	ajWritelineNewline(outseq->File, sseq);
    }

    ajWritebinByte(outseq->File, ';');
    ajWritebinNewline(outseq->File);

    ajStrDel(&sseq);
    AJFREE(seqs);

    return;
}




/* @funcstatic seqWriteMega ***************************************************
**
** Writes a sequence in Mega format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void seqWriteMega(AjPSeqout outseq)
{
    ajulong isize;
    ajuint ilen = 0;
    ajulong i    = 0;
    void** seqs = NULL;
    const AjPSeq seq;
    const AjPSeq seqfirst;
    AjPSeq* seqarr;
    ajulong itest;
    ajuint namewidth;
    AjPStr sseq = NULL;
    AjPStr sseqfirst = NULL;
    ajuint ipos;
    ajuint iend;
    ajuint wid = 50;
    AjBool onedesc = ajTrue;
    AjBool onegene = ajTrue;
    AjPSeqGene gene = NULL;
    AjPStr genestr = NULL;

    isize = ajListGetLength(outseq->Savelist);

    if(!isize)
	return;

/* test for group description only when writing 2+ sequences (a set) */
    
    itest = ajListToarray(outseq->Savelist, (void***) &seqs);
    ajDebug("ajListToarray listed %Lu items\n", itest);
    seqarr = (AjPSeq*) seqs;

    seqfirst = seqarr[0];

    namewidth = ajStrGetLen(seqfirst->Name);

    if(isize == 1)
    {
        onegene = ajFalse;
        onedesc = ajFalse;
    }
    else
    {
        onegene = ajTrue;
        onedesc = ajTrue;
        
        ilen = ajSeqGetLen(seqfirst);

        if(!ajStrGetLen(seqfirst->Desc))
            onedesc = ajFalse;

        if(ajListPeek(seqfirst->Genelist, (void*) &gene))
            ajStrAssignS(&genestr, gene->Name);
        else
            onegene = ajFalse;

        for(i=1UL; i < isize; i++)
        {
            seq = seqarr[i];

	    if(ajStrGetLen(seq->Name) > namewidth)
	      namewidth = ajStrGetLen(seq->Name);
	    if(ilen < ajSeqGetLen(seq))
                ilen = ajSeqGetLen(seq);
            if(onedesc && !ajStrMatchS(seq->Desc, seqfirst->Desc))
                onedesc = ajFalse;
            if(onegene && ajListPeek(seq->Genelist, (void*) &gene))
            {
                if(!ajStrMatchS(genestr, gene->Name))
                    onegene = ajFalse;
            }
        }
    }

    if(namewidth > 40)
      namewidth = 40;

    if(outseq->Count == (ajint) isize)
    {
      ajFmtPrintF(outseq->File,		/* header text */
		  "#mega\n");
      if(onedesc)
        ajFmtPrintF(outseq->File,		/* dummy title */
                    "!Title: %S;\n", seqfirst->Desc);
      else
        ajFmtPrintF(outseq->File,		/* dummy title */
                    "!Title: Written by EMBOSS %D;\n", ajTimeRefToday());

      ajFmtPrintF(outseq->File,"!Format\n");
      if(ajSeqIsProt(seqfirst))
        ajFmtPrintF(outseq->File,
                    "    DataType=Protein DataFormat=Interleaved\n");
      else
        ajFmtPrintF(outseq->File,
                    "    DataType=Nucleotide DataFormat=Interleaved\n");

      /*
      ** this is sensible for one set of sequences,
      ** but multiple sequence sets cause problems when it appears on later sets
      */
    
      /*ajFmtPrintF(outseq->File,
	"    NSeqs=%Lu NSites=%u\n", isize, ilen);*/

      ajFmtPrintF(outseq->File,
		  "    Identical=. Indel=- Missing=?\n");
      if(!ajSeqIsProt(seqfirst))
        ajFmtPrintF(outseq->File,
                    "    CodeTable=Standard\n");
      ajFmtPrintF(outseq->File,
		  "    ;\n\n");
    }

    ajWritebinNewline(outseq->File);
    ajWritebinNewline(outseq->File);

    if(onegene)
        ajFmtPrintF(outseq->File,
                    "!Gene=%S;\n", genestr);

    for(ipos=1; ipos <= ilen; ipos += wid)
    {
	/* interleaved */
	iend = ipos + wid -1;

	if(iend > ilen)
	    iend = ilen;

        ajStrAssignSubS(&sseqfirst, seqfirst->Seq, ipos-1, iend-1);
        ajSeqGapS(&sseqfirst, '-');

        ajWritebinNewline(outseq->File); /* blank space for comments */

	for(i=0; i < isize; i++)
	{
	    /* loop over sequences */
	    seq = seqarr[i];
	    ajStrAssignSubS(&sseq, seq->Seq, ipos-1, iend-1);
	    ajSeqGapS(&sseq, '-');
            if(i)
                ajStrMaskIdent(&sseq, sseqfirst, '.');
            if(!onedesc && ipos == 1 && ajStrGetLen(seq->Desc))
                ajFmtPrintF(outseq->File, "[%S]\n", seq->Desc);
	    ajFmtPrintF(outseq->File, "#%-*.*S %S\n",
			namewidth, namewidth, seq->Name, sseq);
	}
    }

    ajWritebinNewline(outseq->File);

    ajStrDel(&genestr);
    ajStrDel(&sseq);
    ajStrDel(&sseqfirst);
    AJFREE(seqs);

    return;
}




/* @funcstatic seqWriteMeganon ************************************************
**
** Writes a sequence in Mega non-interleaved format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void seqWriteMeganon(AjPSeqout outseq)
{
    ajulong isize;
    ajuint ilen = 0;
    ajulong i    = 0;
    void** seqs = NULL;
    const AjPSeq seq;
    const AjPSeq seqfirst;
    AjPSeq* seqarr;
    ajulong itest; 
    ajuint namewidth;
    AjPStr sseq = NULL;
    AjPStr sseqfirst = NULL;
    AjBool onedesc = ajTrue;
    AjBool onegene = ajTrue;
    AjPSeqGene gene = NULL;
    AjPStr genestr = NULL;

    ajDebug("seqWriteMeganon list size %Lu\n",
            ajListGetLength(outseq->Savelist));

    isize = ajListGetLength(outseq->Savelist);

    if(!isize)
	return;

    itest = ajListToarray(outseq->Savelist, (void***) &seqs);
    ajDebug("ajListToarray listed %Lu items\n", itest);
    seqarr = (AjPSeq*) seqs;

    seqfirst = seqarr[0];

    namewidth = ajStrGetLen(seqfirst->Name);

    if(isize == 1)
    {
        onegene = ajFalse;
        onedesc = ajFalse;
    }
    else
    {
        onegene = ajTrue;
        onedesc = ajTrue;
        
        ilen = ajSeqGetLen(seqfirst);

        if(!ajStrGetLen(seqfirst->Desc))
            onedesc = ajFalse;

        if(ajListPeek(seqfirst->Genelist, (void*) &gene))
            ajStrAssignS(&genestr, gene->Name);
        else
            onegene = ajFalse;

        for(i=1UL; i < isize; i++)
        {
            seq = seqarr[i];

	    if(ajStrGetLen(seq->Name) > namewidth)
	      namewidth = ajStrGetLen(seq->Name);

            if(ilen < ajSeqGetLen(seq))
                ilen = ajSeqGetLen(seq);
            if(onedesc && !ajStrMatchS(seq->Desc, seqfirst->Desc))
                onedesc = ajFalse;
            if(onegene && ajListPeek(seq->Genelist, (void*) &gene))
            {
                if(!ajStrMatchS(genestr, gene->Name))
                    onegene = ajFalse;
            }
        }
    }
    
    if(namewidth > 40)
      namewidth = 40;

    for(i=0UL; i < isize; i++)
    {
	seq = seqarr[i];

	if(ilen < ajSeqGetLen(seq))
	    ilen = ajSeqGetLen(seq);
        if(!ajStrMatchS(seq->Desc, seqfirst->Desc))
            onedesc = ajFalse;
    }

    if(outseq->Count == (ajint) isize)
    {
      ajFmtPrintF(outseq->File,		/* header text */
		  "#mega\n");
      if(onedesc)
        ajFmtPrintF(outseq->File,		/* dummy title */
                    "!Title: %S;\n", seqfirst->Desc);
      else
        ajFmtPrintF(outseq->File,		/* dummy title */
                    "!Title: Written by EMBOSS %D;\n", ajTimeRefToday());

      ajFmtPrintF(outseq->File,"!Format\n");
      if(ajSeqIsProt(seqfirst))
        ajFmtPrintF(outseq->File,
                    "    DataType=Protein\n");
      else
        ajFmtPrintF(outseq->File,
                    "    DataType=Nucleotide\n");
      /*ajFmtPrintF(outseq->File,
	"    NSeqs=%Lu NSites=%u\n", isize, ilen);*/
      ajFmtPrintF(outseq->File,
		  "    Identical=. Indel=- Missing=?\n");
      if(!ajSeqIsProt(seqfirst))
        ajFmtPrintF(outseq->File,
                    "    CodeTable=Standard\n");
      ajFmtPrintF(outseq->File,
		  "    ;\n\n");
    }

    ajStrAssignS(&sseqfirst, seqfirst->Seq);
    ajSeqGapS(&sseqfirst, '-');

    if(onegene)
        ajFmtPrintF(outseq->File,
                    "!Gene=%S;\n", genestr);

    for(i=0; i < isize; i++)
    {					/* loop over sequences */
	seq = seqarr[i];
	ajStrAssignS(&sseq, seq->Seq);
	ajSeqGapS(&sseq, '-');
        if(i)
            ajStrMaskIdent(&sseq, sseqfirst, '.');
        if(!onedesc && ajStrGetLen(seq->Desc))
           ajFmtPrintF(outseq->File, "[%S]\n", seq->Desc);
        ajFmtPrintF(outseq->File,
		    "#%-*.*S\n%S\n",
		    namewidth, namewidth, seq->Name, sseq);
    }

    ajStrDel(&genestr);
    ajStrDel(&sseq);
    ajStrDel(&sseqfirst);
    AJFREE(seqs);

    return;
}




/* @funcstatic seqWriteNexus **************************************************
**
** Writes a sequence in Nexus interleaved format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void seqWriteNexus(AjPSeqout outseq)
{
    ajulong isize;
    ajuint ilen = 0;
    ajulong i    = 0;
    void** seqs = NULL;
    AjPSeq seq;
    AjPSeq* seqarr;
    ajulong itest;
    AjPStr sseq = NULL;
    ajuint ipos;
    ajuint iend;
    ajuint wid = 50;
    
    ajDebug("seqWriteNexus list size %Lu\n",
	    ajListGetLength(outseq->Savelist));
    
    isize = ajListGetLength(outseq->Savelist);

    if(!isize)
	return;
    
    itest = ajListToarray(outseq->Savelist, (void***) &seqs);
    ajDebug("ajListToarray listed %Lu items\n", itest);
    seqarr = (AjPSeq*) seqs;

    for(i=0UL; i < isize; i++)
    {
	seq = seqarr[i];

	if(ilen < ajSeqGetLen(seq))
	    ilen = ajSeqGetLen(seq);
    }
    
    for(i=0UL; i < isize; i++)
    {
	seq = seqarr[i];
	ajSeqGapLen(seq, '-', '-', ilen); /* need to pad if any are shorter */
    }
    
    ajFmtPrintF(outseq->File,		/* header text */
		"#NEXUS\n");
    ajFmtPrintF(outseq->File,		/* dummy title */
		"[TITLE: Written by EMBOSS %D]\n\n", ajTimeRefToday());
    ajFmtPrintF(outseq->File,
		"begin data;\n");
    ajFmtPrintF(outseq->File,		/* count, length */
		"dimensions ntax=%Lu nchar=%u;\n", isize, ilen);
    ajDebug("seqWriteNexus outseq->Type '%S'\n", outseq->Type);

    if(ajStrMatchC(outseq->Type, "P"))
	ajFmtPrintF(outseq->File,
		    "format interleave datatype=protein missing=X gap=-;\n");
    else
	ajFmtPrintF(outseq->File,
		    "format interleave datatype=DNA missing=N gap=-;\n");

    ajWritebinNewline(outseq->File);
    
    ajFmtPrintF(outseq->File,
		"matrix\n");
    for(ipos=1; ipos <= ilen; ipos += wid)
    {					/* interleaved */
	iend = ipos +wid -1;

	if(iend > ilen)
	    iend = ilen;

	if(ipos > 1)
	    ajWritebinNewline(outseq->File);

	for(i=0; i < isize; i++)
	{				/* loop over sequences */
	    seq = seqarr[i];
	    ajStrAssignSubS(&sseq, seq->Seq, ipos-1, iend-1);
	    ajSeqGapS(&sseq, '-');
	    ajFmtPrintF(outseq->File,
			"%-20.20S %S\n",
			seq->Name, sseq);
	}
    }
    
    ajFmtPrintF(outseq->File,
		";\n\n");
    ajFmtPrintF(outseq->File,
		"end;\n");
    ajFmtPrintF(outseq->File,
		"begin assumptions;\n");
    ajFmtPrintF(outseq->File,
		"options deftype=unord;\n");
    ajFmtPrintF(outseq->File,
		"end;\n");

    ajStrDel(&sseq);
    AJFREE(seqs);

    return;
}




/* @funcstatic seqWriteNexusnon ***********************************************
**
** Writes a sequence in Nexus non-interleaved format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void seqWriteNexusnon(AjPSeqout outseq)
{
    ajulong isize;
    ajuint ilen = 0;
    ajulong i    = 0;
    void** seqs = NULL;
    AjPSeq seq;
    AjPSeq* seqarr;
    ajulong itest;
    AjPStr sseq = NULL;

    ajDebug("seqWriteNexusnon list size %Lu\n",
	    ajListGetLength(outseq->Savelist));

    isize = ajListGetLength(outseq->Savelist);

    if(!isize)
	return;

    itest = ajListToarray(outseq->Savelist, (void***) &seqs);
    ajDebug("ajListToarray listed %Lu items\n", itest);
    seqarr = (AjPSeq*) seqs;

    for(i=0UL; i < isize; i++)
    {
	seq = seqarr[i];

	if(ilen < ajSeqGetLen(seq))
	    ilen = ajSeqGetLen(seq);
    }

    ajFmtPrintF(outseq->File,		/* header text */
		"#NEXUS\n");
    ajFmtPrintF(outseq->File,		/* dummy title */
		"[TITLE: Written by EMBOSS %D]\n\n", ajTimeRefToday());
    ajFmtPrintF(outseq->File,
		"begin data;\n");
    ajFmtPrintF(outseq->File,		/* count, length */
		"dimensions ntax=%Lu nchar=%u;\n", isize, ilen);

    if(ajStrMatchC(outseq->Type, "P"))
	ajFmtPrintF(outseq->File,
		    "format datatype=protein missing=X gap=-;\n");
    else
	ajFmtPrintF(outseq->File,
		    "format datatype=DNA missing=N gap=-;\n");

    ajWritebinNewline(outseq->File);

    ajFmtPrintF(outseq->File,
		"matrix\n");

    for(i=0UL; i < isize; i++)
    {
	/* loop over sequences */
	seq = seqarr[i];
	ajStrAssignS(&sseq, seq->Seq);
	ajSeqGapS(&sseq, '-');
	ajFmtPrintF(outseq->File,
		    "%S\n%S\n",
		    seq->Name, sseq);
    }

    ajFmtPrintF(outseq->File,
		";\n\n");
    ajFmtPrintF(outseq->File,
		"end;\n");
    ajFmtPrintF(outseq->File,
		"begin assumptions;\n");
    ajFmtPrintF(outseq->File,
		"options deftype=unord;\n");
    ajFmtPrintF(outseq->File,
		"end;\n");

    ajStrDel(&sseq);
    AJFREE(seqs);

    return;
}




/* @funcstatic seqWriteJackknifer *********************************************
**
** Writes a sequence in Jackknifer format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void seqWriteJackknifer(AjPSeqout outseq)
{
    ajulong isize;
    ajuint ilen = 0;
    ajulong i    = 0;
    void** seqs = NULL;
    AjPSeq seq;
    AjPSeq* seqarr;
    ajulong itest;
    AjPStr sseq = NULL;
    ajuint ipos;
    ajuint iend;
    ajuint wid = 50;
    AjPStr tmpid = NULL;

    ajDebug("seqWriteJackknifer list size %Lu\n",
	    ajListGetLength(outseq->Savelist));

    isize = ajListGetLength(outseq->Savelist);

    if(!isize)
	return;

    itest = ajListToarray(outseq->Savelist, (void***) &seqs);
    ajDebug("ajListToarray listed %Lu items\n", itest);
    seqarr = (AjPSeq*) seqs;

    for(i=0UL; i < isize; i++)
    {
	seq = seqarr[i];

	if(ilen < ajSeqGetLen(seq))
	    ilen = ajSeqGetLen(seq);
    }

    ajFmtPrintF(outseq->File,		/* header text */
		"' Written by EMBOSS %D \n", ajTimeRefToday());

    for(ipos=1; ipos <= ilen; ipos += wid)
    {					/* interleaved */
	iend = ipos +wid -1;

	if(iend > ilen)
	    iend = ilen;

	for(i=0; i < isize; i++)
	{				/* loop over sequences */
	    seq = seqarr[i];
	    ajStrAssignSubS(&sseq, seq->Seq, ipos-1, iend-1);
	    ajSeqGapS(&sseq, '-');
	    ajFmtPrintS(&tmpid, "(%S)", seq->Name);
	    ajFmtPrintF(outseq->File,
			"%-20.20S %S\n",
			tmpid, sseq);
	}
    }

    ajFmtPrintF(outseq->File, ";\n");

    ajStrDel(&sseq);
    ajStrDel(&tmpid);
    AJFREE(seqs);

    return;
}




/* @funcstatic seqWriteJackknifernon ******************************************
**
** Writes a sequence in Jackknifer on-interleaved format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void seqWriteJackknifernon(AjPSeqout outseq)
{
    ajulong isize;
    ajuint ilen = 0;
    ajulong i    = 0;
    void** seqs = NULL;
    AjPSeq seq;
    AjPSeq* seqarr;
    ajulong itest;
    AjPStr sseq = NULL;
    ajuint ipos;
    ajuint iend;
    ajuint wid = 50;
    static AjPStr tmpid = NULL;

    ajDebug("seqWriteJackknifernon list size %Lu\n",
	    ajListGetLength(outseq->Savelist));

    isize = ajListGetLength(outseq->Savelist);

    if(!isize)
	return;

    itest = ajListToarray(outseq->Savelist, (void***) &seqs);
    ajDebug("ajListToarray listed %Lu items\n", itest);
    seqarr = (AjPSeq*) seqs;

    for(i=0UL; i < isize; i++)
    {
	seq = seqarr[i];

	if(ilen < ajSeqGetLen(seq))
	    ilen = ajSeqGetLen(seq);
    }

    ajFmtPrintF(outseq->File,		/* header text */
		"' Written by EMBOSS %D \n", ajTimeRefToday());

    for(i=0; i < isize; i++)
    {
	/* loop over sequences */
	seq = seqarr[i];

	for(ipos=1; ipos <= ilen; ipos += wid)
	{				/* interleaved */
	    iend = ipos +wid -1;

	    if(iend > ilen)
		iend = ilen;

	    ajStrAssignSubS(&sseq, seq->Seq, ipos-1, iend-1);
	    ajSeqGapS(&sseq, '-');

	    if(ipos == 1)
	    {
		ajFmtPrintS(&tmpid, "(%S)", seq->Name);
		ajFmtPrintF(outseq->File,
			    "%-20.20S %S\n",
			    tmpid, sseq);
	    }
	    else
		ajWritelineNewline(outseq->File, sseq);
	}
    }

    ajFmtPrintF(outseq->File, ";\n");

    ajStrDel(&sseq);
    AJFREE(seqs);

    return;
}




/* @funcstatic seqWriteTreecon ************************************************
**
** Writes a sequence in Treecon format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void seqWriteTreecon(AjPSeqout outseq)
{
    ajulong isize;
    ajuint ilen = 0;
    ajulong i    = 0;
    void** seqs = NULL;
    AjPSeq seq;
    AjPSeq* seqarr;
    ajulong itest;
    AjPStr sseq = NULL;

    ajDebug("seqWriteTreecon list size %Lu\n",
            ajListGetLength(outseq->Savelist));

    isize = ajListGetLength(outseq->Savelist);

    if(!isize)
	return;

    itest = ajListToarray(outseq->Savelist, (void***) &seqs);
    ajDebug("ajListToarray listed %Lu items\n", itest);
    seqarr = (AjPSeq*) seqs;

    for(i=0UL; i < isize; i++)
    {
	seq = seqarr[i];

	if(ilen < ajSeqGetLen(seq))
	    ilen = ajSeqGetLen(seq);
    }

    ajFmtPrintF(outseq->File,		/* count */
		"%d\n", ilen);

    for(i=0; i < isize; i++)
    {
	/* loop over sequences */
	seq = seqarr[i];
	ajStrAssignS(&sseq, seq->Seq);
	ajSeqGapS(&sseq, '-');
	ajFmtPrintF(outseq->File,
		    "%S\n%S\n",
		    seq->Name, sseq);
    }

    ajStrDel(&sseq);
    AJFREE(seqs);

    return;
}




/* @funcstatic seqWriteClustal ************************************************
**
** Writes a sequence in Clustal (ALN) format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void seqWriteClustal(AjPSeqout outseq)
{
    ajulong isize;
    ajuint ilen = 0;
    ajulong i    = 0;
    void** seqs = NULL;
    AjPSeq seq;
    AjPSeq* seqarr;
    ajulong itest;
    AjPStr sseq = NULL;
    ajuint ipos;
    ajuint iend;
    ajuint iwidth = 50;
    
    ajDebug("seqWriteClustal list size %Lu\n",
            ajListGetLength(outseq->Savelist));
    
    
    isize = ajListGetLength(outseq->Savelist);

    if(!isize)
	return;
    
    itest = ajListToarray(outseq->Savelist, (void***) &seqs);
    ajDebug("ajListToarray listed %Lu items\n", itest);
    seqarr = (AjPSeq*) seqs;

    for(i=0UL; i < isize; i++)
    {
	seq = seqarr[i];

	if(ilen < ajSeqGetLen(seq))
	    ilen = ajSeqGetLen(seq);
    }
    
    for(i=0UL; i < isize; i++)
    {
	seq = seqarr[i];

	if(ilen > ajSeqGetLen(seq))
	    ajSeqGapFill(seq, ilen);
    }
    
    ajFmtPrintF(outseq->File,
		"CLUSTAL W (1.83) multiple sequence alignment\n");
    
    ajFmtPrintF(outseq->File, "\n\n");
    
    iwidth = 60;

    for(ipos=1; ipos <= ilen; ipos += 60)
    {
	iend = ipos + 60 -1;

	if(iend > ilen)
	{
	    iend = ilen;
	    iwidth = ilen - ipos + 1;
	}

	for(i=0; i < isize; i++)
	{
	    seq = seqarr[i];
	    ajStrAssignSubS(&sseq, seq->Seq, ipos-1, iend-1);
	    ajSeqGapS(&sseq, '-');
	    /* clustalw no longer uses blocks of 10 - after version 1.4 */
	    /*ajStrFmtBlock(&sseq, 10);*/ 
	    ajFmtPrintF(outseq->File,
			"%-15.15S %S\n",
			seq->Name, sseq);
	}
	ajFmtPrintF(outseq->File,	/* *. conserved line */
		    "%-15.15s %*.*s\n", "", iwidth, iwidth, "");
	if(iend < ilen)
	    ajWritebinNewline(outseq->File);
    }

    ajStrDel(&sseq);
    AJFREE(seqs);

    return;
}




/* @funcstatic seqWriteSelex **************************************************
**
** Writes a sequence in Selex format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 2.0.1
** @@
******************************************************************************/

static void seqWriteSelex(AjPSeqout outseq)
{
    ajulong n;
    ajuint len = 0;
    ajulong i   = 0;
    ajuint j   = 0;
    
    AjPSeq seq   = NULL;
    AjPSeq* seqs = NULL;
    ajulong test;
/*
    ajuint k   = 0;
    ajuint namelen = 0;
    ajuint v       = 0;
    AjBool sep    = ajFalse;
*/
    AjPStr rfstr  = NULL;
    AjPStr csstr  = NULL;
    AjPStr ssstr  = NULL;
    const char *p       = NULL;
    AjPStr *names;
    ajuint  extra;
    ajuint  nlen   = 0;
    ajuint  slen   = 0;
    AjPStr *aseqs = NULL;
    
    ajDebug("seqWriteSelex list size %Lu\n",
	    ajListGetLength(outseq->Savelist));
    
    rfstr = ajStrNewC("#=RF");
    csstr = ajStrNewC("#=CS");
    ssstr = ajStrNewC("#=SS");
    
    n = ajListGetLength(outseq->Savelist);

    if(!n)
	return;
    
    test = ajListToarray(outseq->Savelist, (void***) &seqs);
    ajDebug("ajListToarray listed %Lu items\n", test);
    
    
    
    for(i=0UL; i < n; ++i)
    {
	seq = seqs[i];

	if(len < ajSeqGetLen(seq))
	    len = ajSeqGetLen(seq);
    }

    /*
    sdata = seqs[0]->Selexdata;
    if(sdata)
    {
	
	if(ajStrGetLen(sdata->id))
	{
	    sep=ajTrue;
	    ajFmtPrintF(outseq->File,"#=ID %S\n",sdata->id);
	}

	if(ajStrGetLen(sdata->ac))
	{
	    sep=ajTrue;
	    ajFmtPrintF(outseq->File,"#=AC %S\n",sdata->ac);
	}

	if(ajStrGetLen(sdata->de))
	{
	    sep=ajTrue;
	    ajFmtPrintF(outseq->File,"#=DE %S\n",sdata->de);
	}
	
	if(sdata->ga[0] || sdata->ga[1])
	{
	    sep=ajTrue;
	    ajFmtPrintF(outseq->File,"#=GA %.2f %.2f\n",sdata->ga[0],
			sdata->ga[1]);
	}
	
	if(sdata->tc[0] || sdata->tc[1])
	{
	    sep=ajTrue;
	    ajFmtPrintF(outseq->File,"#=TC %.2f %.2f\n",sdata->tc[0],
			sdata->tc[1]);
	}
	
	if(sdata->nc[0] || sdata->nc[1])
	{
	    sep=ajTrue;
	    ajFmtPrintF(outseq->File,"#=NC %.2f %.2f\n",sdata->nc[0],
			sdata->nc[1]);
	}
	
	if(ajStrGetLen(sdata->au))
	{
	    sep=ajTrue;
	    ajFmtPrintF(outseq->File,"#=AU %S\n",sdata->au);
	}
	
	if(sep)
	    ajFmtPrintF(outseq->File,"\n");
	
	
	v=4;
	for(i=0;i<n;++i)
	{
	    v = ajStrGetLen(seqs[i]->Selexdata->sq->name);
	    namelen = (namelen > v) ? namelen : v;
	}

	for(i=0UL;i<n;++i)
	{
	    v = namelen - ajStrGetLen(seqs[i]->Selexdata->sq->name);
	    for(j=0;j<v;++j)
		ajStrAppendK(&seqs[i]->Selexdata->sq->name,' ');
	}
	
	
	if(ajStrGetLen(sdata->sq->ac))
	    for(i=0UL;i<n;++i)
	    {
		qdata = seqs[i]->Selexdata->sq;
		ajFmtPrintF(outseq->File,"#=SQ %S %.2f %S %S %d..%d:%d %S\n",
			    qdata->name,qdata->wt,qdata->source,qdata->ac,
			    qdata->start,qdata->stop,qdata->len,qdata->de);
	    }
	ajFmtPrintF(outseq->File,"\n");
	
	
	
	if(ajStrGetLen(seqs[0]->Selexdata->rf))
	{
	    v = namelen - 4;
	    for(k=0;k<v;++k)
		ajStrAppendK(&rfstr,' ');
	}

	if(ajStrGetLen(seqs[0]->Selexdata->cs))
	{
	    v = namelen - 4;
	    for(k=0;k<v;++k)
		ajStrAppendK(&csstr,' ');
	}
	if(ajStrGetLen(seqs[0]->Selexdata->ss))
	{
	    v = namelen - 4;
	    for(k=0;k<v;++k)
		ajStrAppendK(&ssstr,' ');
	}
	
	
	
	for(i=0UL;i<len;i+=50)
	{
	    if(ajStrGetLen(seqs[0]->Selexdata->rf))
	    {
		p = ajStrGetPtr(seqs[0]->Selexdata->rf);
		if(i+50>=len)
		    ajFmtPrintF(outseq->File,"%S %s\n",rfstr, &p[i]);
		else
		    ajFmtPrintF(outseq->File,"%S %-50.50s\n",rfstr,
				&p[i]);
	    }

	    if(ajStrGetLen(seqs[0]->Selexdata->cs))
	    {
		p = ajStrGetPtr(seqs[0]->Selexdata->cs);
		if(i+50>=len)
		    ajFmtPrintF(outseq->File,"%S %s\n",csstr,&p[i]);
		else
		    ajFmtPrintF(outseq->File,"%S %-50.50s\n",csstr,
				&p[i]);
	    }


	    for(j=0;j<n;++j)
	    {
		sdata = seqs[j]->Selexdata;

		p = ajStrGetPtr(sdata->str);
		if(i+50>=len)
		    ajFmtPrintF(outseq->File,"%S %s\n",sdata->sq->name,&p[i]);
		else
		    ajFmtPrintF(outseq->File,"%S %-50.50s\n",sdata->sq->name,
				&p[i]);

		if(ajStrGetLen(seqs[0]->Selexdata->ss))
		{
		    p = ajStrGetPtr(seqs[0]->Selexdata->ss);
		    if(i+50>=len)
			ajFmtPrintF(outseq->File,"%S %s\n",ssstr,&p[i]);
		    else
			ajFmtPrintF(outseq->File,"%S %-50.50s\n",ssstr,
				    &p[i]);
		}

	    }

	    if(i+50<len)
		ajFmtPrintF(outseq->File,"\n");
	}
    }
    else	/ * Wasn't originally Selex format * /
    {
*/

    AJCNEW0(aseqs,n);
    AJCNEW0(names,n);

    for(i=0UL; i < n; ++i)
    {
        seq = seqs[i];
        aseqs[i] = ajStrNew();
        names[i] = ajStrNew();
        ajStrAssignS(&names[i],seq->Name);

        if((len=ajStrGetLen(names[i])) > nlen)
            nlen = len;

        if((len=ajStrGetLen(seq->Seq)) > slen)
            slen = len;

        ajStrAssignS(&aseqs[i],seq->Seq);
    }

    for(i=0UL;i<n;++i)
    {
        seq = seqs[i];
        extra = nlen - ajStrGetLen(names[i]);

        for(j=0;j<extra;++j)
            ajStrAppendK(&names[i],' ');

        extra = slen - ajStrGetLen(seq->Seq);

        for(j=0;j<extra;++j)
            ajStrAppendK(&aseqs[i],' ');

        ajFmtPrintF(outseq->File,"#=SQ %S %.2f - - 0..0:0 ",
                    names[i],seq->Weight);
        if(ajStrGetLen(seq->Desc))
            ajWritelineNewline(outseq->File, seq->Desc);
        else
            ajFmtPrintF(outseq->File,"-\n");
    }

    ajWritebinNewline(outseq->File);


    for(i=0UL;i<slen;i+=50)
    {
        for(j=0;j<n;++j)
        {
            p = ajStrGetPtr(aseqs[j]);

            if(i+50>=len)
                ajFmtPrintF(outseq->File,"%S %s\n",names[j],&p[i]);
            else
                ajFmtPrintF(outseq->File,"%S %-50.50s\n",names[j],
                            &p[i]);
        }
        if(i+50<len)
            ajWritebinNewline(outseq->File);

    }

    for(i=0UL;i<n;++i)
    {
        ajStrDel(&names[i]);
        ajStrDel(&aseqs[i]);
    }
    AJFREE(names);
    AJFREE(aseqs);

/*
    }
  */  

    AJFREE(seqs);
    
    ajStrDel(&rfstr);
    ajStrDel(&csstr);
    ajStrDel(&ssstr);
    
    return;
}




/* @funcstatic seqWriteMsf ****************************************************
**
** Writes a sequence in GCG Multiple Sequence File format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void seqWriteMsf(AjPSeqout outseq)
{
    ajulong isize;
    ajuint ilen = 0;
    ajulong i    = 0;
    void** seqs = NULL;
    AjPSeq seq;
    AjPSeq* seqarr;
    ajuint checktot = 0;
    ajuint check;
    ajulong itest;
    AjPStr sqbeg = NULL;
    AjPStr sqend = NULL;
    AjPStr sseq = NULL;
    ajuint ipos;
    ajuint iend;
    ajuint igap;
    ajuint maxnamelen = 10;
    
    ajDebug("seqWriteMsf list size %Lu\n",
            ajListGetLength(outseq->Savelist));
    
    isize = ajListGetLength(outseq->Savelist);

    if(!isize)
	return;
    
    itest = ajListToarray(outseq->Savelist, (void***) &seqs);
    
    ajDebug("ajListToarray listed %Lu items\n", itest);
    seqarr = (AjPSeq*) seqs;
    maxnamelen = 10;

    for(i=0UL; i < isize; i++)
    {
	seq = seqarr[i];

	if(ilen < ajSeqGetLen(seq))
	    ilen = ajSeqGetLen(seq);

	if (ajStrGetLen(seq->Name) > maxnamelen)
	    maxnamelen = ajStrGetLen(seq->Name);
    }
    
    for(i=0UL; i < isize; i++)
    {
	seq = seqarr[i];
	ajSeqGapLen(seq, '.', '~', ilen); /* need to pad if any are shorter */
	check = ajSeqCalcCheckgcg(seq);
	ajDebug(" '%S' len: %d checksum: %d\n",
		ajSeqGetNameS(seq), ajSeqGetLen(seq), check);
	checktot += check;
	checktot = checktot % 10000;
    }
    
    ajDebug("checksum %d\n", checktot);
    ajDebug("outseq->Type '%S'\n", outseq->Type);
    
    if(!ajStrGetLen(outseq->Type))
    {
	ajSeqType(seqarr[0]);
	ajStrAssignEmptyS(&outseq->Type, seqarr[0]->Type);
    }
    ajDebug("outseq->Type '%S'\n", outseq->Type);
    
    if(ajStrMatchC(outseq->Type, "P"))
    {
	ajFmtPrintF(outseq->File, "!!AA_MULTIPLE_ALIGNMENT 1.0\n\n");
	ajFmtPrintF(outseq->File,
		    "  %F MSF:  %d Type: P %D CompCheck: %4d ..\n\n",
		    outseq->File, ilen, ajTimeRefToday(), checktot);
    }
    else
    {
	ajFmtPrintF(outseq->File, "!!NA_MULTIPLE_ALIGNMENT 1.0\n\n");
	ajFmtPrintF(outseq->File,
		    "  %F MSF: %d Type: N %D CompCheck: %4d ..\n\n",
		    outseq->File, ilen, ajTimeRefToday(), checktot);
    }
    
    for(i=0UL; i < isize; i++)
    {
	seq = seqarr[i];
	check = ajSeqCalcCheckgcg(seq);
	ajFmtPrintF(outseq->File,
		    "  Name: %-*S Len: %d  Check: %4d Weight: %.2f\n",
		    maxnamelen, seq->Name, ajStrGetLen(seq->Seq),
		    check, seq->Weight);
    }
    
    ajFmtPrintF(outseq->File, "\n//\n\n");
    
    for(ipos=1; ipos <= ilen; ipos += 50)
    {
	iend = ipos + 50 -1;
	if(iend > ilen)
	    iend = ilen;
	ajFmtPrintS(&sqbeg, "%d", ipos);
	ajFmtPrintS(&sqend, "%d", iend);

	if(iend == ilen)
        {
	    igap = iend - ipos - ajStrGetLen(sqbeg);
	    ajDebug("sqbeg: %S sqend: %S ipos: %d iend: %d igap: %d len: %d\n",
		    sqbeg, sqend, ipos, iend, igap, ajStrGetLen(sqend));

	    if(igap >= ajStrGetLen(sqend))
		ajFmtPrintF(outseq->File,
			    "%*s %S %*S\n", maxnamelen, " ", sqbeg, igap, sqend);
	    else
		ajFmtPrintF(outseq->File, "           %S\n", sqbeg);
	}
	else
	    ajFmtPrintF(outseq->File, "           %-25S%25S\n",
			sqbeg, sqend);

	for(i=0UL; i < isize; i++)
	{
	    seq = seqarr[i];
	    check = ajSeqCalcCheckgcg(seq);
	    ajStrAssignSubS(&sseq, seq->Seq, ipos-1, iend-1);
	    ajFmtPrintF(outseq->File,
			"%-*S %S\n",
			maxnamelen, seq->Name, sseq);
	}
	ajWritebinNewline(outseq->File);
    }
    
    
    /* AJB: Shouldn't this be left to ajSeqoutDel? */
    while(ajListPop(outseq->Savelist,(void **)&seq))
	ajSeqDel(&seq);
    ajListFree(&outseq->Savelist);
    
    ajStrDel(&sqbeg);
    ajStrDel(&sqend);
    ajStrDel(&sseq);
    AJFREE(seqs);
    
    return;
}




/* @funcstatic seqWriteCodata *************************************************
**
** Writes a sequence in Codata format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void seqWriteCodata(AjPSeqout outseq)
{

    static SeqPSeqFormat sf = NULL;
    ajuint j;

    ajFmtPrintF(outseq->File, "ENTRY           %S \n", outseq->Name);

    if(ajStrGetLen(outseq->Desc))
	ajFmtPrintF(outseq->File, "TITLE           %S, %d bases\n",
		    outseq->Desc, ajStrGetLen(outseq->Seq));

    if(ajStrGetLen(outseq->Acc))
	ajFmtPrintF(outseq->File, "ACCESSION       %S\n",
		    outseq->Acc);
    ajFmtPrintF(outseq->File, "SEQUENCE        \n");

    seqSeqFormat(ajStrGetLen(outseq->Seq), &sf);
    sf->numwidth = 7;
    sf->width = 30;
    sf->numleft = ajTrue;
    sf->spacer = seqSpaceAll;
    strcpy(sf->endstr, "\n///");

    for(j = 0; j <= sf->numwidth; j++)
	ajFmtPrintF(outseq->File, " ");

    for(j = 5; j <= sf->width; j+=5)
	ajFmtPrintF(outseq->File, "%10d", j);

    ajWritebinNewline(outseq->File);

    seqWriteSeq(outseq, sf);
    seqFormatDel(&sf);

    return;
}




/* @funcstatic seqWriteNbrf ***************************************************
**
** Writes a sequence in NBRF format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void seqWriteNbrf(AjPSeqout outseq)
{
    static SeqPSeqFormat sf = NULL;

    if(!outseq->Type)
	ajFmtPrintF(outseq->File, ">D1;%S\n", outseq->Name);
    else if(ajStrMatchC(outseq->Type, "P"))
	ajFmtPrintF(outseq->File, ">P1;%S\n", outseq->Name);
    else
	ajFmtPrintF(outseq->File, ">D1;%S\n", outseq->Name);

    ajFmtPrintF(outseq->File, "%S, %d bases\n",
		outseq->Desc, ajStrGetLen(outseq->Seq));

    if(seqoutUfoLocal(outseq))
    {
	ajFeattabOutDel(&outseq->Ftquery);
	outseq->Ftquery = ajFeattabOutNewCSF("pir", outseq->Name,
					     ajStrGetPtr(outseq->Type),
					     outseq->File);

	if(!ajFeattableWrite(outseq->Ftquery, outseq->Fttable))
	    ajWarn("seqWriteNbrf features output failed UFO: '%S'",
		   outseq->Ufo);
    }

    seqSeqFormat(ajStrGetLen(outseq->Seq), &sf);
    sf->spacer = 11;
    strcpy(sf->endstr, "*\n");
    seqWriteSeq(outseq, sf);
    seqFormatDel(&sf);

    return;
}




/* @funcstatic seqWriteExperiment *********************************************
**
** Writes a sequence in Staden experiment format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 3.0.0
** @@
******************************************************************************/

static void seqWriteExperiment(AjPSeqout outseq)
{
    static SeqPSeqFormat sf = NULL;
    ajuint b[5];
    AjIList it;
    AjPStr cur;
    ajuint ilen;
    ajuint i;
    ajuint j;
    ajuint jend;
    
    if(ajStrMatchC(outseq->Type, "P"))
    {
	seqWriteSwiss(outseq);

	return;
    }
    
    ajFmtPrintF(outseq->File,
		"ID   %-10S standard; DNA; UNC; %d BP.\n",
		outseq->Name, ajStrGetLen(outseq->Seq));
    
    if(ajListGetLength(outseq->Acclist))
    {
	ilen=0;
	it = ajListIterNewread(outseq->Acclist);

	while((cur = (AjPStr) ajListIterGet(it)))
	{
	    if(ilen + ajStrGetLen(cur) > 79)
	    {
		ajFmtPrintF(outseq->File, ";\n");
		ilen = 0;
	    }

	    if(ilen == 0)
	    {
		ajFmtPrintF(outseq->File, "AC   ");
		ilen = 6;
	    }
	    else
	    {
		ajFmtPrintF(outseq->File, "; ");
		ilen += 2;
	    }

	    ajWriteline(outseq->File, cur);
	    ilen += ajStrGetLen(cur);

	}

	ajListIterDel(&it) ;
	ajFmtPrintF(outseq->File, ";\n");
    }
    
    if(ajStrGetLen(outseq->Sv))
	ajFmtPrintF(outseq->File, "SV   %S\n", outseq->Sv);
    
    /* no need to bother with outseq->Gi because Staden doesn't use it */
    
    if(ajStrGetLen(outseq->Desc))
	ajFmtPrintF(outseq->File, "EX   %S\n", outseq->Desc);
    
    if(ajListGetLength(outseq->Keylist))
    {
	ilen=0;
	it = ajListIterNewread(outseq->Keylist);

	while((cur = (AjPStr) ajListIterGet(it)))
	{
	    if(ilen+ajStrGetLen(cur) >= 79)
	    {
		ajFmtPrintF(outseq->File, ";\n");
		ilen = 0;
	    }

	    if(ilen == 0)
	    {
		ajFmtPrintF(outseq->File, "KW   ");
		ilen = 6;
	    }
	    else
	    {
		ajFmtPrintF(outseq->File, "; ");
		ilen += 2;
	    }

	    ajWriteline(outseq->File, cur);
	    ilen += ajStrGetLen(cur);
	}

	ajListIterDel(&it) ;
	ajFmtPrintF(outseq->File, ".\n");
    }
    
    if(ajStrGetLen(outseq->Tax))
	ajFmtPrintF(outseq->File, "OS   %S\n", outseq->Tax);
    
    if(ajListGetLength(outseq->Taxlist))
    {
	ilen=0;
	it = ajListIterNewread(outseq->Taxlist);

	while((cur = (AjPStr) ajListIterGet(it)))
	{
	    if(ilen+ajStrGetLen(cur) >= 79)
	    {
		ajFmtPrintF(outseq->File, ";\n");
		ilen = 0;
	    }

	    if(ilen == 0)
	    {
		ajFmtPrintF(outseq->File, "OC   ");
		ilen = 6;
	    }
	    else
	    {
		ajFmtPrintF(outseq->File, "; ");
		ilen += 2;
	    }

	    ajWriteline(outseq->File, cur);
	    ilen += ajStrGetLen(cur);
	}

	ajListIterDel(&it) ;
	ajFmtPrintF(outseq->File, ".\n");
    }
    
    if(seqoutUfoLocal(outseq))
    {
	ajFeattabOutDel(&outseq->Ftquery);
        outseq->Ftquery = ajFeattabOutNewCSF("embl", outseq->Name,
					     ajStrGetPtr(outseq->Type),
					     outseq->File);
	if(!ajFeattableWrite(outseq->Ftquery, outseq->Fttable))
	    ajWarn("seqWriteEmbl features output failed UFO: '%S'",
		   outseq->Ufo);
    }
    

    if(outseq->Accuracy)
    {
	ilen = ajStrGetLen(outseq->Seq);

	for(i=0; i<ilen;i+=20)
	{
	    ajFmtPrintF(outseq->File, "AV  ");
	    jend = i+20;

	    if(jend > ilen)
		jend = ilen;

	    for(j=i;j<jend;j++)
		ajFmtPrintF(outseq->File, " %2d", (ajint) outseq->Accuracy[j]);

	    ajWritebinNewline(outseq->File);
	}
    }

    ajSeqoutGetBasecount(outseq, b);
    ajFmtPrintF(outseq->File,
		"SQ   Sequence %d BP; %d A; %d C; %d G; %d T; %d other;\n",
		ajStrGetLen(outseq->Seq), b[0], b[1], b[2], b[3], b[4]);
    
    seqSeqFormat(ajStrGetLen(outseq->Seq), &sf);
    strcpy(sf->endstr, "\n//");
    sf->tab = 4;
    sf->spacer = 11;
    sf->width = 60;
    sf->numright = ajTrue;
    sf->numwidth = 9;
    sf->numjust = ajTrue;
    
    seqWriteSeq(outseq, sf);
    seqFormatDel(&sf);

    return;
}




/* @funcstatic seqWriteEmbl ***************************************************
**
** Writes a sequence in EMBL format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void seqWriteEmbl(AjPSeqout outseq)
{
    static SeqPSeqFormat sf = NULL;
    ajuint b[5];
    AjIList it;
    AjPStr cur;
    ajuint ilen;
    AjPStr tmpstr = NULL;
    const AjPStr tmpline = NULL;

    if(ajStrMatchC(outseq->Type, "P"))
    {
	seqWriteSwiss(outseq);

	return;
    }
    
    ajFmtPrintF(outseq->File,
		"ID   %-10S standard; DNA; UNC; %d BP.\n",
		outseq->Name, ajStrGetLen(outseq->Seq));
    
    if(ajListGetLength(outseq->Acclist))
    {
	ilen=0;
	it = ajListIterNewread(outseq->Acclist);

	while((cur = (AjPStr) ajListIterGet(it)))
	{
	    if(ilen + ajStrGetLen(cur) > 79)
	    {
		ajFmtPrintF(outseq->File, ";\n");
		ilen = 0;
	    }

	    if(ilen == 0)
	    {
		ajFmtPrintF(outseq->File, "AC   ");
		ilen = 6;
	    }
	    else
	    {
		ajFmtPrintF(outseq->File, "; ");
		ilen += 2;
	    }

	    ajWriteline(outseq->File, cur);
	    ilen += ajStrGetLen(cur);

	}

	ajListIterDel(&it) ;
	ajFmtPrintF(outseq->File, ";\n");
    }
    
    if(ajStrGetLen(outseq->Sv))
	ajFmtPrintF(outseq->File, "SV   %S\n", outseq->Sv);
    
    /* no need to bother with outseq->Gi because EMBL doesn't use it */
    
    if(ajStrGetLen(outseq->Desc))
    {
	ajStrAssignS(&tmpstr,  outseq->Desc);
	ajStrFmtWrap(&tmpstr, 75);
	tmpline = ajStrParseC(tmpstr, "\n");

	while (tmpline)
	{
	    ajFmtPrintF(outseq->File, "DE   %S\n", tmpline);
	    tmpline = ajStrParseC(NULL, "\n");
	}
    }

    if(ajListGetLength(outseq->Keylist))
    {
	ilen=0;
	it = ajListIterNewread(outseq->Keylist);

	while((cur = (AjPStr) ajListIterGet(it)))
	{
	    if(ilen+ajStrGetLen(cur) >= 79)
	    {
		ajFmtPrintF(outseq->File, ";\n");
		ilen = 0;
	    }

	    if(ilen == 0)
	    {
		ajFmtPrintF(outseq->File, "KW   ");
		ilen = 6;
	    }
	    else
	    {
		ajFmtPrintF(outseq->File, "; ");
		ilen += 2;
	    }

	    ajWriteline(outseq->File, cur);
	    ilen += ajStrGetLen(cur);
	}

	ajListIterDel(&it) ;
	ajFmtPrintF(outseq->File, ".\n");
    }
    
    if(ajStrGetLen(outseq->Tax))
	ajFmtPrintF(outseq->File, "OS   %S\n", outseq->Tax);
    
    if(ajListGetLength(outseq->Taxlist) > 1)
    {
	ilen=0;
	it = ajListIterNewread(outseq->Taxlist);

	while((cur = (AjPStr) ajListIterGet(it)))
	{
	    if(ilen+ajStrGetLen(cur) >= 79)
	    {
		ajFmtPrintF(outseq->File, ";\n");
		ilen = 0;
	    }

	    if(ilen == 0)
	    {
		ajFmtPrintF(outseq->File, "OC   ");
		ilen = 6;
	    }
	    else
	    {
		ajFmtPrintF(outseq->File, "; ");
		ilen += 2;
	    }

	    ajWriteline(outseq->File, cur);
	    ilen += ajStrGetLen(cur);
	}

	ajListIterDel(&it) ;
	ajFmtPrintF(outseq->File, ".\n");
    }
    
    if(seqoutUfoLocal(outseq))
    {
	ajFeattabOutDel(&outseq->Ftquery);
        outseq->Ftquery = ajFeattabOutNewCSF("embl", outseq->Name,
					     ajStrGetPtr(outseq->Type),
					     outseq->File);
	if(!ajFeattableWrite(outseq->Ftquery, outseq->Fttable))
	    ajWarn("seqWriteEmbl features output failed UFO: '%S'",
		   outseq->Ufo);
    }
    
    ajSeqoutGetBasecount(outseq, b);
    ajFmtPrintF(outseq->File,
		"SQ   Sequence %d BP; %d A; %d C; %d G; %d T; %d other;\n",
		ajStrGetLen(outseq->Seq), b[0], b[1], b[2], b[3], b[4]);
    
    seqSeqFormat(ajStrGetLen(outseq->Seq), &sf);
    strcpy(sf->endstr, "\n//");
    sf->tab = 4;
    sf->spacer = 11;
    sf->width = 60;
    sf->numright = ajTrue;
    sf->numwidth = 9;
    sf->numjust = ajTrue;
    
    seqWriteSeq(outseq, sf);
    seqFormatDel(&sf);

    ajStrDel(&tmpstr);

    return;
}




/* @funcstatic seqWriteEmblnew ************************************************
**
** Writes a sequence in new EMBL format, introduced in EMBL release 87.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

static void seqWriteEmblnew(AjPSeqout outseq)
{
    static SeqPSeqFormat sf = NULL;
    ajuint b[5];
    AjIList it;
    const AjPStr cur;
    ajuint ilen;
    ajlong ifind;
    AjPStr idstr = NULL;
    AjPStr svstr = NULL;
    const AjPStr cmtstr = NULL;		/* from list - do not delete */
    AjPStr tmpstr = NULL;
    const AjPSeqRef seqref = NULL;
    const AjPSeqXref  xref = NULL;
    const AjPStr tmpline = NULL;

    if(ajStrMatchC(outseq->Type, "P"))
    {
	seqWriteSwiss(outseq);

	return;
    }
    
    if(ajStrGetLen(outseq->Sv))
    {
	ajStrAssignS(&svstr, outseq->Sv);
	ifind = ajStrFindC(svstr, ".");

	if(ifind >= 0)
	  ajStrCutStart(&svstr, (size_t) (ifind+1));
    }
    else
       ajStrAssignC(&svstr, "1");

    if(ajStrGetLen(outseq->Acc))
	ajStrAssignS(&idstr, outseq->Acc);
    else
	ajStrAssignS(&idstr, outseq->Name);

    ajFmtPrintF(outseq->File,
		"ID   %S; SV %S; %s;",
		idstr, svstr, outseq->Circular? "circular" : "linear");
 
    ajFmtPrintF(outseq->File, " %s;",ajSeqmolGetEmbl(outseq->Molecule));

   if(ajStrGetLen(outseq->Class))
	ajFmtPrintF(outseq->File, " %S;",outseq->Class);
    else
	ajFmtPrintF(outseq->File, " STD;");

   if(ajStrGetLen(outseq->Division))
	ajFmtPrintF(outseq->File, " %S;",outseq->Division);
    else
	ajFmtPrintF(outseq->File, " UNC;");

   ajFmtPrintF(outseq->File,
		" %d BP.\nXX\n",
		ajStrGetLen(outseq->Seq));
    ajStrDel(&svstr);
    
    if(ajListGetLength(outseq->Acclist))
    {
	ilen=0;
	it = ajListIterNewread(outseq->Acclist);

	while((cur = (AjPStr) ajListIterGet(it)))
	{
	    if(ilen + ajStrGetLen(cur) > 79)
	    {
		ajFmtPrintF(outseq->File, ";\n");
		ilen = 0;
	    }

	    if(ilen == 0)
	    {
		ajFmtPrintF(outseq->File, "AC   ");
		ilen = 6;
	    }
	    else
	    {
		ajFmtPrintF(outseq->File, "; ");
		ilen += 2;
	    }

	    ajWriteline(outseq->File, cur);
	    ilen += ajStrGetLen(cur);

	}

	ajListIterDel(&it) ;
	ajFmtPrintF(outseq->File, ";\nXX\n");
    }

    /* no SV line in the new format - see the ID line */
    /*
    if(ajStrGetLen(outseq->Sv))
	ajFmtPrintF(outseq->File, "SV   %S\n", outseq->Sv);
    */
    
    /* no need to bother with outseq->Gi because EMBL doesn't use it */


    if(outseq->Date)
    {
	if(outseq->Date->CreDate)
	    ajFmtPrintF(outseq->File,
			"DT   %D (Rel. %S, Created)\n",
			outseq->Date->CreDate, outseq->Date->CreRel);
	else if (outseq->Date->ModDate)
	    ajFmtPrintF(outseq->File,
			"DT   %D (Rel. %S, Created)\n",
			outseq->Date->ModDate, outseq->Date->ModRel);

	if(outseq->Date->ModDate)
	    ajFmtPrintF(outseq->File,
			"DT   %D (Rel. %S, Last updated, Version %S)\n",
			outseq->Date->ModDate, outseq->Date->ModRel,
			outseq->Date->ModVer);
 	ajFmtPrintF(outseq->File, "XX\n");
   }

    if(ajStrGetLen(outseq->Desc))
    {
	ajStrAssignS(&tmpstr,  outseq->Desc);
	ajStrFmtWrap(&tmpstr, 75);
	tmpline = ajStrParseC(tmpstr, "\n");

	while (tmpline)
	{
	    ajFmtPrintF(outseq->File, "DE   %S\n", tmpline);
	    tmpline = ajStrParseC(NULL, "\n");
	}

	ajFmtPrintF(outseq->File, "XX\n");
    }

    if(ajListGetLength(outseq->Keylist))
    {
	ilen=0;
	it = ajListIterNewread(outseq->Keylist);

	while((cur = (AjPStr) ajListIterGet(it)))
	{
	    if(ilen+ajStrGetLen(cur) >= 79)
	    {
		ajFmtPrintF(outseq->File, ";\n");
		ilen = 0;
	    }

	    if(ilen == 0)
	    {
		ajFmtPrintF(outseq->File, "KW   ");
		ilen = 6;
	    }
	    else
	    {
		ajFmtPrintF(outseq->File, "; ");
		ilen += 2;
	    }

	    ajWriteline(outseq->File, cur);
	    ilen += ajStrGetLen(cur);
	}

	ajListIterDel(&it) ;
	ajFmtPrintF(outseq->File, ".\nXX\n");
    }
    
    if(ajStrGetLen(outseq->Tax))
	ajFmtPrintF(outseq->File, "OS   %S\n", outseq->Tax);
    
    if(ajListGetLength(outseq->Taxlist) > 1)
    {
	ilen=0;
	it = ajListIterNewread(outseq->Taxlist);

	while((cur = (AjPStr) ajListIterGet(it)))
	{
	    if(ilen+ajStrGetLen(cur) >= 79)
	    {
		ajFmtPrintF(outseq->File, ";\n");
		ilen = 0;
	    }

	    if(ilen == 0)
	    {
		ajFmtPrintF(outseq->File, "OC   ");
		ilen = 6;
	    }
	    else
	    {
		ajFmtPrintF(outseq->File, "; ");
		ilen += 2;
	    }

	    ajWriteline(outseq->File, cur);
	    ilen += ajStrGetLen(cur);
	}

	ajListIterDel(&it) ;
	ajFmtPrintF(outseq->File, ".\n");
    }

    if(ajStrGetLen(outseq->Organelle))
	ajFmtPrintF(outseq->File, "OG   %S\n", outseq->Organelle);
    
    if(ajStrGetLen(outseq->Tax) ||
       ajStrGetLen(outseq->Organelle) ||
       ajListGetLength(outseq->Taxlist) > 1)
	ajFmtPrintF(outseq->File, "XX\n");
    
    if(ajListGetLength(outseq->Reflist))
    {
	it = ajListIterNewread(outseq->Reflist);

	while ((seqref = (const AjPSeqRef) ajListIterGet(it)))
	{
	    ajFmtPrintF(outseq->File, "RN   [%u]\n", seqref->Number);

	    if(ajStrGetLen(seqref->Comment))
	    {
		ajStrAssignS(&tmpstr, seqref->Comment);
		ajStrFmtWrap(&tmpstr, 75);
		tmpline = ajStrParseC(tmpstr, "\n");

		while (tmpline)
		{
		    ajFmtPrintF(outseq->File, "RC   %S\n", tmpline);
		    tmpline = ajStrParseC(NULL, "\n");
		}
	    }

	    if(ajStrGetLen(seqref->Position))
	    {
		ajStrAssignS(&tmpstr, seqref->Position);
		ajStrFmtWrap(&tmpstr, 75);
		tmpline = ajStrParseC(tmpstr, "\n");

		while (tmpline)
		{
		    ajFmtPrintF(outseq->File, "RP   %S\n", tmpline);
		    tmpline = ajStrParseC(NULL, "\n");
		}
	    }

	    if(ajStrGetLen(seqref->Xref))
	    {
		ajStrAssignS(&tmpstr, seqref->Xref);
		ajStrAppendK(&tmpstr, '.');
		ajStrFmtWrap(&tmpstr, 75);
		tmpline = ajStrParseC(tmpstr, "\n");

		while (tmpline)
		{
		    ajFmtPrintF(outseq->File, "RX   %S\n", tmpline);
		    tmpline = ajStrParseC(NULL, "\n");
		}
	    }

	    if(ajStrGetLen(seqref->Groupname))
	    {
		ajStrAssignS(&tmpstr, seqref->Groupname);
		ajStrFmtWrap(&tmpstr, 75);
		tmpline = ajStrParseC(tmpstr, "\n");

		while (tmpline)
		{
		    ajFmtPrintF(outseq->File, "RG   %S\n", tmpline);
		    tmpline = ajStrParseC(NULL, "\n");
		}
	    }

	    if(ajStrGetLen(seqref->Authors))
	    {
		ajStrAssignS(&tmpstr, seqref->Authors);
		ajStrAppendK(&tmpstr, ';');
		ajStrFmtWrapAt(&tmpstr, 75, ',');
		tmpline = ajStrParseC(tmpstr, "\n");

		while (tmpline)
		{
		    ajFmtPrintF(outseq->File, "RA   %S\n", tmpline);
		    tmpline = ajStrParseC(NULL, "\n");
		}
	    }

	    if(ajStrGetLen(seqref->Title))
	    {
		ajStrAssignS(&tmpstr, seqref->Title);
		ajStrInsertC(&tmpstr, 0, "\"");
		ajStrAppendC(&tmpstr, "\";");
		ajStrFmtWrap(&tmpstr, 75);
		tmpline = ajStrParseC(tmpstr, "\n");

		while (tmpline)
		{
		    ajFmtPrintF(outseq->File, "RT   %S\n", tmpline);
		    tmpline = ajStrParseC(NULL, "\n");
		}
	    }
	    else
		ajFmtPrintF(outseq->File, "RT   ;\n");

	    if(ajStrGetLen(seqref->Location))
	    {
		ajStrAssignS(&tmpstr, seqref->Location);
		ajStrAppendK(&tmpstr, '.');
		ajStrFmtWrap(&tmpstr, 75);
		tmpline = ajStrParseC(tmpstr, "\n");

		while (tmpline)
		{
		    ajFmtPrintF(outseq->File, "RL   %S\n", tmpline);
		    tmpline = ajStrParseC(NULL, "\n");
		}
	    }

	    ajFmtPrintF(outseq->File, "XX\n");
	}

	ajListIterDel(&it);
    }

    if(ajListGetLength(outseq->Xreflist))
    {
	it = ajListIterNewread(outseq->Xreflist);

	while ((xref = (const AjPSeqXref) ajListIterGet(it)))
	{
            if(xref->Type == XREF_DR)
            {
                if(ajStrGetLen(xref->Quatid))
                {
                    ajFmtPrintF(outseq->File, "DR   %S; %S; %S; %S; %S.\n",
                                xref->Db, xref->Id, xref->Secid,
                                xref->Terid, xref->Quatid);
                }
                else if(ajStrGetLen(xref->Terid))
                {
                    ajFmtPrintF(outseq->File, "DR   %S; %S; %S; %S.\n",
                                xref->Db, xref->Id, xref->Secid, xref->Terid);
                }
                else if(ajStrGetLen(xref->Secid))
                {
                    ajFmtPrintF(outseq->File, "DR   %S; %S; %S.\n",
                                xref->Db, xref->Id, xref->Secid);
                }
                else 
                {
                    ajFmtPrintF(outseq->File, "DR   %S; %S.\n",
                                xref->Db, xref->Id);
                }
            }
	}

	ajListIterDel(&it);
	ajFmtPrintF(outseq->File, "XX\n");
    }

    if(ajListGetLength(outseq->Cmtlist))
    {
	it = ajListIterNewread(outseq->Cmtlist);

	while ((cmtstr = (const AjPStr) ajListIterGet(it)))
	{
	    ajStrAssignS(&tmpstr,  cmtstr);
	    ajStrFmtWrapAt(&tmpstr, 75, ',');
	    tmpline = ajStrParseC(tmpstr, "\n");

	    while (tmpline)
	    {
		if(ajStrMatchC(tmpline, " "))
		    ajFmtPrintF(outseq->File, "CC   \n");
		else
		    ajFmtPrintF(outseq->File, "CC   %S\n", tmpline);

		tmpline = ajStrParseC(NULL, "\n");
	    }

	    ajFmtPrintF(outseq->File, "XX\n");
	}

	ajListIterDel(&it);
    }

    if(seqoutUfoLocal(outseq))
    {
	ajFeattabOutDel(&outseq->Ftquery);
        outseq->Ftquery = ajFeattabOutNewCSF("embl", outseq->Name,
					     ajStrGetPtr(outseq->Type),
					     outseq->File);
	if(!ajFeattableWrite(outseq->Ftquery, outseq->Fttable))
	    ajWarn("seqWriteEmbl features output failed UFO: '%S'",
		   outseq->Ufo);
	ajFmtPrintF(outseq->File, "XX\n");
    }
    
    ajSeqoutGetBasecount(outseq, b);
    ajFmtPrintF(outseq->File,
		"SQ   Sequence %d BP; %d A; %d C; %d G; %d T; %d other;\n",
		ajStrGetLen(outseq->Seq), b[0], b[1], b[2], b[3], b[4]);
    
    seqSeqFormat(ajStrGetLen(outseq->Seq), &sf);
    strcpy(sf->endstr, "\n//");
    sf->tab = 4;
    sf->spacer = 11;
    sf->width = 60;
    sf->numright = ajTrue;
    sf->numwidth = 9;
    sf->numjust = ajTrue;
    
    seqWriteSeq(outseq, sf);
    seqFormatDel(&sf);
    ajStrDel(&tmpstr);
    ajStrDel(&idstr);

    return;
}




/* @funcstatic seqWriteSwiss **************************************************
**
** Writes a sequence in SWISSPROT format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void seqWriteSwiss(AjPSeqout outseq)
{
    static SeqPSeqFormat sf = NULL;
    ajuint mw;
    /*  ajuint crc; old 32-bit crc */
    unsigned long long crc;
    AjIList it;
    AjPStr cur;
    ajuint ilen;
    AjPStr tmpstr = NULL;
    const AjPStr tmpline = NULL;
    
    if(ajStrMatchC(outseq->Type, "N"))
    {
	seqWriteEmbl(outseq);

	return;
    }
    
    ajFmtPrintF(outseq->File,
		"ID   %-10S     STANDARD;      PRT; %5d AA.\n",
		outseq->Name, ajStrGetLen(outseq->Seq));
    
    if(ajListGetLength(outseq->Acclist))
    {
	ilen = 0;
	it = ajListIterNewread(outseq->Acclist);

	while((cur = (AjPStr) ajListIterGet(it)))
	{
	    if(ilen + ajStrGetLen(cur) > 79)
	    {
		ajFmtPrintF(outseq->File, ";\n");
		ilen = 0;
	    }

	    if(ilen == 0)
	    {
		ajFmtPrintF(outseq->File, "AC   ");
		ilen = 6;
	    }
	    else
	    {
		ajFmtPrintF(outseq->File, "; ");
		ilen += 2;
	    }

	    ajWriteline(outseq->File, cur);
	    ilen += ajStrGetLen(cur);
	}
	
	ajListIterDel(&it) ;
	ajFmtPrintF(outseq->File, ";\n");
    }
    
    if(ajStrGetLen(outseq->Desc))
	ajFmtPrintF(outseq->File, "DE   %S\n", outseq->Desc);
    
    if(ajStrGetLen(outseq->Tax))
    {
	ajStrAssignS(&tmpstr,  outseq->Tax);
        ajStrAppendK(&tmpstr, '.');
	ajStrFmtWrap(&tmpstr, 75);
	tmpline = ajStrParseC(tmpstr, "\n");

	while (tmpline)
	{
	    ajFmtPrintF(outseq->File, "OS   %S\n", tmpline);
	    tmpline = ajStrParseC(NULL, "\n");
	}
    }

    if(ajListGetLength(outseq->Taxlist) > 1)
    {
	ilen = 0;
	it   = ajListIterNewread(outseq->Taxlist);

	while((cur = (AjPStr) ajListIterGet(it)))
	{
	    if(ilen+ajStrGetLen(cur) >= 79)
	    {
		ajFmtPrintF(outseq->File, ";\n");
		ilen = 0;
	    }

	    if(ilen == 0)
	    {
		ajFmtPrintF(outseq->File, "OC   ");
		ilen = 6;
	    }
	    else
	    {
		ajFmtPrintF(outseq->File, "; ");
		ilen += 2;
	    }

	    ajWriteline(outseq->File, cur);
	    ilen += ajStrGetLen(cur);
	}
	
	ajListIterDel(&it) ;
	ajFmtPrintF(outseq->File, ".\n");
    }
    
    if(ajListGetLength(outseq->Keylist))
    {
	ilen = 0;
	it   = ajListIterNewread(outseq->Keylist);

	while((cur = (AjPStr) ajListIterGet(it)))
	{
	    if(ilen+ajStrGetLen(cur) >= 79)
	    {
		ajFmtPrintF(outseq->File, ";\n");
		ilen = 0;
	    }

	    if(ilen == 0)
	    {
		ajFmtPrintF(outseq->File, "KW   ");
		ilen = 6;
	    }
	    else
	    {
		ajFmtPrintF(outseq->File, "; ");
		ilen += 2;
	    }

	    ajWriteline(outseq->File, cur);
	    ilen += ajStrGetLen(cur);
	}

	ajListIterDel(&it) ;
	ajFmtPrintF(outseq->File, ".\n");
    }
    
    if(seqoutUfoLocal(outseq))
    {
	ajFeattabOutDel(&outseq->Ftquery);
	outseq->Ftquery = ajFeattabOutNewCSF("swiss", outseq->Name,
					     ajStrGetPtr(outseq->Type),
					     outseq->File);
	if(!ajFeattableWrite(outseq->Ftquery, outseq->Fttable))
	    ajWarn("seqWriteSwiss features output failed UFO: '%S'",
		   outseq->Ufo);
    }

    crc = ajMathCrc64(outseq->Seq);
    mw = (ajint) (0.5+ajSeqstrCalcMolwt(outseq->Seq));
    
    ajFmtPrintF(outseq->File,
		"SQ   SEQUENCE %5d AA; %6d MW;  %08X",
		ajStrGetLen(outseq->Seq), mw, (crc>>32)&0xffffffff);
    ajFmtPrintF(outseq->File,
		"%08X CRC64;\n",crc&0xffffffff);
    
    seqSeqFormat(ajStrGetLen(outseq->Seq), &sf);
    strcpy(sf->endstr, "\n//");
    sf->tab = 4;
    sf->spacer = 11;
    sf->width = 60;
    
    seqWriteSeq(outseq, sf);
    seqFormatDel(&sf);

    return;
}




/* @funcstatic seqWriteSwissnew ***********************************************
**
** Writes a sequence in SWISSPROT/UNIPROT format, revised in September 2006
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 4.0.0
** @@
******************************************************************************/

static void seqWriteSwissnew(AjPSeqout outseq)
{
    static SeqPSeqFormat sf = NULL;
    ajuint mw;
    /*  ajuint crc; old 32-bit crc */
    unsigned long long crc;
    AjIList it;
    AjIList itb;
    AjIList itc;
    AjPStr cur;
    ajuint ilen;
    const AjPStr cmtstr = NULL;		/* from list - do not delete */
    AjPStr tmpstr = NULL;
    const AjPSeqRef  seqref  = NULL;
    const AjPSeqXref xref    = NULL;
    const AjPSeqGene seqgene = NULL;
    const AjPSeqDesc    desc    = NULL;
    const AjPStr tmpline = NULL;
    const AjPSeqSubdesc subdesc = NULL;
    const char* altnames="AltName:";
    const char* altspace="        ";
    const char* alttext;

    if(ajStrMatchC(outseq->Type, "N"))
    {
	seqWriteEmbl(outseq);

	return;
    }

    if(ajStrFindAnyK(outseq->Name, '_') > 0)
        ajFmtPrintF(outseq->File,
                    "ID   %-19S     Reviewed;    %8d AA.\n",
                    outseq->Name, ajStrGetLen(outseq->Seq));
    else
        ajFmtPrintF(outseq->File,
                    "ID   %-19S     Unreviewed;    %8d AA.\n",
                    outseq->Name, ajStrGetLen(outseq->Seq));

    if(ajListGetLength(outseq->Acclist))
    {
	ilen = 0;
	it = ajListIterNewread(outseq->Acclist);

	while((cur = (AjPStr) ajListIterGet(it)))
	{
	    if(ilen + ajStrGetLen(cur) > 73)
	    {
		ajFmtPrintF(outseq->File, ";\n");
		ilen = 0;
	    }

	    if(ilen == 0)
	    {
		ajFmtPrintF(outseq->File, "AC   ");
		ilen = 6;
	    }
	    else
	    {
		ajFmtPrintF(outseq->File, "; ");
		ilen += 2;
	    }

	    ajWriteline(outseq->File, cur);
	    ilen += ajStrGetLen(cur);

	}
	
	ajListIterDel(&it) ;
	ajFmtPrintF(outseq->File, ";\n");
    }
    
    if(outseq->Date)
    {
	if(outseq->Date->CreDate)
	    ajFmtPrintF(outseq->File,
			"DT   %D, integrated into %S.\n",
			outseq->Date->CreDate, outseq->Date->CreVer);
	if (outseq->Date->SeqDate)
	    ajFmtPrintF(outseq->File,
			"DT   %D, sequence version %S.\n",
			outseq->Date->SeqDate, outseq->Date->SeqVer);

	if(outseq->Date->ModDate)
	    ajFmtPrintF(outseq->File,
			"DT   %D, entry version %S.\n",
			outseq->Date->ModDate, outseq->Date->ModVer);
    }

    if(outseq->Fulldesc && ajStrGetLen(outseq->Fulldesc->Name))
    {
        ajFmtPrintF(outseq->File,
                    "DE   RecName: Full=%S;\n", outseq->Fulldesc->Name);

        it = ajListIterNewread(outseq->Fulldesc->Short);

        while((cur = (AjPStr) ajListIterGet(it)))
            ajFmtPrintF(outseq->File,
                        "DE            Short=%S;\n", cur);
        ajListIterDel(&it);
        
        it = ajListIterNewread(outseq->Fulldesc->EC);

        while((cur = (AjPStr) ajListIterGet(it)))
            ajFmtPrintF(outseq->File,
                        "DE            EC=%S;\n", cur);
        ajListIterDel(&it);
        
        it = ajListIterNewread(outseq->Fulldesc->AltNames);

        while((subdesc = (AjPSeqSubdesc) ajListIterGet(it)))
        {
            alttext = altnames;

            if(ajStrGetLen(subdesc->Name))
            {
                ajFmtPrintF(outseq->File,
                            "DE   %s Full=%S;\n", alttext, subdesc->Name);
                alttext = altspace;
            }

            itb = ajListIterNewread(subdesc->Inn);
            while((cur = (AjPStr) ajListIterGet(itb)))
            {
                ajFmtPrintF(outseq->File,
                            "DE   %s INN=%S;\n", alttext, cur);
                alttext = altspace;
            }
            
            ajListIterDel(&itb);

            itb = ajListIterNewread(subdesc->Short);

            while((cur = (AjPStr) ajListIterGet(itb)))
                ajFmtPrintF(outseq->File,
                            "DE            Short=%S;\n", cur);

            ajListIterDel(&itb);

            itb = ajListIterNewread(subdesc->EC);

            while((cur = (AjPStr) ajListIterGet(itb)))
                ajFmtPrintF(outseq->File,
                            "DE            EC=%S;\n", cur);
            ajListIterDel(&itb);

            itb = ajListIterNewread(subdesc->Allergen);

            while((cur = (AjPStr) ajListIterGet(itb)))
                ajFmtPrintF(outseq->File,
                            "DE            Allergen=%S;\n", cur);
            ajListIterDel(&itb);

            itb = ajListIterNewread(subdesc->Biotech);

            while((cur = (AjPStr) ajListIterGet(itb)))
                ajFmtPrintF(outseq->File,
                            "DE            Biotech=%S;\n", cur);
            ajListIterDel(&itb);

            itb = ajListIterNewread(subdesc->Cdantigen);

            while((cur = (AjPStr) ajListIterGet(itb)))
                ajFmtPrintF(outseq->File,
                            "DE            CD_antigen=%S;\n", cur);
            ajListIterDel(&itb);

        }

        ajListIterDel(&it);
        
        it = ajListIterNewread(outseq->Fulldesc->SubNames);

        while((subdesc = (AjPSeqSubdesc) ajListIterGet(it)))
        {
            ajFmtPrintF(outseq->File,
                        "DE     SubName: Full=%S;\n", subdesc->Name);
            itb = ajListIterNewread(subdesc->Short);

            while((cur = (AjPStr) ajListIterGet(itb)))
                ajFmtPrintF(outseq->File,
                            "DE              Short=%S;\n", cur);
            ajListIterDel(&itb);
            itb = ajListIterNewread(subdesc->EC);

            while((cur = (AjPStr) ajListIterGet(itb)))
                ajFmtPrintF(outseq->File,
                            "DE              EC=%S;\n", cur);
            ajListIterDel(&itb);
        }

        ajListIterDel(&it);

        itc = ajListIterNewread(outseq->Fulldesc->Includes);

        while((desc = (AjPSeqDesc) ajListIterGet(itc)))
        {
            ajFmtPrintF(outseq->File,
                        "DE   Includes:\n");
            ajFmtPrintF(outseq->File,
                        "DE     RecName: Full=%S;\n", desc->Name);
            it = ajListIterNewread(desc->Short);

            while((cur = (AjPStr) ajListIterGet(it)))
                ajFmtPrintF(outseq->File,
                            "DE              Short=%S;\n", cur);
            ajListIterDel(&it);

            it = ajListIterNewread(desc->EC);

            while((cur = (AjPStr) ajListIterGet(it)))
                ajFmtPrintF(outseq->File,
                            "DE              EC=%S;\n", cur);
            ajListIterDel(&it);

            it = ajListIterNewread(desc->AltNames);

            while((subdesc = (AjPSeqSubdesc) ajListIterGet(it)))
            {
                alttext = altnames;

                if(ajStrGetLen(subdesc->Name))
                {
                    ajFmtPrintF(outseq->File,
                                "DE     %s Full=%S;\n", alttext, subdesc->Name);
                    alttext = altspace;
                }

                itb = ajListIterNewread(subdesc->Inn);

                while((cur = (AjPStr) ajListIterGet(itb)))
                {
                    ajFmtPrintF(outseq->File,
                                "DE     %s INN=%S;\n", alttext, cur);
                    alttext = altspace;
                }

                ajListIterDel(&itb);
                itb = ajListIterNewread(subdesc->Short);

                while((cur = (AjPStr) ajListIterGet(itb)))
                    ajFmtPrintF(outseq->File,
                                "DE              Short=%S;\n", cur);
                ajListIterDel(&itb);

                itb = ajListIterNewread(subdesc->EC);

                while((cur = (AjPStr) ajListIterGet(itb)))
                    ajFmtPrintF(outseq->File,
                                "DE              EC=%S;\n", cur);
                ajListIterDel(&itb);

                itb = ajListIterNewread(subdesc->Allergen);

                while((cur = (AjPStr) ajListIterGet(itb)))
                    ajFmtPrintF(outseq->File,
                                "DE              Allergen=%S;\n", cur);
                ajListIterDel(&itb);

                itb = ajListIterNewread(subdesc->Biotech);

                while((cur = (AjPStr) ajListIterGet(itb)))
                    ajFmtPrintF(outseq->File,
                                "DE              Biotech=%S;\n", cur);
                ajListIterDel(&itb);

                itb = ajListIterNewread(subdesc->Cdantigen);

                while((cur = (AjPStr) ajListIterGet(itb)))
                    ajFmtPrintF(outseq->File,
                                "DE              CD_antigen=%S;\n", cur);
                ajListIterDel(&itb);
            }

            ajListIterDel(&it);
        
            it = ajListIterNewread(desc->SubNames);

            while((subdesc = (AjPSeqSubdesc) ajListIterGet(it)))
            {
                ajFmtPrintF(outseq->File,
                            "DE     SubName: Full=%S;\n", subdesc->Name);
                itb = ajListIterNewread(subdesc->Short);

                while((cur = (AjPStr) ajListIterGet(itb)))
                    ajFmtPrintF(outseq->File,
                                "DE              Short=%S;\n", cur);
                ajListIterDel(&itb);
                itb = ajListIterNewread(subdesc->EC);

                while((cur = (AjPStr) ajListIterGet(itb)))
                    ajFmtPrintF(outseq->File,
                                "DE              EC=%S;\n", cur);
                ajListIterDel(&itb);
            }

            ajListIterDel(&it);

        }

        ajListIterDel(&itc);

        itc = ajListIterNewread(outseq->Fulldesc->Contains);

        while((desc = (AjPSeqDesc) ajListIterGet(itc)))
        {
            ajFmtPrintF(outseq->File,
                        "DE   Contains:\n");
            ajFmtPrintF(outseq->File,
                        "DE     RecName: Full=%S;\n", desc->Name);
            it = ajListIterNewread(desc->Short);

            while((cur = (AjPStr) ajListIterGet(it)))
                ajFmtPrintF(outseq->File,
                            "DE              Short=%S;\n", cur);
            ajListIterDel(&it);

            it = ajListIterNewread(desc->EC);

            while((cur = (AjPStr) ajListIterGet(it)))
                ajFmtPrintF(outseq->File,
                            "DE              EC=%S;\n", cur);
            ajListIterDel(&it);

            it = ajListIterNewread(desc->AltNames);

            while((subdesc = (AjPSeqSubdesc) ajListIterGet(it)))
            {
                alttext = altnames;

                if(ajStrGetLen(subdesc->Name))
                {
                    ajFmtPrintF(outseq->File,
                                "DE     %s Full=%S;\n", alttext, subdesc->Name);
                    alttext = altspace;
                }

                itb = ajListIterNewread(subdesc->Inn);

                while((cur = (AjPStr) ajListIterGet(itb)))
                {
                    ajFmtPrintF(outseq->File,
                                "DE     %s INN=%S;\n", alttext, cur);
                    alttext = altspace;
                }

                ajListIterDel(&itb);

                itb = ajListIterNewread(subdesc->Short);

                while((cur = (AjPStr) ajListIterGet(itb)))
                    ajFmtPrintF(outseq->File,
                                "DE              Short=%S;\n", cur);
                ajListIterDel(&itb);

                itb = ajListIterNewread(subdesc->EC);

                while((cur = (AjPStr) ajListIterGet(itb)))
                    ajFmtPrintF(outseq->File,
                                "DE              EC=%S;\n", cur);
                ajListIterDel(&itb);

                itb = ajListIterNewread(subdesc->Allergen);

                while((cur = (AjPStr) ajListIterGet(itb)))
                    ajFmtPrintF(outseq->File,
                                "DE              Allergen=%S;\n", cur);
                ajListIterDel(&itb);

                itb = ajListIterNewread(subdesc->Biotech);

                while((cur = (AjPStr) ajListIterGet(itb)))
                    ajFmtPrintF(outseq->File,
                                "DE              Biotech=%S;\n", cur);
                ajListIterDel(&itb);

                itb = ajListIterNewread(subdesc->Cdantigen);

                while((cur = (AjPStr) ajListIterGet(itb)))
                    ajFmtPrintF(outseq->File,
                                "DE              CD_antigen=%S;\n", cur);
                ajListIterDel(&itb);
            }

            ajListIterDel(&it);
        
            it = ajListIterNewread(desc->SubNames);

            while((subdesc = (AjPSeqSubdesc) ajListIterGet(it)))
            {
                ajFmtPrintF(outseq->File,
                            "DE     SubName: Full=%S;\n", subdesc->Name);
                itb = ajListIterNewread(subdesc->Short);

                while((cur = (AjPStr) ajListIterGet(itb)))
                    ajFmtPrintF(outseq->File,
                                "DE            Short=%S;\n", cur);

                ajListIterDel(&itb);
                itb = ajListIterNewread(subdesc->EC);

                while((cur = (AjPStr) ajListIterGet(itb)))
                    ajFmtPrintF(outseq->File,
                                "DE            EC=%S;\n", cur);
                ajListIterDel(&itb);
            }

            ajListIterDel(&it);
        }

        ajListIterDel(&itc);

        if(outseq->Fulldesc->Fragments || outseq->Fulldesc->Precursor)
        {
            ajFmtPrintF(outseq->File,
                        "DE   Flags:");

            if(outseq->Fulldesc->Fragments == 1)
                ajFmtPrintF(outseq->File,
                            " Fragment;");

            if(outseq->Fulldesc->Fragments == 2)
                ajFmtPrintF(outseq->File,
                            " Fragments;");

            if(outseq->Fulldesc->Precursor)
                ajFmtPrintF(outseq->File,
                            " Precursor;");

            ajWritebinNewline(outseq->File);
        }
                

    }
    else if(ajStrGetLen(outseq->Desc))
	ajFmtPrintF(outseq->File, "DE   %S\n", outseq->Desc);
    
    if(ajListGetLength(outseq->Genelist))
    {
        ajStrAssignClear(&tmpstr);
	it = ajListIterNewread(outseq->Genelist);

	while ((seqgene = (const AjPSeqGene) ajListIterGet(it)))
	{
            if(ajStrGetLen(tmpstr))
                ajFmtPrintF(outseq->File,
                            "GN   and\n");

            ajStrAssignClear(&tmpstr);

            if(ajStrGetLen(seqgene->Name))
		ajFmtPrintAppS(&tmpstr, " Name=%S;", seqgene->Name);

            if(ajStrGetLen(seqgene->Synonyms))
		ajFmtPrintAppS(&tmpstr, " Synonyms=%S;", seqgene->Synonyms);

            if(ajStrGetLen(seqgene->Oln))
		ajFmtPrintAppS(&tmpstr, " OrderedLocusNames=%S;", seqgene->Oln);

            if(ajStrGetLen(seqgene->Orf))
		ajFmtPrintAppS(&tmpstr, " ORFNames=%S;", seqgene->Orf);

            if(ajStrGetLen(tmpstr))
                ajFmtPrintF(outseq->File,
                            "GN  %S\n", tmpstr);
        }

        ajListIterDel(&it);
    }

    if(ajStrGetLen(outseq->Tax))
    {
	ajStrAssignS(&tmpstr,  outseq->Tax);
        ajStrAppendK(&tmpstr, '.');
	ajStrFmtWrap(&tmpstr, 75);
	tmpline = ajStrParseC(tmpstr, "\n");

	while (tmpline)
	{
	    ajFmtPrintF(outseq->File, "OS   %S\n", tmpline);
	    tmpline = ajStrParseC(NULL, "\n");
	}
    }

    if(ajStrGetLen(outseq->Organelle))
	ajFmtPrintF(outseq->File, "OG   %S.\n", outseq->Organelle);
    
    if(ajListGetLength(outseq->Taxlist) > 1)
    {
	ilen = 0;
	it   = ajListIterNewread(outseq->Taxlist);

	while((cur = (AjPStr) ajListIterGet(it)))
	{
	    if(ilen+ajStrGetLen(cur) >= 74)
	    {
		ajFmtPrintF(outseq->File, ";\n");
		ilen = 0;
	    }

	    if(ilen == 0)
	    {
		ajFmtPrintF(outseq->File, "OC   ");
		ilen = 6;
	    }
	    else
	    {
		ajFmtPrintF(outseq->File, "; ");
		ilen += 2;
	    }
	    ajWriteline(outseq->File, cur);
	    ilen += ajStrGetLen(cur);
	}
	
	ajListIterDel(&it) ;
	ajFmtPrintF(outseq->File, ".\n");
    }
    
    if(ajStrGetLen(outseq->Taxid))
	ajFmtPrintF(outseq->File, "OX   NCBI_TaxID=%S;\n", outseq->Taxid);
    
    if(ajListGetLength(outseq->Reflist))
    {
	it = ajListIterNewread(outseq->Reflist);

	while ((seqref = (const AjPSeqRef) ajListIterGet(it)))
	{
	    ajFmtPrintF(outseq->File, "RN   [%u]\n", seqref->Number);

	    if(ajStrGetLen(seqref->Position))
	    {
		ajStrAssignS(&tmpstr, seqref->Position);
		ajStrFmtWrap(&tmpstr, 70);
		tmpline = ajStrParseC(tmpstr, "\n");

		while (tmpline)
		{
		    ajFmtPrintF(outseq->File, "RP   %S\n", tmpline);
		    tmpline = ajStrParseC(NULL, "\n");
		}
	    }

	    if(ajStrGetLen(seqref->Comment))
	    {
		ajStrAssignS(&tmpstr, seqref->Comment);
		ajStrFmtWrap(&tmpstr, 70);
		tmpline = ajStrParseC(tmpstr, "\n");

		while (tmpline)
		{
		    ajFmtPrintF(outseq->File, "RC   %S\n", tmpline);
		    tmpline = ajStrParseC(NULL, "\n");
		}
	    }

	    if(ajStrGetLen(seqref->Xref))
	    {
		ajStrAssignS(&tmpstr, seqref->Xref);
		ajStrFmtWrap(&tmpstr, 70);
		tmpline = ajStrParseC(tmpstr, "\n");

		while (tmpline)
		{
		    ajFmtPrintF(outseq->File, "RX   %S\n", tmpline);
		    tmpline = ajStrParseC(NULL, "\n");
		}
	    }

	    if(ajStrGetLen(seqref->Groupname))
	    {
		ajStrAssignS(&tmpstr, seqref->Groupname);
		ajStrFmtWrap(&tmpstr, 70);
		tmpline = ajStrParseC(tmpstr, "\n");

		while (tmpline)
		{
		    ajFmtPrintF(outseq->File, "RG   %S\n", tmpline);
		    tmpline = ajStrParseC(NULL, "\n");
		}
	    }

	    if(ajStrGetLen(seqref->Authors))
	    {
		ajStrAssignS(&tmpstr, seqref->Authors);
		ajStrAppendK(&tmpstr, ';');
		ajStrFmtWrapAt(&tmpstr, 70, ',');
		tmpline = ajStrParseC(tmpstr, "\n");

		while (tmpline)
		{
		    ajFmtPrintF(outseq->File, "RA   %S\n", tmpline);
		    tmpline = ajStrParseC(NULL, "\n");
		}
	    }

	    if(ajStrGetLen(seqref->Title))
	    {
		ajStrAssignS(&tmpstr, seqref->Title);
		ajStrInsertC(&tmpstr, 0, "\"");
		ajStrAppendC(&tmpstr, "\";");
		ajStrFmtWrap(&tmpstr, 70);
		tmpline = ajStrParseC(tmpstr, "\n");

		while (tmpline)
		{
		    ajFmtPrintF(outseq->File, "RT   %S\n", tmpline);
		    tmpline = ajStrParseC(NULL, "\n");
		}
	    }

	    if(ajStrGetLen(seqref->Location))
	    {
		ajStrAssignS(&tmpstr, seqref->Location);
		ajStrAppendK(&tmpstr, '.');
		ajStrFmtWrap(&tmpstr, 70);
		tmpline = ajStrParseC(tmpstr, "\n");

		while (tmpline)
		{
		    ajFmtPrintF(outseq->File, "RL   %S\n", tmpline);
		    tmpline = ajStrParseC(NULL, "\n");
		}
	    }
	}
	ajListIterDel(&it);
    }

    if(ajListGetLength(outseq->Cmtlist))
    {
	it = ajListIterNewread(outseq->Cmtlist);

	while ((cmtstr = (const AjPStr) ajListIterGet(it)))
	{
	    ajStrAssignS(&tmpstr,  cmtstr);
	    tmpline = ajStrParseC(tmpstr, "\n");

	    while (tmpline)
	    {
		if(ajStrMatchC(tmpline, " "))
		    ajFmtPrintF(outseq->File, "CC   \n");
		else
		    ajFmtPrintF(outseq->File, "CC   %S\n", tmpline);

		tmpline = ajStrParseC(NULL, "\n");
	    }
	}

	ajListIterDel(&it);
    }


    if(ajListGetLength(outseq->Xreflist))
    {
	it = ajListIterNewread(outseq->Xreflist);

	while ((xref = (const AjPSeqXref) ajListIterGet(it)))
	{
            if(xref->Type == XREF_DR)
            {
                if(ajStrGetLen(xref->Quatid))
                {
                    ajFmtPrintF(outseq->File, "DR   %S; %S; %S; %S; %S.\n",
                                xref->Db, xref->Id, xref->Secid,
                                xref->Terid, xref->Quatid);
                }
                else if(ajStrGetLen(xref->Terid))
                {
                    ajFmtPrintF(outseq->File, "DR   %S; %S; %S; %S.\n",
                                xref->Db, xref->Id, xref->Secid, xref->Terid);
                }
                else if(ajStrGetLen(xref->Secid))
                {
                    ajFmtPrintF(outseq->File, "DR   %S; %S; %S.\n",
                                xref->Db, xref->Id, xref->Secid);
                }
                else 
                {
                    ajFmtPrintF(outseq->File, "DR   %S; %S.\n",
                                xref->Db, xref->Id);
                }
            }
	}

	ajListIterDel(&it);
    }

    if(ajStrGetLen(outseq->Evidence))
	ajFmtPrintF(outseq->File, "PE   %S\n", outseq->Evidence);

    if(ajListGetLength(outseq->Keylist))
    {
	ilen = 0;
	it   = ajListIterNewread(outseq->Keylist);

	while((cur = (AjPStr) ajListIterGet(it)))
	{
	    if(ilen+ajStrGetLen(cur) >= 74)
	    {
		ajFmtPrintF(outseq->File, ";\n");
		ilen = 0;
	    }

	    if(ilen == 0)
	    {
		ajFmtPrintF(outseq->File, "KW   ");
		ilen = 6;
	    }
	    else
	    {
		ajFmtPrintF(outseq->File, "; ");
		ilen += 2;
	    }

	    ajWriteline(outseq->File, cur);
	    ilen += ajStrGetLen(cur);
	}

	ajListIterDel(&it) ;
	ajFmtPrintF(outseq->File, ".\n");
    }
    
    if(seqoutUfoLocal(outseq))
    {
	ajFeattabOutDel(&outseq->Ftquery);
	outseq->Ftquery = ajFeattabOutNewCSF("swiss", outseq->Name,
					     ajStrGetPtr(outseq->Type),
					     outseq->File);
	if(!ajFeattableWrite(outseq->Ftquery, outseq->Fttable))
	    ajWarn("seqWriteSwiss features output failed UFO: '%S'",
		   outseq->Ufo);
    }

    crc = ajMathCrc64(outseq->Seq);
    mw = (ajint) (0.5+ajSeqstrCalcMolwt(outseq->Seq));
    
    /* old 32-bit crc
       ajFmtPrintF(outseq->File,
       "SQ   SEQUENCE %5d AA; %6d MW;  %08X CRC32;\n",
       ajStrGetLen(outseq->Seq), mw, crc);
       */
    
    ajFmtPrintF(outseq->File,
		"SQ   SEQUENCE   %d AA;  %d MW;  %08X",
		ajStrGetLen(outseq->Seq), mw, (crc>>32)&0xffffffff);
    ajFmtPrintF(outseq->File,
		"%08X CRC64;\n",crc&0xffffffff);
    
    seqSeqFormat(ajStrGetLen(outseq->Seq), &sf);
    strcpy(sf->endstr, "\n//");
    sf->tab = 4;
    sf->spacer = 11;
    sf->width = 60;
    
    seqWriteSeq(outseq, sf);
    seqFormatDel(&sf);

    ajStrDel(&tmpstr);

    return;
}




/* @funcstatic seqWriteGenbank ************************************************
**
** Writes a sequence in GENBANK format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void seqWriteGenbank(AjPSeqout outseq)
{
    
    static SeqPSeqFormat sf = NULL;
    /*ajuint b[5];*/                /* was used for BASE COUNT line */
    AjPStr ftfmt = NULL;
    AjPStr tmpstr = NULL;
    AjPStr tmpstr2 = NULL;
    const AjPStr tmpline = NULL;
    const AjPSeqRef seqref = NULL;
    AjIList it;
    AjPStr cur;
    ajuint ilen;
   
    if(!ftfmt)
	ajStrAssignC(&ftfmt, "genbank");
    
    ajSeqoutTrace(outseq);
    
    ajFmtPrintF(outseq->File, "LOCUS       %-17S %10u bp   ",
		outseq->Name, ajStrGetLen(outseq->Seq));

    ajFmtPrintF(outseq->File, " %-7s",
		    ajSeqmolGetGb(outseq->Molecule));

    if(outseq->Circular)
       ajFmtPrintF(outseq->File, " %-8s", "circular");
    else
       ajFmtPrintF(outseq->File, " %-8s", "linear");

    if(ajStrGetLen(outseq->Division))
       ajFmtPrintF(outseq->File, " %-3s",ajSeqdivGetGb(outseq->Division));
    else
       ajFmtPrintF(outseq->File, " UNC");

    if(outseq->Date)
    {
	if(outseq->Date->ModDate)
	    ajFmtPrintF(outseq->File, " %D", outseq->Date->ModDate);
	else if(outseq->Date->CreDate)
	    ajFmtPrintF(outseq->File, " %D", outseq->Date->CreDate);
    }
    else
	ajFmtPrintF(outseq->File, " %D", ajTimeRefTodayFmt("dtline"));
       
    ajWritebinNewline(outseq->File);

    if(ajStrGetLen(outseq->Desc))
    {
	ajStrAssignS(&tmpstr,  outseq->Desc);

	if(ajStrGetCharLast(tmpstr) != '.')
	    ajStrAppendK(&tmpstr, '.');

	ajStrFmtWrap(&tmpstr, 67);
	tmpline = ajStrParseC(tmpstr, "\n");
	ajFmtPrintF(outseq->File, "DEFINITION  %S\n", tmpline);
	tmpline = ajStrParseC(NULL, "\n");

	while (tmpline)
	{
	    ajFmtPrintF(outseq->File, "            %S\n", tmpline);
	    tmpline = ajStrParseC(NULL, "\n");
	}
    }

    if(ajListGetLength(outseq->Acclist))
    {
	ilen = 0;
	it   = ajListIterNewread(outseq->Acclist);
	while((cur = (AjPStr) ajListIterGet(it)))
	{
	    if(ilen == 0)
	    {
		ajFmtPrintF(outseq->File, "ACCESSION   ");
		ilen = 11;
	    }

	    if(ilen + ajStrGetLen(cur) > 79)
	    {
		ajFmtPrintF(outseq->File, "\n            ");
		ilen = 11;
	    }

            if(ilen > 11)
                ajFmtPrintF(outseq->File, " ");
            ilen += 1;

	    ajWriteline(outseq->File, cur);
	    ilen += ajStrGetLen(cur);

	}

	ajListIterDel(&it);

	if(ilen > 0)
	    ajWritebinNewline(outseq->File);
    }
    
    if(ajStrGetLen(outseq->Sv))
    {
	if(ajStrGetLen(outseq->Gi))
	    ajFmtPrintF(outseq->File, "VERSION     %S  GI:%S\n",
			outseq->Sv, outseq->Gi);
	else
	    ajFmtPrintF(outseq->File, "VERSION     %S\n", outseq->Sv);
    }
    
    if(ajListGetLength(outseq->Keylist))
    {
	ilen = 0;
	it = ajListIterNewread(outseq->Keylist);

	while((cur = (AjPStr) ajListIterGet(it)))
	{
	    if(ilen == 0)
	    {
		ajFmtPrintF(outseq->File, "KEYWORDS    ");
		ilen = 11;
	    }

	    if(ilen+ajStrGetLen(cur) >= 79)
	    {
		ajFmtPrintF(outseq->File, ";\n            ");
		ilen = 11;
	    }

            if(ilen > 11)
                ajFmtPrintF(outseq->File, "; ");

            ilen += 2;

	    ajWriteline(outseq->File, cur);
	    ilen += ajStrGetLen(cur);
	}

	ajListIterDel(&it) ;
	ajFmtPrintF(outseq->File, ".\n");
    }
    
    if(ajStrGetLen(outseq->Tax))
    {
	ajFmtPrintF(outseq->File, "SOURCE      %S\n", outseq->Tax);

	ajFmtPrintF(outseq->File, "  ORGANISM  %S\n", outseq->Tax);

	if(ajListGetLength(outseq->Taxlist))
	{
	    ilen = 0;
	    it   = ajListIterNewread(outseq->Taxlist);

	    while((cur = (AjPStr) ajListIterGet(it)))
	    {
		if(ilen+ajStrGetLen(cur) >= 79)
		{
		    ajFmtPrintF(outseq->File, ";\n");
		    ilen = 0;
		}
		
		if(ilen == 0)
		{
		    ajFmtPrintF(outseq->File, "            ");
		    ilen = 12;
		}
		else
		{
		    ajFmtPrintF(outseq->File, "; ");
		    ilen += 2;
		}
		ajWriteline(outseq->File, cur);
		ilen += ajStrGetLen(cur);
	    }

	    ajListIterDel(&it) ;
	    ajFmtPrintF(outseq->File, ".\n");
	}
    }

    if(ajListGetLength(outseq->Reflist))
    {
	it = ajListIterNewread(outseq->Reflist);

	while ((seqref = (const AjPSeqRef) ajListIterGet(it)))
	{
	    ajFmtPrintF(outseq->File, "REFERENCE   %u", seqref->Number);

	    if(ajStrGetLen(seqref->Position))
	    {
		ajStrAssignS(&tmpstr, seqref->Position);
		ajStrExchangeCC(&tmpstr, "-", " to ");
		ajFmtPrintF(outseq->File, "  (bases %S)", tmpstr);
	    }

	    ajWritebinNewline(outseq->File);

	    if(ajStrGetLen(seqref->Authors))
	    {
		ajSeqrefFmtAuthorsGb(seqref, &tmpstr);
		ajStrFmtWrapAt(&tmpstr, 68, ',');
		tmpline = ajStrParseC(tmpstr, "\n");
		ajFmtPrintF(outseq->File, "  AUTHORS   %S\n", tmpline);
		tmpline = ajStrParseC(NULL, "\n");

		while (tmpline)
		{
		    ajFmtPrintF(outseq->File, "            %S\n", tmpline);
		    tmpline = ajStrParseC(NULL, "\n");
		}
	    }

	    ajSeqrefFmtTitleGb(seqref, &tmpstr); /* may set an empty title */

	    if(ajStrGetLen(tmpstr))
	    {
		ajStrFmtWrap(&tmpstr, 68);
		tmpline = ajStrParseC(tmpstr, "\n");
		ajFmtPrintF(outseq->File, "  TITLE     %S\n", tmpline);
		tmpline = ajStrParseC(NULL, "\n");

		while (tmpline)
		{
		    ajFmtPrintF(outseq->File, "            %S\n", tmpline);
		    tmpline = ajStrParseC(NULL, "\n");
		}
	    }

	    if(ajStrGetLen(seqref->Location))
	    {
		ajSeqrefFmtLocationGb(seqref, &tmpstr);
		ajStrFmtWrap(&tmpstr, 68);
		tmpline = ajStrParseC(tmpstr, "\n");
	   	   ajFmtPrintF(outseq->File, "  JOURNAL   %S\n", tmpline);
		tmpline = ajStrParseC(NULL, "\n");

		while (tmpline)
		{
		    ajFmtPrintF(outseq->File, "            %S\n", tmpline);
		    tmpline = ajStrParseC(NULL, "\n");
		}
	    }

	    if(ajStrGetLen(seqref->Xref))
	    {
		ajStrAssignS(&tmpstr, seqref->Xref);
		ajStrFmtWrap(&tmpstr, 75);
		tmpline = ajStrParseC(tmpstr, "\n");

		while (tmpline)
		{
		    if(ajStrPrefixC(tmpline, "PUBMED; "))
		    {
			ajStrAssignSubS(&tmpstr2, tmpline, 8, -1);
			ajFmtPrintF(outseq->File, "  PUBMED    %S\n", tmpstr2);
		    }

		    tmpline = ajStrParseC(NULL, "\n");
		}
	    }

	}

	ajListIterDel(&it);
    }

    if(seqoutUfoLocal(outseq))
    {
	ajFeattabOutDel(&outseq->Ftquery);
        outseq->Ftquery = ajFeattabOutNewSSF(ftfmt, outseq->Name,
					     ajStrGetPtr(outseq->Type),
					     outseq->File);
	if(!ajFeattableWrite(outseq->Ftquery, outseq->Fttable))
	    ajWarn("seqWriteGenbank features output failed UFO: '%S'",
		   outseq->Ufo);
    }

/* no longer used by GenBank */
/*    
    ajSeqoutGetBasecount(outseq, b);
    if(b[4])
	ajFmtPrintF(outseq->File,
		    "BASE COUNT   %6d a %6d c %6d g %6d t %6d others\n",
		    b[0], b[1], b[2], b[3], b[4]);
    else
	ajFmtPrintF(outseq->File,
		    "BASE COUNT   %6d a %6d c %6d g %6d t\n",
		    b[0], b[1], b[2], b[3]);
*/

    ajFmtPrintF(outseq->File, "ORIGIN\n");
    
    seqSeqFormat(ajStrGetLen(outseq->Seq), &sf);
    strcpy(sf->endstr, "\n//");
    sf->tab = 1;
    sf->spacer = 11;
    sf->width = 60;
    sf->numleft = ajTrue;
    sf->numwidth = 8;
    
    seqWriteSeq(outseq, sf);
    seqFormatDel(&sf);
    ajStrDel(&ftfmt);
    ajStrDel(&tmpstr);
    ajStrDel(&tmpstr2);

    return;
}




/* @funcstatic seqWriteGenpept ************************************************
**
** Writes a sequence in GENPEPT format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

static void seqWriteGenpept(AjPSeqout outseq)
{
    
    static SeqPSeqFormat sf = NULL;
    /*ajuint b[5];*/                /* was used for BASE COUNT line */
    AjPStr ftfmt = NULL;
    AjPStr tmpstr = NULL;
    AjPStr tmpstr2 = NULL;
    const AjPStr tmpline = NULL;
    const AjPSeqRef seqref = NULL;
    AjIList it;
    AjPStr cur;
    ajuint ilen;
   
    if(!ftfmt)
	ajStrAssignC(&ftfmt, "genpept");
    
    ajSeqoutTrace(outseq);
    
    ajFmtPrintF(outseq->File, "LOCUS       %-17S %10u bp   ",
		outseq->Name, ajStrGetLen(outseq->Seq));

    ajFmtPrintF(outseq->File, " %-7s",
		    ajSeqmolGetGb(outseq->Molecule));

    if(outseq->Circular)
       ajFmtPrintF(outseq->File, " %-8s", "circular");
    else
       ajFmtPrintF(outseq->File, " %-8s", "linear");

    if(ajStrGetLen(outseq->Division))
       ajFmtPrintF(outseq->File, " %-3s",ajSeqdivGetGb(outseq->Division));
    else
       ajFmtPrintF(outseq->File, " UNC");

    if(outseq->Date)
    {
	if(outseq->Date->ModDate)
	    ajFmtPrintF(outseq->File, " %D", outseq->Date->ModDate);
	else if(outseq->Date->CreDate)
	    ajFmtPrintF(outseq->File, " %D", outseq->Date->CreDate);
    }
    else
	ajFmtPrintF(outseq->File, " %D", ajTimeRefTodayFmt("dtline"));
       
    ajWritebinNewline(outseq->File);

    if(ajStrGetLen(outseq->Desc))
    {
	ajStrAssignS(&tmpstr,  outseq->Desc);

	if(ajStrGetCharLast(tmpstr) != '.')
	    ajStrAppendK(&tmpstr, '.');

	ajStrFmtWrap(&tmpstr, 67);
	tmpline = ajStrParseC(tmpstr, "\n");
	ajFmtPrintF(outseq->File, "DEFINITION  %S\n", tmpline);
	tmpline = ajStrParseC(NULL, "\n");

	while (tmpline)
	{
	    ajFmtPrintF(outseq->File, "            %S\n", tmpline);
	    tmpline = ajStrParseC(NULL, "\n");
	}
    }

    if(ajListGetLength(outseq->Acclist))
    {
	ilen = 0;
	it   = ajListIterNewread(outseq->Acclist);
	while((cur = (AjPStr) ajListIterGet(it)))
	{
	    if(ilen == 0)
	    {
		ajFmtPrintF(outseq->File, "ACCESSION   ");
		ilen = 11;
	    }

	    if(ilen + ajStrGetLen(cur) > 79)
	    {
		ajFmtPrintF(outseq->File, "\n            ");
		ilen = 11;
	    }

            if(ilen > 11)
                ajFmtPrintF(outseq->File, " ");
            ilen += 1;

	    ajWriteline(outseq->File, cur);
	    ilen += ajStrGetLen(cur);

	}

	ajListIterDel(&it);

	if(ilen > 0)
	    ajWritebinNewline(outseq->File);
    }
    
    if(ajStrGetLen(outseq->Sv))
    {
	if(ajStrGetLen(outseq->Gi))
	    ajFmtPrintF(outseq->File, "VERSION     %S  GI:%S\n",
			outseq->Sv, outseq->Gi);
	else
	    ajFmtPrintF(outseq->File, "VERSION     %S\n", outseq->Sv);
    }
    
    if(ajListGetLength(outseq->Keylist))
    {
	ilen = 0;
	it = ajListIterNewread(outseq->Keylist);

	while((cur = (AjPStr) ajListIterGet(it)))
	{
	    if(ilen == 0)
	    {
		ajFmtPrintF(outseq->File, "KEYWORDS    ");
		ilen = 11;
	    }

	    if(ilen+ajStrGetLen(cur) >= 79)
	    {
		ajFmtPrintF(outseq->File, ";\n            ");
		ilen = 11;
	    }

            if(ilen > 11)
                ajFmtPrintF(outseq->File, "; ");

            ilen += 2;

	    ajWriteline(outseq->File, cur);
	    ilen += ajStrGetLen(cur);
	}

	ajListIterDel(&it) ;
	ajFmtPrintF(outseq->File, ".\n");
    }
    
    if(ajStrGetLen(outseq->Tax))
    {
	ajFmtPrintF(outseq->File, "SOURCE      %S\n", outseq->Tax);

	ajFmtPrintF(outseq->File, "  ORGANISM  %S\n", outseq->Tax);

	if(ajListGetLength(outseq->Taxlist))
	{
	    ilen = 0;
	    it   = ajListIterNewread(outseq->Taxlist);

	    while((cur = (AjPStr) ajListIterGet(it)))
	    {
		if(ilen+ajStrGetLen(cur) >= 79)
		{
		    ajFmtPrintF(outseq->File, ";\n");
		    ilen = 0;
		}
		
		if(ilen == 0)
		{
		    ajFmtPrintF(outseq->File, "            ");
		    ilen = 12;
		}
		else
		{
		    ajFmtPrintF(outseq->File, "; ");
		    ilen += 2;
		}
		ajWriteline(outseq->File, cur);
		ilen += ajStrGetLen(cur);
	    }

	    ajListIterDel(&it) ;
	    ajFmtPrintF(outseq->File, ".\n");
	}
    }

    if(ajListGetLength(outseq->Reflist))
    {
	it = ajListIterNewread(outseq->Reflist);

	while ((seqref = (const AjPSeqRef) ajListIterGet(it)))
	{
	    ajFmtPrintF(outseq->File, "REFERENCE   %u", seqref->Number);

	    if(ajStrGetLen(seqref->Position))
	    {
		ajStrAssignS(&tmpstr, seqref->Position);
		ajStrExchangeCC(&tmpstr, "-", " to ");
		ajFmtPrintF(outseq->File, "  (bases %S)", tmpstr);
	    }

	    ajWritebinNewline(outseq->File);

	    if(ajStrGetLen(seqref->Authors))
	    {
		ajSeqrefFmtAuthorsGb(seqref, &tmpstr);
		ajStrFmtWrapAt(&tmpstr, 68, ',');
		tmpline = ajStrParseC(tmpstr, "\n");
		ajFmtPrintF(outseq->File, "  AUTHORS   %S\n", tmpline);
		tmpline = ajStrParseC(NULL, "\n");

		while (tmpline)
		{
		    ajFmtPrintF(outseq->File, "            %S\n", tmpline);
		    tmpline = ajStrParseC(NULL, "\n");
		}
	    }

	    ajSeqrefFmtTitleGb(seqref, &tmpstr); /* may set an empty title */

	    if(ajStrGetLen(tmpstr))
	    {
		ajStrFmtWrap(&tmpstr, 68);
		tmpline = ajStrParseC(tmpstr, "\n");
		ajFmtPrintF(outseq->File, "  TITLE     %S\n", tmpline);
		tmpline = ajStrParseC(NULL, "\n");

		while (tmpline)
		{
		    ajFmtPrintF(outseq->File, "            %S\n", tmpline);
		    tmpline = ajStrParseC(NULL, "\n");
		}
	    }

	    if(ajStrGetLen(seqref->Location))
	    {
		ajSeqrefFmtLocationGb(seqref, &tmpstr);
		ajStrFmtWrap(&tmpstr, 68);
		tmpline = ajStrParseC(tmpstr, "\n");
	   	   ajFmtPrintF(outseq->File, "  JOURNAL   %S\n", tmpline);
		tmpline = ajStrParseC(NULL, "\n");

		while (tmpline)
		{
		    ajFmtPrintF(outseq->File, "            %S\n", tmpline);
		    tmpline = ajStrParseC(NULL, "\n");
		}
	    }

	    if(ajStrGetLen(seqref->Xref))
	    {
		ajStrAssignS(&tmpstr, seqref->Xref);
		ajStrFmtWrap(&tmpstr, 75);
		tmpline = ajStrParseC(tmpstr, "\n");

		while (tmpline)
		{
		    if(ajStrPrefixC(tmpline, "PUBMED; "))
		    {
			ajStrAssignSubS(&tmpstr2, tmpline, 8, -1);
			ajFmtPrintF(outseq->File, "  PUBMED    %S\n", tmpstr2);
		    }

		    tmpline = ajStrParseC(NULL, "\n");
		}
	    }

	}

	ajListIterDel(&it);
    }

    if(seqoutUfoLocal(outseq))
    {
	ajFeattabOutDel(&outseq->Ftquery);
        outseq->Ftquery = ajFeattabOutNewSSF(ftfmt, outseq->Name,
					     ajStrGetPtr(outseq->Type),
					     outseq->File);
	if(!ajFeattableWrite(outseq->Ftquery, outseq->Fttable))
	    ajWarn("seqWriteGenpept features output failed UFO: '%S'",
		   outseq->Ufo);
    }

    ajFmtPrintF(outseq->File, "ORIGIN\n");
    
    seqSeqFormat(ajStrGetLen(outseq->Seq), &sf);
    strcpy(sf->endstr, "\n//");
    sf->tab = 1;
    sf->spacer = 11;
    sf->width = 60;
    sf->numleft = ajTrue;
    sf->numwidth = 8;
    
    seqWriteSeq(outseq, sf);
    seqFormatDel(&sf);
    ajStrDel(&ftfmt);
    ajStrDel(&tmpstr);
    ajStrDel(&tmpstr2);

    return;
}




/* @funcstatic seqWriteRefseq *************************************************
**
** Writes a sequence in REFSEQ format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

static void seqWriteRefseq(AjPSeqout outseq)
{
    
    static SeqPSeqFormat sf = NULL;
    /*ajuint b[5];*/                /* was used for BASE COUNT line */
    AjPStr ftfmt = NULL;
    AjPStr tmpstr = NULL;
    AjPStr tmpstr2 = NULL;
    const AjPStr tmpline = NULL;
    const AjPSeqRef seqref = NULL;
    AjIList it;
    AjPStr cur;
    ajuint ilen;
   
    if(!ftfmt)
	ajStrAssignC(&ftfmt, "refseq");
    
    ajSeqoutTrace(outseq);
    
    ajFmtPrintF(outseq->File, "LOCUS       %-17S %10u bp   ",
		outseq->Name, ajStrGetLen(outseq->Seq));

    ajFmtPrintF(outseq->File, " %-7s",
		    ajSeqmolGetGb(outseq->Molecule));

    if(outseq->Circular)
       ajFmtPrintF(outseq->File, " %-8s", "circular");
    else
       ajFmtPrintF(outseq->File, " %-8s", "linear");

    if(ajStrGetLen(outseq->Division))
       ajFmtPrintF(outseq->File, " %-3s",ajSeqdivGetGb(outseq->Division));
    else
       ajFmtPrintF(outseq->File, " UNC");

    if(outseq->Date)
    {
	if(outseq->Date->ModDate)
	    ajFmtPrintF(outseq->File, " %D", outseq->Date->ModDate);
	else if(outseq->Date->CreDate)
	    ajFmtPrintF(outseq->File, " %D", outseq->Date->CreDate);
    }
    else
	ajFmtPrintF(outseq->File, " %D", ajTimeRefTodayFmt("dtline"));
       
    ajWritebinNewline(outseq->File);

    if(ajStrGetLen(outseq->Desc))
    {
	ajStrAssignS(&tmpstr,  outseq->Desc);

	if(ajStrGetCharLast(tmpstr) != '.')
	    ajStrAppendK(&tmpstr, '.');

	ajStrFmtWrap(&tmpstr, 67);
	tmpline = ajStrParseC(tmpstr, "\n");
	ajFmtPrintF(outseq->File, "DEFINITION  %S\n", tmpline);
	tmpline = ajStrParseC(NULL, "\n");

	while (tmpline)
	{
	    ajFmtPrintF(outseq->File, "            %S\n", tmpline);
	    tmpline = ajStrParseC(NULL, "\n");
	}
    }

    if(ajListGetLength(outseq->Acclist))
    {
	ilen = 0;
	it   = ajListIterNewread(outseq->Acclist);
	while((cur = (AjPStr) ajListIterGet(it)))
	{
	    if(ilen == 0)
	    {
		ajFmtPrintF(outseq->File, "ACCESSION   ");
		ilen = 11;
	    }

	    if(ilen + ajStrGetLen(cur) > 79)
	    {
		ajFmtPrintF(outseq->File, "\n            ");
		ilen = 11;
	    }

            if(ilen > 11)
                ajFmtPrintF(outseq->File, " ");
            ilen += 1;

	    ajWriteline(outseq->File, cur);
	    ilen += ajStrGetLen(cur);

	}

	ajListIterDel(&it);

	if(ilen > 0)
	    ajWritebinNewline(outseq->File);
    }
    
    if(ajStrGetLen(outseq->Sv))
    {
	if(ajStrGetLen(outseq->Gi))
	    ajFmtPrintF(outseq->File, "VERSION     %S  GI:%S\n",
			outseq->Sv, outseq->Gi);
	else
	    ajFmtPrintF(outseq->File, "VERSION     %S\n", outseq->Sv);
    }
    
    if(ajListGetLength(outseq->Keylist))
    {
	ilen = 0;
	it = ajListIterNewread(outseq->Keylist);

	while((cur = (AjPStr) ajListIterGet(it)))
	{
	    if(ilen == 0)
	    {
		ajFmtPrintF(outseq->File, "KEYWORDS    ");
		ilen = 11;
	    }

	    if(ilen+ajStrGetLen(cur) >= 79)
	    {
		ajFmtPrintF(outseq->File, ";\n            ");
		ilen = 11;
	    }

            if(ilen > 11)
                ajFmtPrintF(outseq->File, "; ");

            ilen += 2;

	    ajWriteline(outseq->File, cur);
	    ilen += ajStrGetLen(cur);
	}

	ajListIterDel(&it) ;
	ajFmtPrintF(outseq->File, ".\n");
    }
    
    if(ajStrGetLen(outseq->Tax))
    {
	ajFmtPrintF(outseq->File, "SOURCE      %S\n", outseq->Tax);

	ajFmtPrintF(outseq->File, "  ORGANISM  %S\n", outseq->Tax);

	if(ajListGetLength(outseq->Taxlist))
	{
	    ilen = 0;
	    it   = ajListIterNewread(outseq->Taxlist);

	    while((cur = (AjPStr) ajListIterGet(it)))
	    {
		if(ilen+ajStrGetLen(cur) >= 79)
		{
		    ajFmtPrintF(outseq->File, ";\n");
		    ilen = 0;
		}
		
		if(ilen == 0)
		{
		    ajFmtPrintF(outseq->File, "            ");
		    ilen = 12;
		}
		else
		{
		    ajFmtPrintF(outseq->File, "; ");
		    ilen += 2;
		}
		ajWriteline(outseq->File, cur);
		ilen += ajStrGetLen(cur);
	    }

	    ajListIterDel(&it) ;
	    ajFmtPrintF(outseq->File, ".\n");
	}
    }

    if(ajListGetLength(outseq->Reflist))
    {
	it = ajListIterNewread(outseq->Reflist);

	while ((seqref = (const AjPSeqRef) ajListIterGet(it)))
	{
	    ajFmtPrintF(outseq->File, "REFERENCE   %u", seqref->Number);

	    if(ajStrGetLen(seqref->Position))
	    {
		ajStrAssignS(&tmpstr, seqref->Position);
		ajStrExchangeCC(&tmpstr, "-", " to ");
		ajFmtPrintF(outseq->File, "  (bases %S)", tmpstr);
	    }

	    ajWritebinNewline(outseq->File);

	    if(ajStrGetLen(seqref->Authors))
	    {
		ajSeqrefFmtAuthorsGb(seqref, &tmpstr);
		ajStrFmtWrapAt(&tmpstr, 68, ',');
		tmpline = ajStrParseC(tmpstr, "\n");
		ajFmtPrintF(outseq->File, "  AUTHORS   %S\n", tmpline);
		tmpline = ajStrParseC(NULL, "\n");

		while (tmpline)
		{
		    ajFmtPrintF(outseq->File, "            %S\n", tmpline);
		    tmpline = ajStrParseC(NULL, "\n");
		}
	    }

	    ajSeqrefFmtTitleGb(seqref, &tmpstr); /* may set an empty title */

	    if(ajStrGetLen(tmpstr))
	    {
		ajStrFmtWrap(&tmpstr, 68);
		tmpline = ajStrParseC(tmpstr, "\n");
		ajFmtPrintF(outseq->File, "  TITLE     %S\n", tmpline);
		tmpline = ajStrParseC(NULL, "\n");

		while (tmpline)
		{
		    ajFmtPrintF(outseq->File, "            %S\n", tmpline);
		    tmpline = ajStrParseC(NULL, "\n");
		}
	    }

	    if(ajStrGetLen(seqref->Location))
	    {
		ajSeqrefFmtLocationGb(seqref, &tmpstr);
		ajStrFmtWrap(&tmpstr, 68);
		tmpline = ajStrParseC(tmpstr, "\n");
	   	   ajFmtPrintF(outseq->File, "  JOURNAL   %S\n", tmpline);
		tmpline = ajStrParseC(NULL, "\n");

		while (tmpline)
		{
		    ajFmtPrintF(outseq->File, "            %S\n", tmpline);
		    tmpline = ajStrParseC(NULL, "\n");
		}
	    }

	    if(ajStrGetLen(seqref->Xref))
	    {
		ajStrAssignS(&tmpstr, seqref->Xref);
		ajStrFmtWrap(&tmpstr, 75);
		tmpline = ajStrParseC(tmpstr, "\n");

		while (tmpline)
		{
		    if(ajStrPrefixC(tmpline, "PUBMED; "))
		    {
			ajStrAssignSubS(&tmpstr2, tmpline, 8, -1);
			ajFmtPrintF(outseq->File, "  PUBMED    %S\n", tmpstr2);
		    }

		    tmpline = ajStrParseC(NULL, "\n");
		}
	    }

	}

	ajListIterDel(&it);
    }

    if(seqoutUfoLocal(outseq))
    {
	ajFeattabOutDel(&outseq->Ftquery);
        outseq->Ftquery = ajFeattabOutNewSSF(ftfmt, outseq->Name,
					     ajStrGetPtr(outseq->Type),
					     outseq->File);
	if(!ajFeattableWrite(outseq->Ftquery, outseq->Fttable))
	    ajWarn("seqWriteRefseq features output failed UFO: '%S'",
		   outseq->Ufo);
    }

    ajFmtPrintF(outseq->File, "ORIGIN\n");
    
    seqSeqFormat(ajStrGetLen(outseq->Seq), &sf);
    strcpy(sf->endstr, "\n//");
    sf->tab = 1;
    sf->spacer = 11;
    sf->width = 60;
    sf->numleft = ajTrue;
    sf->numwidth = 8;
    
    seqWriteSeq(outseq, sf);
    seqFormatDel(&sf);
    ajStrDel(&ftfmt);
    ajStrDel(&tmpstr);
    ajStrDel(&tmpstr2);

    return;
}




/* @funcstatic seqWriteRefseqp ************************************************
**
** Writes a sequence in REFSEQP format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

static void seqWriteRefseqp(AjPSeqout outseq)
{
    
    static SeqPSeqFormat sf = NULL;
    /*ajuint b[5];*/                /* was used for BASE COUNT line */
    AjPStr tmpstr = NULL;
    AjPStr tmpstr2 = NULL;
    const AjPStr tmpline = NULL;
    const AjPSeqRef seqref = NULL;
    AjIList it;
    AjPStr cur;
    ajuint ilen;
   
    ajSeqoutTrace(outseq);
    
    ajFmtPrintF(outseq->File, "LOCUS       %-9S %7u aa                   ",
		outseq->Name, ajStrGetLen(outseq->Seq));

    if(ajStrGetLen(outseq->Division))
       ajFmtPrintF(outseq->File, " %-3s",ajSeqdivGetGb(outseq->Division));
    else
       ajFmtPrintF(outseq->File, " UNC");

    if(outseq->Date)
    {
	if(outseq->Date->ModDate)
	    ajFmtPrintF(outseq->File, "       %D", outseq->Date->ModDate);
	else if(outseq->Date->CreDate)
	    ajFmtPrintF(outseq->File, "       %D", outseq->Date->CreDate);
    }
    else
	ajFmtPrintF(outseq->File, "       %D", ajTimeRefTodayFmt("dtline"));
       
    ajWritebinNewline(outseq->File);

    if(ajStrGetLen(outseq->Desc))
    {
	ajStrAssignS(&tmpstr,  outseq->Desc);

	if(ajStrGetCharLast(tmpstr) != '.')
	    ajStrAppendK(&tmpstr, '.');

	ajStrFmtWrap(&tmpstr, 67);
	tmpline = ajStrParseC(tmpstr, "\n");
	ajFmtPrintF(outseq->File, "DEFINITION  %S\n", tmpline);
	tmpline = ajStrParseC(NULL, "\n");

	while (tmpline)
	{
	    ajFmtPrintF(outseq->File, "            %S\n", tmpline);
	    tmpline = ajStrParseC(NULL, "\n");
	}
    }

    if(ajListGetLength(outseq->Acclist))
    {
	ilen = 0;
	it   = ajListIterNewread(outseq->Acclist);
	while((cur = (AjPStr) ajListIterGet(it)))
	{
	    if(ilen == 0)
	    {
		ajFmtPrintF(outseq->File, "ACCESSION   ");
		ilen = 11;
	    }

	    if(ilen + ajStrGetLen(cur) > 79)
	    {
		ajFmtPrintF(outseq->File, "\n            ");
		ilen = 11;
	    }

            if(ilen > 11)
                ajFmtPrintF(outseq->File, " ");
            ilen += 1;

	    ajWriteline(outseq->File, cur);
	    ilen += ajStrGetLen(cur);

	}

	ajListIterDel(&it);

	if(ilen > 0)
	    ajWritebinNewline(outseq->File);
    }
    
    if(ajStrGetLen(outseq->Sv))
    {
	if(ajStrGetLen(outseq->Gi))
	    ajFmtPrintF(outseq->File, "VERSION     %S  GI:%S\n",
			outseq->Sv, outseq->Gi);
	else
	    ajFmtPrintF(outseq->File, "VERSION     %S\n", outseq->Sv);
    }
    
    if(ajListGetLength(outseq->Keylist))
    {
	ilen = 0;
	it = ajListIterNewread(outseq->Keylist);

	while((cur = (AjPStr) ajListIterGet(it)))
	{
	    if(ilen == 0)
	    {
		ajFmtPrintF(outseq->File, "KEYWORDS    ");
		ilen = 11;
	    }

	    if(ilen+ajStrGetLen(cur) >= 79)
	    {
		ajFmtPrintF(outseq->File, ";\n            ");
		ilen = 11;
	    }

            if(ilen > 11)
                ajFmtPrintF(outseq->File, "; ");

            ilen += 2;

	    ajWriteline(outseq->File, cur);
	    ilen += ajStrGetLen(cur);
	}

	ajListIterDel(&it) ;
	ajFmtPrintF(outseq->File, ".\n");
    }
    
    if(ajStrGetLen(outseq->Tax))
    {
	ajFmtPrintF(outseq->File, "SOURCE      %S\n", outseq->Tax);

	ajFmtPrintF(outseq->File, "  ORGANISM  %S\n", outseq->Tax);

	if(ajListGetLength(outseq->Taxlist))
	{
	    ilen = 0;
	    it   = ajListIterNewread(outseq->Taxlist);

	    while((cur = (AjPStr) ajListIterGet(it)))
	    {
		if(ilen+ajStrGetLen(cur) >= 79)
		{
		    ajFmtPrintF(outseq->File, ";\n");
		    ilen = 0;
		}
		
		if(ilen == 0)
		{
		    ajFmtPrintF(outseq->File, "            ");
		    ilen = 12;
		}
		else
		{
		    ajFmtPrintF(outseq->File, "; ");
		    ilen += 2;
		}
		ajWriteline(outseq->File, cur);
		ilen += ajStrGetLen(cur);
	    }

	    ajListIterDel(&it) ;
	    ajFmtPrintF(outseq->File, ".\n");
	}
    }

    if(ajListGetLength(outseq->Reflist))
    {
	it = ajListIterNewread(outseq->Reflist);

	while ((seqref = (const AjPSeqRef) ajListIterGet(it)))
	{
	    ajFmtPrintF(outseq->File, "REFERENCE   %u", seqref->Number);

	    if(ajStrGetLen(seqref->Position))
	    {
		ajStrAssignS(&tmpstr, seqref->Position);
		ajStrExchangeCC(&tmpstr, "-", " to ");
		ajFmtPrintF(outseq->File, "  (residues %S)", tmpstr);
	    }

	    ajWritebinNewline(outseq->File);

	    if(ajStrGetLen(seqref->Authors))
	    {
		ajSeqrefFmtAuthorsGb(seqref, &tmpstr);
		ajStrFmtWrapAt(&tmpstr, 68, ',');
		tmpline = ajStrParseC(tmpstr, "\n");
		ajFmtPrintF(outseq->File, "  AUTHORS   %S\n", tmpline);
		tmpline = ajStrParseC(NULL, "\n");

		while (tmpline)
		{
		    ajFmtPrintF(outseq->File, "            %S\n", tmpline);
		    tmpline = ajStrParseC(NULL, "\n");
		}
	    }

	    ajSeqrefFmtTitleGb(seqref, &tmpstr); /* may set an empty title */

	    if(ajStrGetLen(tmpstr))
	    {
		ajStrFmtWrap(&tmpstr, 68);
		tmpline = ajStrParseC(tmpstr, "\n");
		ajFmtPrintF(outseq->File, "  TITLE     %S\n", tmpline);
		tmpline = ajStrParseC(NULL, "\n");

		while (tmpline)
		{
		    ajFmtPrintF(outseq->File, "            %S\n", tmpline);
		    tmpline = ajStrParseC(NULL, "\n");
		}
	    }

	    if(ajStrGetLen(seqref->Location))
	    {
		ajSeqrefFmtLocationGb(seqref, &tmpstr);
		ajStrFmtWrap(&tmpstr, 68);
		tmpline = ajStrParseC(tmpstr, "\n");
	   	   ajFmtPrintF(outseq->File, "  JOURNAL   %S\n", tmpline);
		tmpline = ajStrParseC(NULL, "\n");

		while (tmpline)
		{
		    ajFmtPrintF(outseq->File, "            %S\n", tmpline);
		    tmpline = ajStrParseC(NULL, "\n");
		}
	    }

	    if(ajStrGetLen(seqref->Xref))
	    {
		ajStrAssignS(&tmpstr, seqref->Xref);
		ajStrFmtWrap(&tmpstr, 75);
		tmpline = ajStrParseC(tmpstr, "\n");

		while (tmpline)
		{
		    if(ajStrPrefixC(tmpline, "PUBMED; "))
		    {
			ajStrAssignSubS(&tmpstr2, tmpline, 8, -1);
			ajFmtPrintF(outseq->File, "  PUBMED    %S\n", tmpstr2);
		    }

		    tmpline = ajStrParseC(NULL, "\n");
		}
	    }

	}

	ajListIterDel(&it);
    }

    if(seqoutUfoLocal(outseq))
    {
	ajFeattabOutDel(&outseq->Ftquery);
        outseq->Ftquery = ajFeattabOutNewCSF("refseqp", outseq->Name,
					     ajStrGetPtr(outseq->Type),
					     outseq->File);
	if(!ajFeattableWrite(outseq->Ftquery, outseq->Fttable))
	    ajWarn("seqWriteRefseqp features output failed UFO: '%S'",
		   outseq->Ufo);
    }

    ajFmtPrintF(outseq->File, "ORIGIN      \n");
    
    seqSeqFormat(ajStrGetLen(outseq->Seq), &sf);
    strcpy(sf->endstr, "\n//");
    sf->tab = 0;
    sf->spacer = 11;
    sf->width = 60;
    sf->numleft = ajTrue;
    sf->numwidth = 9;

    seqWriteSeq(outseq, sf);
    seqFormatDel(&sf);
    ajStrDel(&tmpstr);
    ajStrDel(&tmpstr2);

    return;
}




/* @funcstatic seqWriteGff2 ***************************************************
**
** Writes a sequence in GFF 2.0 format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 6.0.0
** @@
******************************************************************************/

static void seqWriteGff2(AjPSeqout outseq)
{
    SeqPSeqFormat sf = NULL;
    
    ajFmtPrintF(outseq->File,
		"##gff-version 2\n");
    ajFmtPrintF(outseq->File,
		"##source-version EMBOSS %S\n", ajNamValueVersion());
    ajFmtPrintF(outseq->File,
		"##date %D\n", ajTimeRefTodayFmt("GFF"));

    if(ajStrMatchC(outseq->Type, "P"))
	ajFmtPrintF(outseq->File,
		    "##Protein %S\n", outseq->Name);
    else
	ajFmtPrintF(outseq->File,
		    "##DNA %S\n", outseq->Name);
    
    seqSeqFormat(ajStrGetLen(outseq->Seq), &sf);
    
    strcpy(sf->leftstr, "##");
    sf->width = 60;
    /*
       sf->tab = 4;
       sf->spacer = 11;
       sf->numright = ajTrue;
       sf->numwidth = 9;
       sf->numjust = ajTrue;
       */
    
    seqWriteSeq(outseq, sf);
    seqFormatDel(&sf);
    
    if(ajStrMatchC(outseq->Type, "P"))
	ajFmtPrintF(outseq->File, "##end-Protein\n");
    else
	ajFmtPrintF(outseq->File, "##end-DNA\n");
   
    if(seqoutUfoLocal(outseq))
    {
	ajFeattabOutDel(&outseq->Ftquery);
	outseq->Ftquery = ajFeattabOutNewCSF("gff2", outseq->Name,
					     ajStrGetPtr(outseq->Type),
					     outseq->File);
	if(ajStrMatchC(outseq->Type, "P"))
	    ajFeattableSetProt(outseq->Fttable);
	else
	    ajFeattableSetNuc(outseq->Fttable);
	
	if(!ajFeattableWrite(outseq->Ftquery, outseq->Fttable))
	    ajWarn("seqWriteGff2 features output failed UFO: '%S'",
		   outseq->Ufo);

    }

    return;
}




/* @funcstatic seqWriteGff3 ***************************************************
**
** Writes a sequence in GFF 3 format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 6.0.0
** @@
******************************************************************************/

static void seqWriteGff3(AjPSeqout outseq)
{
    if(seqoutUfoLocal(outseq))
    {
	ajFeattabOutDel(&outseq->Ftquery);
	outseq->Ftquery = ajFeattabOutNewCSF("gff3", outseq->Name,
					     ajStrGetPtr(outseq->Type),
					     outseq->File);
	if(ajStrMatchC(outseq->Type, "P"))
	    ajFeattableSetProt(outseq->Fttable);
	else
	    ajFeattableSetNuc(outseq->Fttable);
	
	if(!ajFeattableWrite(outseq->Ftquery, outseq->Fttable))
	    ajWarn("seqWriteGff3 features output failed UFO: '%S'",
		   outseq->Ufo);

    }
    else
    {
	/* GFF3 header */
	ajFmtPrintF(outseq->File, "##gff-version 3\n") ;
	ajFmtPrintF(outseq->File, "##sequence-region %S %u %u\n",
		    outseq->Name, 1, ajStrGetLen(outseq->Seq));

	/* extra EMBOSS header lines */
	ajFmtPrintF(outseq->File,
		    "#!Date %D\n", ajTimeRefTodayFmt("GFF"));

	if(ajStrMatchC(outseq->Type, "P"))
	    ajFmtPrintF(outseq->File,"#!Type Protein\n");
	else
	    ajFmtPrintF(outseq->File, "#!Type DNA\n");

    	ajFmtPrintF(outseq->File, "#!Source-version EMBOSS %S\n",
                    ajNamValueVersion());
    }

    ajFmtPrintF(outseq->File, "##FASTA\n");

    seqWriteFasta(outseq);

    return;
}




/* @funcstatic seqWriteBam ****************************************************
**
** Writes a sequence in binary sequence alignment/map (BAM) format.
**
** The sort order is "unsorted". Samtools can re-sort the file.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

static void seqWriteBam(AjPSeqout outseq)
{
    struct bamdata
    {
        ajuint Count;
        ajuint Nref;
        AjPSeqBamBgzf gzfile;
        AjPSeqBam bam;
    } *bamdata = NULL;

    AjPSeqBamHeader header;
    AjPSeqBam bam;
    AjPSeqBamCore core;
    unsigned char *dpos;
    AjPStr qualstr = NULL;
    const char *s;
    ajuint ilen;
    ajuint slen;
    ajuint i;

    unsigned char bam_nt16_table[256] =
        {
	15,15,15,15, 15,15,15,15, 15,15,15,15, 15,15,15,15,
	15,15,15,15, 15,15,15,15, 15,15,15,15, 15,15,15,15,
	15,15,15,15, 15,15,15,15, 15,15,15,15, 15,15,15,15,
	 1, 2, 4, 8, 15,15,15,15, 15,15,15,15, 15, 0 /*=*/,15,15,
	15, 1,14, 2, 13,15,15, 4, 11,15,15,12, 15, 3,15,15,
	15,15, 5, 6,  8,15, 7, 9, 15,10,15,15, 15,15,15,15,
	15, 1,14, 2, 13,15,15, 4, 11,15,15,12, 15, 3,15,15,
	15,15, 5, 6,  8,15, 7, 9, 15,10,15,15, 15,15,15,15,
	15,15,15,15, 15,15,15,15, 15,15,15,15, 15,15,15,15,
	15,15,15,15, 15,15,15,15, 15,15,15,15, 15,15,15,15,
	15,15,15,15, 15,15,15,15, 15,15,15,15, 15,15,15,15,
	15,15,15,15, 15,15,15,15, 15,15,15,15, 15,15,15,15,
	15,15,15,15, 15,15,15,15, 15,15,15,15, 15,15,15,15,
	15,15,15,15, 15,15,15,15, 15,15,15,15, 15,15,15,15,
	15,15,15,15, 15,15,15,15, 15,15,15,15, 15,15,15,15,
	15,15,15,15, 15,15,15,15, 15,15,15,15, 15,15,15,15
};


    if(!outseq->Count)
    {
        outseq->Cleanup = seqCleanBam;
        AJNEW0(bamdata);
        AJNEW0(bam);

        bamdata->bam = bam;
        
        bamdata->gzfile =
        	ajSeqBamBgzfNew(ajFileGetFileptr(outseq->File), "w");

        /* header text is simply copied from SAM */

        header = ajSeqBamHeaderNewTextC("@HD\tVN:1.3\tSO:unsorted\n");

        ajSeqBamHeaderWrite(bamdata->gzfile, header);
        outseq->Data = bamdata;
        ajSeqBamHeaderDel(&header);
    }

    /* bam_write1 for each sequence */

    /* get data for name, flag 0x0004, seq, quality */
    bamdata = outseq->Data;
    bam = bamdata->bam;
    core = &bam->core;

    ilen = ajStrGetLen(outseq->Seq);

    core->tid = -1;
    core->pos = -1; /* BAM format is zero based;
                       -1 is translated to 0, meaning unmapped */
    core->bin = 0;
    core->qual = '\0';
    core->l_qname = 1 + ajStrGetLen(outseq->Name);
    core->flag = 0x0004;
    core->n_cigar = 0;
    core->l_qseq = ilen;
    core->mtid = -1;
    core->mpos = 0;
    core->isize = 0;

    qualstr = ajStrNewRes(ilen+1);

    if(outseq->Accuracy)
    {
        for(i=0;i<ilen;i++)
	{
	    ajStrAppendK(&qualstr, (int) outseq->Accuracy[i]);
	}
    }

    else 
    {
        ajStrAppendCountK(&qualstr,'\"' - 33, ilen);
    }
    

    bam->data_len = core->n_cigar*4 + core->l_qname +
        (ilen + 1)/2 + ilen;
    if(bam->data_len > bam->m_data)
    {
        AJCRESIZE0(bam->data,bam->m_data, bam->data_len);
        bam->m_data = bam->data_len;
    }

    dpos = bam->data;
    memcpy(dpos, ajStrGetPtr(outseq->Name), core->l_qname);

    dpos += core->l_qname;
    dpos += core->n_cigar*4;

    s = ajStrGetPtr(outseq->Seq);
    slen = (ilen+1)/2;
    for (i = 0; i < slen; ++i)
        dpos[i] = 0;
    for (i = 0; i < ilen; ++i)
        dpos[i/2] |= bam_nt16_table[(ajuint)s[i]] << 4*(1-i%2);

    dpos += slen;
    memcpy(dpos, ajStrGetPtr(qualstr), ilen);

    ajSeqBamWrite(bamdata->gzfile, bam);

    ajStrDel(&qualstr);

    return;
}




/* @funcstatic seqCleanBam ****************************************************
**
** Writes the remaining lines to complete and close a BAM file
**
** @param [u] outseq [AjPSeqout] Sequence output object
** @return [void]
**
** @release 6.3.0
** @@
******************************************************************************/

static void seqCleanBam(AjPSeqout outseq)
{
    struct bamdata
    {
        ajuint Count;
        ajuint Nref;
        AjPSeqBamBgzf gzfile;
        AjPSeqBam bam;
    } *bamdata = NULL;

    bamdata = outseq->Data;

    ajSeqBamBgzfClose(bamdata->gzfile);
    AJFREE(bamdata->bam->data);
    AJFREE(bamdata->bam);
    AJFREE(bamdata);

    return;
}




/* @funcstatic seqWriteSam ****************************************************
**
** Writes a sequence in sequence alignment/map (SAM) format.
**
** The sort order is "unsorted". Samtools can re-sort the file.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

static void seqWriteSam(AjPSeqout outseq)
{
    AjPStr argstr = NULL;
    AjPStr qualstr = NULL;
    ajint flag = 0;
    ajuint ilen;
    ajuint i;

    if(!outseq->Count)
    {
        outseq->Cleanup = NULL;
        ajFmtPrintF(outseq->File, "@HD\tVN:1.3\tSO:unsorted\n");

        /* SQ is a reference sequence. Can we omit this if not aligned? */

        /*ajFmtPrintF(outseq->File, "@SQ\tSN:%S\tLN:%d",
          outseq->Name, ajStrGetLen(outseq->Seq));*/

        /* AS assembly identifier */
        /* M5 checksum */
        /* UR URI */
        /* SP species */
        /* ajFmtPrintF(outseq->File, "\n"); */ /* end of @SQ record */

        /* Read group */

        /* Program record */
        argstr = ajStrNewS(ajUtilGetCmdline());
        ajStrExchangeKK(&argstr, '\n', ' ');
        ajFmtPrintF(outseq->File, "@PG\tID:%S\tVN:%S\tCL:%S\n",
                    ajUtilGetProgram(), ajNamValueVersion(), argstr);
        ajStrDel(&argstr);

        /* Comment */
        /*ajFmtPrintF(outseq->File, "@CO\t%S\n", cmtstr);*/
    }

    flag = 0x0004;              /* query not mapped */

    ilen = ajStrGetLen(outseq->Seq);

    qualstr = ajStrNewRes(ilen+1);

    if(outseq->Accuracy)
    {
        for(i=0;i<ilen;i++)
	{
	    ajStrAppendK(&qualstr, 33 + (int) outseq->Accuracy[i]);
	}
    }

    else 
    {
        ajStrAppendCountK(&qualstr,'\"', ilen);
    }
    

    ajFmtPrintF(outseq->File, "%S\t%d\t*\t0\t0\t*\t*\t0\t0\t%S\t%S\n",
                outseq->Name, flag, outseq->Seq, qualstr);

    /* could add tag:vtype:value fields at end of record */

    ajStrDel(&qualstr);

    return;
}





/* @funcstatic seqWriteScf ****************************************************
**
** Writes a sequence in SCF version 3 format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void seqWriteScf(AjPSeqout outseq)
{
    AjPFile outf = outseq->File;
    ajuint filepos = 128;
    AjPStr tmpstr = NULL;
    ajuint i;
    unsigned char iqual;
    ajuint spare[18] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    ajuint seqlen = ajStrGetLen(outseq->Seq);
    const char* cp;

    ajStrAssignS(&tmpstr, outseq->Desc);
    ajStrExchangeCC(&tmpstr, "; ", "\n");
    ajStrExchangeCC(&tmpstr, ";", "\n");

    ajWritebinChar(outf, ".scf", 4);
    ajWritebinUint4(outf, 0);    /* no samples */
    ajWritebinUint4(outf, filepos);  /* after header */
    ajWritebinUint4(outf, seqlen);
    ajWritebinUint4(outf, 0);    /* unused left clip */
    ajWritebinUint4(outf, 0);    /* unused right clip */
    filepos += 4;                /* one dummy sample */
    ajWritebinUint4(outf, filepos);  /* no samples */   
    filepos += 12 * ajStrGetLen(outseq->Seq);
    ajWritebinUint4(outf, ajStrGetLen(tmpstr)); /* comments */
    ajWritebinUint4(outf, filepos);  /* no samples */   
    filepos += ajStrGetLen(tmpstr);
    ajWritebinChar(outf, "3.00", 4);
    ajWritebinUint4(outf, 1);       /* 8bit unused samples */
    ajWritebinUint4(outf, 0);       /* standard ACGT and - */
    ajWritebinUint4(outf, 0);       /* no private data */
    ajWritebinUint4(outf, filepos);  /* end of file */   
    ajWritebinBinary(outf, 18, 4, spare);

    ajWritebinUint4(outf, 0); /* dummy 4 byte sample */

    cp = ajStrGetPtr(outseq->Seq);

    if(outseq->Accuracy)
    {        
        for(i=0; i < seqlen; i++)
            ajWritebinUint4(outf, 0);
        for(i=0; i < seqlen; i++)
        {
            iqual = 0;
            if(cp[i] == 'A' || cp[i] == 'a')
	      iqual = (int) outseq->Accuracy[i];
            ajWritebinByte(outf, iqual);
        }
        for(i=0; i < seqlen; i++)
        {
            iqual = 0;
            if(cp[i] == 'C' || cp[i] == 'c')
	      iqual = (int) outseq->Accuracy[i];
            ajWritebinByte(outf, iqual);
        }
        for(i=0; i < seqlen; i++)
        {
            iqual = 0;
            if(cp[i] == 'G' || cp[i] == 'g')
	      iqual = (int) outseq->Accuracy[i];
            ajWritebinByte(outf, iqual);
        }
        for(i=0; i < seqlen; i++)
        {
            iqual = 0;
            if(cp[i] == 'T' || cp[i] == 't')
	      iqual = (int) outseq->Accuracy[i];
            ajWritebinByte(outf, iqual);
        }
    }
    else
    {
        for(i=0; i < seqlen; i++)
            ajWritebinUint4(outf, 0);
        for(i=0; i < seqlen; i++)
            ajWritebinByte(outf, '\0');
        for(i=0; i < seqlen; i++)
            ajWritebinByte(outf, '\0');
        for(i=0; i < seqlen; i++)
            ajWritebinByte(outf, '\0');
        for(i=0; i < seqlen; i++)
            ajWritebinByte(outf, '\0');
    }
    
    ajWritebinStr(outf, outseq->Seq, seqlen);

    /* spare bytes */
    for(i=0; i < seqlen; i++)
        ajWritebinChar(outf, "\0\0\0", 3);

    ajWritebinStr(outf, tmpstr, ajStrGetLen(tmpstr));
    
    return;
}




/* @funcstatic seqWriteStrider ************************************************
**
** Writes a sequence in DNA STRIDER format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void seqWriteStrider(AjPSeqout outseq)
{
    static SeqPSeqFormat sf = NULL;

    ajFmtPrintF(outseq->File, "; ### from DNA Strider ;-)\n");
    ajFmtPrintF(outseq->File, "; DNA sequence  %S, %d bases\n;\n",
		outseq->Name, ajStrGetLen(outseq->Seq));

    seqSeqFormat(ajStrGetLen(outseq->Seq), &sf);
    strcpy(sf->endstr, "\n//");

    seqWriteSeq(outseq, sf);
    seqFormatDel(&sf);

    return;
}




/* @funcstatic seqWriteFitch **************************************************
**
** Writes a sequence in FITCH format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void seqWriteFitch(AjPSeqout outseq)
{
    static SeqPSeqFormat sf = NULL;

    ajFmtPrintF(outseq->File, "%S, %d bases\n",
		outseq->Name, ajStrGetLen(outseq->Seq));

    seqSeqFormat(ajStrGetLen(outseq->Seq), &sf);
    sf->spacer = 4;
    sf->width  = 60;

    seqWriteSeq(outseq, sf);
    seqFormatDel(&sf);

    return;
}




/* @funcstatic seqWriteMase ***************************************************
**
** Writes a sequence in MASE format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 2.8.0
** @@
******************************************************************************/

static void seqWriteMase(AjPSeqout outseq)
{
    ajuint i;
    ajuint ilen;
    AjPStr seq = NULL;
    ajuint linelen = 60;
    ajuint iend;

    if (!ajFileResetPos(outseq->File))
	ajFmtPrintF(outseq->File, ";;Written by EMBOSS on %D\n",
		ajTimeRefTodayFmt("report"));

    ajFmtPrintF(outseq->File, ";%S\n",
		outseq->Desc);

    ajWritelineNewline(outseq->File, outseq->Name);

    ilen = ajStrGetLen(outseq->Seq);

    for(i=0; i < ilen; i += linelen)
    {
	iend = AJMIN(ilen-1, i+linelen-1);
	ajStrAssignSubS(&seq, outseq->Seq, i, iend);
	ajWritelineNewline(outseq->File, seq);
    }

    ajStrDel(&seq);

    return;
}




/* @funcstatic seqWritePhylip *************************************************
**
** Writes a sequence in PHYLIP interleaved format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void seqWritePhylip(AjPSeqout outseq)
{
    ajulong isize;
    ajuint ilen = 0;
    ajulong i    = 0;
    ajuint j    = 0;
    char *p    = NULL;
    void** seqs = NULL;
    AjPSeq seq;
    AjPSeq* seqarr;
    ajulong itest;
    AjPStr sseq = NULL;
    ajuint ipos;
    ajuint iend;
    AjPStr tstr = NULL;
    
    ajDebug("seqWritePhylip list size %Lu\n",
	    ajListGetLength(outseq->Savelist));
    
    isize = ajListGetLength(outseq->Savelist);

    if(!isize)
	return;
    
    itest = ajListToarray(outseq->Savelist, (void***) &seqs);
    ajDebug("ajListToarray listed %Lu items\n", itest);
    seqarr = (AjPSeq*) seqs;

    for(i=0UL; i < isize; i++)
    {
	seq = seqarr[i];

	if(ilen < ajSeqGetLen(seq))
	    ilen = ajSeqGetLen(seq);
    }
    
    tstr = ajStrNewRes(ilen+1);
    ajFmtPrintF(outseq->File, " %Lu %u\n", isize, ilen);
    
    for(ipos=1; ipos <= ilen; ipos += 50)
    {
	iend = ipos + 50 -1;

	if(iend > ilen)
	    iend = ilen;
	
	for(i=0UL; i < isize; i++)
	{
	    seq = seqarr[i];

	    ajStrAssignC(&tstr,ajStrGetPtr(seq->Seq));
	    p = ajStrGetuniquePtr(&tstr);

	    for(j=ajStrGetLen(tstr);j<ilen;++j)
		*(p+j)='-';

	    *(p+j)='\0';
	    tstr->Len=ilen;
	    ajStrAssignSubS(&sseq, tstr, ipos-1, iend-1);
	    ajSeqGapS(&sseq, '-');
	    ajStrFmtBlock(&sseq, 10);

	    if(ipos == 1)
		ajFmtPrintF(outseq->File,
			    "%-10.10S%S\n",
			    seq->Name, sseq);
	    else
		ajFmtPrintF(outseq->File,
			    "%10s%S\n",
			    " ", sseq);
	}

	if(iend < ilen)
	    ajWritebinNewline(outseq->File);
    }
    
    ajStrDel(&tstr);
    ajStrDel(&sseq);
    AJFREE(seqs);

    return;
}




/* @funcstatic seqWritePhylipnon **********************************************
**
** Writes a sequence in PHYLIP non-interleaved format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 3.0.0
** @@
******************************************************************************/

static void seqWritePhylipnon(AjPSeqout outseq)
{
    ajulong isize;
    ajuint ilen = 0;
    ajuint i    = 0;
    ajulong j    = 0;
    ajuint n    = 0;
    char *p = NULL;
    void** seqs = NULL;
    AjPSeq seq;
    AjPSeq* seqarr;
    ajulong itest;
    AjPStr sseq = NULL;
    ajuint ipos;
    ajuint iend  = 0;
    AjPStr tstr = NULL;
    
    ajDebug("seqWritePhylipnon list size %Lu\n",
	    ajListGetLength(outseq->Savelist));
    
    isize = ajListGetLength(outseq->Savelist);

    if(!isize)
	return;
    
    itest = ajListToarray(outseq->Savelist, (void***) &seqs);
    ajDebug("ajListToarray listed %Lu items\n", itest);
    seqarr = (AjPSeq*) seqs;

    for(i=0UL; i < isize; i++)
    {
	seq = seqarr[i];

	if(ilen < ajSeqGetLen(seq))
	    ilen = ajSeqGetLen(seq);
    }
    
    tstr = ajStrNewRes(ilen+1);
    ajFmtPrintF(outseq->File, "%Lu %u\n", isize, ilen);
    
    for(n=0;n<isize;++n)
    {
	seq = seqarr[n];
	ajStrAssignC(&tstr,ajStrGetPtr(seq->Seq));
	p = ajStrGetuniquePtr(&tstr);

	for(j=ajStrGetLen(tstr);j<ilen;++j)
	    *(p+j)='-';

	*(p+j)='\0';
	tstr->Len=ilen;


	for(ipos=1; ipos <= ilen; ipos += 50)
	{
	    iend = ipos + 50 -1;

	    if(iend > ilen)
		iend = ilen;

	    ajStrAssignSubS(&sseq, tstr, ipos-1, iend-1);
	    ajSeqGapS(&sseq, '-');
	    ajStrFmtBlock(&sseq, 10);

	    if(ipos == 1)
		ajFmtPrintF(outseq->File,
			    "%-10.10S%S\n",
			    seq->Name, sseq);
	    else
		ajFmtPrintF(outseq->File,
			    "%10s%S\n",
			    " ", sseq);
	}

	if(iend < ilen)
	    ajWritebinNewline(outseq->File);

    }
    
    ajStrDel(&tstr);
    ajStrDel(&sseq);
    AJFREE(seqs);

    return;
}




/* @funcstatic seqWriteAsn1 ***************************************************
**
** Writes a sequence in ASN.1 format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void seqWriteAsn1(AjPSeqout outseq)
{
    static SeqPSeqFormat sf = NULL;

    ajFmtPrintF(outseq->File, "  seq {\n");
    ajFmtPrintF(outseq->File, "    id { local id 1 },\n");
    ajFmtPrintF(outseq->File, "    descr { title \"%S\" },\n",
		outseq->Desc);
    ajFmtPrintF(outseq->File, "    inst {\n");

    if(!outseq->Type)
	ajFmtPrintF(outseq->File,
		    "      repr raw, mol dna, length %d, "
		    "topology linear,\n {\n",
		    ajStrGetLen(outseq->Seq));
    else if(ajStrMatchC(outseq->Type, "P"))
	ajFmtPrintF(outseq->File,
		    "      repr raw, mol aa, length %d, "
		    "topology linear,\n {\n",
		    ajStrGetLen(outseq->Seq));
    else
	ajFmtPrintF(outseq->File,
		    "      repr raw, mol dna, length %d, "
		    "topology linear,\n",
		    ajStrGetLen(outseq->Seq));

    ajFmtPrintF(outseq->File, "      seq-data\n");

    if(ajStrMatchC(outseq->Type, "P"))
	ajFmtPrintF(outseq->File, "        iupacaa \"");
    else
	ajFmtPrintF(outseq->File, "        iupacna \"");

    seqSeqFormat(ajStrGetLen(outseq->Seq), &sf);
    sf->linepos = 17;
    sf->spacer  = 0;
    sf->width   = 78;
    sf->tab     = 0;
    strcpy(sf->endstr, "\"\n      } } ,");

    seqWriteSeq(outseq, sf);
    seqFormatDel(&sf);

    return;
}




/* @funcstatic seqWriteIg *****************************************************
**
** Writes a sequence in INTELLIGENETICS format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void seqWriteIg(AjPSeqout outseq)
{
    static SeqPSeqFormat sf = NULL;

    ajFmtPrintF(outseq->File, ";%S, %d bases\n",
			outseq->Desc, ajStrGetLen(outseq->Seq));
    ajWritelineNewline(outseq->File, outseq->Name);

    seqSeqFormat(ajStrGetLen(outseq->Seq), &sf);
    strcpy(sf->endstr, "1");	/* linear (DNA) */

    seqWriteSeq(outseq, sf);
    seqFormatDel(&sf);

    return;
}




/* @funcstatic seqWriteAcedb **************************************************
**
** Writes a sequence in ACEDB format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void seqWriteAcedb(AjPSeqout outseq)
{
    static SeqPSeqFormat sf = NULL;

    if(ajStrMatchC(outseq->Type, "P"))
	ajFmtPrintF(outseq->File, "Peptide : \"%S\"\n", outseq->Name);
    else
	ajFmtPrintF(outseq->File, "DNA : \"%S\"\n", outseq->Name);

    seqSeqFormat(ajStrGetLen(outseq->Seq), &sf);
    strcpy(sf->endstr, "\n");

    seqWriteSeq(outseq, sf);
    seqFormatDel(&sf);

    return;
}




/* @funcstatic seqWriteDasdna *************************************************
**
** Writes a sequence in DASDNA XML format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 6.1.0
** @@
******************************************************************************/

static void seqWriteDasdna(AjPSeqout outseq)
{
    ajuint i;
    ajuint ilen;
    ajuint iend;
    ajuint linelen     = 50;
    AjPStr seq = NULL;

    ilen = ajStrGetLen(outseq->Seq);

    if(!outseq->Count)
    {
        outseq->Cleanup = seqCleanDasdna;
        ajFmtPrintF(outseq->File,
                    "<?xml version=\"1.0\" standalone=\"no\"?>\n");
        ajFmtPrintF(outseq->File,
                    "<!DOCTYPE DASDNA SYSTEM "
                    "\"http://www.biodas.org/dtd/dasdna.dtd\">\n");
            ajFmtPrintF(outseq->File,
                        "<DASDNA>\n");
    }
    

    ajFmtPrintF(outseq->File,
                "  <SEQUENCE id=\"%S\" start=\"%d\" stop=\"%d\" "
                "version=\"%S\">\n",
                outseq->Name, 1+outseq->Offset,
                ilen+outseq->Offset,
                outseq->Sv);

    ajFmtPrintF(outseq->File,
                "    <DNA length=\"%d\">\n", ilen);

    for(i=0; i < ilen; i += linelen)
    {
	iend = AJMIN(ilen-1, i+linelen-1);
	ajStrAssignSubS(&seq, outseq->Seq, i, iend);
	ajFmtPrintF(outseq->File, "      %S\n", seq);
    }

    ajFmtPrintF(outseq->File,
                "    </DNA>\n");
    ajFmtPrintF(outseq->File,
                "  </SEQUENCE>\n");

    ajStrDel(&seq);

    return;
}




/* @funcstatic seqCleanDasdna *************************************************
**
** Writes the remaining lines to complete and close a DASDNA XML file
**
** @param [u] outseq [AjPSeqout] Sequence output object
** @return [void]
**
** @release 6.1.0
** @@
******************************************************************************/


static void seqCleanDasdna(AjPSeqout outseq)
{
    AjPFile file = outseq->File;
    ajFmtPrintF(file,
                "</DASDNA>\n");

    return;
}




/* @funcstatic seqWriteDasseq *************************************************
**
** Writes a sequence in DAS SEQUENCE XML format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 6.1.0
** @@
******************************************************************************/

static void seqWriteDasseq(AjPSeqout outseq)
{
    ajuint i;
    ajuint ilen;
    ajuint iend;
    ajuint linelen     = 50;
    AjPStr seq = NULL;

    ilen = ajStrGetLen(outseq->Seq);

    if(!outseq->Count)
    {
        outseq->Cleanup = seqCleanDasseq;
        ajFmtPrintF(outseq->File,
                    "<?xml version=\"1.0\" standalone=\"no\"?>\n");
        ajFmtPrintF(outseq->File,
                    "<!DOCTYPE DASSEQUENCE SYSTEM "
                    "\"http://www.biodas.org/dtd/dassequence.dtd\">\n");
    }


    ajFmtPrintF(outseq->File,
                "<DASSEQUENCE>\n");

    ajFmtPrintF(outseq->File,
                "  <SEQUENCE id=\"%S\" start=\"%d\" stop=\"%d\"\n",
                outseq->Name,
                1+outseq->Offset,
                ilen+outseq->Offset);
    if(ajStrMatchC(outseq->Type, "P"))
        ajFmtPrintF(outseq->File,
                    "               moltype=\"Protein\"",
                    outseq->Sv);
    else
        ajFmtPrintF(outseq->File,
                    "               moltype=\"DNA\"");
    if(ajStrGetLen(outseq->Sv))
        ajFmtPrintF(outseq->File,
                    " version=\"%S\">\n",
                    outseq->Sv);
    else
        ajFmtPrintF(outseq->File,
                    " version=\"0.0\">\n");

    for(i=0; i < ilen; i += linelen)
    {
	iend = AJMIN(ilen-1, i+linelen-1);
	ajStrAssignSubS(&seq, outseq->Seq, i, iend);
	ajFmtPrintF(outseq->File, "      %S\n", seq);
    }

    ajFmtPrintF(outseq->File,
                "  </SEQUENCE>\n");

    ajStrDel(&seq);

    return;
}




/* @funcstatic seqCleanDasseq *************************************************
**
** Writes the remaining lines to complete and close a DASDNA XML file
**
** @param [u] outseq [AjPSeqout] Sequence output object
** @return [void]
**
** @release 6.1.0
** @@
******************************************************************************/


static void seqCleanDasseq(AjPSeqout outseq)
{
    AjPFile file = outseq->File;
    ajFmtPrintF(file,
                "</DASSEQUENCE>\n");

    return;
}




/* @funcstatic seqWriteDebug **************************************************
**
** Writes a sequence in debug report format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void seqWriteDebug(AjPSeqout outseq)
{
    static SeqPSeqFormat sf = NULL;
    AjIList it;
    AjPStr cur;
    AjPSeqRef  curref;
    AjPSeqXref curxref;
    ajuint ilen;
    ajuint i;
    ajuint j;
    ajuint jend;

    const char* xreftypes[] = {
        "unknown", "DRline", "db_xref",
        "ECnumber", "DescTag", "TaxId", "RXline", NULL
    };

    ajFmtPrintF(outseq->File, "Sequence output trace\n");
    ajFmtPrintF(outseq->File, "=====================\n\n");
    ajFmtPrintF(outseq->File, "  Name: '%S'\n", ajStrConstS(outseq->Name));
    ajFmtPrintF(outseq->File, "  Accession: '%S'\n", ajStrConstS(outseq->Acc));

    if(ajListGetLength(outseq->Acclist))
    {
	ajFmtPrintF(outseq->File, "  Acclist: (%Lu)",
		    ajListGetLength(outseq->Acclist));
	it = ajListIterNewread(outseq->Acclist);

	while((cur = (AjPStr) ajListIterGet(it)))
	    ajFmtPrintF(outseq->File, " %S\n", cur);

	ajListIterDel(&it);
	ajWritebinNewline(outseq->File);
    }
    
    ajFmtPrintF(outseq->File, "  SeqVersion: '%S'\n", ajStrConstS(outseq->Sv));
    ajFmtPrintF(outseq->File, "  GenInfo Id: '%S'\n", ajStrConstS(outseq->Gi));
    ajFmtPrintF(outseq->File, "  Description: '%S'\n",
                ajStrConstS(outseq->Desc));

    if(ajListGetLength(outseq->Keylist))
    {
	ajFmtPrintF(outseq->File, "  Keywordlist: (%Lu)\n",
		    ajListGetLength(outseq->Keylist));
	it = ajListIterNewread(outseq->Keylist);

	while((cur = (AjPStr) ajListIterGet(it)))
	    ajFmtPrintF(outseq->File, "    '%S'\n", cur);

	ajListIterDel(&it);
    }

    ajFmtPrintF(outseq->File, "  Taxonomy: '%S'\n", ajStrConstS(outseq->Tax));
    ajFmtPrintF(outseq->File, "  TaxId: '%S'\n", ajStrConstS(outseq->Taxid));
    ajFmtPrintF(outseq->File, "  Organelle: '%S'\n",
                ajStrConstS(outseq->Organelle));

    if(ajListGetLength(outseq->Taxlist))
    {
	ajFmtPrintF(outseq->File, "  Taxlist: (%Lu)\n",
		    ajListGetLength(outseq->Taxlist));
	it = ajListIterNewread(outseq->Taxlist);

	while((cur = (AjPStr) ajListIterGet(it)))
	    ajFmtPrintF(outseq->File, "    '%S'\n", cur);

	ajListIterDel(&it);
    }

    ajFmtPrintF(outseq->File, "  Type: '%S'\n", ajStrConstS(outseq->Type));
    ajFmtPrintF(outseq->File, "  Output type: '%S'\n",
                ajStrConstS(outseq->Outputtype));
    ajFmtPrintF(outseq->File, "  Molecule: '%S'\n",
                ajStrConstS(outseq->Molecule));
    ajFmtPrintF(outseq->File, "  Class: '%S'\n", ajStrConstS(outseq->Class));
    ajFmtPrintF(outseq->File, "  Division: '%S'\n",
                ajStrConstS(outseq->Division));
    ajFmtPrintF(outseq->File, "  Database: '%S'\n", ajStrConstS(outseq->Db));
    ajFmtPrintF(outseq->File, "  Set database: '%S'\n",
                ajStrConstS(outseq->Setdb));
    ajFmtPrintF(outseq->File, "  Output database: '%S'\n",
                ajStrConstS(outseq->Setoutdb));
    ajFmtPrintF(outseq->File, "  Full name: '%S'\n", ajStrConstS(outseq->Full));

    if(outseq->Date)
    {
	if(outseq->Date->CreDate)
	    ajFmtPrintF(outseq->File,
			"  Created: '%D' Rel. '%S' Ver. '%S'\n",
			outseq->Date->CreDate, outseq->Date->CreRel,
			outseq->Date->CreVer);
	if(outseq->Date->ModDate)
	    ajFmtPrintF(outseq->File,
			"  Modified: '%D' Rel. '%S' Ver. '%S'\n",
			outseq->Date->ModDate, outseq->Date->ModRel,
			outseq->Date->ModVer);
	if(outseq->Date->SeqDate)
	    ajFmtPrintF(outseq->File,
			"  Seq modified: '%D' Rel. '%S' Ver. '%S'\n",
			outseq->Date->SeqDate, outseq->Date->SeqRel,
			outseq->Date->SeqVer);
    }

    if(ajListGetLength(outseq->Cmtlist))
    {
	ajFmtPrintF(outseq->File, "  Commentlist: (%Lu)\n",
		    ajListGetLength(outseq->Cmtlist));
	it = ajListIterNewread(outseq->Cmtlist);

	while((cur = (AjPStr) ajListIterGet(it)))
	    ajFmtPrintF(outseq->File, "    '%S'\n", cur);

	ajListIterDel(&it);
    }

    if(ajListGetLength(outseq->Xreflist))
    {
	ajFmtPrintF(outseq->File, "  Xreflist: (%Lu)\n",
		    ajListGetLength(outseq->Xreflist));
	it = ajListIterNewread(outseq->Xreflist);

        i = 0;

	while((curxref = (AjPSeqXref) ajListIterGet(it)))
        {
            if(curxref->Type < XREF_MAX)
                ajFmtPrintF(outseq->File, "%3d Type: '%u' %s\n",
                            i++, curxref->Type, xreftypes[curxref->Type]);
            else
                ajFmtPrintF(outseq->File, "%3d Type: '%u' ..BAD..\n",
                            i++, curxref->Type);
	    ajFmtPrintF(outseq->File, "       Db: '%S'\n", curxref->Db);
	    ajFmtPrintF(outseq->File, "       Id: '%S'\n", curxref->Id);
	    if(ajStrGetLen(curxref->Secid))
               ajFmtPrintF(outseq->File, "    Secid: '%S'\n", curxref->Secid);
	    if(ajStrGetLen(curxref->Terid))
               ajFmtPrintF(outseq->File, "    Terid: '%S'\n", curxref->Terid);
	    if(ajStrGetLen(curxref->Quatid))
               ajFmtPrintF(outseq->File, "   Quatid: '%S'\n", curxref->Quatid);
            if(curxref->Start || curxref->End)
                 ajFmtPrintF(outseq->File, "    Range: %d..%d\n",
                             curxref->Start, curxref->End);
        }
        
	ajListIterDel(&it);
    }

    if(ajListGetLength(outseq->Reflist))
    {
	ajFmtPrintF(outseq->File, "  Citationlist: (%Lu)\n",
		    ajListGetLength(outseq->Reflist));
	it = ajListIterNewread(outseq->Reflist);

	while((curref = (AjPSeqRef) ajListIterGet(it)))
	{
	    ajFmtPrintF(outseq->File, "    Number: %u\n",
			curref->Number);

	    if(ajStrGetLen(curref->Position))
		ajFmtPrintF(outseq->File, "      Position: '%S'\n",
			    curref->Position);

	    if(ajStrGetLen(curref->Groupname))
		ajFmtPrintF(outseq->File, "      Groupname: '%S'\n",
			    curref->Groupname);

	    if(ajStrGetLen(curref->Authors))
		ajFmtPrintF(outseq->File, "      Authors: '%S'\n",
			    curref->Authors);

	    if(ajStrGetLen(curref->Title))
		ajFmtPrintF(outseq->File, "      Title: '%S'\n",
			    curref->Title);

	    if(ajStrGetLen(curref->Comment))
		ajFmtPrintF(outseq->File, "      Comment: '%S'\n",
			    curref->Comment);

	    if(ajStrGetLen(curref->Xref))
		ajFmtPrintF(outseq->File, "      Xref: '%S'\n",
			    curref->Xref);

	    if(ajStrGetLen(curref->Location))
		ajFmtPrintF(outseq->File, "      Location: '%S'\n",
			    curref->Location);

	    if(ajStrGetLen(curref->Loctype))
		ajFmtPrintF(outseq->File, "      Loctype: '%S'\n",
			    curref->Loctype);
	}

	ajListIterDel(&it);
    }

    ajFmtPrintF(outseq->File, "  Usa: '%S'\n", ajStrConstS(outseq->Usa));
    ajFmtPrintF(outseq->File, "  Ufo: '%S'\n", ajStrConstS(outseq->Ufo));
    ajFmtPrintF(outseq->File, "  Input format: '%S'\n",
		ajStrConstS(outseq->Informatstr));
    ajFmtPrintF(outseq->File, "  Output format: '%S'\n",
		ajStrConstS(outseq->Formatstr));
    ajFmtPrintF(outseq->File, "  Filename: '%S'\n",
                ajStrConstS(outseq->Filename));
    ajFmtPrintF(outseq->File, "  Output directory: '%S'\n",
		ajStrConstS(outseq->Directory));
    ajFmtPrintF(outseq->File, "  Entryname: '%S'\n",
		ajStrConstS(outseq->Entryname));
    ajFmtPrintF(outseq->File, "  File name: '%F'\n",
		outseq->File);
    ajFmtPrintF(outseq->File, "  Known file name: '%D'\n",
		outseq->Knownfile);
    ajFmtPrintF(outseq->File, "  Extension: '%S'\n",
		ajStrConstS(outseq->Extension));
    ajFmtPrintF(outseq->File, "  Single: '%B'\n", outseq->Single);
    ajFmtPrintF(outseq->File, "  Features: '%B'\n", outseq->Features);
    ajFmtPrintF(outseq->File, "  Rev: '%B'\n", outseq->Rev);
    ajFmtPrintF(outseq->File, "  Circular: '%B'\n", outseq->Circular);
    ajFmtPrintF(outseq->File, "  Offset: '%d'\n", outseq->Offset);
    ajFmtPrintF(outseq->File, "  Count: '%d'\n", outseq->Count);
    ajFmtPrintF(outseq->File, "  Documentation:...\n%S\n",
		ajStrConstS(outseq->Doc));
    ajFmtPrintF(outseq->File, "  Features filename: '%S'\n",
		outseq->FtFilename);
    ajFmtPrintF(outseq->File, "  Features format: '%S'\n",
		outseq->FtFormat);
    ajFmtPrintF(outseq->File, "  Feature table size: '%u'\n",
		ajFeattableGetSize(outseq->Fttable));

    if(outseq->Accuracy)
    {
	ilen = ajStrGetLen(outseq->Seq);
	ajFmtPrintF(outseq->File, "  Accuracy: \n");

	for(i=0; i<ilen;i+=20)
	{
	    ajFmtPrintF(outseq->File, "    ");
	    jend = i+20;

	    if(jend > ilen)
		jend = ilen;

	    for(j=i;j<jend;j++)
		ajFmtPrintF(outseq->File, " %2d", (ajint) outseq->Accuracy[j]);

	    ajWritebinNewline(outseq->File);
	}
    }

    seqSeqFormat(ajStrGetLen(outseq->Seq), &sf);
    sf->numright = ajTrue;
    sf->numleft  = ajTrue;
    sf->numjust  = ajTrue;
    sf->tab      = 1;
    sf->spacer   = 11;
    sf->width    = 50;
    
    seqWriteSeq(outseq, sf);
    seqFormatDel(&sf);
    
    return;
}




/* @funcstatic seqWriteSeq ****************************************************
**
** Writes an output sequence. The format and all other information is
** already stored in the output sequence object and the formatting structure.
**
** @param [u] outseq [AjPSeqout] Output sequence.
** @param [r] sf [const SeqPSeqFormat] Output formatting structure.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void seqWriteSeq(AjPSeqout outseq, const SeqPSeqFormat sf)
{
    /* code adapted from what readseq did */
    
    static ajuint maxSeqWidth       = 250;
    static const char* defNocountSymbols = "_.-?";
    
    ajuint i = 0;
    ajuint j = 0;
    ajuint l = 0;
    ajuint ibase    = 0;
    ajuint linesout = 0;
    ajuint seqlen;
    const char *seq;
    const char *idword;
    char *cp;
    char s[1024];			/* the output line */
    
    char nameform[20];
    char numform[20];
    char nocountsymbols[20];
    
    ajuint width;
    ajuint l1;				/* can be negative */
    
    AjPFile file;
    FILE* outf;
    
    /*
       ajint numline = 0;
       
       ajint namewidth = sf->namewidth;
       ajint numwidth = sf->numwidth;
       ajint spacer = sf->spacer;
       ajint tab = sf->tab;
       AjBool nameleft = sf->nameleft;
       AjBool nameright = sf->nameright;
       AjBool numleft = sf->numleft;
       AjBool numright = sf->numright;
       AjBool numjust = sf->numjust;
       AjBool skipbefore = sf->skipbefore;
       AjBool skipafter = sf->skipafter;
       AjBool baseonlynum = sf->baseonlynum;
       AjBool pretty = sf->pretty;
       char *endstr = sf->endstr;
       char *leftstr = sf->leftstr;
       */
    
    ajDebug("seqWriteSeq\n");


    seqlen = ajStrGetLen(outseq->Seq);
    seq    = ajStrGetPtr(outseq->Seq);
    width  = sf->width;
    l1     = sf->linepos;
    file   = outseq->File;
    outf   = ajFileGetFileptr(file);


    /* if(sf->numline) numline = 1;*/
    
    if(sf->nameleft || sf->nameright)
	sprintf(nameform, "%%%u.%us ",sf->namewidth,sf->namewidth);

    if(sf->numline)
	sprintf(numform, "%%%us ",sf->numwidth);
    else
	sprintf(numform, "%%%ud",sf->numwidth);
    
    strcpy( nocountsymbols, defNocountSymbols);

    if(sf->baseonlynum)
    {				      /* add gap character to skips */
	if(strchr(nocountsymbols,sf->gapchar)==NULL)
	{
	    strcat(nocountsymbols," ");
	    nocountsymbols[strlen(nocountsymbols)-1]= sf->gapchar;
	}

	if(sf->domatch &&	 /* remove gap character from skips */
	   (cp=strchr(nocountsymbols,sf->matchchar))!=NULL)
	    *cp= ' ';
    }
    
    if(sf->numline)
	idword= "";
    else
	idword = ajStrGetPtr(outseq->Name);
    
    width = AJMIN(width,maxSeqWidth);
    
    i=0;				/* seqpos position in seq[]*/
    l=0;		     /* linepos position in output line s[] */
    
    ibase = 1;				/* base count */
    
    while(i < seqlen)
    {
	
	if(l1 == 0)
	{
	    /* start of a new line */
	    if(sf->skipbefore)
	    {
		fprintf(outf, "\n");   /* blank line before writing */
		linesout++;
	    }

	    if(*(sf->leftstr))
		fprintf(outf, "%s", sf->leftstr); /* string at line start */

	    if(sf->nameleft)
		fprintf(outf, nameform, idword);

	    if(sf->numleft)
	    {
		if(sf->numline)
		    fprintf(outf, numform, "");
		else
		    fprintf(outf, numform, ibase);
	    }

	    for(j=0; j < sf->tab; j++)
		fputc(' ',outf);
	}
	
	l1++;			     /* don't count spaces for width */

	if(sf->numline)
	{
	    if(sf->spacer==seqSpaceAll ||
	       (sf->spacer != 0 && (l+1) % sf->spacer == 1))
	    {
		if(sf->numline)
                    fputc(' ',outf);

		s[l++] = ' ';
	    }
	
	    if(l1 % 10 == 1 || l1 == width)
	    {
		if(sf->numline)
                    fprintf(outf,"%-9u ",i+1);

		s[l++]= '|';		/* == put a number here */
	    }
	    else
                s[l++]= ' ';

	    i++;
	}
	else
	{
	    if(sf->spacer==seqSpaceAll ||
	       (sf->spacer != 0 && (l+1) % sf->spacer == 1))
		s[l++] = ' ';

	    if(!sf->baseonlynum)
		ibase++;
	    else if(0==strchr(nocountsymbols,seq[i]))
		ibase++;

	    s[l++] = seq[i++];
	}
	
	if(l1 == width || i == seqlen)
	{
	    if(sf->pretty || sf->numjust)
		for( ; l1<width; l1++)
		{
		    if(sf->spacer==seqSpaceAll ||
		       (sf->spacer != 0 && (l+1) % sf->spacer == 1))
			s[l++] = ' ';

		    s[l++]=' ';		/* pad with blanks */
		}

	    s[l] = '\0';
	    l = 0; l1 = 0;

	    if(!sf->numline)
	    {
		fprintf(outf,"%s",s);

		if(sf->numright || sf->nameright)
		    fputc(' ',outf);

		if(sf->numright)
		    fprintf(outf,numform, ibase-1);

		if(sf->nameright)
		    fprintf(outf, nameform,idword);

		if(i == seqlen)
		    fprintf(outf,"%s",sf->endstr);
	    }

	    fprintf(outf, "\n");
	    linesout++;

	    if(sf->skipafter)
	    {
		fprintf(outf, "\n");
		linesout++;
	    }
	}
    }
    
    return;
}




/* @funcstatic seqoutUfoLocal *************************************************
**
** Tests whether a sequence output object will write features to the
** sequence output file. The alternative is to use a separate UFO.
**
** @param [r] thys [const AjPSeqout] Sequence output object.
** @return [AjBool] ajTrue if the features will be written to the sequence
**
** @release 2.0.0
** @@
******************************************************************************/

static AjBool seqoutUfoLocal(const AjPSeqout thys)
{
    ajDebug("seqoutUfoLocal Features %B Ufo %d '%S'\n",
	    thys->Features, ajStrGetLen(thys->Ufo), thys->Ufo);

    if(thys->Features && !ajStrGetLen(thys->Ufo))
	return ajTrue;

    return ajFalse;
}




/* @funcstatic seqoutUsaProcess ***********************************************
**
** Converts a USA Universal Sequence Address into an open output file.
**
** First tests for format:: and sets this if it is found
**
** Then looks for file:id and opens the file.
** In this case the file position is not known and sequence reading
** will have to scan for the entry/entries we need.
**
** @param [u] thys [AjPSeqout] Sequence output definition.
** @return [AjBool] ajTrue on success.
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool seqoutUsaProcess(AjPSeqout thys)
{    
    AjBool fmtstat;
    AjBool regstat;
    
#ifdef __CYGWIN__
    AjPStr usatmp = NULL;
#endif /* __CYGWIN__ */

    ajDebug("seqoutUsaProcess\n");
    if(!seqoutRegFmt)
#ifndef WIN32
	seqoutRegFmt = ajRegCompC("^([A-Za-z0-9-]*)::?(.*)$");
    /* \1 format */
    /* \2 remainder */
#else /* WIN32 */
    /* Windows file names can start with e.g.: 'C:\' */
    /* -> Require that format names have at least 2 letters */
    seqoutRegFmt = ajRegCompC("^([A-Za-z0-9][A-Za-z0-9-][A-Za-z0-9-]*)"
                              "::?(.*)$");
    /* \1 format */
    /* \2 remainder */
#endif /* !WIN32 */


    if(!seqoutRegId)			  /* \1 is filename \3 is the qryid */
	seqoutRegId = ajRegCompC("^(.*)$");
    
    ajStrAssignS(&seqoutUsaTest, thys->Usa);

#ifdef __CYGWIN__
    if(*(ajStrGetPtr(seqoutUsaTest)+1)==':')
    {
	usatmp = ajStrNew();
        ajFmtPrintS(&usatmp,"/cygdrive/%c/%s",*ajStrGetPtr(seqoutUsaTest),
		    ajStrGetPtr(seqoutUsaTest)+2);
        ajStrAssignRef(&seqoutUsaTest,usatmp);
        ajStrDel(&usatmp);
    }
#endif /* __CYGWIN__ */

    ajDebug("output USA to test: '%S'\n\n", seqoutUsaTest);
    
    fmtstat = ajRegExec(seqoutRegFmt, seqoutUsaTest);
    ajDebug("format regexp: %B\n", fmtstat);
    
    if(fmtstat)
    {
	ajRegSubI(seqoutRegFmt, 1, &thys->Formatstr);
	ajStrAssignEmptyC(&thys->Formatstr, seqOutFormat[0].Name);
	/* default  unknown */

	ajRegSubI(seqoutRegFmt, 2, &seqoutUsaTest);
	ajDebug("found format %S\n", thys->Formatstr);

	if(!seqoutFindOutFormat(thys->Formatstr, &thys->Format))
	{
	    ajDebug("unknown format '%S'\n", thys->Formatstr);

	    return ajFalse;
	}
    }
    else
	ajDebug("no format specified in USA\n");

    ajDebug("\n");
    
    regstat = ajRegExec(seqoutRegId, seqoutUsaTest);
    ajDebug("file:id regexp: %B\n", regstat);
    
    if(regstat)
    {
	ajRegSubI(seqoutRegId, 1, &thys->Filename);
	ajDebug("found filename %S single: %B dir: '%S'\n",
		thys->Filename, thys->Single, thys->Directory);

	if(thys->Single)
	    ajDebug("single output file per sequence, open later\n");
	else
	{
	    if(thys->Knownfile)
		thys->File = thys->Knownfile;
	    else
		thys->File = ajFileNewOutNamePathS(thys->Filename,
                                                   thys->Directory);

	    if(!thys->File)
	    {
		if(ajStrGetLen(thys->Directory))
		    ajErr("failed to open filename '%S' in directory '%S'",
			  thys->Filename, thys->Directory);
		else
		    ajErr("failed to open filename '%S'", thys->Filename);

		return ajFalse;
	    }
	}
    }
    else
	ajDebug("no filename specified\n");

    ajDebug("\n");
    
    return ajTrue;
}




/* @section sequence output opening *****************************************
**
** These functions use the contents of a sequence output object and
** update them.
**
** @fdata [AjPSeqout]
** @fcategory modify
**
** @nam3rule Clear Reset output object
** @nam4rule ClearUsa Reset output object and set new Usa
** @nam3rule Close Close output file
** @nam4rule CloseEmpty Close output file and check it is empty
** @nam3rule Flush Flush saved sequences to output file and clear list
** @nam3rule Open Open output file
** @nam4rule OpenFilename Open named output file
** @nam3rule Reset Clear ready to accept further sequences
**
** @argrule * seqout [AjPSeqout] Sequence output object
** @argrule OpenFilename name [const AjPStr] Output filename
** @argrule ClearUsa usa [const AjPStr] Uniform sequence address
**
** @valrule * [void]
** @valrule *Open [AjBool] True on success
**
******************************************************************************/




/* @func ajSeqoutClear ********************************************************
**
** Clears a Sequence output object back to "as new" condition
**
** @param [u] seqout [AjPSeqout] Sequence output object
** @category modify [AjPSeqout] Resets ready for reuse.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajSeqoutClear(AjPSeqout seqout)
{

    AjPStr ptr = NULL;
    AjPSeqRef  tmpref  = NULL;
    AjPSeqXref tmpxref = NULL;
    AjPSeqGene tmpgene = NULL;

    ajDebug("ajSeqoutClear called\n");

    ajStrSetClear(&seqout->Name);
    ajStrSetClear(&seqout->Acc);
    ajStrSetClear(&seqout->Sv);
    ajStrSetClear(&seqout->Gi);
    ajStrSetClear(&seqout->Tax);
    ajStrSetClear(&seqout->Taxid);
    ajStrSetClear(&seqout->Organelle);
    ajStrSetClear(&seqout->Desc);
    ajStrSetClear(&seqout->Type);
    ajStrSetClear(&seqout->Outputtype);
    ajStrSetClear(&seqout->Full);
    ajStrSetClear(&seqout->Doc);
    ajStrSetClear(&seqout->Usa);
    ajStrSetClear(&seqout->Ufo);
    ajStrSetClear(&seqout->Informatstr);
    ajStrSetClear(&seqout->Formatstr);
    ajStrSetClear(&seqout->Filename);
    ajStrSetClear(&seqout->Directory);
    ajStrSetClear(&seqout->Entryname);
    ajStrSetClear(&seqout->Extension);
    ajStrSetClear(&seqout->Seq);
    seqout->EType  = 0;
    seqout->Rev    = ajFalse;
    seqout->Format = 0;


    if(seqout->File)
    {
        if(seqout->Cleanup)
            (*seqout->Cleanup)(seqout);

	if(seqout->Knownfile)
	    seqout->File = NULL;
	else
	    ajFileClose(&seqout->File);
    }

    seqout->Cleanup = NULL;

    seqout->Count    = 0;
    seqout->Single   = ajFalse;
    seqout->Features = ajFalse;

    while(ajListstrPop(seqout->Acclist,&ptr))
	ajStrDel(&ptr);

    while(ajListstrPop(seqout->Keylist,&ptr))
	ajStrDel(&ptr);

    while(ajListstrPop(seqout->Taxlist,&ptr))
	ajStrDel(&ptr);

    while(ajListPop(seqout->Genelist,(void **)&tmpgene))
	ajSeqgeneDel(&tmpgene);

    while(ajListPop(seqout->Reflist,(void **)&tmpref))
	ajSeqrefDel(&tmpref);

    while(ajListstrPop(seqout->Cmtlist,&ptr))
	ajStrDel(&ptr);

    while(ajListPop(seqout->Xreflist,(void **)&tmpxref))
	ajSeqxrefDel(&tmpxref);

    ajSeqdescClear(seqout->Fulldesc);
    ajFeattabOutClear(&seqout->Ftquery);
    AJCSET0(seqout->Accuracy, seqout->Qualsize);

    return;
}




/* @func ajSeqoutClearUsa *****************************************************
**
** Creates or resets a sequence output object using a new Universal
** Sequence Address
**
** @param [u] seqout [AjPSeqout] Sequence output object.
** @param [r] usa [const AjPStr] USA
** @return [void]
** @category modify [AjPSeqout] Resets using a new USA
**
** @release 5.0.0
** @@
******************************************************************************/

void ajSeqoutClearUsa(AjPSeqout seqout, const AjPStr usa)
{
    ajSeqoutClear(seqout);

    ajStrAssignS(&seqout->Usa, usa);

    return;
}




/* @func ajSeqoutClose ********************************************************
**
** Close a sequence output file. For formats that save everything up
** and write at the end, call the Write function first.
**
** @param [u] seqout [AjPSeqout] Sequence output
** @return [void]
**
** @release 4.1.0
** @@
******************************************************************************/

void ajSeqoutClose(AjPSeqout seqout)
{

    ajDebug("ajSeqoutClose '%F'\n", seqout->File);

    if(seqOutFormat[seqout->Format].Save)
    {
	/* Calling funclist seqOutFormat() */
	(*seqOutFormat[seqout->Format].Write)(seqout);
    }

    if(seqout->Cleanup)
        (*seqout->Cleanup)(seqout);
    seqout->Cleanup = NULL;

    if(seqout->File && !seqout->Count)
        ajWarn("No sequences written to output file '%F'", seqout->File);

    if(seqout->Knownfile)
	seqout->File = NULL;
    else
	ajFileClose(&seqout->File);

    return;
}




/* @func ajSeqoutCloseEmpty ***************************************************
**
** Close a sequence output file with no output.
**
** Warns if the file has been written to.
**
** Closes the file so that no warning message is written bu the destructor
**
** @param [u] seqout [AjPSeqout] Sequence output
** @return [void]
**
** @release 6.5.0
** @@
******************************************************************************/

void ajSeqoutCloseEmpty(AjPSeqout seqout)
{

    ajDebug("ajSeqoutCloseEmpty '%F'\n", seqout->File);

    if(seqOutFormat[seqout->Format].Save)
    {
	/* Calling funclist seqOutFormat() */
	(*seqOutFormat[seqout->Format].Write)(seqout);
    }

    if(seqout->Cleanup)
        (*seqout->Cleanup)(seqout);
    seqout->Cleanup = NULL;

    if(seqout->File && seqout->Count)
        ajWarn("Not empty file: %d sequence(s) written to output file '%F'",
               seqout->Count, seqout->File);

    if(seqout->Knownfile)
	seqout->File = NULL;
    else
	ajFileClose(&seqout->File);

    return;
}




/* @func ajSeqoutFlush ********************************************************
**
** Flush output to a sequence output file. For formats that save everything up
** and write at the end, call the Write function first.
**
** Do not close the file. It can be reused for more output
**
** @param [u] seqout [AjPSeqout] Sequence output
** @return [void]
**
** @release 6.1.0
** @@
******************************************************************************/

void ajSeqoutFlush(AjPSeqout seqout)
{

    ajDebug("ajSeqoutFlush '%F'\n", seqout->File);

    if(seqOutFormat[seqout->Format].Save)
    {
	/* Calling funclist seqOutFormat() */
	(*seqOutFormat[seqout->Format].Write)(seqout);
    }

    seqWriteListClear(seqout);

    if(seqout->Cleanup)
        (*seqout->Cleanup)(seqout);

    return;
}




/* @func ajSeqoutOpen *********************************************************
**
** If the file is not yet open, calls seqoutUsaProcess to convert the USA into
** an open output file stream.
**
** Returns the results in the AjPSeqout object.
**
** @param [w] seqout [AjPSeqout] Sequence output object.
** @return [AjBool] ajTrue on success.
** @category modify [AjPSeqout] If the file is not yet open, calls
**                seqoutUsaProcess
**
** @release 1.0.0
** @@
******************************************************************************/

AjBool ajSeqoutOpen(AjPSeqout seqout)
{
    AjBool ret = ajFalse;

    if(seqout->Ftquery)
	ajDebug("ajSeqoutOpen dir '%S' qrydir '%S'\n",
		seqout->Directory, seqout->Ftquery->Directory);
    else
	ajDebug("ajSeqoutOpen dir '%S' (no ftquery)\n",
		seqout->Directory);

    ret = seqoutUsaProcess(seqout);

    if(!ret)
	return ajFalse;

    if(!seqout->Features)
	return ret;

    ajStrAssignEmptyS(&seqout->Ftquery->Seqname, seqout->Name);
    ajFeattabOutSetBasename(seqout->Ftquery, seqout->Filename);
    ret = ajFeattabOutSet(seqout->Ftquery, seqout->Ufo);

    return ret;
}




/* @func ajSeqoutOpenFilename *************************************************
**
** Opens an output file for sequence writing. 'stdout' and 'stderr' are
** special cases using standard output and standard error respectively.
**
** @param [u] seqout [AjPSeqout] Sequence output object.
** @param [r] name [const AjPStr] Output filename.
** @return [AjBool] ajTrue on success.
** @category modify [AjPSeqout] Opens an output file for sequence
**                writing.
**
** @release 4.1.0
** @@
******************************************************************************/

AjBool ajSeqoutOpenFilename(AjPSeqout seqout, const AjPStr name)
{
    AjBool single;
    AjBool features;

    single   = seqout->Single;
    features = seqout->Features;

    if(ajStrMatchCaseC(name, "stdout"))
	single = ajFalse;

    if(ajStrMatchCaseC(name, "stderr"))
	single = ajFalse;

    if(single)
    {				     /* OK, but nothing to open yet */
	ajStrAssignEmptyS(&seqout->Extension, seqout->Formatstr);

	return ajTrue;
    }
    else
    {
	seqout->File = ajFileNewOutNameS(name);

	if(seqout->File)
	    return ajTrue;
    }

    if(features)
	ajWarn("ajSeqFileNewOut features not yet implemented");

    return ajFalse;
}




/* @func ajSeqoutReset ********************************************************
**
** Clears a Sequence output object ready to accept further sequences
**
** @param [u] seqout [AjPSeqout] Sequence output object
** @category modify [AjPSeqout] Resets ready for reuse.
** @return [void]
**
** @release 6.1.0
** @@
******************************************************************************/

void ajSeqoutReset(AjPSeqout seqout)
{

    AjPStr ptr = NULL;
    AjPSeqRef  tmpref  = NULL;
    AjPSeqXref tmpxref = NULL;
    AjPSeqGene tmpgene = NULL;

    ajDebug("ajSeqoutClear called\n");

    ajStrSetClear(&seqout->Name);
    ajStrSetClear(&seqout->Acc);
    ajStrSetClear(&seqout->Sv);
    ajStrSetClear(&seqout->Gi);
    ajStrSetClear(&seqout->Tax);
    ajStrSetClear(&seqout->Taxid);
    ajStrSetClear(&seqout->Organelle);
    ajStrSetClear(&seqout->Desc);
    ajStrSetClear(&seqout->Type);
    ajStrSetClear(&seqout->Outputtype);
    ajStrSetClear(&seqout->Full);
    ajStrSetClear(&seqout->Doc);
    ajStrSetClear(&seqout->Usa);
    ajStrSetClear(&seqout->Ufo);
    ajStrSetClear(&seqout->Informatstr);
    ajStrSetClear(&seqout->Formatstr);
    ajStrSetClear(&seqout->Filename);
    ajStrSetClear(&seqout->Directory);
    ajStrSetClear(&seqout->Entryname);
    ajStrSetClear(&seqout->Extension);
    ajStrSetClear(&seqout->Seq);
    seqout->EType  = 0;
    seqout->Rev    = ajFalse;
    seqout->Format = 0;


    if(seqout->File)
    {
        if(seqout->Cleanup)
            (*seqout->Cleanup)(seqout);

	if(seqout->Knownfile)
	    seqout->File = NULL;
	else
	    ajFileClose(&seqout->File);
    }

    seqout->Cleanup = NULL;

    seqout->Count    = 0;
    seqout->Single   = ajFalse;
    seqout->Features = ajFalse;

    while(ajListstrPop(seqout->Acclist,&ptr))
	ajStrDel(&ptr);

    while(ajListstrPop(seqout->Keylist,&ptr))
	ajStrDel(&ptr);

    while(ajListstrPop(seqout->Taxlist,&ptr))
	ajStrDel(&ptr);

    while(ajListPop(seqout->Genelist,(void **)&tmpgene))
	ajSeqgeneDel(&tmpgene);

    while(ajListPop(seqout->Reflist,(void **)&tmpref))
	ajSeqrefDel(&tmpref);

    while(ajListstrPop(seqout->Cmtlist,&ptr))
	ajStrDel(&ptr);

    while(ajListPop(seqout->Xreflist,(void **)&tmpxref))
	ajSeqxrefDel(&tmpxref);

    ajSeqdescClear(seqout->Fulldesc);
    ajFeattabOutClear(&seqout->Ftquery);
    AJCSET0(seqout->Accuracy, seqout->Qualsize);

    return;
}




/* @section set ***************************************************************
**
** @fdata [AjPSeqout]
** @fcategory modify
**
** @nam3rule Set Set values
** @nam4rule Format Set output format
** @nam4rule Name Set output name
** @nam5rule NameDefault Set output name to default if not already set
**
** @suffix C [char*] C character string
** @suffix S [AjPStr] string object
**
** @argrule Set seqout [AjPSeqout] Sequence output object
** @argrule NameDefault multi [AjBool] True if number is to be appended
** @argrule C txt [const char*] Format name
** @argrule S str [const AjPStr] Format name
**
** @valrule * [AjBool] True on success
**
******************************************************************************/




/* @func ajSeqoutSetFormatC ***************************************************
**
** Sets the output format. Currently hard coded but will be replaced
** in future by a variable.
**
** @param [u] seqout [AjPSeqout] Sequence output object.
** @param [r] txt [const char *] Output format.
** @return [AjBool] ajTrue on success.
**
** @release 5.0.0
** @@
******************************************************************************/

AjBool ajSeqoutSetFormatC(AjPSeqout seqout, const char* txt)
{
    AjPStr fmt = NULL;
    AjBool ret;

    fmt = ajStrNewC(txt);
    ret = ajSeqoutSetFormatS(seqout,fmt);
    ajStrDel(&fmt);

    return ret;
}




/* @func ajSeqoutSetFormatS ***************************************************
**
** Sets the output format. Currently hard coded but will be replaced
** in future by a variable.
**
** @param [u] seqout [AjPSeqout] Sequence output object.
** @param [r] str [const AjPStr] Output format.
** @return [AjBool] ajTrue on success.
**
** @release 5.0.0
** @@
******************************************************************************/

AjBool ajSeqoutSetFormatS(AjPSeqout seqout, const AjPStr str)
{
    AjPStr fmt = NULL;

    ajDebug("ajSeqoutSetFormatS '%S'\n", str);
    ajStrAssignS(&fmt, str);
    ajSeqoutstrGetFormatDefault(&fmt);

    ajStrAssignEmptyS(&seqout->Formatstr, fmt);
    ajDebug("... output format set to '%S'\n", fmt);

    ajStrDel(&fmt);

    return ajTrue;
}




/* @func ajSeqoutSetNameDefaultC **********************************************
**
** Provides a unique (for this program run) name for a sequence.
**
** @param [w] seqout [AjPSeqout] Sequence output object
** @param [r] multi [AjBool] If true, appends a number to the name.
** @param [r] txt [const char*] Name set by caller
** @return [AjBool] True on success
**
** @release 5.0.0
** @@
******************************************************************************/

AjBool ajSeqoutSetNameDefaultC(AjPSeqout seqout,
			       AjBool multi, const char* txt)
{
    static ajint count = 0;

    if(ajStrGetLen(seqout->Name))
    {
	ajDebug("ajSeqoutSetNameDefaultC already has a name '%S'\n",
		seqout->Name);

	return ajFalse;
    }

    if (*txt)
    {
	if(multi && count)
	    ajFmtPrintS(&seqout->Name, "%s_%3.3d", txt, ++count);
	else
	{
	    ajStrAssignC(&seqout->Name, txt);
	    ++count;
	}
    }
    else
    {
	if(multi)
	    ajFmtPrintS(&seqout->Name, "EMBOSS_%3.3d", ++count);
	else
	{
	    ajStrAssignC(&seqout->Name, "EMBOSS");
	    ++count;
	}
    }

    ajDebug("ajSeqoutSetNameDefaultC set to '%S'\n", seqout->Name);

    return ajTrue;
}




/* @func ajSeqoutSetNameDefaultS **********************************************
**
** Provides a unique (for this program run) name for a sequence.
**
** @param [w] seqout [AjPSeqout] Sequence output object
** @param [r] multi [AjBool] If true, appends a number to the name.
** @param [r] str [const AjPStr] Name set by caller
** @return [AjBool] True on success
**
** @release 5.0.0
** @@
******************************************************************************/

AjBool ajSeqoutSetNameDefaultS(AjPSeqout seqout,
			    AjBool multi, const AjPStr str)
{
    static ajint count = 0;

    if(ajStrGetLen(seqout->Name))
    {
	ajDebug("ajSeqoutSetNameDefaultS already has a name '%S'\n",
		seqout->Name);

	return ajFalse;
    }

    if (ajStrGetLen(str))
    {
	if(multi && count)
	    ajFmtPrintS(&seqout->Name, "%S_%3.3d", str, ++count);
	else
	{
	    ajStrAssignS(&seqout->Name, str);
	    ++count;
	}
    }
    else
    {
	if(multi)
	    ajFmtPrintS(&seqout->Name, "EMBOSS_%3.3d", ++count);
	else
	{
	    ajStrAssignC(&seqout->Name, "EMBOSS");
	    ++count;
	}
    }

    ajDebug("ajSeqoutSetNameDefaultS set to '%S'\n", seqout->Name);

    return ajTrue;
}




/* @funcstatic seqDbName ******************************************************
**
** Adds the database name (if any) to the name provided.
**
** @param [w] name [AjPStr*] Derived name.
** @param [r] db [const AjPStr] Database name (if any)
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void seqDbName(AjPStr* name, const AjPStr db)
{
    AjPStr tmpname = NULL;

    if(!ajStrGetLen(db))
	return;

    ajStrAssignS(&tmpname, *name);
    ajFmtPrintS(name, "%S:%S", db, tmpname);

    ajStrDel(&tmpname);

    return;
}




/* @section print *************************************************************
**
** @fdata [AjPSeqout]
**
** Print internal details for use by entrails
**
** @nam3rule Print Print internal details
** @nam3rule Printbook Print internal details as docbook
** @nam3rule Printhtml Print internal details as html
** @nam3rule Printwiki Print internal details as wikitext
** @nam4rule Format Print details of sequence output formats
**
** @argrule Print outf [AjPFile] Output file
** @argrule Printbook outf [AjPFile] Output file
** @argrule Printhtml outf [AjPFile] Output file
** @argrule Printwiki outf [AjPFile] Output file
** @argrule Print full [AjBool] Print full details
**
** @valrule * [void]
** @fcategory misc
**
******************************************************************************/




/* @func ajSeqoutPrintFormat **************************************************
**
** Reports the internal data structures
**
** @param [u] outf [AjPFile] Output file
** @param [r] full [AjBool] Full report (usually ajFalse)
** @return [void]
**
** @release 5.0.0
** @@
******************************************************************************/

void ajSeqoutPrintFormat(AjPFile outf, AjBool full)
{

    ajint i = 0;

    (void) full;	 /* make it used - no extra detail reported */

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# Sequence output formats\n");
    ajFmtPrintF(outf, "# Alias Alias name\n");
    ajFmtPrintF(outf, "# Single: If true, write each sequence to new file\n");
    ajFmtPrintF(outf, "# Save: If true, save sequences, write when closed\n");
    ajFmtPrintF(outf, "# Nuc   Can read nucleotide input\n");
    ajFmtPrintF(outf, "# Pro   Can read protein input\n");
    ajFmtPrintF(outf, "# Feat  Can read feature annotation\n");
    ajFmtPrintF(outf, "# Gap   Can read gap characters\n");
    ajFmtPrintF(outf, "# Mset  Can read seqsetall (multiple seqsets)\n");
    ajFmtPrintF(outf, "# Name          Alias Single Save  Pro  Nuc Feat  "
		"Gap MSet Description\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "OutFormat {\n");

    for(i=0; seqOutFormat[i].Name; i++)
    {
	ajFmtPrintF(outf,
		    "  %-15s %3B    %3B  %3B  %3B  %3B  %3B  %3B  %3B \"%s\"\n",
		    seqOutFormat[i].Name,
		    seqOutFormat[i].Alias,
		    seqOutFormat[i].Single,
		    seqOutFormat[i].Save,
		    seqOutFormat[i].Nucleotide,
		    seqOutFormat[i].Protein,
		    seqOutFormat[i].Feature,
		    seqOutFormat[i].Gap,
		    seqOutFormat[i].Multiset,
		    seqOutFormat[i].Desc);
    }

    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajSeqoutPrintbookFormat **********************************************
**
** Reports the internal data structures as EMBOSS book format
**
** @param [u] outf [AjPFile] Output file
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ajSeqoutPrintbookFormat(AjPFile outf)
{

    ajint i = 0;
    ajint j = 0;
    AjPStr namestr = NULL;
    AjPList fmtlist;
    AjPStr* names;

    fmtlist = ajListstrNew();

    ajFmtPrintF(outf, "<para>The supported sequence formats are summarised "
                "in the table below. "
                "The columns are as follows: "
                "<emphasis>Input format</emphasis> (format name), "
                "<emphasis>Output format</emphasis> (format name), "
                "<emphasis>Sngl</emphasis> "
                "(indicates whether each sequence is written to a new file. "
                "This behaviour is the default and can be set by the "
                "<option>-ossingle</option> command line qualifier.  "
                "<emphasis>Save</emphasis> (indicates that sequence data is "
                "stored internally and written when the output is closed. "
                "This is needed for 'interleaved' formats such as Phylip "
                "and MSF), <emphasis>Try</emphasis> (indicates whether the "
                "format can be detected automatically on input), "
                "<emphasis>Nuc</emphasis> (\"true\" indicates nucleotide "
                "sequence data may be represented), <emphasis>Pro</emphasis> "
                "(\"true\" indicates protein sequence data may be represented, "
                "<emphasis>Feat</emphasis> (whether the format includes "
                "feature annotation data. "
                "EMBOSS can also read feature data from a separate "
                "feature file).  "
                "<emphasis>Gap</emphasis> (whether the format supports "
                "sequence data with gap characters, for example the results "
                "of an alignment), "
                "<emphasis>Mset</emphasis> (\"true\" indicates that more "
                "than one set of sequences can be stored in a single file. "
                "This is used by, for example, phylogenetic analysis "
                "applications to store many versions of a multiple alignment "
                "for statistical analysis) and "
                "<emphasis>Description</emphasis> (short description of "
                "the format).</para> \n");


    ajFmtPrintF(outf, "<table frame=\"box\" rules=\"cols\">\n");
    ajFmtPrintF(outf, "  <caption>Output sequence formats</caption>\n");
    ajFmtPrintF(outf, "  <thead>\n");
    ajFmtPrintF(outf, "    <tr align=\"center\">\n");
    ajFmtPrintF(outf, "      <th>Input Format</th>\n");
    ajFmtPrintF(outf, "      <th>Sngl</th>\n");
    ajFmtPrintF(outf, "      <th>Save</th>\n");
    ajFmtPrintF(outf, "      <th>Nuc</th>\n");
    ajFmtPrintF(outf, "      <th>Pro</th>\n");
    ajFmtPrintF(outf, "      <th>Feat</th>\n");
    ajFmtPrintF(outf, "      <th>Gap</th>\n");
    ajFmtPrintF(outf, "      <th>Multi</th>\n");
    ajFmtPrintF(outf, "      <th>Description</th>\n");
    ajFmtPrintF(outf, "    </tr>\n");
    ajFmtPrintF(outf, "  </thead>\n");
    ajFmtPrintF(outf, "  <tbody>\n");

    for(i=1; seqOutFormat[i].Name; i++)
    {
	if(!seqOutFormat[i].Alias)
        {
            namestr = ajStrNewC(seqOutFormat[i].Name);
            ajListPush(fmtlist, namestr);
            namestr = NULL;
        }
    }

    ajListSort(fmtlist, &ajStrVcmp);
    ajListstrToarray(fmtlist, &names);

    for(i=0; names[i]; i++)
    {
        for(j=0; seqOutFormat[j].Name; j++)
        {
	    if(ajStrMatchC(names[i],seqOutFormat[j].Name))
            {
                ajFmtPrintF(outf, "    <tr>\n");
                ajFmtPrintF(outf, "      <td>%s</td>\n",
                            seqOutFormat[j].Name);
                ajFmtPrintF(outf, "      <td>%B</td>\n",
                            seqOutFormat[j].Single);
                ajFmtPrintF(outf, "      <td>%B</td>\n",
                            seqOutFormat[j].Save);
                ajFmtPrintF(outf, "      <td>%B</td>\n",
                            seqOutFormat[j].Nucleotide);
                ajFmtPrintF(outf, "      <td>%B</td>\n",
                            seqOutFormat[j].Protein);
                ajFmtPrintF(outf, "      <td>%B</td>\n",
                            seqOutFormat[j].Feature);
                ajFmtPrintF(outf, "      <td>%B</td>\n",
                            seqOutFormat[j].Gap);
                ajFmtPrintF(outf, "      <td>%B</td>\n",
                            seqOutFormat[j].Multiset);
                ajFmtPrintF(outf, "      <td>%s</td>\n",
			seqOutFormat[j].Desc);
                ajFmtPrintF(outf, "    </tr>\n");
            }
        }
    }

    ajFmtPrintF(outf, "  </tbody>\n");
    ajFmtPrintF(outf, "</table>\n");
    ajStrDel(&namestr);

    return;
}




/* @func ajSeqoutPrinthtmlFormat **********************************************
**
** Reports the internal data structures as wikitext
**
** @param [u] outf [AjPFile] Output file
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ajSeqoutPrinthtmlFormat(AjPFile outf)
{

    ajint i = 0;
    ajint j = 0;
    AjPStr namestr = NULL;

    ajFmtPrintF(outf, "<table border=3>");
    ajFmtPrintF(outf, "<tr><th>Output Format</th>\n");
    ajFmtPrintF(outf, "<th>Single</th><th>Save</th>\n");
    ajFmtPrintF(outf, "<th>Nuc</th><th>Pro</th><th>Feat</th><th>Gap</th>\n");
    ajFmtPrintF(outf, "<th>Multi</th><th>Description</th></tr>\n");

    for(i=1; seqOutFormat[i].Name; i++)
    {
        if(!seqOutFormat[i].Alias)
        {
            ajStrAssignC(&namestr, seqOutFormat[i].Name);

            for(j=i+1; seqOutFormat[j].Name; j++)
            {
                if(seqOutFormat[j].Write == seqOutFormat[i].Write)
                {
                    ajFmtPrintAppS(&namestr, "<br>%s", seqOutFormat[j].Name);
                    if(!seqOutFormat[j].Alias) 
                    {
                        ajWarn("Output format '%s' same as '%s' but not alias",
                               seqOutFormat[j].Name, seqOutFormat[i].Name);
                    }
                }
            }

	    ajFmtPrintF(outf, "<tr><td>\n%S\n</td><td>%B</td><td>%B</td>\n",
                        namestr,
			seqOutFormat[i].Single,
                        seqOutFormat[i].Save);
            ajFmtPrintF(outf, "<td>%B</td><td>%B</td><td>%B</td><td>%B</td>\n",
                        seqOutFormat[i].Nucleotide,
			seqOutFormat[i].Protein,
			seqOutFormat[i].Feature,
                        seqOutFormat[i].Gap);
            ajFmtPrintF(outf, "<td>%B</td><td>\n%s\n</td></tr>\n",
			seqOutFormat[i].Multiset,
			seqOutFormat[i].Desc);
        }
    }
    

    ajFmtPrintF(outf, "</table>\n");
    ajStrDel(&namestr);

    return;
}




/* @func ajSeqoutPrintwikiFormat **********************************************
**
** Reports the internal data structures as wikitext
**
** @param [u] outf [AjPFile] Output file
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ajSeqoutPrintwikiFormat(AjPFile outf)
{

    ajint i = 0;
    ajint j = 0;
    AjPStr namestr = NULL;

    ajFmtPrintF(outf, "{| class=\"wikitable sortable\" border=\"2\"\n");
    ajFmtPrintF(outf, "|-\n");
    ajFmtPrintF(outf, "!Format!!Sngl!!Save!!Nuc!!Pro!!Feat!!Gap!!MSet!!"
                "class=\"unsortable\"|Description\n");

    for(i=1; seqOutFormat[i].Name; i++)
    {
        if(!seqOutFormat[i].Alias)
        {
            ajStrAssignC(&namestr, seqOutFormat[i].Name);

            for(j=i+1; seqOutFormat[j].Name; j++)
            {
                if(seqOutFormat[j].Write == seqOutFormat[i].Write)
                {
                    ajFmtPrintAppS(&namestr, "<br>%s", seqOutFormat[j].Name);
                    if(!seqOutFormat[j].Alias) 
                    {
                        ajWarn("Sequence output format '%s' same as '%s' "
                               "but not alias",
                               seqOutFormat[j].Name, seqOutFormat[i].Name);
                    }
                }
            }

            ajFmtPrintF(outf, "|-\n");
	    ajFmtPrintF(outf,
			"|%S||%B||%B||%B||%B||%B||%B||%B||%s\n",
			namestr,
			seqOutFormat[i].Single,
			seqOutFormat[i].Save,
			seqOutFormat[i].Nucleotide,
			seqOutFormat[i].Protein,
			seqOutFormat[i].Feature,
			seqOutFormat[i].Gap,
			seqOutFormat[i].Multiset,
			seqOutFormat[i].Desc);
         }
    }
    

    ajFmtPrintF(outf, "|}\n\n");
    ajStrDel(&namestr);

    return;
}




/* @funcstatic seqSeqFormat ***************************************************
**
** Initialises sequence output formatting parameters.
**
** @param [r] seqlen [ajint] Sequence length
** @param [u] Psf [SeqPSeqFormat*] Sequence format object
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void seqSeqFormat(ajint seqlen, SeqPSeqFormat* Psf)
{
    char numform[20];
    SeqPSeqFormat sf;
    ajint i;
    ajint j;

    j = 1;

    for(i = seqlen; i; i /= 10)
	j++;

    sprintf(numform, "%d", seqlen);
    ajDebug("seqSeqFormat numwidth old: %d new: %d\n", strlen(numform)+1, j);

    if(!*Psf)
    {
	sf = AJNEW0(*Psf);
	sf->namewidth = 8;
	sf->spacer    = 0;
	sf->width     = 50;
	sf->tab       = 0;
	sf->numleft   = ajFalse;
	sf->numright  = sf->numleft = sf->numjust = ajFalse;
	sf->nameright = sf->nameleft = ajFalse;
	sf->numline   = 0;
	sf->linepos   = 0;

	sf->skipbefore  = ajFalse;
	sf->skipafter   = ajFalse;
	sf->isactive    = ajFalse;
	sf->baseonlynum = ajFalse;
	sf->gapchar     = '-';
	sf->matchchar   = '.';
	sf->noleaves    = sf->domatch = sf->degap = ajFalse;
	sf->pretty = ajFalse;
	strcpy(sf->endstr, "");
	/*sf->interline = 1;*/
    }
    else
	sf = *Psf;

    sf->numwidth = j;		    /* or 8 as a reasonable minimum */

    return;
}




/* ==================================================================== */
/* ============================ Casts ================================= */
/* ==================================================================== */




/* @section Sequence Output Casts *********************************************
**
** @fdata [AjPSeqout]
**
** These functions examine the contents of a sequence output object
** and return some derived information. Some of them provide access to
** the internal components of a sequence output object. They are
** provided for programming convenience but should be used with
** caution.
**
** @nam3rule Get Return an element or property
** @nam4rule Basecount Counts of nucleotide bases
** @nam4rule Checkgcg GCG checksum
** @nam4rule Filename Output filename
**
** @argrule Get seqout [const AjPSeqout] Sequence output object
** @argrule Basecount bases [ajuint*] Base counts
**
** @valrule Checkgcg [ajint] GCG checksum
** @valrule Basecount [void]
** @valrule Filename [const AjPStr] Output filename
**
** @fcategory cast
**
******************************************************************************/




/* @func ajSeqoutGetBasecount *************************************************
**
** Counts the numbers of A, C, G and T in a nucleotide sequence.
**
** @param [r] seqout [const AjPSeqout] Sequence output object
** @param [w] bases [ajuint*] Integer array, minimum size 5,
**                                 to hold the results.
** @return [void]
**
** @release 5.0.0
** @@
******************************************************************************/

void ajSeqoutGetBasecount(const AjPSeqout seqout, ajuint* bases)
{
    const char* cp;

    ajDebug("ajSeqoutGetBasecount %d bases\n", ajStrGetLen(seqout->Seq));

    bases[0] = bases[1] = bases[2] = bases[3] = bases[4] = 0;

    cp = ajStrGetPtr(seqout->Seq);

    while(*cp)
    {
	switch (*cp)
	{
            case 'A':
            case 'a':
                bases[0]++;
                break;
            case 'C':
            case 'c':
                bases[1]++;
                break;
            case 'G':
            case 'g':
                bases[2]++;
                break;
            case 'T':
            case 't':
            case 'U':
            case 'u':
                bases[3]++;
                break;
            default:
                break;
	}
	cp++;
    }

    bases[4] = ajStrGetLen(seqout->Seq) -
	bases[0] - bases[1] - bases[2] - bases[3];

    return;
}




/* @func ajSeqoutGetCheckgcg **************************************************
**
** Calculates a GCG checksum for an output sequence.
**
** @param [r] seqout [const AjPSeqout] Output sequence.
** @return [ajint] GCG checksum.
** @category cast [AjPSeqout] Calculates the GCG checksum for a
**                sequence set.
**
** @release 5.0.0
** @@
******************************************************************************/

ajint ajSeqoutGetCheckgcg(const AjPSeqout seqout)
{
    ajlong  i;
    ajlong check = 0;
    ajlong count = 0;
    const char *cp;
    ajint ilen;

    cp   = ajStrGetPtr(seqout->Seq);
    ilen = ajStrGetLen(seqout->Seq);

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




/* @func ajSeqoutGetFilename **************************************************
**
** Returns the filename for a sequence output object
**
** @param [r] seqout [const AjPSeqout] Sequence output object
** @return [const AjPStr] Filename
**
**
** @release 6.1.0
******************************************************************************/

const AjPStr ajSeqoutGetFilename(const AjPSeqout seqout)
{
    if(!seqout)
        return NULL;

    return ajFileGetPrintnameS(seqout->File);
}




/* @funcstatic seqClone *******************************************************
**
** Copies data from a sequence into a sequence output object.
** Used before writing the sequence. This version works with sequence streams.
** The difference is that the output object must be overwritten.
**
** @param [u] outseq [AjPSeqout] Sequence output.
** @param [r] seq [const AjPSeq] Sequence.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void seqClone(AjPSeqout outseq, const AjPSeq seq)
{

    ajint ibegin = 1;
    ajint iend;
    ajint ilen;
    AjPSeqRef  tmpref  = NULL;
    AjPSeqXref tmpxref = NULL;
    
    ajDebug("seqClone out Setdb '%S' Db '%S' seq Setdb '%S' Db '%S'\n",
	    outseq->Setdb, outseq->Db,
	    seq->Setdb, seq->Db);

    iend = MAJSTRGETLEN(seq->Seq);

    if(seq->Begin)
    {
	ibegin = ajSeqGetBegin(seq);
	ajDebug("seqClone begin: %d\n", ibegin);
    }

    if(seq->End)
    {
	iend = ajSeqGetEnd(seq);
	ajDebug("seqClone end: %d\n", iend);
    }

    ajDebug("seqClone outseq->Type '%S' seq->Type '%S'\n",
	    outseq->Type, seq->Type);

 
    if(MAJSTRGETLEN(seq->Setdb))
        ajStrAssignS(&outseq->Setdb, seq->Setdb);

    /* replace this with anything from -osdbname which takes precedence */
    if(MAJSTRGETLEN(outseq->Setoutdb))
      ajStrAssignS(&outseq->Setdb, outseq->Setoutdb);

    if(MAJSTRGETLEN(seq->Molecule))
        ajStrAssignS(&outseq->Molecule, seq->Molecule);
    if(MAJSTRGETLEN(seq->Class))
        ajStrAssignS(&outseq->Class, seq->Class);
    if(MAJSTRGETLEN(seq->Division))
        ajStrAssignS(&outseq->Division, seq->Division);
    if(MAJSTRGETLEN(seq->Evidence))
        ajStrAssignS(&outseq->Evidence, seq->Evidence);
    if(MAJSTRGETLEN(seq->Db))
        ajStrAssignS(&outseq->Db, seq->Db);

    if(MAJSTRGETLEN(seq->Name))
      ajStrAssignS(&outseq->Name, seq->Name);
    if(MAJSTRGETLEN(seq->Acc))
      ajStrAssignS(&outseq->Acc, seq->Acc);

    if(outseq->Acclist)
            ajListstrFreeData(&outseq->Acclist);
    if(seq->Acclist)
        outseq->Acclist = ajListstrNewList(seq->Acclist);
        
    if(MAJSTRGETLEN(seq->Sv))
      ajStrAssignS(&outseq->Sv, seq->Sv);
    if(MAJSTRGETLEN(seq->Gi))
      ajStrAssignS(&outseq->Gi, seq->Gi);

    if(MAJSTRGETLEN(seq->Tax))
        ajStrAssignS(&outseq->Tax, seq->Tax);
    if(MAJSTRGETLEN(seq->Taxid))
        ajStrAssignS(&outseq->Taxid, seq->Taxid);
    if(MAJSTRGETLEN(seq->Organelle))
        ajStrAssignS(&outseq->Organelle, seq->Organelle);

    if(outseq->Taxlist)
        ajListstrFreeData(&outseq->Taxlist);
    if(seq->Taxlist)
        outseq->Taxlist = ajListstrNewList(seq->Taxlist);

    if(outseq->Keylist)
        ajListstrFreeData(&outseq->Keylist);
    if(seq->Keylist)
        outseq->Keylist = ajListstrNewList(seq->Keylist);

    if(outseq->Cmtlist)
        ajListstrFreeData(&outseq->Cmtlist);
    if(seq->Cmtlist)
        outseq->Cmtlist = ajListstrNewList(seq->Cmtlist);

    if(outseq->Reflist)
    {
        while(ajListPop(outseq->Reflist,(void **)&tmpref))
            ajSeqrefDel(&tmpref);

        if(seq->Reflist)
            ajSeqreflistClone(seq->Reflist, outseq->Reflist);
    }
    else if(seq->Reflist)
    {
        outseq->Reflist = ajListNew();
        ajSeqreflistClone(seq->Reflist, outseq->Reflist);
    }
    
    if(outseq->Xreflist)
    {
        while(ajListPop(outseq->Xreflist,(void **)&tmpxref))
            ajSeqxrefDel(&tmpxref);

        if (seq->Xreflist)
            ajSeqxreflistClone(seq->Xreflist, outseq->Xreflist);
    }
    else if (seq->Xreflist)
    {
        outseq->Xreflist = ajListNew();
        ajSeqxreflistClone(seq->Xreflist, outseq->Xreflist);
    }

    if(outseq->Genelist)
    {
        if(seq->Genelist)
            ajSeqgenelistClone(seq->Genelist, outseq->Genelist);
    }
    else if(seq->Genelist)
    {
        outseq->Genelist = ajListstrNew();
        ajSeqgenelistClone(seq->Genelist, outseq->Genelist);
    }
    
    if(MAJSTRGETLEN(seq->Desc))
      ajStrAssignS(&outseq->Desc, seq->Desc);
    if(MAJSTRGETLEN(seq->Type))
      ajStrAssignS(&outseq->Type, seq->Type);
    if(MAJSTRGETLEN(seq->Formatstr))
      ajStrAssignS(&outseq->Informatstr, seq->Formatstr);
    if(MAJSTRGETLEN(seq->Entryname))
      ajStrAssignS(&outseq->Entryname, seq->Entryname);

    if(outseq->Date)
	ajSeqdateDel(&outseq->Date);

    if(seq->Date)
	outseq->Date = ajSeqdateNewDate(seq->Date);

    if(outseq->Fulldesc)
        ajSeqdescDel(&outseq->Fulldesc);
    if(seq->Fulldesc)
        outseq->Fulldesc = ajSeqdescNewDesc(seq->Fulldesc);

    if(seq->Accuracy)
    {
	ilen = MAJSTRGETLEN(seq->Seq);
	AJCRESIZE(outseq->Accuracy, ilen);
        outseq->Qualsize = ilen;
        memmove(outseq->Accuracy,seq->Accuracy+ibegin-1,ilen*sizeof(float));
    }
    else
    {
        if(outseq->Accuracy)
            AJCSET0(outseq->Accuracy,outseq->Qualsize);
    }

    outseq->Offset = ibegin - 1;

    if(iend >= ibegin)
	ajStrAssignSubS(&outseq->Seq, seq->Seq, ibegin-1, iend-1);
    else				/* empty sequence */
	ajStrAssignClear(&outseq->Seq);

    outseq->Fttable = seq->Fttable;

    if(outseq->Fttable)
	ajFeattableTrimOff(outseq->Fttable,
			    outseq->Offset, MAJSTRGETLEN(outseq->Seq));
    outseq->Rev = seq->Rev;
    outseq->Circular = seq->Circular;

    ajDebug("seqClone %d .. %d %d .. %d len: %d type: '%S'\n",
	     seq->Begin, seq->End, ibegin, iend,
	     MAJSTRGETLEN(outseq->Seq), outseq->Type);
    ajDebug("  Db: '%S' Name: '%S' Entryname: '%S'\n",
	     outseq->Db, outseq->Name, outseq->Entryname);

    ajSeqTypeCheckS(&outseq->Seq, outseq->Outputtype);

    return;
}




/* @funcstatic seqsetClone ****************************************************
**
** Clones one sequence from a set ready for output.
**
** @param [u] outseq [AjPSeqout] Sequence output.
** @param [r] seqset [const AjPSeqset] Sequence set.
** @param [r] i [ajint] Sequence number, zero for the first sequence.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void seqsetClone(AjPSeqout outseq, const AjPSeqset seqset, ajint i)
{
    /* intended to clone ith sequence in the set */
    AjPSeq seq;

    seq = seqset->Seq[i];

    seqClone(outseq, seq);

    return;
}




/* @funcstatic seqDeclone *****************************************************
**
** Clears cloned data in a sequence output object.
**
** @param [u] outseq [AjPSeqout] Sequence output.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

static void seqDeclone(AjPSeqout outseq)
{
    AjPStr ptr = NULL;
    AjPSeqRef  tmpref  = NULL;
    AjPSeqXref tmpxref = NULL;
    AjPSeqGene tmpgene = NULL;

    if(MAJSTRGETLEN(outseq->Db))
        ajStrSetClear(&outseq->Db);
    if(MAJSTRGETLEN(outseq->Setdb))
        ajStrSetClear(&outseq->Setdb);
    if(MAJSTRGETLEN(outseq->Name))
        ajStrSetClear(&outseq->Name);
    if(MAJSTRGETLEN(outseq->Acc))
        ajStrSetClear(&outseq->Acc);
    if(MAJSTRGETLEN(outseq->Sv))
        ajStrSetClear(&outseq->Sv);
    if(MAJSTRGETLEN(outseq->Gi))
        ajStrSetClear(&outseq->Gi);
    if(MAJSTRGETLEN(outseq->Tax))
        ajStrSetClear(&outseq->Tax);
    if(MAJSTRGETLEN(outseq->Taxid))
        ajStrSetClear(&outseq->Taxid);
    if(MAJSTRGETLEN(outseq->Organelle))
        ajStrSetClear(&outseq->Organelle);
    if(MAJSTRGETLEN(outseq->Desc))
        ajStrSetClear(&outseq->Desc);
    if(MAJSTRGETLEN(outseq->Type))
        ajStrSetClear(&outseq->Type);
    if(MAJSTRGETLEN(outseq->Informatstr))
        ajStrSetClear(&outseq->Informatstr);
    if(MAJSTRGETLEN(outseq->Entryname))
        ajStrSetClear(&outseq->Entryname);

    if(outseq->Date)
	ajSeqdateDel(&outseq->Date);

    if(outseq->Acclist)
        while(ajListstrPop(outseq->Acclist,&ptr))
            ajStrDel(&ptr);

    if(outseq->Keylist)
        while(ajListstrPop(outseq->Keylist,&ptr))
            ajStrDel(&ptr);

    if(outseq->Taxlist)
        while(ajListstrPop(outseq->Taxlist,&ptr))
            ajStrDel(&ptr);

    if(outseq->Cmtlist)
        while(ajListstrPop(outseq->Cmtlist,&ptr))
            ajStrDel(&ptr);

    if(outseq->Xreflist)
        while(ajListPop(outseq->Xreflist,(void **)&tmpxref))
            ajSeqxrefDel(&tmpxref);

    if(outseq->Genelist)
        while(ajListPop(outseq->Genelist,(void **)&tmpgene))
            ajSeqgeneDel(&tmpgene);

    if(outseq->Reflist)
        while(ajListPop(outseq->Reflist,(void **)&tmpref))
            ajSeqrefDel(&tmpref);

    if(MAJSTRGETLEN(outseq->Seq))
        ajStrSetClear(&outseq->Seq);

    if(outseq->Fulldesc)
        ajSeqdescClear(outseq->Fulldesc);

    AJCSET0(outseq->Accuracy, outseq->Qualsize);

    return;
}




/* @funcstatic seqFileReopen **************************************************
**
** Reopen a sequence output file. Used after the file name has been changed
** when writing a set of sequences one to each file.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [AjBool] ajTrue on success
**
** @release 1.0.0
** @@
******************************************************************************/

static AjBool seqFileReopen(AjPSeqout outseq)
{
    AjPStr name = NULL;

    if(outseq->File)
	ajFileClose(&outseq->File);

    if(outseq->Knownfile)
	outseq->Knownfile = NULL;

    ajFmtPrintS(&name, "%S.%S", outseq->Name, outseq->Extension);
    ajStrFmtLower(&name);
    outseq->File = ajFileNewOutNamePathS(name, outseq->Directory);
    ajDebug("seqFileReopen single: %B file '%S'\n", outseq->Single, name);
    ajStrDel(&name);

    if(!outseq->File)
	return ajFalse;

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of a string object.
**
** @fdata       [AjPSeqout]
**
** @nam3rule   Trace        Report string elements to debug file 
**
** @argrule Trace seqout [const AjPSeqout] String
**
** @valrule * [void]
**
** @fcategory misc
*/




/* @func ajSeqoutTrace ********************************************************
**
** Debug calls to trace the data in a sequence object.
**
** @param [r] seqout [const AjPSeqout] Sequence output object.
** @return [void]
**
** @release 1.0.0
** @@
******************************************************************************/

void ajSeqoutTrace(const AjPSeqout seqout)
{
    AjIList it;
    AjPStr cur;
    
    ajDebug("\n\n\nSequence Out trace\n");
    ajDebug( "==============\n\n");
    ajDebug( "  Name: '%S'\n", seqout->Name);

    if(ajStrGetLen(seqout->Acc))
	ajDebug( "  Accession: '%S'\n", seqout->Acc);

    if(ajListGetLength(seqout->Acclist))
    {
	ajDebug("  Acclist: (%Lu)",
		ajListGetLength(seqout->Acclist));
	it = ajListIterNewread(seqout->Acclist);

	while((cur = (AjPStr) ajListIterGet(it)))
	    ajDebug(" %S\n", cur);

	ajListIterDel(&it);
	ajDebug("\n");
    }

    if(ajStrGetLen(seqout->Sv))
	ajDebug( "  SeqVersion: '%S'\n", seqout->Sv);

    if(ajStrGetLen(seqout->Gi))
	ajDebug( "  GenInfo Id: '%S'\n", seqout->Gi);

    if(ajStrGetLen(seqout->Desc))
	ajDebug( "  Description: '%S'\n", seqout->Desc);

    if(ajStrGetRes(seqout->Seq))
	ajDebug( "  Reserved: %d\n", ajStrGetRes(seqout->Seq));

    if(ajListGetLength(seqout->Keylist))
    {
	ajDebug("  Keywordlist: (%Lu)",
		ajListGetLength(seqout->Keylist));
	it = ajListIterNewread(seqout->Keylist);

	while((cur = (AjPStr) ajListIterGet(it)))
	    ajDebug("   '%S'\n", cur);

	ajListIterDel(&it);
	ajDebug("\n");
    }

    ajDebug("  Taxonomy: '%S'\n", seqout->Tax);
    ajDebug("  Taxid: '%S'\n", seqout->Taxid);
    ajDebug("  Organelle: '%S'\n", seqout->Organelle);

    if(ajListGetLength(seqout->Taxlist))
    {
	ajDebug("  Taxlist: (%Lu)",
		ajListGetLength(seqout->Taxlist));
	it = ajListIterNewread(seqout->Taxlist);

	while((cur = (AjPStr) ajListIterGet(it)))
	    ajDebug("   '%S'\n", cur);

	ajListIterDel(&it);
    }

    if(ajStrGetLen(seqout->Type))
	ajDebug( "  Type: '%S'\n", seqout->Type);

    if(ajStrGetLen(seqout->Outputtype))
	ajDebug( "  Output type: '%S'\n", seqout->Outputtype);

    if(ajStrGetLen(seqout->Db))
	ajDebug( "  Database: '%S'\n", seqout->Db);

    if(ajStrGetLen(seqout->Full))
	ajDebug( "  Full name: '%S'\n", seqout->Full);

    if(ajStrGetLen(seqout->Usa))
	ajDebug( "  Usa: '%S'\n", seqout->Usa);

    if(ajStrGetLen(seqout->Ufo))
	ajDebug( "  Ufo: '%S'\n", seqout->Ufo);

    if(ajStrGetLen(seqout->Formatstr))
	ajDebug( "  Output format: '%S'\n", seqout->Formatstr);

    if(ajStrGetLen(seqout->Filename))
	ajDebug( "  Filename: '%S'\n", seqout->Filename);

    if(ajStrGetLen(seqout->Directory))
	ajDebug( "  Directory: '%S'\n", seqout->Directory);

    if(ajStrGetLen(seqout->Entryname))
	ajDebug( "  Entryname: '%S'\n", seqout->Entryname);

    if(ajStrGetLen(seqout->Doc))
	ajDebug( "  Documentation:...\n%S\n", seqout->Doc);

    if(seqout->Fttable)
	ajFeattableTrace(seqout->Fttable);
    else
	ajDebug( "  No Feature table present\n");

    if(seqout->Features)
	ajDebug( "  Features ON\n");
    else
	ajDebug( "  Features OFF\n");

    return;
}




/* @funcstatic seqFormatDel ***************************************************
**
** Delete a sequence format object
**
** @param [d] pformat [SeqPSeqFormat*] Sequence format
** @return [void]
**
** @release 4.0.0
******************************************************************************/

static void seqFormatDel(SeqPSeqFormat* pformat)
{
    AJFREE(*pformat);

    return;
}




/* @section exit **************************************************************
**
** Functions called on exit
**
** @fdata [AjPSeqout]
**
** @nam3rule Exit Cleanup of internals when program exits
**
** @valrule * [void]
** @fcategory misc
**
******************************************************************************/




/* @func ajSeqoutExit *********************************************************
**
** Cleans up sequence output processing internal memory
**
** @return [void]
**
** @release 5.0.0
** @@
******************************************************************************/

void ajSeqoutExit(void)
{
    ajRegFree(&seqoutRegFmt);
    ajRegFree(&seqoutRegId);

    ajStrDel(&seqoutUsaTest);

    return;
}




/* @datasection [AjPStr] Sequence output strings ******************************
**
** Function is for manipulating sequence output objects
**
** @nam2rule Seqoutstr
**
******************************************************************************/




/* @section other *******************************************************
**
** @fdata [AjPStr]
** @fcategory use
**
** @nam3rule Get Return value
** @nam3rule Is Test value
** @nam4rule Format Sequence format names
** @nam5rule FormatDefault Return name of default output format
** @nam5rule Exists Test format exists
** @nam5rule Single Test format writes to single or multiple files
**
** @argrule GetFormatDefault Pformat [AjPStr*] Default output format name
** @argrule IsFormat format [const AjPStr] Output format required
**
** @valrule FormatDefault [AjBool] True on success
** @valrule FormatExists [AjBool] True format is defined
** @valrule FormatSingle [AjBool] True if single files are written
**
******************************************************************************/




/* @func ajSeqoutstrGetFormatDefault ******************************************
**
** Sets the default output format.
** Checks the _OUTFORMAT variable,
** and uses FASTA if no other definition is found.
**
** @param [w] Pformat [AjPStr*] Default output format.
** @return [AjBool] ajTrue on success.
**
** @release 5.0.0
** @@
******************************************************************************/

AjBool ajSeqoutstrGetFormatDefault(AjPStr* Pformat)
{

    if(ajStrGetLen(*Pformat))
	ajDebug("... output format '%S'\n", *Pformat);
    else
    {
	/* ajStrAssignEmptyC(pformat, seqOutFormat[0].Name);*/
	if (ajNamGetValueC("outformat", Pformat))
	    ajDebug("ajSeqoutstrGetFormatDefault '%S' from EMBOSS_OUTFORMAT\n",
		     *Pformat);
	else
	{
	    ajStrAssignEmptyC(Pformat, "fasta"); /* use the real name */
	    ajDebug("... output format not set, default to '%S'\n", *Pformat);
	}
    }

    return ajTrue;
}




/* @funcstatic seqoutFindOutFormat ********************************************
**
** Looks for the specified output format in the internal definitions and
** returns the index.
**
** @param [r] format [const AjPStr] Format required.
** @param [w] iformat [ajint*] Index
** @return [AjBool] ajTrue on success.
**
** @release 5.0.0
** @@
******************************************************************************/

static AjBool seqoutFindOutFormat(const AjPStr format, ajint* iformat)
{

    AjPStr tmpformat = NULL;
    ajint i = 0;

    if(!ajStrGetLen(format))
    {
	if (ajNamGetValueC("outformat", &tmpformat))
	    ajDebug("seqoutFindOutFormat '%S' from EMBOSS_OUTFORMAT\n",
		    tmpformat);
	else
	    return ajFalse;

    }
    else
	ajStrAssignS(&tmpformat, format);

    ajStrFmtLower(&tmpformat);

    while(seqOutFormat[i].Name)
    {
	if(ajStrMatchCaseC(tmpformat, seqOutFormat[i].Name))
	{
	    *iformat = i;
	    ajStrDel(&tmpformat);

	    return ajTrue;
	}

	i++;
    }

    ajStrDel(&tmpformat);

    return ajFalse;
}




/* @func ajSeqoutstrIsFormatExists ********************************************
**
** Checks whether an output format name exists.
**
** @param [r] format [const AjPStr] Output format required.
** @return [AjBool] ajTrue if format is known.
**
** @release 5.0.0
** @@
******************************************************************************/

AjBool ajSeqoutstrIsFormatExists(const AjPStr format)
{
    ajint iformat;

    if(!seqoutFindOutFormat(format, &iformat))
	return ajFalse;

    return ajTrue;
}




/* @func ajSeqoutstrIsFormatSingle ********************************************
**
** Checks whether an output format should go to single files, rather than
** all sequences being written to one file. Some formats do not work when
** more than one sequence is written to a file. Obvious examples are plain
** text and GCG formats.
**
** @param [r] format [const AjPStr] Output format required.
** @return [AjBool] ajTrue if separate file is needed for each sequence.
**
** @release 5.0.0
** @@
******************************************************************************/

AjBool ajSeqoutstrIsFormatSingle(const AjPStr format)
{
    ajint iformat;

    if(!seqoutFindOutFormat(format, &iformat))
    {
	ajDebug("ajSeqoutstrIsFormatSingle: unknown output format '%S'",
                format);

	return ajFalse;
    }

    return seqOutFormat[iformat].Single;
}




#ifdef AJ_COMPILE_DEPRECATED_BOOK
#endif /* AJ_COMPILE_DEPRECATED_BOOK */




#ifdef AJ_COMPILE_DEPRECATED
/* @obsolete ajSeqoutNewF
** @rename ajSeqoutNewFile
*/
__deprecated AjPSeqout  ajSeqoutNewF(AjPFile file)
{
    return ajSeqoutNewFile(file);
}




/* @obsolete ajSeqWrite
** @rename ajSeqoutWriteSeq
*/
__deprecated void  ajSeqWrite(AjPSeqout outseq, const AjPSeq seq)
{
    ajSeqoutWriteSeq(outseq, seq);
    return;
}




/* @obsolete ajSeqWriteXyz
** @replace ajSeqoutDumpSwisslike (1,2,3/2,1,3)
*/

__deprecated void ajSeqWriteXyz(AjPFile outf, const AjPStr seq,
				const char *prefix)
{
    AjPSeqout outseq;
    outseq = ajSeqoutNewFile(outf);
    ajSeqoutDumpSwisslike(outseq, seq, prefix);
    ajSeqoutDel(&outseq);

    return;
}




/* @obsolete ajSeqoutUsa
** @rename ajSeqoutClearUsa
*/

__deprecated void ajSeqoutUsa(AjPSeqout* pthis, const AjPStr Usa)
{
    ajSeqoutClearUsa(*pthis, Usa);

    return;
}




/* @obsolete ajSeqWriteClose
** @rename ajSeqoutClose
*/
__deprecated void  ajSeqWriteClose(AjPSeqout outseq)
{
    ajSeqoutClose(outseq);
}




/* @obsolete ajSeqFileNewOut
** @rename ajSeqoutOpenFilename
*/
__deprecated AjBool  ajSeqFileNewOut(AjPSeqout seqout, const AjPStr name)
{
    return ajSeqoutOpenFilename(seqout, name);
}




/* @obsolete ajSeqsetWrite
** @rename ajSeqoutWriteSet
*/
__deprecated void  ajSeqsetWrite(AjPSeqout outseq, const AjPSeqset seq)
{
    ajSeqoutWriteSet(outseq,seq);

    return;
}




/* @obsolete ajSeqOutSetFormatC
** @rename ajSeqoutSetFormatC
*/

__deprecated AjBool ajSeqOutSetFormatC(AjPSeqout thys, const char* txt)
{
    return ajSeqoutSetFormatC(thys, txt);
}




/* @obsolete ajSeqOutSetFormat
** @rename ajSeqoutSetFormatS
*/

__deprecated AjBool ajSeqOutSetFormat(AjPSeqout seqout, const AjPStr format)
{
    return ajSeqoutSetFormatS(seqout, format);
}




/* @obsolete ajSeqoutDefName
** @replace ajSeqoutSetNameDefaultS (1,2,3/1,3,2)
*/

__deprecated void ajSeqoutDefName(AjPSeqout thys,
				  const AjPStr setname, AjBool multi)
{
    ajSeqoutSetNameDefaultS(thys, multi, setname);

    return;
}




/* @obsolete ajSeqPrintOutFormat
** @rename ajSeqoutPrintFormat
*/

__deprecated void ajSeqPrintOutFormat(AjPFile outf, AjBool full)
{
    ajSeqoutPrintFormat(outf, full);

    return;
}




/* @obsolete ajSeqoutCount
** @rename ajSeqoutGetBasecount
*/

__deprecated void ajSeqoutCount(const AjPSeqout seqout, ajuint* b)
{
    ajSeqoutGetBasecount(seqout, b);

    return;
}




/* @obsolete ajSeqoutCheckGcg
** @rename ajSeqoutGetCheckgcg
*/

__deprecated ajint ajSeqoutCheckGcg(const AjPSeqout seqout)
{
    return ajSeqoutGetCheckgcg(seqout);
}




/* @obsolete ajSeqWriteExit
** @rename ajSeqoutExit
*/

__deprecated void ajSeqWriteExit(void)
{
    ajSeqoutExit();

    return;
}




/* @obsolete ajSeqOutFormatDefault
** @rename ajSeqoutstrGetFormatDefault
*/

__deprecated AjBool ajSeqOutFormatDefault(AjPStr* pformat)
{
    return ajSeqoutstrGetFormatDefault(pformat);
}




/* @obsolete ajSeqFindOutFormat
** @rename seqoutFindOutFormat
*/

__deprecated AjBool ajSeqFindOutFormat(const AjPStr format, ajint* iformat)
{
    return seqoutFindOutFormat(format, iformat);
}




/* @obsolete ajSeqAllWrite
** @rename ajSeqoutWriteSeq
*/
__deprecated void  ajSeqAllWrite(AjPSeqout outseq, const AjPSeq seq)
{
    ajSeqoutWriteSeq(outseq, seq);

    return;
}




/* @obsolete ajSssWriteXyz
** @remove No longer called ... see ajSeqWriteSwisslike
*/

__deprecated void ajSssWriteXyz(AjPFile outf, const AjPStr seq,
				const char *prefix)
{
    AjPSeqout outseq        = NULL;
    static SeqPSeqFormat sf = NULL;

    outseq = ajSeqoutNew();

    outseq->File = outf;
    ajStrAssignS(&outseq->Seq,seq);

    ajFmtPrintF(outseq->File,
		"%-5sSEQUENCE %5d AA;\n",
		prefix, ajStrGetLen(outseq->Seq));

    seqSeqFormat(ajStrGetLen(outseq->Seq), &sf);
    strcpy(sf->endstr, "");
    sf->tab    = 4;
    sf->spacer = 11;
    sf->width  = 60;

    seqWriteSeq(outseq, sf);
    seqFormatDel(&sf);

    return;
}




/* @obsolete ajSeqOutFormatSingle
** @rename ajSeqoutstrIsFormatSingle
*/

__deprecated AjBool ajSeqOutFormatSingle(AjPStr format)
{
    return ajSeqoutstrIsFormatSingle(format);
}

#endif /* AJ_COMPILE_DEPRECATED */

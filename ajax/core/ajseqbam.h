/* The MIT License
**
**   Copyright (c) 2008 Genome Research Ltd (GRL).
**
**   Permission is hereby granted, free of charge, to any person obtaining
**   a copy of this software and associated documentation files (the
**   "Software"), to deal in the Software without restriction, including
**   without limitation the rights to use, copy, modify, merge, publish,
**   distribute, sublicense, and/or sell copies of the Software, and to
**   permit persons to whom the Software is furnished to do so, subject to
**   the following conditions:
**
**   The above copyright notice and this permission notice shall be
**   included in all copies or substantial portions of the Software.
**
**   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
**   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
**   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
**   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
**   BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
**   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
**   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
**   SOFTWARE.
*/

/* Contact: Heng Li <lh3@sanger.ac.uk> */

/*
** much modified for EMBOSS by Peter Rice pmr@ebi.ac.uk May 2010
** lists changed to AjPList
** hashes changed to AjPTable
** strings changed to AjPStr
** fixed-length datatypes int32_t etc. changed to EMBOSS types
*/

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BAM_BAM_H
#define BAM_BAM_H

/*
**  BAM library provides I/O and various operations on manipulating files
**  in the BAM (Binary Alignment/Mapping) or SAM (Sequence Alignment/Map)
**  format. It now supports importing from or exporting to TAM, sorting,
**  merging, generating pileup, and quickly retrieval of reads overlapped
**  with a specified region.
**
**  copyright Genome Research Ltd.
*/

#include "ajax.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "zlib.h"

#ifdef WIN32
#define inline __inline
#endif




/* @data AjPSeqBamBgzf ********************************************************
**
** BGZF file handling object
**
** @alias AjOSeqBamBgzf
** @alias AjSSeqBamBgzf
**
** @attr file [FILE*] File object
** @attr cache [AjPTable] Block cache
** @attr uncompressed_block [void*] Uncompressed block data
** @attr compressed_block [void*] Compressed block data
** @attr error [const char*] Error description
** @attr block_address [ajlong] Block offset
** @attr file_descriptor [int] File descriptor
** @attr cache_size [int] Cache size
** @attr uncompressed_block_size [int] Uncompressed block size
** @attr compressed_block_size [int] Compressed block size
** @attr block_length [int] Block length
** @attr block_offset [int] Block offset
** @attr open_mode [char] Open_mode 'r' or 'w'
** @attr owned_file [char] Boolean
** @attr is_uncompressed [char] Boolean
** @attr Padding [char[5]] Padding 
**
******************************************************************************/

typedef struct AjSSeqBamBgzf
{
    FILE* file;
    AjPTable cache;
    void* uncompressed_block;
    void* compressed_block;
    const char* error;
    ajlong block_address;
    int file_descriptor;
    int cache_size;
    int uncompressed_block_size;
    int compressed_block_size;
    int block_length;
    int block_offset;
    char open_mode;
    char owned_file;
    char is_uncompressed;
    char Padding[5];
} AjOSeqBamBgzf;

#define AjPSeqBamBgzf AjOSeqBamBgzf*




#define BAM_VIRTUAL_OFFSET16




/* #abstract BAM file handler */

/* @data AjPSeqBamHeader *******************************************************
**
** BAM alignment file header data
**
** @attr  target_name [char**] names of the reference sequences
** @attr  target_len  [ajuint*] lengths of the reference sequences
** @attr  dict        [AjPList] header dictionary
** @attr  hash        [AjPTable] hash table for fast name lookup
** @attr  rg2lib      [AjPTable] hash table for @RG-ID -> LB lookup
** @attr  text        [char*] plain text
** @attr  n_targets   [ajint] number of reference sequences
** @attr  l_text      [ajint] length of the plain text in the header
**
** @@
** discussion Field hash points to null by default. It is a private
**  member.
******************************************************************************/

typedef struct AjSSeqBamheader
{
    char **target_name;
    ajuint *target_len;
    AjPList dict;
    AjPTable hash;
    AjPTable rg2lib;
    char *text;
    ajint n_targets;
    ajint l_text;
} AjOSeqBamHeader;

#define AjPSeqBamHeader AjOSeqBamHeader*




/* The read is paired in sequencing, no matter whether it is mapped in a pair */
#define BAM_FPAIRED        1

/* The read is mapped in a proper pair */
#define BAM_FPROPER_PAIR   2

/* The read itself is unmapped; conflictive with BAM_FPROPER_PAIR */
#define BAM_FUNMAP         4

/* The mate is unmapped */
#define BAM_FMUNMAP        8

/* The read is mapped to the reverse strand */
#define BAM_FREVERSE      16

/* The mate is mapped to the reverse strand */
#define BAM_FMREVERSE     32

/* This is read1 */
#define BAM_FREAD1        64

/* This is read2 */
#define BAM_FREAD2       128

/* Not primary alignment */
#define BAM_FSECONDARY   256

/* QC failure */
#define BAM_FQCFAIL      512

/* Optical or PCR duplicate */
#define BAM_FDUP        1024


#define BAM_OFDEC          0
#define BAM_OFHEX          1
#define BAM_OFSTR          2


/* Defautl mask for pileup */
#define BAM_DEF_MASK (BAM_FUNMAP | BAM_FSECONDARY | BAM_FQCFAIL | BAM_FDUP)

#define BAM_CORE_SIZE   sizeof(AjOSeqBamCore)

/*
** Describing how CIGAR operation/length is packed in a 32-bit integer.
*/

#define BAM_CIGAR_SHIFT 4
#define BAM_CIGAR_MASK  ((1 << BAM_CIGAR_SHIFT) - 1)

/*
**  CIGAR operations.
*/

/* CIGAR: M match */
#define BAM_CMATCH      0

/* CIGAR: I insertion to the reference */
#define BAM_CINS        1

/* CIGAR: D deletion from the reference */
#define BAM_CDEL        2

/* CIGAR: N skip on the reference (e.g. spliced alignment) */
#define BAM_CREF_SKIP   3

/* CIGAR: S clip on the read with clipped sequence present in qseq */
#define BAM_CSOFT_CLIP  4

/* CIGAR: H clip on the read with clipped sequence trimmed off */
#define BAM_CHARD_CLIP  5

/* CIGAR: P padding */
#define BAM_CPAD        6




extern const char* cigarcode;
extern const char* bam_nt16_rev_table;




/* @data AjPSeqBamCore ********************************************************
**
** Structure for core alignment information.
**
** @attr  tid     [ajint]  read ID, defined by AjPSeqBamheader
** @attr  pos     [ajint]  0-based leftmost coordinate
** @attr  bin     [ajushort]  bin calculated by bam_reg2bin()
** @attr  qual    [unsigned char]  mapping quality
** @attr  l_qname [unsigned char]  length of the query name
** @attr  flag    [ajushort]  bitwise flag
** @attr  n_cigar [ajushort]  number of CIGAR operations
** @attr  l_qseq  [ajint]  length of the query sequence (read)
** @attr  mtid  [ajint]  paired read (mate) ID
** @attr  mpos  [ajint]  paire read (mate) position
** @attr  isize  [ajint]  insert size for paired reads
*******************************************************************************/

typedef struct AjSBamSeqCore
{
    ajint tid;
    ajint pos;
    ajushort bin;
    unsigned char qual;
    unsigned char l_qname;
    ajushort flag;
    ajushort n_cigar;
    ajint l_qseq;
    ajint mtid;
    ajint mpos;
    ajint isize;
} AjOSeqBamCore;

#define AjPSeqBamCore AjOSeqBamCore*




/* @data AjPSeqBam ************************************************************
**
** Structure for one alignment.
**
** @alias AjSSeqBam
** @alias AjOSeqBam
**
** @attr  core      [AjOSeqBamCore]  core information about the alignment
** @attr   data    [unsigned char*]   all variable-length data, concatenated;
**                             structure: cigar-qname-seq-qual-aux
** @attr  l_aux      [int]  length of auxiliary data
** @attr  data_len   [int]  current length of data
** @attr  m_data     [int]  maximum reserved size of data
** @attr  Padding    [int]  Padding to alignment boundary
**
** @@
** discussion Notes:
**
**   1. qname is zero tailed and core.l_qname includes the tailing '\0'.
**
**   2. l_qseq is calculated from the total length of an alignment block
**      on reading or from CIGAR.
******************************************************************************/

typedef struct AjSSeqBam
{
    AjOSeqBamCore core;
    unsigned char *data;
    int l_aux;
    int data_len;
    int m_data;
    int Padding;
} AjOSeqBam;

#define AjPSeqBam AjOSeqBam*




#define MAJSEQBAMSTRAND(b) (((b)->core.flag&BAM_FREVERSE) != 0)
#define MAJSEQBAMMSTRAND(b) (((b)->core.flag&BAM_FMREVERSE) != 0)




/*
**  Get the CIGAR array
**  param  b  pointer to an alignment
**  return    pointer to the CIGAR array
**
**  In the CIGAR array, each element is a 32-bit integer. The
**  lower 4 bits gives a CIGAR operation and the higher 28 bits keep the
**  length of a CIGAR.
 */
#define MAJSEQBAMCIGAR(b) ((ajuint*)((b)->data + (b)->core.l_qname))


/*
**  Get the name of the query
**  param  b  pointer to an alignment
**  return    pointer to the name string, null terminated
*/
#define MAJSEQBAMQNAME(b) ((char*)((b)->data))


/*
**  Get query sequence
**  param  b  pointer to an alignment
**  return    pointer to sequence
**
**  Each base is encoded in 4 bits: 1 for A, 2 for C, 4 for G,
**  8 for T and 15 for N. Two bases are packed in one byte with the base
**  at the higher 4 bits having smaller coordinate on the read. It is
**  recommended to use bam1_seqi() macro to get the base.
*/
#define MAJSEQBAMSEQ(b) ((b)->data + (b)->core.n_cigar*4 + (b)->core.l_qname)


/*
**  Get query quality
**  param  b  pointer to an alignment
**  return    pointer to quality string
*/
#define MAJSEQBAMQUAL(b) ((b)->data + (b)->core.n_cigar*4 + \
                          (b)->core.l_qname + ((b)->core.l_qseq + 1)/2)

/*
**  Get a base on read
**  param  s  Query sequence returned by bam1_seq()
**  param  i  The i-th position, 0-based
**  return    4-bit integer representing the base.
*/
#define MAJSEQBAMSEQI(s, i) ((s)[(i)/2] >> 4*(1-(i)%2) & 0xf)

/*
**  Get query sequence and quality
**  param  b  pointer to an alignment
**  return    pointer to the concatenated auxiliary data
*/
#define bam1_aux(b) ((b)->data + (b)->core.n_cigar*4 + (b)->core.l_qname + \
                     (b)->core.l_qseq + ((b)->core.l_qseq + 1)/2)

#ifndef kroundup32
/*
**  Round an integer to the next closest power-2 integer.
**  param  x  integer to be rounded (in place)
**  x will be modified.
*/
#define kroundup32(x) (--(x), (x)|=(x)>>1, (x)|=(x)>>2, (x)|=(x)>>4, \
                       (x)|=(x)>>8, (x)|=(x)>>16, ++(x))
#endif




/*
** Prototype definitions
*/

AjPSeqBamBgzf ajSeqBamBgzfOpenfd(int fd, const char *mode);
AjPSeqBamBgzf ajSeqBamBgzfOpenC(const char* path, const char *mode);

int ajSeqBamBgzfClose(AjPSeqBamBgzf fp);
int ajSeqBamBgzfEof(AjPSeqBamBgzf fp);
int ajSeqBamBgzfRead(AjPSeqBamBgzf fp, void* data, int length);
int ajSeqBamBgzfWrite(AjPSeqBamBgzf fp, const void* data, int length);

AjPSeqBamHeader ajSeqBamHeaderNew(void);
AjPSeqBamHeader ajSeqBamHeaderNewTextC(const char* txt);

void ajSeqBamHeaderDel(AjPSeqBamHeader *Pheader);

int ajSeqBamHeaderWrite(AjPSeqBamBgzf fp, const AjPSeqBamHeader header);

int ajSeqBamRead(AjPSeqBamBgzf fp, AjPSeqBam b);

int ajSeqBamWrite(AjPSeqBamBgzf fp, const AjPSeqBam b);

const char *ajSeqBamGetLibrary(AjPSeqBamHeader header, const AjPSeqBam b);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif

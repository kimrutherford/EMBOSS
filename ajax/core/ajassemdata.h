/* @include ajassemdata *******************************************************
**
** AJAX assembly datatypes
**
** These functions control all aspects of AJAX assembly
** parsing and include simple utilities.
**
** @author Copyright (C) 2010 Peter Rice
** @version $Revision: 1.29 $
** @modified Oct 5 pmr First version
** @modified $Date: 2012/07/02 16:44:55 $ by $Author: rice $
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

#ifndef AJASSEMDATA_H
#define AJASSEMDATA_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajstr.h"
#include "ajtable.h"
#include "ajtime.h"
#include "ajtextdata.h"
#include "ajseqbam.h"
#include "ajbamindex.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* @enum AjEAssemSortOrder ****************************************************
**
** Assembly reads sort order Type enumeration.
**
** @value ajEAssemSortOrderUnknown Unknown - default in SAM/BAM formats
** @value ajEAssemSortOrderUnsorted Unsorted - unsorted
** @value ajEAssemSortOrderQueryname Queryname - sorted by query/read names
** @value ajEAssemSortOrderCoordinate Coordinate - major sort key is the
**        the reference/contig name, order defined by the order of @SQ lines
**        in header in SAM/BAM assemblies.
** @@
******************************************************************************/

typedef enum AjOAssemSortOrder
{
    ajEAssemSortOrderUnknown,
    ajEAssemSortOrderUnsorted,
    ajEAssemSortOrderQueryname,
    ajEAssemSortOrderCoordinate
} AjEAssemSortOrder;




/* @enum AjEAssemPlatform *****************************************************
**
** Platforms/technologies to produce reads, as enumerated in SAM specv1.4.
**
** @value ajEAssemPlatformUnknown Unknown
** @value ajEAssemPlatformCapillary Capillary
** @value ajEAssemPlatformLS454 LS 454
** @value ajEAssemPlatformIllumina Illumina
** @value ajEAssemPlatformSolid Solid
** @value ajEAssemPlatformHelicos Helicos
** @value ajEAssemPlatformIontorrent IonTorrent
** @value ajEAssemPlatformPacbio Pacific Biosciences
** @@
******************************************************************************/

typedef enum AjOAssemPlatform
{
    ajEAssemPlatformUnknown,
    ajEAssemPlatformCapillary,
    ajEAssemPlatformLS454,
    ajEAssemPlatformIllumina,
    ajEAssemPlatformSolid,
    ajEAssemPlatformHelicos,
    ajEAssemPlatformIontorrent,
    ajEAssemPlatformPacbio
} AjEAssemPlatform;




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* @data AjPAssemRead *********************************************************
**
** Store individual alignments of reads in assemblies.
**
** In some assemblies some reads can align to more than one contig
** or reference sequence, so current name of the data type is not perfect.
**
** For storing read information as part of an assembly object.
** Current version of this data type is MIRA/SAM oriented,
** it needs to be improved to be a proper common data type,
** some of the current fields are used by MIRA MAF format only
** while few others are used by the SAM format only.
**
** @alias AjSAssemRead
** @alias AjOAssemRead
**
**
** In it's simplest form, a DNA template is sequenced only once.
** In paired-end sequencing, a DNA template is sequenced once in forward
** and once in reverse direction (Sanger, 454, Solexa).
** In Sanger sequencing, several forward and/or reverse reads
** can be sequenced from a DNA template.
** In PacBio sequencing, a DNA template can be sequenced
** in several "strobes", leading to multiple reads on a DNA template.
** (ref:MAF format specification on mira web site)
**
**
** @attr Name       [AjPStr]  Name
** @attr Seq        [AjPStr]  Sequence
** @attr SeqQ       [AjPStr]  Sequence quality string
** @attr Template   [AjPStr]  Name of the DNA template a sequence comes from
** @attr File       [AjPStr]  Name of the sequencing file
**                            which contains raw data for this read
** @attr Technology [AjPStr]  Sequencing technology
** @attr Cigar      [AjPStr]  CIGAR string
** @attr Tags       [AjPList] List of AjPAssemTag objects
** @attr AlignmentBlocks [AjPList] Alignment blocks
** @attr Rnext      [ajlong]  Reference number of the mate/next fragment
** @attr Reference  [ajlong]  Reference sequence
** @attr Pnext      [ajlong]  Position of the mate/next fragment
** @attr Tlen       [ajint]   Observed template length
** @attr Flag       [ajint]   Flag
** @attr MapQ       [ajint]   Map quality
** @attr TemplateSizeMin [ajint] Minimum template size
** @attr TemplateSizeMax [ajint] Maximum template size
** @attr ClipLeft     [ajint] Clip left
** @attr ClipRight    [ajint] Clip right
** @attr VectorLeft   [ajint] Clip left due to sequencing vector
** @attr VectorRight  [ajint] Clip right due to sequencing vector
** @attr QualLeft     [ajint] Clip left due to quality
** @attr QualRight    [ajint] Clip right due to quality
** @attr x1      [ajint] interval of the contig (1-based as in SAM and MAF)
** @attr y1      [ajint] end of contig interval
** @attr x2      [ajint] interval of the read
** @attr y2      [ajint] end of read interval
** @attr Reversed  [AjBool] true: has been reverse-complemented
** @attr Direction [char] Direction of the read with respect to the template
** @attr Padding [char[7]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSAssemRead
{
    AjPStr  Name;
    AjPStr  Seq;
    AjPStr  SeqQ;
    AjPStr  Template;
    AjPStr  File;
    AjPStr  Technology;
    AjPStr  Cigar;
    AjPList Tags;
    AjPList AlignmentBlocks;
    ajlong  Rnext;
    ajlong  Reference;
    ajlong  Pnext;
    ajint  Tlen;
    ajint  Flag;
    ajint  MapQ;
    ajint  TemplateSizeMin;
    ajint  TemplateSizeMax;
    ajint  ClipLeft;
    ajint  ClipRight;
    ajint  VectorLeft;
    ajint  VectorRight;
    ajint  QualLeft;
    ajint  QualRight;
    ajint  x1;
    ajint  y1;
    ajint  x2;
    ajint  y2;
    AjBool Reversed;
    char   Direction;
    char   Padding[7];
} AjOAssemRead;

#define AjPAssemRead AjOAssemRead*




/* @data AjPAssemContig *******************************************************
**
** Ajax AssemContig object to store contigs in assemblies.
**
** In mapping assemblies consensus sequence refers to the reference sequence.
**
** @attr Name       [AjPStr]  Name
** @attr Consensus  [AjPStr]  Consensus/reference sequence
** @attr ConsensusQ [AjPStr]  Quality string for the consensus sequence
** @attr Tags       [AjPList] Tags for the consensus sequence
** @attr Length     [ajint]   Length of the consensus sequence
** @attr Nreads     [ajint]   Number of reads
** @attr AssemblyID [AjPStr]  Assembly ID
** @attr MD5        [AjPStr]  MD5 checksum of the consensus sequence
**                            in the uppercase, with gaps and spaces removed
** @attr Species    [AjPStr]  Species
** @attr URI        [AjPStr]  URI of the consensus sequences
**
** @@
******************************************************************************/

typedef struct AjSContig
{
    AjPStr  Name;
    AjPStr  Consensus;
    AjPStr  ConsensusQ;
    AjPList Tags;
    ajint   Length;
    ajint   Nreads;
    AjPStr  AssemblyID;
    AjPStr  MD5;
    AjPStr  Species;
    AjPStr  URI;
} AjOContig;

#define AjPAssemContig AjOContig*




/* @data AjPAssemin ***********************************************************
**
** Ajax Assembly Input object.
**
** Holds the input specification and information needed to read
** the assembly and possible further entries
**
** @alias AjSAssemin
** @alias AjOAssemin
**
** @attr Input    [AjPTextin] General text input object
** @attr BamIdx   [AjPBamIndex] BAM index
** @attr BamInput [AjBool] BAM file input
** @attr cbegin   [ajint]     Contig start position
** @attr cend     [ajint]     Contig end position
** @attr Loading  [AjBool] True if data is now loading
** @@
******************************************************************************/

typedef struct AjSAssemin
{
    AjPTextin Input;
    AjPBamIndex BamIdx;
    AjBool BamInput;
    ajint cbegin;
    ajint cend;
    AjBool Loading;
} AjOAssemin;

#define AjPAssemin AjOAssemin*




/* @data AjPAssem *************************************************************
**
** Ajax Assembly object.
**
** Holds the assembly itself, plus associated information.
**
** @alias AjSAssem
** @alias AjOAssem
**
** @attr Id           [AjPStr]   Id of term
** @attr Db           [AjPStr]   Database name from input
** @attr Setdb        [AjPStr]   Database name from command line
** @attr Full         [AjPStr]   Full name
** @attr Qry          [AjPStr]   Query for re-reading
** @attr Formatstr    [AjPStr]   Input format name
** @attr Filename     [AjPStr]   Original filename
** @attr Textptr      [AjPStr]   Full text
** @attr BamHeader    [AjPSeqBamHeader] BAM header
** @attr Contigs      [AjPTable] Contigs table, storing contigs by name
** @attr ContigsIgnored [AjPTable]  Contigs named as '*' because of
** 				    missing header '@SQ' entries
** @attr ContigsOrder [AjPList] Order of contigs in the assembly
** @attr ContigArray  [AjPAssemContig*] Contigs array for fast access
** @attr Reads        [AjPList]  List of reads in the assembly
** @attr Readgroups   [AjPTable] Table of read-groups in the assembly
** @attr rec          [AjPAssemRead] Current/last read/alignment record read
** @attr Fpos         [ajlong]   File position
** @attr Format       [AjEnum]   Input format enum
** @attr Count        [ajuint]   Number of lines read (contigs in case of BAM)
** @attr Hasdata      [AjBool]  True when data has been loaded
** @attr SO           [AjEAssemSortOrder] Reads sort order in the assembly
**
** @@
******************************************************************************/

typedef struct AjSAssem
{
    AjPStr  Id;
    AjPStr  Db;
    AjPStr  Setdb;
    AjPStr  Full;
    AjPStr  Qry;
    AjPStr  Formatstr;
    AjPStr  Filename;
    AjPStr  Textptr;
    AjPSeqBamHeader BamHeader;
    AjPTable Contigs;
    AjPTable ContigsIgnored;
    AjPList ContigsOrder;
    AjPAssemContig* ContigArray;
    AjPList Reads;
    AjPTable Readgroups;
    AjPAssemRead rec;
    ajlong  Fpos;
    AjEnum  Format;
    ajuint  Count;
    AjBool Hasdata;
    AjEAssemSortOrder SO;
} AjOAssem;

#define AjPAssem AjOAssem*




/* @data AjPAssemload *********************************************************
**
** Ajax assembly loader object.
**
** Inherits an AjPAssem but allows more assembly data to be read from the
** same input by also inheriting the AjPVarin input object.
**
** @alias AjSAssemload
** @alias AjOAssemload
**
** @attr Assem [AjPAssem] Current variation
** @attr Assemin [AjPAssemin] Assembly input for reading next
** @attr Count [ajuint] Count of terms so far
** @attr Loading [AjBool] True if data is now loading
** @attr Returned [AjBool] if true: Assembly object has been returned to a new
**                         owner and is not to be deleted by the destructor
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSAssemload
{
    AjPAssem Assem;
    AjPAssemin Assemin;
    ajuint Count;
    AjBool Loading;
    AjBool Returned;
    ajuint Padding;
} AjOAssemload;

#define AjPAssemload AjOAssemload*




/* @data AjPAssemAccess *******************************************************
**
** Ajax assembly access database reading object.
**
** Holds information needed to read an assembly entry from a database.
** Access methods are defined for each known database type.
**
** Assembly entries are read from the database using the defined
** database access function, which is usually a static function
** within ajassemdb.c
**
** This should be a static data object but is needed for the definition
** of AjPAssemin.
**
** @alias AjSAssemAccess
** @alias AjOAssemAccess
**
** @attr Name [const char*] Access method name used in emboss.default
** @attr Access [AjBool function] Access function
** @attr AccessFree [AjBool function] Access cleanup function
** @attr Qlink [const char*] Supported query link operators
** @attr Desc [const char*] Description
** @attr Alias [AjBool] Alias for another name
** @attr Entry [AjBool] Supports retrieval of single entries
** @attr Query [AjBool] Supports retrieval of selected entries
** @attr All [AjBool] Supports retrieval of all entries
** @attr Chunked [AjBool] Supports retrieval of entries in chunks
** @attr Padding [AjBool] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSAssemAccess
{
    const char *Name;
    AjBool (*Access) (AjPAssemin assemin);
    AjBool (*AccessFree) (void* qry);
    const char* Qlink;
    const char* Desc;
    AjBool Alias;
    AjBool Entry;
    AjBool Query;
    AjBool All;
    AjBool Chunked;
    AjBool Padding;
} AjOAssemAccess;

#define AjPAssemAccess AjOAssemAccess*




/* @data AjPAssemTag **********************************************************
**
** Ajax AssemTag object to store MIRA tags for the read sequences as well as
** the contig consensus sequences.
**
** SAM format also allows a set of predefined tags
** as well as it reserves lowercase tags for end users.
**
** @attr Name    [AjPStr] Name
** @attr Comment [AjPStr] Tag comment in case of MAF,
**			  or tag value in case of SAM
** @attr x1      [ajuint] X value
** @attr y1      [ajuint] Y value
** @attr type    [char]   Value type: AcCsSiIfZHB
** @attr Padding [char[7]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSAssemTag
{
    AjPStr  Name;
    AjPStr  Comment;
    ajuint  x1;
    ajuint  y1;
    char    type;
    char    Padding[7];
} AjOAssemTag;

#define AjPAssemTag AjOAssemTag*




/* @data AjPAssemReadalignmentblock *******************************************
**
** read alignment block
**
** @attr readStart      [int] Read start
** @attr referenceStart [int] Reference start
** @attr length         [int] alignment length
** @@
******************************************************************************/

typedef struct AjSAssemReadalignmentblock
{
    int readStart;
    int referenceStart;
    int length;
} AjOAssemReadalignmentblock;

#define AjPAssemReadalignmentblock AjOAssemReadalignmentblock*




/* @data AjPAssemReadgroup ****************************************************
**
** read-group object to store information about read groups as described
** in SAM spec.
**
** @attr ID        [AjPStr] Read group identifier
** @attr CN        [AjPStr] Name of the sequencing center producing the read
** @attr Desc      [AjPStr] Description
** @attr Date      [AjPStr] Date the run was produced
** @attr FlowOrder [AjPStr] The array of nucleotide bases that correspond to
**                          the nucleotides used for each flow of each record
** @attr KeySeq    [AjPStr] The array of nucleotide bases that correspond to
**                          the key sequence of each read
** @attr Library   [AjPStr] Library
** @attr Programs  [AjPStr] Programs used for processing the read group
** @attr Unit      [AjPStr] Platform unit (e.g. flowcell-barcode for Illumina)
** @attr Sample    [AjPStr] Sample name, or pool name when a pool was sequenced
** @attr Isize     [ajint]  Predicted median insert size
**
**
** @attr Platform [AjEAssemPlatform] Instrument platform
** @@
******************************************************************************/

typedef struct AjSAssemReadgroup
{
    AjPStr  ID;
    AjPStr  CN;
    AjPStr  Desc;
    AjPStr  Date;
    AjPStr  FlowOrder;
    AjPStr  KeySeq;
    AjPStr  Library;
    AjPStr  Programs;
    AjPStr  Unit;
    AjPStr  Sample;
    ajint   Isize;
    AjEAssemPlatform Platform;
} AjOAssemReadgroup;

#define AjPAssemReadgroup AjOAssemReadgroup*




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */




/*
** Prototype definitions
*/

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !AJASSEMDATA_H */

/* @include ajseqdata *********************************************************
**
** AJAX SEQ (sequence) data structures
**
** @author Copyright (C) 1998 Peter Rice
** @version $Revision: 1.82 $
** @modified Jun 25 pmr First version
** @modified $Date: 2012/12/07 10:09:13 $ by $Author: rice $
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

#ifndef AJSEQDATA_H
#define AJSEQDATA_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajtextdata.h"
#include "ajfeatdata.h"
#include "ajtime.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* @enum AjEXrefType **********************************************************
**
** Enumerated cross-reference type
**
** @value XREF_UNKNOWN type not defined
** @value XREF_DR      DR line in EMBL, SwissProt
** @value XREF_DBXREF  db_xref in EMBL/Genbank
** @value XREF_EC      EC= in SwissProt
** @value XREF_DESC    Allergen= and CD_Antigen= in SwissProt DE
** @value XREF_TAX     NCBI_TaxID
** @value XREF_RX      RX line in EMBL or SwissProt
** @value XREF_MAX     Beyond last defined value
******************************************************************************/

typedef enum AjOXrefType
{
    XREF_UNKNOWN,
    XREF_DR,
    XREF_DBXREF,
    XREF_EC,
    XREF_DESC,
    XREF_TAX,
    XREF_RX,
    XREF_MAX
} AjEXrefType;




/* @data AjPSeqDesc ***********************************************************
**
** Ajax sequence description object.
**
** Defines the gene fields needed to support various standard
** sequence database entry formats, especially UniProt
**
** @alias AjSSeqDesc
** @alias AjOSeqDesc
**
** @attr Name [AjPStr] Recommended name (only one per sequence)
** @attr Short [AjPList] String list of short names
** @attr EC [AjPList] String list of EC numbers
** @attr Multi [AjPList] String list of multiple description lines
** @attr AltNames [AjPList] List of alternate description objects
** @attr SubNames [AjPList] List of submitted name objects
** @attr Includes [AjPList] List of names for included functional domains
** @attr Contains [AjPList] List of names for post-processing components
** @attr Precursor [AjBool] True if this is a precursor
** @attr Fragments [ajuint] 1 to mark as a fragment, more to mark as fragments
**
** @@
******************************************************************************/

typedef struct AjSSeqDesc
{
    AjPStr Name;
    AjPList Short;
    AjPList EC;
    AjPList Multi;
    AjPList AltNames;
    AjPList SubNames;
    AjPList Includes;
    AjPList Contains;
    AjBool Precursor;
    ajuint Fragments;
} AjOSeqDesc;

#define AjPSeqDesc AjOSeqDesc*




/* @data AjPSeqSubdesc ********************************************************
**
** Ajax sequence sub-description object.
**
** Defines the gene fields needed to support various standard
** sequence database entry formats, especially UniProt
**
** @alias AjSSeqSubdesc
** @alias AjOSeqSubdesc
**
** @attr Name [AjPStr] Recommended name (only one per sequence)
** @attr Short [AjPList] String list of short names
** @attr EC [AjPList] String list of EC numbers
** @attr Allergen [AjPList] String list of Ig-E mediated atopic allergens
** @attr Biotech [AjPList] String list of biotechnology context names
** @attr Cdantigen [AjPList] String list of Cell Differentiation antigens
** @attr Inn [AjPList] String list of International Non-proprietary Names
**
** @@
******************************************************************************/

typedef struct AjSSeqSubdesc
{
    AjPStr Name;
    AjPList Short;
    AjPList EC;
    AjPList Allergen;
    AjPList Biotech;
    AjPList Cdantigen;
    AjPList Inn;
} AjOSeqSubdesc;

#define AjPSeqSubdesc AjOSeqSubdesc*




/* @data AjPSeqGene ***********************************************************
**
** Ajax genes object.
**
** Defines the gene fields needed to support various standard
** sequence database entry formats, especially UniProt
**
** @alias AjSSeqGene
** @alias AjOSeqGene
**
** @attr Name [AjPStr] Gene standard name
** @attr Synonyms [AjPStr] Accepted synonyms
** @attr Orf [AjPStr] Recognised open reading frame (ORF) names
**                         usually for sequencing projects in progress
** @attr Oln [AjPStr] Ordered locus name(s) representing order on chromosome
** @@
******************************************************************************/

typedef struct AjSSeqGene
{
    AjPStr Name;
    AjPStr Synonyms;
    AjPStr Orf;
    AjPStr Oln;
} AjOSeqGene;

#define AjPSeqGene AjOSeqGene*




/* @data AjPSeqDate ***********************************************************
**
** Ajax sequence dates object.
**
** Defines the date fields needed to support various standard
** sequence database entry formats
**
** @alias AjSSeqDate
** @alias AjOSeqDate
**
** @attr CreDate [AjPTime] Creation date
** @attr ModDate [AjPTime] Entry modification date
** @attr SeqDate [AjPTime] Sequence modification date
** @attr CreRel [AjPStr] Database release when first released
** @attr ModRel [AjPStr] Database release when entry last changed
** @attr SeqRel [AjPStr] Database release when sequence last changed
** @attr CreVer [AjPStr] Entry version when last changed
** @attr ModVer [AjPStr] Entry version when last changed
** @attr SeqVer [AjPStr] Entry version when sequence last changed
** @@
******************************************************************************/

typedef struct AjSSeqDate
{
    AjPTime CreDate;
    AjPTime ModDate;
    AjPTime SeqDate;
    AjPStr CreRel;
    AjPStr ModRel;
    AjPStr SeqRel;
    AjPStr CreVer;
    AjPStr ModVer;
    AjPStr SeqVer;
} AjOSeqDate;

#define AjPSeqDate AjOSeqDate*




/* @data AjPSeqRange **********************************************************
**
** Ajax sequence range
**
** Defines the queries that map to positions in a sequence (or physical map)
**
** @alias AjSSeqRange
** @alias AjOSeqRange
**
** @attr Query [AjPStr] Query
** @attr Start [ajulong] Start position
** @attr Length [ajulong] Length
** @@
******************************************************************************/

typedef struct AjSSeqRange
{
    AjPStr Query;
    ajulong Start;
    ajulong Length;
} AjOSeqRange;

#define AjPSeqRange AjOSeqRange*




/* @data AjPSeqRef ************************************************************
**
** Ajax sequence citation object.
**
** Defines the fields needed to support various standard
** entry citation formats
**
** @alias AjSSeqRef
** @alias AjOSeqRef
**
** @attr Position [AjPStr] Sequence positions
** @attr Groupname [AjPStr] Working group or consortium
** @attr Authors [AjPStr] Author list, comma-delimited
** @attr Title [AjPStr] Title
** @attr Comment [AjPStr] Comment
** @attr Xref [AjPStr] Cross reference
** @attr Location [AjPStr] Location (journal, book, submission)
** @attr Loctype [AjPStr] Location type
** @attr Number [ajuint] Reference number. This may be used in the feature
**                       table and references can disappear so the position
**                       in the list is not enough
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSSeqRef
{
    AjPStr Position;
    AjPStr Groupname;
    AjPStr Authors;
    AjPStr Title;
    AjPStr Comment;
    AjPStr Xref;
    AjPStr Location;
    AjPStr Loctype;
    ajuint Number;
    char Padding[4];
} AjOSeqRef;

#define AjPSeqRef AjOSeqRef*




/* @data AjPSeqXref ***********************************************************
**
** Ajax sequence cross-reference object.
**
** Defines the fields needed to support various standard
** entry cross-reference formats
**
** @alias AjSSeqXref
** @alias AjOSeqXref
**
** @attr Db [AjPStr] Database name
** @attr Id [AjPStr] Primary identifier
** @attr Secid [AjPStr] Secondary identifier
** @attr Terid [AjPStr] Tertiary identifier
** @attr Quatid [AjPStr] Quaternary identifier
** @attr Type [ajuint] Type of cross-reference
** @attr Start [ajuint] Start position
** @attr End [ajuint] End position
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSSeqXref
{
    AjPStr Db;
    AjPStr Id;
    AjPStr Secid;
    AjPStr Terid;
    AjPStr Quatid;
    ajuint Type;
    ajuint Start;
    ajuint End;
    char Padding[4];
} AjOSeqXref;

#define AjPSeqXref AjOSeqXref*




/* @data AjPSeq ***************************************************************
**
** Ajax Sequence object.
**
** Holds the sequence itself, plus associated information such as a
** sequence name, accession number, format, type.
**
** Also holds information on a selected sequence range and other
** options.
**
** Sequence features can also be stored, but for efficiency reasons
** features are turned off by default.
**
** @alias AjOSeq
** @alias AjSSeq
**
** @attr Name      [AjPStr] Name (ID)
** @attr Acc       [AjPStr] Accession number (primary only)
** @attr Sv        [AjPStr] SeqVersion number
** @attr Gi        [AjPStr] GI NCBI version number
** @attr Tax       [AjPStr] Main taxonomy (species)
** @attr Taxcommon [AjPStr] Main taxonomy (species) common name
** @attr Taxid     [AjPStr] Main taxonomy (species) id in NCBI taxonomy
** @attr Organelle [AjPStr] Organelle taxonomy
** @attr Type      [AjPStr] Type N or P
** @attr Molecule  [AjPStr] Molecule type
** @attr Class     [AjPStr] Class of entry
** @attr Division  [AjPStr] Database division
** @attr Evidence  [AjPStr] Experimental evidence (e.g. from UniProt)
** @attr Db        [AjPStr] Database name from input
** @attr Setdb     [AjPStr] Database name from command line
** @attr Full      [AjPStr] Full name
** @attr Date      [AjPSeqDate] Creation, modification and sequence mod dates
** @attr Desc      [AjPStr] One-line description
** @attr Fulldesc  [AjPSeqDesc] Detailed description
** @attr Doc       [AjPStr] Obsolete - see TextPtr
** @attr Usa       [AjPStr] USA for re-reading
** @attr Ufo       [AjPStr] UFO for re-reading
** @attr Formatstr [AjPStr] Input format name
** @attr Filename  [AjPStr] Original filename
** @attr Entryname [AjPStr] Entryname (ID)
** @attr TextPtr   [AjPStr] Full text
** @attr Acclist   [AjPList] Secondary accessions
** @attr Keylist   [AjPList] Keyword list
** @attr Taxlist   [AjPList] Taxonomy list (organelle, species, taxa)
** @attr Genelist  [AjPList] Gene names list
** @attr Reflist   [AjPList] Reference citation list
** @attr Cmtlist   [AjPList] Comment block list
** @attr Xreflist  [AjPList] Cross reference list
** @attr Hostlist  [AjPList] Viral host species list
** @attr Seq       [AjPStr] The sequence
** @attr Fttable   [AjPFeattable] Feature table
** @attr Accuracy  [float*] Accuracy values (one per base) from base calling
** @attr Fpos      [ajlong] File position (fseek) for USA
** @attr Rev       [AjBool] true: to be reverse-complemented
** @attr Reversed  [AjBool] true: has been reverse-complemented
** @attr Trimmed   [AjBool] true: has been trimmed
** @attr Circular  [AjBool] true: circular nucleotide molecule
** @attr Begin     [ajint] start position (processed on reading)
** @attr End       [ajint] end position (processed on reading)
** @attr Offset    [ajuint] offset from start
** @attr Offend    [ajuint] offset from end
** @attr Qualsize  [ajuint] Size of Accuracy array
** @attr Weight    [float] Weight from multiple alignment
** @attr Format    [AjEnum] Input format enum
** @attr EType     [AjEnum] unused, obsolete
**
** @@
******************************************************************************/

typedef struct AjSSeq
{
    AjPStr Name;
    AjPStr Acc;
    AjPStr Sv;
    AjPStr Gi;
    AjPStr Tax;
    AjPStr Taxcommon;
    AjPStr Taxid;
    AjPStr Organelle;
    AjPStr Type;
    AjPStr Molecule;
    AjPStr Class;
    AjPStr Division;
    AjPStr Evidence;
    AjPStr Db;
    AjPStr Setdb;
    AjPStr Full;
    AjPSeqDate Date;
    AjPStr Desc;
    AjPSeqDesc Fulldesc;
    AjPStr Doc;
    AjPStr Usa;
    AjPStr Ufo;
    AjPStr Formatstr;
    AjPStr Filename;
    AjPStr Entryname;
    AjPStr TextPtr;
    AjPList Acclist;
    AjPList Keylist;
    AjPList Taxlist;
    AjPList Genelist;
    AjPList Reflist;
    AjPList Cmtlist;
    AjPList Xreflist;
    AjPList Hostlist;
    AjPStr Seq;
    AjPFeattable Fttable;
    float* Accuracy;
    ajlong Fpos;
    AjBool Rev;
    AjBool Reversed;
    AjBool Trimmed;
    AjBool Circular;
    ajint Begin;
    ajint End;
    ajuint Offset;
    ajuint Offend;
    ajuint Qualsize;
    float Weight;
    AjEnum Format;
    AjEnum EType;
} AjOSeq;

#define AjPSeq AjOSeq*




/* @data AjPSeqset ************************************************************
**
** Ajax Sequence set object. A sequence set contains one or more
** sequences together in memory, for example as a sequence alignment.
**
** Holds the sequence set itself, plus associated information such as a
** sequence names, accession number, format, type.
**
** Also holds information on a selected sequence range and other
** options.
**
** Sequence set features can also be stored, but for efficiency reasons
** features are turned off by default.
**
** @alias AjSSeqset
** @alias AjOSeqset
** @other AjPSeq Sequences
** @other AjPSeqall Sequence streams
**
** @attr Size [ajuint] Number of sequences
** @attr Len [ajuint] Maximum sequence length
** @attr Begin [ajint] start position
** @attr End [ajint] end position
** @attr Offset [ajuint] offset from start
** @attr Offend [ajuint] offset from end
** @attr Rev [AjBool] true: reverse-complemented
** @attr Trimmed [AjBool] true: has been trimmed
** @attr Type [AjPStr] Type N or P
** @attr Totweight [float] total weight (usually 1.0 * Size)
** @attr EType [AjEnum] enum type obsolete
** @attr Formatstr [AjPStr] Input format name
** @attr Filename [AjPStr] Original filename
** @attr Full [AjPStr] Full name
** @attr Name [AjPStr] Name
** @attr Usa [AjPStr] USA for re-reading
** @attr Ufo [AjPStr] UFO for re-reading
** @attr Seq [AjPSeq*] Sequence array (see Size)
** @attr Seqweight [float*] Sequence weights (see also AjPSeq)
** @attr Format [AjEnum] Input format enum
** @attr Padding [char[4]] Padding to alignment boundary
**
** @new ajSeqsetNew Default constructor
** @delete ajSeqsetDel Default destructor
** @input ajSeqsetRead Master input routine for a sequence set
** @modify ajSeqsetToLower Converts a sequence set to lower case
** @modify ajSeqsetToUpper Converts a sequence set to upper case
** @cast ajSeqsetLen Returns the maximum length of a sequence set
** @cast ajSeqsetSize Returns the number of sequences in a sequence set
** @cast ajSeqsetAcc Returns the accession number of a sequence in a set
** @cast ajSeqsetName Returns the name of a sequence in a set
** @cast ajSeqsetSeq Returns the char* pointer to a sequence in a set
** @cast ajSeqsetIsNuc Tests whether the sequence set is nucleotide
** @cast ajSeqsetIsProt Tests whether the sequence set is protein
** @cast ajSeqsetGetFilename Returns the filename of a sequence set
** @output ajSeqsetWrite Writes out all sequences in a set
** @@
******************************************************************************/

typedef struct AjSSeqset
{
    ajuint Size;
    ajuint Len;
    ajint Begin;
    ajint End;
    ajuint Offset;
    ajuint Offend;
    AjBool Rev;
    AjBool Trimmed;
    AjPStr Type;
    float Totweight;
    AjEnum EType;
    AjPStr Formatstr;
    AjPStr Filename;
    AjPStr Full;
    AjPStr Name;
    AjPStr Usa;
    AjPStr Ufo;
    AjPSeq* Seq;
    float* Seqweight;
    AjEnum Format;
    char Padding[4];
} AjOSeqset;

#define AjPSeqset AjOSeqset*




/* @data AjPSeqin *************************************************************
**
** Ajax Sequence Input object.
**
** Holds the sequence specification and information needed to read
** the sequence and possible further sequences.
**
** Also holds information on a selected sequence range and other
** options.
**
** @alias AjSSeqin
** @alias AjOSeqin
**
** @other AjPSeq Sequences
** @other AjPSeqset Sequence sets
** @other AjPSeqall Sequence streams
**
** @attr Input     [AjPTextin] Text file input object
** @attr Name      [AjPStr]    Sequence name (replace on reading)
** @attr Acc       [AjPStr]    Sequence accession number (replace on reading)
** @attr Inputtype [AjPStr]    Sequence type from ACD
** @attr Type      [AjPStr]    Sequence type N or P
** @attr Full      [AjPStr]    Full name
** @attr Date      [AjPStr]    Date
** @attr Desc      [AjPStr]    One-line description
** @attr Doc       [AjPStr]    Full text
** @attr Inseq     [AjPStr]    Temporary input sequence holder
** @attr DbSequence   [AjPStr] Field name of sequence string
** @attr Usalist   [AjPList]   List of USA processing nodes
** @attr Begin     [ajint]     Start position
** @attr End       [ajint]     End position
** @attr Ufo       [AjPStr]    UFO for features (if any)
** @attr Fttable   [AjPFeattable] Input feature table (why in AjPSeqin?)
** @attr Ftquery   [AjPFeattabin] Feature table input spec
** @attr Entryname [AjPStr]    Entry name
** @attr Minimal   [AjBool]    true: read minimal information only
** @attr Features  [AjBool]    true: read features if any
** @attr IsNuc     [AjBool]    true: known to be nucleic
** @attr IsProt    [AjBool]    true: known to be protein
** @attr Multiset  [AjBool]    true: seqsetall input
** @attr Multidone [AjBool]    seqsetall input: true when set completed
** @attr Lower     [AjBool]    true: convert to lower case -slower
** @attr Upper     [AjBool]    true: convert to upper case -supper
** @attr Rev       [AjBool]    Reverse/complement if true
** @attr Circular  [AjBool]    Set sequences to be circular
** @attr SeqData   [void*]     Format data for reuse,
**                               e.g. multiple sequence input
** @@
******************************************************************************/

typedef struct AjSSeqin
{
    AjPTextin Input;
    AjPStr Name;
    AjPStr Acc;
    AjPStr Inputtype;
    AjPStr Type;
    AjPStr Full;
    AjPStr Date;
    AjPStr Desc;
    AjPStr Doc;
    AjPStr Inseq;
    AjPStr DbSequence;
    AjPList Usalist;
    ajint Begin;
    ajint End;
    AjPStr Ufo;
    AjPFeattable Fttable;
    AjPFeattabin Ftquery;
    AjPStr Entryname;
    AjBool Minimal;
    AjBool Features;
    AjBool IsNuc;
    AjBool IsProt;
    AjBool Multiset;
    AjBool Multidone;
    AjBool Lower;
    AjBool Upper;
    AjBool Rev;
    AjBool Circular;
    void *SeqData;
} AjOSeqin;

#define AjPSeqin AjOSeqin*




/* @data AjPSeqall ************************************************************
**
** Ajax Sequence all (stream) object.
**
** Inherits an AjPSeq but allows more sequences to be read from the
** same input by also inheriting the AjPSeqin input object.
**
** @alias AjSSeqall
** @alias AjOSeqall
**
** @new ajSeqallNew Default constructor
** @delete ajSeqallDel Default destructor
** @modify ajSeqallNext Master sequence stream input, reads next sequence
**                   from an open input stream.
** @cast ajSeqallGetFilename Returns the filename of a seqall object.
** @other AjPSeq Sequences
** @other AjPSeqin Sequence input
** @other AjPSeqset Sequence sets
**
** @attr Seq [AjPSeq] Current sequence
** @attr Seqin [AjPSeqin] Sequence input for reading next
** @attr Totseqs [ajlong] Count of sequences so far
** @attr Totlength [ajlong] Count of sequence lengths so far
** @attr Count [ajint] Count of sequences so far
** @attr Begin [ajint] start position
** @attr End [ajint] end position
** @attr Rev [AjBool] if true: reverse-complement
** @attr Returned [AjBool] if true: Seq object has been returned to a new owner
**                         and is not to be deleted by the destructor
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSSeqall
{
    AjPSeq Seq;
    AjPSeqin Seqin;
    ajlong Totseqs;
    ajlong Totlength;
    ajint Count;
    ajint Begin;
    ajint End;
    AjBool Rev;
    AjBool Returned;
    char Padding[4];
} AjOSeqall;

#define AjPSeqall AjOSeqall*




/* @data AjPSeqAccess *********************************************************
**
** Ajax sequence access database reading object.
**
** Holds information needed to read a sequence from a database.
** Access methods are defined for each known database type.
**
** Sequences are read from the database using the defined
** database access function, which is usually a static function
** within ajtextdb.c ajseqdb.c
**
** This should be a static data object but is needed for the definition
** of AjPSeqin.
**
** @alias AjSSeqAccess
** @alias AjOSeqAccess
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

typedef struct AjSSeqAccess
{
    const char *Name;
    AjBool (*Access)(AjPSeqin seqin);
    AjBool (*AccessFree)(void* qry);
    const char* Qlink;
    const char* Desc;
    AjBool Alias;
    AjBool Entry;
    AjBool Query;
    AjBool All;
    AjBool Chunked;
    AjBool Padding;
} AjOSeqAccess;

#define AjPSeqAccess AjOSeqAccess*




/* @data AjPSeqout ************************************************************
**
** Ajax Sequence Output object.
**
** Holds definition of sequence output.
**
** @alias AjSSeqout
** @alias AjOSeqout
**
** @attr Name [AjPStr] Name (ID)
** @attr Acc [AjPStr] Accession number (primary only)
** @attr Sv [AjPStr] SeqVersion number
** @attr Gi [AjPStr] GI NCBI version number
** @attr Desc [AjPStr] One-line description
** @attr Tax [AjPStr] Main taxonomy (species)
** @attr Taxcommon [AjPStr] Main taxonomy (species) common name
** @attr Taxid [AjPStr] Main taxonomy (species) id in NCBI taxonomy
** @attr Organelle [AjPStr] Organelle taxonomy
** @attr Type [AjPStr] Type N or P
** @attr Outputtype [AjPStr] Output sequence known type
** @attr Molecule [AjPStr] Molecule type
** @attr Class [AjPStr] Class of entry
** @attr Division [AjPStr] Database division
** @attr Evidence [AjPStr] Experimental evidence (e.g. from UniProt)
** @attr Db [AjPStr] Database name from input name
** @attr Setdb [AjPStr] Database name from input command line
** @attr Setoutdb [AjPStr] Database name from command line
** @attr Full [AjPStr] Full name
** @attr Date [AjPSeqDate] Dates
** @attr Fulldesc [AjPSeqDesc] Dates
** @attr Doc [AjPStr] Obsolete - see TextPtr
** @attr Usa [AjPStr] USA for re-reading
** @attr Ufo [AjPStr] UFO for re-reading
** @attr Fttable [AjPFeattable] Feature table
** @attr Ftquery [AjPFeattabOut] Feature table output
** @attr FtFormat [AjPStr] Feature output format (if not in UFO)
** @attr FtFilename [AjPStr] Feature output filename (if not in UFO)
** @attr Informatstr [AjPStr] Input format
** @attr Formatstr [AjPStr] Output format
** @attr EType [AjEnum] unused, obsolete
** @attr Format [AjEnum] Output format index
** @attr Filename [AjPStr] Output filename (if not in USA)
** @attr Directory [AjPStr] Output directory
** @attr Entryname [AjPStr] Entry name
** @attr Acclist [AjPList] Secondary accessions
** @attr Keylist [AjPList] Keyword list
** @attr Taxlist [AjPList] Taxonomy list
** @attr Genelist [AjPList] Gene list
** @attr Reflist [AjPList] References (citations)
** @attr Cmtlist [AjPList] Comment block list
** @attr Xreflist [AjPList] Database cross reference list
** @attr Seq [AjPStr] The sequence
** @attr File [AjPFile] Output file
** @attr Knownfile [AjPFile] Already open output file (we don't close this one)
** @attr Extension [AjPStr] File extension
** @attr Savelist [AjPList] Previous sequences saved for later output
**                          (e.g. MSF format)
** @attr Accuracy [float*] Accuracy values (one per base) from base calling
** @attr Data [void*] Format data for reuse, e.g. multiple sequence output
** @attr Cleanup [void function] Function to write remaining lines on closing
** @attr Rev [AjBool] true: to be reverse-complemented
** @attr Circular [AjBool] true: circular nucleotide molecule
** @attr Single [AjBool] If true, single sequence in each file (-ossingle)
** @attr Features [AjBool] If true, save features with sequence or in file
** @attr Qualsize [ajuint] Size of Accuracy array
** @attr Count [ajint] Number of sequences
** @attr Offset [ajint] offset from start
**
** @new ajSeqoutNew Default constructor
** @delete ajSeqoutDel Default destructor
** @modify ajSeqoutUsa Resets using a new USA
** @modify ajSeqoutClear Resets ready for reuse.
** @modify ajSeqoutOpen If the file is not yet open, calls seqoutUsaProcess
** @cast ajSeqoutCheckGcg Calculates the GCG checksum for a sequence set.
** @modify ajSeqWrite Master sequence output routine
** @modify ajSeqsetWrite Master sequence set output routine
** @modify ajSeqFileNewOut Opens an output file for sequence writing.
** @other AjPSeq Sequences
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSSeqout
{
    AjPStr Name;
    AjPStr Acc;
    AjPStr Sv;
    AjPStr Gi;
    AjPStr Desc;
    AjPStr Tax;
    AjPStr Taxcommon;
    AjPStr Taxid;
    AjPStr Organelle;
    AjPStr Type;
    AjPStr Outputtype;
    AjPStr Molecule;
    AjPStr Class;
    AjPStr Division;
    AjPStr Evidence;
    AjPStr Db;
    AjPStr Setdb;
    AjPStr Setoutdb;
    AjPStr Full;
    AjPSeqDate Date;
    AjPSeqDesc Fulldesc;
    AjPStr Doc;
    AjPStr Usa;
    AjPStr Ufo;
    AjPFeattable Fttable;
    AjPFeattabOut Ftquery;
    AjPStr FtFormat;
    AjPStr FtFilename;
    AjPStr Informatstr;
    AjPStr Formatstr;
    AjEnum EType;
    AjEnum Format;
    AjPStr Filename;
    AjPStr Directory;
    AjPStr Entryname;
    AjPList Acclist;
    AjPList Keylist;
    AjPList Taxlist;
    AjPList Genelist;
    AjPList Reflist;
    AjPList Cmtlist;
    AjPList Xreflist;
    AjPStr Seq;
    AjPFile File;
    AjPFile Knownfile;
    AjPStr Extension;
    AjPList Savelist;
    float* Accuracy;
    void *Data;
    void (*Cleanup)(struct AjSSeqout * outseq);
    AjBool Rev;
    AjBool Circular;
    AjBool Single;
    AjBool Features;
    ajuint Qualsize;
    ajint Count;
    ajint Offset;
    char Padding[4];
} AjOSeqout;

#define AjPSeqout AjOSeqout*




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

#endif /* !AJSEQDATA_H */

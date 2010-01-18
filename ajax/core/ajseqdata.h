#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajseqdata_h
#define ajseqdata_h




#define NULLFPOS -1

enum AjEQryType {QRY_UNKNOWN, QRY_ENTRY, QRY_QUERY, QRY_ALL};

enum AjEXrefType {
    XREF_UNKNOWN,      /* type not defined */
    XREF_DR,           /* DR line in EMBL, SwissProt */
    XREF_DBXREF,       /* /db_xref in EMBL/Genbank */
    XREF_EC,           /* EC= in SwissProt */
    XREF_DESC,         /* Allergen= and CD_Antigen= in SwissProt DE */
    XREF_TAX,          /* NCBI_TaxID */
    XREF_RX,            /* RX line in EMBL or SwissProt */
    XREF_MAX            /* to test we are within bounds */
};

typedef struct SeqSAccess SeqSAccess;

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
** @attr AltNames [AjPList] List of alternate description objects
** @attr SubNames [AjPList] List of submitted name objects
** @attr Includes [AjPList] List of names for included functional domains
** @attr Contains [AjPList] List of names for post-processing components
** @attr Precursor [AjBool] True if this is a precursor
** @attr Fragments [ajuint] 1 to mark as a fragment, more to mark as fragments
**
** @@
******************************************************************************/

typedef struct AjSSeqDesc {
    AjPStr Name;
    AjPList Short;
    AjPList EC;
    AjPList AltNames;
    AjPList SubNames;
    AjPList Includes;
    AjPList Contains;
    AjBool Precursor;
    ajuint Fragments;
} AjOSeqDesc;
#define AjPSeqDesc AjOSeqDesc*


    
/* @data AjPSeqSubdesc *********************************************************
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

typedef struct AjSSeqSubdesc {
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
** @attr Synonyms [AjPStr] Accepted synonynms
** @attr Orf [AjPStr] Recognized open reading frame (ORF) names
**                         usually for sequencing projects in progress
** @attr Oln [AjPStr] Ordered locus name(s) representing order on chromosome
** @@
******************************************************************************/

typedef struct AjSSeqGene {
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

typedef struct AjSSeqDate {
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


    
/* @data AjPSeqRef ***********************************************************
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
**                       in the lst is not enough
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSSeqRef {
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

typedef struct AjSSeqXref {
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


/* @data AjPSeqQuery **********************************************************
**
** Ajax Sequence Query object.
**
** Holds data needed to interpret the entry specification part of a USA.
** This can refer to an entry name (or "id"), and accession number or
** other queriable items.
**
** ajpseqquery is created with the entry specification part of a USA
** (Uniform Sequence Address). The syntax is currently related to that
** used by SRS release 5.1.
**
** @alias AjSSeqQuery
** @alias AjOSeqQuery
** @other AjPSeqset Sequence sets
** @other AjPSeqall Sequence streams
**
** @attr DbName [AjPStr] Database name used by EMBOSS
** @attr DbAlias [AjPStr] Database name used by access method
** @attr DbType [AjPStr] Database type
** @attr Id [AjPStr] ID Wildcard
** @attr Acc [AjPStr] Accession Wildcard
** @attr Des [AjPStr] Description Wildcard
** @attr Key [AjPStr] Keyword Wildcard
** @attr Org [AjPStr] Taxonomy Wildcard
** @attr Sv [AjPStr] SeqVersion Wildcard
** @attr Gi [AjPStr] GenInfo Identifier Wildcard
** @attr CaseId [AjBool] True if ID match is case-sensitive
** @attr HasAcc [AjBool] True if entries have acc field
** @attr Method [AjPStr] Name of access method
** @attr Formatstr [AjPStr] Name of input sequence format
** @attr IndexDir [AjPStr] Index directory
** @attr Directory [AjPStr] Data directory
** @attr Filename [AjPStr] Individual filename
** @attr Exclude [AjPStr] File wildcards to exclude (spaced)
** @attr DbFields [AjPStr] Query fields (plus id and acc)
** @attr DbProxy [AjPStr] Proxy host
** @attr DbHttpVer [AjPStr] HTTP Version
** @attr Field [AjPStr] Query field
** @attr QryString [AjPStr] Query term
** @attr Application [AjPStr] External application command
** @attr Fpos [ajlong] File position from fseek
** @attr Type [enum AjEQryType] Enumerated query type
** @attr QryDone [AjBool] Has the query been done yet
** @attr Access [SeqSAccess*] Access function : see ajseqdb.h
** @attr QryData [void*] Private data for access function
** @attr Wild [AjBool] True if query contains '*' or '?'
** @attr Padding [char[4]] Padding to alignment boundary
**
** @new ajSeqQueryNew Default constructor
** @delete ajSeqQueryDel Default destructor
** @modify ajSeqQueryClear Clears all contents
** @use seqQueryMatch Compares an AjPSeq to a query.
** @modify ajSeqQueryStarclear Clears fully wild elements of a query because
**                          empty elements are the same.
** @use ajSeqQueryWild Tests whether a query includes wildcards
** @use ajSeqQueryIs Tests whether a query has been defined
** @@
******************************************************************************/

typedef struct AjSSeqQuery {
  AjPStr DbName;
  AjPStr DbAlias;
  AjPStr DbType;
  AjPStr Id;
  AjPStr Acc;
  AjPStr Des;
  AjPStr Key;
  AjPStr Org;
  AjPStr Sv;
  AjPStr Gi;
  AjBool CaseId;
  AjBool HasAcc;
  AjPStr Method;
  AjPStr Formatstr;
  AjPStr IndexDir;
  AjPStr Directory;
  AjPStr Filename;
  AjPStr Exclude;
  AjPStr DbFields;
  AjPStr DbProxy;
  AjPStr DbHttpVer;
  AjPStr Field;
  AjPStr QryString;
  AjPStr Application;
  ajlong Fpos;
  enum AjEQryType Type;
  AjBool QryDone;
  SeqSAccess* Access;
  void* QryData;
  AjBool Wild;
  char Padding[4];
} AjOSeqQuery;

#define AjPSeqQuery AjOSeqQuery*




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
** @new ajSeqinNew Default constructor
** @delete ajSeqinDel Default destructor
** @modify ajSeqinUsa Resets using a new USA
** @modify ajSeqinClear Resets ready for reuse.
** @modify ajSeqinSetRange Sets a sequence range for all input sequences
**
** @other AjPSeq Sequences
** @other AjPSeqset Sequence sets
** @other AjPSeqall Sequence streams
**
** @attr Name [AjPStr] Sequence name (replace on reading)
** @attr Acc [AjPStr] Sequence accession number (replace on reading)
** @attr Inputtype [AjPStr] Sequence type from ACD
** @attr Type [AjPStr] Sequence type N or P
** @attr Db [AjPStr] Database name (from commandline, replace on reading)
** @attr Full [AjPStr] Full name
** @attr Date [AjPStr] Date
** @attr Desc [AjPStr] One-line description
** @attr Doc [AjPStr] Full text
** @attr Inseq [AjPStr] Temporary input sequence holder
** @attr Begin [ajint] Start position
** @attr End [ajint] End position
** @attr List [AjPList] List of USAs to be read
** @attr Usa [AjPStr] USA for the sequence
** @attr Ufo [AjPStr] UFO for features (if any)
** @attr Fttable [AjPFeattable] Input feature table (why in AjPSeqin?)
** @attr Ftquery [AjPFeattabIn] Feature table input spec
** @attr Formatstr [AjPStr] Sequence input format name
** @attr Filename [AjPStr] Original filename
** @attr Entryname [AjPStr] Entry name
** @attr Filebuff [AjPFilebuff] Input sequence buffered file
** @attr Search [AjBool] Search for more entries (always true?)
** @attr Single [AjBool] Read single entries
** @attr CaseId [AjBool] Id case sensitive (default false)
** @attr Features [AjBool] true: read features if any
** @attr IsNuc [AjBool] true: known to be nucleic
** @attr IsProt [AjBool] true: known to be protein
** @attr multi [AjBool] ???? see also Single
** @attr multiset [AjBool] true: seqsetall input
** @attr multidone [AjBool] seqsetall input: true when set completed
** @attr Lower [AjBool] true: convert to lower case -slower
** @attr Upper [AjBool] true: convert to upper case -supper
** @attr Text [AjBool] true: save full text of entry
** @attr Count [ajint] count of entries so far. Used when ACD reads first
**                     sequence and we need to reuse it in a Next loop
** @attr Filecount [ajint] Number of files read - used by seqsetall input
** @attr Fileseqs [ajint] Number of seqs in file - used by seqsetall input
** @attr Rev [AjBool] Reverse/complement if true
** @attr Fpos [ajlong] File position (fseek) for building USA
** @attr Query [AjPSeqQuery] Query data - see AjPSeqQuery
** @attr Data [void*] Format data for reuse, e.g. multiple sequence input
** @attr Format [AjEnum] Sequence input format enum
** @attr Records [ajuint] Records processed
** @@
******************************************************************************/

typedef struct AjSSeqin {
  AjPStr Name;
  AjPStr Acc;
  AjPStr Inputtype;
  AjPStr Type;
  AjPStr Db;
  AjPStr Full;
  AjPStr Date;
  AjPStr Desc;
  AjPStr Doc;
  AjPStr Inseq;
  ajint Begin;
  ajint End;
  AjPList List;
  AjPStr Usa;
  AjPStr Ufo;
  AjPFeattable Fttable;
  AjPFeattabIn Ftquery;
  AjPStr Formatstr;
  AjPStr Filename;
  AjPStr Entryname;
  AjPFilebuff Filebuff;
  AjBool Search;
  AjBool Single;
  AjBool CaseId;
  AjBool Features;
  AjBool IsNuc;
  AjBool IsProt;
  AjBool multi;
  AjBool multiset;
  AjBool multidone;
  AjBool Lower;
  AjBool Upper;
  AjBool Text;
  ajint Count;
  ajint Filecount;
  ajint Fileseqs;
  AjBool Rev;
  ajlong Fpos;
  AjPSeqQuery Query;
  void *Data;
  AjEnum Format;
  ajuint Records;
} AjOSeqin;

#define AjPSeqin AjOSeqin*




/* @data SeqPAccess ***********************************************************
**
** Ajax Sequence Access database reading object.
**
** Holds information needed to read a sequence from a database.
** Access methods are defined for each known database type.
**
** Sequences are read from the database using the defined
** database access function, which is usually a static function
** within ajseq.c
**
** This should be a static data object but is needed for the definition
** of AjPSeqin.
**
** @alias SeqSAccess
** @new ajSeqMethod returns a copy of a known access method definition.
** @other AjPSeqin Sequence input
**
** @attr Name [const char*] Access method name used in emboss.default
** @attr Alias [AjBool] Alias for another name
** @attr Entry [AjBool] Supports retrieval of single entries
** @attr Query [AjBool] Supports retrieval of selected entries
** @attr All [AjBool] Supports retrieval of all entries
** @attr Access [(AjBool*)] Access function
** @attr AccessFree [(AjBool*)] Access cleanup function
** @attr Desc [const char*] Description
** @@
******************************************************************************/

typedef struct SeqSAccess {
  const char *Name;
  AjBool Alias;
  AjBool Entry;
  AjBool Query;
  AjBool All;
  AjBool (*Access) (AjPSeqin seqin);
  AjBool (*AccessFree) (void* qry);
  const char* Desc;
} SeqOAccess;

#define SeqPAccess SeqOAccess*



    

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
** @attr Name [AjPStr] Name (ID)
** @attr Acc [AjPStr] Accession number (primary only)
** @attr Sv [AjPStr] SeqVersion number
** @attr Gi [AjPStr] GI NCBI version number
** @attr Tax [AjPStr] Main taxonomy (species)
** @attr Taxid [AjPStr] Main taxonomy (species) id in NCBI taxonomy
** @attr Organelle [AjPStr] Organelle taxonomy
** @attr Type [AjPStr] Type N or P
** @attr Molecule [AjPStr] Molecule type
** @attr Class [AjPStr] Class of entry
** @attr Division [AjPStr] Database division
** @attr Evidence [AjPStr] Experimental evidence (e.g. from UniProt)
** @attr Db [AjPStr] Database name from input
** @attr Setdb [AjPStr] Database name from command line
** @attr Full [AjPStr] Full name
** @attr Date [AjPSeqDate] Creation, modification and sequence mod dates
** @attr Desc [AjPStr] One-line description
** @attr Fulldesc [AjPSeqDesc] Detailed description
** @attr Doc [AjPStr] Obsolete - see TextPtr
** @attr Usa [AjPStr] USA for re-reading
** @attr Ufo [AjPStr] UFO for re-reading
** @attr Formatstr [AjPStr] Input format name
** @attr Filename [AjPStr] Original filename
** @attr Entryname [AjPStr] Entryname (ID)
** @attr TextPtr [AjPStr] Full text
** @attr Acclist [AjPList] Secondary accessions
** @attr Keylist [AjPList] Keyword list
** @attr Taxlist [AjPList] Taxonomy list (organelle, species, taxa)
** @attr Genelist [AjPList] Gene names list
** @attr Reflist [AjPList] Reference citation list
** @attr Cmtlist [AjPList] Comment block list
** @attr Xreflist [AjPList] Cross reference list
** @attr Seq [AjPStr] The sequence
** @attr Fttable [AjPFeattable] Feature table
** @attr Accuracy [float*] Accuracy values (one per base) from base calling
** @attr Fpos [ajlong] File position (fseek) for USA
** @attr Rev [AjBool] true: to be reverse-complemented
** @attr Reversed [AjBool] true: has been reverse-complemented
** @attr Trimmed [AjBool] true: has been trimmed
** @attr Circular [AjBool] true: circular nucleotide molecule
** @attr Begin [ajint] start position (processed on reading)
** @attr End [ajint] end position (processed on reading)
** @attr Offset [ajuint] offset from start
** @attr Offend [ajuint] offset from end 
** @attr Qualsize [ajuint] Size of Accuracy array
** @attr Weight [float] Weight from multiple alignment
** @attr Format [AjEnum] Input format enum
** @attr EType [AjEnum] unused, obsolete
**
** @@
******************************************************************************/

typedef struct AjSSeq {
  AjPStr Name;
  AjPStr Acc;
  AjPStr Sv;
  AjPStr Gi;
  AjPStr Tax;
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

typedef struct AjSSeqset {
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

typedef struct AjSSeqall {
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

#endif

#ifdef __cplusplus
}
#endif

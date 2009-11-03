#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajnexus_h
#define ajnexus_h

#include "ajfile.h"
#include "ajstr.h"

/* @data AjPNexusTaxa *****************************************************
**
** Ajax nexus data taxa block object.
**
** @alias AjSNexusTaxa
** @alias AjONexusTaxa
**
** @new nexusTaxaNew Default constructor
**
** @delete nexusTaxaDel Default destructor
**
** @attr TaxLabels [AjPStr*] Taxon names
** @attr Ntax [ajuint] Number of taxons
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSNexusTaxa {
    AjPStr* TaxLabels;
    ajuint  Ntax;
    char    Padding[4];
} AjONexusTaxa;
#define AjPNexusTaxa AjONexusTaxa*



/* @data AjPNexusCharacters ***************************************************
**
** Ajax nexus data characters block object.
**
** @alias AjSNexusCharacters
** @alias AjONexusCharacters
**
** @new nexusCharactersNew Default constructor
**
** @delete nexusCharactersDel Default destructor
**
** @attr NewTaxa [AjBool] New taxa read from data block
** @attr Ntax [ajuint] Number of taxons
** @attr Nchar [ajuint] Number of characters
** @attr RespectCase [AjBool] Respect case if true
** @attr DataType [AjPStr] Data type
** @attr Symbols [AjPStr] Character symbols
** @attr Equate [AjPStr] Character equivalent names
** @attr Labels [AjBool] Labels if true
** @attr Transpose [AjBool] Transpose data if true
** @attr Interleave [AjBool] Interleaved input if true
** @attr Tokens [AjBool] If true, tokens set
** @attr Items [AjPStr] Character items
** @attr StatesFormat [AjPStr] Statesformat string
** @attr Eliminate [AjPStr] Elimioate string
** @attr CharStateLabels [AjPStr*] Character and state labels
** @attr CharLabels [AjPStr*] Character labels
** @attr StateLabels [AjPStr*] State labels
** @attr Matrix [AjPStr*] Matrix data
** @attr Sequences [AjPStr*] Sequence data
** @attr Missing [char] Missing character in input data
** @attr Gap [char] Gap character in input data
** @attr MatchChar [char] Matching charater in input
** @attr Padding [char[5]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSNexusCharacters {
    AjBool NewTaxa;
    ajuint Ntax;
    ajuint Nchar;
    AjBool RespectCase;
    AjPStr DataType;

    AjPStr Symbols;
    AjPStr Equate;
    AjBool Labels;
    AjBool Transpose;
    AjBool Interleave;
    AjBool Tokens;
    AjPStr Items;
    AjPStr StatesFormat;
    AjPStr Eliminate;
    AjPStr* CharStateLabels;
    AjPStr* CharLabels;
    AjPStr* StateLabels;
    AjPStr* Matrix;
    AjPStr* Sequences;
    char Missing;
    char Gap;
    char MatchChar;
    char Padding[5];
} AjONexusCharacters;
#define AjPNexusCharacters AjONexusCharacters*



/* @data AjPNexusUnaligned ****************************************************
**
** Ajax nexus data unaligned block object.
**
** Very similar to a character block and will be merged with AjPNexusCharacter
** in the near future
**
** @alias AjSNexusUnaligned
** @alias AjONexusUnaligned
**
** @new nexusUnalignedNew Default constructor
**
** @delete nexusUnalignedDel Default destructor
**
** @attr NewTaxa [AjBool] New taxa read from data block
** @attr Ntax [ajuint] Number of taxons
** @attr DataType [AjPStr] Data type
** @attr RespectCase [AjBool] Respect case if true
** @attr Labels [AjBool] Labels if true
** @attr Symbols [AjPStr] Character symbols
** @attr Equate [AjPStr] Character equivalent names
** @attr Matrix [AjPStr*] Matrix data
** @attr Missing [char] Missing character in input data
** @attr Padding [char[7]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSNexusUnaligned {
    AjBool NewTaxa;
    ajuint Ntax;
    AjPStr DataType;
    AjBool RespectCase;
    AjBool Labels;
    AjPStr Symbols;
    AjPStr Equate;
    AjPStr* Matrix;
    char Missing;
    char Padding[7];
} AjONexusUnaligned;
#define AjPNexusUnaligned AjONexusUnaligned*



/* @data AjPNexusDistances ****************************************************
**
** Ajax nexus data distances block object.
**
** @alias AjSNexusDistances
** @alias AjONexusDistances
**
** @new nexusDistancesNew Default constructor
**
** @delete nexusDistancesDel Default destructor
**
** @attr NewTaxa [AjBool] New taxa read from data block
** @attr Ntax [ajuint] Number of taxons
** @attr Nchar [ajuint] Number of characters
** @attr Diagonal [AjBool] If true, expect to read diagonal of matrix
** @attr Labels [AjBool] Labels if true
** @attr Interleave [AjBool] Interleaved input if true
** @attr Triangle [AjPStr] Triangular distances block type
** @attr Matrix [AjPStr*] Matrix data
** @attr Missing [char] Missing character in input data
** @attr Padding [char[7]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSNexusDistances {
    AjBool NewTaxa;
    ajuint Ntax;
    ajuint Nchar;
    AjBool Diagonal;
    AjBool Labels;
    AjBool Interleave;
    AjPStr Triangle;
    AjPStr* Matrix;
    char Missing;
    char Padding[7];
} AjONexusDistances;
#define AjPNexusDistances AjONexusDistances*



/* @data AjPNexusSets *********************************************************
**
** Ajax nexus data sets block object.
**
** The data is generally the NEXUS command strings as in the original file.
**
** @alias AjSNexusSets
** @alias AjONexusSets
**
** @new nexusSetsNew Default constructor
**
** @delete nexusSetsDel Default destructor
**
** @attr CharSet [AjPStr*] Character set commands
** @attr StateSet [AjPStr*] State set commands
** @attr ChangeSet [AjPStr*] Change set commands
** @attr TaxSet [AjPStr*] Taxon set commands
** @attr TreeSet [AjPStr*] Tree set commands
** @attr CharPartition [AjPStr*] Character partition statements
** @attr TaxPartition [AjPStr*] Taxa partition statements
** @attr TreePartition [AjPStr*] Tree partition statements
** @@
******************************************************************************/

typedef struct AjSNexusSets {
    AjPStr* CharSet;
    AjPStr* StateSet;
    AjPStr* ChangeSet;
    AjPStr* TaxSet;
    AjPStr* TreeSet;
    AjPStr* CharPartition;
    AjPStr* TaxPartition;
    AjPStr* TreePartition;
} AjONexusSets;
#define AjPNexusSets AjONexusSets*



/* @data AjPNexusAssumptions **************************************************
**
** Ajax nexus data sssumptions block object.
**
** @alias AjSNexusAssumptions
** @alias AjONexusAssumptions
**
** @new nexusAssumptionsNew Default constructor
**
** @delete nexusAssumptionsDel Default destructor
**
** @attr DefType [AjPStr] Options deftype subcommand
** @attr PolyTCount [AjPStr] Options polytcount subcommand
** @attr GapMode [AjPStr] Options gapmode subcommand
** @attr UserType [AjPStr*] Usertype commands
** @attr TypeSet [AjPStr*] Typeset commands
** @attr WtSet [AjPStr*] Wtset commands
** @attr ExSet [AjPStr*] Exset commands
** @attr AncStates [AjPStr*] Ancstates commands
** @@
******************************************************************************/

typedef struct AjSNexusAssumptions {
    AjPStr DefType;
    AjPStr PolyTCount;
    AjPStr GapMode;
    AjPStr* UserType;
    AjPStr* TypeSet;
    AjPStr* WtSet;
    AjPStr* ExSet;
    AjPStr* AncStates;
} AjONexusAssumptions;
#define AjPNexusAssumptions AjONexusAssumptions*



/* @data AjPNexusCodons *******************************************************
**
** Ajax nexus data codons block object.
**
** @alias AjSNexusCodons
** @alias AjONexusCodons
**
** @new nexusCodonsNew Default constructor
**
** @delete nexusCodonsDel Default destructor
**
** @attr CodonPosSet [AjPStr*] CodonPosSetcommands
** @attr GeneticCode [AjPStr*] GeneticCodecommands
** @attr CodeSet [AjPStr*] CodeSetcommands
** @@
******************************************************************************/

typedef struct AjSNexusCodons {
    AjPStr* CodonPosSet;
    AjPStr* GeneticCode;
    AjPStr* CodeSet;
} AjONexusCodons;
#define AjPNexusCodons AjONexusCodons*



/* @data AjPNexusTrees ********************************************************
**
** Ajax nexus data trees block object.
**
** @alias AjSNexusTrees
** @alias AjONexusTrees
**
** @new nexusTreesNew Default constructor
**
** @delete nexusTreesDel Default destructor
**
** @attr Translate [AjPStr*] Translate commands
** @attr Tree [AjPStr*] Tree commands
** @@
******************************************************************************/

typedef struct AjSNexusTrees {
    AjPStr* Translate;
    AjPStr* Tree;
} AjONexusTrees;
#define AjPNexusTrees AjONexusTrees*



/* @data AjPNexusNotes *****************************************************
**
** Ajax nexus data notes block object.
**
** @alias AjSNexusNotes
** @alias AjONexusNotes
**
** @new nexusNotesNew Default constructor
**
** @delete nexusNotesDel Default destructor
**
** @attr Text [AjPStr*] Text commands
** @attr Picture [AjPStr*] Picture commands
** @@
******************************************************************************/

typedef struct AjSNexusNotes {
    AjPStr* Text;
    AjPStr* Picture;
} AjONexusNotes;
#define AjPNexusNotes AjONexusNotes*



/* @data AjPNexus *************************************************************
**
** Ajax nexus data object.
**
** @alias AjSNexus
** @alias AjONexus
**
** @new ajNexusNew Default constructor
**
** @delete ajNexusDel Default destructor
**
** @attr Taxa [AjPNexusTaxa] Taxa
** @attr Characters [AjPNexusCharacters] Characters (or data)
** @attr Unaligned [AjPNexusUnaligned] Unaligned
** @attr Distances [AjPNexusDistances] Distances
** @attr Sets [AjPNexusSets] Sets
** @attr Assumptions [AjPNexusAssumptions] Assumptions
** @attr Codons [AjPNexusCodons] Codons
** @attr Trees [AjPNexusTrees] Trees
** @attr Notes [AjPNexusNotes] Notes
** @attr Ntax [ajuint] Number of taxa (wherever they were defined)
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSNexus {
    AjPNexusTaxa Taxa;
    AjPNexusCharacters Characters;
    AjPNexusUnaligned Unaligned;
    AjPNexusDistances Distances;
    AjPNexusSets Sets;
    AjPNexusAssumptions Assumptions;
    AjPNexusCodons Codons;
    AjPNexusTrees Trees;
    AjPNexusNotes Notes;
    ajuint Ntax;
    char Padding[4];
} AjONexus;
#define AjPNexus AjONexus*



/*
** Prototype definitions
*/

void     ajNexusDel(AjPNexus* pthys);
ajuint   ajNexusGetNtaxa(const AjPNexus thys);
AjPStr*  ajNexusGetTaxa(const AjPNexus thys);
AjPStr*  ajNexusGetSequences(AjPNexus thys);
AjPNexus ajNexusNew(void);
AjPNexus ajNexusParse(AjPFilebuff buff);
void     ajNexusTrace(const AjPNexus thys);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif

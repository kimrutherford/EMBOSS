/* @include ajpdb *************************************************************
**
** AJAX low-level functions for handling protein structural data.
** For use with the Atom, Chain and Pdb objects defined in ajpdb.h
** Also for use with Hetent, Het, Vdwres, Vdwall, Cmap and Pdbtosp objects
** (also defined in ajpdb.h).
** Includes functions for reading and writing ccf (clean coordinate file)
** format.
**
** @author Copyright (C) 2004 Jon Ison (jison@hgmp.mrc.ac.uk)
** @version $Revision: 1.32 $
** @modified $Date: 2012/04/12 20:37:30 $ by $Author: mks $
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

#ifndef AJPDB_H
#define AJPDB_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ajdefine.h"
#include "ajarr.h"
#include "ajstr.h"
#include "ajlist.h"
#include "ajfile.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* @enum AjECmapType **********************************************************
**
** AJAX Cmap Type enumeration
**
** @value ajECmapTypeNULL NULL
** @value ajECmapTypeIntra Intra-molecular contact
** @value ajECmapTypeInter Inter-molecular contact
** @value ajECmapTypeLigand Ligand contact
** @@
******************************************************************************/

typedef enum AjOCmapType
{
    ajECmapTypeNULL,
    ajECmapTypeIntra,
    ajECmapTypeInter,
    ajECmapTypeLigand
} AjECmapType;




/* @enum AjEPdbMethod *********************************************************
**
** AJAX PDB Method enumeration
**
** @value ajEPdbMethodNULL NULL
** @value ajEPdbMethodXray Structure was determined by X-ray crystallography
** @value ajEPdbMethodNmr  Structure was determined by NMR or is a model
** @@
******************************************************************************/

typedef enum AjOPdbMethod
{
    ajEPdbMethodNULL,
    ajEPdbMethodXray,
    ajEPdbMethodNmr
} AjEPdbMethod;




/* @enum AjEPdbMode ***********************************************************
**
** AJAX PDB Mode enumeration
**
** @value ajEPdbModeNULL NULL
** @value ajEPdbModePdb Original PDB residue numbering
** @value ajEPdbModeIdx Corrected residue numbering
**                      (index into SEQRES sequence)
** @@
******************************************************************************/

typedef enum AjOPdbMode
{
    ajEPdbModeNULL,
    ajEPdbModePdb,
    ajEPdbModeIdx
} AjEPdbMode;




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* @data AjPAtom **************************************************************
**
** Ajax atom object.
**
** Holds protein atom data
**
** AjPAtom is implemented as a pointer to a C data structure.
**
** @alias AjSAtom
** @alias AjOAtom
**
** @attr  Mod     [ajuint]  Model number
** @attr  Chn     [ajuint]  Chain number
** @attr  Gpn     [ajint]   Group number
** @attr  Idx     [ajuint]  Residue number - index into sequence.
** @attr  Pdb     [AjPStr]  Residue number string from the original PDB file.
** @attr  Id3     [AjPStr]  Residue or group identifier.
** @attr  Atm     [AjPStr]  Atom identifier.
** @attr  X       [float]   X coordinate.
** @attr  Y       [float]   Y coordinate.
** @attr  Z       [float]   Z coordinate.
** @attr  O       [float]   Occupancy.
** @attr  B       [float]   B value thermal factor.
** @attr  Id1     [char]    Standard residue identifier or 'X' for unknown
**                          types or '.' for heterogens and water.
** @attr  Type    [char]    'P' (protein atom), 'H' ("heterogens") or 'W'
**                          (water).
** @attr  Padding [char[2]] Padding to alignment boundary
**
** @new     ajAtomNew Default Atom constructor.
** @delete  ajAtomDel Default Atom destructor.
** @assign  ajAtomCopy Replicates an Atom object.
** @assign  ajAtomListCopy Replicates a list of Atom objects.
** @use     embAtomInContact Determines whether two atoms are in physical
**          contact.
** @use     embAtomDistance Returns the distance (Angstroms) between two
**          atoms.
** @use     embVdwRad Returns the van der Waals radius of an atom.
** @other   embPdbListHeterogens Construct a list of arrays of Atom objects
**          for ligands in the current Pdb object (a single array for each
**          ligand).
** @@
******************************************************************************/

typedef struct AjSAtom
{
    ajuint Mod;
    ajuint Chn;
    ajint Gpn;
    ajuint Idx;
    AjPStr Pdb;
    AjPStr Id3;
    AjPStr Atm;
    float X;
    float Y;
    float Z;
    float O;
    float B;
    char Id1;
    char Type;
    char Padding[2];
} AjOAtom;

#define AjPAtom AjOAtom*




/* @data AjPResidue ***********************************************************
**
** Ajax Residue object.
**
** Holds data for an amino acid residue.
**
** AjPResidue is implemented as a pointer to a C data structure.
**
** @alias AjSResidue
** @alias AjOResidue
**
** @attr Mod    [ajuint]    Model number
** @attr Chn    [ajuint]    Chain number
** @attr Idx    [ajuint]    Residue number. Can be used to index into the
**                          polypeptide sequence (the Seq element of an
**                          AjSChain object). Idx numbers start at 1 and run
**                          sequentially.
** @attr eNum   [ajint]     Element serial number (for secondary structure
**                          from the PDB file).
** @attr Pdb    [AjPStr]    Residue number string from the original PDB file.
** @attr Id3    [AjPStr]    3-letter residue identifier code.
** @attr eId    [AjPStr]    Element identifier (for secondary structure from
**                          the PDB file).
** @attr eClass [ajint]     Class of helix, an int from 1-10,  from
**        http://www.rcsb.org/pdb/docs/format/pdbguide2.2/guide2.2_frame.html
**                          (for secondary structure from the PDB file).
**
** @attr eStrideNum [ajint] Number of the element: sequential count from
**                          N-term (for secondary structure from STRIDE).
** @attr Phi    [float]     Phi angle.
** @attr Psi    [float]     Psi angle.
** @attr Area   [float]     Residue solvent accessible area.
**
** @attr all_abs    [float] Absolute accessibility, all atoms.
** @attr all_rel    [float] Relative accessibility, all atoms.
** @attr side_abs   [float] Absolute accessibility, atoms in side chain.
** @attr side_rel   [float] Relative accessibility, atoms in side chain.
** @attr main_abs   [float] Absolute accessibility, atoms in main chain.
** @attr main_rel   [float] Relative accessibility, atoms in main chain.
** @attr npol_abs   [float] Absolute accessibility, non-polar atoms.
** @attr npol_rel   [float] Relative accessibility, non-polar atoms.
** @attr pol_abs    [float] Absolute accessibility, polar atoms.
** @attr pol_rel    [float] Relative accessibility, polar atoms.
** @attr Id1        [char]  Standard 1-letter residue identifier or 'X' for
**                          unknown types.
** @attr eType      [char]  Element type COIL ('C'), HELIX ('H'), SHEET ('E')
**                          or TURN ('T'). Has a default value of COIL
**                          (for secondary structure from the PDB file).
** @attr eStrideType    [char]  Element type:
**                                  ALPHA HELIX ('H'),
**                                  3-10 HELIX ('G'),
**                                  PI-HELIX ('I'),
**                                  EXTENDED CONFORMATION ('E'),
**                                  ISOLATED BRIDGE ('B' or 'b'),
**                                  TURN ('T') or
**                                  COIL (none of the above) ('C')
**                              (for secondary structure from STRIDE).
** @attr Padding    [char[1]]   Padding to alignment boundary
**
** @new     ajResidueNew Default Residue constructor.
** @delete  ajResidueDel Default Residue destructor.
** @assign  ajResidueCopy Replicates a Residue object.
** @assign  ajResidueListCopy Replicates a list of Residue objects.
** @use ajResidueEnv1 Assigns environment based on side chain accessibility and
**  secondary structure.
** @use ajResidueEnv2 Assigns environment based on side chain accessibility and
**  secondary structure.
** @use ajResidueEnv3 Assigns environment based on side chain accessibility and
**  secondary structure.
** @use ajResidueEnv4 Assigns environment based on side chain accessibility and
**  secondary structure.
** @use ajResidueEnv5 Assigns environment based on side chain accessibility and
**  secondary structure.
** @use ajResidueEnv6 Assigns environment based on side chain accessibility and
**  secondary structure.
** @use ajResidueEnv7 Assigns environment based on side chain accessibility and
**  secondary structure.
** @use ajResidueEnv8 Assigns environment based on side chain accessibility and
**  secondary structure.
** @use ajResidueEnv9 Assigns environment based on side chain accessibility
**  and secondary structure.
** @use ajResidueEnv10 Assigns environment based on side chain accessibility
** and secondary structure.
** @use ajResidueEnv11 Assigns environment based on side chain accessibility
**  and secondary structure.
** @use ajResidueEnv12 Assigns environment based on side chain accessibility
**  and secondary structure.
** @use ajResidueEnv13 Assigns environment based on side chain accessibility
**  and secondary structure.
** @use ajResidueEnv14 Assigns environment based on side chain accessibility
**  and secondary structure.
** @use ajResidueEnv15 Assigns environment based on side chain accessibility
**  and secondary structure.
** @use ajResidueEnv16 Assigns environment based on side chain accessibility
**  and secondary structure.
** @use ajResidueSSEnv  Assigns secondary structure environment of a residue.
** @@
******************************************************************************/

typedef struct AjSResidue
{
    ajuint Mod;
    ajuint Chn;
    ajuint Idx;
    ajint eNum;
    AjPStr Pdb;
    AjPStr Id3;

    AjPStr eId;

    ajint eClass;
    ajint eStrideNum;
    float Phi;
    float Psi;
    float Area;

    float all_abs;
    float all_rel;
    float side_abs;
    float side_rel;
    float main_abs;
    float main_rel;
    float npol_abs;
    float npol_rel;
    float pol_abs;
    float pol_rel;
    char Id1;
    char eType;
    char eStrideType;
    char Padding[1];
} AjOResidue;

#define AjPResidue AjOResidue*




/* @data AjPChain *************************************************************
**
** Ajax chain object.
**
** Holds protein chain data
**
** AjPChain is implemented as a pointer to a C data structure.
**
** @alias AjSChain
** @alias AjOChain
**
** @attr  Nres        [ajuint]   Number of amino acid residues.
** @attr  Nlig        [ajuint]   Number of groups which are non-covalently
**                               associated with the chain, excluding water
**                               ("heterogens").
** @attr  numHelices  [ajint]    No. of helices in the chain according to the
**                               PDB file.
** @attr  numStrands  [ajint]    No. of strands in the chain according to the
**                               PDB file.
** @attr  Seq         [AjPStr]   Protein sequence as string.
** @attr  Atoms       [AjPList]  List of Atom objects for (potentially multiple
**                               models) of the polypeptide chain and any
**                               groups (ligands) that could be uniquely
**                               associated with a chain.
** @attr  Residues    [AjPList]  List of Residue objects for (potentially
**                               multiple models) of the polypeptide chain.
** @attr  Id          [char]     Chain id, ('.' if one wasn't specified in
**                               the PDB file).
** @attr Padding [char[7]] Padding to alignment boundary
**
**
**
** @new     ajChainNew Default Chain constructor.
** @delete  ajChainDel Default Chain destructor.
** @@
******************************************************************************/

typedef struct AjSChain
{
    ajuint Nres;
    ajuint Nlig;
    ajint numHelices;
    ajint numStrands;
    AjPStr Seq;
    AjPList Atoms;
    AjPList Residues;
    char Id;
    char Padding[7];
} AjOChain;

#define AjPChain AjOChain*




/* @data AjPPdb ***************************************************************
**
** Ajax pdb object.
**
** Holds arrays describing pdb data
**
** AjPPdb is implemented as a pointer to a C data structure.
**
** @alias AjSPdb
** @alias AjOPdb
**
** @attr Pdb    [AjPStr]        PDB code
** @attr Compnd [AjPStr]        Text from COMPND records in PDB file
** @attr Source [AjPStr]        Text from SOURCE records in PDB file
** @attr Method [AjEPdbMethod]  AJAX PDB Method enumeration
** @attr Reso   [float]         Resolution of an XRAY structure or 0
** @attr Nmod   [ajuint]        Number of models (always 1 for XRAY structures)
** @attr Nchn   [ajuint]        Number of polypeptide chains
** @attr Chains [AjPChain*]     Array of pointers to AjSChain structures
** @attr gpid   [AjPChar]       Array of chain (group) id's for groups that
**                              could not be uniquely associated with a chain.
** @attr Groups [AjPList]       List of Atom objects for groups that could not
**                              be uniquely associated with a chain.
** @attr Water  [AjPList]       List of Atom objects for water molecules.
** @attr Ngp    [ajuint]        Number of groups that could not be uniquely
**                              associated with a chain in the SEQRES records.
** @attr Padding    [char[4]]   Padding to alignment boundary
**
** @new     ajPdbNew Default Pdb constructor.
** @new     ajPdbReadRawNew Pdb constructor from reading pdb format file.
** @new     ajPdbReadNew Pdb constructor from reading CCF format file.
** @new     ajPdbReadFirstModelNew Pdb constructor from reading CCF format
**          file (retrieve data for 1st model only).
** @new     ajPdbReadoldNew Pdb constructor from reading CCF format file
**          lacking residue-level description in RE records.
** @new     ajPdbReadoldFirstModelNew Pdb constructor from reading CCF format
**          file lacking residue-level description in RE records.
** @delete  ajPdbDel Default Pdb destructor.
** @assign  ajPdbCopy Replicates a Pdb object.
** @use     ajPdbGetEStrideType Reads a Pdb object and writes a string with
**          the secondary structure.
** @use     ajPdbChnidToNum Finds the chain number for a given chain
**          identifier.
** @use     embPdbToIdx Reads a Pdb object and writes an integer which gives
**          the index into the protein sequence for a residue with a
**          specified pdb residue number and a specified chain number.
** @use     embPdbidToSp  Read a pdb identifier code and writes the equivalent
**          swissprot identifier code.
** @use     embPdbidToAcc Read a pdb identifier code and writes the equivalent
**          accession number.
** @use     embPdbidToScop Writes a list of scop identifier codes for the
**          domains that a Pdb object contains.
** @cast    embPdbAtomIndexI Reads a Pdb object and writes an integer array
**          which gives the index into the protein sequence for structured
**          residues (residues for which electron density was determined) for
**          a given chain.
** @cast    embPdbAtomIndexICA Reads a Pdb object and writes an integer array
**          which gives the index into the protein sequence for structured
**          residues (residues for which electron density was determined) for
**          a given chain, EXCLUDING those residues for which CA atoms are
**          missing.
** @cast    embPdbAtomIndexCCA Reads a Pdb object and writes an integer array
**          which gives the index into the protein sequence for structured
**          residues (residues for which electron density was determined) for
**          a given chain, EXCLUDING those residues for which CA atoms are
**          missing.
** @output  ajPdbWriteDomainRecordRaw Writes lines to a pdb format file for
**          a domain.
** @output  ajPdbWriteRecordRaw Writes lines to a pdb format file for a
**          protein.
** @output  ajPdbWriteAllRaw Writes a pdb-format file for a protein.
** @output  ajPdbWriteDomainRaw Writes a pdb-format file for a SCOP domain.
** @output  ajPdbWriteAll Writes a CCF-format file for a protein.
** @output  ajPdbWriteDomain Writes a CCF-format file for a domain).
** @output  ajPdbWriteSegment Writes a CCF-format file for a segment of a
**          protein.
** @cast    embPdbListHeterogens Construct a list of arrays of Atom objects
**          for ligands in the current Pdb object (a single array for each
**          ligand).
**
** @@
******************************************************************************/

typedef struct AjSPdb
{
    AjPStr Pdb;
    AjPStr Compnd;
    AjPStr Source;
    AjEPdbMethod Method;
    float Reso;
    ajuint Nmod;
    ajuint Nchn;
    AjPChain *Chains;
    AjPChar gpid;
    AjPList Groups;
    AjPList Water;
    ajuint Ngp;
    char Padding[4];
} AjOPdb;

#define AjPPdb AjOPdb*




/* @data AjPHetent ************************************************************
**
** AJAX Hetent object.
**
** Holds a single entry from a dictionary of heterogen groups.
**
** AjPHetent is implemented as a pointer to a C data structure.
**
** @alias AjSHetent
** @alias AjOHetent
**
** @attr  abv     [AjPStr]  3-letter abbreviation of heterogen.
** @attr  syn     [AjPStr]  Synonym.
** @attr  ful     [AjPStr]  Full name.
** @attr  cnt     [ajint]   No. of occurrences (files) of this heterogen in a
**                          directory.
** @attr  Padding [char[4]] Padding to alignment boundary
**
** @new     ajHetentNew Default Hetent constructor.
** @delete  ajHetentDel Default Hetent destructor.
**
** @@
******************************************************************************/

typedef struct AjSHetent
{
    AjPStr abv;
    AjPStr syn;
    AjPStr ful;
    ajint cnt;
    char Padding[4];
} AjOHetent;

#define AjPHetent AjOHetent*




/* @data AjPHet ***************************************************************
**
** AJAX Het object.
** Holds a dictionary of heterogen groups.
**
** AjPHet is implemented as a pointer to a C data structure.
**
** @alias AjSHet
** @alias AjOHet
**
** @attr  Entries  [AjPHetent*] Array of AJAX Heterogen Entries
** @attr  Number   [ajuint] Number of entries
** @attr  Padding  [char[4]] Padding to alignment boundary
**
** @new     ajHetNew Default Het constructor.
** @new     ajHetReadRawNew Het constructor from reading dictionary of
**          heterogen groups in raw format.
** @new     ajHetReadNew Het constructor from reading dictionary of
**          heterogen groups in embl-like format (see documentation for the
**          EMBASSY DOMAINATRIX package).
** @delete  ajHetDel Default Het destructor.
** @output  ajHetWrite Write Het object to file in embl-like format (see
**          documentation for the EMBASSY DOMAINATRIX package).
**
** @@
******************************************************************************/

typedef struct AjSHet
{
    AjPHetent *Entries;
    ajuint Number;
    char Padding[4];
} AjOHet;

#define AjPHet AjOHet*




/* @data AjPVdwres ************************************************************
**
** AJAX Vdwres object.
**
** Holds the Van der Waals radius for atoms in a residue
**
** AjPVdwres is implemented as a pointer to a C data structure.
**
** @alias AjSVdwres
** @alias AjOVdwres
**
** @attr  Id3  [AjPStr] 3 character residue identifier.
** @attr  Atm  [AjPStr*] Array of atom identifiers.
** @attr  Rad  [float*] Array of van der Waals radii.
** @attr  N    [ajuint] Number of atoms in residue.
** @attr  Id1  [char] Standard residue identifier or 'X' for unknown.
** @attr  Padding [char[3]] Padding to alignment boundary
**
** @new     ajVdwresNew Default Vdwres constructor.
** @delete  ajVdwresDel Default Vdwres destructor.
** @@
******************************************************************************/

typedef struct AjSVdwres
{
    AjPStr Id3;
    AjPStr *Atm;
    float *Rad;
    ajuint N;
    char Id1;
    char Padding[3];
} AjOVdwres;

#define AjPVdwres AjOVdwres*




/* @data AjPVdwall ************************************************************
**
** Ajax Vdwall object.
**
** Holds the Van der Waals radii for all types of protein atoms
**
** AjPVdwall is implemented as a pointer to a C data structure.
**
** @alias AjSVdwall
** @alias AjOVdwall
**
** @attr  Res     [AjPVdwres*] Array of Vdwres structures.
** @attr  N       [ajuint] Number of residues.
** @attr  Padding [char[4]] Padding to alignment boundary
**
** @new     ajVdwallNew Default Vdwall constructor.
** @new     ajVdwallReadNew Vdwall constructor from reading file in embl-like
**          format (see documentation for the EMBASSY DOMAINATRIX package).
** @delete  ajVdwallDel Default Vdwall destructor.
** @@
******************************************************************************/

typedef struct AjSVdwall
{
    AjPVdwres *Res;
    ajuint N;
    char Padding[4];
} AjOVdwall;

#define AjPVdwall AjOVdwall*




/* @data AjPCmap **************************************************************
**
** Ajax Cmap object.
**
** Holds a contact map and associated data for a protein domain / chain
** (intra or inter-chain contacts) or between a protein / domain and a
** ligand.  For ligand contacts, the first row / column only of the contact
** map is used.
**
** AjPCmap is implemented as a pointer to a C data structure.
**
**
** @alias AjSCmap
** @alias AjOCmap
**
**
** for intra-chain, inter-chain and chain-ligand contacts respectively.
** @attr  Id    [AjPStr]   Protein id code.
** @attr  Domid [AjPStr]   Domain id code.
** @attr  Ligid [AjPStr]   Ligand id code.
** @attr  Chn1  [ajint]    Chain number 1 (first chain)
** @attr  Chn2  [ajint]    Chain number 2 (second chain if available)
** @attr  Nres1 [ajint]    Number of residues in chain/domain 1
** @attr  Nres2 [ajint]    Number of residues in chain/domain 2
** @attr  Seq1  [AjPStr]   The sequence of the first domain or chain.
** @attr  Seq2  [AjPStr]   The sequence of the second domain or chain.
** @attr  Mat   [AjPUint2d] Contact map.
** @attr  Dim   [ajint]    Dimension of contact map.
** @attr  Ncon  [ajint]    No. of contacts (1's in contact map).
** @attr  en    [ajint]    Entry number.
** @attr  ns    [ajint]    No. of sites (ajLIGAND only)
** @attr  sn    [ajint]    Site number (ajLIGAND only)
** @attr  Type  [AjECmapType] AJAX Cmap Type enumeration
** @attr  Desc  [AjPStr]   Description of ligand (ajLIGAND only)
** @attr  Chid1 [char]     Chain identifier 1 (first chain)
** @attr  Chid2 [char]     Chain identifier 2 (second chain if available)
** @attr  Padding [char[6]] Padding to alignment boundary
**
**
** @new     ajCmapNew Default Cmap constructor.
** @new     ajCmapReadINew Cmap constructor from reading file in CON
**          (embl-like)format (see documentation for the EMBASSY DOMAINATRIX
**          package).
** @new     ajCmapReadCNew Cmap constructor from reading file in CON
**          format (see documentation for the EMBASSY DOMAINATRIX package).
** @new     ajCmapReadNew Cmap constructor from reading file in CON
**          format (see documentation for the EMBASSY DOMAINATRIX package).
** @new     ajCmapReadAllNew Constructs list of Cmap objects from reading file
**          in CON format (see documentation for the EMBASSY
**          DOMAINATRIX package).
** @delete  ajCmapDel Default Cmap destructor.
** @output  ajCmapWrite Write Cmap object to file in CON format.
**
** @@
******************************************************************************/

typedef struct AjSCmap
{
    AjPStr Id;
    AjPStr Domid;
    AjPStr Ligid;

    ajint Chn1;
    ajint Chn2;
    ajint Nres1;
    ajint Nres2;
    AjPStr Seq1;
    AjPStr Seq2;

    AjPUint2d Mat;
    ajint Dim;
    ajint Ncon;

    ajint en;
    ajint ns;
    ajint sn;
    AjECmapType Type;
    AjPStr Desc;
    char Chid1;
    char Chid2;
    char Padding[6];
} AjOCmap;

#define AjPCmap AjOCmap*




/* @data AjPPdbtosp ***********************************************************
**
** Ajax Pdbtosp object.
**
** Holds swissprot codes and accession numbers for a PDB code.
**
** AjPPdbtosp is implemented as a pointer to a C data structure.
**
**
**
** @alias AjSPdbtosp
** @alias AjOPdbtosp
**
**
**
** @attr  Pdb     [AjPStr]   PDB code
** @attr  Acc     [AjPStr*]  Accession numbers
** @attr  Spr     [AjPStr*]  Swissprot codes
** @attr  Number  [ajuint]   Number of entries for this pdb code
** @attr  Padding [char[4]]  Padding to alignment boundary
**
**
**
** @new     ajPdbtospNew Default Pdbtosp constructor.
** @new     ajPdbtospReadAllRawNew Pdbtosp constructor from reading swissprot-
**          pdb equivalence table in raw format.
** @new     ajPdbtospReadNew Pdbtosp constructor from reading file in
**          embl-like format (see documentation for the EMBASSY DOMAINATRIX
**          package).
** @new     ajPdbtospReadCNew Pdbtosp constructor from reading file in
**          embl-like format (see documentation for the EMBASSY DOMAINATRIX
**          package).
** @delete  ajPdbtospDel Default Pdbtosp destructor.
** @new     ajPdbtospReadAllNew Constructor for list of Pdbtosp objects from
**          reading file in embl-like format (see documentation for the
**          EMBASSY DOMAINATRIX package).
** @output  ajPdbtospWrite Write Pdbtosp object to file in embl-like format
**          (see documentation for the EMBASSY DOMAINATRIX package).
** @use     ajPdbtospArrFindPdbid Binary search for Pdb element over array
**          of Pdbtosp objects.
**
** @@
******************************************************************************/

typedef struct AjSPdbtosp
{
    AjPStr Pdb;
    AjPStr *Acc;
    AjPStr *Spr;
    ajuint Number;
    char Padding[4];
} AjOPdbtosp;

#define AjPPdbtosp AjOPdbtosp*




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */




/*
** Prototype definitions
*/


/* ======================================================================= */
/* ============================ Het objects ============================== */
/* ======================================================================= */

AjPHet ajHetNew(ajuint n);
void ajHetDel(AjPHet *Phet);




/* ======================================================================= */
/* ============================ Hetent object ============================ */
/* ======================================================================= */

AjPHetent ajHetentNew(void);
void ajHetentDel(AjPHetent *Phetent);




/* ======================================================================= */
/* ============================ Vdwall object ============================ */
/* ======================================================================= */

AjPVdwall ajVdwallNew(ajuint n);
void ajVdwallDel(AjPVdwall *Pvdwall);




/* ======================================================================= */
/* ============================ Vdwres object ============================ */
/* ======================================================================= */

AjPVdwres ajVdwresNew(ajuint n);
void ajVdwresDel(AjPVdwres *Pvdwres);




/* ======================================================================= */
/* ============================ Atom object ============================== */
/* ======================================================================= */

AjPAtom ajAtomNew(void);
void ajAtomDel(AjPAtom *Patom);
ajint ajAtomCopy(AjPAtom *Pto, const AjPAtom from);
ajint ajAtomListCopy(AjPList *Pto, const AjPList from);




/* ======================================================================= */
/* ========================== Residue object ============================= */
/* ======================================================================= */

AjPResidue ajResidueNew(void);
void ajResidueDel(AjPResidue *Presidue);
ajint ajResidueCopy(AjPResidue *Pto, const AjPResidue from);
AjBool ajResidueListCopy(AjPList *to, const AjPList from);

ajint ajResidueEnv1(const AjPResidue residue, char SEnv,
                    AjPStr *OEnv, AjPFile flog);
ajint ajResidueEnv2(const AjPResidue residue, char SEnv,
                    AjPStr *OEnv, AjPFile flog);
ajint ajResidueEnv3(const AjPResidue residue, char SEnv,
                    AjPStr *OEnv, AjPFile flog);
ajint ajResidueEnv4(const AjPResidue residue, char SEnv,
                    AjPStr *OEnv, AjPFile flog);
ajint ajResidueEnv5(const AjPResidue residue, char SEnv,
                    AjPStr *OEnv, AjPFile flog);
ajint ajResidueEnv6(const AjPResidue residue, char SEnv,
                    AjPStr *OEnv, AjPFile flog);
ajint ajResidueEnv7(const AjPResidue residue, char SEnv,
                    AjPStr *OEnv, AjPFile flog);
ajint ajResidueEnv8(const AjPResidue residue, char SEnv,
                    AjPStr *OEnv, AjPFile flog);
ajint ajResidueEnv9(const AjPResidue residue, char SEnv,
                    AjPStr *OEnv, AjPFile flog);
ajint ajResidueEnv10(const AjPResidue residue, char SEnv,
                     AjPStr *OEnv, AjPFile flog);
ajint ajResidueEnv11(const AjPResidue residue, char SEnv,
                     AjPStr *OEnv, AjPFile flog);
ajint ajResidueEnv12(const AjPResidue residue, char SEnv,
                     AjPStr *OEnv, AjPFile flog);
ajint ajResidueEnv13(const AjPResidue residue, char SEnv,
                     AjPStr *OEnv, AjPFile flog);
ajint ajResidueEnv14(const AjPResidue residue, char SEnv,
                     AjPStr *OEnv, AjPFile flog);
ajint ajResidueEnv15(const AjPResidue residue, char SEnv,
                     AjPStr *OEnv, AjPFile flog);
ajint ajResidueEnv16(const AjPResidue residue, char SEnv,
                     AjPStr *OEnv, AjPFile flog);

ajint ajResidueSSEnv(const AjPResidue residue, char *SEnv, AjPFile flog);




/* ======================================================================= */
/* ============================ Pdbtosp object =========================== */
/* ======================================================================= */

AjPPdbtosp ajPdbtospNew(ajuint n);
void ajPdbtospDel(AjPPdbtosp *Ppdbtosp);
ajint ajPdbtospArrFindPdbid(AjPPdbtosp const *array, ajint size,
                            const AjPStr identifier);




/* ======================================================================= */
/* ============================ Chain object ============================= */
/* ======================================================================= */

AjPChain ajChainNew(void);
void ajChainDel(AjPChain *Pchain);




/* ======================================================================= */
/* ============================ Pdb object =============================== */
/* ======================================================================= */

AjPPdb ajPdbNew(ajuint n);
void ajPdbDel(AjPPdb *Ppdb);
AjBool ajPdbCopy(AjPPdb *Pto, const AjPPdb from);
AjBool ajPdbChnidToNum(char identifier, const AjPPdb pdb, ajuint *chn);

AjPPdb ajPdbReadNew(AjPFile inf, ajint mode);
AjPPdb ajPdbReadAllModelsNew(AjPFile inf);
AjPPdb ajPdbReadFirstModelNew(AjPFile inf);
AjPPdb ajPdbReadoldNew(AjPFile inf);
AjPPdb ajPdbReadoldFirstModelNew(AjPFile inf);
AjBool ajPdbWriteAll(AjPFile out, const AjPPdb pdb);
AjBool ajPdbWriteSegment(AjPFile outf, const AjPPdb pdb,
                         const AjPStr seq, char chn,
                         const AjPStr domain,
                         AjPFile errf);
ajint ajPdbGetEStrideType(const AjPPdb obj, ajuint chn,
                          AjPStr *EStrideType);




/* ======================================================================= */
/* ====================== Het & Hetent objects =========================== */
/* ======================================================================= */

AjPHet ajHetReadRawNew(AjPFile inf);
AjPHet ajHetReadNew(AjPFile inf);
AjBool ajHetWrite(AjPFile outf, const AjPHet het, AjBool dogrep);




/* ======================================================================= */
/* ================ Vdwall, Vdwres object ================================ */
/* ======================================================================= */

AjPVdwall ajVdwallReadNew(AjPFile inf);




/* ======================================================================= */
/* =========================== Cmap object =============================== */
/* ======================================================================= */

AjPCmap ajCmapNew(ajuint n);
AjPCmap ajCmapReadCNew(AjPFile inf, char chn, ajint mod);
AjPCmap ajCmapReadINew(AjPFile inf, ajint chn, ajint mod);
AjPCmap ajCmapReadNew(AjPFile inf, ajint mode, ajint chn, ajint mod);
AjPList ajCmapReadAllNew(AjPFile inf);
void ajCmapDel(AjPCmap *Pcmap);
AjBool ajCmapWrite(AjPFile outf, const AjPCmap cmap);




/* ======================================================================= */
/* ======================== Pdbtosp object =============================== */
/* ======================================================================= */

AjPList ajPdbtospReadAllRawNew(AjPFile inf);
AjPPdbtosp ajPdbtospReadNew(AjPFile inf, const AjPStr entry);
AjPPdbtosp ajPdbtospReadCNew(AjPFile inf, const char *entry);
AjPList ajPdbtospReadAllNew(AjPFile inf);
AjBool ajPdbtospWrite(AjPFile outf, const AjPList list);

void ajPdbExit(void);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !AJPDB_H */
